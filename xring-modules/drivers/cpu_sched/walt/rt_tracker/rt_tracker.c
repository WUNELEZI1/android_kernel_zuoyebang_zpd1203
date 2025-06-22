// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/sched.h>
#include <trace/hooks/sched.h>
#include <soc/xring/walt.h>
#include <soc/xring/perf_actuator.h>
#include "rt_tracker.h"
#include "trace_rt_tracker.h"

#define CMD_START_RT_TRACKER	PERF_W_CMD(START_RT_TRACKER, pid_t)
#define CMD_STOP_RT_TRACKER	PERF_W_CMD(STOP_RT_TRACKER, pid_t)
#define CMD_GET_RT_INFO		PERF_RW_CMD(GET_RT_INFO, struct rt_info)

/* g_rt_rwlock --the global lock for struct related_thread_tracker */
static DEFINE_RWLOCK(g_rt_rwlock);

/* copy_to/form_user may sleep, hold mutex lock to protect result */
static DEFINE_MUTEX(g_result_lock);

/* use global variable to keep result, avoid frequently memory alloc */
static struct rt_info g_rt_result;

static struct related_thread_tracker g_rt_tracker;

/* g_rt_tracker_ptr: pointer to global data of the module */
static struct related_thread_tracker *g_rt_tracker_ptr __read_mostly;

static inline struct related_thread_tracker *get_rt_tracker(void)
{
	return g_rt_tracker_ptr;
}

static inline void set_rt_tracker(struct related_thread_tracker *tracker)
{
	if (tracker == NULL)
		trace_stop_rt_tracker(g_rt_tracker_ptr->task);

	g_rt_tracker_ptr = tracker;
}

static inline void reset_rt_tracker(struct related_thread_tracker *tracker,
				    struct task_struct *task)
{
	/* clear all the members */
	tracker->task = task;
	tracker->pid = task->pid;
	tracker->root = RB_ROOT;
	tracker->curr = tracker->frame_info[0];
	tracker->prev = NULL;
	bitmap_zero(tracker->thread_bitmap, MAX_RTHREAD_NUM);

	/* mark the start timestamp */
	tracker->window_start = walt_sched_clock();
}

static inline bool is_rthread(struct related_thread_tracker *tracker,
			      struct task_struct *task)
{
	return same_thread_group(task, tracker->task);
}

/*
 * both_rthread() - test if the two tasks are rthread
 * @a: pointer to one of the two tasks
 * @b: pointer to one of the two tasks
 */
static inline bool both_rthread(struct related_thread_tracker *tracker,
				struct task_struct *a, struct task_struct *b)
{
	return is_rthread(tracker, a) && is_rthread(tracker, b);
}

/*
 * Add the task to sorted related_threads space
 */
void rt_tracker_add_thread(struct task_struct *task)
{
	struct related_thread_tracker *tracker;
	struct rthread *rthread = NULL;
	struct rthread *waker_thread = NULL;
	unsigned long flags;
	int reason = 0;

	if (oops_in_progress || task == NULL)
		return;

	write_lock_irqsave(&g_rt_rwlock, flags);

	tracker = get_rt_tracker();
	if (tracker == NULL)
		goto err;

	if (!is_rthread(tracker, task))
		goto err;

	rthread = insert_rthread(tracker, task);
	if (rthread == NULL) {
		reason = 1;
		goto err;
	}

	if (!is_rthread(tracker, current))
		goto err;

	waker_thread = insert_rthread(tracker, current);
	if (unlikely(waker_thread == NULL)) {
		reason = 2;
		goto err;
	}

	update_wake_count(tracker, rthread, waker_thread->idx);
err:
	write_unlock_irqrestore(&g_rt_rwlock, flags);

	if (reason)
		pr_warn_ratelimited("Error [add rthread] reason=%d\n", reason);
}

/*
 * mark the task is exited and update the util of the task
 *
 * just set the flag to indicates the task is exited,
 * the actual remove operation to be done in rollover_frame_info function.
 */
void rt_tracker_remove_thread(struct task_struct *task)
{
	struct related_thread_tracker *tracker;
	struct rthread *exited_thread = NULL;
	unsigned long flags;
	int reason = 0;

	if (oops_in_progress || task == NULL)
		return;

	write_lock_irqsave(&g_rt_rwlock, flags);

	tracker = get_rt_tracker();
	if (tracker == NULL)
		goto err;

	/* when the tracker thread exit, disable tracker */
	if (task == tracker->task ||
	    task->pid == tracker->pid) {
		set_rt_tracker(NULL);
		goto err;
	}

	if (!is_rthread(tracker, task))
		goto err;

	exited_thread = get_rthread(tracker, task);
	if (exited_thread == NULL) {
		reason = 1;
		goto err;
	}

	if (exited_thread->exited) {
		reason = 2;
		goto err;
	}

	update_rthread_load(tracker, exited_thread);
	exited_thread->exited = true;
	trace_remove_rthread(task);
err:
	write_unlock_irqrestore(&g_rt_rwlock, flags);

	if (reason)
		pr_warn_ratelimited("[remove rthread] reason=%d pid=%d\n",
				    reason, task->pid);
}

/*
 * update waker count of wakee's info
 */
void rt_tracker_update_wake_count(struct task_struct *task)
{
	struct related_thread_tracker *tracker;
	struct rthread *waker_thread = NULL;
	struct rthread *wakee_thread = NULL;
	unsigned long flags;
	int fail_reason = 0;
	bool rthread_full;
	ktime_t start;

	if (oops_in_progress || task == NULL)
		return;

	start = ktime_get();
	read_lock_irqsave(&g_rt_rwlock, flags);
	tracker = get_rt_tracker();
	if (tracker == NULL)
		goto err;

	/* ignore wakeup event if waker or wakee is not a rthread */
	if (!both_rthread(tracker, current, task))
		goto err;

	rthread_full = bitmap_full(tracker->thread_bitmap, MAX_RTHREAD_NUM);
	wakee_thread = get_rthread(tracker, task);
	if (unlikely(wakee_thread == NULL)) {
		fail_reason = 1;
		goto err;
	}

	waker_thread = get_rthread(tracker, current);
	if (unlikely(waker_thread == NULL)) {
		fail_reason = 2;
		goto err;
	}

	update_wake_count(tracker, wakee_thread, waker_thread->idx);
err:
	read_unlock_irqrestore(&g_rt_rwlock, flags);
	trace_try_update_wake(ktime_us_delta(ktime_get(), start));

	/*
	 * if update success or rthread array is full,
	 * no need to try insert rthread
	 */
	if (fail_reason == 0 || rthread_full)
		return;

	/* try insert rthread if not found in rthread */
	start = ktime_get();
	write_lock_irqsave(&g_rt_rwlock, flags);
	if (wakee_thread == NULL)
		wakee_thread = insert_rthread(tracker, task);
	if (waker_thread == NULL)
		waker_thread = insert_rthread(tracker, current);

	if (wakee_thread != NULL && waker_thread != NULL)
		update_wake_count(tracker, wakee_thread,
				  waker_thread->idx);

	write_unlock_irqrestore(&g_rt_rwlock, flags);
	trace_insert_update_wake(ktime_us_delta(ktime_get(), start));
}

/*
 * rt_tracker_get_info() - get related thread info
 *
 * update the util of last frame if needed
 * get tracker info and save the results
 */
static int rt_tracker_get_info(void __user *uarg)
{
	struct related_thread_tracker *tracker;
	unsigned long flags;
	int ret = 0;
	ktime_t start;

	mutex_lock(&g_result_lock);
	if (copy_from_user(&g_rt_result, uarg,
			   offsetof(struct rt_info, utils))) {
		ret = -EFAULT;
		goto result_unlock;
	}

	start = ktime_get();
	write_lock_irqsave(&g_rt_rwlock, flags);

	tracker = get_rt_tracker();
	if (tracker == NULL) {
		ret = -ENOENT;
		goto err;
	}

	if (g_rt_result.target_pid != tracker->pid) {
		ret = -ENODEV;
		goto err;
	}

	if (g_rt_result.update || tracker->prev == NULL)
		rollover_frame_info(tracker);

	ret = get_rthread_info(tracker, &g_rt_result);
err:
	write_unlock_irqrestore(&g_rt_rwlock, flags);

	if (ret == 0) {
		trace_get_rt_info(ktime_us_delta(ktime_get(), start));
		if (copy_to_user(uarg, &g_rt_result,
				 sizeof(struct rt_info)))
			ret = -EIO;
	}

result_unlock:
	mutex_unlock(&g_result_lock);

	if (ret < 0)
		pr_err_ratelimited("[get rt info] ret=%d\n", ret);

	return ret;
}

/*
 * rt_tracker_start() - initialize the tracker
 *
 * if tracker has been started before, the function will
 * restart the tracker any way.
 */
static int rt_tracker_start(void __user *uarg)
{
	struct related_thread_tracker *tracker = NULL;
	struct rthread *rthread = NULL;
	struct task_struct *leader = NULL;
	unsigned long flags;
	pid_t target_pid;
	int ret = 0;

	if (copy_from_user(&target_pid, uarg, sizeof(pid_t)))
		return -EFAULT;

	write_lock_irqsave(&g_rt_rwlock, flags);

	rcu_read_lock();
	leader = find_task_by_vpid(target_pid);
	if (leader == NULL) {
		ret = -ENODEV;
		goto err;
	}

	tracker = &g_rt_tracker;
	reset_rt_tracker(tracker, leader);
	set_rt_tracker(tracker);
	rthread = insert_rthread(tracker, leader);
	if (rthread == NULL)
		ret = -ENOMEM;
err:
	rcu_read_unlock();
	write_unlock_irqrestore(&g_rt_rwlock, flags);
	if (ret != 0)
		pr_err("[init tracker] ret=%d\n", ret);

	return ret;
}

/*
 * rt_tracker_stop() - stop the tracker
 */
static int rt_tracker_stop(void __user *uarg)
{
	struct related_thread_tracker *tracker;
	pid_t destroy_pid;
	unsigned long flags;
	int ret = 0;

	if (copy_from_user(&destroy_pid, uarg, sizeof(pid_t))) {
		pr_err("related_tid copy_from_user fail.\n");
		return -EFAULT;
	}

	write_lock_irqsave(&g_rt_rwlock, flags);
	tracker = get_rt_tracker();
	if (tracker == NULL) {
		ret = -ENOENT;
		goto unlock;
	}

	if (destroy_pid != tracker->pid) {
		ret = -ENODEV;
		goto unlock;
	}

	set_rt_tracker(NULL);
unlock:
	write_unlock_irqrestore(&g_rt_rwlock, flags);
	if (ret != 0)
		pr_err("[destroy tracker] ret=%d\n", ret);

	return ret;
}

void rt_tracker_init(void)
{
	register_perf_actuator(CMD_GET_RT_INFO, rt_tracker_get_info);
	register_perf_actuator(CMD_STOP_RT_TRACKER, rt_tracker_stop);
	register_perf_actuator(CMD_START_RT_TRACKER, rt_tracker_start);
}
