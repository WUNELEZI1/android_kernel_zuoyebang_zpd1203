// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */


#include <linux/dma-buf.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/sched/task.h>
#include <linux/seq_file.h>
#include <linux/ww_mutex.h>
#include <linux/dma-resv.h>

#include "memcheck_ioctl.h"
#include "memcheck_dma-buf.h"

struct dmabuf_info_args {
	struct seq_file *seq;
	struct task_struct *task;
	size_t sum;
};

static void dmabuf_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_puts(s, "Process dmabuf heap info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%-8s\t%-8s\t%-8s\t%-8s\texp_name\t%-8s\n",
					"size", "flags", "mode", "count", "ino");
	} else {
		memcheck_info("Process dmabuf heap info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%-8s\t%-8s\t%-8s\t%-8s\texp_name\t%-8s\n",
						"size", "flags", "mode", "count", "ino");;
	}
}

static void dmabuf_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

static struct dma_buf *file_to_dma_buf(struct file *file)
{
	return file->private_data;
}

static int dmabuf_info_cb(const void *data, struct file *f, unsigned int fd)
{
	struct dmabuf_info_args *args = (struct dmabuf_info_args *)data;
	struct dma_buf *dbuf = NULL;

	//todo: oem dma buf fops
	if (!is_dma_buf_file(f))
		return 0;

	dbuf = file_to_dma_buf(f);
	if (!dbuf)
		return 0;

	//tips:used for refcounting when exporter is a kernel module.
	// if (dbuf->owner != THIS_MODULE)
		// return 0;

	if (args->sum == 0) {
		if (args->seq) {
			seq_printf(args->seq, "\n%s %d\n",
						args->task->comm, args->task->pid);
		} else {
			memcheck_info("\n%s %d\n", args->task->comm, args->task->pid);
		}
	}
	if (args->seq) {
		seq_printf(args->seq, "%08zu\t%08x\t%08x\t%08ld\t%s\t%08lu\t%s\n",
					dbuf->size, dbuf->file->f_flags, dbuf->file->f_mode,
					file_count(dbuf->file), dbuf->exp_name,
					file_inode(dbuf->file)->i_ino,
					dbuf->name ?: "");
	} else {
		memcheck_info("%08zu\t%08x\t%08x\t%08ld\t%s\t%08lu\t%s\n",
					dbuf->size, dbuf->file->f_flags, dbuf->file->f_mode,
					file_count(dbuf->file), dbuf->exp_name,
					file_inode(dbuf->file)->i_ino,
					dbuf->name ?: "");
	}
	args->sum += dbuf->size;
	return 0;
}

static int dmabuf_info_show(struct seq_file *s, void *d)
{
	struct task_struct *task = NULL;
	struct task_struct *thread = NULL;
	struct files_struct *files;
	struct dmabuf_info_args cb_args;
	unsigned long old_ns = ktime_get();

	dmabuf_info_print_header(s);

	rcu_read_lock();
	for_each_process(task) {
		// if (task->flags & PF_KTHREAD)
		//	 continue;
		struct files_struct *group_leader_files = NULL;
		cb_args.sum = 0;
		cb_args.seq = s;
		cb_args.task = task;
		for_each_thread(task, thread) {
			task_lock(thread);
			if (unlikely(!group_leader_files))
				group_leader_files = task->group_leader->files;
			files = thread->files;
			if (files && (group_leader_files != files ||
							thread == task->group_leader))
				iterate_fd(files, 0, dmabuf_info_cb, (void *)&cb_args);
			task_unlock(thread);
		}
		if (s && cb_args.sum)
			seq_printf(s, "Total dmabuf usage for %s is %zu bytes\n",
						task->comm, cb_args.sum);
	}
	rcu_read_unlock();

	memcheck_info("take %llu ns", ktime_get() - old_ns);
	dmabuf_info_print_foot(s);

	return 0;
}

void memcheck_dmabuf_info_show(void)
{
	dmabuf_info_show(NULL, NULL);
}
EXPORT_SYMBOL(memcheck_dmabuf_info_show);

static int dmabuf_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, dmabuf_info_show, pde_data(inode));
}

struct dma_loop_args {
	struct seq_file *seq;
	int count;
	size_t size;
};

static inline int print_dma_info(const struct dma_buf *dmabuf, void *private)
{
	int ret;
	struct dma_loop_args *args = (struct dma_loop_args *)private;
	struct dma_buf_attachment *attach_obj;
	int attach_count;

	ret = dma_resv_lock(dmabuf->resv, NULL);
	if (ret)
		return -EINVAL;

	seq_printf(args->seq, "%08zu\t%08x\t%08x\t%08ld\t%s\t%08lu\t%s\n",
				dmabuf->size, dmabuf->file->f_flags, dmabuf->file->f_mode,
				file_count(dmabuf->file), dmabuf->exp_name,
				file_inode(dmabuf->file)->i_ino,
				dmabuf->name ?: "");
	//todo: dma_resv_for_each_fence
	seq_printf(args->seq, "\tAttached Devices:\n");
	attach_count = 0;

	list_for_each_entry(attach_obj, &dmabuf->attachments, node) {
		seq_printf(args->seq, "\t%s\n", dev_name(attach_obj->dev));
		attach_count++;
	}
	dma_resv_unlock(dmabuf->resv);

	seq_printf(args->seq, "Total %d devices attached\n", attach_count);

	args->count +=1;
	args->size += dmabuf->size;

	return ret;
}

static int dmabuf_memtrack_show(struct seq_file *m, void *d)
{
	struct dma_loop_args args;
	unsigned long old_ns = ktime_get();

	args.seq = m;
	seq_printf(args.seq, "%-8s\t%-8s\t%-8s\t%-8s\texp_name\t%-8s\n",
				"size", "flags", "mode", "count", "ino");

	if (get_dmabuf_debugfs_data(print_dma_info, (void *)&args))
		memcheck_err("Error occurred during dmabuf debugfs data collection!\n");

	seq_printf(args.seq, "Total %d objects, %zu bytes\n",
				args.count, args.size);
	memcheck_info("take %llu ns", ktime_get() - old_ns);
	return 0;
}

static inline int account_dma_info(const struct dma_buf *dmabuf, void *private)
{
	unsigned long *total = (unsigned long *)private;
	*total += dmabuf->size;
	return 0;
}

unsigned long memcheck_dmabuf_get_total(void)
{
	unsigned long total = 0;
	if (get_dmabuf_debugfs_data(account_dma_info, (void *)&total))
		memcheck_err("Error occurred during dmabuf debugfs data collection!\n");
	return total;
}

static int dmabuf_memtrack_open(struct inode *inode, struct file *file)
{
	return single_open(file, dmabuf_memtrack_show, pde_data(inode));
}

static const struct proc_ops dmabuf_info_fops = {
	.proc_open = dmabuf_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static const struct proc_ops memtrack_fops = {
	.proc_open = dmabuf_memtrack_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int memcheck_dmabuf_createfs(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create("dmabuf_process_info", 0444,
						parent, &dmabuf_info_fops);
	if (!entry)
		memcheck_err("Failed to create dmabuf buffer debug info\n");

	entry = proc_create("memtrack_dmabuf", 0444,
						parent, &memtrack_fops);
	if (!entry)
		memcheck_err("Failed to create heap debug memtrack\n");

	return (!entry ? -EFAULT : 0);
}