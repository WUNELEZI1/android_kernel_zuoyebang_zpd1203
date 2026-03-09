// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/seq_file.h>
#include <linux/fdtable.h>
#include <linux/proc_fs.h>
#include <linux/pipe_fs_i.h>

#include "memcheck_ioctl.h"

struct pipe_info_args {
	struct seq_file *seq;
	struct task_struct *task;
};

static struct file_operations *mmcheck_pipefifo_fops;

static void pipe_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_puts(s, "Process pipe info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%s %s %s %s %s %s %s %s\n",
					"ProcessName", "ProcessID", "Fd", "PipeName", "inode",
					"MaxUsage", "NumAccounted", "RingSize");
	} else {
		memcheck_info("Process pipe info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%s %s %s %s %s %s %s %s\n",
						"ProcessName", "ProcessID", "Fd", "PipeName", "inode",
						"MaxUsage", "NumAccounted", "RingSize");
	}
}

static void pipe_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

static struct pipe_inode_info *mi_get_pipe_info(struct file *file, bool for_splice)
{
	struct pipe_inode_info *pipe = file->private_data;

	if (!mmcheck_pipefifo_fops)
		return NULL;
	if (file->f_op != mmcheck_pipefifo_fops || !pipe)
		return NULL;
	if (for_splice && pipe_has_watch_queue(pipe))
		return NULL;
	return pipe;
}

static int pipe_info_cb(const void *data, struct file *f, unsigned int fd)
{
	struct pipe_info_args *args = (struct pipe_info_args *)data;
	struct task_struct *task = args->task;
	struct pipe_inode_info *pipe = NULL;

	pipe = mi_get_pipe_info(f, false);
	if (!pipe)
		return 0;

	seq_printf(args->seq, "%12s %10u %3u %10s %10zu %10u %10u %u10\n",
		   task->comm, task->pid, fd, f->f_path.dentry->d_iname, file_inode(f)->i_ino,
		   pipe->max_usage, pipe->nr_accounted, pipe->ring_size);

	return 0;
}

static int pipe_info_show(struct seq_file *s, void *d)
{
	struct task_struct *task = NULL;
	struct pipe_info_args cb_args;
	unsigned long old_ns = ktime_get();

	pipe_info_print_header(s);

	rcu_read_lock();
	for_each_process(task) {
		if (task->flags & PF_KTHREAD)
			continue;

		cb_args.seq = s;
		cb_args.task = task;

		task_lock(task);
		iterate_fd(task->files, 0, pipe_info_cb, (void *)&cb_args);
		task_unlock(task);
	}
	rcu_read_unlock();

	memcheck_info("take %llu ns", ktime_get() - old_ns);

	pipe_info_print_foot(s);
	return 0;
}

static int pipe_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, pipe_info_show, pde_data(inode));
}

static const struct proc_ops pipe_info_fops = {
	.proc_open = pipe_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int memcheck_fd_pipe_createfs(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *entry = NULL;

	MEMCHECK_DEBUG_LOOKUP(pipefifo_fops, struct file_operations);

	if (!mmcheck_pipefifo_fops)
		return -EFAULT;

	entry = proc_create("pipe_process_info", 0444, parent, &pipe_info_fops);
	if (!entry)
		memcheck_err("Failed to create pipe debug info\n");

	return (!entry ? -EFAULT : 0);
}
