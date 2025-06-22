// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __IPC_PROTOCOL_SH_H__
#define __IPC_PROTOCOL_SH_H__

#include "ipc_protocol.h"
#include "sensorhub_ddr_layout.h"

#define AP_IPC_SHM_AP2SH_ADDR          (SHUB_NS_DDR_SHM_IPC_ADDR)
#define AP_IPC_SHM_AP2SH_SIZE          (0x30000)
#define AP_IPC_SHM_SH2AP_ADDR          (SHUB_NS_DDR_SHM_IPC_ADDR + AP_IPC_SHM_AP2SH_SIZE)
#define AP_IPC_SHM_SH2AP_SIZE          (0x40000)

#define SH_IPC_SHM_AP2SH_ADDR          (MCU_SHUB_NS_DDR_SHM_IPC_ADDR)
#define SH_IPC_SHM_AP2SH_SIZE          (0x30000)
#define SH_IPC_SHM_SH2AP_ADDR          (MCU_SHUB_NS_DDR_SHM_IPC_ADDR + SH_IPC_SHM_AP2SH_SIZE)
#define SH_IPC_SHM_SH2AP_SIZE          (0x40000)


#define IPC_SHM_MAGIC                  (0x55AA)
#define IPC_SHM_PKT_SIZE               (sizeof(struct ipc_shm_pkt))
#define IPC_SHM_PKT_SHM_INFO_SIZE      (IPC_SHM_PKT_SIZE - IPC_PKT_HEADER_SIZE)
#define IPC_SHM_PKT_DATA_HEADER_SIZE   (sizeof(struct ipc_shm))
#define IPC_SHM_PKT_DATA_SIZE          (64 * 1024 - sizeof(struct ipc_shm))


struct ipc_shm_pkt {
	struct ipc_pkt_header header;
	uint32_t shm_addr_offset;
	uint32_t shm_len;
};


struct ipc_shm {
	uint16_t bf:1;
	uint16_t magic:15;
	uint16_t data_len;
	uint8_t data[0];
};

#endif
