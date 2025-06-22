// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/minmax.h>
#include <linux/sched.h>
#include <soc/xring/walt.h>
#include "rt_tracker.h"

#define CREATE_TRACE_POINTS
#include "trace_rt_tracker.h"

/*
 * get_rthread() - binary search the index of the task
 * @tracker: tracker for searching
 * @task: task to search in related_threads tree
 *
 * Returns MAX_RTHREAD_NUM if the @task cannot found
 */
struct rthread *get_rthread(struct related_thread_tracker *tracker,
			    struct task_struct *task)
{
	struct rthread *thread = NULL;
	struct rb_node *node = tracker->root.rb_node;
	pid_t target_pid = task->pid;

	while (node) {
		thread = container_of(node, struct rthread, node);

		if (target_pid < thread->pid)
			node = node->rb_left;
		else if (target_pid > thread->pid)
			node = node->rb_right;
		else
			return thread;
	}
	return NULL;
}

static inline void init_rthread_frame(struct related_thread_tracker *tracker,
				      unsigned int idx)
{
	struct rthread_frame *frame = &tracker->curr[idx];

	frame->frame_load = 0;
	frame->frame_runtime = 0;
	frame->woken_count = 0;
	/* waker_count will init zero when set waker_bitmap,
	 * only need to clear waker_bitmap here
	 */
	bitmap_zero(frame->waker_bitmap, MAX_RTHREAD_NUM);
}

static inline void init_rthread(struct rthread *thread,
				struct task_struct *p, unsigned int idx)
{
	raw_spin_lock_init(&thread->lock);
	thread->idx = idx;
	thread->task = p;
	thread->pid = p->pid;
	thread->exited = false;
	xr_get_task_running_sum(p, &thread->last_load_sum,
				&thread->last_exec_sum);
}

struct rthread *insert_rthread(struct related_thread_tracker *tracker,
			       struct task_struct *task)
{
	struct rb_node **new = &(tracker->root.rb_node), *parent = NULL;
	pid_t target_pid = task->pid;
	struct rthread *thread = NULL;
	unsigned int idx;

	/* Figure out where to put new node */
	while (*new) {
		thread = container_of(*new, struct rthread, node);

		parent = *new;
		if (target_pid < thread->pid)
			new = &((*new)->rb_left);
		else if (target_pid > thread->pid)
			new = &((*new)->rb_right);
		else if (thread->exited)
			return NULL;
		else
			return thread;
	}

	idx = find_first_zero_bit(tracker->thread_bitmap, MAX_RTHREAD_NUM);
	if (idx == MAX_RTHREAD_NUM)
		return NULL;

	__set_bit(idx, tracker->thread_bitmap);

	thread = &tracker->related_threads[idx];

	/* Add new node and rebalance tree. */
	rb_link_node(&thread->node, parent, new);
	rb_insert_color(&thread->node, &tracker->root);

	/* init rthread data */
	init_rthread(thread, task, idx);
	init_rthread_frame(tracker, idx);
	trace_add_rthread(task);

	return thread;
}

void update_wake_count(struct related_thread_tracker *tracker,
		       struct rthread *wakee_thread,
		       unsigned int waker_index)
{
	struct rthread_frame *frame = &tracker->curr[wakee_thread->idx];

	raw_spin_lock(&wakee_thread->lock);

	if (!test_bit(waker_index, frame->waker_bitmap)) {
		frame->waker_count[waker_index] = 1;
		__set_bit(waker_index, frame->waker_bitmap);
	} else if (frame->waker_count[waker_index] < MAX_WAKE_TIMES) {
		frame->waker_count[waker_index]++;
	}

	trace_update_rthread_wake_count(current, wakee_thread->task,
					frame->waker_count[waker_index]);

	frame->woken_count++;

	raw_spin_unlock(&wakee_thread->lock);
}

static inline u64 safe_sub(u64 a, u64 b)
{
	if (unlikely(a < b))
		return (U64_MAX - b) + 1 + a;

	return a - b;
}

void update_rthread_load(struct related_thread_tracker *tracker,
			 struct rthread *thread)
{
	struct rthread_frame *frame = &tracker->curr[thread->idx];
	u64 curr_load_sum, curr_exec_sum;

	xr_get_task_running_sum(thread->task, &curr_load_sum, &curr_exec_sum);

	frame->frame_runtime = safe_sub(curr_exec_sum, thread->last_exec_sum);
	thread->last_exec_sum = curr_exec_sum;

	frame->frame_load = safe_sub(curr_load_sum, thread->last_load_sum);
	thread->last_load_sum = curr_load_sum;

	frame->pid = thread->pid;

	trace_update_rthread_load(thread->task, frame->frame_load,
				  frame->frame_runtime);
}

/*
 * rollover_frame_info()
 * update util of all related threads, window_start, then switch frame
 */
void rollover_frame_info(struct related_thread_tracker *tracker)
{
	struct rthread *thread = NULL;
	u64 curr_ktime, window_size;
	int index;

	/*
	 * check window size, rollover will not happen when
	 * window size is too small
	 */
	curr_ktime = walt_sched_clock();
	window_size = safe_sub(curr_ktime, tracker->window_start);
	window_size >>= SCHED_CAPACITY_SHIFT;
	if (window_size == 0)
		return;

	walt_update_all_cpu(curr_ktime);

	/* update window_start and window size */
	tracker->prev_window_size = window_size;
	tracker->window_start = curr_ktime;

	/* save curr frame thread bitmap */
	bitmap_copy(tracker->frame_bitmap,
		    tracker->thread_bitmap, MAX_RTHREAD_NUM);

	/* update curr frame util */
	for_each_set_bit(index, tracker->thread_bitmap, MAX_RTHREAD_NUM) {
		thread = &tracker->related_threads[index];

		/* The exited thread load have already updated */
		if (thread->exited) {
			/* remove the exited task */
			rb_erase(&thread->node, &tracker->root);
			__clear_bit(index, tracker->thread_bitmap);
			thread->exited = false;
			continue;
		}

		update_rthread_load(tracker, thread);
	}

	/* switch frame */
	tracker->prev = tracker->curr;
	index = (tracker->curr == tracker->frame_info[0]) ? 1 : 0;
	tracker->curr = tracker->frame_info[index];

	/* reset next frame */
	for_each_set_bit(index, tracker->thread_bitmap, MAX_RTHREAD_NUM) {
		init_rthread_frame(tracker, index);
	}

	trace_rollover_frame(window_size);
}
