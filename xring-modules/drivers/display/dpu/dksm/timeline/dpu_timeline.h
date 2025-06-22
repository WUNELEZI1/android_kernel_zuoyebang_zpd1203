/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef TIMELINE_H
#define TIMELINE_H

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kref.h>
#include <linux/kthread.h>

#include "dpu_timeline_listener.h"
#include "dpu_isr.h"

#define SYNC_NAME_SIZE 256

/**
 * @brief release fence is created on (pt_value + next_value), and signaled on pt_value
 * The relationship between them is :
 *     pt_value += ++next_value
 */
struct dpu_timeline {
	struct kref kref;
	char name[SYNC_NAME_SIZE];
	u32 listening_id;
	u32 isr_unmask_bit;

	void *parent;
	void *isr;

	/**
	 * @brief These values relationship between them is as follows: (online mode)
	 *
	 *             new_frame                 new_frame
	 *                |                         |
	 *        vsync    \                 vsync   \                  vsync
	 *          ^       \                  ^      \                   ^
	 * timelin _|__################___..___|__################___..___|__
	 *          |       |             |    |                     |    `--- pt=n+2
	 *          |  *create fence      |     `--- pt=n+1          |         inc=0
	 *          |       |              ` pt=n    inc=0           `pt=n+1   next=0
	 *        pt=n       `- pt=n         next=1  next=0           next++   *singal fence*
	 *        next=0        next++       inc=1                    inc=1
	 *        inc=0         inc=0
	 *
	 *  1. New frame's fence is created based on pt_value and next_value. (default value is 0,
	 *     but sometimes it will be greater than 1 in asynchronous processing).
	 *  2. The next_value will increase by 1 every time a fence is created.
	 *  3. The inc_step will increase by 1 every time a new frame takes effect.
	 *  4. The pt_value will increase by inc_step every time vsync comes.
	 *
	 *  5. The fence will be signaled based on pt_value being greater than fence seqno.
	 *
	 */
	spinlock_t value_lock;
	u64 pt_value;
	u32 next_value;
	u32 inc_step;
	u64 context;
	u32 need_recovery;
	bool need_dump;
	u64 notify_timestamp;
	/* pending frame num need do inc */
	atomic_t pending_frame_num;

	/* isr will notify timeline to handle event */
	struct notifier_block *notifier;

	struct mutex list_lock;
	/* list to struct timeline_listener */
	struct list_head listener_list;

	struct kthread_worker handle_worker;
	struct kthread_work timeline_sync_work;
	struct task_struct *timeline_task;
};

static inline void dpu_timeline_add_listener(struct dpu_timeline *timeline, struct timeline_listener *node)
{
	mutex_lock(&timeline->list_lock);
	list_add_tail(&node->list_node, &timeline->listener_list);
	mutex_unlock(&timeline->list_lock);
}

static inline void timeline_dec_next_value(struct dpu_timeline *timeline)
{
	unsigned long flags;

	spin_lock_irqsave(&timeline->value_lock, flags);
	if (timeline->next_value > 0)
		--timeline->next_value;
	spin_unlock_irqrestore(&timeline->value_lock, flags);
}

static inline u64 timeline_get_next_value(struct dpu_timeline *timeline)
{
	unsigned long flags;
	u64 value;

	spin_lock_irqsave(&timeline->value_lock, flags);
	++timeline->next_value;
	value = timeline->pt_value + timeline->next_value;
	spin_unlock_irqrestore(&timeline->value_lock, flags);

	return value;
}

static inline void timeline_inc_step(struct dpu_timeline *timeline)
{
	unsigned long flags;

	spin_lock_irqsave(&timeline->value_lock, flags);
	if (timeline->next_value > 0)
		++timeline->inc_step;
	spin_unlock_irqrestore(&timeline->value_lock, flags);
}

static inline void timeline_force_signal_set(struct dpu_timeline *timeline, u32 value)
{
	timeline->need_recovery = value;
}

static inline void timeline_dump_flag_set(struct dpu_timeline *timeline, bool value)
{
	timeline->need_dump = value;
}

u64 dpu_timeline_get_pt_value(struct dpu_timeline *timeline);

struct timeline_listener *dpu_timeline_alloc_listener(
		struct timeline_listener_ops *listener_ops,
		void *listener_data, u64 value);

int dpu_timeline_init(struct dpu_timeline *timeline,
		const char *name, void *parent, u32 listening_bit, struct dpu_isr *isr);
void dpu_timeline_deinit(struct dpu_timeline *timeline, struct dpu_isr *isr);

void dpu_timeline_resync_pt(struct dpu_timeline *timeline, u32 value);
void dpu_timeline_dump(struct dpu_timeline *timeline);

#endif /* DKMD_TIMELINE_H */
