/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __UFS_XRING_FPGA_MPHY_H__
#define __UFS_XRING_FPGA_MPHY_H__

#include "ufs-xring.h"

#define TEST_DONT_CAL    1
#define TEST_CAL_DONT_LINK   1


#define IS_NORMAL_SEL       0
#define IS_CB_BLOCK_SEL     1
#define IS_SFR_BLOCK_SEL    7

#define IS_TX0_BLOCK_SEL    0x01
#define IS_TX1_BLOCK_SEL    0x02
#define IS_RX0_BLOCK_SEL    0x03
#define IS_RX1_BLOCK_SEL    0x04

#define OCS_5X1503_SLV_ADDR 0xD0


int ufs_xring_setup_mphy_pro_calibration(struct ufs_hba *hba);
int ufs_xring_setup_mphy_pwr_up_seq(struct ufs_hba *hba);
int ufs_xring_mphy_assert_reset(struct ufs_hba *hba);
int ufs_xring_mphy_deassert_reset(struct ufs_hba *hba);
int ufs_xring_mphy_hostcontroller_reset_deassert(struct ufs_hba *hba);
#endif
