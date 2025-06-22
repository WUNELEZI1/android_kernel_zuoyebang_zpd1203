// SPDX-License-Identifier: GPL-2.0 only.
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/types.h>
#include "mtk_pwrctl_common.h"
#include "mtk_pwrctl_interface.h"
#ifdef CONFIG_WWAN_GPIO_PWRCTL_UT
#include "ut_pwrctl_pcie_fake.h"
#endif

#define PWRCTL_PCIE_PORT_NUM		(1)
#define PWRCTL_PCIE_PIN_OFF		(4)

/*
static __attribute__ ((weakref("mtk_pcie_probe_port"))) int mtk_pwrctl_pcie_probe_port(int port);
static __attribute__ ((weakref("mtk_pcie_remove_port"))) int mtk_pwrctl_pcie_remove_port(int port);

static __attribute__ ((weakref("mtk_pcie_soft_on"))) \
		       int mtk_pwrctl_pcie_soft_on(struct pci_bus *bus);
static __attribute__ ((weakref("mtk_pcie_soft_off"))) \
		       int mtk_pwrctl_pcie_soft_off(struct pci_bus *bus);
*/
/*
static __attribute__ ((weakref("mtk_pcie_disable_refclk"))) \
			int mtk_pwrctl_pcie_disabel_refclk(int port);
static __attribute__ ((weakref("mtk_pcie_pinmux_select"))) \
			int mtk_pwrctl_pcie_pinmux_select(int port_num, int state);
static __attribute__ ((weakref("mtk_pcie_disable_data_trans"))) \
			int mtk_pwrctl_pcie_disable_data_trans(int port);
*/
/*
#define MTK_PCIE_PROBE_PORT(port) \
	do { if (mtk_pwrctl_pcie_probe_port) mtk_pwrctl_pcie_probe_port(port); } while(0)
#define MTK_PCIE_REMOVE_PORT(port) \
	do { if (mtk_pwrctl_pcie_remove_port)  mtk_pwrctl_pcie_remove_port(port); } while(0)

#define MTK_PCIE_SOFT_ON(bus) \
	do { if (mtk_pwrctl_pcie_soft_on) mtk_pwrctl_pcie_soft_on(bus); } while(0)
#define MTK_PCIE_SOFT_OFF(bus) \
	do { if (mtk_pwrctl_pcie_soft_off) mtk_pwrctl_pcie_soft_off(bus); } while(0)
*/
/*
#define MTK_PCIE_DISABLE_REFCLK(port) \
	do { if (mtk_pwrctl_pcie_disabel_refclk) mtk_pwrctl_pcie_disabel_refclk(port); } while(0)
#define MTK_PCIE_PIN_SELECT(port, state) \
	do { if (mtk_pwrctl_pcie_pinmux_select) mtk_pwrctl_pcie_pinmux_select(port, state); } \
	while(0)
#define MTK_PCIE_DISABLE_DATA_TRANS(port) \
	do { if (mtk_pwrctl_pcie_disable_data_trans) mtk_pwrctl_pcie_disable_data_trans(port);} \
	while(0)
*/
extern int xring_pcie_soft_on(struct pci_bus *bus);
extern int xring_pcie_soft_off(struct pci_bus *bus);
extern int xring_pcie_probe_port_by_port(int port);
extern int xring_pcie_remove_port_by_port(int port);

int mtk_pwrctl_request_to_controller(struct pwrctl_mdev *mdev, int type)
{
	struct pci_bus *bus;

	bus = pci_find_bus(0, 0);
	switch(type) {
		case PWRCTL_CONTROLLER_REQUEST_SOFT_OFF:
			if (bus)
				xring_pcie_soft_off(bus);
			else
				pr_err("pwrctl: not found pcie bus of port 1\n");
			break;
		case PWRCTL_CONTROLLER_REQUEST_SOFT_ON:
			if (bus)
				xring_pcie_soft_on(bus);
			else
				pr_err("pwrctl: not found pcie bus of port 1\n");
			break;
		case PWRCTL_CONTROLLER_REQUEST_SCAN_PORT:
			xring_pcie_probe_port_by_port(0);
			pr_info("pwrctl: trigger controller probe device done\n");
			break;
		case PWRCTL_CONTROLLER_REQUEST_REMOVE_PORT:
			xring_pcie_remove_port_by_port(0);
			pr_info("pwrctl: trigger controller remove device done\n");
			break;
		default:
			break;
	}
	return 0;
}

/*
int mtk_pwrctl_controller_pin_off(struct pwrctl_mdev *mdev)
{
	MTK_PCIE_PIN_SELECT(PWRCTL_PCIE_PORT_NUM, PWRCTL_PCIE_PIN_OFF);
	pr_info("pwrctl: set controller pin off done\n");
	return 0;
}

int mtk_pwrctl_controller_disable_refclk(struct pwrctl_mdev *mdev)
{
	MTK_PCIE_DISABLE_REFCLK(PWRCTL_PCIE_PORT_NUM);
	pr_info("pwrctl: disable controller refclk done\n");
	return 0;
}

int mtk_pwrctl_controller_disable_data_trans(struct pwrctl_mdev *mdev)
{
	MTK_PCIE_DISABLE_DATA_TRANS(PWRCTL_PCIE_PORT_NUM);
	pr_info("pwrctl: disable trans data\n");
	return 0;
}
*/