// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <linux/dma-buf.h>
#include <linux/fdtable.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/ktime.h>

#include "memcheck_ioctl.h"
#include "memcheck_process_mem.h"

struct proc_meminfo {
	int rss_size;
	int ion_size;
	pid_t pid;
	char task_name[TASK_COMM_LEN];
	struct list_head node_task;
};

struct pmem_event_info {
	pid_t pid;
	// pid_t tgid;
	unsigned long rss_used;
	unsigned long ion_used;
	char comm[TASK_COMM_LEN];
};

#define TOPN_DEFAULT 3

static unsigned int top_n = TOPN_DEFAULT;
static struct kmem_cache *process_cache;
static DEFINE_MUTEX(dump_class_mutex);
static struct list_head dump_class;

struct pmem_report_info {
	struct pmem_event_info top1;
	struct pmem_event_info top2;
	struct pmem_event_info top3;
} report_info;


static struct dma_buf *file_to_dma_buf(struct file *file)
{
	return file->private_data;
}

ssize_t	process_mem_write(struct file *file, const char __user *buffer,
							size_t count, loff_t *ppos)
{
	int rv;

	rv = kstrtouint_from_user(buffer, count, 10, &top_n);
	if (rv < 0)
		return rv;
	if (top_n > 100)
		top_n = TOPN_DEFAULT;

	if (top_n == 0)
		top_n = TOPN_DEFAULT;

	return count;
}

static unsigned long process_get_ion_size(struct task_struct *task)
{
	int n = 0;
	size_t ion_size = 0;
	struct files_struct	*files = NULL;
	struct fdtable *fdt = NULL;

	if (task->flags & PF_KTHREAD)
		return ion_size;

	files = task->files;
	if (!files)
		return ion_size;

	spin_lock(&files->file_lock);
	for (fdt = files_fdtable(files); n < fdt->max_fds; n++) {
		struct dma_buf *dbuf = NULL;
		struct file *f = rcu_dereference_check_fdtable(files,
					fdt->fd[n]);

		if (!f)
			continue;
		if (!is_dma_buf_file(f))
			continue;
		dbuf = file_to_dma_buf(f);
		if (!dbuf)
			continue;
		// if (dbuf->owner != THIS_MODULE)
			// continue;
		if (!dbuf->priv)
			continue;
		ion_size +=dbuf->size;
	}
	spin_unlock(&files->file_lock);

	return (unsigned long)TO_KB(ion_size);
}

static struct proc_meminfo *process_info_add(const struct task_struct *task,
											int rss_size, int ion_size)
{
	struct proc_meminfo *dump_info = NULL;

	dump_info = kmem_cache_zalloc(process_cache, GFP_ATOMIC);
	if (dump_info && task) {
		memcpy(dump_info->task_name, task->comm, TASK_COMM_LEN);
		dump_info->rss_size = rss_size;
		dump_info->ion_size = ion_size;
		dump_info->pid = task->pid;
	}
	return dump_info;
}

/*
 * The process p may have detached its own ->mm while exiting or through
 * kthread_use_mm(), but one or more of its subthreads may still have a valid
 * pointer.  Return p, or any of its subthreads with a valid ->mm, with
 * task_lock() held.
 */
static struct task_struct *find_lock_task_mm(struct task_struct *p)
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

static void rss_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_printf(s, "Process RSS info:\n");
		seq_printf(s, "----------------------------------------------------\n");
		seq_printf(s, "pid  common		  total:rss:ion    top%d\n", top_n);
	} else {
		memcheck_info("Process RSS info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("pid  common		  total:rss:ion    top%d\n", top_n);
	}
}

static void rss_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

int process_mem_scan(void)
{
	int count;
	struct task_struct *p;
	struct task_struct *task;
	unsigned long rss_size;
	unsigned long ion_size;
	struct proc_meminfo *dump_info = NULL;
	struct proc_meminfo *temp_info = NULL;
	ktime_t old_ns = ktime_get();

	while (!list_empty(&dump_class)){
		temp_info = list_entry(dump_class.next, struct proc_meminfo, node_task);
		list_del(&temp_info->node_task);
		kmem_cache_free(process_cache, temp_info);
		temp_info = NULL;
	}

	rcu_read_lock();
	for_each_process(p) {
		if (!p) continue;

		task = find_lock_task_mm(p);
		if (!task)
			continue;

		rss_size = get_mm_rss(task->mm) +
					get_mm_counter(task->mm, MM_SWAPENTS);
		rss_size = PAGES_TO_KB(rss_size);
		ion_size = process_get_ion_size(task);

		if (list_empty(&dump_class)) {
			dump_info = process_info_add(task, rss_size, ion_size);
			if (dump_info)
				list_add(&dump_info->node_task, &dump_class);
			goto t_unlock;
		}

		count = 0;
		list_for_each_entry(temp_info, &dump_class, node_task) {
			count++;
			if (count >= top_n)
				break;
			if ((rss_size + ion_size) > (temp_info->rss_size + temp_info->ion_size)) {
				dump_info = process_info_add(task, rss_size, ion_size);
				if (dump_info)
					list_add_tail(&dump_info->node_task,
									&temp_info->node_task);
				break;
			} else if (list_is_last(&temp_info->node_task, &dump_class)) {
				dump_info = process_info_add(task, rss_size, ion_size);
				if (dump_info)
					list_add(&dump_info->node_task,
								&temp_info->node_task);
				break;
			}
		}
t_unlock:
		task_unlock(task);
	}
	rcu_read_unlock();
	memcheck_info("take %llu ns", ktime_get() - old_ns);

	return 0;
}

static int process_mem_show(struct seq_file *m, void *v)
{
	int count = 0;
	struct proc_meminfo *temp_info = NULL;

	mutex_lock(&dump_class_mutex);
	process_mem_scan();

	if (list_empty(&dump_class)) {
		memcheck_err("dump_class is empty\n");
		mutex_unlock(&dump_class_mutex);
		return -1;
	}

	rss_info_print_header(m);
	list_for_each_entry(temp_info, &dump_class, node_task) {
		if (!temp_info)
			continue;
		if (count++ < top_n) {
			if (m) {
				seq_printf(m, "%-5d %-16s %dKB:%dKB:%dKB\n",
							temp_info->pid, temp_info->task_name,
							temp_info->rss_size + temp_info->ion_size,
							temp_info->rss_size, temp_info->ion_size);
			} else {
				memcheck_info("pid: %-5d, comm: %-16s, size:%dKB, rss:%dKB, ion:%dKB\n",
							temp_info->pid, temp_info->task_name,
							temp_info->rss_size + temp_info->ion_size,
							temp_info->rss_size, temp_info->ion_size);
			}
		}
	}
	rss_info_print_foot(m);
	mutex_unlock(&dump_class_mutex);

	return 0;
}

void memcheck_top_pmem_info(unsigned int topN)
{
	if (topN <= 0 || topN > 100)
		top_n = TOPN_DEFAULT;

	top_n = topN;

	process_mem_show(NULL, NULL);
}

static int process_mem_open(struct inode *inode, struct file *file)
{
	return single_open(file, process_mem_show, pde_data(inode));
}

static const struct proc_ops process_mem_fops = {
	.proc_open	= process_mem_open,
	.proc_write	= process_mem_write,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};

int memcheck_pmem_createfs(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *proc_entry_mem = NULL;

	INIT_LIST_HEAD(&dump_class);

	proc_entry_mem = proc_create("pmem_info", 0660, parent, &process_mem_fops);
	if (!proc_entry_mem) {
		memcheck_err("can't create %s\n", "pmem_info");
		return -ENOMEM;
	}

	process_cache = kmem_cache_create("pmeminfo_cache",
			sizeof(struct proc_meminfo), 0, 0, NULL);
	if (!process_cache)
		return -ENOMEM;

	return 0;
}