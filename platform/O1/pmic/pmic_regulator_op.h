// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */


#ifndef __PMIC_REGULATOR_OP_H__
#define __PMIC_REGULATOR_OP_H__

#include "pmic_regulator_def.h"
#include "top_reg.h"

struct xr_pmic_regulator_desc {
	u8 id;
	u8 is_sec;


	u16 pwr_off;
	u16 lp_off;
	u16 vol_rd_off;
	u16 vol_wr_off;


	u16 vol_min;
	u16 vol_max;
	u8  vol_step;
	u8  reg_val_min;



	u16 enable_delay;

	u16 ramp_speed;

	u16 ramp_end_delay;
};

#ifndef NULL
#define NULL 0
#endif

#define XR_REG_NA 0xffff

#define XR_PMIC_SUCC        0
#define XR_PMIC_ERR_INVALID -1
#define XR_PMIC_ERR_SPMI    -2

#define XR_REGULATOR_MODE_NORMAL  0
#define XR_REGULATOR_MODE_LP      1
#define XR_REGULATOR_MODE_INVALID 0xff
#define XR_REGULATOR_MODE_CNT        (XR_REGULATOR_MODE_LP + 1)

#define XR_REGL_SEC      1
#define XR_REGL_UNSEC    0
#define XR_REGL_ON       1
#define XR_REGL_OFF      0

#define PWR_EN_RO_MASK  TOP_REG_LLDO3_PWR_REG_VR_LLDO3_EN_MASK
#define PWR_EN_WR_MASK  TOP_REG_LLDO3_PWR_REG_SW_LLDO3_PWR_EN_MASK
#define LP_EN_RO_MASK   TOP_REG_LLDO3_LP_REG_VR_LLDO3_LP_EN_MASK
#define LP_EN_WR_MASK   TOP_REG_LLDO3_LP_REG_SW_LLDO3_LP_EN_MASK

#define PWR_OFF(n)         TOP_REG_##n##_PWR_REG
#define LP_OFF(n)          TOP_REG_##n##_LP_REG
#define VOL_RD_OFF(n)      TOP_REG_VR_##n##_VSET
#define VOL_WR_OFF(n)      TOP_REG_SW_##n##_VSET
#define XR_REGULATOR_DESC(name, is_sec, min, max, step, reg_val_min, enable_delay, ramp_speed, ramp_end_delay) \
			{MPMIC_##name, is_sec,                                           \
			PWR_OFF(name), LP_OFF(name), VOL_RD_OFF(name), VOL_WR_OFF(name), \
			min, max, step, reg_val_min, enable_delay, ramp_speed, ramp_end_delay}

#define XR_REGULATOR_DESC_NOLP(name, is_sec, min, max, step, reg_val_min)    \
			{MPMIC_##name, is_sec,                                           \
			PWR_OFF(name), XR_REG_NA, VOL_RD_OFF(name), VOL_WR_OFF(name),    \
			min, max, step, reg_val_min, 0, 0, 0}


#define MBUCK1_INFO   XR_REGULATOR_DESC(     MBUCK1,   XR_REGL_SEC,    270, 1350,  5, 0,    180, 12, 32)
#define MBUCK2_INFO   XR_REGULATOR_DESC(     MBUCK2,   XR_REGL_SEC,    270, 1350,  5, 0,    180, 12, 32)
#define MBUCK3_INFO   XR_REGULATOR_DESC(     MBUCK3,   XR_REGL_SEC,    270, 1350,  5, 0,    400, 24, 18)
#define MBUCK4_INFO   XR_REGULATOR_DESC(     MBUCK4,   XR_REGL_SEC,    270, 1350,  5, 0,    400, 0,  0)
#define MBUCK5_INFO   XR_REGULATOR_DESC(     MBUCK5,   XR_REGL_SEC,    270, 1350,  5, 0,    0,   0,  0)
#define MBUCK6_INFO   XR_REGULATOR_DESC(     MBUCK6,   XR_REGL_SEC,    270, 1350,  5, 0,    200, 4,  32)
#define MBUCK7_INFO   XR_REGULATOR_DESC(     MBUCK7,   XR_REGL_SEC,    270, 1350,  5, 0,    0,   12, 32)
#define MBUCK8_INFO   XR_REGULATOR_DESC(     MBUCK8,   XR_REGL_SEC,    270, 1350,  5, 0,    0,   0,  0)
#define MBUCK9_INFO   XR_REGULATOR_DESC(     MBUCK9,   XR_REGL_SEC,    270, 1420,  5, 0,    0,   0,  0)
#define MBUCK10_INFO  XR_REGULATOR_DESC(     MBUCK10,  XR_REGL_SEC,   1470, 2100, 10, 0x40, 0,   0,  0)
#define ULLDO1_INFO   XR_REGULATOR_DESC(     ULLDO1,   XR_REGL_SEC,    540, 1320, 10, 0x4,  120, 12, 32)
#define ULLDO2_INFO   XR_REGULATOR_DESC(     ULLDO2,   XR_REGL_SEC,    540, 1320, 10, 0x4,  120, 12, 32)
#define ULLDO3_INFO   XR_REGULATOR_DESC(     ULLDO3,   XR_REGL_SEC,    540, 1320, 10, 0x4,  0,   12, 32)
#define ULLDO4_INFO   XR_REGULATOR_DESC(     ULLDO4,   XR_REGL_SEC,    540, 1320, 10, 0x4,  0,   0,  0)
#define ULLDO5_INFO   XR_REGULATOR_DESC(     ULLDO5,   XR_REGL_UNSEC,  540, 1320, 10, 0x4,  120, 0,  0)
#define ULLDO6_INFO   XR_REGULATOR_DESC(     ULLDO6,   XR_REGL_SEC,    540, 1320, 10, 0x4,  120, 0,  0)
#define LLDO1_INFO    XR_REGULATOR_DESC(     LLDO1,    XR_REGL_SEC,    540, 1320, 10, 0x4,  120, 0,  0)
#define LLDO2_INFO    XR_REGULATOR_DESC(     LLDO2,    XR_REGL_SEC,    540, 1320, 10, 0x4,  120, 0,  0)
#define LLDO3_INFO    XR_REGULATOR_DESC(     LLDO3,    XR_REGL_UNSEC,  540, 1320, 10, 0x4,  120, 0,  0)
#define LLDO4_INFO    XR_REGULATOR_DESC(     LLDO4,    XR_REGL_UNSEC,  540, 1320, 10, 0x4,  120, 0,  0)
#define LLDO5_INFO    XR_REGULATOR_DESC(     LLDO5,    XR_REGL_UNSEC,  540, 1320, 10, 0x4,  120, 0,  0)
#define MLDO1_INFO    XR_REGULATOR_DESC(     MLDO1,    XR_REGL_UNSEC, 1650, 1960, 10, 0,    240, 0,  0)
#define MLDO2_INFO    XR_REGULATOR_DESC(     MLDO2,    XR_REGL_SEC,   1650, 1960, 10, 0,    240, 0,  0)
#define MLDO3_INFO    XR_REGULATOR_DESC(     MLDO3,    XR_REGL_SEC,   1650, 1960, 10, 0,    330, 0,  0)
#define MLDO4_INFO    XR_REGULATOR_DESC(     MLDO4,    XR_REGL_UNSEC, 1650, 1960, 10, 0,    240, 0,  0)
#define MLDO5_INFO    XR_REGULATOR_DESC(     MLDO5,    XR_REGL_SEC,   1650, 1960, 10, 0,    240, 0,  0)
#define MLDO6_INFO    XR_REGULATOR_DESC(     MLDO6,    XR_REGL_UNSEC, 1650, 1960, 10, 0,    240, 0,  0)
#define MLDO7_INFO    XR_REGULATOR_DESC(     MLDO7,    XR_REGL_UNSEC, 1650, 1960, 10, 0,    240, 0,  0)
#define MLDO8_INFO    XR_REGULATOR_DESC(     MLDO8,    XR_REGL_UNSEC, 1650, 1960, 10, 0,    240, 0,  0)
#define HLDO1_INFO    XR_REGULATOR_DESC(     HLDO1,    XR_REGL_UNSEC, 2400, 3020, 20, 0,    240, 0,  0)
#define HLDO2_INFO    XR_REGULATOR_DESC(     HLDO2,    XR_REGL_UNSEC, 2700, 3475, 25, 0,    240, 0,  0)
#define HLDO3_INFO    XR_REGULATOR_DESC(     HLDO3,    XR_REGL_UNSEC, 2700, 3475, 25, 0,    240, 0,  0)
#define HLDO4_INFO    XR_REGULATOR_DESC(     HLDO4,    XR_REGL_UNSEC, 2700, 3475, 25, 0,    240, 0,  0)
#define HLDO5_INFO    XR_REGULATOR_DESC(     HLDO5,    XR_REGL_UNSEC, 2700, 3475, 25, 0,    240, 0,  0)
#define HLDO6_INFO    XR_REGULATOR_DESC(     HLDO6,    XR_REGL_UNSEC, 2700, 3475, 25, 0,    240, 0,  0)
#define HLDO7_INFO    XR_REGULATOR_DESC(     HLDO7,    XR_REGL_UNSEC, 2700, 3475, 25, 0,    240, 0,  0)
#define HLDO8_INFO    XR_REGULATOR_DESC(     HLDO8,    XR_REGL_UNSEC, 2700, 3475, 25, 0,    240, 0,  0)
#define HLDO9_INFO    XR_REGULATOR_DESC(     HLDO9,    XR_REGL_UNSEC, 2700, 3475, 25, 0,    240, 0,  0)
#define HLDO10_INFO   XR_REGULATOR_DESC(     HLDO10,   XR_REGL_UNSEC, 2700, 3475, 25, 0,    240, 0,  0)
#define LDO_COUL_INFO XR_REGULATOR_DESC_NOLP(LDO_COUL, XR_REGL_UNSEC, 1700, 1875, 25, 0)
#define LDO_ADC_INFO  XR_REGULATOR_DESC_NOLP(LDO_ADC,  XR_REGL_UNSEC, 1100, 1850, 50, 0)

#ifndef XR_AVAILABLE_PMIC_REGULATORS
#define XR_AVAILABLE_PMIC_REGULATORS {\
		MBUCK1_INFO,    \
		MBUCK2_INFO,    \
		MBUCK3_INFO,    \
		MBUCK4_INFO,    \
		MBUCK5_INFO,    \
		MBUCK6_INFO,    \
		MBUCK7_INFO,    \
		MBUCK8_INFO,    \
		MBUCK9_INFO,    \
		MBUCK10_INFO,   \
		ULLDO1_INFO,    \
		ULLDO2_INFO,    \
		ULLDO3_INFO,    \
		ULLDO4_INFO,    \
		ULLDO5_INFO,    \
		ULLDO6_INFO,    \
		LLDO1_INFO,     \
		LLDO2_INFO,     \
		LLDO3_INFO,     \
		LLDO4_INFO,     \
		LLDO5_INFO,     \
		MLDO1_INFO,     \
		MLDO2_INFO,     \
		MLDO3_INFO,     \
		MLDO4_INFO,     \
		MLDO5_INFO,     \
		MLDO6_INFO,     \
		MLDO7_INFO,     \
		MLDO8_INFO,     \
		HLDO1_INFO,     \
		HLDO2_INFO,     \
		HLDO3_INFO,     \
		HLDO4_INFO,     \
		HLDO5_INFO,     \
		HLDO6_INFO,     \
		HLDO7_INFO,     \
		HLDO8_INFO,     \
		HLDO9_INFO,     \
		HLDO10_INFO,    \
		LDO_COUL_INFO,  \
		LDO_ADC_INFO,   \
}
#endif

static struct xr_pmic_regulator_desc xr_mpmic_regulators[] = XR_AVAILABLE_PMIC_REGULATORS;

#define XR_MPMIC_REGULATORS_CNT (sizeof(xr_mpmic_regulators) / sizeof(struct xr_pmic_regulator_desc))

static inline struct xr_pmic_regulator_desc *find_desc(int id)
{
	u8 i;

	if ((id < 0) || (id >= MPMIC_REGULATOR_CNT))
		return NULL;

	for (i = 0; i < XR_MPMIC_REGULATORS_CNT; i++) {
		if (xr_mpmic_regulators[i].id == id)
			return &xr_mpmic_regulators[i];
	}

	return NULL;
}

static inline int xr_regulator_is_enabled(__maybe_unused void *spmi_hdl, int id)
{
	u8 data;
	int is_enabled;
	struct xr_pmic_regulator_desc* desc;

	desc = find_desc(id);
	if (!desc)
		return XR_PMIC_ERR_INVALID;

	if (XR_SPMI_READ(spmi_hdl, desc->pwr_off, &data))
		return XR_PMIC_ERR_SPMI;

	is_enabled = (data & PWR_EN_RO_MASK) ? 1 : 0;

	return is_enabled;
}

static inline int xr_regulator_enable(__maybe_unused void *spmi_hdl, int id, int enable)
{
	u8 data;
	struct xr_pmic_regulator_desc* desc;

	desc = find_desc(id);
	if (!desc)
		return XR_PMIC_ERR_INVALID;

	data = enable ? PWR_EN_WR_MASK : 0;
	if (XR_SPMI_WRITE(spmi_hdl, desc->pwr_off, data))
		return XR_PMIC_ERR_SPMI;

	if (enable && desc->enable_delay)
		XR_SPMI_UDELAY(desc->enable_delay);

	return XR_PMIC_SUCC;
}

static inline int xr_regulator_get_voltage(__maybe_unused void *spmi_hdl, int id, u16 *mv)
{
	struct xr_pmic_regulator_desc* desc;
	u8 data;
	u16 get_mv;

	desc = find_desc(id);
	if (!desc)
		return XR_PMIC_ERR_INVALID;

	if (XR_SPMI_READ(spmi_hdl, desc->vol_rd_off, &data))
		return XR_PMIC_ERR_SPMI;

	get_mv = (data - desc->reg_val_min) * desc->vol_step + desc->vol_min;


	if ((id == MPMIC_MBUCK5) && (get_mv > 900)) {
		get_mv += desc->vol_step;
	}

	*mv = get_mv;

	return XR_PMIC_SUCC;
}

static inline int xr_regulator_set_voltage(__maybe_unused void *spmi_hdl, int id, u16 mv)
{
	struct xr_pmic_regulator_desc* desc;
	u8 data;
	u16 old_mv;
	u16 delta_mv;
	u16 ramp_delay_us = 0;
	int ret;

	desc = find_desc(id);
	if (!desc)
		return XR_PMIC_ERR_INVALID;

	if ((mv < desc->vol_min) || (mv > desc->vol_max))
		return XR_PMIC_ERR_INVALID;

	ret = xr_regulator_get_voltage(spmi_hdl, id, &old_mv);
	if (ret != XR_PMIC_SUCC)
		return ret;


	data = (u8)(((mv - desc->vol_min) + desc->vol_step / 2) / desc->vol_step);


	if ((id == MPMIC_MBUCK5) && (mv > 900)) {
		data = data - 1;
	}

	if (XR_SPMI_WRITE(spmi_hdl, desc->vol_wr_off, data + desc->reg_val_min))
		return XR_PMIC_ERR_SPMI;


	if (desc->ramp_speed) {
		delta_mv = (mv > old_mv) ? (mv - old_mv) : (old_mv - mv);


		if ((id == MPMIC_MBUCK7) && (mv > old_mv) && (delta_mv > 50)) {
			ramp_delay_us = 5 + ((delta_mv * 10) / 35);
		} else {
			ramp_delay_us = delta_mv / desc->ramp_speed;
		}
	}
	ramp_delay_us += desc->ramp_end_delay;
	if (ramp_delay_us)
		XR_SPMI_UDELAY(ramp_delay_us);

	return XR_PMIC_SUCC;
}

__maybe_unused static inline int xr_regulator_is_sec(int id)
{
	struct xr_pmic_regulator_desc* desc;

	desc = find_desc(id);
	if (!desc)
		return XR_PMIC_ERR_INVALID;

	return desc->is_sec;
}

__maybe_unused
static inline int xr_regulator_get_mode(__maybe_unused void *spmi_hdl, int id, u16 *mode)
{
	u8 data;
	struct xr_pmic_regulator_desc* desc;

	desc = find_desc(id);
	if (!desc)
		return XR_PMIC_ERR_INVALID;

	if (desc->lp_off == XR_REG_NA) {
		*mode = XR_REGULATOR_MODE_NORMAL;
		return XR_PMIC_SUCC;
	}

	if (XR_SPMI_READ(spmi_hdl, desc->lp_off, &data))
		return XR_PMIC_ERR_SPMI;

	*mode = (data & LP_EN_RO_MASK) ? XR_REGULATOR_MODE_LP : XR_REGULATOR_MODE_NORMAL;

	return XR_PMIC_SUCC;
}

__maybe_unused
static inline int xr_regulator_set_mode(__maybe_unused void *spmi_hdl, int id, u16 mode)
{
	u8 data;
	struct xr_pmic_regulator_desc* desc;

	desc = find_desc(id);
	if (!desc)
		return XR_PMIC_ERR_INVALID;

	if (mode >= XR_REGULATOR_MODE_CNT)
		return XR_PMIC_ERR_INVALID;

	if (desc->lp_off == XR_REG_NA) {
		if (mode == XR_REGULATOR_MODE_NORMAL)
			return XR_PMIC_SUCC;
		else
			return XR_PMIC_ERR_INVALID;
	}

	data = (mode == XR_REGULATOR_MODE_LP) ? LP_EN_WR_MASK : 0;
	if (XR_SPMI_WRITE(spmi_hdl, desc->lp_off, data))
		return XR_PMIC_ERR_SPMI;

	return XR_PMIC_SUCC;
}

#endif
