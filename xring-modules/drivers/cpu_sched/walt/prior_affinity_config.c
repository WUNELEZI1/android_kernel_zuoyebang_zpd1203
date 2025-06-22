// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#define MODULE_NAME "prior_affinity_config"
#define pr_fmt(fmt) MODULE_NAME ": " fmt

#include <linux/sched/clock.h>
#include <trace/hooks/sched.h>
#include <soc/xring/sched.h>
#include <soc/xring/perf_actuator.h>
#include "walt.h"

#define CREATE_TRACE_POINTS
#include "prior_affinity_config_trace.h"

#define CMD_SET_PRIOR_AFFINITY	PERF_W_CMD(SET_PRIOR_AFFINITY, struct prior_affinity_info)

struct prior_affinity_info {
	pid_t pid;
	unsigned int prior_mask;
	bool preempt;
};

/*
 * set_prior_affinity() - set or cancel the prior affinity of a task
 */
int set_prior_affinity(void __user *uarg)
{
	struct prior_affinity_info input;
	struct task_struct *p = NULL;
	struct walt_task_struct *wts = NULL;
	unsigned long flags;
	int ret = 0;

	if (copy_from_user(&input, uarg, sizeof(struct prior_affinity_info))) {
		pr_err("Fail in copy_from_user.\n");
		return -EFAULT;
	}

	rcu_read_lock();
	p = find_task_by_vpid(input.pid);
	if (p == NULL) {
		rcu_read_unlock();
		pr_err("Fail in find_task_by_vpid.\n");
		return -ESRCH;
	}

	/* Prevent p going away */
	get_task_struct(p);
	rcu_read_unlock();

	wts = (struct walt_task_struct *)(p->android_vendor_data1);
	raw_spin_lock_irqsave(&p->pi_lock, flags);

	if (!p->user_cpus_ptr) {
		p->user_cpus_ptr = kzalloc(sizeof(struct cpumask), GFP_ATOMIC);
		if (!p->user_cpus_ptr) {
			ret = -ENOMEM;
			raw_spin_unlock_irqrestore(&p->pi_lock, flags);
			goto out_put_task;
		}
		cpumask_copy(p->user_cpus_ptr, &p->cpus_mask);
	}

	wts->prior_mask.bits[0] = input.prior_mask;
	if (cpumask_empty(&wts->prior_mask)) {
		if (!cpumask_intersects(p->user_cpus_ptr, cpu_active_mask))
			cpumask_copy(p->user_cpus_ptr, cpu_possible_mask);
	} else {
		if (!cpumask_subset(&wts->prior_mask, cpu_possible_mask)) {
			pr_err("Fail: No such cpus.\n");
			ret = -EINVAL;
			raw_spin_unlock_irqrestore(&p->pi_lock, flags);
			goto out_put_task;
		}

		if (!cpumask_intersects(&wts->prior_mask, cpu_active_mask)) {
			pr_err("Fail: No active cpus.\n");
			ret = -EINVAL;
			raw_spin_unlock_irqrestore(&p->pi_lock, flags);
			goto out_put_task;
		}
	}
	raw_spin_unlock_irqrestore(&p->pi_lock, flags);

	wts->preempt = input.preempt;
	wts->preempt_ts = sched_clock();

	cpus_read_lock();
	ret = set_cpus_allowed_ptr(p, cpumask_empty(&wts->prior_mask) ?
				   p->user_cpus_ptr : &wts->prior_mask);
	cpus_read_unlock();

	trace_prior_affinity_config_show(p->pid, &wts->prior_mask, p->user_cpus_ptr, wts->preempt);
out_put_task:
	put_task_struct(p);

	if (ret != 0)
		pr_err("Fail to set prior affinity: ret=%d\n", ret);
	return ret;
}

#define PREEMPT_LIMIT_NS (1000 * 1000 * 1000)
bool prior_check_preempt(struct walt_task_struct *wts)
{
	if (wts->preempt && sched_clock() - wts->preempt_ts > PREEMPT_LIMIT_NS)
		wts->preempt = false;

	return wts->preempt;
}

static void spread_affinity(struct cpumask *mask)
{
	struct cpumask restrict_cpus;
	struct cpumask assist_cpus;

	restrict_cpus.bits[0] = sysctl_sched_spread_affinity_restrict_cpumask;
	assist_cpus.bits[0] = sysctl_sched_spread_affinity_assist_cpumask;

	if (cpumask_subset(mask, &restrict_cpus))
		cpumask_or(mask, mask, &assist_cpus);
}

/**
 * android_rvh_sched_setaffinity: trace when user set affinity
 *
 */
static void android_rvh_sched_setaffinity(
	void *unused, struct task_struct *p, const struct cpumask *in_mask, int *retval)
{
	struct walt_task_struct *wts =
		(struct walt_task_struct *)(p->android_vendor_data1);

	trace_setaffinity_show(current, p, in_mask, *retval, &wts->prior_mask);
}

#define ALLOWED_PTR_TAG			(BIT(4))
#define ALLOWED_COMM_TAG		(BIT(5))
/**
 * android_rvh_set_cpus_allowed_ptr: skip masking by p->user_cpus_ptr
 *
 */
static void android_rvh_set_cpus_allowed_ptr(
	void *unused, struct task_struct *p, struct affinity_context *ctx, bool *skip_user_ptr)
{
	struct walt_task_struct *wts =
		(struct walt_task_struct *)(p->android_vendor_data1);

	if (!cpumask_empty(&wts->prior_mask))
		*skip_user_ptr = true;

	trace_setaffinity_show(current, p, ctx ? ctx->new_mask : NULL,
			ALLOWED_PTR_TAG, &wts->prior_mask);
}

/**
 * android_rvh_set_cpus_allowed_by_task: set &p->cpus_mask
 *
 * NOTES: new_mask may be user_mask or prior_mask.
 */
static void android_rvh_set_cpus_allowed_comm(
	void *unused, struct task_struct *p, const struct cpumask *new_mask)
{
	struct walt_task_struct *wts =
		(struct walt_task_struct *)(p->android_vendor_data1);

	trace_setaffinity_show(current, p, new_mask, ALLOWED_COMM_TAG,
			&wts->prior_mask);

	if (p->flags & (PF_NO_SETAFFINITY | PF_KTHREAD))
		return;

	if (cpumask_empty(&wts->prior_mask))
		goto do_spread;

	if (!cpumask_intersects(&wts->prior_mask, cpu_active_mask)) {
		cpumask_clear(&wts->prior_mask);
		goto do_spread;
	}

	cpumask_copy(&p->cpus_mask, &wts->prior_mask);
	p->nr_cpus_allowed = cpumask_weight(&p->cpus_mask);
	return;

do_spread:
	if (!sysctl_sched_spread_affinity_enabled || !p->user_cpus_ptr)
		return;

	spread_affinity(p->user_cpus_ptr);
	spread_affinity(&p->cpus_mask);
	p->nr_cpus_allowed = cpumask_weight(&p->cpus_mask);
}

void prior_affinity_config_init(void)
{
	register_trace_android_rvh_sched_setaffinity(android_rvh_sched_setaffinity, NULL);
	register_trace_android_rvh_set_cpus_allowed_ptr(android_rvh_set_cpus_allowed_ptr, NULL);
	register_trace_android_rvh_set_cpus_allowed_comm(android_rvh_set_cpus_allowed_comm, NULL);
	register_perf_actuator(CMD_SET_PRIOR_AFFINITY, set_prior_affinity);
}
