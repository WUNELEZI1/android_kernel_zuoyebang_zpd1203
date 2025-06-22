/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM rt_tracker

#if !defined(_TRACE_RT_TRACKER_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_RT_TRACKER_H

#include <linux/tracepoint.h>
#include <linux/version.h>

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../../../xring-modules/drivers/cpu_sched/walt/rt_tracker

#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE trace_rt_tracker

DECLARE_EVENT_CLASS(sched_process_template,

	TP_PROTO(struct task_struct *p),

	TP_ARGS(p),

	TP_STRUCT__entry(
		__array(char, comm, TASK_COMM_LEN)
		__field(pid_t, pid)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid = p->pid;
	),

	TP_printk("comm=%s pid=%d",
		  __entry->comm, __entry->pid)
);

DEFINE_EVENT(sched_process_template, add_rthread,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

DEFINE_EVENT(sched_process_template, remove_rthread,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

DEFINE_EVENT(sched_process_template, stop_rt_tracker,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

DECLARE_EVENT_CLASS(time_template,

	TP_PROTO(u64 interval),

	TP_ARGS(interval),

	TP_STRUCT__entry(
		__field(u64, interval)
	),

	TP_fast_assign(
		__entry->interval = interval;
	),

	TP_printk("interval=%llu",
		  __entry->interval)
);

DEFINE_EVENT(time_template, rollover_frame,
	     TP_PROTO(u64 interval),
	     TP_ARGS(interval));

DEFINE_EVENT(time_template, get_rt_info,
	     TP_PROTO(u64 interval),
	     TP_ARGS(interval));

DEFINE_EVENT(time_template, try_update_wake,
	     TP_PROTO(u64 interval),
	     TP_ARGS(interval));

DEFINE_EVENT(time_template, insert_update_wake,
	     TP_PROTO(u64 interval),
	     TP_ARGS(interval));

TRACE_EVENT(update_rthread_wake_count,

	TP_PROTO(struct task_struct *waker_task,
		 struct task_struct *wakee_task,
		 int waker_cnt),

	TP_ARGS(waker_task, wakee_task, waker_cnt),

	TP_STRUCT__entry(
		__array(char, waker_comm, TASK_COMM_LEN)
		__field(pid_t, waker_pid)
		__array(char, wakee_comm, TASK_COMM_LEN)
		__field(pid_t, wakee_pid)
		__field(int, waker_cnt)
	),

	TP_fast_assign(
		memcpy(__entry->waker_comm, waker_task->comm, TASK_COMM_LEN);
		__entry->waker_pid	= waker_task->pid;
		memcpy(__entry->wakee_comm, wakee_task->comm, TASK_COMM_LEN);
		__entry->wakee_pid	= wakee_task->pid;
		__entry->waker_cnt	= waker_cnt;
	),

	TP_printk("waker:comm=%s pid=%d wakee:comm=%s pid=%d wake_cnt=%d",
		  __entry->waker_comm, __entry->waker_pid,
		  __entry->wakee_comm, __entry->wakee_pid,
		  __entry->waker_cnt)
);

TRACE_EVENT(update_rthread_load,

	TP_PROTO(struct task_struct *task, u64 load, u64 exec),

	TP_ARGS(task, load, exec),

	TP_STRUCT__entry(
		__array(char, comm, TASK_COMM_LEN)
		__field(pid_t, pid)
		__field(u64, load)
		__field(u64, exec)
	),

	TP_fast_assign(
		memcpy(__entry->comm, task->comm, TASK_COMM_LEN);
		__entry->pid	= task->pid;
		__entry->load	= load;
		__entry->exec	= exec;
	),

	TP_printk("comm=%s pid=%d load=%llu exec=%llu",
		  __entry->comm, __entry->pid, __entry->load,
		  __entry->exec)
);

#endif /* _TRACE_RT_TRACKER_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
