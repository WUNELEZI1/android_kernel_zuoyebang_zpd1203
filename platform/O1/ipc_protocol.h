// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __IPC_PROTOCOL_H__
#define __IPC_PROTOCOL_H__

#define IPC_PKT_MAX_SIZE               (60)
#define IPC_PKT_HEADER_SIZE            (sizeof(struct ipc_pkt_header))
#define IPC_LONG_PKT_SPLIT_NUM         (8)

#define IPC_SEND_NO_RESP               (0)
#define IPC_SEND_NEED_RESP             (1)

#define IPC_PKT_NOTEND                 (0)
#define IPC_PKT_END                    (1)


#define IPC_SHORT_PKT_SIZE             (IPC_PKT_MAX_SIZE)
#define IPC_LONG_PKT_SIZE              (IPC_PKT_MAX_SIZE)

#define IPC_SHORT_PKT_DATA_SIZE        (IPC_SHORT_PKT_SIZE - IPC_PKT_HEADER_SIZE)
#define IPC_LONG_PKT_DATA_SIZE         (IPC_SHORT_PKT_DATA_SIZE * IPC_LONG_PKT_SPLIT_NUM)


enum ipc_pkt_type {
	IPC_SHORT_PKT = 0,
	IPC_LONG_PKT = 1u,
	IPC_SHM_PKT,
	IPC_FIXED_SHM_PKT,
	IPC_PKT_TYPE_INVALID,
};


struct ipc_pkt_header {
	uint8_t seqid;
	struct {
		uint8_t subid:4;
		uint8_t end:1;
		uint8_t resp:1;
		uint8_t type:2;
	} control;
	uint16_t tag;
	uint8_t cmd;
	uint8_t rsvd;
	uint16_t data_len;
};


struct ipc_pkt {
	struct ipc_pkt_header header;
	uint8_t data[0];
};

#endif
