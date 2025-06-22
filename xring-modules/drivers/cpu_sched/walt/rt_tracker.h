/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#ifndef __RT_TRACKER_H__
#define __RT_TRACKER_H__

#include <linux/types.h>
#include <linux/sched.h>

#if IS_ENABLED(CONFIG_XRING_RT_TRACKER)
void rt_tracker_add_thread(struct task_struct *task);
void rt_tracker_remove_thread(struct task_struct *task);
void rt_tracker_update_wake_count(struct task_struct *task);
void rt_tracker_init(void);
#else
static inline void rt_tracker_add_thread(struct task_struct *task)
{
}
static inline void rt_tracker_remove_thread(struct task_struct *task)
{
}
static inline void rt_tracker_update_wake_count(struct task_struct *task)
{
}
static inline void rt_tracker_init(void)
{
}
#endif

#endif /* __RT_TRACKER_H__ */
