// SPDX-License-Identifier: GPL-2.0
/*
 * xr-usb-o1.c - USB hardware config for XRing O1 Platform
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "xr-usb-plat.h"
#include "xr-usb-core.h"
#include "xr-usb-generic-phy.h"
#include "xr-usb-util.h"

#include <asm-generic/errno-base.h>
#include <dt-bindings/xring/platform-specific/hss1_crg.h>
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>
#include <dt-bindings/xring/platform-specific/usb_sctrl.h>
#include <dt-bindings/xring/xr-usb-combophy.h>
#include <dt-bindings/xring/xr-usb-hsphy.h>
#include <dwc3/core.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/seq_file.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>

#define BMBW(n)	(BIT(n) << 16)
#define HSS1_BUS_USB_IDLEACK_REQ_STATUS                                         \
	(BIT(LPIS_ACTRL_HSS1_BUS_PDOM_IDLE_HSS1_BUS_USB_IDLEACK_SHIFT) |	\
	 BIT(LPIS_ACTRL_HSS1_BUS_PDOM_IDLE_HSS1_BUS_USB_IDLE_SHIFT))
#define HSS1_BUSIDLE_WAIT_INTERVAL      (10ul)
#define HSS1_BUSIDLE_LOOP_NUM           (100ul)

#define USB_MUX_GET_HW_STATE	0x10000u
#define DSSR_STATEA_MASK		0xFu
#define DSSR_STATE_SHIFT		0
#define PRT_STATEA_MASK			0x7C00000u
#define PRT_STATE_SHIFT			22
#define LTSSM_STATEA_MASK		0x3C0000u
#define LTSSM_STATE_SHIFT		18
#define DWC3_LLUCTL_SUPORT_P4		BIT(28)
#define DWC3_LLUCTL_SUPORT_P4PG		BIT(29)
#define USB3_INDEX			1
#define UPCS_PIPE_CONFIG_P4PG		BIT(9)
#define LANE1_POWER_DOWN_P4PG		0xCu
#define LANE1_POWER_DOWN_NORMAL		0x3u

/* Reg of xHCI USB3 portsc */
#define XHCI_USB3_PORTSC		0x440
#define XHCI_PORTSC_PR			BIT(9)
#define XHCI_PORTSC_PED			BIT(1)

struct xring_usb_o1 {
	void __iomem *usbcore;
	void __iomem *usbsctrl;
	void __iomem *hss1actrl;
	void __iomem *hss1crg;
	struct clk *usbsctrl_apb_clk;
};

static const unsigned int hsphy_eye_default[HSPHY_EYE_PARAM_MAX] = {0x1090386};
static const unsigned int combophy_eye_default[COMBOPHY_EYE_PARAM_MAX] = {
	0x0, 0x0, 0x1800, 0x7F
};

static inline struct xring_usb_o1 *xr_usb_to_o1(struct xring_usb *xr_usb)
{
	return (struct xring_usb_o1 *)(xr_usb->plat_priv);
}

static void xr_usb_usbphy_eye_default_set(struct xring_usb *xr_usb)
{
	unsigned int index;

	for (index = 0; index < HSPHY_EYE_PARAM_MAX; index++)
		xr_usb->device_u2eye[index] = hsphy_eye_default[index];

	for (index = 0; index < HSPHY_EYE_PARAM_MAX; index++)
		xr_usb->host_u2eye[index] = hsphy_eye_default[index];

	for (index = 0; index < COMBOPHY_EYE_PARAM_MAX; index++)
		xr_usb->device_u3eye[index] = combophy_eye_default[index];

	for (index = 0; index < COMBOPHY_EYE_PARAM_MAX; index++)
		xr_usb->host_u3eye[index] = combophy_eye_default[index];
}

static void o1_usb_subsys_sctrl_apb_reset(struct xring_usb_o1 *usb_o1, bool reset)
{
	unsigned int reg;

	if (reset) {
		/* hss1 usb_subsys_sctrl_apb reset */
		reg = BIT(HSS1_CRG_RST0_W1C_IP_PRST_USB_SUBSYS_SCTRL_APB_N_SHIFT);
		writel(reg, usb_o1->hss1crg + HSS1_CRG_RST0_W1C);
	} else {
		/* hss1 usb_subsys_sctrl_apb release */
		reg = BIT(HSS1_CRG_RST0_W1S_IP_PRST_USB_SUBSYS_SCTRL_APB_N_SHIFT);
		writel(reg, usb_o1->hss1crg + HSS1_CRG_RST0_W1S);
	}
}

static int plat_fpag_sys_init(struct xring_usb_o1 *usb_o1)
{
	unsigned int reg;

	/* release usb_vcc */
	reg = BIT(HSS1_CRG_RST0_W1S_IP_RST_USB_VCC_N_SHIFT);
	writel(reg, usb_o1->hss1crg + HSS1_CRG_RST0_W1S);

	/* release PHY */
	writel(0, usb_o1->usbsctrl + USB_FPGA_USB_RESET_WIDTH_SEL);
	/* wait for release */
	msleep(100);

	/* release Controller */
	reg = readl(usb_o1->usbsctrl + USB_IP_RST_CONTROLLER_REG_N_W1S);
	reg |= BIT(USB_IP_W1S_IP_RST_CONTROLLER_REG_N_SHIFT);
	writel(reg, usb_o1->usbsctrl + USB_IP_RST_CONTROLLER_REG_N_W1S);

	/* enable vbus valid */
	reg = readl(usb_o1->usbsctrl + USB_USB_VBUS_VALID_CC_SEL);
	reg &= ~BIT(USB_CC_SEL_USB_VBUS_VALID_SEL_SHIFT);
	reg |= BIT(USB_CC_SEL_USB_VBUS_VALID_CC_SHIFT);
	writel(reg, usb_o1->usbsctrl + USB_USB_VBUS_VALID_CC_SEL);
	msleep(20);

	/* combophy bypass */
	reg = readl(usb_o1->usbsctrl + USB_USB_COMBOPHY_BYPASS);
	reg |= BIT(USB_USB_EUSB_COMBO_DIV_EN_SHIFT) |
		   BIT(USB_USB2_ONLY_MODE_ENABLE_SHIFT);
	writel(reg, usb_o1->usbsctrl + USB_USB_COMBOPHY_BYPASS);

	reg = readl(usb_o1->usbsctrl + USB_STAR_FIX_DISABLE_CTL_INP);
	reg |= BIT(3); /* Decouple USB3 PHY */
	writel(reg, usb_o1->usbsctrl + USB_STAR_FIX_DISABLE_CTL_INP);
	msleep(20);

	return 0;
}

static int plat_asic_sys_init(struct xring_usb_o1 *usb_o1)
{
	unsigned int reg;

	/* hss1 usb_subsys_sctrl_apb release */
	o1_usb_subsys_sctrl_apb_reset(usb_o1, false);

	/* release usb_vcc */
	reg = BIT(HSS1_CRG_RST0_W1S_IP_RST_USB_VCC_N_SHIFT);
	writel(reg, usb_o1->hss1crg + HSS1_CRG_RST0_W1S);
	usleep_range(10, 20);

	return 0;
}

static int plat_sys_init(struct xring_usb *xr_usb)
{
	int ret;
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);

	if (xr_usb->is_fpga)
		ret = plat_fpag_sys_init(usb_o1);
	else
		ret = plat_asic_sys_init(usb_o1);

	if (ret) {
		dev_err(xr_usb->dev, "failed to sys_init\n");
		return ret;
	}

	return 0;
}

static int plat_fpag_sys_exit(struct xring_usb_o1 *usb_o1)
{
	unsigned int reg;

	/* reset usb_vcc */
	reg = BIT(HSS1_CRG_RST0_W1C_IP_RST_USB_VCC_N_SHIFT);
	writel(reg, usb_o1->hss1crg + HSS1_CRG_RST0_W1C);

	return 0;
}

static int plat_asic_sys_exit(struct xring_usb_o1 *usb_o1)
{
	unsigned int reg;

	/* hss1 usb_subsys_sctrl_apb reset */
	o1_usb_subsys_sctrl_apb_reset(usb_o1, true);

	/* reset usb_vcc */
	reg = BIT(HSS1_CRG_RST0_W1C_IP_RST_USB_VCC_N_SHIFT);
	writel(reg, usb_o1->hss1crg + HSS1_CRG_RST0_W1C);

	return 0;
}

static void plat_sys_exit(struct xring_usb *xr_usb)
{
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);
	int ret;

	if (xr_usb->is_fpga)
		ret = plat_fpag_sys_exit(usb_o1);
	else
		ret = plat_asic_sys_exit(usb_o1);

	if (ret)
		dev_err(xr_usb->dev, "failed to sys_exit\n");
}

static int plat_exit_bus_idle(struct xring_usb_o1 *usb_o1, bool set)
{
	unsigned int reg;
	unsigned int val;
	int ret = 0;

	if (set) {
		reg = readl(usb_o1->hss1actrl + LPIS_ACTRL_HSS1_BUS_PDOM_IDLE);
		reg &= BIT(LPIS_ACTRL_HSS1_BUS_PDOM_IDLE_HSS1_BUS_USB_IDLE_SHIFT);
		if (reg) {
			reg = BMBW(LPIS_ACTRL_HSS1_BUS_PDOM_REQ_HSS1_BUS_USB_IDLEREQ_SHIFT);
			writel(reg, usb_o1->hss1actrl + LPIS_ACTRL_HSS1_BUS_PDOM_REQ);
			ret = read_poll_timeout(readl, val, !(val & HSS1_BUS_USB_IDLEACK_REQ_STATUS),
						HSS1_BUSIDLE_WAIT_INTERVAL, HSS1_BUSIDLE_WAIT_INTERVAL *
						HSS1_BUSIDLE_LOOP_NUM, false, usb_o1->hss1actrl +
					LPIS_ACTRL_HSS1_BUS_PDOM_IDLE);
		}
	} else {
		reg = BMBW(LPIS_ACTRL_HSS1_BUS_PDOM_REQ_HSS1_BUS_USB_IDLEREQ_SHIFT) |
			BIT(LPIS_ACTRL_HSS1_BUS_PDOM_REQ_HSS1_BUS_USB_IDLEREQ_SHIFT);
		writel(reg, usb_o1->hss1actrl + LPIS_ACTRL_HSS1_BUS_PDOM_REQ);
		ret = read_poll_timeout(readl, val, (val & HSS1_BUS_USB_IDLEACK_REQ_STATUS) ==
					HSS1_BUS_USB_IDLEACK_REQ_STATUS,
					HSS1_BUSIDLE_WAIT_INTERVAL, HSS1_BUSIDLE_WAIT_INTERVAL *
					HSS1_BUSIDLE_LOOP_NUM, false, usb_o1->hss1actrl +
					LPIS_ACTRL_HSS1_BUS_PDOM_IDLE);
	}

	return ret;
}

static int plat_sys_reset(struct xring_usb *xr_usb)
{
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);
	int ret;

	ret = clk_prepare_enable(usb_o1->usbsctrl_apb_clk);
	if (ret < 0) {
		dev_err(xr_usb->dev, "failed to enable usbsctrl_apb_clk\n");
		return ret;
	}

	plat_sys_exit(xr_usb);

	clk_disable_unprepare(usb_o1->usbsctrl_apb_clk);

	usleep_range(100, 200);

	return 0;
}

static int plat_usb_phy_set_powerdown(struct xring_usb *xr_usb)
{
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);
	unsigned int reg;
	int ret;

	ret = clk_prepare_enable(usb_o1->usbsctrl_apb_clk);
	if (ret < 0) {
		dev_err(xr_usb->dev, "failed to enable usbsctrl_apb_clk\n");
		return ret;
	}

	ret = plat_sys_init(xr_usb);
	if (ret)
		dev_err(xr_usb->dev, "failed to set usb_vcc_rst to powerdown\n");

	reg = readl(usb_o1->usbsctrl + USB_PHY_TEST_MODE);
	reg |= BIT(USB_PHY_TEST_MODE_PHY_TEST_POWERDOWN_SHIFT);
	writel(reg, usb_o1->usbsctrl + USB_PHY_TEST_MODE);

	reg = readl(usb_o1->usbsctrl + USB_TEST_IDDQ);
	reg |= BIT(USB_TEST_IDDQ_TEST_IDDQ_SHIFT);
	writel(reg, usb_o1->usbsctrl + USB_TEST_IDDQ);

	usleep_range(20, 40);

	clk_disable_unprepare(usb_o1->usbsctrl_apb_clk);

	return 0;
}

static void plat_init_standby(struct xring_usb *xr_usb, bool en)
{
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);
	unsigned int reg;

	reg = readl(usb_o1->usbsctrl + USB_PHY_PG_MODE_EN);
	if (en)
		reg |= BIT(USB_PHY_PG_MODE_EN_PHY_PG_MODE_EN_SHIFT);
	else
		reg &= ~BIT(USB_PHY_PG_MODE_EN_PHY_PG_MODE_EN_SHIFT);
	writel(reg, usb_o1->usbsctrl + USB_PHY_PG_MODE_EN);

	reg = readl(usb_o1->usbsctrl + USB_UPCS_PIPE_CONFIG);
	reg &= ~USB_UPCS_PIPE_CONFIG_UPCS_PIPE_CONFIG_MASK;
	if (en)
		reg |= UPCS_PIPE_CONFIG_P4PG;
	writel(reg, usb_o1->usbsctrl + USB_UPCS_PIPE_CONFIG);

	reg = readl(usb_o1->usbsctrl + USB_COMBOPHY_USB32_PIPE_LANE1);
	reg &= ~USB_USB32_PIPE_LANE1_POWERDOWN_MASK;
	if (en)
		reg |= (LANE1_POWER_DOWN_P4PG << USB_USB32_PIPE_LANE1_POWERDOWN_SHIFT);
	else
		reg |= (LANE1_POWER_DOWN_NORMAL << USB_USB32_PIPE_LANE1_POWERDOWN_SHIFT);
	writel(reg, usb_o1->usbsctrl + USB_COMBOPHY_USB32_PIPE_LANE1);
}

static void plat_enter_p4pg(struct xring_usb_o1 *usb_o1, bool en)
{
	unsigned int reg;

	reg = readl(usb_o1->usbcore + DWC3_LLUCTL);
	if (en)
		reg |= (DWC3_LLUCTL_SUPORT_P4PG | DWC3_LLUCTL_SUPORT_P4);
	else
		reg &= ~(DWC3_LLUCTL_SUPORT_P4PG | DWC3_LLUCTL_SUPORT_P4);
	writel(reg, usb_o1->usbcore + DWC3_LLUCTL);
}

static void plat_hsphy_power_standby(struct xring_usb_o1 *usb_o1)
{
	unsigned int reg;

	reg = readl(usb_o1->usbsctrl + USB_TEST_IDDQ);
	reg |= BIT(USB_TEST_IDDQ_TEST_IDDQ_SHIFT);
	writel(reg, usb_o1->usbsctrl + USB_TEST_IDDQ);

	usleep_range(20, 40);
}

static void plat_usb_controller_reset(struct xring_usb_o1 *usb_o1, bool reset)
{
	unsigned int reg;

	if (reset) {
		reg = BIT(USB_IP_W1C_IP_RST_CONTROLLER_REG_N_SHIFT);
		writel(reg, usb_o1->usbsctrl + USB_IP_RST_CONTROLLER_REG_N_W1C);
	} else {
		reg = BIT(USB_IP_W1S_IP_RST_CONTROLLER_REG_N_SHIFT);
		writel(reg, usb_o1->usbsctrl + USB_IP_RST_CONTROLLER_REG_N_W1S);
	}
}

static void plat_usb_reset_standby(struct xring_usb_o1 *usb_o1, bool reset)
{
	unsigned int reg;

	if (reset) {
		reg = readl(usb_o1->usbsctrl + USB_USB_VBUS_VALID_CC_SEL);
		reg &= ~BIT(USB_CC_SEL_USB_VBUS_VALID_CC_SHIFT);
		writel(reg, usb_o1->usbsctrl + USB_USB_VBUS_VALID_CC_SEL);
	} else {
		reg = readl(usb_o1->usbsctrl + USB_USB_VBUS_VALID_CC_SEL);
		reg |= BIT(USB_CC_SEL_USB_VBUS_VALID_CC_SHIFT);
		writel(reg, usb_o1->usbsctrl + USB_USB_VBUS_VALID_CC_SEL);
	}
}

static void plat_power_down(struct xring_usb *xr_usb)
{
	int ret;

	if (xr_usb->lpstate < LP_POWERDOWN) {
		plat_sys_exit(xr_usb);
		return;
	}

	ret = plat_usb_phy_set_powerdown(xr_usb);
	if (ret)
		dev_err(xr_usb->dev, "lpstate:%d, set powerdown fail\n", xr_usb->lpstate);
}

static void xr_usb_init_property(struct xring_usb *xr_usb)
{
	xr_usb->lpstate = LP_P4PG;
}

static int o1_plat_sys_init(struct xring_usb *xr_usb)
{
	return plat_sys_init(xr_usb);
}

static void o1_plat_sys_exit(struct xring_usb *xr_usb)
{
	plat_sys_exit(xr_usb);
}

static void o1_plat_mask_irq(struct xring_usb *xr_usb, enum usb_intr_target tgt, bool on)
{
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);
	unsigned int reg;
	unsigned int mask;

	if (tgt == USB_INTR_AP)
		mask = USB_INTR_MASK_INTR_USB_ACPU_MASK_MASK;
	else if (tgt == USB_INTR_ADSP)
		mask = USB_INTR_MASK_INTR_USB_ADSP_MASK_MASK;
	else
		return;

	reg = readl(usb_o1->usbsctrl + USB_INTR_MASK);
	if (on)
		reg |= mask;
	else
		reg &= ~mask;

	writel(reg, usb_o1->usbsctrl + USB_INTR_MASK);
}

static int o1_plat_exit_bus_idle(struct xring_usb *xr_usb, bool set)
{
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);

	return plat_exit_bus_idle(usb_o1, set);
}

static int o1_plat_sys_reset(struct xring_usb *xr_usb)
{
	return plat_sys_reset(xr_usb);
}

static void o1_plat_power_down(struct xring_usb *xr_usb)
{
	plat_power_down(xr_usb);
}

static int o1_plat_suspend(struct xring_usb *xr_usb)
{
	int ret = 0;

	if (xr_usb->lpstate >= LP_POWERDOWN) {
		ret = plat_sys_reset(xr_usb);
		if (ret)
			dev_err(xr_usb->dev, "lpstate:%d, plat suspend fail\n", xr_usb->lpstate);
	}

	return ret;
}

static int o1_plat_resume(struct xring_usb *xr_usb)
{
	return 0;
}

static int o1_plat_enter_standby(struct xring_usb *xr_usb)
{
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);
	int ret;

	if (!xr_usb->usb3_phy || !xr_usb->usb2_phy) {
		dev_err(xr_usb->dev, "fail to enter standby with no phy\n");
		return -EINVAL;
	}

	ret = plat_exit_bus_idle(usb_o1, true);
	if (ret)
		dev_err(xr_usb->dev, "failed to exit bus idle standby\n");

	ret = plat_sys_init(xr_usb);
	if (ret) {
		dev_err(xr_usb->dev, "failed to init usb sys\n");
		goto err_sys_init;
	}

	plat_init_standby(xr_usb, true);

	ret = xr_usb_hsphy_role_set_mode(xr_usb->usb2_phy, PHY_MODE_USB_DEVICE);
	if (ret) {
		dev_err(xr_usb->dev, "failed to set hsphy role mode standby\n");
		goto err_hsphy_role_set_mode;
	}

	ret = xr_usb_hsphy_init(xr_usb->usb2_phy);
	if (ret) {
		dev_err(xr_usb->dev, "failed to init hsphy standby\n");
		goto err_hsphy_init;
	}

	ret = xr_usb_combophy_role_set_mode_ext(xr_usb->usb3_phy,
		PHY_MODE_USB_DEVICE, PHY_FLIP_INVERT | PHY_ORIENT_FLIP);
	if (ret) {
		dev_err(xr_usb->dev, "hsphy_role_set_mode fail\n");
		goto err_combophy_role_set;
	}

	ret = xr_usb_combophy_init(xr_usb->usb3_phy);
	if (ret) {
		dev_err(xr_usb->dev, "failed to init combphy standby\n");
		goto err_combophy_init;
	}

	plat_usb_reset_standby(usb_o1, false);
	plat_usb_controller_reset(usb_o1, false);

	ret = xr_usb_combophy_mode_toggle(xr_usb->usb3_phy, USB_MODE);
	if (ret) {
		dev_err(xr_usb->dev, "failed to phy toggle mode standby\n");
		goto err_toggle_mode_standby;
	}

	plat_enter_p4pg(usb_o1, true);

	ret = xr_usb_combophy_lpcheck(xr_usb->usb3_phy);
	if (ret) {
		dev_err(xr_usb->dev, "failed to combophy lpcheck\n");
		goto err_toggle_mode_standby;
	}

	plat_usb_reset_standby(usb_o1, true);
	plat_exit_bus_idle(usb_o1, false);
	xr_usb_hsphy_exit(xr_usb->usb2_phy);

	plat_hsphy_power_standby(usb_o1);

	return 0;

err_toggle_mode_standby:
	plat_usb_controller_reset(usb_o1, true);
	plat_usb_reset_standby(usb_o1, true);
	xr_usb_combophy_exit(xr_usb->usb3_phy);
err_combophy_init:
	xr_usb_combophy_role_set_mode_ext(xr_usb->usb3_phy, PHY_MODE_INVALID, 0);
err_combophy_role_set:
	xr_usb_hsphy_exit(xr_usb->usb2_phy);
err_hsphy_init:
	xr_usb_hsphy_role_set_mode(xr_usb->usb2_phy, PHY_MODE_INVALID);
err_hsphy_role_set_mode:
	plat_sys_exit(xr_usb);
err_sys_init:
	plat_exit_bus_idle(usb_o1, false);
	return ret;
}

static int o1_plat_exit_standby(struct xring_usb *xr_usb)
{
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);
	int ret;

	if (!xr_usb->usb3_phy) {
		dev_err(xr_usb->dev, "fail to exit standby with no u3phy\n");
		return -EINVAL;
	}

	ret = clk_prepare_enable(usb_o1->usbsctrl_apb_clk);
	if (ret < 0) {
		dev_err(xr_usb->dev, "standby failed to enable usbsctrl_apb_clk\n");
		return ret;
	}

	ret = xr_usb_combophy_exit(xr_usb->usb3_phy);
	if (ret)
		dev_err(xr_usb->dev, "standby combophy_exit fail\n");

	clk_disable_unprepare(usb_o1->usbsctrl_apb_clk);

	return 0;
}

static void disable_usb3_port(struct xring_usb_o1 *usb_o1, bool dis)
{
	unsigned int reg;

	reg = readl(usb_o1->usbcore + XHCI_USB3_PORTSC);
	if (dis)
		reg &= ~XHCI_PORTSC_PR;
	else
		reg |= XHCI_PORTSC_PR;
	writel(reg, usb_o1->usbcore + XHCI_USB3_PORTSC);
}

static void o1_plat_usb3_enter_standby(struct xring_usb *xr_usb)
{
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);

	disable_usb3_port(usb_o1, true);
	plat_init_standby(xr_usb, true);
	plat_enter_p4pg(usb_o1, true);
}

static void o1_plat_usb3_exit_standby(struct xring_usb *xr_usb)
{
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);

	plat_enter_p4pg(usb_o1, false);
	plat_init_standby(xr_usb, false);
	disable_usb3_port(usb_o1, false);
}

static int o1_plat_probe(struct xring_usb *xr_usb)
{
	struct platform_device *pdev = to_platform_device(xr_usb->dev);
	struct xring_usb_o1 *usb_o1 = xr_usb_to_o1(xr_usb);
	int ret = 0;

	usb_o1->usbcore =
		devm_ioremap_resource_byname_no_req_region(pdev, "usbcore");
	if (!usb_o1->usbcore) {
		dev_err(xr_usb->dev, "unable to remap \"usbcore\" memory\n");
		return -ENOMEM;
	}

	usb_o1->usbsctrl =
		devm_ioremap_resource_byname_no_req_region(pdev, "usbsctrl");
	if (!usb_o1->usbsctrl) {
		dev_err(xr_usb->dev, "failed to get usbsctrl\n");
		return -ENOMEM;
	}

	usb_o1->hss1actrl =
		devm_ioremap_resource_byname_no_req_region(pdev, "hss1actrl");
	if (!usb_o1->hss1actrl) {
		dev_err(xr_usb->dev, "failed to get hss1actrl\n");
		return -ENOMEM;
	}

	usb_o1->hss1crg =
		devm_ioremap_resource_byname_no_req_region(pdev, "hss1crg");
	if (!usb_o1->hss1crg) {
		dev_err(xr_usb->dev, "failed to get hss1crg\n");
		return -ENOMEM;
	}

	usb_o1->usbsctrl_apb_clk = devm_clk_get(xr_usb->dev, "usbsctrl_apb_clk");
	if (IS_ERR(usb_o1->usbsctrl_apb_clk)) {
		dev_err(xr_usb->dev, "failed to get usbsctrl_apb_clk\n");
		return  -EFAULT;
	}

	xr_usb_init_property(xr_usb);

	xr_usb_usbphy_eye_default_set(xr_usb);

	return ret;
}

struct xring_usb_plat g_xr_usb_o1_plat = {
	.priv_size = sizeof(struct xring_usb_o1),
	.plat_probe = o1_plat_probe,
	.plat_remove = NULL,
	.set_quirks = NULL,
	.sys_init = o1_plat_sys_init,
	.sys_exit = o1_plat_sys_exit,
	.mask_irq = o1_plat_mask_irq,
	.exit_bus_idle = o1_plat_exit_bus_idle,
	.powerup = o1_plat_sys_reset,
	.powerdown = o1_plat_power_down,
	.plat_suspend = o1_plat_suspend,
	.plat_resume = o1_plat_resume,
	.enter_standby = o1_plat_enter_standby,
	.exit_standby = o1_plat_exit_standby,
	.usb3_enter_standby = o1_plat_usb3_enter_standby,
	.usb3_exit_standby = o1_plat_usb3_exit_standby,
};
