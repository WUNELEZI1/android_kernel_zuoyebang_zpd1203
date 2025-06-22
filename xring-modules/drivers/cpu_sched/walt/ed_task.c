// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "Walt ed-task: " fmt

#include <linux/kernel.h>
#include <linux/sched.h>

#include "walt.h"
#include "trace.h"

static inline bool is_ed_task_wait(struct task_struct *p,
				   struct walt_rq *wrq,
				   struct walt_related_thread_group *grp)
{
	struct walt_task_struct *wts = (struct walt_task_struct *)p->android_vendor_data1;
	u64 thres = grp ? grp->xr_ed_wait_thres : wrq->ed_wait_thres;

	if (thres && wts->xr_runnable_sum >= thres) {
		trace_xr_ed_wait_task(p, grp ? grp->id : 0, wts->xr_runnable_sum, thres,
					wts->last_wake_ts, wts->last_enqueued_ts);
		return true;
	}

	return false;
}

static inline bool is_ed_task_run(struct task_struct *p,
				  struct walt_rq *wrq,
				  struct walt_related_thread_group *grp,
				  u64 wallclock)
{
	struct walt_task_struct *wts = (struct walt_task_struct *)p->android_vendor_data1;
	u64 delta = wallclock - max(wts->last_wake_ts, wts->last_enqueued_ts);
	u64 thres = 0;

	if (is_new_task(p))
		thres = grp ? grp->xr_ed_first_run_thres : wrq->ed_first_run_thres;
	else
		thres = grp ? grp->xr_ed_run_thres : wrq->ed_run_thres;

	if (thres && delta >= thres) {
		trace_xr_ed_run_task(p, grp ? grp->id : 0, wallclock, delta, thres,
			wts->last_wake_ts, wts->last_enqueued_ts);
		return true;
	}

	return false;
}

/* make sure the rq lock is held */
bool is_ed_task(struct task_struct *p,
		struct walt_rq *wrq,
		u64 wallclock)
{
	struct walt_related_thread_group *grp = NULL;
	bool ret = false;

	if (p->prio < MAX_RT_PRIO)
		return false;

	rcu_read_lock();
	grp = task_related_thread_group(p);
	if (is_ed_task_wait(p, wrq, grp) || is_ed_task_run(p, wrq, grp, wallclock))
		ret = true;

	rcu_read_unlock();

	return ret;
}

bool is_ed_task_present(struct rq *rq, u64 wallclock, struct task_struct *deq_task)
{
	struct task_struct *p = NULL;
	int loop_max = 10;
	struct walt_rq *wrq = &per_cpu(walt_rq, cpu_of(rq));

	wrq->ed_task = NULL;

	if (!is_ed_enabled() || !rq->cfs.h_nr_running)
		return false;

	list_for_each_entry(p, &rq->cfs_tasks, se.group_node) {
		if (!loop_max)
			break;

		if (p == deq_task)
			continue;
		if (is_ed_task(p, wrq, wallclock)) {
			wrq->ed_task = p;
			return true;
		}

		--loop_max;
	}

	return false;
}

void sched_stat_wait(void *unused, struct task_struct *p, u64 delta)
{
	struct walt_task_struct *wts = (struct walt_task_struct *)p->android_vendor_data1;

	if (unlikely(walt_disabled))
		return;

	/*
	 * Avoid some sched_stat bugs introduced by
	 * sched/fair: Improve the accuracy of sched_stat_wait statistics
	 * commit b9c88f752268383beff0d56e50d52b8ae62a02f8
	 */
	if (delta <= wts->last_wake_ts)
		wts->xr_runnable_sum += delta;
}
