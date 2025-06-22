/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_CAM_RPC_SERVICE_H_
#define _XRISP_CAM_RPC_SERVICE_H_

#include <linux/workqueue.h>
#include <media/v4l2-event.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>

#define IPCMAXLEN (512)

typedef int (*rpc_func_cb)(void *, int);

struct rpc_msg_head {
	uint32_t   msg_id;
	uint32_t   func_hash;
	uint8_t    ack;
	int32_t    head_index;
} __packed;

struct rpc_head {
	int               *rpc_ret;
	struct completion *sem;
	void              *result_data;
	uint32_t           result_len;
	rpc_func_cb        cb;
	uint8_t            msg_data[IPCMAXLEN];
	size_t             msg_size;
};

enum rpc_ack {
	RPC_SUCCESS = 0,
	RPC_FAIL    = 1,
	RPC_SERVICE = 2
};

int xrisp_cam_rpc_process(u8 *data, size_t size);
int xrisp_cam_rpc_send_ack(void *buffer, size_t size);
int xrisp_cam_rpc_send_msg(struct rpc_head *rpc_head);
int rpc_service_init(void);
int rpc_service_uninit(void);

extern int xrisp_cam_rpc_run_func(u8 *data, size_t size);

#endif
