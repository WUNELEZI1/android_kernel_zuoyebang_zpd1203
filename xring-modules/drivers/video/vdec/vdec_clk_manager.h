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

#ifndef _VDEC_CLK_MANAGER_H_
#define _VDEC_CLK_MANAGER_H_

#include <linux/clk.h>
#include <linux/mutex.h>
#include "hantrovcmd_priv.h"

#define VDEC_CLK_MAX_RATE 835584000
#define VDEC_CLK_MID_RATE 417792000
#define VDEC_CLK_MIN_RATE 278528000
#define VDEC_CLK_MAX_RATE_LOW_TEMP VDEC_CLK_MID_RATE

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
} vdec_clk_mgr;

vdec_clk_mgr* vdec_clk_mgr_init(struct clk *clk);

void vdec_clk_mgr_deinit(vdec_clk_mgr* cm);

void vdec_clk_mgr_update(vdec_clk_mgr* cm, u64 clk_rate, enum clk_mgr_update_mode mode);

u64 vdec_clk_mgr_get_rate(vdec_clk_mgr* cm);

#endif