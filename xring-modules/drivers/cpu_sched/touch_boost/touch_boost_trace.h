/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM touch_boost

#if !defined(_TOUCH_BOOST_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TOUCH_BOOST_TRACE_H

#include <linux/tracepoint.h>

TRACE_EVENT(touch_boost_start,
	TP_PROTO(u32 hwirq, int irq, u32 lim_ns, int cpu, u32 boost_type),
	TP_ARGS(hwirq, irq, lim_ns, cpu, boost_type),
	TP_STRUCT__entry(
		__field(u32, hwirq)
		__field(int, irq)
		__field(u32, lim_ns)
		__field(int, cpu)
		__field(u32, boost_type)
	),
	TP_fast_assign(
		__entry->hwirq = hwirq;
		__entry->irq = irq;
		__entry->lim_ns = lim_ns;
		__entry->cpu = cpu;
		__entry->boost_type = boost_type;
	),
	TP_printk(
		"hwirq %u, irq %d, lim_ns %u, cpu %d, boost %s",
		__entry->hwirq, __entry->irq, __entry->lim_ns,
		__entry->cpu, __entry->boost_type ? "freq" : "idle"
	)
);

TRACE_EVENT(touch_boost_end,
	TP_PROTO(int cpu, u32 boost_type),
	TP_ARGS(cpu, boost_type),
	TP_STRUCT__entry(
		__field(int, cpu)
		__field(u32, boost_type)
	),
	TP_fast_assign(
		__entry->cpu = cpu;
		__entry->boost_type = boost_type;
	),
	TP_printk(
		"cpu %d, unboost %s", __entry->cpu, __entry->boost_type ? "freq" : "idle"
	)
);

#endif /* _TOUCH_BOOST_TRACE_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE touch_boost_trace

/* This part must be outside protection */
#include <trace/define_trace.h>
