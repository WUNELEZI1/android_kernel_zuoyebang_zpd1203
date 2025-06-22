// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */
#include "mdr_print.h"
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/export.h>
#include "mdr_exception_core.h"

static LIST_HEAD(__mdr_exception_list);
static DEFINE_SPINLOCK(__mdr_exception_list_lock);
int g_mdr_debug_level = LOGLEVEL_ERR;

void __mdr_register_exception(struct mdr_exception_info_s *e)
{
	if (e == NULL) {
		MDR_PRINT_ERR("invalid  parameter. e:%pK\n", e);
		return;
	}

	spin_lock(&__mdr_exception_list_lock);
	list_add_tail(&(e->e_list), &__mdr_exception_list);
	spin_unlock(&__mdr_exception_list_lock);
}

void mdr_callback(struct mdr_exception_info_s *p_exce_info, u32 mod_id,
		  char *logpath)
{
	struct mdr_exception_info_s *e_type_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	u64 e_from_core;
	mdr_e_callback e_callback;

	spin_lock(&__mdr_exception_list_lock);
	list_for_each_safe(cur, next, &__mdr_exception_list) {
		e_type_info = list_entry(cur, struct mdr_exception_info_s, e_list);
		if (e_type_info == NULL) {
			MDR_PRINT_ERR("It is abnormal\n");
			continue;
		}

		e_from_core = e_type_info->e_from_core;
		e_callback = e_type_info->e_callback;

		spin_unlock(&__mdr_exception_list_lock);
		if ((e_from_core != p_exce_info->e_from_core) &&
		    (e_from_core & p_exce_info->e_notify_core_mask)) {
			if ((uintptr_t)(e_callback) & MDR_COMMON_CALLBACK) {
				pr_info("call core common callback function.\n");
				((mdr_e_callback)(uintptr_t)((uintptr_t)(e_callback) & ~MDR_CALLBACK_MASK)) (mod_id, logpath);
			}
		}
		spin_lock(&__mdr_exception_list_lock);
	}
	spin_unlock(&__mdr_exception_list_lock);

	if ((uintptr_t)(p_exce_info->e_callback) & ~MDR_CALLBACK_MASK) {
		pr_info("call exception function.\n");
		((mdr_e_callback)(uintptr_t)((uintptr_t) (p_exce_info->e_callback) & ~MDR_CALLBACK_MASK)) (mod_id, logpath);
	}
}

u32 mdr_check_modid(u32 modid, u32 modid_end)
{
	struct mdr_exception_info_s *e_type_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;

	spin_lock(&__mdr_exception_list_lock);
	list_for_each_safe(cur, next, &__mdr_exception_list) {
		e_type_info = list_entry(cur, struct mdr_exception_info_s, e_list);
		if (e_type_info == NULL) {
			MDR_PRINT_ERR("It might be better to look around here.\n");
			continue;
		}
		if ((modid >= e_type_info->e_modid && modid <= e_type_info->e_modid_end) ||
			(modid_end >= e_type_info->e_modid && modid_end <= e_type_info->e_modid_end)) {
			spin_unlock(&__mdr_exception_list_lock);
			return modid;
		}
	}
	spin_unlock(&__mdr_exception_list_lock);

	return 0;
}

/*
 *   u32 modid, exception id;
 *   if modid equal 0, will auto generation modid, and return it.
 *   u32 modid_end, can register modid region. [modid~modid_end];
 *      need modid_end >= modid,
 *      if modid_end equal 0, will be register modid only,
 *      but modid & modid_end cant equal 0 at the same time.
 *   u32 process_priority,  exception process priority
 *   u32 reboot_priority,   exception reboot priority
 *   u32 save_log_mask,     need save log mask
 *   u32 notify_core_mask,  need notify other core mask
 *   u32 reset_core_mask,   need reset other core mask
 *   u32 reentrant,     whether to allow excption reentrant
 *   u32 from_core,     the core of happen excption
 *   char* from_module,     the module of happen excption
 *   char* desc,        the desc of happen excption
 *   bb_e_callback callback,    will be called when excption has processed.
 *   u32 reserve_u32;       reserve u32
 *   void* reserve_p        reserve void *
 */

/*
 * func name: mdr_register_exception_type
 * func args:
 * struct mdr_exception_info_s* s_e_type
 * return value		e_modid
 *	== 0 error
 *	>0 success
 */
u32 mdr_register_exception(struct mdr_exception_info_s *e)
{
	struct mdr_exception_info_s *e_type_info = NULL;
	u32 modid_end;

	if (e == NULL) {
		MDR_PRINT_ERR("mdr_register_exception_type parameter is NULL!\n");
		return 0;
	}

	modid_end = e->e_modid_end;
	if (e->e_modid_end == 0 || e->e_modid_end < e->e_modid) {
		MDR_PRINT_ERR("modid error[0x%x ~ 0x%x],modify modid_end = [0x%x]\n", e->e_modid, e->e_modid_end, e->e_modid);
		modid_end = e->e_modid;
	}

	if (mdr_check_modid(e->e_modid, modid_end) != 0) {
		pr_err("mod_id exist already\n");
		return 0;
	}

	e_type_info = kmalloc(sizeof(struct mdr_exception_info_s), GFP_ATOMIC);
	if (e_type_info == NULL) {
		pr_err("kmalloc failed for e_tpye_info\n");
		return 0;
	}

	memset(e_type_info, 0, sizeof(struct mdr_exception_info_s));
	memcpy(e_type_info, e, sizeof(struct mdr_exception_info_s));
	e_type_info->e_modid_end = modid_end;

	__mdr_register_exception(e_type_info);

	mdr_debug(LOGLEVEL_INFO, "register modid [0x%x ~ 0x%x], register exception succeed.\n", e->e_modid, modid_end);

	return e_type_info->e_modid_end;
}
EXPORT_SYMBOL(mdr_register_exception);

/*
 * func name: mdr_unregister_exception
 * func args:
 * u32 modid: exception id
 * return value
 *	== 0 success
 *	!= 0 failed
 */
int mdr_unregister_exception(u32 modid)
{
	struct mdr_exception_info_s *e_type_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;

	MDR_PRINT_START();

	spin_lock(&__mdr_exception_list_lock);
	list_for_each_safe(cur, next, &__mdr_exception_list) {
		e_type_info = list_entry(cur, struct mdr_exception_info_s, e_list);
		if (e_type_info == NULL) {
			pr_err("It might be better to look around here.\n");
			continue;
		}
		if (modid >= e_type_info->e_modid &&
		    e_type_info->e_modid_end >= modid) {
			pr_err("free exception [0x%x].\n", e_type_info->e_modid);
			list_del(cur);
			kfree(e_type_info);
		}
	}
	spin_unlock(&__mdr_exception_list_lock);

	MDR_PRINT_END();

	return 0;
}
EXPORT_SYMBOL(mdr_unregister_exception);

struct mdr_exception_info_s *mdr_get_exception_info(u32 modid)
{
	struct mdr_exception_info_s *e_type_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;

	spin_lock(&__mdr_exception_list_lock);
	list_for_each_safe(cur, next, &__mdr_exception_list) {
		e_type_info = list_entry(cur, struct mdr_exception_info_s, e_list);
		if (e_type_info == NULL) {
			MDR_PRINT_ERR("It might be better to look around here.\n");
			continue;
		}
		if (e_type_info->e_modid <= modid && e_type_info->e_modid_end >= modid) {
			spin_unlock(&__mdr_exception_list_lock);
			return e_type_info;
		}
	}
	spin_unlock(&__mdr_exception_list_lock);

	return NULL;
}

void mdr_print_one_exc(struct mdr_exception_info_s *e)
{
	e->e_desc[STR_EXCEPTIONDESC_MAXLEN - 1] = '\0';
	e->e_from_module[MODULE_NAME_LEN - 1] = '\0';

	pr_info(" modid:          [0x%x]\n", e->e_modid);
	pr_info(" modid_end:      [0x%x]\n", e->e_modid_end);
	pr_info(" process_pri:    [0x%x]\n", e->e_process_priority);
	pr_info(" reboot_pri:     [0x%x]\n", e->e_reboot_priority);
	pr_info(" notify_core_mk: [0x%llx]\n", e->e_notify_core_mask);
	pr_info(" reset_core_mk:  [0x%llx]\n", e->e_reset_core_mask);
	pr_info(" reentrant:      [0x%x]\n", e->e_reentrant);
	pr_info(" exce_type:      [0x%x]\n", e->e_exce_type);
	pr_info(" exce_subtype:   [0x%x]\n", e->e_exce_subtype);
	pr_info(" from_core:      [0x%llx]\n", e->e_from_core);
	pr_info(" from_module:    [%s]\n", e->e_from_module);
	pr_info(" desc:           [%s]\n", e->e_desc);
	pr_info(" callback:       [0x%pK]\n", e->e_callback);
	pr_info(" reserve_u32:    [0x%x]\n", e->e_reserve_u32);
	pr_info(" reserve_p:      [0x%pK]\n", e->e_reserve_p);
}

void mdr_print_all_exc(void)
{
	int index = 1;
	struct mdr_exception_info_s *e_type_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;

	MDR_PRINT_START();

	spin_lock(&__mdr_exception_list_lock);
	list_for_each_safe(cur, next, &__mdr_exception_list) {
		e_type_info = list_entry(cur, struct mdr_exception_info_s, e_list);
		if (e_type_info == NULL) {
			MDR_PRINT_ERR("It might be better to look around here.\n");
			continue;
		}

		pr_info("==========[%.2d]-start==========\n", index);
		mdr_print_one_exc(e_type_info);
		pr_info("==========[%.2d]-e n d==========\n", index);
		index++;
	}
	spin_unlock(&__mdr_exception_list_lock);

	MDR_PRINT_END();
}

MODULE_LICENSE("GPL");
