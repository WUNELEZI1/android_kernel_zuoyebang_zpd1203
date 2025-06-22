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

#ifndef _FK_AUDIF_IPC_
#define _FK_AUDIF_IPC_

#include <linux/list.h>
#include <soc/xring/ipc/xr_ipc.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include "fk-common-init.h"
#include "../acore/fk-acore-define.h"

// #define DSP_UNSUPPORT 1
//#define ABANDON_FRAME_VER
#ifdef ABANDON_FRAME_VER
#define ABANDON_FRAME_COUNT		(5)
#endif

#define LPCORE_IPC_ID            (0)
#define ADSP_IPC_ID IPC_VC_PERI_NS_AP_ADSP_AUDIO
#define ADSP_AP_WAKEUP IPC_VC_AO_NS_ADSP_AP_WACKUP
#define CONFIG_IPC_MSG_DATA_LEN  (60)

typedef void (*recv_msg_cb)(void *msg, int len, void *priv);

struct audio_callback_t {
	recv_msg_cb callback;
	void *priv;
	ack_callback_t async_cb;
};

enum adsp_source_type {
	AP_AUDIO_MSG = 0,
	AP_USB_MSG = 1,
	AP_AUDIO_TOOL,
	MAX_SOURCE_TYPE_MSG,
};

struct send_data {
	struct xring_aud_msg send_msg;
	int ipc_id;
	int len;
	struct work_struct send_work;
};

struct recv_msg_list_node {
	struct list_head msg_list;
	struct xring_aud_msg msg;
	void *priv;
};

struct recv_data {
	struct work_struct work;
	struct list_head dsp_msg_list;

	struct xring_aud_msg msg;
	spinlock_t msg_lock;

#ifdef ABANDON_FRAME_VER
	int abandon_frame_count;
	int r_count;
#endif
};

struct wakeup_data {
	struct xring_aud_msg wakeup_msg;
	struct work_struct wakeup_work;
};

int fk_audio_ipc_send_sync(unsigned int source_id, unsigned int *msg, unsigned int len);
int fk_audio_ipc_send_async(unsigned int source_id, unsigned int *msg,
			unsigned int len, ack_callback_t cb, void *arg);
int fk_audio_ipc_recv_register(unsigned int source_id, recv_msg_cb cb, void *priv);
int fk_audio_ipc_recv_unregister(unsigned int source_id);
#ifdef ABANDON_FRAME_VER
void fk_audio_ipc_set_abandon_frame_count(int count);
#endif
int fk_audio_reset_ipc_fifo(void);
void fk_audio_dump_ipc(void);

#endif //_FK_AUDIF_IPC_
