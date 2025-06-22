// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __APP_ROUTER_ADT__
#define __APP_ROUTER_ADT__

#include "../private_inc/shub_buffer.h"
#include "shub_port.h"
#include "shub_protocol.h"
#include <linux/timer.h>
#include <linux/types.h>

struct adt_api {
	void (*event_cb)(struct adt_packet *packet, uint16_t packet_size,
			 void *arg);
	void (*rsp_cb)(struct adt_packet *packet, uint16_t packet_size,
		       void *arg);
};

struct channels {
	int id;
	struct list_head list;
};

struct adt_instance {
	struct port_ops *port;
	struct adt_api api;
	struct list_head channel_ids;
	int service_id;
	void *arg;
	struct buffer_s *rx;
	struct buffer_s *tx;
	struct task_struct *read_thread;
	struct task_struct *send_thread;
	unsigned long time;
	struct timer_list timer;
	int data_missed_num;
};

struct msg_meta {
	void *msg;
	uint16_t msg_len;
	bool need_rsp;
	bool is_response;
	uint8_t bit_flag;
	uint32_t max_latency;
	bool disconnect;
};

struct buffer_state {
	uint8_t *first_clip;
	uint32_t first_len;
	uint8_t *second_clip;
	uint32_t second_len;
};

struct adt_meta_s {
	struct adt_api *api;
	struct port_ops *port;
	uint32_t tx_buf_size;
	uint32_t rx_buf_size;
	int service_id;
};

int adt_lookup(struct adt_instance *this, const int service_id);

int adt_write_block(struct buffer_state const *const state,
		    void const *const msg, const uint32_t msg_len);

int adt_add_monitor(struct adt_instance *this, const int channel_id);

int adt_del_monitor(struct adt_instance *this, const int channel_id);

int adt_send(struct adt_instance *this, struct msg_meta const *const meta);

int adt_acquire(struct adt_instance **this,
		struct adt_meta_s const *const adt_meta, void *arg);

int adt_release(struct adt_instance *this, void **arg);

int adt_flush_tx(struct adt_instance *this);
#endif
