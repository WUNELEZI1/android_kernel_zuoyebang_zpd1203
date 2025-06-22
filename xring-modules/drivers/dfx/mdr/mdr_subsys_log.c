// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include "mdr_print.h"
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/export.h>

#include "mdr.h"
#include "mdr_subsys_log.h"

#define LOG_NAME_MAXLEN         20
#define NODE_NAME_MAXLEN        30
#define BUF_MAXSIZE	        128
#define LOG_MAXLEN	        0xA00000

static LIST_HEAD(__mdr_subsys_info_list);
static DEFINE_SPINLOCK(__mdr_subsys_info_list_lock);

static void mdr_save_subsys_loginfo(char *node_name, char *log_name, u32 log_size)
{
	int ret = 0;
	char buf[BUF_MAXSIZE] = {'\0'};
	u32 log_name_size = strlen(log_name);
	u32 node_name_size = strlen(node_name);

	pr_info("start\n");
	if (log_name_size > LOG_NAME_MAXLEN) {
		pr_err("log name size over max log name len");
		return;
	}

	if (node_name_size > NODE_NAME_MAXLEN) {
		pr_err("node name size over max node name len");
		return;
	}

	if (log_size > LOG_MAXLEN) {
		pr_err("log len over max log len");
		return;
	}

	ret = snprintf(buf, BUF_MAXSIZE, "subl node[%s], log_name[%s], size[%d]\n",
			node_name, log_name, log_size);
	if (unlikely(ret > BUF_MAXSIZE)) {
		pr_err("snprintf ret %d!\n", ret);
		return;
	}

	u32 size = strlen(buf);

	if (mdr_send_nl(buf, size) < 0)
		pr_err("subsys log send error\n");

	mdr_wait_subsys_logsave();

	pr_info("exit\n");
}

static u64 mdr_check_subsys_coreid(u64 core_id)
{
	struct mdr_subsys_info_s *sub_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	unsigned long lock_flag;

	MDR_PRINT_START();
	spin_lock_irqsave(&__mdr_subsys_info_list_lock, lock_flag);
	list_for_each_safe(cur, next, &__mdr_subsys_info_list) {
		sub_info = list_entry(cur, struct mdr_subsys_info_s, s_list);
		if (sub_info == NULL) {
			MDR_PRINT_ERR("It might be better to look around here.\n");
			continue;
		}

		if (core_id == sub_info->core_id) {
			spin_unlock_irqrestore(&__mdr_subsys_info_list_lock,
					lock_flag);
			MDR_PRINT_END();
			return core_id;
		}
	}
	spin_unlock_irqrestore(&__mdr_subsys_info_list_lock, lock_flag);
	MDR_PRINT_END();
	return 0;
}

/*
 * core_id: equivalent to the parameter coreid of mdr_register_module_ops
 * node_name: the node that needs to store memory, such as /proc/logbuf_node
 * log_name: log name, such as logbuf.log
 * log_size: log size < 10M
 *
 */
int mdr_filesys_write_log(u64 core_id, char *node_name, char *log_name, u32 log_size)
{
	pr_info("start\n");

	u32 log_name_size = strlen(log_name);
	u32 node_name_size = strlen(node_name);

	if (log_name_size > LOG_NAME_MAXLEN) {
		pr_err("log name size over max log name len");
		return -EINVAL;
	}

	if (node_name_size > NODE_NAME_MAXLEN) {
		pr_err("node name size over max node name len");
		return -EINVAL;
	}

	if (log_size > LOG_MAXLEN) {
		pr_err("log len over max log len");
		return -EINVAL;
	}

	if (mdr_check_subsys_coreid(core_id) != 0) {
		MDR_PRINT_ERR("core_id exist already\n");
		MDR_PRINT_END();
		return -EINVAL;
	}

	struct mdr_subsys_info_s *sub_info = kmalloc(sizeof(struct mdr_subsys_info_s), GFP_ATOMIC);

	if (sub_info == NULL) {
		MDR_PRINT_ERR("kmalloc failed for sub_info\n");
		return -ENOMEM;
	}

	sub_info->core_id = core_id;
	sub_info->node_name = node_name;
	sub_info->log_name = log_name;
	sub_info->log_size = log_size;

	spin_lock(&__mdr_subsys_info_list_lock);
	list_add_tail(&(sub_info->s_list), &__mdr_subsys_info_list);
	spin_unlock(&__mdr_subsys_info_list_lock);
	pr_info("exit\n");

	return 0;
}
EXPORT_SYMBOL(mdr_filesys_write_log);

void mdr_save_subsys_log(struct mdr_exception_info_s *p_exce_info)
{
	u64 ret = 0;
	struct mdr_subsys_info_s *sub_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	u64 mask = 0;
	unsigned long lock_flag;

	if (p_exce_info == NULL) {
		MDR_PRINT_ERR("invalid  parameter. e:%pK\n", p_exce_info);
		MDR_PRINT_END();
		return;
	}
	pr_info("enter\n");

	mask = p_exce_info->e_notify_core_mask;
	pr_info("p_exce_info->e_notify_core_mask = 0x%llx\n", p_exce_info->e_notify_core_mask);

	spin_lock_irqsave(&__mdr_subsys_info_list_lock, lock_flag);

	list_for_each_safe(cur, next, &__mdr_subsys_info_list) {
		sub_info = list_entry(cur, struct mdr_subsys_info_s, s_list);
		if (sub_info == NULL) {
			MDR_PRINT_ERR("It might be better to look around here.\n");
			continue;
		}
		pr_info("sub_info->coreid = 0x%llx\n", sub_info->core_id);

		/*
		 *First condition: Skip modules which hadn't triggered the exception,
		 *to ensure the trigger called firstly .
		 *after loop ,Second condition: Skip the trigger, to avoid calling the dump-callback repeatedly.
		 */
		if ((!ret && p_exce_info->e_from_core != sub_info->core_id)
				|| (ret && (p_exce_info->e_from_core == sub_info->core_id))) {
			pr_err("Skip module core [0x%llx].", sub_info->core_id);
			continue;
		}

		if (mask & sub_info->core_id) {
			spin_unlock_irqrestore(&__mdr_subsys_info_list_lock, lock_flag);
			mdr_save_subsys_loginfo(sub_info->node_name, sub_info->log_name,
					sub_info->log_size);
			spin_lock_irqsave(&__mdr_subsys_info_list_lock, lock_flag);
			ret |= sub_info->core_id;
		}

		/*
		 *First, to call dump-callback of module which triggered the exception.
		 *After that, reset list pointer to the HEAD, and call other modules' callback.
		 */
		if (ret == sub_info->core_id) {
			cur = &__mdr_subsys_info_list;
			next = cur->next;
		}
	}

	spin_unlock_irqrestore(&__mdr_subsys_info_list_lock, lock_flag);
	pr_info("exit\n");
}

