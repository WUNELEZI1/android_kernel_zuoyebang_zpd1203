/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_IPC_PROT_H__
#define __XR_IPC_PROT_H__

#include <dt-bindings/xring/platform-specific/ipc_protocol.h>
#include "xr_ipc.h"

#define IPC_PROT_TEST

typedef int (*notifier_handler_t)(struct ipc_pkt *pkt, u16 ipc_tag, u8 cmd, void *arg);

struct notifier_info {
	notifier_handler_t handler;
	struct notifier_block nb;
	struct list_head node;
	void *arg;
};

/****************************************************************************
 * Function: ap_ipc_send_sync
 *
 * Description:
 *  By sending messages synchronously.
 *
 * Input Parameters:
 *  @vc_id: Virtual ID number.
 *  @ipc_tag: High 16 bit is vc_id, low 16 bit is module tag.
 *  @cmd: Module internal instructions
 *  @tx_buff: Message to be sent.
 *  @tx_len: Message size (unit byte)
 *
 * Return:
 *  @OK: send successfully.
 *  @!OK: send failed.
 ****************************************************************************/
int ap_ipc_send_sync(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len);

/****************************************************************************
 * Function: ap_ipc_send_async
 *
 * Description:
 *  By sending messages asynchronously.
 *
 * Input Parameters:
 *  @vc_id: Virtual ID number.
 *  @ipc_tag: High 16 bit is vc_id, low 16 bit is module tag.
 *  @cmd: Module internal instructions.
 *  @ack_callback_t: Send async callback.
 *  @arg: Callback argue.
 *
 * Return:
 *  @OK: send successfully.
 *  @!OK: send failed.
 ****************************************************************************/
int ap_ipc_send_async(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len, ack_callback_t cb, void *arg);

/****************************************************************************
 * Function: ap_ipc_send_recv
 *
 * Description:
 *  By sending messages synchronously, receives the return message.
 *
 * Input Parameters:
 *  @vc_id: Virtual ID number.
 *  @ipc_tag: High 16 bit is vc_id, low 16 bit is module tag.
 *  @cmd: Module internal instructions.
 *  @tx_buff: Message to be sent.
 *  @tx_len: Message size (unit byte).
 *
 * Return:
 *  @OK: send successfully.
 *  @!OK: send failed.
 ****************************************************************************/
int ap_ipc_send_recv(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len, void *rx_buff, size_t rx_len);

/****************************************************************************
 * Function: ap_ipc_send_resp
 *
 * Description:
 *  By sending response messages.
 *
 * Input Parameters:
 *  @vc_id: Virtual ID number.
 *  @ipc_tag: High 16 bit is vc_id, low 16 bit is module tag.
 *  @cmd: Module internal instructions
 *  @tx_buff: Message to be sent.
 *  @tx_len: Message size (unit byte)
 *
 * Return:
 *  @OK: send successfully.
 *  @!OK: send failed.
 ****************************************************************************/
int ap_ipc_send_resp(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len);

/****************************************************************************
 * Function: ipc_send_shm_pkt
 *
 * Description:
 *  By sending shared memary packets.
 *
 * Input Parameters:
 *  @vc_id: Virtual ID number.
 *  @ipc_tag: High 16 bit is vc_id, low 16 bit is module tag.
 *  @cmd: Module internal instructions
 *  @shm_addr_offset: shared memary.
 *  @shm_len: shared memary length.
 *
 * Return:
 *  @OK: send successfully.
 *  @!OK: send failed.
 ****************************************************************************/
int ipc_send_shm_pkt(u8 vc_id, u16 ipc_tag, u8 cmd,
		u32 shm_addr_offset, u16 shm_len);

/****************************************************************************
 * Function:  ap_ipc_recv_register
 *
 * Description:
 *   User's interface used to register a callback function.
 *   NOTICE: Forbid the use of functions in callbacks that may cause sleep！
 *
 * Input Parameters:
 *  @vc_id: Virtual ID number.
 *  @ipc_tag: specific tag.
 *  @nb: callback function information.
 *  @arg: callback argument.
 *
 * Return:
 *  @OK: register successfully.
 *  @!OK: register failed.
 *
 ****************************************************************************/
int ap_ipc_recv_register(u8 vc_id, u16 ipc_tag,
		notifier_handler_t nb, void *arg);

/****************************************************************************
 * Function:  ap_ipc_recv_unregister
 *
 * Description:
 *   User's interface used to unregister a callback function.
 *
 * Input Parameters:
 *  @vc_id: Virtual ID number.
 *  @ipc_tag: specific tag.
 *  @nb: callback function information.
 *
 * Return:
 *  @OK: register successfully.
 *  @!OK: register failed.
 *
 ****************************************************************************/
int ap_ipc_recv_unregister(u8 vc_id, u16 ipc_tag, notifier_handler_t nb);

#ifdef IPC_PROT_TEST
/****************************************************************************
 * Function: ap_ipc_send_lose_seq_test
 *
 * Description:
 *  Seqid is tested to determine the packet loss when sending the message.
 *
 * Input Parameters:
 *  @vc_id: Virtual ID number.
 *  @ipc_tag: High 16 bit is vc_id, low 16 bit is module tag.
 *  @cmd: Module internal instructions
 *  @tx_buff: Message to be sent.
 *  @tx_len: Message size (unit byte)
 *
 * Return:
 *  @OK: send successfully(test failed).
 *  @!OK: send failed(test successfully).
 ****************************************************************************/
int ap_ipc_send_lose_seq_test(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len);

/****************************************************************************
 * Function: ap_ipc_send_repe_pkt_test
 *
 * Description:
 *  Subid is tested to determine the packet repetitious when sending the message.
 *
 * Input Parameters:
 *  @vc_id: Virtual ID number.
 *  @ipc_tag: High 16 bit is vc_id, low 16 bit is module tag.
 *  @cmd: Module internal instructions
 *  @tx_buff: Message to be sent.
 *  @tx_len: Message size (unit byte)
 *
 * Return:
 *  @OK: send successfully(test failed).
 *  @!OK: send failed(test successfully).
 ****************************************************************************/
int ap_ipc_send_repe_pkt_test(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len);
#endif

#endif /* __XR_IPC_PROT_H__ */
