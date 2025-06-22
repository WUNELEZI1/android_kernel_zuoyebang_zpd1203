// SPDX-License-Identifier: GPL-2.0
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

#include <linux/bitops.h>
#include "dpu_log.h"
#include "dpu_isr.h"

static void dpu_isr_init_listener_list(struct dpu_isr *isr_ctrl)
{
	s32 i;

	for (i = 0; i < INTS_MAX; ++i)
		INIT_LIST_HEAD(&isr_ctrl->isr_listener_list[i]);
}

static struct list_head *dpu_isr_get_listener_list(struct dpu_isr *isr_ctrl, u32 listen_id)
{
	if (listen_id >= INTS_MAX) {
		DPU_ERROR("invalid index:%d", listen_id);
		return NULL;
	}

	return &isr_ctrl->isr_listener_list[listen_id];
}

void dpu_isr_setup(struct dpu_isr *isr_ctrl)
{
	if (unlikely(!isr_ctrl))
		return;

	dpu_isr_init_listener_list(isr_ctrl);

	// TODO: There is currently no need to implement this feature
	isr_ctrl->handle_func = NULL;
}

s32 dpu_isr_register_listener(struct dpu_isr *isr_ctrl,
		struct notifier_block *nb, u32 listen_id, void *listener_data)
{
	struct dpu_isr_listener_node *listener_node = NULL;
	struct list_head *listener_list = NULL;

	if (!isr_ctrl) {
		DPU_ERROR("isr_ctrl is null ptr");
		return -1;
	}

	listener_list = dpu_isr_get_listener_list(isr_ctrl, listen_id);
	if (!listener_list) {
		DPU_ERROR("get listener_list fail, listen_id=0x%x", listen_id);
		return -1;
	}

	listener_node = kzalloc(sizeof(*listener_node), GFP_KERNEL);
	if (!listener_node) {
		DPU_ERROR("alloc listener node fail, listen_id=0x%x", listen_id);
		return -1;
	}

	listener_node->listen_id = listen_id;
	listener_node->listener_data.data = listener_data;

	list_add_tail(&listener_node->list_node, listener_list);

	return raw_notifier_chain_register(&listener_node->irq_nofitier, nb);
}

s32 dpu_isr_unregister_listener(struct dpu_isr *isr_ctrl,
		struct notifier_block *nb, u32 listen_id)
{
	struct list_head *listener_list = NULL;
	struct dpu_isr_listener_node *listener_node = NULL;
	struct dpu_isr_listener_node *_node_ = NULL;

	if (!isr_ctrl || !nb) {
		DPU_ERROR("isr_ctrl or nb is null ptr");
		return -1;
	}

	listener_list = dpu_isr_get_listener_list(isr_ctrl, listen_id);

	list_for_each_entry_safe(listener_node, _node_, listener_list, list_node) {
		if (listener_node->listen_id != listen_id)
			continue;
		if (listener_node->irq_nofitier.head == nb) {
			DPU_DEBUG("listen bit 0x%x, del node\n", listen_id);
			raw_notifier_chain_unregister(&listener_node->irq_nofitier, nb);

			list_del(&listener_node->list_node);
			kfree(listener_node);
		}
	}

	return 0;
}

s32 dpu_isr_notify_listener(struct dpu_isr *isr_ctrl, u32 listen_id, u64 timestamp)
{
	struct list_head *listener_list = NULL;
	struct dpu_isr_listener_node *listener_node = NULL;
	struct dpu_isr_listener_node *_node_ = NULL;
	u64 notify_timestamp;

	if (!isr_ctrl) {
		DPU_ERROR("isr_ctrl is null ptr");
		return -1;
	}

	/**
	 * @brief listen_id MUST be single bit!!!
	 *
	 */
	listener_list = dpu_isr_get_listener_list(isr_ctrl, listen_id);
	notify_timestamp = timestamp;

	list_for_each_entry_safe(listener_node, _node_, listener_list, list_node) {
		if (listener_node->listen_id == listen_id) {
			listener_node->listener_data.notify_timestamp = notify_timestamp;
			raw_notifier_call_chain(&listener_node->irq_nofitier,
					listen_id, &listener_node->listener_data);
		}
	}

	return 0;
}
