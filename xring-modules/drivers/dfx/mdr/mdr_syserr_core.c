// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include "mdr_print.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <uapi/linux/sched/types.h>
#include <linux/utsname.h>

#include "mdr_field_core.h"
#include "mdr_module_core.h"
#include "mdr_exception_core.h"
#include "mdr_syserr_core.h"
#include "mdr_subsys_rst.h"
#include "mdr.h"

static struct semaphore mdr_sem;
static LIST_HEAD(g_mdr_sys_err_list);
static DEFINE_SPINLOCK(g_mdr_sys_err_list_lock);

int mdr_wait_partition(char *path, int timeouts)
{
	return 0;
}

bool mdr_syserr_list_empty(void)
{
	return list_empty(&g_mdr_sys_err_list);
}

void mdr_register_system_error(u32 modid, u32 arg1, u32 arg2)
{
	struct mdr_syserr_param_s *p = NULL;
	struct mdr_exception_info_s *p_exce_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	struct mdr_syserr_param_s *e_cur = NULL;
	int exist = 0;

	MDR_PRINT_START();

	p = kzalloc(sizeof(struct mdr_syserr_param_s), GFP_ATOMIC);
	if (p == NULL) {
		MDR_PRINT_ERR("kzalloc mdr_syserr_param_s faild.\n");
		return;
	}

	p->modid = modid;
	p->arg1 = arg1;
	p->arg2 = arg2;

	p_exce_info = mdr_get_exception_info(modid);

	spin_lock(&g_mdr_sys_err_list_lock);

	if (p_exce_info) {
		if (p_exce_info->e_reentrant == (u32)MDR_REENTRANT_DISALLOW) {
			list_for_each_safe(cur, next, &g_mdr_sys_err_list) {
				e_cur = list_entry(cur, struct mdr_syserr_param_s, syserr_list);
				if (e_cur->modid == p->modid) {
					exist = 1;
					MDR_PRINT_ERR("exception:[0x%x] disallow reentrant.  return.\n", modid);
					break;
				}
			}
		}
	}

	if (exist == 0)
		list_add_tail(&p->syserr_list, &g_mdr_sys_err_list);
	else if (exist == 1)
		kfree(p);

	spin_unlock(&g_mdr_sys_err_list_lock);

	MDR_PRINT_END();
}

void mdr_syserr_process_for_ap(u32 modid, u64 arg1, u64 arg2)
{
	const struct modid_word *modid_map = NULL;
	u32 i;

	modid_map = get_modid_map();
	if (modid_map == NULL) {
		pr_err("get modid map failed\n");
		return;
	}

	for (i = 0; i < get_modid_map_size(); i++) {
		if (modid_map[i].modid_num == modid) {
			pr_info("start process %s", modid_map[i].modid_name);
			panic(modid_map[i].modid_name);
			return;
		}
	}

	pr_err("exception[%x] not in modid map\n", modid);
}
EXPORT_SYMBOL(mdr_syserr_process_for_ap);

void mdr_syserr_process_for_ap_callback(u32 modid, u64 arg1, u64 arg2)
{
	struct mdr_exception_info_s *p_exce_info = NULL;
	struct mdr_syserr_param_s p;

	MDR_PRINT_START();

	p.modid = modid, p.arg1 = arg1, p.arg2 = arg2;
	preempt_disable();

	p_exce_info = mdr_get_exception_info(modid);
	if (p_exce_info == NULL) {
		preempt_enable();
		pr_err("get exception info failed.  return\n");
		return;
	}

	record_exce_type(p_exce_info);

	(void)mdr_notify_module_dump(modid, p_exce_info, NULL);

	mdr_notify_module_reset(modid, p_exce_info);

	preempt_enable();

	MDR_PRINT_END();
}

void mdr_system_error(u32 modid, u32 arg1, u32 arg2)
{
	MDR_PRINT_START();

	/* stub  in_atomic() || delete*/
	if (irqs_disabled() || in_irq())
		MDR_PRINT_ERR("in atomic or irqs disabled or in irq\n");

	dump_stack();

	mdr_register_system_error(modid, arg1, arg2);

	up(&mdr_sem);

	MDR_PRINT_END();
}
EXPORT_SYMBOL(mdr_system_error);

void mdr_module_dump(struct mdr_exception_info_s *p_exce_info, char *path, u32 mod_id)
{
	u32 mask = 0;

	mask = mdr_notify_module_dump(mod_id, p_exce_info, path);

	pr_info("mdr_notify_module_dump done. return mask=[0x%x]\n", mask);
}

static void mdr_save_log(struct mdr_exception_info_s *p_exce_info, char *path, u32 mod_id)
{
	bool need_save_log = true;

	if (need_save_log) {
		mdr_module_dump(p_exce_info, path, mod_id);
		mdr_save_subsys_log(p_exce_info);
	}
}

extern void record_exce_type(struct mdr_exception_info_s *e_info);

void mdr_syserr_process(struct mdr_syserr_param_s *p)
{
	u32 mod_id = p->modid;

	struct mdr_exception_info_s *p_exce_info = NULL;
	char path[PATH_MAXLEN];

	MDR_PRINT_START();

	mdr_field_baseinfo_reinit();
	mdr_save_args(p->modid, p->arg1, p->arg2);
	p_exce_info = mdr_get_exception_info(mod_id);

	if (p_exce_info == NULL) {
		MDR_PRINT_ERR("get exception info faild.  return.\n");
		return;
	}

	if (p_exce_info->e_reset_core_mask & MDR_AP)
		record_exce_type(p_exce_info);

	pr_info("start saving data.\n");

	mdr_print_one_exc(p_exce_info);

	mdr_save_log(p_exce_info, path, mod_id);

	mdr_callback(p_exce_info, mod_id, path);

	mdr_wait_logsave_before_reset(p_exce_info);

	pr_info("saving data done.\n");

	mdr_subsys_reset(p_exce_info);

	/*process to here, upload has completely finished */
	if (p_exce_info->e_upload_flag == (u32)MDR_UPLOAD_YES)
		pr_info("mdr_upload log: done.\n");

	pr_info("mdr_notify_module_reset: start.\n");

	mdr_notify_module_reset(mod_id, p_exce_info);
	pr_info("mdr_notify_module_reset: done.\n");

	MDR_PRINT_END();
}

static int mdr_main_thread_body(void *arg)
{
	struct list_head *cur = NULL;
	struct list_head *process = NULL;
	struct mdr_syserr_param_s *e_cur = NULL;
	struct mdr_syserr_param_s *e_process = NULL;
	struct list_head *next = NULL;
	u32 e_priority = MDR_PPRI_MAX;
	struct mdr_exception_info_s *p_exce_info = NULL;
	long jiffies = 0;
	unsigned long lock_flag;

	MDR_PRINT_START();

	while (!kthread_should_stop()) {
		jiffies = msecs_to_jiffies(1000);
		if (down_timeout(&mdr_sem, jiffies)) {
			if (mdr_syserr_list_empty())
				continue;
		}

		pr_err("enter\n");
		pr_err("wait for fs ready start\n");
		while (mdr_wait_partition("/data/lost+found", 1000) != 0)
			;
		pr_err("wait for fs ready end\n");

		while (!mdr_syserr_list_empty()) {
			spin_lock_irqsave(&g_mdr_sys_err_list_lock, lock_flag);
			list_for_each_safe(cur, next, &g_mdr_sys_err_list) {
				e_cur = list_entry(cur, struct mdr_syserr_param_s, syserr_list);
				pr_err("syserr modid is %x\n", e_cur->modid);
				p_exce_info = mdr_get_exception_info(e_cur->modid);
				if (p_exce_info == NULL) {
					pr_err("mdr_get_exception_info fail\n");
					if (process == NULL) {
						process = cur;
						e_process = e_cur;
					}
					continue;
				}
				if (p_exce_info->e_process_priority >= MDR_PPRI_MAX)
					pr_err("invalid prio[%d], current modid [0x%x]\n", p_exce_info->e_process_priority, e_cur->modid);

				if (p_exce_info->e_process_priority < e_priority) {
					pr_err("current prio[%d], current modid [0x%x]\n", p_exce_info->e_process_priority, e_cur->modid);
					process = cur;
					e_process = e_cur;
					e_priority = p_exce_info->e_process_priority;
				}
			}

			if (process == NULL || e_process == NULL) {
				pr_err("exception: NULL\n");
				spin_unlock_irqrestore(&g_mdr_sys_err_list_lock, lock_flag);
				continue;
			}

			list_del(process);
			spin_unlock_irqrestore(&g_mdr_sys_err_list_lock, lock_flag);

			mdr_syserr_process(e_process);

			kfree(e_process);

			e_priority = MDR_PPRI_MAX;
			process = NULL;
			e_process = NULL;
		}
	}

	MDR_PRINT_END();

	return 0;
}

int mdr_syserr_init(void)
{
	struct task_struct *mdr_main = NULL;

	sema_init(&mdr_sem, 0);

	mdr_main = kthread_run(mdr_main_thread_body, NULL, "mdr_main");
	if (!mdr_main) {
		MDR_PRINT_ERR("create thread mdr_main_thread_body faild.\n");
		return -1;
	}

	return 0;
}

MODULE_LICENSE("GPL");
