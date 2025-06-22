/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM prior_affinity_config

#if !defined(_PRIOR_AFFINITY_CONFIG_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _PRIOR_AFFINITY_CONFIG_TRACE_H

#include <linux/tracepoint.h>

TRACE_EVENT(prior_affinity_config_show,
	TP_PROTO(pid_t pid, struct cpumask *prior_mask, struct cpumask *user_mask, bool preempt),
	TP_ARGS(pid, prior_mask, user_mask, preempt),
	TP_STRUCT__entry(
		__field(pid_t,		pid)
		__field(unsigned int,	prior_mask)
		__field(unsigned int,	user_mask)
		__field(bool,		preempt)
	),
	TP_fast_assign(
		__entry->pid		= pid;
		__entry->prior_mask	= prior_mask->bits[0];
		__entry->user_mask	= user_mask->bits[0];
		__entry->preempt	= preempt;
	),
	TP_printk(
		"pid=%d prior_mask=0x%x user_mask=0x%x preempt=%u",
		__entry->pid, __entry->prior_mask, __entry->user_mask, __entry->preempt
	)
);

TRACE_EVENT(setaffinity_show,
	TP_PROTO(
		struct task_struct *curr, struct task_struct *p,
		const struct cpumask *in_mask, int retval, struct cpumask *prior_mask),
	TP_ARGS(curr, p, in_mask, retval, prior_mask),
	TP_STRUCT__entry(
		__field(pid_t,		curr_pid)
		__array(char,		curr_comm, TASK_COMM_LEN)
		__field(pid_t,		pid)
		__array(char,		comm, TASK_COMM_LEN)
		__field(unsigned int,	in_mask)
		__field(int,		retval)
		__field(unsigned int,	now_mask)
		__field(unsigned int,	prior_mask)
		__field(unsigned int,	user_mask)
	),
	TP_fast_assign(
		__entry->curr_pid	= curr->pid;
		memcpy(__entry->curr_comm, curr->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->in_mask	= in_mask ? in_mask->bits[0] : 0;
		__entry->retval		= retval;
		__entry->now_mask	= p->cpus_mask.bits[0];
		__entry->prior_mask	= prior_mask ? prior_mask->bits[0] : 0;
		__entry->user_mask	= p->user_cpus_ptr ? p->user_cpus_ptr->bits[0] : 0;
	),
	TP_printk(
		"pid %d (%s) set affinity of pid %d (%s) to 0x%x with retval %d. The object task: now_mask=0x%x prior_mask=0x%x user_mask=0x%x",
		__entry->curr_pid, __entry->curr_comm, __entry->pid, __entry->comm,
		__entry->in_mask, __entry->retval,
		__entry->now_mask, __entry->prior_mask, __entry->user_mask
	)
);
#endif /* _PRIOR_AFFINITY_CONFIG_TRACE_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../../xring-modules/drivers/cpu_sched/walt
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE prior_affinity_config_trace

/* This part must be outside protection */
#include <trace/define_trace.h>
