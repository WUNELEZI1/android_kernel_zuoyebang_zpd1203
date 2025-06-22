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
#include <linux/slab.h>
#include <linux/platform_device.h>
#include "venc_clk_manager.h"
#include "venc_clk.h"

extern struct platform_device *platformdev;

venc_clk_mgr* venc_clk_mgr_init(struct clk *clk) {
    venc_clk_mgr *cm = NULL;

    if (!clk) {
        venc_cm_klog(LOGLVL_ERROR, "clk is NULL, invalid para!\n");
        return NULL;
    }

    cm = kzalloc(sizeof(venc_clk_mgr), GFP_KERNEL);
    if (!cm) {
        venc_cm_klog(LOGLVL_ERROR, "venc_clk_mgr kzalloc failed!\n");
        return NULL;
    }

    cm->clk = clk;

    return cm;
}

void venc_clk_mgr_deinit(venc_clk_mgr *cm) {
    if (!cm) {
        venc_cm_klog(LOGLVL_ERROR, "clk_mgr is NULL, invalid para!\n");
        return;
    }
    // cm->clk will free in hantroenc_power_mgr_deinit
    if (cm) {
        kfree(cm);
    }
}

static void venc_clk_mgr_push(venc_clk_mgr  *cm, u64 rate) {
    if (!cm) {
        venc_cm_klog(LOGLVL_ERROR, "clk_mgr is NULL, invalid para!\n");
        return;
    }
    switch (rate) {
        case VENC_CLK_MAX_RATE:
            cm->clk_rate_mgr.max_clk_rate_num++;
            break;
        case VENC_CLK_MID_RATE:
            cm->clk_rate_mgr.mid_clk_rate_num++;
            break;
        case VENC_CLK_MIN_RATE:
            cm->clk_rate_mgr.min_clk_rate_num++;
            break;
        default:
            break;
    }
}

static void venc_clk_mgr_pop(venc_clk_mgr *cm, u64 rate) {
    if (!cm) {
        venc_cm_klog(LOGLVL_ERROR, "clk_mgr is NULL, invalid para!\n");
        return;
    }
    switch (rate) {
        case VENC_CLK_MAX_RATE:
            cm->clk_rate_mgr.max_clk_rate_num--;
            break;
        case VENC_CLK_MID_RATE:
            cm->clk_rate_mgr.mid_clk_rate_num--;
            break;
        case VENC_CLK_MIN_RATE:
            cm->clk_rate_mgr.min_clk_rate_num--;
            break;
        default:
            break;
    }
}

void venc_clk_mgr_update(venc_clk_mgr *cm, u64 clk_rate, enum clk_mgr_update_mode mode) {
    if(!cm){
        venc_cm_klog(LOGLVL_ERROR,"clk_mgr is NULL!\n");
        return;
    }
    // Maintain the maximum frequency in cmdbuf of all work linked
    // lists.
    if (mode == CLK_MGR_PUSH) {
        venc_clk_mgr_push(cm, clk_rate);
    } else if (mode == CLK_MGR_POP) {
        venc_clk_mgr_pop(cm, clk_rate);
    }
    venc_cm_klog(LOGLVL_DEBUG,
                 "[runtime_freq] venc_clk_mgr_update mode = %d, max mid min [%u %u %u]",
                 mode,
                 cm->clk_rate_mgr.max_clk_rate_num,
                 cm->clk_rate_mgr.mid_clk_rate_num,
                 cm->clk_rate_mgr.min_clk_rate_num);
}

u64 venc_clk_mgr_get_rate(venc_clk_mgr *cm) {
    if (!cm) {
        venc_cm_klog(LOGLVL_ERROR, "invalid parameter:clk_mgr NULL!\n");
        return 0;
    }
    u64 desire_rate = 0;
    // return max rate
    if (cm->clk_rate_mgr.max_clk_rate_num) {
      desire_rate = VENC_CLK_MAX_RATE;
    } else if (cm->clk_rate_mgr.mid_clk_rate_num) {
      desire_rate = VENC_CLK_MID_RATE;
    } else if (cm->clk_rate_mgr.min_clk_rate_num) {
      desire_rate = VENC_CLK_MIN_RATE;
    } else {
      desire_rate = 0;
    }
    venc_cm_klog(LOGLVL_DEBUG,
                 "[runtime_freq]venc_clk_mgr_get_rate max mid min [%u %u %u] "
                 "desire_rate = %llu",
                 cm->clk_rate_mgr.max_clk_rate_num,
                 cm->clk_rate_mgr.mid_clk_rate_num,
                 cm->clk_rate_mgr.min_clk_rate_num, desire_rate);
    return desire_rate;
}
