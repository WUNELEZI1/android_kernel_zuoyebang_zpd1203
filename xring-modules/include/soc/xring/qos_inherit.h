/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */
#ifndef __QIS_QOS_H__
#define __QIS_QOS_H__

#include <linux/notifier.h>
#include <linux/sched.h>
#include <linux/types.h>

enum {
	QOS_BINDER,
	QOS_MUTEX,
	QOS_RWSEM,
	QOS_PERCPU_RWSEM,
	QOS_FUTEX,
	QOS_TYPE_NUM,
	QOS_TYPE_FIRST = QOS_BINDER,
};

enum {
	QOS_LOW,
	QOS_NORMAL,
	QOS_HIGH,
	QOS_CRITICAL,
	QOS_NUM,
	QOS_FIRST = QOS_LOW,
};

enum qievent_notif_type {
	QIEVENT_QOS_SET = 1,
	QIEVENT_QOS_UNSET,
	QIEVENT_QOS_INHERIT,
	QIEVENT_QOS_UNINHERIT,
	QIEVENT_QOS_CORRECT,
};

union qos_union {
	u32 qos;
	struct {
		u32 qos_lvl : 4;
		u32 sched_opt : 1;
		u32 vip_sched_qos : 3;
		u32 reserved : 24;
	} bits;
};

struct qievent_struct {
	struct task_struct *from;
	struct task_struct *to;
	union qos_union from_qos;
	union qos_union to_qos;
};

struct inherit_list {
	struct list_head list;
	spinlock_t lock;
	void *lock_or_binder;
};

struct qi_struct {
	spinlock_t lock;
	atomic_t flag;
	atomic_t static_qos;
	atomic_t dyn_qos;
	atomic_t proc_static_qos;
	pid_t futex_owner;
	unsigned long timeout;
	struct inherit_list inherit[QOS_TYPE_NUM];
	u64 opt_spin_start_time;
};

struct xr_binder_transaction {
	pid_t pid;
	u32 reserved;
};

struct xr_rwsem_struct {
	u64 writing_finish_flag : 1;
	u64 reserve : 63;
};

int qi_intf_set_qos(void __user *uarg);
int qi_intf_get_qos_by_tid(void __user *uarg);
int qi_intf_get_qos_by_pid(void __user *uarg);
void qi_intf_schedinfo_update(struct qievent_struct *data);
void qi_intf_schedinfo_reset(struct qievent_struct *data);
int register_qievent_notifier(struct notifier_block *nb);
int unregister_qievent_notifier(struct notifier_block *nb);
bool is_qos_task(struct task_struct *task);
int get_proc_static_qos_lvl(struct task_struct *task);
u32 get_qos_lvl(struct task_struct *task);

#endif
