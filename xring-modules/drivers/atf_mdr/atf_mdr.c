// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 * Description: atf-mdr driver
 * Modify time: 2024-04-15
 */
#include <linux/io.h>
#include <asm/barrier.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/arm-smccc.h>
#include <trace/hooks/debug.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>

static void *mh_addr;
static struct dentry *g_debugfs_dir;

static u32 valid_modid[] = {
	MODID_ATF_BL31_RUNTIME_PANIC,
	MODID_XR_AP_XHEE_PANIC,
	MODID_ATF_XSP_RUNTIME_PANIC,
	MODID_ATF_XSPM_RUNTIME_PANIC
};

static struct mdr_exception_info_s g_mdr_excep[] = {
	{
		.e_modid = (u32)MODID_ATF_BL31_RUNTIME_PANIC,
		.e_modid_end = (u32)MODID_ATF_BL31_RUNTIME_PANIC,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_AP,
		.e_reset_core_mask = MDR_AP,
		.e_from_core = MDR_AP,
		.e_reentrant = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type = XR_AP_BL31_PANIC,
		.e_exce_subtype = ATF_BL31_RUNTIME_PANIC,
		.e_upload_flag = (u32)MDR_UPLOAD_YES,
		.e_from_module = "atf",
		.e_desc = "atf bl31 runtime panic",
	},
	{
		.e_modid = (u32)MODID_XR_AP_XHEE_PANIC,
		.e_modid_end = (u32)MODID_XR_AP_XHEE_PANIC,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_AP,
		.e_reset_core_mask = MDR_AP,
		.e_from_core = MDR_AP,
		.e_reentrant = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type = XR_AP_BL31_PANIC,
		.e_exce_subtype = ATF_XHEE_RUNTIME_PANIC,
		.e_upload_flag = (u32)MDR_UPLOAD_YES,
		.e_from_module = "xhee",
		.e_desc = "atf xhee runtime panic",
	},
	{
		.e_modid = (u32)MODID_ATF_XSPM_RUNTIME_PANIC,
		.e_modid_end = (u32)MODID_ATF_XSPM_RUNTIME_PANIC,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_AP,
		.e_reset_core_mask = MDR_AP,
		.e_from_core = MDR_AP,
		.e_reentrant = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type = XR_AP_BL31_PANIC,
		.e_exce_subtype = ATF_XSPM_RUNTIME_PANIC,
		.e_upload_flag = (u32)MDR_UPLOAD_YES,
		.e_from_module = "xspm",
		.e_desc = "atf xspm runtime panic",
	},
	{
		.e_modid = (u32)MODID_ATF_XSP_RUNTIME_PANIC,
		.e_modid_end = (u32)MODID_ATF_XSP_RUNTIME_PANIC,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_AP,
		.e_reset_core_mask = MDR_AP,
		.e_from_core = MDR_AP,
		.e_reentrant = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type = XR_AP_BL31_PANIC,
		.e_exce_subtype = ATF_XSP_RUNTIME_PANIC,
		.e_upload_flag = (u32)MDR_UPLOAD_YES,
		.e_from_module = "xsp",
		.e_desc = "atf xsp runtime panic",
	},
};

static bool is_valid_mdr(u32 modid)
{
	int i;
	int array_len = ARRAY_SIZE(valid_modid);


	if (modid > MODID_TEE_START && modid < MODID_TEE_END)
		return true;
	for (i = 0; i < array_len; i++) {
		if (valid_modid[i] == modid)
			return true;
	}

	return false;
}

static void mdr_ipi_handler(void *unused, struct pt_regs *regs)
{
	struct atf_mdr_info *mi = (struct atf_mdr_info *)mh_addr;
	u32 modid;

	pr_info("atf_mdr ipi hook handler is invoked\n");

	if (mi->e_modid == MODID_XR_AP_RAS_UNCORRECTED) {
		ras_mdr_notify(mi);
	} else if (is_valid_mdr(mi->e_modid)) {
		pr_crit("received MDR IPI modid: 0x%x\n", mi->e_modid);
		modid = mi->e_modid;
		mi->e_modid = 0;//ensure e_modid set to 0 before mdr_syserr_process_for_ap
		dsb(sy);
		mdr_syserr_process_for_ap(modid, mi->args[0], mi->args[1]);
	}
}

static ssize_t atf_mdr_debug_test(struct file *file, char __user *buf,
			    size_t count, loff_t *offp)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_ATF_MDR_TEST, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		pr_err("atf_mdr debug test fail\n");
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations atf_mdr_debug_fops = {
	.owner   = THIS_MODULE,
	.read    = atf_mdr_debug_test,
	.open    = simple_open,
};

static int atf_mdr_init_debugfs(void)
{
	struct dentry *node = NULL;

	g_debugfs_dir = debugfs_create_dir("atf_mdr", NULL);
	if (!g_debugfs_dir) {
		pr_err("Failed to create /sys/kernel/debug/atf_mdr\n");
		return -EFAULT;
	}

	node = debugfs_create_file("test", 0644, g_debugfs_dir, NULL, &atf_mdr_debug_fops);
	if (!node) {
		pr_err("Failed to create /sys/kernel/debug/atf_mdr/test\n");
		debugfs_remove_recursive(g_debugfs_dir);
		return -EFAULT;
	}
	pr_notice("atf_mdr debugfs init success\n");

	return 0;
}

static int __init atf_mdr_init(void)
{
	int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(g_mdr_excep); i++) {
		ret = mdr_register_exception(&g_mdr_excep[i]);
		if (ret == 0) {
			pr_err("atf_mdr exception %d register fail, ret %d.\n", i, ret);
			return -EFAULT;
		}
	}

	mh_addr = ioremap_cache(DFX_MEM_BL31_MDR_ADDR, DFX_MEM_BL31_MDR_SIZE);
	if (mh_addr == NULL) {
		pr_err("atf_mdr log header remap error\n");
		return -EFAULT;
	}

	ret = register_trace_android_vh_ipi_stop(mdr_ipi_handler, NULL);
	if (ret < 0) {
		pr_err("atf_mdr ipi handler register fail, ret %d.\n", ret);
		return ret;
	}

	ret = atf_mdr_init_debugfs();
	if (ret < 0) {
		pr_err("atf_mdr debugfs init fail, ret %d.\n", ret);
		return ret;
	}
	pr_info("atf_mdr register success\n");

	return 0;
}

/* This is called when the module is removed. */
static void __exit atf_mdr_exit(void)
{
	debugfs_remove_recursive(g_debugfs_dir);
}

MODULE_DESCRIPTION("XRing atf-mdr driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("post: xring-mdr");
module_init(atf_mdr_init);
module_exit(atf_mdr_exit);
