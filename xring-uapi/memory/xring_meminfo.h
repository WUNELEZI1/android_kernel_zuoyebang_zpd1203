#ifndef __XRING_MEMINFO_H__
#define __XRING_MEMINFO_H__

#include <linux/ioctl.h>

#define XR_MEMINFO_IOC_MAGIC 'y'
#define XR_MEMINFO_IOC_SHOW_MEMINFO \
	_IOWR(XR_MEMINFO_IOC_MAGIC, 0x0, int)
#define XR_MEMINFO_IOC_SHOW_DMABUF \
	_IOWR(XR_MEMINFO_IOC_MAGIC, 0x1, int)
#define XR_MEMINFO_IOC_SHOW_TASK \
	_IOWR(XR_MEMINFO_IOC_MAGIC, 0x2, int)
#define XR_MEMINFO_IOC_LEAK_DETECT \
	_IOWR(XR_MEMINFO_IOC_MAGIC, 0x3, int)
#define XR_MEMINFO_IOC_LEAK_READ \
	_IOWR(XR_MEMINFO_IOC_MAGIC, 0x4, int)
#define XR_MEMINFO_IOC_SET_TH \
	_IOWR(XR_MEMINFO_IOC_MAGIC, 0x5, int)
#define XR_MEMINFO_IOC_GET_TH \
	_IOWR(XR_MEMINFO_IOC_MAGIC, 0x6, int)
#define XR_MEMINFO_IOC_TRANSMIT \
	_IOWR(XR_MEMINFO_IOC_MAGIC, 0x7, int)
#define XR_MEMINFO_IOC_SHOW_KILL \
	_IOWR(XR_MEMINFO_IOC_MAGIC, 0x8, int)

#define SLAB_MASK				(1 << 0)
#define VMALLOC_MASK			(1 << 1)
#define GPU_MASK				(1 << 2)
#define SLAB_DETECT_MASK		(1 << 3)
#define VMALLOC_DETECT_MASK		(1 << 4)

#define SLAB_THRESHOLD			(1 << 0)
#define VMALLOC_THRESHOLD		(1 << 1)
#define SLAB_LEAK_THRESHOLD		(1 << 2)
#define VMALLOC_LEAK_THRESHOLD	(1 << 3)
#define THRESHOLD_MASK 			0xFFFFFFFF

struct xr_meminfo_threshold {
	unsigned int target;
	unsigned int slab_k;
	unsigned int vmalloc_k;
	unsigned int slab_leak_k;
	unsigned int vmalloc_leak_k;
};

struct xr_meminfo_args {
	int flag;
	int verbose;
	char taskname[64];
	int pid;
	int uid;
	int oomadj;
	int kill_reason;
	__s64 rss_kb;
	__s64 swap_kb;
};

struct xr_meminfo_write_buf {
	unsigned int len;
	char buf[0];
};

#endif