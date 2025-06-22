// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cpumask.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <soc/xring/walt.h>
#include "walt.h"
#include "vip_sched.h"

static struct dentry *xr_debugfs_sched;
static u32 xr_debug_group_id;

/***********************************************
 * VIP Sched debugfs
 */

#if IS_ENABLED(CONFIG_XRING_SCHED_VIP)
#define WR_BUF_LEN		64
static ssize_t vip_task_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *ppos)
{
	char tmp[WR_BUF_LEN];
	size_t len;
	pid_t pid;
	int prio, ret;
	struct task_struct *p;

	len = min((int)count, WR_BUF_LEN - 1);
	if (copy_from_user(tmp, buf, len))
		return -EINVAL;

	tmp[len] = '\0';

	if (sscanf(tmp, "%d %d", &pid, &prio) != 2)
		return -EINVAL;

	pr_info("pid=%d, prio=%d\n", (int)pid, prio);

	rcu_read_lock();
	p = find_process_by_pid(pid);
	if (!p) {
		rcu_read_unlock();
		return -ESRCH;
	}

	get_task_struct(p);
	rcu_read_unlock();

	ret = set_vip_prio(p, prio, true);
	if (ret)
		pr_err("set vip prio failed\n");

	put_task_struct(p);

	return count;
}

static int vip_task_show(struct seq_file *filp, void *data)
{
	struct vip_task *vip;
	struct task_struct *q, *p;

	read_lock(&tasklist_lock);

	for_each_process_thread(q, p) {
		vip = task_to_vip(p);
		if (vip->prio)
			seq_printf(filp, "task %d: prio=%d, cpumask=0x%lx\n",
					p->pid, vip->prio, cpumask_bits(p->cpus_ptr)[0]);
	}

	read_unlock(&tasklist_lock);

	return 0;
}

static int vip_task_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, vip_task_show, inode->i_private);
}

static const struct file_operations vip_task_fops = {
	.open	= vip_task_open,
	.read	= seq_read,
	.write	= vip_task_write,
};

void vip_build_debugfs(void)
{
	debugfs_create_file("vip_task", 0660, xr_debugfs_sched,
			NULL, &vip_task_fops);
}
#endif /* CONFIG_XRING_SCHED_VIP */

static int xr_related_group_show(struct seq_file *s, void *data)
{
	struct walt_related_thread_group *grp = NULL;
	struct walt_task_struct *wts = NULL;
	struct task_struct *p = NULL;
	u64 prev_util = 0;
	u64 prev_usage = 0;
	unsigned long flag;
	unsigned int i;
	u32 group_id = xr_debug_group_id;

	grp = get_related_thread_group(group_id);
	if (grp == NULL)
		return 0;

	seq_printf(s, "RTG %d tasks:\n", group_id);

	raw_spin_lock_irqsave(&grp->lock, flag);

	list_for_each_entry(wts, &grp->tasks, grp_list) {
		p = wts_to_ts(wts);
		seq_printf(s, "\t%d\n", p->pid);
	}
	seq_printf(s, "lock freq:\t%s\n", grp->xr_lock_freq ? "on" : "off");
	seq_printf(s, "prefer util:\t%llu\n", grp->xr_util);
	seq_printf(s, "prefer cpus:\t%*pb\n", cpumask_pr_args(&grp->xr_prefer_cpus));
	/* window time is a shifted value */
	seq_printf(s, "prev window:\t%llu ns\n",
		   grp->xr_prev_window_time << SCHED_CAPACITY_SHIFT);
	if (grp->xr_prev_window_time) {
		prev_util = grp->xr_prev_running_load / grp->xr_prev_window_time;
		prev_usage = grp->xr_prev_running_exec / grp->xr_prev_window_time;
	}
	seq_printf(s, "prev util:\t%llu\n", prev_util);
	seq_printf(s, "prev usage:\t%llu%%\n",
		   (prev_usage * 100) >> SCHED_CAPACITY_SHIFT);
	seq_printf(s, "prev load:\t%llu\n", grp->xr_prev_running_load);
	seq_printf(s, "prev exec:\t%llu ns\n", grp->xr_prev_running_exec);
	seq_printf(s, "curr load:\t%llu\n", grp->xr_curr_running_load);
	seq_printf(s, "curr exec:\t%llu ns\n", grp->xr_curr_running_exec);
	for (i = 0; i < WALT_NR_CPUS; i++) {
		seq_printf(s, "CPU%u:\n", i);
		seq_printf(s, "\tprev load:\t%llu\n", grp->xr_prev_window_cpu[i]);
		seq_printf(s, "\tcurr load:\t%llu\n", grp->xr_curr_window_cpu[i]);
	}
	seq_printf(s, "ed running threshold:\t%u\n", grp->xr_ed_run_thres);
	seq_printf(s, "ed first running threshold:\t%u\n", grp->xr_ed_first_run_thres);
	seq_printf(s, "ed runnable threshold:\t%u\n", grp->xr_ed_wait_thres);

	raw_spin_unlock_irqrestore(&grp->lock, flag);

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(xr_related_group);

void xr_sched_init_debug(void)
{
	xr_debugfs_sched = debugfs_create_dir("xr_sched", NULL);

	debugfs_create_file("group_info", 0440, xr_debugfs_sched,
			    NULL, &xr_related_group_fops);
	debugfs_create_u32("group_id", 0640, xr_debugfs_sched,
			   &xr_debug_group_id);

#if IS_ENABLED(CONFIG_XRING_SCHED_VIP)
	vip_build_debugfs();
#endif
}
