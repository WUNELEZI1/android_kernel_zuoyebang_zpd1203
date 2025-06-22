/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DPU_IDLE_UTILS_H_
#define _DPU_IDLE_UTILS_H_

#include <linux/sched.h>

#include "dpu_idle_core.h"
#include "dpu_log.h"
#include "dpu_kms.h"
#include "dpu_power_helper.h"

/**
 * dpu_wait_irq_signal - wait both cfg_rdy_clr isr
 *
 * @idle_ctx: struct dpu_idle_ctx
 * @timeout_ms: wait times (ms)
 * @updt_en: new frame config done
 * @return 0 : succ, else : wait timeout
 */
int dpu_wait_irq_signal(struct dpu_idle_ctx *idle_ctx, u32 timeout_ms, enum dpu_intr_id intr_id);

/**
 * dpu_irq_clear - clear irq status
 *
 * @idle_ctx: struct dpu_idle_ctx
 * @intr_id: hw intr id
 */
void dpu_irq_clear(struct dpu_idle_ctx *idle_ctx, enum dpu_intr_id intr_id);

/**
 * get_scene_ctl_hw_res - Get the scene ctl hw res object of crtc
 *
 * @idle_ctx: struct dpu_idle_ctx
 * Return NULL for failure.
 */
static inline struct dpu_hw_scene_ctl *get_scene_ctl_hw_res(struct dpu_idle_ctx *idle_ctx)
{
	return idle_ctx->dpu_kms->virt_pipeline[drm_crtc_index(idle_ctx->crtc)].hw_scene_ctl;
}

/**
 * dpu_clk_ctrl_on_demand - ctrl clk enable on demand
 *
 * @idle_ctx: struct dpu_idle_ctx
 * @part_id: dpu partition id
 * @enable: bool
 */
void dpu_clk_ctrl_on_demand(struct dpu_idle_ctx *idle_ctx, u32 part_id, bool enable);

/**
 * dpu_idle_power_enable - do idle power policy enable
 * @idle_ctx: struct dpu_idle_ctx
 */
void dpu_idle_power_enable(struct dpu_idle_ctx *idle_ctx);

/**
 * dpu_idle_power_disable - do idle power policy disable
 * @idle_ctx: struct dpu_idle_ctx
 */
void dpu_idle_power_disable(struct dpu_idle_ctx *idle_ctx);

#endif
