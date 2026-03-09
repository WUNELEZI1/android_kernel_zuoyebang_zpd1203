#ifndef __MEMCHECK_IOCTL_H__
#define __MEMCHECK_IOCTL_H__

#include <linux/printk.h>
#include "../../soc/qcom/debug_symbol.h"
#include "../hypsys_netlink/hypsys_netlink.h"

#define __MEMCHECK_IO 'M'

// #define MEMCHECK_NETLINK_RECONET	_IO(__MEMCHECK_IO, 0)
#define MEMCHECK_PMEM_INFO	_IO(__MEMCHECK_IO, 1)
#define MEMCHECK_CMA_INFO	_IO(__MEMCHECK_IO, 2)
#define MEMCHECK_DMABUF_INFO	_IO(__MEMCHECK_IO, 3)
#define MEMCHECK_SLAB_INFO	_IO(__MEMCHECK_IO, 4)
#define MEMCHECK_VMALLOC_INFO	_IO(__MEMCHECK_IO, 5)
#define MEMCHECK_GPU_INFO	_IO(__MEMCHECK_IO, 6)
#define MEMCHECK_COMMAND	_IO(__MEMCHECK_IO, 7)

#define MEMCHECK_IOC_MIN	MEMCHECK_PMEM_INFO
#define MEMCHECK_IOC_MAX	MEMCHECK_COMMAND

#define TO_KB(bytes)		((bytes) >> 10)
#define TO_MB(bytes)		(TO_KB(bytes) >> 10)
#define PAGES_TO_KB(n_pages)	((n_pages) << (PAGE_SHIFT - 10))
#define PAGES_TO_MB(n_pages)	(PAGES_TO_KB(n_pages) >> 10)

#define    memcheck_err(format, ...)    \
    pr_err("MemCheck[%s %d] " format, __func__, __LINE__, ##__VA_ARGS__)
#define    memcheck_info(format, ...)    \
    pr_info("MemCheck[%s %d] " format, __func__, __LINE__, ##__VA_ARGS__)

#define MEMCHECK_DEBUG_LOOKUP(_var, type) \
	do {\
		mmcheck_##_var = (type *)DEBUG_SYMBOL_LOOKUP(_var); \
		if (!mmcheck_##_var) { \
			memcheck_err("Can't find symbol %s\n", #_var); \
		} \
	} while (0)

#endif /* _MEMCHECK_IOCTL_H */
