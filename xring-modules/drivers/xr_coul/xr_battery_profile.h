/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#ifndef _XR_BATTERY_PROFILE_H_
#define _XR_BATTERY_PROFILE_H_

#include <xr_coul.h>

#define BATT_AUTH_BY_ADC           "adc_auth"
#define BATT_AUTH_BY_SN            "sn_auth"
#define FULL_CURR_DEFAULT          300

#define TEMP_SAMPLING_POINTS       6
#define ID_SN_SIZE                 6

struct coul_battery_profile {
	char *auth_type;
	char *batt_sn;
	int full_curr;
	struct coul_battery_data battery_data;
};

struct coul_battery_data *get_battery_profile(void);

int battery_profile_init(void);
void battery_profile_exit(void);

#endif
