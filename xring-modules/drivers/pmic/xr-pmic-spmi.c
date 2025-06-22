// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024, The Linux Foundation. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spmi.h>
#include <linux/regmap.h>
#include <linux/of_platform.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/pmic/top_reg.h>
#include <soc/xring/xr-pmic-spmi.h>
#include "xsp_ffa.h"

#define SPMI_SEC_REG_MIN 0x2000
#define CLK_O3_DS_MAX 7

static const struct of_device_id pmic_spmi_id_table[] = {
	{ .compatible = "xring,pmic-spmi",},
	{ }
};

static const struct regmap_config spmi_regmap_config = {
	.reg_bits	= 16,
	.val_bits	= 8,
	.max_register	= 0xffff,
	.fast_io	= true,
};

static struct regmap *regmap;

static int clk_o3_ds_set(struct spmi_device *sdev, u32 ds)
{
	u32 reg_rd;
	u32 reg_wr;
	int ret;

	if (ds > CLK_O3_DS_MAX) {
		dev_err(&sdev->dev, "Invalid CLK_O3_DS %u\n", ds);
		return -EINVAL;
	}

	// O3 O4 drive strength
	ret = xr_pmic_reg_read(TOP_REG_INTF_XOCORE_CFG4, &reg_rd);
	if (ret) {
		dev_err(&sdev->dev, "read drive strength reg failed(%d)\n", ret);
		return -EIO;
	}
	reg_wr = (reg_rd & (~TOP_REG_INTF_XOCORE_CFG4_SW_INTF_CLK_O3_DRV_MASK)) |
				(ds << TOP_REG_INTF_XOCORE_CFG4_SW_INTF_CLK_O3_DRV_SHIFT);
	ret = xr_pmic_reg_write(TOP_REG_INTF_XOCORE_CFG4, reg_wr);
	if (ret) {
		dev_err(&sdev->dev, "write drive strength reg failed(%d)\n", ret);
		return -EIO;
	}
	dev_info(&sdev->dev, "set CLK_O3_DS=%d, reg_val=0x%x\n", ds, reg_wr);

	return 0;
}

static int pmic_init(struct spmi_device *sdev)
{
	struct device_node *np = sdev->dev.of_node;
	u32 o3_ds;
	int ret;

	/* optional param */
	ret = of_property_read_u32(np, "clk_o3_ds", &o3_ds);
	if (ret == 0) {
		ret = clk_o3_ds_set(sdev, o3_ds);
		if (ret)
			return ret;
	}

	return 0;
}

static int pmic_spmi_probe(struct spmi_device *sdev)
{
	regmap = devm_regmap_init_spmi_ext(sdev, &spmi_regmap_config);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	pmic_init(sdev);

	return devm_of_platform_populate(&sdev->dev);
}

static int reg_ffa_write(unsigned int reg, unsigned int val)
{
	int ret;
	struct xsp_ffa_msg ffa_msg;

	ffa_msg.fid = FID_SPMI_BURST_WRITE;
	ffa_msg.data0 = reg;
	ffa_msg.data1 = 1;
	ffa_msg.data2 = val;
	ffa_msg.data3 = 0;

	ret = xrsp_ffa_direct_message(&ffa_msg);
	if (ret) {
		pr_err("ffa write reg failed\n");
		return ret;
	}

	return ret;
}

static int reg_ffa_read(unsigned int reg, unsigned int *val)
{
	int ret;
	struct xsp_ffa_msg ffa_msg;

	ffa_msg.fid = FID_SPMI_BURST_READ;
	ffa_msg.data0 = reg;
	ffa_msg.data1 = 1;
	ffa_msg.data2 = 0;
	ffa_msg.data3 = 0;

	ret = xrsp_ffa_direct_message(&ffa_msg);
	if (ret) {
		pr_err("ffa read reg failed\n");
		return ret;
	}
	*val = ffa_msg.data2;

	return ret;
}

int xr_pmic_reg_read(unsigned int reg, unsigned int *val)
{
	int ret;

	if (val == NULL) {
		pr_err("val is null\n");
		return -EINVAL;
	}

	if (reg >= SPMI_SEC_REG_MIN) {
		ret = reg_ffa_read(reg, val);
		if (ret) {
			pr_err("pmic read reg failed\n");
			return -EINVAL;
		}
		return ret;
	}

	if (!regmap) {
		pr_err("regmap spmi failed\n");
		return -EINVAL;
	}
	return regmap_read(regmap, reg, val);
}
EXPORT_SYMBOL(xr_pmic_reg_read);

int xr_pmic_reg_write(unsigned int reg, unsigned int val)
{
	int ret;

	if (reg >= SPMI_SEC_REG_MIN) {
		ret = reg_ffa_write(reg, val);
		if (ret) {
			pr_err("pmic write reg failed\n");
			return -EINVAL;
		}
		return ret;
	}

	if (!regmap) {
		pr_err("regmap spmi failed\n");
		return -EINVAL;
	}
	return regmap_write(regmap, reg, val);
}
EXPORT_SYMBOL(xr_pmic_reg_write);

int xr_pmic_reg_bulk_read(unsigned int reg, void *val, size_t val_count)
{
	if (!regmap) {
		pr_err("regmap spmi failed\n");
		return -EINVAL;
	}
	return regmap_bulk_read(regmap, reg, val, val_count);
}
EXPORT_SYMBOL(xr_pmic_reg_bulk_read);

int xr_pmic_reg_bulk_write(unsigned int reg, const void *val, size_t val_count)
{
	if (!regmap) {
		pr_err("regmap spmi failed\n");
		return -EINVAL;
	}
	return regmap_bulk_write(regmap, reg, val, val_count);
}
EXPORT_SYMBOL(xr_pmic_reg_bulk_write);

static void pmic_spmi_remove(struct spmi_device *sdev)
{
	dev_info(&sdev->dev, "remove pmic_spmi\n");
}

MODULE_DEVICE_TABLE(of, pmic_spmi_id_table);

static struct spmi_driver xr_pmic_spmi_driver = {
	.probe = pmic_spmi_probe,
	.remove = pmic_spmi_remove,
	.driver = {
		.name = "xr-pmic-spmi",
		.of_match_table = pmic_spmi_id_table,
	},
};
module_spmi_driver(xr_pmic_spmi_driver);

MODULE_SOFTDEP("pre: xr-spmi-controller");
MODULE_DESCRIPTION("xr pmic spmi driver");
MODULE_LICENSE("GPL v2");
