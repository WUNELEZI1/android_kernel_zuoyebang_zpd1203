// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __IOC_REG_H__
#define __IOC_REG_H__


#define CFG_PAD_SL_SHIFT	0
#define CFG_PAD_SL_MASK		0x1
#define CFG_PAD_SL_BITS		1


#define CFG_PAD_ST_SHIFT	1
#define CFG_PAD_ST_MASK		0x2
#define CFG_PAD_ST_BITS		1


#define CFG_PAD_PULL_SHIFT	2
#define CFG_PAD_PULL_MASK	0xC
#define CFG_PAD_PULL_BITS	2


#define CFG_PAD_PD_SHIFT	2
#define CFG_PAD_PD_MASK		0x4
#define CFG_PAD_PD_BITS		1


#define CFG_PAD_PU_SHIFT	3
#define CFG_PAD_PU_MASK		0x8
#define CFG_PAD_PU_BITS		1


#define CFG_PAD_DS_SHIFT	4
#define CFG_PAD_DS_MASK		0xF0
#define CFG_PAD_DS_BITS		4


#define CFG_PAD_RESB_SHIFT	4
#define CFG_PAD_RESB_MASK	0xF00


#define MUX_PAD_SEL_SHIFT	0
#define MUX_PAD_SEL_MASK	0x7
#define MUX_PAD_SEL_BITS	3

#endif
