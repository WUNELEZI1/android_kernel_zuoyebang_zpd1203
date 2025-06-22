// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef LPCTRL_GPU_REGIF_H
#define LPCTRL_GPU_REGIF_H
#define LPCTRL_GPU_GPU_TOP_GPC_PU_REQ_POLL                 0x0000
#define LPCTRL_GPU_GPU_TOP_GPC_PU_REQ_POLL_GPU_TOP_GPC_PU_REQ_POLL_SHIFT      0
#define LPCTRL_GPU_GPU_TOP_GPC_PU_REQ_POLL_GPU_TOP_GPC_PU_REQ_POLL_MASK       0x000000ff
#define LPCTRL_GPU_GPU_TOP_GPC_REQ_POLL_MSK                0x0004
#define LPCTRL_GPU_GPU_TOP_GPC_REQ_POLL_MSK_GPU_TOP_GPC_PU_REQ_POLL_MSK_SHIFT 0
#define LPCTRL_GPU_GPU_TOP_GPC_REQ_POLL_MSK_GPU_TOP_GPC_PU_REQ_POLL_MSK_MASK  0x000000ff
#define LPCTRL_GPU_INTR_CLR_PU_GPU_TOP                     0x0008
#define LPCTRL_GPU_INTR_CLR_PU_GPU_TOP_GPU_TOP_INTR_CLR_PU_GPC_SHIFT          0
#define LPCTRL_GPU_INTR_CLR_PU_GPU_TOP_GPU_TOP_INTR_CLR_PU_GPC_MASK           0x00000001
#define LPCTRL_GPU_INTR_CLR_PU_GPU_TOP_GPU_TOP_INTR_CLR_PD_GPC_SHIFT          1
#define LPCTRL_GPU_INTR_CLR_PU_GPU_TOP_GPU_TOP_INTR_CLR_PD_GPC_MASK           0x00000002
#define LPCTRL_GPU_INTR_PU_GPU_TOP                         0x000c
#define LPCTRL_GPU_INTR_PU_GPU_TOP_GPU_TOP_INTR_PU_GPC_SHIFT                  0
#define LPCTRL_GPU_INTR_PU_GPU_TOP_GPU_TOP_INTR_PU_GPC_MASK                   0x00000001
#define LPCTRL_GPU_INTR_PU_GPU_TOP_GPU_TOP_INTR_PD_GPC_SHIFT                  1
#define LPCTRL_GPU_INTR_PU_GPU_TOP_GPU_TOP_INTR_PD_GPC_MASK                   0x00000002
#define LPCTRL_GPU_INTR_PU_GPU_TOP_GPU_TOP_PWR_ON_DONE_SHIFT                  2
#define LPCTRL_GPU_INTR_PU_GPU_TOP_GPU_TOP_PWR_ON_DONE_MASK                   0x00000004
#define LPCTRL_GPU_INTR_PU_GPU_TOP_GPU_TOP_PWR_OFF_DONE_SHIFT                 3
#define LPCTRL_GPU_INTR_PU_GPU_TOP_GPU_TOP_PWR_OFF_DONE_MASK                  0x00000008
#define LPCTRL_GPU_GPU_TOP_GPC_STAT_0                      0x0014
#define LPCTRL_GPU_GPU_TOP_GPC_STAT_1                      0x0018
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16             0x0100
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_MTCOMS_EN_SHIFT         0
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_MTCOMS_EN_MASK          0x00000001
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_ISO_EN_SHIFT            1
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_ISO_EN_MASK             0x00000002
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_MRB_REQ_SHIFT           2
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_MRB_REQ_MASK            0x00000004
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_TOP_MEM_SD_SHIFT        3
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_TOP_MEM_SD_MASK         0x00000008
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_TOP_MEM_POFF_SHIFT      4
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_TOP_MEM_POFF_MASK       0x00000030
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_BUS_IDLE_REQ_SHIFT      6
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_BUS_IDLE_REQ_MASK       0x00000040
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_TOP_MEM_DS_SHIFT        7
#define LPCTRL_GPU_GPU_GPC_SOFT_CTRL_LP_SIG_16_SC_GPU_TOP_MEM_DS_MASK         0x00000080
#define LPCTRL_GPU_GPC_GPU_MEM_CTRL                        0x0104
#define LPCTRL_GPU_GPC_GPU_MEM_CTRL_GPU_MEM_SD_REQ_SHIFT                      0
#define LPCTRL_GPU_GPC_GPU_MEM_CTRL_GPU_MEM_SD_REQ_MASK                       0x00000001
#define LPCTRL_GPU_GPC_GPU_MEM_CTRL_GPU_MEM_POFF_REQ_SHIFT                    1
#define LPCTRL_GPU_GPC_GPU_MEM_CTRL_GPU_MEM_POFF_REQ_MASK                     0x00000006
#define LPCTRL_GPU_GPU_TOP_LP_STATUS                       0x0108
#define LPCTRL_GPU_GPU_TOP_LP_STATUS_GPU_TOP_MEM_DS_ACK_SHIFT                 0
#define LPCTRL_GPU_GPU_TOP_LP_STATUS_GPU_TOP_MEM_DS_ACK_MASK                  0x00000001
#define LPCTRL_GPU_GPU_TOP_LP_STATUS_GPU_TOP_MEM_SD_ACK_SHIFT                 1
#define LPCTRL_GPU_GPU_TOP_LP_STATUS_GPU_TOP_MEM_SD_ACK_MASK                  0x00000002
#define LPCTRL_GPU_GPU_TOP_LP_STATUS_GPU_MTCMOS_ACK_SHIFT                     2
#define LPCTRL_GPU_GPU_TOP_LP_STATUS_GPU_MTCMOS_ACK_MASK                      0x00000004
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG0               0x0120
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG0_GPU_CORE_POFF_DELAY_SHIFT        0
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG0_GPU_CORE_POFF_DELAY_MASK         0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG0_GPU_CORE_SD_DELAY_SHIFT          16
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG0_GPU_CORE_SD_DELAY_MASK           0xffff0000
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG1               0x0124
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG1_GPU_CORE_DS_DELAY_SHIFT          0
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG1_GPU_CORE_DS_DELAY_MASK           0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG1_GPU_CORE_ISO_DELAY_SHIFT         16
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG1_GPU_CORE_ISO_DELAY_MASK          0xffff0000
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG2               0x0128
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG2_GPU_CORE_MEM_REPAIR_DELAY_SHIFT  0
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG2_GPU_CORE_MEM_REPAIR_DELAY_MASK   0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG2_GPU_CORE_RTU_ISO_DELAY_SHIFT     16
#define LPCTRL_GPU_GPU_LP_CTRL_DELAY_CONFIG2_GPU_CORE_RTU_ISO_DELAY_MASK      0xffff0000
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_0                       0x0130
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_0_SC_GPU_CORE_MTCMOS_EN_SHIFT              0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_0_SC_GPU_CORE_MTCMOS_EN_MASK               0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_1                       0x0134
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_1_SC_GPU_CORE0_7_MEM_POFF_SHIFT            0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_1_SC_GPU_CORE0_7_MEM_POFF_MASK             0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_2                       0x0138
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_2_SC_GPU_CORE8_15_MEM_POFF_SHIFT           0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_2_SC_GPU_CORE8_15_MEM_POFF_MASK            0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_3                       0x013c
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_3_SC_GPU_CORE_MEM_SD_SHIFT                 0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_3_SC_GPU_CORE_MEM_SD_MASK                  0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_4                       0x0140
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_4_SC_GPU_CORE_MEM_DS_SHIFT                 0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_4_SC_GPU_CORE_MEM_DS_MASK                  0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_5                       0x0144
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_5_SC_GPU_CORE_ISO_EN_SHIFT                 0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_5_SC_GPU_CORE_ISO_EN_MASK                  0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_6                       0x0148
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_6_SC_GPU_CORE_MODE_SEL_SHIFT               0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_6_SC_GPU_CORE_MODE_SEL_MASK                0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_7                       0x014c
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_7_SC_GPU_CORE_RTU_MTCMOS_EN_SHIFT          0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_7_SC_GPU_CORE_RTU_MTCMOS_EN_MASK           0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_8                       0x0150
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_8_SC_GPU_CORE_RTU_ISO_EN_SHIFT             0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_8_SC_GPU_CORE_RTU_ISO_EN_MASK              0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_9                       0x0154
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_9_SC_GPU_CORE_RTU_MODE_SEL_SHIFT           0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_9_SC_GPU_CORE_RTU_MODE_SEL_MASK            0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_0                  0x0160
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_0_GPU_CORE_MEM_SD_ACK_SHIFT           0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_0_GPU_CORE_MEM_SD_ACK_MASK            0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_0_GPU_CORE_MEM_DS_ACK_SHIFT           16
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_0_GPU_CORE_MEM_DS_ACK_MASK            0xffff0000
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_1                  0x0164
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_1_GPU_CORE_MTCMOS_ACK_SHIFT           0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_1_GPU_CORE_MTCMOS_ACK_MASK            0x0000ffff
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_3                  0x016c
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_3_GPU_CORE_RTU_MTCMOS_ACK_SHIFT       0
#define LPCTRL_GPU_GPU_LP_CTRL_SIG_READ_3_GPU_CORE_RTU_MTCMOS_ACK_MASK        0x0000ffff
#define LPCTRL_GPU_GPU_CORE_TIMEROUT_SIG                   0x0170
#define LPCTRL_GPU_GPU_CORE_TIMEROUT_SIG_GPU_CORE_MEM_REPAIR_TIMEOUT_SHIFT    0
#define LPCTRL_GPU_GPU_CORE_TIMEROUT_SIG_GPU_CORE_MEM_REPAIR_TIMEOUT_MASK     0x0000ffff
#define LPCTRL_GPU_GPU_CORE_TIMEROUT_SIG_GPU_CORE_SD_ACK_TIMEOUT_SHIFT        16
#define LPCTRL_GPU_GPU_CORE_TIMEROUT_SIG_GPU_CORE_SD_ACK_TIMEOUT_MASK         0xffff0000
#define LPCTRL_GPU_GPU_CORE_CLR_TIMEROUT_SIG               0x0174
#define LPCTRL_GPU_GPU_CORE_CLR_TIMEROUT_SIG_GPU_CORE_TIMEOUT_CLR_SHIFT       0
#define LPCTRL_GPU_GPU_CORE_CLR_TIMEROUT_SIG_GPU_CORE_TIMEOUT_CLR_MASK        0x0000ffff
#define LPCTRL_GPU_MEM_LPCTRL_GROUP0                       0x0190
#define LPCTRL_GPU_MEM_LPCTRL_GROUP0_SC_GPU_BUS_MEM_DS_SHIFT                  0
#define LPCTRL_GPU_MEM_LPCTRL_GROUP0_SC_GPU_BUS_MEM_DS_MASK                   0x00000001
#define LPCTRL_GPU_MEM_LPCTRL_GROUP0_SC_GPU_BUS_MEM_SD_SHIFT                  1
#define LPCTRL_GPU_MEM_LPCTRL_GROUP0_SC_GPU_BUS_MEM_SD_MASK                   0x00000002
#define LPCTRL_GPU_MEM_LPCTRL_GROUP0_SC_GPU_BUS_MEM_POFF_SHIFT                2
#define LPCTRL_GPU_MEM_LPCTRL_GROUP0_SC_GPU_BUS_MEM_POFF_MASK                 0x0000000c
#define LPCTRL_GPU_MEM_LPSTATUS0                           0x0194
#define LPCTRL_GPU_MEM_LPSTATUS0_GPU_BUS_MEM_DS_ACK_SHIFT                     0
#define LPCTRL_GPU_MEM_LPSTATUS0_GPU_BUS_MEM_DS_ACK_MASK                      0x00000001
#define LPCTRL_GPU_MEM_LPSTATUS0_GPU_BUS_MEM_SD_ACK_SHIFT                     1
#define LPCTRL_GPU_MEM_LPSTATUS0_GPU_BUS_MEM_SD_ACK_MASK                      0x00000002
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_0            0x01a0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_0_GPU_CORE0_LPCTRL_STATUS_SHIFT 0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_0_GPU_CORE0_LPCTRL_STATUS_MASK  0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_0_GPU_CORE1_LPCTRL_STATUS_SHIFT 10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_0_GPU_CORE1_LPCTRL_STATUS_MASK  0x000ffc00
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_0_GPU_CORE2_LPCTRL_STATUS_SHIFT 20
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_0_GPU_CORE2_LPCTRL_STATUS_MASK  0x3ff00000
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_1            0x01a4
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_1_GPU_CORE3_LPCTRL_STATUS_SHIFT 0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_1_GPU_CORE3_LPCTRL_STATUS_MASK  0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_1_GPU_CORE4_LPCTRL_STATUS_SHIFT 10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_1_GPU_CORE4_LPCTRL_STATUS_MASK  0x000ffc00
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_1_GPU_CORE5_LPCTRL_STATUS_SHIFT 20
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_1_GPU_CORE5_LPCTRL_STATUS_MASK  0x3ff00000
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_2            0x01a8
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_2_GPU_CORE6_LPCTRL_STATUS_SHIFT 0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_2_GPU_CORE6_LPCTRL_STATUS_MASK  0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_2_GPU_CORE7_LPCTRL_STATUS_SHIFT 10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_2_GPU_CORE7_LPCTRL_STATUS_MASK  0x000ffc00
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_2_GPU_CORE8_LPCTRL_STATUS_SHIFT 20
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_2_GPU_CORE8_LPCTRL_STATUS_MASK  0x3ff00000
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_3            0x01ac
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_3_GPU_CORE9_LPCTRL_STATUS_SHIFT 0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_3_GPU_CORE9_LPCTRL_STATUS_MASK  0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_3_GPU_CORE10_LPCTRL_STATUS_SHIFT 10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_3_GPU_CORE10_LPCTRL_STATUS_MASK  0x000ffc00
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_3_GPU_CORE11_LPCTRL_STATUS_SHIFT 20
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_3_GPU_CORE11_LPCTRL_STATUS_MASK  0x3ff00000
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_4            0x01b0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_4_GPU_CORE12_LPCTRL_STATUS_SHIFT 0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_4_GPU_CORE12_LPCTRL_STATUS_MASK  0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_4_GPU_CORE13_LPCTRL_STATUS_SHIFT 10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_4_GPU_CORE13_LPCTRL_STATUS_MASK  0x000ffc00
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_4_GPU_CORE14_LPCTRL_STATUS_SHIFT 20
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_4_GPU_CORE14_LPCTRL_STATUS_MASK  0x3ff00000
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_5            0x01b4
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_5_GPU_CORE15_LPCTRL_STATUS_SHIFT 0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_5_GPU_CORE15_LPCTRL_STATUS_MASK  0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_5_GPU_RTU0_LPCTRL_STATUS_SHIFT  10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_5_GPU_RTU0_LPCTRL_STATUS_MASK   0x000ffc00
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_5_GPU_RTU1_LPCTRL_STATUS_SHIFT  20
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_5_GPU_RTU1_LPCTRL_STATUS_MASK   0x3ff00000
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_6            0x01b8
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_6_GPU_RTU2_LPCTRL_STATUS_SHIFT  0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_6_GPU_RTU2_LPCTRL_STATUS_MASK   0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_6_GPU_RTU3_LPCTRL_STATUS_SHIFT  10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_6_GPU_RTU3_LPCTRL_STATUS_MASK   0x000ffc00
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_6_GPU_RTU4_LPCTRL_STATUS_SHIFT  20
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_6_GPU_RTU4_LPCTRL_STATUS_MASK   0x3ff00000
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_7            0x01bc
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_7_GPU_RTU5_LPCTRL_STATUS_SHIFT  0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_7_GPU_RTU5_LPCTRL_STATUS_MASK   0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_7_GPU_RTU6_LPCTRL_STATUS_SHIFT  10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_7_GPU_RTU6_LPCTRL_STATUS_MASK   0x000ffc00
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_7_GPU_RTU7_LPCTRL_STATUS_SHIFT  20
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_7_GPU_RTU7_LPCTRL_STATUS_MASK   0x3ff00000
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_8            0x01c0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_8_GPU_RTU8_LPCTRL_STATUS_SHIFT  0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_8_GPU_RTU8_LPCTRL_STATUS_MASK   0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_8_GPU_RTU9_LPCTRL_STATUS_SHIFT  10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_8_GPU_RTU9_LPCTRL_STATUS_MASK   0x000ffc00
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_8_GPU_RTU10_LPCTRL_STATUS_SHIFT 20
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_8_GPU_RTU10_LPCTRL_STATUS_MASK  0x3ff00000
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_9            0x01c4
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_9_GPU_RTU11_LPCTRL_STATUS_SHIFT 0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_9_GPU_RTU11_LPCTRL_STATUS_MASK  0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_9_GPU_RTU12_LPCTRL_STATUS_SHIFT 10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_9_GPU_RTU12_LPCTRL_STATUS_MASK  0x000ffc00
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_9_GPU_RTU13_LPCTRL_STATUS_SHIFT 20
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_9_GPU_RTU13_LPCTRL_STATUS_MASK  0x3ff00000
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_10           0x01c8
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_10_GPU_RTU14_LPCTRL_STATUS_SHIFT 0
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_10_GPU_RTU14_LPCTRL_STATUS_MASK  0x000003ff
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_10_GPU_RTU15_LPCTRL_STATUS_SHIFT 10
#define LPCTRL_GPU_GPU_CORE_RTU_LPCTRL_STATUS_10_GPU_RTU15_LPCTRL_STATUS_MASK  0x000ffc00
#define LPCTRL_GPU_GPU_TOP_GPC_EN                          0x0800
#define LPCTRL_GPU_GPU_TOP_GPC_EN_GPU_GPC_EN_SHIFT                            0
#define LPCTRL_GPU_GPU_TOP_GPC_EN_GPU_GPC_EN_MASK                             0x00000001
#define LPCTRL_GPU_GPU_TOP_GPC_CTRL_0                      0x0804
#define LPCTRL_GPU_GPU_TOP_GPC_CTRL_1                      0x0808
#define LPCTRL_GPU_GPU_GPC_STAT_DEBUG_1                    0x0814
#define LPCTRL_GPU_GPU_GPC_STAT_DEBUG_1_GPU_TOP_GPC_STATE_DEBUG_SHIFT         0
#define LPCTRL_GPU_GPU_GPC_STAT_DEBUG_1_GPU_TOP_GPC_STATE_DEBUG_MASK          0x00000003
#define LPCTRL_GPU_GPU_HPM0_CFG_0                          0x0c00
#define LPCTRL_GPU_GPU_HPM0_CFG_0_GPU_HPM0_CLEAR_SHIFT                        0
#define LPCTRL_GPU_GPU_HPM0_CFG_0_GPU_HPM0_CLEAR_MASK                         0x00000001
#define LPCTRL_GPU_GPU_HPM0_CFG_0_GPU_HPM0_RO_SEL_SHIFT                       1
#define LPCTRL_GPU_GPU_HPM0_CFG_0_GPU_HPM0_RO_SEL_MASK                        0x0000007e
#define LPCTRL_GPU_GPU_HPM0_CFG_0_GPU_HPM0_RO_CLK_DIV_SHIFT                   7
#define LPCTRL_GPU_GPU_HPM0_CFG_0_GPU_HPM0_RO_CLK_DIV_MASK                    0x00000180
#define LPCTRL_GPU_GPU_HPM0_CFG_0_GPU_HPM0_RO_DBG_DIV_SHIFT                   9
#define LPCTRL_GPU_GPU_HPM0_CFG_0_GPU_HPM0_RO_DBG_DIV_MASK                    0x00007e00
#define LPCTRL_GPU_GPU_HPM0_CFG_1                          0x0c04
#define LPCTRL_GPU_GPU_HPM0_CFG_1_GPU_HPM0_RO_EN_SHIFT                        0
#define LPCTRL_GPU_GPU_HPM0_CFG_1_GPU_HPM0_RO_EN_MASK                         0x00000001
#define LPCTRL_GPU_GPU_HPM0_CFG_1_GPU_HPM0_TEST_HW_EN_SHIFT                   1
#define LPCTRL_GPU_GPU_HPM0_CFG_1_GPU_HPM0_TEST_HW_EN_MASK                    0x00000002
#define LPCTRL_GPU_GPU_HPM0_CFG_1_GPU_HPM0_TEST_HW_START_SHIFT                2
#define LPCTRL_GPU_GPU_HPM0_CFG_1_GPU_HPM0_TEST_HW_START_MASK                 0x00000004
#define LPCTRL_GPU_GPU_HPM0_CFG_1_GPU_HPM0_TIMER_CNT_SHIFT                    3
#define LPCTRL_GPU_GPU_HPM0_CFG_1_GPU_HPM0_TIMER_CNT_MASK                     0x000007f8
#define LPCTRL_GPU_GPU_HPM0_CLK_GT_W1S                     0x0c08
#define LPCTRL_GPU_GPU_HPM0_CLK_GT_W1S_GPU_HPM0_CLK_GT_SHIFT                  0
#define LPCTRL_GPU_GPU_HPM0_CLK_GT_W1C                     0x0c0c
#define LPCTRL_GPU_GPU_HPM0_CLK_GT_W1C_GPU_HPM0_CLK_GT_SHIFT                  0
#define LPCTRL_GPU_GPU_HPM0_CLK_GT_RO                      0x0c10
#define LPCTRL_GPU_GPU_HPM0_CLK_GT_RO_GPU_HPM0_CLK_GT_SHIFT                   0
#define LPCTRL_GPU_GPU_HPM0_CLK_GT_RO_GPU_HPM0_CLK_GT_MASK                    0x00000001
#define LPCTRL_GPU_GPU_HPM0_DATA_OUT                       0x0c14
#define LPCTRL_GPU_GPU_HPM0_DATA_OUT_GPU_HPM0_DATA_SHIFT                      0
#define LPCTRL_GPU_GPU_HPM0_DATA_OUT_GPU_HPM0_DATA_MASK                       0x0003ffff
#define LPCTRL_GPU_GPU_HPM1_CFG_0                          0x0c20
#define LPCTRL_GPU_GPU_HPM1_CFG_0_GPU_HPM1_CLEAR_SHIFT                        0
#define LPCTRL_GPU_GPU_HPM1_CFG_0_GPU_HPM1_CLEAR_MASK                         0x00000001
#define LPCTRL_GPU_GPU_HPM1_CFG_0_GPU_HPM1_RO_SEL_SHIFT                       1
#define LPCTRL_GPU_GPU_HPM1_CFG_0_GPU_HPM1_RO_SEL_MASK                        0x0000007e
#define LPCTRL_GPU_GPU_HPM1_CFG_0_GPU_HPM1_RO_CLK_DIV_SHIFT                   7
#define LPCTRL_GPU_GPU_HPM1_CFG_0_GPU_HPM1_RO_CLK_DIV_MASK                    0x00000180
#define LPCTRL_GPU_GPU_HPM1_CFG_0_GPU_HPM1_RO_DBG_DIV_SHIFT                   9
#define LPCTRL_GPU_GPU_HPM1_CFG_0_GPU_HPM1_RO_DBG_DIV_MASK                    0x00007e00
#define LPCTRL_GPU_GPU_HPM1_CFG_1                          0x0c24
#define LPCTRL_GPU_GPU_HPM1_CFG_1_GPU_HPM1_RO_EN_SHIFT                        0
#define LPCTRL_GPU_GPU_HPM1_CFG_1_GPU_HPM1_RO_EN_MASK                         0x00000001
#define LPCTRL_GPU_GPU_HPM1_CFG_1_GPU_HPM1_TEST_HW_EN_SHIFT                   1
#define LPCTRL_GPU_GPU_HPM1_CFG_1_GPU_HPM1_TEST_HW_EN_MASK                    0x00000002
#define LPCTRL_GPU_GPU_HPM1_CFG_1_GPU_HPM1_TEST_HW_START_SHIFT                2
#define LPCTRL_GPU_GPU_HPM1_CFG_1_GPU_HPM1_TEST_HW_START_MASK                 0x00000004
#define LPCTRL_GPU_GPU_HPM1_CFG_1_GPU_HPM1_TIMER_CNT_SHIFT                    3
#define LPCTRL_GPU_GPU_HPM1_CFG_1_GPU_HPM1_TIMER_CNT_MASK                     0x000007f8
#define LPCTRL_GPU_GPU_HPM1_CLK_GT_W1S                     0x0c28
#define LPCTRL_GPU_GPU_HPM1_CLK_GT_W1S_GPU_HPM1_CLK_GT_SHIFT                  0
#define LPCTRL_GPU_GPU_HPM1_CLK_GT_W1C                     0x0c2c
#define LPCTRL_GPU_GPU_HPM1_CLK_GT_W1C_GPU_HPM1_CLK_GT_SHIFT                  0
#define LPCTRL_GPU_GPU_HPM1_CLK_GT_RO                      0x0c30
#define LPCTRL_GPU_GPU_HPM1_CLK_GT_RO_GPU_HPM1_CLK_GT_SHIFT                   0
#define LPCTRL_GPU_GPU_HPM1_CLK_GT_RO_GPU_HPM1_CLK_GT_MASK                    0x00000001
#define LPCTRL_GPU_GPU_HPM1_DATA_OUT                       0x0c34
#define LPCTRL_GPU_GPU_HPM1_DATA_OUT_GPU_HPM1_DATA_SHIFT                      0
#define LPCTRL_GPU_GPU_HPM1_DATA_OUT_GPU_HPM1_DATA_MASK                       0x0003ffff
#define LPCTRL_GPU_GPU_HPM2_CFG_0                          0x0c40
#define LPCTRL_GPU_GPU_HPM2_CFG_0_GPU_HPM2_CLEAR_SHIFT                        0
#define LPCTRL_GPU_GPU_HPM2_CFG_0_GPU_HPM2_CLEAR_MASK                         0x00000001
#define LPCTRL_GPU_GPU_HPM2_CFG_0_GPU_HPM2_RO_SEL_SHIFT                       1
#define LPCTRL_GPU_GPU_HPM2_CFG_0_GPU_HPM2_RO_SEL_MASK                        0x0000007e
#define LPCTRL_GPU_GPU_HPM2_CFG_0_GPU_HPM2_RO_CLK_DIV_SHIFT                   7
#define LPCTRL_GPU_GPU_HPM2_CFG_0_GPU_HPM2_RO_CLK_DIV_MASK                    0x00000180
#define LPCTRL_GPU_GPU_HPM2_CFG_0_GPU_HPM2_RO_DBG_DIV_SHIFT                   9
#define LPCTRL_GPU_GPU_HPM2_CFG_0_GPU_HPM2_RO_DBG_DIV_MASK                    0x00007e00
#define LPCTRL_GPU_GPU_HPM2_CFG_1                          0x0c44
#define LPCTRL_GPU_GPU_HPM2_CFG_1_GPU_HPM2_RO_EN_SHIFT                        0
#define LPCTRL_GPU_GPU_HPM2_CFG_1_GPU_HPM2_RO_EN_MASK                         0x00000001
#define LPCTRL_GPU_GPU_HPM2_CFG_1_GPU_HPM2_TEST_HW_EN_SHIFT                   1
#define LPCTRL_GPU_GPU_HPM2_CFG_1_GPU_HPM2_TEST_HW_EN_MASK                    0x00000002
#define LPCTRL_GPU_GPU_HPM2_CFG_1_GPU_HPM2_TEST_HW_START_SHIFT                2
#define LPCTRL_GPU_GPU_HPM2_CFG_1_GPU_HPM2_TEST_HW_START_MASK                 0x00000004
#define LPCTRL_GPU_GPU_HPM2_CFG_1_GPU_HPM2_TIMER_CNT_SHIFT                    3
#define LPCTRL_GPU_GPU_HPM2_CFG_1_GPU_HPM2_TIMER_CNT_MASK                     0x000007f8
#define LPCTRL_GPU_GPU_HPM2_CLK_GT_W1S                     0x0c48
#define LPCTRL_GPU_GPU_HPM2_CLK_GT_W1S_GPU_HPM2_CLK_GT_SHIFT                  0
#define LPCTRL_GPU_GPU_HPM2_CLK_GT_W1C                     0x0c4c
#define LPCTRL_GPU_GPU_HPM2_CLK_GT_W1C_GPU_HPM2_CLK_GT_SHIFT                  0
#define LPCTRL_GPU_GPU_HPM2_CLK_GT_RO                      0x0c50
#define LPCTRL_GPU_GPU_HPM2_CLK_GT_RO_GPU_HPM2_CLK_GT_SHIFT                   0
#define LPCTRL_GPU_GPU_HPM2_CLK_GT_RO_GPU_HPM2_CLK_GT_MASK                    0x00000001
#define LPCTRL_GPU_GPU_HPM2_DATA_OUT                       0x0c54
#define LPCTRL_GPU_GPU_HPM2_DATA_OUT_GPU_HPM2_DATA_SHIFT                      0
#define LPCTRL_GPU_GPU_HPM2_DATA_OUT_GPU_HPM2_DATA_MASK                       0x0003ffff
#define LPCTRL_GPU_GPU_HPM3_CFG_0                          0x0c60
#define LPCTRL_GPU_GPU_HPM3_CFG_0_GPU_HPM3_CLEAR_SHIFT                        0
#define LPCTRL_GPU_GPU_HPM3_CFG_0_GPU_HPM3_CLEAR_MASK                         0x00000001
#define LPCTRL_GPU_GPU_HPM3_CFG_0_GPU_HPM3_RO_SEL_SHIFT                       1
#define LPCTRL_GPU_GPU_HPM3_CFG_0_GPU_HPM3_RO_SEL_MASK                        0x0000007e
#define LPCTRL_GPU_GPU_HPM3_CFG_0_GPU_HPM3_RO_CLK_DIV_SHIFT                   7
#define LPCTRL_GPU_GPU_HPM3_CFG_0_GPU_HPM3_RO_CLK_DIV_MASK                    0x00000180
#define LPCTRL_GPU_GPU_HPM3_CFG_0_GPU_HPM3_RO_DBG_DIV_SHIFT                   9
#define LPCTRL_GPU_GPU_HPM3_CFG_0_GPU_HPM3_RO_DBG_DIV_MASK                    0x00007e00
#define LPCTRL_GPU_GPU_HPM3_CFG_1                          0x0c64
#define LPCTRL_GPU_GPU_HPM3_CFG_1_GPU_HPM3_RO_EN_SHIFT                        0
#define LPCTRL_GPU_GPU_HPM3_CFG_1_GPU_HPM3_RO_EN_MASK                         0x00000001
#define LPCTRL_GPU_GPU_HPM3_CFG_1_GPU_HPM3_TEST_HW_EN_SHIFT                   1
#define LPCTRL_GPU_GPU_HPM3_CFG_1_GPU_HPM3_TEST_HW_EN_MASK                    0x00000002
#define LPCTRL_GPU_GPU_HPM3_CFG_1_GPU_HPM3_TEST_HW_START_SHIFT                2
#define LPCTRL_GPU_GPU_HPM3_CFG_1_GPU_HPM3_TEST_HW_START_MASK                 0x00000004
#define LPCTRL_GPU_GPU_HPM3_CFG_1_GPU_HPM3_TIMER_CNT_SHIFT                    3
#define LPCTRL_GPU_GPU_HPM3_CFG_1_GPU_HPM3_TIMER_CNT_MASK                     0x000007f8
#define LPCTRL_GPU_GPU_HPM3_CLK_GT_W1S                     0x0c68
#define LPCTRL_GPU_GPU_HPM3_CLK_GT_W1S_GPU_HPM3_CLK_GT_SHIFT                  0
#define LPCTRL_GPU_GPU_HPM3_CLK_GT_W1C                     0x0c6c
#define LPCTRL_GPU_GPU_HPM3_CLK_GT_W1C_GPU_HPM3_CLK_GT_SHIFT                  0
#define LPCTRL_GPU_GPU_HPM3_CLK_GT_RO                      0x0c70
#define LPCTRL_GPU_GPU_HPM3_CLK_GT_RO_GPU_HPM3_CLK_GT_SHIFT                   0
#define LPCTRL_GPU_GPU_HPM3_CLK_GT_RO_GPU_HPM3_CLK_GT_MASK                    0x00000001
#define LPCTRL_GPU_GPU_HPM3_DATA_OUT                       0x0c74
#define LPCTRL_GPU_GPU_HPM3_DATA_OUT_GPU_HPM3_DATA_SHIFT                      0
#define LPCTRL_GPU_GPU_HPM3_DATA_OUT_GPU_HPM3_DATA_MASK                       0x0003ffff
#define LPCTRL_GPU_GPU_PROCM_CFG_0                         0x0c80
#define LPCTRL_GPU_GPU_PROCM_CFG_0_GPU_PROCM_CLEAR_SHIFT                      0
#define LPCTRL_GPU_GPU_PROCM_CFG_0_GPU_PROCM_CLEAR_MASK                       0x00000001
#define LPCTRL_GPU_GPU_PROCM_CFG_0_GPU_PROCM_RO_SEL_SHIFT                     1
#define LPCTRL_GPU_GPU_PROCM_CFG_0_GPU_PROCM_RO_SEL_MASK                      0x000000fe
#define LPCTRL_GPU_GPU_PROCM_CFG_0_GPU_PROCM_TIMER_SEL_SHIFT                  8
#define LPCTRL_GPU_GPU_PROCM_CFG_0_GPU_PROCM_TIMER_SEL_MASK                   0x00000300
#define LPCTRL_GPU_GPU_PROCM_CFG_0_GPU_PROCM_RO_CLK_DIV_SHIFT                 10
#define LPCTRL_GPU_GPU_PROCM_CFG_0_GPU_PROCM_RO_CLK_DIV_MASK                  0x00000400
#define LPCTRL_GPU_GPU_PROCM_CFG_1                         0x0c84
#define LPCTRL_GPU_GPU_PROCM_CFG_1_GPU_PROCM_RO_EN_SHIFT                      0
#define LPCTRL_GPU_GPU_PROCM_CFG_1_GPU_PROCM_RO_EN_MASK                       0x00000001
#define LPCTRL_GPU_GPU_PROCM_CFG_1_GPU_PROCM_TEST_HW_EN_SHIFT                 1
#define LPCTRL_GPU_GPU_PROCM_CFG_1_GPU_PROCM_TEST_HW_EN_MASK                  0x00000002
#define LPCTRL_GPU_GPU_PROCM_CFG_1_GPU_PROCM_TEST_HW_START_SHIFT              2
#define LPCTRL_GPU_GPU_PROCM_CFG_1_GPU_PROCM_TEST_HW_START_MASK               0x00000004
#define LPCTRL_GPU_GPU_PROCM_CFG_1_GPU_PROCM_RO_DBG_DIV_SHIFT                 3
#define LPCTRL_GPU_GPU_PROCM_CFG_1_GPU_PROCM_RO_DBG_DIV_MASK                  0x000001f8
#define LPCTRL_GPU_GPU_PROCM_CFG_1_GPU_PROCM_SEL_H_SHIFT                      9
#define LPCTRL_GPU_GPU_PROCM_CFG_1_GPU_PROCM_SEL_H_MASK                       0x00000200
#define LPCTRL_GPU_GPU_PROCM_CLK_GT_W1S                    0x0c88
#define LPCTRL_GPU_GPU_PROCM_CLK_GT_W1S_GPU_PROCM_CLK_GT_SHIFT                0
#define LPCTRL_GPU_GPU_PROCM_CLK_GT_W1C                    0x0c8c
#define LPCTRL_GPU_GPU_PROCM_CLK_GT_W1C_GPU_PROCM_CLK_GT_SHIFT                0
#define LPCTRL_GPU_GPU_PROCM_CLK_GT_RO                     0x0c90
#define LPCTRL_GPU_GPU_PROCM_CLK_GT_RO_GPU_PROCM_CLK_GT_SHIFT                 0
#define LPCTRL_GPU_GPU_PROCM_CLK_GT_RO_GPU_PROCM_CLK_GT_MASK                  0x00000001
#define LPCTRL_GPU_GPU_PROCM_DATA_OUT                      0x0c94
#define LPCTRL_GPU_GPU_PROCM_DATA_OUT_GPU_PROCM_DATA_SHIFT                    0
#define LPCTRL_GPU_GPU_PROCM_DATA_OUT_GPU_PROCM_DATA_MASK                     0x01ffffff
#define LPCTRL_GPU_REGFILE_CGBYPASS                        0x0c98
#define LPCTRL_GPU_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                    0
#define LPCTRL_GPU_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                     0x00000001
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_top_gpc_pu_req_poll :  8;
        unsigned int reserved_0              :  8;
        unsigned int _bm_                    : 16;
    } reg;
}lpctrl_gpu_gpu_top_gpc_pu_req_poll_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_top_gpc_pu_req_poll_msk :  8;
        unsigned int reserved_0                  :  8;
        unsigned int _bm_                        : 16;
    } reg;
}lpctrl_gpu_gpu_top_gpc_req_poll_msk_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_top_intr_clr_pu_gpc :  1;
        unsigned int gpu_top_intr_clr_pd_gpc :  1;
        unsigned int reserved_0              : 14;
        unsigned int _bm_                    : 16;
    } reg;
}lpctrl_gpu_intr_clr_pu_gpu_top_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_top_intr_pu_gpc  :  1;
        unsigned int gpu_top_intr_pd_gpc  :  1;
        unsigned int gpu_top_pwr_on_done  :  1;
        unsigned int gpu_top_pwr_off_done :  1;
        unsigned int reserved_0           : 28;
    } reg;
}lpctrl_gpu_intr_pu_gpu_top_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_top_gpc_stat : 32;
    } reg;
}lpctrl_gpu_gpu_top_gpc_stat_0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_top_gpc_stat1 : 32;
    } reg;
}lpctrl_gpu_gpu_top_gpc_stat_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_mtcoms_en    :  1;
        unsigned int sc_gpu_iso_en       :  1;
        unsigned int sc_gpu_mrb_req      :  1;
        unsigned int sc_gpu_top_mem_sd   :  1;
        unsigned int sc_gpu_top_mem_poff :  2;
        unsigned int sc_gpu_bus_idle_req :  1;
        unsigned int sc_gpu_top_mem_ds   :  1;
        unsigned int reserved_0          :  8;
        unsigned int _bm_                : 16;
    } reg;
}lpctrl_gpu_gpu_gpc_soft_ctrl_lp_sig_16_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_mem_sd_req   :  1;
        unsigned int gpu_mem_poff_req :  2;
        unsigned int reserved_0       : 13;
        unsigned int _bm_             : 16;
    } reg;
}lpctrl_gpu_gpc_gpu_mem_ctrl_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_top_mem_ds_ack :  1;
        unsigned int gpu_top_mem_sd_ack :  1;
        unsigned int gpu_mtcmos_ack     :  1;
        unsigned int reserved_0         : 29;
    } reg;
}lpctrl_gpu_gpu_top_lp_status_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core_poff_delay : 16;
        unsigned int gpu_core_sd_delay   : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_delay_config0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core_ds_delay  : 16;
        unsigned int gpu_core_iso_delay : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_delay_config1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core_mem_repair_delay : 16;
        unsigned int gpu_core_rtu_iso_delay    : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_delay_config2_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_core_mtcmos_en : 16;
        unsigned int _bm_                  : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_core0_7_mem_poff : 16;
        unsigned int _bm_                    : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_core8_15_mem_poff : 16;
        unsigned int _bm_                     : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_2_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_core_mem_sd : 16;
        unsigned int _bm_               : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_3_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_core_mem_ds : 16;
        unsigned int _bm_               : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_4_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_core_iso_en : 16;
        unsigned int _bm_               : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_5_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_core_mode_sel : 16;
        unsigned int _bm_                 : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_6_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_core_rtu_mtcmos_en : 16;
        unsigned int _bm_                      : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_7_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_core_rtu_iso_en : 16;
        unsigned int _bm_                   : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_8_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_core_rtu_mode_sel : 16;
        unsigned int _bm_                     : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_9_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core_mem_sd_ack : 16;
        unsigned int gpu_core_mem_ds_ack : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_read_0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core_mtcmos_ack : 16;
        unsigned int reserved_0          : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_read_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core_rtu_mtcmos_ack : 16;
        unsigned int reserved_0              : 16;
    } reg;
}lpctrl_gpu_gpu_lp_ctrl_sig_read_3_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core_mem_repair_timeout : 16;
        unsigned int gpu_core_sd_ack_timeout     : 16;
    } reg;
}lpctrl_gpu_gpu_core_timerout_sig_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core_timeout_clr : 16;
        unsigned int _bm_                 : 16;
    } reg;
}lpctrl_gpu_gpu_core_clr_timerout_sig_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int sc_gpu_bus_mem_ds   :  1;
        unsigned int sc_gpu_bus_mem_sd   :  1;
        unsigned int sc_gpu_bus_mem_poff :  2;
        unsigned int reserved_0          : 12;
        unsigned int _bm_                : 16;
    } reg;
}lpctrl_gpu_mem_lpctrl_group0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_bus_mem_ds_ack :  1;
        unsigned int gpu_bus_mem_sd_ack :  1;
        unsigned int reserved_0         : 30;
    } reg;
}lpctrl_gpu_mem_lpstatus0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core0_lpctrl_status : 10;
        unsigned int gpu_core1_lpctrl_status : 10;
        unsigned int gpu_core2_lpctrl_status : 10;
        unsigned int reserved_0              :  2;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core3_lpctrl_status : 10;
        unsigned int gpu_core4_lpctrl_status : 10;
        unsigned int gpu_core5_lpctrl_status : 10;
        unsigned int reserved_0              :  2;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core6_lpctrl_status : 10;
        unsigned int gpu_core7_lpctrl_status : 10;
        unsigned int gpu_core8_lpctrl_status : 10;
        unsigned int reserved_0              :  2;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_2_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core9_lpctrl_status  : 10;
        unsigned int gpu_core10_lpctrl_status : 10;
        unsigned int gpu_core11_lpctrl_status : 10;
        unsigned int reserved_0               :  2;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_3_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core12_lpctrl_status : 10;
        unsigned int gpu_core13_lpctrl_status : 10;
        unsigned int gpu_core14_lpctrl_status : 10;
        unsigned int reserved_0               :  2;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_4_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_core15_lpctrl_status : 10;
        unsigned int gpu_rtu0_lpctrl_status   : 10;
        unsigned int gpu_rtu1_lpctrl_status   : 10;
        unsigned int reserved_0               :  2;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_5_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_rtu2_lpctrl_status : 10;
        unsigned int gpu_rtu3_lpctrl_status : 10;
        unsigned int gpu_rtu4_lpctrl_status : 10;
        unsigned int reserved_0             :  2;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_6_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_rtu5_lpctrl_status : 10;
        unsigned int gpu_rtu6_lpctrl_status : 10;
        unsigned int gpu_rtu7_lpctrl_status : 10;
        unsigned int reserved_0             :  2;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_7_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_rtu8_lpctrl_status  : 10;
        unsigned int gpu_rtu9_lpctrl_status  : 10;
        unsigned int gpu_rtu10_lpctrl_status : 10;
        unsigned int reserved_0              :  2;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_8_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_rtu11_lpctrl_status : 10;
        unsigned int gpu_rtu12_lpctrl_status : 10;
        unsigned int gpu_rtu13_lpctrl_status : 10;
        unsigned int reserved_0              :  2;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_9_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_rtu14_lpctrl_status : 10;
        unsigned int gpu_rtu15_lpctrl_status : 10;
        unsigned int reserved_0              : 12;
    } reg;
}lpctrl_gpu_gpu_core_rtu_lpctrl_status_10_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_gpc_en :  1;
        unsigned int reserved_0 : 15;
        unsigned int _bm_       : 16;
    } reg;
}lpctrl_gpu_gpu_top_gpc_en_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_top_gpc_ctrl0 : 32;
    } reg;
}lpctrl_gpu_gpu_top_gpc_ctrl_0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_top_gpc_ctrl1 : 32;
    } reg;
}lpctrl_gpu_gpu_top_gpc_ctrl_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_top_gpc_state_debug :  2;
        unsigned int reserved_0              : 30;
    } reg;
}lpctrl_gpu_gpu_gpc_stat_debug_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm0_clear      :  1;
        unsigned int gpu_hpm0_ro_sel     :  6;
        unsigned int gpu_hpm0_ro_clk_div :  2;
        unsigned int gpu_hpm0_ro_dbg_div :  6;
        unsigned int reserved_0          :  1;
        unsigned int _bm_                : 16;
    } reg;
}lpctrl_gpu_gpu_hpm0_cfg_0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm0_ro_en         :  1;
        unsigned int gpu_hpm0_test_hw_en    :  1;
        unsigned int gpu_hpm0_test_hw_start :  1;
        unsigned int gpu_hpm0_timer_cnt     :  8;
        unsigned int reserved_0             :  5;
        unsigned int _bm_                   : 16;
    } reg;
}lpctrl_gpu_gpu_hpm0_cfg_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm0_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm0_clk_gt_w1s_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm0_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm0_clk_gt_w1c_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm0_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm0_clk_gt_ro_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm0_data : 18;
        unsigned int reserved_0    : 14;
    } reg;
}lpctrl_gpu_gpu_hpm0_data_out_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm1_clear      :  1;
        unsigned int gpu_hpm1_ro_sel     :  6;
        unsigned int gpu_hpm1_ro_clk_div :  2;
        unsigned int gpu_hpm1_ro_dbg_div :  6;
        unsigned int reserved_0          :  1;
        unsigned int _bm_                : 16;
    } reg;
}lpctrl_gpu_gpu_hpm1_cfg_0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm1_ro_en         :  1;
        unsigned int gpu_hpm1_test_hw_en    :  1;
        unsigned int gpu_hpm1_test_hw_start :  1;
        unsigned int gpu_hpm1_timer_cnt     :  8;
        unsigned int reserved_0             :  5;
        unsigned int _bm_                   : 16;
    } reg;
}lpctrl_gpu_gpu_hpm1_cfg_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm1_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm1_clk_gt_w1s_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm1_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm1_clk_gt_w1c_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm1_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm1_clk_gt_ro_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm1_data : 18;
        unsigned int reserved_0    : 14;
    } reg;
}lpctrl_gpu_gpu_hpm1_data_out_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm2_clear      :  1;
        unsigned int gpu_hpm2_ro_sel     :  6;
        unsigned int gpu_hpm2_ro_clk_div :  2;
        unsigned int gpu_hpm2_ro_dbg_div :  6;
        unsigned int reserved_0          :  1;
        unsigned int _bm_                : 16;
    } reg;
}lpctrl_gpu_gpu_hpm2_cfg_0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm2_ro_en         :  1;
        unsigned int gpu_hpm2_test_hw_en    :  1;
        unsigned int gpu_hpm2_test_hw_start :  1;
        unsigned int gpu_hpm2_timer_cnt     :  8;
        unsigned int reserved_0             :  5;
        unsigned int _bm_                   : 16;
    } reg;
}lpctrl_gpu_gpu_hpm2_cfg_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm2_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm2_clk_gt_w1s_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm2_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm2_clk_gt_w1c_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm2_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm2_clk_gt_ro_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm2_data : 18;
        unsigned int reserved_0    : 14;
    } reg;
}lpctrl_gpu_gpu_hpm2_data_out_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm3_clear      :  1;
        unsigned int gpu_hpm3_ro_sel     :  6;
        unsigned int gpu_hpm3_ro_clk_div :  2;
        unsigned int gpu_hpm3_ro_dbg_div :  6;
        unsigned int reserved_0          :  1;
        unsigned int _bm_                : 16;
    } reg;
}lpctrl_gpu_gpu_hpm3_cfg_0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm3_ro_en         :  1;
        unsigned int gpu_hpm3_test_hw_en    :  1;
        unsigned int gpu_hpm3_test_hw_start :  1;
        unsigned int gpu_hpm3_timer_cnt     :  8;
        unsigned int reserved_0             :  5;
        unsigned int _bm_                   : 16;
    } reg;
}lpctrl_gpu_gpu_hpm3_cfg_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm3_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm3_clk_gt_w1s_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm3_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm3_clk_gt_w1c_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm3_clk_gt :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lpctrl_gpu_gpu_hpm3_clk_gt_ro_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_hpm3_data : 18;
        unsigned int reserved_0    : 14;
    } reg;
}lpctrl_gpu_gpu_hpm3_data_out_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_procm_clear      :  1;
        unsigned int gpu_procm_ro_sel     :  7;
        unsigned int gpu_procm_timer_sel  :  2;
        unsigned int gpu_procm_ro_clk_div :  1;
        unsigned int reserved_0           :  5;
        unsigned int _bm_                 : 16;
    } reg;
}lpctrl_gpu_gpu_procm_cfg_0_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_procm_ro_en         :  1;
        unsigned int gpu_procm_test_hw_en    :  1;
        unsigned int gpu_procm_test_hw_start :  1;
        unsigned int gpu_procm_ro_dbg_div    :  6;
        unsigned int gpu_procm_sel_h         :  1;
        unsigned int reserved_0              :  6;
        unsigned int _bm_                    : 16;
    } reg;
}lpctrl_gpu_gpu_procm_cfg_1_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_procm_clk_gt :  1;
        unsigned int reserved_0       : 31;
    } reg;
}lpctrl_gpu_gpu_procm_clk_gt_w1s_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_procm_clk_gt :  1;
        unsigned int reserved_0       : 31;
    } reg;
}lpctrl_gpu_gpu_procm_clk_gt_w1c_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_procm_clk_gt :  1;
        unsigned int reserved_0       : 31;
    } reg;
}lpctrl_gpu_gpu_procm_clk_gt_ro_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int gpu_procm_data : 25;
        unsigned int reserved_0     :  7;
    } reg;
}lpctrl_gpu_gpu_procm_data_out_t;
typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}lpctrl_gpu_regfile_cgbypass_t;
#endif
