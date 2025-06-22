// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef NPU_BUS_REGIF_H
#define NPU_BUS_REGIF_H

#define NPU_BUS_ERR_NPU_0_ID_COREID                         0x0000
#define NPU_BUS_ERR_NPU_0_ID_COREID_CORETYPEID_SHIFT                              0
#define NPU_BUS_ERR_NPU_0_ID_COREID_CORETYPEID_MASK                               0x000000ff
#define NPU_BUS_ERR_NPU_0_ID_COREID_CORECHECKSUM_SHIFT                            8
#define NPU_BUS_ERR_NPU_0_ID_COREID_CORECHECKSUM_MASK                             0xffffff00
#define NPU_BUS_ERR_NPU_0_ID_REVISIONID                     0x0004
#define NPU_BUS_ERR_NPU_0_ID_REVISIONID_USERID_SHIFT                              0
#define NPU_BUS_ERR_NPU_0_ID_REVISIONID_USERID_MASK                               0x000000ff
#define NPU_BUS_ERR_NPU_0_ID_REVISIONID_FLEXNOCID_SHIFT                           8
#define NPU_BUS_ERR_NPU_0_ID_REVISIONID_FLEXNOCID_MASK                            0xffffff00
#define NPU_BUS_ERR_NPU_0_FAULTEN                           0x0008
#define NPU_BUS_ERR_NPU_0_FAULTEN_FAULTEN_SHIFT                                   0
#define NPU_BUS_ERR_NPU_0_FAULTEN_FAULTEN_MASK                                    0x00000001
#define NPU_BUS_ERR_NPU_0_ERRVLD                            0x000c
#define NPU_BUS_ERR_NPU_0_ERRVLD_ERRVLD_SHIFT                                     0
#define NPU_BUS_ERR_NPU_0_ERRVLD_ERRVLD_MASK                                      0x00000001
#define NPU_BUS_ERR_NPU_0_ERRCLR                            0x0010
#define NPU_BUS_ERR_NPU_0_ERRCLR_ERRCLR_SHIFT                                     0
#define NPU_BUS_ERR_NPU_0_ERRCLR_ERRCLR_MASK                                      0x00000001
#define NPU_BUS_ERR_NPU_0_ERRLOG0                           0x0014
#define NPU_BUS_ERR_NPU_0_ERRLOG0_LOCK_SHIFT                                      0
#define NPU_BUS_ERR_NPU_0_ERRLOG0_LOCK_MASK                                       0x00000001
#define NPU_BUS_ERR_NPU_0_ERRLOG0_OPC_SHIFT                                       1
#define NPU_BUS_ERR_NPU_0_ERRLOG0_OPC_MASK                                        0x0000001e
#define NPU_BUS_ERR_NPU_0_ERRLOG0_ERRCODE_SHIFT                                   8
#define NPU_BUS_ERR_NPU_0_ERRLOG0_ERRCODE_MASK                                    0x00000700
#define NPU_BUS_ERR_NPU_0_ERRLOG0_LEN1_SHIFT                                      16
#define NPU_BUS_ERR_NPU_0_ERRLOG0_LEN1_MASK                                       0x01ff0000
#define NPU_BUS_ERR_NPU_0_ERRLOG0_FORMAT_SHIFT                                    31
#define NPU_BUS_ERR_NPU_0_ERRLOG0_FORMAT_MASK                                     0x80000000
#define NPU_BUS_ERR_NPU_0_ERRLOG1                           0x0018
#define NPU_BUS_ERR_NPU_0_ERRLOG1_ERRLOG1_SHIFT                                   0
#define NPU_BUS_ERR_NPU_0_ERRLOG1_ERRLOG1_MASK                                    0x003fffff
#define NPU_BUS_ERR_NPU_0_ERRLOG3                           0x0020
#define NPU_BUS_ERR_NPU_0_ERRLOG4                           0x0024
#define NPU_BUS_ERR_NPU_0_ERRLOG4_ERRLOG4_SHIFT                                   0
#define NPU_BUS_ERR_NPU_0_ERRLOG4_ERRLOG4_MASK                                    0x0000007f
#define NPU_BUS_ERR_NPU_0_ERRLOG5                           0x0028
#define NPU_BUS_ERR_NPU_0_ERRLOG5_ERRLOG5_SHIFT                                   0
#define NPU_BUS_ERR_NPU_0_ERRLOG5_ERRLOG5_MASK                                    0x001fffff
#define NPU_BUS_ERR_NPU_0_ERRLOG7                           0x0030
#define NPU_BUS_ERR_NPU_0_ERRLOG7_ERRLOG7_SHIFT                                   0
#define NPU_BUS_ERR_NPU_0_ERRLOG7_ERRLOG7_MASK                                    0x00000001
#define NPU_BUS_ERR_NPU_0_STALLEN                           0x004c
#define NPU_BUS_ERR_NPU_0_STALLEN_STALLEN_SHIFT                                   0
#define NPU_BUS_ERR_NPU_0_STALLEN_STALLEN_MASK                                    0x00000001
#define NPU_BUS_ERR_VDSP_0_ID_COREID                        0x0080
#define NPU_BUS_ERR_VDSP_0_ID_COREID_CORETYPEID_SHIFT                             0
#define NPU_BUS_ERR_VDSP_0_ID_COREID_CORETYPEID_MASK                              0x000000ff
#define NPU_BUS_ERR_VDSP_0_ID_COREID_CORECHECKSUM_SHIFT                           8
#define NPU_BUS_ERR_VDSP_0_ID_COREID_CORECHECKSUM_MASK                            0xffffff00
#define NPU_BUS_ERR_VDSP_0_ID_REVISIONID                    0x0084
#define NPU_BUS_ERR_VDSP_0_ID_REVISIONID_USERID_SHIFT                             0
#define NPU_BUS_ERR_VDSP_0_ID_REVISIONID_USERID_MASK                              0x000000ff
#define NPU_BUS_ERR_VDSP_0_ID_REVISIONID_FLEXNOCID_SHIFT                          8
#define NPU_BUS_ERR_VDSP_0_ID_REVISIONID_FLEXNOCID_MASK                           0xffffff00
#define NPU_BUS_ERR_VDSP_0_FAULTEN                          0x0088
#define NPU_BUS_ERR_VDSP_0_FAULTEN_FAULTEN_SHIFT                                  0
#define NPU_BUS_ERR_VDSP_0_FAULTEN_FAULTEN_MASK                                   0x00000001
#define NPU_BUS_ERR_VDSP_0_ERRVLD                           0x008c
#define NPU_BUS_ERR_VDSP_0_ERRVLD_ERRVLD_SHIFT                                    0
#define NPU_BUS_ERR_VDSP_0_ERRVLD_ERRVLD_MASK                                     0x00000001
#define NPU_BUS_ERR_VDSP_0_ERRCLR                           0x0090
#define NPU_BUS_ERR_VDSP_0_ERRCLR_ERRCLR_SHIFT                                    0
#define NPU_BUS_ERR_VDSP_0_ERRCLR_ERRCLR_MASK                                     0x00000001
#define NPU_BUS_ERR_VDSP_0_ERRLOG0                          0x0094
#define NPU_BUS_ERR_VDSP_0_ERRLOG0_LOCK_SHIFT                                     0
#define NPU_BUS_ERR_VDSP_0_ERRLOG0_LOCK_MASK                                      0x00000001
#define NPU_BUS_ERR_VDSP_0_ERRLOG0_OPC_SHIFT                                      1
#define NPU_BUS_ERR_VDSP_0_ERRLOG0_OPC_MASK                                       0x0000001e
#define NPU_BUS_ERR_VDSP_0_ERRLOG0_ERRCODE_SHIFT                                  8
#define NPU_BUS_ERR_VDSP_0_ERRLOG0_ERRCODE_MASK                                   0x00000700
#define NPU_BUS_ERR_VDSP_0_ERRLOG0_LEN1_SHIFT                                     16
#define NPU_BUS_ERR_VDSP_0_ERRLOG0_LEN1_MASK                                      0x01ff0000
#define NPU_BUS_ERR_VDSP_0_ERRLOG0_FORMAT_SHIFT                                   31
#define NPU_BUS_ERR_VDSP_0_ERRLOG0_FORMAT_MASK                                    0x80000000
#define NPU_BUS_ERR_VDSP_0_ERRLOG1                          0x0098
#define NPU_BUS_ERR_VDSP_0_ERRLOG1_ERRLOG1_SHIFT                                  0
#define NPU_BUS_ERR_VDSP_0_ERRLOG1_ERRLOG1_MASK                                   0x003fffff
#define NPU_BUS_ERR_VDSP_0_ERRLOG3                          0x00a0
#define NPU_BUS_ERR_VDSP_0_ERRLOG4                          0x00a4
#define NPU_BUS_ERR_VDSP_0_ERRLOG4_ERRLOG4_SHIFT                                  0
#define NPU_BUS_ERR_VDSP_0_ERRLOG4_ERRLOG4_MASK                                   0x0000007f
#define NPU_BUS_ERR_VDSP_0_ERRLOG5                          0x00a8
#define NPU_BUS_ERR_VDSP_0_ERRLOG5_ERRLOG5_SHIFT                                  0
#define NPU_BUS_ERR_VDSP_0_ERRLOG5_ERRLOG5_MASK                                   0x001fffff
#define NPU_BUS_ERR_VDSP_0_ERRLOG7                          0x00b0
#define NPU_BUS_ERR_VDSP_0_ERRLOG7_ERRLOG7_SHIFT                                  0
#define NPU_BUS_ERR_VDSP_0_ERRLOG7_ERRLOG7_MASK                                   0x00000001
#define NPU_BUS_ERR_VDSP_0_STALLEN                          0x00cc
#define NPU_BUS_ERR_VDSP_0_STALLEN_STALLEN_SHIFT                                  0
#define NPU_BUS_ERR_VDSP_0_STALLEN_STALLEN_MASK                                   0x00000001
#define NPU_BUS_PACKET_PROBE_ID_COREID                      0x0400
#define NPU_BUS_PACKET_PROBE_ID_COREID_CORETYPEID_SHIFT                           0
#define NPU_BUS_PACKET_PROBE_ID_COREID_CORETYPEID_MASK                            0x000000ff
#define NPU_BUS_PACKET_PROBE_ID_COREID_CORECHECKSUM_SHIFT                         8
#define NPU_BUS_PACKET_PROBE_ID_COREID_CORECHECKSUM_MASK                          0xffffff00
#define NPU_BUS_PACKET_PROBE_ID_REVISIONID                  0x0404
#define NPU_BUS_PACKET_PROBE_ID_REVISIONID_USERID_SHIFT                           0
#define NPU_BUS_PACKET_PROBE_ID_REVISIONID_USERID_MASK                            0x000000ff
#define NPU_BUS_PACKET_PROBE_ID_REVISIONID_FLEXNOCID_SHIFT                        8
#define NPU_BUS_PACKET_PROBE_ID_REVISIONID_FLEXNOCID_MASK                         0xffffff00
#define NPU_BUS_PACKET_PROBE_MAINCTL                        0x0408
#define NPU_BUS_PACKET_PROBE_MAINCTL_ERREN_SHIFT                                  0
#define NPU_BUS_PACKET_PROBE_MAINCTL_ERREN_MASK                                   0x00000001
#define NPU_BUS_PACKET_PROBE_MAINCTL_TRACEEN_SHIFT                                1
#define NPU_BUS_PACKET_PROBE_MAINCTL_TRACEEN_MASK                                 0x00000002
#define NPU_BUS_PACKET_PROBE_MAINCTL_PAYLOADEN_SHIFT                              2
#define NPU_BUS_PACKET_PROBE_MAINCTL_PAYLOADEN_MASK                               0x00000004
#define NPU_BUS_PACKET_PROBE_MAINCTL_STATEN_SHIFT                                 3
#define NPU_BUS_PACKET_PROBE_MAINCTL_STATEN_MASK                                  0x00000008
#define NPU_BUS_PACKET_PROBE_MAINCTL_ALARMEN_SHIFT                                4
#define NPU_BUS_PACKET_PROBE_MAINCTL_ALARMEN_MASK                                 0x00000010
#define NPU_BUS_PACKET_PROBE_MAINCTL_STATCONDDUMP_SHIFT                           5
#define NPU_BUS_PACKET_PROBE_MAINCTL_STATCONDDUMP_MASK                            0x00000020
#define NPU_BUS_PACKET_PROBE_MAINCTL_INTRUSIVEMODE_SHIFT                          6
#define NPU_BUS_PACKET_PROBE_MAINCTL_INTRUSIVEMODE_MASK                           0x00000040
#define NPU_BUS_PACKET_PROBE_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_SHIFT              7
#define NPU_BUS_PACKET_PROBE_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_MASK               0x00000080
#define NPU_BUS_PACKET_PROBE_CFGCTL                         0x040c
#define NPU_BUS_PACKET_PROBE_CFGCTL_GLOBALEN_SHIFT                                0
#define NPU_BUS_PACKET_PROBE_CFGCTL_GLOBALEN_MASK                                 0x00000001
#define NPU_BUS_PACKET_PROBE_CFGCTL_ACTIVE_SHIFT                                  1
#define NPU_BUS_PACKET_PROBE_CFGCTL_ACTIVE_MASK                                   0x00000002
#define NPU_BUS_PACKET_PROBE_TRACEPORTSEL                   0x0410
#define NPU_BUS_PACKET_PROBE_TRACEPORTSEL_TRACEPORTSEL_SHIFT                      0
#define NPU_BUS_PACKET_PROBE_TRACEPORTSEL_TRACEPORTSEL_MASK                       0x00000001
#define NPU_BUS_PACKET_PROBE_FILTERLUT                      0x0414
#define NPU_BUS_PACKET_PROBE_FILTERLUT_FILTERLUT_SHIFT                            0
#define NPU_BUS_PACKET_PROBE_FILTERLUT_FILTERLUT_MASK                             0x0000000f
#define NPU_BUS_PACKET_PROBE_TRACEALARMEN                   0x0418
#define NPU_BUS_PACKET_PROBE_TRACEALARMEN_TRACEALARMEN_SHIFT                      0
#define NPU_BUS_PACKET_PROBE_TRACEALARMEN_TRACEALARMEN_MASK                       0x00000007
#define NPU_BUS_PACKET_PROBE_TRACEALARMSTATUS               0x041c
#define NPU_BUS_PACKET_PROBE_TRACEALARMSTATUS_TRACEALARMSTATUS_SHIFT              0
#define NPU_BUS_PACKET_PROBE_TRACEALARMSTATUS_TRACEALARMSTATUS_MASK               0x00000007
#define NPU_BUS_PACKET_PROBE_TRACEALARMCLR                  0x0420
#define NPU_BUS_PACKET_PROBE_TRACEALARMCLR_TRACEALARMCLR_SHIFT                    0
#define NPU_BUS_PACKET_PROBE_TRACEALARMCLR_TRACEALARMCLR_MASK                     0x00000007
#define NPU_BUS_PACKET_PROBE_STATPERIOD                     0x0424
#define NPU_BUS_PACKET_PROBE_STATPERIOD_STATPERIOD_SHIFT                          0
#define NPU_BUS_PACKET_PROBE_STATPERIOD_STATPERIOD_MASK                           0x0000001f
#define NPU_BUS_PACKET_PROBE_STATGO                         0x0428
#define NPU_BUS_PACKET_PROBE_STATGO_STATGO_SHIFT                                  0
#define NPU_BUS_PACKET_PROBE_STATGO_STATGO_MASK                                   0x00000001
#define NPU_BUS_PACKET_PROBE_STATALARMMIN                   0x042c
#define NPU_BUS_PACKET_PROBE_STATALARMMINHIGH               0x0430
#define NPU_BUS_PACKET_PROBE_STATALARMMAX                   0x0434
#define NPU_BUS_PACKET_PROBE_STATALARMMAXHIGH               0x0438
#define NPU_BUS_PACKET_PROBE_STATALARMSTATUS                0x043c
#define NPU_BUS_PACKET_PROBE_STATALARMSTATUS_STATALARMSTATUS_SHIFT                0
#define NPU_BUS_PACKET_PROBE_STATALARMSTATUS_STATALARMSTATUS_MASK                 0x00000001
#define NPU_BUS_PACKET_PROBE_STATALARMCLR                   0x0440
#define NPU_BUS_PACKET_PROBE_STATALARMCLR_STATALARMCLR_SHIFT                      0
#define NPU_BUS_PACKET_PROBE_STATALARMCLR_STATALARMCLR_MASK                       0x00000001
#define NPU_BUS_PACKET_PROBE_STATALARMEN                    0x0444
#define NPU_BUS_PACKET_PROBE_STATALARMEN_STATALARMEN_SHIFT                        0
#define NPU_BUS_PACKET_PROBE_STATALARMEN_STATALARMEN_MASK                         0x00000001
#define NPU_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDBASE          0x0480
#define NPU_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_SHIFT    0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_MASK     0x003fffff
#define NPU_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDMASK          0x0484
#define NPU_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_SHIFT    0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_MASK     0x003fffff
#define NPU_BUS_PACKET_PROBE_FILTERS_0_ADDRBASE_LOW         0x0488
#define NPU_BUS_PACKET_PROBE_FILTERS_0_ADDRBASE_HIGH        0x048c
#define NPU_BUS_PACKET_PROBE_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_SHIFT 0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_MASK  0x0000007f
#define NPU_BUS_PACKET_PROBE_FILTERS_0_WINDOWSIZE           0x0490
#define NPU_BUS_PACKET_PROBE_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_SHIFT      0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_MASK       0x0000003f
#define NPU_BUS_PACKET_PROBE_FILTERS_0_SECURITYBASE         0x0494
#define NPU_BUS_PACKET_PROBE_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_SHIFT  0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_MASK   0x00000001
#define NPU_BUS_PACKET_PROBE_FILTERS_0_SECURITYMASK         0x0498
#define NPU_BUS_PACKET_PROBE_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_SHIFT  0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_MASK   0x00000001
#define NPU_BUS_PACKET_PROBE_FILTERS_0_OPCODE               0x049c
#define NPU_BUS_PACKET_PROBE_FILTERS_0_OPCODE_RDEN_SHIFT                          0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_OPCODE_RDEN_MASK                           0x00000001
#define NPU_BUS_PACKET_PROBE_FILTERS_0_OPCODE_WREN_SHIFT                          1
#define NPU_BUS_PACKET_PROBE_FILTERS_0_OPCODE_WREN_MASK                           0x00000002
#define NPU_BUS_PACKET_PROBE_FILTERS_0_OPCODE_LOCKEN_SHIFT                        2
#define NPU_BUS_PACKET_PROBE_FILTERS_0_OPCODE_LOCKEN_MASK                         0x00000004
#define NPU_BUS_PACKET_PROBE_FILTERS_0_OPCODE_URGEN_SHIFT                         3
#define NPU_BUS_PACKET_PROBE_FILTERS_0_OPCODE_URGEN_MASK                          0x00000008
#define NPU_BUS_PACKET_PROBE_FILTERS_0_STATUS               0x04a0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_STATUS_REQEN_SHIFT                         0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_STATUS_REQEN_MASK                          0x00000001
#define NPU_BUS_PACKET_PROBE_FILTERS_0_STATUS_RSPEN_SHIFT                         1
#define NPU_BUS_PACKET_PROBE_FILTERS_0_STATUS_RSPEN_MASK                          0x00000002
#define NPU_BUS_PACKET_PROBE_FILTERS_0_LENGTH               0x04a4
#define NPU_BUS_PACKET_PROBE_FILTERS_0_LENGTH_FILTERS_0_LENGTH_SHIFT              0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_LENGTH_FILTERS_0_LENGTH_MASK               0x0000000f
#define NPU_BUS_PACKET_PROBE_FILTERS_0_URGENCY              0x04a8
#define NPU_BUS_PACKET_PROBE_FILTERS_0_URGENCY_FILTERS_0_URGENCY_SHIFT            0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_URGENCY_FILTERS_0_URGENCY_MASK             0x00000007
#define NPU_BUS_PACKET_PROBE_FILTERS_0_USERBASE             0x04ac
#define NPU_BUS_PACKET_PROBE_FILTERS_0_USERBASE_FILTERS_0_USERBASE_SHIFT          0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_USERBASE_FILTERS_0_USERBASE_MASK           0x001fffff
#define NPU_BUS_PACKET_PROBE_FILTERS_0_USERMASK             0x04b0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_USERMASK_FILTERS_0_USERMASK_SHIFT          0
#define NPU_BUS_PACKET_PROBE_FILTERS_0_USERMASK_FILTERS_0_USERMASK_MASK           0x001fffff
#define NPU_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDBASE          0x04e0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_SHIFT    0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_MASK     0x003fffff
#define NPU_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDMASK          0x04e4
#define NPU_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_SHIFT    0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_MASK     0x003fffff
#define NPU_BUS_PACKET_PROBE_FILTERS_1_ADDRBASE_LOW         0x04e8
#define NPU_BUS_PACKET_PROBE_FILTERS_1_ADDRBASE_HIGH        0x04ec
#define NPU_BUS_PACKET_PROBE_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_SHIFT 0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_MASK  0x0000007f
#define NPU_BUS_PACKET_PROBE_FILTERS_1_WINDOWSIZE           0x04f0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_SHIFT      0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_MASK       0x0000003f
#define NPU_BUS_PACKET_PROBE_FILTERS_1_SECURITYBASE         0x04f4
#define NPU_BUS_PACKET_PROBE_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_SHIFT  0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_MASK   0x00000001
#define NPU_BUS_PACKET_PROBE_FILTERS_1_SECURITYMASK         0x04f8
#define NPU_BUS_PACKET_PROBE_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_SHIFT  0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_MASK   0x00000001
#define NPU_BUS_PACKET_PROBE_FILTERS_1_OPCODE               0x04fc
#define NPU_BUS_PACKET_PROBE_FILTERS_1_OPCODE_RDEN_SHIFT                          0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_OPCODE_RDEN_MASK                           0x00000001
#define NPU_BUS_PACKET_PROBE_FILTERS_1_OPCODE_WREN_SHIFT                          1
#define NPU_BUS_PACKET_PROBE_FILTERS_1_OPCODE_WREN_MASK                           0x00000002
#define NPU_BUS_PACKET_PROBE_FILTERS_1_OPCODE_LOCKEN_SHIFT                        2
#define NPU_BUS_PACKET_PROBE_FILTERS_1_OPCODE_LOCKEN_MASK                         0x00000004
#define NPU_BUS_PACKET_PROBE_FILTERS_1_OPCODE_URGEN_SHIFT                         3
#define NPU_BUS_PACKET_PROBE_FILTERS_1_OPCODE_URGEN_MASK                          0x00000008
#define NPU_BUS_PACKET_PROBE_FILTERS_1_STATUS               0x0500
#define NPU_BUS_PACKET_PROBE_FILTERS_1_STATUS_REQEN_SHIFT                         0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_STATUS_REQEN_MASK                          0x00000001
#define NPU_BUS_PACKET_PROBE_FILTERS_1_STATUS_RSPEN_SHIFT                         1
#define NPU_BUS_PACKET_PROBE_FILTERS_1_STATUS_RSPEN_MASK                          0x00000002
#define NPU_BUS_PACKET_PROBE_FILTERS_1_LENGTH               0x0504
#define NPU_BUS_PACKET_PROBE_FILTERS_1_LENGTH_FILTERS_1_LENGTH_SHIFT              0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_LENGTH_FILTERS_1_LENGTH_MASK               0x0000000f
#define NPU_BUS_PACKET_PROBE_FILTERS_1_URGENCY              0x0508
#define NPU_BUS_PACKET_PROBE_FILTERS_1_URGENCY_FILTERS_1_URGENCY_SHIFT            0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_URGENCY_FILTERS_1_URGENCY_MASK             0x00000007
#define NPU_BUS_PACKET_PROBE_FILTERS_1_USERBASE             0x050c
#define NPU_BUS_PACKET_PROBE_FILTERS_1_USERBASE_FILTERS_1_USERBASE_SHIFT          0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_USERBASE_FILTERS_1_USERBASE_MASK           0x001fffff
#define NPU_BUS_PACKET_PROBE_FILTERS_1_USERMASK             0x0510
#define NPU_BUS_PACKET_PROBE_FILTERS_1_USERMASK_FILTERS_1_USERMASK_SHIFT          0
#define NPU_BUS_PACKET_PROBE_FILTERS_1_USERMASK_FILTERS_1_USERMASK_MASK           0x001fffff
#define NPU_BUS_PACKET_PROBE_COUNTERS_0_PORTSEL             0x0600
#define NPU_BUS_PACKET_PROBE_COUNTERS_0_PORTSEL_COUNTERS_0_PORTSEL_SHIFT          0
#define NPU_BUS_PACKET_PROBE_COUNTERS_0_PORTSEL_COUNTERS_0_PORTSEL_MASK           0x00000001
#define NPU_BUS_PACKET_PROBE_COUNTERS_0_SRC                 0x0604
#define NPU_BUS_PACKET_PROBE_COUNTERS_0_SRC_INTEVENT_SHIFT                        0
#define NPU_BUS_PACKET_PROBE_COUNTERS_0_SRC_INTEVENT_MASK                         0x0000001f
#define NPU_BUS_PACKET_PROBE_COUNTERS_0_ALARMMODE           0x0608
#define NPU_BUS_PACKET_PROBE_COUNTERS_0_ALARMMODE_COUNTERS_0_ALARMMODE_SHIFT      0
#define NPU_BUS_PACKET_PROBE_COUNTERS_0_ALARMMODE_COUNTERS_0_ALARMMODE_MASK       0x00000003
#define NPU_BUS_PACKET_PROBE_COUNTERS_0_VAL                 0x060c
#define NPU_BUS_PACKET_PROBE_COUNTERS_1_PORTSEL             0x0610
#define NPU_BUS_PACKET_PROBE_COUNTERS_1_PORTSEL_COUNTERS_1_PORTSEL_SHIFT          0
#define NPU_BUS_PACKET_PROBE_COUNTERS_1_PORTSEL_COUNTERS_1_PORTSEL_MASK           0x00000001
#define NPU_BUS_PACKET_PROBE_COUNTERS_1_SRC                 0x0614
#define NPU_BUS_PACKET_PROBE_COUNTERS_1_SRC_INTEVENT_SHIFT                        0
#define NPU_BUS_PACKET_PROBE_COUNTERS_1_SRC_INTEVENT_MASK                         0x0000001f
#define NPU_BUS_PACKET_PROBE_COUNTERS_1_ALARMMODE           0x0618
#define NPU_BUS_PACKET_PROBE_COUNTERS_1_ALARMMODE_COUNTERS_1_ALARMMODE_SHIFT      0
#define NPU_BUS_PACKET_PROBE_COUNTERS_1_ALARMMODE_COUNTERS_1_ALARMMODE_MASK       0x00000003
#define NPU_BUS_PACKET_PROBE_COUNTERS_1_VAL                 0x061c
#define NPU_BUS_PACKET_PROBE_COUNTERS_2_PORTSEL             0x0620
#define NPU_BUS_PACKET_PROBE_COUNTERS_2_PORTSEL_COUNTERS_2_PORTSEL_SHIFT          0
#define NPU_BUS_PACKET_PROBE_COUNTERS_2_PORTSEL_COUNTERS_2_PORTSEL_MASK           0x00000001
#define NPU_BUS_PACKET_PROBE_COUNTERS_2_SRC                 0x0624
#define NPU_BUS_PACKET_PROBE_COUNTERS_2_SRC_INTEVENT_SHIFT                        0
#define NPU_BUS_PACKET_PROBE_COUNTERS_2_SRC_INTEVENT_MASK                         0x0000001f
#define NPU_BUS_PACKET_PROBE_COUNTERS_2_ALARMMODE           0x0628
#define NPU_BUS_PACKET_PROBE_COUNTERS_2_ALARMMODE_COUNTERS_2_ALARMMODE_SHIFT      0
#define NPU_BUS_PACKET_PROBE_COUNTERS_2_ALARMMODE_COUNTERS_2_ALARMMODE_MASK       0x00000003
#define NPU_BUS_PACKET_PROBE_COUNTERS_2_VAL                 0x062c
#define NPU_BUS_PACKET_PROBE_COUNTERS_3_PORTSEL             0x0630
#define NPU_BUS_PACKET_PROBE_COUNTERS_3_PORTSEL_COUNTERS_3_PORTSEL_SHIFT          0
#define NPU_BUS_PACKET_PROBE_COUNTERS_3_PORTSEL_COUNTERS_3_PORTSEL_MASK           0x00000001
#define NPU_BUS_PACKET_PROBE_COUNTERS_3_SRC                 0x0634
#define NPU_BUS_PACKET_PROBE_COUNTERS_3_SRC_INTEVENT_SHIFT                        0
#define NPU_BUS_PACKET_PROBE_COUNTERS_3_SRC_INTEVENT_MASK                         0x0000001f
#define NPU_BUS_PACKET_PROBE_COUNTERS_3_ALARMMODE           0x0638
#define NPU_BUS_PACKET_PROBE_COUNTERS_3_ALARMMODE_COUNTERS_3_ALARMMODE_SHIFT      0
#define NPU_BUS_PACKET_PROBE_COUNTERS_3_ALARMMODE_COUNTERS_3_ALARMMODE_MASK       0x00000003
#define NPU_BUS_PACKET_PROBE_COUNTERS_3_VAL                 0x063c


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}npu_bus_err_npu_0_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}npu_bus_err_npu_0_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int faulten    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_err_npu_0_faulten_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errvld     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_err_npu_0_errvld_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errclr     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_err_npu_0_errclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lock       :  1;
        unsigned int opc        :  4;
        unsigned int reserved_0 :  3;
        unsigned int errcode    :  3;
        unsigned int reserved_1 :  5;
        unsigned int len1       :  9;
        unsigned int reserved_2 :  6;
        unsigned int format     :  1;
    } reg;
}npu_bus_err_npu_0_errlog0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog1    : 22;
        unsigned int reserved_0 : 10;
    } reg;
}npu_bus_err_npu_0_errlog1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog3    : 32;
    } reg;
}npu_bus_err_npu_0_errlog3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog4    :  7;
        unsigned int reserved_0 : 25;
    } reg;
}npu_bus_err_npu_0_errlog4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog5    : 21;
        unsigned int reserved_0 : 11;
    } reg;
}npu_bus_err_npu_0_errlog5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog7    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_err_npu_0_errlog7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int stallen    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_err_npu_0_stallen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}npu_bus_err_vdsp_0_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}npu_bus_err_vdsp_0_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int faulten    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_err_vdsp_0_faulten_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errvld     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_err_vdsp_0_errvld_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errclr     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_err_vdsp_0_errclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lock       :  1;
        unsigned int opc        :  4;
        unsigned int reserved_0 :  3;
        unsigned int errcode    :  3;
        unsigned int reserved_1 :  5;
        unsigned int len1       :  9;
        unsigned int reserved_2 :  6;
        unsigned int format     :  1;
    } reg;
}npu_bus_err_vdsp_0_errlog0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog1    : 22;
        unsigned int reserved_0 : 10;
    } reg;
}npu_bus_err_vdsp_0_errlog1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog3    : 32;
    } reg;
}npu_bus_err_vdsp_0_errlog3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog4    :  7;
        unsigned int reserved_0 : 25;
    } reg;
}npu_bus_err_vdsp_0_errlog4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog5    : 21;
        unsigned int reserved_0 : 11;
    } reg;
}npu_bus_err_vdsp_0_errlog5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog7    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_err_vdsp_0_errlog7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int stallen    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_err_vdsp_0_stallen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}npu_bus_packet_probe_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}npu_bus_packet_probe_id_revisionid_t;


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
}npu_bus_packet_probe_mainctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int globalen   :  1;
        unsigned int active     :  1;
        unsigned int reserved_0 : 30;
    } reg;
}npu_bus_packet_probe_cfgctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int traceportsel :  1;
        unsigned int reserved_0   : 31;
    } reg;
}npu_bus_packet_probe_traceportsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filterlut  :  4;
        unsigned int reserved_0 : 28;
    } reg;
}npu_bus_packet_probe_filterlut_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmen :  3;
        unsigned int reserved_0   : 29;
    } reg;
}npu_bus_packet_probe_tracealarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmstatus :  3;
        unsigned int reserved_0       : 29;
    } reg;
}npu_bus_packet_probe_tracealarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmclr :  3;
        unsigned int reserved_0    : 29;
    } reg;
}npu_bus_packet_probe_tracealarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statperiod :  5;
        unsigned int reserved_0 : 27;
    } reg;
}npu_bus_packet_probe_statperiod_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statgo     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_bus_packet_probe_statgo_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmmin : 32;
    } reg;
}npu_bus_packet_probe_statalarmmin_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmminhigh : 32;
    } reg;
}npu_bus_packet_probe_statalarmminhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmmax : 32;
    } reg;
}npu_bus_packet_probe_statalarmmax_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmmaxhigh : 32;
    } reg;
}npu_bus_packet_probe_statalarmmaxhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmstatus :  1;
        unsigned int reserved_0      : 31;
    } reg;
}npu_bus_packet_probe_statalarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmclr :  1;
        unsigned int reserved_0   : 31;
    } reg;
}npu_bus_packet_probe_statalarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmen :  1;
        unsigned int reserved_0  : 31;
    } reg;
}npu_bus_packet_probe_statalarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidbase : 22;
        unsigned int reserved_0            : 10;
    } reg;
}npu_bus_packet_probe_filters_0_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidmask : 22;
        unsigned int reserved_0            : 10;
    } reg;
}npu_bus_packet_probe_filters_0_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_low : 32;
    } reg;
}npu_bus_packet_probe_filters_0_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}npu_bus_packet_probe_filters_0_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}npu_bus_packet_probe_filters_0_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}npu_bus_packet_probe_filters_0_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}npu_bus_packet_probe_filters_0_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}npu_bus_packet_probe_filters_0_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}npu_bus_packet_probe_filters_0_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}npu_bus_packet_probe_filters_0_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}npu_bus_packet_probe_filters_0_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbase : 21;
        unsigned int reserved_0         : 11;
    } reg;
}npu_bus_packet_probe_filters_0_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermask : 21;
        unsigned int reserved_0         : 11;
    } reg;
}npu_bus_packet_probe_filters_0_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidbase : 22;
        unsigned int reserved_0            : 10;
    } reg;
}npu_bus_packet_probe_filters_1_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidmask : 22;
        unsigned int reserved_0            : 10;
    } reg;
}npu_bus_packet_probe_filters_1_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_low : 32;
    } reg;
}npu_bus_packet_probe_filters_1_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_high :  7;
        unsigned int reserved_0              : 25;
    } reg;
}npu_bus_packet_probe_filters_1_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}npu_bus_packet_probe_filters_1_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}npu_bus_packet_probe_filters_1_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}npu_bus_packet_probe_filters_1_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}npu_bus_packet_probe_filters_1_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}npu_bus_packet_probe_filters_1_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}npu_bus_packet_probe_filters_1_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}npu_bus_packet_probe_filters_1_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbase : 21;
        unsigned int reserved_0         : 11;
    } reg;
}npu_bus_packet_probe_filters_1_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermask : 21;
        unsigned int reserved_0         : 11;
    } reg;
}npu_bus_packet_probe_filters_1_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_0_portsel :  1;
        unsigned int reserved_0         : 31;
    } reg;
}npu_bus_packet_probe_counters_0_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}npu_bus_packet_probe_counters_0_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_0_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}npu_bus_packet_probe_counters_0_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_0_val : 32;
    } reg;
}npu_bus_packet_probe_counters_0_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_1_portsel :  1;
        unsigned int reserved_0         : 31;
    } reg;
}npu_bus_packet_probe_counters_1_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}npu_bus_packet_probe_counters_1_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_1_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}npu_bus_packet_probe_counters_1_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_1_val : 32;
    } reg;
}npu_bus_packet_probe_counters_1_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_2_portsel :  1;
        unsigned int reserved_0         : 31;
    } reg;
}npu_bus_packet_probe_counters_2_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}npu_bus_packet_probe_counters_2_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_2_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}npu_bus_packet_probe_counters_2_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_2_val : 32;
    } reg;
}npu_bus_packet_probe_counters_2_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_3_portsel :  1;
        unsigned int reserved_0         : 31;
    } reg;
}npu_bus_packet_probe_counters_3_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}npu_bus_packet_probe_counters_3_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_3_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}npu_bus_packet_probe_counters_3_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_3_val : 32;
    } reg;
}npu_bus_packet_probe_counters_3_val_t;

#endif
