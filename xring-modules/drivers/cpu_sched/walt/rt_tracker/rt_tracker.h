/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#ifndef __RT_TRACKER_H__
#define __RT_TRACKER_H__

#include <linux/types.h>
#include <linux/rbtree.h>
#include <linux/sched.h>

#define MAX_FRAME_NUM		2U
#define MAX_BREADTH_NUM		4U
#define MAX_EXPEND_NUM		(MAX_BREADTH_NUM + 1U)
#define MAX_RTHREAD_NUM		256U
#define MAX_UTIL_NUM		(MAX_BREADTH_NUM * MAX_BREADTH_NUM + MAX_EXPEND_NUM)
#define MAX_WAKE_TIMES		255U

#define UPDATE_MODE	0
#define HT_MODE		1
#define RHT_MODE	2
#define RT_MODE		3

/*
 * pid == 0 indicates invalid entry.
 * util should be in range of 0 to 1024
 */
struct thread_util {
	pid_t pid;
	unsigned int util;
	unsigned int woken_count;
};

struct rt_info {
	pid_t target_pid;
	int update;
	int mode;
	unsigned int num;
	struct thread_util utils[MAX_UTIL_NUM];
};

/*
 * rthread: used to save the wakeup relationship
 * @pid: the pid of the rthread
 * @task: pointer to task_struct of the rthread
 * @lock: protect the fields in the structure
 * @exited:  flag to mark the task is destroyed
 * @last_load_sum: the normalized load of the last frame
 * @last_exec_sum: the execute time of the last frame
 */
struct rthread {
	unsigned int idx;
	pid_t pid;
	struct rb_node node;
	struct task_struct *task;

	raw_spinlock_t lock;
	bool exited;
	/* fields update every frame */
	u64 last_load_sum;
	u64 last_exec_sum;
};

/*
 * @pid: the pid of the rthread
 * @frame_util: the normalized util of last frame
 * @frame_runtime: the running time of last frame
 * @waker_bitmap: waker's index bitmap
 * @waker_count: wake count of each waker task
 * @woken_count: total woken time by related threads
 */
struct rthread_frame {
	pid_t pid;
	u64 frame_load;
	u64 frame_runtime;
	DECLARE_BITMAP(waker_bitmap, MAX_RTHREAD_NUM);
	u8 waker_count[MAX_RTHREAD_NUM];
	unsigned int woken_count;
};

/*
 * related_thread_tracker: used to save the wakeup relationship
 * @pid: the pid of the tracker
 * @task: task_struct pointer of the tracker
 * @root: rbtree root node
 * @window_start: frame start timestamp
 * @curr: curr frame pointer
 * @prev: prev frame pointer
 * @visit_bitmap: visit map
 * @frame_bitmap: prev frame thread bitmap
 * @thread_bitmap: thread index bitmap of related threads
 * @related_threads: save all the related tasks of tracker
 * @frame_info: frame info save space
 */
struct related_thread_tracker {
	pid_t pid;
	struct task_struct *task;
	struct rb_root root;

	u64 window_start;
	u64 prev_window_size;
	struct rthread_frame *curr;
	struct rthread_frame *prev;
	DECLARE_BITMAP(visit_bitmap, MAX_RTHREAD_NUM);
	DECLARE_BITMAP(frame_bitmap, MAX_RTHREAD_NUM);
	DECLARE_BITMAP(thread_bitmap, MAX_RTHREAD_NUM);
	struct rthread related_threads[MAX_RTHREAD_NUM];
	struct rthread_frame frame_info[MAX_FRAME_NUM][MAX_RTHREAD_NUM];
};

/* rt_tracker_info function */
int get_rthread_info(struct related_thread_tracker *tracker,
		     struct rt_info *result);

/* rt_tracker_ops function */
struct rthread *get_rthread(struct related_thread_tracker *tracker,
			    struct task_struct *task);
struct rthread *insert_rthread(struct related_thread_tracker *tracker,
			       struct task_struct *task);
void update_wake_count(struct related_thread_tracker *tracker,
		       struct rthread *wakee_thread,
		       unsigned int waker_index);
void update_rthread_load(struct related_thread_tracker *tracker,
			 struct rthread *thread);
void rollover_frame_info(struct related_thread_tracker *tracker);

#endif /* __RT_TRACKER_H__ */
