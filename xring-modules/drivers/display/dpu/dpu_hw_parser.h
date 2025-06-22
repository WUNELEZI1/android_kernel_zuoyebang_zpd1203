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

#ifndef _DPU_HW_PARSER_H_
#define _DPU_HW_PARSER_H_

#include <linux/slab.h>
#include <drm/drm_device.h>
#include "dpu_reg_ops.h"
#include "dpu_osal.h"

/* the maximum length of the base hardware name */
#define MAX_NAME_LEN 32

#define INVALID_BLK_ID -1

/**
 * dpu_hw_base - the capability base class
 * @name: the hardware module name
 * @id: the internal hardware module index
 * @addr: the relative address of hardware module
 * @len: the register length of hardware module
 * @features: the supported features of hardware module
 */
struct dpu_hw_base {
	char name[MAX_NAME_LEN];
	int id;
	u32 addr;
	u32 len;
	unsigned long features;
};

/* the index of dpu top */
enum dpu_top {
	DPU_TOP_NONE = -1,
	DPU_TOP_0 = 0,
	DPU_TOP_MAX
};

/**
 * dpu_top_cap - the structure of the dpu top capability
 * @base: the base class of dpu top hardware capability
 */
struct dpu_top_cap {
	struct dpu_hw_base base;
};

/* the index of dpu ctl_top */
enum dpu_ctl_top {
	CTL_TOP_NONE = -1,
	CTL_TOP_0 = 0,
	CTL_TOP_MAX
};

/**
 * dpu_ctl_top_cap - the structure of the ctl_top capability
 * @base: the base class of ctl_top hardware capability
 */
struct dpu_ctl_top_cap {
	struct dpu_hw_base base;
};

/* the index of dpu scene_ctl */
enum dpu_scene_ctl {
	SCENE_CTL_NONE = -1,
	SCENE_CTL_0 = 0,
	SCENE_CTL_1,
	SCENE_CTL_2,
	SCENE_CTL_MAX
};

/**
 * dpu_scene_ctl_cap - the structure of the scene_ctl capability
 * @base: the base class of scene_ctl hardware capability
 */
struct dpu_scene_ctl_cap {
	struct dpu_hw_base base;
};

/* the supported feature of dpu rch */
enum dpu_rch_feature {
	RCH_V_FULL,
	RCH_V_LITE,
	RCH_G,
};

/* the index of dpu rch_top */
enum dpu_rch_top {
	RCH_TOP_NONE = -1,
	RCH_TOP_0 = 0,
	RCH_TOP_1,
	RCH_TOP_2,
	RCH_TOP_3,
	RCH_TOP_4,
	RCH_TOP_5,
	RCH_TOP_6,
	RCH_TOP_7,
	RCH_TOP_8,
	RCH_TOP_9,
	RCH_TOP_MAX
};

/**
 * dpu_rch_top_cap - the structure of the rch_top capability
 * @base: the base class of rch_top hardware capability
 */
struct dpu_rch_top_cap {
	struct dpu_hw_base base;
};

/* the index of dpu rch */
enum dpu_rch {
	RCH_NONE = -1,
	RCH_G_0 = 0,
	RCH_V_0,
	RCH_G_1,
	RCH_V_1,
	RCH_G_2,
	RCH_G_3,
	RCH_G_4,
	RCH_V_2,
	RCH_V_3,
	RCH_G_5,
	RCH_MAX
};

/**
 * dpu_rch_cap - the structure of the rch capability
 * @base: the base class of rch hardware capability
 */
struct dpu_rch_cap {
	struct dpu_hw_base base;
};

#define PREPIPE_TOP_MAX RCH_MAX
/* the supported feature */
enum dpu_prepipe_top_feature {
	PREPIPE_TOP_LITE,
	PREPIPE_TOP_FULL
};

/**
 * dpu_prepipe_top_cap - the structure of the prepipe_top capability
 * @base: the base class of prepipe_top hardware capability
 */
struct dpu_prepipe_top_cap {
	struct dpu_hw_base base;
};

#define PRE_SCALER_MAX 4
/**
 * dpu_pre_scaler_cap - the structure of the pre_scaler capability
 * @base: the base class of pre_scaler hardware capability
 */
struct dpu_pre_scaler_cap {
	struct dpu_hw_base base;
};

#define TM_MAX RCH_MAX

/* the supported feature */
enum dpu_tm_feature {
	FEATURE_TM,
	FEATURE_TM_LITE,
};

/**
 * dpu_tm_cap - the structure of the tm capability
 * @base: the base class of tm hardware capability
 */
struct dpu_tm_cap {
	struct dpu_hw_base base;
};

#define PRE_LUT3D_MAX 1

/**
 * dpu_pre_lut3d_cap - the structure of the pre lut3d capability
 * @base: the base class of pre lut3d hardware capability
 */
struct dpu_pre_lut3d_cap {
	struct dpu_hw_base base;
};

#define PRE_HIST_MAX 1

/**
 * dpu_pre_hist_cap - the structure of the pre hist capability
 * @base: the base class of pre hist hardware capability
 */
struct dpu_pre_hist_cap {
	struct dpu_hw_base base;
};

/* the supported feature of dpu mixer */
enum dpu_mixer_feature {
	MIXER_PRIMARY,
	MIXER_SECONDARY,
	MIXER_WB
};

/* the feature of dpu wb_core */
enum dpu_wb_core_feature {
	WB_CORE_OFFLINE_0,
	WB_CORE_OFFLINE_1
};

/* the index of dpu mixer */
enum dpu_mixer {
	MIXER_NONE = -1,
	MIXER_0 = 0,
	MIXER_1,
	MIXER_2,
	MIXER_MAX
};

/**
 * dpu_mixer_cap - the structure of the mixer capability
 * @base: the base class of mixer hardware capability
 */
struct dpu_mixer_cap {
	struct dpu_hw_base base;
};

enum dpu_post_pipe_top {
	POST_PIPE_TOP_NONE = -1,
	POST_PIPE_TOP_0 = 0,
	POST_PIPE_TOP_1,
	POST_PIPE_TOP_MAX
};

/**
 * dpu_post_pipe_top_cap - the structure of the mixer capability
 * @base: the base class of post_pipe_top hardware capability
 */
struct dpu_post_pipe_top_cap {
	struct dpu_hw_base base;
};

enum dpu_post_pq_top {
	POST_PQ_TOP_NONE = -1,
	POST_PQ_TOP_0 = 0,
	POST_PQ_TOP_1,
	POST_PQ_TOP_MAX
};

/**
 * dpu_post_pq_top_cap - the structure of the mixer capability
 * @base: the base class of post_pq_top hardware capability
 */
struct dpu_post_pq_top_cap {
	struct dpu_hw_base base;
};

enum dpu_post_pre_proc {
	POST_PRE_PROC_NONE = -1,
	POST_PRE_PROC_0 = 0,
	POST_PRE_PROC_MAX
};

/**
 * dpu_post_pre_proc_cap - the structure of the mixer capability
 * @base: the base class of post_pre_proc hardware capability
 */
struct dpu_post_pre_proc_cap {
	struct dpu_hw_base base;
};

enum dpu_acad {
	ACAD_NONE = -1,
	ACAD_0 = 0,
	ACAD_MAX
};

/**
 * dpu_acad_cap - the structure of the mixer capability
 * @base: the base class of acad hardware capability
 */
struct dpu_acad_cap {
	struct dpu_hw_base base;
};

enum dpu_post_3dlut {
	POST_3DLUT_NONE = -1,
	POST_3DLUT_0 = 0,
	POST_3DLUT_MAX
};

/**
 * dpu_post_3dlut_cap - the structure of the mixer capability
 * @base: the base class of post lut3d hardware capability
 */
struct dpu_post_3dlut_cap {
	struct dpu_hw_base base;
};

enum dpu_post_gamma {
	POST_GAMMA_NONE = -1,
	POST_GAMMA_0 = 0,
	POST_GAMMA_1,
	POST_GAMMA_MAX
};

/**
 * dpu_post_gamma_cap - the structure of the mixer capability
 * @base: the base class of post gamma hardware capability
 */
struct dpu_post_gamma_cap {
	struct dpu_hw_base base;
};

enum dpu_post_hist {
	POST_HIST_NONE = -1,
	POST_HIST_0 = 0,
	POST_HIST_MAX
};

/**
 * dpu_post_hist_cap - the structure of the mixer capability
 * @base: the base class of post histogram hardware capability
 */
struct dpu_post_hist_cap {
	struct dpu_hw_base base;
};

enum dpu_post_scaler {
	POST_SCALER_NONE = -1,
	POST_SCALER_0 = 0,
	POST_SCALER_MAX
};

/**
 * dpu_post_scaler_cap - the structure of the mixer capability
 * @base: the base class of post scaler hardware capability
 */
struct dpu_post_scaler_cap {
	struct dpu_hw_base base;
};

/* the index of dpu dsc */
enum dpu_dsc {
	DSC_NONE = -1,
	DSC_0 = 0,
	DSC_1,
	DSC_MAX
};

/**
 * dpu_dsc_cap - the structure of the dsc capability
 * @base: the base class of dsc hardware capability
 */
struct dpu_dsc_cap {
	struct dpu_hw_base base;
};

/* the index of dpu dsc rdma */
enum dpu_dsc_rdma {
	DSC_RDMA_NONE = -1,
	DSC_RDMA_0 = 0,
	DSC_RDMA_1,
	DSC_RDMA_MAX
};

/**
 * dpu_dsc_rdma_cap - the structure of the dsc rdma capability
 * @base: the base class of dsc hardware capability
 */
struct dpu_dsc_rdma_cap {
	struct dpu_hw_base base;
};

/* the index of dpu dsc wdma */
enum dpu_dsc_wdma {
	DSC_WDMA_NONE = -1,
	DSC_WDMA_0 = 0,
	DSC_WDMA_1,
	DSC_WDMA_MAX
};

/**
 * dpu_dsc_wdma_cap - the structure of the dsc wdma capability
 * @base: the base class of dsc hardware capability
 */
struct dpu_dsc_wdma_cap {
	struct dpu_hw_base base;
};

/* the index of dpu wb_top */
enum dpu_wb_top {
	WB_TOP_NONE = -1,
	WB_TOP_0 = 0,
	WB_TOP_MAX
};

/**
 * dpu_wb_top_cap - the structure of the wb_top capability
 * @base: the base class of wb_top hardware capability
 */
struct dpu_wb_top_cap {
	struct dpu_hw_base base;
};

/* the index of dpu wb_core */
enum dpu_wb_core {
	WB_CORE_NONE = -1,
	WB_CORE_0 = 0,
	WB_CORE_1,
	WB_CORE_MAX
};

/**
 * dpu_wb_core_cap - the structure of the wb_core capability
 * @base: the base class of wb_core hardware capability
 */
struct dpu_wb_core_cap {
	struct dpu_hw_base base;
};

/* the index of dpu wb_scaler */
enum dpu_wb_scaler {
	WB_SCALER_NONE = -1,
	WB_SCALER_0 = 0,
	WB_SCALER_1,
	WB_SCALER_MAX
};

/**
 * dpu_wb_scaler_cap - the structure of the wb_scaler capability
 * @base: the base class of wb_scaler hardware capability
 */
struct dpu_wb_scaler_cap {
	struct dpu_hw_base base;
};

/* the index of dpu interrupt */
enum dpu_intr {
	DPU_INTR_NONE = -1,
	DPU_INTR_0 = 0,
	DPU_INTR_MAX
};

/**
 * dpu_intr_cap - the structure of the dpu interrupts capability
 * @base: the base class of dpu interrupts hardware capability
 */
struct dpu_intr_cap {
	struct dpu_hw_base base;
};

/* the index of dpu glb */
enum dpu_glb {
	DPU_GLB_NONE = -1,
	DPU_GLB_0 = 0,
	DPU_GLB_MAX
};

/**
 * dpu_glb_cap - the structure of the dpu glb setting capability
 * @base: the base class of dpu glb hardware capability
 */
struct dpu_glb_cap {
	struct dpu_hw_base base;
};

/**
 * dpu_iomem - the structure of mapped dpu address
 * @base: the mapped basic address of dpu
 * @addr: the physical dpu address value for checking purpose
 * @len: the length of dpu memory space
 */
struct dpu_iomem {
	DPU_IOMEM base;
	u32 addr;
	u32 len;
};

/**
 * dpu_cap - dpu hardware capability structure
 * @dpu_mem: the dpu memory info
 * comments for other members are placed inside the structure
 */
struct dpu_cap {
	struct dpu_iomem dpu_mem;

	/**
	 * @dpu_top_count: the number of dpu top blocks
	 * @dpu_top: the rch resources info
	 */
	u32 dpu_top_count;
	struct dpu_top_cap dpu_top[DPU_TOP_MAX];

	/**
	 * @ctl_top_count: the number of ctl_top blocks
	 * @ctl_top: the ctl_top resources info
	 */
	u32 ctl_top_count;
	struct dpu_ctl_top_cap ctl_top[CTL_TOP_MAX];

	/**
	 * @scene_ctl_count: the number of scene_ctl blocks
	 * @scene_ctl: scene_ctl rch resources info
	 */
	u32 scene_ctl_count;
	struct dpu_scene_ctl_cap scene_ctl[SCENE_CTL_MAX];

	/**
	 * @rch_top_count: the number of rch_top blocks
	 * @rch_top: the rch_top resources info
	 */
	u32 rch_top_count;
	struct dpu_rch_top_cap rch_top[RCH_TOP_MAX];

	/**
	 * @rch_count: the number of rch blocks
	 * @rch: the rch resources info
	 */
	u32 rch_count;
	struct dpu_rch_cap rch[RCH_MAX];

	/**
	 * @prepipe_top_count: the number of prepipe_top blocks
	 * @prepipe_top: the prepipe_top resources info
	 */
	u32 prepipe_top_count;
	struct dpu_prepipe_top_cap prepipe_top[PREPIPE_TOP_MAX];

	/**
	 * @pre_scaler_count: the number of pre_scaler blocks
	 * @pre_scaler: the pre_scaler resources info
	 */
	u32 pre_scaler_count;
	struct dpu_pre_scaler_cap pre_scaler[PRE_SCALER_MAX];

	/**
	 * @tm_count: the number of tm blocks
	 * @tm: the tm resources info
	 */
	u32 tm_count;
	struct dpu_tm_cap tm[TM_MAX];

	/**
	 * @pre_lut3d_count: the number of pre_lut3d blocks
	 * @pre_lut3d: the pre_lut3d resources info
	 */
	u32 pre_lut3d_count;
	struct dpu_pre_lut3d_cap pre_lut3d[PRE_LUT3D_MAX];

	/**
	 * @pre_hist_count: the number of pre_hist blocks
	 * @pre_hist: the pre_hist resources info
	 */
	u32 pre_hist_count;
	struct dpu_pre_hist_cap pre_hist[PRE_HIST_MAX];

	/**
	 * @mixer_count: the number of mixer blocks
	 * @mixer: the rch resources info
	 */
	u32 mixer_count;
	struct dpu_mixer_cap mixer[MIXER_MAX];

	/**
	 * @post_pipe_top_count: the number of post_pipe_top blocks
	 * @post_pipe_top: the post_pipe_top resources info
	 */
	u32 post_pipe_top_count;
	struct dpu_post_pipe_top_cap post_pipe_top[POST_PIPE_TOP_MAX];

	/**
	 * @post_pq_top_count: the number of post_pq_top blocks
	 * @post_pq_top: the post_pq_top resources info
	 */
	u32 post_pq_top_count;
	struct dpu_post_pq_top_cap post_pq_top[POST_PQ_TOP_MAX];

	/**
	 * @post_pre_proc_count: the number of post_pre_proc blocks
	 * @post_pre_proc: the post_pre_proc resources info
	 */
	u32 post_pre_proc_count;
	struct dpu_post_pre_proc_cap post_pre_proc[POST_PRE_PROC_MAX];

	/**
	 * @acad_count: the number of acad blocks
	 * @acad: the acad resources info
	 */
	u32 acad_count;
	struct dpu_acad_cap acad[ACAD_MAX];

	/**
	 * @post_3dlut_count: the number of post_3dlut blocks
	 * @post_3dlut: the post_3dlut resources info
	 */
	u32 post_3dlut_count;
	struct dpu_post_3dlut_cap post_3dlut[POST_3DLUT_MAX];

	/**
	 * @post_gamma_count: the number of post_gamma blocks
	 * @post_gamma: the post_gamma resources info
	 */
	u32 post_gamma_count;
	struct dpu_post_gamma_cap post_gamma[POST_GAMMA_MAX];

	/**
	 * @post_hist_count: the number of post_hist blocks
	 * @post_hist: the post_hist resources info
	 */
	u32 post_hist_count;
	struct dpu_post_hist_cap post_hist[POST_HIST_MAX];

	/**
	 * @post_scaler_count: the number of post_scaler blocks
	 * @post_scaler: the post_scaler resources info
	 */
	u32 post_scaler_count;
	struct dpu_post_scaler_cap post_scaler[POST_SCALER_MAX];

	/**
	 * @dsc_count: the number of dsc blocks
	 * @dsc: the dsc resources info
	 */
	u32 dsc_count;
	struct dpu_dsc_cap dsc[DSC_MAX];

	/**
	 * @dsc_wdma_count: the number of dsc wdma blocks
	 * @dsc_wdma: the dsc wdma resources info
	 */
	u32 dsc_wdma_count;
	struct dpu_dsc_wdma_cap dsc_wdma[DSC_WDMA_MAX];

	/**
	 * @dsc_rdma_count: the number of dsc rdma blocks
	 * @dsc_rdma: the dsc rdama resources info
	 */
	u32 dsc_rdma_count;
	struct dpu_dsc_rdma_cap dsc_rdma[DSC_RDMA_MAX];

	/**
	 * @wb_top_count: the number of wb_top blocks
	 * @wb_top: the wb_top resources info
	 */
	u32 wb_top_count;
	struct dpu_wb_top_cap wb_top[WB_TOP_MAX];

	/**
	 * @wb_core_count: the number of wb_core blocks
	 * @wb_core: the wb_core resources info
	 */
	u32 wb_core_count;
	struct dpu_wb_core_cap wb_core[WB_CORE_MAX];

	/**
	 * @wb_scaler_count: the number of wb_core blocks
	 * @wb_scaler: the wb_core resources info
	 */
	u32 wb_scaler_count;
	struct dpu_wb_scaler_cap wb_scaler[WB_SCALER_MAX];

	/**
	 * @dpu_intr_count: the number of dpu_intr blocks
	 * @dpu_intr: the dpu_intr resources info
	 */
	u32 dpu_intr_count;
	struct dpu_intr_cap dpu_intr[DPU_INTR_MAX];

	/**
	 * @dpu_glb_count;
	 * @dpu_glb: the dpu glb's resources info
	 */
	u32 dpu_glb_count;
	struct dpu_glb_cap dpu_glb[DPU_GLB_MAX];
};

/**
 * dpu_cap_init - initialize all dpu hardware capability
 * @dev: drm device pointer
 * @dpu_cap: hardware capability pointer for storing the parsed data
 *
 * Return: error code on failure, 0 on success
 */
int dpu_cap_init(struct drm_device *dev, struct dpu_cap *dpu_cap);

#endif /* _DPU_HW_PARSER_H_ */
