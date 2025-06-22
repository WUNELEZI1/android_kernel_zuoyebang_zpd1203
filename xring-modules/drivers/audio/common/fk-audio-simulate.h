/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _FK_AUDIF_SIMULATE_
#define _FK_AUDIF_SIMULATE_

#include "fk-audio-ipc.h"

struct dsp_msg_list_node {
	struct list_head msg_list;
	struct xring_aud_msg msg;
	void *priv;
};

struct dsp_sim_ins {
	/* simulate define */
	struct task_struct *msg_thrd;
	struct task_struct *play_data_thrd;
	struct task_struct *cap_data_thrd;

	struct list_head dsp_msg_list;

	struct audio_callback_t audio_simulate_cb;
	struct xring_aud_msg g_aud_msg;

	/* share memory info */
	struct adsp_share_mem play_smem;
	void *play_vir_addr;
	uint32_t play_start_flag;

	struct adsp_share_mem cap_smem;
	void *cap_vir_addr;
	uint32_t cap_start_flag;

};

int fk_simulate_set_vir_addr(void *vir_addr, int dir);

int fk_simulate_send_sync(void *msg, unsigned int len);
int fk_simulate_send_async(void *msg, unsigned int len,
			ack_callback_t cb, void *arg);
int fk_simulate_recv_register(recv_msg_cb cb, void *priv);
int fk_simulate_recv_unregister(void);
int fk_simulate_write_file(void *buf, size_t size);
void fk_simulate_read_file(void *buf, size_t size);

int fk_simulate_proc_thread_init(void *priv);
int fk_simulate_core_set_vir_addr(void *vir_addr, int dir);
int fk_simulate_dsp_send_sync(void *context);
int fk_simulate_audio_process(void *msg, unsigned int len,
			ack_callback_t cb, void *arg);
int fk_simulate_core_recv_register(recv_msg_cb cb, void *priv);
int fk_simulate_core_recv_unregister(void);
int fk_simulate_core_write_file(void *buf, size_t size);
void fk_simulate_core_read_file(void *buf, size_t size);

#endif //_FK_AUDIF_SIMULATE_
