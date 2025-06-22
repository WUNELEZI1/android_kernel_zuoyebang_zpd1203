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
#include <linux/stackdepot.h>
#include <linux/android_debug_symbols.h>
#include <linux/slab.h>
#include <linux/kstrtox.h>
#include <linux/string.h>
#include <trace/hooks/mm.h>

#include <soc/xring/xr_dmabuf_helper.h>
#include "xring_meminfo.h"
#include "../mm/slab.h"
#include "xring_meminfo_notify_lmkd.h"

static unsigned int task_pages_threshhold = 25000;
static unsigned int oom_show_alltask;
static char *const meminfo_source_text[] = {
	"FROM_OOM",
	"FROM_DMAHEAP",
	"FROM_SHRINKER",
	"FROM_KERNEL",
	"FROM_USER",
	"MEMINFO_SOURCE_MAX",
};

typedef void (*show_mem_func)(unsigned int flags, nodemask_t *nodemask, int max_zone_idx);
void xr_show_mem(void)
{
	show_mem_func show_mem_symbol = (show_mem_func)android_debug_symbol(ADS_SHOW_MEM);

	show_mem_symbol(0, NULL, MAX_NR_ZONES - 1);
}

void show_extra(enum meminfo_source where)
{
	unsigned long free, process_total, pagetables, zsmalloc;
	unsigned long sreclaimable, sunreclaim, vmalloc;

	free = K(global_zone_page_state(NR_FREE_PAGES));
	process_total = K(global_node_page_state(NR_ACTIVE_ANON) +
					global_node_page_state(NR_INACTIVE_ANON) +
					global_node_page_state(NR_ACTIVE_FILE) +
					global_node_page_state(NR_INACTIVE_FILE));
	pagetables = K(global_node_page_state(NR_PAGETABLE) +
					global_node_page_state(NR_SECONDARY_PAGETABLE));
	zsmalloc = K(global_zone_page_state(NR_ZSPAGES));
	sreclaimable = K(global_node_page_state_pages(NR_SLAB_RECLAIMABLE_B));
	sunreclaim = K(global_node_page_state_pages(NR_SLAB_UNRECLAIMABLE_B));
	vmalloc = K(vmalloc_nr_pages());

	xrmeminfo_info("Xring-meminfo extra:[%s:%s]",
		current->comm,
		meminfo_source_text[where]);
	xrmeminfo_info("[free:%lukB] [tasks(anon+file):%lukB] [pagetables:%lukB] [zsmalloc:%lukB]\n",
		free,
		process_total,
		pagetables,
		zsmalloc);
	xrmeminfo_info("[slab:%lukB] [slab-unr:%lukB] [vmalloc:%lukB] [skb:%dkB]\n",
		sreclaimable + sunreclaim,
		sunreclaim,
		vmalloc,
		0);
	xrmeminfo_info("[dmabuf:lookdown dmabuf total] [gpu:lookdown GPU memory info]\n");
}

void show_dmabuf(bool verbose)
{
	show_dmabuf_detail(verbose);
}

unsigned long xr_dump_slab(void)
{
	struct kmem_cache *s;
	struct slabinfo sinfo;
	struct list_head *xr_slab_caches = android_debug_symbol(ADS_SLAB_CACHES);
	struct mutex *xr_slab_mutex = android_debug_symbol(ADS_SLAB_MUTEX);
	unsigned long total_size = 0;

	xrmeminfo_info("Unreclaimable slab info:\n");
	xrmeminfo_info("Name                      Used          Total\n");

	if (!mutex_trylock(xr_slab_mutex)) {
		pr_warn("excessive unreclaimable slab but cannot dump stats\n");
		return 0;
	}
	list_for_each_entry(s, xr_slab_caches, list) {
		if (s->flags & SLAB_RECLAIM_ACCOUNT)
			continue;

		get_slabinfo(s, &sinfo);

		if (sinfo.num_objs > 0) {
			xrmeminfo_info("%-32s %10luKB %10luKB\n", s->name,
				(sinfo.active_objs * s->size) / 1024,
				(sinfo.num_objs * s->size) / 1024);
			total_size += sinfo.num_objs * s->size;
		}
	}
	mutex_unlock(xr_slab_mutex);
	xrmeminfo_info("TOTAL               %10luKB\n", total_size / 1024);
	return total_size;
}


static struct task_struct *__find_lock_task_mm(struct task_struct *p)
{
	struct task_struct *t;

	rcu_read_lock();

	for_each_thread(p, t) {
		task_lock(t);
		if (likely(t->mm))
			goto found;
		task_unlock(t);
	}
	t = NULL;
found:
	rcu_read_unlock();

	return t;
}

void show_tasks(bool verbose)
{
	struct task_struct *p;
	struct task_struct *task = NULL;
	unsigned long task_mm_rss;
	unsigned long task_mm_swap;
	short task_oom_score_adj;
	char task_state = 0;
	char frozen_mark = ' ';

	xrmeminfo_info("Tasks state (memory values in pages):\n");
	xrmeminfo_info("[  pid  ]   uid  tgid       vm      rss     pgt_byte     swap   adj state frozen name\n");

	rcu_read_lock();
	for_each_process(p) {
		task = __find_lock_task_mm(p);
		if (!task)
			continue;

		task_oom_score_adj = task->signal->oom_score_adj;
		task_mm_rss = get_mm_rss(task->mm);

		if (!verbose && task_mm_rss < task_pages_threshhold) {
			task_unlock(task);
			continue;
		}

		task_mm_swap = get_mm_counter(task->mm, MM_SWAPENTS);
		task_state = task_state_to_char(task);
		frozen_mark = (READ_ONCE(p->__state) & TASK_FROZEN) ? '*' : '-';

		xrmeminfo_info("[%7d] %5d %5d %8lu %8lu*4KB %8ld %8lu %5d %c %c %s\n",
				task->pid, from_kuid(&init_user_ns, task_uid(task)),
				task->tgid, task->mm->total_vm, task_mm_rss,
				mm_pgtables_bytes(task->mm), task_mm_swap,
				task_oom_score_adj, task_state, frozen_mark, task->comm);
		task_unlock(task);
	}
	rcu_read_unlock();
}

void try_notify_lmkd(void)
{
	unsigned int notify = GPU_MASK;

	if (xr_should_dump_unreclaim_slab())
		notify |= SLAB_MASK;
	if (xr_should_dump_vmalloc())
		notify |= VMALLOC_MASK;
	lmkd_notify(notify);
}

void xr_meminfo_show(bool verbose, enum meminfo_source where)
{
	static DEFINE_RATELIMIT_STATE(brief_meminfo_ratelimit, 600 * HZ, 1);
	static DEFINE_RATELIMIT_STATE(detail_meminfo_ratelimit, 600 * HZ, 1);

	/* only the kernel's printout is limited here. */
	/* from_kernel == false means this log request comes from user. */
	if (where <= FROM_KERNEL && !verbose && !__ratelimit(&brief_meminfo_ratelimit))
		return;
	if (where <= FROM_KERNEL && verbose && !__ratelimit(&detail_meminfo_ratelimit))
		return;

	xr_show_mem();
	show_extra(where);
	if (xr_should_dump_unreclaim_slab())
		xr_dump_slab();
	show_dmabuf(verbose);
	show_tasks(verbose);
	if (where <= FROM_KERNEL)
		try_notify_lmkd();
}

int meminfo_oom_notify(struct notifier_block *nb,
					   unsigned long action, void *data)
{
	static DEFINE_RATELIMIT_STATE(notify_lmkd_ratelimit, 60 * HZ, 1);

	show_extra(FROM_OOM);
	show_dmabuf(true);
	show_tasks(oom_show_alltask);

	if (__ratelimit(&notify_lmkd_ratelimit)) {
		/* notifying lmkd is limited to prevent it from running too frequently */
		try_notify_lmkd();
	}
	return 0;
}

struct notifier_block meminfo_oom_nb = {
	.notifier_call = meminfo_oom_notify,
};

int xr_process_show_meminfo(void *arg)
{
	struct xr_meminfo_args xma;

	if (copy_from_user(&xma, (void __user *)arg, sizeof(struct xr_meminfo_args))) {
		xrmeminfo_err("copy_from_user failed\n");
		return -EFAULT;
	}
	xr_meminfo_show(xma.verbose, FROM_USER);
	return 0;
}

int xr_process_show_kill(void *arg)
{
	struct xr_meminfo_args xma;

	if (copy_from_user(&xma, (void __user *)arg, sizeof(struct xr_meminfo_args))) {
		xrmeminfo_err("copy_from_user failed\n");
		return -EFAULT;
	}
	if (xma.pid)
		xrmeminfo_info("Kill '%s' (%d), uid %d, oom_score_adj %d reason %d to free %lldkB rss, %lldkB swap;\n",
			xma.taskname, xma.pid, xma.uid, xma.oomadj, xma.kill_reason, xma.rss_kb, xma.swap_kb);
	return 0;
}

static inline void print_assist(char *str, size_t length)
{
	int i;
	char *ptr = str;

	for (i = 0; i < length && ptr < (str + length); i++) {
		if (str[i] == ';') {
			str[i] = '\0';
			xrmeminfo_info("%s\n", ptr);
			ptr = str + i + 1;
		}
	}
	if (ptr < (str + length))
		xrmeminfo_info("%s\n", ptr);
}

int xr_process_transmit(void *arg)
{
	struct xr_meminfo_write_buf write_buf;
	char *buf = NULL;
	int ret = 0;

	if (copy_from_user(&write_buf, (void __user *)arg, sizeof(struct xr_meminfo_write_buf))) {
		xrmeminfo_err("copy from user failed\n");
		return -EFAULT;
	}
	if (write_buf.len == 0 || write_buf.len >= TRANSMIT_MAX_LEN) {
		xrmeminfo_err("len %u is invalid\n", write_buf.len);
		return -EFAULT;
	}

	buf = kzalloc(write_buf.len, GFP_KERNEL);
	if (copy_from_user(buf, (void __user *)(arg + sizeof(write_buf)), write_buf.len)) {
		xrmeminfo_err("copy from user failed\n");
		kfree(buf);
		return -EFAULT;
	}
	buf[write_buf.len - 1] = '\0';
	print_assist(buf, write_buf.len);
	kfree(buf);
	return ret;
}

module_param(task_pages_threshhold, uint, 0644);
module_param(oom_show_alltask, uint, 0644);
