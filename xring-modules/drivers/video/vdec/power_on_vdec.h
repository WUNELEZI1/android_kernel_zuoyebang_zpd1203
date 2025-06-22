/* SPDX-License-Identifier: GPL-2.0 */
/**
 * vpu power_on for fpga test
 *
 * media1 power on
 * tcu tbu power on
 */
#ifndef __MEDIA1_POWER_ON__
#define __MEDIA1_POWER_ON__
#include <linux/delay.h>
#include "vdec_log.h"

static int media1_subsys_poweron(void)
{
	int ret = 0;
	u32 val;
	void __iomem *media1_lpctrl_ft;
	void __iomem *media1_crg;
	void __iomem *pctrl;
	void __iomem *lpis_actrl;
	void __iomem *peri_crg;

	peri_crg = ioremap(0xEC001000, 0x1000);
	if (!peri_crg) {
		vdec_pm_klog(LOGLVL_ERROR, "Can't remap peri_crg\n");
		return -EINVAL;
	}

	media1_lpctrl_ft = ioremap(0xE7C05000, 0x1000);
	if (!media1_lpctrl_ft) {
		vdec_pm_klog(LOGLVL_ERROR, "Can't remap media1_lpctrl_ft\n");
		ret = -EINVAL;
		goto err1;
	}

	media1_crg = ioremap(0xE7A00000, 0x1000);
	if (!media1_crg) {
		vdec_pm_klog(LOGLVL_ERROR, "Can't remap media1_crg\n");
		ret = -EINVAL;
		goto err2;
	}

	pctrl = ioremap(0xEC002000, 0x2000);
	if (!pctrl) {
		vdec_pm_klog(LOGLVL_ERROR, "Can't remap pctrl\n");
		ret = -EINVAL;
		goto err3;
	}

	lpis_actrl = ioremap(0xE1508000, 0x1000);
	if (!lpis_actrl) {
		vdec_pm_klog(LOGLVL_ERROR, "Can't remap lpis_actrl\n");
		ret = -EINVAL;
		goto err4;
	}

	//peri_crg reset
	writel(0x40010000, peri_crg + 0x8d0);
	//open peri_crg clock
	writel(0x08000000, peri_crg + 0x840);
	writel(0x01000000, peri_crg + 0x850);
	//memory exit shutdown
	writel(0x6C000000, media1_lpctrl_ft + 0x20);
	usleep_range(1000, 1500);
	writel(0x12000000, media1_lpctrl_ft + 0x20);
	usleep_range(1000, 1500);
	val = readl(media1_lpctrl_ft + 0x40);
	if ((((val >> 8) & 1) && (val >> 7) & 1)) {
		vdec_pm_klog(LOGLVL_ERROR, "media1_lpctrl_ft mem_lpstatus0: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err;
	}
	//peri_crg reset
	writel(0x00010040, peri_crg + 0x8d0);
	//open peri_crg clock
	writel(0x10000000, peri_crg + 0x840);
	writel(0x00020000, peri_crg + 0x850);
	writel(0x10000000, peri_crg + 0x870);
	writel(0x00070005, peri_crg + 0x80);
	// media1_crg open clock
	writel(0x0007FBFF, media1_crg + 0x20);
	writel(0x00682101, media1_crg + 0x40);
	writel(0x0000005A, media1_crg + 0x60);
	usleep_range(1000, 1500);
	// media1_crg close clock
	writel(0x0007FBFF, media1_crg + 0x24);
	writel(0x00682101, media1_crg + 0x44);
	writel(0x0000005A, media1_crg + 0x64);
	usleep_range(1000, 1500);
	writel(0x00010000, pctrl + 0x804);
	usleep_range(1000, 1500);
	val = readl(lpis_actrl + 0x714);
	// if (!((val >> 17) & 1)) {
	if (0) {
		vdec_pm_klog(LOGLVL_ERROR, "lpis_actrl mrc_ack_1: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err;
	}
	// media1_crg reset
	writel(0xDFF0FFF2, media1_crg + 0x80);
	writel(0x00007FF0, media1_crg + 0x90);
	// media1 open clock
	writel(0x0007FBFF, media1_crg + 0x20);
	writel(0x00682101, media1_crg + 0x40);
	writel(0x0000005A, media1_crg + 0x60);
	// Bus interface exits low power state and completes chain building
	writel(0x00000000, lpis_actrl + 0x4A8);
	// media1 readback
	val = readl(lpis_actrl + 0x4AC);
	if ((((val >> 1) & 1) && (val >> 4) & 1)) {
		vdec_pm_klog(LOGLVL_ERROR, "lpis_actrl main_bus_pdom_idle: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err;
	}
	// vdec power on
	writel(0xdf00002f, media1_crg + 0x80);

err:
	iounmap(lpis_actrl);
err4:
	iounmap(pctrl);
err3:
	iounmap(media1_crg);
err2:
	iounmap(media1_lpctrl_ft);
err1:
	iounmap(peri_crg);
	return ret;
}

static int smmu_tcu_tbu_power_on(void)
{
	int ret = 0;
	u32 val;
	void __iomem *media1_smmu_ctrl = ioremap(0xE7A04000, 0x4000);

	if (!media1_smmu_ctrl) {
		vdec_pm_klog(LOGLVL_ERROR, "Can't remap media1_smmu_ctrl\n");
		return -EINVAL;
	}
	//tcu
	writel(0x00010000, media1_smmu_ctrl + 0x8);
	usleep_range(1000, 1500);
	val = readl(media1_smmu_ctrl + 0xC);
	if (!((val >> 7) & 1)) {
		vdec_pm_klog(LOGLVL_ERROR, "media1_smmu_ctrl 0xC: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err;
	}
	writel(0x00000001, media1_smmu_ctrl + 0x10);
	usleep_range(1000, 1500);
	val = readl(media1_smmu_ctrl + 0x14);
	if (!((val >> 2) & 1)) {
		vdec_pm_klog(LOGLVL_ERROR, "media1_smmu_ctrl 0x14: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err;
	}
	writel(0x00020100, media1_smmu_ctrl + 0x8);
	//tbu
	writel(0x00010000, media1_smmu_ctrl + 0x10C);
	usleep_range(1000, 1500);
	val = readl(media1_smmu_ctrl + 0x110);
	if (!((val >> 7) & 1)) {
		vdec_pm_klog(LOGLVL_ERROR, "media1_smmu_ctrl 0x110: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err;
	}
	writel(0x00000001, media1_smmu_ctrl + 0x114);
	usleep_range(1000, 1500);
	val = readl(media1_smmu_ctrl + 0x118);
	if (!((val >> 2) & 1)) {
		vdec_pm_klog(LOGLVL_ERROR, "media1_smmu_ctrl 0x118: %d is not expected value\n", val);
		ret = -EINVAL;
		goto err;
	}
	writel(0x00020100, media1_smmu_ctrl + 0x10C);

err:
	iounmap(media1_smmu_ctrl);
	return ret;
}

static int vdec_test(void)
{
	int ret = 0;
	u32 val;
	void __iomem *vdec_base = ioremap(0xE7400000, 0x100000);

	if (!vdec_base) {
		vdec_pm_klog(LOGLVL_ERROR, "Can't remap media1_smmu_ctrl\n");
		return -EINVAL;
	}

	val = readl(vdec_base + 0x0);
	vdec_pm_klog(LOGLVL_ERROR, "vdec_id =  %d\n", val);

	return ret;
}

#endif
