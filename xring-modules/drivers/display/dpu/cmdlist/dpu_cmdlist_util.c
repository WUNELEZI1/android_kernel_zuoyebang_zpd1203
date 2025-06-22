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

#include <linux/module.h>
#include "dpu_cmdlist_node.h"
#include "dpu_hw_common.h"
#include "dpu_log.h"

void cmdlist_write(struct dpu_hw_blk *hw, u32 reg_offset, u32 val)
{
	if (unlikely(!hw->cmd_node)) {
		DPU_ERROR("hw->cmd_node is a NULL pointer!\n");
		return;
	}

	cmdlist_node_config(*hw->cmd_node, hw->blk_offset + reg_offset, val);
}

void cmdlist_write_with_node_id(struct dpu_hw_blk *hw, s64 node_id, u32 reg_offset, u32 val)
{
	if (unlikely(!node_id || !hw)) {
		DPU_ERROR("node id or hw pointer error!\n");
		return;
	}

	cmdlist_node_config(node_id, hw->blk_offset + reg_offset, val);
}

void stripe_cmdlist_bitmask_write(struct dpu_hw_blk *hw, s64 node_id, u32 reg_offset,
		u32 val, u32 bit_start, u32 bit_mask)
{
	if (unlikely(!node_id || !hw)) {
		DPU_ERROR("node id or hw pointer error!\n");
		return;
	}

	cmdlist_node_bit_config(node_id, hw->blk_offset + reg_offset, val,
			bit_start, bit_mask, hw->iomem_base);
}

void cmdlist_bitmask_write(struct dpu_hw_blk *hw, u32 reg_offset,
		u32 val, u32 bit_start, u32 bit_mask)
{
	if (unlikely(!hw->cmd_node)) {
		DPU_ERROR("hw->cmd_node is a NULL pointer!\n");
		return;
	}

	cmdlist_node_bit_config(*hw->cmd_node, hw->blk_offset + reg_offset, val,
			bit_start, bit_mask, hw->iomem_base);
}

void cmdlist_repeat_write(struct dpu_hw_blk *hw, u32 reg_offset, u32 val)
{
	if (unlikely(!hw->cmd_node)) {
		DPU_ERROR("hw->cmd_node is a NULL pointer!\n");
		return;
	}

	cmdlist_node_same_config(*hw->cmd_node, hw->blk_offset + reg_offset, val);
}

void cmdlist_repeat_write_with_node_id(struct dpu_hw_blk *hw, s64 node_id, u32 reg_offset, u32 val)
{
	if (unlikely(!hw->cmd_node)) {
		DPU_ERROR("hw->cmd_node is a NULL pointer!\n");
		return;
	}

	cmdlist_node_same_config(node_id, hw->blk_offset + reg_offset, val);
}
