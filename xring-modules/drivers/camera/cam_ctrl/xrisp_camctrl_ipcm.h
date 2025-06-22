/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_CAMCTRL_IPCM_H_
#define _XRISP_CAMCTRL_IPCM_H_

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/workqueue.h>

#define IPC_MAX_USE_TIME_US	(50000)
#define MEMPOOL_WORK_ENTITY_MAX (16)

#define TOPIC_LOG	     (0x01)
#define TOPIC_PPM	     (0x02)
#define TOPIC_ISP_RPC_KERNEL (0x03)
#define TOPIC_ISP_RPC_HAL    (0x04)
#define TOPIC_ISP_RPC	     (0x05)
#define TOPIC_EDR	     (0x64)
#define TOPIC_DRV	     (0x65)
#define TOPIC_MBOX_DEBUG     (0x10)
#define XRISP_TOPIC_MAX	     (128)

typedef int (*msg_cb)(void *data, unsigned int len);

enum xrisp_ipc_type {
	IPC_RPMSG,
	IPC_MBOX,
};

enum xrisp_msg_recv_cb_mode {
	IPCM_ORDERED_HANDLE = 0, //irq bottom, public wq, ordered handle
	SYSTEM_HIGHPRI_HANDLE,	 //irq bottom, highpri wq, direct handle
	FAST_ORDERED_HANDLE,	 //irq bottom, fast wq, ordered handle
	FAST_DIRECT_HANDLE,	 //irq top, Must fast, Don't sleep or schedule
	MSG_HANDLE_MODE_MAX,
};

struct topic_handle_t {
	bool is_registered;
	uint32_t mode;
	msg_cb cb;
};

struct ipcm_ctrl_t {
	struct topic_handle_t topic_handle_list[XRISP_TOPIC_MAX];
	struct workqueue_struct *ipcm_order_wq;
	struct workqueue_struct *fast_order_wq;
	struct mutex ipcm_mtx;
};

struct ipc_work_t {
	struct work_struct workq;
	s64 cnt;
	ktime_t start_time;
	uint8_t ipc_type;
	uint8_t topic;
	msg_cb cb;
	uint32_t len;
	unsigned char data[512];
};

struct xrisp_ipc_trace {
	atomic64_t msg_send_cnt;
	uint64_t recv_max_time;
	uint64_t recv_avg_time;
	uint64_t recv_total_time;

	atomic64_t msg_recv_cnt;
	uint64_t send_max_time;
	uint64_t send_avg_time;
	uint64_t send_total_time;
	struct mutex trace_lock;
};

int xrisp_msg_send(void *data, unsigned int len, uint8_t topic);
int xrisp_msg_send_core(void *data, uint32_t len, uint8_t topic, bool data_from_user);

int xrisp_cam_rpc_send_kernel_msg(void *data, unsigned int len);

void ipc_trace_cnt_init(void);

int xrisp_topic_handle_register(uint8_t topic, msg_cb cb, uint32_t mode);
void xrisp_topic_handle_unregister(uint8_t topic);

int xrisp_ipcm_init(void);
void xrisp_ipcm_exit(void);

#endif
