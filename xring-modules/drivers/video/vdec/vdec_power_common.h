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
#ifndef _VDEC_POWER_COMMON_H_
#define _VDEC_POWER_COMMON_H_

#include <linux/regulator/consumer.h>
#include "vdec_clk_manager.h"
#define VDEC_POWER_MAX_NUM 2
#define VDEC_CLK_NAME_LEN 128
#define TBU_SID_NAME_LEN 128
#define SMMU_SID_INVALID 0
#define SMMU_SSID_INVALID 0

typedef enum {
    POWER_VDEC_CORE,
    POWER_VDEC_SUBSYS,
    POWER_DOMAIN_MAX,
} vdec_power_domain_e;

typedef enum {
    POWER_IS_SUSPENDING,
    POWER_IS_SUSPENDED,
    POWER_IS_RESUMING,
    POWER_IS_RESUMED,
    POWER_IS_NONSR,
} vdec_sr_status_e;

typedef struct {
    struct clk *clk;
    char clk_name[VDEC_CLK_NAME_LEN];
    unsigned long work_rate;
    unsigned long default_rate;
} vdec_clock;

typedef struct {
    struct device *dev;

    /* smmu info from dtsi */
    char tbu_name[TBU_SID_NAME_LEN];
    char sid_name[TBU_SID_NAME_LEN];
    unsigned int sid;
    unsigned int ssid;

    vdec_clock *vdec_clk;

    atomic_t power_count[VDEC_POWER_MAX_NUM];
    struct mutex pm_lock;
    struct mutex pm_runtime_lock;
    spinlock_t pm_sr_lock;

    struct regulator *vdec_rg;
    struct regulator *subsys_rg;

    bool runtime_idled;
    bool runtime_init;

    int cur_always_on;
    vdec_sr_status_e sr_status;
    int reset;
} vdec_power_mgr;

struct hantrodec_dev {
    struct device *dev;
    void *priv_data;
    vdec_power_mgr *pm;
    vdec_clk_mgr *cm;
};

#endif