/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#ifndef _XR_COUL_NV_H_
#define _XR_COUL_NV_H_

#define DEFAULT_V_OFF_A         1
#define DEFAULT_V_OFF_B         0
#define DEFAULT_C_OFF_A         1
#define DEFAULT_C_OFFSET_B      0

#include "xr_coul_core.h"
#include <xr_coul.h>

struct coul_cali_params {
	int v_offset_a;
	int v_offset_b;
	int c_offset_a;
	int c_offset_b;
	u64 chip_id;
};

struct coul_nv_data {
	int v_offset_a;
	int v_offset_b;
	int c_offset_a;
	int c_offset_b;
	u64 chip_id;
	struct coul_nv_info nv_data;
};

void get_coul_cali_params(struct coul_cali_params *cali_params);
int coul_nv_params_initial(struct coul_core_device *chip);
int coul_nv_params_deinitial(struct coul_core_device *chip);

int coul_nv_data_initial(void);
int coul_nv_data_exit(void);
int store_nv_data(struct coul_core_device *chip);
int coul_nv_write_cali(struct coul_cali_params *cali_params);
#endif
