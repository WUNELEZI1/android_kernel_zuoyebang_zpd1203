// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/sort.h>
#include <linux/bitmap.h>
#include "rt_tracker.h"

/* global variable for sort */
static unsigned int g_sort_buf[MAX_RTHREAD_NUM];
static u8 *g_waker_count;
static struct rthread_frame *g_sort_frame;

/*
 * Descending order by frame_load
 *
 * Ascending Return: 0 if both are equal, -1 if the first is smaller, else 1
 * Descending Return: 0 if both are equal, 1 if the first is smaller, else -1
 */
static int cmp_load(const void *ax, const void *bx)
{
	u64 prev, next;

	prev = (u64)(*((unsigned int *)ax));
	next = (u64)(*((unsigned int *)bx));

	if (unlikely(g_sort_frame == NULL ||
		     prev >= MAX_RTHREAD_NUM || next >= MAX_RTHREAD_NUM))
		return 0;

	prev = g_sort_frame[prev].frame_load;
	next = g_sort_frame[next].frame_load;

	return (prev > next ? -1 : !!(next - prev));
}

/*
 * Descending order by wake count
 *
 * Ascending Return: 0 if both are equal, -1 if the first is smaller, else 1
 * Descending Return: 0 if both are equal, 1 if the first is smaller, else -1
 */
static int cmp_waker(const void *ax, const void *bx)
{
	unsigned int prev, next;

	prev = *((unsigned int *)ax);
	next = *((unsigned int *)bx);

	if (unlikely(g_waker_count == NULL ||
		     prev >= MAX_RTHREAD_NUM || next >= MAX_RTHREAD_NUM))
		return 0;

	prev = g_waker_count[prev];
	next = g_waker_count[next];

	return (prev > next ? -1 : !!(next - prev));
}

static inline void set_thread_result(struct rthread_frame *frame,
				     struct thread_util *result,
				     u64 window_size)
{
	unsigned int util = (unsigned int)(frame->frame_load / window_size);

	result->pid = frame->pid;
	result->util = clamp_val(util, 1, 1024);
	result->woken_count = frame->woken_count;
}

static inline void set_invalid_thread_result(struct thread_util *result)
{
	result->pid = 0;
	result->util = 0;
	result->woken_count = 0;
}

/*
 * get_top_rthread_info() - get top threads by util
 *
 */
static void get_top_rthread_info(struct related_thread_tracker *tracker,
				 struct rt_info *result)
{
	struct thread_util *rthread_utils = result->utils;
	struct rthread_frame *frame = tracker->prev;
	unsigned int i, thread_num;

	thread_num = 0;

	for_each_set_bit(i, tracker->frame_bitmap, MAX_RTHREAD_NUM) {
		if (frame[i].frame_load < tracker->prev_window_size)
			continue;

		g_sort_buf[thread_num] = i;
		thread_num++;
	}

	g_sort_frame = tracker->prev;
	sort(g_sort_buf, thread_num, sizeof(g_sort_buf[0]), cmp_load, NULL);

	for (i = 0; i < MAX_UTIL_NUM; i++) {
		if (i < thread_num)
			set_thread_result(&frame[g_sort_buf[i]], rthread_utils,
					  tracker->prev_window_size);
		else
			set_invalid_thread_result(rthread_utils);

		rthread_utils++;
	}

	result->num = thread_num;
}

static void get_rthread_waker(struct related_thread_tracker *tracker,
			      unsigned int *rthread_results,
			      unsigned int index, int mode)
{
	unsigned int i, waker_idx, num = 0;
	DECLARE_BITMAP(valid_bitmap, MAX_RTHREAD_NUM);

	bitmap_and(valid_bitmap, tracker->prev[index].waker_bitmap,
		   tracker->visit_bitmap, MAX_RTHREAD_NUM);

	for_each_set_bit(i, valid_bitmap, MAX_RTHREAD_NUM) {
		g_sort_buf[num] = i;
		num++;
	}

	/*
	 * need to sort the wakers by demand only if
	 * the number is bigger than MAX_BREADTH_NUM
	 */
	if (num > MAX_BREADTH_NUM) {
		g_waker_count = tracker->prev[index].waker_count;
		g_sort_frame = tracker->prev;
		sort(g_sort_buf, num, sizeof(g_sort_buf[0]),
		     mode == RT_MODE ? cmp_waker : cmp_load, NULL);
	}

	for (i = 0; i < min(num, MAX_BREADTH_NUM); i++) {
		waker_idx = g_sort_buf[i];
		__clear_bit(waker_idx, tracker->visit_bitmap);
		rthread_results[i] = waker_idx;
	}
}

static void get_closest_rthread_info(struct related_thread_tracker *tracker,
				     struct rt_info *result)
{
	unsigned int i, rthread_idx, result_idx;
	unsigned int rthread_results[MAX_UTIL_NUM];
	struct thread_util *rthread_utils = result->utils;
	unsigned int thread_num = 0;

	/* reset result */
	for (i = 0; i < MAX_UTIL_NUM; i++)
		rthread_results[i] = MAX_RTHREAD_NUM;

	bitmap_copy(tracker->visit_bitmap,
		    tracker->frame_bitmap, MAX_RTHREAD_NUM);

	/* tracker thread always use the first slot */
	__clear_bit(0, tracker->visit_bitmap);
	rthread_results[0] = 0;
	result_idx = 1;

	/*  */
	for (i = 0; i < MAX_EXPEND_NUM; i++) {
		rthread_idx = rthread_results[i];
		if (rthread_idx < MAX_RTHREAD_NUM) {
			get_rthread_waker(tracker, &rthread_results[result_idx],
					  rthread_idx, result->mode);
		}
		result_idx += MAX_BREADTH_NUM;
	}

	for (i = 0; i < MAX_UTIL_NUM; i++) {
		rthread_idx = rthread_results[i];
		if (rthread_idx < MAX_RTHREAD_NUM) {
			set_thread_result(&tracker->prev[rthread_idx],
					  rthread_utils, tracker->prev_window_size);
			thread_num++;
		} else {
			set_invalid_thread_result(rthread_utils);
		}
		rthread_utils++;
	}

	result->num = thread_num;
}

int get_rthread_info(struct related_thread_tracker *tracker,
		     struct rt_info *result)
{
	int ret = 0;

	if (result->mode == UPDATE_MODE)
		return 0;

	if (tracker->prev == NULL)
		return -EAGAIN;

	switch (result->mode) {
	case HT_MODE:
		get_top_rthread_info(tracker, result);
		break;
	case RHT_MODE:
	case RT_MODE:
		get_closest_rthread_info(tracker, result);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}
