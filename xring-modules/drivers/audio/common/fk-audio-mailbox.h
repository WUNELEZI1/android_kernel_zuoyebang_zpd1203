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

#include "../adsp/adsp_ssr_driver.h"

#ifndef _FK_AUDIF_MAILBOX_
#define _FK_AUDIF_MAILBOX_

// #define MBX_SMEM_BASE		(0x49180000)
// #define MBX_SMEM_SIZE		(0x1000)

/* define audio mailbox channel index. */
enum aud_mbx_ch_idx {
	MBX_CH_AP_DSP = 0,
	MBX_CH_DSP_AP,
	MBX_CH_MAX,
};

/* define parameter operation command ID */
enum aud_mbx_param_cmd {
	MBX_PARAM_OV_INFO = 0,
	MBX_PARAM_PUT_INFO,
	MBX_PARAM_GET_INFO,
	MBX_PARAM_BLK_SIZE_INFO,
	MBX_PARAM_MAX,
};

#define AP_DSP_MBX_BLK_NUM		(12)
#define DSP_AP_MBX_BLK_NUM		(28)
#define MBX_BLK_SIZE			(256)

/* define head info of mailbox channel */
struct aud_mbx_head {
	/* message queue base physical address */
	uint32_t msg_base;
	/* mailbox block number */
	uint16_t mbx_blk_num;
	/* size one block. unit byte */
	uint16_t mbx_blk_size;
	/* total message count put to mailbox */
	uint32_t put_msg_cnt;
	/* indicate next index to be put */
	uint32_t put_msg_idx;
	/* total message count get from mailbox */
	uint32_t get_msg_cnt;
	/* indicate next index to be get */
	uint32_t get_msg_idx;
	uint32_t ov_flag;
	uint32_t reserver;
} __packed;

struct aud_mbx_ch_info {
	/* virtual address of share memory */
	void *smem_vir_base;
	uint32_t smem_phy_h;
	uint32_t smem_phy_l;
	uint32_t mbx_blk_num;
	uint32_t mbx_blk_size;

	struct mutex lock;
};

struct aud_mbx_drv {
	struct aud_mbx_ch_info mbx_info[MBX_CH_MAX];
};

/* function define */
bool fk_mbx_msg_is_empty(enum aud_mbx_ch_idx ch);
bool fk_mbx_msg_is_full(enum aud_mbx_ch_idx ch);
int fk_mbx_msg_put(enum aud_mbx_ch_idx ch, void *msg, uint32_t len);
int fk_mbx_msg_get(enum aud_mbx_ch_idx ch, void *msg);
void fk_mbx_msg_dump(void);
int fk_mbx_msg_param_get(enum aud_mbx_ch_idx ch, int cmd, void *val);
void fk_mbx_msg_reset(void *priv);
void fk_mbx_msg_init(void);

#endif //_FK_AUDIF_MAILBOX_
