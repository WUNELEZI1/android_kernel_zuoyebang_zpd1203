// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef SYS_PM_COMM_HEADER_H
#define SYS_PM_COMM_HEADER_H

#if defined(__KERNEL__) && !defined(__NuttX__)
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>
#include <dt-bindings/xring/platform-specific/sys_ctrl_por_reg.h>
#else
#include "lpis_actrl.h"
#include "sys_ctrl_por_reg.h"
#endif

#define SYS_PM_RSV_REG   LPIS_ACTRL_SC_RSV_NS_8
#define VOTEMNG_SIGN_BIT 4
#define PM_DOZE          0
#define PM_SR            1
#define SCTRL_TICKMARK_REG SYS_CTRL_POR_REG_SC_REG_USEC1

#endif
