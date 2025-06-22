// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include "inc/shub_app_router_multiplex.h"
#include "../app_data_transfer/inc/shub_adt.h"
#include "../app_data_transfer/private_inc/shub_buffer.h"
#include <linux/errno.h>
#include <linux/printk.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/wait.h>

struct packet_warpper {
	struct adt_packet *packet;
	uint32_t packet_size;
};

static uint32_t write_data(uint8_t *first_clip, uint32_t first_len,
			   uint8_t *second_clip, uint32_t second_len, void *arg)
{
	struct packet_warpper *warpper = (struct packet_warpper *)arg;
	struct buffer_state state = { .first_clip = first_clip,
				      .first_len = first_len,
				      .second_clip = second_clip,
				      .second_len = second_len };
	return adt_write_block(&state, warpper->packet, warpper->packet_size);
}

static void handle_event(struct adt_packet *packet, uint16_t packet_size,
			 void *arg)
{
	struct app_router_multiplex *this = (struct app_router_multiplex *)arg;
	uint8_t index = 0;
	int res = 0;
	int i;
	int tmp_states;
	struct buffer_s *buffer;
	struct packet_warpper warpper = { .packet = packet,
					  .packet_size = packet_size };

	if (packet->type == CONNECT_PACKET) {
		tmp_states = atomic_read(&this->sensor_framework_states);
		atomic_set(&this->sensor_framework_states, STATES_READY);
		if (!this->first_flag)
			app_router_multiplex_report_exception(this);
		this->first_flag = false;

		return;
	}

	mutex_lock(&this->lock);
	for (i = 0; i < MAX_MULTIPLEX_CLIENT; i++) {
		index = 1u << i;

		if (packet->bit_flag & index) {
			buffer = this->buffers[i];
			if (!buffer)
				continue;
			if (buffer_left(buffer) < packet_size)
				continue;
			res = buffer_write(buffer, write_data,
					   (void *)&warpper);
			if (res > 0) {
				wake_up_interruptible(
					&this->private_datas[i]->ready);
			}
		}
	}
	mutex_unlock(&this->lock);
}

int app_router_multiplex_client_left(struct app_router_multiplex *this)
{
	int i;
	int all_num = 0;

	for (i = 0; i < MAX_MULTIPLEX_CLIENT; i++) {
		if (this->buffers[i])
			all_num++;
	}
	return MAX_MULTIPLEX_CLIENT - all_num;
}

int app_router_multiplex_init(struct app_router_multiplex **this,
			      struct port_ops *port, const uint32_t tx_buf_size,
			      const uint32_t rx_buf_size, const int service_id)
{
	int err = 0;
	struct adt_api api = { .event_cb = handle_event,
			       .rsp_cb = handle_event };
	struct app_router_multiplex *tmp;
	struct adt_meta_s adt_meta;

	if (!this || !port || !service_id)
		return -EINVAL;

	tmp = kmalloc(sizeof(struct app_router_multiplex), GFP_KERNEL);
	if (!tmp) {
		err = -ENOMEM;
		goto fail;
	}
	memset(tmp, 0, sizeof(struct app_router_multiplex));
	tmp->first_flag = true;

	*this = tmp;
	adt_meta.api = &api;
	adt_meta.port = port;
	adt_meta.service_id = service_id;
	adt_meta.tx_buf_size = tx_buf_size;
	adt_meta.rx_buf_size = rx_buf_size;

	err = adt_acquire(&tmp->service, &adt_meta, (void *)*this);
	if (err)
		goto fail;
	mutex_init(&tmp->lock);
	atomic_set(&tmp->sensor_framework_states, STATES_READY);

	return 0;

fail:
	kfree(tmp);
	*this = NULL;
	return err;
}

static int init_private_data(struct app_router_multiplex *this,
			     struct private_data_s **private_data)
{
	struct private_data_s *tmp =
		kmalloc(sizeof(struct private_data_s), GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;
	tmp->buffer = kmalloc(this->service->tx->size, GFP_KERNEL);
	if (!tmp->buffer) {
		kfree(tmp);
		return -ENOMEM;
	}
	memset(tmp->buffer, 0, this->service->tx->size);
	tmp->sensor_reset = false;
	init_waitqueue_head(&tmp->ready);
	*private_data = tmp;
	return 0;
}

static int deinit_private_data(struct private_data_s *private_data)
{
	if (!private_data)
		return 0;
	kfree(private_data->buffer);
	kfree(private_data);
	return 0;
}

int app_router_multiplex_deinit(struct app_router_multiplex *this)
{
	int i;

	mutex_destroy(&this->lock);
	adt_release(this->service, NULL);
	for (i = 0; i < MAX_MULTIPLEX_CLIENT; i++) {
		buffer_deinit(this->buffers[i]);
		deinit_private_data(this->private_datas[i]);
	}
	kfree(this);
	return 0;
}

int app_router_multiplex_add_monitor(struct app_router_multiplex *this,
				     const int channel_id)
{
	if (!this)
		return -EINVAL;
	return adt_add_monitor(this->service, channel_id);
}

int app_router_multiplex_del_monitor(struct app_router_multiplex *this,
				     const int channel_id)
{
	if (!this)
		return -EINVAL;
	return adt_del_monitor(this->service, channel_id);
}

int app_router_multiplex_send(struct app_router_multiplex *this,
			      struct msg_meta const *const meta)
{
	int res;

	if (!this || !meta)
		return -EINVAL;
	mutex_lock(&this->lock);
	res = adt_send(this->service, meta);
	mutex_unlock(&this->lock);
	return res;
}

int app_router_multiplex_reg_hal(struct app_router_multiplex *this,
				 uint32_t index)
{
	int err = 0;

	if (!this)
		return -EINVAL;
	if (index >= MAX_MULTIPLEX_CLIENT)
		return -EINVAL;

	mutex_lock(&this->lock);
	if (this->buffers[index]) {
		err = -EEXIST;
		goto buffer_init_fail;
	}
	err = buffer_init(&this->buffers[index], this->service->rx->size);
	if (err)
		goto buffer_init_fail;
	err = init_private_data(this, &this->private_datas[index]);
	if (err)
		goto fail;
	mutex_unlock(&this->lock);
	return err;

fail:
	buffer_deinit(this->buffers[index]);
buffer_init_fail:
	mutex_unlock(&this->lock);
	return err;
}

int app_router_multiplex_unreg_hal(struct app_router_multiplex *this,
				   uint32_t index)
{
	if (!this)
		return -EINVAL;
	if (index >= MAX_MULTIPLEX_CLIENT)
		return -EINVAL;

	mutex_lock(&this->lock);
	if (this->private_datas[index]) {
		deinit_private_data(this->private_datas[index]);
		this->private_datas[index] = NULL;
	}
	if (this->buffers[index])
		buffer_deinit(this->buffers[index]);
	this->buffers[index] = NULL;
	mutex_unlock(&this->lock);
	return 0;
}

int app_router_multiplex_report_exception(struct app_router_multiplex *this)
{
	int i;

	mutex_lock(&this->lock);
	for (i = 0; i < MAX_MULTIPLEX_CLIENT; i++) {
		if (this->private_datas[i]) {
			this->private_datas[i]->sensor_reset = true;
			wake_up_interruptible(&this->private_datas[i]->ready);
		}
	}
	mutex_unlock(&this->lock);
	return 0;
}

int app_router_multiplex_notify_exit(struct app_router_multiplex *this,
				     uint32_t index)
{
	struct msg_meta meta = {
		.msg = NULL,
		.msg_len = 0,
		.need_rsp = false,
		.is_response = false,
		.bit_flag = (1 << index),
		.max_latency = 0,
		.disconnect = true,
	};

	if (!this || index >= MAX_MULTIPLEX_CLIENT)
		return -EINVAL;

	pr_info("hal client %u exit!\n", index);
	return app_router_multiplex_send(this, &meta);
}
