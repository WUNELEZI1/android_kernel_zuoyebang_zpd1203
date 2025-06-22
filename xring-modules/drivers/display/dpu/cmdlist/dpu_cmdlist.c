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

#include <linux/platform_device.h>
#include <linux/of.h>
#include "dpu_cmdlist_frame_mgr.h"
#include "dpu_hw_cmdlist_ops.h"
#include "dpu_cmdlist_node.h"
#include "dpu_cmdlist_common.h"
#include "dpu_log.h"
#include "dpu_cmdlist.h"
#include "dpu_crtc.h"
#include "dpu_kms.h"
#include "dpu_wfd.h"
#include "dpu_wb.h"
#include "dsi_connector.h"
#include "dpu_plane.h"
#include "dpu_hw_reg_map.h"
#include "dpu_hw_ulps.h"
#include "dpu_power_helper.h"

#define NODE_SPLIT_NUM 16

static inline struct dpu_wb_connector *to_wb_connector(struct drm_connector *drm_conn)
{
	return to_dpu_wb_connector(drm_connector_to_writeback(drm_conn));
}

static inline void cmdlist_node_prepare(struct dpu_hw_blk *hw, s64 *node_id,
		u16 *node_len, u16 reg_num)
{
	hw->cmd_node = node_id;
	*node_len += ALIGN(reg_num, NODE_ITEM_ALIGN) / NODE_ITEM_ALIGN;
}

static void dpu_hw_prepq_blks_prepare(struct plane_color *pq, u16 *node_len)
{
	if (!pq)
		return;

	if (pq->hw_prepipe_top)
		cmdlist_node_prepare(&pq->hw_prepipe_top->hw, NULL, node_len, 50);
	if (pq->hw_scaler)
		cmdlist_node_prepare(&pq->hw_scaler->hw, NULL, node_len, 508);
	if (pq->hw_tm)
		cmdlist_node_prepare(&pq->hw_tm->hw, NULL, node_len, 350);
	if (pq->hw_hist)
		cmdlist_node_prepare(&pq->hw_hist->hw, NULL, node_len, 50);
	if (pq->hw_3dlut)
		cmdlist_node_prepare(&pq->hw_3dlut->hw, NULL, node_len, 7800);
}

static void dpu_hw_rch_blks_prepare(struct dpu_plane *dpu_plane,
		s64 rch_node_id[RCH_MAX], u8 rch_layer_num[RCH_MAX])
{
	struct dpu_plane_state *pstate;
	u16 color_node_len = 0;
	u16 node_len = 0;
	u64 wait_event;
	u32 rch_id;

	pstate = to_dpu_plane_state(dpu_plane->base.state);
	rch_id = dpu_plane->hw_rch->hw.blk_id;

	if (pstate->extender_mode > EXTENDER_MODE_V && IS_CMDLIST_NODE(rch_node_id[rch_id])) {
		pstate->rch_node_id = rch_node_id[rch_id];
		return;
	}

	rch_layer_num[rch_id]++;
	cmdlist_node_prepare(&dpu_plane->hw_rch->hw, NULL, &node_len, 64);
	dpu_hw_prepq_blks_prepare(dpu_plane->color, &color_node_len);

	if (rch_layer_num[rch_id] >= NODE_SPLIT_NUM) {
		wait_event = RDMA_RELOAD;
		pstate->color_node_id = cmdlist_node_create(rch_id, NORMAL_TYPE,
				color_node_len, PREPQ_RELOAD);
	} else {
		wait_event = RDMA_RELOAD | PREPQ_RELOAD;
		node_len += color_node_len;
	}

	pstate->rch_node_id = cmdlist_node_create(rch_id, NORMAL_TYPE, node_len, wait_event);
	rch_node_id[rch_id] = pstate->rch_node_id;
}

static void dpu_hw_postpq_blks_prepare(struct color_postpq *pq, s64 *node_id,
		u16 *node_len)
{
	if (!pq)
		return;

	if (pq->hw_gamma)
		cmdlist_node_prepare(&pq->hw_gamma->hw, node_id, node_len, 610);
	if (pq->hw_3dlut)
		cmdlist_node_prepare(&pq->hw_3dlut->hw, node_id, node_len, 7800);
	if (pq->hw_pq_top)
		cmdlist_node_prepare(&pq->hw_pq_top->hw, node_id, node_len, 50);
	if (pq->hw_pre_proc)
		cmdlist_node_prepare(&pq->hw_pre_proc->hw, node_id, node_len, 20);
	if (pq->hw_acad)
		cmdlist_node_prepare(&pq->hw_acad->hw, node_id, node_len, 151);
	if (pq->hw_hist)
		cmdlist_node_prepare(&pq->hw_hist->hw, node_id, node_len, 32);
	if (pq->hw_scaler)
		cmdlist_node_prepare(&pq->hw_scaler->hw, node_id, node_len, 322);
}

static u32 dpu_cmp_node_wait_event_get(struct dpu_crtc *dpu_crtc,
		struct drm_connector *wb_connector)
{
	struct dpu_wb_connector_state *dpu_wb_conn_state;
	struct dpu_power_ctrl *power_ctrl;
	u32 wait_event = CMPS_RELOAD;

	if (wb_connector) {
		dpu_wb_conn_state = to_dpu_wb_connector_state(wb_connector->state);
		if (dpu_wb_conn_state) {
			if (dpu_wb_conn_state->wb_stripe_count > 1) {
				wait_event |= WB1_RELOAD;
				DPU_CMDLIST_DEBUG("create node wait WB1_reload\n");
			}
		}
	}

	power_ctrl = get_power_ctrl_inst(&dpu_crtc->base);
	if (!power_ctrl ||
			!(power_ctrl->lowpower_ctrl & DPU_LP_HW_ULPS) ||
			!is_dpu_lp_enabled(DPU_LP_HW_ULPS))
		return wait_event;

	wait_event |= TMG_SOF | PRI_NO_BUSY;
	DPU_CMDLIST_DEBUG("add TMG_SOF | PRI_NO_BUSY\n");

	return wait_event;
}

static void dpu_hw_mixer_blks_prepare(struct dpu_crtc *dpu_crtc,
		struct drm_connector *wb_connector)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_wb_connector *dpu_wb;
	u16 node_len = 0;

	dpu_crtc_state = to_dpu_crtc_state(dpu_crtc->base.state);

	cmdlist_node_prepare(&dpu_crtc->hw_mixer->hw, &dpu_crtc_state->node_id,
			&node_len, 145);

	if (dpu_crtc->hw_post_pipe_top)
		cmdlist_node_prepare(&dpu_crtc->hw_post_pipe_top->hw,
				&dpu_crtc_state->node_id, &node_len, 2552);

	if (dpu_crtc->hw_dsc)
		cmdlist_node_prepare(&dpu_crtc->hw_dsc->hw,
				&dpu_crtc_state->node_id, &node_len, 66);

	if (wb_connector) {
		dpu_wb = to_wb_connector(wb_connector);
		if (dpu_wb->hw_wb)
			cmdlist_node_prepare(&dpu_wb->hw_wb->hw,
					&dpu_crtc_state->node_id, &node_len, 322);
		if (dpu_wb->hw_wb_scaler)
			cmdlist_node_prepare(&dpu_wb->hw_wb_scaler->hw,
					&dpu_crtc_state->node_id, &node_len, 126);
	}

	dpu_hw_postpq_blks_prepare(dpu_crtc->postpq, &dpu_crtc_state->node_id, &node_len);

	dpu_crtc_state->node_id = cmdlist_node_create(RCH_MAX + dpu_crtc->hw_mixer->hw.blk_id,
			NORMAL_TYPE, node_len, dpu_cmp_node_wait_event_get(dpu_crtc, wb_connector));
}

static void dpu_connector_blks_prepare(struct drm_connector *connector,
		int cmps_id)
{
	struct dsi_connector_state *dsi_conn_state;
	struct dsi_connector *dsi_conn;
	struct dpu_hw_blk *hw;
	struct dsi_ctrl *ctrl;
	struct dsi_tmg *tmg;
	u16 node_len = 0;
	u32 i;

	dsi_conn = to_dsi_connector(connector);
	dsi_conn_state = to_dsi_connector_state(connector->state);

	display_for_each_active_port(i, dsi_conn->display) {
		ctrl = dsi_conn->display->port[i].ctrl;
		hw = ctrl->ctrl_hw.ctrl_blks.dctrl_blk;
		cmdlist_node_prepare(hw, &dsi_conn_state->node_id,
				&node_len, hw->blk_len);
		hw = ctrl->ctrl_hw.ctrl_blks.dsctrl_blk;
		cmdlist_node_prepare(hw, &dsi_conn_state->node_id,
				&node_len, hw->blk_len);

		tmg = dsi_conn->display->port[i].tmg;
		cmdlist_node_prepare(&tmg->tmg_hw.hw_blk,
				&dsi_conn_state->node_id, &node_len, hw->blk_len);
		/**
		 * The same register will be written repeatedly when using CMDList
		 * to send CMD Set, and each register write operation will be saved
		 * in a separate CMDList Node, so it is necessary to expand the node
		 * size. To send a single cmd the number of register writes required is:
		 * 1. write the CTRL_0X2D0_RI_TX_CTRL register once;
		 * 2. write N times CTRL_0X2C4_CRI_TX_PLD to send payload_size bytes
		 * payload data, N = ⌈ payload_size/ 4 ⌉;
		 * 3. write CTRL_0X2C0_CRI_TX_HDR once, send header;
		 * 4. Write the CTRL_0X2D0_RI_TX_CTRL register once;
		 *
		 * The hardware can send DSI_HDR_MEM_MAX cmd's at a time, and the maximum
		 * total payload_size is DSI_PLD_MEM_MAX. Therefore the required node_len is:
		 * node_len = (3 * DSI_HDR_MEM_MAX) + DSI_PLD_MEM_MAX/ 4 + DSI_HDR_MEM_MAX
		 */
		node_len += (4 * DSI_HDR_MEM_MAX + DSI_PLD_MEM_MAX / 4);
	}
	dsi_conn_state->node_id = cmdlist_node_create(RCH_MAX + cmps_id,
			NORMAL_TYPE, node_len, 0);
}

void dpu_hw_cmdlist_enable(struct dpu_kms *dpu_kms, u32 channel_mask, bool enable)
{
	struct cmdlist_base *base;
	u32 i;

	if (!channel_mask)
		return;

	base = get_cmdlist_instance();
	for (i = 0; i < base->channel_count; i++) {
		if (BIT(i) & channel_mask)
			cmdlist_config_enable(i, enable);
	}
}

static void dpu_cmdlist_enable(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_hw_mixer *hw_mixer;
	struct dpu_crtc *dpu_crtc;
	u32 new_rch_mask, old_rch_mask;
	u32 channel_mask;

	/* step1, do cmdlist rch channel configure */
	new_rch_mask = _get_rch_mask(state, new_state->plane_mask, true);
	old_rch_mask = _get_rch_mask(state, old_state->plane_mask, false);

	dpu_hw_cmdlist_enable(dpu_kms, new_rch_mask, true);

	channel_mask = old_rch_mask & ~new_rch_mask;
	dpu_hw_cmdlist_enable(dpu_kms, channel_mask, false);

	/* step2, do cmdlist mixer channel configure */
	dpu_crtc = to_dpu_crtc(new_state->crtc);
	hw_mixer = dpu_crtc->hw_mixer;
	channel_mask = BIT(hw_mixer->hw.blk_id + RCH_MAX);

	dpu_hw_cmdlist_enable(dpu_kms, channel_mask, true);

	if (need_disable_crtc(new_state))
		dpu_hw_cmdlist_enable(dpu_kms, channel_mask, false);
}

static inline u32 sort_plane(struct drm_device *dev, u32 plane_mask,
		struct dpu_plane *sorted_dpu_plane[MAX_PLANE_COUNT])
{
	struct dpu_plane *dpu_plane;
	struct drm_plane *plane;
	u32 valid_plane_count = 0;
	u8 i = 0;
	u8 j = 0;
	u32 m = 0;
	u32 n = 0;

	drm_for_each_plane_mask(plane, dev, plane_mask) {
		dpu_plane = to_dpu_plane(plane);
		if (!dpu_plane->hw_rch)
			continue;

		sorted_dpu_plane[valid_plane_count++] = dpu_plane;
	}

	/* sort plane order by dpu_plane_state.base.crtc_y */
	for (i = 0; i < valid_plane_count; i++) {
		for (j = i + 1; j < valid_plane_count; j++) {
			m = sorted_dpu_plane[i]->base.state->crtc_y;
			n = sorted_dpu_plane[j]->base.state->crtc_y;
			if (n < m)
				swap(sorted_dpu_plane[i], sorted_dpu_plane[j]);
		}
	}

	return valid_plane_count;
}

void dpu_cmdlist_prepare(struct drm_crtc *crtc, struct drm_crtc_state *crtc_state)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct drm_connector *connector;
	struct dpu_plane *dpu_plane;
	struct dpu_crtc *dpu_crtc;
	struct dpu_kms *dpu_kms;
	struct drm_plane *plane;
	s64 rch_node_id[RCH_MAX];
	u8 rch_layer_num[RCH_MAX];
	u32 mask;

	dpu_kms = to_dpu_drm_dev(crtc->dev)->dpu_kms;
	dpu_crtc = to_dpu_crtc(crtc);
	dpu_crtc_state = to_dpu_crtc_state(crtc_state);
	memset(rch_node_id, 0xff, sizeof(rch_node_id[0]) * RCH_MAX);
	memset(rch_layer_num, 0, sizeof(rch_layer_num[0]) * RCH_MAX);

	mask = crtc_state->plane_mask;
	drm_for_each_plane_mask(plane, dpu_kms->drm_dev, mask) {
		dpu_plane = to_dpu_plane(plane);
		if (!dpu_plane->hw_rch)
			continue;

		dpu_hw_rch_blks_prepare(dpu_plane, rch_node_id, rch_layer_num);
	}

	mask = crtc_state->connector_mask;
	connector = find_connector(dpu_kms->drm_dev, mask, DRM_MODE_CONNECTOR_WRITEBACK);
	dpu_hw_mixer_blks_prepare(dpu_crtc, connector);

	connector = find_connector(dpu_kms->drm_dev, mask, DRM_MODE_CONNECTOR_DSI);
	if (connector)
		dpu_connector_blks_prepare(connector,
				dpu_crtc->hw_mixer->hw.blk_id);
}

void dpu_cmdlist_commit(struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_plane *sorted_dpu_plane[MAX_PLANE_COUNT];
	struct dpu_plane_state *dpu_plane_state;
	struct dpu_crtc_state *dpu_crtc_state;
	struct dsi_connector_state *dsi_conn_state;
	struct dpu_wb_connector *dpu_wb_conn;
	struct dpu_virt_pipeline *pipeline;
	struct drm_connector *connector;
	struct dpu_plane *dpu_plane;
	struct dpu_crtc *dpu_crtc;
	struct dpu_hw_wb *hw_wb;
	struct dpu_kms *dpu_kms;
	u32 valid_plane_count;
	u8 extender_mode;
	u32 cmps_id;
	u32 cfg_rdy;
	u32 rch_id;
	u32 mask;
	u32 i;

	DPU_CMDLIST_DEBUG("Enter\n");

	dpu_kms = to_dpu_drm_dev(crtc->dev)->dpu_kms;
	dpu_crtc_state = to_dpu_crtc_state(new_state);
	pipeline = &dpu_kms->virt_pipeline[drm_crtc_index(crtc)];

	dpu_crtc = to_dpu_crtc(crtc);
	cmps_id = dpu_crtc->hw_mixer->hw.blk_id;

	if (pipeline->actived_cmdlist_frame_id)
		cmdlist_frame_destroy(pipeline->actived_cmdlist_frame_id);
	pipeline->actived_cmdlist_frame_id = pipeline->cmdlist_frame_id;
	pipeline->cmdlist_frame_id = cmdlist_frame_create();

	dpu_cmdlist_enable(dpu_kms, state, old_state, new_state);

	dpu_hw_ulps_disable_with_cmdlist(new_state);

	valid_plane_count = sort_plane(dpu_kms->drm_dev, new_state->plane_mask,
			sorted_dpu_plane);

	for (i = 0; i < valid_plane_count; i++) {
		dpu_plane = sorted_dpu_plane[i];
		dpu_plane_state = to_dpu_plane_state(dpu_plane->base.state);
		extender_mode = dpu_plane_state->extender_mode;

		cmdlist_layer_y_pos_set(dpu_plane_state->rch_node_id, dpu_plane_state->base.crtc_y);

		if (extender_mode == EXTENDER_MODE_RIGHT) {
			CMDLIST_NODE_MARK_USED(dpu_plane_state->rch_node_id);
			continue;
		}

		if (!dpu_plane->hw_rch) {
			DPU_WARN("hw_rch is null\n");
			return;
		}
		rch_id = dpu_plane->hw_rch->hw.blk_id;
		cfg_rdy = CMDLIST_RDMA_CFG_RDY(rch_id);
		if (!dpu_plane_state->color_node_id)
			cfg_rdy |= CMDLIST_PREPQ_CFG_RDY(rch_id);

		cmdlist_cfg_rdy_set(dpu_plane_state->rch_node_id, cfg_rdy);

		cmdlist_frame_node_append(pipeline->cmdlist_frame_id,
				&dpu_plane_state->rch_node_id, DELETE_WITH_FRAME);
		if (dpu_plane_state->color_node_id) {
			cmdlist_cfg_rdy_set(dpu_plane_state->color_node_id,
					CMDLIST_PREPQ_CFG_RDY(rch_id));
			cmdlist_frame_node_append(pipeline->cmdlist_frame_id,
					&dpu_plane_state->color_node_id, DELETE_WITH_FRAME);
		}


		if (dpu_wfd_sink_cmdlist_append(&dpu_plane->base,
				pipeline->cmdlist_frame_id)) {
			cmdlist_wait_event_update(dpu_plane_state->rch_node_id, 0);
			cmdlist_cfg_rdy_set(dpu_plane_state->rch_node_id, 0);
		}

		dpu_plane->hw_rch->hw.cmd_node = NULL;
	}

	mask = new_state->connector_mask;
	cfg_rdy = CMDLIST_CMPS_CFG_RDY(cmps_id);
	connector = find_connector(dpu_kms->drm_dev, mask, DRM_MODE_CONNECTOR_WRITEBACK);
	if (connector) {
		dpu_wb_conn = to_wb_connector(connector);
		hw_wb = dpu_wb_conn->hw_wb;
		if (hw_wb)
			cfg_rdy |= CMDLIST_WB_CFG_RDY(hw_wb->hw.blk_id);
	}

	connector = find_connector(dpu_kms->drm_dev, mask, DRM_MODE_CONNECTOR_DSI);
	if (connector) {
		dsi_conn_state = to_dsi_connector_state(connector->state);
		cmdlist_frame_node_append(pipeline->cmdlist_frame_id,
			&dsi_conn_state->node_id, DELETE_WITH_FRAME);
	}

	cmdlist_cfg_rdy_set(dpu_crtc_state->node_id, cfg_rdy);

	cmdlist_frame_node_append(pipeline->cmdlist_frame_id, &dpu_crtc_state->node_id,
			DELETE_WITH_FRAME);

	dpu_crtc->hw_mixer->hw.cmd_node = NULL;

	dpu_hw_ulps_enable_with_cmdlist(new_state);

	cmdlist_frame_commit(pipeline->cmdlist_frame_id);
	DPU_CMDLIST_DEBUG("Exit\n");
}

void dpu_cmdlist_plane_node_update(struct dpu_plane *dpu_plane)
{
	struct dpu_plane_state *pstate;
	struct plane_color *color;
	s64 *color_node;

	pstate = to_dpu_plane_state(dpu_plane->base.state);
	dpu_plane->hw_rch->hw.cmd_node = &pstate->rch_node_id;

	color = dpu_plane->color;
	if (!color)
		return;

	color_node = pstate->color_node_id ? &pstate->color_node_id : &pstate->rch_node_id;
	if (color->hw_prepipe_top)
		color->hw_prepipe_top->hw.cmd_node = color_node;

	if (color->hw_scaler)
		color->hw_scaler->hw.cmd_node = color_node;

	if (color->hw_tm)
		color->hw_tm->hw.cmd_node = color_node;

	if (color->hw_hist)
		color->hw_hist->hw.cmd_node = color_node;

	if (color->hw_3dlut)
		color->hw_3dlut->hw.cmd_node = color_node;
}

static int dpu_cmdlist_parse_dts(struct device *dev)
{
	struct cmdlist_base *cmdlist;
	int ret;

	cmdlist = get_cmdlist_instance();
	ret = of_property_read_u32(dev->of_node, "cmdlist-offset",
			&cmdlist->cmdlist_offset);
	if (ret) {
		DPU_ERROR("failed to parse cmdlist-offset");
		return ret;
	}

	ret = of_property_read_u32(dev->of_node, "cmdlist-channel-count",
			&cmdlist->channel_count);
	if (ret) {
		DPU_ERROR("failed to parse cmdlist-channel-count");
		return ret;
	}

	ret = of_property_read_u32(dev->of_node, "cmdlist-mem-align",
			&cmdlist->mem_align);
	if (ret) {
		DPU_ERROR("failed to parse cmdlist-mem-align");
		return ret;
	}

	ret = of_property_read_u32(dev->of_node, "ctl-top-offset",
			&cmdlist->ctl_top_offset);
	if (ret) {
		DPU_ERROR("failed to parse ctl-top-offset");
		return ret;
	}

	DPU_CMDLIST_DEBUG("dpu_base = %pK\n", cmdlist->dpu_base);
	DPU_CMDLIST_DEBUG("cmdlist_offset = 0x%x\n", cmdlist->cmdlist_offset);
	DPU_CMDLIST_DEBUG("cmdlist-channel = %d\n", cmdlist->channel_count);
	DPU_CMDLIST_DEBUG("cmdlist-mem-align = %d\n", cmdlist->mem_align);
	DPU_CMDLIST_DEBUG("ctl_top_offset = 0x%x\n", cmdlist->ctl_top_offset);

	return 0;
}

int dpu_cmdlist_init(struct drm_device *drm_dev, struct dksm_mem_pool *mem_pool)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct platform_device *pdev;
	struct cmdlist_base *cmdlist;
	struct dpu_kms *dpu_kms;
	int ret;

	if (!drm_dev || !mem_pool) {
		DPU_ERROR("invalid params: %pK %pK", drm_dev, mem_pool);
		return -EPERM;
	}

	cmdlist = get_cmdlist_instance();
	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	pdev = to_platform_device(drm_dev->dev);

	cmdlist->node_pool = mem_pool;
	cmdlist->dpu_base = dpu_kms->res_mgr_ctx->dpu_cap.dpu_mem.base;
	cmdlist->dpu_phy_addr = dpu_kms->res_mgr_ctx->dpu_cap.dpu_mem.addr;
	ret = dpu_cmdlist_parse_dts(drm_dev->dev);
	if (ret)
		goto error;

	ret = cmdlist_frame_init();
	if (ret)
		goto error;

	ret = cmdlist_channel_init();
	if (ret)
		goto channel_init_error;

	cmdlist->initialized = true;

	return 0;

channel_init_error:
	cmdlist_frame_deinit();
error:
	return ret;
}

void dpu_cmdlist_deinit(void)
{
	struct cmdlist_base *cmdlist;

	cmdlist = get_cmdlist_instance();
	if (!cmdlist->initialized)
		return;

	cmdlist_channel_deinit();
	cmdlist_frame_deinit();
}
