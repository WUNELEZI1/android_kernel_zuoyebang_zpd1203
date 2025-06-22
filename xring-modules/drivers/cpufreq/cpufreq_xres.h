/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#ifndef _CPUFREQ_XRES_H
#define _CPUFREQ_XRES_H
#include "../cpu_sched/walt/walt.h"
#include <linux/sched/cpufreq.h>
#include <uapi/linux/sched/types.h>
#include <linux/sched/clock.h>


struct xrgov_tunables {
	struct gov_attr_set	attr_set;
	unsigned int		rate_limit_us;

	spinlock_t	above_hispeed_delay_lock;
	unsigned int	*above_hispeed_delay;
	int		nabove_hispeed_delay;

	spinlock_t	min_sample_time_lock;
	unsigned int	*min_sample_time;
	int		nmin_sample_time;

	unsigned int	hispeed_freq;
	unsigned int	hispeed_load;

	/* Duration of a boot pulse in usecs */
	unsigned int	boostpulse_duration;
	unsigned int	boostpulse_min_interval;

	unsigned int	overload_duration;

	spinlock_t	target_loads_lock;
	unsigned int	*target_loads;
	int		ntarget_loads;

	unsigned int	timer_slack;

	unsigned int	adaptive_low_freq;
	unsigned int	adaptive_high_freq;

	bool		io_is_busy;

	unsigned int	iowait_boost_step;
	unsigned int	iowait_upper_limit;
	unsigned int	iowait_lower_limit;

	unsigned int	rtg_boost_freq;
	unsigned int	irq_boost_freq;
	unsigned int	ed_boost_freq;
	unsigned int	tt_boost_freq;

	bool	fl;
	bool	pl;
	bool	nl;

};

struct xrgov_policy {
	struct cpufreq_policy	*policy;
	struct xrgov_tunables	*tunables;
	struct list_head	tunables_hook;


	raw_spinlock_t		update_lock;
	bool	governor_enabled;
	/* kthread for fast_switch flag */
	bool	fast_ramp;
	bool	pending_update;

	/* timestamp */
	u64	update_time;
	u64	freq_update_delay_ns;
	u64	last_freq_update_time;
	u64	overload_duration_ns;
	u64	hispeed_validate_time_ns;
	u64	floor_validate_time_ns;
	u64	boostpulse_endtime_us;
	u64	rtg_boost_time;
	u64	last_work_time_ns;

	/* protect slack timer */
	raw_spinlock_t timer_lock;
	/* policy slack timer */
	struct timer_list pol_slack_timer;

	unsigned int	next_freq;
	unsigned int	cached_raw_freq;
	unsigned int	driving_cpu;

	/* The next fields are only needed if fast switch cannot be used: */
	struct		irq_work irq_work;
	struct		kthread_work work;
	struct		mutex work_lock;

	bool		work_in_progress;
	bool		limits_changed;
	bool		need_freq_update;
	bool		rtg_freq_lock;
	bool		rtg_freq_switch;
};

struct xrgov_cpu {
	struct xrgov_policy	*xg_policy;
	struct xresgov_callback	cb;
	struct walt_cpu_load	walt_load;
	unsigned int		cpu;
	bool			iowait_boost_pending;
	unsigned long		iowait_boost;
	unsigned int		iowait_boost_freq;
	u64			last_iowait;

	unsigned int		flags;
	u64			last_idle_sum_time;
	u64			last_idle_update_time;
	unsigned long		util;
	unsigned long		cache_raw_util;
	unsigned long		max;
	unsigned int		reason;
};

/* defaut value for xrgov_tunables */
#define DEFAULT_OVERLOAD_DURATION	(250)

/* Target load.  Lower values result in higher CPU speeds. */
#define DEFAULT_TARGET_LOAD	90
#define DEFAULT_RATE_LIMIT_US	(8 * USEC_PER_MSEC)

#define DEFAULT_HISPEED_LOAD	80
#define DEFAULT_HISPEED_FREQ	1800000 /* 1800MHz */

#define DEFAULT_BOOSTPULSE_DURATION	(120 * USEC_PER_MSEC)
#define DEFAULT_MIN_BOOSTPULSE_INTERVAL	(500 * USEC_PER_MSEC)
#define DEFAULT_TIMER_SLACK		(10 * USEC_PER_MSEC)

#define IOWAIT_BOOST_INC_STEP	200000 /* 200MHz */
#define IOWAIT_BOOST_STEP_MAX	500000 /* 500MHz*/
#define IOWAIT_BOOST_CLEAR_NS	20000000 /* 20ms */

#define RTG_LOCK_LAG_TIME_NS	16000000 /* 16ms */
#define WORK_SPAN_LIMIT_NS	2000000 /* 2ms */

#define RESERVED_FLAGS		WALT_IRQ_BOOST

#endif
