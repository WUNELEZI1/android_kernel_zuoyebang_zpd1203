/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_IPC_MAIN_H__
#define __XR_IPC_MAIN_H__

#if IS_ENABLED(CONFIG_XRING_IPC)
int xr_ipc_dev_init(void);
void xr_ipc_dev_exit(void);
#else
static int xr_ipc_dev_init(void) { return 0; }
static void xr_ipc_dev_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_IPC_PROT)
int xr_ipc_prot_init(void);
void xr_ipc_prot_exit(void);
#else
static int xr_ipc_prot_init(void) { return 0; }
static void xr_ipc_prot_exit(void) { }
#endif

#endif /* __XR_IPC_MAIN_H__ */
