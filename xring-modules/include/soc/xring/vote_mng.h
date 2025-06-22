/* SPDX-License-Identifier: GPL-2.0 */
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
#ifndef __XR_VOTE_MNG_H__
#define __XR_VOTE_MNG_H__
#include <linux/types.h>
#include <linux/platform_device.h>

#include <linux/types.h>

struct vote_mng;

#define VOTE_MNG_MSG_ASYNC	0
#define VOTE_MNG_MSG_SYNC	1

#define VOTE_MNG_OFF		(0 << 0)
#define VOTE_MNG_ON		(1 << 0)
#define VOTE_MNG_OFF_ASYNC	(0 << 1)
#define VOTE_MNG_OFF_SYNC	(1 << 1)

#define VOTE_MNG_MSG_SIZE	4

struct vote_mng_mode1_result {
	uint32_t arb_ch;
	uint32_t ack;
	uint32_t msg[VOTE_MNG_MSG_SIZE];
};

struct vote_mng_mode2_result {
	uint32_t arb_ch;
	uint32_t max;
	uint32_t min;
	uint32_t dmd;
};

struct vote_mng_mode3_result {
	uint32_t arb_ch;
	uint32_t ack;
};

struct vote_mng *vote_mng_get(struct device *dev, const char *con_id);
int vote_mng_put(struct device *dev, struct vote_mng *core);
int vote_mng_mode1_result_dump(struct vote_mng *votemng, struct vote_mng_mode1_result *res);
int vote_mng_mode2_result_dump(struct vote_mng *votemng, struct vote_mng_mode2_result *res);
int vote_mng_mode3_result_dump(struct vote_mng *votemng, struct vote_mng_mode3_result *res);

/*
 * vote_mng: vote_mng device pointer
 * ch: mode2 ch num to get vote val
 * val: return val of vote
 * ret: 0: success, <0: Errno
 */
int vote_mng_mode2_get_dmd(struct vote_mng *vote_mng, uint32_t ch, uint32_t *val);
int vote_mng_mode2_get_max(struct vote_mng *vote_mng, uint32_t ch, uint32_t *val);
int vote_mng_mode2_get_min(struct vote_mng *vote_mng, uint32_t ch, uint32_t *val);
/*
 * ret: >0: ch_num, <0: Errno
 */
int vote_mng_mode2_get_ch_num(struct vote_mng *vote_mng);

int vote_mng_msg_send(struct vote_mng *vote_mng,
		      uint32_t *data,
		      uint32_t size,
		      uint32_t sync);
int vote_mng_vote_dmd(struct vote_mng *vote_mng, uint32_t ch, uint32_t dmd);
int vote_mng_vote_min(struct vote_mng *vote_mng, uint32_t ch, uint32_t min);
int vote_mng_vote_max(struct vote_mng *vote_mng, uint32_t ch, uint32_t max);
int vote_mng_vote_onoff(struct vote_mng *vote_mng, uint32_t ch, uint32_t onoff);

#endif
