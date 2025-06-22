// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __FIRQ_H__
#define __FIRQ_H__

typedef int(*firq_recv_callback_t) (uint32_t buf_addr, uint32_t buf_size);

int firq_recv_cb_register(int chan, firq_recv_callback_t recv_callback);
int firq_recv_cb_unregister(int chan);

#endif
