/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#if !defined(_L3_DEVFREQ_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _L3_DEVFREQ_TRACE_H

#undef TRACE_SYSTEM
#define TRACE_SYSTEM l3_devfreq
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE l3_devfreq_trace

#include <linux/tracepoint.h>

TRACE_EVENT(l3_devfreq_range,
	TP_PROTO(unsigned long min, unsigned long max),
	TP_ARGS(min, max),
	TP_STRUCT__entry(
		__field(unsigned long, min)
		__field(unsigned long, max)
	),
	TP_fast_assign(
		__entry->min = min;
		__entry->max = max;
	),
	TP_printk(
		"min %lu, max %lu .",
		__entry->min, __entry->max
	)
);


#endif /* _L3_DEVFREQ_TRACE_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
