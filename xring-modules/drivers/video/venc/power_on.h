/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef POWER_ON_H
#define POWER_ON_H

#include <linux/delay.h>
#include <asm/io.h>
#include "venc_log.h"

static int media2_subsys_poweron(void)
{
	int ret = 0;
	u32 val;
	void __iomem *lpctrl_ft = ioremap(0xE5808000, 0x1000);

	if (!lpctrl_ft) {
		venc_pm_klog(LOGLVL_ERROR, "Can't remap lpctrl_ft\n");
		ret = -EINVAL;
		goto err1;
	}

	void __iomem *peri_crg = ioremap(0xEC001000, 0x1000);

	if (!peri_crg) {
		venc_pm_klog(LOGLVL_ERROR, "Can't remap peri_crg\n");
		ret = -EINVAL;
		goto err2;
	}

	void __iomem *pctrl = ioremap(0xEC002000, 0x2000);

	if (!pctrl) {
		venc_pm_klog(LOGLVL_ERROR, "Can't remap pctrl\n");
		ret = -EINVAL;
		goto err3;
	}

	void __iomem *media2_crg = ioremap(0xe5609000, 0x1000);

	if (!media2_crg) {
		venc_pm_klog(LOGLVL_ERROR, "Can't remap media2_crg\n");
		ret = -EINVAL;
		goto err4;
	}

	void __iomem *lpis_actrl = ioremap(0xe1508000, 0x1000);

	if (!lpis_actrl) {
		venc_pm_klog(LOGLVL_ERROR, "Can't remap lpis_actrl\n");
		ret = -EINVAL;
		goto err5;
	}

	//media2_ctrl
	void __iomem *media2_ctrl = ioremap(0xE5800000, 0x8000);

	if (!media2_ctrl) {
		venc_pm_klog(LOGLVL_ERROR, "Can't remap media2_ctrl\n");
		ret = -EINVAL;
		goto err6;
	}


	//open M2 mtcmos
	writel(0x01000100, lpctrl_ft + 0x0);
	usleep_range(1000, 1500);

	//memory exit shutdown
	writel(0xc0000000, lpctrl_ft + 0x24);
	usleep_range(1000, 1500);
	writel(0x20000000, lpctrl_ft + 0x24);
	usleep_range(1000, 1500);

	writel(0x01800000, lpctrl_ft + 0x20);
	usleep_range(1000, 1500);
	writel(0x00400000, lpctrl_ft + 0x20);
	usleep_range(1000, 1500);

	writel(0x01800000, lpctrl_ft + 0x24);
	usleep_range(1000, 1500);
	writel(0x00400000, lpctrl_ft + 0x24);
	usleep_range(1000, 1500);

	//peri_crg reset
	writel(0x00008000, peri_crg + 0x8d0);
	writel(0x00010000, peri_crg + 0x860);
	writel(0x00000010, peri_crg + 0x860);
	writel(0x00000008, peri_crg + 0x860);
	writel(0x00000080, peri_crg + 0x860);
	writel(0x00000004, peri_crg + 0x860);
	writel(0x00000002, peri_crg + 0x840);
	writel(0x02000000, peri_crg + 0x850);
	writel(0x00020000, peri_crg + 0x830);
	writel(0x00020000, peri_crg + 0x850);
	writel(0x00200000, media2_crg + 0x230);
	writel(0x00000040, media2_crg + 0x10);
	writel(0x00001000, media2_crg + 0x10);
	writel(0x08000080, media2_crg + 0x150);
	writel(0x00000004, media2_crg + 0x230);
	writel(0x00000004, media2_crg + 0x10);
	writel(0x00004000, media2_crg + 0x4);
	writel(0x00000002, media2_crg + 0x230);
	writel(0x00800080, media2_crg + 0x160);
	writel(0x00800080, media2_crg + 0x170);
	writel(0x00000400, media2_crg + 0x230);
	writel(0x00000100, media2_crg + 0x230);
	writel(0x02000000, media2_crg + 0x230);
	writel(0x00000400, media2_crg + 0x10);
	usleep_range(1000, 1500);

	writel(0x00200000, media2_crg + 0x234);
	writel(0x00000040, media2_crg + 0x14);
	writel(0x00001000, media2_crg + 0x14);
	writel(0x00000004, media2_crg + 0x234);
	writel(0x00000004, media2_crg + 0x14);
	writel(0x00004000, media2_crg + 0x4);
	writel(0x00000002, media2_crg + 0x234);
	writel(0x00800080, media2_crg + 0x160);
	writel(0x00800080, media2_crg + 0x170);
	writel(0x00000400, media2_crg + 0x234);
	writel(0x00000100, media2_crg + 0x234);
	writel(0x02000000, media2_crg + 0x234);
	writel(0x00000400, media2_crg + 0x14);
	usleep_range(1000, 1500);

	//pcrtl
	writel(0x00010000, pctrl + 0x808);
	usleep_range(1000, 1500);

	//peri_crg reset
	writel(0x00000020, peri_crg + 0x8d0);
	// media2_crg reset
	writel(0x00004000, media2_crg + 0x270);
	writel(0x00002000, media2_crg + 0x270);
	writel(0x00001000, media2_crg + 0x270);
	writel(0x00000040, media2_crg + 0x90);
	writel(0x00000080, media2_crg + 0x90);
	writel(0x00000100, media2_crg + 0x90);
	writel(0xffffffff, media2_crg + 0x270);
	// media2 open clock
	writel(0x00200000, media2_crg + 0x230);
	writel(0x00000040, media2_crg + 0x010);
	writel(0x00001000, media2_crg + 0x010);
	writel(0x00000004, media2_crg + 0x230);
	writel(0x00000004, media2_crg + 0x010);
	writel(0x00004000, media2_crg + 0x004);
	writel(0x00000002, media2_crg + 0x230);
	writel(0x02000000, media2_crg + 0x230);
	writel(0x00001000, media2_crg + 0x230);
	writel(0x00000001, media2_crg + 0x230);
	writel(0x00080000, media2_crg + 0x230);
	writel(0x00000400, media2_crg + 0x010);
	//lpis_actrl
	writel(0x00000000, lpis_actrl + 0x4a8);
	usleep_range(1000, 1500);

	// media2_ctrl
	writel(0x00060000, media2_ctrl + 0x400);
	usleep_range(1000, 1500);
	writel(0x0e000000, media2_ctrl + 0x480);
	usleep_range(1000, 1500);

	// venc power on
	writel(0x0000003f, media2_crg + 0x90);

err6:
	iounmap(media2_ctrl);
err5:
	iounmap(lpis_actrl);
err4:
	iounmap(media2_crg);
err3:
	iounmap(pctrl);
err2:
	iounmap(peri_crg);
err1:
	iounmap(lpctrl_ft);
	return ret;
}

static int media2_smmu_tcu_tbu_power_on(void)
{
	int ret = 0;
	u32 val = 0;
	u32 cnt = 0;
	void __iomem *media2_smmu_ctrl = ioremap(0xe5600000, 0x8000);

	if (!media2_smmu_ctrl) {
		venc_pm_klog(LOGLVL_ERROR, "Can't remap media2_smmu_ctrl\n");
		return -EINVAL;
		goto err0;
	}

	//tcu
	writel(0x00010000, media2_smmu_ctrl + 0x8);

	usleep_range(1000, 1500);
	val = readl(media2_smmu_ctrl + 0xC);
	if (!((val >> 7) & 1)) {
		venc_pm_klog(LOGLVL_ERROR, "media2_smmu_ctrl 0xC: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err0;
	}
	writel(0x00000001, media2_smmu_ctrl + 0x10);
	usleep_range(1000, 1500);
	val = readl(media2_smmu_ctrl + 0x14);
	if (!((val >> 2) & 1)) {
		venc_pm_klog(LOGLVL_ERROR, "media2_smmu_ctrl 0x14: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err0;
	}
	writel(0x00020100, media2_smmu_ctrl + 0x8);

	//tbu4
	writel(0x00010000, media2_smmu_ctrl + 0x110C);

	usleep_range(1000, 1500);
	val = readl(media2_smmu_ctrl + 0x1110);
	if (!((val >> 7) & 1)) {
		venc_pm_klog(LOGLVL_ERROR, "media2_smmu_ctrl 0x1110: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err0;
	}
	writel(0x00000001, media2_smmu_ctrl + 0x1114);
	usleep_range(1000, 1500);
	val = readl(media2_smmu_ctrl + 0x1118);
	if (!((val >> 2) & 1)) {
		venc_pm_klog(LOGLVL_ERROR, "media2_smmu_ctrl 0x1118: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err0;
	}
	writel(0x00020100, media2_smmu_ctrl + 0x110C);

err0:
	iounmap(media2_smmu_ctrl);
	return ret;
}

#endif /* POWER_ON_H */
