// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "mpam_partid_map_mgr.h"
#include "mpam_register.h"
#include <linux/cpuidle.h>
#include <linux/cpumask.h>
#include <linux/mutex.h>
#include <trace/hooks/cgroup.h>
#include <trace/hooks/mpam.h>
#include <trace/hooks/sched.h>

#define TAG "xring-mpam-mapmgr: "

const int mpam_subsys_id = cpuset_cgrp_id;
static u16 __read_mostly s_mpam_css_map[MPAM_CGROUP_SUBSYS_MAX] = { 0 };
static DEFINE_PER_CPU(u16, s_mpam_cpu_map);
static bool __read_mostly s_mpam_cpu_enable;
static DEFINE_MUTEX(s_mpam_mgr_config_lock);


static inline u16 __get_cpu_partid(int cpuid)
{
	return per_cpu(s_mpam_cpu_map, cpuid);
}

static inline void __set_cpu_partid(int cpuid, u16 partid)
{
	per_cpu(s_mpam_cpu_map, cpuid) = partid;
}

static inline void __set_cgroup_partid(int css_id, u16 partid)
{
	s_mpam_css_map[css_id] = partid;
}

static inline u16 __read_cgroup_partid(int css_id)
{
	return s_mpam_css_map[css_id];
}

static inline void __set_task_partid(struct task_struct *p, u16 partid)
{
	TASK_PARTID_DATA(p) = partid;
}

static inline u16 __get_task_partid(struct task_struct *p)
{
	return TASK_PARTID_DATA(p);
}

/* Must Use RCU Lock to protect */
static inline u16 __get_cgroup_partid(struct task_struct *p)
{
	struct cgroup_subsys_state *css = NULL;
	u16 partid = 0;

	css = task_css(p, mpam_subsys_id);

	if (css->id < MPAM_CGROUP_SUBSYS_MAX && css->id >= 0)
		partid = __read_cgroup_partid(css->id);
	else
		partid = 0;

	return partid;
}

static inline u16 __get_current_cpu_partid(void)
{
	return this_cpu_read(s_mpam_cpu_map);
}

static u16 mpam_get_partid_mapped(struct task_struct *p)
{
	u16 partid = 0;

	partid = __get_task_partid(p);
	if (!partid) {
		partid = __get_cgroup_partid(p);
		if (!partid)
			partid = __get_current_cpu_partid();
	}

	return partid;
}

static inline void mpam_sync_task(struct task_struct *p, bool do_recover)
{
	u16 partid = mpam_get_partid_mapped(p);

	mpam_write_core_partid(partid, partid, do_recover);
}

static void mpam_map_sync(void *data)
{
	if (!is_idle_task(current))
		mpam_sync_task(current, data != NULL);
}

static inline void mpam_partid_update_sync(void *task)
{
	if (task == current)
		mpam_map_sync(NULL);
}

static void mpam_task_fresh(struct task_struct *p)
{
	struct task_struct *task_cpu_current;

	/* Get The Task Current Running CPU: Core A
	 * Then, Get the Core A Current Task
	 * if the task is running on the Core A
	 * Do smp call, sync partid
	 */
	task_cpu_current = cpu_curr(task_cpu(p));

	if (task_cpu_current == p) {
		smp_call_function_single(task_cpu(p), mpam_partid_update_sync,
					(void *)p, true);
	}
}

int mpam_set_task_partid(int pid, u16 partid)
{
	struct task_struct *task = NULL;
	u16 task_last_partid = 0;

	if (!mpam_partid_check(partid)) {
		pr_err(TAG "Input partid %u Error\n", partid);
		return -EINVAL;
	}

	rcu_read_lock();
	task = find_task_by_vpid((pid_t)pid);
	if (IS_ERR_OR_NULL(task)) {
		rcu_read_unlock();
		pr_err(TAG "pid not find\n");
		return -EINVAL;
	}

	get_task_struct(task);
	task_last_partid = __get_task_partid(task);
	rcu_read_unlock();

	/* If current partid of task != new partid set, update now*/
	if (task_last_partid != partid) {
		__set_task_partid(task, partid);
		if (s_mpam_cpu_enable)
			mpam_task_fresh(task);
	}

	put_task_struct(task);
	return 0;
}

static int __mpam_get_cgrproot_children(struct cgroup_subsys_state *css_root,
					const char *str, int *cgroup_id)
{
	struct cgroup_subsys_state *child = NULL;
	char buf[MPAM_STRING_MAX] = { 0 };
	int indicator = -1;
	int path_length = 0;

	list_for_each_entry_rcu(child, &css_root->children, sibling) {
		if (!child || !child->cgroup)
			continue;

		path_length = cgroup_path(child->cgroup, buf, MPAM_STRING_MAX);
		if (path_length < 0 || path_length >= MPAM_STRING_MAX) {
			pr_err(TAG "cgroup get path error, errorcode = %d\n",
			       path_length);
			return path_length;
		}

		indicator = strncmp(buf + 1, str, path_length);
		if (indicator == 0) {
			*cgroup_id = child->id;
			return 0;
		}
	}

	pr_warn(TAG "%s not found cgroupid\n", str);
	return 0;
}

int mpam_get_cgroup_subsys_id(const char *path_str, int *cgroup_id)
{
	struct cgroup_subsys_state *css = NULL;
	struct cgroup *cgroup = NULL;
	int ret = 0;

	if (IS_ERR_OR_NULL(path_str) || IS_ERR_OR_NULL(cgroup_id)) {
		pr_err(TAG "Input ptr Error\n");
		return -EINVAL;
	}

	rcu_read_lock();
	/*
	 * cgroup_get_from_path() would be much cleaner, but that seems to be v2
	 * only. Getting current's cgroup is only a means to get a cgroup handle,
	 * use that to get to the root. Clearly doesn't work if several roots
	 * are involved.
	 */
	cgroup = task_cgroup(current, mpam_subsys_id);
	if (IS_ERR_OR_NULL(cgroup)) {
		ret = PTR_ERR(cgroup);
		goto out_unlock;
	}

	cgroup = &cgroup->root->cgrp;
	css = rcu_dereference(cgroup->subsys[mpam_subsys_id]);
	if (IS_ERR_OR_NULL(css)) {
		ret = -ENOENT;
		goto out_unlock;
	}

	ret = __mpam_get_cgrproot_children(css, path_str, cgroup_id);

out_unlock:
	if (ret < 0)
		pr_err(TAG "get cgroup id Error, errorcode = %d\n", ret);
	rcu_read_unlock();

	return ret;
}

int mpam_set_cgroup_partid(int cgroup_id, u16 partid)
{
	u16 last_partid = 0;

	if (cgroup_id <= 0 || cgroup_id >= MPAM_CGROUP_SUBSYS_MAX) {
		pr_err(TAG "input cgroupid Error, cgroup_id = %d\n", cgroup_id);
		return -EINVAL;
	}

	if (!mpam_partid_check(partid)) {
		pr_err(TAG "Input partid %u Error\n", partid);
		return -EINVAL;
	}

	mutex_lock(&s_mpam_mgr_config_lock);

	last_partid = __read_cgroup_partid(cgroup_id);

	if (last_partid != partid) {
		__set_cgroup_partid(cgroup_id, partid);
		if (s_mpam_cpu_enable)
			on_each_cpu(mpam_map_sync, NULL, true);
	}

	mutex_unlock(&s_mpam_mgr_config_lock);

	return 0;
}

int mpam_set_cpu_partid(int cpu_id, u16 partid)
{
	u32 last_partid = 0;
	int err = 0;

	if (cpu_id < 0 || !cpumask_test_cpu(cpu_id, cpu_possible_mask)) {
		pr_err(TAG "mpam cpu set out of range, input cpuid = %d\n",
		       cpu_id);
		return -EINVAL;
	}

	if (!mpam_partid_check(partid)) {
		pr_err(TAG "Input partid %u Error\n", partid);
		return -EINVAL;
	}

	mutex_lock(&s_mpam_mgr_config_lock);

	last_partid = __get_cpu_partid(cpu_id);

	if (last_partid != partid) {
		__set_cpu_partid(cpu_id, partid);

		if (s_mpam_cpu_enable) {
			err = smp_call_function_single(cpu_id, mpam_map_sync, NULL,
						true);
			if (err)
				pr_err(TAG
				"smp call function Error, CPUID=%d, errcode = %d\n",
				cpu_id, err);
		}
	}

	mutex_unlock(&s_mpam_mgr_config_lock);

	return err;
}

static inline void mpam_context_switch(void __always_unused *data,
				       struct task_struct *prev,
				       struct task_struct *next)
{
	mpam_sync_task(next, is_idle_task(prev));
}

int mpam_set_cpu_enable(int enable)
{
	int ret = 0;

	mutex_lock(&s_mpam_mgr_config_lock);

	if (enable && !s_mpam_cpu_enable) {
		on_each_cpu(mpam_map_sync, (void *)true, true);
		ret = register_trace_android_vh_mpam_set(mpam_context_switch,
							 NULL);
		if (ret) {
			pr_err(TAG "Context Switch register Error\n");
		} else {
			s_mpam_cpu_enable = true;
			pr_info(TAG "MPAM CPU Enabled\n");
		}
	} else if (!enable && s_mpam_cpu_enable) {
		unregister_trace_android_vh_mpam_set(mpam_context_switch, NULL);
		on_each_cpu(mpam_reset_partid, NULL, true);
		s_mpam_cpu_enable = false;
		pr_info(TAG "MPAM CPU Disabled\n");
	}

	mutex_unlock(&s_mpam_mgr_config_lock);

	return ret;
}

int mpam_get_cpu_enable_status(void)
{
	return s_mpam_cpu_enable;
}

static inline void mpam_partid_map_task_init(void __always_unused *data,
					     struct task_struct *p)
{
	__set_task_partid(p, MPAM_PARTID_RESET);
}

static inline void __mpam_map_reset(void)
{
	int cpu_id = 0;
	int i = 0;
	struct task_struct *p = NULL;
	struct task_struct *t = NULL;

	for_each_possible_cpu(cpu_id)
		__set_cpu_partid(cpu_id, MPAM_PARTID_RESET);

	for (i = 0; i < ARRAY_SIZE(s_mpam_css_map); ++i)
		__set_cgroup_partid(i, MPAM_PARTID_RESET);

	read_lock(&tasklist_lock);
	for_each_process_thread(p, t)
		__set_task_partid(t, MPAM_PARTID_RESET);

	read_unlock(&tasklist_lock);
}

static inline int mpam_android_vendor_hooks_init(void)
{
	int ret = 0;

	ret = register_trace_android_rvh_sched_fork_init(mpam_partid_map_task_init, NULL);
	if (ret)
		pr_err(TAG
		       "register vendor hook failed, errorcode = %d\n",
		       ret);

	return ret;
}

int mpam_map_mgr_init(void)
{
	int ret = 0;

	s_mpam_cpu_enable = false;

	mutex_init(&s_mpam_mgr_config_lock);

	ret = mpam_android_vendor_hooks_init();
	if (ret)
		return ret;

	__mpam_map_reset();

	return 0;
}
