/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_TEST_H__
#define __XR_TEST_H__

#include <soc/xring/ipc/xr_ipc.h>

/* IOCTL commands */

#define IPC_IOC_MAGIC	'k'

enum ipc_ioc_num {
	RX_REGISTER = 0,
	SEND_ASYNC,
	SEND_SYNC,
	SEND_AND_RECV,
	SEND_RESP,
	IOC_BOUNDARY,
};

#define IPC_IOC_WR_RX_REGISTER		_IOW(IPC_IOC_MAGIC, RX_REGISTER, struct xr_ipc_test *)
#define IPC_IOC_WR_SEND_ASYNC		_IOW(IPC_IOC_MAGIC, SEND_ASYNC, struct xr_ipc_test *)
#define IPC_IOC_WR_SEND_SYNC		_IOW(IPC_IOC_MAGIC, SEND_SYNC, struct xr_ipc_test *)
#define IPC_IOC_WR_SEND_AND_RECV	_IOW(IPC_IOC_MAGIC, SEND_AND_RECV, struct xr_ipc_test *)
#define IPC_IOC_WR_SEND_RESP		_IOW(IPC_IOC_MAGIC, IOC_BOUNDARY, struct xr_ipc_test *)

struct xr_ipc_test {
	unsigned int ipc_id;
	unsigned int tx_buffer[MBOX_MAX_DATA_REG];
	unsigned int tx_buffer_len;

	unsigned int rx_buffer[MBOX_MAX_DATA_REG];
	unsigned int rx_buffer_len;
	struct notifier_block nb;
};

#endif /* __XR_TEST_H__ */
