// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cpufreq.h>

#include "cpufreq_xres.h"
#define CREATE_TRACE_POINTS
#include "xres_trace.h"
#undef CREATE_TRACE_POINTS

static DEFINE_PER_CPU(struct xrgov_cpu, xrgov_cpu);
static DEFINE_PER_CPU(struct xrgov_tunables *, cached_tunables);

/************************ Governor internals ***********************/
static unsigned int default_target_loads[] = {DEFAULT_TARGET_LOAD};
static unsigned int default_above_hispeed_delay[] = {DEFAULT_RATE_LIMIT_US};
static unsigned int default_min_sample_time[] = {DEFAULT_RATE_LIMIT_US};
static struct kthread_worker xres_worker;

static inline unsigned int freq_to_util(unsigned int cpu, unsigned long freq)
{
	struct xrgov_cpu *xg_cpu = &per_cpu(xrgov_cpu, cpu);

	return arch_scale_cpu_capacity(cpu) * freq / xg_cpu->xg_policy->policy->cpuinfo.max_freq;
}

static inline unsigned int util_to_freq(unsigned int cpu, unsigned long util)
{
	struct xrgov_cpu *xg_cpu = &per_cpu(xrgov_cpu, cpu);

	return xg_cpu->xg_policy->policy->cpuinfo.max_freq * util / arch_scale_cpu_capacity(cpu);

}

static bool xrgov_should_update_freq(struct xrgov_policy *xg_policy, u64 time)
{
	s64 delta_ns;
	/*
	 * Since cpufreq_update_util() is called with rq->lock held for
	 * the @target_cpu, our per-CPU data is fully serialized.
	 *
	 * However, drivers cannot in general deal with cross-CPU
	 * requests, so while get_next_freq() will work, our
	 * xrgov_deferred_update() call may not for the fast switching platforms.
	 *
	 * Hence stop here for remote requests if they aren't supported
	 * by the hardware, as calculating the frequency is pointless if
	 * we cannot in fact act on it.
	 *
	 * This is needed on the slow switching platforms too to prevent CPUs
	 * going offline from leaving stale IRQ work items behind.
	 */

	if (unlikely(xg_policy->limits_changed)) {
		xg_policy->limits_changed = false;
		xg_policy->need_freq_update = true;
		return true;
	}

	delta_ns = time - xg_policy->last_freq_update_time;

	return delta_ns >= xg_policy->freq_update_delay_ns &&
	       time - xg_policy->last_work_time_ns > WORK_SPAN_LIMIT_NS;
}

static unsigned int freq_to_targetload(struct xrgov_tunables *tunables,
				       unsigned int freq)
{
	unsigned long flags;
	unsigned int ret;
	int i;

	spin_lock_irqsave(&tunables->target_loads_lock, flags);

	for (i = 0; i < tunables->ntarget_loads - 1 &&
	     freq >= tunables->target_loads[i + 1]; i += 2)
		;

	ret = tunables->target_loads[i];
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);

	return ret;
}

/*
 * If increasing frequencies never map to a lower target load then
 * choose_freq() will find the minimum frequency that does not exceed its
 * target load given the current load.
 */
static unsigned int choose_freq(struct xrgov_policy *xg_policy,
				unsigned int loadadjfreq)
{
	struct cpufreq_policy *policy = xg_policy->policy;
	struct cpufreq_frequency_table *freq_table = policy->freq_table;
	unsigned int prevfreq;
	unsigned int freqmin = 0;
	unsigned int freqmax = UINT_MAX;
	unsigned int tl;
	unsigned int freq = policy->cur;
	int index;

	do {
		prevfreq = freq;
		tl = freq_to_targetload(xg_policy->tunables, freq);
		if (tl == 0)
			break;
		/*
		 * Find the lowest frequency where the computed load is less
		 * than or equal to the target load.
		 */

		index = cpufreq_frequency_table_target(policy, loadadjfreq / tl,
			    CPUFREQ_RELATION_L);

		freq = freq_table[index].frequency;

		if (freq > prevfreq) {
			/* The previous frequency is too low */
			freqmin = prevfreq;

			if (freq < freqmax)
				continue;

			/* Find highest frequency that is less than freqmax */
			index = cpufreq_frequency_table_target(policy,
				    freqmax - 1, CPUFREQ_RELATION_H);

			freq = freq_table[index].frequency;

			if (freq == freqmin) {
				/*
				 * The first frequency below freqmax has already
				 * been found to be too low. freqmax is the
				 * lowest speed we found that is fast enough.
				 */
				freq = freqmax;
				break;
			}
		} else if (freq < prevfreq) {
			/* The previous frequency is high enough. */
			freqmax = prevfreq;

			if (freq > freqmin)
				continue;

			/* Find lowest frequency that is higher than freqmin */
			index = cpufreq_frequency_table_target(policy,
				    freqmin + 1, CPUFREQ_RELATION_L);

			freq = freq_table[index].frequency;

			/*
			 * If freqmax is the first frequency above
			 * freqmin then we have already found that
			 * this speed is fast enough.
			 */
			if (freq == freqmax)
				break;
		}
		/* If same frequency chosen as previous then done. */
	} while (freq != prevfreq);

	return freq;
}

static unsigned int xres_resolve_freq(struct cpufreq_policy *policy,
				      unsigned int raw_freq)
{
	unsigned int index, freq;

	index = cpufreq_frequency_table_target(policy,
		    raw_freq, CPUFREQ_RELATION_L);
	freq =  policy->freq_table[index].frequency;

	return freq;
}


static bool cpu_is_overload(struct xrgov_cpu *xg_cpu)
{
	u64 idle_sum_time, time = 0;
	u64 delta_ns;
	struct xrgov_policy *xg_policy = xg_cpu->xg_policy;

	if (time < xg_cpu->last_idle_update_time)
		return false;

	idle_sum_time = get_cpu_idle_time(xg_cpu->cpu, &time, 0);

	if (idle_sum_time > xg_cpu->last_idle_sum_time)
		xg_cpu->last_idle_update_time = time;

	xg_cpu->last_idle_sum_time = idle_sum_time;
	delta_ns = (time - xg_cpu->last_idle_update_time);

	return delta_ns > xg_policy->overload_duration_ns;
}

static inline void max_and_reason(unsigned long *cur_util, unsigned long boost_util,
		struct xrgov_cpu *xg_cpu, unsigned int reason)
{
	if (boost_util && boost_util >= *cur_util) {
		*cur_util = boost_util;
		xg_cpu->reason = reason;
	}
}

static unsigned int get_boost_freq(struct xrgov_policy *xg_policy, unsigned int boost_flag)
{
	unsigned int target_freq = 0;

	if (boost_flag == 0)
		return target_freq;

	if (boost_flag & WALT_RTG_BOOST)
		xg_policy->rtg_freq_lock = xr_is_rtg_freq_locked(xg_policy->policy->cpus);

	if (xg_policy->rtg_freq_lock) {
		if (xg_policy->policy->fast_switch_enabled &&
		    (boost_flag & (WALT_CPUFREQ_ROLLOVER | WALT_CPUFREQ_POLLING)))
			return 0;

		return xg_policy->tunables->rtg_boost_freq;
	}

	if (boost_flag & WALT_CPUFREQ_BOOST_UPDATE)
		target_freq = max(target_freq, xg_policy->tunables->hispeed_freq);

	if (boost_flag & WALT_IRQ_BOOST)
		target_freq = max(target_freq, xg_policy->tunables->irq_boost_freq);

	if (boost_flag & WALT_CPUFREQ_EARLY_DET)
		target_freq = max(target_freq, xg_policy->tunables->ed_boost_freq);

	if (boost_flag & WALT_TOP_TASK_BOOST)
		target_freq = max(target_freq, xg_policy->tunables->tt_boost_freq);

	if (boost_flag & OVERLOAD_BOOST)
		target_freq = xg_policy->policy->max;

	return target_freq;
}

static void xrgov_get_util(struct xrgov_cpu *xg_cpu, bool raw)
{
	struct xrgov_policy *xg_policy = xg_cpu->xg_policy;
	unsigned long max = arch_scale_cpu_capacity(xg_cpu->cpu);

	xg_cpu->reason = 0;
	xg_cpu->max = max;
	xg_cpu->util = cpu_util_freq_walt(xg_cpu->cpu, &xg_cpu->walt_load, &xg_cpu->reason);
	trace_xresgov_cpu_util(xg_cpu->cpu, xg_cpu->util, max, xg_cpu->reason);

	if (raw) {
		if (xg_policy->tunables->fl) {
			xg_cpu->util = xg_cpu->walt_load.fl;
			xg_cpu->reason = CPUFREQ_REASON_FL;
		}
		return;
	}

	if (xg_policy->tunables->fl) {
		xg_cpu->util = xg_cpu->walt_load.fu;
		xg_cpu->reason = CPUFREQ_REASON_FL;
		return;
	}

	if (xg_policy->tunables->pl)
		max_and_reason(&(xg_cpu->util), xg_cpu->walt_load.pl, xg_cpu, CPUFREQ_REASON_PL);

	if (xg_policy->tunables->nl && xg_cpu->walt_load.nl >= max * 3 / 4)
		max_and_reason(&(xg_cpu->util), max, xg_cpu, CPUFREQ_REASON_NWD);
}

static unsigned int adjust_rtg_freq(struct xrgov_policy *xg_policy,
				    unsigned int freq, unsigned int rtg_freq)
{
	if (freq <= rtg_freq)
		return rtg_freq;

	if (xg_policy->rtg_boost_time > xg_policy->update_time)
		return freq;

	/* reactive rtg lock freq and refresh boost start time */
	if (xg_policy->rtg_freq_switch) {
		xg_policy->rtg_freq_switch = false;
		xg_policy->rtg_boost_time = xg_policy->update_time;
	}

	/* during lag time always use rtg freq to avoid previous high util due to higher freq */
	if (xg_policy->update_time - xg_policy->rtg_boost_time < RTG_LOCK_LAG_TIME_NS)
		return rtg_freq;

	/* above rtg lock freq, rtg need to reactive next time */
	xg_policy->rtg_boost_time = xg_policy->update_time + (RTG_LOCK_LAG_TIME_NS >> 1);
	xg_policy->rtg_freq_switch = true;
	return freq;
}

static unsigned int get_next_freq(struct xrgov_policy *xg_policy,
				 unsigned long util, unsigned int boost_freq)
{
	unsigned int cpu_load = 0;
	struct cpufreq_policy *policy = xg_policy->policy;
	unsigned int rtg_freq, freq = policy->cur;
	struct xrgov_tunables *tunables = xg_policy->tunables;
	struct xrgov_cpu *target_xg_cpu = &per_cpu(xrgov_cpu, xg_policy->driving_cpu);
	bool is_ed_task = target_xg_cpu->walt_load.ed_active;
	u64 now;

	cpu_load = util * 100 / capacity_curr_of(policy->cpu);
	freq = choose_freq(xg_policy, cpu_load * policy->cur);
	trace_xresgov_cpu_load(cpu_load, util, capacity_curr_of(policy->cpu),
			       policy->cur, freq, xg_policy->driving_cpu);

	/*rtg_boost*/
	if (xg_policy->rtg_freq_lock) {
		rtg_freq = tunables->rtg_boost_freq;
		freq = adjust_rtg_freq(xg_policy, freq, rtg_freq);
		if (freq == rtg_freq)
			target_xg_cpu->reason = CPUFREQ_REASON_RTG_BOOST;

		return freq;
	}

	/*boost*/
	if (freq < boost_freq) {
		freq = boost_freq;
		target_xg_cpu->reason = CPUFREQ_REASON_BOOST;
	}

	/*should hispeed*/
	if (cpu_load > tunables->hispeed_load && freq < tunables->hispeed_freq) {
		freq = tunables->hispeed_freq;
		target_xg_cpu->reason = CPUFREQ_REASON_HISPEED;
	}

	/*ed_task*/
	if (is_ed_task && freq < tunables->ed_boost_freq) {
		freq = tunables->ed_boost_freq;
		target_xg_cpu->reason = CPUFREQ_REASON_EARLY_DET;
	}

	/*boost_pulse*/
	now = ktime_to_us(ktime_get());
	if (now < xg_policy->boostpulse_endtime_us && freq < tunables->hispeed_freq) {
		freq = tunables->hispeed_freq;
		target_xg_cpu->reason = CPUFREQ_REASON_BOOST;
	}

	/*adaptive_freq*/
	if (freq < tunables->adaptive_low_freq) {
		freq = tunables->adaptive_low_freq;
		target_xg_cpu->reason |= CPUFREQ_REASON_ADAPTIVE_LOW;
	} else if (freq < tunables->adaptive_high_freq) {
		freq = tunables->adaptive_high_freq;
		target_xg_cpu->reason |= CPUFREQ_REASON_ADAPTIVE_HIGH;
	}

	return freq;
}

static unsigned int freq_to_above_hispeed_delay(
	struct xrgov_tunables *tunables,
	unsigned int freq)
{
	int i;
	unsigned int ret;
	unsigned long flags;

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);

	for (i = 0; i < tunables->nabove_hispeed_delay - 1 &&
			freq >= tunables->above_hispeed_delay[i+1]; i += 2)
		;

	ret = tunables->above_hispeed_delay[i];
	spin_unlock_irqrestore(&tunables->above_hispeed_delay_lock, flags);

	return ret;
}

static unsigned int freq_to_min_sample_time(struct xrgov_tunables *tunables,
					    unsigned int freq)
{
	unsigned long flags;
	unsigned int ret;
	int i;

	spin_lock_irqsave(&tunables->min_sample_time_lock, flags);

	for (i = 0; i < tunables->nmin_sample_time - 1 &&
	     freq >= tunables->min_sample_time[i + 1]; i += 2)
		;

	ret = tunables->min_sample_time[i];
	spin_unlock_irqrestore(&tunables->min_sample_time_lock, flags);

	return ret;
}

static bool xrgov_up_down_rate_limit(struct xrgov_policy *xg_policy, u64 time,
				     unsigned int next_freq)
{
	s64 delta_ns;
	struct xrgov_tunables *tunables = xg_policy->tunables;
	u64 min_sample_time;
	u64 above_hispeed_delay;

	if (xg_policy->next_freq < tunables->hispeed_freq)
		return false;

	if (next_freq > xg_policy->next_freq &&
	    xg_policy->next_freq >= tunables->hispeed_freq) {
		delta_ns = time - xg_policy->hispeed_validate_time_ns;
		above_hispeed_delay = freq_to_above_hispeed_delay(xg_policy->tunables,
								 xg_policy->next_freq);
		trace_xresgov_cpu_limits_time(xg_policy->policy->cpu, time, above_hispeed_delay,
						xg_policy->hispeed_validate_time_ns, delta_ns,
						xg_policy->next_freq, next_freq);

		if (delta_ns < NSEC_PER_USEC * above_hispeed_delay)
			return true;
	}

	if (next_freq < xg_policy->next_freq &&
		xg_policy->next_freq >= tunables->hispeed_freq) {
		min_sample_time = freq_to_min_sample_time(xg_policy->tunables,
							 xg_policy->next_freq);
		delta_ns = time - xg_policy->floor_validate_time_ns;
		trace_xresgov_cpu_limits_time(xg_policy->policy->cpu, time, min_sample_time,
						xg_policy->floor_validate_time_ns, delta_ns,
						xg_policy->next_freq, next_freq);

		if (delta_ns < NSEC_PER_USEC * min_sample_time)
			return true;
	}

	return false;
}


static void __xrgov_update_next_freq(struct xrgov_policy *xg_policy, u64 time,
				   unsigned int next_freq, unsigned int cached_raw_freq)
{
	xg_policy->next_freq = next_freq;
	xg_policy->last_freq_update_time = time;
	xg_policy->hispeed_validate_time_ns = time;
	xg_policy->floor_validate_time_ns = time;
	xg_policy->cached_raw_freq = cached_raw_freq;
}

static bool xrgov_update_next_freq(struct xrgov_policy *xg_policy, u64 time,
				   unsigned int next_freq, unsigned int cached_raw_freq)
{

	if (xg_policy->need_freq_update) {
		xg_policy->need_freq_update = false;
		goto out;
	}

	if (xg_policy->next_freq == next_freq)
		return false;

	if (xrgov_up_down_rate_limit(xg_policy, time, next_freq) &&
	    !xg_policy->fast_ramp)
		return false;
out:
	xg_policy->fast_ramp = false;
	__xrgov_update_next_freq(xg_policy, time, next_freq, cached_raw_freq);

	return true;
}

static void xrgov_deferred_update(struct xrgov_policy *xg_policy)
{
	if (!xg_policy->work_in_progress) {
		xg_policy->work_in_progress = true;
		if (likely(cpu_online(raw_smp_processor_id())))
			irq_work_queue(&xg_policy->irq_work);
		else
			irq_work_queue_on(&xg_policy->irq_work, cpumask_any(cpu_online_mask));
	}
}

bool xrgov_cpu_boost(struct xrgov_policy *xg_policy, u64 time, unsigned int boost_flag)
{
	unsigned int target_freq = 0;
	unsigned int cached_raw_freq = 0;

	target_freq = get_boost_freq(xg_policy, boost_flag);

	if (target_freq == 0)
		return false;

	cached_raw_freq = target_freq;

	/* update ts when rtg freq lock enable or reg freq being changed */
	if (boost_flag & WALT_RTG_BOOST)
		xg_policy->rtg_boost_time = time;

	if (xg_policy->cached_raw_freq < cached_raw_freq || (boost_flag & WALT_RTG_BOOST)) {
		target_freq = xres_resolve_freq(xg_policy->policy, cached_raw_freq);
		xg_policy->fast_ramp = true;

		if (xg_policy->policy->fast_switch_enabled) {
			xg_policy->fast_ramp = false;
			__xrgov_update_next_freq(xg_policy, time, target_freq, cached_raw_freq);
			cpufreq_driver_fast_switch(xg_policy->policy, xg_policy->next_freq);
		} else {
			xrgov_deferred_update(xg_policy);
		}

		trace_xresgov_boost_freq(xg_policy->policy->cpu, target_freq, boost_flag);
	}

	return true;
}

/**
 * xrgov_iowait_reset() - Reset the IO boost status of a CPU.
 * @xg_cpu: the xrgov data for the CPU to boost
 * @time: the update time from the caller
 * @set_iowait_boost: true if an IO boost has been requested
 *
 * The IO wait boost of a task is disabled after a tick since the last update
 * of a CPU. If a new IO wait boost is requested after more then a tick, then
 * we enable the boost starting from IOWAIT_BOOST_MIN, which improves energy
 * efficiency by ignoring sporadic wakeups from IO.
 */
static bool xrgov_iowait_reset(struct xrgov_cpu *xg_cpu, u64 time,
			       bool set_iowait_boost)
{
	s64 delta_ns = time - xg_cpu->last_iowait;

	/* Reset boost only if a tick has elapsed since last request */
	if (delta_ns <= IOWAIT_BOOST_CLEAR_NS)
		return false;

	xg_cpu->iowait_boost_pending = set_iowait_boost;
	xg_cpu->iowait_boost_freq = set_iowait_boost ?
				    xg_cpu->xg_policy->tunables->iowait_boost_step : 0;

	if (set_iowait_boost)
		xg_cpu->last_iowait = time;

	return true;
}

/**
 * xrgov_iowait_boost() - Updates the IO boost status of a CPU.
 * @xg_cpu: the xrgov data for the CPU to boost
 * @time: the update time from the caller
 * @flags: SCHED_CPUFREQ_IOWAIT if the task is waking up after an IO wait
 *
 * Each time a task wakes up after an IO operation, the CPU utilization can be
 * boosted to a certain utilization which doubles at each "frequent and
 * successive" wakeup from IO, ranging from IOWAIT_BOOST_MIN to the utilization
 * of the maximum OPP.
 *
 * To keep doubling, an IO boost has to be requested at least once per tick,
 * otherwise we restart from the utilization of the minimum OPP.
 */
static void xrgov_iowait_boost(struct xrgov_cpu *xg_cpu, u64 time,
			       unsigned int flags)
{
	bool set_iowait_boost = flags & WALT_IO_WAIT_BOOST;

	/* Boost only tasks waking up after IO */
	if (!set_iowait_boost)
		return;

	/* Reset boost if the CPU appears to have been idle enough */
	if (xg_cpu->iowait_boost_freq &&
	    xrgov_iowait_reset(xg_cpu, time, set_iowait_boost))
		return;

	/* Ensure boost doubles only one time at each request */
	if (xg_cpu->iowait_boost_pending)
		return;

	xg_cpu->iowait_boost_pending = true;
	xg_cpu->last_iowait = time;

	/*
	 * Ignore pending and increase iowait_boost every time
	 * in a smooth way.
	 */
	if (xg_cpu->iowait_boost_freq) {
		xg_cpu->iowait_boost_freq += xg_cpu->xg_policy->tunables->iowait_boost_step;
	} else {
		/* First wakeup after IO: start with minimum boost */
		xg_cpu->iowait_boost_freq = xg_cpu->xg_policy->tunables->iowait_boost_step;
	}
}

/**
 * xrgov_iowait_apply() - Apply the IO boost to a CPU.
 * @xg_cpu: the xrgov data for the cpu to boost
 * @time: the update time from the caller
 *
 * A CPU running a task which woken up after an IO operation can have its
 * utilization boosted to speed up the completion of those IO operations.
 * The IO boost value is increased each time a task wakes up from IO, in
 * xrgov_iowait_apply(), and it's instead decreased by this function,
 * each time an increase has not been requested (!iowait_boost_pending).
 *
 * A CPU which also appears to have been idle for at least one tick has also
 * its IO boost utilization reset.
 *
 * This mechanism is designed to boost high frequently IO waiting tasks, while
 * being more conservative on tasks which does sporadic IO operations.
 */
void xrgov_iowait_apply(struct xrgov_cpu *xg_cpu, u64 time)
{
	unsigned int freq;

	xg_cpu->iowait_boost = 0;
	/* No boost currently required */
	if (!xg_cpu->iowait_boost_freq)
		return;

	/* Reset boost if the CPU appears to have been idle enough */
	if (xrgov_iowait_reset(xg_cpu, time, false))
		return;

	if (!xg_cpu->iowait_boost_pending) {
		/*
		 * No boost pending; reduce the boost value.
		 */
		xg_cpu->iowait_boost_freq >>= 1;
		if (xg_cpu->iowait_boost_freq < xg_cpu->xg_policy->tunables->iowait_boost_step) {
			xg_cpu->iowait_boost_freq = 0;
			return;
		}
	}

	xg_cpu->iowait_boost_pending = false;
	freq = xg_cpu->iowait_boost_freq + xg_cpu->xg_policy->policy->cur;
	if (freq > xg_cpu->xg_policy->tunables->iowait_upper_limit)
		freq = xg_cpu->xg_policy->tunables->iowait_upper_limit;
	xg_cpu->iowait_boost = freq_to_util(xg_cpu->cpu, freq);
	trace_xresgov_io_wait_boost(xg_cpu->cpu, xg_cpu->util, xg_cpu->iowait_boost, freq);
}

static void xrgov_update_freq(struct xresgov_callback *cb, u64 time,
				unsigned int flags)
{
	struct xrgov_cpu *xg_cpu = container_of(cb, struct xrgov_cpu, cb);
	struct xrgov_policy *xg_policy = xg_cpu->xg_policy;
	struct xrgov_tunables *tunables = xg_policy->tunables;
	unsigned long lock_flag = 0;

	xrgov_iowait_boost(xg_cpu, time, flags);

	if (cpu_is_overload(xg_cpu))
		flags |= OVERLOAD_BOOST;

	raw_spin_lock_irqsave(&xg_policy->update_lock, lock_flag);

	if (flags == WALT_IO_WAIT_BOOST &&
	    xg_policy->policy->cur >= tunables->iowait_upper_limit)
		goto out;

	if (flags == WALT_CPUFREQ_IC_MIGRATION_ADD &&
	    xg_policy->policy->cur >= xg_policy->policy->max)
		goto out;

	if (flags == WALT_CPUFREQ_IC_MIGRATION_SUB &&
	    (xg_policy->policy->cur <= xg_policy->policy->min ||
	     xrgov_up_down_rate_limit(xg_policy, time, xg_policy->next_freq - 1)))
		goto out;

	if (flags == WALT_CPUFREQ_PENDING) {
		if (xg_policy->policy->cur > xg_policy->policy->min) {
			flags = WALT_CPUFREQ_ROLLOVER;
			goto update_freq;
		}

		xg_policy->pending_update = true;
		goto out;
	}

	if (flags & WALT_CPUFREQ_PL) {
		if (tunables->pl)
			goto update_freq;
		else
			flags &= ~WALT_CPUFREQ_PL;
	}

	if (flags == 0)
		goto out;

	if ((flags & WALT_CPUFREQ_POLLING) && !xg_policy->pending_update)
		goto out;

update_freq:
	xg_policy->pending_update = false;

	if (flags & WALT_IRQ_UNBOOST)
		xg_cpu->flags &= ~WALT_IRQ_BOOST;

	xg_cpu->flags |= flags;
	xg_policy->update_time = time;
	if (!xrgov_cpu_boost(xg_policy, time, flags)) {
		if (xrgov_should_update_freq(xg_policy, time)) {
			xg_policy->last_work_time_ns = time;
			trace_xresgov_trigger(xg_cpu->cpu, flags);
			xrgov_deferred_update(xg_policy);
		}
	}

out:
	raw_spin_unlock_irqrestore(&xg_policy->update_lock, lock_flag);
}

static void xrgov_slack_timer_resched(struct xrgov_policy *xg_policy)
{
	u64 expires;

	raw_spin_lock(&xg_policy->timer_lock);

	if (!xg_policy->governor_enabled)
		goto unlock;

	del_timer(&xg_policy->pol_slack_timer);

	if (xg_policy->tunables->timer_slack > 0 &&
	    xg_policy->next_freq > xg_policy->policy->min) {
		expires = jiffies + usecs_to_jiffies(xg_policy->tunables->timer_slack);
		xg_policy->pol_slack_timer.expires = expires;
		add_timer_on(&xg_policy->pol_slack_timer, xg_policy->driving_cpu);
	}

unlock:
	raw_spin_unlock(&xg_policy->timer_lock);
}

static void xrgov_work(struct kthread_work *work)
{
	struct xrgov_policy *xg_policy = container_of(work, struct xrgov_policy, work);
	struct cpufreq_policy *policy = xg_policy->policy;
	unsigned int freq = 0;
	unsigned int raw_freq = 0;
	unsigned long lock_flags;
	unsigned long util = 0, max = 1;
	unsigned int j = -1;
	unsigned int max_boost_freq = 0;
	struct xrgov_cpu *xg_driv_cpu = 0;
	bool do_adjust = false;
	void *cpufreq_drvdata = NULL;
	void (*adjust_func)(unsigned int a, unsigned int b);

	/*
	 * Hold xg_policy->update_lock shortly to handle the case where:
	 * in case xg_policy->next_freq is read here, and then updated by
	 * xrgov_deferred_update() just before work_in_progress is set to false
	 * here, we may miss queueing the new update.
	 *
	 * Note: If a work was queued after the update_lock is released,
	 * xrgov_work() will just be called again by kthread_work code; and the
	 * request will be proceed before the sugov thread sleeps.
	 */
	raw_spin_lock_irqsave(&xg_policy->update_lock, lock_flags);

	/* keep save ts to catch the start point of rtg freq lock */
	if (!xg_policy->rtg_freq_lock)
		xg_policy->rtg_boost_time = xg_policy->update_time;

	xg_policy->rtg_freq_lock = xr_is_rtg_freq_locked(policy->cpus);

	for_each_cpu(j, policy->cpus) {
		struct xrgov_cpu *j_xg_cpu = &per_cpu(xrgov_cpu, j);
		unsigned long j_util, j_max;

		xrgov_get_util(j_xg_cpu, xg_policy->rtg_freq_lock);
		max_boost_freq = max(get_boost_freq(xg_policy, j_xg_cpu->flags), max_boost_freq);
		xrgov_iowait_apply(j_xg_cpu, xg_policy->update_time);

		if (!xg_policy->rtg_freq_lock &&
		    j_xg_cpu->util < j_xg_cpu->iowait_boost) {
			j_xg_cpu->util = j_xg_cpu->iowait_boost;
			j_xg_cpu->reason = CPUFREQ_REASON_IO_WAIT_BOOST;
		}
		if (j_xg_cpu->reason == CPUFREQ_REASON_TT_LOAD ||
		    j_xg_cpu->reason == CPUFREQ_REASON_RTG_BOOST ||
		    j_xg_cpu->reason == CPUFREQ_REASON_IO_WAIT_BOOST)
			do_adjust = true;

		j_util = j_xg_cpu->util;
		j_max = j_xg_cpu->max;

		if (j_util * max > j_max * util) {
			util = j_util;
			max = j_max;
			xg_policy->driving_cpu =  j;
		}

		j_xg_cpu->flags &= RESERVED_FLAGS;
	}

	raw_freq = get_next_freq(xg_policy, util, max_boost_freq);
	freq = xres_resolve_freq(policy, raw_freq);
	xg_driv_cpu = &per_cpu(xrgov_cpu, xg_policy->driving_cpu);
	xg_policy->work_in_progress = false;

	if (!xrgov_update_next_freq(xg_policy, xg_policy->update_time,
		freq, raw_freq)) {
		raw_spin_unlock_irqrestore(&xg_policy->update_lock, lock_flags);
		return;
	}

	cpufreq_drvdata = cpufreq_get_driver_data();
	if (cpufreq_drvdata) {
		adjust_func = (void (*)(unsigned int, unsigned int))cpufreq_drvdata;
		if (do_adjust)
			adjust_func(policy->cpu, freq);
		else
			adjust_func(policy->cpu, 0);
	}

	trace_xresgov_next_freq(policy->cpu, util, max, xg_policy->cached_raw_freq, freq,
				policy->min, policy->max,
				xg_policy->cached_raw_freq, xg_policy->need_freq_update,
				xg_policy->driving_cpu, xg_driv_cpu->reason);

	raw_spin_unlock_irqrestore(&xg_policy->update_lock, lock_flags);
	mutex_lock(&xg_policy->work_lock);
	__cpufreq_driver_target(xg_policy->policy, freq, CPUFREQ_RELATION_L);
	mutex_unlock(&xg_policy->work_lock);
	xrgov_slack_timer_resched(xg_policy);

}

static void xrgov_irq_work(struct irq_work *irq_work)
{
	struct xrgov_policy *xg_policy;

	xg_policy = container_of(irq_work, struct xrgov_policy, irq_work);

	kthread_queue_work(&xres_worker, &xg_policy->work);
}

/************************** sysfs interface ************************/

static inline struct xrgov_tunables *to_xrgov_tunables(struct gov_attr_set *attr_set)
{
	return container_of(attr_set, struct xrgov_tunables, attr_set);
}

#define U32_ATTR_SIMPLE_SHOW(_name)		static ssize_t \
_name##_show(struct gov_attr_set *attr_set, char *buf)	\
{	\
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);	\
	return snprintf(buf, PAGE_SIZE, "%u\n", tunables->_name);	\
}

#define U32_ATTR_SIMPLE_STORE(_name)		static ssize_t \
_name##_store(struct gov_attr_set *attr_set, const char *buf, size_t count) \
{	\
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set); \
	unsigned int val;  \
	if (kstrtouint(buf, 10, &val)) \
		return -EINVAL; \
	tunables->_name = val;\
	return count;\
}

static ssize_t rate_limit_us_show(struct gov_attr_set *attr_set, char *buf)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);

	return sprintf(buf, "%u\n", tunables->rate_limit_us);
}

static ssize_t
rate_limit_us_store(struct gov_attr_set *attr_set, const char *buf, size_t count)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	struct xrgov_policy *xg_policy;
	unsigned int rate_limit_us;

	if (kstrtouint(buf, 10, &rate_limit_us))
		return -EINVAL;

	tunables->rate_limit_us = rate_limit_us;

	list_for_each_entry(xg_policy, &attr_set->policy_list, tunables_hook)
		xg_policy->freq_update_delay_ns = rate_limit_us * NSEC_PER_USEC;

	return count;
}

static unsigned int *get_tokenized_data(const char *buf, int *num_tokens)
{
	const char *cp = NULL;
	int i;
	int ntokens = 1;
	unsigned int *tokenized_data = NULL;
	int err = -EINVAL;

	cp = buf;
	while ((cp = strpbrk(cp + 1, " :")))
		ntokens++;

	if (!(ntokens & 0x1))
		goto err;

	tokenized_data = kmalloc_array(ntokens, sizeof(unsigned int), GFP_KERNEL);
	if (!tokenized_data) {
		err = -ENOMEM;
		goto err;
	}

	cp = buf;
	i = 0;
	while (i < ntokens) {
		if (sscanf(cp, "%u", &tokenized_data[i++]) != 1)
			goto err_kfree;

		cp = strpbrk(cp, " :");
		if (!cp)
			break;
		cp++;
	}

	if (i != ntokens)
		goto err_kfree;

	*num_tokens = ntokens;

	return tokenized_data;

err_kfree:
	kfree(tokenized_data);
err:
	return ERR_PTR(err);
}

static ssize_t above_hispeed_delay_show(struct gov_attr_set *attr_set,
					char *buf)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	unsigned long flags;
	ssize_t ret = 0;
	int i;

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);

	for (i = 0; i < tunables->nabove_hispeed_delay; i++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s",
				 tunables->above_hispeed_delay[i],
				 i & 0x1 ? ":" : " ");

	scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	spin_unlock_irqrestore(&tunables->above_hispeed_delay_lock, flags);

	return ret;
}

static ssize_t above_hispeed_delay_store(struct gov_attr_set *attr_set,
				  const char *buf, size_t count)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	unsigned int *new_above_hispeed_delay;
	unsigned long flags;
	int ntokens;

	new_above_hispeed_delay = get_tokenized_data(buf, &ntokens);

	if (IS_ERR(new_above_hispeed_delay))
		return PTR_ERR(new_above_hispeed_delay);

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);

	if (tunables->above_hispeed_delay != default_above_hispeed_delay)
		kfree(tunables->above_hispeed_delay);

	tunables->above_hispeed_delay = new_above_hispeed_delay;
	tunables->nabove_hispeed_delay = ntokens;
	spin_unlock_irqrestore(&tunables->above_hispeed_delay_lock, flags);

	return count;
}


static ssize_t min_sample_time_show(struct gov_attr_set *attr_set, char *buf)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	unsigned long flags;
	ssize_t ret = 0;
	int i;

	spin_lock_irqsave(&tunables->min_sample_time_lock, flags);

	for (i = 0; i < tunables->nmin_sample_time; i++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s",
				 tunables->min_sample_time[i],
				 i & 0x1 ? ":" : " ");

	scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	spin_unlock_irqrestore(&tunables->min_sample_time_lock, flags);

	return ret;
}

static ssize_t min_sample_time_store(struct gov_attr_set *attr_set,
				  const char *buf, size_t count)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	unsigned int *new_min_sample_time;
	unsigned long flags;
	int ntokens;

	new_min_sample_time = get_tokenized_data(buf, &ntokens);

	if (IS_ERR(new_min_sample_time))
		return PTR_ERR(new_min_sample_time);

	spin_lock_irqsave(&tunables->min_sample_time_lock, flags);

	if (tunables->min_sample_time != default_min_sample_time)
		kfree(tunables->min_sample_time);

	tunables->min_sample_time = new_min_sample_time;
	tunables->nmin_sample_time = ntokens;
	spin_unlock_irqrestore(&tunables->min_sample_time_lock, flags);

	return count;
}

U32_ATTR_SIMPLE_SHOW(hispeed_freq);

U32_ATTR_SIMPLE_STORE(hispeed_freq);

U32_ATTR_SIMPLE_SHOW(hispeed_load);

U32_ATTR_SIMPLE_STORE(hispeed_load);

static ssize_t boostpulse_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	struct xrgov_policy *xg_policy = NULL;
	unsigned int val;
	u64 now;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	if (val != 1)
		return -EINVAL;

	list_for_each_entry(xg_policy, &attr_set->policy_list, tunables_hook) {
		if (xg_policy->tunables == tunables) {
			struct rq *rq = cpu_rq(xg_policy->policy->cpu);
			unsigned long flags;

			now = ktime_to_us(ktime_get());
			if (xg_policy->boostpulse_endtime_us +
			    tunables->boostpulse_min_interval > now)
				return count;

			xg_policy->boostpulse_endtime_us = now +
							   tunables->boostpulse_duration;

			raw_spin_lock_irqsave(&rq->__lock, flags);
			xresgov_run_callback(xg_policy->policy->cpu, WALT_CPUFREQ_BOOST_UPDATE);
			raw_spin_unlock_irqrestore(&rq->__lock, flags);
		}
	}

	return count;
}

U32_ATTR_SIMPLE_SHOW(boostpulse_duration);

U32_ATTR_SIMPLE_STORE(boostpulse_duration);

U32_ATTR_SIMPLE_SHOW(overload_duration);

static ssize_t
overload_duration_store(struct gov_attr_set *attr_set, const char *buf, size_t count)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	struct xrgov_policy *xg_policy = NULL;
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->overload_duration = val;

	list_for_each_entry(xg_policy, &attr_set->policy_list, tunables_hook)
		xg_policy->overload_duration_ns = val * NSEC_PER_MSEC;

	return count;
}

static ssize_t target_loads_show(struct gov_attr_set *attr_set, char *buf)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	unsigned long flags;
	ssize_t ret = 0;
	int i;

	spin_lock_irqsave(&tunables->target_loads_lock, flags);

	for (i = 0; i < tunables->ntarget_loads; i++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s",
				 tunables->target_loads[i],
				 i & 0x1 ? ":" : " ");

	scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);

	return ret;
}

static ssize_t
target_loads_store(struct gov_attr_set *attr_set, const char *buf, size_t count)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	unsigned int *new_target_loads;
	unsigned long flags;
	int ntokens;
	int i;

	new_target_loads = get_tokenized_data(buf, &ntokens);
	if (IS_ERR(new_target_loads))
		return PTR_ERR(new_target_loads);

	for (i = 0; i < ntokens; i++) {
		if (new_target_loads[i] == 0) {
			kfree(new_target_loads);
			return -EINVAL;
		}
	}

	spin_lock_irqsave(&tunables->target_loads_lock, flags);
	if (tunables->target_loads != default_target_loads)
		kfree(tunables->target_loads);
	tunables->target_loads = new_target_loads;
	tunables->ntarget_loads = ntokens;
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);

	return count;
}

U32_ATTR_SIMPLE_SHOW(timer_slack);

U32_ATTR_SIMPLE_STORE(timer_slack);

U32_ATTR_SIMPLE_SHOW(adaptive_low_freq);

U32_ATTR_SIMPLE_STORE(adaptive_low_freq);

U32_ATTR_SIMPLE_SHOW(adaptive_high_freq);

U32_ATTR_SIMPLE_STORE(adaptive_high_freq);

U32_ATTR_SIMPLE_SHOW(iowait_boost_step);

static ssize_t
iowait_boost_step_store(struct gov_attr_set *attr_set, const char *buf, size_t count)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);

	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	if (val < IOWAIT_BOOST_STEP_MAX)
		return -EINVAL;
	tunables->iowait_boost_step = val;

	return count;
};

U32_ATTR_SIMPLE_SHOW(iowait_upper_limit);

static ssize_t
iowait_upper_limit_store(struct gov_attr_set *attr_set, const char *buf, size_t count)
{
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->iowait_upper_limit = val;

	return count;
};

U32_ATTR_SIMPLE_SHOW(rtg_boost_freq);

static ssize_t rtg_boost_freq_store(struct gov_attr_set *attr_set, const char *buf, size_t count)
{
	struct xrgov_policy *xg_policy = NULL;
	struct cpufreq_policy *policy = NULL;
	struct xrgov_tunables *tunables = to_xrgov_tunables(attr_set);
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	list_for_each_entry(xg_policy, &attr_set->policy_list, tunables_hook) {
		if (xg_policy->tunables == tunables) {
			policy = xg_policy->policy;
			if (val != tunables->rtg_boost_freq) {
				struct rq *rq = cpu_rq(policy->cpu);
				unsigned long flags;

				tunables->rtg_boost_freq = val;
				raw_spin_lock_irqsave(&rq->__lock, flags);
				xresgov_run_callback(policy->cpu, WALT_RTG_BOOST);
				raw_spin_unlock_irqrestore(&rq->__lock, flags);
				break;
			}
		}
	}

	return count;
}

U32_ATTR_SIMPLE_SHOW(irq_boost_freq);

U32_ATTR_SIMPLE_STORE(irq_boost_freq);

U32_ATTR_SIMPLE_SHOW(fl);

U32_ATTR_SIMPLE_STORE(fl);

U32_ATTR_SIMPLE_SHOW(pl);

U32_ATTR_SIMPLE_STORE(pl);

U32_ATTR_SIMPLE_SHOW(nl);

U32_ATTR_SIMPLE_STORE(nl);


struct governor_attr rate_limit_us = __ATTR_RW_MODE(rate_limit_us, 0660);
struct governor_attr above_hispeed_delay = __ATTR_RW_MODE(above_hispeed_delay, 0660);
struct governor_attr min_sample_time = __ATTR_RW_MODE(min_sample_time, 0660);
struct governor_attr hispeed_freq = __ATTR_RW_MODE(hispeed_freq, 0660);
struct governor_attr hispeed_load = __ATTR_RW_MODE(hispeed_load, 0660);
struct governor_attr boostpulse = __ATTR_WO(boostpulse);
struct governor_attr boostpulse_duration = __ATTR_RW_MODE(boostpulse_duration, 0660);
struct governor_attr overload_duration = __ATTR_RW_MODE(overload_duration, 0660);
struct governor_attr target_loads = __ATTR_RW_MODE(target_loads, 0660);
struct governor_attr timer_slack = __ATTR_RW_MODE(timer_slack, 0660);
struct governor_attr adaptive_low_freq = __ATTR_RW_MODE(adaptive_low_freq, 0660);
struct governor_attr adaptive_high_freq = __ATTR_RW_MODE(adaptive_high_freq, 0660);
struct governor_attr iowait_boost_step = __ATTR_RW_MODE(iowait_boost_step, 0660);
struct governor_attr iowait_upper_limit = __ATTR_RW_MODE(iowait_upper_limit, 0660);
struct governor_attr rtg_boost_freq = __ATTR_RW_MODE(rtg_boost_freq, 0660);
struct governor_attr irq_boost_freq = __ATTR_RW_MODE(irq_boost_freq, 0660);
struct governor_attr fl = __ATTR_RW_MODE(fl, 0660);
struct governor_attr pl = __ATTR_RW_MODE(pl, 0660);
struct governor_attr nl = __ATTR_RW_MODE(nl, 0660);

#ifdef CONFIG_XRING_DEBUG
#define FLAG_LIST_SIZE 10
static ssize_t touch_run_callback_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct xrgov_policy *xg_policy = NULL;
	struct cpufreq_policy *policy = NULL;
	unsigned int val;
	u64 now;
	unsigned int flag = 0;
	unsigned int flag_map[FLAG_LIST_SIZE] = {WALT_RTG_BOOST, WALT_IO_WAIT_BOOST,
						WALT_TOP_TASK_BOOST, WALT_CPUFREQ_EARLY_DET,
						WALT_IRQ_BOOST, 0};
	int j;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	if (val >= FLAG_LIST_SIZE)
		return -EINVAL;

	now = ktime_to_us(ktime_get());
	flag = flag_map[val];

	list_for_each_entry(xg_policy, &attr_set->policy_list, tunables_hook) {
		policy = xg_policy->policy;
		for_each_cpu(j, policy->cpus) {
			struct rq *rq = cpu_rq(xg_policy->policy->cpu);
			unsigned long flags;

			raw_spin_lock_irqsave(&rq->__lock, flags);
			xresgov_run_callback(j, flag);
			raw_spin_unlock_irqrestore(&rq->__lock, flags);

			if (flag == WALT_IO_WAIT_BOOST)
				xrgov_iowait_apply(&per_cpu(xrgov_cpu, j), walt_sched_clock());
		}
	}

	return count;
}
struct governor_attr touch_run_callback = __ATTR_WO(touch_run_callback);
#endif

static struct attribute *xrgov_attrs[] = {
	&rate_limit_us.attr,
	&above_hispeed_delay.attr,
	&min_sample_time.attr,
	&hispeed_freq.attr,
	&hispeed_load.attr,
	&boostpulse.attr,
	&boostpulse_duration.attr,
	&overload_duration.attr,
	&target_loads.attr,
	&timer_slack.attr,
	&adaptive_low_freq.attr,
	&adaptive_high_freq.attr,
	&iowait_boost_step.attr,
	&iowait_upper_limit.attr,
	&rtg_boost_freq.attr,
	&irq_boost_freq.attr,
	&fl.attr,
	&pl.attr,
	&nl.attr,
#ifdef CONFIG_XRING_DEBUG
	&touch_run_callback.attr,
#endif
	NULL
};
ATTRIBUTE_GROUPS(xrgov);

static void xrgov_tunables_free(struct kobject *kobj)
{
	struct gov_attr_set *attr_set = container_of(kobj, struct gov_attr_set, kobj);

	kfree(to_xrgov_tunables(attr_set));
}

static struct kobj_type xrgov_tunables_ktype = {
	.default_groups = xrgov_groups,
	.sysfs_ops = &governor_sysfs_ops,
	.release = &xrgov_tunables_free,
};

/********************** cpufreq governor interface *********************/

struct cpufreq_governor xres_gov;

static struct xrgov_policy *xrgov_policy_alloc(struct cpufreq_policy *policy)
{
	struct xrgov_policy *xg_policy;

	xg_policy = kzalloc(sizeof(*xg_policy), GFP_KERNEL);
	if (!xg_policy)
		return NULL;

	xg_policy->policy = policy;
	raw_spin_lock_init(&xg_policy->update_lock);
	raw_spin_lock_init(&xg_policy->timer_lock);
	return xg_policy;
}

static void xrgov_policy_free(struct xrgov_policy *xg_policy)
{
	kfree(xg_policy);
}

static int xrgov_kthread_create(void)
{
	struct cpumask bind_cpus;
	struct task_struct *thread;
	struct sched_param param = { .sched_priority = MAX_RT_PRIO / 2 };
	int cluster, cpu, ret;

	kthread_init_worker(&xres_worker);
	thread = kthread_create(kthread_worker_fn, &xres_worker,
				"xrgov");

	if (IS_ERR(thread)) {
		pr_err("failed to create sugov thread: %ld\n", PTR_ERR(thread));
		return PTR_ERR(thread);
	}

	ret = sched_setscheduler_nocheck(thread, SCHED_FIFO, &param);
	if (ret) {
		kthread_stop(thread);
		pr_warn("%s: failed to set SCHED_DEADLINE\n", __func__);
		return ret;
	}

	cpumask_clear(&bind_cpus);
	for_each_possible_cpu(cpu) {
		cluster = topology_cluster_id(cpu);
		if (cluster >= 0)
			cpumask_set_cpu(cluster, &bind_cpus);
	}

	if (!cpumask_empty(&bind_cpus))
		kthread_bind_mask(thread, &bind_cpus);

	wake_up_process(thread);

	return 0;
}

static void xrgov_work_stop(struct xrgov_policy *xg_policy)
{
	kthread_flush_work(&xg_policy->work);
	mutex_destroy(&xg_policy->work_lock);
}

static struct xrgov_tunables *xrgov_tunables_alloc(struct xrgov_policy *xg_policy)
{
	struct xrgov_tunables *tunables;

	tunables = kzalloc(sizeof(*tunables), GFP_KERNEL);
	if (tunables)
		gov_attr_set_init(&tunables->attr_set, &xg_policy->tunables_hook);

	return tunables;
}

static void xresgov_tunables_save(struct cpufreq_policy *policy,
		struct xrgov_tunables *tunables)
{
	int cpu;
	struct xrgov_tunables *cached = per_cpu(cached_tunables, policy->cpu);

	if (!cached) {
		cached = kzalloc(sizeof(*tunables), GFP_KERNEL);
		if (!cached)
			return;

		for_each_cpu(cpu, policy->related_cpus)
			per_cpu(cached_tunables, cpu) = cached;
	}

	if (cached) {
		cached->rate_limit_us = tunables->rate_limit_us;
		cached->above_hispeed_delay = tunables->above_hispeed_delay;
		cached->nabove_hispeed_delay = tunables->nabove_hispeed_delay;
		cached->min_sample_time = tunables->min_sample_time;
		cached->nmin_sample_time = tunables->nmin_sample_time;
		cached->hispeed_freq = tunables->hispeed_freq;
		cached->hispeed_load = tunables->hispeed_load;
		cached->boostpulse_duration = tunables->boostpulse_duration;
		cached->boostpulse_min_interval = tunables->boostpulse_min_interval;
		cached->overload_duration = tunables->overload_duration;
		cached->target_loads = tunables->target_loads;
		cached->ntarget_loads = tunables->ntarget_loads;
		cached->rtg_boost_freq = tunables->rtg_boost_freq;
		cached->irq_boost_freq = tunables->irq_boost_freq;
		cached->tt_boost_freq = tunables->tt_boost_freq;
		cached->ed_boost_freq = tunables->ed_boost_freq;
		cached->timer_slack = tunables->timer_slack;
		cached->iowait_boost_step = tunables->iowait_boost_step;
		cached->iowait_lower_limit = tunables->iowait_lower_limit;
		cached->iowait_upper_limit = tunables->iowait_upper_limit;
		cached->fl = tunables->fl;
		cached->pl = tunables->pl;
		cached->nl = tunables->nl;
	}
}

static void xresgov_tunables_restore(struct cpufreq_policy *policy)
{
	struct xrgov_policy *xg_policy = policy->governor_data;
	struct xrgov_tunables *tunables = xg_policy->tunables;
	struct xrgov_tunables *cached = per_cpu(cached_tunables, policy->cpu);

	if (!cached)
		return;

	tunables->rate_limit_us = cached->rate_limit_us;
	tunables->above_hispeed_delay = cached->above_hispeed_delay;
	tunables->nabove_hispeed_delay = cached->nabove_hispeed_delay;
	tunables->min_sample_time = cached->min_sample_time;
	tunables->nmin_sample_time = cached->nmin_sample_time;
	tunables->hispeed_freq = cached->hispeed_freq;
	tunables->hispeed_load = cached->hispeed_load;
	tunables->boostpulse_duration = cached->boostpulse_duration;
	tunables->boostpulse_min_interval = cached->boostpulse_min_interval;
	tunables->overload_duration = cached->overload_duration;
	tunables->target_loads = cached->target_loads;
	tunables->ntarget_loads = cached->ntarget_loads;
	tunables->rtg_boost_freq = cached->rtg_boost_freq;
	tunables->irq_boost_freq = cached->irq_boost_freq;
	tunables->tt_boost_freq = cached->tt_boost_freq;
	tunables->ed_boost_freq = cached->ed_boost_freq;
	tunables->timer_slack = cached->timer_slack;
	tunables->iowait_boost_step = cached->iowait_boost_step;
	tunables->iowait_lower_limit = cached->iowait_lower_limit;
	tunables->iowait_upper_limit = cached->iowait_upper_limit;
	tunables->fl = cached->fl;
	tunables->pl = cached->pl;
	tunables->nl = cached->nl;
}

static int xrgov_init(struct cpufreq_policy *policy)
{
	struct xrgov_policy *xg_policy;
	struct xrgov_tunables *tunables;
	int ret = 0;

	/* State should be equivalent to EXIT */
	if (policy->governor_data)
		return -EBUSY;

	cpufreq_enable_fast_switch(policy);

	xg_policy = xrgov_policy_alloc(policy);
	if (!xg_policy) {
		ret = -ENOMEM;
		goto disable_fast_switch;
	}

	kthread_init_work(&xg_policy->work, xrgov_work);
	init_irq_work(&xg_policy->irq_work, xrgov_irq_work);
	mutex_init(&xg_policy->work_lock);

	tunables = xrgov_tunables_alloc(xg_policy);
	if (!tunables) {
		ret = -ENOMEM;
		goto stop_work;
	}

	tunables->rate_limit_us = cpufreq_policy_transition_delay_us(policy);
	tunables->above_hispeed_delay = default_above_hispeed_delay;
	tunables->nabove_hispeed_delay = ARRAY_SIZE(default_above_hispeed_delay);
	tunables->min_sample_time = default_min_sample_time;
	tunables->nmin_sample_time = ARRAY_SIZE(default_min_sample_time);
	tunables->hispeed_freq = DEFAULT_HISPEED_FREQ;
	tunables->hispeed_load = DEFAULT_HISPEED_LOAD;
	tunables->boostpulse_duration = DEFAULT_BOOSTPULSE_DURATION;
	tunables->boostpulse_min_interval = DEFAULT_MIN_BOOSTPULSE_INTERVAL;
	tunables->overload_duration = DEFAULT_OVERLOAD_DURATION;
	tunables->target_loads = default_target_loads;
	tunables->ntarget_loads = ARRAY_SIZE(default_target_loads);
	tunables->timer_slack = DEFAULT_TIMER_SLACK;
	tunables->iowait_boost_step = IOWAIT_BOOST_INC_STEP;
	tunables->iowait_lower_limit = policy->cpuinfo.min_freq;
	tunables->iowait_upper_limit = policy->cpuinfo.max_freq;
	tunables->rtg_boost_freq = DEFAULT_HISPEED_FREQ;
	tunables->irq_boost_freq = DEFAULT_HISPEED_FREQ;
	tunables->tt_boost_freq = DEFAULT_HISPEED_FREQ;
	tunables->ed_boost_freq = policy->cpuinfo.max_freq;
	tunables->fl = false;
	tunables->pl = false;
	tunables->nl = true;

	spin_lock_init(&tunables->target_loads_lock);
	spin_lock_init(&tunables->above_hispeed_delay_lock);
	spin_lock_init(&tunables->min_sample_time_lock);

	policy->governor_data = xg_policy;
	xg_policy->tunables = tunables;
	xresgov_tunables_restore(policy);

	ret = kobject_init_and_add(&tunables->attr_set.kobj, &xrgov_tunables_ktype,
				   get_governor_parent_kobj(policy), "%s",
				   xres_gov.name);
	if (ret)
		goto fail;

	return 0;

fail:
	kobject_put(&tunables->attr_set.kobj);
	policy->governor_data = NULL;

stop_work:
	xrgov_work_stop(xg_policy);
	xrgov_policy_free(xg_policy);

disable_fast_switch:
	cpufreq_disable_fast_switch(policy);

	pr_err("initialization failed (error %d)\n", ret);
	return ret;
}

static void xrgov_exit(struct cpufreq_policy *policy)
{
	struct xrgov_policy *xg_policy = policy->governor_data;
	struct xrgov_tunables *tunables = xg_policy->tunables;

	xresgov_tunables_save(policy, tunables);

	gov_attr_set_put(&tunables->attr_set, &xg_policy->tunables_hook);
	policy->governor_data = NULL;
	xrgov_work_stop(xg_policy);
	xrgov_policy_free(xg_policy);
	cpufreq_disable_fast_switch(policy);
}

void xrgov_nop_timer(struct timer_list *unused)
{

}

static int xrgov_start(struct cpufreq_policy *policy)
{
	struct xrgov_policy *xg_policy = policy->governor_data;
	unsigned int cpu = -1;

	xg_policy->fast_ramp = false;
	xg_policy->freq_update_delay_ns	=
			xg_policy->tunables->rate_limit_us * NSEC_PER_USEC;
	xg_policy->overload_duration_ns =
			xg_policy->tunables->overload_duration * NSEC_PER_MSEC;
	xg_policy->next_freq = policy->cur;
	xg_policy->cached_raw_freq = policy->cur;
	xg_policy->work_in_progress = false;
	xg_policy->limits_changed = false;
	xg_policy->need_freq_update = false;

	for_each_cpu(cpu, policy->cpus) {
		struct xrgov_cpu *xg_cpu = &per_cpu(xrgov_cpu, cpu);

		memset(xg_cpu, 0, sizeof(*xg_cpu));
		xg_cpu->xg_policy		= xg_policy;
		xg_cpu->cpu			= cpu;
		xg_cpu->iowait_boost_pending	= false;
		xg_cpu->last_idle_sum_time	= get_cpu_idle_time(xg_cpu->cpu, NULL, 0);
		xg_cpu->last_idle_update_time = walt_sched_clock();
	}

	for_each_cpu(cpu, policy->cpus) {
		struct xrgov_cpu *xg_cpu = &per_cpu(xrgov_cpu, cpu);

		xresgov_add_callback(cpu, &xg_cpu->cb, xrgov_update_freq);
	}

	raw_spin_lock(&xg_policy->timer_lock);
	timer_setup(&xg_policy->pol_slack_timer, xrgov_nop_timer, 0);
	add_timer_on(&xg_policy->pol_slack_timer, policy->cpu);
	xg_policy->governor_enabled = true;
	raw_spin_unlock(&xg_policy->timer_lock);

	return 0;
}

static void xrgov_stop(struct cpufreq_policy *policy)
{
	struct xrgov_policy *xg_policy = policy->governor_data;
	unsigned int cpu;

	raw_spin_lock(&xg_policy->timer_lock);
	xg_policy->governor_enabled = false;
	del_timer_sync(&xg_policy->pol_slack_timer);
	raw_spin_unlock(&xg_policy->timer_lock);

	for_each_cpu(cpu, policy->cpus)
		xresgov_remove_callback(cpu);

	synchronize_rcu();

	irq_work_sync(&xg_policy->irq_work);
	kthread_cancel_work_sync(&xg_policy->work);
}

static void xrgov_limits(struct cpufreq_policy *policy)
{
	struct xrgov_policy *xg_policy = policy->governor_data;

	if (!policy->fast_switch_enabled) {
		mutex_lock(&xg_policy->work_lock);
		cpufreq_policy_apply_limits(policy);
		mutex_unlock(&xg_policy->work_lock);
	} else {
		cpufreq_driver_fast_switch(policy, policy->cur);
	}

	xg_policy->limits_changed = true;
}

struct cpufreq_governor xres_gov = {
	.name			= "xres",
	.owner			= THIS_MODULE,
	.flags			= CPUFREQ_GOV_DYNAMIC_SWITCHING,
	.init			= xrgov_init,
	.exit			= xrgov_exit,
	.start			= xrgov_start,
	.stop			= xrgov_stop,
	.limits			= xrgov_limits,
};

static int __init xres_init(void)
{
	int ret;

	ret = xrgov_kthread_create();
	if (ret) {
		pr_err("worker thread create failed, %d\n", ret);
		return ret;
	}

	ret = cpufreq_register_governor(&xres_gov);
	if (ret) {
		pr_err("gov register failed, %d\n", ret);
		return ret;
	}

	return 0;
}
module_init(xres_init);

MODULE_AUTHOR("Xring Cpu&Sched");
MODULE_DESCRIPTION("cpufreq XRing Energy Scaling governor");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: sched-walt");
