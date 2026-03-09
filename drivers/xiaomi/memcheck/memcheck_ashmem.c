// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched/task.h>
#include <linux/rcupdate.h>
#include <linux/fdtable.h>
// #include <staging/android/ashmem.h>
#include <linux/limits.h>
#include <linux/ioctl.h>
#include <linux/compat.h>

#include "memcheck_ashmem.h"
#include "memcheck_ioctl.h"

/* support of 32bit userspace on 64bit platforms */
#ifdef CONFIG_COMPAT
#define COMPAT_ASHMEM_SET_SIZE		_IOW(__ASHMEMIOC, 3, compat_size_t)
#define COMPAT_ASHMEM_SET_PROT_MASK	_IOW(__ASHMEMIOC, 5, unsigned int)
#endif
#define ASHMEM_NAME_LEN		256
// #define ASHMEM_NAME_DEF		"dev/ashmem"

#define ASHMEM_NAME_PREFIX "dev/ashmem/"
#define ASHMEM_NAME_PREFIX_LEN (sizeof(ASHMEM_NAME_PREFIX) - 1)
#define ASHMEM_FULL_NAME_LEN (ASHMEM_NAME_LEN + ASHMEM_NAME_PREFIX_LEN)

struct ashmem_info_args {
	struct seq_file *seq;
	struct task_struct *task;
	size_t sum;
};

struct ashmem_area {
	char name[ASHMEM_FULL_NAME_LEN];
	struct list_head unpinned_list;
	struct file *file;
	size_t size;
	unsigned long prot_mask;
};

static struct mutex *mmcheck_ashmem_mutex;

static void ashmem_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_puts(s, "Process ashmem info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%s %s %s %s %s %s\n",
					"Process_name", "Process_ID",
					"fd", "ashmem_name", "inode", "size");
	} else {
		memcheck_info("Process ashmem info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%s %s %s %s %s %s\n",
					"Process_name", "Process_ID",
					"fd", "ashmem_name", "inode", "size");
	}
}
static void ashmem_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

static struct ashmem_area *file_to_ashmem(struct file *file)
{
	return file->private_data;
}

static int ashmem_info_cb(const void *data, struct file *f, unsigned fd)
{
	struct ashmem_info_args *args = (struct ashmem_info_args *)data;
	struct task_struct *task = args->task;
	struct ashmem_area *asma = NULL;

	// https://android-review.googlesource.com/c/kernel/common/+/3043597
	// https://android-review.googlesource.com/c/kernel/common/+/3477511
	// if (!is_ashmem_file(f))
		return 0;
	asma = file_to_ashmem(f);
	if (!asma)
		return 0;

	if (args->seq) {
		seq_printf(args->seq, "%-16s %-16d %-16d %-16s %-16lu %-16zu\n",
					task->comm, task->pid, fd, asma->name ?: "",
					file_inode(f)->i_ino, asma->size);
	} else {
		memcheck_info("%-16s %-16d %-16d %-16s %-16lu %-16zu\n",
						task->comm, task->pid, fd, asma->name ?: "",
						file_inode(f)->i_ino, asma->size);
	}
	args->sum += asma->size;

	return 0;
}

static int ashmem_info_show(struct seq_file *s, void *data)
{
	struct task_struct *task = NULL;
	struct ashmem_info_args cb_args;
	unsigned long old_ns = ktime_get();

	ashmem_info_print_header(s);

	//todo:export ashmem mutex
	mutex_lock(mmcheck_ashmem_mutex);
	rcu_read_lock();

	for_each_process(task) {
		if (task->flags & PF_KTHREAD)
			continue;
		cb_args.seq = s;
		cb_args.task = task;
		cb_args.sum = 0;

		task_lock(task);
		iterate_fd(task->files, 0, ashmem_info_cb, (void *)&cb_args);
		task_unlock(task);
		if (!s && cb_args.sum)
			memcheck_info("Total ashmem usage for %s is %zu\n",
							task->comm, cb_args.sum);
	}
	rcu_read_unlock();
	mutex_unlock(mmcheck_ashmem_mutex);

	memcheck_info("take %llu ns", ktime_get() - old_ns);
	ashmem_info_print_foot(s);

	return 0;
}

void memcheck_ashmem_info_show(void)
{
	if (mmcheck_ashmem_mutex == NULL)
		return ;
	ashmem_info_show(NULL, NULL);
}
EXPORT_SYMBOL(memcheck_ashmem_info_show);

static int ashmem_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, ashmem_info_show, pde_data(inode));
}

static const struct proc_ops ashmem_info_fops = {
	.proc_open = ashmem_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int memcheck_ashmem_createfs(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *entry = NULL;

	MEMCHECK_DEBUG_LOOKUP(ashmem_mutex, struct mutex);

	if (mmcheck_ashmem_mutex == NULL)
		return -EFAULT;

	entry = proc_create("ashmem_process_info", 0444, parent, &ashmem_info_fops);
	if (!entry)
		memcheck_err("Failed to create ashmem debug info\n");

	return (!entry ? -EFAULT : 0);
}