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

#include <stddef.h>

#include "osal.h"
#include "dpu_list.h"
#include "dpu_log.h"
#include "dpu_res_mgr.h"
#include "dpu_hw_init_module_ops.h"

struct dpu_res_mgr_ctx g_res_mgr;

struct hw_blk_ops_table {
	enum DPU_BLK_TYPE blk_type;
	char *type_str;
	void (*deinit)(struct dpu_hw_blk *hw);
};

static struct hw_blk_ops_table hw_blk_ops_table[] = {
	{BLK_NONE, "blk_no_defined", NULL},
	{BLK_CTL_TOP, "blk_ctl_top", dpu_hw_ctl_top_deinit},
	{BLK_SCENE_CTL, "blk_scene_ctl", dpu_hw_scene_ctl_deinit},
	{BLK_RCH, "blk_rch", dpu_hw_rch_deinit},
	{BLK_RCH_TOP, "blk_rch_top", dpu_hw_rch_top_deinit},
	{BLK_PREPIPE_TOP, "blk_prepipe_top", dpu_hw_prepipe_top_deinit},
	{BLK_PRE_SCALER, "blk_pre_scaler", dpu_hw_pre_scaler_deinit},
	{BLK_TM, "blk_tm", dpu_hw_tm_deinit},
	{BLK_PRE_3DLUT, "blk_pre_3dlut", dpu_hw_pre_3dlut_deinit},
	{BLK_PRE_HIST, "blk_pre_hist", dpu_hw_pre_hist_deinit},
	{BLK_MIXER, "blk_mixer", dpu_hw_mixer_deinit},
	{BLK_POST_PIPE_TOP, "blk_post_pipe_top", dpu_hw_post_pipe_top_deinit},
	{BLK_POST_PQ_TOP, "blk_postpq_top", dpu_hw_postpq_top_deinit},
	{BLK_POST_PRE_PROC, "blk_postpre_proc", dpu_hw_postpre_proc_deinit},
	{BLK_ACAD, "blk_acad", dpu_hw_post_acad_deinit},
	{BLK_POST_3DLUT, "blk_post_3dlut", dpu_hw_post_3dlut_deinit},
	{BLK_POST_GAMMA, "blk_gamma", dpu_hw_post_gamma_deinit},
	{BLK_POST_HIST, "blk_hist", dpu_hw_post_hist_deinit},
	{BLK_POST_SCALER, "blk_post_scaler", dpu_hw_post_scaler_deinit},
	{BLK_DSC, "blk_dsc", dpu_hw_dsc_deinit},
	{BLK_WB_TOP, "blk_wb_top", dpu_hw_wb_top_deinit},
	{BLK_WB, "blk_wb", dpu_hw_wb_deinit},
	{BLK_WB_SCALER, "blk_wb_scaler", dpu_hw_wb_scaler_deinit},
	{BLK_DPU_INTR, "blk_dpu_intr", dpu_hw_intr_deinit},
};

const char *_blk_type_to_str(enum DPU_BLK_TYPE type)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(hw_blk_ops_table); i++) {
		if (type == hw_blk_ops_table[i].blk_type)
			return hw_blk_ops_table[i].type_str;
	}

	dpu_pr_err("not found type:%d\n", type);
	return hw_blk_ops_table[0].type_str;
}

static struct dpu_hw_blk *dpu_res_mgr_hw_blk_init(
		struct dpu_cap *dpu_cap, uint32_t cap_idx,
		enum DPU_BLK_TYPE blk_type)
{
	struct dpu_hw_blk *hw_blk_ptr = NULL;

	switch (blk_type) {
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
	default:
		dpu_pr_err("unsupported blk type: %d %a", blk_type, _blk_type_to_str(blk_type));
		break;
	}

	if (!hw_blk_ptr)
		dpu_pr_err("failed to init block %a-%d\n", _blk_type_to_str(blk_type), cap_idx);
	return hw_blk_ptr;
}

static void dpu_res_mgr_hw_blk_deinit(struct dpu_hw_blk *hw_blk,
		enum DPU_BLK_TYPE blk_type)
{
	bool found = false;
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(hw_blk_ops_table); i++) {
		if (blk_type == hw_blk_ops_table[i].blk_type) {
			found = true;
			hw_blk_ops_table[i].deinit(hw_blk);
		}
	}

	if (!found)
		dpu_pr_err("unsupported blk type: %a\n", _blk_type_to_str(blk_type));
}

static int32_t dpu_res_mgr_block_alloc(struct dpu_res_mgr_ctx *ctx, struct dpu_cap *dpu_cap,
		uint32_t cap_idx, enum DPU_BLK_TYPE blk_type)
{
	struct dpu_hw_blk *hw_blk_ptr;
	struct dpu_res_blk *res_blk;

	hw_blk_ptr = dpu_res_mgr_hw_blk_init(dpu_cap, cap_idx, blk_type);
	if (!hw_blk_ptr)
		return -1;
	res_blk = dpu_mem_alloc(sizeof(*res_blk));
	if (!res_blk) {
		dpu_res_mgr_hw_blk_deinit(hw_blk_ptr, blk_type);
		return -1;
	}
	hw_blk_ptr->priv = (void *)res_blk;
	res_blk->blk_type = blk_type;
	res_blk->hw = hw_blk_ptr;

	list_add(&res_blk->list, &ctx->free_pool);
	dpu_pr_debug("type:%d id: %u name: %a\n", res_blk->blk_type, res_blk->hw->blk_id,
				_blk_type_to_str(res_blk->blk_type));
	return 0;
}

static int dpu_res_mgr_prepq_blocks_alloc(struct dpu_res_mgr_ctx *ctx,
		struct dpu_cap *dpu_cap)
{
	int ret = 0;
	int i;

	/* TODO: prepq modules to be added here */

	for (i = 0; i < dpu_cap->prepipe_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_PREPIPE_TOP);
		if (ret) {
			dpu_pr_err("failed to alloc dpu prepipe top block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->pre_scaler_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_PRE_SCALER);
		if (ret) {
			dpu_pr_err("failed to alloc dpu pre scaler block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->tm_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_TM);
		if (ret) {
			dpu_pr_err("failed to alloc dpu tm block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->pre_hist_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_PRE_HIST);
		if (ret) {
			dpu_pr_err("failed to alloc dpu pre hist block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->pre_lut3d_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_PRE_3DLUT);
		if (ret) {
			dpu_pr_err("failed to alloc dpu pre 3dlut block %d\n", i);
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
			dpu_pr_err("failed to alloc dpu post_pq_top block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->post_pre_proc_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_PRE_PROC);
		if (ret) {
			dpu_pr_err("failed to alloc dpu post_pre_proc block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->acad_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_ACAD);
		if (ret) {
			dpu_pr_err("failed to alloc dpu acad block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->post_3dlut_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_3DLUT);
		if (ret) {
			dpu_pr_err("failed to alloc dpu post_3dlut block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->post_gamma_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_GAMMA);
		if (ret) {
			dpu_pr_err("failed to alloc dpu post_gamma block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->post_hist_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_HIST);
		if (ret) {
			dpu_pr_err("failed to alloc dpu post_hist block %d\n", i);
			goto err;
		}
	}

	for (i = 0; i < dpu_cap->post_scaler_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_SCALER);
		if (ret) {
			dpu_pr_err("failed to alloc dpu post_scaler block %d\n", i);
			goto err;
		}
	}

err:
	return ret;
}

static int32_t dpu_res_mgr_blocks_alloc(struct dpu_res_mgr_ctx *ctx,
		struct dpu_cap *dpu_cap)
{
	int32_t ret = -1;
	uint32_t i;

	dpu_pr_debug("+\n");

	for (i = 0; i < dpu_cap->ctl_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_CTL_TOP);
		if (ret) {
			dpu_pr_err("failed to alloc ctl top block %d\n", i);
			return -1;
		}
	}

	for (i = 0; i < dpu_cap->scene_ctl_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_SCENE_CTL);
		if (ret) {
			dpu_pr_err("failed to alloc scene ctl block %d\n", i);
			return -1;
		}
	}

	for (i = 0; i < dpu_cap->rch_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_RCH_TOP);
		if (ret) {
			dpu_pr_err("failed to alloc rch top block %d\n", i);
			return -1;
		}
	}

	for (i = 0; i < dpu_cap->rch_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_RCH);
		if (ret) {
			dpu_pr_err("failed to alloc rch block %d\n", i);
			return -1;
		}
	}

	ret = dpu_res_mgr_prepq_blocks_alloc(ctx, dpu_cap);
	if (ret) {
		dpu_pr_err("failed to alloc pre pq blocks\n");
		return -1;
	}

	for (i = 0; i < dpu_cap->mixer_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_MIXER);
		if (ret) {
			dpu_pr_err("failed to alloc mixer block %d\n", i);
			return -1;
		}
	}

	for (i = 0; i < dpu_cap->post_pipe_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_POST_PIPE_TOP);
		if (ret) {
			dpu_pr_err("failed to alloc dpu post_pipe_top block %d\n", i);
			return -1;
		}
	}

	ret = dpu_res_mgr_postpq_blocks_alloc(ctx, dpu_cap);
	if (ret) {
		dpu_pr_err("failed to alloc post pq blocks\n");
		return -1;
	}

	for (i = 0; i < dpu_cap->dsc_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_DSC);
		if (ret) {
			dpu_pr_err("failed to alloc mixer block %d\n", i);
			return -1;
		}
	}

	for (i = 0; i < dpu_cap->wb_top_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_WB_TOP);
		if (ret) {
			dpu_pr_err("failed to alloc dpu wb_top block %d\n", i);
			return -1;
		}
	}

	for (i = 0; i < dpu_cap->wb_core_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_WB);
		if (ret) {
			dpu_pr_err("failed to alloc wb core block %d\n", i);
			return -1;
		}
	}

	for (i = 0; i < dpu_cap->wb_scaler_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_WB_SCALER);
		if (ret) {
			dpu_pr_err("failed to alloc wb scaler block %d\n", i);
			return -1;
		}
	}

	for (i = 0; i < dpu_cap->dpu_intr_count; i++) {
		ret = dpu_res_mgr_block_alloc(ctx, dpu_cap, i, BLK_DPU_INTR);
		if (ret) {
			dpu_pr_err("failed to alloc intr block %d\n", i);
			return -1;
		}
	}

	dpu_pr_debug("-\n");
	return ret;
}

static void dpu_res_mgr_blocks_free(struct list_head *list_head)
{
	struct list_head *pos = NULL;

	list_for_each(pos, &g_res_mgr.free_pool) {
		struct dpu_res_blk *blk_node = container_of(pos, struct dpu_res_blk, list);
		dpu_pr_debug("free id: %u name: %a\n", blk_node->hw->blk_id,
				_blk_type_to_str(blk_node->blk_type));
		list_del(&blk_node->list);
		dpu_res_mgr_hw_blk_deinit(blk_node->hw, blk_node->blk_type);
		dpu_mem_free(blk_node);
	}
}

struct dpu_hw_blk *get_hw_blk(enum DPU_BLK_TYPE blk_type, uint32_t blk_id)
{
	struct list_head *pos = NULL;

	list_for_each(pos, &g_res_mgr.free_pool) {
		struct dpu_res_blk *blk_node = container_of(pos, struct dpu_res_blk, list);

		if (blk_node->blk_type != blk_type)
			continue;
		if (blk_node->hw->blk_id == blk_id)
			return blk_node->hw;
	}
	dpu_pr_err("not found blk_type %u, blk_id %u\n", blk_type, blk_id);
	return NULL;
}

int32_t dpu_res_mgr_init(void)
{
	int32_t ret;

	dpu_pr_debug("+\n");
	INIT_LIST_HEAD(&g_res_mgr.free_pool);
	ret = dpu_cap_init(&g_res_mgr.dpu_cap);
	if (ret) {
		dpu_pr_err("failed to init dpu_cap, ret %d\n", ret);
		return ret;
	}

	ret = dpu_res_mgr_blocks_alloc(&g_res_mgr, &g_res_mgr.dpu_cap);
	if (ret) {
		dpu_pr_err("failed to alloc blocks, ret %d\n", ret);
		goto err;
	}

	/* init lowlevel dpu base */
	dpu_hw_init_module_init(g_res_mgr.dpu_cap.dpu_mem.base);

	dpu_pr_debug("-\n");
	return 0;
err:
	dpu_res_mgr_blocks_free(&g_res_mgr.free_pool);
	return ret;
}
