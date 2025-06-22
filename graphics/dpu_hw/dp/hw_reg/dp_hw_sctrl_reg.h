/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _DP_HW_SCTRL_REG_H_
#define _DP_HW_SCTRL_REG_H_

#define EXT_SDP_CFG_OFFSET0                                        0x0000
#define CFG_EXT_SDP_TX_INT_ENABLE_SHIFT                            0
#define CFG_EXT_SDP_TX_INT_ENABLE_LEN                              1

#define EXT_SDP_CFG_OFFSET1                                        0x0004
#define CFG_EXT_SDP_TX_ENABLE1_SHIFT                               0
#define CFG_EXT_SDP_TX_ENABLE1_LEN                                 1
#define CFG_EXT_SDP_TX_ENABLE2_SHIFT                               1
#define CFG_EXT_SDP_TX_ENABLE2_LEN                                 1
#define CFG_EXT_SDP_TX_ENABLE3_SHIFT                               2
#define CFG_EXT_SDP_TX_ENABLE3_LEN                                 1
#define CFG_EXT_SDP_TX_ENABLE4_SHIFT                               3
#define CFG_EXT_SDP_TX_ENABLE4_LEN                                 1

#define EXT_SDP_CFG_OFFSET2                                        0x0008
#define CFG_EXT_SDP_TX_LINE_NUM3_SHIFT                             0
#define CFG_EXT_SDP_TX_LINE_NUM3_LEN                               16
#define CFG_EXT_SDP_TX_LINE_NUM4_SHIFT                             16
#define CFG_EXT_SDP_TX_LINE_NUM4_LEN                               16

#define EXT_SDP_CFG_OFFSET3                                        0x000c
#define CFG_EXT_SDP_TX_LINE_NUM1_SHIFT                             0
#define CFG_EXT_SDP_TX_LINE_NUM1_LEN                               16
#define CFG_EXT_SDP_TX_LINE_NUM2_SHIFT                             16
#define CFG_EXT_SDP_TX_LINE_NUM2_LEN                               16

#define EXT_SDP_CFG_OFFSET4                                        0x0010
#define CFG_EXT_SDP_LEN1_SHIFT                                     0
#define CFG_EXT_SDP_LEN1_LEN                                       2
#define CFG_EXT_SDP_LEN2_SHIFT                                     2
#define CFG_EXT_SDP_LEN2_LEN                                       2
#define CFG_EXT_SDP_LEN3_SHIFT                                     4
#define CFG_EXT_SDP_LEN3_LEN                                       2
#define CFG_EXT_SDP_LEN4_SHIFT                                     6
#define CFG_EXT_SDP_LEN4_LEN                                       2
#define CFG_EXT_SDP_HORIZONTAL1_SHIFT                              8
#define CFG_EXT_SDP_HORIZONTAL1_LEN                                1
#define CFG_EXT_SDP_HORIZONTAL2_SHIFT                              9
#define CFG_EXT_SDP_HORIZONTAL2_LEN                                1
#define CFG_EXT_SDP_HORIZONTAL3_SHIFT                              10
#define CFG_EXT_SDP_HORIZONTAL3_LEN                                1
#define CFG_EXT_SDP_HORIZONTAL4_SHIFT                              11
#define CFG_EXT_SDP_HORIZONTAL4_LEN                                1

#define COMBO_PHY_OBSERVE_OFFSET                                   0x0018
#define DPTX_PHY_LANE0_PHYSTATUS_SHIFT                             0
#define DPTX_PHY_LANE0_PHYSTATUS_LEN                               1
#define DPTX_PHY_LANE1_PHYSTATUS_SHIFT                             1
#define DPTX_PHY_LANE1_PHYSTATUS_LEN                               1
#define DPTX_PHY_LANE2_PHYSTATUS_SHIFT                             2
#define DPTX_PHY_LANE2_PHYSTATUS_LEN                               1
#define DPTX_PHY_LANE3_PHYSTATUS_SHIFT                             3
#define DPTX_PHY_LANE3_PHYSTATUS_LEN                               1
#define DPALT_DISABLE_SHIFT                                        22
#define DPALT_DISABLE_LEN                                          1
#define DPALT_DP4_SHIFT                                            23
#define DPALT_DP4_LEN                                              1
#define PIPE_LANE0_PCLKCHANGEOK_SHIFT                              24
#define PIPE_LANE0_PCLKCHANGEOK_LEN                                1
#define PIPE_LANE1_PCLKCHANGEOK_SHIFT                              25
#define PIPE_LANE1_PCLKCHANGEOK_LEN                                1
#define PIPE_LANE0_MAXPCLKACK_SHIFT                                26
#define PIPE_LANE0_MAXPCLKACK_LEN                                  2
#define PIPE_LANE1_MAXPCLKACK_SHIFT                                28
#define PIPE_LANE1_MAXPCLKACK_LEN                                  2

#define EXT_SDP_ERR_OFFSET                                         0x001c
#define EXT_SDP_TX_APB_ERR1_SHIFT                                  0
#define EXT_SDP_TX_APB_ERR1_LEN                                    1
#define EXT_SDP_TX_APB_ERR2_SHIFT                                  1
#define EXT_SDP_TX_APB_ERR2_LEN                                    1
#define EXT_SDP_TX_APB_ERR3_SHIFT                                  2
#define EXT_SDP_TX_APB_ERR3_LEN                                    1
#define EXT_SDP_TX_APB_ERR4_SHIFT                                  3
#define EXT_SDP_TX_APB_ERR4_LEN                                    1

#define EXT_SDP_DATA2_OFFSET                                       0x0020
#define EXT_SDP_DATA3_OFFSET                                       0x0044
#define EXT_SDP_DATA4_OFFSET                                       0x0068
#define EXT_SDP_DATA1_OFFSET                                       0x008c

#define DP_MUX_SEL_CFG_OFFSET                                      0x0014
#define CFG_DPTX_IPI_MUX_SEL_SHIFT                                 1
#define CFG_DPTX_IPI_MUX_SEL_LEN                                   1
#define CFG_DPTX_PHY_LANE0_RST_MUX_SEL_SHIFT                       2
#define CFG_DPTX_PHY_LANE0_RST_MUX_SEL_LEN                         1

#define EXT_SDP_TX_VG_SOFT_RESET_OFFSET                            0x00c8
#define IP_RST_DPTX_EXT_SDP_TX_N_SHIFT                             0
#define IP_RST_DPTX_EXT_SDP_TX_N_LEN                               1
#define IP_RST_DPTX_VG_N_SHIFT                                     1
#define IP_RST_DPTX_VG_N_LEN                                       1
#define IP_RST_DPTX_PHY_LANE0_N_SHIFT                              2
#define IP_RST_DPTX_PHY_LANE0_N_LEN                                1
#define IP_RST_DPTX_PHY_LANE1_N_SHIFT                              3
#define IP_RST_DPTX_PHY_LANE1_N_LEN                                1
#define IP_RST_DPTX_PHY_LANE2_N_SHIFT                              4
#define IP_RST_DPTX_PHY_LANE2_N_LEN                                1
#define IP_RST_DPTX_PHY_LANE3_N_SHIFT                              5
#define IP_RST_DPTX_PHY_LANE3_N_LEN                                1
#define IP_RST_DPTX_PHY_N_SHIFT                                    6
#define IP_RST_DPTX_PHY_N_LEN                                      1
#define IP_RST_DPTX_CONTROLLER_N_SHIFT                             7
#define IP_RST_DPTX_CONTROLLER_N_LEN                               1

#define CLK_GATE_EN_OFFSET                                         0x00d4
#define GT_CLK_DPTX_EXT_SDP_TX_SHIFT                               0
#define GT_CLK_DPTX_EXT_SDP_TX_LEN                                 1
#define GT_CLK_DPTX_PIXEL_VG_SHIFT                                 1
#define GT_CLK_DPTX_PIXEL_VG_LEN                                   1

#define DPTX_HPD_CFG_OFFSET                                        0x00f0

#define DPTX_PHY_CFG_OFFSET                                        0x00f4
#define DPALT_DISABLE_ACK_SHIFT                                    0
#define DPALT_DISABLE_ACK_LEN                                      1
#define PIPE_LANE0_PCLKCHANGEACK_SHIFT                             1
#define PIPE_LANE0_PCLKCHANGEACK_LEN                               1
#define PIPE_LANE1_PCLKCHANGEACK_SHIFT                             2
#define PIPE_LANE1_PCLKCHANGEACK_LEN                               1
#define PIPE_LANE0_MAXPCLKREQ_SHIFT                                3
#define PIPE_LANE0_MAXPCLKREQ_LEN                                  2
#define PIPE_LANE1_MAXPCLKREQ_SHIFT                                5
#define PIPE_LANE1_MAXPCLKREQ_LEN                                  2

#define CFG_POWERDOWN_RATE_SYNC_BYPASS                             0x0104

#define DPTX_VG_CONFIG_OFFSET0                                     0x010c
#define CFG_DPTX_HSA_SHIFT                                         16
#define CFG_DPTX_HSA_LEN                                           16
#define CFG_DPTX_HBP_SHIFT                                         0
#define CFG_DPTX_HBP_LEN                                           16

#define DPTX_VG_CONFIG_OFFSET1                                     0x0110
#define CFG_DPTX_HACT_SHIFT                                        16
#define CFG_DPTX_HACT_LEN                                          16
#define CFG_DPTX_HFP_SHIFT                                         0
#define CFG_DPTX_HFP_LEN                                           16

#define DPTX_VG_CONFIG_OFFSET2                                     0x0114
#define CFG_DPTX_VSA_SHIFT                                         16
#define CFG_DPTX_VSA_LEN                                           16
#define CFG_DPTX_VBP_SHIFT                                         0
#define CFG_DPTX_VBP_LEN                                           16

#define DPTX_VG_CONFIG_OFFSET3                                     0x0118
#define CFG_DPTX_VACT_SHIFT                                        16
#define CFG_DPTX_VACT_LEN                                          16
#define CFG_DPTX_VFP_SHIFT                                         0
#define CFG_DPTX_VFP_LEN                                           16

#define DPTX_VG_CONFIG_OFFSET4                                     0x00e0

#define DPTX_VG_CONFIG_OFFSET5                                     0x00e4

#define DPTX_VG_CONFIG_OFFSET6                                     0x00e8
#define CFG_DPTX_IVG_EN_SHIFT                                      0
#define CFG_DPTX_IVG_EN_LEN                                        1
#define CFG_DPTX_DPMODE_SHIFT                                      1
#define CFG_DPTX_DPMODE_LEN                                        2
#define CFG_DPTX_COLOR_DEPTH_SHIFT                                 3
#define CFG_DPTX_COLOR_DEPTH_LEN                                   2
#define CFG_DPTX_INTER_NUM_SHIFT                                   5
#define CFG_DPTX_INTER_NUM_LEN                                     16

#define DPTX_AUX_CFG_OFFSET                                        0x00ec
#define DP_AUX_CTRL_SHIFT                                          0
#define DP_AUX_CTRL_LEN                                            4
#define DP_AUX_DP_DN_SWAP_SHIFT                                    4
#define DP_AUX_DP_DN_SWAP_LEN                                      1
#define DP_AUX_HYS_TUNE_SHIFT                                      6
#define DP_AUX_HYS_TUNE_LEN                                        2
#define CFG_DP_AUX_OE_SHIFT                                        8
#define CFG_DP_AUX_OE_LEN                                          1
#define DP_AUX_PWDNB_SHIFT                                         9
#define DP_AUX_PWDNB_LEN                                           1
#define DP_AUX_VOD_TUNE_SHIFT                                      10
#define DP_AUX_VOD_TUNE_LEN                                        1

#define DPTX_IDLE_CONFIG_OFFSET                                    0X0840
#define DPTX_IDLE_SHIFT                                            0
#define DPTX_IDLE_LEN                                              1

#endif
