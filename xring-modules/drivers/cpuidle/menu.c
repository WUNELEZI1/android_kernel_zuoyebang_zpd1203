// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/cpuidle.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/tick.h>
#include <linux/sched.h>
#include <linux/sched/loadavg.h>
#include <linux/sched/stat.h>
#include <linux/sched/cputime.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/cpu.h>
#include <linux/pm_qos.h>
#include <trace/hooks/cpuidle.h>
#include <soc/xring/securelib/securec.h>
#include <sched.h>
#include "custom.h"
#include "trace.h"

#define BUCKETS 12
#define INTERVAL_SHIFT 3
#define INTERVALS (1UL << INTERVAL_SHIFT)
#define EST_SAMPLE_MIN (INTERVALS >> 1)
#define RESOLUTION 64
#define DECAY 128
#define MAX_INTERESTING 50000
/*
 * Idle state target residency threshold used for deciding whether or not to
 * check the time till the closest expected timer event.
 */
#define RESIDENCY_THRESHOLD_US	(15)

#define HIST_SIZE 12


struct xr_menu_hist_cstate_data {
	int	idle_state_idx;
	int	org_state_idx;
	unsigned int	residency_us;
	s64	exit_time;
};

struct xr_menu_hist_state_info {
	unsigned int	early_wakeup_cnt;
	u64	early_wakeup_sleep_us;

	unsigned int	min_residency_us;
	unsigned int	max_residency_us;
	unsigned int	total_count;
	unsigned int	busy_cnt;
	u64	total_residency_us;
	u64	busy_residency_us;
	u64	total_us;

	u64	last_run_us;
};

struct xr_menu_device {
	int             needs_update;
	int             tick_wakeup;

	unsigned int	bucket;
	unsigned int	correction_factor[BUCKETS];
	unsigned int	intervals[INTERVALS];
	int		interval_ptr;

	/* menu custom */
	int		last_state_idx;
	u64		next_timer_us;
	unsigned int	predicted_us;
	unsigned int	repeat;
	int		org_state_idx;
	int		hist_inv_flag;
	int		hist_inv_repeat;
	int		hist_inv_repeat_old;
	int		hist_inv_predict;
	int		hist_ptr;
	struct xr_menu_hist_cstate_data hist_data[HIST_SIZE];

	unsigned int	hrtime_us;
	unsigned int	hrtime_out;
	unsigned int	hrtime_addtime;
};

enum {
	MENU_HRTIMER_STOP,
	MENU_HRTIMER_REPEAT,		/* To watch the prediction result from menu history */
	MENU_HRTIMER_GENERAL,		/* To watch the wakeup modification made by menu buckets */
	MENU_HRTIMER_INV,		/* To watch the evaluation result */
};


static DEFINE_PER_CPU(struct xr_menu_device, xr_menu_devices);
static DEFINE_PER_CPU(int, hrtimer_status);
static DEFINE_PER_CPU(struct hrtimer, xr_menu_hrtimer);
static int xr_menu_config_created;
/* the cpus to re-select idle states after menu prediction */
static struct cpumask menu_cpumask;
/* the lowest cpu frequency required to turn on monitor timer */
static unsigned int menu_switch_profile __read_mostly = 1600000;
/* the preferred sleep time to enter deep idle states */
static unsigned int perfect_cstate_ms __read_mostly = 30;
/* menu re-select configurables */
static unsigned int hist_window_size_us = 20000;
static unsigned int hist_min_duration_us = 500;
static unsigned int hist_max_duration_us = 3000;
static unsigned int hist_avg_duration_us = 800;
static unsigned int hist_idle_cnt = HIST_SIZE / 3;
static unsigned int hist_idle_ratio = 30;
static unsigned int hist_long_exec_us = 1000;
static unsigned int hist_total_exec_us = 1000 * HIST_SIZE;
static unsigned int hist_recent_busy_thres = 1;
static unsigned int hist_busy_depth = 2;
static unsigned int hist_early_wakeup_step_width = 3;
static unsigned int hist_early_wakeup_depth;
static unsigned int hist_early_wakeup_thres = 2;
static unsigned int hist_mispredict_thres = 2;
static unsigned int hist_repeat_thres = 2;
static unsigned int hrtimer_offset_us = 500;
static unsigned int hrtimer_min_us = 800;
static unsigned int hrtimer_max_us = 20000;

#define menu_value_read(_name)						\
static ssize_t show_##_name(struct cpuidle_driver *drv,			\
			  struct cpuidle_device *dev,			\
			  int idx, char *buf)				\
{									\
	return sprintf(buf, "%u\n", _name);				\
}

#define menu_value_write(_name)						\
static ssize_t store_##_name(struct cpuidle_driver *drv,		\
			     struct cpuidle_device *dev,		\
			     int idx, const char *buf, size_t count)	\
{									\
	unsigned int val;						\
									\
	if (sscanf(buf, "%u\n", &val) != 1)				\
		return -EINVAL;						\
									\
	_name = val;							\
									\
	return count;							\
}

#define menu_attr_rw(name) \
menu_value_read(name); \
menu_value_write(name); \
custom_idle_attr_rw(name)

static ssize_t show_menu_cpumask(struct cpuidle_driver *drv,
					struct cpuidle_device *dev,
					int idx, char *buf)
{
	return sprintf(buf, "0x%lx\n", (*(cpumask_bits(&menu_cpumask))));
}

static ssize_t store_menu_cpumask(struct cpuidle_driver *drv,
					struct cpuidle_device *dev,
					int idx, const char *buf, size_t count)
{
	unsigned long bitmask;
	int ret = 0;

	ret = kstrtoul(buf, 0, &bitmask);
	if (ret < 0)
		return ret;

	bitmap_and(cpumask_bits(&menu_cpumask), &bitmask,
			cpumask_bits(cpu_possible_mask), __BITS_PER_LONG);

	return count;
}

custom_idle_attr_rw(menu_cpumask);
menu_attr_rw(hist_window_size_us);
menu_attr_rw(hist_min_duration_us);
menu_attr_rw(hist_max_duration_us);
menu_attr_rw(hist_avg_duration_us);
menu_attr_rw(hist_idle_cnt);
menu_attr_rw(hist_idle_ratio);
menu_attr_rw(hist_total_exec_us);
menu_attr_rw(hist_long_exec_us);
menu_attr_rw(hist_recent_busy_thres);
menu_attr_rw(hist_busy_depth);
menu_attr_rw(hist_early_wakeup_step_width);
menu_attr_rw(hist_early_wakeup_depth);
menu_attr_rw(hist_early_wakeup_thres);
menu_attr_rw(hist_mispredict_thres);
menu_attr_rw(hist_repeat_thres);
menu_attr_rw(hrtimer_offset_us);
menu_attr_rw(hrtimer_min_us);
menu_attr_rw(hrtimer_max_us);
menu_attr_rw(menu_switch_profile);
menu_attr_rw(perfect_cstate_ms);

static struct attribute *menu_attrs[] = {
	&attr_custom_menu_cpumask.attr,
	&attr_custom_menu_switch_profile.attr,
	&attr_custom_perfect_cstate_ms.attr,
	&attr_custom_hist_window_size_us.attr,
	&attr_custom_hist_min_duration_us.attr,
	&attr_custom_hist_max_duration_us.attr,
	&attr_custom_hist_avg_duration_us.attr,
	&attr_custom_hist_idle_cnt.attr,
	&attr_custom_hist_idle_ratio.attr,
	&attr_custom_hist_total_exec_us.attr,
	&attr_custom_hist_long_exec_us.attr,
	&attr_custom_hist_recent_busy_thres.attr,
	&attr_custom_hist_busy_depth.attr,
	&attr_custom_hist_early_wakeup_step_width.attr,
	&attr_custom_hist_early_wakeup_depth.attr,
	&attr_custom_hist_early_wakeup_thres.attr,
	&attr_custom_hist_mispredict_thres.attr,
	&attr_custom_hist_repeat_thres.attr,
	&attr_custom_hrtimer_offset_us.attr,
	&attr_custom_hrtimer_min_us.attr,
	&attr_custom_hrtimer_max_us.attr,
	NULL
};
ATTRIBUTE_GROUPS(menu);


static int xr_menu_need_timer(int cpu)
{
	struct cpufreq_policy *policy;
	unsigned int freq = 0;

	if (!cpumask_test_cpu(cpu, &menu_cpumask))
		return 0;

	policy = cpufreq_cpu_get(cpu);
	if (policy) {
		freq = policy->cur;
		cpufreq_cpu_put(policy);
	}

	/* switch on the monitor timer if the new frequency is high enough */
	if (menu_switch_profile < freq)
		return 1;
	else
		return 0;
}

/* Cancel the hrtimer if it is not triggered yet */
static void xr_menu_hrtimer_cancel(int cpu)
{
	struct hrtimer *hrtmr = &per_cpu(xr_menu_hrtimer, cpu);

	/* The monitor timer didn't trap */
	if (per_cpu(hrtimer_status, cpu)) {
		hrtimer_cancel(hrtmr);
		per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_STOP;
	}
}

/* Call back for hrtimer is triggered */
static enum hrtimer_restart xr_menu_hrtimer_notify(struct hrtimer *phrtimer)
{
	unsigned int cpu = smp_processor_id();
	struct xr_menu_device *data = &per_cpu(xr_menu_devices, cpu);
	int wakeup_status;

	if (!phrtimer)
		return HRTIMER_NORESTART;

	/*
	 * In general case, the expected residency is much larger than
	 *  deepest C-state target residency, but prediction logic still
	 *  predicts a small predicted residency, so the prediction
	 *  history is totally broken if the timer is triggered.
	 *  So reset the correction factor.
	 */
	wakeup_status = per_cpu(hrtimer_status, cpu);
	per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_STOP;

	if (wakeup_status == MENU_HRTIMER_GENERAL)
		data->correction_factor[data->bucket] = RESOLUTION * DECAY;
	else if (wakeup_status == MENU_HRTIMER_INV)
		data->hist_inv_flag = true;
	else if (wakeup_status == MENU_HRTIMER_REPEAT)
		data->hist_inv_repeat++;
	else
		return HRTIMER_NORESTART;

	data->hrtime_out = true;

	trace_xr_menu_hrtimerout(cpu, wakeup_status, data->hrtime_us);

	return HRTIMER_NORESTART;
}

/*
 * xr_menu_hist_calc_stat_info - statistics the history idle info for
 *    reselect idle stat.including:
 *    a) number and time length of same pre-select idle state
 *       with less idle time than residency_thresh.
 *    b) busy status info, as total running time/ idle time, last run time.
 * @residency_thresh: residency threshold for select excessive deep idle state.
 * @stat_info: the result of history idle state info
 */
static void xr_menu_hist_calc_stat_info(unsigned int residency_thresh,
			struct xr_menu_hist_state_info *stat_info)
{
	int i;
	int hist_ptr;
	unsigned int min_residency_us = hist_window_size_us;
	unsigned int max_residency_us = 0;
	s64 stime;
	s64 enter_time;
	s64 first_enter_time;
	s64 last_exit_time = 0;
	struct xr_menu_device *data = this_cpu_ptr(&xr_menu_devices);
	struct xr_menu_hist_cstate_data *hist_data;
	int recent_busy = true;
	unsigned int step = hist_early_wakeup_step_width;

	stime = ktime_to_us(ktime_get());
	first_enter_time = stime;
	hist_ptr = data->hist_ptr;

	/* Start from the most recent idle history */
	for (i = 0 ; i < HIST_SIZE ; i++) {
		hist_ptr--;
		if (hist_ptr < 0)
			hist_ptr += HIST_SIZE;
		hist_data = &data->hist_data[hist_ptr];

		if (hist_data->residency_us == 0)
			continue;

		/* calc last exit time for last run time. */
		if (last_exit_time < hist_data->exit_time)
			last_exit_time = hist_data->exit_time;

		/* Exclude all the history outside the window */
		enter_time = hist_data->exit_time - hist_data->residency_us;
		if (enter_time < 0)
			continue;
		if (enter_time + hist_window_size_us <= last_exit_time)
			continue;

		if (first_enter_time > enter_time)
			first_enter_time = enter_time;

		if (min_residency_us > hist_data->residency_us)
			min_residency_us = hist_data->residency_us;
		if (max_residency_us < hist_data->residency_us)
			max_residency_us = hist_data->residency_us;

		/* Check if the CPU is experiencing short sleeps */
		if (recent_busy && hist_data->residency_us < hist_min_duration_us) {
			stat_info->busy_residency_us += hist_data->residency_us;
			stat_info->busy_cnt++;
		} else {
			recent_busy = false;
		}

		/* Find out all the early wakeup conditions */
		if (step) {
			if (hist_data->residency_us < residency_thresh) {
				stat_info->early_wakeup_cnt++;
				stat_info->early_wakeup_sleep_us += hist_data->residency_us;

				/* Found a failed prediction! Replenish the look back buffer */
				step = hist_early_wakeup_step_width;
			}

			step--;
		}

		stat_info->total_count++;
		stat_info->total_residency_us += hist_data->residency_us;

		trace_xr_menu_hist_data(hist_ptr,
			hist_data->idle_state_idx, hist_data->org_state_idx,
			hist_data->residency_us, hist_data->exit_time);
	}

	/* exclude the recent execution time, the long execution case would be handled
	 * seperately when watching the history
	 */
	stat_info->total_us =
		min_t(u64, hist_window_size_us, last_exit_time - first_enter_time);
	stat_info->total_residency_us =
		min_t(u64, hist_window_size_us, stat_info->total_residency_us);

	stat_info->last_run_us = stime - last_exit_time;
	stat_info->min_residency_us = min_residency_us;
	stat_info->max_residency_us = max_residency_us;

	trace_xr_menu_hist_info(stat_info->total_count, stat_info->total_residency_us,
		stat_info->busy_residency_us, stat_info->busy_cnt,
		stat_info->total_us, stat_info->last_run_us,
		stat_info->min_residency_us, stat_info->max_residency_us,
		stat_info->early_wakeup_cnt, residency_thresh, stime);
}

static int xr_menu_hist_sel_state_idx(struct cpuidle_driver *drv,
			struct cpuidle_device *dev,
			unsigned int target_residency, int exit_latency)
{
	struct custom_config *cfg = per_cpu_ptr(&custom_idle_config, dev->cpu);
	int i;
	int last_state_idx = 0;

	/* Use history data to choice new idle state*/
	for (i = 1; i < drv->state_count; i++) {
		struct cpuidle_state_usage *lu = &dev->states_usage[i];

		if (lu->disable)
			continue;

		if (cfg->state[i].residency_us > target_residency)
			continue;

		if (cfg->state[i].latency_us > exit_latency) {
			custom_set_reason(LATENCY);
			continue;
		}

		last_state_idx = i;
	}

	return last_state_idx;
}

/*
 * xr_menu_hist_get_busy_flag - get cpu busy status
 * @stat_info: the statistics info of history idle
 */
static int xr_menu_hist_get_busy_flag(struct xr_menu_hist_state_info *stat_info)
{
	/* cpu frequently wakes up recently, it could be some
	 * compeat workload pattern
	 */
	if (stat_info->busy_cnt > hist_recent_busy_thres)
		return 1;

	/*
	 * cpu was running for a long time and
	 * the idle time is less than the thresholds.
	 */
	if (stat_info->last_run_us > hist_long_exec_us) {
		if (stat_info->min_residency_us < hist_min_duration_us)
			return 1;

		if (stat_info->max_residency_us < hist_max_duration_us)
			return 1;
	}

	/*
	 * there are multi idle history records which has short idle time.
	 */
	if (stat_info->total_count > hist_idle_cnt &&
	    stat_info->total_residency_us < stat_info->total_count * hist_avg_duration_us)
		return 2;

	/*
	 * task is busy, do not goto deep sleep.
	 */
	if ((stat_info->total_residency_us) &&
	    ((stat_info->total_residency_us + hist_total_exec_us) < stat_info->total_us ||
	    stat_info->total_residency_us < (stat_info->total_us * hist_idle_ratio / 100)))
		return 2;

	return 0;
}

/*
 * xr_menu_hist_busy_resel - reslect idle state by busy status.
 * @stat_info: the statistics info of history idle
 */
static inline int xr_menu_hist_busy_resel(struct xr_menu_device *data,
			struct xr_menu_hist_state_info *stat_info)
{
	int hist_busy_flag;

	if (data->last_state_idx < 1)
		return 0;

	hist_busy_flag = xr_menu_hist_get_busy_flag(stat_info);
	if (!hist_busy_flag)
		return 0;

	if (hist_busy_flag == 1 && stat_info->busy_cnt) {
		do_div(stat_info->busy_residency_us, stat_info->busy_cnt);
		return min_t(unsigned int, data->predicted_us,
				stat_info->busy_residency_us);
	} else if (stat_info->total_count) {
		do_div(stat_info->total_residency_us, stat_info->total_count);
		return min_t(unsigned int, data->predicted_us,
				stat_info->total_residency_us);
	} else {
		return min_t(unsigned int, data->predicted_us,
				hrtimer_min_us);
	}
}

/*
 * xr_menu_hist_check_state_idx - check state idx enable or disable
 * @drv: cpuidle driver containing state data
 * @dev: cpuidle_device
 * @state_idx: cpu idle state idx
 * return: 1: enable; 0: disabled
 */
static int xr_menu_hist_check_state_idx(struct cpuidle_driver *drv,
			struct cpuidle_device *dev, int state_idx)
{
	if (state_idx >= drv->state_count)
		return 0;

	if (dev->states_usage[state_idx].disable)
		return 0;

	return 1;
}

/*
 * xr_menu_hist_reselect - reselects the next idle state to enter
 *    with history idle info
 * @drv: cpuidle driver containing state data
 * @dev: the CPU
 * @latency_req: latency qos request
 */
static void xr_menu_hist_reselect(struct cpuidle_driver *drv,
			struct cpuidle_device *dev, int latency_req)
{
	unsigned int failed_cnt_th = hist_early_wakeup_thres;
	struct xr_menu_device *data = per_cpu_ptr(&xr_menu_devices, dev->cpu);
	struct custom_config *cfg = per_cpu_ptr(&custom_idle_config, dev->cpu);

	data->org_state_idx = data->last_state_idx;

	if (data->last_state_idx > 0 &&
	    cpumask_test_cpu((int)dev->cpu, &menu_cpumask) &&
	    data->hist_inv_repeat  < hist_repeat_thres &&
	    data->hist_inv_predict < hist_mispredict_thres) {
		struct xr_menu_hist_state_info stat_info = {0};

		xr_menu_hist_calc_stat_info(
			(cfg->state[data->last_state_idx].residency_us +
			cfg->state[data->last_state_idx].latency_us) / 2,
			&stat_info);

		if (data->last_state_idx + hist_early_wakeup_depth >= drv->state_count)
			failed_cnt_th++;

		if (stat_info.early_wakeup_cnt > failed_cnt_th) {
			unsigned int fail_us, fail_idx;

			do_div(stat_info.early_wakeup_sleep_us,
				stat_info.early_wakeup_cnt);

			fail_us = min_t(unsigned int, data->predicted_us,
				stat_info.early_wakeup_sleep_us);

			fail_idx = xr_menu_hist_sel_state_idx(drv, dev, fail_us, latency_req);
			if (fail_idx < data->last_state_idx) {
				custom_set_reason(PREDICT);
				data->last_state_idx = fail_idx;
				data->predicted_us = fail_us;
			}
		}

		if (data->last_state_idx + hist_busy_depth >= drv->state_count &&
		    stat_info.total_count > hist_recent_busy_thres) {
			unsigned int busy_us, busy_idx;

			busy_us = xr_menu_hist_busy_resel(data, &stat_info);
			if (busy_us) {
				busy_idx = xr_menu_hist_sel_state_idx(drv, dev, busy_us, latency_req);
				if (busy_idx < data->last_state_idx) {
					custom_set_reason(PREDICT);
					data->last_state_idx = busy_idx;
					data->predicted_us = busy_us;
				}
			}
		}
	}

	if (data->hist_inv_predict == 1 &&
	    data->last_state_idx < (drv->state_count - 1)) {
		data->last_state_idx++;

		if (data->last_state_idx < data->org_state_idx)
			data->last_state_idx++;
	}

	if (data->org_state_idx != data->last_state_idx) {
		if (!xr_menu_hist_check_state_idx(drv, dev, data->last_state_idx))
			data->last_state_idx = data->org_state_idx;
	}

	if (data->hist_inv_predict >= hist_mispredict_thres)
		data->hist_inv_predict = 0;

	if (data->hist_inv_repeat >= hist_repeat_thres)
		data->hist_inv_repeat = 0;
}


/*
 * xr_menu_sel_hrtimer_algo - calc the hrtime length and type.
 * @data: menu device
 * @drv: idle driver
 * @hrtimer_type: timer type to return
 */
static unsigned int xr_menu_sel_hrtimer_algo(struct xr_menu_device *data,
				struct cpuidle_driver *drv, int *hrtimer_type)
{
	struct custom_config *cfg = this_cpu_ptr(&custom_idle_config);
	unsigned int hrtime_us;
	unsigned int last_time_us;
	unsigned int next_time_us;
	unsigned int this_residency;
	unsigned int next_residency;
	unsigned int this_latency;
	unsigned int next_latency;
	int timer_type;

	if (data->last_state_idx + 1 >= drv->state_count)
		return 0;

	this_residency = cfg->state[data->last_state_idx].residency_us;
	next_residency = cfg->state[data->last_state_idx + 1].residency_us;
	this_latency = cfg->state[data->last_state_idx].latency_us;
	next_latency = cfg->state[data->last_state_idx + 1].latency_us;

	last_time_us = this_residency + this_latency;
	next_time_us = next_residency + next_latency;

	timer_type = MENU_HRTIMER_STOP;
	/* Avoid new predictions result cause shallow sleep */
	if (data->last_state_idx != data->org_state_idx &&
	    data->next_timer_us > last_time_us + next_time_us) {
		timer_type = MENU_HRTIMER_INV;
	}
	/* Avoid error prediction cause shallow sleep */
	else if (data->next_timer_us > perfect_cstate_ms * USEC_PER_MSEC ||
		 data->next_timer_us > data->predicted_us + next_time_us) {

		if (data->repeat)
			/* Repeat mode enabled when we got a prediction from the Menu history.
			 * Start the timer to see whether a deeper state should be selected.
			 */
			timer_type = MENU_HRTIMER_REPEAT;
		else
			timer_type = MENU_HRTIMER_GENERAL;
	}

	hrtime_us = 0;
	if (timer_type != MENU_HRTIMER_STOP) {
		*hrtimer_type = timer_type;
		hrtime_us = data->predicted_us + hrtimer_offset_us;
		/* timer > this_state_residency, and within [next/2, next] */
		hrtime_us = max(hrtime_us, next_residency / 2);
		hrtime_us = min(hrtime_us, next_residency);
		hrtime_us = max(hrtime_us, this_residency);

		hrtime_us = max_t(unsigned int, hrtime_us, hrtimer_min_us);
		hrtime_us = min_t(unsigned int, hrtime_us, hrtimer_max_us);
	}

	return hrtime_us;
}

static void xr_menu_sel_hrtimer_start(struct cpuidle_driver *drv,
			struct cpuidle_device *dev)
{
	unsigned int cpu = dev->cpu;
	struct hrtimer *ek_hrtimer = NULL;
	struct xr_menu_device *data = this_cpu_ptr(&xr_menu_devices);
	int hrtimer_type = MENU_HRTIMER_STOP;

	data->hrtime_us = 0;
	if (xr_menu_need_timer(dev->cpu) &&
	    data->last_state_idx + 1 < drv->state_count) {
		data->hrtime_us = xr_menu_sel_hrtimer_algo(data,
						drv, &hrtimer_type);
		per_cpu(hrtimer_status, cpu) = hrtimer_type;

		if (data->hrtime_us) {
			ek_hrtimer = &per_cpu(xr_menu_hrtimer, cpu);
			hrtimer_start(ek_hrtimer,
				ns_to_ktime(data->hrtime_us * NSEC_PER_USEC),
				HRTIMER_MODE_REL_PINNED);
		}
	}
}

static inline void xr_menu_hist_update_inv_predict(struct xr_menu_device *data)
{
	if (data->hist_inv_flag) {
		if (data->hist_inv_predict < hist_mispredict_thres)
			data->hist_inv_predict++;
	} else {
		data->hist_inv_predict = 0;
	}
}

static inline void xr_menu_hist_update_interval_ptr(struct xr_menu_device *data)
{
	data->interval_ptr++;
	if (data->interval_ptr >= INTERVALS)
		data->interval_ptr = 0;

}

static inline void xr_menu_hist_update_hist_ptr(struct xr_menu_device *data)
{
	data->hist_ptr++;
	if (data->hist_ptr >= HIST_SIZE)
		data->hist_ptr = 0;
}

static inline void xr_menu_hist_update_inv_repeat(struct xr_menu_device *data)
{
	if (data->hist_inv_repeat > 0 &&
	    data->hist_inv_repeat_old == data->hist_inv_repeat)
		data->hist_inv_repeat--;

	data->hist_inv_repeat_old = data->hist_inv_repeat;
}

/*
 * xr_menu_hist_update - update history info
 * @measured_us: last idle time length
 */
static void xr_menu_hist_update(struct xr_menu_device *data, unsigned int measured_us)
{
	if (data->hrtime_addtime) {
		data->intervals[data->interval_ptr] += measured_us;
		data->hist_data[data->hist_ptr].residency_us +=
			measured_us;
	} else {
		data->intervals[data->interval_ptr] = measured_us;
		data->hist_data[data->hist_ptr].residency_us =
			measured_us;
	}

	/* Update history data */
	xr_menu_hist_update_inv_predict(data);

	trace_xr_menu_update(data->hist_data[data->hist_ptr].residency_us,
		data->hist_data[data->hist_ptr].idle_state_idx,
		data->hist_data[data->hist_ptr].exit_time,
		data->hist_inv_flag,
		data->hist_inv_repeat,
		data->hist_inv_predict,
		data->hrtime_addtime);

	data->hist_inv_flag = false;
	data->hrtime_addtime = false;

	if (data->hrtime_out) {
		/*
		 * menu gov's hrtime call up core, set flag to add the next idle time.
		 * the sum of last idle and next idle is real idle time.
		 */
		data->hrtime_out = false;
		data->hrtime_addtime = true;
	} else {
		xr_menu_hist_update_interval_ptr(data);

		xr_menu_hist_update_hist_ptr(data);
	}

	xr_menu_hist_update_inv_repeat(data);
}

static inline int which_bucket(u64 duration_us, unsigned int nr_iowaiters)
{
	int bucket = 0;

	/*
	 * We keep two groups of stats; one with no
	 * IO pending, one without.
	 * This allows us to calculate
	 * E(duration)|iowait
	 */
	if (nr_iowaiters)
		bucket = BUCKETS/2;

	if (duration_us < 10ULL)
		return bucket;
	if (duration_us < 100ULL)
		return bucket + 1;
	if (duration_us < 1000ULL)
		return bucket + 2;
	if (duration_us < 10000ULL)
		return bucket + 3;
	if (duration_us < 100000ULL)
		return bucket + 4;
	return bucket + 5;
}

/*
 * Return a multiplier for the exit latency that is intended
 * to take performance requirements into account.
 * The more performance critical we estimate the system
 * to be, the higher this multiplier, and thus the higher
 * the barrier to go to an expensive C state.
 */
static inline int performance_multiplier(unsigned int nr_iowaiters)
{
	/* for IO wait tasks (per cpu!) we add 10x each */
	return 1 + 10 * nr_iowaiters;
}

static void xr_menu_update(struct cpuidle_driver *drv, struct cpuidle_device *dev);

/*
 * Try detecting repeating patterns by keeping track of the last 8
 * intervals, and checking if the standard deviation of that set
 * of points is below a threshold. If it is... then use the
 * average of these 8 points as the estimated value.
 */
static unsigned int get_typical_interval(struct xr_menu_device *data)
{
	int i, cnt, max_idx;
	unsigned int avg, min;
	unsigned int copy[INTERVALS];
	uint64_t sum, variance, max_diff;

	min = UINT_MAX;
	cnt = 0;
	sum = 0;
	for (i = 0; i < INTERVALS; i++) {
		if (data->intervals[i]) {
			copy[i] = data->intervals[i];

			sum += copy[i];
			cnt++;

			if (min > copy[i])
				min = copy[i];
		} else {
			copy[i] = 0;
		}
	}

	/*
	 * If the result of the computation is going to be discarded anyway,
	 * avoid the computation altogether.
	 */
	if (cnt < EST_SAMPLE_MIN)
		return UINT_MAX;

again:
	if (cnt == INTERVALS)
		avg = sum >> INTERVAL_SHIFT;
	else
		avg = div_u64(sum, cnt);

	/* Then try to determine variance */
	variance = 0;
	max_diff = 0;
	for (i = 0; i < INTERVALS; i++) {
		int64_t diff;
		int64_t value = copy[i];

		if (value == 0)
			continue;

		diff = value - avg;
		diff = diff * diff;
		if (diff > max_diff) {
			max_diff = diff;
			max_idx = i;
		}
		variance += diff;
	}

	if (cnt == INTERVALS)
		variance >>= INTERVAL_SHIFT;
	else
		do_div(variance, cnt);

	/*
	 * The typical interval is obtained when standard deviation is
	 * small (stddev <= 20 us, variance <= 400 us^2) or standard
	 * deviation is small compared to the average interval (avg >
	 * 6*stddev, avg^2 > 36*variance). The average is smaller than
	 * UINT_MAX aka U32_MAX, so computing its square does not
	 * overflow a u64. We simply reject this candidate average if
	 * the standard deviation is greater than 715 s (which is
	 * rather unlikely).
	 *
	 * Use this result only if there is no timer to wake us up sooner.
	 */
	if (likely(variance <= U64_MAX/36)) {
		if ((((u64)avg*avg > variance*36) && (cnt >= EST_SAMPLE_MIN))
							|| variance <= 400) {
			data->repeat = 1;
			return avg;
		}
	}

	if (cnt <= EST_SAMPLE_MIN) {
		data->repeat = 0;
		return UINT_MAX;
	}

	/* remove the data outside of the central group */
	sum -= copy[max_idx];
	copy[max_idx] = 0;
	cnt--;

	goto again;
}

/*
 * xr_menu_select - selects the next idle state to enter
 * @drv: cpuidle driver containing state data
 * @dev: the CPU
 * @stop_tick: indication on whether or not to stop the tick
 */
static int xr_menu_select(struct cpuidle_driver *drv, struct cpuidle_device *dev,
		       bool *stop_tick)
{
	struct xr_menu_device *data = this_cpu_ptr(&xr_menu_devices);
	struct custom_config *cfg = per_cpu_ptr(&custom_idle_config, dev->cpu);
	unsigned int latency_req = ktime_to_us(cpuidle_governor_latency_req(dev->cpu));
	unsigned int predicted_us;
	unsigned int typical_us;
	unsigned int interactivity_req;
	unsigned int nr_iowaiters;
	ktime_t delta, delta_tick;
	int i, idx;
	int max_depth;

	if (data->needs_update) {
		xr_menu_update(drv, dev);
		data->needs_update = 0;
	}

	max_depth = custom_get_allowed_depth(drv, dev);
	if (max_depth == 0) {
		*stop_tick = !(drv->states[0].flags & CPUIDLE_FLAG_POLLING);

		custom_set_reason(LIMITED);

		data->org_state_idx  = 0;
		data->last_state_idx = 0;
		data->hist_inv_predict = 0;
		data->hist_inv_repeat = 0;

		return 0;
	}

	custom_set_reason(ORIGIN);

	nr_iowaiters = atomic_read(&cpu_rq(dev->cpu)->nr_iowait);

	/* Find the shortest expected idle interval. */
	data->repeat = 0;
	typical_us = get_typical_interval(data);
	if (typical_us > RESIDENCY_THRESHOLD_US) {
		unsigned int timer_us;

		/* determine the expected residency time, round up */
		delta = tick_nohz_get_sleep_length(&delta_tick);
		if (unlikely(delta < 0)) {
			delta = 0;
			delta_tick = 0;
		}
		data->next_timer_us = ktime_to_us(delta);

		data->bucket = which_bucket(data->next_timer_us, nr_iowaiters);

		/* Round up the result for half microseconds. */
		timer_us = div_u64(data->next_timer_us * data->correction_factor[data->bucket] +
					(RESOLUTION * DECAY) / 2,
				   RESOLUTION * DECAY);
		predicted_us = min(timer_us, typical_us);
	} else {
		/*
		 * Because the next timer event is not going to be determined
		 * in this case, assume that without the tick the closest timer
		 * will be in distant future and that the closest tick will occur
		 * after 1/2 of the tick period.
		 */
		data->next_timer_us = ktime_to_us(KTIME_MAX);
		delta_tick = TICK_NSEC / 2;
		data->bucket = which_bucket(data->next_timer_us, nr_iowaiters);
		predicted_us = typical_us;
	}

	if (unlikely(drv->state_count <= 1 || latency_req == 0) ||
	    ((data->next_timer_us < cfg->state[1].residency_us ||
	      latency_req < cfg->state[1].latency_us) &&
	     !dev->states_usage[0].disable)) {
		/*
		 * In this case state[0] will be used no matter what, so return
		 * it right away and keep the tick running if state[0] is a
		 * polling one.
		 */
		*stop_tick = !(drv->states[0].flags & CPUIDLE_FLAG_POLLING);

		if (latency_req < cfg->state[1].latency_us)
			custom_set_reason(LATENCY);

		data->org_state_idx  = 0;
		data->last_state_idx = 0;
		data->hist_inv_predict = 0;
		data->hist_inv_repeat = 0;

		return 0;
	}

	if (predicted_us < TICK_USEC) {
		predicted_us = data->next_timer_us;
	} else {
		/*
		 * Use the performance multiplier and the user-configurable
		 * latency_req to determine the maximum exit latency.
		 */
		interactivity_req = predicted_us / performance_multiplier(nr_iowaiters);
		if (latency_req > interactivity_req)
			latency_req = interactivity_req;
	}

	/*
	 * Find the idle state with the lowest power while satisfying
	 * our constraints.
	 */
	idx = -1;
	for (i = 0; i < drv->state_count; i++) {
		if (dev->states_usage[i].disable)
			continue;

		if (idx == -1)
			idx = i; /* first enabled state */

		if (cfg->state[i].residency_us > predicted_us) {
			/*
			 * Use a physical idle state, not busy polling, unless
			 * a timer is going to trigger soon enough.
			 */
			if ((drv->states[idx].flags & CPUIDLE_FLAG_POLLING) &&
			    cfg->state[i].latency_us <= latency_req &&
			    cfg->state[i].residency_us <= data->next_timer_us) {
				predicted_us = cfg->state[i].residency_us;
				idx = i;
				break;
			}
			if (predicted_us < TICK_USEC)
				break;

			/*
			 * If the state selected so far is shallow and this
			 * state's target residency matches the time till the
			 * closest timer event, select this one to avoid getting
			 * stuck in the shallow one for too long.
			 */
			if (cfg->state[idx].residency_us < TICK_USEC &&
			    cfg->state[i].residency_us <= ktime_to_us(delta_tick))
				idx = i;

			break;
		}

		if (cfg->state[i].latency_us > latency_req) {
			custom_set_reason(LATENCY);
			break;
		}

		idx = i;
	}

	if (idx == -1)
		idx = 0; /* No states enabled. Must use 0. */

	data->predicted_us = predicted_us;
	data->last_state_idx = idx;

	/* Evaluate and re-select the idle state by extended history */
	xr_menu_hist_reselect(drv, dev, latency_req);

#if IS_ENABLED(CONFIG_XRING_SCHED_WALT)
	if (cpu_halted(dev->cpu)) {
		custom_set_reason(HALTED);
		goto end;
	}
#endif

	/* Tick is always disabled. Start a monitor timer to prevent
	 * the CPU from getting stuck in shallow states
	 */
	xr_menu_sel_hrtimer_start(drv, dev);

end:
	if (data->last_state_idx > max_depth) {
		for (idx = max_depth; idx > 0; idx--) {
			if (dev->states_usage[idx].disable)
				continue;

			custom_set_reason(LIMITED);
			xr_menu_hrtimer_cancel(dev->cpu);

			break;
		}

		data->last_state_idx = idx;
	}

	trace_xr_menu_select(dev->cpu,
		data->last_state_idx,
		data->predicted_us,
		latency_req,
		per_cpu(hrtimer_status, dev->cpu),
		data->org_state_idx,
		data->next_timer_us,
		data->hrtime_us,
		ktime_to_us(delta_tick),
		predicted_us,
		typical_us);

	return data->last_state_idx;
}

/*
 * xr_menu_reflect - records that data structures need update
 * @dev: the CPU
 * @index: the index of actual entered state
 *
 * NOTE: it's important to be fast here because this operation will add to
 *       the overall exit latency.
 */
static void xr_menu_reflect(struct cpuidle_device *dev, int index)
{
	struct xr_menu_device *data = this_cpu_ptr(&xr_menu_devices);

	dev->last_state_idx = index;
	data->needs_update = 1;
	data->hist_data[data->hist_ptr].idle_state_idx = index;
	data->hist_data[data->hist_ptr].org_state_idx = data->org_state_idx;
	data->hist_data[data->hist_ptr].exit_time = ktime_to_us(ktime_get());
}

/*
 * xr_menu_update - attempts to guess what happened after entry
 * @drv: cpuidle driver containing state data
 * @dev: the CPU
 */
static void xr_menu_update(struct cpuidle_driver *drv, struct cpuidle_device *dev)
{
	struct xr_menu_device *data = this_cpu_ptr(&xr_menu_devices);
	struct custom_config *cfg = per_cpu_ptr(&custom_idle_config, dev->cpu);
	int last_idx = dev->last_state_idx;
	u64 measured_us;
	unsigned int new_factor;

	/*
	 * Try to figure out how much time passed between entry to low
	 * power state and occurrence of the wakeup event.
	 *
	 * If the entered idle state didn't support residency measurements,
	 * we use them anyway if they are short, and if long,
	 * truncate to the whole expected time.
	 *
	 * Any measured amount of time will include the exit latency.
	 * Since we are interested in when the wakeup begun, not when it
	 * was completed, we must subtract the exit latency. However, if
	 * the measured amount of time is less than the exit latency,
	 * assume the state was never reached and the exit latency is 0.
	 */

	if ((drv->states[last_idx].flags & CPUIDLE_FLAG_POLLING) &&
		   dev->poll_time_limit) {
		/*
		 * The CPU exited the "polling" state due to a time limit, so
		 * the idle duration prediction leading to the selection of that
		 * state was inaccurate.  If a better prediction had been made,
		 * the CPU might have been woken up from idle by the next timer.
		 * Assume that to be the case.
		 */
		measured_us = data->next_timer_us;
	} else {
		/* measured value */
		measured_us = div_u64(dev->last_residency_ns, NSEC_PER_USEC);

		/* Deduct exit latency */
		if (measured_us > 2 * cfg->state[last_idx].latency_us)
			measured_us -= cfg->state[last_idx].latency_us;
		else
			measured_us /= 2;
	}

	/* Make sure our coefficients do not exceed unity */
	if (measured_us > data->next_timer_us)
		measured_us = data->next_timer_us;

	custom_collect_stat(drv, dev, last_idx, measured_us);

	if (custom_forced_state())
		return;

	/* Update our correction ratio */
	new_factor = data->correction_factor[data->bucket];
	new_factor -= new_factor / DECAY;

	if (data->next_timer_us > 0 && measured_us < MAX_INTERESTING)
		new_factor += div64_u64(RESOLUTION * measured_us,
					data->next_timer_us);
	else
		/*
		 * we were idle so long that we count it as a perfect
		 * prediction
		 */
		new_factor += RESOLUTION;

	/*
	 * We don't want 0 as factor; we always want at least
	 * a tiny bit of estimated time. Fortunately, due to rounding,
	 * new_factor will stay nonzero regardless of measured_us values
	 * and the compiler can eliminate this test as long as DECAY > 1.
	 */
	if (DECAY == 1 && unlikely(new_factor == 0))
		new_factor = 1;

	data->correction_factor[data->bucket] = new_factor;

	if (cpumask_test_cpu((int)dev->cpu, &menu_cpumask)) {
		xr_menu_hist_update(data, measured_us);
		return;
	}

	/* update the repeating-pattern data */
	data->intervals[data->interval_ptr++] = measured_us;
	if (data->interval_ptr >= INTERVALS)
		data->interval_ptr = 0;
}

static void menu_idle_exit(void *unused, int state, struct cpuidle_device *dev)
{
	xr_menu_hrtimer_cancel(dev->cpu);
}

/**
 * xr_menu_enable_device - scans a CPU's states and does setup
 * @drv: cpuidle driver
 * @dev: the CPU
 */
static int xr_menu_enable_device(struct cpuidle_driver *drv,
				struct cpuidle_device *dev)
{
	struct xr_menu_device *data = &per_cpu(xr_menu_devices, dev->cpu);
	struct hrtimer *t = &per_cpu(xr_menu_hrtimer, dev->cpu);
	int i;
	int ret;


	cpumask_copy(&menu_cpumask, cpu_possible_mask);

	if (!xr_menu_config_created) {
		ret = custom_create_root_attr(menu_groups);
		if (ret)
			return ret;

		ret = register_trace_prio_android_vh_cpu_idle_exit(menu_idle_exit, NULL, INT_MIN);
		if (ret) {
			custom_remove_root_attr(menu_groups);
			return ret;
		}

		xr_menu_config_created = true;
	}

	hrtimer_init(t, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	t->function = xr_menu_hrtimer_notify;

	memset(data, 0, sizeof(struct xr_menu_device));

	ret = custom_create_state_attr(drv, dev, NULL, NULL);
	if (ret)
		return ret;

	custom_clear_stat(dev->cpu);

	/*
	 * if the correction factor is 0 (eg first time init or cpu hotplug
	 * etc), we actually want to start out with a unity factor.
	 */
	for (i = 0; i < BUCKETS; i++)
		data->correction_factor[i] = RESOLUTION * DECAY;

	return 0;
}

/**
 * xr_menu_disable_device - scans a CPU's states and does setup
 * @drv: cpuidle driver
 * @dev: the CPU
 */
static void xr_menu_disable_device(struct cpuidle_driver *drv,
				struct cpuidle_device *dev)
{
	custom_remove_state_attr(drv, dev, NULL, NULL);

	if (xr_menu_config_created) {
		unregister_trace_android_vh_cpu_idle_exit(menu_idle_exit, NULL);
		custom_remove_root_attr(menu_groups);
		xr_menu_config_created = false;
	}
}

static struct cpuidle_governor xr_menu_governor = {
	.name =		"xr_menu",
	.rating =	50,
	.enable =	xr_menu_enable_device,
	.disable =	xr_menu_disable_device,
	.select =	xr_menu_select,
	.reflect =	xr_menu_reflect,
};

int xr_menu_governor_init(void)
{
	int ret;

	ret = cpuidle_register_governor(&xr_menu_governor);

	return ret;
}
