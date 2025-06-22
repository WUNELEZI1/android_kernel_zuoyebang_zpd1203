// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2015 Google, Inc.
 * Copyright (C) 2021 XiaoMi, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"

static struct mdr_exception_info_s g_tee_exception[] = {
	[0] = {
		.e_modid            = (u32)MODID_TEE_EXCEPTION,
		.e_modid_end        = (u32)MODID_TEE_EXCEPTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_TEEOS | MDR_AP,
		.e_reset_core_mask  = MDR_TEEOS | MDR_AP,
		.e_from_core        = MDR_TEEOS,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = TEEOS_EXCEPTION,
		.e_exce_subtype     = TEE_S_EXCEPTION,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "TEEOS",
		.e_desc             = "tee exception",
	},
	[1] = {
		.e_modid            = (u32)MODID_TEE_KERNEL_EXCEPTION,
		.e_modid_end        = (u32)MODID_TEE_KERNEL_EXCEPTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_TEEOS | MDR_AP,
		.e_reset_core_mask  = MDR_TEEOS | MDR_AP,
		.e_from_core        = MDR_TEEOS,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = TEEOS_EXCEPTION,
		.e_exce_subtype     = TEE_S_KERNEL_EXCEPTION,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "TEEOS",
		.e_desc             = "tee kernel exception",
	},
	[2] = {
		.e_modid            = (u32)MODID_TEE_MR_EXCEPTION,
		.e_modid_end        = (u32)MODID_TEE_MR_EXCEPTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_TEEOS | MDR_AP,
		.e_reset_core_mask  = MDR_TEEOS | MDR_AP,
		.e_from_core        = MDR_TEEOS,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = TEEOS_EXCEPTION,
		.e_exce_subtype     = TEE_S_MR_EXCEPTION,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "TEEOS",
		.e_desc             = "tee mr exception",
	},
	[3] = {
		.e_modid            = (u32)MODID_TEE_SERROR_EXCEPTION,
		.e_modid_end        = (u32)MODID_TEE_SERROR_EXCEPTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_TEEOS | MDR_AP,
		.e_reset_core_mask  = MDR_TEEOS | MDR_AP,
		.e_from_core        = MDR_TEEOS,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = TEEOS_EXCEPTION,
		.e_exce_subtype     = TEE_S_SERROR_EXCEPTION,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "TEEOS",
		.e_desc             = "tee serror exception",
	},
	[4] = {
		.e_modid            = (u32)MODID_TEE_CORE_PROCESS_EXCEPTION,
		.e_modid_end        = (u32)MODID_TEE_CORE_PROCESS_EXCEPTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_TEEOS | MDR_AP,
		.e_reset_core_mask  = MDR_TEEOS | MDR_AP,
		.e_from_core        = MDR_TEEOS,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = TEEOS_EXCEPTION,
		.e_exce_subtype     = TEE_S_CORE_PROCESS_EXCEPTION,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "TEEOS",
		.e_desc             = "tee core process exception",
	},
};

struct tee_mdr_device {
	struct mdr_module_ops mdr_ops;
	struct mdr_register_module_result mdr_retinfo;
};

static struct tee_mdr_device *g_tmdev;

static void mdr_tee_dump(u32 modid, u32 etype, u64 coreid, char *log_path,
				pfn_cb_dump_done pfn_cb, void *data)
{
	pr_info("%s\n", __func__);
}

static void mdr_tee_reset(u32 modid, u32 etype, u64 coreid, void *data)
{
	struct tee_mdr_device *tmdev = NULL;

	pr_info("%s\n", __func__);
	pr_info("modid = 0x%x  etype = 0x%x coreid = 0x%llx\n", modid, etype, coreid);
	tmdev = (struct tee_mdr_device *)data;
	if (!tmdev) {
		pr_err("get tmdr dev failed, modid 0x%x\n", modid);
		return;
	}
	pr_info("%s ok\n", __func__);
}

int mitee_register_mdr(struct tee_mdr_device *tmdev)
{
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(g_tee_exception); i++) {
		ret = mdr_register_exception(&g_tee_exception[i]);
		if (ret == 0) {
			pr_err("exception register fail, ret %d i %d.\n", ret, i);
			return ret;
		}
	}

	tmdev->mdr_ops.ops_dump = mdr_tee_dump;
	tmdev->mdr_ops.ops_reset = mdr_tee_reset;
	tmdev->mdr_ops.ops_data = tmdev;

	ret = mdr_register_module_ops(MDR_TEEOS, &tmdev->mdr_ops, &tmdev->mdr_retinfo);
	if (ret < 0) {
		pr_err("module register fail, ret %d.\n", ret);
		return ret;
	}

	return 0;
}

int tee_mdr_init(void)
{
	int ret_code = 0;

	pr_info("tee_mdr_device_probe enter\n");
	g_tmdev = kzalloc(sizeof(*g_tmdev), GFP_KERNEL);
	if (!g_tmdev) {
		ret_code = -ENOMEM;
		pr_err("failed to kzalloc tmdev mem (%d)\n", ret_code);
		return ret_code;
	}

	ret_code = mitee_register_mdr(g_tmdev);
	if (ret_code) {
		kfree(g_tmdev);
		pr_err("failed to register mitee mdr (%d)\n", ret_code);
		return ret_code;
	}
	pr_info("tee_mdr_device_probe succ\n");

	return ret_code;
}

void tee_mdr_exit(void)
{

	kfree(g_tmdev);
}
