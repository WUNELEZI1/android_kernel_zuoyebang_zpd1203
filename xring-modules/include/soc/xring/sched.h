/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _LINUX_XR_SCHED_H
#define _LINUX_XR_SCHED_H

#include <linux/version.h>
#include <linux/sched/cputime.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include "../../../kernel/sched/sched.h"
#include <soc/xring/qos_inherit.h>
#include <soc/xring/qos_sched.h>
#include <soc/xring/walt.h>

enum {
	PINGPONG_THERM = 0,
};

#if IS_ENABLED(CONFIG_XRING_SCHED_VIP)
struct vip_task {
	int prio;
	int orig_prio;
	struct list_head entry;
#if IS_ENABLED(CONFIG_XRING_VIP_BANDWIDTH)
	u64 exec_start;
#endif
};
#endif

struct xr_mutex {
	/*
	 * Number of vip thread in this mutex's
	 * waiter_list
	 */
	atomic_long_t cnt;
};

struct xr_task_struct {
#if IS_ENABLED(CONFIG_XRING_SCHED_WALT)
	/*
	 * wts must be the first member
	 * of xr_task_struct
	 */
	struct walt_task_struct wts;
#endif
#if IS_ENABLED(CONFIG_XRING_MPAM)
	u16 mpam_partid;
	u8 mpam_qi_enabled;
#endif
#if IS_ENABLED(CONFIG_XRING_QOS_INHERIT)
	/*
	 * This flag indicate this vip task is
	 * waiting a mutex or a rwsem
	 */
	u8 blocked;
	struct qi_struct *qi;
#endif
#if IS_ENABLED(CONFIG_XRING_QOS_SCHED)
	struct qs_struct qs;
#endif

#if IS_ENABLED(CONFIG_XRING_SCHED_VIP)
	struct vip_task			vip;
#endif

#if IS_ENABLED(CONFIG_PMT)
	struct pmt_info		*pmt_info;
#endif

	/* Reserved for other modules */
};

#define xrts_to_ts(xrts) ({ \
		void *__mptr = (void *)(xrts); \
		((struct task_struct *)(__mptr - \
			offsetof(struct task_struct, android_vendor_data1))); })

#ifdef CONFIG_UCLAMP_TASK
static inline bool uclamp_boosted(struct task_struct *p)
{
	return uclamp_eff_value(p, UCLAMP_MIN) > 0;
}
#else
static inline bool uclamp_boosted(struct task_struct *p)
{
	return false;
}
#endif

#ifdef CONFIG_UCLAMP_TASK_GROUP
static inline bool uclamp_latency_sensitive(struct task_struct *p)
{
	struct cgroup_subsys_state *css = task_css(p, cpu_cgrp_id);
	struct task_group *tg;

	if (!css)
		return false;

	tg = container_of(css, struct task_group, css);
	return tg->latency_sensitive;
}
#else
static inline bool uclamp_latency_sensitive(struct task_struct *p)
{
	return false;
}
#endif

static inline struct xr_mutex *mutex_to_xr(struct mutex *lock)
{
	return (struct xr_mutex *)lock->android_oem_data1;
}

#if IS_ENABLED(CONFIG_XRING_TASK_PP)
extern void set_pingpong_flag(int type);
extern void clear_pingpong_flag(int type);
extern int get_pingpong_flag(void);
#else /* CONFIG_XRING_TASK_PP */
static inline void set_pingpong_flag(int type) { }
static inline void clear_pingpong_flag(int type) { }
static inline int get_pingpong_flag(void) { return 0; }
#endif /* CONFIG_XRING_TASK_PP */

#endif /* _LINUX_XR_SCHED_H */
