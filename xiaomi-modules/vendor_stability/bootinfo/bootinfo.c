/*
 * bootinfo.c
 *
 * Copyright (C) 2011 Xiaomi Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <asm/setup.h>
#include "bootinfo.h"
#include <linux/bitops.h>
#include <linux/kobject.h>
#include <linux/slab.h>

static const char * const powerup_reasons[PU_REASON_MAX] = {
	[PU_REASON_EVENT_NONE]		         = "none",
	[PU_REASON_EVENT_RTC]		         = "rtc",
	[PU_REASON_EVENT_CHGDET]             = "usb_chg",
	[PU_REASON_EVENT_PWR_KEY]	         = "long_power_key",
	[PU_REASON_EVENT_PWR_KEY_FAST]	     = "fast_power_key",
	[PU_REASON_EVENT_SMPL]	             = "lower_power",
};

static const char * const reboot_reasons[PD_REASON_MAX] = {
	[PD_REASON_EVENT_COLDBOOT]		     = "coldboot",            //normal
	[PD_REASON_EVENT_BOOTLOADER]         = "bootloader",
	[PD_REASON_EVENT_RECOVRTY]	         = "recovery",
	[PD_REASON_EVENT_FASTBOOT]		     = "fastboot",
	[PD_REASON_EVENT_ADB]		         = "adb",
	[PD_REASON_EVENT_TSENSOR]	         = "tsensor",             //hardware exception
	[PD_REASON_EVENT_AWDT]	             = "awdt",
	[PD_REASON_EVENT_LPIS_DIS]	         = "lpis_dis",
	[PD_REASON_EVENT_LPPLL]	             = "lppll",
	[PD_REASON_EVENT_PWR_SQC]	         = "pwr_soc",
	[PD_REASON_EVENT_NPU_DSP_WD]	     = "npu_dsp_wd",
	[PD_REASON_EVENT_NPU_M85_WD]	     = "npu_m85_wd",
	[PD_REASON_EVENT_ISP_WD]	         = "lppll_wd",
	[PD_REASON_EVENT_AUDIO_WD]	         = "audio_wd",
	[PD_REASON_EVENT_PERI_WD3]	         = "peri_wd3",
	[PD_REASON_EVENT_PERI_WD2]	         = "peri_wd2",
	[PD_REASON_EVENT_PERI_WD1]	         = "peri_wd1",
	[PD_REASON_EVENT_LMS_WD]	         = "LMS_WD",
	[PD_REASON_EVENT_XRSE_WD]	         = "xrse_wd",
	[PD_REASON_EVENT_XCTRL_DDR_WD]	     = "xctrl_ddr_wd",
	[PD_REASON_EVENT_XCTRL_CPU_WD]	     = "xctrl_cpu_wd",
	[PD_REASON_EVENT_SENSORHUB_WD]	     = "sensorhub_wd",
	[PD_REASON_EVENT_LPCORE_WDT]	     = "lpcore_wd",
	[PD_REASON_EVENT_XR_AP_PMU]	         = "xr_ap_pmu",
	[PD_REASON_EVENT_AP_HW_WDT]	         = "ap_hw_wd",
	[PD_REASON_EVEN_XR_MULTI_HARDFAULT]	 = "xr_multi_hardfault",
	[PD_REASON_EVENT_XR_AP_PANIC]	     = "xr_ap_panic",          //ap exception
	[PD_REASON_EVENT_XR_AP_NOC]	         = "xr_ap_noc",  
	[PD_REASON_EVENT_XR_AP_BL31_PANIC]	 = "xr_ap_bl31",
	[PD_REASON_EVENT_XR_AP_RAS]	         = "xr_ap_ras",
	[PD_REASON_EVENT_XR_AP_COMB]	     = "xr_ap_comb",
	[PD_REASON_EVENT_BORINGSSL]	         = "xr_ap_boringssl",
	[PD_REASON_EVENT_APEXD]	             = "xr_ap_apexd",
	[PD_REASON_EVENT_BOOTSTRAP_APEXD]	 = "xr_ap_bootstrap",
	[PD_REASON_EVENT_VOLD]	             = "xr_ap_vold",
	[PD_REASON_EVENT_LPCTRL]	         = "xr_ap_lpctrl",
	[PD_REASON_EVENT_XCTRL_DDR]	         = "xr_ap_ddr",
	[PD_REASON_EVENT_XCTRL_CPU]	         = "xr_ap_cpu",
	[PD_REASON_EVENT_TEEOS]	             = "xr_ap_teeos",
	[PD_REASON_EVENT_DM_VERITY_FAILED]	 = "dm_verity_failed",
	[PD_REASON_EVENT_CP_S_MODEM]	     = "cp_s_modem",           //subsys exception
	[PD_REASON_EVENT_AUDIO_EXCEPT]	     = "audio",
	[PD_REASON_EVENT_ISP_EXCEPT]	     = "isp",
	[PD_REASON_EVENT_NPU_EXCEPT]	     = "npu",
	[PD_REASON_EVENT_GPU_EXCEPT]	     = "gpu",
	[PD_REASON_EVENT_DPU_EXCEPT]	     = "dpu",
	[PD_REASON_EVENT_VPU_EXCEPT]	     = "vpu",
	[PD_REASON_EVENT_VDSP_EXCEPT]	     = "vdsp",
	[PD_REASON_EVENT_SHUB_EXCEPT]	     = "shub",
	[PD_REASON_EVENT_XRSE_EXCEPT]	     = "xrse",
	[PD_REASON_EVENT_WCN_S_EXCEPT]	     = "wcn_s",
	[PD_REASON_EVENT_XLOADER_WDT]	     = "xloader_wdt",               //bootup failed exception
	[PD_REASON_EVENT_XLOADER_PANIC]	     = "xloader_panic",
	[PD_REASON_EVENT_UEFI_WDT]	         = "uefi_wdt",
	[PD_REASON_EVENT_UEFI_VERIFY_FAIL]	 = "uefi_verify_fail",
	[PD_REASON_EVENT_UEFI_FLASHIMG_FAIL] = "uefi_flashimg_fail",
	[PD_REASON_EVENT_UEFI_PANIC]	     = "uefi_panic",
	[PD_REASON_EVENT_UEFI_AVS_PANIC]	 = "uefi_avs_panic",
	[PD_REASON_EVENT_XRSE_BOOT_EXCEPT]	 = "xrse_boot",
	[PD_REASON_EVENT_XCTRL_CPU_BOOT]	 = "xctrl_cpu_boot",
	[PD_REASON_EVENT_XCTRL_DDR_BOOT]	 = "xctrl_ddr_boot",
	[PD_REASON_EVENT_ATF_BL2_EXCEPT]	 = "atf_bl2",
	[PD_REASON_EVENT_ATF_BL2_RUNTIME]	 = "atf_bl2_runtime",
	[PD_REASON_EVENT_ATF_BL31_EXCEPT]	 = "atf_bl31",
	[PD_REASON_EVENT_ATF_BL31_RUNTIME]	 = "atf_bl31_runtime",
	[PD_REASON_EVENT_MITEE_EXCEPT]	     = "mitee",
};

#define MAX_CMDLINE_PARAM_LEN 64
static struct kobject *bootinfo_kobj;
static char powerup_reason[MAX_CMDLINE_PARAM_LEN];
static char reboot_reason[MAX_CMDLINE_PARAM_LEN];
u32 pu_reason = 0;
u32 rb_reason = 0;

#define bootinfo_attr(_name) \
static struct kobj_attribute _name##_attr = {	\
	.attr	= {				\
		.name = __stringify(_name),	\
		.mode = 0644,			\
	},					\
	.show	= _name##_show,			\
	.store	= NULL,				\
}

static int get_powerup_reason(char *buf)
{
	int ret;
	if (!buf || !buf[0]) {
		pr_err("get_powerup_reason: pu_reason is null, powerup_reason:%s\n",powerup_reason);
		return -EINVAL;
	}
	ret = kstrtou32(buf, 16, &pu_reason);
	return 0;
}

static int get_reboot_reason(char *buf)
{
	int ret;
	if (!buf || !buf[0]) {
		pr_err("get_reboot_reason: rb_reason is null, reboot_reason:%s\n", reboot_reason);
		return -EINVAL;
	}
	ret = kstrtou32(buf, 16, &rb_reason);
	return 0;
}

static ssize_t powerup_reason_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	char *s = buf;
	int pu_reason_index = PU_REASON_MAX;

	get_powerup_reason(powerup_reason);

	if (pu_reason == MPMIC_POWERON_EVENT_NONE)
		pu_reason_index = 0;
	else if (pu_reason == MPMIC_POWERON_EVENT_RTC)
		pu_reason_index = 1;
	else if (pu_reason == MPMIC_POWERON_EVENT_CHGDET)
		pu_reason_index = 2;
	else if (pu_reason == MPMIC_POWERON_EVENT_PWR_KEY)
		pu_reason_index = 3;
	else if (pu_reason == MPMIC_POWERON_EVENT_PWR_KEY_FAST)
		pu_reason_index = 4;
	else if (pu_reason == MPMIC_POWERON_EVENT_SMPL)
		pu_reason_index = 5;

	if (pu_reason_index < PU_REASON_MAX && pu_reason_index >= 0) {
		s += snprintf(s, strlen(powerup_reasons[pu_reason_index]) + 2,
				"%s\n", powerup_reasons[pu_reason_index]);
		pr_err("%s: pu_reason [0x%x] index %d\n",
			__func__, pu_reason, pu_reason_index);
		goto out;
	}
	s += snprintf(s, 17, "unknown powerup\n");
out:
	return (s - buf);
}

static ssize_t reboot_reason_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	char *s = buf;
	int rb_reason_index = PD_REASON_MAX;

	get_reboot_reason(reboot_reason);

	if (0 <= rb_reason && rb_reason <= NORMAL_REASON )
		rb_reason_index = rb_reason;
	else if (REBOOT_REASON_LABEL1 <= rb_reason && rb_reason <= (REBOOT_REASON_LABEL1 + HRADWARE_REASON))
		rb_reason_index = rb_reason - REBOOT_REASON_LABEL1 + NORMAL_REASON;
	else if (REBOOT_REASON_LABEL2 <= rb_reason && rb_reason <= (REBOOT_REASON_LABEL2 + AP_REASON))
		rb_reason_index = rb_reason - REBOOT_REASON_LABEL2 + NORMAL_REASON + HRADWARE_REASON;
	else if (REBOOT_REASON_LABEL3 <= rb_reason && rb_reason <= (REBOOT_REASON_LABEL3 + SUBSYS_REASON))
		rb_reason_index = rb_reason - REBOOT_REASON_LABEL3 + NORMAL_REASON + HRADWARE_REASON + AP_REASON;
	else if (REBOOT_REASON_LABEL4 <= rb_reason && rb_reason <= (REBOOT_REASON_LABEL4 + BOOTFAIL_REASON))
		rb_reason_index = rb_reason - REBOOT_REASON_LABEL4 + NORMAL_REASON + HRADWARE_REASON + AP_REASON + SUBSYS_REASON;

	if (rb_reason_index < PD_REASON_MAX && rb_reason_index >= 0) {
		s += snprintf(s, strlen(reboot_reasons[rb_reason_index]) + 2,
				"%s\n", reboot_reasons[rb_reason_index]);
		pr_err("%s: rb_reason [0x%x] index %d\n",
			__func__, rb_reason, rb_reason_index);
		goto out;
	}
	s += snprintf(s, 16, "unknown reboot\n");
out:
	return (s - buf);
}

static ssize_t powerup_reason_details_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	get_powerup_reason(powerup_reason);
	return snprintf(buf, 11, "0x%x\n", pu_reason);
}

static ssize_t reboot_reason_details_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	get_reboot_reason(reboot_reason);
	return snprintf(buf, 11, "0x%x\n", rb_reason);
}

bootinfo_attr(powerup_reason);
bootinfo_attr(powerup_reason_details);
bootinfo_attr(reboot_reason);
bootinfo_attr(reboot_reason_details);

static struct attribute *g[] = {
	&powerup_reason_attr.attr,
	&powerup_reason_details_attr.attr,
	&reboot_reason_attr.attr,
	&reboot_reason_details_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = g,
};

static int __init bootinfo_init(void)
{
	int ret = -ENOMEM;

	bootinfo_kobj = kobject_create_and_add("bootinfo", NULL);
	if (bootinfo_kobj == NULL) {
		pr_err("bootinfo_init: subsystem_register failed\n");
		goto fail;
	}

	ret = sysfs_create_group(bootinfo_kobj, &attr_group);
	if (ret) {
		pr_err("bootinfo_init: subsystem_register failed\n");
		goto sys_fail;
	}

	return ret;

sys_fail:
	kobject_del(bootinfo_kobj);
fail:
	return ret;

}

static void __exit bootinfo_exit(void)
{
	if (bootinfo_kobj) {
		sysfs_remove_group(bootinfo_kobj, &attr_group);
		kobject_del(bootinfo_kobj);
	}
}

MODULE_LICENSE("GPL");
core_initcall(bootinfo_init);
module_exit(bootinfo_exit);

module_param_string(pureason, powerup_reason, MAX_CMDLINE_PARAM_LEN,0644);
module_param_string(pdreason, reboot_reason, MAX_CMDLINE_PARAM_LEN,0644);
