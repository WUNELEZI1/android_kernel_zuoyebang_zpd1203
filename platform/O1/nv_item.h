// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */


#ifndef _NV_ITEM_H_
#define _NV_ITEM_H_

typedef enum {
  NV_HEADER_ID          =  0x0,
  NV_UART_CHANNEL_1     =  0x1,
  NV_UART_CHANNEL_2     =  0x2,
  NV_UART_CHANNEL_3     =  0x3,
  NV_UART_CHANNEL_4     =  0x4,
  NV_DFX_SWITCH_CHANNEL =  0x5,
  NV_UEFI_MP_CHANNEL    =  0x6,
  NV_RMEM_SWITCH_CHANNEL=  0x7,
  NV_SN_CHANNEL         =  0x18,
  NV_UEFI_COUL_CHANNEL0 =  0x19,
  NV_FASTBOOT_SHUTDOWN  =  0x1D,
  NV_XHEE_CHANNEL       =  0x29,
  NV_XSPM_CHANNEL       =  0x2A,


  NV_ITEM_ID_MAX        =  0x400
} NV_ITEM_ID;

#endif
