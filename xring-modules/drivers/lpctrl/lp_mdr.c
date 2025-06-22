// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <soc/xring/ipc/xr_ipc_prot.h>
#include <soc/xring/flush_cache.h>
#include <linux/of.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "ipc_resource.h"
#include "ipc_tags_lpctrl_ap.h"
#include "ipc_cmds_lpctrl_ap.h"
#include "mdr_public_if.h"
#include "mdr_pub.h"
#else
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_lpctrl_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_cmds_lpctrl_ap.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#endif
#include "./lp_dmd.h"

#define VC_ID_AP_TO_LP	IPC_VC_PERI_NS_AP_LP_COMMON_1

#ifdef LP_MDR_DEBUG
#define lp_mdr_debug(fmt, ...)         \
	pr_err("[%s][E]: " fmt, __func__, ##__VA_ARGS__)
#else
#define lp_mdr_debug(fmt, ...)
#endif

struct mdr_lp_addr {
	u64 start_addr;
	u32 len;
};

static int send_ipc_to_lp(u8 cmd, void *tx_buff, size_t tx_len)
{
	int i;

	lp_mdr_debug("send ipc to lpcore CMD: %d\n", cmd);
	for (i = 0; i < tx_len; i++)
		lp_mdr_debug("      ipc buf[%d] %d\n", i, *(char *)(tx_buff + i));

	return ap_ipc_send_sync(VC_ID_AP_TO_LP, TAG_LP_MDR, cmd, tx_buff, tx_len);
}

static void lp_dump(u32 modid, u32 etype, u64 coreid, char *log_path,
			pfn_cb_dump_done pfn_cb, void *data)
{
	int ret;

	ret = send_ipc_to_lp(CMD_LP_MDR_DUMP, NULL, 0);
	if (ret != 0) {
		pr_err("Send Dump Request to Lpctrl F:%d!\n", ret);
		return;
	}

	lp_mdr_debug("mdr lpmcu dump\n");
}

static void lp_reset(u32 modid, u32 etype, u64 coreid, void *data)
{
	lp_mdr_debug("mdr lpmcu reset\n");
}

static irqreturn_t lpctrl_notify_reset(int irq_num, void *dev_id)
{
	disable_irq_nosync(irq_num);
	pr_err("lpctrl reset system!\n");
	flush_dfx_cache();

	return IRQ_HANDLED;
}

static void lpctrl_init_mdr_irq(struct platform_device *plat_dev)
{
	int irq;
	int ret;
	struct device *dev = &plat_dev->dev;

	irq = platform_get_irq(plat_dev, 0);
	if (irq < 0) {
		dev_err(dev, "platform_get_irq failed\n");
		return;
	}

	ret = devm_request_irq(dev, irq, lpctrl_notify_reset, 0, "notify_reset", dev);
	if (ret != 0)
		dev_err(dev, "Register notify_reset irq fail %d\n", ret);

	lp_mdr_debug("request IRQ done\n");
}

static void lpctrl_mdr_register(void)
{
	struct mdr_module_ops mdr_lp_ops;
	struct mdr_register_module_result mdr_lp_result;
	struct mdr_lp_addr lp_addr = {0, 0};
	int ret;
	u8 cmd;

	mdr_lp_ops.ops_dump = lp_dump;
	mdr_lp_ops.ops_reset = lp_reset;
	mdr_lp_ops.ops_data = NULL;

	ret = mdr_register_module_ops(MDR_LPM3, &mdr_lp_ops, &mdr_lp_result);
	if (ret < 0) {
		pr_err("lp mdr module register fail, ret %d.\n", ret);
		cmd = CMD_LP_MDR_INIT_F;
	} else {
		cmd = CMD_LP_MDR_DFX_ADDR;
		lp_addr.start_addr = mdr_lp_result.log_addr;
		lp_addr.len = mdr_lp_result.log_len;
	}

	send_ipc_to_lp(cmd, (void *)&lp_addr, sizeof(struct mdr_lp_addr));
}

static int lpctrl_mdr_probe(struct platform_device *plat_dev)
{
	lp_mdr_debug("Start\n");

	lpctrl_init_mdr_irq(plat_dev);

	lpctrl_mdr_register();

	lp_mdr_debug("Succ\n");

	lpcore_dmd_init();

	return 0;
}

static int lpctrl_mdr_remove(struct platform_device *plat_dev)
{
	mdr_unregister_module_ops(MDR_LPM3);

	lpcore_dmd_exit();

	return 0;
}

static const struct of_device_id g_lpctrl_dfx[] = {
	{ .compatible = "xring,lpctrl_dfx" },
	{}
};
MODULE_DEVICE_TABLE(of, g_lpctrl_dfx);

static struct platform_driver g_lpctrl_mdr = {
	.probe = lpctrl_mdr_probe,
	.remove = lpctrl_mdr_remove,
	.driver = {
		.name = "lpctrl_mdr",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_lpctrl_dfx),
	},
};
module_platform_driver(g_lpctrl_mdr);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("dfx driver for lp");
MODULE_AUTHOR("xiaomi.com");
