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

#ifndef _DPU_HW_INTR_REG_H_
#define _DPU_HW_INTR_REG_H_

/* REGISTER dpu_int_reg_0 */
#define ONL0_FRM_TIMING_VSYNC_INT_MSK_OFFSET                          0x00
#define ONL0_FRM_TIMING_VSYNC_INT_MSK_SHIFT                           0
#define ONL0_FRM_TIMING_VSYNC_INT_MSK_LENGTH                          1
#define ONL0_FRM_TIMING_EOF_INT_MSK_SHIFT                             1
#define ONL0_FRM_TIMING_EOF_INT_MSK_LENGTH                            1
#define ONL0_FRM_TIMING_CFG_EOF_INT_MSK_SHIFT                         2
#define ONL0_FRM_TIMING_CFG_EOF_INT_MSK_LENGTH                        2
#define ONL0_FRM_TIMING_CFG_LINE_INT_MSK_SHIFT                        4
#define ONL0_FRM_TIMING_CFG_LINE_INT_MSK_LENGTH                       1
#define ONL0_CFG_RDY_CLR_INT_MSK_SHIFT                                5
#define ONL0_CFG_RDY_CLR_INT_MSK_LENGTH                               1
#define ONL0_WB_DONE_INT_MSK_SHIFT                                    6
#define ONL0_WB_DONE_INT_MSK_LENGTH                                   2
#define ONL0_FRM_TIMING_UNFLOW_INT_MSK_SHIFT                          8
#define ONL0_FRM_TIMING_UNFLOW_INT_MSK_LENGTH                         1
#define ONL0_WB_OVFLOW_INT_MSK_SHIFT                                  9
#define ONL0_WB_OVFLOW_INT_MSK_LENGTH                                 2
#define ONL0_CMDLIST_CH_FRM_CFG_DONE_INT_MSK_SHIFT                    11
#define ONL0_CMDLIST_CH_FRM_CFG_DONE_INT_MSK_LENGTH                   13
#define ONL0_ACAD_FRM_DONE_INT_MSK_SHIFT                              24
#define ONL0_ACAD_FRM_DONE_INT_MSK_LENGTH                             1
#define ONL0_DSCW_FRM_DONE_INT_MSK_SHIFT                              25
#define ONL0_DSCW_FRM_DONE_INT_MSK_LENGTH                             2
#define ONL0_DSCW_OVFLOW_INT_MSK_SHIFT                              27
#define ONL0_DSCW_OVERFLOW_INT_MSK_LENGTH                             2

/* REGISTER dpu_int_reg_1 */
#define ONL0_CMDLIST_CH_ENTER_PEND_INT_MSK_OFFSET                     0x04
#define ONL0_CMDLIST_CH_ENTER_PEND_INT_MSK_SHIFT                      0
#define ONL0_CMDLIST_CH_ENTER_PEND_INT_MSK_LENGTH                     13
#define ONL0_ARB_DEC_PSLVERR_INT_MSK_SHIFT                            13
#define ONL0_ARB_DEC_PSLVERR_INT_MSK_LENGTH                           1
#define ONL0_TMG_SKIP_VIDEO_TE_INIT_MSK_SHIFT                         14
#define ONL0_TMG_SKIP_VIDEO_TE_INIT_MSK_LENGTH                        1
#define ONL0_TMG_PWDN_TRIG_INT_MSK_SHIFT                              15
#define ONL0_TMG_PWDN_TRIG_INT_MSK_LENGTH                             1
#define ONL0_TMG_ULPS_EXIT_REQ_INT_MSK_SHIFT                          16
#define ONL0_TMG_ULPS_EXIT_REQ_INT_MSK_LENGTH                         1
#define ONL0_TMG_ULPS_ENTRY_REQ_INT_MSK_SHIFT                         17
#define ONL0_TMG_ULPS_ENTRY_REQ_INT_MSK_LENGTH                        1
#define ONL0_TMG_PARTITION_PWUP_TRIG_INT_MSK_SHIFT                    18
#define ONL0_TMG_PARTITION_PWUP_TRIG_INT_MSK_LENGTH                   1
#define ONL0_TMG_HOF_INT_MSK_SHIFT                                    19
#define ONL0_TMG_HOF_INT_MSK_LENGTH                                   1
#define ONL0_TMG_VACT_START_INT_MSK_SHIFT                             20
#define ONL0_TMG_VACT_START_INT_MSK_LENGTH                            1
#define DFXCORE_INT_MSK_SHIFT                                         21
#define DFXCORE_INT_MSK_LENGTH                                        1
#define ONL0_DVFS_FAIL_INT_MSK_SHIFT                                  22
#define ONL0_DVFS_FAIL_INT_MSK_LENGTH                                 1

/* REGISTER dpu_int_reg_2 */
#define ONL0_DMA_DBG_INT_MSK_OFFSET                                   0x08
#define ONL0_DMA_DBG_INT_MSK_SHIFT                                    0
#define ONL0_DMA_DBG_INT_MSK_LENGTH                                   16
#define ONL0_OUTCTL_DBG_INT_MSK_SHIFT                                 16
#define ONL0_OUTCTL_DBG_INT_MSK_LENGTH                                1
#define ONL0_CTL_DBG_INT_MSK_SHIFT                                    17
#define ONL0_CTL_DBG_INT_MSK_LENGTH                                   1
#define ONL0_CMDLIST_DBG_INT_MSK_SHIFT                                18
#define ONL0_CMDLIST_DBG_INT_MSK_LENGTH                               1
#define ONL0_DSC_P0_DBG_INT_MSK_SHIFT                                 19
#define ONL0_DSC_P0_DBG_INT_MSK_LENGTH                                1
#define ONL0_DSC_P1_DBG_INT_MSK_SHIFT                                 20
#define ONL0_DSC_P1_DBG_INT_MSK_LENGTH                                1
#define ONL0_DSCR_DBG_INT_MSK_SHIFT                                   21
#define ONL0_DSCR_DBG_INT_MSK_LENGTH                                  1

/* REGISTER dpu_int_reg_3 */
#define CMB_FRM_TIMING_VSYNC_INT_MSK_OFFSET                           0x0c
#define CMB_FRM_TIMING_VSYNC_INT_MSK_SHIFT                            0
#define CMB_FRM_TIMING_VSYNC_INT_MSK_LENGTH                           1
#define CMB_FRM_TIMING_EOF_INT_MSK_SHIFT                              1
#define CMB_FRM_TIMING_EOF_INT_MSK_LENGTH                             1
#define CMB_FRM_TIMING_CFG_EOF_INT_MSK_SHIFT                          2
#define CMB_FRM_TIMING_CFG_EOF_INT_MSK_LENGTH                         2
#define CMB_FRM_TIMING_CFG_LINE_INT_MSK_SHIFT                         4
#define CMB_FRM_TIMING_CFG_LINE_INT_MSK_LENGTH                        1
#define CMB_CFG_RDY_CLR_INT_MSK_SHIFT                                 5
#define CMB_CFG_RDY_CLR_INT_MSK_LENGTH                                1
#define CMB_WB_FRM_DONE_INT_MSK_SHIFT                                 6
#define CMB_WB_FRM_DONE_INT_MSK_LENGTH                                2
#define CMB_FRM_TIMING_UNFLOW_INT_MSK_SHIFT                           8
#define CMB_FRM_TIMING_UNFLOW_INT_MSK_LENGTH                          1
#define CMB_WB_OVFLOW_INT_MSK_SHIFT                                   9
#define CMB_WB_OVFLOW_INT_MSK_LENGTH                                  2
#define CMB_CMDLIST_CH_FRM_CFG_DONE_INT_MSK_SHIFT                     11
#define CMB_CMDLIST_CH_FRM_CFG_DONE_INT_MSK_LENGTH                    13
#define CMB_WB_SLICE_DONE_INT_MSK_SHIFT                               24
#define CMB_WB_SLICE_DONE_INT_MSK_LENGTH                              2

/* REGISTER dpu_int_reg_4 */
#define CMB_CMDLIST_CH_ENTER_PEND_INT_MSK_OFFSET                      0x10
#define CMB_CMDLIST_CH_ENTER_PEND_INT_MSK_SHIFT                       0
#define CMB_CMDLIST_CH_ENTER_PEND_INT_MSK_LENGTH                      13
#define CMB_ARB_DEC_PSLVERR_INT_MSK_SHIFT                             13
#define CMB_ARB_DEC_PSLVERR_INT_MSK_LENGTH                            1
#define CMB_TMG_SKIP_VIDEO_TE_INT_MSK_SHIFT                           14
#define CMB_TMG_SKIP_VIDEO_TE_INT_MSK_LENGTH                          1
#define CMB_TMG_PWDN_TRIG_INT_MSK_SHIFT                               15
#define CMB_TMG_PWDN_TRIG_INT_MSK_LENGTH                              1
#define CMB_TMG_ULPS_EXIT_REQ_INT_MSK_SHIFT                           16
#define CMB_TMG_ULPS_EXIT_REQ_INT_MSK_LENGTH                          1
#define CMB_TMG_ULPS_ENTRY_REQ_INT_MSK_SHIFT                          17
#define CMB_TMG_ULPS_ENTRY_REQ_INT_MSK_LENGTH                         1
#define CMB_TMG_PARTITION_PWUP_TRIG_INT_MSK_SHIFT                     18
#define CMB_TMG_PARTITION_PWUP_TRIG_INT_MSK_LENGTH                    1
#define CMB_TMG_HOF_INT_MSK_SHIFT                                     19
#define CMB_TMG_HOF_INT_MSK_LENGTH                                    1
#define CMB_TMG_VACT_START_INT_MSK_SHIFT                              20
#define CMB_TMG_VACT_START_INT_MSK_LENGTH                             1

/* REGISTER dpu_int_reg_5 */
#define CMB_DMA_DBG_INT_MSK_OFFSET                                    0x14
#define CMB_DMA_DBG_INT_MSK_SHIFT                                     0
#define CMB_DMA_DBG_INT_MSK_LENGTH                                    16
#define CMB_OUTCTL_DBG_INT_MSK_SHIFT                                  16
#define CMB_OUTCTL_DBG_INT_MSK_LENGTH                                 1
#define CMB_CTL_DBG_INT_MSK_SHIFT                                     17
#define CMB_CTL_DBG_INT_MSK_LENGTH                                    1
#define CMB_CMDLIST_DBG_INT_MSK_SHIFT                                 18
#define CMB_CMDLIST_DBG_INT_MSK_LENGTH                                1
#define CMB_DSC_P0_DBG_INT_MSK_SHIFT                                  19
#define CMB_DSC_P0_DBG_INT_MSK_LENGTH                                 1
#define CMB_DSCR_DBG_INT_MSK_SHIFT                                    20
#define CMB_DSCR_DBG_INT_MSK_LENGTH                                   1

/* REGISTER dpu_int_reg_6 */
#define OFFL0_CFG_RDY_CLR_INT_MSK_OFFSET                              0x18
#define OFFL0_CFG_RDY_CLR_INT_MSK_SHIFT                               0
#define OFFL0_CFG_RDY_CLR_INT_MSK_LENGTH                              1
#define OFFL0_WB_FRM_DONE_INT_MSK_SHIFT                               1
#define OFFL0_WB_FRM_DONE_INT_MSK_LENGTH                              2
#define OFFL0_WB_SLICE_DONE_INT_MSK_SHIFT                             3
#define OFFL0_WB_SLICE_DONE_INT_MSK_LENGTH                            2
#define OFFL0_CMDLIST_CH_FRM_CFG_DONE_INT_MSK_SHIFT                   5
#define OFFL0_CMDLIST_CH_FRM_CFG_DONE_INT_MSK_LENGTH                  13

/* REGISTER dpu_int_reg_7 */
#define OFFL0_CMDLIST_CH_ENTER_PEND_INT_MSK_OFFSET                    0x1c
#define OFFL0_CMDLIST_CH_ENTER_PEND_INT_MSK_SHIFT                     0
#define OFFL0_CMDLIST_CH_ENTER_PEND_INT_MSK_LENGTH                    13
#define OFFL0_ARB_DEC_PSLVERR_INT_MSK_SHIFT                           13
#define OFFL0_ARB_DEC_PSLVERR_INT_MSK_LENGTH                          1

/* REGISTER dpu_int_reg_8 */
#define OFFL0_NML_DMA_DBG_INT_MSK_OFFSET                              0x20
#define OFFL0_NML_DMA_DBG_INT_MSK_SHIFT                               0
#define OFFL0_NML_DMA_DBG_INT_MSK_LENGTH                              16
#define OFFL0_NML_CTL_DBG_INT_MSK_SHIFT                               16
#define OFFL0_NML_CTL_DBG_INT_MSK_LENGTH                              1
#define OFFL0_NML_CMDLIST_DBG_INT_MSK_SHIFT                           17
#define OFFL0_NML_CMDLIST_DBG_INT_MSK_LENGTH                          1

/* REGISTER dpu_int_reg_9 */
#define ONL0_FRM_TIMING_VSYNC_INTS_OFFSET                             0x24
#define ONL0_FRM_TIMING_VSYNC_INTS_SHIFT                              0
#define ONL0_FRM_TIMING_VSYNC_INTS_LENGTH                             1
#define ONL0_FRM_TIMING_EOF_INTS_SHIFT                                1
#define ONL0_FRM_TIMING_EOF_INTS_LENGTH                               1
#define ONL0_FRM_TIMING_CFG_EOF_INTS_SHIFT                            2
#define ONL0_FRM_TIMING_CFG_EOF_INTS_LENGTH                           2
#define ONL0_FRM_TIMING_CFG_LINE_INTS_SHIFT                           4
#define ONL0_FRM_TIMING_CFG_LINE_INTS_LENGTH                          1
#define ONL0_CFG_RDY_CLR_INTS_SHIFT                                   5
#define ONL0_CFG_RDY_CLR_INTS_LENGTH                                  1
#define ONL0_WB_DONE_INTS_SHIFT                                       6
#define ONL0_WB_DONE_INTS_LENGTH                                      2
#define ONL0_FRM_TIMING_UNFLOW_INTS_SHIFT                             8
#define ONL0_FRM_TIMING_UNFLOW_INTS_LENGTH                            1
#define ONL0_WB_OVFLOW_INTS_SHIFT                                     9
#define ONL0_WB_OVFLOW_INTS_LENGTH                                    2
#define ONL0_CMDLIST_CH_FRM_CFG_DONE_INTS_SHIFT                       11
#define ONL0_CMDLIST_CH_FRM_CFG_DONE_INTS_LENGTH                      13
#define ONL0_ACAD_FRM_DONE_INTS_SHIFT                                 24
#define ONL0_ACAD_FRM_DONE_INTS_LENGTH                                1
#define ONL0_DSCW_FRM_DONE_INTS_SHIFT                                 25
#define ONL0_DSCW_FRM_DONE_INTS_LENGTH                                2
#define ONL0_DSCW_OVFLOW_INTS_SHIFT                                   27
#define ONL0_DSCW_OVFLOW_INTS_LENGTH                                  2

/* REGISTER dpu_int_reg_10 */
#define ONL0_CMDLIST_CH_ENTER_PEND_INTS_OFFSET                        0x28
#define ONL0_CMDLIST_CH_ENTER_PEND_INTS_SHIFT                         0
#define ONL0_CMDLIST_CH_ENTER_PEND_INTS_LENGTH                        13
#define ONL0_ARB_DEC_PSLVERR_INTS_SHIFT                               13
#define ONL0_ARB_DEC_PSLVERR_INTS_LENGTH                              1
#define ONL0_TMG_SKIP_VIDEO_TE_INITS_SHIFT                            14
#define ONL0_TMG_SKIP_VIDEO_TE_INITS_LENGTH                           1
#define ONL0_TMG_PWDN_TRIG_INTS_SHIFT                                 15
#define ONL0_TMG_PWDN_TRIG_INTS_LENGTH                                1
#define ONL0_TMG_ULPS_EXIT_REQ_INTS_SHIFT                             16
#define ONL0_TMG_ULPS_EXIT_REQ_INTS_LENGTH                            1
#define ONL0_TMG_ULPS_ENTRY_REQ_INTS_SHIFT                            17
#define ONL0_TMG_ULPS_ENTRY_REQ_INTS_LENGTH                           1
#define ONL0_TMG_PARTITION_PWUP_TRIG_INTS_SHIFT                       18
#define ONL0_TMG_PARTITION_PWUP_TRIG_INTS_LENGTH                      1
#define ONL0_TMG_HOF_INTS_SHIFT                                       19
#define ONL0_TMG_HOF_INTS_LENGTH                                      1
#define ONL0_TMG_VACT_START_INTS_SHIFT                                20
#define ONL0_TMG_VACT_START_INTS_LENGTH                               1
#define DFXCORE_INTS_SHIFT                                            21
#define DFXCORE_INTS_LENGTH                                           1
#define ONL0_DVFS_FAIL_INTS_SHIFT                                     22
#define ONL0_DVFS_FAIL_INTS_LENGTH                                    1

/* REGISTER dpu_int_reg_11 */
#define ONL0_DMA_DBG_INTS_OFFSET                                      0x2c
#define ONL0_DMA_DBG_INTS_SHIFT                                       0
#define ONL0_DMA_DBG_INTS_LENGTH                                      16
#define ONL0_OUTCTL_DBG_INTS_SHIFT                                    16
#define ONL0_OUTCTL_DBG_INTS_LENGTH                                   1
#define ONL0_CTL_DBG_INTS_SHIFT                                       17
#define ONL0_CTL_DBG_INTS_LENGTH                                      1
#define ONL0_CMDLIST_DBG_INTS_SHIFT                                   18
#define ONL0_CMDLIST_DBG_INTS_LENGTH                                  1
#define ONL0_DSC_P0_DBG_INTS_SHIFT                                    19
#define ONL0_DSC_P0_DBG_INTS_LENGTH                                   1
#define ONL0_DSC_P1_DBG_INTS_SHIFT                                    20
#define ONL0_DSC_P1_DBG_INTS_LENGTH                                   1
#define ONL0_DSCR_DBG_INTS_SHIFT                                      21
#define ONL0_DSCR_DBG_INTS_LENGTH                                     1

/* REGISTER dpu_int_reg_12 */
#define CMB_FRM_TIMING_VSYNC_INTS_OFFSET                              0x30
#define CMB_FRM_TIMING_VSYNC_INTS_SHIFT                               0
#define CMB_FRM_TIMING_VSYNC_INTS_LENGTH                              1
#define CMB_FRM_TIMING_EOF_INTS_SHIFT                                 1
#define CMB_FRM_TIMING_EOF_INTS_LENGTH                                1
#define CMB_FRM_TIMING_CFG_EOF_INTS_SHIFT                             2
#define CMB_FRM_TIMING_CFG_EOF_INTS_LENGTH                            2
#define CMB_FRM_TIMING_CFG_LINE_INTS_SHIFT                            4
#define CMB_FRM_TIMING_CFG_LINE_INTS_LENGTH                           1
#define CMB_CFG_RDY_CLR_INTS_SHIFT                                    5
#define CMB_CFG_RDY_CLR_INTS_LENGTH                                   1
#define CMB_WB_FRM_DONE_INTS_SHIFT                                    6
#define CMB_WB_FRM_DONE_INTS_LENGTH                                   2
#define CMB_FRM_TIMING_UNFLOW_INTS_SHIFT                              8
#define CMB_FRM_TIMING_UNFLOW_INTS_LENGTH                             1
#define CMB_WB_OVFLOW_INTS_SHIFT                                      9
#define CMB_WB_OVFLOW_INTS_LENGTH                                     2
#define CMB_CMDLIST_CH_FRM_CFG_DONE_INTS_SHIFT                        11
#define CMB_CMDLIST_CH_FRM_CFG_DONE_INTS_LENGTH                       13
#define CMB_WB_SLICE_DONE_INTS_SHIFT                                  24
#define CMB_WB_SLICE_DONE_INTS_LENGTH                                 2

/* REGISTER dpu_int_reg_13 */
#define CMB_CMDLIST_CH_ENTER_PEND_INTS_OFFSET                         0x34
#define CMB_CMDLIST_CH_ENTER_PEND_INTS_SHIFT                          0
#define CMB_CMDLIST_CH_ENTER_PEND_INTS_LENGTH                         13
#define CMB_ARB_DEC_PSLVERR_INTS_SHIFT                                13
#define CMB_ARB_DEC_PSLVERR_INTS_LENGTH                               1
#define CMB_TMG_SKIP_VIDEO_TE_INITS_SHIFT                             14
#define CMB_TMG_SKIP_VIDEO_TE_INITS_LENGTH                            1
#define CMB_TMG_PWDN_TRIG_INTS_SHIFT                                  15
#define CMB_TMG_PWDN_TRIG_INTS_LENGTH                                 1
#define CMB_TMG_ULPS_EXIT_REQ_INTS_SHIFT                              16
#define CMB_TMG_ULPS_EXIT_REQ_INTS_LENGTH                             1
#define CMB_TMG_ULPS_ENTRY_REQ_INTS_SHIFT                             17
#define CMB_TMG_ULPS_ENTRY_REQ_INTS_LENGTH                            1
#define CMB_TMG_PARTITION_PWUP_TRIG_INTS_SHIFT                        18
#define CMB_TMG_PARTITION_PWUP_TRIG_INTS_LENGTH                       1
#define CMB_TMG_HOF_INTS_SHIFT                                        19
#define CMB_TMG_HOF_INTS_LENGTH                                       1
#define CMB_TMG_VACT_START_INTS_SHIFT                                 20
#define CMB_TMG_VACT_START_INTS_LENGTH                                1

/* REGISTER dpu_int_reg_14 */
#define CMB_DMA_DBG_INTS_OFFSET                                       0x38
#define CMB_DMA_DBG_INTS_SHIFT                                        0
#define CMB_DMA_DBG_INTS_LENGTH                                       16
#define CMB_OUTCTL_DBG_INTS_SHIFT                                     16
#define CMB_OUTCTL_DBG_INTS_LENGTH                                    1
#define CMB_CTL_DBG_INTS_SHIFT                                        17
#define CMB_CTL_DBG_INTS_LENGTH                                       1
#define CMB_CMDLIST_DBG_INTS_SHIFT                                    18
#define CMB_CMDLIST_DBG_INTS_LENGTH                                   1
#define CMB_DSC_P0_DBG_INTS_SHIFT                                     19
#define CMB_DSC_P0_DBG_INTS_LENGTH                                    1
#define CMB_DSCR_DBG_INTS_SHIFT                                       20
#define CMB_DSCR_DBG_INTS_LENGTH                                      1

/* REGISTER dpu_int_reg_15 */
#define OFFL0_CFG_RDY_CLR_INTS_OFFSET                                 0x3c
#define OFFL0_CFG_RDY_CLR_INTS_SHIFT                                  0
#define OFFL0_CFG_RDY_CLR_INTS_LENGTH                                 1
#define OFFL0_WB_FRM_DONE_INTS_SHIFT                                  1
#define OFFL0_WB_FRM_DONE_INTS_LENGTH                                 2
#define OFFL0_WB_SLICE_DONE_INTS_SHIFT                                3
#define OFFL0_WB_SLICE_DONE_INTS_LENGTH                               2
#define OFFL0_CMDLIST_CH_FRM_CFG_DONE_INTS_SHIFT                      5
#define OFFL0_CMDLIST_CH_FRM_CFG_DONE_INTS_LENGTH                     13

/* REGISTER dpu_int_reg_16 */
#define OFF0_CMDLIST_CH_ENTER_PEND_INTS_OFFSET                        0x40
#define OFF0_CMDLIST_CH_ENTER_PEND_INTS_SHIFT                         0
#define OFF0_CMDLIST_CH_ENTER_PEND_INTS_LENGTH                        13
#define OFF0_ARB_DEC_PSLVERR_INTS_SHIFT                               13
#define OFF0_ARB_DEC_PSLVERR_INTS_LENGTH                              1

/* REGISTER dpu_int_reg_17 */
#define OFFL0_NML_DMA_DBG_INTS_OFFSET                                 0x44
#define OFFL0_NML_DMA_DBG_INTS_SHIFT                                  0
#define OFFL0_NML_DMA_DBG_INTS_LENGTH                                 16
#define OFFL0_NML_CTL_DBG_INTS_SHIFT                                  16
#define OFFL0_NML_CTL_DBG_INTS_LENGTH                                 1
#define OFFL0_NML_CMDLIST_DBG_INTS_SHIFT                              17
#define OFFL0_NML_CMDLIST_DBG_INTS_LENGTH                             1

/* REGISTER dpu_int_reg_18 */
#define ONL0_FRM_TIMING_VSYNC_INT_RAW_OFFSET                          0x48
#define ONL0_FRM_TIMING_VSYNC_INT_RAW_SHIFT                           0
#define ONL0_FRM_TIMING_VSYNC_INT_RAW_LENGTH                          1
#define ONL0_FRM_TIMING_EOF_INT_RAW_SHIFT                             1
#define ONL0_FRM_TIMING_EOF_INT_RAW_LENGTH                            1
#define ONL0_FRM_TIMING_CFG_EOF_INT_RAW_SHIFT                         2
#define ONL0_FRM_TIMING_CFG_EOF_INT_RAW_LENGTH                        2
#define ONL0_FRM_TIMING_CFG_LINE_INT_RAW_SHIFT                        4
#define ONL0_FRM_TIMING_CFG_LINE_INT_RAW_LENGTH                       1
#define ONL0_CFG_RDY_CLR_INT_RAW_SHIFT                                5
#define ONL0_CFG_RDY_CLR_INT_RAW_LENGTH                               1
#define ONL0_WB_DONE_INT_RAW_SHIFT                                    6
#define ONL0_WB_DONE_INT_RAW_LENGTH                                   2
#define ONL0_FRM_TIMING_UNFLOW_INT_RAW_SHIFT                          8
#define ONL0_FRM_TIMING_UNFLOW_INT_RAW_LENGTH                         1
#define ONL0_WB_OVFLOW_INT_RAW_SHIFT                                  9
#define ONL0_WB_OVFLOW_INT_RAW_LENGTH                                 2
#define ONL0_CMDLIST_CH_FRM_CFG_DONE_INT_RAW_SHIFT                    11
#define ONL0_CMDLIST_CH_FRM_CFG_DONE_INT_RAW_LENGTH                   13
#define ONL0_ACAD_FRM_DONE_INT_RAW_SHIFT                              24
#define ONL0_ACAD_FRM_DONE_INT_RAW_LENGTH                             1
#define ONL0_DSCW_FRM_DONE_INT_RAW_SHIFT                              25
#define ONL0_DSCW_FRM_DONE_INT_RAW_LENGTH                             2
#define ONL0_DSCW_OVFLOW_INT_RAW_SHIFT                                27
#define ONL0_DSCW_OVFLOW_INT_RAW_LENGTH                               2

/* REGISTER dpu_int_reg_19 */
#define ONL0_CMDLIST_CH_ENTER_PEND_INT_RAW_OFFSET                     0x4c
#define ONL0_CMDLIST_CH_ENTER_PEND_INT_RAW_SHIFT                      0
#define ONL0_CMDLIST_CH_ENTER_PEND_INT_RAW_LENGTH                     13
#define ONL0_ARB_DEC_PSLVERR_INT_RAW_SHIFT                            13
#define ONL0_ARB_DEC_PSLVERR_INT_RAW_LENGTH                           1
#define ONL0_TMG_SKIP_VIDEO_TE_INIT_RAW_SHIFT                         14
#define ONL0_TMG_SKIP_VIDEO_TE_INIT_RAW_LENGTH                        1
#define ONL0_TMG_PWDN_TRIG_INT_RAW_SHIFT                              15
#define ONL0_TMG_PWDN_TRIG_INT_RAW_LENGTH                             1
#define ONL0_TMG_ULPS_EXIT_REQ_INT_RAW_SHIFT                          16
#define ONL0_TMG_ULPS_EXIT_REQ_INT_RAW_LENGTH                         1
#define ONL0_TMG_ULPS_ENTRY_REQ_INT_RAW_SHIFT                         17
#define ONL0_TMG_ULPS_ENTRY_REQ_INT_RAW_LENGTH                        1
#define ONL0_TMG_PARTITION_PWUP_TRIG_INT_RAW_SHIFT                    18
#define ONL0_TMG_PARTITION_PWUP_TRIG_INT_RAW_LENGTH                   1
#define ONL0_TMG_HOF_INT_RAW_SHIFT                                    19
#define ONL0_TMG_HOF_INT_RAW_LENGTH                                   1
#define ONL0_TMG_VACT_START_INT_RAW_SHIFT                             20
#define ONL0_TMG_VACT_START_INT_RAW_LENGTH                            1
#define DFXCORE_INT_RAW_SHIFT                                         21
#define DFXCORE_INT_RAW_LENGTH                                        1
#define ONL0_DVFS_FAIL_INT_RAW_SHIFT                                  22
#define ONL0_DVFS_FAIL_INT_RAW_LENGTH                                 1

/* REGISTER dpu_int_reg_20 */
#define ONL0_DMA_DBG_INT_RAW_OFFSET                                   0x50
#define ONL0_DMA_DBG_INT_RAW_SHIFT                                    0
#define ONL0_DMA_DBG_INT_RAW_LENGTH                                   16
#define ONL0_OUTCTL_DBG_INT_RAW_SHIFT                                 16
#define ONL0_OUTCTL_DBG_INT_RAW_LENGTH                                1
#define ONL0_CTL_DBG_INT_RAW_SHIFT                                    17
#define ONL0_CTL_DBG_INT_RAW_LENGTH                                   1
#define ONL0_CMDLIST_DBG_INT_RAW_SHIFT                                18
#define ONL0_CMDLIST_DBG_INT_RAW_LENGTH                               1
#define ONL0_DSC_P0_DBG_INT_RAW_SHIFT                                 19
#define ONL0_DSC_P0_DBG_INT_RAW_LENGTH                                1
#define ONL0_DSC_P1_DBG_INT_RAW_SHIFT                                 20
#define ONL0_DSC_P1_DBG_INT_RAW_LENGTH                                1
#define ONL0_DSCR_DBG_INT_RAW_SHIFT                                   21
#define ONL0_DSCR_DBG_INT_RAW_LENGTH                                  1

/* REGISTER dpu_int_reg_21 */
#define ONL0_ARB_DEC_PADDR_OFFSET                                     0x54

/* REGISTER dpu_int_reg_22 */
#define CMB_FRM_TIMING_VSYNC_INT_RAW_OFFSET                           0x58
#define CMB_FRM_TIMING_VSYNC_INT_RAW_SHIFT                            0
#define CMB_FRM_TIMING_VSYNC_INT_RAW_LENGTH                           1
#define CMB_FRM_TIMING_EOF_INT_RAW_SHIFT                              1
#define CMB_FRM_TIMING_EOF_INT_RAW_LENGTH                             1
#define CMB_FRM_TIMING_CFG_EOF_INT_RAW_SHIFT                          2
#define CMB_FRM_TIMING_CFG_EOF_INT_RAW_LENGTH                         2
#define CMB_FRM_TIMING_CFG_LINE_INT_RAW_SHIFT                         4
#define CMB_FRM_TIMING_CFG_LINE_INT_RAW_LENGTH                        1
#define CMB_CFG_RDY_CLR_INT_RAW_SHIFT                                 5
#define CMB_CFG_RDY_CLR_INT_RAW_LENGTH                                1
#define CMB_WB_FRM_DONE_INT_RAW_SHIFT                                 6
#define CMB_WB_FRM_DONE_INT_RAW_LENGTH                                2
#define CMB_FRM_TIMING_UNFLOW_INT_RAW_SHIFT                           8
#define CMB_FRM_TIMING_UNFLOW_INT_RAW_LENGTH                          1
#define CMB_WB_OVFLOW_INT_RAW_SHIFT                                   9
#define CMB_WB_OVFLOW_INT_RAW_LENGTH                                  2
#define CMB_CMDLIST_CH_FRM_CFG_DONE_INT_RAW_SHIFT                     11
#define CMB_CMDLIST_CH_FRM_CFG_DONE_INT_RAW_LENGTH                    13
#define CMB_WB_SLICE_DONE_INT_RAW_SHIFT                               24
#define CMB_WB_SLICE_DONE_INT_RAW_LENGTH                              2

/* REGISTER dpu_int_reg_23 */
#define CMB_CMDLIST_CH_ENTER_PEND_INT_RAW_OFFSET                      0x5c
#define CMB_CMDLIST_CH_ENTER_PEND_INT_RAW_SHIFT                       0
#define CMB_CMDLIST_CH_ENTER_PEND_INT_RAW_LENGTH                      13
#define CMB_ARB_DEC_PSLVERR_INT_RAW_SHIFT                             13
#define CMB_ARB_DEC_PSLVERR_INT_RAW_LENGTH                            1
#define CMB_TMG_SKIP_VIDEO_TE_INT_RAW_SHIFT                           14
#define CMB_TMG_SKIP_VIDEO_TE_INT_RAW_LENGTH                          1
#define CMB_TMG_PWDN_TRIG_INT_RAW_SHIFT                               15
#define CMB_TMG_PWDN_TRIG_INT_RAW_LENGTH                              1
#define CMB_TMG_ULPS_EXIT_REQ_INT_RAW_SHIFT                           16
#define CMB_TMG_ULPS_EXIT_REQ_INT_RAW_LENGTH                          1
#define CMB_TMG_ULPS_ENTRY_REQ_INT_RAW_SHIFT                          17
#define CMB_TMG_ULPS_ENTRY_REQ_INT_RAW_LENGTH                         1
#define CMB_TMG_PARTITION_PWUP_TRIG_INT_RAW_SHIFT                     18
#define CMB_TMG_PARTITION_PWUP_TRIG_INT_RAW_LENGTH                    1
#define CMB_TMG_HOF_INT_RAW_SHIFT                                     19
#define CMB_TMG_HOF_INT_RAW_LENGTH                                    1
#define CMB_TMG_VACT_START_INT_RAW_SHIFT                              20
#define CMB_TMG_VACT_START_INT_RAW_LENGTH                             1

/* REGISTER dpu_int_reg_24 */
#define CMB_DMA_DBG_INT_RAW_OFFSET                                    0x60
#define CMB_DMA_DBG_INT_RAW_SHIFT                                     0
#define CMB_DMA_DBG_INT_RAW_LENGTH                                    16
#define CMB_OUTCTL_DBG_INT_RAW_SHIFT                                  16
#define CMB_OUTCTL_DBG_INT_RAW_LENGTH                                 1
#define CMB_CTL_DBG_INT_RAW_SHIFT                                     17
#define CMB_CTL_DBG_INT_RAW_LENGTH                                    1
#define CMB_CMDLIST_DBG_INT_RAW_SHIFT                                 18
#define CMB_CMDLIST_DBG_INT_RAW_LENGTH                                1
#define CMB_DSC_P0_DBG_INT_RAW_SHIFT                                  19
#define CMB_DSC_P0_DBG_INT_RAW_LENGTH                                 1
#define CMB_DSCR_DBG_INT_RAW_SHIFT                                    20
#define CMB_DSCR_DBG_INT_RAW_LENGTH                                   1

/* REGISTER dpu_int_reg_25 */
#define CMB_ARB_DEC_PADDR_OFFSET                                      0x64

/* REGISTER dpu_int_reg_26 */
#define OFFL0_CFG_RDY_CLR_INT_RAW_OFFSET                              0x68
#define OFFL0_CFG_RDY_CLR_INT_RAW_SHIFT                               0
#define OFFL0_CFG_RDY_CLR_INT_RAW_LENGTH                              1
#define OFFL0_WB_FRM_DONE_INT_RAW_SHIFT                               1
#define OFFL0_WB_FRM_DONE_INT_RAW_LENGTH                              2
#define OFFL0_WB_SLICE_DONE_INT_RAW_SHIFT                             3
#define OFFL0_WB_SLICE_DONE_INT_RAW_LENGTH                            2
#define OFFL0_CMDLIST_CH_FRM_CFG_DONE_INT_RAW_SHIFT                   5
#define OFFL0_CMDLIST_CH_FRM_CFG_DONE_INT_RAW_LENGTH                  13

/* REGISTER dpu_int_reg_27 */
#define OFF0_CMDLIST_CH_ENTER_PEND_INT_RAW_OFFSET                     0x6c
#define OFF0_CMDLIST_CH_ENTER_PEND_INT_RAW_SHIFT                      0
#define OFF0_CMDLIST_CH_ENTER_PEND_INT_RAW_LENGTH                     13
#define OFF0_ARB_DEC_PSLVERR_INT_RAW_SHIFT                            13
#define OFF0_ARB_DEC_PSLVERR_INT_RAW_LENGTH                           1

/* REGISTER dpu_int_reg_28 */
#define OFFL0_NML_DMA_DBG_INT_RAW_OFFSET                              0x70
#define OFFL0_NML_DMA_DBG_INT_RAW_SHIFT                               0
#define OFFL0_NML_DMA_DBG_INT_RAW_LENGTH                              16
#define OFFL0_NML_CTL_DBG_INT_RAW_SHIFT                               16
#define OFFL0_NML_CTL_DBG_INT_RAW_LENGTH                              1
#define OFFL0_NML_CMDLIST_DBG_INT_RAW_SHIFT                           17
#define OFFL0_NML_CMDLIST_DBG_INT_RAW_LENGTH                          1

/* REGISTER dpu_int_reg_29 */
#define OFFL0_ARB_DEC_PADDR_OFFSET                                    0x74

/* REGISTER dpu_int_reg_30 */
#define ONL0_SECU_FRM_TIMING_VSYNC_INT_MSK_OFFSET                     0x78
#define ONL0_SECU_FRM_TIMING_VSYNC_INT_MSK_SHIFT                      0
#define ONL0_SECU_FRM_TIMING_VSYNC_INT_MSK_LENGTH                     1
#define ONL0_SECU_FRM_TIMING_EOF_INT_MSK_SHIFT                        1
#define ONL0_SECU_FRM_TIMING_EOF_INT_MSK_LENGTH                       1
#define ONL0_SECU_FRM_TIMING_CFG_EOF_INT_MSK_SHIFT                    2
#define ONL0_SECU_FRM_TIMING_CFG_EOF_INT_MSK_LENGTH                   2
#define ONL0_SECU_FRM_TIMING_CFG_LINE_INT_MSK_SHIFT                   4
#define ONL0_SECU_FRM_TIMING_CFG_LINE_INT_MSK_LENGTH                  1
#define ONL0_SECU_CFG_RDY_CLR_INT_MSK_SHIFT                           5
#define ONL0_SECU_CFG_RDY_CLR_INT_MSK_LENGTH                          1
#define ONL0_SECU_WB_DONE_INT_MSK_SHIFT                               6
#define ONL0_SECU_WB_DONE_INT_MSK_LENGTH                              2
#define ONL0_SECU_FRM_TIMING_UNFLOW_INT_MSK_SHIFT                     8
#define ONL0_SECU_FRM_TIMING_UNFLOW_INT_MSK_LENGTH                    1
#define ONL0_SECU_WB_OVFLOW_INT_MSK_SHIFT                             9
#define ONL0_SECU_WB_OVFLOW_INT_MSK_LENGTH                            2
#define ONL0_SECU_CMDLIST_CH_FRM_CFG_DONE_INT_MSK_SHIFT               11
#define ONL0_SECU_CMDLIST_CH_FRM_CFG_DONE_INT_MSK_LENGTH              13
#define ONL0_SECU_ACAD_FRM_DONE_INT_MSK_SHIFT                         24
#define ONL0_SECU_ACAD_FRM_DONE_INT_MSK_LENGTH                        1
#define ONL0_SECU_DSCW_FRM_DONE_INT_MSK_SHIFT                         25
#define ONL0_SECU_DSCW_FRM_DONE_INT_MSK_LENGTH                        2
#define ONL0_SECU_DSCW_OVFLOW_INT_MSK_SHIFT                           27
#define ONL0_SECU_DSCW_OVFLOW_INT_MSK_LENGTH                          2

/* REGISTER dpu_int_reg_31 */
#define ONL0_SECU_CMDLIST_CH_ENTER_PEND_INT_MSK_OFFSET                0x7c
#define ONL0_SECU_CMDLIST_CH_ENTER_PEND_INT_MSK_SHIFT                 0
#define ONL0_SECU_CMDLIST_CH_ENTER_PEND_INT_MSK_LENGTH                13
#define ONL0_SECU_ARB_DEC_PSLVERR_INT_MSK_SHIFT                       13
#define ONL0_SECU_ARB_DEC_PSLVERR_INT_MSK_LENGTH                      1
#define ONL0_SECU_TMG_SKIP_VIDEO_TE_INT_MSK_SHIFT                     14
#define ONL0_SECU_TMG_SKIP_VIDEO_TE_INT_MSK_LENGTH                    1
#define ONL0_SECU_TMG_PWDN_TRIG_INT_MSK_SHIFT                         15
#define ONL0_SECU_TMG_PWDN_TRIG_INT_MSK_LENGTH                        1
#define ONL0_SECU_TMG_ULPS_EXIT_REQ_INT_MSK_SHIFT                     16
#define ONL0_SECU_TMG_ULPS_EXIT_REQ_INT_MSK_LENGTH                    1
#define ONL0_SECU_TMG_ULPS_ENTRY_REQ_INT_MSK_SHIFT                    17
#define ONL0_SECU_TMG_ULPS_ENTRY_REQ_INT_MSK_LENGTH                   1
#define ONL0_SECU_TMG_PARTITION_PWUP_TRIG_INT_MSK_SHIFT               18
#define ONL0_SECU_TMG_PARTITION_PWUP_TRIG_INT_MSK_LENGTH              1
#define ONL0_SECU_TMG_HOF_INT_MSK_SHIFT                               19
#define ONL0_SECU_TMG_HOF_INT_MSK_LENGTH                              1
#define ONL0_SECU_TMG_VACT_START_INT_MSK_SHIFT                        20
#define ONL0_SECU_TMG_VACT_START_INT_MSK_LENGTH                       1
#define ONL0_SECU_DDR_DVFS_REQ_INT_MSK_SHIFT                          21
#define ONL0_SECU_DDR_DVFS_REQ_INT_MSK_LENGTH                         1
#define ONL0_SECU_DVFS_FAIL_INT_MSK_SHIFT                             22
#define ONL0_SECU_DVFS_FAIL_INT_MSK_LENGTH                            1

/* REGISTER dpu_int_reg_32 */
#define ONL0_SECU_DMA_DBG_INT_MSK_OFFSET                              0x80
#define ONL0_SECU_DMA_DBG_INT_MSK_SHIFT                               0
#define ONL0_SECU_DMA_DBG_INT_MSK_LENGTH                              16
#define ONL0_SECU_OUTCTL_DBG_INT_MSK_SHIFT                            16
#define ONL0_SECU_OUTCTL_DBG_INT_MSK_LENGTH                           1
#define ONL0_SECU_CTL_DBG_INT_MSK_SHIFT                               17
#define ONL0_SECU_CTL_DBG_INT_MSK_LENGTH                              1
#define ONL0_SECU_CMDLIST_DBG_INT_MSK_SHIFT                           18
#define ONL0_SECU_CMDLIST_DBG_INT_MSK_LENGTH                          1
#define ONL0_SECU_DSC_P0_DBG_INT_MSK_SHIFT                            19
#define ONL0_SECU_DSC_P0_DBG_INT_MSK_LENGTH                           1
#define ONL0_SECU_DSC_P1_DBG_INT_MSK_SHIFT                            20
#define ONL0_SECU_DSC_P1_DBG_INT_MSK_LENGTH                           1
#define ONL0_SECU_DSCR_DBG_INT_MSK_SHIFT                              21
#define ONL0_SECU_DSCR_DBG_INT_MSK_LENGTH                             1

/* REGISTER dpu_int_reg_33 */
#define ONL0_SECU_FRM_TIMING_VSYNC_INTS_OFFSET                        0x84
#define ONL0_SECU_FRM_TIMING_VSYNC_INTS_SHIFT                         0
#define ONL0_SECU_FRM_TIMING_VSYNC_INTS_LENGTH                        1
#define ONL0_SECU_FRM_TIMING_EOF_INTS_SHIFT                           1
#define ONL0_SECU_FRM_TIMING_EOF_INTS_LENGTH                          1
#define ONL0_SECU_FRM_TIMING_CFG_EOF_INTS_SHIFT                       2
#define ONL0_SECU_FRM_TIMING_CFG_EOF_INTS_LENGTH                      2
#define ONL0_SECU_FRM_TIMING_CFG_LINE_INTS_SHIFT                      4
#define ONL0_SECU_FRM_TIMING_CFG_LINE_INTS_LENGTH                     1
#define ONL0_SECU_CFG_RDY_CLR_INTS_SHIFT                              5
#define ONL0_SECU_CFG_RDY_CLR_INTS_LENGTH                             1
#define ONL0_SECU_WB_DONE_INTS_SHIFT                                  6
#define ONL0_SECU_WB_DONE_INTS_LENGTH                                 2
#define ONL0_SECU_FRM_TIMING_UNFLOW_INTS_SHIFT                        8
#define ONL0_SECU_FRM_TIMING_UNFLOW_INTS_LENGTH                       1
#define ONL0_SECU_WB_OVFLOW_INTS_SHIFT                                9
#define ONL0_SECU_WB_OVFLOW_INTS_LENGTH                               2
#define ONL0_SECU_CMDLIST_CH_FRM_CFG_DONE_INTS_SHIFT                  11
#define ONL0_SECU_CMDLIST_CH_FRM_CFG_DONE_INTS_LENGTH                 13
#define ONL0_SECU_ACAD_FRM_DONE_INTS_SHIFT                            24
#define ONL0_SECU_ACAD_FRM_DONE_INTS_LENGTH                           1
#define ONL0_SECU_DSCW_FRM_DONE_INTS_SHIFT                            25
#define ONL0_SECU_DSCW_FRM_DONE_INTS_LENGTH                           2
#define ONL0_SECU_DSCW_OVFLOW_INTS_SHIFT                              27
#define ONL0_SECU_DSCW_OVFLOW_INTS_LENGTH                             2

/* REGISTER dpu_int_reg_34 */
#define ONL0_SECU_CMDLIST_CH_ENTER_PEND_INTS_OFFSET                   0x88
#define ONL0_SECU_CMDLIST_CH_ENTER_PEND_INTS_SHIFT                    0
#define ONL0_SECU_CMDLIST_CH_ENTER_PEND_INTS_LENGTH                   13
#define ONL0_SECU_ARB_DEC_PSLVERR_INTS_SHIFT                          13
#define ONL0_SECU_ARB_DEC_PSLVERR_INTS_LENGTH                         1
#define ONL0_SECU_TMG_SKIP_VIDEO_TE_INTS_SHIFT                        14
#define ONL0_SECU_TMG_SKIP_VIDEO_TE_INTS_LENGTH                       1
#define ONL0_SECU_TMG_PWDN_TRIG_INTS_SHIFT                            15
#define ONL0_SECU_TMG_PWDN_TRIG_INTS_LENGTH                           1
#define ONL0_SECU_TMG_ULPS_EXIT_REQ_INTS_SHIFT                        16
#define ONL0_SECU_TMG_ULPS_EXIT_REQ_INTS_LENGTH                       1
#define ONL0_SECU_TMG_ULPS_ENTRY_REQ_INTS_SHIFT                       17
#define ONL0_SECU_TMG_ULPS_ENTRY_REQ_INTS_LENGTH                      1
#define ONL0_SECU_TMG_PARTITION_PWUP_TRIG_INTS_SHIFT                  18
#define ONL0_SECU_TMG_PARTITION_PWUP_TRIG_INTS_LENGTH                 1
#define ONL0_SECU_TMG_HOF_INTS_SHIFT                                  19
#define ONL0_SECU_TMG_HOF_INTS_LENGTH                                 1
#define ONL0_SECU_TMG_VACT_START_INTS_SHIFT                           20
#define ONL0_SECU_TMG_VACT_START_INTS_LENGTH                          1
#define ONL0_SECU_DDR_DVFS_REQ_INTS_SHIFT                             21
#define ONL0_SECU_DDR_DVFS_REQ_INTS_LENGTH                            1
#define ONL0_SECU_DVFS_FAIL_INTS_SHIFT                                22
#define ONL0_SECU_DVFS_FAIL_INTS_LENGTH                               1

/* REGISTER dpu_int_reg_35 */
#define ONL0_SECU_DMA_DBG_INTS_OFFSET                                 0x8c
#define ONL0_SECU_DMA_DBG_INTS_SHIFT                                  0
#define ONL0_SECU_DMA_DBG_INTS_LENGTH                                 16
#define ONL0_SECU_OUTCTL_DBG_INTS_SHIFT                               16
#define ONL0_SECU_OUTCTL_DBG_INTS_LENGTH                              1
#define ONL0_SECU_CTL_DBG_INTS_SHIFT                                  17
#define ONL0_SECU_CTL_DBG_INTS_LENGTH                                 1
#define ONL0_SECU_CMDLIST_DBG_INTS_SHIFT                              18
#define ONL0_SECU_CMDLIST_DBG_INTS_LENGTH                             1
#define ONL0_SECU_DSC_P0_DBG_INTS_SHIFT                               19
#define ONL0_SECU_DSC_P0_DBG_INTS_LENGTH                              1
#define ONL0_SECU_DSC_P1_DBG_INTS_SHIFT                               20
#define ONL0_SECU_DSC_P1_DBG_INTS_LENGTH                              1
#define ONL0_SECU_DSCR_DBG_INTS_SHIFT                                 21
#define ONL0_SECU_DSCR_DBG_INTS_LENGTH                                1

/* REGISTER dpu_int_reg_36 */
#define ONL0_SECU_NML_FRM_TIMING_VSYNC_INT_RAW_OFFSET                 0x90
#define ONL0_SECU_NML_FRM_TIMING_VSYNC_INT_RAW_SHIFT                  0
#define ONL0_SECU_NML_FRM_TIMING_VSYNC_INT_RAW_LENGTH                 1
#define ONL0_SECU_NML_FRM_TIMING_EOF_INT_RAW_SHIFT                    1
#define ONL0_SECU_NML_FRM_TIMING_EOF_INT_RAW_LENGTH                   1
#define ONL0_SECU_NML_FRM_TIMING_CFG_EOF_INT_RAW_SHIFT                2
#define ONL0_SECU_NML_FRM_TIMING_CFG_EOF_INT_RAW_LENGTH               2
#define ONL0_SECU_NML_FRM_TIMING_CFG_LINE_INT_RAW_SHIFT               4
#define ONL0_SECU_NML_FRM_TIMING_CFG_LINE_INT_RAW_LENGTH              1
#define ONL0_SECU_NML_CFG_RDY_CLR_INT_RAW_SHIFT                       5
#define ONL0_SECU_NML_CFG_RDY_CLR_INT_RAW_LENGTH                      1
#define ONL0_SECU_NML_WB_DONE_INT_RAW_SHIFT                           6
#define ONL0_SECU_NML_WB_DONE_INT_RAW_LENGTH                          2
#define ONL0_SECU_NML_FRM_TIMING_UNFLOW_INT_RAW_SHIFT                 8
#define ONL0_SECU_NML_FRM_TIMING_UNFLOW_INT_RAW_LENGTH                1
#define ONL0_SECU_NML_WB_OVFLOW_INT_RAW_SHIFT                         9
#define ONL0_SECU_NML_WB_OVFLOW_INT_RAW_LENGTH                        2
#define ONL0_SECU_NML_CMDLIST_CH_FRM_CFG_DONE_INT_RAW_SHIFT           11
#define ONL0_SECU_NML_CMDLIST_CH_FRM_CFG_DONE_INT_RAW_LENGTH          13
#define ONL0_SECU_ACAD_FRM_DONE_INT_RAW_SHIFT                         24
#define ONL0_SECU_ACAD_FRM_DONE_INT_RAW_LENGTH                        1
#define ONL0_SECU_DSCW_FRM_DONE_INT_RAW_SHIFT                         25
#define ONL0_SECU_DSCW_FRM_DONE_INT_RAW_LENGTH                        2
#define ONL0_SECU_DSCW_OVFLOW_INT_RAW_SHIFT                           27
#define ONL0_SECU_DSCW_OVFLOW_INT_RAW_LENGTH                          2

/* REGISTER dpu_int_reg_37 */
#define ONL0_SECU_CMDLIST_CH_ENTER_PEND_INT_RAW_OFFSET                0x94
#define ONL0_SECU_CMDLIST_CH_ENTER_PEND_INT_RAW_SHIFT                 0
#define ONL0_SECU_CMDLIST_CH_ENTER_PEND_INT_RAW_LENGTH                13
#define ONL0_SECU_ARB_DEC_PSLVERR_INT_RAW_SHIFT                       13
#define ONL0_SECU_ARB_DEC_PSLVERR_INT_RAW_LENGTH                      1
#define ONL0_SECU_TMG_SKIP_VIDEO_TE_INT_RAW_SHIFT                     14
#define ONL0_SECU_TMG_SKIP_VIDEO_TE_INT_RAW_LENGTH                    1
#define ONL0_SECU_TMG_PWDN_TRIG_INT_RAW_SHIFT                         15
#define ONL0_SECU_TMG_PWDN_TRIG_INT_RAW_LENGTH                        1
#define ONL0_SECU_TMG_ULPS_EXIT_REQ_INT_RAW_SHIFT                     16
#define ONL0_SECU_TMG_ULPS_EXIT_REQ_INT_RAW_LENGTH                    1
#define ONL0_SECU_TMG_ULPS_ENTRY_REQ_INT_RAW_SHIFT                    17
#define ONL0_SECU_TMG_ULPS_ENTRY_REQ_INT_RAW_LENGTH                   1
#define ONL0_SECU_TMG_PARTITION_PWUP_TRIG_INT_RAW_SHIFT               18
#define ONL0_SECU_TMG_PARTITION_PWUP_TRIG_INT_RAW_LENGTH              1
#define ONL0_SECU_TMG_HOF_INT_RAW_SHIFT                               19
#define ONL0_SECU_TMG_HOF_INT_RAW_LENGTH                              1
#define ONL0_SECU_TMG_VACT_START_INT_RAW_SHIFT                        20
#define ONL0_SECU_TMG_VACT_START_INT_RAW_LENGTH                       1
#define ONL0_SECU_DDR_DVFS_REQ_INT_RAW_SHIFT                          21
#define ONL0_SECU_DDR_DVFS_REQ_INT_RAW_LENGTH                         1
#define ONL0_SECU_DVFS_FAIL_INT_RAW_SHIFT                             22
#define ONL0_SECU_DVFS_FAIL_INT_RAW_LENGTH                            1

/* REGISTER dpu_int_reg_38 */
#define ONL0_SECU_DMA_DBG_INT_RAW_OFFSET                              0x98
#define ONL0_SECU_DMA_DBG_INT_RAW_SHIFT                               0
#define ONL0_SECU_DMA_DBG_INT_RAW_LENGTH                              16
#define ONL0_SECU_OUTCTL_DBG_INT_RAW_SHIFT                            16
#define ONL0_SECU_OUTCTL_DBG_INT_RAW_LENGTH                           1
#define ONL0_SECU_CTL_DBG_INT_RAW_SHIFT                               17
#define ONL0_SECU_CTL_DBG_INT_RAW_LENGTH                              1
#define ONL0_SECU_CMDLIST_DBG_INT_RAW_SHIFT                           18
#define ONL0_SECU_CMDLIST_DBG_INT_RAW_LENGTH                          1
#define ONL0_SECU_DSC_P0_DBG_INT_RAW_SHIFT                            19
#define ONL0_SECU_DSC_P0_DBG_INT_RAW_LENGTH                           1
#define ONL0_SECU_DSC_P1_DBG_INT_RAW_SHIFT                            20
#define ONL0_SECU_DSC_P1_DBG_INT_RAW_LENGTH                           1
#define ONL0_SECU_DSCR_DBG_INT_RAW_SHIFT                              21
#define ONL0_SECU_DSCR_DBG_INT_RAW_LENGTH                             1

/* REGISTER dpu_int_reg_39 */
#define ONL0_SECU_ARB_DEC_PADDR_OFFSET                                0x9c

#endif
