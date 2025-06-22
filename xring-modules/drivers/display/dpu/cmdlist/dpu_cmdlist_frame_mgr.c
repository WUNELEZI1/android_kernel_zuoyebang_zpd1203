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

#include <linux/slab.h>

#include "dpu_cmdlist_frame_mgr.h"
#include "dpu_cmdlist_node.h"
#include "dpu_flow_ctrl.h"
#include "dpu_hw_ctl_top_reg.h"
#include "dpu_hw_cmdlist_reg.h"
#include "dpu_hw_cmdlist_ops.h"
#include "dpu_cmdlist_common.h"
#include "dpu_hw_parser.h"
#include "dpu_log.h"
#include "dpu_dbg.h"

#define RCH_PREFETCH 24

/**
 * frame_node - cmdlist_node in frame
 * @frame_head: head in frame channel
 * @node: cmdlist_node
 * @flag: 0 - destory with frame, (other value) - self-define
 */
struct frame_node {
	struct list_head frame_head;
	struct cmdlist_node *node;
	u32 flag;
};

static atomic_t g_frame_id;

/**
 * cmdlist_frame - manage each node for a frame
 * @channel_head: head in frame channel
 * @frame_list: list header for each node head
 * @lock: lock of node list
 * @id: cmdlist frame id
 * @commited: cmdlist frame has been commited
 */
struct cmdlist_frame {
	struct list_head channel_head;
	struct list_head frame_list;
	spinlock_t lock;

	u32 id;
	bool commited;
};

static struct cmdlist_frame *get_frame(s64 frame_id)
{
	struct cmdlist_frame *frame;
	struct cmdlist_base *base;

	base = get_cmdlist_instance();
	if (unlikely(CMDLIST_CH_ID(frame_id) != base->frame_channel.ch_id)) {
		DPU_ERROR("id: %d:%u is not a frame id!\n",
				CMDLIST_CH_ID(frame_id), (u32)frame_id);
		return NULL;
	}

	spin_lock(&base->frame_channel.lock);
	list_for_each_entry(frame, &base->frame_channel.list, channel_head) {
		if (frame->id == (u32)frame_id) {
			spin_unlock(&base->frame_channel.lock);
			return frame;
		}
	}
	spin_unlock(&base->frame_channel.lock);

	DPU_ERROR("cannot find frame: %u\n", (u32)frame_id);

	return NULL;

}

s64 cmdlist_frame_create(void)
{
	struct cmdlist_frame *frame;
	struct cmdlist_base *base;

	base = get_cmdlist_instance();
	frame = kzalloc(sizeof(struct cmdlist_frame), GFP_KERNEL);
	if (unlikely(!frame)) {
		DPU_ERROR("frame alloc failed!\n");
		return -ENOMEM;
	}

	frame->id = atomic_inc_return(&g_frame_id);

	INIT_LIST_HEAD(&frame->frame_list);
	spin_lock_init(&frame->lock);
	spin_lock(&base->frame_channel.lock);
	list_add_tail(&frame->channel_head, &base->frame_channel.list);
	spin_unlock(&base->frame_channel.lock);

	DPU_CMDLIST_DEBUG("cmdlist create frame: %u\n", frame->id);

	return MAKE_NODE_ID(base->frame_channel.ch_id, frame->id);
}

int cmdlist_frame_node_append(s64 frame_id, s64 *node_id, u32 flag)
{
	struct cmdlist_frame *frame;
	struct cmdlist_node *node;
	struct frame_node *f_node;

	if (unlikely(!node_id)) {
		DPU_ERROR("node_id nullptr!\n");
		return -ENOMEM;
	}

	frame = get_frame(frame_id);
	if (unlikely(!frame))
		return -ECHRNG;

	if (unlikely(frame->commited)) {
		DPU_ERROR("frame: %u has been commited\n", frame->id);
		WARN_ON(true);
		return -EPERM;
	}

	node = get_cmdlist_node(*node_id);
	if (unlikely(!node))
		return -ENOMEM;

	if (unlikely(node->is_used)) {
		DPU_ERROR("node: %d:%u cannot reuse for frame: %u\n",
				CMDLIST_CH_ID(*node_id), (u32)*node_id, frame->id);
		WARN_ON(true);
		return -EPERM;
	}

	f_node = kzalloc(sizeof(struct frame_node), GFP_KERNEL);
	if (unlikely(!f_node)) {
		DPU_ERROR("f_node alloc failed!\n");
		return -ENOMEM;
	}

	DPU_CMDLIST_DEBUG("add node: %d:%u to frame: %u\n", CMDLIST_CH_ID(*node_id),
			(u32)*node_id, frame->id);

	f_node->node = node;
	f_node->flag = flag;
	CMDLIST_NODE_MARK_USED(*node_id);
	spin_lock(&frame->lock);
	list_add_tail(&f_node->frame_head, &frame->frame_list);
	spin_unlock(&frame->lock);

	return 0;
}

int cmdlist_frame_commit(s64 id)
{
	struct cmdlist_header **header;
	struct cmdlist_frame *frame;
	struct cmdlist_base *base;
	struct cmdlist_node *node;
	struct frame_node *f_node;
	u8 ch_id;
	u32 qos;
	u32 y;
	u8 i;

	base = get_cmdlist_instance();
	frame = get_frame(id);
	if (unlikely(!frame))
		return -ECHRNG;

	if (unlikely(frame->commited)) {
		DPU_ERROR("frame: %u has been commited\n", (u32)id);
		WARN_ON(true);
		return -EPERM;
	}

	if (unlikely(list_empty(&frame->frame_list))) {
		DPU_ERROR("frame %u is empty!\n", (u32)id);
		return -EMEDIUMTYPE;
	}
	DPU_CMDLIST_DEBUG("%u\n", (u32)id);

	header = kcalloc(base->frame_channel.ch_id, sizeof(struct cmdlist_header *),
			GFP_KERNEL);
	if (unlikely(!header)) {
		DPU_ERROR("temp node alloc error!\n");
		return -ENOMEM;
	}

	spin_lock(&frame->lock);
	list_for_each_entry(f_node, &frame->frame_list, frame_head) {
		node = f_node->node;
		ch_id = CMDLIST_CH_ID(node->id);
		if (header[ch_id])
			header[ch_id]->next_node_addr = node->header_addr;
		else
			cmdlist_first_addr_setup(ch_id, node->header_addr);

		dpu_flow_qos_get(FLOW_QOS_CMDLIST, NULL, &qos);
		cmdlist_qos_config(ch_id, qos);

		if (node->is_y_pos_set) {
			y = node->layer_y_pos;
			cmdlist_node_config(node->id, base->ctl_top_offset +
					(ch_id << 2) + RCH_START_CMPS_Y_OFFSET,
					(y > RCH_PREFETCH) ? (y - RCH_PREFETCH) : 0);

			if (header[ch_id])
				cmdlist_node_config(node->id, base->cmdlist_offset +
						(ch_id << 2) + CMDLIST_CH_Y_OTHER_OFFSET, y);
			else
				cmdlist_first_layer_y_setup(ch_id, y, qos);
		}

		if (node->cfg_rdy)
			cmdlist_node_config(node->id, base->ctl_top_offset +
					CMDLIST_RDMA_CFG_RDY_OFFSET, node->cfg_rdy);

		node->node_length = node->current_index + 2;
		node->header->node_length = node->node_length;
		node->items[node->current_index].last_row = 1;
		header[ch_id] = node->header;
		node->is_used = f_node->flag == DELETE_WITH_FRAME;
	}
	spin_unlock(&frame->lock);

	for (i = 0; i < base->channel_count; i++) {
		if (header[i])
			header[i]->node_type = LAST_TYPE;
	}

	kfree(header);
	frame->commited = true;

	return 0;
}


int cmdlist_frame_destroy(s64 id)
{
	struct cmdlist_frame *frame;
	struct cmdlist_base *base;
	struct frame_node *f_node;

	frame = get_frame(id);
	if (unlikely(!frame))
		return -ECHRNG;

	DPU_CMDLIST_DEBUG("frame: %u destroy!\n", (u32)id);

	base = get_cmdlist_instance();
	spin_lock(&frame->lock);
	while (!list_empty(&frame->frame_list)) {
		f_node = container_of(frame->frame_list.next, typeof(*f_node),
				frame_head);

		list_del(&f_node->frame_head);
		if (f_node->flag == DELETE_WITH_FRAME)
			cmdlist_node_delete(f_node->node->id);
		kfree(f_node);
	}
	spin_unlock(&frame->lock);

	spin_lock(&base->frame_channel.lock);
	list_del(&frame->channel_head);
	spin_unlock(&base->frame_channel.lock);
	kfree(frame);

	return 0;
}

int cmdlist_frame_init(void)
{
	struct cmdlist_base *base;

	base = get_cmdlist_instance();
	base->frame_channel.ch_id = base->channel_count;
	INIT_LIST_HEAD(&base->frame_channel.list);
	spin_lock_init(&base->frame_channel.lock);

	return 0;
}

void cmdlist_frame_deinit(void)
{
	struct cmdlist_frame *frame;
	struct cmdlist_base *base;
	struct frame_node *f_node;

	base = get_cmdlist_instance();

	while (!list_empty(&base->frame_channel.list)) {
		frame = container_of(base->frame_channel.list.next, typeof(*frame),
				channel_head);
		list_del(&frame->channel_head);
		while (!list_empty(&frame->frame_list)) {
			f_node = container_of(frame->frame_list.next, typeof(*f_node),
					frame_head);

			list_del(&f_node->frame_head);
			kfree(f_node);
		}

		kfree(frame);
	}
}
