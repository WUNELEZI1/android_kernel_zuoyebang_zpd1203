// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __APP_ROUTER_PORT__
#define __APP_ROUTER_PORT__

#include <linux/types.h>

typedef int (*rsp_cb)(void *msg, uint16_t msg_len, void *arg);

struct port_ops {
	int (*lookup)(const int id);

	int (*open)(const int id, rsp_cb cb, void *arg);
	int (*close)(const int id, void **arg);
	int (*send)(const int id, void *msg, uint16_t msg_len);

	int align;
	uint32_t max_len;
};

struct port_ops *get_ipc_port(void);

#endif
