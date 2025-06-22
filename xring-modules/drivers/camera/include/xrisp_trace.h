/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#if !defined(_XRISP_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _XRISP_TRACE_H

#undef TRACE_SYSTEM
#define TRACE_SYSTEM xrisp
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE xrisp_trace

#include <linux/tracepoint.h>

TRACE_EVENT(xrisp_cbm_tlb_status,
	TP_PROTO(const char *func, uint32_t idx, uint32_t status),
	TP_ARGS(func, idx, status),
	TP_STRUCT__entry(
		__string(func, func)
		__field(uint32_t, idx)
		__field(uint32_t, status)
	),
	TP_fast_assign(
		__assign_str(func, func);
		__entry->idx = idx;
		__entry->status = status;
	),
	TP_printk(
		"%s: table element idx %d, status %d .",
			__get_str(func), __entry->idx, __entry->status
	)
);


DECLARE_EVENT_CLASS(xrisp_cbm_trace,
	TP_PROTO(const char *func, uint32_t idx, uint32_t status),
	TP_ARGS(func, idx, status),
	TP_STRUCT__entry(
		__string(func, func)
		__field(int, idx)
		__field(uint32_t, status)
	),
	TP_fast_assign(
		__assign_str(func, func);
		__entry->idx = idx;
		__entry->status = status;
	),
	TP_printk(
		"%s: table element idx %d, status %d .",
			__get_str(func), __entry->idx, __entry->status
	)
);

DEFINE_EVENT(xrisp_cbm_trace, xrisp_cbm_tlb_get,
	TP_PROTO(const char *func, uint32_t idx, uint32_t status),
	TP_ARGS(func, idx, status));

DEFINE_EVENT(xrisp_cbm_trace, xrisp_cbm_tlb_put,
	TP_PROTO(const char *func, uint32_t idx, uint32_t status),
	TP_ARGS(func, idx, status));


#endif /* _XRISP_TRACE_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
