// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef MEDIA1_CRG_REGIF_H
#define MEDIA1_CRG_REGIF_H

#define MEDIA1_CRG_CLKSW0                               0x0000
#define MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_CORE_SHIFT                             0
#define MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_CORE_MASK                              0x00000003
#define MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_VEU_SHIFT                              2
#define MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_VEU_MASK                               0x0000000c
#define MEDIA1_CRG_CLKSW0_SEL_CLK_VDEC_CORE_SHIFT                            4
#define MEDIA1_CRG_CLKSW0_SEL_CLK_VDEC_CORE_MASK                             0x00000030
#define MEDIA1_CRG_CLKSW0_SEL_ACLK_MEDIA1_BUS_SHIFT                          6
#define MEDIA1_CRG_CLKSW0_SEL_ACLK_MEDIA1_BUS_MASK                           0x000000c0
#define MEDIA1_CRG_CLKSW0_SEL_PCLK_MEDIA1_CFG_SHIFT                          8
#define MEDIA1_CRG_CLKSW0_SEL_PCLK_MEDIA1_CFG_MASK                           0x00000300
#define MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_DSC1_SHIFT                             10
#define MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_DSC1_MASK                              0x00000400
#define MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_DSC0_SHIFT                             11
#define MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_DSC0_MASK                              0x00000800
#define MEDIA1_CRG_CLKST0                               0x0004
#define MEDIA1_CRG_CLKST0_SW_ACK_CLK_DPU_CORE_SHIFT                          0
#define MEDIA1_CRG_CLKST0_SW_ACK_CLK_DPU_CORE_MASK                           0x0000000f
#define MEDIA1_CRG_CLKST0_SW_ACK_CLK_DPU_VEU_SHIFT                           4
#define MEDIA1_CRG_CLKST0_SW_ACK_CLK_DPU_VEU_MASK                            0x000000f0
#define MEDIA1_CRG_CLKST0_SW_ACK_CLK_VDEC_CORE_SHIFT                         8
#define MEDIA1_CRG_CLKST0_SW_ACK_CLK_VDEC_CORE_MASK                          0x00000f00
#define MEDIA1_CRG_CLKST0_SW_ACK_ACLK_MEDIA1_BUS_SHIFT                       12
#define MEDIA1_CRG_CLKST0_SW_ACK_ACLK_MEDIA1_BUS_MASK                        0x0000f000
#define MEDIA1_CRG_CLKST0_SW_ACK_PCLK_MEDIA1_CFG_SHIFT                       16
#define MEDIA1_CRG_CLKST0_SW_ACK_PCLK_MEDIA1_CFG_MASK                        0x000f0000
#define MEDIA1_CRG_CLKDIV1                              0x0008
#define MEDIA1_CRG_CLKDIV1_SC_GT_CLK_DPU_CORE_SHIFT                          0
#define MEDIA1_CRG_CLKDIV1_SC_GT_CLK_DPU_CORE_MASK                           0x00000001
#define MEDIA1_CRG_CLKDIV1_DIV_SW_CLK_DPU_CORE_SHIFT                         1
#define MEDIA1_CRG_CLKDIV1_DIV_SW_CLK_DPU_CORE_MASK                          0x0000007e
#define MEDIA1_CRG_CLKDIV1_SC_GT_CLK_DPU_VEU_SHIFT                           7
#define MEDIA1_CRG_CLKDIV1_SC_GT_CLK_DPU_VEU_MASK                            0x00000080
#define MEDIA1_CRG_CLKDIV1_DIV_SW_CLK_DPU_VEU_SHIFT                          8
#define MEDIA1_CRG_CLKDIV1_DIV_SW_CLK_DPU_VEU_MASK                           0x00003f00
#define MEDIA1_CRG_CLKDIV2                              0x000c
#define MEDIA1_CRG_CLKDIV2_SC_GT_CLK_VDEC_CORE_SHIFT                         0
#define MEDIA1_CRG_CLKDIV2_SC_GT_CLK_VDEC_CORE_MASK                          0x00000001
#define MEDIA1_CRG_CLKDIV2_DIV_CLK_VDEC_CORE_SHIFT                           1
#define MEDIA1_CRG_CLKDIV2_DIV_CLK_VDEC_CORE_MASK                            0x0000007e
#define MEDIA1_CRG_CLKDIV2_SC_GT_ACLK_MEDIA1_BUS_SHIFT                       7
#define MEDIA1_CRG_CLKDIV2_SC_GT_ACLK_MEDIA1_BUS_MASK                        0x00000080
#define MEDIA1_CRG_CLKDIV2_DIV_SW_ACLK_MEDIA1_BUS_SHIFT                      8
#define MEDIA1_CRG_CLKDIV2_DIV_SW_ACLK_MEDIA1_BUS_MASK                       0x00003f00
#define MEDIA1_CRG_CLKDIV3                              0x0010
#define MEDIA1_CRG_CLKDIV3_DIV_SW_PCLK_MEDIA1_CFG_SHIFT                      0
#define MEDIA1_CRG_CLKDIV3_DIV_SW_PCLK_MEDIA1_CFG_MASK                       0x0000003f
#define MEDIA1_CRG_CLKDIV4                              0x0014
#define MEDIA1_CRG_CLKDIV4_SC_GT_CLK_MEDIA1_HPM_SHIFT                        0
#define MEDIA1_CRG_CLKDIV4_SC_GT_CLK_MEDIA1_HPM_MASK                         0x00000001
#define MEDIA1_CRG_CLKDIV4_DIV_CLK_MEDIA1_HPM_SHIFT                          1
#define MEDIA1_CRG_CLKDIV4_DIV_CLK_MEDIA1_HPM_MASK                           0x0000007e
#define MEDIA1_CRG_CLKDIV4_SC_GT_CLK_MEDIA1_GPC_SHIFT                        7
#define MEDIA1_CRG_CLKDIV4_SC_GT_CLK_MEDIA1_GPC_MASK                         0x00000080
#define MEDIA1_CRG_CLKDIV4_DIV_CLK_MEDIA1_GPC_SHIFT                          8
#define MEDIA1_CRG_CLKDIV4_DIV_CLK_MEDIA1_GPC_MASK                           0x00003f00
#define MEDIA1_CRG_CLKDIV5                              0x0018
#define MEDIA1_CRG_CLKDIV5_DIV_CLK_DPU_DSC1_SHIFT                            0
#define MEDIA1_CRG_CLKDIV5_DIV_CLK_DPU_DSC1_MASK                             0x0000003f
#define MEDIA1_CRG_CLKDIV5_DIV_CLK_DPU_DSC0_SHIFT                            6
#define MEDIA1_CRG_CLKDIV5_DIV_CLK_DPU_DSC0_MASK                             0x00000fc0
#define MEDIA1_CRG_CLKST1                               0x001c
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_DSC0_SHIFT                        0
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_DSC0_MASK                         0x00000001
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_DSC1_SHIFT                        1
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_DSC1_MASK                         0x00000002
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_MEDIA1_HPM_SHIFT                      2
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_MEDIA1_HPM_MASK                       0x00000004
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_MEDIA1_GPC_SHIFT                      3
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_MEDIA1_GPC_MASK                       0x00000008
#define MEDIA1_CRG_CLKST1_DIV_DONE_PCLK_MEDIA1_CFG_SHIFT                     4
#define MEDIA1_CRG_CLKST1_DIV_DONE_PCLK_MEDIA1_CFG_MASK                      0x00000010
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_VDEC_CORE_SHIFT                       5
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_VDEC_CORE_MASK                        0x00000020
#define MEDIA1_CRG_CLKST1_DIV_DONE_ACLK_MEDIA1_BUS_SHIFT                     6
#define MEDIA1_CRG_CLKST1_DIV_DONE_ACLK_MEDIA1_BUS_MASK                      0x00000040
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_CORE_SHIFT                        7
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_CORE_MASK                         0x00000080
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_VEU_SHIFT                         8
#define MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_VEU_MASK                          0x00000100
#define MEDIA1_CRG_CLKGT0_W1S                           0x0020
#define MEDIA1_CRG_CLKGT0_W1S_GT_ACLK_MEDIA1_BUS_SHIFT                       3
#define MEDIA1_CRG_CLKGT0_W1S_GT_ACLK_MEDIA1_TCU_SHIFT                       4
#define MEDIA1_CRG_CLKGT0_W1S_GT_PCLK_DSI_CFG_SHIFT                          9
#define MEDIA1_CRG_CLKGT0_W1S_GT_ACLK_MEDIA1_BIST_SW_SHIFT                   10
#define MEDIA1_CRG_CLKGT0_W1S_GT_CLK_MEDIA1_HPM_SHIFT                        11
#define MEDIA1_CRG_CLKGT0_W1S_GT_CLK_MEDIA1_GPC0_SHIFT                       12
#define MEDIA1_CRG_CLKGT0_W1S_GT_CLK_MEDIA1_GPC1_SHIFT                       13
#define MEDIA1_CRG_CLKGT0_W1S_GT_CLK_MEDIA1_GPC2_SHIFT                       14
#define MEDIA1_CRG_CLKGT0_W1S_GT_CLK_MEDIA1_GPC3_SHIFT                       15
#define MEDIA1_CRG_CLKGT0_W1S_GT_ACLK_MEDIA1_PERF_SHIFT                      16
#define MEDIA1_CRG_CLKGT0_W1S_GT_PCLK_MEDIA1_BIST_SHIFT                      17
#define MEDIA1_CRG_CLKGT0_W1S_GT_PCLK_MEDIA1_PERF_SHIFT                      18
#define MEDIA1_CRG_CLKGT0_W1S_GT_PCLK_MEDIA1_TCU_SHIFT                       19
#define MEDIA1_CRG_CLKGT0_W1C                           0x0024
#define MEDIA1_CRG_CLKGT0_W1C_GT_ACLK_MEDIA1_BUS_SHIFT                       3
#define MEDIA1_CRG_CLKGT0_W1C_GT_ACLK_MEDIA1_TCU_SHIFT                       4
#define MEDIA1_CRG_CLKGT0_W1C_GT_PCLK_DSI_CFG_SHIFT                          9
#define MEDIA1_CRG_CLKGT0_W1C_GT_ACLK_MEDIA1_BIST_SW_SHIFT                   10
#define MEDIA1_CRG_CLKGT0_W1C_GT_CLK_MEDIA1_HPM_SHIFT                        11
#define MEDIA1_CRG_CLKGT0_W1C_GT_CLK_MEDIA1_GPC0_SHIFT                       12
#define MEDIA1_CRG_CLKGT0_W1C_GT_CLK_MEDIA1_GPC1_SHIFT                       13
#define MEDIA1_CRG_CLKGT0_W1C_GT_CLK_MEDIA1_GPC2_SHIFT                       14
#define MEDIA1_CRG_CLKGT0_W1C_GT_CLK_MEDIA1_GPC3_SHIFT                       15
#define MEDIA1_CRG_CLKGT0_W1C_GT_ACLK_MEDIA1_PERF_SHIFT                      16
#define MEDIA1_CRG_CLKGT0_W1C_GT_PCLK_MEDIA1_BIST_SHIFT                      17
#define MEDIA1_CRG_CLKGT0_W1C_GT_PCLK_MEDIA1_PERF_SHIFT                      18
#define MEDIA1_CRG_CLKGT0_W1C_GT_PCLK_MEDIA1_TCU_SHIFT                       19
#define MEDIA1_CRG_CLKGT0_RO                            0x0028
#define MEDIA1_CRG_CLKGT0_RO_GT_ACLK_MEDIA1_BUS_SHIFT                        3
#define MEDIA1_CRG_CLKGT0_RO_GT_ACLK_MEDIA1_BUS_MASK                         0x00000008
#define MEDIA1_CRG_CLKGT0_RO_GT_ACLK_MEDIA1_TCU_SHIFT                        4
#define MEDIA1_CRG_CLKGT0_RO_GT_ACLK_MEDIA1_TCU_MASK                         0x00000010
#define MEDIA1_CRG_CLKGT0_RO_GT_PCLK_DSI_CFG_SHIFT                           9
#define MEDIA1_CRG_CLKGT0_RO_GT_PCLK_DSI_CFG_MASK                            0x00000200
#define MEDIA1_CRG_CLKGT0_RO_GT_ACLK_MEDIA1_BIST_SW_SHIFT                    10
#define MEDIA1_CRG_CLKGT0_RO_GT_ACLK_MEDIA1_BIST_SW_MASK                     0x00000400
#define MEDIA1_CRG_CLKGT0_RO_GT_CLK_MEDIA1_HPM_SHIFT                         11
#define MEDIA1_CRG_CLKGT0_RO_GT_CLK_MEDIA1_HPM_MASK                          0x00000800
#define MEDIA1_CRG_CLKGT0_RO_GT_CLK_MEDIA1_GPC0_SHIFT                        12
#define MEDIA1_CRG_CLKGT0_RO_GT_CLK_MEDIA1_GPC0_MASK                         0x00001000
#define MEDIA1_CRG_CLKGT0_RO_GT_CLK_MEDIA1_GPC1_SHIFT                        13
#define MEDIA1_CRG_CLKGT0_RO_GT_CLK_MEDIA1_GPC1_MASK                         0x00002000
#define MEDIA1_CRG_CLKGT0_RO_GT_CLK_MEDIA1_GPC2_SHIFT                        14
#define MEDIA1_CRG_CLKGT0_RO_GT_CLK_MEDIA1_GPC2_MASK                         0x00004000
#define MEDIA1_CRG_CLKGT0_RO_GT_CLK_MEDIA1_GPC3_SHIFT                        15
#define MEDIA1_CRG_CLKGT0_RO_GT_CLK_MEDIA1_GPC3_MASK                         0x00008000
#define MEDIA1_CRG_CLKGT0_RO_GT_ACLK_MEDIA1_PERF_SHIFT                       16
#define MEDIA1_CRG_CLKGT0_RO_GT_ACLK_MEDIA1_PERF_MASK                        0x00010000
#define MEDIA1_CRG_CLKGT0_RO_GT_PCLK_MEDIA1_BIST_SHIFT                       17
#define MEDIA1_CRG_CLKGT0_RO_GT_PCLK_MEDIA1_BIST_MASK                        0x00020000
#define MEDIA1_CRG_CLKGT0_RO_GT_PCLK_MEDIA1_PERF_SHIFT                       18
#define MEDIA1_CRG_CLKGT0_RO_GT_PCLK_MEDIA1_PERF_MASK                        0x00040000
#define MEDIA1_CRG_CLKGT0_RO_GT_PCLK_MEDIA1_TCU_SHIFT                        19
#define MEDIA1_CRG_CLKGT0_RO_GT_PCLK_MEDIA1_TCU_MASK                         0x00080000
#define MEDIA1_CRG_CLKST2                               0x0030
#define MEDIA1_CRG_CLKST2_ST_ACLK_MEDIA1_BUS_SHIFT                           3
#define MEDIA1_CRG_CLKST2_ST_ACLK_MEDIA1_BUS_MASK                            0x00000008
#define MEDIA1_CRG_CLKST2_ST_ACLK_MEDIA1_TCU_SHIFT                           4
#define MEDIA1_CRG_CLKST2_ST_ACLK_MEDIA1_TCU_MASK                            0x00000010
#define MEDIA1_CRG_CLKST2_ST_PCLK_DSI_CFG_SHIFT                              9
#define MEDIA1_CRG_CLKST2_ST_PCLK_DSI_CFG_MASK                               0x00000200
#define MEDIA1_CRG_CLKST2_ST_ACLK_MEDIA1_BIST_SW_SHIFT                       10
#define MEDIA1_CRG_CLKST2_ST_ACLK_MEDIA1_BIST_SW_MASK                        0x00000400
#define MEDIA1_CRG_CLKST2_ST_CLK_MEDIA1_HPM_SHIFT                            11
#define MEDIA1_CRG_CLKST2_ST_CLK_MEDIA1_HPM_MASK                             0x00000800
#define MEDIA1_CRG_CLKST2_ST_CLK_MEDIA1_GPC0_SHIFT                           12
#define MEDIA1_CRG_CLKST2_ST_CLK_MEDIA1_GPC0_MASK                            0x00001000
#define MEDIA1_CRG_CLKST2_ST_CLK_MEDIA1_GPC1_SHIFT                           13
#define MEDIA1_CRG_CLKST2_ST_CLK_MEDIA1_GPC1_MASK                            0x00002000
#define MEDIA1_CRG_CLKST2_ST_CLK_MEDIA1_GPC2_SHIFT                           14
#define MEDIA1_CRG_CLKST2_ST_CLK_MEDIA1_GPC2_MASK                            0x00004000
#define MEDIA1_CRG_CLKST2_ST_CLK_MEDIA1_GPC3_SHIFT                           15
#define MEDIA1_CRG_CLKST2_ST_CLK_MEDIA1_GPC3_MASK                            0x00008000
#define MEDIA1_CRG_CLKST2_ST_ACLK_MEDIA1_PERF_SHIFT                          16
#define MEDIA1_CRG_CLKST2_ST_ACLK_MEDIA1_PERF_MASK                           0x00010000
#define MEDIA1_CRG_CLKST2_ST_PCLK_MEDIA1_BIST_SHIFT                          17
#define MEDIA1_CRG_CLKST2_ST_PCLK_MEDIA1_BIST_MASK                           0x00020000
#define MEDIA1_CRG_CLKST2_ST_PCLK_MEDIA1_PERF_SHIFT                          18
#define MEDIA1_CRG_CLKST2_ST_PCLK_MEDIA1_PERF_MASK                           0x00040000
#define MEDIA1_CRG_CLKST2_ST_PCLK_MEDIA1_TCU_SHIFT                           19
#define MEDIA1_CRG_CLKST2_ST_PCLK_MEDIA1_TCU_MASK                            0x00080000
#define MEDIA1_CRG_CLKGT1_W1S                           0x0040
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_CORE0_SHIFT                         0
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_CORE1_SHIFT                         1
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_CORE2_SHIFT                         2
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_CORE3_SHIFT                         3
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_DSC0_SHIFT                          5
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_DSC1_SHIFT                          6
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_VEU_SHIFT                           7
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI0_SHIFT                          8
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI1_SHIFT                          9
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI2_SHIFT                          10
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI3_SHIFT                          11
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_VEU_AXI_SHIFT                       12
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DISP_BUS_DATA_SHIFT                     13
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI0_PERF_SHIFT                     14
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI1_PERF_SHIFT                     15
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI2_PERF_SHIFT                     16
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI3_PERF_SHIFT                     17
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_VEU_AXI_PERF_SHIFT                  18
#define MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_DPU_CFG_SHIFT                          19
#define MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_VEU_CFG_SHIFT                          20
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DISP_BUS_CFG_SHIFT                      21
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_DSI0_SHIFT                          22
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_DSI1_SHIFT                          23
#define MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_DP0_SHIFT                           24
#define MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_DPU_PERF0_SHIFT                        25
#define MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_DPU_PERF1_SHIFT                        26
#define MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_DPU_PERF2_SHIFT                        27
#define MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_DPU_PERF3_SHIFT                        28
#define MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_VEU_PERF_SHIFT                         29
#define MEDIA1_CRG_CLKGT1_W1C                           0x0044
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_CORE0_SHIFT                         0
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_CORE1_SHIFT                         1
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_CORE2_SHIFT                         2
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_CORE3_SHIFT                         3
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_DSC0_SHIFT                          5
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_DSC1_SHIFT                          6
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_VEU_SHIFT                           7
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_AXI0_SHIFT                          8
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_AXI1_SHIFT                          9
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_AXI2_SHIFT                          10
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_AXI3_SHIFT                          11
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_VEU_AXI_SHIFT                       12
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DISP_BUS_DATA_SHIFT                     13
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_AXI0_PERF_SHIFT                     14
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_AXI1_PERF_SHIFT                     15
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_AXI2_PERF_SHIFT                     16
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_AXI3_PERF_SHIFT                     17
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_VEU_AXI_PERF_SHIFT                  18
#define MEDIA1_CRG_CLKGT1_W1C_GT_PCLK_DPU_CFG_SHIFT                          19
#define MEDIA1_CRG_CLKGT1_W1C_GT_PCLK_VEU_CFG_SHIFT                          20
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DISP_BUS_CFG_SHIFT                      21
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_DSI0_SHIFT                          22
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_DSI1_SHIFT                          23
#define MEDIA1_CRG_CLKGT1_W1C_GT_CLK_DPU_DP0_SHIFT                           24
#define MEDIA1_CRG_CLKGT1_W1C_GT_PCLK_DPU_PERF0_SHIFT                        25
#define MEDIA1_CRG_CLKGT1_W1C_GT_PCLK_DPU_PERF1_SHIFT                        26
#define MEDIA1_CRG_CLKGT1_W1C_GT_PCLK_DPU_PERF2_SHIFT                        27
#define MEDIA1_CRG_CLKGT1_W1C_GT_PCLK_DPU_PERF3_SHIFT                        28
#define MEDIA1_CRG_CLKGT1_W1C_GT_PCLK_VEU_PERF_SHIFT                         29
#define MEDIA1_CRG_CLKGT1_RO                            0x0048
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_CORE0_SHIFT                          0
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_CORE0_MASK                           0x00000001
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_CORE1_SHIFT                          1
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_CORE1_MASK                           0x00000002
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_CORE2_SHIFT                          2
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_CORE2_MASK                           0x00000004
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_CORE3_SHIFT                          3
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_CORE3_MASK                           0x00000008
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_DSC0_SHIFT                           5
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_DSC0_MASK                            0x00000020
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_DSC1_SHIFT                           6
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_DSC1_MASK                            0x00000040
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_VEU_SHIFT                            7
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_VEU_MASK                             0x00000080
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI0_SHIFT                           8
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI0_MASK                            0x00000100
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI1_SHIFT                           9
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI1_MASK                            0x00000200
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI2_SHIFT                           10
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI2_MASK                            0x00000400
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI3_SHIFT                           11
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI3_MASK                            0x00000800
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_VEU_AXI_SHIFT                        12
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_VEU_AXI_MASK                         0x00001000
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DISP_BUS_DATA_SHIFT                      13
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DISP_BUS_DATA_MASK                       0x00002000
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI0_PERF_SHIFT                      14
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI0_PERF_MASK                       0x00004000
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI1_PERF_SHIFT                      15
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI1_PERF_MASK                       0x00008000
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI2_PERF_SHIFT                      16
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI2_PERF_MASK                       0x00010000
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI3_PERF_SHIFT                      17
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_AXI3_PERF_MASK                       0x00020000
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_VEU_AXI_PERF_SHIFT                   18
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_VEU_AXI_PERF_MASK                    0x00040000
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_DPU_CFG_SHIFT                           19
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_DPU_CFG_MASK                            0x00080000
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_VEU_CFG_SHIFT                           20
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_VEU_CFG_MASK                            0x00100000
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DISP_BUS_CFG_SHIFT                       21
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DISP_BUS_CFG_MASK                        0x00200000
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_DSI0_SHIFT                           22
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_DSI0_MASK                            0x00400000
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_DSI1_SHIFT                           23
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_DSI1_MASK                            0x00800000
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_DP0_SHIFT                            24
#define MEDIA1_CRG_CLKGT1_RO_GT_CLK_DPU_DP0_MASK                             0x01000000
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_DPU_PERF0_SHIFT                         25
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_DPU_PERF0_MASK                          0x02000000
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_DPU_PERF1_SHIFT                         26
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_DPU_PERF1_MASK                          0x04000000
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_DPU_PERF2_SHIFT                         27
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_DPU_PERF2_MASK                          0x08000000
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_DPU_PERF3_SHIFT                         28
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_DPU_PERF3_MASK                          0x10000000
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_VEU_PERF_SHIFT                          29
#define MEDIA1_CRG_CLKGT1_RO_GT_PCLK_VEU_PERF_MASK                           0x20000000
#define MEDIA1_CRG_CLKST3                               0x0050
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_CORE0_SHIFT                             0
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_CORE0_MASK                              0x00000001
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_CORE1_SHIFT                             1
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_CORE1_MASK                              0x00000002
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_CORE2_SHIFT                             2
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_CORE2_MASK                              0x00000004
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_CORE3_SHIFT                             3
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_CORE3_MASK                              0x00000008
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_DSC0_SHIFT                              5
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_DSC0_MASK                               0x00000020
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_DSC1_SHIFT                              6
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_DSC1_MASK                               0x00000040
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_VEU_SHIFT                               7
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_VEU_MASK                                0x00000080
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI0_SHIFT                              8
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI0_MASK                               0x00000100
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI1_SHIFT                              9
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI1_MASK                               0x00000200
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI2_SHIFT                              10
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI2_MASK                               0x00000400
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI3_SHIFT                              11
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI3_MASK                               0x00000800
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_VEU_AXI_SHIFT                           12
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_VEU_AXI_MASK                            0x00001000
#define MEDIA1_CRG_CLKST3_ST_CLK_DISP_BUS_DATA_SHIFT                         13
#define MEDIA1_CRG_CLKST3_ST_CLK_DISP_BUS_DATA_MASK                          0x00002000
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI0_PERF_SHIFT                         14
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI0_PERF_MASK                          0x00004000
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI1_PERF_SHIFT                         15
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI1_PERF_MASK                          0x00008000
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI2_PERF_SHIFT                         16
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI2_PERF_MASK                          0x00010000
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI3_PERF_SHIFT                         17
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_AXI3_PERF_MASK                          0x00020000
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_VEU_AXI_PERF_SHIFT                      18
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_VEU_AXI_PERF_MASK                       0x00040000
#define MEDIA1_CRG_CLKST3_ST_PCLK_DPU_CFG_SHIFT                              19
#define MEDIA1_CRG_CLKST3_ST_PCLK_DPU_CFG_MASK                               0x00080000
#define MEDIA1_CRG_CLKST3_ST_PCLK_VEU_CFG_SHIFT                              20
#define MEDIA1_CRG_CLKST3_ST_PCLK_VEU_CFG_MASK                               0x00100000
#define MEDIA1_CRG_CLKST3_ST_CLK_DISP_BUS_CFG_SHIFT                          21
#define MEDIA1_CRG_CLKST3_ST_CLK_DISP_BUS_CFG_MASK                           0x00200000
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_DSI0_SHIFT                              22
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_DSI0_MASK                               0x00400000
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_DSI1_SHIFT                              23
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_DSI1_MASK                               0x00800000
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_DP0_SHIFT                               24
#define MEDIA1_CRG_CLKST3_ST_CLK_DPU_DP0_MASK                                0x01000000
#define MEDIA1_CRG_CLKST3_ST_PCLK_DPU_PERF0_SHIFT                            25
#define MEDIA1_CRG_CLKST3_ST_PCLK_DPU_PERF0_MASK                             0x02000000
#define MEDIA1_CRG_CLKST3_ST_PCLK_DPU_PERF1_SHIFT                            26
#define MEDIA1_CRG_CLKST3_ST_PCLK_DPU_PERF1_MASK                             0x04000000
#define MEDIA1_CRG_CLKST3_ST_PCLK_DPU_PERF2_SHIFT                            27
#define MEDIA1_CRG_CLKST3_ST_PCLK_DPU_PERF2_MASK                             0x08000000
#define MEDIA1_CRG_CLKST3_ST_PCLK_DPU_PERF3_SHIFT                            28
#define MEDIA1_CRG_CLKST3_ST_PCLK_DPU_PERF3_MASK                             0x10000000
#define MEDIA1_CRG_CLKST3_ST_PCLK_VEU_PERF_SHIFT                             29
#define MEDIA1_CRG_CLKST3_ST_PCLK_VEU_PERF_MASK                              0x20000000
#define MEDIA1_CRG_CLKGT2_W1S                           0x0060
#define MEDIA1_CRG_CLKGT2_W1S_GT_CLK_VDEC_CORE_SHIFT                         0
#define MEDIA1_CRG_CLKGT2_W1S_GT_ACLK_VDEC_AXI_SHIFT                         1
#define MEDIA1_CRG_CLKGT2_W1S_GT_ACLK_VDEC_CORE_AXI_SHIFT                    2
#define MEDIA1_CRG_CLKGT2_W1S_GT_ACLK_VDEC_PERF_SHIFT                        3
#define MEDIA1_CRG_CLKGT2_W1S_GT_PCLK_VDEC_CFG_SHIFT                         4
#define MEDIA1_CRG_CLKGT2_W1S_GT_PCLK_VDEC_CORE_CFG_SHIFT                    5
#define MEDIA1_CRG_CLKGT2_W1S_GT_PCLK_VDEC_PERF_SHIFT                        6
#define MEDIA1_CRG_CLKGT2_W1C                           0x0064
#define MEDIA1_CRG_CLKGT2_W1C_GT_CLK_VDEC_CORE_SHIFT                         0
#define MEDIA1_CRG_CLKGT2_W1C_GT_ACLK_VDEC_AXI_SHIFT                         1
#define MEDIA1_CRG_CLKGT2_W1C_GT_ACLK_VDEC_CORE_AXI_SHIFT                    2
#define MEDIA1_CRG_CLKGT2_W1C_GT_ACLK_VDEC_PERF_SHIFT                        3
#define MEDIA1_CRG_CLKGT2_W1C_GT_PCLK_VDEC_CFG_SHIFT                         4
#define MEDIA1_CRG_CLKGT2_W1C_GT_PCLK_VDEC_CORE_CFG_SHIFT                    5
#define MEDIA1_CRG_CLKGT2_W1C_GT_PCLK_VDEC_PERF_SHIFT                        6
#define MEDIA1_CRG_CLKGT2_RO                            0x0068
#define MEDIA1_CRG_CLKGT2_RO_GT_CLK_VDEC_CORE_SHIFT                          0
#define MEDIA1_CRG_CLKGT2_RO_GT_CLK_VDEC_CORE_MASK                           0x00000001
#define MEDIA1_CRG_CLKGT2_RO_GT_ACLK_VDEC_AXI_SHIFT                          1
#define MEDIA1_CRG_CLKGT2_RO_GT_ACLK_VDEC_AXI_MASK                           0x00000002
#define MEDIA1_CRG_CLKGT2_RO_GT_ACLK_VDEC_CORE_AXI_SHIFT                     2
#define MEDIA1_CRG_CLKGT2_RO_GT_ACLK_VDEC_CORE_AXI_MASK                      0x00000004
#define MEDIA1_CRG_CLKGT2_RO_GT_ACLK_VDEC_PERF_SHIFT                         3
#define MEDIA1_CRG_CLKGT2_RO_GT_ACLK_VDEC_PERF_MASK                          0x00000008
#define MEDIA1_CRG_CLKGT2_RO_GT_PCLK_VDEC_CFG_SHIFT                          4
#define MEDIA1_CRG_CLKGT2_RO_GT_PCLK_VDEC_CFG_MASK                           0x00000010
#define MEDIA1_CRG_CLKGT2_RO_GT_PCLK_VDEC_CORE_CFG_SHIFT                     5
#define MEDIA1_CRG_CLKGT2_RO_GT_PCLK_VDEC_CORE_CFG_MASK                      0x00000020
#define MEDIA1_CRG_CLKGT2_RO_GT_PCLK_VDEC_PERF_SHIFT                         6
#define MEDIA1_CRG_CLKGT2_RO_GT_PCLK_VDEC_PERF_MASK                          0x00000040
#define MEDIA1_CRG_CLKST4                               0x0070
#define MEDIA1_CRG_CLKST4_ST_CLK_VDEC_CORE_SHIFT                             0
#define MEDIA1_CRG_CLKST4_ST_CLK_VDEC_CORE_MASK                              0x00000001
#define MEDIA1_CRG_CLKST4_ST_ACLK_VDEC_AXI_SHIFT                             1
#define MEDIA1_CRG_CLKST4_ST_ACLK_VDEC_AXI_MASK                              0x00000002
#define MEDIA1_CRG_CLKST4_ST_ACLK_VDEC_CORE_AXI_SHIFT                        2
#define MEDIA1_CRG_CLKST4_ST_ACLK_VDEC_CORE_AXI_MASK                         0x00000004
#define MEDIA1_CRG_CLKST4_ST_ACLK_VDEC_PERF_SHIFT                            3
#define MEDIA1_CRG_CLKST4_ST_ACLK_VDEC_PERF_MASK                             0x00000008
#define MEDIA1_CRG_CLKST4_ST_PCLK_VDEC_CFG_SHIFT                             4
#define MEDIA1_CRG_CLKST4_ST_PCLK_VDEC_CFG_MASK                              0x00000010
#define MEDIA1_CRG_CLKST4_ST_PCLK_VDEC_CORE_CFG_SHIFT                        5
#define MEDIA1_CRG_CLKST4_ST_PCLK_VDEC_CORE_CFG_MASK                         0x00000020
#define MEDIA1_CRG_CLKST4_ST_PCLK_VDEC_PERF_SHIFT                            6
#define MEDIA1_CRG_CLKST4_ST_PCLK_VDEC_PERF_MASK                             0x00000040
#define MEDIA1_CRG_CLKST5                               0x0074
#define MEDIA1_CRG_CLKST5_DPU_DVFS_ENABLE_SHIFT                              0
#define MEDIA1_CRG_CLKST5_DPU_DVFS_ENABLE_MASK                               0x00000001
#define MEDIA1_CRG_CLKST5_DIV_ACLK_MEDIA1_BUS_SHIFT                          1
#define MEDIA1_CRG_CLKST5_DIV_ACLK_MEDIA1_BUS_MASK                           0x0000007e
#define MEDIA1_CRG_CLKST5_DIV_PCLK_MEDIA1_CFG_SHIFT                          7
#define MEDIA1_CRG_CLKST5_DIV_PCLK_MEDIA1_CFG_MASK                           0x00001f80
#define MEDIA1_CRG_CLKST5_DIV_CLK_DPU_CORE_SHIFT                             13
#define MEDIA1_CRG_CLKST5_DIV_CLK_DPU_CORE_MASK                              0x0007e000
#define MEDIA1_CRG_CLKST5_DIV_CLK_DPU_VEU_SHIFT                              19
#define MEDIA1_CRG_CLKST5_DIV_CLK_DPU_VEU_MASK                               0x01f80000
#define MEDIA1_CRG_RST0_W1S                             0x0080
#define MEDIA1_CRG_RST0_W1S_IP_RST_VDEC_CORE_CFG_N_SHIFT                     0
#define MEDIA1_CRG_RST0_W1S_IP_RST_VDEC_CFG_N_SHIFT                          1
#define MEDIA1_CRG_RST0_W1S_IP_RST_VDEC_CORE_N_SHIFT                         2
#define MEDIA1_CRG_RST0_W1S_IP_RST_VDEC_CORE_AXI_N_SHIFT                     3
#define MEDIA1_CRG_RST0_W1S_IP_RST_VDEC_AXI_N_SHIFT                          4
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_DSI0_N_SHIFT                          6
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_DSI1_N_SHIFT                          7
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_DP_N_SHIFT                            8
#define MEDIA1_CRG_RST0_W1S_IP_RST_DISP_BUS_CFG_N_SHIFT                      9
#define MEDIA1_CRG_RST0_W1S_IP_RST_DISP_BUS_DATA_N_SHIFT                     10
#define MEDIA1_CRG_RST0_W1S_IP_RST_VEU_BRG_N_SHIFT                           11
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_BRG_N_SHIFT                           12
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_VEU_N_SHIFT                           13
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_DSC1_N_SHIFT                          14
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_DSC0_N_SHIFT                          15
#define MEDIA1_CRG_RST0_W1S_IP_RST_DSI_BRG_N_SHIFT                           16
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_CORE3_N_SHIFT                         17
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_CORE2_N_SHIFT                         18
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_CORE1_N_SHIFT                         19
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_CORE0_N_SHIFT                         20
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_DSI_CFG_N_SHIFT                       21
#define MEDIA1_CRG_RST0_W1S_IP_RST_DPU_N_SHIFT                               22
#define MEDIA1_CRG_RST0_W1S_IP_RST_MEDIA1_HPM_N_SHIFT                        24
#define MEDIA1_CRG_RST0_W1S_IP_RST_MEDIA1_GPC3_N_SHIFT                       25
#define MEDIA1_CRG_RST0_W1S_IP_RST_MEDIA1_GPC2_N_SHIFT                       26
#define MEDIA1_CRG_RST0_W1S_IP_RST_MEDIA1_GPC1_N_SHIFT                       27
#define MEDIA1_CRG_RST0_W1S_IP_RST_MEDIA1_GPC0_N_SHIFT                       28
#define MEDIA1_CRG_RST0_W1S_IP_RST_MEDIA1_TCU_N_SHIFT                        30
#define MEDIA1_CRG_RST0_W1S_IP_RST_MEDIA1_NOC_N_SHIFT                        31
#define MEDIA1_CRG_RST0_W1C                             0x0084
#define MEDIA1_CRG_RST0_W1C_IP_RST_VDEC_CORE_CFG_N_SHIFT                     0
#define MEDIA1_CRG_RST0_W1C_IP_RST_VDEC_CFG_N_SHIFT                          1
#define MEDIA1_CRG_RST0_W1C_IP_RST_VDEC_CORE_N_SHIFT                         2
#define MEDIA1_CRG_RST0_W1C_IP_RST_VDEC_CORE_AXI_N_SHIFT                     3
#define MEDIA1_CRG_RST0_W1C_IP_RST_VDEC_AXI_N_SHIFT                          4
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_DSI0_N_SHIFT                          6
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_DSI1_N_SHIFT                          7
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_DP_N_SHIFT                            8
#define MEDIA1_CRG_RST0_W1C_IP_RST_DISP_BUS_CFG_N_SHIFT                      9
#define MEDIA1_CRG_RST0_W1C_IP_RST_DISP_BUS_DATA_N_SHIFT                     10
#define MEDIA1_CRG_RST0_W1C_IP_RST_VEU_BRG_N_SHIFT                           11
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_BRG_N_SHIFT                           12
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_VEU_N_SHIFT                           13
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_DSC1_N_SHIFT                          14
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_DSC0_N_SHIFT                          15
#define MEDIA1_CRG_RST0_W1C_IP_RST_DSI_BRG_N_SHIFT                           16
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_CORE3_N_SHIFT                         17
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_CORE2_N_SHIFT                         18
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_CORE1_N_SHIFT                         19
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_CORE0_N_SHIFT                         20
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_DSI_CFG_N_SHIFT                       21
#define MEDIA1_CRG_RST0_W1C_IP_RST_DPU_N_SHIFT                               22
#define MEDIA1_CRG_RST0_W1C_IP_RST_MEDIA1_HPM_N_SHIFT                        24
#define MEDIA1_CRG_RST0_W1C_IP_RST_MEDIA1_GPC3_N_SHIFT                       25
#define MEDIA1_CRG_RST0_W1C_IP_RST_MEDIA1_GPC2_N_SHIFT                       26
#define MEDIA1_CRG_RST0_W1C_IP_RST_MEDIA1_GPC1_N_SHIFT                       27
#define MEDIA1_CRG_RST0_W1C_IP_RST_MEDIA1_GPC0_N_SHIFT                       28
#define MEDIA1_CRG_RST0_W1C_IP_RST_MEDIA1_TCU_N_SHIFT                        30
#define MEDIA1_CRG_RST0_W1C_IP_RST_MEDIA1_NOC_N_SHIFT                        31
#define MEDIA1_CRG_RST0_RO                              0x0088
#define MEDIA1_CRG_RST0_RO_IP_RST_VDEC_CORE_CFG_N_SHIFT                      0
#define MEDIA1_CRG_RST0_RO_IP_RST_VDEC_CORE_CFG_N_MASK                       0x00000001
#define MEDIA1_CRG_RST0_RO_IP_RST_VDEC_CFG_N_SHIFT                           1
#define MEDIA1_CRG_RST0_RO_IP_RST_VDEC_CFG_N_MASK                            0x00000002
#define MEDIA1_CRG_RST0_RO_IP_RST_VDEC_CORE_N_SHIFT                          2
#define MEDIA1_CRG_RST0_RO_IP_RST_VDEC_CORE_N_MASK                           0x00000004
#define MEDIA1_CRG_RST0_RO_IP_RST_VDEC_CORE_AXI_N_SHIFT                      3
#define MEDIA1_CRG_RST0_RO_IP_RST_VDEC_CORE_AXI_N_MASK                       0x00000008
#define MEDIA1_CRG_RST0_RO_IP_RST_VDEC_AXI_N_SHIFT                           4
#define MEDIA1_CRG_RST0_RO_IP_RST_VDEC_AXI_N_MASK                            0x00000010
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DSI0_N_SHIFT                           6
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DSI0_N_MASK                            0x00000040
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DSI1_N_SHIFT                           7
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DSI1_N_MASK                            0x00000080
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DP_N_SHIFT                             8
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DP_N_MASK                              0x00000100
#define MEDIA1_CRG_RST0_RO_IP_RST_DISP_BUS_CFG_N_SHIFT                       9
#define MEDIA1_CRG_RST0_RO_IP_RST_DISP_BUS_CFG_N_MASK                        0x00000200
#define MEDIA1_CRG_RST0_RO_IP_RST_DISP_BUS_DATA_N_SHIFT                      10
#define MEDIA1_CRG_RST0_RO_IP_RST_DISP_BUS_DATA_N_MASK                       0x00000400
#define MEDIA1_CRG_RST0_RO_IP_RST_VEU_BRG_N_SHIFT                            11
#define MEDIA1_CRG_RST0_RO_IP_RST_VEU_BRG_N_MASK                             0x00000800
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_BRG_N_SHIFT                            12
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_BRG_N_MASK                             0x00001000
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_VEU_N_SHIFT                            13
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_VEU_N_MASK                             0x00002000
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DSC1_N_SHIFT                           14
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DSC1_N_MASK                            0x00004000
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DSC0_N_SHIFT                           15
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DSC0_N_MASK                            0x00008000
#define MEDIA1_CRG_RST0_RO_IP_RST_DSI_BRG_N_SHIFT                            16
#define MEDIA1_CRG_RST0_RO_IP_RST_DSI_BRG_N_MASK                             0x00010000
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_CORE3_N_SHIFT                          17
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_CORE3_N_MASK                           0x00020000
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_CORE2_N_SHIFT                          18
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_CORE2_N_MASK                           0x00040000
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_CORE1_N_SHIFT                          19
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_CORE1_N_MASK                           0x00080000
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_CORE0_N_SHIFT                          20
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_CORE0_N_MASK                           0x00100000
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DSI_CFG_N_SHIFT                        21
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_DSI_CFG_N_MASK                         0x00200000
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_N_SHIFT                                22
#define MEDIA1_CRG_RST0_RO_IP_RST_DPU_N_MASK                                 0x00400000
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_HPM_N_SHIFT                         24
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_HPM_N_MASK                          0x01000000
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_GPC3_N_SHIFT                        25
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_GPC3_N_MASK                         0x02000000
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_GPC2_N_SHIFT                        26
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_GPC2_N_MASK                         0x04000000
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_GPC1_N_SHIFT                        27
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_GPC1_N_MASK                         0x08000000
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_GPC0_N_SHIFT                        28
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_GPC0_N_MASK                         0x10000000
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_TCU_N_SHIFT                         30
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_TCU_N_MASK                          0x40000000
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_NOC_N_SHIFT                         31
#define MEDIA1_CRG_RST0_RO_IP_RST_MEDIA1_NOC_N_MASK                          0x80000000
#define MEDIA1_CRG_RST1_W1S                             0x0090
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_CORE3_CFG_N_SHIFT                     1
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_CORE2_CFG_N_SHIFT                     2
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_CORE1_CFG_N_SHIFT                     3
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_CORE0_CFG_N_SHIFT                     4
#define MEDIA1_CRG_RST1_W1S_IP_ARST_VDEC_PERF_N_SHIFT                        5
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_VEU_AXI_PERF_N_SHIFT                  6
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_AXI3_PERF_N_SHIFT                     7
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_AXI2_PERF_N_SHIFT                     8
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_AXI1_PERF_N_SHIFT                     9
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_AXI0_PERF_N_SHIFT                     10
#define MEDIA1_CRG_RST1_W1S_IP_RST_MEDIA1_PERF_N_SHIFT                       11
#define MEDIA1_CRG_RST1_W1S_IP_RST_MEDIA1_BIST_N_SHIFT                       13
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_VEU_CFG_N_SHIFT                       15
#define MEDIA1_CRG_RST1_W1S_IP_RST_DPU_CFG_N_SHIFT                           16
#define MEDIA1_CRG_RST1_W1C                             0x0094
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_CORE3_CFG_N_SHIFT                     1
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_CORE2_CFG_N_SHIFT                     2
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_CORE1_CFG_N_SHIFT                     3
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_CORE0_CFG_N_SHIFT                     4
#define MEDIA1_CRG_RST1_W1C_IP_ARST_VDEC_PERF_N_SHIFT                        5
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_VEU_AXI_PERF_N_SHIFT                  6
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_AXI3_PERF_N_SHIFT                     7
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_AXI2_PERF_N_SHIFT                     8
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_AXI1_PERF_N_SHIFT                     9
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_AXI0_PERF_N_SHIFT                     10
#define MEDIA1_CRG_RST1_W1C_IP_RST_MEDIA1_PERF_N_SHIFT                       11
#define MEDIA1_CRG_RST1_W1C_IP_RST_MEDIA1_BIST_N_SHIFT                       13
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_VEU_CFG_N_SHIFT                       15
#define MEDIA1_CRG_RST1_W1C_IP_RST_DPU_CFG_N_SHIFT                           16
#define MEDIA1_CRG_RST1_RO                              0x0098
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_CORE3_CFG_N_SHIFT                      1
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_CORE3_CFG_N_MASK                       0x00000002
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_CORE2_CFG_N_SHIFT                      2
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_CORE2_CFG_N_MASK                       0x00000004
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_CORE1_CFG_N_SHIFT                      3
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_CORE1_CFG_N_MASK                       0x00000008
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_CORE0_CFG_N_SHIFT                      4
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_CORE0_CFG_N_MASK                       0x00000010
#define MEDIA1_CRG_RST1_RO_IP_ARST_VDEC_PERF_N_SHIFT                         5
#define MEDIA1_CRG_RST1_RO_IP_ARST_VDEC_PERF_N_MASK                          0x00000020
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_VEU_AXI_PERF_N_SHIFT                   6
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_VEU_AXI_PERF_N_MASK                    0x00000040
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_AXI3_PERF_N_SHIFT                      7
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_AXI3_PERF_N_MASK                       0x00000080
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_AXI2_PERF_N_SHIFT                      8
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_AXI2_PERF_N_MASK                       0x00000100
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_AXI1_PERF_N_SHIFT                      9
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_AXI1_PERF_N_MASK                       0x00000200
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_AXI0_PERF_N_SHIFT                      10
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_AXI0_PERF_N_MASK                       0x00000400
#define MEDIA1_CRG_RST1_RO_IP_RST_MEDIA1_PERF_N_SHIFT                        11
#define MEDIA1_CRG_RST1_RO_IP_RST_MEDIA1_PERF_N_MASK                         0x00000800
#define MEDIA1_CRG_RST1_RO_IP_RST_MEDIA1_BIST_N_SHIFT                        13
#define MEDIA1_CRG_RST1_RO_IP_RST_MEDIA1_BIST_N_MASK                         0x00002000
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_VEU_CFG_N_SHIFT                        15
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_VEU_CFG_N_MASK                         0x00008000
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_CFG_N_SHIFT                            16
#define MEDIA1_CRG_RST1_RO_IP_RST_DPU_CFG_N_MASK                             0x00010000
#define MEDIA1_CRG_M1_CLKRST_BYPASS                     0x0100
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_TCU_BYPASS_SHIFT             0
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_TCU_BYPASS_MASK              0x00000001
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_DEBUG_VDEC_BYPASS_SHIFT      1
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_DEBUG_VDEC_BYPASS_MASK       0x00000002
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_BIST_BYPASS_SHIFT            2
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_BIST_BYPASS_MASK             0x00000004
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_TPC_BYPASS_SHIFT             3
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_TPC_BYPASS_MASK              0x00000008
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_SYS_CTRL_BYPASS_SHIFT        4
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_SYS_CTRL_BYPASS_MASK         0x00000010
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_SMMU_BYPASS_SHIFT            5
#define MEDIA1_CRG_M1_CLKRST_BYPASS_CLKRST_FLAG_SMMU_BYPASS_MASK             0x00000020
#define MEDIA1_CRG_M1_DPU_BYPASS                        0x0110
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_CORE0_BYPASS_SHIFT               0
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_CORE0_BYPASS_MASK                0x00000001
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_CORE1_BYPASS_SHIFT               1
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_CORE1_BYPASS_MASK                0x00000002
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_CORE2_BYPASS_SHIFT               2
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_CORE2_BYPASS_MASK                0x00000004
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_CORE3_BYPASS_SHIFT               3
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_CORE3_BYPASS_MASK                0x00000008
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_AXI0_BYPASS_SHIFT                4
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_AXI0_BYPASS_MASK                 0x00000010
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_AXI1_BYPASS_SHIFT                5
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_AXI1_BYPASS_MASK                 0x00000020
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_AXI2_BYPASS_SHIFT                6
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_AXI2_BYPASS_MASK                 0x00000040
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_AXI3_BYPASS_SHIFT                7
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_AXI3_BYPASS_MASK                 0x00000080
#define MEDIA1_CRG_M1_DPU_BYPASS_GT_CLK_DPU_SW_EN_SHIFT                      8
#define MEDIA1_CRG_M1_DPU_BYPASS_GT_CLK_DPU_SW_EN_MASK                       0x00000f00
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_BYPASS_SHIFT                     12
#define MEDIA1_CRG_M1_DPU_BYPASS_AUTOGT_DPU_BYPASS_MASK                      0x0000f000
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS                   0x0114
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_BLOCK_SHIFT                   0
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_BLOCK_MASK                    0x00000001
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_ACLK_MEDIA1_BYPASS_SHIFT      1
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_ACLK_MEDIA1_BYPASS_MASK       0x00000002
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_PCLK_MEDIA1_BYPASS_SHIFT      2
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_PCLK_MEDIA1_BYPASS_MASK       0x00000004
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_CLK_DPU_CORE_BYPASS_SHIFT     3
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_CLK_DPU_CORE_BYPASS_MASK      0x00000008
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_CLK_DPU_VEU_BYPASS_SHIFT      4
#define MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_CLK_DPU_VEU_BYPASS_MASK       0x00000010
#define MEDIA1_CRG_GPC_PU_REQ_POLL_DPU_PART1            0x0200
#define MEDIA1_CRG_GPC_PU_REQ_POLL_DPU_PART1_PU_REQ_POLL_DPU_PART1_SHIFT     0
#define MEDIA1_CRG_GPC_PU_REQ_POLL_DPU_PART1_PU_REQ_POLL_DPU_PART1_MASK      0x000000ff
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_DPU_PART1           0x0204
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_DPU_PART1_PU_REQ_POLL_MSK_DPU_PART1_SHIFT 0
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_DPU_PART1_PU_REQ_POLL_MSK_DPU_PART1_MASK  0x000000ff
#define MEDIA1_CRG_INTR_CLR_PU_DPU_PART1                0x0208
#define MEDIA1_CRG_INTR_CLR_PU_DPU_PART1_INTR_CLR_PU_GPC_DPU_PART1_SHIFT     0
#define MEDIA1_CRG_INTR_CLR_PU_DPU_PART1_INTR_CLR_PU_GPC_DPU_PART1_MASK      0x00000001
#define MEDIA1_CRG_INTR_CLR_PU_DPU_PART1_INTR_CLR_PD_GPC_DPU_PART1_SHIFT     1
#define MEDIA1_CRG_INTR_CLR_PU_DPU_PART1_INTR_CLR_PD_GPC_DPU_PART1_MASK      0x00000002
#define MEDIA1_CRG_INTR_PU_DPU_PART1                    0x020c
#define MEDIA1_CRG_INTR_PU_DPU_PART1_INTR_PU_GPC_DPU_PART1_SHIFT             0
#define MEDIA1_CRG_INTR_PU_DPU_PART1_INTR_PU_GPC_DPU_PART1_MASK              0x00000001
#define MEDIA1_CRG_INTR_PU_DPU_PART1_INTR_PD_GPC_DPU_PART1_SHIFT             1
#define MEDIA1_CRG_INTR_PU_DPU_PART1_INTR_PD_GPC_DPU_PART1_MASK              0x00000002
#define MEDIA1_CRG_INTR_PU_DPU_PART1_PWR_ON_DONE_DPU_PART1_SHIFT             2
#define MEDIA1_CRG_INTR_PU_DPU_PART1_PWR_ON_DONE_DPU_PART1_MASK              0x00000004
#define MEDIA1_CRG_INTR_PU_DPU_PART1_PWR_OFF_DONE_DPU_PART1_SHIFT            3
#define MEDIA1_CRG_INTR_PU_DPU_PART1_PWR_OFF_DONE_DPU_PART1_MASK             0x00000008
#define MEDIA1_CRG_GPC_STAT_DPU_PART1                   0x0214
#define MEDIA1_CRG_GPC_STAT_1_DPU_PART1                 0x0218
#define MEDIA1_CRG_GPC_PU_REQ_POLL_DPU_PART2            0x0220
#define MEDIA1_CRG_GPC_PU_REQ_POLL_DPU_PART2_PU_REQ_POLL_DPU_PART2_SHIFT     0
#define MEDIA1_CRG_GPC_PU_REQ_POLL_DPU_PART2_PU_REQ_POLL_DPU_PART2_MASK      0x000000ff
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_DPU_PART2           0x0224
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_DPU_PART2_PU_REQ_POLL_MSK_DPU_PART2_SHIFT 0
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_DPU_PART2_PU_REQ_POLL_MSK_DPU_PART2_MASK  0x000000ff
#define MEDIA1_CRG_INTR_CLR_PU_DPU_PART2                0x0228
#define MEDIA1_CRG_INTR_CLR_PU_DPU_PART2_INTR_CLR_PU_GPC_DPU_PART2_SHIFT     0
#define MEDIA1_CRG_INTR_CLR_PU_DPU_PART2_INTR_CLR_PU_GPC_DPU_PART2_MASK      0x00000001
#define MEDIA1_CRG_INTR_CLR_PU_DPU_PART2_INTR_CLR_PD_GPC_DPU_PART2_SHIFT     1
#define MEDIA1_CRG_INTR_CLR_PU_DPU_PART2_INTR_CLR_PD_GPC_DPU_PART2_MASK      0x00000002
#define MEDIA1_CRG_INTR_PU_DPU_PART2                    0x022c
#define MEDIA1_CRG_INTR_PU_DPU_PART2_INTR_PU_GPC_DPU_PART2_SHIFT             0
#define MEDIA1_CRG_INTR_PU_DPU_PART2_INTR_PU_GPC_DPU_PART2_MASK              0x00000001
#define MEDIA1_CRG_INTR_PU_DPU_PART2_INTR_PD_GPC_DPU_PART2_SHIFT             1
#define MEDIA1_CRG_INTR_PU_DPU_PART2_INTR_PD_GPC_DPU_PART2_MASK              0x00000002
#define MEDIA1_CRG_INTR_PU_DPU_PART2_PWR_ON_DONE_DPU_PART2_SHIFT             2
#define MEDIA1_CRG_INTR_PU_DPU_PART2_PWR_ON_DONE_DPU_PART2_MASK              0x00000004
#define MEDIA1_CRG_INTR_PU_DPU_PART2_PWR_OFF_DONE_DPU_PART2_SHIFT            3
#define MEDIA1_CRG_INTR_PU_DPU_PART2_PWR_OFF_DONE_DPU_PART2_MASK             0x00000008
#define MEDIA1_CRG_GPC_STAT_DPU_PART2                   0x0234
#define MEDIA1_CRG_GPC_STAT_1_DPU_PART2                 0x0238
#define MEDIA1_CRG_GPC_PU_REQ_POLL_VEU                  0x0240
#define MEDIA1_CRG_GPC_PU_REQ_POLL_VEU_PU_REQ_POLL_VEU_SHIFT                 0
#define MEDIA1_CRG_GPC_PU_REQ_POLL_VEU_PU_REQ_POLL_VEU_MASK                  0x000000ff
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_VEU                 0x0244
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_VEU_PU_REQ_POLL_MSK_VEU_SHIFT            0
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_VEU_PU_REQ_POLL_MSK_VEU_MASK             0x000000ff
#define MEDIA1_CRG_INTR_CLR_PU_VEU                      0x0248
#define MEDIA1_CRG_INTR_CLR_PU_VEU_INTR_CLR_PU_GPC_VEU_SHIFT                 0
#define MEDIA1_CRG_INTR_CLR_PU_VEU_INTR_CLR_PU_GPC_VEU_MASK                  0x00000001
#define MEDIA1_CRG_INTR_CLR_PU_VEU_INTR_CLR_PD_GPC_VEU_SHIFT                 1
#define MEDIA1_CRG_INTR_CLR_PU_VEU_INTR_CLR_PD_GPC_VEU_MASK                  0x00000002
#define MEDIA1_CRG_INTR_PU_VEU                          0x024c
#define MEDIA1_CRG_INTR_PU_VEU_INTR_PU_GPC_VEU_SHIFT                         0
#define MEDIA1_CRG_INTR_PU_VEU_INTR_PU_GPC_VEU_MASK                          0x00000001
#define MEDIA1_CRG_INTR_PU_VEU_INTR_PD_GPC_VEU_SHIFT                         1
#define MEDIA1_CRG_INTR_PU_VEU_INTR_PD_GPC_VEU_MASK                          0x00000002
#define MEDIA1_CRG_INTR_PU_VEU_PWR_ON_DONE_VEU_SHIFT                         2
#define MEDIA1_CRG_INTR_PU_VEU_PWR_ON_DONE_VEU_MASK                          0x00000004
#define MEDIA1_CRG_INTR_PU_VEU_PWR_OFF_DONE_VEU_SHIFT                        3
#define MEDIA1_CRG_INTR_PU_VEU_PWR_OFF_DONE_VEU_MASK                         0x00000008
#define MEDIA1_CRG_GPC_STAT_VEU                         0x0254
#define MEDIA1_CRG_GPC_STAT_1_VEU                       0x0258
#define MEDIA1_CRG_GPC_PU_REQ_POLL_VDEC                 0x0260
#define MEDIA1_CRG_GPC_PU_REQ_POLL_VDEC_PU_REQ_POLL_VDEC_SHIFT               0
#define MEDIA1_CRG_GPC_PU_REQ_POLL_VDEC_PU_REQ_POLL_VDEC_MASK                0x000000ff
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_VDEC                0x0264
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_VDEC_PU_REQ_POLL_MSK_VDEC_SHIFT          0
#define MEDIA1_CRG_GPC_REQ_POLL_MSK_VDEC_PU_REQ_POLL_MSK_VDEC_MASK           0x000000ff
#define MEDIA1_CRG_INTR_CLR_PU_VDEC                     0x0268
#define MEDIA1_CRG_INTR_CLR_PU_VDEC_INTR_CLR_PU_GPC_VDEC_SHIFT               0
#define MEDIA1_CRG_INTR_CLR_PU_VDEC_INTR_CLR_PU_GPC_VDEC_MASK                0x00000001
#define MEDIA1_CRG_INTR_CLR_PU_VDEC_INTR_CLR_PD_GPC_VDEC_SHIFT               1
#define MEDIA1_CRG_INTR_CLR_PU_VDEC_INTR_CLR_PD_GPC_VDEC_MASK                0x00000002
#define MEDIA1_CRG_INTR_PU_VDEC                         0x026c
#define MEDIA1_CRG_INTR_PU_VDEC_INTR_PU_GPC_VDEC_SHIFT                       0
#define MEDIA1_CRG_INTR_PU_VDEC_INTR_PU_GPC_VDEC_MASK                        0x00000001
#define MEDIA1_CRG_INTR_PU_VDEC_INTR_PD_GPC_VDEC_SHIFT                       1
#define MEDIA1_CRG_INTR_PU_VDEC_INTR_PD_GPC_VDEC_MASK                        0x00000002
#define MEDIA1_CRG_INTR_PU_VDEC_PWR_ON_DONE_VDEC_SHIFT                       2
#define MEDIA1_CRG_INTR_PU_VDEC_PWR_ON_DONE_VDEC_MASK                        0x00000004
#define MEDIA1_CRG_INTR_PU_VDEC_PWR_OFF_DONE_VDEC_SHIFT                      3
#define MEDIA1_CRG_INTR_PU_VDEC_PWR_OFF_DONE_VDEC_MASK                       0x00000008
#define MEDIA1_CRG_GPC_STAT_VDEC                        0x0274
#define MEDIA1_CRG_GPC_STAT_1_VDEC                      0x0278
#define MEDIA1_CRG_AUTOFSCTRL0                          0x0280
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_DISP_BUS_INI_BYPASS_SHIFT         0
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_DISP_BUS_INI_BYPASS_MASK          0x0000003f
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_DISP_BUS_TGT_BYPASS_SHIFT         6
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_DISP_BUS_TGT_BYPASS_MASK          0x000000c0
#define MEDIA1_CRG_AUTOFSCTRL0_DISP_IDLE_FLAG_DATA_BYPASS_SHIFT              8
#define MEDIA1_CRG_AUTOFSCTRL0_DISP_IDLE_FLAG_DATA_BYPASS_MASK               0x00000100
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_VDEC_BUS_INI_BYPASS_SHIFT         9
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_VDEC_BUS_INI_BYPASS_MASK          0x00000200
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_VDEC_BUS_TGT_BYPASS_SHIFT         10
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_VDEC_BUS_TGT_BYPASS_MASK          0x00000400
#define MEDIA1_CRG_AUTOFSCTRL0_IDLE_VDEC_CORE_DATA_BYPASS_SHIFT              11
#define MEDIA1_CRG_AUTOFSCTRL0_IDLE_VDEC_CORE_DATA_BYPASS_MASK               0x00000800
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_MEDIA1_BUS_INI_BYPASS_SHIFT       12
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_MEDIA1_BUS_INI_BYPASS_MASK        0x0000f000
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_MEDIA1_BUS_TGT_BYPASS_SHIFT       16
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_DATA_MEDIA1_BUS_TGT_BYPASS_MASK        0x00070000
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_DISP_BUS_INI_BYPASS_SHIFT          19
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_DISP_BUS_INI_BYPASS_MASK           0x00080000
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_DISP_BUS_TGT_BYPASS_SHIFT          20
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_DISP_BUS_TGT_BYPASS_MASK           0x00300000
#define MEDIA1_CRG_AUTOFSCTRL0_DISP_IDLE_FLAG_CFG_BYPASS_SHIFT               22
#define MEDIA1_CRG_AUTOFSCTRL0_DISP_IDLE_FLAG_CFG_BYPASS_MASK                0x00400000
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_VDEC_BUS_INI_BYPASS_SHIFT          23
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_VDEC_BUS_INI_BYPASS_MASK           0x00800000
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_VDEC_BUS_TGT_BYPASS_SHIFT          24
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_VDEC_BUS_TGT_BYPASS_MASK           0x03000000
#define MEDIA1_CRG_AUTOFSCTRL0_IDLE_VDEC_CORE_CFG_BYPASS_SHIFT               26
#define MEDIA1_CRG_AUTOFSCTRL0_IDLE_VDEC_CORE_CFG_BYPASS_MASK                0x04000000
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_MEDIA1_BUS_INI_BYPASS_SHIFT        27
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_MEDIA1_BUS_INI_BYPASS_MASK         0x08000000
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_MEDIA1_BUS_TGT_BYPASS_SHIFT        28
#define MEDIA1_CRG_AUTOFSCTRL0_M1_BUS_CFG_MEDIA1_BUS_TGT_BYPASS_MASK         0xf0000000
#define MEDIA1_CRG_AUTOFSCTRL1                          0x0290
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_VEU_BYPASS_SHIFT           0
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_VEU_BYPASS_MASK            0x00000001
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_DPU3_BYPASS_SHIFT          1
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_DPU3_BYPASS_MASK           0x00000002
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_DPU2_BYPASS_SHIFT          2
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_DPU2_BYPASS_MASK           0x00000004
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_DPU1_BYPASS_SHIFT          3
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_DPU1_BYPASS_MASK           0x00000008
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_DPU0_BYPASS_SHIFT          4
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_DPU0_BYPASS_MASK           0x00000010
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_VDEC_BYPASS_SHIFT          5
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_VDEC_BYPASS_MASK           0x00000020
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_MEDIA1_BYPASS_SHIFT        6
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_MEDIA1_BYPASS_MASK         0x00000040
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_TCU_BYPASS_SHIFT                    7
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_TCU_BYPASS_MASK                     0x00000080
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_DPU4_BYPASS_SHIFT          8
#define MEDIA1_CRG_AUTOFSCTRL1_IDLE_FLAG_PERFSTAT_DPU4_BYPASS_MASK           0x00000100
#define MEDIA1_CRG_AUTOFSCTRL2                          0x0300
#define MEDIA1_CRG_AUTOFSCTRL3                          0x0310
#define MEDIA1_CRG_AUTOFSCTRL3_DEBOUNCE_IN_M1_BUS_CFG_SHIFT                  0
#define MEDIA1_CRG_AUTOFSCTRL3_DEBOUNCE_IN_M1_BUS_CFG_MASK                   0x000003ff
#define MEDIA1_CRG_AUTOFSCTRL3_DEBOUNCE_OUT_M1_BUS_CFG_SHIFT                 10
#define MEDIA1_CRG_AUTOFSCTRL3_DEBOUNCE_OUT_M1_BUS_CFG_MASK                  0x000ffc00
#define MEDIA1_CRG_AUTOFSCTRL3_DEBOUNCE_BYPASS_M1_BUS_CFG_SHIFT              20
#define MEDIA1_CRG_AUTOFSCTRL3_DEBOUNCE_BYPASS_M1_BUS_CFG_MASK               0x00100000
#define MEDIA1_CRG_AUTOFSCTRL3_DIV_AUTO_CLK_M1_BUS_CFG_SHIFT                 21
#define MEDIA1_CRG_AUTOFSCTRL3_DIV_AUTO_CLK_M1_BUS_CFG_MASK                  0x07e00000
#define MEDIA1_CRG_AUTOFSCTRL3_AUTOFS_EN_M1_BUS_CFG_SHIFT                    31
#define MEDIA1_CRG_AUTOFSCTRL3_AUTOFS_EN_M1_BUS_CFG_MASK                     0x80000000
#define MEDIA1_CRG_AUTOFSCTRL4                          0x0320
#define MEDIA1_CRG_AUTOFSCTRL4_DEBOUNCE_IN_M1_BUS_DATA_SHIFT                 0
#define MEDIA1_CRG_AUTOFSCTRL4_DEBOUNCE_IN_M1_BUS_DATA_MASK                  0x000003ff
#define MEDIA1_CRG_AUTOFSCTRL4_DEBOUNCE_OUT_M1_BUS_DATA_SHIFT                10
#define MEDIA1_CRG_AUTOFSCTRL4_DEBOUNCE_OUT_M1_BUS_DATA_MASK                 0x000ffc00
#define MEDIA1_CRG_AUTOFSCTRL4_DEBOUNCE_BYPASS_M1_BUS_DATA_SHIFT             20
#define MEDIA1_CRG_AUTOFSCTRL4_DEBOUNCE_BYPASS_M1_BUS_DATA_MASK              0x00100000
#define MEDIA1_CRG_AUTOFSCTRL4_DIV_AUTO_CLK_M1_BUS_DATA_SHIFT                21
#define MEDIA1_CRG_AUTOFSCTRL4_DIV_AUTO_CLK_M1_BUS_DATA_MASK                 0x07e00000
#define MEDIA1_CRG_AUTOFSCTRL4_AUTOFS_EN_M1_BUS_DATA_SHIFT                   31
#define MEDIA1_CRG_AUTOFSCTRL4_AUTOFS_EN_M1_BUS_DATA_MASK                    0x80000000
#define MEDIA1_CRG_AUTOGTCTRL5                          0x0330
#define MEDIA1_CRG_AUTOGTCTRL5_DEBOUNCE_IN_DPU_CORE_SHIFT                    0
#define MEDIA1_CRG_AUTOGTCTRL5_DEBOUNCE_IN_DPU_CORE_MASK                     0x000003ff
#define MEDIA1_CRG_AUTOGTCTRL5_DEBOUNCE_OUT_DPU_CORE_SHIFT                   10
#define MEDIA1_CRG_AUTOGTCTRL5_DEBOUNCE_OUT_DPU_CORE_MASK                    0x000ffc00
#define MEDIA1_CRG_AUTOGTCTRL5_DEBOUNCE_BYPASS_DPU_CORE_SHIFT                20
#define MEDIA1_CRG_AUTOGTCTRL5_DEBOUNCE_BYPASS_DPU_CORE_MASK                 0x00100000
#define MEDIA1_CRG_AUTOGTCTRL6                          0x0340
#define MEDIA1_CRG_AUTOGTCTRL6_DEBOUNCE_IN_DPU_AXI_SHIFT                     0
#define MEDIA1_CRG_AUTOGTCTRL6_DEBOUNCE_IN_DPU_AXI_MASK                      0x000003ff
#define MEDIA1_CRG_AUTOGTCTRL6_DEBOUNCE_OUT_DPU_AXI_SHIFT                    10
#define MEDIA1_CRG_AUTOGTCTRL6_DEBOUNCE_OUT_DPU_AXI_MASK                     0x000ffc00
#define MEDIA1_CRG_AUTOGTCTRL6_DEBOUNCE_BYPASS_DPU_AXI_SHIFT                 20
#define MEDIA1_CRG_AUTOGTCTRL6_DEBOUNCE_BYPASS_DPU_AXI_MASK                  0x00100000
#define MEDIA1_CRG_IDLE_STATE                           0x0400
#define MEDIA1_CRG_IDLE_STATE_DISP_IDLE_FLAG_SHIFT                           0
#define MEDIA1_CRG_IDLE_STATE_DISP_IDLE_FLAG_MASK                            0x00000001
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_VEU_SHIFT                   1
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_VEU_MASK                    0x00000002
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_DPU4_SHIFT                  2
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_DPU4_MASK                   0x00000004
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_DPU3_SHIFT                  3
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_DPU3_MASK                   0x00000008
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_DPU2_SHIFT                  4
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_DPU2_MASK                   0x00000010
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_DPU1_SHIFT                  5
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_DPU1_MASK                   0x00000020
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_DPU0_SHIFT                  6
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_DPU0_MASK                   0x00000040
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_VDEC_SHIFT                  7
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_VDEC_MASK                   0x00000080
#define MEDIA1_CRG_IDLE_STATE_IDLE_VDEC_CORE_SHIFT                           8
#define MEDIA1_CRG_IDLE_STATE_IDLE_VDEC_CORE_MASK                            0x00000100
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_TCU_SHIFT                            9
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_TCU_MASK                             0x00000200
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_MEDIA1_SHIFT                10
#define MEDIA1_CRG_IDLE_STATE_IDLE_FLAG_PERFSTAT_MEDIA1_MASK                 0x00000400
#define MEDIA1_CRG_IDLE_STATE_M1_BUS_DATA_DVFS_STATE_SHIFT                   11
#define MEDIA1_CRG_IDLE_STATE_M1_BUS_DATA_DVFS_STATE_MASK                    0x00000800
#define MEDIA1_CRG_IDLE_STATE_M1_BUS_CFG_DVFS_STATE_SHIFT                    12
#define MEDIA1_CRG_IDLE_STATE_M1_BUS_CFG_DVFS_STATE_MASK                     0x00001000
#define MEDIA1_CRG_IDLE_STATE_DPU_CORE_DVFS_STATE_SHIFT                      13
#define MEDIA1_CRG_IDLE_STATE_DPU_CORE_DVFS_STATE_MASK                       0x00002000
#define MEDIA1_CRG_IDLE_STATE_DPU_VEU_DVFS_STATE_SHIFT                       14
#define MEDIA1_CRG_IDLE_STATE_DPU_VEU_DVFS_STATE_MASK                        0x00004000
#define MEDIA1_CRG_AUTOFS_BYPASS0                       0x0500
#define MEDIA1_CRG_AUTOFS_BYPASS0_AUTOFS_BYPASS_M1_BUS_DATA_SHIFT            0
#define MEDIA1_CRG_AUTOFS_BYPASS0_AUTOFS_BYPASS_M1_BUS_DATA_MASK             0x0000ffff
#define MEDIA1_CRG_AUTOFS_BYPASS1                       0x0520
#define MEDIA1_CRG_AUTOFS_BYPASS1_AUTOFS_BYPASS_M1_BUS_CFG_SHIFT             0
#define MEDIA1_CRG_AUTOFS_BYPASS1_AUTOFS_BYPASS_M1_BUS_CFG_MASK              0x0000ffff
#define MEDIA1_CRG_AUTOGT_BYPASS2                       0x0530
#define MEDIA1_CRG_AUTOGT_BYPASS2_CORE_DISP_BUS_INI_NOPENGDING_BYPASS_SHIFT  0
#define MEDIA1_CRG_AUTOGT_BYPASS2_CORE_DISP_BUS_INI_NOPENGDING_BYPASS_MASK   0x0000007f
#define MEDIA1_CRG_AUTOGT_BYPASS2_CORE_DISP_BUS_TGT_NOPENGDING_BYPASS_SHIFT  7
#define MEDIA1_CRG_AUTOGT_BYPASS2_CORE_DISP_BUS_TGT_NOPENGDING_BYPASS_MASK   0x00000780
#define MEDIA1_CRG_AUTOGT_BYPASS2_AXI_DISP_BUS_INI_NOPENGDING_BYPASS_SHIFT   11
#define MEDIA1_CRG_AUTOGT_BYPASS2_AXI_DISP_BUS_INI_NOPENGDING_BYPASS_MASK    0x0003f800
#define MEDIA1_CRG_AUTOGT_BYPASS2_AXI_DISP_BUS_TGT_NOPENGDING_BYPASS_SHIFT   18
#define MEDIA1_CRG_AUTOGT_BYPASS2_AXI_DISP_BUS_TGT_NOPENGDING_BYPASS_MASK    0x003c0000
#define MEDIA1_CRG_AUTOGT_BYPASS2_CORE_DPU_IDLE_BYPASS_SHIFT                 22
#define MEDIA1_CRG_AUTOGT_BYPASS2_CORE_DPU_IDLE_BYPASS_MASK                  0x01c00000
#define MEDIA1_CRG_AUTOGT_BYPASS2_AXI_DPU_IDLE_BYPASS_SHIFT                  25
#define MEDIA1_CRG_AUTOGT_BYPASS2_AXI_DPU_IDLE_BYPASS_MASK                   0x0e000000
#define MEDIA1_CRG_AUTOGT_STATE                         0x0600
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_CORE0_SHIFT                   0
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_CORE0_MASK                    0x00000001
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_CORE1_SHIFT                   1
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_CORE1_MASK                    0x00000002
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_CORE2_SHIFT                   2
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_CORE2_MASK                    0x00000004
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_CORE3_SHIFT                   3
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_CORE3_MASK                    0x00000008
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_AXI0_SHIFT                    4
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_AXI0_MASK                     0x00000010
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_AXI1_SHIFT                    5
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_AXI1_MASK                     0x00000020
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_AXI2_SHIFT                    6
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_AXI2_MASK                     0x00000040
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_AXI3_SHIFT                    7
#define MEDIA1_CRG_AUTOGT_STATE_AUTOGT_CLK_DPU_AXI3_MASK                     0x00000080
#define MEDIA1_CRG_M1_HPM0_CFG_0                        0x0800
#define MEDIA1_CRG_M1_HPM0_CFG_0_M1_HPM_CLEAR_0_SHIFT                        0
#define MEDIA1_CRG_M1_HPM0_CFG_0_M1_HPM_CLEAR_0_MASK                         0x00000001
#define MEDIA1_CRG_M1_HPM0_CFG_0_M1_HPM_RO_SEL_0_SHIFT                       1
#define MEDIA1_CRG_M1_HPM0_CFG_0_M1_HPM_RO_SEL_0_MASK                        0x0000007e
#define MEDIA1_CRG_M1_HPM0_CFG_0_M1_HPM_RO_CLK_DIV_0_SHIFT                   7
#define MEDIA1_CRG_M1_HPM0_CFG_0_M1_HPM_RO_CLK_DIV_0_MASK                    0x00000180
#define MEDIA1_CRG_M1_HPM0_CFG_0_M1_HPM_RO_DBG_DIV_0_SHIFT                   9
#define MEDIA1_CRG_M1_HPM0_CFG_0_M1_HPM_RO_DBG_DIV_0_MASK                    0x00007e00
#define MEDIA1_CRG_M1_HPM0_CFG_1                        0x0804
#define MEDIA1_CRG_M1_HPM0_CFG_1_M1_HPM_RO_EN_0_SHIFT                        0
#define MEDIA1_CRG_M1_HPM0_CFG_1_M1_HPM_RO_EN_0_MASK                         0x00000001
#define MEDIA1_CRG_M1_HPM0_CFG_1_M1_HPM_TEST_HW_EN_0_SHIFT                   1
#define MEDIA1_CRG_M1_HPM0_CFG_1_M1_HPM_TEST_HW_EN_0_MASK                    0x00000002
#define MEDIA1_CRG_M1_HPM0_CFG_1_M1_HPM_TEST_HW_START_0_SHIFT                2
#define MEDIA1_CRG_M1_HPM0_CFG_1_M1_HPM_TEST_HW_START_0_MASK                 0x00000004
#define MEDIA1_CRG_M1_HPM0_CFG_1_M1_HPM_TIMER_CNT_0_SHIFT                    3
#define MEDIA1_CRG_M1_HPM0_CFG_1_M1_HPM_TIMER_CNT_0_MASK                     0x000007f8
#define MEDIA1_CRG_M1_HPM0_CLK_GT_W1S                   0x0808
#define MEDIA1_CRG_M1_HPM0_CLK_GT_W1S_M1_HPM_CLK_GT_0_SHIFT                  0
#define MEDIA1_CRG_M1_HPM0_CLK_GT_W1C                   0x080c
#define MEDIA1_CRG_M1_HPM0_CLK_GT_W1C_M1_HPM_CLK_GT_0_SHIFT                  0
#define MEDIA1_CRG_M1_HPM0_CLK_GT_RO                    0x0810
#define MEDIA1_CRG_M1_HPM0_CLK_GT_RO_M1_HPM_CLK_GT_0_SHIFT                   0
#define MEDIA1_CRG_M1_HPM0_CLK_GT_RO_M1_HPM_CLK_GT_0_MASK                    0x00000001
#define MEDIA1_CRG_M1_HPM0_DOUT                         0x0814
#define MEDIA1_CRG_M1_HPM0_DOUT_M1_HPM_DOUT_0_SHIFT                          0
#define MEDIA1_CRG_M1_HPM0_DOUT_M1_HPM_DOUT_0_MASK                           0x0003ffff
#define MEDIA1_CRG_M1_HPM1_CFG_0                        0x0820
#define MEDIA1_CRG_M1_HPM1_CFG_0_M1_HPM_CLEAR_1_SHIFT                        0
#define MEDIA1_CRG_M1_HPM1_CFG_0_M1_HPM_CLEAR_1_MASK                         0x00000001
#define MEDIA1_CRG_M1_HPM1_CFG_0_M1_HPM_RO_SEL_1_SHIFT                       1
#define MEDIA1_CRG_M1_HPM1_CFG_0_M1_HPM_RO_SEL_1_MASK                        0x0000007e
#define MEDIA1_CRG_M1_HPM1_CFG_0_M1_HPM_RO_CLK_DIV_1_SHIFT                   7
#define MEDIA1_CRG_M1_HPM1_CFG_0_M1_HPM_RO_CLK_DIV_1_MASK                    0x00000180
#define MEDIA1_CRG_M1_HPM1_CFG_0_M1_HPM_RO_DBG_DIV_1_SHIFT                   9
#define MEDIA1_CRG_M1_HPM1_CFG_0_M1_HPM_RO_DBG_DIV_1_MASK                    0x00007e00
#define MEDIA1_CRG_M1_HPM1_CFG_1                        0x0824
#define MEDIA1_CRG_M1_HPM1_CFG_1_M1_HPM_RO_EN_1_SHIFT                        0
#define MEDIA1_CRG_M1_HPM1_CFG_1_M1_HPM_RO_EN_1_MASK                         0x00000001
#define MEDIA1_CRG_M1_HPM1_CFG_1_M1_HPM_TEST_HW_EN_1_SHIFT                   1
#define MEDIA1_CRG_M1_HPM1_CFG_1_M1_HPM_TEST_HW_EN_1_MASK                    0x00000002
#define MEDIA1_CRG_M1_HPM1_CFG_1_M1_HPM_TEST_HW_START_1_SHIFT                2
#define MEDIA1_CRG_M1_HPM1_CFG_1_M1_HPM_TEST_HW_START_1_MASK                 0x00000004
#define MEDIA1_CRG_M1_HPM1_CFG_1_M1_HPM_TIMER_CNT_1_SHIFT                    3
#define MEDIA1_CRG_M1_HPM1_CFG_1_M1_HPM_TIMER_CNT_1_MASK                     0x000007f8
#define MEDIA1_CRG_M1_HPM1_CLK_GT_W1S                   0x0828
#define MEDIA1_CRG_M1_HPM1_CLK_GT_W1S_M1_HPM_CLK_GT_1_SHIFT                  0
#define MEDIA1_CRG_M1_HPM1_CLK_GT_W1C                   0x082c
#define MEDIA1_CRG_M1_HPM1_CLK_GT_W1C_M1_HPM_CLK_GT_1_SHIFT                  0
#define MEDIA1_CRG_M1_HPM1_CLK_GT_RO                    0x0830
#define MEDIA1_CRG_M1_HPM1_CLK_GT_RO_M1_HPM_CLK_GT_1_SHIFT                   0
#define MEDIA1_CRG_M1_HPM1_CLK_GT_RO_M1_HPM_CLK_GT_1_MASK                    0x00000001
#define MEDIA1_CRG_M1_HPM1_DOUT                         0x0834
#define MEDIA1_CRG_M1_HPM1_DOUT_M1_HPM_DOUT_1_SHIFT                          0
#define MEDIA1_CRG_M1_HPM1_DOUT_M1_HPM_DOUT_1_MASK                           0x0003ffff
#define MEDIA1_CRG_GPC_EN_DPU_PART1                     0x0900
#define MEDIA1_CRG_GPC_EN_DPU_PART1_GPC_EN_DPU_PART1_SHIFT                   0
#define MEDIA1_CRG_GPC_EN_DPU_PART1_GPC_EN_DPU_PART1_MASK                    0x00000001
#define MEDIA1_CRG_GPC_CTRL_0_DPU_PART1                 0x0904
#define MEDIA1_CRG_GPC_CTRL_1_DPU_PART1                 0x0908
#define MEDIA1_CRG_GPC_STAT_DEBUG_DPU_PART1             0x0914
#define MEDIA1_CRG_GPC_STAT_DEBUG_DPU_PART1_STATE_DEBUG_DPU_PART1_SHIFT      0
#define MEDIA1_CRG_GPC_STAT_DEBUG_DPU_PART1_STATE_DEBUG_DPU_PART1_MASK       0x00000003
#define MEDIA1_CRG_GPC_EN_DPU_PART2                     0x0930
#define MEDIA1_CRG_GPC_EN_DPU_PART2_GPC_EN_DPU_PART2_SHIFT                   0
#define MEDIA1_CRG_GPC_EN_DPU_PART2_GPC_EN_DPU_PART2_MASK                    0x00000001
#define MEDIA1_CRG_GPC_CTRL_0_DPU_PART2                 0x0934
#define MEDIA1_CRG_GPC_CTRL_1_DPU_PART2                 0x0938
#define MEDIA1_CRG_GPC_STAT_DEBUG_DPU_PART2             0x0944
#define MEDIA1_CRG_GPC_STAT_DEBUG_DPU_PART2_STATE_DEBUG_DPU_PART2_SHIFT      0
#define MEDIA1_CRG_GPC_STAT_DEBUG_DPU_PART2_STATE_DEBUG_DPU_PART2_MASK       0x00000003
#define MEDIA1_CRG_GPC_EN_VEU                           0x0960
#define MEDIA1_CRG_GPC_EN_VEU_GPC_EN_VEU_SHIFT                               0
#define MEDIA1_CRG_GPC_EN_VEU_GPC_EN_VEU_MASK                                0x00000001
#define MEDIA1_CRG_GPC_CTRL_0_VEU                       0x0964
#define MEDIA1_CRG_GPC_CTRL_1_VEU                       0x0968
#define MEDIA1_CRG_GPC_STAT_DEBUG_VEU                   0x0974
#define MEDIA1_CRG_GPC_STAT_DEBUG_VEU_STATE_DEBUG_VEU_SHIFT                  0
#define MEDIA1_CRG_GPC_STAT_DEBUG_VEU_STATE_DEBUG_VEU_MASK                   0x00000003
#define MEDIA1_CRG_GPC_EN_VDEC                          0x0990
#define MEDIA1_CRG_GPC_EN_VDEC_GPC_EN_VDEC_SHIFT                             0
#define MEDIA1_CRG_GPC_EN_VDEC_GPC_EN_VDEC_MASK                              0x00000001
#define MEDIA1_CRG_GPC_CTRL_0_VDEC                      0x0994
#define MEDIA1_CRG_GPC_CTRL_1_VDEC                      0x0998
#define MEDIA1_CRG_GPC_STAT_DEBUG_VDEC                  0x09a4
#define MEDIA1_CRG_GPC_STAT_DEBUG_VDEC_STATE_DEBUG_VDEC_SHIFT                0
#define MEDIA1_CRG_GPC_STAT_DEBUG_VDEC_STATE_DEBUG_VDEC_MASK                 0x00000003
#define MEDIA1_CRG_REGFILE_CGBYPASS                     0x09a8
#define MEDIA1_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                   0
#define MEDIA1_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                    0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_clk_dpu_core    :  2;
        unsigned int sel_clk_dpu_veu     :  2;
        unsigned int sel_clk_vdec_core   :  2;
        unsigned int sel_aclk_media1_bus :  2;
        unsigned int sel_pclk_media1_cfg :  2;
        unsigned int sel_clk_dpu_dsc1    :  1;
        unsigned int sel_clk_dpu_dsc0    :  1;
        unsigned int reserved_0          :  4;
        unsigned int _bm_                : 16;
    } reg;
}media1_crg_clksw0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_ack_clk_dpu_core    :  4;
        unsigned int sw_ack_clk_dpu_veu     :  4;
        unsigned int sw_ack_clk_vdec_core   :  4;
        unsigned int sw_ack_aclk_media1_bus :  4;
        unsigned int sw_ack_pclk_media1_cfg :  4;
        unsigned int reserved_0             : 12;
    } reg;
}media1_crg_clkst0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gt_clk_dpu_core  :  1;
        unsigned int div_sw_clk_dpu_core :  6;
        unsigned int sc_gt_clk_dpu_veu   :  1;
        unsigned int div_sw_clk_dpu_veu  :  6;
        unsigned int reserved_0          :  2;
        unsigned int _bm_                : 16;
    } reg;
}media1_crg_clkdiv1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gt_clk_vdec_core    :  1;
        unsigned int div_clk_vdec_core      :  6;
        unsigned int sc_gt_aclk_media1_bus  :  1;
        unsigned int div_sw_aclk_media1_bus :  6;
        unsigned int reserved_0             :  2;
        unsigned int _bm_                   : 16;
    } reg;
}media1_crg_clkdiv2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_sw_pclk_media1_cfg :  6;
        unsigned int reserved_0             : 10;
        unsigned int _bm_                   : 16;
    } reg;
}media1_crg_clkdiv3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gt_clk_media1_hpm :  1;
        unsigned int div_clk_media1_hpm   :  6;
        unsigned int sc_gt_clk_media1_gpc :  1;
        unsigned int div_clk_media1_gpc   :  6;
        unsigned int reserved_0           :  2;
        unsigned int _bm_                 : 16;
    } reg;
}media1_crg_clkdiv4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_dpu_dsc1 :  6;
        unsigned int div_clk_dpu_dsc0 :  6;
        unsigned int reserved_0       :  4;
        unsigned int _bm_             : 16;
    } reg;
}media1_crg_clkdiv5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_done_clk_dpu_dsc0    :  1;
        unsigned int div_done_clk_dpu_dsc1    :  1;
        unsigned int div_done_clk_media1_hpm  :  1;
        unsigned int div_done_clk_media1_gpc  :  1;
        unsigned int div_done_pclk_media1_cfg :  1;
        unsigned int div_done_clk_vdec_core   :  1;
        unsigned int div_done_aclk_media1_bus :  1;
        unsigned int div_done_clk_dpu_core    :  1;
        unsigned int div_done_clk_dpu_veu     :  1;
        unsigned int reserved_0               : 23;
    } reg;
}media1_crg_clkst1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0             :  3;
        unsigned int gt_aclk_media1_bus     :  1;
        unsigned int gt_aclk_media1_tcu     :  1;
        unsigned int reserved_1             :  4;
        unsigned int gt_pclk_dsi_cfg        :  1;
        unsigned int gt_aclk_media1_bist_sw :  1;
        unsigned int gt_clk_media1_hpm      :  1;
        unsigned int gt_clk_media1_gpc0     :  1;
        unsigned int gt_clk_media1_gpc1     :  1;
        unsigned int gt_clk_media1_gpc2     :  1;
        unsigned int gt_clk_media1_gpc3     :  1;
        unsigned int gt_aclk_media1_perf    :  1;
        unsigned int gt_pclk_media1_bist    :  1;
        unsigned int gt_pclk_media1_perf    :  1;
        unsigned int gt_pclk_media1_tcu     :  1;
        unsigned int reserved_2             : 12;
    } reg;
}media1_crg_clkgt0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0             :  3;
        unsigned int gt_aclk_media1_bus     :  1;
        unsigned int gt_aclk_media1_tcu     :  1;
        unsigned int reserved_1             :  4;
        unsigned int gt_pclk_dsi_cfg        :  1;
        unsigned int gt_aclk_media1_bist_sw :  1;
        unsigned int gt_clk_media1_hpm      :  1;
        unsigned int gt_clk_media1_gpc0     :  1;
        unsigned int gt_clk_media1_gpc1     :  1;
        unsigned int gt_clk_media1_gpc2     :  1;
        unsigned int gt_clk_media1_gpc3     :  1;
        unsigned int gt_aclk_media1_perf    :  1;
        unsigned int gt_pclk_media1_bist    :  1;
        unsigned int gt_pclk_media1_perf    :  1;
        unsigned int gt_pclk_media1_tcu     :  1;
        unsigned int reserved_2             : 12;
    } reg;
}media1_crg_clkgt0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0             :  3;
        unsigned int gt_aclk_media1_bus     :  1;
        unsigned int gt_aclk_media1_tcu     :  1;
        unsigned int reserved_1             :  4;
        unsigned int gt_pclk_dsi_cfg        :  1;
        unsigned int gt_aclk_media1_bist_sw :  1;
        unsigned int gt_clk_media1_hpm      :  1;
        unsigned int gt_clk_media1_gpc0     :  1;
        unsigned int gt_clk_media1_gpc1     :  1;
        unsigned int gt_clk_media1_gpc2     :  1;
        unsigned int gt_clk_media1_gpc3     :  1;
        unsigned int gt_aclk_media1_perf    :  1;
        unsigned int gt_pclk_media1_bist    :  1;
        unsigned int gt_pclk_media1_perf    :  1;
        unsigned int gt_pclk_media1_tcu     :  1;
        unsigned int reserved_2             : 12;
    } reg;
}media1_crg_clkgt0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0             :  3;
        unsigned int st_aclk_media1_bus     :  1;
        unsigned int st_aclk_media1_tcu     :  1;
        unsigned int reserved_1             :  4;
        unsigned int st_pclk_dsi_cfg        :  1;
        unsigned int st_aclk_media1_bist_sw :  1;
        unsigned int st_clk_media1_hpm      :  1;
        unsigned int st_clk_media1_gpc0     :  1;
        unsigned int st_clk_media1_gpc1     :  1;
        unsigned int st_clk_media1_gpc2     :  1;
        unsigned int st_clk_media1_gpc3     :  1;
        unsigned int st_aclk_media1_perf    :  1;
        unsigned int st_pclk_media1_bist    :  1;
        unsigned int st_pclk_media1_perf    :  1;
        unsigned int st_pclk_media1_tcu     :  1;
        unsigned int reserved_2             : 12;
    } reg;
}media1_crg_clkst2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_dpu_core0        :  1;
        unsigned int gt_clk_dpu_core1        :  1;
        unsigned int gt_clk_dpu_core2        :  1;
        unsigned int gt_clk_dpu_core3        :  1;
        unsigned int reserved_0              :  1;
        unsigned int gt_clk_dpu_dsc0         :  1;
        unsigned int gt_clk_dpu_dsc1         :  1;
        unsigned int gt_clk_dpu_veu          :  1;
        unsigned int gt_clk_dpu_axi0         :  1;
        unsigned int gt_clk_dpu_axi1         :  1;
        unsigned int gt_clk_dpu_axi2         :  1;
        unsigned int gt_clk_dpu_axi3         :  1;
        unsigned int gt_clk_dpu_veu_axi      :  1;
        unsigned int gt_clk_disp_bus_data    :  1;
        unsigned int gt_clk_dpu_axi0_perf    :  1;
        unsigned int gt_clk_dpu_axi1_perf    :  1;
        unsigned int gt_clk_dpu_axi2_perf    :  1;
        unsigned int gt_clk_dpu_axi3_perf    :  1;
        unsigned int gt_clk_dpu_veu_axi_perf :  1;
        unsigned int gt_pclk_dpu_cfg         :  1;
        unsigned int gt_pclk_veu_cfg         :  1;
        unsigned int gt_clk_disp_bus_cfg     :  1;
        unsigned int gt_clk_dpu_dsi0         :  1;
        unsigned int gt_clk_dpu_dsi1         :  1;
        unsigned int gt_clk_dpu_dp0          :  1;
        unsigned int gt_pclk_dpu_perf0       :  1;
        unsigned int gt_pclk_dpu_perf1       :  1;
        unsigned int gt_pclk_dpu_perf2       :  1;
        unsigned int gt_pclk_dpu_perf3       :  1;
        unsigned int gt_pclk_veu_perf        :  1;
        unsigned int reserved_1              :  2;
    } reg;
}media1_crg_clkgt1_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_dpu_core0        :  1;
        unsigned int gt_clk_dpu_core1        :  1;
        unsigned int gt_clk_dpu_core2        :  1;
        unsigned int gt_clk_dpu_core3        :  1;
        unsigned int reserved_0              :  1;
        unsigned int gt_clk_dpu_dsc0         :  1;
        unsigned int gt_clk_dpu_dsc1         :  1;
        unsigned int gt_clk_dpu_veu          :  1;
        unsigned int gt_clk_dpu_axi0         :  1;
        unsigned int gt_clk_dpu_axi1         :  1;
        unsigned int gt_clk_dpu_axi2         :  1;
        unsigned int gt_clk_dpu_axi3         :  1;
        unsigned int gt_clk_dpu_veu_axi      :  1;
        unsigned int gt_clk_disp_bus_data    :  1;
        unsigned int gt_clk_dpu_axi0_perf    :  1;
        unsigned int gt_clk_dpu_axi1_perf    :  1;
        unsigned int gt_clk_dpu_axi2_perf    :  1;
        unsigned int gt_clk_dpu_axi3_perf    :  1;
        unsigned int gt_clk_dpu_veu_axi_perf :  1;
        unsigned int gt_pclk_dpu_cfg         :  1;
        unsigned int gt_pclk_veu_cfg         :  1;
        unsigned int gt_clk_disp_bus_cfg     :  1;
        unsigned int gt_clk_dpu_dsi0         :  1;
        unsigned int gt_clk_dpu_dsi1         :  1;
        unsigned int gt_clk_dpu_dp0          :  1;
        unsigned int gt_pclk_dpu_perf0       :  1;
        unsigned int gt_pclk_dpu_perf1       :  1;
        unsigned int gt_pclk_dpu_perf2       :  1;
        unsigned int gt_pclk_dpu_perf3       :  1;
        unsigned int gt_pclk_veu_perf        :  1;
        unsigned int reserved_1              :  2;
    } reg;
}media1_crg_clkgt1_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_dpu_core0        :  1;
        unsigned int gt_clk_dpu_core1        :  1;
        unsigned int gt_clk_dpu_core2        :  1;
        unsigned int gt_clk_dpu_core3        :  1;
        unsigned int reserved_0              :  1;
        unsigned int gt_clk_dpu_dsc0         :  1;
        unsigned int gt_clk_dpu_dsc1         :  1;
        unsigned int gt_clk_dpu_veu          :  1;
        unsigned int gt_clk_dpu_axi0         :  1;
        unsigned int gt_clk_dpu_axi1         :  1;
        unsigned int gt_clk_dpu_axi2         :  1;
        unsigned int gt_clk_dpu_axi3         :  1;
        unsigned int gt_clk_dpu_veu_axi      :  1;
        unsigned int gt_clk_disp_bus_data    :  1;
        unsigned int gt_clk_dpu_axi0_perf    :  1;
        unsigned int gt_clk_dpu_axi1_perf    :  1;
        unsigned int gt_clk_dpu_axi2_perf    :  1;
        unsigned int gt_clk_dpu_axi3_perf    :  1;
        unsigned int gt_clk_dpu_veu_axi_perf :  1;
        unsigned int gt_pclk_dpu_cfg         :  1;
        unsigned int gt_pclk_veu_cfg         :  1;
        unsigned int gt_clk_disp_bus_cfg     :  1;
        unsigned int gt_clk_dpu_dsi0         :  1;
        unsigned int gt_clk_dpu_dsi1         :  1;
        unsigned int gt_clk_dpu_dp0          :  1;
        unsigned int gt_pclk_dpu_perf0       :  1;
        unsigned int gt_pclk_dpu_perf1       :  1;
        unsigned int gt_pclk_dpu_perf2       :  1;
        unsigned int gt_pclk_dpu_perf3       :  1;
        unsigned int gt_pclk_veu_perf        :  1;
        unsigned int reserved_1              :  2;
    } reg;
}media1_crg_clkgt1_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_dpu_core0        :  1;
        unsigned int st_clk_dpu_core1        :  1;
        unsigned int st_clk_dpu_core2        :  1;
        unsigned int st_clk_dpu_core3        :  1;
        unsigned int reserved_0              :  1;
        unsigned int st_clk_dpu_dsc0         :  1;
        unsigned int st_clk_dpu_dsc1         :  1;
        unsigned int st_clk_dpu_veu          :  1;
        unsigned int st_clk_dpu_axi0         :  1;
        unsigned int st_clk_dpu_axi1         :  1;
        unsigned int st_clk_dpu_axi2         :  1;
        unsigned int st_clk_dpu_axi3         :  1;
        unsigned int st_clk_dpu_veu_axi      :  1;
        unsigned int st_clk_disp_bus_data    :  1;
        unsigned int st_clk_dpu_axi0_perf    :  1;
        unsigned int st_clk_dpu_axi1_perf    :  1;
        unsigned int st_clk_dpu_axi2_perf    :  1;
        unsigned int st_clk_dpu_axi3_perf    :  1;
        unsigned int st_clk_dpu_veu_axi_perf :  1;
        unsigned int st_pclk_dpu_cfg         :  1;
        unsigned int st_pclk_veu_cfg         :  1;
        unsigned int st_clk_disp_bus_cfg     :  1;
        unsigned int st_clk_dpu_dsi0         :  1;
        unsigned int st_clk_dpu_dsi1         :  1;
        unsigned int st_clk_dpu_dp0          :  1;
        unsigned int st_pclk_dpu_perf0       :  1;
        unsigned int st_pclk_dpu_perf1       :  1;
        unsigned int st_pclk_dpu_perf2       :  1;
        unsigned int st_pclk_dpu_perf3       :  1;
        unsigned int st_pclk_veu_perf        :  1;
        unsigned int reserved_1              :  2;
    } reg;
}media1_crg_clkst3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_vdec_core      :  1;
        unsigned int gt_aclk_vdec_axi      :  1;
        unsigned int gt_aclk_vdec_core_axi :  1;
        unsigned int gt_aclk_vdec_perf     :  1;
        unsigned int gt_pclk_vdec_cfg      :  1;
        unsigned int gt_pclk_vdec_core_cfg :  1;
        unsigned int gt_pclk_vdec_perf     :  1;
        unsigned int reserved_0            : 25;
    } reg;
}media1_crg_clkgt2_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_vdec_core      :  1;
        unsigned int gt_aclk_vdec_axi      :  1;
        unsigned int gt_aclk_vdec_core_axi :  1;
        unsigned int gt_aclk_vdec_perf     :  1;
        unsigned int gt_pclk_vdec_cfg      :  1;
        unsigned int gt_pclk_vdec_core_cfg :  1;
        unsigned int gt_pclk_vdec_perf     :  1;
        unsigned int reserved_0            : 25;
    } reg;
}media1_crg_clkgt2_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_vdec_core      :  1;
        unsigned int gt_aclk_vdec_axi      :  1;
        unsigned int gt_aclk_vdec_core_axi :  1;
        unsigned int gt_aclk_vdec_perf     :  1;
        unsigned int gt_pclk_vdec_cfg      :  1;
        unsigned int gt_pclk_vdec_core_cfg :  1;
        unsigned int gt_pclk_vdec_perf     :  1;
        unsigned int reserved_0            : 25;
    } reg;
}media1_crg_clkgt2_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_vdec_core      :  1;
        unsigned int st_aclk_vdec_axi      :  1;
        unsigned int st_aclk_vdec_core_axi :  1;
        unsigned int st_aclk_vdec_perf     :  1;
        unsigned int st_pclk_vdec_cfg      :  1;
        unsigned int st_pclk_vdec_core_cfg :  1;
        unsigned int st_pclk_vdec_perf     :  1;
        unsigned int reserved_0            : 25;
    } reg;
}media1_crg_clkst4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dpu_dvfs_enable     :  1;
        unsigned int div_aclk_media1_bus :  6;
        unsigned int div_pclk_media1_cfg :  6;
        unsigned int div_clk_dpu_core    :  6;
        unsigned int div_clk_dpu_veu     :  6;
        unsigned int reserved_0          :  7;
    } reg;
}media1_crg_clkst5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_vdec_core_cfg_n :  1;
        unsigned int ip_rst_vdec_cfg_n      :  1;
        unsigned int ip_rst_vdec_core_n     :  1;
        unsigned int ip_rst_vdec_core_axi_n :  1;
        unsigned int ip_rst_vdec_axi_n      :  1;
        unsigned int reserved_0             :  1;
        unsigned int ip_rst_dpu_dsi0_n      :  1;
        unsigned int ip_rst_dpu_dsi1_n      :  1;
        unsigned int ip_rst_dpu_dp_n        :  1;
        unsigned int ip_rst_disp_bus_cfg_n  :  1;
        unsigned int ip_rst_disp_bus_data_n :  1;
        unsigned int ip_rst_veu_brg_n       :  1;
        unsigned int ip_rst_dpu_brg_n       :  1;
        unsigned int ip_rst_dpu_veu_n       :  1;
        unsigned int ip_rst_dpu_dsc1_n      :  1;
        unsigned int ip_rst_dpu_dsc0_n      :  1;
        unsigned int ip_rst_dsi_brg_n       :  1;
        unsigned int ip_rst_dpu_core3_n     :  1;
        unsigned int ip_rst_dpu_core2_n     :  1;
        unsigned int ip_rst_dpu_core1_n     :  1;
        unsigned int ip_rst_dpu_core0_n     :  1;
        unsigned int ip_rst_dpu_dsi_cfg_n   :  1;
        unsigned int ip_rst_dpu_n           :  1;
        unsigned int reserved_1             :  1;
        unsigned int ip_rst_media1_hpm_n    :  1;
        unsigned int ip_rst_media1_gpc3_n   :  1;
        unsigned int ip_rst_media1_gpc2_n   :  1;
        unsigned int ip_rst_media1_gpc1_n   :  1;
        unsigned int ip_rst_media1_gpc0_n   :  1;
        unsigned int reserved_2             :  1;
        unsigned int ip_rst_media1_tcu_n    :  1;
        unsigned int ip_rst_media1_noc_n    :  1;
    } reg;
}media1_crg_rst0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_vdec_core_cfg_n :  1;
        unsigned int ip_rst_vdec_cfg_n      :  1;
        unsigned int ip_rst_vdec_core_n     :  1;
        unsigned int ip_rst_vdec_core_axi_n :  1;
        unsigned int ip_rst_vdec_axi_n      :  1;
        unsigned int reserved_0             :  1;
        unsigned int ip_rst_dpu_dsi0_n      :  1;
        unsigned int ip_rst_dpu_dsi1_n      :  1;
        unsigned int ip_rst_dpu_dp_n        :  1;
        unsigned int ip_rst_disp_bus_cfg_n  :  1;
        unsigned int ip_rst_disp_bus_data_n :  1;
        unsigned int ip_rst_veu_brg_n       :  1;
        unsigned int ip_rst_dpu_brg_n       :  1;
        unsigned int ip_rst_dpu_veu_n       :  1;
        unsigned int ip_rst_dpu_dsc1_n      :  1;
        unsigned int ip_rst_dpu_dsc0_n      :  1;
        unsigned int ip_rst_dsi_brg_n       :  1;
        unsigned int ip_rst_dpu_core3_n     :  1;
        unsigned int ip_rst_dpu_core2_n     :  1;
        unsigned int ip_rst_dpu_core1_n     :  1;
        unsigned int ip_rst_dpu_core0_n     :  1;
        unsigned int ip_rst_dpu_dsi_cfg_n   :  1;
        unsigned int ip_rst_dpu_n           :  1;
        unsigned int reserved_1             :  1;
        unsigned int ip_rst_media1_hpm_n    :  1;
        unsigned int ip_rst_media1_gpc3_n   :  1;
        unsigned int ip_rst_media1_gpc2_n   :  1;
        unsigned int ip_rst_media1_gpc1_n   :  1;
        unsigned int ip_rst_media1_gpc0_n   :  1;
        unsigned int reserved_2             :  1;
        unsigned int ip_rst_media1_tcu_n    :  1;
        unsigned int ip_rst_media1_noc_n    :  1;
    } reg;
}media1_crg_rst0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_vdec_core_cfg_n :  1;
        unsigned int ip_rst_vdec_cfg_n      :  1;
        unsigned int ip_rst_vdec_core_n     :  1;
        unsigned int ip_rst_vdec_core_axi_n :  1;
        unsigned int ip_rst_vdec_axi_n      :  1;
        unsigned int reserved_0             :  1;
        unsigned int ip_rst_dpu_dsi0_n      :  1;
        unsigned int ip_rst_dpu_dsi1_n      :  1;
        unsigned int ip_rst_dpu_dp_n        :  1;
        unsigned int ip_rst_disp_bus_cfg_n  :  1;
        unsigned int ip_rst_disp_bus_data_n :  1;
        unsigned int ip_rst_veu_brg_n       :  1;
        unsigned int ip_rst_dpu_brg_n       :  1;
        unsigned int ip_rst_dpu_veu_n       :  1;
        unsigned int ip_rst_dpu_dsc1_n      :  1;
        unsigned int ip_rst_dpu_dsc0_n      :  1;
        unsigned int ip_rst_dsi_brg_n       :  1;
        unsigned int ip_rst_dpu_core3_n     :  1;
        unsigned int ip_rst_dpu_core2_n     :  1;
        unsigned int ip_rst_dpu_core1_n     :  1;
        unsigned int ip_rst_dpu_core0_n     :  1;
        unsigned int ip_rst_dpu_dsi_cfg_n   :  1;
        unsigned int ip_rst_dpu_n           :  1;
        unsigned int reserved_1             :  1;
        unsigned int ip_rst_media1_hpm_n    :  1;
        unsigned int ip_rst_media1_gpc3_n   :  1;
        unsigned int ip_rst_media1_gpc2_n   :  1;
        unsigned int ip_rst_media1_gpc1_n   :  1;
        unsigned int ip_rst_media1_gpc0_n   :  1;
        unsigned int reserved_2             :  1;
        unsigned int ip_rst_media1_tcu_n    :  1;
        unsigned int ip_rst_media1_noc_n    :  1;
    } reg;
}media1_crg_rst0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                :  1;
        unsigned int ip_rst_dpu_core3_cfg_n    :  1;
        unsigned int ip_rst_dpu_core2_cfg_n    :  1;
        unsigned int ip_rst_dpu_core1_cfg_n    :  1;
        unsigned int ip_rst_dpu_core0_cfg_n    :  1;
        unsigned int ip_arst_vdec_perf_n       :  1;
        unsigned int ip_rst_dpu_veu_axi_perf_n :  1;
        unsigned int ip_rst_dpu_axi3_perf_n    :  1;
        unsigned int ip_rst_dpu_axi2_perf_n    :  1;
        unsigned int ip_rst_dpu_axi1_perf_n    :  1;
        unsigned int ip_rst_dpu_axi0_perf_n    :  1;
        unsigned int ip_rst_media1_perf_n      :  1;
        unsigned int reserved_1                :  1;
        unsigned int ip_rst_media1_bist_n      :  1;
        unsigned int reserved_2                :  1;
        unsigned int ip_rst_dpu_veu_cfg_n      :  1;
        unsigned int ip_rst_dpu_cfg_n          :  1;
        unsigned int reserved_3                : 15;
    } reg;
}media1_crg_rst1_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                :  1;
        unsigned int ip_rst_dpu_core3_cfg_n    :  1;
        unsigned int ip_rst_dpu_core2_cfg_n    :  1;
        unsigned int ip_rst_dpu_core1_cfg_n    :  1;
        unsigned int ip_rst_dpu_core0_cfg_n    :  1;
        unsigned int ip_arst_vdec_perf_n       :  1;
        unsigned int ip_rst_dpu_veu_axi_perf_n :  1;
        unsigned int ip_rst_dpu_axi3_perf_n    :  1;
        unsigned int ip_rst_dpu_axi2_perf_n    :  1;
        unsigned int ip_rst_dpu_axi1_perf_n    :  1;
        unsigned int ip_rst_dpu_axi0_perf_n    :  1;
        unsigned int ip_rst_media1_perf_n      :  1;
        unsigned int reserved_1                :  1;
        unsigned int ip_rst_media1_bist_n      :  1;
        unsigned int reserved_2                :  1;
        unsigned int ip_rst_dpu_veu_cfg_n      :  1;
        unsigned int ip_rst_dpu_cfg_n          :  1;
        unsigned int reserved_3                : 15;
    } reg;
}media1_crg_rst1_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                :  1;
        unsigned int ip_rst_dpu_core3_cfg_n    :  1;
        unsigned int ip_rst_dpu_core2_cfg_n    :  1;
        unsigned int ip_rst_dpu_core1_cfg_n    :  1;
        unsigned int ip_rst_dpu_core0_cfg_n    :  1;
        unsigned int ip_arst_vdec_perf_n       :  1;
        unsigned int ip_rst_dpu_veu_axi_perf_n :  1;
        unsigned int ip_rst_dpu_axi3_perf_n    :  1;
        unsigned int ip_rst_dpu_axi2_perf_n    :  1;
        unsigned int ip_rst_dpu_axi1_perf_n    :  1;
        unsigned int ip_rst_dpu_axi0_perf_n    :  1;
        unsigned int ip_rst_media1_perf_n      :  1;
        unsigned int reserved_1                :  1;
        unsigned int ip_rst_media1_bist_n      :  1;
        unsigned int reserved_2                :  1;
        unsigned int ip_rst_dpu_veu_cfg_n      :  1;
        unsigned int ip_rst_dpu_cfg_n          :  1;
        unsigned int reserved_3                : 15;
    } reg;
}media1_crg_rst1_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clkrst_flag_tcu_bypass        :  1;
        unsigned int clkrst_flag_debug_vdec_bypass :  1;
        unsigned int clkrst_flag_bist_bypass       :  1;
        unsigned int clkrst_flag_tpc_bypass        :  1;
        unsigned int clkrst_flag_sys_ctrl_bypass   :  1;
        unsigned int clkrst_flag_smmu_bypass       :  1;
        unsigned int reserved_0                    : 10;
        unsigned int _bm_                          : 16;
    } reg;
}media1_crg_m1_clkrst_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autogt_dpu_core0_bypass :  1;
        unsigned int autogt_dpu_core1_bypass :  1;
        unsigned int autogt_dpu_core2_bypass :  1;
        unsigned int autogt_dpu_core3_bypass :  1;
        unsigned int autogt_dpu_axi0_bypass  :  1;
        unsigned int autogt_dpu_axi1_bypass  :  1;
        unsigned int autogt_dpu_axi2_bypass  :  1;
        unsigned int autogt_dpu_axi3_bypass  :  1;
        unsigned int gt_clk_dpu_sw_en        :  4;
        unsigned int autogt_dpu_bypass       :  4;
        unsigned int _bm_                    : 16;
    } reg;
}media1_crg_m1_dpu_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dpu_dvfs_block               :  1;
        unsigned int dpu_dvfs_aclk_media1_bypass  :  1;
        unsigned int dpu_dvfs_pclk_media1_bypass  :  1;
        unsigned int dpu_dvfs_clk_dpu_core_bypass :  1;
        unsigned int dpu_dvfs_clk_dpu_veu_bypass  :  1;
        unsigned int reserved_0                   : 11;
        unsigned int _bm_                         : 16;
    } reg;
}media1_crg_m1_dpu_dvfs_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pu_req_poll_dpu_part1 :  8;
        unsigned int reserved_0            :  8;
        unsigned int _bm_                  : 16;
    } reg;
}media1_crg_gpc_pu_req_poll_dpu_part1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pu_req_poll_msk_dpu_part1 :  8;
        unsigned int reserved_0                :  8;
        unsigned int _bm_                      : 16;
    } reg;
}media1_crg_gpc_req_poll_msk_dpu_part1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clr_pu_gpc_dpu_part1 :  1;
        unsigned int intr_clr_pd_gpc_dpu_part1 :  1;
        unsigned int reserved_0                : 14;
        unsigned int _bm_                      : 16;
    } reg;
}media1_crg_intr_clr_pu_dpu_part1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pu_gpc_dpu_part1  :  1;
        unsigned int intr_pd_gpc_dpu_part1  :  1;
        unsigned int pwr_on_done_dpu_part1  :  1;
        unsigned int pwr_off_done_dpu_part1 :  1;
        unsigned int reserved_0             : 28;
    } reg;
}media1_crg_intr_pu_dpu_part1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_stat_dpu_part1 : 32;
    } reg;
}media1_crg_gpc_stat_dpu_part1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_stat_1_dpu_part1 : 32;
    } reg;
}media1_crg_gpc_stat_1_dpu_part1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pu_req_poll_dpu_part2 :  8;
        unsigned int reserved_0            :  8;
        unsigned int _bm_                  : 16;
    } reg;
}media1_crg_gpc_pu_req_poll_dpu_part2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pu_req_poll_msk_dpu_part2 :  8;
        unsigned int reserved_0                :  8;
        unsigned int _bm_                      : 16;
    } reg;
}media1_crg_gpc_req_poll_msk_dpu_part2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clr_pu_gpc_dpu_part2 :  1;
        unsigned int intr_clr_pd_gpc_dpu_part2 :  1;
        unsigned int reserved_0                : 14;
        unsigned int _bm_                      : 16;
    } reg;
}media1_crg_intr_clr_pu_dpu_part2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pu_gpc_dpu_part2  :  1;
        unsigned int intr_pd_gpc_dpu_part2  :  1;
        unsigned int pwr_on_done_dpu_part2  :  1;
        unsigned int pwr_off_done_dpu_part2 :  1;
        unsigned int reserved_0             : 28;
    } reg;
}media1_crg_intr_pu_dpu_part2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_stat_dpu_part2 : 32;
    } reg;
}media1_crg_gpc_stat_dpu_part2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_stat_1_dpu_part2 : 32;
    } reg;
}media1_crg_gpc_stat_1_dpu_part2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pu_req_poll_veu :  8;
        unsigned int reserved_0      :  8;
        unsigned int _bm_            : 16;
    } reg;
}media1_crg_gpc_pu_req_poll_veu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pu_req_poll_msk_veu :  8;
        unsigned int reserved_0          :  8;
        unsigned int _bm_                : 16;
    } reg;
}media1_crg_gpc_req_poll_msk_veu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clr_pu_gpc_veu :  1;
        unsigned int intr_clr_pd_gpc_veu :  1;
        unsigned int reserved_0          : 14;
        unsigned int _bm_                : 16;
    } reg;
}media1_crg_intr_clr_pu_veu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pu_gpc_veu  :  1;
        unsigned int intr_pd_gpc_veu  :  1;
        unsigned int pwr_on_done_veu  :  1;
        unsigned int pwr_off_done_veu :  1;
        unsigned int reserved_0       : 28;
    } reg;
}media1_crg_intr_pu_veu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_stat_veu : 32;
    } reg;
}media1_crg_gpc_stat_veu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_stat_1_veu : 32;
    } reg;
}media1_crg_gpc_stat_1_veu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pu_req_poll_vdec :  8;
        unsigned int reserved_0       :  8;
        unsigned int _bm_             : 16;
    } reg;
}media1_crg_gpc_pu_req_poll_vdec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pu_req_poll_msk_vdec :  8;
        unsigned int reserved_0           :  8;
        unsigned int _bm_                 : 16;
    } reg;
}media1_crg_gpc_req_poll_msk_vdec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clr_pu_gpc_vdec :  1;
        unsigned int intr_clr_pd_gpc_vdec :  1;
        unsigned int reserved_0           : 14;
        unsigned int _bm_                 : 16;
    } reg;
}media1_crg_intr_clr_pu_vdec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pu_gpc_vdec  :  1;
        unsigned int intr_pd_gpc_vdec  :  1;
        unsigned int pwr_on_done_vdec  :  1;
        unsigned int pwr_off_done_vdec :  1;
        unsigned int reserved_0        : 28;
    } reg;
}media1_crg_intr_pu_vdec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_stat_vdec : 32;
    } reg;
}media1_crg_gpc_stat_vdec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_stat_1_vdec : 32;
    } reg;
}media1_crg_gpc_stat_1_vdec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_bus_data_disp_bus_ini_bypass   :  6;
        unsigned int m1_bus_data_disp_bus_tgt_bypass   :  2;
        unsigned int disp_idle_flag_data_bypass        :  1;
        unsigned int m1_bus_data_vdec_bus_ini_bypass   :  1;
        unsigned int m1_bus_data_vdec_bus_tgt_bypass   :  1;
        unsigned int idle_vdec_core_data_bypass        :  1;
        unsigned int m1_bus_data_media1_bus_ini_bypass :  4;
        unsigned int m1_bus_data_media1_bus_tgt_bypass :  3;
        unsigned int m1_bus_cfg_disp_bus_ini_bypass    :  1;
        unsigned int m1_bus_cfg_disp_bus_tgt_bypass    :  2;
        unsigned int disp_idle_flag_cfg_bypass         :  1;
        unsigned int m1_bus_cfg_vdec_bus_ini_bypass    :  1;
        unsigned int m1_bus_cfg_vdec_bus_tgt_bypass    :  2;
        unsigned int idle_vdec_core_cfg_bypass         :  1;
        unsigned int m1_bus_cfg_media1_bus_ini_bypass  :  1;
        unsigned int m1_bus_cfg_media1_bus_tgt_bypass  :  4;
    } reg;
}media1_crg_autofsctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int idle_flag_perfstat_veu_bypass    :  1;
        unsigned int idle_flag_perfstat_dpu3_bypass   :  1;
        unsigned int idle_flag_perfstat_dpu2_bypass   :  1;
        unsigned int idle_flag_perfstat_dpu1_bypass   :  1;
        unsigned int idle_flag_perfstat_dpu0_bypass   :  1;
        unsigned int idle_flag_perfstat_vdec_bypass   :  1;
        unsigned int idle_flag_perfstat_media1_bypass :  1;
        unsigned int idle_flag_tcu_bypass             :  1;
        unsigned int idle_flag_perfstat_dpu4_bypass   :  1;
        unsigned int reserved_0                       : 23;
    } reg;
}media1_crg_autofsctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0 : 32;
    } reg;
}media1_crg_autofsctrl2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_m1_bus_cfg     : 10;
        unsigned int debounce_out_m1_bus_cfg    : 10;
        unsigned int debounce_bypass_m1_bus_cfg :  1;
        unsigned int div_auto_clk_m1_bus_cfg    :  6;
        unsigned int reserved_0                 :  4;
        unsigned int autofs_en_m1_bus_cfg       :  1;
    } reg;
}media1_crg_autofsctrl3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_m1_bus_data     : 10;
        unsigned int debounce_out_m1_bus_data    : 10;
        unsigned int debounce_bypass_m1_bus_data :  1;
        unsigned int div_auto_clk_m1_bus_data    :  6;
        unsigned int reserved_0                  :  4;
        unsigned int autofs_en_m1_bus_data       :  1;
    } reg;
}media1_crg_autofsctrl4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_dpu_core     : 10;
        unsigned int debounce_out_dpu_core    : 10;
        unsigned int debounce_bypass_dpu_core :  1;
        unsigned int reserved_0               : 11;
    } reg;
}media1_crg_autogtctrl5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_dpu_axi     : 10;
        unsigned int debounce_out_dpu_axi    : 10;
        unsigned int debounce_bypass_dpu_axi :  1;
        unsigned int reserved_0              : 11;
    } reg;
}media1_crg_autogtctrl6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int disp_idle_flag            :  1;
        unsigned int idle_flag_perfstat_veu    :  1;
        unsigned int idle_flag_perfstat_dpu4   :  1;
        unsigned int idle_flag_perfstat_dpu3   :  1;
        unsigned int idle_flag_perfstat_dpu2   :  1;
        unsigned int idle_flag_perfstat_dpu1   :  1;
        unsigned int idle_flag_perfstat_dpu0   :  1;
        unsigned int idle_flag_perfstat_vdec   :  1;
        unsigned int idle_vdec_core            :  1;
        unsigned int idle_flag_tcu             :  1;
        unsigned int idle_flag_perfstat_media1 :  1;
        unsigned int m1_bus_data_dvfs_state    :  1;
        unsigned int m1_bus_cfg_dvfs_state     :  1;
        unsigned int dpu_core_dvfs_state       :  1;
        unsigned int dpu_veu_dvfs_state        :  1;
        unsigned int reserved_0                : 17;
    } reg;
}media1_crg_idle_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_m1_bus_data : 16;
        unsigned int _bm_                      : 16;
    } reg;
}media1_crg_autofs_bypass0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_m1_bus_cfg : 16;
        unsigned int _bm_                     : 16;
    } reg;
}media1_crg_autofs_bypass1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int core_disp_bus_ini_nopengding_bypass :  7;
        unsigned int core_disp_bus_tgt_nopengding_bypass :  4;
        unsigned int axi_disp_bus_ini_nopengding_bypass  :  7;
        unsigned int axi_disp_bus_tgt_nopengding_bypass  :  4;
        unsigned int core_dpu_idle_bypass                :  3;
        unsigned int axi_dpu_idle_bypass                 :  3;
        unsigned int reserved_0                          :  4;
    } reg;
}media1_crg_autogt_bypass2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autogt_clk_dpu_core0 :  1;
        unsigned int autogt_clk_dpu_core1 :  1;
        unsigned int autogt_clk_dpu_core2 :  1;
        unsigned int autogt_clk_dpu_core3 :  1;
        unsigned int autogt_clk_dpu_axi0  :  1;
        unsigned int autogt_clk_dpu_axi1  :  1;
        unsigned int autogt_clk_dpu_axi2  :  1;
        unsigned int autogt_clk_dpu_axi3  :  1;
        unsigned int reserved_0           : 24;
    } reg;
}media1_crg_autogt_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_clear_0      :  1;
        unsigned int m1_hpm_ro_sel_0     :  6;
        unsigned int m1_hpm_ro_clk_div_0 :  2;
        unsigned int m1_hpm_ro_dbg_div_0 :  6;
        unsigned int reserved_0          :  1;
        unsigned int _bm_                : 16;
    } reg;
}media1_crg_m1_hpm0_cfg_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_ro_en_0         :  1;
        unsigned int m1_hpm_test_hw_en_0    :  1;
        unsigned int m1_hpm_test_hw_start_0 :  1;
        unsigned int m1_hpm_timer_cnt_0     :  8;
        unsigned int reserved_0             :  5;
        unsigned int _bm_                   : 16;
    } reg;
}media1_crg_m1_hpm0_cfg_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_clk_gt_0 :  1;
        unsigned int reserved_0      : 31;
    } reg;
}media1_crg_m1_hpm0_clk_gt_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_clk_gt_0 :  1;
        unsigned int reserved_0      : 31;
    } reg;
}media1_crg_m1_hpm0_clk_gt_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_clk_gt_0 :  1;
        unsigned int reserved_0      : 31;
    } reg;
}media1_crg_m1_hpm0_clk_gt_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_dout_0 : 18;
        unsigned int reserved_0    : 14;
    } reg;
}media1_crg_m1_hpm0_dout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_clear_1      :  1;
        unsigned int m1_hpm_ro_sel_1     :  6;
        unsigned int m1_hpm_ro_clk_div_1 :  2;
        unsigned int m1_hpm_ro_dbg_div_1 :  6;
        unsigned int reserved_0          :  1;
        unsigned int _bm_                : 16;
    } reg;
}media1_crg_m1_hpm1_cfg_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_ro_en_1         :  1;
        unsigned int m1_hpm_test_hw_en_1    :  1;
        unsigned int m1_hpm_test_hw_start_1 :  1;
        unsigned int m1_hpm_timer_cnt_1     :  8;
        unsigned int reserved_0             :  5;
        unsigned int _bm_                   : 16;
    } reg;
}media1_crg_m1_hpm1_cfg_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_clk_gt_1 :  1;
        unsigned int reserved_0      : 31;
    } reg;
}media1_crg_m1_hpm1_clk_gt_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_clk_gt_1 :  1;
        unsigned int reserved_0      : 31;
    } reg;
}media1_crg_m1_hpm1_clk_gt_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_clk_gt_1 :  1;
        unsigned int reserved_0      : 31;
    } reg;
}media1_crg_m1_hpm1_clk_gt_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m1_hpm_dout_1 : 18;
        unsigned int reserved_0    : 14;
    } reg;
}media1_crg_m1_hpm1_dout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_en_dpu_part1 :  1;
        unsigned int reserved_0       : 15;
        unsigned int _bm_             : 16;
    } reg;
}media1_crg_gpc_en_dpu_part1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_ctrl0_dpu_part1 : 32;
    } reg;
}media1_crg_gpc_ctrl_0_dpu_part1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_ctrl1_dpu_part1 : 32;
    } reg;
}media1_crg_gpc_ctrl_1_dpu_part1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int state_debug_dpu_part1 :  2;
        unsigned int reserved_0            : 30;
    } reg;
}media1_crg_gpc_stat_debug_dpu_part1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_en_dpu_part2 :  1;
        unsigned int reserved_0       : 15;
        unsigned int _bm_             : 16;
    } reg;
}media1_crg_gpc_en_dpu_part2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_ctrl0_dpu_part2 : 32;
    } reg;
}media1_crg_gpc_ctrl_0_dpu_part2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_ctrl1_dpu_part2 : 32;
    } reg;
}media1_crg_gpc_ctrl_1_dpu_part2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int state_debug_dpu_part2 :  2;
        unsigned int reserved_0            : 30;
    } reg;
}media1_crg_gpc_stat_debug_dpu_part2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_en_veu :  1;
        unsigned int reserved_0 : 15;
        unsigned int _bm_       : 16;
    } reg;
}media1_crg_gpc_en_veu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_ctrl0_veu : 32;
    } reg;
}media1_crg_gpc_ctrl_0_veu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_ctrl1_veu : 32;
    } reg;
}media1_crg_gpc_ctrl_1_veu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int state_debug_veu :  2;
        unsigned int reserved_0      : 30;
    } reg;
}media1_crg_gpc_stat_debug_veu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_en_vdec :  1;
        unsigned int reserved_0  : 15;
        unsigned int _bm_        : 16;
    } reg;
}media1_crg_gpc_en_vdec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_ctrl0_vdec : 32;
    } reg;
}media1_crg_gpc_ctrl_0_vdec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpc_ctrl1_vdec : 32;
    } reg;
}media1_crg_gpc_ctrl_1_vdec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int state_debug_vdec :  2;
        unsigned int reserved_0       : 30;
    } reg;
}media1_crg_gpc_stat_debug_vdec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}media1_crg_regfile_cgbypass_t;

#endif
