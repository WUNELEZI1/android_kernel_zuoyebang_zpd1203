/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM xring_heap

#if !defined(_XRING_TRACE_HEAP_H) || defined(TRACE_HEADER_MULTI_READ)
#define _XRING_TRACE_HEAP_H

#include <linux/tracepoint.h>

TRACE_EVENT(dma_buf_cpu_access,

	TP_PROTO(const char *label,
		void *dmabuf,
		size_t size,
		int dir,
		unsigned int offset,
		unsigned int len),

	TP_ARGS(label, dmabuf, size, dir, offset, len),

	TP_STRUCT__entry(
		__string(label, label)
		__field(void *, dmabuf)
		__field(size_t, size)
		__field(int, dir)
		__field(unsigned int, offset)
		__field(unsigned int, len)
	),

	TP_fast_assign(
		__assign_str(label, label);
		__entry->dmabuf = dmabuf;
		__entry->size = size;
		__entry->dir = dir;
		__entry->offset = offset;
		__entry->len = len;
	),

	TP_printk("%s dmabuf:%p %ld dir:%d offset:%u len:%u",
		__get_str(label),
		__entry->dmabuf,
		__entry->size,
		__entry->dir,
		__entry->offset,
		__entry->len
	)
);

TRACE_EVENT(map_dma_buf,

	TP_PROTO(const char *label,
		const char *devname,
		void *dmabuf,
		size_t size,
		int dir),

	TP_ARGS(label, devname, dmabuf, size, dir),

	TP_STRUCT__entry(
		__string(label, label)
		__string(devname, devname)
		__field(void *, dmabuf)
		__field(size_t, size)
		__field(int, dir)
	),

	TP_fast_assign(
		__assign_str(label, label);
		__assign_str(devname, devname);
		__entry->dmabuf = dmabuf;
		__entry->size = size;
		__entry->dir = dir;
	),

	TP_printk("%s %s dmabuf:%p %ld dir:%d",
		__get_str(label),
		__get_str(devname),
		__entry->dmabuf,
		__entry->size,
		__entry->dir
	)
);

#endif /* _XRING_TRACE_HEAP_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../xring-modules/drivers/heaps
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE xring_heap_trace
/* This part must be outside protection */
#include <trace/define_trace.h>
