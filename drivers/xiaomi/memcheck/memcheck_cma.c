// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/cma.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/kobject.h>

#include "memcheck_ioctl.h"
#include "memcheck_cma.h"
// #include "../hypsys_netlink/hypsys_netlink.h"

// sync mm/cma.h
struct cma_kobject {
	struct kobject kobj;
	struct cma *cma;
};

struct cma {
	unsigned long   base_pfn;
	unsigned long   count;
	unsigned long   *bitmap;
	unsigned int order_per_bit; /* Order of pages represented by one bit */
	spinlock_t	lock;
#ifdef CONFIG_CMA_DEBUGFS
	struct hlist_head mem_head;
	spinlock_t mem_head_lock;
	struct debugfs_u32_array dfs_bitmap;
#endif
	char name[CMA_MAX_NAME];
#ifdef CONFIG_CMA_SYSFS
	/* the number of CMA page successful allocations */
	atomic64_t nr_pages_succeeded;
	/* the number of CMA page allocation failures */
	atomic64_t nr_pages_failed;
	/* the number of CMA page released */
	atomic64_t nr_pages_released;
	/* kobject requires dynamic object */
	struct cma_kobject *cma_kobj;
#endif
	bool reserve_pages_on_error;
};

static inline unsigned long cma_bitmap_maxno(struct cma *cma)
{
	return cma->count >> cma->order_per_bit;
}
// mm/cma.h

#define CMA_REPORT_PERCENT 50

struct event_info {
	pid_t pid;
	pid_t tgid;
	char comm[TASK_COMM_LEN];
	char *name;
	unsigned long total;
	unsigned long req;
};

static unsigned long cma_report_area_used;
static struct event_info event;
static struct work_struct cma_report_work;

static unsigned long cma_get_used(struct cma *cma)
{
	unsigned long used;

	spin_lock_irq(&cma->lock);
	/* pages counter is smaller than sizeof(int) */
	used = bitmap_weight(cma->bitmap, (int)cma_bitmap_maxno(cma));
	spin_unlock_irq(&cma->lock);

	return ((unsigned long)used << cma->order_per_bit) << PAGE_SHIFT;
}

static int cma_report_cb(struct cma *cma, void *data)
{
	const char *report_name = (const char *)data;
	const char *name = cma_get_name(cma);

	if (!strcmp(report_name, name)) {
		cma_report_area_used = cma_get_used(cma);
		return 1;
	}
	return 0;
}

static void cma_report_work_func(struct work_struct *work)
{
	int ret = 0;
	struct report_event *report_event;

	report_event = hypsys_event_alloc(CMA_REPORT);
	if (!report_event)
		return;

	ret += hypsys_event_add_str(report_event, "comm", event.comm);
	ret += hypsys_event_add_int(report_event, "pid", event.pid);
	ret += hypsys_event_add_int(report_event, "tgid", event.tgid);
	ret += hypsys_event_add_str(report_event, "name", event.name);
	ret += hypsys_event_add_int(report_event, "total", event.total);
	ret += hypsys_event_add_int(report_event, "req_count", event.req);
	if (ret) {
		memcheck_err("add info to cma_event failed, ret=%d\n", ret);
		goto end;
	}

	if (!hypsys_event_report(report_event))
		memcheck_err("report cma_event failed\n");

end:
	hypsys_event_destroy(report_event);
}

void memcheck_cma_report(char *name, unsigned long count, unsigned long req_count)
{
	/* Not more than 1 messages every 1 hour */
	static DEFINE_RATELIMIT_STATE(cma_report, 60 * 60 * HZ, 1);
	int ret;

	if (!__ratelimit(&cma_report))
		return;

	ret = cma_for_each_area(cma_report_cb, name);
	if (!ret)
		return;

	if ((cma_report_area_used * 100) < (count << PAGE_SHIFT) * CMA_REPORT_PERCENT) {
		memcheck_err("less than CMA_REPORT_PERCENT_%d, cma_area: %s, "
					"total: %ld, req: %ld, used: %ld",
					CMA_REPORT_PERCENT, name, count, req_count, cma_report_area_used);
		return;
	}

	event.pid = current->pid;
	event.tgid = current->tgid;
	memcpy(event.comm, current->comm, TASK_COMM_LEN);
	event.name = name;
	event.total = count;
	event.req = req_count;
	schedule_work(&cma_report_work);
}

static void cma_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_puts(s, "CMA info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%15s %30s %15s %15s %10s\n",
					"cma_base_pfn", "cma_area", "Total", "Used", "Percent");
	} else {
		memcheck_info("CMA info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%15s %30s %15s %15s %10s\n",
						"cma_base_pfn", "cma_area", "Total", "Used", "Percent");
	}

}

static void cma_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

static int cma_area_cb(struct cma *cma, void *data)
{
	struct seq_file *s = (struct seq_file *)data;
	const char *name = cma_get_name(cma);
	unsigned long total = cma->count << PAGE_SHIFT;
	unsigned long used = cma_get_used(cma);
	unsigned long start = PFN_PHYS(cma->base_pfn);

	if (!total)
		return 0;

	if (s)
		seq_printf(s, "%15lu %30s %15lu %15lu %10lu\n",
					start, name, total, used, used * 100 / total);
	else
		memcheck_info("%15lu %30s %15lu %15lu %10lu\n",
					start, name, total, used, used * 100 / total);

	return 0;
}

static int cma_info_show(struct seq_file *s, void *data)
{
	unsigned long old_ns = ktime_get();

	cma_info_print_header(s);

	cma_for_each_area(cma_area_cb, s);

	memcheck_info("take %llu ns", ktime_get() - old_ns);
	cma_info_print_foot(s);

	return 0;
}

void memcheck_cma_info_show(void)
{
	cma_info_show(NULL, NULL);
}
EXPORT_SYMBOL(memcheck_cma_info_show);

static int cma_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, cma_info_show, pde_data(inode));
}

static const struct proc_ops cma_info_fops = {
	.proc_open = cma_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int memcheck_cma_createfs(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create("cma_info", 0444, parent, &cma_info_fops);
	if (!entry)
		memcheck_err("Failed to create cma debug info\n");

	INIT_WORK(&cma_report_work, cma_report_work_func);

	return (!entry ? -EFAULT : 0);
}