/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 *
 * Description: nv interface header
 */
#ifndef __XR_NV_INTERFACE_H__
#define __XR_NV_INTERFACE_H__

#include <linux/types.h>

#define NV_ITEM_HEADER_SIZE     sizeof(struct nv_item_header)
#define NV_ITEM_SIZE            128
#define NV_DATA_SIZE            (NV_ITEM_SIZE - NV_ITEM_HEADER_SIZE)
#define NV_READ                 0
#define NV_WRITE                1

enum nv_item_id {
	NV_HEADER_ID          =  0x0,
	NV_UART_CHANNEL_1     =  0x1,
	NV_UART_CHANNEL_2     =  0x2,
	NV_UART_CHANNEL_3     =  0x3,
	NV_UART_CHANNEL_4     =  0x4,
	NV_DFX_SWITCH_CHANNEL =  0x5,
	NV_UEFI_MP_CHANNEL    =  0x6,
	NV_UEFI_COUL_CHANNEL0 =  0x19,
	NV_UEFI_COUL_CHANNEL1 =  0x1a,
	NV_UEFI_COUL_CHANNEL2 =  0x1b,
	NV_UEFI_COUL_CHANNEL3 =  0x1c,

	/* NV Item  Max id: 0x400, Not allowed to exceed */
	NV_ITEM_ID_MAX        =  0x400
};

struct nv_item_header {
	u32 crc32;
	u16 item_index;
	u16 valid_size;
	u16 nv_update;
	u16 key;
};

struct nv_item {
	struct nv_item_header nv_header;
	u8 nv_data[NV_DATA_SIZE];
};

struct nv_info_struct {
	u8 operation;
	enum nv_item_id item_id;
	struct nv_item buf;
};

int nv_read_write_item(struct nv_info_struct *nv_info);

#endif /* __XR_NV_INTERFACE_H__ */
