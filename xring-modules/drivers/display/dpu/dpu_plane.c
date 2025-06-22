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

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/sync_file.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_blend.h>
#include "dpu_kms.h"
#include "dpu_cap.h"
#include "dpu_format.h"
#include "dpu_gem.h"
#include "dpu_plane.h"
#include "dpu_crtc.h"
#include "dpu_hw_mixer.h"
#include "dpu_hw_tpg_ops.h"
#include "dksm_iommu.h"
#include "dpu_pre_color.h"
#include "dpu_cmdlist.h"
#include "dpu_wfd.h"
#include "dpu_sec_drm.h"
#include "dpu_power_mgr.h"
#include "dpu_core_perf.h"
#include "dpu_flow_ctrl.h"
#include "dpu_cmdlist_node.h"

#define GET_16x16_VALUE(src_value)    ((src_value) >> 16)
#define RAW_ROTATE_HEIGHT_LIMIT    544

#define LINE_SPACE_ALIGN(src_value, value)    \
		(DIV_ROUND_UP((src_value), (value)))

#define IS_ODD_RCH(rch_id)    \
		((rch_id) == RCH_V_0 || (rch_id) == RCH_V_1 \
		|| (rch_id) == RCH_G_3 || (rch_id) == RCH_V_2 \
		|| (rch_id) == RCH_G_5)

#define GET_RECT_END(length, start)    ((length) + (start) - 1)
#define ONE_SRAM_SIZE    4096
#define DOUBLE_SRAM_SIZE    8192

#define RCH_LBUF_MAX_VALUE                0xA000200000000000
#define RCH_LBUF_MEM_MAP_MASK             0x8000000000000000
#define RCH_LBUF_MEM_DEFAULT              0x0800080008000000
#define RCH_LBUF_MEM_VALUE_START          48
#define RCH_LEFT_LBUF_MEM_VALUE_START     32
#define RCH_RIGHT_LBUF_MEM_VALUE_START    16
#define ACQUIRE_FENCE_FD_DEFAULT          -1

static const u64 plane_format_modifiers[] = {
	RCH_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_NONE, AFBC_NONE),
	RCH_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_NONE, AFBC_SPLIT),
	RCH_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_YTR,  AFBC_NONE),
	RCH_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_YTR,  AFBC_SPLIT),
	RCH_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_NONE, AFBC_NONE),
	RCH_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_NONE, AFBC_SPLIT),
	RCH_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_YTR,  AFBC_NONE),
	RCH_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_YTR,  AFBC_SPLIT),
	RCH_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_NONE, AFBC_NONE),
	RCH_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_NONE, AFBC_SPLIT),
	RCH_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_YTR,  AFBC_NONE),
	RCH_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_YTR,  AFBC_SPLIT),
	RCH_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_NONE, AFBC_NONE),
	RCH_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_NONE, AFBC_SPLIT),
	RCH_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_YTR,  AFBC_NONE),
	RCH_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_YTR,  AFBC_SPLIT),
	DRM_FORMAT_MOD_INVALID
};

static inline bool is_dpu_plane_using_vchannel(u32 rch_id)
{
	switch (rch_id) {
	case RCH_V_0:
	case RCH_V_1:
	case RCH_V_2:
	case RCH_V_3:
		return true;
	default:
		return false;
	}
}

static int dpu_plane_pipe_check(struct drm_plane_state *pstate,
		struct drm_framebuffer *fb)
{
	const struct dpu_format_map *format_map;
	struct dpu_plane_state *state = NULL;
	struct dpu_sram_param *sram = NULL;
	bool is_video_rch;
	u16 max_lbuf_size;
	u8 rotate_mode;

	state = to_dpu_plane_state(pstate);
	format_map = dpu_format_get(FMT_MODULE_RCH, fb->format->format,
			fb->modifier);
	is_video_rch = is_dpu_plane_using_vchannel(state->rch_id);

	sram = &state->sram_param;

	if (sram->total_sram_size == 0) {
		DPU_ERROR("rch %d don't have sram size, do not work", state->rch_id);
		return -EINVAL;
	}
	max_lbuf_size = IS_ODD_RCH(state->rch_id) ?
			DOUBLE_SRAM_SIZE : ONE_SRAM_SIZE;
	if (sram->total_sram_size > max_lbuf_size) {
		DPU_ERROR("total lbuf mem siz overflow: %d, max: %d\n",
				sram->total_sram_size, max_lbuf_size);
		return -EINVAL;
	}

	/* check sram mem map status */
	if (IS_ODD_RCH(state->rch_id)) {
		if ((sram->sram_base_addr + sram->total_sram_size) > ONE_SRAM_SIZE &&
				!state->sram_param.sram_mem_map) {
			DPU_ERROR("rch %d used two sram, mem_map must true", state->rch_id);
			return -EINVAL;
		}
	} else if (sram->sram_mem_map != true) {
		DPU_ERROR("even rch %d used, mem_map must true", state->rch_id);
		return -EINVAL;
	}

	/* check relationg between left_sram_size and total_mem_size */
	if (!DPU_FMT_IS_AFBC(fb->modifier) &&
			format_map->plane_count != PLANE_ONE &&
			(sram->left_sram_size << 1) != sram->total_sram_size) {
		DPU_ERROR("multiple plane format need specifical mem size");
		return -EINVAL;
	}
	if (state->extender_mode == EXTENDER_MODE_RIGHT &&
			(sram->left_sram_size + sram->right_sram_size !=
			sram->total_sram_size)) {
		DPU_ERROR("h-extender mode, total mem size unequal left add right");
		return -EINVAL;
	}

	/* check video rdma capability */
	rotate_mode = (DRM_MODE_ROTATE_90 | DRM_MODE_ROTATE_270);
	if (!is_video_rch) {
		if ((pstate->rotation & rotate_mode) || format_map->is_yuv) {
			DPU_ERROR("need video rdma channel process");
			return -EINVAL;
		}
	}

	return 0;
}

static int dpu_plane_scale_size_check(struct drm_plane_state *plane_state,
		u32 rch_id, u32 src_height, u32 src_width)
{
	u32 scale_input_width_max = MAX_HW_RCH_VX_SCALE_INPUT_WIDTH;
	u32 scale_input_height;
	u32 scale_input_width;
	u32 scale_height;
	u32 scale_width;
	u8 rotate_mode;

	rotate_mode = (DRM_MODE_ROTATE_90 | DRM_MODE_ROTATE_270);
	if (plane_state->rotation & rotate_mode) {
		scale_input_height = src_width;
		scale_input_width = src_height;
	} else {
		scale_input_height = src_height;
		scale_input_width = src_width;
	}

	if (scale_input_height == plane_state->crtc_h ||
			scale_input_width == plane_state->crtc_w)
		return 0;

	if ((rch_id == RCH_V_0) && (scale_input_height != plane_state->crtc_h) &&
			(scale_input_width != plane_state->crtc_w)) {
		scale_input_width_max = MAX_HW_RCH_V0_SCALE_INPUT_WIDTH;
	}

	if ((scale_input_height > MAX_HW_RCH_SCALE_INPUT_HEIGHT) ||
			(scale_input_height < MIN_HW_RCH_SCALE_INPUT_HEIGHT) ||
			(scale_input_width > scale_input_width_max) ||
			(scale_input_width < MIN_HW_RCH_SCALE_INPUT_WIDTH)) {
		DPU_ERROR("rch_id: %d, invalid scale input size: ", rch_id);
		DPU_ERROR("src_height: %d, src_width: %d\n",
				scale_input_height, scale_input_width);
		DPU_ERROR("src_height_limit: %d src_width_limit: %d\n",
				MIN_HW_RCH_SCALE_INPUT_HEIGHT,
				scale_input_width_max);
		return -EINVAL;
	}

	if ((plane_state->crtc_h > MAX_HW_RCH_SCALE_OUTPUT_HEIGHT) ||
			(plane_state->crtc_w > MAX_HW_RCH_SCALE_OUTPUT_WIDTH) ||
			(plane_state->crtc_h < MIN_HW_RCH_SCALE_OUTPUT_HEIGHT) ||
			(plane_state->crtc_w < MIN_HW_RCH_SCALE_OUTPUT_WIDTH)) {
		DPU_ERROR("rch_id: %d, invalid scaler output size: ", rch_id);
		DPU_ERROR("crtc_height: %d, crtc_width: %d\n",
				plane_state->crtc_h, plane_state->crtc_w);
		return -EINVAL;
	}

	if ((scale_input_height > plane_state->crtc_h) ||
			(scale_input_width > plane_state->crtc_w)) {
		scale_height = scale_input_height >> MAX_HW_RCH_SCALE_DOWN;
		scale_width = scale_input_width >> MAX_HW_RCH_SCALE_DOWN;
		if ((scale_height > plane_state->crtc_h) ||
				(scale_width > plane_state->crtc_w)) {
			DPU_ERROR("invalid scale size down, ");
			DPU_ERROR("src_w:%d, src_h:%d, scale_down_max:%lu\n",
					scale_input_width, scale_input_height,
					BIT(MAX_HW_RCH_SCALE_DOWN));
			return -EINVAL;
		}
	} else {
		scale_height = scale_input_height << MAX_HW_RCH_SCALE_UP;
		scale_width = scale_input_width << MAX_HW_RCH_SCALE_UP;
		if ((scale_height < plane_state->crtc_h) ||
				(scale_width < plane_state->crtc_w)) {
			DPU_ERROR("invalid scale size up, ");
			DPU_ERROR("src_w:%d, src_h:%d, scale_up_max:%lu\n",
					scale_width, scale_height,
					BIT(MAX_HW_RCH_SCALE_UP));
			return -EINVAL;
		}
	}

	return 0;
}

static int dpu_plane_size_check(struct drm_framebuffer *fb,
		struct drm_plane_state *pstate, struct dpu_rect_v2 src_rect)
{
	struct dpu_plane_state *state = NULL;
	u32 height_limit;
	u32 width_limit;
	u8 rotate_mode;
	int ret = 0;

	state = to_dpu_plane_state(pstate);
	if ((src_rect.w > MAX_HW_RCH_WIDTH) ||
			(src_rect.h > MAX_HW_RCH_HEIGHT) ||
			(src_rect.w < MIN_HW_RCH_WIDTH) ||
			(src_rect.h < MIN_HW_RCH_HEIGHT)) {
		DPU_ERROR("invalid rch_input_size, width :%d, height :%d\n",
				src_rect.w, src_rect.h);
		return -EINVAL;
	}

	if ((pstate->crtc_h > MAX_HW_RCH_SCALE_OUTPUT_HEIGHT) ||
			(pstate->crtc_w > MAX_HW_RCH_SCALE_OUTPUT_WIDTH) ||
			(pstate->crtc_h < MIN_HW_RCH_HEIGHT) ||
			(pstate->crtc_w < MIN_HW_RCH_WIDTH)) {
		DPU_ERROR("invalid dst size, dst_width :%d, dst_height :%d\n",
				pstate->crtc_w, pstate->crtc_h);
		return -EINVAL;
	}

	width_limit = src_rect.x + src_rect.w;
	height_limit = src_rect.y + src_rect.h;
	if ((width_limit > fb->width) || (height_limit > fb->height)) {
		DPU_ERROR("invalid crop size, ");
		DPU_ERROR("width :%d, height :%d, fb_width :%d, fb_height :%d\n",
				width_limit, height_limit,
				fb->width, fb->height);
		return -EINVAL;
	}

	rotate_mode = (DRM_MODE_ROTATE_90 | DRM_MODE_ROTATE_270);
	if ((!(DPU_FMT_IS_AFBC(fb->modifier))) &&
			(pstate->rotation & rotate_mode) &&
			(src_rect.h > RAW_ROTATE_HEIGHT_LIMIT)) {
		DPU_ERROR("invalid raw image size for rotate, ");
		DPU_ERROR("height limit: %d, src_height: %d\n",
				RAW_ROTATE_HEIGHT_LIMIT, src_rect.h);
		return -EINVAL;
	}

	if (is_dpu_plane_using_vchannel(state->rch_id))
		ret = dpu_plane_scale_size_check(pstate,
			state->rch_id, src_rect.h, src_rect.w);

	return ret;
}

static int dpu_plane_atomic_check(struct drm_plane *plane,
		struct drm_atomic_state *state)
{
	struct drm_plane_state *plane_state = NULL;
	struct dpu_plane_state *dpu_plane_state;
	struct dpu_drm_device *dpu_drm_dev;
	struct drm_framebuffer *fb = NULL;
	struct dpu_plane *dpu_plane;
	struct dpu_kms *dpu_kms;
	struct dpu_rect_v2 src_rect;
	u32 crtc_index;
	u32 ret;

	if (!plane || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				plane, state);
		return -EINVAL;
	}

	dpu_drm_dev = to_dpu_drm_dev(plane->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	dpu_plane = to_dpu_plane(plane);
	plane_state = drm_atomic_get_new_plane_state(state, plane);
	dpu_plane_state = to_dpu_plane_state(plane_state);

	fb = plane_state->fb;
	if (!fb) {
		LAYER_DEBUG("plane[%d]: no fb in reset case\n",
				drm_plane_index(plane));
		return 0;
	}

	if (!plane_state->crtc) {
		LAYER_DEBUG("plane[%d]: is disabled\n", drm_plane_index(plane));
		return 0;
	}

	if (dpu_plane_state->rch_id >= RCH_MAX) {
		DPU_ERROR("invalid plane rch_id :%d\n", dpu_plane_state->rch_id);
		return -EINVAL;
	}

	crtc_index = drm_crtc_index(plane_state->crtc);

	ret = dpu_plane_pipe_check(plane_state, fb);
	if (ret) {
		DPU_ERROR("crtc %d: failed to do pipe check, ret %d\n",
				crtc_index,
				ret);
		return -EINVAL;
	}

	src_rect.x = GET_16x16_VALUE(plane_state->src_x);
	src_rect.y = GET_16x16_VALUE(plane_state->src_y);
	src_rect.h = GET_16x16_VALUE(plane_state->src_h);
	src_rect.w = GET_16x16_VALUE(plane_state->src_w);

	ret = dpu_plane_size_check(fb, plane_state, src_rect);
	if (ret) {
		DPU_ERROR("crtc %d: failed to do size check, ret %d\n",
				crtc_index,
				ret);
		return -EINVAL;
	}

	return 0;
}

static u32 dpu_plane_get_axi_port(u32 rch_id, u32 axi_id)
{
	u32 axi_port_cfg = 0;

	LAYER_DEBUG("axi_id %u\n", axi_id);
	if (rch_id < RCH_G_4) {
		if (axi_id == RDMA_AXI_0)
			axi_port_cfg = 0;
		else if (axi_id == RDMA_AXI_1)
			axi_port_cfg = 1;
		else if (axi_id == RDMA_AXI_DEFAULT)
			axi_port_cfg = (rch_id > RCH_G_1) ? 1 : 0;
		else
			DPU_WARN("invalid axi %u for rch %u\n", axi_id, rch_id);
	}

	return axi_port_cfg;
}

static void dpu_plane_get_addr_info(struct rdma_layer_cfg *layer_cfg,
		struct drm_framebuffer *fb, u32 plane_count)
{
	struct iommu_format_info format_info;
	struct dpu_gem_object *dpu_gem_obj;
	u64 dma_addr;

	dpu_gem_obj = to_dpu_gem_obj(fb->obj[0]);
	if (!dpu_gem_obj->iova_dom) {
		DPU_INFO("iova_dom is null, need get now\n");
		memcpy(format_info.offsets, fb->offsets, sizeof(format_info.offsets));
		format_info.is_afbc = fb->modifier && DPU_FMT_IS_AFBC(fb->modifier);
		dpu_gem_obj->iova_dom = dksm_iommu_map_dmabuf(dpu_gem_obj->dmabuf, dpu_gem_obj->sgt,
				&format_info);
	}

	dma_addr = dpu_gem_obj->iova_dom ? dpu_gem_obj->iova_dom->iova : dpu_gem_obj->paddr;

	switch (plane_count) {
	case PLANE_ONE:
		layer_cfg->layer_strides[STRIDE_Y_RGB] = fb->pitches[STRIDE_Y_RGB];
		layer_cfg->layer_addrs[PLANE_Y] = dma_addr + fb->offsets[PLANE_Y];
		break;
	case PLANE_TWO:
		layer_cfg->layer_strides[STRIDE_Y_RGB] = fb->pitches[STRIDE_Y_RGB];
		layer_cfg->layer_addrs[PLANE_Y] = dma_addr + fb->offsets[PLANE_Y];
		layer_cfg->layer_strides[STRIDE_UV] = fb->pitches[STRIDE_UV];
		layer_cfg->layer_addrs[PLANE_U] = dma_addr + fb->offsets[PLANE_U];
		break;
	case PLANE_THREE:
		layer_cfg->layer_strides[STRIDE_Y_RGB] = fb->pitches[STRIDE_Y_RGB];
		layer_cfg->layer_addrs[PLANE_Y] = dma_addr + fb->offsets[PLANE_Y];
		layer_cfg->layer_strides[STRIDE_UV] = fb->pitches[STRIDE_UV];
		layer_cfg->layer_addrs[PLANE_U] = dma_addr + fb->offsets[PLANE_U];
		layer_cfg->layer_addrs[PLANE_V] = dma_addr + fb->offsets[PLANE_V];
		break;
	default:
		DPU_ERROR("invalid format plane count: %d\n", plane_count);
		break;
	}
}

static void dpu_plane_get_exclusive_roi_info(struct rdma_layer_cfg *layer_cfg,
		struct dpu_plane_roi *roi_cfg)
{
	layer_cfg->roi0_en = false;
	layer_cfg->roi1_en = false;

	if (roi_cfg->count >= 1) {
		LAYER_DEBUG("roi0, x: %d, y: %d, w: %d, h: %d\n",
			roi_cfg->roi[0].x, roi_cfg->roi[0].y,
			roi_cfg->roi[0].w, roi_cfg->roi[0].h);
		layer_cfg->roi0_en = true;
		layer_cfg->roi0_area.x1 = roi_cfg->roi[0].x;
		layer_cfg->roi0_area.x2 =
			GET_RECT_END(roi_cfg->roi[0].w, roi_cfg->roi[0].x);
		layer_cfg->roi0_area.y1 = roi_cfg->roi[0].y;
		layer_cfg->roi0_area.y2 =
			GET_RECT_END(roi_cfg->roi[0].h, roi_cfg->roi[0].y);
	}

	if (roi_cfg->count == 2) {
		LAYER_DEBUG("roi1, x: %d, y: %d, w: %d, h: %d\n",
			roi_cfg->roi[1].x, roi_cfg->roi[1].y,
			roi_cfg->roi[1].w, roi_cfg->roi[1].h);
		layer_cfg->roi1_en = true;
		layer_cfg->roi1_area.x1 = roi_cfg->roi[1].x;
		layer_cfg->roi1_area.x2 =
			GET_RECT_END(roi_cfg->roi[1].w, roi_cfg->roi[1].x);
		layer_cfg->roi1_area.y1 = roi_cfg->roi[1].y;
		layer_cfg->roi1_area.y2 =
			GET_RECT_END(roi_cfg->roi[1].h, roi_cfg->roi[1].y);
	}
}

static void dpu_plane_get_rotation_info(struct rdma_layer_cfg *layer_cfg,
		u32 rotation)
{
	u8 flip_mode;
	u8 rot_mode;

	flip_mode = rotation & DRM_MODE_REFLECT_MASK;
	switch (flip_mode) {
	case DRM_MODE_REFLECT_X:
		layer_cfg->layer_flip_mode = H_FLIP;
		break;
	case DRM_MODE_REFLECT_Y:
		layer_cfg->layer_flip_mode = V_FLIP;
		break;
	case DRM_MODE_REFLECT_MASK:
		layer_cfg->layer_flip_mode = HV_FLIP;
		break;
	default:
		layer_cfg->layer_flip_mode = NO_FLIP;
		break;
	}

	rot_mode = rotation & DRM_MODE_ROTATE_MASK;
	switch (rot_mode) {
	case DRM_MODE_ROTATE_90:
		layer_cfg->layer_rot_mode = ROT_90;
		break;
	case DRM_MODE_ROTATE_180:
		layer_cfg->layer_rot_mode = ROT_180;
		break;
	case DRM_MODE_ROTATE_270:
		layer_cfg->layer_rot_mode = ROT_270;
		break;
	default:
		layer_cfg->layer_rot_mode = ROT_0;
		break;
	}
}

static void dpu_plane_get_rch_sram_info(struct rdma_layer_cfg *layer_cfg,
		struct  drm_plane_state *plane_state)
{
	struct dpu_plane_state *state = NULL;

	state = to_dpu_plane_state(plane_state);

	layer_cfg->lbuf_size = state->sram_param.total_sram_size;
	layer_cfg->left_lbuf_size = state->sram_param.left_sram_size;
	layer_cfg->right_lbuf_size = state->sram_param.right_sram_size;
	layer_cfg->lbuf_base_addr = state->sram_param.sram_base_addr;
	layer_cfg->lbuf_mem_map = state->sram_param.sram_mem_map;

	LAYER_DEBUG("lbuf: size: %d, left: %d, right: %d, base: %d, sw: %d\n",
		layer_cfg->lbuf_size, layer_cfg->left_lbuf_size,
		layer_cfg->right_lbuf_size, layer_cfg->lbuf_base_addr,
		layer_cfg->lbuf_mem_map);
}

static void dpu_plane_get_layer_cfg(struct rdma_layer_cfg *layer_cfg,
		struct drm_plane *plane)
{
	const struct dpu_format_map *format_map;
	struct drm_plane_state *plane_state;
	struct dpu_plane_state *dpu_plane_state;
	struct drm_framebuffer *fb;
	struct dpu_rect_v2 src_rect;

	plane_state = plane->state;
	dpu_plane_state = to_dpu_plane_state(plane_state);
	fb = plane_state->fb;
	format_map = dpu_format_get(FMT_MODULE_RCH, fb->format->format,
			fb->modifier);

	layer_cfg->drm_format = fb->format->format;
	layer_cfg->pixel_format = format_map->dpu_format;
	layer_cfg->is_yuv = format_map->is_yuv;
	layer_cfg->is_uv_swap = format_map->uv_swap;

	/* TODO: temporarily only support wide yuv, it will be modified */
	layer_cfg->is_narrow_yuv = false;

	layer_cfg->extender_mode = dpu_plane_state->extender_mode;

	layer_cfg->is_afbc = DPU_FMT_IS_AFBC(fb->modifier);
	layer_cfg->is_fbc_split_mode = DPU_FMT_HAS_AFBC_SPLIT(fb->modifier);
	layer_cfg->is_fbc_32x8 = DPU_FMT_IS_AFBC_32x8(fb->modifier);
	layer_cfg->is_fbc_yuv_transform = DPU_FMT_HAS_AFBC_Y2R(fb->modifier);
	layer_cfg->is_fbc_tile_mode = DPU_FMT_HAS_AFBC_TILED(fb->modifier);

	layer_cfg->plane_count = format_map->plane_count;
	if (layer_cfg->is_afbc)
		layer_cfg->plane_count = PLANE_ONE;

	dpu_plane_get_addr_info(layer_cfg, fb, layer_cfg->plane_count);
	LAYER_DEBUG("IOVA: rch:%u Y:0x%llx U:0x%llx V:0x%llx",
			dpu_plane_state->rch_id, layer_cfg->layer_addrs[PLANE_Y],
			layer_cfg->layer_addrs[PLANE_U], layer_cfg->layer_addrs[PLANE_V]);

	src_rect.h = GET_16x16_VALUE(plane_state->src_h);
	src_rect.w = GET_16x16_VALUE(plane_state->src_w);
	src_rect.x = GET_16x16_VALUE(plane_state->src_x);
	src_rect.y = GET_16x16_VALUE(plane_state->src_y);

	layer_cfg->crop_area.x1 = src_rect.x;
	layer_cfg->crop_area.x2 = MINUS_ONE(src_rect.x + src_rect.w);
	layer_cfg->crop_area.y1 = src_rect.y;
	layer_cfg->crop_area.y2 = MINUS_ONE(src_rect.y + src_rect.h);

	layer_cfg->img_size.w = fb->width;
	layer_cfg->img_size.h = fb->height;

	dpu_plane_get_exclusive_roi_info(layer_cfg, &dpu_plane_state->roi_cfg);

	dpu_plane_get_rotation_info(layer_cfg, plane_state->rotation);

	dpu_plane_get_rch_sram_info(layer_cfg, plane_state);

	dpu_wfd_sink_get_slice_info(layer_cfg, plane_state, plane);

	layer_cfg->layer_mixer_y_offset = plane_state->crtc_y;
}

static void dpu_tpg_plane_enable(struct drm_plane *plane)
{
	struct tpg_config cfg;
	struct dpu_plane *dpu_plane;
	struct drm_plane_state *plane_state;
	int tpg_pos;

	dpu_plane = to_dpu_plane(plane);

	tpg_pos = GET_TPG_POSITION(g_dpu_tpg_ctrl);
	if (dpu_plane->hw_rch->hw.blk_id != tpg_pos)
		return;

	plane_state = plane->state;
	cfg.height = GET_16x16_VALUE(plane_state->src_h);
	cfg.width = GET_16x16_VALUE(plane_state->src_w);
	cfg.mode_index = GET_TPG_MODE(g_dpu_tpg_ctrl);
	cfg.tpg_enable = GET_TPG_ENABLE(g_dpu_tpg_ctrl);

	dpu_tpg_param_set(&cfg, g_dpu_tpg_param);

	if (cfg.tpg_enable &&
			(is_dpu_lp_enabled(DPU_LP_TOP_AUTO_CG_ENABLE) ||
			is_dpu_lp_enabled(DPU_LP_SRAM_LP_ENABLE)))
		DPU_ERROR("TPG need disable top clk auto cg & disable sram lp\n");

	dpu_hw_tpg_enable(dpu_plane->hw_rch->hw.iomem_base,
			tpg_pos, &cfg);
}

static void dpu_plane_atomic_update(struct drm_plane *plane,
		struct drm_atomic_state *state)
{
	struct dpu_plane_state *dpu_plane_state;
	struct rdma_layer_cfg layer_cfg;
	struct framebuffer_info fb_info;
	struct rdma_pipe_cfg pipe_cfg;
	struct dpu_plane *dpu_plane;
	struct dpu_crtc *dpu_crtc;
	struct dpu_hw_rch *hw_rch;
	struct drm_crtc *crtc;
	struct dpu_flow_ctrl *flow_ctrl;

	if (!plane || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				plane, state);
		return;
	}

	crtc = plane->state->crtc;
	if (!crtc) {
		LAYER_DEBUG("plane %d disable\n", drm_plane_index(plane));
		return;
	}

	dpu_plane = to_dpu_plane(plane);
	hw_rch = dpu_plane->hw_rch;
	dpu_plane_state = to_dpu_plane_state(plane->state);

	dpu_crtc = to_dpu_crtc(crtc);
	pipe_cfg.mixer_id = dpu_crtc->hw_mixer->hw.blk_id;
	pipe_cfg.axi_port = dpu_plane_get_axi_port(dpu_plane_state->rch_id, dpu_plane_state->axi_id);

	/* TODO: temporarily only support online & wb0, it will be modified */
	pipe_cfg.offline_wb_id = WB0;

	pipe_cfg.is_offline = false;
	/* is_offline decide obuf arqos lvl come from wb or real obuf */
	if (pipe_cfg.mixer_id == MIXER_2)
		pipe_cfg.is_offline = true;

	layer_cfg.drm_enable_status = dpu_sec_drm_rdma_cfg(plane, state);
	dpu_plane_get_layer_cfg(&layer_cfg, plane);

	dpu_flow_qos_get(FLOW_QOS_RCH_RDMA, &pipe_cfg.is_offline, &layer_cfg.qos_cfg);

	if (hw_rch) {
		flow_ctrl = dpu_flow_ctrl_get(state->dev);
		dpu_cmdlist_plane_node_update(dpu_plane);

		if (dpu_plane_state->extender_mode <= EXTENDER_MODE_V)
			hw_rch->ops.reset(&hw_rch->hw);

		hw_rch->ops.layer_config(&hw_rch->hw, &hw_rch->channel_param, &layer_cfg);
		hw_rch->ops.channel_config(&hw_rch->hw, &hw_rch->channel_param,
				&layer_cfg, &pipe_cfg);

		dpu_tpg_plane_enable(plane);

		dpu_gem_get_framebuffer_info(plane->state->fb, &fb_info);
		dpu_wfd_sink_update_iova(&fb_info.iova, layer_cfg.slice_cnt > 0);

		dpu_flow_qos_get(FLOW_QOS_MMU, NULL, &fb_info.qos);

		if (dpu_plane_state->extender_mode == EXTENDER_MODE_RIGHT) {
			hw_rch->ops.mmu_tbu_config(&hw_rch->hw, &fb_info,
					MODULE_RIGHT_RDMA);
		} else {
			hw_rch->ops.mmu_tbu_config(&hw_rch->hw, &fb_info,
					MODULE_LEFT_RDMA);
		}

		dpu_flow_line_buf_level_cfg(flow_ctrl, &hw_rch->hw, FLOW_LBUF_LEVEL_RDMA, &layer_cfg);

		if (dpu_plane->color)
			dpu_plane->color->funcs->commit(plane);
	}
}

static void dpu_plane_atomic_disable(struct drm_plane *plane,
		struct drm_atomic_state *state)
{
	LAYER_DEBUG("plane disable\n");

	if (!plane || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				plane, state);
		return;
	}
}

static void dpu_plane_color_blk_id_get(u32 rch_id,
		struct dpu_plane_color_blks *blks)
{
	blks->scaler_id = -1;
	blks->lut3d_id = -1;
	blks->hist_id = -1;

	switch (rch_id) {
	case RCH_V_0:
		blks->scaler_id = RCH_V_0;
		blks->lut3d_id = RCH_V_0;
		blks->hist_id = RCH_V_0;
		break;
	case RCH_V_1:
		blks->scaler_id = RCH_V_1;
		break;
	case RCH_V_2:
		blks->scaler_id = RCH_V_2;
		break;
	case RCH_V_3:
		blks->scaler_id = RCH_V_3;
		break;
	default:
		break;
	}
}

static int dpu_plane_color_hw_blk_release(struct dpu_res_mgr_ctx *ctx,
		struct dpu_plane *dpu_plane)
{
	if (!dpu_plane->color)
		return 0;

	if (dpu_plane->color->hw_prepipe_top) {
		dpu_res_mgr_block_release(ctx,
				&dpu_plane->color->hw_prepipe_top->hw);
		dpu_plane->color->hw_prepipe_top = NULL;
	}

	if (dpu_plane->color->hw_scaler) {
		dpu_res_mgr_block_release(ctx,
				&dpu_plane->color->hw_scaler->hw);
		dpu_plane->color->hw_scaler = NULL;
	}

	if (dpu_plane->color->hw_tm) {
		dpu_res_mgr_block_release(ctx,
				&dpu_plane->color->hw_tm->hw);
		dpu_plane->color->hw_tm = NULL;
	}

	if (dpu_plane->color->hw_hist) {
		dpu_res_mgr_block_release(ctx,
				&dpu_plane->color->hw_hist->hw);
		dpu_plane->color->hw_hist = NULL;
	}

	if (dpu_plane->color->hw_3dlut) {
		dpu_res_mgr_block_release(ctx,
				&dpu_plane->color->hw_3dlut->hw);
		dpu_plane->color->hw_3dlut = NULL;
	}

	return 0;
}

static int dpu_plane_color_hw_blk_reserve(struct dpu_res_mgr_ctx *ctx,
		struct dpu_plane *dpu_plane, struct dpu_plane_state *dpu_pstate,
		u32 pipe, u32 rch_id)
{
	struct dpu_plane_color_blks blks;
	struct dpu_hw_blk *hw_blk;

	if (!dpu_plane->color)
		return 0;

	dpu_plane_color_blk_id_get(rch_id, &blks);

	hw_blk = dpu_res_mgr_block_reserve_id(ctx,
		pipe, BLK_PREPIPE_TOP, rch_id, false);
	if (IS_ERR_OR_NULL(hw_blk)) {
		DPU_ERROR("reserve prepipe top blk failed, rch-%d\n", rch_id);
		return PTR_ERR(hw_blk);
	}
	dpu_plane->color->hw_prepipe_top = to_dpu_hw_prepipe_top(hw_blk);

	if (blks.scaler_id >= 0 && dpu_pstate->extender_mode <= EXTENDER_MODE_V) {
		hw_blk = dpu_res_mgr_block_reserve_id(ctx,
				pipe, BLK_PRE_SCALER, blks.scaler_id, false);
		if (IS_ERR_OR_NULL(hw_blk)) {
			DPU_ERROR("failed to reserve pre scaler %d\n", blks.scaler_id);
			return PTR_ERR(hw_blk);
		}
		dpu_plane->color->hw_scaler = to_dpu_hw_pre_scaler(hw_blk);
	} else {
		dpu_plane->color->hw_scaler = NULL;
	}

	hw_blk = dpu_res_mgr_block_reserve_id(ctx,
			pipe, BLK_TM, rch_id, false);
	if (IS_ERR_OR_NULL(hw_blk)) {
		DPU_ERROR("failed to reserve pre tm %d\n", rch_id);
		return PTR_ERR(hw_blk);
	}
	dpu_plane->color->hw_tm = to_dpu_hw_tm(hw_blk);

	if (blks.hist_id >= 0 && dpu_pstate->extender_mode == EXTENDER_MODE_NONE) {
		hw_blk = dpu_res_mgr_block_reserve_id(ctx,
				pipe, BLK_PRE_HIST, blks.hist_id, false);
		if (IS_ERR_OR_NULL(hw_blk)) {
			DPU_ERROR("failed to reserve pre hist %d\n", blks.hist_id);
			return PTR_ERR(hw_blk);
		}
		dpu_plane->color->hw_hist = to_dpu_hw_pre_hist(hw_blk);
	} else {
		dpu_plane->color->hw_hist = NULL;
		RES_DEBUG("pre hist id: %d, extender mode: %d\n",
				blks.hist_id, dpu_pstate->extender_mode);
	}

	if (blks.lut3d_id >= 0) {
		hw_blk = dpu_res_mgr_block_reserve_id(ctx,
				pipe, BLK_PRE_3DLUT, blks.lut3d_id, false);
		if (IS_ERR_OR_NULL(hw_blk)) {
			DPU_ERROR("failed to reserve pre 3dlut %d\n", blks.lut3d_id);
			return PTR_ERR(hw_blk);
		}
		dpu_plane->color->hw_3dlut = to_dpu_hw_pre_3dlut(hw_blk);
	} else {
		dpu_plane->color->hw_3dlut = NULL;
	}

	return 0;
}

static int dpu_plane_rch_hw_blk_release(struct dpu_res_mgr_ctx *ctx,
		struct dpu_plane *dpu_plane)
{
	if (dpu_plane->hw_rch) {
		dpu_res_mgr_block_release(ctx, &dpu_plane->hw_rch->hw);
		dpu_plane->hw_rch = NULL;
	}

	if (dpu_plane->hw_rch_top) {
		dpu_res_mgr_block_release(ctx, &dpu_plane->hw_rch_top->hw);
		dpu_plane->hw_rch_top = NULL;
	}

	return 0;
}

static int dpu_plane_rch_hw_blk_reserve(struct dpu_res_mgr_ctx *ctx,
		struct dpu_plane *dpu_plane, struct dpu_plane_state *dpu_pstate,
		u32 pipe, u32 blk_id)
{
	struct dpu_hw_blk *hw_blk;

	hw_blk = dpu_res_mgr_block_reserve_id(ctx,
			pipe, BLK_RCH, blk_id, false);
	if (IS_ERR_OR_NULL(hw_blk)) {
		DPU_ERROR("failed to reserve rch%d blk\n", blk_id);
		return PTR_ERR(hw_blk);
	}
	dpu_plane->hw_rch = to_dpu_hw_rch(hw_blk);

	hw_blk = dpu_res_mgr_block_reserve_id(ctx,
			pipe, BLK_RCH_TOP, blk_id, false);
	if (IS_ERR_OR_NULL(hw_blk)) {
		DPU_ERROR("failed to reserve rch_top%d blk\n", blk_id);
		return PTR_ERR(hw_blk);
	}
	dpu_plane->hw_rch_top = to_dpu_hw_rch_top(hw_blk);

	return 0;
}

int dpu_plane_prepare_fb(struct drm_plane *plane,
		struct drm_plane_state *new_state)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_plane_state *dpu_pstate;
	struct dpu_plane *dpu_plane;
	struct dpu_kms *dpu_kms;
	u32 blk_id;
	u32 pipe;
	int ret;

	if (!plane || !new_state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				plane, new_state);
		return -EINVAL;
	}

	dpu_plane = to_dpu_plane(plane);
	dpu_pstate = to_dpu_plane_state(new_state);
	dpu_drm_dev = to_dpu_drm_dev(plane->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	mutex_lock(&dpu_plane->mutex);
	if (dpu_plane->hw_rch) {
		dpu_plane_rch_hw_blk_release(dpu_kms->res_mgr_ctx, dpu_plane);
		DPU_ERROR("plane:%u hardware block has not been released",
				plane->base.id);
	}

	if (new_state->crtc && !dpu_plane->hw_rch) {
		pipe = drm_crtc_index(new_state->crtc);
		blk_id = dpu_pstate->rch_id;

		ret = dpu_plane_rch_hw_blk_reserve(dpu_kms->res_mgr_ctx,
				dpu_plane, dpu_pstate, pipe, blk_id);
		if (ret != 0) {
			mutex_unlock(&dpu_plane->mutex);
			return -EINVAL;
		}

		dpu_plane_color_hw_blk_reserve(dpu_kms->res_mgr_ctx,
				dpu_plane, dpu_pstate, pipe, blk_id);
	}
	mutex_unlock(&dpu_plane->mutex);

	return 0;
}

void dpu_plane_cleanup_fb(struct drm_plane *plane,
		struct drm_plane_state *old_state)
{
	struct dpu_plane_state *dpu_plane_state;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_plane *dpu_plane;
	struct dpu_kms *dpu_kms;

	if (!plane || !old_state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				plane, old_state);
		return;
	}

	dpu_plane = to_dpu_plane(plane);
	dpu_drm_dev = to_dpu_drm_dev(plane->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	dpu_plane_state = to_dpu_plane_state(plane->state);

	mutex_lock(&dpu_plane->mutex);

	dpu_plane_rch_hw_blk_release(dpu_kms->res_mgr_ctx, dpu_plane);

	dpu_plane_color_hw_blk_release(dpu_kms->res_mgr_ctx, dpu_plane);

	mutex_unlock(&dpu_plane->mutex);
}

static void dpu_plane_res_mgr_lite_update(struct dpu_plane *plane,
		struct dpu_plane_state *old_state, struct dpu_plane_state *new_state)
{
	static struct dpu_plane_state *zops_owner[MAX_PLANE_ZPOS + 1];
	static struct dpu_plane_state *rch_owner[RCH_MAX];
	int i;

	if (!new_state) {
		if (plane->pre_rch_id < RCH_MAX && rch_owner[plane->pre_rch_id] == old_state) {
			rch_owner[plane->pre_rch_id] = NULL;
			plane->pre_rch_id = RCH_NONE;
		}

		if (plane->pre_zpos <= MAX_PLANE_ZPOS && zops_owner[plane->pre_zpos] == old_state) {
			zops_owner[plane->pre_zpos] = NULL;
			plane->pre_zpos = -1;
		}
	} else if (!old_state) {
		plane->pre_rch_id = RCH_NONE;
		/* only single sram rch */
		for (i = 0; i < RCH_MAX; i += 2) {
			if (rch_owner[i])
				continue;

			rch_owner[i] = new_state;
			plane->pre_rch_id = i;
			new_state->rch_id = i;
			new_state->sram_param.total_sram_size = ONE_SRAM_SIZE;
			new_state->sram_param.left_sram_size = ONE_SRAM_SIZE;
			new_state->sram_param.sram_mem_map = 1;
			break;
		}

		plane->pre_zpos = -1;
		for (i = MAX_PLANE_ZPOS; i >= 0; i--) {
			if (zops_owner[i])
				continue;

			zops_owner[i] = new_state;
			plane->pre_zpos = i;
			new_state->base.zpos = i;
			break;
		}
	} else {
		new_state->rch_id = old_state->rch_id;
		new_state->sram_param = old_state->sram_param;
		new_state->base.zpos = old_state->base.zpos;

		if (plane->pre_rch_id < RCH_MAX) {
			if (plane->pre_rch_id == old_state->rch_id) {
				rch_owner[plane->pre_rch_id] = new_state;
			} else {
				rch_owner[plane->pre_rch_id] = NULL;
				plane->pre_rch_id = RCH_NONE;
			}
		}

		if (plane->pre_zpos <= MAX_PLANE_ZPOS) {
			if (plane->pre_zpos == old_state->base.zpos) {
				zops_owner[plane->pre_zpos] = new_state;
			} else {
				zops_owner[plane->pre_zpos] = NULL;
				plane->pre_zpos = -1;
			}
		}
	}
}

static struct drm_plane_state *dpu_plane_atomic_duplicate_state(
		struct drm_plane *plane)
{
	struct dpu_plane_state *new_pstate;
	struct dpu_plane_state *old_pstate;
	struct dpu_plane *dpu_plane;

	if (!plane) {
		DPU_ERROR("invalid parameters, %pK\n", plane);
		return NULL;
	}

	new_pstate = kzalloc(sizeof(*new_pstate), GFP_KERNEL);
	if (!new_pstate)
		return NULL;

	__drm_atomic_helper_plane_duplicate_state(plane,
			&new_pstate->base);

	old_pstate = to_dpu_plane_state(plane->state);
	dpu_plane = to_dpu_plane(plane);

	new_pstate->extender_mode = old_pstate->extender_mode;
	memcpy(&new_pstate->roi_cfg, &old_pstate->roi_cfg, sizeof(struct dpu_plane_roi));
	new_pstate->slice_cnt = old_pstate->slice_cnt;
	dpu_plane_res_mgr_lite_update(dpu_plane, old_pstate, new_pstate);

	if (dpu_plane->color)
		dpu_plane->color->funcs->duplicate_state(
				&new_pstate->base, &old_pstate->base);

	return &new_pstate->base;
}

static void dpu_plane_atomic_destroy_state(struct drm_plane *plane,
		struct drm_plane_state *state)
{
	struct dpu_plane_state *dpu_pstate;
	struct dpu_plane *dpu_plane;

	if (!plane || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				plane, state);
		return;
	}

	dpu_pstate = to_dpu_plane_state(state);
	dpu_plane = to_dpu_plane(plane);

	dpu_plane_res_mgr_lite_update(dpu_plane, dpu_pstate, NULL);

	if (dpu_plane->color)
		dpu_plane->color->funcs->destroy_state(plane, state);

	if (dpu_pstate->fence) {
		dma_fence_put(dpu_pstate->fence);
		dpu_pstate->fence = NULL;
	}

	if (IS_CMDLIST_NODE(dpu_pstate->rch_node_id) &&
			CMDLIST_NODE_UNUSED(dpu_pstate->rch_node_id))
		cmdlist_node_delete(dpu_pstate->rch_node_id);

	if (IS_CMDLIST_NODE(dpu_pstate->color_node_id) &&
			CMDLIST_NODE_UNUSED(dpu_pstate->color_node_id))
		cmdlist_node_delete(dpu_pstate->color_node_id);

	__drm_atomic_helper_plane_destroy_state(&dpu_pstate->base);

	kfree(dpu_pstate);
}

static void dpu_plane_destroy(struct drm_plane *drm_plane)
{
	struct dpu_plane *dpu_plane;

	if (!drm_plane) {
		DPU_ERROR("invalid parameters, %pK\n", drm_plane);
		return;
	}

	dpu_plane = to_dpu_plane(drm_plane);

	dpu_pre_color_deinit(drm_plane);

	drm_plane_cleanup(drm_plane);

	kfree(dpu_plane);
}

static void dpu_plane_reset(struct drm_plane *plane)
{
	struct dpu_plane *dpu_plane;
	struct dpu_plane_state *dpu_pstate;

	if (!plane) {
		DPU_ERROR("invalid parameters, %pK\n", plane);
		return;
	}

	dpu_plane = to_dpu_plane(plane);

	if (plane->state) {
		dpu_plane_atomic_destroy_state(plane, plane->state);
		plane->state = NULL;
	}

	dpu_pstate = kzalloc(sizeof(*dpu_pstate), GFP_KERNEL);
	if (!dpu_plane)
		return;

	__drm_atomic_helper_plane_reset(plane, &dpu_pstate->base);

	dpu_pstate->extender_mode = EXTENDER_MODE_NONE;
	memset(&dpu_pstate->roi_cfg, 0, sizeof(struct dpu_plane_roi));
	dpu_pstate->slice_cnt = 0;
	dpu_plane_res_mgr_lite_update(dpu_plane, NULL, dpu_pstate);
}

static int dpu_plane_set_sram(struct drm_plane *plane,
		struct dpu_plane_state *dpu_pstate,
		void __user *user_ptr)
{
	struct dpu_sram_param sram_param;

	if (!user_ptr) {
		DPU_ERROR("invalid user pointer\n");
		return -EINVAL;
	}

	if (copy_from_user(&sram_param, user_ptr, sizeof(struct dpu_sram_param))) {
		DPU_ERROR("failed to copy dpu_sram_param\n");
		return -EINVAL;
	}

	memcpy(&dpu_pstate->sram_param, &sram_param, sizeof(struct dpu_sram_param));

	LAYER_DEBUG("sram: size: %d, left: %d, right: %d, base: %d, sw: %d\n",
		sram_param.total_sram_size, sram_param.left_sram_size,
		sram_param.right_sram_size, sram_param.sram_base_addr,
		sram_param.sram_mem_map);
	return 0;
}

static int dpu_plane_set_roi(struct drm_plane *plane,
		struct dpu_plane_state *dpu_pstate,
		void __user *user_ptr)
{
	struct dpu_plane_roi plane_roi;

	if (!user_ptr) {
		DPU_ERROR("invalid user pointer\n");
		return -EINVAL;
	}

	if (copy_from_user(&plane_roi, user_ptr, sizeof(struct dpu_plane_roi))) {
		DPU_ERROR("failed to copy dpu_plane_roi\n");
		return -EINVAL;
	}

	if (plane_roi.count > MAX_ROI_COUNT) {
		DPU_ERROR("invalid layer roi count %u\n", plane_roi.count);
		plane_roi.count = 0;
	}

	memcpy(&dpu_pstate->roi_cfg, &plane_roi, sizeof(struct dpu_plane_roi));

	return 0;
}

static int dpu_plane_atomic_set_property(struct drm_plane *plane,
		struct drm_plane_state *state,
		struct drm_property *property,
		u64 val)
{
	struct dpu_plane_state *dpu_pstate;
	struct dpu_plane *dpu_plane;
	int ret = -EINVAL;

	if (!plane || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", plane, state);
		return -EINVAL;
	}

	dpu_plane = to_dpu_plane(plane);
	dpu_pstate = to_dpu_plane_state(state);

	if (property == dpu_plane->acquire_fence_fd_prop) {
		if (dpu_pstate->fence) {
			DPU_ERROR("acquire fence has already set\n");
			return -EINVAL;
		}

		if (U642I64(val) < 0)
			return 0;

		dpu_pstate->fence = sync_file_get_fence(val);
		if (!dpu_pstate->fence)
			return -EINVAL;
	} else if (property == dpu_plane->rch_id_prop) {
		dpu_pstate->rch_id = val;
	} else if (property == dpu_plane->extender_mode_prop) {
		dpu_pstate->extender_mode = val;
	} else if (property == dpu_plane->sram_prop) {
		ret = dpu_plane_set_sram(plane, dpu_pstate,
				(void __user *)(uintptr_t)val);
		if (ret) {
			DPU_ERROR("failed to set plane sram prop, ret %d\n", ret);
			return ret;
		}
	} else if (property == dpu_plane->roi_prop) {
		ret = dpu_plane_set_roi(plane, dpu_pstate,
				(void __user *)(uintptr_t)val);
		if (ret) {
			DPU_ERROR("failed to set plane roi, ret %d\n", ret);
			return ret;
		}
	} else if (property == dpu_plane->wfd_sink_slice_prop) {
		dpu_pstate->slice_cnt = val > MAX_WFD_SLICE_CNT ? 0 : val;
	} else if (property == dpu_plane->axi_prop) {
		dpu_pstate->axi_id = val;
	} else if (property == dpu_plane->protected_buffer_status_prop) {
		dpu_pstate->is_protected_buffer = val;
	} else if (property == dpu_plane->sharefd_prop) {
		if (U642I64(val) < 0)
			return 0;
		dpu_pstate->sharefd = val;
	} else {
		if (dpu_plane->color)
			ret = dpu_plane->color->funcs->set_property(plane,
					state, property, val);
		if (ret) {
			DPU_ERROR("unknown property %s\n", property->name);
			return -EINVAL;
		}
	}

	DPU_PROPERTY_DEBUG("update property %s: %lld\n", property->name, val);
	return 0;
}

static int dpu_plane_atomic_get_property(struct drm_plane *plane,
		const struct drm_plane_state *state,
		struct drm_property *property,
		u64 *val)
{
	struct dpu_plane_state *dpu_pstate;
	struct dpu_plane *dpu_plane;
	int ret = -EINVAL;

	if (!plane || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", plane, state);
		return -EINVAL;
	}

	dpu_plane = to_dpu_plane(plane);
	dpu_pstate = to_dpu_plane_state(state);

	if (property == dpu_plane->acquire_fence_fd_prop) {
		*val = 0;
	} else if (property == dpu_plane->rch_id_prop) {
		*val = dpu_pstate->rch_id;
	} else if (property == dpu_plane->extender_mode_prop) {
		*val = dpu_pstate->extender_mode;
	} else if (property == dpu_plane->sram_prop) {
		*val = 0;
	} else if (property == dpu_plane->roi_prop) {
		*val = 0;
	} else if (property == dpu_plane->wfd_sink_slice_prop) {
		*val = dpu_pstate->slice_cnt;
	} else if (property == dpu_plane->protected_buffer_status_prop) {
		*val = dpu_pstate->is_protected_buffer;
	} else if (property == dpu_plane->axi_prop) {
		*val = 0;
	} else if (property == dpu_plane->sharefd_prop) {
		*val = 0;
	} else {
		if (dpu_plane->color)
			ret = dpu_plane->color->funcs->get_property(plane,
					state, property, val);

		if (ret) {
			DPU_ERROR("unknown property %s\n", property->name);
			return -EINVAL;
		}
	}

	DPU_PROPERTY_DEBUG("get property %s: %lld\n", property->name, *val);
	return 0;
}

static struct drm_property *dpu_plane_create_enum_property(struct drm_plane *plane,
		char *name, const struct drm_prop_enum_list *props, int num_values)
{
	struct drm_property *prop;

	prop = drm_property_create_enum(plane->dev, 0,
			name, props, num_values);
	if (!prop)
		return NULL;

	drm_object_attach_property(&plane->base, prop, 0);
	return prop;
}

static struct drm_property *dpu_plane_create_range_property(struct drm_plane *plane,
		char *name, uint64_t min, uint64_t max)
{
	struct drm_property *prop;

	prop = drm_property_create_range(plane->dev, 0,
			name, min, max);
	if (!prop)
		return NULL;

	drm_object_attach_property(&plane->base, prop, 0);
	return prop;
}

static struct drm_property *dpu_plane_create_signed_range_property(struct drm_plane *plane,
		char *name, int64_t min, int64_t max)
{
	struct drm_property *prop;

	prop = drm_property_create_signed_range(plane->dev, 0,
			name, min, max);
	if (!prop)
		return NULL;

	drm_object_attach_property(&plane->base, prop, 0);
	return prop;
}

static bool dpu_plane_create_sram_property(struct drm_plane *plane,
		struct dpu_plane *dpu_plane)
{
	struct drm_property *property;

	property = dpu_plane_create_range_property(plane,
			"sram", 0, U64_MAX);
	if (!property) {
		DPU_ERROR("sram property create sram failed\n");
		return -ENOMEM;
	}

	dpu_plane->sram_prop = property;

	return true;
}

static int dpu_plane_create_properties(struct drm_plane *plane)
{
	struct dpu_plane *dpu_plane;
	unsigned int blend_modes = BIT(DRM_MODE_BLEND_PIXEL_NONE)
			| BIT(DRM_MODE_BLEND_PREMULTI)
			| BIT(DRM_MODE_BLEND_COVERAGE);
	const struct drm_prop_enum_list rch_id_list[] = {
		{ RCH_NONE, "rch_none"},
		{ RCH_G_0, "rch_g_0" },
		{ RCH_V_0, "rch_v_0" },
		{ RCH_G_1, "rch_g_1" },
		{ RCH_V_1, "rch_v_1" },
		{ RCH_G_2, "rch_g_2" },
		{ RCH_G_3, "rch_g_3" },
		{ RCH_G_4, "rch_g_4" },
		{ RCH_V_2, "rch_v_2" },
		{ RCH_V_3, "rch_v_3" },
		{ RCH_G_5, "rch_g_5" },
	};
	const struct drm_prop_enum_list extender_mode_list[] = {
		{ EXTENDER_MODE_NONE, "single_extender"},
		{ EXTENDER_MODE_V, "v_extender"},
		{ EXTENDER_MODE_LEFT, "left_extender"},
		{ EXTENDER_MODE_RIGHT, "right_extender"},
	};

	const struct drm_prop_enum_list axi_id_list[] = {
		{ RDMA_AXI_0, "axi_0"},
		{ RDMA_AXI_1, "axi_1"},
		{ RDMA_AXI_2, "axi_2"},
		{ RDMA_AXI_DEFAULT, "axi_default"},
	};

	if (!plane) {
		DPU_ERROR("invalid parameters, %pK\n", plane);
		return -EINVAL;
	}

	dpu_plane = to_dpu_plane(plane);

	if (dpu_plane->color)
		dpu_plane->color->funcs->create_properties(plane);

	/* create rotation property */
	drm_plane_create_rotation_property(plane,
			DRM_MODE_ROTATE_0,
			DRM_MODE_ROTATE_MASK | DRM_MODE_REFLECT_MASK);

	/* create zpos property */
	drm_plane_create_zpos_property(plane, 0, 0, MAX_PLANE_ZPOS);

	/* create layer alpha property */
	drm_plane_create_alpha_property(plane);

	/* create blend mode property */
	drm_plane_create_blend_mode_property(plane, blend_modes);

	dpu_plane_create_sec_drm_properties(plane);

	dpu_plane->rch_id_prop = dpu_plane_create_enum_property(plane,
			"rch_id", rch_id_list, ARRAY_SIZE(rch_id_list));
	if (!dpu_plane->rch_id_prop)
		return -ENOMEM;

	dpu_plane->extender_mode_prop = dpu_plane_create_enum_property(plane,
			"extender_mode", extender_mode_list, ARRAY_SIZE(extender_mode_list));
	if (!dpu_plane->extender_mode_prop)
		return -ENOMEM;

	dpu_plane->acquire_fence_fd_prop = dpu_plane_create_signed_range_property(plane,
			"acquire_fence_fd", -1, INT_MAX);
	if (!dpu_plane->acquire_fence_fd_prop)
		return -ENOMEM;

	dpu_plane->sharefd_prop = dpu_plane_create_signed_range_property(plane,
			"sharefd", -1, INT_MAX);
	if (!dpu_plane->sharefd_prop)
		return -ENOMEM;

	if (!dpu_plane_create_sram_property(plane, dpu_plane)) {
		DPU_ERROR("sram property create failed\n");
		return -ENOMEM;
	}

	dpu_plane->axi_prop = dpu_plane_create_enum_property(plane,
			"axi_id", axi_id_list, ARRAY_SIZE(axi_id_list));
	if (!dpu_plane->axi_prop)
		return -ENOMEM;

	dpu_plane->roi_prop = dpu_plane_create_range_property(plane,
			"roi", 0, U64_MAX);
	if (!dpu_plane->roi_prop)
		return -ENOMEM;

	dpu_plane->wfd_sink_slice_prop = dpu_plane_create_range_property(plane,
			"wfd_sink_slice_num", 0, MAX_WFD_SLICE_CNT);
	if (!dpu_plane->wfd_sink_slice_prop)
		return -ENOMEM;

	return 0;
}

bool dpu_plane_format_mod_supported(struct drm_plane *plane,
	uint32_t format, uint64_t modifier)
{
	return dpu_format_is_afbc_supported(FMT_MODULE_RCH, format, modifier);
}

static const struct drm_plane_helper_funcs dpu_plane_helper_funcs = {
	.atomic_check = dpu_plane_atomic_check,
	.atomic_update = dpu_plane_atomic_update,
	.atomic_disable = dpu_plane_atomic_disable,
	.prepare_fb = dpu_plane_prepare_fb,
	.cleanup_fb = dpu_plane_cleanup_fb,
};

static const struct drm_plane_funcs dpu_plane_funcs = {
	.update_plane = drm_atomic_helper_update_plane,
	.disable_plane = drm_atomic_helper_disable_plane,
	.destroy = dpu_plane_destroy,
	.reset = dpu_plane_reset,
	.atomic_duplicate_state = dpu_plane_atomic_duplicate_state,
	.atomic_destroy_state = dpu_plane_atomic_destroy_state,
	.atomic_set_property = dpu_plane_atomic_set_property,
	.atomic_get_property = dpu_plane_atomic_get_property,
	.format_mod_supported = dpu_plane_format_mod_supported,
};

struct drm_plane *dpu_plane_init(struct drm_device *drm_dev,
		u32 possible_crtcs, enum drm_plane_type plane_type)
{
	struct dpu_plane *dpu_plane;
	u32 *plane_formats;
	u32 format_count;
	int ret;

	dpu_plane = kzalloc(sizeof(*dpu_plane), GFP_KERNEL);
	if (!dpu_plane)
		return ERR_PTR(-ENOMEM);

	format_count = dpu_format_all_supported_get(FMT_MODULE_RCH,
			&plane_formats);
	if (format_count <= 0) {
		ret = format_count;
		DPU_ERROR("failed to get plane formats, %d\n", ret);
		goto error;
	}

	ret = drm_universal_plane_init(drm_dev, &dpu_plane->base,
			possible_crtcs, &dpu_plane_funcs,
			plane_formats, format_count,
			plane_format_modifiers, plane_type, NULL);
	if (ret) {
		DPU_ERROR("failed to init plane, ret %d\n", ret);
		goto error_init;
	}

	drm_plane_helper_add(&dpu_plane->base,
			&dpu_plane_helper_funcs);

	ret = dpu_pre_color_init(&dpu_plane->base);
	if (ret) {
		DPU_ERROR("failed to init plane color, ret %d\n", ret);
		goto error_init;
	}

	ret = dpu_plane_create_properties(&dpu_plane->base);
	if (ret) {
		DPU_ERROR("failed to create plane prop, ret %d\n", ret);
		goto error_init;
	}

	mutex_init(&dpu_plane->mutex);

	kfree(plane_formats);

	return &dpu_plane->base;

error_init:
	kfree(plane_formats);
error:
	kfree(dpu_plane);
	return ERR_PTR(ret);
}
