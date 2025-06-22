/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM pa_trace

#if !defined(__PA_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define __PA_TRACE_H

#include <linux/types.h>
#include <linux/tracepoint.h>
#include <asm/byteorder.h>

#define TRACE_MSG_MAX 500

DECLARE_EVENT_CLASS(pa_trace_class,
	TP_PROTO(struct va_format *vaf),
	TP_ARGS(vaf),
	TP_STRUCT__entry(__dynamic_array(char, msg, TRACE_MSG_MAX)),
	TP_fast_assign(
		vsnprintf(__get_str(msg), TRACE_MSG_MAX, vaf->fmt, *vaf->va);
	),
	TP_printk("%s", __get_str(msg))
);


DEFINE_EVENT(pa_trace_class, pa_event,
	TP_PROTO(struct va_format *vaf),
	TP_ARGS(vaf)
);

#endif /* __PA_TRACE_H */

#ifndef __PA_DBG_TRACE_H_
#define __PA_DBG_TRACE_H_
void pa_dbg_trace(void (*trace)(struct va_format *), const char *fmt,
		...);
#endif /* __PA_DBG_TRACE_H_ */

/* this part has to be here */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .

#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE pa-trace

#include <trace/define_trace.h>

