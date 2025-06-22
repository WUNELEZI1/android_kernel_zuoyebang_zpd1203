/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/pci.h>

int xring_pcie_probe_port_by_port(int port);
int xring_pcie_remove_port_by_port(int port);
int xring_pcie_probe_port_by_name(char *ep_name);
int xring_pcie_remove_port_by_name(char *ep_name);

int xring_pcie_soft_on(struct pci_bus *bus);
int xring_pcie_soft_off(struct pci_bus *bus);

void xring_pcie_pm_runtime_allow(int port, bool en);

int xring_pcie_disable_data_trans(int port);

int xring_pcie_set_speed(struct pci_dev *dev, int speed);
int xring_pcie_app_l1sub_cfg(int port, bool enable);
