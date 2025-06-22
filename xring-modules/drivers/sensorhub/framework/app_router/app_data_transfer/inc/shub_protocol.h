// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __APP_ROUTER__PROTOCOL__
#define __APP_ROUTER__PROTOCOL__

#include <linux/types.h>

enum adt_type {
	INVALID_PACKET,
	EMPTY_PACKET,
	NORMAL_PACKET,
	REQUEST_PACKET,
	RSP_PACKET,
	DISCARD_PACKET,
	DISCONNECT_PACKET,
	CONNECT_PACKET,
	PACKET_END
};

struct adt_packet {
	uint8_t type : 4;
	uint8_t magic : 4;
	uint8_t bit_flag;
	uint16_t data_len;
	uint8_t data[0];
};

#endif