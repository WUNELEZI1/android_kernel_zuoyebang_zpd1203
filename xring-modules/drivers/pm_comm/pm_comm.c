// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/of_platform.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/kstrtox.h>
#include <linux/spinlock.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <soc/xring/pm_comm.h>
#include <soc/xring/doze.h>
#include <soc/xring/xr_timestamp.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_pm_comm.h>
#include <dt-bindings/xring/platform-specific/pm/include/sys_pm_comm_plat.h>
#include <soc/xring/vote_mng.h>
#include <dt-bindings/xring/vote_mng_define.h>

#define BM_SHIFT 16
#define xr_pm_comm_print(s, fmt, args ...) \
	do { \
		if (s == NULL) \
			pr_debug(fmt, ##args); \
		else \
			seq_printf(s, fmt, ##args); \
	} while (0)

void __iomem *g_sign;
static unsigned int g_xctrl_vote = VOTE_MNG_ON;
static unsigned int g_lpctrl_vote = VOTE_MNG_ON;
struct vote_mng_ptr g_vote_mng_ptr;

unsigned long sys_state_reg_read(unsigned long addr)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_PM_INFO, addr, 0, 0, 0, 0, 0, 0, &res);
	return res.a1;
}
EXPORT_SYMBOL(sys_state_reg_read);

void sys_state_reg_write(unsigned long addr, unsigned long val)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_PM_WRITE, addr, val, 0, 0, 0, 0, 0, &res);
}
EXPORT_SYMBOL(sys_state_reg_write);

void sys_state_votemng_xctrl_shmem(int flag)
{
	int ret = 0;

	if (flag != PM_SR && flag != PM_DOZE) {
		pr_err("%s:%d invalid flag!\n", __func__, __LINE__);
		return;
	}

	if (flag == PM_SR)
		writel(BIT(VOTEMNG_SIGN_BIT) << BM_SHIFT |
			BIT(VOTEMNG_SIGN_BIT), g_sign);
	else
		writel(BIT(VOTEMNG_SIGN_BIT) << BM_SHIFT, g_sign);

	ret = vote_mng_vote_onoff(g_vote_mng_ptr.xctrl_shmem, 0, g_xctrl_vote);
	if (ret) {
		xr_pm_comm_print(NULL, "%s,%d, send xctrl failed ret=%d\n",
			      __func__, __LINE__, ret);
		return;
	}
	xr_pm_comm_print(NULL, "%s,%d, xctrl send info finish\n",
		      __func__, __LINE__);

	if (g_xctrl_vote == VOTE_MNG_ON) {
		g_xctrl_vote = VOTE_MNG_OFF | VOTE_MNG_OFF_SYNC;
		return;
	}

	g_xctrl_vote = VOTE_MNG_ON;
}
EXPORT_SYMBOL(sys_state_votemng_xctrl_shmem);

void sys_state_votemng_lpctrl_shmem(int flag)
{
	int ret = 0;

	if (flag == PM_SR)
		writel(BIT(VOTEMNG_SIGN_BIT) << BM_SHIFT |
			BIT(VOTEMNG_SIGN_BIT), g_sign);
	else
		writel(BIT(VOTEMNG_SIGN_BIT) << BM_SHIFT, g_sign);

	ret = vote_mng_vote_onoff(g_vote_mng_ptr.lpctrl_shmem, 0, g_lpctrl_vote);
	if (ret) {
		xr_pm_comm_print(NULL, "%s,%d, send lpctrl failed ret=%d\n",
			__func__, __LINE__, ret);
		return;
	}
	xr_pm_comm_print(NULL, "%s,%d, lpctrl send info finish\n",
		      __func__, __LINE__);

	if (g_lpctrl_vote == VOTE_MNG_ON) {
		g_lpctrl_vote = VOTE_MNG_OFF | VOTE_MNG_OFF_SYNC;
		return;
	}

	g_lpctrl_vote = VOTE_MNG_ON;
}
EXPORT_SYMBOL(sys_state_votemng_lpctrl_shmem);

static int pm_comm_probe(struct platform_device *plat_dev)
{
	struct device *dev = &plat_dev->dev;
	struct resource *mem = NULL;

	g_sign = devm_platform_get_and_ioremap_resource(plat_dev, 0, &mem);
	if (g_sign == NULL) {
		pr_err("%s:%d g_sign ioremap failed!\n", __func__, __LINE__);
		return -EINVAL;
	}

	g_vote_mng_ptr.xctrl_shmem = vote_mng_get(dev, "xctrl_cpu_doze_shmem");
	g_vote_mng_ptr.lpctrl_shmem = vote_mng_get(dev, "lpctrl_doze_shmem");
	if (IS_ERR_OR_NULL(g_vote_mng_ptr.xctrl_shmem) ||
	    IS_ERR_OR_NULL(g_vote_mng_ptr.lpctrl_shmem)) {
		dev_err(dev, "doze shmem get votemng failed\n");
		return -ENOMEM;
	}
	return 0;
}

static int pm_comm_remove(struct platform_device *pdev)
{
	struct device *dev = &(pdev->dev);

	vote_mng_put(dev, g_vote_mng_ptr.xctrl_shmem);
	vote_mng_put(dev, g_vote_mng_ptr.lpctrl_shmem);

	iounmap(g_sign);
	if (g_sign != NULL)
		g_sign = NULL;

	return 0;
}

static const struct of_device_id pm_comm_acpu_match_table[] = {
	{.compatible = "xring,pm_comm"},
	{}};

static struct platform_driver pm_comm_driver = {
	.driver = {
		.name = "pm_comm",
		.of_match_table = pm_comm_acpu_match_table,
	},
	.probe = pm_comm_probe,
	.remove = pm_comm_remove,
};

static int __init pm_comm_driver_init(void)
{
	int ret;

	ret = platform_driver_register(&pm_comm_driver);
	if (ret) {
		pr_err("platdrv register failed, %d\n", ret);
		return ret;
	}
	return 0;
}

static void __exit pm_comm_driver_exit(void)
{
	platform_driver_unregister(&pm_comm_driver);
}

module_init(pm_comm_driver_init);
module_exit(pm_comm_driver_exit);
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: xr_vote_mng");
