/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __UFS_XRING_FPGA_H__
#define __UFS_XRING_FPGA_H__
#include "ufshcd.h"

void ufs_xring_mphy_clk_cfg_fpga(struct ufs_hba *hba);
int ufs_xring_fpga_ioremap(struct ufs_hba *hba);

#endif
