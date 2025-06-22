// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include "mdr_print.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <uapi/linux/sched/types.h>
#include <linux/buildid.h>
#include <linux/smp.h>
#include <linux/kmsg_dump.h>
#include <asm/cacheflush.h>
#include <linux/panic_notifier.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/delay.h>
#include <linux/arm-smccc.h>
#include <linux/console.h>
#include <linux/err.h>
#include <linux/reboot.h>
#include <linux/notifier.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <trace/hooks/debug.h>
#include "mdr_field_core.h"
#include "mdr_module_core.h"
#include "mdr_exception_core.h"
#include "mdr_syserr_core.h"
#include "mdr_adapter_ap.h"
#include "mdr_subsys_rst.h"
#include "mdr.h"
#include <soc/xring/flush_cache.h>

static int g_mdr_ap_init;
static void __iomem *g_mi_notify_lpm3_addr;

static struct notifier_block mdr_ap_panic_block = {
	.notifier_call = mdr_ap_panic_notify,
	.priority = INT_MIN,
};

struct mdr_exception_info_s g_einfo[] = {
	{ { 0, 0 }, MODID_XR_AP_PANIC, MODID_XR_AP_PANIC, MDR_ERR, MDR_REBOOT_NOW, MDR_AP, MDR_AP,
		MDR_AP, (u32)MDR_REENTRANT_DISALLOW, (u32)XR_AP_PANIC, XR_APPANIC_RESERVED,
		(u32)MDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_XR_AP_COMBINATIONKEY, MODID_XR_AP_COMBINATIONKEY, MDR_ERR, MDR_REBOOT_NOW,
		MDR_AP, MDR_AP, MDR_AP, (u32)MDR_REENTRANT_DISALLOW, (u32)XR_AP_COMBINATIONKEY, 0,
		(u32)MDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
};

static int lpcore_nmi_notify_init(void)
{
	struct mdr_data *data = get_mdrdata();

	if (g_mi_notify_lpm3_addr == NULL) {
		g_mi_notify_lpm3_addr = ioremap(data->lpcore_nmi_addr, LPM3_NMI_SIZE);
		if (g_mi_notify_lpm3_addr == NULL) {
			pr_err("nmi_notify_init failed\n");
			return -1;
		}
	}

	return 0;
}

static void ap_nmi_notify_lpm3(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_MNTN_M3_NMI_REG, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("smc return fail.\n");
}

static void ap_regs_dump(void)
{
	pr_err("begin\n");
	pr_err("exit!\n");
}

char *mdr_get_timestamp(void)
{
	struct rtc_device *rtc;
	ssize_t retval;
	struct rtc_time tm;
	static char databuf[DATA_MAXLEN];
	int ret;

	rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);
	if (rtc == NULL) {
		pr_info("failed to open rtc device %s\n",
				CONFIG_RTC_HCTOSYS_DEVICE);
		return 0;
	}

	retval = rtc_read_time(rtc, &tm);
	if (retval)
		pr_info("read rtc time failed");

	ret = snprintf(databuf, DATA_MAXLEN, "%04d%02d%02d%02d%02d%02d",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
	if (unlikely(ret > DATA_MAXLEN)) {
		pr_err("snprintf ret %d!\n", ret);
		return 0;
	}

	pr_info("timestamp[%s] !\n", databuf);

	rtc_class_close(rtc);

	return databuf;
}
EXPORT_SYMBOL(mdr_get_timestamp);

u32 mdr_get_tick(void)
{
	struct timespec64 uptime;

	ktime_get_boottime_ts64(&uptime);

	return (u32)(uptime.tv_nsec / NSEC_PER_MSEC);
}
EXPORT_SYMBOL(mdr_get_tick);

u32 get_boot_keypoint(void)
{
	return 0;
}

/*
 * set_log_path
 * @brief: set dump log path
 * @date: date string
 * @path: path to save log
 */
static void set_log_path(char *date, char *path)
{
	int ret;

	if (date == NULL || path == NULL) {
		pr_err("null date or path\n");
		return;
	}

	memset(path, 0, PATH_MAXLEN);
	ret = snprintf(path, PATH_MAXLEN, "%s%s/", XRLOG_PATH_ROOT, date);
	if (unlikely(ret >= PATH_MAXLEN)) {
		pr_err("get path failed %d\n", ret);
		return;
	}

	pr_info("mdr logsave path is %s\n", path);
}

static bool check_reset_state(struct mdr_exception_info_s *p)
{
	if (check_subsys_sysrst_enable(p))
		return true;

	if (p->e_reboot_priority == MDR_REBOOT_NOW)
		return true;

	return false;
}

int get_mdr_entry(struct mdr_exception_info_s *p, char *buf, u32 *size, char *path)
{
	u32 ret;
	u32 core_id;
	u32 exception;
	u32 sub_exception;
	char date[DATATIME_MAXLEN];
	u32 bootup_keypoint = get_boot_keypoint();

	if ((p == NULL) || (buf == NULL) || (size == NULL)) {
		pr_err("input is null\n");
		return -EINVAL;
	}

	core_id = p->e_from_core;
	exception = p->e_exce_type;
	sub_exception = p->e_exce_subtype;

	memset(buf, 0, MDR_LOG_SIZE);
	memset(date, 0, DATATIME_MAXLEN);

	ret = snprintf(date, DATATIME_MAXLEN - 1, "%s.%03d",
			mdr_get_timestamp(), mdr_get_tick());
	if (unlikely(ret > DATATIME_MAXLEN)) {
		pr_err("snprintf ret %d!\n", ret);
		return -EINVAL;
	}

	ret = snprintf(buf, MDR_LOG_SIZE,
			"category[%s], coreid[%s], exception[%s], sub_exception[%s], time[%s], bootup[%d], SR[NO_SR], reboot[%s]\n",
			mdr_get_category_name(p->e_exce_type, p->e_exce_subtype),
			get_core_name(core_id),
			get_reboot_reason_name(exception),
			get_reboot_subtype_name(exception, sub_exception),
			date,
			bootup_keypoint,
			check_reset_state(p) ? "true" : "false");
	if (unlikely(ret > MDR_LOG_SIZE)) {
		pr_err("snprintf ret %d!\n", ret);
		return -EINVAL;
	}

	*size = strlen(buf);

	set_log_path(date, path);

	return 0;
}

static int save_mdr_log(struct mdr_exception_info_s *p)
{
	int ret = 0;
	char buf[MDR_LOG_SIZE];
	void *mem_addr = get_mdrmem_addr();
	u32 size = 0;

	ret = get_mdr_entry(p, buf, &size, NULL);
	if (ret < 0) {
		pr_err("get mdr entry failed\n");
		return -EINVAL;
	}

	memcpy(mem_addr, buf, MDR_LOG_SIZE);

	return 0;
}

void mdr_ap_dump(u32 modid, u32 etype,
		u64 coreid, char *log_path,
		pfn_cb_dump_done pfn_cb, void *data)
{
	int ret;
	struct mdr_exception_info_s *p_exce_info = NULL;

	pr_err("begin\n");
	pr_err("modid is 0x%x, etype is 0x%x\n", modid, etype);

	if (modid > MDR_MOD_AP_END)
		pr_err("modid exceeds the range\n");

	if (etype != XR_AP_PANIC)
		pr_err("etype is not panic\n");

	if (!g_mdr_ap_init) {
		pr_err("mdr_ap_adapter is not ready\n");
		if (pfn_cb)
			pfn_cb(modid, coreid);
		return;
	}

	console_loglevel = MDR_CONSOLE_LOGLEVEL_DEFAULT;

	pr_err("modid[%x],etype[%x],coreid[%llx], log_path[%s]\n", modid, etype,
			coreid, log_path);

	pr_err("regs_dump start!\n");
	ap_regs_dump();

	p_exce_info = mdr_get_exception_info(modid);
	if (p_exce_info == NULL) {
		preempt_enable();
		pr_err("get exception info failed.  return\n");
		return;
	}

	ret = save_mdr_log(p_exce_info);
	if (ret < 0) {
		pr_err("save mdr log failed\n");
		return;
	}

	pr_err("exit!\n");
	if (pfn_cb)
		pfn_cb(modid, coreid);
}

static void android_vh_ipi_flush_cache_handler(void *unused, struct pt_regs *regs)
{
	pr_err("start ==>\n");
	dump_stack();
	flush_cache_all_by_setway();
}

static void __mdr_ap_reset(void)
{
	/*
	 * if there is a panic during the interrupt handling process, in the console_lock state,
	 * and there are many printed logs, incomplete printing information may occur.
	 *
	 */
	console_unlock();
	flush_dfx_cache();
	flush_cache_all_by_setway();

	if (is_switch_on(SWITCH_EXCEPTION_LOOP)) {
		pr_err("enter exception_loop!\n");
		do {
		} while (1);
	}

	ap_nmi_notify_lpm3();

	pr_err("end ==>\n");

	while (1)
		;
}

void mdr_ap_reset(u32 modid, u32 etype, u64 coreid, void *data)
{

	pr_err("start ==>\n");

	/* Flush the storage device cache */

	if (modid > MDR_MOD_AP_END)
		pr_err("modid[0x%x] exceeds the range\n", modid);

	if (etype != XR_AP_PANIC) {
		pr_err("etype is not panic\n");
		dump_stack();
		preempt_disable();
	}

	__mdr_ap_reset();
}

static int mdr_ap_register_module(void)
{
	struct mdr_module_ops s_soc_ops;
	struct mdr_register_module_result retinfo;
	int ret;
	u64 coreid = MDR_AP;

	s_soc_ops.ops_dump = mdr_ap_dump;
	s_soc_ops.ops_reset = mdr_ap_reset;

	ret = mdr_register_module_ops(coreid, &s_soc_ops, &retinfo);
	if (ret < 0) {
		pr_err("mdr_register_module_ops fail, ret = [%d]\n", ret);
		return ret;
	}

	return ret;
}

static void mdr_panic_of_modid(void *panic_buf)
{
	u32 i;
	u32 modid;
	/* panic to this notify, for no smp stop interface, so all rst process use panic API,
	 * so we should get the string of panic para for mdr_syserr_process_for_ap.
	 */
	const struct modid_word *modid_map = get_modid_map();

	for (i = 0; i < get_modid_map_size(); i++) {
		if (!strncmp(modid_map[i].modid_name, panic_buf,
					strlen(modid_map[i].modid_name))) {
			modid = modid_map[i].modid_num;
			mdr_syserr_process_for_ap_callback(modid, 0, 0);

			do {
			} while (1);
		}
	}

	mdr_syserr_process_for_ap_callback(MODID_XR_AP_PANIC, 0, 0);

	do {
	} while (1);
}

int mdr_ap_panic_notify(struct notifier_block *nb, unsigned long event, void *buf)
{
	pr_err("===> enter panic notify!\n");

	char *panic_buf = (char *)buf;

	if (strlen(buf) > MDR_MODID_MAXLEN)
		pr_err("panic buf over max buf\n");

	pr_err("notify buf = %s", panic_buf);
	if (!strncmp(panic_buf, ACPU_WDT_FIQ, strlen(ACPU_WDT_FIQ))) {
		pr_err("acpu wdt fiq !!!\n");
		console_unlock();
		flush_dfx_cache();
		flush_cache_all_by_setway();

		do {
		} while (1);
	}

	/* sub sys rst process */
	if (!strncmp(panic_buf, SUBSYS_RESET, strlen(SUBSYS_RESET)))
		__mdr_ap_reset();

	/* all sysrest exception, call panic("modid") */
	mdr_panic_of_modid(panic_buf);

	return 0;
}

static void ap_callback(u32 argc, void *argv)
{
	pr_err("===> start\n");
}

static void mdr_ap_register_exception(void)
{
	unsigned int i;
	u32 ret;

	for (i = 0; i < sizeof(g_einfo) / sizeof(struct mdr_exception_info_s); i++) {
		mdr_debug(LOGLEVEL_INFO, "register exception:%u", g_einfo[i].e_exce_type);
		g_einfo[i].e_callback = ap_callback;
		if (i == 0)
			g_einfo[i].e_callback = (mdr_e_callback)(uintptr_t)(
					(uintptr_t)(g_einfo[i].e_callback) | MDR_COMMON_CALLBACK);
		ret = mdr_register_exception(&g_einfo[i]);
		if (ret == 0)
			pr_err("mdr_register_exception fail, ret = [%u]\n", ret);
	}

	mdr_debug(LOGLEVEL_INFO, "end");
}

static int mdr_ap_dfx_power_off(struct sys_off_data *data)
{
	pr_info("called\n");
	return NOTIFY_DONE;
}

static void mdr_ap_register_power_off(void)
{
	struct sys_off_handler *handler;

	handler = register_sys_off_handler(SYS_OFF_MODE_POWER_OFF,
			SYS_OFF_PRIO_DEFAULT,
			mdr_ap_dfx_power_off,
			NULL
			);
	if (IS_ERR(handler))
		pr_err("register_sys_off_handler return error %ld\n", PTR_ERR(handler));
}

int mdr_ap_adapter_init(void)
{
	int ret;

	mdr_debug(LOGLEVEL_INFO, "init start\n");

	ret = lpcore_nmi_notify_init();
	if (ret) {
		pr_err("lpcore_nmi_notify_init fail\n");
		return ret;
	}

	ret = mdr_mem_init();
	if (ret < 0) {
		pr_err("mdr mem init failed\n");
		return ret;
	}

	mdr_ap_register_exception();

	ret = mdr_ap_register_module();
	if (ret) {
		pr_err("mdr_ap_register_module fail, ret = [%d]\n", ret);
		return ret;
	}

	atomic_notifier_chain_register(&panic_notifier_list, &mdr_ap_panic_block);

	mdr_ap_register_power_off();
	ret = register_trace_android_vh_ipi_stop(android_vh_ipi_flush_cache_handler, NULL);

	g_mdr_ap_init = 1;

	mdr_debug(LOGLEVEL_INFO, "init end\n");

	return 0;
}
