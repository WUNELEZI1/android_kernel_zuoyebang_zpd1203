/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_IPC_H__
#define __XR_IPC_H__

#include <dt-bindings/xring/platform-specific/ipc_protocol.h>

#define MBOX_MAX_DATA_REG		(15)
#define IPC_MBOX_B2B_DATA_SIZE		(IPC_PKT_MAX_SIZE * IPC_LONG_PKT_SPLIT_NUM)
#define MBOX_CHAN_DATA_SIZE		(MBOX_MAX_DATA_REG * 4)

typedef int (*mbox_irq_handler_t)(int irq, void *p);
typedef void (*ack_callback_t)(int result, void *arg);
typedef void (*fast_ack_callback_t)(void *rx_buff, void *arg);

struct xr_mbox_task {
	/* use static memory to cache the async tx buffer*/
	unsigned int	tx_buffer[IPC_MBOX_B2B_DATA_SIZE];
	/* alloc by mailbox core, shouldn't be free when a tx task complete by mailbox users */
	unsigned int	*rx_buffer;
	unsigned int	tx_buffer_len;
	unsigned int	rx_buffer_len;
	int		send_len;
	int		need_auto_ack;
	ack_callback_t	ack_cb;
	void		*ack_arg;
	/* for performance */
#ifdef CONFIG_XR_MAILBOX_PERFORMANCE_DEBUG
	int		perf_debug;
	struct timespec	tts[IPC_TTS_MAX];
#endif
};

#endif /* __XR_IPC_H__ */
