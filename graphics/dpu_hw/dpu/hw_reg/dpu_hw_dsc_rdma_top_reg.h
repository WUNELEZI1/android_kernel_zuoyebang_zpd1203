/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

/* Warning: Please do not change this file yourself! */
/* File generation time: 2024-01-26 17:03:52. */

#ifndef _DPU_HW_DSC_RDMA_TOP_REG_H_
#define _DPU_HW_DSC_RDMA_TOP_REG_H_

/* REGISTER dsc_rdma_top_reg_0 */
#define DMAC_ID_TRANS_OFFSET                                          0x00
#define DMAC_ID_TRANS_SHIFT                                           0
#define DMAC_ID_TRANS_LENGTH                                          1
#define DMAC_BURST_LEN_SHIFT                                          1
#define DMAC_BURST_LEN_LENGTH                                         3
#define DAMC_QOS_MODE_SHIFT                                           4
#define DAMC_QOS_MODE_LENGTH                                          1

/* REGISTER dsc_rdma_top_reg_1 */
#define DAMC_OUTSTANDING_NUM_OFFSET                                   0x04

/* REGISTER dsc_rdma_top_reg_2 */
#define DMAC_RDMA_LIMIT_OFFSET                                        0x08
#define DMAC_RDMA_LIMIT_SHIFT                                         0
#define DMAC_RDMA_LIMIT_LENGTH                                        16

/* REGISTER dsc_rdma_top_reg_3 */
#define DSC_RDMA_TOP_IRQ_RAW_OFFSET                                   0x0c

/* REGISTER dsc_rdma_top_reg_4 */
#define DSC_RDMA_TOP_IRQ_MASK_OFFSET                                  0x10

/* REGISTER dsc_rdma_top_reg_5 */
#define DSC_RDMA_TOP_IRQ_STATUS_OFFSET                                0x14

/* REGISTER dsc_rdma_top_reg_6 */
#define ROUND_ROBIN_MODE_OFFSET                                       0x18
#define ROUND_ROBIN_MODE_SHIFT                                        0
#define ROUND_ROBIN_MODE_LENGTH                                       1
#define IMG_RR_RATIO_SHIFT                                            8
#define IMG_RR_RATIO_LENGTH                                           8
#define PIXEL_NUM_TH_SHIFT                                            16
#define PIXEL_NUM_TH_LENGTH                                           6

/* REGISTER dsc_rdma_top_reg_7 */
#define ARB_DEBUG_STA_OFFSET                                          0x1c

/* REGISTER dsc_rdma_top_reg_8 */
#define DSC_RDMA_TOP_FORCE_UPDATE_EN_OFFSET                           0x20
#define DSC_RDMA_TOP_FORCE_UPDATE_EN_SHIFT                            0
#define DSC_RDMA_TOP_FORCE_UPDATE_EN_LENGTH                           1
#define DSC_RDMA_TOP_VSYNC_UPDATE_EN_SHIFT                            1
#define DSC_RDMA_TOP_VSYNC_UPDATE_EN_LENGTH                           1
#define DSC_RDMA_TOP_SHADOW_READ_EN_SHIFT                             2
#define DSC_RDMA_TOP_SHADOW_READ_EN_LENGTH                            1

/* REGISTER dsc_rdma_top_reg_9 */
#define DSC_RDMA_TOP_FORCE_UPDATE_PULSE_OFFSET                        0x24
#define DSC_RDMA_TOP_FORCE_UPDATE_PULSE_SHIFT                         0
#define DSC_RDMA_TOP_FORCE_UPDATE_PULSE_LENGTH                        1

/* REGISTER dsc_rdma_top_reg_10 */
#define DSC_RDMA_TOP_ICG_OVERRIDE_OFFSET                              0x28
#define DSC_RDMA_TOP_ICG_OVERRIDE_SHIFT                               0
#define DSC_RDMA_TOP_ICG_OVERRIDE_LENGTH                              1

/* REGISTER dsc_rdma_top_reg_11 */
#define DSC_RDMA_TOP_TRIGGER_OFFSET                                   0x2c
#define DSC_RDMA_TOP_TRIGGER_SHIFT                                    0
#define DSC_RDMA_TOP_TRIGGER_LENGTH                                   1

/* REGISTER dsc_rdma_top_reg_12 */
#define DSC_RDMA_TOP_TRIGGER2_OFFSET                                  0x30
#define DSC_RDMA_TOP_TRIGGER2_SHIFT                                   0
#define DSC_RDMA_TOP_TRIGGER2_LENGTH                                  1

/* REGISTER dsc_rdma_top_reg_13 */
#define DSC_RDMA_TOP_RO_TRIG_ENABLE_OFFSET                            0x34
#define DSC_RDMA_TOP_RO_TRIG_ENABLE_SHIFT                             0
#define DSC_RDMA_TOP_RO_TRIG_ENABLE_LENGTH                            1

#endif
