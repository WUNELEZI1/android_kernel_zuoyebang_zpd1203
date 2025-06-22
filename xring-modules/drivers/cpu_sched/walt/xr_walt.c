// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/list.h>
#include <linux/sched.h>
#include <soc/xring/perf_actuator.h>
#include <soc/xring/walt.h>
#include "walt.h"

struct sched_stat {
	pid_t pid;
	unsigned long long sum_exec_runtime;
	unsigned long long sum_yield_time;
	unsigned long long run_delay;
	unsigned long pcount;
};

struct task_rtg {
	pid_t pid;
	unsigned int rtg_id;
};

struct rtg_cpus {
	unsigned int rtg_id;
	unsigned int cpumask;
};

struct rtg_util {
	unsigned int rtg_id;
	unsigned int util;
};

struct xr_ed_task {
	unsigned int id;
	unsigned int wait_thres;
	unsigned int run_thres;
	unsigned int first_run_thres;
};

struct rtg_freq {
	unsigned int rtg_id;
	unsigned int lock;
};

#define CPU_MAX_NUM	10
struct per_cpu_util {
	unsigned long util[CPU_MAX_NUM];
	unsigned long util_max[CPU_MAX_NUM];
};

#define CMD_GET_SCHED_STAT	PERF_R_CMD(GET_SCHED_STAT, struct sched_stat)
#define CMD_SET_TASK_RTG	PERF_W_CMD(SET_TASK_RTG, struct task_rtg)
#define CMD_SET_RTG_CPUS	PERF_W_CMD(SET_RTG_CPUS, struct rtg_cpus)
#define CMD_SET_RTG_UTIL	PERF_W_CMD(SET_RTG_UTIL, struct rtg_util)
#define CMD_SET_RTG_LOCKFREQ	PERF_W_CMD(SET_RTG_LOCKFREQ, struct rtg_freq)
#define CMD_SET_RTG_ROLLOVER	PERF_W_CMD(SET_RTG_ROLLOVER, unsigned int)
#define CMD_CLR_RTG		PERF_W_CMD(CLR_RTG, unsigned int)

#define CMD_SET_RTG_ED_TASK	PERF_W_CMD(SET_RTG_ED_TASK, struct xr_ed_task)

#define CMD_GET_CPU_UTIL	PERF_R_CMD(GET_CPU_UTIL, struct per_cpu_util)

unsigned long xr_get_group_util(struct walt_related_thread_group *grp,
				const struct cpumask *query_cpus)
{
	unsigned int cpu;
	u64 util_sum = 0;

	raw_spin_lock(&grp->lock);
	if (list_empty(&grp->tasks) || grp->xr_prev_window_time == 0)
		goto unlock;

	for_each_cpu_and(cpu, query_cpus, &grp->xr_prefer_cpus)
		util_sum += grp->xr_prev_window_cpu[cpu];

	util_sum = do_div(util_sum, grp->xr_prev_window_time);

	if (util_sum > 0)
		util_sum = max(grp->xr_util, util_sum);
unlock:
	raw_spin_unlock(&grp->lock);

	return util_sum;
}

bool xr_is_group_freq_locked(struct walt_related_thread_group *grp,
			     const struct cpumask *query_cpus)
{
	if (list_empty(&grp->tasks) || !grp->xr_lock_freq)
		return false;

	if (cpumask_intersects(query_cpus, &grp->xr_prefer_cpus))
		return true;

	return false;
}

static int xr_set_group_prefer_util(unsigned int grp_id, u32 util)
{
	struct walt_related_thread_group *grp = NULL;

	grp = get_related_thread_group(grp_id);
	if (grp == NULL) {
		pr_err("set prefer util for group %d fail\n", grp_id);
		return -ENODEV;
	}

	grp->xr_util = clamp(util, 0U, (u32)SCHED_CAPACITY_SCALE);

	return 0;
}

static int xr_set_group_prefer_cpus(unsigned int grp_id, unsigned int mask)
{
	struct walt_related_thread_group *grp = NULL;

	grp = get_related_thread_group(grp_id);
	if (grp == NULL) {
		pr_err("set preferred cpus for group %d fail\n", grp_id);
		return -ENODEV;
	}

	grp->xr_prefer_cpus.bits[0] = mask;

	core_ctl_update();

	return 0;
}

static int xr_set_group_ed_task_para(struct xr_ed_task *ed_task_config)
{
	struct walt_related_thread_group *grp = NULL;

	grp = get_related_thread_group(ed_task_config->id);
	if (grp == NULL) {
		pr_err("Set ed task parameter for group %u fail\n",
			ed_task_config->id);
		return -ENODEV;
	}

	grp->xr_ed_run_thres = ed_task_config->run_thres;
	grp->xr_ed_wait_thres = ed_task_config->wait_thres;
	grp->xr_ed_first_run_thres = ed_task_config->first_run_thres;

	return 0;
}

void xr_get_group_prefer_cpus(struct task_struct *p, struct cpumask *perfer_cpus)
{
	struct walt_related_thread_group *grp = NULL;
	struct walt_task_struct *wts =
		(struct walt_task_struct *) p->android_vendor_data1;

	grp = wts->grp;
	if (grp != NULL && !cpumask_empty(&grp->xr_prefer_cpus))
		cpumask_copy(perfer_cpus, &grp->xr_prefer_cpus);
	else
		cpumask_setall(perfer_cpus);
}

unsigned long xr_get_group_prefer_util(struct task_struct *p)
{
	struct walt_related_thread_group *grp = NULL;
	struct walt_task_struct *wts =
		(struct walt_task_struct *) p->android_vendor_data1;

	grp = wts->grp;
	return grp ? grp->xr_util : 0;
}

bool xr_is_task_prefer_cpu(struct task_struct *p, unsigned int cpu)
{
	struct walt_related_thread_group *grp = NULL;
	struct walt_task_struct *wts =
		(struct walt_task_struct *) p->android_vendor_data1;

	grp = wts->grp;
	if (grp != NULL && !cpumask_empty(&grp->xr_prefer_cpus))
		return (cpumask_test_cpu(cpu, &grp->xr_prefer_cpus) != 0);

	return true;
}

bool xr_can_migrate_task(struct task_struct *p, unsigned int src_cpu, unsigned int dst_cpu)
{
	struct walt_related_thread_group *grp = NULL;
	struct walt_task_struct *wts =
		(struct walt_task_struct *) p->android_vendor_data1;

	grp = wts->grp;
	if (grp != NULL && !cpumask_empty(&grp->xr_prefer_cpus))
		return !cpumask_test_cpu(src_cpu, &grp->xr_prefer_cpus) ||
			cpumask_test_cpu(dst_cpu, &grp->xr_prefer_cpus);

	return true;
}

static int xr_set_group_window_rollover(unsigned int grp_id)
{
	struct walt_related_thread_group *grp = NULL;
	struct walt_task_struct *wts = NULL;
	u64 wallclock;
	unsigned int cpu;
	unsigned long flag;

	grp = get_related_thread_group(grp_id);
	if (grp == NULL) {
		pr_err("set window start for group %d fail\n", grp_id);
		return -ENODEV;
	}

	raw_spin_lock_irqsave(&grp->lock, flag);

	wallclock = walt_sched_clock();
	grp->xr_prev_window_time =
		(wallclock - grp->xr_window_start) >> SCHED_CAPACITY_SHIFT;
	grp->xr_window_start = wallclock;

	list_for_each_entry(wts, &grp->tasks, grp_list) {
		if (wts->xr_last_exec_sum)
			wts->xr_last_window_exec =
				wts->xr_running_exec_sum - wts->xr_last_exec_sum;
		wts->xr_last_exec_sum = wts->xr_running_exec_sum;
		if (wts->xr_last_load_sum)
			wts->xr_last_window_load =
				wts->xr_running_load_sum - wts->xr_last_load_sum;
		wts->xr_last_load_sum = wts->xr_running_load_sum;
	}

	grp->xr_prev_running_load = grp->xr_curr_running_load;
	grp->xr_curr_running_load = 0;
	grp->xr_prev_running_exec = grp->xr_curr_running_exec;
	grp->xr_curr_running_exec = 0;
	for (cpu = 0; cpu < WALT_NR_CPUS; cpu++) {
		grp->xr_prev_window_cpu[cpu] = grp->xr_curr_window_cpu[cpu];
		grp->xr_curr_window_cpu[cpu] = 0;
	}

	raw_spin_unlock_irqrestore(&grp->lock, flag);

	return 0;
}

static int xr_set_group_lockfreq(unsigned int grp_id, bool enable)
{
	struct walt_related_thread_group *grp = NULL;
	unsigned int cpu;
	unsigned long flag;

	grp = get_related_thread_group(grp_id);
	if (grp == NULL)
		return -ENODEV;

	grp->xr_lock_freq = enable;

	if (list_empty(&grp->tasks))
		return 0;

	for_each_cpu(cpu, &grp->xr_prefer_cpus) {
		raw_spin_lock_irqsave(&cpu_rq(cpu)->__lock, flag);
		xresgov_run_callback(cpu, enable ? WALT_RTG_BOOST : WALT_CPUFREQ_CONTINUE);
		raw_spin_unlock_irqrestore(&cpu_rq(cpu)->__lock, flag);
	}


	return 0;
}

void xr_get_task_running_sum(struct task_struct *p, u64 *load_sum, u64 *exec_sum)
{
	struct walt_task_struct *wts =
		(struct walt_task_struct *) p->android_vendor_data1;

	if (load_sum != NULL)
		*load_sum = wts->xr_running_load_sum;

	if (exec_sum != NULL)
		*exec_sum = wts->xr_running_exec_sum;
}
EXPORT_SYMBOL(xr_get_task_running_sum);

void xr_update_task_running_sum(struct task_struct *p, struct rq *rq,
				int event, u64 delta_exec, u64 delta_load)
{
	struct walt_related_thread_group *grp;
	struct walt_task_struct *wts = (struct walt_task_struct *) p->android_vendor_data1;
	struct walt_rq *wrq = &per_cpu(walt_rq, cpu_of(rq));

	/* wait time not take into account */
	if (event == PICK_NEXT_TASK || event == TASK_MIGRATE ||
	    delta_load == 0)
		return;

	if (event == TASK_UPDATE && rq->curr != p)
		return;

	wts->xr_running_load_sum += delta_load;
	wts->xr_running_exec_sum += delta_exec;
	wrq->xr_running_sum += delta_load;
	wrq->xr_exec_sum += delta_exec;

	/* update group running sum */
	grp = wts->grp;
	if (grp == NULL)
		return;

	raw_spin_lock(&grp->lock);
	grp->xr_curr_running_load += delta_load;
	grp->xr_curr_running_exec += delta_exec;
	grp->xr_curr_window_cpu[rq->cpu] += delta_load;
	raw_spin_unlock(&grp->lock);
}

void xr_reset_task_sum(struct walt_related_thread_group *grp,
		       struct walt_task_struct *wts)
{
	/* reset task's last window load/exec and last load/exec sum */
	grp->xr_prev_running_load -= wts->xr_last_window_load;
	wts->xr_last_window_load = 0;
	if (wts->xr_last_load_sum) {
		grp->xr_curr_running_load -=
			wts->xr_running_load_sum - wts->xr_last_load_sum;
		wts->xr_last_load_sum = 0;
	}
	grp->xr_prev_running_exec -= wts->xr_last_window_exec;
	wts->xr_last_window_exec = 0;
	if (wts->xr_last_exec_sum) {
		grp->xr_curr_running_exec -=
			wts->xr_running_exec_sum - wts->xr_last_exec_sum;
		wts->xr_last_exec_sum = 0;
	}
}

void xr_init_account_sum(struct walt_task_struct *wts)
{
	wts->xr_running_exec_sum = 0;
	wts->xr_running_load_sum = 0;
	wts->xr_last_exec_sum = 0;
	wts->xr_last_load_sum = 0;
	wts->xr_last_window_exec = 0;
	wts->xr_last_window_load = 0;
	wts->xr_runnable_sum = 0;
}

void xr_account_yield_time(struct walt_task_struct *wts, u64 now)
{
	u64 delta;

	/* detemine whether yield cause a context switch */
	if (wts->xr_last_yield_ts) {
		delta = now - wts->xr_last_yield_ts;
		/* long yield duration time means it's not a simple yield loop */
		if (delta < sysctl_xr_yield_invalid_time_ns)
			wts->xr_yield_time_sum += delta;
	}

	wts->xr_last_yield_ts = now;
}

static u64 xr_get_yield_time(struct task_struct *p)
{
	struct walt_task_struct *wts =
		(struct walt_task_struct *) p->android_vendor_data1;

	return wts->xr_yield_time_sum;
}

static int get_schedstat(struct sched_stat *sched_stat_val)
{
	struct task_struct *task = NULL;

	rcu_read_lock();
	task = find_task_by_vpid(sched_stat_val->pid);
	if (task == NULL) {
		rcu_read_unlock();
		return -ENOENT;
	}
	get_task_struct(task);
	rcu_read_unlock();

	sched_stat_val->sum_exec_runtime = task_sched_runtime(task);
	sched_stat_val->run_delay = task->sched_info.run_delay;
	sched_stat_val->pcount = task->sched_info.pcount;
	sched_stat_val->sum_yield_time = xr_get_yield_time(task);
	put_task_struct(task);

	return 0;
}

static int perf_actuator_clear_rtg(void __user *uarg)
{
	struct walt_related_thread_group *grp = NULL;
	struct walt_task_struct *wts = NULL;
	struct task_struct *p = NULL;
	unsigned int grp_id;
	unsigned long flags;
	int max_loop = 200;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&grp_id, uarg, sizeof(grp_id)))
		return -EFAULT;

	grp = get_related_thread_group(grp_id);
	if (grp == NULL)
		return -ENOENT;

	local_irq_save(flags);
	preempt_disable();
	/*
	 * the entire loop can't hold grp->lock because sched_set_group_id
	 * also take lock, so there is no guarantee that the group can be
	 * totally cleared when adding to group is happened concurrently.
	 */
	do {
		raw_spin_lock(&grp->lock);
		wts = list_first_entry_or_null(&grp->tasks,
					       struct walt_task_struct,
					       grp_list);
		if (wts == NULL) {
			raw_spin_unlock(&grp->lock);
			break;
		}

		p = wts_to_ts(wts);
		get_task_struct(p);
		raw_spin_unlock(&grp->lock);

		sched_set_group_id(p, 0);
		put_task_struct(p);
	} while (--max_loop > 0);

	local_irq_restore(flags);
	preempt_enable();

	return max_loop ? 0 : -EAGAIN;
}

static inline u64 update_frame_status(u64 sum, u64 *last_sum, u64 total)
{
	u64 delta = *last_sum;

	*last_sum = sum;

	if (sum < delta)
		delta = 0;

	if (delta != 0)
		delta = sum - delta;

	return total ? delta / total : 0;
}

static DEFINE_SPINLOCK(frame_update_lock);

void walt_update_all_cpu(u64 timestamp)
{
	unsigned long flags;
	int cpu;

	local_irq_save(flags);

	for_each_possible_cpu(cpu) {
		struct walt_rq *wrq = &per_cpu(walt_rq, cpu);
		struct rq *rq = cpu_rq(cpu);
		u64 wallclock = timestamp;

		raw_spin_lock(&rq->__lock);

		if (wallclock < wrq->latest_clock)
			wallclock = wrq->latest_clock;

		walt_update_task_ravg(rq->curr, rq, TASK_UPDATE, wallclock, 0);

		raw_spin_unlock(&rq->__lock);
	}

	local_irq_restore(flags);
}

static int perf_actuator_get_cpu_util(void __user *uarg)
{
	static u64 frame_ts;
	struct per_cpu_util cpu_util = {0};
	unsigned long flags;
	u64 frame_time;
	int cpu, ret = 0;

	if (uarg == NULL)
		return -EINVAL;

	spin_lock_irqsave(&frame_update_lock, flags);

	frame_time = frame_ts;
	frame_ts = walt_sched_clock();
	if (frame_time != 0)
		frame_time = (frame_ts - frame_time) >> SCHED_CAPACITY_SHIFT;

	walt_update_all_cpu(frame_ts);

	for_each_possible_cpu(cpu) {
		struct walt_rq *wrq = &per_cpu(walt_rq, cpu);

		wrq->frame_load = update_frame_status(wrq->xr_exec_sum,
						      &wrq->xr_last_exec_sum,
						      frame_time);

		wrq->frame_util = update_frame_status(wrq->xr_running_sum,
						      &wrq->xr_last_running_sum,
						      frame_time);

		if (cpu < CPU_MAX_NUM) {
			cpu_util.util[cpu] = wrq->frame_util;
			cpu_util.util_max[cpu] = arch_scale_cpu_capacity(cpu);
		}
	}

	spin_unlock_irqrestore(&frame_update_lock, flags);

	if (copy_to_user(uarg, &cpu_util, sizeof(struct per_cpu_util)))
		ret = -EFAULT;

	return ret;
}

static int perf_actuator_get_sched_stat(void __user *uarg)
{
	struct sched_stat sched_stat_val;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&sched_stat_val, uarg, sizeof(struct sched_stat)))
		return -EFAULT;

	ret = get_schedstat(&sched_stat_val);
	if (ret != 0)
		return ret;

	if (copy_to_user(uarg, &sched_stat_val, sizeof(struct sched_stat)))
		ret = -EFAULT;

	return ret;
}


static int perf_actuator_set_task_rtg(void __user *uarg)
{
	struct task_rtg task_rtg_val;
	struct task_struct *task = NULL;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&task_rtg_val, uarg, sizeof(struct task_rtg)))
		return -EFAULT;

	rcu_read_lock();
	task = find_task_by_vpid(task_rtg_val.pid);
	if (task == NULL) {
		rcu_read_unlock();
		return -ENOENT;
	}
	ret = sched_set_group_id(task, task_rtg_val.rtg_id);
	rcu_read_unlock();

	if (ret != 0)
		pr_err("set task %d to rtg %d failed",
		       task_rtg_val.pid, task_rtg_val.rtg_id);
	return ret;
}

static int perf_actuator_set_rtg_cpus(void __user *uarg)
{
	struct rtg_cpus rtg_cpus_val;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&rtg_cpus_val, uarg, sizeof(struct rtg_cpus)))
		return -EFAULT;

	return xr_set_group_prefer_cpus(rtg_cpus_val.rtg_id,
					rtg_cpus_val.cpumask);
}

static int perf_actuator_set_rtg_util(void __user *uarg)
{
	struct rtg_util rtg_util_val;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&rtg_util_val, uarg, sizeof(struct rtg_util)))
		return -EFAULT;

	return xr_set_group_prefer_util(rtg_util_val.rtg_id,
					rtg_util_val.util);
}

static int perf_actuator_set_rtg_rollover(void __user *uarg)
{
	unsigned int rtg_id;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&rtg_id, uarg, sizeof(rtg_id)))
		return -EFAULT;

	return xr_set_group_window_rollover(rtg_id);
}

static int perf_actuator_set_rtg_ed_task(void __user *uarg)
{
	struct xr_ed_task rtg_ed;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&rtg_ed, uarg, sizeof(rtg_ed)))
		return -EFAULT;

	return xr_set_group_ed_task_para(&rtg_ed);
}

static int perf_actuator_set_rtg_lockfreq(void __user *uarg)
{
	struct rtg_freq data;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&data, uarg, sizeof(data)))
		return -EFAULT;

	return xr_set_group_lockfreq(data.rtg_id, data.lock > 0);
}

void xr_perf_actuator_init(void)
{
	register_perf_actuator(CMD_GET_SCHED_STAT, perf_actuator_get_sched_stat);
	register_perf_actuator(CMD_SET_TASK_RTG, perf_actuator_set_task_rtg);
	register_perf_actuator(CMD_SET_RTG_CPUS, perf_actuator_set_rtg_cpus);
	register_perf_actuator(CMD_SET_RTG_UTIL, perf_actuator_set_rtg_util);
	register_perf_actuator(CMD_SET_RTG_ROLLOVER, perf_actuator_set_rtg_rollover);
	register_perf_actuator(CMD_SET_RTG_LOCKFREQ, perf_actuator_set_rtg_lockfreq);
	register_perf_actuator(CMD_CLR_RTG, perf_actuator_clear_rtg);
	register_perf_actuator(CMD_SET_RTG_ED_TASK, perf_actuator_set_rtg_ed_task);
	register_perf_actuator(CMD_GET_CPU_UTIL, perf_actuator_get_cpu_util);
}
MODULE_SOFTDEP("post: perf-actuator");
