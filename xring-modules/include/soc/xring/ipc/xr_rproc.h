/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_RPROC_H__
#define __XR_RPROC_H__

#include <linux/errno.h>

#include <soc/xring/ipc/xr_ipc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

struct rproc_list {
	int *vc_list;
	int vc_num;
};

/* used to get vc_id information */
struct rproc_list *xr_rproc_list_get(void);
/* used to release vc_id list information */
int xr_rproc_list_free(struct rproc_list *list);
int xr_mbox_fastack_sta(unsigned int ipc_id);

int xr_rproc_send_async(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len,
			ack_callback_t cb, void *arg);
int xr_rproc_async_fifo_reset(unsigned int ipc_id);
int xr_rproc_send_sync(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len);
int xr_rproc_send_resp(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len);
int xr_rproc_send_recv(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len,
			unsigned int *rx_buffer, unsigned int rx_buffer_len);

/****************************************************************************
 * Function:  xr_rproc_recv_register
 * NOTICE: Forbid the use of functions in callbacks that may cause sleep！
 ****************************************************************************/
int xr_rproc_recv_register(unsigned int ipc_id, struct notifier_block *nb);
int xr_rproc_recv_unregister(unsigned int ipc_id, struct notifier_block *nb);
int xr_rproc_fast_ack_recv_register(unsigned int ipc_id, fast_ack_callback_t cb, void *arg);
int xr_rproc_fast_ack_recv_unregister(unsigned int ipc_id);

void xr_rproc_dump_async_cnt(unsigned int ipc_id);

#endif /* __XR_RPROC_H__ */
