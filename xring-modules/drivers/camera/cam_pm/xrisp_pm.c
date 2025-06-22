// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "[XRISP_DRV][%14s] %s(%d): " fmt, "xrisp_pm", __func__, __LINE__

#include <linux/device.h>
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/kernel.h>
#include <linux/atomic.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <dt-bindings/xring/platform-specific/peri_crg.h>
#include <dt-bindings/xring/platform-specific/media2_ctrl.h>
#include "xrisp_address_map.h"
#include "xrisp_pm.h"
#include "xrisp_log.h"

static struct xrisp_pm_mgnt g_ctl_pm;

const char isp_rgltr_name[ISP_REGULATOR_MAX][RGLTR_NAME_LEN] = {
	"isp_top_fe_core0", "isp_top_fe_core1", "isp_top_fe_core2", "isp_top_be",
	"isp_top_pe"
};

int xrisp_be_ocm_link(void)
{
	int ret = 0;
	uint32_t rd_val = 0;
	void __iomem *peri = NULL;
	void __iomem *m2_ctrl = NULL;

	if (!(g_ctl_pm.isp_rgltrs[ISP_BE] &&
	      regulator_is_enabled(g_ctl_pm.isp_rgltrs[ISP_BE]))) {
		XRISP_PR_ERROR("isp_be_ocm link need isp_be power_on, link failed");
		return -EINVAL;
	}

	if (g_ctl_pm.ocm_link_count++ > 0) {
		XRISP_PR_INFO("isp_be_ocm alreadly link, not need to link again");
		return -EBUSY;
	}

	XRISP_PR_INFO("isp_be_ocm link start\n");

	peri = ioremap(XRISP_PERI_CRG_BASE, XRISP_PERI_CRG_SIZE);
	if (unlikely(peri == NULL)) {
		XRISP_PR_ERROR("peri reg iomap failed");
		return -ENOMEM;
	}
	m2_ctrl = ioremap(XRISP_M2_CTRL_BASE, XRISP_M2_CTRL_SIZE);
	if (unlikely(m2_ctrl == NULL)) {
		XRISP_PR_ERROR("m2_ctrl reg iomap failed");
		iounmap(peri);
		return -ENOMEM;
	}

	mutex_lock(&g_ctl_pm.ocm_lock);
	writel_io(peri, XRISP_PERI_CRG_BASE, PERI_CRG_CLKGT13_W1S,
		  REG_BIT(PERI_CRG_CLKGT13_W1S_GT_CLK_SYSBUS_DATA_MEDIA2_RS_CORE_SHIFT));

	writel_io(
		m2_ctrl, XRISP_M2_CTRL_BASE, MEDIA2_CTRL_MEDIA2_BUS_CTRL,
		BMRW_CLEAR(MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_NPU_IDLEREQ_SHIFT));

	xr_delay_us(50);

	rd_val = readl_io(m2_ctrl, XRISP_M2_CTRL_BASE, MEDIA2_CTRL_MEDIA2_BUS_STATUS) &
		 (REG_BIT(MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_NPU_IDLE_SHIFT) |
		  REG_BIT(MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_NPU_IDLEACK_SHIFT));
	if (rd_val != 0) {
		XRISP_PR_ERROR("media2_bus_npu status error, reg: 0x%x=0x%x, exp=0x%x",
		       XRISP_M2_CTRL_BASE + MEDIA2_CTRL_MEDIA2_BUS_STATUS, rd_val, 0);
		ret = -EINVAL;
	}

	writel_io(m2_ctrl, XRISP_M2_CTRL_BASE, MEDIA2_CTRL_ADB400_SLV_PW_CTRL,
		  BMRW_SET(MEDIA2_CTRL_ADB400_SLV_PW_CTRL_SC_ADB400_SLV_PWRQ_REQN_SHIFT));

	xr_delay_us(50);

	rd_val =
		readl_io(m2_ctrl, XRISP_M2_CTRL_BASE, MEDIA2_CTRL_ADB400_SLV_PW_STATUS) &
		(REG_BIT(
			MEDIA2_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACCEPTN_SHIFT));

	if (rd_val !=
	    REG_BIT(MEDIA2_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACCEPTN_SHIFT)) {
		XRISP_PR_ERROR("adb400_slv_pw_status error, reg: 0x%x = 0x%x, exp = 0x%x",
		       XRISP_M2_CTRL_BASE + MEDIA2_CTRL_ADB400_SLV_PW_STATUS, rd_val, 1);
		ret = -EINVAL;
	}

	if (ret)
		XRISP_PR_ERROR("isp_be_ocm link failed");
	else {
		g_ctl_pm.ocm_link_flag = true;
		XRISP_PR_INFO("isp_be_ocm link success\n");
	}
	mutex_unlock(&g_ctl_pm.ocm_lock);

	iounmap(m2_ctrl);
	iounmap(peri);
	return ret;
}

int __xrisp_be_ocm_unlink(void)
{
	int ret = 0;
	uint32_t rd_val = 0;
	void __iomem *peri = NULL;
	void __iomem *m2_ctrl = NULL;

	XRISP_PR_INFO("isp_be_ocm unlink start\n");

	peri = ioremap(XRISP_PERI_CRG_BASE, XRISP_PERI_CRG_SIZE);
	if (unlikely(peri == NULL)) {
		XRISP_PR_ERROR("peri reg iomap failed");
		return -ENOMEM;
	}
	m2_ctrl = ioremap(XRISP_M2_CTRL_BASE, XRISP_M2_CTRL_SIZE);
	if (unlikely(m2_ctrl == NULL)) {
		XRISP_PR_ERROR("m2_ctrl reg iomap failed");
		iounmap(peri);
		return -ENOMEM;
	}

	mutex_lock(&g_ctl_pm.ocm_lock);
	writel_io(m2_ctrl, XRISP_M2_CTRL_BASE, MEDIA2_CTRL_MEDIA2_BUS_CTRL,
		  BMRW_SET(MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_NPU_IDLEREQ_SHIFT));

	xr_delay_us(50);
	rd_val = readl_io(m2_ctrl, XRISP_M2_CTRL_BASE, MEDIA2_CTRL_MEDIA2_BUS_STATUS) &
		 (REG_BIT(MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_NPU_IDLE_SHIFT) |
		  REG_BIT(MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_NPU_IDLEACK_SHIFT));

	if (rd_val !=
	    (REG_BIT(MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_NPU_IDLE_SHIFT) |
	     REG_BIT(MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_NPU_IDLEACK_SHIFT))) {
		XRISP_PR_ERROR("media2_bus_npu status error, reg: 0x%x = 0x%x, exp = 0x%x",
		       XRISP_M2_CTRL_BASE + MEDIA2_CTRL_MEDIA2_BUS_STATUS, rd_val, 0b11);
		ret = -EINVAL;
	}

	writel_io(
		m2_ctrl, XRISP_M2_CTRL_BASE, MEDIA2_CTRL_ADB400_SLV_PW_CTRL,
		BMRW_CLEAR(MEDIA2_CTRL_ADB400_SLV_PW_CTRL_SC_ADB400_SLV_PWRQ_REQN_SHIFT));

	xr_delay_us(50);
	rd_val =
		readl_io(m2_ctrl, XRISP_M2_CTRL_BASE, MEDIA2_CTRL_ADB400_SLV_PW_STATUS) &
		REG_BIT(MEDIA2_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACCEPTN_SHIFT);
	if (rd_val != 0) {
		XRISP_PR_ERROR("adb400_slv_pw_status error, reg: 0x%x = 0x%x, exp = 0x%x",
		       XRISP_M2_CTRL_BASE + MEDIA2_CTRL_ADB400_SLV_PW_STATUS, rd_val, 0);
		ret = -EINVAL;
	}

	writel_io(peri, XRISP_PERI_CRG_BASE, PERI_CRG_CLKGT13_W1C,
		  REG_BIT(PERI_CRG_CLKGT13_W1C_GT_CLK_SYSBUS_DATA_MEDIA2_RS_CORE_SHIFT));
	if (ret)
		XRISP_PR_ERROR("isp_be_ocm unlink failed");
	else {
		g_ctl_pm.ocm_link_flag = false;
		g_ctl_pm.ocm_link_count = 0;
		XRISP_PR_INFO("isp_be_ocm unlink success\n");
	}
	mutex_unlock(&g_ctl_pm.ocm_lock);

	iounmap(m2_ctrl);
	iounmap(peri);


	return ret;
}

int xrisp_be_ocm_unlink(void)
{

	if (g_ctl_pm.ocm_link_count == 0) {
		XRISP_PR_DEBUG("isp_be_ocm alreadly unlink, not need to unlink again");
		return -EBUSY;
	}
	if (--g_ctl_pm.ocm_link_count > 0) {
		XRISP_PR_INFO("isp_be_ocm count (%d) > 0", g_ctl_pm.ocm_link_count);
		return -EBUSY;
	}

	return __xrisp_be_ocm_unlink();
}

int xrisp_be_ocm_unlink_reap(void)
{
	int ret = 0;

	if (g_ctl_pm.ocm_link_flag)
		ret = __xrisp_be_ocm_unlink();
	return ret;
}

int xrisp_store_pipergltr_set(u8 rgltr_set[], int rgltr_num)
{
	if (rgltr_num != ISP_REGULATOR_MAX) {
		XRISP_PR_ERROR("input rgltr_num not match, in=%d, exp=%d", rgltr_num,
		       ISP_REGULATOR_MAX);
		return -EINVAL;
	}

	mutex_lock(&g_ctl_pm.rgltr_lock);
	memcpy(g_ctl_pm.pipe_rgltr_on_set, rgltr_set, rgltr_num);
	mutex_unlock(&g_ctl_pm.rgltr_lock);

	XRISP_PR_DEBUG("store pipe_poweron_set success");
	return 0;
}

int xrisp_boot_pipe_rgltr_enable(void)
{
	u8 rgltr_set[ISP_REGULATOR_MAX];

	memcpy(rgltr_set, g_ctl_pm.pipe_rgltr_on_set, ISP_REGULATOR_MAX);
	xrisp_pipe_rgltr_enable(rgltr_set, ISP_REGULATOR_MAX);
	return 0;
}

static int xrisp_pm_rgltr_get(void)
{
	struct device *dev = g_ctl_pm.pm_dev;
	int rgltr_idx = 0;

	for (rgltr_idx = 0; rgltr_idx < ISP_REGULATOR_MAX; rgltr_idx++) {
		if (g_ctl_pm.isp_rgltrs[rgltr_idx] != NULL) {
			XRISP_PR_DEBUG("regulator %s already get, not need to get again",
				isp_rgltr_name[rgltr_idx]);
			continue;
		}

		g_ctl_pm.isp_rgltrs[rgltr_idx] =
			regulator_get(dev, isp_rgltr_name[rgltr_idx]);

		if (g_ctl_pm.isp_rgltrs[rgltr_idx] == NULL) {
			XRISP_PR_ERROR("regulator %s get error, into error process",
			       isp_rgltr_name[rgltr_idx]);
			goto rgltr_get_error;
		}
		XRISP_PR_DEBUG("regulator %s get success", isp_rgltr_name[rgltr_idx]);
	}

	XRISP_PR_DEBUG("xrisp all pipe regulator get finish");
	return 0;

rgltr_get_error:

	for (rgltr_idx -= 1; rgltr_idx >= 0; rgltr_idx--) {
		regulator_put(g_ctl_pm.isp_rgltrs[rgltr_idx]);
		XRISP_PR_DEBUG("regulator %s put success", isp_rgltr_name[rgltr_idx]);
		g_ctl_pm.isp_rgltrs[rgltr_idx] = NULL;
	}

	return -EINVAL;
}

static void xrisp_pm_rgltr_put(void)
{
	int rgltr_idx = 0;

	for (rgltr_idx = ISP_REGULATOR_MAX - 1; rgltr_idx >= 0; rgltr_idx--) {
		if (g_ctl_pm.isp_rgltrs[rgltr_idx] == NULL) {
			XRISP_PR_DEBUG("regulator %s already put, not need to put again",
				isp_rgltr_name[rgltr_idx]);
			continue;
		}

		while (atomic_read(&g_ctl_pm.rgltr_open_cnt[rgltr_idx]) > 0 &&
		       atomic_dec_return(&g_ctl_pm.rgltr_open_cnt[rgltr_idx]) >= 0)
			regulator_disable(g_ctl_pm.isp_rgltrs[rgltr_idx]);

		regulator_put(g_ctl_pm.isp_rgltrs[rgltr_idx]);
		g_ctl_pm.isp_rgltrs[rgltr_idx] = NULL;
		XRISP_PR_DEBUG("regulator %s put success", isp_rgltr_name[rgltr_idx]);
	}

	XRISP_PR_DEBUG("xrisp all regulator put finish");
}

int xrisp_pipe_rgltr_enable(u8 rgltr_on_set[], int rgltr_num)
{
	int i;

	if (rgltr_num != ISP_REGULATOR_MAX) {
		XRISP_PR_ERROR("input regulator num not match, in=%d, exp=%d", rgltr_num,
		       ISP_REGULATOR_MAX);
		return -EINVAL;
	}

	mutex_lock(&g_ctl_pm.rgltr_lock);
	for (i = 0; i < ISP_REGULATOR_MAX; i++) {
		if (g_ctl_pm.isp_rgltrs[i] == NULL) {
			XRISP_PR_ERROR("regulator %s is NULL, enable failed", isp_rgltr_name[i]);
			continue;
		}

		if (rgltr_on_set[i] == true) {
			int open_cnt;

			if (regulator_enable(g_ctl_pm.isp_rgltrs[i])) {
				XRISP_PR_ERROR("regulator %s enable failed, into error process",
				       isp_rgltr_name[i]);
				goto rgltr_enable_err;
			}

			open_cnt = atomic_inc_return(&g_ctl_pm.rgltr_open_cnt[i]);

			XRISP_PR_DEBUG("regulator %s enable success, open_cnt=%d",
				isp_rgltr_name[i], open_cnt);
		}
	}
	mutex_unlock(&g_ctl_pm.rgltr_lock);
	XRISP_PR_DEBUG("xrisp pipe regulator enable set success");
	return 0;

rgltr_enable_err:

	for (i -= 1; i >= ISP_FE_CORE0; i--) {
		if (rgltr_on_set[i] == true) {
			int open_cnt;

			regulator_disable(g_ctl_pm.isp_rgltrs[i]);
			open_cnt = atomic_dec_return(&g_ctl_pm.rgltr_open_cnt[i]);

			XRISP_PR_DEBUG("regulator %s disable success, open_cnt=%d",
				isp_rgltr_name[i], open_cnt);
		}
	}
	mutex_unlock(&g_ctl_pm.rgltr_lock);
	XRISP_PR_DEBUG("xrisp pipe regulator enable set failed");
	return -EINVAL;
}

int xrisp_pipe_rgltr_disable(u8 rgltr_off_set[], int rgltr_num)
{
	int i;

	if (rgltr_num != ISP_REGULATOR_MAX) {
		XRISP_PR_ERROR("input rgltr num not match, in=%d, exp=%d", rgltr_num,
		       ISP_REGULATOR_MAX);
		return -EINVAL;
	}

	mutex_lock(&g_ctl_pm.rgltr_lock);
	for (i = ISP_REGULATOR_MAX - 1; i >= 0; i--) {
		if (rgltr_off_set[i] == true) {
			if (g_ctl_pm.isp_rgltrs[i] == NULL) {
				XRISP_PR_ERROR("regulator %s is NULL, disable failed",
				       isp_rgltr_name[i]);
				continue;
			}

			if (atomic_read(&g_ctl_pm.rgltr_open_cnt[i]) > 0) {
				int open_cnt;

				regulator_disable(g_ctl_pm.isp_rgltrs[i]);
				open_cnt = atomic_dec_return(&g_ctl_pm.rgltr_open_cnt[i]);
				XRISP_PR_DEBUG("regulator %s disable success, open_cnt=%d",
					isp_rgltr_name[i], open_cnt);
			} else
				XRISP_PR_ERROR("regulator %s disable failed, open_cnt=%d",
				       isp_rgltr_name[i],
				       atomic_read(&g_ctl_pm.rgltr_open_cnt[i]));
		}
	}
	mutex_unlock(&g_ctl_pm.rgltr_lock);
	XRISP_PR_DEBUG("xrisp pipe regulator disable set finish");
	return 0;
}

void xrisp_pipe_rgltr_release(void)
{
	int i;

	mutex_lock(&g_ctl_pm.rgltr_lock);
	for (i = 0; i < ISP_REGULATOR_MAX; i++) {
		if (g_ctl_pm.isp_rgltrs[i] == NULL) {
			XRISP_PR_ERROR("regulator %s is NULL, release failed", isp_rgltr_name[i]);
			continue;
		}

		while (atomic_read(&g_ctl_pm.rgltr_open_cnt[i]) > 0 &&
		       atomic_dec_return(&g_ctl_pm.rgltr_open_cnt[i]) >= 0)
			regulator_disable(g_ctl_pm.isp_rgltrs[i]);

		XRISP_PR_DEBUG("regulator %s release success, open_cnt=%d", isp_rgltr_name[i],
			atomic_read(&g_ctl_pm.rgltr_open_cnt[i]));
	}
	mutex_unlock(&g_ctl_pm.rgltr_lock);
	XRISP_PR_DEBUG("xrisp pipe regulator release success");
}

int xrisp_get_rgltr_open_cnt(int type)
{
	if (type < ISP_FE_CORE0 || type >= ISP_REGULATOR_MAX ||
	    g_ctl_pm.isp_rgltrs[type] == NULL)
		return -1;
	return regulator_is_enabled(g_ctl_pm.isp_rgltrs[type]);
}

int xring_regulator_probe(struct platform_device *pdev)
{
	int i = 0;

	XRISP_PR_DEBUG("xrisp pm probe start\n");

	g_ctl_pm.pm_dev = &pdev->dev;
	g_ctl_pm.ocm_link_count = 0;
	g_ctl_pm.ocm_link_flag = false;
	mutex_init(&g_ctl_pm.rgltr_lock);
	mutex_init(&g_ctl_pm.ocm_lock);
	for (i = 0; i < ISP_REGULATOR_MAX; i++) {
		atomic_set(&g_ctl_pm.rgltr_open_cnt[i], 0);
		g_ctl_pm.isp_rgltrs[i] = NULL;
	}

	if (xrisp_pm_rgltr_get()) {
		XRISP_PR_ERROR("xrisp pm probe failed\n");
		return -EINVAL;
	}

	XRISP_PR_DEBUG("xrisp pm probe finish\n");

	return 0;
}

int xring_regulator_remove(struct platform_device *pdev)
{
	xrisp_pm_rgltr_put();

	XRISP_PR_DEBUG("xrisp pm remove finish\n");
	return 0;
}

static const struct of_device_id xring_of_match_tbl[] = {
	{
		.compatible = "xring,regulator_isp",
	},
	{ /* end */ }
};

static struct platform_driver xring_pm_driver = {
	.driver = {
		.name = "xrisp_pm_module",
		.owner = THIS_MODULE,
		.of_match_table = xring_of_match_tbl,
	},
	.probe = xring_regulator_probe,
	.remove = xring_regulator_remove,
};

int xrisp_regulator_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&xring_pm_driver);
	if (ret)
		XRISP_PR_ERROR("Failed to register xrisp_pm driver: %d", ret);

	XRISP_PR_INFO("xrisp pm init finish\n");

	return ret;
}

void xrisp_regulator_exit(void)
{
	platform_driver_unregister(&xring_pm_driver);
	XRISP_PR_INFO("xrisp pm exit finish\n");
}

MODULE_DESCRIPTION("Xring ISP Regulator Driver ");
MODULE_LICENSE("GPL v2");
