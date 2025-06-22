// SPDX-License-Identifier: GPL-2.0-only
/*
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/idr.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>
#include <linux/sched/cputime.h>
#include <linux/sched/prio.h>
#include <linux/semaphore.h>

#include "fk-audio-test.h"
#include "fk-audio-ipc.h"
/* FK-AUDIO_MAILBOX begin */
#include "fk-audio-mailbox.h"
/* FK-AUDIO_MAILBOX end */

#include <soc/xring/ipc/xr_rproc.h>
#include <soc/xring/ipc/xr_ipc_test.h>
#include "fk-audio-simulate.h"
#include "audio-trace.h"
#include "fk-audio-log.h"

struct notifier_block fk_audio_nb;
struct notifier_block fk_audio_aon_nb;
static struct audio_callback_t audio_ipc_cb[MAX_SOURCE_TYPE_MSG];

static struct send_data data;
static struct recv_data data_recv;
static struct wakeup_data wk_data;
static int ipc_send_cnt;

static struct task_struct *dsp2ap_msg_thrd;
struct completion k_dsp2ap_signal;

//cpus is cpu number,audioCpuSet is open for 2~9
static int audioCpuSet = 1020;
static int cpus = 10;

int fk_audio_reset_ipc_fifo(void)
{
	ipc_send_cnt = 0;
	return xr_rproc_async_fifo_reset(ADSP_IPC_ID);
}
EXPORT_SYMBOL(fk_audio_reset_ipc_fifo);

void fk_audio_dump_ipc(void)
{
	AUD_LOG_INFO(AUD_COMM, "audio ipc send cnt:%d", ipc_send_cnt);
	xr_rproc_dump_async_cnt(ADSP_IPC_ID);
}
EXPORT_SYMBOL(fk_audio_dump_ipc);

void fk_audio_ipc_send_workqueue(struct work_struct *work)
{
	struct xring_aud_msg *aud_msg = &data.send_msg;
	int ret = 0;

	AUD_LOG_DBG(AUD_COMM, "scene_id = %d, ins_id = %d, cmd_id = %d, func_id = %d, len = %d",
					aud_msg->header.scene_id, aud_msg->header.ins_id,
					aud_msg->header.cmd_id, aud_msg->header.func_id,
					aud_msg->header.len);
	AUD_LOG_DBG_LIM(AUD_COMM, "len = %d", data.len);
	ret = xr_rproc_send_sync(data.ipc_id, (unsigned int *)aud_msg, data.len);
	if (ret)
		AUD_LOG_ERR(AUD_COMM, "fail to sync send");
}

int fk_audio_ipc_send_sync(unsigned int source_id,
			unsigned int *msg, unsigned int len)
{
	struct xring_aud_msg *aud_msg = &data.send_msg;
	int ret = 0;

	if (!msg) {
		AUD_LOG_ERR(AUD_COMM, "invalid msg data");
		return -EINVAL;
	}

	if (len > sizeof(struct xring_aud_msg)) {
		AUD_LOG_ERR(AUD_COMM, "msg length exceeds");
		ret = -EINVAL;
		goto out;
	}

	memcpy(aud_msg, msg, len);

#ifdef SIMULATE_COMMON
	fk_simulate_send_sync((void *)msg, len);
#else
	//INIT_WORK(&data.send_work, fk_audio_ipc_send_workqueue);
	data.len = len;
	data.ipc_id = ADSP_IPC_ID;
	queue_work(system_highpri_wq, &data.send_work);
#endif
out:
	return ret;
}
EXPORT_SYMBOL(fk_audio_ipc_send_sync);

int fk_audio_ipc_send_async(unsigned int source_id,
			unsigned int *msg, unsigned int len,
			ack_callback_t cb, void *arg)
{
	int ret = 0;
	int ipc_len = 0;

#ifndef SIMULATE_COMMON
	struct xring_aud_msg *aud_msg = (struct xring_aud_msg *)msg;
	int count = 0;
#endif

	if (source_id >= MAX_SOURCE_TYPE_MSG)
		return -EINVAL;

	if (!msg) {
		AUD_LOG_ERR(AUD_COMM, "invalid msg data");
		return -EINVAL;
	}

	if (len > sizeof(struct xring_aud_msg)) {
		AUD_LOG_ERR(AUD_COMM, "msg length exceeds");
		ret = -EINVAL;
		goto out;
	}

#ifdef SIMULATE_COMMON
	fk_simulate_send_async((void *)msg, len, cb, arg);
#else
	AUD_LOG_DBG(AUD_COMM, "scene_id = %d,ins_id = %d,cmd_id = %d,func_id = %d,len = %d",
						aud_msg->header.scene_id, aud_msg->header.ins_id,
						aud_msg->header.cmd_id, aud_msg->header.func_id,
						aud_msg->header.len);

	/* put message to mailbox message queue */
	ret = fk_mbx_msg_put(MBX_CH_AP_DSP, (void *)msg, len);
	if ((ret < 0) && (get_adsp_reset_process() == false)) {
		//xr_rproc_send_async(ADSP_IPC_ID, msg, ipc_len, cb, arg);
		AUD_LOG_ERR(AUD_COMM, "put message(0x%x) failure",
			*(uint32_t *)msg);
		/* trigger adsp reset */
		AUD_LOG_INFO(AUD_COMM, "Dump message start.");
		fk_mbx_msg_dump();
		AUD_LOG_INFO(AUD_COMM, "Dump message complete.");
		fk_audio_dump_ipc();
		/* 34: ap find adsp assert */
		adsp_reset(34);
	} else {
		ipc_len = (len <= CONFIG_IPC_MSG_DATA_LEN)?len:CONFIG_IPC_MSG_DATA_LEN;
		ret = xr_rproc_send_async(ADSP_IPC_ID, msg, ipc_len,
			cb, arg);
		if (ret)
			AUD_LOG_ERR(AUD_COMM, "send failed, count(%d), err(%d)",
				ret, count);
		ipc_send_cnt++;
	}

#endif
out:
	return ret;
}
EXPORT_SYMBOL(fk_audio_ipc_send_async);

static int fk_audio_ipc_recv_workqueue(void *work)
{
	struct xring_aud_msg aud_msg = {0};
	uint8_t source = 0;

	do {
		int ret = 0;

		if (wait_for_completion_interruptible(&k_dsp2ap_signal) != 0)
			continue;

		reinit_completion(&k_dsp2ap_signal);

		if (kthread_should_stop()) {
			AUD_LOG_INFO(AUD_COMM, "exit");
			break;
		}

		/* get message info */
		while (fk_mbx_msg_get(MBX_CH_DSP_AP, (void *)&aud_msg) >= 0) {
			if ((aud_msg.header.func_id != ADSP_AUD_CTRL_WRITE) &&
				(aud_msg.header.func_id != ADSP_AUD_CTRL_READ)) {
				AUD_LOG_DBG(AUD_COMM, "scene_id = %d,ins_id = %d,cmd_id = %d,func_id = %d,source = %d",
					aud_msg.header.scene_id, aud_msg.header.ins_id,
					aud_msg.header.cmd_id, aud_msg.header.func_id,
					aud_msg.header.msg_source);
#ifdef ATRACE_MACRO
				AUD_TRACE("msg sid(%d) ins id(%d) cmd id(%d) fun id(%d) rsp",
					aud_msg.header.scene_id, aud_msg.header.ins_id,
					aud_msg.header.cmd_id, aud_msg.header.func_id);
#endif
			}

			if (aud_msg.header.msg_source >= MAX_SOURCE_TYPE_MSG) {
				AUD_LOG_DBG(AUD_COMM, "source(%d) data(%d)",
					aud_msg.header.msg_source, aud_msg.data[0]);
				aud_msg.header.msg_source = AP_AUDIO_MSG;
			}

			source = aud_msg.header.msg_source;
			if (audio_ipc_cb[source].callback)
				audio_ipc_cb[source].callback(&aud_msg, aud_msg.header.len,
					audio_ipc_cb[source].priv);

#ifdef ATRACE_MACRO
			if ((aud_msg.header.func_id != ADSP_AUD_CTRL_WRITE) &&
				(aud_msg.header.func_id != ADSP_AUD_CTRL_READ))
				AUD_TRACE("msg sid(%d) ins id(%d) cmd id(%d) fun id(%d) rsp end",
					aud_msg.header.scene_id, aud_msg.header.ins_id,
					aud_msg.header.cmd_id, aud_msg.header.func_id);
#endif

			/* dump mailbox message info when processing more than
			 * ten messages in one scheduling.
			 */
			if (++ret >= DSP_AP_MBX_BLK_NUM) {
				AUD_LOG_INFO(AUD_COMM, "Dump message start.");
				fk_mbx_msg_dump();
				AUD_LOG_INFO(AUD_COMM, "Dump message complete.");
				break;
			}
		}
	} while (1);

	return 0;
}

#ifdef ABANDON_FRAME_VER
void fk_audio_ipc_set_abandon_frame_count(int count)
{
	data_recv.abandon_frame_count = count;
	AUD_LOG_INFO(AUD_COMM, "abandon_frame_count = %d", count);
}
#endif

void fk_audio_aon_ipc_recv_workqueue(struct work_struct *work)
{
	struct xring_aud_msg *aud_msg = &wk_data.wakeup_msg;
	uint8_t source = 0;
	void *priv = NULL;

	AUD_LOG_DBG(AUD_COMM, "scene_id = %d,ins_id = %d,cmd_id = %d,func_id = %d,len = %d",
					aud_msg->header.scene_id, aud_msg->header.ins_id,
					aud_msg->header.cmd_id, aud_msg->header.func_id,
					aud_msg->header.len);

	source = aud_msg->header.msg_source;
	priv = audio_ipc_cb[source].priv;
	if (audio_ipc_cb[source].callback)
		audio_ipc_cb[source].callback(aud_msg, aud_msg->header.len, priv);
}
/*voice trigger aon ipc callback*/
static int fk_audio_aon_ipc_recv_callback(struct notifier_block *nb,
			unsigned long action, void *data)
{
	struct xring_aud_msg *aud_msg = (struct xring_aud_msg *)data;

	memcpy(&wk_data.wakeup_msg, aud_msg, sizeof(struct xring_aud_msg));

	queue_work(system_highpri_wq, &wk_data.wakeup_work);

	return 0;
}

static void fk_audio_ipc_recv_callback(void *rx_buff, void *arg)
{
	complete(&k_dsp2ap_signal);
}

int fk_audio_ipc_recv_register(unsigned int source_id,
			recv_msg_cb cb, void *priv)
{
	if (!cb) {
		AUD_LOG_ERR(AUD_COMM, "invalid notifier block");
		return -EINVAL;
	}

	if (source_id >= MAX_SOURCE_TYPE_MSG)
		return -EINVAL;

	/* register audio ipc cb */
	audio_ipc_cb[source_id].callback = cb;
	audio_ipc_cb[source_id].priv = priv;

#ifdef SIMULATE_COMMON
	return fk_simulate_recv_register(cb, priv);
#else
	return 0;
#endif
}
EXPORT_SYMBOL(fk_audio_ipc_recv_register);

int fk_audio_ipc_recv_unregister(unsigned int source_id)
{
	if (source_id >= MAX_SOURCE_TYPE_MSG)
		return -EINVAL;

	/* unregister audio ipc cb */
	audio_ipc_cb[source_id].callback = NULL;
	audio_ipc_cb[source_id].priv = NULL;

#ifdef SIMULATE_COMMON
	return fk_simulate_recv_unregister();
#else
	return 0;
#endif
}
EXPORT_SYMBOL(fk_audio_ipc_recv_unregister);

int __init fk_audio_ipc_init(void)
{
	int i, ret = 0;
#ifdef ABANDON_FRAME_VER
	data_recv.abandon_frame_count = ABANDON_FRAME_COUNT;
#endif
	struct sched_param param = {.sched_priority = 1};
	struct cpumask cpu_mask;

	ipc_send_cnt = 0;
	INIT_LIST_HEAD(&data_recv.dsp_msg_list);

	INIT_WORK(&data.send_work, fk_audio_ipc_send_workqueue);
	//INIT_WORK(&data_recv.work, fk_audio_ipc_recv_workqueue);
	INIT_WORK(&wk_data.wakeup_work, fk_audio_aon_ipc_recv_workqueue);

	spin_lock_init(&data_recv.msg_lock);

	for (i = 0; i < MAX_SOURCE_TYPE_MSG; i++) {
		audio_ipc_cb[i].callback = NULL;
		audio_ipc_cb[i].priv = NULL;
		audio_ipc_cb[i].async_cb = NULL;
	}

	fk_mbx_msg_init();

	/* init wait complete signal */
	init_completion(&k_dsp2ap_signal);

	//create dsp2ap message work thread
	dsp2ap_msg_thrd = kthread_run(fk_audio_ipc_recv_workqueue, NULL, "dsp2ap_msg_thrd");

	//bind cpu
	cpumask_clear(&cpu_mask);
	for (i = 0; i < cpus; i++) {
		if ((1 << i) & audioCpuSet)
			cpumask_set_cpu(i, &cpu_mask);
	}
	//cpumask_or(&cpu_mask, cpumask_of(2), cpumask_of(3));
	set_cpus_allowed_ptr(dsp2ap_msg_thrd, &cpu_mask);

	//set prio
	WARN_ON_ONCE(sched_setscheduler_nocheck(dsp2ap_msg_thrd, SCHED_FIFO, &param));

#ifdef SIMULATE_COMMON
	return 0;
#else
	ret = xr_rproc_fast_ack_recv_register(ADSP_IPC_ID,
			fk_audio_ipc_recv_callback, NULL);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_COMM, "ADSP_IPC_ID register failed");
		return ret;
	}

	fk_audio_aon_nb.next = NULL;
	fk_audio_aon_nb.notifier_call = fk_audio_aon_ipc_recv_callback;
	fk_audio_aon_nb.priority = 0;

	ret = xr_rproc_recv_register(ADSP_AP_WAKEUP, &fk_audio_aon_nb);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_COMM, "ADSP_AP_WAKEUP register failed");
		return ret;
	}

	return ret;
#endif
}

void fk_audio_ipc_exit(void)
{
#ifndef SIMULATE_COMMON
	xr_rproc_recv_unregister(ADSP_IPC_ID, &fk_audio_nb);
#endif
	xr_rproc_recv_unregister(ADSP_IPC_ID, &fk_audio_nb);
	xr_rproc_recv_unregister(ADSP_AP_WAKEUP, &fk_audio_nb);

	// stop workqueue thread
	kthread_stop(dsp2ap_msg_thrd);
	complete(&k_dsp2ap_signal);
}

/* Module information */
MODULE_DESCRIPTION("XRING AUDIO COMMON IPC");
MODULE_LICENSE("Dual BSD/GPL");
