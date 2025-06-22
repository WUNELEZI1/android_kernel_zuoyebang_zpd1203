// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef SYSCNT_FASTREAD_REG_REGIF_H
#define SYSCNT_FASTREAD_REG_REGIF_H

#define SYSCNT_FASTREAD_REG_CNT_VALUE_L                           0x0000
#define SYSCNT_FASTREAD_REG_CNT_VALUE_H                           0x0004
#define SYSCNT_FASTREAD_REG_IRQ_IRQ_INT_RAW                       0x0800
#define SYSCNT_FASTREAD_REG_IRQ_IRQ_INT_RAW_TSFORCESYNC_RAW_SHIFT      0
#define SYSCNT_FASTREAD_REG_IRQ_IRQ_INT_FORCE                     0x0804
#define SYSCNT_FASTREAD_REG_IRQ_IRQ_INT_FORCE_TSFORCESYNC_RAW_SHIFT    0
#define SYSCNT_FASTREAD_REG_IRQ_IRQ_INT_MASK                      0x0808
#define SYSCNT_FASTREAD_REG_IRQ_IRQ_INT_MASK_TSFORCESYNC_MASK_SHIFT    0
#define SYSCNT_FASTREAD_REG_IRQ_IRQ_INT_MASK_TSFORCESYNC_MASK_MASK     0x00000001
#define SYSCNT_FASTREAD_REG_IRQ_IRQ_INT_STATUS                    0x080c
#define SYSCNT_FASTREAD_REG_IRQ_IRQ_INT_STATUS_TSFORCESYNC_STATUS_SHIFT 0
#define SYSCNT_FASTREAD_REG_IRQ_IRQ_INT_STATUS_TSFORCESYNC_STATUS_MASK  0x00000001
#define SYSCNT_FASTREAD_REG_CGBYPASS                              0x0810
#define SYSCNT_FASTREAD_REG_CGBYPASS_CGBYPASS_SHIFT                    0
#define SYSCNT_FASTREAD_REG_CGBYPASS_CGBYPASS_MASK                     0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int cntvalue_l : 32;
    } reg;
}syscnt_fastread_reg_cnt_value_l_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cntvalue_h : 32;
    } reg;
}syscnt_fastread_reg_cnt_value_h_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tsforcesync_raw :  1;
        unsigned int reserved_0      : 31;
    } reg;
}syscnt_fastread_reg_irq_irq_int_raw_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tsforcesync_raw :  1;
        unsigned int reserved_0      : 31;
    } reg;
}syscnt_fastread_reg_irq_irq_int_force_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tsforcesync_mask :  1;
        unsigned int reserved_0       : 31;
    } reg;
}syscnt_fastread_reg_irq_irq_int_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tsforcesync_status :  1;
        unsigned int reserved_0         : 31;
    } reg;
}syscnt_fastread_reg_irq_irq_int_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cgbypass   :  1;
        unsigned int reserved_0 : 31;
    } reg;
}syscnt_fastread_reg_cgbypass_t;

#endif
