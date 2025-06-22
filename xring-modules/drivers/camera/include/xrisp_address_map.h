/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_ADDRESS_MAP_H__
#define __XRING_ADDRESS_MAP_H__
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>

#define XRISP_M2_CRG_BASE			ACPU_M2_CRG /* 4KB */
#define XRISP_M2_CRG_SIZE			0x1000

#define XRISP_ISP_MCU_CTRL_BASE			ACPU_ISP_MCU_CTRL /* 4KB */
#define XRISP_ISP_MCU_CTRL_SIZE			0x1000

#define XRISP_ISP_CRG_BASE			ACPU_ISP_CRG /* 4KB */
#define XRISP_ISP_CRG_SIZE			0x1000

#define XRISP_ISP_SC_BASE			ACPU_ISP_SC /* 4KB */
#define XRISP_ISP_SC_SIZE			0x1000

#define XRISP_ISP_NIC_BASE			ACPU_CMD_DMA /* 1MB */
#define XRISP_ISP_NIC_SIZE			0x100000

#define XRISP_PERI_CRG_BASE			ACPU_PERI_CRG /* 4KB */
#define XRISP_PERI_CRG_SIZE			(0x1000)

#define XRISP_M2_CTRL_BASE			ACPU_M2_CTRL /* 32KB */
#define XRISP_M2_CTRL_SIZE			(0x8000)

#define XRISP_LPIS_ACTRL_BASE			ACPU_LPIS_ACTRL /* 4KB */
#define XRISP_LPIS_ACTRL_SIZE			(0x1000)

#define XRISP_PCTRL_BASE			ACPU_PCTRL /* 8KB */
#define XRISP_PCTRL_SIZE			(0x2000)

#endif /* __XRING_ADDRESS_MAP_H__ */
