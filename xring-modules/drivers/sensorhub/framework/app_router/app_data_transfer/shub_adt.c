// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include "inc/shub_adt.h"
#include "inc/shub_protocol.h"
#include <linux/align.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/printk.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/delay.h>

#define MAGIC 0xA
#define LPIS_ALIGN 4
#define ADT_SEND_RETRY_COUNT 5

struct msg_warpper {
	void *msg;
	uint16_t msg_len;
};

static bool data_droped;
static bool need_print;

int adt_write_block(struct buffer_state const *const state,
		    void const *const msg, const uint32_t msg_len)
{
	if (!state || !msg)
		return -EINVAL;
	if (!state->first_clip || !state->second_clip)
		return -EFAULT;

	if (msg_len <= state->first_len) {
		memcpy(state->first_clip, msg, msg_len);
		return msg_len;
	}
	if (msg_len > state->second_len) {
		pr_err("overflow no continuous space first:%d second:%d msg_len:%d\n",
		       state->first_len, state->second_len, msg_len);
		return 0;
	}
	if (state->first_len) {
		struct adt_packet *packet =
			(struct adt_packet *)state->first_clip;
		packet->type = DISCARD_PACKET;
	}
	memcpy(state->second_clip, msg, msg_len);
	return state->first_len + msg_len;
}

static uint32_t write_rx(uint8_t *first_clip, uint32_t first_len,
			 uint8_t *second_clip, uint32_t second_len, void *arg)
{
	struct msg_warpper *msg = (struct msg_warpper *)arg;
	struct buffer_state state = { .first_clip = first_clip,
				      .first_len = first_len,
				      .second_clip = second_clip,
				      .second_len = second_len };
	if (!msg->msg_len)
		return 0;
	return adt_write_block(&state, msg->msg, msg->msg_len);
}

static int recv_data(void *msg, uint16_t msg_len, void *arg)
{
	struct adt_instance *this = (struct adt_instance *)arg;
	struct msg_warpper warpper = { .msg = msg, .msg_len = 0 };
	struct adt_packet *packet;
	void *r_current = msg;
	int left = buffer_left(this->rx);
	int payload_len = 0;
	int i;
	int tmp;

	for (i = 0; i < msg_len;) {
		packet = (struct adt_packet *)r_current;
		if (packet->magic != MAGIC)
			break;
		tmp = sizeof(struct adt_packet) + packet->data_len;
		if (i + tmp > msg_len)
			break;
		i += tmp;
		r_current += tmp;
		payload_len += tmp;
	}
	warpper.msg_len = payload_len;
	if (left < payload_len) {
		this->data_missed_num++;
		if (!data_droped) {
			need_print = true;
			data_droped = true;
		}
		return true;
	}
	data_droped = false;
	buffer_write_with_lock(this->rx, write_rx, (void *)&warpper);
	complete(&this->rx->ready);
	return true;
}

static void handle_event(struct adt_instance *this, struct adt_packet *packet,
			 uint16_t packet_size)
{
	if (!packet_size)
		return;
	switch (packet->type) {
	case DISCONNECT_PACKET:
	case NORMAL_PACKET:
	case REQUEST_PACKET:
	case CONNECT_PACKET:
		if (this->api.event_cb)
			this->api.event_cb(packet, packet_size, this->arg);
		break;
	case RSP_PACKET:
		if (this->api.rsp_cb)
			this->api.rsp_cb(packet, packet_size, this->arg);
		break;
	default:
		break;
	}
}

static uint32_t read_rx(uint8_t *first_clip, uint32_t first_len,
			uint8_t *second_clip, uint32_t second_len, void *arg)
{
	struct adt_instance *this = (struct adt_instance *)arg;
	uint8_t *r_current = first_clip;
	uint16_t packet_size;
	int len = first_len;
	int clip;
	int i;

	for (clip = 0; clip < 2; clip++) {
		for (i = 0; i < len;) {
			struct adt_packet *packet =
				(struct adt_packet *)r_current;
			if (packet->magic != MAGIC)
				break;
			if (packet->type == DISCARD_PACKET)
				break;
			packet_size =
				sizeof(struct adt_packet) + packet->data_len;

			if (i + packet_size > len)
				break;
			handle_event(this, packet, packet_size);
			i += packet_size;
			r_current += packet_size;
		}
		r_current = second_clip;
		len = second_len;
	}
	return first_len + second_len;
}

static int handle_rx_data(void *arg)
{
	struct adt_instance *this = (struct adt_instance *)arg;
	int res;

	allow_signal(SIGKILL);
	while (!kthread_should_stop()) {
		res = wait_for_completion_interruptible(&this->rx->ready);
		if (res) {
			pr_info("rx sema %d", res);
			break;
		}
		if (need_print) {
			pr_err_ratelimited("rx overflow missed num:%d\n",
					   this->data_missed_num);
			need_print = false;
		}
		buffer_read(this->rx, read_rx, arg);
	}
	pr_info("rx thread exit\n");
	return 0;
}

static uint32_t read_tx(uint8_t *first_clip, uint32_t first_len,
			uint8_t *second_clip, uint32_t second_len, void *arg)
{
	struct adt_instance *this = (struct adt_instance *)arg;
	int all_data = 0;
	int tmp;
	int clip;
	int i;
	int res;
	uint8_t *r_current = first_clip;
	int len = first_len;

	for (clip = 0; clip < 2; clip++) {
		for (i = 0; i < len;) {
			struct adt_packet *packet =
				(struct adt_packet *)r_current;
			if (packet->type == DISCARD_PACKET)
				break;
			tmp = sizeof(struct adt_packet) + packet->data_len;
			r_current += tmp;
			all_data += tmp;
			i += tmp;
		}
		res = 0;
		int retry_cnt = ADT_SEND_RETRY_COUNT;

		while (retry_cnt) {
			if (all_data)
				res = this->port->send(this->service_id,
							r_current - all_data, all_data);

			if (res < 0) {
				msleep(1);
				retry_cnt--;
				pr_err("send fail %d, retry:%d times left\n", res, retry_cnt);
			} else
				break;
		}
		all_data = 0;
		len = second_len;
		r_current = second_clip;
	}
	return first_len + second_len;
}

static int handle_tx_data(void *arg)
{
	struct adt_instance *this = (struct adt_instance *)arg;
	int res;

	allow_signal(SIGKILL);
	while (!kthread_should_stop()) {
		res = wait_for_completion_interruptible(&this->tx->ready);
		if (res) {
			pr_info("tx sema %d", res);
			break;
		}
		buffer_read(this->tx, read_tx, arg);
	}
	pr_info("tx thread exit\n");
	return 0;
}

int adt_lookup(struct adt_instance *this, const int service_id)
{
	if (!this)
		return -EINVAL;
	if (!this->port || !this->port->lookup)
		return -EFAULT;

	return this->port->lookup(service_id);
}
EXPORT_SYMBOL(adt_lookup);

static int open(struct adt_instance *this, const int channel_id)
{
	if (!this)
		return -EINVAL;
	if (!this->port || !this->port->open)
		return -EFAULT;

	return this->port->open(channel_id, recv_data, (void *)this);
}

static int close(struct adt_instance *this, const int channel_id, void **arg)
{
	int res;

	if (!this || !channel_id)
		return -EINVAL;
	if (!this->port || !this->port->close)
		return -EFAULT;

	res = this->port->close(channel_id, arg);
	if (res)
		return res;
	pr_info("adt clear: %d\n", channel_id);
	return 0;
}

static int adt_create_head(struct msg_meta const *const meta, void *begin)
{
	struct adt_packet *packet = (struct adt_packet *)begin;

	packet->magic = MAGIC;
	packet->data_len = meta->msg_len;
	if (meta->need_rsp)
		packet->type = REQUEST_PACKET;
	else if (meta->is_response)
		packet->type = RSP_PACKET;
	else if (meta->disconnect)
		packet->type = DISCONNECT_PACKET;
	else
		packet->type = NORMAL_PACKET;
	packet->bit_flag = meta->bit_flag;

	return 0;
}

static uint32_t write_tx(uint8_t *first_clip, uint32_t first_len,
			 uint8_t *second_clip, uint32_t second_len, void *arg)
{
	struct msg_meta *meta = (struct msg_meta *)arg;
	int all_len = sizeof(struct adt_packet) + meta->msg_len;
	struct adt_packet *head;

	if (first_len >= all_len) {
		adt_create_head(meta, first_clip);
		memcpy(first_clip + sizeof(struct adt_packet), meta->msg,
		       meta->msg_len);
		return all_len;
	}
	if (second_len < all_len)
		return 0;
	if (first_len) {
		head = (struct adt_packet *)first_clip;
		head->type = DISCARD_PACKET;
	}
	adt_create_head(meta, second_clip);
	memcpy(second_clip + sizeof(struct adt_packet), meta->msg,
	       meta->msg_len);
	return all_len + first_len;
}

void timer_callback(struct timer_list *timer)
{
	struct adt_instance *this = from_timer(this, timer, timer);

	adt_flush_tx(this);
}

int adt_send(struct adt_instance *this, struct msg_meta const *const meta)
{
	int res = 0;
	unsigned long tmp_time;

	if (!this)
		return -EINVAL;
	if (meta->need_rsp && meta->is_response)
		return -EINVAL;
	if (meta->msg_len & (LPIS_ALIGN - 1))
		return -EINVAL;

	res = buffer_write(this->tx, write_tx, (void *)meta);
	if (res < 0)
		return res;
	if (res == 0 || meta->max_latency == 0) {
		mod_timer(&this->timer, jiffies);
		return res;
	}

	tmp_time = jiffies + msecs_to_jiffies(meta->max_latency);
	if (timer_pending(&this->timer) && time_after_eq(tmp_time, this->time))
		return res;
	mod_timer(&this->timer, tmp_time);
	this->time = tmp_time;

	return res;
}
EXPORT_SYMBOL(adt_send);

int adt_flush_tx(struct adt_instance *this)
{
	complete(&this->tx->ready);
	return 0;
}
EXPORT_SYMBOL(adt_flush_tx);

int adt_acquire(struct adt_instance **this,
		struct adt_meta_s const *const adt_meta, void *arg)
{
	struct adt_instance *tmp;
	int err = 0;

	if (!this || !adt_meta)
		return -EINVAL;
	if (!adt_meta->api || !adt_meta->port)
		return -EINVAL;

	tmp = kmalloc(sizeof(struct adt_instance), GFP_KERNEL);
	if (!tmp) {
		err = -ENOMEM;
		goto fail;
	}
	memset(tmp, 0, sizeof(struct adt_instance));

	err = buffer_init(&tmp->rx, adt_meta->rx_buf_size);
	if (err)
		goto fail;
	err = buffer_init(&tmp->tx, adt_meta->tx_buf_size);
	if (err)
		goto fail;

	tmp->arg = arg;
	tmp->api = *adt_meta->api;
	tmp->port = adt_meta->port;
	tmp->service_id = adt_meta->service_id;
	INIT_LIST_HEAD(&tmp->channel_ids);
	*this = tmp;

	tmp->read_thread = kthread_run(handle_rx_data, (void *)tmp,
				       "service_%d_read", adt_meta->service_id);
	if (!tmp->read_thread) {
		err = -ECHILD;
		goto fail;
	}
	tmp->send_thread = kthread_run(handle_tx_data, (void *)tmp,
				       "service_%d_send", adt_meta->service_id);
	if (!tmp->send_thread) {
		err = -ECHILD;
		goto fail;
	}
	timer_setup(&tmp->timer, timer_callback, 0);

	return 0;

fail:
	if (!tmp)
		return err;
	if (tmp->tx)
		buffer_deinit(tmp->tx);
	if (tmp->rx)
		buffer_deinit(tmp->rx);
	if (tmp->send_thread)
		kill_pid(find_get_pid(tmp->send_thread->pid), SIGKILL, 1);
	if (tmp->read_thread)
		kill_pid(find_get_pid(tmp->read_thread->pid), SIGKILL, 1);
	kfree(tmp);
	*this = NULL;
	return err;
}
EXPORT_SYMBOL(adt_acquire);

int adt_release(struct adt_instance *this, void **arg)
{
	int res = 0;
	struct list_head *cur, *cur_bak;
	struct channels *channel;

	if (!this)
		return -EINVAL;
	if (arg)
		*arg = this->arg;

	del_timer(&this->timer);
	list_for_each_safe(cur, cur_bak, &this->channel_ids) {
		channel = list_entry(cur, struct channels, list);
		res |= close(this, channel->id, NULL);
		list_del(cur);
		kfree(channel);
	}
	if (this->send_thread)
		kill_pid(find_get_pid(this->send_thread->pid), SIGKILL, 1);
	if (this->read_thread)
		kill_pid(find_get_pid(this->read_thread->pid), SIGKILL, 1);
	if (this->tx)
		buffer_deinit(this->tx);
	if (this->rx)
		buffer_deinit(this->rx);

	kfree(this);

	return res;
}
EXPORT_SYMBOL(adt_release);

int adt_add_monitor(struct adt_instance *this, const int channel_id)
{
	int res;
	struct channels *channel = NULL;

	if (!this || !channel_id)
		return -EINVAL;

	list_for_each_entry(channel, &this->channel_ids, list) {
		if (channel->id == channel_id)
			return -EEXIST;
	}

	channel = kmalloc(sizeof(struct channels), GFP_KERNEL);
	if (!channel)
		return -ENOMEM;
	channel->id = channel_id;

	res = open(this, channel_id);
	if (res) {
		kfree(channel);
		pr_err("open fail %d\n", channel_id);
		return res;
	}
	list_add(&channel->list, &this->channel_ids);
	return 0;
}
EXPORT_SYMBOL(adt_add_monitor);

int adt_del_monitor(struct adt_instance *this, const int channel_id)
{
	struct list_head *cur, *bak;
	struct channels *channel;
	int res;

	if (!this || !channel_id)
		return -EINVAL;

	list_for_each_safe(cur, bak, &this->channel_ids) {
		channel = list_entry(cur, struct channels, list);
		if (channel->id == channel_id) {
			res = close(this, channel_id, NULL);
			list_del(cur);
			kfree(channel);
			return res;
		}
	}
	return -ENXIO;
}
EXPORT_SYMBOL(adt_del_monitor);
