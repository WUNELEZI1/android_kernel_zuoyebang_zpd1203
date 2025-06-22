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

#ifndef _DPU_HW_DVFS_H_
#define _DPU_HW_DVFS_H_

#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <drm/drm_atomic.h>

#include "dsi_tmg_hw.h"
#include "dpu_hw_glb.h"
#include "dpu_hw_dvs.h"

struct dpu_kms;

#define DPU_HW_DVFS_TRY_AGAIN_DELAY                      1000
#define DPU_HW_DVFS_MAX_ALLOWED_FAILED_TIMES             5

enum {
	DPU_HW_DVFS_ENABLED,
	DPU_HW_DVFS_DISABLED,
};

/**
 * dpu_hw_dvfs_mgr - dpu hw dvfs manager
 * @dpu_kms: the dpu kms pointer
 * @hw_glb: the dpu glb hw block pointer
 * @hw_dvs: the dvs hw block pointer
 * @avs_codes: the avs codes for hw dvfs
 * @status: current hw dvfs status
 * @dvfs_mutex: hw dvfs mutex to protect vote count
 * @vote_counter: current hw dvfs vote counter
 * @fail_counter: the hw dvfs module failed counter
 * @curr_profile: the current profile id which hdc module worked at
 * @hw_dvfs_work: used to do hw dvfs worked abnormal business
 * @hw_dvfs_wq: workqueue for hw dvfs do some work
 */
struct dpu_hw_dvfs_mgr {
	struct dpu_kms *dpu_kms;
	struct dpu_hw_glb *hw_glb;
	struct dpu_hw_dvs *hw_dvs;

	u8 avs_codes[DPU_PROFILE_MAX_NUM];
	u32 status;
	struct mutex dvfs_mutex;

	int vote_counter;
	u64 fail_counter;

	int curr_profile;

	struct work_struct hw_dvfs_work;
	struct workqueue_struct *hw_dvfs_wq;
};

/**
 * dpu_hw_dvfs_status_dump - dump hw dvfs work status
 * @dvfs_mgr: hw dvfs mgr pointer
 * @status: current hw dvfs status
 */
void dpu_hw_dvfs_status_dump(struct dpu_hw_dvfs_mgr *dvfs_mgr,
		struct hdc_status *status, int flag);

/**
 * dpu_hw_dvfs_dvs_status_dump - dump dvs hw states
 * @dvfs_mgr: hw dvfs mgr pointer
 * @cnt: counter for polling dvs arbit reg
 */
void dpu_hw_dvfs_dvs_status_dump(struct dpu_hw_dvfs_mgr *dvfs_mgr, int cnt);

/**
 * dpu_hw_dvfs_enable - vote hw dvfs enable
 * @dvfs_mgr: hw dvfs mgr pointer
 */
void dpu_hw_dvfs_enable(struct dpu_hw_dvfs_mgr *dvfs_mgr);

/**
 * dpu_hw_dvfs_disable - vote hw dvfs disable
 * @dvfs_mgr: hw dvfs mgr pointer
 */
void dpu_hw_dvfs_disable(struct dpu_hw_dvfs_mgr *dvfs_mgr);

/**
 * dpu_hw_dvfs_mgr_init - do hw dvfs mgr init
 * @dvfs_mgr: hw dvfs mgr pointer
 * @dpu_kms: the dpu kms pointer
 *
 * Returns: 0 success, others failed
 */
int dpu_hw_dvfs_mgr_init(struct dpu_hw_dvfs_mgr *dvfs_mgr,
		struct dpu_kms *dpu_kms);

/**
 * dpu_hw_dvfs_mgr_deinit - do hw dvfs mgr deinit
 * @dvfs_mgr: hw dvfs mgr pointer
 */
void dpu_hw_dvfs_mgr_deinit(struct dpu_hw_dvfs_mgr *dvfs_mgr);

#endif
