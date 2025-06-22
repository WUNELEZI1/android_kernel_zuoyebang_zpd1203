/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_IPCM_API_H__
#define __XRING_IPCM_API_H__

int xrisp_rpmsg_register_recv_cb(void *priv,
	void (*recv_cb)(void *priv, void *data, int len));

int xrisp_rpmsg_send(void *data, int len);

#endif /* __XRING_IPCM_API_H__ */
