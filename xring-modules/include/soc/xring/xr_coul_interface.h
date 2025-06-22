/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#ifndef _XR_COUL_INTERFACE_H_
#define _XR_COUL_INTERFACE_H_

#include <linux/kernel.h>
#include <linux/power_supply.h>

#define BATTERY_NORMAL_POWEROFF_VOL       3150
#define DEFAULT_RPCB            10 /* mohm */

#define MAX_SOC                 100
#define MIN_SOC                 0
#define SOC_1                   1
#define SHUT_DOWN_VBAT          3000000 /*uv*/

/*TODO mv psy health to bci satrt */
#define POWER_SUPPLY_HEALTH_UNDERVOLTAGE 3
/*TODO mv psy health to bci end */

enum charge_status_event {
	COUL_CHG_NONE_EVENT = 0,
	COUL_CHG_START_EVENT,
	COUL_CHG_STOP_EVENT,
	COUL_CHG_FULL_EVENT,
};

int coul_intf_init_state(void);
int coul_intf_read_batt_cap(void);
int coul_intf_read_batt_cc(void);
int coul_intf_read_batt_fcc(void);
int coul_intf_read_batt_curr(void);
int coul_intf_read_batt_vol_uv(void);
int coul_get_tbatt(void);
int coul_intf_read_batt_fcc_design(void);
int coul_intf_read_vbatt_max(void);
int coul_intf_read_chg_cycle(void);
void coul_intf_resp_charger_event(unsigned int event);
int coul_intf_is_batt_exist(void);
int coul_intf_get_chg_status(void);

int coul_intf_init(void);
void coul_intf_exit(void);
#endif
