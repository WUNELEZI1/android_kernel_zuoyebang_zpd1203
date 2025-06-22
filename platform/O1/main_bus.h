// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef MAIN_BUS_REGIF_H
#define MAIN_BUS_REGIF_H

#define MAIN_BUS_ERR_MAIN_0_ID_COREID                                        0x0000
#define MAIN_BUS_ERR_MAIN_0_ID_COREID_CORETYPEID_SHIFT                                            0
#define MAIN_BUS_ERR_MAIN_0_ID_COREID_CORETYPEID_MASK                                             0x000000ff
#define MAIN_BUS_ERR_MAIN_0_ID_COREID_CORECHECKSUM_SHIFT                                          8
#define MAIN_BUS_ERR_MAIN_0_ID_COREID_CORECHECKSUM_MASK                                           0xffffff00
#define MAIN_BUS_ERR_MAIN_0_ID_REVISIONID                                    0x0004
#define MAIN_BUS_ERR_MAIN_0_ID_REVISIONID_USERID_SHIFT                                            0
#define MAIN_BUS_ERR_MAIN_0_ID_REVISIONID_USERID_MASK                                             0x000000ff
#define MAIN_BUS_ERR_MAIN_0_ID_REVISIONID_FLEXNOCID_SHIFT                                         8
#define MAIN_BUS_ERR_MAIN_0_ID_REVISIONID_FLEXNOCID_MASK                                          0xffffff00
#define MAIN_BUS_ERR_MAIN_0_FAULTEN                                          0x0008
#define MAIN_BUS_ERR_MAIN_0_FAULTEN_FAULTEN_SHIFT                                                 0
#define MAIN_BUS_ERR_MAIN_0_FAULTEN_FAULTEN_MASK                                                  0x00000001
#define MAIN_BUS_ERR_MAIN_0_ERRVLD                                           0x000c
#define MAIN_BUS_ERR_MAIN_0_ERRVLD_ERRVLD_SHIFT                                                   0
#define MAIN_BUS_ERR_MAIN_0_ERRVLD_ERRVLD_MASK                                                    0x00000001
#define MAIN_BUS_ERR_MAIN_0_ERRCLR                                           0x0010
#define MAIN_BUS_ERR_MAIN_0_ERRCLR_ERRCLR_SHIFT                                                   0
#define MAIN_BUS_ERR_MAIN_0_ERRCLR_ERRCLR_MASK                                                    0x00000001
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0                                          0x0014
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0_LOCK_SHIFT                                                    0
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0_LOCK_MASK                                                     0x00000001
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0_OPC_SHIFT                                                     1
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0_OPC_MASK                                                      0x0000001e
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0_ERRCODE_SHIFT                                                 8
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0_ERRCODE_MASK                                                  0x00000700
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0_LEN1_SHIFT                                                    16
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0_LEN1_MASK                                                     0x007f0000
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0_FORMAT_SHIFT                                                  31
#define MAIN_BUS_ERR_MAIN_0_ERRLOG0_FORMAT_MASK                                                   0x80000000
#define MAIN_BUS_ERR_MAIN_0_ERRLOG1                                          0x0018
#define MAIN_BUS_ERR_MAIN_0_ERRLOG1_ERRLOG1_SHIFT                                                 0
#define MAIN_BUS_ERR_MAIN_0_ERRLOG1_ERRLOG1_MASK                                                  0x01ffffff
#define MAIN_BUS_ERR_MAIN_0_ERRLOG3                                          0x0020
#define MAIN_BUS_ERR_MAIN_0_ERRLOG4                                          0x0024
#define MAIN_BUS_ERR_MAIN_0_ERRLOG4_ERRLOG4_SHIFT                                                 0
#define MAIN_BUS_ERR_MAIN_0_ERRLOG4_ERRLOG4_MASK                                                  0x0000007f
#define MAIN_BUS_ERR_MAIN_0_ERRLOG5                                          0x0028
#define MAIN_BUS_ERR_MAIN_0_ERRLOG6                                          0x002c
#define MAIN_BUS_ERR_MAIN_0_ERRLOG6_ERRLOG6_SHIFT                                                 0
#define MAIN_BUS_ERR_MAIN_0_ERRLOG6_ERRLOG6_MASK                                                  0x00003fff
#define MAIN_BUS_ERR_MAIN_0_ERRLOG7                                          0x0030
#define MAIN_BUS_ERR_MAIN_0_ERRLOG7_ERRLOG7_SHIFT                                                 0
#define MAIN_BUS_ERR_MAIN_0_ERRLOG7_ERRLOG7_MASK                                                  0x00000001
#define MAIN_BUS_ERR_MAIN_0_STALLEN                                          0x004c
#define MAIN_BUS_ERR_MAIN_0_STALLEN_STALLEN_SHIFT                                                 0
#define MAIN_BUS_ERR_MAIN_0_STALLEN_STALLEN_MASK                                                  0x00000001
#define MAIN_BUS_ERR_CPU_0_ID_COREID                                         0x0080
#define MAIN_BUS_ERR_CPU_0_ID_COREID_CORETYPEID_SHIFT                                             0
#define MAIN_BUS_ERR_CPU_0_ID_COREID_CORETYPEID_MASK                                              0x000000ff
#define MAIN_BUS_ERR_CPU_0_ID_COREID_CORECHECKSUM_SHIFT                                           8
#define MAIN_BUS_ERR_CPU_0_ID_COREID_CORECHECKSUM_MASK                                            0xffffff00
#define MAIN_BUS_ERR_CPU_0_ID_REVISIONID                                     0x0084
#define MAIN_BUS_ERR_CPU_0_ID_REVISIONID_USERID_SHIFT                                             0
#define MAIN_BUS_ERR_CPU_0_ID_REVISIONID_USERID_MASK                                              0x000000ff
#define MAIN_BUS_ERR_CPU_0_ID_REVISIONID_FLEXNOCID_SHIFT                                          8
#define MAIN_BUS_ERR_CPU_0_ID_REVISIONID_FLEXNOCID_MASK                                           0xffffff00
#define MAIN_BUS_ERR_CPU_0_FAULTEN                                           0x0088
#define MAIN_BUS_ERR_CPU_0_FAULTEN_FAULTEN_SHIFT                                                  0
#define MAIN_BUS_ERR_CPU_0_FAULTEN_FAULTEN_MASK                                                   0x00000001
#define MAIN_BUS_ERR_CPU_0_ERRVLD                                            0x008c
#define MAIN_BUS_ERR_CPU_0_ERRVLD_ERRVLD_SHIFT                                                    0
#define MAIN_BUS_ERR_CPU_0_ERRVLD_ERRVLD_MASK                                                     0x00000001
#define MAIN_BUS_ERR_CPU_0_ERRCLR                                            0x0090
#define MAIN_BUS_ERR_CPU_0_ERRCLR_ERRCLR_SHIFT                                                    0
#define MAIN_BUS_ERR_CPU_0_ERRCLR_ERRCLR_MASK                                                     0x00000001
#define MAIN_BUS_ERR_CPU_0_ERRLOG0                                           0x0094
#define MAIN_BUS_ERR_CPU_0_ERRLOG0_LOCK_SHIFT                                                     0
#define MAIN_BUS_ERR_CPU_0_ERRLOG0_LOCK_MASK                                                      0x00000001
#define MAIN_BUS_ERR_CPU_0_ERRLOG0_OPC_SHIFT                                                      1
#define MAIN_BUS_ERR_CPU_0_ERRLOG0_OPC_MASK                                                       0x0000001e
#define MAIN_BUS_ERR_CPU_0_ERRLOG0_ERRCODE_SHIFT                                                  8
#define MAIN_BUS_ERR_CPU_0_ERRLOG0_ERRCODE_MASK                                                   0x00000700
#define MAIN_BUS_ERR_CPU_0_ERRLOG0_LEN1_SHIFT                                                     16
#define MAIN_BUS_ERR_CPU_0_ERRLOG0_LEN1_MASK                                                      0x007f0000
#define MAIN_BUS_ERR_CPU_0_ERRLOG0_FORMAT_SHIFT                                                   31
#define MAIN_BUS_ERR_CPU_0_ERRLOG0_FORMAT_MASK                                                    0x80000000
#define MAIN_BUS_ERR_CPU_0_ERRLOG1                                           0x0098
#define MAIN_BUS_ERR_CPU_0_ERRLOG1_ERRLOG1_SHIFT                                                  0
#define MAIN_BUS_ERR_CPU_0_ERRLOG1_ERRLOG1_MASK                                                   0x01ffffff
#define MAIN_BUS_ERR_CPU_0_ERRLOG3                                           0x00a0
#define MAIN_BUS_ERR_CPU_0_ERRLOG4                                           0x00a4
#define MAIN_BUS_ERR_CPU_0_ERRLOG4_ERRLOG4_SHIFT                                                  0
#define MAIN_BUS_ERR_CPU_0_ERRLOG4_ERRLOG4_MASK                                                   0x0000007f
#define MAIN_BUS_ERR_CPU_0_ERRLOG5                                           0x00a8
#define MAIN_BUS_ERR_CPU_0_ERRLOG6                                           0x00ac
#define MAIN_BUS_ERR_CPU_0_ERRLOG6_ERRLOG6_SHIFT                                                  0
#define MAIN_BUS_ERR_CPU_0_ERRLOG6_ERRLOG6_MASK                                                   0x00003fff
#define MAIN_BUS_ERR_CPU_0_ERRLOG7                                           0x00b0
#define MAIN_BUS_ERR_CPU_0_ERRLOG7_ERRLOG7_SHIFT                                                  0
#define MAIN_BUS_ERR_CPU_0_ERRLOG7_ERRLOG7_MASK                                                   0x00000001
#define MAIN_BUS_ERR_CPU_0_STALLEN                                           0x00cc
#define MAIN_BUS_ERR_CPU_0_STALLEN_STALLEN_SHIFT                                                  0
#define MAIN_BUS_ERR_CPU_0_STALLEN_STALLEN_MASK                                                   0x00000001
#define MAIN_BUS_ERR_GPU_0_ID_COREID                                         0x0100
#define MAIN_BUS_ERR_GPU_0_ID_COREID_CORETYPEID_SHIFT                                             0
#define MAIN_BUS_ERR_GPU_0_ID_COREID_CORETYPEID_MASK                                              0x000000ff
#define MAIN_BUS_ERR_GPU_0_ID_COREID_CORECHECKSUM_SHIFT                                           8
#define MAIN_BUS_ERR_GPU_0_ID_COREID_CORECHECKSUM_MASK                                            0xffffff00
#define MAIN_BUS_ERR_GPU_0_ID_REVISIONID                                     0x0104
#define MAIN_BUS_ERR_GPU_0_ID_REVISIONID_USERID_SHIFT                                             0
#define MAIN_BUS_ERR_GPU_0_ID_REVISIONID_USERID_MASK                                              0x000000ff
#define MAIN_BUS_ERR_GPU_0_ID_REVISIONID_FLEXNOCID_SHIFT                                          8
#define MAIN_BUS_ERR_GPU_0_ID_REVISIONID_FLEXNOCID_MASK                                           0xffffff00
#define MAIN_BUS_ERR_GPU_0_FAULTEN                                           0x0108
#define MAIN_BUS_ERR_GPU_0_FAULTEN_FAULTEN_SHIFT                                                  0
#define MAIN_BUS_ERR_GPU_0_FAULTEN_FAULTEN_MASK                                                   0x00000001
#define MAIN_BUS_ERR_GPU_0_ERRVLD                                            0x010c
#define MAIN_BUS_ERR_GPU_0_ERRVLD_ERRVLD_SHIFT                                                    0
#define MAIN_BUS_ERR_GPU_0_ERRVLD_ERRVLD_MASK                                                     0x00000001
#define MAIN_BUS_ERR_GPU_0_ERRCLR                                            0x0110
#define MAIN_BUS_ERR_GPU_0_ERRCLR_ERRCLR_SHIFT                                                    0
#define MAIN_BUS_ERR_GPU_0_ERRCLR_ERRCLR_MASK                                                     0x00000001
#define MAIN_BUS_ERR_GPU_0_ERRLOG0                                           0x0114
#define MAIN_BUS_ERR_GPU_0_ERRLOG0_LOCK_SHIFT                                                     0
#define MAIN_BUS_ERR_GPU_0_ERRLOG0_LOCK_MASK                                                      0x00000001
#define MAIN_BUS_ERR_GPU_0_ERRLOG0_OPC_SHIFT                                                      1
#define MAIN_BUS_ERR_GPU_0_ERRLOG0_OPC_MASK                                                       0x0000001e
#define MAIN_BUS_ERR_GPU_0_ERRLOG0_ERRCODE_SHIFT                                                  8
#define MAIN_BUS_ERR_GPU_0_ERRLOG0_ERRCODE_MASK                                                   0x00000700
#define MAIN_BUS_ERR_GPU_0_ERRLOG0_LEN1_SHIFT                                                     16
#define MAIN_BUS_ERR_GPU_0_ERRLOG0_LEN1_MASK                                                      0x007f0000
#define MAIN_BUS_ERR_GPU_0_ERRLOG0_FORMAT_SHIFT                                                   31
#define MAIN_BUS_ERR_GPU_0_ERRLOG0_FORMAT_MASK                                                    0x80000000
#define MAIN_BUS_ERR_GPU_0_ERRLOG1                                           0x0118
#define MAIN_BUS_ERR_GPU_0_ERRLOG1_ERRLOG1_SHIFT                                                  0
#define MAIN_BUS_ERR_GPU_0_ERRLOG1_ERRLOG1_MASK                                                   0x01ffffff
#define MAIN_BUS_ERR_GPU_0_ERRLOG3                                           0x0120
#define MAIN_BUS_ERR_GPU_0_ERRLOG4                                           0x0124
#define MAIN_BUS_ERR_GPU_0_ERRLOG4_ERRLOG4_SHIFT                                                  0
#define MAIN_BUS_ERR_GPU_0_ERRLOG4_ERRLOG4_MASK                                                   0x0000007f
#define MAIN_BUS_ERR_GPU_0_ERRLOG5                                           0x0128
#define MAIN_BUS_ERR_GPU_0_ERRLOG6                                           0x012c
#define MAIN_BUS_ERR_GPU_0_ERRLOG6_ERRLOG6_SHIFT                                                  0
#define MAIN_BUS_ERR_GPU_0_ERRLOG6_ERRLOG6_MASK                                                   0x00003fff
#define MAIN_BUS_ERR_GPU_0_ERRLOG7                                           0x0130
#define MAIN_BUS_ERR_GPU_0_ERRLOG7_ERRLOG7_SHIFT                                                  0
#define MAIN_BUS_ERR_GPU_0_ERRLOG7_ERRLOG7_MASK                                                   0x00000001
#define MAIN_BUS_ERR_GPU_0_STALLEN                                           0x014c
#define MAIN_BUS_ERR_GPU_0_STALLEN_STALLEN_SHIFT                                                  0
#define MAIN_BUS_ERR_GPU_0_STALLEN_STALLEN_MASK                                                   0x00000001
#define MAIN_BUS_CPU2MAIN_P0_RQOS_ID_COREID                                  0x0180
#define MAIN_BUS_CPU2MAIN_P0_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P0_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P0_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_CPU2MAIN_P0_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_CPU2MAIN_P0_RQOS_ID_REVISIONID                              0x0184
#define MAIN_BUS_CPU2MAIN_P0_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P0_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P0_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_CPU2MAIN_P0_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_CPU2MAIN_P0_RQOS_PRIORITY                                   0x0188
#define MAIN_BUS_CPU2MAIN_P0_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_CPU2MAIN_P0_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_CPU2MAIN_P0_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_CPU2MAIN_P0_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_CPU2MAIN_P0_RQOS_MODE                                       0x018c
#define MAIN_BUS_CPU2MAIN_P0_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_CPU2MAIN_P0_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_CPU2MAIN_P0_RQOS_BANDWIDTH                                  0x0190
#define MAIN_BUS_CPU2MAIN_P0_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_CPU2MAIN_P0_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_CPU2MAIN_P0_RQOS_SATURATION                                 0x0194
#define MAIN_BUS_CPU2MAIN_P0_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_CPU2MAIN_P0_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_CPU2MAIN_P0_RQOS_EXTCONTROL                                 0x0198
#define MAIN_BUS_CPU2MAIN_P0_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_CPU2MAIN_P0_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_CPU2MAIN_P0_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_CPU2MAIN_P0_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_CPU2MAIN_P0_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_CPU2MAIN_P0_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_CPU2MAIN_P0_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_CPU2MAIN_P0_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_CPU2MAIN_P0_WQOS_ID_COREID                                  0x0200
#define MAIN_BUS_CPU2MAIN_P0_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P0_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P0_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_CPU2MAIN_P0_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_CPU2MAIN_P0_WQOS_ID_REVISIONID                              0x0204
#define MAIN_BUS_CPU2MAIN_P0_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P0_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P0_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_CPU2MAIN_P0_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_CPU2MAIN_P0_WQOS_PRIORITY                                   0x0208
#define MAIN_BUS_CPU2MAIN_P0_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_CPU2MAIN_P0_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_CPU2MAIN_P0_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_CPU2MAIN_P0_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_CPU2MAIN_P0_WQOS_MODE                                       0x020c
#define MAIN_BUS_CPU2MAIN_P0_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_CPU2MAIN_P0_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_CPU2MAIN_P0_WQOS_BANDWIDTH                                  0x0210
#define MAIN_BUS_CPU2MAIN_P0_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_CPU2MAIN_P0_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_CPU2MAIN_P0_WQOS_SATURATION                                 0x0214
#define MAIN_BUS_CPU2MAIN_P0_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_CPU2MAIN_P0_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_CPU2MAIN_P0_WQOS_EXTCONTROL                                 0x0218
#define MAIN_BUS_CPU2MAIN_P0_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_CPU2MAIN_P0_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_CPU2MAIN_P0_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_CPU2MAIN_P0_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_CPU2MAIN_P0_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_CPU2MAIN_P0_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_CPU2MAIN_P0_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_CPU2MAIN_P0_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_CPU2MAIN_P1_RQOS_ID_COREID                                  0x0280
#define MAIN_BUS_CPU2MAIN_P1_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P1_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P1_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_CPU2MAIN_P1_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_CPU2MAIN_P1_RQOS_ID_REVISIONID                              0x0284
#define MAIN_BUS_CPU2MAIN_P1_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P1_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P1_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_CPU2MAIN_P1_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_CPU2MAIN_P1_RQOS_PRIORITY                                   0x0288
#define MAIN_BUS_CPU2MAIN_P1_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_CPU2MAIN_P1_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_CPU2MAIN_P1_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_CPU2MAIN_P1_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_CPU2MAIN_P1_RQOS_MODE                                       0x028c
#define MAIN_BUS_CPU2MAIN_P1_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_CPU2MAIN_P1_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_CPU2MAIN_P1_RQOS_BANDWIDTH                                  0x0290
#define MAIN_BUS_CPU2MAIN_P1_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_CPU2MAIN_P1_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_CPU2MAIN_P1_RQOS_SATURATION                                 0x0294
#define MAIN_BUS_CPU2MAIN_P1_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_CPU2MAIN_P1_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_CPU2MAIN_P1_RQOS_EXTCONTROL                                 0x0298
#define MAIN_BUS_CPU2MAIN_P1_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_CPU2MAIN_P1_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_CPU2MAIN_P1_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_CPU2MAIN_P1_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_CPU2MAIN_P1_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_CPU2MAIN_P1_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_CPU2MAIN_P1_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_CPU2MAIN_P1_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_CPU2MAIN_P1_WQOS_ID_COREID                                  0x0300
#define MAIN_BUS_CPU2MAIN_P1_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P1_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P1_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_CPU2MAIN_P1_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_CPU2MAIN_P1_WQOS_ID_REVISIONID                              0x0304
#define MAIN_BUS_CPU2MAIN_P1_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P1_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P1_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_CPU2MAIN_P1_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_CPU2MAIN_P1_WQOS_PRIORITY                                   0x0308
#define MAIN_BUS_CPU2MAIN_P1_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_CPU2MAIN_P1_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_CPU2MAIN_P1_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_CPU2MAIN_P1_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_CPU2MAIN_P1_WQOS_MODE                                       0x030c
#define MAIN_BUS_CPU2MAIN_P1_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_CPU2MAIN_P1_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_CPU2MAIN_P1_WQOS_BANDWIDTH                                  0x0310
#define MAIN_BUS_CPU2MAIN_P1_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_CPU2MAIN_P1_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_CPU2MAIN_P1_WQOS_SATURATION                                 0x0314
#define MAIN_BUS_CPU2MAIN_P1_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_CPU2MAIN_P1_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_CPU2MAIN_P1_WQOS_EXTCONTROL                                 0x0318
#define MAIN_BUS_CPU2MAIN_P1_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_CPU2MAIN_P1_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_CPU2MAIN_P1_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_CPU2MAIN_P1_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_CPU2MAIN_P1_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_CPU2MAIN_P1_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_CPU2MAIN_P1_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_CPU2MAIN_P1_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_CPU2MAIN_P2_RQOS_ID_COREID                                  0x0380
#define MAIN_BUS_CPU2MAIN_P2_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P2_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P2_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_CPU2MAIN_P2_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_CPU2MAIN_P2_RQOS_ID_REVISIONID                              0x0384
#define MAIN_BUS_CPU2MAIN_P2_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P2_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P2_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_CPU2MAIN_P2_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_CPU2MAIN_P2_RQOS_PRIORITY                                   0x0388
#define MAIN_BUS_CPU2MAIN_P2_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_CPU2MAIN_P2_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_CPU2MAIN_P2_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_CPU2MAIN_P2_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_CPU2MAIN_P2_RQOS_MODE                                       0x038c
#define MAIN_BUS_CPU2MAIN_P2_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_CPU2MAIN_P2_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_CPU2MAIN_P2_RQOS_BANDWIDTH                                  0x0390
#define MAIN_BUS_CPU2MAIN_P2_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_CPU2MAIN_P2_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_CPU2MAIN_P2_RQOS_SATURATION                                 0x0394
#define MAIN_BUS_CPU2MAIN_P2_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_CPU2MAIN_P2_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_CPU2MAIN_P2_RQOS_EXTCONTROL                                 0x0398
#define MAIN_BUS_CPU2MAIN_P2_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_CPU2MAIN_P2_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_CPU2MAIN_P2_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_CPU2MAIN_P2_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_CPU2MAIN_P2_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_CPU2MAIN_P2_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_CPU2MAIN_P2_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_CPU2MAIN_P2_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_CPU2MAIN_P2_WQOS_ID_COREID                                  0x0400
#define MAIN_BUS_CPU2MAIN_P2_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P2_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P2_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_CPU2MAIN_P2_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_CPU2MAIN_P2_WQOS_ID_REVISIONID                              0x0404
#define MAIN_BUS_CPU2MAIN_P2_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P2_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P2_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_CPU2MAIN_P2_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_CPU2MAIN_P2_WQOS_PRIORITY                                   0x0408
#define MAIN_BUS_CPU2MAIN_P2_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_CPU2MAIN_P2_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_CPU2MAIN_P2_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_CPU2MAIN_P2_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_CPU2MAIN_P2_WQOS_MODE                                       0x040c
#define MAIN_BUS_CPU2MAIN_P2_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_CPU2MAIN_P2_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_CPU2MAIN_P2_WQOS_BANDWIDTH                                  0x0410
#define MAIN_BUS_CPU2MAIN_P2_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_CPU2MAIN_P2_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_CPU2MAIN_P2_WQOS_SATURATION                                 0x0414
#define MAIN_BUS_CPU2MAIN_P2_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_CPU2MAIN_P2_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_CPU2MAIN_P2_WQOS_EXTCONTROL                                 0x0418
#define MAIN_BUS_CPU2MAIN_P2_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_CPU2MAIN_P2_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_CPU2MAIN_P2_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_CPU2MAIN_P2_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_CPU2MAIN_P2_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_CPU2MAIN_P2_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_CPU2MAIN_P2_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_CPU2MAIN_P2_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_CPU2MAIN_P3_RQOS_ID_COREID                                  0x0480
#define MAIN_BUS_CPU2MAIN_P3_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P3_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P3_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_CPU2MAIN_P3_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_CPU2MAIN_P3_RQOS_ID_REVISIONID                              0x0484
#define MAIN_BUS_CPU2MAIN_P3_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P3_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P3_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_CPU2MAIN_P3_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_CPU2MAIN_P3_RQOS_PRIORITY                                   0x0488
#define MAIN_BUS_CPU2MAIN_P3_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_CPU2MAIN_P3_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_CPU2MAIN_P3_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_CPU2MAIN_P3_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_CPU2MAIN_P3_RQOS_MODE                                       0x048c
#define MAIN_BUS_CPU2MAIN_P3_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_CPU2MAIN_P3_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_CPU2MAIN_P3_RQOS_BANDWIDTH                                  0x0490
#define MAIN_BUS_CPU2MAIN_P3_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_CPU2MAIN_P3_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_CPU2MAIN_P3_RQOS_SATURATION                                 0x0494
#define MAIN_BUS_CPU2MAIN_P3_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_CPU2MAIN_P3_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_CPU2MAIN_P3_RQOS_EXTCONTROL                                 0x0498
#define MAIN_BUS_CPU2MAIN_P3_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_CPU2MAIN_P3_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_CPU2MAIN_P3_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_CPU2MAIN_P3_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_CPU2MAIN_P3_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_CPU2MAIN_P3_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_CPU2MAIN_P3_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_CPU2MAIN_P3_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_CPU2MAIN_P3_WQOS_ID_COREID                                  0x0500
#define MAIN_BUS_CPU2MAIN_P3_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P3_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P3_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_CPU2MAIN_P3_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_CPU2MAIN_P3_WQOS_ID_REVISIONID                              0x0504
#define MAIN_BUS_CPU2MAIN_P3_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_CPU2MAIN_P3_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_CPU2MAIN_P3_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_CPU2MAIN_P3_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_CPU2MAIN_P3_WQOS_PRIORITY                                   0x0508
#define MAIN_BUS_CPU2MAIN_P3_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_CPU2MAIN_P3_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_CPU2MAIN_P3_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_CPU2MAIN_P3_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_CPU2MAIN_P3_WQOS_MODE                                       0x050c
#define MAIN_BUS_CPU2MAIN_P3_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_CPU2MAIN_P3_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_CPU2MAIN_P3_WQOS_BANDWIDTH                                  0x0510
#define MAIN_BUS_CPU2MAIN_P3_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_CPU2MAIN_P3_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_CPU2MAIN_P3_WQOS_SATURATION                                 0x0514
#define MAIN_BUS_CPU2MAIN_P3_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_CPU2MAIN_P3_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_CPU2MAIN_P3_WQOS_EXTCONTROL                                 0x0518
#define MAIN_BUS_CPU2MAIN_P3_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_CPU2MAIN_P3_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_CPU2MAIN_P3_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_CPU2MAIN_P3_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_CPU2MAIN_P3_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_CPU2MAIN_P3_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_CPU2MAIN_P3_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_CPU2MAIN_P3_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_GPU2MAIN_P0_RQOS_ID_COREID                                  0x0580
#define MAIN_BUS_GPU2MAIN_P0_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P0_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P0_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_GPU2MAIN_P0_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_GPU2MAIN_P0_RQOS_ID_REVISIONID                              0x0584
#define MAIN_BUS_GPU2MAIN_P0_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P0_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P0_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_GPU2MAIN_P0_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_GPU2MAIN_P0_RQOS_PRIORITY                                   0x0588
#define MAIN_BUS_GPU2MAIN_P0_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_GPU2MAIN_P0_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_GPU2MAIN_P0_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_GPU2MAIN_P0_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_GPU2MAIN_P0_RQOS_MODE                                       0x058c
#define MAIN_BUS_GPU2MAIN_P0_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_GPU2MAIN_P0_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_GPU2MAIN_P0_RQOS_BANDWIDTH                                  0x0590
#define MAIN_BUS_GPU2MAIN_P0_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_GPU2MAIN_P0_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_GPU2MAIN_P0_RQOS_SATURATION                                 0x0594
#define MAIN_BUS_GPU2MAIN_P0_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_GPU2MAIN_P0_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_GPU2MAIN_P0_RQOS_EXTCONTROL                                 0x0598
#define MAIN_BUS_GPU2MAIN_P0_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_GPU2MAIN_P0_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_GPU2MAIN_P0_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_GPU2MAIN_P0_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_GPU2MAIN_P0_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_GPU2MAIN_P0_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_GPU2MAIN_P0_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_GPU2MAIN_P0_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_GPU2MAIN_P0_WQOS_ID_COREID                                  0x0600
#define MAIN_BUS_GPU2MAIN_P0_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P0_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P0_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_GPU2MAIN_P0_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_GPU2MAIN_P0_WQOS_ID_REVISIONID                              0x0604
#define MAIN_BUS_GPU2MAIN_P0_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P0_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P0_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_GPU2MAIN_P0_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_GPU2MAIN_P0_WQOS_PRIORITY                                   0x0608
#define MAIN_BUS_GPU2MAIN_P0_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_GPU2MAIN_P0_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_GPU2MAIN_P0_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_GPU2MAIN_P0_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_GPU2MAIN_P0_WQOS_MODE                                       0x060c
#define MAIN_BUS_GPU2MAIN_P0_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_GPU2MAIN_P0_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_GPU2MAIN_P0_WQOS_BANDWIDTH                                  0x0610
#define MAIN_BUS_GPU2MAIN_P0_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_GPU2MAIN_P0_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_GPU2MAIN_P0_WQOS_SATURATION                                 0x0614
#define MAIN_BUS_GPU2MAIN_P0_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_GPU2MAIN_P0_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_GPU2MAIN_P0_WQOS_EXTCONTROL                                 0x0618
#define MAIN_BUS_GPU2MAIN_P0_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_GPU2MAIN_P0_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_GPU2MAIN_P0_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_GPU2MAIN_P0_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_GPU2MAIN_P0_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_GPU2MAIN_P0_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_GPU2MAIN_P0_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_GPU2MAIN_P0_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_GPU2MAIN_P1_RQOS_ID_COREID                                  0x0680
#define MAIN_BUS_GPU2MAIN_P1_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P1_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P1_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_GPU2MAIN_P1_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_GPU2MAIN_P1_RQOS_ID_REVISIONID                              0x0684
#define MAIN_BUS_GPU2MAIN_P1_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P1_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P1_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_GPU2MAIN_P1_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_GPU2MAIN_P1_RQOS_PRIORITY                                   0x0688
#define MAIN_BUS_GPU2MAIN_P1_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_GPU2MAIN_P1_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_GPU2MAIN_P1_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_GPU2MAIN_P1_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_GPU2MAIN_P1_RQOS_MODE                                       0x068c
#define MAIN_BUS_GPU2MAIN_P1_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_GPU2MAIN_P1_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_GPU2MAIN_P1_RQOS_BANDWIDTH                                  0x0690
#define MAIN_BUS_GPU2MAIN_P1_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_GPU2MAIN_P1_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_GPU2MAIN_P1_RQOS_SATURATION                                 0x0694
#define MAIN_BUS_GPU2MAIN_P1_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_GPU2MAIN_P1_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_GPU2MAIN_P1_RQOS_EXTCONTROL                                 0x0698
#define MAIN_BUS_GPU2MAIN_P1_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_GPU2MAIN_P1_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_GPU2MAIN_P1_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_GPU2MAIN_P1_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_GPU2MAIN_P1_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_GPU2MAIN_P1_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_GPU2MAIN_P1_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_GPU2MAIN_P1_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_GPU2MAIN_P1_WQOS_ID_COREID                                  0x0700
#define MAIN_BUS_GPU2MAIN_P1_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P1_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P1_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_GPU2MAIN_P1_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_GPU2MAIN_P1_WQOS_ID_REVISIONID                              0x0704
#define MAIN_BUS_GPU2MAIN_P1_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P1_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P1_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_GPU2MAIN_P1_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_GPU2MAIN_P1_WQOS_PRIORITY                                   0x0708
#define MAIN_BUS_GPU2MAIN_P1_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_GPU2MAIN_P1_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_GPU2MAIN_P1_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_GPU2MAIN_P1_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_GPU2MAIN_P1_WQOS_MODE                                       0x070c
#define MAIN_BUS_GPU2MAIN_P1_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_GPU2MAIN_P1_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_GPU2MAIN_P1_WQOS_BANDWIDTH                                  0x0710
#define MAIN_BUS_GPU2MAIN_P1_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_GPU2MAIN_P1_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_GPU2MAIN_P1_WQOS_SATURATION                                 0x0714
#define MAIN_BUS_GPU2MAIN_P1_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_GPU2MAIN_P1_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_GPU2MAIN_P1_WQOS_EXTCONTROL                                 0x0718
#define MAIN_BUS_GPU2MAIN_P1_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_GPU2MAIN_P1_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_GPU2MAIN_P1_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_GPU2MAIN_P1_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_GPU2MAIN_P1_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_GPU2MAIN_P1_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_GPU2MAIN_P1_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_GPU2MAIN_P1_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_GPU2MAIN_P2_RQOS_ID_COREID                                  0x0780
#define MAIN_BUS_GPU2MAIN_P2_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P2_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P2_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_GPU2MAIN_P2_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_GPU2MAIN_P2_RQOS_ID_REVISIONID                              0x0784
#define MAIN_BUS_GPU2MAIN_P2_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P2_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P2_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_GPU2MAIN_P2_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_GPU2MAIN_P2_RQOS_PRIORITY                                   0x0788
#define MAIN_BUS_GPU2MAIN_P2_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_GPU2MAIN_P2_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_GPU2MAIN_P2_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_GPU2MAIN_P2_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_GPU2MAIN_P2_RQOS_MODE                                       0x078c
#define MAIN_BUS_GPU2MAIN_P2_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_GPU2MAIN_P2_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_GPU2MAIN_P2_RQOS_BANDWIDTH                                  0x0790
#define MAIN_BUS_GPU2MAIN_P2_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_GPU2MAIN_P2_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_GPU2MAIN_P2_RQOS_SATURATION                                 0x0794
#define MAIN_BUS_GPU2MAIN_P2_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_GPU2MAIN_P2_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_GPU2MAIN_P2_RQOS_EXTCONTROL                                 0x0798
#define MAIN_BUS_GPU2MAIN_P2_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_GPU2MAIN_P2_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_GPU2MAIN_P2_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_GPU2MAIN_P2_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_GPU2MAIN_P2_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_GPU2MAIN_P2_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_GPU2MAIN_P2_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_GPU2MAIN_P2_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_GPU2MAIN_P2_WQOS_ID_COREID                                  0x0800
#define MAIN_BUS_GPU2MAIN_P2_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P2_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P2_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_GPU2MAIN_P2_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_GPU2MAIN_P2_WQOS_ID_REVISIONID                              0x0804
#define MAIN_BUS_GPU2MAIN_P2_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P2_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P2_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_GPU2MAIN_P2_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_GPU2MAIN_P2_WQOS_PRIORITY                                   0x0808
#define MAIN_BUS_GPU2MAIN_P2_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_GPU2MAIN_P2_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_GPU2MAIN_P2_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_GPU2MAIN_P2_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_GPU2MAIN_P2_WQOS_MODE                                       0x080c
#define MAIN_BUS_GPU2MAIN_P2_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_GPU2MAIN_P2_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_GPU2MAIN_P2_WQOS_BANDWIDTH                                  0x0810
#define MAIN_BUS_GPU2MAIN_P2_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_GPU2MAIN_P2_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_GPU2MAIN_P2_WQOS_SATURATION                                 0x0814
#define MAIN_BUS_GPU2MAIN_P2_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_GPU2MAIN_P2_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_GPU2MAIN_P2_WQOS_EXTCONTROL                                 0x0818
#define MAIN_BUS_GPU2MAIN_P2_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_GPU2MAIN_P2_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_GPU2MAIN_P2_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_GPU2MAIN_P2_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_GPU2MAIN_P2_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_GPU2MAIN_P2_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_GPU2MAIN_P2_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_GPU2MAIN_P2_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_GPU2MAIN_P3_RQOS_ID_COREID                                  0x0880
#define MAIN_BUS_GPU2MAIN_P3_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P3_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P3_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_GPU2MAIN_P3_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_GPU2MAIN_P3_RQOS_ID_REVISIONID                              0x0884
#define MAIN_BUS_GPU2MAIN_P3_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P3_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P3_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_GPU2MAIN_P3_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_GPU2MAIN_P3_RQOS_PRIORITY                                   0x0888
#define MAIN_BUS_GPU2MAIN_P3_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_GPU2MAIN_P3_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_GPU2MAIN_P3_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_GPU2MAIN_P3_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_GPU2MAIN_P3_RQOS_MODE                                       0x088c
#define MAIN_BUS_GPU2MAIN_P3_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_GPU2MAIN_P3_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_GPU2MAIN_P3_RQOS_BANDWIDTH                                  0x0890
#define MAIN_BUS_GPU2MAIN_P3_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_GPU2MAIN_P3_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_GPU2MAIN_P3_RQOS_SATURATION                                 0x0894
#define MAIN_BUS_GPU2MAIN_P3_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_GPU2MAIN_P3_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_GPU2MAIN_P3_RQOS_EXTCONTROL                                 0x0898
#define MAIN_BUS_GPU2MAIN_P3_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_GPU2MAIN_P3_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_GPU2MAIN_P3_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_GPU2MAIN_P3_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_GPU2MAIN_P3_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_GPU2MAIN_P3_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_GPU2MAIN_P3_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_GPU2MAIN_P3_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_GPU2MAIN_P3_WQOS_ID_COREID                                  0x0900
#define MAIN_BUS_GPU2MAIN_P3_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P3_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P3_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_GPU2MAIN_P3_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_GPU2MAIN_P3_WQOS_ID_REVISIONID                              0x0904
#define MAIN_BUS_GPU2MAIN_P3_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_GPU2MAIN_P3_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_GPU2MAIN_P3_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_GPU2MAIN_P3_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_GPU2MAIN_P3_WQOS_PRIORITY                                   0x0908
#define MAIN_BUS_GPU2MAIN_P3_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_GPU2MAIN_P3_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_GPU2MAIN_P3_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_GPU2MAIN_P3_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_GPU2MAIN_P3_WQOS_MODE                                       0x090c
#define MAIN_BUS_GPU2MAIN_P3_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_GPU2MAIN_P3_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_GPU2MAIN_P3_WQOS_BANDWIDTH                                  0x0910
#define MAIN_BUS_GPU2MAIN_P3_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_GPU2MAIN_P3_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00003fff
#define MAIN_BUS_GPU2MAIN_P3_WQOS_SATURATION                                 0x0914
#define MAIN_BUS_GPU2MAIN_P3_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_GPU2MAIN_P3_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_GPU2MAIN_P3_WQOS_EXTCONTROL                                 0x0918
#define MAIN_BUS_GPU2MAIN_P3_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_GPU2MAIN_P3_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_GPU2MAIN_P3_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_GPU2MAIN_P3_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_GPU2MAIN_P3_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_GPU2MAIN_P3_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_GPU2MAIN_P3_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_GPU2MAIN_P3_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_M1_MAIN_P0_RQOS_ID_COREID                                   0x0980
#define MAIN_BUS_M1_MAIN_P0_RQOS_ID_COREID_CORETYPEID_SHIFT                                       0
#define MAIN_BUS_M1_MAIN_P0_RQOS_ID_COREID_CORETYPEID_MASK                                        0x000000ff
#define MAIN_BUS_M1_MAIN_P0_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                     8
#define MAIN_BUS_M1_MAIN_P0_RQOS_ID_COREID_CORECHECKSUM_MASK                                      0xffffff00
#define MAIN_BUS_M1_MAIN_P0_RQOS_ID_REVISIONID                               0x0984
#define MAIN_BUS_M1_MAIN_P0_RQOS_ID_REVISIONID_USERID_SHIFT                                       0
#define MAIN_BUS_M1_MAIN_P0_RQOS_ID_REVISIONID_USERID_MASK                                        0x000000ff
#define MAIN_BUS_M1_MAIN_P0_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                    8
#define MAIN_BUS_M1_MAIN_P0_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                     0xffffff00
#define MAIN_BUS_M1_MAIN_P0_RQOS_PRIORITY                                    0x0988
#define MAIN_BUS_M1_MAIN_P0_RQOS_PRIORITY_P0_SHIFT                                                0
#define MAIN_BUS_M1_MAIN_P0_RQOS_PRIORITY_P0_MASK                                                 0x00000007
#define MAIN_BUS_M1_MAIN_P0_RQOS_PRIORITY_P1_SHIFT                                                8
#define MAIN_BUS_M1_MAIN_P0_RQOS_PRIORITY_P1_MASK                                                 0x00000700
#define MAIN_BUS_M1_MAIN_P0_RQOS_MODE                                        0x098c
#define MAIN_BUS_M1_MAIN_P0_RQOS_MODE_MODE_SHIFT                                                  0
#define MAIN_BUS_M1_MAIN_P0_RQOS_MODE_MODE_MASK                                                   0x00000003
#define MAIN_BUS_M1_MAIN_P0_RQOS_BANDWIDTH                                   0x0990
#define MAIN_BUS_M1_MAIN_P0_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                        0
#define MAIN_BUS_M1_MAIN_P0_RQOS_BANDWIDTH_BANDWIDTH_MASK                                         0x00001fff
#define MAIN_BUS_M1_MAIN_P0_RQOS_SATURATION                                  0x0994
#define MAIN_BUS_M1_MAIN_P0_RQOS_SATURATION_SATURATION_SHIFT                                      0
#define MAIN_BUS_M1_MAIN_P0_RQOS_SATURATION_SATURATION_MASK                                       0x000003ff
#define MAIN_BUS_M1_MAIN_P0_RQOS_EXTCONTROL                                  0x0998
#define MAIN_BUS_M1_MAIN_P0_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                     0
#define MAIN_BUS_M1_MAIN_P0_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                      0x00000001
#define MAIN_BUS_M1_MAIN_P0_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                        1
#define MAIN_BUS_M1_MAIN_P0_RQOS_EXTCONTROL_EXTTHREN_MASK                                         0x00000002
#define MAIN_BUS_M1_MAIN_P0_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                        2
#define MAIN_BUS_M1_MAIN_P0_RQOS_EXTCONTROL_INTCLKEN_MASK                                         0x00000004
#define MAIN_BUS_M1_MAIN_P0_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                      3
#define MAIN_BUS_M1_MAIN_P0_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                       0x00000008
#define MAIN_BUS_M1_MAIN_P0_WQOS_ID_COREID                                   0x1000
#define MAIN_BUS_M1_MAIN_P0_WQOS_ID_COREID_CORETYPEID_SHIFT                                       0
#define MAIN_BUS_M1_MAIN_P0_WQOS_ID_COREID_CORETYPEID_MASK                                        0x000000ff
#define MAIN_BUS_M1_MAIN_P0_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                     8
#define MAIN_BUS_M1_MAIN_P0_WQOS_ID_COREID_CORECHECKSUM_MASK                                      0xffffff00
#define MAIN_BUS_M1_MAIN_P0_WQOS_ID_REVISIONID                               0x1004
#define MAIN_BUS_M1_MAIN_P0_WQOS_ID_REVISIONID_USERID_SHIFT                                       0
#define MAIN_BUS_M1_MAIN_P0_WQOS_ID_REVISIONID_USERID_MASK                                        0x000000ff
#define MAIN_BUS_M1_MAIN_P0_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                    8
#define MAIN_BUS_M1_MAIN_P0_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                     0xffffff00
#define MAIN_BUS_M1_MAIN_P0_WQOS_PRIORITY                                    0x1008
#define MAIN_BUS_M1_MAIN_P0_WQOS_PRIORITY_P0_SHIFT                                                0
#define MAIN_BUS_M1_MAIN_P0_WQOS_PRIORITY_P0_MASK                                                 0x00000007
#define MAIN_BUS_M1_MAIN_P0_WQOS_PRIORITY_P1_SHIFT                                                8
#define MAIN_BUS_M1_MAIN_P0_WQOS_PRIORITY_P1_MASK                                                 0x00000700
#define MAIN_BUS_M1_MAIN_P0_WQOS_MODE                                        0x100c
#define MAIN_BUS_M1_MAIN_P0_WQOS_MODE_MODE_SHIFT                                                  0
#define MAIN_BUS_M1_MAIN_P0_WQOS_MODE_MODE_MASK                                                   0x00000003
#define MAIN_BUS_M1_MAIN_P0_WQOS_BANDWIDTH                                   0x1010
#define MAIN_BUS_M1_MAIN_P0_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                        0
#define MAIN_BUS_M1_MAIN_P0_WQOS_BANDWIDTH_BANDWIDTH_MASK                                         0x00001fff
#define MAIN_BUS_M1_MAIN_P0_WQOS_SATURATION                                  0x1014
#define MAIN_BUS_M1_MAIN_P0_WQOS_SATURATION_SATURATION_SHIFT                                      0
#define MAIN_BUS_M1_MAIN_P0_WQOS_SATURATION_SATURATION_MASK                                       0x000003ff
#define MAIN_BUS_M1_MAIN_P0_WQOS_EXTCONTROL                                  0x1018
#define MAIN_BUS_M1_MAIN_P0_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                     0
#define MAIN_BUS_M1_MAIN_P0_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                      0x00000001
#define MAIN_BUS_M1_MAIN_P0_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                        1
#define MAIN_BUS_M1_MAIN_P0_WQOS_EXTCONTROL_EXTTHREN_MASK                                         0x00000002
#define MAIN_BUS_M1_MAIN_P0_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                        2
#define MAIN_BUS_M1_MAIN_P0_WQOS_EXTCONTROL_INTCLKEN_MASK                                         0x00000004
#define MAIN_BUS_M1_MAIN_P0_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                      3
#define MAIN_BUS_M1_MAIN_P0_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                       0x00000008
#define MAIN_BUS_M1_MAIN_P1_RQOS_ID_COREID                                   0x1080
#define MAIN_BUS_M1_MAIN_P1_RQOS_ID_COREID_CORETYPEID_SHIFT                                       0
#define MAIN_BUS_M1_MAIN_P1_RQOS_ID_COREID_CORETYPEID_MASK                                        0x000000ff
#define MAIN_BUS_M1_MAIN_P1_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                     8
#define MAIN_BUS_M1_MAIN_P1_RQOS_ID_COREID_CORECHECKSUM_MASK                                      0xffffff00
#define MAIN_BUS_M1_MAIN_P1_RQOS_ID_REVISIONID                               0x1084
#define MAIN_BUS_M1_MAIN_P1_RQOS_ID_REVISIONID_USERID_SHIFT                                       0
#define MAIN_BUS_M1_MAIN_P1_RQOS_ID_REVISIONID_USERID_MASK                                        0x000000ff
#define MAIN_BUS_M1_MAIN_P1_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                    8
#define MAIN_BUS_M1_MAIN_P1_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                     0xffffff00
#define MAIN_BUS_M1_MAIN_P1_RQOS_PRIORITY                                    0x1088
#define MAIN_BUS_M1_MAIN_P1_RQOS_PRIORITY_P0_SHIFT                                                0
#define MAIN_BUS_M1_MAIN_P1_RQOS_PRIORITY_P0_MASK                                                 0x00000007
#define MAIN_BUS_M1_MAIN_P1_RQOS_PRIORITY_P1_SHIFT                                                8
#define MAIN_BUS_M1_MAIN_P1_RQOS_PRIORITY_P1_MASK                                                 0x00000700
#define MAIN_BUS_M1_MAIN_P1_RQOS_MODE                                        0x108c
#define MAIN_BUS_M1_MAIN_P1_RQOS_MODE_MODE_SHIFT                                                  0
#define MAIN_BUS_M1_MAIN_P1_RQOS_MODE_MODE_MASK                                                   0x00000003
#define MAIN_BUS_M1_MAIN_P1_RQOS_BANDWIDTH                                   0x1090
#define MAIN_BUS_M1_MAIN_P1_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                        0
#define MAIN_BUS_M1_MAIN_P1_RQOS_BANDWIDTH_BANDWIDTH_MASK                                         0x00001fff
#define MAIN_BUS_M1_MAIN_P1_RQOS_SATURATION                                  0x1094
#define MAIN_BUS_M1_MAIN_P1_RQOS_SATURATION_SATURATION_SHIFT                                      0
#define MAIN_BUS_M1_MAIN_P1_RQOS_SATURATION_SATURATION_MASK                                       0x000003ff
#define MAIN_BUS_M1_MAIN_P1_RQOS_EXTCONTROL                                  0x1098
#define MAIN_BUS_M1_MAIN_P1_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                     0
#define MAIN_BUS_M1_MAIN_P1_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                      0x00000001
#define MAIN_BUS_M1_MAIN_P1_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                        1
#define MAIN_BUS_M1_MAIN_P1_RQOS_EXTCONTROL_EXTTHREN_MASK                                         0x00000002
#define MAIN_BUS_M1_MAIN_P1_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                        2
#define MAIN_BUS_M1_MAIN_P1_RQOS_EXTCONTROL_INTCLKEN_MASK                                         0x00000004
#define MAIN_BUS_M1_MAIN_P1_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                      3
#define MAIN_BUS_M1_MAIN_P1_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                       0x00000008
#define MAIN_BUS_M1_MAIN_P1_WQOS_ID_COREID                                   0x1100
#define MAIN_BUS_M1_MAIN_P1_WQOS_ID_COREID_CORETYPEID_SHIFT                                       0
#define MAIN_BUS_M1_MAIN_P1_WQOS_ID_COREID_CORETYPEID_MASK                                        0x000000ff
#define MAIN_BUS_M1_MAIN_P1_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                     8
#define MAIN_BUS_M1_MAIN_P1_WQOS_ID_COREID_CORECHECKSUM_MASK                                      0xffffff00
#define MAIN_BUS_M1_MAIN_P1_WQOS_ID_REVISIONID                               0x1104
#define MAIN_BUS_M1_MAIN_P1_WQOS_ID_REVISIONID_USERID_SHIFT                                       0
#define MAIN_BUS_M1_MAIN_P1_WQOS_ID_REVISIONID_USERID_MASK                                        0x000000ff
#define MAIN_BUS_M1_MAIN_P1_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                    8
#define MAIN_BUS_M1_MAIN_P1_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                     0xffffff00
#define MAIN_BUS_M1_MAIN_P1_WQOS_PRIORITY                                    0x1108
#define MAIN_BUS_M1_MAIN_P1_WQOS_PRIORITY_P0_SHIFT                                                0
#define MAIN_BUS_M1_MAIN_P1_WQOS_PRIORITY_P0_MASK                                                 0x00000007
#define MAIN_BUS_M1_MAIN_P1_WQOS_PRIORITY_P1_SHIFT                                                8
#define MAIN_BUS_M1_MAIN_P1_WQOS_PRIORITY_P1_MASK                                                 0x00000700
#define MAIN_BUS_M1_MAIN_P1_WQOS_MODE                                        0x110c
#define MAIN_BUS_M1_MAIN_P1_WQOS_MODE_MODE_SHIFT                                                  0
#define MAIN_BUS_M1_MAIN_P1_WQOS_MODE_MODE_MASK                                                   0x00000003
#define MAIN_BUS_M1_MAIN_P1_WQOS_BANDWIDTH                                   0x1110
#define MAIN_BUS_M1_MAIN_P1_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                        0
#define MAIN_BUS_M1_MAIN_P1_WQOS_BANDWIDTH_BANDWIDTH_MASK                                         0x00001fff
#define MAIN_BUS_M1_MAIN_P1_WQOS_SATURATION                                  0x1114
#define MAIN_BUS_M1_MAIN_P1_WQOS_SATURATION_SATURATION_SHIFT                                      0
#define MAIN_BUS_M1_MAIN_P1_WQOS_SATURATION_SATURATION_MASK                                       0x000003ff
#define MAIN_BUS_M1_MAIN_P1_WQOS_EXTCONTROL                                  0x1118
#define MAIN_BUS_M1_MAIN_P1_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                     0
#define MAIN_BUS_M1_MAIN_P1_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                      0x00000001
#define MAIN_BUS_M1_MAIN_P1_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                        1
#define MAIN_BUS_M1_MAIN_P1_WQOS_EXTCONTROL_EXTTHREN_MASK                                         0x00000002
#define MAIN_BUS_M1_MAIN_P1_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                        2
#define MAIN_BUS_M1_MAIN_P1_WQOS_EXTCONTROL_INTCLKEN_MASK                                         0x00000004
#define MAIN_BUS_M1_MAIN_P1_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                      3
#define MAIN_BUS_M1_MAIN_P1_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                       0x00000008
#define MAIN_BUS_M2_MAIN_P0_RQOS_ID_COREID                                   0x1180
#define MAIN_BUS_M2_MAIN_P0_RQOS_ID_COREID_CORETYPEID_SHIFT                                       0
#define MAIN_BUS_M2_MAIN_P0_RQOS_ID_COREID_CORETYPEID_MASK                                        0x000000ff
#define MAIN_BUS_M2_MAIN_P0_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                     8
#define MAIN_BUS_M2_MAIN_P0_RQOS_ID_COREID_CORECHECKSUM_MASK                                      0xffffff00
#define MAIN_BUS_M2_MAIN_P0_RQOS_ID_REVISIONID                               0x1184
#define MAIN_BUS_M2_MAIN_P0_RQOS_ID_REVISIONID_USERID_SHIFT                                       0
#define MAIN_BUS_M2_MAIN_P0_RQOS_ID_REVISIONID_USERID_MASK                                        0x000000ff
#define MAIN_BUS_M2_MAIN_P0_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                    8
#define MAIN_BUS_M2_MAIN_P0_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                     0xffffff00
#define MAIN_BUS_M2_MAIN_P0_RQOS_PRIORITY                                    0x1188
#define MAIN_BUS_M2_MAIN_P0_RQOS_PRIORITY_P0_SHIFT                                                0
#define MAIN_BUS_M2_MAIN_P0_RQOS_PRIORITY_P0_MASK                                                 0x00000007
#define MAIN_BUS_M2_MAIN_P0_RQOS_PRIORITY_P1_SHIFT                                                8
#define MAIN_BUS_M2_MAIN_P0_RQOS_PRIORITY_P1_MASK                                                 0x00000700
#define MAIN_BUS_M2_MAIN_P0_RQOS_MODE                                        0x118c
#define MAIN_BUS_M2_MAIN_P0_RQOS_MODE_MODE_SHIFT                                                  0
#define MAIN_BUS_M2_MAIN_P0_RQOS_MODE_MODE_MASK                                                   0x00000003
#define MAIN_BUS_M2_MAIN_P0_RQOS_BANDWIDTH                                   0x1190
#define MAIN_BUS_M2_MAIN_P0_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                        0
#define MAIN_BUS_M2_MAIN_P0_RQOS_BANDWIDTH_BANDWIDTH_MASK                                         0x00001fff
#define MAIN_BUS_M2_MAIN_P0_RQOS_SATURATION                                  0x1194
#define MAIN_BUS_M2_MAIN_P0_RQOS_SATURATION_SATURATION_SHIFT                                      0
#define MAIN_BUS_M2_MAIN_P0_RQOS_SATURATION_SATURATION_MASK                                       0x000003ff
#define MAIN_BUS_M2_MAIN_P0_RQOS_EXTCONTROL                                  0x1198
#define MAIN_BUS_M2_MAIN_P0_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                     0
#define MAIN_BUS_M2_MAIN_P0_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                      0x00000001
#define MAIN_BUS_M2_MAIN_P0_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                        1
#define MAIN_BUS_M2_MAIN_P0_RQOS_EXTCONTROL_EXTTHREN_MASK                                         0x00000002
#define MAIN_BUS_M2_MAIN_P0_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                        2
#define MAIN_BUS_M2_MAIN_P0_RQOS_EXTCONTROL_INTCLKEN_MASK                                         0x00000004
#define MAIN_BUS_M2_MAIN_P0_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                      3
#define MAIN_BUS_M2_MAIN_P0_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                       0x00000008
#define MAIN_BUS_M2_MAIN_P0_WQOS_ID_COREID                                   0x1200
#define MAIN_BUS_M2_MAIN_P0_WQOS_ID_COREID_CORETYPEID_SHIFT                                       0
#define MAIN_BUS_M2_MAIN_P0_WQOS_ID_COREID_CORETYPEID_MASK                                        0x000000ff
#define MAIN_BUS_M2_MAIN_P0_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                     8
#define MAIN_BUS_M2_MAIN_P0_WQOS_ID_COREID_CORECHECKSUM_MASK                                      0xffffff00
#define MAIN_BUS_M2_MAIN_P0_WQOS_ID_REVISIONID                               0x1204
#define MAIN_BUS_M2_MAIN_P0_WQOS_ID_REVISIONID_USERID_SHIFT                                       0
#define MAIN_BUS_M2_MAIN_P0_WQOS_ID_REVISIONID_USERID_MASK                                        0x000000ff
#define MAIN_BUS_M2_MAIN_P0_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                    8
#define MAIN_BUS_M2_MAIN_P0_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                     0xffffff00
#define MAIN_BUS_M2_MAIN_P0_WQOS_PRIORITY                                    0x1208
#define MAIN_BUS_M2_MAIN_P0_WQOS_PRIORITY_P0_SHIFT                                                0
#define MAIN_BUS_M2_MAIN_P0_WQOS_PRIORITY_P0_MASK                                                 0x00000007
#define MAIN_BUS_M2_MAIN_P0_WQOS_PRIORITY_P1_SHIFT                                                8
#define MAIN_BUS_M2_MAIN_P0_WQOS_PRIORITY_P1_MASK                                                 0x00000700
#define MAIN_BUS_M2_MAIN_P0_WQOS_MODE                                        0x120c
#define MAIN_BUS_M2_MAIN_P0_WQOS_MODE_MODE_SHIFT                                                  0
#define MAIN_BUS_M2_MAIN_P0_WQOS_MODE_MODE_MASK                                                   0x00000003
#define MAIN_BUS_M2_MAIN_P0_WQOS_BANDWIDTH                                   0x1210
#define MAIN_BUS_M2_MAIN_P0_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                        0
#define MAIN_BUS_M2_MAIN_P0_WQOS_BANDWIDTH_BANDWIDTH_MASK                                         0x00001fff
#define MAIN_BUS_M2_MAIN_P0_WQOS_SATURATION                                  0x1214
#define MAIN_BUS_M2_MAIN_P0_WQOS_SATURATION_SATURATION_SHIFT                                      0
#define MAIN_BUS_M2_MAIN_P0_WQOS_SATURATION_SATURATION_MASK                                       0x000003ff
#define MAIN_BUS_M2_MAIN_P0_WQOS_EXTCONTROL                                  0x1218
#define MAIN_BUS_M2_MAIN_P0_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                     0
#define MAIN_BUS_M2_MAIN_P0_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                      0x00000001
#define MAIN_BUS_M2_MAIN_P0_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                        1
#define MAIN_BUS_M2_MAIN_P0_WQOS_EXTCONTROL_EXTTHREN_MASK                                         0x00000002
#define MAIN_BUS_M2_MAIN_P0_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                        2
#define MAIN_BUS_M2_MAIN_P0_WQOS_EXTCONTROL_INTCLKEN_MASK                                         0x00000004
#define MAIN_BUS_M2_MAIN_P0_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                      3
#define MAIN_BUS_M2_MAIN_P0_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                       0x00000008
#define MAIN_BUS_M2_MAIN_P1_RQOS_ID_COREID                                   0x1280
#define MAIN_BUS_M2_MAIN_P1_RQOS_ID_COREID_CORETYPEID_SHIFT                                       0
#define MAIN_BUS_M2_MAIN_P1_RQOS_ID_COREID_CORETYPEID_MASK                                        0x000000ff
#define MAIN_BUS_M2_MAIN_P1_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                     8
#define MAIN_BUS_M2_MAIN_P1_RQOS_ID_COREID_CORECHECKSUM_MASK                                      0xffffff00
#define MAIN_BUS_M2_MAIN_P1_RQOS_ID_REVISIONID                               0x1284
#define MAIN_BUS_M2_MAIN_P1_RQOS_ID_REVISIONID_USERID_SHIFT                                       0
#define MAIN_BUS_M2_MAIN_P1_RQOS_ID_REVISIONID_USERID_MASK                                        0x000000ff
#define MAIN_BUS_M2_MAIN_P1_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                    8
#define MAIN_BUS_M2_MAIN_P1_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                     0xffffff00
#define MAIN_BUS_M2_MAIN_P1_RQOS_PRIORITY                                    0x1288
#define MAIN_BUS_M2_MAIN_P1_RQOS_PRIORITY_P0_SHIFT                                                0
#define MAIN_BUS_M2_MAIN_P1_RQOS_PRIORITY_P0_MASK                                                 0x00000007
#define MAIN_BUS_M2_MAIN_P1_RQOS_PRIORITY_P1_SHIFT                                                8
#define MAIN_BUS_M2_MAIN_P1_RQOS_PRIORITY_P1_MASK                                                 0x00000700
#define MAIN_BUS_M2_MAIN_P1_RQOS_MODE                                        0x128c
#define MAIN_BUS_M2_MAIN_P1_RQOS_MODE_MODE_SHIFT                                                  0
#define MAIN_BUS_M2_MAIN_P1_RQOS_MODE_MODE_MASK                                                   0x00000003
#define MAIN_BUS_M2_MAIN_P1_RQOS_BANDWIDTH                                   0x1290
#define MAIN_BUS_M2_MAIN_P1_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                        0
#define MAIN_BUS_M2_MAIN_P1_RQOS_BANDWIDTH_BANDWIDTH_MASK                                         0x00001fff
#define MAIN_BUS_M2_MAIN_P1_RQOS_SATURATION                                  0x1294
#define MAIN_BUS_M2_MAIN_P1_RQOS_SATURATION_SATURATION_SHIFT                                      0
#define MAIN_BUS_M2_MAIN_P1_RQOS_SATURATION_SATURATION_MASK                                       0x000003ff
#define MAIN_BUS_M2_MAIN_P1_RQOS_EXTCONTROL                                  0x1298
#define MAIN_BUS_M2_MAIN_P1_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                     0
#define MAIN_BUS_M2_MAIN_P1_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                      0x00000001
#define MAIN_BUS_M2_MAIN_P1_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                        1
#define MAIN_BUS_M2_MAIN_P1_RQOS_EXTCONTROL_EXTTHREN_MASK                                         0x00000002
#define MAIN_BUS_M2_MAIN_P1_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                        2
#define MAIN_BUS_M2_MAIN_P1_RQOS_EXTCONTROL_INTCLKEN_MASK                                         0x00000004
#define MAIN_BUS_M2_MAIN_P1_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                      3
#define MAIN_BUS_M2_MAIN_P1_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                       0x00000008
#define MAIN_BUS_M2_MAIN_P1_WQOS_ID_COREID                                   0x1300
#define MAIN_BUS_M2_MAIN_P1_WQOS_ID_COREID_CORETYPEID_SHIFT                                       0
#define MAIN_BUS_M2_MAIN_P1_WQOS_ID_COREID_CORETYPEID_MASK                                        0x000000ff
#define MAIN_BUS_M2_MAIN_P1_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                     8
#define MAIN_BUS_M2_MAIN_P1_WQOS_ID_COREID_CORECHECKSUM_MASK                                      0xffffff00
#define MAIN_BUS_M2_MAIN_P1_WQOS_ID_REVISIONID                               0x1304
#define MAIN_BUS_M2_MAIN_P1_WQOS_ID_REVISIONID_USERID_SHIFT                                       0
#define MAIN_BUS_M2_MAIN_P1_WQOS_ID_REVISIONID_USERID_MASK                                        0x000000ff
#define MAIN_BUS_M2_MAIN_P1_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                    8
#define MAIN_BUS_M2_MAIN_P1_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                     0xffffff00
#define MAIN_BUS_M2_MAIN_P1_WQOS_PRIORITY                                    0x1308
#define MAIN_BUS_M2_MAIN_P1_WQOS_PRIORITY_P0_SHIFT                                                0
#define MAIN_BUS_M2_MAIN_P1_WQOS_PRIORITY_P0_MASK                                                 0x00000007
#define MAIN_BUS_M2_MAIN_P1_WQOS_PRIORITY_P1_SHIFT                                                8
#define MAIN_BUS_M2_MAIN_P1_WQOS_PRIORITY_P1_MASK                                                 0x00000700
#define MAIN_BUS_M2_MAIN_P1_WQOS_MODE                                        0x130c
#define MAIN_BUS_M2_MAIN_P1_WQOS_MODE_MODE_SHIFT                                                  0
#define MAIN_BUS_M2_MAIN_P1_WQOS_MODE_MODE_MASK                                                   0x00000003
#define MAIN_BUS_M2_MAIN_P1_WQOS_BANDWIDTH                                   0x1310
#define MAIN_BUS_M2_MAIN_P1_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                        0
#define MAIN_BUS_M2_MAIN_P1_WQOS_BANDWIDTH_BANDWIDTH_MASK                                         0x00001fff
#define MAIN_BUS_M2_MAIN_P1_WQOS_SATURATION                                  0x1314
#define MAIN_BUS_M2_MAIN_P1_WQOS_SATURATION_SATURATION_SHIFT                                      0
#define MAIN_BUS_M2_MAIN_P1_WQOS_SATURATION_SATURATION_MASK                                       0x000003ff
#define MAIN_BUS_M2_MAIN_P1_WQOS_EXTCONTROL                                  0x1318
#define MAIN_BUS_M2_MAIN_P1_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                     0
#define MAIN_BUS_M2_MAIN_P1_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                      0x00000001
#define MAIN_BUS_M2_MAIN_P1_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                        1
#define MAIN_BUS_M2_MAIN_P1_WQOS_EXTCONTROL_EXTTHREN_MASK                                         0x00000002
#define MAIN_BUS_M2_MAIN_P1_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                        2
#define MAIN_BUS_M2_MAIN_P1_WQOS_EXTCONTROL_INTCLKEN_MASK                                         0x00000004
#define MAIN_BUS_M2_MAIN_P1_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                      3
#define MAIN_BUS_M2_MAIN_P1_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                       0x00000008
#define MAIN_BUS_NPU2MAIN_P0_RQOS_ID_COREID                                  0x1380
#define MAIN_BUS_NPU2MAIN_P0_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P0_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P0_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_NPU2MAIN_P0_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_NPU2MAIN_P0_RQOS_ID_REVISIONID                              0x1384
#define MAIN_BUS_NPU2MAIN_P0_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P0_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P0_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_NPU2MAIN_P0_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_NPU2MAIN_P0_RQOS_PRIORITY                                   0x1388
#define MAIN_BUS_NPU2MAIN_P0_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_NPU2MAIN_P0_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_NPU2MAIN_P0_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_NPU2MAIN_P0_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_NPU2MAIN_P0_RQOS_MODE                                       0x138c
#define MAIN_BUS_NPU2MAIN_P0_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_NPU2MAIN_P0_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_NPU2MAIN_P0_RQOS_BANDWIDTH                                  0x1390
#define MAIN_BUS_NPU2MAIN_P0_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_NPU2MAIN_P0_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00001fff
#define MAIN_BUS_NPU2MAIN_P0_RQOS_SATURATION                                 0x1394
#define MAIN_BUS_NPU2MAIN_P0_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_NPU2MAIN_P0_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_NPU2MAIN_P0_RQOS_EXTCONTROL                                 0x1398
#define MAIN_BUS_NPU2MAIN_P0_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_NPU2MAIN_P0_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_NPU2MAIN_P0_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_NPU2MAIN_P0_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_NPU2MAIN_P0_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_NPU2MAIN_P0_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_NPU2MAIN_P0_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_NPU2MAIN_P0_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_NPU2MAIN_P0_WQOS_ID_COREID                                  0x1400
#define MAIN_BUS_NPU2MAIN_P0_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P0_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P0_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_NPU2MAIN_P0_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_NPU2MAIN_P0_WQOS_ID_REVISIONID                              0x1404
#define MAIN_BUS_NPU2MAIN_P0_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P0_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P0_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_NPU2MAIN_P0_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_NPU2MAIN_P0_WQOS_PRIORITY                                   0x1408
#define MAIN_BUS_NPU2MAIN_P0_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_NPU2MAIN_P0_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_NPU2MAIN_P0_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_NPU2MAIN_P0_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_NPU2MAIN_P0_WQOS_MODE                                       0x140c
#define MAIN_BUS_NPU2MAIN_P0_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_NPU2MAIN_P0_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_NPU2MAIN_P0_WQOS_BANDWIDTH                                  0x1410
#define MAIN_BUS_NPU2MAIN_P0_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_NPU2MAIN_P0_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00001fff
#define MAIN_BUS_NPU2MAIN_P0_WQOS_SATURATION                                 0x1414
#define MAIN_BUS_NPU2MAIN_P0_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_NPU2MAIN_P0_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_NPU2MAIN_P0_WQOS_EXTCONTROL                                 0x1418
#define MAIN_BUS_NPU2MAIN_P0_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_NPU2MAIN_P0_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_NPU2MAIN_P0_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_NPU2MAIN_P0_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_NPU2MAIN_P0_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_NPU2MAIN_P0_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_NPU2MAIN_P0_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_NPU2MAIN_P0_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_NPU2MAIN_P1_RQOS_ID_COREID                                  0x1480
#define MAIN_BUS_NPU2MAIN_P1_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P1_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P1_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_NPU2MAIN_P1_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_NPU2MAIN_P1_RQOS_ID_REVISIONID                              0x1484
#define MAIN_BUS_NPU2MAIN_P1_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P1_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P1_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_NPU2MAIN_P1_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_NPU2MAIN_P1_RQOS_PRIORITY                                   0x1488
#define MAIN_BUS_NPU2MAIN_P1_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_NPU2MAIN_P1_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_NPU2MAIN_P1_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_NPU2MAIN_P1_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_NPU2MAIN_P1_RQOS_MODE                                       0x148c
#define MAIN_BUS_NPU2MAIN_P1_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_NPU2MAIN_P1_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_NPU2MAIN_P1_RQOS_BANDWIDTH                                  0x1490
#define MAIN_BUS_NPU2MAIN_P1_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_NPU2MAIN_P1_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00001fff
#define MAIN_BUS_NPU2MAIN_P1_RQOS_SATURATION                                 0x1494
#define MAIN_BUS_NPU2MAIN_P1_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_NPU2MAIN_P1_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_NPU2MAIN_P1_RQOS_EXTCONTROL                                 0x1498
#define MAIN_BUS_NPU2MAIN_P1_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_NPU2MAIN_P1_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_NPU2MAIN_P1_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_NPU2MAIN_P1_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_NPU2MAIN_P1_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_NPU2MAIN_P1_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_NPU2MAIN_P1_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_NPU2MAIN_P1_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_NPU2MAIN_P1_WQOS_ID_COREID                                  0x1500
#define MAIN_BUS_NPU2MAIN_P1_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P1_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P1_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_NPU2MAIN_P1_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_NPU2MAIN_P1_WQOS_ID_REVISIONID                              0x1504
#define MAIN_BUS_NPU2MAIN_P1_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P1_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P1_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_NPU2MAIN_P1_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_NPU2MAIN_P1_WQOS_PRIORITY                                   0x1508
#define MAIN_BUS_NPU2MAIN_P1_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_NPU2MAIN_P1_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_NPU2MAIN_P1_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_NPU2MAIN_P1_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_NPU2MAIN_P1_WQOS_MODE                                       0x150c
#define MAIN_BUS_NPU2MAIN_P1_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_NPU2MAIN_P1_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_NPU2MAIN_P1_WQOS_BANDWIDTH                                  0x1510
#define MAIN_BUS_NPU2MAIN_P1_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_NPU2MAIN_P1_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00001fff
#define MAIN_BUS_NPU2MAIN_P1_WQOS_SATURATION                                 0x1514
#define MAIN_BUS_NPU2MAIN_P1_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_NPU2MAIN_P1_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_NPU2MAIN_P1_WQOS_EXTCONTROL                                 0x1518
#define MAIN_BUS_NPU2MAIN_P1_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_NPU2MAIN_P1_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_NPU2MAIN_P1_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_NPU2MAIN_P1_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_NPU2MAIN_P1_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_NPU2MAIN_P1_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_NPU2MAIN_P1_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_NPU2MAIN_P1_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_NPU2MAIN_P2_RQOS_ID_COREID                                  0x1580
#define MAIN_BUS_NPU2MAIN_P2_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P2_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P2_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_NPU2MAIN_P2_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_NPU2MAIN_P2_RQOS_ID_REVISIONID                              0x1584
#define MAIN_BUS_NPU2MAIN_P2_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P2_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P2_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_NPU2MAIN_P2_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_NPU2MAIN_P2_RQOS_PRIORITY                                   0x1588
#define MAIN_BUS_NPU2MAIN_P2_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_NPU2MAIN_P2_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_NPU2MAIN_P2_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_NPU2MAIN_P2_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_NPU2MAIN_P2_RQOS_MODE                                       0x158c
#define MAIN_BUS_NPU2MAIN_P2_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_NPU2MAIN_P2_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_NPU2MAIN_P2_RQOS_BANDWIDTH                                  0x1590
#define MAIN_BUS_NPU2MAIN_P2_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_NPU2MAIN_P2_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00001fff
#define MAIN_BUS_NPU2MAIN_P2_RQOS_SATURATION                                 0x1594
#define MAIN_BUS_NPU2MAIN_P2_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_NPU2MAIN_P2_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_NPU2MAIN_P2_RQOS_EXTCONTROL                                 0x1598
#define MAIN_BUS_NPU2MAIN_P2_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_NPU2MAIN_P2_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_NPU2MAIN_P2_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_NPU2MAIN_P2_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_NPU2MAIN_P2_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_NPU2MAIN_P2_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_NPU2MAIN_P2_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_NPU2MAIN_P2_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_NPU2MAIN_P2_WQOS_ID_COREID                                  0x1600
#define MAIN_BUS_NPU2MAIN_P2_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P2_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P2_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_NPU2MAIN_P2_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_NPU2MAIN_P2_WQOS_ID_REVISIONID                              0x1604
#define MAIN_BUS_NPU2MAIN_P2_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P2_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P2_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_NPU2MAIN_P2_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_NPU2MAIN_P2_WQOS_PRIORITY                                   0x1608
#define MAIN_BUS_NPU2MAIN_P2_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_NPU2MAIN_P2_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_NPU2MAIN_P2_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_NPU2MAIN_P2_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_NPU2MAIN_P2_WQOS_MODE                                       0x160c
#define MAIN_BUS_NPU2MAIN_P2_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_NPU2MAIN_P2_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_NPU2MAIN_P2_WQOS_BANDWIDTH                                  0x1610
#define MAIN_BUS_NPU2MAIN_P2_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_NPU2MAIN_P2_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00001fff
#define MAIN_BUS_NPU2MAIN_P2_WQOS_SATURATION                                 0x1614
#define MAIN_BUS_NPU2MAIN_P2_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_NPU2MAIN_P2_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_NPU2MAIN_P2_WQOS_EXTCONTROL                                 0x1618
#define MAIN_BUS_NPU2MAIN_P2_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_NPU2MAIN_P2_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_NPU2MAIN_P2_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_NPU2MAIN_P2_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_NPU2MAIN_P2_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_NPU2MAIN_P2_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_NPU2MAIN_P2_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_NPU2MAIN_P2_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_NPU2MAIN_P3_RQOS_ID_COREID                                  0x1680
#define MAIN_BUS_NPU2MAIN_P3_RQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P3_RQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P3_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_NPU2MAIN_P3_RQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_NPU2MAIN_P3_RQOS_ID_REVISIONID                              0x1684
#define MAIN_BUS_NPU2MAIN_P3_RQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P3_RQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P3_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_NPU2MAIN_P3_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_NPU2MAIN_P3_RQOS_PRIORITY                                   0x1688
#define MAIN_BUS_NPU2MAIN_P3_RQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_NPU2MAIN_P3_RQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_NPU2MAIN_P3_RQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_NPU2MAIN_P3_RQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_NPU2MAIN_P3_RQOS_MODE                                       0x168c
#define MAIN_BUS_NPU2MAIN_P3_RQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_NPU2MAIN_P3_RQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_NPU2MAIN_P3_RQOS_BANDWIDTH                                  0x1690
#define MAIN_BUS_NPU2MAIN_P3_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_NPU2MAIN_P3_RQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00001fff
#define MAIN_BUS_NPU2MAIN_P3_RQOS_SATURATION                                 0x1694
#define MAIN_BUS_NPU2MAIN_P3_RQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_NPU2MAIN_P3_RQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_NPU2MAIN_P3_RQOS_EXTCONTROL                                 0x1698
#define MAIN_BUS_NPU2MAIN_P3_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_NPU2MAIN_P3_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_NPU2MAIN_P3_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_NPU2MAIN_P3_RQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_NPU2MAIN_P3_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_NPU2MAIN_P3_RQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_NPU2MAIN_P3_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_NPU2MAIN_P3_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_NPU2MAIN_P3_WQOS_ID_COREID                                  0x1700
#define MAIN_BUS_NPU2MAIN_P3_WQOS_ID_COREID_CORETYPEID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P3_WQOS_ID_COREID_CORETYPEID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P3_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                    8
#define MAIN_BUS_NPU2MAIN_P3_WQOS_ID_COREID_CORECHECKSUM_MASK                                     0xffffff00
#define MAIN_BUS_NPU2MAIN_P3_WQOS_ID_REVISIONID                              0x1704
#define MAIN_BUS_NPU2MAIN_P3_WQOS_ID_REVISIONID_USERID_SHIFT                                      0
#define MAIN_BUS_NPU2MAIN_P3_WQOS_ID_REVISIONID_USERID_MASK                                       0x000000ff
#define MAIN_BUS_NPU2MAIN_P3_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                   8
#define MAIN_BUS_NPU2MAIN_P3_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                    0xffffff00
#define MAIN_BUS_NPU2MAIN_P3_WQOS_PRIORITY                                   0x1708
#define MAIN_BUS_NPU2MAIN_P3_WQOS_PRIORITY_P0_SHIFT                                               0
#define MAIN_BUS_NPU2MAIN_P3_WQOS_PRIORITY_P0_MASK                                                0x00000007
#define MAIN_BUS_NPU2MAIN_P3_WQOS_PRIORITY_P1_SHIFT                                               8
#define MAIN_BUS_NPU2MAIN_P3_WQOS_PRIORITY_P1_MASK                                                0x00000700
#define MAIN_BUS_NPU2MAIN_P3_WQOS_MODE                                       0x170c
#define MAIN_BUS_NPU2MAIN_P3_WQOS_MODE_MODE_SHIFT                                                 0
#define MAIN_BUS_NPU2MAIN_P3_WQOS_MODE_MODE_MASK                                                  0x00000003
#define MAIN_BUS_NPU2MAIN_P3_WQOS_BANDWIDTH                                  0x1710
#define MAIN_BUS_NPU2MAIN_P3_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                       0
#define MAIN_BUS_NPU2MAIN_P3_WQOS_BANDWIDTH_BANDWIDTH_MASK                                        0x00001fff
#define MAIN_BUS_NPU2MAIN_P3_WQOS_SATURATION                                 0x1714
#define MAIN_BUS_NPU2MAIN_P3_WQOS_SATURATION_SATURATION_SHIFT                                     0
#define MAIN_BUS_NPU2MAIN_P3_WQOS_SATURATION_SATURATION_MASK                                      0x000003ff
#define MAIN_BUS_NPU2MAIN_P3_WQOS_EXTCONTROL                                 0x1718
#define MAIN_BUS_NPU2MAIN_P3_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                    0
#define MAIN_BUS_NPU2MAIN_P3_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                     0x00000001
#define MAIN_BUS_NPU2MAIN_P3_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                       1
#define MAIN_BUS_NPU2MAIN_P3_WQOS_EXTCONTROL_EXTTHREN_MASK                                        0x00000002
#define MAIN_BUS_NPU2MAIN_P3_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                       2
#define MAIN_BUS_NPU2MAIN_P3_WQOS_EXTCONTROL_INTCLKEN_MASK                                        0x00000004
#define MAIN_BUS_NPU2MAIN_P3_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                     3
#define MAIN_BUS_NPU2MAIN_P3_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                      0x00000008
#define MAIN_BUS_SYS1_MAIN_RQOS_ID_COREID                                    0x1780
#define MAIN_BUS_SYS1_MAIN_RQOS_ID_COREID_CORETYPEID_SHIFT                                        0
#define MAIN_BUS_SYS1_MAIN_RQOS_ID_COREID_CORETYPEID_MASK                                         0x000000ff
#define MAIN_BUS_SYS1_MAIN_RQOS_ID_COREID_CORECHECKSUM_SHIFT                                      8
#define MAIN_BUS_SYS1_MAIN_RQOS_ID_COREID_CORECHECKSUM_MASK                                       0xffffff00
#define MAIN_BUS_SYS1_MAIN_RQOS_ID_REVISIONID                                0x1784
#define MAIN_BUS_SYS1_MAIN_RQOS_ID_REVISIONID_USERID_SHIFT                                        0
#define MAIN_BUS_SYS1_MAIN_RQOS_ID_REVISIONID_USERID_MASK                                         0x000000ff
#define MAIN_BUS_SYS1_MAIN_RQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                     8
#define MAIN_BUS_SYS1_MAIN_RQOS_ID_REVISIONID_FLEXNOCID_MASK                                      0xffffff00
#define MAIN_BUS_SYS1_MAIN_RQOS_PRIORITY                                     0x1788
#define MAIN_BUS_SYS1_MAIN_RQOS_PRIORITY_P0_SHIFT                                                 0
#define MAIN_BUS_SYS1_MAIN_RQOS_PRIORITY_P0_MASK                                                  0x00000007
#define MAIN_BUS_SYS1_MAIN_RQOS_PRIORITY_P1_SHIFT                                                 8
#define MAIN_BUS_SYS1_MAIN_RQOS_PRIORITY_P1_MASK                                                  0x00000700
#define MAIN_BUS_SYS1_MAIN_RQOS_MODE                                         0x178c
#define MAIN_BUS_SYS1_MAIN_RQOS_MODE_MODE_SHIFT                                                   0
#define MAIN_BUS_SYS1_MAIN_RQOS_MODE_MODE_MASK                                                    0x00000003
#define MAIN_BUS_SYS1_MAIN_RQOS_BANDWIDTH                                    0x1790
#define MAIN_BUS_SYS1_MAIN_RQOS_BANDWIDTH_BANDWIDTH_SHIFT                                         0
#define MAIN_BUS_SYS1_MAIN_RQOS_BANDWIDTH_BANDWIDTH_MASK                                          0x00001fff
#define MAIN_BUS_SYS1_MAIN_RQOS_SATURATION                                   0x1794
#define MAIN_BUS_SYS1_MAIN_RQOS_SATURATION_SATURATION_SHIFT                                       0
#define MAIN_BUS_SYS1_MAIN_RQOS_SATURATION_SATURATION_MASK                                        0x000003ff
#define MAIN_BUS_SYS1_MAIN_RQOS_EXTCONTROL                                   0x1798
#define MAIN_BUS_SYS1_MAIN_RQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                      0
#define MAIN_BUS_SYS1_MAIN_RQOS_EXTCONTROL_SOCKETQOSEN_MASK                                       0x00000001
#define MAIN_BUS_SYS1_MAIN_RQOS_EXTCONTROL_EXTTHREN_SHIFT                                         1
#define MAIN_BUS_SYS1_MAIN_RQOS_EXTCONTROL_EXTTHREN_MASK                                          0x00000002
#define MAIN_BUS_SYS1_MAIN_RQOS_EXTCONTROL_INTCLKEN_SHIFT                                         2
#define MAIN_BUS_SYS1_MAIN_RQOS_EXTCONTROL_INTCLKEN_MASK                                          0x00000004
#define MAIN_BUS_SYS1_MAIN_RQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                       3
#define MAIN_BUS_SYS1_MAIN_RQOS_EXTCONTROL_EXTLIMITEN_MASK                                        0x00000008
#define MAIN_BUS_SYS1_MAIN_WQOS_ID_COREID                                    0x1800
#define MAIN_BUS_SYS1_MAIN_WQOS_ID_COREID_CORETYPEID_SHIFT                                        0
#define MAIN_BUS_SYS1_MAIN_WQOS_ID_COREID_CORETYPEID_MASK                                         0x000000ff
#define MAIN_BUS_SYS1_MAIN_WQOS_ID_COREID_CORECHECKSUM_SHIFT                                      8
#define MAIN_BUS_SYS1_MAIN_WQOS_ID_COREID_CORECHECKSUM_MASK                                       0xffffff00
#define MAIN_BUS_SYS1_MAIN_WQOS_ID_REVISIONID                                0x1804
#define MAIN_BUS_SYS1_MAIN_WQOS_ID_REVISIONID_USERID_SHIFT                                        0
#define MAIN_BUS_SYS1_MAIN_WQOS_ID_REVISIONID_USERID_MASK                                         0x000000ff
#define MAIN_BUS_SYS1_MAIN_WQOS_ID_REVISIONID_FLEXNOCID_SHIFT                                     8
#define MAIN_BUS_SYS1_MAIN_WQOS_ID_REVISIONID_FLEXNOCID_MASK                                      0xffffff00
#define MAIN_BUS_SYS1_MAIN_WQOS_PRIORITY                                     0x1808
#define MAIN_BUS_SYS1_MAIN_WQOS_PRIORITY_P0_SHIFT                                                 0
#define MAIN_BUS_SYS1_MAIN_WQOS_PRIORITY_P0_MASK                                                  0x00000007
#define MAIN_BUS_SYS1_MAIN_WQOS_PRIORITY_P1_SHIFT                                                 8
#define MAIN_BUS_SYS1_MAIN_WQOS_PRIORITY_P1_MASK                                                  0x00000700
#define MAIN_BUS_SYS1_MAIN_WQOS_MODE                                         0x180c
#define MAIN_BUS_SYS1_MAIN_WQOS_MODE_MODE_SHIFT                                                   0
#define MAIN_BUS_SYS1_MAIN_WQOS_MODE_MODE_MASK                                                    0x00000003
#define MAIN_BUS_SYS1_MAIN_WQOS_BANDWIDTH                                    0x1810
#define MAIN_BUS_SYS1_MAIN_WQOS_BANDWIDTH_BANDWIDTH_SHIFT                                         0
#define MAIN_BUS_SYS1_MAIN_WQOS_BANDWIDTH_BANDWIDTH_MASK                                          0x00001fff
#define MAIN_BUS_SYS1_MAIN_WQOS_SATURATION                                   0x1814
#define MAIN_BUS_SYS1_MAIN_WQOS_SATURATION_SATURATION_SHIFT                                       0
#define MAIN_BUS_SYS1_MAIN_WQOS_SATURATION_SATURATION_MASK                                        0x000003ff
#define MAIN_BUS_SYS1_MAIN_WQOS_EXTCONTROL                                   0x1818
#define MAIN_BUS_SYS1_MAIN_WQOS_EXTCONTROL_SOCKETQOSEN_SHIFT                                      0
#define MAIN_BUS_SYS1_MAIN_WQOS_EXTCONTROL_SOCKETQOSEN_MASK                                       0x00000001
#define MAIN_BUS_SYS1_MAIN_WQOS_EXTCONTROL_EXTTHREN_SHIFT                                         1
#define MAIN_BUS_SYS1_MAIN_WQOS_EXTCONTROL_EXTTHREN_MASK                                          0x00000002
#define MAIN_BUS_SYS1_MAIN_WQOS_EXTCONTROL_INTCLKEN_SHIFT                                         2
#define MAIN_BUS_SYS1_MAIN_WQOS_EXTCONTROL_INTCLKEN_MASK                                          0x00000004
#define MAIN_BUS_SYS1_MAIN_WQOS_EXTCONTROL_EXTLIMITEN_SHIFT                                       3
#define MAIN_BUS_SYS1_MAIN_WQOS_EXTCONTROL_EXTLIMITEN_MASK                                        0x00000008
#define MAIN_BUS_TRACE_0_ID_COREID                                           0x1880
#define MAIN_BUS_TRACE_0_ID_COREID_CORETYPEID_SHIFT                                               0
#define MAIN_BUS_TRACE_0_ID_COREID_CORETYPEID_MASK                                                0x000000ff
#define MAIN_BUS_TRACE_0_ID_COREID_CORECHECKSUM_SHIFT                                             8
#define MAIN_BUS_TRACE_0_ID_COREID_CORECHECKSUM_MASK                                              0xffffff00
#define MAIN_BUS_TRACE_0_ID_REVISIONID                                       0x1884
#define MAIN_BUS_TRACE_0_ID_REVISIONID_USERID_SHIFT                                               0
#define MAIN_BUS_TRACE_0_ID_REVISIONID_USERID_MASK                                                0x000000ff
#define MAIN_BUS_TRACE_0_ID_REVISIONID_FLEXNOCID_SHIFT                                            8
#define MAIN_BUS_TRACE_0_ID_REVISIONID_FLEXNOCID_MASK                                             0xffffff00
#define MAIN_BUS_TRACE_0_FAULTEN                                             0x1888
#define MAIN_BUS_TRACE_0_FAULTEN_FAULTEN_SHIFT                                                    0
#define MAIN_BUS_TRACE_0_FAULTEN_FAULTEN_MASK                                                     0x00000001
#define MAIN_BUS_TRACE_0_ERRVLD                                              0x188c
#define MAIN_BUS_TRACE_0_ERRVLD_ERRVLD_SHIFT                                                      0
#define MAIN_BUS_TRACE_0_ERRVLD_ERRVLD_MASK                                                       0x00000001
#define MAIN_BUS_TRACE_0_ERRCLR                                              0x1890
#define MAIN_BUS_TRACE_0_ERRCLR_ERRCLR_SHIFT                                                      0
#define MAIN_BUS_TRACE_0_ERRCLR_ERRCLR_MASK                                                       0x00000001
#define MAIN_BUS_TRACE_0_ERRLOG0                                             0x1894
#define MAIN_BUS_TRACE_0_ERRLOG0_LOCK_SHIFT                                                       0
#define MAIN_BUS_TRACE_0_ERRLOG0_LOCK_MASK                                                        0x00000001
#define MAIN_BUS_TRACE_0_ERRLOG0_OPC_SHIFT                                                        1
#define MAIN_BUS_TRACE_0_ERRLOG0_OPC_MASK                                                         0x0000001e
#define MAIN_BUS_TRACE_0_ERRLOG0_ERRCODE_SHIFT                                                    8
#define MAIN_BUS_TRACE_0_ERRLOG0_ERRCODE_MASK                                                     0x00000700
#define MAIN_BUS_TRACE_0_ERRLOG0_LEN1_SHIFT                                                       16
#define MAIN_BUS_TRACE_0_ERRLOG0_LEN1_MASK                                                        0x007f0000
#define MAIN_BUS_TRACE_0_ERRLOG0_FORMAT_SHIFT                                                     31
#define MAIN_BUS_TRACE_0_ERRLOG0_FORMAT_MASK                                                      0x80000000
#define MAIN_BUS_TRACE_0_ERRLOG1                                             0x1898
#define MAIN_BUS_TRACE_0_ERRLOG1_ERRLOG1_SHIFT                                                    0
#define MAIN_BUS_TRACE_0_ERRLOG1_ERRLOG1_MASK                                                     0x01ffffff
#define MAIN_BUS_TRACE_0_ERRLOG3                                             0x18a0
#define MAIN_BUS_TRACE_0_ERRLOG4                                             0x18a4
#define MAIN_BUS_TRACE_0_ERRLOG4_ERRLOG4_SHIFT                                                    0
#define MAIN_BUS_TRACE_0_ERRLOG4_ERRLOG4_MASK                                                     0x0000007f
#define MAIN_BUS_TRACE_0_ERRLOG5                                             0x18a8
#define MAIN_BUS_TRACE_0_ERRLOG6                                             0x18ac
#define MAIN_BUS_TRACE_0_ERRLOG6_ERRLOG6_SHIFT                                                    0
#define MAIN_BUS_TRACE_0_ERRLOG6_ERRLOG6_MASK                                                     0x00003fff
#define MAIN_BUS_TRACE_0_ERRLOG7                                             0x18b0
#define MAIN_BUS_TRACE_0_ERRLOG7_ERRLOG7_SHIFT                                                    0
#define MAIN_BUS_TRACE_0_ERRLOG7_ERRLOG7_MASK                                                     0x00000001
#define MAIN_BUS_TRACE_0_STALLEN                                             0x18cc
#define MAIN_BUS_TRACE_0_STALLEN_STALLEN_SHIFT                                                    0
#define MAIN_BUS_TRACE_0_STALLEN_STALLEN_MASK                                                     0x00000001
#define MAIN_BUS_ATB_ID_COREID                                               0x1900
#define MAIN_BUS_ATB_ID_COREID_CORETYPEID_SHIFT                                                   0
#define MAIN_BUS_ATB_ID_COREID_CORETYPEID_MASK                                                    0x000000ff
#define MAIN_BUS_ATB_ID_COREID_CORECHECKSUM_SHIFT                                                 8
#define MAIN_BUS_ATB_ID_COREID_CORECHECKSUM_MASK                                                  0xffffff00
#define MAIN_BUS_ATB_ID_REVISIONID                                           0x1904
#define MAIN_BUS_ATB_ID_REVISIONID_USERID_SHIFT                                                   0
#define MAIN_BUS_ATB_ID_REVISIONID_USERID_MASK                                                    0x000000ff
#define MAIN_BUS_ATB_ID_REVISIONID_FLEXNOCID_SHIFT                                                8
#define MAIN_BUS_ATB_ID_REVISIONID_FLEXNOCID_MASK                                                 0xffffff00
#define MAIN_BUS_ATB_ATBID                                                   0x1908
#define MAIN_BUS_ATB_ATBID_ATBID_SHIFT                                                            0
#define MAIN_BUS_ATB_ATBID_ATBID_MASK                                                             0x0000007f
#define MAIN_BUS_ATB_ATBEN                                                   0x190c
#define MAIN_BUS_ATB_ATBEN_ATBEN_SHIFT                                                            0
#define MAIN_BUS_ATB_ATBEN_ATBEN_MASK                                                             0x00000001
#define MAIN_BUS_ATB_SYNCPERIOD                                              0x1910
#define MAIN_BUS_ATB_SYNCPERIOD_SYNCPERIOD_SHIFT                                                  0
#define MAIN_BUS_ATB_SYNCPERIOD_SYNCPERIOD_MASK                                                   0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_ID_COREID                       0x2000
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_ID_COREID_CORETYPEID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_ID_COREID_CORETYPEID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_ID_COREID_CORECHECKSUM_SHIFT                         8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_ID_COREID_CORECHECKSUM_MASK                          0xffffff00
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_ID_REVISIONID                   0x2004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_ID_REVISIONID_USERID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_ID_REVISIONID_USERID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_ID_REVISIONID_FLEXNOCID_SHIFT                        8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_ID_REVISIONID_FLEXNOCID_MASK                         0xffffff00
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL                         0x2008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_ERREN_SHIFT                                  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_ERREN_MASK                                   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_TRACEEN_SHIFT                                1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_TRACEEN_MASK                                 0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_PAYLOADEN_SHIFT                              2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_PAYLOADEN_MASK                               0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_STATEN_SHIFT                                 3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_STATEN_MASK                                  0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_ALARMEN_SHIFT                                4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_ALARMEN_MASK                                 0x00000010
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_STATCONDDUMP_SHIFT                           5
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_STATCONDDUMP_MASK                            0x00000020
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_INTRUSIVEMODE_SHIFT                          6
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_INTRUSIVEMODE_MASK                           0x00000040
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_SHIFT              7
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_MASK               0x00000080
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_CFGCTL                          0x200c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_CFGCTL_GLOBALEN_SHIFT                                0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_CFGCTL_GLOBALEN_MASK                                 0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_CFGCTL_ACTIVE_SHIFT                                  1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_CFGCTL_ACTIVE_MASK                                   0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERLUT                       0x2014
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERLUT_FILTERLUT_SHIFT                            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERLUT_FILTERLUT_MASK                             0x0000ffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_TRACEALARMEN                    0x2018
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_TRACEALARMEN_TRACEALARMEN_SHIFT                      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_TRACEALARMEN_TRACEALARMEN_MASK                       0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_TRACEALARMSTATUS                0x201c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_TRACEALARMSTATUS_TRACEALARMSTATUS_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_TRACEALARMSTATUS_TRACEALARMSTATUS_MASK               0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_TRACEALARMCLR                   0x2020
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_TRACEALARMCLR_TRACEALARMCLR_SHIFT                    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_TRACEALARMCLR_TRACEALARMCLR_MASK                     0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDBASE           0x2080
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDMASK           0x2084
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_ADDRBASE_LOW          0x2088
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_ADDRBASE_HIGH         0x208c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_WINDOWSIZE            0x2090
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_SECURITYBASE          0x2094
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_SECURITYMASK          0x2098
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_OPCODE                0x209c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_STATUS                0x20a0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_LENGTH                0x20a4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_LENGTH_FILTERS_0_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_LENGTH_FILTERS_0_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_URGENCY               0x20a8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_URGENCY_FILTERS_0_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_URGENCY_FILTERS_0_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_USERBASE              0x20ac
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_USERMASK              0x20b0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_USERBASEHIGH          0x20b4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_USERMASKHIGH          0x20b8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDBASE           0x20e0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDMASK           0x20e4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_ADDRBASE_LOW          0x20e8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_ADDRBASE_HIGH         0x20ec
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_WINDOWSIZE            0x20f0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_SECURITYBASE          0x20f4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_SECURITYMASK          0x20f8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_OPCODE                0x20fc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_STATUS                0x2100
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_LENGTH                0x2104
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_LENGTH_FILTERS_1_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_LENGTH_FILTERS_1_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_URGENCY               0x2108
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_URGENCY_FILTERS_1_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_URGENCY_FILTERS_1_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_USERBASE              0x210c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_USERMASK              0x2110
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_USERBASEHIGH          0x2114
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_USERMASKHIGH          0x2118
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDBASE           0x2140
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDMASK           0x2144
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_ADDRBASE_LOW          0x2148
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_ADDRBASE_HIGH         0x214c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_WINDOWSIZE            0x2150
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_SECURITYBASE          0x2154
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_SECURITYMASK          0x2158
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_OPCODE                0x215c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_STATUS                0x2160
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_LENGTH                0x2164
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_LENGTH_FILTERS_2_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_LENGTH_FILTERS_2_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_URGENCY               0x2168
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_URGENCY_FILTERS_2_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_URGENCY_FILTERS_2_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_USERBASE              0x216c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_USERMASK              0x2170
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_USERBASEHIGH          0x2174
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_USERMASKHIGH          0x2178
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDBASE           0x21a0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDMASK           0x21a4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_ADDRBASE_LOW          0x21a8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_ADDRBASE_HIGH         0x21ac
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_WINDOWSIZE            0x21b0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_SECURITYBASE          0x21b4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_SECURITYMASK          0x21b8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_OPCODE                0x21bc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_STATUS                0x21c0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_LENGTH                0x21c4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_LENGTH_FILTERS_3_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_LENGTH_FILTERS_3_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_URGENCY               0x21c8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_URGENCY_FILTERS_3_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_URGENCY_FILTERS_3_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_USERBASE              0x21cc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_USERMASK              0x21d0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_USERBASEHIGH          0x21d4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_USERMASKHIGH          0x21d8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P0_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_ID_COREID                       0x2400
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_ID_COREID_CORETYPEID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_ID_COREID_CORETYPEID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_ID_COREID_CORECHECKSUM_SHIFT                         8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_ID_COREID_CORECHECKSUM_MASK                          0xffffff00
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_ID_REVISIONID                   0x2404
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_ID_REVISIONID_USERID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_ID_REVISIONID_USERID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_ID_REVISIONID_FLEXNOCID_SHIFT                        8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_ID_REVISIONID_FLEXNOCID_MASK                         0xffffff00
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL                         0x2408
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_ERREN_SHIFT                                  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_ERREN_MASK                                   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_TRACEEN_SHIFT                                1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_TRACEEN_MASK                                 0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_PAYLOADEN_SHIFT                              2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_PAYLOADEN_MASK                               0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_STATEN_SHIFT                                 3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_STATEN_MASK                                  0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_ALARMEN_SHIFT                                4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_ALARMEN_MASK                                 0x00000010
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_STATCONDDUMP_SHIFT                           5
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_STATCONDDUMP_MASK                            0x00000020
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_INTRUSIVEMODE_SHIFT                          6
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_INTRUSIVEMODE_MASK                           0x00000040
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_SHIFT              7
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_MASK               0x00000080
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_CFGCTL                          0x240c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_CFGCTL_GLOBALEN_SHIFT                                0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_CFGCTL_GLOBALEN_MASK                                 0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_CFGCTL_ACTIVE_SHIFT                                  1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_CFGCTL_ACTIVE_MASK                                   0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERLUT                       0x2414
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERLUT_FILTERLUT_SHIFT                            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERLUT_FILTERLUT_MASK                             0x0000ffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_TRACEALARMEN                    0x2418
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_TRACEALARMEN_TRACEALARMEN_SHIFT                      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_TRACEALARMEN_TRACEALARMEN_MASK                       0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_TRACEALARMSTATUS                0x241c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_TRACEALARMSTATUS_TRACEALARMSTATUS_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_TRACEALARMSTATUS_TRACEALARMSTATUS_MASK               0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_TRACEALARMCLR                   0x2420
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_TRACEALARMCLR_TRACEALARMCLR_SHIFT                    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_TRACEALARMCLR_TRACEALARMCLR_MASK                     0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDBASE           0x2480
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDMASK           0x2484
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_ADDRBASE_LOW          0x2488
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_ADDRBASE_HIGH         0x248c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_WINDOWSIZE            0x2490
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_SECURITYBASE          0x2494
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_SECURITYMASK          0x2498
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_OPCODE                0x249c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_STATUS                0x24a0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_LENGTH                0x24a4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_LENGTH_FILTERS_0_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_LENGTH_FILTERS_0_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_URGENCY               0x24a8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_URGENCY_FILTERS_0_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_URGENCY_FILTERS_0_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_USERBASE              0x24ac
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_USERMASK              0x24b0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_USERBASEHIGH          0x24b4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_USERMASKHIGH          0x24b8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDBASE           0x24e0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDMASK           0x24e4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_ADDRBASE_LOW          0x24e8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_ADDRBASE_HIGH         0x24ec
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_WINDOWSIZE            0x24f0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_SECURITYBASE          0x24f4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_SECURITYMASK          0x24f8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_OPCODE                0x24fc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_STATUS                0x2500
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_LENGTH                0x2504
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_LENGTH_FILTERS_1_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_LENGTH_FILTERS_1_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_URGENCY               0x2508
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_URGENCY_FILTERS_1_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_URGENCY_FILTERS_1_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_USERBASE              0x250c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_USERMASK              0x2510
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_USERBASEHIGH          0x2514
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_USERMASKHIGH          0x2518
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDBASE           0x2540
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDMASK           0x2544
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_ADDRBASE_LOW          0x2548
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_ADDRBASE_HIGH         0x254c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_WINDOWSIZE            0x2550
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_SECURITYBASE          0x2554
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_SECURITYMASK          0x2558
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_OPCODE                0x255c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_STATUS                0x2560
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_LENGTH                0x2564
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_LENGTH_FILTERS_2_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_LENGTH_FILTERS_2_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_URGENCY               0x2568
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_URGENCY_FILTERS_2_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_URGENCY_FILTERS_2_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_USERBASE              0x256c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_USERMASK              0x2570
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_USERBASEHIGH          0x2574
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_USERMASKHIGH          0x2578
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDBASE           0x25a0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDMASK           0x25a4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_ADDRBASE_LOW          0x25a8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_ADDRBASE_HIGH         0x25ac
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_WINDOWSIZE            0x25b0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_SECURITYBASE          0x25b4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_SECURITYMASK          0x25b8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_OPCODE                0x25bc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_STATUS                0x25c0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_LENGTH                0x25c4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_LENGTH_FILTERS_3_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_LENGTH_FILTERS_3_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_URGENCY               0x25c8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_URGENCY_FILTERS_3_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_URGENCY_FILTERS_3_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_USERBASE              0x25cc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_USERMASK              0x25d0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_USERBASEHIGH          0x25d4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_USERMASKHIGH          0x25d8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P1_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_ID_COREID                       0x2800
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_ID_COREID_CORETYPEID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_ID_COREID_CORETYPEID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_ID_COREID_CORECHECKSUM_SHIFT                         8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_ID_COREID_CORECHECKSUM_MASK                          0xffffff00
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_ID_REVISIONID                   0x2804
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_ID_REVISIONID_USERID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_ID_REVISIONID_USERID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_ID_REVISIONID_FLEXNOCID_SHIFT                        8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_ID_REVISIONID_FLEXNOCID_MASK                         0xffffff00
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL                         0x2808
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_ERREN_SHIFT                                  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_ERREN_MASK                                   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_TRACEEN_SHIFT                                1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_TRACEEN_MASK                                 0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_PAYLOADEN_SHIFT                              2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_PAYLOADEN_MASK                               0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_STATEN_SHIFT                                 3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_STATEN_MASK                                  0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_ALARMEN_SHIFT                                4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_ALARMEN_MASK                                 0x00000010
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_STATCONDDUMP_SHIFT                           5
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_STATCONDDUMP_MASK                            0x00000020
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_INTRUSIVEMODE_SHIFT                          6
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_INTRUSIVEMODE_MASK                           0x00000040
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_SHIFT              7
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_MASK               0x00000080
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_CFGCTL                          0x280c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_CFGCTL_GLOBALEN_SHIFT                                0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_CFGCTL_GLOBALEN_MASK                                 0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_CFGCTL_ACTIVE_SHIFT                                  1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_CFGCTL_ACTIVE_MASK                                   0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERLUT                       0x2814
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERLUT_FILTERLUT_SHIFT                            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERLUT_FILTERLUT_MASK                             0x0000ffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_TRACEALARMEN                    0x2818
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_TRACEALARMEN_TRACEALARMEN_SHIFT                      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_TRACEALARMEN_TRACEALARMEN_MASK                       0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_TRACEALARMSTATUS                0x281c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_TRACEALARMSTATUS_TRACEALARMSTATUS_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_TRACEALARMSTATUS_TRACEALARMSTATUS_MASK               0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_TRACEALARMCLR                   0x2820
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_TRACEALARMCLR_TRACEALARMCLR_SHIFT                    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_TRACEALARMCLR_TRACEALARMCLR_MASK                     0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDBASE           0x2880
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDMASK           0x2884
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_ADDRBASE_LOW          0x2888
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_ADDRBASE_HIGH         0x288c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_WINDOWSIZE            0x2890
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_SECURITYBASE          0x2894
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_SECURITYMASK          0x2898
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_OPCODE                0x289c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_STATUS                0x28a0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_LENGTH                0x28a4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_LENGTH_FILTERS_0_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_LENGTH_FILTERS_0_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_URGENCY               0x28a8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_URGENCY_FILTERS_0_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_URGENCY_FILTERS_0_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_USERBASE              0x28ac
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_USERMASK              0x28b0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_USERBASEHIGH          0x28b4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_USERMASKHIGH          0x28b8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDBASE           0x28e0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDMASK           0x28e4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_ADDRBASE_LOW          0x28e8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_ADDRBASE_HIGH         0x28ec
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_WINDOWSIZE            0x28f0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_SECURITYBASE          0x28f4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_SECURITYMASK          0x28f8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_OPCODE                0x28fc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_STATUS                0x2900
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_LENGTH                0x2904
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_LENGTH_FILTERS_1_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_LENGTH_FILTERS_1_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_URGENCY               0x2908
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_URGENCY_FILTERS_1_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_URGENCY_FILTERS_1_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_USERBASE              0x290c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_USERMASK              0x2910
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_USERBASEHIGH          0x2914
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_USERMASKHIGH          0x2918
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDBASE           0x2940
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDMASK           0x2944
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_ADDRBASE_LOW          0x2948
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_ADDRBASE_HIGH         0x294c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_WINDOWSIZE            0x2950
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_SECURITYBASE          0x2954
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_SECURITYMASK          0x2958
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_OPCODE                0x295c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_STATUS                0x2960
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_LENGTH                0x2964
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_LENGTH_FILTERS_2_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_LENGTH_FILTERS_2_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_URGENCY               0x2968
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_URGENCY_FILTERS_2_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_URGENCY_FILTERS_2_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_USERBASE              0x296c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_USERMASK              0x2970
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_USERBASEHIGH          0x2974
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_USERMASKHIGH          0x2978
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDBASE           0x29a0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDMASK           0x29a4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_ADDRBASE_LOW          0x29a8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_ADDRBASE_HIGH         0x29ac
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_WINDOWSIZE            0x29b0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_SECURITYBASE          0x29b4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_SECURITYMASK          0x29b8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_OPCODE                0x29bc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_STATUS                0x29c0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_LENGTH                0x29c4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_LENGTH_FILTERS_3_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_LENGTH_FILTERS_3_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_URGENCY               0x29c8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_URGENCY_FILTERS_3_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_URGENCY_FILTERS_3_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_USERBASE              0x29cc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_USERMASK              0x29d0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_USERBASEHIGH          0x29d4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_USERMASKHIGH          0x29d8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P2_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_ID_COREID                       0x2c00
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_ID_COREID_CORETYPEID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_ID_COREID_CORETYPEID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_ID_COREID_CORECHECKSUM_SHIFT                         8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_ID_COREID_CORECHECKSUM_MASK                          0xffffff00
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_ID_REVISIONID                   0x2c04
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_ID_REVISIONID_USERID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_ID_REVISIONID_USERID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_ID_REVISIONID_FLEXNOCID_SHIFT                        8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_ID_REVISIONID_FLEXNOCID_MASK                         0xffffff00
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL                         0x2c08
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_ERREN_SHIFT                                  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_ERREN_MASK                                   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_TRACEEN_SHIFT                                1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_TRACEEN_MASK                                 0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_PAYLOADEN_SHIFT                              2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_PAYLOADEN_MASK                               0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_STATEN_SHIFT                                 3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_STATEN_MASK                                  0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_ALARMEN_SHIFT                                4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_ALARMEN_MASK                                 0x00000010
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_STATCONDDUMP_SHIFT                           5
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_STATCONDDUMP_MASK                            0x00000020
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_INTRUSIVEMODE_SHIFT                          6
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_INTRUSIVEMODE_MASK                           0x00000040
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_SHIFT              7
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_MASK               0x00000080
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_CFGCTL                          0x2c0c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_CFGCTL_GLOBALEN_SHIFT                                0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_CFGCTL_GLOBALEN_MASK                                 0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_CFGCTL_ACTIVE_SHIFT                                  1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_CFGCTL_ACTIVE_MASK                                   0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERLUT                       0x2c14
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERLUT_FILTERLUT_SHIFT                            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERLUT_FILTERLUT_MASK                             0x0000ffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_TRACEALARMEN                    0x2c18
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_TRACEALARMEN_TRACEALARMEN_SHIFT                      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_TRACEALARMEN_TRACEALARMEN_MASK                       0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_TRACEALARMSTATUS                0x2c1c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_TRACEALARMSTATUS_TRACEALARMSTATUS_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_TRACEALARMSTATUS_TRACEALARMSTATUS_MASK               0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_TRACEALARMCLR                   0x2c20
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_TRACEALARMCLR_TRACEALARMCLR_SHIFT                    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_TRACEALARMCLR_TRACEALARMCLR_MASK                     0x0000001f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDBASE           0x2c80
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDMASK           0x2c84
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_ADDRBASE_LOW          0x2c88
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_ADDRBASE_HIGH         0x2c8c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_WINDOWSIZE            0x2c90
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_SECURITYBASE          0x2c94
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_SECURITYMASK          0x2c98
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_OPCODE                0x2c9c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_STATUS                0x2ca0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_LENGTH                0x2ca4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_LENGTH_FILTERS_0_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_LENGTH_FILTERS_0_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_URGENCY               0x2ca8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_URGENCY_FILTERS_0_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_URGENCY_FILTERS_0_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_USERBASE              0x2cac
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_USERMASK              0x2cb0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_USERBASEHIGH          0x2cb4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_USERMASKHIGH          0x2cb8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDBASE           0x2ce0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDMASK           0x2ce4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_ADDRBASE_LOW          0x2ce8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_ADDRBASE_HIGH         0x2cec
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_WINDOWSIZE            0x2cf0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_SECURITYBASE          0x2cf4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_SECURITYMASK          0x2cf8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_OPCODE                0x2cfc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_STATUS                0x2d00
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_LENGTH                0x2d04
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_LENGTH_FILTERS_1_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_LENGTH_FILTERS_1_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_URGENCY               0x2d08
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_URGENCY_FILTERS_1_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_URGENCY_FILTERS_1_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_USERBASE              0x2d0c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_USERMASK              0x2d10
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_USERBASEHIGH          0x2d14
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_USERMASKHIGH          0x2d18
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDBASE           0x2d40
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDMASK           0x2d44
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_ADDRBASE_LOW          0x2d48
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_ADDRBASE_HIGH         0x2d4c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_WINDOWSIZE            0x2d50
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_SECURITYBASE          0x2d54
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_SECURITYMASK          0x2d58
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_OPCODE                0x2d5c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_STATUS                0x2d60
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_LENGTH                0x2d64
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_LENGTH_FILTERS_2_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_LENGTH_FILTERS_2_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_URGENCY               0x2d68
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_URGENCY_FILTERS_2_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_URGENCY_FILTERS_2_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_USERBASE              0x2d6c
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_USERMASK              0x2d70
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_USERBASEHIGH          0x2d74
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_USERMASKHIGH          0x2d78
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDBASE           0x2da0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDMASK           0x2da4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_ADDRBASE_LOW          0x2da8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_ADDRBASE_HIGH         0x2dac
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_WINDOWSIZE            0x2db0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_SECURITYBASE          0x2db4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_SECURITYMASK          0x2db8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_OPCODE                0x2dbc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_STATUS                0x2dc0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_LENGTH                0x2dc4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_LENGTH_FILTERS_3_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_LENGTH_FILTERS_3_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_URGENCY               0x2dc8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_URGENCY_FILTERS_3_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_URGENCY_FILTERS_3_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_USERBASE              0x2dcc
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_USERMASK              0x2dd0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_USERBASEHIGH          0x2dd4
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_USERMASKHIGH          0x2dd8
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_RREQ_MAIN2DDR_P3_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_ID_COREID                       0x3000
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_ID_COREID_CORETYPEID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_ID_COREID_CORETYPEID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_ID_COREID_CORECHECKSUM_SHIFT                         8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_ID_COREID_CORECHECKSUM_MASK                          0xffffff00
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_ID_REVISIONID                   0x3004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_ID_REVISIONID_USERID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_ID_REVISIONID_USERID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_ID_REVISIONID_FLEXNOCID_SHIFT                        8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_ID_REVISIONID_FLEXNOCID_MASK                         0xffffff00
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL                         0x3008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_ERREN_SHIFT                                  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_ERREN_MASK                                   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_TRACEEN_SHIFT                                1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_TRACEEN_MASK                                 0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_PAYLOADEN_SHIFT                              2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_PAYLOADEN_MASK                               0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_STATEN_SHIFT                                 3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_STATEN_MASK                                  0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_ALARMEN_SHIFT                                4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_ALARMEN_MASK                                 0x00000010
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_STATCONDDUMP_SHIFT                           5
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_STATCONDDUMP_MASK                            0x00000020
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_INTRUSIVEMODE_SHIFT                          6
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_INTRUSIVEMODE_MASK                           0x00000040
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_SHIFT              7
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_MASK               0x00000080
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_CFGCTL                          0x300c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_CFGCTL_GLOBALEN_SHIFT                                0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_CFGCTL_GLOBALEN_MASK                                 0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_CFGCTL_ACTIVE_SHIFT                                  1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_CFGCTL_ACTIVE_MASK                                   0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERLUT                       0x3014
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERLUT_FILTERLUT_SHIFT                            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERLUT_FILTERLUT_MASK                             0x0000ffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_TRACEALARMEN                    0x3018
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_TRACEALARMEN_TRACEALARMEN_SHIFT                      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_TRACEALARMEN_TRACEALARMEN_MASK                       0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_TRACEALARMSTATUS                0x301c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_TRACEALARMSTATUS_TRACEALARMSTATUS_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_TRACEALARMSTATUS_TRACEALARMSTATUS_MASK               0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_TRACEALARMCLR                   0x3020
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_TRACEALARMCLR_TRACEALARMCLR_SHIFT                    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_TRACEALARMCLR_TRACEALARMCLR_MASK                     0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDBASE           0x3080
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDMASK           0x3084
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_ADDRBASE_LOW          0x3088
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_ADDRBASE_HIGH         0x308c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_WINDOWSIZE            0x3090
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_SECURITYBASE          0x3094
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_SECURITYMASK          0x3098
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_OPCODE                0x309c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_STATUS                0x30a0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_LENGTH                0x30a4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_LENGTH_FILTERS_0_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_LENGTH_FILTERS_0_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_URGENCY               0x30a8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_URGENCY_FILTERS_0_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_URGENCY_FILTERS_0_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_USERBASE              0x30ac
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_USERMASK              0x30b0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_USERBASEHIGH          0x30b4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_USERMASKHIGH          0x30b8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDBASE           0x30e0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDMASK           0x30e4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_ADDRBASE_LOW          0x30e8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_ADDRBASE_HIGH         0x30ec
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_WINDOWSIZE            0x30f0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_SECURITYBASE          0x30f4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_SECURITYMASK          0x30f8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_OPCODE                0x30fc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_STATUS                0x3100
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_LENGTH                0x3104
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_LENGTH_FILTERS_1_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_LENGTH_FILTERS_1_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_URGENCY               0x3108
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_URGENCY_FILTERS_1_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_URGENCY_FILTERS_1_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_USERBASE              0x310c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_USERMASK              0x3110
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_USERBASEHIGH          0x3114
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_USERMASKHIGH          0x3118
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDBASE           0x3140
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDMASK           0x3144
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_ADDRBASE_LOW          0x3148
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_ADDRBASE_HIGH         0x314c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_WINDOWSIZE            0x3150
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_SECURITYBASE          0x3154
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_SECURITYMASK          0x3158
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_OPCODE                0x315c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_STATUS                0x3160
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_LENGTH                0x3164
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_LENGTH_FILTERS_2_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_LENGTH_FILTERS_2_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_URGENCY               0x3168
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_URGENCY_FILTERS_2_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_URGENCY_FILTERS_2_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_USERBASE              0x316c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_USERMASK              0x3170
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_USERBASEHIGH          0x3174
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_USERMASKHIGH          0x3178
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDBASE           0x31a0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDMASK           0x31a4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_ADDRBASE_LOW          0x31a8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_ADDRBASE_HIGH         0x31ac
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_WINDOWSIZE            0x31b0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_SECURITYBASE          0x31b4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_SECURITYMASK          0x31b8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_OPCODE                0x31bc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_STATUS                0x31c0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_LENGTH                0x31c4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_LENGTH_FILTERS_3_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_LENGTH_FILTERS_3_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_URGENCY               0x31c8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_URGENCY_FILTERS_3_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_URGENCY_FILTERS_3_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_USERBASE              0x31cc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_USERMASK              0x31d0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_USERBASEHIGH          0x31d4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_USERMASKHIGH          0x31d8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P0_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_ID_COREID                       0x3400
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_ID_COREID_CORETYPEID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_ID_COREID_CORETYPEID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_ID_COREID_CORECHECKSUM_SHIFT                         8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_ID_COREID_CORECHECKSUM_MASK                          0xffffff00
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_ID_REVISIONID                   0x3404
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_ID_REVISIONID_USERID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_ID_REVISIONID_USERID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_ID_REVISIONID_FLEXNOCID_SHIFT                        8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_ID_REVISIONID_FLEXNOCID_MASK                         0xffffff00
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL                         0x3408
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_ERREN_SHIFT                                  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_ERREN_MASK                                   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_TRACEEN_SHIFT                                1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_TRACEEN_MASK                                 0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_PAYLOADEN_SHIFT                              2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_PAYLOADEN_MASK                               0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_STATEN_SHIFT                                 3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_STATEN_MASK                                  0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_ALARMEN_SHIFT                                4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_ALARMEN_MASK                                 0x00000010
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_STATCONDDUMP_SHIFT                           5
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_STATCONDDUMP_MASK                            0x00000020
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_INTRUSIVEMODE_SHIFT                          6
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_INTRUSIVEMODE_MASK                           0x00000040
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_SHIFT              7
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_MASK               0x00000080
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_CFGCTL                          0x340c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_CFGCTL_GLOBALEN_SHIFT                                0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_CFGCTL_GLOBALEN_MASK                                 0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_CFGCTL_ACTIVE_SHIFT                                  1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_CFGCTL_ACTIVE_MASK                                   0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERLUT                       0x3414
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERLUT_FILTERLUT_SHIFT                            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERLUT_FILTERLUT_MASK                             0x0000ffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_TRACEALARMEN                    0x3418
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_TRACEALARMEN_TRACEALARMEN_SHIFT                      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_TRACEALARMEN_TRACEALARMEN_MASK                       0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_TRACEALARMSTATUS                0x341c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_TRACEALARMSTATUS_TRACEALARMSTATUS_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_TRACEALARMSTATUS_TRACEALARMSTATUS_MASK               0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_TRACEALARMCLR                   0x3420
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_TRACEALARMCLR_TRACEALARMCLR_SHIFT                    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_TRACEALARMCLR_TRACEALARMCLR_MASK                     0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDBASE           0x3480
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDMASK           0x3484
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_ADDRBASE_LOW          0x3488
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_ADDRBASE_HIGH         0x348c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_WINDOWSIZE            0x3490
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_SECURITYBASE          0x3494
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_SECURITYMASK          0x3498
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_OPCODE                0x349c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_STATUS                0x34a0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_LENGTH                0x34a4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_LENGTH_FILTERS_0_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_LENGTH_FILTERS_0_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_URGENCY               0x34a8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_URGENCY_FILTERS_0_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_URGENCY_FILTERS_0_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_USERBASE              0x34ac
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_USERMASK              0x34b0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_USERBASEHIGH          0x34b4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_USERMASKHIGH          0x34b8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDBASE           0x34e0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDMASK           0x34e4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_ADDRBASE_LOW          0x34e8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_ADDRBASE_HIGH         0x34ec
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_WINDOWSIZE            0x34f0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_SECURITYBASE          0x34f4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_SECURITYMASK          0x34f8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_OPCODE                0x34fc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_STATUS                0x3500
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_LENGTH                0x3504
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_LENGTH_FILTERS_1_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_LENGTH_FILTERS_1_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_URGENCY               0x3508
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_URGENCY_FILTERS_1_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_URGENCY_FILTERS_1_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_USERBASE              0x350c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_USERMASK              0x3510
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_USERBASEHIGH          0x3514
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_USERMASKHIGH          0x3518
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDBASE           0x3540
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDMASK           0x3544
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_ADDRBASE_LOW          0x3548
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_ADDRBASE_HIGH         0x354c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_WINDOWSIZE            0x3550
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_SECURITYBASE          0x3554
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_SECURITYMASK          0x3558
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_OPCODE                0x355c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_STATUS                0x3560
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_LENGTH                0x3564
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_LENGTH_FILTERS_2_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_LENGTH_FILTERS_2_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_URGENCY               0x3568
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_URGENCY_FILTERS_2_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_URGENCY_FILTERS_2_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_USERBASE              0x356c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_USERMASK              0x3570
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_USERBASEHIGH          0x3574
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_USERMASKHIGH          0x3578
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDBASE           0x35a0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDMASK           0x35a4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_ADDRBASE_LOW          0x35a8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_ADDRBASE_HIGH         0x35ac
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_WINDOWSIZE            0x35b0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_SECURITYBASE          0x35b4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_SECURITYMASK          0x35b8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_OPCODE                0x35bc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_STATUS                0x35c0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_LENGTH                0x35c4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_LENGTH_FILTERS_3_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_LENGTH_FILTERS_3_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_URGENCY               0x35c8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_URGENCY_FILTERS_3_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_URGENCY_FILTERS_3_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_USERBASE              0x35cc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_USERMASK              0x35d0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_USERBASEHIGH          0x35d4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_USERMASKHIGH          0x35d8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P1_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_ID_COREID                       0x3800
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_ID_COREID_CORETYPEID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_ID_COREID_CORETYPEID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_ID_COREID_CORECHECKSUM_SHIFT                         8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_ID_COREID_CORECHECKSUM_MASK                          0xffffff00
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_ID_REVISIONID                   0x3804
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_ID_REVISIONID_USERID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_ID_REVISIONID_USERID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_ID_REVISIONID_FLEXNOCID_SHIFT                        8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_ID_REVISIONID_FLEXNOCID_MASK                         0xffffff00
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL                         0x3808
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_ERREN_SHIFT                                  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_ERREN_MASK                                   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_TRACEEN_SHIFT                                1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_TRACEEN_MASK                                 0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_PAYLOADEN_SHIFT                              2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_PAYLOADEN_MASK                               0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_STATEN_SHIFT                                 3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_STATEN_MASK                                  0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_ALARMEN_SHIFT                                4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_ALARMEN_MASK                                 0x00000010
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_STATCONDDUMP_SHIFT                           5
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_STATCONDDUMP_MASK                            0x00000020
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_INTRUSIVEMODE_SHIFT                          6
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_INTRUSIVEMODE_MASK                           0x00000040
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_SHIFT              7
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_MASK               0x00000080
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_CFGCTL                          0x380c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_CFGCTL_GLOBALEN_SHIFT                                0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_CFGCTL_GLOBALEN_MASK                                 0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_CFGCTL_ACTIVE_SHIFT                                  1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_CFGCTL_ACTIVE_MASK                                   0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERLUT                       0x3814
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERLUT_FILTERLUT_SHIFT                            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERLUT_FILTERLUT_MASK                             0x0000ffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_TRACEALARMEN                    0x3818
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_TRACEALARMEN_TRACEALARMEN_SHIFT                      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_TRACEALARMEN_TRACEALARMEN_MASK                       0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_TRACEALARMSTATUS                0x381c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_TRACEALARMSTATUS_TRACEALARMSTATUS_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_TRACEALARMSTATUS_TRACEALARMSTATUS_MASK               0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_TRACEALARMCLR                   0x3820
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_TRACEALARMCLR_TRACEALARMCLR_SHIFT                    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_TRACEALARMCLR_TRACEALARMCLR_MASK                     0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDBASE           0x3880
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDMASK           0x3884
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_ADDRBASE_LOW          0x3888
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_ADDRBASE_HIGH         0x388c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_WINDOWSIZE            0x3890
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_SECURITYBASE          0x3894
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_SECURITYMASK          0x3898
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_OPCODE                0x389c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_STATUS                0x38a0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_LENGTH                0x38a4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_LENGTH_FILTERS_0_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_LENGTH_FILTERS_0_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_URGENCY               0x38a8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_URGENCY_FILTERS_0_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_URGENCY_FILTERS_0_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_USERBASE              0x38ac
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_USERMASK              0x38b0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_USERBASEHIGH          0x38b4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_USERMASKHIGH          0x38b8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDBASE           0x38e0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDMASK           0x38e4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_ADDRBASE_LOW          0x38e8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_ADDRBASE_HIGH         0x38ec
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_WINDOWSIZE            0x38f0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_SECURITYBASE          0x38f4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_SECURITYMASK          0x38f8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_OPCODE                0x38fc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_STATUS                0x3900
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_LENGTH                0x3904
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_LENGTH_FILTERS_1_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_LENGTH_FILTERS_1_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_URGENCY               0x3908
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_URGENCY_FILTERS_1_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_URGENCY_FILTERS_1_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_USERBASE              0x390c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_USERMASK              0x3910
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_USERBASEHIGH          0x3914
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_USERMASKHIGH          0x3918
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDBASE           0x3940
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDMASK           0x3944
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_ADDRBASE_LOW          0x3948
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_ADDRBASE_HIGH         0x394c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_WINDOWSIZE            0x3950
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_SECURITYBASE          0x3954
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_SECURITYMASK          0x3958
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_OPCODE                0x395c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_STATUS                0x3960
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_LENGTH                0x3964
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_LENGTH_FILTERS_2_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_LENGTH_FILTERS_2_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_URGENCY               0x3968
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_URGENCY_FILTERS_2_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_URGENCY_FILTERS_2_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_USERBASE              0x396c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_USERMASK              0x3970
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_USERBASEHIGH          0x3974
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_USERMASKHIGH          0x3978
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDBASE           0x39a0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDMASK           0x39a4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_ADDRBASE_LOW          0x39a8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_ADDRBASE_HIGH         0x39ac
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_WINDOWSIZE            0x39b0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_SECURITYBASE          0x39b4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_SECURITYMASK          0x39b8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_OPCODE                0x39bc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_STATUS                0x39c0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_LENGTH                0x39c4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_LENGTH_FILTERS_3_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_LENGTH_FILTERS_3_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_URGENCY               0x39c8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_URGENCY_FILTERS_3_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_URGENCY_FILTERS_3_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_USERBASE              0x39cc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_USERMASK              0x39d0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_USERBASEHIGH          0x39d4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_USERMASKHIGH          0x39d8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P2_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_ID_COREID                       0x3c00
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_ID_COREID_CORETYPEID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_ID_COREID_CORETYPEID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_ID_COREID_CORECHECKSUM_SHIFT                         8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_ID_COREID_CORECHECKSUM_MASK                          0xffffff00
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_ID_REVISIONID                   0x3c04
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_ID_REVISIONID_USERID_SHIFT                           0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_ID_REVISIONID_USERID_MASK                            0x000000ff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_ID_REVISIONID_FLEXNOCID_SHIFT                        8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_ID_REVISIONID_FLEXNOCID_MASK                         0xffffff00
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL                         0x3c08
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_ERREN_SHIFT                                  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_ERREN_MASK                                   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_TRACEEN_SHIFT                                1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_TRACEEN_MASK                                 0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_PAYLOADEN_SHIFT                              2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_PAYLOADEN_MASK                               0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_STATEN_SHIFT                                 3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_STATEN_MASK                                  0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_ALARMEN_SHIFT                                4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_ALARMEN_MASK                                 0x00000010
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_STATCONDDUMP_SHIFT                           5
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_STATCONDDUMP_MASK                            0x00000020
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_INTRUSIVEMODE_SHIFT                          6
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_INTRUSIVEMODE_MASK                           0x00000040
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_SHIFT              7
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_MASK               0x00000080
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_CFGCTL                          0x3c0c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_CFGCTL_GLOBALEN_SHIFT                                0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_CFGCTL_GLOBALEN_MASK                                 0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_CFGCTL_ACTIVE_SHIFT                                  1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_CFGCTL_ACTIVE_MASK                                   0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERLUT                       0x3c14
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERLUT_FILTERLUT_SHIFT                            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERLUT_FILTERLUT_MASK                             0x0000ffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_TRACEALARMEN                    0x3c18
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_TRACEALARMEN_TRACEALARMEN_SHIFT                      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_TRACEALARMEN_TRACEALARMEN_MASK                       0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_TRACEALARMSTATUS                0x3c1c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_TRACEALARMSTATUS_TRACEALARMSTATUS_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_TRACEALARMSTATUS_TRACEALARMSTATUS_MASK               0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_TRACEALARMCLR                   0x3c20
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_TRACEALARMCLR_TRACEALARMCLR_SHIFT                    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_TRACEALARMCLR_TRACEALARMCLR_MASK                     0x0000001f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDBASE           0x3c80
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDMASK           0x3c84
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_ADDRBASE_LOW          0x3c88
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_ADDRBASE_HIGH         0x3c8c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_WINDOWSIZE            0x3c90
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_SECURITYBASE          0x3c94
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_SECURITYMASK          0x3c98
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_OPCODE                0x3c9c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_STATUS                0x3ca0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_LENGTH                0x3ca4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_LENGTH_FILTERS_0_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_LENGTH_FILTERS_0_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_URGENCY               0x3ca8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_URGENCY_FILTERS_0_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_URGENCY_FILTERS_0_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_USERBASE              0x3cac
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_USERMASK              0x3cb0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_USERBASEHIGH          0x3cb4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_USERBASEHIGH_FILTERS_0_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_USERMASKHIGH          0x3cb8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_0_USERMASKHIGH_FILTERS_0_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDBASE           0x3ce0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDMASK           0x3ce4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_ADDRBASE_LOW          0x3ce8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_ADDRBASE_HIGH         0x3cec
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_WINDOWSIZE            0x3cf0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_SECURITYBASE          0x3cf4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_SECURITYMASK          0x3cf8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_OPCODE                0x3cfc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_STATUS                0x3d00
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_LENGTH                0x3d04
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_LENGTH_FILTERS_1_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_LENGTH_FILTERS_1_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_URGENCY               0x3d08
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_URGENCY_FILTERS_1_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_URGENCY_FILTERS_1_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_USERBASE              0x3d0c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_USERMASK              0x3d10
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_USERBASEHIGH          0x3d14
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_USERBASEHIGH_FILTERS_1_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_USERMASKHIGH          0x3d18
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_1_USERMASKHIGH_FILTERS_1_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDBASE           0x3d40
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDMASK           0x3d44
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_ADDRBASE_LOW          0x3d48
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_ADDRBASE_HIGH         0x3d4c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_WINDOWSIZE            0x3d50
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_SECURITYBASE          0x3d54
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_SECURITYMASK          0x3d58
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_OPCODE                0x3d5c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_STATUS                0x3d60
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_LENGTH                0x3d64
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_LENGTH_FILTERS_2_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_LENGTH_FILTERS_2_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_URGENCY               0x3d68
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_URGENCY_FILTERS_2_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_URGENCY_FILTERS_2_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_USERBASE              0x3d6c
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_USERMASK              0x3d70
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_USERBASEHIGH          0x3d74
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_USERBASEHIGH_FILTERS_2_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_USERMASKHIGH          0x3d78
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_2_USERMASKHIGH_FILTERS_2_USERMASKHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDBASE           0x3da0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDMASK           0x3da4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_SHIFT    0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_MASK     0x01ffffff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_ADDRBASE_LOW          0x3da8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_ADDRBASE_HIGH         0x3dac
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_SHIFT 0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_MASK  0x0000007f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_WINDOWSIZE            0x3db0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_SHIFT      0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_MASK       0x0000003f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_SECURITYBASE          0x3db4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_SECURITYMASK          0x3db8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_MASK   0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_OPCODE                0x3dbc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_RDEN_SHIFT                          0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_RDEN_MASK                           0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_WREN_SHIFT                          1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_WREN_MASK                           0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_LOCKEN_SHIFT                        2
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_LOCKEN_MASK                         0x00000004
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_URGEN_SHIFT                         3
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_OPCODE_URGEN_MASK                          0x00000008
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_STATUS                0x3dc0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_STATUS_REQEN_SHIFT                         0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_STATUS_REQEN_MASK                          0x00000001
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_STATUS_RSPEN_SHIFT                         1
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_STATUS_RSPEN_MASK                          0x00000002
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_LENGTH                0x3dc4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_LENGTH_FILTERS_3_LENGTH_SHIFT              0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_LENGTH_FILTERS_3_LENGTH_MASK               0x0000000f
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_URGENCY               0x3dc8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_URGENCY_FILTERS_3_URGENCY_SHIFT            0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_URGENCY_FILTERS_3_URGENCY_MASK             0x00000007
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_USERBASE              0x3dcc
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_USERMASK              0x3dd0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_USERBASEHIGH          0x3dd4
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_USERBASEHIGH_FILTERS_3_USERBASEHIGH_MASK   0x00003fff
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_USERMASKHIGH          0x3dd8
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_SHIFT  0
#define MAIN_BUS_TRACEPROBE_WREQ_MAIN2DDR_P3_FILTERS_3_USERMASKHIGH_FILTERS_3_USERMASKHIGH_MASK   0x00003fff


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_err_main_0_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_err_main_0_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int faulten    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_main_0_faulten_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errvld     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_main_0_errvld_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errclr     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_main_0_errclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lock       :  1;
        unsigned int opc        :  4;
        unsigned int reserved_0 :  3;
        unsigned int errcode    :  3;
        unsigned int reserved_1 :  5;
        unsigned int len1       :  7;
        unsigned int reserved_2 :  8;
        unsigned int format     :  1;
    } reg;
}main_bus_err_main_0_errlog0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog1    : 25;
        unsigned int reserved_0 :  7;
    } reg;
}main_bus_err_main_0_errlog1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog3    : 32;
    } reg;
}main_bus_err_main_0_errlog3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog4    :  7;
        unsigned int reserved_0 : 25;
    } reg;
}main_bus_err_main_0_errlog4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog5    : 32;
    } reg;
}main_bus_err_main_0_errlog5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog6    : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_err_main_0_errlog6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog7    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_main_0_errlog7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int stallen    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_main_0_stallen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_err_cpu_0_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_err_cpu_0_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int faulten    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_cpu_0_faulten_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errvld     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_cpu_0_errvld_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errclr     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_cpu_0_errclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lock       :  1;
        unsigned int opc        :  4;
        unsigned int reserved_0 :  3;
        unsigned int errcode    :  3;
        unsigned int reserved_1 :  5;
        unsigned int len1       :  7;
        unsigned int reserved_2 :  8;
        unsigned int format     :  1;
    } reg;
}main_bus_err_cpu_0_errlog0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog1    : 25;
        unsigned int reserved_0 :  7;
    } reg;
}main_bus_err_cpu_0_errlog1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog3    : 32;
    } reg;
}main_bus_err_cpu_0_errlog3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog4    :  7;
        unsigned int reserved_0 : 25;
    } reg;
}main_bus_err_cpu_0_errlog4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog5    : 32;
    } reg;
}main_bus_err_cpu_0_errlog5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog6    : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_err_cpu_0_errlog6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog7    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_cpu_0_errlog7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int stallen    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_cpu_0_stallen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_err_gpu_0_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_err_gpu_0_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int faulten    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_gpu_0_faulten_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errvld     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_gpu_0_errvld_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errclr     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_gpu_0_errclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lock       :  1;
        unsigned int opc        :  4;
        unsigned int reserved_0 :  3;
        unsigned int errcode    :  3;
        unsigned int reserved_1 :  5;
        unsigned int len1       :  7;
        unsigned int reserved_2 :  8;
        unsigned int format     :  1;
    } reg;
}main_bus_err_gpu_0_errlog0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog1    : 25;
        unsigned int reserved_0 :  7;
    } reg;
}main_bus_err_gpu_0_errlog1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog3    : 32;
    } reg;
}main_bus_err_gpu_0_errlog3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog4    :  7;
        unsigned int reserved_0 : 25;
    } reg;
}main_bus_err_gpu_0_errlog4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog5    : 32;
    } reg;
}main_bus_err_gpu_0_errlog5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog6    : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_err_gpu_0_errlog6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog7    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_gpu_0_errlog7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int stallen    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_err_gpu_0_stallen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_cpu2main_p0_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_cpu2main_p0_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_cpu2main_p0_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_cpu2main_p0_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_cpu2main_p0_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_cpu2main_p0_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_cpu2main_p0_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_cpu2main_p0_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_cpu2main_p0_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_cpu2main_p0_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_cpu2main_p0_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_cpu2main_p0_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_cpu2main_p0_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_cpu2main_p0_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_cpu2main_p1_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_cpu2main_p1_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_cpu2main_p1_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_cpu2main_p1_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_cpu2main_p1_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_cpu2main_p1_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_cpu2main_p1_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_cpu2main_p1_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_cpu2main_p1_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_cpu2main_p1_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_cpu2main_p1_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_cpu2main_p1_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_cpu2main_p1_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_cpu2main_p1_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_cpu2main_p2_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_cpu2main_p2_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_cpu2main_p2_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_cpu2main_p2_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_cpu2main_p2_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_cpu2main_p2_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_cpu2main_p2_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_cpu2main_p2_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_cpu2main_p2_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_cpu2main_p2_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_cpu2main_p2_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_cpu2main_p2_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_cpu2main_p2_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_cpu2main_p2_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_cpu2main_p3_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_cpu2main_p3_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_cpu2main_p3_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_cpu2main_p3_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_cpu2main_p3_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_cpu2main_p3_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_cpu2main_p3_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_cpu2main_p3_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_cpu2main_p3_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_cpu2main_p3_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_cpu2main_p3_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_cpu2main_p3_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_cpu2main_p3_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_cpu2main_p3_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_gpu2main_p0_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_gpu2main_p0_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_gpu2main_p0_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_gpu2main_p0_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_gpu2main_p0_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_gpu2main_p0_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_gpu2main_p0_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_gpu2main_p0_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_gpu2main_p0_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_gpu2main_p0_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_gpu2main_p0_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_gpu2main_p0_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_gpu2main_p0_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_gpu2main_p0_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_gpu2main_p1_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_gpu2main_p1_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_gpu2main_p1_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_gpu2main_p1_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_gpu2main_p1_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_gpu2main_p1_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_gpu2main_p1_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_gpu2main_p1_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_gpu2main_p1_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_gpu2main_p1_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_gpu2main_p1_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_gpu2main_p1_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_gpu2main_p1_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_gpu2main_p1_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_gpu2main_p2_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_gpu2main_p2_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_gpu2main_p2_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_gpu2main_p2_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_gpu2main_p2_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_gpu2main_p2_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_gpu2main_p2_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_gpu2main_p2_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_gpu2main_p2_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_gpu2main_p2_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_gpu2main_p2_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_gpu2main_p2_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_gpu2main_p2_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_gpu2main_p2_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_gpu2main_p3_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_gpu2main_p3_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_gpu2main_p3_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_gpu2main_p3_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_gpu2main_p3_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_gpu2main_p3_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_gpu2main_p3_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_gpu2main_p3_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_gpu2main_p3_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_gpu2main_p3_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_gpu2main_p3_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_gpu2main_p3_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_gpu2main_p3_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_gpu2main_p3_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_m1_main_p0_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_m1_main_p0_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_m1_main_p0_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_m1_main_p0_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_m1_main_p0_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_m1_main_p0_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_m1_main_p0_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_m1_main_p0_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_m1_main_p0_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_m1_main_p0_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_m1_main_p0_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_m1_main_p0_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_m1_main_p0_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_m1_main_p0_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_m1_main_p1_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_m1_main_p1_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_m1_main_p1_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_m1_main_p1_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_m1_main_p1_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_m1_main_p1_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_m1_main_p1_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_m1_main_p1_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_m1_main_p1_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_m1_main_p1_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_m1_main_p1_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_m1_main_p1_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_m1_main_p1_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_m1_main_p1_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_m2_main_p0_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_m2_main_p0_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_m2_main_p0_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_m2_main_p0_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_m2_main_p0_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_m2_main_p0_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_m2_main_p0_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_m2_main_p0_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_m2_main_p0_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_m2_main_p0_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_m2_main_p0_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_m2_main_p0_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_m2_main_p0_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_m2_main_p0_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_m2_main_p1_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_m2_main_p1_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_m2_main_p1_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_m2_main_p1_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_m2_main_p1_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_m2_main_p1_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_m2_main_p1_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_m2_main_p1_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_m2_main_p1_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_m2_main_p1_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_m2_main_p1_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_m2_main_p1_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_m2_main_p1_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_m2_main_p1_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_npu2main_p0_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_npu2main_p0_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_npu2main_p0_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_npu2main_p0_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_npu2main_p0_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_npu2main_p0_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_npu2main_p0_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_npu2main_p0_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_npu2main_p0_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_npu2main_p0_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_npu2main_p0_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_npu2main_p0_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_npu2main_p0_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_npu2main_p0_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_npu2main_p1_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_npu2main_p1_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_npu2main_p1_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_npu2main_p1_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_npu2main_p1_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_npu2main_p1_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_npu2main_p1_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_npu2main_p1_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_npu2main_p1_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_npu2main_p1_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_npu2main_p1_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_npu2main_p1_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_npu2main_p1_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_npu2main_p1_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_npu2main_p2_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_npu2main_p2_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_npu2main_p2_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_npu2main_p2_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_npu2main_p2_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_npu2main_p2_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_npu2main_p2_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_npu2main_p2_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_npu2main_p2_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_npu2main_p2_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_npu2main_p2_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_npu2main_p2_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_npu2main_p2_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_npu2main_p2_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_npu2main_p3_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_npu2main_p3_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_npu2main_p3_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_npu2main_p3_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_npu2main_p3_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_npu2main_p3_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_npu2main_p3_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_npu2main_p3_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_npu2main_p3_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_npu2main_p3_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_npu2main_p3_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_npu2main_p3_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_npu2main_p3_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_npu2main_p3_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_sys1_main_rqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_sys1_main_rqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_sys1_main_rqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_sys1_main_rqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_sys1_main_rqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_sys1_main_rqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_sys1_main_rqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_sys1_main_wqos_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_sys1_main_wqos_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int p0         :  3;
        unsigned int reserved_0 :  5;
        unsigned int p1         :  3;
        unsigned int reserved_1 : 21;
    } reg;
}main_bus_sys1_main_wqos_priority_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mode       :  2;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_sys1_main_wqos_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int bandwidth  : 13;
        unsigned int reserved_0 : 19;
    } reg;
}main_bus_sys1_main_wqos_bandwidth_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int saturation : 10;
        unsigned int reserved_0 : 22;
    } reg;
}main_bus_sys1_main_wqos_saturation_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int socketqosen :  1;
        unsigned int extthren    :  1;
        unsigned int intclken    :  1;
        unsigned int extlimiten  :  1;
        unsigned int reserved_0  : 28;
    } reg;
}main_bus_sys1_main_wqos_extcontrol_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_trace_0_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_trace_0_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int faulten    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_trace_0_faulten_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errvld     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_trace_0_errvld_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errclr     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_trace_0_errclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lock       :  1;
        unsigned int opc        :  4;
        unsigned int reserved_0 :  3;
        unsigned int errcode    :  3;
        unsigned int reserved_1 :  5;
        unsigned int len1       :  7;
        unsigned int reserved_2 :  8;
        unsigned int format     :  1;
    } reg;
}main_bus_trace_0_errlog0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog1    : 25;
        unsigned int reserved_0 :  7;
    } reg;
}main_bus_trace_0_errlog1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog3    : 32;
    } reg;
}main_bus_trace_0_errlog3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog4    :  7;
        unsigned int reserved_0 : 25;
    } reg;
}main_bus_trace_0_errlog4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog5    : 32;
    } reg;
}main_bus_trace_0_errlog5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog6    : 14;
        unsigned int reserved_0 : 18;
    } reg;
}main_bus_trace_0_errlog6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog7    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_trace_0_errlog7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int stallen    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_trace_0_stallen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_atb_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_atb_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int atbid      :  7;
        unsigned int reserved_0 : 25;
    } reg;
}main_bus_atb_atbid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int atben      :  1;
        unsigned int reserved_0 : 31;
    } reg;
}main_bus_atb_atben_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int syncperiod :  5;
        unsigned int reserved_0 : 27;
    } reg;
}main_bus_atb_syncperiod_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int erren                     :  1;
        unsigned int traceen                   :  1;
        unsigned int payloaden                 :  1;
        unsigned int staten                    :  1;
        unsigned int alarmen                   :  1;
        unsigned int statconddump              :  1;
        unsigned int intrusivemode             :  1;
        unsigned int filtbytealwayschainableen :  1;
        unsigned int reserved_0                : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_mainctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int globalen   :  1;
        unsigned int active     :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_cfgctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filterlut  : 16;
        unsigned int reserved_0 : 16;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filterlut_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmen :  5;
        unsigned int reserved_0   : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_tracealarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmstatus :  5;
        unsigned int reserved_0       : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_tracealarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmclr :  5;
        unsigned int reserved_0    : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_tracealarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_0_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_1_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_2_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p0_filters_3_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int erren                     :  1;
        unsigned int traceen                   :  1;
        unsigned int payloaden                 :  1;
        unsigned int staten                    :  1;
        unsigned int alarmen                   :  1;
        unsigned int statconddump              :  1;
        unsigned int intrusivemode             :  1;
        unsigned int filtbytealwayschainableen :  1;
        unsigned int reserved_0                : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_mainctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int globalen   :  1;
        unsigned int active     :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_cfgctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filterlut  : 16;
        unsigned int reserved_0 : 16;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filterlut_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmen :  5;
        unsigned int reserved_0   : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_tracealarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmstatus :  5;
        unsigned int reserved_0       : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_tracealarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmclr :  5;
        unsigned int reserved_0    : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_tracealarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_0_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_1_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_2_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p1_filters_3_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int erren                     :  1;
        unsigned int traceen                   :  1;
        unsigned int payloaden                 :  1;
        unsigned int staten                    :  1;
        unsigned int alarmen                   :  1;
        unsigned int statconddump              :  1;
        unsigned int intrusivemode             :  1;
        unsigned int filtbytealwayschainableen :  1;
        unsigned int reserved_0                : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_mainctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int globalen   :  1;
        unsigned int active     :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_cfgctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filterlut  : 16;
        unsigned int reserved_0 : 16;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filterlut_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmen :  5;
        unsigned int reserved_0   : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_tracealarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmstatus :  5;
        unsigned int reserved_0       : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_tracealarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmclr :  5;
        unsigned int reserved_0    : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_tracealarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_0_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_1_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_2_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p2_filters_3_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int erren                     :  1;
        unsigned int traceen                   :  1;
        unsigned int payloaden                 :  1;
        unsigned int staten                    :  1;
        unsigned int alarmen                   :  1;
        unsigned int statconddump              :  1;
        unsigned int intrusivemode             :  1;
        unsigned int filtbytealwayschainableen :  1;
        unsigned int reserved_0                : 24;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_mainctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int globalen   :  1;
        unsigned int active     :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_cfgctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filterlut  : 16;
        unsigned int reserved_0 : 16;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filterlut_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmen :  5;
        unsigned int reserved_0   : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_tracealarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmstatus :  5;
        unsigned int reserved_0       : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_tracealarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmclr :  5;
        unsigned int reserved_0    : 27;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_tracealarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_0_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_1_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_2_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbase : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermask : 32;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_rreq_main2ddr_p3_filters_3_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int erren                     :  1;
        unsigned int traceen                   :  1;
        unsigned int payloaden                 :  1;
        unsigned int staten                    :  1;
        unsigned int alarmen                   :  1;
        unsigned int statconddump              :  1;
        unsigned int intrusivemode             :  1;
        unsigned int filtbytealwayschainableen :  1;
        unsigned int reserved_0                : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_mainctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int globalen   :  1;
        unsigned int active     :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_cfgctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filterlut  : 16;
        unsigned int reserved_0 : 16;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filterlut_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmen :  5;
        unsigned int reserved_0   : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_tracealarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmstatus :  5;
        unsigned int reserved_0       : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_tracealarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmclr :  5;
        unsigned int reserved_0    : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_tracealarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_0_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_1_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_2_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p0_filters_3_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int erren                     :  1;
        unsigned int traceen                   :  1;
        unsigned int payloaden                 :  1;
        unsigned int staten                    :  1;
        unsigned int alarmen                   :  1;
        unsigned int statconddump              :  1;
        unsigned int intrusivemode             :  1;
        unsigned int filtbytealwayschainableen :  1;
        unsigned int reserved_0                : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_mainctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int globalen   :  1;
        unsigned int active     :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_cfgctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filterlut  : 16;
        unsigned int reserved_0 : 16;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filterlut_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmen :  5;
        unsigned int reserved_0   : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_tracealarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmstatus :  5;
        unsigned int reserved_0       : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_tracealarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmclr :  5;
        unsigned int reserved_0    : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_tracealarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_0_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_1_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_2_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p1_filters_3_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int erren                     :  1;
        unsigned int traceen                   :  1;
        unsigned int payloaden                 :  1;
        unsigned int staten                    :  1;
        unsigned int alarmen                   :  1;
        unsigned int statconddump              :  1;
        unsigned int intrusivemode             :  1;
        unsigned int filtbytealwayschainableen :  1;
        unsigned int reserved_0                : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_mainctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int globalen   :  1;
        unsigned int active     :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_cfgctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filterlut  : 16;
        unsigned int reserved_0 : 16;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filterlut_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmen :  5;
        unsigned int reserved_0   : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_tracealarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmstatus :  5;
        unsigned int reserved_0       : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_tracealarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmclr :  5;
        unsigned int reserved_0    : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_tracealarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_0_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_1_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_2_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p2_filters_3_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int erren                     :  1;
        unsigned int traceen                   :  1;
        unsigned int payloaden                 :  1;
        unsigned int staten                    :  1;
        unsigned int alarmen                   :  1;
        unsigned int statconddump              :  1;
        unsigned int intrusivemode             :  1;
        unsigned int filtbytealwayschainableen :  1;
        unsigned int reserved_0                : 24;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_mainctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int globalen   :  1;
        unsigned int active     :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_cfgctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filterlut  : 16;
        unsigned int reserved_0 : 16;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filterlut_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmen :  5;
        unsigned int reserved_0   : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_tracealarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmstatus :  5;
        unsigned int reserved_0       : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_tracealarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmclr :  5;
        unsigned int reserved_0    : 27;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_tracealarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_0_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_1_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_2_usermaskhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_low : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbase : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermask : 32;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbasehigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_userbasehigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermaskhigh : 14;
        unsigned int reserved_0             : 18;
    } reg;
}main_bus_traceprobe_wreq_main2ddr_p3_filters_3_usermaskhigh_t;

#endif
