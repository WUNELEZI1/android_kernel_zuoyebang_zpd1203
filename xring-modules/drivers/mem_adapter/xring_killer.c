// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Xring Mem adapter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the ION heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/sched.h>
#include <linux/oom.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/mm.h>
#include <soc/xring/xring_mem_adapter.h>

#include "xring_mem_adapter.h"

#define XRING_MAX_KILL_NUM 20
#define XRING_KILL_PROCESS_ADJ 600

/* copy from oom_kill.c - find_lock_task_mm */
struct task_struct *find_task_mm(struct task_struct *p)
{
	struct task_struct *t = NULL;

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
EXPORT_SYMBOL(find_task_mm);

static void xring_selected(struct task_struct *selected,
			unsigned long selected_task_size)
{
	task_lock(selected);
	send_sig(SIGKILL, selected, 0);
	task_unlock(selected);

	xrmem_info("kill task is %s PID is %d\n", selected->comm, selected->pid);
}

unsigned long xring_direct_kill(int model, unsigned long reclaim_by_kill)
{
	int nr_to_kill = XRING_MAX_KILL_NUM;
	unsigned long selected_task_size = 0;
	struct task_struct *selected = NULL;
	struct task_struct *tsk = NULL;
	struct task_struct *p = NULL;
	unsigned long task_size = 0;
	short oom_score_adj = 0;
	unsigned long rem = 0;

	rcu_read_lock();
	do {
		for_each_process(tsk) {
			if (tsk->flags & PF_KTHREAD)
				continue;

			p = find_task_mm(tsk);
			if (!p)
				continue;

			if (selected == p) {
				task_unlock(p);
				continue;
			}

			if (test_tsk_thread_flag(p, TIF_MEMDIE)) {
				task_unlock(p);
				continue;
			}

			if ((unsigned long)p->__state & TASK_UNINTERRUPTIBLE) {
				task_unlock(p);
				xrmem_debug("filter D state process: %d (%s) state:0x%x\n",
					p->pid, p->comm, p->__state);
				continue;
			}

			oom_score_adj = p->signal->oom_score_adj;
			if (oom_score_adj < XRING_KILL_PROCESS_ADJ) {
				task_unlock(p);
				continue;
			}

			if (!model)
				task_size = get_mm_rss(p->mm);
			else
				task_size = get_mm_counter(p->mm, MM_SWAPENTS);

			task_unlock(p);

			if (task_size <= 0)
				continue;

			if (selected) {
				if (task_size < selected_task_size)
					continue;
			}
			selected = p;
			selected_task_size = task_size;
		}

		if (selected) {
			xring_selected(selected, selected_task_size);
			rem += selected_task_size;
		}

		tsk = NULL;
		selected_task_size = 0;
		xrmem_debug("selected: 0x%llx\n", (unsigned long long)selected);
	} while ((rem < reclaim_by_kill) && (--nr_to_kill));

	rcu_read_unlock();

	return rem;
}
EXPORT_SYMBOL(xring_direct_kill);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring MEM_ADAPTER Driver");
MODULE_LICENSE("GPL v2");
