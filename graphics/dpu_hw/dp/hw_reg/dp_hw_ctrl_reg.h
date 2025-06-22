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
#ifndef _DP_HW_REG_CTRL_H_
#define _DP_HW_REG_CTRL_H_

/* DPTX REGISTER */
#define DPTX_VERSION_NUMBER_OFFSET                                 0x0
#define DPTX_VERSION_TYPE_OFFSET                                   0x4

#define DPTX_ID_OFFSET                                             0x8
#define VENDOR_ID_SHIFT                                            0
#define VENDOR_ID_LEN                                              16
#define DEVICE_ID_SHIFT                                            16
#define DEVICE_ID_LEN                                              16

#define DPTX_CONFIG_REG_OFFSET1                                    0x100
#define HDCP_SELECT_SHIFT                                          0
#define HDCP_SELECT_LEN                                            1
#define AUDIO_SELECT_SHIFT                                         1
#define AUDIO_SELECT_LEN                                           2
#define PHY_USED_SHIFT                                             3
#define PHY_USED_LEN                                               1
#define SDP_REG_BANK_SIZE_SHIFT                                    4
#define SDP_REG_BANK_SIZE_LEN                                      5
#define FPGA_EN_SHIFT                                              9
#define FPGA_EN_LEN                                                1
#define DPK_ROMLESS_SHIFT                                          10
#define DPK_ROMLESS_LEN                                            1
#define DPK_8BIT_SHIFT                                             11
#define DPK_8BIT_LEN                                               1
#define SYNC_DEPTH_SHIFT                                           12
#define SYNC_DEPTH_LEN                                             2
#define NUM_STREAMS_SHIFT                                          16
#define NUM_STREAMS_LEN                                            3
#define MP_MODE_SHIFT                                              19
#define MP_MODE_LEN                                                3
#define DSC_EN_SHIFT                                               22
#define DSC_EN_LEN                                                 1
#define EDP_EN_SHIFT                                               23
#define EDP_EN_LEN                                                 1
#define FEC_EN_SHIFT                                               24
#define FEC_EN_LEN                                                 1
#define GEN2_PHY_SHIFT                                             29
#define GEN2_PHY_LEN                                               1

#define DPTX_CONFIG_REG_OFFSET3                                    0x108
#define PHY_TYPE_SHIFT                                             7
#define PHY_TYPE_LEN                                               2
#define ADSYNC_EN_SHIFT                                            16
#define ADSYNC_EN_LEN                                              1
#define PSR_VER_SHIFT                                              20
#define PSR_VER_LEN                                                2

#define CCTL_OFFSET                                                0x200
#define SCRAMBLE_DIS_SHIFT                                         0
#define SCRAMBLE_DIS_LEN                                           1
#define ENHANCE_FRAMING_EN_SHIFT                                   1
#define ENHANCE_FRAMING_EN_LEN                                     1
#define DEFAULT_FAST_LINK_TRAIN_EN_SHIFT                           2
#define DEFAULT_FAST_LINK_TRAIN_EN_LEN                             1
#define SCALE_DOWN_MODE_SHIFT                                      3
#define SCALE_DOWN_MODE_LEN                                        1
#define FORCE_HPD_SHIFT                                            4
#define FORCE_HPD_LEN                                              1
#define DISABLE_INTERLEAVING_SHIFT                                 5
#define DISABLE_INTERLEAVING_LEN                                   1
#define SEL_AUX_TIMEOUT_32MS_SHIFT                                 6
#define SEL_AUX_TIMEOUT_32MS_LEN                                   1
#define DEBUG_CONTROL_SHIFT                                        8
#define DEBUG_CONTROL_LEN                                          4
#define SR_SCALE_DOWN_SHIFT                                        16
#define SR_SCALE_DOWN_LEN                                          1
#define BS_512_SCALE_DOWN_SHIFT                                    17
#define BS_512_SCALE_DOWN_LEN                                      8
#define ENABLE_MST_MODE_SHIFT                                      25
#define ENABLE_MST_MODE_LEN                                        1
#define ENABLE_FEC_SHIFT                                           26
#define ENABLE_FEC_LEN                                             1
#define ENABLE_eDP_SHIFT                                           27
#define ENABLE_eDP_LEN                                             1
#define INITIATE_MST_ACT_SEQ_SHIFT                                 28
#define INITIATE_MST_ACT_SEQ_LEN                                   1
#define ENHANCE_FRAMING_WITH_FEC_EN_SHIFT                          29
#define ENHANCE_FRAMING_WITH_FEC_EN_LEN                            1

#define SOFT_RESET_CTRL_OFFSET                                     0x204
#define CONTROLLER_RESET_SHIFT                                     0
#define CONTROLLER_RESET_LEN                                       1
#define PHY_SOFT_RESET_SHIFT                                       1
#define PHY_SOFT_RESET_LEN                                         1
#define HDCP_MODULE_RESET_SHIFT                                    2
#define HDCP_MODULE_RESET_LEN                                      1
#define AUDIO_SAMPLER_RESET_SHIFT                                  3
#define AUDIO_SAMPLER_RESET_LEN                                    1
#define AUX_RESET_SHIFT                                            4
#define AUX_RESET_LEN                                              1
#define VIDEO_RESET_SHIFT                                          5
#define VIDEO_RESET_LEN                                            4
# define VIDEO_RESET_STREAM_SHIFT0                                 5
# define VIDEO_RESET_STREAM_LEN0                                   1
# define VIDEO_RESET_STREAM_SHIFT1                                 6
# define VIDEO_RESET_STREAM_LEN1                                   1
# define VIDEO_RESET_STREAM_SHIFT2                                 7
# define VIDEO_RESET_STREAM_LEN2                                   1
# define VIDEO_RESET_STREAM_SHIFT3                                 8
# define VIDEO_RESET_STREAM_LEN3                                   1
#define AUDIO_SAMPLER_RESET_STREAM_SHIFT1                          9
#define AUDIO_SAMPLER_RESET_STREAM_LEN1                            1
#define AUDIO_SAMPLER_RESET_STREAM_SHIFT2                          10
#define AUDIO_SAMPLER_RESET_STREAM_LEN2                            1
#define AUDIO_SAMPLER_RESET_STREAM_SHIFT3                          11
#define AUDIO_SAMPLER_RESET_STREAM_LEN3                            1
#define AUX_CDR_STATE_SHIFT                                        16
#define AUX_CDR_STATE_LEN                                          4
#define AUX_CDR_CLOCK_CYCLE_SHIFT                                  24
#define AUX_CDR_CLOCK_CYCLE_LEN	                                   5

#define DSC_CTL_OFFSET                                             0x234
#define PPS_REPEAT_CNT_SHIFT                                       16
#define PPS_REPEAT_CNT_LEN                                         2

#define DSC_INTSTS_OFFSET                                          0x238
#define DSC_STREAM0_CRC_READY_SHIFT                                8
#define DSC_STREAM0_CRC_READY_LEN                                  1
#define DSC_STREAM1_CRC_READY_SHIFT                                9
#define DSC_STREAM1_CRC_READY_LEN                                  1
#define DSC_STREAM2_CRC_READY_SHIFT                                10
#define DSC_STREAM2_CRC_READY_LEN                                  1
#define DSC_STREAM3_CRC_READY_SHIFT                                11
#define DSC_STREAM3_CRC_READY_LEN                                  1
#define CDS_FIFO_OVERFLOW_STREAM0_SHIFT                            12
#define CDS_FIFO_OVERFLOW_STREAM0_LEN                              1
#define CDS_FIFO_OVERFLOW_STREAM1_SHIFT                            13
#define CDS_FIFO_OVERFLOW_STREAM1_LEN                              1
#define CDS_FIFO_OVERFLOW_STREAM2_SHIFT                            14
#define CDS_FIFO_OVERFLOW_STREAM2_LEN                              1
#define CDS_FIFO_OVERFLOW_STREAM3_SHIFT	                           15
#define CDS_FIFO_OVERFLOW_STREAM3_LEN                              1
#define CDS_FIFO_UNDERFLOW_STREAM0_SHIFT                           16
#define CDS_FIFO_UNDERFLOW_STREAM0_LEN                             1
#define CDS_FIFO_UNDERFLOW_STREAM1_SHIFT                           17
#define CDS_FIFO_UNDERFLOW_STREAM1_LEN                             1
#define CDS_FIFO_UNDERFLOW_STREAM2_SHIFT                           18
#define CDS_FIFO_UNDERFLOW_STREAM2_LEN                             1
#define CDS_FIFO_UNDERFLOW_STREAM3_SHIFT                           19
#define CDS_FIFO_UNDERFLOW_STREAM3_LEN                             1

#define PM_INT_STS_OFFSET                                          0x240
#define ML_POWEROFF_STS_CHG_SHIFT                                  0
#define ML_POWEROFF_STS_CHG_LEN                                    1
#define ALPM_FW_EXIT_LATENCY_ELAPSED_SHIFT                         4
#define ALPM_FW_EXIT_LATENCY_ELAPSED_LEN                           1
#define ALPM_AUX_PHY_WAKE_ACK_SHIFT                                5
#define ALPM_AUX_PHY_WAKE_ACK_LEN                                  1
#define ALPM_AUX_PHY_WAKE_ACK_TIMEOUT_SHIFT                        6
#define ALPM_AUX_PHY_WAKE_ACK_TIMEOUT_LEN                          1

#define PM_INT_EN_OFFSET                                           0x244
#define ML_POWEROFF_STS_CHG_EN_SHIFT                               0
#define ML_POWEROFF_STS_CHG_EN_LEN                                 1
#define ALPM_FW_EXIT_LATENCY_ELAPSED_EN_SHIFT                      4
#define ALPM_FW_EXIT_LATENCY_ELAPSED_EN_LEN                        1
#define ALPM_AUX_PHY_WAKE_ACK_EN_SHIFT                             5
#define ALPM_AUX_PHY_WAKE_ACK_EN_LEN                               1
#define ALPM_AUX_PHY_WAKE_ACK_TIMEOUT_EN_SHIFT                     6
#define ALPM_AUX_PHY_WAKE_ACK_TIMEOUT_EN_LEN                       1

#define VSAMPLE_CTRL_OFFSET                                        0x300
#define VIDEO_STREAM_ENABLE_SHIFT                                  5
#define VIDEO_STREAM_ENABLE_LEN                                    1
#define VIDEO_MAPPING_IPI_EN_SHIFT                                 15
#define VIDEO_MAPPING_IPI_EN_LEN                                   1
#define VIDEO_MAPPING_SHIFT                                        16
#define VIDEO_MAPPING_LEN                                          5
#define PIXEL_MODE_SELECT_SHIFT                                    21
#define PIXEL_MODE_SELECT_LEN                                      2
#define ENABLE_DSC_SHIFT                                           23
#define ENABLE_DSC_LEN                                             1
#define ENCRYPTION_ENABLE_SHIFT                                    24
#define ENCRYPTION_ENABLE_LEN                                      1
#define STREAM_TYPE_SHIFT                                          25
#define STREAM_TYPE_LEN                                            1
#define LINK_UPD_PPS_SHIFT                                         27
#define LINK_UPD_PPS_LEN                                           1

#define VSAMPLE_STUFF_CTRL_OFFSET1                                 0x304
#define BCB_DATA_STUFFING_EN_SHIFT                                 0
#define BCB_DATA_STUFFING_EN_LEN                                   1
#define RCR_DATA_STUFFING_EN_SHIFT                                 1
#define RCR_DATA_STUFFING_EN_LEN                                   1
#define GY_DATA_STUFFING_EN_SHIFT                                  2
#define GY_DATA_STUFFING_EN_LEN                                    1
#define BCB_STUFF_DATA_SHIFT                                       16
#define BCB_STUFF_DATA_LEN                                         16

#define VSAMPLE_STUFF_CTRL_OFFSET2                                 0x308
#define RCR_STUFF_DAT_SHIFT                                        0
#define RCR_STUFF_DAT_LEN                                          16
#define GY_STUFF_DATA_SHIFT                                        16
#define GY_STUFF_DATA_LEN                                          16

#define VINPUT_POLARITY_CTRL_OFFSET                                0x30c
#define VSYNC_IN_POLARITY_SHIFT                                    0
#define VSYNC_IN_POLARITY_LEN                                      1
#define HSYNC_IN_POLARITY_SHIFT                                    1
#define HSYNC_IN_POLARITY_LEN                                      1
#define DE_IN_POLARITY_SHIFT                                       2
#define DE_IN_POLARITY_LEN                                         1

#define VIDEO_CONFIG_OFFSET1                                       0x310
#define R_V_BLANK_IN_OSC_SHIFT                                     0
#define R_V_BLANK_IN_OSC_LEN                                       1
#define I_P_SHIFT                                                  1
#define I_P_LEN                                                    1
#define HBLANK_SHIFT                                               2
#define HBLANK_LEN                                                 14
#define HACTIVE_SHIFT                                              16
#define HACTIVE_LEN                                                16

#define VIDEO_CONFIG_OFFSET2                                       0x314
#define VACTIVE_SHIFT                                              0
#define VACTIVE_LEN                                                16
#define VBLANK_SHIFT                                               16
#define VBLANK_LEN                                                 16

#define VIDEO_CONFIG_OFFSET3                                       0x318
#define VIDEO_CONFIG_H_FRONT_PORCH_SHIFT                           0
#define VIDEO_CONFIG_H_FRONT_PORCH_LEN                             16
#define VIDEO_CONFIG_H_SYNC_WIDTH_SHIFT                            16
#define VIDEO_CONFIG_H_SYNC_WIDTH_LEN                              15

#define VIDEO_CONFIG_OFFSET4                                       0x31c
#define VIDEO_CONFIG_V_FRONT_PORCH_SHIFT                           0
#define VIDEO_CONFIG_V_FRONT_PORCH_LEN                             16
#define VIDEO_CONFIG_V_SYNC_WIDTH_SHIFT                            16
#define VIDEO_CONFIG_V_SYNC_WIDTH_LEN                              15

#define VIDEO_CONFIG_OFFSET5                                       0x320
#define AVERAGE_BYTES_PER_TU_SHIFT                                 0
#define AVERAGE_BYTES_PER_TU_LEN                                   7
#define INIT_THRESHOLD_SHIFT                                       7
#define INIT_THRESHOLD_LEN                                         7
#define AVERAGE_BYTES_PER_TU_FRAC_SST_SHIFT                        16
#define AVERAGE_BYTES_PER_TU_FRAC_SST_LEN                          4
#define AVERAGE_BYTES_PER_TU_FRAC_MST_SHIFT                        14
#define AVERAGE_BYTES_PER_TU_FRAC_MST_LEN                          6
#define ENABLE_3D_FRAME_FIELD_SEQ_SHIFT                            20
#define ENABLE_3D_FRAME_FIELD_SEQ_LEN                              1
#define INIT_THRESHOLD_HI_SHIFT                                    21
#define INIT_THRESHOLD_HI_LEN                                      2

#define VIDEO_MSA_OFFSET1                                          0x324
#define HSTART_SHIFT                                               0
#define HSTART_LEN                                                 16
#define VSTART_SHIFT                                               16
#define VSTART_LEN                                                 16

#define VIDEO_MSA_OFFSET2                                          0x328
#define MVID_SHIFT                                                 0
#define MVID_LEN                                                   24
#define MISC0_SHIFT                                                24
#define MISC0_LEN                                                  8
# define MISC0_MODE_SHIFT                                          0
# define MISC0_MODE_LEN                                            1
# define MISC0_ASYNCHRONOUS_MODE                                   0
# define MISC0_SYNCHRONOUS_MODE                                    1
# define MISC0_COLORIMETRY_SHIFT                                   1
# define MISC0_COLORIMETRY_LEN                                     4
# define MISC0_BPC_SHIFT                                           5
# define MISC0_BPC_LEN                                             3

#define VIDEO_MSA_OFFSET3                                          0x32c
#define NVID_SHIFT                                                 0
#define NVID_LEN                                                   24
#define MISC1_SHIFT                                                24
#define MISC1_LEN                                                  8
# define MISC1_COLORIMETRY_SHIFT                                   7
# define MISC1_COLORIMETRY_LEN                                     1
#define PIX_ENC_SHIFT                                              31
#define PIX_ENC_LEN                                                1
#define PIX_ENC_YCBCR420_SHIFT                                     30
#define PIX_ENC_YCBCR420_LEN                                       1

#define VIDEO_HBLANK_INTERVAL_OFFSET                               0x330
#define HBLANK_INTERVAL_SHIFT                                      0
#define HBLANK_INTERVAL_LEN                                        16

#define VIDEO_DSCCFG_OFFSET                                        0x334
#define DSC_CRC_ENABLE_SHIFT                                       10
#define DSC_CRC_ENABLE_LEN                                         1

#define MVID_CONFIG_OFFSET1                                        0x338
#define MVID_CUST_EN_SHIFT                                         0
#define MVID_CUST_EN_LEN                                           1
#define MVID_CUST_DEN_SHIFT                                        16
#define MVID_CUST_DEN_LEN                                          16

#define MVID_CONFIG_OFFSET2                                        0x33c
#define MVID_CUST_MOD_SHIFT                                        0
#define MVID_CUST_MOD_LEN                                          15
#define MVID_CUST_QUO_SHIFT                                        16
#define MVID_CUST_QUO_LEN                                          16

#define PM_CONFIG_OFFSET1                                          0x350
#define ML_POWEROFF_START_LINE_SHIFT                               0
#define ML_POWEROFF_START_LINE_LEN                                 16
#define ML_POWEROFF_START_LINE_EN_SHIFT                            16
#define ML_POWEROFF_START_LINE_EN_LEN                              1
#define ML_POWEROFF_COMPLETE_FEC_BLK_DIS_SHIFT                     17
#define ML_POWEROFF_COMPLETE_FEC_BLK_DIS_LEN                       1
#define ML_POWEROFF_SEQ_IF_EN_SHIFT                                18
#define ML_POWEROFF_SEQ_IF_EN_LEN                                  1
#define ML_POWEROFF_IF_EN_SHIFT                                    19
#define ML_POWEROFF_IF_EN_LEN                                      1
#define ML_POWEROFF_SEQ_CNT_SHIFT                                  20
#define ML_POWEROFF_SEQ_CNT_LEN                                    8
#define ML_POWEROFF_SEQ_SHIFT                                      28
#define ML_POWEROFF_SEQ_LEN                                        1
#define ALPM_AUX_PHY_WAKE_EN_SHIFT                                 31
#define ALPM_AUX_PHY_WAKE_EN_LEN                                   1

#define PM_CONFIG_OFFSET2                                          0x354
#define ML_POWEROFF_PHY_HOLD_PATTERN_CNT_SHIFT                     0
#define ML_POWEROFF_PHY_HOLD_PATTERN_CNT_LEN                       8
#define ML_POWEROFF_IDLE_PATTERN_CNT_SHIFT                         16
#define ML_POWEROFF_IDLE_PATTERN_CNT_LEN                           8

#define PM_CTRL_OFFSET1                                            0x360
#define ML_POWEROFF_SHIFT                                          0
#define ML_POWEROFF_LEN                                            1

#define PM_STS_OFFSET1                                             0x370
#define ML_POWEROFF_STS_SHIFT                                      0
#define ML_POWEROFF_STS_LEN                                        1

#define AUD_CONFIG_OFFSET1                                         0x400
#define AUDIO_INF_SELECT_SHIFT                                     0
#define AUDIO_INF_SELECT_LEN                                       1
#define AUDIO_DATA_IN_EN_SHIFT                                     1
#define AUDIO_DATA_IN_EN_LEN                                       4
#define AUDIO_DATA_WIDTH_SHIFT                                     5
#define AUDIO_DATA_WIDTH_LEN                                       5
#define HBR_MODE_ENABLE_SHIFT                                      10
#define HBR_MODE_ENABLE_LEN                                        1
#define NUM_CHANNELS_SHIFT                                         12
#define NUM_CHANNELS_LEN                                           3
#define AUDIO_MUTE_SHIFT                                           15
#define AUDIO_MUTE_LEN                                             1
#define AUDIO_PACKET_ID_SHIFT                                      16
#define AUDIO_PACKET_ID_LEN                                        8
#define AUDIO_TIMESTAMP_VERSION_NUM_SHIFT                          24
#define AUDIO_TIMESTAMP_VERSION_NUM_LEN                            6
#define AUDIO_CLK_MULT_FS_SHIFT                                    30
#define AUDIO_CLK_MULT_FS_LEN                                      2

/* i range [0,3] */
#define GTC_CTRL_OFFSET0                                           0x460
#define GTC_ACCU_EN_SHIFT                                          0
#define GTC_ACCU_EN_LEN                                            1
#define GTC_AUX_WRITE_FIRST_CMD_EDGE_EN_SHIFT                      1
#define GTC_AUX_WRITE_FIRST_CMD_EDGE_EN_LEN                        1
#define GTC_AUX_READ_FIRST_CMD_EDGE_EN_SHIFT                       2
#define GTC_AUX_READ_FIRST_CMD_EDGE_EN_LEN                         1
#define GTC_AUX_READ_RXGTC_RECEPTION_EN_SHIFT                      3
#define GTC_AUX_READ_RXGTC_RECEPTION_EN_LEN                        1
#define AUXIN_PHY_DELAY_SHIFT                                      16
#define AUXIN_PHY_DELAY_LEN                                        8
#define AUXOUT_PHY_DELAY_SHIFT                                     24
#define AUXOUT_PHY_DELAY_LEN                                       8

#define GTC_CTRL_OFFSET1                                           0x464
#define GTC_ACCU_INCR_RATE_SHIFT                                   0
#define GTC_ACCU_INCR_RATE_LEN                                     20

#define GTC_CTRL_OFFSET2                                           0x468

#define GTC_CTRL_OFFSET3                                           0x46c
#define TX_GTC_ADD_VALUE_SHIFT                                     0
#define TX_GTC_ADD_VALUE_LEN                                       16

/* i range [0,2] */
#define GTC_STATUS_OFFSET0                                         0x480

#define GTC_STATUS_OFFSET1                                         0x484

#define GTC_STATUS_OFFSET2                                         0x488
#define AUX_FRAME_SYNC_CTRL_OFFSET0                                0x4a0
#define AUX_FRAMME_SYNC_VBE_SAMPLE_EN_SHIFT                        0
#define AUX_FRAMME_SYNC_VBE_SAMPLE_EN_LEN                          1

#define AUX_FRAME_SYNC_STATUS_OFFSET0                              0x4a8

#define SDP_VERTICAL_CTRL_OFFSET                                   0x500
#define EN_AUDIO_TIMESTAMP_SDP_SHIFT                               0
#define EN_AUDIO_TIMESTAMP_SDP_LEN                                 1
#define EN_AUDIO_STREAM_SDP_SHIFT                                  1
#define EN_AUDIO_STREAM_SDP_LEN                                    1
#define EN_VERTICAL_SDP_N_SHIFT                                    2
#define EN_VERTICAL_SDP_N_LEN                                      18
#define EN_128BYTES_SDP_1_SHIFT                                    24
#define EN_128BYTES_SDP_1_LEN                                      1
#define DISABLE_EXT_SDP_SHIFT                                      30
#define DISABLE_EXT_SDP_LEN                                        1
#define FIXED_PRIORITY_ARBITRATION_VERTICAL_SHIFT                  31
#define FIXED_PRIORITY_ARBITRATION_VERTICAL_LEN                    1

#define SDP_HORIZONTAL_CTRL_OFFSET                                 0x504
#define EN_HORIZONTAL_SDP_N_SHIFT                                  2
#define EN_HORIZONTAL_SDP_N_LEN                                    18
#define FIXED_PRIORITY_ARBITRATION_HORIZONTAL_SHIFT                31
#define FIXED_PRIORITY_ARBITRATION_HORIZONTAL_LEN                  1

#define SDP_STATUS_REGISTER_OFFSET                                 0x508
#define AUDIO_TIMESTAMP_SDP_STATUS_SHIFT                           0
#define AUDIO_TIMESTAMP_SDP_STATUS_LEN                             1
#define AUDIO_STREAM_SDP_STATUS_SHIFT                              1
#define AUDIO_STREAM_SDP_STATUS_LEN                                1
#define SDP_N_TX_STATUS_SHIFT                                      2
#define SDP_N_TX_STATUS_LEN                                        18

#define SDP_MANUAL_CTRL_OFFSET                                     0x50c
#define MANUAL_MODE_SDP_SHIFT                                      0
#define MANUAL_MODE_SDP_LEN                                        20

#define SDP_STATUS_EN_OFFSET                                       0x510
#define AUDIO_TIMESTAMP_SDP_STATUS_EN_SHIFT                        0
#define AUDIO_TIMESTAMP_SDP_STATUS_EN_LEN                          1
#define AUDIO_STREAM_SDP_STATUS_EN_SHIFT                           1
#define AUDIO_STREAM_SDP_STATUS_EN_LEN                             1
#define SDP_STATUS_EN_SHIFT                                        2
#define SDP_STATUS_EN_LEN                                          18

/* i range [1,3] */
#define SDP_CONFIG_OFFSET1                                         0x520
#define SDP_16B_BYTES_REQD_VBLANK_OVR_SHIFT                        0
#define SDP_16B_BYTES_REQD_VBLANK_OVR_LEN                          8
#define SDP_16B_BYTES_REQD_HBLANK_OVR_SHIFT                        16
#define SDP_16B_BYTES_REQD_HBLANK_OVR_LEN                          8

#define SDP_CONFIG_OFFSET2                                         0x524
#define SDP_32B_BYTES_REQD_VBLANK_OVR_SHIFT                        0
#define SDP_32B_BYTES_REQD_VBLANK_OVR_LEN                          8
#define SDP_32B_BYTES_REQD_HBLANK_OVR_SHIFT                        16
#define SDP_32B_BYTES_REQD_HBLANK_OVR_LEN                          8

#define SDP_CONFIG_OFFSET3                                         0x528
#define SDP_128B_BYTES_REQD_VBLANK_OVR_SHIFT                       0
#define SDP_128B_BYTES_REQD_VBLANK_OVR_LEN                         8
#define SDP_128B_BYTES_REQD_HBLANK_OVR_SHIFT                       16
#define SDP_128B_BYTES_REQD_HBLANK_OVR_LEN                         8

/* i range [0,DWC_DPTX_NUM_SDP_REGISTER_BANKS * 9 - 1] */
#define SDP_REGISTER_BANK_OFFSET(i)                                (0x600 + (i) * 0x4)

#define PHYIF_CTRL_OFFSET                                          0xa00
#define TPS_SEL_SHIFT                                              0
#define TPS_SEL_LEN                                                4
#define PHY_RATE_LEN                                               2
#define PHY_RATE_SHIFT                                             4
# define PHY_RATE_RBR                                              0x0
# define PHY_RATE_HBR                                              0x1
# define PHY_RATE_HBR2                                             0x2
# define PHY_RATE_HBR3                                             0x3
#define PHY_LANES_SHIFT                                            6
#define PHY_LANES_LEN                                              2
# define PHY_LANE_CNT_1                                            0
# define PHY_LANE_CNT_2                                            1
# define PHY_LANE_CNT_4                                            2
#define XMIT_ENABLE_SHIFT                                          8
#define XMIT_ENABLE_LEN                                            4
#define PHY_BUSY_SHIFT                                             12
#define PHY_BUSY_LEN                                               4
#define SSC_DIS_SHIFT                                              16
#define SSC_DIS_LEN                                                1
#define PHY_POWERDOWN_SHIFT                                        17
#define PHY_POWERDOWN_LEN                                          4
# define PHY_POWERDOWN_STATE_POWER_ON                              0x0
# define PHY_POWERDOWN_STATE_INTER_P2_POWER                        0x2
# define PHY_POWERDOWN_STATE_POWER_DOWN                            0x3
# define PHY_POWERDOWN_STATE_P4_POWER_STATE                        0xc
#define PHY_WIDTH_SHIFT                                            25
#define PHY_WIDTH_LEN                                              1
# define PHY_WIDTH_IF_20BIT                                        0
# define PHY_WIDTH_IF_40BIT                                        1
#define EDP_PHY_RATE_SHIFT                                         26
#define EDP_PHY_RATE_LEN                                           3

#define PHY_TX_EQ_OFFSET                                           0xa04
#define LANE0_TX_PREEMP_SHIFT(i)                                   (6 * (i))
#define LANE0_TX_PREEMP_LEN(i)                                     2
#define LANE0_TX_VSWING_SHIFT(i)                                   (6 * (i) + 2)
#define LANE0_TX_VSWING_LEN(i)                                     2
# define VOLTAGE_SWING_LEVEL0                                      0
# define VOLTAGE_SWING_LEVEL1                                      1
# define VOLTAGE_SWING_LEVEL2                                      2
# define VOLTAGE_SWING_LEVEL3                                      3
# define MAX_VOLTAGE_SWING_LEVEL                                   VOLTAGE_SWING_LEVEL3
# define PRE_EMPHASIS_LEVEL0                                       0
# define PRE_EMPHASIS_LEVEL1                                       1
# define PRE_EMPHASIS_LEVEL2                                       2
# define PRE_EMPHASIS_LEVEL3                                       3
# define MAX_PRE_EMPHASIS_LEVEL                                    PRE_EMPHASIS_LEVEL3
# define MAX_VSWING_PREEMP_LEVEL                                   3

/* i range [0,2] */
#define CUSTOMPAT_OFFSET0                                          0xa08
#define CUSTOM80B_0_SHIFT                                          0
#define CUSTOM80B_0_LEN                                            30
#define CUSTOMPAT_OFFSET1                                          0xa0c
#define CUSTOM80B_1_SHIFT                                          0
#define CUSTOM80B_1_LEN                                            30
#define CUSTOMPAT_OFFSET2                                          0xa10
#define CUSTOM80B_2_SHIFT                                          0
#define CUSTOM80B_2_LEN                                            20

#define HBR2_COMPLIANCE_SCRAMBLER_RESET_OFFSET                     0xa14
#define NUM_SR_ZEROS_SHIFT                                         0
#define NUM_SR_ZEROS_LEN                                           16

#define PHYIF_PWRDOWN_CTRL_OFFSET                                  0xa18
#define PHYIF_PWRDOWN_CTRL_SHIFT                                   0
#define PHYIF_PWRDOWN_CTRL_LEN                                     16
/* i range [0, 3] */
# define POWER_DOWN_CTRL_LANE_SHIFT(i)                             (0 + (i) * 4)
# define POWER_DOWN_CTRL_LANE_LEN(i)                               4
#define PER_LANE_PWRDOWN_CTL_EN_SHIFT                              16
#define PER_LANE_PWRDOWN_CTL_EN_LEN                                1

#define AUX_CMD_OFFSET                                             0xb00
#define AUX_LEN_REQ_SHIFT                                          0
#define AUX_LEN_REQ_LEN                                            4
#define I2C_ADDR_ONLY_SHIFT                                        4
#define I2C_ADDR_ONLY_LEN                                          1
#define AUX_PHY_WAKE_SHIFT                                         5
#define AUX_PHY_WAKE_LEN                                           1
#define AUX_ADDR_SHIFT                                             8
#define AUX_ADDR_LEN                                               20
#define AUX_CMD_TYPE_SHIFT                                         28
#define AUX_CMD_TYPE_LEN                                           4
/* aux request type */
#define AUX_REQ_NATIVE_WRITE                                       0x8
#define AUX_REQ_NATIVE_READ                                        0x9
#define AUX_REQ_I2C_MOT_WRITE                                      0x4
#define AUX_REQ_I2C_MOT_READ                                       0x5
#define AUX_REQ_I2C_WRITE                                          0x0
#define AUX_REQ_I2C_READ                                           0x1
#define AUX_REQ_I2C_WRITE_STATUS_UPDATE                            0x2
#define AUX_REQ_I2C_MOT_WRITE_STATUS_UPDATE                        0x6

#define AUX_STATUS_OFFSET                                          0xb04
#define AUX_STATUS_SHIFT                                           4
#define AUX_STATUS_LEN                                             4
/* aux reply type */
# define AUX_REPLY_ACK                                             0x0
# define AUX_REPLY_NACK                                            0x1
# define AUX_REPLY_DEFER                                           0x2
# define AUX_REPLY_I2C_NACK                                        0x4
# define AUX_REPLY_I2C_DEFER                                       0x8
#define AUX_M_SHIFT                                                8
#define AUX_M_LEN                                                  8
#define AUX_REPLY_RECEIVED_SHIFT                                   16
#define AUX_REPLY_RECEIVED_LEN                                     1
#define AUX_TIMEOUT_SHIFT                                          17
#define AUX_TIMEOUT_LEN                                            1
#define AUX_REPLY_ERR_SHIFT                                        18
#define AUX_REPLY_ERR_LEN                                          1
#define AUX_BYTES_READ_SHIFT                                       19
#define AUX_BYTES_READ_LEN                                         5
#define SINK_DISCONNECT_WHILE_ACTIVE_SHIFT                         24
#define SINK_DISCONNECT_WHILE_ACTIVE_LEN                           1
#define AUX_REPLY_ERR_CODE_SHIFT                                   25
#define AUX_REPLY_ERR_CODE_LEN                                     3
#define AUX_STATE_SHIFT                                            28
#define AUX_STATE_LEN                                              4
/* i range [0,3] */
#define AUX_DATA_OFFSET(i)                                         (0xb08 + (i) * 0x4)
#define AUX_DATA_COUNT                                             4

#define AUX_CONFIG_OFFSET1                                         0xb20
#define ALPM_FW_EXIT_LATENCY_STANDBY_CNT_LIMIT_SHIFT               0
#define ALPM_FW_EXIT_LATENCY_STANDBY_CNT_LIMIT_LEN                 16
#define ALPM_FW_EXIT_LATENCY_SLEEP_CNT_LIMIT_SHIFT                 16
#define ALPM_FW_EXIT_LATENCY_SLEEP_CNT_LIMIT_LEN                   16

#define AUX_CONFIG_OFFSET2                                         0xb24
#define ALPM_AUX_PHY_WAKE_ACK_TIMEOUT_CNT_LIMIT_SHIFT              0
#define ALPM_AUX_PHY_WAKE_ACK_TIMEOUT_CNT_LIMIT_LEN                8

#define AUX_250US_CNT_LIMIT_OFFSET                                 0xb40
#define AUX_250US_CNT_LIMIT_SHIFT                                  0
#define AUX_250US_CNT_LIMIT_LEN                                    17

#define AUX_2000US_CNT_LIMT_OFFSET                                 0xb44
#define AUX_2000US_CNT_LIMIT_SHIFT                                 0
#define AUX_2000US_CNT_LIMIT_LEN                                   17

#define AUX_100000US_CNT_LIMIT_OFFSET                              0xb48
#define AUX_100000US_CNT_LIMIT_SHIFT                               0
#define AUX_100000US_CNT_LIMIT_LEN                                 17

#define COMBO_PHY_CTRL_OFFSET1                                     0xc0c
#define TX0_IN_GENERIC_BUS_SHIFT                                   0
#define TX0_IN_GENERIC_BUS_LEN                                     5
#define TX0_HP_PROT_EN_SHIFT                                       5
#define TX0_HP_PROT_EN_LEN                                         1
#define TX0_BYPASS_EQ_CALC_SHIFT                                   6
#define TX0_BYPASS_EQ_CALC_LEN                                     1
#define TX1_IN_GENERIC_BUS_SHIFT                                   8
#define TX1_IN_GENERIC_BUS_LEN                                     5
#define TX1_HP_PROT_EN_SHIFT                                       13
#define TX1_HP_PROT_EN_LEN                                         1
#define TX1_BYPASS_EQ_CALC_SHIFT                                   14
#define TX1_BYPASS_EQ_CALC_LEN                                     1
#define TX2_IN_GENERIC_BUS_SHIFT                                   16
#define TX2_IN_GENERIC_BUS_LEN                                     5
#define TX2_HP_PROT_EN_SHIFT                                       21
#define TX2_HP_PROT_EN_LEN                                         1
#define TX2_BYPASS_EQ_CALC_SHIFT                                   22
#define TX2_BYPASS_EQ_CALC_LEN                                     1
#define TX3_IN_GENERIC_BUS_SHIFT                                   24
#define TX3_IN_GENERIC_BUS_LEN                                     5
#define TX3_HP_PROT_EN_SHIFT                                       29
#define TX3_HP_PROT_EN_LEN                                         1
#define TX3_BYPASS_EQ_CALC_SHIFT                                   30
#define TX3_BYPASS_EQ_CALC_LEN                                     1

#define COMBO_PHY_STATUS_OFFSET1                                   0xc10
#define TX0_OUT_GENERIC_BUS_SHIFT                                  0
#define TX0_OUT_GENERIC_BUS_LEN                                    5
#define TX1_OUT_GENERIC_BUS_SHIFT                                  8
#define TX1_OUT_GENERIC_BUS_LEN                                    5
#define TX2_OUT_GENERIC_BUS_SHIFT                                  16
#define TX2_OUT_GENERIC_BUS_LEN                                    5
#define TX3_OUT_GENERIC_BUS_SHIFT                                  24
#define TX3_OUT_GENERIC_BUS_LEN                                    5

#define COMBO_PHY_OVR_OFFSET                                       0xc14
#define COMBO_PHY_OVR_SHIFT                                        0
#define COMBO_PHY_OVR_LEN                                          1

/* i range [0,2] */
#define COMBO_PHY_OVR_MPLL_CTRL_OFFSET0                            0xc18
#define COMBO_PHY_OVR_MPLL_MULTIPLIER_SHIFT                        0
#define COMBO_PHY_OVR_MPLL_MULTIPLIER_LEN                          12
#define COMBO_PHY_OVR_MPLL_DIV_MULTIPLIER_SHIFT                    12
#define COMBO_PHY_OVR_MPLL_DIV_MULTIPLIER_LEN                      8
#define COMBO_PHY_OVR_MPLL_TX_CLK_DIV_SHIFT                        20
#define COMBO_PHY_OVR_MPLL_TX_CLK_DIV_LEN                          3

#define COMBO_PHY_OVR_MPLL_CTRL_OFFSET1                            0xc1c
#define COMBO_PHY_OVR_MPLL_SSC_FREQ_CNT_INIT_SHIFT                 0
#define COMBO_PHY_OVR_MPLL_SSC_FREQ_CNT_INIT_LEN                   12
#define COMBO_PHY_OVR_MPLL_SSC_FREQ_CNT_PEAK_SHIFT                 12
#define COMBO_PHY_OVR_MPLL_SSC_FREQ_CNT_PEAK_LEN                   8
#define COMBO_PHY_OVR_MPLL_SSC_FREQ_CNT_OVRD_EN_SHIFT              20
#define COMBO_PHY_OVR_MPLL_SSC_FREQ_CNT_OVRD_EN_LEN                1

#define COMBO_PHY_OVR_MPLL_CTRL_OFFSET2                            0xc20
#define COMBO_PHY_OVR_MPLL_DIV_CLK_EN_SHIFT                        0
#define COMBO_PHY_OVR_MPLL_DIV_CLK_EN_LEN                          1
#define COMBO_PHY_OVR_MPLL_WORD_DIV2_EN_SHIFT                      4
#define COMBO_PHY_OVR_MPLL_WORD_DIV2_EN_LEN                        1
#define COMBO_PHY_OVR_MPLL_INIT_CAL_DISABLE_SHIFT                  8
#define COMBO_PHY_OVR_MPLL_INIT_CAL_DISABLE_LEN                    1

/* i range [0,5] */
#define COMBO_PHY_GEN2_OVR_MPLL_CTRL_OFFSET0                       0xc24
#define COMBO_PHY_OVR_MPLL_PMIX_EN_SHIFT                           0
#define COMBO_PHY_OVR_MPLL_PMIX_EN_LEN                             1
#define COMBO_PHY_OVR_MPLL_V2I_SHIFT                               4
#define COMBO_PHY_OVR_MPLL_V2I_LEN                                 2
#define COMBO_PHY_OVR_MPLL_CP_INT_SHIFT                            8
#define COMBO_PHY_OVR_MPLL_CP_INT_LEN                              7
#define COMBO_PHY_OVR_MPLL_CP_PROP_SHIFT                           16
#define COMBO_PHY_OVR_MPLL_CP_PROP_LEN                             7
#define COMBO_PHY_OVR_MPLL_SSC_UP_SPREAD_SHIFT                     24
#define COMBO_PHY_OVR_MPLL_SSC_UP_SPREAD_LEN                       1

#define COMBO_PHY_GEN2_OVR_MPLL_CTRL_OFFSET1                       0xc28
#define COMBO_PHY_OVR_MPLL_SSC_PEAK_SHIFT                          0
#define COMBO_PHY_OVR_MPLL_SSC_PEAK_LEN                            20

#define COMBO_PHY_GEN2_OVR_MPLL_CTRL_OFFSET2                       0xc2c
#define COMBO_PHY_OVR_MPLL_SSC_STEPSIZE_SHIFT                      0
#define COMBO_PHY_OVR_MPLL_SSC_STEPSIZE_LEN                        21

#define COMBO_PHY_GEN2_OVR_MPLL_CTRL_OFFSET3                       0xc30
#define COMBO_PHY_OVR_MPLL_FRACN_CFG_UPDATE_EN_SHIFT               0
#define COMBO_PHY_OVR_MPLL_FRACN_CFG_UPDATE_EN_LEN                 1
#define COMBO_PHY_OVR_MPLL_FRACN_EN_SHIFT                          5
#define COMBO_PHY_OVR_MPLL_FRACN_EN_LEN                            3
#define COMBO_PHY_OVR_MPLL_FRACN_DEN_SHIFT                         24
#define COMBO_PHY_OVR_MPLL_FRACN_DEN_LEN                           8

#define COMBO_PHY_GEN2_OVR_MPLL_CTRL_OFFSET4                       0xc34
#define COMBO_PHY_OVR_MPLL_FRACN_QUO_SHIFT                         0
#define COMBO_PHY_OVR_MPLL_FRACN_QUO_LEN                           16
#define COMBO_PHY_OVR_MPLL_FRACN_REM_SHIFT                         16
#define COMBO_PHY_OVR_MPLL_FRACN_REM_LEN                           16

#define COMBO_PHY_GEN2_OVR_MPLL_CTRL_OFFSET5                       0xc38
#define COMBO_PHY_OVR_MPLL_FREQ_VCO_SHIFT                          0
#define COMBO_PHY_OVR_MPLL_FREQ_VCO_LEN                            2
#define COMBO_PHY_OVR_REF_CLK_MPLL_DIV_SHIFT                       4
#define COMBO_PHY_OVR_REF_CLK_MPLL_DIV_LEN                         3
#define COMBO_PHY_OVR_MPLL_DIV5_CLK_EN_SHIFT                       8
#define COMBO_PHY_OVR_MPLL_DIV5_CLK_EN_LEN                         1

#define COMBO_PHY_GEN1_OVR_MPLL_CTRL_OFFSET0                       0xc3c
#define COMBO_PHY_OVR_MPLL_SSC_CLK_SE_SHIFT                        0
#define COMBO_PHY_OVR_MPLL_SSC_CLK_SE_LEN                          3
#define COMBO_PHY_OVR_MPLL_SSC_RANGE_SHIFT                         4
#define COMBO_PHY_OVR_MPLL_SSC_RANGE_LEN                           3
#define COMBO_PHY_OVR_MPLL_FRACN_CTRL_SHIFT                        19
#define COMBO_PHY_OVR_MPLL_FRACN_CTRL_LEN                          13

#define COMBO_PHY_GEN1_OVR_MPLL_CTRL_OFFSET1                       0xc40
#define COMBO_PHY_OVR_MPLL_BANDWIDTH_SHIFT                         0
#define COMBO_PHY_OVR_MPLL_BANDWIDTH_LEN                           16
#define COMBO_PHY_OVR_MPLL_DIV10_CLK_EN_SHIFT                      16
#define COMBO_PHY_OVR_MPLL_DIV10_CLK_EN_LEN                        1
#define COMBO_PHY_OVR_MPLL_DIV8_CLK_EN_SHIFT                       20
#define COMBO_PHY_OVR_MPLL_DIV8_CLK_EN_LEN                         1
#define COMBO_PHY_OVR_REF_CLK_MPLL_DIV2_EN_SHIFT                   24
#define COMBO_PHY_OVR_REF_CLK_MPLL_DIV2_EN_LEN                     1

#define COMBO_PHY_OVR_TERM_CTRL_OFFSET                             0xc44
#define COMBO_PHY_OVR_TX0_TERM_CTRL_SHIFT                          0
#define COMBO_PHY_OVR_TX0_TERM_CTRL_LEN                            3
#define COMBO_PHY_OVR_TX1_TERM_CTRL_SHIFT                          4
#define COMBO_PHY_OVR_TX1_TERM_CTRL_LEN                            3
#define COMBO_PHY_OVR_TX2_TERM_CTRL_SHIFT                          8
#define COMBO_PHY_OVR_TX2_TERM_CTRL_LEN                            3
#define COMBO_PHY_OVR_TX3_TERM_CTRL_SHIFT                          12
#define COMBO_PHY_OVR_TX3_TERM_CTRL_LEN                            3

/* i range [1,8] */
#define COMBO_PHY_OVR_TX_EQ_G1_CTRL_OFFSET                         0xc48
#define COMBO_PHY_OVR_TX_EQ_G1_SHIFT                               0
#define COMBO_PHY_OVR_TX_EQ_G1_LEN                                 1
#define COMBO_PHY_OVR_TX_EQ_MAIN_G1_SHIFT                          8
#define COMBO_PHY_OVR_TX_EQ_MAIN_G1_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_POST_G1_SHIFT                          16
#define COMBO_PHY_OVR_TX_EQ_POST_G1_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_PRE_G1_SHIFT                           24
#define COMBO_PHY_OVR_TX_EQ_PRE_G1_LEN                             6
#define COMBO_PHY_OVR_TX_EQ_G2_CTRL_OFFSET                         0xc4c
#define COMBO_PHY_OVR_TX_EQ_G2_SHIFT                               0
#define COMBO_PHY_OVR_TX_EQ_G2_LEN                                 1
#define COMBO_PHY_OVR_TX_EQ_MAIN_G2_SHIFT                          8
#define COMBO_PHY_OVR_TX_EQ_MAIN_G2_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_POST_G2_SHIFT                          16
#define COMBO_PHY_OVR_TX_EQ_POST_G2_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_PRE_G2_SHIFT                           24
#define COMBO_PHY_OVR_TX_EQ_PRE_G2_LEN                             6
#define COMBO_PHY_OVR_TX_EQ_G3_CTRL_OFFSET                         0xc50
#define COMBO_PHY_OVR_TX_EQ_G3_SHIFT                               0
#define COMBO_PHY_OVR_TX_EQ_G3_LEN                                 1
#define COMBO_PHY_OVR_TX_EQ_MAIN_G3_SHIFT                          8
#define COMBO_PHY_OVR_TX_EQ_MAIN_G3_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_POST_G3_SHIFT                          16
#define COMBO_PHY_OVR_TX_EQ_POST_G3_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_PRE_G3_SHIFT                           24
#define COMBO_PHY_OVR_TX_EQ_PRE_G3_LEN                             6
#define COMBO_PHY_OVR_TX_EQ_G4_CTRL_OFFSET                         0xc54
#define COMBO_PHY_OVR_TX_EQ_G4_SHIFT                               0
#define COMBO_PHY_OVR_TX_EQ_G4_LEN                                 1
#define COMBO_PHY_OVR_TX_EQ_MAIN_G4_SHIFT                          8
#define COMBO_PHY_OVR_TX_EQ_MAIN_G4_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_POST_G4_SHIFT                          16
#define COMBO_PHY_OVR_TX_EQ_POST_G4_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_PRE_G4_SHIFT                           24
#define COMBO_PHY_OVR_TX_EQ_PRE_G4_LEN                             6
#define COMBO_PHY_OVR_TX_EQ_G5_CTRL_OFFSET                         0xc58
#define COMBO_PHY_OVR_TX_EQ_G5_SHIFT                               0
#define COMBO_PHY_OVR_TX_EQ_G5_LEN                                 1
#define COMBO_PHY_OVR_TX_EQ_MAIN_G5_SHIFT                          8
#define COMBO_PHY_OVR_TX_EQ_MAIN_G5_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_POST_G5_SHIFT                          16
#define COMBO_PHY_OVR_TX_EQ_POST_G5_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_PRE_G5_SHIFT                           24
#define COMBO_PHY_OVR_TX_EQ_PRE_G5_LEN                             6
#define COMBO_PHY_OVR_TX_EQ_G6_CTRL_OFFSET                         0xc5c
#define COMBO_PHY_OVR_TX_EQ_G6_SHIFT                               0
#define COMBO_PHY_OVR_TX_EQ_G6_LEN                                 1
#define COMBO_PHY_OVR_TX_EQ_MAIN_G6_SHIFT                          8
#define COMBO_PHY_OVR_TX_EQ_MAIN_G6_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_POST_G6_SHIFT                          16
#define COMBO_PHY_OVR_TX_EQ_POST_G6_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_PRE_G6_SHIFT                           24
#define COMBO_PHY_OVR_TX_EQ_PRE_G6_LEN                             6
#define COMBO_PHY_OVR_TX_EQ_G7_CTRL_OFFSET                         0xc60
#define COMBO_PHY_OVR_TX_EQ_G7_SHIFT                               0
#define COMBO_PHY_OVR_TX_EQ_G7_LEN                                 1
#define COMBO_PHY_OVR_TX_EQ_MAIN_G7_SHIFT                          8
#define COMBO_PHY_OVR_TX_EQ_MAIN_G7_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_POST_G7_SHIFT                          16
#define COMBO_PHY_OVR_TX_EQ_POST_G7_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_PRE_G7_SHIFT                           24
#define COMBO_PHY_OVR_TX_EQ_PRE_G7_LEN                             6
#define COMBO_PHY_OVR_TX_EQ_G8_CTRL_OFFSET                         0xc64
#define COMBO_PHY_OVR_TX_EQ_G8_SHIFT                               0
#define COMBO_PHY_OVR_TX_EQ_G8_LEN                                 1
#define COMBO_PHY_OVR_TX_EQ_MAIN_G8_SHIFT                          8
#define COMBO_PHY_OVR_TX_EQ_MAIN_G8_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_POST_G8_SHIFT                          16
#define COMBO_PHY_OVR_TX_EQ_POST_G8_LEN                            6
#define COMBO_PHY_OVR_TX_EQ_PRE_G8_SHIFT                           24
#define COMBO_PHY_OVR_TX_EQ_PRE_G8_LEN                             6

/* i range [0,3] */
#define COMBO_PHY_OVR_TX_LANE0_CTRL_OFFSET                         0xc68
#define COMBO_PHY_OVR_TX0_EN_SHIFT                                 0
#define COMBO_PHY_OVR_TX0_EN_LEN                                   1
#define COMBO_PHY_OVR_TX0_VBOOST_EN_SHIFT                          4
#define COMBO_PHY_OVR_TX0_VBOOST_EN_LEN	                           1
#define COMBO_PHY_OVR_TX0_IBOOST_LVL_SHIFT                         8
#define COMBO_PHY_OVR_TX0_IBOOST_LVL_LEN                           4
#define COMBO_PHY_OVR_TX0_CLK_RDY_SHIFT                            12
#define COMBO_PHY_OVR_TX0_CLK_RDY_LEN                              1
#define COMBO_PHY_OVR_TX0_INVERT_SHIFT                             16
#define COMBO_PHY_OVR_TX0_INVERT_LEN                               1
#define COMBO_PHY_OVR_TX_LANE1_CTRL_OFFSET                         0xc6c
#define COMBO_PHY_OVR_TX1_EN_SHIFT                                 0
#define COMBO_PHY_OVR_TX1_EN_LEN                                   1
#define COMBO_PHY_OVR_TX1_VBOOST_EN_SHIFT                          4
#define COMBO_PHY_OVR_TX1_VBOOST_EN_LEN                            1
#define COMBO_PHY_OVR_TX1_IBOOST_LVL_SHIFT                         8
#define COMBO_PHY_OVR_TX1_IBOOST_LVL_LEN                           4
#define COMBO_PHY_OVR_TX1_CLK_RDY_SHIFT                            12
#define COMBO_PHY_OVR_TX1_CLK_RDY_LEN                              1
#define COMBO_PHY_OVR_TX1_INVERT_SHIFT                             16
#define COMBO_PHY_OVR_TX1_INVERT_LEN                               1
#define COMBO_PHY_OVR_TX_LANE2_CTRL_OFFSET                         0xc70
#define COMBO_PHY_OVR_TX2_EN_SHIFT                                 0
#define COMBO_PHY_OVR_TX2_EN_LEN                                   1
#define COMBO_PHY_OVR_TX2_VBOOST_EN_SHIFT                          4
#define COMBO_PHY_OVR_TX2_VBOOST_EN_LEN                            1
#define COMBO_PHY_OVR_TX2_IBOOST_LVL_SHIFT                         8
#define COMBO_PHY_OVR_TX2_IBOOST_LVL_LEN                           4
#define COMBO_PHY_OVR_TX2_CLK_RDY_SHIFT                            12
#define COMBO_PHY_OVR_TX2_CLK_RDY_LEN                              1
#define COMBO_PHY_OVR_TX2_INVERT_SHIFT                             16
#define COMBO_PHY_OVR_TX2_INVERT_LEN                               1
#define COMBO_PHY_OVR_TX_LANE3_CTRL_OFFSET                         0xc74
#define COMBO_PHY_OVR_TX3_EN_SHIFT                                 0
#define COMBO_PHY_OVR_TX3_EN_LEN                                   1
#define COMBO_PHY_OVR_TX3_VBOOST_EN_SHIFT                          4
#define COMBO_PHY_OVR_TX3_VBOOST_EN_LEN                            1
#define COMBO_PHY_OVR_TX3_IBOOST_LVL_SHIFT                         8
#define COMBO_PHY_OVR_TX3_IBOOST_LVL_LEN                           4
#define COMBO_PHY_OVR_TX3_CLK_RDY_SHIFT                            12
#define COMBO_PHY_OVR_TX3_CLK_RDY_LEN                              1
#define COMBO_PHY_OVR_TX3_INVERT_SHIFT                             16
#define COMBO_PHY_OVR_TX3_INVERT_LEN                               1

#define GENERAL_INTERRUPT_OFFSET                                   0xd00
#define HPD_EVENT_SHIFT                                            0
#define HPD_EVENT_LEN                                              1
#define AUX_REPLY_EVENT_SHIFT                                      1
#define AUX_REPLY_EVENT_LEN                                        1
#define HDCP_EVENT_SHIFT                                           2
#define HDCP_EVENT_LEN                                             1
#define AUX_CMD_INVALID_SHIFT                                      3
#define AUX_CMD_INVALID_LEN                                        1
#define SDP_EVENT_STREAM0_SHIFT                                    4
#define SDP_EVENT_STREAM0_LEN                                      1
#define AUDIO_FIFO_OVERFLOW_STREAM0_SHIFT                          5
#define AUDIO_FIFO_OVERFLOW_STREAM0_LEN                            1
#define VIDEO_FIFO_OVERFLOW_STREAM0_SHIFT                          6
#define VIDEO_FIFO_OVERFLOW_STREAM0_LEN                            1
#define VIDEO_FIFO_UNDERFLOW_STREAM0_SHIFT                         8
#define VIDEO_FIFO_UNDERFLOW_STREAM0_LEN                           1
#define SDP_EVENT_STREAM1_SHIFT                                    12
#define SDP_EVENT_STREAM1_LEN                                      1
#define AUDIO_FIFO_OVERFLOW_STREAM1_SHIFT                          13
#define AUDIO_FIFO_OVERFLOW_STREAM1_LEN                            1
#define VIDEO_FIFO_OVERFLOW_STREAM1_SHIFT                          14
#define VIDEO_FIFO_OVERFLOW_STREAM1_LEN                            1
#define VIDEO_FIFO_UNDERFLOW_STREAM1_SHIFT                         15
#define VIDEO_FIFO_UNDERFLOW_STREAM1_LEN                           1
#define SDP_EVENT_STREAM2_SHIFT                                    18
#define SDP_EVENT_STREAM2_LEN                                      1
#define AUDIO_FIFO_OVERFLOW_STREAM2_SHIFT                          19
#define AUDIO_FIFO_OVERFLOW_STREAM2_LEN                            1
#define VIDEO_FIFO_OVERFLOW_STREAM2_SHIFT                          20
#define VIDEO_FIFO_OVERFLOW_STREAM2_LEN                            1
#define VIDEO_FIFO_UNDERFLOW_STREAM2_SHIFT                         21
#define VIDEO_FIFO_UNDERFLOW_STREAM2_LEN                           1
#define SDP_EVENT_STREAM3_SHIFT                                    24
#define SDP_EVENT_STREAM3_LEN                                      1
#define AUDIO_FIFO_OVERFLOW_STREAM3_SHIFT                          25
#define AUDIO_FIFO_OVERFLOW_STREAM3_LEN                            1
#define VIDEO_FIFO_OVERFLOW_STREAM3_SHIFT                          26
#define VIDEO_FIFO_OVERFLOW_STREAM3_LEN                            1
#define VIDEO_FIFO_UNDERFLOW_STREAM3_SHIFT                         27
#define VIDEO_FIFO_UNDERFLOW_STREAM3_LEN                           1
#define DSC_EVENT_SHIFT                                            30
#define DSC_EVENT_LEN                                              1
#define PM_EVENT_SHIFT                                             31
#define PM_EVENT_LEN                                               1

#define GENERAL_INTERRUPT_ENABLE_OFFSET                            0xd04
#define HPD_EVENT_EN_SHIFT                                         0
#define HPD_EVENT_EN_LEN                                           1
#define AUX_REPLY_EVENT_EN_SHIFT                                   1
#define AUX_REPLY_EVENT_EN_LEN                                     1
#define HDCP_EVENT_EN_SHIFT                                        2
#define HDCP_EVENT_EN_LEN                                          1
#define AUX_CMD_INVALID_EN_SHIFT                                   3
#define AUX_CMD_INVALID_EN_LEN                                     1
#define SDP_EVENT_EN_STREAM0_SHIFT                                 4
#define SDP_EVENT_EN_STREAM0_LEN                                   1
#define AUDIO_FIFO_OVERFLOW_EN_STREAM0_SHIFT                       5
#define AUDIO_FIFO_OVERFLOW_EN_STREAM0_LEN                         1
#define VIDEO_FIFO_OVERFLOW_EN_STREAM0_SHIFT                       6
#define VIDEO_FIFO_OVERFLOW_EN_STREAM0_LEN                         1
#define VIDEO_FIFO_UNDERFLOW_EN_STREAM0_SHIFT                      8
#define VIDEO_FIFO_UNDERFLOW_EN_STREAM0_LEN                        1
#define SDP_EVENT_EN_STREAM1_SHIFT                                 12
#define SDP_EVENT_EN_STREAM1_LEN                                   1
#define AUDIO_FIFO_OVERFLOW_EN_STREAM1_SHIFT                       13
#define AUDIO_FIFO_OVERFLOW_EN_STREAM1_LEN                         1
#define VIDEO_FIFO_OVERFLOW_EN_STREAM1_SHIFT                       14
#define VIDEO_FIFO_OVERFLOW_EN_STREAM1_LEN                         1
#define VIDEO_FIFO_UNDERFLOW_EN_STREAM1_SHIFT                      15
#define VIDEO_FIFO_UNDERFLOW_EN_STREAM1_LEN                        1
#define SDP_EVENT_EN_STREAM2_SHIFT                                 18
#define SDP_EVENT_EN_STREAM2_LEN                                   1
#define AUDIO_FIFO_OVERFLOW_EN_STREAM2_SHIFT                       19
#define AUDIO_FIFO_OVERFLOW_EN_STREAM2_LEN                         1
#define VIDEO_FIFO_OVERFLOW_EN_STREAM2_SHIFT                       20
#define VIDEO_FIFO_OVERFLOW_EN_STREAM2_LEN                         1
#define VIDEO_FIFO_UNDERFLOW_EN_STREAM2_SHIFT                      21
#define VIDEO_FIFO_UNDERFLOW_EN_STREAM2_LEN                        1
#define SDP_EVENT_EN_STREAM3_SHIFT                                 24
#define SDP_EVENT_EN_STREAM3_LEN                                   1
#define AUDIO_FIFO_OVERFLOW_EN_STREAM3_SHIFT                       25
#define AUDIO_FIFO_OVERFLOW_EN_STREAM3_LEN                         1
#define VIDEO_FIFO_OVERFLOW_EN_STREAM3_SHIFT                       26
#define VIDEO_FIFO_OVERFLOW_EN_STREAM3_LEN                         1
#define VIDEO_FIFO_UNDERFLOW_EN_STREAM3_SHIFT                      27
#define VIDEO_FIFO_UNDERFLOW_EN_STREAM3_LEN                        1
#define DSC_EVENT_EN_SHIFT                                         30
#define DSC_EVENT_EN_LEN                                           1
#define PM_EVENT_EN_SHIFT                                          31
#define PM_EVENT_EN_LEN                                            1

#define HPD_STATUS_OFFSET                                          0xd08
#define HPD_IRQ_SHIFT                                              0
#define HPD_IRQ_LEN                                                1
#define HPD_HOT_PLUG_SHIFT                                         1
#define HPD_HOT_PLUG_LEN                                           1
#define HPD_HOT_UNPLUG_SHIFT                                       2
#define HPD_HOT_UNPLUG_LEN                                         1
#define HPD_UNPLUG_ERR_SHIFT                                       3
#define HPD_UNPLUG_ERR_LEN                                         1
#define HPD_STATUS_SHIFT                                           8
#define HPD_STATUS_LEN                                             1
#define HPD_STATE_SHIFT                                            9
#define HPD_STATE_LEN                                              3
#define HPD_TIMER_SHIFT                                            15
#define HPD_TIMER_LEN                                              17

#define HPD_INTERRUPT_ENABLE_OFFSET                                0xd0c
#define HPD_IRQ_EN_SHIFT                                           0
#define HPD_IRQ_EN_LEN                                             1
#define HPD_PLUG_EN_SHIFT                                          1
#define HPD_PLUG_EN_LEN                                            1
#define HPD_UNPLUG_EN_SHIFT                                        2
#define HPD_UNPLUG_EN_LEN                                          1
#define HPD_UNPLUG_ERR_EN_SHIFT                                    3
#define HPD_UNPLUG_ERR_EN_LEN                                      1

#define HDCPCFG_OFFSET                                             0xe00
#define ENABLE_HDCP_SHIFT                                          1
#define ENABLE_HDCP_LEN                                            1
#define ENABLE_HDCP_13_SHIFT                                       2
#define ENABLE_HDCP_13_LEN                                         1
#define ENCRYPTIONDISABLE_SHIFT                                    3
#define ENCRYPTIONDISABLE_LEN                                      1
#define HDCP_LOCK_SHIFT                                            4
#define HDCP_LOCK_LEN                                              1
#define BYPENCRYPTION_SHIFT                                        5
#define BYPENCRYPTION_LEN                                          1
#define CP_IRQ_SHIFT                                               6
#define CP_IRQ_LEN                                                 1
#define DPCD12PLUS_SHIFT                                           7
#define DPCD12PLUS_LEN                                             1

#define HDCPOBS_OFFSET                                             0xe04
#define HDCPENGAGED_SHIFT                                          0
#define HDCPENGAGED_LEN                                            1
#define SUBSTATEA_SHIFT                                            1
#define SUBSTATEA_LEN                                              3
#define STATEA_SHIFT                                               4
#define STATEA_LEN                                                 4
#define STATER_SHIFT                                               8
#define STATER_LEN                                                 3
#define STATEOEG_SHIFT                                             11
#define STATEOEG_LEN                                               3
#define STATEE_SHIFT                                               14
#define STATEE_LEN                                                 3
#define HDCP_CAPABLE_SHIFT                                         17
#define HDCP_CAPABLE_LEN                                           1
#define REPEATER_SHIFT                                             18
#define REPEATER_LEN                                               1
#define HDCP13_BSTATUS_SHIFT                                       19
#define HDCP13_BSTATUS_LEN                                         4
#define HDCP2_BOOTED_SHIFT                                         23
#define HDCP2_BOOTED_LEN                                           1
#define HDCP2_STATE_SHIFT                                          24
#define HDCP2_STATE_LEN                                            3
#define HDCP2_SINK_CAP_CHECK_COMPLETE_SHIFT                        27
#define HDCP2_SINK_CAP_CHECK_COMPLETE_LEN                          1
#define HDCP2_CAPABLE_SINK_SHIFT                                   28
#define HDCP2_CAPABLE_SINK_LEN                                     1
#define HDCP2_AUTHENTICATION_SUCCESS_SHIFT                         29
#define HDCP2_AUTHENTICATION_SUCCESS_LEN                           1
#define HDCP2_AUTHENTICATION_FAILED_SHIFT                          30
#define HDCP2_AUTHENTICATION_FAILED_LEN                            1
#define HDCP2_RE_AUTHENTICATION_REQ_SHIFT                          31
#define HDCP2_RE_AUTHENTICATION_REQ_LEN                            1

#define HDCPAPIINTCLR_OFFSET                                       0xe08
#define HDCPAPIINTSTAT_OFFSET                                      0xe0c
#define HDCPAPIINTMSK_OFFSET                                       0xe10
#define KSVACCESSINT_SHIFT                                         0
#define KSVACCESSINT_LEN                                           1
#define AUXRESPDEFER7TIMES_SHIFT                                   2
#define AUXRESPDEFER7TIMES_LEN                                     1
#define AUXRESPTIMEOUT_SHIFT                                       3
#define AUXRESPTIMEOUT_LEN                                         1
#define AUXRESPNACK7TIMES_SHIFT                                    4
#define AUXRESPNACK7TIMES_LEN                                      1
#define KSVSHA1CALCDONEINT_SHIFT                                   5
#define KSVSHA1CALCDONEINT_LEN                                     1
#define HDCP_FAILED_SHIFT                                          6
#define HDCP_FAILED_LEN                                            1
#define HDCP_ENGAGED_SHIFT                                         7
#define HDCP_ENGAGED_LEN                                           1
#define HDCP2_GPIOINT_SHIFT                                        8
#define HDCP2_GPIOINT_LEN                                          1

#define HDCPKSVMEMCTRL_OFFSET                                      0xe18
#define KSVMEMREQUEST_SHIFT                                        0
#define KSVMEMREQUEST_LEN                                          1
#define KSVMEMACCESS_SHIFT                                         1
#define KSVMEMACCESS_LEN                                           1
#define KSVLISTPROCESSUPD_SHIFT                                    2
#define KSVLISTPROCESSUPD_LEN                                      1
#define KSVSHA1SWSTATUS_SHIFT                                      3
#define KSVSHA1SWSTATUS_LEN                                        1
#define KSVSHA1STATUS_SHIFT                                        4
#define KSVSHA1STATUS_LEN                                          1

/* i range [0,1431] */
#define HDCP_REVCC_RAM_OFFSET(i)                                   (0xe20 + (i) * 0x4)
#define REVOC_RAM_BYTE0_SHIFT                                      0
#define REVOC_RAM_BYTE0_LEN                                        8
#define REVOC_RAM_BYTE1_SHIFT                                      8
#define REVOC_RAM_BYTE1_LEN                                        8
#define REVOC_RAM_BYTE2_SHIFT                                      16
#define REVOC_RAM_BYTE2_LEN                                        8
#define REVOC_RAM_BYTE3_SHIFT                                      24
#define REVOC_RAM_BYTE3_LEN                                        8

#define HDCPREG_BKSV_OFFSET0                                       0x3600

#define HDCPREG_BKSV_OFFSET1                                       0x3604
#define HDCPREG_BKSV1_SHIFT                                        0
#define HDCPREG_BKSV1_LEN                                          8

#define HDCPREG_ANCONF_OFFSET                                      0x3608
#define OANBYPASS_SHIFT                                            0
#define OANBYPASS_LEN                                              1

#define HDCPREG_AN_OFFSET0                                         0x360c

#define HDCPREG_AN_OFFSET1                                         0x3610

#define HDCPREG_RMLCTL_OFFSET                                      0x3614
#define ODPK_DECRYPT_ENABLE_SHIFT                                  0
#define ODPK_DECRYPT_ENABLE_LEN                                    1

#define HDCPREG_RMLSTS_OFFSET                                      0x3618
#define IDPK_DATA_INDEX_SHIFT                                      0
#define IDPK_DATA_INDEX_LEN                                        6
#define IDPK_WR_OK_STS_SHIFT                                       6
#define IDPK_WR_OK_STS_LEN                                         1

#define HDCPREG_SEED_OFFSET                                        0x361c
#define HDCPREG_SEED_SHIFT                                         0
#define HDCPREG_SEED_LEN                                           16

#define HDCPREG_DPK_OFFSET0                                        0x3620
#define HDCPREG_DPK_OFFSET1                                        0x3624

#define HDCP2GPIOSTS_OFFSET                                        0x3628
#define HDCP2GPIOOUTSTS_SHIFT                                      0
#define HDCP2GPIOOUTSTS_LEN                                        20

#define HDCP2GPIOCHNGSTS_OFFSET                                    0x362c
#define HDCP2GPIOOUTCHNGSTS_SHIFT                                  0
#define HDCP2GPIOOUTCHNGSTS_LEN                                    20

#define HDCPREG_DPK_CRC_OFFSET                                     0x3630
#define VG_SWRST_OFFSET                                            0x3800
#define VG_SWRS_SHIFT                                              0
#define VG_SWRS_LEN                                                1

/* i range [1,6] */
#define VG_CONFIG_OFFSET1                                          0x3804
#define ODEPOLARITY_SHIFT                                          0
#define ODEPOLARITY_LEN                                            1
#define OHSYNCPOLARITY_SHIFT                                       1
#define OHSYNCPOLARITY_LEN                                         1
#define OVSYNCPOLARITY_SHIFT                                       2
#define OVSYNCPOLARITY_LEN                                         1
#define OIP_SHIFT                                                  3
#define OIP_LEN                                                    1
#define OCOLORINCREMENT_SHIFT                                      4
#define OCOLORINCREMENT_LEN                                        1
#define OVBLANKOSCILLATION_SHIFT                                   5
#define OVBLANKOSCILLATION_LEN                                     1
#define yec_422_MAPPING_SHIFT                                      6
#define yec_422_MAPPING_LEN                                        1
#define YCC_PATTERN_GENERATION_SHIFT                               7
#define YCC_PATTERN_GENERATION_LEN                                 1
#define PIXEL_REPETITION_SHIFT                                     8
#define PIXEL_REPETITION_LEN                                       4
#define BITS_PER_COMP_SHIFT                                        12
#define BITS_PER_COMP_LEN                                          3
#define YCC_420_MAPPING_SHIFT                                      15
#define YCC_420_MAPPING_LEN                                        1
#define INTERNAL_EXTERNAL_GEN_SHIFT                                16
#define INTERNAL_EXTERNAL_GEN_LEN                                  1
#define PATTERN_MODE_SHIFT                                         17
#define PATTERN_MODE_LEN                                           2

#define VG_CONFIG_OFFSET2                                          0x3808
#define VG_CONFIG2_HACTIVE_SHIFT                                   0
#define VG_CONFIG2_HACTIVE_LEN                                     16
#define VG_CONFIG2_HBLANK_SHIFT                                    16
#define VG_CONFIG2_HBLANK_LEN                                      16

#define VG_CONFIG_OFFSET3                                          0x380c
#define VG_CONFIG_H_FRONT_PORCH_SHIFT                              0
#define VG_CONFIG_H_FRONT_PORCH_LEN                                16
#define VG_CONFIG_H_SYNC_WIDTH_SHIFT                               16
#define VG_CONFIG_H_SYNC_WIDTH_LEN                                 16

#define VG_CONFIG_OFFSET4                                          0x3810
#define VACTIVE_SHIFT                                              0
#define VACTIVE_LEN                                                16
#define VBLANK_SHIFT                                               16
#define VBLANK_LEN                                                 16

#define VG_CONFIG_OFFSET5                                          0x3814
#define VG_CONFIG_V_FRONT_PORCH_SHIFT                              0
#define VG_CONFIG_V_FRONT_PORCH_LEN                                16
#define VG_CONFIG_V_SYNC_WIDTH_SHIFT                               16
#define VG_CONFIG_V_SYNC_WIDTH_LEN                                 16

#define VG_CONFIG_OFFSET6                                          0x3818
#define TD_STRUCTURE_SHIFT                                         0
#define TD_STRUCTURE_LEN                                           4
#define TD_ENABLE_SHIFT                                            4
#define TD_ENABLE_LEN                                              1
#define TD_FRAMESEQ_SHIFT                                          5
#define TD_FRAMESEQ_LEN                                            1
#define IPI_ENABLE_SHIFT                                           6
#define IPI_ENABLE_LEN                                             1
#define IPI_SELECT_SHIFT                                           7
#define IPI_SELECT_LEN                                             5

#define VG_RAM_ADDR_OFFSET                                         0x381c
#define RAM_ADDR_START_SHIFT                                       0
#define RAM_ADDR_START_LEN                                         13

#define VG_WRT_RAM_CTRL_OFFSET                                     0x3820
#define START_WRITE_RAM_SHIFT                                      0
#define START_WRITE_RAM_LEN                                        1

#define VG_WRT_RAM_DATA_OFFSET                                     0x3824
#define WRITE_RAM_DATA_SHIFT                                       0
#define WRITE_RAM_DATA_LEN                                         8

#define VG_WRT_RAM_STOP_ADDR_OFFSET                                0x3828
#define RAM_STOP_ADDR_SHIFT                                        0
#define RAM_STOP_ADDR_LEN                                          13

#define VG_CB_PATTERN_CONFIG_OFFSET                                0x382c
#define VG_CB_WIDTH_SHIFT                                          0
#define VG_CB_WIDTH_LEN                                            10
#define VG_CB_HEIGHT_SHIFT                                         16
#define VG_CB_HEIGHT_LEN                                           9

#define VG_CB_COLOR_A__OFFSET1                                     0x3830

#define VG_CB_COLOR_A__OFFSET2                                     0x3834
#define VG_CB_COLOR_A_MSB_SHIFT                                    0
#define VG_CB_COLOR_A_MSB_LEN                                      16

#define VG_CB_COLOR_B__OFFSET1                                     0x3838
#define VG_CB_COLOR_B__OFFSET2                                     0x383c
#define VG_CB_COLOR_B_MSB_SHIFT                                    0
#define VG_CB_COLOR_B_MSB_LEN                                      16

#define AG_SWRSTZ_OFFSET                                           0x3900
#define AG_SWRS_SHIFT                                              0
#define AG_SWRS_LEN                                                1

/* i range [1,6] */
#define AG_CONFIG_OFFSET1                                          0x3904
#define HBREN_SHIFT                                                2
#define HBREN_LEN                                                  1
#define AUDIOSOURCE_CLOCKMULTIPLIER_SHIFT                          3
#define AUDIOSOURCE_CLOCKMULTIPLIER_LEN                            2
#define I2S_WORDWIDTH_SHIFT                                        6
#define I2S_WORDWIDTH_LEN                                          4
#define AUDIO_SOURCE_SHIFT                                         10
#define AUDIO_SOURCE_LEN                                           1
#define NLPCM_EN_SHIFT                                             11
#define NLPCM_EN_LEN                                               1
#define SPDIFTXDATA_SHIFT                                          13
#define SPDIFTXDATA_LEN                                            1
#define AUDIO_USE_LUT_SHIFT                                        14
#define AUDIO_USE_LUT_LEN                                          1
#define AUDIO_USE_COUNTER_SHIFT                                    15
#define AUDIO_USE_COUNTER_LEN                                      1
#define AUDIO_COUNTER_OFFSET_SHIFT                                 16
#define AUDIO_COUNTER_OFFSET_LEN                                   8

#define AG_CONFIG_OFFSET2                                          0x3908
#define INCLEFT_SHIFT                                              0
#define INCLEFT_LEN                                                16
#define INCRIGHT_SHIFT                                             16
#define INCRIGHT_LEN                                               16

#define AG_CONFIG_OFFSET3                                          0x390c
#define IEC_COPYRIGHT_SHIFT                                        0
#define IEC_COPYRIGHT_LEN                                          1
#define IEC_CGMSA_SHIFT                                            1
#define IEC_CGMSA_LEN                                              2
#define IEC_NLPCM_SHIFT                                            3
#define IEC_NLPCM_LEN                                              1
#define IEC_CATEGORYCODE_SHIFT                                     8
#define IEC_CATEGORYCODE_LEN                                       8
#define IEC_SOURCENUMBER_SHIFT                                     16
#define IEC_SOURCENUMBER_LEN                                       4
#define IEC_PCM_AUDIO_MODE_SHIFT                                   20
#define IEC_PCM_AUDIO_MODE_LEN                                     3
#define IEC_CHANNELNUMCL0_SHIFT                                    24
#define IEC_CHANNELNUMCL0_LEN                                      4
#define IEC_CHANNELNUMCR0_SHIFT                                    28
#define IEC_CHANNELNUMCR0_LEN                                      4

#define AG_CONFIG_OFFSET4                                          0x3910
#define IEC_SAMP_FREQ_SHIFT                                        0
#define IEC_SAMP_FREQ_LEN                                          6
#define IEC_CLKACCURACY_SHIFT                                      6
#define IEC_CLKACCURACY_LEN                                        2
#define IEC_WORD_LENGTH_SHIFT                                      8
#define IEC_WORD_LENGTH_LEN                                        4
#define IEC_ORIGSAMPFREQ_SHIFT                                     12
#define IEC_ORIGSAMPFREQ_LEN                                       4

#define AG_CONFIG_OFFSET5                                          0x3914
#define AG_CONFIG5_IEC_CHANNELNUMCL0_SHIFT                         0
#define AG_CONFIG5_IEC_CHANNELNUMCL0_LEN                           4
#define AG_CONFIG5_IEC_CHANNELNUMCR0_SHIFT                         4
#define AG_CONFIG5_IEC_CHANNELNUMCR0_LEN                           4
#define AG_CONFIG5_IEC_CHANNELNUMCL1_SHIFT                         8
#define AG_CONFIG5_IEC_CHANNELNUMCL1_LEN                           4
#define AG_CONFIG5_IEC_CHANNELNUMCR1_SHIFT                         12
#define AG_CONFIG5_IEC_CHANNELNUMCR1_LEN                           4
#define AG_CONFIG5_IEC_CHANNELNUMCL2_SHIFT                         16
#define AG_CONFIG5_IEC_CHANNELNUMCL2_LEN                           4
#define AG_CONFIG5_IEC_CHANNELNUMCR2_SHIFT                         20
#define AG_CONFIG5_IEC_CHANNELNUMCR2_LEN                           4
#define AG_CONFIG5_IEC_CHANNELNUMCL2A_SHIFT                        24
#define AG_CONFIG5_IEC_CHANNELNUMCL2A_LEN                          4
#define AG_CONFIG5_IEC_CHANNELNUMCR2A_SHIFT                        28
#define AG_CONFIG5_IEC_CHANNELNUMCR2A_LEN                          4

#define AG_CONFIG_OFFSET6                                          0x3918
#define USERDATA_CL0_SHIFT                                         0
#define USERDATA_CL0_LEN                                           1
#define USERDATA_CR0_SHIFT                                         1
#define USERDATA_CR0_LEN                                           1
#define USERDATA_CL1_SHIFT                                         2
#define USERDATA_CL1_LEN                                           1
#define USERDATA_CR1_SHIFT                                         3
#define USERDATA_CR1_LEN                                           1
#define USERDATA_CL2_SHIFT                                         4
#define USERDATA_CL2_LEN                                           1
#define USERDATA_CR2_SHIFT                                         5
#define USERDATA_CR2_LEN                                           1
#define USERDATA_CL3_SHIFT                                         6
#define USERDATA_CL3_LEN                                           1
#define USERDATA_CR3_SHIFT                                         7
#define USERDATA_CR3_LEN                                           1
#define VALIDITY_BIT_CL0_SHIFT                                     8
#define VALIDITY_BIT_CL0_LEN                                       1
#define VALIDITY_BIT_CR0_SHIFT                                     9
#define VALIDITY_BIT_CR0_LEN                                       1
#define VALIDITY_BIT_CL1_SHIFT                                     10
#define VALIDITY_BIT_CL1_LEN                                       1
#define VALIDITY_BIT_CR1_SHIFT                                     11
#define VALIDITY_BIT_CR1_LEN                                       1
#define VALIDITY_BIT_CL2_SHIFT                                     12
#define VALIDITY_BIT_CL2_LEN                                       1
#define VALIDITY_BIT_CR2_SHIFT                                     13
#define VALIDITY_BIT_CR2_LEN                                       1
#define VALIDITY_BIT_CL3_SHIFT                                     14
#define VALIDITY_BIT_CL3_LEN                                       1
#define VALIDITY_BIT_CR3_SHIFT                                     15
#define VALIDITY_BIT_CR3_LEN                                       1

/* i range [0,31] */
#define DSC_PPS_OFFSET(i)                                          (0x3a00 + (i) * 0x4)
#define DSC_CRC_OFFSET1                                            0x3a80
#define DSC_CRC_ENGINE0_SHIFT                                      0
#define DSC_CRC_ENGINE0_LEN                                        16
#define DSC_CRC_ENGINE1_SHIFT                                      16
#define DSC_CRC_ENGINE1_LEN                                        16

#define DSC_CRC_OFFSET2                                            0x3a84
#define DSC_CRC_ENGINE2_SHIFT                                      0
#define DSC_CRC_ENGINE2_LEN                                        16

#endif
