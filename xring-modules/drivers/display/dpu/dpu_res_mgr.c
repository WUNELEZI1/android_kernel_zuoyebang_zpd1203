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

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "dpu_log.h"
#include "dpu_res_mgr.h"

struct dpu_blk_type_str {
	enum dpu_blk_type type;
	char *type_str;
};

/* block type string for converting enum to string */
static struct dpu_blk_type_str blk_type_str[] = {
	{BLK_NONE, "blk_no_defined"},
	{BLK_DPU_TOP, "blk_dpu_top"},
	{BLK_CTL_TOP, "blk_ctl_top"},
	{BLK_SCENE_CTL, "blk_scene_ctl"},
	{BLK_RCH_TOP, "blk_rch_top"},
	{BLK_RCH, "blk_rch"},
	{BLK_PREPIPE_TOP, "blk_prepipe_top"},
	{BLK_PRE_SCALER, "blk_pre_scaler"},
	{BLK_TM, "blk_tm"},
	{BLK_PRE_3DLUT, "blk_pre_3dlut"},
	{BLK_PRE_HIST, "blk_pre_hist"},
	{BLK_MIXER, "blk_mixer"},
	{BLK_POST_PIPE_TOP, "blk_post_pipe_top"},
	{BLK_POST_PQ_TOP, "blk_post_pq_top"},
	{BLK_POST_PRE_PROC, "blk_post_pre_proc"},
	{BLK_ACAD, "blk_acad"},
	{BLK_POST_3DLUT, "blk_post_3dlut"},
	{BLK_POST_GAMMA, "blk_gamma"},
	{BLK_POST_HIST, "blk_hist"},
	{BLK_POST_SCALER, "blk_scaler"},
	{BLK_DSC, "blk_dsc"},
	{BLK_DSC_WDMA, "blk_dsc_wdma"},
	{BLK_DSC_RDMA, "blk_dsc_rdma"},
	{BLK_WB_TOP, "blk_wb_top"},
	{BLK_WB, "blk_wb"},
	{BLK_WB_SCALER, "blk_wb_scaler"},
	{BLK_DPU_INTR, "blk_dpu_intr"},
	{BLK_DPU_GLB, "blk_dpu_glb"},
};

const char *_blk_type_to_str(enum dpu_blk_type type)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(blk_type_str); i++)
		if (type == blk_type_str[i].type)
			return blk_type_str[i].type_str;

	return blk_type_str[0].type_str;
}

static void dpu_res_mgr_dpu_cap_init(struct dpu_cap *dpu_cap)
{
	int i;

	for (i = 0; i < DPU_TOP_MAX; i++)
		dpu_cap->dpu_top[i].base.id = DPU_TOP_NONE;

	for (i = 0; i < CTL_TOP_MAX; i++)
		dpu_cap->ctl_top[i].base.id = CTL_TOP_NONE;

	for (i = 0; i < SCENE_CTL_MAX; i++)
		dpu_cap->scene_ctl[i].base.id = SCENE_CTL_NONE;

	for (i = 0; i < RCH_TOP_MAX; i++)
		dpu_cap->rch_top[i].base.id = RCH_TOP_NONE;

	for (i = 0; i < RCH_MAX; i++)
		dpu_cap->rch[i].base.id = RCH_NONE;

	for (i = 0; i < PREPIPE_TOP_MAX; i++)
		dpu_cap->prepipe_top[i].base.id = RCH_NONE;

	for (i = 0; i < PRE_SCALER_MAX; i++)
		dpu_cap->pre_scaler[i].base.id = RCH_NONE;

	for (i = 0; i < TM_MAX; i++)
		dpu_cap->tm[i].base.id = RCH_NONE;

	for (i = 0; i < PRE_LUT3D_MAX; i++)
		dpu_cap->pre_lut3d[i].base.id = RCH_NONE;

	for (i = 0; i < PRE_HIST_MAX; i++)
		dpu_cap->pre_hist[i].base.id = RCH_NONE;

	for (i = 0; i < MIXER_MAX; i++)
		dpu_cap->mixer[i].base.id = MIXER_NONE;

	for (i = 0; i < POST_PIPE_TOP_MAX; i++)
		dpu_cap->post_pipe_top[i].base.id = POST_PIPE_TOP_NONE;

	for (i = 0; i < POST_PQ_TOP_MAX; i++)
		dpu_cap->post_pq_top[i].base.id = POST_PQ_TOP_NONE;

	for (i = 0; i < POST_PRE_PROC_MAX; i++)
		dpu_cap->post_pre_proc[i].base.id = POST_PRE_PROC_NONE;

	for (i = 0; i < ACAD_MAX; i++)
		dpu_cap->acad[i].base.id = ACAD_NONE;

	for (i = 0; i < POST_3DLUT_MAX; i++)
		dpu_cap->post_3dlut[i].base.id = POST_3DLUT_NONE;

	for (i = 0; i < POST_GAMMA_MAX; i++)
		dpu_cap->post_gamma[i].base.id = POST_GAMMA_NONE;

	for (i = 0; i < POST_HIST_MAX; i++)
		dpu_cap->post_hist[i].base.id = POST_HIST_NONE;

	for (i = 0; i < POST_SCALER_MAX; i++)
		dpu_cap->post_scaler[i].base.id = POST_SCALER_NONE;

	for (i = 0; i < DSC_MAX; i++)
		dpu_cap->dsc[i].base.id = DSC_NONE;

	for (i = 0; i < DSC_WDMA_MAX; i++)
		dpu_cap->dsc_wdma[i].base.id = DSC_WDMA_NONE;

	for (i = 0; i < DSC_RDMA_MAX; i++)
		dpu_cap->dsc_rdma[i].base.id = DSC_RDMA_NONE;

	for (i = 0; i < WB_TOP_MAX; i++)
		dpu_cap->wb_top[i].base.id = WB_TOP_NONE;

	for (i = 0; i < WB_CORE_MAX; i++)
		dpu_cap->wb_core[i].base.id = WB_CORE_NONE;

	for (i = 0; i < WB_SCALER_MAX; i++)
		dpu_cap->wb_scaler[i].base.id = WB_SCALER_NONE;

	for (i = 0; i < DPU_INTR_MAX; i++)
		dpu_cap->dpu_intr[i].base.id = DPU_INTR_NONE;
}

static struct dpu_hw_blk *dpu_res_mgr_hw_blk_init(
		struct dpu_cap *dpu_cap, u32 cap_idx,
		enum dpu_blk_type blk_type)
{
	struct dpu_hw_blk *hw_blk_ptr;

	switch (blk_type) {
	case BLK_DPU_TOP:
		hw_blk_ptr = dpu_hw_dpu_top_init(&dpu_cap->dpu_top[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_CTL_TOP:
		hw_blk_ptr = dpu_hw_ctl_top_init(&dpu_cap->ctl_top[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_SCENE_CTL:
		hw_blk_ptr = dpu_hw_scene_ctl_init(&dpu_cap->scene_ctl[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_RCH_TOP:
		hw_blk_ptr = dpu_hw_rch_top_init(&dpu_cap->rch_top[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_RCH:
		hw_blk_ptr = dpu_hw_rch_init(&dpu_cap->rch[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_PREPIPE_TOP:
		hw_blk_ptr = dpu_hw_prepipe_top_init(&dpu_cap->prepipe_top[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_PRE_SCALER:
		hw_blk_ptr = dpu_hw_pre_scaler_init(&dpu_cap->pre_scaler[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_TM:
		hw_blk_ptr = dpu_hw_tm_init(&dpu_cap->tm[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_PRE_HIST:
		hw_blk_ptr = dpu_hw_pre_hist_init(&dpu_cap->pre_hist[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_PRE_3DLUT:
		hw_blk_ptr = dpu_hw_pre_3dlut_init(&dpu_cap->pre_lut3d[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_MIXER:
		hw_blk_ptr = dpu_hw_mixer_init(&dpu_cap->mixer[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_POST_PIPE_TOP:
		hw_blk_ptr = dpu_hw_post_pipe_top_init(&dpu_cap->post_pipe_top[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_POST_GAMMA:
		hw_blk_ptr = dpu_hw_post_gamma_init(&dpu_cap->post_gamma[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_POST_PQ_TOP:
		hw_blk_ptr = dpu_hw_postpq_top_init(&dpu_cap->post_pq_top[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_POST_3DLUT:
		hw_blk_ptr = dpu_hw_post_3dlut_init(&dpu_cap->post_3dlut[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_POST_PRE_PROC:
		hw_blk_ptr = dpu_hw_postpre_proc_init(&dpu_cap->post_pre_proc[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_ACAD:
		hw_blk_ptr = dpu_hw_post_acad_init(&dpu_cap->acad[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_POST_HIST:
		hw_blk_ptr = dpu_hw_post_hist_init(&dpu_cap->post_hist[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_POST_SCALER:
		hw_blk_ptr = dpu_hw_post_scaler_init(&dpu_cap->post_scaler[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	/* TODO: postpq modules to be added here */

	case BLK_DSC:
		hw_blk_ptr = dpu_hw_dsc_init(&dpu_cap->dsc[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_DSC_WDMA:
		hw_blk_ptr = dpu_hw_dsc_wdma_init(&dpu_cap->dsc_wdma[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_DSC_RDMA:
		hw_blk_ptr = dpu_hw_dsc_rdma_init(&dpu_cap->dsc_rdma[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_WB_TOP:
		hw_blk_ptr = dpu_hw_wb_top_init(&dpu_cap->wb_top[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_WB:
		hw_blk_ptr = dpu_hw_wb_init(&dpu_cap->wb_core[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_WB_SCALER:
		hw_blk_ptr = dpu_hw_wb_scaler_init(&dpu_cap->wb_scaler[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_DPU_INTR:
		hw_blk_ptr = dpu_hw_intr_init(&dpu_cap->dpu_intr[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	case BLK_DPU_GLB:
		hw_blk_ptr = dpu_hw_glb_init(&dpu_cap->dpu_glb[cap_idx],
				&dpu_cap->dpu_mem);
		break;
	default:
		DPU_ERROR("unsupported blk type: %s\n",
				_blk_type_to_str(blk_type));
		hw_blk_ptr = ERR_PTR(-EINVAL);
		break;
	}

	if (IS_ERR_OR_NULL(hw_blk_ptr)) {
		DPU_ERROR("failed to init block %s-%d, ret %ld\n",
				_blk_type_to_str(blk_type),
				cap_idx, PTR_ERR(hw_blk_ptr));
	}

	return hw_blk_ptr;
}

static void dpu_res_mgr_hw_blk_deinit(struct dpu_hw_blk *hw_blk,
		enum dpu_blk_type blk_type)
{
	switch (blk_type) {
	case BLK_DPU_TOP:
		dpu_hw_dpu_top_deinit(hw_blk);
		break;
	case BLK_CTL_TOP:
		dpu_hw_ctl_top_deinit(hw_blk);
		break;
	case BLK_SCENE_CTL:
		dpu_hw_scene_ctl_deinit(hw_blk);
		break;
	case BLK_RCH_TOP:
		dpu_hw_rch_top_deinit(hw_blk);
		break;
	case BLK_RCH:
		dpu_hw_rch_deinit(hw_blk);
		break;
	case BLK_PREPIPE_TOP:
		dpu_hw_prepipe_top_deinit(hw_blk);
		break;
	case BLK_PRE_SCALER:
		dpu_hw_pre_scaler_deinit(hw_blk);
		break;
	case BLK_TM:
		dpu_hw_tm_deinit(hw_blk);
		break;
	case BLK_PRE_3DLUT:
		dpu_hw_pre_3dlut_deinit(hw_blk);
		break;
	case BLK_PRE_HIST:
		dpu_hw_pre_hist_deinit(hw_blk);
		break;
	case BLK_MIXER:
		dpu_hw_mixer_deinit(hw_blk);
		break;
	case BLK_POST_PIPE_TOP:
		dpu_hw_post_pipe_top_deinit(hw_blk);
		break;
	case BLK_POST_GAMMA:
		dpu_hw_post_gamma_deinit(hw_blk);
		break;
	case BLK_POST_3DLUT:
		dpu_hw_post_3dlut_deinit(hw_blk);
		break;
	case BLK_POST_PQ_TOP:
		dpu_hw_postpq_top_deinit(hw_blk);
		break;
	case BLK_POST_PRE_PROC:
		dpu_hw_postpre_proc_deinit(hw_blk);
		break;
	case BLK_ACAD:
		dpu_hw_post_acad_deinit(hw_blk);
		break;
	case BLK_POST_HIST:
		dpu_hw_post_hist_deinit(hw_blk);
		break;
	case BLK_POST_SCALER:
		dpu_hw_post_scaler_deinit(hw_blk);
		break;
	/* TODO: postpq modules to be added here */
	case BLK_DSC:
		dpu_hw_dsc_deinit(hw_blk);
		break;
	case BLK_DSC_WDMA:
		dpu_hw_dsc_wdma_deinit(hw_blk);
		break;
	case BLK_DSC_RDMA:
		dpu_hw_dsc_rdma_deinit(hw_blk);
		break;
	case BLK_WB_TOP:
		dpu_hw_wb_top_deinit(hw_blk);
		break;
	case BLK_WB:
		dpu_hw_wb_deinit(hw_blk);
		break;
	case BLK_WB_SCALER:
		dpu_hw_wb_scaler_deinit(hw_blk);
		break;
	case BLK_DPU_INTR:
		dpu_hw_intr_deinit(hw_blk);
		break;
	case BLK_DPU_GLB:
		dpu_hw_glb_deinit(hw_blk);
		break;
	default:
		DPU_ERROR("unsupported blk type: %s\n",
				_blk_type_to_str(blk_type));
		break;
	}
}

static int dpu_res_mgr_block_alloc(
		struct dpu_res_mgr_ctx *ctx,
		struct dpu_cap *dpu_cap,
		u32 cap_idx,
		enum dpu_blk_type blk_type)
{
	struct dpu_res_blk *res_blk;
	struct dpu_hw_blk *hw_blk_ptr;

	hw_blk_ptr = dpu_res_mgr_hw_blk_init(dpu_cap, cap_idx, blk_type);
	if (IS_ERR_OR_NULL(hw_blk_ptr))
		return PTR_ERR(hw_blk_ptr);

	res_blk = kzalloc(sizeof(*res_blk), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(res_blk)) {
		dpu_res_mgr_hw_blk_deinit(hw_blk_ptr, blk_type);
		return -ENOMEM;
	}

	hw_blk_ptr->priv = (void *)res_blk;
	res_blk->blk_type = blk_type;
	res_blk->hw = hw_blk_ptr;
	atomic_set(&res_blk->refcount, 0);
	if (blk_type == BLK_RCH)
		ctx->rch_hw_blk[cap_idx] = hw_blk_ptr;

	mutex_lock(&ctx->res_lock);
	list_add_tail(&res_blk->list, &ctx->free_pool);
	mutex_unlock(&ctx->res_lock);

	return 0;
}

static int dpu_res_mgr_prepq_blocks_alloc(struct dpu_res_mgr_ctx *ctx,
		struct dpu_cap *dpu_cap)
{
	int ret = 0;
	int i;

	for (i = 0; i < dpu_cap->prepipe_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_PREPIPE_TOP);
		if (ret) {
			DPU_ERROR("failed to alloc dpu prepipe top block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->pre_scaler_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_PRE_SCALER);
		if (ret) {
			DPU_ERROR("failed to alloc dpu pre scaler block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->tm_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_TM);
		if (ret) {
			DPU_ERROR("failed to alloc dpu tm block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->pre_hist_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_PRE_HIST);
		if (ret) {
			DPU_ERROR("failed to alloc dpu pre hist block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->pre_lut3d_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_PRE_3DLUT);
		if (ret) {
			DPU_ERROR("failed to alloc dpu pre 3dlut block %d\n", i);
			goto err;
		}
	}

err:
	return ret;
}

static int dpu_res_mgr_postpq_blocks_alloc(struct dpu_res_mgr_ctx *ctx,
		struct dpu_cap *dpu_cap)
{
	int ret = 0;
	int i;

	for (i = 0; i < dpu_cap->post_pq_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_PQ_TOP);
		if (ret) {
			DPU_ERROR("failed to alloc dpu post_pq_top block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->post_pre_proc_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_PRE_PROC);
		if (ret) {
			DPU_ERROR("failed to alloc dpu post_pre_proc block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->acad_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_ACAD);
		if (ret) {
			DPU_ERROR("failed to alloc dpu acad block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->post_3dlut_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_3DLUT);
		if (ret) {
			DPU_ERROR("failed to alloc dpu post_3dlut block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->post_gamma_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_GAMMA);
		if (ret) {
			DPU_ERROR("failed to alloc dpu post_gamma block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->post_hist_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_HIST);
		if (ret) {
			DPU_ERROR("failed to alloc dpu post_hist block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->post_scaler_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_SCALER);
		if (ret) {
			DPU_ERROR("failed to alloc dpu post_scaler block %d\n", i);
			goto err;
		}
	}

err:
	return ret;
}

static int dpu_res_mgr_blocks_alloc(struct dpu_res_mgr_ctx *ctx,
		struct dpu_cap *dpu_cap)
{
	struct dpu_res_blk *blk_node = NULL;
	int ret = -EINVAL;
	int i;

	for (i = 0; i < dpu_cap->dpu_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_DPU_TOP);
		if (ret) {
			DPU_ERROR("failed to alloc dpu top block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->ctl_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_CTL_TOP);
		if (ret) {
			DPU_ERROR("failed to alloc ctl top block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->scene_ctl_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_SCENE_CTL);
		if (ret) {
			DPU_ERROR("failed to alloc scene ctl block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->rch_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_RCH_TOP);
		if (ret) {
			DPU_ERROR("failed to alloc rch top block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->rch_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_RCH);
		if (ret) {
			DPU_ERROR("failed to alloc rch block %d\n", i);
			goto err;
		}
	}

	ret = dpu_res_mgr_prepq_blocks_alloc(ctx, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to alloc pre pq blocks\n");
		goto err;
	}

	for (i = 0; i < dpu_cap->mixer_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_MIXER);
		if (ret) {
			DPU_ERROR("failed to alloc mixer block %d\n", i);
			goto err;
		}
	}

	ret = dpu_res_mgr_postpq_blocks_alloc(ctx, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to alloc post pq blocks\n");
		goto err;
	}


	for (i = 0; i < dpu_cap->post_pipe_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_PIPE_TOP);
		if (ret) {
			DPU_ERROR("failed to alloc dpu post_pipe_top block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->dsc_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_DSC);
		if (ret) {
			DPU_ERROR("failed to alloc dsc block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->dsc_wdma_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_DSC_WDMA);
		if (ret) {
			DPU_ERROR("failed to alloc dsc wdma block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->dsc_rdma_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_DSC_RDMA);
		if (ret) {
			DPU_ERROR("failed to alloc dsc rdma block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->wb_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_WB_TOP);
		if (ret) {
			DPU_ERROR("failed to alloc wb top block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->wb_core_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_WB);
		if (ret) {
			DPU_ERROR("failed to alloc wb core block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->wb_scaler_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_WB_SCALER);
		if (ret) {
			DPU_ERROR("failed to alloc wb scaler block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->dpu_intr_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_DPU_INTR);
		if (ret) {
			DPU_ERROR("failed to alloc intr block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->dpu_glb_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_DPU_GLB);
		if (ret) {
			DPU_ERROR("failed to alloc glb block %d\n", i);
			goto err;
		}
	}

	list_for_each_entry(blk_node, &ctx->free_pool, list)
		DPU_DEBUG("name: %s, id: %u\n",
				_blk_type_to_str(blk_node->blk_type),
				blk_node->hw->blk_id);

err:
	return ret;
}

static void dpu_res_mgr_blocks_free(struct list_head *list_head)
{
	struct dpu_res_blk *res_blk = NULL;
	struct dpu_res_blk *temp = NULL;

	list_for_each_entry_safe(res_blk, temp, list_head, list) {
		list_del_init(&res_blk->list);
		dpu_res_mgr_hw_blk_deinit(res_blk->hw, res_blk->blk_type);
		kfree(res_blk);
	}
}

static int dpu_res_mgr_blocks_init(
		struct dpu_res_mgr_ctx *ctx,
		struct drm_device *drm_dev,
		struct dpu_cap *dpu_cap)
{
	int ret;

	ret = dpu_cap_init(drm_dev, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to init dpu_cap, ret %d\n", ret);
		return ret;
	}

	ret = dpu_res_mgr_blocks_alloc(ctx, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to alloc blocks, ret %d\n", ret);
		goto err;
	}

	return 0;
err:
	dpu_res_mgr_blocks_free(&ctx->free_pool);
	devm_iounmap(drm_dev->dev, dpu_cap->dpu_mem.base);
	return ret;
}

static int dpu_res_mgr_find_avail_chain(
		struct dpu_res_mgr_ctx *ctx,
		int pipe_id)
{
	int chain_id = -EINVAL;
	bool is_empty = false;
	int i;

	mutex_lock(&ctx->res_lock);
	for (i = 0; i < MAX_RES_PIPELINE_COUNT; i++) {
		/* reset the pipe of all empty chain */
		is_empty = list_empty(&ctx->res_chain[i].chain_list);
		if (is_empty)
			ctx->res_chain[i].pipe_id = -1;

		if (ctx->res_chain[i].pipe_id == pipe_id) {
			chain_id = i;
			break;
		}

		/* find a available slot */
		if (is_empty && (chain_id < 0))
			chain_id = i;
	}
	mutex_unlock(&ctx->res_lock);

	return chain_id;
}

static bool dpu_res_mgr_blk_id_compare(struct dpu_res_blk *blk_node,
		u32 blk_attr)
{
	int blk_id = blk_attr;
	int node_id = blk_node->hw->blk_id;

	if (blk_id < BLK_MAX && (node_id != blk_id))
		return false;

	return true;
}

static bool dpu_res_mgr_blk_feature_compare(struct dpu_res_blk *blk_node,
		u32 blk_attr)
{
	const unsigned long node_feature = blk_node->hw->features;
	int blk_feature = blk_attr;

	if (test_bit(blk_feature, &node_feature))
		return true;

	return false;
}

static struct dpu_res_blk *dpu_res_mgr_find_free_blk(
		struct dpu_res_mgr_ctx *ctx,
		struct dpu_res_chain *res_chain,
		enum dpu_blk_type blk_type,
		bool (*blk_compare)(struct dpu_res_blk *blk_node, u32 blk_attr),
		u32 blk_attr,
		bool test_only)
{
	struct dpu_res_blk *blk_node = NULL, *temp = NULL;
	bool found = false;

	mutex_lock(&ctx->res_lock);
	list_for_each_entry_safe(blk_node, temp, &ctx->free_pool, list) {
		if (blk_node->blk_type != blk_type)
			continue;

		if (!blk_compare(blk_node, blk_attr))
			continue;

		if (!test_only) {
			list_move(&blk_node->list, &res_chain->chain_list);
			atomic_inc(&blk_node->refcount);
		}

		found = true;
		break;
	}
	mutex_unlock(&ctx->res_lock);

	if (!found) {
		DPU_ERROR("failed to reserve block %s\n",
				_blk_type_to_str(blk_type));
		return ERR_PTR(-ENOENT);
	}

	return blk_node;
}

static inline bool is_reused_blk(enum dpu_blk_type blk_type)
{
	switch (blk_type) {
	case BLK_RCH:
	case BLK_RCH_TOP:
	case BLK_PREPIPE_TOP:
	case BLK_TM:
	case BLK_PRE_SCALER:
	case BLK_PRE_3DLUT:
		return true;
	default:
		return false;
	}
}

static struct dpu_res_blk *dpu_res_mgr_find_reserved_blk(
		struct dpu_res_mgr_ctx *ctx,
		struct dpu_res_chain *res_chain,
		enum dpu_blk_type blk_type, int blk_id,
		bool test_only)
{
	struct dpu_res_blk *blk_node = NULL;
	bool found = false;

	if (!is_reused_blk(blk_type))
		return ERR_PTR(-EINVAL);

	if (blk_id >= BLK_MAX)
		return ERR_PTR(-EINVAL);

	mutex_lock(&ctx->res_lock);
	list_for_each_entry(blk_node, &res_chain->chain_list, list) {
		if ((blk_node->blk_type == blk_type)
				&& (blk_node->hw->blk_id == blk_id)) {
			found = true;
			break;
		}
	}
	mutex_unlock(&ctx->res_lock);

	if (!found)
		return NULL;

	if (!test_only)
		atomic_inc(&blk_node->refcount);

	return blk_node;
}

struct dpu_hw_blk *dpu_res_mgr_block_reserve_feature(
		struct dpu_res_mgr_ctx *ctx, int pipe_id,
		enum dpu_blk_type blk_type, unsigned long blk_feature,
		bool test_only)
{
	struct dpu_res_chain *chain_ptr;
	struct dpu_res_blk *res_blk;
	int chain_id;

	chain_id = dpu_res_mgr_find_avail_chain(ctx, pipe_id);
	if (chain_id < 0) {
		DPU_ERROR("pipe[%d] failed to get a chain slot %s-%lx\n",
				pipe_id, _blk_type_to_str(blk_type), blk_feature);
		return ERR_PTR(chain_id);
	}

	chain_ptr = &ctx->res_chain[chain_id];

	res_blk = dpu_res_mgr_find_free_blk(ctx, chain_ptr, blk_type,
			&dpu_res_mgr_blk_feature_compare,
			blk_feature, test_only);
	if (IS_ERR_OR_NULL(res_blk)) {
		DPU_ERROR("pipe[%d] failed to reserve block %s with feature %lx\n",
				pipe_id, _blk_type_to_str(blk_type), blk_feature);
		return ERR_PTR(-EBUSY);
	}

	chain_ptr->pipe_id = pipe_id;

	return res_blk->hw;
}

struct dpu_hw_blk *dpu_res_mgr_block_reserve_id(
		struct dpu_res_mgr_ctx *ctx, int pipe_id,
		enum dpu_blk_type blk_type, u32 blk_id,
		bool test_only)
{
	struct dpu_res_chain *chain_ptr;
	struct dpu_res_blk *res_blk;
	int chain_id;

	if (!ctx) {
		DPU_ERROR("ctx is null\n");
		return NULL;
	}

	chain_id = dpu_res_mgr_find_avail_chain(ctx, pipe_id);
	if (chain_id < 0) {
		DPU_ERROR("pipe[%d] failed to get a chain for slot %s-%u\n",
				pipe_id, _blk_type_to_str(blk_type), blk_id);
		return ERR_PTR(chain_id);
	}

	chain_ptr = &ctx->res_chain[chain_id];

	res_blk = dpu_res_mgr_find_reserved_blk(ctx,
			chain_ptr, blk_type, blk_id, test_only);
	if (!IS_ERR_OR_NULL(res_blk))
		return res_blk->hw;

	res_blk = dpu_res_mgr_find_free_blk(ctx, chain_ptr, blk_type,
			&dpu_res_mgr_blk_id_compare,
			blk_id, test_only);
	if (IS_ERR_OR_NULL(res_blk)) {
		DPU_ERROR("pipe[%d] failed to reserve block %s-%u\n",
				pipe_id, _blk_type_to_str(blk_type), blk_id);
		return ERR_PTR(-EBUSY);
	}

	chain_ptr->pipe_id = pipe_id;

	return res_blk->hw;
}

struct dpu_hw_blk *dpu_res_mgr_block_reserve(
		struct dpu_res_mgr_ctx *ctx, int pipe_id,
		enum dpu_blk_type blk_type, bool test_only)
{
	return dpu_res_mgr_block_reserve_id(ctx,
			pipe_id, blk_type, BLK_MAX, test_only);
}

void dpu_res_mgr_block_release(
		struct dpu_res_mgr_ctx *ctx,
		struct dpu_hw_blk *hw_blk)
{
	struct dpu_res_blk *res_blk;

	if (!ctx || !hw_blk) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", ctx, hw_blk);
		return;
	}

	res_blk = (struct dpu_res_blk *)hw_blk->priv;

	if (atomic_sub_and_test(1, &res_blk->refcount)) {
		mutex_lock(&ctx->res_lock);
		list_move(&res_blk->list, &ctx->free_pool);
		mutex_unlock(&ctx->res_lock);

		RES_DEBUG("release %s, id %u\n",
				_blk_type_to_str(res_blk->blk_type),
				res_blk->hw->blk_id);
	}
}

int dpu_res_mgr_context_init(struct dpu_res_mgr_ctx **ctx,
		struct drm_device *drm_dev)
{
	struct dpu_res_mgr_ctx *mgr_ctx;
	int i;
	int ret;

	if (*ctx || !drm_dev) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", ctx, drm_dev);
		return -EINVAL;
	}

	mgr_ctx = kzalloc(sizeof(*mgr_ctx), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(mgr_ctx))
		return -ENOMEM;

	INIT_LIST_HEAD(&mgr_ctx->free_pool);
	mutex_init(&mgr_ctx->res_lock);
	dpu_res_mgr_dpu_cap_init(&mgr_ctx->dpu_cap);

	for (i = 0; i < MAX_RES_PIPELINE_COUNT; i++) {
		INIT_LIST_HEAD(&mgr_ctx->res_chain[i].chain_list);
		mgr_ctx->res_chain[i].pipe_id = -1;
	}

	ret = dpu_res_mgr_blocks_init(mgr_ctx, drm_dev, &mgr_ctx->dpu_cap);
	if (ret)
		goto failed;

	*ctx = mgr_ctx;

	return 0;
failed:
	DPU_ERROR("failed to init resource manager context, ret %d\n", ret);
	mutex_destroy(&mgr_ctx->res_lock);
	kfree(mgr_ctx);
	return ret;
}

void dpu_res_mgr_context_cleanup(struct dpu_res_mgr_ctx *ctx)
{
	int i;

	if (!ctx) {
		DPU_ERROR("invalid parameters\n");
		return;
	}

	for (i = 0; i < MAX_RES_PIPELINE_COUNT; i++)
		dpu_res_mgr_blocks_free(&ctx->res_chain[i].chain_list);

	dpu_res_mgr_blocks_free(&ctx->free_pool);

	mutex_destroy(&ctx->res_lock);

	kfree(ctx);
}
