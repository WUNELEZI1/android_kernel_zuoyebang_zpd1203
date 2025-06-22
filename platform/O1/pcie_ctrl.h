// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef PCIE_CTRL_REGIF_H
#define PCIE_CTRL_REGIF_H

#define PCIE_CTRL_CPL_TIMEOUT_INFO                     0x0000
#define PCIE_CTRL_CPL_TIMEOUT_INFO_RADM_TIMEOUT_CPL_TAG_SHIFT                            0
#define PCIE_CTRL_CPL_TIMEOUT_INFO_RADM_TIMEOUT_CPL_TAG_MASK                             0x000003ff
#define PCIE_CTRL_CPL_TIMEOUT_INFO_RADM_TIMEOUT_CPL_LEN_SHIFT                            10
#define PCIE_CTRL_CPL_TIMEOUT_INFO_RADM_TIMEOUT_CPL_LEN_MASK                             0x003ffc00
#define PCIE_CTRL_CPL_TIMEOUT_INFO_RADM_TIMEOUT_CPL_ATTR_SHIFT                           22
#define PCIE_CTRL_CPL_TIMEOUT_INFO_RADM_TIMEOUT_CPL_ATTR_MASK                            0x00c00000
#define PCIE_CTRL_CPL_TIMEOUT_INFO_RADM_TIMEOUT_CPL_TC_SHIFT                             24
#define PCIE_CTRL_CPL_TIMEOUT_INFO_RADM_TIMEOUT_CPL_TC_MASK                              0x07000000
#define PCIE_CTRL_CPL_TIMEOUT_INFO_RADM_TIMEOUT_FUNC_NUM_SHIFT                           27
#define PCIE_CTRL_CPL_TIMEOUT_INFO_RADM_TIMEOUT_FUNC_NUM_MASK                            0x38000000
#define PCIE_CTRL_VMI_SIGNALS_0                        0x0004
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_TYPE_SHIFT                                       0
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_TYPE_MASK                                        0x0000001f
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_FMT_SHIFT                                        5
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_FMT_MASK                                         0x00000060
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_TC_SHIFT                                         7
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_TC_MASK                                          0x00000380
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_ATTR_SHIFT                                       10
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_ATTR_MASK                                        0x00000c00
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_EP_SHIFT                                         12
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_EP_MASK                                          0x00001000
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_TD_SHIFT                                         13
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_TD_MASK                                          0x00002000
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_LEN_SHIFT                                        14
#define PCIE_CTRL_VMI_SIGNALS_0_VEN_MSG_LEN_MASK                                         0x00ffc000
#define PCIE_CTRL_VMI_SIGNALS_1                        0x0008
#define PCIE_CTRL_VMI_SIGNALS_1_VEN_MSG_CODE_SHIFT                                       0
#define PCIE_CTRL_VMI_SIGNALS_1_VEN_MSG_CODE_MASK                                        0x000000ff
#define PCIE_CTRL_VMI_SIGNALS_1_VEN_MSG_TAG_SHIFT                                        8
#define PCIE_CTRL_VMI_SIGNALS_1_VEN_MSG_TAG_MASK                                         0x0003ff00
#define PCIE_CTRL_VMI_SIGNALS_1_VEN_MSG_FUNC_NUM_SHIFT                                   18
#define PCIE_CTRL_VMI_SIGNALS_1_VEN_MSG_FUNC_NUM_MASK                                    0x001c0000
#define PCIE_CTRL_VMI_SIGNALS_2                        0x000c
#define PCIE_CTRL_VMI_SIGNALS_3                        0x0010
#define PCIE_CTRL_CFG_VEN_MSG_REQ                      0x0014
#define PCIE_CTRL_CFG_VEN_MSG_REQ_CFG_VEN_MSG_REQ_SHIFT                                  0
#define PCIE_CTRL_SII_GENERAL_CORE_CTRL                0x0018
#define PCIE_CTRL_SII_GENERAL_CORE_CTRL_APP_LTSSM_ENABLE_SHIFT                           0
#define PCIE_CTRL_SII_GENERAL_CORE_CTRL_APP_LTSSM_ENABLE_MASK                            0x00000001
#define PCIE_CTRL_SII_GENERAL_CORE_CTRL_APP_SRIS_MODE_SHIFT                              1
#define PCIE_CTRL_SII_GENERAL_CORE_CTRL_APP_SRIS_MODE_MASK                               0x00000002
#define PCIE_CTRL_SII_GENERAL_CORE_CTRL_TX_LANE_FLIP_EN_SHIFT                            2
#define PCIE_CTRL_SII_GENERAL_CORE_CTRL_TX_LANE_FLIP_EN_MASK                             0x00000004
#define PCIE_CTRL_SII_GENERAL_CORE_CTRL_RX_LANE_FLIP_EN_SHIFT                            3
#define PCIE_CTRL_SII_GENERAL_CORE_CTRL_RX_LANE_FLIP_EN_MASK                             0x00000008
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0               0x001c
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_L1SUB_DISABLE_SHIFT                         0
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_L1SUB_DISABLE_MASK                          0x00000001
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_CLK_PM_EN_SHIFT                             1
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_CLK_PM_EN_MASK                              0x00000002
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_XFER_PENDING_SHIFT                          2
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_XFER_PENDING_MASK                           0x00000004
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_REQ_EXIT_L1_SHIFT                           3
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_READY_ENTR_L23_SHIFT                        4
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_READY_ENTR_L23_MASK                         0x00000010
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_REQ_ENTR_L1_SHIFT                           5
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APPS_PM_XMT_PME_SHIFT                           6
#define PCIE_CTRL_SII_POWER_MANAGEMENT_0_APPS_PM_XMT_PME_MASK                            0x000003c0
#define PCIE_CTRL_SII_POWER_MANAGEMENT_1               0x0020
#define PCIE_CTRL_SII_POWER_MANAGEMENT_1_PM_DSTATE_SHIFT                                 0
#define PCIE_CTRL_SII_POWER_MANAGEMENT_1_PM_DSTATE_MASK                                  0x00000fff
#define PCIE_CTRL_SII_POWER_MANAGEMENT_1_PM_PME_EN_SHIFT                                 12
#define PCIE_CTRL_SII_POWER_MANAGEMENT_1_PM_PME_EN_MASK                                  0x0000f000
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2               0x0024
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_L1_ENTRY_STARTED_SHIFT                       0
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_L1_ENTRY_STARTED_MASK                        0x00000001
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_L1SUB_STATE_SHIFT                            1
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_L1SUB_STATE_MASK                             0x0000000e
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_SLAVE_STATE_SHIFT                            4
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_SLAVE_STATE_MASK                             0x000001f0
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_MASTER_STATE_SHIFT                           9
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_MASTER_STATE_MASK                            0x00003e00
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_L2_EXIT_SHIFT                         14
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_L2_EXIT_MASK                          0x00004000
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_IN_L2_SHIFT                           15
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_IN_L2_MASK                            0x00008000
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_IN_L1SUB_SHIFT                        16
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_IN_L1SUB_MASK                         0x00010000
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_IN_L1_SHIFT                           17
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_IN_L1_MASK                            0x00020000
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_IN_L0S_SHIFT                          18
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_IN_L0S_MASK                           0x00040000
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_CURNT_STATE_SHIFT                            19
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_CURNT_STATE_MASK                             0x00380000
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_STATUS_SHIFT                                 22
#define PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_STATUS_MASK                                  0x03c00000
#define PCIE_CTRL_SII_POWER_MANAGEMENT_3               0x0028
#define PCIE_CTRL_SII_POWER_MANAGEMENT_3_CFG_L1SUB_EN_SHIFT                              0
#define PCIE_CTRL_SII_POWER_MANAGEMENT_3_CFG_L1SUB_EN_MASK                               0x00000001
#define PCIE_CTRL_SII_POWER_MANAGEMENT_3_PM_ASPM_L1_ENTER_READY_SHIFT                    1
#define PCIE_CTRL_SII_POWER_MANAGEMENT_3_PM_ASPM_L1_ENTER_READY_MASK                     0x00000002
#define PCIE_CTRL_SII_POWER_MANAGEMENT_3_AUX_PM_EN_SHIFT                                 2
#define PCIE_CTRL_SII_POWER_MANAGEMENT_3_AUX_PM_EN_MASK                                  0x0000003c
#define PCIE_CTRL_SII_POWER_MANAGEMENT_3_SMLH_LTSSM_STATE_RCVRY_EQ_SHIFT                 6
#define PCIE_CTRL_SII_POWER_MANAGEMENT_3_SMLH_LTSSM_STATE_RCVRY_EQ_MASK                  0x00000040
#define PCIE_CTRL_SII_TRANSMIT_CTRL_0                  0x002c
#define PCIE_CTRL_SII_TRANSMIT_CTRL_0_PM_XTLH_BLOCK_TLP_SHIFT                            0
#define PCIE_CTRL_SII_TRANSMIT_CTRL_0_PM_XTLH_BLOCK_TLP_MASK                             0x00000001
#define PCIE_CTRL_SII_TRANSMIT_CTRL_1                  0x0030
#define PCIE_CTRL_SII_TRANSMIT_CTRL_2                  0x0034
#define PCIE_CTRL_SII_TRANSMIT_CTRL_3                  0x0038
#define PCIE_CTRL_SII_TRANSMIT_CTRL_4                  0x003c
#define PCIE_CTRL_SII_TRANSMIT_CTRL_5                  0x0040
#define PCIE_CTRL_SII_TRANSMIT_CTRL_5_APP_ERR_BUS_SHIFT                                  0
#define PCIE_CTRL_SII_TRANSMIT_CTRL_5_APP_ERR_BUS_MASK                                   0x07ffffff
#define PCIE_CTRL_SII_TRANSMIT_CTRL_6                  0x0044
#define PCIE_CTRL_SII_TRANSMIT_CTRL_6_APP_ERR_FUNC_NUM_SHIFT                             0
#define PCIE_CTRL_SII_TRANSMIT_CTRL_6_APP_ERR_FUNC_NUM_MASK                              0x00000007
#define PCIE_CTRL_SII_TRANSMIT_CTRL_6_APP_POISONED_TLP_TYPE_SHIFT                        3
#define PCIE_CTRL_SII_TRANSMIT_CTRL_6_APP_POISONED_TLP_TYPE_MASK                         0x00000008
#define PCIE_CTRL_SII_TRANSMIT_CTRL_6_APP_ERR_ADVISORY_SHIFT                             4
#define PCIE_CTRL_SII_TRANSMIT_CTRL_6_APP_ERR_ADVISORY_MASK                              0x00000010
#define PCIE_CTRL_SII_TRANSMIT_CTRL_7                  0x0048
#define PCIE_CTRL_SII_TRANSMIT_CTRL_7_APP_HDR_VALID_SHIFT                                0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_0             0x004c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_1             0x0050
#define PCIE_CTRL_SII_CONFIGURATION_INFO_2             0x0054
#define PCIE_CTRL_SII_CONFIGURATION_INFO_3             0x0058
#define PCIE_CTRL_SII_CONFIGURATION_INFO_4             0x005c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_5             0x0060
#define PCIE_CTRL_SII_CONFIGURATION_INFO_6             0x0064
#define PCIE_CTRL_SII_CONFIGURATION_INFO_7             0x0068
#define PCIE_CTRL_SII_CONFIGURATION_INFO_8             0x006c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_9             0x0070
#define PCIE_CTRL_SII_CONFIGURATION_INFO_10            0x0074
#define PCIE_CTRL_SII_CONFIGURATION_INFO_11            0x0078
#define PCIE_CTRL_SII_CONFIGURATION_INFO_12            0x007c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_13            0x0080
#define PCIE_CTRL_SII_CONFIGURATION_INFO_14            0x0084
#define PCIE_CTRL_SII_CONFIGURATION_INFO_15            0x0088
#define PCIE_CTRL_SII_CONFIGURATION_INFO_16            0x008c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_17            0x0090
#define PCIE_CTRL_SII_CONFIGURATION_INFO_18            0x0094
#define PCIE_CTRL_SII_CONFIGURATION_INFO_19            0x0098
#define PCIE_CTRL_SII_CONFIGURATION_INFO_20            0x009c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_21            0x00a0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_22            0x00a4
#define PCIE_CTRL_SII_CONFIGURATION_INFO_23            0x00a8
#define PCIE_CTRL_SII_CONFIGURATION_INFO_24            0x00ac
#define PCIE_CTRL_SII_CONFIGURATION_INFO_25            0x00b0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_26            0x00b4
#define PCIE_CTRL_SII_CONFIGURATION_INFO_27            0x00b8
#define PCIE_CTRL_SII_CONFIGURATION_INFO_28            0x00bc
#define PCIE_CTRL_SII_CONFIGURATION_INFO_29            0x00c0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_30            0x00c4
#define PCIE_CTRL_SII_CONFIGURATION_INFO_31            0x00c8
#define PCIE_CTRL_SII_CONFIGURATION_INFO_32            0x00cc
#define PCIE_CTRL_SII_CONFIGURATION_INFO_33            0x00d0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_34            0x00d4
#define PCIE_CTRL_SII_CONFIGURATION_INFO_35            0x00d8
#define PCIE_CTRL_SII_CONFIGURATION_INFO_36            0x00dc
#define PCIE_CTRL_SII_CONFIGURATION_INFO_37            0x00e0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_38            0x00e4
#define PCIE_CTRL_SII_CONFIGURATION_INFO_39            0x00e8
#define PCIE_CTRL_SII_CONFIGURATION_INFO_40            0x00ec
#define PCIE_CTRL_SII_CONFIGURATION_INFO_41            0x00f0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_42            0x00f4
#define PCIE_CTRL_SII_CONFIGURATION_INFO_43            0x00f8
#define PCIE_CTRL_SII_CONFIGURATION_INFO_44            0x00fc
#define PCIE_CTRL_SII_CONFIGURATION_INFO_45            0x0100
#define PCIE_CTRL_SII_CONFIGURATION_INFO_46            0x0104
#define PCIE_CTRL_SII_CONFIGURATION_INFO_47            0x0108
#define PCIE_CTRL_SII_CONFIGURATION_INFO_48            0x010c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_49            0x0110
#define PCIE_CTRL_SII_CONFIGURATION_INFO_50            0x0114
#define PCIE_CTRL_SII_CONFIGURATION_INFO_51            0x0118
#define PCIE_CTRL_SII_CONFIGURATION_INFO_52            0x011c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_53            0x0120
#define PCIE_CTRL_SII_CONFIGURATION_INFO_54            0x0124
#define PCIE_CTRL_SII_CONFIGURATION_INFO_55            0x0128
#define PCIE_CTRL_SII_CONFIGURATION_INFO_56            0x012c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_57            0x0130
#define PCIE_CTRL_SII_CONFIGURATION_INFO_58            0x0134
#define PCIE_CTRL_SII_CONFIGURATION_INFO_59            0x0138
#define PCIE_CTRL_SII_CONFIGURATION_INFO_60            0x013c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_61            0x0140
#define PCIE_CTRL_SII_CONFIGURATION_INFO_62            0x0144
#define PCIE_CTRL_SII_CONFIGURATION_INFO_63            0x0148
#define PCIE_CTRL_SII_CONFIGURATION_INFO_64            0x014c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_65            0x0150
#define PCIE_CTRL_SII_CONFIGURATION_INFO_66            0x0154
#define PCIE_CTRL_SII_CONFIGURATION_INFO_67            0x0158
#define PCIE_CTRL_SII_CONFIGURATION_INFO_68            0x015c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_69            0x0160
#define PCIE_CTRL_SII_CONFIGURATION_INFO_70            0x0164
#define PCIE_CTRL_SII_CONFIGURATION_INFO_71            0x0168
#define PCIE_CTRL_SII_CONFIGURATION_INFO_72            0x016c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_73            0x0170
#define PCIE_CTRL_SII_CONFIGURATION_INFO_74            0x0174
#define PCIE_CTRL_SII_CONFIGURATION_INFO_75            0x0178
#define PCIE_CTRL_SII_CONFIGURATION_INFO_76            0x017c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_77            0x0180
#define PCIE_CTRL_SII_CONFIGURATION_INFO_78            0x0184
#define PCIE_CTRL_SII_CONFIGURATION_INFO_79            0x0188
#define PCIE_CTRL_SII_CONFIGURATION_INFO_80            0x018c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_81            0x0190
#define PCIE_CTRL_SII_CONFIGURATION_INFO_82            0x0194
#define PCIE_CTRL_SII_CONFIGURATION_INFO_83            0x0198
#define PCIE_CTRL_SII_CONFIGURATION_INFO_84            0x019c
#define PCIE_CTRL_SII_CONFIGURATION_INFO_84_CFG_MAX_RD_REQ_SIZE_SHIFT                    0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_84_CFG_MAX_RD_REQ_SIZE_MASK                     0x00000fff
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85            0x01a0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85_CFG_PBUS_NUM_SHIFT                           0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85_CFG_PBUS_NUM_MASK                            0x000000ff
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85_CFG_PM_NO_SOFT_RST_SHIFT                     8
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85_CFG_PM_NO_SOFT_RST_MASK                      0x00000f00
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85_CFG_EXT_TAG_EN_SHIFT                         12
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85_CFG_EXT_TAG_EN_MASK                          0x0000f000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85_CFG_MAX_PAYLOAD_SIZE_SHIFT                   16
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85_CFG_MAX_PAYLOAD_SIZE_MASK                    0x0fff0000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85_CFG_MEM_SPACE_EN_SHIFT                       28
#define PCIE_CTRL_SII_CONFIGURATION_INFO_85_CFG_MEM_SPACE_EN_MASK                        0xf0000000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86            0x01a4
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_HP_INT_EN_SHIFT                          0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_HP_INT_EN_MASK                           0x0000000f
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_CMD_CPLED_INT_EN_SHIFT                   4
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_CMD_CPLED_INT_EN_MASK                    0x000000f0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_DLL_STATE_CHGED_EN_SHIFT                 8
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_DLL_STATE_CHGED_EN_MASK                  0x00000f00
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_HP_SLOT_CTRL_ACCESS_SHIFT                12
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_HP_SLOT_CTRL_ACCESS_MASK                 0x0000f000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_RELAX_ORDER_EN_SHIFT                     16
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_RELAX_ORDER_EN_MASK                      0x000f0000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_NO_SNOOP_EN_SHIFT                        20
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_NO_SNOOP_EN_MASK                         0x00f00000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_PBUS_DEV_NUM_SHIFT                       24
#define PCIE_CTRL_SII_CONFIGURATION_INFO_86_CFG_PBUS_DEV_NUM_MASK                        0x1f000000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87            0x01a8
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_NEG_LINK_WIDTH_SHIFT                     0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_NEG_LINK_WIDTH_MASK                      0x0000003f
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_ATTEN_BUTTON_PRESSED_EN_SHIFT            6
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_ATTEN_BUTTON_PRESSED_EN_MASK             0x000003c0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_PWR_FAULT_DET_EN_SHIFT                   10
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_PWR_FAULT_DET_EN_MASK                    0x00003c00
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_MRL_SENSOR_CHGED_EN_SHIFT                14
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_MRL_SENSOR_CHGED_EN_MASK                 0x0003c000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_RCB_SHIFT                                18
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_RCB_MASK                                 0x003c0000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_PRE_DET_CHGED_EN_SHIFT                   22
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_PRE_DET_CHGED_EN_MASK                    0x03c00000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_BUS_MASTER_EN_SHIFT                      26
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_BUS_MASTER_EN_MASK                       0x3c000000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_DISABLE_LTR_CLR_MSG_SHIFT                30
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_DISABLE_LTR_CLR_MSG_MASK                 0x40000000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_LTR_M_EN_SHIFT                           31
#define PCIE_CTRL_SII_CONFIGURATION_INFO_87_CFG_LTR_M_EN_MASK                            0x80000000
#define PCIE_CTRL_SII_CONFIGURATION_INFO_88            0x01ac
#define PCIE_CTRL_SII_CONFIGURATION_INFO_88_APP_DBI_RO_WR_DISABLE_SHIFT                  0
#define PCIE_CTRL_SII_CONFIGURATION_INFO_88_APP_DBI_RO_WR_DISABLE_MASK                   0x00000001
#define PCIE_CTRL_SII_CONFIGURATION_INFO_88_APP_PF_REQ_RETRY_EN_SHIFT                    1
#define PCIE_CTRL_SII_CONFIGURATION_INFO_88_APP_PF_REQ_RETRY_EN_MASK                     0x0000001e
#define PCIE_CTRL_SII_CONFIGURATION_INFO_88_APP_REQ_RETRY_EN_SHIFT                       5
#define PCIE_CTRL_SII_CONFIGURATION_INFO_88_APP_REQ_RETRY_EN_MASK                        0x00000020
#define PCIE_CTRL_SII_DEBUG_INFO_0                     0x01b0
#define PCIE_CTRL_SII_DEBUG_INFO_0_RADM_Q_NOT_EMPTY_SHIFT                                0
#define PCIE_CTRL_SII_DEBUG_INFO_0_RADM_Q_NOT_EMPTY_MASK                                 0x00000001
#define PCIE_CTRL_SII_DEBUG_INFO_0_RADM_XFER_PENDING_SHIFT                               1
#define PCIE_CTRL_SII_DEBUG_INFO_0_RADM_XFER_PENDING_MASK                                0x00000002
#define PCIE_CTRL_SII_DEBUG_INFO_0_EDMA_XFER_PENDING_SHIFT                               2
#define PCIE_CTRL_SII_DEBUG_INFO_0_EDMA_XFER_PENDING_MASK                                0x00000004
#define PCIE_CTRL_SII_DEBUG_INFO_0_BRDG_SLV_XFER_PENDING_SHIFT                           3
#define PCIE_CTRL_SII_DEBUG_INFO_0_BRDG_SLV_XFER_PENDING_MASK                            0x00000008
#define PCIE_CTRL_SII_DEBUG_INFO_0_BRDG_DBI_XFER_PENDING_SHIFT                           4
#define PCIE_CTRL_SII_DEBUG_INFO_0_BRDG_DBI_XFER_PENDING_MASK                            0x00000010
#define PCIE_CTRL_SII_DEBUG_INFO_0_RDLH_LINK_UP_SHIFT                                    5
#define PCIE_CTRL_SII_DEBUG_INFO_0_RDLH_LINK_UP_MASK                                     0x00000020
#define PCIE_CTRL_SII_LINK_RST_OR_STATU                0x01b4
#define PCIE_CTRL_SII_LINK_RST_OR_STATU_SMLH_LINK_UP_SHIFT                               0
#define PCIE_CTRL_SII_LINK_RST_OR_STATU_SMLH_LINK_UP_MASK                                0x00000001
#define PCIE_CTRL_PHY_STATUS                           0x01b8
#define PCIE_CTRL_PHY_STATUS_PHY_MAC_PHYSTATUS_SHIFT                                     0
#define PCIE_CTRL_PHY_STATUS_PHY_MAC_PHYSTATUS_MASK                                      0x00000001
#define PCIE_CTRL_SII_MESSAGE_0                        0x01bc
#define PCIE_CTRL_SII_MESSAGE_1                        0x01c0
#define PCIE_CTRL_SII_MESSAGE_2                        0x01c4
#define PCIE_CTRL_SII_MESSAGE_3                        0x01c8
#define PCIE_CTRL_SII_MESSAGE_3_RADM_MSG_REQ_ID_SHIFT                                    0
#define PCIE_CTRL_SII_MESSAGE_3_RADM_MSG_REQ_ID_MASK                                     0x0000ffff
#define PCIE_CTRL_SII_LTR_MESSAGE_0                    0x01cc
#define PCIE_CTRL_SII_LTR_MESSAGE_1                    0x01d0
#define PCIE_CTRL_SII_LTR_MESSAGE_1_CFG_LTR_REQ_SHIFT                                    0
#define PCIE_CTRL_SII_LTR_MESSAGE_1_APP_LTR_MSG_FUNC_NUM_SHIFT                           1
#define PCIE_CTRL_SII_LTR_MESSAGE_1_APP_LTR_MSG_FUNC_NUM_MASK                            0x0000000e
#define PCIE_CTRL_SII_LTR_MESSAGE_2                    0x01d4
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0              0x01d8
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0_TRGT_TIMEOUT_LOOKUP_ID_SHIFT                   0
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0_TRGT_TIMEOUT_LOOKUP_ID_MASK                    0x000003ff
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0_TRGT_TIMEOUT_CPL_LEN_SHIFT                     10
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0_TRGT_TIMEOUT_CPL_LEN_MASK                      0x003ffc00
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0_TRGT_TIMEOUT_CPL_ATTR_SHIFT                    22
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0_TRGT_TIMEOUT_CPL_ATTR_MASK                     0x00c00000
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0_TRGT_TIMEOUT_CPL_TC_SHIFT                      24
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0_TRGT_TIMEOUT_CPL_TC_MASK                       0x07000000
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0_TRGT_TIMEOUT_CPL_FUNC_NUM_SHIFT                27
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_0_TRGT_TIMEOUT_CPL_FUNC_NUM_MASK                 0x38000000
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_1              0x01dc
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_1_TRGT_LOOKUP_EMPTY_SHIFT                        0
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_1_TRGT_LOOKUP_EMPTY_MASK                         0x00000001
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_1_TRGT_LOOKUP_ID_SHIFT                           1
#define PCIE_CTRL_TRAGET_COMPLETION_LUT_1_TRGT_LOOKUP_ID_MASK                            0x000007fe
#define PCIE_CTRL_GEN4_MARGIN                          0x01e0
#define PCIE_CTRL_GEN4_MARGIN_APP_MARGINING_SOFTWARE_READY_SHIFT                         0
#define PCIE_CTRL_GEN4_MARGIN_APP_MARGINING_SOFTWARE_READY_MASK                          0x00000001
#define PCIE_CTRL_GEN4_MARGIN_APP_MARGINING_READY_SHIFT                                  1
#define PCIE_CTRL_GEN4_MARGIN_APP_MARGINING_READY_MASK                                   0x00000002
#define PCIE_CTRL_HOT_RESET                            0x01e4
#define PCIE_CTRL_HOT_RESET_APP_INIT_RST_SHIFT                                           0
#define PCIE_CTRL_HOT_RESET_APP_INIT_RST_MASK                                            0x00000001
#define PCIE_CTRL_APP_CLK_REQ_N                        0x01e8
#define PCIE_CTRL_APP_CLK_REQ_N_APP_CLK_REQ_N_SHIFT                                      0
#define PCIE_CTRL_APP_CLK_REQ_N_APP_CLK_REQ_N_MASK                                       0x00000001
#define PCIE_CTRL_EXTSRIVO_PF_REG_VALUE                0x01ec
#define PCIE_CTRL_EXTSRIVO_PF_REG_VALUE_CFG_F_ERR_RPT_EN_SHIFT                           0
#define PCIE_CTRL_EXTSRIVO_PF_REG_VALUE_CFG_F_ERR_RPT_EN_MASK                            0x0000000f
#define PCIE_CTRL_EXTSRIVO_PF_REG_VALUE_CFG_NF_ERR_RPT_EN_SHIFT                          4
#define PCIE_CTRL_EXTSRIVO_PF_REG_VALUE_CFG_NF_ERR_RPT_EN_MASK                           0x000000f0
#define PCIE_CTRL_EXTSRIVO_PF_REG_VALUE_CFG_COR_ERR_RPT_EN_SHIFT                         8
#define PCIE_CTRL_EXTSRIVO_PF_REG_VALUE_CFG_COR_ERR_RPT_EN_MASK                          0x00000f00
#define PCIE_CTRL_EXTSRIVO_PF_REG_VALUE_CFG_REG_SERREN_SHIFT                             12
#define PCIE_CTRL_EXTSRIVO_PF_REG_VALUE_CFG_REG_SERREN_MASK                              0x0000f000
#define PCIE_CTRL_DIAGNOSTIC_SIGS                      0x01f0
#define PCIE_CTRL_DIAGNOSTIC_SIGS_DIAG_CTRL_BUS_SHIFT                                    0
#define PCIE_CTRL_DIAGNOSTIC_SIGS_DIAG_CTRL_BUS_MASK                                     0x00000007
#define PCIE_CTRL_RAS_DES_TBA                          0x01f4
#define PCIE_CTRL_RAS_DES_TBA_APP_RAS_DES_TBA_CTRL_SHIFT                                 0
#define PCIE_CTRL_RAS_DES_TBA_APP_RAS_DES_TBA_CTRL_MASK                                  0x00000001
#define PCIE_CTRL_RAS_DES_SILI_DBG                     0x01f8
#define PCIE_CTRL_RAS_DES_SILI_DBG_APP_RAS_DES_SD_HOLD_LTSSM_SHIFT                       0
#define PCIE_CTRL_RAS_DES_SILI_DBG_APP_RAS_DES_SD_HOLD_LTSSM_MASK                        0x00000001
#define PCIE_CTRL_SII_ROM_VALIDATION_0                 0x01fc
#define PCIE_CTRL_SII_ROM_VALIDATION_0_EXP_ROM_VALIDATION_DETAILS_STROBE_SHIFT           0
#define PCIE_CTRL_SII_ROM_VALIDATION_0_EXP_ROM_VALIDATION_DETAILS_STROBE_MASK            0x0000000f
#define PCIE_CTRL_SII_ROM_VALIDATION_0_EXP_ROM_VALIDATION_DETAILS_SHIFT                  4
#define PCIE_CTRL_SII_ROM_VALIDATION_0_EXP_ROM_VALIDATION_DETAILS_MASK                   0x000ffff0
#define PCIE_CTRL_SII_ROM_VALIDATION_1                 0x0200
#define PCIE_CTRL_SII_ROM_VALIDATION_1_EXP_ROM_VALIDATION_STATUS_STROBE_SHIFT            0
#define PCIE_CTRL_SII_ROM_VALIDATION_1_EXP_ROM_VALIDATION_STATUS_STROBE_MASK             0x0000000f
#define PCIE_CTRL_SII_ROM_VALIDATION_1_EXP_ROM_VALIDATION_STATUS_SHIFT                   4
#define PCIE_CTRL_SII_ROM_VALIDATION_1_EXP_ROM_VALIDATION_STATUS_MASK                    0x0000fff0
#define PCIE_CTRL_SII_INTERRUPT_0                      0x0204
#define PCIE_CTRL_SII_INTERRUPT_1                      0x0208
#define PCIE_CTRL_SII_INTERRUPT_1_CFG_INT_DISABLE_SHIFT                                  0
#define PCIE_CTRL_SII_INTERRUPT_1_CFG_INT_DISABLE_MASK                                   0x0000000f
#define PCIE_CTRL_PHY_REGISTER_ACCESS_CTRL             0x020c
#define PCIE_CTRL_PHY_REGISTER_ACCESS_CTRL_APP_HOLD_PHY_RST_SHIFT                        0
#define PCIE_CTRL_PHY_REGISTER_ACCESS_CTRL_APP_HOLD_PHY_RST_MASK                         0x00000001
#define PCIE_CTRL_PIPE_SIGS                            0x0210
#define PCIE_CTRL_PIPE_SIGS_PM_CURRENT_DATA_RATE_SHIFT                                   0
#define PCIE_CTRL_PIPE_SIGS_PM_CURRENT_DATA_RATE_MASK                                    0x00000007
#define PCIE_CTRL_PIPE_SIGS_MAC_PHY_RXELECIDLE_DISABLE_SHIFT                             3
#define PCIE_CTRL_PIPE_SIGS_MAC_PHY_RXELECIDLE_DISABLE_MASK                              0x00000008
#define PCIE_CTRL_PIPE_SIGS_MAC_PHY_TXCOMMONMODE_DISABLE_SHIFT                           4
#define PCIE_CTRL_PIPE_SIGS_MAC_PHY_TXCOMMONMODE_DISABLE_MASK                            0x00000010
#define PCIE_CTRL_SII_DETECT_POISONED_CFG              0x0214
#define PCIE_CTRL_SII_DETECT_POISONED_CFG_RADM_RCVD_CFGWR_POISONED_REG_NUM_SHIFT         0
#define PCIE_CTRL_SII_DETECT_POISONED_CFG_RADM_RCVD_CFGWR_POISONED_REG_NUM_MASK          0x0000003f
#define PCIE_CTRL_SII_DETECT_POISONED_CFG_RADM_RCVD_CFGWR_POISONED_EXT_REG_NUM_SHIFT     6
#define PCIE_CTRL_SII_DETECT_POISONED_CFG_RADM_RCVD_CFGWR_POISONED_EXT_REG_NUM_MASK      0x000003c0
#define PCIE_CTRL_SII_DETECT_POISONED_CFG_RADM_RCVD_CFGWR_POISONED_FUNC_NUM_SHIFT        10
#define PCIE_CTRL_SII_DETECT_POISONED_CFG_RADM_RCVD_CFGWR_POISONED_FUNC_NUM_MASK         0x00001c00
#define PCIE_CTRL_AXI_MISC_0                           0x0218
#define PCIE_CTRL_AXI_MISC_0_MSTR_ARMISC_INFO_DMA_SHIFT                                  0
#define PCIE_CTRL_AXI_MISC_0_MSTR_ARMISC_INFO_DMA_MASK                                   0x0000003f
#define PCIE_CTRL_AXI_MISC_0_MSTR_AWMISC_INFO_DMA_SHIFT                                  6
#define PCIE_CTRL_AXI_MISC_0_MSTR_AWMISC_INFO_DMA_MASK                                   0x00000fc0
#define PCIE_CTRL_AXI_MISC_1                           0x021c
#define PCIE_CTRL_AXI_MISC_2                           0x0220
#define PCIE_CTRL_AXI_MISC_2_MSTR_AWMISC_INFO_H_SHIFT                                    0
#define PCIE_CTRL_AXI_MISC_2_MSTR_AWMISC_INFO_H_MASK                                     0x0003ffff
#define PCIE_CTRL_AXI_MISC_3                           0x0224
#define PCIE_CTRL_AXI_MISC_4                           0x0228
#define PCIE_CTRL_AXI_MISC_5                           0x022c
#define PCIE_CTRL_AXI_MISC_6                           0x0230
#define PCIE_CTRL_AXI_MISC_6_MSTR_ARMISC_INFO_H_SHIFT                                    0
#define PCIE_CTRL_AXI_MISC_6_MSTR_ARMISC_INFO_H_MASK                                     0x0003ffff
#define PCIE_CTRL_AXI_MISC_7                           0x0234
#define PCIE_CTRL_AXI_MISC_7_MSTR_AWMISC_INFO_EP_SHIFT                                   0
#define PCIE_CTRL_AXI_MISC_7_MSTR_AWMISC_INFO_EP_MASK                                    0x00000001
#define PCIE_CTRL_AXI_MISC_7_MSTR_AWMISC_INFO_LAST_DCMP_TLP_SHIFT                        1
#define PCIE_CTRL_AXI_MISC_7_MSTR_AWMISC_INFO_LAST_DCMP_TLP_MASK                         0x00000002
#define PCIE_CTRL_AXI_MISC_7_MSTR_ARMISC_INFO_ZEROREAD_SHIFT                             2
#define PCIE_CTRL_AXI_MISC_7_MSTR_ARMISC_INFO_ZEROREAD_MASK                              0x00000004
#define PCIE_CTRL_AXI_MISC_7_MSTR_ARMISC_INFO_LAST_DCMP_TLP_SHIFT                        3
#define PCIE_CTRL_AXI_MISC_7_MSTR_ARMISC_INFO_LAST_DCMP_TLP_MASK                         0x00000008
#define PCIE_CTRL_AXI_MISC_8                           0x0238
#define PCIE_CTRL_AXI_MISC_8_SLV_RMISC_INFO_SHIFT                                        0
#define PCIE_CTRL_AXI_MISC_8_SLV_RMISC_INFO_MASK                                         0x00003fff
#define PCIE_CTRL_AXI_MISC_8_SLV_BMISC_INFO_SHIFT                                        14
#define PCIE_CTRL_AXI_MISC_8_SLV_BMISC_INFO_MASK                                         0x0fffc000
#define PCIE_CTRL_AXI_MISC_9                           0x023c
#define PCIE_CTRL_AXI_MISC_9_MSTR_RMISC_INFO_CPL_STAT_SHIFT                              0
#define PCIE_CTRL_AXI_MISC_9_MSTR_RMISC_INFO_CPL_STAT_MASK                               0x00000007
#define PCIE_CTRL_AXI_MISC_9_MSTR_BMISC_INFO_CPL_STAT_SHIFT                              3
#define PCIE_CTRL_AXI_MISC_9_MSTR_BMISC_INFO_CPL_STAT_MASK                               0x00000038
#define PCIE_CTRL_AXI_MISC_10                          0x0240
#define PCIE_CTRL_AXI_MISC_10_SLV_AWMISC_INFO_SHIFT                                      0
#define PCIE_CTRL_AXI_MISC_10_SLV_AWMISC_INFO_MASK                                       0x01ffffff
#define PCIE_CTRL_AXI_MISC_11                          0x0244
#define PCIE_CTRL_AXI_MISC_12                          0x0248
#define PCIE_CTRL_AXI_MISC_13                          0x024c
#define PCIE_CTRL_AXI_MISC_13_SLV_AWMISC_INFO_ATU_BYPASS_SHIFT                           0
#define PCIE_CTRL_AXI_MISC_13_SLV_AWMISC_INFO_ATU_BYPASS_MASK                            0x00000001
#define PCIE_CTRL_AXI_MISC_13_SLV_AWMISC_INFO_P_TAG_SHIFT                                1
#define PCIE_CTRL_AXI_MISC_13_SLV_AWMISC_INFO_P_TAG_MASK                                 0x000007fe
#define PCIE_CTRL_AXI_MISC_14                          0x0250
#define PCIE_CTRL_AXI_MISC_14_SLV_ARMISC_INFO_ATU_BYPASS_SHIFT                           0
#define PCIE_CTRL_AXI_MISC_14_SLV_ARMISC_INFO_ATU_BYPASS_MASK                            0x00000001
#define PCIE_CTRL_AXI_MISC_14_SLV_ARMISC_INFO_SHIFT                                      1
#define PCIE_CTRL_AXI_MISC_14_SLV_ARMISC_INFO_MASK                                       0x03fffffe
#define PCIE_CTRL_AXI_MISC_15                          0x0254
#define PCIE_CTRL_AXI_MISC_15_SLV_WMISC_INFO_SILENTDROP_SHIFT                            0
#define PCIE_CTRL_AXI_MISC_15_SLV_WMISC_INFO_SILENTDROP_MASK                             0x00000001
#define PCIE_CTRL_AXI_MISC_15_SLV_WMISC_INFO_EP_SHIFT                                    1
#define PCIE_CTRL_AXI_MISC_15_SLV_WMISC_INFO_EP_MASK                                     0x00000002
#define PCIE_CTRL_AXI_MISC_15_MSTR_RMISC_INFO_SHIFT                                      2
#define PCIE_CTRL_AXI_MISC_15_MSTR_RMISC_INFO_MASK                                       0x00007ffc
#define PCIE_CTRL_RTRGT1_INTF_SIGS                     0x0258
#define PCIE_CTRL_RTRGT1_INTF_SIGS_RADM_TRGT1_ATU_SLOC_MATCH_SHIFT                       0
#define PCIE_CTRL_RTRGT1_INTF_SIGS_RADM_TRGT1_ATU_SLOC_MATCH_MASK                        0x0000ffff
#define PCIE_CTRL_DIAG_STATU_0                         0x025c
#define PCIE_CTRL_DIAG_STATU_1                         0x0260
#define PCIE_CTRL_DIAG_STATU_2                         0x0264
#define PCIE_CTRL_DIAG_STATU_3                         0x0268
#define PCIE_CTRL_DIAG_STATU_4                         0x026c
#define PCIE_CTRL_DIAG_STATU_5                         0x0270
#define PCIE_CTRL_DIAG_STATU_6                         0x0274
#define PCIE_CTRL_DIAG_STATU_7                         0x0278
#define PCIE_CTRL_DIAG_STATU_8                         0x027c
#define PCIE_CTRL_DIAG_STATU_9                         0x0280
#define PCIE_CTRL_DIAG_STATU_10                        0x0284
#define PCIE_CTRL_DIAG_STATU_11                        0x0288
#define PCIE_CTRL_DIAG_STATU_12                        0x028c
#define PCIE_CTRL_DIAG_STATU_13                        0x0290
#define PCIE_CTRL_DIAG_STATU_14                        0x0294
#define PCIE_CTRL_DIAG_STATU_15                        0x0298
#define PCIE_CTRL_DIAG_STATU_16                        0x029c
#define PCIE_CTRL_DIAG_STATU_17                        0x02a0
#define PCIE_CTRL_DIAG_STATU_18                        0x02a4
#define PCIE_CTRL_DIAG_STATU_19                        0x02a8
#define PCIE_CTRL_DIAG_STATU_20                        0x02ac
#define PCIE_CTRL_DIAG_STATU_20_DIAG_STATU_DW20_SHIFT                                    0
#define PCIE_CTRL_DIAG_STATU_20_DIAG_STATU_DW20_MASK                                     0x00ffffff
#define PCIE_CTRL_CXPL_DEBUG_INFO_L                    0x02b0
#define PCIE_CTRL_CXPL_DEBUG_INFO_H                    0x02b4
#define PCIE_CTRL_CXPL_DEBUG_INFO_EI                   0x02b8
#define PCIE_CTRL_CXPL_DEBUG_INFO_EI_CXPL_DEBUG_INFO_EI_SHIFT                            0
#define PCIE_CTRL_CXPL_DEBUG_INFO_EI_CXPL_DEBUG_INFO_EI_MASK                             0x0000ffff
#define PCIE_CTRL_PCIE_TESTPIN_GROUP_SEL               0x02bc
#define PCIE_CTRL_PCIE_TESTPIN_GROUP_SEL_PCIE_TESTPIN_GROUP_SEL_SHIFT                    0
#define PCIE_CTRL_PCIE_TESTPIN_GROUP_SEL_PCIE_TESTPIN_GROUP_SEL_MASK                     0x000000ff
#define PCIE_CTRL_LTSSM_SW_CTRL_EN                     0x02c0
#define PCIE_CTRL_LTSSM_SW_CTRL_EN_LTSSM_SW_CTRL_EN_SHIFT                                0
#define PCIE_CTRL_LTSSM_SW_CTRL_EN_LTSSM_SW_CTRL_EN_MASK                                 0x00000001
#define PCIE_CTRL_SRAM                                 0x02c4
#define PCIE_CTRL_SRAM_RAM_DSLP_SHIFT                                                    0
#define PCIE_CTRL_SRAM_RAM_DSLP_MASK                                                     0x00000001
#define PCIE_CTRL_APP_INTX_ENABLE                      0x02c8
#define PCIE_CTRL_APP_INTX_ENABLE_APP_INTX_ENABLE_SHIFT                                  0
#define PCIE_CTRL_APP_INTX_ENABLE_APP_INTX_ENABLE_MASK                                   0x00000001
#define PCIE_CTRL_APP_CLKRST_SYNC_PERST_N_SEL          0x02cc
#define PCIE_CTRL_APP_CLKRST_SYNC_PERST_N_SEL_APP_CLKRST_SYNC_PERST_N_SEL_SHIFT          0
#define PCIE_CTRL_APP_CLKRST_SYNC_PERST_N_SEL_APP_CLKRST_SYNC_PERST_N_SEL_MASK           0x00000001
#define PCIE_CTRL_APP_CLKRST_SYNC_PERST_N              0x02d0
#define PCIE_CTRL_APP_CLKRST_SYNC_PERST_N_APP_CLKRST_SYNC_PERST_N_SHIFT                  0
#define PCIE_CTRL_APP_CLKRST_SYNC_PERST_N_APP_CLKRST_SYNC_PERST_N_MASK                   0x00000001
#define PCIE_CTRL_DEVICE_CFG                           0x02d4
#define PCIE_CTRL_DEVICE_CFG_APP_DEV_NUM_SHIFT                                           0
#define PCIE_CTRL_DEVICE_CFG_APP_DEV_NUM_MASK                                            0x0000001f
#define PCIE_CTRL_DEVICE_CFG_APP_BUS_NUM_SHIFT                                           5
#define PCIE_CTRL_DEVICE_CFG_APP_BUS_NUM_MASK                                            0x00001fe0
#define PCIE_CTRL_DBI_ACCESS                           0x02d8
#define PCIE_CTRL_DBI_ACCESS_DBI_IO_ACCESS_SHIFT                                         0
#define PCIE_CTRL_DBI_ACCESS_DBI_IO_ACCESS_MASK                                          0x00000001
#define PCIE_CTRL_DBI_ACCESS_DBI_ROM_ACCESS_SHIFT                                        1
#define PCIE_CTRL_DBI_ACCESS_DBI_ROM_ACCESS_MASK                                         0x00000002
#define PCIE_CTRL_DBI_ACCESS_DBI_BAR_NUM_SHIFT                                           2
#define PCIE_CTRL_DBI_ACCESS_DBI_BAR_NUM_MASK                                            0x0000001c
#define PCIE_CTRL_DBI_ACCESS_DBI_FUNC_NUM_SHIFT                                          5
#define PCIE_CTRL_DBI_ACCESS_DBI_FUNC_NUM_MASK                                           0x000000e0
#define PCIE_CTRL_PM_UNLOCK_MSG                        0x02dc
#define PCIE_CTRL_PM_UNLOCK_MSG_APP_UNLOCK_MSG_SHIFT                                     0
#define PCIE_CTRL_L2_ENTRY_EN                          0x02e0
#define PCIE_CTRL_L2_ENTRY_EN_APPS_PM_XMT_TURNOFF_SHIFT                                  0
#define PCIE_CTRL_POWER_CFG                            0x02e4
#define PCIE_CTRL_POWER_CFG_CFG_PWR_CTRLER_CTRL_SHIFT                                    0
#define PCIE_CTRL_POWER_CFG_CFG_PWR_CTRLER_CTRL_MASK                                     0x0000000f
#define PCIE_CTRL_POWER_CFG_CFG_ATTEN_IND_SHIFT                                          4
#define PCIE_CTRL_POWER_CFG_CFG_ATTEN_IND_MASK                                           0x00000ff0
#define PCIE_CTRL_POWER_CFG_CFG_PWR_IND_SHIFT                                            12
#define PCIE_CTRL_POWER_CFG_CFG_PWR_IND_MASK                                             0x000ff000
#define PCIE_CTRL_CFG_2NDBUS_NUM                       0x02e8
#define PCIE_CTRL_CFG_SUBBUS_NUM                       0x02ec
#define PCIE_CTRL_CFG_2ND_RESET                        0x02f0
#define PCIE_CTRL_CFG_2ND_RESET_CFG_2ND_RESET_SHIFT                                      0
#define PCIE_CTRL_CFG_2ND_RESET_CFG_2ND_RESET_MASK                                       0x00000001
#define PCIE_CTRL_MSI_CTRL_INT_VEC                     0x02f4
#define PCIE_CTRL_MSI_CTRL_INT_VEC_MSI_CTRL_INT_VEC_SHIFT                                0
#define PCIE_CTRL_MSI_CTRL_INT_VEC_MSI_CTRL_INT_VEC_MASK                                 0x000000ff
#define PCIE_CTRL_CFG_AER_INT_MSG_NUM                  0x02f8
#define PCIE_CTRL_CFG_AER_INT_MSG_NUM_CFG_AER_INT_MSG_NUM_SHIFT                          0
#define PCIE_CTRL_CFG_AER_INT_MSG_NUM_CFG_AER_INT_MSG_NUM_MASK                           0x000fffff
#define PCIE_CTRL_CFG_PCIE_CAP_INT_MSG_NUM             0x02fc
#define PCIE_CTRL_CFG_PCIE_CAP_INT_MSG_NUM_CFG_PCIE_CAP_INT_MSG_NUM_SHIFT                0
#define PCIE_CTRL_CFG_PCIE_CAP_INT_MSG_NUM_CFG_PCIE_CAP_INT_MSG_NUM_MASK                 0x000fffff
#define PCIE_CTRL_CFG_BR_CTRL_SERREN                   0x0300
#define PCIE_CTRL_CFG_BR_CTRL_SERREN_CFG_BR_CTRL_SERREN_SHIFT                            0
#define PCIE_CTRL_CFG_BR_CTRL_SERREN_CFG_BR_CTRL_SERREN_MASK                             0x0000000f
#define PCIE_CTRL_CFG_CRS_SW_VIS_EN                    0x0304
#define PCIE_CTRL_CFG_CRS_SW_VIS_EN_CFG_CRS_SW_VIS_EN_SHIFT                              0
#define PCIE_CTRL_CFG_CRS_SW_VIS_EN_CFG_CRS_SW_VIS_EN_MASK                               0x0000000f
#define PCIE_CTRL_RTLH_RFC_DATA                        0x0308
#define PCIE_CTRL_CFG_EML_CONTROL                      0x030c
#define PCIE_CTRL_CFG_EML_CONTROL_CFG_EML_CONTROL_SHIFT                                  0
#define PCIE_CTRL_CFG_EML_CONTROL_CFG_EML_CONTROL_MASK                                   0x0000000f
#define PCIE_CTRL_DBG_MSIX                             0x0310
#define PCIE_CTRL_DBG_MSIX_DBG_TABLE_SHIFT                                               0
#define PCIE_CTRL_DBG_MSIX_DBG_TABLE_MASK                                                0x00000001
#define PCIE_CTRL_DBG_MSIX_DBG_PBA_SHIFT                                                 1
#define PCIE_CTRL_DBG_MSIX_DBG_PBA_MASK                                                  0x00000002
#define PCIE_CTRL_PERST_N_OVERRIDE                     0x0314
#define PCIE_CTRL_PERST_N_OVERRIDE_PERST_N_SRC_SEL_SHIFT                                 0
#define PCIE_CTRL_PERST_N_OVERRIDE_PERST_N_SRC_SEL_MASK                                  0x00000001
#define PCIE_CTRL_PERST_N_OVERRIDE_PERST_N_SRC_OVER_SHIFT                                1
#define PCIE_CTRL_PERST_N_OVERRIDE_PERST_N_SRC_OVER_MASK                                 0x00000002
#define PCIE_CTRL_WAKE_OVERRIDE                        0x0318
#define PCIE_CTRL_WAKE_OVERRIDE_WAKE_SRC_SEL_SHIFT                                       0
#define PCIE_CTRL_WAKE_OVERRIDE_WAKE_SRC_SEL_MASK                                        0x00000001
#define PCIE_CTRL_WAKE_OVERRIDE_WAKE_SRC_OVER_SHIFT                                      1
#define PCIE_CTRL_WAKE_OVERRIDE_WAKE_SRC_OVER_MASK                                       0x00000002
#define PCIE_CTRL_EP_TO_RC_INT                         0x031c
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT0_SHIFT                                       0
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT0_MASK                                        0x00000001
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT1_SHIFT                                       1
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT1_MASK                                        0x00000002
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT2_SHIFT                                       2
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT2_MASK                                        0x00000004
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT3_SHIFT                                       3
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT3_MASK                                        0x00000008
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT4_SHIFT                                       4
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT4_MASK                                        0x00000010
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT5_SHIFT                                       5
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT5_MASK                                        0x00000020
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT6_SHIFT                                       6
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT6_MASK                                        0x00000040
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT7_SHIFT                                       7
#define PCIE_CTRL_EP_TO_RC_INT_EP_TO_RC_INT7_MASK                                        0x00000080
#define PCIE_CTRL_SMLH_LTSSM_STATE                     0x0320
#define PCIE_CTRL_SMLH_LTSSM_STATE_SMLH_LTSSM_STATE_SHIFT                                0
#define PCIE_CTRL_SMLH_LTSSM_STATE_SMLH_LTSSM_STATE_MASK                                 0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_EN                  0x0324
#define PCIE_CTRL_SMLH_LTSSM_STATE_EN_SMLH_LTSSM_STATE_EN_SHIFT                          0
#define PCIE_CTRL_SMLH_LTSSM_STATE_EN_SMLH_LTSSM_STATE_EN_MASK                           0x00000001
#define PCIE_CTRL_SMLH_LTSSM_STATE_1D                  0x0328
#define PCIE_CTRL_SMLH_LTSSM_STATE_1D_SMLH_LTSSM_STATE_1D_SHIFT                          0
#define PCIE_CTRL_SMLH_LTSSM_STATE_1D_SMLH_LTSSM_STATE_1D_MASK                           0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_2D                  0x032c
#define PCIE_CTRL_SMLH_LTSSM_STATE_2D_SMLH_LTSSM_STATE_2D_SHIFT                          0
#define PCIE_CTRL_SMLH_LTSSM_STATE_2D_SMLH_LTSSM_STATE_2D_MASK                           0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_3D                  0x0330
#define PCIE_CTRL_SMLH_LTSSM_STATE_3D_SMLH_LTSSM_STATE_3D_SHIFT                          0
#define PCIE_CTRL_SMLH_LTSSM_STATE_3D_SMLH_LTSSM_STATE_3D_MASK                           0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_4D                  0x0334
#define PCIE_CTRL_SMLH_LTSSM_STATE_4D_SMLH_LTSSM_STATE_4D_SHIFT                          0
#define PCIE_CTRL_SMLH_LTSSM_STATE_4D_SMLH_LTSSM_STATE_4D_MASK                           0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_5D                  0x0338
#define PCIE_CTRL_SMLH_LTSSM_STATE_5D_SMLH_LTSSM_STATE_5D_SHIFT                          0
#define PCIE_CTRL_SMLH_LTSSM_STATE_5D_SMLH_LTSSM_STATE_5D_MASK                           0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_6D                  0x033c
#define PCIE_CTRL_SMLH_LTSSM_STATE_6D_SMLH_LTSSM_STATE_6D_SHIFT                          0
#define PCIE_CTRL_SMLH_LTSSM_STATE_6D_SMLH_LTSSM_STATE_6D_MASK                           0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_7D                  0x0340
#define PCIE_CTRL_SMLH_LTSSM_STATE_7D_SMLH_LTSSM_STATE_7D_SHIFT                          0
#define PCIE_CTRL_SMLH_LTSSM_STATE_7D_SMLH_LTSSM_STATE_7D_MASK                           0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_8D                  0x0344
#define PCIE_CTRL_SMLH_LTSSM_STATE_8D_SMLH_LTSSM_STATE_8D_SHIFT                          0
#define PCIE_CTRL_SMLH_LTSSM_STATE_8D_SMLH_LTSSM_STATE_8D_MASK                           0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_9D                  0x0348
#define PCIE_CTRL_SMLH_LTSSM_STATE_9D_SMLH_LTSSM_STATE_9D_SHIFT                          0
#define PCIE_CTRL_SMLH_LTSSM_STATE_9D_SMLH_LTSSM_STATE_9D_MASK                           0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_10D                 0x034c
#define PCIE_CTRL_SMLH_LTSSM_STATE_10D_SMLH_LTSSM_STATE_10D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_10D_SMLH_LTSSM_STATE_10D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_11D                 0x0350
#define PCIE_CTRL_SMLH_LTSSM_STATE_11D_SMLH_LTSSM_STATE_11D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_11D_SMLH_LTSSM_STATE_11D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_12D                 0x0354
#define PCIE_CTRL_SMLH_LTSSM_STATE_12D_SMLH_LTSSM_STATE_12D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_12D_SMLH_LTSSM_STATE_12D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_13D                 0x0358
#define PCIE_CTRL_SMLH_LTSSM_STATE_13D_SMLH_LTSSM_STATE_13D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_13D_SMLH_LTSSM_STATE_13D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_14D                 0x035c
#define PCIE_CTRL_SMLH_LTSSM_STATE_14D_SMLH_LTSSM_STATE_14D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_14D_SMLH_LTSSM_STATE_14D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_15D                 0x0360
#define PCIE_CTRL_SMLH_LTSSM_STATE_15D_SMLH_LTSSM_STATE_15D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_15D_SMLH_LTSSM_STATE_15D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_16D                 0x0364
#define PCIE_CTRL_SMLH_LTSSM_STATE_16D_SMLH_LTSSM_STATE_16D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_16D_SMLH_LTSSM_STATE_16D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_17D                 0x0368
#define PCIE_CTRL_SMLH_LTSSM_STATE_17D_SMLH_LTSSM_STATE_17D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_17D_SMLH_LTSSM_STATE_17D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_18D                 0x036c
#define PCIE_CTRL_SMLH_LTSSM_STATE_18D_SMLH_LTSSM_STATE_18D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_18D_SMLH_LTSSM_STATE_18D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_19D                 0x0370
#define PCIE_CTRL_SMLH_LTSSM_STATE_19D_SMLH_LTSSM_STATE_19D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_19D_SMLH_LTSSM_STATE_19D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_20D                 0x0374
#define PCIE_CTRL_SMLH_LTSSM_STATE_20D_SMLH_LTSSM_STATE_20D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_20D_SMLH_LTSSM_STATE_20D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_21D                 0x0378
#define PCIE_CTRL_SMLH_LTSSM_STATE_21D_SMLH_LTSSM_STATE_21D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_21D_SMLH_LTSSM_STATE_21D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_22D                 0x037c
#define PCIE_CTRL_SMLH_LTSSM_STATE_22D_SMLH_LTSSM_STATE_22D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_22D_SMLH_LTSSM_STATE_22D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_23D                 0x0380
#define PCIE_CTRL_SMLH_LTSSM_STATE_23D_SMLH_LTSSM_STATE_23D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_23D_SMLH_LTSSM_STATE_23D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_24D                 0x0384
#define PCIE_CTRL_SMLH_LTSSM_STATE_24D_SMLH_LTSSM_STATE_24D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_24D_SMLH_LTSSM_STATE_24D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_25D                 0x0388
#define PCIE_CTRL_SMLH_LTSSM_STATE_25D_SMLH_LTSSM_STATE_25D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_25D_SMLH_LTSSM_STATE_25D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_26D                 0x038c
#define PCIE_CTRL_SMLH_LTSSM_STATE_26D_SMLH_LTSSM_STATE_26D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_26D_SMLH_LTSSM_STATE_26D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_27D                 0x0390
#define PCIE_CTRL_SMLH_LTSSM_STATE_27D_SMLH_LTSSM_STATE_27D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_27D_SMLH_LTSSM_STATE_27D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_28D                 0x0394
#define PCIE_CTRL_SMLH_LTSSM_STATE_28D_SMLH_LTSSM_STATE_28D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_28D_SMLH_LTSSM_STATE_28D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_29D                 0x0398
#define PCIE_CTRL_SMLH_LTSSM_STATE_29D_SMLH_LTSSM_STATE_29D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_29D_SMLH_LTSSM_STATE_29D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_30D                 0x039c
#define PCIE_CTRL_SMLH_LTSSM_STATE_30D_SMLH_LTSSM_STATE_30D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_30D_SMLH_LTSSM_STATE_30D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_31D                 0x03a0
#define PCIE_CTRL_SMLH_LTSSM_STATE_31D_SMLH_LTSSM_STATE_31D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_31D_SMLH_LTSSM_STATE_31D_MASK                         0x0000003f
#define PCIE_CTRL_SMLH_LTSSM_STATE_32D                 0x03a4
#define PCIE_CTRL_SMLH_LTSSM_STATE_32D_SMLH_LTSSM_STATE_32D_SHIFT                        0
#define PCIE_CTRL_SMLH_LTSSM_STATE_32D_SMLH_LTSSM_STATE_32D_MASK                         0x0000003f
#define PCIE_CTRL_PM_L1SUB_STATE_EN                    0x03a8
#define PCIE_CTRL_PM_L1SUB_STATE_EN_PM_L1SUB_STATE_EN_SHIFT                              0
#define PCIE_CTRL_PM_L1SUB_STATE_EN_PM_L1SUB_STATE_EN_MASK                               0x00000001
#define PCIE_CTRL_PM_L1SUB_STATE_1D                    0x03ac
#define PCIE_CTRL_PM_L1SUB_STATE_1D_PM_L1SUB_STATE_1D_SHIFT                              0
#define PCIE_CTRL_PM_L1SUB_STATE_1D_PM_L1SUB_STATE_1D_MASK                               0x00000007
#define PCIE_CTRL_PM_L1SUB_STATE_2D                    0x03b0
#define PCIE_CTRL_PM_L1SUB_STATE_2D_PM_L1SUB_STATE_2D_SHIFT                              0
#define PCIE_CTRL_PM_L1SUB_STATE_2D_PM_L1SUB_STATE_2D_MASK                               0x00000007
#define PCIE_CTRL_PM_L1SUB_STATE_3D                    0x03b4
#define PCIE_CTRL_PM_L1SUB_STATE_3D_PM_L1SUB_STATE_3D_SHIFT                              0
#define PCIE_CTRL_PM_L1SUB_STATE_3D_PM_L1SUB_STATE_3D_MASK                               0x00000007
#define PCIE_CTRL_PM_L1SUB_STATE_4D                    0x03b8
#define PCIE_CTRL_PM_L1SUB_STATE_4D_PM_L1SUB_STATE_4D_SHIFT                              0
#define PCIE_CTRL_PM_L1SUB_STATE_4D_PM_L1SUB_STATE_4D_MASK                               0x00000007
#define PCIE_CTRL_PM_L1SUB_STATE_5D                    0x03bc
#define PCIE_CTRL_PM_L1SUB_STATE_5D_PM_L1SUB_STATE_5D_SHIFT                              0
#define PCIE_CTRL_PM_L1SUB_STATE_5D_PM_L1SUB_STATE_5D_MASK                               0x00000007
#define PCIE_CTRL_PM_L1SUB_STATE_6D                    0x03c0
#define PCIE_CTRL_PM_L1SUB_STATE_6D_PM_L1SUB_STATE_6D_SHIFT                              0
#define PCIE_CTRL_PM_L1SUB_STATE_6D_PM_L1SUB_STATE_6D_MASK                               0x00000007
#define PCIE_CTRL_PM_L1SUB_STATE_7D                    0x03c4
#define PCIE_CTRL_PM_L1SUB_STATE_7D_PM_L1SUB_STATE_7D_SHIFT                              0
#define PCIE_CTRL_PM_L1SUB_STATE_7D_PM_L1SUB_STATE_7D_MASK                               0x00000007
#define PCIE_CTRL_PM_L1SUB_STATE_8D                    0x03c8
#define PCIE_CTRL_PM_L1SUB_STATE_8D_PM_L1SUB_STATE_8D_SHIFT                              0
#define PCIE_CTRL_PM_L1SUB_STATE_8D_PM_L1SUB_STATE_8D_MASK                               0x00000007
#define PCIE_CTRL_PCIE_L1SS_STATE_OVRD                 0x03cc
#define PCIE_CTRL_PCIE_L1SS_STATE_OVRD_PCIE_L1SS_STATE_OVRD_SHIFT                        0
#define PCIE_CTRL_PCIE_L1SS_STATE_OVRD_PCIE_L1SS_STATE_OVRD_MASK                         0x00000001
#define PCIE_CTRL_MSTR_AWQOS_SEL                       0x03d0
#define PCIE_CTRL_MSTR_AWQOS_SEL_MSTR_AWQOS_SEL_SHIFT                                    0
#define PCIE_CTRL_MSTR_AWQOS_SEL_MSTR_AWQOS_SEL_MASK                                     0x00000001
#define PCIE_CTRL_MSTR_AWQOS_VAL                       0x03d4
#define PCIE_CTRL_MSTR_AWQOS_VAL_MSTR_AWQOS_VAL_SHIFT                                    0
#define PCIE_CTRL_MSTR_AWQOS_VAL_MSTR_AWQOS_VAL_MASK                                     0x0000000f
#define PCIE_CTRL_MSTR_ARQOS_SEL                       0x03d8
#define PCIE_CTRL_MSTR_ARQOS_SEL_MSTR_ARQOS_SEL_SHIFT                                    0
#define PCIE_CTRL_MSTR_ARQOS_SEL_MSTR_ARQOS_SEL_MASK                                     0x00000001
#define PCIE_CTRL_MSTR_ARQOS_VAL                       0x03dc
#define PCIE_CTRL_MSTR_ARQOS_VAL_MSTR_ARQOS_VAL_SHIFT                                    0
#define PCIE_CTRL_MSTR_ARQOS_VAL_MSTR_ARQOS_VAL_MASK                                     0x0000000f
#define PCIE_CTRL_MASK_L11_DOZE                        0x03e0
#define PCIE_CTRL_MASK_L11_DOZE_MASK_L11_DOZE_SHIFT                                      0
#define PCIE_CTRL_MASK_L11_DOZE_MASK_L11_DOZE_MASK                                       0x00000001
#define PCIE_CTRL_MASK_L12_DOZE                        0x03e4
#define PCIE_CTRL_MASK_L12_DOZE_MASK_L12_DOZE_SHIFT                                      0
#define PCIE_CTRL_MASK_L12_DOZE_MASK_L12_DOZE_MASK                                       0x00000001
#define PCIE_CTRL_MASK_L2_DOZE                         0x03e8
#define PCIE_CTRL_MASK_L2_DOZE_MASK_L2_DOZE_SHIFT                                        0
#define PCIE_CTRL_MASK_L2_DOZE_MASK_L2_DOZE_MASK                                         0x00000001
#define PCIE_CTRL_OUTBAND_PWRUP_CMD                    0x03ec
#define PCIE_CTRL_OUTBAND_PWRUP_CMD_OUTBAND_PWRUP_CMD_SHIFT                              0
#define PCIE_CTRL_OUTBAND_PWRUP_CMD_OUTBAND_PWRUP_CMD_MASK                               0x0000000f
#define PCIE_CTRL_SYS_AUX_PWR_DET                      0x03f0
#define PCIE_CTRL_SYS_AUX_PWR_DET_SYS_AUX_PWR_DET_SHIFT                                  0
#define PCIE_CTRL_SYS_AUX_PWR_DET_SYS_AUX_PWR_DET_MASK                                   0x00000001
#define PCIE_CTRL_SYS_ATTEN_BUTTON_PRESSED             0x03f4
#define PCIE_CTRL_SYS_ATTEN_BUTTON_PRESSED_SYS_ATTEN_BUTTON_PRESSED_SHIFT                0
#define PCIE_CTRL_SYS_ATTEN_BUTTON_PRESSED_SYS_ATTEN_BUTTON_PRESSED_MASK                 0x0000000f
#define PCIE_CTRL_SYS_PRE_DET_STATE                    0x03f8
#define PCIE_CTRL_SYS_PRE_DET_STATE_SYS_PRE_DET_STATE_SHIFT                              0
#define PCIE_CTRL_SYS_PRE_DET_STATE_SYS_PRE_DET_STATE_MASK                               0x0000000f
#define PCIE_CTRL_SYS_MRL_SENSOR_STATE                 0x03fc
#define PCIE_CTRL_SYS_MRL_SENSOR_STATE_SYS_MRL_SENSOR_STATE_SHIFT                        0
#define PCIE_CTRL_SYS_MRL_SENSOR_STATE_SYS_MRL_SENSOR_STATE_MASK                         0x0000000f
#define PCIE_CTRL_SYS_PWR_FAULT_DET                    0x0400
#define PCIE_CTRL_SYS_PWR_FAULT_DET_SYS_PWR_FAULT_DET_SHIFT                              0
#define PCIE_CTRL_SYS_PWR_FAULT_DET_SYS_PWR_FAULT_DET_MASK                               0x0000000f
#define PCIE_CTRL_SYS_MRL_SENSOR_CHGED                 0x0404
#define PCIE_CTRL_SYS_MRL_SENSOR_CHGED_SYS_MRL_SENSOR_CHGED_SHIFT                        0
#define PCIE_CTRL_SYS_MRL_SENSOR_CHGED_SYS_MRL_SENSOR_CHGED_MASK                         0x0000000f
#define PCIE_CTRL_SYS_PRE_DET_CHGED                    0x0408
#define PCIE_CTRL_SYS_PRE_DET_CHGED_SYS_PRE_DET_CHGED_SHIFT                              0
#define PCIE_CTRL_SYS_PRE_DET_CHGED_SYS_PRE_DET_CHGED_MASK                               0x0000000f
#define PCIE_CTRL_SYS_CMD_CPLED_INT                    0x040c
#define PCIE_CTRL_SYS_CMD_CPLED_INT_SYS_CMD_CPLED_INT_SHIFT                              0
#define PCIE_CTRL_SYS_CMD_CPLED_INT_SYS_CMD_CPLED_INT_MASK                               0x0000000f
#define PCIE_CTRL_SYS_EML_INTERLOCK_ENGAGED            0x0410
#define PCIE_CTRL_SYS_EML_INTERLOCK_ENGAGED_SYS_EML_INTERLOCK_ENGAGED_SHIFT              0
#define PCIE_CTRL_SYS_EML_INTERLOCK_ENGAGED_SYS_EML_INTERLOCK_ENGAGED_MASK               0x0000000f
#define PCIE_CTRL_DEVICE_TYPE                          0x0414
#define PCIE_CTRL_DEVICE_TYPE_DEVICE_TYPE_SHIFT                                          0
#define PCIE_CTRL_DEVICE_TYPE_DEVICE_TYPE_MASK                                           0x0000000f
#define PCIE_CTRL_CFG_MSI_ADDR_0                       0x0418
#define PCIE_CTRL_CFG_MSI_ADDR_1                       0x041c
#define PCIE_CTRL_CFG_MSI_ADDR_2                       0x0420
#define PCIE_CTRL_CFG_MSI_ADDR_3                       0x0424
#define PCIE_CTRL_CFG_MSI_ADDR_4                       0x0428
#define PCIE_CTRL_CFG_MSI_ADDR_5                       0x042c
#define PCIE_CTRL_CFG_MSI_ADDR_6                       0x0430
#define PCIE_CTRL_CFG_MSI_ADDR_7                       0x0434
#define PCIE_CTRL_CFG_MSI_DATA_0                       0x0438
#define PCIE_CTRL_CFG_MSI_DATA_1                       0x043c
#define PCIE_CTRL_CFG_MSI_DATA_2                       0x0440
#define PCIE_CTRL_CFG_MSI_DATA_3                       0x0444
#define PCIE_CTRL_CFG_MSI_64                           0x0448
#define PCIE_CTRL_CFG_MSI_64_CFG_MSI_64_SHIFT                                            0
#define PCIE_CTRL_CFG_MSI_64_CFG_MSI_64_MASK                                             0x0000000f
#define PCIE_CTRL_CFG_MULTI_MSI_EN                     0x044c
#define PCIE_CTRL_CFG_MULTI_MSI_EN_CFG_MULTI_MSI_EN_SHIFT                                0
#define PCIE_CTRL_CFG_MULTI_MSI_EN_CFG_MULTI_MSI_EN_MASK                                 0x00000fff
#define PCIE_CTRL_CFG_MSI_EXT_DATA_EN                  0x0450
#define PCIE_CTRL_CFG_MSI_EXT_DATA_EN_CFG_MSI_EXT_DATA_EN_SHIFT                          0
#define PCIE_CTRL_CFG_MSI_EXT_DATA_EN_CFG_MSI_EXT_DATA_EN_MASK                           0x0000000f
#define PCIE_CTRL_CFG_MSI_EN                           0x0454
#define PCIE_CTRL_CFG_MSI_EN_CFG_MSI_EN_SHIFT                                            0
#define PCIE_CTRL_CFG_MSI_EN_CFG_MSI_EN_MASK                                             0x0000000f
#define PCIE_CTRL_VEN_MSI_REQ                          0x0458
#define PCIE_CTRL_VEN_MSI_REQ_VEN_MSI_REQ_SHIFT                                          0
#define PCIE_CTRL_VEN_MSI_REQ_VEN_MSI_REQ_MASK                                           0x00000001
#define PCIE_CTRL_VEN_MSI_FUNC_NUM                     0x045c
#define PCIE_CTRL_VEN_MSI_FUNC_NUM_VEN_MSI_FUNC_NUM_SHIFT                                0
#define PCIE_CTRL_VEN_MSI_FUNC_NUM_VEN_MSI_FUNC_NUM_MASK                                 0x00000007
#define PCIE_CTRL_VEN_MSI_TC                           0x0460
#define PCIE_CTRL_VEN_MSI_TC_VEN_MSI_TC_SHIFT                                            0
#define PCIE_CTRL_VEN_MSI_TC_VEN_MSI_TC_MASK                                             0x00000007
#define PCIE_CTRL_VEN_MSI_VECTOR                       0x0464
#define PCIE_CTRL_VEN_MSI_VECTOR_VEN_MSI_VECTOR_SHIFT                                    0
#define PCIE_CTRL_VEN_MSI_VECTOR_VEN_MSI_VECTOR_MASK                                     0x0000001f
#define PCIE_CTRL_VEN_MSI_GRANT                        0x0468
#define PCIE_CTRL_VEN_MSI_GRANT_VEN_MSI_GRANT_SHIFT                                      0
#define PCIE_CTRL_VEN_MSI_GRANT_VEN_MSI_GRANT_MASK                                       0x00000001
#define PCIE_CTRL_SYS_INT                              0x046c
#define PCIE_CTRL_SYS_INT_SYS_INT_SHIFT                                                  0
#define PCIE_CTRL_SYS_INT_SYS_INT_MASK                                                   0x0000000f
#define PCIE_CTRL_MSI_CTRL_IO                          0x0470
#define PCIE_CTRL_RADM_TRGT1_VC                        0x0474
#define PCIE_CTRL_RADM_TRGT1_VC_RADM_TRGT1_VC_SHIFT                                      0
#define PCIE_CTRL_RADM_TRGT1_VC_RADM_TRGT1_VC_MASK                                       0x00000007
#define PCIE_CTRL_CFG_HW_AUTO_SP_DIS                   0x0478
#define PCIE_CTRL_CFG_HW_AUTO_SP_DIS_CFG_HW_AUTO_SP_DIS_SHIFT                            0
#define PCIE_CTRL_CFG_HW_AUTO_SP_DIS_CFG_HW_AUTO_SP_DIS_MASK                             0x00000001
#define PCIE_CTRL_EXT_LBC_DIN_0                        0x047c
#define PCIE_CTRL_EXT_LBC_DIN_1                        0x0480
#define PCIE_CTRL_EXT_LBC_DIN_2                        0x0484
#define PCIE_CTRL_EXT_LBC_DIN_3                        0x0488
#define PCIE_CTRL_RADM_IDLE                            0x048c
#define PCIE_CTRL_RADM_IDLE_RADM_IDLE_SHIFT                                              0
#define PCIE_CTRL_RADM_IDLE_RADM_IDLE_MASK                                               0x00000001
#define PCIE_CTRL_ACPU_MSI_ROUTE_0                     0x0490
#define PCIE_CTRL_ACPU_MSI_ROUTE_1                     0x0494
#define PCIE_CTRL_ACPU_MSI_ROUTE_2                     0x0498
#define PCIE_CTRL_ACPU_MSI_ROUTE_3                     0x049c
#define PCIE_CTRL_ACPU_MSI_ROUTE_4                     0x04a0
#define PCIE_CTRL_ACPU_MSI_ROUTE_5                     0x04a4
#define PCIE_CTRL_ACPU_MSI_ROUTE_6                     0x04a8
#define PCIE_CTRL_ACPU_MSI_ROUTE_7                     0x04ac
#define PCIE_CTRL_M3_MSI_ROUTE_0                       0x04b0
#define PCIE_CTRL_M3_MSI_ROUTE_1                       0x04b4
#define PCIE_CTRL_M3_MSI_ROUTE_2                       0x04b8
#define PCIE_CTRL_M3_MSI_ROUTE_3                       0x04bc
#define PCIE_CTRL_M3_MSI_ROUTE_4                       0x04c0
#define PCIE_CTRL_M3_MSI_ROUTE_5                       0x04c4
#define PCIE_CTRL_M3_MSI_ROUTE_6                       0x04c8
#define PCIE_CTRL_M3_MSI_ROUTE_7                       0x04cc
#define PCIE_CTRL_AUDIO_MSI_ROUTE_0                    0x04d0
#define PCIE_CTRL_AUDIO_MSI_ROUTE_1                    0x04d4
#define PCIE_CTRL_AUDIO_MSI_ROUTE_2                    0x04d8
#define PCIE_CTRL_AUDIO_MSI_ROUTE_3                    0x04dc
#define PCIE_CTRL_AUDIO_MSI_ROUTE_4                    0x04e0
#define PCIE_CTRL_AUDIO_MSI_ROUTE_5                    0x04e4
#define PCIE_CTRL_AUDIO_MSI_ROUTE_6                    0x04e8
#define PCIE_CTRL_AUDIO_MSI_ROUTE_7                    0x04ec
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW                 0x1000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_RADM_QOVERFLOW_RAW_SHIFT                          0
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_TRGT_CPL_TIMEOUT_RAW_SHIFT                        1
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_RADM_CPL_TIMEOUT_RAW_SHIFT                        2
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_LINK_DOWN_ERR_RAW_SHIFT                           3
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_LTSSM_L0_ENTRY_RCVRY_P_RAW_SHIFT                  4
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_SEND_F_ERR_RAW_SHIFT                          5
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_SEND_NF_ERR_RAW_SHIFT                         6
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_SEND_COR_ERR_RAW_SHIFT                        7
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_UNCOR_INTERNAL_ERR_STS_RAW_SHIFT              8
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_RCVR_OVERFLOW_ERR_STS_RAW_SHIFT               9
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_FC_PROTOCOL_ERR_STS_RAW_SHIFT                 10
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_MLF_TLP_ERR_STS_RAW_SHIFT                     11
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_SURPRISE_DOWN_ER_STS_RAW_SHIFT                12
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_DL_PROTOCOL_ERR_STS_RAW_SHIFT                 13
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_ECRC_ERR_STS_RAW_SHIFT                        14
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_CORRECTED_INTERNAL_ERR_STS_RAW_SHIFT          15
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_REPLAY_NUMBER_ROLLOVER_ERR_STS_RAW_SHIFT      16
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_REPLAY_TIMER_TIMEOUT_ERR_STS_RAW_SHIFT        17
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_BAD_DLLP_ERR_STS_RAW_SHIFT                    18
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_BAD_TLP_ERR_STS_RAW_SHIFT                     19
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_RCVR_ERR_STS_RAW_SHIFT                        20
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_RADM_RCVD_CFG0WR_POISONED_RAW_SHIFT               21
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_RADM_RCVD_CFG1WR_POISONED_RAW_SHIFT               22
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_ADVISORY_NF_STS_3_RAW_SHIFT                   23
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_ADVISORY_NF_STS_2_RAW_SHIFT                   24
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_ADVISORY_NF_STS_1_RAW_SHIFT                   25
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_ADVISORY_NF_STS_0_RAW_SHIFT                   26
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_HDR_LOG_OVERFLOW_STS_3_RAW_SHIFT              27
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_HDR_LOG_OVERFLOW_STS_2_RAW_SHIFT              28
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_HDR_LOG_OVERFLOW_STS_1_RAW_SHIFT              29
#define PCIE_CTRL_SYS_INT0_IRQ_INT_RAW_CFG_HDR_LOG_OVERFLOW_STS_0_RAW_SHIFT              30
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE               0x1004
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_RADM_QOVERFLOW_RAW_SHIFT                        0
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_TRGT_CPL_TIMEOUT_RAW_SHIFT                      1
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_RADM_CPL_TIMEOUT_RAW_SHIFT                      2
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_LINK_DOWN_ERR_RAW_SHIFT                         3
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_LTSSM_L0_ENTRY_RCVRY_P_RAW_SHIFT                4
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_SEND_F_ERR_RAW_SHIFT                        5
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_SEND_NF_ERR_RAW_SHIFT                       6
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_SEND_COR_ERR_RAW_SHIFT                      7
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_UNCOR_INTERNAL_ERR_STS_RAW_SHIFT            8
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_RCVR_OVERFLOW_ERR_STS_RAW_SHIFT             9
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_FC_PROTOCOL_ERR_STS_RAW_SHIFT               10
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_MLF_TLP_ERR_STS_RAW_SHIFT                   11
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_SURPRISE_DOWN_ER_STS_RAW_SHIFT              12
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_DL_PROTOCOL_ERR_STS_RAW_SHIFT               13
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_ECRC_ERR_STS_RAW_SHIFT                      14
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_CORRECTED_INTERNAL_ERR_STS_RAW_SHIFT        15
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_REPLAY_NUMBER_ROLLOVER_ERR_STS_RAW_SHIFT    16
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_REPLAY_TIMER_TIMEOUT_ERR_STS_RAW_SHIFT      17
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_BAD_DLLP_ERR_STS_RAW_SHIFT                  18
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_BAD_TLP_ERR_STS_RAW_SHIFT                   19
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_RCVR_ERR_STS_RAW_SHIFT                      20
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_RADM_RCVD_CFG0WR_POISONED_RAW_SHIFT             21
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_RADM_RCVD_CFG1WR_POISONED_RAW_SHIFT             22
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_ADVISORY_NF_STS_3_RAW_SHIFT                 23
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_ADVISORY_NF_STS_2_RAW_SHIFT                 24
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_ADVISORY_NF_STS_1_RAW_SHIFT                 25
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_ADVISORY_NF_STS_0_RAW_SHIFT                 26
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_HDR_LOG_OVERFLOW_STS_3_RAW_SHIFT            27
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_HDR_LOG_OVERFLOW_STS_2_RAW_SHIFT            28
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_HDR_LOG_OVERFLOW_STS_1_RAW_SHIFT            29
#define PCIE_CTRL_SYS_INT0_IRQ_INT_FORCE_CFG_HDR_LOG_OVERFLOW_STS_0_RAW_SHIFT            30
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK                0x1008
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_RADM_QOVERFLOW_MASK_SHIFT                        0
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_RADM_QOVERFLOW_MASK_MASK                         0x00000001
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_TRGT_CPL_TIMEOUT_MASK_SHIFT                      1
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_TRGT_CPL_TIMEOUT_MASK_MASK                       0x00000002
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_RADM_CPL_TIMEOUT_MASK_SHIFT                      2
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_RADM_CPL_TIMEOUT_MASK_MASK                       0x00000004
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_LINK_DOWN_ERR_MASK_SHIFT                         3
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_LINK_DOWN_ERR_MASK_MASK                          0x00000008
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_LTSSM_L0_ENTRY_RCVRY_P_MASK_SHIFT                4
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_LTSSM_L0_ENTRY_RCVRY_P_MASK_MASK                 0x00000010
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_SEND_F_ERR_MASK_SHIFT                        5
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_SEND_F_ERR_MASK_MASK                         0x00000020
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_SEND_NF_ERR_MASK_SHIFT                       6
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_SEND_NF_ERR_MASK_MASK                        0x00000040
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_SEND_COR_ERR_MASK_SHIFT                      7
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_SEND_COR_ERR_MASK_MASK                       0x00000080
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_UNCOR_INTERNAL_ERR_STS_MASK_SHIFT            8
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_UNCOR_INTERNAL_ERR_STS_MASK_MASK             0x00000100
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_RCVR_OVERFLOW_ERR_STS_MASK_SHIFT             9
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_RCVR_OVERFLOW_ERR_STS_MASK_MASK              0x00000200
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_FC_PROTOCOL_ERR_STS_MASK_SHIFT               10
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_FC_PROTOCOL_ERR_STS_MASK_MASK                0x00000400
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_MLF_TLP_ERR_STS_MASK_SHIFT                   11
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_MLF_TLP_ERR_STS_MASK_MASK                    0x00000800
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_SURPRISE_DOWN_ER_STS_MASK_SHIFT              12
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_SURPRISE_DOWN_ER_STS_MASK_MASK               0x00001000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_DL_PROTOCOL_ERR_STS_MASK_SHIFT               13
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_DL_PROTOCOL_ERR_STS_MASK_MASK                0x00002000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_ECRC_ERR_STS_MASK_SHIFT                      14
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_ECRC_ERR_STS_MASK_MASK                       0x00004000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_CORRECTED_INTERNAL_ERR_STS_MASK_SHIFT        15
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_CORRECTED_INTERNAL_ERR_STS_MASK_MASK         0x00008000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_REPLAY_NUMBER_ROLLOVER_ERR_STS_MASK_SHIFT    16
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_REPLAY_NUMBER_ROLLOVER_ERR_STS_MASK_MASK     0x00010000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_REPLAY_TIMER_TIMEOUT_ERR_STS_MASK_SHIFT      17
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_REPLAY_TIMER_TIMEOUT_ERR_STS_MASK_MASK       0x00020000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_BAD_DLLP_ERR_STS_MASK_SHIFT                  18
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_BAD_DLLP_ERR_STS_MASK_MASK                   0x00040000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_BAD_TLP_ERR_STS_MASK_SHIFT                   19
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_BAD_TLP_ERR_STS_MASK_MASK                    0x00080000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_RCVR_ERR_STS_MASK_SHIFT                      20
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_RCVR_ERR_STS_MASK_MASK                       0x00100000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_RADM_RCVD_CFG0WR_POISONED_MASK_SHIFT             21
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_RADM_RCVD_CFG0WR_POISONED_MASK_MASK              0x00200000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_RADM_RCVD_CFG1WR_POISONED_MASK_SHIFT             22
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_RADM_RCVD_CFG1WR_POISONED_MASK_MASK              0x00400000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_ADVISORY_NF_STS_3_MASK_SHIFT                 23
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_ADVISORY_NF_STS_3_MASK_MASK                  0x00800000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_ADVISORY_NF_STS_2_MASK_SHIFT                 24
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_ADVISORY_NF_STS_2_MASK_MASK                  0x01000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_ADVISORY_NF_STS_1_MASK_SHIFT                 25
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_ADVISORY_NF_STS_1_MASK_MASK                  0x02000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_ADVISORY_NF_STS_0_MASK_SHIFT                 26
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_ADVISORY_NF_STS_0_MASK_MASK                  0x04000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_HDR_LOG_OVERFLOW_STS_3_MASK_SHIFT            27
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_HDR_LOG_OVERFLOW_STS_3_MASK_MASK             0x08000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_HDR_LOG_OVERFLOW_STS_2_MASK_SHIFT            28
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_HDR_LOG_OVERFLOW_STS_2_MASK_MASK             0x10000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_HDR_LOG_OVERFLOW_STS_1_MASK_SHIFT            29
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_HDR_LOG_OVERFLOW_STS_1_MASK_MASK             0x20000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_HDR_LOG_OVERFLOW_STS_0_MASK_SHIFT            30
#define PCIE_CTRL_SYS_INT0_IRQ_INT_MASK_CFG_HDR_LOG_OVERFLOW_STS_0_MASK_MASK             0x40000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS              0x100c
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_RADM_QOVERFLOW_STATUS_SHIFT                    0
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_RADM_QOVERFLOW_STATUS_MASK                     0x00000001
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_TRGT_CPL_TIMEOUT_STATUS_SHIFT                  1
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_TRGT_CPL_TIMEOUT_STATUS_MASK                   0x00000002
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_RADM_CPL_TIMEOUT_STATUS_SHIFT                  2
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_RADM_CPL_TIMEOUT_STATUS_MASK                   0x00000004
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_LINK_DOWN_ERR_STATUS_SHIFT                     3
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_LINK_DOWN_ERR_STATUS_MASK                      0x00000008
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_LTSSM_L0_ENTRY_RCVRY_P_STATUS_SHIFT            4
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_LTSSM_L0_ENTRY_RCVRY_P_STATUS_MASK             0x00000010
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_SEND_F_ERR_STATUS_SHIFT                    5
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_SEND_F_ERR_STATUS_MASK                     0x00000020
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_SEND_NF_ERR_STATUS_SHIFT                   6
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_SEND_NF_ERR_STATUS_MASK                    0x00000040
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_SEND_COR_ERR_STATUS_SHIFT                  7
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_SEND_COR_ERR_STATUS_MASK                   0x00000080
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_UNCOR_INTERNAL_ERR_STS_STATUS_SHIFT        8
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_UNCOR_INTERNAL_ERR_STS_STATUS_MASK         0x00000100
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_RCVR_OVERFLOW_ERR_STS_STATUS_SHIFT         9
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_RCVR_OVERFLOW_ERR_STS_STATUS_MASK          0x00000200
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_FC_PROTOCOL_ERR_STS_STATUS_SHIFT           10
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_FC_PROTOCOL_ERR_STS_STATUS_MASK            0x00000400
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_MLF_TLP_ERR_STS_STATUS_SHIFT               11
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_MLF_TLP_ERR_STS_STATUS_MASK                0x00000800
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_SURPRISE_DOWN_ER_STS_STATUS_SHIFT          12
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_SURPRISE_DOWN_ER_STS_STATUS_MASK           0x00001000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_DL_PROTOCOL_ERR_STS_STATUS_SHIFT           13
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_DL_PROTOCOL_ERR_STS_STATUS_MASK            0x00002000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_ECRC_ERR_STS_STATUS_SHIFT                  14
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_ECRC_ERR_STS_STATUS_MASK                   0x00004000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_CORRECTED_INTERNAL_ERR_STS_STATUS_SHIFT    15
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_CORRECTED_INTERNAL_ERR_STS_STATUS_MASK     0x00008000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_REPLAY_NUMBER_ROLLOVER_ERR_STS_STATUS_SHIFT 16
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_REPLAY_NUMBER_ROLLOVER_ERR_STS_STATUS_MASK  0x00010000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_REPLAY_TIMER_TIMEOUT_ERR_STS_STATUS_SHIFT  17
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_REPLAY_TIMER_TIMEOUT_ERR_STS_STATUS_MASK   0x00020000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_BAD_DLLP_ERR_STS_STATUS_SHIFT              18
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_BAD_DLLP_ERR_STS_STATUS_MASK               0x00040000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_BAD_TLP_ERR_STS_STATUS_SHIFT               19
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_BAD_TLP_ERR_STS_STATUS_MASK                0x00080000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_RCVR_ERR_STS_STATUS_SHIFT                  20
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_RCVR_ERR_STS_STATUS_MASK                   0x00100000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_RADM_RCVD_CFG0WR_POISONED_STATUS_SHIFT         21
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_RADM_RCVD_CFG0WR_POISONED_STATUS_MASK          0x00200000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_RADM_RCVD_CFG1WR_POISONED_STATUS_SHIFT         22
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_RADM_RCVD_CFG1WR_POISONED_STATUS_MASK          0x00400000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_ADVISORY_NF_STS_3_STATUS_SHIFT             23
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_ADVISORY_NF_STS_3_STATUS_MASK              0x00800000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_ADVISORY_NF_STS_2_STATUS_SHIFT             24
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_ADVISORY_NF_STS_2_STATUS_MASK              0x01000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_ADVISORY_NF_STS_1_STATUS_SHIFT             25
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_ADVISORY_NF_STS_1_STATUS_MASK              0x02000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_ADVISORY_NF_STS_0_STATUS_SHIFT             26
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_ADVISORY_NF_STS_0_STATUS_MASK              0x04000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_HDR_LOG_OVERFLOW_STS_3_STATUS_SHIFT        27
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_HDR_LOG_OVERFLOW_STS_3_STATUS_MASK         0x08000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_HDR_LOG_OVERFLOW_STS_2_STATUS_SHIFT        28
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_HDR_LOG_OVERFLOW_STS_2_STATUS_MASK         0x10000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_HDR_LOG_OVERFLOW_STS_1_STATUS_SHIFT        29
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_HDR_LOG_OVERFLOW_STS_1_STATUS_MASK         0x20000000
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_HDR_LOG_OVERFLOW_STS_0_STATUS_SHIFT        30
#define PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS_CFG_HDR_LOG_OVERFLOW_STS_0_STATUS_MASK         0x40000000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW                 0x1010
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_CORRECTABLE_ERR_RAW_SHIFT                    0
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_NONFATAL_ERR_RAW_SHIFT                       1
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_FATAL_ERR_RAW_SHIFT                          2
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_CFG_SYS_ERR_RC_0_RAW_SHIFT                        3
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_CFG_SYS_ERR_RC_1_RAW_SHIFT                        4
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_CFG_SYS_ERR_RC_2_RAW_SHIFT                        5
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_CFG_SYS_ERR_RC_3_RAW_SHIFT                        6
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_CFG_AER_RC_ERR_MSI_0_RAW_SHIFT                    7
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_CFG_AER_RC_ERR_MSI_1_RAW_SHIFT                    8
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_CFG_AER_RC_ERR_MSI_2_RAW_SHIFT                    9
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_CFG_AER_RC_ERR_MSI_3_RAW_SHIFT                    10
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_15_RAW_SHIFT              11
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_14_RAW_SHIFT              12
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_13_RAW_SHIFT              13
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_12_RAW_SHIFT              14
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_11_RAW_SHIFT              15
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_10_RAW_SHIFT              16
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_9_RAW_SHIFT               17
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_8_RAW_SHIFT               18
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_7_RAW_SHIFT               19
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_6_RAW_SHIFT               20
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_5_RAW_SHIFT               21
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_4_RAW_SHIFT               22
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_3_RAW_SHIFT               23
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_2_RAW_SHIFT               24
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_1_RAW_SHIFT               25
#define PCIE_CTRL_SYS_INT1_IRQ_INT_RAW_RADM_TRGT1_ATU_CBUF_ERR_0_RAW_SHIFT               26
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE               0x1014
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_CORRECTABLE_ERR_RAW_SHIFT                  0
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_NONFATAL_ERR_RAW_SHIFT                     1
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_FATAL_ERR_RAW_SHIFT                        2
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_CFG_SYS_ERR_RC_0_RAW_SHIFT                      3
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_CFG_SYS_ERR_RC_1_RAW_SHIFT                      4
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_CFG_SYS_ERR_RC_2_RAW_SHIFT                      5
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_CFG_SYS_ERR_RC_3_RAW_SHIFT                      6
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_CFG_AER_RC_ERR_MSI_0_RAW_SHIFT                  7
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_CFG_AER_RC_ERR_MSI_1_RAW_SHIFT                  8
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_CFG_AER_RC_ERR_MSI_2_RAW_SHIFT                  9
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_CFG_AER_RC_ERR_MSI_3_RAW_SHIFT                  10
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_15_RAW_SHIFT            11
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_14_RAW_SHIFT            12
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_13_RAW_SHIFT            13
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_12_RAW_SHIFT            14
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_11_RAW_SHIFT            15
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_10_RAW_SHIFT            16
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_9_RAW_SHIFT             17
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_8_RAW_SHIFT             18
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_7_RAW_SHIFT             19
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_6_RAW_SHIFT             20
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_5_RAW_SHIFT             21
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_4_RAW_SHIFT             22
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_3_RAW_SHIFT             23
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_2_RAW_SHIFT             24
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_1_RAW_SHIFT             25
#define PCIE_CTRL_SYS_INT1_IRQ_INT_FORCE_RADM_TRGT1_ATU_CBUF_ERR_0_RAW_SHIFT             26
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK                0x1018
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_CORRECTABLE_ERR_MASK_SHIFT                  0
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_CORRECTABLE_ERR_MASK_MASK                   0x00000001
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_NONFATAL_ERR_MASK_SHIFT                     1
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_NONFATAL_ERR_MASK_MASK                      0x00000002
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_FATAL_ERR_MASK_SHIFT                        2
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_FATAL_ERR_MASK_MASK                         0x00000004
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_SYS_ERR_RC_0_MASK_SHIFT                      3
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_SYS_ERR_RC_0_MASK_MASK                       0x00000008
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_SYS_ERR_RC_1_MASK_SHIFT                      4
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_SYS_ERR_RC_1_MASK_MASK                       0x00000010
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_SYS_ERR_RC_2_MASK_SHIFT                      5
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_SYS_ERR_RC_2_MASK_MASK                       0x00000020
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_SYS_ERR_RC_3_MASK_SHIFT                      6
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_SYS_ERR_RC_3_MASK_MASK                       0x00000040
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_AER_RC_ERR_MSI_0_MASK_SHIFT                  7
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_AER_RC_ERR_MSI_0_MASK_MASK                   0x00000080
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_AER_RC_ERR_MSI_1_MASK_SHIFT                  8
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_AER_RC_ERR_MSI_1_MASK_MASK                   0x00000100
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_AER_RC_ERR_MSI_2_MASK_SHIFT                  9
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_AER_RC_ERR_MSI_2_MASK_MASK                   0x00000200
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_AER_RC_ERR_MSI_3_MASK_SHIFT                  10
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_CFG_AER_RC_ERR_MSI_3_MASK_MASK                   0x00000400
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_15_MASK_SHIFT            11
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_15_MASK_MASK             0x00000800
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_14_MASK_SHIFT            12
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_14_MASK_MASK             0x00001000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_13_MASK_SHIFT            13
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_13_MASK_MASK             0x00002000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_12_MASK_SHIFT            14
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_12_MASK_MASK             0x00004000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_11_MASK_SHIFT            15
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_11_MASK_MASK             0x00008000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_10_MASK_SHIFT            16
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_10_MASK_MASK             0x00010000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_9_MASK_SHIFT             17
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_9_MASK_MASK              0x00020000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_8_MASK_SHIFT             18
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_8_MASK_MASK              0x00040000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_7_MASK_SHIFT             19
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_7_MASK_MASK              0x00080000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_6_MASK_SHIFT             20
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_6_MASK_MASK              0x00100000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_5_MASK_SHIFT             21
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_5_MASK_MASK              0x00200000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_4_MASK_SHIFT             22
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_4_MASK_MASK              0x00400000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_3_MASK_SHIFT             23
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_3_MASK_MASK              0x00800000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_2_MASK_SHIFT             24
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_2_MASK_MASK              0x01000000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_1_MASK_SHIFT             25
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_1_MASK_MASK              0x02000000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_0_MASK_SHIFT             26
#define PCIE_CTRL_SYS_INT1_IRQ_INT_MASK_RADM_TRGT1_ATU_CBUF_ERR_0_MASK_MASK              0x04000000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS              0x101c
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_CORRECTABLE_ERR_STATUS_SHIFT              0
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_CORRECTABLE_ERR_STATUS_MASK               0x00000001
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_NONFATAL_ERR_STATUS_SHIFT                 1
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_NONFATAL_ERR_STATUS_MASK                  0x00000002
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_FATAL_ERR_STATUS_SHIFT                    2
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_FATAL_ERR_STATUS_MASK                     0x00000004
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_SYS_ERR_RC_0_STATUS_SHIFT                  3
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_SYS_ERR_RC_0_STATUS_MASK                   0x00000008
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_SYS_ERR_RC_1_STATUS_SHIFT                  4
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_SYS_ERR_RC_1_STATUS_MASK                   0x00000010
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_SYS_ERR_RC_2_STATUS_SHIFT                  5
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_SYS_ERR_RC_2_STATUS_MASK                   0x00000020
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_SYS_ERR_RC_3_STATUS_SHIFT                  6
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_SYS_ERR_RC_3_STATUS_MASK                   0x00000040
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_AER_RC_ERR_MSI_0_STATUS_SHIFT              7
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_AER_RC_ERR_MSI_0_STATUS_MASK               0x00000080
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_AER_RC_ERR_MSI_1_STATUS_SHIFT              8
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_AER_RC_ERR_MSI_1_STATUS_MASK               0x00000100
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_AER_RC_ERR_MSI_2_STATUS_SHIFT              9
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_AER_RC_ERR_MSI_2_STATUS_MASK               0x00000200
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_AER_RC_ERR_MSI_3_STATUS_SHIFT              10
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_CFG_AER_RC_ERR_MSI_3_STATUS_MASK               0x00000400
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_15_STATUS_SHIFT        11
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_15_STATUS_MASK         0x00000800
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_14_STATUS_SHIFT        12
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_14_STATUS_MASK         0x00001000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_13_STATUS_SHIFT        13
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_13_STATUS_MASK         0x00002000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_12_STATUS_SHIFT        14
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_12_STATUS_MASK         0x00004000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_11_STATUS_SHIFT        15
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_11_STATUS_MASK         0x00008000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_10_STATUS_SHIFT        16
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_10_STATUS_MASK         0x00010000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_9_STATUS_SHIFT         17
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_9_STATUS_MASK          0x00020000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_8_STATUS_SHIFT         18
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_8_STATUS_MASK          0x00040000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_7_STATUS_SHIFT         19
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_7_STATUS_MASK          0x00080000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_6_STATUS_SHIFT         20
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_6_STATUS_MASK          0x00100000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_5_STATUS_SHIFT         21
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_5_STATUS_MASK          0x00200000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_4_STATUS_SHIFT         22
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_4_STATUS_MASK          0x00400000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_3_STATUS_SHIFT         23
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_3_STATUS_MASK          0x00800000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_2_STATUS_SHIFT         24
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_2_STATUS_MASK          0x01000000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_1_STATUS_SHIFT         25
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_1_STATUS_MASK          0x02000000
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_0_STATUS_SHIFT         26
#define PCIE_CTRL_SYS_INT1_IRQ_INT_STATUS_RADM_TRGT1_ATU_CBUF_ERR_0_STATUS_MASK          0x04000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW                 0x1020
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_RTLH_RFC_UPD_RAW_SHIFT                            0
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_RADM_PM_PME_RAW_SHIFT                             1
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_RADM_PM_TO_ACK_RAW_SHIFT                          2
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_CFG_PME_MSI_0_RAW_SHIFT                           3
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_CFG_PME_MSI_1_RAW_SHIFT                           4
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_CFG_PME_MSI_2_RAW_SHIFT                           5
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_CFG_PME_MSI_3_RAW_SHIFT                           6
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_RADM_PM_TURNOFF_RAW_SHIFT                         7
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_RADM_SLOT_PWR_LIMIT_RAW_SHIFT                     8
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_RADM_MSG_UNLOCK_RAW_SHIFT                         9
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_RADM_VENDOR_MSG_RAW_SHIFT                         10
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_RADM_MSG_LTR_RAW_SHIFT                            11
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_SMLH_LINK_UP_INT_RAW_SHIFT                        12
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_USP_EQ_REDO_EXECUTED_INT_RAW_SHIFT                13
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_CFG_LINK_EQ_REQ_INT_RAW_SHIFT                     14
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_15_RAW_SHIFT                             15
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_14_RAW_SHIFT                             16
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_13_RAW_SHIFT                             17
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_12_RAW_SHIFT                             18
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_11_RAW_SHIFT                             19
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_10_RAW_SHIFT                             20
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_9_RAW_SHIFT                              21
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_8_RAW_SHIFT                              22
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_7_RAW_SHIFT                              23
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_6_RAW_SHIFT                              24
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_5_RAW_SHIFT                              25
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_4_RAW_SHIFT                              26
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_3_RAW_SHIFT                              27
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_2_RAW_SHIFT                              28
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_1_RAW_SHIFT                              29
#define PCIE_CTRL_SYS_INT2_IRQ_INT_RAW_EDMA_INT_0_RAW_SHIFT                              30
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE               0x1024
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_RTLH_RFC_UPD_RAW_SHIFT                          0
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_RADM_PM_PME_RAW_SHIFT                           1
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_RADM_PM_TO_ACK_RAW_SHIFT                        2
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_CFG_PME_MSI_0_RAW_SHIFT                         3
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_CFG_PME_MSI_1_RAW_SHIFT                         4
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_CFG_PME_MSI_2_RAW_SHIFT                         5
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_CFG_PME_MSI_3_RAW_SHIFT                         6
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_RADM_PM_TURNOFF_RAW_SHIFT                       7
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_RADM_SLOT_PWR_LIMIT_RAW_SHIFT                   8
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_RADM_MSG_UNLOCK_RAW_SHIFT                       9
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_RADM_VENDOR_MSG_RAW_SHIFT                       10
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_RADM_MSG_LTR_RAW_SHIFT                          11
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_SMLH_LINK_UP_INT_RAW_SHIFT                      12
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_USP_EQ_REDO_EXECUTED_INT_RAW_SHIFT              13
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_CFG_LINK_EQ_REQ_INT_RAW_SHIFT                   14
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_15_RAW_SHIFT                           15
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_14_RAW_SHIFT                           16
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_13_RAW_SHIFT                           17
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_12_RAW_SHIFT                           18
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_11_RAW_SHIFT                           19
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_10_RAW_SHIFT                           20
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_9_RAW_SHIFT                            21
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_8_RAW_SHIFT                            22
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_7_RAW_SHIFT                            23
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_6_RAW_SHIFT                            24
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_5_RAW_SHIFT                            25
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_4_RAW_SHIFT                            26
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_3_RAW_SHIFT                            27
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_2_RAW_SHIFT                            28
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_1_RAW_SHIFT                            29
#define PCIE_CTRL_SYS_INT2_IRQ_INT_FORCE_EDMA_INT_0_RAW_SHIFT                            30
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK                0x1028
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RTLH_RFC_UPD_MASK_SHIFT                          0
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RTLH_RFC_UPD_MASK_MASK                           0x00000001
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_PM_PME_MASK_SHIFT                           1
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_PM_PME_MASK_MASK                            0x00000002
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_PM_TO_ACK_MASK_SHIFT                        2
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_PM_TO_ACK_MASK_MASK                         0x00000004
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_CFG_PME_MSI_0_MASK_SHIFT                         3
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_CFG_PME_MSI_0_MASK_MASK                          0x00000008
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_CFG_PME_MSI_1_MASK_SHIFT                         4
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_CFG_PME_MSI_1_MASK_MASK                          0x00000010
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_CFG_PME_MSI_2_MASK_SHIFT                         5
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_CFG_PME_MSI_2_MASK_MASK                          0x00000020
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_CFG_PME_MSI_3_MASK_SHIFT                         6
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_CFG_PME_MSI_3_MASK_MASK                          0x00000040
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_PM_TURNOFF_MASK_SHIFT                       7
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_PM_TURNOFF_MASK_MASK                        0x00000080
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_SLOT_PWR_LIMIT_MASK_SHIFT                   8
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_SLOT_PWR_LIMIT_MASK_MASK                    0x00000100
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_MSG_UNLOCK_MASK_SHIFT                       9
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_MSG_UNLOCK_MASK_MASK                        0x00000200
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_VENDOR_MSG_MASK_SHIFT                       10
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_VENDOR_MSG_MASK_MASK                        0x00000400
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_MSG_LTR_MASK_SHIFT                          11
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_RADM_MSG_LTR_MASK_MASK                           0x00000800
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_SMLH_LINK_UP_INT_MASK_SHIFT                      12
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_SMLH_LINK_UP_INT_MASK_MASK                       0x00001000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_USP_EQ_REDO_EXECUTED_INT_MASK_SHIFT              13
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_USP_EQ_REDO_EXECUTED_INT_MASK_MASK               0x00002000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_CFG_LINK_EQ_REQ_INT_MASK_SHIFT                   14
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_CFG_LINK_EQ_REQ_INT_MASK_MASK                    0x00004000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_15_MASK_SHIFT                           15
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_15_MASK_MASK                            0x00008000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_14_MASK_SHIFT                           16
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_14_MASK_MASK                            0x00010000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_13_MASK_SHIFT                           17
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_13_MASK_MASK                            0x00020000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_12_MASK_SHIFT                           18
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_12_MASK_MASK                            0x00040000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_11_MASK_SHIFT                           19
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_11_MASK_MASK                            0x00080000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_10_MASK_SHIFT                           20
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_10_MASK_MASK                            0x00100000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_9_MASK_SHIFT                            21
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_9_MASK_MASK                             0x00200000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_8_MASK_SHIFT                            22
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_8_MASK_MASK                             0x00400000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_7_MASK_SHIFT                            23
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_7_MASK_MASK                             0x00800000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_6_MASK_SHIFT                            24
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_6_MASK_MASK                             0x01000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_5_MASK_SHIFT                            25
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_5_MASK_MASK                             0x02000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_4_MASK_SHIFT                            26
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_4_MASK_MASK                             0x04000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_3_MASK_SHIFT                            27
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_3_MASK_MASK                             0x08000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_2_MASK_SHIFT                            28
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_2_MASK_MASK                             0x10000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_1_MASK_SHIFT                            29
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_1_MASK_MASK                             0x20000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_0_MASK_SHIFT                            30
#define PCIE_CTRL_SYS_INT2_IRQ_INT_MASK_EDMA_INT_0_MASK_MASK                             0x40000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS              0x102c
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RTLH_RFC_UPD_STATUS_SHIFT                      0
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RTLH_RFC_UPD_STATUS_MASK                       0x00000001
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_PM_PME_STATUS_SHIFT                       1
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_PM_PME_STATUS_MASK                        0x00000002
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_PM_TO_ACK_STATUS_SHIFT                    2
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_PM_TO_ACK_STATUS_MASK                     0x00000004
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_CFG_PME_MSI_0_STATUS_SHIFT                     3
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_CFG_PME_MSI_0_STATUS_MASK                      0x00000008
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_CFG_PME_MSI_1_STATUS_SHIFT                     4
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_CFG_PME_MSI_1_STATUS_MASK                      0x00000010
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_CFG_PME_MSI_2_STATUS_SHIFT                     5
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_CFG_PME_MSI_2_STATUS_MASK                      0x00000020
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_CFG_PME_MSI_3_STATUS_SHIFT                     6
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_CFG_PME_MSI_3_STATUS_MASK                      0x00000040
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_PM_TURNOFF_STATUS_SHIFT                   7
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_PM_TURNOFF_STATUS_MASK                    0x00000080
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_SLOT_PWR_LIMIT_STATUS_SHIFT               8
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_SLOT_PWR_LIMIT_STATUS_MASK                0x00000100
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_MSG_UNLOCK_STATUS_SHIFT                   9
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_MSG_UNLOCK_STATUS_MASK                    0x00000200
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_VENDOR_MSG_STATUS_SHIFT                   10
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_VENDOR_MSG_STATUS_MASK                    0x00000400
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_MSG_LTR_STATUS_SHIFT                      11
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_RADM_MSG_LTR_STATUS_MASK                       0x00000800
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_SMLH_LINK_UP_INT_STATUS_SHIFT                  12
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_SMLH_LINK_UP_INT_STATUS_MASK                   0x00001000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_USP_EQ_REDO_EXECUTED_INT_STATUS_SHIFT          13
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_USP_EQ_REDO_EXECUTED_INT_STATUS_MASK           0x00002000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_CFG_LINK_EQ_REQ_INT_STATUS_SHIFT               14
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_CFG_LINK_EQ_REQ_INT_STATUS_MASK                0x00004000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_15_STATUS_SHIFT                       15
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_15_STATUS_MASK                        0x00008000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_14_STATUS_SHIFT                       16
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_14_STATUS_MASK                        0x00010000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_13_STATUS_SHIFT                       17
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_13_STATUS_MASK                        0x00020000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_12_STATUS_SHIFT                       18
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_12_STATUS_MASK                        0x00040000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_11_STATUS_SHIFT                       19
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_11_STATUS_MASK                        0x00080000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_10_STATUS_SHIFT                       20
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_10_STATUS_MASK                        0x00100000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_9_STATUS_SHIFT                        21
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_9_STATUS_MASK                         0x00200000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_8_STATUS_SHIFT                        22
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_8_STATUS_MASK                         0x00400000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_7_STATUS_SHIFT                        23
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_7_STATUS_MASK                         0x00800000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_6_STATUS_SHIFT                        24
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_6_STATUS_MASK                         0x01000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_5_STATUS_SHIFT                        25
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_5_STATUS_MASK                         0x02000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_4_STATUS_SHIFT                        26
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_4_STATUS_MASK                         0x04000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_3_STATUS_SHIFT                        27
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_3_STATUS_MASK                         0x08000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_2_STATUS_SHIFT                        28
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_2_STATUS_MASK                         0x10000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_1_STATUS_SHIFT                        29
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_1_STATUS_MASK                         0x20000000
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_0_STATUS_SHIFT                        30
#define PCIE_CTRL_SYS_INT2_IRQ_INT_STATUS_EDMA_INT_0_STATUS_MASK                         0x40000000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW                 0x1030
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_RADM_INTA_ASSERTED_RAW_SHIFT                      0
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_RADM_INTB_ASSERTED_RAW_SHIFT                      1
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_RADM_INTC_ASSERTED_RAW_SHIFT                      2
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_RADM_INTD_ASSERTED_RAW_SHIFT                      3
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_RADM_INTA_DEASSERTED_RAW_SHIFT                    4
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_RADM_INTB_DEASSERTED_RAW_SHIFT                    5
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_RADM_INTC_DEASSERTED_RAW_SHIFT                    6
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_RADM_INTD_DEASSERTED_RAW_SHIFT                    7
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_CFG_LINK_AUTO_BW_MSI_RAW_SHIFT                    8
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_CFG_LINK_AUTO_BW_INT_RAW_SHIFT                    9
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_CFG_BW_MGT_INT_RAW_SHIFT                          10
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_CFG_BW_MGT_MSI_RAW_SHIFT                          11
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_ASSERT_INTA_GRT_RAW_SHIFT                         12
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_ASSERT_INTB_GRT_RAW_SHIFT                         13
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_ASSERT_INTC_GRT_RAW_SHIFT                         14
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_ASSERT_INTD_GRT_RAW_SHIFT                         15
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_DEASSERT_INTA_GRT_RAW_SHIFT                       16
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_DEASSERT_INTB_GRT_RAW_SHIFT                       17
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_DEASSERT_INTC_GRT_RAW_SHIFT                       18
#define PCIE_CTRL_SYS_INT3_IRQ_INT_RAW_DEASSERT_INTD_GRT_RAW_SHIFT                       19
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE               0x1034
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_RADM_INTA_ASSERTED_RAW_SHIFT                    0
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_RADM_INTB_ASSERTED_RAW_SHIFT                    1
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_RADM_INTC_ASSERTED_RAW_SHIFT                    2
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_RADM_INTD_ASSERTED_RAW_SHIFT                    3
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_RADM_INTA_DEASSERTED_RAW_SHIFT                  4
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_RADM_INTB_DEASSERTED_RAW_SHIFT                  5
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_RADM_INTC_DEASSERTED_RAW_SHIFT                  6
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_RADM_INTD_DEASSERTED_RAW_SHIFT                  7
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_CFG_LINK_AUTO_BW_MSI_RAW_SHIFT                  8
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_CFG_LINK_AUTO_BW_INT_RAW_SHIFT                  9
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_CFG_BW_MGT_INT_RAW_SHIFT                        10
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_CFG_BW_MGT_MSI_RAW_SHIFT                        11
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_ASSERT_INTA_GRT_RAW_SHIFT                       12
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_ASSERT_INTB_GRT_RAW_SHIFT                       13
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_ASSERT_INTC_GRT_RAW_SHIFT                       14
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_ASSERT_INTD_GRT_RAW_SHIFT                       15
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_DEASSERT_INTA_GRT_RAW_SHIFT                     16
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_DEASSERT_INTB_GRT_RAW_SHIFT                     17
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_DEASSERT_INTC_GRT_RAW_SHIFT                     18
#define PCIE_CTRL_SYS_INT3_IRQ_INT_FORCE_DEASSERT_INTD_GRT_RAW_SHIFT                     19
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK                0x1038
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTA_ASSERTED_MASK_SHIFT                    0
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTA_ASSERTED_MASK_MASK                     0x00000001
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTB_ASSERTED_MASK_SHIFT                    1
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTB_ASSERTED_MASK_MASK                     0x00000002
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTC_ASSERTED_MASK_SHIFT                    2
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTC_ASSERTED_MASK_MASK                     0x00000004
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTD_ASSERTED_MASK_SHIFT                    3
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTD_ASSERTED_MASK_MASK                     0x00000008
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTA_DEASSERTED_MASK_SHIFT                  4
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTA_DEASSERTED_MASK_MASK                   0x00000010
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTB_DEASSERTED_MASK_SHIFT                  5
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTB_DEASSERTED_MASK_MASK                   0x00000020
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTC_DEASSERTED_MASK_SHIFT                  6
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTC_DEASSERTED_MASK_MASK                   0x00000040
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTD_DEASSERTED_MASK_SHIFT                  7
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_RADM_INTD_DEASSERTED_MASK_MASK                   0x00000080
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_CFG_LINK_AUTO_BW_MSI_MASK_SHIFT                  8
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_CFG_LINK_AUTO_BW_MSI_MASK_MASK                   0x00000100
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_CFG_LINK_AUTO_BW_INT_MASK_SHIFT                  9
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_CFG_LINK_AUTO_BW_INT_MASK_MASK                   0x00000200
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_CFG_BW_MGT_INT_MASK_SHIFT                        10
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_CFG_BW_MGT_INT_MASK_MASK                         0x00000400
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_CFG_BW_MGT_MSI_MASK_SHIFT                        11
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_CFG_BW_MGT_MSI_MASK_MASK                         0x00000800
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_ASSERT_INTA_GRT_MASK_SHIFT                       12
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_ASSERT_INTA_GRT_MASK_MASK                        0x00001000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_ASSERT_INTB_GRT_MASK_SHIFT                       13
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_ASSERT_INTB_GRT_MASK_MASK                        0x00002000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_ASSERT_INTC_GRT_MASK_SHIFT                       14
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_ASSERT_INTC_GRT_MASK_MASK                        0x00004000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_ASSERT_INTD_GRT_MASK_SHIFT                       15
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_ASSERT_INTD_GRT_MASK_MASK                        0x00008000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_DEASSERT_INTA_GRT_MASK_SHIFT                     16
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_DEASSERT_INTA_GRT_MASK_MASK                      0x00010000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_DEASSERT_INTB_GRT_MASK_SHIFT                     17
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_DEASSERT_INTB_GRT_MASK_MASK                      0x00020000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_DEASSERT_INTC_GRT_MASK_SHIFT                     18
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_DEASSERT_INTC_GRT_MASK_MASK                      0x00040000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_DEASSERT_INTD_GRT_MASK_SHIFT                     19
#define PCIE_CTRL_SYS_INT3_IRQ_INT_MASK_DEASSERT_INTD_GRT_MASK_MASK                      0x00080000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS              0x103c
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTA_ASSERTED_STATUS_SHIFT                0
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTA_ASSERTED_STATUS_MASK                 0x00000001
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTB_ASSERTED_STATUS_SHIFT                1
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTB_ASSERTED_STATUS_MASK                 0x00000002
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTC_ASSERTED_STATUS_SHIFT                2
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTC_ASSERTED_STATUS_MASK                 0x00000004
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTD_ASSERTED_STATUS_SHIFT                3
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTD_ASSERTED_STATUS_MASK                 0x00000008
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTA_DEASSERTED_STATUS_SHIFT              4
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTA_DEASSERTED_STATUS_MASK               0x00000010
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTB_DEASSERTED_STATUS_SHIFT              5
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTB_DEASSERTED_STATUS_MASK               0x00000020
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTC_DEASSERTED_STATUS_SHIFT              6
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTC_DEASSERTED_STATUS_MASK               0x00000040
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTD_DEASSERTED_STATUS_SHIFT              7
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_RADM_INTD_DEASSERTED_STATUS_MASK               0x00000080
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_CFG_LINK_AUTO_BW_MSI_STATUS_SHIFT              8
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_CFG_LINK_AUTO_BW_MSI_STATUS_MASK               0x00000100
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_CFG_LINK_AUTO_BW_INT_STATUS_SHIFT              9
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_CFG_LINK_AUTO_BW_INT_STATUS_MASK               0x00000200
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_CFG_BW_MGT_INT_STATUS_SHIFT                    10
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_CFG_BW_MGT_INT_STATUS_MASK                     0x00000400
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_CFG_BW_MGT_MSI_STATUS_SHIFT                    11
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_CFG_BW_MGT_MSI_STATUS_MASK                     0x00000800
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_ASSERT_INTA_GRT_STATUS_SHIFT                   12
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_ASSERT_INTA_GRT_STATUS_MASK                    0x00001000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_ASSERT_INTB_GRT_STATUS_SHIFT                   13
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_ASSERT_INTB_GRT_STATUS_MASK                    0x00002000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_ASSERT_INTC_GRT_STATUS_SHIFT                   14
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_ASSERT_INTC_GRT_STATUS_MASK                    0x00004000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_ASSERT_INTD_GRT_STATUS_SHIFT                   15
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_ASSERT_INTD_GRT_STATUS_MASK                    0x00008000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_DEASSERT_INTA_GRT_STATUS_SHIFT                 16
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_DEASSERT_INTA_GRT_STATUS_MASK                  0x00010000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_DEASSERT_INTB_GRT_STATUS_SHIFT                 17
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_DEASSERT_INTB_GRT_STATUS_MASK                  0x00020000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_DEASSERT_INTC_GRT_STATUS_SHIFT                 18
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_DEASSERT_INTC_GRT_STATUS_MASK                  0x00040000
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_DEASSERT_INTD_GRT_STATUS_SHIFT                 19
#define PCIE_CTRL_SYS_INT3_IRQ_INT_STATUS_DEASSERT_INTD_GRT_STATUS_MASK                  0x00080000
#define PCIE_CTRL_REGFILE_CGBYPASS                     0x1040
#define PCIE_CTRL_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                                0
#define PCIE_CTRL_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                                 0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_timeout_cpl_tag  : 10;
        unsigned int radm_timeout_cpl_len  : 12;
        unsigned int radm_timeout_cpl_attr :  2;
        unsigned int radm_timeout_cpl_tc   :  3;
        unsigned int radm_timeout_func_num :  3;
        unsigned int reserved_0            :  2;
    } reg;
}pcie_ctrl_cpl_timeout_info_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ven_msg_type :  5;
        unsigned int ven_msg_fmt  :  2;
        unsigned int ven_msg_tc   :  3;
        unsigned int ven_msg_attr :  2;
        unsigned int ven_msg_ep   :  1;
        unsigned int ven_msg_td   :  1;
        unsigned int ven_msg_len  : 10;
        unsigned int reserved_0   :  8;
    } reg;
}pcie_ctrl_vmi_signals_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ven_msg_code     :  8;
        unsigned int ven_msg_tag      : 10;
        unsigned int ven_msg_func_num :  3;
        unsigned int reserved_0       : 11;
    } reg;
}pcie_ctrl_vmi_signals_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ven_msg_data_h : 32;
    } reg;
}pcie_ctrl_vmi_signals_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ven_msg_data_l : 32;
    } reg;
}pcie_ctrl_vmi_signals_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_ven_msg_req :  1;
        unsigned int reserved_0      : 31;
    } reg;
}pcie_ctrl_cfg_ven_msg_req_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_ltssm_enable :  1;
        unsigned int app_sris_mode    :  1;
        unsigned int tx_lane_flip_en  :  1;
        unsigned int rx_lane_flip_en  :  1;
        unsigned int reserved_0       : 28;
    } reg;
}pcie_ctrl_sii_general_core_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_l1sub_disable  :  1;
        unsigned int app_clk_pm_en      :  1;
        unsigned int app_xfer_pending   :  1;
        unsigned int app_req_exit_l1    :  1;
        unsigned int app_ready_entr_l23 :  1;
        unsigned int app_req_entr_l1    :  1;
        unsigned int apps_pm_xmt_pme    :  4;
        unsigned int reserved_0         : 22;
    } reg;
}pcie_ctrl_sii_power_management_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_dstate  : 12;
        unsigned int pm_pme_en  :  4;
        unsigned int reserved_0 : 16;
    } reg;
}pcie_ctrl_sii_power_management_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_l1_entry_started :  1;
        unsigned int pm_l1sub_state      :  3;
        unsigned int pm_slave_state      :  5;
        unsigned int pm_master_state     :  5;
        unsigned int pm_linkst_l2_exit   :  1;
        unsigned int pm_linkst_in_l2     :  1;
        unsigned int pm_linkst_in_l1sub  :  1;
        unsigned int pm_linkst_in_l1     :  1;
        unsigned int pm_linkst_in_l0s    :  1;
        unsigned int pm_curnt_state      :  3;
        unsigned int pm_status           :  4;
        unsigned int reserved_0          :  6;
    } reg;
}pcie_ctrl_sii_power_management_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_l1sub_en              :  1;
        unsigned int pm_aspm_l1_enter_ready    :  1;
        unsigned int aux_pm_en                 :  4;
        unsigned int smlh_ltssm_state_rcvry_eq :  1;
        unsigned int reserved_0                : 25;
    } reg;
}pcie_ctrl_sii_power_management_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_xtlh_block_tlp :  1;
        unsigned int reserved_0        : 31;
    } reg;
}pcie_ctrl_sii_transmit_ctrl_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_hdr_log_0 : 32;
    } reg;
}pcie_ctrl_sii_transmit_ctrl_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_hdr_log_1 : 32;
    } reg;
}pcie_ctrl_sii_transmit_ctrl_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_hdr_log_2 : 32;
    } reg;
}pcie_ctrl_sii_transmit_ctrl_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_hdr_log_3 : 32;
    } reg;
}pcie_ctrl_sii_transmit_ctrl_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_err_bus : 27;
        unsigned int reserved_0  :  5;
    } reg;
}pcie_ctrl_sii_transmit_ctrl_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_err_func_num      :  3;
        unsigned int app_poisoned_tlp_type :  1;
        unsigned int app_err_advisory      :  1;
        unsigned int reserved_0            : 27;
    } reg;
}pcie_ctrl_sii_transmit_ctrl_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_hdr_valid :  1;
        unsigned int reserved_0    : 31;
    } reg;
}pcie_ctrl_sii_transmit_ctrl_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_start_h_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_start_l_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_start_h_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_start_l_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_start_h_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_start_l_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_start_h_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_start_l_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_limit_h_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_limit_l_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_limit_h_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_limit_l_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_limit_h_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_12_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_limit_l_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_13_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_limit_h_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_14_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar0_limit_l_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_15_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar1_start_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_16_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar1_start_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_17_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar1_start_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_18_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar1_start_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_19_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar1_limit_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_20_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar1_limit_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_21_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar1_limit_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_22_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar1_limit_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_23_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_start_h_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_24_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_start_l_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_25_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_start_h_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_26_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_start_l_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_27_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_start_h_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_28_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_start_l_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_29_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_start_h_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_30_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_start_l_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_31_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_limit_h_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_32_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_limit_l_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_33_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_limit_h_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_34_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_limit_l_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_35_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_limit_h_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_36_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_limit_l_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_37_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_limit_h_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_38_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar2_limit_l_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_39_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar3_start_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_40_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar3_start_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_41_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar3_start_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_42_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar3_start_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_43_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar3_limit_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_44_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar3_limit_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_45_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar3_limit_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_46_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar3_limit_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_47_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_start_h_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_48_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_start_l_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_49_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_start_h_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_50_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_start_l_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_51_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_start_h_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_52_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_start_l_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_53_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_start_h_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_54_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_start_l_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_55_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_limit_h_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_56_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_limit_l_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_57_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_limit_h_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_58_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_limit_l_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_59_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_limit_h_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_60_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_limit_l_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_61_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_limit_h_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_62_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar4_limit_l_fucn3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_63_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar5_start_fun0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_64_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar5_start_fun1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_65_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar5_start_fun2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_66_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar5_start_fun3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_67_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar5_limit_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_68_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar5_limit_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_69_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar5_limit_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_70_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_bar5_limit_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_71_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_exp_rom_start_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_72_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_exp_rom_start_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_73_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_exp_rom_start_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_74_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_exp_rom_start_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_75_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_exp_rom_limit_func0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_76_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_exp_rom_limit_func1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_77_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_exp_rom_limit_func2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_78_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_exp_rom_limit_func3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_79_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_ltr_max_latency_0 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_80_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_ltr_max_latency_1 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_81_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_ltr_max_latency_2 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_82_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_ltr_max_latency_3 : 32;
    } reg;
}pcie_ctrl_sii_configuration_info_83_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_max_rd_req_size : 12;
        unsigned int reserved_0          : 20;
    } reg;
}pcie_ctrl_sii_configuration_info_84_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_pbus_num         :  8;
        unsigned int cfg_pm_no_soft_rst   :  4;
        unsigned int cfg_ext_tag_en       :  4;
        unsigned int cfg_max_payload_size : 12;
        unsigned int cfg_mem_space_en     :  4;
    } reg;
}pcie_ctrl_sii_configuration_info_85_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_hp_int_en           :  4;
        unsigned int cfg_cmd_cpled_int_en    :  4;
        unsigned int cfg_dll_state_chged_en  :  4;
        unsigned int cfg_hp_slot_ctrl_access :  4;
        unsigned int cfg_relax_order_en      :  4;
        unsigned int cfg_no_snoop_en         :  4;
        unsigned int cfg_pbus_dev_num        :  5;
        unsigned int reserved_0              :  3;
    } reg;
}pcie_ctrl_sii_configuration_info_86_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_neg_link_width          :  6;
        unsigned int cfg_atten_button_pressed_en :  4;
        unsigned int cfg_pwr_fault_det_en        :  4;
        unsigned int cfg_mrl_sensor_chged_en     :  4;
        unsigned int cfg_rcb                     :  4;
        unsigned int cfg_pre_det_chged_en        :  4;
        unsigned int cfg_bus_master_en           :  4;
        unsigned int cfg_disable_ltr_clr_msg     :  1;
        unsigned int cfg_ltr_m_en                :  1;
    } reg;
}pcie_ctrl_sii_configuration_info_87_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_dbi_ro_wr_disable :  1;
        unsigned int app_pf_req_retry_en   :  4;
        unsigned int app_req_retry_en      :  1;
        unsigned int reserved_0            : 26;
    } reg;
}pcie_ctrl_sii_configuration_info_88_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_q_not_empty      :  1;
        unsigned int radm_xfer_pending     :  1;
        unsigned int edma_xfer_pending     :  1;
        unsigned int brdg_slv_xfer_pending :  1;
        unsigned int brdg_dbi_xfer_pending :  1;
        unsigned int rdlh_link_up          :  1;
        unsigned int reserved_0            : 26;
    } reg;
}pcie_ctrl_sii_debug_info_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_link_up :  1;
        unsigned int reserved_0   : 31;
    } reg;
}pcie_ctrl_sii_link_rst_or_statu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_mac_phystatus :  1;
        unsigned int reserved_0        : 31;
    } reg;
}pcie_ctrl_phy_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_msg_payload_l : 32;
    } reg;
}pcie_ctrl_sii_message_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_msg_payload_h : 32;
    } reg;
}pcie_ctrl_sii_message_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_slot_pwr_payload : 32;
    } reg;
}pcie_ctrl_sii_message_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_msg_req_id : 16;
        unsigned int reserved_0      : 16;
    } reg;
}pcie_ctrl_sii_message_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_ltr_msg_latency : 32;
    } reg;
}pcie_ctrl_sii_ltr_message_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_ltr_req          :  1;
        unsigned int app_ltr_msg_func_num :  3;
        unsigned int reserved_0           : 28;
    } reg;
}pcie_ctrl_sii_ltr_message_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_ltr_latency : 32;
    } reg;
}pcie_ctrl_sii_ltr_message_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int trgt_timeout_lookup_id    : 10;
        unsigned int trgt_timeout_cpl_len      : 12;
        unsigned int trgt_timeout_cpl_attr     :  2;
        unsigned int trgt_timeout_cpl_tc       :  3;
        unsigned int trgt_timeout_cpl_func_num :  3;
        unsigned int reserved_0                :  2;
    } reg;
}pcie_ctrl_traget_completion_lut_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int trgt_lookup_empty :  1;
        unsigned int trgt_lookup_id    : 10;
        unsigned int reserved_0        : 21;
    } reg;
}pcie_ctrl_traget_completion_lut_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_margining_software_ready :  1;
        unsigned int app_margining_ready          :  1;
        unsigned int reserved_0                   : 30;
    } reg;
}pcie_ctrl_gen4_margin_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_init_rst :  1;
        unsigned int reserved_0   : 31;
    } reg;
}pcie_ctrl_hot_reset_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_clk_req_n :  1;
        unsigned int reserved_0    : 31;
    } reg;
}pcie_ctrl_app_clk_req_n_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_f_err_rpt_en   :  4;
        unsigned int cfg_nf_err_rpt_en  :  4;
        unsigned int cfg_cor_err_rpt_en :  4;
        unsigned int cfg_reg_serren     :  4;
        unsigned int reserved_0         : 16;
    } reg;
}pcie_ctrl_extsrivo_pf_reg_value_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_ctrl_bus :  3;
        unsigned int reserved_0    : 29;
    } reg;
}pcie_ctrl_diagnostic_sigs_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_ras_des_tba_ctrl :  1;
        unsigned int reserved_0           : 31;
    } reg;
}pcie_ctrl_ras_des_tba_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_ras_des_sd_hold_ltssm :  1;
        unsigned int reserved_0                : 31;
    } reg;
}pcie_ctrl_ras_des_sili_dbg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int exp_rom_validation_details_strobe :  4;
        unsigned int exp_rom_validation_details        : 16;
        unsigned int reserved_0                        : 12;
    } reg;
}pcie_ctrl_sii_rom_validation_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int exp_rom_validation_status_strobe :  4;
        unsigned int exp_rom_validation_status        : 12;
        unsigned int reserved_0                       : 16;
    } reg;
}pcie_ctrl_sii_rom_validation_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_int_pin : 32;
    } reg;
}pcie_ctrl_sii_interrupt_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_int_disable :  4;
        unsigned int reserved_0      : 28;
    } reg;
}pcie_ctrl_sii_interrupt_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_hold_phy_rst :  1;
        unsigned int reserved_0       : 31;
    } reg;
}pcie_ctrl_phy_register_access_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_current_data_rate         :  3;
        unsigned int mac_phy_rxelecidle_disable   :  1;
        unsigned int mac_phy_txcommonmode_disable :  1;
        unsigned int reserved_0                   : 27;
    } reg;
}pcie_ctrl_pipe_sigs_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_rcvd_cfgwr_poisoned_reg_num     :  6;
        unsigned int radm_rcvd_cfgwr_poisoned_ext_reg_num :  4;
        unsigned int radm_rcvd_cfgwr_poisoned_func_num    :  3;
        unsigned int reserved_0                           : 19;
    } reg;
}pcie_ctrl_sii_detect_poisoned_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_armisc_info_dma :  6;
        unsigned int mstr_awmisc_info_dma :  6;
        unsigned int reserved_0           : 20;
    } reg;
}pcie_ctrl_axi_misc_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_awmisc_info_l : 32;
    } reg;
}pcie_ctrl_axi_misc_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_awmisc_info_h : 18;
        unsigned int reserved_0         : 14;
    } reg;
}pcie_ctrl_axi_misc_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_awmisc_info_hdr_34dw_l : 32;
    } reg;
}pcie_ctrl_axi_misc_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_awmisc_info_hdr_34dw_h : 32;
    } reg;
}pcie_ctrl_axi_misc_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_armisc_info_l : 32;
    } reg;
}pcie_ctrl_axi_misc_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_armisc_info_h : 18;
        unsigned int reserved_0         : 14;
    } reg;
}pcie_ctrl_axi_misc_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_awmisc_info_ep            :  1;
        unsigned int mstr_awmisc_info_last_dcmp_tlp :  1;
        unsigned int mstr_armisc_info_zeroread      :  1;
        unsigned int mstr_armisc_info_last_dcmp_tlp :  1;
        unsigned int reserved_0                     : 28;
    } reg;
}pcie_ctrl_axi_misc_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int slv_rmisc_info : 14;
        unsigned int slv_bmisc_info : 14;
        unsigned int reserved_0     :  4;
    } reg;
}pcie_ctrl_axi_misc_8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_rmisc_info_cpl_stat :  3;
        unsigned int mstr_bmisc_info_cpl_stat :  3;
        unsigned int reserved_0               : 26;
    } reg;
}pcie_ctrl_axi_misc_9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int slv_awmisc_info : 25;
        unsigned int reserved_0      :  7;
    } reg;
}pcie_ctrl_axi_misc_10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int slv_awmisc_info_hdr_34dw_l : 32;
    } reg;
}pcie_ctrl_axi_misc_11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int slv_awmisc_info_hdr_34dw_h : 32;
    } reg;
}pcie_ctrl_axi_misc_12_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int slv_awmisc_info_atu_bypass :  1;
        unsigned int slv_awmisc_info_p_tag      : 10;
        unsigned int reserved_0                 : 21;
    } reg;
}pcie_ctrl_axi_misc_13_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int slv_armisc_info_atu_bypass :  1;
        unsigned int slv_armisc_info            : 25;
        unsigned int reserved_0                 :  6;
    } reg;
}pcie_ctrl_axi_misc_14_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int slv_wmisc_info_silentdrop :  1;
        unsigned int slv_wmisc_info_ep         :  1;
        unsigned int mstr_rmisc_info           : 13;
        unsigned int reserved_0                : 17;
    } reg;
}pcie_ctrl_axi_misc_15_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_trgt1_atu_sloc_match : 16;
        unsigned int reserved_0                : 16;
    } reg;
}pcie_ctrl_rtrgt1_intf_sigs_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw0 : 32;
    } reg;
}pcie_ctrl_diag_statu_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw1 : 32;
    } reg;
}pcie_ctrl_diag_statu_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw2 : 32;
    } reg;
}pcie_ctrl_diag_statu_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw3 : 32;
    } reg;
}pcie_ctrl_diag_statu_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw4 : 32;
    } reg;
}pcie_ctrl_diag_statu_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw5 : 32;
    } reg;
}pcie_ctrl_diag_statu_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw6 : 32;
    } reg;
}pcie_ctrl_diag_statu_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw7 : 32;
    } reg;
}pcie_ctrl_diag_statu_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw8 : 32;
    } reg;
}pcie_ctrl_diag_statu_8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw9 : 32;
    } reg;
}pcie_ctrl_diag_statu_9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw10 : 32;
    } reg;
}pcie_ctrl_diag_statu_10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw11 : 32;
    } reg;
}pcie_ctrl_diag_statu_11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw12 : 32;
    } reg;
}pcie_ctrl_diag_statu_12_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw13 : 32;
    } reg;
}pcie_ctrl_diag_statu_13_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw14 : 32;
    } reg;
}pcie_ctrl_diag_statu_14_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw15 : 32;
    } reg;
}pcie_ctrl_diag_statu_15_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw16 : 32;
    } reg;
}pcie_ctrl_diag_statu_16_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw17 : 32;
    } reg;
}pcie_ctrl_diag_statu_17_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw18 : 32;
    } reg;
}pcie_ctrl_diag_statu_18_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw19 : 32;
    } reg;
}pcie_ctrl_diag_statu_19_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int diag_statu_dw20 : 24;
        unsigned int reserved_0      :  8;
    } reg;
}pcie_ctrl_diag_statu_20_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cxpl_debug_info_l : 32;
    } reg;
}pcie_ctrl_cxpl_debug_info_l_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cxpl_debug_info_h : 32;
    } reg;
}pcie_ctrl_cxpl_debug_info_h_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cxpl_debug_info_ei : 16;
        unsigned int reserved_0         : 16;
    } reg;
}pcie_ctrl_cxpl_debug_info_ei_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pcie_testpin_group_sel :  8;
        unsigned int reserved_0             : 24;
    } reg;
}pcie_ctrl_pcie_testpin_group_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ltssm_sw_ctrl_en :  1;
        unsigned int reserved_0       : 31;
    } reg;
}pcie_ctrl_ltssm_sw_ctrl_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ram_dslp   :  1;
        unsigned int reserved_0 : 15;
        unsigned int _bm_       : 16;
    } reg;
}pcie_ctrl_sram_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_intx_enable :  1;
        unsigned int reserved_0      : 31;
    } reg;
}pcie_ctrl_app_intx_enable_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_clkrst_sync_perst_n_sel :  1;
        unsigned int reserved_0                  : 31;
    } reg;
}pcie_ctrl_app_clkrst_sync_perst_n_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_clkrst_sync_perst_n :  1;
        unsigned int reserved_0              : 31;
    } reg;
}pcie_ctrl_app_clkrst_sync_perst_n_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_dev_num :  5;
        unsigned int app_bus_num :  8;
        unsigned int reserved_0  : 19;
    } reg;
}pcie_ctrl_device_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dbi_io_access  :  1;
        unsigned int dbi_rom_access :  1;
        unsigned int dbi_bar_num    :  3;
        unsigned int dbi_func_num   :  3;
        unsigned int reserved_0     : 24;
    } reg;
}pcie_ctrl_dbi_access_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_unlock_msg :  1;
        unsigned int reserved_0     : 31;
    } reg;
}pcie_ctrl_pm_unlock_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int apps_pm_xmt_turnoff :  1;
        unsigned int reserved_0          : 31;
    } reg;
}pcie_ctrl_l2_entry_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_pwr_ctrler_ctrl :  4;
        unsigned int cfg_atten_ind       :  8;
        unsigned int cfg_pwr_ind         :  8;
        unsigned int reserved_0          : 12;
    } reg;
}pcie_ctrl_power_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_2ndbus_num : 32;
    } reg;
}pcie_ctrl_cfg_2ndbus_num_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_subbus_num : 32;
    } reg;
}pcie_ctrl_cfg_subbus_num_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_2nd_reset :  1;
        unsigned int reserved_0    : 31;
    } reg;
}pcie_ctrl_cfg_2nd_reset_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int msi_ctrl_int_vec :  8;
        unsigned int reserved_0       : 24;
    } reg;
}pcie_ctrl_msi_ctrl_int_vec_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_aer_int_msg_num : 20;
        unsigned int reserved_0          : 12;
    } reg;
}pcie_ctrl_cfg_aer_int_msg_num_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_pcie_cap_int_msg_num : 20;
        unsigned int reserved_0               : 12;
    } reg;
}pcie_ctrl_cfg_pcie_cap_int_msg_num_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_br_ctrl_serren :  4;
        unsigned int reserved_0         : 28;
    } reg;
}pcie_ctrl_cfg_br_ctrl_serren_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_crs_sw_vis_en :  4;
        unsigned int reserved_0        : 28;
    } reg;
}pcie_ctrl_cfg_crs_sw_vis_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rtlh_rfc_data : 32;
    } reg;
}pcie_ctrl_rtlh_rfc_data_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_eml_control :  4;
        unsigned int reserved_0      : 28;
    } reg;
}pcie_ctrl_cfg_eml_control_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dbg_table  :  1;
        unsigned int dbg_pba    :  1;
        unsigned int reserved_0 : 30;
    } reg;
}pcie_ctrl_dbg_msix_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int perst_n_src_sel  :  1;
        unsigned int perst_n_src_over :  1;
        unsigned int reserved_0       : 30;
    } reg;
}pcie_ctrl_perst_n_override_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int wake_src_sel  :  1;
        unsigned int wake_src_over :  1;
        unsigned int reserved_0    : 30;
    } reg;
}pcie_ctrl_wake_override_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ep_to_rc_int0 :  1;
        unsigned int ep_to_rc_int1 :  1;
        unsigned int ep_to_rc_int2 :  1;
        unsigned int ep_to_rc_int3 :  1;
        unsigned int ep_to_rc_int4 :  1;
        unsigned int ep_to_rc_int5 :  1;
        unsigned int ep_to_rc_int6 :  1;
        unsigned int ep_to_rc_int7 :  1;
        unsigned int reserved_0    : 24;
    } reg;
}pcie_ctrl_ep_to_rc_int_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state :  6;
        unsigned int reserved_0       : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_en :  1;
        unsigned int reserved_0          : 31;
    } reg;
}pcie_ctrl_smlh_ltssm_state_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_1d :  6;
        unsigned int reserved_0          : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_1d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_2d :  6;
        unsigned int reserved_0          : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_2d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_3d :  6;
        unsigned int reserved_0          : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_3d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_4d :  6;
        unsigned int reserved_0          : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_4d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_5d :  6;
        unsigned int reserved_0          : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_5d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_6d :  6;
        unsigned int reserved_0          : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_6d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_7d :  6;
        unsigned int reserved_0          : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_7d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_8d :  6;
        unsigned int reserved_0          : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_8d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_9d :  6;
        unsigned int reserved_0          : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_9d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_10d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_10d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_11d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_11d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_12d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_12d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_13d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_13d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_14d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_14d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_15d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_15d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_16d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_16d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_17d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_17d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_18d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_18d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_19d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_19d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_20d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_20d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_21d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_21d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_22d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_22d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_23d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_23d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_24d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_24d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_25d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_25d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_26d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_26d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_27d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_27d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_28d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_28d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_29d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_29d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_30d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_30d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_31d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_31d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int smlh_ltssm_state_32d :  6;
        unsigned int reserved_0           : 26;
    } reg;
}pcie_ctrl_smlh_ltssm_state_32d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_l1sub_state_en :  1;
        unsigned int reserved_0        : 31;
    } reg;
}pcie_ctrl_pm_l1sub_state_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_l1sub_state_1d :  3;
        unsigned int reserved_0        : 29;
    } reg;
}pcie_ctrl_pm_l1sub_state_1d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_l1sub_state_2d :  3;
        unsigned int reserved_0        : 29;
    } reg;
}pcie_ctrl_pm_l1sub_state_2d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_l1sub_state_3d :  3;
        unsigned int reserved_0        : 29;
    } reg;
}pcie_ctrl_pm_l1sub_state_3d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_l1sub_state_4d :  3;
        unsigned int reserved_0        : 29;
    } reg;
}pcie_ctrl_pm_l1sub_state_4d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_l1sub_state_5d :  3;
        unsigned int reserved_0        : 29;
    } reg;
}pcie_ctrl_pm_l1sub_state_5d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_l1sub_state_6d :  3;
        unsigned int reserved_0        : 29;
    } reg;
}pcie_ctrl_pm_l1sub_state_6d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_l1sub_state_7d :  3;
        unsigned int reserved_0        : 29;
    } reg;
}pcie_ctrl_pm_l1sub_state_7d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_l1sub_state_8d :  3;
        unsigned int reserved_0        : 29;
    } reg;
}pcie_ctrl_pm_l1sub_state_8d_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pcie_l1ss_state_ovrd :  1;
        unsigned int reserved_0           : 31;
    } reg;
}pcie_ctrl_pcie_l1ss_state_ovrd_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_awqos_sel :  1;
        unsigned int reserved_0     : 31;
    } reg;
}pcie_ctrl_mstr_awqos_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_awqos_val :  4;
        unsigned int reserved_0     : 28;
    } reg;
}pcie_ctrl_mstr_awqos_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_arqos_sel :  1;
        unsigned int reserved_0     : 31;
    } reg;
}pcie_ctrl_mstr_arqos_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mstr_arqos_val :  4;
        unsigned int reserved_0     : 28;
    } reg;
}pcie_ctrl_mstr_arqos_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mask_l11_doze :  1;
        unsigned int reserved_0    : 31;
    } reg;
}pcie_ctrl_mask_l11_doze_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mask_l12_doze :  1;
        unsigned int reserved_0    : 31;
    } reg;
}pcie_ctrl_mask_l12_doze_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mask_l2_doze :  1;
        unsigned int reserved_0   : 31;
    } reg;
}pcie_ctrl_mask_l2_doze_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int outband_pwrup_cmd :  4;
        unsigned int reserved_0        : 28;
    } reg;
}pcie_ctrl_outband_pwrup_cmd_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_aux_pwr_det :  1;
        unsigned int reserved_0      : 31;
    } reg;
}pcie_ctrl_sys_aux_pwr_det_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_atten_button_pressed :  4;
        unsigned int reserved_0               : 28;
    } reg;
}pcie_ctrl_sys_atten_button_pressed_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_pre_det_state :  4;
        unsigned int reserved_0        : 28;
    } reg;
}pcie_ctrl_sys_pre_det_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_mrl_sensor_state :  4;
        unsigned int reserved_0           : 28;
    } reg;
}pcie_ctrl_sys_mrl_sensor_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_pwr_fault_det :  4;
        unsigned int reserved_0        : 28;
    } reg;
}pcie_ctrl_sys_pwr_fault_det_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_mrl_sensor_chged :  4;
        unsigned int reserved_0           : 28;
    } reg;
}pcie_ctrl_sys_mrl_sensor_chged_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_pre_det_chged :  4;
        unsigned int reserved_0        : 28;
    } reg;
}pcie_ctrl_sys_pre_det_chged_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_cmd_cpled_int :  4;
        unsigned int reserved_0        : 28;
    } reg;
}pcie_ctrl_sys_cmd_cpled_int_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_eml_interlock_engaged :  4;
        unsigned int reserved_0                : 28;
    } reg;
}pcie_ctrl_sys_eml_interlock_engaged_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int device_type :  4;
        unsigned int reserved_0  : 28;
    } reg;
}pcie_ctrl_device_type_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_addr_0 : 32;
    } reg;
}pcie_ctrl_cfg_msi_addr_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_addr_1 : 32;
    } reg;
}pcie_ctrl_cfg_msi_addr_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_addr_2 : 32;
    } reg;
}pcie_ctrl_cfg_msi_addr_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_addr_3 : 32;
    } reg;
}pcie_ctrl_cfg_msi_addr_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_addr_4 : 32;
    } reg;
}pcie_ctrl_cfg_msi_addr_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_addr_5 : 32;
    } reg;
}pcie_ctrl_cfg_msi_addr_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_addr_6 : 32;
    } reg;
}pcie_ctrl_cfg_msi_addr_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_addr_7 : 32;
    } reg;
}pcie_ctrl_cfg_msi_addr_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_data_0 : 32;
    } reg;
}pcie_ctrl_cfg_msi_data_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_data_1 : 32;
    } reg;
}pcie_ctrl_cfg_msi_data_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_data_2 : 32;
    } reg;
}pcie_ctrl_cfg_msi_data_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_data_3 : 32;
    } reg;
}pcie_ctrl_cfg_msi_data_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_64 :  4;
        unsigned int reserved_0 : 28;
    } reg;
}pcie_ctrl_cfg_msi_64_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_multi_msi_en : 12;
        unsigned int reserved_0       : 20;
    } reg;
}pcie_ctrl_cfg_multi_msi_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_ext_data_en :  4;
        unsigned int reserved_0          : 28;
    } reg;
}pcie_ctrl_cfg_msi_ext_data_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_msi_en :  4;
        unsigned int reserved_0 : 28;
    } reg;
}pcie_ctrl_cfg_msi_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ven_msi_req :  1;
        unsigned int reserved_0  : 31;
    } reg;
}pcie_ctrl_ven_msi_req_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ven_msi_func_num :  3;
        unsigned int reserved_0       : 29;
    } reg;
}pcie_ctrl_ven_msi_func_num_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ven_msi_tc :  3;
        unsigned int reserved_0 : 29;
    } reg;
}pcie_ctrl_ven_msi_tc_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ven_msi_vector :  5;
        unsigned int reserved_0     : 27;
    } reg;
}pcie_ctrl_ven_msi_vector_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ven_msi_grant :  1;
        unsigned int reserved_0    : 31;
    } reg;
}pcie_ctrl_ven_msi_grant_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys_int    :  4;
        unsigned int reserved_0 : 28;
    } reg;
}pcie_ctrl_sys_int_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int msi_ctrl_io : 32;
    } reg;
}pcie_ctrl_msi_ctrl_io_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_trgt1_vc :  3;
        unsigned int reserved_0    : 29;
    } reg;
}pcie_ctrl_radm_trgt1_vc_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_hw_auto_sp_dis :  1;
        unsigned int reserved_0         : 31;
    } reg;
}pcie_ctrl_cfg_hw_auto_sp_dis_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ext_lbc_din_0 : 32;
    } reg;
}pcie_ctrl_ext_lbc_din_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ext_lbc_din_1 : 32;
    } reg;
}pcie_ctrl_ext_lbc_din_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ext_lbc_din_2 : 32;
    } reg;
}pcie_ctrl_ext_lbc_din_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ext_lbc_din_3 : 32;
    } reg;
}pcie_ctrl_ext_lbc_din_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_idle  :  1;
        unsigned int reserved_0 : 31;
    } reg;
}pcie_ctrl_radm_idle_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int acpu_msi_route_0 : 32;
    } reg;
}pcie_ctrl_acpu_msi_route_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int acpu_msi_route_1 : 32;
    } reg;
}pcie_ctrl_acpu_msi_route_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int acpu_msi_route_2 : 32;
    } reg;
}pcie_ctrl_acpu_msi_route_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int acpu_msi_route_3 : 32;
    } reg;
}pcie_ctrl_acpu_msi_route_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int acpu_msi_route_4 : 32;
    } reg;
}pcie_ctrl_acpu_msi_route_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int acpu_msi_route_5 : 32;
    } reg;
}pcie_ctrl_acpu_msi_route_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int acpu_msi_route_6 : 32;
    } reg;
}pcie_ctrl_acpu_msi_route_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int acpu_msi_route_7 : 32;
    } reg;
}pcie_ctrl_acpu_msi_route_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m3_msi_route_0 : 32;
    } reg;
}pcie_ctrl_m3_msi_route_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m3_msi_route_1 : 32;
    } reg;
}pcie_ctrl_m3_msi_route_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m3_msi_route_2 : 32;
    } reg;
}pcie_ctrl_m3_msi_route_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m3_msi_route_3 : 32;
    } reg;
}pcie_ctrl_m3_msi_route_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m3_msi_route_4 : 32;
    } reg;
}pcie_ctrl_m3_msi_route_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m3_msi_route_5 : 32;
    } reg;
}pcie_ctrl_m3_msi_route_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m3_msi_route_6 : 32;
    } reg;
}pcie_ctrl_m3_msi_route_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int m3_msi_route_7 : 32;
    } reg;
}pcie_ctrl_m3_msi_route_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int audio_msi_route_0 : 32;
    } reg;
}pcie_ctrl_audio_msi_route_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int audio_msi_route_1 : 32;
    } reg;
}pcie_ctrl_audio_msi_route_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int audio_msi_route_2 : 32;
    } reg;
}pcie_ctrl_audio_msi_route_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int audio_msi_route_3 : 32;
    } reg;
}pcie_ctrl_audio_msi_route_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int audio_msi_route_4 : 32;
    } reg;
}pcie_ctrl_audio_msi_route_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int audio_msi_route_5 : 32;
    } reg;
}pcie_ctrl_audio_msi_route_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int audio_msi_route_6 : 32;
    } reg;
}pcie_ctrl_audio_msi_route_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int audio_msi_route_7 : 32;
    } reg;
}pcie_ctrl_audio_msi_route_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_qoverflow_raw                     :  1;
        unsigned int trgt_cpl_timeout_raw                   :  1;
        unsigned int radm_cpl_timeout_raw                   :  1;
        unsigned int link_down_err_raw                      :  1;
        unsigned int ltssm_l0_entry_rcvry_p_raw             :  1;
        unsigned int cfg_send_f_err_raw                     :  1;
        unsigned int cfg_send_nf_err_raw                    :  1;
        unsigned int cfg_send_cor_err_raw                   :  1;
        unsigned int cfg_uncor_internal_err_sts_raw         :  1;
        unsigned int cfg_rcvr_overflow_err_sts_raw          :  1;
        unsigned int cfg_fc_protocol_err_sts_raw            :  1;
        unsigned int cfg_mlf_tlp_err_sts_raw                :  1;
        unsigned int cfg_surprise_down_er_sts_raw           :  1;
        unsigned int cfg_dl_protocol_err_sts_raw            :  1;
        unsigned int cfg_ecrc_err_sts_raw                   :  1;
        unsigned int cfg_corrected_internal_err_sts_raw     :  1;
        unsigned int cfg_replay_number_rollover_err_sts_raw :  1;
        unsigned int cfg_replay_timer_timeout_err_sts_raw   :  1;
        unsigned int cfg_bad_dllp_err_sts_raw               :  1;
        unsigned int cfg_bad_tlp_err_sts_raw                :  1;
        unsigned int cfg_rcvr_err_sts_raw                   :  1;
        unsigned int radm_rcvd_cfg0wr_poisoned_raw          :  1;
        unsigned int radm_rcvd_cfg1wr_poisoned_raw          :  1;
        unsigned int cfg_advisory_nf_sts_3_raw              :  1;
        unsigned int cfg_advisory_nf_sts_2_raw              :  1;
        unsigned int cfg_advisory_nf_sts_1_raw              :  1;
        unsigned int cfg_advisory_nf_sts_0_raw              :  1;
        unsigned int cfg_hdr_log_overflow_sts_3_raw         :  1;
        unsigned int cfg_hdr_log_overflow_sts_2_raw         :  1;
        unsigned int cfg_hdr_log_overflow_sts_1_raw         :  1;
        unsigned int cfg_hdr_log_overflow_sts_0_raw         :  1;
        unsigned int reserved_0                             :  1;
    } reg;
}pcie_ctrl_sys_int0_irq_int_raw_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_qoverflow_raw                     :  1;
        unsigned int trgt_cpl_timeout_raw                   :  1;
        unsigned int radm_cpl_timeout_raw                   :  1;
        unsigned int link_down_err_raw                      :  1;
        unsigned int ltssm_l0_entry_rcvry_p_raw             :  1;
        unsigned int cfg_send_f_err_raw                     :  1;
        unsigned int cfg_send_nf_err_raw                    :  1;
        unsigned int cfg_send_cor_err_raw                   :  1;
        unsigned int cfg_uncor_internal_err_sts_raw         :  1;
        unsigned int cfg_rcvr_overflow_err_sts_raw          :  1;
        unsigned int cfg_fc_protocol_err_sts_raw            :  1;
        unsigned int cfg_mlf_tlp_err_sts_raw                :  1;
        unsigned int cfg_surprise_down_er_sts_raw           :  1;
        unsigned int cfg_dl_protocol_err_sts_raw            :  1;
        unsigned int cfg_ecrc_err_sts_raw                   :  1;
        unsigned int cfg_corrected_internal_err_sts_raw     :  1;
        unsigned int cfg_replay_number_rollover_err_sts_raw :  1;
        unsigned int cfg_replay_timer_timeout_err_sts_raw   :  1;
        unsigned int cfg_bad_dllp_err_sts_raw               :  1;
        unsigned int cfg_bad_tlp_err_sts_raw                :  1;
        unsigned int cfg_rcvr_err_sts_raw                   :  1;
        unsigned int radm_rcvd_cfg0wr_poisoned_raw          :  1;
        unsigned int radm_rcvd_cfg1wr_poisoned_raw          :  1;
        unsigned int cfg_advisory_nf_sts_3_raw              :  1;
        unsigned int cfg_advisory_nf_sts_2_raw              :  1;
        unsigned int cfg_advisory_nf_sts_1_raw              :  1;
        unsigned int cfg_advisory_nf_sts_0_raw              :  1;
        unsigned int cfg_hdr_log_overflow_sts_3_raw         :  1;
        unsigned int cfg_hdr_log_overflow_sts_2_raw         :  1;
        unsigned int cfg_hdr_log_overflow_sts_1_raw         :  1;
        unsigned int cfg_hdr_log_overflow_sts_0_raw         :  1;
        unsigned int reserved_0                             :  1;
    } reg;
}pcie_ctrl_sys_int0_irq_int_force_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_qoverflow_mask                     :  1;
        unsigned int trgt_cpl_timeout_mask                   :  1;
        unsigned int radm_cpl_timeout_mask                   :  1;
        unsigned int link_down_err_mask                      :  1;
        unsigned int ltssm_l0_entry_rcvry_p_mask             :  1;
        unsigned int cfg_send_f_err_mask                     :  1;
        unsigned int cfg_send_nf_err_mask                    :  1;
        unsigned int cfg_send_cor_err_mask                   :  1;
        unsigned int cfg_uncor_internal_err_sts_mask         :  1;
        unsigned int cfg_rcvr_overflow_err_sts_mask          :  1;
        unsigned int cfg_fc_protocol_err_sts_mask            :  1;
        unsigned int cfg_mlf_tlp_err_sts_mask                :  1;
        unsigned int cfg_surprise_down_er_sts_mask           :  1;
        unsigned int cfg_dl_protocol_err_sts_mask            :  1;
        unsigned int cfg_ecrc_err_sts_mask                   :  1;
        unsigned int cfg_corrected_internal_err_sts_mask     :  1;
        unsigned int cfg_replay_number_rollover_err_sts_mask :  1;
        unsigned int cfg_replay_timer_timeout_err_sts_mask   :  1;
        unsigned int cfg_bad_dllp_err_sts_mask               :  1;
        unsigned int cfg_bad_tlp_err_sts_mask                :  1;
        unsigned int cfg_rcvr_err_sts_mask                   :  1;
        unsigned int radm_rcvd_cfg0wr_poisoned_mask          :  1;
        unsigned int radm_rcvd_cfg1wr_poisoned_mask          :  1;
        unsigned int cfg_advisory_nf_sts_3_mask              :  1;
        unsigned int cfg_advisory_nf_sts_2_mask              :  1;
        unsigned int cfg_advisory_nf_sts_1_mask              :  1;
        unsigned int cfg_advisory_nf_sts_0_mask              :  1;
        unsigned int cfg_hdr_log_overflow_sts_3_mask         :  1;
        unsigned int cfg_hdr_log_overflow_sts_2_mask         :  1;
        unsigned int cfg_hdr_log_overflow_sts_1_mask         :  1;
        unsigned int cfg_hdr_log_overflow_sts_0_mask         :  1;
        unsigned int reserved_0                              :  1;
    } reg;
}pcie_ctrl_sys_int0_irq_int_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_qoverflow_status                     :  1;
        unsigned int trgt_cpl_timeout_status                   :  1;
        unsigned int radm_cpl_timeout_status                   :  1;
        unsigned int link_down_err_status                      :  1;
        unsigned int ltssm_l0_entry_rcvry_p_status             :  1;
        unsigned int cfg_send_f_err_status                     :  1;
        unsigned int cfg_send_nf_err_status                    :  1;
        unsigned int cfg_send_cor_err_status                   :  1;
        unsigned int cfg_uncor_internal_err_sts_status         :  1;
        unsigned int cfg_rcvr_overflow_err_sts_status          :  1;
        unsigned int cfg_fc_protocol_err_sts_status            :  1;
        unsigned int cfg_mlf_tlp_err_sts_status                :  1;
        unsigned int cfg_surprise_down_er_sts_status           :  1;
        unsigned int cfg_dl_protocol_err_sts_status            :  1;
        unsigned int cfg_ecrc_err_sts_status                   :  1;
        unsigned int cfg_corrected_internal_err_sts_status     :  1;
        unsigned int cfg_replay_number_rollover_err_sts_status :  1;
        unsigned int cfg_replay_timer_timeout_err_sts_status   :  1;
        unsigned int cfg_bad_dllp_err_sts_status               :  1;
        unsigned int cfg_bad_tlp_err_sts_status                :  1;
        unsigned int cfg_rcvr_err_sts_status                   :  1;
        unsigned int radm_rcvd_cfg0wr_poisoned_status          :  1;
        unsigned int radm_rcvd_cfg1wr_poisoned_status          :  1;
        unsigned int cfg_advisory_nf_sts_3_status              :  1;
        unsigned int cfg_advisory_nf_sts_2_status              :  1;
        unsigned int cfg_advisory_nf_sts_1_status              :  1;
        unsigned int cfg_advisory_nf_sts_0_status              :  1;
        unsigned int cfg_hdr_log_overflow_sts_3_status         :  1;
        unsigned int cfg_hdr_log_overflow_sts_2_status         :  1;
        unsigned int cfg_hdr_log_overflow_sts_1_status         :  1;
        unsigned int cfg_hdr_log_overflow_sts_0_status         :  1;
        unsigned int reserved_0                                :  1;
    } reg;
}pcie_ctrl_sys_int0_irq_int_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_correctable_err_raw       :  1;
        unsigned int radm_nonfatal_err_raw          :  1;
        unsigned int radm_fatal_err_raw             :  1;
        unsigned int cfg_sys_err_rc_0_raw           :  1;
        unsigned int cfg_sys_err_rc_1_raw           :  1;
        unsigned int cfg_sys_err_rc_2_raw           :  1;
        unsigned int cfg_sys_err_rc_3_raw           :  1;
        unsigned int cfg_aer_rc_err_msi_0_raw       :  1;
        unsigned int cfg_aer_rc_err_msi_1_raw       :  1;
        unsigned int cfg_aer_rc_err_msi_2_raw       :  1;
        unsigned int cfg_aer_rc_err_msi_3_raw       :  1;
        unsigned int radm_trgt1_atu_cbuf_err_15_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_14_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_13_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_12_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_11_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_10_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_9_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_8_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_7_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_6_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_5_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_4_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_3_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_2_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_1_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_0_raw  :  1;
        unsigned int reserved_0                     :  5;
    } reg;
}pcie_ctrl_sys_int1_irq_int_raw_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_correctable_err_raw       :  1;
        unsigned int radm_nonfatal_err_raw          :  1;
        unsigned int radm_fatal_err_raw             :  1;
        unsigned int cfg_sys_err_rc_0_raw           :  1;
        unsigned int cfg_sys_err_rc_1_raw           :  1;
        unsigned int cfg_sys_err_rc_2_raw           :  1;
        unsigned int cfg_sys_err_rc_3_raw           :  1;
        unsigned int cfg_aer_rc_err_msi_0_raw       :  1;
        unsigned int cfg_aer_rc_err_msi_1_raw       :  1;
        unsigned int cfg_aer_rc_err_msi_2_raw       :  1;
        unsigned int cfg_aer_rc_err_msi_3_raw       :  1;
        unsigned int radm_trgt1_atu_cbuf_err_15_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_14_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_13_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_12_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_11_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_10_raw :  1;
        unsigned int radm_trgt1_atu_cbuf_err_9_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_8_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_7_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_6_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_5_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_4_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_3_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_2_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_1_raw  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_0_raw  :  1;
        unsigned int reserved_0                     :  5;
    } reg;
}pcie_ctrl_sys_int1_irq_int_force_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_correctable_err_mask       :  1;
        unsigned int radm_nonfatal_err_mask          :  1;
        unsigned int radm_fatal_err_mask             :  1;
        unsigned int cfg_sys_err_rc_0_mask           :  1;
        unsigned int cfg_sys_err_rc_1_mask           :  1;
        unsigned int cfg_sys_err_rc_2_mask           :  1;
        unsigned int cfg_sys_err_rc_3_mask           :  1;
        unsigned int cfg_aer_rc_err_msi_0_mask       :  1;
        unsigned int cfg_aer_rc_err_msi_1_mask       :  1;
        unsigned int cfg_aer_rc_err_msi_2_mask       :  1;
        unsigned int cfg_aer_rc_err_msi_3_mask       :  1;
        unsigned int radm_trgt1_atu_cbuf_err_15_mask :  1;
        unsigned int radm_trgt1_atu_cbuf_err_14_mask :  1;
        unsigned int radm_trgt1_atu_cbuf_err_13_mask :  1;
        unsigned int radm_trgt1_atu_cbuf_err_12_mask :  1;
        unsigned int radm_trgt1_atu_cbuf_err_11_mask :  1;
        unsigned int radm_trgt1_atu_cbuf_err_10_mask :  1;
        unsigned int radm_trgt1_atu_cbuf_err_9_mask  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_8_mask  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_7_mask  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_6_mask  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_5_mask  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_4_mask  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_3_mask  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_2_mask  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_1_mask  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_0_mask  :  1;
        unsigned int reserved_0                      :  5;
    } reg;
}pcie_ctrl_sys_int1_irq_int_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_correctable_err_status       :  1;
        unsigned int radm_nonfatal_err_status          :  1;
        unsigned int radm_fatal_err_status             :  1;
        unsigned int cfg_sys_err_rc_0_status           :  1;
        unsigned int cfg_sys_err_rc_1_status           :  1;
        unsigned int cfg_sys_err_rc_2_status           :  1;
        unsigned int cfg_sys_err_rc_3_status           :  1;
        unsigned int cfg_aer_rc_err_msi_0_status       :  1;
        unsigned int cfg_aer_rc_err_msi_1_status       :  1;
        unsigned int cfg_aer_rc_err_msi_2_status       :  1;
        unsigned int cfg_aer_rc_err_msi_3_status       :  1;
        unsigned int radm_trgt1_atu_cbuf_err_15_status :  1;
        unsigned int radm_trgt1_atu_cbuf_err_14_status :  1;
        unsigned int radm_trgt1_atu_cbuf_err_13_status :  1;
        unsigned int radm_trgt1_atu_cbuf_err_12_status :  1;
        unsigned int radm_trgt1_atu_cbuf_err_11_status :  1;
        unsigned int radm_trgt1_atu_cbuf_err_10_status :  1;
        unsigned int radm_trgt1_atu_cbuf_err_9_status  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_8_status  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_7_status  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_6_status  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_5_status  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_4_status  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_3_status  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_2_status  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_1_status  :  1;
        unsigned int radm_trgt1_atu_cbuf_err_0_status  :  1;
        unsigned int reserved_0                        :  5;
    } reg;
}pcie_ctrl_sys_int1_irq_int_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rtlh_rfc_upd_raw             :  1;
        unsigned int radm_pm_pme_raw              :  1;
        unsigned int radm_pm_to_ack_raw           :  1;
        unsigned int cfg_pme_msi_0_raw            :  1;
        unsigned int cfg_pme_msi_1_raw            :  1;
        unsigned int cfg_pme_msi_2_raw            :  1;
        unsigned int cfg_pme_msi_3_raw            :  1;
        unsigned int radm_pm_turnoff_raw          :  1;
        unsigned int radm_slot_pwr_limit_raw      :  1;
        unsigned int radm_msg_unlock_raw          :  1;
        unsigned int radm_vendor_msg_raw          :  1;
        unsigned int radm_msg_ltr_raw             :  1;
        unsigned int smlh_link_up_int_raw         :  1;
        unsigned int usp_eq_redo_executed_int_raw :  1;
        unsigned int cfg_link_eq_req_int_raw      :  1;
        unsigned int edma_int_15_raw              :  1;
        unsigned int edma_int_14_raw              :  1;
        unsigned int edma_int_13_raw              :  1;
        unsigned int edma_int_12_raw              :  1;
        unsigned int edma_int_11_raw              :  1;
        unsigned int edma_int_10_raw              :  1;
        unsigned int edma_int_9_raw               :  1;
        unsigned int edma_int_8_raw               :  1;
        unsigned int edma_int_7_raw               :  1;
        unsigned int edma_int_6_raw               :  1;
        unsigned int edma_int_5_raw               :  1;
        unsigned int edma_int_4_raw               :  1;
        unsigned int edma_int_3_raw               :  1;
        unsigned int edma_int_2_raw               :  1;
        unsigned int edma_int_1_raw               :  1;
        unsigned int edma_int_0_raw               :  1;
        unsigned int reserved_0                   :  1;
    } reg;
}pcie_ctrl_sys_int2_irq_int_raw_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rtlh_rfc_upd_raw             :  1;
        unsigned int radm_pm_pme_raw              :  1;
        unsigned int radm_pm_to_ack_raw           :  1;
        unsigned int cfg_pme_msi_0_raw            :  1;
        unsigned int cfg_pme_msi_1_raw            :  1;
        unsigned int cfg_pme_msi_2_raw            :  1;
        unsigned int cfg_pme_msi_3_raw            :  1;
        unsigned int radm_pm_turnoff_raw          :  1;
        unsigned int radm_slot_pwr_limit_raw      :  1;
        unsigned int radm_msg_unlock_raw          :  1;
        unsigned int radm_vendor_msg_raw          :  1;
        unsigned int radm_msg_ltr_raw             :  1;
        unsigned int smlh_link_up_int_raw         :  1;
        unsigned int usp_eq_redo_executed_int_raw :  1;
        unsigned int cfg_link_eq_req_int_raw      :  1;
        unsigned int edma_int_15_raw              :  1;
        unsigned int edma_int_14_raw              :  1;
        unsigned int edma_int_13_raw              :  1;
        unsigned int edma_int_12_raw              :  1;
        unsigned int edma_int_11_raw              :  1;
        unsigned int edma_int_10_raw              :  1;
        unsigned int edma_int_9_raw               :  1;
        unsigned int edma_int_8_raw               :  1;
        unsigned int edma_int_7_raw               :  1;
        unsigned int edma_int_6_raw               :  1;
        unsigned int edma_int_5_raw               :  1;
        unsigned int edma_int_4_raw               :  1;
        unsigned int edma_int_3_raw               :  1;
        unsigned int edma_int_2_raw               :  1;
        unsigned int edma_int_1_raw               :  1;
        unsigned int edma_int_0_raw               :  1;
        unsigned int reserved_0                   :  1;
    } reg;
}pcie_ctrl_sys_int2_irq_int_force_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rtlh_rfc_upd_mask             :  1;
        unsigned int radm_pm_pme_mask              :  1;
        unsigned int radm_pm_to_ack_mask           :  1;
        unsigned int cfg_pme_msi_0_mask            :  1;
        unsigned int cfg_pme_msi_1_mask            :  1;
        unsigned int cfg_pme_msi_2_mask            :  1;
        unsigned int cfg_pme_msi_3_mask            :  1;
        unsigned int radm_pm_turnoff_mask          :  1;
        unsigned int radm_slot_pwr_limit_mask      :  1;
        unsigned int radm_msg_unlock_mask          :  1;
        unsigned int radm_vendor_msg_mask          :  1;
        unsigned int radm_msg_ltr_mask             :  1;
        unsigned int smlh_link_up_int_mask         :  1;
        unsigned int usp_eq_redo_executed_int_mask :  1;
        unsigned int cfg_link_eq_req_int_mask      :  1;
        unsigned int edma_int_15_mask              :  1;
        unsigned int edma_int_14_mask              :  1;
        unsigned int edma_int_13_mask              :  1;
        unsigned int edma_int_12_mask              :  1;
        unsigned int edma_int_11_mask              :  1;
        unsigned int edma_int_10_mask              :  1;
        unsigned int edma_int_9_mask               :  1;
        unsigned int edma_int_8_mask               :  1;
        unsigned int edma_int_7_mask               :  1;
        unsigned int edma_int_6_mask               :  1;
        unsigned int edma_int_5_mask               :  1;
        unsigned int edma_int_4_mask               :  1;
        unsigned int edma_int_3_mask               :  1;
        unsigned int edma_int_2_mask               :  1;
        unsigned int edma_int_1_mask               :  1;
        unsigned int edma_int_0_mask               :  1;
        unsigned int reserved_0                    :  1;
    } reg;
}pcie_ctrl_sys_int2_irq_int_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rtlh_rfc_upd_status             :  1;
        unsigned int radm_pm_pme_status              :  1;
        unsigned int radm_pm_to_ack_status           :  1;
        unsigned int cfg_pme_msi_0_status            :  1;
        unsigned int cfg_pme_msi_1_status            :  1;
        unsigned int cfg_pme_msi_2_status            :  1;
        unsigned int cfg_pme_msi_3_status            :  1;
        unsigned int radm_pm_turnoff_status          :  1;
        unsigned int radm_slot_pwr_limit_status      :  1;
        unsigned int radm_msg_unlock_status          :  1;
        unsigned int radm_vendor_msg_status          :  1;
        unsigned int radm_msg_ltr_status             :  1;
        unsigned int smlh_link_up_int_status         :  1;
        unsigned int usp_eq_redo_executed_int_status :  1;
        unsigned int cfg_link_eq_req_int_status      :  1;
        unsigned int edma_int_15_status              :  1;
        unsigned int edma_int_14_status              :  1;
        unsigned int edma_int_13_status              :  1;
        unsigned int edma_int_12_status              :  1;
        unsigned int edma_int_11_status              :  1;
        unsigned int edma_int_10_status              :  1;
        unsigned int edma_int_9_status               :  1;
        unsigned int edma_int_8_status               :  1;
        unsigned int edma_int_7_status               :  1;
        unsigned int edma_int_6_status               :  1;
        unsigned int edma_int_5_status               :  1;
        unsigned int edma_int_4_status               :  1;
        unsigned int edma_int_3_status               :  1;
        unsigned int edma_int_2_status               :  1;
        unsigned int edma_int_1_status               :  1;
        unsigned int edma_int_0_status               :  1;
        unsigned int reserved_0                      :  1;
    } reg;
}pcie_ctrl_sys_int2_irq_int_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_inta_asserted_raw   :  1;
        unsigned int radm_intb_asserted_raw   :  1;
        unsigned int radm_intc_asserted_raw   :  1;
        unsigned int radm_intd_asserted_raw   :  1;
        unsigned int radm_inta_deasserted_raw :  1;
        unsigned int radm_intb_deasserted_raw :  1;
        unsigned int radm_intc_deasserted_raw :  1;
        unsigned int radm_intd_deasserted_raw :  1;
        unsigned int cfg_link_auto_bw_msi_raw :  1;
        unsigned int cfg_link_auto_bw_int_raw :  1;
        unsigned int cfg_bw_mgt_int_raw       :  1;
        unsigned int cfg_bw_mgt_msi_raw       :  1;
        unsigned int assert_inta_grt_raw      :  1;
        unsigned int assert_intb_grt_raw      :  1;
        unsigned int assert_intc_grt_raw      :  1;
        unsigned int assert_intd_grt_raw      :  1;
        unsigned int deassert_inta_grt_raw    :  1;
        unsigned int deassert_intb_grt_raw    :  1;
        unsigned int deassert_intc_grt_raw    :  1;
        unsigned int deassert_intd_grt_raw    :  1;
        unsigned int reserved_0               : 12;
    } reg;
}pcie_ctrl_sys_int3_irq_int_raw_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_inta_asserted_raw   :  1;
        unsigned int radm_intb_asserted_raw   :  1;
        unsigned int radm_intc_asserted_raw   :  1;
        unsigned int radm_intd_asserted_raw   :  1;
        unsigned int radm_inta_deasserted_raw :  1;
        unsigned int radm_intb_deasserted_raw :  1;
        unsigned int radm_intc_deasserted_raw :  1;
        unsigned int radm_intd_deasserted_raw :  1;
        unsigned int cfg_link_auto_bw_msi_raw :  1;
        unsigned int cfg_link_auto_bw_int_raw :  1;
        unsigned int cfg_bw_mgt_int_raw       :  1;
        unsigned int cfg_bw_mgt_msi_raw       :  1;
        unsigned int assert_inta_grt_raw      :  1;
        unsigned int assert_intb_grt_raw      :  1;
        unsigned int assert_intc_grt_raw      :  1;
        unsigned int assert_intd_grt_raw      :  1;
        unsigned int deassert_inta_grt_raw    :  1;
        unsigned int deassert_intb_grt_raw    :  1;
        unsigned int deassert_intc_grt_raw    :  1;
        unsigned int deassert_intd_grt_raw    :  1;
        unsigned int reserved_0               : 12;
    } reg;
}pcie_ctrl_sys_int3_irq_int_force_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_inta_asserted_mask   :  1;
        unsigned int radm_intb_asserted_mask   :  1;
        unsigned int radm_intc_asserted_mask   :  1;
        unsigned int radm_intd_asserted_mask   :  1;
        unsigned int radm_inta_deasserted_mask :  1;
        unsigned int radm_intb_deasserted_mask :  1;
        unsigned int radm_intc_deasserted_mask :  1;
        unsigned int radm_intd_deasserted_mask :  1;
        unsigned int cfg_link_auto_bw_msi_mask :  1;
        unsigned int cfg_link_auto_bw_int_mask :  1;
        unsigned int cfg_bw_mgt_int_mask       :  1;
        unsigned int cfg_bw_mgt_msi_mask       :  1;
        unsigned int assert_inta_grt_mask      :  1;
        unsigned int assert_intb_grt_mask      :  1;
        unsigned int assert_intc_grt_mask      :  1;
        unsigned int assert_intd_grt_mask      :  1;
        unsigned int deassert_inta_grt_mask    :  1;
        unsigned int deassert_intb_grt_mask    :  1;
        unsigned int deassert_intc_grt_mask    :  1;
        unsigned int deassert_intd_grt_mask    :  1;
        unsigned int reserved_0                : 12;
    } reg;
}pcie_ctrl_sys_int3_irq_int_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int radm_inta_asserted_status   :  1;
        unsigned int radm_intb_asserted_status   :  1;
        unsigned int radm_intc_asserted_status   :  1;
        unsigned int radm_intd_asserted_status   :  1;
        unsigned int radm_inta_deasserted_status :  1;
        unsigned int radm_intb_deasserted_status :  1;
        unsigned int radm_intc_deasserted_status :  1;
        unsigned int radm_intd_deasserted_status :  1;
        unsigned int cfg_link_auto_bw_msi_status :  1;
        unsigned int cfg_link_auto_bw_int_status :  1;
        unsigned int cfg_bw_mgt_int_status       :  1;
        unsigned int cfg_bw_mgt_msi_status       :  1;
        unsigned int assert_inta_grt_status      :  1;
        unsigned int assert_intb_grt_status      :  1;
        unsigned int assert_intc_grt_status      :  1;
        unsigned int assert_intd_grt_status      :  1;
        unsigned int deassert_inta_grt_status    :  1;
        unsigned int deassert_intb_grt_status    :  1;
        unsigned int deassert_intc_grt_status    :  1;
        unsigned int deassert_intd_grt_status    :  1;
        unsigned int reserved_0                  : 12;
    } reg;
}pcie_ctrl_sys_int3_irq_int_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}pcie_ctrl_regfile_cgbypass_t;

#endif
