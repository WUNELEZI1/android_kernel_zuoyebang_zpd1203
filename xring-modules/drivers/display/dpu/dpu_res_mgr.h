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

#ifndef _DPU_RES_MGR_H_
#define _DPU_RES_MGR_H_

#include <linux/list.h>
#include <linux/kref.h>
#include <linux/mutex.h>
#include "dpu_cap.h"
#include "dpu_hw_parser.h"
#include "dpu_hw_intr.h"
#include "dpu_hw_top.h"
#include "dpu_hw_ctl_top.h"
#include "dpu_hw_scene_ctl.h"
#include "dpu_hw_rch_top.h"
#include "dpu_hw_rch.h"
#include "dpu_hw_prepipe_top.h"
#include "dpu_hw_pre_scaler.h"
#include "dpu_hw_tm.h"
#include "dpu_hw_pre_hist.h"
#include "dpu_hw_pre_3dlut.h"
#include "dpu_hw_mixer.h"
#include "dpu_hw_post_gamma.h"
#include "dpu_hw_post_3dlut.h"
#include "dpu_hw_postpq_top.h"
#include "dpu_hw_postpre_proc.h"
#include "dpu_hw_post_acad.h"
#include "dpu_hw_post_hist.h"
#include "dpu_hw_post_scaler.h"
#include "dpu_hw_post_pipe_top.h"
#include "dpu_hw_dsc.h"
#include "dpu_hw_dsc_wdma.h"
#include "dpu_hw_dsc_rdma.h"
#include "dpu_hw_wb_top.h"
#include "dpu_hw_wb.h"
#include "dpu_hw_wb_scaler.h"
#include "dpu_hw_glb.h"

/* the hardware block type which supported by resource manager */
enum dpu_blk_type {
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
	BLK_DPU_GLB,
	BLK_MAX
};

/**
 * dpu_res_blk - the resource block info for one hardware block,
 *                 it is the basic unit of resource manager
 * @list: the list head of this block
 * @refcount: the reference count of this block
 * @blk_type: the type of this block
 * @hw: the hardware block pointer
 */
struct dpu_res_blk {
	struct list_head list;

	atomic_t refcount;
	enum dpu_blk_type blk_type;
	struct dpu_hw_blk *hw;
};

/**
 * dpu_res_chain - the all resources of one pipeline
 * @chain_list: the list head for managing the resources which on this chain
 * @pipe_id: the pipe id of this resource chain
 */
struct dpu_res_chain {
	struct list_head chain_list;

	int pipe_id;
};

/**
 * dpu_res_mgr_ctx - the resource manager context
 * @free_pool: the list head of free resources pool
 * @res_lock: context resource lock
 * @res_chain: the per pipeline resources
 * @rch_blk: rch hw blk for dma dbg irq
 * @dpu_cap: the hardware resources and capabilities
 */
struct dpu_res_mgr_ctx {
	struct list_head free_pool;

	struct mutex res_lock;
	struct dpu_res_chain res_chain[MAX_RES_PIPELINE_COUNT];

	struct dpu_hw_blk *rch_hw_blk[RCH_MAX];

	struct dpu_cap dpu_cap;
};

static inline struct dpu_res_blk *to_dpu_res_blk(struct list_head *list)
{
	return container_of(list, struct dpu_res_blk, list);
}

/**
 * dpu_res_mgr_context_init - init resource manager context
 * @ctx: [output param], the context pointer for allocating
 *       & initializing the resource manager context
 * @drm_dev: the drm device pointer
 *
 * Return: 0 on success, error code on failure
 */
int dpu_res_mgr_context_init(struct dpu_res_mgr_ctx **ctx,
		struct drm_device *drm_dev);

/**
 * dpu_res_mgr_context_cleanup - cleanup the resource manager context
 * @ctx: the resource manager context pointer
 */
void dpu_res_mgr_context_cleanup(struct dpu_res_mgr_ctx *ctx);

/**
 * dpu_res_mgr_block_reserve_feature - reserve a resource block with feature
 * @ctx: the resource manager context pointer
 * @pipe_id: the pipeline id
 * @blk_type: the block type which need to be reserved
 * @blk_feature: the block feature for reserving the specific block
 * @test_only: test the block of the given type can be reserved or not
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_res_mgr_block_reserve_feature(
		struct dpu_res_mgr_ctx *ctx, int pipe_id,
		enum dpu_blk_type blk_type, unsigned long blk_feature,
		bool test_only);

/**
 * dpu_res_mgr_block_reserve_id - reserve a resource block with id
 * @ctx: the resource manager context pointer
 * @pipe_id: the pipeline id
 * @blk_type: the block type which need to be reserved
 * @blk_id: the block id for reserving the specific block
 * @test_only: test the block of the given type can be reserved or not
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_res_mgr_block_reserve_id(
		struct dpu_res_mgr_ctx *ctx, int pipe_id,
		enum dpu_blk_type blk_type, u32 blk_id,
		bool test_only);

/**
 * dpu_res_mgr_block_reserve - reserve a resource block from free block pool
 * @ctx: the resource manager context pointer
 * @pipe_id: the pipeline id
 * @blk_type: the block type which need to be reserved
 * @test_only: test the block of the given type can be reserved or not
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_res_mgr_block_reserve(
		struct dpu_res_mgr_ctx *ctx, int pipe_id,
		enum dpu_blk_type blk_type, bool test_only);

/**
 * dpu_res_mgr_block_release - release a resource block to free block pool
 *
 * @ctx: the resource manager context pointer
 * @hw_blk: the hw block pointer which need to be released
 */
void dpu_res_mgr_block_release(struct dpu_res_mgr_ctx *ctx,
		struct dpu_hw_blk *hw_blk);

#endif /* _DPU_RES_MGR_H_ */
