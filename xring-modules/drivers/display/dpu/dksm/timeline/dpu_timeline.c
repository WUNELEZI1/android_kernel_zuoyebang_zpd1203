// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#include <linux/sync_file.h>
#include <linux/slab.h>
#include <uapi/linux/sched/types.h>

#include "dpu_timeline.h"
#include "dpu_isr.h"
#include "dpu_log.h"

static s32 _timeline_step_pt_value(struct dpu_timeline *timeline)
{
	unsigned long flags = 0;

	if (g_debug_fence_timeline)
		DPU_INFO("%s: pt_value = %llu, inc_step = %u, next_value = %u",
				timeline->name, timeline->pt_value, timeline->inc_step, timeline->next_value);

	spin_lock_irqsave(&timeline->value_lock, flags);
	if (timeline->inc_step == 0) {
		spin_unlock_irqrestore(&timeline->value_lock, flags);
		return -1;
	}
	timeline->pt_value += timeline->inc_step;

	if (likely(timeline->next_value >= timeline->inc_step))
		timeline->next_value -= timeline->inc_step;

	timeline->inc_step = 0;
	spin_unlock_irqrestore(&timeline->value_lock, flags);

	return 0;
}

void dpu_timeline_dump(struct dpu_timeline *timeline)
{
	struct timeline_listener *listener_node = NULL;
	struct timeline_listener *_node_ = NULL;

	mutex_lock(&timeline->list_lock);
	list_for_each_entry_safe(listener_node, _node_, &timeline->listener_list, list_node) {
		if (!listener_node->ops) {
			DPU_ERROR("listener_node ops is NULL");
			continue;
		}
		DPU_INFO("%s: timeline_value=%llu, listener_name:%s value:%llu, timeline next_value %u, inc_step %u",
				timeline->name, timeline->pt_value, listener_node->ops->get_listener_name(listener_node),
				listener_node->pt_value, timeline->next_value, timeline->inc_step);
	}

	mutex_unlock(&timeline->list_lock);
}

static void timeline_sync_workqueue(struct kthread_work *work)
{
	struct dpu_timeline *timeline = NULL;
	struct timeline_listener *listener_node = NULL;
	struct timeline_listener *_node_ = NULL;
	s32 ret = 0;

	if (!work) {
		DPU_ERROR("work is NULL!!!");
		return;
	}

	timeline = container_of(work, typeof(*timeline), timeline_sync_work);
	if (!timeline) {
		DPU_ERROR("timeline is NULL!!!");
		return;
	}

	mutex_lock(&timeline->list_lock);
	list_for_each_entry_safe(listener_node, _node_, &timeline->listener_list, list_node) {
		if (!listener_node->ops) {
			DPU_ERROR("listener_node ops is NULL!!!");
			continue;
		}
		listener_node->notify_timestamp = timeline->notify_timestamp;

		if (g_debug_fence_timeline || timeline->need_dump)
			DPU_INFO("%s: value=%llu, listener_name:%s value:%llu notify_timestamp=%lld",
				timeline->name, timeline->pt_value,
				listener_node->ops->get_listener_name(listener_node), listener_node->pt_value,
				ktime_to_ns(timeline->notify_timestamp));

		if (!listener_node->ops->is_signaled(listener_node, timeline->pt_value))
			continue;

		if (listener_node->ops->handle_signal) {
			ret = listener_node->ops->handle_signal(listener_node);
			if (ret != 0)
				DPU_WARN("listener = 0x%pK handle fail", listener_node);
		}

		if (listener_node->ops->release)
			listener_node->ops->release(listener_node);

		list_del(&listener_node->list_node);
		kfree(listener_node);
	}

	timeline_dump_flag_set(timeline, false);
	mutex_unlock(&timeline->list_lock);
}

static s32 _timeline_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct listener_data *listener_data = (struct listener_data *)data;
	struct dpu_timeline *timeline = (struct dpu_timeline *)(listener_data->data);

	if (action != timeline->listening_id) {
		DPU_INFO("action %#lx is not equal to listening_id %#x",
				action, timeline->listening_id);
		return 0;
	}

	/* pt_value doesn't need step */
	if ((_timeline_step_pt_value(timeline) < 0) && (timeline->need_recovery == 0))
		return 0;

	timeline->notify_timestamp = listener_data->notify_timestamp;
	kthread_queue_work(&timeline->handle_worker, &timeline->timeline_sync_work);

	return 0;
}

static struct notifier_block timeline_isr_notifier = {
	.notifier_call = _timeline_isr_notify,
};

int dpu_timeline_init(struct dpu_timeline *timeline,
		const char *name, void *parent, u32 listening_bit, struct dpu_isr *isr)
{
	struct sched_param sp;
	int ret = 0;

	snprintf(timeline->name, sizeof(timeline->name), "%s", name);
	kref_init(&timeline->kref);
	spin_lock_init(&timeline->value_lock);
	mutex_init(&timeline->list_lock);
	INIT_LIST_HEAD(&timeline->listener_list);

	/* init timeline value, which fence create on */
	timeline->pt_value = 1;
	timeline->inc_step = 0;
	timeline->next_value = 0;
	timeline->parent = parent;
	timeline->context = dma_fence_context_alloc(1);
	timeline->notifier = &timeline_isr_notifier;
	timeline->listening_id = listening_bit;
	kthread_init_work(&timeline->timeline_sync_work, timeline_sync_workqueue);
	kthread_init_worker(&timeline->handle_worker);

	timeline->timeline_task = kthread_run(kthread_worker_fn,
			&timeline->handle_worker, name);
	if (IS_ERR(timeline->timeline_task)) {
		DPU_ERROR("failed to run %s thread\n", name);
		ret = PTR_ERR(timeline->timeline_task);
		goto end;
	}
	sp.sched_priority = MAX_RT_PRIO - 1;
	WARN_ON_ONCE(sched_setscheduler_nocheck(timeline->timeline_task,
			SCHED_FIFO, &sp) != 0);

	DPU_INFO("init %s listening_id=%#x", timeline->name, timeline->listening_id);

	dpu_isr_register_listener(isr, timeline->notifier, timeline->listening_id, timeline);

end:
	return ret;
}

void dpu_timeline_deinit(struct dpu_timeline *timeline, struct dpu_isr *isr)
{
	if (!timeline) {
		DPU_ERROR("invalid timeline pointer\n");
		return;
	}

	mutex_destroy(&timeline->list_lock);
	if (timeline->timeline_task)
		kthread_stop(timeline->timeline_task);

	dpu_isr_unregister_listener(isr, timeline->notifier, timeline->listening_id);
}

void dpu_timeline_resync_pt(struct dpu_timeline *timeline, u32 value)
{
	unsigned long flags;
	u32 inc_step;

	if (g_debug_fence_timeline)
		DPU_INFO("%s: pt_value = %llu, inc_step = %u, next_value = %u, value = %u",
			timeline->name, timeline->pt_value, timeline->inc_step, timeline->next_value, value);

	spin_lock_irqsave(&timeline->value_lock, flags);
	if (value > timeline->pt_value)
		inc_step = timeline->inc_step + (value - timeline->pt_value);
	else
		inc_step = timeline->inc_step + 2;

	timeline->pt_value += inc_step;
	timeline->next_value = 0;
	timeline->inc_step = 0;

	spin_unlock_irqrestore(&timeline->value_lock, flags);
}

u64 dpu_timeline_get_pt_value(struct dpu_timeline *timeline)
{
	u64 value;
	unsigned long flags;

	spin_lock_irqsave(&timeline->value_lock, flags);
	value = timeline->pt_value;
	spin_unlock_irqrestore(&timeline->value_lock, flags);

	return value;
}

struct timeline_listener *dpu_timeline_alloc_listener(struct timeline_listener_ops *listener_ops,
	void *listener_data, u64 value)
{
	struct timeline_listener *listener = NULL;

	listener = kzalloc(sizeof(*listener), GFP_KERNEL);
	if (!listener) {
		DPU_ERROR("listener kzalloc failed");
		return NULL;
	}
	listener->listener_data = listener_data;
	listener->ops = listener_ops;
	listener->pt_value = value;

	DPU_SYNC_DEBUG("create listener %pK at pt_value %llu", listener, value);

	return listener;
}
