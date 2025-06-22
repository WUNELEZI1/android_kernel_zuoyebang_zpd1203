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

#ifndef _DPU_EXCEPTION_H_
#define _DPU_EXCEPTION_H_

#include "dpu_hw_parser.h"
#include "dpu_intr_core.h"
#include "dpu_kms.h"

struct dpu_kms;
struct drm_crtc;
struct dpu_intr_context;
struct drm_crtc_state;

#define DUMP_RCH_CNT 1
#define DUMP_DMA_BUF_FILE_MAX_NUM   1000

enum dump_frame {
	PREV_FRAME = 0,
	CURR_FRAME = 1,
};

enum dpu_hw_cfg_dump_type {
	NORMAL_DUMP = 0,
	UNDERFLOW_DUMP,
	RECOVERY_DUMP,
};

static inline char *get_dump_file_suffix(enum dpu_hw_cfg_dump_type type)
{
	char *str;

	switch (type) {
	case NORMAL_DUMP:
		str = "normal_dump";
		break;
	case UNDERFLOW_DUMP:
		str = "underflow_dump";
		break;
	case RECOVERY_DUMP:
		str = "recovery_dump";
		break;
	default:
		str = "unknow_dump";
		break;
	}

	return str;
}

struct dpu_dbg_irq_work {
	struct dpu_kms *dpu_kms;
	struct drm_crtc *crtc;
	struct work_struct work;
	u32 wdma_dbg_irq_status;
	u32 rch_dbg_irq_status[RCH_MAX];
	u32 plsverr_status;
	u32 cmdlist_ch_clr_timeout_status;

	bool complete_print;
	u32 old_rch_mask;
	u32 new_rch_mask;
};

enum dpu_dma_buf_debug_type {
	DPU_DMA_BUF_DEBUG_NONE = 0,
	DPU_DMA_BUF_BIT_CHECK_AFTER_ACQUIRE_FENCE = BIT(0),
	DPU_DMA_BUF_DUMP_TO_FILE_WITH_AFBC_DEC_ERR = BIT(1),
};

struct dpu_dbg_ctx {
	struct dpu_dbg_irq_work work_data[DPU_INTR_TYPE_MAX];
	struct workqueue_struct *wq[DPU_INTR_TYPE_MAX];
	struct dpu_hw_blk *rch_hw_blk[RCH_MAX];

	/* bit mask, see enum dpu_dma_buf_debug_type */
	u32 dma_buf_debug_type;

	void *input_buf[2][MAX_PLANE_ZPOS + 1];

	bool need_dump_buf;
	u32  need_dump_rch_id[RCH_MAX];
};

/**
 * dpu_freeze_current_frame - freeze current frame's setting
 * and skip all subsequent frame's configure
 * @dpu_kms: point of dpu_kms
 */
void dpu_freeze_current_frame(struct dpu_kms *dpu_kms);

/**
 * dpu_exception_recovery - do sense ctrl abnormal recovery
 * @dpu_kms: point of dpu_kms
 * @crtc_state: point of drm crtc state
 */
void dpu_exception_recovery(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *crtc_state);

/**
 * dpu_hw_cfg_dump - dump dpu hw cfg and hw status to dmesg and files
 * @pipeline: point of dpu virt pipeline
 * @crtc_state: point of crtc state
 * @type: dpu hw cfg dump type
 */
void dpu_hw_cfg_dump(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *crtc_state,
		enum dpu_hw_cfg_dump_type type);

/**
 * dpu_core_perf_dump - dump core perf status
 */
void dpu_core_perf_dump(struct dpu_kms *dpu_kms);

/**
 * dpu_ddr_dvfs_ok_dump - dump ddr dvfs ok status
 */
void dpu_ddr_dvfs_ok_dump(void);

/**
 * dpu_exception_online_dbg_irq_register
 * @intr_ctx: pointer of dpu_intr_context
 */
void dpu_exception_online_dbg_irq_register(struct dpu_intr_context *intr_ctx);

/**
 * dpu_exception_cmb_dbg_irq_register
 * @intr_ctx: pointer of dpu_intr_context
 */
void dpu_exception_cmb_dbg_irq_register(struct dpu_intr_context *intr_ctx);

/**
 * dpu_exception_offline_dbg_irq_register
 * @intr_ctx: pointer of dpu_intr_context
 */
void dpu_exception_offline_dbg_irq_register(struct dpu_intr_context *intr_ctx);

/**
 * dpu_exception_online_dbg_irq_unregister
 * @intr_ctx: pointer of dpu_intr_context
 */
void dpu_exception_online_dbg_irq_unregister(struct dpu_intr_context *intr_ctx);

/**
 * dpu_exception_cmb_dbg_irq_unregister
 * @intr_ctx: pointer of dpu_intr_context
 */
void dpu_exception_cmb_dbg_irq_unregister(struct dpu_intr_context *intr_ctx);

/**
 * dpu_exception_offline_dbg_irq_unregister
 * @intr_ctx: pointer of dpu_intr_context
 */
void dpu_exception_offline_dbg_irq_unregister(struct dpu_intr_context *intr_ctx);

/**
 * dpu_exception_dbg_irq_enable
 * @dpu_kms: point of dpu_kms
 * @crtc: point of drm crtc
 */
void dpu_exception_dbg_irq_enable(struct dpu_kms *dpu_kms,
		struct drm_crtc *crtc);

/**
 * dpu_exception_dbg_irq_clear
 * @pipeline: point of dpu virt pipeline
 * @crtc: point of drm crtc
 */
void dpu_exception_dbg_irq_clear(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc *crtc);

/**
 * dpu_exception_dbg_ctx_init
 * @dpu_kms: point of dpu_kms
 */
int dpu_exception_dbg_ctx_init(struct dpu_kms *dpu_kms);

/**
 * dpu_exception_dbg_ctx_deinit
 * @dpu_kms: point of dpu_kms
 */
void dpu_exception_dbg_ctx_deinit(struct dpu_kms *dpu_kms);

/**
 * dpu_dbg_flush_workqueue - flush dbg workqueue
 * @ctx: the dpu dbg ctx
 * @crtc: the pointer of drm crtc
 */
void dpu_dbg_flush_workqueue(struct dpu_dbg_ctx *ctx,
		struct drm_crtc *crtc);

/**
 * dpu_acquire_fences_debug - check dma buf is stabe or not
 * @commit_ctx: the virt pipeline commit ctx
 * @state: the atomic state
 */
void dpu_acquire_fences_debug(struct dpu_commit_ctx *commit_ctx,
		struct drm_atomic_state *state);

#endif
