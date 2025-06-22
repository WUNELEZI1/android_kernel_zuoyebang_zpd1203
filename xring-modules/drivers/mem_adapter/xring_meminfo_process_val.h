/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _XRING_MEMINFO_PROCESS_VAL_H_
#define _XRING_MEMINFO_PROCESS_VAL_H_

#define SLAB_THRESHOLD			(1 << 0)
#define VMALLOC_THRESHOLD		(1 << 1)
#define SLAB_LEAK_THRESHOLD		(1 << 2)
#define VMALLOC_LEAK_THRESHOLD	(1 << 3)
#define THRESHOLD_MASK			0xFFFFFFFF

struct xr_meminfo_threshold {
	unsigned int target;
	unsigned int slab_k;
	unsigned int vmalloc_k;
	unsigned int slab_leak_k;
	unsigned int vmalloc_leak_k;
};

bool xr_should_dump_unreclaim_slab(void);
bool xr_should_dump_vmalloc(void);
bool xr_should_detect_slab_leak(void);
bool xr_should_detect_vmalloc_leak(void);
int xr_process_set_th(void *arg);
int xr_process_get_val(void *arg);

#endif
