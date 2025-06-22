// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/pci_regs.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/sizes.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/hss2_crg.h>
#include <dt-bindings/xring/platform-specific/pcie_phy.h>
#include <dt-bindings/xring/platform-specific/pcie_ctrl.h>
#include "../../pci.h"
#include "../pci/pcie-xring.h"
#include "../pci/pcie-xring-interface.h"
#include "../pci/dwc_c20pcie4_phy_x2_ns_pcs_raw_ref_100m_ext_rom.h"
#if !IS_ENABLED(CONFIG_XR_PCIE_BW)
#define putreg32	writel
#define getreg32	readl
#endif
#define BAR_INFO_ELEMENT_NUM	2
#define XR_PCIE_NUM_CLKS	11
#define XR_PCIE_CAP_OFF				0x70
#define HSS2_CRG_RST0_PCIE_EP		(HSS2_CRG_RST0_RO_IP_RST_PLL_LOGIC_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_SSMOD_N_MASK |\
					HSS2_CRG_RST0_RO_IP_ARST_HSS2_TCU_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_HSS2_BUS_DATA_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_TCU_TPC_N_MASK |\
					HSS2_CRG_RST0_RO_IP_PRST_PCIE1_DBI_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_PCIE1_AUX_N_MASK |\
					HSS2_CRG_RST0_RO_IP_PRST_PCIE1_N_MASK |\
					HSS2_CRG_RST0_RO_IP_ARST_HSS2_TBU1_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_PCIE1_TPC_N_MASK)
const char *bar_names[] = {"bar0", "bar1", "bar2", "bar3", "bar4", "bar5"};
#define NULL_VENDOR_DEV_ID 0xffffffff
#define MAX_CLK_NAME_LEN 64

struct bar_info {
	bool	editable;
	u32	type;
	u32	mask;
};

struct xr_pcie_ep_clk {
	struct clk	*clk;
	u32		rate;
};

struct xr_pcie_ep {
	struct device			*dev;
#if IS_ENABLED(CONFIG_XR_PCIE_BW)
	u32	pci_dbi;
	u32	pci_ctrl;
	u32	pci_phy;
	u32	pci_phy_apb0;
	u32	pci_dbi2;
	u32 hss2_top;
	u32 hss2_crg;
#else
	void __iomem			*pci_dbi;
	void __iomem			*pci_ctrl;
	void __iomem			*pci_phy;
	void __iomem			*pci_phy_apb0;
	void __iomem            *pci_dbi2;
	void __iomem            *hss2_top;
	void __iomem            *hss2_crg;
#endif
	bool				is_asic;
	u32                 port_num;
	bool				is_clkreq_always_on;
	struct xr_pcie_ep_clk		clk[XR_PCIE_NUM_CLKS];
	const char			*clk_names[XR_PCIE_NUM_CLKS];
	int				perstn_gpio;
	int				num_lanes;
	int				link_gen;
	u32				dev_ven_id;
	struct bar_info			bar_info[PCI_STD_NUM_BARS];
};

static int xr_pcie_get_reg_resources(struct xr_pcie_ep *xr_pcie_ep)
{
	struct platform_device *pdev = to_platform_device(xr_pcie_ep->dev);
#if IS_ENABLED(CONFIG_XR_PCIE_BW)
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	xr_pcie_ep->pci_dbi = res->start;
	dev_info(xr_pcie_ep->dev, "dbi base:0x%x\n", xr_pcie_ep->pci_dbi);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pci_ctrl");
	xr_pcie_ep->pci_ctrl = res->start;
	dev_info(xr_pcie_ep->dev, "pci_ctrl base:0x%x\n", xr_pcie_ep->pci_ctrl);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pci_dbi2");
	xr_pcie_ep->pci_dbi2 = res->start;
	dev_info(xr_pcie_ep->dev, "pci_dbi2 base:0x%x\n", xr_pcie_ep->pci_dbi2);
	if (xr_pcie_ep->is_asic) {
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pcie_phy");
		xr_pcie_ep->pci_phy = res->start;
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pcie_phy_apb0");
		xr_pcie_ep->pci_phy_apb0 = res->start;
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "hss2_crg");
		xr_pcie_ep->hss2_crg = res->start;
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "hss2_top");
		xr_pcie_ep->hss2_top = res->start;
	}
#else
	xr_pcie_ep->pci_dbi = devm_platform_ioremap_resource_byname(pdev, "dbi");
	if (IS_ERR(xr_pcie_ep->pci_dbi))
		return PTR_ERR(xr_pcie_ep->pci_dbi);
	xr_pcie_ep->pci_dbi2 = devm_platform_ioremap_resource_byname(pdev, "pci_dbi2");
	if (IS_ERR(xr_pcie_ep->pci_dbi2))
		return PTR_ERR(xr_pcie_ep->pci_dbi2);
	xr_pcie_ep->pci_dbi2 = xr_pcie_ep->pci_dbi + 0x40000;
	xr_pcie_ep->pci_ctrl = devm_platform_ioremap_resource_byname(pdev, "pci_ctrl");
	if (IS_ERR(xr_pcie_ep->pci_ctrl))
		return PTR_ERR(xr_pcie_ep->pci_ctrl);
	if (xr_pcie_ep->is_asic) {
		xr_pcie_ep->pci_phy = devm_platform_ioremap_resource_byname(pdev, "pcie_phy");
		if (IS_ERR(xr_pcie_ep->pci_phy))
			return PTR_ERR(xr_pcie_ep->pci_phy);
		xr_pcie_ep->pci_phy_apb0 = devm_platform_ioremap_resource_byname(pdev, "pcie_phy_apb0");
		if (IS_ERR(xr_pcie_ep->pci_phy_apb0))
			return PTR_ERR(xr_pcie_ep->pci_phy_apb0);
		xr_pcie_ep->hss2_top = devm_platform_ioremap_resource_byname(pdev, "hss2_top");
		if (IS_ERR(xr_pcie_ep->hss2_top))
			return PTR_ERR(xr_pcie_ep->hss2_top);
		xr_pcie_ep->hss2_crg = devm_platform_ioremap_resource_byname(pdev, "hss2_crg");
		if (IS_ERR(xr_pcie_ep->hss2_crg))
			return PTR_ERR(xr_pcie_ep->hss2_crg);
	}
#endif
	dev_info(xr_pcie_ep->dev, "pcie ep get reg resource succeed\n");
	return 0;
}

static void xr_pcie_get_bar_info(struct xr_pcie_ep *xr_pcie_ep)
{
	struct device *dev = xr_pcie_ep->dev;
	u32 bar_info[BAR_INFO_ELEMENT_NUM];
	int i;
	int ret;

	for (i = 0; i < PCI_STD_NUM_BARS; i++) {
		ret = of_property_read_u32_array(dev->of_node,
				bar_names[i], bar_info, BAR_INFO_ELEMENT_NUM);
		if (ret) {
			xr_pcie_ep->bar_info[i].editable = false;
			dev_info(dev, "bar%d: not editable\n", i);
			continue;
		}
		xr_pcie_ep->bar_info[i].editable = true;
		xr_pcie_ep->bar_info[i].type = bar_info[1];
		xr_pcie_ep->bar_info[i].mask = bar_info[0];
		dev_info(dev, "bar%d: editable, type = 0x%x, mask = 0x%x\n",
			i, xr_pcie_ep->bar_info[i].type, xr_pcie_ep->bar_info[i].mask);
	}
}

static int xr_pcie_ep_parse_clk(struct xr_pcie_ep *xr_pcie_ep)
{
	struct device *dev = xr_pcie_ep->dev;
	struct device_node *np = dev->of_node;
	char clk_rate_str[MAX_CLK_NAME_LEN];
	int ret;
	int i;

	of_property_read_string_array(np, "clock-names", xr_pcie_ep->clk_names, XR_PCIE_NUM_CLKS);
	for (i = 0; i < XR_PCIE_NUM_CLKS; i++) {
		xr_pcie_ep->clk[i].clk = devm_clk_get(dev, xr_pcie_ep->clk_names[i]);
		strscpy(clk_rate_str, xr_pcie_ep->clk_names[i], MAX_CLK_NAME_LEN);
		strcat(clk_rate_str, "_clk_rate");
		ret = of_property_read_u32(np, clk_rate_str, &xr_pcie_ep->clk[i].rate);
		if (ret || IS_ERR(&xr_pcie_ep->clk[i])) {
			dev_err(dev, "failed to parse clk %s\n", xr_pcie_ep->clk_names[i]);
			return PTR_ERR(&xr_pcie_ep->clk[i]);
		}
	}
	for (i = 0; i < XR_PCIE_NUM_CLKS; i++)
		dev_info(dev, "xr_pcie_ep->clk_names[%d]: %s, rate: %d\n",
					 i, xr_pcie_ep->clk_names[i], xr_pcie_ep->clk[i].rate);
	return 0;
}

static int xr_pcie_get_resources(struct xr_pcie_ep *xr_pcie_ep)
{
	struct device_node *np = dev_of_node(xr_pcie_ep->dev);
	int ret;

	ret = xr_pcie_get_reg_resources(xr_pcie_ep);
	if (ret)
		return ret;
	xr_pcie_get_bar_info(xr_pcie_ep);
	if (xr_pcie_ep->is_asic) {
		ret = xr_pcie_ep_parse_clk(xr_pcie_ep);
		if (ret)
			return ret;
	}
	xr_pcie_ep->link_gen = of_pci_get_max_link_speed(np);
	if (of_property_read_u32(np, "num-lanes", &xr_pcie_ep->num_lanes))
		xr_pcie_ep->num_lanes = 1;
	if (of_property_read_u32(np, "dev_ven_id", &xr_pcie_ep->dev_ven_id))
		xr_pcie_ep->dev_ven_id = NULL_VENDOR_DEV_ID;
	return 0;
}

static int xr_pcie_ep_clk_init(struct xr_pcie_ep *xr_pcie_ep)
{
	struct device *dev = xr_pcie_ep->dev;
	int ret;
	int i;

	for (i = 0; i < XR_PCIE_NUM_CLKS; i++) {
		ret = clk_prepare_enable(xr_pcie_ep->clk[i].clk);
		if (ret) {
			dev_err(dev, "failed to enable clk %s\n", xr_pcie_ep->clk_names[i]);
			return ret;
		}
		if (xr_pcie_ep->clk[i].rate) {
			ret = clk_set_rate(xr_pcie_ep->clk[i].clk, xr_pcie_ep->clk[i].rate);
			if (ret) {
				dev_err(dev, "failed to set rate. clk name:%s\n",
						xr_pcie_ep->clk_names[i]);
				return ret;
			}
		}
	}
	dev_info(dev, "enable clk and set rate done\n");
	return 0;
}

static int xr_pcie_ep_gate_init(struct xr_pcie_ep *pcie_ep)
{
	u32 val;

	val = getreg32(pcie_ep->hss2_crg + HSS2_CRG_RST0_RO);
	val = (~val) & HSS2_CRG_RST0_PCIE;
	putreg32(HSS2_CRG_RST0_PCIE, pcie_ep->hss2_crg + HSS2_CRG_RST0_W1S);
	val = getreg32(pcie_ep->hss2_crg + HSS2_CRG_CLKGT0_RO);
	val = (~val) & HSS2_CRG_CLKGT0_PCIE;
	putreg32(HSS2_CRG_CLKGT0_PCIE, pcie_ep->hss2_crg + HSS2_CRG_CLKGT0_W1S);
	if (pcie_ep->port_num == 0) {
		putreg32(HSS2_CRG_CLKGT0_PCIE0, pcie_ep->hss2_crg + HSS2_CRG_CLKGT0_W1S);
		putreg32(HSS2_CRG_RST0_PCIE0, pcie_ep->hss2_crg + HSS2_CRG_RST0_W1S);
	} else if (pcie_ep->port_num == 1) {
		putreg32(HSS2_CRG_CLKGT0_PCIE1, pcie_ep->hss2_crg + HSS2_CRG_CLKGT0_W1S);
		putreg32(HSS2_CRG_RST0_PCIE1, pcie_ep->hss2_crg + HSS2_CRG_RST0_W1S);
	} else {
		dev_err(pcie_ep->dev, "Invalid port num, %d\n", pcie_ep->port_num);
		return -EINVAL;
	}
	return 0;
}

static int xr_pcie_ep_crg_init(struct xr_pcie_ep *xr_pcie_ep)
{
	int ret;

	ret = xr_pcie_ep_gate_init(xr_pcie_ep);
	if (ret)
		return ret;
	ret = xr_pcie_ep_clk_init(xr_pcie_ep);
	if (ret)
		return ret;
	return 0;
}

static void xring_pcie_clkreq_sw_mux_set(struct xr_pcie_ep *pcie_ep, bool state)
{
	struct device *dev = pcie_ep->dev;
	u32 reg;

	reg = getreg32(pcie_ep->pci_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
	if (state) {
		reg |= 1 << PCIE_PHY_PCIE_SYS_TOP_CFG_SW_MUX_CLKREQ_SHIFT;
		dev_info(dev, "ep set clk_req_n off by sw.\n");
	} else {
		reg &= ~PCIE_PHY_PCIE_SYS_TOP_CFG_SW_MUX_CLKREQ_MASK;
		dev_info(dev, "ep set clk_req_n follow hw.\n");
	}
	putreg32(reg, pcie_ep->pci_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
}

static int xr_pcie_ep_mem_shutdown(struct xr_pcie_ep *xr_pcie_ep)
{
	struct device *dev = xr_pcie_ep->dev;
	void __iomem *pctrl;
	u32 val;

	pctrl = ioremap(PCIE_PCTRL_BASE, PCIE_PCTRL_SIZE);
	if (!pctrl) {
		dev_err(dev, "failed to ioremap pctrl\n");
		return -ENOMEM;
	}
	if (xr_pcie_ep->port_num == 0) {
		val = (PCTRL_MEM_LGROUP0_SC_PCIE0_MEM_SD_MASK << 16) &
			(~PCTRL_MEM_LGROUP0_SC_PCIE0_MEM_SD_MASK);
		writel(val, pctrl + PCTRL_MEM_LGROUP0);
		if ((readl(pctrl + PCTRL_MEM_LPSTATUS0) &
			PCTRL_MEM_LPSTATUS0_PCIE0_MEM_SD_ACK_MASK) != 0) {
			dev_err(dev, "failed to set pcie0 mem shutdown\n");
			iounmap(pctrl);
			return -EINVAL;
		}
	} else if (xr_pcie_ep->port_num == 1) {
		val = (PCTRL_MEM_LGROUP1_SC_PCIE1_MEM_SD_MASK << 16) &
			(~PCTRL_MEM_LGROUP1_SC_PCIE1_MEM_SD_MASK);
		writel(val, pctrl + PCTRL_MEM_LGROUP1);
		if ((readl(pctrl + PCTRL_MEM_LPSTATUS1) &
			PCTRL_MEM_LPSTATUS1_PCIE1_MEM_SD_ACK_MASK) != 0) {
			dev_err(dev, "failed to set pcie1 mem shutdown\n");
			iounmap(pctrl);
			return -EINVAL;
		}
	} else {
		dev_err(dev, "Invalid port num, %d\n", xr_pcie_ep->port_num);
		iounmap(pctrl);
		return -EINVAL;
	}
	iounmap(pctrl);
	return 0;
}

static void xr_pcie_ep_powerup_unreset(struct xr_pcie_ep *xr_pcie_ep)
{
	struct device *dev = xr_pcie_ep->dev;

	if (xr_pcie_ep->port_num == 0)
		putreg32(HSS2_CRG_RST0_RO_IP_RST_PCIE0_POWER_UP_N_MASK, xr_pcie_ep->hss2_crg + HSS2_CRG_RST0_W1S);
	else if (xr_pcie_ep->port_num == 1)
		putreg32(HSS2_CRG_RST0_RO_IP_RST_PCIE1_POWER_UP_N_MASK, xr_pcie_ep->hss2_crg + HSS2_CRG_RST0_W1S);
	else
		dev_err(dev, "Invalid port num, %d\n", xr_pcie_ep->port_num);

	dev_info(dev, "pcie%d pwr up de-rst.\n", xr_pcie_ep->port_num);
}

static int xr_pcie_ep_phy_fw_upd(struct xr_pcie_ep *xr_pcie_ep)
{
	u32 val;
	int i;

	for (i =  0; i < 100; i++) {
		val = getreg32(xr_pcie_ep->pci_phy + PCIE_PHY_PHY_MISC_RPT);
		val &= PCIE_PHY_PHY_MISC_RPT_PHY0_SRAM_INIT_DONE_MASK;
		val >>= PCIE_PHY_PHY_MISC_RPT_PHY0_SRAM_INIT_DONE_SHIFT;
		/* 100 times for waiting sram_init_done == 0x1 */
		if (val == 0x1)
			break;
		usleep_range(100, 200);
	}
	if (i == 100) {
		dev_err(xr_pcie_ep->dev,
			"xring_pcie_phy_firmware_upd wait sram_init_done failed\n");
		return -1;
	}
	/* Load Firmware to SRAM, SRAM Base on DWC_PHY CSR 0x8000  */
	for (i = 0; i < sizeof(xring_pcie_phy_firmware) / sizeof(u32); i++)
		putreg32(xring_pcie_phy_firmware[i], xr_pcie_ep->pci_phy_apb0 + ((0x8000 + i) << 2));

	putreg32(0x1, xr_pcie_ep->pci_phy + PCIE_PHY_PHY_MISC_CFG2);
	dev_info(xr_pcie_ep->dev, "xring_pcie_phy_firmware_upd procedure done\n");
	return 0;
}

static int xr_pcie_ep_phy_init(struct xr_pcie_ep *xr_pcie_ep)
{
	u32 val;
	int ret;

	/* set refclk cmn mode */
	val = getreg32(xr_pcie_ep->pci_phy + PCIE_PHY_PIPE_RX0_CFG);
	val |= PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_ES0_CMN_REFCLK_MODE_MASK;
	putreg32(val, xr_pcie_ep->pci_phy + PCIE_PHY_PIPE_RX0_CFG);
	val = getreg32(xr_pcie_ep->pci_phy + PCIE_PHY_PIPE_RX1_CFG);
	val |= PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_ES0_CMN_REFCLK_MODE_MASK;
	putreg32(val, xr_pcie_ep->pci_phy + PCIE_PHY_PIPE_RX1_CFG);
	/* set diffbuf DE_EMP */
	val = getreg32(xr_pcie_ep->pci_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
	val &= ~PCIE_PHY_PCIE_SYS_TOP_CFG_SW_DE_EMP_MASK;
	putreg32(val, xr_pcie_ep->pci_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
	/* Change rom boot mode to sram boot mode */
	val = getreg32(xr_pcie_ep->pci_phy + PCIE_PHY_PHY_MISC_CFG);
	val &= ~PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BYPASS_MODE_MASK;
	val &= ~PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_MASK;
	val |= 0x3 << PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_SHIFT;
	val |= 0x0 << PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BYPASS_MODE_SHIFT;
	putreg32(val, xr_pcie_ep->pci_phy + PCIE_PHY_PHY_MISC_CFG);
	/* clkreq always on */
	if (xr_pcie_ep->is_clkreq_always_on)
		xring_pcie_clkreq_sw_mux_set(xr_pcie_ep, 1);
	xr_pcie_ep_mem_shutdown(xr_pcie_ep);
	xr_pcie_ep_powerup_unreset(xr_pcie_ep);
	ret = xr_pcie_ep_phy_fw_upd(xr_pcie_ep);
	if (ret)
		return ret;
	return 0;
}

static void xr_pcie_dbi_ro_wr_en(struct xr_pcie_ep *xr_pcie_ep)
{
	u32 reg;
	u32 val;

	reg = PCIE_MISC_CONTROL_1_OFF;
	val = getreg32(xr_pcie_ep->pci_dbi + reg);
	val |= PCIE_DBI_RO_WR_EN;
	putreg32(val, xr_pcie_ep->pci_dbi + reg);
}

static void xr_pcie_dbi_ro_wr_dis(struct xr_pcie_ep *xr_pcie_ep)
{
	u32 reg;
	u32 val;

	reg = PCIE_MISC_CONTROL_1_OFF;
	val = getreg32(xr_pcie_ep->pci_dbi + reg);
	val &= ~PCIE_DBI_RO_WR_EN;
	putreg32(val, xr_pcie_ep->pci_dbi + reg);
}

static void xr_pcie_ep_reset_bar(struct xr_pcie_ep *xr_pcie_ep)
{
	u32 reg;
	int i;

	for (i = 0; i < PCI_STD_NUM_BARS; i++) {
		if (xr_pcie_ep->bar_info[i].editable == true) {
			reg = PCI_BASE_ADDRESS_0 + (4 * i);
			putreg32(0x0, xr_pcie_ep->pci_dbi2 + reg);
			putreg32(0x0, xr_pcie_ep->pci_dbi + reg);
		}
	}
}

static void xr_pcie_ep_bar_init(struct xr_pcie_ep *xr_pcie_ep)
{
	u32 reg;
	int i;

	for (i = 0; i < PCI_STD_NUM_BARS; i++) {
		if (xr_pcie_ep->bar_info[i].editable == true) {
			reg = PCI_BASE_ADDRESS_0 + (4 * i);
			putreg32(xr_pcie_ep->bar_info[i].mask, xr_pcie_ep->pci_dbi2 + reg);
			putreg32(xr_pcie_ep->bar_info[i].type, xr_pcie_ep->pci_dbi + reg);
		}
	}
}

static void xr_pcie_ep_perst_unreset(struct xr_pcie_ep *xr_pcie_ep)
{
#if !IS_ENABLED(CONFIG_XR_PCIE_BW)
	u32 val;

	val = getreg32(xr_pcie_ep->pci_ctrl + PCIE_CTRL_PERST_N_OVERRIDE);
	val |= PCIE_CTRL_PERST_N_OVERRIDE_PERST_N_SRC_OVER_MASK;
	putreg32(val, xr_pcie_ep->pci_ctrl + PCIE_CTRL_PERST_N_OVERRIDE);
#else
	gpio_set_value(xr_pcie_ep->perstn_gpio, 1);
#endif
}

static void xr_pcie_link_set_max_speed(struct xr_pcie_ep *xr_pcie_ep)
{
	u32 cap, ctrl2, link_speed;
	u8 offset = XR_PCIE_CAP_OFF;

	cap = getreg32(xr_pcie_ep->pci_dbi + offset + PCI_EXP_LNKCAP);
	ctrl2 = getreg32(xr_pcie_ep->pci_dbi + offset + PCI_EXP_LNKCTL2);
	ctrl2 &= ~PCI_EXP_LNKCTL2_TLS;
	switch (pcie_link_speed[xr_pcie_ep->link_gen]) {
	case PCIE_SPEED_2_5GT:
		link_speed = PCI_EXP_LNKCTL2_TLS_2_5GT;
		break;
	case PCIE_SPEED_5_0GT:
		link_speed = PCI_EXP_LNKCTL2_TLS_5_0GT;
		break;
	case PCIE_SPEED_8_0GT:
		link_speed = PCI_EXP_LNKCTL2_TLS_8_0GT;
		break;
	case PCIE_SPEED_16_0GT:
		link_speed = PCI_EXP_LNKCTL2_TLS_16_0GT;
		break;
	default:
		/* Use hardware capability */
		link_speed = FIELD_GET(PCI_EXP_LNKCAP_SLS, cap);
		ctrl2 &= ~PCI_EXP_LNKCTL2_HASD;
		break;
	}
	putreg32(ctrl2 | link_speed, xr_pcie_ep->pci_dbi + offset + PCI_EXP_LNKCTL2);
	cap &= ~((u32)PCI_EXP_LNKCAP_SLS);
	putreg32(cap | link_speed, xr_pcie_ep->pci_dbi + offset + PCI_EXP_LNKCAP);
}

static void xr_pcie_link_set_max_link_width(struct xr_pcie_ep *xr_pcie_ep)
{
	u32 lnkcap, lwsc, plc;
	u8 cap;

	if (!xr_pcie_ep->num_lanes)
		return;
	/* Set the number of lanes */
	plc = getreg32(xr_pcie_ep->pci_dbi + PCIE_PORT_LINK_CONTROL);
	plc &= ~PORT_LINK_FAST_LINK_MODE;
	plc &= ~PORT_LINK_MODE_MASK;
	/* Set link width speed control register */
	lwsc = getreg32(xr_pcie_ep->pci_dbi + PCIE_LINK_WIDTH_SPEED_CONTROL);
	lwsc &= ~PORT_LOGIC_LINK_WIDTH_MASK;
	switch (xr_pcie_ep->num_lanes) {
	case 1:
		plc |= PORT_LINK_MODE_1_LANES;
		lwsc |= PORT_LOGIC_LINK_WIDTH_1_LANES;
		break;
	case 2:
		plc |= PORT_LINK_MODE_2_LANES;
		lwsc |= PORT_LOGIC_LINK_WIDTH_2_LANES;
		break;
	case 4:
		plc |= PORT_LINK_MODE_4_LANES;
		lwsc |= PORT_LOGIC_LINK_WIDTH_4_LANES;
		break;
	case 8:
		plc |= PORT_LINK_MODE_8_LANES;
		lwsc |= PORT_LOGIC_LINK_WIDTH_8_LANES;
		break;
	default:
		dev_err(xr_pcie_ep->dev, "num-lanes %u: invalid value\n", xr_pcie_ep->num_lanes);
		return;
	}
	putreg32(plc, xr_pcie_ep->pci_dbi + PCIE_PORT_LINK_CONTROL);
	putreg32(lwsc, xr_pcie_ep->pci_dbi + PCIE_LINK_WIDTH_SPEED_CONTROL);
	cap = XR_PCIE_CAP_OFF;
	lnkcap = getreg32(xr_pcie_ep->pci_dbi + cap + PCI_EXP_LNKCAP);
	lnkcap &= ~PCI_EXP_LNKCAP_MLW;
	lnkcap |= FIELD_PREP(PCI_EXP_LNKCAP_MLW, xr_pcie_ep->num_lanes);
	putreg32(lnkcap, xr_pcie_ep->pci_dbi + cap + PCI_EXP_LNKCAP);
}

static void xr_pcie_ep_control_init(struct xr_pcie_ep *xr_pcie_ep)
{
	u32 val;

	xr_pcie_ep->perstn_gpio = of_get_named_gpio(xr_pcie_ep->dev->of_node, "perstn-gpios", 0);
	xr_pcie_ep_perst_unreset(xr_pcie_ep);
	xr_pcie_dbi_ro_wr_en(xr_pcie_ep);
	if (xr_pcie_ep->dev_ven_id)
		putreg32(xr_pcie_ep->dev_ven_id, xr_pcie_ep->pci_dbi + PCI_VENDOR_ID);
	xr_pcie_ep_reset_bar(xr_pcie_ep);
	xr_pcie_ep_bar_init(xr_pcie_ep);
	xr_pcie_dbi_ro_wr_dis(xr_pcie_ep);
	/* set aux clk rate */
	val = getreg32(xr_pcie_ep->pci_dbi + AUX_CLK_FREQ_OFF);
	val &= ~AUX_CLK_FREQ_MASK;
	val |= AUX_CLK_FREQ;
	putreg32(val, xr_pcie_ep->pci_dbi + AUX_CLK_FREQ_OFF);
	if (xr_pcie_ep->link_gen > 0)
		xr_pcie_link_set_max_speed(xr_pcie_ep);
	xr_pcie_link_set_max_link_width(xr_pcie_ep);
}

static int xring_pcie_ep_wait_for_phystatus(struct xr_pcie_ep *xr_pcie_ep)
{
	int retries;
	u32 reg;

	for (retries = 0; retries < PHY_WAIT_MAX_RETRIES; retries++) {
		reg = getreg32(xr_pcie_ep->pci_ctrl + PCIE_CTRL_PHY_STATUS);
		reg &= BIT(PCIE_CTRL_PHY_STATUS_PHY_MAC_PHYSTATUS_SHIFT);
		if (reg == PCIE_CTRL_PHY_STATUS_PHY_MAC_PHYSTATUS_MASK) {
			dev_info(xr_pcie_ep->dev, "phy status is ready\n");
			return 0;
		}
		usleep_range(PHY_WAIT_USLEEP_MIN, PHY_WAIT_USLEEP_MAX);
	}
	dev_err(xr_pcie_ep->dev, "phy status is not ready\n");
	return -ETIMEDOUT;
}

static int xr_pcie_ep_start_link(struct xr_pcie_ep *xr_pcie_ep)
{
	u32 reg;
	int ret;

	if (xr_pcie_ep->is_asic) {
		ret = xring_pcie_ep_wait_for_phystatus(xr_pcie_ep);
		if (ret)
			return ret;
	}
	reg = getreg32(xr_pcie_ep->pci_ctrl + PCIE_CTRL_SII_GENERAL_CORE_CTRL);
	reg |= PCIE_CTRL_SII_GENERAL_CORE_CTRL_APP_LTSSM_ENABLE_MASK;
	putreg32(reg, xr_pcie_ep->pci_ctrl + PCIE_CTRL_SII_GENERAL_CORE_CTRL);
	xring_pcie_probe_port_by_port(0);
	return 0;
}

static int xr_pcie_ep_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xr_pcie_ep *xr_pcie_ep;
	int ret;

	xring_pcie_remove_port_by_port(0);
	xr_pcie_ep = devm_kzalloc(dev, sizeof(*xr_pcie_ep), GFP_KERNEL);
	if (!xr_pcie_ep)
		return -ENOMEM;
	xr_pcie_ep->dev = dev;
	xr_pcie_ep->is_asic = of_property_read_bool(dev->of_node, "asic");
	if (of_property_read_u32(dev->of_node, "port-num", &xr_pcie_ep->port_num)) {
		dev_err(dev, "Not define port-num for this pcie port.\n");
		return -EINVAL;
	}
	xr_pcie_ep->is_clkreq_always_on =
				 of_property_read_bool(dev->of_node, "refclk-always-on");
	dev_info(dev, "dev_name:%s device is_asic:%d port_num:%u\n",
				 dev_name(dev), xr_pcie_ep->is_asic, xr_pcie_ep->port_num);
	ret = xr_pcie_get_resources(xr_pcie_ep);
	if (ret)
		return ret;
	if (xr_pcie_ep->is_asic) {
		ret = xr_pcie_ep_crg_init(xr_pcie_ep);
		if (ret)
			return ret;
	}
	putreg32(0, xr_pcie_ep->pci_ctrl + PCIE_CTRL_DEVICE_TYPE);
	if (xr_pcie_ep->is_asic) {
		ret = xr_pcie_ep_phy_init(xr_pcie_ep);
		if (ret)
			return ret;
	}
	xr_pcie_ep_control_init(xr_pcie_ep);
	xr_pcie_ep_start_link(xr_pcie_ep);
	return 0;
}

static int xr_pcie_ep_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id xr_pcie_ep_of_match[] = {
	{ .compatible = "xring,pcie-ep-pcie1" },
	{},
};
static struct platform_driver xr_pcie_ep_driver = {
	.driver = {
		.name	= "xring-pcie-ep",
		.of_match_table = xr_pcie_ep_of_match,
	},
	.probe = xr_pcie_ep_probe,
	.remove = xr_pcie_ep_remove,
};
module_platform_driver(xr_pcie_ep_driver);
MODULE_LICENSE("GPL v2");
MODULE_DEVICE_TABLE(of, xr_pcie_ep_of_match);

