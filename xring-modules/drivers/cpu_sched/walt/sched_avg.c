// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

/*
 * Scheduler hook for average runqueue determination
 */
#include <linux/module.h>
#include <linux/percpu.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <linux/math64.h>

#include "walt.h"
#include "trace.h"

static DEFINE_PER_CPU(u64, nr_high_sum);
static DEFINE_PER_CPU(u64, nr_prod_sum);
static DEFINE_PER_CPU(u64, last_time);
static DEFINE_PER_CPU(u64, nr_big_prod_sum);
static DEFINE_PER_CPU(u64, nr);
static DEFINE_PER_CPU(u64, nr_max);

static DEFINE_PER_CPU(spinlock_t, nr_lock) = __SPIN_LOCK_UNLOCKED(nr_lock);
static s64 last_get_time;

#define NR_THRESHOLD_PCT		40

struct sched_avg_stats stats[WALT_NR_CPUS];
unsigned int cstats_util_pct[MAX_CLUSTERS];

/**
 * sched_get_cluster_util_pct
 * @return: provide the percentage of this cluter that was used in the
 *          previous window.
 *
 * This routine may be called any number of times as needed during
 * a window, but will always return the same result until window
 * rollover.
 */
unsigned int sched_get_cluster_util_pct(struct walt_sched_cluster *cluster)
{
	unsigned int cluster_util_pct = 0;

	if (cluster->id < MAX_CLUSTERS)
		cluster_util_pct = cstats_util_pct[cluster->id];

	return cluster_util_pct;
}

/**
 * sched_get_high_load
 * @return: Returns the average percent of CPU needed of when tasks
 *      are executing concurrently since the last poll. A value
 *      greater than 100 means the CPU can not handle the tasks
 *      within the time slice.
 */
u64 sched_get_high_load(int cpu)
{
	u64 cur_time = sched_clock();
	u64 period = cur_time - last_get_time;
	u64 diff = cur_time - per_cpu(last_time, cpu);
	u64 nr_high;

	nr_high = per_cpu(nr_high_sum, cpu);
	if (per_cpu(nr, cpu) >= sysctl_sched_concurrency_thres)
		nr_high += per_cpu(nr, cpu) * diff;
	nr_high = div64_u64((nr_high * 100), period);

	return nr_high;
}

/**
 * sched_get_nr_running_avg
 * @return: Average nr_running, iowait and nr_big_tasks value since last poll.
 *	    Returns the avg * 100 to return up to two decimal points
 *	    of accuracy.
 *
 * Obtains the average nr_running value since the last poll.
 * This function may not be called concurrently with itself.
 *
 * It is assumed that this function is called at most once per window
 * rollover.
 */
struct sched_avg_stats *sched_get_nr_running_avg(void)
{
	int cpu = 0;
	u64 curr_time = sched_clock();
	u64 period = curr_time - last_get_time;
	u64 tmp_nr, tmp_misfit;
	u64 tmp_high;
	struct walt_sched_cluster *cluster = NULL;

	if (!period)
		goto done;

	/* read and reset nr_running counts */
	for_each_possible_cpu(cpu) {
		unsigned long flags;
		u64 diff;

		spin_lock_irqsave(&per_cpu(nr_lock, cpu), flags);
		curr_time = sched_clock();
		diff = curr_time - per_cpu(last_time, cpu);
		WARN_ON((s64)diff < 0);

		tmp_nr = per_cpu(nr_prod_sum, cpu);
		tmp_nr += per_cpu(nr, cpu) * diff;
		tmp_nr = div64_u64((tmp_nr * 100), period);

		tmp_misfit = per_cpu(nr_big_prod_sum, cpu);
		tmp_misfit += walt_big_tasks(cpu) * diff;
		tmp_misfit = div64_u64((tmp_misfit * 100), period);

		tmp_high = per_cpu(nr_high_sum, cpu);
		if (per_cpu(nr, cpu) >= sysctl_sched_concurrency_thres)
			tmp_high += per_cpu(nr, cpu) * diff;
		tmp_high = div64_u64((tmp_high * 100), period);

		/*
		 * NR_THRESHOLD_PCT is to make sure that the task ran
		 * at least 85% in the last window to compensate any
		 * over estimating being done.
		 */
		stats[cpu].nr = (int)div64_u64((tmp_nr + NR_THRESHOLD_PCT),
								100);
		stats[cpu].nr_misfit = (int)div64_u64((tmp_misfit +
						NR_THRESHOLD_PCT), 100);
		stats[cpu].nr_max = per_cpu(nr_max, cpu);
		stats[cpu].nr_scaled = tmp_nr;
		stats[cpu].nr_high = tmp_high;

		trace_sched_get_nr_running_avg(cpu, stats[cpu].nr,
				stats[cpu].nr_misfit, stats[cpu].nr_max,
				stats[cpu].nr_scaled, stats[cpu].nr_high);

		per_cpu(last_time, cpu) = curr_time;
		per_cpu(nr_prod_sum, cpu) = 0;
		per_cpu(nr_big_prod_sum, cpu) = 0;
		per_cpu(nr_high_sum, cpu) = 0;
		per_cpu(nr_max, cpu) = per_cpu(nr, cpu);

		spin_unlock_irqrestore(&per_cpu(nr_lock, cpu), flags);
	}

	/* collect cluster load stats */
	for_each_sched_cluster(cluster) {
		unsigned int num_cpus = cpumask_weight(&cluster->cpus);
		unsigned int sum_util_pct = 0;

		/* load is already scaled, see freq_policy_load/prev_runnable_sum */
		for_each_cpu(cpu, &cluster->cpus) {
			struct rq *rq = cpu_rq(cpu);
			struct walt_rq *wrq = &per_cpu(walt_rq, cpu_of(rq));

			/* compute the % this cpu's utilization of the cpu capacity,
			 * and sum it across all cpus
			 */
			sum_util_pct +=
				(wrq->util * 100) / arch_scale_cpu_capacity(cpu);
		}

		/* calculate the averge per-cpu utilization */
		cstats_util_pct[cluster->id] = sum_util_pct / num_cpus;
	}

	last_get_time = curr_time;

done:
	return &stats[0];
}
EXPORT_SYMBOL(sched_get_nr_running_avg);

/**
 * sched_update_nr_prod
 * @cpu: The core id of the nr running driver.
 * @enq: enqueue/dequeue/misfit happening on this CPU.
 * @return: N/A
 *
 * Update average with latest nr_running value for CPU
 */
void sched_update_nr_prod(int cpu, int enq)
{
	u64 diff;
	u64 curr_time;
	unsigned long flags, nr_running;

	spin_lock_irqsave(&per_cpu(nr_lock, cpu), flags);
	nr_running = per_cpu(nr, cpu);
	curr_time = sched_clock();
	diff = curr_time - per_cpu(last_time, cpu);
	WARN_ON((s64)diff < 0);
	per_cpu(last_time, cpu) = curr_time;
	per_cpu(nr, cpu) = cpu_rq(cpu)->nr_running + enq;

	if (per_cpu(nr, cpu) > per_cpu(nr_max, cpu))
		per_cpu(nr_max, cpu) = per_cpu(nr, cpu);

	per_cpu(nr_prod_sum, cpu) += nr_running * diff;
	per_cpu(nr_big_prod_sum, cpu) += walt_big_tasks(cpu) * diff;

	if (nr_running >= sysctl_sched_concurrency_thres)
		per_cpu(nr_high_sum, cpu) += nr_running * diff;

	spin_unlock_irqrestore(&per_cpu(nr_lock, cpu), flags);
}

/*
 * Returns the CPU utilization % in the last window.
 */
unsigned int sched_get_cpu_util_pct(int cpu)
{
	struct rq *rq = cpu_rq(cpu);
	u64 util;
	unsigned long capacity, flags;
	unsigned int busy;
	struct walt_rq *wrq = &per_cpu(walt_rq, cpu);

	raw_spin_lock_irqsave(&rq->__lock, flags);

	capacity = capacity_orig_of(cpu);

	util = wrq->prev_runnable_sum + wrq->grp_time.prev_runnable_sum;
	util = scale_time_to_util(util);
	raw_spin_unlock_irqrestore(&rq->__lock, flags);

	util = (util >= capacity) ? capacity : util;
	busy = div64_ul((util * 100), capacity);
	return busy;
}
