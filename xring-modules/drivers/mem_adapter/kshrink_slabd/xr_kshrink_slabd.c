// SPDX-License-Identifier: GPL-2.0
/*
 * XRING Page Table implementation
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/module.h>
#include <trace/hooks/vmscan.h>
#include <linux/swap.h>
#include <linux/proc_fs.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/cpufreq.h>
#include <linux/freezer.h>
#include <linux/wait.h>

#include "../xring_mem_adapter.h"

static int kshrink_slabd_pid;
static struct task_struct *shrink_slabd_tsk;
static bool async_shrink_slabd_setup;

wait_queue_head_t shrink_slabd_wait;

struct async_slabd_parameter {
	struct mem_cgroup *shrink_slabd_memcg;
	gfp_t shrink_slabd_gfp_mask;
	atomic_t shrink_slabd_runnable;
	int shrink_slabd_nid;
	int priority;
} asp;

static struct reclaim_state async_reclaim_state = {
		.reclaimed = 0,
};

static bool is_shrink_slabd_task(struct task_struct *tsk)
{
	return tsk->pid == kshrink_slabd_pid;
}

static int kshrink_slabd_func(void *p)
{
	struct mem_cgroup *memcg;
	gfp_t gfp_mask;
	int nid, priority;
	/*
	 * Tell the memory management that we're a "memory allocator",
	 * and that if we need more memory we should get access to it
	 * regardless (see "__alloc_pages()"). "kswapd" should
	 * never get caught in the normal page freeing logic.
	 *
	 * (Kswapd normally doesn't need memory anyway, but sometimes
	 * you need a small amount of memory in order to be able to
	 * page out something else, and this flag essentially protects
	 * us from recursively trying to free more memory as we're
	 * trying to free the first piece of memory in the first place).
	 */
	current->flags |= PF_MEMALLOC | PF_KSWAPD;
	set_freezable();

	current->reclaim_state = &async_reclaim_state;
	asp.shrink_slabd_gfp_mask = 0;
	asp.shrink_slabd_nid = 0;
	asp.shrink_slabd_memcg = NULL;
	atomic_set(&(asp.shrink_slabd_runnable), 0);
	asp.priority = 0;

	while (!kthread_should_stop()) {
		wait_event_freezable(shrink_slabd_wait,
					(atomic_read(&(asp.shrink_slabd_runnable)) == 1));

		nid = asp.shrink_slabd_nid;
		gfp_mask = asp.shrink_slabd_gfp_mask;
		priority = asp.priority;
		memcg = asp.shrink_slabd_memcg;

		shrink_slab(gfp_mask, nid, memcg, priority);

		css_put(&memcg->css);

		atomic_set(&(asp.shrink_slabd_runnable), 0);
	}
	current->flags &= ~(PF_MEMALLOC | PF_KSWAPD);
	current->reclaim_state = NULL;

	return 0;
}

bool wakeup_shrink_slabd(gfp_t gfp_mask, int nid,
				 struct mem_cgroup *memcg,
				 int priority)
{
	int old = 0;

	if (unlikely(!async_shrink_slabd_setup))
		return false;

	if (!atomic_try_cmpxchg(&(asp.shrink_slabd_runnable), &old, 1))
		return true;

	asp.shrink_slabd_gfp_mask = gfp_mask;
	asp.shrink_slabd_nid = nid;
	asp.shrink_slabd_memcg = memcg;
	asp.priority = priority;

	rcu_read_lock();
	if (!css_tryget_online(&asp.shrink_slabd_memcg->css)) {
		rcu_read_unlock();
		return true;
	}
	rcu_read_unlock();

	wake_up_interruptible(&shrink_slabd_wait);

	return true;
}

static void should_shrink_async(void *data, gfp_t gfp_mask, int nid,
						struct mem_cgroup *memcg, int priority, bool *bypass)
{
	if (unlikely(!async_shrink_slabd_setup)) {
		*bypass = false;
		return;
	}

	if (is_shrink_slabd_task(current)) {
		*bypass = false;
	} else {
		*bypass = true;
		wakeup_shrink_slabd(gfp_mask, nid, memcg, priority);
	}
}

int shrink_slabd_init(void)
{
	int ret;

	ret = register_trace_android_vh_shrink_slab_bypass(should_shrink_async, NULL);
	if (ret != 0) {
		xrmem_err("register_trace_android_vh_shrink_slab_bypass failed! ret=%d\n", ret);
		goto out;
	}

	init_waitqueue_head(&shrink_slabd_wait);

	shrink_slabd_tsk = kthread_run(kshrink_slabd_func, NULL, "kshrink_slabd");
	if (IS_ERR_OR_NULL(shrink_slabd_tsk)) {
		xrmem_err("Failed to start shrink_slabd on node 0\n");
		shrink_slabd_tsk = NULL;
		ret = PTR_ERR(shrink_slabd_tsk);
		goto out;
	}

	kshrink_slabd_pid = shrink_slabd_tsk->pid;
	async_shrink_slabd_setup = true;

	xrmem_info("kshrink_async_succeed!\n");

out:
	return ret;
}

void shrink_slabd_exit(void)
{
	unregister_trace_android_vh_shrink_slab_bypass(should_shrink_async, NULL);

	xrmem_info("shrink_async exit succeed!\n");
}
