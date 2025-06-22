/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#include <linux/tracepoint.h>
#undef TRACE_SYSTEM

#define TRACE_SYSTEM cpufreq_xres
#if !defined(_TRACE_XRES_H) || defined(TRACE_HEADER_MULTI_READ)

#define _TRACE_XRES_H

TRACE_EVENT(xresgov_next_freq,
	    TP_PROTO(unsigned int cpu, unsigned long util, unsigned long max, unsigned int raw_freq,
		     unsigned int freq, unsigned int policy_min_freq, unsigned int policy_max_freq,
		     unsigned int cached_raw_freq, bool need_freq_update,
		     unsigned int driving_cpu, unsigned int reason),
	    TP_ARGS(cpu, util, max, raw_freq, freq, policy_min_freq, policy_max_freq,
		    cached_raw_freq, need_freq_update, driving_cpu, reason),
	    TP_STRUCT__entry(
		    __field(unsigned int, cpu)
		    __field(unsigned long, util)
		    __field(unsigned long, max)
		    __field(unsigned int, raw_freq)
		    __field(unsigned int, freq)
		    __field(unsigned int, policy_min_freq)
		    __field(unsigned int, policy_max_freq)
		    __field(unsigned int, cached_raw_freq)
		    __field(bool, need_freq_update)
		    __field(unsigned int, rt_util)
		    __field(unsigned int, driving_cpu)
		    __field(unsigned int, reason)
	    ),
	    TP_fast_assign(
		    __entry->cpu		= cpu;
		    __entry->util		= util;
		    __entry->max		= max;
		    __entry->raw_freq		= raw_freq;
		    __entry->freq		= freq;
		    __entry->policy_min_freq	= policy_min_freq;
		    __entry->policy_max_freq	= policy_max_freq;
		    __entry->cached_raw_freq	= cached_raw_freq;
		    __entry->need_freq_update	= need_freq_update;
		    __entry->rt_util		= cpu_util_rt(cpu_rq(cpu));
		    __entry->driving_cpu	= driving_cpu;
		    __entry->reason		= reason;
	    ),
	    TP_printk("cpu=%u util=%lu max=%lu raw_freq=%u freq=%u policy_min_freq=%u policy_max_freq=%u cached_raw_freq=%u need_update=%d rt_util=%u driv_cpu=%u reason=0x%x",
		      __entry->cpu,
		      __entry->util,
		      __entry->max,
		      __entry->raw_freq,
		      __entry->freq,
		      __entry->policy_min_freq,
		      __entry->policy_max_freq,
		      __entry->cached_raw_freq,
		      __entry->need_freq_update,
		      __entry->rt_util,
		      __entry->driving_cpu,
		      __entry->reason)
);

TRACE_EVENT(xresgov_boost_freq,
	    TP_PROTO(unsigned int cpu, unsigned int freq,  unsigned int flag),
	    TP_ARGS(cpu, freq, flag),
	    TP_STRUCT__entry(
		    __field(unsigned int, cpu)
		    __field(unsigned int, freq)
		    __field(unsigned int, flag)
	    ),
	    TP_fast_assign(
		    __entry->cpu		= cpu;
		    __entry->freq		= freq;
		    __entry->flag		= flag;
	    ),
	    TP_printk("cpu=%u boost_freq=%u  boost_type=0x%x",
		      __entry->cpu,
		      __entry->freq,
		      __entry->flag)
);


TRACE_EVENT(xresgov_trigger,
	    TP_PROTO(unsigned int cpu,  unsigned int flag),
	    TP_ARGS(cpu, flag),
	    TP_STRUCT__entry(
		    __field(unsigned int, cpu)
		    __field(unsigned int, flag)
	    ),
	    TP_fast_assign(
		    __entry->cpu		= cpu;
		    __entry->flag		= flag;
	    ),
	    TP_printk("cpu=%u flag=0x%x", __entry->cpu, __entry->flag)
);

TRACE_EVENT(xresgov_io_wait_boost,
	    TP_PROTO(unsigned int cpu, unsigned long  util, unsigned long boost_util,
		    unsigned int boost_freq),
	    TP_ARGS(cpu, util, boost_util, boost_freq),
	    TP_STRUCT__entry(
		    __field(unsigned int, cpu)
		    __field(unsigned long, util)
		    __field(unsigned long, boost_util)
		    __field(unsigned int, boost_freq)
	    ),
	    TP_fast_assign(
		    __entry->cpu		= cpu;
		    __entry->util		= util;
		    __entry->boost_util		= boost_util;
		    __entry->boost_freq		= boost_freq;
	    ),
	    TP_printk("cpu=%u util=%lu  boost_util=%lu boost_freq = %u",
		      __entry->cpu,
		      __entry->util,
		      __entry->boost_util,
		      __entry->boost_freq)
);

TRACE_EVENT(xresgov_cpu_util,
	    TP_PROTO(unsigned int cpu, unsigned long util, unsigned long max_util, unsigned int reason),
	    TP_ARGS(cpu, util, max_util, reason),
	    TP_STRUCT__entry(
		    __field(unsigned int, cpu)
		    __field(unsigned long, util)
		    __field(unsigned long, max_util)
		    __field(unsigned int, reason)
	    ),
	    TP_fast_assign(
		    __entry->cpu		= cpu;
		    __entry->util		= util;
		    __entry->max_util		= max_util;
		    __entry->reason		= reason;
	    ),
	    TP_printk("cpu=%u util=%lu max_util=%lu reason=%x",
		      __entry->cpu,
		      __entry->util,
		      __entry->max_util,
		      __entry->reason)
);

TRACE_EVENT(xresgov_cpu_limits_time,
	    TP_PROTO(unsigned int cpu, unsigned long now, unsigned long limit_time,
		     unsigned long start, unsigned long delta_ns,
		     unsigned int cur_freq, unsigned int next_freq),
	    TP_ARGS(cpu, now, limit_time, start, delta_ns, cur_freq, next_freq),
	    TP_STRUCT__entry(
		    __field(unsigned int, cpu)
		    __field(unsigned long, now)
		    __field(unsigned long, limit_time)
		    __field(unsigned long, start)
		    __field(unsigned long, delta_ns)
		    __field(unsigned int, cur_freq)
		    __field(unsigned int, next_freq)
	    ),
	    TP_fast_assign(
		    __entry->cpu		= cpu;
		    __entry->now		= now;
		    __entry->limit_time		= limit_time;
		    __entry->start		= start;
		    __entry->delta_ns	= delta_ns;
		    __entry->cur_freq	= cur_freq;
		    __entry->next_freq	= next_freq;
	    ),
	    TP_printk("cpu=%u now=%lu limit_time=%lu start_time=%lu delat_ns=%lu cur_freq=%u next_freq=%u",
		      __entry->cpu,
		      __entry->now,
		      __entry->limit_time,
		      __entry->start,
		      __entry->delta_ns,
		      __entry->cur_freq,
		      __entry->next_freq)
);

TRACE_EVENT(xresgov_cpu_load,
	    TP_PROTO(unsigned int cpu_load, unsigned long  util, unsigned long curfreq_util,
		     unsigned int cur_freq, unsigned int target_freq, unsigned int cpu),
	    TP_ARGS(cpu_load, util, curfreq_util, cur_freq, target_freq, cpu),
	    TP_STRUCT__entry(
		    __field(unsigned int, cpu_load)
		    __field(unsigned long, util)
		    __field(unsigned long, curfreq_util)
		    __field(unsigned int, cur_freq)
		    __field(unsigned int, target_freq)
		    __field(unsigned int, cpu)
	    ),
	    TP_fast_assign(
		    __entry->cpu_load		= cpu_load;
		    __entry->util		= util;
		    __entry->curfreq_util	= curfreq_util;
		    __entry->cur_freq		= cur_freq;
		    __entry->target_freq	= target_freq;
		    __entry->cpu		= cpu;
	    ),
	    TP_printk("cpu_load=%u util=%lu curfreq_util=%lu cur_freq=%u target_freq=%u cpu=%u",
		      __entry->cpu_load,
		      __entry->util,
		      __entry->curfreq_util,
		      __entry->cur_freq,
		      __entry->target_freq,
		      __entry->cpu)
);

#endif
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../../xring-modules/drivers/cpufreq
#define TRACE_INCLUDE_FILE xres_trace

#include <trace/define_trace.h>
