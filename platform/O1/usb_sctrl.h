// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef USB_SUBSYS_SCTRL_APB_REGIF_H
#define USB_SUBSYS_SCTRL_APB_REGIF_H

#define USB_USB_VERSION                                             0x0000
#define USB_USB_SUBSYS_TESTPIN_SEL                                  0x0004
#define USB_SEL_USB_SUBSYS_TESTPIN_SEL_SHIFT                        0
#define USB_SEL_USB_SUBSYS_TESTPIN_SEL_MASK                         0x000000ff
#define USB_SEL_USB_COMBOPHY_TESTPIN_SEL_SHIFT                      8
#define USB_SEL_USB_COMBOPHY_TESTPIN_SEL_MASK                       0x0000ff00
#define USB_USB_VBUS_VALID_DEBO_BYPASS                              0x0008
#define USB_BYPASS_USB_VBUS_VALID_DEBO_BYPASS_SHIFT                0
#define USB_BYPASS_USB_VBUS_VALID_DEBO_BYPASS_MASK                 0x00000001
#define USB_PIPE_PCLK_SEL                                           0x000c
#define USB_PIPE_PCLK_SEL_PIPE_PCLK_SEL_SHIFT                                          0
#define USB_PIPE_PCLK_SEL_PIPE_PCLK_SEL_MASK                                           0x00000001
#define USB_USB_SUBSYS_MEM_CFG_REG_0                                0x0010
#define USB_USB_SUBSYS_MEM_CFG_REG_1                                0x0014
#define USB_USB_SUBSYS_MEM_CFG_REG_2                                0x0018
#define USB_UTMI_HOST_DISCONNECT                                    0x001c
#define USB_UTMI_HOST_DISCONNECT_SEL_SHIFT                        0
#define USB_UTMI_HOST_DISCONNECT_SEL_MASK                         0x00000001
#define USB_UTMI_HOST_DISCONNECT_DEG_SHIFT                        1
#define USB_UTMI_HOST_DISCONNECT_DEG_MASK                         0x00000002
#define USB_UTMI_SIGANL_DEBUG                                       0x0020
#define USB_DEBUG_UTMI_TX_DATA_SEL_SHIFT                                   0
#define USB_DEBUG_UTMI_TX_DATA_SEL_MASK                                    0x00000001
#define USB_DEBUG_UTMI_TX_DATA_DEG_SHIFT                                   1
#define USB_DEBUG_UTMI_TX_DATA_DEG_MASK                                    0x000001fe
#define USB_DEBUG_UTMI_TXVALID_SEL_SHIFT                                   9
#define USB_DEBUG_UTMI_TXVALID_SEL_MASK                                    0x00000200
#define USB_DEBUG_UTMI_TXVALID_DEG_SHIFT                                   10
#define USB_DEBUG_UTMI_TXVALID_DEG_MASK                                    0x00000400
#define USB_DEBUG_UTMI_DMPULLDOWN_SEL_SHIFT                                11
#define USB_DEBUG_UTMI_DMPULLDOWN_SEL_MASK                                 0x00000800
#define USB_DEBUG_UTMI_DMPULLDOWN_DEG_SHIFT                                12
#define USB_DEBUG_UTMI_DMPULLDOWN_DEG_MASK                                 0x00001000
#define USB_DEBUG_UTMI_DPPULLDOWN_SEL_SHIFT                                13
#define USB_DEBUG_UTMI_DPPULLDOWN_SEL_MASK                                 0x00002000
#define USB_DEBUG_UTMI_DPPULLDOWN_DEG_SHIFT                                14
#define USB_DEBUG_UTMI_DPPULLDOWN_DEG_MASK                                 0x00004000
#define USB_DEBUG_UTMI_XCVRSELECT_SEL_SHIFT                                15
#define USB_DEBUG_UTMI_XCVRSELECT_SEL_MASK                                 0x00008000
#define USB_DEBUG_UTMI_XCVRSELECT_DEG_SHIFT                                16
#define USB_DEBUG_UTMI_XCVRSELECT_DEG_MASK                                 0x00030000
#define USB_DEBUG_UTMI_TERMSELECT_SEL_SHIFT                                18
#define USB_DEBUG_UTMI_TERMSELECT_SEL_MASK                                 0x00040000
#define USB_DEBUG_UTMI_TERMSELECT_DEG_SHIFT                                19
#define USB_DEBUG_UTMI_TERMSELECT_DEG_MASK                                 0x00080000
#define USB_DEBUG_UTMI_OPMODE_SEL_SHIFT                                    20
#define USB_DEBUG_UTMI_OPMODE_SEL_MASK                                     0x00100000
#define USB_DEBUG_UTMI_OPMODE_DEG_SHIFT                                    21
#define USB_DEBUG_UTMI_OPMODE_DEG_MASK                                     0x00600000
#define USB_LINK_STATUS_CLR                                         0x0024
#define USB_LINK_STATUS_CLR_LINK_STATUS_CLR_SHIFT                                      0
#define USB_LINK_STATUS_CLR_LINK_STATUS_CLR_MASK                                       0x00000001
#define USB_LINK_STATE_STATUS                                       0x0028
#define USB_LINK_STATE_STATUS_LINK_STATE_STATUS_SHIFT                                  0
#define USB_LINK_STATE_STATUS_LINK_STATE_STATUS_MASK                                   0x3fffffff
#define USB_POLL_RECOVERY_STATE_STATUS                              0x002c
#define USB_STATUS_POLL_STATE_STATUS_SHIFT                         0
#define USB_STATUS_POLL_STATE_STATUS_MASK                          0x003fffff
#define USB_STATUS_RECOVERY_STATE_STATUS_SHIFT                     22
#define USB_STATUS_RECOVERY_STATE_STATUS_MASK                      0x1fc00000
#define USB_OCLA_HIGH_LOW_VLD_SEL                                   0x0030
#define USB_SEL_OCLA_LOW_HIGH_SEL_SHIFT                              0
#define USB_SEL_OCLA_LOW_HIGH_SEL_MASK                               0x00000001
#define USB_SEL_OCLA_VLD_WIDTH_SEL_SHIFT                             1
#define USB_SEL_OCLA_VLD_WIDTH_SEL_MASK                              0x00000002
#define USB_USB_DEBUG_STATUS                                        0x0034
#define USB_DEBUG_STATUS_ST_USBC_APB_CLK_GATE_SHIFT                                0
#define USB_DEBUG_STATUS_ST_USBC_APB_CLK_GATE_MASK                                 0x00000001
#define USB_DEBUG_STATUS_ST_USBC_LINK_CLK_GATE_SHIFT                               1
#define USB_DEBUG_STATUS_ST_USBC_LINK_CLK_GATE_MASK                                0x00000002
#define USB_PIPE_STATUS                                             0x0038
#define USB_STATUS_PIPE_PHY_MODE_SHIFT                                            0
#define USB_STATUS_PIPE_PHY_MODE_MASK                                             0x00000003
#define USB_STATUS_PIPE_COMPLIANCE_SHIFT                                          2
#define USB_STATUS_PIPE_COMPLIANCE_MASK                                           0x00000004
#define USB_STATUS_PIPE_TXMARGIN_SHIFT                                            3
#define USB_STATUS_PIPE_TXMARGIN_MASK                                             0x00000038
#define USB_STATUS_PIPE_TXSWING_SHIFT                                             6
#define USB_STATUS_PIPE_TXSWING_MASK                                              0x00000040
#define USB_STATUS_HOST_LEGACY_SMI_INTERRUPT_SHIFT                                7
#define USB_STATUS_HOST_LEGACY_SMI_INTERRUPT_MASK                                 0x00000080
#define USB_STATUS_RX_SE_DP_HV_SHIFT                                              8
#define USB_STATUS_RX_SE_DP_HV_MASK                                               0x00000100
#define USB_STATUS_RX_SE_DM_HV_SHIFT                                              9
#define USB_STATUS_RX_SE_DM_HV_MASK                                               0x00000200
#define USB_USB_VBUS_VALID_CC_SEL                                   0x0040
#define USB_CC_SEL_USB_VBUS_VALID_SEL_SHIFT                             0
#define USB_CC_SEL_USB_VBUS_VALID_SEL_MASK                              0x00000001
#define USB_CC_SEL_USB_VBUS_VALID_CC_SHIFT                              1
#define USB_CC_SEL_USB_VBUS_VALID_CC_MASK                               0x00000002
#define USB_CC_SEL_CTRL_SRPOTG_VALID_SEL_SHIFT                          2
#define USB_CC_SEL_CTRL_SRPOTG_VALID_SEL_MASK                           0x00000004
#define USB_CC_SEL_COMBOPHY_DP_CLK_SEL_SHIFT                            3
#define USB_CC_SEL_COMBOPHY_DP_CLK_SEL_MASK                             0x00000008
#define USB_CONTROLLER_RAM_PUDELAY                                  0x0044
#define USB_RAM0_SD_PUDELAY_SHIFT                               0
#define USB_RAM0_SD_PUDELAY_MASK                                0x00000001
#define USB_RAM0_DSLP_PUDELAY_SHIFT                             1
#define USB_RAM0_DSLP_PUDELAY_MASK                              0x00000002
#define USB_RAM1_SD_PUDELAY_SHIFT                               2
#define USB_RAM1_SD_PUDELAY_MASK                                0x00000004
#define USB_RAM1_DSLP_PUDELAY_SHIFT                             3
#define USB_RAM1_DSLP_PUDELAY_MASK                              0x00000008
#define USB_RAM2_SD_PUDELAY_SHIFT                               4
#define USB_RAM2_SD_PUDELAY_MASK                                0x00000010
#define USB_RAM2_DSLP_PUDELAY_SHIFT                             5
#define USB_RAM2_DSLP_PUDELAY_MASK                              0x00000020
#define USB_BIGENDIAN_GS                                            0x0048
#define USB_BIGENDIAN_GS_BIGENDIAN_GS_SHIFT                                            0
#define USB_BIGENDIAN_GS_BIGENDIAN_GS_MASK                                             0x00000001
#define USB_STAR_FIX_DISABLE_CTL_INP                                0x004c
#define USB_HOST_CURRENT_BELT                                       0x0050
#define USB_BELT_HOST_CURRENT_BELT_SHIFT                                  0
#define USB_BELT_HOST_CURRENT_BELT_MASK                                   0x00000fff
#define USB_HOST_U2_U3_PORT                                         0x0054
#define USB_U2_U3_PORT_HOST_U2_PORT_DISABLE_SHIFT                                 0
#define USB_U2_U3_PORT_HOST_U2_PORT_DISABLE_MASK                                  0x00000001
#define USB_U2_U3_PORT_HOST_U3_PORT_DISABLE_SHIFT                                 1
#define USB_U2_U3_PORT_HOST_U3_PORT_DISABLE_MASK                                  0x00000002
#define USB_U2_U3_PORT_HUB_PORT_PERM_ATTACH_SHIFT                                 2
#define USB_U2_U3_PORT_HUB_PORT_PERM_ATTACH_MASK                                  0x0000000c
#define USB_FORCE_GEN1_SPEED                                   0x0058
#define USB_GEN1_SPEED_HOST_FORCE_GEN1_SPEED_SHIFT                          0
#define USB_GEN1_SPEED_HOST_FORCE_GEN1_SPEED_MASK                           0x00000001
#define USB_LTSSM_CLK_STATE                                         0x0064
#define USB_LTSSM_CLK_STATE_LTSSM_CLK_STATE_SHIFT                                      0
#define USB_LTSSM_CLK_STATE_LTSSM_CLK_STATE_MASK                                       0x0000000f
#define USB_HUB_VBUS_CTRL_CLK_GATE                                  0x0068
#define USB_CLK_GATE_HUB_VBUS_CTRL_SHIFT                                 0
#define USB_CLK_GATE_HUB_VBUS_CTRL_MASK                                  0x00000003
#define USB_CLK_GATE_PMGT_EXT_BUS_CLK_GATE_SHIFT                         2
#define USB_CLK_GATE_PMGT_EXT_BUS_CLK_GATE_MASK                          0x00000004
#define USB_HOST_PORT_OVERCURRENT                                   0x006c
#define USB_HOST_PORT_OVERCURRENT_SHIFT                          0
#define USB_HOST_PORT_OVERCURRENT_MASK                           0x00000003
#define USB_HOST_PORT_POWER_CONTROL_PRESENT_SHIFT                2
#define USB_HOST_PORT_POWER_CONTROL_PRESENT_MASK                 0x00000004
#define USB_HOST_MIS_ENABLE_XHC_BME                                 0x0070
#define USB_XHC_BME_HOST_MSI_ENABLE_SHIFT                              0
#define USB_XHC_BME_HOST_MSI_ENABLE_MASK                               0x00000001
#define USB_XHC_BME_XHC_BME_SHIFT                                      1
#define USB_XHC_BME_XHC_BME_MASK                                       0x00000002
#define USB_BUS_FILTER_BYPASS                                       0x0074
#define USB_BUS_FILTER_BYPASS_BUS_FILTER_BYPASS_SHIFT                                  0
#define USB_BUS_FILTER_BYPASS_BUS_FILTER_BYPASS_MASK                                   0x0000000f
#define USB_SOC_COMMON_RD_WR_BUS                                    0x0078
#define USB_SOC_COMMON_RD_WR_BUS_SOC_COMMON_RD_WR_BUS_SHIFT                            0
#define USB_SOC_COMMON_RD_WR_BUS_SOC_COMMON_RD_WR_BUS_MASK                             0x00000001
#define USB_SOC_RD_UF_KB_BANDWIDTH                                  0x007c
#define USB_SOC_RD_UF_KB_BANDWIDTH_SHIFT                        0
#define USB_SOC_RD_UF_KB_BANDWIDTH_MASK                         0x00007fff
#define USB_SOC_WR_UF_KB_BANDWIDTH_SHIFT                        15
#define USB_SOC_WR_UF_KB_BANDWIDTH_MASK                         0x3fff8000
#define USB_DEVSPD_OVRD                                             0x0080
#define USB_DEVSPD_OVRD_DEVSPD_OVRD_SHIFT                                              0
#define USB_DEVSPD_OVRD_DEVSPD_OVRD_MASK                                               0x0000000f
#define USB_SDBN_DS_PORT_ESS_INACTIVE                               0x0084
#define USB_SDBN_DS_PORT_ESS_INACTIVE_SHIFT                  0
#define USB_SDBN_DS_PORT_ESS_INACTIVE_MASK                   0x00000001
#define USB_LOGIC_ANALYZER_TRACE_EXT_MUX_EN                         0x0088
#define USB_MUX_EN_LOGIC_ANALYZER_TRACE_EXT_MUX_EN_SHIFT      0
#define USB_MUX_EN_LOGIC_ANALYZER_TRACE_EXT_MUX_EN_MASK       0x00000001
#define USB_MUX                            0x008c
#define USB_PIPE_MX_PCLK_READY                                      0x0090
#define USB_PIPE_MX_PCLK_READY_SHIFT                                0
#define USB_PIPE_MX_PCLK_READY_MASK                                 0x00000001
#define USB_USB_CTRL_DEBUG_0                                        0x0094
#define USB_USB_CTRL_DEBUG_1                                        0x0098
#define USB_USB_CTRL_DEBUG_2                                        0x009c
#define USB_USB_CTRL_DEBUG_3                                        0x00a0
#define USB_USB_CTRL_DEBUG_3_USB_CTRL_DEBUG_3_SHIFT                                    0
#define USB_USB_CTRL_DEBUG_3_USB_CTRL_DEBUG_3_MASK                                     0x07ffffff
#define USB_UTMI_RXVALIDH                                           0x00a4
#define USB_UTMI_RXVALIDH_UTMI_RXVALIDH_SHIFT                                          0
#define USB_UTMI_RXVALIDH_UTMI_RXVALIDH_MASK                                           0x00000001
#define USB_UTMI_RXVALIDH_UTMI_TXVALIDH_SHIFT                                          1
#define USB_UTMI_RXVALIDH_UTMI_TXVALIDH_MASK                                           0x00000002
#define USB_UTMI_RXVALIDH_UTMI_WORD_IF_SHIFT                                           2
#define USB_UTMI_RXVALIDH_UTMI_WORD_IF_MASK                                            0x00000004
#define USB_UTMI_RXVALIDH_UTMI_FSLS_LOW_POWER_SHIFT                                    3
#define USB_UTMI_RXVALIDH_UTMI_FSLS_LOW_POWER_MASK                                     0x00000008
#define USB_UTMI_RXVALIDH_UTMI_FSLSSERIALMODE_SHIFT                                    4
#define USB_UTMI_RXVALIDH_UTMI_FSLSSERIALMODE_MASK                                     0x00000010
#define USB_UTMI_RXVALIDH_UTMISRP_BVALID_SHIFT                                         5
#define USB_UTMI_RXVALIDH_UTMISRP_BVALID_MASK                                          0x00000020
#define USB_UTMI_RXVALIDH_RES_REQ_OUT_SHIFT                                            6
#define USB_UTMI_RXVALIDH_RES_REQ_OUT_MASK                                             0x00000040
#define USB_GP_OUT                                                  0x00a8
#define USB_GP_OUT_GP_OUT_SHIFT                                                        0
#define USB_GP_OUT_GP_OUT_MASK                                                         0x0000ffff
#define USB_GP_IN_REG                                               0x00ac
#define USB_GP_IN_REG_GP_IN_SEL_SHIFT                                                  0
#define USB_GP_IN_REG_GP_IN_SEL_MASK                                                   0x00000001
#define USB_GP_IN_REG_GP_IN_REG_SHIFT                                                  1
#define USB_GP_IN_REG_GP_IN_REG_MASK                                                   0x0001fffe
#define USB_PME_EN                                                  0x00b0
#define USB_PME_EN_PME_EN_SHIFT                                                        0
#define USB_PME_EN_PME_EN_MASK                                                         0x00000001
#define USB_PME_GENERATION                                          0x00b4
#define USB_PME_GENERATION_PME_GENERATION_SHIFT                                        0
#define USB_PME_GENERATION_PME_GENERATION_MASK                                         0x00000001
#define USB_PIPE_DATABUSWIDTH                                       0x00b8
#define USB_PIPE_DATABUSWIDTH_PIPE_DATABUSWIDTH_SHIFT                                  0
#define USB_PIPE_DATABUSWIDTH_PIPE_DATABUSWIDTH_MASK                                   0x00000003
#define USB_INTR_MASK                                               0x00bc
#define USB_INTR_MASK_INTR_USB_ACPU_MASK_SHIFT                                         0
#define USB_INTR_MASK_INTR_USB_ACPU_MASK_MASK                                          0x00000001
#define USB_INTR_MASK_INTR_USB_LPMCU_MASK_SHIFT                                        1
#define USB_INTR_MASK_INTR_USB_LPMCU_MASK_MASK                                         0x00000002
#define USB_INTR_MASK_INTR_USB_ADSP_MASK_SHIFT                                         2
#define USB_INTR_MASK_INTR_USB_ADSP_MASK_MASK                                          0x00000004
#define USB_INTR_MASK_INTR_USB_ACPU_ERR_MASK_SHIFT                                     3
#define USB_INTR_MASK_INTR_USB_ACPU_ERR_MASK_MASK                                      0x00000008
#define USB_INTR_MASK_INTR_USB_LPMCU_ERR_MASK_SHIFT                                    4
#define USB_INTR_MASK_INTR_USB_LPMCU_ERR_MASK_MASK                                     0x00000010
#define USB_INTR_MASK_INTR_USB_ADSP_ERR_MASK_SHIFT                                     5
#define USB_INTR_MASK_INTR_USB_ADSP_ERR_MASK_MASK                                      0x00000020
#define USB_INTR_MASK_INTR_SUB_VBUS_ACPU_MASK_SHIFT                                    6
#define USB_INTR_MASK_INTR_SUB_VBUS_ACPU_MASK_MASK                                     0x00000040
#define USB_INTR_MASK_INTR_SUB_VBUS_LPMCU_MASK_SHIFT                                   7
#define USB_INTR_MASK_INTR_SUB_VBUS_LPMCU_MASK_MASK                                    0x00000080
#define USB_INTR_MASK_INTR_SUB_VBUS_ADSP_MASK_SHIFT                                    8
#define USB_INTR_MASK_INTR_SUB_VBUS_ADSP_MASK_MASK                                     0x00000100
#define USB_IP_RST_CONTROLLER_REG_N_W1S                             0x00c0
#define USB_IP_W1S_IP_RST_CONTROLLER_REG_N_SHIFT                  0
#define USB_IP_RST_CONTROLLER_REG_N_W1C                             0x00c4
#define USB_IP_W1C_IP_RST_CONTROLLER_REG_N_SHIFT                  0
#define USB_IP_RST_CONTROLLER_REG_N_RO                              0x00c8
#define USB_IP_RO_IP_RST_CONTROLLER_REG_N_SHIFT                   0
#define USB_IP_RO_IP_RST_CONTROLLER_REG_N_MASK                    0x00000001
#define USB_HOST_NUM_PORT                                           0x00d0
#define USB_HOST_NUM_PORT_HOST_NUM_U2_PORT_SHIFT                                       0
#define USB_HOST_NUM_PORT_HOST_NUM_U2_PORT_MASK                                        0x0000000f
#define USB_HOST_NUM_PORT_HOST_NUM_U3_PORT_SHIFT                                       4
#define USB_HOST_NUM_PORT_HOST_NUM_U3_PORT_MASK                                        0x000000f0
#define USB_PIPE_PCLK_RATE                                          0x00d4
#define USB_PIPE_PCLK_RATE_PIPE_PCLK_RATE_SHIFT                                        0
#define USB_PIPE_PCLK_RATE_PIPE_PCLK_RATE_MASK                                         0x00000007
#define USB_LOGIC_ANALYZER_TRACE_LOW                                0x00d8
#define USB_LOGIC_ANALYZER_TRACE_HIGH                               0x00dc
#define USB_USB_IDLE_SOFT_OVRD                                      0x00e0
#define USB_USB_IDLE_SOFT_OVRD_USB_IDLE_SOFT_OVRD_SHIFT                                0
#define USB_USB_IDLE_SOFT_OVRD_USB_IDLE_SOFT_OVRD_MASK                                 0x00000001
#define USB_USB_QOS_VALUE                                           0x00e4
#define USB_USB_QOS_VALUE_USB_AWQOS_VALUE_SHIFT                                        0
#define USB_USB_QOS_VALUE_USB_AWQOS_VALUE_MASK                                         0x0000000f
#define USB_USB_QOS_VALUE_USB_ARQOS_VALUE_SHIFT                                        4
#define USB_USB_QOS_VALUE_USB_ARQOS_VALUE_MASK                                         0x000000f0
#define USB_PHY_AUTORESUME_ENABLE                                   0x0100
#define USB_PHY_AUTORESUME_ENABLE_SHIFT                          0
#define USB_PHY_AUTORESUME_ENABLE_MASK                           0x00000001
#define USB_PHY_CFG_CR_CLK_SEL                                      0x0104
#define USB_PHY_CFG_CR_CLK_SEL_PHY_CFG_CR_CLK_SEL_SHIFT                                0
#define USB_PHY_CFG_CR_CLK_SEL_PHY_CFG_CR_CLK_SEL_MASK                                 0x00000001
#define USB_PHY_CFG_PLL_CPBIAS_CNTRL                                0x010c
#define USB_CNTRL_PHY_CFG_PLL_CPBIAS_CNTRL_SHIFT                    0
#define USB_CNTRL_PHY_CFG_PLL_CPBIAS_CNTRL_MASK                     0x0000007f
#define USB_CNTRL_PHY_CFG_PLL_GMP_CNTRL_SHIFT                       7
#define USB_CNTRL_PHY_CFG_PLL_GMP_CNTRL_MASK                        0x00000180
#define USB_CNTRL_PHY_CFG_PLL_INT_CNTRL_SHIFT                       9
#define USB_CNTRL_PHY_CFG_PLL_INT_CNTRL_MASK                        0x00007e00
#define USB_CNTRL_PHY_CFG_PLL_PROP_CNTRL_SHIFT                      15
#define USB_CNTRL_PHY_CFG_PLL_PROP_CNTRL_MASK                       0x001f8000
#define USB_CNTRL_PHY_CFG_PLL_VCO_CNTRL_SHIFT                       21
#define USB_CNTRL_PHY_CFG_PLL_VCO_CNTRL_MASK                        0x00e00000
#define USB_CNTRL_PHY_CFG_PLL_VREF_TUNE_SHIFT                       24
#define USB_CNTRL_PHY_CFG_PLL_VREF_TUNE_MASK                        0x03000000
#define USB_CNTRL_REF_FREQ_SEL_SHIFT                                26
#define USB_CNTRL_REF_FREQ_SEL_MASK                                 0x1c000000
#define USB_PHY_CFG_PLL_REF_DIV                                     0x0110
#define USB_REF_DIV_PHY_CFG_PLL_REF_DIV_SHIFT                              0
#define USB_REF_DIV_PHY_CFG_PLL_REF_DIV_MASK                               0x0000000f
#define USB_REF_DIV_PHY_CFG_PLL_FB_DIV_SHIFT                               4
#define USB_REF_DIV_PHY_CFG_PLL_FB_DIV_MASK                                0x0000fff0
#define USB_PHY_CFG_POR_IN_LX                                       0x011c
#define USB_PHY_CFG_POR_IN_LX_PHY_CFG_POR_IN_LX_SHIFT                                  0
#define USB_PHY_CFG_POR_IN_LX_PHY_CFG_POR_IN_LX_MASK                                   0x00000001
#define USB_PHY_CFG_RCAL_BYPASS                                     0x0120
#define USB_PHY_CFG_RCAL_BYPASS_PHY_CFG_RCAL_BYPASS_SHIFT                              0
#define USB_PHY_CFG_RCAL_BYPASS_PHY_CFG_RCAL_BYPASS_MASK                               0x00000001
#define USB_PHY_CFG_RCAL_CODE                                       0x0124
#define USB_PHY_CFG_RCAL_CODE_PHY_CFG_RCAL_CODE_SHIFT                                  0
#define USB_PHY_CFG_RCAL_CODE_PHY_CFG_RCAL_CODE_MASK                                   0x0000000f
#define USB_PHY_CFG_RCAL_CODE_PHY_CFG_RCAL_OFFSET_SHIFT                                4
#define USB_PHY_CFG_RCAL_CODE_PHY_CFG_RCAL_OFFSET_MASK                                 0x000000f0
#define USB_PHY_CFG_RXEQ_CTLE                                       0x0128
#define USB_PHY_CFG_RXEQ_CTLE_PHY_CFG_RX_EQ_CTLE_SHIFT                                 0
#define USB_PHY_CFG_RXEQ_CTLE_PHY_CFG_RX_EQ_CTLE_MASK                                  0x00000003
#define USB_PHY_CFG_RX_HS_TERM_EN                                   0x012c
#define USB_PHY_CFG_RX_HS_TERM_EN_SHIFT                          0
#define USB_PHY_CFG_RX_HS_TERM_EN_MASK                           0x00000001
#define USB_PHY_CFG_RX_HS_TUNE_SHIFT                             1
#define USB_PHY_CFG_RX_HS_TUNE_MASK                              0x0000000e
#define USB_PHY_CFG_TX_FSLS_SLEW_RATE_TUNE_SHIFT                 4
#define USB_PHY_CFG_TX_FSLS_SLEW_RATE_TUNE_MASK                  0x00000010
#define USB_PHY_CFG_TX_FSLS_VREG_BYPASS_SHIFT                    5
#define USB_PHY_CFG_TX_FSLS_VREG_BYPASS_MASK                     0x00000020
#define USB_PHY_CFG_TX_HS_DRV_OFST_SHIFT                         6
#define USB_PHY_CFG_TX_HS_DRV_OFST_MASK                          0x000000c0
#define USB_PHY_CFG_TX_HS_VREF_TUNE_SHIFT                        8
#define USB_PHY_CFG_TX_HS_VREF_TUNE_MASK                         0x00000700
#define USB_PHY_CFG_TX_HS_XV_TUNE_SHIFT                          11
#define USB_PHY_CFG_TX_HS_XV_TUNE_MASK                           0x00001800
#define USB_PHY_CFG_TX_PREEMP_TUNE_SHIFT                         13
#define USB_PHY_CFG_TX_PREEMP_TUNE_MASK                          0x0000e000
#define USB_PHY_CFG_TX_RES_TUNE_SHIFT                            16
#define USB_PHY_CFG_TX_RES_TUNE_MASK                             0x00030000
#define USB_PHY_CFG_TX_RISE_TUNE_SHIFT                           18
#define USB_PHY_CFG_TX_RISE_TUNE_MASK                            0x000c0000
#define USB_EUSB_SOFT_ENABLE                                        0x0138
#define USB_EUSB_SOFT_ENABLE_EUSBPHY_SOFT_ENABLE_SHIFT                                 0
#define USB_EUSB_SOFT_ENABLE_EUSBPHY_SOFT_ENABLE_MASK                                  0x00000001
#define USB_PHY_TX_DIG_BYPASS_SEL                                   0x013c
#define USB_SEL_PHY_TX_DIG_BYPASS_SEL_SHIFT                          0
#define USB_SEL_PHY_TX_DIG_BYPASS_SEL_MASK                           0x00000001
#define USB_PHY_RX_SE_DP_DM                                         0x0140
#define USB_PHY_RX_SE_DP_DM_PHY_RX_SE_DP_SHIFT                                         0
#define USB_PHY_RX_SE_DP_DM_PHY_RX_SE_DP_MASK                                          0x00000001
#define USB_PHY_RX_SE_DP_DM_PHY_RX_SE_DM_SHIFT                                         1
#define USB_PHY_RX_SE_DP_DM_PHY_RX_SE_DM_MASK                                          0x00000002
#define USB_PHY_TX_SE_DM_EN                                         0x0144
#define USB_PHY_TX_SE_DM_EN_PHY_TX_SE_DM_EN_SHIFT                                      0
#define USB_PHY_TX_SE_DM_EN_PHY_TX_SE_DM_EN_MASK                                       0x00000001
#define USB_PHY_TX_SE_DM_EN_PHY_TX_SE_DM_SHIFT                                         1
#define USB_PHY_TX_SE_DM_EN_PHY_TX_SE_DM_MASK                                          0x00000002
#define USB_PHY_TX_SE_DP_EN                                         0x0148
#define USB_PHY_TX_SE_DP_EN_PHY_TX_SE_DP_EN_SHIFT                                      0
#define USB_PHY_TX_SE_DP_EN_PHY_TX_SE_DP_EN_MASK                                       0x00000001
#define USB_PHY_TX_SE_DP_EN_PHY_TX_SE_DP_SHIFT                                         1
#define USB_PHY_TX_SE_DP_EN_PHY_TX_SE_DP_MASK                                          0x00000002
#define USB_TEST_BURNIN                                             0x014c
#define USB_TEST_BURNIN_TEST_BURNIN_SHIFT                                              0
#define USB_TEST_BURNIN_TEST_BURNIN_MASK                                               0x00000001
#define USB_TEST_IDDQ                                               0x0150
#define USB_TEST_IDDQ_TEST_IDDQ_SHIFT                                                  0
#define USB_TEST_IDDQ_TEST_IDDQ_MASK                                                   0x00000001
#define USB_TEST_LOOPBACK_EN                                        0x0154
#define USB_TEST_LOOPBACK_EN_TEST_LOOPBACK_EN_SHIFT                                    0
#define USB_TEST_LOOPBACK_EN_TEST_LOOPBACK_EN_MASK                                     0x00000001
#define USB_TEST_STOP_CLK_EN                                        0x0158
#define USB_TEST_STOP_CLK_EN_TEST_STOP_CLK_EN_SHIFT                                    0
#define USB_TEST_STOP_CLK_EN_TEST_STOP_CLK_EN_MASK                                     0x00000001
#define USB_UTMI_TXBITSTUFFEN                                       0x015c
#define USB_UTMI_TXBITSTUFFEN_UTMI_TXBITSTUFFEN_SHIFT                                  0
#define USB_UTMI_TXBITSTUFFEN_UTMI_TXBITSTUFFEN_MASK                                   0x00000001
#define USB_PHY_CFG_RPTR_MDOE                                       0x0160
#define USB_PHY_CFG_RPTR_MDOE_PHY_CFG_RPTR_MODE_SHIFT                                  0
#define USB_PHY_CFG_RPTR_MDOE_PHY_CFG_RPTR_MODE_MASK                                   0x00000001
#define USB_UTMI_HOSTDISCONNECT                                     0x0168
#define USB_UTMI_HOSTDISCONNECT_SHIFT                              0
#define USB_UTMI_HOSTDISCONNECT_MASK                               0x00000001
#define USB_IP_RST_EUSB_REG_W1S                                     0x016c
#define USB_IP_RST_EUSB_REG_W1S_IP_RST_EUSB_REG_SHIFT                                  0
#define USB_IP_RST_EUSB_REG_W1C                                     0x0170
#define USB_IP_RST_EUSB_REG_W1C_IP_RST_EUSB_REG_SHIFT                                  0
#define USB_IP_RST_EUSB_REG_RO                                      0x0174
#define USB_IP_RST_EUSB_REG_RO_IP_RST_EUSB_REG_SHIFT                                   0
#define USB_IP_RST_EUSB_REG_RO_IP_RST_EUSB_REG_MASK                                    0x00000001
#define USB_UTMI_XCVRSELECT                                         0x017c
#define USB_UTMI_XCVRSELECT_UTMI_XCVRSELECT_SHIFT                                      0
#define USB_UTMI_XCVRSELECT_UTMI_XCVRSELECT_MASK                                       0x00000003
#define USB_UTMI_XCVRSELECT_UTMI_TERMSELECT_SHIFT                                      2
#define USB_UTMI_XCVRSELECT_UTMI_TERMSELECT_MASK                                       0x00000004
#define USB_UTMI_XCVRSELECT_UTMI_OPMODE_SHIFT                                          3
#define USB_UTMI_XCVRSELECT_UTMI_OPMODE_MASK                                           0x00000018
#define USB_UTMI_DP_DM_PULLDOWN                                     0x0180
#define USB_UTMI_DP_DM_PULLDOWN_UTMI_DP_PULLDOWN_SHIFT                                 0
#define USB_UTMI_DP_DM_PULLDOWN_UTMI_DP_PULLDOWN_MASK                                  0x00000001
#define USB_UTMI_DP_DM_PULLDOWN_UTMI_DM_PULLDOWN_SHIFT                                 1
#define USB_UTMI_DP_DM_PULLDOWN_UTMI_DM_PULLDOWN_MASK                                  0x00000002
#define USB_UTMI_CLK_FORCE_EN                                       0x0184
#define USB_UTMI_CLK_FORCE_EN_UTMI_CLK_FORCE_EN_SHIFT                                  0
#define USB_UTMI_CLK_FORCE_EN_UTMI_CLK_FORCE_EN_MASK                                   0x00000001
#define USB_PHY0_ROM_CE_DISABLE                                     0x01b0
#define USB_DISABLE_PHY0_ROM_CE_DISABLE_SHIFT                              0
#define USB_DISABLE_PHY0_ROM_CE_DISABLE_MASK                               0x00000001
#define USB_PHY0_ROM_SD_PUDELAY                                     0x01c0
#define USB_PHY0_ROM_SD_PUDELAY_PHY0_ROM_SD_PUDELAY_SHIFT                              0
#define USB_PHY0_ROM_SD_PUDELAY_PHY0_ROM_SD_PUDELAY_MASK                               0x00000001
#define USB_PHY0_SRAM_PUDELAY                                       0x01d4
#define USB_PUDELAY_PHY0_SRAM_SD_PUDELAY_SHIFT                               0
#define USB_PUDELAY_PHY0_SRAM_SD_PUDELAY_MASK                                0x00000001
#define USB_PUDELAY_PHY0_SRAM_DSLP_PUDELAY_SHIFT                             1
#define USB_PUDELAY_PHY0_SRAM_DSLP_PUDELAY_MASK                              0x00000002
#define USB_PROTOCOL1_EXT_REF_CLK_DIV2_EN                           0x01d8
#define USB_EN_PROTOCOL1_EXT_REF_CLK_DIV2_EN_SHIFT          0
#define USB_EN_PROTOCOL1_EXT_REF_CLK_DIV2_EN_MASK           0x00000001
#define USB_PROTOCOL1_EXT_REF_RANGE                                 0x01dc
#define USB_PROTOCOL1_EXT_REF_RANGE_SHIFT                      0
#define USB_PROTOCOL1_EXT_REF_RANGE_MASK                       0x00000007
#define USB_PROTOCOL1_EXT_TX_EQ_MAIN_G1_G2                          0x01e0
#define USB_EXT_TX_EQ_MAIN_G1_SHIFT           0
#define USB_EXT_TX_EQ_MAIN_G1_MASK            0x00000fff
#define USB_EXT_TX_EQ_MAIN_G2_SHIFT           12
#define USB_EXT_TX_EQ_MAIN_G2_MASK            0x00fff000
#define USB_PROTOCOL1_EXT_TX_EQ_OVRD_G1_G2                          0x01e4
#define USB_PROTOCOL1_EXT_TX_EQ_OVRD_G1_SHIFT           0
#define USB_PROTOCOL1_EXT_TX_EQ_OVRD_G1_MASK            0x00000003
#define USB_PROTOCOL1_EXT_TX_EQ_OVRD_G2_SHIFT           2
#define USB_PROTOCOL1_EXT_TX_EQ_OVRD_G2_MASK            0x0000000c
#define USB_PROTOCOL1_EXT_TX_EQ_POST_G1_G2                          0x01e8
#define USB_PROTOCOL1_EXT_TX_EQ_POST_G1_SHIFT           0
#define USB_PROTOCOL1_EXT_TX_EQ_POST_G1_MASK            0x00000fff
#define USB_PROTOCOL1_EXT_TX_EQ_POST_G2_SHIFT           12
#define USB_PROTOCOL1_EXT_TX_EQ_POST_G2_MASK            0x00fff000
#define USB_PROTOCOL1_EXT_TX_EQ_PRE_G1_G2                           0x01ec
#define USB_PROTOCOL1_EXT_TX_EQ_PRE_G1_SHIFT             0
#define USB_PROTOCOL1_EXT_TX_EQ_PRE_G1_MASK              0x00000fff
#define USB_PROTOCOL1_EXT_TX_EQ_PRE_G2_SHIFT             12
#define USB_PROTOCOL1_EXT_TX_EQ_PRE_G2_MASK              0x00fff000
#define USB_PROTOCOL3_EXT_REF_CLK_DIV2_EN                           0x01f0
#define USB_PROTOCOL3_EXT_REF_CLK_DIV2_EN_SHIFT          0
#define USB_PROTOCOL3_EXT_REF_CLK_DIV2_EN_MASK           0x00000001
#define USB_PROTOCOL3_EXT_REF_RANGE_SHIFT                1
#define USB_PROTOCOL3_EXT_REF_RANGE_MASK                 0x0000000e
#define USB_PROTOCOL7_EXT_REF_CLK_DIV2_EN_SHIFT          4
#define USB_PROTOCOL7_EXT_REF_CLK_DIV2_EN_MASK           0x00000010
#define USB_PROTOCOL7_EXT_REF_RANGE_SHIFT                5
#define USB_PROTOCOL7_EXT_REF_RANGE_MASK                 0x000000e0
#define USB_PROTOCOL9_EXT_REF_CLK_DIV2_EN_SHIFT          8
#define USB_PROTOCOL9_EXT_REF_CLK_DIV2_EN_MASK           0x00000100
#define USB_PROTOCOL9_EXT_REF_RANGE_SHIFT                9
#define USB_PROTOCOL9_EXT_REF_RANGE_MASK                 0x00000e00
#define USB_PHY0_MPLLA_SSC_EN                                       0x01f4
#define USB_PHY0_MPLLA_SSC_EN_PHY0_MPLLA_SSC_EN_SHIFT                                  0
#define USB_PHY0_MPLLA_SSC_EN_PHY0_MPLLA_SSC_EN_MASK                                   0x00000001
#define USB_PHY0_MPLLA_STATE_SYNC_CLK_EN                            0x01f8
#define USB_PHY0_MPLLA_STATE_SHIFT                        0
#define USB_PHY0_MPLLA_STATE_MASK                         0x00000001
#define USB_PHY0_MPLLA_WORD_SYNC_CLK_EN_SHIFT             1
#define USB_PHY0_MPLLA_WORD_SYNC_CLK_EN_MASK              0x00000002
#define USB_PHY0_MPLLA_FORCE_ACK_SHIFT                    2
#define USB_PHY0_MPLLA_FORCE_ACK_MASK                     0x00000004
#define USB_PHY0_REF_ALT_CLK_LP_SEL                                 0x020c
#define USB_PHY0_REF_ALT_CLK_LP_SEL_SHIFT                      0
#define USB_PHY0_REF_ALT_CLK_LP_SEL_MASK                       0x00000001
#define USB_PHY0_REF_CLKDET_EN_SHIFT                           1
#define USB_PHY0_REF_CLKDET_EN_MASK                            0x00000002
#define USB_PHY0_REF_CLKDET_RESULT_SHIFT                       2
#define USB_PHY0_REF_CLKDET_RESULT_MASK                        0x00000004
#define USB_PHY0_REF_PRE_VREG_BYPASS_SHIFT                     3
#define USB_PHY0_REF_PRE_VREG_BYPASS_MASK                      0x00000008
#define USB_PHY_TEST_MODE                                           0x0224
#define USB_PHY_TEST_MODE_PHY_TEST_BURNIN_SHIFT                                        0
#define USB_PHY_TEST_MODE_PHY_TEST_BURNIN_MASK                                         0x00000001
#define USB_PHY_TEST_MODE_PHY_TEST_POWERDOWN_SHIFT                                     1
#define USB_PHY_TEST_MODE_PHY_TEST_POWERDOWN_MASK                                      0x00000002
#define USB_PHY_TEST_MODE_PHY_TEST_STOP_CLK_EN_SHIFT                                   2
#define USB_PHY_TEST_MODE_PHY_TEST_STOP_CLK_EN_MASK                                    0x00000004
#define USB_PHY_TEST_MODE_PHY_TEST_TX_REF_CLK_EN_SHIFT                                 3
#define USB_PHY_TEST_MODE_PHY_TEST_TX_REF_CLK_EN_MASK                                  0x00000008
#define USB_PHY_PG_MODE_EN                                          0x0228
#define USB_PHY_PG_MODE_EN_PHY_PG_MODE_EN_SHIFT                                        0
#define USB_PHY_PG_MODE_EN_PHY_PG_MODE_EN_MASK                                         0x00000001
#define USB_PHY0_BS_ACMODE_ACTEST                                   0x022c
#define USB_PHY0_BS_ACMODE_ACTEST_PHY_BS_ACMODE_SHIFT                                  0
#define USB_PHY0_BS_ACMODE_ACTEST_PHY_BS_ACMODE_MASK                                   0x00000001
#define USB_PHY0_BS_ACMODE_ACTEST_PHY0_BS_ACTEST_SHIFT                                 1
#define USB_PHY0_BS_ACMODE_ACTEST_PHY0_BS_ACTEST_MASK                                  0x00000002
#define USB_PHY0_REF_USE_PAD                                        0x0230
#define USB_PHY0_REF_USE_PAD_PHY0_REF_USE_PAD_SHIFT                                    0
#define USB_PHY0_REF_USE_PAD_PHY0_REF_USE_PAD_MASK                                     0x00000001
#define USB_PHY0_RES_EXT_EN                                         0x0234
#define USB_PHY0_RES_EXT_EN_PHY0_RES_EXT_EN_SHIFT                                      0
#define USB_PHY0_RES_EXT_EN_PHY0_RES_EXT_EN_MASK                                       0x00000001
#define USB_PHY_EXT_REF_OVRD_SEL                                    0x0238
#define USB_PHY_PHY_EXT_REF_OVRD_SEL_SHIFT                            0
#define USB_PHY_PHY_EXT_REF_OVRD_SEL_MASK                             0x00000001
#define USB_PHY_PHY0_RES_EXT_RCAL_SHIFT                               1
#define USB_PHY_PHY0_RES_EXT_RCAL_MASK                                0x0000007e
#define USB_PHY_LANE0_POWER_PRESENT                                 0x023c
#define USB_PHY_LANE0_POWER_PRESENT_SHIFT                      0
#define USB_PHY_LANE0_POWER_PRESENT_MASK                       0x00000001
#define USB_PHY_LANE0_RX2TX_PAR_LB_EN_SHIFT                    1
#define USB_PHY_LANE0_RX2TX_PAR_LB_EN_MASK                     0x00000002
#define USB_PHY_LANE1_POWER_PRESENT_SHIFT                      2
#define USB_PHY_LANE1_POWER_PRESENT_MASK                       0x00000004
#define USB_PHY_LANE3_RX2TX_PAR_LB_EN_SHIFT                    3
#define USB_PHY_LANE3_RX2TX_PAR_LB_EN_MASK                     0x00000008
#define USB_PHY_RTUNE_REQ                                           0x0240
#define USB_PHY_RTUNE_REQ_PHY_RTUNE_REQ_SHIFT                                          0
#define USB_PHY_RTUNE_REQ_PHY_RTUNE_REQ_MASK                                           0x00000001
#define USB_PHY_RTUNE_REQ_PHY_RTUNE_ACK_SHIFT                                          1
#define USB_PHY_RTUNE_REQ_PHY_RTUNE_ACK_MASK                                           0x00000002
#define USB_PHY_RX1_TERM_ACDC                                       0x0244
#define USB_PHY_RX1_TERM_ACDC_PHY_RX1_TERM_ACDC_SHIFT                                  0
#define USB_PHY_RX1_TERM_ACDC_PHY_RX1_TERM_ACDC_MASK                                   0x00000001
#define USB_PHY_RX1_TERM_ACDC_PHY_RX2_TERM_ACDC_SHIFT                                  1
#define USB_PHY_RX1_TERM_ACDC_PHY_RX2_TERM_ACDC_MASK                                   0x00000002
#define USB_USB32_PIPE_LANE0_LINK_NUM                               0x0248
#define USB_USB32_PIPE_LANE0_LINK_NUM_SHIFT                  0
#define USB_USB32_PIPE_LANE0_LINK_NUM_MASK                   0x0000000f
#define USB_USB32_PIPE_LANE0_MAXPCLK_DIV_RATIO                      0x024c
#define USB_USB32_PIPE_LANE0_MAXPCLK_DIV_RATIO_SHIFT 0
#define USB_USB32_PIPE_LANE0_MAXPCLK_DIV_RATIO_MASK  0x0000000f
#define USB_PHY0_APB0_IF_MODE                                       0x0250
#define USB_PHY0_APB0_IF_MODE_PHY0_APB0_IF_MODE_SHIFT                                  0
#define USB_PHY0_APB0_IF_MODE_PHY0_APB0_IF_MODE_MASK                                   0x00000003
#define USB_USB32_PIPE_LANE0_DISABLE                                0x0254
#define USB_DISABLE_USB32_PIPE_LANE0_DISABLE_SHIFT                    0
#define USB_DISABLE_USB32_PIPE_LANE0_DISABLE_MASK                     0x00000001
#define USB_USB32_PIPE_LAN0_TX2RX_LOOPBK                            0x0258
#define USB_TX2RX_LOOPBK_USB32_PIPE_LAN0_TX2RX_LOOPBK_SHIFT            0
#define USB_TX2RX_LOOPBK_USB32_PIPE_LAN0_TX2RX_LOOPBK_MASK             0x00000001
#define USB_USB32_PIPE_RX0_IF_WIDTH                                 0x025c
#define USB_WIDTH_USB32_PIPE_RX0_IF_WIDTH_SHIFT                      0
#define USB_WIDTH_USB32_PIPE_RX0_IF_WIDTH_MASK                       0x00000003
#define USB_PHY0_REF_REPEAT_CLK_EN                                  0x0260
#define USB_EN_PHY0_REF_REPEAT_CLK_EN_SHIFT                        0
#define USB_EN_PHY0_REF_REPEAT_CLK_EN_MASK                         0x00000001
#define USB_UPCS_PIPE_CONFIG                                        0x0264
#define USB_UPCS_PIPE_CONFIG_UPCS_PIPE_CONFIG_SHIFT                                    0
#define USB_UPCS_PIPE_CONFIG_UPCS_PIPE_CONFIG_MASK                                     0x0000ffff
#define USB_PHY0_MPLLA_FORCE_EN                                     0x0268
#define USB_PHY0_MPLLA_FORCE_EN_PHY0_MPLLA_FORCE_EN_SHIFT                              0
#define USB_PHY0_MPLLA_FORCE_EN_PHY0_MPLLA_FORCE_EN_MASK                               0x00000001
#define USB_PHY0_TEST_FLYOVER_EN                                    0x0270
#define USB_PHY0_TEST_FLYOVER_EN_PHY0_TEST_FLYOVER_EN_SHIFT                            0
#define USB_PHY0_TEST_FLYOVER_EN_PHY0_TEST_FLYOVER_EN_MASK                             0x00000001
#define USB_USB32_PIPE_LANE0_MAXPCLKREQ                             0x0280
#define USB_USB32_PIPE_LANE0_MAXPCLKREQ_SHIFT              0
#define USB_USB32_PIPE_LANE0_MAXPCLKREQ_MASK               0x00000003
#define USB_USB32_PIPE_LANE0_MAXPCLKACK_SHIFT              2
#define USB_USB32_PIPE_LANE0_MAXPCLKACK_MASK               0x0000000c
#define USB_SS_RXDET_DISABLE_ACK_1                                  0x0284
#define USB_ACK_1_SS_RXDET_DISABLE_ACK_1_SHIFT                        0
#define USB_ACK_1_SS_RXDET_DISABLE_ACK_1_MASK                         0x00000001
#define USB_SS_RXDET_DISABLE_1_TCA                                  0x0288
#define USB_1_TCA_SS_RXDET_DISABLE_1_SHIFT                            0
#define USB_1_TCA_SS_RXDET_DISABLE_1_MASK                             0x00000001
#define USB_1_TCA_TCA_VBUSVALID_SHIFT                                 1
#define USB_1_TCA_TCA_VBUSVALID_MASK                                  0x00000002
#define USB_1_TCA_PHY_RES_ACK_OUT_SHIFT                               2
#define USB_1_TCA_PHY_RES_ACK_OUT_MASK                                0x00000004
#define USB_1_TCA_ST_APB_CLK_GATE_SHIFT                               3
#define USB_1_TCA_ST_APB_CLK_GATE_MASK                                0x00000008
#define USB_1_TCA_ST_PCS_LANE0_REF_CLK_GATE_SHIFT                     4
#define USB_1_TCA_ST_PCS_LANE0_REF_CLK_GATE_MASK                      0x00000010
#define USB_1_TCA_ST_PCS_LANE1_REF_CLK_GATE_SHIFT                     5
#define USB_1_TCA_ST_PCS_LANE1_REF_CLK_GATE_MASK                      0x00000020
#define USB_1_TCA_ST_LANE0_ES_WR_CLK_GATE_SHIFT                       6
#define USB_1_TCA_ST_LANE0_ES_WR_CLK_GATE_MASK                        0x00000040
#define USB_1_TCA_ST_LANE1_ES_WR_CLK_GATE_SHIFT                       7
#define USB_1_TCA_ST_LANE1_ES_WR_CLK_GATE_MASK                        0x00000080
#define USB_1_TCA_ST_PCS_LANE0_PCS_NONSDS_CLK_GATE_SHIFT              8
#define USB_1_TCA_ST_PCS_LANE0_PCS_NONSDS_CLK_GATE_MASK               0x00000100
#define USB_1_TCA_ST_PCS_LANE1_PCS_NONSDS_CLK_GATE_SHIFT              9
#define USB_1_TCA_ST_PCS_LANE1_PCS_NONSDS_CLK_GATE_MASK               0x00000200
#define USB_1_TCA_USB32_PIPE_LANE0_DATABUSWIDTH_SHIFT                 10
#define USB_1_TCA_USB32_PIPE_LANE0_DATABUSWIDTH_MASK                  0x00000c00
#define USB_1_TCA_PHY_RX2_PPM_DRIFT_VLD_SHIFT                         12
#define USB_1_TCA_PHY_RX2_PPM_DRIFT_VLD_MASK                          0x00001000
#define USB_1_TCA_PHY_RX2_PPM_DRIFT_SHIFT                             13
#define USB_1_TCA_PHY_RX2_PPM_DRIFT_MASK                              0x0007e000
#define USB_1_TCA_PHY_RX1_PPM_DRIFT_VLD_SHIFT                         19
#define USB_1_TCA_PHY_RX1_PPM_DRIFT_VLD_MASK                          0x00080000
#define USB_1_TCA_PHY_RX1_PPM_DRIFT_SHIFT                             20
#define USB_1_TCA_PHY_RX1_PPM_DRIFT_MASK                              0x03f00000
#define USB_EXT_PCLK_REQ                                            0x028c
#define USB_EXT_PCLK_REQ_EXT_PCLK_REQ_SHIFT                                            0
#define USB_EXT_PCLK_REQ_EXT_PCLK_REQ_MASK                                             0x00000001
#define USB_IP_RST_COMBOPHY_REG_W1S                                 0x0290
#define USB_W1S_IP_RST_COMBOPHY_REG_SHIFT                          0
#define USB_W1C                                 0x0294
#define USB_W1C_IP_RST_COMBOPHY_REG_SHIFT                          0
#define USB_RO                                  0x0298
#define USB_RO_IP_RST_COMBOPHY_REG_SHIFT                           0
#define USB_RO_IP_RST_COMBOPHY_REG_MASK                            0x00000001
#define USB_PHY0_PCS_PWR_EN                                         0x02a0
#define USB_PHY0_PCS_PWR_EN_PHY0_PCS_PWR_EN_SHIFT                                      0
#define USB_PHY0_PCS_PWR_EN_PHY0_PCS_PWR_EN_MASK                                       0x00000001
#define USB_PHY0_PCS_PWR_EN_PHY0_PMA_PWR_EN_SHIFT                                      1
#define USB_PHY0_PCS_PWR_EN_PHY0_PMA_PWR_EN_MASK                                       0x00000002
#define USB_PHY0_PCS_PWR_EN_UPCS_PWR_EN_SHIFT                                          2
#define USB_PHY0_PCS_PWR_EN_UPCS_PWR_EN_MASK                                           0x00000004
#define USB_PHY0_PCS_PWR_STABLE                                     0x02a4
#define USB_STABLE_PHY0_PCS_PWR_STABLE_SHIFT                              0
#define USB_STABLE_PHY0_PCS_PWR_STABLE_MASK                               0x00000001
#define USB_STABLE_UPCS_PWR_STABLE_SHIFT                                  1
#define USB_STABLE_UPCS_PWR_STABLE_MASK                                   0x00000002
#define USB_PHY_SRAM_EXT_DONE_BYPASS_MODE                           0x02a8
#define USB_MODE_PHY_SRAM_EXT_LD_DONE_SHIFT                   0
#define USB_MODE_PHY_SRAM_EXT_LD_DONE_MASK                    0x00000001
#define USB_MODE_PHY0_SRAM_BYPASS_MODE_SHIFT                  1
#define USB_MODE_PHY0_SRAM_BYPASS_MODE_MASK                   0x00000006
#define USB_MODE_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_SHIFT         3
#define USB_MODE_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_MASK          0x00000018
#define USB_PHY_SRAM_INIT_DONE                                      0x02ac
#define USB_PHY_SRAM_INIT_DONE_PHY_SRAM_INIT_DONE_SHIFT                                0
#define USB_PHY_SRAM_INIT_DONE_PHY_SRAM_INIT_DONE_MASK                                 0x00000001
#define USB_COMBOPHY_SS_ACTIVE                                      0x02b0
#define USB_COMBOPHY_SS_ACTIVE_SS_LANE0_ACTIVE_SHIFT                                   0
#define USB_COMBOPHY_SS_ACTIVE_SS_LANE0_ACTIVE_MASK                                    0x00000001
#define USB_COMBOPHY_SS_ACTIVE_SS_LANE1_ACTIVE_SHIFT                                   1
#define USB_COMBOPHY_SS_ACTIVE_SS_LANE1_ACTIVE_MASK                                    0x00000002
#define USB_TYPEC_FLIP_INVERT                                       0x02b4
#define USB_TYPEC_FLIP_INVERT_TYPEC_FLIP_INVERT_SHIFT                                  0
#define USB_TYPEC_FLIP_INVERT_TYPEC_FLIP_INVERT_MASK                                   0x00000001
#define USB_HOST_LEGACY_SMI_PCI                                     0x02b8
#define USB_HOST_LEGACY_SMI_PCI_CMD_REG_WR_SHIFT                   0
#define USB_HOST_LEGACY_SMI_PCI_CMD_REG_WR_MASK                    0x00000001
#define USB_HOST_LEGACY_SMI_BAR_WR_SHIFT                           1
#define USB_HOST_LEGACY_SMI_BAR_WR_MASK                            0x00000002
#define USB_USB_COMBOPHY_BYPASS                                     0x02bc
#define USB_USB_EUSB_COMBO_DIV_EN_SHIFT                            0
#define USB_USB_EUSB_COMBO_DIV_EN_MASK                             0x00000001
#define USB_USB2_ONLY_MODE_ENABLE_SHIFT                            1
#define USB_USB2_ONLY_MODE_ENABLE_MASK                             0x00000002
#define USB_COMBOPHY_PIPE_LANE0_RESET_REG_SEL                       0x02e0
#define USB_SEL_COMBOPHY_PIPE_LANE0_RESET_REG_SEL_SHIFT  0
#define USB_SEL_COMBOPHY_PIPE_LANE0_RESET_REG_SEL_MASK   0x00000001
#define USB_SEL_USB32_PIPE_LANE0_CLK_SEL_SHIFT           1
#define USB_SEL_USB32_PIPE_LANE0_CLK_SEL_MASK            0x00000002
#define USB_COMBOPHY_PIPE_LANE0_RESET_REG_N_W1S                     0x02e4
#define USB_W1S_COMBOPHY_PIPE_LANE0_RESET_REG_N_SHIFT  0
#define USB_COMBOPHY_PIPE_LANE0_RESET_REG_N_W1C                     0x02e8
#define USB_W1C_COMBOPHY_PIPE_LANE0_RESET_REG_N_SHIFT  0
#define USB_COMBOPHY_PIPE_LANE0_RESET_REG_N_RO                      0x02ec
#define USB_COMBOPHY_PIPE_LANE0_RESET_REG_N_SHIFT   0
#define USB_COMBOPHY_PIPE_LANE0_RESET_REG_N_MASK    0x00000001
#define USB_FPGA_USB_RESET_WIDTH_SEL                                0x02f0
#define USB_FPGA_USB_RESET_SHIFT                              0
#define USB_FPGA_USB_RESET_MASK                               0x00000001
#define USB_FPGA_USB_DATA16_8_SHIFT                           1
#define USB_FPGA_USB_DATA16_8_MASK                            0x00000002
#define USB_FPGA_USB_UNI_BIDI                                       0x02f4
#define USB_BIDI_FPGA_USB_UNI_BIDI_SHIFT                                  0
#define USB_BIDI_FPGA_USB_UNI_BIDI_MASK                                   0x00000001
#define USB_FPGA_IDPULLUP                                           0x02f8
#define USB_FPGA_IDPULLUP_FPGA_IDPULLUP_SHIFT                                          0
#define USB_FPGA_IDPULLUP_FPGA_IDPULLUP_MASK                                           0x00000001
#define USB_FPGA_IDDIG                                              0x02fc
#define USB_FPGA_IDDIG_FPGA_IDDIG_SHIFT                                                0
#define USB_FPGA_IDDIG_FPGA_IDDIG_MASK                                                 0x00000001
#define USB_COMBOPHY_DEBUG_LANE0_0                                  0x0300
#define USB_0_LANE0_CFG_PIPE_CTL_CLR_SHIFT                        0
#define USB_0_LANE0_CFG_PIPE_CTL_CLR_MASK                         0x00000001
#define USB_0_LANE0_CFG_PIPE_CTL_ST0_SHIFT                        1
#define USB_0_LANE0_CFG_PIPE_CTL_ST0_MASK                         0x0000003e
#define USB_0_LANE0_CFG_PIPE_CTL_ST1_SHIFT                        6
#define USB_0_LANE0_CFG_PIPE_CTL_ST1_MASK                         0x000007c0
#define USB_0_LANE0_CFG_PIPE_LOAD_SHIFT                           11
#define USB_0_LANE0_CFG_PIPE_LOAD_MASK                            0x00000800
#define USB_COMBOPHY_DEBUG_LANE0_1                                  0x0304
#define USB_LANE0_PIPE_CTL_ST_SHIFT                             0
#define USB_LANE0_PIPE_CTL_ST_MASK                              0x000fffff
#define USB_COMBOPHY_DEBUG_LANE0_2                                  0x0308
#define USB_COMBOPHY_DEBUG_LANE1_0                                  0x030c
#define USB_LANE1_CFG_PIPE_CTL_CLR_SHIFT                        0
#define USB_LANE1_CFG_PIPE_CTL_CLR_MASK                         0x00000001
#define USB_LANE1_CFG_PIPE_CTL_ST0_SHIFT                        1
#define USB_LANE1_CFG_PIPE_CTL_ST0_MASK                         0x0000003e
#define USB_LANE1_CFG_PIPE_CTL_ST1_SHIFT                        6
#define USB_LANE1_CFG_PIPE_CTL_ST1_MASK                         0x000007c0
#define USB_LANE1_CFG_PIPE_LOAD_SHIFT                           11
#define USB_LANE1_CFG_PIPE_LOAD_MASK                            0x00000800
#define USB_COMBOPHY_DEBUG_LANE1_1                                  0x0310
#define USB_COMBOPHY_DEBUG_LANE1_1_LANE1_PIPE_CTL_ST_SHIFT                             0
#define USB_COMBOPHY_DEBUG_LANE1_1_LANE1_PIPE_CTL_ST_MASK                              0x000fffff
#define USB_COMBOPHY_DEBUG_LANE1_2                                  0x0314
#define USB_LANE1_SYM_CODE_ERR_CNT0                                 0x031c
#define USB_LANE1_SYM_CODE_ERR_CNT1                                 0x0320
#define USB_LANE1_SYM_DISP_ERR_CNT0                                 0x0324
#define USB_LANE1_SYM_DISP_ERR_CNT1                                 0x0328
#define USB_LANE1_CFG_CLR_LOAD                                      0x032c
#define USB_LANE1_LANE1_CFG_DISP_ERR_CLR_SHIFT                            0
#define USB_LANE1_LANE1_CFG_DISP_ERR_CLR_MASK                             0x00000001
#define USB_LANE1_LANE1_CFG_CODE_ERR_CLR_SHIFT                            1
#define USB_LANE1_LANE1_CFG_CODE_ERR_CLR_MASK                             0x00000002
#define USB_LANE1_LANE1_CFG_ERR_CNT_LOAD_SHIFT                            2
#define USB_LANE1_LANE1_CFG_ERR_CNT_LOAD_MASK                             0x00000004
#define USB_LANE0_SYM_CODE_ERR_CNT0                                 0x0330
#define USB_LANE0_SYM_CODE_ERR_CNT1                                 0x0334
#define USB_LANE0_SYM_DISP_ERR_CNT0                                 0x0338
#define USB_LANE0_SYM_DISP_ERR_CNT1                                 0x033c
#define USB_LANE0_CFG_CLR_LOAD                                      0x0340
#define USB_LANE0_LANE0_CFG_DISP_ERR_CLR_SHIFT                            0
#define USB_LANE0_LANE0_CFG_DISP_ERR_CLR_MASK                             0x00000001
#define USB_LANE0_LANE0_CFG_CODE_ERR_CLR_SHIFT                            1
#define USB_LANE0_LANE0_CFG_CODE_ERR_CLR_MASK                             0x00000002
#define USB_LANE0_LANE0_CFG_ERR_CNT_LOAD_SHIFT                            2
#define USB_LANE0_LANE0_CFG_ERR_CNT_LOAD_MASK                             0x00000004
#define USB_LANE1_WR_STATUS                                         0x0344
#define USB_LANE1_RD_STATUS                                         0x0348
#define USB_LANE1_TP                                                0x034c
#define USB_LANE0_WR_STATUS                                         0x0350
#define USB_LANE0_RD_STATUS                                         0x0354
#define USB_LANE0_TP                                                0x0358
#define USB_LANE1_ALIGN_DS                                          0x035c
#define USB_LANE1_SYNC_ALIGN_DEBUG_SHIFT                                0
#define USB_LANE1_SYNC_ALIGN_DEBUG_MASK                                 0x000fffff
#define USB_LANE1_SYNC_ALIGN_STATUS_SHIFT                               20
#define USB_LANE1_SYNC_ALIGN_STATUS_MASK                                0x01f00000
#define USB_LANE0_ALIGN_DS                                          0x0360
#define USB_LANE0_SYNC_ALIGN_DEBUG_SHIFT                                0
#define USB_LANE0_SYNC_ALIGN_DEBUG_MASK                                 0x000fffff
#define USB_LANE0_SYNC_ALIGN_STATUS_SHIFT                               20
#define USB_LANE0_SYNC_ALIGN_STATUS_MASK                                0x01f00000
#define USB_LANE1_SYMBOL_DS                                         0x0364
#define USB_LANE1_SYMBOL_ALIGN_DEBUG_SHIFT                             0
#define USB_LANE1_SYMBOL_ALIGN_DEBUG_MASK                              0x000007ff
#define USB_LANE1_SYMBOL_ALIGN_STATUS_SHIFT                            11
#define USB_LANE1_SYMBOL_ALIGN_STATUS_MASK                             0x003ff800
#define USB_LANE1_SYMBOL_CFG                                        0x0368
#define USB_LANE1_CFG_SYM_STS_CLR_SHIFT                               0
#define USB_LANE1_CFG_SYM_STS_CLR_MASK                                0x00000001
#define USB_LANE1_CFG_SYM_CAPUTRE1_SHIFT                              1
#define USB_LANE1_CFG_SYM_CAPUTRE1_MASK                               0x0000000e
#define USB_LANE1_CFG_SYM_CAPUTRE0_SHIFT                              4
#define USB_LANE1_CFG_SYM_CAPUTRE0_MASK                               0x00000070
#define USB_LANE1_CFG_SYMBOL_CTL_LOAD_SHIFT                           7
#define USB_LANE1_CFG_SYMBOL_CTL_LOAD_MASK                            0x00000080
#define USB_LANE0_SYMBOL_DS                                         0x036c
#define USB_LANE0_SYMBOL_ALIGN_DEBUG_SHIFT                             0
#define USB_LANE0_SYMBOL_ALIGN_DEBUG_MASK                              0x000007ff
#define USB_LANE0_SYMBOL_ALIGN_STATUS_SHIFT                            11
#define USB_LANE0_SYMBOL_ALIGN_STATUS_MASK                             0x003ff800
#define USB_LANE0_SYMBOL_CFG                                        0x0370
#define USB_LANE0_CFG_SYM_STS_CLR_SHIFT                               0
#define USB_LANE0_CFG_SYM_STS_CLR_MASK                                0x00000001
#define USB_LANE0_CFG_SYM_CAPUTRE1_SHIFT                              1
#define USB_LANE0_CFG_SYM_CAPUTRE1_MASK                               0x0000000e
#define USB_LANE0_CFG_SYM_CAPUTRE0_SHIFT                              4
#define USB_LANE0_CFG_SYM_CAPUTRE0_MASK                               0x00000070
#define USB_LANE0_CFG_SYMBOL_CTL_LOAD_SHIFT                           7
#define USB_LANE0_CFG_SYMBOL_CTL_LOAD_MASK                            0x00000080
#define USB_LANE0_CFG_SYMBOL_CFG                                    0x0374
#define USB_LANE0_CFG_SYNC_STS_CLR_SHIFT                          0
#define USB_LANE0_CFG_SYNC_STS_CLR_MASK                           0x00000001
#define USB_LANE0_CFG_SYNC_CAPUTRE0_SHIFT                         1
#define USB_LANE0_CFG_SYNC_CAPUTRE0_MASK                          0x0000000e
#define USB_LANE0_CFG_SYNC_CAPUTRE1_SHIFT                         4
#define USB_LANE0_CFG_SYNC_CAPUTRE1_MASK                          0x00000070
#define USB_LANE0_CFG_SYNC_CTL_LOAD_SHIFT                         7
#define USB_LANE0_CFG_SYNC_CTL_LOAD_MASK                          0x00000080
#define USB_LANE1_CFG_SYMBOL_CFG                                    0x0378
#define USB_LANE1_CFG_SYNC_STS_CLR_SHIFT                          0
#define USB_LANE1_CFG_SYNC_STS_CLR_MASK                           0x00000001
#define USB_LANE1_CFG_SYNC_CAPUTRE0_SHIFT                         1
#define USB_LANE1_CFG_SYNC_CAPUTRE0_MASK                          0x0000000e
#define USB_LANE1_CFG_SYNC_CAPUTRE1_SHIFT                         4
#define USB_LANE1_CFG_SYNC_CAPUTRE1_MASK                          0x00000070
#define USB_LANE1_CFG_SYNC_CTL_LOAD_SHIFT                         7
#define USB_LANE1_CFG_SYNC_CTL_LOAD_MASK                          0x00000080
#define USB_LANE0_BLK_DISP                                          0x037c
#define USB_LANE0_BLK_DISP_ERR_CLR_SHIFT                                0
#define USB_LANE0_BLK_DISP_ERR_CLR_MASK                                 0x00000001
#define USB_LANE0_BLK_CODE_ERR_CLR_SHIFT                                1
#define USB_LANE0_BLK_CODE_ERR_CLR_MASK                                 0x00000002
#define USB_LANE0_BLK_ERR_CNT_LOAD_SHIFT                                2
#define USB_LANE0_BLK_ERR_CNT_LOAD_MASK                                 0x00000004
#define USB_LANE1_BLK_DISP                                          0x0380
#define USB_LANE1_BLK_DISP_ERR_CLR_SHIFT                                0
#define USB_LANE1_BLK_DISP_ERR_CLR_MASK                                 0x00000001
#define USB_LANE1_BLK_CODE_ERR_CLR_SHIFT                                1
#define USB_LANE1_BLK_CODE_ERR_CLR_MASK                                 0x00000002
#define USB_LANE1_BLK_ERR_CNT_LOAD_SHIFT                                2
#define USB_LANE1_BLK_ERR_CNT_LOAD_MASK                                 0x00000004
#define USB_LANE0_BLK_DISP_STATUS                                   0x0384
#define USB_LANE0_BLK_CODE_STATUS                                   0x0388
#define USB_LANE0_BLK_DET_ERR                                       0x038c
#define USB_LANE1_BLK_DISP_STATUS                                   0x0390
#define USB_LANE1_BLK_CODE_STATUS                                   0x0394
#define USB_LANE1_BLK_DET_ERR                                       0x0398
#define USB_PMA_PWR_EN_OVRD                                         0x039c
#define USB_PMA_PWR_EN_OVRD_PMA_PWR_EN_OVRD_SHIFT                                      0
#define USB_PMA_PWR_EN_OVRD_PMA_PWR_EN_OVRD_MASK                                       0x00000001
#define USB_FLADJ_30MHZ_REG                                         0x03a0
#define USB_FLADJ_30MHZ_REG_FLADJ_30MHZ_REG_SHIFT                                      0
#define USB_FLADJ_30MHZ_REG_FLADJ_30MHZ_REG_MASK                                       0x0000003f
#define USB_PTM_TIME                                                0x03a4
#define USB_PTM_TIME_VLD                                            0x03a8
#define USB_PTM_TIME_VLD_PTM_TIME_VLD_SHIFT                                            0
#define USB_PTM_TIME_VLD_PTM_TIME_VLD_MASK                                             0x00000001
#define USB_TCA_DP4_POR                                             0x03ac
#define USB_TCA_DP4_POR_TCA_DP4_POR_SHIFT                                              0
#define USB_TCA_DP4_POR_TCA_DP4_POR_MASK                                               0x00000001
#define USB_DEBUG_RST_W1S                                           0x03b0
#define USB_DEBUG_RST_W1S_IP_RST_COMBOPHY_DEBUG_SHIFT                                  0
#define USB_DEBUG_RST_W1C                                           0x03b4
#define USB_DEBUG_RST_W1C_IP_RST_COMBOPHY_DEBUG_SHIFT                                  0
#define USB_DEBUG_RST_RO                                            0x03b8
#define USB_DEBUG_RST_RO_IP_RST_COMBOPHY_DEBUG_SHIFT                                   0
#define USB_DEBUG_RST_RO_IP_RST_COMBOPHY_DEBUG_MASK                                    0x00000001
#define USB_DEBUG_CLK_GATE_W1S                                      0x03bc
#define USB_DEBUG_IP_CLK_COMBOPHY_DEBUG_GATE_SHIFT                        0
#define USB_DEBUG_CLK_GATE_W1C                                      0x03c0
#define USB_DEBUG_IP_CLK_COMBOPHY_DEBUG_GATE_SHIFT                        0
#define USB_DEBUG_CLK_GATE_RO                                       0x03c4
#define USB_DEBUG_CLK_COMBOPHY_DEBUG_GATE_SHIFT                         0
#define USB_DEBUG_IP_CLK_COMBOPHY_DEBUG_GATE_MASK                          0x00000001
#define USB_USB_DEBUG_CTRL_RST_W1S                                  0x03d8
#define USB_CTRL_RST_W1S_IP_RST_USBC_DEBUG_SHIFT                             0
#define USB_CTRL_RST_W1C                                  0x03dc
#define USB_CTRL_RST_W1C_IP_RST_USBC_DEBUG_SHIFT                             0
#define USB_CTRL_RST_RO                                   0x03e0
#define USB_CTRL_RST_RO_IP_RST_USBC_DEBUG_SHIFT                              0
#define USB_CTRL_RST_RO_IP_RST_USBC_DEBUG_MASK                               0x00000001
#define USB_CTRL_CLK_GATE_W1S                             0x03e4
#define USB_CTRL_CLK_GATE_W1S_IP_CLK_USBC_DEBUG_GATE_SHIFT                   0
#define USB_CTRL_CLK_GATE_W1C                             0x03e8
#define USB_CTRL_CLK_GATE_W1C_IP_CLK_USBC_DEBUG_GATE_SHIFT                   0
#define USB_CTRL_CLK_GATE_RO                              0x03ec
#define USB_CTRL_CLK_GATE_RO_IP_CLK_USBC_DEBUG_GATE_SHIFT                    0
#define USB_CTRL_CLK_GATE_RO_IP_CLK_USBC_DEBUG_GATE_MASK                     0x00000001
#define USB_REF_ALT_LP_GATE_W1S                          0x03f0
#define USB_W1S_IP_CLK_USB_REF_ALT_LP_GATE_SHIFT            0
#define USB_REF_ALT_LP_GATE_W1C                          0x03f4
#define USB_W1C_IP_CLK_USB_REF_ALT_LP_GATE_SHIFT            0
#define USB_REF_ALT_LP_GATE_RO                           0x03f8
#define USB_RO_IP_CLK_USB_REF_ALT_LP_GATE_SHIFT             0
#define USB_RO_IP_CLK_USB_REF_ALT_LP_GATE_MASK              0x00000001
#define USB_REF_ALT_GATE_W1S                             0x03fc
#define USB_IP_CLK_USB_REF_ALT_GATE_SHIFT                  0
#define USB_REF_ALT_GATE_W1C                             0x0400
#define USB_W1C_IP_CLK_USB_REF_ALT_GATE_SHIFT                  0
#define USB_REF_ALT_GATE_RO                              0x0404
#define USB_IP_CLK_USB_REF_ALT_GATE_SHIFT                   0
#define USB_IP_CLK_USB_REF_ALT_GATE_MASK                    0x00000001
#define USB_REQ_CLK_IP_ENABLE                                       0x0408
#define USB_CLK_CONTROLLER_REF_ENABLE_SHIFT                      0
#define USB_CLK_CONTROLLER_REF_ENABLE_MASK                       0x00000001
#define USB_CLK_USB_REF_ALT_ENABLE_SHIFT                         1
#define USB_CLK_USB_REF_ALT_ENABLE_MASK                          0x00000002
#define USB_CLK_USB_BUS_EARLY_ENABLE_SHIFT                       2
#define USB_CLK_USB_BUS_EARLY_ENABLE_MASK                        0x00000004
#define USB_CLK_USB_EUSB_REF_ENABLE_SHIFT                        3
#define USB_CLK_USB_EUSB_REF_ENABLE_MASK                         0x00000008
#define USB_CLK_USB_COMBOPHY_FW_ENABLE_SHIFT                     4
#define USB_CLK_USB_COMBOPHY_FW_ENABLE_MASK                      0x00000010
#define USB_COMBOPHY_USB32_PIPE_LANE1                               0x040c
#define USB_USB32_PIPE_LANE1_ENCDEC_BYPASS_SHIFT             0
#define USB_USB32_PIPE_LANE1_ENCDEC_BYPASS_MASK              0x00000001
#define USB_USB32_PIPE_LANE1_IF_WIDTH_SHIFT                  1
#define USB_USB32_PIPE_LANE1_IF_WIDTH_MASK                   0x00000006
#define USB_USB32_PIPE_LANE1_LINK_NUM_SHIFT                  3
#define USB_USB32_PIPE_LANE1_LINK_NUM_MASK                   0x00000078
#define USB_USB32_PIPE_LANE1_MAXPCLK_DIV_RATIO_SHIFT         7
#define USB_USB32_PIPE_LANE1_MAXPCLK_DIV_RATIO_MASK          0x00000780
#define USB_USB32_PIPE_LANE1_MAXPCLKREQ_SHIFT                11
#define USB_USB32_PIPE_LANE1_MAXPCLKREQ_MASK                 0x00001800
#define USB_USB32_PIPE_LANE1_PCLKCHANGEACK_SHIFT             13
#define USB_USB32_PIPE_LANE1_PCLKCHANGEACK_MASK              0x00002000
#define USB_USB32_PIPE_LANE1_POWERDOWN_SHIFT                 14
#define USB_USB32_PIPE_LANE1_POWERDOWN_MASK                  0x0003c000
#define USB_USB32_PIPE_LANE1_RATE_SHIFT                      18
#define USB_USB32_PIPE_LANE1_RATE_MASK                       0x003c0000
#define USB_USB32_PIPE_LANE1_RESET_N_SHIFT                   22
#define USB_USB32_PIPE_LANE1_RESET_N_MASK                    0x00400000
#define USB_USB32_PIPE_LANE1_DISABLE_SHIFT                   23
#define USB_USB32_PIPE_LANE1_DISABLE_MASK                    0x00800000
#define USB_USB32_PIPE_LANE1_TX2RX_LOOPBK_SHIFT              24
#define USB_USB32_PIPE_LANE1_TX2RX_LOOPBK_MASK               0x01000000
#define USB_USB32_PIPE_RX1_BLK_ALIGN_CTL_SHIFT               25
#define USB_USB32_PIPE_RX1_BLK_ALIGN_CTL_MASK                0x02000000
#define USB_USB32_PIPE_RX1_EQ_TRAINING_SHIFT                 26
#define USB_USB32_PIPE_RX1_EQ_TRAINING_MASK                  0x04000000
#define USB_USB32_PIPE_RX1_ES_MODE_SHIFT                     27
#define USB_USB32_PIPE_RX1_ES_MODE_MASK                      0x08000000
#define USB_USB32_PIPE_RX1_IF_WIDTH_SHIFT                    28
#define USB_USB32_PIPE_RX1_IF_WIDTH_MASK                     0x30000000
#define USB_USB32_PIPE_RX1_POLARITY_SHIFT                    30
#define USB_USB32_PIPE_RX1_POLARITY_MASK                     0x40000000
#define USB_COMBOPHY_USB32_PIPE_RX1                                 0x0410
#define USB_USB32_PIPE_RX1_TERMINATION_SHIFT                   0
#define USB_USB32_PIPE_RX1_TERMINATION_MASK                    0x00000001
#define USB_USB32_PIPE_RX1_STANDBY_SHIFT                       1
#define USB_USB32_PIPE_RX1_STANDBY_MASK                        0x00000002
#define USB_CFG_USB_MAXPCLK_REQ_SHIFT                          2
#define USB_CFG_USB_MAXPCLK_REQ_MASK                           0x0000000c
#define USB_COMBOPHY_USB32_PIPE_TX1                                 0x0414
#define USB_TX1_USB32_PIPE_TX1_DATAK_SHIFT                         0
#define USB_TX1_USB32_PIPE_TX1_DATAK_MASK                          0x0000000f
#define USB_TX1_USB32_PIPE_TX1_DATAVALID_SHIFT                     4
#define USB_TX1_USB32_PIPE_TX1_DATAVALID_MASK                      0x00000010
#define USB_TX1_USB32_PIPE_TX1_DEEMPH_SHIFT                        5
#define USB_TX1_USB32_PIPE_TX1_DEEMPH_MASK                         0x007fffe0
#define USB_TX1_USB32_PIPE_TX1_DETECTRX_SHIFT                      23
#define USB_TX1_USB32_PIPE_TX1_DETECTRX_MASK                       0x00800000
#define USB_TX1_USB32_PIPE_TX1_ELECIDLE_SHIFT                      24
#define USB_TX1_USB32_PIPE_TX1_ELECIDLE_MASK                       0x01000000
#define USB_TX1_USB32_PIPE_TX1_ONES_ZEROS_SHIFT                    25
#define USB_TX1_USB32_PIPE_TX1_ONES_ZEROS_MASK                     0x02000000
#define USB_TX1_USB32_PIPE_TX1_STRATBLOCK_SHIFT                    26
#define USB_TX1_USB32_PIPE_TX1_STRATBLOCK_MASK                     0x04000000
#define USB_TX1_USB32_PIPE_TX1_SYNCHEADER_SHIFT                    27
#define USB_TX1_USB32_PIPE_TX1_SYNCHEADER_MASK                     0x78000000
#define USB_COMBOPHY_USB32_PIPES_STATUS                             0x0418
#define USB_STATUS_USB32_PIPE_LANE1_DATABUSWIDTH_SHIFT            0
#define USB_STATUS_USB32_PIPE_LANE1_DATABUSWIDTH_MASK             0x00000003
#define USB_STATUS_USB32_PIPE_LANE1_PHY_SATUS_SHIFT               2
#define USB_STATUS_USB32_PIPE_LANE1_PHY_SATUS_MASK                0x00000004
#define USB_STATUS_USB32_PIPE_LANE1_POWER_PRESENT_SHIFT           3
#define USB_STATUS_USB32_PIPE_LANE1_POWER_PRESENT_MASK            0x00000008
#define USB_STATUS_USB32_PIPE_LANE1_REF_CLK_REQ_N_SHIFT           4
#define USB_STATUS_USB32_PIPE_LANE1_REF_CLK_REQ_N_MASK            0x00000010
#define USB_STATUS_USB32_PIPE_LANE1_DATAK_SHIFT                   5
#define USB_STATUS_USB32_PIPE_LANE1_DATAK_MASK                    0x000001e0
#define USB_STATUS_USB32_PIPE_LANE1_DATAVALID_SHIFT               9
#define USB_STATUS_USB32_PIPE_LANE1_DATAVALID_MASK                0x00000200
#define USB_STATUS_USB32_PIPE_RX1_EBUFF_LOACTION_SHIFT            10
#define USB_STATUS_USB32_PIPE_RX1_EBUFF_LOACTION_MASK             0x0007fc00
#define USB_STATUS_USB32_PIPE_RX1_ELECIDLE_SHIFT                  19
#define USB_STATUS_USB32_PIPE_RX1_ELECIDLE_MASK                   0x00080000
#define USB_STATUS_USB32_PIPE_RX1_STANDBY_STATUS_SHIFT            20
#define USB_STATUS_USB32_PIPE_RX1_STANDBY_STATUS_MASK             0x00100000
#define USB_STATUS_USB32_PIPE_RX1_STARTBLOCK_SHIFT                21
#define USB_STATUS_USB32_PIPE_RX1_STARTBLOCK_MASK                 0x00200000
#define USB_STATUS_USB32_PIPE_RX1_STATUS_SHIFT                    22
#define USB_STATUS_USB32_PIPE_RX1_STATUS_MASK                     0x01c00000
#define USB_STATUS_USB32_PIPE_RX1_SYNCHEADER_SHIFT                25
#define USB_STATUS_USB32_PIPE_RX1_SYNCHEADER_MASK                 0x1e000000
#define USB_STATUS_USB32_PIPE_RX1_VALID_SHIFT                     29
#define USB_STATUS_USB32_PIPE_RX1_VALID_MASK                      0x20000000
#define USB_STATUS_USB32_PIPE_LANE1_PCLKCHANGEOK_SHIFT            30
#define USB_STATUS_USB32_PIPE_LANE1_PCLKCHANGEOK_MASK             0x40000000
#define USB_USB32_PIPE_LANE1_MAXPCLKACK                             0x041c
#define USB_USB32_PIPE_LANE1_MAXPCLKACK_SHIFT              0
#define USB_USB32_PIPE_LANE1_MAXPCLKACK_MASK               0x00000003
#define USB_LANE0_BLK_BLK_LENGTH_ERR_CNT                            0x0420
#define USB_LANE1_BLK_BLK_LENGTH_ERR_CNT                            0x0424
#define USB_LANE0_BLK_DEC_MULT_ERR_CNT                              0x0428
#define USB_LANE1_BLK_DEC_MULT_ERR_CNT                              0x042c
#define USB_IP_RST_USB_VAUX_REG_N_W1S                               0x0430
#define USB_W1S_IP_RST_USB_VAUX_REG_N_SHIFT                      0
#define USB_IP_RST_USB_VAUX_REG_N_W1C                               0x0434
#define USB_W1C_IP_RST_USB_VAUX_REG_N_SHIFT                      0
#define USB_IP_RST_USB_VAUX_REG_N_RO                                0x0438
#define USB_IP_RO_IP_RST_USB_VAUX_REG_N_SHIFT                       0
#define USB_IP_RO_IP_RST_USB_VAUX_REG_N_MASK                        0x00000001
#define USB_PM_POWER_STATE_REQUEST                                  0x043c
#define USB_PM_POWER_STATE_REQUEST_SHIFT                        0
#define USB_PM_POWER_STATE_REQUEST_MASK                         0x00000003
#define USB_REGFILE_CGBYPASS                                        0x0440
#define USB_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                                    0
#define USB_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                                     0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int rov_0      : 32;
    } reg;
}usb_subsys_sctrl_apb_usb_version_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_subsys_testpin_sel   :  8;
        unsigned int usb_combophy_testpin_sel :  8;
        unsigned int reserved_0               : 16;
    } reg;
}usb_subsys_sctrl_apb_usb_subsys_testpin_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_vbus_valid_debo_bypass :  1;
        unsigned int reserved_0                 : 31;
    } reg;
}usb_subsys_sctrl_apb_usb_vbus_valid_debo_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_pclk_sel :  1;
        unsigned int reserved_0    : 31;
    } reg;
}usb_subsys_sctrl_apb_pipe_pclk_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_subsys_ram_ctrl_bus_0 : 32;
    } reg;
}usb_subsys_sctrl_apb_usb_subsys_mem_cfg_reg_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_subsys_ram_ctrl_bus_1 : 32;
    } reg;
}usb_subsys_sctrl_apb_usb_subsys_mem_cfg_reg_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_subsys_ram_ctrl_bus_2 : 32;
    } reg;
}usb_subsys_sctrl_apb_usb_subsys_mem_cfg_reg_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int utmi_host_disconnect_sel :  1;
        unsigned int utmi_host_disconnect_deg :  1;
        unsigned int reserved_0               : 30;
    } reg;
}usb_subsys_sctrl_apb_utmi_host_disconnect_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int utmi_tx_data_sel    :  1;
        unsigned int utmi_tx_data_deg    :  8;
        unsigned int utmi_txvalid_sel    :  1;
        unsigned int utmi_txvalid_deg    :  1;
        unsigned int utmi_dmpulldown_sel :  1;
        unsigned int utmi_dmpulldown_deg :  1;
        unsigned int utmi_dppulldown_sel :  1;
        unsigned int utmi_dppulldown_deg :  1;
        unsigned int utmi_xcvrselect_sel :  1;
        unsigned int utmi_xcvrselect_deg :  2;
        unsigned int utmi_termselect_sel :  1;
        unsigned int utmi_termselect_deg :  1;
        unsigned int utmi_opmode_sel     :  1;
        unsigned int utmi_opmode_deg     :  2;
        unsigned int reserved_0          :  9;
    } reg;
}usb_subsys_sctrl_apb_utmi_siganl_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int link_status_clr :  1;
        unsigned int reserved_0      : 31;
    } reg;
}usb_subsys_sctrl_apb_link_status_clr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int link_state_status : 30;
        unsigned int reserved_0        :  2;
    } reg;
}usb_subsys_sctrl_apb_link_state_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int poll_state_status     : 22;
        unsigned int recovery_state_status :  7;
        unsigned int reserved_0            :  3;
    } reg;
}usb_subsys_sctrl_apb_poll_recovery_state_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ocla_low_high_sel  :  1;
        unsigned int ocla_vld_width_sel :  1;
        unsigned int reserved_0         : 30;
    } reg;
}usb_subsys_sctrl_apb_ocla_high_low_vld_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_usbc_apb_clk_gate  :  1;
        unsigned int st_usbc_link_clk_gate :  1;
        unsigned int reserved_0            : 30;
    } reg;
}usb_subsys_sctrl_apb_usb_debug_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_phy_mode             :  2;
        unsigned int pipe_compliance           :  1;
        unsigned int pipe_txmargin             :  3;
        unsigned int pipe_txswing              :  1;
        unsigned int host_legacy_smi_interrupt :  1;
        unsigned int rx_se_dp_hv               :  1;
        unsigned int rx_se_dm_hv               :  1;
        unsigned int reserved_0                : 22;
    } reg;
}usb_subsys_sctrl_apb_pipe_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_vbus_valid_sel    :  1;
        unsigned int usb_vbus_valid_cc     :  1;
        unsigned int ctrl_srpotg_valid_sel :  1;
        unsigned int combophy_dp_clk_sel   :  1;
        unsigned int reserved_0            : 28;
    } reg;
}usb_subsys_sctrl_apb_usb_vbus_valid_cc_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ram0_sd_pudelay   :  1;
        unsigned int ram0_dslp_pudelay :  1;
        unsigned int ram1_sd_pudelay   :  1;
        unsigned int ram1_dslp_pudelay :  1;
        unsigned int ram2_sd_pudelay   :  1;
        unsigned int ram2_dslp_pudelay :  1;
        unsigned int reserved_0        : 26;
    } reg;
}usb_subsys_sctrl_apb_controller_ram_pudelay_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bigendian_gs :  1;
        unsigned int reserved_0   : 31;
    } reg;
}usb_subsys_sctrl_apb_bigendian_gs_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int star_fix_disable_ctl_inp : 32;
    } reg;
}usb_subsys_sctrl_apb_star_fix_disable_ctl_inp_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int host_current_belt : 12;
        unsigned int reserved_0        : 20;
    } reg;
}usb_subsys_sctrl_apb_host_current_belt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int host_u2_port_disable :  1;
        unsigned int host_u3_port_disable :  1;
        unsigned int hub_port_perm_attach :  2;
        unsigned int reserved_0           : 28;
    } reg;
}usb_subsys_sctrl_apb_host_u2_u3_port_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int host_force_gen1_speed :  1;
        unsigned int reserved_0            : 31;
    } reg;
}usb_subsys_sctrl_apb_host_force_gen1_speed_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ltssm_clk_state :  4;
        unsigned int reserved_0      : 28;
    } reg;
}usb_subsys_sctrl_apb_ltssm_clk_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hub_vbus_ctrl         :  2;
        unsigned int pmgt_ext_bus_clk_gate :  1;
        unsigned int reserved_0            : 29;
    } reg;
}usb_subsys_sctrl_apb_hub_vbus_ctrl_clk_gate_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int host_port_overcurrent           :  2;
        unsigned int host_port_power_control_present :  1;
        unsigned int reserved_0                      : 29;
    } reg;
}usb_subsys_sctrl_apb_host_port_overcurrent_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int host_msi_enable :  1;
        unsigned int xhc_bme         :  1;
        unsigned int reserved_0      : 30;
    } reg;
}usb_subsys_sctrl_apb_host_mis_enable_xhc_bme_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bus_filter_bypass :  4;
        unsigned int reserved_0        : 28;
    } reg;
}usb_subsys_sctrl_apb_bus_filter_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int soc_common_rd_wr_bus :  1;
        unsigned int reserved_0           : 31;
    } reg;
}usb_subsys_sctrl_apb_soc_common_rd_wr_bus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int soc_rd_uf_kb_bandwidth : 15;
        unsigned int soc_wr_uf_kb_bandwidth : 15;
        unsigned int reserved_0             :  2;
    } reg;
}usb_subsys_sctrl_apb_soc_rd_uf_kb_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int devspd_ovrd :  4;
        unsigned int reserved_0  : 28;
    } reg;
}usb_subsys_sctrl_apb_devspd_ovrd_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sdbn_ds_port_ess_inactive :  1;
        unsigned int reserved_0                : 31;
    } reg;
}usb_subsys_sctrl_apb_sdbn_ds_port_ess_inactive_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int logic_analyzer_trace_ext_mux_en :  1;
        unsigned int reserved_0                      : 31;
    } reg;
}usb_subsys_sctrl_apb_logic_analyzer_trace_ext_mux_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int logic_analyzer_trace_ext_mux : 32;
    } reg;
}usb_subsys_sctrl_apb_logic_analyzer_trace_ext_mux_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_mx_pclk_ready :  1;
        unsigned int reserved_0         : 31;
    } reg;
}usb_subsys_sctrl_apb_pipe_mx_pclk_ready_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_ctrl_debug_0 : 32;
    } reg;
}usb_subsys_sctrl_apb_usb_ctrl_debug_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_ctrl_debug_1 : 32;
    } reg;
}usb_subsys_sctrl_apb_usb_ctrl_debug_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_ctrl_debug_2 : 32;
    } reg;
}usb_subsys_sctrl_apb_usb_ctrl_debug_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_ctrl_debug_3 : 27;
        unsigned int reserved_0       :  5;
    } reg;
}usb_subsys_sctrl_apb_usb_ctrl_debug_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int utmi_rxvalidh       :  1;
        unsigned int utmi_txvalidh       :  1;
        unsigned int utmi_word_if        :  1;
        unsigned int utmi_fsls_low_power :  1;
        unsigned int utmi_fslsserialmode :  1;
        unsigned int utmisrp_bvalid      :  1;
        unsigned int res_req_out         :  1;
        unsigned int reserved_0          : 25;
    } reg;
}usb_subsys_sctrl_apb_utmi_rxvalidh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gp_out     : 16;
        unsigned int reserved_0 : 16;
    } reg;
}usb_subsys_sctrl_apb_gp_out_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gp_in_sel  :  1;
        unsigned int gp_in_reg  : 16;
        unsigned int reserved_0 : 15;
    } reg;
}usb_subsys_sctrl_apb_gp_in_reg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pme_en     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}usb_subsys_sctrl_apb_pme_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pme_generation :  1;
        unsigned int reserved_0     : 31;
    } reg;
}usb_subsys_sctrl_apb_pme_generation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_databuswidth :  2;
        unsigned int reserved_0        : 30;
    } reg;
}usb_subsys_sctrl_apb_pipe_databuswidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_usb_acpu_mask       :  1;
        unsigned int intr_usb_lpmcu_mask      :  1;
        unsigned int intr_usb_adsp_mask       :  1;
        unsigned int intr_usb_acpu_err_mask   :  1;
        unsigned int intr_usb_lpmcu_err_mask  :  1;
        unsigned int intr_usb_adsp_err_mask   :  1;
        unsigned int intr_sub_vbus_acpu_mask  :  1;
        unsigned int intr_sub_vbus_lpmcu_mask :  1;
        unsigned int intr_sub_vbus_adsp_mask  :  1;
        unsigned int reserved_0               : 23;
    } reg;
}usb_subsys_sctrl_apb_intr_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_controller_reg_n :  1;
        unsigned int reserved_0              : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_controller_reg_n_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_controller_reg_n :  1;
        unsigned int reserved_0              : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_controller_reg_n_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_controller_reg_n :  1;
        unsigned int reserved_0              : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_controller_reg_n_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int host_num_u2_port :  4;
        unsigned int host_num_u3_port :  4;
        unsigned int reserved_0       : 24;
    } reg;
}usb_subsys_sctrl_apb_host_num_port_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_pclk_rate :  3;
        unsigned int reserved_0     : 29;
    } reg;
}usb_subsys_sctrl_apb_pipe_pclk_rate_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int logic_analyzer_trace_low : 32;
    } reg;
}usb_subsys_sctrl_apb_logic_analyzer_trace_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int logic_analyzer_trace_high : 32;
    } reg;
}usb_subsys_sctrl_apb_logic_analyzer_trace_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_idle_soft_ovrd :  1;
        unsigned int reserved_0         : 31;
    } reg;
}usb_subsys_sctrl_apb_usb_idle_soft_ovrd_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_awqos_value :  4;
        unsigned int usb_arqos_value :  4;
        unsigned int reserved_0      : 24;
    } reg;
}usb_subsys_sctrl_apb_usb_qos_value_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_autoresume_enable :  1;
        unsigned int reserved_0            : 31;
    } reg;
}usb_subsys_sctrl_apb_phy_autoresume_enable_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_cfg_cr_clk_sel :  1;
        unsigned int reserved_0         : 31;
    } reg;
}usb_subsys_sctrl_apb_phy_cfg_cr_clk_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_cfg_pll_cpbias_cntrl :  7;
        unsigned int phy_cfg_pll_gmp_cntrl    :  2;
        unsigned int phy_cfg_pll_int_cntrl    :  6;
        unsigned int phy_cfg_pll_prop_cntrl   :  6;
        unsigned int phy_cfg_pll_vco_cntrl    :  3;
        unsigned int phy_cfg_pll_vref_tune    :  2;
        unsigned int ref_freq_sel             :  3;
        unsigned int reserved_0               :  3;
    } reg;
}usb_subsys_sctrl_apb_phy_cfg_pll_cpbias_cntrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_cfg_pll_ref_div :  4;
        unsigned int phy_cfg_pll_fb_div  : 12;
        unsigned int reserved_0          : 16;
    } reg;
}usb_subsys_sctrl_apb_phy_cfg_pll_ref_div_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_cfg_por_in_lx :  1;
        unsigned int reserved_0        : 31;
    } reg;
}usb_subsys_sctrl_apb_phy_cfg_por_in_lx_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_cfg_rcal_bypass :  1;
        unsigned int reserved_0          : 31;
    } reg;
}usb_subsys_sctrl_apb_phy_cfg_rcal_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_cfg_rcal_code   :  4;
        unsigned int phy_cfg_rcal_offset :  4;
        unsigned int reserved_0          : 24;
    } reg;
}usb_subsys_sctrl_apb_phy_cfg_rcal_code_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_cfg_rx_eq_ctle :  2;
        unsigned int reserved_0         : 30;
    } reg;
}usb_subsys_sctrl_apb_phy_cfg_rxeq_ctle_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_cfg_rx_hs_term_en          :  1;
        unsigned int phy_cfg_rx_hs_tune             :  3;
        unsigned int phy_cfg_tx_fsls_slew_rate_tune :  1;
        unsigned int phy_cfg_tx_fsls_vreg_bypass    :  1;
        unsigned int phy_cfg_tx_hs_drv_ofst         :  2;
        unsigned int phy_cfg_tx_hs_vref_tune        :  3;
        unsigned int phy_cfg_tx_hs_xv_tune          :  2;
        unsigned int phy_cfg_tx_preemp_tune         :  3;
        unsigned int phy_cfg_tx_res_tune            :  2;
        unsigned int phy_cfg_tx_rise_tune           :  2;
        unsigned int reserved_0                     : 12;
    } reg;
}usb_subsys_sctrl_apb_phy_cfg_rx_hs_term_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int eusbphy_soft_enable :  1;
        unsigned int reserved_0          : 31;
    } reg;
}usb_subsys_sctrl_apb_eusb_soft_enable_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_tx_dig_bypass_sel :  1;
        unsigned int reserved_0            : 31;
    } reg;
}usb_subsys_sctrl_apb_phy_tx_dig_bypass_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_rx_se_dp :  1;
        unsigned int phy_rx_se_dm :  1;
        unsigned int reserved_0   : 30;
    } reg;
}usb_subsys_sctrl_apb_phy_rx_se_dp_dm_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_tx_se_dm_en :  1;
        unsigned int phy_tx_se_dm    :  1;
        unsigned int reserved_0      : 30;
    } reg;
}usb_subsys_sctrl_apb_phy_tx_se_dm_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_tx_se_dp_en :  1;
        unsigned int phy_tx_se_dp    :  1;
        unsigned int reserved_0      : 30;
    } reg;
}usb_subsys_sctrl_apb_phy_tx_se_dp_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int test_burnin :  1;
        unsigned int reserved_0  : 31;
    } reg;
}usb_subsys_sctrl_apb_test_burnin_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int test_iddq  :  1;
        unsigned int reserved_0 : 31;
    } reg;
}usb_subsys_sctrl_apb_test_iddq_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int test_loopback_en :  1;
        unsigned int reserved_0       : 31;
    } reg;
}usb_subsys_sctrl_apb_test_loopback_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int test_stop_clk_en :  1;
        unsigned int reserved_0       : 31;
    } reg;
}usb_subsys_sctrl_apb_test_stop_clk_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int utmi_txbitstuffen :  1;
        unsigned int reserved_0        : 31;
    } reg;
}usb_subsys_sctrl_apb_utmi_txbitstuffen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_cfg_rptr_mode :  1;
        unsigned int reserved_0        : 31;
    } reg;
}usb_subsys_sctrl_apb_phy_cfg_rptr_mdoe_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int utmi_hostdisconnect :  1;
        unsigned int reserved_0          : 31;
    } reg;
}usb_subsys_sctrl_apb_utmi_hostdisconnect_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_eusb_reg :  1;
        unsigned int reserved_0      : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_eusb_reg_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_eusb_reg :  1;
        unsigned int reserved_0      : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_eusb_reg_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_eusb_reg :  1;
        unsigned int reserved_0      : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_eusb_reg_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int utmi_xcvrselect :  2;
        unsigned int utmi_termselect :  1;
        unsigned int utmi_opmode     :  2;
        unsigned int reserved_0      : 27;
    } reg;
}usb_subsys_sctrl_apb_utmi_xcvrselect_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int utmi_dp_pulldown :  1;
        unsigned int utmi_dm_pulldown :  1;
        unsigned int reserved_0       : 30;
    } reg;
}usb_subsys_sctrl_apb_utmi_dp_dm_pulldown_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int utmi_clk_force_en :  1;
        unsigned int reserved_0        : 31;
    } reg;
}usb_subsys_sctrl_apb_utmi_clk_force_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_rom_ce_disable :  1;
        unsigned int reserved_0          : 31;
    } reg;
}usb_subsys_sctrl_apb_phy0_rom_ce_disable_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_rom_sd_pudelay :  1;
        unsigned int reserved_0          : 31;
    } reg;
}usb_subsys_sctrl_apb_phy0_rom_sd_pudelay_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_sram_sd_pudelay   :  1;
        unsigned int phy0_sram_dslp_pudelay :  1;
        unsigned int reserved_0             : 30;
    } reg;
}usb_subsys_sctrl_apb_phy0_sram_pudelay_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol1_ext_ref_clk_div2_en :  1;
        unsigned int reserved_0                    : 31;
    } reg;
}usb_subsys_sctrl_apb_protocol1_ext_ref_clk_div2_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol1_ext_ref_range :  3;
        unsigned int reserved_0              : 29;
    } reg;
}usb_subsys_sctrl_apb_protocol1_ext_ref_range_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol1_ext_tx_eq_main_g1 : 12;
        unsigned int protocol1_ext_tx_eq_main_g2 : 12;
        unsigned int reserved_0                  :  8;
    } reg;
}usb_subsys_sctrl_apb_protocol1_ext_tx_eq_main_g1_g2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol1_ext_tx_eq_ovrd_g1 :  2;
        unsigned int protocol1_ext_tx_eq_ovrd_g2 :  2;
        unsigned int reserved_0                  : 28;
    } reg;
}usb_subsys_sctrl_apb_protocol1_ext_tx_eq_ovrd_g1_g2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol1_ext_tx_eq_post_g1 : 12;
        unsigned int protocol1_ext_tx_eq_post_g2 : 12;
        unsigned int reserved_0                  :  8;
    } reg;
}usb_subsys_sctrl_apb_protocol1_ext_tx_eq_post_g1_g2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol1_ext_tx_eq_pre_g1 : 12;
        unsigned int protocol1_ext_tx_eq_pre_g2 : 12;
        unsigned int reserved_0                 :  8;
    } reg;
}usb_subsys_sctrl_apb_protocol1_ext_tx_eq_pre_g1_g2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol3_ext_ref_clk_div2_en :  1;
        unsigned int protocol3_ext_ref_range       :  3;
        unsigned int protocol7_ext_ref_clk_div2_en :  1;
        unsigned int protocol7_ext_ref_range       :  3;
        unsigned int protocol9_ext_ref_clk_div2_en :  1;
        unsigned int protocol9_ext_ref_range       :  3;
        unsigned int reserved_0                    : 20;
    } reg;
}usb_subsys_sctrl_apb_protocol3_ext_ref_clk_div2_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_mplla_ssc_en :  1;
        unsigned int reserved_0        : 31;
    } reg;
}usb_subsys_sctrl_apb_phy0_mplla_ssc_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_mplla_state            :  1;
        unsigned int phy0_mplla_word_sync_clk_en :  1;
        unsigned int phy0_mplla_force_ack        :  1;
        unsigned int reserved_0                  : 29;
    } reg;
}usb_subsys_sctrl_apb_phy0_mplla_state_sync_clk_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_ref_alt_clk_lp_sel  :  1;
        unsigned int phy0_ref_clkdet_en       :  1;
        unsigned int phy0_ref_clkdet_result   :  1;
        unsigned int phy0_ref_pre_vreg_bypass :  1;
        unsigned int reserved_0               : 28;
    } reg;
}usb_subsys_sctrl_apb_phy0_ref_alt_clk_lp_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_test_burnin        :  1;
        unsigned int phy_test_powerdown     :  1;
        unsigned int phy_test_stop_clk_en   :  1;
        unsigned int phy_test_tx_ref_clk_en :  1;
        unsigned int reserved_0             : 28;
    } reg;
}usb_subsys_sctrl_apb_phy_test_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_pg_mode_en :  1;
        unsigned int reserved_0     : 31;
    } reg;
}usb_subsys_sctrl_apb_phy_pg_mode_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_bs_acmode  :  1;
        unsigned int phy0_bs_actest :  1;
        unsigned int reserved_0     : 30;
    } reg;
}usb_subsys_sctrl_apb_phy0_bs_acmode_actest_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_ref_use_pad :  1;
        unsigned int reserved_0       : 31;
    } reg;
}usb_subsys_sctrl_apb_phy0_ref_use_pad_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_res_ext_en :  1;
        unsigned int reserved_0      : 31;
    } reg;
}usb_subsys_sctrl_apb_phy0_res_ext_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_ext_ref_ovrd_sel :  1;
        unsigned int phy0_res_ext_rcal    :  6;
        unsigned int reserved_0           : 25;
    } reg;
}usb_subsys_sctrl_apb_phy_ext_ref_ovrd_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_lane0_power_present   :  1;
        unsigned int phy_lane0_rx2tx_par_lb_en :  1;
        unsigned int phy_lane1_power_present   :  1;
        unsigned int phy_lane3_rx2tx_par_lb_en :  1;
        unsigned int reserved_0                : 28;
    } reg;
}usb_subsys_sctrl_apb_phy_lane0_power_present_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_rtune_req :  1;
        unsigned int phy_rtune_ack :  1;
        unsigned int reserved_0    : 30;
    } reg;
}usb_subsys_sctrl_apb_phy_rtune_req_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_rx1_term_acdc :  1;
        unsigned int phy_rx2_term_acdc :  1;
        unsigned int reserved_0        : 30;
    } reg;
}usb_subsys_sctrl_apb_phy_rx1_term_acdc_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_lane0_link_num :  4;
        unsigned int reserved_0                : 28;
    } reg;
}usb_subsys_sctrl_apb_usb32_pipe_lane0_link_num_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_lane0_maxpclk_div_ratio :  4;
        unsigned int reserved_0                         : 28;
    } reg;
}usb_subsys_sctrl_apb_usb32_pipe_lane0_maxpclk_div_ratio_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_apb0_if_mode :  2;
        unsigned int reserved_0        : 30;
    } reg;
}usb_subsys_sctrl_apb_phy0_apb0_if_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_lane0_disable :  1;
        unsigned int reserved_0               : 31;
    } reg;
}usb_subsys_sctrl_apb_usb32_pipe_lane0_disable_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_lan0_tx2rx_loopbk :  1;
        unsigned int reserved_0                   : 31;
    } reg;
}usb_subsys_sctrl_apb_usb32_pipe_lan0_tx2rx_loopbk_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_rx0_if_width :  2;
        unsigned int reserved_0              : 30;
    } reg;
}usb_subsys_sctrl_apb_usb32_pipe_rx0_if_width_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_ref_repeat_clk_en :  1;
        unsigned int reserved_0             : 31;
    } reg;
}usb_subsys_sctrl_apb_phy0_ref_repeat_clk_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int upcs_pipe_config : 16;
        unsigned int reserved_0       : 16;
    } reg;
}usb_subsys_sctrl_apb_upcs_pipe_config_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_mplla_force_en :  1;
        unsigned int reserved_0          : 31;
    } reg;
}usb_subsys_sctrl_apb_phy0_mplla_force_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_test_flyover_en :  1;
        unsigned int reserved_0           : 31;
    } reg;
}usb_subsys_sctrl_apb_phy0_test_flyover_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_lane0_maxpclkreq :  2;
        unsigned int usb32_pipe_lane0_maxpclkack :  2;
        unsigned int reserved_0                  : 28;
    } reg;
}usb_subsys_sctrl_apb_usb32_pipe_lane0_maxpclkreq_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ss_rxdet_disable_ack_1 :  1;
        unsigned int reserved_0             : 31;
    } reg;
}usb_subsys_sctrl_apb_ss_rxdet_disable_ack_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ss_rxdet_disable_1               :  1;
        unsigned int tca_vbusvalid                    :  1;
        unsigned int phy_res_ack_out                  :  1;
        unsigned int st_apb_clk_gate                  :  1;
        unsigned int st_pcs_lane0_ref_clk_gate        :  1;
        unsigned int st_pcs_lane1_ref_clk_gate        :  1;
        unsigned int st_lane0_es_wr_clk_gate          :  1;
        unsigned int st_lane1_es_wr_clk_gate          :  1;
        unsigned int st_pcs_lane0_pcs_nonsds_clk_gate :  1;
        unsigned int st_pcs_lane1_pcs_nonsds_clk_gate :  1;
        unsigned int usb32_pipe_lane0_databuswidth    :  2;
        unsigned int phy_rx2_ppm_drift_vld            :  1;
        unsigned int phy_rx2_ppm_drift                :  6;
        unsigned int phy_rx1_ppm_drift_vld            :  1;
        unsigned int phy_rx1_ppm_drift                :  6;
        unsigned int reserved_0                       :  6;
    } reg;
}usb_subsys_sctrl_apb_ss_rxdet_disable_1_tca_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ext_pclk_req :  1;
        unsigned int reserved_0   : 31;
    } reg;
}usb_subsys_sctrl_apb_ext_pclk_req_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_combophy_reg :  1;
        unsigned int reserved_0          : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_combophy_reg_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_combophy_reg :  1;
        unsigned int reserved_0          : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_combophy_reg_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_combophy_reg :  1;
        unsigned int reserved_0          : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_combophy_reg_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_pcs_pwr_en :  1;
        unsigned int phy0_pma_pwr_en :  1;
        unsigned int upcs_pwr_en     :  1;
        unsigned int reserved_0      : 29;
    } reg;
}usb_subsys_sctrl_apb_phy0_pcs_pwr_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_pcs_pwr_stable :  1;
        unsigned int upcs_pwr_stable     :  1;
        unsigned int reserved_0          : 30;
    } reg;
}usb_subsys_sctrl_apb_phy0_pcs_pwr_stable_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_sram_ext_ld_done           :  1;
        unsigned int phy0_sram_bypass_mode          :  2;
        unsigned int phy0_sram_bootload_bypass_mode :  2;
        unsigned int reserved_0                     : 27;
    } reg;
}usb_subsys_sctrl_apb_phy_sram_ext_done_bypass_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_sram_init_done :  1;
        unsigned int reserved_0         : 31;
    } reg;
}usb_subsys_sctrl_apb_phy_sram_init_done_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ss_lane0_active :  1;
        unsigned int ss_lane1_active :  1;
        unsigned int reserved_0      : 30;
    } reg;
}usb_subsys_sctrl_apb_combophy_ss_active_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int typec_flip_invert :  1;
        unsigned int reserved_0        : 31;
    } reg;
}usb_subsys_sctrl_apb_typec_flip_invert_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int host_legacy_smi_pci_cmd_reg_wr :  1;
        unsigned int host_legacy_smi_bar_wr         :  1;
        unsigned int reserved_0                     : 30;
    } reg;
}usb_subsys_sctrl_apb_host_legacy_smi_pci_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb_eusb_combo_div_en :  1;
        unsigned int usb2_only_mode_enable :  1;
        unsigned int reserved_0            : 30;
    } reg;
}usb_subsys_sctrl_apb_usb_combophy_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int combophy_pipe_lane0_reset_reg_sel :  1;
        unsigned int usb32_pipe_lane0_clk_sel          :  1;
        unsigned int reserved_0                        : 30;
    } reg;
}usb_subsys_sctrl_apb_combophy_pipe_lane0_reset_reg_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int combophy_pipe_lane0_reset_reg_n :  1;
        unsigned int reserved_0                      : 31;
    } reg;
}usb_subsys_sctrl_apb_combophy_pipe_lane0_reset_reg_n_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int combophy_pipe_lane0_reset_reg_n :  1;
        unsigned int reserved_0                      : 31;
    } reg;
}usb_subsys_sctrl_apb_combophy_pipe_lane0_reset_reg_n_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int combophy_pipe_lane0_reset_reg_n :  1;
        unsigned int reserved_0                      : 31;
    } reg;
}usb_subsys_sctrl_apb_combophy_pipe_lane0_reset_reg_n_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fpga_usb_reset    :  1;
        unsigned int fpga_usb_data16_8 :  1;
        unsigned int reserved_0        : 30;
    } reg;
}usb_subsys_sctrl_apb_fpga_usb_reset_width_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fpga_usb_uni_bidi :  1;
        unsigned int reserved_0        : 31;
    } reg;
}usb_subsys_sctrl_apb_fpga_usb_uni_bidi_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fpga_idpullup :  1;
        unsigned int reserved_0    : 31;
    } reg;
}usb_subsys_sctrl_apb_fpga_idpullup_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fpga_iddig :  1;
        unsigned int reserved_0 : 31;
    } reg;
}usb_subsys_sctrl_apb_fpga_iddig_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_cfg_pipe_ctl_clr :  1;
        unsigned int lane0_cfg_pipe_ctl_st0 :  5;
        unsigned int lane0_cfg_pipe_ctl_st1 :  5;
        unsigned int lane0_cfg_pipe_load    :  1;
        unsigned int reserved_0             : 20;
    } reg;
}usb_subsys_sctrl_apb_combophy_debug_lane0_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_pipe_ctl_st : 20;
        unsigned int reserved_0        : 12;
    } reg;
}usb_subsys_sctrl_apb_combophy_debug_lane0_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_pipe_ctl_debug : 32;
    } reg;
}usb_subsys_sctrl_apb_combophy_debug_lane0_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_cfg_pipe_ctl_clr :  1;
        unsigned int lane1_cfg_pipe_ctl_st0 :  5;
        unsigned int lane1_cfg_pipe_ctl_st1 :  5;
        unsigned int lane1_cfg_pipe_load    :  1;
        unsigned int reserved_0             : 20;
    } reg;
}usb_subsys_sctrl_apb_combophy_debug_lane1_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_pipe_ctl_st : 20;
        unsigned int reserved_0        : 12;
    } reg;
}usb_subsys_sctrl_apb_combophy_debug_lane1_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_pipe_ctl_debug : 32;
    } reg;
}usb_subsys_sctrl_apb_combophy_debug_lane1_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_sym_code_err_cnt0 : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_sym_code_err_cnt0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_sym_code_err_cnt1 : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_sym_code_err_cnt1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_sym_disp_err_cnt0 : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_sym_disp_err_cnt0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_sym_disp_err_cnt1 : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_sym_disp_err_cnt1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_cfg_disp_err_clr :  1;
        unsigned int lane1_cfg_code_err_clr :  1;
        unsigned int lane1_cfg_err_cnt_load :  1;
        unsigned int reserved_0             : 29;
    } reg;
}usb_subsys_sctrl_apb_lane1_cfg_clr_load_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_sym_code_err_cnt0 : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_sym_code_err_cnt0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_sym_code_err_cnt1 : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_sym_code_err_cnt1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_sym_disp_err_cnt0 : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_sym_disp_err_cnt0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_sym_disp_err_cnt1 : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_sym_disp_err_cnt1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_cfg_disp_err_clr :  1;
        unsigned int lane0_cfg_code_err_clr :  1;
        unsigned int lane0_cfg_err_cnt_load :  1;
        unsigned int reserved_0             : 29;
    } reg;
}usb_subsys_sctrl_apb_lane0_cfg_clr_load_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_elastic_buf_wr_status : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_wr_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_elastic_buf_rd_status : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_rd_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_elastic_buf_tp : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_tp_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_elastic_buf_wr_status : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_wr_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_elastic_buf_rd_status : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_rd_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_elastic_buf_tp : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_tp_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_sync_align_debug  : 20;
        unsigned int lane1_sync_align_status :  5;
        unsigned int reserved_0              :  7;
    } reg;
}usb_subsys_sctrl_apb_lane1_align_ds_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_sync_align_debug  : 20;
        unsigned int lane0_sync_align_status :  5;
        unsigned int reserved_0              :  7;
    } reg;
}usb_subsys_sctrl_apb_lane0_align_ds_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_symbol_align_debug  : 11;
        unsigned int lane1_symbol_align_status : 11;
        unsigned int reserved_0                : 10;
    } reg;
}usb_subsys_sctrl_apb_lane1_symbol_ds_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_cfg_sym_sts_clr     :  1;
        unsigned int lane1_cfg_sym_caputre1    :  3;
        unsigned int lane1_cfg_sym_caputre0    :  3;
        unsigned int lane1_cfg_symbol_ctl_load :  1;
        unsigned int reserved_0                : 24;
    } reg;
}usb_subsys_sctrl_apb_lane1_symbol_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_symbol_align_debug  : 11;
        unsigned int lane0_symbol_align_status : 11;
        unsigned int reserved_0                : 10;
    } reg;
}usb_subsys_sctrl_apb_lane0_symbol_ds_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_cfg_sym_sts_clr     :  1;
        unsigned int lane0_cfg_sym_caputre1    :  3;
        unsigned int lane0_cfg_sym_caputre0    :  3;
        unsigned int lane0_cfg_symbol_ctl_load :  1;
        unsigned int reserved_0                : 24;
    } reg;
}usb_subsys_sctrl_apb_lane0_symbol_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_cfg_sync_sts_clr  :  1;
        unsigned int lane0_cfg_sync_caputre0 :  3;
        unsigned int lane0_cfg_sync_caputre1 :  3;
        unsigned int lane0_cfg_sync_ctl_load :  1;
        unsigned int reserved_0              : 24;
    } reg;
}usb_subsys_sctrl_apb_lane0_cfg_symbol_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_cfg_sync_sts_clr  :  1;
        unsigned int lane1_cfg_sync_caputre0 :  3;
        unsigned int lane1_cfg_sync_caputre1 :  3;
        unsigned int lane1_cfg_sync_ctl_load :  1;
        unsigned int reserved_0              : 24;
    } reg;
}usb_subsys_sctrl_apb_lane1_cfg_symbol_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_blk_disp_err_clr :  1;
        unsigned int lane0_blk_code_err_clr :  1;
        unsigned int lane0_blk_err_cnt_load :  1;
        unsigned int reserved_0             : 29;
    } reg;
}usb_subsys_sctrl_apb_lane0_blk_disp_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_blk_disp_err_clr :  1;
        unsigned int lane1_blk_code_err_clr :  1;
        unsigned int lane1_blk_err_cnt_load :  1;
        unsigned int reserved_0             : 29;
    } reg;
}usb_subsys_sctrl_apb_lane1_blk_disp_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_blk_disp_err_cnt : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_blk_disp_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_blk_code_err_cnt : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_blk_code_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_blk_det_data_err_cnt : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_blk_det_err_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_blk_disp_err_cnt : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_blk_disp_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_blk_code_err_cnt : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_blk_code_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_blk_det_data_err_cnt : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_blk_det_err_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pma_pwr_en_ovrd :  1;
        unsigned int reserved_0      : 31;
    } reg;
}usb_subsys_sctrl_apb_pma_pwr_en_ovrd_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int fladj_30mhz_reg :  6;
        unsigned int reserved_0      : 26;
    } reg;
}usb_subsys_sctrl_apb_fladj_30mhz_reg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ptm_time   : 32;
    } reg;
}usb_subsys_sctrl_apb_ptm_time_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ptm_time_vld :  1;
        unsigned int reserved_0   : 31;
    } reg;
}usb_subsys_sctrl_apb_ptm_time_vld_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tca_dp4_por :  1;
        unsigned int reserved_0  : 31;
    } reg;
}usb_subsys_sctrl_apb_tca_dp4_por_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_combophy_debug :  1;
        unsigned int reserved_0            : 31;
    } reg;
}usb_subsys_sctrl_apb_debug_rst_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_combophy_debug :  1;
        unsigned int reserved_0            : 31;
    } reg;
}usb_subsys_sctrl_apb_debug_rst_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_combophy_debug :  1;
        unsigned int reserved_0            : 31;
    } reg;
}usb_subsys_sctrl_apb_debug_rst_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_combophy_debug_gate :  1;
        unsigned int reserved_0                 : 31;
    } reg;
}usb_subsys_sctrl_apb_debug_clk_gate_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_combophy_debug_gate :  1;
        unsigned int reserved_0                 : 31;
    } reg;
}usb_subsys_sctrl_apb_debug_clk_gate_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_combophy_debug_gate :  1;
        unsigned int reserved_0                 : 31;
    } reg;
}usb_subsys_sctrl_apb_debug_clk_gate_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_usbc_debug :  1;
        unsigned int reserved_0        : 31;
    } reg;
}usb_subsys_sctrl_apb_usb_debug_ctrl_rst_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_usbc_debug :  1;
        unsigned int reserved_0        : 31;
    } reg;
}usb_subsys_sctrl_apb_usb_debug_ctrl_rst_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_usbc_debug :  1;
        unsigned int reserved_0        : 31;
    } reg;
}usb_subsys_sctrl_apb_usb_debug_ctrl_rst_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_usbc_debug_gate :  1;
        unsigned int reserved_0             : 31;
    } reg;
}usb_subsys_sctrl_apb_usb_debug_ctrl_clk_gate_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_usbc_debug_gate :  1;
        unsigned int reserved_0             : 31;
    } reg;
}usb_subsys_sctrl_apb_usb_debug_ctrl_clk_gate_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_usbc_debug_gate :  1;
        unsigned int reserved_0             : 31;
    } reg;
}usb_subsys_sctrl_apb_usb_debug_ctrl_clk_gate_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_usb_ref_alt_lp_gate :  1;
        unsigned int reserved_0                 : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_clk_usb_ref_alt_lp_gate_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_usb_ref_alt_lp_gate :  1;
        unsigned int reserved_0                 : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_clk_usb_ref_alt_lp_gate_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_usb_ref_alt_lp_gate :  1;
        unsigned int reserved_0                 : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_clk_usb_ref_alt_lp_gate_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_usb_ref_alt_gate :  1;
        unsigned int reserved_0              : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_clk_usb_ref_alt_gate_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_usb_ref_alt_gate :  1;
        unsigned int reserved_0              : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_clk_usb_ref_alt_gate_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_clk_usb_ref_alt_gate :  1;
        unsigned int reserved_0              : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_clk_usb_ref_alt_gate_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int req_clk_controller_ref_enable  :  1;
        unsigned int req_clk_usb_ref_alt_enable     :  1;
        unsigned int req_clk_usb_bus_early_enable   :  1;
        unsigned int req_clk_usb_eusb_ref_enable    :  1;
        unsigned int req_clk_usb_combophy_fw_enable :  1;
        unsigned int reserved_0                     : 27;
    } reg;
}usb_subsys_sctrl_apb_req_clk_ip_enable_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_lane1_encdec_bypass     :  1;
        unsigned int usb32_pipe_lane1_if_width          :  2;
        unsigned int usb32_pipe_lane1_link_num          :  4;
        unsigned int usb32_pipe_lane1_maxpclk_div_ratio :  4;
        unsigned int usb32_pipe_lane1_maxpclkreq        :  2;
        unsigned int usb32_pipe_lane1_pclkchangeack     :  1;
        unsigned int usb32_pipe_lane1_powerdown         :  4;
        unsigned int usb32_pipe_lane1_rate              :  4;
        unsigned int usb32_pipe_lane1_reset_n           :  1;
        unsigned int usb32_pipe_lane1_disable           :  1;
        unsigned int usb32_pipe_lane1_tx2rx_loopbk      :  1;
        unsigned int usb32_pipe_rx1_blk_align_ctl       :  1;
        unsigned int usb32_pipe_rx1_eq_training         :  1;
        unsigned int usb32_pipe_rx1_es_mode             :  1;
        unsigned int usb32_pipe_rx1_if_width            :  2;
        unsigned int usb32_pipe_rx1_polarity            :  1;
        unsigned int reserved_0                         :  1;
    } reg;
}usb_subsys_sctrl_apb_combophy_usb32_pipe_lane1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_rx1_termination :  1;
        unsigned int usb32_pipe_rx1_standby     :  1;
        unsigned int cfg_usb_maxpclk_req        :  2;
        unsigned int reserved_0                 : 28;
    } reg;
}usb_subsys_sctrl_apb_combophy_usb32_pipe_rx1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_tx1_datak      :  4;
        unsigned int usb32_pipe_tx1_datavalid  :  1;
        unsigned int usb32_pipe_tx1_deemph     : 18;
        unsigned int usb32_pipe_tx1_detectrx   :  1;
        unsigned int usb32_pipe_tx1_elecidle   :  1;
        unsigned int usb32_pipe_tx1_ones_zeros :  1;
        unsigned int usb32_pipe_tx1_stratblock :  1;
        unsigned int usb32_pipe_tx1_syncheader :  4;
        unsigned int reserved_0                :  1;
    } reg;
}usb_subsys_sctrl_apb_combophy_usb32_pipe_tx1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_lane1_databuswidth  :  2;
        unsigned int usb32_pipe_lane1_phy_satus     :  1;
        unsigned int usb32_pipe_lane1_power_present :  1;
        unsigned int usb32_pipe_lane1_ref_clk_req_n :  1;
        unsigned int usb32_pipe_lane1_datak         :  4;
        unsigned int usb32_pipe_lane1_datavalid     :  1;
        unsigned int usb32_pipe_rx1_ebuff_loaction  :  9;
        unsigned int usb32_pipe_rx1_elecidle        :  1;
        unsigned int usb32_pipe_rx1_standby_status  :  1;
        unsigned int usb32_pipe_rx1_startblock      :  1;
        unsigned int usb32_pipe_rx1_status          :  3;
        unsigned int usb32_pipe_rx1_syncheader      :  4;
        unsigned int usb32_pipe_rx1_valid           :  1;
        unsigned int usb32_pipe_lane1_pclkchangeok  :  1;
        unsigned int reserved_0                     :  1;
    } reg;
}usb_subsys_sctrl_apb_combophy_usb32_pipes_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int usb32_pipe_lane1_maxpclkack :  2;
        unsigned int reserved_0                  : 30;
    } reg;
}usb_subsys_sctrl_apb_usb32_pipe_lane1_maxpclkack_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_blk_blk_length_err_cnt : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_blk_blk_length_err_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_blk_blk_length_err_cnt : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_blk_blk_length_err_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane0_blk_dec_mult_err_cnt : 32;
    } reg;
}usb_subsys_sctrl_apb_lane0_blk_dec_mult_err_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lane1_blk_dec_mult_err_cnt : 32;
    } reg;
}usb_subsys_sctrl_apb_lane1_blk_dec_mult_err_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_usb_vaux_reg_n :  1;
        unsigned int reserved_0            : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_usb_vaux_reg_n_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_usb_vaux_reg_n :  1;
        unsigned int reserved_0            : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_usb_vaux_reg_n_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_usb_vaux_reg_n :  1;
        unsigned int reserved_0            : 31;
    } reg;
}usb_subsys_sctrl_apb_ip_rst_usb_vaux_reg_n_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pm_power_state_request :  2;
        unsigned int reserved_0             : 30;
    } reg;
}usb_subsys_sctrl_apb_pm_power_state_request_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}usb_subsys_sctrl_apb_regfile_cgbypass_t;

#endif
