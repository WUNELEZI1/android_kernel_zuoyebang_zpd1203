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

#ifndef __XR_VOTE_MNG_INTERNAL_H__
#define __XR_VOTE_MNG_INTERNAL_H__

#include <linux/types.h>
#include <linux/io.h>

enum {
	VOTE_MNG_MODE2_T_MAX,
	VOTE_MNG_MODE2_T_MIN,
	VOTE_MNG_MODE2_T_DMD,
	VOTE_MNG_MODE2_T_NUM
};

struct vote_mng_mode1_hw {
	void __iomem *en;
	void __iomem *ack;
	void __iomem *msg;
	uint32_t arb_num;
	uint32_t ch_num;
	uint32_t core;
};
#define VOTE_MNG_MODE1_MSG_OFFSET(ch)	((ch) * 0x10)

struct vote_mng_mode2_hw {
	void __iomem *max;
	void __iomem *min;
	void __iomem *dmd;
	uint32_t arb_num;
	uint32_t ch_num;
	uint32_t core;
};
#define VOTE_MNG_MODE2_VOTE_OFFSET(ch)	((ch) * 0x0C)
#define VOTE_MNG_MODE2_HW_OFFSET(hw)	((hw) * 0xC0)

static inline uint32_t vote_mng_mode2_max_result(struct vote_mng_mode2_hw *hw, uint32_t arb)
{
	void __iomem *addr = hw->max + VOTE_MNG_MODE2_HW_OFFSET(arb) - 0xc;

	return (readl(addr) >> (hw->ch_num)) & 0xffff;
}

static inline uint32_t vote_mng_mode2_min_result(struct vote_mng_mode2_hw *hw, uint32_t arb)
{
	void __iomem *addr = hw->min + VOTE_MNG_MODE2_HW_OFFSET(arb) - 0xc;

	return (readl(addr) >> (hw->ch_num)) & 0xffff;
}

static inline uint32_t vote_mng_mode2_dmd_result(struct vote_mng_mode2_hw *hw, uint32_t arb)
{
	void __iomem *addr = hw->dmd + VOTE_MNG_MODE2_HW_OFFSET(arb) - 0xc;

	return (readl(addr) >> (hw->ch_num)) & 0xfffff;
}

struct vote_mng_mode3_hw {
	void __iomem *vote;
	void __iomem *ack;
	void __iomem *rsv;
	uint32_t arb_num;
	uint32_t ch_num;
	uint32_t core;
};
#define VOTE_MNG_MODE3_HW_OFFSET(hw)	((hw) * 0x10)

int vote_mng_mode1_msg_send(void *hw_data,
			    uint32_t arb,
			    uint32_t *data,
			    uint32_t size,
			    uint32_t sync);
int vote_mng_mode2_vote(void *hw_data,
			uint32_t arb,
			uint32_t ch,
			uint32_t data,
			uint32_t type);
int vote_mng_mode3_vote(void *hw_data,
			uint32_t arb,
			uint32_t ch,
			uint32_t flag);
uint32_t vote_mng_mode1_ack_get(struct vote_mng_mode1_hw *hw, uint32_t ch);

#endif
