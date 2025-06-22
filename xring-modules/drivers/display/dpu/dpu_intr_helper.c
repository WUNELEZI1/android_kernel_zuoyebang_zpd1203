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
#include "dpu_intr_helper.h"
#include "dpu_isr.h"
#include "dpu_kms.h"

void dpu_online_irq_status_dump(struct dpu_intr_context *intr_ctx,
		struct dpu_connector_intr_context *conn_intr_ctx)
{
	int i, intr_id;

	for (i = 0; i < DPU_CONN_INTR_TYPE_MAX_NUM; i++) {
		if (!conn_intr_ctx->enable[i])
			continue;

		intr_id = conn_intr_ctx->intr_id[i];
		if (intr_id >= INTR_TOTAL_HW_IRQ_NUM)
			continue;

		DPU_INFO("%-30s(id:%3d), counts %8d, curr ts %12lld(us), last ts %12lld(us)\n",
				intr_map[intr_id].irq_name,
				intr_id,
				atomic_read(&intr_ctx->intr_status.irq_counts[intr_id]),
				ktime_to_us(intr_ctx->intr_status.curr_timestamp[intr_id]),
				ktime_to_us(intr_ctx->intr_status.last_timestamp[intr_id]));
	}
}

void dpu_intr_online_cfg_rdy_clr_handler(struct dpu_virt_pipeline *pipeline)
{
	atomic_set(&pipeline->cfg_done_flag, 1);
	wake_up(&pipeline->cfg_done_wq);

	if (pipeline->updt_en) {
		atomic_inc(&pipeline->commit_ctx.release_timeline.pending_frame_num);
		timeline_inc_step(&pipeline->commit_ctx.timeline);
	}

	atomic_set(&pipeline->vsync_flag, 0);
}

void dpu_intr_online_vsync_handler(struct dpu_virt_pipeline *pipeline, u64 timestamp)
{
	atomic_set(&pipeline->vsync_flag, 1);

	dpu_isr_notify_listener(&pipeline->commit_ctx.isr,
			INTS_FRM_TIMING_VSYNC, timestamp);

	if (atomic_dec_if_positive(&pipeline->commit_ctx.release_timeline.pending_frame_num) >= 0)
		timeline_inc_step(&pipeline->commit_ctx.release_timeline);
}

void dpu_intr_online_vstart_handler(struct dpu_virt_pipeline *pipeline)
{
	atomic_set(&pipeline->vstart_flag, 1);
	wake_up(&pipeline->vstart_wq);
}

void dpu_intr_online_underflow_handler(struct dpu_intr_context *intr_ctx,
		struct dpu_virt_pipeline *pipeline,
		int intr_id)
{
	atomic_set(&pipeline->underflow_flag, 1);
	intr_ctx->hw_intr->ops.enable(&intr_ctx->hw_intr->hw, intr_id, false);
}

u32 get_online_intr_id(u32 intr_type, enum dpu_online_conn_intr_type conn_intr_type)
{
	u32 intr_id;
	bool is_online0 = !!(intr_type == DPU_INTR_ONLINE0);

	switch (conn_intr_type) {
	case DPU_CFG_RDY_CLR_INT:
		intr_id = is_online0 ? INTR_ONLINE0_CFG_RDY : INTR_COMBO_CFG_RDY;
		break;
	case DPU_VSYNC_INT:
		intr_id = is_online0 ?
				INTR_ONLINE0_FRM_TIMING_VSYNC : INTR_COMBO_FRM_TIMING_VSYNC;
		break;
	case DPU_VSTART_INT:
		intr_id = is_online0 ?
				INTR_ONLINE0_TMG_VACTIVE_START : INTR_COMBO_TMG_VACTIVE_START;
		break;
	case DPU_EOF_INT:
		intr_id = is_online0 ?
				INTR_ONLINE0_FRM_TIMING_EOF : INTR_COMBO_FRM_TIMING_EOF;
		break;
	case DPU_UNDERFLOW_INT:
		intr_id = is_online0 ?
				INTR_ONLINE0_FRM_TIMING_UNDERFLOW : INTR_COMBO_FRM_TIMING_UNDERFLOW;
		break;
	case DPU_CFG_WIN_START_INT:
		intr_id = is_online0 ?
				INTR_ONLINE0_FRM_TIMING_EOF_START : INTR_COMBO_FRM_TIMING_EOF_START;
		break;
	case DPU_CFG_WIN_END_INT:
		intr_id = is_online0 ?
				INTR_ONLINE0_FRM_TIMING_EOF_END : INTR_COMBO_FRM_TIMING_EOF_END;
		break;
	default:
		DPU_ERROR("unknow connector intr type\n");
		intr_id = INTR_TOTAL_HW_IRQ_NUM;
		break;
	}

	return intr_id;
}

int dpu_intr_online_irq_register(struct drm_connector *connector,
		struct dpu_connector_intr_context *conn_intr_ctx,
		void (*cb)(int intr_id, void *data))
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;
	int ret;
	int i;

	dpu_drm_dev = to_dpu_drm_dev(connector->dev);
	intr_ctx = dpu_drm_dev->dpu_kms->virt_ctrl.intr_ctx;

	for (i = 0; i < DPU_CONN_INTR_TYPE_MAX_NUM; i++) {
		if (!conn_intr_ctx->enable[i])
			continue;

		ret = dpu_interrupt_register(intr_ctx, conn_intr_ctx->intr_id[i],
				cb);
		if (ret) {
			DPU_ERROR("failed to register intr_id %d, ret %d\n",
					conn_intr_ctx->intr_id[i], ret);
			goto error;
		}
	}

	return 0;

error:
	for (--i; i >= 0; i--) {
		if (!conn_intr_ctx->enable[i])
			continue;
		dpu_interrupt_unregister(intr_ctx, conn_intr_ctx->intr_id[i]);
		conn_intr_ctx->enable[i] = false;
	}
	return ret;
}

void dpu_intr_online_irq_unregister(struct drm_connector *connector,
		struct dpu_connector_intr_context *conn_intr_ctx)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;
	int i;

	dpu_drm_dev = to_dpu_drm_dev(connector->dev);
	intr_ctx = dpu_drm_dev->dpu_kms->virt_ctrl.intr_ctx;

	for (i = 0; i < DPU_CONN_INTR_TYPE_MAX_NUM; i++) {
		if (!conn_intr_ctx->enable[i])
			continue;

		dpu_interrupt_unregister(intr_ctx, conn_intr_ctx->intr_id[i]);
		conn_intr_ctx->enable[i] = false;
	}
}
