// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "[XRISP_DRV][%14s] %s(%d): " fmt, "csi", __func__, __LINE__

#include <linux/io.h>
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
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>
#include <dt-bindings/xring/platform-specific/pctrl.h>
#include "xrisp_address_map.h"
#include "xrisp_pm.h"
#include "xrisp_log.h"

struct rgltr_s *p_csi_rgltr;
enum xrisp_csi_reg_type {
	REG_LPIS_ACTRL,
	REG_PERI_CRG,
	REG_PCTRL,
	REG_CSI_MAP_MAX,
};

static struct rgltr_reg csi_reg[REG_CSI_MAP_MAX] = {
	{ NULL, XRISP_LPIS_ACTRL_BASE, XRISP_LPIS_ACTRL_SIZE, "LPIS_ACTRL" },
	{ NULL, XRISP_PERI_CRG_BASE, XRISP_PERI_CRG_SIZE, "PERI_CRG" },
	{ NULL, XRISP_PCTRL_BASE, XRISP_PCTRL_SIZE, "PCTRL" },
};

static int xring_csi_is_enabled(struct regulator_dev *dev)
{
	if (p_csi_rgltr) {
		XRISP_PR_INFO("regulator %s is_enable is %d\n", "isp_csi",
			atomic_read(&p_csi_rgltr->rgltr_cnt));
		return atomic_read(&p_csi_rgltr->rgltr_cnt);
	}
	return -EINVAL;
}

int xring_isp_csi_enabled(struct regulator_dev *dev)
{
	int ret = 0;

	if (atomic_read(&p_csi_rgltr->rgltr_cnt) >= 1) {
		atomic_inc(&p_csi_rgltr->rgltr_cnt);
		XRISP_PR_INFO("isp_csi already powered on, open cnt = %d\n",
			atomic_read(&p_csi_rgltr->rgltr_cnt));
		return 0;
	}

	//memory shutdown
	writel_io(csi_reg[REG_PCTRL].va, csi_reg[REG_PCTRL].base, PCTRL_MEM_LGROUP2,
		  BMRW_CLEAR(PCTRL_MEM_LGROUP2_SC_CSI_MEM_SD_SHIFT));

	xr_delay_us(100);
	if (read_field_cmp(csi_reg[REG_PCTRL].va, csi_reg[REG_PCTRL].base, 0,
			   PCTRL_MEM_LPSTATUS2, 0, 1)) {
		XRISP_PR_ERROR("isp_csi mem_lpstatus2 error, enable failed");
		ret = -EINVAL;
	}

	//CRG set	SCR reg
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base, PERI_CRG_RST0_W1S,
		  REG_BIT(PERI_CRG_RST0_W1S_IP_RST_CSI_CRG_N_SHIFT));
	//ip open clock	SCR reg
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1S, REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_CSI_SYS_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1S,
		  REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_SYSBUS_CSI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1S,
		  REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_CFGBUS_CSI_SHIFT));

	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_PERI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_CORE_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_PERI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_CORE_SHIFT));
	xr_delay_us(1);

	//ip close clock
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1C, REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_CSI_SYS_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1C,
		  REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_SYSBUS_CSI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1C,
		  REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_CFGBUS_CSI_SHIFT));

	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1C,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_PERI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1C,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_CORE_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1C,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_PERI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1C,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_CORE_SHIFT));

	xr_delay_us(1);

	//IP unclamp
	writel_io(csi_reg[REG_PCTRL].va, csi_reg[REG_PCTRL].base, PCTRL_ISOLATION_GROUP3,
		  BMRW_CLEAR(PCTRL_ISOLATION_GROUP3_SC_CSI_ISO_EN_SHIFT));
	xr_delay_us(100);

	//mrc_ack
	if (read_field_cmp(csi_reg[REG_LPIS_ACTRL].va, csi_reg[REG_LPIS_ACTRL].base, 1,
			   LPIS_ACTRL_MRC_ACK_1, 14, 1)) {
		XRISP_PR_ERROR("isp_csi mrc_ack error, enable failed");
		ret = -EINVAL;
	}

	//ip dereset
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base, PERI_CRG_RST0_W1S,
		  REG_BIT(PERI_CRG_RST0_W1S_IP_RST_CSI_N_SHIFT));
	//ip open clock
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1S, REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_CSI_SYS_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1S,
		  REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_SYSBUS_CSI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1S,
		  REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_CFGBUS_CSI_SHIFT));

	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_PERI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_CORE_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_PERI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_CORE_SHIFT));

	//bus link
	writel_io(
		csi_reg[REG_LPIS_ACTRL].va, csi_reg[REG_LPIS_ACTRL].base,
		LPIS_ACTRL_SYS1_BUS_PDOM_REQ,
		BMRW_CLEAR(LPIS_ACTRL_SYS1_BUS_PDOM_REQ_SYS1_BUS_PDOM_CSI_IDLEREQ_SHIFT));

	xr_delay_us(50);
	if (read_field_cmp(csi_reg[REG_LPIS_ACTRL].va, csi_reg[REG_LPIS_ACTRL].base, 0,
			   LPIS_ACTRL_SYS1_BUS_PDOM_IDLE, 14, 1)) {
		XRISP_PR_ERROR("isp_csi sys1_bus_pdom_idle error, enable failed");
		ret = -EINVAL;
	}

	//csi sysbus clock close
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1C,
		  REG_BIT(PERI_CRG_CLKGT0_W1C_GT_CLK_SYSBUS_CSI_SHIFT));

	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1C,
		  REG_BIT(PERI_CRG_CLKGT1_W1C_GT_CLK_SYSBUS_CSI_RS_PERI_SHIFT));

	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1C,
		  REG_BIT(PERI_CRG_CLKGT1_W1C_GT_CLK_SYSBUS_CSI_RS_CORE_SHIFT));

	atomic_set(&p_csi_rgltr->rgltr_cnt, 1);
	if (ret)
		XRISP_PR_ERROR("isp_csi power on failed");
	else
		XRISP_PR_INFO("isp_csi power on success\n");
	return 0;
}
EXPORT_SYMBOL(xring_isp_csi_enabled);

int xring_isp_csi_disabled(struct regulator_dev *dev)
{
	int ret = 0;

	if (atomic_read(&p_csi_rgltr->rgltr_cnt) == 0) {
		XRISP_PR_INFO("isp_csi already powered off\n");
		return 0;
	} else if (atomic_read(&p_csi_rgltr->rgltr_cnt) > 1) {
		atomic_dec(&p_csi_rgltr->rgltr_cnt);
		XRISP_PR_INFO("isp_csi cannot power off, open cnt = %d\n",
			atomic_read(&p_csi_rgltr->rgltr_cnt));
		return 0;
	} else if (atomic_read(&p_csi_rgltr->rgltr_cnt) == 1) {
		XRISP_PR_INFO("isp_csi doing power off\n");
	} else {
		XRISP_PR_ERROR("isp_csi rgltr_cnt error, cnt = %d\n",
		       atomic_read(&p_csi_rgltr->rgltr_cnt));
		return -EINVAL;
	}

	//csi sysbus clock open
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1S,
		  REG_BIT(PERI_CRG_CLKGT0_W1C_GT_CLK_SYSBUS_CSI_SHIFT));

	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1C_GT_CLK_SYSBUS_CSI_RS_PERI_SHIFT));

	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1C_GT_CLK_SYSBUS_CSI_RS_CORE_SHIFT));

	//bus unlink
	writel_io(csi_reg[REG_LPIS_ACTRL].va, csi_reg[REG_LPIS_ACTRL].base,
		  LPIS_ACTRL_SYS1_BUS_PDOM_REQ,
		  BMRW_SET(LPIS_ACTRL_SYS1_BUS_PDOM_REQ_SYS1_BUS_PDOM_CSI_IDLEREQ_SHIFT));
	xr_delay_us(50);
	if (read_field_cmp(csi_reg[REG_LPIS_ACTRL].va, csi_reg[REG_LPIS_ACTRL].base, 1,
			   LPIS_ACTRL_SYS1_BUS_PDOM_IDLE, 6, 1)) {
		XRISP_PR_ERROR("isp_csi sys1_bus_pdom_idle error, enable failed");
		ret = -EINVAL;
	}

	if (read_field_cmp(csi_reg[REG_LPIS_ACTRL].va, csi_reg[REG_LPIS_ACTRL].base, 1,
			   LPIS_ACTRL_SYS1_BUS_PDOM_IDLE, 14, 1)) {
		XRISP_PR_ERROR("isp_csi sys1_bus_pdom_idle error, enable failed");
		ret = -EINVAL;
	}

	//ip close clock
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1C, REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_CSI_SYS_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1C,
		  REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_SYSBUS_CSI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT0_W1C,
		  REG_BIT(PERI_CRG_CLKGT0_W1S_GT_CLK_CFGBUS_CSI_SHIFT));

	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1C,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_PERI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1C,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_CORE_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1C,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_PERI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1C,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_CORE_SHIFT));
	xr_delay_us(1);

	//ip reset
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base, PERI_CRG_RST0_W1C,
		  REG_BIT(PERI_CRG_RST0_W1C_IP_RST_CSI_N_SHIFT));

	//IP clamp
	writel_io(csi_reg[REG_PCTRL].va, csi_reg[REG_PCTRL].base, PCTRL_ISOLATION_GROUP3,
		  BMRW_SET(PCTRL_ISOLATION_GROUP3_SC_CSI_ISO_EN_SHIFT));
	xr_delay_us(1);
	//crg reset
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base, PERI_CRG_RST0_W1C,
		  REG_BIT(PERI_CRG_RST0_W1C_IP_RST_CSI_CRG_N_SHIFT));

	//rs open clock
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_PERI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_CORE_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_PERI_SHIFT));
	writel_io(csi_reg[REG_PERI_CRG].va, csi_reg[REG_PERI_CRG].base,
		  PERI_CRG_CLKGT1_W1S,
		  REG_BIT(PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_CORE_SHIFT));

	//memory shutdown
	writel_io(csi_reg[REG_PCTRL].va, csi_reg[REG_PCTRL].base, PCTRL_MEM_LGROUP2,
		  BMRW_SET(PCTRL_MEM_LGROUP2_SC_CSI_MEM_SD_SHIFT));
	xr_delay_us(100);
	if (read_field_cmp(csi_reg[REG_PCTRL].va, csi_reg[REG_PCTRL].base, 1,
			   PCTRL_MEM_LPSTATUS2, 0, 1)) {
		XRISP_PR_ERROR("isp_csi mem_lpstatus2 error, enable failed");
		ret = -EINVAL;
	}

	atomic_set(&p_csi_rgltr->rgltr_cnt, 0);
	if (ret)
		XRISP_PR_ERROR("isp_csi power off failed\n");
	else
		XRISP_PR_INFO("isp_csi power off success\n");
	return 0;
}
EXPORT_SYMBOL(xring_isp_csi_disabled);

const struct regulator_ops isp_csi_ops = {
	.is_enabled = xring_csi_is_enabled,
	.enable = xring_isp_csi_enabled,
	.disable = xring_isp_csi_disabled,
};

struct rgltr_s rgltr_csi = { "isp_csi", NULL, &isp_csi_ops };

__maybe_unused static int rgltr_csi_iomap(void)
{
	int i;

	for (i = 0; i < REG_CSI_MAP_MAX; i++) {
		csi_reg[i].va = ioremap(csi_reg[i].base, csi_reg[i].size);
		if (csi_reg[i].va == NULL) {
			XRISP_PR_ERROR("reg %s ioreamp failed", csi_reg[i].name);
			goto iomap_err;
		}
		XRISP_PR_INFO("reg %s base 0x%x, size 0x%x, ioreamp success", csi_reg[i].name,
			csi_reg[i].base, csi_reg[i].size);
	}

	XRISP_PR_INFO("csi all reg ioremap success");
	return 0;
iomap_err:
	for (i = i - 1; i >= 0; i--) {
		if (csi_reg[i].va != NULL)
			iounmap(csi_reg[i].va);
		csi_reg[i].va = NULL;
	}
	return -ENOMEM;
}

__maybe_unused static void rgltr_csi_iounmap(void)
{
	int i;

	for (i = 0; i < REG_CSI_MAP_MAX; i++) {
		if (csi_reg[i].va != NULL)
			iounmap(csi_reg[i].va);

		csi_reg[i].va = NULL;
	}
	XRISP_PR_INFO("csi all reg iounmap success");
}

int xring_csi_probe(struct platform_device *pdev)
{
	struct device_node *np_pm;
	struct regulator_init_data *initdata = NULL;
	struct regulator_config config = {};

	np_pm = pdev->dev.of_node;
	XRISP_PR_INFO("xirng csi probe start\n");

	if (rgltr_csi_iomap()) {
		XRISP_PR_ERROR("reg iomap failed, probe exit\n");
		return -EINVAL;
	}

	initdata = of_get_regulator_init_data(&pdev->dev, np_pm, NULL);
	if (!initdata) {
		XRISP_PR_ERROR("get regulator isp_csi init_data error!\n");
		goto rgltr_reg_fail;
	}

	p_csi_rgltr = &rgltr_csi;
	atomic_set(&rgltr_csi.rgltr_cnt, 0);
	rgltr_csi.rgltr_desc.name = "isp_csi";
	rgltr_csi.rgltr_desc.type = REGULATOR_VOLTAGE;
	rgltr_csi.rgltr_desc.owner = THIS_MODULE;
	rgltr_csi.rgltr_desc.ops = rgltr_csi.ops;
	rgltr_csi.rgltr_desc.n_voltages = 1;

	config.dev = &pdev->dev;
	config.init_data = initdata;
	config.of_node = pdev->dev.of_node;
	config.ena_gpiod = NULL;

	rgltr_csi.rgltr_dev = devm_regulator_register(&pdev->dev, &rgltr_csi.rgltr_desc, &config);

	if (IS_ERR(rgltr_csi.rgltr_dev)) {
		XRISP_PR_ERROR("regulator %s failed to register", "isp_csi");
		goto rgltr_reg_fail;
	}
	XRISP_PR_INFO("regulator %s register success", "isp_csi");

	XRISP_PR_INFO("xrisp csi probe success\n");

	return 0;

rgltr_reg_fail:

	rgltr_csi_iounmap();
	XRISP_PR_ERROR("xrisp csi probe failed\n");
	return -EINVAL;
}

int xring_csi_remove(struct platform_device *pdev)
{

	rgltr_csi.rgltr_dev = NULL;

	rgltr_csi_iounmap();
	XRISP_PR_INFO("xrisp csi remove finish\n");

	return 0;
}

static const struct of_device_id xring_of_match_tbl[] = {
	{
		.compatible = "xring,regulator_csi",
	},
	{ /* end */ }
};

static struct platform_driver xring_pm_driver = {
	.driver = {
		.name = "xrisp_csi",
		.owner = THIS_MODULE,
		.of_match_table = xring_of_match_tbl,
	},
	.probe = xring_csi_probe,
	.remove = xring_csi_remove,
};

static int __init xring_pm_driver_init(void)
{
	return platform_driver_register(&xring_pm_driver);
}

static void __exit xring_pm_driver_exit(void)
{
	platform_driver_unregister(&xring_pm_driver);
}

module_init(xring_pm_driver_init);
module_exit(xring_pm_driver_exit);

MODULE_DESCRIPTION("Regulator Driver for CSI");
MODULE_LICENSE("GPL v2");
