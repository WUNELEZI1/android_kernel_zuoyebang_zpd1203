// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef SYSCNT_RW_REG_REGIF_H
#define SYSCNT_RW_REG_REGIF_H

#define SYSCNT_RW_REG_CTRL_CNTCR                0x0000
#define SYSCNT_RW_REG_CTRL_CNTCR_RF_CNT_ENABLE_SHIFT  0
#define SYSCNT_RW_REG_CTRL_CNTCR_RF_CNT_ENABLE_MASK   0x00000001
#define SYSCNT_RW_REG_CTRL_CNTCR_RF_HALT_ENABLE_SHIFT 1
#define SYSCNT_RW_REG_CTRL_CNTCR_RF_HALT_ENABLE_MASK  0x00000002
#define SYSCNT_RW_REG_CTRL_CNTSR                0x0004
#define SYSCNT_RW_REG_CTRL_CNTSR_HALT_STATUS_SHIFT    1
#define SYSCNT_RW_REG_CTRL_CNTSR_HALT_STATUS_MASK     0x00000002
#define SYSCNT_RW_REG_CTRL_CNTCVL               0x0008
#define SYSCNT_RW_REG_CTRL_CNTCVU               0x000c
#define SYSCNT_RW_REG_FREQ_SCALE                0x0010
#define SYSCNT_RW_REG_STEP_CYCLE                0x0014
#define SYSCNT_RW_REG_STEP_CYCLE_CYCLE_NUM_SHIFT      0
#define SYSCNT_RW_REG_STEP_CYCLE_CYCLE_NUM_MASK       0x0000ffff
#define SYSCNT_RW_REG_CTRL_CNTFID0              0x0020
#define SYSCNT_RW_REG_CGBYPASS                  0x0024
#define SYSCNT_RW_REG_CGBYPASS_CGBYPASS_SHIFT         0
#define SYSCNT_RW_REG_CGBYPASS_CGBYPASS_MASK          0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int rf_cnt_enable  :  1;
        unsigned int rf_halt_enable :  1;
        unsigned int reserved_0     : 30;
    } reg;
}syscnt_rw_reg_ctrl_cntcr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0  :  1;
        unsigned int halt_status :  1;
        unsigned int reserved_1  : 30;
    } reg;
}syscnt_rw_reg_ctrl_cntsr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cntvalue_l : 32;
    } reg;
}syscnt_rw_reg_ctrl_cntcvl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cntvalue_h : 32;
    } reg;
}syscnt_rw_reg_ctrl_cntcvu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int scale_num  : 32;
    } reg;
}syscnt_rw_reg_freq_scale_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cycle_num  : 16;
        unsigned int reserved_0 : 16;
    } reg;
}syscnt_rw_reg_step_cycle_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cnt_frequency : 32;
    } reg;
}syscnt_rw_reg_ctrl_cntfid0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cgbypass   :  1;
        unsigned int reserved_0 : 31;
    } reg;
}syscnt_rw_reg_cgbypass_t;

#endif
