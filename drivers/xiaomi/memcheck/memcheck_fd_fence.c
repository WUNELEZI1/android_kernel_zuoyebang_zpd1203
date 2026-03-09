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
#include <linux/sync_file.h>
#include <linux/fdtable.h>
#include <linux/file.h>

#include "memcheck_ioctl.h"
#include "memcheck_fd_fence.h"

struct fence_info_args {
	struct seq_file *seq;
	struct task_struct *task;
};

static struct file_operations *mmcheck_sync_file_fops;

static void fence_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_puts(s, "Process fence info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%15s %10s %5s %10s %5s %10s\n",
					"ProcessName", "ProcessID", "Fd", "FenceName", "inode", "FenceNum");
		seq_printf(s, "	%15s %10s %5s %10s\n",
					"TimelineName", "DriverName", "Status", "Timestamp");


	} else {
		memcheck_info("Process fence info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%15s %10s %5s %10s %5s %10s\n",
						"ProcessName", "ProcessID", "Fd", "FenceName", "inode", "FenceNum");
		memcheck_info("	%15s %10s %5s %10s\n",
						"TimelineName", "DriverName", "Status", "Timestamp");
	}
}

static void fence_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

static int is_sync_file(struct file *f)
{
	if (!f)
		return 0;

	if (!mmcheck_sync_file_fops)
		return 0;

	return (f->f_op == mmcheck_sync_file_fops);
}

static int fence_info_cb(const void *data, struct file *f, unsigned int fd)
{
	struct fence_info_args *args = (struct fence_info_args *)data;
	struct task_struct *task = args->task;
	struct sync_file *sync_file = NULL;
	struct dma_fence **fences = NULL;
	int i;
	int num_fences;
	char *name = NULL;

	if (!is_sync_file(f))
		return 0;

	sync_file = (struct sync_file *)f->private_data;
	get_file(sync_file->file);
	if (dma_fence_is_array(sync_file->fence)) {
		struct dma_fence_array *array = to_dma_fence_array(sync_file->fence);

		num_fences = array->num_fences;
		fences = array->fences;
	} else {
		num_fences = 1;
		fences = &sync_file->fence;
	}

	if (strlen(sync_file->user_name)) {
		name = sync_file->user_name;
	} else {
		name = "NULL";
	}
	seq_printf(args->seq, "%15s %10u %5u %10s %5zu %10u\n",
				task->comm, task->pid, fd, name, file_inode(f)->i_ino, num_fences);

	for (i = 0; i < num_fences; i++)
		seq_printf(args->seq, "%d   %15s %10s %5d %10lld\n",
					i, fences[i]->ops->get_timeline_name(fences[i]),
					fences[i]->ops->get_driver_name(fences[i]), dma_fence_get_status(fences[i]),
					ktime_to_ns(fences[i]->timestamp));
	fput(sync_file->file);

	return 0;
}

static int fence_info_show(struct seq_file *s, void *data)
{
	struct task_struct *task = NULL;
	struct fence_info_args cb_args;
	unsigned long old_ns = ktime_get();

	fence_info_print_header(s);
	rcu_read_lock();
	for_each_process(task) {
		if (task->flags & PF_KTHREAD)
			continue;

		cb_args.seq = s;
		cb_args.task = task;

		task_lock(task);
		iterate_fd(task->files, 0, fence_info_cb, (void *)&cb_args);
		task_unlock(task);
	}
	rcu_read_unlock();

	memcheck_info("take %llu ns", ktime_get() - old_ns);
	fence_info_print_foot(s);

	return 0;
}

static int fence_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, fence_info_show, pde_data(inode));
}

static const struct proc_ops fence_info_fops = {
	.proc_open = fence_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int memcheck_fd_fence_createfs(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *entry = NULL;

	MEMCHECK_DEBUG_LOOKUP(sync_file_fops, struct file_operations);

	if (!mmcheck_sync_file_fops)
		return -EFAULT;

	entry = proc_create("fence_process_info", 0444,	parent, &fence_info_fops);
	if (!entry)
		memcheck_err("Failed to create fence debug info\n");

	return (!entry ? -EFAULT : 0);
}
