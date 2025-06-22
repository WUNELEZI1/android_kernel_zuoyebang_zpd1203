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
#include <linux/delay.h>
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

#include "../acore/fk-acore-define.h"
#include "../common/fk-audio-cma.h"
#include "fk-audio-mailbox.h"
#include "fk-audio-log.h"
#include "fk-audio-memlayout.h"
#if IS_ENABLED(CONFIG_MIEV)
#include <miev/mievent.h>
#endif

struct aud_mbx_drv mbx_drv_priv;

/************************************************************************
 * function: fk_mbx_msg_is_empty
 * description: Check if the mailbox message is empty.
 * parameter:
 *		enum aud_mbx_ch_idx ch : mailbox channel id.
 * return:
 *		true: empty
 *		false: not empty
 ************************************************************************/
bool fk_mbx_msg_is_empty(enum aud_mbx_ch_idx ch)
{
	int ret = false;
	struct aud_mbx_ch_info *mbx_ch_info = NULL;
	struct aud_mbx_head *mbx_head = NULL;

	/* Check if the channel ID is legal */
	if (ch >= MBX_CH_MAX) {
		ret = true;
		AUD_LOG_INFO(AUD_COMM, "invalid ch(%d)", ch);
		return ret;
	}

	mbx_ch_info = (struct aud_mbx_ch_info *)&mbx_drv_priv.mbx_info[ch];
	mbx_head = (struct aud_mbx_head *)mbx_ch_info->smem_vir_base;

	if (mbx_head->put_msg_cnt <= mbx_head->get_msg_cnt)
		ret = true;

	return ret;
}

/************************************************************************
 * function: fk_mbx_msg_is_full
 * description: Check if the mailbox message is full.
 * parameter:
 *		enum aud_mbx_ch_idx ch : mailbox channel id.
 * return:
 *		true: full
 *		false: not full
 ************************************************************************/
bool fk_mbx_msg_is_full(enum aud_mbx_ch_idx ch)
{
	int ret = false;
	struct aud_mbx_ch_info *mbx_ch_info = NULL;
	struct aud_mbx_head *mbx_head = NULL;
	/* try query three times */
	int try_cnt = 0;
#if IS_ENABLED(CONFIG_MIEV)
	struct misight_mievent *mievent;
#endif

	/* Check if the channel ID is legal */
	if (ch >= MBX_CH_MAX) {
		ret = true;
		AUD_LOG_INFO(AUD_COMM, "invalid ch(%d)", ch);
		return ret;
	}

	mbx_ch_info = (struct aud_mbx_ch_info *)&mbx_drv_priv.mbx_info[ch];
	mbx_head = (struct aud_mbx_head *)mbx_ch_info->smem_vir_base;

	/* When the put index is equal to the get index, but the
	 * put count not equal to the get count. that there are
	 * no avalid buffer block in the mailbox message queue.
	 */
	while ((mbx_head->put_msg_idx == mbx_head->get_msg_idx) &&
		(mbx_head->put_msg_cnt != mbx_head->get_msg_cnt)) {
		udelay(300);
		try_cnt++;
		if (try_cnt < 3)
			continue;
		ret = true;
#if IS_ENABLED(CONFIG_MIEV)
		mievent  = cdev_tevent_alloc(906001451);
		cdev_tevent_add_str(mievent, "Keyword", "mailbox message full");
		cdev_tevent_write(mievent);
		cdev_tevent_destroy(mievent);
#endif
		AUD_LOG_ERR(AUD_COMM, "msg full: put(%x)(%x) get(%x)(%x)",
			mbx_head->put_msg_cnt, mbx_head->put_msg_idx,
			mbx_head->get_msg_cnt, mbx_head->get_msg_idx);
		break;
	}

	return ret;
}

/************************************************************************
 * function: fk_mbx_msg_put
 * description: put message to mailbox message queue.
 * parameter:
 *		enum aud_mbx_ch_idx ch : mailbox channel id.
 *		void *msg : message data pointer.
 *		uint32_t len : message data length.
 * return:
 *		0: success
 *		other: fail
 ************************************************************************/
int fk_mbx_msg_put(enum aud_mbx_ch_idx ch, void *msg, uint32_t len)
{
	int ret = 0;
	struct aud_mbx_ch_info *mbx_ch_info = NULL;
	struct aud_mbx_head *mbx_head = NULL;
	void *msg_blk_ptr = NULL;

	mbx_ch_info = (struct aud_mbx_ch_info *)&mbx_drv_priv.mbx_info[ch];
	mbx_head = (struct aud_mbx_head *)mbx_ch_info->smem_vir_base;

	if (len > mbx_head->mbx_blk_size) {
		ret = -1;
		AUD_LOG_INFO(AUD_COMM, "invalid length(%d)(%d)",
			len, mbx_head->mbx_blk_size);
		return ret;
	}

	if ((mbx_head->put_msg_idx >= AP_DSP_MBX_BLK_NUM) ||
		(mbx_head->mbx_blk_size != MBX_BLK_SIZE)) {
		ret = -1;
		AUD_LOG_ERR(AUD_COMM, "invalid idx(%d) size(%d)",
			mbx_head->put_msg_idx, mbx_head->mbx_blk_size);
		return ret;
	}

	mutex_lock(&mbx_ch_info->lock);

	if (fk_mbx_msg_is_full(ch) == true) {
		mutex_unlock(&mbx_ch_info->lock);
		mbx_head->ov_flag++;
		AUD_LOG_INFO(AUD_COMM, "msg full. data(0x%x)", *(uint32_t *)msg);
		AUD_LOG_INFO(AUD_COMM, "base(%llx)", (uint64_t)mbx_ch_info->smem_vir_base);
		return -1;
	}

	/* calculate the share memory address of message block */
	msg_blk_ptr = mbx_ch_info->smem_vir_base + sizeof(struct aud_mbx_head) +
			mbx_head->put_msg_idx * mbx_head->mbx_blk_size;

	/* copy message data to the share memory */
	memcpy(msg_blk_ptr, msg, len);

	/* update put message count */
	if (mbx_head->put_msg_cnt == 0xFFFFFFFF)
		mbx_head->put_msg_cnt = 0;
	else
		mbx_head->put_msg_cnt++;

	/* update put message index */
	if (mbx_head->put_msg_idx == (mbx_head->mbx_blk_num - 1))
		mbx_head->put_msg_idx = 0;
	else
		mbx_head->put_msg_idx++;

	mutex_unlock(&mbx_ch_info->lock);

	return ret;
}

/************************************************************************
 * function: fk_mbx_msg_dump
 * description: dump mailbox message info
 * parameter:
 * return:
 *
 ************************************************************************/
void fk_mbx_msg_dump(void)
{
	struct aud_mbx_ch_info *mbx_ch_info = NULL;
	struct aud_mbx_head *mbx_head = NULL;
	int ch = 0;
	int idx = 0;
	int blk_num;

	void *msg_blk_ptr = NULL;
	struct xring_aud_msg *aud_msg;

	for (ch = 0; ch < MBX_CH_MAX; ch++) {
		mbx_ch_info = (struct aud_mbx_ch_info *)&mbx_drv_priv.mbx_info[ch];
		mbx_head = (struct aud_mbx_head *)mbx_ch_info->smem_vir_base;

		AUD_LOG_INFO(AUD_COMM, "mbx id: %d", ch);
		AUD_LOG_INFO(AUD_COMM, "mbx info:%d. size:%d",
			mbx_ch_info->mbx_blk_num, mbx_ch_info->mbx_blk_size);

		AUD_LOG_INFO(AUD_COMM, "message base:%x blk_num:%d blk_size:%d",
			mbx_head->msg_base, mbx_head->mbx_blk_num, mbx_head->mbx_blk_size);
		AUD_LOG_INFO(AUD_COMM, "message put count:%d index:%d",
			mbx_head->put_msg_cnt, mbx_head->put_msg_idx);
		AUD_LOG_INFO(AUD_COMM, "message get count:%d index:%d",
			mbx_head->get_msg_cnt, mbx_head->get_msg_idx);

		blk_num = mbx_head->mbx_blk_num;
		for (idx = 0; idx < blk_num; idx++) {
			msg_blk_ptr = mbx_ch_info->smem_vir_base + sizeof(struct aud_mbx_head) +
				idx * mbx_head->mbx_blk_size;
			aud_msg = msg_blk_ptr;
			AUD_LOG_INFO(AUD_COMM, "message %d: scene:%d ins:%d cmd:%d func:%d",
				idx, aud_msg->header.scene_id, aud_msg->header.ins_id,
				aud_msg->header.cmd_id, aud_msg->header.func_id);
		}
	}

}

/************************************************************************
 * function: fk_mbx_msg_get
 * description: get message from mailbox message queue.
 * parameter:
 *		enum aud_mbx_ch_idx ch : mailbox channel id.
 *		void *msg : message data pointer.
 *		uint32_t len : message data length.
 * return:
 *		>0: message data length
 *		<=0: get message data fail
 ************************************************************************/
int fk_mbx_msg_get(enum aud_mbx_ch_idx ch, void *msg)
{
	int ret = 0;
	struct aud_mbx_ch_info *mbx_ch_info = NULL;
	struct aud_mbx_head *mbx_head = NULL;
	void *msg_blk_ptr = NULL;

	mbx_ch_info = (struct aud_mbx_ch_info *)&mbx_drv_priv.mbx_info[ch];
	mbx_head = (struct aud_mbx_head *)mbx_ch_info->smem_vir_base;

	if ((mbx_head->get_msg_idx >= DSP_AP_MBX_BLK_NUM) ||
		(mbx_head->mbx_blk_size != MBX_BLK_SIZE) ||
		((mbx_head->get_msg_cnt + DSP_AP_MBX_BLK_NUM) <
		mbx_head->put_msg_cnt)) {
		ret = -1;
		AUD_LOG_ERR(AUD_COMM, "invalid idx(%d) size(%d) put(%d) get(%d)",
			mbx_head->put_msg_idx, mbx_head->mbx_blk_size,
			mbx_head->put_msg_cnt, mbx_head->get_msg_cnt);
		return ret;
	}

	mutex_lock(&mbx_ch_info->lock);

	if (fk_mbx_msg_is_empty(ch) == true) {
		mutex_unlock(&mbx_ch_info->lock);
		return -1;
	}

	msg_blk_ptr = mbx_ch_info->smem_vir_base + sizeof(struct aud_mbx_head) +
		mbx_head->get_msg_idx * mbx_head->mbx_blk_size;

	/* copy message data from the share memory
	 * Note: Exit atomic state to avoid entering sleep
	 * when page fault occurs
	 */
	memcpy(msg, msg_blk_ptr, MBX_BLK_SIZE);

	/* update get message count and index */
	if (mbx_head->get_msg_cnt == 0xFFFFFFFF)
		mbx_head->get_msg_cnt = 0;
	else
		mbx_head->get_msg_cnt++;

	if (mbx_head->get_msg_idx == (mbx_head->mbx_blk_num - 1))
		mbx_head->get_msg_idx = 0;
	else
		mbx_head->get_msg_idx++;

	ret = mbx_head->mbx_blk_size;

	mutex_unlock(&mbx_ch_info->lock);

	return ret;
}

/************************************************************************
 * function: fk_mbx_msg_param_get
 * description: get mailbox channel info.
 * parameter:
 * return:
 *		0: success
 *		other: fail
 ************************************************************************/
int fk_mbx_msg_param_get(enum aud_mbx_ch_idx ch, int cmd, void *val)
{
	int ret = 0;
	struct aud_mbx_ch_info *mbx_ch_info = NULL;
	struct aud_mbx_head *mbx_head = NULL;
	uint32_t *data = (uint32_t *)val;

	/* Check if the channel ID is legal */
	if (ch >= MBX_CH_MAX) {
		ret = -1;
		AUD_LOG_INFO(AUD_COMM, "invalid ch(%d)", ch);
		return ret;
	}

	if (data == NULL) {
		ret = -1;
		AUD_LOG_INFO(AUD_COMM, "val is NULL");
		return ret;
	}

	mbx_ch_info = (struct aud_mbx_ch_info *)&mbx_drv_priv.mbx_info[ch];
	mbx_head = (struct aud_mbx_head *)mbx_ch_info->smem_vir_base;

	mutex_lock(&mbx_ch_info->lock);

	switch (cmd) {
	case MBX_PARAM_OV_INFO:
		*data = mbx_head->ov_flag;
		break;
	case MBX_PARAM_PUT_INFO:
		*data = mbx_head->put_msg_cnt;
		break;
	case MBX_PARAM_GET_INFO:
		*data = mbx_head->get_msg_cnt;
		break;
	case MBX_PARAM_BLK_SIZE_INFO:
		*data = (uint32_t)mbx_head->mbx_blk_size;
		break;
	default:
		ret = -1;
		break;
	}

	mutex_unlock(&mbx_ch_info->lock);

	AUD_LOG_INFO(AUD_COMM, "parameter(%x) is %x", cmd, *data);

	return ret;
}
EXPORT_SYMBOL(fk_mbx_msg_param_get);

/************************************************************************
 * function: fk_mbx_msg_reset
 * description: reset mailbox channel info.
 * parameter:
 * return:
 ************************************************************************/
void fk_mbx_msg_reset(void *priv)
{
	struct aud_mbx_ch_info *mbx_ch_info = NULL;
	struct aud_mbx_head *mbx_head = NULL;
	int i = 0;

	/* init mailbox channel share memory head info */
	for (i = 0; i < MBX_CH_MAX; i++) {
		mbx_ch_info = (struct aud_mbx_ch_info *)&mbx_drv_priv.mbx_info[i];

		mutex_lock(&mbx_ch_info->lock);

		mbx_head = (struct aud_mbx_head *)mbx_ch_info->smem_vir_base;
		mbx_head->msg_base = mbx_ch_info->smem_phy_l + sizeof(struct aud_mbx_head);

		mbx_head->put_msg_cnt = 0;
		mbx_head->put_msg_idx = 0;
		mbx_head->get_msg_cnt = 0;
		mbx_head->get_msg_idx = 0;
		mbx_head->ov_flag = 0;

		mutex_unlock(&mbx_ch_info->lock);
	}

	AUD_LOG_INFO(AUD_COMM, "mbx smem head reset.");
}

/************************************************************************
 * function: fk_mbx_msg_init
 * description: init mailbox channel info.
 * parameter:
 * return:
 ************************************************************************/
void fk_mbx_msg_init(void)
{
	struct aud_mbx_ch_info *mbx_ch_info = NULL;
	struct aud_mbx_head *mbx_head = NULL;
	int i = 0;
	void *smem_base_ptr = NULL;
	uint32_t mbx_ch_base = 0;
	uint32_t mbx_ch_smem_size = 0;

	/* map mailbox share memory */
	smem_base_ptr = fk_get_audio_static_mem(MBX_MEM, &mbx_ch_base, MBH_MAX_SIZE);

	/* init ap to dsp mailbox channel driver info */
	mbx_ch_info = (struct aud_mbx_ch_info *)&mbx_drv_priv.mbx_info[MBX_CH_AP_DSP];
	mbx_ch_info->smem_vir_base = smem_base_ptr;
	mbx_ch_info->smem_phy_h = 0;
	mbx_ch_info->smem_phy_l = mbx_ch_base;
	mbx_ch_info->mbx_blk_num = AP_DSP_MBX_BLK_NUM;
	mbx_ch_smem_size = sizeof(struct aud_mbx_head) +
		AP_DSP_MBX_BLK_NUM * MBX_BLK_SIZE;
	AUD_LOG_INFO(AUD_COMM, "mbx ap-dsp smem init. phy(%llx) vir(%llx)",
		(uint64_t)&mbx_ch_info->smem_phy_l, (uint64_t)mbx_ch_info->smem_vir_base);

	/* init dsp to ap mailbox channel driver info */
	mbx_ch_info = (struct aud_mbx_ch_info *)&mbx_drv_priv.mbx_info[MBX_CH_DSP_AP];
	mbx_ch_info->smem_vir_base = smem_base_ptr + mbx_ch_smem_size;
	mbx_ch_info->smem_phy_h = 0;
	mbx_ch_info->smem_phy_l = mbx_ch_base + mbx_ch_smem_size;
	mbx_ch_info->mbx_blk_num = DSP_AP_MBX_BLK_NUM;
	AUD_LOG_INFO(AUD_COMM, "mbx dsp-ap smem init. phy(%llx) vir(%llx)",
		(uint64_t)&mbx_ch_info->smem_phy_l, (uint64_t)mbx_ch_info->smem_vir_base);

	/* init mailbox channel share memory head info */
	for (i = 0; i < MBX_CH_MAX; i++) {
		mbx_ch_info = (struct aud_mbx_ch_info *)&mbx_drv_priv.mbx_info[i];

		mbx_ch_info->mbx_blk_size = MBX_BLK_SIZE;
		mutex_init(&mbx_ch_info->lock);

		mbx_head = (struct aud_mbx_head *)mbx_ch_info->smem_vir_base;
		mbx_head->msg_base = mbx_ch_info->smem_phy_l +
			sizeof(struct aud_mbx_head);

		if (i == MBX_CH_AP_DSP)
			mbx_head->mbx_blk_num = AP_DSP_MBX_BLK_NUM;
		else
			mbx_head->mbx_blk_num = DSP_AP_MBX_BLK_NUM;

		mbx_head->mbx_blk_size = MBX_BLK_SIZE;
		mbx_head->put_msg_cnt = 0;
		mbx_head->put_msg_idx = 0;
		mbx_head->get_msg_cnt = 0;
		mbx_head->get_msg_idx = 0;
		mbx_head->ov_flag = 0;
	}
}

