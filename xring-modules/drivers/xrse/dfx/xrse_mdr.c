// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 * Description: xrse dfx driver
 * Modify time: 2024-04-03
 */

#include "../xrse_internal.h"
#include <linux/arm-smccc.h>
#include <linux/atomic.h>
#include <linux/preempt.h>
#include <linux/memory.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/irqreturn.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/ipc/xr_ipc.h>
#include <soc/xring/ipc/xr_rproc.h>
#include <soc/xring/ipc/xr_ipc_prot.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include "miev/mievent.h"

#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/xrse_platform.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>


#ifdef CONFIG_XRING_XRSE_BOOT_RETRY_RECORD
#define XRSE_BOOT_RETRY_WORK_FIRST_DELAY_MS      (1000 * 300)
#define XRSE_BOOT_RETRY_WORK_DELAY_MS            (1000 * 60 * 60 * 24)

#define XRSE_BOOT_RETRY_RECORD_NO                (1000)
#define XRSE_BOOT_RETRY_RECORD_APPID             "31000401732"
#define XRSE_BOOT_RETRY_RECORD_EVENT             "XrseBootRetryRecord"
#define XRSE_BOOT_RETRY_RECORD_PARA_0            "XrseRamBootRetryTimes"
#define XRSE_BOOT_RETRY_RECORD_PARA_1            "XrseDDRBootRetryTimes"
#define XRSE_BOOT_RETRY_RECORD_PARA_2            "XrseColdBootRetryTimes"

#define XRSE_BOOT_RETRY_RECORD_ADDR              (ACPU_LPIS_ACTRL + LPIS_ACTRL_SC_RSV_NS_20)
#define XRSE_BOOT_RETRY_RECORD_SIZE              (8)
#define XRSE_COLD_BOOT_RETRY_RECORD_OFFSET       (4)

#define XRSE_RAM_BOOT_RETRY_RECORD_MASK          0xFFU
#define XRSE_DDR_BOOT_RETRY_RECORD_MASK          0xFF00U
#define XRSE_COLD_BOOT_RETRY_RECORD_MASK         0xFFFFU

static void __iomem *g_xrse_boot_retry_record_addr;

static struct delayed_work g_xrse_boot_retry_work = {0};
#endif


#define XRSE_LOG_RECORD_FLAG_OFFSET              (4)
#define XRSE_LOG_RECORD_CNT_MAX                  (10000)

#define XRSE_DDR_LOG_DEV_NAME                    "xrse_rt_log"

#define XRSE_WAIT_READY_CNT_MAX                  (50)

struct xrse_dfx_intr_info {
	int irq_num;
	u32 irq_modid;
	char *intr_name;
};

enum xrse_log_record_status {
	XRSE_LOG_RECORD_UNREADY       = 0x3c3c3c3cU,
	XRSE_LOG_RECORD_READY_XRSE    = 0x3c3c5a5aU,
	XRSE_LOG_RECORD_READY_LPCORE  = 0x5a5a3c3cU,
	XRSE_LOG_RECORD_READY_ACPU    = 0x5a5a5a5aU,
};

static void __iomem *g_xrse_log_addr;

static struct xrse_dfx_intr_info g_info_table[] = {
	{0, MODID_XRSE_SENSOR_ERR, "sensor_intr"},
	{0, MODID_XRSE_SHIELD_ERR, "shield_intr"},
	{0, MODID_XRSE_SYS_PANIC,  "sysrst_intr"},
	{0, MODID_XRSE_MCU_LOCKUP, "lockup_intr"},
	{0, MODID_XRSE_WDT_TIMEOUT, "wdt_intr"  }
};

static struct mdr_exception_info_s g_xrse_exp[] = {
	{
		.e_modid            = (u32)MODID_XRSE_SYS_PANIC,
		.e_modid_end        = (u32)MODID_XRSE_SYS_PANIC,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_XRSE | MDR_AP | MDR_LPM3,
		.e_reset_core_mask  = MDR_XRSE,
		.e_from_core        = MDR_XRSE,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = XRSE_EXCEPTION,
		.e_exce_subtype     = XRSE_SYS_PANIC,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "xrse",
		.e_desc             = "xrse sys panic",
	},
	{
		.e_modid            = (u32)MODID_XRSE_MODULE_ERR,
		.e_modid_end        = (u32)MODID_XRSE_MODULE_ERR,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_XRSE | MDR_AP | MDR_LPM3,
		.e_reset_core_mask  = MDR_XRSE,
		.e_from_core        = MDR_XRSE,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = XRSE_EXCEPTION,
		.e_exce_subtype     = XRSE_MODULE_ERR,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "xrse",
		.e_desc             = "xrse module error",
	},
	{
		.e_modid            = (u32)MODID_XRSE_WDT_TIMEOUT,
		.e_modid_end        = (u32)MODID_XRSE_WDT_TIMEOUT,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_XRSE | MDR_AP | MDR_LPM3,
		.e_reset_core_mask  = MDR_XRSE,
		.e_from_core        = MDR_XRSE,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = XRSE_EXCEPTION,
		.e_exce_subtype     = XRSE_WDT_TIMEOUT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "xrse",
		.e_desc             = "xrse wdt timeout",
	},
	{
		.e_modid            = (u32)MODID_XRSE_MCU_LOCKUP,
		.e_modid_end        = (u32)MODID_XRSE_MCU_LOCKUP,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_XRSE | MDR_AP | MDR_LPM3,
		.e_reset_core_mask  = MDR_XRSE,
		.e_from_core        = MDR_XRSE,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = XRSE_EXCEPTION,
		.e_exce_subtype     = XRSE_MCU_LOCKUP,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "xrse",
		.e_desc             = "xrse mcu lockup",
	},
	{
		.e_modid            = (u32)MODID_XRSE_SENSOR_ERR,
		.e_modid_end        = (u32)MODID_XRSE_SENSOR_ERR,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_XRSE | MDR_AP | MDR_LPM3,
		.e_reset_core_mask  = MDR_XRSE,
		.e_from_core        = MDR_XRSE,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = XRSE_EXCEPTION,
		.e_exce_subtype     = XRSE_SENSOR_ERR,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "xrse",
		.e_desc             = "xrse sensor error",
	},
	{
		.e_modid            = (u32)MODID_XRSE_SHIELD_ERR,
		.e_modid_end        = (u32)MODID_XRSE_SHIELD_ERR,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_XRSE | MDR_AP | MDR_LPM3,
		.e_reset_core_mask  = MDR_XRSE,
		.e_from_core        = MDR_XRSE,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = XRSE_EXCEPTION,
		.e_exce_subtype     = XRSE_SHIELD_ERR,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "xrse",
		.e_desc             = "xrse shield error",
	},
};

static struct mdr_exception_info_s g_xrse_noc_exception = {
	.e_modid            = (uint32_t)MODID_XRSE_NOC_ERR,
	.e_modid_end        = (uint32_t)MODID_XRSE_NOC_ERR,
	.e_process_priority = MDR_ERR,
	.e_reboot_priority  = MDR_REBOOT_NO,
	.e_notify_core_mask = MDR_XRSE | MDR_AP | MDR_LPM3,
	.e_reset_core_mask  = MDR_XRSE,
	.e_from_core        = MDR_XRSE,
	.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
	.e_exce_type        = XRSE_EXCEPTION,
	.e_exce_subtype     = XRSE_NOC_ERR,
	.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
	.e_from_module      = "xrse",
	.e_desc             = "xrse noc error",
};

static void xrse_dfx_intr_enable(void)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(g_info_table); i++)
		enable_irq(g_info_table[i].irq_num);
}

static void xrse_dfx_intr_disable(void)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(g_info_table); i++)
		disable_irq_nosync(g_info_table[i].irq_num);
}

static inline void set_xrse_exception_done(void)
{
	writel(XRSE_LOG_RECORD_READY_ACPU, g_xrse_log_addr + XRSE_LOG_RECORD_FLAG_OFFSET);
}

static void xrse_mdr_dump(u32 modid, u32 etype, u64 coreid, char *log_path,
			  pfn_cb_dump_done pfn_cb, void *data)
{
	u32 value;
	u32 cnt = 0;

	xrse_err("mdr_dump: modid[%x],etype[%x],coreid[%llx],log_path[%s]\n",
		modid, etype, coreid, log_path);

	/* Waiting for xrse logging done */
	while (cnt < XRSE_LOG_RECORD_CNT_MAX) {
		value = readl(g_xrse_log_addr + XRSE_LOG_RECORD_FLAG_OFFSET);
		if ((value == XRSE_LOG_RECORD_READY_XRSE) || (value == XRSE_LOG_RECORD_READY_LPCORE)) {
			xrse_info("mdr_dump: ready value:0x%x\n", value);
			break;
		}

		udelay(10);
		cnt++;
	}

	if (cnt >= XRSE_LOG_RECORD_CNT_MAX)
		xrse_err("mdr log timeout! value: 0x%x\n", value);
}

static void xrse_mdr_reset(u32 modid, u32 etype, u64 coreid, void *data)
{
	u32 cnt = 0;

	xrse_err("mdr_reset: reset by lpctrl, acpu do nothing\n");

	set_xrse_exception_done();

	while (!is_xrse_ready()) {
		if (cnt > XRSE_WAIT_READY_CNT_MAX) {
			xrse_err("mdr_reset: wait xrse ready timeout\n");
			break;
		}

		msleep(20);
		cnt++;
	}

	xrse_dfx_intr_enable();
}

static int xrse_register_mdr(void)
{
	int i;
	int ret;
	struct mdr_module_ops mdr_xrse_ops = {0};
	struct mdr_register_module_result mdr_retinfo = {0};

	for (i = 0; i < ARRAY_SIZE(g_xrse_exp); i++) {
		ret = mdr_register_exception(&g_xrse_exp[i]);
		if (ret == 0) {
			xrse_err("exception %d register fail, ret %d.\n", i, ret);
			return ret;
		}
	}

	ret = noc_register_exception(&g_xrse_noc_exception);
	if (ret != 0) {
		xrse_err("noc exception register fail, ret=%d.\n", ret);
		return ret;
	}

	mdr_xrse_ops.ops_dump = xrse_mdr_dump;
	mdr_xrse_ops.ops_reset = xrse_mdr_reset;
	mdr_xrse_ops.ops_data = NULL;

	ret = mdr_register_module_ops(MDR_XRSE, &mdr_xrse_ops, &mdr_retinfo);
	if (ret < 0) {
		xrse_err("mdr ops register fail, ret %d.\n", ret);
		return ret;
	}

	xrse_info("register mdr success\n");

	return 0;
}

static u32 xrse_dfx_exception_identify(int irq)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(g_info_table); i++) {
		if (irq == g_info_table[i].irq_num)
			return g_info_table[i].irq_modid;
	}

	return 0;
}

static irqreturn_t xrse_dfx_common_isr(int irq, void *dev_id)
{
	u32 modid;

	xrse_dfx_intr_disable();

	xrse_warn("exception IRQ %d\n", irq);

	modid = xrse_dfx_exception_identify(irq);
	if (modid == 0) {
		xrse_dfx_intr_enable();
		return IRQ_HANDLED;
	}

	mdr_system_error(modid, 0, 0);

	return IRQ_HANDLED;
}

static int xrse_register_exception_irq(struct platform_device *pdev)
{
	int i;
	int ret = 0;
	struct device *dev = NULL;

	if (pdev == NULL) {
		xrse_err("dfx: pdev is null\n");
		return -EINVAL;
	}

	dev = &pdev->dev;

	for (i = 0; i < ARRAY_SIZE(g_info_table); i++) {
		g_info_table[i].irq_num = platform_get_irq(pdev, i);
		if (g_info_table[i].irq_num < 0) {
			dev_err(dev, "Failed to get %s interrupt\n", g_info_table[i].intr_name);
			return g_info_table[i].irq_num;
		}

		dev_info(dev, "%s = %d\n", g_info_table[i].intr_name, g_info_table[i].irq_num);

		ret = devm_request_irq(dev, g_info_table[i].irq_num, xrse_dfx_common_isr,
				       IRQF_SHARED, g_info_table[i].intr_name, pdev);
		if (ret) {
			dev_err(dev, "Failed to request %s interrupt\n", g_info_table[i].intr_name);
			return ret;
		}
	}

	return ret;
}

#ifdef CONFIG_XRING_XRSE_BOOT_RETRY_RECORD

static void xrse_mi_event_write(int ram_boot_retry_times, int ddr_boot_retry_times, int cold_boot_retry_times)
{
	struct misight_mievent *event;

	/* NOTICE: record content */
	event = cdev_tevent_alloc(XRSE_BOOT_RETRY_RECORD_NO);
	if (!event) {
		xrse_err("misight event is error\n");
		return;
	}

	cdev_tevent_add_str(event, "appId", XRSE_BOOT_RETRY_RECORD_APPID);
	cdev_tevent_add_str(event, "eventName", XRSE_BOOT_RETRY_RECORD_EVENT);
	cdev_tevent_add_int(event, XRSE_BOOT_RETRY_RECORD_PARA_0, ram_boot_retry_times);
	cdev_tevent_add_int(event, XRSE_BOOT_RETRY_RECORD_PARA_1, ddr_boot_retry_times);
	cdev_tevent_add_int(event, XRSE_BOOT_RETRY_RECORD_PARA_2, cold_boot_retry_times);
	cdev_tevent_write(event);
	cdev_tevent_destroy(event);
}

static void xrse_boot_retry_record(void)
{
	int ram_boot_retry_times = 0;
	int ddr_boot_retry_times = 0;
	int cold_boot_retry_times = 0;

	if (!g_xrse_boot_retry_record_addr) {
		xrse_err("xrse_boot_retry_record_addr is null\n");
		return;
	}

	ram_boot_retry_times = readl(g_xrse_boot_retry_record_addr) & XRSE_RAM_BOOT_RETRY_RECORD_MASK;
	ddr_boot_retry_times = (readl(g_xrse_boot_retry_record_addr) & XRSE_DDR_BOOT_RETRY_RECORD_MASK) >> 8;
	cold_boot_retry_times = readl((void __iomem *)((uintptr_t)g_xrse_boot_retry_record_addr +
					XRSE_COLD_BOOT_RETRY_RECORD_OFFSET)) & XRSE_COLD_BOOT_RETRY_RECORD_MASK;

	if (!(ram_boot_retry_times | ddr_boot_retry_times | cold_boot_retry_times))
		return;

	xrse_warn("xrse boot retry record:ram_boot: %d, ddr_boot %d, cold_boot %d\n",
			ram_boot_retry_times, ddr_boot_retry_times, cold_boot_retry_times);

	xrse_mi_event_write(ram_boot_retry_times, ddr_boot_retry_times, cold_boot_retry_times);

	/* clear BMRW reg */
	writel(0xffff0000, g_xrse_boot_retry_record_addr);
	writel(0xffff0000, (void __iomem *)((uintptr_t)g_xrse_boot_retry_record_addr +
	       XRSE_COLD_BOOT_RETRY_RECORD_OFFSET));
}

static void xrse_boot_retry_work_fn(struct work_struct *work)
{
	int ret;

	xrse_boot_retry_record();
	ret = schedule_delayed_work(&g_xrse_boot_retry_work, msecs_to_jiffies(XRSE_BOOT_RETRY_WORK_DELAY_MS));
	if (!ret)
		xrse_warn("xrse retry record work insert kernel-global workqueue failed!\n");
}
#endif

static int xrse_dfx_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = xrse_register_mdr();
	if (ret != 0) {
		xrse_err("mdr register xrse failed: %d\n", ret);
		return ret;
	}

	ret = xrse_register_exception_irq(pdev);
	if (ret != 0) {
		xrse_err("register exception isr failed: %d\n", ret);
		return ret;
	}

	g_xrse_log_addr = ioremap_wc(DFX_MEM_XRSE_LOG_ADDR, DFX_MEM_XRSE_LOG_SIZE);
	if (!g_xrse_log_addr) {
		xrse_err("dfx ioremap failed\n");
		return -ENOMEM;
	}

#ifdef CONFIG_XRING_XRSE_BOOT_RETRY_RECORD
	g_xrse_boot_retry_record_addr = ioremap(XRSE_BOOT_RETRY_RECORD_ADDR, XRSE_BOOT_RETRY_RECORD_SIZE);
	if (!g_xrse_boot_retry_record_addr) {
		xrse_err("record addr ioremap failed\n");
		return -ENOMEM;
	}

	INIT_DEFERRABLE_WORK(&g_xrse_boot_retry_work, xrse_boot_retry_work_fn);
	ret = schedule_delayed_work(&g_xrse_boot_retry_work, msecs_to_jiffies(XRSE_BOOT_RETRY_WORK_FIRST_DELAY_MS));
	if (!ret)
		xrse_warn("xrse retry record work insert kernel-global workqueue failed!\n");
#endif

	xrse_info("dfx driver init success\n");

	return 0;
}

static int xrse_dfx_remove(struct platform_device *pdev)
{
	mdr_unregister_module_ops(MDR_XRSE);

	if (g_xrse_log_addr != NULL) {
		iounmap(g_xrse_log_addr);
		g_xrse_log_addr = NULL;
	}

#ifdef CONFIG_XRING_XRSE_BOOT_RETRY_RECORD
	if (g_xrse_boot_retry_record_addr != NULL) {
		iounmap(g_xrse_boot_retry_record_addr);
		g_xrse_boot_retry_record_addr = NULL;
	}
#endif

	xrse_info("dfx driver exit\n");

	return 0;
}

static const struct of_device_id xrse_dfx_of_match[] = {
	{ .compatible = "xring,xrse_dfx" },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, xrse_dfx_of_match);

static struct platform_driver xrse_dfx_driver = {
	.probe = xrse_dfx_probe,
	.remove = xrse_dfx_remove,
	.driver = {
		.name = "xrse-dfx",
		.of_match_table = of_match_ptr(xrse_dfx_of_match),
	},
};

int xrse_dfx_init(void)
{
	return platform_driver_register(&xrse_dfx_driver);
}

void xrse_dfx_exit(void)
{
	platform_driver_unregister(&xrse_dfx_driver);
}

MODULE_DESCRIPTION("XRing xrse dfx driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("post: xring-mdr");
