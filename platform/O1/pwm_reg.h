// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef PWM_REGIF_H
#define PWM_REGIF_H

#define PWM_PWM_CR     0x0000
#define PWM_PWM_CR_PWM_LEV_SHIFT               0
#define PWM_PWM_CR_PWM_LEV_MASK                0x00000001
#define PWM_PWM_CR_PWM_EN_SHIFT                1
#define PWM_PWM_CR_PWM_EN_MASK                 0x00000002
#define PWM_PWM_PCVR   0x0004
#define PWM_PWM_PCVR_PWM_PERIOD_SHIFT          0
#define PWM_PWM_PCVR_PWM_PERIOD_MASK           0x003fffff
#define PWM_PWM_SCVR   0x0008
#define PWM_PWM_SCVR_STEP_PERIOD_SHIFT         0
#define PWM_PWM_SCVR_STEP_PERIOD_MASK          0x0000ffff
#define PWM_PWM_PWPCVR 0x000c
#define PWM_PWM_PWPCVR_PULSE_WIDTH_VALUE_SHIFT 0
#define PWM_PWM_PWPCVR_PULSE_WIDTH_VALUE_MASK  0x00000fff


#endif
