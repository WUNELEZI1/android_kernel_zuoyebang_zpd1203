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
#include <linux/stat.h>

#include "fk-audio-simulate.h"
#include "fk-audio-log.h"

int fk_simulate_set_vir_addr(void *vir_addr, int dir)
{
	return fk_simulate_core_set_vir_addr(vir_addr, dir);
}
EXPORT_SYMBOL(fk_simulate_set_vir_addr);

int fk_simulate_send_sync(void *msg, unsigned int len)
{
	int ret;

	if (!msg) {
		AUD_LOG_ERR(AUD_COMM, "invalid notifier block");
		ret = -EINVAL;
		goto out;

	}

	if (len > CONFIG_IPC_MSG_DATA_LEN) {
		AUD_LOG_ERR(AUD_COMM, "msg length exceeds\n");
		ret = -EINVAL;
		goto out;
	}

	ret = fk_simulate_audio_process(msg, len, NULL, NULL);

out:
	return ret;
}
EXPORT_SYMBOL(fk_simulate_send_sync);

int fk_simulate_send_async(void *msg, unsigned int len,
			ack_callback_t cb, void *arg)
{
	int ret;

	if (!msg) {
		AUD_LOG_ERR(AUD_COMM, "invalid notifier block");
		ret = -EINVAL;
		goto out;
	}

	if (len > CONFIG_IPC_MSG_DATA_LEN) {
		AUD_LOG_ERR(AUD_COMM, "msg length exceeds");
		ret = -EINVAL;
		goto out;
	}

	ret = fk_simulate_audio_process(msg, len, cb, arg);

out:
	return ret;
}
EXPORT_SYMBOL(fk_simulate_send_async);

int fk_simulate_recv_register(recv_msg_cb cb, void *priv)
{
	if (!cb) {
		AUD_LOG_ERR(AUD_COMM, "invalid notifier block");
		return -EINVAL;
	}

	return fk_simulate_core_recv_register(cb, priv);
}
EXPORT_SYMBOL(fk_simulate_recv_register);

int fk_simulate_recv_unregister(void)
{
	return fk_simulate_core_recv_unregister();
}
EXPORT_SYMBOL(fk_simulate_recv_unregister);

int fk_simulate_write_file(void *buf, size_t size)
{

	return fk_simulate_core_write_file(buf, size);
}
EXPORT_SYMBOL(fk_simulate_write_file);

void fk_simulate_read_file(void *buf, size_t size)
{
	fk_simulate_core_read_file(buf, size);
}
EXPORT_SYMBOL(fk_simulate_read_file);

int __init fk_simulate_init(void)
{
	int ret = 0;

/*
 * create message process thread, playback data process
 * thread, capture data process thread
 */
	ret = fk_simulate_proc_thread_init(NULL);

	return ret;
}

void fk_simulate_exit(void)
{
}

MODULE_DESCRIPTION("XRING AUDIO COMMON SIMULATE");
MODULE_LICENSE("Dual BSD/GPL");
#endif

