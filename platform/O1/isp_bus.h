// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef ISP_BUS_REGIF_H
#define ISP_BUS_REGIF_H

#define ISP_BUS_ERR_0_ID_COREID                             0x0000
#define ISP_BUS_ERR_0_ID_COREID_CORETYPEID_SHIFT                                  0
#define ISP_BUS_ERR_0_ID_COREID_CORETYPEID_MASK                                   0x000000ff
#define ISP_BUS_ERR_0_ID_COREID_CORECHECKSUM_SHIFT                                8
#define ISP_BUS_ERR_0_ID_COREID_CORECHECKSUM_MASK                                 0xffffff00
#define ISP_BUS_ERR_0_ID_REVISIONID                         0x0004
#define ISP_BUS_ERR_0_ID_REVISIONID_USERID_SHIFT                                  0
#define ISP_BUS_ERR_0_ID_REVISIONID_USERID_MASK                                   0x000000ff
#define ISP_BUS_ERR_0_ID_REVISIONID_FLEXNOCID_SHIFT                               8
#define ISP_BUS_ERR_0_ID_REVISIONID_FLEXNOCID_MASK                                0xffffff00
#define ISP_BUS_ERR_0_FAULTEN                               0x0008
#define ISP_BUS_ERR_0_FAULTEN_FAULTEN_SHIFT                                       0
#define ISP_BUS_ERR_0_FAULTEN_FAULTEN_MASK                                        0x00000001
#define ISP_BUS_ERR_0_ERRVLD                                0x000c
#define ISP_BUS_ERR_0_ERRVLD_ERRVLD_SHIFT                                         0
#define ISP_BUS_ERR_0_ERRVLD_ERRVLD_MASK                                          0x00000001
#define ISP_BUS_ERR_0_ERRCLR                                0x0010
#define ISP_BUS_ERR_0_ERRCLR_ERRCLR_SHIFT                                         0
#define ISP_BUS_ERR_0_ERRCLR_ERRCLR_MASK                                          0x00000001
#define ISP_BUS_ERR_0_ERRLOG0                               0x0014
#define ISP_BUS_ERR_0_ERRLOG0_LOCK_SHIFT                                          0
#define ISP_BUS_ERR_0_ERRLOG0_LOCK_MASK                                           0x00000001
#define ISP_BUS_ERR_0_ERRLOG0_OPC_SHIFT                                           1
#define ISP_BUS_ERR_0_ERRLOG0_OPC_MASK                                            0x0000001e
#define ISP_BUS_ERR_0_ERRLOG0_ERRCODE_SHIFT                                       8
#define ISP_BUS_ERR_0_ERRLOG0_ERRCODE_MASK                                        0x00000700
#define ISP_BUS_ERR_0_ERRLOG0_LEN1_SHIFT                                          16
#define ISP_BUS_ERR_0_ERRLOG0_LEN1_MASK                                           0x03ff0000
#define ISP_BUS_ERR_0_ERRLOG0_FORMAT_SHIFT                                        31
#define ISP_BUS_ERR_0_ERRLOG0_FORMAT_MASK                                         0x80000000
#define ISP_BUS_ERR_0_ERRLOG1                               0x0018
#define ISP_BUS_ERR_0_ERRLOG1_ERRLOG1_SHIFT                                       0
#define ISP_BUS_ERR_0_ERRLOG1_ERRLOG1_MASK                                        0x01ffffff
#define ISP_BUS_ERR_0_ERRLOG3                               0x0020
#define ISP_BUS_ERR_0_ERRLOG4                               0x0024
#define ISP_BUS_ERR_0_ERRLOG4_ERRLOG4_SHIFT                                       0
#define ISP_BUS_ERR_0_ERRLOG4_ERRLOG4_MASK                                        0x000001ff
#define ISP_BUS_ERR_0_ERRLOG5                               0x0028
#define ISP_BUS_ERR_0_ERRLOG5_ERRLOG5_SHIFT                                       0
#define ISP_BUS_ERR_0_ERRLOG5_ERRLOG5_MASK                                        0x001fffff
#define ISP_BUS_ERR_0_ERRLOG7                               0x0030
#define ISP_BUS_ERR_0_ERRLOG7_ERRLOG7_SHIFT                                       0
#define ISP_BUS_ERR_0_ERRLOG7_ERRLOG7_MASK                                        0x00000001
#define ISP_BUS_ERR_0_STALLEN                               0x004c
#define ISP_BUS_ERR_0_STALLEN_STALLEN_SHIFT                                       0
#define ISP_BUS_ERR_0_STALLEN_STALLEN_MASK                                        0x00000001
#define ISP_BUS_PACKET_PROBE_ID_COREID                      0x0400
#define ISP_BUS_PACKET_PROBE_ID_COREID_CORETYPEID_SHIFT                           0
#define ISP_BUS_PACKET_PROBE_ID_COREID_CORETYPEID_MASK                            0x000000ff
#define ISP_BUS_PACKET_PROBE_ID_COREID_CORECHECKSUM_SHIFT                         8
#define ISP_BUS_PACKET_PROBE_ID_COREID_CORECHECKSUM_MASK                          0xffffff00
#define ISP_BUS_PACKET_PROBE_ID_REVISIONID                  0x0404
#define ISP_BUS_PACKET_PROBE_ID_REVISIONID_USERID_SHIFT                           0
#define ISP_BUS_PACKET_PROBE_ID_REVISIONID_USERID_MASK                            0x000000ff
#define ISP_BUS_PACKET_PROBE_ID_REVISIONID_FLEXNOCID_SHIFT                        8
#define ISP_BUS_PACKET_PROBE_ID_REVISIONID_FLEXNOCID_MASK                         0xffffff00
#define ISP_BUS_PACKET_PROBE_MAINCTL                        0x0408
#define ISP_BUS_PACKET_PROBE_MAINCTL_ERREN_SHIFT                                  0
#define ISP_BUS_PACKET_PROBE_MAINCTL_ERREN_MASK                                   0x00000001
#define ISP_BUS_PACKET_PROBE_MAINCTL_TRACEEN_SHIFT                                1
#define ISP_BUS_PACKET_PROBE_MAINCTL_TRACEEN_MASK                                 0x00000002
#define ISP_BUS_PACKET_PROBE_MAINCTL_PAYLOADEN_SHIFT                              2
#define ISP_BUS_PACKET_PROBE_MAINCTL_PAYLOADEN_MASK                               0x00000004
#define ISP_BUS_PACKET_PROBE_MAINCTL_STATEN_SHIFT                                 3
#define ISP_BUS_PACKET_PROBE_MAINCTL_STATEN_MASK                                  0x00000008
#define ISP_BUS_PACKET_PROBE_MAINCTL_ALARMEN_SHIFT                                4
#define ISP_BUS_PACKET_PROBE_MAINCTL_ALARMEN_MASK                                 0x00000010
#define ISP_BUS_PACKET_PROBE_MAINCTL_STATCONDDUMP_SHIFT                           5
#define ISP_BUS_PACKET_PROBE_MAINCTL_STATCONDDUMP_MASK                            0x00000020
#define ISP_BUS_PACKET_PROBE_MAINCTL_INTRUSIVEMODE_SHIFT                          6
#define ISP_BUS_PACKET_PROBE_MAINCTL_INTRUSIVEMODE_MASK                           0x00000040
#define ISP_BUS_PACKET_PROBE_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_SHIFT              7
#define ISP_BUS_PACKET_PROBE_MAINCTL_FILTBYTEALWAYSCHAINABLEEN_MASK               0x00000080
#define ISP_BUS_PACKET_PROBE_CFGCTL                         0x040c
#define ISP_BUS_PACKET_PROBE_CFGCTL_GLOBALEN_SHIFT                                0
#define ISP_BUS_PACKET_PROBE_CFGCTL_GLOBALEN_MASK                                 0x00000001
#define ISP_BUS_PACKET_PROBE_CFGCTL_ACTIVE_SHIFT                                  1
#define ISP_BUS_PACKET_PROBE_CFGCTL_ACTIVE_MASK                                   0x00000002
#define ISP_BUS_PACKET_PROBE_TRACEPORTSEL                   0x0410
#define ISP_BUS_PACKET_PROBE_TRACEPORTSEL_TRACEPORTSEL_SHIFT                      0
#define ISP_BUS_PACKET_PROBE_TRACEPORTSEL_TRACEPORTSEL_MASK                       0x00000003
#define ISP_BUS_PACKET_PROBE_FILTERLUT                      0x0414
#define ISP_BUS_PACKET_PROBE_FILTERLUT_FILTERLUT_SHIFT                            0
#define ISP_BUS_PACKET_PROBE_FILTERLUT_FILTERLUT_MASK                             0x0000ffff
#define ISP_BUS_PACKET_PROBE_TRACEALARMEN                   0x0418
#define ISP_BUS_PACKET_PROBE_TRACEALARMEN_TRACEALARMEN_SHIFT                      0
#define ISP_BUS_PACKET_PROBE_TRACEALARMEN_TRACEALARMEN_MASK                       0x0000001f
#define ISP_BUS_PACKET_PROBE_TRACEALARMSTATUS               0x041c
#define ISP_BUS_PACKET_PROBE_TRACEALARMSTATUS_TRACEALARMSTATUS_SHIFT              0
#define ISP_BUS_PACKET_PROBE_TRACEALARMSTATUS_TRACEALARMSTATUS_MASK               0x0000001f
#define ISP_BUS_PACKET_PROBE_TRACEALARMCLR                  0x0420
#define ISP_BUS_PACKET_PROBE_TRACEALARMCLR_TRACEALARMCLR_SHIFT                    0
#define ISP_BUS_PACKET_PROBE_TRACEALARMCLR_TRACEALARMCLR_MASK                     0x0000001f
#define ISP_BUS_PACKET_PROBE_STATPERIOD                     0x0424
#define ISP_BUS_PACKET_PROBE_STATPERIOD_STATPERIOD_SHIFT                          0
#define ISP_BUS_PACKET_PROBE_STATPERIOD_STATPERIOD_MASK                           0x0000001f
#define ISP_BUS_PACKET_PROBE_STATGO                         0x0428
#define ISP_BUS_PACKET_PROBE_STATGO_STATGO_SHIFT                                  0
#define ISP_BUS_PACKET_PROBE_STATGO_STATGO_MASK                                   0x00000001
#define ISP_BUS_PACKET_PROBE_STATALARMMIN                   0x042c
#define ISP_BUS_PACKET_PROBE_STATALARMMINHIGH               0x0430
#define ISP_BUS_PACKET_PROBE_STATALARMMAX                   0x0434
#define ISP_BUS_PACKET_PROBE_STATALARMMAXHIGH               0x0438
#define ISP_BUS_PACKET_PROBE_STATALARMSTATUS                0x043c
#define ISP_BUS_PACKET_PROBE_STATALARMSTATUS_STATALARMSTATUS_SHIFT                0
#define ISP_BUS_PACKET_PROBE_STATALARMSTATUS_STATALARMSTATUS_MASK                 0x00000001
#define ISP_BUS_PACKET_PROBE_STATALARMCLR                   0x0440
#define ISP_BUS_PACKET_PROBE_STATALARMCLR_STATALARMCLR_SHIFT                      0
#define ISP_BUS_PACKET_PROBE_STATALARMCLR_STATALARMCLR_MASK                       0x00000001
#define ISP_BUS_PACKET_PROBE_STATALARMEN                    0x0444
#define ISP_BUS_PACKET_PROBE_STATALARMEN_STATALARMEN_SHIFT                        0
#define ISP_BUS_PACKET_PROBE_STATALARMEN_STATALARMEN_MASK                         0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDBASE          0x0480
#define ISP_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_SHIFT    0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDBASE_FILTERS_0_ROUTEIDBASE_MASK     0x01ffffff
#define ISP_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDMASK          0x0484
#define ISP_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_SHIFT    0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_ROUTEIDMASK_FILTERS_0_ROUTEIDMASK_MASK     0x01ffffff
#define ISP_BUS_PACKET_PROBE_FILTERS_0_ADDRBASE_LOW         0x0488
#define ISP_BUS_PACKET_PROBE_FILTERS_0_ADDRBASE_HIGH        0x048c
#define ISP_BUS_PACKET_PROBE_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_SHIFT 0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_ADDRBASE_HIGH_FILTERS_0_ADDRBASE_HIGH_MASK  0x000001ff
#define ISP_BUS_PACKET_PROBE_FILTERS_0_WINDOWSIZE           0x0490
#define ISP_BUS_PACKET_PROBE_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_WINDOWSIZE_FILTERS_0_WINDOWSIZE_MASK       0x0000003f
#define ISP_BUS_PACKET_PROBE_FILTERS_0_SECURITYBASE         0x0494
#define ISP_BUS_PACKET_PROBE_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_SHIFT  0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_SECURITYBASE_FILTERS_0_SECURITYBASE_MASK   0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_0_SECURITYMASK         0x0498
#define ISP_BUS_PACKET_PROBE_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_SHIFT  0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_SECURITYMASK_FILTERS_0_SECURITYMASK_MASK   0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_0_OPCODE               0x049c
#define ISP_BUS_PACKET_PROBE_FILTERS_0_OPCODE_RDEN_SHIFT                          0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_OPCODE_RDEN_MASK                           0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_0_OPCODE_WREN_SHIFT                          1
#define ISP_BUS_PACKET_PROBE_FILTERS_0_OPCODE_WREN_MASK                           0x00000002
#define ISP_BUS_PACKET_PROBE_FILTERS_0_OPCODE_LOCKEN_SHIFT                        2
#define ISP_BUS_PACKET_PROBE_FILTERS_0_OPCODE_LOCKEN_MASK                         0x00000004
#define ISP_BUS_PACKET_PROBE_FILTERS_0_OPCODE_URGEN_SHIFT                         3
#define ISP_BUS_PACKET_PROBE_FILTERS_0_OPCODE_URGEN_MASK                          0x00000008
#define ISP_BUS_PACKET_PROBE_FILTERS_0_STATUS               0x04a0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_STATUS_REQEN_SHIFT                         0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_STATUS_REQEN_MASK                          0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_0_STATUS_RSPEN_SHIFT                         1
#define ISP_BUS_PACKET_PROBE_FILTERS_0_STATUS_RSPEN_MASK                          0x00000002
#define ISP_BUS_PACKET_PROBE_FILTERS_0_LENGTH               0x04a4
#define ISP_BUS_PACKET_PROBE_FILTERS_0_LENGTH_FILTERS_0_LENGTH_SHIFT              0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_LENGTH_FILTERS_0_LENGTH_MASK               0x0000000f
#define ISP_BUS_PACKET_PROBE_FILTERS_0_URGENCY              0x04a8
#define ISP_BUS_PACKET_PROBE_FILTERS_0_URGENCY_FILTERS_0_URGENCY_SHIFT            0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_URGENCY_FILTERS_0_URGENCY_MASK             0x00000007
#define ISP_BUS_PACKET_PROBE_FILTERS_0_USERBASE             0x04ac
#define ISP_BUS_PACKET_PROBE_FILTERS_0_USERBASE_FILTERS_0_USERBASE_SHIFT          0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_USERBASE_FILTERS_0_USERBASE_MASK           0x001fffff
#define ISP_BUS_PACKET_PROBE_FILTERS_0_USERMASK             0x04b0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_USERMASK_FILTERS_0_USERMASK_SHIFT          0
#define ISP_BUS_PACKET_PROBE_FILTERS_0_USERMASK_FILTERS_0_USERMASK_MASK           0x001fffff
#define ISP_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDBASE          0x04e0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_SHIFT    0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDBASE_FILTERS_1_ROUTEIDBASE_MASK     0x01ffffff
#define ISP_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDMASK          0x04e4
#define ISP_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_SHIFT    0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_ROUTEIDMASK_FILTERS_1_ROUTEIDMASK_MASK     0x01ffffff
#define ISP_BUS_PACKET_PROBE_FILTERS_1_ADDRBASE_LOW         0x04e8
#define ISP_BUS_PACKET_PROBE_FILTERS_1_ADDRBASE_HIGH        0x04ec
#define ISP_BUS_PACKET_PROBE_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_SHIFT 0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_ADDRBASE_HIGH_FILTERS_1_ADDRBASE_HIGH_MASK  0x000001ff
#define ISP_BUS_PACKET_PROBE_FILTERS_1_WINDOWSIZE           0x04f0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_WINDOWSIZE_FILTERS_1_WINDOWSIZE_MASK       0x0000003f
#define ISP_BUS_PACKET_PROBE_FILTERS_1_SECURITYBASE         0x04f4
#define ISP_BUS_PACKET_PROBE_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_SHIFT  0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_SECURITYBASE_FILTERS_1_SECURITYBASE_MASK   0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_1_SECURITYMASK         0x04f8
#define ISP_BUS_PACKET_PROBE_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_SHIFT  0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_SECURITYMASK_FILTERS_1_SECURITYMASK_MASK   0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_1_OPCODE               0x04fc
#define ISP_BUS_PACKET_PROBE_FILTERS_1_OPCODE_RDEN_SHIFT                          0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_OPCODE_RDEN_MASK                           0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_1_OPCODE_WREN_SHIFT                          1
#define ISP_BUS_PACKET_PROBE_FILTERS_1_OPCODE_WREN_MASK                           0x00000002
#define ISP_BUS_PACKET_PROBE_FILTERS_1_OPCODE_LOCKEN_SHIFT                        2
#define ISP_BUS_PACKET_PROBE_FILTERS_1_OPCODE_LOCKEN_MASK                         0x00000004
#define ISP_BUS_PACKET_PROBE_FILTERS_1_OPCODE_URGEN_SHIFT                         3
#define ISP_BUS_PACKET_PROBE_FILTERS_1_OPCODE_URGEN_MASK                          0x00000008
#define ISP_BUS_PACKET_PROBE_FILTERS_1_STATUS               0x0500
#define ISP_BUS_PACKET_PROBE_FILTERS_1_STATUS_REQEN_SHIFT                         0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_STATUS_REQEN_MASK                          0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_1_STATUS_RSPEN_SHIFT                         1
#define ISP_BUS_PACKET_PROBE_FILTERS_1_STATUS_RSPEN_MASK                          0x00000002
#define ISP_BUS_PACKET_PROBE_FILTERS_1_LENGTH               0x0504
#define ISP_BUS_PACKET_PROBE_FILTERS_1_LENGTH_FILTERS_1_LENGTH_SHIFT              0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_LENGTH_FILTERS_1_LENGTH_MASK               0x0000000f
#define ISP_BUS_PACKET_PROBE_FILTERS_1_URGENCY              0x0508
#define ISP_BUS_PACKET_PROBE_FILTERS_1_URGENCY_FILTERS_1_URGENCY_SHIFT            0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_URGENCY_FILTERS_1_URGENCY_MASK             0x00000007
#define ISP_BUS_PACKET_PROBE_FILTERS_1_USERBASE             0x050c
#define ISP_BUS_PACKET_PROBE_FILTERS_1_USERBASE_FILTERS_1_USERBASE_SHIFT          0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_USERBASE_FILTERS_1_USERBASE_MASK           0x001fffff
#define ISP_BUS_PACKET_PROBE_FILTERS_1_USERMASK             0x0510
#define ISP_BUS_PACKET_PROBE_FILTERS_1_USERMASK_FILTERS_1_USERMASK_SHIFT          0
#define ISP_BUS_PACKET_PROBE_FILTERS_1_USERMASK_FILTERS_1_USERMASK_MASK           0x001fffff
#define ISP_BUS_PACKET_PROBE_FILTERS_2_ROUTEIDBASE          0x0540
#define ISP_BUS_PACKET_PROBE_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_SHIFT    0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_ROUTEIDBASE_FILTERS_2_ROUTEIDBASE_MASK     0x01ffffff
#define ISP_BUS_PACKET_PROBE_FILTERS_2_ROUTEIDMASK          0x0544
#define ISP_BUS_PACKET_PROBE_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_SHIFT    0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_ROUTEIDMASK_FILTERS_2_ROUTEIDMASK_MASK     0x01ffffff
#define ISP_BUS_PACKET_PROBE_FILTERS_2_ADDRBASE_LOW         0x0548
#define ISP_BUS_PACKET_PROBE_FILTERS_2_ADDRBASE_HIGH        0x054c
#define ISP_BUS_PACKET_PROBE_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_SHIFT 0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_ADDRBASE_HIGH_FILTERS_2_ADDRBASE_HIGH_MASK  0x000001ff
#define ISP_BUS_PACKET_PROBE_FILTERS_2_WINDOWSIZE           0x0550
#define ISP_BUS_PACKET_PROBE_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_WINDOWSIZE_FILTERS_2_WINDOWSIZE_MASK       0x0000003f
#define ISP_BUS_PACKET_PROBE_FILTERS_2_SECURITYBASE         0x0554
#define ISP_BUS_PACKET_PROBE_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_SHIFT  0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_SECURITYBASE_FILTERS_2_SECURITYBASE_MASK   0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_2_SECURITYMASK         0x0558
#define ISP_BUS_PACKET_PROBE_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_SHIFT  0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_SECURITYMASK_FILTERS_2_SECURITYMASK_MASK   0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_2_OPCODE               0x055c
#define ISP_BUS_PACKET_PROBE_FILTERS_2_OPCODE_RDEN_SHIFT                          0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_OPCODE_RDEN_MASK                           0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_2_OPCODE_WREN_SHIFT                          1
#define ISP_BUS_PACKET_PROBE_FILTERS_2_OPCODE_WREN_MASK                           0x00000002
#define ISP_BUS_PACKET_PROBE_FILTERS_2_OPCODE_LOCKEN_SHIFT                        2
#define ISP_BUS_PACKET_PROBE_FILTERS_2_OPCODE_LOCKEN_MASK                         0x00000004
#define ISP_BUS_PACKET_PROBE_FILTERS_2_OPCODE_URGEN_SHIFT                         3
#define ISP_BUS_PACKET_PROBE_FILTERS_2_OPCODE_URGEN_MASK                          0x00000008
#define ISP_BUS_PACKET_PROBE_FILTERS_2_STATUS               0x0560
#define ISP_BUS_PACKET_PROBE_FILTERS_2_STATUS_REQEN_SHIFT                         0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_STATUS_REQEN_MASK                          0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_2_STATUS_RSPEN_SHIFT                         1
#define ISP_BUS_PACKET_PROBE_FILTERS_2_STATUS_RSPEN_MASK                          0x00000002
#define ISP_BUS_PACKET_PROBE_FILTERS_2_LENGTH               0x0564
#define ISP_BUS_PACKET_PROBE_FILTERS_2_LENGTH_FILTERS_2_LENGTH_SHIFT              0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_LENGTH_FILTERS_2_LENGTH_MASK               0x0000000f
#define ISP_BUS_PACKET_PROBE_FILTERS_2_URGENCY              0x0568
#define ISP_BUS_PACKET_PROBE_FILTERS_2_URGENCY_FILTERS_2_URGENCY_SHIFT            0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_URGENCY_FILTERS_2_URGENCY_MASK             0x00000007
#define ISP_BUS_PACKET_PROBE_FILTERS_2_USERBASE             0x056c
#define ISP_BUS_PACKET_PROBE_FILTERS_2_USERBASE_FILTERS_2_USERBASE_SHIFT          0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_USERBASE_FILTERS_2_USERBASE_MASK           0x001fffff
#define ISP_BUS_PACKET_PROBE_FILTERS_2_USERMASK             0x0570
#define ISP_BUS_PACKET_PROBE_FILTERS_2_USERMASK_FILTERS_2_USERMASK_SHIFT          0
#define ISP_BUS_PACKET_PROBE_FILTERS_2_USERMASK_FILTERS_2_USERMASK_MASK           0x001fffff
#define ISP_BUS_PACKET_PROBE_FILTERS_3_ROUTEIDBASE          0x05a0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_SHIFT    0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_ROUTEIDBASE_FILTERS_3_ROUTEIDBASE_MASK     0x01ffffff
#define ISP_BUS_PACKET_PROBE_FILTERS_3_ROUTEIDMASK          0x05a4
#define ISP_BUS_PACKET_PROBE_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_SHIFT    0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_ROUTEIDMASK_FILTERS_3_ROUTEIDMASK_MASK     0x01ffffff
#define ISP_BUS_PACKET_PROBE_FILTERS_3_ADDRBASE_LOW         0x05a8
#define ISP_BUS_PACKET_PROBE_FILTERS_3_ADDRBASE_HIGH        0x05ac
#define ISP_BUS_PACKET_PROBE_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_SHIFT 0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_ADDRBASE_HIGH_FILTERS_3_ADDRBASE_HIGH_MASK  0x000001ff
#define ISP_BUS_PACKET_PROBE_FILTERS_3_WINDOWSIZE           0x05b0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_WINDOWSIZE_FILTERS_3_WINDOWSIZE_MASK       0x0000003f
#define ISP_BUS_PACKET_PROBE_FILTERS_3_SECURITYBASE         0x05b4
#define ISP_BUS_PACKET_PROBE_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_SHIFT  0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_SECURITYBASE_FILTERS_3_SECURITYBASE_MASK   0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_3_SECURITYMASK         0x05b8
#define ISP_BUS_PACKET_PROBE_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_SHIFT  0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_SECURITYMASK_FILTERS_3_SECURITYMASK_MASK   0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_3_OPCODE               0x05bc
#define ISP_BUS_PACKET_PROBE_FILTERS_3_OPCODE_RDEN_SHIFT                          0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_OPCODE_RDEN_MASK                           0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_3_OPCODE_WREN_SHIFT                          1
#define ISP_BUS_PACKET_PROBE_FILTERS_3_OPCODE_WREN_MASK                           0x00000002
#define ISP_BUS_PACKET_PROBE_FILTERS_3_OPCODE_LOCKEN_SHIFT                        2
#define ISP_BUS_PACKET_PROBE_FILTERS_3_OPCODE_LOCKEN_MASK                         0x00000004
#define ISP_BUS_PACKET_PROBE_FILTERS_3_OPCODE_URGEN_SHIFT                         3
#define ISP_BUS_PACKET_PROBE_FILTERS_3_OPCODE_URGEN_MASK                          0x00000008
#define ISP_BUS_PACKET_PROBE_FILTERS_3_STATUS               0x05c0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_STATUS_REQEN_SHIFT                         0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_STATUS_REQEN_MASK                          0x00000001
#define ISP_BUS_PACKET_PROBE_FILTERS_3_STATUS_RSPEN_SHIFT                         1
#define ISP_BUS_PACKET_PROBE_FILTERS_3_STATUS_RSPEN_MASK                          0x00000002
#define ISP_BUS_PACKET_PROBE_FILTERS_3_LENGTH               0x05c4
#define ISP_BUS_PACKET_PROBE_FILTERS_3_LENGTH_FILTERS_3_LENGTH_SHIFT              0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_LENGTH_FILTERS_3_LENGTH_MASK               0x0000000f
#define ISP_BUS_PACKET_PROBE_FILTERS_3_URGENCY              0x05c8
#define ISP_BUS_PACKET_PROBE_FILTERS_3_URGENCY_FILTERS_3_URGENCY_SHIFT            0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_URGENCY_FILTERS_3_URGENCY_MASK             0x00000007
#define ISP_BUS_PACKET_PROBE_FILTERS_3_USERBASE             0x05cc
#define ISP_BUS_PACKET_PROBE_FILTERS_3_USERBASE_FILTERS_3_USERBASE_SHIFT          0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_USERBASE_FILTERS_3_USERBASE_MASK           0x001fffff
#define ISP_BUS_PACKET_PROBE_FILTERS_3_USERMASK             0x05d0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_USERMASK_FILTERS_3_USERMASK_SHIFT          0
#define ISP_BUS_PACKET_PROBE_FILTERS_3_USERMASK_FILTERS_3_USERMASK_MASK           0x001fffff
#define ISP_BUS_PACKET_PROBE_COUNTERS_0_PORTSEL             0x0600
#define ISP_BUS_PACKET_PROBE_COUNTERS_0_PORTSEL_COUNTERS_0_PORTSEL_SHIFT          0
#define ISP_BUS_PACKET_PROBE_COUNTERS_0_PORTSEL_COUNTERS_0_PORTSEL_MASK           0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_0_SRC                 0x0604
#define ISP_BUS_PACKET_PROBE_COUNTERS_0_SRC_INTEVENT_SHIFT                        0
#define ISP_BUS_PACKET_PROBE_COUNTERS_0_SRC_INTEVENT_MASK                         0x0000001f
#define ISP_BUS_PACKET_PROBE_COUNTERS_0_ALARMMODE           0x0608
#define ISP_BUS_PACKET_PROBE_COUNTERS_0_ALARMMODE_COUNTERS_0_ALARMMODE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_COUNTERS_0_ALARMMODE_COUNTERS_0_ALARMMODE_MASK       0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_0_VAL                 0x060c
#define ISP_BUS_PACKET_PROBE_COUNTERS_1_PORTSEL             0x0610
#define ISP_BUS_PACKET_PROBE_COUNTERS_1_PORTSEL_COUNTERS_1_PORTSEL_SHIFT          0
#define ISP_BUS_PACKET_PROBE_COUNTERS_1_PORTSEL_COUNTERS_1_PORTSEL_MASK           0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_1_SRC                 0x0614
#define ISP_BUS_PACKET_PROBE_COUNTERS_1_SRC_INTEVENT_SHIFT                        0
#define ISP_BUS_PACKET_PROBE_COUNTERS_1_SRC_INTEVENT_MASK                         0x0000001f
#define ISP_BUS_PACKET_PROBE_COUNTERS_1_ALARMMODE           0x0618
#define ISP_BUS_PACKET_PROBE_COUNTERS_1_ALARMMODE_COUNTERS_1_ALARMMODE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_COUNTERS_1_ALARMMODE_COUNTERS_1_ALARMMODE_MASK       0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_1_VAL                 0x061c
#define ISP_BUS_PACKET_PROBE_COUNTERS_2_PORTSEL             0x0620
#define ISP_BUS_PACKET_PROBE_COUNTERS_2_PORTSEL_COUNTERS_2_PORTSEL_SHIFT          0
#define ISP_BUS_PACKET_PROBE_COUNTERS_2_PORTSEL_COUNTERS_2_PORTSEL_MASK           0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_2_SRC                 0x0624
#define ISP_BUS_PACKET_PROBE_COUNTERS_2_SRC_INTEVENT_SHIFT                        0
#define ISP_BUS_PACKET_PROBE_COUNTERS_2_SRC_INTEVENT_MASK                         0x0000001f
#define ISP_BUS_PACKET_PROBE_COUNTERS_2_ALARMMODE           0x0628
#define ISP_BUS_PACKET_PROBE_COUNTERS_2_ALARMMODE_COUNTERS_2_ALARMMODE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_COUNTERS_2_ALARMMODE_COUNTERS_2_ALARMMODE_MASK       0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_2_VAL                 0x062c
#define ISP_BUS_PACKET_PROBE_COUNTERS_3_PORTSEL             0x0630
#define ISP_BUS_PACKET_PROBE_COUNTERS_3_PORTSEL_COUNTERS_3_PORTSEL_SHIFT          0
#define ISP_BUS_PACKET_PROBE_COUNTERS_3_PORTSEL_COUNTERS_3_PORTSEL_MASK           0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_3_SRC                 0x0634
#define ISP_BUS_PACKET_PROBE_COUNTERS_3_SRC_INTEVENT_SHIFT                        0
#define ISP_BUS_PACKET_PROBE_COUNTERS_3_SRC_INTEVENT_MASK                         0x0000001f
#define ISP_BUS_PACKET_PROBE_COUNTERS_3_ALARMMODE           0x0638
#define ISP_BUS_PACKET_PROBE_COUNTERS_3_ALARMMODE_COUNTERS_3_ALARMMODE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_COUNTERS_3_ALARMMODE_COUNTERS_3_ALARMMODE_MASK       0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_3_VAL                 0x063c
#define ISP_BUS_PACKET_PROBE_COUNTERS_4_PORTSEL             0x0640
#define ISP_BUS_PACKET_PROBE_COUNTERS_4_PORTSEL_COUNTERS_4_PORTSEL_SHIFT          0
#define ISP_BUS_PACKET_PROBE_COUNTERS_4_PORTSEL_COUNTERS_4_PORTSEL_MASK           0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_4_SRC                 0x0644
#define ISP_BUS_PACKET_PROBE_COUNTERS_4_SRC_INTEVENT_SHIFT                        0
#define ISP_BUS_PACKET_PROBE_COUNTERS_4_SRC_INTEVENT_MASK                         0x0000001f
#define ISP_BUS_PACKET_PROBE_COUNTERS_4_ALARMMODE           0x0648
#define ISP_BUS_PACKET_PROBE_COUNTERS_4_ALARMMODE_COUNTERS_4_ALARMMODE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_COUNTERS_4_ALARMMODE_COUNTERS_4_ALARMMODE_MASK       0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_4_VAL                 0x064c
#define ISP_BUS_PACKET_PROBE_COUNTERS_5_PORTSEL             0x0650
#define ISP_BUS_PACKET_PROBE_COUNTERS_5_PORTSEL_COUNTERS_5_PORTSEL_SHIFT          0
#define ISP_BUS_PACKET_PROBE_COUNTERS_5_PORTSEL_COUNTERS_5_PORTSEL_MASK           0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_5_SRC                 0x0654
#define ISP_BUS_PACKET_PROBE_COUNTERS_5_SRC_INTEVENT_SHIFT                        0
#define ISP_BUS_PACKET_PROBE_COUNTERS_5_SRC_INTEVENT_MASK                         0x0000001f
#define ISP_BUS_PACKET_PROBE_COUNTERS_5_ALARMMODE           0x0658
#define ISP_BUS_PACKET_PROBE_COUNTERS_5_ALARMMODE_COUNTERS_5_ALARMMODE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_COUNTERS_5_ALARMMODE_COUNTERS_5_ALARMMODE_MASK       0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_5_VAL                 0x065c
#define ISP_BUS_PACKET_PROBE_COUNTERS_6_PORTSEL             0x0660
#define ISP_BUS_PACKET_PROBE_COUNTERS_6_PORTSEL_COUNTERS_6_PORTSEL_SHIFT          0
#define ISP_BUS_PACKET_PROBE_COUNTERS_6_PORTSEL_COUNTERS_6_PORTSEL_MASK           0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_6_SRC                 0x0664
#define ISP_BUS_PACKET_PROBE_COUNTERS_6_SRC_INTEVENT_SHIFT                        0
#define ISP_BUS_PACKET_PROBE_COUNTERS_6_SRC_INTEVENT_MASK                         0x0000001f
#define ISP_BUS_PACKET_PROBE_COUNTERS_6_ALARMMODE           0x0668
#define ISP_BUS_PACKET_PROBE_COUNTERS_6_ALARMMODE_COUNTERS_6_ALARMMODE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_COUNTERS_6_ALARMMODE_COUNTERS_6_ALARMMODE_MASK       0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_6_VAL                 0x066c
#define ISP_BUS_PACKET_PROBE_COUNTERS_7_PORTSEL             0x0670
#define ISP_BUS_PACKET_PROBE_COUNTERS_7_PORTSEL_COUNTERS_7_PORTSEL_SHIFT          0
#define ISP_BUS_PACKET_PROBE_COUNTERS_7_PORTSEL_COUNTERS_7_PORTSEL_MASK           0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_7_SRC                 0x0674
#define ISP_BUS_PACKET_PROBE_COUNTERS_7_SRC_INTEVENT_SHIFT                        0
#define ISP_BUS_PACKET_PROBE_COUNTERS_7_SRC_INTEVENT_MASK                         0x0000001f
#define ISP_BUS_PACKET_PROBE_COUNTERS_7_ALARMMODE           0x0678
#define ISP_BUS_PACKET_PROBE_COUNTERS_7_ALARMMODE_COUNTERS_7_ALARMMODE_SHIFT      0
#define ISP_BUS_PACKET_PROBE_COUNTERS_7_ALARMMODE_COUNTERS_7_ALARMMODE_MASK       0x00000003
#define ISP_BUS_PACKET_PROBE_COUNTERS_7_VAL                 0x067c


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}isp_bus_err_0_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}isp_bus_err_0_id_revisionid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int faulten    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}isp_bus_err_0_faulten_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errvld     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}isp_bus_err_0_errvld_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errclr     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}isp_bus_err_0_errclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lock       :  1;
        unsigned int opc        :  4;
        unsigned int reserved_0 :  3;
        unsigned int errcode    :  3;
        unsigned int reserved_1 :  5;
        unsigned int len1       : 10;
        unsigned int reserved_2 :  5;
        unsigned int format     :  1;
    } reg;
}isp_bus_err_0_errlog0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog1    : 25;
        unsigned int reserved_0 :  7;
    } reg;
}isp_bus_err_0_errlog1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog3    : 32;
    } reg;
}isp_bus_err_0_errlog3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog4    :  9;
        unsigned int reserved_0 : 23;
    } reg;
}isp_bus_err_0_errlog4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog5    : 21;
        unsigned int reserved_0 : 11;
    } reg;
}isp_bus_err_0_errlog5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int errlog7    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}isp_bus_err_0_errlog7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int stallen    :  1;
        unsigned int reserved_0 : 31;
    } reg;
}isp_bus_err_0_stallen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int coretypeid   :  8;
        unsigned int corechecksum : 24;
    } reg;
}isp_bus_packet_probe_id_coreid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int userid     :  8;
        unsigned int flexnocid  : 24;
    } reg;
}isp_bus_packet_probe_id_revisionid_t;


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
}isp_bus_packet_probe_mainctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int globalen   :  1;
        unsigned int active     :  1;
        unsigned int reserved_0 : 30;
    } reg;
}isp_bus_packet_probe_cfgctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int traceportsel :  2;
        unsigned int reserved_0   : 30;
    } reg;
}isp_bus_packet_probe_traceportsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filterlut  : 16;
        unsigned int reserved_0 : 16;
    } reg;
}isp_bus_packet_probe_filterlut_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmen :  5;
        unsigned int reserved_0   : 27;
    } reg;
}isp_bus_packet_probe_tracealarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmstatus :  5;
        unsigned int reserved_0       : 27;
    } reg;
}isp_bus_packet_probe_tracealarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tracealarmclr :  5;
        unsigned int reserved_0    : 27;
    } reg;
}isp_bus_packet_probe_tracealarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statperiod :  5;
        unsigned int reserved_0 : 27;
    } reg;
}isp_bus_packet_probe_statperiod_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statgo     :  1;
        unsigned int reserved_0 : 31;
    } reg;
}isp_bus_packet_probe_statgo_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmmin : 32;
    } reg;
}isp_bus_packet_probe_statalarmmin_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmminhigh : 32;
    } reg;
}isp_bus_packet_probe_statalarmminhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmmax : 32;
    } reg;
}isp_bus_packet_probe_statalarmmax_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmmaxhigh : 32;
    } reg;
}isp_bus_packet_probe_statalarmmaxhigh_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmstatus :  1;
        unsigned int reserved_0      : 31;
    } reg;
}isp_bus_packet_probe_statalarmstatus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmclr :  1;
        unsigned int reserved_0   : 31;
    } reg;
}isp_bus_packet_probe_statalarmclr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int statalarmen :  1;
        unsigned int reserved_0  : 31;
    } reg;
}isp_bus_packet_probe_statalarmen_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}isp_bus_packet_probe_filters_0_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}isp_bus_packet_probe_filters_0_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_low : 32;
    } reg;
}isp_bus_packet_probe_filters_0_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_addrbase_high :  9;
        unsigned int reserved_0              : 23;
    } reg;
}isp_bus_packet_probe_filters_0_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}isp_bus_packet_probe_filters_0_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}isp_bus_packet_probe_filters_0_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}isp_bus_packet_probe_filters_0_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}isp_bus_packet_probe_filters_0_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}isp_bus_packet_probe_filters_0_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}isp_bus_packet_probe_filters_0_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}isp_bus_packet_probe_filters_0_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_userbase : 21;
        unsigned int reserved_0         : 11;
    } reg;
}isp_bus_packet_probe_filters_0_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_0_usermask : 21;
        unsigned int reserved_0         : 11;
    } reg;
}isp_bus_packet_probe_filters_0_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}isp_bus_packet_probe_filters_1_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}isp_bus_packet_probe_filters_1_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_low : 32;
    } reg;
}isp_bus_packet_probe_filters_1_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_addrbase_high :  9;
        unsigned int reserved_0              : 23;
    } reg;
}isp_bus_packet_probe_filters_1_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}isp_bus_packet_probe_filters_1_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}isp_bus_packet_probe_filters_1_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}isp_bus_packet_probe_filters_1_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}isp_bus_packet_probe_filters_1_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}isp_bus_packet_probe_filters_1_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}isp_bus_packet_probe_filters_1_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}isp_bus_packet_probe_filters_1_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_userbase : 21;
        unsigned int reserved_0         : 11;
    } reg;
}isp_bus_packet_probe_filters_1_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_1_usermask : 21;
        unsigned int reserved_0         : 11;
    } reg;
}isp_bus_packet_probe_filters_1_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}isp_bus_packet_probe_filters_2_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}isp_bus_packet_probe_filters_2_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_low : 32;
    } reg;
}isp_bus_packet_probe_filters_2_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_addrbase_high :  9;
        unsigned int reserved_0              : 23;
    } reg;
}isp_bus_packet_probe_filters_2_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}isp_bus_packet_probe_filters_2_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}isp_bus_packet_probe_filters_2_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}isp_bus_packet_probe_filters_2_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}isp_bus_packet_probe_filters_2_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}isp_bus_packet_probe_filters_2_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}isp_bus_packet_probe_filters_2_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}isp_bus_packet_probe_filters_2_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_userbase : 21;
        unsigned int reserved_0         : 11;
    } reg;
}isp_bus_packet_probe_filters_2_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_2_usermask : 21;
        unsigned int reserved_0         : 11;
    } reg;
}isp_bus_packet_probe_filters_2_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidbase : 25;
        unsigned int reserved_0            :  7;
    } reg;
}isp_bus_packet_probe_filters_3_routeidbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_routeidmask : 25;
        unsigned int reserved_0            :  7;
    } reg;
}isp_bus_packet_probe_filters_3_routeidmask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_low : 32;
    } reg;
}isp_bus_packet_probe_filters_3_addrbase_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_addrbase_high :  9;
        unsigned int reserved_0              : 23;
    } reg;
}isp_bus_packet_probe_filters_3_addrbase_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_windowsize :  6;
        unsigned int reserved_0           : 26;
    } reg;
}isp_bus_packet_probe_filters_3_windowsize_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitybase :  1;
        unsigned int reserved_0             : 31;
    } reg;
}isp_bus_packet_probe_filters_3_securitybase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_securitymask :  1;
        unsigned int reserved_0             : 31;
    } reg;
}isp_bus_packet_probe_filters_3_securitymask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rden       :  1;
        unsigned int wren       :  1;
        unsigned int locken     :  1;
        unsigned int urgen      :  1;
        unsigned int reserved_0 : 28;
    } reg;
}isp_bus_packet_probe_filters_3_opcode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reqen      :  1;
        unsigned int rspen      :  1;
        unsigned int reserved_0 : 30;
    } reg;
}isp_bus_packet_probe_filters_3_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_length :  4;
        unsigned int reserved_0       : 28;
    } reg;
}isp_bus_packet_probe_filters_3_length_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_urgency :  3;
        unsigned int reserved_0        : 29;
    } reg;
}isp_bus_packet_probe_filters_3_urgency_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_userbase : 21;
        unsigned int reserved_0         : 11;
    } reg;
}isp_bus_packet_probe_filters_3_userbase_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int filters_3_usermask : 21;
        unsigned int reserved_0         : 11;
    } reg;
}isp_bus_packet_probe_filters_3_usermask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_0_portsel :  2;
        unsigned int reserved_0         : 30;
    } reg;
}isp_bus_packet_probe_counters_0_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}isp_bus_packet_probe_counters_0_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_0_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}isp_bus_packet_probe_counters_0_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_0_val : 32;
    } reg;
}isp_bus_packet_probe_counters_0_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_1_portsel :  2;
        unsigned int reserved_0         : 30;
    } reg;
}isp_bus_packet_probe_counters_1_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}isp_bus_packet_probe_counters_1_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_1_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}isp_bus_packet_probe_counters_1_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_1_val : 32;
    } reg;
}isp_bus_packet_probe_counters_1_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_2_portsel :  2;
        unsigned int reserved_0         : 30;
    } reg;
}isp_bus_packet_probe_counters_2_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}isp_bus_packet_probe_counters_2_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_2_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}isp_bus_packet_probe_counters_2_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_2_val : 32;
    } reg;
}isp_bus_packet_probe_counters_2_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_3_portsel :  2;
        unsigned int reserved_0         : 30;
    } reg;
}isp_bus_packet_probe_counters_3_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}isp_bus_packet_probe_counters_3_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_3_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}isp_bus_packet_probe_counters_3_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_3_val : 32;
    } reg;
}isp_bus_packet_probe_counters_3_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_4_portsel :  2;
        unsigned int reserved_0         : 30;
    } reg;
}isp_bus_packet_probe_counters_4_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}isp_bus_packet_probe_counters_4_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_4_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}isp_bus_packet_probe_counters_4_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_4_val : 32;
    } reg;
}isp_bus_packet_probe_counters_4_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_5_portsel :  2;
        unsigned int reserved_0         : 30;
    } reg;
}isp_bus_packet_probe_counters_5_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}isp_bus_packet_probe_counters_5_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_5_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}isp_bus_packet_probe_counters_5_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_5_val : 32;
    } reg;
}isp_bus_packet_probe_counters_5_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_6_portsel :  2;
        unsigned int reserved_0         : 30;
    } reg;
}isp_bus_packet_probe_counters_6_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}isp_bus_packet_probe_counters_6_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_6_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}isp_bus_packet_probe_counters_6_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_6_val : 32;
    } reg;
}isp_bus_packet_probe_counters_6_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_7_portsel :  2;
        unsigned int reserved_0         : 30;
    } reg;
}isp_bus_packet_probe_counters_7_portsel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intevent   :  5;
        unsigned int reserved_0 : 27;
    } reg;
}isp_bus_packet_probe_counters_7_src_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_7_alarmmode :  2;
        unsigned int reserved_0           : 30;
    } reg;
}isp_bus_packet_probe_counters_7_alarmmode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int counters_7_val : 32;
    } reg;
}isp_bus_packet_probe_counters_7_val_t;

#endif
