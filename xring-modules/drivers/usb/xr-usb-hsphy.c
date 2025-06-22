// SPDX-License-Identifier: GPL-2.0
/*
 * xr-usb-hsphy.c - 2.0 Phy Driver for XRing USB.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "xr-usb-generic-phy.h"
#include "xr-usb-repeater.h"
#include "xr-usb-util.h"

#include <asm-generic/errno-base.h>
#include <dt-bindings/xring/platform-specific/usb_sctrl.h>
#include <dt-bindings/xring/xr-usb-hsphy.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>

#undef pr_fmt
#undef dev_fmt
#define pr_fmt(fmt) "[%s]: %s:%d: " fmt, KBUILD_MODNAME, __func__, __LINE__
#define dev_fmt(fmt) ": %s:%d: " fmt, __func__, __LINE__

#define GCTL			0xC110ul
#define GCTL_DSBLCLKGTNG	BIT(0)
#define GBMUCTL			0xC164ul
#define GBMUCTL_ACTIVE_ID_EN	BIT(1)
#define GUCTL			0xC12Cul
#define GUCTL_REFCLKPER(n)	((n) << 22)
#define GUCTL_REFCLKPER_MASK	GUCTL_REFCLKPER(0x3ff)
#define GUCTL1			0xC11Cul
#define GUCTL1_HW_LPM_CAP_DISABLE	BIT(14)
#define GUCTL1_HW_LPM_HLE_DISABLE	BIT(13)

#define REFCLK_PER 0x34

/*
 * |-------+-----------------------+---------+-------+----------+-----------|
 * | Index | Prop                  | BitSet  | Shift |     Mask | RegConfig |
 * |-------+-----------------------+---------+-------+----------+-----------|
 * |     0 | RX_HS                 | [3:1]   |     1 |      0x7 | 1:[3:1]   |
 * |       | TX_FSLS_SLEW_RATE     | [4]     |     4 |      0x1 | 1:[4]     |
 * |       | TX_HS_DRV_OFST        | [7:6]   |     6 |      0x3 | 1:[7:6]   |
 * |       | TX_HS_VREF            | [10:8]  |     8 |      0x7 | 1:[10:8]  |
 * |       | TX_HS_XV              | [12:11] |    11 |      0x3 | 1:[12:11] |
 * |       | TX_PREEMP             | [15:13] |    13 |     0x07 | 1:[15:13] |
 * |       | TX_RES                | [17:16] |    16 |     0x03 | 1:[17:16] |
 * |       | TX_RISE               | [19:18] |    18 |      0x3 | 1:[19:18] |
 * |       | RX_EQ_CTLE            | [25:24] |    24 |      0x3 | 0:[1:0]   |
 * |-------+-----------------------+---------+-------+----------+-----------|
 *
 * |-------+-----------------------------+----------------|
 * | Index | RegName                     | Offset         |
 * |-------+-----------------------------+----------------|
 * |     0 | RXEQ_CTLE                   | 0x128          |
 * |     1 | RX_HS_TERM_EN               | 0x12C          |
 * |-------+-----------------------------+----------------|
 */
#pragma pack(4)
struct hsphy_eye_param {
	unsigned int reserved : 1;
	unsigned int rx_hs : 3;
	unsigned int tx_fsls_slew_rate : 1;
	unsigned int reserved2 : 1;
	unsigned int tx_hs_drv_ofst : 2;
	unsigned int tx_hs_vref : 3;
	unsigned int tx_hs_xv : 2;
	unsigned int tx_preemp : 3;
	unsigned int tx_res : 2;
	unsigned int tx_rise : 2;
	unsigned int reserved3 : 4;
	unsigned int rx_eq_ctle : 2;
	unsigned int reserved4 : 6;
};
#pragma pack()

struct xr_usb_hsphy_priv {
	struct device *dev;
	void __iomem *usbcore;
	void __iomem *eusbphy;
	void __iomem *usbctrl;

	struct phy *phy;
	struct usb_repeater *repeater;
	struct xr_usb_hsphy hsphy;

	unsigned int core_active_id_disable_quirk : 1;
	unsigned int core_dis_clkgating_quirk : 1;
	unsigned int core_dis_lpm_quirk : 1;
	unsigned int is_fpga : 1;
};

void hsphy_eye_set(struct xr_usb_hsphy_priv *priv, const void *param)
{
	const struct hsphy_eye_param *p = param;
	unsigned int val = 0;
	unsigned int reg;

	reg = readl(priv->usbctrl + USB_PHY_CFG_RX_HS_TERM_EN);
	reg &= ~(unsigned int)(USB_PHY_CFG_RX_HS_TUNE_MASK |
			       USB_PHY_CFG_TX_FSLS_SLEW_RATE_TUNE_MASK |
			       USB_PHY_CFG_TX_HS_DRV_OFST_MASK |
			       USB_PHY_CFG_TX_HS_VREF_TUNE_MASK |
			       USB_PHY_CFG_TX_HS_XV_TUNE_MASK |
			       USB_PHY_CFG_TX_PREEMP_TUNE_MASK |
			       USB_PHY_CFG_TX_RES_TUNE_MASK |
			       USB_PHY_CFG_TX_RISE_TUNE_MASK);
	val = (p->rx_hs << USB_PHY_CFG_RX_HS_TUNE_SHIFT) |
	      (p->tx_fsls_slew_rate << USB_PHY_CFG_TX_FSLS_SLEW_RATE_TUNE_SHIFT) |
	      (p->tx_hs_drv_ofst << USB_PHY_CFG_TX_HS_DRV_OFST_SHIFT) |
	      (p->tx_hs_vref << USB_PHY_CFG_TX_HS_VREF_TUNE_SHIFT) |
	      (p->tx_hs_xv << USB_PHY_CFG_TX_HS_XV_TUNE_SHIFT) |
	      (p->tx_preemp << USB_PHY_CFG_TX_PREEMP_TUNE_SHIFT) |
	      (p->tx_res << USB_PHY_CFG_TX_RES_TUNE_SHIFT) |
	      (p->tx_rise << USB_PHY_CFG_TX_RISE_TUNE_SHIFT);
	reg |= val;
	writel(reg, priv->usbctrl + USB_PHY_CFG_RX_HS_TERM_EN);

	reg = readl(priv->usbctrl + USB_PHY_CFG_RXEQ_CTLE);
	reg &= ~(unsigned int)USB_PHY_CFG_RXEQ_CTLE_PHY_CFG_RX_EQ_CTLE_MASK;
	val = p->rx_eq_ctle << USB_PHY_CFG_RXEQ_CTLE_PHY_CFG_RX_EQ_CTLE_SHIFT;
	reg |= val;
	writel(reg, priv->usbctrl + USB_PHY_CFG_RXEQ_CTLE);
}

unsigned int hsphy_eye_get(struct xr_usb_hsphy_priv *priv)
{
	struct hsphy_eye_param *p;
	unsigned int val = 0;
	unsigned int reg;

	p = (struct hsphy_eye_param *)&val;
	reg = readl(priv->usbctrl + USB_PHY_CFG_RX_HS_TERM_EN);
	p->rx_hs = (reg & USB_PHY_CFG_RX_HS_TUNE_MASK) >>
		    USB_PHY_CFG_RX_HS_TUNE_SHIFT;
	p->tx_fsls_slew_rate = (reg & USB_PHY_CFG_TX_FSLS_SLEW_RATE_TUNE_MASK) >>
				USB_PHY_CFG_TX_FSLS_SLEW_RATE_TUNE_SHIFT;
	p->tx_hs_drv_ofst = (reg & USB_PHY_CFG_TX_HS_DRV_OFST_MASK) >>
			     USB_PHY_CFG_TX_HS_DRV_OFST_SHIFT;
	p->tx_hs_vref = (reg & USB_PHY_CFG_TX_HS_VREF_TUNE_MASK) >>
			 USB_PHY_CFG_TX_HS_VREF_TUNE_SHIFT;
	p->tx_hs_xv = (reg & USB_PHY_CFG_TX_HS_XV_TUNE_MASK) >>
		       USB_PHY_CFG_TX_HS_XV_TUNE_SHIFT;
	p->tx_preemp = (reg & USB_PHY_CFG_TX_PREEMP_TUNE_MASK) >>
			USB_PHY_CFG_TX_PREEMP_TUNE_SHIFT;
	p->tx_res = (reg & USB_PHY_CFG_TX_RES_TUNE_MASK) >>
		     USB_PHY_CFG_TX_RES_TUNE_SHIFT;
	p->tx_rise = (reg & USB_PHY_CFG_TX_RISE_TUNE_MASK) >>
		      USB_PHY_CFG_TX_RISE_TUNE_SHIFT;

	reg = readl(priv->usbctrl + USB_PHY_CFG_RXEQ_CTLE);
	p->rx_eq_ctle = (reg & USB_PHY_CFG_RXEQ_CTLE_PHY_CFG_RX_EQ_CTLE_MASK) >>
			 USB_PHY_CFG_RXEQ_CTLE_PHY_CFG_RX_EQ_CTLE_SHIFT;

	return val;
}

void (*hsphy_eye_set_func[])(struct xr_usb_hsphy_priv *, const void *) = {
	hsphy_eye_set,
};

unsigned int (*hsphy_eye_get_func[])(struct xr_usb_hsphy_priv *) = {
	hsphy_eye_get,
};

static const char * const eye_param1_regname[] = {
	"[3:1]RX_HS",
	"[4]TX_FSLS_SLEW_RATE",
	"[7:6]TX_HS_DRV_OFST",
	"[10:8]TX_HS_VREF",
	"[12:11]TX_HS_XV",
	"[15:13]TX_PREEMP",
	"[17:16]TX_RES",
	"[19:18]TX_RISE",
	"[25:24]RX_EQ_CTLE",
	NULL
};

static const char *const *xr_usb_hsphy_eye_param_name_array[] = {
	&eye_param1_regname[0],
};

static void hsphy_param_form(struct seq_file *s)
{
	int index;
	const char *const *src = NULL;

	seq_puts(s, "***************************************\n");

	for (index = 0; index < HSPHY_EYE_PARAM_MAX; index++) {
		seq_printf(s, "parameter group[%d]\n", index);
		src = xr_usb_hsphy_eye_param_name_array[index];

		for (; *src != NULL; src++)
			seq_printf(s, "    %s\n", *src);
	}
	seq_puts(s, "***************************************\n");
}

static void write_hsphy_eye_param(struct xr_usb_hsphy_priv *priv,
		int index, const unsigned int *value)
{
	if (priv->is_fpga) {
		dev_info(priv->dev, "hsphy set eye param[%d]:0x%x\n", index, *value);
		return;
	}

	hsphy_eye_set_func[index](priv, value);
}

static int read_hsphy_eye_param(struct xr_usb_hsphy_priv *priv,
		int index, unsigned int *val)
{
	if (priv->is_fpga) {
		dev_info(priv->dev, "fpga no phy to get!\n");
		return -ENOENT;
	}

	*val = hsphy_eye_get_func[index](priv);
	return 0;
}

static int hsphy_set_eye_param(struct xr_usb_hsphy *hsphy_data,
		int index, const unsigned int *value)
{
	struct xr_usb_hsphy_priv *priv =
		container_of(hsphy_data, struct xr_usb_hsphy_priv, hsphy);
	struct phy *phy = NULL;

	if (index >= HSPHY_EYE_PARAM_MAX) {
		dev_err(priv->dev, "no valid string matched to set\n");
		return -EINVAL;
	}

	phy = priv->hsphy.phy;
	if (!phy)
		return -EFAULT;

	mutex_lock(&phy->mutex);

	if (phy->power_count > 0)
		write_hsphy_eye_param(priv, index, value);

	mutex_unlock(&phy->mutex);

	return 0;
}

static int hsphy_get_eye_param(struct xr_usb_hsphy *hsphy_data,
		int index, unsigned int *val)
{
	struct xr_usb_hsphy_priv *priv =
		container_of(hsphy_data, struct xr_usb_hsphy_priv, hsphy);
	struct phy *phy = NULL;
	int ret;

	if (index >= HSPHY_EYE_PARAM_MAX) {
		dev_err(priv->dev, "no valid string matched to get\n");
		return -EINVAL;
	}

	phy = priv->hsphy.phy;
	if (!phy)
		return -EINVAL;

	mutex_lock(&phy->mutex);

	if (phy->power_count > 0)
		ret = read_hsphy_eye_param(priv, index, val);
	else
		ret = -ENOENT;

	mutex_unlock(&phy->mutex);

	return ret;
}

static int xr_usb_hsphy_core_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	struct xr_usb_hsphy_priv *priv = phy_get_drvdata(phy);
	unsigned int reg;

	/* ACTIVE_ID DISABLE */
	if (priv->core_active_id_disable_quirk) {
		dev_info(priv->dev, "disable core active id feature\n");
		reg = readl(priv->usbcore + GBMUCTL);
		reg &= ~GBMUCTL_ACTIVE_ID_EN;
		writel(reg, priv->usbcore + GBMUCTL);
	}

	/* CLOCK GATE DISABLE */
	if (priv->core_dis_clkgating_quirk) {
		dev_info(priv->dev, "disable core clock gate\n");
		reg = readl(priv->usbcore + GCTL);
		reg |= GCTL_DSBLCLKGTNG;
		writel(reg, priv->usbcore + GCTL);
	}

	if (priv->core_dis_lpm_quirk) {
		dev_info(priv->dev, "disable core lpm\n");
		reg = readl(priv->usbcore + GUCTL1);
		reg |= (GUCTL1_HW_LPM_CAP_DISABLE |
			GUCTL1_HW_LPM_HLE_DISABLE);
		writel(reg, priv->usbcore + GUCTL1);
	}

	/* REFCLKPER set */
	if (mode == PHY_MODE_USB_HOST) {
		reg = readl(priv->usbcore + GUCTL);
		reg &= ~GUCTL_REFCLKPER_MASK;
		reg |= GUCTL_REFCLKPER(REFCLK_PER);
		writel(reg, priv->usbcore + GUCTL);
	}

	return 0;
}

static const struct phy_ops xr_usb_hsphy_core_ops = {
	.set_mode = xr_usb_hsphy_core_set_mode,
	.owner = THIS_MODULE,
};

static int hsphy_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	struct xr_usb_hsphy *data_hsphy = phy_get_drvdata(phy);

	dev_info(&phy->dev, "enter\n");
	data_hsphy->mode = mode;
	dev_info(&phy->dev, "hsphy mode:%d\n", data_hsphy->mode);

	return 0;
}

static void hsphy_enable_setting(struct xr_usb_hsphy_priv *priv)
{
	unsigned int reg;

	if (priv->is_fpga)
		return;

	reg = readl(priv->usbctrl + USB_PHY_CFG_PLL_CPBIAS_CNTRL);
	reg &= ~(unsigned int)USB_CNTRL_PHY_CFG_PLL_CPBIAS_CNTRL_MASK;
	writel(reg, priv->usbctrl + USB_PHY_CFG_PLL_CPBIAS_CNTRL);

	reg = BIT(USB_IP_RST_EUSB_REG_W1C_IP_RST_EUSB_REG_SHIFT);
	writel(reg, priv->usbctrl + USB_IP_RST_EUSB_REG_W1C);

	reg = readl(priv->usbctrl + USB_EUSB_SOFT_ENABLE);
	reg |= BIT(USB_EUSB_SOFT_ENABLE_EUSBPHY_SOFT_ENABLE_SHIFT);
	writel(reg, priv->usbctrl + USB_EUSB_SOFT_ENABLE);
}

static void hsphy_disable_setting(struct xr_usb_hsphy_priv *priv)
{
	unsigned int reg;

	if (priv->is_fpga)
		return;

	reg = BIT(USB_IP_RST_EUSB_REG_W1S_IP_RST_EUSB_REG_SHIFT);
	writel(reg, priv->usbctrl + USB_IP_RST_EUSB_REG_W1S);
}

static int repeater_reset(struct xr_usb_hsphy_priv *priv, bool set)
{
	int ret;

	if (!priv->repeater)
		return -ENODEV;

	ret = usb_repeater_reset(priv->repeater, set);
	if (ret)
		dev_err(priv->dev, "repeater %s fail\n",
			set ? "reset" : "release");

	return ret;
}

static int repeater_init(struct xr_usb_hsphy_priv *priv)
{
	int ret;

	if (!priv->repeater)
		return -ENODEV;

	ret = usb_repeater_init(priv->repeater, priv->hsphy.mode);
	if (ret)
		dev_err(priv->dev, "repeater init fail\n");

	return ret;
}

static int hsphy_init(struct phy *phy)
{
	struct xr_usb_hsphy *data_hsphy = phy_get_drvdata(phy);
	struct xr_usb_hsphy_priv *priv =
		container_of(data_hsphy, struct xr_usb_hsphy_priv, hsphy);
	int ret;

	dev_info(priv->dev, "enter\n");

	ret = repeater_reset(priv, false);
	if (ret)
		dev_err(priv->dev, "usb repeater release fail\n");

	usleep_range(1000, 2000);

	ret = repeater_init(priv);
	if (ret)
		dev_err(priv->dev, "usb repeater not init\n");

	hsphy_enable_setting(priv);

	usleep_range(50, 100);

	return 0;
}

static int hsphy_exit(struct phy *phy)
{
	struct xr_usb_hsphy *data_hsphy = phy_get_drvdata(phy);
	struct xr_usb_hsphy_priv *priv =
		container_of(data_hsphy, struct xr_usb_hsphy_priv, hsphy);
	int ret;

	dev_info(priv->dev, "enter\n");

	data_hsphy->mode = PHY_MODE_INVALID;

	hsphy_disable_setting(priv);

	ret = repeater_reset(priv, true);
	if (ret)
		dev_err(priv->dev, "usb repeater reset fail\n");

	return 0;
}

static int hsphy_poweron(struct phy *phy)
{
	dev_info(&phy->dev, "power on\n");
	return 0;
}

static int hsphy_poweroff(struct phy *phy)
{
	dev_info(&phy->dev, "power off\n");
	return 0;
}

static const struct phy_ops xr_usb_hsphy_ops = {
	.owner = THIS_MODULE,
	.init = hsphy_init,
	.exit = hsphy_exit,
	.power_on = hsphy_poweron,
	.power_off = hsphy_poweroff,
	.set_mode = hsphy_set_mode,
};

static struct phy *xr_usb_hsphy_of_xlate(struct device *dev,
					 struct of_phandle_args *args)
{
	struct xr_usb_hsphy_priv *priv = dev_get_drvdata(dev);
	unsigned int phy_id;
	struct phy *phy = NULL;

	if (!priv)
		return ERR_PTR(-ENOENT);

	if (args->args_count != XR_USB_HSPHY_CELL_SIZE)
		return ERR_PTR(-EINVAL);

	phy_id = args->args[XR_USB_HSPHY_CELL_ID];
	if (phy_id >= USB_HSPHY_IDX_MAX)
		return ERR_PTR(-EINVAL);

	switch (phy_id) {
	case USB_HSPHY_IDX_DWC3:
		if (priv->phy)
			phy = priv->phy;
		break;

	case USB_HSPHY_IDX_XR_USB:
		if (priv->hsphy.phy)
			phy = priv->hsphy.phy;
		break;

	default:
		dev_err(dev, "unknown hsphy\n");
		break;
	}

	if (!phy) {
		dev_err(dev, "hsphy register fail\n");
		return ERR_PTR(-EINVAL);
	}

	return phy;
}

static int xr_usb_hsphy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct phy *phy = NULL;
	struct phy_provider *phy_provider = NULL;
	struct xr_usb_hsphy_priv *priv = NULL;
	struct usb_repeater *usb_repeater;

	dev_info(dev, "probe enter\n");
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = dev;

	priv->usbcore = devm_ioremap_resource_byname_no_req_region(pdev, "usbcore");
	if (!priv->usbcore) {
		dev_err(dev, "unable to remap \"usbcore\" memory\n");
		return -ENOMEM;
	}

	priv->eusbphy = devm_ioremap_resource_byname_no_req_region(pdev, "eusb_phy");
	if (!priv->eusbphy) {
		dev_err(dev, "unable to remap \"eusbphy\" memory\n");
		return -ENOMEM;
	}

	priv->usbctrl = devm_ioremap_resource_byname_no_req_region(pdev, "usbctrl");
	if (!priv->usbctrl) {
		dev_err(dev, "unable to remap \"usbctrl\" memory\n");
		return -ENOMEM;
	}

	/* get property */
	priv->is_fpga = device_property_read_bool(dev, "fpga");

	priv->core_active_id_disable_quirk =
		device_property_read_bool(dev, "core-active-id-disable-quirk");

	priv->core_dis_clkgating_quirk =
		device_property_read_bool(dev, "core-dis-clkgating-quirk");

	priv->core_dis_lpm_quirk =
		device_property_read_bool(dev, "core-dis-lpm-quirk");

	/* hsphy for dwc3 core */
	phy = devm_phy_create(dev, NULL, &xr_usb_hsphy_core_ops);
	if (IS_ERR(phy))
		return PTR_ERR(phy);
	priv->phy = phy;
	phy_set_drvdata(phy, priv);

	/* hsphy for xr-usb */
	phy = devm_phy_create(dev, NULL, &xr_usb_hsphy_ops);
	if (IS_ERR(phy))
		return PTR_ERR(phy);
	priv->hsphy.phy = phy;
	phy_set_drvdata(phy, &priv->hsphy);

	/* repeater handle get */
	usb_repeater = usb_get_repeater_by_phandle(dev, "usb-repeater", 0);
	if (IS_ERR(usb_repeater)) {
		priv->repeater = NULL;
		if (PTR_ERR(usb_repeater) == -ENODEV) {
			dev_info(dev, "no repeater phandle\n");
		} else {
			dev_warn(dev, "failed to get repeater - %ld", PTR_ERR(usb_repeater));
			return PTR_ERR(usb_repeater);
		}
	} else {
		priv->repeater = usb_repeater;
	}

	priv->hsphy.set_eye_param = hsphy_set_eye_param;
	priv->hsphy.get_eye_param = hsphy_get_eye_param;
	priv->hsphy.param_form = hsphy_param_form;
	priv->hsphy.mode = PHY_MODE_INVALID;

	dev_set_drvdata(dev, priv);
	phy_provider = devm_of_phy_provider_register(dev, xr_usb_hsphy_of_xlate);
	return PTR_ERR_OR_ZERO(phy_provider);
}

static int xr_usb_hsphy_remove(struct platform_device *pdev)
{
	struct xr_usb_hsphy_priv *priv = dev_get_drvdata(&pdev->dev);

	if (priv->repeater)
		usb_put_repeater(priv->repeater);

	return 0;
}

static const struct of_device_id xr_usb_hsphy_of_match[] = {
	{ .compatible = "xring,o1,usb-hsphy" },
	{},
};
MODULE_DEVICE_TABLE(of, xr_usb_hsphy_of_match);

static struct platform_driver
	xr_usb_hsphy_driver = { .probe = xr_usb_hsphy_probe,
				.remove = xr_usb_hsphy_remove,
				.driver = {
					.name = "xr-usb-hsphy",
					.of_match_table = xr_usb_hsphy_of_match,
				} };

int __init usb_hsphy_init(void)
{
	int ret;

	ret = platform_driver_register(&xr_usb_hsphy_driver);
	if (ret)
		pr_err("fail to register xr_usb_hsphy");

	return ret;
}

void usb_hsphy_exit(void)
{
	platform_driver_unregister(&xr_usb_hsphy_driver);
}
