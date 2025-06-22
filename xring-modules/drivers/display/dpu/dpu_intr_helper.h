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
/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved. */

#ifndef _DPU_INTR_HELPER_H_
#define _DPU_INTR_HELPER_H_

#include <linux/atomic.h>
#include <drm/drm_vblank.h>
#include <drm/drm_crtc.h>
#include <drm/drm_connector.h>

struct dpu_virt_pipeline;
struct dpu_intr_pipe;
struct dpu_intr_context;

/* make sure that the intr type order is consistent with the order in which the hardware occurs */
enum dpu_online_conn_intr_type {
	DPU_UNDERFLOW_INT = 0,
	DPU_EOF_INT,
	DPU_CFG_WIN_START_INT,
	DPU_CFG_RDY_CLR_INT,
	DPU_CFG_WIN_END_INT,
	DPU_VSYNC_INT,
	DPU_VSTART_INT,
	DPU_CONN_INTR_TYPE_MAX_NUM,
};

/**
 * dpu_connector_intr_context - dpu connector interrupt context
 * @intr_id: the interrupt id for each intr type
 * @enable: the enable status for each intr type
 */
struct dpu_connector_intr_context {
	u32 intr_id[DPU_CONN_INTR_TYPE_MAX_NUM];
	bool enable[DPU_CONN_INTR_TYPE_MAX_NUM];
};

#define INIT_ONLINE_CONN_INTR(ctx, intr_type, conn_intr_type) \
{\
	ctx->intr_id[conn_intr_type] = get_online_intr_id(intr_type, conn_intr_type); \
	ctx->enable[conn_intr_type] = true; \
} \

void dpu_online_irq_status_dump(struct dpu_intr_context *intr_ctx,
		struct dpu_connector_intr_context *conn_intr_ctx);

void dpu_intr_online_cfg_rdy_clr_handler(struct dpu_virt_pipeline *pipeline);
void dpu_intr_online_vsync_handler(struct dpu_virt_pipeline *pipeline, u64 timestamp);
void dpu_intr_online_vstart_handler(struct dpu_virt_pipeline *pipeline);
void dpu_intr_online_underflow_handler(struct dpu_intr_context *intr_ctx,
		struct dpu_virt_pipeline *pipeline,
		int intr_id);
u32 get_online_intr_id(u32 intr_type, enum dpu_online_conn_intr_type conn_intr_type);

/**
 * dpu_intr_online_irq_register - register online connector irqs
 * @connector: the pointer of connector
 * @conn_intr_ctx: the pointer of connecotr intr ctx
 * @cb: the main dispatcher for those online connector irqs
 *
 * Return: zero on success, -errno on failure
 */
int dpu_intr_online_irq_register(struct drm_connector *connector,
		struct dpu_connector_intr_context *conn_intr_ctx,
		void (*cb)(int intr_id, void *data));

/**
 * dpu_intr_online_irq_unregister - register online connector irqs
 * @connector: the pointer of connector
 * @conn_intr_ctx: the pointer of connecotr intr ctx
 */
void dpu_intr_online_irq_unregister(struct drm_connector *connector,
		struct dpu_connector_intr_context *conn_intr_ctx);

#endif
