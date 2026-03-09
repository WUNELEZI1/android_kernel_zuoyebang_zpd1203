// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/shrinker.h>
#include <trace/hooks/mm.h>
#include <linux/sched/clock.h>
#include "mm/slab.h"
#include <linux/cma.h>

#include "memcheck_account.h"
#include "memcheck_ioctl.h"
#include "memcheck_cma.h"
#include "memcheck_process_mem.h"
#include "memcheck_gpumem.h"
#include "memcheck_slab.h"

// Reporting threshold > 3G
static unsigned long slab_leak_maxsize_kb __read_mostly = (3*1024*1024);
module_param(slab_leak_maxsize_kb, ulong, 0644);

static void hook_slab_alloc_node(void *unused, void *object,
								unsigned long addr, struct kmem_cache *s)
{
	int i;
	unsigned long slab_unrec_kb = global_node_page_state_pages(NR_SLAB_UNRECLAIMABLE_B)<<(PAGE_SHIFT-10);

	if (slab_unrec_kb < slab_leak_maxsize_kb)
		return;

	for (i = 0; i < ARRAY_SIZE(slubobj_ignore_list); i++) {
		if (strstr(slubobj_ignore_list[i], s->name)) {
			return;
		}
	}

	memcheck_slab_alloc_node_report(object, addr, s);
}

static void hook_meminfo_proc_show(void *data, struct seq_file *m)
{
	memcheck_gpumem_hook_meminfo(m);
}

static unsigned long memcheck_shrinker_count_objects(struct shrinker *shrinker,
													struct shrink_control * sc)
{ return 1; }

static unsigned long memcheck_shrinker_scan_objects(struct shrinker *shrinker,
													struct shrink_control * sc)
{
	memcheck_warn_alloc_show_mem();

	return 0;
}

static struct shrinker *memcheck_lowmem_shrinker;
static int __init memcheck_init(void)
{
	memcheck_createfs();

	memcheck_lowmem_shrinker = shrinker_alloc(0, "memcheck_lowermem");
	if (!memcheck_lowmem_shrinker)
		return -ENOMEM;

	memcheck_lowmem_shrinker->count_objects = memcheck_shrinker_count_objects;
	memcheck_lowmem_shrinker->scan_objects = memcheck_shrinker_scan_objects;
	memcheck_lowmem_shrinker->seeks = DEFAULT_SEEKS;
	shrinker_register(memcheck_lowmem_shrinker);

	register_trace_android_vh_meminfo_proc_show(hook_meminfo_proc_show, NULL);
	register_trace_android_vh_slab_alloc_node(hook_slab_alloc_node, NULL);

	return 0;
}

module_init(memcheck_init);

MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(DMA_BUF);
MODULE_IMPORT_NS(MINIDUMP);
MODULE_AUTHOR("yangshiguang <yangshiguang@xiaomi.com>");
MODULE_DESCRIPTION("MI Memcheck Module");
