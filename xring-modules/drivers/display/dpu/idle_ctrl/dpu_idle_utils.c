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

#include "dpu_intr_core.h"
#include "dpu_idle_utils.h"

void dpu_irq_clear(struct dpu_idle_ctx *idle_ctx, enum dpu_intr_id intr_id)
{
	struct dpu_hw_intr *hw_intr;

	if (!idle_ctx) {
		IDLE_ERROR("idle_ctx is null\n");
		return;
	}

	hw_intr = idle_ctx->intr_ctx->hw_intr;
	hw_intr->ops.status_clear(&hw_intr->hw, intr_id);
}

int dpu_wait_irq_signal(struct dpu_idle_ctx *idle_ctx, u32 timeout_ms, enum dpu_intr_id intr_id)
{
	bool is_triggered = false;
	u32 timeout_count = timeout_ms;
	struct dpu_hw_intr *hw_intr;

	if (!idle_ctx) {
		IDLE_ERROR("idle_ctx is null\n");
		return -EINVAL;
	}

	hw_intr = idle_ctx->intr_ctx->hw_intr;
	while (!is_triggered && (timeout_count > 0)) {
		mdelay(1);
		is_triggered = hw_intr->ops.status_get(&hw_intr->hw, intr_id);
		if (is_triggered) {
			hw_intr->ops.status_clear(&hw_intr->hw, intr_id);
			break;
		}

		timeout_count--;
	}
	if (timeout_count == 0)
		return -EAGAIN;

	IDLE_DEBUG("wait intr:%d use %u ms\n", intr_id, timeout_ms - timeout_count);
	return 0;
}

void dpu_clk_ctrl_on_demand(struct dpu_idle_ctx *idle_ctx, u32 part_id, bool enable)
{
	switch (part_id) {
	case DPU_PARTITION_1:
		if (idle_ctx->pinfo->dsc_en)
			dpu_clk_ctrl(idle_ctx->power_mgr->clk_mgr, CLK_DPU_DSC0, enable);
		break;
	default:
		break;
	}
}

void dpu_idle_power_disable(struct dpu_idle_ctx *idle_ctx)
{
	struct dpu_virt_pipeline *pipeline;
	u32 crtc_power_mask;
	u32 mask = 0;
	u32 index;

	index = drm_crtc_index(idle_ctx->crtc);
	pipeline = &idle_ctx->dpu_kms->virt_pipeline[index];

	crtc_power_mask = get_crtc_desired_power_mask(idle_ctx->dpu_kms, idle_ctx->crtc);

	if (pipeline->power_ctrl.frame_ctrl_enabled)
		DPU_WARN("why frame power ctrl is enabled\n");

	if ((idle_ctx->idle_policy & IDLE_P2_POWER_OFF) &&
			crtc_power_mask & BIT(DPU_PARTITION_2))
		mask |= BIT(DPU_PARTITION_2);

	if ((idle_ctx->idle_policy & IDLE_P1_POWER_OFF) &&
			crtc_power_mask & BIT(DPU_PARTITION_1))
		mask |= BIT(DPU_PARTITION_1);

	if ((idle_ctx->idle_policy & IDLE_P0_POWER_OFF) &&
			crtc_power_mask & BIT(DPU_PARTITION_0))
		mask |= BIT(DPU_PARTITION_0);

	IDLE_DEBUG("mask is 0x%x\n", mask);
	dpu_power_disable_for_crtc(idle_ctx->crtc, mask, IDLE_POWER_REQUEST);
}

void dpu_idle_power_enable(struct dpu_idle_ctx *idle_ctx)
{
	struct dpu_virt_pipeline *pipeline;
	u32 crtc_power_mask;
	u32 mask = 0;
	u32 index;

	index = drm_crtc_index(idle_ctx->crtc);
	pipeline = &idle_ctx->dpu_kms->virt_pipeline[index];

	crtc_power_mask = get_crtc_desired_power_mask(idle_ctx->dpu_kms, idle_ctx->crtc);

	if (pipeline->power_ctrl.frame_ctrl_enabled)
		DPU_WARN("why frame power ctrl is enabled\n");

	if ((idle_ctx->idle_policy & IDLE_P2_POWER_OFF) &&
			crtc_power_mask & BIT(DPU_PARTITION_2))
		mask |= BIT(DPU_PARTITION_2);

	if ((idle_ctx->idle_policy & IDLE_P1_POWER_OFF) &&
			crtc_power_mask & BIT(DPU_PARTITION_1))
		mask |= BIT(DPU_PARTITION_1);


	if ((idle_ctx->idle_policy & IDLE_P0_POWER_OFF) &&
			crtc_power_mask & BIT(DPU_PARTITION_0))
		mask |= BIT(DPU_PARTITION_0);
	IDLE_DEBUG("mask is 0x%x\n", mask);

	dpu_power_enable_for_crtc(idle_ctx->crtc, mask, IDLE_POWER_REQUEST);
}
