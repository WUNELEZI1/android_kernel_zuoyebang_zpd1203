// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Xring, Inc. All rights reserved.
 */
#include <linux/acpi.h>
#include <linux/time.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/phy/phy.h>
#include <linux/gpio/consumer.h>
#include <linux/reset-controller.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/errno.h>
#include <ufs/ufshcd.h>
#include <ufs_sys_ctrl.h>
#include "ufs-xring.h"

void ufs_xring_mphy_clk_cfg_fpga(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	ufs_sctrl_writel(host, 0xFF, 0xA8);

	/* ufs host controller CG tick config */
	/* TBD,[31:16]for TAG0509, [15:0] for TAG0529 */
	ufs_sctrl_rmwl(host, GENMASK(15, 0), 0x13, UFS_SYS_CTRL_UFS_MISC_CTRL);
	dev_dbg(hba->dev, "%d: CTRL(0x%x), val = 0x%x\n",
			__LINE__, UFS_SYS_CTRL_UFS_MISC_CTRL,
			ufs_sctrl_readl(host, UFS_SYS_CTRL_UFS_MISC_CTRL));
}

int ufs_xring_fpga_ioremap(struct ufs_hba *hba)
{
	int ret = 0;
	struct resource *res;
	struct device *dev = hba->dev;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	struct platform_device *pdev = to_platform_device(dev);

	/* Setup the UFS_MPHY ioremap*/
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ufs_mphy_mem");
	if (!res) {
		dev_warn(dev, "UFS_MPHY registers not found\n");
		ret = -EIO;
		goto out;
	}

	host->ufs_mphy_mmio = devm_ioremap_resource(dev, res);
	if (IS_ERR(host->ufs_mphy_mmio)) {
		ret = PTR_ERR(host->ufs_mphy_mmio);
		dev_err(dev, "Failed to map UFS_MPHY registers; ret=%d\n", ret);
		goto out;
	}

out:
	return ret;
}
