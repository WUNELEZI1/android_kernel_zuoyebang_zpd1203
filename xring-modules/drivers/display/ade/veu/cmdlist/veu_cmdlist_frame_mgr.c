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

#include <linux/slab.h>

#include "veu_cmdlist_node.h"
#include "hw/veu_cmdlist_hw_ops.h"
#include "../veu_defs.h"

static u32 g_frame_id;
#define VEU_CMDLIST_CHN_ID 0
#define CMDLIST_RDMA_CFG_RDY_OFFSET 0x34

int cmdlist_frame_commit(struct cmdlist_base *base)
{
	struct cmdlist_header *header;
	struct cmdlist_frame *frame;
	struct cmdlist_node *node;

	veu_check_and_return(!base, -EINVAL, "base is null");

	header = NULL;
	node = NULL;
	frame = &base->frame;

	list_for_each_entry(node, &frame->list, frame_head) {
		if (header)
			header->next_node_addr = node->header_addr;
		else
			cmdlist_frame_first_node_addr_setup(base->veu_cmdlist_base,
					VEU_CMDLIST_CHN_ID, node->header_addr);

		if (node->cfg_rdy)
			cmdlist_node_config(base, node->id,
					base->ctl_top_offset + CMDLIST_RDMA_CFG_RDY_OFFSET, node->cfg_rdy);
		header = node->header;

		/**
		 * current_index was module feature regs configure
		 * add 2 row to config cmdlist_cfg_rdy.
		 */
		node->node_length = node->current_index + 2;
		node->header->node_length = node->node_length;
		node->items[node->current_index].last_row = 1;
	}

	header->node_type = LAST_TYPE;
	cmdlist_config_enable(base->veu_base + base->ctl_top_offset, VEU_CMDLIST_CHN_ID, true);

	return 0;
}


int cmdlist_frame_create(struct cmdlist_base *base)
{
	struct cmdlist_frame *frame;

	veu_check_and_return(!base, -EINVAL, "base is null");

	frame = &base->frame;

	frame->frm_id = g_frame_id++;

	VEU_DBG("cmdlist frame update frame id: %u", frame->frm_id);
	return 0;
}

int cmdlist_frame_destroy(struct device *dev, struct cmdlist_base *base)
{
	struct cmdlist_frame *frame;
	struct cmdlist_node *node;

	veu_check_and_return(!base, -EINVAL, "base is null");

	frame = &base->frame;
	VEU_DBG("frame id %d", frame->frm_id);
	node = NULL;

	while (!list_empty(&frame->list)) {
		VEU_DBG("frame->list is not null, we need delete node first");
		node = container_of(frame->list.next, typeof(*node), frame_head);
		cmdlist_node_delete(dev, base, node->id);
	}

	cmdlist_config_enable(base->veu_base + base->ctl_top_offset, VEU_CMDLIST_CHN_ID, false);

	return 0;
}

int cmdlist_frame_init(struct cmdlist_base *base)
{
	veu_check_and_return(!base, -EINVAL, "base is null");

	base->frame.frm_id = 0;

	INIT_LIST_HEAD(&base->frame.list);
	rwlock_init(&base->frame.lock);

	return 0;
}

void cmdlist_frame_deinit(struct cmdlist_base *base)
{
	struct cmdlist_frame *frame;
	bool is_empty;

	veu_check_and_void_return(!base, "base is null");

	is_empty = list_empty(&base->frame.list);
	if (is_empty)
		return;

	while (!list_empty(&base->frame.list)) {
		frame = container_of(base->frame.list.next,
				typeof(*frame), list);
		list_del(&frame->list);
		kfree(frame);
	}
}
