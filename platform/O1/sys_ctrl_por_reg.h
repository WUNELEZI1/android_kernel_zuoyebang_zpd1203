// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef SYS_CTRL_POR_REG_REGIF_H
#define SYS_CTRL_POR_REG_REGIF_H

#define SYS_CTRL_POR_REG_SC_REG_USEC0 0x0700
#define SYS_CTRL_POR_REG_SC_REG_USEC0_SC_REG_USEC0_SHIFT 0
#define SYS_CTRL_POR_REG_SC_REG_USEC0_SC_REG_USEC0_MASK  0x0000ffff
#define SYS_CTRL_POR_REG_SC_REG_USEC0__BM__SHIFT         16
#define SYS_CTRL_POR_REG_SC_REG_USEC0__BM__MASK          0xffff0000
#define SYS_CTRL_POR_REG_SC_REG_USEC1 0x0704
#define SYS_CTRL_POR_REG_SC_REG_USEC1_SC_REG_USEC1_SHIFT 0
#define SYS_CTRL_POR_REG_SC_REG_USEC1_SC_REG_USEC1_MASK  0x0000ffff
#define SYS_CTRL_POR_REG_SC_REG_USEC1__BM__SHIFT         16
#define SYS_CTRL_POR_REG_SC_REG_USEC1__BM__MASK          0xffff0000
#define SYS_CTRL_POR_REG_SC_REG_USEC2 0x0708
#define SYS_CTRL_POR_REG_SC_REG_USEC2_SC_REG_USEC2_SHIFT 0
#define SYS_CTRL_POR_REG_SC_REG_USEC2_SC_REG_USEC2_MASK  0x0000ffff
#define SYS_CTRL_POR_REG_SC_REG_USEC2__BM__SHIFT         16
#define SYS_CTRL_POR_REG_SC_REG_USEC2__BM__MASK          0xffff0000
#define SYS_CTRL_POR_REG_SC_REG_USEC3 0x070c
#define SYS_CTRL_POR_REG_SC_REG_USEC3_SC_REG_USEC3_SHIFT 0
#define SYS_CTRL_POR_REG_SC_REG_USEC3_SC_REG_USEC3_MASK  0x0000ffff
#define SYS_CTRL_POR_REG_SC_REG_USEC3__BM__SHIFT         16
#define SYS_CTRL_POR_REG_SC_REG_USEC3__BM__MASK          0xffff0000
#define SYS_CTRL_POR_REG_SC_REG_USEC4 0x0710
#define SYS_CTRL_POR_REG_SC_REG_USEC4_SC_REG_USEC4_SHIFT 0
#define SYS_CTRL_POR_REG_SC_REG_USEC4_SC_REG_USEC4_MASK  0x0000ffff
#define SYS_CTRL_POR_REG_SC_REG_USEC4__BM__SHIFT         16
#define SYS_CTRL_POR_REG_SC_REG_USEC4__BM__MASK          0xffff0000
#define SYS_CTRL_POR_REG_SC_REG_USEC5 0x0714
#define SYS_CTRL_POR_REG_SC_REG_USEC5_SC_REG_USEC5_SHIFT 0
#define SYS_CTRL_POR_REG_SC_REG_USEC5_SC_REG_USEC5_MASK  0x0000ffff
#define SYS_CTRL_POR_REG_SC_REG_USEC5__BM__SHIFT         16
#define SYS_CTRL_POR_REG_SC_REG_USEC5__BM__MASK          0xffff0000
#define SYS_CTRL_POR_REG_SC_REG_USEC6 0x0718
#define SYS_CTRL_POR_REG_SC_REG_USEC6_SC_REG_USEC6_SHIFT 0
#define SYS_CTRL_POR_REG_SC_REG_USEC6_SC_REG_USEC6_MASK  0x0000ffff
#define SYS_CTRL_POR_REG_SC_REG_USEC6__BM__SHIFT         16
#define SYS_CTRL_POR_REG_SC_REG_USEC6__BM__MASK          0xffff0000
#define SYS_CTRL_POR_REG_SC_REG_USEC7 0x071c
#define SYS_CTRL_POR_REG_SC_REG_USEC7_SC_REG_USEC7_SHIFT 0
#define SYS_CTRL_POR_REG_SC_REG_USEC7_SC_REG_USEC7_MASK  0x0000ffff
#define SYS_CTRL_POR_REG_SC_REG_USEC7__BM__SHIFT         16
#define SYS_CTRL_POR_REG_SC_REG_USEC7__BM__MASK          0xffff0000
#define SYS_CTRL_POR_REG_SC_REG_SEC0  0x0f00
#define SYS_CTRL_POR_REG_SC_REG_SEC0_SC_REG_SEC0_SHIFT   0
#define SYS_CTRL_POR_REG_SC_REG_SEC0_SC_REG_SEC0_MASK    0x0000ffff
#define SYS_CTRL_POR_REG_SC_REG_SEC0__BM__SHIFT          16
#define SYS_CTRL_POR_REG_SC_REG_SEC0__BM__MASK           0xffff0000
#define SYS_CTRL_POR_REG_SC_REG_SEC1  0x0f04
#define SYS_CTRL_POR_REG_SC_REG_SEC1_SC_REG_SEC1_SHIFT   0
#define SYS_CTRL_POR_REG_SC_REG_SEC1_SC_REG_SEC1_MASK    0x0000ffff
#define SYS_CTRL_POR_REG_SC_REG_SEC1__BM__SHIFT          16
#define SYS_CTRL_POR_REG_SC_REG_SEC1__BM__MASK           0xffff0000


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_reg_usec0 : 16;
        unsigned int _bm_         : 16;
    } reg;
}sys_ctrl_por_reg_sc_reg_usec0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_reg_usec1 : 16;
        unsigned int _bm_         : 16;
    } reg;
}sys_ctrl_por_reg_sc_reg_usec1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_reg_usec2 : 16;
        unsigned int _bm_         : 16;
    } reg;
}sys_ctrl_por_reg_sc_reg_usec2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_reg_usec3 : 16;
        unsigned int _bm_         : 16;
    } reg;
}sys_ctrl_por_reg_sc_reg_usec3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_reg_usec4 : 16;
        unsigned int _bm_         : 16;
    } reg;
}sys_ctrl_por_reg_sc_reg_usec4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_reg_usec5 : 16;
        unsigned int _bm_         : 16;
    } reg;
}sys_ctrl_por_reg_sc_reg_usec5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_reg_usec6 : 16;
        unsigned int _bm_         : 16;
    } reg;
}sys_ctrl_por_reg_sc_reg_usec6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_reg_usec7 : 16;
        unsigned int _bm_         : 16;
    } reg;
}sys_ctrl_por_reg_sc_reg_usec7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_reg_sec0 : 16;
        unsigned int _bm_        : 16;
    } reg;
}sys_ctrl_por_reg_sc_reg_sec0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_reg_sec1 : 16;
        unsigned int _bm_        : 16;
    } reg;
}sys_ctrl_por_reg_sc_reg_sec1_t;

#endif
