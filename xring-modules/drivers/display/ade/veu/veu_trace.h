/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM veu

#if !defined(_TRACE_VEU_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_VEU_H

#include <linux/tracepoint.h>

#define COMM_LEN (50)

DECLARE_EVENT_CLASS(veu_comm_class,
	TP_PROTO(char *comm),
	TP_ARGS(comm),

	TP_STRUCT__entry(
		__array(char, comm, COMM_LEN)
	),

	TP_fast_assign(
		strncpy(__entry->comm, comm, COMM_LEN);

	),

	TP_printk("comm=%s",
		  __entry->comm
	)
)

DEFINE_EVENT(veu_comm_class, veu_process,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(veu_comm_class, veu_on,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(veu_comm_class, veu_config,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(veu_comm_class, veu_get_iova_by_sharefd,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(veu_comm_class, veu_postprocess,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(veu_comm_class, veu_mmu_unmap_layer_buffer,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

#endif /* if !defined(_TRACE_VEU_H) || defined(TRACE_HEADER_MULTI_READ) */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../xring-modules/drivers/display/ade/veu
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE veu_trace
/* This part must be outside protection */
#include <trace/define_trace.h>
