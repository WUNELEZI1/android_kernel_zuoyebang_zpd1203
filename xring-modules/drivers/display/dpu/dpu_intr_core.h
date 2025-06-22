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

#ifndef _DPU_INTR_CORE_H_
#define _DPU_INTR_CORE_H_

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include "dpu_drv.h"
#include "dpu_hw_intr.h"

/**
 * dpu_intr_info - the interrupt info struture for one interrupt
 * @intr_id: the logic interrupt index
 * @cb: the callback of interrupt handler
 * @list: list to interrupt pipe
 */
struct dpu_intr_info {
	int intr_id;
	void (*cb)(int irq, void *data);
	struct list_head list;
};

/**
 * dpu_intr_pipe - the all interrupt info structure for one pipe
 * @irq: the irq number
 * @is_enabled: the irq is enabled or not
 * @connector: the attached connector pointer for interrupt pipe
 * @crtc: the attached crtc pointer for interrupt pipe
 * @clone_connector: the attached clone connector pointer
 * @irq_lock: the spinlock for irq internal resource protection
 * @irq_list: the list head for all registered interrupt
 */
struct dpu_intr_pipe {
	int irq;
	bool is_enabled;
	struct drm_connector *connector;
	struct drm_crtc *crtc;
	struct drm_connector *clone_connector;

	spinlock_t irq_lock;
	struct list_head irq_list;
};

/**
 * dpu_intr_status - the dpu intr status structure
 * @total_irqs: dpu total irqs num
 * @irq_enable: each dpu irq enable status
 * @irq_counts: each dpu irq trigged times
 * @curr_timestamp: recode current irq happend timestamp
 * @last_timestamp: recode last irq happend timestamp
 */
struct dpu_intr_status {
	u32 total_irqs;
	bool *irq_enable;

	atomic_t *irq_counts;
	ktime_t *curr_timestamp;
	ktime_t *last_timestamp;
};

/**
 * dpu_intr_context - the dpu interrupt context structure
 * @drm_dev: the drm device pointer
 * @intr_status: the dpu interrupt status
 * @hw_intr: the interrupt hardware handle
 * @intr_pipe: the interrupt pipe instances
 */
struct dpu_intr_context {
	struct drm_device *drm_dev;

	struct dpu_intr_status intr_status;
	struct dpu_hw_intr *hw_intr;
	struct dpu_intr_pipe intr_pipe[DPU_INTR_TYPE_MAX];
};

static inline enum dpu_intr_type dpu_interrupt_id_to_type(u32 intr_id)
{
	if (intr_id < INTR_ONLINE0_MAX)
		return DPU_INTR_ONLINE0;
	else if ((intr_id > INTR_ONLINE0_MAX) && (intr_id < INTR_COMBO_MAX))
		return DPU_INTR_ONLINE1;
	else if ((intr_id > INTR_COMBO_MAX) && (intr_id < INTR_OFFLINE0_MAX))
		return DPU_INTR_OFFLINE0;
	else
		return DPU_INTR_TYPE_MAX;
}

/**
 * dpu_interrupt_clone_connector_update - update clone connector
 * @ctx: the interrupt context pointer
 * @type: indicate which pipe need to be enabled
 * @connector: the attached connector for the interrupt pipe
 */
void dpu_interrupt_clone_connector_update(struct dpu_intr_context *ctx,
		enum dpu_intr_type type, struct drm_connector *connector);

/**
 * dpu_interrupt_register - register one interrupt
 * @ctx: the interrupt context pointer
 * @intr_id: the logic interrupt index
 * @cb: the callback for interrupt handling
 *
 * Return: zero on success, -errno on failure
 */
int dpu_interrupt_register(struct dpu_intr_context *ctx, u32 intr_id,
		void (*cb)(int intr_id, void *data));

/**
 * dpu_interrupt_unregister - unregister one interrupt
 * @ctx: the interrupt context pointer
 * @intr_id: the logic interrupt index
 */
void dpu_interrupt_unregister(struct dpu_intr_context *ctx, u32 intr_id);

/**
 * dpu_interrupt_irq_enable - enable the irq for one interrupt pipe
 * @ctx: the interrupt context pointer
 * @connector: the attached connector for the interrupt pipe
 * @type: indicate which pipe need to be enabled
 * @enable: true for enabling, false for disabling
 *
 * Return: zero on success, -errno on failure
 */
int dpu_interrupt_irq_enable(struct dpu_intr_context *ctx,
		struct drm_connector *connector,
		enum dpu_intr_type type, bool enable);

/**
 * dpu_interrupt_irq_direct_ctrl - ctrl the irq for one interrupt pipe directly
 * @ctx: the interrupt context pointer
 * @type: indicate which pipe need to be enabled
 * @enable: true for enabling, false for disabling
 *
 * Return: zero on success, -errno on failure
 */
int dpu_interrupt_irq_direct_ctrl(struct dpu_intr_context *ctx,
		enum dpu_intr_type type, bool enable);

/**
 * dpu_intr_pipe_irq_disable - close all irq in intr_type (used in cmd idle.)
 *
 * @ctx：the interrupt context pointer
 * @intr_type: indicate which pipe need to be enabled
 */
void dpu_intr_pipe_irq_disable(struct dpu_intr_context *ctx, enum dpu_intr_type intr_type);

/**
 * dpu_intr_pipe_irq_restore - restore all irq in intr_type (used in cmd idle.)
 *
 * @ctx：the interrupt context pointer
 * @intr_type: indicate which pipe need to be enabled
 */
void dpu_intr_pipe_irq_restore(struct dpu_intr_context *ctx, enum dpu_intr_type intr_type);

/**
 * dpu_interrupt_context_init - initialize the interrupt context
 * @ctx: the interrupt context pointer
 * @drm_dev: the drm device pointer
 *
 * Return: zero on success, -errno on failure
 */
int dpu_interrupt_context_init(struct dpu_intr_context **ctx,
		struct drm_device *drm_dev);

/**
 * dpu_interrupt_context_init - initialize the interrupt context
 * @ctx: the interrupt context pointer
 */
void dpu_interrupt_context_deinit(struct dpu_intr_context *ctx);

/**
 * dpu_intr_type_get - get the intr type of crtc
 * @crtc: drm crtc pointer
 * @type_out: intr type result output pointer
 *
 * Return: zero on success, -errno on failure
 */
int dpu_intr_type_get(struct drm_crtc *crtc, u32 *type_out);

#endif /* _DPU_INTR_CORE_H_ */
