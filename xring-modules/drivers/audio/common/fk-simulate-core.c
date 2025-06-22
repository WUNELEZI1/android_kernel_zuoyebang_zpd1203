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

#ifdef SIMULATE_COMMON

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/vmalloc.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/sched/mm.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/notifier.h>
#include <linux/wait.h>

#include <linux/kernel.h>
#include <linux/kthread.h>


#include "fk-audio-simulate.h"
#include "fk-audio-log.h"

#define SMEM_HEAD_SIZE (32)


struct dsp_sim_ins dspSimIns;

static char *cache_file = "/data/audio_test";
loff_t write_pos;
loff_t read_pos;

module_param(cache_file, charp, 0664);

MODULE_PARM_DESC(cache_file, "File to use to cache pages instead of memory");

MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);

int fk_simulate_core_set_vir_addr(void *vir_addr, int dir)
{
	/* dir: 0-playback 1-capture */
	if (!dir)
		dspSimIns.play_vir_addr = vir_addr;
	else
		dspSimIns.cap_vir_addr = vir_addr;

	AUD_LOG_INFO(AUD_COMM, "dir(%d) vir addr(0x%llx)", dir, (uint64_t)vir_addr);

	return 0;
}

int fk_simulate_dsp_send_sync(void *context)
{
	struct xring_aud_msg *msg_ack = (struct xring_aud_msg *)context;

	AUD_LOG_INFO(AUD_COMM, "xring_aud_msg len: %d", msg_ack->header.len);

	msg_ack->header.result = 0;

	dspSimIns.audio_simulate_cb.callback((void *)msg_ack, msg_ack->header.len, NULL);

	return 0;
}
EXPORT_SYMBOL(fk_simulate_dsp_send_sync);

static int fk_simulate_audio_msg_put(void *msg, unsigned int len)
{
	struct xring_aud_msg *aud_msg = (struct xring_aud_msg *)msg;
	struct dsp_msg_list_node *msg_node;
	int ret = 0;

	aud_msg = (struct xring_aud_msg *)msg;
	AUD_LOG_INFO(AUD_COMM, "scene_id = %d, cmd_id = %d, func_id = %d, len = %d",
				aud_msg->header.scene_id, aud_msg->header.cmd_id,
				aud_msg->header.func_id, aud_msg->header.len);

	/* malloc message node, save message info and put audio message node to data list tail */
	msg_node = kzalloc(sizeof(struct dsp_msg_list_node), GFP_KERNEL);
	if (!msg_node)
		return -ENOMEM;

	/* save message info */
	memcpy((void *)&msg_node->msg, (void *)aud_msg, aud_msg->header.len);

	/* put message node to the tail of dsp message list */
	list_add_tail(&msg_node->msg_list, &dspSimIns.dsp_msg_list);

	return ret;
}

static int fk_simulate_msg_proc_thread(void *priv)
{
	struct dsp_msg_list_node *dsp_msg_node = NULL;
	struct xring_aud_msg *dsp_msg = NULL;

	for (;;) {
		if (list_empty(&dspSimIns.dsp_msg_list)) {
			msleep(20);
			continue;
		} else {
			/* get msg from message list */
			dsp_msg_node = list_first_entry(&dspSimIns.dsp_msg_list,
				struct dsp_msg_list_node, msg_list);
			if (!dsp_msg_node)
				AUD_LOG_ERR(AUD_COMM, " dsp message node is NULL");

			/* del node from list */
			list_del(&dsp_msg_node->msg_list);

			/* print msg info, check if meets expectations */
			dsp_msg = (struct xring_aud_msg *)&dsp_msg_node->msg;
			AUD_LOG_INFO(AUD_COMM, "scene(%d) cmd(%d) func(%d) len(%d)",
				dsp_msg->header.scene_id, dsp_msg->header.cmd_id,
				dsp_msg->header.func_id, dsp_msg->header.len);

			if ((dsp_msg->header.scene_id == ADSP_SCENE_FAST_PLAY) ||
				(dsp_msg->header.scene_id == ADSP_SCENE_NORMAL_PLAY) ||
				(dsp_msg->header.scene_id == ADSP_SCENE_OFFLOAD_PLAY) ||
				(dsp_msg->header.scene_id == ADSP_SCENE_VOIP_DL)) {

				/* save share memory address, buffer size, buffer number */
				if (dsp_msg->header.cmd_id == ADSP_AUD_SCENE_STARTUP) {
					struct adsp_scene_startup_info *msg_startup;

					msg_startup = (struct adsp_scene_startup_info *)
						&dsp_msg->data[0];

					dspSimIns.play_smem.p_addr =
						msg_startup->smem.p_addr;
					dspSimIns.play_smem.blk_buf_size =
						msg_startup->smem.blk_buf_size;
					dspSimIns.play_smem.blk_buf_num =
						msg_startup->smem.blk_buf_num;

					memcpy((void *)&dspSimIns.g_aud_msg, (void *)dsp_msg,
						sizeof(struct xring_aud_msg));
				}

				if (dsp_msg->header.cmd_id == ADSP_AUD_CTRL) {
					if (dsp_msg->header.func_id == ADSP_AUD_CTRL_START)
						dspSimIns.play_start_flag = 1;
					else if (dsp_msg->header.func_id == ADSP_AUD_CTRL_STOP) {
						dspSimIns.play_start_flag = 0;
						write_pos = 0;
					}
				}

			} else if ((dsp_msg->header.scene_id == ADSP_SCENE_NORMAL_RECORD)
			|| (dsp_msg->header.scene_id == ADSP_SCENE_OFFLOAD_RECORD)
			|| (dsp_msg->header.scene_id == ADSP_SCENE_VOIP_UL)
			|| (dsp_msg->header.scene_id == ADSP_SCENE_FAST_RECORD)) {
				/* save share memory address, buffer size, buffer number */
				if (dsp_msg->header.cmd_id == ADSP_AUD_SCENE_STARTUP) {
					struct adsp_scene_startup_info *msg_startup;

					msg_startup = (struct adsp_scene_startup_info *)
						&dsp_msg->data[0];

					dspSimIns.cap_smem.p_addr =
						msg_startup->smem.p_addr;
					dspSimIns.cap_smem.blk_buf_size =
						msg_startup->smem.blk_buf_size;
					dspSimIns.cap_smem.blk_buf_num =
						msg_startup->smem.blk_buf_num;
					AUD_LOG_INFO(AUD_COMM, "buf size(%d) num(%d)",
						msg_startup->smem.blk_buf_size,
						msg_startup->smem.blk_buf_num);

					memcpy((void *)&dspSimIns.g_aud_msg, (void *)dsp_msg,
						sizeof(struct xring_aud_msg));
				}

				if (dsp_msg->header.cmd_id == ADSP_AUD_CTRL) {
					if (dsp_msg->header.func_id == ADSP_AUD_CTRL_START)
						dspSimIns.cap_start_flag = 1;
					else if (dsp_msg->header.func_id == ADSP_AUD_CTRL_STOP) {
						dspSimIns.cap_start_flag = 0;
						read_pos = 0;
					}
				}

			}

			/* send message back to callback. */
			fk_simulate_dsp_send_sync(dsp_msg);

			/* free message node */
			kfree(dsp_msg_node);
		}
	}
}

static int fk_simulate_playback_data_proc_thread(void *priv)
{
	void *cur_ptr = NULL;
	struct adsp_pcm_smem_head *head_info = NULL;
	int idx = 0;
	int frame_no = 0;
	struct xring_aud_msg write_done;
	struct adsp_data_buf_info *data = NULL;

	uint8_t scene_id = 0;

	for (;;) {
		msleep(20);
		if (!dspSimIns.play_start_flag) {
			frame_no = 0;
			continue;
		} else {
			/* check buffer block */
			head_info = (dspSimIns.play_vir_addr - SMEM_HEAD_SIZE);
			AUD_LOG_INFO(AUD_COMM, "write info: r_off(%d) w_off(%d)",
				head_info->r_off, head_info->w_off);
			if (head_info->r_off >= head_info->w_off)  {
				msleep(60);
				continue;
			}
			head_info->r_off++;

			//get playback data by share memory address + (buffer size * buffer index)
			cur_ptr = dspSimIns.play_vir_addr + idx * dspSimIns.play_smem.blk_buf_size;
			AUD_LOG_INFO(AUD_COMM, "write:addr(0x%llx) data(0x%x). frame no(%d)",
				(uint64_t)cur_ptr, *(uint32_t *)cur_ptr, frame_no);

			//write playback data to file
			fk_simulate_core_write_file(cur_ptr, dspSimIns.play_smem.blk_buf_size);

			//fill write done info
			scene_id = dspSimIns.g_aud_msg.header.scene_id;
			if ((scene_id == ADSP_SCENE_FAST_PLAY) ||
				(scene_id == ADSP_SCENE_NORMAL_PLAY) ||
				(scene_id == ADSP_SCENE_OFFLOAD_PLAY) ||
				(scene_id == ADSP_SCENE_VOIP_DL)) {
				write_done.header.scene_id = scene_id;
			} else {
				write_done.header.scene_id = ADSP_SCENE_NORMAL_PLAY;
			}
			write_done.header.ins_id = dspSimIns.g_aud_msg.header.ins_id;
			write_done.header.cmd_id = ADSP_AUD_CTRL;
			write_done.header.func_id = ADSP_AUD_CTRL_WRITE;
			write_done.header.len = sizeof(struct xring_aud_msg_header) +
				sizeof(struct adsp_data_buf_info);
			write_done.header.result = 0;

			data = (struct adsp_data_buf_info *)&write_done.data[0];
			data->phy_addr = (uint32_t)(dspSimIns.play_smem.p_addr + idx *
				dspSimIns.play_smem.blk_buf_size);
			data->len = dspSimIns.play_smem.blk_buf_size;
			data->frame_no = ++frame_no;

			idx++;
			if (idx >= dspSimIns.play_smem.blk_buf_num)
				idx = 0;

			//send write done message by callback.
			fk_simulate_dsp_send_sync((void *)&write_done);
		}
	}
}

static int fk_simulate_capture_data_proc_thread(void *priv)
{
	void *cur_ptr = NULL;
	struct adsp_pcm_smem_head *head_info = NULL;
	int idx = 0;
	int frame_no = 0;
	struct xring_aud_msg read_done;
	struct adsp_data_buf_info *data = NULL;

	uint8_t scene_id = 0;

	for (;;) {
		msleep(20);
		if (!dspSimIns.cap_start_flag) {
			idx = 0;
			frame_no = 0;
			continue;
		} else {
			/* check buffer block */
			head_info = (dspSimIns.cap_vir_addr - SMEM_HEAD_SIZE);
			AUD_LOG_INFO(AUD_COMM, "read info: r_off(%d) w_off(%d)",
				head_info->r_off, head_info->w_off);
			if (head_info->w_off >= head_info->r_off)  {
				msleep(60);
				continue;
			}
			head_info->w_off++;

			//get playback data by share memory address + (buffer size * buffer index)
			cur_ptr = (dspSimIns.cap_vir_addr +
				(idx * dspSimIns.cap_smem.blk_buf_size));
			AUD_LOG_INFO(AUD_COMM, "read: idx(%d) cur_ptr(0x%llx) size(%d)",
				idx, (uint64_t)cur_ptr, dspSimIns.cap_smem.blk_buf_size);

			//fill data to buffer as read data
			memset(cur_ptr, frame_no, dspSimIns.cap_smem.blk_buf_size);

			AUD_LOG_INFO(AUD_COMM, "read: 0x%x. frame no(0x%x)",
				*(uint32_t *)cur_ptr, frame_no);

			//fill read done info
			scene_id = dspSimIns.g_aud_msg.header.scene_id;
			if ((scene_id == ADSP_SCENE_NORMAL_RECORD) ||
				(scene_id == ADSP_SCENE_OFFLOAD_RECORD) ||
				(scene_id == ADSP_SCENE_VOIP_UL) ||
				(scene_id == ADSP_SCENE_FAST_RECORD)) {
				read_done.header.scene_id = scene_id;
			} else {
				read_done.header.scene_id = ADSP_SCENE_NORMAL_RECORD;
			}

			read_done.header.ins_id = dspSimIns.g_aud_msg.header.ins_id;
			read_done.header.cmd_id = ADSP_AUD_CTRL;
			read_done.header.func_id = ADSP_AUD_CTRL_READ;
			read_done.header.len = sizeof(struct xring_aud_msg_header) +
				sizeof(struct adsp_data_buf_info);
			read_done.header.result = 0;

			data = (struct adsp_data_buf_info *)&read_done.data[0];
			data->phy_addr = (uint32_t)(dspSimIns.cap_smem.p_addr +
				idx * dspSimIns.cap_smem.blk_buf_size);
			if (scene_id == ADSP_SCENE_OFFLOAD_RECORD)
				data->len = dspSimIns.cap_smem.blk_buf_size / 2;
			else
				data->len = dspSimIns.cap_smem.blk_buf_size;
			data->frame_no = frame_no++;

			idx++;
			if (idx >= dspSimIns.cap_smem.blk_buf_num)
				idx = 0;

			//send write done message by callback.
			fk_simulate_dsp_send_sync((void *)&read_done);
		}
	}
}

int fk_simulate_proc_thread_init(void *priv)
{
	/* init message list */
	INIT_LIST_HEAD(&dspSimIns.dsp_msg_list);

	/* create simulate process thread */
	dspSimIns.msg_thrd = kthread_run(fk_simulate_msg_proc_thread,
		NULL, "msg%d", 1);
	dspSimIns.play_data_thrd = kthread_run(fk_simulate_playback_data_proc_thread,
		NULL, "pdata%d", 1);
	dspSimIns.cap_data_thrd = kthread_run(fk_simulate_capture_data_proc_thread,
		NULL, "cdata%d", 1);

	return 0;
}

int fk_simulate_audio_process(void *msg, unsigned int len,
			ack_callback_t cb, void *arg)
{
	struct xring_aud_msg *aud_msg = NULL;
	int ret = 0;

	aud_msg = (struct xring_aud_msg *)msg;
	AUD_LOG_INFO(AUD_COMM, "scene_id = %d, cmd_id = %d, func_id = %d, len = %d",
				aud_msg->header.scene_id, aud_msg->header.cmd_id,
				aud_msg->header.func_id, aud_msg->header.len);

	if (cb) {
		dspSimIns.audio_simulate_cb.async_cb = cb;
		dspSimIns.audio_simulate_cb.async_cb(ret, arg);
	}

	switch (aud_msg->header.cmd_id) {
	case ADSP_AUD_SCENE_STARTUP:
	case ADSP_AUD_SCENE_SHUTDOWN:
	case ADSP_AUD_SCENE_TRIGGER:
	case ADSP_AUD_CTRL:
		ret = fk_simulate_audio_msg_put(msg, len);
		break;
	case ADSP_VOICE_CTRL:
	case ADSP_PARAM_CTRL:
	case ADSP_HW_MODULE_CTRL:
	case ADSP_DEBUG_CTRL:
	case ADSP_SYS_CTRL:
	case ADSP_GET_VALUE:
		break;
	default:
		AUD_LOG_INFO(AUD_COMM, "err cmd_id = %d", aud_msg->header.cmd_id);
		ret = -EINVAL;
		break;
	}

	return ret;
}

int fk_simulate_core_recv_register(recv_msg_cb cb, void *priv)
{
	if (!cb) {
		AUD_LOG_INFO(AUD_COMM, "invalid notifier block");
		return -EINVAL;
	}

	dspSimIns.audio_simulate_cb.callback = cb;
	dspSimIns.audio_simulate_cb.priv = priv;

	return 0;
}

int fk_simulate_core_recv_unregister(void)
{
	dspSimIns.audio_simulate_cb.callback = NULL;
	dspSimIns.audio_simulate_cb.priv = NULL;
	return 0;
}

int fk_simulate_core_write_file(void *buf, size_t size)
{
	struct file *cfile;
	int err;
	ssize_t tx = 0;

	if (cache_file) {
		cfile = filp_open(cache_file, O_CREAT | O_RDWR, 0664);
		if (IS_ERR(cfile)) {
			AUD_LOG_INFO(AUD_COMM, "%s open failed", cache_file);
			return PTR_ERR(cfile);
		}

		if (!(cfile->f_mode & FMODE_CAN_WRITE)) {
			AUD_LOG_INFO(AUD_COMM, "%s not writeable", cache_file);
			err = -EINVAL;
			goto err_close_filp;
		}
		AUD_LOG_INFO(AUD_COMM, "before write tmp_int: tx %lld, pos %lld", tx, write_pos);
		tx = kernel_write(cfile, buf, size, &write_pos);
		AUD_LOG_INFO(AUD_COMM, "after write tmp_int: tx %lld, pos %lld", tx, write_pos);

err_close_filp:
		filp_close(cfile, NULL);
	}
	return 0;
}

void fk_simulate_core_read_file(void *buf, size_t size)
{
	struct file *cfile;
	int err;
	ssize_t rx = 0;

	if (cache_file) {
		cfile = filp_open(cache_file, O_CREAT | O_RDWR, 0664);
		if (IS_ERR(cfile)) {
			AUD_LOG_INFO(AUD_COMM, "%s open failed", cache_file);
			return;
		}
		if (!(cfile->f_mode & FMODE_CAN_READ)) {
			AUD_LOG_INFO(AUD_COMM, "%s not readable", cache_file);
			err = -EINVAL;
			goto err_close_filp;
		}
		AUD_LOG_INFO(AUD_COMM, "before read tmp_int: rx %ld, pos %ld", rx, read_pos);
		rx = kernel_read(cfile, buf, size, &read_pos);
		AUD_LOG_INFO(AUD_COMM, "after read tmp_int: rx %lld, pos %lld", rx, read_pos);

err_close_filp:
		filp_close(cfile, NULL);
	}
}
#endif
