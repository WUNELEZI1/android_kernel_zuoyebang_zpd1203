// SPDX-License-Identifier: GPL-2.0-only
/*
 * Xring pcie aspm test module
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/of_device.h>
#include <linux/pci.h>
#include <linux/resource.h>
#include <linux/types.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/version.h>

#include <dt-bindings/xring/platform-specific/flowctrl_cfg_define.h>
#include <dt-bindings/xring/platform-specific/pcie_ctrl.h>
#include <dt-bindings/xring/platform-specific/pcie_phy.h>
#include <dt-bindings/xring/platform-specific/hss2_top.h>
#include "../../pci.h"
#include "pcie-xring.h"
#include "pcie-xring-interface.h"

bool l0s_enable;
bool l1_enable;
bool l1ss_enable;
static unsigned int L12 = 1;
static unsigned int L11 = 1;
static unsigned int pgmode = 1;
#if IS_ENABLED(CONFIG_XRING_DEBUG)
module_param(L12, uint, 0644);
MODULE_PARM_DESC(L12, "L12 Ctrl");
module_param(L11, uint, 0644);
MODULE_PARM_DESC(L11, "L11 Ctrl");
module_param(pgmode, uint, 0644);
MODULE_PARM_DESC(pgmode, "pgmode");
#endif

void xring_pcie_config_phy_pg_mode(struct xring_pcie *xring_pcie, bool enable)
{
	u32 reg;

	if (xring_pcie->is_fpga)
		return;

	if (!pgmode)
		return;

	reg = readl(xring_pcie->pcie_phy + PCIE_PHY_PHY_MISC_CFG);
	reg |= 1 << PCIE_PHY_PHY_MISC_CFG_PG_MODE_EN_SHIFT;
	writel(reg, xring_pcie->pcie_phy + PCIE_PHY_PHY_MISC_CFG);
	dev_info(xring_pcie->pci->dev, "pg_mode_en status end:0x%x\n",
			readl(xring_pcie->pcie_phy + PCIE_PHY_PHY_MISC_CFG));
}

static inline void xring_pcie_config_clear_set_dword(struct pci_dev *pdev,
	int pos, u32 clear, u32 set)
{
	u32 val;

	pci_read_config_dword(pdev, pos, &val);
	val &= ~clear;
	val |= set;
	pci_write_config_dword(pdev, pos, val);
}
static bool xring_pcie_check_aspm_support(struct pci_dev *pdev, int ASPM_STATE)
{
	struct pci_dev *parent = pdev->bus->self;
	u32 val = 0;
	u32 offset = 0;

	switch (ASPM_STATE) {
	case PCI_EXP_LNKCTL_ASPM_L0S:
		offset = BIT(10);
		break;
	case PCI_EXP_LNKCTL_ASPM_L1:
		offset = BIT(11);
		break;
	}
	/* check parent supports L0s */
	if (parent) {
		u32 val2;

		pci_read_config_dword(parent, parent->pcie_cap + PCI_EXP_LNKCAP,
					&val);
		pci_read_config_dword(parent, parent->pcie_cap + PCI_EXP_LNKCTL,
					&val2);
		val = (val & offset) && (val2 & ASPM_STATE);
		if (!val) {
			dev_info(&pdev->dev,
				"Parent PCI device %02x:%02x.%01x does not support L0s\n",
				parent->bus->number,
				PCI_SLOT(parent->devfn),
				PCI_FUNC(parent->devfn));
			return false;
		}
	}
	pci_read_config_dword(pdev, pdev->pcie_cap + PCI_EXP_LNKCAP, &val);
	if (!(val & offset)) {
		dev_info(&pdev->dev,
			"PCI device %02x:%02x.%01x does not support L0s\n",
			pdev->bus->number,
			PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));
		return true;
	}
	return true;
}
static void xring_pcie_config_l0s(struct pci_dev *pdev, bool enable)
{
	struct device *dev = &pdev->dev;
	u32 lnkctl_offset = pdev->pcie_cap + PCI_EXP_LNKCTL;
	int ret;

	dev_info(dev, "PCI device %02x:%02x.%01x %s\n",
		pdev->bus->number, PCI_SLOT(pdev->devfn),
		PCI_FUNC(pdev->devfn), enable ? "enable" : "disable");
	if (enable) {
		ret = xring_pcie_check_aspm_support(pdev, PCI_EXP_LNKCTL_ASPM_L0S);
		if (!ret)
			return;
		xring_pcie_config_clear_set_dword(pdev, lnkctl_offset, 0,
			PCI_EXP_LNKCTL_ASPM_L0S);
	} else {
		xring_pcie_config_clear_set_dword(pdev, lnkctl_offset,
			PCI_EXP_LNKCTL_ASPM_L0S, 0);
	}
}
static void xring_pcie_config_l1(struct pci_dev *pdev, bool enable)
{
	struct device *dev = &pdev->dev;
	u32 lnkctl_offset = pdev->pcie_cap + PCI_EXP_LNKCTL;
	int ret;

	dev_info(dev, "PCI device %02x:%02x.%01x %s\n",
		pdev->bus->number, PCI_SLOT(pdev->devfn),
		PCI_FUNC(pdev->devfn), enable ? "enable" : "disable");
	if (enable) {
		ret = xring_pcie_check_aspm_support(pdev, PCI_EXP_LNKCTL_ASPM_L1);
		if (!ret)
			return;
		xring_pcie_config_clear_set_dword(pdev, lnkctl_offset, 0,
			PCI_EXP_LNKCTL_ASPM_L1);
	} else {
		xring_pcie_config_clear_set_dword(pdev, lnkctl_offset,
			PCI_EXP_LNKCTL_ASPM_L1, 0);
	}
}
static void xring_pcie_config_l1ss(struct pci_dev *pdev, bool enable)
{
	struct device *dev = &pdev->dev;
	u32 val;
	u32 l1ss_cap_id_offset, l1ss_ctl1_offset;

	dev_info(dev, "PCI device %02x:%02x.%01x %s\n",
		 pdev->bus->number, PCI_SLOT(pdev->devfn),
		PCI_FUNC(pdev->devfn), enable ? "enable" : "disable");
	l1ss_cap_id_offset = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_L1SS);
	if (!l1ss_cap_id_offset) {
		dev_info(dev,
			"PCI device %02x:%02x.%01x could not find L1ss capability register\n",
			pdev->bus->number, PCI_SLOT(pdev->devfn),
			PCI_FUNC(pdev->devfn));
		return;
	}
	l1ss_ctl1_offset = l1ss_cap_id_offset + PCI_L1SS_CTL1;
	if (enable) {
		val = PCI_L1SS_CTL1_ASPM_L1_2 | PCI_L1SS_CTL1_ASPM_L1_1;
		if (!L12)
			val	&= ~PCI_L1SS_CTL1_ASPM_L1_2;
		if (!L11)
			val &= ~PCI_L1SS_CTL1_ASPM_L1_1;
		if (!L11 && !L12) {
			dev_info(dev, "%s failed, forget enable L1ss module param", __func__);
			return;
		}
		dev_info(dev, "%s success, L11:%d, L12:%d", __func__, L11, L12);
		xring_pcie_config_clear_set_dword(pdev, l1ss_ctl1_offset, 0,
			val);
	} else {
		xring_pcie_config_clear_set_dword(pdev, l1ss_ctl1_offset,
			PCI_L1SS_CTL1_ASPM_L1_1 | PCI_L1SS_CTL1_ASPM_L1_2, 0);
	}
	pci_read_config_dword(pdev, l1ss_ctl1_offset, &val);
	dev_info(dev, "L1SUB_CONTROL1:0x%x\n", val);
}
static int xring_pcie_config_l1ss_enable(struct pci_dev *pdev, void *dev)
{
	xring_pcie_config_l1ss(pdev, l1ss_enable);
	return 0;
}
static void xring_pcie_config_l1ss_enable_all_device(struct xring_pcie *xring_pcie, bool enable)
{
	struct pci_bus *bus = pci_find_bus(xring_pcie->port_num, 0);

	if (bus) {
		l1ss_enable = enable;
		pci_walk_bus(bus, xring_pcie_config_l1ss_enable, xring_pcie);
	} else {
		dev_err(xring_pcie->pci->dev, "can not find bus>>>>%s\n", __func__);
	}
}
static int xring_pcie_config_l1_enable(struct pci_dev *pdev, void *dev)
{
	xring_pcie_config_l1(pdev, l1_enable);
	return 0;
}
static void xring_pcie_config_l1_enable_all_device(struct xring_pcie *xring_pcie, bool enable)
{
	struct pci_bus *bus = pci_find_bus(xring_pcie->port_num, 0);

	if (bus) {
		l1_enable = enable;
		pci_walk_bus(bus, xring_pcie_config_l1_enable, xring_pcie);
	} else {
		dev_err(xring_pcie->pci->dev, "can not find bus>>>>%s\n", __func__);
	}
}
static int xring_pcie_config_l0s_enable(struct pci_dev *pdev, void *dev)
{
	xring_pcie_config_l0s(pdev, l0s_enable);
	return 0;
}
static void xring_pcie_config_l0s_enable_all_device(struct xring_pcie *xring_pcie, bool enable)
{
	u8 offset = dw_pcie_find_capability(xring_pcie->pci, PCI_CAP_ID_EXP);
	u32 val = 0;
	struct pci_bus *bus = pci_find_bus(xring_pcie->port_num, 0);

	dw_pcie_dbi_ro_wr_en(xring_pcie->pci);
	val = dw_pcie_readl_dbi(xring_pcie->pci, offset + PCI_EXP_LNKCAP);
	if (!(val & PCI_EXP_LNKCAP_ASPM_L0S)) {
		val |= PCI_EXP_LNKCAP_ASPM_L0S;
		dev_info(xring_pcie->pci->dev, "need set l0s support");
		dw_pcie_writel_dbi(xring_pcie->pci, offset + PCI_EXP_LNKCAP, val);
	}
	dw_pcie_dbi_ro_wr_dis(xring_pcie->pci);
	if (bus) {
		l0s_enable = enable;
		pci_walk_bus(bus, xring_pcie_config_l0s_enable, xring_pcie);
	} else {
		dev_err(xring_pcie->pci->dev, "can not find bus>>>>%s\n", __func__);
	}
}


static ssize_t pcie_l0s_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	dev_info(dev, "PCIE l0s_enable:%d", l0s_enable);
	return 0;
}
static ssize_t pcie_l0s_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	u32 enable;
	int ret;
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);

	ret = kstrtoint(buf, 10, &enable);
	xring_pcie_config_l0s_enable_all_device(xring_pcie, enable);
	return count;
}

static ssize_t pcie_l1_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	dev_info(dev, "PCIE l1_enable:%d", l1_enable);
	return 0;
}

static ssize_t pcie_l1_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	u32 enable;
	int ret;
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);

	ret = kstrtoint(buf, 10, &enable);
	xring_pcie_config_l1_enable_all_device(xring_pcie, enable);
	return count;
}

static ssize_t pcie_l1ss_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	dev_info(dev, "PCIE l1ss_enable:%d", l1ss_enable);
	return 0;
}

static ssize_t pcie_l1ss_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	u32 enable;
	int ret;
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);

	ret = kstrtoint(buf, 10, &enable);
	xring_pcie_config_l1ss_enable_all_device(xring_pcie, enable);
	return count;
}

DEVICE_ATTR_RW(pcie_l0s);
DEVICE_ATTR_RW(pcie_l1);
DEVICE_ATTR_RW(pcie_l1ss);

static struct attribute *sys_pcie_aspm_attributes[] = {
	&dev_attr_pcie_l0s.attr,
	&dev_attr_pcie_l1.attr,
	&dev_attr_pcie_l1ss.attr,
	NULL
};
static const struct attribute_group sys_pcie_aspm_attr_group = {
	.attrs = sys_pcie_aspm_attributes,
};

void xring_pcie_lp_attach_sysfs(struct device *dev)
{
	if (sysfs_create_group(&(dev->kobj), &sys_pcie_aspm_attr_group))
		dev_err(dev, "Failed to create pcie aspm group");
	else
		dev_info(dev, "Success to create pcie aspm group");
}

void xring_pcie_lp_detach_sysfs(struct device *dev)
{
	sysfs_remove_group(&(dev->kobj), &sys_pcie_aspm_attr_group);
}
