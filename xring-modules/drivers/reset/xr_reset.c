// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/reset-controller.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/reset/xr_reset_common.h>
#include <dt-bindings/xring/platform-specific/common/reset/xr_reset_op.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include "xr_reset_internal.h"
#include "xsp_ffa.h"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)     "XR_IP_RESET: " fmt

#define RST_WRITE(addr, val)    writel(val, addr)
#define RST_READ(addr)          readl(addr)

static inline struct xr_reset_data *to_xr_reset_data(struct reset_controller_dev *rcdev)
{
	return container_of(rcdev, struct xr_reset_data, rcdev);
}

static int secure_reset_op(struct reset_controller_dev *rcdev,
		unsigned long opcode, unsigned long idx)
{
	struct xr_reset_data *data = to_xr_reset_data(rcdev);
	const struct xr_reset_config *cfg = &data->desc->resets[idx];
	struct xsp_ffa_msg msg;
	int ret;

	memset(&msg, 0, sizeof(msg));

	if (opcode == XR_RST_OP_STATUS)
		msg.fid = FID_BL31_RESET_GET;
	else
		msg.fid = FID_BL31_RESET_SET;

	msg.data0 = opcode;
	msg.data1 = XR_RST_METHOD_SCR;
	msg.data2 = XR_RST_SMC_PARA_PACK(data->id, cfg->offset, cfg->shift);

	ret = xrsp_ffa_direct_message(&msg);
	if (ret != 0) {
		dev_err(rcdev->dev, "ffa message failed, ret = %d\n", ret);
		return -EINVAL;
	}
	if (msg.ret != 0) {
		dev_err(rcdev->dev, "%s: error: ret = %#lx\n", __func__, msg.ret);
		return -EINVAL;
	}

	if (opcode == XR_RST_OP_STATUS)
		return msg.data1;
	else
		return 0;
}

static int scr_control_assert(struct reset_controller_dev *rcdev, unsigned long idx)
{
	struct xr_reset_data *data = to_xr_reset_data(rcdev);
	const struct xr_reset_config *cfg = &data->desc->resets[idx];
	int ret = 0;

	if ((rcdev == NULL) || (rcdev->dev == NULL)) {
		pr_err("xr_reset no dev\n");
		return -ENODEV;
	}

	if (cfg->name == 0) {
		dev_err(rcdev->dev, "Invalid reset index: %lu\n", idx);
		return -EINVAL;
	}

	if (cfg->is_secured)
		ret = secure_reset_op(rcdev, XR_RST_OP_ASSERT, idx);
	else
		SCR_RESET_ASSERT(data->base, cfg->offset, cfg->shift);

	return ret;
}

static int scr_control_deassert(struct reset_controller_dev *rcdev, unsigned long idx)
{
	struct xr_reset_data *data = to_xr_reset_data(rcdev);
	const struct xr_reset_config *cfg = &data->desc->resets[idx];
	int ret = 0;

	if ((rcdev == NULL) || (rcdev->dev == NULL)) {
		pr_err("xr_reset no dev\n");
		return -ENODEV;
	}

	if (cfg->name == 0) {
		dev_err(rcdev->dev, "Invalid reset index: %lu\n", idx);
		return -EINVAL;
	}

	if (cfg->is_secured)
		ret = secure_reset_op(rcdev, XR_RST_OP_DEASSERT, idx);
	else
		SCR_RESET_DEASSERT(data->base, cfg->offset, cfg->shift);

	return 0;
}

static int scr_control_reset(struct reset_controller_dev *rcdev, unsigned long idx)
{
	int ret;

	ret = scr_control_assert(rcdev, idx);
	if (ret)
		return ret;

	udelay(10);

	return scr_control_deassert(rcdev, idx);
}

static int scr_control_status(struct reset_controller_dev *rcdev, unsigned long idx)
{
	struct xr_reset_data *data = to_xr_reset_data(rcdev);
	const struct xr_reset_config *cfg = &data->desc->resets[idx];
	int status;

	if ((rcdev == NULL) || (rcdev->dev == NULL)) {
		pr_err("xr_reset no dev\n");
		return -ENODEV;
	}

	if (cfg->name == 0) {
		dev_err(rcdev->dev, "Invalid reset index: %lu\n", idx);
		return -EINVAL;
	}

	if (cfg->is_secured) {
		status = secure_reset_op(rcdev, XR_RST_OP_STATUS, idx);
		if (status < 0)
			return status;
	} else {
		status = SCR_RESET_STATUS(data->base, cfg->offset, cfg->shift);
	}

	return status;
}

static const struct reset_control_ops xr_reset_ops[] = {
	[XR_RST_METHOD_SCR] = {
		.reset    = scr_control_reset,
		.assert   = scr_control_assert,
		.deassert = scr_control_deassert,
		.status   = scr_control_status,
	},
};

int xr_reset_probe(struct platform_device *pdev, const struct xr_reset_desc *desc)
{
	int ret;
	struct xr_reset_data *data = NULL;
	struct resource *res = NULL;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	u32 opcode, id;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->desc = desc;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	data->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(data->base))
		return PTR_ERR(data->base);

	ret = of_property_read_u32_array(np, "opcode", &opcode, 1);
	if (ret) {
		dev_err(dev, "Get opcode failed\n");
		return ret;
	}

	if (opcode >= XR_RST_METHOD_CNT) {
		dev_err(dev, "Invalid opcode: %d\n", opcode);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "rst_id", &id, 1);
	if (ret) {
		dev_err(dev, "Get rst_id failed\n");
		return ret;
	}

	data->id = id;

	data->rcdev.owner = THIS_MODULE;
	data->rcdev.ops = &xr_reset_ops[opcode];
	data->rcdev.nr_resets = desc->num_resets;
	data->rcdev.of_node = dev->of_node;
	data->rcdev.dev = dev;

	return devm_reset_controller_register(dev, &data->rcdev);
}

MODULE_AUTHOR("Jinfei Weng <wengjinfei@xiaomi.com>");
MODULE_DESCRIPTION("X-Ring Reset Driver");
MODULE_LICENSE("GPL v2");
