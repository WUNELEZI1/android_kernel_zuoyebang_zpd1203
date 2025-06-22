/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM vpu_v4l2_trace

#if !defined(_TRACE_VPU_V4L2_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_VPU_V4L2_H

#include <linux/tracepoint.h>

/*
 * Tracepoint for vsi req_buf;
 */
TRACE_EVENT(vpu_reqbuf,

	TP_PROTO(unsigned long ctx_id, unsigned int type),

	TP_ARGS(ctx_id, type),

	TP_STRUCT__entry(
		__field(	unsigned long,	ctx_id	)
		__field(	unsigned int,	type	)
	),

	TP_fast_assign(
		__entry->ctx_id	= ctx_id;
		__entry->type	= type;
	),

	TP_printk("ctx_id=%#lx type=%u ", __entry->ctx_id, __entry->type)
);

TRACE_EVENT(vpu_dec_setfmt,

	TP_PROTO(unsigned long ctx_id, unsigned int type, unsigned int planes,
				unsigned int size0, unsigned int size1, unsigned int size2),

	TP_ARGS(ctx_id, type, planes, size0, size1, size2),

	TP_STRUCT__entry(
		__field(unsigned long,	ctx_id)
		__field(unsigned int,	type)
		__field(unsigned int,	planes)
		__field(unsigned int,	size0)
		__field(unsigned int,	size1)
		__field(unsigned int,	size2)
	),

	TP_fast_assign(
		__entry->ctx_id	= ctx_id;
		__entry->type	= type;
		__entry->planes	= planes;
		__entry->size0	= size0;
		__entry->size1	= size1;
		__entry->size2	= size2;
	),

	TP_printk("ctx_id=%#lx type=%u planes=%u planes_size=%u-%u-%u",
		__entry->ctx_id, __entry->type, __entry->planes,
		__entry->size0, __entry->size1, __entry->size2)
);


#endif /* if !defined(_TRACE_VDEC_H) || defined(TRACE_HEADER_MULTI_READ) */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE vpu_v4l2_trace
/* This part must be outside protection */
#include <trace/define_trace.h>