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

#include <linux/list.h>
#include <linux/of_irq.h>
#include <linux/device.h>
#include <drm/drm_device.h>
#include <drm/drm_print.h>
#include "dpu_kms.h"
#include "dpu_intr_core.h"
#include "dpu_log.h"
#include "dpu_core_perf.h"
#include "dpu_exception.h"
#include "dpu_crtc.h"

const char *online0_irq_name = "online0_irq";
const char *online1_irq_name = "online1_irq";
const char *offline0_irq_name = "offline0_irq";

static inline int __dpu_interrupt_get_type(struct dpu_intr_context *ctx,
		int irq)
{
	int i;

	for (i = DPU_INTR_ONLINE0; i < DPU_INTR_TYPE_MAX; i++)
		if (irq == ctx->intr_pipe[i].irq)
			return i;

	return -EINVAL;
}

static void dpu_interrupt_irq_status_update(struct dpu_intr_context *ctx, int intr_id)
{
	atomic_inc(&ctx->intr_status.irq_counts[intr_id]);

	ctx->intr_status.last_timestamp[intr_id] = ctx->intr_status.curr_timestamp[intr_id];
	ctx->intr_status.curr_timestamp[intr_id] = ktime_get();
}

static irqreturn_t dpu_interrupt_irq_handler(int irq, void *data)
{
	struct dpu_intr_context *ctx = data;
	struct dpu_intr_info *intr_info;
	struct dpu_intr_pipe *intr_pipe;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_hw_intr *hw_intr;
	struct list_head *irq_list;
	unsigned long flags;
	bool is_triggered = false;
	int intr_type;
	int intr_id;

	intr_type = __dpu_interrupt_get_type(ctx, irq);
	if (intr_type < 0) {
		DRM_ERROR_RATELIMITED("incorrect irq number %d\n", irq);
		return IRQ_HANDLED;
	}

	intr_pipe = &ctx->intr_pipe[intr_type];
	irq_list = &intr_pipe->irq_list;
	hw_intr = ctx->hw_intr;
	dpu_drm_dev = to_dpu_drm_dev(ctx->drm_dev);

	dpu_core_perf_clk_active();

	spin_lock_irqsave(&intr_pipe->irq_lock, flags);

	if (!intr_pipe->is_enabled) {
		spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);
		DRM_ERROR_RATELIMITED("interrupt is not enabled %d\n", irq);
		return IRQ_HANDLED;
	}

	hw_intr->ops.status_get_all(&hw_intr->hw, intr_type,
			hw_intr->intr_reg_val, hw_intr->intr_reg_num);

	list_for_each_entry(intr_info, irq_list, list) {
		intr_id = intr_info->intr_id;
		is_triggered = hw_intr->ops.status_get_from_cache(&hw_intr->hw, intr_id,
				hw_intr->intr_reg_val, hw_intr->intr_reg_num);
		if (is_triggered) {
			dpu_interrupt_irq_status_update(ctx, intr_id);
			intr_info->cb(intr_id, ctx);
		}
	}

	spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);

	dpu_core_perf_clk_deactive();

	return IRQ_HANDLED;
}

void dpu_intr_pipe_irq_disable(struct dpu_intr_context *ctx, enum dpu_intr_type intr_type)
{
	struct dpu_intr_pipe *intr_pipe;
	struct dpu_intr_info *intr_info;
	struct dpu_hw_intr *hw_intr;
	struct dpu_intr_info *temp;
	unsigned long flags;

	intr_pipe = &ctx->intr_pipe[intr_type];
	hw_intr = ctx->hw_intr;

	spin_lock_irqsave(&intr_pipe->irq_lock, flags);
	dpu_core_perf_clk_active();
	list_for_each_entry_safe(intr_info, temp, &intr_pipe->irq_list, list) {
		hw_intr->ops.enable(&hw_intr->hw, intr_info->intr_id, false);
		ctx->intr_status.irq_enable[intr_info->intr_id] = false;
	}
	dpu_core_perf_clk_deactive();
	spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);
}

void dpu_intr_pipe_irq_restore(struct dpu_intr_context *ctx, enum dpu_intr_type intr_type)
{
	struct dpu_intr_pipe *intr_pipe;
	struct dpu_intr_info *intr_info;
	struct dpu_hw_intr *hw_intr;
	struct dpu_intr_info *temp;
	unsigned long flags;

	intr_pipe = &ctx->intr_pipe[intr_type];
	hw_intr = ctx->hw_intr;

	spin_lock_irqsave(&intr_pipe->irq_lock, flags);
	dpu_core_perf_clk_active();
	list_for_each_entry_safe(intr_info, temp, &intr_pipe->irq_list, list) {
		hw_intr->ops.enable(&hw_intr->hw, intr_info->intr_id, true);
		ctx->intr_status.irq_enable[intr_info->intr_id] = true;
	}
	dpu_core_perf_clk_deactive();
	spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);
}

int dpu_interrupt_register(struct dpu_intr_context *ctx, u32 intr_id,
		void (*cb)(int intr_id, void *data))
{
	struct dpu_intr_pipe *intr_pipe;
	struct dpu_intr_info *intr_info;
	enum dpu_intr_type intr_type;
	struct dpu_hw_intr *hw_intr;
	unsigned long flags;

	DPU_ISR_DEBUG("intr_id = %d\n", intr_id);

	intr_type = dpu_interrupt_id_to_type(intr_id);
	if (intr_type >= DPU_INTR_TYPE_MAX) {
		DPU_ERROR("failed to get interrupt type\n");
		return -EINVAL;
	}

	hw_intr = ctx->hw_intr;
	intr_pipe = &ctx->intr_pipe[intr_type];

	intr_info = kzalloc(sizeof(*intr_info), GFP_KERNEL);
	if (!intr_info)
		return -ENOMEM;

	intr_info->intr_id = intr_id;
	intr_info->cb = cb;
	ctx->intr_status.irq_enable[intr_id] = true;

	dpu_core_perf_clk_active();
	hw_intr->ops.status_clear(&hw_intr->hw, intr_id);
	hw_intr->ops.enable(&hw_intr->hw, intr_id, true);
	dpu_core_perf_clk_deactive();

	spin_lock_irqsave(&intr_pipe->irq_lock, flags);
	list_add_tail(&intr_info->list, &intr_pipe->irq_list);
	spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);

	return 0;
}

void dpu_interrupt_unregister(struct dpu_intr_context *ctx, u32 intr_id)
{
	struct dpu_intr_pipe *intr_pipe;
	struct dpu_intr_info *intr_info;
	struct dpu_intr_info *temp;
	enum dpu_intr_type intr_type;
	struct dpu_hw_intr *hw_intr;
	unsigned long flags;

	DPU_ISR_DEBUG("intr_id = %d\n", intr_id);
	intr_type = dpu_interrupt_id_to_type(intr_id);
	if (intr_type >= DPU_INTR_TYPE_MAX) {
		DPU_ERROR("failed to get interrupt type\n");
		return;
	}

	hw_intr = ctx->hw_intr;
	intr_pipe = &ctx->intr_pipe[intr_type];

	dpu_core_perf_clk_active();
	hw_intr->ops.enable(&hw_intr->hw, intr_id, false);
	ctx->intr_status.irq_enable[intr_id] = false;
	dpu_core_perf_clk_deactive();

	spin_lock_irqsave(&intr_pipe->irq_lock, flags);
	list_for_each_entry_safe(intr_info, temp, &intr_pipe->irq_list, list) {
		if (intr_info->intr_id != intr_id)
			continue;

		list_del(&intr_info->list);
		kfree(intr_info);
	}
	spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);
}

void dpu_interrupt_clone_connector_update(struct dpu_intr_context *ctx,
		enum dpu_intr_type type, struct drm_connector *connector)
{
	struct dpu_intr_pipe *intr_pipe;
	unsigned long flags;

	if (!ctx) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	if (type < 0 || type >= DPU_INTR_TYPE_MAX) {
		DPU_ERROR("incorrect interrupt type %d\n", type);
		return;
	}

	intr_pipe = &ctx->intr_pipe[type];

	spin_lock_irqsave(&intr_pipe->irq_lock, flags);
	intr_pipe->clone_connector = connector;
	spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);
}

int dpu_interrupt_irq_enable(struct dpu_intr_context *ctx,
		struct drm_connector *connector,
		enum dpu_intr_type type, bool enable)
{
	struct dpu_intr_pipe *intr_pipe;
	unsigned long flags;

	if (!ctx) {
		DPU_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	if (type < 0 || type >= DPU_INTR_TYPE_MAX) {
		DPU_ERROR("incorrect interrupt type %d\n", type);
		return -EINVAL;
	}

	intr_pipe = &ctx->intr_pipe[type];

	if (intr_pipe->is_enabled == enable) {
		DPU_ISR_DEBUG("irq state is not changed, intr type %d\n", type);
		return 0;
	}

	if (enable) {
		spin_lock_irqsave(&intr_pipe->irq_lock, flags);
		intr_pipe->is_enabled = true;
		intr_pipe->connector = connector;
		intr_pipe->crtc = connector->state->crtc;
		spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);

		enable_irq(intr_pipe->irq);
	} else {
		disable_irq(intr_pipe->irq);

		spin_lock_irqsave(&intr_pipe->irq_lock, flags);
		intr_pipe->is_enabled = false;
		intr_pipe->connector = NULL;
		intr_pipe->crtc = NULL;
		spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);
	}

	DPU_DEBUG("irq enabled: %d\n", intr_pipe->is_enabled);

	return 0;
}

int dpu_interrupt_irq_direct_ctrl(struct dpu_intr_context *ctx,
		enum dpu_intr_type type, bool enable)
{
	struct dpu_intr_pipe *intr_pipe;
	unsigned long flags;

	if (!ctx) {
		DPU_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	if (type < 0 || type >= DPU_INTR_TYPE_MAX) {
		DPU_ERROR("incorrect interrupt type %d\n", type);
		return -EINVAL;
	}

	intr_pipe = &ctx->intr_pipe[type];

	if (intr_pipe->is_enabled == enable) {
		DPU_ISR_DEBUG("irq state is not changed\n");
		return 0;
	}

	if (enable) {
		spin_lock_irqsave(&intr_pipe->irq_lock, flags);
		intr_pipe->is_enabled = true;
		spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);

		enable_irq(intr_pipe->irq);
	} else {
		disable_irq(intr_pipe->irq);

		spin_lock_irqsave(&intr_pipe->irq_lock, flags);
		intr_pipe->is_enabled = false;
		spin_unlock_irqrestore(&intr_pipe->irq_lock, flags);
	}

	DPU_ISR_DEBUG("irq enabled: %d\n", intr_pipe->is_enabled);

	return 0;
}

static int dpu_interrupt_irq_parser(struct dpu_intr_context *ctx,
		struct device_node *np)
{
	int irq;

	if (!np || !ctx) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, ctx);
		return -EINVAL;
	}

	irq = of_irq_get_byname(np, online0_irq_name);
	if (irq <= 0) {
		DPU_ERROR("failed to get online0 irq\n");
		return irq;
	}

	ctx->intr_pipe[DPU_INTR_ONLINE0].irq = irq;

	irq = of_irq_get_byname(np, online1_irq_name);
	if (irq <= 0) {
		DPU_ERROR("failed to get online1 irq\n");
		return irq;
	}

	ctx->intr_pipe[DPU_INTR_ONLINE1].irq = irq;

	irq = of_irq_get_byname(np, offline0_irq_name);
	if (irq <= 0) {
		DPU_ERROR("failed to get offline0 irq\n");
		return irq;
	}

	ctx->intr_pipe[DPU_INTR_OFFLINE0].irq = irq;

	return 0;
}

int dpu_interrupt_context_init(struct dpu_intr_context **ctx,
		struct drm_device *drm_dev)
{
	struct dpu_res_mgr_ctx *res_mgr_ctx;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *new_ctx;
	struct dpu_hw_blk *hw_blk;
	struct dpu_kms *dpu_kms;
	int ret;
	int i;

	DPU_DEBUG("Enter\n");

	if (!ctx || !drm_dev) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", ctx, drm_dev);
		return -EINVAL;
	}

	new_ctx = kzalloc(sizeof(*new_ctx), GFP_KERNEL);
	if (!new_ctx)
		return -ENOMEM;

	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	res_mgr_ctx = dpu_kms->res_mgr_ctx;

	ret = dpu_interrupt_irq_parser(new_ctx, drm_dev->dev->of_node);
	if (ret) {
		DPU_ERROR("failed to parse irq, ret %d\n", ret);
		goto err_parse;
	}

	hw_blk = dpu_res_mgr_block_reserve(res_mgr_ctx,
			VIRTUAL_CTRL_PIPE_ID, BLK_DPU_INTR, false);
	if (IS_ERR_OR_NULL(hw_blk)) {
		DPU_ERROR("failed to reserve dpu interrupt\n");
		ret = PTR_ERR(hw_blk);
		goto err_parse;
	}

	new_ctx->hw_intr = to_dpu_hw_intr(hw_blk);
	new_ctx->drm_dev = drm_dev;

	new_ctx->intr_status.total_irqs = INTR_TOTAL_HW_IRQ_NUM;
	new_ctx->intr_status.curr_timestamp = kcalloc(new_ctx->intr_status.total_irqs,
			sizeof(ktime_t), GFP_KERNEL);
	new_ctx->intr_status.last_timestamp = kcalloc(new_ctx->intr_status.total_irqs,
			sizeof(ktime_t), GFP_KERNEL);
	new_ctx->intr_status.irq_counts = kcalloc(new_ctx->intr_status.total_irqs,
			sizeof(atomic_t), GFP_KERNEL);
	new_ctx->intr_status.irq_enable = kcalloc(new_ctx->intr_status.total_irqs,
			sizeof(bool), GFP_KERNEL);

	if (!new_ctx->intr_status.curr_timestamp || !new_ctx->intr_status.last_timestamp ||
			!new_ctx->intr_status.irq_counts || !new_ctx->intr_status.irq_enable) {
		DPU_ERROR("failed to alloc memory\n");
		goto err_alloc;
	}

	ret = request_irq(new_ctx->intr_pipe[DPU_INTR_ONLINE0].irq,
			&dpu_interrupt_irq_handler, 0,
			online0_irq_name, new_ctx);
	if (ret) {
		DPU_ERROR("failed to request online0 irq, ret %d\n", ret);
		goto err_online0;
	}
	disable_irq(new_ctx->intr_pipe[DPU_INTR_ONLINE0].irq);

	ret = request_irq(new_ctx->intr_pipe[DPU_INTR_ONLINE1].irq,
			&dpu_interrupt_irq_handler, 0,
			online1_irq_name, new_ctx);
	if (ret) {
		DPU_ERROR("failed to request online1 irq, ret %d\n", ret);
		goto err_online1;
	}
	disable_irq(new_ctx->intr_pipe[DPU_INTR_ONLINE1].irq);

	ret = request_irq(new_ctx->intr_pipe[DPU_INTR_OFFLINE0].irq,
			&dpu_interrupt_irq_handler, 0,
			offline0_irq_name, new_ctx);
	if (ret) {
		DPU_ERROR("failed to request offline0 irq, ret %d\n", ret);
		goto err_offline0;
	}
	disable_irq(new_ctx->intr_pipe[DPU_INTR_OFFLINE0].irq);

	for (i = 0; i < DPU_INTR_TYPE_MAX; i++) {
		INIT_LIST_HEAD(&new_ctx->intr_pipe[i].irq_list);
		spin_lock_init(&new_ctx->intr_pipe[i].irq_lock);
	}

	*ctx = new_ctx;

	DPU_DEBUG("Exit\n");

	return 0;

err_offline0:
	free_irq(new_ctx->intr_pipe[DPU_INTR_ONLINE1].irq, ctx);
err_online1:
	free_irq(new_ctx->intr_pipe[DPU_INTR_ONLINE0].irq, ctx);
err_alloc:
	kfree(new_ctx->intr_status.curr_timestamp);
	kfree(new_ctx->intr_status.last_timestamp);
	kfree(new_ctx->intr_status.irq_counts);
	kfree(new_ctx->intr_status.irq_enable);
err_online0:
	dpu_res_mgr_block_release(res_mgr_ctx, hw_blk);
err_parse:
	kfree(new_ctx);
	return ret;
}

void dpu_interrupt_context_deinit(struct dpu_intr_context *ctx)
{
	struct dpu_res_mgr_ctx *res_mgr_ctx;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;
	int i;

	if (!ctx)
		return;

	dpu_drm_dev = to_dpu_drm_dev(ctx->drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	res_mgr_ctx = dpu_kms->res_mgr_ctx;

	for (i = 0; i < DPU_INTR_TYPE_MAX; i++)
		free_irq(ctx->intr_pipe[i].irq, ctx);

	if (ctx->hw_intr)
		dpu_res_mgr_block_release(res_mgr_ctx, &ctx->hw_intr->hw);

	kfree(ctx->intr_status.curr_timestamp);
	kfree(ctx->intr_status.last_timestamp);
	kfree(ctx->intr_status.irq_counts);
	kfree(ctx->intr_status.irq_enable);
}

int dpu_intr_type_get(struct drm_crtc *drm_crtc, u32 *type_out)
{
	struct dpu_crtc *dpu_crtc;
	unsigned long mixer_feature;
	u32 intr_type = DPU_INTR_TYPE_MAX;

	if (!drm_crtc) {
		DPU_ERROR("invalid parameter, %pK\n", drm_crtc);
		goto error_get;
	}

	dpu_crtc = to_dpu_crtc(drm_crtc);
	if (!dpu_crtc->hw_mixer) {
		DPU_DEBUG("hw_mixer is null\n");
		goto error_get;
	}

	mixer_feature = dpu_crtc->hw_mixer->hw.features;
	if (test_bit(MIXER_PRIMARY, &mixer_feature))
		intr_type = DPU_INTR_ONLINE0;
	else if (test_bit(MIXER_SECONDARY, &mixer_feature))
		intr_type = DPU_INTR_ONLINE1;
	else if (test_bit(MIXER_WB, &mixer_feature))
		intr_type = DPU_INTR_OFFLINE0;

	if (intr_type == DPU_INTR_TYPE_MAX) {
		DPU_ERROR("failed to get interrupt type\n");
		goto error_get;
	}

	*type_out = intr_type;
	return 0;

error_get:
	*type_out = DPU_INTR_TYPE_MAX;
	return -EINVAL;
}
