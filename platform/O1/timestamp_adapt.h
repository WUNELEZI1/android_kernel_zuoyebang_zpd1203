// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __TIMESTAMP_ADAPT_H__
#define __TIMESTAMP_ADAPT_H__

#include "syscnt_rw_reg.h"

#define TS_CTRL_CNTCR                   SYSCNT_RW_REG_CTRL_CNTCR
#define TS_RW_CTRL_CNTCVL               SYSCNT_RW_REG_CTRL_CNTCVL
#define TS_RW_CTRL_CNTCVH               SYSCNT_RW_REG_CTRL_CNTCVU
#define TS_CTRL_CNTCVL                  0x1000
#define TS_CTRL_CNTCVH                  0x1004

#endif
