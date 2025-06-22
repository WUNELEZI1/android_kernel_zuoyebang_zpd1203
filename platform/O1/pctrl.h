// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef PCTRL_REGIF_H
#define PCTRL_REGIF_H

#define PCTRL_VERSION                               0x0000
#define PCTRL_MEM_LGROUP0                           0x0080
#define PCTRL_MEM_LGROUP0_SC_PCIE0_MEM_SD_SHIFT                                         0
#define PCTRL_MEM_LGROUP0_SC_PCIE0_MEM_SD_MASK                                          0x00000001
#define PCTRL_MEM_LPSTATUS0                         0x0084
#define PCTRL_MEM_LPSTATUS0_PCIE0_MEM_SD_ACK_SHIFT                                      0
#define PCTRL_MEM_LPSTATUS0_PCIE0_MEM_SD_ACK_MASK                                       0x00000001
#define PCTRL_MEM_LGROUP1                           0x0088
#define PCTRL_MEM_LGROUP1_SC_PCIE1_MEM_SD_SHIFT                                         0
#define PCTRL_MEM_LGROUP1_SC_PCIE1_MEM_SD_MASK                                          0x00000001
#define PCTRL_MEM_LPSTATUS1                         0x008c
#define PCTRL_MEM_LPSTATUS1_PCIE1_MEM_SD_ACK_SHIFT                                      0
#define PCTRL_MEM_LPSTATUS1_PCIE1_MEM_SD_ACK_MASK                                       0x00000001
#define PCTRL_DMANS_DEBUG_CH_NUM_I                  0x0100
#define PCTRL_DMANS_DEBUG_CH_NUM_I_DMANS_DEBUG_CH_NUM_I_SHIFT                           0
#define PCTRL_DMANS_DEBUG_CH_NUM_I_DMANS_DEBUG_CH_NUM_I_MASK                            0x0000001f
#define PCTRL_DMANS_DEBUG0                          0x0104
#define PCTRL_DMANS_DEBUG1                          0x0108
#define PCTRL_DMANS_DEBUG2                          0x010c
#define PCTRL_DMANS_DEBUG3                          0x0110
#define PCTRL_DMANS_DEBUG4                          0x0114
#define PCTRL_DMANS_DEBUG5                          0x0118
#define PCTRL_DMANS_LP0                             0x011c
#define PCTRL_DMANS_LP1                             0x0120
#define PCTRL_INTR_EXC0_RAW_ST                      0x0200
#define PCTRL_INTR_EXC0_MASK_ST                     0x0204
#define PCTRL_INTR_EXC0_MASK_L                      0x0208
#define PCTRL_INTR_EXC0_MASK_L_INTR_EXC0_MASK_L_SHIFT                                   0
#define PCTRL_INTR_EXC0_MASK_L_INTR_EXC0_MASK_L_MASK                                    0x0000ffff
#define PCTRL_INTR_EXC0_MASK_H                      0x020c
#define PCTRL_INTR_EXC0_MASK_H_INTR_EXC0_MASK_H_SHIFT                                   0
#define PCTRL_INTR_EXC0_MASK_H_INTR_EXC0_MASK_H_MASK                                    0x0000ffff
#define PCTRL_INTR_EXC2_RAW_ST                      0x0210
#define PCTRL_INTR_EXC2_MASK_ST                     0x0214
#define PCTRL_INTR_EXC2_MASK_L                      0x0218
#define PCTRL_INTR_EXC2_MASK_L_INTR_EXC2_MASK_L_SHIFT                                   0
#define PCTRL_INTR_EXC2_MASK_L_INTR_EXC2_MASK_L_MASK                                    0x0000ffff
#define PCTRL_INTR_EXC2_MASK_H                      0x021c
#define PCTRL_INTR_EXC2_MASK_H_INTR_EXC2_MASK_H_SHIFT                                   0
#define PCTRL_INTR_EXC2_MASK_H_INTR_EXC2_MASK_H_MASK                                    0x0000ffff
#define PCTRL_IDLE_FLAG_PERFSTAT                    0x0280
#define PCTRL_IDLE_FLAG_PERFSTAT_IDLE_FLAG_PERFSTAT_NPU0_SHIFT                          0
#define PCTRL_IDLE_FLAG_PERFSTAT_IDLE_FLAG_PERFSTAT_NPU0_MASK                           0x00000001
#define PCTRL_IDLE_FLAG_PERFSTAT_IDLE_FLAG_PERFSTAT_NPU1_SHIFT                          1
#define PCTRL_IDLE_FLAG_PERFSTAT_IDLE_FLAG_PERFSTAT_NPU1_MASK                           0x00000002
#define PCTRL_IDLE_FLAG_PERFSTAT_IDLE_FLAG_PERFSTAT_NPU2_SHIFT                          2
#define PCTRL_IDLE_FLAG_PERFSTAT_IDLE_FLAG_PERFSTAT_NPU2_MASK                           0x00000004
#define PCTRL_IDLE_FLAG_PERFSTAT_IDLE_FLAG_PERFSTAT_NPU3_SHIFT                          3
#define PCTRL_IDLE_FLAG_PERFSTAT_IDLE_FLAG_PERFSTAT_NPU3_MASK                           0x00000008
#define PCTRL_IDLE_FLAG_PERFSTAT_IDLE_FLAG_PERFSTAT_SYS1_SHIFT                          4
#define PCTRL_IDLE_FLAG_PERFSTAT_IDLE_FLAG_PERFSTAT_SYS1_MASK                           0x00000010
#define PCTRL_IPC_GT_BYPASS                         0x0300
#define PCTRL_IPC_GT_BYPASS_IPC0_GT_BYPASS_SHIFT                                        0
#define PCTRL_IPC_GT_BYPASS_IPC0_GT_BYPASS_MASK                                         0x00000001
#define PCTRL_IPC_GT_BYPASS_IPC1_GT_BYPASS_SHIFT                                        1
#define PCTRL_IPC_GT_BYPASS_IPC1_GT_BYPASS_MASK                                         0x00000002
#define PCTRL_IPC_GT_BYPASS_IPC2_GT_BYPASS_SHIFT                                        2
#define PCTRL_IPC_GT_BYPASS_IPC2_GT_BYPASS_MASK                                         0x00000004
#define PCTRL_ISOLATION_GROUP0                      0x0800
#define PCTRL_ISOLATION_GROUP0_SC_DDR_ISO_EN_SHIFT                                      0
#define PCTRL_ISOLATION_GROUP0_SC_DDR_ISO_EN_MASK                                       0x00000001
#define PCTRL_ISOLATION_GROUP1                      0x0804
#define PCTRL_ISOLATION_GROUP1_SC_MEDIA1_ISO_EN_SHIFT                                   0
#define PCTRL_ISOLATION_GROUP1_SC_MEDIA1_ISO_EN_MASK                                    0x00000001
#define PCTRL_ISOLATION_GROUP2                      0x0808
#define PCTRL_ISOLATION_GROUP2_SC_MEDIA2_ISO_EN_SHIFT                                   0
#define PCTRL_ISOLATION_GROUP2_SC_MEDIA2_ISO_EN_MASK                                    0x00000001
#define PCTRL_MEM_LGROUP5                           0x0880
#define PCTRL_MEM_LGROUP5_SC_CPU_TOP_MEM_SD_SHIFT                                       0
#define PCTRL_MEM_LGROUP5_SC_CPU_TOP_MEM_SD_MASK                                        0x00000001
#define PCTRL_MEM_LGROUP5_SC_CPU_TOP_MEM_POFF_SHIFT                                     1
#define PCTRL_MEM_LGROUP5_SC_CPU_TOP_MEM_POFF_MASK                                      0x00000006
#define PCTRL_MEM_LGROUP5_SC_CPU_TOP_MEM_DS_SHIFT                                       3
#define PCTRL_MEM_LGROUP5_SC_CPU_TOP_MEM_DS_MASK                                        0x00000008
#define PCTRL_MEM_LPSTATUS5                         0x0884
#define PCTRL_MEM_LPSTATUS5_CPU_TOP_MEM_SD_ACK_SHIFT                                    0
#define PCTRL_MEM_LPSTATUS5_CPU_TOP_MEM_SD_ACK_MASK                                     0x00000001
#define PCTRL_MEM_LPSTATUS5_CPU_TOP_MEM_DS_ACK_SHIFT                                    1
#define PCTRL_MEM_LPSTATUS5_CPU_TOP_MEM_DS_ACK_MASK                                     0x00000002
#define PCTRL_MEM_LGROUP6                           0x0888
#define PCTRL_MEM_LGROUP6_SC_DDR_MEM_SD_SHIFT                                           0
#define PCTRL_MEM_LGROUP6_SC_DDR_MEM_SD_MASK                                            0x00000001
#define PCTRL_MEM_LGROUP6_SC_DDR_MEM_POFF_SHIFT                                         1
#define PCTRL_MEM_LGROUP6_SC_DDR_MEM_POFF_MASK                                          0x00000006
#define PCTRL_MEM_LGROUP6_SC_DDR_MEM_DS_SHIFT                                           3
#define PCTRL_MEM_LGROUP6_SC_DDR_MEM_DS_MASK                                            0x00000008
#define PCTRL_MEM_LPSTATUS6                         0x088c
#define PCTRL_MEM_LPSTATUS6_DDRA_MEM_SD_ACK_SHIFT                                       0
#define PCTRL_MEM_LPSTATUS6_DDRA_MEM_SD_ACK_MASK                                        0x00000001
#define PCTRL_MEM_LPSTATUS6_DDRB_MEM_SD_ACK_SHIFT                                       1
#define PCTRL_MEM_LPSTATUS6_DDRB_MEM_SD_ACK_MASK                                        0x00000002
#define PCTRL_MEM_LPSTATUS6_DDRC_MEM_SD_ACK_SHIFT                                       2
#define PCTRL_MEM_LPSTATUS6_DDRC_MEM_SD_ACK_MASK                                        0x00000004
#define PCTRL_MEM_LPSTATUS6_DDRD_MEM_SD_ACK_SHIFT                                       3
#define PCTRL_MEM_LPSTATUS6_DDRD_MEM_SD_ACK_MASK                                        0x00000008
#define PCTRL_MEM_LPSTATUS6_DDRA_MEM_DS_ACK_SHIFT                                       4
#define PCTRL_MEM_LPSTATUS6_DDRA_MEM_DS_ACK_MASK                                        0x00000010
#define PCTRL_MEM_LPSTATUS6_DDRB_MEM_DS_ACK_SHIFT                                       5
#define PCTRL_MEM_LPSTATUS6_DDRB_MEM_DS_ACK_MASK                                        0x00000020
#define PCTRL_MEM_LPSTATUS6_DDRC_MEM_DS_ACK_SHIFT                                       6
#define PCTRL_MEM_LPSTATUS6_DDRC_MEM_DS_ACK_MASK                                        0x00000040
#define PCTRL_MEM_LPSTATUS6_DDRD_MEM_DS_ACK_SHIFT                                       7
#define PCTRL_MEM_LPSTATUS6_DDRD_MEM_DS_ACK_MASK                                        0x00000080
#define PCTRL_MEM_CTRL_BUS_LOW                      0x08c0
#define PCTRL_MEM_CTRL_BUS_MID                      0x08c4
#define PCTRL_MEM_CTRL_BUS_HIGH                     0x08c8
#define PCTRL_DMAS_DEBUG_CH_NUM_I                   0x0900
#define PCTRL_DMAS_DEBUG_CH_NUM_I_DMAS_DEBUG_CH_NUM_I_SHIFT                             0
#define PCTRL_DMAS_DEBUG_CH_NUM_I_DMAS_DEBUG_CH_NUM_I_MASK                              0x0000001f
#define PCTRL_DMAS_DEBUG0                           0x0904
#define PCTRL_DMAS_DEBUG1                           0x0908
#define PCTRL_DMAS_DEBUG2                           0x090c
#define PCTRL_DMAS_DEBUG3                           0x0910
#define PCTRL_DMAS_DEBUG4                           0x0914
#define PCTRL_DMAS_DEBUG5                           0x0918
#define PCTRL_DMAS_LP0                              0x091c
#define PCTRL_DMAS_LP1                              0x0920
#define PCTRL_PERICFG_NSFW0_FAIL                    0x0a00
#define PCTRL_PERICFG_NSFW0_FAIL_PERICFG_NSFW0_PERROR_SHIFT                             0
#define PCTRL_PERICFG_NSFW0_FAIL_PERICFG_NSFW0_PERROR_MASK                              0x00000001
#define PCTRL_PERICFG_NSFW0_FAIL_PERICFG_NSFW0_CLEAR_SHIFT                              4
#define PCTRL_PERICFG_NSFW0_FAIL_PERICFG_NSFW0_FAIL_PMID_SHIFT                          8
#define PCTRL_PERICFG_NSFW0_FAIL_PERICFG_NSFW0_FAIL_PMID_MASK                           0x0000ff00
#define PCTRL_PERICFG_NSFW0_FAIL_ADDR               0x0a04
#define PCTRL_PERICFG_NSFW1_FAIL                    0x0a08
#define PCTRL_PERICFG_NSFW1_FAIL_PERICFG_NSFW1_PERROR_SHIFT                             0
#define PCTRL_PERICFG_NSFW1_FAIL_PERICFG_NSFW1_PERROR_MASK                              0x00000001
#define PCTRL_PERICFG_NSFW1_FAIL_PERICFG_NSFW1_CLEAR_SHIFT                              4
#define PCTRL_PERICFG_NSFW1_FAIL_PERICFG_NSFW1_FAIL_PMID_SHIFT                          8
#define PCTRL_PERICFG_NSFW1_FAIL_PERICFG_NSFW1_FAIL_PMID_MASK                           0x0000ff00
#define PCTRL_PERICFG_NSFW1_FAIL_ADDR               0x0a0c
#define PCTRL_DMA_NSFW_FAIL                         0x0a10
#define PCTRL_DMA_NSFW_FAIL_DMA_NSFW_PERROR_SHIFT                                       0
#define PCTRL_DMA_NSFW_FAIL_DMA_NSFW_PERROR_MASK                                        0x00000001
#define PCTRL_DMA_NSFW_FAIL_DMA_NSFW_CLEAR_SHIFT                                        4
#define PCTRL_DMA_NSFW_FAIL_DMA_NSFW_FAIL_PMID_SHIFT                                    8
#define PCTRL_DMA_NSFW_FAIL_DMA_NSFW_FAIL_PMID_MASK                                     0x0000ff00
#define PCTRL_DMA_NSFW_FAIL_ADDR                    0x0a14
#define PCTRL_PMPU_BYPASS                           0x0a80
#define PCTRL_PMPU_BYPASS_PMPU_BYPASS_SHIFT                                             0
#define PCTRL_PMPU_BYPASS_PMPU_BYPASS_MASK                                              0x00000001
#define PCTRL_TESTPIN_SEL_CFG                       0x0b00
#define PCTRL_TESTPIN_SEL_CFG_TESTPIN_SEL_CFG_SHIFT                                     0
#define PCTRL_TESTPIN_SEL_CFG_TESTPIN_SEL_CFG_MASK                                      0x0000000f
#define PCTRL_PERICFG_TESTPIN_SEL_CFG               0x0b04
#define PCTRL_PERICFG_TESTPIN_SEL_CFG_PERICFG_TESTPIN_SEL_CFG_SHIFT                     0
#define PCTRL_PERICFG_TESTPIN_SEL_CFG_PERICFG_TESTPIN_SEL_CFG_MASK                      0x0000000f
#define PCTRL_TPC_ADDR_MASKED0                      0x0c00
#define PCTRL_TPC_ADDR_MASKED0_TPC_ADDR_MASKED0_SHIFT                                   0
#define PCTRL_TPC_ADDR_MASKED0_TPC_ADDR_MASKED0_MASK                                    0x07ffffff
#define PCTRL_TPC_ADDR_MASKED1                      0x0c04
#define PCTRL_TPC_ADDR_MASKED1_TPC_ADDR_MASKED1_SHIFT                                   0
#define PCTRL_TPC_ADDR_MASKED1_TPC_ADDR_MASKED1_MASK                                    0x07ffffff
#define PCTRL_INTL_MODE                             0x0c08
#define PCTRL_INTL_MODE_INTL_MODE_SHIFT                                                 0
#define PCTRL_INTL_MODE_INTL_MODE_MASK                                                  0x00000003
#define PCTRL_PERI_BROADCAST_BUS_CTRL               0x0c0c
#define PCTRL_PERI_BROADCAST_BUS_CTRL_BROADCAST_SEL_R_SHIFT                             0
#define PCTRL_PERI_BROADCAST_BUS_CTRL_BROADCAST_SEL_R_MASK                              0x0000000f
#define PCTRL_PERI_BROADCAST_BUS_CTRL_BROADCAST_EN_R_SHIFT                              4
#define PCTRL_PERI_BROADCAST_BUS_CTRL_BROADCAST_EN_R_MASK                               0x00000010
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB                 0x0d00
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_BROADCAST2DDRA_ADB400_PWRQ_PERMIT_DENY_SAR_SHIFT    0
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_BROADCAST2DDRA_ADB400_PWRQ_PERMIT_DENY_SAR_MASK     0x00000001
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_BROADCAST2DDRA_ADB400_PWRQACTIVES_SHIFT             1
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_BROADCAST2DDRA_ADB400_PWRQACTIVES_MASK              0x00000002
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_BROADCAST2DDRA_ADB400_PWRQDENYS_SHIFT               2
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_BROADCAST2DDRA_ADB400_PWRQDENYS_MASK                0x00000004
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_BROADCAST2DDRA_ADB400_PWRQACCEPTNS_SHIFT            3
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_BROADCAST2DDRA_ADB400_PWRQACCEPTNS_MASK             0x00000008
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_BROADCAST2DDRA_ADB400_PWRQREQNS_SHIFT               4
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_BROADCAST2DDRA_ADB400_PWRQREQNS_MASK                0x00000010
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_MAIN2DDRA_ADB400_PWRQ_PERMIT_DENY_SAR_SHIFT         5
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_MAIN2DDRA_ADB400_PWRQ_PERMIT_DENY_SAR_MASK          0x00000020
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_MAIN2DDRA_ADB400_PWRQACTIVES_SHIFT                  6
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_MAIN2DDRA_ADB400_PWRQACTIVES_MASK                   0x00000040
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_MAIN2DDRA_ADB400_PWRQDENYS_SHIFT                    7
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_MAIN2DDRA_ADB400_PWRQDENYS_MASK                     0x00000080
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_MAIN2DDRA_ADB400_PWRQACCEPTNS_SHIFT                 8
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_MAIN2DDRA_ADB400_PWRQACCEPTNS_MASK                  0x00000100
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_MAIN2DDRA_ADB400_PWRQREQNS_SHIFT                    9
#define PCTRL_DDR_SUBSYS_WRAP_A_ADB_MAIN2DDRA_ADB400_PWRQREQNS_MASK                     0x00000200
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB                 0x0d04
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_BROADCAST2DDRB_ADB400_PWRQ_PERMIT_DENY_SAR_SHIFT    0
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_BROADCAST2DDRB_ADB400_PWRQ_PERMIT_DENY_SAR_MASK     0x00000001
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_BROADCAST2DDRB_ADB400_PWRQACTIVES_SHIFT             1
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_BROADCAST2DDRB_ADB400_PWRQACTIVES_MASK              0x00000002
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_BROADCAST2DDRB_ADB400_PWRQDENYS_SHIFT               2
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_BROADCAST2DDRB_ADB400_PWRQDENYS_MASK                0x00000004
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_BROADCAST2DDRB_ADB400_PWRQACCEPTNS_SHIFT            3
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_BROADCAST2DDRB_ADB400_PWRQACCEPTNS_MASK             0x00000008
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_BROADCAST2DDRB_ADB400_PWRQREQNS_SHIFT               4
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_BROADCAST2DDRB_ADB400_PWRQREQNS_MASK                0x00000010
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_MAIN2DDRB_ADB400_PWRQ_PERMIT_DENY_SAR_SHIFT         5
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_MAIN2DDRB_ADB400_PWRQ_PERMIT_DENY_SAR_MASK          0x00000020
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_MAIN2DDRB_ADB400_PWRQACTIVES_SHIFT                  6
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_MAIN2DDRB_ADB400_PWRQACTIVES_MASK                   0x00000040
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_MAIN2DDRB_ADB400_PWRQDENYS_SHIFT                    7
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_MAIN2DDRB_ADB400_PWRQDENYS_MASK                     0x00000080
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_MAIN2DDRB_ADB400_PWRQACCEPTNS_SHIFT                 8
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_MAIN2DDRB_ADB400_PWRQACCEPTNS_MASK                  0x00000100
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_MAIN2DDRB_ADB400_PWRQREQNS_SHIFT                    9
#define PCTRL_DDR_SUBSYS_WRAP_B_ADB_MAIN2DDRB_ADB400_PWRQREQNS_MASK                     0x00000200
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB                 0x0d08
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_BROADCAST2DDRC_ADB400_PWRQ_PERMIT_DENY_SAR_SHIFT    0
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_BROADCAST2DDRC_ADB400_PWRQ_PERMIT_DENY_SAR_MASK     0x00000001
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_BROADCAST2DDRC_ADB400_PWRQACTIVES_SHIFT             1
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_BROADCAST2DDRC_ADB400_PWRQACTIVES_MASK              0x00000002
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_BROADCAST2DDRC_ADB400_PWRQDENYS_SHIFT               2
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_BROADCAST2DDRC_ADB400_PWRQDENYS_MASK                0x00000004
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_BROADCAST2DDRC_ADB400_PWRQACCEPTNS_SHIFT            3
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_BROADCAST2DDRC_ADB400_PWRQACCEPTNS_MASK             0x00000008
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_BROADCAST2DDRC_ADB400_PWRQREQNS_SHIFT               4
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_BROADCAST2DDRC_ADB400_PWRQREQNS_MASK                0x00000010
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_MAIN2DDRC_ADB400_PWRQ_PERMIT_DENY_SAR_SHIFT         5
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_MAIN2DDRC_ADB400_PWRQ_PERMIT_DENY_SAR_MASK          0x00000020
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_MAIN2DDRC_ADB400_PWRQACTIVES_SHIFT                  6
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_MAIN2DDRC_ADB400_PWRQACTIVES_MASK                   0x00000040
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_MAIN2DDRC_ADB400_PWRQDENYS_SHIFT                    7
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_MAIN2DDRC_ADB400_PWRQDENYS_MASK                     0x00000080
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_MAIN2DDRC_ADB400_PWRQACCEPTNS_SHIFT                 8
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_MAIN2DDRC_ADB400_PWRQACCEPTNS_MASK                  0x00000100
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_MAIN2DDRC_ADB400_PWRQREQNS_SHIFT                    9
#define PCTRL_DDR_SUBSYS_WRAP_C_ADB_MAIN2DDRC_ADB400_PWRQREQNS_MASK                     0x00000200
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB                 0x0d0c
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_BROADCAST2DDRD_ADB400_PWRQ_PERMIT_DENY_SAR_SHIFT    0
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_BROADCAST2DDRD_ADB400_PWRQ_PERMIT_DENY_SAR_MASK     0x00000001
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_BROADCAST2DDRD_ADB400_PWRQACTIVES_SHIFT             1
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_BROADCAST2DDRD_ADB400_PWRQACTIVES_MASK              0x00000002
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_BROADCAST2DDRD_ADB400_PWRQDENYS_SHIFT               2
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_BROADCAST2DDRD_ADB400_PWRQDENYS_MASK                0x00000004
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_BROADCAST2DDRD_ADB400_PWRQACCEPTNS_SHIFT            3
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_BROADCAST2DDRD_ADB400_PWRQACCEPTNS_MASK             0x00000008
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_BROADCAST2DDRD_ADB400_PWRQREQNS_SHIFT               4
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_BROADCAST2DDRD_ADB400_PWRQREQNS_MASK                0x00000010
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_MAIN2DDRD_ADB400_PWRQ_PERMIT_DENY_SAR_SHIFT         5
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_MAIN2DDRD_ADB400_PWRQ_PERMIT_DENY_SAR_MASK          0x00000020
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_MAIN2DDRD_ADB400_PWRQACTIVES_SHIFT                  6
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_MAIN2DDRD_ADB400_PWRQACTIVES_MASK                   0x00000040
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_MAIN2DDRD_ADB400_PWRQDENYS_SHIFT                    7
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_MAIN2DDRD_ADB400_PWRQDENYS_MASK                     0x00000080
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_MAIN2DDRD_ADB400_PWRQACCEPTNS_SHIFT                 8
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_MAIN2DDRD_ADB400_PWRQACCEPTNS_MASK                  0x00000100
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_MAIN2DDRD_ADB400_PWRQREQNS_SHIFT                    9
#define PCTRL_DDR_SUBSYS_WRAP_D_ADB_MAIN2DDRD_ADB400_PWRQREQNS_MASK                     0x00000200
#define PCTRL_XCTRL_CPU_STATUS                      0x0e00
#define PCTRL_XCTRL_DDR_STATUS                      0x0e04
#define PCTRL_INTR_EXC1_RAW_ST                      0x0e80
#define PCTRL_INTR_EXC1_MASK_ST                     0x0e84
#define PCTRL_INTR_EXC1_MASK_L                      0x0e88
#define PCTRL_INTR_EXC1_MASK_L_INTR_EXC1_MASK_L_SHIFT                                   0
#define PCTRL_INTR_EXC1_MASK_L_INTR_EXC1_MASK_L_MASK                                    0x0000ffff
#define PCTRL_INTR_EXC1_MASK_H                      0x0e8c
#define PCTRL_INTR_EXC1_MASK_H_INTR_EXC1_MASK_H_SHIFT                                   0
#define PCTRL_INTR_EXC1_MASK_H_INTR_EXC1_MASK_H_MASK                                    0x0000ffff
#define PCTRL_TIMER_NS_DEBUG                        0x1000
#define PCTRL_TIMER_NS_DEBUG_TIMER_NS_PAUSE_SHIFT                                       0
#define PCTRL_TIMER_NS_DEBUG_TIMER_NS_PAUSE_MASK                                        0x0000000f
#define PCTRL_TIMER_NS_DEBUG_TIMER_NS_EN_SHIFT                                          4
#define PCTRL_TIMER_NS_DEBUG_TIMER_NS_EN_MASK                                           0x000000f0
#define PCTRL_WDT_NS_DEBUG                          0x1004
#define PCTRL_WDT_NS_DEBUG_WDT_NS_PAUSE_SHIFT                                           0
#define PCTRL_WDT_NS_DEBUG_WDT_NS_PAUSE_MASK                                            0x00000003
#define PCTRL_I2C0_DEBUG                            0x1080
#define PCTRL_I2C0_DEBUG_I2C0_DEBUG_SHIFT                                               0
#define PCTRL_I2C0_DEBUG_I2C0_DEBUG_MASK                                                0x003fffff
#define PCTRL_I2C1_DEBUG                            0x1084
#define PCTRL_I2C1_DEBUG_I2C1_DEBUG_SHIFT                                               0
#define PCTRL_I2C1_DEBUG_I2C1_DEBUG_MASK                                                0x003fffff
#define PCTRL_I2C2_DEBUG                            0x1088
#define PCTRL_I2C2_DEBUG_I2C2_DEBUG_SHIFT                                               0
#define PCTRL_I2C2_DEBUG_I2C2_DEBUG_MASK                                                0x003fffff
#define PCTRL_I2C3_DEBUG                            0x108c
#define PCTRL_I2C3_DEBUG_I2C3_DEBUG_SHIFT                                               0
#define PCTRL_I2C3_DEBUG_I2C3_DEBUG_MASK                                                0x003fffff
#define PCTRL_I2C4_DEBUG                            0x1090
#define PCTRL_I2C4_DEBUG_I2C4_DEBUG_SHIFT                                               0
#define PCTRL_I2C4_DEBUG_I2C4_DEBUG_MASK                                                0x003fffff
#define PCTRL_I2C5_DEBUG                            0x1094
#define PCTRL_I2C5_DEBUG_I2C5_DEBUG_SHIFT                                               0
#define PCTRL_I2C5_DEBUG_I2C5_DEBUG_MASK                                                0x003fffff
#define PCTRL_I2C6_DEBUG                            0x1098
#define PCTRL_I2C6_DEBUG_I2C6_DEBUG_SHIFT                                               0
#define PCTRL_I2C6_DEBUG_I2C6_DEBUG_MASK                                                0x003fffff
#define PCTRL_I2C9_DEBUG                            0x109c
#define PCTRL_I2C9_DEBUG_I2C9_DEBUG_SHIFT                                               0
#define PCTRL_I2C9_DEBUG_I2C9_DEBUG_MASK                                                0x003fffff
#define PCTRL_I2C10_DEBUG                           0x10a0
#define PCTRL_I2C10_DEBUG_I2C10_DEBUG_SHIFT                                             0
#define PCTRL_I2C10_DEBUG_I2C10_DEBUG_MASK                                              0x003fffff
#define PCTRL_I2C11_DEBUG                           0x10a4
#define PCTRL_I2C11_DEBUG_I2C11_DEBUG_SHIFT                                             0
#define PCTRL_I2C11_DEBUG_I2C11_DEBUG_MASK                                              0x003fffff
#define PCTRL_I2C12_DEBUG                           0x10a8
#define PCTRL_I2C12_DEBUG_I2C12_DEBUG_SHIFT                                             0
#define PCTRL_I2C12_DEBUG_I2C12_DEBUG_MASK                                              0x003fffff
#define PCTRL_I2C13_DEBUG                           0x10ac
#define PCTRL_I2C13_DEBUG_I2C13_DEBUG_SHIFT                                             0
#define PCTRL_I2C13_DEBUG_I2C13_DEBUG_MASK                                              0x003fffff
#define PCTRL_I2C20_DEBUG                           0x10b0
#define PCTRL_I2C20_DEBUG_I2C20_DEBUG_SHIFT                                             0
#define PCTRL_I2C20_DEBUG_I2C20_DEBUG_MASK                                              0x003fffff
#define PCTRL_SPI4_DEBUG                            0x10b4
#define PCTRL_SPI4_DEBUG_SPI4_DEBUG_SHIFT                                               0
#define PCTRL_SPI4_DEBUG_SPI4_DEBUG_MASK                                                0x0000000f
#define PCTRL_SPI4_DEBUG_SPI4_SS_IN_N_SHIFT                                             4
#define PCTRL_SPI4_DEBUG_SPI4_SS_IN_N_MASK                                              0x00000010
#define PCTRL_SPI5_DEBUG                            0x10b8
#define PCTRL_SPI5_DEBUG_SPI5_DEBUG_SHIFT                                               0
#define PCTRL_SPI5_DEBUG_SPI5_DEBUG_MASK                                                0x0000000f
#define PCTRL_SPI5_DEBUG_SPI5_SS_IN_N_SHIFT                                             4
#define PCTRL_SPI5_DEBUG_SPI5_SS_IN_N_MASK                                              0x00000010
#define PCTRL_SPI6_DEBUG                            0x10bc
#define PCTRL_SPI6_DEBUG_SPI6_DEBUG_SHIFT                                               0
#define PCTRL_SPI6_DEBUG_SPI6_DEBUG_MASK                                                0x0000000f
#define PCTRL_SPI6_DEBUG_SPI6_SS_IN_N_SHIFT                                             4
#define PCTRL_SPI6_DEBUG_SPI6_SS_IN_N_MASK                                              0x00000010
#define PCTRL_UART3_DEBUG                           0x10c0
#define PCTRL_UART6_DEBUG                           0x10c4
#define PCTRL_I3C0_DEBUG_LOW                        0x10c8
#define PCTRL_I3C0_DEBUG_HIGH                       0x10cc
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_0              0x1100
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_0_GPIO_ACPU_MASK_NS_PERI_0_SHIFT                   0
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_0_GPIO_ACPU_MASK_NS_PERI_0_MASK                    0x0000ffff
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_1              0x1104
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_1_GPIO_ACPU_MASK_NS_PERI_1_SHIFT                   0
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_1_GPIO_ACPU_MASK_NS_PERI_1_MASK                    0x0000ffff
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_2              0x1108
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_2_GPIO_ACPU_MASK_NS_PERI_2_SHIFT                   0
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_2_GPIO_ACPU_MASK_NS_PERI_2_MASK                    0x0000ffff
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_3              0x110c
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_3_GPIO_ACPU_MASK_NS_PERI_3_SHIFT                   0
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_3_GPIO_ACPU_MASK_NS_PERI_3_MASK                    0x0000ffff
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_4              0x1110
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_4_GPIO_ACPU_MASK_NS_PERI_4_SHIFT                   0
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_4_GPIO_ACPU_MASK_NS_PERI_4_MASK                    0x0000ffff
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_5              0x1114
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_5_GPIO_ACPU_MASK_NS_PERI_5_SHIFT                   0
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_5_GPIO_ACPU_MASK_NS_PERI_5_MASK                    0x0000ffff
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_6              0x1118
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_6_GPIO_ACPU_MASK_NS_PERI_6_SHIFT                   0
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_6_GPIO_ACPU_MASK_NS_PERI_6_MASK                    0x0000ffff
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_7              0x111c
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_7_GPIO_ACPU_MASK_NS_PERI_7_SHIFT                   0
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_7_GPIO_ACPU_MASK_NS_PERI_7_MASK                    0x0000ffff
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_8              0x1120
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_8_GPIO_ACPU_MASK_NS_PERI_8_SHIFT                   0
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_8_GPIO_ACPU_MASK_NS_PERI_8_MASK                    0x0000ffff
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_9              0x1124
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_9_GPIO_ACPU_MASK_NS_PERI_9_SHIFT                   0
#define PCTRL_GPIO_ACPU_MASK_NS_PERI_9_GPIO_ACPU_MASK_NS_PERI_9_MASK                    0x0000000f
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_0            0x1128
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_0_GPIO_LPCORE_MASK_NS_PERI_0_SHIFT               0
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_0_GPIO_LPCORE_MASK_NS_PERI_0_MASK                0x0000ffff
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_1            0x112c
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_1_GPIO_LPCORE_MASK_NS_PERI_1_SHIFT               0
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_1_GPIO_LPCORE_MASK_NS_PERI_1_MASK                0x0000ffff
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_2            0x1130
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_2_GPIO_LPCORE_MASK_NS_PERI_2_SHIFT               0
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_2_GPIO_LPCORE_MASK_NS_PERI_2_MASK                0x0000ffff
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_3            0x1134
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_3_GPIO_LPCORE_MASK_NS_PERI_3_SHIFT               0
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_3_GPIO_LPCORE_MASK_NS_PERI_3_MASK                0x0000ffff
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_4            0x1138
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_4_GPIO_LPCORE_MASK_NS_PERI_4_SHIFT               0
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_4_GPIO_LPCORE_MASK_NS_PERI_4_MASK                0x0000ffff
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_5            0x113c
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_5_GPIO_LPCORE_MASK_NS_PERI_5_SHIFT               0
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_5_GPIO_LPCORE_MASK_NS_PERI_5_MASK                0x0000ffff
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_6            0x1140
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_6_GPIO_LPCORE_MASK_NS_PERI_6_SHIFT               0
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_6_GPIO_LPCORE_MASK_NS_PERI_6_MASK                0x0000ffff
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_7            0x1144
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_7_GPIO_LPCORE_MASK_NS_PERI_7_SHIFT               0
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_7_GPIO_LPCORE_MASK_NS_PERI_7_MASK                0x0000ffff
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_8            0x1148
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_8_GPIO_LPCORE_MASK_NS_PERI_8_SHIFT               0
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_8_GPIO_LPCORE_MASK_NS_PERI_8_MASK                0x0000ffff
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_9            0x114c
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_9_GPIO_LPCORE_MASK_NS_PERI_9_SHIFT               0
#define PCTRL_GPIO_LPCORE_MASK_NS_PERI_9_GPIO_LPCORE_MASK_NS_PERI_9_MASK                0x0000000f
#define PCTRL_GPIO_SH_MASK_NS_PERI_0                0x1150
#define PCTRL_GPIO_SH_MASK_NS_PERI_0_GPIO_SH_MASK_NS_PERI_0_SHIFT                       0
#define PCTRL_GPIO_SH_MASK_NS_PERI_0_GPIO_SH_MASK_NS_PERI_0_MASK                        0x0000ffff
#define PCTRL_GPIO_SH_MASK_NS_PERI_1                0x1154
#define PCTRL_GPIO_SH_MASK_NS_PERI_1_GPIO_SH_MASK_NS_PERI_1_SHIFT                       0
#define PCTRL_GPIO_SH_MASK_NS_PERI_1_GPIO_SH_MASK_NS_PERI_1_MASK                        0x0000ffff
#define PCTRL_GPIO_SH_MASK_NS_PERI_2                0x1158
#define PCTRL_GPIO_SH_MASK_NS_PERI_2_GPIO_SH_MASK_NS_PERI_2_SHIFT                       0
#define PCTRL_GPIO_SH_MASK_NS_PERI_2_GPIO_SH_MASK_NS_PERI_2_MASK                        0x0000ffff
#define PCTRL_GPIO_SH_MASK_NS_PERI_3                0x115c
#define PCTRL_GPIO_SH_MASK_NS_PERI_3_GPIO_SH_MASK_NS_PERI_3_SHIFT                       0
#define PCTRL_GPIO_SH_MASK_NS_PERI_3_GPIO_SH_MASK_NS_PERI_3_MASK                        0x0000ffff
#define PCTRL_GPIO_SH_MASK_NS_PERI_4                0x1160
#define PCTRL_GPIO_SH_MASK_NS_PERI_4_GPIO_SH_MASK_NS_PERI_4_SHIFT                       0
#define PCTRL_GPIO_SH_MASK_NS_PERI_4_GPIO_SH_MASK_NS_PERI_4_MASK                        0x0000ffff
#define PCTRL_GPIO_SH_MASK_NS_PERI_5                0x1164
#define PCTRL_GPIO_SH_MASK_NS_PERI_5_GPIO_SH_MASK_NS_PERI_5_SHIFT                       0
#define PCTRL_GPIO_SH_MASK_NS_PERI_5_GPIO_SH_MASK_NS_PERI_5_MASK                        0x0000ffff
#define PCTRL_GPIO_SH_MASK_NS_PERI_6                0x1168
#define PCTRL_GPIO_SH_MASK_NS_PERI_6_GPIO_SH_MASK_NS_PERI_6_SHIFT                       0
#define PCTRL_GPIO_SH_MASK_NS_PERI_6_GPIO_SH_MASK_NS_PERI_6_MASK                        0x0000ffff
#define PCTRL_GPIO_SH_MASK_NS_PERI_7                0x116c
#define PCTRL_GPIO_SH_MASK_NS_PERI_7_GPIO_SH_MASK_NS_PERI_7_SHIFT                       0
#define PCTRL_GPIO_SH_MASK_NS_PERI_7_GPIO_SH_MASK_NS_PERI_7_MASK                        0x0000ffff
#define PCTRL_GPIO_SH_MASK_NS_PERI_8                0x1170
#define PCTRL_GPIO_SH_MASK_NS_PERI_8_GPIO_SH_MASK_NS_PERI_8_SHIFT                       0
#define PCTRL_GPIO_SH_MASK_NS_PERI_8_GPIO_SH_MASK_NS_PERI_8_MASK                        0x0000ffff
#define PCTRL_GPIO_SH_MASK_NS_PERI_9                0x1174
#define PCTRL_GPIO_SH_MASK_NS_PERI_9_GPIO_SH_MASK_NS_PERI_9_SHIFT                       0
#define PCTRL_GPIO_SH_MASK_NS_PERI_9_GPIO_SH_MASK_NS_PERI_9_MASK                        0x0000000f
#define PCTRL_FC_QOS                                0x1200
#define PCTRL_FC_QOS_FC_QOS_0_SHIFT                                                     0
#define PCTRL_FC_QOS_FC_QOS_0_MASK                                                      0x0000000f
#define PCTRL_FC_QOS_FC_QOS_1_SHIFT                                                     4
#define PCTRL_FC_QOS_FC_QOS_1_MASK                                                      0x000000f0
#define PCTRL_FC_QOS_FC_QOS_2_SHIFT                                                     8
#define PCTRL_FC_QOS_FC_QOS_2_MASK                                                      0x00000f00
#define PCTRL_FC_QOS_FC_QOS_3_SHIFT                                                     12
#define PCTRL_FC_QOS_FC_QOS_3_MASK                                                      0x0000f000
#define PCTRL_FC_QOS_FC_QOS_4_SHIFT                                                     16
#define PCTRL_FC_QOS_FC_QOS_4_MASK                                                      0x000f0000
#define PCTRL_FC_QOS_FC_QOS_5_SHIFT                                                     20
#define PCTRL_FC_QOS_FC_QOS_5_MASK                                                      0x00f00000
#define PCTRL_FC_QOS_FC_QOS_6_SHIFT                                                     24
#define PCTRL_FC_QOS_FC_QOS_6_MASK                                                      0x0f000000
#define PCTRL_FC_QOS_FC_QOS_7_SHIFT                                                     28
#define PCTRL_FC_QOS_FC_QOS_7_MASK                                                      0xf0000000
#define PCTRL_FLUX_REQ                              0x1204
#define PCTRL_FLUX_REQ_ISP_FLUX_REQ_R_SHIFT                                             0
#define PCTRL_FLUX_REQ_ISP_FLUX_REQ_R_MASK                                              0x0000000f
#define PCTRL_FLUX_REQ_ISP_FLUX_REQ_W_SHIFT                                             4
#define PCTRL_FLUX_REQ_ISP_FLUX_REQ_W_MASK                                              0x000000f0
#define PCTRL_FLUX_REQ_DPU_FLUX_REQ_R_SHIFT                                             8
#define PCTRL_FLUX_REQ_DPU_FLUX_REQ_R_MASK                                              0x00000f00
#define PCTRL_FLUX_REQ_DPU_FLUX_REQ_W_SHIFT                                             12
#define PCTRL_FLUX_REQ_DPU_FLUX_REQ_W_MASK                                              0x0000f000
#define PCTRL_DDR0_QOS_ALLOW                        0x1208
#define PCTRL_DDR0_QOS_ALLOW_DDR0_WL_QOS_ALLOW_R_SHIFT                                  0
#define PCTRL_DDR0_QOS_ALLOW_DDR0_WL_QOS_ALLOW_R_MASK                                   0x000000ff
#define PCTRL_DDR0_QOS_ALLOW_DDR0_WL_QOS_ALLOW_W_SHIFT                                  8
#define PCTRL_DDR0_QOS_ALLOW_DDR0_WL_QOS_ALLOW_W_MASK                                   0x0000ff00
#define PCTRL_DDR0_QOS_ALLOW_DDR0_DVFS_GOING_QOS_ALLOW_R_SHIFT                          16
#define PCTRL_DDR0_QOS_ALLOW_DDR0_DVFS_GOING_QOS_ALLOW_R_MASK                           0x00ff0000
#define PCTRL_DDR0_QOS_ALLOW_DDR0_DVFS_GOING_QOS_ALLOW_W_SHIFT                          24
#define PCTRL_DDR0_QOS_ALLOW_DDR0_DVFS_GOING_QOS_ALLOW_W_MASK                           0xff000000
#define PCTRL_DDR1_QOS_ALLOW                        0x120c
#define PCTRL_DDR1_QOS_ALLOW_DDR1_WL_QOS_ALLOW_R_SHIFT                                  0
#define PCTRL_DDR1_QOS_ALLOW_DDR1_WL_QOS_ALLOW_R_MASK                                   0x000000ff
#define PCTRL_DDR1_QOS_ALLOW_DDR1_WL_QOS_ALLOW_W_SHIFT                                  8
#define PCTRL_DDR1_QOS_ALLOW_DDR1_WL_QOS_ALLOW_W_MASK                                   0x0000ff00
#define PCTRL_DDR1_QOS_ALLOW_DDR1_DVFS_GOING_QOS_ALLOW_R_SHIFT                          16
#define PCTRL_DDR1_QOS_ALLOW_DDR1_DVFS_GOING_QOS_ALLOW_R_MASK                           0x00ff0000
#define PCTRL_DDR1_QOS_ALLOW_DDR1_DVFS_GOING_QOS_ALLOW_W_SHIFT                          24
#define PCTRL_DDR1_QOS_ALLOW_DDR1_DVFS_GOING_QOS_ALLOW_W_MASK                           0xff000000
#define PCTRL_DDR2_QOS_ALLOW                        0x1210
#define PCTRL_DDR2_QOS_ALLOW_DDR2_WL_QOS_ALLOW_R_SHIFT                                  0
#define PCTRL_DDR2_QOS_ALLOW_DDR2_WL_QOS_ALLOW_R_MASK                                   0x000000ff
#define PCTRL_DDR2_QOS_ALLOW_DDR2_WL_QOS_ALLOW_W_SHIFT                                  8
#define PCTRL_DDR2_QOS_ALLOW_DDR2_WL_QOS_ALLOW_W_MASK                                   0x0000ff00
#define PCTRL_DDR2_QOS_ALLOW_DDR2_DVFS_GOING_QOS_ALLOW_R_SHIFT                          16
#define PCTRL_DDR2_QOS_ALLOW_DDR2_DVFS_GOING_QOS_ALLOW_R_MASK                           0x00ff0000
#define PCTRL_DDR2_QOS_ALLOW_DDR2_DVFS_GOING_QOS_ALLOW_W_SHIFT                          24
#define PCTRL_DDR2_QOS_ALLOW_DDR2_DVFS_GOING_QOS_ALLOW_W_MASK                           0xff000000
#define PCTRL_DDR3_QOS_ALLOW                        0x1214
#define PCTRL_DDR3_QOS_ALLOW_DDR3_WL_QOS_ALLOW_R_SHIFT                                  0
#define PCTRL_DDR3_QOS_ALLOW_DDR3_WL_QOS_ALLOW_R_MASK                                   0x000000ff
#define PCTRL_DDR3_QOS_ALLOW_DDR3_WL_QOS_ALLOW_W_SHIFT                                  8
#define PCTRL_DDR3_QOS_ALLOW_DDR3_WL_QOS_ALLOW_W_MASK                                   0x0000ff00
#define PCTRL_DDR3_QOS_ALLOW_DDR3_DVFS_GOING_QOS_ALLOW_R_SHIFT                          16
#define PCTRL_DDR3_QOS_ALLOW_DDR3_DVFS_GOING_QOS_ALLOW_R_MASK                           0x00ff0000
#define PCTRL_DDR3_QOS_ALLOW_DDR3_DVFS_GOING_QOS_ALLOW_W_SHIFT                          24
#define PCTRL_DDR3_QOS_ALLOW_DDR3_DVFS_GOING_QOS_ALLOW_W_MASK                           0xff000000
#define PCTRL_SYS_QOS_ALLOW                         0x1218
#define PCTRL_SYS_QOS_ALLOW_SYS_QOS_ALLOW_R_SHIFT                                       0
#define PCTRL_SYS_QOS_ALLOW_SYS_QOS_ALLOW_R_MASK                                        0x000000ff
#define PCTRL_SYS_QOS_ALLOW_SYS_QOS_ALLOW_W_SHIFT                                       8
#define PCTRL_SYS_QOS_ALLOW_SYS_QOS_ALLOW_W_MASK                                        0x0000ff00
#define PCTRL_FLOW_CTRL_EN_L                        0x1220
#define PCTRL_FLOW_CTRL_EN_L_FLOW_CTRL_EN_L_SHIFT                                       0
#define PCTRL_FLOW_CTRL_EN_L_FLOW_CTRL_EN_L_MASK                                        0x0000ffff
#define PCTRL_FLOW_CTRL_EN_H                        0x1224
#define PCTRL_FLOW_CTRL_EN_H_FLOW_CTRL_EN_H_SHIFT                                       0
#define PCTRL_FLOW_CTRL_EN_H_FLOW_CTRL_EN_H_MASK                                        0x0000ffff
#define PCTRL_FC_QOS_0_ALLOW_CNT_R                  0x1228
#define PCTRL_FC_QOS_1_ALLOW_CNT_R                  0x122c
#define PCTRL_FC_QOS_2_ALLOW_CNT_R                  0x1230
#define PCTRL_FC_QOS_3_ALLOW_CNT_R                  0x1234
#define PCTRL_FC_QOS_4_ALLOW_CNT_R                  0x1238
#define PCTRL_FC_QOS_5_ALLOW_CNT_R                  0x123c
#define PCTRL_FC_QOS_6_ALLOW_CNT_R                  0x1240
#define PCTRL_FC_QOS_7_ALLOW_CNT_R                  0x1244
#define PCTRL_FC_QOS_0_ALLOW_CNT_W                  0x1248
#define PCTRL_FC_QOS_1_ALLOW_CNT_W                  0x124c
#define PCTRL_FC_QOS_2_ALLOW_CNT_W                  0x1250
#define PCTRL_FC_QOS_3_ALLOW_CNT_W                  0x1254
#define PCTRL_FC_QOS_4_ALLOW_CNT_W                  0x1258
#define PCTRL_FC_QOS_5_ALLOW_CNT_W                  0x125c
#define PCTRL_FC_QOS_6_ALLOW_CNT_W                  0x1260
#define PCTRL_FC_QOS_7_ALLOW_CNT_W                  0x1264
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG0      0x1280
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG1      0x1284
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG2      0x1288
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG3      0x128c
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG4      0x1290
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG5      0x1294
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG6      0x1298
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG6_SYS1_MAIN_TPC_BWC_DYN_QOS4_W_TARGET_SHIFT 0
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG6_SYS1_MAIN_TPC_BWC_DYN_QOS4_W_TARGET_MASK  0x000000ff
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG6_SYS1_MAIN_TPC_BWC_DYN_QOS5_W_TARGET_SHIFT 8
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG6_SYS1_MAIN_TPC_BWC_DYN_QOS5_W_TARGET_MASK  0x0000ff00
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG6_SYS1_MAIN_TPC_BWC_DYN_QOS6_W_TARGET_SHIFT 16
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG6_SYS1_MAIN_TPC_BWC_DYN_QOS6_W_TARGET_MASK  0x00ff0000
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG6_SYS1_MAIN_TPC_BWC_DYN_QOS7_W_TARGET_SHIFT 24
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG6_SYS1_MAIN_TPC_BWC_DYN_QOS7_W_TARGET_MASK  0xff000000
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG7      0x129c
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG7_SYS1_MAIN_TPC_BWC_DYN_QOS0_W_TARGET_SHIFT 0
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG7_SYS1_MAIN_TPC_BWC_DYN_QOS0_W_TARGET_MASK  0x000000ff
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG7_SYS1_MAIN_TPC_BWC_DYN_QOS1_W_TARGET_SHIFT 8
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG7_SYS1_MAIN_TPC_BWC_DYN_QOS1_W_TARGET_MASK  0x0000ff00
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG7_SYS1_MAIN_TPC_BWC_DYN_QOS2_W_TARGET_SHIFT 16
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG7_SYS1_MAIN_TPC_BWC_DYN_QOS2_W_TARGET_MASK  0x00ff0000
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG7_SYS1_MAIN_TPC_BWC_DYN_QOS3_W_TARGET_SHIFT 24
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG7_SYS1_MAIN_TPC_BWC_DYN_QOS3_W_TARGET_MASK  0xff000000
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG8      0x12a0
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG8_SYS1_MAIN_TPC_BWC_DYN_QOS4_R_TARGET_SHIFT 0
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG8_SYS1_MAIN_TPC_BWC_DYN_QOS4_R_TARGET_MASK  0x000000ff
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG8_SYS1_MAIN_TPC_BWC_DYN_QOS5_R_TARGET_SHIFT 8
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG8_SYS1_MAIN_TPC_BWC_DYN_QOS5_R_TARGET_MASK  0x0000ff00
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG8_SYS1_MAIN_TPC_BWC_DYN_QOS6_R_TARGET_SHIFT 16
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG8_SYS1_MAIN_TPC_BWC_DYN_QOS6_R_TARGET_MASK  0x00ff0000
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG8_SYS1_MAIN_TPC_BWC_DYN_QOS7_R_TARGET_SHIFT 24
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG8_SYS1_MAIN_TPC_BWC_DYN_QOS7_R_TARGET_MASK  0xff000000
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG9      0x12a4
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG9_SYS1_MAIN_TPC_BWC_DYN_QOS0_R_TARGET_SHIFT 0
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG9_SYS1_MAIN_TPC_BWC_DYN_QOS0_R_TARGET_MASK  0x000000ff
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG9_SYS1_MAIN_TPC_BWC_DYN_QOS1_R_TARGET_SHIFT 8
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG9_SYS1_MAIN_TPC_BWC_DYN_QOS1_R_TARGET_MASK  0x0000ff00
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG9_SYS1_MAIN_TPC_BWC_DYN_QOS2_R_TARGET_SHIFT 16
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG9_SYS1_MAIN_TPC_BWC_DYN_QOS2_R_TARGET_MASK  0x00ff0000
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG9_SYS1_MAIN_TPC_BWC_DYN_QOS3_R_TARGET_SHIFT 24
#define PCTRL_SYS1_MAIN_TPC_NORMAL_CRG_CONFIG9_SYS1_MAIN_TPC_BWC_DYN_QOS3_R_TARGET_MASK  0xff000000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0             0x12a8
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_EN_SHIFT                          0
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_EN_MASK                           0x00000001
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_BW_COUNTER_EN_SHIFT               1
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_BW_COUNTER_EN_MASK                0x00000002
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_BWC_STATIC_R_EN_SHIFT             2
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_BWC_STATIC_R_EN_MASK              0x00000004
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_BWC_STATIC_W_EN_SHIFT             3
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_BWC_STATIC_W_EN_MASK              0x00000008
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_BWC_DYN_R_EN_SHIFT                4
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_BWC_DYN_R_EN_MASK                 0x00000010
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_BWC_DYN_W_EN_SHIFT                5
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_BWC_DYN_W_EN_MASK                 0x00000020
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_COUNT_OUTSTANDING_EN_SHIFT        6
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_COUNT_OUTSTANDING_EN_MASK         0x00000040
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_AFBC_ADDR_SCRAMBLE_EN_SHIFT       7
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_AFBC_ADDR_SCRAMBLE_EN_MASK        0x00000080
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_COUNT_AR_MAX_SHIFT                8
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_COUNT_AR_MAX_MASK                 0x00007f00
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0__SHIFT                                          15
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0__MASK                                           0x00008000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_COUNT_AW_MAX_SHIFT                16
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_SYS1_MAIN_TPC_COUNT_AW_MAX_MASK                 0x007f0000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_1_SHIFT                                         23
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG0_1_MASK                                          0xff800000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG1             0x12ac
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG1_SYS1_MAIN_TPC_BWC_STATIC_R_TARGET_SHIFT         0
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG1_SYS1_MAIN_TPC_BWC_STATIC_R_TARGET_MASK          0x000000ff
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG1_SYS1_MAIN_TPC_BWC_STATIC_W_TARGET_SHIFT         8
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG1_SYS1_MAIN_TPC_BWC_STATIC_W_TARGET_MASK          0x0000ff00
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG1__SHIFT                                          16
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG1__MASK                                           0xffff0000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2             0x12b0
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_0_SHIFT                       0
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_0_MASK                        0x0000000f
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_1_SHIFT                       4
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_1_MASK                        0x000000f0
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_2_SHIFT                       8
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_2_MASK                        0x00000f00
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_3_SHIFT                       12
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_3_MASK                        0x0000f000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_4_SHIFT                       16
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_4_MASK                        0x000f0000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_5_SHIFT                       20
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_5_MASK                        0x00f00000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_6_SHIFT                       24
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_6_MASK                        0x0f000000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_7_SHIFT                       28
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_7_MASK                        0x70000000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_SEL_SHIFT                     31
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG2_SYS1_MAIN_TPC_QOS_SEL_MASK                      0x80000000
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG3             0x12b4
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG3_SYS1_MAIN_TPC_BWC_R_SAT_SHIFT                   0
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG3_SYS1_MAIN_TPC_BWC_R_SAT_MASK                    0x0000ffff
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG3_SYS1_MAIN_TPC_BWC_W_SAT_SHIFT                   16
#define PCTRL_SYS1_MAIN_TPC_CRG_CONFIG3_SYS1_MAIN_TPC_BWC_W_SAT_MASK                    0xffff0000
#define PCTRL_SYS1_MAIN_TPC_CRG_STATUS0             0x12b8
#define PCTRL_SYS1_MAIN_TPC_CRG_STATUS1             0x12bc
#define PCTRL_SYS1_MAIN_TPC_CRG_STATUS2             0x12c0
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG0       0x1300
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG1       0x1304
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG2       0x1308
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG3       0x130c
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG4       0x1310
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG5       0x1314
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG6       0x1318
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG6_NPU_MAIN_TPC_BWC_DYN_QOS4_W_TARGET_SHIFT  0
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG6_NPU_MAIN_TPC_BWC_DYN_QOS4_W_TARGET_MASK   0x000000ff
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG6_NPU_MAIN_TPC_BWC_DYN_QOS5_W_TARGET_SHIFT  8
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG6_NPU_MAIN_TPC_BWC_DYN_QOS5_W_TARGET_MASK   0x0000ff00
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG6_NPU_MAIN_TPC_BWC_DYN_QOS6_W_TARGET_SHIFT  16
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG6_NPU_MAIN_TPC_BWC_DYN_QOS6_W_TARGET_MASK   0x00ff0000
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG6_NPU_MAIN_TPC_BWC_DYN_QOS7_W_TARGET_SHIFT  24
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG6_NPU_MAIN_TPC_BWC_DYN_QOS7_W_TARGET_MASK   0xff000000
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG7       0x131c
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG7_NPU_MAIN_TPC_BWC_DYN_QOS0_W_TARGET_SHIFT  0
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG7_NPU_MAIN_TPC_BWC_DYN_QOS0_W_TARGET_MASK   0x000000ff
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG7_NPU_MAIN_TPC_BWC_DYN_QOS1_W_TARGET_SHIFT  8
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG7_NPU_MAIN_TPC_BWC_DYN_QOS1_W_TARGET_MASK   0x0000ff00
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG7_NPU_MAIN_TPC_BWC_DYN_QOS2_W_TARGET_SHIFT  16
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG7_NPU_MAIN_TPC_BWC_DYN_QOS2_W_TARGET_MASK   0x00ff0000
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG7_NPU_MAIN_TPC_BWC_DYN_QOS3_W_TARGET_SHIFT  24
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG7_NPU_MAIN_TPC_BWC_DYN_QOS3_W_TARGET_MASK   0xff000000
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG8       0x1320
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG8_NPU_MAIN_TPC_BWC_DYN_QOS4_R_TARGET_SHIFT  0
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG8_NPU_MAIN_TPC_BWC_DYN_QOS4_R_TARGET_MASK   0x000000ff
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG8_NPU_MAIN_TPC_BWC_DYN_QOS5_R_TARGET_SHIFT  8
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG8_NPU_MAIN_TPC_BWC_DYN_QOS5_R_TARGET_MASK   0x0000ff00
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG8_NPU_MAIN_TPC_BWC_DYN_QOS6_R_TARGET_SHIFT  16
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG8_NPU_MAIN_TPC_BWC_DYN_QOS6_R_TARGET_MASK   0x00ff0000
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG8_NPU_MAIN_TPC_BWC_DYN_QOS7_R_TARGET_SHIFT  24
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG8_NPU_MAIN_TPC_BWC_DYN_QOS7_R_TARGET_MASK   0xff000000
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG9       0x1324
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG9_NPU_MAIN_TPC_BWC_DYN_QOS0_R_TARGET_SHIFT  0
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG9_NPU_MAIN_TPC_BWC_DYN_QOS0_R_TARGET_MASK   0x000000ff
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG9_NPU_MAIN_TPC_BWC_DYN_QOS1_R_TARGET_SHIFT  8
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG9_NPU_MAIN_TPC_BWC_DYN_QOS1_R_TARGET_MASK   0x0000ff00
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG9_NPU_MAIN_TPC_BWC_DYN_QOS2_R_TARGET_SHIFT  16
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG9_NPU_MAIN_TPC_BWC_DYN_QOS2_R_TARGET_MASK   0x00ff0000
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG9_NPU_MAIN_TPC_BWC_DYN_QOS3_R_TARGET_SHIFT  24
#define PCTRL_NPU_MAIN_TPC_NORMAL_CRG_CONFIG9_NPU_MAIN_TPC_BWC_DYN_QOS3_R_TARGET_MASK   0xff000000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0              0x1328
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_EN_SHIFT                            0
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_EN_MASK                             0x00000001
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_BW_COUNTER_EN_SHIFT                 1
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_BW_COUNTER_EN_MASK                  0x00000002
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_BWC_STATIC_R_EN_SHIFT               2
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_BWC_STATIC_R_EN_MASK                0x00000004
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_BWC_STATIC_W_EN_SHIFT               3
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_BWC_STATIC_W_EN_MASK                0x00000008
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_BWC_DYN_R_EN_SHIFT                  4
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_BWC_DYN_R_EN_MASK                   0x00000010
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_BWC_DYN_W_EN_SHIFT                  5
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_BWC_DYN_W_EN_MASK                   0x00000020
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_COUNT_OUTSTANDING_EN_SHIFT          6
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_COUNT_OUTSTANDING_EN_MASK           0x00000040
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_AFBC_ADDR_SCRAMBLE_EN_SHIFT         7
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_AFBC_ADDR_SCRAMBLE_EN_MASK          0x00000080
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_COUNT_AR_MAX_SHIFT                  8
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_COUNT_AR_MAX_MASK                   0x00007f00
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0__SHIFT                                           15
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0__MASK                                            0x00008000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_COUNT_AW_MAX_SHIFT                  16
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_NPU_MAIN_TPC_COUNT_AW_MAX_MASK                   0x007f0000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_1_SHIFT                                          23
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG0_1_MASK                                           0xff800000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG1              0x132c
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG1_NPU_MAIN_TPC_BWC_STATIC_R_TARGET_SHIFT           0
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG1_NPU_MAIN_TPC_BWC_STATIC_R_TARGET_MASK            0x000000ff
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG1_NPU_MAIN_TPC_BWC_STATIC_W_TARGET_SHIFT           8
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG1_NPU_MAIN_TPC_BWC_STATIC_W_TARGET_MASK            0x0000ff00
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG1__SHIFT                                           16
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG1__MASK                                            0xffff0000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2              0x1330
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_0_SHIFT                         0
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_0_MASK                          0x0000000f
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_1_SHIFT                         4
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_1_MASK                          0x000000f0
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_2_SHIFT                         8
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_2_MASK                          0x00000f00
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_3_SHIFT                         12
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_3_MASK                          0x0000f000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_4_SHIFT                         16
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_4_MASK                          0x000f0000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_5_SHIFT                         20
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_5_MASK                          0x00f00000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_6_SHIFT                         24
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_6_MASK                          0x0f000000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_7_SHIFT                         28
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_7_MASK                          0x70000000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_SEL_SHIFT                       31
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG2_NPU_MAIN_TPC_QOS_SEL_MASK                        0x80000000
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG3              0x1334
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG3_NPU_MAIN_TPC_BWC_R_SAT_SHIFT                     0
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG3_NPU_MAIN_TPC_BWC_R_SAT_MASK                      0x0000ffff
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG3_NPU_MAIN_TPC_BWC_W_SAT_SHIFT                     16
#define PCTRL_NPU_MAIN_TPC_CRG_CONFIG3_NPU_MAIN_TPC_BWC_W_SAT_MASK                      0xffff0000
#define PCTRL_NPU_MAIN_TPC0_CRG_STATUS0             0x1338
#define PCTRL_NPU_MAIN_TPC0_CRG_STATUS1             0x133c
#define PCTRL_NPU_MAIN_TPC0_CRG_STATUS2             0x1340
#define PCTRL_NPU_MAIN_TPC1_CRG_STATUS0             0x1344
#define PCTRL_NPU_MAIN_TPC1_CRG_STATUS1             0x1348
#define PCTRL_NPU_MAIN_TPC1_CRG_STATUS2             0x134c
#define PCTRL_NPU_MAIN_TPC2_CRG_STATUS0             0x1350
#define PCTRL_NPU_MAIN_TPC2_CRG_STATUS1             0x1354
#define PCTRL_NPU_MAIN_TPC2_CRG_STATUS2             0x1358
#define PCTRL_NPU_MAIN_TPC3_CRG_STATUS0             0x135c
#define PCTRL_NPU_MAIN_TPC3_CRG_STATUS1             0x1360
#define PCTRL_NPU_MAIN_TPC3_CRG_STATUS2             0x1364
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG0           0x1380
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG1           0x1384
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG2           0x1388
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG3           0x138c
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG4           0x1390
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG5           0x1394
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG6           0x1398
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG6_PERI_DMA_TPC_BWC_DYN_QOS4_W_TARGET_SHIFT      0
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG6_PERI_DMA_TPC_BWC_DYN_QOS4_W_TARGET_MASK       0x000000ff
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG6_PERI_DMA_TPC_BWC_DYN_QOS5_W_TARGET_SHIFT      8
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG6_PERI_DMA_TPC_BWC_DYN_QOS5_W_TARGET_MASK       0x0000ff00
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG6_PERI_DMA_TPC_BWC_DYN_QOS6_W_TARGET_SHIFT      16
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG6_PERI_DMA_TPC_BWC_DYN_QOS6_W_TARGET_MASK       0x00ff0000
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG6_PERI_DMA_TPC_BWC_DYN_QOS7_W_TARGET_SHIFT      24
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG6_PERI_DMA_TPC_BWC_DYN_QOS7_W_TARGET_MASK       0xff000000
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG7           0x139c
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG7_PERI_DMA_TPC_BWC_DYN_QOS0_W_TARGET_SHIFT      0
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG7_PERI_DMA_TPC_BWC_DYN_QOS0_W_TARGET_MASK       0x000000ff
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG7_PERI_DMA_TPC_BWC_DYN_QOS1_W_TARGET_SHIFT      8
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG7_PERI_DMA_TPC_BWC_DYN_QOS1_W_TARGET_MASK       0x0000ff00
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG7_PERI_DMA_TPC_BWC_DYN_QOS2_W_TARGET_SHIFT      16
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG7_PERI_DMA_TPC_BWC_DYN_QOS2_W_TARGET_MASK       0x00ff0000
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG7_PERI_DMA_TPC_BWC_DYN_QOS3_W_TARGET_SHIFT      24
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG7_PERI_DMA_TPC_BWC_DYN_QOS3_W_TARGET_MASK       0xff000000
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG8           0x13a0
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG8_PERI_DMA_TPC_BWC_DYN_QOS4_R_TARGET_SHIFT      0
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG8_PERI_DMA_TPC_BWC_DYN_QOS4_R_TARGET_MASK       0x000000ff
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG8_PERI_DMA_TPC_BWC_DYN_QOS5_R_TARGET_SHIFT      8
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG8_PERI_DMA_TPC_BWC_DYN_QOS5_R_TARGET_MASK       0x0000ff00
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG8_PERI_DMA_TPC_BWC_DYN_QOS6_R_TARGET_SHIFT      16
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG8_PERI_DMA_TPC_BWC_DYN_QOS6_R_TARGET_MASK       0x00ff0000
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG8_PERI_DMA_TPC_BWC_DYN_QOS7_R_TARGET_SHIFT      24
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG8_PERI_DMA_TPC_BWC_DYN_QOS7_R_TARGET_MASK       0xff000000
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG9           0x13a4
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG9_PERI_DMA_TPC_BWC_DYN_QOS0_R_TARGET_SHIFT      0
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG9_PERI_DMA_TPC_BWC_DYN_QOS0_R_TARGET_MASK       0x000000ff
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG9_PERI_DMA_TPC_BWC_DYN_QOS1_R_TARGET_SHIFT      8
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG9_PERI_DMA_TPC_BWC_DYN_QOS1_R_TARGET_MASK       0x0000ff00
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG9_PERI_DMA_TPC_BWC_DYN_QOS2_R_TARGET_SHIFT      16
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG9_PERI_DMA_TPC_BWC_DYN_QOS2_R_TARGET_MASK       0x00ff0000
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG9_PERI_DMA_TPC_BWC_DYN_QOS3_R_TARGET_SHIFT      24
#define PCTRL_PERI_DMA_TPC_NORMAL_CONFIG9_PERI_DMA_TPC_BWC_DYN_QOS3_R_TARGET_MASK       0xff000000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0              0x13a8
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_EN_SHIFT                            0
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_EN_MASK                             0x00000001
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_BW_COUNTER_EN_SHIFT                 1
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_BW_COUNTER_EN_MASK                  0x00000002
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_BWC_STATIC_R_EN_SHIFT               2
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_BWC_STATIC_R_EN_MASK                0x00000004
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_BWC_STATIC_W_EN_SHIFT               3
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_BWC_STATIC_W_EN_MASK                0x00000008
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_BWC_DYN_R_EN_SHIFT                  4
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_BWC_DYN_R_EN_MASK                   0x00000010
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_BWC_DYN_W_EN_SHIFT                  5
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_BWC_DYN_W_EN_MASK                   0x00000020
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_COUNT_OUTSTANDING_EN_SHIFT          6
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_COUNT_OUTSTANDING_EN_MASK           0x00000040
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_AFBC_ADDR_SCRAMBLE_EN_SHIFT         7
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_AFBC_ADDR_SCRAMBLE_EN_MASK          0x00000080
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_COUNT_AR_MAX_SHIFT                  8
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_COUNT_AR_MAX_MASK                   0x00007f00
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0__SHIFT                                           15
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0__MASK                                            0x00008000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_COUNT_AW_MAX_SHIFT                  16
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_PERI_DMA_TPC_COUNT_AW_MAX_MASK                   0x007f0000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_1_SHIFT                                          23
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG0_1_MASK                                           0xff800000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG1              0x13ac
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG1_PERI_DMA_TPC_BWC_STATIC_R_TARGET_SHIFT           0
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG1_PERI_DMA_TPC_BWC_STATIC_R_TARGET_MASK            0x000000ff
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG1_PERI_DMA_TPC_BWC_STATIC_W_TARGET_SHIFT           8
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG1_PERI_DMA_TPC_BWC_STATIC_W_TARGET_MASK            0x0000ff00
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG1__SHIFT                                           16
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG1__MASK                                            0xffff0000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2              0x13b0
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_0_SHIFT                         0
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_0_MASK                          0x0000000f
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_1_SHIFT                         4
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_1_MASK                          0x000000f0
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_2_SHIFT                         8
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_2_MASK                          0x00000f00
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_3_SHIFT                         12
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_3_MASK                          0x0000f000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_4_SHIFT                         16
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_4_MASK                          0x000f0000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_5_SHIFT                         20
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_5_MASK                          0x00f00000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_6_SHIFT                         24
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_6_MASK                          0x0f000000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_7_SHIFT                         28
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_7_MASK                          0x70000000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_SEL_SHIFT                       31
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG2_PERI_DMA_TPC_QOS_SEL_MASK                        0x80000000
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG3              0x13b4
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG3_PERI_DMA_TPC_BWC_R_SAT_SHIFT                     0
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG3_PERI_DMA_TPC_BWC_R_SAT_MASK                      0x0000ffff
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG3_PERI_DMA_TPC_BWC_W_SAT_SHIFT                     16
#define PCTRL_PERI_DMA_TPC_CRG_CONFIG3_PERI_DMA_TPC_BWC_W_SAT_MASK                      0xffff0000
#define PCTRL_PERI_DMANS_TPC_CRG_STATUS0            0x13b8
#define PCTRL_PERI_DMANS_TPC_CRG_STATUS1            0x13bc
#define PCTRL_PERI_DMANS_TPC_CRG_STATUS2            0x13c0
#define PCTRL_PERI_DMAS_TPC_CRG_STATUS0             0x13c4
#define PCTRL_PERI_DMAS_TPC_CRG_STATUS1             0x13c8
#define PCTRL_PERI_DMAS_TPC_CRG_STATUS2             0x13cc
#define PCTRL_MAIN_BUS_STATUS                       0x1400
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_TIMEOUT_EN_SHIFT                                 0
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_TIMEOUT_EN_MASK                                  0x00000001
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_TGT_TIMEOUT_SHIFT                                1
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_TGT_TIMEOUT_MASK                                 0x0000003e
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_INI_NO_PENDING_TRANS_SHIFT                       6
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_INI_NO_PENDING_TRANS_MASK                        0x0000ffc0
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_TGT_NO_PENDING_TRANS_SHIFT                       16
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_TGT_NO_PENDING_TRANS_MASK                        0x001f0000
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_CPU_INI_NO_PENDING_TRANS_SHIFT                   21
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_CPU_INI_NO_PENDING_TRANS_MASK                    0x01e00000
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_GPU_INI_NO_PENDING_TRANS_SHIFT                   25
#define PCTRL_MAIN_BUS_STATUS_MAIN_BUS_GPU_INI_NO_PENDING_TRANS_MASK                    0x1e000000
#define PCTRL_SYS0_BUS_STATUS                       0x1410
#define PCTRL_SYS0_BUS_STATUS_SYS0_BUS_TIMEOUT_EN_SHIFT                                 0
#define PCTRL_SYS0_BUS_STATUS_SYS0_BUS_TIMEOUT_EN_MASK                                  0x00000001
#define PCTRL_SYS0_BUS_STATUS_SYS0_BUS_TGT_TIMEOUT_SHIFT                                1
#define PCTRL_SYS0_BUS_STATUS_SYS0_BUS_TGT_TIMEOUT_MASK                                 0x00000006
#define PCTRL_SYS0_BUS_STATUS_SYS0_BUS_INI_NO_PENDING_TRANS_SHIFT                       3
#define PCTRL_SYS0_BUS_STATUS_SYS0_BUS_INI_NO_PENDING_TRANS_MASK                        0x00000038
#define PCTRL_SYS0_BUS_STATUS_SYS0_BUS_TGT_NO_PENDING_TRANS_SHIFT                       6
#define PCTRL_SYS0_BUS_STATUS_SYS0_BUS_TGT_NO_PENDING_TRANS_MASK                        0x000000c0
#define PCTRL_SYS1_BUS_STATUS                       0x1420
#define PCTRL_SYS1_BUS_STATUS_SYS1_BUS_TIMEOUT_EN_SHIFT                                 0
#define PCTRL_SYS1_BUS_STATUS_SYS1_BUS_TIMEOUT_EN_MASK                                  0x00000001
#define PCTRL_SYS1_BUS_STATUS_SYS1_BUS_TGT_TIMEOUT_SHIFT                                1
#define PCTRL_SYS1_BUS_STATUS_SYS1_BUS_TGT_TIMEOUT_MASK                                 0x00000006
#define PCTRL_SYS1_BUS_STATUS_SYS1_BUS_INI_NO_PENDING_TRANS_SHIFT                       3
#define PCTRL_SYS1_BUS_STATUS_SYS1_BUS_INI_NO_PENDING_TRANS_MASK                        0x00001ff8
#define PCTRL_SYS1_BUS_STATUS_SYS1_BUS_TGT_NO_PENDING_TRANS_SHIFT                       13
#define PCTRL_SYS1_BUS_STATUS_SYS1_BUS_TGT_NO_PENDING_TRANS_MASK                        0x00006000
#define PCTRL_SYSCFG_BUS_TIMEOUT_STATUS             0x1430
#define PCTRL_SYSCFG_BUS_TIMEOUT_STATUS_SYSCFG_BUS_TIMEOUT_EN_SHIFT                     0
#define PCTRL_SYSCFG_BUS_TIMEOUT_STATUS_SYSCFG_BUS_TIMEOUT_EN_MASK                      0x00000001
#define PCTRL_SYSCFG_BUS_TIMEOUT_STATUS_SYSCFG_BUS_TGT_TIMEOUT_SHIFT                    1
#define PCTRL_SYSCFG_BUS_TIMEOUT_STATUS_SYSCFG_BUS_TGT_TIMEOUT_MASK                     0x007ffffe
#define PCTRL_SYSCFG_BUS_TRANS_STATUS               0x1434
#define PCTRL_SYSCFG_BUS_TRANS_STATUS_SYSCFG_BUS_INI_NO_PENDING_TRANS_SHIFT             0
#define PCTRL_SYSCFG_BUS_TRANS_STATUS_SYSCFG_BUS_INI_NO_PENDING_TRANS_MASK              0x000003ff
#define PCTRL_SYSCFG_BUS_TRANS_STATUS_SYSCFG_BUS_TGT_NO_PENDING_TRANS_SHIFT             10
#define PCTRL_SYSCFG_BUS_TRANS_STATUS_SYSCFG_BUS_TGT_NO_PENDING_TRANS_MASK              0xfffffc00
#define PCTRL_PERICFG_BUS_STATUS                    0x1440
#define PCTRL_PERICFG_BUS_STATUS_PERICFG_BUS_TIMEOUT_EN_SHIFT                           0
#define PCTRL_PERICFG_BUS_STATUS_PERICFG_BUS_TIMEOUT_EN_MASK                            0x00000001
#define PCTRL_PERICFG_BUS_STATUS_PERICFG_BUS_INI_NO_PENDING_TRANS_SHIFT                 1
#define PCTRL_PERICFG_BUS_STATUS_PERICFG_BUS_INI_NO_PENDING_TRANS_MASK                  0x00000002
#define PCTRL_PERICFG_BUS_STATUS_PERICFG_BUS_TGT_NO_PENDING_TRANS_SHIFT                 2
#define PCTRL_PERICFG_BUS_STATUS_PERICFG_BUS_TGT_NO_PENDING_TRANS_MASK                  0x0000001c
#define PCTRL_PERICFG_BUS_STATUS_PERICFG_BUS_TGT_TIMEOUT_SHIFT                          5
#define PCTRL_PERICFG_BUS_STATUS_PERICFG_BUS_TGT_TIMEOUT_MASK                           0x000000e0
#define PCTRL_PERICFG_BUS_STATUS_PERICFG_BUS_NO_PENDING_TRANS_SHIFT                     14
#define PCTRL_PERICFG_BUS_STATUS_PERICFG_BUS_NO_PENDING_TRANS_MASK                      0x00004000
#define PCTRL_DMA_BUS_STATUS                        0x1450
#define PCTRL_DMA_BUS_STATUS_DMA_BUS_TIMEOUT_EN_SHIFT                                   0
#define PCTRL_DMA_BUS_STATUS_DMA_BUS_TIMEOUT_EN_MASK                                    0x00000001
#define PCTRL_DMA_BUS_STATUS_DMA_BUS_TGT_TIMEOUT_SHIFT                                  1
#define PCTRL_DMA_BUS_STATUS_DMA_BUS_TGT_TIMEOUT_MASK                                   0x0000001e
#define PCTRL_DMA_BUS_STATUS_DMA_BUS_INI_NO_PENDING_TRANS_SHIFT                         5
#define PCTRL_DMA_BUS_STATUS_DMA_BUS_INI_NO_PENDING_TRANS_MASK                          0x000003e0
#define PCTRL_DMA_BUS_STATUS_DMA_BUS_TGT_NO_PENDING_TRANS_SHIFT                         10
#define PCTRL_DMA_BUS_STATUS_DMA_BUS_TGT_NO_PENDING_TRANS_MASK                          0x00003c00
#define PCTRL_DMA_BUS_STATUS_DMA_BUS_NO_PENDING_TRANS_SHIFT                             14
#define PCTRL_DMA_BUS_STATUS_DMA_BUS_NO_PENDING_TRANS_MASK                              0x00004000
#define PCTRL_HSS1_BUS_STATUS                       0x1460
#define PCTRL_HSS1_BUS_STATUS_HSS1_BUS_TGT_TIMEOUT_SHIFT                                0
#define PCTRL_HSS1_BUS_STATUS_HSS1_BUS_TGT_TIMEOUT_MASK                                 0x0000001f
#define PCTRL_HSS1_BUS_STATUS_HSS1_BUS_INI_NO_PENDING_TRANS_SHIFT                       5
#define PCTRL_HSS1_BUS_STATUS_HSS1_BUS_INI_NO_PENDING_TRANS_MASK                        0x000003e0
#define PCTRL_HSS1_BUS_STATUS_HSS1_BUS_TGT_NO_PENDING_TRANS_SHIFT                       10
#define PCTRL_HSS1_BUS_STATUS_HSS1_BUS_TGT_NO_PENDING_TRANS_MASK                        0x00007c00
#define PCTRL_HSS2_BUS_STATUS                       0x1470
#define PCTRL_HSS2_BUS_STATUS_HSS2_BUS_TGT_TIMEOUT_SHIFT                                0
#define PCTRL_HSS2_BUS_STATUS_HSS2_BUS_TGT_TIMEOUT_MASK                                 0x0000007f
#define PCTRL_HSS2_BUS_STATUS_HSS2_BUS_INI_NO_PENDING_TRANS_SHIFT                       7
#define PCTRL_HSS2_BUS_STATUS_HSS2_BUS_INI_NO_PENDING_TRANS_MASK                        0x00000f80
#define PCTRL_HSS2_BUS_STATUS_HSS2_BUS_TGT_NO_PENDING_TRANS_SHIFT                       12
#define PCTRL_HSS2_BUS_STATUS_HSS2_BUS_TGT_NO_PENDING_TRANS_MASK                        0x0007f000
#define PCTRL_CPU_BUS_STATUS                        0x1480
#define PCTRL_CPU_BUS_STATUS_CPU_BUS_TGT_NO_PENDING_TRANS_SHIFT                         0
#define PCTRL_CPU_BUS_STATUS_CPU_BUS_TGT_NO_PENDING_TRANS_MASK                          0x0000007f
#define PCTRL_CPU_BUS_STATUS_CPU_BUS_INI_NO_PENDING_TRANS_SHIFT                         7
#define PCTRL_CPU_BUS_STATUS_CPU_BUS_INI_NO_PENDING_TRANS_MASK                          0x00000f80
#define PCTRL_CPU_BUS_STATUS_CPU_BUS_TGT_TIMEOUT_SHIFT                                  12
#define PCTRL_CPU_BUS_STATUS_CPU_BUS_TGT_TIMEOUT_MASK                                   0x0007f000
#define PCTRL_CPU_BUS_STATUS_CPU_BUS_NO_PENDING_TRANS_SHIFT                             19
#define PCTRL_CPU_BUS_STATUS_CPU_BUS_NO_PENDING_TRANS_MASK                              0x00080000
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU                 0x1484
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_CFG_IDLEREQ_SHIFT             0
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_CFG_IDLEREQ_MASK              0x00000001
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_CFG_IDLEACK_SHIFT             1
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_CFG_IDLEACK_MASK              0x00000002
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_CFG_IDLE_SHIFT                2
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_CFG_IDLE_MASK                 0x00000004
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_DATA_IDLEREQ_SHIFT            3
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_DATA_IDLEREQ_MASK             0x00000008
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_DATA_IDLEACK_SHIFT            4
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_DATA_IDLEACK_MASK             0x00000010
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_DATA_IDLE_SHIFT               5
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_XCTRLCPU_DATA_IDLE_MASK                0x00000020
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_CFG_IDLEREQ_SHIFT                  6
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_CFG_IDLEREQ_MASK                   0x00000040
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_CFG_IDLEACK_SHIFT                  7
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_CFG_IDLEACK_MASK                   0x00000080
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_CFG_IDLE_SHIFT                     8
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_CFG_IDLE_MASK                      0x00000100
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_DATA_IDLEREQ_SHIFT                 9
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_DATA_IDLEREQ_MASK                  0x00000200
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_DATA_IDLEACK_SHIFT                 10
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_DATA_IDLEACK_MASK                  0x00000400
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_DATA_IDLE_SHIFT                    11
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_GIC_DATA_IDLE_MASK                     0x00000800
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_UTILITY_IDLEREQ_SHIFT                  12
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_UTILITY_IDLEREQ_MASK                   0x00001000
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_UTILITY_IDLEACK_SHIFT                  13
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_UTILITY_IDLEACK_MASK                   0x00002000
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_UTILITY_IDLE_SHIFT                     14
#define PCTRL_CPU_BUS_PDOM_XCTRLCPU_CPU_BUS_PDOM_UTILITY_IDLE_MASK                      0x00004000
#define PCTRL_PERI_BROADCAST_BUS_STATUS             0x1490
#define PCTRL_PERI_BROADCAST_BUS_STATUS_PERI_BROADCAST_BUS_TIMEOUT_EN_SHIFT             0
#define PCTRL_PERI_BROADCAST_BUS_STATUS_PERI_BROADCAST_BUS_TIMEOUT_EN_MASK              0x00000001
#define PCTRL_PERI_BROADCAST_BUS_STATUS_PERI_BROADCAST_BUS_TGT_TIMEOUT_SHIFT            1
#define PCTRL_PERI_BROADCAST_BUS_STATUS_PERI_BROADCAST_BUS_TGT_TIMEOUT_MASK             0x000003fe
#define PCTRL_PERI_BROADCAST_BUS_STATUS_PERI_BROADCAST_BUS_INI_NO_PENDING_TRANS_SHIFT   10
#define PCTRL_PERI_BROADCAST_BUS_STATUS_PERI_BROADCAST_BUS_INI_NO_PENDING_TRANS_MASK    0x00000c00
#define PCTRL_PERI_BROADCAST_BUS_STATUS_PERI_BROADCAST_BUS_TGT_NO_PENDING_TRANS_SHIFT   12
#define PCTRL_PERI_BROADCAST_BUS_STATUS_PERI_BROADCAST_BUS_TGT_NO_PENDING_TRANS_MASK    0x001ff000
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR      0x1494
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_XCTRLDDR_IDLEREQ_SHIFT 0
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_XCTRLDDR_IDLEREQ_MASK  0x00000001
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_XCTRLDDR_IDLEACK_SHIFT 1
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_XCTRLDDR_IDLEACK_MASK  0x00000002
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_XCTRLDDR_IDLE_SHIFT   2
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_XCTRLDDR_IDLE_MASK    0x00000004
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_DDR_CFG_IDLEREQ_SHIFT 3
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_DDR_CFG_IDLEREQ_MASK  0x00000008
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_DDR_CFG_IDLEACK_SHIFT 4
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_DDR_CFG_IDLEACK_MASK  0x00000010
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_DDR_CFG_IDLE_SHIFT    5
#define PCTRL_PERI_BROADCAST_BUS_PDOM_XCTRLDDR_PERI_BROADCAST_BUS_DDR_CFG_IDLE_MASK     0x00000020
#define PCTRL_XRSE_M85_BUS_STATUS                   0x14a0
#define PCTRL_XRSE_M85_BUS_STATUS_XRSE_M85_BUS_TGT_TIMEOUT_SHIFT                        0
#define PCTRL_XRSE_M85_BUS_STATUS_XRSE_M85_BUS_TGT_TIMEOUT_MASK                         0x000000ff
#define PCTRL_XRSE_M85_BUS_STATUS_XRSE_M85_BUS_INI_NO_PENDING_TRANS_SHIFT               8
#define PCTRL_XRSE_M85_BUS_STATUS_XRSE_M85_BUS_INI_NO_PENDING_TRANS_MASK                0x00003f00
#define PCTRL_XRSE_M85_BUS_STATUS_XRSE_M85_BUS_TGT_NO_PENDING_TRANS_SHIFT               14
#define PCTRL_XRSE_M85_BUS_STATUS_XRSE_M85_BUS_TGT_NO_PENDING_TRANS_MASK                0x003fc000
#define PCTRL_HSS1_NDB_ASYNC_PWR                    0x1500
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_REQ_0_SHIFT                         0
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_REQ_0_MASK                          0x00000001
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_ACK_0_SHIFT                         1
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_ACK_0_MASK                          0x00000002
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_REQ_1_SHIFT                         2
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_REQ_1_MASK                          0x00000004
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_ACK_1_SHIFT                         3
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_ACK_1_MASK                          0x00000008
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_REQ_2_SHIFT                         4
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_REQ_2_MASK                          0x00000010
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_ACK_2_SHIFT                         5
#define PCTRL_HSS1_NDB_ASYNC_PWR_HSS1_NDB_ASYNC_PWR_ACK_2_MASK                          0x00000020
#define PCTRL_HSS2_NDB_ASYNC_PWR                    0x1504
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_REQ_0_SHIFT                         0
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_REQ_0_MASK                          0x00000001
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_ACK_0_SHIFT                         1
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_ACK_0_MASK                          0x00000002
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_REQ_1_SHIFT                         2
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_REQ_1_MASK                          0x00000004
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_ACK_1_SHIFT                         3
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_ACK_1_MASK                          0x00000008
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_REQ_2_SHIFT                         4
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_REQ_2_MASK                          0x00000010
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_ACK_2_SHIFT                         5
#define PCTRL_HSS2_NDB_ASYNC_PWR_HSS2_NDB_ASYNC_PWR_ACK_2_MASK                          0x00000020
#define PCTRL_LPDDR_RANK_TYPE                       0x1580
#define PCTRL_LPDDR_RANK_TYPE_LPDDR_RANK_SHIFT                                          0
#define PCTRL_LPDDR_RANK_TYPE_LPDDR_RANK_MASK                                           0x00000001
#define PCTRL_LPDDR_RANK_TYPE_LPDDR5_5X_SEL_SHIFT                                       1
#define PCTRL_LPDDR_RANK_TYPE_LPDDR5_5X_SEL_MASK                                        0x00000002
#define PCTRL_ISOLATION_GROUP3                      0x1600
#define PCTRL_ISOLATION_GROUP3_SC_CSI_ISO_EN_SHIFT                                      0
#define PCTRL_ISOLATION_GROUP3_SC_CSI_ISO_EN_MASK                                       0x00000001
#define PCTRL_ISOLATION_GROUP4                      0x1604
#define PCTRL_ISOLATION_GROUP4_SC_DSI_ISO_EN_SHIFT                                      0
#define PCTRL_ISOLATION_GROUP4_SC_DSI_ISO_EN_MASK                                       0x00000001
#define PCTRL_MEM_LGROUP2                           0x1680
#define PCTRL_MEM_LGROUP2_SC_CSI_MEM_SD_SHIFT                                           0
#define PCTRL_MEM_LGROUP2_SC_CSI_MEM_SD_MASK                                            0x00000001
#define PCTRL_MEM_LPSTATUS2                         0x1684
#define PCTRL_MEM_LPSTATUS2_CSI_MEM_SD_ACK_SHIFT                                        0
#define PCTRL_MEM_LPSTATUS2_CSI_MEM_SD_ACK_MASK                                         0x00000001
#define PCTRL_MEM_LGROUP3                           0x1688
#define PCTRL_MEM_LGROUP3_SC_DSI0_MEM_SD_SHIFT                                          0
#define PCTRL_MEM_LGROUP3_SC_DSI0_MEM_SD_MASK                                           0x00000001
#define PCTRL_MEM_LPSTATUS3                         0x168c
#define PCTRL_MEM_LPSTATUS3_DSI0_MEM_SD_ACK_SHIFT                                       0
#define PCTRL_MEM_LPSTATUS3_DSI0_MEM_SD_ACK_MASK                                        0x00000001
#define PCTRL_MEM_LGROUP4                           0x1690
#define PCTRL_MEM_LGROUP4_SC_DSI1_MEM_SD_SHIFT                                          0
#define PCTRL_MEM_LGROUP4_SC_DSI1_MEM_SD_MASK                                           0x00000001
#define PCTRL_MEM_LPSTATUS4                         0x1694
#define PCTRL_MEM_LPSTATUS4_DSI1_MEM_SD_ACK_SHIFT                                       0
#define PCTRL_MEM_LPSTATUS4_DSI1_MEM_SD_ACK_MASK                                        0x00000001
#define PCTRL_TIMER_S_DEBUG                         0x1800
#define PCTRL_TIMER_S_DEBUG_TIMER_S_PAUSE_SHIFT                                         0
#define PCTRL_TIMER_S_DEBUG_TIMER_S_PAUSE_MASK                                          0x0000000f
#define PCTRL_TIMER_S_DEBUG_TIMER_S_EN_SHIFT                                            4
#define PCTRL_TIMER_S_DEBUG_TIMER_S_EN_MASK                                             0x000000f0
#define PCTRL_WDT_S_DEBUG                           0x1804
#define PCTRL_WDT_S_DEBUG_WDT_S_PAUSE_SHIFT                                             0
#define PCTRL_WDT_S_DEBUG_WDT_S_PAUSE_MASK                                              0x00000003
#define PCTRL_SPI0_DEBUG                            0x1880
#define PCTRL_SPI0_DEBUG_SPI0_DEBUG_SHIFT                                               0
#define PCTRL_SPI0_DEBUG_SPI0_DEBUG_MASK                                                0x0000000f
#define PCTRL_SPI0_DEBUG_SPI0_SS_IN_N_SHIFT                                             4
#define PCTRL_SPI0_DEBUG_SPI0_SS_IN_N_MASK                                              0x00000010
#define PCTRL_SPI1_DEBUG                            0x1884
#define PCTRL_SPI1_DEBUG_SPI1_DEBUG_SHIFT                                               0
#define PCTRL_SPI1_DEBUG_SPI1_DEBUG_MASK                                                0x0000000f
#define PCTRL_SPI1_DEBUG_SPI1_SS_IN_N_SHIFT                                             4
#define PCTRL_SPI1_DEBUG_SPI1_SS_IN_N_MASK                                              0x00000010
#define PCTRL_SPI2_DEBUG                            0x1888
#define PCTRL_SPI2_DEBUG_SPI2_DEBUG_SHIFT                                               0
#define PCTRL_SPI2_DEBUG_SPI2_DEBUG_MASK                                                0x0000000f
#define PCTRL_SPI2_DEBUG_SPI2_SS_IN_N_SHIFT                                             4
#define PCTRL_SPI2_DEBUG_SPI2_SS_IN_N_MASK                                              0x00000010
#define PCTRL_GPIO_ACPU_MASK_S_PERI                 0x1900
#define PCTRL_GPIO_ACPU_MASK_S_PERI_GPIO_ACPU_MASK_S_PERI_SHIFT                         0
#define PCTRL_GPIO_ACPU_MASK_S_PERI_GPIO_ACPU_MASK_S_PERI_MASK                          0x000000ff
#define PCTRL_GPIO_LPCORE_MASK_S_PERI               0x1904
#define PCTRL_GPIO_LPCORE_MASK_S_PERI_GPIO_LPCORE_MASK_S_PERI_SHIFT                     0
#define PCTRL_GPIO_LPCORE_MASK_S_PERI_GPIO_LPCORE_MASK_S_PERI_MASK                      0x000000ff
#define PCTRL_GPIO_SH_MASK_S_PERI                   0x1908
#define PCTRL_GPIO_SH_MASK_S_PERI_GPIO_SH_MASK_S_PERI_SHIFT                             0
#define PCTRL_GPIO_SH_MASK_S_PERI_GPIO_SH_MASK_S_PERI_MASK                              0x000000ff
#define PCTRL_REGFILE_CGBYPASS                      0x190c
#define PCTRL_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                                   0
#define PCTRL_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                                    0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int rov_0      : 32;
    } reg;
}pctrl_version_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_pcie0_mem_sd :  1;
        unsigned int reserved_0      : 15;
        unsigned int _bm_            : 16;
    } reg;
}pctrl_mem_lgroup0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pcie0_mem_sd_ack :  1;
        unsigned int reserved_0       : 31;
    } reg;
}pctrl_mem_lpstatus0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_pcie1_mem_sd :  1;
        unsigned int reserved_0      : 15;
        unsigned int _bm_            : 16;
    } reg;
}pctrl_mem_lgroup1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pcie1_mem_sd_ack :  1;
        unsigned int reserved_0       : 31;
    } reg;
}pctrl_mem_lpstatus1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmans_debug_ch_num_i :  5;
        unsigned int reserved_0           : 27;
    } reg;
}pctrl_dmans_debug_ch_num_i_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmans_debug0 : 32;
    } reg;
}pctrl_dmans_debug0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmans_debug1 : 32;
    } reg;
}pctrl_dmans_debug1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmans_debug2 : 32;
    } reg;
}pctrl_dmans_debug2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmans_debug3 : 32;
    } reg;
}pctrl_dmans_debug3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmans_debug4 : 32;
    } reg;
}pctrl_dmans_debug4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmans_debug5 : 32;
    } reg;
}pctrl_dmans_debug5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmans_lp0  : 32;
    } reg;
}pctrl_dmans_lp0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmans_lp1  : 32;
    } reg;
}pctrl_dmans_lp1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc0_raw_st : 32;
    } reg;
}pctrl_intr_exc0_raw_st_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc0_mask_st : 32;
    } reg;
}pctrl_intr_exc0_mask_st_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc0_mask_l : 16;
        unsigned int _bm_             : 16;
    } reg;
}pctrl_intr_exc0_mask_l_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc0_mask_h : 16;
        unsigned int _bm_             : 16;
    } reg;
}pctrl_intr_exc0_mask_h_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc2_raw_st : 32;
    } reg;
}pctrl_intr_exc2_raw_st_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc2_mask_st : 32;
    } reg;
}pctrl_intr_exc2_mask_st_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc2_mask_l : 16;
        unsigned int _bm_             : 16;
    } reg;
}pctrl_intr_exc2_mask_l_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc2_mask_h : 16;
        unsigned int _bm_             : 16;
    } reg;
}pctrl_intr_exc2_mask_h_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int idle_flag_perfstat_npu0 :  1;
        unsigned int idle_flag_perfstat_npu1 :  1;
        unsigned int idle_flag_perfstat_npu2 :  1;
        unsigned int idle_flag_perfstat_npu3 :  1;
        unsigned int idle_flag_perfstat_sys1 :  1;
        unsigned int reserved_0              : 27;
    } reg;
}pctrl_idle_flag_perfstat_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ipc0_gt_bypass :  1;
        unsigned int ipc1_gt_bypass :  1;
        unsigned int ipc2_gt_bypass :  1;
        unsigned int reserved_0     : 13;
        unsigned int _bm_           : 16;
    } reg;
}pctrl_ipc_gt_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ddr_iso_en :  1;
        unsigned int reserved_0    : 15;
        unsigned int _bm_          : 16;
    } reg;
}pctrl_isolation_group0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_media1_iso_en :  1;
        unsigned int reserved_0       : 15;
        unsigned int _bm_             : 16;
    } reg;
}pctrl_isolation_group1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_media2_iso_en :  1;
        unsigned int reserved_0       : 15;
        unsigned int _bm_             : 16;
    } reg;
}pctrl_isolation_group2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_cpu_top_mem_sd   :  1;
        unsigned int sc_cpu_top_mem_poff :  2;
        unsigned int sc_cpu_top_mem_ds   :  1;
        unsigned int reserved_0          : 12;
        unsigned int _bm_                : 16;
    } reg;
}pctrl_mem_lgroup5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cpu_top_mem_sd_ack :  1;
        unsigned int cpu_top_mem_ds_ack :  1;
        unsigned int reserved_0         : 30;
    } reg;
}pctrl_mem_lpstatus5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ddr_mem_sd   :  1;
        unsigned int sc_ddr_mem_poff :  2;
        unsigned int sc_ddr_mem_ds   :  1;
        unsigned int reserved_0      : 12;
        unsigned int _bm_            : 16;
    } reg;
}pctrl_mem_lgroup6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddra_mem_sd_ack :  1;
        unsigned int ddrb_mem_sd_ack :  1;
        unsigned int ddrc_mem_sd_ack :  1;
        unsigned int ddrd_mem_sd_ack :  1;
        unsigned int ddra_mem_ds_ack :  1;
        unsigned int ddrb_mem_ds_ack :  1;
        unsigned int ddrc_mem_ds_ack :  1;
        unsigned int ddrd_mem_ds_ack :  1;
        unsigned int reserved_0      : 24;
    } reg;
}pctrl_mem_lpstatus6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_mem_ctrl_bus_low : 32;
    } reg;
}pctrl_mem_ctrl_bus_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_mem_ctrl_bus_mid : 32;
    } reg;
}pctrl_mem_ctrl_bus_mid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_mem_ctrl_bus_high : 32;
    } reg;
}pctrl_mem_ctrl_bus_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmas_debug_ch_num_i :  5;
        unsigned int reserved_0          : 27;
    } reg;
}pctrl_dmas_debug_ch_num_i_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmas_debug0 : 32;
    } reg;
}pctrl_dmas_debug0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmas_debug1 : 32;
    } reg;
}pctrl_dmas_debug1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmas_debug2 : 32;
    } reg;
}pctrl_dmas_debug2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmas_debug3 : 32;
    } reg;
}pctrl_dmas_debug3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmas_debug4 : 32;
    } reg;
}pctrl_dmas_debug4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmas_debug5 : 32;
    } reg;
}pctrl_dmas_debug5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmas_lp0   : 32;
    } reg;
}pctrl_dmas_lp0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dmas_lp1   : 32;
    } reg;
}pctrl_dmas_lp1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pericfg_nsfw0_perror    :  1;
        unsigned int reserved_0              :  3;
        unsigned int pericfg_nsfw0_clear     :  1;
        unsigned int reserved_1              :  3;
        unsigned int pericfg_nsfw0_fail_pmid :  8;
        unsigned int reserved_2              : 16;
    } reg;
}pctrl_pericfg_nsfw0_fail_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pericfg_nsfw0_fail_paddr : 32;
    } reg;
}pctrl_pericfg_nsfw0_fail_addr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pericfg_nsfw1_perror    :  1;
        unsigned int reserved_0              :  3;
        unsigned int pericfg_nsfw1_clear     :  1;
        unsigned int reserved_1              :  3;
        unsigned int pericfg_nsfw1_fail_pmid :  8;
        unsigned int reserved_2              : 16;
    } reg;
}pctrl_pericfg_nsfw1_fail_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pericfg_nsfw1_fail_paddr : 32;
    } reg;
}pctrl_pericfg_nsfw1_fail_addr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dma_nsfw_perror    :  1;
        unsigned int reserved_0         :  3;
        unsigned int dma_nsfw_clear     :  1;
        unsigned int reserved_1         :  3;
        unsigned int dma_nsfw_fail_pmid :  8;
        unsigned int reserved_2         : 16;
    } reg;
}pctrl_dma_nsfw_fail_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dma_nsfw_fail_paddr : 32;
    } reg;
}pctrl_dma_nsfw_fail_addr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pmpu_bypass :  1;
        unsigned int reserved_0  : 31;
    } reg;
}pctrl_pmpu_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int testpin_sel_cfg :  4;
        unsigned int reserved_0      : 28;
    } reg;
}pctrl_testpin_sel_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pericfg_testpin_sel_cfg :  4;
        unsigned int reserved_0              : 28;
    } reg;
}pctrl_pericfg_testpin_sel_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tpc_addr_masked0 : 27;
        unsigned int reserved_0       :  5;
    } reg;
}pctrl_tpc_addr_masked0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tpc_addr_masked1 : 27;
        unsigned int reserved_0       :  5;
    } reg;
}pctrl_tpc_addr_masked1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intl_mode  :  2;
        unsigned int reserved_0 : 30;
    } reg;
}pctrl_intl_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int broadcast_sel_r :  4;
        unsigned int broadcast_en_r  :  1;
        unsigned int reserved_0      : 11;
        unsigned int _bm_            : 16;
    } reg;
}pctrl_peri_broadcast_bus_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int broadcast2ddra_adb400_pwrq_permit_deny_sar :  1;
        unsigned int broadcast2ddra_adb400_pwrqactives          :  1;
        unsigned int broadcast2ddra_adb400_pwrqdenys            :  1;
        unsigned int broadcast2ddra_adb400_pwrqacceptns         :  1;
        unsigned int broadcast2ddra_adb400_pwrqreqns            :  1;
        unsigned int main2ddra_adb400_pwrq_permit_deny_sar      :  1;
        unsigned int main2ddra_adb400_pwrqactives               :  1;
        unsigned int main2ddra_adb400_pwrqdenys                 :  1;
        unsigned int main2ddra_adb400_pwrqacceptns              :  1;
        unsigned int main2ddra_adb400_pwrqreqns                 :  1;
        unsigned int reserved_0                                 : 22;
    } reg;
}pctrl_ddr_subsys_wrap_a_adb_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int broadcast2ddrb_adb400_pwrq_permit_deny_sar :  1;
        unsigned int broadcast2ddrb_adb400_pwrqactives          :  1;
        unsigned int broadcast2ddrb_adb400_pwrqdenys            :  1;
        unsigned int broadcast2ddrb_adb400_pwrqacceptns         :  1;
        unsigned int broadcast2ddrb_adb400_pwrqreqns            :  1;
        unsigned int main2ddrb_adb400_pwrq_permit_deny_sar      :  1;
        unsigned int main2ddrb_adb400_pwrqactives               :  1;
        unsigned int main2ddrb_adb400_pwrqdenys                 :  1;
        unsigned int main2ddrb_adb400_pwrqacceptns              :  1;
        unsigned int main2ddrb_adb400_pwrqreqns                 :  1;
        unsigned int reserved_0                                 : 22;
    } reg;
}pctrl_ddr_subsys_wrap_b_adb_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int broadcast2ddrc_adb400_pwrq_permit_deny_sar :  1;
        unsigned int broadcast2ddrc_adb400_pwrqactives          :  1;
        unsigned int broadcast2ddrc_adb400_pwrqdenys            :  1;
        unsigned int broadcast2ddrc_adb400_pwrqacceptns         :  1;
        unsigned int broadcast2ddrc_adb400_pwrqreqns            :  1;
        unsigned int main2ddrc_adb400_pwrq_permit_deny_sar      :  1;
        unsigned int main2ddrc_adb400_pwrqactives               :  1;
        unsigned int main2ddrc_adb400_pwrqdenys                 :  1;
        unsigned int main2ddrc_adb400_pwrqacceptns              :  1;
        unsigned int main2ddrc_adb400_pwrqreqns                 :  1;
        unsigned int reserved_0                                 : 22;
    } reg;
}pctrl_ddr_subsys_wrap_c_adb_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int broadcast2ddrd_adb400_pwrq_permit_deny_sar :  1;
        unsigned int broadcast2ddrd_adb400_pwrqactives          :  1;
        unsigned int broadcast2ddrd_adb400_pwrqdenys            :  1;
        unsigned int broadcast2ddrd_adb400_pwrqacceptns         :  1;
        unsigned int broadcast2ddrd_adb400_pwrqreqns            :  1;
        unsigned int main2ddrd_adb400_pwrq_permit_deny_sar      :  1;
        unsigned int main2ddrd_adb400_pwrqactives               :  1;
        unsigned int main2ddrd_adb400_pwrqdenys                 :  1;
        unsigned int main2ddrd_adb400_pwrqacceptns              :  1;
        unsigned int main2ddrd_adb400_pwrqreqns                 :  1;
        unsigned int reserved_0                                 : 22;
    } reg;
}pctrl_ddr_subsys_wrap_d_adb_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int xctrl_cpu_pctrl_status : 32;
    } reg;
}pctrl_xctrl_cpu_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int xctrl_ddr_pctrl_status : 32;
    } reg;
}pctrl_xctrl_ddr_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc1_raw_st : 32;
    } reg;
}pctrl_intr_exc1_raw_st_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc1_mask_st : 32;
    } reg;
}pctrl_intr_exc1_mask_st_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc1_mask_l : 16;
        unsigned int _bm_             : 16;
    } reg;
}pctrl_intr_exc1_mask_l_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_exc1_mask_h : 16;
        unsigned int _bm_             : 16;
    } reg;
}pctrl_intr_exc1_mask_h_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int timer_ns_pause :  4;
        unsigned int timer_ns_en    :  4;
        unsigned int reserved_0     :  8;
        unsigned int _bm_           : 16;
    } reg;
}pctrl_timer_ns_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int wdt_ns_pause :  2;
        unsigned int reserved_0   : 14;
        unsigned int _bm_         : 16;
    } reg;
}pctrl_wdt_ns_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c0_debug : 22;
        unsigned int reserved_0 : 10;
    } reg;
}pctrl_i2c0_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c1_debug : 22;
        unsigned int reserved_0 : 10;
    } reg;
}pctrl_i2c1_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c2_debug : 22;
        unsigned int reserved_0 : 10;
    } reg;
}pctrl_i2c2_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c3_debug : 22;
        unsigned int reserved_0 : 10;
    } reg;
}pctrl_i2c3_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c4_debug : 22;
        unsigned int reserved_0 : 10;
    } reg;
}pctrl_i2c4_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c5_debug : 22;
        unsigned int reserved_0 : 10;
    } reg;
}pctrl_i2c5_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c6_debug : 22;
        unsigned int reserved_0 : 10;
    } reg;
}pctrl_i2c6_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c9_debug : 22;
        unsigned int reserved_0 : 10;
    } reg;
}pctrl_i2c9_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c10_debug : 22;
        unsigned int reserved_0  : 10;
    } reg;
}pctrl_i2c10_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c11_debug : 22;
        unsigned int reserved_0  : 10;
    } reg;
}pctrl_i2c11_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c12_debug : 22;
        unsigned int reserved_0  : 10;
    } reg;
}pctrl_i2c12_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c13_debug : 22;
        unsigned int reserved_0  : 10;
    } reg;
}pctrl_i2c13_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c20_debug : 22;
        unsigned int reserved_0  : 10;
    } reg;
}pctrl_i2c20_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int spi4_debug   :  4;
        unsigned int spi4_ss_in_n :  1;
        unsigned int reserved_0   : 27;
    } reg;
}pctrl_spi4_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int spi5_debug   :  4;
        unsigned int spi5_ss_in_n :  1;
        unsigned int reserved_0   : 27;
    } reg;
}pctrl_spi5_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int spi6_debug   :  4;
        unsigned int spi6_ss_in_n :  1;
        unsigned int reserved_0   : 27;
    } reg;
}pctrl_spi6_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int uart3_debug : 32;
    } reg;
}pctrl_uart3_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int uart6_debug : 32;
    } reg;
}pctrl_uart6_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i3c0_debug_low : 32;
    } reg;
}pctrl_i3c0_debug_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i3c0_debug_high : 32;
    } reg;
}pctrl_i3c0_debug_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_ns_peri_0 : 16;
        unsigned int _bm_                     : 16;
    } reg;
}pctrl_gpio_acpu_mask_ns_peri_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_ns_peri_1 : 16;
        unsigned int _bm_                     : 16;
    } reg;
}pctrl_gpio_acpu_mask_ns_peri_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_ns_peri_2 : 16;
        unsigned int _bm_                     : 16;
    } reg;
}pctrl_gpio_acpu_mask_ns_peri_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_ns_peri_3 : 16;
        unsigned int _bm_                     : 16;
    } reg;
}pctrl_gpio_acpu_mask_ns_peri_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_ns_peri_4 : 16;
        unsigned int _bm_                     : 16;
    } reg;
}pctrl_gpio_acpu_mask_ns_peri_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_ns_peri_5 : 16;
        unsigned int _bm_                     : 16;
    } reg;
}pctrl_gpio_acpu_mask_ns_peri_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_ns_peri_6 : 16;
        unsigned int _bm_                     : 16;
    } reg;
}pctrl_gpio_acpu_mask_ns_peri_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_ns_peri_7 : 16;
        unsigned int _bm_                     : 16;
    } reg;
}pctrl_gpio_acpu_mask_ns_peri_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_ns_peri_8 : 16;
        unsigned int _bm_                     : 16;
    } reg;
}pctrl_gpio_acpu_mask_ns_peri_8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_ns_peri_9 :  4;
        unsigned int reserved_0               : 12;
        unsigned int _bm_                     : 16;
    } reg;
}pctrl_gpio_acpu_mask_ns_peri_9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_ns_peri_0 : 16;
        unsigned int _bm_                       : 16;
    } reg;
}pctrl_gpio_lpcore_mask_ns_peri_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_ns_peri_1 : 16;
        unsigned int _bm_                       : 16;
    } reg;
}pctrl_gpio_lpcore_mask_ns_peri_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_ns_peri_2 : 16;
        unsigned int _bm_                       : 16;
    } reg;
}pctrl_gpio_lpcore_mask_ns_peri_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_ns_peri_3 : 16;
        unsigned int _bm_                       : 16;
    } reg;
}pctrl_gpio_lpcore_mask_ns_peri_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_ns_peri_4 : 16;
        unsigned int _bm_                       : 16;
    } reg;
}pctrl_gpio_lpcore_mask_ns_peri_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_ns_peri_5 : 16;
        unsigned int _bm_                       : 16;
    } reg;
}pctrl_gpio_lpcore_mask_ns_peri_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_ns_peri_6 : 16;
        unsigned int _bm_                       : 16;
    } reg;
}pctrl_gpio_lpcore_mask_ns_peri_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_ns_peri_7 : 16;
        unsigned int _bm_                       : 16;
    } reg;
}pctrl_gpio_lpcore_mask_ns_peri_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_ns_peri_8 : 16;
        unsigned int _bm_                       : 16;
    } reg;
}pctrl_gpio_lpcore_mask_ns_peri_8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_ns_peri_9 :  4;
        unsigned int reserved_0                 : 12;
        unsigned int _bm_                       : 16;
    } reg;
}pctrl_gpio_lpcore_mask_ns_peri_9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_ns_peri_0 : 16;
        unsigned int _bm_                   : 16;
    } reg;
}pctrl_gpio_sh_mask_ns_peri_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_ns_peri_1 : 16;
        unsigned int _bm_                   : 16;
    } reg;
}pctrl_gpio_sh_mask_ns_peri_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_ns_peri_2 : 16;
        unsigned int _bm_                   : 16;
    } reg;
}pctrl_gpio_sh_mask_ns_peri_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_ns_peri_3 : 16;
        unsigned int _bm_                   : 16;
    } reg;
}pctrl_gpio_sh_mask_ns_peri_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_ns_peri_4 : 16;
        unsigned int _bm_                   : 16;
    } reg;
}pctrl_gpio_sh_mask_ns_peri_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_ns_peri_5 : 16;
        unsigned int _bm_                   : 16;
    } reg;
}pctrl_gpio_sh_mask_ns_peri_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_ns_peri_6 : 16;
        unsigned int _bm_                   : 16;
    } reg;
}pctrl_gpio_sh_mask_ns_peri_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_ns_peri_7 : 16;
        unsigned int _bm_                   : 16;
    } reg;
}pctrl_gpio_sh_mask_ns_peri_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_ns_peri_8 : 16;
        unsigned int _bm_                   : 16;
    } reg;
}pctrl_gpio_sh_mask_ns_peri_8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_ns_peri_9 :  4;
        unsigned int reserved_0             : 12;
        unsigned int _bm_                   : 16;
    } reg;
}pctrl_gpio_sh_mask_ns_peri_9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_0   :  4;
        unsigned int fc_qos_1   :  4;
        unsigned int fc_qos_2   :  4;
        unsigned int fc_qos_3   :  4;
        unsigned int fc_qos_4   :  4;
        unsigned int fc_qos_5   :  4;
        unsigned int fc_qos_6   :  4;
        unsigned int fc_qos_7   :  4;
    } reg;
}pctrl_fc_qos_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int isp_flux_req_r :  4;
        unsigned int isp_flux_req_w :  4;
        unsigned int dpu_flux_req_r :  4;
        unsigned int dpu_flux_req_w :  4;
        unsigned int reserved_0     : 16;
    } reg;
}pctrl_flux_req_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr0_wl_qos_allow_r         :  8;
        unsigned int ddr0_wl_qos_allow_w         :  8;
        unsigned int ddr0_dvfs_going_qos_allow_r :  8;
        unsigned int ddr0_dvfs_going_qos_allow_w :  8;
    } reg;
}pctrl_ddr0_qos_allow_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr1_wl_qos_allow_r         :  8;
        unsigned int ddr1_wl_qos_allow_w         :  8;
        unsigned int ddr1_dvfs_going_qos_allow_r :  8;
        unsigned int ddr1_dvfs_going_qos_allow_w :  8;
    } reg;
}pctrl_ddr1_qos_allow_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr2_wl_qos_allow_r         :  8;
        unsigned int ddr2_wl_qos_allow_w         :  8;
        unsigned int ddr2_dvfs_going_qos_allow_r :  8;
        unsigned int ddr2_dvfs_going_qos_allow_w :  8;
    } reg;
}pctrl_ddr2_qos_allow_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr3_wl_qos_allow_r         :  8;
        unsigned int ddr3_wl_qos_allow_w         :  8;
        unsigned int ddr3_dvfs_going_qos_allow_r :  8;
        unsigned int ddr3_dvfs_going_qos_allow_w :  8;
    } reg;
}pctrl_ddr3_qos_allow_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_qos_allow_r :  8;
        unsigned int sys_qos_allow_w :  8;
        unsigned int reserved_0      : 16;
    } reg;
}pctrl_sys_qos_allow_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int flow_ctrl_en_l : 16;
        unsigned int _bm_           : 16;
    } reg;
}pctrl_flow_ctrl_en_l_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int flow_ctrl_en_h : 16;
        unsigned int _bm_           : 16;
    } reg;
}pctrl_flow_ctrl_en_h_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_0_allow_cnt_r : 32;
    } reg;
}pctrl_fc_qos_0_allow_cnt_r_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_1_allow_cnt_r : 32;
    } reg;
}pctrl_fc_qos_1_allow_cnt_r_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_2_allow_cnt_r : 32;
    } reg;
}pctrl_fc_qos_2_allow_cnt_r_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_3_allow_cnt_r : 32;
    } reg;
}pctrl_fc_qos_3_allow_cnt_r_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_4_allow_cnt_r : 32;
    } reg;
}pctrl_fc_qos_4_allow_cnt_r_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_5_allow_cnt_r : 32;
    } reg;
}pctrl_fc_qos_5_allow_cnt_r_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_6_allow_cnt_r : 32;
    } reg;
}pctrl_fc_qos_6_allow_cnt_r_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_7_allow_cnt_r : 32;
    } reg;
}pctrl_fc_qos_7_allow_cnt_r_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_0_allow_cnt_w : 32;
    } reg;
}pctrl_fc_qos_0_allow_cnt_w_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_1_allow_cnt_w : 32;
    } reg;
}pctrl_fc_qos_1_allow_cnt_w_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_2_allow_cnt_w : 32;
    } reg;
}pctrl_fc_qos_2_allow_cnt_w_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_3_allow_cnt_w : 32;
    } reg;
}pctrl_fc_qos_3_allow_cnt_w_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_4_allow_cnt_w : 32;
    } reg;
}pctrl_fc_qos_4_allow_cnt_w_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_5_allow_cnt_w : 32;
    } reg;
}pctrl_fc_qos_5_allow_cnt_w_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_6_allow_cnt_w : 32;
    } reg;
}pctrl_fc_qos_6_allow_cnt_w_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fc_qos_7_allow_cnt_w : 32;
    } reg;
}pctrl_fc_qos_7_allow_cnt_w_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_normal_crg_config0 : 32;
    } reg;
}pctrl_sys1_main_tpc_normal_crg_config0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_normal_crg_config1 : 32;
    } reg;
}pctrl_sys1_main_tpc_normal_crg_config1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_normal_crg_config2 : 32;
    } reg;
}pctrl_sys1_main_tpc_normal_crg_config2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_normal_crg_config3 : 32;
    } reg;
}pctrl_sys1_main_tpc_normal_crg_config3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_normal_crg_config4 : 32;
    } reg;
}pctrl_sys1_main_tpc_normal_crg_config4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_normal_crg_config5 : 32;
    } reg;
}pctrl_sys1_main_tpc_normal_crg_config5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_bwc_dyn_qos4_w_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos5_w_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos6_w_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos7_w_target :  8;
    } reg;
}pctrl_sys1_main_tpc_normal_crg_config6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_bwc_dyn_qos0_w_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos1_w_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos2_w_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos3_w_target :  8;
    } reg;
}pctrl_sys1_main_tpc_normal_crg_config7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_bwc_dyn_qos4_r_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos5_r_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos6_r_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos7_r_target :  8;
    } reg;
}pctrl_sys1_main_tpc_normal_crg_config8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_bwc_dyn_qos0_r_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos1_r_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos2_r_target :  8;
        unsigned int sys1_main_tpc_bwc_dyn_qos3_r_target :  8;
    } reg;
}pctrl_sys1_main_tpc_normal_crg_config9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_en                    :  1;
        unsigned int sys1_main_tpc_bw_counter_en         :  1;
        unsigned int sys1_main_tpc_bwc_static_r_en       :  1;
        unsigned int sys1_main_tpc_bwc_static_w_en       :  1;
        unsigned int sys1_main_tpc_bwc_dyn_r_en          :  1;
        unsigned int sys1_main_tpc_bwc_dyn_w_en          :  1;
        unsigned int sys1_main_tpc_count_outstanding_en  :  1;
        unsigned int sys1_main_tpc_afbc_addr_scramble_en :  1;
        unsigned int sys1_main_tpc_count_ar_max          :  7;
        unsigned int rov_0                               :  1;
        unsigned int sys1_main_tpc_count_aw_max          :  7;
        unsigned int rov_1                               :  9;
    } reg;
}pctrl_sys1_main_tpc_crg_config0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_bwc_static_r_target :  8;
        unsigned int sys1_main_tpc_bwc_static_w_target :  8;
        unsigned int rov_0                             : 16;
    } reg;
}pctrl_sys1_main_tpc_crg_config1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_qos_0   :  4;
        unsigned int sys1_main_tpc_qos_1   :  4;
        unsigned int sys1_main_tpc_qos_2   :  4;
        unsigned int sys1_main_tpc_qos_3   :  4;
        unsigned int sys1_main_tpc_qos_4   :  4;
        unsigned int sys1_main_tpc_qos_5   :  4;
        unsigned int sys1_main_tpc_qos_6   :  4;
        unsigned int sys1_main_tpc_qos_7   :  3;
        unsigned int sys1_main_tpc_qos_sel :  1;
    } reg;
}pctrl_sys1_main_tpc_crg_config2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_bwc_r_sat : 16;
        unsigned int sys1_main_tpc_bwc_w_sat : 16;
    } reg;
}pctrl_sys1_main_tpc_crg_config3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_crg_status0 : 32;
    } reg;
}pctrl_sys1_main_tpc_crg_status0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_crg_status1 : 32;
    } reg;
}pctrl_sys1_main_tpc_crg_status1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_main_tpc_crg_status2 : 32;
    } reg;
}pctrl_sys1_main_tpc_crg_status2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_normal_crg_config0 : 32;
    } reg;
}pctrl_npu_main_tpc_normal_crg_config0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_normal_crg_config1 : 32;
    } reg;
}pctrl_npu_main_tpc_normal_crg_config1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_normal_crg_config2 : 32;
    } reg;
}pctrl_npu_main_tpc_normal_crg_config2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_normal_crg_config3 : 32;
    } reg;
}pctrl_npu_main_tpc_normal_crg_config3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_normal_crg_config4 : 32;
    } reg;
}pctrl_npu_main_tpc_normal_crg_config4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_normal_crg_config5 : 32;
    } reg;
}pctrl_npu_main_tpc_normal_crg_config5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_bwc_dyn_qos4_w_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos5_w_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos6_w_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos7_w_target :  8;
    } reg;
}pctrl_npu_main_tpc_normal_crg_config6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_bwc_dyn_qos0_w_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos1_w_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos2_w_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos3_w_target :  8;
    } reg;
}pctrl_npu_main_tpc_normal_crg_config7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_bwc_dyn_qos4_r_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos5_r_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos6_r_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos7_r_target :  8;
    } reg;
}pctrl_npu_main_tpc_normal_crg_config8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_bwc_dyn_qos0_r_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos1_r_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos2_r_target :  8;
        unsigned int npu_main_tpc_bwc_dyn_qos3_r_target :  8;
    } reg;
}pctrl_npu_main_tpc_normal_crg_config9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_en                    :  1;
        unsigned int npu_main_tpc_bw_counter_en         :  1;
        unsigned int npu_main_tpc_bwc_static_r_en       :  1;
        unsigned int npu_main_tpc_bwc_static_w_en       :  1;
        unsigned int npu_main_tpc_bwc_dyn_r_en          :  1;
        unsigned int npu_main_tpc_bwc_dyn_w_en          :  1;
        unsigned int npu_main_tpc_count_outstanding_en  :  1;
        unsigned int npu_main_tpc_afbc_addr_scramble_en :  1;
        unsigned int npu_main_tpc_count_ar_max          :  7;
        unsigned int rov_0                              :  1;
        unsigned int npu_main_tpc_count_aw_max          :  7;
        unsigned int rov_1                              :  9;
    } reg;
}pctrl_npu_main_tpc_crg_config0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_bwc_static_r_target :  8;
        unsigned int npu_main_tpc_bwc_static_w_target :  8;
        unsigned int rov_0                            : 16;
    } reg;
}pctrl_npu_main_tpc_crg_config1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_qos_0   :  4;
        unsigned int npu_main_tpc_qos_1   :  4;
        unsigned int npu_main_tpc_qos_2   :  4;
        unsigned int npu_main_tpc_qos_3   :  4;
        unsigned int npu_main_tpc_qos_4   :  4;
        unsigned int npu_main_tpc_qos_5   :  4;
        unsigned int npu_main_tpc_qos_6   :  4;
        unsigned int npu_main_tpc_qos_7   :  3;
        unsigned int npu_main_tpc_qos_sel :  1;
    } reg;
}pctrl_npu_main_tpc_crg_config2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc_bwc_r_sat : 16;
        unsigned int npu_main_tpc_bwc_w_sat : 16;
    } reg;
}pctrl_npu_main_tpc_crg_config3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc0_crg_status0 : 32;
    } reg;
}pctrl_npu_main_tpc0_crg_status0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc0_crg_status1 : 32;
    } reg;
}pctrl_npu_main_tpc0_crg_status1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc0_crg_status2 : 32;
    } reg;
}pctrl_npu_main_tpc0_crg_status2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc1_crg_status0 : 32;
    } reg;
}pctrl_npu_main_tpc1_crg_status0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc1_crg_status1 : 32;
    } reg;
}pctrl_npu_main_tpc1_crg_status1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc1_crg_status2 : 32;
    } reg;
}pctrl_npu_main_tpc1_crg_status2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc2_crg_status0 : 32;
    } reg;
}pctrl_npu_main_tpc2_crg_status0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc2_crg_status1 : 32;
    } reg;
}pctrl_npu_main_tpc2_crg_status1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc2_crg_status2 : 32;
    } reg;
}pctrl_npu_main_tpc2_crg_status2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc3_crg_status0 : 32;
    } reg;
}pctrl_npu_main_tpc3_crg_status0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc3_crg_status1 : 32;
    } reg;
}pctrl_npu_main_tpc3_crg_status1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_main_tpc3_crg_status2 : 32;
    } reg;
}pctrl_npu_main_tpc3_crg_status2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_normal_config0 : 32;
    } reg;
}pctrl_peri_dma_tpc_normal_config0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_normal_config1 : 32;
    } reg;
}pctrl_peri_dma_tpc_normal_config1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_normal_config2 : 32;
    } reg;
}pctrl_peri_dma_tpc_normal_config2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_normal_config3 : 32;
    } reg;
}pctrl_peri_dma_tpc_normal_config3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_normal_config4 : 32;
    } reg;
}pctrl_peri_dma_tpc_normal_config4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_normal_config5 : 32;
    } reg;
}pctrl_peri_dma_tpc_normal_config5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_bwc_dyn_qos4_w_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos5_w_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos6_w_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos7_w_target :  8;
    } reg;
}pctrl_peri_dma_tpc_normal_config6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_bwc_dyn_qos0_w_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos1_w_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos2_w_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos3_w_target :  8;
    } reg;
}pctrl_peri_dma_tpc_normal_config7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_bwc_dyn_qos4_r_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos5_r_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos6_r_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos7_r_target :  8;
    } reg;
}pctrl_peri_dma_tpc_normal_config8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_bwc_dyn_qos0_r_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos1_r_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos2_r_target :  8;
        unsigned int peri_dma_tpc_bwc_dyn_qos3_r_target :  8;
    } reg;
}pctrl_peri_dma_tpc_normal_config9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_en                    :  1;
        unsigned int peri_dma_tpc_bw_counter_en         :  1;
        unsigned int peri_dma_tpc_bwc_static_r_en       :  1;
        unsigned int peri_dma_tpc_bwc_static_w_en       :  1;
        unsigned int peri_dma_tpc_bwc_dyn_r_en          :  1;
        unsigned int peri_dma_tpc_bwc_dyn_w_en          :  1;
        unsigned int peri_dma_tpc_count_outstanding_en  :  1;
        unsigned int peri_dma_tpc_afbc_addr_scramble_en :  1;
        unsigned int peri_dma_tpc_count_ar_max          :  7;
        unsigned int rov_0                              :  1;
        unsigned int peri_dma_tpc_count_aw_max          :  7;
        unsigned int rov_1                              :  9;
    } reg;
}pctrl_peri_dma_tpc_crg_config0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_bwc_static_r_target :  8;
        unsigned int peri_dma_tpc_bwc_static_w_target :  8;
        unsigned int rov_0                            : 16;
    } reg;
}pctrl_peri_dma_tpc_crg_config1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_qos_0   :  4;
        unsigned int peri_dma_tpc_qos_1   :  4;
        unsigned int peri_dma_tpc_qos_2   :  4;
        unsigned int peri_dma_tpc_qos_3   :  4;
        unsigned int peri_dma_tpc_qos_4   :  4;
        unsigned int peri_dma_tpc_qos_5   :  4;
        unsigned int peri_dma_tpc_qos_6   :  4;
        unsigned int peri_dma_tpc_qos_7   :  3;
        unsigned int peri_dma_tpc_qos_sel :  1;
    } reg;
}pctrl_peri_dma_tpc_crg_config2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dma_tpc_bwc_r_sat : 16;
        unsigned int peri_dma_tpc_bwc_w_sat : 16;
    } reg;
}pctrl_peri_dma_tpc_crg_config3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dmans_tpc_crg_status0 : 32;
    } reg;
}pctrl_peri_dmans_tpc_crg_status0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dmans_tpc_crg_status1 : 32;
    } reg;
}pctrl_peri_dmans_tpc_crg_status1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dmans_tpc_crg_status2 : 32;
    } reg;
}pctrl_peri_dmans_tpc_crg_status2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dmas_tpc_crg_status0 : 32;
    } reg;
}pctrl_peri_dmas_tpc_crg_status0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dmas_tpc_crg_status1 : 32;
    } reg;
}pctrl_peri_dmas_tpc_crg_status1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_dmas_tpc_crg_status2 : 32;
    } reg;
}pctrl_peri_dmas_tpc_crg_status2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int main_bus_timeout_en               :  1;
        unsigned int main_bus_tgt_timeout              :  5;
        unsigned int main_bus_ini_no_pending_trans     : 10;
        unsigned int main_bus_tgt_no_pending_trans     :  5;
        unsigned int main_bus_cpu_ini_no_pending_trans :  4;
        unsigned int main_bus_gpu_ini_no_pending_trans :  4;
        unsigned int reserved_0                        :  3;
    } reg;
}pctrl_main_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys0_bus_timeout_en           :  1;
        unsigned int sys0_bus_tgt_timeout          :  2;
        unsigned int sys0_bus_ini_no_pending_trans :  3;
        unsigned int sys0_bus_tgt_no_pending_trans :  2;
        unsigned int reserved_0                    : 24;
    } reg;
}pctrl_sys0_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_bus_timeout_en           :  1;
        unsigned int sys1_bus_tgt_timeout          :  2;
        unsigned int sys1_bus_ini_no_pending_trans : 10;
        unsigned int sys1_bus_tgt_no_pending_trans :  2;
        unsigned int reserved_0                    : 17;
    } reg;
}pctrl_sys1_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int syscfg_bus_timeout_en  :  1;
        unsigned int syscfg_bus_tgt_timeout : 22;
        unsigned int reserved_0             :  9;
    } reg;
}pctrl_syscfg_bus_timeout_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int syscfg_bus_ini_no_pending_trans : 10;
        unsigned int syscfg_bus_tgt_no_pending_trans : 22;
    } reg;
}pctrl_syscfg_bus_trans_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pericfg_bus_timeout_en           :  1;
        unsigned int pericfg_bus_ini_no_pending_trans :  1;
        unsigned int pericfg_bus_tgt_no_pending_trans :  3;
        unsigned int pericfg_bus_tgt_timeout          :  3;
        unsigned int reserved_0                       :  6;
        unsigned int pericfg_bus_no_pending_trans     :  1;
        unsigned int reserved_1                       : 17;
    } reg;
}pctrl_pericfg_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dma_bus_timeout_en           :  1;
        unsigned int dma_bus_tgt_timeout          :  4;
        unsigned int dma_bus_ini_no_pending_trans :  5;
        unsigned int dma_bus_tgt_no_pending_trans :  4;
        unsigned int dma_bus_no_pending_trans     :  1;
        unsigned int reserved_0                   : 17;
    } reg;
}pctrl_dma_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hss1_bus_tgt_timeout          :  5;
        unsigned int hss1_bus_ini_no_pending_trans :  5;
        unsigned int hss1_bus_tgt_no_pending_trans :  5;
        unsigned int reserved_0                    : 17;
    } reg;
}pctrl_hss1_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hss2_bus_tgt_timeout          :  7;
        unsigned int hss2_bus_ini_no_pending_trans :  5;
        unsigned int hss2_bus_tgt_no_pending_trans :  7;
        unsigned int reserved_0                    : 13;
    } reg;
}pctrl_hss2_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cpu_bus_tgt_no_pending_trans :  7;
        unsigned int cpu_bus_ini_no_pending_trans :  5;
        unsigned int cpu_bus_tgt_timeout          :  7;
        unsigned int cpu_bus_no_pending_trans     :  1;
        unsigned int reserved_0                   : 12;
    } reg;
}pctrl_cpu_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cpu_bus_pdom_xctrlcpu_cfg_idlereq  :  1;
        unsigned int cpu_bus_pdom_xctrlcpu_cfg_idleack  :  1;
        unsigned int cpu_bus_pdom_xctrlcpu_cfg_idle     :  1;
        unsigned int cpu_bus_pdom_xctrlcpu_data_idlereq :  1;
        unsigned int cpu_bus_pdom_xctrlcpu_data_idleack :  1;
        unsigned int cpu_bus_pdom_xctrlcpu_data_idle    :  1;
        unsigned int cpu_bus_pdom_gic_cfg_idlereq       :  1;
        unsigned int cpu_bus_pdom_gic_cfg_idleack       :  1;
        unsigned int cpu_bus_pdom_gic_cfg_idle          :  1;
        unsigned int cpu_bus_pdom_gic_data_idlereq      :  1;
        unsigned int cpu_bus_pdom_gic_data_idleack      :  1;
        unsigned int cpu_bus_pdom_gic_data_idle         :  1;
        unsigned int cpu_bus_pdom_utility_idlereq       :  1;
        unsigned int cpu_bus_pdom_utility_idleack       :  1;
        unsigned int cpu_bus_pdom_utility_idle          :  1;
        unsigned int reserved_0                         :  1;
        unsigned int _bm_                               : 16;
    } reg;
}pctrl_cpu_bus_pdom_xctrlcpu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_broadcast_bus_timeout_en           :  1;
        unsigned int peri_broadcast_bus_tgt_timeout          :  9;
        unsigned int peri_broadcast_bus_ini_no_pending_trans :  2;
        unsigned int peri_broadcast_bus_tgt_no_pending_trans :  9;
        unsigned int reserved_0                              : 11;
    } reg;
}pctrl_peri_broadcast_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_broadcast_bus_xctrlddr_idlereq :  1;
        unsigned int peri_broadcast_bus_xctrlddr_idleack :  1;
        unsigned int peri_broadcast_bus_xctrlddr_idle    :  1;
        unsigned int peri_broadcast_bus_ddr_cfg_idlereq  :  1;
        unsigned int peri_broadcast_bus_ddr_cfg_idleack  :  1;
        unsigned int peri_broadcast_bus_ddr_cfg_idle     :  1;
        unsigned int reserved_0                          : 10;
        unsigned int _bm_                                : 16;
    } reg;
}pctrl_peri_broadcast_bus_pdom_xctrlddr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int xrse_m85_bus_tgt_timeout          :  8;
        unsigned int xrse_m85_bus_ini_no_pending_trans :  6;
        unsigned int xrse_m85_bus_tgt_no_pending_trans :  8;
        unsigned int reserved_0                        : 10;
    } reg;
}pctrl_xrse_m85_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hss1_ndb_async_pwr_req_0 :  1;
        unsigned int hss1_ndb_async_pwr_ack_0 :  1;
        unsigned int hss1_ndb_async_pwr_req_1 :  1;
        unsigned int hss1_ndb_async_pwr_ack_1 :  1;
        unsigned int hss1_ndb_async_pwr_req_2 :  1;
        unsigned int hss1_ndb_async_pwr_ack_2 :  1;
        unsigned int reserved_0               : 26;
    } reg;
}pctrl_hss1_ndb_async_pwr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hss2_ndb_async_pwr_req_0 :  1;
        unsigned int hss2_ndb_async_pwr_ack_0 :  1;
        unsigned int hss2_ndb_async_pwr_req_1 :  1;
        unsigned int hss2_ndb_async_pwr_ack_1 :  1;
        unsigned int hss2_ndb_async_pwr_req_2 :  1;
        unsigned int hss2_ndb_async_pwr_ack_2 :  1;
        unsigned int reserved_0               : 26;
    } reg;
}pctrl_hss2_ndb_async_pwr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lpddr_rank    :  1;
        unsigned int lpddr5_5x_sel :  1;
        unsigned int reserved_0    : 30;
    } reg;
}pctrl_lpddr_rank_type_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_csi_iso_en :  1;
        unsigned int reserved_0    : 15;
        unsigned int _bm_          : 16;
    } reg;
}pctrl_isolation_group3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_dsi_iso_en :  1;
        unsigned int reserved_0    : 15;
        unsigned int _bm_          : 16;
    } reg;
}pctrl_isolation_group4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_csi_mem_sd :  1;
        unsigned int reserved_0    : 15;
        unsigned int _bm_          : 16;
    } reg;
}pctrl_mem_lgroup2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int csi_mem_sd_ack :  1;
        unsigned int reserved_0     : 31;
    } reg;
}pctrl_mem_lpstatus2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_dsi0_mem_sd :  1;
        unsigned int reserved_0     : 15;
        unsigned int _bm_           : 16;
    } reg;
}pctrl_mem_lgroup3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dsi0_mem_sd_ack :  1;
        unsigned int reserved_0      : 31;
    } reg;
}pctrl_mem_lpstatus3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_dsi1_mem_sd :  1;
        unsigned int reserved_0     : 15;
        unsigned int _bm_           : 16;
    } reg;
}pctrl_mem_lgroup4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dsi1_mem_sd_ack :  1;
        unsigned int reserved_0      : 31;
    } reg;
}pctrl_mem_lpstatus4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int timer_s_pause :  4;
        unsigned int timer_s_en    :  4;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}pctrl_timer_s_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int wdt_s_pause :  2;
        unsigned int reserved_0  : 14;
        unsigned int _bm_        : 16;
    } reg;
}pctrl_wdt_s_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int spi0_debug   :  4;
        unsigned int spi0_ss_in_n :  1;
        unsigned int reserved_0   : 27;
    } reg;
}pctrl_spi0_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int spi1_debug   :  4;
        unsigned int spi1_ss_in_n :  1;
        unsigned int reserved_0   : 27;
    } reg;
}pctrl_spi1_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int spi2_debug   :  4;
        unsigned int spi2_ss_in_n :  1;
        unsigned int reserved_0   : 27;
    } reg;
}pctrl_spi2_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_acpu_mask_s_peri :  8;
        unsigned int reserved_0            :  8;
        unsigned int _bm_                  : 16;
    } reg;
}pctrl_gpio_acpu_mask_s_peri_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_lpcore_mask_s_peri :  8;
        unsigned int reserved_0              :  8;
        unsigned int _bm_                    : 16;
    } reg;
}pctrl_gpio_lpcore_mask_s_peri_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpio_sh_mask_s_peri :  8;
        unsigned int reserved_0          :  8;
        unsigned int _bm_                : 16;
    } reg;
}pctrl_gpio_sh_mask_s_peri_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}pctrl_regfile_cgbypass_t;

#endif
