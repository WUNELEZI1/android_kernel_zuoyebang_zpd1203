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

#ifndef _VENC_CLK_MANAGER_H_
#define _VENC_CLK_MANAGER_H_

#include <linux/clk.h>
#include <linux/mutex.h>
#include "vcx_vcmd_priv.h"

#define VENC_CLK_MAX_RATE 960000000
#define VENC_CLK_MID_RATE 600000000
#define VENC_CLK_MIN_RATE 334233600
#define VENC_CLK_MAX_RATE_LOW_TEMP VENC_CLK_MID_RATE

enum clk_mgr_update_mode{
    CLK_MGR_PUSH,
    CLK_MGR_POP
};

typedef struct {
    unsigned int max_clk_rate_num;
    unsigned int mid_clk_rate_num;
    unsigned int min_clk_rate_num;
} cmdbuf_clk_rate_mgr;

typedef struct {
    struct clk *clk;
    cmdbuf_clk_rate_mgr clk_rate_mgr;
    u32 fixed_clk_rate;
} venc_clk_mgr;

venc_clk_mgr* venc_clk_mgr_init(struct clk *clk);

void venc_clk_mgr_deinit(venc_clk_mgr* cm);

void venc_clk_mgr_update(venc_clk_mgr* cm, u64 clk_rate, enum clk_mgr_update_mode mode);

u64 venc_clk_mgr_get_rate(venc_clk_mgr* cm);

#endif