/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __XRING_MEMINFO_H__
#define __XRING_MEMINFO_H__

#include <linux/ioctl.h>
#include <linux/notifier.h>

#define xrmeminfo_err(fmt, ...) \
	pr_err("[MEM] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define xrmeminfo_warn(fmt, ...) \
	pr_warn("[MEM] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define xrmeminfo_info(fmt, ...) \
	pr_info("[MEM] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define xrmeminfo_debug(fmt, ...) \
	pr_debug("[MEM] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

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

#define K(x) ((x) << (PAGE_SHIFT - 10))
#define TRANSMIT_MAX_LEN	0x4000

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

enum meminfo_source {
	FROM_OOM = 0,
	FROM_DMAHEAP,
	FROM_SHRINKER,
	FROM_KERNEL,
	FROM_USER,
	MEMINFO_SOURCE_MAX,
};

extern struct notifier_block meminfo_oom_nb;

bool xr_should_dump_unreclaim_slab(void);
bool xr_should_dump_vmalloc(void);
bool xr_should_detect_slab_leak(void);
int xr_meminfo_init(void);
int xr_meminfo_parameter_init(void);
int xr_memory_netlink_init(void);
int xr_lowmem_shrinker_init(void);
int xr_memtrack_init(void);
void xr_meminfo_exit(void);
void xr_meminfo_parameter_exit(void);
void xr_memory_netlink_exit(void);
void xr_lowmem_shrinker_exit(void);
void xr_memtrack_exit(void);

int xr_process_show_meminfo(void *arg);
int xr_process_leak_detect(const void *arg);
int xr_process_leak_read(void *arg);
int xr_process_set_th(void *arg);
int xr_process_get_val(void *arg);
int xr_process_transmit(void *arg);
int xr_process_show_kill(void *arg);

void xr_meminfo_show(bool verbose, enum meminfo_source where);
void show_extra(enum meminfo_source where);
void show_dmabuf(bool verbose);
void show_tasks(bool verbose);
int lmkd_notify(unsigned int value);

#endif
