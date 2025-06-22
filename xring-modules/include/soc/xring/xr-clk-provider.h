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
#ifndef __XR_CLK_PROVIDER_H__
#define __XR_CLK_PROVIDER_H__

#include "dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h"

enum {
	NORMAL_TEMPRATURE = 0,
	LOW_TEMPRATURE = 1,
};

enum mainbus_rate {
	PERI_MAINBUS_RATE_NONE,
	PERI_MAINBUS_RATE_418,
	PERI_MAINBUS_RATE_675,
	PERI_MAINBUS_RATE_836,
	PERI_MAINBUS_RATE_1200,
	PERI_MAINBUS_RATE_MAX,
};

#if IS_ENABLED(CONFIG_XRING_CLK_PERIDVFS)
int peribus_set_rate(unsigned int rate);
#else
static inline int peribus_set_rate(unsigned int rate) {return 0;}
#endif

#if IS_ENABLED(CONFIG_XRING_CLK_MEDIADVFS)
int get_dpu_avs_volt(unsigned int volt_level, uint8_t *dac_code);
int media_dvs_is_low_temperature(void);
#else
static inline int get_dpu_avs_volt(unsigned int volt_level, uint8_t *dac_code)
{
	*dac_code = 0;
	return 0;
}
static inline int media_dvs_is_low_temperature(void) {return NORMAL_TEMPRATURE;}
#endif

int dpu_ppll2_vote_bypass(unsigned int flag);
#endif /* __XR_CLK_PROVIDER_H__ */
