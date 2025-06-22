// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef MAINBUS_AUTOGT_H
#define MAINBUS_AUTOGT_H

#include "../peri_crg.h"
#include "../fcm_acpu_address_map.h"


#define MAINBUS_AUTOGT_VOTE_OFFSET               PERI_CRG_AUTOFSCTRL27

#define MAINBUS_AUTOGT_GPU_POWER_BIT             0
#define MAINBUS_AUTOGT_M1_POWER_BIT              1
#define MAINBUS_AUTOGT_M2_POWER_BIT              2
#define MAINBUS_AUTOGT_XRSE_POWER_BIT            3
#define MAINBUS_AUTOGT_NPU_POWER_BIT             4
#define MAINBUS_AUTOGT_CPU_POWER_BIT             5
#define MAINBUS_AUTOGT_MREGION_SETTING_BIT       6
#define MAINBUS_AUTOGT_ETR_NOC_DFX_BIT           7
#define MAINBUS_AUTOGT_DDR_SUBSYS_POWER_BIT      8
#endif
