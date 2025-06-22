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

#include <linux/kernel.h>

#include "dpu_idle_helper.h"
#include "dpu_power_helper.h"
#include "dpu_intr_core.h"
#include "dpu_idle_utils.h"
#include "dsi_encoder.h"
#include "dpu_idle_utils.h"
#include "dpu_exception.h"
#include "dpu_hw_power_ops.h"
#include "dpu_hw_dvfs.h"
#include "dpu_hw_ctl_ops.h"
#include "dpu_cmdlist.h"
#include "dpu_crtc.h"
#include "dpu_trace.h"

#define BASIC_POLICY (IDLE_ISR_OFF | IDLE_MIPI_ULPS)

static u32 g_avail_idle_policy[] = {
	/* 0x6 */
	BASIC_POLICY,
	/* 0x386 */
	BASIC_POLICY | IDLE_P1_POWER_OFF | IDLE_P2_POWER_OFF | IDLE_P0_POWER_OFF,
};

bool dpu_cmd_idle_check_idle_policy(u32 idle_policy)
{
	u32 i;
	bool found = false;

	for (i = 0; i < ARRAY_SIZE(g_avail_idle_policy); i++) {
		if (idle_policy == g_avail_idle_policy[i]) {
			found = true;
			break;
		}
	}
	if (!found && (idle_policy != IDLE_NONE))
		IDLE_ERROR("invalid policy:0x%x\n", idle_policy);
	return found;
}

u32 dpu_cmd_idle_get_basic_policy(void)
{
	return BASIC_POLICY;
}

int dpu_cmd_idle_init(struct dpu_idle_ctx *idle_ctx)
{
	return 0;
}

static bool need_cfg_work(u32 idle_policy)
{
	if ((idle_policy & IDLE_DSI_CLK_OFF) || (idle_policy & IDLE_P1_POWER_OFF) ||
		(idle_policy & IDLE_P2_POWER_OFF) || (idle_policy & IDLE_P0_POWER_OFF))
		return true;
	return false;
}

/* HW Constraints:
 * Need to disable tmg firstly if wants to close dsi clk in idle.
 */
static bool need_disable_tmg_first(u32 idle_policy)
{
	return (idle_policy & IDLE_DSI_CLK_OFF) ||
			(idle_policy & IDLE_P0_POWER_OFF);
}

/* HW Constraints:
 * Need unhook rch and wb from scene ctl before P1/P2 power off.
 */
static bool need_unmount_scene_ctrl(u32 idle_policy)
{
	return (idle_policy & IDLE_P1_POWER_OFF) ||
			(idle_policy & IDLE_P2_POWER_OFF) ||
			(idle_policy & IDLE_P0_POWER_OFF);
}

int dpu_cmd_idle_prepare(struct dpu_idle_ctx *idle_ctx)
{
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dsi_connector *dsi_connector;
	struct dsi_encoder *dsi_encoder;
	struct dpu_virt_pipeline *pipeline;
	struct drm_crtc_state *crtc_state;
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_crtc *dpu_crtc;
	ktime_t start_time;
	u32 mixer_mask;
	u32 rch_mask = 0;
	int ret;
	u32 i;

	if (!idle_ctx) {
		IDLE_ERROR("idle_ctx is null\n");
		return -EINVAL;
	}

	crtc_state = idle_ctx->crtc->state;
	if (!crtc_state) {
		IDLE_ERROR("crtc_state is null\n");
		return -EINVAL;
	}
	dpu_crtc = to_dpu_crtc(crtc_state->crtc);
	dpu_crtc_state = to_dpu_crtc_state(crtc_state);
	IDLE_DEBUG("frame:%llu\n", dpu_crtc_state->frame_no);
	idle_ctx->trace.enter_frame_no = dpu_crtc_state->frame_no;

	INIT_START_TIMING(start_time);
	flush_workqueue(dpu_crtc->event_wq);
	END_TIMING_AND_PRINT_IF_EXCEEDS("flush_workqueue", g_idle_duration_ms, start_time);

	pipeline = &idle_ctx->dpu_kms->virt_pipeline[drm_crtc_index(idle_ctx->crtc)];
	hw_scene_ctl = get_scene_ctl_hw_res(idle_ctx);

	dpu_interrupt_irq_direct_ctrl(idle_ctx->intr_ctx, DPU_INTR_ONLINE0, false);

	if (need_disable_tmg_first(idle_ctx->idle_policy)) {
		dsi_connector = to_dsi_connector(idle_ctx->conn);
		dsi_encoder = to_dsi_encoder(dsi_connector->encoder);
		dsi_encoder_tmg_disable(dsi_encoder);
		hw_scene_ctl->ops.timing_engine_mount(&hw_scene_ctl->hw, 0x0);
	}

	if (need_unmount_scene_ctrl(idle_ctx->idle_policy)) {
		hw_scene_ctl->ops.rch_mount(&hw_scene_ctl->hw, 0x0);
		hw_scene_ctl->ops.wb_mount(&hw_scene_ctl->hw, 0x0);

		for (i = 0; i < RCH_MAX; i++) {
			if (pipeline->working_state.rch[i])
				rch_mask |= BIT(i);
		}
		dpu_hw_cmdlist_enable(idle_ctx->dpu_kms, rch_mask, false);

		mixer_mask = BIT(dpu_crtc->hw_mixer->hw.blk_id + RCH_MAX);
		dpu_hw_cmdlist_enable(idle_ctx->dpu_kms, mixer_mask, false);
		IDLE_DEBUG("frame:%llu rch_mask:0x%x mixer_mask:0x%x\n",
				dpu_crtc_state->frame_no, rch_mask, mixer_mask);
	}

	if (need_cfg_work(idle_ctx->idle_policy)) {
		hw_scene_ctl->ops.cfg_ready_update(&hw_scene_ctl->hw, NO_NEED_UPDT_EN,
				dpu_get_tui_state());

		ret = dpu_wait_irq_signal(idle_ctx, ONE_FRAME_TIME_MS, INTR_ONLINE0_CFG_RDY);
		if (ret) {
			DPU_WARN("frame:%llu wait cfg rdy clr failed\n", dpu_crtc_state->frame_no);
			dpu_exception_recovery(pipeline, crtc_state);
		}
	}

	dpu_kms_fence_force_signal(idle_ctx->crtc, false);

	return 0;
}

void dpu_cmd_idle_enter(struct dpu_idle_ctx *idle_ctx)
{
	struct dsi_connector *dsi_connector;
	struct dsi_encoder *dsi_encoder;
	u32 power_mask;
	ktime_t start_time;

	if (!idle_ctx) {
		IDLE_ERROR("idle_ctx is null\n");
		return;
	}

	if ((idle_ctx->idle_policy & IDLE_MIPI_ULPS) && (idle_ctx->ulps_status == false)) {
		INIT_START_TIMING(start_time);
		dsi_connector_ulps_enter(idle_ctx->conn);
		idle_ctx->ulps_status = true;
		END_TIMING_AND_PRINT_IF_EXCEEDS("ulps enter", 1, start_time);
		IDLE_DEBUG("enter ulps\n");
	}

	if (idle_ctx->idle_policy & IDLE_P0_POWER_OFF) {
		dsi_connector = to_dsi_connector(idle_ctx->conn);
		dsi_encoder = to_dsi_encoder(dsi_connector->encoder);
		dpu_intr_pipe_irq_disable(idle_ctx->intr_ctx, DPU_INTR_ONLINE0);
		IDLE_DEBUG("IDLE_P0_POWER_OFF");
	}

	/* Constraint: P0 clk off need to be later than P1/P2 power off */
	dpu_idle_power_disable(idle_ctx);

	power_mask = get_crtc_desired_power_mask(idle_ctx->dpu_kms, idle_ctx->crtc);
	if ((idle_ctx->idle_policy & IDLE_DSI_CLK_OFF) &&
			(power_mask & BIT(DPU_PARTITION_0))) {
		dsi_connector_clk_ctrl(idle_ctx->conn, false);
		IDLE_DEBUG("IDLE_DSI_CLK_OFF");
	}
	trace_dpu_cmd_idle_enter("idle enter", idle_ctx->idle_policy);
}

void dpu_cmd_idle_exit(struct dpu_idle_ctx *idle_ctx)
{
	struct dsi_connector *dsi_connector;
	struct dsi_encoder *dsi_encoder;
	u32 power_mask = 0;
	ktime_t start_time;

	dsi_connector = to_dsi_connector(idle_ctx->conn);
	dsi_encoder = to_dsi_encoder(dsi_connector->encoder);
	power_mask = get_crtc_desired_power_mask(idle_ctx->dpu_kms, idle_ctx->crtc);

	dpu_idle_power_enable(idle_ctx);
	if (idle_ctx->idle_policy & IDLE_P0_POWER_OFF) {
		dsi_display_tmg_init(dsi_encoder->display);
		dpu_intr_pipe_irq_restore(idle_ctx->intr_ctx, DPU_INTR_ONLINE0);
		IDLE_DEBUG("IDLE_P0_POWER_ON");
	}

	if (idle_ctx->idle_policy & IDLE_DSI_CLK_OFF) {
		dsi_connector_clk_ctrl(idle_ctx->conn, true);
		IDLE_DEBUG("IDLE_DSI_CLK_ON");
	}

	if (need_disable_tmg_first(idle_ctx->idle_policy)) {
		dsi_connector = to_dsi_connector(idle_ctx->conn);
		dsi_encoder = to_dsi_encoder(dsi_connector->encoder);
		dsi_encoder_tmg_enable(dsi_encoder);
		IDLE_DEBUG("enable tmg\n");
	}

	if ((idle_ctx->idle_policy & IDLE_MIPI_ULPS) && (idle_ctx->ulps_status == true)) {
		INIT_START_TIMING(start_time);
		dsi_connector_ulps_exit(idle_ctx->conn);
		idle_ctx->ulps_status = false;
		END_TIMING_AND_PRINT_IF_EXCEEDS("ulps exit", 1, start_time);
		IDLE_DEBUG("exit ulps\n");
	}

	if (idle_ctx->idle_policy & IDLE_ISR_OFF)
		dpu_interrupt_irq_direct_ctrl(idle_ctx->intr_ctx, DPU_INTR_ONLINE0, true);

	trace_dpu_cmd_idle_exit("idle exit", idle_ctx->idle_policy);
}

void dpu_cmd_idle_deinit(struct dpu_idle_ctx *idle_ctx)
{
}
