// SPDX-License-Identifier: GPL-2.0

#include <linux/sizes.h>
#include <linux/types.h>
#include <linux/seq_file.h>
#include <linux/spinlock_types.h>
#include <linux/err.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <trace/hooks/mm.h>
#include <linux/sort.h>
#include <linux/stackdepot.h>
#include <linux/android_debug_symbols.h>
#include "memory_track.h"
#include "../xring_meminfo_notify_lmkd.h"
#include "../xring_meminfo_process_val.h"
#include "soc/xring/xring_mem_adapter.h"

static int vmalloc_track_flag;

void vmalloc_maybe_need_notify(void)
{
	static DEFINE_RATELIMIT_STATE(vmalloc_detect_notify, 30 * HZ, 1);

	if (unlikely(xr_should_detect_vmalloc_leak())) {
		/* dont use vmalloc_track_flag to prevent notify */
		if (!__ratelimit(&vmalloc_detect_notify))
			return;
		xrmem_debug("vmalloc track notify\n");
		lmkd_notify(VMALLOC_DETECT_MASK); // notify lmkd vmalloc is too big

		xring_memory_event_report(MM_LEAK_VMALLOC);
	}
}

void vh_save_vmalloc_stack(void *data, unsigned long flags, struct vm_struct *vm)
{
	unsigned long entries[VM_STACK_ENTRY_NUM];
	unsigned int nr_entries;
	depot_stack_handle_t handle;

	vmalloc_maybe_need_notify();

	if (!vmalloc_track_flag)
		return;
	nr_entries = stack_trace_save(entries, ARRAY_SIZE(entries), VM_IGNORED_ENTRY_NUM);
	handle = stack_depot_save(entries, nr_entries, GFP_NOWAIT);
	if (handle)
		vm->caller = (void *)(handle | VM_CALLER_HAS_STACK << 32);
}

void vh_show_stack_hash(void *data, struct seq_file *m, struct vm_struct *vm)
{
	unsigned long *entries;
	unsigned int nr_entries;
	int i;

	if (((unsigned long)vm->caller >> 32) == VM_CALLER_HAS_STACK) {
		seq_putc(m, '\n');
		nr_entries = stack_depot_fetch((unsigned long)vm->caller, &entries);
		seq_printf(m, VM_STACK_MARKER_STR "%pS %7ld %d %u\n", vm->caller, vm->size, vm->nr_pages, nr_entries);
		for (i = 0; i < nr_entries; i++)
			seq_printf(m, "    %pS\n", (void *)entries[i]);
	}
}

int memtrack_vmap_purge_notify(struct notifier_block *nb,
					   unsigned long action, void *data)
{
	static DEFINE_RATELIMIT_STATE(vmalloc_detect_notify, 30 * HZ, 1);

	if (likely(!__ratelimit(&vmalloc_detect_notify)))
		return 0;
	lmkd_notify(VMALLOC_DETECT_MASK);
	xrmem_debug("vmalloc purge notify\n");
	return 0;
}

struct notifier_block memtrack_vmap_purge_nb = {
	.notifier_call = memtrack_vmap_purge_notify,
};

int xr_vmalloc_track_on(char *name)
{
	vmalloc_track_flag = 1;
	return 0;
}
int xr_vmalloc_track_off(char *name)
{
	vmalloc_track_flag = 0;
	return 0;
}

int xr_vmalloc_stack_open(int type)
{
	return 0;
}
int xr_vmalloc_stack_close(void)
{
	return 0;
}

size_t xr_vmalloc_stack_read(struct xr_stack_info *stack_info_list, size_t len, int type)
{
	return 0;
}

int vmalloc_track_init(void)
{
	register_trace_android_vh_save_vmalloc_stack(vh_save_vmalloc_stack, NULL);
	register_trace_android_vh_show_stack_hash(vh_show_stack_hash, NULL);
	(void)register_vmap_purge_notifier(&memtrack_vmap_purge_nb);
	return 0;
}

void vmalloc_track_exit(void)
{
	(void)unregister_vmap_purge_notifier(&memtrack_vmap_purge_nb);
	unregister_trace_android_vh_show_stack_hash(vh_show_stack_hash, NULL);
	unregister_trace_android_vh_save_vmalloc_stack(vh_save_vmalloc_stack, NULL);
}

module_param(vmalloc_track_flag, int, 0644);
