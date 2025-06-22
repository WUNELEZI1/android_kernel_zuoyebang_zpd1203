/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM venc

#if !defined(_TRACE_VENC_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_VENC_H

#include <linux/tracepoint.h>
#define COMM_LEN (50)

DECLARE_EVENT_CLASS(venc_event_class,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id),

	TP_STRUCT__entry(
		__array(char, comm, COMM_LEN)
		__field(u64, cmdbuf_id)
	),

	TP_fast_assign(
		strncpy(__entry->comm, comm, COMM_LEN);
		__entry->cmdbuf_id = cmdbuf_id;

	),

	TP_printk("comm=%s  index=%llu",
		  __entry->comm,
		  __entry->cmdbuf_id
	)
)

DEFINE_EVENT(venc_event_class, process_vce_slice_irq,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

DEFINE_EVENT(venc_event_class, proccess_vce_segment_polling_timeout_irq,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

DEFINE_EVENT(venc_event_class, process_abnormal_err_irq,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

DEFINE_EVENT(venc_event_class, process_ufbc_abn_irq,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

DEFINE_EVENT(venc_event_class, send_slice_msg_to_xplayer,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

DEFINE_EVENT(venc_event_class, link_and_run_cmdbuf,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

DEFINE_EVENT(venc_event_class, wait_cmdbuf_ready,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

DEFINE_EVENT(venc_event_class, hantrovcmd_isr_hw_abort,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

DEFINE_EVENT(venc_event_class, hantrovcmd_isr_hw_timeout,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

DEFINE_EVENT(venc_event_class, hantrovcmd_isr_cmd_err,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

DEFINE_EVENT(venc_event_class, hantrovcmd_isr_finish_success,
	TP_PROTO(char *comm, u64 cmdbuf_id),
	TP_ARGS(comm, cmdbuf_id)
);

#endif /* if !defined(_TRACE_VENC_H) || defined(TRACE_HEADER_MULTI_READ) */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE venc_trace
/* This part must be outside protection */
#include <trace/define_trace.h>
