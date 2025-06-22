// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef NPU_CRG_REGIF_H
#define NPU_CRG_REGIF_H

#define NPU_CRG_NPU_CORE_PLL_VOTE_EN                0x0130
#define NPU_CRG_NPU_CORE_PLL_VOTE_EN_NPU_CORE_PLL_VOTE_EN_SHIFT                     0
#define NPU_CRG_NPU_CORE_PLL_VOTE_EN_NPU_CORE_PLL_VOTE_EN_MASK                      0x000000ff
#define NPU_CRG_NPU_CORE_PLL_LOCK_STATE             0x0150
#define NPU_CRG_NPU_CORE_PLL_LOCK_STATE_NPU_CORE_PLL_LOCK_SHIFT                     0
#define NPU_CRG_NPU_CORE_PLL_LOCK_STATE_NPU_CORE_PLL_LOCK_MASK                      0x00000001
#define NPU_CRG_NPU_CORE_PLL_INTR_MASK              0x0154
#define NPU_CRG_NPU_CORE_PLL_INTR_MASK_INTR_MASK_PLL_INI_ERR_CORE_SHIFT             0
#define NPU_CRG_NPU_CORE_PLL_INTR_MASK_INTR_MASK_PLL_INI_ERR_CORE_MASK              0x00000001
#define NPU_CRG_NPU_CORE_PLL_INTR_MASK_INTR_MASK_PLL_UNLOCK_CORE_SHIFT              1
#define NPU_CRG_NPU_CORE_PLL_INTR_MASK_INTR_MASK_PLL_UNLOCK_CORE_MASK               0x00000002
#define NPU_CRG_NPU_CORE_PLL_INTR_MASK_INTR_MASK_VOTE_REQ_ERR_CORE_SHIFT            2
#define NPU_CRG_NPU_CORE_PLL_INTR_MASK_INTR_MASK_VOTE_REQ_ERR_CORE_MASK             0x00000004
#define NPU_CRG_NPU_CORE_PLL_INTR_CLEAR             0x0158
#define NPU_CRG_NPU_CORE_PLL_INTR_CLEAR_INTR_CLEAR_PLL_INI_ERR_CORE_SHIFT           0
#define NPU_CRG_NPU_CORE_PLL_INTR_CLEAR_INTR_CLEAR_PLL_INI_ERR_CORE_MASK            0x00000001
#define NPU_CRG_NPU_CORE_PLL_INTR_CLEAR_INTR_CLEAR_PLL_UNLOCK_CORE_SHIFT            1
#define NPU_CRG_NPU_CORE_PLL_INTR_CLEAR_INTR_CLEAR_PLL_UNLOCK_CORE_MASK             0x00000002
#define NPU_CRG_NPU_CORE_PLL_INTR_CLEAR_INTR_CLEAR_VOTE_REQ_ERR_CORE_SHIFT          2
#define NPU_CRG_NPU_CORE_PLL_INTR_CLEAR_INTR_CLEAR_VOTE_REQ_ERR_CORE_MASK           0x00000004
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS            0x015c
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_INTR_PLL_INI_ERR_STATUS_CORE_SHIFT         0
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_INTR_PLL_INI_ERR_STATUS_CORE_MASK          0x00000001
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_INTR_PLL_UNLOCK_STATUS_CORE_SHIFT          1
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_INTR_PLL_UNLOCK_STATUS_CORE_MASK           0x00000002
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_INTR_VOTE_REQ_ERR_STATUS_CORE_SHIFT        2
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_INTR_VOTE_REQ_ERR_STATUS_CORE_MASK         0x00000004
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_MSK        0x0160
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_MSK_INTR_PLL_INI_ERR_STATUS_CORE_MSK_SHIFT 0
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_MSK_INTR_PLL_INI_ERR_STATUS_CORE_MSK_MASK  0x00000001
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_MSK_INTR_PLL_UNLOCK_STATUS_CORE_MSK_SHIFT  1
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_MSK_INTR_PLL_UNLOCK_STATUS_CORE_MSK_MASK   0x00000002
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_MSK_INTR_VOTE_REQ_ERR_STATUS_CORE_MSK_SHIFT 2
#define NPU_CRG_NPU_CORE_PLL_INTR_STATUS_MSK_INTR_VOTE_REQ_ERR_STATUS_CORE_MSK_MASK  0x00000004
#define NPU_CRG_NPU_VDSP_PLL_VOTE_EN                0x0230
#define NPU_CRG_NPU_VDSP_PLL_VOTE_EN_NPU_VDSP_PLL_VOTE_EN_SHIFT                     0
#define NPU_CRG_NPU_VDSP_PLL_VOTE_EN_NPU_VDSP_PLL_VOTE_EN_MASK                      0x000000ff
#define NPU_CRG_NPU_VDSP_PLL_LOCK_STATE             0x0250
#define NPU_CRG_NPU_VDSP_PLL_LOCK_STATE_NPU_VDSP_PLL_LOCK_SHIFT                     0
#define NPU_CRG_NPU_VDSP_PLL_LOCK_STATE_NPU_VDSP_PLL_LOCK_MASK                      0x00000001
#define NPU_CRG_NPU_VDSP_PLL_INTR_MASK              0x0254
#define NPU_CRG_NPU_VDSP_PLL_INTR_MASK_INTR_MASK_PLL_INI_ERR_VDSP_SHIFT             0
#define NPU_CRG_NPU_VDSP_PLL_INTR_MASK_INTR_MASK_PLL_INI_ERR_VDSP_MASK              0x00000001
#define NPU_CRG_NPU_VDSP_PLL_INTR_MASK_INTR_MASK_PLL_UNLOCK_VDSP_SHIFT              1
#define NPU_CRG_NPU_VDSP_PLL_INTR_MASK_INTR_MASK_PLL_UNLOCK_VDSP_MASK               0x00000002
#define NPU_CRG_NPU_VDSP_PLL_INTR_MASK_INTR_MASK_VOTE_REQ_ERR_VDSP_SHIFT            2
#define NPU_CRG_NPU_VDSP_PLL_INTR_MASK_INTR_MASK_VOTE_REQ_ERR_VDSP_MASK             0x00000004
#define NPU_CRG_NPU_VDSP_PLL_INTR_CLEAR             0x0258
#define NPU_CRG_NPU_VDSP_PLL_INTR_CLEAR_INTR_CLEAR_PLL_INI_ERR_VDSP_SHIFT           0
#define NPU_CRG_NPU_VDSP_PLL_INTR_CLEAR_INTR_CLEAR_PLL_INI_ERR_VDSP_MASK            0x00000001
#define NPU_CRG_NPU_VDSP_PLL_INTR_CLEAR_INTR_CLEAR_PLL_UNLOCK_VDSP_SHIFT            1
#define NPU_CRG_NPU_VDSP_PLL_INTR_CLEAR_INTR_CLEAR_PLL_UNLOCK_VDSP_MASK             0x00000002
#define NPU_CRG_NPU_VDSP_PLL_INTR_CLEAR_INTR_CLEAR_VOTE_REQ_ERR_VDSP_SHIFT          2
#define NPU_CRG_NPU_VDSP_PLL_INTR_CLEAR_INTR_CLEAR_VOTE_REQ_ERR_VDSP_MASK           0x00000004
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS            0x025c
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_INTR_PLL_INI_ERR_STATUS_VDSP_SHIFT         0
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_INTR_PLL_INI_ERR_STATUS_VDSP_MASK          0x00000001
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_INTR_PLL_UNLOCK_STATUS_VDSP_SHIFT          1
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_INTR_PLL_UNLOCK_STATUS_VDSP_MASK           0x00000002
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_INTR_VOTE_REQ_ERR_STATUS_VDSP_SHIFT        2
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_INTR_VOTE_REQ_ERR_STATUS_VDSP_MASK         0x00000004
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_MSK        0x0260
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_MSK_INTR_PLL_INI_ERR_STATUS_VDSP_MSK_SHIFT 0
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_MSK_INTR_PLL_INI_ERR_STATUS_VDSP_MSK_MASK  0x00000001
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_MSK_INTR_PLL_UNLOCK_STATUS_VDSP_MSK_SHIFT  1
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_MSK_INTR_PLL_UNLOCK_STATUS_VDSP_MSK_MASK   0x00000002
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_MSK_INTR_VOTE_REQ_ERR_STATUS_VDSP_MSK_SHIFT 2
#define NPU_CRG_NPU_VDSP_PLL_INTR_STATUS_MSK_INTR_VOTE_REQ_ERR_STATUS_VDSP_MSK_MASK  0x00000004
#define NPU_CRG_AUTOFSBYPASS0                       0x0300
#define NPU_CRG_AUTOFSBYPASS0_AUTOFS_BYPASS_NPU_BUS_DATA_SHIFT                      0
#define NPU_CRG_AUTOFSBYPASS0_AUTOFS_BYPASS_NPU_BUS_DATA_MASK                       0x0000ffff
#define NPU_CRG_AUTOFSBYPASS1                       0x0304
#define NPU_CRG_AUTOFSBYPASS1_AUTOFS_BYPASS_NPU_MCU_CORE_SHIFT                      0
#define NPU_CRG_AUTOFSBYPASS1_AUTOFS_BYPASS_NPU_MCU_CORE_MASK                       0x0000ffff
#define NPU_CRG_AUTOFSBYPASS2                       0x0308
#define NPU_CRG_AUTOFSBYPASS2_AUTOFS_BYPASS_NPU_BUS_CFG_SHIFT                       0
#define NPU_CRG_AUTOFSBYPASS2_AUTOFS_BYPASS_NPU_BUS_CFG_MASK                        0x0000ffff
#define NPU_CRG_AUTOFSBYPASS3                       0x030c
#define NPU_CRG_AUTOFSBYPASS3_AUTOFS_BYPASS_NPU_VIP_CORE_SHIFT                      0
#define NPU_CRG_AUTOFSBYPASS3_AUTOFS_BYPASS_NPU_VIP_CORE_MASK                       0x0000ffff
#define NPU_CRG_AUTOFSBYPASS4                       0x0310
#define NPU_CRG_AUTOFSBYPASS4_AUTOFS_BYPASS_NPU_VDSP_CORE_SHIFT                     0
#define NPU_CRG_AUTOFSBYPASS4_AUTOFS_BYPASS_NPU_VDSP_CORE_MASK                      0x0000ffff
#define NPU_CRG_CLOCK_CG_0_W1S                      0x0800
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_TBU7_SHIFT                               0
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_TBU6_SHIFT                               1
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_TBU5_SHIFT                               2
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_TBU4_SHIFT                               3
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_TBU3_SHIFT                               4
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_TBU2_SHIFT                               5
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_TBU1_SHIFT                               6
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_TBU0_SHIFT                               7
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_VDSP_APB_SHIFT                            10
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_M85_APB_SHIFT                             11
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_M85_ATB_SHIFT                             12
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_M85_TS_SHIFT                              13
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_VDSP_TS_SHIFT                             14
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_VDSP_ATB_SHIFT                            15
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_DEBUG_ATB_SHIFT                           16
#define NPU_CRG_CLOCK_CG_0_W1S_GT_PCLK_NPU_DEBUG_SHIFT                              17
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_CCD_SHIFT                                 18
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_DMMU1_SHIFT                              19
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_DMMU0_SHIFT                              20
#define NPU_CRG_CLOCK_CG_0_W1S_GT_ACLK_NPU_TCU_TPC_SHIFT                            21
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_CORE3_SHIFT                               26
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_CORE5_SHIFT                               27
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_CORE2_SHIFT                               28
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_CORE4_SHIFT                               29
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_CORE1_SHIFT                               30
#define NPU_CRG_CLOCK_CG_0_W1S_GT_CLK_NPU_CORE0_SHIFT                               31
#define NPU_CRG_CLOCK_CG_0_W1C                      0x0804
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_TBU7_SHIFT                               0
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_TBU6_SHIFT                               1
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_TBU5_SHIFT                               2
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_TBU4_SHIFT                               3
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_TBU3_SHIFT                               4
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_TBU2_SHIFT                               5
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_TBU1_SHIFT                               6
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_TBU0_SHIFT                               7
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_VDSP_APB_SHIFT                            10
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_M85_APB_SHIFT                             11
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_M85_ATB_SHIFT                             12
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_M85_TS_SHIFT                              13
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_VDSP_TS_SHIFT                             14
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_VDSP_ATB_SHIFT                            15
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_DEBUG_ATB_SHIFT                           16
#define NPU_CRG_CLOCK_CG_0_W1C_GT_PCLK_NPU_DEBUG_SHIFT                              17
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_CCD_SHIFT                                 18
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_DMMU1_SHIFT                              19
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_DMMU0_SHIFT                              20
#define NPU_CRG_CLOCK_CG_0_W1C_GT_ACLK_NPU_TCU_TPC_SHIFT                            21
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_CORE3_SHIFT                               26
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_CORE5_SHIFT                               27
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_CORE2_SHIFT                               28
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_CORE4_SHIFT                               29
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_CORE1_SHIFT                               30
#define NPU_CRG_CLOCK_CG_0_W1C_GT_CLK_NPU_CORE0_SHIFT                               31
#define NPU_CRG_CLOCK_CG_0_RO                       0x0808
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU7_SHIFT                                0
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU7_MASK                                 0x00000001
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU6_SHIFT                                1
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU6_MASK                                 0x00000002
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU5_SHIFT                                2
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU5_MASK                                 0x00000004
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU4_SHIFT                                3
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU4_MASK                                 0x00000008
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU3_SHIFT                                4
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU3_MASK                                 0x00000010
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU2_SHIFT                                5
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU2_MASK                                 0x00000020
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU1_SHIFT                                6
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU1_MASK                                 0x00000040
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU0_SHIFT                                7
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TBU0_MASK                                 0x00000080
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_VDSP_APB_SHIFT                             10
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_VDSP_APB_MASK                              0x00000400
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_M85_APB_SHIFT                              11
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_M85_APB_MASK                               0x00000800
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_M85_ATB_SHIFT                              12
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_M85_ATB_MASK                               0x00001000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_M85_TS_SHIFT                               13
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_M85_TS_MASK                                0x00002000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_VDSP_TS_SHIFT                              14
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_VDSP_TS_MASK                               0x00004000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_VDSP_ATB_SHIFT                             15
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_VDSP_ATB_MASK                              0x00008000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_DEBUG_ATB_SHIFT                            16
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_DEBUG_ATB_MASK                             0x00010000
#define NPU_CRG_CLOCK_CG_0_RO_GT_PCLK_NPU_DEBUG_SHIFT                               17
#define NPU_CRG_CLOCK_CG_0_RO_GT_PCLK_NPU_DEBUG_MASK                                0x00020000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CCD_SHIFT                                  18
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CCD_MASK                                   0x00040000
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_DMMU1_SHIFT                               19
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_DMMU1_MASK                                0x00080000
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_DMMU0_SHIFT                               20
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_DMMU0_MASK                                0x00100000
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TCU_TPC_SHIFT                             21
#define NPU_CRG_CLOCK_CG_0_RO_GT_ACLK_NPU_TCU_TPC_MASK                              0x00200000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE3_SHIFT                                26
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE3_MASK                                 0x04000000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE5_SHIFT                                27
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE5_MASK                                 0x08000000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE2_SHIFT                                28
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE2_MASK                                 0x10000000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE4_SHIFT                                29
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE4_MASK                                 0x20000000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE1_SHIFT                                30
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE1_MASK                                 0x40000000
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE0_SHIFT                                31
#define NPU_CRG_CLOCK_CG_0_RO_GT_CLK_NPU_CORE0_MASK                                 0x80000000
#define NPU_CRG_CLOCK_ST_0                          0x080c
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU7_SHIFT                                   0
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU7_MASK                                    0x00000001
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU6_SHIFT                                   1
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU6_MASK                                    0x00000002
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU5_SHIFT                                   2
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU5_MASK                                    0x00000004
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU4_SHIFT                                   3
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU4_MASK                                    0x00000008
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU3_SHIFT                                   4
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU3_MASK                                    0x00000010
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU2_SHIFT                                   5
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU2_MASK                                    0x00000020
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU1_SHIFT                                   6
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU1_MASK                                    0x00000040
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU0_SHIFT                                   7
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TBU0_MASK                                    0x00000080
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_VDSP_APB_SHIFT                                10
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_VDSP_APB_MASK                                 0x00000400
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_M85_APB_SHIFT                                 11
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_M85_APB_MASK                                  0x00000800
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_M85_ATB_SHIFT                                 12
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_M85_ATB_MASK                                  0x00001000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_M85_TS_SHIFT                                  13
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_M85_TS_MASK                                   0x00002000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_VDSP_TS_SHIFT                                 14
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_VDSP_TS_MASK                                  0x00004000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_VDSP_ATB_SHIFT                                15
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_VDSP_ATB_MASK                                 0x00008000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_DEBUG_ATB_SHIFT                               16
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_DEBUG_ATB_MASK                                0x00010000
#define NPU_CRG_CLOCK_ST_0_ST_PCLK_NPU_DEBUG_SHIFT                                  17
#define NPU_CRG_CLOCK_ST_0_ST_PCLK_NPU_DEBUG_MASK                                   0x00020000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CCD_SHIFT                                     18
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CCD_MASK                                      0x00040000
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_DMMU1_SHIFT                                  19
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_DMMU1_MASK                                   0x00080000
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_DMMU0_SHIFT                                  20
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_DMMU0_MASK                                   0x00100000
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TCU_TPC_SHIFT                                21
#define NPU_CRG_CLOCK_ST_0_ST_ACLK_NPU_TCU_TPC_MASK                                 0x00200000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE3_SHIFT                                   26
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE3_MASK                                    0x04000000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE5_SHIFT                                   27
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE5_MASK                                    0x08000000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE2_SHIFT                                   28
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE2_MASK                                    0x10000000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE4_SHIFT                                   29
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE4_MASK                                    0x20000000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE1_SHIFT                                   30
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE1_MASK                                    0x40000000
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE0_SHIFT                                   31
#define NPU_CRG_CLOCK_ST_0_ST_CLK_NPU_CORE0_MASK                                    0x80000000
#define NPU_CRG_CLOCK_CG_1_W1S                      0x0810
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_NS_IPCM_SHIFT                            0
#define NPU_CRG_CLOCK_CG_1_W1S_GT_ACLK_NPU_CORE_SHIFT                               1
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_WTD1_SHIFT                               2
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_WTD0_SHIFT                               3
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_TIMER0_SHIFT                             5
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_CCD_SHIFT                                6
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_SYS_CTRL_SHIFT                           7
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_MISC_SHIFT                               8
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_DMA_SHIFT                                9
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_SEC_REG_SHIFT                            10
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID10_SHIFT                             12
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID9_SHIFT                              13
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID8_SHIFT                              14
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID7_SHIFT                              15
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID6_SHIFT                              16
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID5_SHIFT                              17
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID4_SHIFT                              18
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID3_SHIFT                              19
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID2_SHIFT                              20
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID1_SHIFT                              21
#define NPU_CRG_CLOCK_CG_1_W1S_GT_PCLK_NPU_VMID0_SHIFT                              22
#define NPU_CRG_CLOCK_CG_1_W1S_GT_ACLK_NPU_DMA_SHIFT                                25
#define NPU_CRG_CLOCK_CG_1_W1S_GT_CLK_NPU_LOCAL_ATB_SHIFT                           26
#define NPU_CRG_CLOCK_CG_1_W1S_GT_HCLK_NPU_CORE_CFG_SHIFT                           27
#define NPU_CRG_CLOCK_CG_1_W1S_GT_ACLK_NPU_TCU_SHIFT                                29
#define NPU_CRG_CLOCK_CG_1_W1S_GT_ACLK_NPU_TBU9_SHIFT                               30
#define NPU_CRG_CLOCK_CG_1_W1S_GT_ACLK_NPU_TBU8_SHIFT                               31
#define NPU_CRG_CLOCK_CG_1_W1C                      0x0814
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_NS_IPCM_SHIFT                            0
#define NPU_CRG_CLOCK_CG_1_W1C_GT_ACLK_NPU_CORE_SHIFT                               1
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_WTD1_SHIFT                               2
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_WTD0_SHIFT                               3
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_TIMER0_SHIFT                             5
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_CCD_SHIFT                                6
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_SYS_CTRL_SHIFT                           7
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_MISC_SHIFT                               8
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_DMA_SHIFT                                9
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_SEC_REG_SHIFT                            10
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID10_SHIFT                             12
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID9_SHIFT                              13
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID8_SHIFT                              14
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID7_SHIFT                              15
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID6_SHIFT                              16
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID5_SHIFT                              17
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID4_SHIFT                              18
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID3_SHIFT                              19
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID2_SHIFT                              20
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID1_SHIFT                              21
#define NPU_CRG_CLOCK_CG_1_W1C_GT_PCLK_NPU_VMID0_SHIFT                              22
#define NPU_CRG_CLOCK_CG_1_W1C_GT_ACLK_NPU_DMA_SHIFT                                25
#define NPU_CRG_CLOCK_CG_1_W1C_GT_CLK_NPU_LOCAL_ATB_SHIFT                           26
#define NPU_CRG_CLOCK_CG_1_W1C_GT_HCLK_NPU_CORE_CFG_SHIFT                           27
#define NPU_CRG_CLOCK_CG_1_W1C_GT_ACLK_NPU_TCU_SHIFT                                29
#define NPU_CRG_CLOCK_CG_1_W1C_GT_ACLK_NPU_TBU9_SHIFT                               30
#define NPU_CRG_CLOCK_CG_1_W1C_GT_ACLK_NPU_TBU8_SHIFT                               31
#define NPU_CRG_CLOCK_CG_1_RO                       0x0818
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_NS_IPCM_SHIFT                             0
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_NS_IPCM_MASK                              0x00000001
#define NPU_CRG_CLOCK_CG_1_RO_GT_ACLK_NPU_CORE_SHIFT                                1
#define NPU_CRG_CLOCK_CG_1_RO_GT_ACLK_NPU_CORE_MASK                                 0x00000002
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_WTD1_SHIFT                                2
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_WTD1_MASK                                 0x00000004
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_WTD0_SHIFT                                3
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_WTD0_MASK                                 0x00000008
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_TIMER0_SHIFT                              5
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_TIMER0_MASK                               0x00000020
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_CCD_SHIFT                                 6
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_CCD_MASK                                  0x00000040
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_SYS_CTRL_SHIFT                            7
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_SYS_CTRL_MASK                             0x00000080
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_MISC_SHIFT                                8
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_MISC_MASK                                 0x00000100
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_DMA_SHIFT                                 9
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_DMA_MASK                                  0x00000200
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_SEC_REG_SHIFT                             10
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_SEC_REG_MASK                              0x00000400
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID10_SHIFT                              12
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID10_MASK                               0x00001000
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID9_SHIFT                               13
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID9_MASK                                0x00002000
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID8_SHIFT                               14
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID8_MASK                                0x00004000
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID7_SHIFT                               15
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID7_MASK                                0x00008000
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID6_SHIFT                               16
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID6_MASK                                0x00010000
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID5_SHIFT                               17
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID5_MASK                                0x00020000
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID4_SHIFT                               18
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID4_MASK                                0x00040000
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID3_SHIFT                               19
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID3_MASK                                0x00080000
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID2_SHIFT                               20
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID2_MASK                                0x00100000
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID1_SHIFT                               21
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID1_MASK                                0x00200000
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID0_SHIFT                               22
#define NPU_CRG_CLOCK_CG_1_RO_GT_PCLK_NPU_VMID0_MASK                                0x00400000
#define NPU_CRG_CLOCK_CG_1_RO_GT_ACLK_NPU_DMA_SHIFT                                 25
#define NPU_CRG_CLOCK_CG_1_RO_GT_ACLK_NPU_DMA_MASK                                  0x02000000
#define NPU_CRG_CLOCK_CG_1_RO_GT_CLK_NPU_LOCAL_ATB_SHIFT                            26
#define NPU_CRG_CLOCK_CG_1_RO_GT_CLK_NPU_LOCAL_ATB_MASK                             0x04000000
#define NPU_CRG_CLOCK_CG_1_RO_GT_HCLK_NPU_CORE_CFG_SHIFT                            27
#define NPU_CRG_CLOCK_CG_1_RO_GT_HCLK_NPU_CORE_CFG_MASK                             0x08000000
#define NPU_CRG_CLOCK_CG_1_RO_GT_ACLK_NPU_TCU_SHIFT                                 29
#define NPU_CRG_CLOCK_CG_1_RO_GT_ACLK_NPU_TCU_MASK                                  0x20000000
#define NPU_CRG_CLOCK_CG_1_RO_GT_ACLK_NPU_TBU9_SHIFT                                30
#define NPU_CRG_CLOCK_CG_1_RO_GT_ACLK_NPU_TBU9_MASK                                 0x40000000
#define NPU_CRG_CLOCK_CG_1_RO_GT_ACLK_NPU_TBU8_SHIFT                                31
#define NPU_CRG_CLOCK_CG_1_RO_GT_ACLK_NPU_TBU8_MASK                                 0x80000000
#define NPU_CRG_CLOCK_ST_1                          0x081c
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_NS_IPCM_SHIFT                                0
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_NS_IPCM_MASK                                 0x00000001
#define NPU_CRG_CLOCK_ST_1_ST_ACLK_NPU_CORE_SHIFT                                   1
#define NPU_CRG_CLOCK_ST_1_ST_ACLK_NPU_CORE_MASK                                    0x00000002
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_WTD1_SHIFT                                   2
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_WTD1_MASK                                    0x00000004
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_WTD0_SHIFT                                   3
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_WTD0_MASK                                    0x00000008
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_TIMER0_SHIFT                                 5
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_TIMER0_MASK                                  0x00000020
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_CCD_SHIFT                                    6
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_CCD_MASK                                     0x00000040
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_SYS_CTRL_SHIFT                               7
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_SYS_CTRL_MASK                                0x00000080
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_MISC_SHIFT                                   8
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_MISC_MASK                                    0x00000100
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_DMA_SHIFT                                    9
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_DMA_MASK                                     0x00000200
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_SEC_REG_SHIFT                                10
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_SEC_REG_MASK                                 0x00000400
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID10_SHIFT                                 12
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID10_MASK                                  0x00001000
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID9_SHIFT                                  13
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID9_MASK                                   0x00002000
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID8_SHIFT                                  14
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID8_MASK                                   0x00004000
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID7_SHIFT                                  15
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID7_MASK                                   0x00008000
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID6_SHIFT                                  16
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID6_MASK                                   0x00010000
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID5_SHIFT                                  17
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID5_MASK                                   0x00020000
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID4_SHIFT                                  18
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID4_MASK                                   0x00040000
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID3_SHIFT                                  19
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID3_MASK                                   0x00080000
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID2_SHIFT                                  20
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID2_MASK                                   0x00100000
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID1_SHIFT                                  21
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID1_MASK                                   0x00200000
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID0_SHIFT                                  22
#define NPU_CRG_CLOCK_ST_1_ST_PCLK_NPU_VMID0_MASK                                   0x00400000
#define NPU_CRG_CLOCK_ST_1_ST_ACLK_NPU_DMA_SHIFT                                    25
#define NPU_CRG_CLOCK_ST_1_ST_ACLK_NPU_DMA_MASK                                     0x02000000
#define NPU_CRG_CLOCK_ST_1_ST_CLK_NPU_LOCAL_ATB_SHIFT                               26
#define NPU_CRG_CLOCK_ST_1_ST_CLK_NPU_LOCAL_ATB_MASK                                0x04000000
#define NPU_CRG_CLOCK_ST_1_ST_HCLK_NPU_CORE_CFG_SHIFT                               27
#define NPU_CRG_CLOCK_ST_1_ST_HCLK_NPU_CORE_CFG_MASK                                0x08000000
#define NPU_CRG_CLOCK_ST_1_ST_ACLK_NPU_TCU_SHIFT                                    29
#define NPU_CRG_CLOCK_ST_1_ST_ACLK_NPU_TCU_MASK                                     0x20000000
#define NPU_CRG_CLOCK_ST_1_ST_ACLK_NPU_TBU9_SHIFT                                   30
#define NPU_CRG_CLOCK_ST_1_ST_ACLK_NPU_TBU9_MASK                                    0x40000000
#define NPU_CRG_CLOCK_ST_1_ST_ACLK_NPU_TBU8_SHIFT                                   31
#define NPU_CRG_CLOCK_ST_1_ST_ACLK_NPU_TBU8_MASK                                    0x80000000
#define NPU_CRG_CLOCK_CG_2_W1S                      0x0820
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_NOC_DATA_SHIFT                            0
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_MCU_SYSTICK_SHIFT                         2
#define NPU_CRG_CLOCK_CG_2_W1S_GT_PCLK_NPU_UART_SHIFT                               3
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_WTD1_SHIFT                                4
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_WTD0_SHIFT                                5
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_TIMER1_SHIFT                              6
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_TIMER0_SHIFT                              7
#define NPU_CRG_CLOCK_CG_2_W1S_GT_ACLK_NPU_BIST_SHIFT                               8
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_PROCM_SHIFT                               9
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_HPM1_SHIFT                                10
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_HPM0_SHIFT                                11
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_BUS_DATA_SHIFT                            12
#define NPU_CRG_CLOCK_CG_2_W1S_GT_ACLK_NPU_VDSP_SLV_SHIFT                           13
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_NPU_VDSP_CORE_SHIFT                           15
#define NPU_CRG_CLOCK_CG_2_W1S_GT_PCLK_NPU_S_IPCM_SHIFT                             16
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_VDSP_PLL_LOGIC_SHIFT                          17
#define NPU_CRG_CLOCK_CG_2_W1S_GT_CLK_CORE_PLL_LOGIC_SHIFT                          18
#define NPU_CRG_CLOCK_CG_2_W1S_GT_ACLK_PERF_NPU_M85_SHIFT                           19
#define NPU_CRG_CLOCK_CG_2_W1S_GT_ACLK_PERF_NPU_TCU_SHIFT                           20
#define NPU_CRG_CLOCK_CG_2_W1S_GT_ACLK_PERF_NPU_XIO_DMA_SHIFT                       21
#define NPU_CRG_CLOCK_CG_2_W1S_GT_ACLK_PERF_NPU_VDSP_SHIFT                          22
#define NPU_CRG_CLOCK_CG_2_W1S_GT_ACLK_PERF_NPU_GP_SHIFT                            23
#define NPU_CRG_CLOCK_CG_2_W1S_GT_ACLK_PERF_NPU_DSA_SHIFT                           24
#define NPU_CRG_CLOCK_CG_2_W1S_GT_PCLK_PERF_NPU_M85_SHIFT                           25
#define NPU_CRG_CLOCK_CG_2_W1S_GT_PCLK_PERF_NPU_TCU_SHIFT                           26
#define NPU_CRG_CLOCK_CG_2_W1S_GT_PCLK_PERF_NPU_XIO_DMA_SHIFT                       27
#define NPU_CRG_CLOCK_CG_2_W1S_GT_PCLK_PERF_NPU_VDSP_SHIFT                          28
#define NPU_CRG_CLOCK_CG_2_W1S_GT_PCLK_PERF_NPU_GP_SHIFT                            29
#define NPU_CRG_CLOCK_CG_2_W1S_GT_PCLK_PERF_NPU_DSA_SHIFT                           30
#define NPU_CRG_CLOCK_CG_2_W1S_GT_PCLK_NPU_BIST_SHIFT                               31
#define NPU_CRG_CLOCK_CG_2_W1C                      0x0824
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_NOC_DATA_SHIFT                            0
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_MCU_SYSTICK_SHIFT                         2
#define NPU_CRG_CLOCK_CG_2_W1C_GT_PCLK_NPU_UART_SHIFT                               3
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_WTD1_SHIFT                                4
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_WTD0_SHIFT                                5
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_TIMER1_SHIFT                              6
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_TIMER0_SHIFT                              7
#define NPU_CRG_CLOCK_CG_2_W1C_GT_ACLK_NPU_BIST_SHIFT                               8
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_PROCM_SHIFT                               9
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_HPM1_SHIFT                                10
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_HPM0_SHIFT                                11
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_BUS_DATA_SHIFT                            12
#define NPU_CRG_CLOCK_CG_2_W1C_GT_ACLK_NPU_VDSP_SLV_SHIFT                           13
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_NPU_VDSP_CORE_SHIFT                           15
#define NPU_CRG_CLOCK_CG_2_W1C_GT_PCLK_NPU_S_IPCM_SHIFT                             16
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_VDSP_PLL_LOGIC_SHIFT                          17
#define NPU_CRG_CLOCK_CG_2_W1C_GT_CLK_CORE_PLL_LOGIC_SHIFT                          18
#define NPU_CRG_CLOCK_CG_2_W1C_GT_ACLK_PERF_NPU_M85_SHIFT                           19
#define NPU_CRG_CLOCK_CG_2_W1C_GT_ACLK_PERF_NPU_TCU_SHIFT                           20
#define NPU_CRG_CLOCK_CG_2_W1C_GT_ACLK_PERF_NPU_XIO_DMA_SHIFT                       21
#define NPU_CRG_CLOCK_CG_2_W1C_GT_ACLK_PERF_NPU_VDSP_SHIFT                          22
#define NPU_CRG_CLOCK_CG_2_W1C_GT_ACLK_PERF_NPU_GP_SHIFT                            23
#define NPU_CRG_CLOCK_CG_2_W1C_GT_ACLK_PERF_NPU_DSA_SHIFT                           24
#define NPU_CRG_CLOCK_CG_2_W1C_GT_PCLK_PERF_NPU_M85_SHIFT                           25
#define NPU_CRG_CLOCK_CG_2_W1C_GT_PCLK_PERF_NPU_TCU_SHIFT                           26
#define NPU_CRG_CLOCK_CG_2_W1C_GT_PCLK_PERF_NPU_XIO_DMA_SHIFT                       27
#define NPU_CRG_CLOCK_CG_2_W1C_GT_PCLK_PERF_NPU_VDSP_SHIFT                          28
#define NPU_CRG_CLOCK_CG_2_W1C_GT_PCLK_PERF_NPU_GP_SHIFT                            29
#define NPU_CRG_CLOCK_CG_2_W1C_GT_PCLK_PERF_NPU_DSA_SHIFT                           30
#define NPU_CRG_CLOCK_CG_2_W1C_GT_PCLK_NPU_BIST_SHIFT                               31
#define NPU_CRG_CLOCK_CG_2_RO                       0x0828
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_NOC_DATA_SHIFT                             0
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_NOC_DATA_MASK                              0x00000001
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_MCU_SYSTICK_SHIFT                          2
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_MCU_SYSTICK_MASK                           0x00000004
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_NPU_UART_SHIFT                                3
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_NPU_UART_MASK                                 0x00000008
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_WTD1_SHIFT                                 4
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_WTD1_MASK                                  0x00000010
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_WTD0_SHIFT                                 5
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_WTD0_MASK                                  0x00000020
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_TIMER1_SHIFT                               6
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_TIMER1_MASK                                0x00000040
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_TIMER0_SHIFT                               7
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_TIMER0_MASK                                0x00000080
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_NPU_BIST_SHIFT                                8
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_NPU_BIST_MASK                                 0x00000100
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_PROCM_SHIFT                                9
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_PROCM_MASK                                 0x00000200
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_HPM1_SHIFT                                 10
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_HPM1_MASK                                  0x00000400
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_HPM0_SHIFT                                 11
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_HPM0_MASK                                  0x00000800
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_BUS_DATA_SHIFT                             12
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_BUS_DATA_MASK                              0x00001000
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_NPU_VDSP_SLV_SHIFT                            13
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_NPU_VDSP_SLV_MASK                             0x00002000
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_VDSP_CORE_SHIFT                            15
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_NPU_VDSP_CORE_MASK                             0x00008000
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_NPU_S_IPCM_SHIFT                              16
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_NPU_S_IPCM_MASK                               0x00010000
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_VDSP_PLL_LOGIC_SHIFT                           17
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_VDSP_PLL_LOGIC_MASK                            0x00020000
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_CORE_PLL_LOGIC_SHIFT                           18
#define NPU_CRG_CLOCK_CG_2_RO_GT_CLK_CORE_PLL_LOGIC_MASK                            0x00040000
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_M85_SHIFT                            19
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_M85_MASK                             0x00080000
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_TCU_SHIFT                            20
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_TCU_MASK                             0x00100000
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_XIO_DMA_SHIFT                        21
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_XIO_DMA_MASK                         0x00200000
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_VDSP_SHIFT                           22
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_VDSP_MASK                            0x00400000
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_GP_SHIFT                             23
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_GP_MASK                              0x00800000
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_DSA_SHIFT                            24
#define NPU_CRG_CLOCK_CG_2_RO_GT_ACLK_PERF_NPU_DSA_MASK                             0x01000000
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_M85_SHIFT                            25
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_M85_MASK                             0x02000000
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_TCU_SHIFT                            26
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_TCU_MASK                             0x04000000
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_XIO_DMA_SHIFT                        27
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_XIO_DMA_MASK                         0x08000000
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_VDSP_SHIFT                           28
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_VDSP_MASK                            0x10000000
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_GP_SHIFT                             29
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_GP_MASK                              0x20000000
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_DSA_SHIFT                            30
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_PERF_NPU_DSA_MASK                             0x40000000
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_NPU_BIST_SHIFT                                31
#define NPU_CRG_CLOCK_CG_2_RO_GT_PCLK_NPU_BIST_MASK                                 0x80000000
#define NPU_CRG_CLOCK_ST_2                          0x082c
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_NOC_DATA_SHIFT                                0
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_NOC_DATA_MASK                                 0x00000001
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_MCU_SYSTICK_SHIFT                             2
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_MCU_SYSTICK_MASK                              0x00000004
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_NPU_UART_SHIFT                                   3
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_NPU_UART_MASK                                    0x00000008
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_WTD1_SHIFT                                    4
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_WTD1_MASK                                     0x00000010
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_WTD0_SHIFT                                    5
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_WTD0_MASK                                     0x00000020
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_TIMER1_SHIFT                                  6
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_TIMER1_MASK                                   0x00000040
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_TIMER0_SHIFT                                  7
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_TIMER0_MASK                                   0x00000080
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_NPU_BIST_SHIFT                                   8
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_NPU_BIST_MASK                                    0x00000100
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_PROCM_SHIFT                                   9
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_PROCM_MASK                                    0x00000200
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_HPM1_SHIFT                                    10
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_HPM1_MASK                                     0x00000400
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_HPM0_SHIFT                                    11
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_HPM0_MASK                                     0x00000800
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_BUS_DATA_SHIFT                                12
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_BUS_DATA_MASK                                 0x00001000
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_NPU_VDSP_SLV_SHIFT                               13
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_NPU_VDSP_SLV_MASK                                0x00002000
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_VDSP_CORE_SHIFT                               15
#define NPU_CRG_CLOCK_ST_2_ST_CLK_NPU_VDSP_CORE_MASK                                0x00008000
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_NPU_S_IPCM_SHIFT                                 16
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_NPU_S_IPCM_MASK                                  0x00010000
#define NPU_CRG_CLOCK_ST_2_ST_CLK_VDSP_PLL_LOGIC_SHIFT                              17
#define NPU_CRG_CLOCK_ST_2_ST_CLK_VDSP_PLL_LOGIC_MASK                               0x00020000
#define NPU_CRG_CLOCK_ST_2_ST_CLK_CORE_PLL_LOGIC_SHIFT                              18
#define NPU_CRG_CLOCK_ST_2_ST_CLK_CORE_PLL_LOGIC_MASK                               0x00040000
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_M85_SHIFT                               19
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_M85_MASK                                0x00080000
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_TCU_SHIFT                               20
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_TCU_MASK                                0x00100000
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_XIO_DMA_SHIFT                           21
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_XIO_DMA_MASK                            0x00200000
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_VDSP_SHIFT                              22
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_VDSP_MASK                               0x00400000
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_GP_SHIFT                                23
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_GP_MASK                                 0x00800000
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_DSA_SHIFT                               24
#define NPU_CRG_CLOCK_ST_2_ST_ACLK_PERF_NPU_DSA_MASK                                0x01000000
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_M85_SHIFT                               25
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_M85_MASK                                0x02000000
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_TCU_SHIFT                               26
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_TCU_MASK                                0x04000000
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_XIO_DMA_SHIFT                           27
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_XIO_DMA_MASK                            0x08000000
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_VDSP_SHIFT                              28
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_VDSP_MASK                               0x10000000
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_GP_SHIFT                                29
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_GP_MASK                                 0x20000000
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_DSA_SHIFT                               30
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_PERF_NPU_DSA_MASK                                0x40000000
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_NPU_BIST_SHIFT                                   31
#define NPU_CRG_CLOCK_ST_2_ST_PCLK_NPU_BIST_MASK                                    0x80000000
#define NPU_CRG_CLOCK_CG_3_W1S                      0x0830
#define NPU_CRG_CLOCK_CG_3_W1S_GT_CLK_NPU_MCU_SHIFT                                 7
#define NPU_CRG_CLOCK_CG_3_W1S_GT_CLK_NPU_MCU_IWIC_SHIFT                            8
#define NPU_CRG_CLOCK_CG_3_W1S_GT_CLK_NPU_MCU_DBG_SHIFT                             9
#define NPU_CRG_CLOCK_CG_3_W1S_GT_CLK_NPU_MCU_CORE_SHIFT                            10
#define NPU_CRG_CLOCK_CG_3_W1C                      0x0834
#define NPU_CRG_CLOCK_CG_3_W1C_GT_CLK_NPU_MCU_SHIFT                                 7
#define NPU_CRG_CLOCK_CG_3_W1C_GT_CLK_NPU_MCU_IWIC_SHIFT                            8
#define NPU_CRG_CLOCK_CG_3_W1C_GT_CLK_NPU_MCU_DBG_SHIFT                             9
#define NPU_CRG_CLOCK_CG_3_W1C_GT_CLK_NPU_MCU_CORE_SHIFT                            10
#define NPU_CRG_CLOCK_CG_3_RO                       0x0838
#define NPU_CRG_CLOCK_CG_3_RO_GT_CLK_NPU_MCU_SHIFT                                  7
#define NPU_CRG_CLOCK_CG_3_RO_GT_CLK_NPU_MCU_MASK                                   0x00000080
#define NPU_CRG_CLOCK_CG_3_RO_GT_CLK_NPU_MCU_IWIC_SHIFT                             8
#define NPU_CRG_CLOCK_CG_3_RO_GT_CLK_NPU_MCU_IWIC_MASK                              0x00000100
#define NPU_CRG_CLOCK_CG_3_RO_GT_CLK_NPU_MCU_DBG_SHIFT                              9
#define NPU_CRG_CLOCK_CG_3_RO_GT_CLK_NPU_MCU_DBG_MASK                               0x00000200
#define NPU_CRG_CLOCK_CG_3_RO_GT_CLK_NPU_MCU_CORE_SHIFT                             10
#define NPU_CRG_CLOCK_CG_3_RO_GT_CLK_NPU_MCU_CORE_MASK                              0x00000400
#define NPU_CRG_CLOCK_ST_3                          0x083c
#define NPU_CRG_CLOCK_ST_3_ST_CLK_NPU_MCU_SHIFT                                     7
#define NPU_CRG_CLOCK_ST_3_ST_CLK_NPU_MCU_MASK                                      0x00000080
#define NPU_CRG_CLOCK_ST_3_ST_CLK_NPU_MCU_IWIC_SHIFT                                8
#define NPU_CRG_CLOCK_ST_3_ST_CLK_NPU_MCU_IWIC_MASK                                 0x00000100
#define NPU_CRG_CLOCK_ST_3_ST_CLK_NPU_MCU_DBG_SHIFT                                 9
#define NPU_CRG_CLOCK_ST_3_ST_CLK_NPU_MCU_DBG_MASK                                  0x00000200
#define NPU_CRG_CLOCK_ST_3_ST_CLK_NPU_MCU_CORE_SHIFT                                10
#define NPU_CRG_CLOCK_ST_3_ST_CLK_NPU_MCU_CORE_MASK                                 0x00000400
#define NPU_CRG_CLOCK_SW_0                          0x0860
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_VDSP_CORE_SHIFT                              0
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_VDSP_CORE_MASK                               0x00000003
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_MCU_SHIFT                                    2
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_MCU_MASK                                     0x0000000c
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_NOC_DATA_SHIFT                               4
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_NOC_DATA_MASK                                0x00000010
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_CORE_SHIFT                                   8
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_CORE_MASK                                    0x00000300
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_TIMER1_SHIFT                                 10
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_TIMER1_MASK                                  0x00000400
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_TIMER0_SHIFT                                 11
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_TIMER0_MASK                                  0x00000800
#define NPU_CRG_CLOCK_SW_0_SEL_NPU_PLL_TEST_SHIFT                                   12
#define NPU_CRG_CLOCK_SW_0_SEL_NPU_PLL_TEST_MASK                                    0x00003000
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_VDSP_VDCS_SHIFT                              14
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_VDSP_VDCS_MASK                               0x00004000
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_CORE_VDCS_SHIFT                              15
#define NPU_CRG_CLOCK_SW_0_SEL_CLK_NPU_CORE_VDCS_MASK                               0x00008000
#define NPU_CRG_CLOCK_SW_ST_0                       0x0864
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_VDSP_CORE_SW_SHIFT                     0
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_VDSP_CORE_SW_MASK                      0x00000007
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_MCU_SW_SHIFT                           3
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_MCU_SW_MASK                            0x00000038
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_NOC_DATA_SW_SHIFT                      6
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_NOC_DATA_SW_MASK                       0x000000c0
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_VDSP_VDCS_SW_SHIFT                     8
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_VDSP_VDCS_SW_MASK                      0x00000300
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_CORE_VDCS_SW_SHIFT                     10
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_CORE_VDCS_SW_MASK                      0x00000c00
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_CORE_SW_SHIFT                          13
#define NPU_CRG_CLOCK_SW_ST_0_SW_ACK_CLK_NPU_CORE_SW_MASK                           0x0000e000
#define NPU_CRG_CLOCK_DIV_0                         0x0880
#define NPU_CRG_CLOCK_DIV_0_DIV_CLK_NPU_CORE_PRE_SHIFT                              0
#define NPU_CRG_CLOCK_DIV_0_DIV_CLK_NPU_CORE_PRE_MASK                               0x0000003f
#define NPU_CRG_CLOCK_DIV_0_SC_GT_CLK_NPU_CORE_SHIFT                                6
#define NPU_CRG_CLOCK_DIV_0_SC_GT_CLK_NPU_CORE_MASK                                 0x00000040
#define NPU_CRG_NPU_CLOCK_DIV_1                     0x0888
#define NPU_CRG_NPU_CLOCK_DIV_1_DIV_CLK_NPU_CORE134_SHIFT                           0
#define NPU_CRG_NPU_CLOCK_DIV_1_DIV_CLK_NPU_CORE134_MASK                            0x00000003
#define NPU_CRG_NPU_CLOCK_DIV_1_DIV_CLK_NPU_CORE025_SHIFT                           10
#define NPU_CRG_NPU_CLOCK_DIV_1_DIV_CLK_NPU_CORE025_MASK                            0x00000c00
#define NPU_CRG_NPU_CLOCK_DIV_2                     0x088c
#define NPU_CRG_NPU_CLOCK_DIV_2_DIV_CLK_NPU_BUS_CFG_SHIFT                           0
#define NPU_CRG_NPU_CLOCK_DIV_2_DIV_CLK_NPU_BUS_CFG_MASK                            0x0000003f
#define NPU_CRG_NPU_CLOCK_DIV_2_DIV_CLK_NPU_BUS_DATA_SHIFT                          7
#define NPU_CRG_NPU_CLOCK_DIV_2_DIV_CLK_NPU_BUS_DATA_MASK                           0x00001f80
#define NPU_CRG_NPU_CLOCK_DIV_2_SC_GT_CLK_NPU_BUS_DATA_SHIFT                        13
#define NPU_CRG_NPU_CLOCK_DIV_2_SC_GT_CLK_NPU_BUS_DATA_MASK                         0x00002000
#define NPU_CRG_NPU_CLOCK_DIV_3                     0x0890
#define NPU_CRG_NPU_CLOCK_DIV_3_DIV_CLK_NPU_MCU_SHIFT                               0
#define NPU_CRG_NPU_CLOCK_DIV_3_DIV_CLK_NPU_MCU_MASK                                0x0000003f
#define NPU_CRG_NPU_CLOCK_DIV_3_SC_GT_CLK_NPU_MCU_SHIFT                             6
#define NPU_CRG_NPU_CLOCK_DIV_3_SC_GT_CLK_NPU_MCU_MASK                              0x00000040
#define NPU_CRG_NPU_CLOCK_DIV_3_DIV_CLK_NPU_VDSP_CORE_SHIFT                         7
#define NPU_CRG_NPU_CLOCK_DIV_3_DIV_CLK_NPU_VDSP_CORE_MASK                          0x00001f80
#define NPU_CRG_NPU_CLOCK_DIV_3_SC_GT_CLK_NPU_VDSP_CORE_SHIFT                       13
#define NPU_CRG_NPU_CLOCK_DIV_3_SC_GT_CLK_NPU_VDSP_CORE_MASK                        0x00002000
#define NPU_CRG_NPU_CLOCK_DIV_4                     0x0894
#define NPU_CRG_NPU_CLOCK_DIV_4_DIV_CLK_NPU_NOC_DATA_SHIFT                          0
#define NPU_CRG_NPU_CLOCK_DIV_4_DIV_CLK_NPU_NOC_DATA_MASK                           0x0000003f
#define NPU_CRG_NPU_CLOCK_DIV_4_SC_GT_CLK_NPU_NOC_DATA_SHIFT                        6
#define NPU_CRG_NPU_CLOCK_DIV_4_SC_GT_CLK_NPU_NOC_DATA_MASK                         0x00000040
#define NPU_CRG_NPU_CLOCK_DIV_5                     0x0898
#define NPU_CRG_NPU_CLOCK_DIV_5_DIV_CLK_NPU_REF_SHIFT                               6
#define NPU_CRG_NPU_CLOCK_DIV_5_DIV_CLK_NPU_REF_MASK                                0x00000fc0
#define NPU_CRG_NPU_CLOCK_DIV_7                     0x08a0
#define NPU_CRG_NPU_CLOCK_DIV_7_DIV_CLK_NPU_MCU_SYSTICK_SHIFT                       0
#define NPU_CRG_NPU_CLOCK_DIV_7_DIV_CLK_NPU_MCU_SYSTICK_MASK                        0x0000003f
#define NPU_CRG_NPU_CLOCK_DIV_8                     0x08a4
#define NPU_CRG_NPU_CLOCK_DIV_8_DIV_CLK_NPU_TIMER1_SHIFT                            0
#define NPU_CRG_NPU_CLOCK_DIV_8_DIV_CLK_NPU_TIMER1_MASK                             0x0000003f
#define NPU_CRG_NPU_CLOCK_DIV_8_DIV_CLK_NPU_TIMER0_SHIFT                            6
#define NPU_CRG_NPU_CLOCK_DIV_8_DIV_CLK_NPU_TIMER0_MASK                             0x00000fc0
#define NPU_CRG_NPU_CLOCK_DIV_9                     0x08a8
#define NPU_CRG_NPU_CLOCK_DIV_9_DIV_CLK_NPU_CORE_VDCS_TEST_SHIFT                    0
#define NPU_CRG_NPU_CLOCK_DIV_9_DIV_CLK_NPU_CORE_VDCS_TEST_MASK                     0x0000003f
#define NPU_CRG_NPU_CLOCK_DIV_9_SC_GT_CLK_NPU_CORE_VDCS_TEST_SHIFT                  6
#define NPU_CRG_NPU_CLOCK_DIV_9_SC_GT_CLK_NPU_CORE_VDCS_TEST_MASK                   0x00000040
#define NPU_CRG_NPU_CLOCK_DIV_9_DIV_CLK_NPU_VDSP_VDCS_TEST_SHIFT                    7
#define NPU_CRG_NPU_CLOCK_DIV_9_DIV_CLK_NPU_VDSP_VDCS_TEST_MASK                     0x00001f80
#define NPU_CRG_NPU_CLOCK_DIV_9_SC_GT_CLK_NPU_VDSP_VDCS_TEST_SHIFT                  13
#define NPU_CRG_NPU_CLOCK_DIV_9_SC_GT_CLK_NPU_VDSP_VDCS_TEST_MASK                   0x00002000
#define NPU_CRG_NPU_CLOCK_DIV_10                    0x08ac
#define NPU_CRG_NPU_CLOCK_DIV_10_DIV_CLK_NPU_CORE_PLL_TEST_SHIFT                    0
#define NPU_CRG_NPU_CLOCK_DIV_10_DIV_CLK_NPU_CORE_PLL_TEST_MASK                     0x0000003f
#define NPU_CRG_NPU_CLOCK_DIV_10_SC_GT_CLK_NPU_CORE_PLL_TEST_SHIFT                  6
#define NPU_CRG_NPU_CLOCK_DIV_10_SC_GT_CLK_NPU_CORE_PLL_TEST_MASK                   0x00000040
#define NPU_CRG_NPU_CLOCK_DIV_10_DIV_CLK_NPU_VDSP_PLL_TEST_SHIFT                    7
#define NPU_CRG_NPU_CLOCK_DIV_10_DIV_CLK_NPU_VDSP_PLL_TEST_MASK                     0x00001f80
#define NPU_CRG_NPU_CLOCK_DIV_10_SC_GT_CLK_NPU_VDSP_PLL_TEST_SHIFT                  13
#define NPU_CRG_NPU_CLOCK_DIV_10_SC_GT_CLK_NPU_VDSP_PLL_TEST_MASK                   0x00002000
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0                0x08e0
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_MCU_SYSTICK_SHIFT             0
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_MCU_SYSTICK_MASK              0x00000001
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_HPM0_SHIFT                    1
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_HPM0_MASK                     0x00000002
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_CORE_PRE_DIV_SHIFT            2
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_CORE_PRE_DIV_MASK             0x00000004
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_VDSP_CORE_SHIFT               3
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_VDSP_CORE_MASK                0x00000008
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_MCU_SHIFT                     4
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_MCU_MASK                      0x00000010
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_BUS_CFG_SHIFT                 6
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_BUS_CFG_MASK                  0x00000040
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_BUS_DATA_SHIFT                7
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_BUS_DATA_MASK                 0x00000080
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_NOC_DATA_SHIFT                15
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_NOC_DATA_MASK                 0x00008000
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_TIMER1_SHIFT                  16
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_TIMER1_MASK                   0x00010000
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_TIMER0_SHIFT                  17
#define NPU_CRG_NPU_CLOCK_DIV_DONE_0_DIV_DONE_CLK_NPU_TIMER0_MASK                   0x00020000
#define NPU_CRG_NPU_CLOCK_RST_0_W1S                 0x08f0
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_NS_IPCM_N_SHIFT                     0
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_S_IPCM_N_SHIFT                      1
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_SYS_CTRL_N_SHIFT                    2
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_SEC_REG_N_SHIFT                     3
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_CCD_N_SHIFT                         4
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID10_N_SHIFT                      5
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID9_N_SHIFT                       6
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID8_N_SHIFT                       7
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID7_N_SHIFT                       8
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID6_N_SHIFT                       9
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID5_N_SHIFT                       10
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID4_N_SHIFT                       11
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID3_N_SHIFT                       12
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID2_N_SHIFT                       13
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID1_N_SHIFT                       14
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_VMID0_N_SHIFT                       15
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_PRST_NPU_MISC_N_SHIFT                        17
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_ARST_NPU_CORE_N_SHIFT                        20
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_HRST_NPU_CORE_CFG_N_SHIFT                    21
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_RST_NPU_CORE5_N_SHIFT                        22
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_RST_NPU_CORE4_N_SHIFT                        23
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_RST_NPU_CORE3_N_SHIFT                        24
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_RST_NPU_CORE2_N_SHIFT                        25
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_RST_NPU_CORE1_N_SHIFT                        26
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_RST_NPU_CORE0_N_SHIFT                        27
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_RST_NPU_CORE_N_SHIFT                         28
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_RST_NPU_PROCM_N_SHIFT                        29
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_RST_NPU_HPM1_N_SHIFT                         30
#define NPU_CRG_NPU_CLOCK_RST_0_W1S_IP_RST_NPU_HPM0_N_SHIFT                         31
#define NPU_CRG_NPU_CLOCK_RST_0_W1C                 0x08f4
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_NS_IPCM_N_SHIFT                     0
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_S_IPCM_N_SHIFT                      1
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_SYS_CTRL_N_SHIFT                    2
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_SEC_REG_N_SHIFT                     3
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_CCD_N_SHIFT                         4
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID10_N_SHIFT                      5
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID9_N_SHIFT                       6
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID8_N_SHIFT                       7
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID7_N_SHIFT                       8
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID6_N_SHIFT                       9
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID5_N_SHIFT                       10
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID4_N_SHIFT                       11
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID3_N_SHIFT                       12
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID2_N_SHIFT                       13
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID1_N_SHIFT                       14
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_VMID0_N_SHIFT                       15
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_PRST_NPU_MISC_N_SHIFT                        17
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_ARST_NPU_CORE_N_SHIFT                        20
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_HRST_NPU_CORE_CFG_N_SHIFT                    21
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_RST_NPU_CORE5_N_SHIFT                        22
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_RST_NPU_CORE4_N_SHIFT                        23
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_RST_NPU_CORE3_N_SHIFT                        24
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_RST_NPU_CORE2_N_SHIFT                        25
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_RST_NPU_CORE1_N_SHIFT                        26
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_RST_NPU_CORE0_N_SHIFT                        27
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_RST_NPU_CORE_N_SHIFT                         28
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_RST_NPU_PROCM_N_SHIFT                        29
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_RST_NPU_HPM1_N_SHIFT                         30
#define NPU_CRG_NPU_CLOCK_RST_0_W1C_IP_RST_NPU_HPM0_N_SHIFT                         31
#define NPU_CRG_NPU_CLOCK_RST_0_RO                  0x08f8
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_NS_IPCM_N_SHIFT                      0
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_NS_IPCM_N_MASK                       0x00000001
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_S_IPCM_N_SHIFT                       1
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_S_IPCM_N_MASK                        0x00000002
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_SYS_CTRL_N_SHIFT                     2
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_SYS_CTRL_N_MASK                      0x00000004
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_SEC_REG_N_SHIFT                      3
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_SEC_REG_N_MASK                       0x00000008
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_CCD_N_SHIFT                          4
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_CCD_N_MASK                           0x00000010
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID10_N_SHIFT                       5
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID10_N_MASK                        0x00000020
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID9_N_SHIFT                        6
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID9_N_MASK                         0x00000040
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID8_N_SHIFT                        7
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID8_N_MASK                         0x00000080
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID7_N_SHIFT                        8
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID7_N_MASK                         0x00000100
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID6_N_SHIFT                        9
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID6_N_MASK                         0x00000200
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID5_N_SHIFT                        10
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID5_N_MASK                         0x00000400
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID4_N_SHIFT                        11
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID4_N_MASK                         0x00000800
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID3_N_SHIFT                        12
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID3_N_MASK                         0x00001000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID2_N_SHIFT                        13
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID2_N_MASK                         0x00002000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID1_N_SHIFT                        14
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID1_N_MASK                         0x00004000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID0_N_SHIFT                        15
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_VMID0_N_MASK                         0x00008000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_MISC_N_SHIFT                         17
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_PRST_NPU_MISC_N_MASK                          0x00020000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_ARST_NPU_CORE_N_SHIFT                         20
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_ARST_NPU_CORE_N_MASK                          0x00100000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_HRST_NPU_CORE_CFG_N_SHIFT                     21
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_HRST_NPU_CORE_CFG_N_MASK                      0x00200000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE5_N_SHIFT                         22
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE5_N_MASK                          0x00400000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE4_N_SHIFT                         23
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE4_N_MASK                          0x00800000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE3_N_SHIFT                         24
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE3_N_MASK                          0x01000000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE2_N_SHIFT                         25
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE2_N_MASK                          0x02000000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE1_N_SHIFT                         26
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE1_N_MASK                          0x04000000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE0_N_SHIFT                         27
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE0_N_MASK                          0x08000000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE_N_SHIFT                          28
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_CORE_N_MASK                           0x10000000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_PROCM_N_SHIFT                         29
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_PROCM_N_MASK                          0x20000000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_HPM1_N_SHIFT                          30
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_HPM1_N_MASK                           0x40000000
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_HPM0_N_SHIFT                          31
#define NPU_CRG_NPU_CLOCK_RST_0_RO_IP_RST_NPU_HPM0_N_MASK                           0x80000000
#define NPU_CRG_NPU_CLOCK_RST_1_W1S                 0x08fc
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_DMA_CORE_N_SHIFT                     1
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_VDSP_DBG_N_SHIFT                     3
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_VDSP_N_SHIFT                         4
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_MCU_IWIC_N_SHIFT                     6
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_MCU_POR_N_SHIFT                      7
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_MCU_SYSTICK_N_SHIFT                  8
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_MCU_SYS_N_SHIFT                      9
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_PRST_NPU_UART_N_SHIFT                        10
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_WTD1_N_SHIFT                         11
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_WTD0_N_SHIFT                         12
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_PRST_NPU_WTD1_N_SHIFT                        13
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_PRST_NPU_WTD0_N_SHIFT                        14
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_TIMER1_N_SHIFT                       15
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_TIMER0_N_SHIFT                       16
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_PRST_NPU_TIMER0_N_SHIFT                      18
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_VDSP_PLL_SSMOD_N_SHIFT                   19
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_CORE_PLL_SSMOD_N_SHIFT                   20
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_VDSP_PLL_LOGIC_N_SHIFT                   21
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_CORE_PLL_LOGIC_N_SHIFT                   22
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_BIST_N_SHIFT                         23
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_TCU_N_SHIFT                          25
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_NIC_N_SHIFT                          26
#define NPU_CRG_NPU_CLOCK_RST_1_W1S_IP_RST_NPU_VDSP_APB_N_SHIFT                     27
#define NPU_CRG_NPU_CLOCK_RST_1_W1C                 0x0900
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_DMA_CORE_N_SHIFT                     1
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_VDSP_DBG_N_SHIFT                     3
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_VDSP_N_SHIFT                         4
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_MCU_IWIC_N_SHIFT                     6
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_MCU_POR_N_SHIFT                      7
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_MCU_SYSTICK_N_SHIFT                  8
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_MCU_SYS_N_SHIFT                      9
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_PRST_NPU_UART_N_SHIFT                        10
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_WTD1_N_SHIFT                         11
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_WTD0_N_SHIFT                         12
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_PRST_NPU_WTD1_N_SHIFT                        13
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_PRST_NPU_WTD0_N_SHIFT                        14
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_TIMER1_N_SHIFT                       15
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_TIMER0_N_SHIFT                       16
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_PRST_NPU_TIMER0_N_SHIFT                      18
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_VDSP_PLL_SSMOD_N_SHIFT                   19
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_CORE_PLL_SSMOD_N_SHIFT                   20
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_VDSP_PLL_LOGIC_N_SHIFT                   21
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_CORE_PLL_LOGIC_N_SHIFT                   22
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_BIST_N_SHIFT                         23
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_TCU_N_SHIFT                          25
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_NIC_N_SHIFT                          26
#define NPU_CRG_NPU_CLOCK_RST_1_W1C_IP_RST_NPU_VDSP_APB_N_SHIFT                     27
#define NPU_CRG_NPU_CLOCK_RST_1_RO                  0x0904
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_DMA_CORE_N_SHIFT                      1
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_DMA_CORE_N_MASK                       0x00000002
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_VDSP_DBG_N_SHIFT                      3
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_VDSP_DBG_N_MASK                       0x00000008
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_VDSP_N_SHIFT                          4
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_VDSP_N_MASK                           0x00000010
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_MCU_IWIC_N_SHIFT                      6
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_MCU_IWIC_N_MASK                       0x00000040
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_MCU_POR_N_SHIFT                       7
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_MCU_POR_N_MASK                        0x00000080
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_MCU_SYSTICK_N_SHIFT                   8
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_MCU_SYSTICK_N_MASK                    0x00000100
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_MCU_SYS_N_SHIFT                       9
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_MCU_SYS_N_MASK                        0x00000200
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_PRST_NPU_UART_N_SHIFT                         10
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_PRST_NPU_UART_N_MASK                          0x00000400
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_WTD1_N_SHIFT                          11
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_WTD1_N_MASK                           0x00000800
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_WTD0_N_SHIFT                          12
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_WTD0_N_MASK                           0x00001000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_PRST_NPU_WTD1_N_SHIFT                         13
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_PRST_NPU_WTD1_N_MASK                          0x00002000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_PRST_NPU_WTD0_N_SHIFT                         14
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_PRST_NPU_WTD0_N_MASK                          0x00004000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_TIMER1_N_SHIFT                        15
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_TIMER1_N_MASK                         0x00008000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_TIMER0_N_SHIFT                        16
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_TIMER0_N_MASK                         0x00010000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_PRST_NPU_TIMER0_N_SHIFT                       18
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_PRST_NPU_TIMER0_N_MASK                        0x00040000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_VDSP_PLL_SSMOD_N_SHIFT                    19
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_VDSP_PLL_SSMOD_N_MASK                     0x00080000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_CORE_PLL_SSMOD_N_SHIFT                    20
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_CORE_PLL_SSMOD_N_MASK                     0x00100000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_VDSP_PLL_LOGIC_N_SHIFT                    21
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_VDSP_PLL_LOGIC_N_MASK                     0x00200000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_CORE_PLL_LOGIC_N_SHIFT                    22
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_CORE_PLL_LOGIC_N_MASK                     0x00400000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_BIST_N_SHIFT                          23
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_BIST_N_MASK                           0x00800000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_TCU_N_SHIFT                           25
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_TCU_N_MASK                            0x02000000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_NIC_N_SHIFT                           26
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_NIC_N_MASK                            0x04000000
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_VDSP_APB_N_SHIFT                      27
#define NPU_CRG_NPU_CLOCK_RST_1_RO_IP_RST_NPU_VDSP_APB_N_MASK                       0x08000000
#define NPU_CRG_NPU_CLOCK_RST_2_W1S                 0x0908
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_PERI_N_SHIFT                         0
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_LOCAL_ATB_N_SHIFT                    1
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TCU_TPC_N_SHIFT                      2
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_CCD_N_SHIFT                          3
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_PERF_NPU_M85_N_SHIFT                     4
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_PERF_NPU_TCU_N_SHIFT                     5
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_PERF_NPU_XIO_DMA_N_SHIFT                 6
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_PERF_NPU_VDSP_N_SHIFT                    7
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_PERF_NPU_GP_N_SHIFT                      8
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_PERF_NPU_DSA_N_SHIFT                     9
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_DMMU1_N_SHIFT                        10
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_DMMU0_N_SHIFT                        11
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_PRST_NPU_DEBUG_N_SHIFT                       12
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_DEBUG_ATB_N_SHIFT                    13
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TBU9_N_SHIFT                         14
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TBU8_N_SHIFT                         15
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TBU7_N_SHIFT                         16
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TBU6_N_SHIFT                         17
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TBU5_N_SHIFT                         18
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TBU4_N_SHIFT                         19
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TBU3_N_SHIFT                         20
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TBU2_N_SHIFT                         21
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TBU1_N_SHIFT                         22
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_NPU_TBU0_N_SHIFT                         23
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_VDSP_VDCS_CS_N_SHIFT                     24
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_VDSP_VDCS_VD_N_SHIFT                     25
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_VDSP_VDCS_N_SHIFT                        26
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_CORE_VDCS_CS_N_SHIFT                     27
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_CORE_VDCS_VD_N_SHIFT                     28
#define NPU_CRG_NPU_CLOCK_RST_2_W1S_IP_RST_CORE_VDCS_N_SHIFT                        29
#define NPU_CRG_NPU_CLOCK_RST_2_W1C                 0x090c
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_PERI_N_SHIFT                         0
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_LOCAL_ATB_N_SHIFT                    1
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TCU_TPC_N_SHIFT                      2
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_CCD_N_SHIFT                          3
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_PERF_NPU_M85_N_SHIFT                     4
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_PERF_NPU_TCU_N_SHIFT                     5
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_PERF_NPU_XIO_DMA_N_SHIFT                 6
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_PERF_NPU_VDSP_N_SHIFT                    7
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_PERF_NPU_GP_N_SHIFT                      8
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_PERF_NPU_DSA_N_SHIFT                     9
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_DMMU1_N_SHIFT                        10
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_DMMU0_N_SHIFT                        11
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_PRST_NPU_DEBUG_N_SHIFT                       12
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_DEBUG_ATB_N_SHIFT                    13
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TBU9_N_SHIFT                         14
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TBU8_N_SHIFT                         15
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TBU7_N_SHIFT                         16
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TBU6_N_SHIFT                         17
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TBU5_N_SHIFT                         18
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TBU4_N_SHIFT                         19
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TBU3_N_SHIFT                         20
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TBU2_N_SHIFT                         21
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TBU1_N_SHIFT                         22
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_NPU_TBU0_N_SHIFT                         23
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_VDSP_VDCS_CS_N_SHIFT                     24
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_VDSP_VDCS_VD_N_SHIFT                     25
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_VDSP_VDCS_N_SHIFT                        26
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_CORE_VDCS_CS_N_SHIFT                     27
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_CORE_VDCS_VD_N_SHIFT                     28
#define NPU_CRG_NPU_CLOCK_RST_2_W1C_IP_RST_CORE_VDCS_N_SHIFT                        29
#define NPU_CRG_NPU_CLOCK_RST_2_RO                  0x0910
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_PERI_N_SHIFT                          0
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_PERI_N_MASK                           0x00000001
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_LOCAL_ATB_N_SHIFT                     1
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_LOCAL_ATB_N_MASK                      0x00000002
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TCU_TPC_N_SHIFT                       2
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TCU_TPC_N_MASK                        0x00000004
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_CCD_N_SHIFT                           3
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_CCD_N_MASK                            0x00000008
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_M85_N_SHIFT                      4
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_M85_N_MASK                       0x00000010
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_TCU_N_SHIFT                      5
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_TCU_N_MASK                       0x00000020
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_XIO_DMA_N_SHIFT                  6
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_XIO_DMA_N_MASK                   0x00000040
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_VDSP_N_SHIFT                     7
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_VDSP_N_MASK                      0x00000080
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_GP_N_SHIFT                       8
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_GP_N_MASK                        0x00000100
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_DSA_N_SHIFT                      9
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_PERF_NPU_DSA_N_MASK                       0x00000200
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_DMMU1_N_SHIFT                         10
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_DMMU1_N_MASK                          0x00000400
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_DMMU0_N_SHIFT                         11
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_DMMU0_N_MASK                          0x00000800
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_PRST_NPU_DEBUG_N_SHIFT                        12
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_PRST_NPU_DEBUG_N_MASK                         0x00001000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_DEBUG_ATB_N_SHIFT                     13
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_DEBUG_ATB_N_MASK                      0x00002000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU9_N_SHIFT                          14
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU9_N_MASK                           0x00004000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU8_N_SHIFT                          15
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU8_N_MASK                           0x00008000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU7_N_SHIFT                          16
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU7_N_MASK                           0x00010000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU6_N_SHIFT                          17
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU6_N_MASK                           0x00020000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU5_N_SHIFT                          18
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU5_N_MASK                           0x00040000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU4_N_SHIFT                          19
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU4_N_MASK                           0x00080000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU3_N_SHIFT                          20
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU3_N_MASK                           0x00100000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU2_N_SHIFT                          21
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU2_N_MASK                           0x00200000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU1_N_SHIFT                          22
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU1_N_MASK                           0x00400000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU0_N_SHIFT                          23
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_NPU_TBU0_N_MASK                           0x00800000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_VDSP_VDCS_CS_N_SHIFT                      24
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_VDSP_VDCS_CS_N_MASK                       0x01000000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_VDSP_VDCS_VD_N_SHIFT                      25
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_VDSP_VDCS_VD_N_MASK                       0x02000000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_VDSP_VDCS_N_SHIFT                         26
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_VDSP_VDCS_N_MASK                          0x04000000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_CORE_VDCS_CS_N_SHIFT                      27
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_CORE_VDCS_CS_N_MASK                       0x08000000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_CORE_VDCS_VD_N_SHIFT                      28
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_CORE_VDCS_VD_N_MASK                       0x10000000
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_CORE_VDCS_N_SHIFT                         29
#define NPU_CRG_NPU_CLOCK_RST_2_RO_IP_RST_CORE_VDCS_N_MASK                          0x20000000
#define NPU_CRG_NPU_DVFS_FREQ_DN_EN                 0x0918
#define NPU_CRG_NPU_DVFS_FREQ_DN_EN_CORE_VDCS_FREQ_DN_EN_SHIFT                      0
#define NPU_CRG_NPU_DVFS_FREQ_DN_EN_CORE_VDCS_FREQ_DN_EN_MASK                       0x00000001
#define NPU_CRG_NPU_DVFS_FREQ_DN_EN_VDSP_VDCS_FREQ_DN_EN_SHIFT                      1
#define NPU_CRG_NPU_DVFS_FREQ_DN_EN_VDSP_VDCS_FREQ_DN_EN_MASK                       0x00000002
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG               0x0920
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG_NPU_CORE_PLL_START_FSM_BYPASS_SHIFT           0
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG_NPU_CORE_PLL_START_FSM_BYPASS_MASK            0x00000001
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG_NPU_CORE_PLL_LOCK_TIMEROUT_SHIFT              1
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG_NPU_CORE_PLL_LOCK_TIMEROUT_MASK               0x000003fe
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG_NPU_CORE_PLL_RETRY_NUM_SHIFT                  10
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG_NPU_CORE_PLL_RETRY_NUM_MASK                   0x00001c00
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG_NPU_CORE_PLL_FSM_EN_SHIFT                     13
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG_NPU_CORE_PLL_FSM_EN_MASK                      0x00002000
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG_NPU_CORE_PLL_CFG_RSV0_SHIFT                   14
#define NPU_CRG_NPU_CORE_PLL_VOTE_CFG_NPU_CORE_PLL_CFG_RSV0_MASK                    0xffffc000
#define NPU_CRG_NPU_CORE_PLL_PRESS_TEST             0x0924
#define NPU_CRG_NPU_CORE_PLL_PRESS_TEST_NPU_CORE_PLL_PRESS_TEST_EN_SHIFT            0
#define NPU_CRG_NPU_CORE_PLL_PRESS_TEST_NPU_CORE_PLL_PRESS_TEST_EN_MASK             0x00000001
#define NPU_CRG_NPU_CORE_PLL_PRESS_TEST_NPU_CORE_PLL_PRESS_TEST_CNT_SHIFT           1
#define NPU_CRG_NPU_CORE_PLL_PRESS_TEST_NPU_CORE_PLL_PRESS_TEST_CNT_MASK            0x01fffffe
#define NPU_CRG_NPU_CORE_PLL_PRESS_TEST_NPU_CORE_PLL_CFG_RSV1_SHIFT                 25
#define NPU_CRG_NPU_CORE_PLL_PRESS_TEST_NPU_CORE_PLL_CFG_RSV1_MASK                  0xfe000000
#define NPU_CRG_NPU_CORE_PLL_ERR_BOOT_CNT           0x0928
#define NPU_CRG_NPU_CORE_PLL_ERR_BOOT_CNT_NPU_CORE_PLL_ERR_BOOT_CNT_SHIFT           0
#define NPU_CRG_NPU_CORE_PLL_ERR_BOOT_CNT_NPU_CORE_PLL_ERR_BOOT_CNT_MASK            0x00ffffff
#define NPU_CRG_NPU_CORE_PLL_ERR_BOOT_CNT_NPU_CORE_PLL_PRESS_TEST_END_SHIFT         24
#define NPU_CRG_NPU_CORE_PLL_ERR_BOOT_CNT_NPU_CORE_PLL_PRESS_TEST_END_MASK          0x01000000
#define NPU_CRG_NPU_CORE_PLL_VOTE_MASK              0x092c
#define NPU_CRG_NPU_CORE_PLL_VOTE_MASK_NPU_CORE_PLL_HW_VOTE_MASK_SHIFT              0
#define NPU_CRG_NPU_CORE_PLL_VOTE_MASK_NPU_CORE_PLL_HW_VOTE_MASK_MASK               0x000000ff
#define NPU_CRG_NPU_CORE_PLL_VOTE_MASK_NPU_CORE_PLL_SFT_VOTE_MASK_SHIFT             8
#define NPU_CRG_NPU_CORE_PLL_VOTE_MASK_NPU_CORE_PLL_SFT_VOTE_MASK_MASK              0x0000ff00
#define NPU_CRG_NPU_CORE_PLL_VOTE_MASK_NPU_CORE_PLL_CFG_RSV2_SHIFT                  16
#define NPU_CRG_NPU_CORE_PLL_VOTE_MASK_NPU_CORE_PLL_CFG_RSV2_MASK                   0xffff0000
#define NPU_CRG_NPU_CORE_PLL_VOTE_BYPASS            0x0934
#define NPU_CRG_NPU_CORE_PLL_VOTE_BYPASS_NPU_CORE_PLL_VOTE_BYPASS_SHIFT             0
#define NPU_CRG_NPU_CORE_PLL_VOTE_BYPASS_NPU_CORE_PLL_VOTE_BYPASS_MASK              0x000000ff
#define NPU_CRG_NPU_CORE_PLL_VOTE_GT                0x0938
#define NPU_CRG_NPU_CORE_PLL_VOTE_GT_NPU_CORE_PLL_VOTE_GT_SHIFT                     0
#define NPU_CRG_NPU_CORE_PLL_VOTE_GT_NPU_CORE_PLL_VOTE_GT_MASK                      0x000000ff
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG              0x093c
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG_NPU_CORE_PLL_SSMOD_EN_SHIFT                  0
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG_NPU_CORE_PLL_SSMOD_EN_MASK                   0x00000001
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG_NPU_CORE_PLL_DIVVAL_SHIFT                    1
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG_NPU_CORE_PLL_DIVVAL_MASK                     0x0000007e
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG_NPU_CORE_PLL_SPREAD_SHIFT                    7
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG_NPU_CORE_PLL_SPREAD_MASK                     0x00000f80
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG_NPU_CORE_PLL_DOWNSPREAD_SHIFT                12
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG_NPU_CORE_PLL_DOWNSPREAD_MASK                 0x00001000
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG_NPU_CORE_PLL_CFG_RSV3_SHIFT                  13
#define NPU_CRG_NPU_CORE_PLL_SSMOD_CFG_NPU_CORE_PLL_CFG_RSV3_MASK                   0xffffe000
#define NPU_CRG_NPU_CORE_PLL_CTRL0                  0x0940
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_EN_SHIFT                            0
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_EN_MASK                             0x00000001
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_BYPASS_SHIFT                        1
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_BYPASS_MASK                         0x00000002
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_REFDIV_SHIFT                        2
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_REFDIV_MASK                         0x000000fc
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_FBDIV_SHIFT                         8
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_FBDIV_MASK                          0x000fff00
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_POSTDIV1_SHIFT                      20
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_POSTDIV1_MASK                       0x00700000
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_POSTDIV2_SHIFT                      23
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_POSTDIV2_MASK                       0x03800000
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_CFG_VALID_SHIFT                     26
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_CFG_VALID_MASK                      0x04000000
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_GT_SHIFT                            27
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_GT_MASK                             0x08000000
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_CFG_RSV4_SHIFT                      28
#define NPU_CRG_NPU_CORE_PLL_CTRL0_NPU_CORE_PLL_CFG_RSV4_MASK                       0xf0000000
#define NPU_CRG_NPU_CORE_PLL_CTRL1                  0x0944
#define NPU_CRG_NPU_CORE_PLL_CTRL1_NPU_CORE_PLL_FRAC_SHIFT                          0
#define NPU_CRG_NPU_CORE_PLL_CTRL1_NPU_CORE_PLL_FRAC_MASK                           0x00ffffff
#define NPU_CRG_NPU_CORE_PLL_CTRL1_NPU_CORE_PLL_DSM_EN_SHIFT                        24
#define NPU_CRG_NPU_CORE_PLL_CTRL1_NPU_CORE_PLL_DSM_EN_MASK                         0x01000000
#define NPU_CRG_NPU_CORE_PLL_CTRL1_NPU_CORE_PLL_CFG_RSV5_SHIFT                      25
#define NPU_CRG_NPU_CORE_PLL_CTRL1_NPU_CORE_PLL_CFG_RSV5_MASK                       0xfe000000
#define NPU_CRG_NPU_CORE_PLL_DEBUG                  0x0948
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETCALBYP_SHIFT                  0
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETCALBYP_MASK                   0x00000001
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETCALCNT_SHIFT                  1
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETCALCNT_MASK                   0x0000000e
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETCALEN_SHIFT                   4
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETCALEN_MASK                    0x00000010
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETCALIN_SHIFT                   5
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETCALIN_MASK                    0x0001ffe0
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETCALRSTN_SHIFT                 17
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETCALRSTN_MASK                  0x00020000
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETFASTCAL_SHIFT                 18
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_OFFSETFASTCAL_MASK                  0x00040000
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_CFG_RSV6_SHIFT                      19
#define NPU_CRG_NPU_CORE_PLL_DEBUG_NPU_CORE_PLL_CFG_RSV6_MASK                       0xfff80000
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE            0x094c
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_EN_STATE_SHIFT                0
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_EN_STATE_MASK                 0x00000001
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_OFFSETCALOUT_SHIFT            1
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_OFFSETCALOUT_MASK             0x00001ffe
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_OFFSETCALLOCK_SHIFT           13
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_OFFSETCALLOCK_MASK            0x00002000
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_OFFSETCALOVF_SHIFT            14
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_OFFSETCALOVF_MASK             0x00004000
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_START_FSM_STATE_SHIFT         15
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_START_FSM_STATE_MASK          0x00038000
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_REAL_RETRY_CNT_SHIFT          18
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_PLL_REAL_RETRY_CNT_MASK           0x001c0000
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_ST_CLK_PLL_SHIFT                  21
#define NPU_CRG_NPU_CORE_PLL_DEBUG_STATE_NPU_CORE_ST_CLK_PLL_MASK                   0x00200000
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG               0x0980
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG_NPU_VDSP_PLL_START_FSM_BYPASS_SHIFT           0
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG_NPU_VDSP_PLL_START_FSM_BYPASS_MASK            0x00000001
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG_NPU_VDSP_PLL_LOCK_TIMEROUT_SHIFT              1
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG_NPU_VDSP_PLL_LOCK_TIMEROUT_MASK               0x000003fe
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG_NPU_VDSP_PLL_RETRY_NUM_SHIFT                  10
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG_NPU_VDSP_PLL_RETRY_NUM_MASK                   0x00001c00
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG_NPU_VDSP_PLL_FSM_EN_SHIFT                     13
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG_NPU_VDSP_PLL_FSM_EN_MASK                      0x00002000
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG_NPU_VDSP_PLL_CFG_RSV0_SHIFT                   14
#define NPU_CRG_NPU_VDSP_PLL_VOTE_CFG_NPU_VDSP_PLL_CFG_RSV0_MASK                    0xffffc000
#define NPU_CRG_NPU_VDSP_PLL_PRESS_TEST             0x0984
#define NPU_CRG_NPU_VDSP_PLL_PRESS_TEST_NPU_VDSP_PLL_PRESS_TEST_EN_SHIFT            0
#define NPU_CRG_NPU_VDSP_PLL_PRESS_TEST_NPU_VDSP_PLL_PRESS_TEST_EN_MASK             0x00000001
#define NPU_CRG_NPU_VDSP_PLL_PRESS_TEST_NPU_VDSP_PLL_PRESS_TEST_CNT_SHIFT           1
#define NPU_CRG_NPU_VDSP_PLL_PRESS_TEST_NPU_VDSP_PLL_PRESS_TEST_CNT_MASK            0x01fffffe
#define NPU_CRG_NPU_VDSP_PLL_PRESS_TEST_NPU_VDSP_PLL_CFG_RSV1_SHIFT                 25
#define NPU_CRG_NPU_VDSP_PLL_PRESS_TEST_NPU_VDSP_PLL_CFG_RSV1_MASK                  0xfe000000
#define NPU_CRG_NPU_VDSP_PLL_ERR_BOOT_CNT           0x0988
#define NPU_CRG_NPU_VDSP_PLL_ERR_BOOT_CNT_NPU_VDSP_PLL_ERR_BOOT_CNT_SHIFT           0
#define NPU_CRG_NPU_VDSP_PLL_ERR_BOOT_CNT_NPU_VDSP_PLL_ERR_BOOT_CNT_MASK            0x00ffffff
#define NPU_CRG_NPU_VDSP_PLL_ERR_BOOT_CNT_NPU_VDSP_PLL_PRESS_TEST_END_SHIFT         24
#define NPU_CRG_NPU_VDSP_PLL_ERR_BOOT_CNT_NPU_VDSP_PLL_PRESS_TEST_END_MASK          0x01000000
#define NPU_CRG_NPU_VDSP_PLL_VOTE_MASK              0x098c
#define NPU_CRG_NPU_VDSP_PLL_VOTE_MASK_NPU_VDSP_PLL_HW_VOTE_MASK_SHIFT              0
#define NPU_CRG_NPU_VDSP_PLL_VOTE_MASK_NPU_VDSP_PLL_HW_VOTE_MASK_MASK               0x000000ff
#define NPU_CRG_NPU_VDSP_PLL_VOTE_MASK_NPU_VDSP_PLL_SFT_VOTE_MASK_SHIFT             8
#define NPU_CRG_NPU_VDSP_PLL_VOTE_MASK_NPU_VDSP_PLL_SFT_VOTE_MASK_MASK              0x0000ff00
#define NPU_CRG_NPU_VDSP_PLL_VOTE_MASK_NPU_VDSP_PLL_CFG_RSV2_SHIFT                  16
#define NPU_CRG_NPU_VDSP_PLL_VOTE_MASK_NPU_VDSP_PLL_CFG_RSV2_MASK                   0xffff0000
#define NPU_CRG_NPU_VDSP_PLL_VOTE_BYPASS            0x0994
#define NPU_CRG_NPU_VDSP_PLL_VOTE_BYPASS_NPU_VDSP_PLL_VOTE_BYPASS_SHIFT             0
#define NPU_CRG_NPU_VDSP_PLL_VOTE_BYPASS_NPU_VDSP_PLL_VOTE_BYPASS_MASK              0x000000ff
#define NPU_CRG_NPU_VDSP_PLL_VOTE_GT                0x0998
#define NPU_CRG_NPU_VDSP_PLL_VOTE_GT_NPU_VDSP_PLL_VOTE_GT_SHIFT                     0
#define NPU_CRG_NPU_VDSP_PLL_VOTE_GT_NPU_VDSP_PLL_VOTE_GT_MASK                      0x000000ff
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG              0x099c
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG_NPU_VDSP_PLL_SSMOD_EN_SHIFT                  0
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG_NPU_VDSP_PLL_SSMOD_EN_MASK                   0x00000001
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG_NPU_VDSP_PLL_DIVVAL_SHIFT                    1
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG_NPU_VDSP_PLL_DIVVAL_MASK                     0x0000007e
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG_NPU_VDSP_PLL_SPREAD_SHIFT                    7
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG_NPU_VDSP_PLL_SPREAD_MASK                     0x00000f80
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG_NPU_VDSP_PLL_DOWNSPREAD_SHIFT                12
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG_NPU_VDSP_PLL_DOWNSPREAD_MASK                 0x00001000
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG_NPU_VDSP_PLL_CFG_RSV3_SHIFT                  13
#define NPU_CRG_NPU_VDSP_PLL_SSMOD_CFG_NPU_VDSP_PLL_CFG_RSV3_MASK                   0xffffe000
#define NPU_CRG_NPU_VDSP_PLL_CTRL0                  0x09a0
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_EN_SHIFT                            0
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_EN_MASK                             0x00000001
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_BYPASS_SHIFT                        1
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_BYPASS_MASK                         0x00000002
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_REFDIV_SHIFT                        2
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_REFDIV_MASK                         0x000000fc
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_FBDIV_SHIFT                         8
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_FBDIV_MASK                          0x000fff00
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_POSTDIV1_SHIFT                      20
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_POSTDIV1_MASK                       0x00700000
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_POSTDIV2_SHIFT                      23
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_POSTDIV2_MASK                       0x03800000
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_CFG_VALID_SHIFT                     26
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_CFG_VALID_MASK                      0x04000000
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_GT_SHIFT                            27
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_GT_MASK                             0x08000000
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_CFG_RSV4_SHIFT                      28
#define NPU_CRG_NPU_VDSP_PLL_CTRL0_NPU_VDSP_PLL_CFG_RSV4_MASK                       0xf0000000
#define NPU_CRG_NPU_VDSP_PLL_CTRL1                  0x09a4
#define NPU_CRG_NPU_VDSP_PLL_CTRL1_NPU_VDSP_PLL_FRAC_SHIFT                          0
#define NPU_CRG_NPU_VDSP_PLL_CTRL1_NPU_VDSP_PLL_FRAC_MASK                           0x00ffffff
#define NPU_CRG_NPU_VDSP_PLL_CTRL1_NPU_VDSP_PLL_DSM_EN_SHIFT                        24
#define NPU_CRG_NPU_VDSP_PLL_CTRL1_NPU_VDSP_PLL_DSM_EN_MASK                         0x01000000
#define NPU_CRG_NPU_VDSP_PLL_CTRL1_NPU_VDSP_PLL_CFG_RSV5_SHIFT                      25
#define NPU_CRG_NPU_VDSP_PLL_CTRL1_NPU_VDSP_PLL_CFG_RSV5_MASK                       0xfe000000
#define NPU_CRG_NPU_VDSP_PLL_DEBUG                  0x09a8
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETCALBYP_SHIFT                  0
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETCALBYP_MASK                   0x00000001
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETCALCNT_SHIFT                  1
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETCALCNT_MASK                   0x0000000e
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETCALEN_SHIFT                   4
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETCALEN_MASK                    0x00000010
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETCALIN_SHIFT                   5
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETCALIN_MASK                    0x0001ffe0
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETCALRSTN_SHIFT                 17
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETCALRSTN_MASK                  0x00020000
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETFASTCAL_SHIFT                 18
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_OFFSETFASTCAL_MASK                  0x00040000
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_CFG_RSV6_SHIFT                      19
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_NPU_VDSP_PLL_CFG_RSV6_MASK                       0xfff80000
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE            0x09ac
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_EN_STATE_SHIFT                0
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_EN_STATE_MASK                 0x00000001
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_OFFSETCALOUT_SHIFT            1
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_OFFSETCALOUT_MASK             0x00001ffe
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_OFFSETCALLOCK_SHIFT           13
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_OFFSETCALLOCK_MASK            0x00002000
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_OFFSETCALOVF_SHIFT            14
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_OFFSETCALOVF_MASK             0x00004000
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_START_FSM_STATE_SHIFT         15
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_START_FSM_STATE_MASK          0x00038000
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_REAL_RETRY_CNT_SHIFT          18
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_PLL_REAL_RETRY_CNT_MASK           0x001c0000
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_ST_CLK_PLL_SHIFT                  21
#define NPU_CRG_NPU_VDSP_PLL_DEBUG_STATE_NPU_VDSP_ST_CLK_PLL_MASK                   0x00200000
#define NPU_CRG_HPM0_CFG_0                          0x0a00
#define NPU_CRG_HPM0_CFG_0_NPU_HPM_CLEAR_0_SHIFT                                    0
#define NPU_CRG_HPM0_CFG_0_NPU_HPM_CLEAR_0_MASK                                     0x00000001
#define NPU_CRG_HPM0_CFG_0_NPU_HPM_RO_SEL_0_SHIFT                                   1
#define NPU_CRG_HPM0_CFG_0_NPU_HPM_RO_SEL_0_MASK                                    0x0000007e
#define NPU_CRG_HPM0_CFG_0_NPU_HPM_RO_CLK_DIV_0_SHIFT                               7
#define NPU_CRG_HPM0_CFG_0_NPU_HPM_RO_CLK_DIV_0_MASK                                0x00000180
#define NPU_CRG_HPM0_CFG_0_NPU_HPM_RO_DBG_CLK_DIV_0_SHIFT                           9
#define NPU_CRG_HPM0_CFG_0_NPU_HPM_RO_DBG_CLK_DIV_0_MASK                            0x00007e00
#define NPU_CRG_HPM0_CFG_1                          0x0a04
#define NPU_CRG_HPM0_CFG_1_NPU_HPM_RO_EN_0_SHIFT                                    0
#define NPU_CRG_HPM0_CFG_1_NPU_HPM_RO_EN_0_MASK                                     0x00000001
#define NPU_CRG_HPM0_CFG_1_NPU_HPM_TEST_HW_EN_0_SHIFT                               1
#define NPU_CRG_HPM0_CFG_1_NPU_HPM_TEST_HW_EN_0_MASK                                0x00000002
#define NPU_CRG_HPM0_CFG_1_NPU_HPM_TEST_HW_START_0_SHIFT                            2
#define NPU_CRG_HPM0_CFG_1_NPU_HPM_TEST_HW_START_0_MASK                             0x00000004
#define NPU_CRG_HPM0_CFG_1_NPU_HPM_TIMER_CNT_0_SHIFT                                3
#define NPU_CRG_HPM0_CFG_1_NPU_HPM_TIMER_CNT_0_MASK                                 0x000007f8
#define NPU_CRG_HPM0_CLK_GT_W1S                     0x0a08
#define NPU_CRG_HPM0_CLK_GT_W1S_NPU_HPM_CLK_GT_0_SHIFT                              0
#define NPU_CRG_HPM0_CLK_GT_W1C                     0x0a0c
#define NPU_CRG_HPM0_CLK_GT_W1C_NPU_HPM_CLK_GT_0_SHIFT                              0
#define NPU_CRG_HPM0_CLK_GT_RO                      0x0a10
#define NPU_CRG_HPM0_CLK_GT_RO_NPU_HPM_CLK_GT_0_SHIFT                               0
#define NPU_CRG_HPM0_CLK_GT_RO_NPU_HPM_CLK_GT_0_MASK                                0x00000001
#define NPU_CRG_HPM0_DOUT                           0x0a14
#define NPU_CRG_HPM0_DOUT_NPU_HPM_DOUT_0_SHIFT                                      0
#define NPU_CRG_HPM0_DOUT_NPU_HPM_DOUT_0_MASK                                       0x0003ffff
#define NPU_CRG_NPU_HPM1_CFG_0                      0x0a18
#define NPU_CRG_NPU_HPM1_CFG_0_NPU_HPM_CLEAR_1_SHIFT                                0
#define NPU_CRG_NPU_HPM1_CFG_0_NPU_HPM_CLEAR_1_MASK                                 0x00000001
#define NPU_CRG_NPU_HPM1_CFG_0_NPU_HPM_RO_SEL_1_SHIFT                               1
#define NPU_CRG_NPU_HPM1_CFG_0_NPU_HPM_RO_SEL_1_MASK                                0x0000007e
#define NPU_CRG_NPU_HPM1_CFG_0_NPU_HPM_RO_CLK_DIV_1_SHIFT                           7
#define NPU_CRG_NPU_HPM1_CFG_0_NPU_HPM_RO_CLK_DIV_1_MASK                            0x00000180
#define NPU_CRG_NPU_HPM1_CFG_0_NPU_HPM_RO_DBG_CLK_DIV_1_SHIFT                       9
#define NPU_CRG_NPU_HPM1_CFG_0_NPU_HPM_RO_DBG_CLK_DIV_1_MASK                        0x00007e00
#define NPU_CRG_NPU_HPM1_CFG_1                      0x0a1c
#define NPU_CRG_NPU_HPM1_CFG_1_NPU_HPM_RO_EN_1_SHIFT                                0
#define NPU_CRG_NPU_HPM1_CFG_1_NPU_HPM_RO_EN_1_MASK                                 0x00000001
#define NPU_CRG_NPU_HPM1_CFG_1_NPU_HPM_TEST_HW_EN_1_SHIFT                           1
#define NPU_CRG_NPU_HPM1_CFG_1_NPU_HPM_TEST_HW_EN_1_MASK                            0x00000002
#define NPU_CRG_NPU_HPM1_CFG_1_NPU_HPM_TEST_HW_START_1_SHIFT                        2
#define NPU_CRG_NPU_HPM1_CFG_1_NPU_HPM_TEST_HW_START_1_MASK                         0x00000004
#define NPU_CRG_NPU_HPM1_CFG_1_NPU_HPM_TIMER_CNT_1_SHIFT                            3
#define NPU_CRG_NPU_HPM1_CFG_1_NPU_HPM_TIMER_CNT_1_MASK                             0x000007f8
#define NPU_CRG_NPU_HPM1_CLK_GT_W1S                 0x0a20
#define NPU_CRG_NPU_HPM1_CLK_GT_W1S_NPU_HPM_CLK_GT_1_SHIFT                          0
#define NPU_CRG_NPU_HPM1_CLK_GT_W1C                 0x0a24
#define NPU_CRG_NPU_HPM1_CLK_GT_W1C_NPU_HPM_CLK_GT_1_SHIFT                          0
#define NPU_CRG_NPU_HPM1_CLK_GT_RO                  0x0a28
#define NPU_CRG_NPU_HPM1_CLK_GT_RO_NPU_HPM_CLK_GT_1_SHIFT                           0
#define NPU_CRG_NPU_HPM1_CLK_GT_RO_NPU_HPM_CLK_GT_1_MASK                            0x00000001
#define NPU_CRG_NPU_HPM1_DOUT                       0x0a2c
#define NPU_CRG_NPU_HPM1_DOUT_NPU_HPM_DOUT_1_SHIFT                                  0
#define NPU_CRG_NPU_HPM1_DOUT_NPU_HPM_DOUT_1_MASK                                   0x0003ffff
#define NPU_CRG_NPU_PROCM_CFG_0                     0x0a30
#define NPU_CRG_NPU_PROCM_CFG_0_NPU_PROCM_CLEAR_SHIFT                               0
#define NPU_CRG_NPU_PROCM_CFG_0_NPU_PROCM_CLEAR_MASK                                0x00000001
#define NPU_CRG_NPU_PROCM_CFG_0_NPU_PROCM_RO_SEL_SHIFT                              1
#define NPU_CRG_NPU_PROCM_CFG_0_NPU_PROCM_RO_SEL_MASK                               0x000000fe
#define NPU_CRG_NPU_PROCM_CFG_0_NPU_PROCM_TIMER_SEL_SHIFT                           8
#define NPU_CRG_NPU_PROCM_CFG_0_NPU_PROCM_TIMER_SEL_MASK                            0x00000300
#define NPU_CRG_NPU_PROCM_CFG_0_NPU_PROCM_RO_CLK_DIV_SHIFT                          10
#define NPU_CRG_NPU_PROCM_CFG_0_NPU_PROCM_RO_CLK_DIV_MASK                           0x00000400
#define NPU_CRG_NPU_PROCM_CFG_1                     0x0a34
#define NPU_CRG_NPU_PROCM_CFG_1_NPU_PROCM_RO_EN_SHIFT                               0
#define NPU_CRG_NPU_PROCM_CFG_1_NPU_PROCM_RO_EN_MASK                                0x00000001
#define NPU_CRG_NPU_PROCM_CFG_1_NPU_PROCM_TEST_HW_EN_SHIFT                          1
#define NPU_CRG_NPU_PROCM_CFG_1_NPU_PROCM_TEST_HW_EN_MASK                           0x00000002
#define NPU_CRG_NPU_PROCM_CFG_1_NPU_PROCM_TEST_HW_START_SHIFT                       2
#define NPU_CRG_NPU_PROCM_CFG_1_NPU_PROCM_TEST_HW_START_MASK                        0x00000004
#define NPU_CRG_NPU_PROCM_CFG_1_NPU_PROCM_RO_DBG_CLK_DIV_SHIFT                      3
#define NPU_CRG_NPU_PROCM_CFG_1_NPU_PROCM_RO_DBG_CLK_DIV_MASK                       0x000001f8
#define NPU_CRG_NPU_PROCM_CFG_1_NPU_PROCM_SEL_H_SHIFT                               9
#define NPU_CRG_NPU_PROCM_CFG_1_NPU_PROCM_SEL_H_MASK                                0x00000200
#define NPU_CRG_NPU_PROCM_CLK_GT_W1S                0x0a38
#define NPU_CRG_NPU_PROCM_CLK_GT_W1S_NPU_PROCM_CLK_GT_SHIFT                         0
#define NPU_CRG_NPU_PROCM_CLK_GT_W1C                0x0a3c
#define NPU_CRG_NPU_PROCM_CLK_GT_W1C_NPU_PROCM_CLK_GT_SHIFT                         0
#define NPU_CRG_NPU_PROCM_CLK_GT_RO                 0x0a40
#define NPU_CRG_NPU_PROCM_CLK_GT_RO_NPU_PROCM_CLK_GT_SHIFT                          0
#define NPU_CRG_NPU_PROCM_CLK_GT_RO_NPU_PROCM_CLK_GT_MASK                           0x00000001
#define NPU_CRG_NPU_PROCM_DOUT                      0x0a44
#define NPU_CRG_NPU_PROCM_DOUT_NPU_PROCM_DOUT_SHIFT                                 0
#define NPU_CRG_NPU_PROCM_DOUT_NPU_PROCM_DOUT_MASK                                  0x01ffffff
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS              0x0a60
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID9_BYPASS_SHIFT               0
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID9_BYPASS_MASK                0x00000001
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID8_BYPASS_SHIFT               1
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID8_BYPASS_MASK                0x00000002
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID7_BYPASS_SHIFT               2
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID7_BYPASS_MASK                0x00000004
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID6_BYPASS_SHIFT               3
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID6_BYPASS_MASK                0x00000008
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID5_BYPASS_SHIFT               4
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID5_BYPASS_MASK                0x00000010
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID4_BYPASS_SHIFT               5
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID4_BYPASS_MASK                0x00000020
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID3_BYPASS_SHIFT               6
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID3_BYPASS_MASK                0x00000040
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID2_BYPASS_SHIFT               7
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID2_BYPASS_MASK                0x00000080
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID1_BYPASS_SHIFT               8
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID1_BYPASS_MASK                0x00000100
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID0_BYPASS_SHIFT               9
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID0_BYPASS_MASK                0x00000200
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_MCU_BYPASS_SHIFT                 10
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_MCU_BYPASS_MASK                  0x00000400
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_SYSCTRL_TOP_BYPASS_SHIFT         11
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_SYSCTRL_TOP_BYPASS_MASK          0x00000800
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_DRAM_BIST_TOP_BYPASS_SHIFT       12
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_DRAM_BIST_TOP_BYPASS_MASK        0x00001000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_SEC_REG_TOP_BYPASS_SHIFT         13
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_SEC_REG_TOP_BYPASS_MASK          0x00002000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_DMAC_TOP_BYPASS_SHIFT            14
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_DMAC_TOP_BYPASS_MASK             0x00004000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_DMMU1_BYPASS_SHIFT               15
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_DMMU1_BYPASS_MASK                0x00008000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_DMMU0_BYPASS_SHIFT               16
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_DMMU0_BYPASS_MASK                0x00010000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_TCU_BYPASS_SHIFT                 17
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_TCU_BYPASS_MASK                  0x00020000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_IPCM1_BYPASS_SHIFT               18
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_IPCM1_BYPASS_MASK                0x00040000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_IPCM0_BYPASS_SHIFT               19
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_IPCM0_BYPASS_MASK                0x00080000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_UART_BYPASS_SHIFT                20
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_UART_BYPASS_MASK                 0x00100000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_WTD1_BYPASS_SHIFT                21
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_WTD1_BYPASS_MASK                 0x00200000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_WTD0_BYPASS_SHIFT                22
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_WTD0_BYPASS_MASK                 0x00400000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_TIMERS_BYPASS_SHIFT              23
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_TIMERS_BYPASS_MASK               0x00800000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_CCD_BYPASS_SHIFT                 24
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_CCD_BYPASS_MASK                  0x01000000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_MISC_BYPASS_SHIFT                25
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_MISC_BYPASS_MASK                 0x02000000
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID10_BYPASS_SHIFT              26
#define NPU_CRG_NPU_CLKRST_FLAG_BYPASS_CLKRST_FLAG_VMID10_BYPASS_MASK               0x04000000
#define NPU_CRG_AUTOFSCTRL1                         0x0a70
#define NPU_CRG_AUTOFSCTRL1_NPU_BUS_TGT_NO_PENDING_TRANS_DATA_BYPASS_SHIFT          0
#define NPU_CRG_AUTOFSCTRL1_NPU_BUS_TGT_NO_PENDING_TRANS_DATA_BYPASS_MASK           0x0000007f
#define NPU_CRG_AUTOFSCTRL1_NPU_BUS_INI_NO_PENDING_TRANS_DATA_BYPASS_SHIFT          7
#define NPU_CRG_AUTOFSCTRL1_NPU_BUS_INI_NO_PENDING_TRANS_DATA_BYPASS_MASK           0x000fff80
#define NPU_CRG_AUTOFSCTRL1_TCU_IDLE_FLAG_DATA_BYPASS_SHIFT                         20
#define NPU_CRG_AUTOFSCTRL1_TCU_IDLE_FLAG_DATA_BYPASS_MASK                          0x00100000
#define NPU_CRG_AUTOFSCTRL1_TCU_IDLE_FLAG_PERFSTAT_DATA_BYPASS_SHIFT                21
#define NPU_CRG_AUTOFSCTRL1_TCU_IDLE_FLAG_PERFSTAT_DATA_BYPASS_MASK                 0x00200000
#define NPU_CRG_AUTOFSCTRL1_VDSP_IDLE_FLAG_PERFSTAT_DATA_BYPASS_SHIFT               22
#define NPU_CRG_AUTOFSCTRL1_VDSP_IDLE_FLAG_PERFSTAT_DATA_BYPASS_MASK                0x00400000
#define NPU_CRG_AUTOFSCTRL1_GP_IDLE_FLAG_PERFSTAT_DATA_BYPASS_SHIFT                 23
#define NPU_CRG_AUTOFSCTRL1_GP_IDLE_FLAG_PERFSTAT_DATA_BYPASS_MASK                  0x00800000
#define NPU_CRG_AUTOFSCTRL1_DSA_IDLE_FLAG_PERFSTAT_DATA_BYPASS_SHIFT                24
#define NPU_CRG_AUTOFSCTRL1_DSA_IDLE_FLAG_PERFSTAT_DATA_BYPASS_MASK                 0x01000000
#define NPU_CRG_AUTOFSCTRL1_XIO_DMA_IDLE_FLAG_PERFSTAT_DATA_BYPASS_SHIFT            25
#define NPU_CRG_AUTOFSCTRL1_XIO_DMA_IDLE_FLAG_PERFSTAT_DATA_BYPASS_MASK             0x02000000
#define NPU_CRG_AUTOFSCTRL1_CACTIVE_INV_DATA_BYPASS_SHIFT                           26
#define NPU_CRG_AUTOFSCTRL1_CACTIVE_INV_DATA_BYPASS_MASK                            0x04000000
#define NPU_CRG_AUTOFSCTRL1_NPU_VDSP_CORE_IDLE_DATA_BYPASS_SHIFT                    27
#define NPU_CRG_AUTOFSCTRL1_NPU_VDSP_CORE_IDLE_DATA_BYPASS_MASK                     0x08000000
#define NPU_CRG_AUTOFSCTRL1_NPU_VIP_CORE_IDLE_DATA_BYPASS_SHIFT                     28
#define NPU_CRG_AUTOFSCTRL1_NPU_VIP_CORE_IDLE_DATA_BYPASS_MASK                      0x10000000
#define NPU_CRG_AUTOFSCTRL1_TOP_DMA_IDLE_FLAG_BYPASS_SHIFT                          29
#define NPU_CRG_AUTOFSCTRL1_TOP_DMA_IDLE_FLAG_BYPASS_MASK                           0x20000000
#define NPU_CRG_AUTOFSCTRL1_NPU_MCU_CORE_IDLE_DATA_BYPASS_SHIFT                     30
#define NPU_CRG_AUTOFSCTRL1_NPU_MCU_CORE_IDLE_DATA_BYPASS_MASK                      0x40000000
#define NPU_CRG_AUTOFSCTRL4                         0x0a7c
#define NPU_CRG_AUTOFSCTRL4_DEBOUNCE_IN_NPU_BUS_DATA_SHIFT                          0
#define NPU_CRG_AUTOFSCTRL4_DEBOUNCE_IN_NPU_BUS_DATA_MASK                           0x000003ff
#define NPU_CRG_AUTOFSCTRL4_DEBOUNCE_OUT_NPU_BUS_DATA_SHIFT                         10
#define NPU_CRG_AUTOFSCTRL4_DEBOUNCE_OUT_NPU_BUS_DATA_MASK                          0x000ffc00
#define NPU_CRG_AUTOFSCTRL4_DEBOUNCE_BYPASS_NPU_BUS_DATA_SHIFT                      20
#define NPU_CRG_AUTOFSCTRL4_DEBOUNCE_BYPASS_NPU_BUS_DATA_MASK                       0x00100000
#define NPU_CRG_AUTOFSCTRL4_DIV_AUTO_CLK_NPU_BUS_DATA_SHIFT                         21
#define NPU_CRG_AUTOFSCTRL4_DIV_AUTO_CLK_NPU_BUS_DATA_MASK                          0x07e00000
#define NPU_CRG_AUTOFSCTRL4_AUTOFS_EN_NPU_BUS_DATA_SHIFT                            31
#define NPU_CRG_AUTOFSCTRL4_AUTOFS_EN_NPU_BUS_DATA_MASK                             0x80000000
#define NPU_CRG_AUTOFSCTRL5                         0x0a80
#define NPU_CRG_AUTOFSCTRL5_NPU_MCU_CORE_IDLE_BYPASS_SHIFT                          0
#define NPU_CRG_AUTOFSCTRL5_NPU_MCU_CORE_IDLE_BYPASS_MASK                           0x00000001
#define NPU_CRG_AUTOFSCTRL6                         0x0a84
#define NPU_CRG_AUTOFSCTRL6_DEBOUNCE_IN_MCU_CORE_SHIFT                              0
#define NPU_CRG_AUTOFSCTRL6_DEBOUNCE_IN_MCU_CORE_MASK                               0x000003ff
#define NPU_CRG_AUTOFSCTRL6_DEBOUNCE_OUT_MCU_CORE_SHIFT                             10
#define NPU_CRG_AUTOFSCTRL6_DEBOUNCE_OUT_MCU_CORE_MASK                              0x000ffc00
#define NPU_CRG_AUTOFSCTRL6_DEBOUNCE_BYPASS_MCU_CORE_SHIFT                          20
#define NPU_CRG_AUTOFSCTRL6_DEBOUNCE_BYPASS_MCU_CORE_MASK                           0x00100000
#define NPU_CRG_AUTOFSCTRL6_DIV_AUTO_CLK_MCU_CORE_SHIFT                             21
#define NPU_CRG_AUTOFSCTRL6_DIV_AUTO_CLK_MCU_CORE_MASK                              0x07e00000
#define NPU_CRG_AUTOFSCTRL6_AUTOFS_EN_MCU_CORE_SHIFT                                31
#define NPU_CRG_AUTOFSCTRL6_AUTOFS_EN_MCU_CORE_MASK                                 0x80000000
#define NPU_CRG_AUTOFSCTRL7                         0x0a88
#define NPU_CRG_AUTOFSCTRL7_CACTIVE_INV_CFG_BYPASS_SHIFT                            0
#define NPU_CRG_AUTOFSCTRL7_CACTIVE_INV_CFG_BYPASS_MASK                             0x00000001
#define NPU_CRG_AUTOFSCTRL7_NPU_BUS_TGT_NO_PENDING_TRANS_CFG_BYPASS_SHIFT           1
#define NPU_CRG_AUTOFSCTRL7_NPU_BUS_TGT_NO_PENDING_TRANS_CFG_BYPASS_MASK            0x00000002
#define NPU_CRG_AUTOFSCTRL7_NPU_BUS_INI_NO_PENDING_TRANS_CFG_BYPASS_SHIFT           2
#define NPU_CRG_AUTOFSCTRL7_NPU_BUS_INI_NO_PENDING_TRANS_CFG_BYPASS_MASK            0x00000004
#define NPU_CRG_AUTOFSCTRL7_IDLE_FLAG_AXI_BRG_BYPASS_SHIFT                          3
#define NPU_CRG_AUTOFSCTRL7_IDLE_FLAG_AXI_BRG_BYPASS_MASK                           0x00000008
#define NPU_CRG_AUTOFSCTRL7_NPU_VDSP_CORE_IDLE_CFG_BYPASS_SHIFT                     4
#define NPU_CRG_AUTOFSCTRL7_NPU_VDSP_CORE_IDLE_CFG_BYPASS_MASK                      0x00000010
#define NPU_CRG_AUTOFSCTRL8                         0x0a8c
#define NPU_CRG_AUTOFSCTRL8_DEBOUNCE_IN_NPU_BUS_CFG_SHIFT                           0
#define NPU_CRG_AUTOFSCTRL8_DEBOUNCE_IN_NPU_BUS_CFG_MASK                            0x000003ff
#define NPU_CRG_AUTOFSCTRL8_DEBOUNCE_OUT_NPU_BUS_CFG_SHIFT                          10
#define NPU_CRG_AUTOFSCTRL8_DEBOUNCE_OUT_NPU_BUS_CFG_MASK                           0x000ffc00
#define NPU_CRG_AUTOFSCTRL8_DEBOUNCE_BYPASS_NPU_BUS_CFG_SHIFT                       20
#define NPU_CRG_AUTOFSCTRL8_DEBOUNCE_BYPASS_NPU_BUS_CFG_MASK                        0x00100000
#define NPU_CRG_AUTOFSCTRL8_DIV_AUTO_CLK_NPU_BUS_CFG_SHIFT                          21
#define NPU_CRG_AUTOFSCTRL8_DIV_AUTO_CLK_NPU_BUS_CFG_MASK                           0x07e00000
#define NPU_CRG_AUTOFSCTRL8_AUTOFS_EN_NPU_BUS_CFG_SHIFT                             31
#define NPU_CRG_AUTOFSCTRL8_AUTOFS_EN_NPU_BUS_CFG_MASK                              0x80000000
#define NPU_CRG_AUTOFSCTRL9                         0x0a90
#define NPU_CRG_AUTOFSCTRL9_NPU_VIP_CORE_IDLE_BYPASS_SHIFT                          0
#define NPU_CRG_AUTOFSCTRL9_NPU_VIP_CORE_IDLE_BYPASS_MASK                           0x00000001
#define NPU_CRG_AUTOFSCTRL10                        0x0a94
#define NPU_CRG_AUTOFSCTRL10_DEBOUNCE_IN_VIP_CORE_SHIFT                             0
#define NPU_CRG_AUTOFSCTRL10_DEBOUNCE_IN_VIP_CORE_MASK                              0x000003ff
#define NPU_CRG_AUTOFSCTRL10_DEBOUNCE_OUT_VIP_CORE_SHIFT                            10
#define NPU_CRG_AUTOFSCTRL10_DEBOUNCE_OUT_VIP_CORE_MASK                             0x000ffc00
#define NPU_CRG_AUTOFSCTRL10_DEBOUNCE_BYPASS_VIP_CORE_SHIFT                         20
#define NPU_CRG_AUTOFSCTRL10_DEBOUNCE_BYPASS_VIP_CORE_MASK                          0x00100000
#define NPU_CRG_AUTOFSCTRL10_DIV_AUTO_CLK_VIP_CORE_SHIFT                            21
#define NPU_CRG_AUTOFSCTRL10_DIV_AUTO_CLK_VIP_CORE_MASK                             0x07e00000
#define NPU_CRG_AUTOFSCTRL10_AUTOFS_EN_VIP_CORE_SHIFT                               31
#define NPU_CRG_AUTOFSCTRL10_AUTOFS_EN_VIP_CORE_MASK                                0x80000000
#define NPU_CRG_AUTOFSCTRL11                        0x0a98
#define NPU_CRG_AUTOFSCTRL11_NPU_VDSP_CORE_IDLE_BYPASS_SHIFT                        0
#define NPU_CRG_AUTOFSCTRL11_NPU_VDSP_CORE_IDLE_BYPASS_MASK                         0x00000001
#define NPU_CRG_AUTOFSCTRL12                        0x0a9c
#define NPU_CRG_AUTOFSCTRL12_DEBOUNCE_IN_VDSP_CORE_SHIFT                            0
#define NPU_CRG_AUTOFSCTRL12_DEBOUNCE_IN_VDSP_CORE_MASK                             0x000003ff
#define NPU_CRG_AUTOFSCTRL12_DEBOUNCE_OUT_VDSP_CORE_SHIFT                           10
#define NPU_CRG_AUTOFSCTRL12_DEBOUNCE_OUT_VDSP_CORE_MASK                            0x000ffc00
#define NPU_CRG_AUTOFSCTRL12_DEBOUNCE_BYPASS_VDSP_CORE_SHIFT                        20
#define NPU_CRG_AUTOFSCTRL12_DEBOUNCE_BYPASS_VDSP_CORE_MASK                         0x00100000
#define NPU_CRG_AUTOFSCTRL12_DIV_AUTO_CLK_VDSP_CORE_SHIFT                           21
#define NPU_CRG_AUTOFSCTRL12_DIV_AUTO_CLK_VDSP_CORE_MASK                            0x07e00000
#define NPU_CRG_AUTOFSCTRL12_AUTOFS_EN_VDSP_CORE_SHIFT                              31
#define NPU_CRG_AUTOFSCTRL12_AUTOFS_EN_VDSP_CORE_MASK                               0x80000000
#define NPU_CRG_IDLESTATE0                          0x0ab0
#define NPU_CRG_IDLESTATE0_NPU_MCU_CORE_IDLE_SHIFT                                  0
#define NPU_CRG_IDLESTATE0_NPU_MCU_CORE_IDLE_MASK                                   0x00000001
#define NPU_CRG_IDLESTATE0_NPU_VDSP_CORE_IDLE_SHIFT                                 1
#define NPU_CRG_IDLESTATE0_NPU_VDSP_CORE_IDLE_MASK                                  0x00000002
#define NPU_CRG_IDLESTATE0_NPU_VIP_CORE_IDLE_SHIFT                                  2
#define NPU_CRG_IDLESTATE0_NPU_VIP_CORE_IDLE_MASK                                   0x00000004
#define NPU_CRG_IDLESTATE0_TCU_IDLE_FLAG_SHIFT                                      3
#define NPU_CRG_IDLESTATE0_TCU_IDLE_FLAG_MASK                                       0x00000008
#define NPU_CRG_IDLESTATE0_TCU_IDLE_FLAG_PERFSTAT_SHIFT                             4
#define NPU_CRG_IDLESTATE0_TCU_IDLE_FLAG_PERFSTAT_MASK                              0x00000010
#define NPU_CRG_IDLESTATE0_VDSP_IDLE_FLAG_PERFSTAT_SHIFT                            5
#define NPU_CRG_IDLESTATE0_VDSP_IDLE_FLAG_PERFSTAT_MASK                             0x00000020
#define NPU_CRG_IDLESTATE0_GP_IDLE_FLAG_PERFSTAT_SHIFT                              6
#define NPU_CRG_IDLESTATE0_GP_IDLE_FLAG_PERFSTAT_MASK                               0x00000040
#define NPU_CRG_IDLESTATE0_DSA_IDLE_FLAG_PERFSTAT_SHIFT                             7
#define NPU_CRG_IDLESTATE0_DSA_IDLE_FLAG_PERFSTAT_MASK                              0x00000080
#define NPU_CRG_IDLESTATE0_XIO_DMA_IDLE_FLAG_PERFSTAT_SHIFT                         8
#define NPU_CRG_IDLESTATE0_XIO_DMA_IDLE_FLAG_PERFSTAT_MASK                          0x00000100
#define NPU_CRG_IDLESTATE0_CACTIVE_NIC_INV_SHIFT                                    9
#define NPU_CRG_IDLESTATE0_CACTIVE_NIC_INV_MASK                                     0x00000200
#define NPU_CRG_IDLESTATE0_CACTIVE_NOC_INV_SHIFT                                    10
#define NPU_CRG_IDLESTATE0_CACTIVE_NOC_INV_MASK                                     0x00000400
#define NPU_CRG_IDLESTATE0_NPU_BUS_DATA_IDLE_STATE_SHIFT                            11
#define NPU_CRG_IDLESTATE0_NPU_BUS_DATA_IDLE_STATE_MASK                             0x00000800
#define NPU_CRG_IDLESTATE0_NPU_BUS_CFG_IDLE_STATE_SHIFT                             12
#define NPU_CRG_IDLESTATE0_NPU_BUS_CFG_IDLE_STATE_MASK                              0x00001000
#define NPU_CRG_IDLESTATE0_NPU_MCU_CORE_IDLE_STATE_SHIFT                            13
#define NPU_CRG_IDLESTATE0_NPU_MCU_CORE_IDLE_STATE_MASK                             0x00002000
#define NPU_CRG_IDLESTATE0_NPU_VDSP_CORE_IDLE_STATE_SHIFT                           14
#define NPU_CRG_IDLESTATE0_NPU_VDSP_CORE_IDLE_STATE_MASK                            0x00004000
#define NPU_CRG_IDLESTATE0_NPU_VIP_CORE_IDLE_STATE_SHIFT                            15
#define NPU_CRG_IDLESTATE0_NPU_VIP_CORE_IDLE_STATE_MASK                             0x00008000
#define NPU_CRG_IDLESTATE0_TOP_DMA_IDLE_FLAG_SHIFT                                  16
#define NPU_CRG_IDLESTATE0_TOP_DMA_IDLE_FLAG_MASK                                   0x00010000
#define NPU_CRG_IDLESTATE0_IDLE_FLAG_AXI_BRG_SHIFT                                  17
#define NPU_CRG_IDLESTATE0_IDLE_FLAG_AXI_BRG_MASK                                   0x00020000
#define NPU_CRG_CORE_VDCS_CTRL0                     0x0b00
#define NPU_CRG_CORE_VDCS_CTRL0_VDCS_CODE_PG_CORE_SHIFT                             0
#define NPU_CRG_CORE_VDCS_CTRL0_VDCS_CODE_PG_CORE_MASK                              0x00000003
#define NPU_CRG_CORE_VDCS_CTRL0_VDCS_CODE_RST_CORE_SHIFT                            2
#define NPU_CRG_CORE_VDCS_CTRL0_VDCS_CODE_RST_CORE_MASK                             0x0000000c
#define NPU_CRG_CORE_VDCS_CTRL0_DROP_EXIT_HOLD_CORE_SHIFT                           4
#define NPU_CRG_CORE_VDCS_CTRL0_DROP_EXIT_HOLD_CORE_MASK                            0x00003ff0
#define NPU_CRG_CORE_VDCS_CTRL0_CPU_WFX_BPS_CORE_SHIFT                              14
#define NPU_CRG_CORE_VDCS_CTRL0_CPU_WFX_BPS_CORE_MASK                               0x00004000
#define NPU_CRG_CORE_VDCS_CTRL0_L2_IDLE_BPS_CORE_SHIFT                              15
#define NPU_CRG_CORE_VDCS_CTRL0_L2_IDLE_BPS_CORE_MASK                               0x00008000
#define NPU_CRG_CORE_VDCS_CTRL0_DVFS_EN_BPS_CORE_SHIFT                              16
#define NPU_CRG_CORE_VDCS_CTRL0_DVFS_EN_BPS_CORE_MASK                               0x00010000
#define NPU_CRG_CORE_VDCS_CTRL0_CPU_LOAD_BPS_CORE_SHIFT                             17
#define NPU_CRG_CORE_VDCS_CTRL0_CPU_LOAD_BPS_CORE_MASK                              0x00020000
#define NPU_CRG_CORE_VDCS_CTRL0_VBAT_PROTECT_BPS_CORE_SHIFT                         18
#define NPU_CRG_CORE_VDCS_CTRL0_VBAT_PROTECT_BPS_CORE_MASK                          0x00040000
#define NPU_CRG_CORE_VDCS_CTRL0_CCD_EN_BPS_CORE_SHIFT                               19
#define NPU_CRG_CORE_VDCS_CTRL0_CCD_EN_BPS_CORE_MASK                                0x00080000
#define NPU_CRG_CORE_VDCS_CTRL0_RESERVED0_CORE_SHIFT                                20
#define NPU_CRG_CORE_VDCS_CTRL0_RESERVED0_CORE_MASK                                 0xfff00000
#define NPU_CRG_CORE_VDCS_CTRL1                     0x0b04
#define NPU_CRG_CORE_VDCS_CTRL1_VDCS_BPS_CORE_SHIFT                                 0
#define NPU_CRG_CORE_VDCS_CTRL1_VDCS_BPS_CORE_MASK                                  0x00000001
#define NPU_CRG_CORE_VDCS_CTRL1_VDCS_EN_HOLD_CORE_SHIFT                             1
#define NPU_CRG_CORE_VDCS_CTRL1_VDCS_EN_HOLD_CORE_MASK                              0x000007fe
#define NPU_CRG_CORE_VDCS_CTRL1_GT_CLK_VDCS_BPS_CORE_SHIFT                          11
#define NPU_CRG_CORE_VDCS_CTRL1_GT_CLK_VDCS_BPS_CORE_MASK                           0x00000800
#define NPU_CRG_CORE_VDCS_CTRL1_DIG_VBAT_DEN_CORE_SHIFT                             12
#define NPU_CRG_CORE_VDCS_CTRL1_DIG_VBAT_DEN_CORE_MASK                              0x00007000
#define NPU_CRG_CORE_VDCS_CTRL1_DIG_VBAT_NUM_CORE_SHIFT                             15
#define NPU_CRG_CORE_VDCS_CTRL1_DIG_VBAT_NUM_CORE_MASK                              0x00038000
#define NPU_CRG_CORE_VDCS_CTRL1_DIG_L2_DEN_CORE_SHIFT                               18
#define NPU_CRG_CORE_VDCS_CTRL1_DIG_L2_DEN_CORE_MASK                                0x001c0000
#define NPU_CRG_CORE_VDCS_CTRL1_DIG_L2_NUM_CORE_SHIFT                               21
#define NPU_CRG_CORE_VDCS_CTRL1_DIG_L2_NUM_CORE_MASK                                0x00e00000
#define NPU_CRG_CORE_VDCS_CTRL1_WFX_CORE_NUM_CORE_SHIFT                             24
#define NPU_CRG_CORE_VDCS_CTRL1_WFX_CORE_NUM_CORE_MASK                              0x03000000
#define NPU_CRG_CORE_VDCS_CTRL1_CS_LOCK_BPS_CORE_SHIFT                              26
#define NPU_CRG_CORE_VDCS_CTRL1_CS_LOCK_BPS_CORE_MASK                               0x04000000
#define NPU_CRG_CORE_VDCS_CTRL1_RESERVED1_CORE_SHIFT                                27
#define NPU_CRG_CORE_VDCS_CTRL1_RESERVED1_CORE_MASK                                 0xf8000000
#define NPU_CRG_CORE_VDCS_CTRL2                     0x0b08
#define NPU_CRG_CORE_VDCS_CTRL2_CS_DELAYLINE_RES_CORE_SHIFT                         0
#define NPU_CRG_CORE_VDCS_CTRL2_CS_DELAYLINE_RES_CORE_MASK                          0x00000003
#define NPU_CRG_CORE_VDCS_CTRL2_CS_FILTERSEL_CORE_SHIFT                             2
#define NPU_CRG_CORE_VDCS_CTRL2_CS_FILTERSEL_CORE_MASK                              0x0000001c
#define NPU_CRG_CORE_VDCS_CTRL2_RESERVED8_CORE_SHIFT                                5
#define NPU_CRG_CORE_VDCS_CTRL2_RESERVED8_CORE_MASK                                 0x00000060
#define NPU_CRG_CORE_VDCS_CTRL2_CS_LOCKDET_COUNTER_CORE_SHIFT                       7
#define NPU_CRG_CORE_VDCS_CTRL2_CS_LOCKDET_COUNTER_CORE_MASK                        0x00000180
#define NPU_CRG_CORE_VDCS_CTRL2_CS_BIASSEL_CORE_SHIFT                               9
#define NPU_CRG_CORE_VDCS_CTRL2_CS_BIASSEL_CORE_MASK                                0x00000600
#define NPU_CRG_CORE_VDCS_CTRL2_CS_ICPSEL_CORE_SHIFT                                11
#define NPU_CRG_CORE_VDCS_CTRL2_CS_ICPSEL_CORE_MASK                                 0x00001800
#define NPU_CRG_CORE_VDCS_CTRL2_CS_FREQSEL_CORE_SHIFT                               13
#define NPU_CRG_CORE_VDCS_CTRL2_CS_FREQSEL_CORE_MASK                                0x0000e000
#define NPU_CRG_CORE_VDCS_CTRL2_LOCKDET_DELAY_CORE_SHIFT                            16
#define NPU_CRG_CORE_VDCS_CTRL2_LOCKDET_DELAY_CORE_MASK                             0x00030000
#define NPU_CRG_CORE_VDCS_CTRL2_LOCKDET_PATH_CORE_SHIFT                             18
#define NPU_CRG_CORE_VDCS_CTRL2_LOCKDET_PATH_CORE_MASK                              0x000c0000
#define NPU_CRG_CORE_VDCS_CTRL2_RESERVED2_CORE_SHIFT                                20
#define NPU_CRG_CORE_VDCS_CTRL2_RESERVED2_CORE_MASK                                 0xfff00000
#define NPU_CRG_CORE_VDCS_CTRL3                     0x0b0c
#define NPU_CRG_CORE_VDCS_CTRL4                     0x0b10
#define NPU_CRG_CORE_VDCS_CTRL4_VD_CLK_DRIVE_EN_CORE_SHIFT                          0
#define NPU_CRG_CORE_VDCS_CTRL4_VD_CLK_DRIVE_EN_CORE_MASK                           0x00000001
#define NPU_CRG_CORE_VDCS_CTRL4_RESERVED4_CORE_SHIFT                                1
#define NPU_CRG_CORE_VDCS_CTRL4_RESERVED4_CORE_MASK                                 0xfffffffe
#define NPU_CRG_CORE_VDCS_CTRL5                     0x0b14
#define NPU_CRG_CORE_VDCS_CTRL6                     0x0b18
#define NPU_CRG_CORE_VDCS_CTRL6_VDCS_CODE_CDL_CORE_SHIFT                            0
#define NPU_CRG_CORE_VDCS_CTRL6_VDCS_CODE_CDL_CORE_MASK                             0x0000003f
#define NPU_CRG_CORE_VDCS_CTRL6_VDCS_CODE_FDL_CORE_SHIFT                            6
#define NPU_CRG_CORE_VDCS_CTRL6_VDCS_CODE_FDL_CORE_MASK                             0x000001c0
#define NPU_CRG_CORE_VDCS_CTRL6_VDCS_PATH_SEL_CORE_SHIFT                            9
#define NPU_CRG_CORE_VDCS_CTRL6_VDCS_PATH_SEL_CORE_MASK                             0x00000600
#define NPU_CRG_CORE_VDCS_CTRL6_VDCS_THRESHOLD_CORE_SHIFT                           11
#define NPU_CRG_CORE_VDCS_CTRL6_VDCS_THRESHOLD_CORE_MASK                            0x0000f800
#define NPU_CRG_CORE_VDCS_CTRL7                     0x0b1c
#define NPU_CRG_CORE_VDCS_CTRL7_VD_DIV_EN_CORE_SHIFT                                0
#define NPU_CRG_CORE_VDCS_CTRL7_VD_DIV_EN_CORE_MASK                                 0x00000001
#define NPU_CRG_CORE_VDCS_CTRL7_STRETCH_ACTIVE_EN_CORE_SHIFT                        1
#define NPU_CRG_CORE_VDCS_CTRL7_STRETCH_ACTIVE_EN_CORE_MASK                         0x00000002
#define NPU_CRG_CORE_VDCS_CTRL7_STRETCH_PASSIVE_EN_CORE_SHIFT                       2
#define NPU_CRG_CORE_VDCS_CTRL7_STRETCH_PASSIVE_EN_CORE_MASK                        0x00000004
#define NPU_CRG_CORE_VDCS_CTRL7_RESERVED7_CORE_SHIFT                                3
#define NPU_CRG_CORE_VDCS_CTRL7_RESERVED7_CORE_MASK                                 0x0000fff8
#define NPU_CRG_CORE_VDCS_STATE0                    0x0b20
#define NPU_CRG_CORE_VDCS_STATE0_VD_OUT_CORE_SHIFT                                  0
#define NPU_CRG_CORE_VDCS_STATE0_VD_OUT_CORE_MASK                                   0x0000003f
#define NPU_CRG_CORE_VDCS_STATE0_CLOCK_STRETCH_FLAG_CORE_SHIFT                      6
#define NPU_CRG_CORE_VDCS_STATE0_CLOCK_STRETCH_FLAG_CORE_MASK                       0x000000c0
#define NPU_CRG_CORE_VDCS_STATE0_DROP_DETECT_CORE_SHIFT                             8
#define NPU_CRG_CORE_VDCS_STATE0_DROP_DETECT_CORE_MASK                              0x00000100
#define NPU_CRG_CORE_VDCS_STATE0_VDCS_LOCK_CORE_SHIFT                               9
#define NPU_CRG_CORE_VDCS_STATE0_VDCS_LOCK_CORE_MASK                                0x00000200
#define NPU_CRG_CORE_VDCS_STATE0_VDCS_OVER_CORE_SHIFT                               10
#define NPU_CRG_CORE_VDCS_STATE0_VDCS_OVER_CORE_MASK                                0x00000400
#define NPU_CRG_CORE_VDCS_STATE0_VDCS_UNDER_CORE_SHIFT                              11
#define NPU_CRG_CORE_VDCS_STATE0_VDCS_UNDER_CORE_MASK                               0x00000800
#define NPU_CRG_CORE_VDCS_STATE0_GT_CLK_VDCS_CORE_SHIFT                             12
#define NPU_CRG_CORE_VDCS_STATE0_GT_CLK_VDCS_CORE_MASK                              0x00001000
#define NPU_CRG_CORE_VDCS_STATE0_RESERVED_STATE0_CORE_SHIFT                         13
#define NPU_CRG_CORE_VDCS_STATE0_RESERVED_STATE0_CORE_MASK                          0xffffe000
#define NPU_CRG_CORE_VDCS_STATE1                    0x0b24
#define NPU_CRG_CORE_VDCS_STATE1_WFX_STATE_CORE_SHIFT                               0
#define NPU_CRG_CORE_VDCS_STATE1_WFX_STATE_CORE_MASK                                0x0000000f
#define NPU_CRG_CORE_VDCS_STATE1_L2_IDLE_STATE_CORE_SHIFT                           4
#define NPU_CRG_CORE_VDCS_STATE1_L2_IDLE_STATE_CORE_MASK                            0x00000010
#define NPU_CRG_CORE_VDCS_STATE1_DVFS_EN_STATE_CORE_SHIFT                           5
#define NPU_CRG_CORE_VDCS_STATE1_DVFS_EN_STATE_CORE_MASK                            0x00000020
#define NPU_CRG_CORE_VDCS_STATE1_CPU_LOAD_STATE_CORE_SHIFT                          6
#define NPU_CRG_CORE_VDCS_STATE1_CPU_LOAD_STATE_CORE_MASK                           0x00000040
#define NPU_CRG_CORE_VDCS_STATE1_CORE_IDLE_STATE_CORE_SHIFT                         7
#define NPU_CRG_CORE_VDCS_STATE1_CORE_IDLE_STATE_CORE_MASK                          0x00000080
#define NPU_CRG_CORE_VDCS_STATE1_VBAT_PROTECT_STATE_CORE_SHIFT                      8
#define NPU_CRG_CORE_VDCS_STATE1_VBAT_PROTECT_STATE_CORE_MASK                       0x00000100
#define NPU_CRG_CORE_VDCS_STATE1_CCD_EN_STATE_CORE_SHIFT                            9
#define NPU_CRG_CORE_VDCS_STATE1_CCD_EN_STATE_CORE_MASK                             0x00000200
#define NPU_CRG_CORE_VDCS_STATE1_RESERVED_STATE1_CORE_SHIFT                         10
#define NPU_CRG_CORE_VDCS_STATE1_RESERVED_STATE1_CORE_MASK                          0xfffffc00
#define NPU_CRG_VDSP_VDCS_CTRL0                     0x0b40
#define NPU_CRG_VDSP_VDCS_CTRL0_VDCS_CODE_PG_VDSP_SHIFT                             0
#define NPU_CRG_VDSP_VDCS_CTRL0_VDCS_CODE_PG_VDSP_MASK                              0x00000003
#define NPU_CRG_VDSP_VDCS_CTRL0_VDCS_CODE_RST_VDSP_SHIFT                            2
#define NPU_CRG_VDSP_VDCS_CTRL0_VDCS_CODE_RST_VDSP_MASK                             0x0000000c
#define NPU_CRG_VDSP_VDCS_CTRL0_DROP_EXIT_HOLD_VDSP_SHIFT                           4
#define NPU_CRG_VDSP_VDCS_CTRL0_DROP_EXIT_HOLD_VDSP_MASK                            0x00003ff0
#define NPU_CRG_VDSP_VDCS_CTRL0_CPU_WFX_BPS_VDSP_SHIFT                              14
#define NPU_CRG_VDSP_VDCS_CTRL0_CPU_WFX_BPS_VDSP_MASK                               0x00004000
#define NPU_CRG_VDSP_VDCS_CTRL0_L2_IDLE_BPS_VDSP_SHIFT                              15
#define NPU_CRG_VDSP_VDCS_CTRL0_L2_IDLE_BPS_VDSP_MASK                               0x00008000
#define NPU_CRG_VDSP_VDCS_CTRL0_DVFS_EN_BPS_VDSP_SHIFT                              16
#define NPU_CRG_VDSP_VDCS_CTRL0_DVFS_EN_BPS_VDSP_MASK                               0x00010000
#define NPU_CRG_VDSP_VDCS_CTRL0_CPU_LOAD_BPS_VDSP_SHIFT                             17
#define NPU_CRG_VDSP_VDCS_CTRL0_CPU_LOAD_BPS_VDSP_MASK                              0x00020000
#define NPU_CRG_VDSP_VDCS_CTRL0_VBAT_PROTECT_BPS_VDSP_SHIFT                         18
#define NPU_CRG_VDSP_VDCS_CTRL0_VBAT_PROTECT_BPS_VDSP_MASK                          0x00040000
#define NPU_CRG_VDSP_VDCS_CTRL0_CCD_EN_BPS_VDSP_SHIFT                               19
#define NPU_CRG_VDSP_VDCS_CTRL0_CCD_EN_BPS_VDSP_MASK                                0x00080000
#define NPU_CRG_VDSP_VDCS_CTRL0_RESERVED0_VDSP_SHIFT                                20
#define NPU_CRG_VDSP_VDCS_CTRL0_RESERVED0_VDSP_MASK                                 0xfff00000
#define NPU_CRG_VDSP_VDCS_CTRL1                     0x0b44
#define NPU_CRG_VDSP_VDCS_CTRL1_VDCS_BPS_VDSP_SHIFT                                 0
#define NPU_CRG_VDSP_VDCS_CTRL1_VDCS_BPS_VDSP_MASK                                  0x00000001
#define NPU_CRG_VDSP_VDCS_CTRL1_VDCS_EN_HOLD_VDSP_SHIFT                             1
#define NPU_CRG_VDSP_VDCS_CTRL1_VDCS_EN_HOLD_VDSP_MASK                              0x000007fe
#define NPU_CRG_VDSP_VDCS_CTRL1_GT_CLK_VDCS_BPS_VDSP_SHIFT                          11
#define NPU_CRG_VDSP_VDCS_CTRL1_GT_CLK_VDCS_BPS_VDSP_MASK                           0x00000800
#define NPU_CRG_VDSP_VDCS_CTRL1_DIG_VBAT_DEN_VDSP_SHIFT                             12
#define NPU_CRG_VDSP_VDCS_CTRL1_DIG_VBAT_DEN_VDSP_MASK                              0x00007000
#define NPU_CRG_VDSP_VDCS_CTRL1_DIG_VBAT_NUM_VDSP_SHIFT                             15
#define NPU_CRG_VDSP_VDCS_CTRL1_DIG_VBAT_NUM_VDSP_MASK                              0x00038000
#define NPU_CRG_VDSP_VDCS_CTRL1_DIG_L2_DEN_VDSP_SHIFT                               18
#define NPU_CRG_VDSP_VDCS_CTRL1_DIG_L2_DEN_VDSP_MASK                                0x001c0000
#define NPU_CRG_VDSP_VDCS_CTRL1_DIG_L2_NUM_VDSP_SHIFT                               21
#define NPU_CRG_VDSP_VDCS_CTRL1_DIG_L2_NUM_VDSP_MASK                                0x00e00000
#define NPU_CRG_VDSP_VDCS_CTRL1_WFX_VDSP_NUM_VDSP_SHIFT                             24
#define NPU_CRG_VDSP_VDCS_CTRL1_WFX_VDSP_NUM_VDSP_MASK                              0x03000000
#define NPU_CRG_VDSP_VDCS_CTRL1_CS_LOCK_BPS_VDSP_SHIFT                              26
#define NPU_CRG_VDSP_VDCS_CTRL1_CS_LOCK_BPS_VDSP_MASK                               0x04000000
#define NPU_CRG_VDSP_VDCS_CTRL1_RESERVED1_VDSP_SHIFT                                27
#define NPU_CRG_VDSP_VDCS_CTRL1_RESERVED1_VDSP_MASK                                 0xf8000000
#define NPU_CRG_VDSP_VDCS_CTRL2                     0x0b48
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_DELAYLINE_RES_VDSP_SHIFT                         0
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_DELAYLINE_RES_VDSP_MASK                          0x00000003
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_FILTERSEL_VDSP_SHIFT                             2
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_FILTERSEL_VDSP_MASK                              0x0000001c
#define NPU_CRG_VDSP_VDCS_CTRL2_RESERVED8_VDSP_SHIFT                                5
#define NPU_CRG_VDSP_VDCS_CTRL2_RESERVED8_VDSP_MASK                                 0x00000060
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_LOCKDET_COUNTER_VDSP_SHIFT                       7
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_LOCKDET_COUNTER_VDSP_MASK                        0x00000180
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_BIASSEL_VDSP_SHIFT                               9
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_BIASSEL_VDSP_MASK                                0x00000600
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_ICPSEL_VDSP_SHIFT                                11
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_ICPSEL_VDSP_MASK                                 0x00001800
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_FREQSEL_VDSP_SHIFT                               13
#define NPU_CRG_VDSP_VDCS_CTRL2_CS_FREQSEL_VDSP_MASK                                0x0000e000
#define NPU_CRG_VDSP_VDCS_CTRL2_LOCKDET_DELAY_VDSP_SHIFT                            16
#define NPU_CRG_VDSP_VDCS_CTRL2_LOCKDET_DELAY_VDSP_MASK                             0x00030000
#define NPU_CRG_VDSP_VDCS_CTRL2_LOCKDET_PATH_VDSP_SHIFT                             18
#define NPU_CRG_VDSP_VDCS_CTRL2_LOCKDET_PATH_VDSP_MASK                              0x000c0000
#define NPU_CRG_VDSP_VDCS_CTRL2_RESERVED2_VDSP_SHIFT                                20
#define NPU_CRG_VDSP_VDCS_CTRL2_RESERVED2_VDSP_MASK                                 0xfff00000
#define NPU_CRG_VDSP_VDCS_CTRL3                     0x0b4c
#define NPU_CRG_VDSP_VDCS_CTRL4                     0x0b50
#define NPU_CRG_VDSP_VDCS_CTRL4_VD_CLK_DRIVE_EN_VDSP_SHIFT                          0
#define NPU_CRG_VDSP_VDCS_CTRL4_VD_CLK_DRIVE_EN_VDSP_MASK                           0x00000001
#define NPU_CRG_VDSP_VDCS_CTRL4_RESERVED4_VDSP_SHIFT                                1
#define NPU_CRG_VDSP_VDCS_CTRL4_RESERVED4_VDSP_MASK                                 0xfffffffe
#define NPU_CRG_VDSP_VDCS_CTRL5                     0x0b54
#define NPU_CRG_VDSP_VDCS_CTRL6                     0x0b58
#define NPU_CRG_VDSP_VDCS_CTRL6_VDCS_CODE_CDL_VDSP_SHIFT                            0
#define NPU_CRG_VDSP_VDCS_CTRL6_VDCS_CODE_CDL_VDSP_MASK                             0x0000003f
#define NPU_CRG_VDSP_VDCS_CTRL6_VDCS_CODE_FDL_VDSP_SHIFT                            6
#define NPU_CRG_VDSP_VDCS_CTRL6_VDCS_CODE_FDL_VDSP_MASK                             0x000001c0
#define NPU_CRG_VDSP_VDCS_CTRL6_VDCS_PATH_SEL_VDSP_SHIFT                            9
#define NPU_CRG_VDSP_VDCS_CTRL6_VDCS_PATH_SEL_VDSP_MASK                             0x00000600
#define NPU_CRG_VDSP_VDCS_CTRL6_VDCS_THRESHOLD_VDSP_SHIFT                           11
#define NPU_CRG_VDSP_VDCS_CTRL6_VDCS_THRESHOLD_VDSP_MASK                            0x0000f800
#define NPU_CRG_VDSP_VDCS_CTRL7                     0x0b5c
#define NPU_CRG_VDSP_VDCS_CTRL7_VD_DIV_EN_VDSP_SHIFT                                0
#define NPU_CRG_VDSP_VDCS_CTRL7_VD_DIV_EN_VDSP_MASK                                 0x00000001
#define NPU_CRG_VDSP_VDCS_CTRL7_STRETCH_ACTIVE_EN_VDSP_SHIFT                        1
#define NPU_CRG_VDSP_VDCS_CTRL7_STRETCH_ACTIVE_EN_VDSP_MASK                         0x00000002
#define NPU_CRG_VDSP_VDCS_CTRL7_STRETCH_PASSIVE_EN_VDSP_SHIFT                       2
#define NPU_CRG_VDSP_VDCS_CTRL7_STRETCH_PASSIVE_EN_VDSP_MASK                        0x00000004
#define NPU_CRG_VDSP_VDCS_CTRL7_RESERVED7_VDSP_SHIFT                                3
#define NPU_CRG_VDSP_VDCS_CTRL7_RESERVED7_VDSP_MASK                                 0x0000fff8
#define NPU_CRG_VDSP_VDCS_STATE0                    0x0b60
#define NPU_CRG_VDSP_VDCS_STATE0_VD_OUT_VDSP_SHIFT                                  0
#define NPU_CRG_VDSP_VDCS_STATE0_VD_OUT_VDSP_MASK                                   0x0000003f
#define NPU_CRG_VDSP_VDCS_STATE0_CLOCK_STRETCH_FLAG_VDSP_SHIFT                      6
#define NPU_CRG_VDSP_VDCS_STATE0_CLOCK_STRETCH_FLAG_VDSP_MASK                       0x000000c0
#define NPU_CRG_VDSP_VDCS_STATE0_DROP_DETECT_VDSP_SHIFT                             8
#define NPU_CRG_VDSP_VDCS_STATE0_DROP_DETECT_VDSP_MASK                              0x00000100
#define NPU_CRG_VDSP_VDCS_STATE0_VDCS_LOCK_VDSP_SHIFT                               9
#define NPU_CRG_VDSP_VDCS_STATE0_VDCS_LOCK_VDSP_MASK                                0x00000200
#define NPU_CRG_VDSP_VDCS_STATE0_VDCS_OVER_VDSP_SHIFT                               10
#define NPU_CRG_VDSP_VDCS_STATE0_VDCS_OVER_VDSP_MASK                                0x00000400
#define NPU_CRG_VDSP_VDCS_STATE0_VDCS_UNDER_VDSP_SHIFT                              11
#define NPU_CRG_VDSP_VDCS_STATE0_VDCS_UNDER_VDSP_MASK                               0x00000800
#define NPU_CRG_VDSP_VDCS_STATE0_GT_CLK_VDCS_VDSP_SHIFT                             12
#define NPU_CRG_VDSP_VDCS_STATE0_GT_CLK_VDCS_VDSP_MASK                              0x00001000
#define NPU_CRG_VDSP_VDCS_STATE0_RESERVED_STATE0_VDSP_SHIFT                         13
#define NPU_CRG_VDSP_VDCS_STATE0_RESERVED_STATE0_VDSP_MASK                          0xffffe000
#define NPU_CRG_VDSP_VDCS_STATE1                    0x0b64
#define NPU_CRG_VDSP_VDCS_STATE1_WFX_STATE_VDSP_SHIFT                               0
#define NPU_CRG_VDSP_VDCS_STATE1_WFX_STATE_VDSP_MASK                                0x0000000f
#define NPU_CRG_VDSP_VDCS_STATE1_L2_IDLE_STATE_VDSP_SHIFT                           4
#define NPU_CRG_VDSP_VDCS_STATE1_L2_IDLE_STATE_VDSP_MASK                            0x00000010
#define NPU_CRG_VDSP_VDCS_STATE1_DVFS_EN_STATE_VDSP_SHIFT                           5
#define NPU_CRG_VDSP_VDCS_STATE1_DVFS_EN_STATE_VDSP_MASK                            0x00000020
#define NPU_CRG_VDSP_VDCS_STATE1_CPU_LOAD_STATE_VDSP_SHIFT                          6
#define NPU_CRG_VDSP_VDCS_STATE1_CPU_LOAD_STATE_VDSP_MASK                           0x00000040
#define NPU_CRG_VDSP_VDCS_STATE1_CORE_IDLE_STATE_VDSP_SHIFT                         7
#define NPU_CRG_VDSP_VDCS_STATE1_CORE_IDLE_STATE_VDSP_MASK                          0x00000080
#define NPU_CRG_VDSP_VDCS_STATE1_VBAT_PROTECT_STATE_VDSP_SHIFT                      8
#define NPU_CRG_VDSP_VDCS_STATE1_VBAT_PROTECT_STATE_VDSP_MASK                       0x00000100
#define NPU_CRG_VDSP_VDCS_STATE1_CCD_EN_STATE_VDSP_SHIFT                            9
#define NPU_CRG_VDSP_VDCS_STATE1_CCD_EN_STATE_VDSP_MASK                             0x00000200
#define NPU_CRG_VDSP_VDCS_STATE1_RESERVED_STATE1_VDSP_SHIFT                         10
#define NPU_CRG_VDSP_VDCS_STATE1_RESERVED_STATE1_VDSP_MASK                          0xfffffc00
#define NPU_CRG_REGFILE_CGBYPASS                    0x0b68
#define NPU_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                             0
#define NPU_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                              0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_vote_en :  8;
        unsigned int reserved_0           :  8;
        unsigned int _bm_                 : 16;
    } reg;
}npu_crg_npu_core_pll_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_lock :  1;
        unsigned int reserved_0        : 31;
    } reg;
}npu_crg_npu_core_pll_lock_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_core  :  1;
        unsigned int intr_mask_pll_unlock_core   :  1;
        unsigned int intr_mask_vote_req_err_core :  1;
        unsigned int reserved_0                  : 29;
    } reg;
}npu_crg_npu_core_pll_intr_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_core  :  1;
        unsigned int intr_clear_pll_unlock_core   :  1;
        unsigned int intr_clear_vote_req_err_core :  1;
        unsigned int reserved_0                   : 13;
        unsigned int _bm_                         : 16;
    } reg;
}npu_crg_npu_core_pll_intr_clear_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pll_ini_err_status_core  :  1;
        unsigned int intr_pll_unlock_status_core   :  1;
        unsigned int intr_vote_req_err_status_core :  1;
        unsigned int reserved_0                    : 29;
    } reg;
}npu_crg_npu_core_pll_intr_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pll_ini_err_status_core_msk  :  1;
        unsigned int intr_pll_unlock_status_core_msk   :  1;
        unsigned int intr_vote_req_err_status_core_msk :  1;
        unsigned int reserved_0                        : 29;
    } reg;
}npu_crg_npu_core_pll_intr_status_msk_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_vote_en :  8;
        unsigned int reserved_0           :  8;
        unsigned int _bm_                 : 16;
    } reg;
}npu_crg_npu_vdsp_pll_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_lock :  1;
        unsigned int reserved_0        : 31;
    } reg;
}npu_crg_npu_vdsp_pll_lock_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_vdsp  :  1;
        unsigned int intr_mask_pll_unlock_vdsp   :  1;
        unsigned int intr_mask_vote_req_err_vdsp :  1;
        unsigned int reserved_0                  : 29;
    } reg;
}npu_crg_npu_vdsp_pll_intr_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_vdsp  :  1;
        unsigned int intr_clear_pll_unlock_vdsp   :  1;
        unsigned int intr_clear_vote_req_err_vdsp :  1;
        unsigned int reserved_0                   : 13;
        unsigned int _bm_                         : 16;
    } reg;
}npu_crg_npu_vdsp_pll_intr_clear_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pll_ini_err_status_vdsp  :  1;
        unsigned int intr_pll_unlock_status_vdsp   :  1;
        unsigned int intr_vote_req_err_status_vdsp :  1;
        unsigned int reserved_0                    : 29;
    } reg;
}npu_crg_npu_vdsp_pll_intr_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pll_ini_err_status_vdsp_msk  :  1;
        unsigned int intr_pll_unlock_status_vdsp_msk   :  1;
        unsigned int intr_vote_req_err_status_vdsp_msk :  1;
        unsigned int reserved_0                        : 29;
    } reg;
}npu_crg_npu_vdsp_pll_intr_status_msk_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_npu_bus_data : 16;
        unsigned int _bm_                       : 16;
    } reg;
}npu_crg_autofsbypass0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_npu_mcu_core : 16;
        unsigned int _bm_                       : 16;
    } reg;
}npu_crg_autofsbypass1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_npu_bus_cfg : 16;
        unsigned int _bm_                      : 16;
    } reg;
}npu_crg_autofsbypass2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_npu_vip_core : 16;
        unsigned int _bm_                       : 16;
    } reg;
}npu_crg_autofsbypass3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_npu_vdsp_core : 16;
        unsigned int _bm_                        : 16;
    } reg;
}npu_crg_autofsbypass4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_aclk_npu_tbu7     :  1;
        unsigned int gt_aclk_npu_tbu6     :  1;
        unsigned int gt_aclk_npu_tbu5     :  1;
        unsigned int gt_aclk_npu_tbu4     :  1;
        unsigned int gt_aclk_npu_tbu3     :  1;
        unsigned int gt_aclk_npu_tbu2     :  1;
        unsigned int gt_aclk_npu_tbu1     :  1;
        unsigned int gt_aclk_npu_tbu0     :  1;
        unsigned int reserved_0           :  2;
        unsigned int gt_clk_npu_vdsp_apb  :  1;
        unsigned int gt_clk_npu_m85_apb   :  1;
        unsigned int gt_clk_npu_m85_atb   :  1;
        unsigned int gt_clk_npu_m85_ts    :  1;
        unsigned int gt_clk_npu_vdsp_ts   :  1;
        unsigned int gt_clk_npu_vdsp_atb  :  1;
        unsigned int gt_clk_npu_debug_atb :  1;
        unsigned int gt_pclk_npu_debug    :  1;
        unsigned int gt_clk_npu_ccd       :  1;
        unsigned int gt_aclk_npu_dmmu1    :  1;
        unsigned int gt_aclk_npu_dmmu0    :  1;
        unsigned int gt_aclk_npu_tcu_tpc  :  1;
        unsigned int reserved_1           :  4;
        unsigned int gt_clk_npu_core3     :  1;
        unsigned int gt_clk_npu_core5     :  1;
        unsigned int gt_clk_npu_core2     :  1;
        unsigned int gt_clk_npu_core4     :  1;
        unsigned int gt_clk_npu_core1     :  1;
        unsigned int gt_clk_npu_core0     :  1;
    } reg;
}npu_crg_clock_cg_0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_aclk_npu_tbu7     :  1;
        unsigned int gt_aclk_npu_tbu6     :  1;
        unsigned int gt_aclk_npu_tbu5     :  1;
        unsigned int gt_aclk_npu_tbu4     :  1;
        unsigned int gt_aclk_npu_tbu3     :  1;
        unsigned int gt_aclk_npu_tbu2     :  1;
        unsigned int gt_aclk_npu_tbu1     :  1;
        unsigned int gt_aclk_npu_tbu0     :  1;
        unsigned int reserved_0           :  2;
        unsigned int gt_clk_npu_vdsp_apb  :  1;
        unsigned int gt_clk_npu_m85_apb   :  1;
        unsigned int gt_clk_npu_m85_atb   :  1;
        unsigned int gt_clk_npu_m85_ts    :  1;
        unsigned int gt_clk_npu_vdsp_ts   :  1;
        unsigned int gt_clk_npu_vdsp_atb  :  1;
        unsigned int gt_clk_npu_debug_atb :  1;
        unsigned int gt_pclk_npu_debug    :  1;
        unsigned int gt_clk_npu_ccd       :  1;
        unsigned int gt_aclk_npu_dmmu1    :  1;
        unsigned int gt_aclk_npu_dmmu0    :  1;
        unsigned int gt_aclk_npu_tcu_tpc  :  1;
        unsigned int reserved_1           :  4;
        unsigned int gt_clk_npu_core3     :  1;
        unsigned int gt_clk_npu_core5     :  1;
        unsigned int gt_clk_npu_core2     :  1;
        unsigned int gt_clk_npu_core4     :  1;
        unsigned int gt_clk_npu_core1     :  1;
        unsigned int gt_clk_npu_core0     :  1;
    } reg;
}npu_crg_clock_cg_0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_aclk_npu_tbu7     :  1;
        unsigned int gt_aclk_npu_tbu6     :  1;
        unsigned int gt_aclk_npu_tbu5     :  1;
        unsigned int gt_aclk_npu_tbu4     :  1;
        unsigned int gt_aclk_npu_tbu3     :  1;
        unsigned int gt_aclk_npu_tbu2     :  1;
        unsigned int gt_aclk_npu_tbu1     :  1;
        unsigned int gt_aclk_npu_tbu0     :  1;
        unsigned int reserved_0           :  2;
        unsigned int gt_clk_npu_vdsp_apb  :  1;
        unsigned int gt_clk_npu_m85_apb   :  1;
        unsigned int gt_clk_npu_m85_atb   :  1;
        unsigned int gt_clk_npu_m85_ts    :  1;
        unsigned int gt_clk_npu_vdsp_ts   :  1;
        unsigned int gt_clk_npu_vdsp_atb  :  1;
        unsigned int gt_clk_npu_debug_atb :  1;
        unsigned int gt_pclk_npu_debug    :  1;
        unsigned int gt_clk_npu_ccd       :  1;
        unsigned int gt_aclk_npu_dmmu1    :  1;
        unsigned int gt_aclk_npu_dmmu0    :  1;
        unsigned int gt_aclk_npu_tcu_tpc  :  1;
        unsigned int reserved_1           :  4;
        unsigned int gt_clk_npu_core3     :  1;
        unsigned int gt_clk_npu_core5     :  1;
        unsigned int gt_clk_npu_core2     :  1;
        unsigned int gt_clk_npu_core4     :  1;
        unsigned int gt_clk_npu_core1     :  1;
        unsigned int gt_clk_npu_core0     :  1;
    } reg;
}npu_crg_clock_cg_0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_aclk_npu_tbu7     :  1;
        unsigned int st_aclk_npu_tbu6     :  1;
        unsigned int st_aclk_npu_tbu5     :  1;
        unsigned int st_aclk_npu_tbu4     :  1;
        unsigned int st_aclk_npu_tbu3     :  1;
        unsigned int st_aclk_npu_tbu2     :  1;
        unsigned int st_aclk_npu_tbu1     :  1;
        unsigned int st_aclk_npu_tbu0     :  1;
        unsigned int reserved_0           :  2;
        unsigned int st_clk_npu_vdsp_apb  :  1;
        unsigned int st_clk_npu_m85_apb   :  1;
        unsigned int st_clk_npu_m85_atb   :  1;
        unsigned int st_clk_npu_m85_ts    :  1;
        unsigned int st_clk_npu_vdsp_ts   :  1;
        unsigned int st_clk_npu_vdsp_atb  :  1;
        unsigned int st_clk_npu_debug_atb :  1;
        unsigned int st_pclk_npu_debug    :  1;
        unsigned int st_clk_npu_ccd       :  1;
        unsigned int st_aclk_npu_dmmu1    :  1;
        unsigned int st_aclk_npu_dmmu0    :  1;
        unsigned int st_aclk_npu_tcu_tpc  :  1;
        unsigned int reserved_1           :  4;
        unsigned int st_clk_npu_core3     :  1;
        unsigned int st_clk_npu_core5     :  1;
        unsigned int st_clk_npu_core2     :  1;
        unsigned int st_clk_npu_core4     :  1;
        unsigned int st_clk_npu_core1     :  1;
        unsigned int st_clk_npu_core0     :  1;
    } reg;
}npu_crg_clock_st_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_npu_ns_ipcm  :  1;
        unsigned int gt_aclk_npu_core     :  1;
        unsigned int gt_pclk_npu_wtd1     :  1;
        unsigned int gt_pclk_npu_wtd0     :  1;
        unsigned int reserved_0           :  1;
        unsigned int gt_pclk_npu_timer0   :  1;
        unsigned int gt_pclk_npu_ccd      :  1;
        unsigned int gt_pclk_npu_sys_ctrl :  1;
        unsigned int gt_pclk_npu_misc     :  1;
        unsigned int gt_pclk_npu_dma      :  1;
        unsigned int gt_pclk_npu_sec_reg  :  1;
        unsigned int reserved_1           :  1;
        unsigned int gt_pclk_npu_vmid10   :  1;
        unsigned int gt_pclk_npu_vmid9    :  1;
        unsigned int gt_pclk_npu_vmid8    :  1;
        unsigned int gt_pclk_npu_vmid7    :  1;
        unsigned int gt_pclk_npu_vmid6    :  1;
        unsigned int gt_pclk_npu_vmid5    :  1;
        unsigned int gt_pclk_npu_vmid4    :  1;
        unsigned int gt_pclk_npu_vmid3    :  1;
        unsigned int gt_pclk_npu_vmid2    :  1;
        unsigned int gt_pclk_npu_vmid1    :  1;
        unsigned int gt_pclk_npu_vmid0    :  1;
        unsigned int reserved_2           :  2;
        unsigned int gt_aclk_npu_dma      :  1;
        unsigned int gt_clk_npu_local_atb :  1;
        unsigned int gt_hclk_npu_core_cfg :  1;
        unsigned int reserved_3           :  1;
        unsigned int gt_aclk_npu_tcu      :  1;
        unsigned int gt_aclk_npu_tbu9     :  1;
        unsigned int gt_aclk_npu_tbu8     :  1;
    } reg;
}npu_crg_clock_cg_1_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_npu_ns_ipcm  :  1;
        unsigned int gt_aclk_npu_core     :  1;
        unsigned int gt_pclk_npu_wtd1     :  1;
        unsigned int gt_pclk_npu_wtd0     :  1;
        unsigned int reserved_0           :  1;
        unsigned int gt_pclk_npu_timer0   :  1;
        unsigned int gt_pclk_npu_ccd      :  1;
        unsigned int gt_pclk_npu_sys_ctrl :  1;
        unsigned int gt_pclk_npu_misc     :  1;
        unsigned int gt_pclk_npu_dma      :  1;
        unsigned int gt_pclk_npu_sec_reg  :  1;
        unsigned int reserved_1           :  1;
        unsigned int gt_pclk_npu_vmid10   :  1;
        unsigned int gt_pclk_npu_vmid9    :  1;
        unsigned int gt_pclk_npu_vmid8    :  1;
        unsigned int gt_pclk_npu_vmid7    :  1;
        unsigned int gt_pclk_npu_vmid6    :  1;
        unsigned int gt_pclk_npu_vmid5    :  1;
        unsigned int gt_pclk_npu_vmid4    :  1;
        unsigned int gt_pclk_npu_vmid3    :  1;
        unsigned int gt_pclk_npu_vmid2    :  1;
        unsigned int gt_pclk_npu_vmid1    :  1;
        unsigned int gt_pclk_npu_vmid0    :  1;
        unsigned int reserved_2           :  2;
        unsigned int gt_aclk_npu_dma      :  1;
        unsigned int gt_clk_npu_local_atb :  1;
        unsigned int gt_hclk_npu_core_cfg :  1;
        unsigned int reserved_3           :  1;
        unsigned int gt_aclk_npu_tcu      :  1;
        unsigned int gt_aclk_npu_tbu9     :  1;
        unsigned int gt_aclk_npu_tbu8     :  1;
    } reg;
}npu_crg_clock_cg_1_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_npu_ns_ipcm  :  1;
        unsigned int gt_aclk_npu_core     :  1;
        unsigned int gt_pclk_npu_wtd1     :  1;
        unsigned int gt_pclk_npu_wtd0     :  1;
        unsigned int reserved_0           :  1;
        unsigned int gt_pclk_npu_timer0   :  1;
        unsigned int gt_pclk_npu_ccd      :  1;
        unsigned int gt_pclk_npu_sys_ctrl :  1;
        unsigned int gt_pclk_npu_misc     :  1;
        unsigned int gt_pclk_npu_dma      :  1;
        unsigned int gt_pclk_npu_sec_reg  :  1;
        unsigned int reserved_1           :  1;
        unsigned int gt_pclk_npu_vmid10   :  1;
        unsigned int gt_pclk_npu_vmid9    :  1;
        unsigned int gt_pclk_npu_vmid8    :  1;
        unsigned int gt_pclk_npu_vmid7    :  1;
        unsigned int gt_pclk_npu_vmid6    :  1;
        unsigned int gt_pclk_npu_vmid5    :  1;
        unsigned int gt_pclk_npu_vmid4    :  1;
        unsigned int gt_pclk_npu_vmid3    :  1;
        unsigned int gt_pclk_npu_vmid2    :  1;
        unsigned int gt_pclk_npu_vmid1    :  1;
        unsigned int gt_pclk_npu_vmid0    :  1;
        unsigned int reserved_2           :  2;
        unsigned int gt_aclk_npu_dma      :  1;
        unsigned int gt_clk_npu_local_atb :  1;
        unsigned int gt_hclk_npu_core_cfg :  1;
        unsigned int reserved_3           :  1;
        unsigned int gt_aclk_npu_tcu      :  1;
        unsigned int gt_aclk_npu_tbu9     :  1;
        unsigned int gt_aclk_npu_tbu8     :  1;
    } reg;
}npu_crg_clock_cg_1_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_pclk_npu_ns_ipcm  :  1;
        unsigned int st_aclk_npu_core     :  1;
        unsigned int st_pclk_npu_wtd1     :  1;
        unsigned int st_pclk_npu_wtd0     :  1;
        unsigned int reserved_0           :  1;
        unsigned int st_pclk_npu_timer0   :  1;
        unsigned int st_pclk_npu_ccd      :  1;
        unsigned int st_pclk_npu_sys_ctrl :  1;
        unsigned int st_pclk_npu_misc     :  1;
        unsigned int st_pclk_npu_dma      :  1;
        unsigned int st_pclk_npu_sec_reg  :  1;
        unsigned int reserved_1           :  1;
        unsigned int st_pclk_npu_vmid10   :  1;
        unsigned int st_pclk_npu_vmid9    :  1;
        unsigned int st_pclk_npu_vmid8    :  1;
        unsigned int st_pclk_npu_vmid7    :  1;
        unsigned int st_pclk_npu_vmid6    :  1;
        unsigned int st_pclk_npu_vmid5    :  1;
        unsigned int st_pclk_npu_vmid4    :  1;
        unsigned int st_pclk_npu_vmid3    :  1;
        unsigned int st_pclk_npu_vmid2    :  1;
        unsigned int st_pclk_npu_vmid1    :  1;
        unsigned int st_pclk_npu_vmid0    :  1;
        unsigned int reserved_2           :  2;
        unsigned int st_aclk_npu_dma      :  1;
        unsigned int st_clk_npu_local_atb :  1;
        unsigned int st_hclk_npu_core_cfg :  1;
        unsigned int reserved_3           :  1;
        unsigned int st_aclk_npu_tcu      :  1;
        unsigned int st_aclk_npu_tbu9     :  1;
        unsigned int st_aclk_npu_tbu8     :  1;
    } reg;
}npu_crg_clock_st_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_npu_noc_data      :  1;
        unsigned int reserved_0               :  1;
        unsigned int gt_clk_npu_mcu_systick   :  1;
        unsigned int gt_pclk_npu_uart         :  1;
        unsigned int gt_clk_npu_wtd1          :  1;
        unsigned int gt_clk_npu_wtd0          :  1;
        unsigned int gt_clk_npu_timer1        :  1;
        unsigned int gt_clk_npu_timer0        :  1;
        unsigned int gt_aclk_npu_bist         :  1;
        unsigned int gt_clk_npu_procm         :  1;
        unsigned int gt_clk_npu_hpm1          :  1;
        unsigned int gt_clk_npu_hpm0          :  1;
        unsigned int gt_clk_npu_bus_data      :  1;
        unsigned int gt_aclk_npu_vdsp_slv     :  1;
        unsigned int reserved_1               :  1;
        unsigned int gt_clk_npu_vdsp_core     :  1;
        unsigned int gt_pclk_npu_s_ipcm       :  1;
        unsigned int gt_clk_vdsp_pll_logic    :  1;
        unsigned int gt_clk_core_pll_logic    :  1;
        unsigned int gt_aclk_perf_npu_m85     :  1;
        unsigned int gt_aclk_perf_npu_tcu     :  1;
        unsigned int gt_aclk_perf_npu_xio_dma :  1;
        unsigned int gt_aclk_perf_npu_vdsp    :  1;
        unsigned int gt_aclk_perf_npu_gp      :  1;
        unsigned int gt_aclk_perf_npu_dsa     :  1;
        unsigned int gt_pclk_perf_npu_m85     :  1;
        unsigned int gt_pclk_perf_npu_tcu     :  1;
        unsigned int gt_pclk_perf_npu_xio_dma :  1;
        unsigned int gt_pclk_perf_npu_vdsp    :  1;
        unsigned int gt_pclk_perf_npu_gp      :  1;
        unsigned int gt_pclk_perf_npu_dsa     :  1;
        unsigned int gt_pclk_npu_bist         :  1;
    } reg;
}npu_crg_clock_cg_2_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_npu_noc_data      :  1;
        unsigned int reserved_0               :  1;
        unsigned int gt_clk_npu_mcu_systick   :  1;
        unsigned int gt_pclk_npu_uart         :  1;
        unsigned int gt_clk_npu_wtd1          :  1;
        unsigned int gt_clk_npu_wtd0          :  1;
        unsigned int gt_clk_npu_timer1        :  1;
        unsigned int gt_clk_npu_timer0        :  1;
        unsigned int gt_aclk_npu_bist         :  1;
        unsigned int gt_clk_npu_procm         :  1;
        unsigned int gt_clk_npu_hpm1          :  1;
        unsigned int gt_clk_npu_hpm0          :  1;
        unsigned int gt_clk_npu_bus_data      :  1;
        unsigned int gt_aclk_npu_vdsp_slv     :  1;
        unsigned int reserved_1               :  1;
        unsigned int gt_clk_npu_vdsp_core     :  1;
        unsigned int gt_pclk_npu_s_ipcm       :  1;
        unsigned int gt_clk_vdsp_pll_logic    :  1;
        unsigned int gt_clk_core_pll_logic    :  1;
        unsigned int gt_aclk_perf_npu_m85     :  1;
        unsigned int gt_aclk_perf_npu_tcu     :  1;
        unsigned int gt_aclk_perf_npu_xio_dma :  1;
        unsigned int gt_aclk_perf_npu_vdsp    :  1;
        unsigned int gt_aclk_perf_npu_gp      :  1;
        unsigned int gt_aclk_perf_npu_dsa     :  1;
        unsigned int gt_pclk_perf_npu_m85     :  1;
        unsigned int gt_pclk_perf_npu_tcu     :  1;
        unsigned int gt_pclk_perf_npu_xio_dma :  1;
        unsigned int gt_pclk_perf_npu_vdsp    :  1;
        unsigned int gt_pclk_perf_npu_gp      :  1;
        unsigned int gt_pclk_perf_npu_dsa     :  1;
        unsigned int gt_pclk_npu_bist         :  1;
    } reg;
}npu_crg_clock_cg_2_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_npu_noc_data      :  1;
        unsigned int reserved_0               :  1;
        unsigned int gt_clk_npu_mcu_systick   :  1;
        unsigned int gt_pclk_npu_uart         :  1;
        unsigned int gt_clk_npu_wtd1          :  1;
        unsigned int gt_clk_npu_wtd0          :  1;
        unsigned int gt_clk_npu_timer1        :  1;
        unsigned int gt_clk_npu_timer0        :  1;
        unsigned int gt_aclk_npu_bist         :  1;
        unsigned int gt_clk_npu_procm         :  1;
        unsigned int gt_clk_npu_hpm1          :  1;
        unsigned int gt_clk_npu_hpm0          :  1;
        unsigned int gt_clk_npu_bus_data      :  1;
        unsigned int gt_aclk_npu_vdsp_slv     :  1;
        unsigned int reserved_1               :  1;
        unsigned int gt_clk_npu_vdsp_core     :  1;
        unsigned int gt_pclk_npu_s_ipcm       :  1;
        unsigned int gt_clk_vdsp_pll_logic    :  1;
        unsigned int gt_clk_core_pll_logic    :  1;
        unsigned int gt_aclk_perf_npu_m85     :  1;
        unsigned int gt_aclk_perf_npu_tcu     :  1;
        unsigned int gt_aclk_perf_npu_xio_dma :  1;
        unsigned int gt_aclk_perf_npu_vdsp    :  1;
        unsigned int gt_aclk_perf_npu_gp      :  1;
        unsigned int gt_aclk_perf_npu_dsa     :  1;
        unsigned int gt_pclk_perf_npu_m85     :  1;
        unsigned int gt_pclk_perf_npu_tcu     :  1;
        unsigned int gt_pclk_perf_npu_xio_dma :  1;
        unsigned int gt_pclk_perf_npu_vdsp    :  1;
        unsigned int gt_pclk_perf_npu_gp      :  1;
        unsigned int gt_pclk_perf_npu_dsa     :  1;
        unsigned int gt_pclk_npu_bist         :  1;
    } reg;
}npu_crg_clock_cg_2_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_npu_noc_data      :  1;
        unsigned int reserved_0               :  1;
        unsigned int st_clk_npu_mcu_systick   :  1;
        unsigned int st_pclk_npu_uart         :  1;
        unsigned int st_clk_npu_wtd1          :  1;
        unsigned int st_clk_npu_wtd0          :  1;
        unsigned int st_clk_npu_timer1        :  1;
        unsigned int st_clk_npu_timer0        :  1;
        unsigned int st_aclk_npu_bist         :  1;
        unsigned int st_clk_npu_procm         :  1;
        unsigned int st_clk_npu_hpm1          :  1;
        unsigned int st_clk_npu_hpm0          :  1;
        unsigned int st_clk_npu_bus_data      :  1;
        unsigned int st_aclk_npu_vdsp_slv     :  1;
        unsigned int reserved_1               :  1;
        unsigned int st_clk_npu_vdsp_core     :  1;
        unsigned int st_pclk_npu_s_ipcm       :  1;
        unsigned int st_clk_vdsp_pll_logic    :  1;
        unsigned int st_clk_core_pll_logic    :  1;
        unsigned int st_aclk_perf_npu_m85     :  1;
        unsigned int st_aclk_perf_npu_tcu     :  1;
        unsigned int st_aclk_perf_npu_xio_dma :  1;
        unsigned int st_aclk_perf_npu_vdsp    :  1;
        unsigned int st_aclk_perf_npu_gp      :  1;
        unsigned int st_aclk_perf_npu_dsa     :  1;
        unsigned int st_pclk_perf_npu_m85     :  1;
        unsigned int st_pclk_perf_npu_tcu     :  1;
        unsigned int st_pclk_perf_npu_xio_dma :  1;
        unsigned int st_pclk_perf_npu_vdsp    :  1;
        unsigned int st_pclk_perf_npu_gp      :  1;
        unsigned int st_pclk_perf_npu_dsa     :  1;
        unsigned int st_pclk_npu_bist         :  1;
    } reg;
}npu_crg_clock_st_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0          :  7;
        unsigned int gt_clk_npu_mcu      :  1;
        unsigned int gt_clk_npu_mcu_iwic :  1;
        unsigned int gt_clk_npu_mcu_dbg  :  1;
        unsigned int gt_clk_npu_mcu_core :  1;
        unsigned int reserved_1          : 21;
    } reg;
}npu_crg_clock_cg_3_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0          :  7;
        unsigned int gt_clk_npu_mcu      :  1;
        unsigned int gt_clk_npu_mcu_iwic :  1;
        unsigned int gt_clk_npu_mcu_dbg  :  1;
        unsigned int gt_clk_npu_mcu_core :  1;
        unsigned int reserved_1          : 21;
    } reg;
}npu_crg_clock_cg_3_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0          :  7;
        unsigned int gt_clk_npu_mcu      :  1;
        unsigned int gt_clk_npu_mcu_iwic :  1;
        unsigned int gt_clk_npu_mcu_dbg  :  1;
        unsigned int gt_clk_npu_mcu_core :  1;
        unsigned int reserved_1          : 21;
    } reg;
}npu_crg_clock_cg_3_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0          :  7;
        unsigned int st_clk_npu_mcu      :  1;
        unsigned int st_clk_npu_mcu_iwic :  1;
        unsigned int st_clk_npu_mcu_dbg  :  1;
        unsigned int st_clk_npu_mcu_core :  1;
        unsigned int reserved_1          : 21;
    } reg;
}npu_crg_clock_st_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_clk_npu_vdsp_core :  2;
        unsigned int sel_clk_npu_mcu       :  2;
        unsigned int sel_clk_npu_noc_data  :  1;
        unsigned int reserved_0            :  3;
        unsigned int sel_clk_npu_core      :  2;
        unsigned int sel_clk_npu_timer1    :  1;
        unsigned int sel_clk_npu_timer0    :  1;
        unsigned int sel_npu_pll_test      :  2;
        unsigned int sel_clk_npu_vdsp_vdcs :  1;
        unsigned int sel_clk_npu_core_vdcs :  1;
        unsigned int _bm_                  : 16;
    } reg;
}npu_crg_clock_sw_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_ack_clk_npu_vdsp_core_sw :  3;
        unsigned int sw_ack_clk_npu_mcu_sw       :  3;
        unsigned int sw_ack_clk_npu_noc_data_sw  :  2;
        unsigned int sw_ack_clk_npu_vdsp_vdcs_sw :  2;
        unsigned int sw_ack_clk_npu_core_vdcs_sw :  2;
        unsigned int reserved_0                  :  1;
        unsigned int sw_ack_clk_npu_core_sw      :  3;
        unsigned int reserved_1                  : 16;
    } reg;
}npu_crg_clock_sw_st_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_npu_core_pre :  6;
        unsigned int sc_gt_clk_npu_core   :  1;
        unsigned int reserved_0           :  9;
        unsigned int _bm_                 : 16;
    } reg;
}npu_crg_clock_div_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_npu_core134 :  2;
        unsigned int reserved_0          :  8;
        unsigned int div_clk_npu_core025 :  2;
        unsigned int reserved_1          :  4;
        unsigned int _bm_                : 16;
    } reg;
}npu_crg_npu_clock_div_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_npu_bus_cfg    :  6;
        unsigned int reserved_0             :  1;
        unsigned int div_clk_npu_bus_data   :  6;
        unsigned int sc_gt_clk_npu_bus_data :  1;
        unsigned int reserved_1             :  2;
        unsigned int _bm_                   : 16;
    } reg;
}npu_crg_npu_clock_div_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_npu_mcu         :  6;
        unsigned int sc_gt_clk_npu_mcu       :  1;
        unsigned int div_clk_npu_vdsp_core   :  6;
        unsigned int sc_gt_clk_npu_vdsp_core :  1;
        unsigned int reserved_0              :  2;
        unsigned int _bm_                    : 16;
    } reg;
}npu_crg_npu_clock_div_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_npu_noc_data   :  6;
        unsigned int sc_gt_clk_npu_noc_data :  1;
        unsigned int reserved_0             :  9;
        unsigned int _bm_                   : 16;
    } reg;
}npu_crg_npu_clock_div_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0      :  6;
        unsigned int div_clk_npu_ref :  6;
        unsigned int reserved_1      :  4;
        unsigned int _bm_            : 16;
    } reg;
}npu_crg_npu_clock_div_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_npu_mcu_systick :  6;
        unsigned int reserved_0              : 10;
        unsigned int _bm_                    : 16;
    } reg;
}npu_crg_npu_clock_div_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_npu_timer1 :  6;
        unsigned int div_clk_npu_timer0 :  6;
        unsigned int reserved_0         :  4;
        unsigned int _bm_               : 16;
    } reg;
}npu_crg_npu_clock_div_8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_npu_core_vdcs_test   :  6;
        unsigned int sc_gt_clk_npu_core_vdcs_test :  1;
        unsigned int div_clk_npu_vdsp_vdcs_test   :  6;
        unsigned int sc_gt_clk_npu_vdsp_vdcs_test :  1;
        unsigned int reserved_0                   :  2;
        unsigned int _bm_                         : 16;
    } reg;
}npu_crg_npu_clock_div_9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_npu_core_pll_test   :  6;
        unsigned int sc_gt_clk_npu_core_pll_test :  1;
        unsigned int div_clk_npu_vdsp_pll_test   :  6;
        unsigned int sc_gt_clk_npu_vdsp_pll_test :  1;
        unsigned int reserved_0                  :  2;
        unsigned int _bm_                        : 16;
    } reg;
}npu_crg_npu_clock_div_10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_done_clk_npu_mcu_systick  :  1;
        unsigned int div_done_clk_npu_hpm0         :  1;
        unsigned int div_done_clk_npu_core_pre_div :  1;
        unsigned int div_done_clk_npu_vdsp_core    :  1;
        unsigned int div_done_clk_npu_mcu          :  1;
        unsigned int reserved_0                    :  1;
        unsigned int div_done_clk_npu_bus_cfg      :  1;
        unsigned int div_done_clk_npu_bus_data     :  1;
        unsigned int reserved_1                    :  7;
        unsigned int div_done_clk_npu_noc_data     :  1;
        unsigned int div_done_clk_npu_timer1       :  1;
        unsigned int div_done_clk_npu_timer0       :  1;
        unsigned int reserved_2                    : 14;
    } reg;
}npu_crg_npu_clock_div_done_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_prst_npu_ns_ipcm_n  :  1;
        unsigned int ip_prst_npu_s_ipcm_n   :  1;
        unsigned int ip_prst_npu_sys_ctrl_n :  1;
        unsigned int ip_prst_npu_sec_reg_n  :  1;
        unsigned int ip_prst_npu_ccd_n      :  1;
        unsigned int ip_prst_npu_vmid10_n   :  1;
        unsigned int ip_prst_npu_vmid9_n    :  1;
        unsigned int ip_prst_npu_vmid8_n    :  1;
        unsigned int ip_prst_npu_vmid7_n    :  1;
        unsigned int ip_prst_npu_vmid6_n    :  1;
        unsigned int ip_prst_npu_vmid5_n    :  1;
        unsigned int ip_prst_npu_vmid4_n    :  1;
        unsigned int ip_prst_npu_vmid3_n    :  1;
        unsigned int ip_prst_npu_vmid2_n    :  1;
        unsigned int ip_prst_npu_vmid1_n    :  1;
        unsigned int ip_prst_npu_vmid0_n    :  1;
        unsigned int reserved_0             :  1;
        unsigned int ip_prst_npu_misc_n     :  1;
        unsigned int reserved_1             :  2;
        unsigned int ip_arst_npu_core_n     :  1;
        unsigned int ip_hrst_npu_core_cfg_n :  1;
        unsigned int ip_rst_npu_core5_n     :  1;
        unsigned int ip_rst_npu_core4_n     :  1;
        unsigned int ip_rst_npu_core3_n     :  1;
        unsigned int ip_rst_npu_core2_n     :  1;
        unsigned int ip_rst_npu_core1_n     :  1;
        unsigned int ip_rst_npu_core0_n     :  1;
        unsigned int ip_rst_npu_core_n      :  1;
        unsigned int ip_rst_npu_procm_n     :  1;
        unsigned int ip_rst_npu_hpm1_n      :  1;
        unsigned int ip_rst_npu_hpm0_n      :  1;
    } reg;
}npu_crg_npu_clock_rst_0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_prst_npu_ns_ipcm_n  :  1;
        unsigned int ip_prst_npu_s_ipcm_n   :  1;
        unsigned int ip_prst_npu_sys_ctrl_n :  1;
        unsigned int ip_prst_npu_sec_reg_n  :  1;
        unsigned int ip_prst_npu_ccd_n      :  1;
        unsigned int ip_prst_npu_vmid10_n   :  1;
        unsigned int ip_prst_npu_vmid9_n    :  1;
        unsigned int ip_prst_npu_vmid8_n    :  1;
        unsigned int ip_prst_npu_vmid7_n    :  1;
        unsigned int ip_prst_npu_vmid6_n    :  1;
        unsigned int ip_prst_npu_vmid5_n    :  1;
        unsigned int ip_prst_npu_vmid4_n    :  1;
        unsigned int ip_prst_npu_vmid3_n    :  1;
        unsigned int ip_prst_npu_vmid2_n    :  1;
        unsigned int ip_prst_npu_vmid1_n    :  1;
        unsigned int ip_prst_npu_vmid0_n    :  1;
        unsigned int reserved_0             :  1;
        unsigned int ip_prst_npu_misc_n     :  1;
        unsigned int reserved_1             :  2;
        unsigned int ip_arst_npu_core_n     :  1;
        unsigned int ip_hrst_npu_core_cfg_n :  1;
        unsigned int ip_rst_npu_core5_n     :  1;
        unsigned int ip_rst_npu_core4_n     :  1;
        unsigned int ip_rst_npu_core3_n     :  1;
        unsigned int ip_rst_npu_core2_n     :  1;
        unsigned int ip_rst_npu_core1_n     :  1;
        unsigned int ip_rst_npu_core0_n     :  1;
        unsigned int ip_rst_npu_core_n      :  1;
        unsigned int ip_rst_npu_procm_n     :  1;
        unsigned int ip_rst_npu_hpm1_n      :  1;
        unsigned int ip_rst_npu_hpm0_n      :  1;
    } reg;
}npu_crg_npu_clock_rst_0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_prst_npu_ns_ipcm_n  :  1;
        unsigned int ip_prst_npu_s_ipcm_n   :  1;
        unsigned int ip_prst_npu_sys_ctrl_n :  1;
        unsigned int ip_prst_npu_sec_reg_n  :  1;
        unsigned int ip_prst_npu_ccd_n      :  1;
        unsigned int ip_prst_npu_vmid10_n   :  1;
        unsigned int ip_prst_npu_vmid9_n    :  1;
        unsigned int ip_prst_npu_vmid8_n    :  1;
        unsigned int ip_prst_npu_vmid7_n    :  1;
        unsigned int ip_prst_npu_vmid6_n    :  1;
        unsigned int ip_prst_npu_vmid5_n    :  1;
        unsigned int ip_prst_npu_vmid4_n    :  1;
        unsigned int ip_prst_npu_vmid3_n    :  1;
        unsigned int ip_prst_npu_vmid2_n    :  1;
        unsigned int ip_prst_npu_vmid1_n    :  1;
        unsigned int ip_prst_npu_vmid0_n    :  1;
        unsigned int reserved_0             :  1;
        unsigned int ip_prst_npu_misc_n     :  1;
        unsigned int reserved_1             :  2;
        unsigned int ip_arst_npu_core_n     :  1;
        unsigned int ip_hrst_npu_core_cfg_n :  1;
        unsigned int ip_rst_npu_core5_n     :  1;
        unsigned int ip_rst_npu_core4_n     :  1;
        unsigned int ip_rst_npu_core3_n     :  1;
        unsigned int ip_rst_npu_core2_n     :  1;
        unsigned int ip_rst_npu_core1_n     :  1;
        unsigned int ip_rst_npu_core0_n     :  1;
        unsigned int ip_rst_npu_core_n      :  1;
        unsigned int ip_rst_npu_procm_n     :  1;
        unsigned int ip_rst_npu_hpm1_n      :  1;
        unsigned int ip_rst_npu_hpm0_n      :  1;
    } reg;
}npu_crg_npu_clock_rst_0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0               :  1;
        unsigned int ip_rst_npu_dma_core_n    :  1;
        unsigned int reserved_1               :  1;
        unsigned int ip_rst_npu_vdsp_dbg_n    :  1;
        unsigned int ip_rst_npu_vdsp_n        :  1;
        unsigned int reserved_2               :  1;
        unsigned int ip_rst_npu_mcu_iwic_n    :  1;
        unsigned int ip_rst_npu_mcu_por_n     :  1;
        unsigned int ip_rst_npu_mcu_systick_n :  1;
        unsigned int ip_rst_npu_mcu_sys_n     :  1;
        unsigned int ip_prst_npu_uart_n       :  1;
        unsigned int ip_rst_npu_wtd1_n        :  1;
        unsigned int ip_rst_npu_wtd0_n        :  1;
        unsigned int ip_prst_npu_wtd1_n       :  1;
        unsigned int ip_prst_npu_wtd0_n       :  1;
        unsigned int ip_rst_npu_timer1_n      :  1;
        unsigned int ip_rst_npu_timer0_n      :  1;
        unsigned int reserved_3               :  1;
        unsigned int ip_prst_npu_timer0_n     :  1;
        unsigned int ip_rst_vdsp_pll_ssmod_n  :  1;
        unsigned int ip_rst_core_pll_ssmod_n  :  1;
        unsigned int ip_rst_vdsp_pll_logic_n  :  1;
        unsigned int ip_rst_core_pll_logic_n  :  1;
        unsigned int ip_rst_npu_bist_n        :  1;
        unsigned int reserved_4               :  1;
        unsigned int ip_rst_npu_tcu_n         :  1;
        unsigned int ip_rst_npu_nic_n         :  1;
        unsigned int ip_rst_npu_vdsp_apb_n    :  1;
        unsigned int reserved_5               :  4;
    } reg;
}npu_crg_npu_clock_rst_1_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0               :  1;
        unsigned int ip_rst_npu_dma_core_n    :  1;
        unsigned int reserved_1               :  1;
        unsigned int ip_rst_npu_vdsp_dbg_n    :  1;
        unsigned int ip_rst_npu_vdsp_n        :  1;
        unsigned int reserved_2               :  1;
        unsigned int ip_rst_npu_mcu_iwic_n    :  1;
        unsigned int ip_rst_npu_mcu_por_n     :  1;
        unsigned int ip_rst_npu_mcu_systick_n :  1;
        unsigned int ip_rst_npu_mcu_sys_n     :  1;
        unsigned int ip_prst_npu_uart_n       :  1;
        unsigned int ip_rst_npu_wtd1_n        :  1;
        unsigned int ip_rst_npu_wtd0_n        :  1;
        unsigned int ip_prst_npu_wtd1_n       :  1;
        unsigned int ip_prst_npu_wtd0_n       :  1;
        unsigned int ip_rst_npu_timer1_n      :  1;
        unsigned int ip_rst_npu_timer0_n      :  1;
        unsigned int reserved_3               :  1;
        unsigned int ip_prst_npu_timer0_n     :  1;
        unsigned int ip_rst_vdsp_pll_ssmod_n  :  1;
        unsigned int ip_rst_core_pll_ssmod_n  :  1;
        unsigned int ip_rst_vdsp_pll_logic_n  :  1;
        unsigned int ip_rst_core_pll_logic_n  :  1;
        unsigned int ip_rst_npu_bist_n        :  1;
        unsigned int reserved_4               :  1;
        unsigned int ip_rst_npu_tcu_n         :  1;
        unsigned int ip_rst_npu_nic_n         :  1;
        unsigned int ip_rst_npu_vdsp_apb_n    :  1;
        unsigned int reserved_5               :  4;
    } reg;
}npu_crg_npu_clock_rst_1_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0               :  1;
        unsigned int ip_rst_npu_dma_core_n    :  1;
        unsigned int reserved_1               :  1;
        unsigned int ip_rst_npu_vdsp_dbg_n    :  1;
        unsigned int ip_rst_npu_vdsp_n        :  1;
        unsigned int reserved_2               :  1;
        unsigned int ip_rst_npu_mcu_iwic_n    :  1;
        unsigned int ip_rst_npu_mcu_por_n     :  1;
        unsigned int ip_rst_npu_mcu_systick_n :  1;
        unsigned int ip_rst_npu_mcu_sys_n     :  1;
        unsigned int ip_prst_npu_uart_n       :  1;
        unsigned int ip_rst_npu_wtd1_n        :  1;
        unsigned int ip_rst_npu_wtd0_n        :  1;
        unsigned int ip_prst_npu_wtd1_n       :  1;
        unsigned int ip_prst_npu_wtd0_n       :  1;
        unsigned int ip_rst_npu_timer1_n      :  1;
        unsigned int ip_rst_npu_timer0_n      :  1;
        unsigned int reserved_3               :  1;
        unsigned int ip_prst_npu_timer0_n     :  1;
        unsigned int ip_rst_vdsp_pll_ssmod_n  :  1;
        unsigned int ip_rst_core_pll_ssmod_n  :  1;
        unsigned int ip_rst_vdsp_pll_logic_n  :  1;
        unsigned int ip_rst_core_pll_logic_n  :  1;
        unsigned int ip_rst_npu_bist_n        :  1;
        unsigned int reserved_4               :  1;
        unsigned int ip_rst_npu_tcu_n         :  1;
        unsigned int ip_rst_npu_nic_n         :  1;
        unsigned int ip_rst_npu_vdsp_apb_n    :  1;
        unsigned int reserved_5               :  4;
    } reg;
}npu_crg_npu_clock_rst_1_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_npu_peri_n         :  1;
        unsigned int ip_rst_npu_local_atb_n    :  1;
        unsigned int ip_rst_npu_tcu_tpc_n      :  1;
        unsigned int ip_rst_npu_ccd_n          :  1;
        unsigned int ip_rst_perf_npu_m85_n     :  1;
        unsigned int ip_rst_perf_npu_tcu_n     :  1;
        unsigned int ip_rst_perf_npu_xio_dma_n :  1;
        unsigned int ip_rst_perf_npu_vdsp_n    :  1;
        unsigned int ip_rst_perf_npu_gp_n      :  1;
        unsigned int ip_rst_perf_npu_dsa_n     :  1;
        unsigned int ip_rst_npu_dmmu1_n        :  1;
        unsigned int ip_rst_npu_dmmu0_n        :  1;
        unsigned int ip_prst_npu_debug_n       :  1;
        unsigned int ip_rst_npu_debug_atb_n    :  1;
        unsigned int ip_rst_npu_tbu9_n         :  1;
        unsigned int ip_rst_npu_tbu8_n         :  1;
        unsigned int ip_rst_npu_tbu7_n         :  1;
        unsigned int ip_rst_npu_tbu6_n         :  1;
        unsigned int ip_rst_npu_tbu5_n         :  1;
        unsigned int ip_rst_npu_tbu4_n         :  1;
        unsigned int ip_rst_npu_tbu3_n         :  1;
        unsigned int ip_rst_npu_tbu2_n         :  1;
        unsigned int ip_rst_npu_tbu1_n         :  1;
        unsigned int ip_rst_npu_tbu0_n         :  1;
        unsigned int ip_rst_vdsp_vdcs_cs_n     :  1;
        unsigned int ip_rst_vdsp_vdcs_vd_n     :  1;
        unsigned int ip_rst_vdsp_vdcs_n        :  1;
        unsigned int ip_rst_core_vdcs_cs_n     :  1;
        unsigned int ip_rst_core_vdcs_vd_n     :  1;
        unsigned int ip_rst_core_vdcs_n        :  1;
        unsigned int reserved_0                :  2;
    } reg;
}npu_crg_npu_clock_rst_2_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_npu_peri_n         :  1;
        unsigned int ip_rst_npu_local_atb_n    :  1;
        unsigned int ip_rst_npu_tcu_tpc_n      :  1;
        unsigned int ip_rst_npu_ccd_n          :  1;
        unsigned int ip_rst_perf_npu_m85_n     :  1;
        unsigned int ip_rst_perf_npu_tcu_n     :  1;
        unsigned int ip_rst_perf_npu_xio_dma_n :  1;
        unsigned int ip_rst_perf_npu_vdsp_n    :  1;
        unsigned int ip_rst_perf_npu_gp_n      :  1;
        unsigned int ip_rst_perf_npu_dsa_n     :  1;
        unsigned int ip_rst_npu_dmmu1_n        :  1;
        unsigned int ip_rst_npu_dmmu0_n        :  1;
        unsigned int ip_prst_npu_debug_n       :  1;
        unsigned int ip_rst_npu_debug_atb_n    :  1;
        unsigned int ip_rst_npu_tbu9_n         :  1;
        unsigned int ip_rst_npu_tbu8_n         :  1;
        unsigned int ip_rst_npu_tbu7_n         :  1;
        unsigned int ip_rst_npu_tbu6_n         :  1;
        unsigned int ip_rst_npu_tbu5_n         :  1;
        unsigned int ip_rst_npu_tbu4_n         :  1;
        unsigned int ip_rst_npu_tbu3_n         :  1;
        unsigned int ip_rst_npu_tbu2_n         :  1;
        unsigned int ip_rst_npu_tbu1_n         :  1;
        unsigned int ip_rst_npu_tbu0_n         :  1;
        unsigned int ip_rst_vdsp_vdcs_cs_n     :  1;
        unsigned int ip_rst_vdsp_vdcs_vd_n     :  1;
        unsigned int ip_rst_vdsp_vdcs_n        :  1;
        unsigned int ip_rst_core_vdcs_cs_n     :  1;
        unsigned int ip_rst_core_vdcs_vd_n     :  1;
        unsigned int ip_rst_core_vdcs_n        :  1;
        unsigned int reserved_0                :  2;
    } reg;
}npu_crg_npu_clock_rst_2_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_npu_peri_n         :  1;
        unsigned int ip_rst_npu_local_atb_n    :  1;
        unsigned int ip_rst_npu_tcu_tpc_n      :  1;
        unsigned int ip_rst_npu_ccd_n          :  1;
        unsigned int ip_rst_perf_npu_m85_n     :  1;
        unsigned int ip_rst_perf_npu_tcu_n     :  1;
        unsigned int ip_rst_perf_npu_xio_dma_n :  1;
        unsigned int ip_rst_perf_npu_vdsp_n    :  1;
        unsigned int ip_rst_perf_npu_gp_n      :  1;
        unsigned int ip_rst_perf_npu_dsa_n     :  1;
        unsigned int ip_rst_npu_dmmu1_n        :  1;
        unsigned int ip_rst_npu_dmmu0_n        :  1;
        unsigned int ip_prst_npu_debug_n       :  1;
        unsigned int ip_rst_npu_debug_atb_n    :  1;
        unsigned int ip_rst_npu_tbu9_n         :  1;
        unsigned int ip_rst_npu_tbu8_n         :  1;
        unsigned int ip_rst_npu_tbu7_n         :  1;
        unsigned int ip_rst_npu_tbu6_n         :  1;
        unsigned int ip_rst_npu_tbu5_n         :  1;
        unsigned int ip_rst_npu_tbu4_n         :  1;
        unsigned int ip_rst_npu_tbu3_n         :  1;
        unsigned int ip_rst_npu_tbu2_n         :  1;
        unsigned int ip_rst_npu_tbu1_n         :  1;
        unsigned int ip_rst_npu_tbu0_n         :  1;
        unsigned int ip_rst_vdsp_vdcs_cs_n     :  1;
        unsigned int ip_rst_vdsp_vdcs_vd_n     :  1;
        unsigned int ip_rst_vdsp_vdcs_n        :  1;
        unsigned int ip_rst_core_vdcs_cs_n     :  1;
        unsigned int ip_rst_core_vdcs_vd_n     :  1;
        unsigned int ip_rst_core_vdcs_n        :  1;
        unsigned int reserved_0                :  2;
    } reg;
}npu_crg_npu_clock_rst_2_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int core_vdcs_freq_dn_en :  1;
        unsigned int vdsp_vdcs_freq_dn_en :  1;
        unsigned int reserved_0           : 14;
        unsigned int _bm_                 : 16;
    } reg;
}npu_crg_npu_dvfs_freq_dn_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_start_fsm_bypass :  1;
        unsigned int npu_core_pll_lock_timerout    :  9;
        unsigned int npu_core_pll_retry_num        :  3;
        unsigned int npu_core_pll_fsm_en           :  1;
        unsigned int npu_core_pll_cfg_rsv0         : 18;
    } reg;
}npu_crg_npu_core_pll_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_press_test_en  :  1;
        unsigned int npu_core_pll_press_test_cnt : 24;
        unsigned int npu_core_pll_cfg_rsv1       :  7;
    } reg;
}npu_crg_npu_core_pll_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_err_boot_cnt   : 24;
        unsigned int npu_core_pll_press_test_end :  1;
        unsigned int reserved_0                  :  7;
    } reg;
}npu_crg_npu_core_pll_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_hw_vote_mask  :  8;
        unsigned int npu_core_pll_sft_vote_mask :  8;
        unsigned int npu_core_pll_cfg_rsv2      : 16;
    } reg;
}npu_crg_npu_core_pll_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_vote_bypass :  8;
        unsigned int reserved_0               :  8;
        unsigned int _bm_                     : 16;
    } reg;
}npu_crg_npu_core_pll_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_vote_gt :  8;
        unsigned int reserved_0           :  8;
        unsigned int _bm_                 : 16;
    } reg;
}npu_crg_npu_core_pll_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_ssmod_en   :  1;
        unsigned int npu_core_pll_divval     :  6;
        unsigned int npu_core_pll_spread     :  5;
        unsigned int npu_core_pll_downspread :  1;
        unsigned int npu_core_pll_cfg_rsv3   : 19;
    } reg;
}npu_crg_npu_core_pll_ssmod_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_en        :  1;
        unsigned int npu_core_pll_bypass    :  1;
        unsigned int npu_core_pll_refdiv    :  6;
        unsigned int npu_core_pll_fbdiv     : 12;
        unsigned int npu_core_pll_postdiv1  :  3;
        unsigned int npu_core_pll_postdiv2  :  3;
        unsigned int npu_core_pll_cfg_valid :  1;
        unsigned int npu_core_pll_gt        :  1;
        unsigned int npu_core_pll_cfg_rsv4  :  4;
    } reg;
}npu_crg_npu_core_pll_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_frac     : 24;
        unsigned int npu_core_pll_dsm_en   :  1;
        unsigned int npu_core_pll_cfg_rsv5 :  7;
    } reg;
}npu_crg_npu_core_pll_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_offsetcalbyp  :  1;
        unsigned int npu_core_pll_offsetcalcnt  :  3;
        unsigned int npu_core_pll_offsetcalen   :  1;
        unsigned int npu_core_pll_offsetcalin   : 12;
        unsigned int npu_core_pll_offsetcalrstn :  1;
        unsigned int npu_core_pll_offsetfastcal :  1;
        unsigned int npu_core_pll_cfg_rsv6      : 13;
    } reg;
}npu_crg_npu_core_pll_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_core_pll_en_state        :  1;
        unsigned int npu_core_pll_offsetcalout    : 12;
        unsigned int npu_core_pll_offsetcallock   :  1;
        unsigned int npu_core_pll_offsetcalovf    :  1;
        unsigned int npu_core_pll_start_fsm_state :  3;
        unsigned int npu_core_pll_real_retry_cnt  :  3;
        unsigned int npu_core_st_clk_pll          :  1;
        unsigned int reserved_0                   : 10;
    } reg;
}npu_crg_npu_core_pll_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_start_fsm_bypass :  1;
        unsigned int npu_vdsp_pll_lock_timerout    :  9;
        unsigned int npu_vdsp_pll_retry_num        :  3;
        unsigned int npu_vdsp_pll_fsm_en           :  1;
        unsigned int npu_vdsp_pll_cfg_rsv0         : 18;
    } reg;
}npu_crg_npu_vdsp_pll_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_press_test_en  :  1;
        unsigned int npu_vdsp_pll_press_test_cnt : 24;
        unsigned int npu_vdsp_pll_cfg_rsv1       :  7;
    } reg;
}npu_crg_npu_vdsp_pll_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_err_boot_cnt   : 24;
        unsigned int npu_vdsp_pll_press_test_end :  1;
        unsigned int reserved_0                  :  7;
    } reg;
}npu_crg_npu_vdsp_pll_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_hw_vote_mask  :  8;
        unsigned int npu_vdsp_pll_sft_vote_mask :  8;
        unsigned int npu_vdsp_pll_cfg_rsv2      : 16;
    } reg;
}npu_crg_npu_vdsp_pll_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_vote_bypass :  8;
        unsigned int reserved_0               :  8;
        unsigned int _bm_                     : 16;
    } reg;
}npu_crg_npu_vdsp_pll_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_vote_gt :  8;
        unsigned int reserved_0           :  8;
        unsigned int _bm_                 : 16;
    } reg;
}npu_crg_npu_vdsp_pll_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_ssmod_en   :  1;
        unsigned int npu_vdsp_pll_divval     :  6;
        unsigned int npu_vdsp_pll_spread     :  5;
        unsigned int npu_vdsp_pll_downspread :  1;
        unsigned int npu_vdsp_pll_cfg_rsv3   : 19;
    } reg;
}npu_crg_npu_vdsp_pll_ssmod_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_en        :  1;
        unsigned int npu_vdsp_pll_bypass    :  1;
        unsigned int npu_vdsp_pll_refdiv    :  6;
        unsigned int npu_vdsp_pll_fbdiv     : 12;
        unsigned int npu_vdsp_pll_postdiv1  :  3;
        unsigned int npu_vdsp_pll_postdiv2  :  3;
        unsigned int npu_vdsp_pll_cfg_valid :  1;
        unsigned int npu_vdsp_pll_gt        :  1;
        unsigned int npu_vdsp_pll_cfg_rsv4  :  4;
    } reg;
}npu_crg_npu_vdsp_pll_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_frac     : 24;
        unsigned int npu_vdsp_pll_dsm_en   :  1;
        unsigned int npu_vdsp_pll_cfg_rsv5 :  7;
    } reg;
}npu_crg_npu_vdsp_pll_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_offsetcalbyp  :  1;
        unsigned int npu_vdsp_pll_offsetcalcnt  :  3;
        unsigned int npu_vdsp_pll_offsetcalen   :  1;
        unsigned int npu_vdsp_pll_offsetcalin   : 12;
        unsigned int npu_vdsp_pll_offsetcalrstn :  1;
        unsigned int npu_vdsp_pll_offsetfastcal :  1;
        unsigned int npu_vdsp_pll_cfg_rsv6      : 13;
    } reg;
}npu_crg_npu_vdsp_pll_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_pll_en_state        :  1;
        unsigned int npu_vdsp_pll_offsetcalout    : 12;
        unsigned int npu_vdsp_pll_offsetcallock   :  1;
        unsigned int npu_vdsp_pll_offsetcalovf    :  1;
        unsigned int npu_vdsp_pll_start_fsm_state :  3;
        unsigned int npu_vdsp_pll_real_retry_cnt  :  3;
        unsigned int npu_vdsp_st_clk_pll          :  1;
        unsigned int reserved_0                   : 10;
    } reg;
}npu_crg_npu_vdsp_pll_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_clear_0          :  1;
        unsigned int npu_hpm_ro_sel_0         :  6;
        unsigned int npu_hpm_ro_clk_div_0     :  2;
        unsigned int npu_hpm_ro_dbg_clk_div_0 :  6;
        unsigned int reserved_0               :  1;
        unsigned int _bm_                     : 16;
    } reg;
}npu_crg_hpm0_cfg_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_ro_en_0         :  1;
        unsigned int npu_hpm_test_hw_en_0    :  1;
        unsigned int npu_hpm_test_hw_start_0 :  1;
        unsigned int npu_hpm_timer_cnt_0     :  8;
        unsigned int reserved_0              :  5;
        unsigned int _bm_                    : 16;
    } reg;
}npu_crg_hpm0_cfg_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_clk_gt_0 :  1;
        unsigned int reserved_0       : 31;
    } reg;
}npu_crg_hpm0_clk_gt_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_clk_gt_0 :  1;
        unsigned int reserved_0       : 31;
    } reg;
}npu_crg_hpm0_clk_gt_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_clk_gt_0 :  1;
        unsigned int reserved_0       : 31;
    } reg;
}npu_crg_hpm0_clk_gt_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_dout_0 : 18;
        unsigned int reserved_0     : 14;
    } reg;
}npu_crg_hpm0_dout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_clear_1          :  1;
        unsigned int npu_hpm_ro_sel_1         :  6;
        unsigned int npu_hpm_ro_clk_div_1     :  2;
        unsigned int npu_hpm_ro_dbg_clk_div_1 :  6;
        unsigned int reserved_0               :  1;
        unsigned int _bm_                     : 16;
    } reg;
}npu_crg_npu_hpm1_cfg_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_ro_en_1         :  1;
        unsigned int npu_hpm_test_hw_en_1    :  1;
        unsigned int npu_hpm_test_hw_start_1 :  1;
        unsigned int npu_hpm_timer_cnt_1     :  8;
        unsigned int reserved_0              :  5;
        unsigned int _bm_                    : 16;
    } reg;
}npu_crg_npu_hpm1_cfg_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_clk_gt_1 :  1;
        unsigned int reserved_0       : 31;
    } reg;
}npu_crg_npu_hpm1_clk_gt_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_clk_gt_1 :  1;
        unsigned int reserved_0       : 31;
    } reg;
}npu_crg_npu_hpm1_clk_gt_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_clk_gt_1 :  1;
        unsigned int reserved_0       : 31;
    } reg;
}npu_crg_npu_hpm1_clk_gt_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_hpm_dout_1 : 18;
        unsigned int reserved_0     : 14;
    } reg;
}npu_crg_npu_hpm1_dout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_procm_clear      :  1;
        unsigned int npu_procm_ro_sel     :  7;
        unsigned int npu_procm_timer_sel  :  2;
        unsigned int npu_procm_ro_clk_div :  1;
        unsigned int reserved_0           :  5;
        unsigned int _bm_                 : 16;
    } reg;
}npu_crg_npu_procm_cfg_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_procm_ro_en          :  1;
        unsigned int npu_procm_test_hw_en     :  1;
        unsigned int npu_procm_test_hw_start  :  1;
        unsigned int npu_procm_ro_dbg_clk_div :  6;
        unsigned int npu_procm_sel_h          :  1;
        unsigned int reserved_0               :  6;
        unsigned int _bm_                     : 16;
    } reg;
}npu_crg_npu_procm_cfg_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_procm_clk_gt :  1;
        unsigned int reserved_0       : 31;
    } reg;
}npu_crg_npu_procm_clk_gt_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_procm_clk_gt :  1;
        unsigned int reserved_0       : 31;
    } reg;
}npu_crg_npu_procm_clk_gt_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_procm_clk_gt :  1;
        unsigned int reserved_0       : 31;
    } reg;
}npu_crg_npu_procm_clk_gt_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_procm_dout : 25;
        unsigned int reserved_0     :  7;
    } reg;
}npu_crg_npu_procm_dout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clkrst_flag_vmid9_bypass         :  1;
        unsigned int clkrst_flag_vmid8_bypass         :  1;
        unsigned int clkrst_flag_vmid7_bypass         :  1;
        unsigned int clkrst_flag_vmid6_bypass         :  1;
        unsigned int clkrst_flag_vmid5_bypass         :  1;
        unsigned int clkrst_flag_vmid4_bypass         :  1;
        unsigned int clkrst_flag_vmid3_bypass         :  1;
        unsigned int clkrst_flag_vmid2_bypass         :  1;
        unsigned int clkrst_flag_vmid1_bypass         :  1;
        unsigned int clkrst_flag_vmid0_bypass         :  1;
        unsigned int clkrst_flag_mcu_bypass           :  1;
        unsigned int clkrst_flag_sysctrl_top_bypass   :  1;
        unsigned int clkrst_flag_dram_bist_top_bypass :  1;
        unsigned int clkrst_flag_sec_reg_top_bypass   :  1;
        unsigned int clkrst_flag_dmac_top_bypass      :  1;
        unsigned int clkrst_flag_dmmu1_bypass         :  1;
        unsigned int clkrst_flag_dmmu0_bypass         :  1;
        unsigned int clkrst_flag_tcu_bypass           :  1;
        unsigned int clkrst_flag_ipcm1_bypass         :  1;
        unsigned int clkrst_flag_ipcm0_bypass         :  1;
        unsigned int clkrst_flag_uart_bypass          :  1;
        unsigned int clkrst_flag_wtd1_bypass          :  1;
        unsigned int clkrst_flag_wtd0_bypass          :  1;
        unsigned int clkrst_flag_timers_bypass        :  1;
        unsigned int clkrst_flag_ccd_bypass           :  1;
        unsigned int clkrst_flag_misc_bypass          :  1;
        unsigned int clkrst_flag_vmid10_bypass        :  1;
        unsigned int reserved_0                       :  5;
    } reg;
}npu_crg_npu_clkrst_flag_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_bus_tgt_no_pending_trans_data_bypass :  7;
        unsigned int npu_bus_ini_no_pending_trans_data_bypass : 13;
        unsigned int tcu_idle_flag_data_bypass                :  1;
        unsigned int tcu_idle_flag_perfstat_data_bypass       :  1;
        unsigned int vdsp_idle_flag_perfstat_data_bypass      :  1;
        unsigned int gp_idle_flag_perfstat_data_bypass        :  1;
        unsigned int dsa_idle_flag_perfstat_data_bypass       :  1;
        unsigned int xio_dma_idle_flag_perfstat_data_bypass   :  1;
        unsigned int cactive_inv_data_bypass                  :  1;
        unsigned int npu_vdsp_core_idle_data_bypass           :  1;
        unsigned int npu_vip_core_idle_data_bypass            :  1;
        unsigned int top_dma_idle_flag_bypass                 :  1;
        unsigned int npu_mcu_core_idle_data_bypass            :  1;
        unsigned int reserved_0                               :  1;
    } reg;
}npu_crg_autofsctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_npu_bus_data     : 10;
        unsigned int debounce_out_npu_bus_data    : 10;
        unsigned int debounce_bypass_npu_bus_data :  1;
        unsigned int div_auto_clk_npu_bus_data    :  6;
        unsigned int reserved_0                   :  4;
        unsigned int autofs_en_npu_bus_data       :  1;
    } reg;
}npu_crg_autofsctrl4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_mcu_core_idle_bypass :  1;
        unsigned int reserved_0               : 31;
    } reg;
}npu_crg_autofsctrl5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_mcu_core     : 10;
        unsigned int debounce_out_mcu_core    : 10;
        unsigned int debounce_bypass_mcu_core :  1;
        unsigned int div_auto_clk_mcu_core    :  6;
        unsigned int reserved_0               :  4;
        unsigned int autofs_en_mcu_core       :  1;
    } reg;
}npu_crg_autofsctrl6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cactive_inv_cfg_bypass                  :  1;
        unsigned int npu_bus_tgt_no_pending_trans_cfg_bypass :  1;
        unsigned int npu_bus_ini_no_pending_trans_cfg_bypass :  1;
        unsigned int idle_flag_axi_brg_bypass                :  1;
        unsigned int npu_vdsp_core_idle_cfg_bypass           :  1;
        unsigned int reserved_0                              : 27;
    } reg;
}npu_crg_autofsctrl7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_npu_bus_cfg     : 10;
        unsigned int debounce_out_npu_bus_cfg    : 10;
        unsigned int debounce_bypass_npu_bus_cfg :  1;
        unsigned int div_auto_clk_npu_bus_cfg    :  6;
        unsigned int reserved_0                  :  4;
        unsigned int autofs_en_npu_bus_cfg       :  1;
    } reg;
}npu_crg_autofsctrl8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vip_core_idle_bypass :  1;
        unsigned int reserved_0               : 31;
    } reg;
}npu_crg_autofsctrl9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_vip_core     : 10;
        unsigned int debounce_out_vip_core    : 10;
        unsigned int debounce_bypass_vip_core :  1;
        unsigned int div_auto_clk_vip_core    :  6;
        unsigned int reserved_0               :  4;
        unsigned int autofs_en_vip_core       :  1;
    } reg;
}npu_crg_autofsctrl10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_vdsp_core_idle_bypass :  1;
        unsigned int reserved_0                : 31;
    } reg;
}npu_crg_autofsctrl11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_vdsp_core     : 10;
        unsigned int debounce_out_vdsp_core    : 10;
        unsigned int debounce_bypass_vdsp_core :  1;
        unsigned int div_auto_clk_vdsp_core    :  6;
        unsigned int reserved_0                :  4;
        unsigned int autofs_en_vdsp_core       :  1;
    } reg;
}npu_crg_autofsctrl12_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_mcu_core_idle          :  1;
        unsigned int npu_vdsp_core_idle         :  1;
        unsigned int npu_vip_core_idle          :  1;
        unsigned int tcu_idle_flag              :  1;
        unsigned int tcu_idle_flag_perfstat     :  1;
        unsigned int vdsp_idle_flag_perfstat    :  1;
        unsigned int gp_idle_flag_perfstat      :  1;
        unsigned int dsa_idle_flag_perfstat     :  1;
        unsigned int xio_dma_idle_flag_perfstat :  1;
        unsigned int cactive_nic_inv            :  1;
        unsigned int cactive_noc_inv            :  1;
        unsigned int npu_bus_data_idle_state    :  1;
        unsigned int npu_bus_cfg_idle_state     :  1;
        unsigned int npu_mcu_core_idle_state    :  1;
        unsigned int npu_vdsp_core_idle_state   :  1;
        unsigned int npu_vip_core_idle_state    :  1;
        unsigned int top_dma_idle_flag          :  1;
        unsigned int idle_flag_axi_brg          :  1;
        unsigned int reserved_0                 : 14;
    } reg;
}npu_crg_idlestate0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vdcs_code_pg_core     :  2;
        unsigned int vdcs_code_rst_core    :  2;
        unsigned int drop_exit_hold_core   : 10;
        unsigned int cpu_wfx_bps_core      :  1;
        unsigned int l2_idle_bps_core      :  1;
        unsigned int dvfs_en_bps_core      :  1;
        unsigned int cpu_load_bps_core     :  1;
        unsigned int vbat_protect_bps_core :  1;
        unsigned int ccd_en_bps_core       :  1;
        unsigned int reserved0_core        : 12;
    } reg;
}npu_crg_core_vdcs_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vdcs_bps_core        :  1;
        unsigned int vdcs_en_hold_core    : 10;
        unsigned int gt_clk_vdcs_bps_core :  1;
        unsigned int dig_vbat_den_core    :  3;
        unsigned int dig_vbat_num_core    :  3;
        unsigned int dig_l2_den_core      :  3;
        unsigned int dig_l2_num_core      :  3;
        unsigned int wfx_core_num_core    :  2;
        unsigned int cs_lock_bps_core     :  1;
        unsigned int reserved1_core       :  5;
    } reg;
}npu_crg_core_vdcs_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cs_delayline_res_core   :  2;
        unsigned int cs_filtersel_core       :  3;
        unsigned int reserved8_core          :  2;
        unsigned int cs_lockdet_counter_core :  2;
        unsigned int cs_biassel_core         :  2;
        unsigned int cs_icpsel_core          :  2;
        unsigned int cs_freqsel_core         :  3;
        unsigned int lockdet_delay_core      :  2;
        unsigned int lockdet_path_core       :  2;
        unsigned int reserved2_core          : 12;
    } reg;
}npu_crg_core_vdcs_ctrl2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved3_core : 32;
    } reg;
}npu_crg_core_vdcs_ctrl3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vd_clk_drive_en_core :  1;
        unsigned int reserved4_core       : 31;
    } reg;
}npu_crg_core_vdcs_ctrl4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved5_core : 32;
    } reg;
}npu_crg_core_vdcs_ctrl5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vdcs_code_cdl_core  :  6;
        unsigned int vdcs_code_fdl_core  :  3;
        unsigned int vdcs_path_sel_core  :  2;
        unsigned int vdcs_threshold_core :  5;
        unsigned int _bm_                : 16;
    } reg;
}npu_crg_core_vdcs_ctrl6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vd_div_en_core          :  1;
        unsigned int stretch_active_en_core  :  1;
        unsigned int stretch_passive_en_core :  1;
        unsigned int reserved7_core          : 13;
        unsigned int _bm_                    : 16;
    } reg;
}npu_crg_core_vdcs_ctrl7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vd_out_core             :  6;
        unsigned int clock_stretch_flag_core :  2;
        unsigned int drop_detect_core        :  1;
        unsigned int vdcs_lock_core          :  1;
        unsigned int vdcs_over_core          :  1;
        unsigned int vdcs_under_core         :  1;
        unsigned int gt_clk_vdcs_core        :  1;
        unsigned int reserved_state0_core    : 19;
    } reg;
}npu_crg_core_vdcs_state0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int wfx_state_core          :  4;
        unsigned int l2_idle_state_core      :  1;
        unsigned int dvfs_en_state_core      :  1;
        unsigned int cpu_load_state_core     :  1;
        unsigned int core_idle_state_core    :  1;
        unsigned int vbat_protect_state_core :  1;
        unsigned int ccd_en_state_core       :  1;
        unsigned int reserved_state1_core    : 22;
    } reg;
}npu_crg_core_vdcs_state1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vdcs_code_pg_vdsp     :  2;
        unsigned int vdcs_code_rst_vdsp    :  2;
        unsigned int drop_exit_hold_vdsp   : 10;
        unsigned int cpu_wfx_bps_vdsp      :  1;
        unsigned int l2_idle_bps_vdsp      :  1;
        unsigned int dvfs_en_bps_vdsp      :  1;
        unsigned int cpu_load_bps_vdsp     :  1;
        unsigned int vbat_protect_bps_vdsp :  1;
        unsigned int ccd_en_bps_vdsp       :  1;
        unsigned int reserved0_vdsp        : 12;
    } reg;
}npu_crg_vdsp_vdcs_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vdcs_bps_vdsp        :  1;
        unsigned int vdcs_en_hold_vdsp    : 10;
        unsigned int gt_clk_vdcs_bps_vdsp :  1;
        unsigned int dig_vbat_den_vdsp    :  3;
        unsigned int dig_vbat_num_vdsp    :  3;
        unsigned int dig_l2_den_vdsp      :  3;
        unsigned int dig_l2_num_vdsp      :  3;
        unsigned int wfx_vdsp_num_vdsp    :  2;
        unsigned int cs_lock_bps_vdsp     :  1;
        unsigned int reserved1_vdsp       :  5;
    } reg;
}npu_crg_vdsp_vdcs_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cs_delayline_res_vdsp   :  2;
        unsigned int cs_filtersel_vdsp       :  3;
        unsigned int reserved8_vdsp          :  2;
        unsigned int cs_lockdet_counter_vdsp :  2;
        unsigned int cs_biassel_vdsp         :  2;
        unsigned int cs_icpsel_vdsp          :  2;
        unsigned int cs_freqsel_vdsp         :  3;
        unsigned int lockdet_delay_vdsp      :  2;
        unsigned int lockdet_path_vdsp       :  2;
        unsigned int reserved2_vdsp          : 12;
    } reg;
}npu_crg_vdsp_vdcs_ctrl2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved3_vdsp : 32;
    } reg;
}npu_crg_vdsp_vdcs_ctrl3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vd_clk_drive_en_vdsp :  1;
        unsigned int reserved4_vdsp       : 31;
    } reg;
}npu_crg_vdsp_vdcs_ctrl4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved5_vdsp : 32;
    } reg;
}npu_crg_vdsp_vdcs_ctrl5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vdcs_code_cdl_vdsp  :  6;
        unsigned int vdcs_code_fdl_vdsp  :  3;
        unsigned int vdcs_path_sel_vdsp  :  2;
        unsigned int vdcs_threshold_vdsp :  5;
        unsigned int _bm_                : 16;
    } reg;
}npu_crg_vdsp_vdcs_ctrl6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vd_div_en_vdsp          :  1;
        unsigned int stretch_active_en_vdsp  :  1;
        unsigned int stretch_passive_en_vdsp :  1;
        unsigned int reserved7_vdsp          : 13;
        unsigned int _bm_                    : 16;
    } reg;
}npu_crg_vdsp_vdcs_ctrl7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vd_out_vdsp             :  6;
        unsigned int clock_stretch_flag_vdsp :  2;
        unsigned int drop_detect_vdsp        :  1;
        unsigned int vdcs_lock_vdsp          :  1;
        unsigned int vdcs_over_vdsp          :  1;
        unsigned int vdcs_under_vdsp         :  1;
        unsigned int gt_clk_vdcs_vdsp        :  1;
        unsigned int reserved_state0_vdsp    : 19;
    } reg;
}npu_crg_vdsp_vdcs_state0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int wfx_state_vdsp          :  4;
        unsigned int l2_idle_state_vdsp      :  1;
        unsigned int dvfs_en_state_vdsp      :  1;
        unsigned int cpu_load_state_vdsp     :  1;
        unsigned int core_idle_state_vdsp    :  1;
        unsigned int vbat_protect_state_vdsp :  1;
        unsigned int ccd_en_state_vdsp       :  1;
        unsigned int reserved_state1_vdsp    : 22;
    } reg;
}npu_crg_vdsp_vdcs_state1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}npu_crg_regfile_cgbypass_t;

#endif
