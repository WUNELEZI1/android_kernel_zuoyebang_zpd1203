// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef HSS2_CRG_REGIF_H
#define HSS2_CRG_REGIF_H

#define HSS2_CRG_CLKDIV0                       0x0010
#define HSS2_CRG_CLKDIV0_DIV_CLK_HSS2_BUS_CFG_SHIFT                                0
#define HSS2_CRG_CLKDIV0_DIV_CLK_HSS2_BUS_CFG_MASK                                 0x0000003f
#define HSS2_CRG_CLKDIV1                       0x0020
#define HSS2_CRG_CLKDIV1_SC_GT_CLK_HSS2_BUS_DATA_SHIFT                             0
#define HSS2_CRG_CLKDIV1_SC_GT_CLK_HSS2_BUS_DATA_MASK                              0x00000001
#define HSS2_CRG_CLKDIV1_DIV_CLK_HSS2_BUS_DATA_SHIFT                               1
#define HSS2_CRG_CLKDIV1_DIV_CLK_HSS2_BUS_DATA_MASK                                0x0000007e
#define HSS2_CRG_CLKDIV2                       0x0030
#define HSS2_CRG_CLKDIV2_SC_GT_CLK_PCIE0_FW_SHIFT                                  0
#define HSS2_CRG_CLKDIV2_SC_GT_CLK_PCIE0_FW_MASK                                   0x00000001
#define HSS2_CRG_CLKDIV2_DIV_CLK_PCIE0_FW_SHIFT                                    1
#define HSS2_CRG_CLKDIV2_DIV_CLK_PCIE0_FW_MASK                                     0x0000007e
#define HSS2_CRG_CLKDIV3                       0x0040
#define HSS2_CRG_CLKDIV3_SC_GT_HCLK_EMMC_SHIFT                                     0
#define HSS2_CRG_CLKDIV3_SC_GT_HCLK_EMMC_MASK                                      0x00000001
#define HSS2_CRG_CLKDIV3_DIV_HCLK_EMMC_SHIFT                                       1
#define HSS2_CRG_CLKDIV3_DIV_HCLK_EMMC_MASK                                        0x0000007e
#define HSS2_CRG_CLKDIV4                       0x0050
#define HSS2_CRG_CLKDIV4_SC_GT_CLK_EMMC_CCLK_BASE_SHIFT                            0
#define HSS2_CRG_CLKDIV4_SC_GT_CLK_EMMC_CCLK_BASE_MASK                             0x00000001
#define HSS2_CRG_CLKDIV4_DIV_CLK_EMMC_CCLK_BASE_SHIFT                              1
#define HSS2_CRG_CLKDIV4_DIV_CLK_EMMC_CCLK_BASE_MASK                               0x0000007e
#define HSS2_CRG_CLKST0                        0x0070
#define HSS2_CRG_CLKST0_DIV_DONE_CLK_HSS2_BUS_CFG_SHIFT                            0
#define HSS2_CRG_CLKST0_DIV_DONE_CLK_HSS2_BUS_CFG_MASK                             0x00000001
#define HSS2_CRG_CLKST0_DIV_DONE_CLK_HSS2_BUS_DATA_SHIFT                           1
#define HSS2_CRG_CLKST0_DIV_DONE_CLK_HSS2_BUS_DATA_MASK                            0x00000002
#define HSS2_CRG_CLKST0_DIV_DONE_CLK_PCIE0_FW_SHIFT                                2
#define HSS2_CRG_CLKST0_DIV_DONE_CLK_PCIE0_FW_MASK                                 0x00000004
#define HSS2_CRG_CLKST0_DIV_DONE_HCLK_EMMC_SHIFT                                   3
#define HSS2_CRG_CLKST0_DIV_DONE_HCLK_EMMC_MASK                                    0x00000008
#define HSS2_CRG_CLKST0_DIV_DONE_CLK_EMMC_CCLK_BASE_SHIFT                          4
#define HSS2_CRG_CLKST0_DIV_DONE_CLK_EMMC_CCLK_BASE_MASK                           0x00000010
#define HSS2_CRG_CLKGT0_W1S                    0x0080
#define HSS2_CRG_CLKGT0_W1S_GT_PCLK_PCIE0_APB_SHIFT                                0
#define HSS2_CRG_CLKGT0_W1S_GT_PCLK_PCIE1_APB_SHIFT                                1
#define HSS2_CRG_CLKGT0_W1S_GT_PCLK_EMMC_SHIFT                                     2
#define HSS2_CRG_CLKGT0_W1S_GT_PCLK_HSS2_TCU_SLV_SHIFT                             3
#define HSS2_CRG_CLKGT0_W1S_GT_ACLK_PCIE0_AXI_SLV_SHIFT                            4
#define HSS2_CRG_CLKGT0_W1S_GT_ACLK_PCIE1_AXI_SLV_SHIFT                            5
#define HSS2_CRG_CLKGT0_W1S_GT_CLK_HSS2_BUS_DATA_SHIFT                             6
#define HSS2_CRG_CLKGT0_W1S_GT_ACLK_HSS2_TCU_SHIFT                                 7
#define HSS2_CRG_CLKGT0_W1S_GT_ACLK_HSS2_TBU0_SHIFT                                8
#define HSS2_CRG_CLKGT0_W1S_GT_ACLK_HSS2_TBU1_SHIFT                                9
#define HSS2_CRG_CLKGT0_W1S_GT_ACLK_PCIE0_AXI_MST_SHIFT                            10
#define HSS2_CRG_CLKGT0_W1S_GT_ACLK_PCIE1_AXI_MST_SHIFT                            11
#define HSS2_CRG_CLKGT0_W1S_GT_ACLK_PERFMON_PCIE0_SHIFT                            12
#define HSS2_CRG_CLKGT0_W1S_GT_ACLK_PERFMON_PCIE1_SHIFT                            13
#define HSS2_CRG_CLKGT0_W1S_GT_ACLK_PERFMON_TCU_SHIFT                              14
#define HSS2_CRG_CLKGT0_W1S_GT_HCLK_EMMC_SHIFT                                     15
#define HSS2_CRG_CLKGT0_W1S_GT_CLK_PCIE0_AUX_SHIFT                                 16
#define HSS2_CRG_CLKGT0_W1S_GT_CLK_PCIE1_AUX_SHIFT                                 17
#define HSS2_CRG_CLKGT0_W1S_GT_PCLK_PERFMON_PCIE0_SHIFT                            18
#define HSS2_CRG_CLKGT0_W1S_GT_PCLK_PERFMON_PCIE1_SHIFT                            19
#define HSS2_CRG_CLKGT0_W1S_GT_PCLK_PERFMON_TCU_SHIFT                              20
#define HSS2_CRG_CLKGT0_W1S_GT_CLK_PCIE_PLL_LOGIC_SHIFT                            21
#define HSS2_CRG_CLKGT0_W1S_GT_CLK_PCIE0_TPC_SHIFT                                 22
#define HSS2_CRG_CLKGT0_W1S_GT_CLK_PCIE1_TPC_SHIFT                                 23
#define HSS2_CRG_CLKGT0_W1S_GT_CLK_TCU_TPC_SHIFT                                   24
#define HSS2_CRG_CLKGT0_W1S_SC_GT_CLK_PLL_TEST_SHIFT                               27
#define HSS2_CRG_CLKGT0_W1S_GT_SW_CLK_PCIE0_FW_SHIFT                               28
#define HSS2_CRG_CLKGT0_W1S_GT_SW_CLK_PCIE1_FW_SHIFT                               29
#define HSS2_CRG_CLKGT0_W1C                    0x0084
#define HSS2_CRG_CLKGT0_W1C_GT_PCLK_PCIE0_APB_SHIFT                                0
#define HSS2_CRG_CLKGT0_W1C_GT_PCLK_PCIE1_APB_SHIFT                                1
#define HSS2_CRG_CLKGT0_W1C_GT_PCLK_EMMC_SHIFT                                     2
#define HSS2_CRG_CLKGT0_W1C_GT_PCLK_HSS2_TCU_SLV_SHIFT                             3
#define HSS2_CRG_CLKGT0_W1C_GT_ACLK_PCIE0_AXI_SLV_SHIFT                            4
#define HSS2_CRG_CLKGT0_W1C_GT_ACLK_PCIE1_AXI_SLV_SHIFT                            5
#define HSS2_CRG_CLKGT0_W1C_GT_CLK_HSS2_BUS_DATA_SHIFT                             6
#define HSS2_CRG_CLKGT0_W1C_GT_ACLK_HSS2_TCU_SHIFT                                 7
#define HSS2_CRG_CLKGT0_W1C_GT_ACLK_HSS2_TBU0_SHIFT                                8
#define HSS2_CRG_CLKGT0_W1C_GT_ACLK_HSS2_TBU1_SHIFT                                9
#define HSS2_CRG_CLKGT0_W1C_GT_ACLK_PCIE0_AXI_MST_SHIFT                            10
#define HSS2_CRG_CLKGT0_W1C_GT_ACLK_PCIE1_AXI_MST_SHIFT                            11
#define HSS2_CRG_CLKGT0_W1C_GT_ACLK_PERFMON_PCIE0_SHIFT                            12
#define HSS2_CRG_CLKGT0_W1C_GT_ACLK_PERFMON_PCIE1_SHIFT                            13
#define HSS2_CRG_CLKGT0_W1C_GT_ACLK_PERFMON_TCU_SHIFT                              14
#define HSS2_CRG_CLKGT0_W1C_GT_HCLK_EMMC_SHIFT                                     15
#define HSS2_CRG_CLKGT0_W1C_GT_CLK_PCIE0_AUX_SHIFT                                 16
#define HSS2_CRG_CLKGT0_W1C_GT_CLK_PCIE1_AUX_SHIFT                                 17
#define HSS2_CRG_CLKGT0_W1C_GT_PCLK_PERFMON_PCIE0_SHIFT                            18
#define HSS2_CRG_CLKGT0_W1C_GT_PCLK_PERFMON_PCIE1_SHIFT                            19
#define HSS2_CRG_CLKGT0_W1C_GT_PCLK_PERFMON_TCU_SHIFT                              20
#define HSS2_CRG_CLKGT0_W1C_GT_CLK_PCIE_PLL_LOGIC_SHIFT                            21
#define HSS2_CRG_CLKGT0_W1C_GT_CLK_PCIE0_TPC_SHIFT                                 22
#define HSS2_CRG_CLKGT0_W1C_GT_CLK_PCIE1_TPC_SHIFT                                 23
#define HSS2_CRG_CLKGT0_W1C_GT_CLK_TCU_TPC_SHIFT                                   24
#define HSS2_CRG_CLKGT0_W1C_SC_GT_CLK_PLL_TEST_SHIFT                               27
#define HSS2_CRG_CLKGT0_W1C_GT_SW_CLK_PCIE0_FW_SHIFT                               28
#define HSS2_CRG_CLKGT0_W1C_GT_SW_CLK_PCIE1_FW_SHIFT                               29
#define HSS2_CRG_CLKGT0_RO                     0x0088
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_PCIE0_APB_SHIFT                                 0
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_PCIE0_APB_MASK                                  0x00000001
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_PCIE1_APB_SHIFT                                 1
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_PCIE1_APB_MASK                                  0x00000002
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_EMMC_SHIFT                                      2
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_EMMC_MASK                                       0x00000004
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_HSS2_TCU_SLV_SHIFT                              3
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_HSS2_TCU_SLV_MASK                               0x00000008
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PCIE0_AXI_SLV_SHIFT                             4
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PCIE0_AXI_SLV_MASK                              0x00000010
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PCIE1_AXI_SLV_SHIFT                             5
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PCIE1_AXI_SLV_MASK                              0x00000020
#define HSS2_CRG_CLKGT0_RO_GT_CLK_HSS2_BUS_DATA_SHIFT                              6
#define HSS2_CRG_CLKGT0_RO_GT_CLK_HSS2_BUS_DATA_MASK                               0x00000040
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_HSS2_TCU_SHIFT                                  7
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_HSS2_TCU_MASK                                   0x00000080
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_HSS2_TBU0_SHIFT                                 8
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_HSS2_TBU0_MASK                                  0x00000100
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_HSS2_TBU1_SHIFT                                 9
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_HSS2_TBU1_MASK                                  0x00000200
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PCIE0_AXI_MST_SHIFT                             10
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PCIE0_AXI_MST_MASK                              0x00000400
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PCIE1_AXI_MST_SHIFT                             11
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PCIE1_AXI_MST_MASK                              0x00000800
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PERFMON_PCIE0_SHIFT                             12
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PERFMON_PCIE0_MASK                              0x00001000
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PERFMON_PCIE1_SHIFT                             13
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PERFMON_PCIE1_MASK                              0x00002000
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PERFMON_TCU_SHIFT                               14
#define HSS2_CRG_CLKGT0_RO_GT_ACLK_PERFMON_TCU_MASK                                0x00004000
#define HSS2_CRG_CLKGT0_RO_GT_HCLK_EMMC_SHIFT                                      15
#define HSS2_CRG_CLKGT0_RO_GT_HCLK_EMMC_MASK                                       0x00008000
#define HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE0_AUX_SHIFT                                  16
#define HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE0_AUX_MASK                                   0x00010000
#define HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE1_AUX_SHIFT                                  17
#define HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE1_AUX_MASK                                   0x00020000
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_PERFMON_PCIE0_SHIFT                             18
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_PERFMON_PCIE0_MASK                              0x00040000
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_PERFMON_PCIE1_SHIFT                             19
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_PERFMON_PCIE1_MASK                              0x00080000
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_PERFMON_TCU_SHIFT                               20
#define HSS2_CRG_CLKGT0_RO_GT_PCLK_PERFMON_TCU_MASK                                0x00100000
#define HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE_PLL_LOGIC_SHIFT                             21
#define HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE_PLL_LOGIC_MASK                              0x00200000
#define HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE0_TPC_SHIFT                                  22
#define HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE0_TPC_MASK                                   0x00400000
#define HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE1_TPC_SHIFT                                  23
#define HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE1_TPC_MASK                                   0x00800000
#define HSS2_CRG_CLKGT0_RO_GT_CLK_TCU_TPC_SHIFT                                    24
#define HSS2_CRG_CLKGT0_RO_GT_CLK_TCU_TPC_MASK                                     0x01000000
#define HSS2_CRG_CLKGT0_RO_SC_GT_CLK_PLL_TEST_SHIFT                                27
#define HSS2_CRG_CLKGT0_RO_SC_GT_CLK_PLL_TEST_MASK                                 0x08000000
#define HSS2_CRG_CLKGT0_RO_GT_SW_CLK_PCIE0_FW_SHIFT                                28
#define HSS2_CRG_CLKGT0_RO_GT_SW_CLK_PCIE0_FW_MASK                                 0x10000000
#define HSS2_CRG_CLKGT0_RO_GT_SW_CLK_PCIE1_FW_SHIFT                                29
#define HSS2_CRG_CLKGT0_RO_GT_SW_CLK_PCIE1_FW_MASK                                 0x20000000
#define HSS2_CRG_CLKST1                        0x0090
#define HSS2_CRG_CLKST1_ST_PCLK_PCIE0_APB_SHIFT                                    0
#define HSS2_CRG_CLKST1_ST_PCLK_PCIE0_APB_MASK                                     0x00000001
#define HSS2_CRG_CLKST1_ST_PCLK_PCIE1_APB_SHIFT                                    1
#define HSS2_CRG_CLKST1_ST_PCLK_PCIE1_APB_MASK                                     0x00000002
#define HSS2_CRG_CLKST1_ST_PCLK_EMMC_SHIFT                                         2
#define HSS2_CRG_CLKST1_ST_PCLK_EMMC_MASK                                          0x00000004
#define HSS2_CRG_CLKST1_ST_PCLK_HSS2_TCU_SLV_SHIFT                                 3
#define HSS2_CRG_CLKST1_ST_PCLK_HSS2_TCU_SLV_MASK                                  0x00000008
#define HSS2_CRG_CLKST1_ST_ACLK_PCIE0_AXI_SLV_SHIFT                                4
#define HSS2_CRG_CLKST1_ST_ACLK_PCIE0_AXI_SLV_MASK                                 0x00000010
#define HSS2_CRG_CLKST1_ST_ACLK_PCIE1_AXI_SLV_SHIFT                                5
#define HSS2_CRG_CLKST1_ST_ACLK_PCIE1_AXI_SLV_MASK                                 0x00000020
#define HSS2_CRG_CLKST1_ST_CLK_HSS2_BUS_DATA_SHIFT                                 6
#define HSS2_CRG_CLKST1_ST_CLK_HSS2_BUS_DATA_MASK                                  0x00000040
#define HSS2_CRG_CLKST1_ST_ACLK_HSS2_TCU_SHIFT                                     7
#define HSS2_CRG_CLKST1_ST_ACLK_HSS2_TCU_MASK                                      0x00000080
#define HSS2_CRG_CLKST1_ST_ACLK_HSS2_TBU0_SHIFT                                    8
#define HSS2_CRG_CLKST1_ST_ACLK_HSS2_TBU0_MASK                                     0x00000100
#define HSS2_CRG_CLKST1_ST_ACLK_HSS2_TBU1_SHIFT                                    9
#define HSS2_CRG_CLKST1_ST_ACLK_HSS2_TBU1_MASK                                     0x00000200
#define HSS2_CRG_CLKST1_ST_ACLK_PCIE0_AXI_MST_SHIFT                                10
#define HSS2_CRG_CLKST1_ST_ACLK_PCIE0_AXI_MST_MASK                                 0x00000400
#define HSS2_CRG_CLKST1_ST_ACLK_PCIE1_AXI_MST_SHIFT                                11
#define HSS2_CRG_CLKST1_ST_ACLK_PCIE1_AXI_MST_MASK                                 0x00000800
#define HSS2_CRG_CLKST1_ST_ACLK_PERFMON_PCIE0_SHIFT                                12
#define HSS2_CRG_CLKST1_ST_ACLK_PERFMON_PCIE0_MASK                                 0x00001000
#define HSS2_CRG_CLKST1_ST_ACLK_PERFMON_PCIE1_SHIFT                                13
#define HSS2_CRG_CLKST1_ST_ACLK_PERFMON_PCIE1_MASK                                 0x00002000
#define HSS2_CRG_CLKST1_ST_ACLK_PERFMON_TCU_SHIFT                                  14
#define HSS2_CRG_CLKST1_ST_ACLK_PERFMON_TCU_MASK                                   0x00004000
#define HSS2_CRG_CLKST1_ST_HCLK_EMMC_SHIFT                                         15
#define HSS2_CRG_CLKST1_ST_HCLK_EMMC_MASK                                          0x00008000
#define HSS2_CRG_CLKST1_ST_CLK_PCIE0_AUX_SHIFT                                     16
#define HSS2_CRG_CLKST1_ST_CLK_PCIE0_AUX_MASK                                      0x00010000
#define HSS2_CRG_CLKST1_ST_CLK_PCIE1_AUX_SHIFT                                     17
#define HSS2_CRG_CLKST1_ST_CLK_PCIE1_AUX_MASK                                      0x00020000
#define HSS2_CRG_CLKST1_ST_PCLK_PERFMON_PCIE0_SHIFT                                18
#define HSS2_CRG_CLKST1_ST_PCLK_PERFMON_PCIE0_MASK                                 0x00040000
#define HSS2_CRG_CLKST1_ST_PCLK_PERFMON_PCIE1_SHIFT                                19
#define HSS2_CRG_CLKST1_ST_PCLK_PERFMON_PCIE1_MASK                                 0x00080000
#define HSS2_CRG_CLKST1_ST_PCLK_PERFMON_TCU_SHIFT                                  20
#define HSS2_CRG_CLKST1_ST_PCLK_PERFMON_TCU_MASK                                   0x00100000
#define HSS2_CRG_CLKST1_ST_CLK_PCIE_PLL_LOGIC_SHIFT                                21
#define HSS2_CRG_CLKST1_ST_CLK_PCIE_PLL_LOGIC_MASK                                 0x00200000
#define HSS2_CRG_CLKST1_ST_CLK_PCIE0_TPC_SHIFT                                     22
#define HSS2_CRG_CLKST1_ST_CLK_PCIE0_TPC_MASK                                      0x00400000
#define HSS2_CRG_CLKST1_ST_CLK_PCIE1_TPC_SHIFT                                     23
#define HSS2_CRG_CLKST1_ST_CLK_PCIE1_TPC_MASK                                      0x00800000
#define HSS2_CRG_CLKST1_ST_CLK_TCU_TPC_SHIFT                                       24
#define HSS2_CRG_CLKST1_ST_CLK_TCU_TPC_MASK                                        0x01000000
#define HSS2_CRG_CLKST1_ST_HCLK_EMMC_SLV_SHIFT                                     25
#define HSS2_CRG_CLKST1_ST_HCLK_EMMC_SLV_MASK                                      0x02000000
#define HSS2_CRG_CLKST1_ST_ACLK_EMMC_MST_SHIFT                                     26
#define HSS2_CRG_CLKST1_ST_ACLK_EMMC_MST_MASK                                      0x04000000
#define HSS2_CRG_CLKST1_ST_CLK_PCIE0_FW_SHIFT                                      28
#define HSS2_CRG_CLKST1_ST_CLK_PCIE0_FW_MASK                                       0x10000000
#define HSS2_CRG_CLKST1_ST_CLK_PCIE1_FW_SHIFT                                      29
#define HSS2_CRG_CLKST1_ST_CLK_PCIE1_FW_MASK                                       0x20000000
#define HSS2_CRG_RST0_W1S                      0x00a0
#define HSS2_CRG_RST0_W1S_IP_RST_PLL_LOGIC_N_SHIFT                                 0
#define HSS2_CRG_RST0_W1S_IP_RST_SSMOD_N_SHIFT                                     1
#define HSS2_CRG_RST0_W1S_IP_RST_PCIE0_POWER_UP_N_SHIFT                            2
#define HSS2_CRG_RST0_W1S_IP_PRST_PCIE0_DBI_N_SHIFT                                3
#define HSS2_CRG_RST0_W1S_IP_RST_PCIE0_AUX_N_SHIFT                                 4
#define HSS2_CRG_RST0_W1S_IP_PRST_PCIE0_N_SHIFT                                    6
#define HSS2_CRG_RST0_W1S_IP_RST_PCIE1_POWER_UP_N_SHIFT                            7
#define HSS2_CRG_RST0_W1S_IP_PRST_PCIE1_DBI_N_SHIFT                                8
#define HSS2_CRG_RST0_W1S_IP_RST_PCIE1_AUX_N_SHIFT                                 9
#define HSS2_CRG_RST0_W1S_IP_PRST_PCIE1_N_SHIFT                                    11
#define HSS2_CRG_RST0_W1S_IP_ARST_HSS2_TBU0_N_SHIFT                                12
#define HSS2_CRG_RST0_W1S_IP_ARST_HSS2_TBU1_N_SHIFT                                13
#define HSS2_CRG_RST0_W1S_IP_ARST_HSS2_TCU_N_SHIFT                                 14
#define HSS2_CRG_RST0_W1S_IP_RST_HSS2_BUS_DATA_N_SHIFT                             15
#define HSS2_CRG_RST0_W1S_IP_ARST_PERFMON_PCIE0_N_SHIFT                            16
#define HSS2_CRG_RST0_W1S_IP_ARST_PERFMON_PCIE1_N_SHIFT                            17
#define HSS2_CRG_RST0_W1S_IP_ARST_PERFMON_TCU_N_SHIFT                              18
#define HSS2_CRG_RST0_W1S_IP_PRST_EMMC_N_SHIFT                                     19
#define HSS2_CRG_RST0_W1S_IP_RST_EMMC_N_SHIFT                                      20
#define HSS2_CRG_RST0_W1S_IP_ARST_EMMC_SLV_N_SHIFT                                 21
#define HSS2_CRG_RST0_W1S_IP_HRST_EMMC_MST_N_SHIFT                                 22
#define HSS2_CRG_RST0_W1S_IP_RST_PCIE0_TPC_N_SHIFT                                 23
#define HSS2_CRG_RST0_W1S_IP_RST_PCIE1_TPC_N_SHIFT                                 24
#define HSS2_CRG_RST0_W1S_IP_RST_TCU_TPC_N_SHIFT                                   25
#define HSS2_CRG_RST0_W1C                      0x00a4
#define HSS2_CRG_RST0_W1C_IP_RST_PLL_LOGIC_N_SHIFT                                 0
#define HSS2_CRG_RST0_W1C_IP_RST_SSMOD_N_SHIFT                                     1
#define HSS2_CRG_RST0_W1C_IP_RST_PCIE0_POWER_UP_N_SHIFT                            2
#define HSS2_CRG_RST0_W1C_IP_PRST_PCIE0_DBI_N_SHIFT                                3
#define HSS2_CRG_RST0_W1C_IP_RST_PCIE0_AUX_N_SHIFT                                 4
#define HSS2_CRG_RST0_W1C_IP_PRST_PCIE0_N_SHIFT                                    6
#define HSS2_CRG_RST0_W1C_IP_RST_PCIE1_POWER_UP_N_SHIFT                            7
#define HSS2_CRG_RST0_W1C_IP_PRST_PCIE1_DBI_N_SHIFT                                8
#define HSS2_CRG_RST0_W1C_IP_RST_PCIE1_AUX_N_SHIFT                                 9
#define HSS2_CRG_RST0_W1C_IP_PRST_PCIE1_N_SHIFT                                    11
#define HSS2_CRG_RST0_W1C_IP_ARST_HSS2_TBU0_N_SHIFT                                12
#define HSS2_CRG_RST0_W1C_IP_ARST_HSS2_TBU1_N_SHIFT                                13
#define HSS2_CRG_RST0_W1C_IP_ARST_HSS2_TCU_N_SHIFT                                 14
#define HSS2_CRG_RST0_W1C_IP_RST_HSS2_BUS_DATA_N_SHIFT                             15
#define HSS2_CRG_RST0_W1C_IP_ARST_PERFMON_PCIE0_N_SHIFT                            16
#define HSS2_CRG_RST0_W1C_IP_ARST_PERFMON_PCIE1_N_SHIFT                            17
#define HSS2_CRG_RST0_W1C_IP_ARST_PERFMON_TCU_N_SHIFT                              18
#define HSS2_CRG_RST0_W1C_IP_PRST_EMMC_N_SHIFT                                     19
#define HSS2_CRG_RST0_W1C_IP_RST_EMMC_N_SHIFT                                      20
#define HSS2_CRG_RST0_W1C_IP_ARST_EMMC_SLV_N_SHIFT                                 21
#define HSS2_CRG_RST0_W1C_IP_HRST_EMMC_MST_N_SHIFT                                 22
#define HSS2_CRG_RST0_W1C_IP_RST_PCIE0_TPC_N_SHIFT                                 23
#define HSS2_CRG_RST0_W1C_IP_RST_PCIE1_TPC_N_SHIFT                                 24
#define HSS2_CRG_RST0_W1C_IP_RST_TCU_TPC_N_SHIFT                                   25
#define HSS2_CRG_RST0_RO                       0x00a8
#define HSS2_CRG_RST0_RO_IP_RST_PLL_LOGIC_N_SHIFT                                  0
#define HSS2_CRG_RST0_RO_IP_RST_PLL_LOGIC_N_MASK                                   0x00000001
#define HSS2_CRG_RST0_RO_IP_RST_SSMOD_N_SHIFT                                      1
#define HSS2_CRG_RST0_RO_IP_RST_SSMOD_N_MASK                                       0x00000002
#define HSS2_CRG_RST0_RO_IP_RST_PCIE0_POWER_UP_N_SHIFT                             2
#define HSS2_CRG_RST0_RO_IP_RST_PCIE0_POWER_UP_N_MASK                              0x00000004
#define HSS2_CRG_RST0_RO_IP_PRST_PCIE0_DBI_N_SHIFT                                 3
#define HSS2_CRG_RST0_RO_IP_PRST_PCIE0_DBI_N_MASK                                  0x00000008
#define HSS2_CRG_RST0_RO_IP_RST_PCIE0_AUX_N_SHIFT                                  4
#define HSS2_CRG_RST0_RO_IP_RST_PCIE0_AUX_N_MASK                                   0x00000010
#define HSS2_CRG_RST0_RO_IP_PRST_PCIE0_N_SHIFT                                     6
#define HSS2_CRG_RST0_RO_IP_PRST_PCIE0_N_MASK                                      0x00000040
#define HSS2_CRG_RST0_RO_IP_RST_PCIE1_POWER_UP_N_SHIFT                             7
#define HSS2_CRG_RST0_RO_IP_RST_PCIE1_POWER_UP_N_MASK                              0x00000080
#define HSS2_CRG_RST0_RO_IP_PRST_PCIE1_DBI_N_SHIFT                                 8
#define HSS2_CRG_RST0_RO_IP_PRST_PCIE1_DBI_N_MASK                                  0x00000100
#define HSS2_CRG_RST0_RO_IP_RST_PCIE1_AUX_N_SHIFT                                  9
#define HSS2_CRG_RST0_RO_IP_RST_PCIE1_AUX_N_MASK                                   0x00000200
#define HSS2_CRG_RST0_RO_IP_PRST_PCIE1_N_SHIFT                                     11
#define HSS2_CRG_RST0_RO_IP_PRST_PCIE1_N_MASK                                      0x00000800
#define HSS2_CRG_RST0_RO_IP_ARST_HSS2_TBU0_N_SHIFT                                 12
#define HSS2_CRG_RST0_RO_IP_ARST_HSS2_TBU0_N_MASK                                  0x00001000
#define HSS2_CRG_RST0_RO_IP_ARST_HSS2_TBU1_N_SHIFT                                 13
#define HSS2_CRG_RST0_RO_IP_ARST_HSS2_TBU1_N_MASK                                  0x00002000
#define HSS2_CRG_RST0_RO_IP_ARST_HSS2_TCU_N_SHIFT                                  14
#define HSS2_CRG_RST0_RO_IP_ARST_HSS2_TCU_N_MASK                                   0x00004000
#define HSS2_CRG_RST0_RO_IP_RST_HSS2_BUS_DATA_N_SHIFT                              15
#define HSS2_CRG_RST0_RO_IP_RST_HSS2_BUS_DATA_N_MASK                               0x00008000
#define HSS2_CRG_RST0_RO_IP_ARST_PERFMON_PCIE0_N_SHIFT                             16
#define HSS2_CRG_RST0_RO_IP_ARST_PERFMON_PCIE0_N_MASK                              0x00010000
#define HSS2_CRG_RST0_RO_IP_ARST_PERFMON_PCIE1_N_SHIFT                             17
#define HSS2_CRG_RST0_RO_IP_ARST_PERFMON_PCIE1_N_MASK                              0x00020000
#define HSS2_CRG_RST0_RO_IP_ARST_PERFMON_TCU_N_SHIFT                               18
#define HSS2_CRG_RST0_RO_IP_ARST_PERFMON_TCU_N_MASK                                0x00040000
#define HSS2_CRG_RST0_RO_IP_PRST_EMMC_N_SHIFT                                      19
#define HSS2_CRG_RST0_RO_IP_PRST_EMMC_N_MASK                                       0x00080000
#define HSS2_CRG_RST0_RO_IP_RST_EMMC_N_SHIFT                                       20
#define HSS2_CRG_RST0_RO_IP_RST_EMMC_N_MASK                                        0x00100000
#define HSS2_CRG_RST0_RO_IP_ARST_EMMC_SLV_N_SHIFT                                  21
#define HSS2_CRG_RST0_RO_IP_ARST_EMMC_SLV_N_MASK                                   0x00200000
#define HSS2_CRG_RST0_RO_IP_HRST_EMMC_MST_N_SHIFT                                  22
#define HSS2_CRG_RST0_RO_IP_HRST_EMMC_MST_N_MASK                                   0x00400000
#define HSS2_CRG_RST0_RO_IP_RST_PCIE0_TPC_N_SHIFT                                  23
#define HSS2_CRG_RST0_RO_IP_RST_PCIE0_TPC_N_MASK                                   0x00800000
#define HSS2_CRG_RST0_RO_IP_RST_PCIE1_TPC_N_SHIFT                                  24
#define HSS2_CRG_RST0_RO_IP_RST_PCIE1_TPC_N_MASK                                   0x01000000
#define HSS2_CRG_RST0_RO_IP_RST_TCU_TPC_N_SHIFT                                    25
#define HSS2_CRG_RST0_RO_IP_RST_TCU_TPC_N_MASK                                     0x02000000
#define HSS2_CRG_HSS2CTRL0                     0x00b0
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE1_PHY_APB1_BYPASS_SHIFT                 0
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE1_PHY_APB1_BYPASS_MASK                  0x00000001
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE1_PHY_APB0_BYPASS_SHIFT                 1
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE1_PHY_APB0_BYPASS_MASK                  0x00000002
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE1_PHY_BYPASS_SHIFT                      2
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE1_PHY_BYPASS_MASK                       0x00000004
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE1_CTRL_SII_BYPASS_SHIFT                 3
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE1_CTRL_SII_BYPASS_MASK                  0x00000008
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE1_CTRL_DBI_BYPASS_SHIFT                 4
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE1_CTRL_DBI_BYPASS_MASK                  0x00000010
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE0_PHY_APB1_BYPASS_SHIFT                 5
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE0_PHY_APB1_BYPASS_MASK                  0x00000020
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE0_PHY_APB0_BYPASS_SHIFT                 6
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE0_PHY_APB0_BYPASS_MASK                  0x00000040
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE0_PHY_BYPASS_SHIFT                      7
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE0_PHY_BYPASS_MASK                       0x00000080
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE0_CTRL_SII_BYPASS_SHIFT                 8
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE0_CTRL_SII_BYPASS_MASK                  0x00000100
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE0_CTRL_DBI_BYPASS_SHIFT                 9
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_PCIE0_CTRL_DBI_BYPASS_MASK                  0x00000200
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_EMMC_BYPASS_SHIFT                           10
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_EMMC_BYPASS_MASK                            0x00000400
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_SMMU_BYPASS_SHIFT                           11
#define HSS2_CRG_HSS2CTRL0_CLKRST_FLAG_SMMU_BYPASS_MASK                            0x00000800
#define HSS2_CRG_HSS2CTRL1                     0x00c0
#define HSS2_CRG_HSS2CTRL1_GT_AUTO_CLK_PCIE0_FW_BYPASS_SHIFT                       0
#define HSS2_CRG_HSS2CTRL1_GT_AUTO_CLK_PCIE0_FW_BYPASS_MASK                        0x00000001
#define HSS2_CRG_HSS2CTRL1_GT_AUTO_CLK_PCIE1_FW_BYPASS_SHIFT                       1
#define HSS2_CRG_HSS2CTRL1_GT_AUTO_CLK_PCIE1_FW_BYPASS_MASK                        0x00000002
#define HSS2_CRG_AUTOFSCTRL0                   0x0280
#define HSS2_CRG_AUTOFSCTRL0_HSS2_BUS_INI_NO_PENDING_TRANS_DATA_BYPASS_SHIFT       0
#define HSS2_CRG_AUTOFSCTRL0_HSS2_BUS_INI_NO_PENDING_TRANS_DATA_BYPASS_MASK        0x0000001f
#define HSS2_CRG_AUTOFSCTRL0_HSS2_BUS_TGT_NO_PENDING_TRANS_DATA_BYPASS_SHIFT       5
#define HSS2_CRG_AUTOFSCTRL0_HSS2_BUS_TGT_NO_PENDING_TRANS_DATA_BYPASS_MASK        0x00000fe0
#define HSS2_CRG_AUTOFSCTRL0_IDLE_FLAG_SMMU_TCU_DATA_BYPASS_SHIFT                  12
#define HSS2_CRG_AUTOFSCTRL0_IDLE_FLAG_SMMU_TCU_DATA_BYPASS_MASK                   0x00001000
#define HSS2_CRG_AUTOFSCTRL0_IDLE_FLAG_PERFSTAT_PCIE1_DATA_BYPASS_SHIFT            13
#define HSS2_CRG_AUTOFSCTRL0_IDLE_FLAG_PERFSTAT_PCIE1_DATA_BYPASS_MASK             0x00002000
#define HSS2_CRG_AUTOFSCTRL0_IDLE_FLAG_PERFSTAT_PCIE0_DATA_BYPASS_SHIFT            14
#define HSS2_CRG_AUTOFSCTRL0_IDLE_FLAG_PERFSTAT_PCIE0_DATA_BYPASS_MASK             0x00004000
#define HSS2_CRG_AUTOFSCTRL0_IDLE_FLAG_PERFSTAT_TCU_DATA_BYPASS_SHIFT              15
#define HSS2_CRG_AUTOFSCTRL0_IDLE_FLAG_PERFSTAT_TCU_DATA_BYPASS_MASK               0x00008000
#define HSS2_CRG_AUTOFSCTRL0_CFGBUS2HSS2_BUS_TGT_NO_PENDING_TRANS_DATA_BYPASS_SHIFT 16
#define HSS2_CRG_AUTOFSCTRL0_CFGBUS2HSS2_BUS_TGT_NO_PENDING_TRANS_DATA_BYPASS_MASK  0x00010000
#define HSS2_CRG_AUTOFSCTRL1                   0x0290
#define HSS2_CRG_AUTOFSCTRL1_DEBOUNCE_IN_HSS2_BUS_DATA_SHIFT                       0
#define HSS2_CRG_AUTOFSCTRL1_DEBOUNCE_IN_HSS2_BUS_DATA_MASK                        0x000003ff
#define HSS2_CRG_AUTOFSCTRL1_DEBOUNCE_OUT_HSS2_BUS_DATA_SHIFT                      10
#define HSS2_CRG_AUTOFSCTRL1_DEBOUNCE_OUT_HSS2_BUS_DATA_MASK                       0x000ffc00
#define HSS2_CRG_AUTOFSCTRL1_DEBOUNCE_BYPASS_HSS2_BUS_DATA_SHIFT                   20
#define HSS2_CRG_AUTOFSCTRL1_DEBOUNCE_BYPASS_HSS2_BUS_DATA_MASK                    0x00100000
#define HSS2_CRG_AUTOFSCTRL1_DIV_AUTO_CLK_HSS2_BUS_DATA_SHIFT                      21
#define HSS2_CRG_AUTOFSCTRL1_DIV_AUTO_CLK_HSS2_BUS_DATA_MASK                       0x07e00000
#define HSS2_CRG_AUTOFSCTRL1_AUTOFS_EN_HSS2_BUS_DATA_SHIFT                         31
#define HSS2_CRG_AUTOFSCTRL1_AUTOFS_EN_HSS2_BUS_DATA_MASK                          0x80000000
#define HSS2_CRG_AUTOFSCTRL2                   0x0300
#define HSS2_CRG_AUTOFSCTRL2_HSS2_BUS_INI_NO_PENDING_TRANS_CFG_BYPASS_SHIFT        0
#define HSS2_CRG_AUTOFSCTRL2_HSS2_BUS_INI_NO_PENDING_TRANS_CFG_BYPASS_MASK         0x0000001f
#define HSS2_CRG_AUTOFSCTRL2_HSS2_BUS_TGT_NO_PENDING_TRANS_CFG_BYPASS_SHIFT        5
#define HSS2_CRG_AUTOFSCTRL2_HSS2_BUS_TGT_NO_PENDING_TRANS_CFG_BYPASS_MASK         0x00000fe0
#define HSS2_CRG_AUTOFSCTRL2_IDLE_FLAG_SMMU_TCU_CFG_BYPASS_SHIFT                   12
#define HSS2_CRG_AUTOFSCTRL2_IDLE_FLAG_SMMU_TCU_CFG_BYPASS_MASK                    0x00001000
#define HSS2_CRG_AUTOFSCTRL2_IDLE_FLAG_PERFSTAT_PCIE1_CFG_BYPASS_SHIFT             13
#define HSS2_CRG_AUTOFSCTRL2_IDLE_FLAG_PERFSTAT_PCIE1_CFG_BYPASS_MASK              0x00002000
#define HSS2_CRG_AUTOFSCTRL2_IDLE_FLAG_PERFSTAT_PCIE0_CFG_BYPASS_SHIFT             14
#define HSS2_CRG_AUTOFSCTRL2_IDLE_FLAG_PERFSTAT_PCIE0_CFG_BYPASS_MASK              0x00004000
#define HSS2_CRG_AUTOFSCTRL2_IDLE_FLAG_PERFSTAT_TCU_CFG_BYPASS_SHIFT               15
#define HSS2_CRG_AUTOFSCTRL2_IDLE_FLAG_PERFSTAT_TCU_CFG_BYPASS_MASK                0x00008000
#define HSS2_CRG_AUTOFSCTRL2_CFGBUS2HSS2_BUS_TGT_NO_PENDING_TRANS_CFG_BYPASS_SHIFT 16
#define HSS2_CRG_AUTOFSCTRL2_CFGBUS2HSS2_BUS_TGT_NO_PENDING_TRANS_CFG_BYPASS_MASK  0x00010000
#define HSS2_CRG_AUTOFSCTRL3                   0x0310
#define HSS2_CRG_AUTOFSCTRL3_DEBOUNCE_IN_HSS2_BUS_CFG_SHIFT                        0
#define HSS2_CRG_AUTOFSCTRL3_DEBOUNCE_IN_HSS2_BUS_CFG_MASK                         0x000003ff
#define HSS2_CRG_AUTOFSCTRL3_DEBOUNCE_OUT_HSS2_BUS_CFG_SHIFT                       10
#define HSS2_CRG_AUTOFSCTRL3_DEBOUNCE_OUT_HSS2_BUS_CFG_MASK                        0x000ffc00
#define HSS2_CRG_AUTOFSCTRL3_DEBOUNCE_BYPASS_HSS2_BUS_CFG_SHIFT                    20
#define HSS2_CRG_AUTOFSCTRL3_DEBOUNCE_BYPASS_HSS2_BUS_CFG_MASK                     0x00100000
#define HSS2_CRG_AUTOFSCTRL3_DIV_AUTO_CLK_HSS2_BUS_CFG_SHIFT                       21
#define HSS2_CRG_AUTOFSCTRL3_DIV_AUTO_CLK_HSS2_BUS_CFG_MASK                        0x07e00000
#define HSS2_CRG_AUTOFSCTRL3_AUTOFS_EN_HSS2_BUS_CFG_SHIFT                          31
#define HSS2_CRG_AUTOFSCTRL3_AUTOFS_EN_HSS2_BUS_CFG_MASK                           0x80000000
#define HSS2_CRG_AUTOFSCTRL4                   0x0320
#define HSS2_CRG_AUTOFSCTRL4_IDLE_FLAG_SMMU_TCU_SHIFT                              0
#define HSS2_CRG_AUTOFSCTRL4_IDLE_FLAG_SMMU_TCU_MASK                               0x00000001
#define HSS2_CRG_AUTOFSCTRL4_IDLE_FLAG_PERFSTAT_PCIE1_SHIFT                        1
#define HSS2_CRG_AUTOFSCTRL4_IDLE_FLAG_PERFSTAT_PCIE1_MASK                         0x00000002
#define HSS2_CRG_AUTOFSCTRL4_IDLE_FLAG_PERFSTAT_PCIE0_SHIFT                        2
#define HSS2_CRG_AUTOFSCTRL4_IDLE_FLAG_PERFSTAT_PCIE0_MASK                         0x00000004
#define HSS2_CRG_AUTOFSCTRL4_IDLE_FLAG_PERFSTAT_TCU_SHIFT                          3
#define HSS2_CRG_AUTOFSCTRL4_IDLE_FLAG_PERFSTAT_TCU_MASK                           0x00000008
#define HSS2_CRG_AUTOFSDISABLE0                0x0324
#define HSS2_CRG_AUTOFSDISABLE0_AUTOFS_BYPASS_HSS2_BUS_DATA_SHIFT                  0
#define HSS2_CRG_AUTOFSDISABLE0_AUTOFS_BYPASS_HSS2_BUS_DATA_MASK                   0x0000ffff
#define HSS2_CRG_AUTOFSDISABLE1                0x0328
#define HSS2_CRG_AUTOFSDISABLE1_AUTOFS_BYPASS_HSS2_BUS_CFG_SHIFT                   0
#define HSS2_CRG_AUTOFSDISABLE1_AUTOFS_BYPASS_HSS2_BUS_CFG_MASK                    0x0000ffff
#define HSS2_CRG_PCIE_PLL_LOCK_STATE           0x0330
#define HSS2_CRG_PCIE_PLL_LOCK_STATE_PLL_LOCK_SHIFT                                0
#define HSS2_CRG_PCIE_PLL_LOCK_STATE_PLL_LOCK_MASK                                 0x00000001
#define HSS2_CRG_INTR_MASK                     0x0334
#define HSS2_CRG_INTR_MASK_INTR_MASK_PLL_INI_ERR_SHIFT                             0
#define HSS2_CRG_INTR_MASK_INTR_MASK_PLL_INI_ERR_MASK                              0x00000001
#define HSS2_CRG_INTR_MASK_INTR_MASK_PLL_UNLOCK_SHIFT                              1
#define HSS2_CRG_INTR_MASK_INTR_MASK_PLL_UNLOCK_MASK                               0x00000002
#define HSS2_CRG_INTR_MASK_INTR_MASK_VOTE_REQ_ERR_SHIFT                            2
#define HSS2_CRG_INTR_MASK_INTR_MASK_VOTE_REQ_ERR_MASK                             0x00000004
#define HSS2_CRG_INTR_CLEAR                    0x0338
#define HSS2_CRG_INTR_CLEAR_INTR_CLEAR_PLL_INI_ERR_SHIFT                           0
#define HSS2_CRG_INTR_CLEAR_INTR_CLEAR_PLL_INI_ERR_MASK                            0x00000001
#define HSS2_CRG_INTR_CLEAR_INTR_CLEAR_PLL_UNLOCK_SHIFT                            1
#define HSS2_CRG_INTR_CLEAR_INTR_CLEAR_PLL_UNLOCK_MASK                             0x00000002
#define HSS2_CRG_INTR_CLEAR_INTR_CLEAR_VOTE_REQ_ERR_SHIFT                          2
#define HSS2_CRG_INTR_CLEAR_INTR_CLEAR_VOTE_REQ_ERR_MASK                           0x00000004
#define HSS2_CRG_INTR_STATUS                   0x033c
#define HSS2_CRG_INTR_STATUS_INTR_PLL_INI_ERR_STATUS_SHIFT                         0
#define HSS2_CRG_INTR_STATUS_INTR_PLL_INI_ERR_STATUS_MASK                          0x00000001
#define HSS2_CRG_INTR_STATUS_INTR_PLL_UNLOCK_STATUS_SHIFT                          1
#define HSS2_CRG_INTR_STATUS_INTR_PLL_UNLOCK_STATUS_MASK                           0x00000002
#define HSS2_CRG_INTR_STATUS_INTR_VOTE_REQ_ERR_STATUS_SHIFT                        2
#define HSS2_CRG_INTR_STATUS_INTR_VOTE_REQ_ERR_STATUS_MASK                         0x00000004
#define HSS2_CRG_INTR_STATUS_MSK               0x0340
#define HSS2_CRG_INTR_STATUS_MSK_INTR_PLL_INI_ERR_STATUS_MSK_SHIFT                 0
#define HSS2_CRG_INTR_STATUS_MSK_INTR_PLL_INI_ERR_STATUS_MSK_MASK                  0x00000001
#define HSS2_CRG_INTR_STATUS_MSK_INTR_PLL_UNLOCK_STATUS_MSK_SHIFT                  1
#define HSS2_CRG_INTR_STATUS_MSK_INTR_PLL_UNLOCK_STATUS_MSK_MASK                   0x00000002
#define HSS2_CRG_INTR_STATUS_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_SHIFT                2
#define HSS2_CRG_INTR_STATUS_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_MASK                 0x00000004
#define HSS2_CRG_PCIE_PLL_VOTE_EN              0x0350
#define HSS2_CRG_PCIE_PLL_VOTE_EN_PLL_VOTE_EN_SHIFT                                0
#define HSS2_CRG_PCIE_PLL_VOTE_EN_PLL_VOTE_EN_MASK                                 0x000000ff
#define HSS2_CRG_PCIE_PLL_VOTE_CFG             0x0800
#define HSS2_CRG_PCIE_PLL_VOTE_CFG_PLL_START_FSM_BYPASS_SHIFT                      0
#define HSS2_CRG_PCIE_PLL_VOTE_CFG_PLL_START_FSM_BYPASS_MASK                       0x00000001
#define HSS2_CRG_PCIE_PLL_VOTE_CFG_PLL_LOCK_TIMEROUT_SHIFT                         1
#define HSS2_CRG_PCIE_PLL_VOTE_CFG_PLL_LOCK_TIMEROUT_MASK                          0x000003fe
#define HSS2_CRG_PCIE_PLL_VOTE_CFG_PLL_RETRY_NUM_SHIFT                             10
#define HSS2_CRG_PCIE_PLL_VOTE_CFG_PLL_RETRY_NUM_MASK                              0x00001c00
#define HSS2_CRG_PCIE_PLL_VOTE_CFG_PCIE_PLL_FSM_EN_SHIFT                           13
#define HSS2_CRG_PCIE_PLL_VOTE_CFG_PCIE_PLL_FSM_EN_MASK                            0x00002000
#define HSS2_CRG_PCIE_PLL_VOTE_CFG_PLL_CFG_RSV0_SHIFT                              14
#define HSS2_CRG_PCIE_PLL_VOTE_CFG_PLL_CFG_RSV0_MASK                               0xffffc000
#define HSS2_CRG_PCIE_PLL_PRESS_TEST           0x0804
#define HSS2_CRG_PCIE_PLL_PRESS_TEST_PLL_PRESS_TEST_EN_SHIFT                       0
#define HSS2_CRG_PCIE_PLL_PRESS_TEST_PLL_PRESS_TEST_EN_MASK                        0x00000001
#define HSS2_CRG_PCIE_PLL_PRESS_TEST_PLL_PRESS_TEST_CNT_SHIFT                      1
#define HSS2_CRG_PCIE_PLL_PRESS_TEST_PLL_PRESS_TEST_CNT_MASK                       0x01fffffe
#define HSS2_CRG_PCIE_PLL_PRESS_TEST_PLL_CFG_RSV1_SHIFT                            25
#define HSS2_CRG_PCIE_PLL_PRESS_TEST_PLL_CFG_RSV1_MASK                             0xfe000000
#define HSS2_CRG_PCIE_PLL_ERR_BOOT_CNT         0x0808
#define HSS2_CRG_PCIE_PLL_ERR_BOOT_CNT_PLL_ERR_BOOT_CNT_SHIFT                      0
#define HSS2_CRG_PCIE_PLL_ERR_BOOT_CNT_PLL_ERR_BOOT_CNT_MASK                       0x00ffffff
#define HSS2_CRG_PCIE_PLL_ERR_BOOT_CNT_PLL_PRESS_TEST_END_SHIFT                    24
#define HSS2_CRG_PCIE_PLL_ERR_BOOT_CNT_PLL_PRESS_TEST_END_MASK                     0x01000000
#define HSS2_CRG_PCIE_PLL_VOTE_MASK            0x080c
#define HSS2_CRG_PCIE_PLL_VOTE_MASK_PLL_HW_VOTE_MASK_SHIFT                         0
#define HSS2_CRG_PCIE_PLL_VOTE_MASK_PLL_HW_VOTE_MASK_MASK                          0x000000ff
#define HSS2_CRG_PCIE_PLL_VOTE_MASK_PLL_SFT_VOTE_MASK_SHIFT                        8
#define HSS2_CRG_PCIE_PLL_VOTE_MASK_PLL_SFT_VOTE_MASK_MASK                         0x0000ff00
#define HSS2_CRG_PCIE_PLL_VOTE_MASK_PLL_CFG_RSV2_SHIFT                             16
#define HSS2_CRG_PCIE_PLL_VOTE_MASK_PLL_CFG_RSV2_MASK                              0xffff0000
#define HSS2_CRG_PCIE_PLL_VOTE_BYPASS          0x0814
#define HSS2_CRG_PCIE_PLL_VOTE_BYPASS_PLL_VOTE_BYPASS_SHIFT                        0
#define HSS2_CRG_PCIE_PLL_VOTE_BYPASS_PLL_VOTE_BYPASS_MASK                         0x000000ff
#define HSS2_CRG_PCIE_PLL_VOTE_GT              0x0818
#define HSS2_CRG_PCIE_PLL_VOTE_GT_PLL_VOTE_GT_SHIFT                                0
#define HSS2_CRG_PCIE_PLL_VOTE_GT_PLL_VOTE_GT_MASK                                 0x000000ff
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG            0x081c
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG_PLL_SSMOD_EN_SHIFT                             0
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG_PLL_SSMOD_EN_MASK                              0x00000001
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG_PLL_DIVVAL_SHIFT                               1
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG_PLL_DIVVAL_MASK                                0x0000007e
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG_PLL_SPREAD_SHIFT                               7
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG_PLL_SPREAD_MASK                                0x00000f80
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG_PLL_DOWNSPREAD_SHIFT                           12
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG_PLL_DOWNSPREAD_MASK                            0x00001000
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG_PLL_CFG_RSV3_SHIFT                             13
#define HSS2_CRG_PCIE_PLL_SSMOD_CFG_PLL_CFG_RSV3_MASK                              0xffffe000
#define HSS2_CRG_PCIE_PLL_CTRL0                0x0820
#define HSS2_CRG_PCIE_PLL_CTRL0_PLL_EN_SHIFT                                       0
#define HSS2_CRG_PCIE_PLL_CTRL0_PLL_EN_MASK                                        0x00000001
#define HSS2_CRG_PCIE_PLL_CTRL0_PLL_BYPASS_SHIFT                                   1
#define HSS2_CRG_PCIE_PLL_CTRL0_PLL_BYPASS_MASK                                    0x00000002
#define HSS2_CRG_PCIE_PLL_CTRL0_REFDIV_SHIFT                                       2
#define HSS2_CRG_PCIE_PLL_CTRL0_REFDIV_MASK                                        0x000000fc
#define HSS2_CRG_PCIE_PLL_CTRL0_FBDIV_SHIFT                                        8
#define HSS2_CRG_PCIE_PLL_CTRL0_FBDIV_MASK                                         0x000fff00
#define HSS2_CRG_PCIE_PLL_CTRL0_POSTDIV1_SHIFT                                     20
#define HSS2_CRG_PCIE_PLL_CTRL0_POSTDIV1_MASK                                      0x00700000
#define HSS2_CRG_PCIE_PLL_CTRL0_POSTDIV2_SHIFT                                     23
#define HSS2_CRG_PCIE_PLL_CTRL0_POSTDIV2_MASK                                      0x03800000
#define HSS2_CRG_PCIE_PLL_CTRL0_CFG_VALID_SHIFT                                    26
#define HSS2_CRG_PCIE_PLL_CTRL0_CFG_VALID_MASK                                     0x04000000
#define HSS2_CRG_PCIE_PLL_CTRL0_PLL_GT_SHIFT                                       27
#define HSS2_CRG_PCIE_PLL_CTRL0_PLL_GT_MASK                                        0x08000000
#define HSS2_CRG_PCIE_PLL_CTRL0_PLL_CFG_RSV4_SHIFT                                 28
#define HSS2_CRG_PCIE_PLL_CTRL0_PLL_CFG_RSV4_MASK                                  0xf0000000
#define HSS2_CRG_PCIE_PLL_CTRL1                0x0824
#define HSS2_CRG_PCIE_PLL_CTRL1_FRAC_SHIFT                                         0
#define HSS2_CRG_PCIE_PLL_CTRL1_FRAC_MASK                                          0x00ffffff
#define HSS2_CRG_PCIE_PLL_CTRL1_DSM_EN_SHIFT                                       24
#define HSS2_CRG_PCIE_PLL_CTRL1_DSM_EN_MASK                                        0x01000000
#define HSS2_CRG_PCIE_PLL_CTRL1_PLL_CFG_RSV5_SHIFT                                 25
#define HSS2_CRG_PCIE_PLL_CTRL1_PLL_CFG_RSV5_MASK                                  0xfe000000
#define HSS2_CRG_PCIE_PLL_DEBUG                0x0828
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETCALBYP_SHIFT                                 0
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETCALBYP_MASK                                  0x00000001
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETCALCNT_SHIFT                                 1
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETCALCNT_MASK                                  0x0000000e
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETCALEN_SHIFT                                  4
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETCALEN_MASK                                   0x00000010
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETCALIN_SHIFT                                  5
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETCALIN_MASK                                   0x0001ffe0
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETCALRSTN_SHIFT                                17
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETCALRSTN_MASK                                 0x00020000
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETFASTCAL_SHIFT                                18
#define HSS2_CRG_PCIE_PLL_DEBUG_OFFSETFASTCAL_MASK                                 0x00040000
#define HSS2_CRG_PCIE_PLL_DEBUG_PLL_CFG_RSV6_SHIFT                                 19
#define HSS2_CRG_PCIE_PLL_DEBUG_PLL_CFG_RSV6_MASK                                  0xfff80000
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE          0x082c
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_PLL_EN_STATE_SHIFT                           0
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_PLL_EN_STATE_MASK                            0x00000001
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_OFFSETCALOUT_SHIFT                           1
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_OFFSETCALOUT_MASK                            0x00001ffe
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_OFFSETCALLOCK_SHIFT                          13
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_OFFSETCALLOCK_MASK                           0x00002000
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_OFFSETCALOVF_SHIFT                           14
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_OFFSETCALOVF_MASK                            0x00004000
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_PLL_START_FSM_STATE_SHIFT                    15
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_PLL_START_FSM_STATE_MASK                     0x00038000
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_PLL_REAL_RETRY_CNT_SHIFT                     18
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_PLL_REAL_RETRY_CNT_MASK                      0x001c0000
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_ST_CLK_PLL_SHIFT                             21
#define HSS2_CRG_PCIE_PLL_DEBUG_STATE_ST_CLK_PLL_MASK                              0x00200000
#define HSS2_CRG_REGFILE_CGBYPASS              0x0830
#define HSS2_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                           0
#define HSS2_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                            0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_hss2_bus_cfg :  6;
        unsigned int reserved_0           : 10;
        unsigned int _bm_                 : 16;
    } reg;
}hss2_crg_clkdiv0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gt_clk_hss2_bus_data :  1;
        unsigned int div_clk_hss2_bus_data   :  6;
        unsigned int reserved_0              :  9;
        unsigned int _bm_                    : 16;
    } reg;
}hss2_crg_clkdiv1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gt_clk_pcie0_fw :  1;
        unsigned int div_clk_pcie0_fw   :  6;
        unsigned int reserved_0         :  9;
        unsigned int _bm_               : 16;
    } reg;
}hss2_crg_clkdiv2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gt_hclk_emmc :  1;
        unsigned int div_hclk_emmc   :  6;
        unsigned int reserved_0      :  9;
        unsigned int _bm_            : 16;
    } reg;
}hss2_crg_clkdiv3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gt_clk_emmc_cclk_base :  1;
        unsigned int div_clk_emmc_cclk_base   :  6;
        unsigned int reserved_0               :  9;
        unsigned int _bm_                     : 16;
    } reg;
}hss2_crg_clkdiv4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_done_clk_hss2_bus_cfg   :  1;
        unsigned int div_done_clk_hss2_bus_data  :  1;
        unsigned int div_done_clk_pcie0_fw       :  1;
        unsigned int div_done_hclk_emmc          :  1;
        unsigned int div_done_clk_emmc_cclk_base :  1;
        unsigned int reserved_0                  : 27;
    } reg;
}hss2_crg_clkst0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_pcie0_apb     :  1;
        unsigned int gt_pclk_pcie1_apb     :  1;
        unsigned int gt_pclk_emmc          :  1;
        unsigned int gt_pclk_hss2_tcu_slv  :  1;
        unsigned int gt_aclk_pcie0_axi_slv :  1;
        unsigned int gt_aclk_pcie1_axi_slv :  1;
        unsigned int gt_clk_hss2_bus_data  :  1;
        unsigned int gt_aclk_hss2_tcu      :  1;
        unsigned int gt_aclk_hss2_tbu0     :  1;
        unsigned int gt_aclk_hss2_tbu1     :  1;
        unsigned int gt_aclk_pcie0_axi_mst :  1;
        unsigned int gt_aclk_pcie1_axi_mst :  1;
        unsigned int gt_aclk_perfmon_pcie0 :  1;
        unsigned int gt_aclk_perfmon_pcie1 :  1;
        unsigned int gt_aclk_perfmon_tcu   :  1;
        unsigned int gt_hclk_emmc          :  1;
        unsigned int gt_clk_pcie0_aux      :  1;
        unsigned int gt_clk_pcie1_aux      :  1;
        unsigned int gt_pclk_perfmon_pcie0 :  1;
        unsigned int gt_pclk_perfmon_pcie1 :  1;
        unsigned int gt_pclk_perfmon_tcu   :  1;
        unsigned int gt_clk_pcie_pll_logic :  1;
        unsigned int gt_clk_pcie0_tpc      :  1;
        unsigned int gt_clk_pcie1_tpc      :  1;
        unsigned int gt_clk_tcu_tpc        :  1;
        unsigned int reserved_0            :  2;
        unsigned int sc_gt_clk_pll_test    :  1;
        unsigned int gt_sw_clk_pcie0_fw    :  1;
        unsigned int gt_sw_clk_pcie1_fw    :  1;
        unsigned int reserved_1            :  2;
    } reg;
}hss2_crg_clkgt0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_pcie0_apb     :  1;
        unsigned int gt_pclk_pcie1_apb     :  1;
        unsigned int gt_pclk_emmc          :  1;
        unsigned int gt_pclk_hss2_tcu_slv  :  1;
        unsigned int gt_aclk_pcie0_axi_slv :  1;
        unsigned int gt_aclk_pcie1_axi_slv :  1;
        unsigned int gt_clk_hss2_bus_data  :  1;
        unsigned int gt_aclk_hss2_tcu      :  1;
        unsigned int gt_aclk_hss2_tbu0     :  1;
        unsigned int gt_aclk_hss2_tbu1     :  1;
        unsigned int gt_aclk_pcie0_axi_mst :  1;
        unsigned int gt_aclk_pcie1_axi_mst :  1;
        unsigned int gt_aclk_perfmon_pcie0 :  1;
        unsigned int gt_aclk_perfmon_pcie1 :  1;
        unsigned int gt_aclk_perfmon_tcu   :  1;
        unsigned int gt_hclk_emmc          :  1;
        unsigned int gt_clk_pcie0_aux      :  1;
        unsigned int gt_clk_pcie1_aux      :  1;
        unsigned int gt_pclk_perfmon_pcie0 :  1;
        unsigned int gt_pclk_perfmon_pcie1 :  1;
        unsigned int gt_pclk_perfmon_tcu   :  1;
        unsigned int gt_clk_pcie_pll_logic :  1;
        unsigned int gt_clk_pcie0_tpc      :  1;
        unsigned int gt_clk_pcie1_tpc      :  1;
        unsigned int gt_clk_tcu_tpc        :  1;
        unsigned int reserved_0            :  2;
        unsigned int sc_gt_clk_pll_test    :  1;
        unsigned int gt_sw_clk_pcie0_fw    :  1;
        unsigned int gt_sw_clk_pcie1_fw    :  1;
        unsigned int reserved_1            :  2;
    } reg;
}hss2_crg_clkgt0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_pcie0_apb     :  1;
        unsigned int gt_pclk_pcie1_apb     :  1;
        unsigned int gt_pclk_emmc          :  1;
        unsigned int gt_pclk_hss2_tcu_slv  :  1;
        unsigned int gt_aclk_pcie0_axi_slv :  1;
        unsigned int gt_aclk_pcie1_axi_slv :  1;
        unsigned int gt_clk_hss2_bus_data  :  1;
        unsigned int gt_aclk_hss2_tcu      :  1;
        unsigned int gt_aclk_hss2_tbu0     :  1;
        unsigned int gt_aclk_hss2_tbu1     :  1;
        unsigned int gt_aclk_pcie0_axi_mst :  1;
        unsigned int gt_aclk_pcie1_axi_mst :  1;
        unsigned int gt_aclk_perfmon_pcie0 :  1;
        unsigned int gt_aclk_perfmon_pcie1 :  1;
        unsigned int gt_aclk_perfmon_tcu   :  1;
        unsigned int gt_hclk_emmc          :  1;
        unsigned int gt_clk_pcie0_aux      :  1;
        unsigned int gt_clk_pcie1_aux      :  1;
        unsigned int gt_pclk_perfmon_pcie0 :  1;
        unsigned int gt_pclk_perfmon_pcie1 :  1;
        unsigned int gt_pclk_perfmon_tcu   :  1;
        unsigned int gt_clk_pcie_pll_logic :  1;
        unsigned int gt_clk_pcie0_tpc      :  1;
        unsigned int gt_clk_pcie1_tpc      :  1;
        unsigned int gt_clk_tcu_tpc        :  1;
        unsigned int reserved_0            :  2;
        unsigned int sc_gt_clk_pll_test    :  1;
        unsigned int gt_sw_clk_pcie0_fw    :  1;
        unsigned int gt_sw_clk_pcie1_fw    :  1;
        unsigned int reserved_1            :  2;
    } reg;
}hss2_crg_clkgt0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_pclk_pcie0_apb     :  1;
        unsigned int st_pclk_pcie1_apb     :  1;
        unsigned int st_pclk_emmc          :  1;
        unsigned int st_pclk_hss2_tcu_slv  :  1;
        unsigned int st_aclk_pcie0_axi_slv :  1;
        unsigned int st_aclk_pcie1_axi_slv :  1;
        unsigned int st_clk_hss2_bus_data  :  1;
        unsigned int st_aclk_hss2_tcu      :  1;
        unsigned int st_aclk_hss2_tbu0     :  1;
        unsigned int st_aclk_hss2_tbu1     :  1;
        unsigned int st_aclk_pcie0_axi_mst :  1;
        unsigned int st_aclk_pcie1_axi_mst :  1;
        unsigned int st_aclk_perfmon_pcie0 :  1;
        unsigned int st_aclk_perfmon_pcie1 :  1;
        unsigned int st_aclk_perfmon_tcu   :  1;
        unsigned int st_hclk_emmc          :  1;
        unsigned int st_clk_pcie0_aux      :  1;
        unsigned int st_clk_pcie1_aux      :  1;
        unsigned int st_pclk_perfmon_pcie0 :  1;
        unsigned int st_pclk_perfmon_pcie1 :  1;
        unsigned int st_pclk_perfmon_tcu   :  1;
        unsigned int st_clk_pcie_pll_logic :  1;
        unsigned int st_clk_pcie0_tpc      :  1;
        unsigned int st_clk_pcie1_tpc      :  1;
        unsigned int st_clk_tcu_tpc        :  1;
        unsigned int st_hclk_emmc_slv      :  1;
        unsigned int st_aclk_emmc_mst      :  1;
        unsigned int reserved_0            :  1;
        unsigned int st_clk_pcie0_fw       :  1;
        unsigned int st_clk_pcie1_fw       :  1;
        unsigned int reserved_1            :  2;
    } reg;
}hss2_crg_clkst1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_pll_logic_n      :  1;
        unsigned int ip_rst_ssmod_n          :  1;
        unsigned int ip_rst_pcie0_power_up_n :  1;
        unsigned int ip_prst_pcie0_dbi_n     :  1;
        unsigned int ip_rst_pcie0_aux_n      :  1;
        unsigned int reserved_0              :  1;
        unsigned int ip_prst_pcie0_n         :  1;
        unsigned int ip_rst_pcie1_power_up_n :  1;
        unsigned int ip_prst_pcie1_dbi_n     :  1;
        unsigned int ip_rst_pcie1_aux_n      :  1;
        unsigned int reserved_1              :  1;
        unsigned int ip_prst_pcie1_n         :  1;
        unsigned int ip_arst_hss2_tbu0_n     :  1;
        unsigned int ip_arst_hss2_tbu1_n     :  1;
        unsigned int ip_arst_hss2_tcu_n      :  1;
        unsigned int ip_rst_hss2_bus_data_n  :  1;
        unsigned int ip_arst_perfmon_pcie0_n :  1;
        unsigned int ip_arst_perfmon_pcie1_n :  1;
        unsigned int ip_arst_perfmon_tcu_n   :  1;
        unsigned int ip_prst_emmc_n          :  1;
        unsigned int ip_rst_emmc_n           :  1;
        unsigned int ip_arst_emmc_slv_n      :  1;
        unsigned int ip_hrst_emmc_mst_n      :  1;
        unsigned int ip_rst_pcie0_tpc_n      :  1;
        unsigned int ip_rst_pcie1_tpc_n      :  1;
        unsigned int ip_rst_tcu_tpc_n        :  1;
        unsigned int reserved_2              :  6;
    } reg;
}hss2_crg_rst0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_pll_logic_n      :  1;
        unsigned int ip_rst_ssmod_n          :  1;
        unsigned int ip_rst_pcie0_power_up_n :  1;
        unsigned int ip_prst_pcie0_dbi_n     :  1;
        unsigned int ip_rst_pcie0_aux_n      :  1;
        unsigned int reserved_0              :  1;
        unsigned int ip_prst_pcie0_n         :  1;
        unsigned int ip_rst_pcie1_power_up_n :  1;
        unsigned int ip_prst_pcie1_dbi_n     :  1;
        unsigned int ip_rst_pcie1_aux_n      :  1;
        unsigned int reserved_1              :  1;
        unsigned int ip_prst_pcie1_n         :  1;
        unsigned int ip_arst_hss2_tbu0_n     :  1;
        unsigned int ip_arst_hss2_tbu1_n     :  1;
        unsigned int ip_arst_hss2_tcu_n      :  1;
        unsigned int ip_rst_hss2_bus_data_n  :  1;
        unsigned int ip_arst_perfmon_pcie0_n :  1;
        unsigned int ip_arst_perfmon_pcie1_n :  1;
        unsigned int ip_arst_perfmon_tcu_n   :  1;
        unsigned int ip_prst_emmc_n          :  1;
        unsigned int ip_rst_emmc_n           :  1;
        unsigned int ip_arst_emmc_slv_n      :  1;
        unsigned int ip_hrst_emmc_mst_n      :  1;
        unsigned int ip_rst_pcie0_tpc_n      :  1;
        unsigned int ip_rst_pcie1_tpc_n      :  1;
        unsigned int ip_rst_tcu_tpc_n        :  1;
        unsigned int reserved_2              :  6;
    } reg;
}hss2_crg_rst0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_pll_logic_n      :  1;
        unsigned int ip_rst_ssmod_n          :  1;
        unsigned int ip_rst_pcie0_power_up_n :  1;
        unsigned int ip_prst_pcie0_dbi_n     :  1;
        unsigned int ip_rst_pcie0_aux_n      :  1;
        unsigned int reserved_0              :  1;
        unsigned int ip_prst_pcie0_n         :  1;
        unsigned int ip_rst_pcie1_power_up_n :  1;
        unsigned int ip_prst_pcie1_dbi_n     :  1;
        unsigned int ip_rst_pcie1_aux_n      :  1;
        unsigned int reserved_1              :  1;
        unsigned int ip_prst_pcie1_n         :  1;
        unsigned int ip_arst_hss2_tbu0_n     :  1;
        unsigned int ip_arst_hss2_tbu1_n     :  1;
        unsigned int ip_arst_hss2_tcu_n      :  1;
        unsigned int ip_rst_hss2_bus_data_n  :  1;
        unsigned int ip_arst_perfmon_pcie0_n :  1;
        unsigned int ip_arst_perfmon_pcie1_n :  1;
        unsigned int ip_arst_perfmon_tcu_n   :  1;
        unsigned int ip_prst_emmc_n          :  1;
        unsigned int ip_rst_emmc_n           :  1;
        unsigned int ip_arst_emmc_slv_n      :  1;
        unsigned int ip_hrst_emmc_mst_n      :  1;
        unsigned int ip_rst_pcie0_tpc_n      :  1;
        unsigned int ip_rst_pcie1_tpc_n      :  1;
        unsigned int ip_rst_tcu_tpc_n        :  1;
        unsigned int reserved_2              :  6;
    } reg;
}hss2_crg_rst0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clkrst_flag_pcie1_phy_apb1_bypass :  1;
        unsigned int clkrst_flag_pcie1_phy_apb0_bypass :  1;
        unsigned int clkrst_flag_pcie1_phy_bypass      :  1;
        unsigned int clkrst_flag_pcie1_ctrl_sii_bypass :  1;
        unsigned int clkrst_flag_pcie1_ctrl_dbi_bypass :  1;
        unsigned int clkrst_flag_pcie0_phy_apb1_bypass :  1;
        unsigned int clkrst_flag_pcie0_phy_apb0_bypass :  1;
        unsigned int clkrst_flag_pcie0_phy_bypass      :  1;
        unsigned int clkrst_flag_pcie0_ctrl_sii_bypass :  1;
        unsigned int clkrst_flag_pcie0_ctrl_dbi_bypass :  1;
        unsigned int clkrst_flag_emmc_bypass           :  1;
        unsigned int clkrst_flag_smmu_bypass           :  1;
        unsigned int reserved_0                        : 20;
    } reg;
}hss2_crg_hss2ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_auto_clk_pcie0_fw_bypass :  1;
        unsigned int gt_auto_clk_pcie1_fw_bypass :  1;
        unsigned int reserved_0                  : 14;
        unsigned int _bm_                        : 16;
    } reg;
}hss2_crg_hss2ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hss2_bus_ini_no_pending_trans_data_bypass        :  5;
        unsigned int hss2_bus_tgt_no_pending_trans_data_bypass        :  7;
        unsigned int idle_flag_smmu_tcu_data_bypass                   :  1;
        unsigned int idle_flag_perfstat_pcie1_data_bypass             :  1;
        unsigned int idle_flag_perfstat_pcie0_data_bypass             :  1;
        unsigned int idle_flag_perfstat_tcu_data_bypass               :  1;
        unsigned int cfgbus2hss2_bus_tgt_no_pending_trans_data_bypass :  1;
        unsigned int reserved_0                                       : 15;
    } reg;
}hss2_crg_autofsctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_hss2_bus_data     : 10;
        unsigned int debounce_out_hss2_bus_data    : 10;
        unsigned int debounce_bypass_hss2_bus_data :  1;
        unsigned int div_auto_clk_hss2_bus_data    :  6;
        unsigned int reserved_0                    :  4;
        unsigned int autofs_en_hss2_bus_data       :  1;
    } reg;
}hss2_crg_autofsctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hss2_bus_ini_no_pending_trans_cfg_bypass        :  5;
        unsigned int hss2_bus_tgt_no_pending_trans_cfg_bypass        :  7;
        unsigned int idle_flag_smmu_tcu_cfg_bypass                   :  1;
        unsigned int idle_flag_perfstat_pcie1_cfg_bypass             :  1;
        unsigned int idle_flag_perfstat_pcie0_cfg_bypass             :  1;
        unsigned int idle_flag_perfstat_tcu_cfg_bypass               :  1;
        unsigned int cfgbus2hss2_bus_tgt_no_pending_trans_cfg_bypass :  1;
        unsigned int reserved_0                                      : 15;
    } reg;
}hss2_crg_autofsctrl2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_hss2_bus_cfg     : 10;
        unsigned int debounce_out_hss2_bus_cfg    : 10;
        unsigned int debounce_bypass_hss2_bus_cfg :  1;
        unsigned int div_auto_clk_hss2_bus_cfg    :  6;
        unsigned int reserved_0                   :  4;
        unsigned int autofs_en_hss2_bus_cfg       :  1;
    } reg;
}hss2_crg_autofsctrl3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int idle_flag_smmu_tcu       :  1;
        unsigned int idle_flag_perfstat_pcie1 :  1;
        unsigned int idle_flag_perfstat_pcie0 :  1;
        unsigned int idle_flag_perfstat_tcu   :  1;
        unsigned int reserved_0               : 28;
    } reg;
}hss2_crg_autofsctrl4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_hss2_bus_data : 16;
        unsigned int _bm_                        : 16;
    } reg;
}hss2_crg_autofsdisable0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_hss2_bus_cfg : 16;
        unsigned int _bm_                       : 16;
    } reg;
}hss2_crg_autofsdisable1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_lock   :  1;
        unsigned int reserved_0 : 31;
    } reg;
}hss2_crg_pcie_pll_lock_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err  :  1;
        unsigned int intr_mask_pll_unlock   :  1;
        unsigned int intr_mask_vote_req_err :  1;
        unsigned int reserved_0             : 29;
    } reg;
}hss2_crg_intr_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err  :  1;
        unsigned int intr_clear_pll_unlock   :  1;
        unsigned int intr_clear_vote_req_err :  1;
        unsigned int reserved_0              : 13;
        unsigned int _bm_                    : 16;
    } reg;
}hss2_crg_intr_clear_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pll_ini_err_status  :  1;
        unsigned int intr_pll_unlock_status   :  1;
        unsigned int intr_vote_req_err_status :  1;
        unsigned int reserved_0               : 29;
    } reg;
}hss2_crg_intr_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pll_ini_err_status_msk  :  1;
        unsigned int intr_pll_unlock_status_msk   :  1;
        unsigned int intr_vote_req_err_status_msk :  1;
        unsigned int reserved_0                   : 29;
    } reg;
}hss2_crg_intr_status_msk_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_vote_en :  8;
        unsigned int reserved_0  :  8;
        unsigned int _bm_        : 16;
    } reg;
}hss2_crg_pcie_pll_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_start_fsm_bypass :  1;
        unsigned int pll_lock_timerout    :  9;
        unsigned int pll_retry_num        :  3;
        unsigned int pcie_pll_fsm_en      :  1;
        unsigned int pll_cfg_rsv0         : 18;
    } reg;
}hss2_crg_pcie_pll_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_press_test_en  :  1;
        unsigned int pll_press_test_cnt : 24;
        unsigned int pll_cfg_rsv1       :  7;
    } reg;
}hss2_crg_pcie_pll_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_err_boot_cnt   : 24;
        unsigned int pll_press_test_end :  1;
        unsigned int reserved_0         :  7;
    } reg;
}hss2_crg_pcie_pll_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_hw_vote_mask  :  8;
        unsigned int pll_sft_vote_mask :  8;
        unsigned int pll_cfg_rsv2      : 16;
    } reg;
}hss2_crg_pcie_pll_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_vote_bypass :  8;
        unsigned int reserved_0      :  8;
        unsigned int _bm_            : 16;
    } reg;
}hss2_crg_pcie_pll_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_vote_gt :  8;
        unsigned int reserved_0  :  8;
        unsigned int _bm_        : 16;
    } reg;
}hss2_crg_pcie_pll_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_ssmod_en   :  1;
        unsigned int pll_divval     :  6;
        unsigned int pll_spread     :  5;
        unsigned int pll_downspread :  1;
        unsigned int pll_cfg_rsv3   : 19;
    } reg;
}hss2_crg_pcie_pll_ssmod_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_en       :  1;
        unsigned int pll_bypass   :  1;
        unsigned int refdiv       :  6;
        unsigned int fbdiv        : 12;
        unsigned int postdiv1     :  3;
        unsigned int postdiv2     :  3;
        unsigned int cfg_valid    :  1;
        unsigned int pll_gt       :  1;
        unsigned int pll_cfg_rsv4 :  4;
    } reg;
}hss2_crg_pcie_pll_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int frac         : 24;
        unsigned int dsm_en       :  1;
        unsigned int pll_cfg_rsv5 :  7;
    } reg;
}hss2_crg_pcie_pll_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int offsetcalbyp  :  1;
        unsigned int offsetcalcnt  :  3;
        unsigned int offsetcalen   :  1;
        unsigned int offsetcalin   : 12;
        unsigned int offsetcalrstn :  1;
        unsigned int offsetfastcal :  1;
        unsigned int pll_cfg_rsv6  : 13;
    } reg;
}hss2_crg_pcie_pll_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pll_en_state        :  1;
        unsigned int offsetcalout        : 12;
        unsigned int offsetcallock       :  1;
        unsigned int offsetcalovf        :  1;
        unsigned int pll_start_fsm_state :  3;
        unsigned int pll_real_retry_cnt  :  3;
        unsigned int st_clk_pll          :  1;
        unsigned int reserved_0          : 10;
    } reg;
}hss2_crg_pcie_pll_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}hss2_crg_regfile_cgbypass_t;

#endif
