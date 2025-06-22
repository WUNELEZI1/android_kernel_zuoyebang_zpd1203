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

#include <linux/list.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "dpu_cmdlist_common.h"
#include "dpu_cmdlist_node.h"
#include "dpu_hw_cmdlist_ops.h"
#include "dpu_log.h"

#define CMDLIST_ALIGNMENT_SIZE 16
#define NODE_LEN_EXTRA 2 // cmdlist header line and cfg_rdy line

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

static int alloc_items(struct cmdlist_node *node, u32 nod_len)
{
	struct cmdlist_base *base = get_cmdlist_instance();

	node->node_size = ALIGN(nod_len * CMDLIST_ALIGNMENT_SIZE, base->mem_align);
	node->header = dksm_mem_pool_alloc(base->node_pool, node->node_size,
			&node->header_addr);
	if (unlikely(!node->header))
		return -ENOMEM;

	node->items = (struct cmdlist_item *)node->header + 1;

	DPU_CMDLIST_DEBUG("alloc node: 0x%llx\n", node->header_addr);
	return 0;
}

static struct cmdlist_channel *get_cmd_ch(u8 ch_id)
{
	struct cmdlist_base *base = get_cmdlist_instance();

	if (unlikely(ch_id >= base->channel_count)) {
		DPU_ERROR("bad cmdlist channel id: %d!\n", ch_id);
		return NULL;
	}

	return &base->cmdlist_channels[ch_id];
}

static s64 reserve_node_id(u8 ch_id, struct cmdlist_node *node)
{
	struct cmdlist_channel *ch;
	int index;
	int i;

	ch = get_cmd_ch(ch_id);
	if (unlikely(!ch))
		return CMDLIST_INVALID_NODE_ID;

	spin_lock(&ch->lock);
	for (i = 0; i < MAX_CHANNEL_NODE; i++) {
		index = (ch->next_node_id + i) % MAX_CHANNEL_NODE;
		if (likely(!ch->nodes[index])) {
			ch->nodes[index] = node;
			ch->next_node_id = index + 1;
			spin_unlock(&ch->lock);
			return MAKE_NODE_ID(ch_id, index);
		}
	}
	spin_unlock(&ch->lock);

	return CMDLIST_INVALID_NODE_ID;
}

static void release_node_id(s64 id)
{
	struct cmdlist_channel *ch;

	ch = get_cmd_ch(CMDLIST_CH_ID(id));
	if (likely(ch))
		ch->nodes[(u32)id] = NULL;
}

s64 cmdlist_node_create(u8 ch_id, u8 nod_type, u16 nod_len, u64 wait_event)
{
	struct cmdlist_node *node;
	s64 node_id;

	nod_len += NODE_LEN_EXTRA;

	node = kzalloc(sizeof(struct cmdlist_node), GFP_KERNEL);
	if (unlikely(!node)) {
		DPU_ERROR("failed to create node!\n");
		goto error;
	}

	node_id = reserve_node_id(ch_id, node);
	if (unlikely(node_id == CMDLIST_INVALID_NODE_ID)) {
		DPU_ERROR("channel: %d, len %d cannot reserve id\n", ch_id, nod_len);
		goto node_id_reserve_fail;
	}

	if (unlikely(alloc_items(node, nod_len))) {
		DPU_ERROR("failed to alloc node item!\n");
		goto node_buf_alloc_fail;
	}

	node->id = node_id;
	node->node_length = nod_len;
	node->header->node_type = nod_type;
	node->header->wait_event_l = wait_event;
	node->header->wait_event_h = wait_event >> 5;

	DPU_CMDLIST_DEBUG("id %d:%u, type %d, len %d, event 0x%llx\n",
			ch_id, (u32)node->id, nod_type, nod_len, wait_event);

	return node_id;

node_buf_alloc_fail:
	release_node_id(node_id);

node_id_reserve_fail:
	kfree(node);

error:
	return CMDLIST_INVALID_NODE_ID;
}

int cmdlist_node_delete(s64 id)
{
	struct cmdlist_node *node;
	struct cmdlist_base *base;

	base = get_cmdlist_instance();
	node = get_cmdlist_node(id);
	if (unlikely(!node))
		return -ENOMEM;

	DPU_CMDLIST_DEBUG("%d:%u\n", CMDLIST_CH_ID(id), (u32)id);
	release_node_id(id);
	if (likely(node->header))
		dksm_mem_pool_free(base->node_pool, node->header, node->node_size);

	kfree(node);

	return 0;
}

struct cmdlist_node *get_cmdlist_node(s64 id)
{
	struct cmdlist_channel *channel;

	if (unlikely(!IS_CMDLIST_NODE(id))) {
		DPU_ERROR("not cmdlist node: 0x%llx\n", id);
		WARN_ON(true);
		return NULL;
	}

	channel = get_cmd_ch(CMDLIST_CH_ID(id));
	if (unlikely(!channel))
		return NULL;

	if (unlikely((u32)id >= MAX_CHANNEL_NODE)) {
		DPU_ERROR("cannot find node: %d:%u!\n", channel->ch_id, (u32)id);
		return NULL;
	}

	return channel->nodes[(u32)id];
}

int cmdlist_node_config(s64 id, u32 reg_addr, u32 value)
{
	struct cmdlist_node *node;
	struct cmdlist_item *item;
	u32 offset;
	u32 *data;

	node = get_cmdlist_node(id);
	if (unlikely(!node))
		return -ENOMEM;

	offset = 0;
	reg_addr >>= 2;
	if (likely(node->last_addr != 0))
		offset = reg_addr - node->last_addr;
	else
		node->last_addr = reg_addr;

	if (offset > 2) {
		if (unlikely(node->current_index == node->node_length - NODE_LEN_EXTRA)) {
			DPU_ERROR("node %hhu:%u is full when config reg: 0x%x, " \
					"curr index: %d\n", CMDLIST_CH_ID(id), (u32)id,
					reg_addr << 2, node->current_index);
			WARN_ON(true);
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

int cmdlist_node_bit_config(s64 id, u32 reg_addr, u32 value, u8 loc, u32 mask,
		void __iomem *base)
{
	struct list_head *bitw_list;
	struct cmdlist_channel *ch;
	struct bit_reg *tmp;

	if (unlikely(!IS_CMDLIST_NODE(id))) {
		DPU_ERROR("not cmdlist node: 0x%llx\n", id);
		return -ENOMEM;
	}

	ch = get_cmd_ch(CMDLIST_CH_ID(id));
	if (unlikely(!ch))
		return -ENOMEM;

	DPU_CMDLIST_DEBUG("reg: 0x%x val: 0x%x, bit: %d, mask: 0x%x\n",
		reg_addr, value, loc, mask);

	bitw_list = &ch->bitw_list;
	value = (value  << loc) & mask;

	list_for_each_entry(tmp, bitw_list, head) {
		if (tmp->reg == reg_addr) {
			tmp->value &= ~mask;
			tmp->value |= value;
			break;
		}
	}

	if (&tmp->head == bitw_list) {
		tmp = kzalloc(sizeof(struct bit_reg), GFP_KERNEL);
		if (unlikely(!tmp)) {
			DPU_ERROR("failed to alloc bit_reg\n");
			return -ENOMEM;
		}

		tmp->value = cmdlist_inp32(reg_addr);
		tmp->value &= ~mask;
		tmp->value |= value;
		tmp->reg = reg_addr;
		list_add_tail(&tmp->head, bitw_list);
	}

	return cmdlist_node_config(id, reg_addr, tmp->value);
}

int cmdlist_node_same_config(s64 id, u32 reg_addr, u32 value)
{
	struct cmdlist_node *node;
	struct cmdlist_item *item;

	node = get_cmdlist_node(id);
	if (unlikely(!node))
		return -ENOMEM;

	DPU_CMDLIST_DEBUG("reg: 0x%x val: 0x%x\n", reg_addr, value);

	if (node->last_addr != 0) {
		if (unlikely(node->current_index == node->node_length - NODE_LEN_EXTRA)) {
			DPU_ERROR("node %hhu:%u is full when config reg: 0x%x, " \
					"curr index: %d\n", CMDLIST_CH_ID(id), (u32)id,
					reg_addr, node->current_index);
			WARN_ON(true);
			return -ELNRNG;
		}

		node->current_index++;
	}

	node->last_addr = reg_addr >> 2;
	node->last_strobe = 0xf;
	item = &node->items[node->current_index];
	item->cfg_info = node->last_cfg_info;
	item->data0 = value;

	return 0;
}

int cmdlist_cfg_rdy_set(s64 id, u32 cfg_rdy)
{
	struct cmdlist_node *node;

	node = get_cmdlist_node(id);
	if (unlikely(!node))
		return -ENOMEM;

	DPU_CMDLIST_DEBUG("node: %hhu:%u cfg_rdy: 0x%x\n", CMDLIST_CH_ID(id), (u32)id,
			cfg_rdy);

	node->cfg_rdy = cfg_rdy;

	return 0;
}

int cmdlist_wait_event_update(s64 id, u64 wait_event)
{
	struct cmdlist_node *node;

	node = get_cmdlist_node(id);
	if (unlikely(!node))
		return -ENOMEM;

	DPU_CMDLIST_DEBUG("node: %hhu:%u event 0x%llx\n", CMDLIST_CH_ID(id), (u32)id, wait_event);

	node->header->wait_event_l = wait_event;
	node->header->wait_event_h = wait_event >> 5;
	return 0;
}

int cmdlist_node_type_update(s64 id, u8 node_type)
{
	struct cmdlist_node *node;

	node = get_cmdlist_node(id);
	if (unlikely(!node))
		return -ENOMEM;

	DPU_CMDLIST_DEBUG("node: %hhu:%u type 0x%x\n", CMDLIST_CH_ID(id), (u32)id, node_type);

	node->header->node_type = node_type;
	return 0;
}

int cmdlist_layer_y_pos_set(s64 id, u32 y)
{
	struct cmdlist_node *node;

	node = get_cmdlist_node(id);
	if (unlikely(!node))
		return -ENOMEM;

	DPU_CMDLIST_DEBUG("node: %hhu:%u y: 0x%x\n", CMDLIST_CH_ID(id), (u32)id, y);

	if (!node->is_y_pos_set || node->layer_y_pos > y)
		node->layer_y_pos = y;

	node->is_y_pos_set = true;

	return 0;
}

int cmdlist_channel_init(void)
{
	struct cmdlist_base *base;
	u8 i;

	base = get_cmdlist_instance();
	if (base->channel_count < 1) {
		DPU_ERROR("cmdlist channel cannot init by ch_num: %d\n",
				base->channel_count);
		return -ENODATA;
	}

	base->cmdlist_channels = kcalloc(base->channel_count,
			sizeof(struct cmdlist_channel), GFP_KERNEL);

	for (i = 0; i < base->channel_count; i++) {
		base->cmdlist_channels[i].ch_id = i;
		spin_lock_init(&base->cmdlist_channels[i].lock);
		INIT_LIST_HEAD(&base->cmdlist_channels[i].bitw_list);
	}

	DPU_CMDLIST_INFO("init by ch_num: %d\n", base->channel_count);

	return 0;
}

void cmdlist_channel_deinit(void)
{
	struct cmdlist_channel *ch;
	struct cmdlist_base *base;
	struct bit_reg *tmp;
	int j;
	u8 i;

	DPU_CMDLIST_INFO("deinit!\n");

	base = get_cmdlist_instance();
	for (i = 0; i < base->channel_count; i++) {
		ch = get_cmd_ch(i);
		if (!ch)
			continue;

		for (j = 0; j < MAX_CHANNEL_NODE; j++)
			if (ch->nodes[j])
				cmdlist_node_delete(((struct cmdlist_node *)ch->nodes[j])->id);

		while (!list_empty(&ch->bitw_list)) {
			tmp = container_of(ch->bitw_list.next, struct bit_reg, head);
			list_del(&tmp->head);
			kfree(tmp);
		}
	}

	kfree(base->cmdlist_channels);
}

#if defined(CONFIG_XRING_DEBUG)
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
#endif
