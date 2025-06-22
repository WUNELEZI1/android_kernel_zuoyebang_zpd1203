// SPDX-License-Identifier: GPL-2.0
/*
 * xr-usb-reset.c - Reset control for USB subsys.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "xr-usb-reset-plat.h"

#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/reset-controller.h>

#undef pr_fmt
#undef dev_fmt
#define pr_fmt(fmt) "[%s]: %s:%d: " fmt, KBUILD_MODNAME, __func__, __LINE__
#define dev_fmt(fmt) ": %s:%d: " fmt, __func__, __LINE__

#define XR_USB_RESET_MAX_REG_NUM 2

struct xr_usb_reset {
	struct reset_controller_dev rcdev;
	void __iomem *regs[XR_USB_RESET_MAX_REG_NUM];
	const struct xring_usb_reset_plat *plat_data;
};

static inline struct xr_usb_reset *
to_xr_usb_reset(struct reset_controller_dev *rcdev)
{
	return container_of(rcdev, struct xr_usb_reset, rcdev);
}

static void xr_usb_reset_config_reg_scr(void __iomem *addr, unsigned int val,
					bool assert, bool active_low)
{
	size_t clear_reg_shift = sizeof(unsigned int);

	if (assert ^ active_low)
		writel(val, addr);
	else
		writel(val, addr + clear_reg_shift);
}

static void xr_usb_reset_config_reg_rw(void __iomem *addr, unsigned int mask, unsigned int val,
				       bool assert, bool active_low)
{
	unsigned int reg = readl(addr);

	reg &= ~mask;
	val &= mask;

	if (assert ^ active_low)
		reg |= val;
	else
		reg &= ~val;

	writel(reg, addr);
}

static void xr_usb_reset_config_reg_bmrw(void __iomem *addr, unsigned int mask, unsigned int val,
					 bool assert, bool active_low)
{
	int bit_mask_shift = 16;
	unsigned int config_mask = 0xFFFF;
	unsigned int reg;

	mask &= config_mask;
	val &= config_mask;
	reg = (mask << bit_mask_shift);

	if (assert ^ active_low)
		reg |= val;

	writel(reg, addr);
}

static void __iomem *xr_usb_reset_get_reg_base(struct xr_usb_reset *data,
					       unsigned long id)
{
	struct device *dev = data->rcdev.dev;
	const struct xring_usb_reset_plat *plat_data = data->plat_data;
	const struct xring_usb_reset_data *reset_data = NULL;

	if (id >= USB_RESET_IDX_MAX) {
		dev_err(dev, "id %lu exceeds max\n", id);
		return NULL;
	}

	reset_data = &plat_data->resets_data[id];
	if (reset_data->base_idx < 0 ||
	    reset_data->base_idx >= plat_data->num_reg_base) {
		dev_err(dev, "id%lu invalid base_idx %d max %d\n", id,
			reset_data->base_idx, plat_data->num_reg_base);
		return NULL;
	}

	return data->regs[reset_data->base_idx];
}

static int xr_usb_reset_update(struct reset_controller_dev *rcdev,
			       unsigned long id, bool assert)
{
	struct xr_usb_reset *data = to_xr_usb_reset(rcdev);
	const struct xring_usb_reset_plat *plat_data = data->plat_data;
	const struct xring_usb_reset_data *reset_data = NULL;
	void __iomem *reg_base = NULL;

	if (!plat_data) {
		dev_err(rcdev->dev, "plat data is null\n");
		return -ENOENT;
	}

	reg_base = xr_usb_reset_get_reg_base(data, id);
	if (!reg_base) {
		dev_err(rcdev->dev, "reg_base is null\n");
		return -EINVAL;
	}
	reset_data = &plat_data->resets_data[id];
	switch (reset_data->type) {
	case XR_USB_REG_TYPE_SCR:
		xr_usb_reset_config_reg_scr(reg_base + reset_data->offset,
					    reset_data->value, assert,
					    reset_data->active_low);
		break;
	case XR_USB_REG_TYPE_RW:
		xr_usb_reset_config_reg_rw(reg_base + reset_data->offset,
					   reset_data->mask, reset_data->value,
					   assert, reset_data->active_low);
		break;
	case XR_USB_REG_TYPE_BMRW:
		xr_usb_reset_config_reg_bmrw(reg_base + reset_data->offset,
					     reset_data->mask,
					     reset_data->value, assert,
					     reset_data->active_low);
		break;
	default:
		dev_err(rcdev->dev, "unsupported reg type\n");
		break;
	}
	dev_dbg(rcdev->dev,
		"config reg id %lu, idx %d, offset 0x%x, value 0x%x, mask 0x%x, type %d, active_low %s, assert %s\n",
		id, reset_data->base_idx, reset_data->offset, reset_data->value,
		reset_data->mask, reset_data->type,
		reset_data->active_low ? "active_low" : "active_high",
		assert ? "assert" : "deassert");
	return 0;
}

static int xr_usb_reset_assert(struct reset_controller_dev *rcdev,
			       unsigned long id)
{
	return xr_usb_reset_update(rcdev, id, true);
}

static int xr_usb_reset_deassert(struct reset_controller_dev *rcdev,
				 unsigned long id)
{
	return xr_usb_reset_update(rcdev, id, false);
}

static int xr_usb_reset_status(struct reset_controller_dev *rcdev,
			       unsigned long id)
{
	struct xr_usb_reset *data = to_xr_usb_reset(rcdev);
	const struct xring_usb_reset_plat *plat_data = data->plat_data;
	const struct xring_usb_reset_data *reset_data = NULL;
	void __iomem *reg_base = NULL;
	unsigned int status_reg_shift;
	unsigned int reg = 0;

	if (!plat_data) {
		dev_err(rcdev->dev, "plat data is null\n");
		return -ENOENT;
	}

	reg_base = xr_usb_reset_get_reg_base(data, id);
	if (!reg_base) {
		dev_err(rcdev->dev, "reg_base is null\n");
		return -EINVAL;
	}
	reset_data = &plat_data->resets_data[id];
	switch (reset_data->type) {
	case XR_USB_REG_TYPE_SCR:
		status_reg_shift = 2 * sizeof(unsigned int);
		reg = readl(reg_base + reset_data->offset + status_reg_shift);
		break;
	case XR_USB_REG_TYPE_RW:
	case XR_USB_REG_TYPE_BMRW:
		reg = readl(reg_base + reset_data->offset);
		break;
	default:
		dev_err(rcdev->dev, "unsupported reg type\n");
		break;
	}

	return !(reg & reset_data->mask) ^ !reset_data->active_low;
}

const struct reset_control_ops xr_usb_reset_ops = {
	.assert = xr_usb_reset_assert,
	.deassert = xr_usb_reset_deassert,
	.status = xr_usb_reset_status,
};

static void __iomem *
devm_ioremap_resource_no_req_region(struct platform_device *pdev,
				    unsigned int index)
{
	struct resource *res = NULL;

	res = platform_get_resource(pdev, IORESOURCE_MEM, index);
	if (!res) {
		dev_err(&pdev->dev, "failed to get index %u memory property\n",
			index);
		return NULL;
	}

	return devm_ioremap(&pdev->dev, res->start, resource_size(res));
}

static int xr_usb_reset_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct xr_usb_reset *data = NULL;
	const struct xring_usb_reset_plat *plat_data = NULL;
	unsigned int i;

	dev_info(dev, "probe enter\n");

	if (!np)
		return -ENOENT;

	plat_data = of_device_get_match_data(dev);
	if (!plat_data) {
		dev_err(dev, "plat_data is null\n");
		return -ENOENT;
	}

	if (plat_data->num_reg_base > XR_USB_RESET_MAX_REG_NUM) {
		dev_err(dev, "num_reg_base %d exceeds max %d\n",
			plat_data->num_reg_base, XR_USB_RESET_MAX_REG_NUM);
		return -ENOENT;
	}

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	for (i = 0; i < plat_data->num_reg_base; ++i) {
		data->regs[i] = devm_ioremap_resource_no_req_region(pdev, i);
		if (!data->regs[i])
			return -ENOMEM;
	}

	data->plat_data = plat_data;
	data->rcdev.owner = THIS_MODULE;
	data->rcdev.ops = &xr_usb_reset_ops;
	data->rcdev.of_node = dev->of_node;
	data->rcdev.nr_resets = USB_RESET_IDX_MAX;
	data->rcdev.dev = dev;

	return devm_reset_controller_register(dev, &data->rcdev);
}

const struct of_device_id xr_usb_reset_of_match[] = {
	{ .compatible = "xring,o1,usb-reset", SET_XR_USB_RESET_O1_DATA() },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, xr_usb_reset_of_match);

struct platform_driver xr_usb_reset_driver = {
	.probe	= xr_usb_reset_probe,
	.driver	= {
		.name	= "xr_usb_reset",
		.of_match_table = xr_usb_reset_of_match,
	},
};
module_platform_driver(xr_usb_reset_driver);

MODULE_SOFTDEP("post: xring_usb");
MODULE_AUTHOR("Yu Chen <chenyu45@xiaomi.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Reset Driver For XRing USB Module");
