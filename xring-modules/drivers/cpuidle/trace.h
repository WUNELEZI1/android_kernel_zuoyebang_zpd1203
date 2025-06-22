/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM cpuidle

#if !defined(_TRACE_CPUIDLE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_CPUIDLE_H

#include <linux/tracepoint.h>


TRACE_EVENT(xr_menu_update,
	TP_PROTO(unsigned int last_residency, int last_state,
		 s64 hist_etime, int hist_inv_data, int hist_inv_repeat, int hist_inv_predict,
		 unsigned int hrtime_addtime),

	TP_ARGS(last_residency, last_state, hist_etime,
		hist_inv_data, hist_inv_repeat, hist_inv_predict, hrtime_addtime),

	TP_STRUCT__entry(
		__field(unsigned int, last_residency)
		__field(int, last_state)
		__field(s64, hist_etime)
		__field(int, hist_inv_data)
		__field(int, hist_inv_repeat)
		__field(int, hist_inv_predict)
		__field(unsigned int, hrtime_addtime)
		),

	TP_fast_assign(
		__entry->last_residency = last_residency;
		__entry->last_state = last_state;
		__entry->hist_etime = hist_etime;
		__entry->hist_inv_data = hist_inv_data;
		__entry->hist_inv_repeat = hist_inv_repeat;
		__entry->hist_inv_predict = hist_inv_predict;
		__entry->hrtime_addtime = hrtime_addtime;
		),

	TP_printk("last_residency %u, idle_idx %d, hist_etime %lld, hist_inv_data %d, hist_inv_repeat %d, hist_inv_predict %d addtime %u",
		__entry->last_residency,
		__entry->last_state,
		__entry->hist_etime,
		__entry->hist_inv_data,
		__entry->hist_inv_repeat,
		__entry->hist_inv_predict,
		__entry->hrtime_addtime
		)
);

TRACE_EVENT(xr_menu_hist_data,
	TP_PROTO(int idx, int idle_state_idx, int org_state_idx,
		 unsigned int residency_us, s64 exit_time),

	TP_ARGS(idx, idle_state_idx, org_state_idx, residency_us, exit_time),

	TP_STRUCT__entry(
		__field(int, idx)
		__field(int, idle_state_idx)
		__field(int, org_state_idx)
		__field(unsigned int, residency_us)
		__field(s64, exit_time)
		),

	TP_fast_assign(
		__entry->idx = idx;
		__entry->idle_state_idx = idle_state_idx;
		__entry->org_state_idx = org_state_idx;
		__entry->residency_us = residency_us;
		__entry->exit_time = exit_time;
		),

	TP_printk("[%d], idle_state_idx %d, org_idle_idx %d, residency_us %u, exit_time %lld",
		__entry->idx,
		__entry->idle_state_idx,
		__entry->org_state_idx,
		__entry->residency_us,
		__entry->exit_time
		)
);


TRACE_EVENT(xr_menu_hist_info,
	TP_PROTO(unsigned int total_count, u64 total_residency_us,
		 u64 busy_us, unsigned int busy_cnt,
		 u64 total_us, u64 last_run_us,
		 unsigned int min_residency_us, unsigned int max_residency_us,
		 unsigned int failed_count, unsigned int thresh, s64 stime),

	TP_ARGS(total_count, total_residency_us, busy_us, busy_cnt, total_us, last_run_us,
		min_residency_us, max_residency_us, failed_count, thresh, stime),

	TP_STRUCT__entry(
		__field(unsigned int, total_count)
		__field(unsigned int, busy_cnt)
		__field(u64, total_residency_us)
		__field(u64, busy_us)
		__field(u64, total_us)
		__field(u64, last_run_us)
		__field(unsigned int, min_residency_us)
		__field(unsigned int, max_residency_us)
		__field(unsigned int, failed_count)
		__field(unsigned int, thresh)
		__field(s64, stime)
		),

	TP_fast_assign(
		__entry->total_count = total_count;
		__entry->total_residency_us = total_residency_us;
		__entry->busy_us = busy_us;
		__entry->busy_cnt = busy_cnt;
		__entry->total_us = total_us;
		__entry->last_run_us = last_run_us;
		__entry->min_residency_us = min_residency_us;
		__entry->max_residency_us = max_residency_us;
		__entry->failed_count = failed_count;
		__entry->thresh = thresh;
		__entry->stime = stime;
		),

	TP_printk("total_count %u, total_idle %llu, busy_count %u, busy idle %llu, total_time %llu, last_run_us %llu, residency_us min %u max %u, failed_count %u, thresh %u, curr_time %lld",
		__entry->total_count,
		__entry->total_residency_us,
		__entry->busy_cnt,
		__entry->busy_us,
		__entry->total_us,
		__entry->last_run_us,
		__entry->min_residency_us,
		__entry->max_residency_us,
		__entry->failed_count,
		__entry->thresh,
		__entry->stime
		)
);


TRACE_EVENT(xr_menu_hrtimerout,
	TP_PROTO(unsigned int cpu, int wakeup_status, unsigned int hrtime_us),

	TP_ARGS(cpu, wakeup_status, hrtime_us),

	TP_STRUCT__entry(
		__field(unsigned int, cpu)
		__field(int, wakeup_status)
		__field(unsigned int, hrtime_us)
		),

	TP_fast_assign(
		__entry->cpu = cpu;
		__entry->wakeup_status = wakeup_status;
		__entry->hrtime_us = hrtime_us;
		),

	TP_printk(" cpu=%u, wakeup_status = %d hrtime_us %d",
		__entry->cpu,
		__entry->wakeup_status,
		__entry->hrtime_us
		)
);


TRACE_EVENT(xr_menu_select,
	TP_PROTO(unsigned int cpu, int last_state_idx,
		 unsigned int predicted_us, int latency_req, int early_wakeup_status,
		 int org_state_idx, unsigned int next_wakeup_us, unsigned int hrtime_us,
		 s64 delta_next_us, unsigned int expected_interval, unsigned int typical_interval),

	TP_ARGS(cpu, last_state_idx, predicted_us, latency_req, early_wakeup_status,
		org_state_idx, next_wakeup_us, hrtime_us, delta_next_us,
		expected_interval, typical_interval),

	TP_STRUCT__entry(
		__field(unsigned int, cpu)
		__field(int, predicted_state)
		__field(unsigned int, predicted_us)
		__field(int, latency_req)
		__field(int, early_wakeup_status)
		__field(int, org_state_idx)
		__field(unsigned int, next_wakeup_us)
		__field(unsigned int, hrtime_us)
		__field(s64, delta_next_us)
		__field(unsigned int, expected_interval)
		__field(unsigned int, typical_interval)
		),

	TP_fast_assign(
		__entry->cpu = cpu;
		__entry->predicted_state = last_state_idx;
		__entry->predicted_us = predicted_us;
		__entry->latency_req = latency_req;
		__entry->early_wakeup_status = early_wakeup_status;
		__entry->org_state_idx = org_state_idx;
		__entry->next_wakeup_us = next_wakeup_us;
		__entry->hrtime_us = hrtime_us;
		__entry->delta_next_us = delta_next_us;
		__entry->expected_interval = expected_interval;
		__entry->typical_interval = typical_interval;
		),

	TP_printk("cpu %u, selected %d, origin %d, predicted %u, expected %u, typical %u, next_timer %u, next_tick %lld, latency_req %d, wakeup_status %d, hrtime_us %u",
		  __entry->cpu,
		  __entry->predicted_state,
		  __entry->org_state_idx,
		  __entry->predicted_us,
		  __entry->expected_interval,
		  __entry->typical_interval,
		  __entry->next_wakeup_us,
		  __entry->delta_next_us,
		  __entry->latency_req,
		  __entry->early_wakeup_status,
		  __entry->hrtime_us
		)
);


TRACE_EVENT(idle_stat,
	TP_PROTO(unsigned int cpu, int sel, int entr, int reason),

	TP_ARGS(cpu, sel, entr, reason),

	TP_STRUCT__entry(
		__field(unsigned int, cpu)
		__field(int, sel)
		__field(int, entr)
		__field(int, reason)
		),

	TP_fast_assign(
		__entry->cpu = cpu;
		__entry->sel = sel;
		__entry->entr = entr;
		__entry->reason = reason;
		),

	TP_printk("cpu%u selected %u entered %u reason %u",
			__entry->cpu, __entry->sel, __entry->entr, __entry->reason)
);

#endif


#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../../xring-modules/drivers/cpuidle
#define TRACE_INCLUDE_FILE trace

#include <trace/define_trace.h>
