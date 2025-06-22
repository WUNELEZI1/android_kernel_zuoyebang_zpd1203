// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */
#define DRVNAME		"xring_avs"

#include <linux/arm-smccc.h>
#include <linux/crc32.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <soc/xring/dmd.h>
#include <soc/xring/xr_nv_interface.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

#define avs_log_err(args...)	pr_err("[avs err]"args)
#define avs_log_info(args...)	pr_info("[avs info]"args)
#define avs_log_debug(args...)	pr_info("[avs debug]"args)

#define DMD_BUFF_SIZE	512

#define VALID_IDX	0
#define SUBSYS_IDX	1
#define HPM_IDX		2
#define RO_IDX		3

struct avs_context {
	uint32_t nv_id;
	uint32_t nv_period_hz;
	struct delayed_work work;
};

static int reboot_notifier(struct notifier_block *this, unsigned long code, void *unused);

static struct dmd_dev g_dmd_dev = {
	.name = "avs",
	.module_name = "ACPU",
	.buff_size = DMD_BUFF_SIZE,
	.fault_level = "WARNING",
};
static struct dmd_client *g_dmd_client;
static struct avs_context g_avs_context = {0};
static struct notifier_block g_reboot_nb = {
	.notifier_call = reboot_notifier,
	.priority = INT_MAX,
};

static int report_hpm_error(void)
{
	struct arm_smccc_res res = {0};
	uint8_t *header;
	uint16_t hpm_codes[2];
	int ret;

	arm_smccc_smc(FID_BL31_AVS_GET_HPM_ERROR, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		avs_log_err("failed to get hpm error info\n");
		return -EIO;
	}
	header = (uint8_t *)&res.a1;
	if (!header[VALID_IDX]) {
		avs_log_info("no hpm error\n");
		return 0;
	}
	hpm_codes[0] = (uint16_t)res.a2;
	hpm_codes[1] = (uint16_t)res.a3;
	avs_log_err("subsys=%u hpm=%u ro=%u codes=[%u %u]\n",
		header[SUBSYS_IDX], header[HPM_IDX], header[RO_IDX], hpm_codes[0], hpm_codes[1]);
	if (!g_dmd_client)
		return -EIO;

	ret = dmd_client_record(g_dmd_client, DMD_ACPU_AVS_HPM_NO,
		"subsys=%u hpm=%u ro=%u codes=[%u %u]",
		header[SUBSYS_IDX], header[HPM_IDX], header[RO_IDX], hpm_codes[0], hpm_codes[1]);
	if (ret <= 0)
		avs_log_err("failed to report dmd ret %d\n", ret);

	return 0;
}

static int avs_dt_parse(struct device *dev, struct avs_context *avs_context)
{
	struct device_node *np = dev->of_node;
	uint32_t nv_period;

	if (of_property_read_u32(np, "nv-id",
		&avs_context->nv_id)) {
		avs_log_err("read prop nv-id fail\n");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "nv-period",
		&nv_period)) {
		avs_log_err("read prop nv-period fail\n");
		return -EINVAL;
	}
	avs_context->nv_period_hz = HZ * nv_period;
	avs_log_debug("nv-id %u nv-period %u hz\n",
		avs_context->nv_id, avs_context->nv_period_hz);

	return 0;
}

static void _work_handler(void)
{
	int ret;
	uint32_t size;
	uint32_t running_time;
	struct arm_smccc_res res = {0};
	struct nv_info_struct nv_info;
	uint32_t *crc_addr;
	uint32_t crc;
	uint32_t *content_addr;
	uint32_t content_size;

	arm_smccc_smc(FID_BL31_AVS_GET_RUNNING_TIME, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		avs_log_err("failed to get running time\n");
		return;
	}
	size = (uint32_t)res.a1;
	running_time = (uint32_t)res.a2;
	avs_log_debug("size %u running_time %u\n", size, running_time);

	nv_info.item_id = g_avs_context.nv_id;
	nv_info.operation = NV_READ;
	ret = nv_read_write_item(&nv_info);
	if (ret) {
		avs_log_err("failed to read nv item, ret:%d\n", ret);
		return;
	}

	crc_addr = (uint32_t *)nv_info.buf.nv_data;
	content_addr = crc_addr + 1;
	content_size = size - sizeof(uint32_t);
	crc = crc32(~0, (void *)content_addr, content_size) ^ ~0U;
	*content_addr = running_time;
	if (crc != *crc_addr)
		avs_log_err("nv invalid, expected crc 0x%x actual crc 0x%x\n",
			*crc_addr, crc);
	else
		*crc_addr = crc32(~0, (void *)content_addr, content_size) ^ ~0U;

	nv_info.operation = NV_WRITE;
	ret = nv_read_write_item(&nv_info);
	if (ret) {
		avs_log_err("failed to write nv item, ret:%d\n", ret);
		return;
	}
}

static void work_handler(struct work_struct *work)
{
	_work_handler();
	mod_delayed_work(system_freezable_power_efficient_wq,
		&g_avs_context.work, g_avs_context.nv_period_hz);
}

static int reboot_notifier(struct notifier_block *this, unsigned long code, void *unused)
{
	_work_handler();

	return NOTIFY_DONE;
}

static int avs_device_probe(struct platform_device *pdev)
{
	int ret;

	ret = avs_dt_parse(&pdev->dev, &g_avs_context);
	if (ret)
		return ret;
	ret = register_reboot_notifier(&g_reboot_nb);
	if (ret) {
		avs_log_err("failed to register reboot notifier, ret %d\n", ret);
		return ret;
	}

	INIT_DEFERRABLE_WORK(&g_avs_context.work, work_handler);
	queue_delayed_work(system_freezable_power_efficient_wq,
		&g_avs_context.work, g_avs_context.nv_period_hz);

	return 0;
}

static int avs_device_remove(struct platform_device *pdev)
{
	int ret;

	(void)cancel_delayed_work(&g_avs_context.work);
	ret = unregister_reboot_notifier(&g_reboot_nb);
	if (ret) {
		avs_log_err("failed to unregister reboot notifier, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct of_device_id g_avs_of_match[] = {
	{ .compatible = "xring,avs", },
	{},
};
MODULE_DEVICE_TABLE(of, g_avs_of_match);

static struct platform_driver g_avs_driver = {
	.driver = {
		.name = DRVNAME,
		.of_match_table = of_match_ptr(g_avs_of_match),
		.suppress_bind_attrs = true,
	},
	.probe = avs_device_probe,
	.remove = avs_device_remove,
};

static int __init avs_init(void)
{
	int ret;
	int ret_code = 0;

	g_dmd_client = dmd_register_client(&g_dmd_dev);
	if (!g_dmd_client) {
		avs_log_err("failed to regitser dmd\n");
		ret_code |= (-EIO);
	}
	ret_code |= report_hpm_error();

	ret = platform_driver_register(&g_avs_driver);
	if (ret != 0) {
		avs_log_err("failed to regitser platform driver %d\n", ret);
		ret_code |= ret;
	}

	if (ret_code != 0)
		avs_log_err("failed to init avs\n");

	return ret_code;
}
module_init(avs_init);

static void __exit avs_exit(void)
{
	if (g_dmd_client != NULL) {
		dmd_unregister_client(g_dmd_client, &g_dmd_dev);
		g_dmd_client = NULL;
	}
	platform_driver_unregister(&g_avs_driver);
}
module_exit(avs_exit);

MODULE_SOFTDEP("pre: dmd");
MODULE_SOFTDEP("pre: xr_nv");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("XRing avs driver");
MODULE_AUTHOR("xiaomi.com");
