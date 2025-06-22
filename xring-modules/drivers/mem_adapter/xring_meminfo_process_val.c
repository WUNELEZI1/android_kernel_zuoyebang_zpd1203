// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/vmstat.h>
#include <linux/mmzone.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/freezer.h>
#include <linux/oom.h>
#include <linux/notifier.h>
#include <linux/vmalloc.h>
#include <linux/ratelimit.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/swap.h>
#include <linux/slab.h>

#include "xring_meminfo_process_val.h"
#include "xring_mem_adapter.h"

#define THRESHOLD_RATIO 12

static unsigned int slab_k = 1 << 20; /* slab normal size */
static unsigned int vmalloc_k = 1 << 20; /* vmalloc normal size */
static unsigned int slab_leak_k = 1 << 20; /* slab leak detect */
static unsigned int vmalloc_leak_k = 1 << 20; /* vmalloc leak detect */

#define K(x) ((x) << (PAGE_SHIFT - 10))

bool xr_should_dump_unreclaim_slab(void)
{
	return K(global_node_page_state_pages(NR_SLAB_UNRECLAIMABLE_B)) > slab_k;
}
EXPORT_SYMBOL(xr_should_dump_unreclaim_slab);

bool xr_should_dump_vmalloc(void)
{
	return K(vmalloc_nr_pages()) >= vmalloc_k;
}
EXPORT_SYMBOL(xr_should_dump_vmalloc);

bool xr_should_detect_slab_leak(void)
{
	return K(global_node_page_state_pages(NR_SLAB_UNRECLAIMABLE_B)) > slab_leak_k;
}
EXPORT_SYMBOL(xr_should_detect_slab_leak);

bool xr_should_detect_vmalloc_leak(void)
{
	return K(vmalloc_nr_pages()) >= vmalloc_leak_k;
}
EXPORT_SYMBOL(xr_should_detect_vmalloc_leak);

int xr_process_set_th(void *arg)
{
	struct xr_meminfo_threshold xmt;

	if (copy_from_user(&xmt, (void __user *)arg, sizeof(xmt))) {
		xrmem_err("copy from user failed\n");
		return -EFAULT;
	}
	if (xmt.target & SLAB_THRESHOLD) {
		slab_k = xmt.slab_k;
		xrmem_debug("set slab_k %d\n", slab_k);
	}
	if (xmt.target & VMALLOC_THRESHOLD) {
		vmalloc_k = xmt.vmalloc_k;
		xrmem_debug("set vmalloc_k %d\n", vmalloc_k);
	}
	if (xmt.target & SLAB_LEAK_THRESHOLD) {
		slab_leak_k = xmt.slab_leak_k;
		xrmem_debug("set slab_leak_k %d\n", slab_leak_k);
	}
	if (xmt.target & VMALLOC_LEAK_THRESHOLD) {
		vmalloc_leak_k = xmt.vmalloc_leak_k;
		xrmem_debug("set vmalloc_leak_k %d\n", vmalloc_leak_k);
	}
	return 0;
}
EXPORT_SYMBOL(xr_process_set_th);

int xr_process_get_val(void *arg)
{
	struct xr_meminfo_threshold xmt;

	if (copy_from_user(&xmt, (void __user *)arg, sizeof(xmt))) {
		xrmem_err("copy from user failed\n");
		return -EFAULT;
	}

	if (xmt.target & SLAB_THRESHOLD)
		xmt.slab_k = slab_k;
	if (xmt.target & VMALLOC_THRESHOLD)
		xmt.vmalloc_k = vmalloc_k;
	if (xmt.target & SLAB_LEAK_THRESHOLD)
		xmt.slab_leak_k = slab_leak_k;
	if (xmt.target & VMALLOC_LEAK_THRESHOLD)
		xmt.vmalloc_leak_k = vmalloc_leak_k;

	if (copy_to_user((void __user *)arg, &xmt, sizeof(xmt))) {
		xrmem_err("copy to user failed\n");
		return -EFAULT;
	}
	return 0;
}
EXPORT_SYMBOL(xr_process_get_val);

void xr_set_threshold(int type, unsigned int val_k)
{
	if (type == SLAB_THRESHOLD)
		slab_k = val_k;
	else if (type == VMALLOC_THRESHOLD)
		vmalloc_k = val_k;
	else if (type == SLAB_LEAK_THRESHOLD)
		slab_leak_k = val_k;
	else if (type == VMALLOC_LEAK_THRESHOLD)
		vmalloc_leak_k = val_k;
	else
		return;
}

int xr_meminfo_parameter_init(void)
{
	unsigned int total_k, threshold_k;

	total_k = K(totalram_pages());
	threshold_k = total_k / THRESHOLD_RATIO;
	xr_set_threshold(SLAB_THRESHOLD, threshold_k);
	xr_set_threshold(VMALLOC_THRESHOLD, threshold_k);
	xr_set_threshold(SLAB_LEAK_THRESHOLD, threshold_k);
	xr_set_threshold(VMALLOC_LEAK_THRESHOLD, threshold_k);
	return 0;
}
EXPORT_SYMBOL(xr_meminfo_parameter_init);

void xr_meminfo_parameter_exit(void)
{
}
EXPORT_SYMBOL(xr_meminfo_parameter_exit);

module_param(slab_k, uint, 0644);
module_param(vmalloc_k, uint, 0644);
module_param(slab_leak_k, uint, 0644);
module_param(vmalloc_leak_k, uint, 0644);
