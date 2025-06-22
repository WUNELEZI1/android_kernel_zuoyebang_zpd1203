// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kfifo.h>
#include "dpu_gem.h"
#include "dpu_cmdlist_frame_mgr.h"
#include "dpu_cmdlist_node.h"
#include "dpu_hw_rdma_path_reg.h"
#include "dpu_wfd.h"
#include "dksm_iommu.h"

#define GET_CONTIG_REG_ADDR(base, index) ((base) + (index) * 0x04)
#define RDMA_LAYER_OFFSET_REG_NUM 2
#define ADDR_LOW_BIT_SIZE 32
#define NEXT_ONE_REG 1
#define MAX_VIDEO_SLICE_NOD_CNT 8
#define SLICE_HEIGHT_ALIGN 64
#define MAX_VIDEO_BYPASS_CNT 3

#define HAS_TRANSFORM(f) ((f & (DRM_MODE_ROTATE_90 | DRM_MODE_ROTATE_180 | \
		DRM_MODE_ROTATE_270 | DRM_MODE_REFLECT_MASK)) || \
		!g_wfd_sink_slice_enabled)

static s64 g_wfd_sink_slice_node_id[MAX_VIDEO_SLICE_CNT];
static s64 g_wfd_sink_bypass_node_id[MAX_VIDEO_BYPASS_CNT];
static s64 g_cfg_rdy_node_id;
static u8 g_wfd_sink_present_index;
static u8 g_wfd_sink_last_present_index;
static u8 g_wfd_sink_slice_enabled;

static struct iova_domain g_iova_dom[MAX_VIDEO_TLB_COUNT];
static int g_last_iova_dom_index;

static bool g_wfd_sink_initialized;

typedef void (*xplayer_notify_event_func)(enum xplayer_event);
static xplayer_notify_event_func xplayer_notify_event;

static bool dpu_wfd_source_check(struct wb_frame_cfg *wb_cfg)
{
	if ((wb_cfg->frame_flip_mode != NO_FLIP) ||
		(wb_cfg->frame_rot_mode != ROT_0)) {
		DPU_WARN("WFD source can not support rotation or flip");
		return false;
	}

	return true;
}

static u64 dpu_wfd_source_get_slice_info_addr(struct dpu_gem_object *obj,
		u32 buffer_size)
{
	struct sg_dma_page_iter dma_iter = {0};
	u32 page_num = buffer_size / PAGE_SIZE;
	u32 offset = buffer_size % PAGE_SIZE;
	u64 slice_info_addr = 0;
	dma_addr_t dma_addr;
	int num = 0;

	if (!obj || !obj->sgt) {
		DPU_ERROR("invalid parameter\n");
		return slice_info_addr;
	}

	for_each_sgtable_dma_page(obj->sgt, &dma_iter, 0) {
		num++;
		dma_addr = sg_page_iter_dma_address(&dma_iter);
		if (num == page_num) {
			slice_info_addr = dma_addr + offset;
			break;
		}
	}

	DPU_DEBUG("slice_info_addr: 0x%llx\n", slice_info_addr);
	return slice_info_addr;
}

static bool dpu_wfd_sink_check(struct rdma_layer_cfg *layer_cfg)
{
	if (layer_cfg->extender_mode) {
		DPU_WARN("WFD sink can not support extender mode");
		return false;
	}

	return true;
}

int dpu_wfd_source_create_slice_property(struct drm_writeback_connector *drm_wb_conn,
		struct dpu_wb_connector *dpu_wb_conn)
{
	struct drm_property *prop;

	prop = drm_property_create_range(drm_wb_conn->base.dev, 0,
			"wfd_source_slice_num", 0, 4);
	if (!prop)
		return -ENOMEM;

	drm_object_attach_property(&drm_wb_conn->base.base, prop, 0);
	dpu_wb_conn->wfd_source_slice_prop = prop;

	return 0;
}

void dpu_wfd_source_config(struct wb_frame_cfg *wb_cfg,
		struct dpu_wb_connector_state *wb_state,
		struct drm_framebuffer *fb)
{
	struct dpu_gem_object *gem_obj;
	u64 sliceinfo_addr_va;
	u64 sliceinfo_addr_pa;
	u32 buffer_size;
	u64 dma_addr;

	gem_obj = to_dpu_gem_obj(fb->obj[0]);
	dma_addr = gem_obj->iova_dom ? gem_obj->iova_dom->iova : gem_obj->paddr;

	if (wb_state->wfd_source_slice_num == 0)
		return;

	if (!dpu_wfd_source_check(wb_cfg)) {
		wb_cfg->slice_header_wren = 0;
		return;
	}

	wb_cfg->slice_num = wb_state->wfd_source_slice_num;
	if (wb_cfg->slice_num > 0) {
		wb_cfg->slice_header_wren = 1;
		wb_cfg->slice_size = ALIGN(
				wb_cfg->output_original_height / wb_cfg->slice_num, SLICE_HEIGHT_ALIGN);
		buffer_size = fb->obj[0]->size;
		DPU_DEBUG("buffer_size: %d\n", buffer_size);
		sliceinfo_addr_va = dma_addr + buffer_size;
		/* sliceinfo_addr must be PA */
		sliceinfo_addr_pa = dpu_wfd_source_get_slice_info_addr(gem_obj, buffer_size);
		wb_cfg->sliceinfor_addr_low = sliceinfo_addr_pa;
		wb_cfg->sliceinfor_addr_high = sliceinfo_addr_pa >> ADDR_LOW_BIT_SIZE;
		DPU_DEBUG("slice_num: %d, slice_size: %d\n",
				wb_cfg->slice_num, wb_cfg->slice_size);
	} else {
		wb_cfg->slice_header_wren = 0;
	}
}

static u8 dpu_wfd_sink_get_present_index(u8 slice_cnt, bool has_transform)
{
	u8 present_index = g_wfd_sink_present_index;

	present_index = (present_index >= slice_cnt || has_transform) ?
			(slice_cnt - 1) : present_index;
	if (present_index != g_wfd_sink_last_present_index) {
		DPU_INFO("change %d to %d\n", g_wfd_sink_last_present_index, present_index);
		cmdlist_wait_event_update(g_wfd_sink_slice_node_id[g_wfd_sink_last_present_index],
				RDMA_SLICE_DONE);
		cmdlist_wait_event_update(g_wfd_sink_slice_node_id[present_index], 0);
		cmdlist_node_type_update(g_wfd_sink_slice_node_id[present_index], NORMAL_TYPE);
		cmdlist_node_type_update(g_cfg_rdy_node_id, NORMAL_TYPE);
		g_wfd_sink_last_present_index = present_index;
	}
	return present_index;
}

bool dpu_wfd_sink_cmdlist_append(struct drm_plane *plane, s64 frame_id)
{
	struct dpu_plane_state *dpu_pstate;
	struct dpu_plane *dpu_plane;
	u8 valid_slice_cnt;
	u8 present_index;
	int i;

	dpu_plane = to_dpu_plane(plane);
	dpu_pstate = to_dpu_plane_state(plane->state);
	if (dpu_pstate->slice_cnt == 0)
		return false;

	valid_slice_cnt = dpu_pstate->slice_cnt;

	DPU_DEBUG("valid_slice_cnt is %d\n", valid_slice_cnt);
	bool has_transform = plane->state->crtc_h < (plane->state->src_h >> 16) ||
			HAS_TRANSFORM(plane->state->rotation);
	present_index = dpu_wfd_sink_get_present_index(valid_slice_cnt, has_transform);
	cmdlist_frame_node_append(frame_id, &g_wfd_sink_slice_node_id[present_index],
			USED_BY_FRAME);
	cmdlist_frame_node_append(frame_id, &g_cfg_rdy_node_id, USED_BY_FRAME);
	if (has_transform)
		return true;

	for (i = 0; i < present_index; i++)
		cmdlist_frame_node_append(frame_id, &g_wfd_sink_bypass_node_id[i], USED_BY_FRAME);
	for (i = present_index + 1; i < valid_slice_cnt; i++)
		cmdlist_frame_node_append(frame_id, &g_wfd_sink_slice_node_id[i], USED_BY_FRAME);

	return true;
}

void dpu_wfd_sink_get_slice_info(struct rdma_layer_cfg *layer_cfg,
		struct drm_plane_state *plane_state,
		struct drm_plane *plane)
{
	struct dpu_plane_state *state;
	struct dpu_plane *dpu_plane;

	state = to_dpu_plane_state(plane_state);
	dpu_plane = to_dpu_plane(plane);

	layer_cfg->slice_cnt = state->slice_cnt;

	if (layer_cfg->slice_cnt == 0)
		return;

	if (!dpu_wfd_sink_check(layer_cfg)) {
		layer_cfg->slice_cnt = 0;
		return;
	}

	if (plane_state->crtc_h < (plane_state->src_h >> 16) ||
			HAS_TRANSFORM(plane_state->rotation))
		layer_cfg->slice_cnt = 1;

	layer_cfg->layer_addrs[0] = WFD_SINK_IOVA_RESERVED;

	if (layer_cfg->slice_cnt > 1) {
		layer_cfg->slice_line_num =
				ALIGN(plane_state->crtc_h / layer_cfg->slice_cnt, SLICE_HEIGHT_ALIGN);
		DPU_DEBUG("slice_line_num: %d\n", layer_cfg->slice_line_num);
	}
}

void dpu_wfd_sink_update_iova(u64 *iova, bool wfd_slice_en)
{

	if (!wfd_slice_en)
		return;

	*iova = WFD_SINK_IOVA_RESERVED;
}

void dpu_wfd_sink_set_sg_table(struct dma_buf *buf, struct sg_table *sgt,
		int tlb_index, struct xplayer_iommu_format_info *iommu_info)
{
	if (!sgt || !buf || !iommu_info) {
		DPU_ERROR("nullptr, %pK, %pK, %pK\n", buf, sgt, iommu_info);
		return;
	}

	dksm_iommu_fill_pt_entry(buf, sgt, (struct iommu_format_info *)iommu_info,
			&g_iova_dom[tlb_index]);
}
EXPORT_SYMBOL(dpu_wfd_sink_set_sg_table);

void dpu_wfd_event_register(xplayer_notify_event_func func)
{
	xplayer_notify_event = func;
}
EXPORT_SYMBOL(dpu_wfd_event_register);

void dpu_wfd_event_unregister(void)
{
	xplayer_notify_event = NULL;
}
EXPORT_SYMBOL(dpu_wfd_event_unregister);

void dpu_wfd_notify_event(enum xplayer_event ev)
{
	if (xplayer_notify_event)
		xplayer_notify_event(ev);
}

void dpu_wfd_sink_cmdlist_init(void)
{
	s64 id;
	int i;

	for (i = 0; i < MAX_VIDEO_SLICE_CNT; i++) {
		id = cmdlist_node_create(WFD_RESERVE_RCH_ID, NORMAL_TYPE,
				MAX_VIDEO_SLICE_NOD_CNT, i == 0 ? 0 : RDMA_SLICE_DONE);
		DPU_INFO("wfd_sink_slice_node_id[%d]: 0x%llx\n", i, id);

		cmdlist_node_config(id, 0xc104, 0);
		cmdlist_node_config(id, 0xc034, WFD_SINK_IOVA_RESERVED);
		g_wfd_sink_slice_node_id[i] = id;
	}

	for (i = 0; i < MAX_VIDEO_BYPASS_CNT; i++) {
		id = cmdlist_node_create(WFD_RESERVE_RCH_ID, NORMAL_TYPE,
				MAX_VIDEO_SLICE_NOD_CNT, RDMA_SLICE_DONE);
		DPU_INFO("wfd_sink_bypass_node_id[%d]: 0x%llx\n", i, id);

		cmdlist_node_config(id, 0xc034, WFD_SINK_IOVA_RESERVED);
		g_wfd_sink_bypass_node_id[i] = id;
	}
	g_cfg_rdy_node_id = cmdlist_node_create(WFD_RESERVE_RCH_ID, NORMAL_TYPE,
			MAX_VIDEO_SLICE_NOD_CNT, RDMA_SLICE_DONE);
	DPU_INFO("cfg_rdy_node_id: 0x%llx\n", g_cfg_rdy_node_id);
	cmdlist_node_config(g_cfg_rdy_node_id, 0x3f4,
			CMDLIST_RDMA_CFG_RDY(WFD_RESERVE_RCH_ID) |
			CMDLIST_PREPQ_CFG_RDY(WFD_RESERVE_RCH_ID));

	for (i = 0; i < MAX_VIDEO_TLB_COUNT; i++)
		dksm_iommu_create_iova_domain(MAX_VIDEO_SIZE, &g_iova_dom[i]);
}

void dpu_wfd_sink_cmdlist_deinit(void)
{
	int i;

	for (i = 0; i < MAX_VIDEO_SLICE_CNT; i++)
		cmdlist_node_delete(g_wfd_sink_slice_node_id[i]);
	for (i = 0; i < MAX_VIDEO_BYPASS_CNT; i++)
		cmdlist_node_delete(g_wfd_sink_bypass_node_id[i]);
	cmdlist_node_delete(g_cfg_rdy_node_id);

	for (i = 0; i < MAX_VIDEO_TLB_COUNT; i++)
		dksm_iommu_destory_iova_domain(&g_iova_dom[i]);
}

void dpu_wfd_sink_init(void)
{
	int i;

	if (g_wfd_sink_initialized)
		return;

	for (i = 0; i < MAX_VIDEO_SLICE_CNT; i++) {
		cmdlist_node_type_update(g_wfd_sink_slice_node_id[i], NORMAL_TYPE);
		cmdlist_wait_event_update(g_wfd_sink_slice_node_id[i], i == 0 ? 0 : RDMA_SLICE_DONE);
	}
	for (i = 0; i < MAX_VIDEO_BYPASS_CNT; i++)
		cmdlist_node_type_update(g_wfd_sink_bypass_node_id[i], NORMAL_TYPE);
	cmdlist_node_type_update(g_cfg_rdy_node_id, NORMAL_TYPE);

	dpu_idle_enable_ctrl(false);
	g_wfd_sink_present_index = 0;
	g_wfd_sink_last_present_index = 0;
	g_wfd_sink_initialized = true;
}
EXPORT_SYMBOL(dpu_wfd_sink_init);

void dpu_wfd_sink_deinit(void)
{
	if (!g_wfd_sink_initialized)
		return;

	dpu_idle_enable_ctrl(true);
	g_wfd_sink_initialized = false;
}
EXPORT_SYMBOL(dpu_wfd_sink_deinit);

void dpu_wfd_sink_get_cmdlist_addr(dma_addr_t phy_addr[])
{
	struct cmdlist_node *node;
	int i;

	if (phy_addr == NULL) {
		DPU_ERROR("phy_addr is null pointer\n");
		return;
	}

	for (i = 0; i < MAX_VIDEO_SLICE_CNT; i++) {
		node = get_cmdlist_node(g_wfd_sink_slice_node_id[i]);
		if (node == NULL) {
			DPU_ERROR("node is null pointer\n");
			return;
		}
		phy_addr[i] = node->header_addr + 0x10;
	}
}
EXPORT_SYMBOL(dpu_wfd_sink_get_cmdlist_addr);

void dpu_wfd_sink_get_cmdlist_config(uint32_t value[], int *tlb_index)
{
	/**
	 * Use WFD_RESERVE_RCH_ID for WFD layer
	 * the left_base_addr0_low addr is 0xC034
	 * the reserved iova for WFD layer is 0x8000000ULL
	 * the cmdlist item structures are as follows:
	 * -------------+-------------+-------------+---+--------------+-------------
	 *  Module_cfg_ | Module_cfg_ | Module_cfg_ | 0 | Module_cfg_  | Module_cfg
	 *  data2[31:0] | data1[31:0] | data0[31:0] |   | strobe[11:0] | _addr[18:0]
	 * -------------+-------------+-------------+---+--------------+-------------
	 */

	*tlb_index = g_last_iova_dom_index;
	g_last_iova_dom_index = (g_last_iova_dom_index + 1) % MAX_VIDEO_TLB_COUNT;

	value[0] = (0xC104 >> 2) | (0xFFF << 19);
	value[1] = g_iova_dom[*tlb_index].tlb_pa;
	value[2] = 0;
	value[3] = g_iova_dom[*tlb_index].tlb_pa;

	DPU_DEBUG("write_value: 0x%x, 0x%x, 0x%x, 0x%x", value[0], value[1], value[2], value[3]);
}
EXPORT_SYMBOL(dpu_wfd_sink_get_cmdlist_config);

void dpu_wfd_sink_set_present_index(u8 slice_index)
{
	g_wfd_sink_present_index = slice_index;
}
EXPORT_SYMBOL(dpu_wfd_sink_set_present_index);
