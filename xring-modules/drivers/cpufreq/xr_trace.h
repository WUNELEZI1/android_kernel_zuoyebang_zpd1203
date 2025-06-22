/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#include <linux/tracepoint.h>
#undef TRACE_SYSTEM

#define TRACE_SYSTEM cpufreq_xres
#if !defined(_TRACE_XR_H) || defined(TRACE_HEADER_MULTI_READ)

#define _TRACE_XR_H

TRACE_EVENT(freq_qos_update,
	    TP_PROTO(int cpufreq, unsigned int pid, char *comm, unsigned int cpu, const char *type_str),
	    TP_ARGS(cpufreq, pid, comm, cpu, type_str),
	    TP_STRUCT__entry(
		    __field(int, cpufreq)
		    __field(unsigned int, pid)
		    __field(char *, comm)
		    __field(unsigned int, cpu)
		    __field(const char *, type_str)
	    ),
	    TP_fast_assign(
		    __entry->cpufreq		= cpufreq;
		    __entry->pid		= pid;
		    __entry->comm		= comm;
		    __entry->cpu		= cpu;
		    __entry->type_str		= type_str;
	    ),
	    TP_printk("freq_qos_update cpufreq=%d pid=%u comm=%s policy=%u type=%s",
		      __entry->cpufreq,
		      __entry->pid,
		      __entry->comm,
		      __entry->cpu,
		      __entry->type_str)
);

#endif
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../../xring-modules/drivers/cpufreq
#define TRACE_INCLUDE_FILE xr_trace

#include <trace/define_trace.h>
