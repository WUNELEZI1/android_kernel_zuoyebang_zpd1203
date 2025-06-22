/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM audio_trace

#if !defined(__AUDIO_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define __AUDIO_TRACE_H

#include <linux/types.h>
#include <linux/tracepoint.h>
#include <asm/byteorder.h>

#define TRACE_MSG_MAX 500

DECLARE_EVENT_CLASS(audio_trace_class,
	TP_PROTO(struct va_format *vaf),
	TP_ARGS(vaf),
	TP_STRUCT__entry(__dynamic_array(char, msg, TRACE_MSG_MAX)),
	TP_fast_assign(
		vsnprintf(__get_str(msg), TRACE_MSG_MAX, vaf->fmt, *vaf->va);
	),
	TP_printk("%s", __get_str(msg))
);


DEFINE_EVENT(audio_trace_class, audio_event,
	TP_PROTO(struct va_format *vaf),
	TP_ARGS(vaf)
);
#endif /* __AUDIO_TRACE_H */

/* this part has to be here */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ./common

#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE audio-trace

#include <trace/define_trace.h>

