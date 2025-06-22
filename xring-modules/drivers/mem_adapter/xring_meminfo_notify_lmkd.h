/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _XRING_MEMINFO_NOTIFY_LMKD_H_
#define _XRING_MEMINFO_NOTIFY_LMKD_H_

#define SLAB_MASK				(1 << 0)
#define VMALLOC_MASK			(1 << 1)
#define GPU_MASK				(1 << 2)
#define SLAB_DETECT_MASK		(1 << 3)
#define VMALLOC_DETECT_MASK		(1 << 4)

int xr_memory_netlink_init(void);
void xr_memory_netlink_exit(void);
int lmkd_notify(unsigned int value);

#endif
