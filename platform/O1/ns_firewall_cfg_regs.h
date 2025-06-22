// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef NS_FIREWALL_CFG_REGS_REGIF_H
#define NS_FIREWALL_CFG_REGS_REGIF_H

#define NS_FIREWALL_CFG_REGS_NS_FIREWALL0 0x0000
#define NS_FIREWALL_CFG_REGS_NS_FIREWALL0_NS_FIREWALL_CLEAR_SHIFT      0
#define NS_FIREWALL_CFG_REGS_NS_FIREWALL1 0x0004
#define NS_FIREWALL_CFG_REGS_NS_FIREWALL1_NS_FIREWALL_FAIL_MID_SHIFT   0
#define NS_FIREWALL_CFG_REGS_NS_FIREWALL1_NS_FIREWALL_FAIL_MID_MASK    0x000000ff
#define NS_FIREWALL_CFG_REGS_NS_FIREWALL1_NS_FIREWALL_PERROR_SHIFT     8
#define NS_FIREWALL_CFG_REGS_NS_FIREWALL1_NS_FIREWALL_PERROR_MASK      0x00000100
#define NS_FIREWALL_CFG_REGS_NS_FIREWALL2 0x0008


typedef union {
    unsigned int val;
    struct {
        unsigned int ns_firewall_clear :  1;
        unsigned int reserved_0        : 31;
    } reg;
}ns_firewall_cfg_regs_ns_firewall0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ns_firewall_fail_mid :  8;
        unsigned int ns_firewall_perror   :  1;
        unsigned int reserved_0           : 23;
    } reg;
}ns_firewall_cfg_regs_ns_firewall1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ns_firewall_fail_paddr : 32;
    } reg;
}ns_firewall_cfg_regs_ns_firewall2_t;

#endif
