/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mca_battery_full.h
 *
 * mca battery full driver
 *
 * Copyright (c) 2023-2023 Xiaomi Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __BATTERY_FULL_H__
#define __BATTERY_FULL_H__

#define STRATEGY_FG_SUPPORT_FULL_CURR_MONITOR 0

enum battery_full_status {
    MCA_BATTERY_FULL_ENTRY,
    MCA_BATTERY_FULL_KEEP,
    MCA_BATTERY_FULL_SC_AU_WORKING, /* screen and audio status */
    MCA_BATTERY_FULL_SC_AU_NOT_WORK,
    MCA_BATTERY_FULL_EXIT,
};

enum co_mos_status {
    MCA_BATTERY_FULL_CO_MOS_AUTOCTRL = 0,
    MCA_BATTERY_FULL_CO_MOS_OPEN = 1,
};

int mca_battery_full_init(void *data);
int mca_battery_full_shutdown(void *data);
int mca_battery_full_parse_dt(void *data);
int mca_battery_full_cancel_curr_monitor_work(void *data);

#endif /* __BATTERY_FULL_H__ */
