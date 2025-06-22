// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef ISP_CRG_REGIF_H
#define ISP_CRG_REGIF_H

#define ISP_CRG_ISP_CLOCK_CG_0_W1S         0x0000
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP_CMDDMA_SHIFT          0
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP_GPDMA_SHIFT           1
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_ACLK_ISP_CVE_SHIFT            2
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_ACLK_ISP_PE_SHIFT             3
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_ACLK_ISP_BE_SHIFT             4
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_ACLK_ISP_FE_CORE2_SHIFT       5
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_ACLK_ISP_FE_CORE1_SHIFT       6
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_ACLK_ISP_FE_CORE0_SHIFT       7
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_ACLK_ISP_FE_ROUTER_SHIFT      8
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP2CSI_FE_SHIFT          9
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP_CFG_NIC_SHIFT         10
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP_CVE_SHIFT             11
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP_PE_SHIFT              12
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP_BE_SHIFT              13
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP_FE_CORE2_SHIFT        14
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP_FE_CORE1_SHIFT        15
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP_FE_CORE0_SHIFT        16
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_CLK_ISP_FE_ROUTER_SHIFT       17
#define ISP_CRG_ISP_CLOCK_CG_0_W1S_GT_PCLK_ISP_GPDMA_SHIFT          18
#define ISP_CRG_ISP_CLOCK_CG_0_W1C         0x0004
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP_CMDDMA_SHIFT          0
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP_GPDMA_SHIFT           1
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_ACLK_ISP_CVE_SHIFT            2
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_ACLK_ISP_PE_SHIFT             3
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_ACLK_ISP_BE_SHIFT             4
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_ACLK_ISP_FE_CORE2_SHIFT       5
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_ACLK_ISP_FE_CORE1_SHIFT       6
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_ACLK_ISP_FE_CORE0_SHIFT       7
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_ACLK_ISP_FE_ROUTER_SHIFT      8
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP2CSI_FE_SHIFT          9
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP_CFG_NIC_SHIFT         10
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP_CVE_SHIFT             11
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP_PE_SHIFT              12
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP_BE_SHIFT              13
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP_FE_CORE2_SHIFT        14
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP_FE_CORE1_SHIFT        15
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP_FE_CORE0_SHIFT        16
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_CLK_ISP_FE_ROUTER_SHIFT       17
#define ISP_CRG_ISP_CLOCK_CG_0_W1C_GT_PCLK_ISP_GPDMA_SHIFT          18
#define ISP_CRG_ISP_CLOCK_CG_0_RO          0x0008
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_CMDDMA_SHIFT           0
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_CMDDMA_MASK            0x00000001
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_GPDMA_SHIFT            1
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_GPDMA_MASK             0x00000002
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_CVE_SHIFT             2
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_CVE_MASK              0x00000004
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_PE_SHIFT              3
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_PE_MASK               0x00000008
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_BE_SHIFT              4
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_BE_MASK               0x00000010
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_FE_CORE2_SHIFT        5
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_FE_CORE2_MASK         0x00000020
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_FE_CORE1_SHIFT        6
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_FE_CORE1_MASK         0x00000040
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_FE_CORE0_SHIFT        7
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_FE_CORE0_MASK         0x00000080
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_FE_ROUTER_SHIFT       8
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_ACLK_ISP_FE_ROUTER_MASK        0x00000100
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP2CSI_FE_SHIFT           9
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP2CSI_FE_MASK            0x00000200
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_CFG_NIC_SHIFT          10
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_CFG_NIC_MASK           0x00000400
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_CVE_SHIFT              11
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_CVE_MASK               0x00000800
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_PE_SHIFT               12
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_PE_MASK                0x00001000
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_BE_SHIFT               13
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_BE_MASK                0x00002000
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_FE_CORE2_SHIFT         14
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_FE_CORE2_MASK          0x00004000
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_FE_CORE1_SHIFT         15
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_FE_CORE1_MASK          0x00008000
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_FE_CORE0_SHIFT         16
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_FE_CORE0_MASK          0x00010000
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_FE_ROUTER_SHIFT        17
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_CLK_ISP_FE_ROUTER_MASK         0x00020000
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_PCLK_ISP_GPDMA_SHIFT           18
#define ISP_CRG_ISP_CLOCK_CG_0_RO_GT_PCLK_ISP_GPDMA_MASK            0x00040000
#define ISP_CRG_ISP_CLOCK_CG_ST_0          0x000c
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_CMDDMA_SHIFT           0
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_CMDDMA_MASK            0x00000001
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_GPDMA_SHIFT            1
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_GPDMA_MASK             0x00000002
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_CVE_SHIFT             2
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_CVE_MASK              0x00000004
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_PE_SHIFT              3
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_PE_MASK               0x00000008
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_BE_SHIFT              4
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_BE_MASK               0x00000010
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_FE_CORE2_SHIFT        5
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_FE_CORE2_MASK         0x00000020
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_FE_CORE1_SHIFT        6
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_FE_CORE1_MASK         0x00000040
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_FE_CORE0_SHIFT        7
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_FE_CORE0_MASK         0x00000080
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_FE_ROUTER_SHIFT       8
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_ACLK_ISP_FE_ROUTER_MASK        0x00000100
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP2CSI_FE_SHIFT           9
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP2CSI_FE_MASK            0x00000200
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_CFG_NIC_SHIFT          10
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_CFG_NIC_MASK           0x00000400
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_CVE_SHIFT              11
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_CVE_MASK               0x00000800
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_PE_SHIFT               12
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_PE_MASK                0x00001000
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_BE_SHIFT               13
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_BE_MASK                0x00002000
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_FE_CORE2_SHIFT         14
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_FE_CORE2_MASK          0x00004000
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_FE_CORE1_SHIFT         15
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_FE_CORE1_MASK          0x00008000
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_FE_CORE0_SHIFT         16
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_FE_CORE0_MASK          0x00010000
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_FE_ROUTER_SHIFT        17
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_CLK_ISP_FE_ROUTER_MASK         0x00020000
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_PCLK_ISP_GPDMA_SHIFT           18
#define ISP_CRG_ISP_CLOCK_CG_ST_0_ST_PCLK_ISP_GPDMA_MASK            0x00040000
#define ISP_CRG_ISP_CLOCK_RST_0_W1S        0x0010
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_CMDDMA_CFG_N_SHIFT   0
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_CVE_CFG_N_SHIFT      1
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_PE_CFG_N_SHIFT       2
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_BE_CFG_N_SHIFT       3
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_FE_CORE2_CFG_N_SHIFT 4
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_FE_CORE1_CFG_N_SHIFT 5
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_FE_CORE0_CFG_N_SHIFT 6
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_FE_ROUTER_CFG_N_SHIFT 7
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_CFG_NIC_N_SHIFT      8
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_GPDMA_N_SHIFT        9
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_CMDDMA_N_SHIFT       10
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_CVE_N_SHIFT          11
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_PE_N_SHIFT           12
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_BE_N_SHIFT           13
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP2CSI_FE_N_SHIFT       14
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_FE_CORE2_N_SHIFT     15
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_FE_CORE1_N_SHIFT     16
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_FE_CORE0_N_SHIFT     17
#define ISP_CRG_ISP_CLOCK_RST_0_W1S_IP_RST_ISP_FE_ROUTER_N_SHIFT    18
#define ISP_CRG_ISP_CLOCK_RST_0_W1C        0x0014
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_CMDDMA_CFG_N_SHIFT   0
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_CVE_CFG_N_SHIFT      1
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_PE_CFG_N_SHIFT       2
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_BE_CFG_N_SHIFT       3
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_FE_CORE2_CFG_N_SHIFT 4
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_FE_CORE1_CFG_N_SHIFT 5
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_FE_CORE0_CFG_N_SHIFT 6
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_FE_ROUTER_CFG_N_SHIFT 7
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_CFG_NIC_N_SHIFT      8
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_GPDMA_N_SHIFT        9
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_CMDDMA_N_SHIFT       10
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_CVE_N_SHIFT          11
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_PE_N_SHIFT           12
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_BE_N_SHIFT           13
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP2CSI_FE_N_SHIFT       14
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_FE_CORE2_N_SHIFT     15
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_FE_CORE1_N_SHIFT     16
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_FE_CORE0_N_SHIFT     17
#define ISP_CRG_ISP_CLOCK_RST_0_W1C_IP_RST_ISP_FE_ROUTER_N_SHIFT    18
#define ISP_CRG_ISP_CLOCK_RST_0_RO         0x0018
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_CMDDMA_CFG_N_SHIFT    0
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_CMDDMA_CFG_N_MASK     0x00000001
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_CVE_CFG_N_SHIFT       1
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_CVE_CFG_N_MASK        0x00000002
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_PE_CFG_N_SHIFT        2
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_PE_CFG_N_MASK         0x00000004
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_BE_CFG_N_SHIFT        3
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_BE_CFG_N_MASK         0x00000008
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE2_CFG_N_SHIFT  4
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE2_CFG_N_MASK   0x00000010
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE1_CFG_N_SHIFT  5
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE1_CFG_N_MASK   0x00000020
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE0_CFG_N_SHIFT  6
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE0_CFG_N_MASK   0x00000040
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_ROUTER_CFG_N_SHIFT 7
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_ROUTER_CFG_N_MASK  0x00000080
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_CFG_NIC_N_SHIFT       8
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_CFG_NIC_N_MASK        0x00000100
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_GPDMA_N_SHIFT         9
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_GPDMA_N_MASK          0x00000200
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_CMDDMA_N_SHIFT        10
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_CMDDMA_N_MASK         0x00000400
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_CVE_N_SHIFT           11
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_CVE_N_MASK            0x00000800
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_PE_N_SHIFT            12
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_PE_N_MASK             0x00001000
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_BE_N_SHIFT            13
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_BE_N_MASK             0x00002000
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP2CSI_FE_N_SHIFT        14
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP2CSI_FE_N_MASK         0x00004000
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE2_N_SHIFT      15
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE2_N_MASK       0x00008000
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE1_N_SHIFT      16
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE1_N_MASK       0x00010000
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE0_N_SHIFT      17
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_CORE0_N_MASK       0x00020000
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_ROUTER_N_SHIFT     18
#define ISP_CRG_ISP_CLOCK_RST_0_RO_IP_RST_ISP_FE_ROUTER_N_MASK      0x00040000
#define ISP_CRG_REGFILE_CGBYPASS           0x001c
#define ISP_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT             0
#define ISP_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK              0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_isp_cmddma     :  1;
        unsigned int gt_clk_isp_gpdma      :  1;
        unsigned int gt_aclk_isp_cve       :  1;
        unsigned int gt_aclk_isp_pe        :  1;
        unsigned int gt_aclk_isp_be        :  1;
        unsigned int gt_aclk_isp_fe_core2  :  1;
        unsigned int gt_aclk_isp_fe_core1  :  1;
        unsigned int gt_aclk_isp_fe_core0  :  1;
        unsigned int gt_aclk_isp_fe_router :  1;
        unsigned int gt_clk_isp2csi_fe     :  1;
        unsigned int gt_clk_isp_cfg_nic    :  1;
        unsigned int gt_clk_isp_cve        :  1;
        unsigned int gt_clk_isp_pe         :  1;
        unsigned int gt_clk_isp_be         :  1;
        unsigned int gt_clk_isp_fe_core2   :  1;
        unsigned int gt_clk_isp_fe_core1   :  1;
        unsigned int gt_clk_isp_fe_core0   :  1;
        unsigned int gt_clk_isp_fe_router  :  1;
        unsigned int gt_pclk_isp_gpdma     :  1;
        unsigned int reserved_0            : 13;
    } reg;
}isp_crg_isp_clock_cg_0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_isp_cmddma     :  1;
        unsigned int gt_clk_isp_gpdma      :  1;
        unsigned int gt_aclk_isp_cve       :  1;
        unsigned int gt_aclk_isp_pe        :  1;
        unsigned int gt_aclk_isp_be        :  1;
        unsigned int gt_aclk_isp_fe_core2  :  1;
        unsigned int gt_aclk_isp_fe_core1  :  1;
        unsigned int gt_aclk_isp_fe_core0  :  1;
        unsigned int gt_aclk_isp_fe_router :  1;
        unsigned int gt_clk_isp2csi_fe     :  1;
        unsigned int gt_clk_isp_cfg_nic    :  1;
        unsigned int gt_clk_isp_cve        :  1;
        unsigned int gt_clk_isp_pe         :  1;
        unsigned int gt_clk_isp_be         :  1;
        unsigned int gt_clk_isp_fe_core2   :  1;
        unsigned int gt_clk_isp_fe_core1   :  1;
        unsigned int gt_clk_isp_fe_core0   :  1;
        unsigned int gt_clk_isp_fe_router  :  1;
        unsigned int gt_pclk_isp_gpdma     :  1;
        unsigned int reserved_0            : 13;
    } reg;
}isp_crg_isp_clock_cg_0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_isp_cmddma     :  1;
        unsigned int gt_clk_isp_gpdma      :  1;
        unsigned int gt_aclk_isp_cve       :  1;
        unsigned int gt_aclk_isp_pe        :  1;
        unsigned int gt_aclk_isp_be        :  1;
        unsigned int gt_aclk_isp_fe_core2  :  1;
        unsigned int gt_aclk_isp_fe_core1  :  1;
        unsigned int gt_aclk_isp_fe_core0  :  1;
        unsigned int gt_aclk_isp_fe_router :  1;
        unsigned int gt_clk_isp2csi_fe     :  1;
        unsigned int gt_clk_isp_cfg_nic    :  1;
        unsigned int gt_clk_isp_cve        :  1;
        unsigned int gt_clk_isp_pe         :  1;
        unsigned int gt_clk_isp_be         :  1;
        unsigned int gt_clk_isp_fe_core2   :  1;
        unsigned int gt_clk_isp_fe_core1   :  1;
        unsigned int gt_clk_isp_fe_core0   :  1;
        unsigned int gt_clk_isp_fe_router  :  1;
        unsigned int gt_pclk_isp_gpdma     :  1;
        unsigned int reserved_0            : 13;
    } reg;
}isp_crg_isp_clock_cg_0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_isp_cmddma     :  1;
        unsigned int st_clk_isp_gpdma      :  1;
        unsigned int st_aclk_isp_cve       :  1;
        unsigned int st_aclk_isp_pe        :  1;
        unsigned int st_aclk_isp_be        :  1;
        unsigned int st_aclk_isp_fe_core2  :  1;
        unsigned int st_aclk_isp_fe_core1  :  1;
        unsigned int st_aclk_isp_fe_core0  :  1;
        unsigned int st_aclk_isp_fe_router :  1;
        unsigned int st_clk_isp2csi_fe     :  1;
        unsigned int st_clk_isp_cfg_nic    :  1;
        unsigned int st_clk_isp_cve        :  1;
        unsigned int st_clk_isp_pe         :  1;
        unsigned int st_clk_isp_be         :  1;
        unsigned int st_clk_isp_fe_core2   :  1;
        unsigned int st_clk_isp_fe_core1   :  1;
        unsigned int st_clk_isp_fe_core0   :  1;
        unsigned int st_clk_isp_fe_router  :  1;
        unsigned int st_pclk_isp_gpdma     :  1;
        unsigned int reserved_0            : 13;
    } reg;
}isp_crg_isp_clock_cg_st_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_isp_cmddma_cfg_n    :  1;
        unsigned int ip_rst_isp_cve_cfg_n       :  1;
        unsigned int ip_rst_isp_pe_cfg_n        :  1;
        unsigned int ip_rst_isp_be_cfg_n        :  1;
        unsigned int ip_rst_isp_fe_core2_cfg_n  :  1;
        unsigned int ip_rst_isp_fe_core1_cfg_n  :  1;
        unsigned int ip_rst_isp_fe_core0_cfg_n  :  1;
        unsigned int ip_rst_isp_fe_router_cfg_n :  1;
        unsigned int ip_rst_isp_cfg_nic_n       :  1;
        unsigned int ip_rst_isp_gpdma_n         :  1;
        unsigned int ip_rst_isp_cmddma_n        :  1;
        unsigned int ip_rst_isp_cve_n           :  1;
        unsigned int ip_rst_isp_pe_n            :  1;
        unsigned int ip_rst_isp_be_n            :  1;
        unsigned int ip_rst_isp2csi_fe_n        :  1;
        unsigned int ip_rst_isp_fe_core2_n      :  1;
        unsigned int ip_rst_isp_fe_core1_n      :  1;
        unsigned int ip_rst_isp_fe_core0_n      :  1;
        unsigned int ip_rst_isp_fe_router_n     :  1;
        unsigned int reserved_0                 : 13;
    } reg;
}isp_crg_isp_clock_rst_0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_isp_cmddma_cfg_n    :  1;
        unsigned int ip_rst_isp_cve_cfg_n       :  1;
        unsigned int ip_rst_isp_pe_cfg_n        :  1;
        unsigned int ip_rst_isp_be_cfg_n        :  1;
        unsigned int ip_rst_isp_fe_core2_cfg_n  :  1;
        unsigned int ip_rst_isp_fe_core1_cfg_n  :  1;
        unsigned int ip_rst_isp_fe_core0_cfg_n  :  1;
        unsigned int ip_rst_isp_fe_router_cfg_n :  1;
        unsigned int ip_rst_isp_cfg_nic_n       :  1;
        unsigned int ip_rst_isp_gpdma_n         :  1;
        unsigned int ip_rst_isp_cmddma_n        :  1;
        unsigned int ip_rst_isp_cve_n           :  1;
        unsigned int ip_rst_isp_pe_n            :  1;
        unsigned int ip_rst_isp_be_n            :  1;
        unsigned int ip_rst_isp2csi_fe_n        :  1;
        unsigned int ip_rst_isp_fe_core2_n      :  1;
        unsigned int ip_rst_isp_fe_core1_n      :  1;
        unsigned int ip_rst_isp_fe_core0_n      :  1;
        unsigned int ip_rst_isp_fe_router_n     :  1;
        unsigned int reserved_0                 : 13;
    } reg;
}isp_crg_isp_clock_rst_0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_isp_cmddma_cfg_n    :  1;
        unsigned int ip_rst_isp_cve_cfg_n       :  1;
        unsigned int ip_rst_isp_pe_cfg_n        :  1;
        unsigned int ip_rst_isp_be_cfg_n        :  1;
        unsigned int ip_rst_isp_fe_core2_cfg_n  :  1;
        unsigned int ip_rst_isp_fe_core1_cfg_n  :  1;
        unsigned int ip_rst_isp_fe_core0_cfg_n  :  1;
        unsigned int ip_rst_isp_fe_router_cfg_n :  1;
        unsigned int ip_rst_isp_cfg_nic_n       :  1;
        unsigned int ip_rst_isp_gpdma_n         :  1;
        unsigned int ip_rst_isp_cmddma_n        :  1;
        unsigned int ip_rst_isp_cve_n           :  1;
        unsigned int ip_rst_isp_pe_n            :  1;
        unsigned int ip_rst_isp_be_n            :  1;
        unsigned int ip_rst_isp2csi_fe_n        :  1;
        unsigned int ip_rst_isp_fe_core2_n      :  1;
        unsigned int ip_rst_isp_fe_core1_n      :  1;
        unsigned int ip_rst_isp_fe_core0_n      :  1;
        unsigned int ip_rst_isp_fe_router_n     :  1;
        unsigned int reserved_0                 : 13;
    } reg;
}isp_crg_isp_clock_rst_0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}isp_crg_regfile_cgbypass_t;

#endif
