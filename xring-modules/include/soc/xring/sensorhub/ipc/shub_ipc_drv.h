/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __SHUB_IPC_DRV_H__
#define __SHUB_IPC_DRV_H__

#include <dt-bindings/xring/platform-specific/ipc_protocol_sh.h>

typedef int (*ipc_notifier_callback_t)(struct ipc_pkt *pkt, u16 tag, u8 cmd, void *arg);

struct ipc_notifier_info {
	ipc_notifier_callback_t callback;
	void *arg;
};

int sh_ipc_send_msg_sync(u8 vc_id, u16 tag, u8 cmd, void *msg, u16 msg_size);
int sh_ipc_send_msg_async(u8 vc_id, u16 tag, u8 cmd, void *msg, u16 msg_size,
	void (*complete)(int result, void *arg), void *arg);
int sh_ipc_send_with_resp_msg(u8 vc_id, u16 tag, u8 cmd, void *msg, u16 msg_size,
	void *rx_buff, u16 rx_len);
int sh_register_recv_notifier(u8 vc_id, u16 tag, struct ipc_notifier_info *notify_info);
int sh_unregister_recv_notifier(u8 vc_id, u16 tag, struct ipc_notifier_info *notify_info);
struct ipc_shm *sh_ipc_get_shm(struct ipc_shm_pkt *shm_pkt);

#endif /* __SHUB_IPC_DRV_H__ */
