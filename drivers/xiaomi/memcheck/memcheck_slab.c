// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/dma-resv.h>
#include <linux/mm.h>
#include <linux/ratelimit.h>

#include "mm/slab.h"
#include "mm/internal.h"
#include "memcheck_ioctl.h"


#define MAX_SLUB_OBJ_SIZE 800
static unsigned long max_slub_obj_size_mb __read_mostly = MAX_SLUB_OBJ_SIZE;
module_param(max_slub_obj_size_mb, ulong, 0644);

static struct work_struct slab_report_work;
static struct list_head *mmcheck_slab_caches;
static struct mutex *mmcheck_slab_mutex;

// mm/slub.c
/*
 * The slab lists for all objects.
 */
struct kmem_cache_node {
	spinlock_t list_lock;
	unsigned long nr_partial;
	struct list_head partial;
#ifdef CONFIG_SLUB_DEBUG
	atomic_long_t nr_slabs;
	atomic_long_t total_objects;
	struct list_head full;
#endif
};

struct event_info {
	pid_t pid;
	pid_t tgid;
	char comm[TASK_COMM_LEN];
	const char *name;
	unsigned long total;
	unsigned long num_objs;
};
struct event_info event = {0};

static void slab_report_work_func(struct work_struct *work)
{
	int ret = 0;
	struct report_event *report_event;

	report_event = hypsys_event_alloc(SLAB_REPORT);
	ret = hypsys_event_add_int(report_event, "pid", event.pid);
	ret += hypsys_event_add_int(report_event, "tgid", event.tgid);
	ret += hypsys_event_add_str(report_event, "comm", event.comm);
	ret += hypsys_event_add_str(report_event, "name", event.name);
	ret += hypsys_event_add_int(report_event, "total", event.total);
	ret += hypsys_event_add_int(report_event, "num_objs", event.num_objs);
	if (ret) {
		memcheck_err("add info to slab_event failed, ret=%d\n", ret);
		goto end;
	}

	if (!hypsys_event_report(report_event))
		memcheck_err("report slab_event failed\n");

end:
	hypsys_event_destroy(report_event);
}

#ifdef CONFIG_SLUB_DEBUG
static inline unsigned long node_nr_objs(struct kmem_cache_node *n)
{
	return atomic_long_read(&n->total_objects);
}
#endif

static inline struct kmem_cache_node *get_node(struct kmem_cache *s, int node)
{
	return s->node[node];
}

/*
 * Iterator over all nodes. The body will be executed for each node that has
 * a kmem_cache_node structure allocated (which is true for all online nodes)
 */
#define for_each_kmem_cache_node(__s, __node, __n) \
	for (__node = 0; __node < nr_node_ids; __node++) \
		 if ((__n = get_node(__s, __node)))

void memcheck_slab_alloc_node_report(void *object, unsigned long addr, struct kmem_cache *s)
{
	int node;
	unsigned long size = 0;
	unsigned long nr_objs = 0;
	struct kmem_cache_node *n;
	// report once for one hour maximum
	static DEFINE_RATELIMIT_STATE(ratelimit_report, 60 * 60 * HZ, 1);
	// print stack twicw for 5 second maximum
	static DEFINE_RATELIMIT_STATE(ratelimit_stack, 5 * HZ, 2); 

#ifdef CONFIG_SLUB_DEBUG
	for_each_kmem_cache_node(s, node, n)
		nr_objs += node_nr_objs(n);
	size = nr_objs * s->size / 1024 / 1024;
#endif

	if (size <= max_slub_obj_size_mb)
		return;

	if (__ratelimit(&ratelimit_stack)) {
		memcheck_err("slub may leak, cache_name=%s, cache_size=%lu mB", s->name, size);
		dump_stack();
	}

	if (__ratelimit(&ratelimit_report)) {
		event.pid = current->pid;
		event.tgid = current->tgid;
		memcpy(event.comm, current->comm, TASK_COMM_LEN);
		event.name = s->name;
		event.total = size;
		event.num_objs = nr_objs;
		schedule_work(&slab_report_work);
	}
}

static void slab_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_printf(s, "dump slab info:\n");
		seq_printf(s, "----------------------------------------------------\n");
		seq_printf(s, "# name            <active_objs> <num_objs> <objsize> <objperslab> <pagesperslab>");
		seq_printf(s, " : tunables <limit> <batchcount> <sharedfactor>");
		seq_printf(s, " : slabdata <active_slabs> <num_slabs> <sharedavail>");
#ifdef CONFIG_DEBUG_SLAB
		seq_printf(s, " : globalstat <listallocs> <maxobjs> <grown> <reaped> <error> <maxfreeable> <nodeallocs> <remotefrees> <alienoverflow>");
		seq_printf(m, " : cpustat <allochit> <allocmiss> <freehit> <freemiss>");
#endif
	} else {
		memcheck_info("dump slab info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("# name			<active_objs> <num_objs> <objsize> <objperslab> <pagesperslab>");
		memcheck_info(" : tunables <limit> <batchcount> <sharedfactor>");
		memcheck_info(" : slabdata <active_slabs> <num_slabs> <sharedavail>");
#ifdef CONFIG_DEBUG_SLAB
		memcheck_info(" : globalstat <listallocs> <maxobjs> <grown> <reaped> <error> <maxfreeable> <nodeallocs> <remotefrees> <alienoverflow>");
		memcheck_info(" : cpustat <allochit> <allocmiss> <freehit> <freemiss>");
#endif
	}
}

static void slab_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_printf(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

#ifdef CONFIG_SLUB_DEBUG
static void slabinfo_stats(struct seq_file *m, struct kmem_cache *cachep)
{
#ifdef CONFIG_DEBUG_SLAB
	{			/* node stats */
		unsigned long high = cachep->high_mark;
		unsigned long allocs = cachep->num_allocations;
		unsigned long grown = cachep->grown;
		unsigned long reaped = cachep->reaped;
		unsigned long errors = cachep->errors;
		unsigned long max_freeable = cachep->max_freeable;
		unsigned long node_allocs = cachep->node_allocs;
		unsigned long node_frees = cachep->node_frees;
		unsigned long overflows = cachep->node_overflow;

		seq_printf(m, " : globalstat %7lu %6lu %5lu %4lu %4lu %4lu %4lu %4lu %4lu",
					allocs, high, grown, reaped, errors, max_freeable,
					node_allocs, node_frees, overflows);
	}
	/* cpu stats */
	{
		unsigned long allochit = atomic_read(&cachep->allochit);
		unsigned long allocmiss = atomic_read(&cachep->allocmiss);
		unsigned long freehit = atomic_read(&cachep->freehit);
		unsigned long freemiss = atomic_read(&cachep->freemiss);

		seq_printf(m, " : cpustat %6lu %6lu %6lu %6lu",
					allochit, allocmiss, freehit, freemiss);
	}
#endif
}

static int slab_info_show(struct seq_file *s, void *data)
{
	struct kmem_cache *cachep;
	struct slabinfo sinfo;
	unsigned long old_ns = ktime_get();

	slab_info_print_header(s);

	if (!mutex_trylock(mmcheck_slab_mutex))
		return -EFAULT;

	list_for_each_entry(cachep, mmcheck_slab_caches, list) {
		memset(&sinfo, 0, sizeof(sinfo));
		get_slabinfo(cachep, &sinfo);

		seq_printf(s, "%-17s %6lu %6lu %6u %4u %4d",
					cachep->name, sinfo.active_objs, sinfo.num_objs, cachep->size,
					sinfo.objects_per_slab, (1 << sinfo.cache_order));
		seq_printf(s, " : tunables %4u %4u %4u",
					sinfo.limit, sinfo.batchcount, sinfo.shared);
		seq_printf(s, " : slabdata %6lu %6lu %6lu",
					sinfo.active_slabs, sinfo.num_slabs, sinfo.shared_avail);
		slabinfo_stats(s, cachep);
		seq_printf(s, "\n");
	}
	mutex_unlock(mmcheck_slab_mutex);

	memcheck_info("take %llu ns", ktime_get() - old_ns);
	slab_info_print_foot(s);

	return 0;
}
#else
static int slab_info_show(struct seq_file *s, void *data) {}
#endif

void memcheck_slab_info_show(void)
{
	if (!mmcheck_slab_caches)
		return ;
	if (!mmcheck_slab_mutex)
		return ;
	slab_info_show(NULL, NULL);
}
EXPORT_SYMBOL(memcheck_slab_info_show);

static int slab_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, slab_info_show, pde_data(inode));
}

static const struct proc_ops slab_info_fops = {
	.proc_open = slab_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int memcheck_slab_createfs(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *entry = NULL;

	MEMCHECK_DEBUG_LOOKUP(slab_caches, struct list_head);
	MEMCHECK_DEBUG_LOOKUP(slab_mutex, struct mutex);

	if (!mmcheck_slab_caches)
		return -EFAULT;
	if (!mmcheck_slab_mutex)
		return -EFAULT;

	entry = proc_create("slab_info", 0444, parent, &slab_info_fops);
	if (!entry)
		memcheck_err("Failed to create slab buffer debug info\n");

	INIT_WORK(&slab_report_work, slab_report_work_func);

	return (!entry ? -EFAULT : 0);
}
