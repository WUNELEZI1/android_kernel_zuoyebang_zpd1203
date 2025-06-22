// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <soc/xring/sensorhub/ipc/shub_ipc_drv.h>
#include <soc/xring/sensorhub/shub_boot_prepare.h>
#include <soc/xring/sensorhub/shub_notifier.h>
#include <soc/xring/sensorhub/shub_time_sync.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/dfx_switch.h>
#include <dt-bindings/xring/platform-specific/sensorhub_ddr_layout.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/ipc_cmds_sh_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_sh_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/sensorhub_dfx.h>
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>
#include <asm/io.h>
#include "shub_dfx_priv.h"

enum {
	SH_BOOT_EN = 1,
	AP_WAKEUP_SH,
	SH_BOOT_STATE
};

#define AP_SR_IPC_CHN_SH       IPC_VC_AO_NS_SH_AP_DEFAULT

static struct ipc_notifier_info g_sh_boot_msg;

static int aoc_power_sync_reg_reset(void)
{
	int      val;
	void __iomem  *baseaddr;

	baseaddr = ioremap(SH_XRISP_AP_CAM_POWER_STATUS_BASE,
		SH_XRISP_AP_CAM_POWER_STATUS_SIZE);
	if (unlikely(baseaddr == NULL)) {
		pr_err("aoc reg iomap failed");
		return -ENOMEM;
	}

	val = SH_XRISP_SH_CAM_POWER_STATUS_IDLE & SH_XRISP_SH_CAM_POWER_STATUS_MASK;
	val |= (SH_XRISP_SH_CAM_POWER_STATUS_MASK << SH_XRISP_AP_CAM_ACTRL_BMRW_SHIFT);
	writel(val, baseaddr);
	pr_info("set aoc power sync_reg[%ld]=[0x%x]", SH_XRISP_SH_CAM_POWER_STATUS_MASK, val);

	val = SH_XRISP_SH_CAM_POWER_STATUS_IDLE & SH_XRISP_SH_CAM_CRASH_STATUS_MASK;
	val |= (SH_XRISP_SH_CAM_CRASH_STATUS_MASK << SH_XRISP_AP_CAM_ACTRL_BMRW_SHIFT);
	writel(val, baseaddr);
	pr_info("set aoc power sync_reg[%ld]=[0x%x]", SH_XRISP_SH_CAM_CRASH_STATUS_MASK, val);

	if (likely(baseaddr))
		iounmap(baseaddr);
	return 0;
}

static void shub_wdt_sw_init(void)
{
	bool flag = is_switch_on(SWITCH_SHUB_WDT);
	struct sh_bsp_dts *bsp_dts = shub_get_bsp_dts();

	bsp_dts->wdt_switch = flag ? 1 : 0;
	pr_warn("%s flag=%u\n", __func__, flag);
}

static int shub_boot_atomic_ipc_cb(struct ipc_pkt *pkt, uint16_t tag, uint8_t cmd, void *arg)
{
	int ret = 0;
	int data = *(uint32_t *)pkt->data;
	uint8_t recovery_state = 0;

	pr_warn("sh boot base ipc_cb tag = 0x%x, cmd = 0x%x, data = 0x%x!\n", tag, cmd, data);
	if ((tag == TAG_SH_PM_AP_SR) && (cmd == CMD_SH_BOOT_STATE)) {
		recovery_state = (uint8_t)(data >> SH_RECOVERY_MESSAGE_PART_SHIFT);
		data = data & SH_BOOT_MESSAGE_PART_MASK;
		ret = shub_boot_atomic_notifier_call_chain(data);
		pr_info("sh boot recovery_state=0x%x\n", recovery_state);
		if (recovery_state == FLAG_SH_ALL_READY)
			recovery_msg_handler(ACTION_SH_RECOVERY_ALL_READY);

		pr_warn("sh boot state=0x%x,ret=%d!\n", data, ret);
	}
	return ret;
}

static int shub_boot_probe(struct platform_device *pdev)
{
	int ret = 0;

	shub_wdt_sw_init();

	g_sh_boot_msg.callback = shub_boot_atomic_ipc_cb;
	g_sh_boot_msg.arg = NULL;
	ret = sh_register_recv_notifier(IPC_VC_AO_NS_SH_AP_DEFAULT,
		TAG_SH_PM_AP_SR, &g_sh_boot_msg);
	if (ret != 0) {
		pr_warn("register ipc notifier err, ret = %d!\n", ret);
		goto _err_node;
	}

	sh_log_ddr_rd_ptr_init();
	shub_timesync_init();
	shub_timesync_start();

	ret = sh_ipc_send_msg_sync(AP_SR_IPC_CHN_SH, TAG_SH_PM_AP_SR, CMD_SH_PM_AP_SR_ON, NULL, 0);
	if (ret < 0)
		pr_err("ap->lpctrl ipc failed, ret=0x%x\n", ret);
	if (aoc_power_sync_reg_reset())
		pr_err("%s failed!\n", __func__);
	else
		pr_info("%s success!\n", __func__);

_err_node:
	pr_info("shub boot probe end\n");
	return ret;
}

static int shub_boot_remove(struct platform_device *pdev)
{
	int ret = 0;

	ret = sh_unregister_recv_notifier(IPC_VC_AO_NS_SH_AP_DEFAULT,
		TAG_SH_PM_AP_SR, &g_sh_boot_msg);
	if (ret != 0)
		pr_warn("unregister ipc notifier err,ret = %d!\n", ret);

	pr_info("shub boot remove end!\n");

	return ret;
}

#if IS_ENABLED(CONFIG_PM_SLEEP)
static int shub_boot_suspend(struct device *dev)
{
	int ret = 0;

	sh_timesync_cancel();

	dev_info(dev, "%s s\n", __func__);
	ret = sh_ipc_send_msg_sync(AP_SR_IPC_CHN_SH, TAG_SH_PM_AP_SR, CMD_SH_PM_AP_SR_OFF, NULL, 0);
	if (ret < 0)
		dev_err(dev, "Acore suspend: ap->sh ipc failed,ret=%x\n", ret);

	dev_info(dev, "%s e\n", __func__);
	return 0;
}

static int shub_boot_resume(struct device *dev)
{
	int ret = 0;

	shub_timesync_start();

	dev_info(dev, "%s s\n", __func__);
	ret = sh_ipc_send_msg_sync(AP_SR_IPC_CHN_SH, TAG_SH_PM_AP_SR, CMD_SH_PM_AP_SR_ON, NULL, 0);
	if (ret < 0)
		dev_err(dev, "Acore resume: ap->sh ipc failed,ret=%x\n", ret);

	dev_info(dev, "%s e\n", __func__);
	return 0;
}

#endif

static SIMPLE_DEV_PM_OPS(shub_boot_pm_ops, shub_boot_suspend, shub_boot_resume);

static const struct of_device_id shub_boot_dt_match[] = {
	{.compatible = "xring,sensorhub",},
	{}
};

static struct platform_driver shub_boot_driver = {
	.driver = {
		.name = "shub_boot",
		.pm = &shub_boot_pm_ops,
		.of_match_table = of_match_ptr(shub_boot_dt_match),
	},
	.probe  = shub_boot_probe,
	.remove = shub_boot_remove,
};

static int shub_boot_drv_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&shub_boot_driver);
	if (ret) {
		pr_err("shub_boot driver register failed, %d\n", ret);
		goto _err_node;
	}

	pr_info("shub boot drv init end\n");

_err_node:
	return ret;
}

static void shub_boot_drv_exit(void)
{
	platform_driver_unregister(&shub_boot_driver);
	pr_info("shub boot drv exit\n");
}

module_init(shub_boot_drv_init);
module_exit(shub_boot_drv_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_SOFTDEP("pre: shub_sensor");
