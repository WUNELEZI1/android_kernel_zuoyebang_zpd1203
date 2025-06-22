/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 XiaoMi, Inc. All rights reserved.
 */

#ifndef _MI_PANEL_ID_H_
#define _MI_PANEL_ID_H_

#include <linux/types.h>
#include "dsi_panel.h"

/*
 * Naming Rules,Wiki Dec .
 * 4Byte : Project ASCII Value .Exemple "L18" ASCII Is 004C3138
 * 1Byte : Prim Panel Is 'P' ASCII Value , Sec Panel Is 'S' Value
 * 1Byte : Panel Vendor
 * 1Byte : DDIC Vendor ,Samsung 0x0C Novatek 0x02
 * 1Byte : Production Batch Num
 */
#define O2S_42_02_0A_PANEL_ID  0x004F32530042020A
#define O80_42_02_0B_PANEL_ID  0x007F38300042020B

/*
 * PA: Primary display, First selection screen
 * PB: Primary display, Second selection screen
 * SA: Secondary display, First selection screen
 * SB: Secondary display, Second selection screen
 */
enum mi_project_panel_id {
	PANEL_ID_INVALID = 0,
	O2S_PANEL_PA,
	O80_PANEL_PB,
	PANEL_ID_MAX
};

enum mi_panel_build_id {
	PANEL_P01 = 0x01,
	PANEL_P10 = 0x10,
	PANEL_P11 = 0x11,
	PANEL_P12 = 0x12,
	PANEL_P20 = 0x20,
	PANEL_P21 = 0x21,
	PANEL_MP = 0xAA,
	PANEL_MAX = 0xFF
};

static inline enum mi_panel_build_id mi_get_panel_build_id(u64 build_id)
{
	switch (build_id) {
	case 0x10:
		return PANEL_P01;
	case 0x40:
		return PANEL_P10;
	case 0x50:
		return PANEL_P11;
	default:
		return PANEL_MP;
	}
	return PANEL_MP;
}

static inline enum mi_project_panel_id mi_get_panel_id(u64 mi_panel_id)
{
	switch (mi_panel_id) {
	case O2S_42_02_0A_PANEL_ID:
		return O2S_PANEL_PA;
	case O80_42_02_0B_PANEL_ID:
		return O80_PANEL_PB;
	default:
		return PANEL_ID_INVALID;
	}
}

static inline const char *mi_get_panel_id_name(u64 mi_panel_id)
{
	switch (mi_get_panel_id(mi_panel_id)) {
	case O2S_PANEL_PA:
		return "O2S_PANEL_PA";
	case O80_PANEL_PB:
		return "O80_PANEL_PB";
	default:
		return "unknown";
	}
}

enum mi_project_panel_id mi_get_panel_id_by_dsi_panel(struct dsi_panel *panel);
enum mi_project_panel_id mi_get_panel_id_by_disp_id(int disp_id);

#endif /* _MI_PANEL_ID_H_ */
