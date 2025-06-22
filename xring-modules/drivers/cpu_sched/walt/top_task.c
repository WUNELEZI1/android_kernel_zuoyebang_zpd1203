// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "Walt top-task: " fmt

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pm_qos.h>

#include "walt.h"
#include "trace.h"

/* Size of bitmaps maintained to track top tasks */
static const unsigned int top_tasks_bitmap_size =
		BITS_TO_LONGS(NUM_LOAD_INDICES + 1) * sizeof(unsigned long);

/*
 * If modify top task bitshift,
 * please synchronize the parameter limits and initial value in sysctl.c
 */
#define XR_WALT_TOP_TASK_BITSHIFT 10
#define XR_WALT_TOP_TASK_FRAC (1U << XR_WALT_TOP_TASK_BITSHIFT)

static u32 load_to_index(u32 load)
{
	u32 index = load / sched_load_granule;

	return min(index, (u32)(NUM_LOAD_INDICES - 1));
}


void clear_top_tasks_bitmap(unsigned long *bitmap)
{
	memset(bitmap, 0, top_tasks_bitmap_size);
	__set_bit(NUM_LOAD_INDICES, bitmap);
}

static inline void clear_top_tasks_table(u8 *table)
{
	memset(table, 0, NUM_LOAD_INDICES * sizeof(u8));
}


static int get_top_index(unsigned long *bitmap, unsigned long old_top)
{
	int index = find_next_bit(bitmap, NUM_LOAD_INDICES, old_top);

	if (index == NUM_LOAD_INDICES)
		return 0;

	return NUM_LOAD_INDICES - 1 - index;
}

/*
 * Multiplies an integer by a fraction
 * Use Original mult_frac with binary shift operations.
 */
#define MULT_FRAC_BIT(x, numer, bitshift)(			\
{								\
	typeof(x) quot = (x) >> (bitshift);			\
	typeof(x) muti = 1;					\
	typeof(x) rem = (x) & ((muti << (bitshift)) - 1);	\
	(quot * (numer)) + ((rem * (numer)) >> (bitshift));	\
}								\
)

/*
 * Use a first-order filter for top task
 * ret = old + k * (new - old)
 * k is 0 ~ 1 in float, but 0 ~ 1024 in integer
 */
static inline u32 top_task_filter(u32 old, u32 new)
{
	u32 ret = old;
	u32 delta = 0;

	if (new >= old) {
		delta = MULT_FRAC_BIT(new - old,
				      sysctl_xr_top_task_filter_coef,
				      XR_WALT_TOP_TASK_BITSHIFT);
		ret += delta;
	} else {
		delta = MULT_FRAC_BIT(old - new,
				      sysctl_xr_top_task_filter_coef,
				      XR_WALT_TOP_TASK_BITSHIFT);
		ret -= delta;
	}

	return ret;
}

void update_top_tasks(struct task_struct *p, struct rq *rq,
		u32 old_curr_window, int new_window, bool full_window)
{
	struct walt_rq *wrq = &per_cpu(walt_rq, cpu_of(rq));
	struct walt_task_struct *wts = (struct walt_task_struct *) p->android_vendor_data1;
	u8 curr = wrq->curr_table;
	u8 prev = 1 - curr;
	u8 *curr_table = wrq->top_tasks[curr];
	u8 *prev_table = wrq->top_tasks[prev];
	int old_index, new_index, update_index;
	u32 curr_window = wts->curr_window;
	u32 prev_window = wts->prev_window;
	u32 hist_window_old = wts->history_window;
	int old_history_index;
	bool zero_index_update;

	if (old_curr_window == curr_window && !new_window)
		return;

	old_index = load_to_index(old_curr_window);
	new_index = load_to_index(curr_window);

	if (!new_window) {
		zero_index_update = !old_curr_window && curr_window;
		if (old_index != new_index || zero_index_update) {
			if (old_curr_window)
				curr_table[old_index] -= 1;
			if (curr_window)
				curr_table[new_index] += 1;
			if (new_index > wrq->curr_top)
				wrq->curr_top = new_index;
		}

		if (!curr_table[old_index])
			__clear_bit(NUM_LOAD_INDICES - old_index - 1,
				wrq->top_tasks_bitmap[curr]);

		if (curr_table[new_index] == 1)
			__set_bit(NUM_LOAD_INDICES - new_index - 1,
				wrq->top_tasks_bitmap[curr]);

		return;
	}

	/*
	 * The window has rolled over for this task. By the time we get
	 * here, curr/prev swaps would has already occurred. So we need
	 * to use prev_window for the new index.
	 */
	/*
	 * Use a first order filter on top task.
	 * if task not running in last window, use real value
	 */
	if (prev_window && hist_window_old)
		wts->history_window = top_task_filter(hist_window_old, prev_window);
	else
		wts->history_window = prev_window;

	update_index = load_to_index(wts->history_window);

	if (full_window) {
		/*
		 * Two cases here. Either 'p' ran for the entire window or
		 * it didn't run at all. In either case there is no entry
		 * in the prev table. If 'p' ran the entire window, we just
		 * need to create a new entry in the prev table. In this case
		 * update_index will be correspond to sched_ravg_window
		 * so we can unconditionally update the top index.
		 */
		if (prev_window) {
			prev_table[update_index] += 1;
			wrq->prev_top = update_index;
		}

		if (prev_table[update_index] == 1)
			__set_bit(NUM_LOAD_INDICES - update_index - 1,
				wrq->top_tasks_bitmap[prev]);

	} else {
		zero_index_update = !old_curr_window && prev_window;
		if (old_index != update_index || zero_index_update) {
			if (old_curr_window)
				prev_table[old_index] -= 1;

			prev_table[update_index] += 1;

			if (update_index > wrq->prev_top)
				wrq->prev_top = update_index;

			if (!prev_table[old_index])
				__clear_bit(NUM_LOAD_INDICES - old_index - 1,
						wrq->top_tasks_bitmap[prev]);

			if (prev_table[update_index] == 1)
				__set_bit(NUM_LOAD_INDICES - update_index - 1,
						wrq->top_tasks_bitmap[prev]);
		}
	}

	if (curr_window) {
		curr_table[new_index] += 1;

		if (new_index > wrq->curr_top)
			wrq->curr_top = new_index;

		if (curr_table[new_index] == 1)
			__set_bit(NUM_LOAD_INDICES - new_index - 1,
				wrq->top_tasks_bitmap[curr]);
	}

	old_history_index = load_to_index(hist_window_old);
	trace_xr_top_task(smp_processor_id(), p,
		old_history_index, update_index, old_curr_window, curr_window, prev_window, wts->history_window);

	/*
	 * if new top task window is quite large, trigger freq boost
	 * immediately.
	 */
	if (old_history_index &&
	    old_history_index + sysctl_xr_top_task_boost <= update_index) {
		xresgov_run_callback(cpu_of(rq), WALT_TOP_TASK_BOOST);
		trace_xr_ed_top_task_boost(p, cpu_of(rq), WALT_TOP_TASK_BOOST);
	}
}

void rollover_top_tasks(struct rq *rq, bool full_window)
{
	struct walt_rq *wrq = &per_cpu(walt_rq, cpu_of(rq));
	u8 curr_table = wrq->curr_table;
	u8 prev_table = 1 - curr_table;
	int curr_top = wrq->curr_top;

	clear_top_tasks_table(wrq->top_tasks[prev_table]);
	clear_top_tasks_bitmap(wrq->top_tasks_bitmap[prev_table]);

	if (full_window) {
		curr_top = 0;
		clear_top_tasks_table(wrq->top_tasks[curr_table]);
		clear_top_tasks_bitmap(wrq->top_tasks_bitmap[curr_table]);
	}

	wrq->curr_table = prev_table;
	wrq->prev_top = curr_top;
	wrq->curr_top = 0;
}

void migrate_top_tasks_subtraction(struct task_struct *p, struct rq *src_rq)
{
	int index;
	int top_index;
	struct walt_task_struct *wts = (struct walt_task_struct *) p->android_vendor_data1;
	u32 curr_window = wts->curr_window;
	u32 prev_window = wts->prev_window;
	struct walt_rq *src_wrq = &per_cpu(walt_rq, cpu_of(src_rq));
	u8 src = src_wrq->curr_table;
	u8 *src_table;

	if (curr_window) {
		src_table = src_wrq->top_tasks[src];
		index = load_to_index(curr_window);
		src_table[index] -= 1;

		if (!src_table[index])
			__clear_bit(NUM_LOAD_INDICES - index - 1,
				src_wrq->top_tasks_bitmap[src]);

		top_index = src_wrq->curr_top;
		if (index == top_index && !src_table[index])
			src_wrq->curr_top = get_top_index(
				src_wrq->top_tasks_bitmap[src], top_index);
	}

	if (prev_window) {
		src = 1 - src;
		src_table = src_wrq->top_tasks[src];
		index = load_to_index(prev_window);
		src_table[index] -= 1;

		if (!src_table[index])
			__clear_bit(NUM_LOAD_INDICES - index - 1,
				src_wrq->top_tasks_bitmap[src]);

		top_index = src_wrq->prev_top;
		if (index == top_index && !src_table[index])
			src_wrq->prev_top = get_top_index(
				src_wrq->top_tasks_bitmap[src], top_index);
	}
}

void migrate_top_tasks_addition(struct task_struct *p, struct rq *rq)
{
	int index;
	struct walt_task_struct *wts = (struct walt_task_struct *) p->android_vendor_data1;
	u32 curr_window = wts->curr_window;
	u32 prev_window = wts->prev_window;
	struct walt_rq *dst_wrq = &per_cpu(walt_rq, cpu_of(rq));
	u8 dst = dst_wrq->curr_table;
	u8 *dst_table;

	if (curr_window) {
		dst_table = dst_wrq->top_tasks[dst];
		index = load_to_index(curr_window);
		dst_table[index] += 1;

		if (dst_table[index] == 1)
			__set_bit(NUM_LOAD_INDICES - index - 1,
				dst_wrq->top_tasks_bitmap[dst]);

		if (index > dst_wrq->curr_top)
			dst_wrq->curr_top = index;
	}

	if (prev_window) {
		dst = 1 - dst;
		dst_table = dst_wrq->top_tasks[dst];
		index = load_to_index(prev_window);
		dst_table[index] += 1;

		if (dst_table[index] == 1)
			__set_bit(NUM_LOAD_INDICES - index - 1,
				dst_wrq->top_tasks_bitmap[dst]);

		if (index > dst_wrq->prev_top)
			dst_wrq->prev_top = index;
	}
}

/*
 * Special case the last index and provide a fast path for index = 0.
 * Note that sched_load_granule can change underneath us if we are not
 * holding any runqueue locks while calling the two functions below.
 */
u32 top_task_load(struct rq *rq)
{
	struct walt_rq *wrq = &per_cpu(walt_rq, cpu_of(rq));
	int index = wrq->prev_top;
	u8 prev = 1 - wrq->curr_table;

	if (!index) {
		int msb = NUM_LOAD_INDICES - 1;

		if (!test_bit(msb, wrq->top_tasks_bitmap[prev]))
			return 0;
		else
			return sched_load_granule;

	} else if (index == NUM_LOAD_INDICES - 1) {
		return sched_ravg_window;
	} else {
		return (index + 1) * sched_load_granule;
	}
}
