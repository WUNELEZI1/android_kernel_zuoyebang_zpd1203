// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/kmemleak.h>

#include <trace/hooks/sched.h>

#include "walt.h"
#include "trace.h"

static int neg_four = -4;
static int four = 4;
static unsigned int one_hundred_thousand = 100000;
static unsigned int two_hundred_million = 200000000;
static int __maybe_unused two = 2;
static int one_hundred = 100;
static int one_thousand = 1000;
static int one_thousand_twenty_four = 1024;
static int two_thousand = 2000;

/*
 * CFS task prio range is [100 ... 139]
 * 120 is the default prio.
 * RTG boost range is [100 ... 119] because giving
 * boost for [120 .. 139] does not make sense.
 * 99 means disabled and it is the default value.
 */
static unsigned int min_cfs_boost_prio = 99;
static unsigned int max_cfs_boost_prio = 119;

unsigned int sysctl_sched_capacity_margin_up_pct[MAX_MARGIN_LEVELS];
unsigned int sysctl_sched_capacity_margin_dn_pct[MAX_MARGIN_LEVELS];
unsigned int sysctl_sched_cpu_capacity_margin_pct[MAX_MARGIN_LEVELS];
unsigned int sysctl_sched_boost;

unsigned int sysctl_input_boost_ms;
unsigned int sysctl_input_boost_freq[WALT_NR_CPUS];
unsigned int sysctl_sched_boost_on_input;

unsigned int sysctl_powerkey_boost_ms;
unsigned int sysctl_powerkey_boost_freq[WALT_NR_CPUS];
unsigned int sysctl_sched_boost_on_powerkey;

unsigned int sysctl_volkey_boost_ms;
unsigned int sysctl_volkey_boost_freq[WALT_NR_CPUS];
unsigned int sysctl_sched_boost_on_volkey;

unsigned int sysctl_sched_early_up[MAX_MARGIN_LEVELS];
unsigned int sysctl_sched_early_down[MAX_MARGIN_LEVELS];
unsigned int sysctl_sched_concurrency_thres;
unsigned int sysctl_sched_spread_affinity_enabled;
unsigned int sysctl_sched_spread_affinity_restrict_cpumask;
unsigned int sysctl_sched_spread_affinity_assist_cpumask;

/* sysctl nodes accessed by other files */
unsigned int __read_mostly sysctl_sched_coloc_downmigrate_ns;
unsigned int __read_mostly sysctl_sched_group_downmigrate_pct;
unsigned int __read_mostly sysctl_sched_group_upmigrate_pct;
unsigned int __read_mostly sysctl_sched_window_stats_policy;
unsigned int sysctl_sched_ravg_window_nr_ticks;
unsigned int sysctl_sched_walt_rotate_big_tasks;
unsigned int sysctl_sched_task_unfilter_period;
unsigned int sysctl_sched_global_boost_min_uclamp;
unsigned int sysctl_sched_min_task_util_for_boost = 51;
unsigned int sysctl_sched_min_task_util_for_uclamp = 51;
unsigned int sysctl_sched_min_task_util_for_colocation = 35;
unsigned int sysctl_sched_many_wakeup_threshold = WALT_MANY_WAKEUP_DEFAULT;
const int sched_user_hint_max = 1000;
unsigned int sysctl_walt_rtg_cfs_boost_prio = 99; /* disabled by default */
unsigned int sysctl_sched_sync_hint_enable = 1;
unsigned int sysctl_panic_on_walt_bug;
unsigned int sysctl_sched_hyst_min_coloc_ns = 80000000;
unsigned int sysctl_sched_asymcap_boost;
unsigned int sysctl_sched_long_running_rt_task_ms;
unsigned int sysctl_sched_idle_enough;
unsigned int sysctl_sched_cluster_util_thres_pct;
#if IS_ENABLED(CONFIG_XRING_CORECTL)
unsigned int sysctl_max_freq_partial_halt[MAX_CLUSTERS];
#else
unsigned int sysctl_max_freq_partial_halt = FREQ_QOS_MAX_DEFAULT_VALUE;
#endif
unsigned int sysctl_fmax_cap[MAX_CLUSTERS];
unsigned int sysctl_sched_sbt_pause_cpus;
unsigned int sysctl_sched_sbt_delay_windows;
unsigned int high_perf_cluster_freq_cap[MAX_CLUSTERS];
unsigned int __read_mostly sysctl_xr_yield_invalid_time_ns = 10000;
/* 1~1000, 1000 is off, the max value = NUM_LOAD_INDICES */
unsigned int __read_mostly sysctl_xr_top_task_boost = 360U;
/* 1~1024, 1024 is off, the max value = XR_WALT_TOP_TASK_FRAC */
unsigned int __read_mostly sysctl_xr_top_task_filter_coef = 1024U;
/* Early Detected Task */
unsigned int __read_mostly sysctl_ed_run_res[MAX_CLUSTERS];
unsigned int __read_mostly sysctl_ed_wait_res[MAX_CLUSTERS];
unsigned int __read_mostly sysctl_ed_new_res[MAX_CLUSTERS];
unsigned int __read_mostly sysctl_ed_task_enabled = 1;

#if IS_ENABLED(CONFIG_XRING_TASK_PP)
unsigned int sysctl_task_pp_idle_thres = 10000000;
#endif

//MIUI ADD: Performance_BoostFramework
unsigned int sysctl_sched_prefer_cluster_boost_thres = 1024;
//END Performance_BoostFramework

/* range is [1 .. INT_MAX] */
static int sysctl_task_read_pid = 1;

/* Early Detected Task Handler */
static int walt_proc_ed_thresholds_handler(struct ctl_table *table, int write,
					   void __user *buffer, size_t *lenp,
					   loff_t *ppos)
{
	int ret = 0;
	int cpu;
	int i;
	struct walt_rq *wrq;
	static DEFINE_MUTEX(mutex);

	mutex_lock(&mutex);

	if (table->maxlen > (sizeof(unsigned int) * num_sched_clusters))
		table->maxlen = sizeof(unsigned int) * num_sched_clusters;

	ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);

	if (!ret && write) {
		for (i = 0; i < num_sched_clusters; ++i) {
			for_each_cpu(cpu, &sched_cluster[i]->cpus) {
				wrq = &per_cpu(walt_rq, cpu);
				wrq->ed_run_thres = sysctl_ed_run_res[i];
				wrq->ed_wait_thres = sysctl_ed_wait_res[i];
				wrq->ed_first_run_thres = sysctl_ed_run_res[i];
			}
		}
	}

	mutex_unlock(&mutex);
	return ret;
}

static int walt_proc_group_thresholds_handler(struct ctl_table *table, int write,
				       void __user *buffer, size_t *lenp,
				       loff_t *ppos)
{
	int ret;
	static DEFINE_MUTEX(mutex);
	struct rq *rq = cpu_rq(cpumask_first(cpu_possible_mask));
	unsigned long flags;

	if (unlikely(num_sched_clusters <= 0))
		return -EPERM;

	mutex_lock(&mutex);
	ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
	if (ret || !write) {
		mutex_unlock(&mutex);
		return ret;
	}

	/*
	 * The load scale factor update happens with all
	 * rqs locked. so acquiring 1 CPU rq lock and
	 * updating the thresholds is sufficient for
	 * an atomic update.
	 */
	raw_spin_lock_irqsave(&rq->__lock, flags);
	walt_update_group_thresholds();
	raw_spin_unlock_irqrestore(&rq->__lock, flags);

	mutex_unlock(&mutex);

	return ret;
}

static int walt_proc_user_hint_handler(struct ctl_table *table,
				int write, void __user *buffer, size_t *lenp,
				loff_t *ppos)
{
	int ret;
	unsigned int old_value;
	static DEFINE_MUTEX(mutex);

	mutex_lock(&mutex);

	old_value = sysctl_sched_user_hint;
	ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
	if (ret || !write || (old_value == sysctl_sched_user_hint))
		goto unlock;

	sched_user_hint_reset_time = jiffies + HZ;
	walt_irq_work_queue(&walt_migration_irq_work);

unlock:
	mutex_unlock(&mutex);
	return ret;
}

DECLARE_BITMAP(sysctl_bitmap, WALT_NR_CPUS);

static int walt_proc_sbt_pause_handler(struct ctl_table *table,
				       int write, void __user *buffer, size_t *lenp,
				       loff_t *ppos)
{
	int ret = 0;
	unsigned int old_value;
	unsigned long bitmask;
	const unsigned long *bitmaskp = &bitmask;
	static bool written_once;
	static DEFINE_MUTEX(mutex);

	mutex_lock(&mutex);

	old_value = sysctl_sched_sbt_pause_cpus;
	ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
	if (ret || !write || (old_value == sysctl_sched_sbt_pause_cpus))
		goto unlock;

	if (written_once) {
		sysctl_sched_sbt_pause_cpus = old_value;
		goto unlock;
	}

	bitmask = (unsigned long)sysctl_sched_sbt_pause_cpus;
	bitmap_copy(sysctl_bitmap, bitmaskp, WALT_NR_CPUS);
	cpumask_copy(&cpus_for_sbt_pause, to_cpumask(sysctl_bitmap));

	written_once = true;
unlock:
	mutex_unlock(&mutex);
	return ret;
}

static int sched_ravg_window_handler(struct ctl_table *table,
				int write, void __user *buffer, size_t *lenp,
				loff_t *ppos)
{
	int ret = -EPERM;
	static DEFINE_MUTEX(mutex);
	int val;

	struct ctl_table tmp = {
		.data	= &val,
		.maxlen	= sizeof(val),
		.mode	= table->mode,
	};

	mutex_lock(&mutex);

	if (write && HZ != 250)
		goto unlock;

	val = sysctl_sched_ravg_window_nr_ticks;
	ret = proc_dointvec(&tmp, write, buffer, lenp, ppos);
	if (ret || !write || (val == sysctl_sched_ravg_window_nr_ticks))
		goto unlock;

	if (val != 2 && val != 3 && val != 4 && val != 5 && val != 8) {
		ret = -EINVAL;
		goto unlock;
	}

	sysctl_sched_ravg_window_nr_ticks = val;
	sched_window_nr_ticks_change();

unlock:
	mutex_unlock(&mutex);
	return ret;
}

static DEFINE_MUTEX(sysctl_pid_mutex);
static int sched_task_read_pid_handler(struct ctl_table *table, int write,
				       void __user *buffer, size_t *lenp,
				       loff_t *ppos)
{
	int ret;

	mutex_lock(&sysctl_pid_mutex);
	ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
	mutex_unlock(&sysctl_pid_mutex);

	return ret;
}

enum {
	TASK_BEGIN = 0,
	WAKE_UP_IDLE,
	INIT_TASK_LOAD,
	GROUP_ID,
	PER_TASK_BOOST,
	PER_TASK_BOOST_PERIOD_MS,
	LOAD_BOOST,
//MIUI ADD: Performance_BoostFramework
	LEADER_TASK_PREFER_CLUSTER,
//END Performance_BoostFramework
};

static int sched_task_handler(struct ctl_table *table, int write,
				void __user *buffer, size_t *lenp,
				loff_t *ppos)
{
	int ret, param;
	struct task_struct *task;
	int pid_and_val[2] = {-1, -1};
	int val;
	struct walt_task_struct *wts;
	// MIUI ADD: Performance_BoostFramework
	struct walt_task_struct *wts_gleader;
	// END Performance_BoostFramework

	struct ctl_table tmp = {
		.data	= &pid_and_val,
		.maxlen	= sizeof(pid_and_val),
		.mode	= table->mode,
	};

	mutex_lock(&sysctl_pid_mutex);

	if (!write) {
		task = get_pid_task(find_vpid(sysctl_task_read_pid),
				PIDTYPE_PID);
		if (!task) {
			ret = -ENOENT;
			goto unlock_mutex;
		}
		wts = (struct walt_task_struct *) task->android_vendor_data1;
		pid_and_val[0] = sysctl_task_read_pid;
		param = (unsigned long)table->data;
		switch (param) {
		case WAKE_UP_IDLE:
			pid_and_val[1] = wts->wake_up_idle;
			break;
		case INIT_TASK_LOAD:
			pid_and_val[1] = wts->init_load_pct;
			break;
		case GROUP_ID:
			pid_and_val[1] = sched_get_group_id(task);
			break;
		case PER_TASK_BOOST:
			pid_and_val[1] = wts->boost;
			break;
		case PER_TASK_BOOST_PERIOD_MS:
			pid_and_val[1] =
				div64_ul(wts->boost_period,
					 1000000UL);
			break;
		case LOAD_BOOST:
			pid_and_val[1] = wts->load_boost;
			break;
		//MIUI ADD: Performance_BoostFramework
		case LEADER_TASK_PREFER_CLUSTER:
			wts_gleader = (struct walt_task_struct *) task->group_leader->android_vendor_data1;
			pid_and_val[1] = wts_gleader->prefer_cluster_index;
			break;
		//END Performance_BoostFramework
		default:
			ret = -EINVAL;
			goto put_task;
		}
		ret = proc_dointvec(&tmp, write, buffer, lenp, ppos);
		goto put_task;
	}

	ret = proc_dointvec(&tmp, write, buffer, lenp, ppos);
	if (ret)
		goto unlock_mutex;

	if (pid_and_val[0] <= 0) {
		ret = -ENOENT;
		goto unlock_mutex;
	}

	/* parsed the values successfully in pid_and_val[] array */
	task = get_pid_task(find_vpid(pid_and_val[0]), PIDTYPE_PID);
	if (!task) {
		ret = -ENOENT;
		goto unlock_mutex;
	}
	wts = (struct walt_task_struct *)task->android_vendor_data1;
	param = (unsigned long)table->data;
	val = pid_and_val[1];
	if (param != LOAD_BOOST && val < 0) {
		ret = -EINVAL;
		goto put_task;
	}
	switch (param) {
	case WAKE_UP_IDLE:
		wts->wake_up_idle = val;
		break;
	case INIT_TASK_LOAD:
		if (pid_and_val[1] < 0 || pid_and_val[1] > 100) {
			ret = -EINVAL;
			goto put_task;
		}
		wts->init_load_pct = val;
		break;
	case GROUP_ID:
		ret = sched_set_group_id(task, val);
		break;
	case PER_TASK_BOOST:
		if (val < TASK_BOOST_NONE || val >= TASK_BOOST_END) {
			ret = -EINVAL;
			goto put_task;
		}
		wts->boost = val;
		if (val == 0)
			wts->boost_period = 0;
		break;
	case PER_TASK_BOOST_PERIOD_MS:
		if (wts->boost == 0 && val) {
			/* setting boost period w/o boost is invalid */
			ret = -EINVAL;
			goto put_task;
		}
		wts->boost_period = (u64)val * 1000 * 1000;
		wts->boost_expires = sched_clock() + wts->boost_period;
		break;
	case LOAD_BOOST:
		if (pid_and_val[1] < -90 || pid_and_val[1] > 90) {
			ret = -EINVAL;
			goto put_task;
		}
		wts->load_boost = val;
		if (val)
			wts->boosted_task_load = mult_frac((int64_t)SCHED_CAPACITY_SCALE,
					(int64_t)val, 100);
		else
			wts->boosted_task_load = 0;
		break;
	// MIUI ADD: Performance_BoostFramework
	case LEADER_TASK_PREFER_CLUSTER:
		wts_gleader = (struct walt_task_struct *)task->group_leader->android_vendor_data1;
		if (val >= 0) {
			int end_index = val & 0xF;
			int order_index = val >> 4;

			if (order_index < num_sched_clusters && end_index < num_sched_clusters)
				wts_gleader->prefer_cluster_index = val;
		}
		break;
	// END Performance_BoostFramework
	default:
		ret = -EINVAL;
	}

	trace_sched_task_handler(task, param, val, CALLER_ADDR0, CALLER_ADDR1,
				CALLER_ADDR2, CALLER_ADDR3, CALLER_ADDR4, CALLER_ADDR5);
put_task:
	put_task_struct(task);
unlock_mutex:
	mutex_unlock(&sysctl_pid_mutex);

	return ret;
}

#ifdef CONFIG_PROC_SYSCTL
static void sched_update_updown_migrate_values(bool up)
{
	int i = 0, cpu = 0;
	struct walt_sched_cluster *cluster = NULL;

	for_each_sched_cluster(cluster) {
		/*
		 * No need to worry about CPUs in last cluster
		 * if there are more than 2 clusters in the system
		 */
		for_each_cpu(cpu, &cluster->cpus) {
			if (up)
				sched_capacity_margin_up[cpu] =
				SCHED_FIXEDPOINT_SCALE * 100 /
				sysctl_sched_capacity_margin_up_pct[i];
			else
				sched_capacity_margin_down[cpu] =
				SCHED_FIXEDPOINT_SCALE * 100 /
				sysctl_sched_capacity_margin_dn_pct[i];
		}

		trace_sched_update_updown_migrate_values(up, i);
		if (++i >= num_sched_clusters - 1)
			break;
	}
}

int sched_updown_migrate_handler(struct ctl_table *table, int write,
				void __user *buffer, size_t *lenp,
				loff_t *ppos)
{
	int ret, i;
	unsigned int *data = (unsigned int *)table->data;
	static DEFINE_MUTEX(mutex);
	int cap_margin_levels = num_sched_clusters ? num_sched_clusters - 1 : 0;
	int val[MAX_MARGIN_LEVELS];
	struct ctl_table tmp = {
		.data	= &val,
		.maxlen	= sizeof(int) * cap_margin_levels,
		.mode	= table->mode,
	};

	if (cap_margin_levels <= 0)
		return -EINVAL;

	mutex_lock(&mutex);

	if (!write) {
		ret = proc_dointvec(table, write, buffer, lenp, ppos);
		goto unlock_mutex;
	}

	ret = proc_dointvec(&tmp, write, buffer, lenp, ppos);
	if (ret)
		goto unlock_mutex;

	/* check if valid pct values are passed in */
	for (i = 0; i < cap_margin_levels; i++) {
		if (val[i] <= 0 || val[i] > 100) {
			ret = -EINVAL;
			goto unlock_mutex;
		}
	}

	/* check up pct is greater than dn pct */
	if (data == &sysctl_sched_capacity_margin_up_pct[0]) {
		for (i = 0; i < cap_margin_levels; i++) {
			if (val[i] < sysctl_sched_capacity_margin_dn_pct[i]) {
				ret = -EINVAL;
				goto unlock_mutex;
			}
		}
	} else {
		for (i = 0; i < cap_margin_levels; i++) {
			if (sysctl_sched_capacity_margin_up_pct[i] < val[i]) {
				ret = -EINVAL;
				goto unlock_mutex;
			}
		}
	}

	/* all things checkout update the value */
	for (i = 0; i < cap_margin_levels; i++)
		data[i] = val[i];

	/* update individual cpu thresholds */
	sched_update_updown_migrate_values(data == &sysctl_sched_capacity_margin_up_pct[0]);

unlock_mutex:
	mutex_unlock(&mutex);

	return ret;
}

static void sched_update_cpu_migrate_values(void)
{
	int i = 0, cpu = 0;
	struct walt_sched_cluster *cluster = NULL;

	for_each_sched_cluster(cluster) {
		/*
		 * No need to worry about CPUs in last cluster
		 * if there are more than 2 clusters in the system
		 */
		for_each_cpu(cpu, &cluster->cpus)
			sched_cpu_capacity_margin[cpu] =
			SCHED_FIXEDPOINT_SCALE * 100 /
			sysctl_sched_cpu_capacity_margin_pct[i];

		trace_sched_update_cpu_migrate_values(i);
		if (++i >= num_sched_clusters - 1)
			break;
	}
}

int sched_cpu_migrate_handler(struct ctl_table *table, int write,
				void __user *buffer, size_t *lenp,
				loff_t *ppos)
{
	int ret, i;
	unsigned int *data = (unsigned int *)table->data;
	static DEFINE_MUTEX(mutex);
	int cap_margin_levels = num_sched_clusters ? num_sched_clusters - 1 : 0;
	int val[MAX_MARGIN_LEVELS];
	struct ctl_table tmp = {
		.data	= &val,
		.maxlen	= sizeof(int) * cap_margin_levels,
		.mode	= table->mode,
	};

	if (cap_margin_levels <= 0)
		return -EINVAL;

	mutex_lock(&mutex);

	if (!write) {
		ret = proc_dointvec(table, write, buffer, lenp, ppos);
		goto unlock_mutex;
	}

	ret = proc_dointvec(&tmp, write, buffer, lenp, ppos);
	if (ret)
		goto unlock_mutex;

	/* check if valid pct values are passed in */
	for (i = 0; i < cap_margin_levels; i++) {
		if (val[i] <= 0 || val[i] > 100) {
			ret = -EINVAL;
			goto unlock_mutex;
		}
	}

	/* all things checkout update the value */
	for (i = 0; i < cap_margin_levels; i++)
		data[i] = val[i];

	/* update individual cpu thresholds */
	sched_update_cpu_migrate_values();

unlock_mutex:
	mutex_unlock(&mutex);

	return ret;
}

static void sched_update_updown_early_migrate_values(bool up)
{
	int i = 0, cpu = 0;
	struct walt_sched_cluster *cluster = NULL;

	for_each_sched_cluster(cluster) {
		/*
		 * No need to worry about CPUs in last cluster
		 * if there are more than 2 clusters in the system
		 */
		for_each_cpu(cpu, &cluster->cpus) {
			if (up)
				sched_capacity_margin_early_up[cpu] = sysctl_sched_early_up[i];
			else
				sched_capacity_margin_early_down[cpu] = sysctl_sched_early_down[i];
		}

		trace_sched_update_updown_early_migrate_values(up, i);
		if (++i >= num_sched_clusters - 1)
			break;
	}
}

int sched_updown_early_migrate_handler(struct ctl_table *table, int write,
				void __user *buffer, size_t *lenp,
				loff_t *ppos)
{
	int ret, i;
	unsigned int *data = (unsigned int *)table->data;
	static DEFINE_MUTEX(mutex);
	int cap_margin_levels = num_sched_clusters ? num_sched_clusters - 1 : 0;
	int val[MAX_MARGIN_LEVELS];
	struct ctl_table tmp = {
		.data	= &val,
		.maxlen	= sizeof(int) * cap_margin_levels,
		.mode	= table->mode,
	};

	if (cap_margin_levels <= 0)
		return -EINVAL;

	mutex_lock(&mutex);

	if (!write) {
		ret = proc_dointvec(table, write, buffer, lenp, ppos);
		goto unlock_mutex;
	}

	ret = proc_dointvec(&tmp, write, buffer, lenp, ppos);
	if (ret)
		goto unlock_mutex;

	for (i = 0; i < cap_margin_levels; i++) {
		if (val[i] < SCHED_CAPACITY_SCALE) {
			ret = -EINVAL;
			goto unlock_mutex;
		}
	}

	/* check up thresh is greater than dn thresh */
	if (data == &sysctl_sched_early_up[0]) {
		for (i = 0; i < cap_margin_levels; i++) {
			if (val[i] >= sysctl_sched_early_down[i]) {
				ret = -EINVAL;
				goto unlock_mutex;
			}
		}
	} else {
		for (i = 0; i < cap_margin_levels; i++) {
			if (sysctl_sched_early_up[i] >= val[i]) {
				ret = -EINVAL;
				goto unlock_mutex;
			}
		}
	}

	/* all things checkout update the value */
	for (i = 0; i < cap_margin_levels; i++)
		data[i] = val[i];

	/* update individual cpu thresholds */
	sched_update_updown_early_migrate_values(data == &sysctl_sched_early_up[0]);
unlock_mutex:
	mutex_unlock(&mutex);

	return ret;
}

int sched_fmax_cap_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp,
		loff_t *ppos)
{
	int ret, i;
	unsigned int *data = (unsigned int *)table->data;
	static DEFINE_MUTEX(mutex);
	int cap_margin_levels = num_sched_clusters;
	int val[MAX_CLUSTERS];
	struct ctl_table tmp = {
		.data	= &val,
		.maxlen	= sizeof(int) * cap_margin_levels,
		.mode	= table->mode,
	};

	if (cap_margin_levels <= 0)
		return -EINVAL;

	mutex_lock(&mutex);

	if (!write) {
		ret = proc_dointvec(table, write, buffer, lenp, ppos);
		goto unlock_mutex;
	}

	ret = proc_dointvec(&tmp, write, buffer, lenp, ppos);
	if (ret)
		goto unlock_mutex;

	for (i = 0; i < cap_margin_levels; i++) {
		if (val[i] < 0) {
			ret = -EINVAL;
			goto unlock_mutex;
		}
		data[i] = val[i];
	}
unlock_mutex:
	mutex_unlock(&mutex);

	return ret;
}

#endif /* CONFIG_PROC_SYSCTL */

static struct ctl_table input_boost_sysctls[] = {
	{
		.procname	= "input_boost_ms",
		.data		= &sysctl_input_boost_ms,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &one_hundred_thousand,
	},
	{
		.procname	= "input_boost_freq",
		.data		= &sysctl_input_boost_freq,
		.maxlen		= sizeof(unsigned int) * WALT_NR_CPUS,
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_boost_on_input",
		.data		= &sysctl_sched_boost_on_input,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "powerkey_boost_ms",
		.data		= &sysctl_powerkey_boost_ms,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &one_hundred_thousand,
	},
	{
		.procname	= "powerkey_boost_freq",
		.data		= &sysctl_powerkey_boost_freq,
		.maxlen		= sizeof(unsigned int) * WALT_NR_CPUS,
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_boost_on_powerkey",
		.data		= &sysctl_sched_boost_on_powerkey,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "volkey_boost_ms",
		.data		= &sysctl_volkey_boost_ms,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &one_hundred_thousand,
	},
	{
		.procname	= "volkey_boost_freq",
		.data		= &sysctl_volkey_boost_freq,
		.maxlen		= sizeof(unsigned int) * WALT_NR_CPUS,
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_boost_on_volkey",
		.data		= &sysctl_sched_boost_on_volkey,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{ }
};

static struct ctl_table walt_table[] = {
	{
		.procname	= "sched_user_hint",
		.data		= &sysctl_sched_user_hint,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= walt_proc_user_hint_handler,
		.extra1		= SYSCTL_ZERO,
		.extra2		= (void *)&sched_user_hint_max,
	},
	{
		.procname	= "sched_window_stats_policy",
		.data		= &sysctl_sched_window_stats_policy,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &four,
	},
	{
		.procname	= "sched_group_upmigrate",
		.data		= &sysctl_sched_group_upmigrate_pct,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0664,
		.proc_handler	= walt_proc_group_thresholds_handler,
		.extra1		= &sysctl_sched_group_downmigrate_pct,
	},
	{
		.procname	= "sched_group_downmigrate",
		.data		= &sysctl_sched_group_downmigrate_pct,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0664,
		.proc_handler	= walt_proc_group_thresholds_handler,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &sysctl_sched_group_upmigrate_pct,
	},
	{
		.procname	= "sched_boost",
		.data		= &sysctl_sched_boost,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= sched_boost_handler,
		.extra1		= &neg_four,
		.extra2		= &four,
	},
	{
		.procname	= "sched_many_wakeup_threshold",
		.data		= &sysctl_sched_many_wakeup_threshold,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &two,
		.extra2		= &one_thousand,
	},
	{
		.procname	= "sched_walt_rotate_big_tasks",
		.data		= &sysctl_sched_walt_rotate_big_tasks,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_ONE,
	},
	{
		.procname	= "sched_spread_affinity_enabled",
		.data		= &sysctl_sched_spread_affinity_enabled,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_ONE,
	},
	{
		.procname	= "sched_spread_affinity_restrict_cpumask",
		.data		= &sysctl_sched_spread_affinity_restrict_cpumask,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_spread_affinity_assist_cpumask",
		.data		= &sysctl_sched_spread_affinity_assist_cpumask,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_global_boost_min_uclamp",
		.data		= &sysctl_sched_global_boost_min_uclamp,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &one_thousand_twenty_four,
	},
	{
		.procname	= "sched_min_task_util_for_boost",
		.data		= &sysctl_sched_min_task_util_for_boost,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &one_thousand,
	},
	{
		.procname	= "sched_min_task_util_for_uclamp",
		.data		= &sysctl_sched_min_task_util_for_uclamp,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &one_thousand,
	},
	{
		.procname	= "sched_min_task_util_for_colocation",
		.data		= &sysctl_sched_min_task_util_for_colocation,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &one_thousand,
	},
	{
		.procname	= "sched_coloc_downmigrate_ns",
		.data		= &sysctl_sched_coloc_downmigrate_ns,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
	},
	{
		.procname	= "sched_task_unfilter_period",
		.data		= &sysctl_sched_task_unfilter_period,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ONE,
		.extra2		= &two_hundred_million,
	},
	{
		.procname	= "sched_ravg_window_nr_ticks",
		.data		= &sysctl_sched_ravg_window_nr_ticks,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= sched_ravg_window_handler,
	},
	{
		.procname	= "sched_upmigrate",
		.data		= &sysctl_sched_capacity_margin_up_pct,
		.maxlen		= sizeof(unsigned int) * MAX_MARGIN_LEVELS,
		.mode		= 0664,
		.proc_handler	= sched_updown_migrate_handler,
	},
	{
		.procname	= "sched_downmigrate",
		.data		= &sysctl_sched_capacity_margin_dn_pct,
		.maxlen		= sizeof(unsigned int) * MAX_MARGIN_LEVELS,
		.mode		= 0664,
		.proc_handler	= sched_updown_migrate_handler,
	},
	{
		.procname	= "sched_cpu_migrate",
		.data		= &sysctl_sched_cpu_capacity_margin_pct,
		.maxlen		= sizeof(unsigned int) * MAX_MARGIN_LEVELS,
		.mode		= 0664,
		.proc_handler	= sched_cpu_migrate_handler,
	},
	{
		.procname	= "sched_early_upmigrate",
		.data		= &sysctl_sched_early_up,
		.maxlen		= sizeof(unsigned int) * MAX_MARGIN_LEVELS,
		.mode		= 0644,
		.proc_handler	= sched_updown_early_migrate_handler,
	},
	{
		.procname	= "sched_early_downmigrate",
		.data		= &sysctl_sched_early_down,
		.maxlen		= sizeof(unsigned int) * MAX_MARGIN_LEVELS,
		.mode		= 0644,
		.proc_handler	= sched_updown_early_migrate_handler,
	},
	{
		.procname	= "walt_rtg_cfs_boost_prio",
		.data		= &sysctl_walt_rtg_cfs_boost_prio,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &min_cfs_boost_prio,
		.extra2		= &max_cfs_boost_prio,
	},
	{
		.procname       = "sched_sync_hint_enable",
		.data           = &sysctl_sched_sync_hint_enable,
		.maxlen         = sizeof(unsigned int),
		.mode           = 0644,
		.proc_handler   = proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_ONE,
	},
	{
		.procname       = "sched_hyst_min_coloc_ns",
		.data           = &sysctl_sched_hyst_min_coloc_ns,
		.maxlen         = sizeof(unsigned int),
		.mode           = 0644,
		.proc_handler   = proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
	},
	{
		.procname       = "panic_on_walt_bug",
		.data           = &sysctl_panic_on_walt_bug,
		.maxlen         = sizeof(unsigned int),
		.mode           = 0644,
		.proc_handler   = proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_wake_up_idle",
		.data		= (int *) WAKE_UP_IDLE,
		.maxlen		= sizeof(unsigned int) * 2,
		.mode		= 0644,
		.proc_handler	= sched_task_handler,
	},
	{
		.procname	= "sched_init_task_load",
		.data		= (int *) INIT_TASK_LOAD,
		.maxlen		= sizeof(unsigned int) * 2,
		.mode		= 0644,
		.proc_handler	= sched_task_handler,
	},
	{
		.procname	= "sched_group_id",
		.data		= (int *) GROUP_ID,
		.maxlen		= sizeof(unsigned int) * 2,
		.mode		= 0644,
		.proc_handler	= sched_task_handler,
	},
	{
		.procname	= "sched_per_task_boost",
		.data		= (int *) PER_TASK_BOOST,
		.maxlen		= sizeof(unsigned int) * 2,
		.mode		= 0644,
		.proc_handler	= sched_task_handler,
	},
	{
		.procname	= "sched_per_task_boost_period_ms",
		.data		= (int *) PER_TASK_BOOST_PERIOD_MS,
		.maxlen		= sizeof(unsigned int) * 2,
		.mode		= 0644,
		.proc_handler	= sched_task_handler,
	},
	{
		.procname	= "task_load_boost",
		.data		= (int *) LOAD_BOOST,
		.maxlen		= sizeof(unsigned int) * 2,
		.mode		= 0644,
		.proc_handler	= sched_task_handler,
	},
	{
		.procname	= "sched_task_read_pid",
		.data		= &sysctl_task_read_pid,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= sched_task_read_pid_handler,
		.extra1		= SYSCTL_ONE,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_enable_tp",
		.data		= &sysctl_sched_dynamic_tp_enable,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= sched_dynamic_tp_handler,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_ONE,
	},
	{
		.procname	= "sched_asymcap_boost",
		.data		= &sysctl_sched_asymcap_boost,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_ONE,
	},
	{
		.procname	= "sched_cluster_util_thres_pct",
		.data		= &sysctl_sched_cluster_util_thres_pct,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_concurrency_thres",
		.data		= &sysctl_sched_concurrency_thres,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &one_hundred,
	},
	{
		.procname	= "sched_idle_enough",
		.data		= &sysctl_sched_idle_enough,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_long_running_rt_task_ms",
		.data		= &sysctl_sched_long_running_rt_task_ms,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= sched_long_running_rt_task_ms_handler,
		.extra1		= SYSCTL_ZERO,
		.extra2		= &two_thousand,
	},
	{
		.procname	= "sched_sbt_pause_cpus",
		.data		= &sysctl_sched_sbt_pause_cpus,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= walt_proc_sbt_pause_handler,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_sbt_delay_windows",
		.data		= &sysctl_sched_sbt_delay_windows,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "sched_max_freq_partial_halt",
		.data		= &sysctl_max_freq_partial_halt,
#if IS_ENABLED(CONFIG_XRING_CORECTL)
		.maxlen		= sizeof(unsigned int) * MAX_CLUSTERS,
		.mode		= 0644,
		.proc_handler	= sched_fmax_cap_handler,
#else
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
#endif
	},
	{
		.procname	= "sched_fmax_cap",
		.data		= &sysctl_fmax_cap,
		.maxlen		= sizeof(unsigned int) * MAX_CLUSTERS,
		.mode		= 0644,
		.proc_handler	= sched_fmax_cap_handler,
	},
	{
		.procname	= "sched_high_perf_cluster_freq_cap",
		.data		= &high_perf_cluster_freq_cap,
		.maxlen		= sizeof(unsigned int) * MAX_CLUSTERS,
		.mode		= 0644,
		.proc_handler	= sched_fmax_cap_handler,
	},
	{
		.procname	= "xr_yield_invalid_time_ns",
		.data		= &sysctl_xr_yield_invalid_time_ns,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "xr_top_task_boost",
		.data		= &sysctl_xr_top_task_boost,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ONE,
		/* make sure the extra2 match the NUM_LOAD_INDICES */
		.extra2		= &one_thousand,
	},
	{
		.procname	= "xr_top_task_filter_coefficient",
		.data		= &sysctl_xr_top_task_filter_coef,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ONE,
		/* make sure the extra2 match the XR_WALT_TOP_TASK_FRAC */
		.extra2		= &one_thousand_twenty_four,
	},
	{
		.procname	= "xr_ed_task_run_threshold",
		.data		= &sysctl_ed_run_res,
		.maxlen		= sizeof(unsigned int) * MAX_CLUSTERS,
		.mode		= 0644,
		.proc_handler	= walt_proc_ed_thresholds_handler,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "xr_ed_task_wait_threshold",
		.data		= &sysctl_ed_wait_res,
		.maxlen		= sizeof(unsigned int) * MAX_CLUSTERS,
		.mode		= 0644,
		.proc_handler	= walt_proc_ed_thresholds_handler,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "xr_ed_task_new_task_run_threshold",
		.data		= &sysctl_ed_new_res,
		.maxlen		= sizeof(unsigned int) * MAX_CLUSTERS,
		.mode		= 0644,
		.proc_handler	= walt_proc_ed_thresholds_handler,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	{
		.procname	= "xr_ed_task_enable",
		.data		= &sysctl_ed_task_enabled,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_ONE,
	},
	//MIUI ADD: Performance_BoostFramework
	{
		.procname	= "sched_leader_task_prefer_cluster",
		.data		= (int *) LEADER_TASK_PREFER_CLUSTER,
		.maxlen		= sizeof(unsigned int) * 2,
		.mode		= 0644,
		.proc_handler	= sched_task_handler,
	},
	{
		.procname	= "sched_prefer_cluster_boost_thres",
		.data		= &sysctl_sched_prefer_cluster_boost_thres,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_INT_MAX,
	},
	//END Performance_BoostFramework
#if IS_ENABLED(CONFIG_XRING_TASK_PP)
	{
		.procname	= "task_pp_idle_thres",
		.data		= &sysctl_task_pp_idle_thres,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
	},
#endif
	{ }
};

void walt_tunables(void)
{
	int i;

	for (i = 0; i < MAX_MARGIN_LEVELS; i++) {
		sysctl_sched_capacity_margin_up_pct[i] = 95; /* ~5% margin */
		sysctl_sched_capacity_margin_dn_pct[i] = 85; /* ~15% margin */
		sysctl_sched_cpu_capacity_margin_pct[i] = 95;
		sysctl_sched_early_up[i] = 1077;
		sysctl_sched_early_down[i] = 1204;
	}

	sysctl_sched_concurrency_thres = 2;

	sysctl_sched_group_upmigrate_pct = 100;

	sysctl_sched_group_downmigrate_pct = 95;

	sysctl_sched_task_unfilter_period = 100000000;

	sysctl_sched_window_stats_policy = WINDOW_STATS_MAX_RECENT_AVG;

	sysctl_sched_ravg_window_nr_ticks = (HZ / NR_WINDOWS_PER_SEC);

	sched_load_granule = DEFAULT_SCHED_RAVG_WINDOW / NUM_LOAD_INDICES;

	sched_ravg_window = DEFAULT_SCHED_RAVG_WINDOW;

	/* Boot global boost to big cores */
	sysctl_sched_global_boost_min_uclamp = one_thousand_twenty_four;

	for (i = 0; i < MAX_CLUSTERS; i++) {
		sysctl_fmax_cap[i] = FREQ_QOS_MAX_DEFAULT_VALUE;
		high_perf_cluster_freq_cap[i] = FREQ_QOS_MAX_DEFAULT_VALUE;
#if IS_ENABLED(CONFIG_XRING_CORECTL)
		sysctl_max_freq_partial_halt[i] = FREQ_QOS_MAX_DEFAULT_VALUE;
#endif
		/*
		 * initialize the param of ed_task per cpu
		 */
		sysctl_ed_run_res[i] = WALT_XR_ED_RUN_RES;
		sysctl_ed_wait_res[i] = WALT_XR_ED_WAIT_RES;
		sysctl_ed_new_res[i] = WALT_XR_ED_NEW_RES;
	}
}

void walt_sysctl_init(void)
{
	if (!register_sysctl("walt", walt_table))
		WALT_BUG(WALT_BUG_WALT, NULL, " %s sysctl init failed",
				"walt");

	if (!register_sysctl("walt/input_boost", input_boost_sysctls))
		WALT_BUG(WALT_BUG_WALT, NULL, " %s sysctl init failed",
				"input_boost");
}
