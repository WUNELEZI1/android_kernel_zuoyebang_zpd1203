/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM dpu

#if !defined(_TRACE_DPU_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_DPU_H

#include <linux/tracepoint.h>

#define COMM_LEN (50)

DECLARE_EVENT_CLASS(dpu_comm_class,
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

DECLARE_EVENT_CLASS(dpu_comm_class_v2,
	TP_PROTO(char *comm, u64 frame_no),
	TP_ARGS(comm, frame_no),

	TP_STRUCT__entry(
		__array(char, comm, COMM_LEN)
		__field(u64, frame_no)
	),

	TP_fast_assign(
		strncpy(__entry->comm, comm, COMM_LEN);
		__entry->frame_no = frame_no;

	),

	TP_printk("comm=%s, frame_no=%llu",
		  __entry->comm,
		  __entry->frame_no
	)
)

DECLARE_EVENT_CLASS(dpu_irq_class,
	TP_PROTO(char *irq_name, u64 intr_type),
	TP_ARGS(irq_name, intr_type),

	TP_STRUCT__entry(
		__array(char, irq_name, COMM_LEN)
		__field(u64, intr_type)
	),

	TP_fast_assign(
		strncpy(__entry->irq_name, irq_name, COMM_LEN);
		__entry->intr_type = intr_type;

	),

	TP_printk("comm=%s, intr_type=%llu",
		  __entry->irq_name,
		  __entry->intr_type
	)
)

DECLARE_EVENT_CLASS(dpu_perf_class,
	TP_PROTO(char *comm, u64 aclk_rate, u64 mclk_rate),
	TP_ARGS(comm, aclk_rate, mclk_rate),

	TP_STRUCT__entry(
		__array(char, comm, COMM_LEN)
		__field(u64, aclk_rate)
		__field(u64, mclk_rate)
	),

	TP_fast_assign(
		strncpy(__entry->comm, comm, COMM_LEN);
		__entry->aclk_rate = aclk_rate;
		__entry->mclk_rate = mclk_rate;

	),

	TP_printk("perf_update=%s, aclk_rate=%llu, mclk_rate=%llu",
		  __entry->comm,
		  __entry->aclk_rate,
		  __entry->mclk_rate
	)
)

DECLARE_EVENT_CLASS(dpu_feature_class,
	TP_PROTO(char *feature_name, u64 info),
	TP_ARGS(feature_name, info),

	TP_STRUCT__entry(
		__array(char, feature_name, COMM_LEN)
		__field(u64, info)
	),

	TP_fast_assign(
		strncpy(__entry->feature_name, feature_name, COMM_LEN);
		__entry->info = info;

	),

	TP_printk("comm=%s, info=%llu",
		  __entry->feature_name,
		  __entry->info
	)
)

DEFINE_EVENT(dpu_comm_class, dpu_kms_fence_force_signal,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(dpu_comm_class_v2, dpu_kms_commit_work,
	TP_PROTO(char *comm, u64 frame_no),
	TP_ARGS(comm, frame_no)
);

DEFINE_EVENT(dpu_comm_class, dpu_kms_wait_for_fences,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(dpu_comm_class, dpu_drm_atomic_commit_tail,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(dpu_comm_class, dpu_hw_dvfs_enable,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(dpu_comm_class, dpu_exception_dma_dbg_irq_dump,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(dpu_comm_class, dpu_hw_dvfs_disable,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(dpu_irq_class, dsi_irq_handler,
	TP_PROTO(char *irq_name, u64 intr_type),
	TP_ARGS(irq_name, intr_type)
);

DEFINE_EVENT(dpu_irq_class, dpu_wb_irq_handler,
	TP_PROTO(char *irq_name, u64 intr_type),
	TP_ARGS(irq_name, intr_type)
);

DEFINE_EVENT(dpu_irq_class, dpu_cmd_idle_enter,
	TP_PROTO(char *irq_name, u64 intr_type),
	TP_ARGS(irq_name, intr_type)
);

DEFINE_EVENT(dpu_irq_class, dpu_cmd_idle_exit,
	TP_PROTO(char *irq_name, u64 intr_type),
	TP_ARGS(irq_name, intr_type)
);

DEFINE_EVENT(dpu_perf_class, _do_dpu_core_perf_update,
	TP_PROTO(char *clk, u64 aclk_rate, u64 mclk_rate),
	TP_ARGS(clk, aclk_rate, mclk_rate)
);

DEFINE_EVENT(dpu_irq_class, dp_irq_handler,
	TP_PROTO(char *irq_name, u64 id),
	TP_ARGS(irq_name, id)
);

DEFINE_EVENT(dpu_comm_class, dsi_panel_esd_irq_ctrl,
	TP_PROTO(char *comm),
	TP_ARGS(comm)
);

DEFINE_EVENT(dpu_irq_class, dsi_display_esd_irq_handle,
	TP_PROTO(char *irq_name, u64 id),
	TP_ARGS(irq_name, id)
);

DEFINE_EVENT(dpu_feature_class, dpu_crtc_dsc_update,
	TP_PROTO(char *comm, u64 damage_height),
	TP_ARGS(comm, damage_height)
);

#endif /* if !defined(_TRACE_DPU_H) || defined(TRACE_HEADER_MULTI_READ) */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE dpu_trace
/* This part must be outside protection */
#include <trace/define_trace.h>
