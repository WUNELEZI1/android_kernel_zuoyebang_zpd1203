/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#ifndef __XR_WALT_H__
#define __XR_WALT_H__

#include <linux/cpumask.h>
#include <soc/xring/walt.h>

extern void xr_sched_init_debug(void);
extern unsigned long xr_get_group_util(struct walt_related_thread_group *grp,
				       const struct cpumask *query_cpus);
extern bool xr_is_group_freq_locked(struct walt_related_thread_group *grp,
				    const struct cpumask *query_cpus);
extern bool xr_is_group_prefer_cpus(const struct cpumask *query_cpus);
extern void xr_update_task_running_sum(struct task_struct *p, struct rq *rq,
				       int event, u64 delta_exec, u64 delta_load);
extern void xr_reset_task_sum(struct walt_related_thread_group *grp,
			      struct walt_task_struct *wts);
extern void xr_init_account_sum(struct walt_task_struct *wts);
extern void xr_account_yield_time(struct walt_task_struct *wts, u64 now);
extern void xr_perf_actuator_init(void);
extern unsigned long xr_get_max_group_util(const struct cpumask *query_cpus);
extern unsigned long xr_get_group_prefer_util(struct task_struct *p);
extern void xr_get_group_prefer_cpus(struct task_struct *p, struct cpumask *perfer_cpus);
extern bool xr_is_task_prefer_cpu(struct task_struct *p, unsigned int cpu);
extern bool xr_can_migrate_task(struct task_struct *p, unsigned int src_cpu, unsigned int dst_cpu);
#define xr_has_group_prefer_cpus(grp)	(!cpumask_empty(&grp->xr_prefer_cpus))

#endif
