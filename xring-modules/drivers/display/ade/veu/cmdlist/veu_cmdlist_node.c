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

#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include "veu_cmdlist_node.h"
#include "hw/veu_cmdlist_hw_ops.h"
#include "../veu_defs.h"

#define NODE_ITEM_ALIGN 2
#define CMDLIST_ALIGNMENT_SIZE 16
#define NODE_LEN_EXTRA 2

static struct cmdlist_node *g_last_node;
static s64 g_last_id;

/**
 * bit_reg - for bit write
 * @head: the item of bitw_header
 * @reg: the bit write reg
 * @value: current value
 */
struct bit_reg {
	struct list_head head;
	u32 reg;
	u32 value;
};

void cmdlist_node_prepare(int *node_len, int reg_num)
{
	*node_len += ALIGN(reg_num, NODE_ITEM_ALIGN) / NODE_ITEM_ALIGN;
}

struct cmdlist_node *get_cmdlist_node(struct cmdlist_base *base, s64 id)
{
	struct cmdlist_frame *frame;
	struct cmdlist_node *node;

	veu_check_and_return(!base, NULL, "base is null");

	if (g_last_id == id)
		return g_last_node;

	frame = &base->frame;
	node = NULL;

	read_lock(&frame->lock);
	list_for_each_entry(node, &frame->list, frame_head) {
		if (node->id == id) {
			read_unlock(&frame->lock);
			g_last_id = id;
			g_last_node = node;
			return node;
		}
	}
	read_unlock(&frame->lock);

	VEU_DBG("cannot find node: %u!", (u32)id);

	return NULL;
}

static void *cmdlist_node_alloc(struct device *dev, u32 size,
		dma_addr_t *phy_addr)
{
	void *vir_addr;

	vir_addr = dma_alloc_coherent(dev, size, phy_addr, GFP_KERNEL);
	if (!vir_addr) {
		VEU_ERR("alloc for cmdlist node mem fail, size:%zu", (size_t)size);
		return NULL;
	}

	memset(vir_addr, 0, size);

	return vir_addr;
}

static int alloc_items(struct device *dev, struct cmdlist_node *node,
		u32 nod_len, u32 align_size)
{
	node->node_size = ALIGN(nod_len * CMDLIST_ALIGNMENT_SIZE, align_size);
	node->header = cmdlist_node_alloc(dev, node->node_size,
			&node->header_addr);
	if (!node->header)
		return -ENOMEM;

	node->items = (struct cmdlist_item *)node->header + 1;

	VEU_DBG("alloc node: 0x%llx", node->header_addr);
	return 0;
}

s64 cmdlist_node_create(struct device *dev, struct cmdlist_base *base, u8 nod_type,
		u16 nod_len, s64 blk_id, u64 wait_event)
{
	struct cmdlist_frame *frame;
	struct cmdlist_node *node;

	veu_check_and_return(!base, -EINVAL, "base is null");

	VEU_DBG("blk id %lld, type %d, len %d, event 0x%llx", blk_id, nod_type,
			nod_len, wait_event);

	frame = &base->frame;

	node = kzalloc(sizeof(struct cmdlist_node), GFP_KERNEL);
	if (!node) {
		VEU_ERR("failed to create node!");
		return -ENOMEM;
	}

	if (alloc_items(dev, node, nod_len, base->mem_align)) {
		VEU_ERR("failed to alloc node item!");
		kfree(node);
		return -ENOMEM;
	}

	node->id = MAKE_NODE_ID(frame->frm_id, blk_id);
	VEU_DBG("node id %lld", node->id);
	node->node_length = nod_len;
	node->header->node_type = nod_type;
	node->header->wait_event_l = wait_event;
	node->header->wait_event_h = wait_event >> 5;

	write_lock(&frame->lock);
	list_add_tail(&node->frame_head, &frame->list);
	write_unlock(&frame->lock);

	return node->id;
}

int cmdlist_node_delete(struct device *dev, struct cmdlist_base *base, s64 id)
{
	struct cmdlist_frame *frame;
	struct cmdlist_node *node;

	veu_check_and_return(!base, -EINVAL, "base is null");

	frame = &base->frame;

	node = get_cmdlist_node(base, id);
	if (!node)
		return -ENOMEM;
	VEU_DBG("get cmdlist nod: id%u", (u32)id);

	write_lock(&frame->lock);
	list_del(&node->frame_head);
	write_unlock(&frame->lock);

	if (g_last_id == id)
		g_last_id = -1;

	dma_free_coherent(dev, node->node_size, node->header, node->header_addr);

	kfree(node);
	return 0;
}

static int node_config(struct cmdlist_node *node, u32 reg_addr, u32 value)
{
	struct cmdlist_item *item;
	u32 offset;
	u32 *data;

	reg_addr >>= 2;

	offset = 0;
	if (node->last_addr == 0)
		node->last_addr = reg_addr;
	else
		offset = reg_addr - node->last_addr;

	if (offset > 2) {
		if (node->current_index == node->node_length - NODE_LEN_EXTRA) {
			VEU_ERR("node %hhu:%u is full when config reg: 0x%x\n",
					CMDLIST_FRM_ID(node->id), (u32)node->id, reg_addr);
			return -ELNRNG;
		}

		offset = 0;
		node->current_index++;
		node->last_cfg_info = 0;
		node->last_addr = reg_addr;
	}

	item = &node->items[node->current_index];
	data = (u32 *)item + 1;
	data[offset] = value;
	node->last_strobe |= 0xf << (offset << 2);
	item->cfg_info = node->last_cfg_info;

	return 0;
}

int cmdlist_node_config(struct cmdlist_base *base,
		s64 id, u32 reg_addr, u32 value)
{
	struct cmdlist_node *node;

	veu_check_and_return(!base, -EINVAL, "base is null");

	VEU_DBG("node_id : %lld, reg: 0x%x val: 0x%x", id, reg_addr, value);

	node = get_cmdlist_node(base, id);
	if (!node) {
		VEU_ERR("node unfound");
		return -ENOMEM;
	}

	return node_config(node, reg_addr, value);
}

int cmdlist_cfg_rdy_config(struct cmdlist_base *base, uint32_t cfg_rdy)
{
	struct cmdlist_frame *frame;
	struct cmdlist_node *node;

	veu_check_and_return(!base, -EINVAL, "base is null");

	frame = &base->frame;
	node = NULL;

	if (!list_empty(&frame->list)) {
		list_for_each_entry(node, &frame->list, frame_head) {
			node->cfg_rdy = cfg_rdy;
			VEU_DBG("node id:%u, cfg_rdy: 0x%x", (u32)node->id, cfg_rdy);
		}
	}

	return 0;
}

#if defined(CONFIG_XRING_DEBUG)
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
#endif
