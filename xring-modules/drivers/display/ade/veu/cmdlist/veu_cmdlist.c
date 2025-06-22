// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#include <linux/platform_device.h>
#include <linux/of.h>

#include "veu_cmdlist_frame_mgr.h"
#include "veu_cmdlist_node.h"
#include "veu_cmdlist.h"
#include "../veu_drv.h"
#include "../veu_defs.h"
#include "hw/hw_reg/veu_hw_cmdlist_reg.h"

#define RDMA_PATH_REG_NUM 64
#define PIPE_REG_NUM      58
#define WB_CORE_REG_NUM   50
#define CMDLIST_MEM_ALIGN 0x1000
#define SCALER_REG_NUM    500
#define STRIPE_REG_NUM    50

static void veu_rch_cmdlist_node_prepare(int *node_len)
{
	cmdlist_node_prepare(node_len, RDMA_PATH_REG_NUM);
}

static void veu_scaler_pipe_node_prepare(int *node_len)
{
	cmdlist_node_prepare(node_len, SCALER_REG_NUM);
}

static void veu_pq_cmdlist_node_prepare(int *node_len)
{
	cmdlist_node_prepare(node_len, PIPE_REG_NUM);
}

static void veu_wch_cmdlist_node_prepare(int *node_len)
{
	cmdlist_node_prepare(node_len, WB_CORE_REG_NUM);
}

static void veu_stripe_cmdlist_node_prepare(int *node_len)
{
	cmdlist_node_prepare(node_len, STRIPE_REG_NUM);
}

static void veu_cmdlist_prepare(struct veu_data *veu_dev,
		s64 blk_id, bool is_first_blk, bool is_last_blk)
{
	struct veu_cmdlist *cmdlist;
	u32 node_len = 1;

	if (veu_dev->veu_stripe == NULL) {
		VEU_ERR("input param null!");
		return;
	}

	cmdlist = veu_dev->cmdlist;

	if (is_first_blk && (blk_id == 0)) {
		cmdlist_frame_destroy(veu_dev->of_dev, &cmdlist->base);
		cmdlist_frame_create(&cmdlist->base);
		veu_rch_cmdlist_node_prepare(&node_len);

		if (veu_dev->veu_stripe->stripe_param[0].scaler.scaler_en)
			veu_scaler_pipe_node_prepare(&node_len);
		else
			veu_pq_cmdlist_node_prepare(&node_len);

		veu_wch_cmdlist_node_prepare(&node_len);
	} else {
		veu_stripe_cmdlist_node_prepare(&node_len);
	}

	if (!is_last_blk)
		veu_dev->node_id[blk_id] = cmdlist_node_create(veu_dev->of_dev,
				&cmdlist->base, NORMAL_TYPE, node_len, blk_id, WB0_SLICE_DONE);
	else
		veu_dev->node_id[blk_id] = cmdlist_node_create(veu_dev->of_dev,
				&cmdlist->base, LAST_TYPE, node_len, blk_id,
				WB0_RELOAD | RDMA_RELOAD | PREPQ_RELOAD);

	VEU_DBG("veu_dev->node_id[%lld] = %ld",	blk_id,
			veu_dev->node_id[blk_id]);
}

static void veu_cmdlist_commit(struct veu_data *veu_dev)
{
	struct veu_cmdlist *cmdlist;
	struct cmdlist_base *base;
	uint32_t cfg_rdy;

	VEU_DBG("enter");

	cmdlist = veu_dev->cmdlist;
	base = &cmdlist->base;
	cfg_rdy = CMDLIST_RDMA_CFG_RDY(0) | CMDLIST_PREPQ_CFG_RDY(0) | CMDLIST_WB_CFG_RDY(0);

	cmdlist_cfg_rdy_config(base, cfg_rdy);
	cmdlist_frame_commit(base);

	VEU_DBG("exit");
}

static struct cmdlist_funcs funcs = {
	.prepare = veu_cmdlist_prepare,
	.commit = veu_cmdlist_commit,
};

static void cmdlist_reg_base_init(struct veu_data *veu_dev,
		struct cmdlist_base *base)
{
	base->veu_base = veu_dev->base;
	base->veu_cmdlist_base = base->veu_base + VEU_CMDLIST_ADDR;
	base->ctl_top_offset = VEU_CTRL_TOP_ADDR;
	base->mem_align = CMDLIST_MEM_ALIGN;
	VEU_DBG("veu_base = 0x%pK", base->veu_base);
	VEU_DBG("veu_cmdlist_base = 0x%pK", base->veu_cmdlist_base);
	VEU_DBG("cmdlist-mem-align = %d", base->mem_align);
	VEU_DBG("ctl_top_offset = 0x%x", base->ctl_top_offset);
}

int veu_cmdlist_init(struct veu_data *veu_dev)
{
	struct veu_cmdlist *cmdlist;
	int ret;

	veu_check_and_return(!veu_dev, -EINVAL, "veu_dev is null");

	cmdlist = kzalloc(sizeof(*cmdlist), GFP_KERNEL);
	if (!cmdlist) {
		VEU_ERR("failed to alloc veu cmdlist");
		return -ENOMEM;
	}

	cmdlist_reg_base_init(veu_dev, &cmdlist->base);

	ret = cmdlist_frame_init(&cmdlist->base);
	if (ret)
		goto err_out;

	cmdlist->funcs = &funcs;
	veu_dev->cmdlist = cmdlist;

	return 0;

err_out:
	kfree(cmdlist);
	return ret;
}

void veu_cmdlist_deinit(struct veu_data *veu_dev)
{
	struct veu_cmdlist *cmdlist;

	veu_check_and_void_return(!veu_dev, "veu_dev is null");

	cmdlist = veu_dev->cmdlist;
	veu_check_and_void_return(!cmdlist, "cmdlist is null");

	cmdlist_frame_deinit(&cmdlist->base);

	kfree(cmdlist);
	veu_dev->cmdlist = NULL;
}
