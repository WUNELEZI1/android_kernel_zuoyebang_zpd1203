// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fdtable.h>
#include <linux/sync_file.h>
#include <linux/net.h>
#include <net/sock.h>
#include <linux/dma-fence.h>
#include <linux/dma-fence-array.h>

#include "memcheck_ioctl.h"


static void socket_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_puts(s, "Process socket info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%15s %10s %5s %5s %10s\n",
					"ProcessName", "ProcessID", "Fd", "inode", "PeerTid");
	} else {
		memcheck_info("Process socket info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%15s %10s %5s %5s %10s\n",
						"ProcessName", "ProcessID", "Fd", "inode", "PeerTid");
	}
}
static void socket_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

struct socket_info_args {
	struct seq_file *seq;
	struct task_struct *task;
};

static int socket_info_cb(const void *data, struct file *f, unsigned int fd)
{
	struct socket_info_args *args = (struct socket_info_args *)data;
	struct task_struct *task = args->task;
	struct socket *sock = NULL;
	struct sock *sk = NULL;
	pid_t peer_tid;


	sock = sock_from_file(f);
	if (!sock || !sock->sk)
		return 0;
	if (unlikely(!sock))
		return 0;

	sk = sock->sk;
	spin_lock(&sk->sk_peer_lock);
	peer_tid = pid_vnr(sk->sk_peer_pid);
	spin_unlock(&sk->sk_peer_lock);
	seq_printf(args->seq, "%15s %10u %5u %5zu %10u\n", task->comm, task->pid, fd, file_inode(f)->i_ino, peer_tid);

	return 0;
}

static int socket_info_show(struct seq_file *s, void *d)
{
	struct task_struct *task = NULL;
	struct socket_info_args cb_args;
	unsigned long old_ns = ktime_get();

	socket_info_print_header(s);

	rcu_read_lock();
	for_each_process(task) {
		if (task->flags & PF_KTHREAD)
			continue;

		cb_args.seq = s;
		cb_args.task = task;

		task_lock(task);
		iterate_fd(task->files, 0, socket_info_cb, (void *)&cb_args);
		task_unlock(task);
	}
	rcu_read_unlock();

	memcheck_info("take %llu ns", ktime_get() - old_ns);
	socket_info_print_foot(s);

	return 0;
}

static int socket_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, socket_info_show, pde_data(inode));
}

static const struct proc_ops socket_info_fops = {
	.proc_open = socket_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int memcheck_fd_socket_createfs(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create("socket_process_info", 0444, parent, &socket_info_fops);
	if (!entry)
		memcheck_err("Failed to create socket debug info\n");

	return (!entry ? -EFAULT : 0);
}
