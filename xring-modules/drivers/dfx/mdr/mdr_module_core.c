// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include "mdr_print.h"
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/export.h>

#include "mdr_module_core.h"
#include "mdr_syserr_core.h"
#include "mdr_field_core.h"
#include "mdr.h"

static LIST_HEAD(__mdr_module_ops_list);
static DEFINE_SPINLOCK(__mdr_module_ops_list_lock);
static u64 g_s_cur_regcore;
static u64 g_current_coremk;
static u32 g_current_modid;
static u64 g_current_mask;

u64 mdr_get_nve(void)
{
	return 0;
}

char *mdr_get_exception_core(u64 coreid)
{
	char *core = NULL;
	return core;
}

u64 mdr_get_cur_regcore(void)
{
	return g_s_cur_regcore;
}

void mdr_add_cur_regcore(u64 corid)
{
	g_s_cur_regcore |= corid;
}

static void __mdr_register_module_ops(struct mdr_module_ops_s *ops)
{
	struct mdr_module_ops_s *p_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	unsigned long lock_flag;

	MDR_PRINT_START();

	if (ops == NULL) {
		MDR_PRINT_ERR("invalid  parameter. ops:%pK\n", ops);
		MDR_PRINT_END();
		return;
	}
	spin_lock_irqsave(&__mdr_module_ops_list_lock, lock_flag);
	if (list_empty(&__mdr_module_ops_list)) {
		pr_err("list_add_tail coreid is [0x%llx]\n", ops->s_core_id);
		list_add_tail(&(ops->s_list), &__mdr_module_ops_list);
		goto out;
	}

	list_for_each_safe(cur, next, &__mdr_module_ops_list) {
		p_info = list_entry(cur, struct mdr_module_ops_s, s_list);
		if (ops->s_core_id > p_info->s_core_id) {
			pr_err("list_add2 coreid is [0x%llx]\n", ops->s_core_id);
			list_add(&(ops->s_list), cur);
			goto out;
		}
	}

	pr_err("list_add_tail2 coreid is [0x%llx]\n", ops->s_core_id);
	list_add_tail(&(ops->s_list), &__mdr_module_ops_list);
out:
	spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);
	MDR_PRINT_END();
}

u64 mdr_check_coreid(u64 core_id)
{
	struct mdr_module_ops_s *p_module_ops = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	unsigned long lock_flag;

	spin_lock_irqsave(&__mdr_module_ops_list_lock, lock_flag);
	list_for_each_safe(cur, next, &__mdr_module_ops_list) {
		p_module_ops = list_entry(cur, struct mdr_module_ops_s, s_list);
		if (p_module_ops == NULL) {
			MDR_PRINT_ERR("It might be better to look around here.\n");
			continue;
		}
		if (core_id == p_module_ops->s_core_id) {
			spin_unlock_irqrestore(&__mdr_module_ops_list_lock,
					       lock_flag);
			MDR_PRINT_END();
			return core_id;
		}
	}
	spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);
	return 0;
}

static inline void mdr_get_module_infonve(int ret, struct mdr_register_module_result *retinfo)
{
	if (ret >= 0)
		retinfo->nve = mdr_get_nve();
}

int mdr_get_module_info(u64 coreid, struct mdr_register_module_result *retinfo)
{
	int ret = -1;

	switch (coreid) {
	case MDR_AP:
		ret = mdr_get_areainfo(MDR_AREA_AP, retinfo);
		break;
	case MDR_TEEOS:
		ret = mdr_get_areainfo(MDR_AREA_TEEOS, retinfo);
		break;
	case MDR_AUDIO:
		ret = mdr_get_areainfo(MDR_AREA_AUDIO, retinfo);
		break;
	case MDR_LPM3:
		ret = mdr_get_areainfo(MDR_AREA_LPM3, retinfo);
		break;
	case MDR_ISP:
		ret = mdr_get_areainfo(MDR_AREA_ISP, retinfo);
		break;
	case MDR_GPU:
		ret = 0;
		break;
	case MDR_VDSP:
	case MDR_SHUB:
		ret = 0;
		break;
	case MDR_XRSE:
		ret = 0;
		break;
	case MDR_DPU:
		ret = 0;
		break;
	case MDR_NPU:
		ret = 0;
		break;
	case MDR_VENC:
	case MDR_VDEC:
		ret = 0;
		break;
	default:
		ret = -1;
	}

	mdr_get_module_infonve(ret, retinfo);

	return ret;
}

int mdr_register_module_ops(u64 coreid, struct mdr_module_ops *ops, struct mdr_register_module_result *retinfo)
{
	struct mdr_module_ops_s *p_module_ops = NULL;

	int ret = -1;

	MDR_PRINT_START();

	if (ops == NULL) {
		MDR_PRINT_ERR("invalid  parameter. ops:%pK\n", ops);
		MDR_PRINT_END();
		return ret;
	}
	if (ops->ops_dump == NULL && ops->ops_reset == NULL) {
		MDR_PRINT_ERR("invalid  parameter. ops.dump or reset:%pK\n", ops);
		MDR_PRINT_END();
		return ret;
	}

	if (mdr_check_coreid(coreid) != 0) {
		MDR_PRINT_ERR("core_id exist already\n");
		MDR_PRINT_END();
		return ret;
	}
	p_module_ops = kmalloc(sizeof(struct mdr_module_ops_s), GFP_ATOMIC);
	if (p_module_ops == NULL) {
		MDR_PRINT_ERR("kmalloc error, e_tpye_info\n");
		MDR_PRINT_END();
		return ret;
	}
	memset(p_module_ops, 0, sizeof(struct mdr_module_ops_s));

	p_module_ops->s_core_id = coreid;
	p_module_ops->s_ops.ops_dump = ops->ops_dump;
	p_module_ops->s_ops.ops_reset = ops->ops_reset;
	p_module_ops->s_ops.ops_data = ops->ops_data;

	__mdr_register_module_ops(p_module_ops);

	mdr_add_cur_regcore(coreid);

	ret = mdr_get_module_info(coreid, retinfo);
	pr_err("success\n");
	MDR_PRINT_END();
	return ret;
}
EXPORT_SYMBOL(mdr_register_module_ops);

/*
 * func name: mdr_unregister_module_ops
 * func args:
 * u64 coreid: core id
 * return value
 *	== 0 success
 *	!= 0 failed
 */
int mdr_unregister_module_ops(u64 coreid)
{
	struct mdr_module_ops_s *p_module_ops = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	unsigned long lock_flag;

	MDR_PRINT_START();
	spin_lock_irqsave(&__mdr_module_ops_list_lock, lock_flag);
	list_for_each_safe(cur, next, &__mdr_module_ops_list) {
		p_module_ops = list_entry(cur, struct mdr_module_ops_s, s_list);
		if (p_module_ops == NULL) {
			MDR_PRINT_ERR("It might be better to look around here.\n");
			continue;
		}
		if (coreid == p_module_ops->s_core_id) {
			list_del(cur);
			kfree(p_module_ops);
			break;
		}
	}
	spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);

	MDR_PRINT_END();
	return 0;
}
EXPORT_SYMBOL(mdr_unregister_module_ops);

static inline void set_curr_modid_coreid(u32 modid, u64 coremk)
{
	g_current_modid = modid;
	g_current_coremk = coremk;
	g_current_mask = 0;
}

static inline u64 check_coreid(u64 id)
{
	return g_current_coremk & id;
}

static inline u32 get_curr_modid(void)
{
	return g_current_modid;
}

void mdr_dump_done(u32 modid, u64 coreid)
{
	MDR_PRINT_START();
	pr_info("modid:[0x%x], coreid:[0x%llx]\n", modid, coreid);
	if (modid != get_curr_modid()) {
		pr_err("invaild modid!!!\n");
		MDR_PRINT_END();
		return;
	}
	if (check_coreid(coreid) == 0) {
		MDR_PRINT_END();
		return;
	}
	g_current_mask |= coreid;
	pr_info("current mask:[0x%llx]\n", g_current_mask);
	MDR_PRINT_END();
}

u64 mdr_get_dump_result(u32 modid)
{
	MDR_PRINT_START();
	if (modid != get_curr_modid()) {
		pr_err("invalid modid :[0x%x]:[0x%x]\n", modid, get_curr_modid());
		MDR_PRINT_END();
		return 0;
	}
	pr_info("current mask:[0x%llx]\n", g_current_mask);
	MDR_PRINT_END();
	return g_current_mask;
}

void mdr_notify_module_reset(u32 modid, struct mdr_exception_info_s *e_info)
{
	struct mdr_module_ops_s *p_module_ops = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	u64 mask = 0;
	static bool mdr_reboot_later_flag; /* default value is false */
	unsigned long lock_flag;

	MDR_PRINT_START();

	if (e_info == NULL) {
		MDR_PRINT_ERR("invalid  parameter. e:%pK\n", e_info);
		MDR_PRINT_END();
		return;
	}

	mask = e_info->e_reset_core_mask;

	spin_lock_irqsave(&__mdr_module_ops_list_lock, lock_flag);

	list_for_each_safe(cur, next, &__mdr_module_ops_list) {
		p_module_ops = list_entry(cur, struct mdr_module_ops_s, s_list);
		if (p_module_ops == NULL) {
			MDR_PRINT_ERR("It might be better to look around here.\n");
			continue;
		}

		pr_info("p_module_ops->s_core_id is [0x%llx]\n", p_module_ops->s_core_id);

		if (p_module_ops->s_core_id == MDR_AP) {
			pr_info("reboot priority[%s], reboot later flag[%s]"
				     "syserr list[%s], coreid mask[0x%llx]\n",
				     e_info->e_reboot_priority == MDR_REBOOT_NOW ? "Now" : "Later",
				     mdr_reboot_later_flag == true ? "true" : "false",
				     mdr_syserr_list_empty() ? "empty" : "Non empty",
				     mask & p_module_ops->s_core_id);
			/* at ap, maybe many fault, but we want the last exception to reboot ,so before exception need reboot later */
			if (e_info->e_reboot_priority == MDR_REBOOT_NOW || ((mdr_reboot_later_flag || (mask & p_module_ops->s_core_id)) && mdr_syserr_list_empty())) {
				pr_info("reboot now!\n");
				spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);
				(*(p_module_ops->s_ops.ops_reset)) (modid, e_info->e_exce_type,
						e_info->e_from_core, p_module_ops->s_ops.ops_data);
				spin_lock_irqsave(&__mdr_module_ops_list_lock, lock_flag);
			} else if ((mask & p_module_ops->s_core_id) && !mdr_syserr_list_empty()) {
				pr_info("reboot later!\n");
				mdr_reboot_later_flag = true;
			}
		} else if ((mask & p_module_ops->s_core_id) && (p_module_ops->s_ops.ops_reset != NULL)) {
			pr_info("reset module [%s] start!\n", mdr_get_exception_core(p_module_ops->s_core_id));
			spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);
			(*(p_module_ops->s_ops.ops_reset)) (modid, e_info->e_exce_type,
					e_info->e_from_core, p_module_ops->s_ops.ops_data);
			spin_lock_irqsave(&__mdr_module_ops_list_lock, lock_flag);
			pr_info("reset module [%s] end!\n", mdr_get_exception_core(p_module_ops->s_core_id));
		}
	}
	spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);

	MDR_PRINT_END();
}

u64 mdr_notify_onemodule_dump(u32 modid, u64 core, u32 type, u64 fromcore, char *path)
{
	struct mdr_module_ops_s *p_module_ops = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	u64 coreid = 0;
	unsigned long lock_flag;

	MDR_PRINT_START();

	if (path == NULL) {
		MDR_PRINT_ERR("invalid  parameter. path:%pK\n", path);
		MDR_PRINT_END();
		return 0;
	}

	spin_lock_irqsave(&__mdr_module_ops_list_lock, lock_flag);

	list_for_each_safe(cur, next, &__mdr_module_ops_list) {
		p_module_ops = list_entry(cur, struct mdr_module_ops_s, s_list);
		if (p_module_ops == NULL) {
			MDR_PRINT_ERR("It might be better to look around here.\n");
			continue;
		}

		pr_info("core is [%llx],p_module_ops->s_core_id is [%llx]\n", core, p_module_ops->s_core_id);
		if ((core & p_module_ops->s_core_id) && (p_module_ops->s_ops.ops_dump != NULL)) {
			pr_info("dump module data [%s] start!\n", mdr_get_exception_core(p_module_ops->s_core_id));
			spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);
			(*(p_module_ops->s_ops.ops_dump)) (modid, type, fromcore, path,
				mdr_dump_done, p_module_ops->s_ops.ops_data);
			spin_lock_irqsave(&__mdr_module_ops_list_lock, lock_flag);
			pr_info("dump module data [%s] end!\n", mdr_get_exception_core(p_module_ops->s_core_id));
			//spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);
			coreid = p_module_ops->s_core_id;
			break;
		}
	}

	spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);

	MDR_PRINT_END();
	return coreid;
}

static void mdr_entry_save(struct mdr_exception_info_s *e_info, char *path)
{
	char entry[MDR_LOG_SIZE];
	u32 entry_size = 0;

	if (e_info->e_reset_core_mask & MDR_AP)
		return;

	if (get_mdr_entry(e_info, entry, &entry_size, path) < 0) {
		MDR_PRINT_ERR("get_mdr_entry error\n");
	} else {
		if (mdr_send_nl(entry, entry_size) < 0)
			MDR_PRINT_ERR("mdr_send_nl error\n");
	}
}

u64 mdr_notify_module_dump(u32 modid, struct mdr_exception_info_s *e_info, char *path)
{
	u64 ret = 0;
	struct mdr_module_ops_s *p_module_ops = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	u64 mask = 0;
	unsigned long lock_flag;

	MDR_PRINT_START();

	if (e_info == NULL) {
		MDR_PRINT_ERR("invalid  parameter. e:%pK, p:%pK\n", e_info, path);
		MDR_PRINT_END();
		return ret;
	}
	mask = e_info->e_notify_core_mask;
	set_curr_modid_coreid(modid, mask);

	mdr_entry_save(e_info, path);
	pr_info("mdr entry save done\n");

	spin_lock_irqsave(&__mdr_module_ops_list_lock, lock_flag);

	list_for_each_safe(cur, next, &__mdr_module_ops_list) {
		p_module_ops = list_entry(cur, struct mdr_module_ops_s, s_list);
		if (p_module_ops == NULL) {
			MDR_PRINT_ERR("It might be better to look around here.\n");
			continue;
		}

		/*
		 *First condition: Skip modules which hadn't triggered the exception, to ensure the trigger called firstly .
		 *after loop ,Second condition: Skip the trigger, to avoid calling the dump-callback repeatedly.
		 */
		if ((!ret && e_info->e_from_core != p_module_ops->s_core_id) || (ret && (e_info->e_from_core == p_module_ops->s_core_id))) {
			pr_info("Skip module core [0x%llx].\n", p_module_ops->s_core_id);
			continue;
		}

		if ((mask & p_module_ops->s_core_id) && (p_module_ops->s_ops.ops_dump != NULL)) {
			pr_info("dump module data [%s] start!\n", mdr_get_exception_core(p_module_ops->s_core_id));

			spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);
			(*(p_module_ops->s_ops.ops_dump)) (modid, e_info->e_exce_type,
				p_module_ops->s_core_id, path, mdr_dump_done,
				p_module_ops->s_ops.ops_data);
			spin_lock_irqsave(&__mdr_module_ops_list_lock, lock_flag);

			pr_info("dump module data [%s] end!\n", mdr_get_exception_core(p_module_ops->s_core_id));
			ret |= p_module_ops->s_core_id;
		}

		/*
		 *First, to call dump-callback of module which triggered the exception.
		 *After that, reset list pointer to the HEAD, and call other modules' callback.
		 */
		if (ret == p_module_ops->s_core_id) {
			cur = &__mdr_module_ops_list;
			next = cur->next;
		}
	}

	spin_unlock_irqrestore(&__mdr_module_ops_list_lock, lock_flag);

	MDR_PRINT_END();

	return ret;
}

void mdr_print_all_ops(void)
{
	int index = 1;
	struct mdr_module_ops_s *p_module_ops = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;

	MDR_PRINT_START();

	spin_lock(&__mdr_module_ops_list_lock);
	list_for_each_safe(cur, next, &__mdr_module_ops_list) {
		p_module_ops = list_entry(cur, struct mdr_module_ops_s, s_list);
		if (p_module_ops == NULL) {
			MDR_PRINT_ERR("@@ abnormal here.\n");
			continue;
		}

		pr_info("==========[%d]-start==========\n", index);
		pr_info(" core-id:        [0x%llx]\n", p_module_ops->s_core_id);
		pr_info(" dump-fn:        [0x%pK]\n", p_module_ops->s_ops.ops_dump);
		pr_info(" reset-fn:       [0x%pK]\n", p_module_ops->s_ops.ops_reset);
		pr_info(" data:           [0x%pK]\n", p_module_ops->s_ops.ops_data);
		pr_info("==========[%d]-end==========\n", index);
		index++;
	}
	spin_unlock(&__mdr_module_ops_list_lock);

	MDR_PRINT_END();
}

MODULE_LICENSE("GPL");
