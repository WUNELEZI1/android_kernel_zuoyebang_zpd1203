// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __SHUB_UTIL_H__
#define __SHUB_UTIL_H__

#ifndef BIT
#define BIT(nr)                    (1UL << (nr))
#endif

#ifndef BM__SHIFT
#define BM__SHIFT                  (16)
#endif

#define NO_MASK                    (0x0)

#define SH_CFG_BASE_ADDR           (0xE0804000)
#define SH_CFG_NMI_INTR_ADDR       (SH_CFG_BASE_ADDR + 0x0908)

#define SH_WDT_BASE_ADDR           (0xE0813000)
#define SH_WDT_CRR_OFFSET          (0xc)
#define SH_WDT_CRR_ADDR            (SH_WDT_BASE_ADDR + SH_WDT_CRR_OFFSET)
#define SH_WDT_REST_VAL            (0x76)

#define LPIS_ACTRL_BASE_ADDR       (0xE1508000)
#define LPIS_ACTRL_DFX_SYS_ADDR    (LPIS_ACTRL_BASE_ADDR + 0x0080)
#define SH_RESET_FLAG_OFFSET       (0x4)
#define SH_STATUS_OFFSET           (0x8)

#define SH_WDT_CRR_REGS_SIZE       (4)
#define SH_CFG_DFX_REGS_SIZE       (28)
#define LPIS_ACTRL_DFX_REGS_SIZE   (12)
#define NMI_FLAG_OFFSET            (0x4)
#define NSINTR_MASK_OFFSET         (0x10)

#define SH_CFG_SW_NMI_TRIGGER      (BIT(0x0))

#define NSINTR_MASK_BM_SET         (0x70000)

#define NSINTR_MASK_ALL_USED    (0x7)

#endif
