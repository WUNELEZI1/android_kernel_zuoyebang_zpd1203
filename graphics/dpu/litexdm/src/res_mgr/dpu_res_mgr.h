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

#ifndef DPU_RES_MGR_H
#define DPU_RES_MGR_H

#include "dpu_list.h"
#include "dpu_hw_res_parser.h"
#include "dpu_hw_common.h"

#include "dpu_hw_intr.h"
#include "dpu_hw_ctl_top.h"
#include "dpu_hw_scene_ctl.h"
#include "dpu_hw_rch_top.h"
#include "dpu_hw_rch.h"
#include "dpu_hw_mixer.h"
#include "color/dpu_hw_prepipe_top.h"
#include "color/dpu_hw_pre_scaler.h"
#include "color/dpu_hw_tm.h"
#include "color/dpu_hw_pre_hist.h"
#include "color/dpu_hw_pre_3dlut.h"
#include "color/dpu_hw_post_gamma.h"
#include "color/dpu_hw_post_3dlut.h"
#include "color/dpu_hw_postpq_top.h"
#include "color/dpu_hw_postpre_proc.h"
#include "color/dpu_hw_post_acad.h"
#include "color/dpu_hw_post_hist.h"
#include "color/dpu_hw_post_scaler.h"
#include "dpu_hw_post_pipe_top.h"
#include "dpu_hw_dsc.h"
#include "dpu_hw_wb_top.h"
#include "dpu_hw_wb.h"
#include "dpu_hw_wb_scaler.h"

enum DPU_BLK_TYPE {
	BLK_NONE = 0x0,
	BLK_DPU_TOP,
	BLK_CTL_TOP,
	BLK_SCENE_CTL,
	BLK_RCH_TOP,
	BLK_RCH,
	BLK_PREPIPE_TOP,
	BLK_TM,
	BLK_PRE_SCALER,
	BLK_PRE_HIST,
	BLK_PRE_3DLUT,
	BLK_MIXER,
	BLK_POST_PIPE_TOP,
	BLK_POST_PQ_TOP,
	BLK_POST_PRE_PROC,
	BLK_ACAD,
	BLK_POST_3DLUT,
	BLK_POST_GAMMA,
	BLK_POST_HIST,
	BLK_POST_SCALER,
	BLK_DSC,
	BLK_DSC_WDMA,
	BLK_DSC_RDMA,
	BLK_WB_TOP,
	BLK_WB,
	BLK_WB_SCALER,
	BLK_DPU_INTR,
	BLK_MAX
};

struct dpu_res_blk {
	struct list_head list;
	enum DPU_BLK_TYPE blk_type;
	struct dpu_hw_blk *hw;
};

struct dpu_res_mgr_ctx {
	struct dpu_cap dpu_cap;
	struct list_head free_pool;
};

/**
 * get_hw_blk - Get the hw blk object
 *
 * @blk_type: DPU_BLK_TYPE
 * @blk_id: blk id
 * @return hw object
 */
struct dpu_hw_blk *get_hw_blk(enum DPU_BLK_TYPE blk_type, uint32_t blk_id);

/**
 * dpu_res_mgr_init - init res mgr
 *
 * @return 0 on success, -1 on failure
 */
int32_t dpu_res_mgr_init(void);

#endif
