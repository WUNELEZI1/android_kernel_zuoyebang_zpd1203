#undef TRACE_SYSTEM
#define TRACE_SYSTEM nanosic

#if !defined(_NANOSIC_TRACE_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _NANOSIC_TRACE_H_

#include <linux/stringify.h>
#include <linux/types.h>
#include <linux/tracepoint.h>

TRACE_EVENT(tracing_mark_write,
    TP_PROTO(char trace_type, const struct task_struct *task, const char *name, int value),
    TP_ARGS(trace_type, task, name, value),
    TP_STRUCT__entry(
            __field(char, trace_type)
            __field(int, pid)
            __dynamic_array(char, trace_name, strlen(name) + 1)
            __field(int, value)
    ), 
    TP_fast_assign(
            __entry->trace_type = trace_type;
            __entry->pid = task ? task->tgid : 0;
            strncpy(__get_dynamic_array(trace_name), name, strlen(name) + 1);
            __entry->value = value;
    ),
    TP_printk("%c|%d|%s|%d", __entry->trace_type, __entry->pid,
                                __get_str(trace_name), __entry->value)
)

#define ATRACE_END() trace_tracing_mark_write('E', current, "", 0)
#define ATRACE_BEGIN(name) trace_tracing_mark_write('B', current, name, 0)
#define ATRACE_INT(name, value) trace_tracing_mark_write('C', current, name, value)

#endif /* _NANOSIC_TRACE_H_ */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../../xiaomi-modules/keyboard_driver/jinghu/nanosic_driver
/*
 * vendor/xring/android-kernel/common/include/trace/
 *      ../../../xiaomi-modules/keyboard_driver/jinghu/nanosic_driver
 */
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE nanosic_trace
#include <trace/define_trace.h>
