// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __APP_ROUTER_MULTIPLEX__
#define __APP_ROUTER_MULTIPLEX__

#include "../../app_data_transfer/inc/shub_adt.h"
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/atomic.h>

#define MAX_MULTIPLEX_CLIENT 8

struct private_data_s {
	void *buffer;
	wait_queue_head_t ready;
	bool sensor_reset;
};

struct app_router_multiplex {
	struct adt_instance *service;
	struct private_data_s *private_datas[MAX_MULTIPLEX_CLIENT];
	struct buffer_s *buffers[MAX_MULTIPLEX_CLIENT];
	struct mutex lock;
	atomic_t sensor_framework_states;
	bool first_flag;
};

enum sensor_framework_states {
	STATES_UNKNOW,
	STATES_NOT_READY,
	STATES_READY,
};

int app_router_multiplex_init(struct app_router_multiplex **this,
			      struct port_ops *port, const uint32_t tx_buf_size,
			      const uint32_t rx_buf_size, const int service_id);

int app_router_multiplex_deinit(struct app_router_multiplex *this);

int app_router_multiplex_add_monitor(struct app_router_multiplex *this,
				     const int channel_id);

int app_router_multiplex_del_monitor(struct app_router_multiplex *this,
				     const int channel_id);

int app_router_multiplex_client_left(struct app_router_multiplex *this);

int app_router_multiplex_send(struct app_router_multiplex *this,
			      struct msg_meta const *const meta);

int app_router_multiplex_reg_hal(struct app_router_multiplex *this,
				 uint32_t index);

int app_router_multiplex_unreg_hal(struct app_router_multiplex *this,
				   uint32_t index);

int app_router_multiplex_report_exception(struct app_router_multiplex *this);

int app_router_multiplex_notify_exit(struct app_router_multiplex *this,
				     uint32_t index);

#endif
