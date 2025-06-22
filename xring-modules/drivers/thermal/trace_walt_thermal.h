/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM walt_thermal

#if !defined(_TRACE_WALT_THERMAL_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_WALT_THERMAL_H

#include <linux/tracepoint.h>

#define THRES_NUM		(2)

TRACE_EVENT(walt_temp_check,

	TP_PROTO(int ret, int temp, unsigned int delay,  unsigned int *thres),

	TP_ARGS(ret, temp, delay, thres),

	TP_STRUCT__entry(
		__field(int,			ret)
		__field(int,			temp)
		__field(u32,			delay)
		__array(u32,	thres, THRES_NUM)
	),

	TP_fast_assign(
		__entry->ret		= ret;
		__entry->temp		= temp;
		__entry->delay		= delay;
		memcpy(__entry->thres, thres, THRES_NUM * sizeof(u32));
	),

	TP_printk("ret=%d temp=%d delay=%u trip=%u hyst=%u",
		__entry->ret, __entry->temp, __entry->delay,
		__entry->thres[0], __entry->thres[1])
);


#endif /* _TRACE_WALT_THERMAL_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../../xring-modules/drivers/thermal

#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE trace_walt_thermal

#include <trace/define_trace.h>
