/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_IPC_TRACE_HOOK_H__
#define __XR_IPC_TRACE_HOOK_H__

#define IPC_DIRECTION_IN		(1)
#define IPC_DIRECTION_OUT		(0)
#define IPC_IRQ_TOP			(1)
#define IPC_IRQ_LOW			(0)

#if IS_ENABLED(CONFIG_XRING_IPC_HOOK)
int ipc_trace_hook_init(void);
void ipc_trace_hook_exit(void);
void ipc_trace_hook_call_chain(u8 vc_id, u8 irq_sta, u8 dir);
#else
static inline int ipc_trace_hook_init(void)
{
	return 0;
}
static inline void ipc_trace_hook_exit(void)
{
}
static inline void ipc_trace_hook_call_chain(u8 vc_id, u8 irq_sta, u8 dir)
{
}
#endif

#endif  /* __XR_IPC_TRACE_HOOK_H__ */
