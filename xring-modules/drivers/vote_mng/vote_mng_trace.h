/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2023, X-Ring technologies Inc., All rights reserved. */

#include <linux/tracepoint.h>
#undef TRACE_SYSTEM

#define TRACE_SYSTEM xr_votemng
#if !defined(_TRACE_XR_VOTE_MNG_H) || defined(TRACE_HEADER_MULTI_READ)

#define _TRACE_XR_VOTE_MNG_H

TRACE_EVENT(vote_mng_mode1,
	    TP_PROTO(unsigned int pid, char *comm, unsigned int arb, unsigned int bidirection,
		     unsigned int *msg, signed long ktime),
	    TP_ARGS(pid, comm, arb, bidirection, msg, ktime),
	    TP_STRUCT__entry(
		    __field(unsigned int, pid)
		    __field(char *, comm)
		    __field(unsigned int, arb)
		    __field(unsigned int, bidirection)
		    __field(unsigned int *, msg)
		    __field(signed long, ktime)
	    ),
	    TP_fast_assign(
		    __entry->pid		= pid;
		    __entry->comm		= comm;
		    __entry->arb		= arb;
		    __entry->bidirection	= bidirection;
		    __entry->msg		= msg;
		    __entry->ktime		= ktime;
	    ),
	    TP_printk("vote_mng_mode1 pid=%u comm=%s arb=%u time=%ld, direction=%u, msg0=%u, msg1=%u, msg2=%u, msg3=%u",
		      __entry->pid,
		      __entry->comm,
		      __entry->arb,
		      __entry->ktime,
		      __entry->bidirection,
		      __entry->msg[0],
		      __entry->msg[1],
		      __entry->msg[2],
		      __entry->msg[3])
);

TRACE_EVENT(vote_mng_mode2,
	    TP_PROTO(unsigned int pid, char *comm, unsigned int arb, unsigned int ch, unsigned int type,
		     unsigned int val, signed long ktime),
	    TP_ARGS(pid, comm, arb, ch, type, val, ktime),
	    TP_STRUCT__entry(
		    __field(unsigned int, pid)
		    __field(char *, comm)
		    __field(unsigned int, arb)
		    __field(unsigned int, ch)
		    __field(unsigned int, type)
		    __field(unsigned int, val)
		    __field(signed long, ktime)
	    ),
	    TP_fast_assign(
		    __entry->pid		= pid;
		    __entry->comm		= comm;
		    __entry->arb		= arb;
		    __entry->ch			= ch;
		    __entry->type		= type;
		    __entry->val		= val;
		    __entry->ktime		= ktime;
	    ),
	    TP_printk("vote_mng_mode2 pid=%u comm=%s arb=%u time=%ld, ch=%u, tyep=%u, val=%u",
		      __entry->pid,
		      __entry->comm,
		      __entry->arb,
		      __entry->ktime,
		      __entry->ch,
		      __entry->type,
		      __entry->val)
);

TRACE_EVENT(vote_mng_mode3,
	    TP_PROTO(unsigned int pid, char *comm, unsigned int arb, unsigned int ch,
		     unsigned int val, unsigned int bidirection, signed long ktime),
	    TP_ARGS(pid, comm, arb, ch, val, bidirection, ktime),
	    TP_STRUCT__entry(
		    __field(unsigned int, pid)
		    __field(char *, comm)
		    __field(unsigned int, arb)
		    __field(unsigned int, ch)
		    __field(unsigned int, val)
		    __field(unsigned int, bidirection)
		    __field(signed long, ktime)
	    ),
	    TP_fast_assign(
		    __entry->pid		= pid;
		    __entry->comm		= comm;
		    __entry->arb		= arb;
		    __entry->ch			= ch;
		    __entry->val		= val;
		    __entry->bidirection	= bidirection;
		    __entry->ktime		= ktime;
	    ),
	    TP_printk("vote_mng_mode1 pid=%u comm=%s arb=%u time=%ld, ch=%u, val=%u, direction=%u",
		      __entry->pid,
		      __entry->comm,
		      __entry->arb,
		      __entry->ktime,
		      __entry->ch,
		      __entry->val,
		      __entry->bidirection)
);

#endif
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../../xring-modules/drivers/vote_mng
#define TRACE_INCLUDE_FILE vote_mng_trace

#include <trace/define_trace.h>
