/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc.
 */

#ifndef XRISP_CSIPHY_CTRL_H
#define XRISP_CSIPHY_CTRL_H

int xrisp_csiphy_enable(void);
int xrisp_csiphy_disable(void);
int xrisp_csiphy_disable_force(void);

int xrisp_csiphy_init(void);
void xrisp_csiphy_exit(void);

#endif /* XRISP_CSIPHY_CTRL_H */
