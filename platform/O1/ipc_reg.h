// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef IPC_REGIF_H
#define IPC_REGIF_H

#define IPCMxSOURCE(m)                      ((m) * 0x40)
#define IPCMx_SR_IPCMxSOURCE_SHIFT          0
#define IPCMx_SR_IPCMxSOURCE_MASK           0x00000000

#define IPCMxDSET(m)                        (((m) * 0x40) + 0x004)
#define IPCMx_DSET_IPCMxDSET_SHIFT          0
#define IPCMx_DSET_IPCMxDSET_MASK           0x00000000

#define IPCMxDCLEAR(m)                      (((m) * 0x40) + 0x008)
#define IPCMx_DCLR_IPCMxDCLEAR_SHIFT        0
#define IPCMx_DCLR_IPCMxDCLEAR_MASK         0x00000000

#define IPCMxDSTATUS(m)                     (((m) * 0x40) + 0x00C)

#define IPCMxMODE(m)                        (((m) * 0x40) + 0x010)
#define IPCMx_MODE_IPCMx_AUTO_ACK_SHIFT     0
#define IPCMx_MODE_IPCMx_AUTO_ACK_MASK      0x00000001
#define IPCMx_MODE_IPCMx_AUTO_LINK_SHIFT    1
#define IPCMx_MODE_IPCMx_AUTO_LINK_MASK     0x00000002

#define IPCMxMSET(m)                        (((m) * 0x40) + 0x014)
#define IPCMx_MSET_IPCMxMSET_SHIFT          0
#define IPCMx_MSET_IPCMxMSET_MASK           0x00000000

#define IPCMxMCLEAR(m)                      (((m) * 0x40) + 0x018)
#define IPCMx_MCLR_IPCMxMCLEAR_SHIFT        0
#define IPCMx_MCLR_IPCMxMCLEAR_MASK         0x00000000

#define IPCMxMSTATUS(m)                     (((m) * 0x40) + 0x01C)

#define IPCMxSEND(m)                        (((m) * 0x40) + 0x020)
#define IPCMx_SEND_IPCMx_SEND_SHIFT         0
#define IPCMx_SEND_IPCMx_SEND_MASK          0x00000003

#define IPCMxDR(m, dr)                      (((m) * 0x40) + ((dr) * 4) + 0x024)
#define IPCMx_DAT_IPCMx_DR_SHIFT            0
#define IPCMx_DAT_IPCMx_DR_MASK             0x00000000

#define IPCMMIS(irq)                        (((irq) * 8) + 0x800)
#define IPCMRIS(irq)                        (((irq) * 8) + 0x804)

#define IPCM_CFG_STAT                       0x0900

#define IPCMxDR_NEW(m, dr)                  (((m) * 0x20) + ((dr) * 4) + 0x0a00)
#define IPCMx_DAT_IPCMx_DR_SHIFT            0
#define IPCMx_DAT_IPCMx_DR_MASK             0x00000000

#define IPCM_TCR                            0x0f00
#define IPCM_TCR_ITEN_SHIFT                 0
#define IPCM_TCR_ITEN_MASK                  0x00000001
#define IPCM_TOR                            0x0f04
#define IPCM_TOR_INTTEST_SHIFT              0
#define IPCM_TOR_INTTEST_MASK               0x00000000
#define IPCMPERIPHID0                       0x0fe0
#define IPCMPERIPHID1                       0x0fe4
#define IPCMPERIPHID2                       0x0fe8
#define IPCMPERIPHID3                       0x0fec
#define IPCMPCELLID0                        0x0ff0
#define IPCMPCELLID1                        0x0ff4
#define IPCMPCELLID2                        0x0ff8
#define IPCMPCELLID3                        0x0ffc

#endif
