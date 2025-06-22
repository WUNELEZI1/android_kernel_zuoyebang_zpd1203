// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <trace/hooks/sched.h>

#include "walt.h"
#include "trace.h"

#if IS_ENABLED(CONFIG_XRING_TASK_PP)
static atomic_t pingpong_flag = ATOMIC_INIT(0);

void set_pingpong_flag(int type)
{
	atomic_or(BIT(type), &pingpong_flag);
}
EXPORT_SYMBOL_GPL(set_pingpong_flag);

void clear_pingpong_flag(int type)
{
	atomic_andnot(BIT(type), &pingpong_flag);
}
EXPORT_SYMBOL_GPL(clear_pingpong_flag);

int get_pingpong_flag(void)
{
	return atomic_read(&pingpong_flag);
}
EXPORT_SYMBOL_GPL(get_pingpong_flag);
#endif

static void walt_kick_cpu(int cpu)
{
	unsigned int flags = NOHZ_KICK_MASK;

	if (cpu == -1)
		return;

	/*
	 * Access to rq::nohz_csd is serialized by NOHZ_KICK_MASK; he who sets
	 * the first flag owns it; cleared by nohz_csd_func().
	 */
	flags = atomic_fetch_or(flags, nohz_flags(cpu));
	if (flags & NOHZ_KICK_MASK)
		return;

	/*
	 * This way we generate an IPI on the target CPU which
	 * is idle. And the softirq performing nohz idle load balance
	 * will be run before returning from the IPI.
	 */
	smp_call_function_single_async(cpu, &cpu_rq(cpu)->nohz_csd);
}

static inline unsigned long walt_lb_cpu_util(int cpu)
{
	struct walt_rq *wrq = &per_cpu(walt_rq, cpu);

	return wrq->walt_stats.cumulative_runnable_avg_scaled;
}

static void walt_detach_task(struct task_struct *p, struct rq *src_rq,
			     struct rq *dst_rq)
{
	//TODO can we just replace with detach_task in fair.c??
	deactivate_task(src_rq, p, 0);
	set_task_cpu(p, dst_rq->cpu);
}

static void walt_attach_task(struct task_struct *p, struct rq *rq)
{
	activate_task(rq, p, 0);
	check_preempt_curr(rq, p, 0);
}

static int stop_walt_lb_active_migration(void *data)
{
	struct rq *busiest_rq = data;
	int busiest_cpu = cpu_of(busiest_rq);
	int target_cpu;
	struct rq *target_rq;
	struct walt_rq *wrq = &per_cpu(walt_rq, busiest_cpu);
	struct task_struct *push_task;
	int push_task_detached = 0;

	raw_spin_lock_irq(&busiest_rq->__lock);
	target_cpu = busiest_rq->push_cpu;
	target_rq = cpu_rq(target_cpu);
	push_task = wrq->push_task;

	/* sanity checks before initiating the pull */
	if (!cpu_active(busiest_cpu) || !cpu_active(target_cpu) || !push_task)
		goto out_unlock;

	if (unlikely(busiest_cpu != raw_smp_processor_id() ||
		     !busiest_rq->active_balance))
		goto out_unlock;

	if (busiest_rq->nr_running <= 1)
		goto out_unlock;

	WARN_ON(busiest_rq == target_rq);

	if (task_on_rq_queued(push_task) &&
			READ_ONCE(push_task->__state) == TASK_RUNNING &&
			task_cpu(push_task) == busiest_cpu &&
			cpu_active(target_cpu) &&
			cpumask_test_cpu(target_cpu, push_task->cpus_ptr)) {
		walt_detach_task(push_task, busiest_rq, target_rq);
		push_task_detached = 1;
	}

out_unlock: /* called with busiest_rq lock */
	busiest_rq->active_balance = 0;
	target_cpu = busiest_rq->push_cpu;
	clear_reserved(target_cpu);
	wrq->push_task = NULL;
	raw_spin_unlock(&busiest_rq->__lock);

	if (push_task_detached) {
		raw_spin_lock(&target_rq->__lock);
		walt_attach_task(push_task, target_rq);
		raw_spin_unlock(&target_rq->__lock);
	}

	if (push_task)
		put_task_struct(push_task);

	local_irq_enable();

	return 0;
}

struct walt_rotate {
	struct work_struct	work;
	struct task_struct	*src_task;
	struct task_struct	*dst_task;
	int			src_cpu;
	int			dst_cpu;
};

DEFINE_PER_CPU(struct walt_rotate, rotate_work);

static void rotate_work_func(struct work_struct *work)
{
	struct walt_rotate *rotate = container_of(work,
					struct walt_rotate, work);
	struct rq *src_rq = cpu_rq(rotate->src_cpu);
	struct rq *dst_rq = cpu_rq(rotate->dst_cpu);
	unsigned long flags;

	migrate_swap(rotate->src_task, rotate->dst_task,
			rotate->dst_cpu, rotate->src_cpu);

	put_task_struct(rotate->src_task);
	put_task_struct(rotate->dst_task);

	local_irq_save(flags);
	double_rq_lock(src_rq, dst_rq);

	dst_rq->active_balance = 0;
	src_rq->active_balance = 0;

	double_rq_unlock(src_rq, dst_rq);
	local_irq_restore(flags);

	clear_reserved(rotate->src_cpu);
	clear_reserved(rotate->dst_cpu);
}

static void rotate_work_init(void)
{
	int cpu = 0;
	struct walt_rotate *rotate;

	for_each_possible_cpu(cpu) {
		rotate = &per_cpu(rotate_work, cpu);
		INIT_WORK(&rotate->work, rotate_work_func);
	}
}

/* Rotate threshold is 16ms */
#define ROTATE_THRES	16000000
static void rq_rotate_check(struct rq *this_rq)
{
	u64 wallclock, wait, max_wait = 0, run, max_run = 0;
	int src_cpu = nr_cpu_ids, dst_cpu = nr_cpu_ids;
	int i = 0, this_cpu = cpu_of(this_rq);
	struct rq *src_rq, *dst_rq;
	struct walt_rotate *rotate = NULL;
	struct walt_task_struct *wts;

	/* Just LITTLE cpus could be rotate src_cpu */
	if (!is_min_possible_cluster_cpu(this_cpu))
		return;

	/*
	 * Use src_rq->clock directly instead of rq_clock() since
	 * we do not have the rq lock and
	 * src_rq->clock was updated in the tick callpath.
	 */
	wallclock = this_rq->clock;

	/* Find rotate src_cpu */
	for_each_possible_cpu(i) {
		struct rq *rq = cpu_rq(i);

		if (!is_min_possible_cluster_cpu(i))
			break;

		/* Migration pending cpu shouldn't be candidate */
		if (is_reserved(i))
			continue;

		/* Just rotate cfs task */
		if (!rq->misfit_task_load || !walt_fair_task(rq->curr))
			continue;

		wts = (struct walt_task_struct *)rq->curr->android_vendor_data1;
		wait = wallclock - wts->last_enqueued_ts;
		if (wait > max_wait) {
			max_wait = wait;
			src_cpu = i;
		}
	}

	/* Can't rotate task from remote cpu, src_cpu should be local
	 * cpu.
	 */
	if (src_cpu != this_cpu)
		return;

	src_rq = this_rq;

	/* Find rotate dst_cpu */
	for_each_possible_cpu(i) {
		struct rq *rq = cpu_rq(i);

		if (is_min_possible_cluster_cpu(i))
			continue;

		if (is_reserved(i))
			continue;

		if (!walt_fair_task(rq->curr))
			continue;

		/* Don't rotate src task to an overload cpu */
		if (rq->nr_running > 1)
			continue;

		wts = (struct walt_task_struct *)rq->curr->android_vendor_data1;
		run = wallclock - wts->last_enqueued_ts;
		if (run < ROTATE_THRES)
			continue;

		if (run > max_run) {
			max_run = run;
			dst_cpu = i;
		}
	}

	if (dst_cpu == nr_cpu_ids)
		return;

	dst_rq = cpu_rq(dst_cpu);

	double_rq_lock(src_rq, dst_rq);

	if (!src_rq->active_balance && !dst_rq->active_balance &&
		cpumask_test_cpu(dst_cpu, src_rq->curr->cpus_ptr) &&
		cpumask_test_cpu(src_cpu, dst_rq->curr->cpus_ptr)) {
		get_task_struct(src_rq->curr);
		get_task_struct(dst_rq->curr);

		/* Set both CPUs as Migration pending */
		set_reserved(src_cpu);
		set_reserved(dst_cpu);

		rotate = &per_cpu(rotate_work, src_cpu);
		rotate->src_task = src_rq->curr;
		rotate->dst_task = dst_rq->curr;
		rotate->src_cpu = src_cpu;
		rotate->dst_cpu = dst_cpu;

		dst_rq->active_balance = 1;
		src_rq->active_balance = 1;

		trace_walt_lb_rotation(rotate->src_task, rotate->src_cpu,
				rotate->dst_task, rotate->dst_cpu);
	}

	double_rq_unlock(src_rq, dst_rq);

	if (rotate)
		queue_work_on(src_cpu, system_highpri_wq, &rotate->work);
}

static inline bool __walt_can_migrate_task(struct task_struct *p, int dst_cpu,
					  bool to_lower, bool to_higher, bool force)
{
	struct walt_rq *wrq = &per_cpu(walt_rq, task_cpu(p));
	struct walt_task_struct *wts = (struct walt_task_struct *)p->android_vendor_data1;

#if IS_ENABLED(CONFIG_XRING_SCHED_VIP)
	/* Don't migrate the vip task which is running on prefer-cpu */
	if (is_vip_task(p) && xr_is_task_prefer_cpu(p, task_cpu(p)))
		return false;
#endif

	/* Don't detach task if it is under active migration */
	if (wrq->push_task == p)
		return false;

	if (to_lower) {
		if (wts->iowaited)
			return false;

		if (per_task_boost(p) == TASK_BOOST_STRICT_MAX &&
				task_in_related_thread_group(p))
			return false;

		if (!force && walt_get_rtg_status(p))
			return false;

		if (!force && !task_fits_max(p, dst_cpu))
			return false;

	} else {
		if (!to_higher && !task_fits_max(p, dst_cpu) &&
			wrq->walt_stats.nr_big_tasks < 2)
			return false;
	}

	if (!xr_can_migrate_task(p, task_cpu(p), dst_cpu))
		return false;

	/* Don't detach task if dst cpu is halted */
	if (cpu_halted(dst_cpu))
		return false;

	if (task_reject_partialhalt_cpu(p, dst_cpu))
		return false;

	return true;
}

static inline bool need_active_lb(struct task_struct *p, int dst_cpu,
				  int src_cpu)
{
	struct walt_task_struct *wts = (struct walt_task_struct *) p->android_vendor_data1;

	if (!xr_can_migrate_task(p, src_cpu, dst_cpu))
		return false;

	if (cpu_rq(src_cpu)->active_balance)
		return false;

	if (!capacity_higher(dst_cpu, src_cpu))
		return false;

	if (!wts->misfit)
		return false;

	if (!is_min_possible_cluster_cpu(src_cpu) && !task_fits_max(p, dst_cpu))
		return false;

	if (task_reject_partialhalt_cpu(p, dst_cpu))
		return false;

	return true;
}

static int walt_lb_pull_tasks(int dst_cpu, int src_cpu, struct task_struct **pulled_p)
{
	struct rq *dst_rq = cpu_rq(dst_cpu);
	struct rq *src_rq = cpu_rq(src_cpu);
	unsigned long flags;
	struct task_struct *p = NULL;
	bool active_balance = false, to_lower, to_higher;
	struct walt_rq *src_wrq = &per_cpu(walt_rq, src_cpu);
	struct walt_task_struct *wts;
	struct task_struct *pull_me;
	int task_visited;

	WARN_ON(src_cpu == dst_cpu);

	to_lower = capacity_higher(src_cpu, dst_cpu);
	to_higher = capacity_higher(dst_cpu, src_cpu);

	raw_spin_lock_irqsave(&src_rq->__lock, flags);

	pull_me = NULL;
	task_visited = 0;
	list_for_each_entry_reverse(p, &src_rq->cfs_tasks, se.group_node) {
		if (!cpumask_test_cpu(dst_cpu, p->cpus_ptr))
			continue;

		/* Notice: Use p->on_cpu instead of task_on_cpu(src_rq, p) */
		if (p->on_cpu)
			continue;

		if (!__walt_can_migrate_task(p, dst_cpu, to_lower, to_higher,
					false))
			continue;

		if (pull_me == NULL) {
			pull_me = p;
		} else {
			if (to_lower) {
				if (task_util(p) < task_util(pull_me))
					pull_me = p;
			} else if (task_util(p) > task_util(pull_me)) {
				pull_me = p;
			}
		}

		task_visited++;
		if (task_visited > 5)
			break;
	}
	if (pull_me) {
		walt_detach_task(pull_me, src_rq, dst_rq);
		*pulled_p = pull_me;
		goto unlock;
	}

	pull_me = NULL;
	task_visited = 0;
	list_for_each_entry_reverse(p, &src_rq->cfs_tasks, se.group_node) {
		if (!cpumask_test_cpu(dst_cpu, p->cpus_ptr))
			continue;

		/* Notice: Use p->on_cpu instead of task_on_cpu(src_rq, p) */
		if (p->on_cpu)
			continue;

		if (!__walt_can_migrate_task(p, dst_cpu, to_lower, to_higher,
					true))
			continue;

		if (pull_me == NULL) {
			pull_me = p;
		} else {
			if (to_lower) {
				if (task_util(p) < task_util(pull_me))
					pull_me = p;
			} else if (task_util(p) > task_util(pull_me)) {
				pull_me = p;
			}
		}

		task_visited++;
		if (task_visited > 5)
			break;
	}
	if (pull_me) {
		walt_detach_task(pull_me, src_rq, dst_rq);
		*pulled_p = pull_me;
		goto unlock;
	}

	/* Active balance */
	list_for_each_entry_reverse(p, &src_rq->cfs_tasks, se.group_node) {
		/* Notice: Use p->on_cpu instead of task_on_cpu(src_rq, p) */
		if (p->on_cpu) {
			if (cpumask_test_cpu(dst_cpu, p->cpus_ptr)
				&& need_active_lb(p, dst_cpu, src_cpu)) {
				bool success;

				active_balance = true;
				src_rq->active_balance = 1;
				src_rq->push_cpu = dst_cpu;
				get_task_struct(p);
				src_wrq->push_task = p;
				set_reserved(dst_cpu);

				/* lock must be dropped before waking the stopper */
				raw_spin_unlock_irqrestore(&src_rq->__lock, flags);

				/*
				 * Using our custom active load balance callback so that
				 * the push_task is really pulled onto this CPU.
				 */
				wts = (struct walt_task_struct *)p->android_vendor_data1;
				trace_walt_active_load_balance(p, src_cpu, dst_cpu, wts, false);
				success = stop_one_cpu_nowait(src_cpu,
						stop_walt_lb_active_migration,
						src_rq, &src_rq->active_balance_work);
				if (!success)
					clear_reserved(dst_cpu);

				return 0; /* we did not pull any task here */
			}
			goto unlock;
		}
	}
unlock:
	/* Lock must be dropped before waking the stopper */
	raw_spin_unlock_irqrestore(&src_rq->__lock, flags);

	if (!*pulled_p)
		return 0;

	raw_spin_lock_irqsave(&dst_rq->__lock, flags);
	walt_attach_task(*pulled_p, dst_rq);
	raw_spin_unlock_irqrestore(&dst_rq->__lock, flags);

	return 1; /* we pulled 1 task */
}

#define SMALL_TASK_THRESHOLD	102
/*
 * find_first_idle_if_others_are_busy
 *
 * Find a middle-cluster CPU that has slept long enough. It first
 * looks at the high-speed cluster, and turns to the low-speed
 * cluster only if there's not a CPU idle or about to be idle.
 *
 * Returns -1 if there're no middle clusters, no idle cpus in
 * middle clusters, or if there is a CPU that is about to be come
 * newly idle.
 * Returns <cpu> if there is an idle cpu found, and none of the
 * other CPUs in the cluster would do newly idle load balancing.
 */
static int find_first_idle_if_others_are_busy(void)
{
	int i;
	int cpu;
	int first_idle = -1;
	s64 this_sleep_time;
	s64 longest_sleep_time = S64_MIN;

	/* Search from the cluster of higher efficiency */
	for (i = num_sched_clusters - 2; i > 0; i--) {
		for_each_cpu(cpu, &cpu_array[0][i]) {
			if (!cpu_active(cpu))
				continue;

			if (cpu_halted(cpu))
				continue;

			/*
			 * there was one CPU in the mask that was almost idle, but not
			 * quite. when it becomes idle, it will do newidle load-balance,
			 * and start off with it's own cluster. So no reason to kick
			 * anything in this cluster. i.e. don't perform a wasted kick.
			 */
			if (cpu_util(cpu) < SMALL_TASK_THRESHOLD && cpu_rq(cpu)->nr_running == 1)
				return -1;

			/* prefer the CPU that had a long sleep */
			if (available_idle_cpu(cpu)) {
				this_sleep_time = walt_get_sleep_time(cpu);
				if (this_sleep_time > longest_sleep_time) {
					longest_sleep_time = this_sleep_time;
					first_idle = cpu;
				}
			}
		}

		/* Idle CPU found! */
		if (first_idle != -1)
			break;
	}

	return first_idle;
}

static int find_busiest_from_sibling(int dst_cpu, const cpumask_t *src_mask,
		int *has_misfit, bool is_newidle)
{
	int i = 0;
	int busiest_cpu = -1;
	unsigned long util, busiest_util = 0;
	struct walt_rq *wrq;

	for_each_cpu(i, src_mask) {
		wrq = &per_cpu(walt_rq, i);
		trace_walt_lb_cpu_util(i, wrq);

		if (cpu_rq(i)->nr_running < 2 || !cpu_rq(i)->cfs.h_nr_running)
			continue;

		if (cpu_partial_halted(dst_cpu) && !cpu_partial_halted(i))
			continue;

		util = walt_lb_cpu_util(i);
		if (util < busiest_util)
			continue;

		busiest_util = util;
		busiest_cpu = i;
	}

	return busiest_cpu;
}

static int find_busiest_from_higher(int dst_cpu, const cpumask_t *src_mask,
		int *has_misfit, bool is_newidle)
{
	int i = 0;
	int busiest_cpu = -1;
	unsigned long util, busiest_util = 0;
	unsigned long total_capacity = 0, total_util = 0, total_nr = 0;
	int total_cpus = 0;
	struct walt_rq *wrq;
	bool asymcap_boost = ASYMCAP_BOOST(dst_cpu);

	if (cpu_partial_halted(dst_cpu))
		return -1;

	for_each_cpu(i, src_mask) {

		if (!cpu_active(i))
			continue;

		wrq = &per_cpu(walt_rq, i);
		trace_walt_lb_cpu_util(i, wrq);

		util = walt_lb_cpu_util(i);
		total_cpus += 1;
		total_util += util;
		total_capacity += capacity_orig_of(i);
		total_nr += cpu_rq(i)->cfs.h_nr_running;

		if (cpu_rq(i)->cfs.h_nr_running < 2)
			continue;

		if (cpu_rq(i)->cfs.h_nr_running == 2 &&
			task_util(cpu_rq(i)->curr) < SMALL_TASK_THRESHOLD)
			continue;

		/*
		 * During rotation, two silver fmax tasks gets
		 * placed on gold/prime and the CPU may not be
		 * overutilized but for rotation, we have to
		 * spread out.
		 */
		if (!walt_rotate_flag && !cpu_overutilized(i) &&
			!asymcap_boost)
			continue;

		if (util < busiest_util)
			continue;

		busiest_util = util;
		busiest_cpu = i;
	}

	/*
	 * Don't allow migrating to lower cluster unless this high
	 * capacity cluster is sufficiently loaded.
	 */
	if (!walt_rotate_flag && !asymcap_boost) {
		if (total_nr <= total_cpus ||
				total_util * 1280 < total_capacity * SCHED_CAPACITY_SCALE)
			busiest_cpu = -1;
	}

	return busiest_cpu;
}

static int find_busiest_from_lower(int dst_cpu, const cpumask_t *src_mask,
		int *has_misfit, bool is_newidle)
{
	int i = 0;
	int busiest_cpu = -1;
	unsigned long util, busiest_util = 0;
	unsigned long total_capacity = 0, total_util = 0, total_nr = 0;
	int total_cpus = 0;
	int busy_nr_big_tasks = 0;
	struct walt_rq *wrq;
	bool treat_dst_idle = is_newidle || available_idle_cpu(dst_cpu);

	/*
	 * A higher capacity CPU is looking at a lower capacity
	 * cluster. active balance and big tasks are in play.
	 * other than that, it is very much same as above. we
	 * really don't need this as a separate block. will
	 * refactor this after final testing is done.
	 */
	for_each_cpu(i, src_mask) {
		wrq = &per_cpu(walt_rq, i);

		if (!cpu_active(i))
			continue;

		trace_walt_lb_cpu_util(i, wrq);

		util = walt_lb_cpu_util(i);
		total_cpus += 1;
		total_util += util;
		total_capacity += capacity_orig_of(i);
		total_nr += cpu_rq(i)->cfs.h_nr_running;

		/*
		 * no point in selecting this CPU as busy, as
		 * active balance is in progress.
		 */
		if (cpu_rq(i)->active_balance)
			continue;

		/* active migration is allowed only to idle cpu */
		if (cpu_rq(i)->cfs.h_nr_running < 2 &&
			(!wrq->walt_stats.nr_big_tasks || !treat_dst_idle))
			continue;

		if (!walt_rotate_flag && !cpu_overutilized(i) &&
			!ASYMCAP_BOOST(i))
			continue;

		if (util < busiest_util)
			continue;

		busiest_util = util;
		busiest_cpu = i;
		busy_nr_big_tasks = wrq->walt_stats.nr_big_tasks;
	}

	if (!walt_rotate_flag && !busy_nr_big_tasks &&
		!(busiest_cpu != -1 && ASYMCAP_BOOST(busiest_cpu))) {
		if (total_nr <= total_cpus ||
				total_util * 1280 < total_capacity * SCHED_CAPACITY_SCALE)
			busiest_cpu = -1;
	}

	if (busy_nr_big_tasks && busiest_cpu != -1)
		*has_misfit = true;

	return busiest_cpu;
}

static int walt_find_busiest_cpu(int dst_cpu, const cpumask_t *src_mask, int *has_misfit,
				    bool is_newidle)
{
	int fbq_cpu = cpumask_first(src_mask);
	int busiest_cpu;

	if (capacity_higher(dst_cpu, fbq_cpu))
		busiest_cpu = find_busiest_from_lower(dst_cpu,
								src_mask, has_misfit, is_newidle);
	else if (capacity_higher(fbq_cpu, dst_cpu))
		busiest_cpu = find_busiest_from_higher(dst_cpu,
								src_mask, has_misfit, is_newidle);
	else
		busiest_cpu = find_busiest_from_sibling(dst_cpu,
								src_mask, has_misfit, is_newidle);

	return busiest_cpu;
}

#if IS_ENABLED(CONFIG_XRING_TASK_PP)
#define TASK_UTIL_PP_THRES		(256)
static bool pingpong_lb_check(int cpu, struct rq *rq, struct walt_rq *wrq,
		struct task_struct *p, int *sibling)
{
	if (!get_pingpong_flag())
		return false;

	/* Task pingpong only occurs in cpub */
	if (!is_max_possible_cluster_cpu(cpu))
		return false;

	/*
	 * A task for pingpong should running in cpub highest freq
	 * more than 1/4 window.
	 */
	if (task_util(p) < TASK_UTIL_PP_THRES)
		return false;

	/*
	 * Task pingpong only occurs when :
	 * 1. Only 1 task running in this cpu,
	 * 2. There is any idle cpu in cpub.
	 */
	*sibling = find_idle_cpu(&wrq->freq_domain_cpumask);
	if (rq->nr_running > 1 || *sibling < 0)
		return false;

	/* Pingpong target cpu should sleep more than *ms */
	if (walt_get_sleep_time(*sibling) < sysctl_task_pp_idle_thres)
		return false;

	return true;
}
#else /* CONFIG_XRING_TASK_PP */
static bool pingpong_lb_check(int cpu, struct rq *rq, struct walt_rq *wrq,
		struct task_struct *p, int *sibling)
{
	return false;
}
#endif /* CONFIG_XRING_TASK_PP */

static DEFINE_RAW_SPINLOCK(walt_lb_migration_lock);
void walt_lb_tick(struct rq *rq)
{
	int prev_cpu = rq->cpu, new_cpu, ret;
	struct task_struct *p = rq->curr;
	unsigned long flags;
	struct walt_rq *prev_wrq = &per_cpu(walt_rq, cpu_of(rq));
	struct walt_task_struct *wts = (struct walt_task_struct *)p->android_vendor_data1;
	bool pingpong_lb = false;
	int sibling = -1;

	raw_spin_lock(&rq->__lock);
	if (available_idle_cpu(prev_cpu) && is_reserved(prev_cpu) && !rq->active_balance)
		clear_reserved(prev_cpu);
	raw_spin_unlock(&rq->__lock);

	if (!walt_fair_task(p))
		return;

	pingpong_lb = pingpong_lb_check(prev_cpu, rq, prev_wrq, p, &sibling);
	if (!rq->misfit_task_load && !pingpong_lb)
		return;

	if (READ_ONCE(p->__state) != TASK_RUNNING || p->nr_cpus_allowed == 1)
		return;

	raw_spin_lock_irqsave(&walt_lb_migration_lock, flags);

	if (walt_rotate_flag) {
		rq_rotate_check(rq);
		goto out_unlock;
	}

	rcu_read_lock();
	if (pingpong_lb)
		new_cpu = sibling;
	else
		new_cpu = walt_find_energy_efficient_cpu(p, prev_cpu, 0, 1);
	rcu_read_unlock();

	if (new_cpu < 0)
		goto out_unlock;

	if (!available_idle_cpu(new_cpu) ||
		!xr_can_migrate_task(p, prev_cpu, new_cpu))
		goto out_unlock;

	/*
	 * Prevent active task migration to busy or same/lower capacity CPU,
	 * when Task Pingpong NOT occurs.
	 */
	if (!capacity_higher(new_cpu, prev_cpu) && !pingpong_lb)
		goto out_unlock;

	raw_spin_lock(&rq->__lock);
	if (rq->active_balance) {
		raw_spin_unlock(&rq->__lock);
		goto out_unlock;
	}
	rq->active_balance = 1;
	rq->push_cpu = new_cpu;
	get_task_struct(p);
	prev_wrq->push_task = p;
	raw_spin_unlock(&rq->__lock);

	set_reserved(new_cpu);
	raw_spin_unlock_irqrestore(&walt_lb_migration_lock, flags);

	trace_walt_active_load_balance(p, prev_cpu, new_cpu, wts, pingpong_lb);
	ret = stop_one_cpu_nowait(prev_cpu,
			stop_walt_lb_active_migration, rq,
			&rq->active_balance_work);
	if (!ret)
		clear_reserved(new_cpu);
	else
		wake_up_if_idle(new_cpu);

	return;

out_unlock:
	raw_spin_unlock_irqrestore(&walt_lb_migration_lock, flags);
}

static inline int has_pushable_tasks(struct rq *rq)
{
	return !plist_head_empty(&rq->rt.pushable_tasks);
}

#define WALT_RT_PULL_THRESHOLD_NS	1000000
static bool walt_balance_rt(struct rq *this_rq)
{
	int i = 0, this_cpu = this_rq->cpu, src_cpu = this_cpu;
	struct rq *src_rq;
	struct task_struct *p = NULL;
	struct walt_task_struct *wts;
	bool pulled = false;
	u64 wallclock;
	s64 delta = 0;
	int nr_rt = 0;

	/* can't help if this has a runnable RT */
	if (sched_rt_runnable(this_rq))
		return false;

	/* check if any CPU has a pushable RT task */
	for_each_possible_cpu(i) {
		struct rq *rq = cpu_rq(i);
		// MIUI ADD: Performance_BoostFramework
		if (!has_pushable_tasks(rq))
			continue;
		// END Performance_BoostFramework
		src_cpu = i;
		break;
	}

	if (src_cpu == this_cpu)
		return false;

	src_rq = cpu_rq(src_cpu);
	double_lock_balance(this_rq, src_rq);

	/* lock is dropped, so check again */
	if (sched_rt_runnable(this_rq))
		goto unlock;

	p = pick_highest_pushable_task(src_rq, this_cpu);

	if (!p)
		goto unlock;

	if (arch_scale_cpu_capacity(this_cpu) < xr_get_group_prefer_util(p) ||
		!xr_can_migrate_task(p, src_cpu, this_cpu))
		goto unlock;
	// MIUI ADD: Performance_BoostFramework
	if (!task_fits_capacity(p, this_cpu) && !capacity_higher(this_cpu, src_cpu))
		goto unlock;
	// END Performance_BoostFramework
	if (!cpumask_test_cpu(this_cpu, p->cpus_ptr))
		goto unlock;

	wts = (struct walt_task_struct *)p->android_vendor_data1;

	/*
	 * Use rq->clock directly instead of rq_clock() since
	 * rq->clock was updated recently in the __schedule() -> pick_next_task() callpath.
	 * Time lost in grabbing rq locks will likely be corrected via max.
	 */
	wallclock = max(this_rq->clock, src_rq->clock);

	delta = (s64)(wallclock - wts->last_wake_ts);
	if (delta > 0 && delta < WALT_RT_PULL_THRESHOLD_NS)
		goto unlock;

	if (trace_balance_rt_enabled())
		nr_rt = src_rq->rt.rt_nr_running;

	if (!is_migration_disabled(p)) {
		pulled = true;
		deactivate_task(src_rq, p, 0);
		set_task_cpu(p, this_cpu);
		activate_task(this_rq, p, 0);
	}
unlock:
	double_unlock_balance(this_rq, src_rq);
	trace_balance_rt(pulled, p, this_cpu, src_cpu, delta, nr_rt);
	return pulled;
}

int push_cpu_stop(void *arg)
{
	struct rq *lowest_rq = NULL, *rq = this_rq();
	struct task_struct *p = arg;

	raw_spin_lock_irq(&p->pi_lock);
	raw_spin_rq_lock(rq);

	if (task_rq(p) != rq)
		goto out_unlock;

	if (is_migration_disabled(p)) {
		p->migration_flags |= MDF_PUSH;
		goto out_unlock;
	}

	p->migration_flags &= ~MDF_PUSH;

	if (p->sched_class->find_lock_rq)
		lowest_rq = p->sched_class->find_lock_rq(p, rq);

	if (!lowest_rq)
		goto out_unlock;

	// XXX validate p is still the highest prio task
	if (task_rq(p) == rq) {
		deactivate_task(rq, p, 0);
		set_task_cpu(p, lowest_rq->cpu);
		activate_task(lowest_rq, p, 0);
		resched_curr(lowest_rq);
	}

	double_unlock_balance(rq, lowest_rq);

out_unlock:
	rq->active_balance = 0;
	rq->push_busy = false;
	raw_spin_rq_unlock(rq);
	raw_spin_unlock_irq(&p->pi_lock);

	put_task_struct(p);
	return 0;
}

static inline struct task_struct *walt_get_push_task(struct rq *rq,
		struct task_struct *p)
{
	lockdep_assert_rq_held(rq);

	if (rq->push_busy)
		return NULL;

	if (p->nr_cpus_allowed == 1)
		return NULL;

	if (p->migration_disabled)
		return NULL;

	rq->push_busy = true;
	return get_task_struct(p);
}

void check_for_rt_migration(struct rq *rq, struct task_struct *p)
{
	int cpu = cpu_of(rq), new_cpu = -1;
	int ret, change;
	bool misfit = false, old_misfit;
	struct task_struct *push_task;
	struct walt_task_struct *wts;
	struct walt_rq *wrq = &per_cpu(walt_rq, cpu);

	if (!walt_rt_task(p))
		return;

	raw_spin_rq_lock(rq);
	wts = (struct walt_task_struct *)p->android_vendor_data1;
	old_misfit = wts->misfit;
	misfit = !task_fits_max(p, cpu);

	/* Update RT tasks misfit status */
	change = misfit - old_misfit;
	if (change) {
		sched_update_nr_prod(cpu, 0);
		wts->misfit = misfit;
		wrq->walt_stats.nr_big_tasks += change;
		WARN_ON(wrq->walt_stats.nr_big_tasks < 0);
	}
	raw_spin_rq_unlock(rq);

	/* RT active load balance */
	if (misfit && !need_resched()) {
		new_cpu = walt_rt_find_best_cpu(p);
		if (new_cpu > 0 && capacity_higher(new_cpu, cpu) &&
				p->prio < cpu_rq(new_cpu)->rt.highest_prio.curr &&
				!rq->active_balance) {
			raw_spin_rq_lock(rq);
			push_task = walt_get_push_task(rq, p);
			raw_spin_rq_unlock(rq);
			if (!push_task)
				return;

			preempt_disable();
			rq->active_balance = 1;
			ret = stop_one_cpu_nowait(cpu, push_cpu_stop,
					push_task, &rq->push_work);
			if (ret)
				wake_up_if_idle(new_cpu);
			preempt_enable();
		}
	}

	trace_check_for_rt_migration(p, misfit, cpu, new_cpu);
}

/* Similar to sysctl_sched_migration_cost */
#define ILB_THRES	500000
static void walt_newidle_balance(struct rq *this_rq,
				 struct rq_flags *rf, int *pulled_task,
				 int *done, bool force_overload)
{
	int this_cpu = this_rq->cpu;
	struct walt_rq *wrq = &per_cpu(walt_rq, this_cpu);
	int order_index;
	int busy_cpu = -1;
	bool enough_idle = (this_rq->avg_idle > ILB_THRES);
	int first_idle;
	int has_misfit = 0;
	int i;
	struct task_struct *pulled_p = NULL;

	if (unlikely(walt_disabled))
		return;

	/*
	 * Newidle load balance is completely handled here.
	 */
	*done = 1;
	*pulled_task = 0;

	/*
	 * This CPU is about to enter idle, so clear the
	 * misfit_task_load and mark the idle stamp.
	 */
	this_rq->misfit_task_load = 0;
	this_rq->idle_stamp = rq_clock(this_rq);

	/* Don't pull tasks to inactive CPU or halted CPU */
	if (!cpu_active(this_cpu) || cpu_halted(this_cpu))
		return;

	/* Don't pull tasks to Migration pending CPU */
	if (is_reserved(this_cpu))
		return;

	/* Cluster isn't initialized until after WALT is enabled */
	order_index = wrq->cluster->id;

	rq_unpin_lock(this_rq, rf);

	/*
	 * Since we drop rq lock while doing RT balance,
	 * check if any tasks are queued on this and bail out
	 * early.
	 */
	if (walt_balance_rt(this_rq) || this_rq->nr_running)
		goto rt_pulled;

	if (!force_overload && !READ_ONCE(this_rq->rd->overload))
		goto repin;

	if (atomic_read(&this_rq->nr_iowait) && !enough_idle)
		goto repin;

	raw_spin_unlock(&this_rq->__lock);

#if IS_ENABLED(CONFIG_XRING_SCHED_VIP)
	/* No need do more balance if vip balance occurs */
	if (vip_sched_newidle_balance(this_rq)) {
		*pulled_task = 1;
		goto unlock;
	}
#endif

	/*
	 * careful, we dropped the lock, and has to be acquired
	 * before returning. Since rq lock is dropped, tasks
	 * can be queued remotely, so keep a check on nr_running
	 * and bail out.
	 */

	order_index = wrq->cluster->id;
	for (i = 0; i < num_sched_clusters; i++) {
		int first_cpu = cpumask_first(&cpu_array[order_index][i]);
		struct walt_rq *src_wrq = &per_cpu(walt_rq, first_cpu);
		int src_cluster_id = src_wrq->cluster->id;

		busy_cpu = walt_find_busiest_cpu(this_cpu, &cpu_array[order_index][i],
										&has_misfit, true);
		if (busy_cpu == -1)
			continue;
		/* when not enough idle
		 *   Small should not help big.
		 *   Big should help small ONLY is mifit is present.
		 *   Same capacity cpus should help each other
		 */
		if (!enough_idle &&
			(capacity_orig_of(this_cpu) < capacity_orig_of(busy_cpu) ||
			(capacity_orig_of(this_cpu) > capacity_orig_of(busy_cpu) && !has_misfit)))
			continue;

		/* if helping farthest cluster,  kick a middle */
		if (num_sched_clusters > 2 &&
		    ((wrq->cluster->id == 0 && src_cluster_id == num_sched_clusters - 1) ||
		    (wrq->cluster->id == num_sched_clusters - 1 && src_cluster_id == 0))) {
			first_idle = find_first_idle_if_others_are_busy();
			if (first_idle != -1) {
				walt_kick_cpu(first_idle);
			} else {
				if (walt_rotate_flag &&
					capacity_orig_of(this_cpu) >
					capacity_orig_of(busy_cpu)) {
					/*
					 * When BTR active help
					 * smallest immediately
					 */
					goto found_busy_cpu;
				}
			}
		} else {
			goto found_busy_cpu;
		}
	}
	goto unlock;

found_busy_cpu:
	/* sanity checks before attempting the pull */
	if (this_rq->nr_running > 0 || (busy_cpu == this_cpu))
		goto unlock;

	*pulled_task = walt_lb_pull_tasks(this_cpu, busy_cpu, &pulled_p);

unlock:
	raw_spin_lock(&this_rq->__lock);
rt_pulled:
	if (this_rq->cfs.h_nr_running && !*pulled_task)
		*pulled_task = 1;

	/* Is there a task of a high priority class? */
	if (this_rq->nr_running != this_rq->cfs.h_nr_running)
		*pulled_task = -1;

	/* reset the idle time stamp if we pulled any task */
	if (*pulled_task)
		this_rq->idle_stamp = 0;

repin:
	rq_repin_lock(this_rq, rf);

	trace_walt_newidle_balance(this_cpu, busy_cpu, *pulled_task,
			enough_idle, pulled_p);
}

/* Run newidle balance to make an unhalted cpu busy */
void walt_ilb_csd(void *info)
{
	int cpu = raw_smp_processor_id();
	struct rq *rq = cpu_rq(cpu);
	struct rq_flags rf;
	int pulled_task;
	int done = 0;

	rq_lock(rq, &rf);

	update_rq_clock(rq);
	walt_newidle_balance(rq, &rf, &pulled_task, &done, true);
	resched_curr(rq);

	rq_unlock(rq, &rf);
}

static DEFINE_PER_CPU(call_single_data_t, ilb_csd);

void walt_kick_ilb(int cpu)
{
	call_single_data_t *csd = &per_cpu(ilb_csd, cpu);

	if (unlikely(walt_disabled))
		return;

	smp_call_function_single_async(cpu, csd);
}

static void walt_find_busiest_queue(void *unused, int dst_cpu,
				    struct sched_group *group,
				    struct cpumask *env_cpus,
				    struct rq **busiest, int *done)
{
	int busiest_cpu = -1;
	struct cpumask src_mask;
	int has_misfit;
	int fsrc_cpu = cpumask_first(sched_group_span(group));

	if (unlikely(walt_disabled))
		return;

	*done = 1;
	*busiest = NULL;

	/* Pick the single cpu in this group */
	if (same_cluster(dst_cpu, fsrc_cpu)) {
		busiest_cpu = fsrc_cpu;
		goto done;
	}

	/*
	 * We will allow inter cluster migrations
	 * only if the source group is sufficiently
	 * loaded. The upstream load balancer is a
	 * bit more generous.
	 *
	 * re-using the same code that we use it
	 * for newly idle load balance. The policies
	 * remain same.
	 */
	cpumask_and(&src_mask, sched_group_span(group), env_cpus);
	busiest_cpu = walt_find_busiest_cpu(dst_cpu, &src_mask, &has_misfit, false);

done:
	if (busiest_cpu != -1)
		*busiest = cpu_rq(busiest_cpu);

	trace_walt_find_busiest_queue(dst_cpu, busiest_cpu, src_mask.bits[0]);
}

/*
 * we only decide if nohz balance kick is needed or not. the
 * first CPU in the nohz.idle will come out of idle and do
 * load balance on behalf of every CPU. adding another hook
 * to decide which cpu to kick is useless. most of the time,
 * it is impossible to decide which CPU has to come out because
 * we get to kick only once.
 */
static void walt_nohz_balancer_kick(void *unused, struct rq *rq,
				    unsigned int *flags, int *done)
{
	if (unlikely(walt_disabled))
		return;

	/*
	 * tick path migration takes care of misfit task.
	 * so we have to check for nr_running >= 2 here.
	 */
	if (rq->nr_running >= 2 && cpu_overutilized(rq->cpu)) {
		*flags = NOHZ_KICK_MASK;
		trace_walt_nohz_balance_kick(rq);
	}

	/* No need to check sched domain status */
	*done = 1;
}

static void walt_can_migrate_task(void *unused, struct task_struct *p,
				  int dst_cpu, int *can_migrate)
{
	bool to_lower, to_higher;

	if (unlikely(walt_disabled))
		return;

	to_lower = capacity_higher(task_cpu(p), dst_cpu);
	to_higher = capacity_higher(dst_cpu, task_cpu(p));

	if (__walt_can_migrate_task(p, dst_cpu, to_lower,
				to_higher, true))
		return;

	*can_migrate = 0;
}

static void walt_sched_newidle_balance(void *unused, struct rq *this_rq,
				       struct rq_flags *rf, int *pulled_task,
				       int *done)
{
	walt_newidle_balance(this_rq, rf, pulled_task, done, false);
}

static void walt_sched_balance_rt(void *unused, struct rq *this_rq,
				  struct task_struct *p, int *done)
{
	if (unlikely(walt_disabled))
		return;

	walt_balance_rt(this_rq);
	*done = 1;
}

void walt_lb_init(void)
{
	call_single_data_t *csd;
	int cpu = 0;

	rotate_work_init();

	register_trace_android_rvh_sched_nohz_balancer_kick(walt_nohz_balancer_kick, NULL);
	register_trace_android_rvh_can_migrate_task(walt_can_migrate_task, NULL);
	register_trace_android_rvh_find_busiest_queue(walt_find_busiest_queue, NULL);
	register_trace_android_rvh_sched_newidle_balance(walt_sched_newidle_balance, NULL);
	register_trace_android_rvh_sched_balance_rt(walt_sched_balance_rt, NULL);

	for_each_possible_cpu(cpu) {
		csd = &per_cpu(ilb_csd, cpu);
		INIT_CSD(csd, walt_ilb_csd, NULL);
	}
}
