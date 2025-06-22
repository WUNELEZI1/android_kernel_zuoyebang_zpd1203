// SPDX-License-Identifier: GPL-2.0
/*
 * sc8577.c
 *
 * charge-pump ic driver
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

#include <mca/platform/platform_cp_class.h>
#include "inc/sc8577.h"
#include <mca/common/mca_log.h>
#include <mca/common/mca_event.h>
#include <linux/version.h>
#include <mca/common/mca_parse_dts.h>
#include <mca/common/mca_sysfs.h>
#include <mca/platform/platform_cp_class.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "cp_sc8577"
#endif

#define MAX_LENGTH_BYTE 600
#define MAX_REG_COUNT 0x42

static const struct reg_range sc8577_reg_range[] = {
	[SC8577_VBAT_OVP]               = SC8577_CHG_RANGE(7000, 10810, 30, 7000, false),
	[SC8577_VBAT_OVP_ALM]           = SC8577_CHG_RANGE(7000, 10810, 30, 7000, false),
	[SC8577_VOUT_OVP]               = SC8577_CHG_RANGE(7000, 10600, 1200, 7000, false),
	[SC8577_VBUS_OVP_MODE_1_1]      = SC8577_CHG_RANGE(7000, 13350, 50, 7000, false),
	[SC8577_VBUS_OVP_MODE_2_1]      = SC8577_CHG_RANGE(14000, 26700, 100, 14000, false),
	[SC8577_IBUS_OCP]               = SC8577_CHG_RANGE(1000, 8750, 250, 1000, false),
	[SC8577_VAC1_OVP]               = SC8577_CHG_RANGE(6500, 24000, 2500, 6500, false),
	[SC8577_VAC2_OVP]               = SC8577_CHG_RANGE(6500, 24000, 2500, 6500, false),
	[SC8577_PMID2OUT_OVP]           = SC8577_CHG_RANGE(100, 800, 100, 100, false),
	[SC8577_PMID2OUT_UVP]           = SC8577_CHG_RANGE(100, 450, 50, 100, false),
	[SC8577_WD_TIMEOUT]             = SC8577_CHG_RANGE(5, 20, 5, 5, false),
	[SC8577_TDIE_ALM_TH]            = SC8577_CHG_RANGE(250, 1525, 5, 250, false),
};

static const u32 sc8577_adc_accuracy_tbl[ADC_MAX_NUM] = {
	150000,	/* IBUS */
	35000,	/* VBUS */
	35000,	/* VAC1 */
	35000,	/* VAC2 */
	20000,	/* VOUT */
	20000,	/* VBAT */
	0,
	0,
	0,	/* TSBAT */
	4,	/* TDIE */
};

static const int sc8577_adc_m[] =
	{25, 625, 625, 625, 25, 25, 0, 0, 9766, 5};

static const int sc8577_adc_l[] =
	{10, 100, 100, 100, 10, 10, 0, 0, 100000, 10};

static const struct regmap_config sc8577_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = SC8577_REGMAX,
};

//REGISTER
static const struct reg_field sc8577_reg_fields[] = {
	/*reg00*/
	[VBAT_OVP_DIS] = REG_FIELD(0x00, 7, 7),
	[VBAT_OVP] = REG_FIELD(0x00, 0, 6),
	/*reg01*/
	[VBAT_OVP_ALM_DIS] = REG_FIELD(0x01, 7, 7),
	[VBAT_OVP_ALM] = REG_FIELD(0x01, 0, 6),
	/*reg05*/
	[IBUS_UCP_DIS] = REG_FIELD(0x05, 7, 7),
	[IBUS_UCP_TH] = REG_FIELD(0x05, 6, 6),
	[VBUS_IN_RANGE_DIS] = REG_FIELD(0x05, 2, 2),
	/*reg06*/
	[BUS_PD_EN] = REG_FIELD(0x06, 7, 7),
	[VBUS_OVP] = REG_FIELD(0x06, 0, 6),
	/*reg08*/
	[IBUS_OCP_DIS] = REG_FIELD(0x08, 7, 7),
	[IBUS_OCP] = REG_FIELD(0x08, 0, 4),
	/*reg0A*/
	[TSHUT_DIS] = REG_FIELD(0x0A, 7, 7),
	[TDIE_ALM_DIS] = REG_FIELD(0x0A, 4, 4),
	[TSBAT_FLT_DIS] = REG_FIELD(0x0A, 2, 2),
	/*reg0B*/
	[TDIE_ALM] = REG_FIELD(0x0B, 0, 7),
	/*reg0D*/
	[TSBAT_FLT] = REG_FIELD(0x0D, 0, 7),
	/*reg0E*/
	[VAC1_OVP] = REG_FIELD(0x0E, 5, 7),
	[VAC2_OVP] = REG_FIELD(0x0E, 2, 4),
	[VAC1_PD_EN] = REG_FIELD(0x0E, 1, 1),
	[VAC2_PD_EN] = REG_FIELD(0x0E, 0, 0),
	/*reg0F*/
	[REG_RST] = REG_FIELD(0x0F, 7, 7),
	[CHG_EN] = REG_FIELD(0x0F, 4, 4),
	[MODE] = REG_FIELD(0x0F, 3, 3),
	[ACDRV1_STAT] = REG_FIELD(0x0F, 1, 1),
	[ACDRV2_STAT] = REG_FIELD(0x0F, 0, 0),
	/*reg10*/
	[FSW_SET] = REG_FIELD(0x10, 5, 7),
	[WD_TIMEOUT] = REG_FIELD(0x10, 3, 4),
	[WD_TIMEOUT_DIS] = REG_FIELD(0x10, 2, 2),
	/*reg11*/
	[OTG_EN] = REG_FIELD(0x11, 7, 7),
	[SS_TIMEOUT] = REG_FIELD(0x11, 4, 6),
	[IBUS_UCP_FALL_DEG] = REG_FIELD(0x11, 2, 3),
	/*reg12*/
	[VOUT_OVP_DIS] = REG_FIELD(0x12, 7, 7),
	[VOUT_OVP] = REG_FIELD(0x12, 5, 6),
	[FREQ_SHIFT] = REG_FIELD(0x12, 3, 4),
	[MS] = REG_FIELD(0x12, 0, 1),
	/*reg13*/
	[VBAT_OVP_STAT] = REG_FIELD(0x13, 7, 7),
	[VBAT_OVP_ALM_STAT] = REG_FIELD(0x13, 6, 6),
	[VOUT_OVP_STAT] = REG_FIELD(0x13, 5, 5),
	[VBUS_OVP_STAT] = REG_FIELD(0x13, 1, 1),
	/*reg14*/
	[IBUS_OCP_STAT] = REG_FIELD(0x14, 7, 7),
	[IBUS_UCP_FALL_STAT] = REG_FIELD(0x14, 5, 5),
	[PIN_DIAG_FAIL_STAT] = REG_FIELD(0x14, 2, 2),
	/*reg15*/
	[VAC1_OVP_STAT] = REG_FIELD(0x15, 7, 7),
	[VAC2_OVP_STAT] = REG_FIELD(0x15, 6, 6),
	[VBAT_INSERT_STAT] = REG_FIELD(0x15, 5, 5),
	[VAC1_INSERT_STAT] = REG_FIELD(0x15, 4, 4),
	[VAC2_INSERT_STAT] = REG_FIELD(0x15, 3, 3),
	[CP_VBUS_PRESENT_STAT] = REG_FIELD(0x15, 2, 2),
	[ACRB1_CFG_STAT] = REG_FIELD(0x15, 1, 1),
	[ACRB2_CFG_STAT] = REG_FIELD(0x15, 0, 0),
	/*reg16*/
	[ADC_DONE_STAT] = REG_FIELD(0x16, 7, 7),
	[SS_TIMEOUT_STAT] = REG_FIELD(0x16, 6, 6),
	[TSBAT_FLT_STAT] = REG_FIELD(0x16, 3, 3),
	[TSHUT_FLT_STAT] = REG_FIELD(0x16, 2, 2),
	[TDIE_ALM_STAT] = REG_FIELD(0x16, 1, 1),
	[WD_TIMEOUT_STAT] = REG_FIELD(0x16, 0, 0),
	/*reg17*/
	[CP_SWITCHING_STAT] = REG_FIELD(0x17, 6, 6),
	[VBUS_ERRORHI_STAT] = REG_FIELD(0x17, 4, 4),
	[VBUS_ERRORLO_STAT] = REG_FIELD(0x17, 3, 3),
	/*reg18*/
	[VBATOVP_FLAG] = REG_FIELD(0x18, 7, 7),
	[VBATOVP_ALM_FLAG] = REG_FIELD(0x18, 6, 6),
	[VOUT_OVP_FLAG] = REG_FIELD(0x18, 5, 5),
	[VBUS_OVP_FLAG] = REG_FIELD(0x18, 1, 1),
	/*reg19*/
	[IBUS_OCP_FLAG] = REG_FIELD(0x19, 7, 7),
	[IBUS_UCP_FALL_FLAG] = REG_FIELD(0x19, 5, 5),
	[PIN_DIAG_FAIL_FLAG] = REG_FIELD(0x19, 2, 2),
	/*reg1A*/
	[VAC1_OVP_FLAG] = REG_FIELD(0x1A, 7, 7),
	[VAC2_OVP_FLAG] = REG_FIELD(0x1A, 6, 6),
	[VBAT_INSERT_FLAG] = REG_FIELD(0x1A, 5, 5),
	[VAC1_INSERT_FLAG] = REG_FIELD(0x1A, 4, 4),
	[VAC2_INSERT_FLAG] = REG_FIELD(0x1A, 3, 3),
	[VBUS_PRESENT_FLAG] = REG_FIELD(0x1A, 2, 2),
	[ACRB1_CFG_FLAG] = REG_FIELD(0x1A, 1, 1),
	[ACRB2_CFG_FLAG] = REG_FIELD(0x1A, 0, 0),
	/*reg1B*/
	[ADC_DONE_FLAG] = REG_FIELD(0x1B, 7, 7),
	[SS_TIMEOUT_FLAG] = REG_FIELD(0x1B, 6, 6),
	[TSBAT_FLT_FLAG] = REG_FIELD(0x1B, 3, 3),
	[TSHUT_FLAG] = REG_FIELD(0x1B, 2, 2),
	[TDIE_ALM_FLAG] = REG_FIELD(0x1B, 1, 1),
	[WD_TIMEOUT_FLAG] = REG_FIELD(0x1B, 0, 0),
	/*reg1C*/
	[VBUS_ERRORHI_FLAG] = REG_FIELD(0x1C, 4, 4),
	[VBUS_ERRORLO_FLAG] = REG_FIELD(0x1C, 3, 3),
	/*reg1D*/
	[VBAT_OVP_MASK] = REG_FIELD(0x1D, 7, 7),
	[VBAT_OVP_ALM_MASK] = REG_FIELD(0x1D, 6, 6),
	[VOUT_OVP_MASK] = REG_FIELD(0x1D, 5, 5),
	[VBUS_OVP_MASK] = REG_FIELD(0x1D, 1, 1),
	/*reg1E*/
	[IBUS_OCP_MASK] = REG_FIELD(0x1E, 7, 7),
	[IBUS_UCP_FALL_MASK] = REG_FIELD(0x1E, 5, 5),
	[PIN_DIAG_FAIL_MASK] = REG_FIELD(0x1E, 2, 2),
	/*reg1F*/
	[VAC1_OVP_MASK] = REG_FIELD(0x1F, 7, 7),
	[VAC2_OVP_MASK] = REG_FIELD(0x1F, 6, 6),
	[VBAT_INSERT_MASK] = REG_FIELD(0x1F, 5, 5),
	[VAC1_INSERT_MASK] = REG_FIELD(0x1F, 4, 4),
	[VAC2_INSERT_MASK] = REG_FIELD(0x1F, 3, 3),
	[VBUS_PRESENT_MASK] = REG_FIELD(0x1F, 2, 2),
	[ACRB1_CFG_MASK] = REG_FIELD(0x1F, 1, 1),
	[ACRB2_CFG_MASK] = REG_FIELD(0x1F, 0, 0),
	/*reg20*/
	[ADC_DONE_MASK] = REG_FIELD(0x20, 7, 7),
	[SS_TIMEOUT_MASK] = REG_FIELD(0x20, 6, 6),
	[TSBAT_FLT_MASK] = REG_FIELD(0x20, 3, 3),
	[TSHUT_MASK] = REG_FIELD(0x20, 2, 2),
	[TDIE_ALM_MASK] = REG_FIELD(0x20, 1, 1),
	[WD_TIMEOUT_MASK] = REG_FIELD(0x20, 0, 0),
	/*reg21*/
	[VBUS_ERRORHI_MASK] = REG_FIELD(0x21, 4, 4),
	[VBUS_ERRORLO_MASK] = REG_FIELD(0x21, 3, 3),
	/*reg22*/
	[DEVICE_ID] = REG_FIELD(0x22, 0, 7),
	/*reg23*/
	[CP_ADC_EN] = REG_FIELD(0x23, 7, 7),
	[ADC_RATE] = REG_FIELD(0x23, 6, 6),
	[ADC_FREEZE] = REG_FIELD(0x23, 5, 5),
	[IBUS_ADC_DIS] = REG_FIELD(0x23, 1, 1),
	[VBUS_ADC_DIS] = REG_FIELD(0x23, 0, 0),
	/*reg24*/
	[VAC1_ADC_DIS] = REG_FIELD(0x24, 7, 7),
	[VAC2_ADC_DIS] = REG_FIELD(0x24, 6, 6),
	[VOUT_ADC_DIS] = REG_FIELD(0x24, 5, 5),
	[VBAT_ADC_DIS] = REG_FIELD(0x24, 4, 4),
	[CP_TSBAT_ADC_DIS] = REG_FIELD(0x24, 1, 1),
	[CP_TDIE_ADC_DIS] = REG_FIELD(0x24, 0, 0),
	/*reg40*/
	[POR_FLAG] = REG_FIELD(0x40, 7, 7),
	[ACDRV_MANUAL_EN] = REG_FIELD(0x40, 6, 6),
	[ACDRV1_EN] = REG_FIELD(0x40, 5, 5),
	[ACDRV2_EN] = REG_FIELD(0x40, 4, 4),
	[IBUS_UCP_RISE_FLAG] = REG_FIELD(0x40, 3, 3),
	[IBUS_UCP_RISE_MASK] = REG_FIELD(0x40, 2, 2),
	[VOUT_OK_CHG_FLAG] = REG_FIELD(0x40, 1, 1),
	[VOUT_OK_CHG_MASK] = REG_FIELD(0x40, 0, 0),
	/*reg41*/
	[SS_TIMEOUT_DIS] = REG_FIELD(0x41, 7, 7),
	[VBUS_OVP_DIS] = REG_FIELD(0x41, 6, 6),
	[PMID2OUT_OVP_DIS] = REG_FIELD(0x41, 5, 5),
	[PMID2OUT_UVP_DIS] = REG_FIELD(0x41, 4, 4),
	[ACRB1_DET_DIS] = REG_FIELD(0x41, 3, 3),
	[ACRB2_DET_DIS] = REG_FIELD(0x41, 2, 2),
	[VBAT_SNS_DIS] = REG_FIELD(0x41, 0, 0),
	/*reg42*/
	[PMID2OUT_UVP] = REG_FIELD(0x42, 5, 7),
	[PMID2OUT_OVP] = REG_FIELD(0x42, 2, 4),
	[PMID2OUT_UVP_FLAG] = REG_FIELD(0x42, 1, 1),
	[PMID2OUT_OVP_FLAG] = REG_FIELD(0x42, 0, 0),
	/*reg43*/
	[PMID2OUT_UVP_MASK] = REG_FIELD(0x43, 3, 3),
	[PMID2OUT_OVP_MASK] = REG_FIELD(0x43, 2, 2),
	[VBUS_TH_CHG_EN_STAT] = REG_FIELD(0x43, 1, 1),
	[VOUT_TH_SW_REGN_STAT] = REG_FIELD(0x43, 0, 0),
	/*reg50*/
	[VAC_UVLO_FALL_DG] = REG_FIELD(0x50, 5, 5),
	[DEVICE_VER] = REG_FIELD(0x50, 3, 4),
	[SYNC_EN] = REG_FIELD(0x50, 2, 2),
	[ACDRV_BYPASS_EN] = REG_FIELD(0x50, 1, 1),
	/*reg54*/
	[WORK_MODE] = REG_FIELD(0x54, 0, 0),
	/*reg55*/
	[VBUS_FORCE_ON_PD] = REG_FIELD(0x55, 7, 7),
	[VBUS_FORCE_OFF_PD] = REG_FIELD(0x55, 6, 6),
};

/********************COMMON API***********************/
__maybe_unused static u8 val2reg(enum sc8577_reg_range id, u32 val)
{
	int i;
	u8 reg;
	const struct reg_range *range = &sc8577_reg_range[id];

	if (range->table) {
		if (val <= range->table[0])
			return 0;
		for (i = 1; i < range->num_table - 1; i++) {
			if (val == range->table[i])
				return i;
			if (val > range->table[i] &&
				val < range->table[i + 1])
				return range->round_up ? i + 1 : i;
		}
		return range->num_table - 1;
	}
	if (val <= range->min)
		reg = 0;
	else if (val >= range->max)
		reg = (range->max - range->offset) / range->step;
	else if (range->round_up)
		reg = (val - range->offset) / range->step + 1;
	else
		reg = (val - range->offset) / range->step;
	return reg;
}

__maybe_unused static u32 reg2val(enum sc8577_reg_range id, u8 reg)
{
	const struct reg_range *range= &sc8577_reg_range[id];

	return range->table ? range->table[reg] :
				  range->offset + range->step * reg;
}
/*********************************************************/
static int sc8577_field_read(struct sc8577_device *sc,
				enum sc8577_fields field_id, int *val)
{
    int ret;
	int retry_cnt = 0;

	while(retry_cnt < I2C_RETRY_CNT) {
		ret = regmap_field_read(sc->rmap_fields[field_id], val);
		if (ret < 0) {
			mca_log_err("sc8577 read field %d fail: %d, retry cnt:%d\n",
					field_id, ret, retry_cnt);
			retry_cnt++;
		} else {
			return ret;
		}
	}

	mca_log_err("sc8577 read field %d fail: %d, retry reach max:%d!!\n",
			field_id, ret, I2C_RETRY_CNT);

	return ret;
}

static int sc8577_field_write(struct sc8577_device *sc,
				enum sc8577_fields field_id, int val)
{
    int ret;
	int retry_cnt = 0;

	while(retry_cnt < I2C_RETRY_CNT) {
		ret = regmap_field_write(sc->rmap_fields[field_id], val);
		if (ret < 0) {
			mca_log_err("sc8577 write field %d fail: %d, retry cnt:%d\n",
					field_id, ret, retry_cnt);
			retry_cnt++;
		} else {
			return ret;
		}
	}

	mca_log_err("sc8577 write field %d fail: %d, retry reach max:%d!!\n",
			field_id, ret, I2C_RETRY_CNT);

	return ret;
}

static int sc8577_read_block(struct sc8577_device *sc,
				int reg, uint8_t *val, int len)
{
	int ret;

	ret = regmap_bulk_read(sc->regmap, reg, val, len);
	if (ret < 0) {
		mca_log_err("sc8577 read %02x block failed %d\n", reg, ret);
	}

	return ret;
}

/*******************************************************/
__maybe_unused static int sc8577_detect_device(struct sc8577_device *sc)
{
	int ret;
	int val;

	ret = sc8577_field_read(sc, DEVICE_ID, &val);
	if (ret < 0) {
		mca_log_err("%s fail(%d)\n", __func__, ret);
		return ret;
	}

	if (val != SC8577_DEVICE_ID) {
		mca_log_err("%s not find sc8577, ID = 0x%02x\n", __func__, ret);
		return -EINVAL;
	}

	return ret;
}

__maybe_unused static int sc8577_reg_reset(struct sc8577_device *sc)
{
	return sc8577_field_write(sc, REG_RST, 1);
}

__maybe_unused static int sc8577_get_status(struct sc8577_device *sc, uint32_t *status)
{
	int ret, val;
	*status = 0;

	ret = sc8577_field_read(sc, VBUS_ERRORHI_STAT, &val);
	if (ret < 0) {
		mca_log_err("%s fail to read VBUS_ERRORHI_STAT(%d)\n", __func__, ret);
		return ret;
	}
	if (val != 0)
		*status |= BIT(ERROR_VBUS_HIGH);

	ret = sc8577_field_read(sc, VBUS_ERRORLO_STAT, &val);
	if (ret < 0) {
		mca_log_err("%s fail to read VBUS_ERRORLO_STAT(%d)\n", __func__, ret);
		return ret;
	}
	if (val != 0)
		*status |= BIT(ERROR_VBUS_LOW);

	return ret;
}

static int sc8577_enable_adc(struct sc8577_device *sc, bool enable)
{
	return sc8577_field_write(sc, CP_ADC_EN, !!enable);
}

static int sc8577_get_adc_data(struct sc8577_device *sc,
			int channel, int *result)
{
	uint8_t val[2] = {0};
	int ret;

	if(channel >= ADC_MAX_NUM)
		return -EINVAL;

	sc8577_enable_adc(sc, true);
	msleep(50);
	sc8577_field_write(sc, ADC_FREEZE, 1);

	ret = sc8577_read_block(sc, SC8577_REG25 + (channel << 1), val, 2);
	if (ret < 0) {
		return ret;
	}

	*result = (val[1] | (val[0] << 8)) *
				sc8577_adc_m[channel] / sc8577_adc_l[channel];

	mca_log_info("channel:%d data:%d", channel, *result);

	sc8577_field_write(sc, ADC_FREEZE, 0);
	sc8577_enable_adc(sc, false);

	return ret;
}

__maybe_unused static int sc8577_set_vbatovp_alarm(struct sc8577_device *sc, int threshold)
{
	int reg_val = val2reg(SC8577_VBAT_OVP_ALM, threshold);

	mca_log_info("th:%d reg_val:%d\n",threshold, reg_val);

	return sc8577_field_write(sc, VBAT_OVP_ALM, reg_val);
}

__maybe_unused static int sc8577_disable_vbatovp_alarm(struct sc8577_device *sc, bool en)
{
	int ret;
	mca_log_info("%d\n", en);

	ret = sc8577_field_write(sc, VBAT_OVP_ALM_DIS, !!en);

	return ret;
}

__maybe_unused static int sc8577_is_vbuslowerr(struct sc8577_device *sc, bool *err)
{
	int ret;
	int val;

	ret = sc8577_field_read(sc, VBUS_ERRORLO_STAT, &val);
	if(ret < 0) {
		return ret;
	}

	mca_log_info("%d\n",val);

	*err = (bool)val;

	return ret;
}

/********i2c basic read/write interface***********/
__maybe_unused static int cp_read_word(struct i2c_client *client, u8 reg, u16 *val)
{
	s32 ret;

	ret = i2c_smbus_read_word_data(client, reg);
	if (ret < 0) {
		mca_log_err("i2c read word fail: can't read from reg 0x%02X, errcode=%d\n", reg, ret);
		return ret;
	}

	*val = (u16)ret;
	return 0;
}
#ifdef DEBUG_CODE
static int cp_write_word(struct i2c_client *client, u8 reg, u16 val)
{
	s32 ret;

	ret = i2c_smbus_write_word_data(client, reg, val);
	if (ret < 0) {
		mca_log_err("i2c write word fail: can't write to reg 0x%02X\n", reg);
		return ret;
	}
	return 0;
}
#endif

__maybe_unused static int cp_read_byte(struct i2c_client *client, u8 reg, u8 *val)
{
	s32 ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		mca_log_err("i2c read word fail: can't read from reg 0x%02X, errcode=%d\n", reg, ret);
		return ret;
	}

	*val = (u8)ret;
	return 0;

}

static int cp_write_byte(struct i2c_client *client, u8 reg, u8 val)
{
	s32 ret;

	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0) {
		mca_log_err("i2c write word fail: can't write to reg 0x%02X, errcode=%d\n", reg, ret);
		return ret;
	}

	return 0;
}
#ifdef DEBUG_CODE
static int cp_read_block(struct i2c_client *client, u8 reg, u8 *buf, int len)
{
	int ret;
	int i;

	for (i = 0; i < len; i++) {
		ret = i2c_smbus_read_byte_data(client, reg + i);
		if (ret < 0) {
			mca_log_info("i2c read reg 0x%02X faild\n", reg + i);
			return ret;
		}
		buf[i] = ret;
	}
	return 0;
}

static int cp_write_block(struct i2c_client *client, u8 reg, u8 *buf, int len)
{
	int ret;
	int i;

	for (i = 0; i < len; i++) {
		ret = i2c_smbus_write_byte_data(client, reg + i, buf[i]);
		if (ret < 0) {
			mca_log_info("i2c write reg 0x%02X faild\n", reg + i);
			return ret;
		}
	}

	return 0;
}
#endif

__maybe_unused static int cp_update_bits(struct i2c_client *client, u8 reg, u8 mask, u8 val)
{
	u8 tmp;

	cp_read_byte(client, reg, &tmp);
	tmp &= ~mask;
	tmp |= val & mask;
	cp_write_byte(client, reg, tmp);
	return 0;
}
/***********************end **********************/

#ifdef CONFIG_DEBUG_FS
#define CP_MAX_REGS_NUMBER	0x55

enum cp_attr_list {
	CP_DEBUG_PROP_ADDRESS,
	CP_DEBUG_PROP_COUNT,
	CP_DEBUG_PROP_DATA,
};

static struct reg_context {
			int address;
			int count;
			int data;
} reg_info;

static ssize_t cp_debugfs_show(void *priv_data, char *buf)
{
	struct mca_debugfs_attr_data *attr_data = (struct mca_debugfs_attr_data *)priv_data;
	struct mca_debugfs_attr_info *attr_info = attr_data->attr_info;
	struct sc8577_device *dev_data = (struct sc8577_device *)attr_data->private;
	u8 val = 0;
	ssize_t count = 0;
	int ret = 0;
	char read_buf[MAX_LENGTH_BYTE] = {'\0'};
	int i;

	if (!dev_data || !attr_info) {
		mca_log_err("null pointer show\n");
		return count;
	}

	switch (attr_info->debugfs_attr_name) {
	case CP_DEBUG_PROP_ADDRESS:
		count = scnprintf(buf, PAGE_SIZE, "%02x\n", reg_info.address);
		break;
	case CP_DEBUG_PROP_COUNT:
		count = scnprintf(buf, PAGE_SIZE, "%x\n", reg_info.count);
		break;
	case CP_DEBUG_PROP_DATA:
		for (i = 0; i < reg_info.count; i++) {
			ret = cp_read_byte(dev_data->client, (reg_info.address + i), &val);
			count += scnprintf(read_buf, MAX_LENGTH_BYTE, "%02x: %02x\n",
				reg_info.address + i, val);
			strcat(buf, read_buf);
		}
		break;
	default:
		break;
	}
	return count;
}

static ssize_t cp_debugfs_store(void *priv_data, const char *buf, size_t count)
{
	struct mca_debugfs_attr_data *attr_data = (struct mca_debugfs_attr_data *)priv_data;
	struct mca_debugfs_attr_info *attr_info = attr_data->attr_info;
	struct sc8577_device *dev_data = (struct sc8577_device *)attr_data->private;
	int val = 0;
	int ret = 0;

	if (!dev_data || !attr_info) {
		mca_log_err("null pointer store\n");
		return count;
	}

	if (kstrtoint(buf, 16, &val))
		return -EINVAL;

	switch (attr_info->debugfs_attr_name) {
	case CP_DEBUG_PROP_ADDRESS:
		reg_info.address = val;
		break;
	case CP_DEBUG_PROP_COUNT:
		if (val > MAX_REG_COUNT)
			reg_info.count = MAX_REG_COUNT;
		else if (reg_info.count < 1)
			reg_info.count = 1;
		else
			reg_info.count = val;
		break;
	case CP_DEBUG_PROP_DATA:
		ret = cp_write_byte(dev_data->client, reg_info.address, val);
		break;
	default:
		break;
	}

	return count;
}

struct mca_debugfs_attr_info cp_debugfs_field_tbl[] = {
	mca_debugfs_attr(cp_debugfs, 0664, CP_DEBUG_PROP_ADDRESS, address),
	mca_debugfs_attr(cp_debugfs, 0664, CP_DEBUG_PROP_COUNT, count),
	mca_debugfs_attr(cp_debugfs, 0600, CP_DEBUG_PROP_DATA, data),
};

#define CP_DEBUGFS_ATTRS_SIZE			ARRAY_SIZE(cp_debugfs_field_tbl)

#endif
/*******end debugfs******************************/

static int sc8577_init_protection(struct sc8577_device *cp, int forward_work_mode);
/* SC8577_ADC */
#ifdef DEBUG_CODE
static int sc8577_check_adc_enabled(struct sc8577_device *sc, bool *enabled)
{
	int ret = 0;
	unsigned int val;

	ret = cp_read_byte(sc->client, SC8577_REG_15, &val);
	if (!ret)
		*enabled = !!(val & SC8577_ADC_EN_MASK);
	mca_log_info("%s enable adc %x, is %d\n", sc->log_tag, val, *enabled);
	return ret;
}
#endif



static int sc8577_check_charge_enabled(struct sc8577_device *sc, bool *enabled)
{
	int ret, val;

	ret = sc8577_field_read(sc, CHG_EN, &val);

	*enabled = (bool)val;

	return ret;
}

/* SC8577_BAT_OVP */
static int sc8577_enable_batovp(struct sc8577_device *sc, bool enable)
{
	return sc8577_field_write(sc, VBAT_OVP_DIS, !enable);
}

static int sc8577_set_batovp_th(struct sc8577_device *sc, int threshold)
{
	int reg_val = val2reg(SC8577_VBAT_OVP, threshold);

	mca_log_debug("th:%d reg_val:%d\n", threshold, reg_val);

	return sc8577_field_write(sc, VBAT_OVP, reg_val);
}

/* SC8577_BAT_OCP */
static int sc8577_enable_batocp(struct sc8577_device *sc, bool enable)
{
	/* sc8577 not support ibat ocp */
	int ret = 0;
	/* sc8577 not support */
	return ret;
}

static int sc8577_set_batocp_th(struct sc8577_device *sc, int threshold)
{
	/* sc8577 not support ibat ocp */
	int ret = 0;
	/* sc8577 not support */
	return ret;
}

/* SC8577_USB_OVP */
static int sc8577_set_usbovp_th(struct sc8577_device *sc, int threshold)
{
	int reg_val = val2reg(SC8577_VAC1_OVP, threshold);

	mca_log_debug("th:%d reg_val:%d\n", threshold, reg_val);

	return sc8577_field_write(sc, VAC1_OVP, reg_val);
}


/* SC8577_WPC_OVP */
static int sc8577_set_wpcovp_th(struct sc8577_device *sc, int threshold)
{
	int reg_val = val2reg(SC8577_VAC2_OVP, threshold);

	mca_log_debug("th:%d reg_val:%d\n", threshold, reg_val);

	return sc8577_field_write(sc, VAC2_OVP, reg_val);
}

/* SC8577_BUS_OVP */
#define RANGE_MIN_MAX 2
static int sc8577_set_busovp_th(struct sc8577_device *sc, int threshold)
{
	int ret = 0;
	u8 reg_val;
	int vbus_ovp_mode = SC8577_VBUS_OVP_MODE_1_1;

	switch (sc->chip_vendor) {
	case SC8577:
		if(sc->work_mode == CP_MODE_DIV2) {
			vbus_ovp_mode = SC8577_VBUS_OVP_MODE_2_1;
		}

		reg_val = val2reg(vbus_ovp_mode, threshold);
		ret = sc8577_field_write(sc, VBUS_OVP, reg_val);
		mca_log_info("%s bus_ovpth= %d, val = %d\n",
				sc->log_tag, threshold, reg2val(vbus_ovp_mode, reg_val));
		break;
	default:
		mca_log_info("%s chip_vendor =%x no valid\n", sc->log_tag, sc->chip_vendor);
		return -1;
	}

	return ret;
}

/* SC8577_OUT_OVP */
static int sc8577_set_outovp_th(struct sc8577_device *sc, int threshold)
{
	int reg_val = val2reg(SC8577_VOUT_OVP, threshold);

	mca_log_debug("th:%d reg_val:%d\n", threshold, reg_val);

	return sc8577_field_write(sc, VOUT_OVP, reg_val);
}

/* SC8577_BUS_OCP */
static int sc8577_enable_busocp(struct sc8577_device *sc, bool enable)
{
	return sc8577_field_write(sc, IBUS_OCP_DIS, !enable);
}

static int sc8577_set_busocp_th(struct sc8577_device *sc, int threshold)
{
	int reg_val = val2reg(SC8577_IBUS_OCP, threshold);

	mca_log_debug("th:%d reg_val:%d\n", threshold, reg_val);

	return sc8577_field_write(sc, IBUS_OCP, reg_val);
}

/* SC8577_BUS_UCP */
static int sc8577_enable_busucp(struct sc8577_device *sc, bool enable)
{
	return sc8577_field_write(sc, IBUS_UCP_DIS, !enable);
}

/* SC8577_PMID2OUT_OVP */
static int sc8577_enable_pmid2outovp(struct sc8577_device *sc, bool enable)
{
	return sc8577_field_write(sc, PMID2OUT_OVP_DIS, !enable);
}

static int sc8577_set_pmid2outovp_th(struct sc8577_device *sc, int threshold)
{
	int reg_val = val2reg(SC8577_PMID2OUT_OVP, threshold);

	mca_log_debug("th:%d reg_val:%d\n", threshold, reg_val);

	return sc8577_field_write(sc, PMID2OUT_OVP, reg_val);
}

/* SC8577_PMID2OUT_UVP */
static int sc8577_enable_pmid2outuvp(struct sc8577_device *sc, bool enable)
{
	return sc8577_field_write(sc, PMID2OUT_UVP_DIS, !enable);
}

static int sc8577_set_pmid2outuvp_th(struct sc8577_device *sc, int threshold)
{
	int reg_val = val2reg(SC8577_PMID2OUT_UVP, threshold);

	mca_log_debug("th:%d reg_val:%d\n", threshold, reg_val);

	return sc8577_field_write(sc, PMID2OUT_UVP, reg_val);
}

/* SC8577_BAT_OVP_ALM */
static int sc8577_enable_batovp_alarm(struct sc8577_device *sc, bool enable)
{
	return sc8577_field_write(sc, VBAT_OVP_ALM_DIS, !enable);
}

static int sc8577_set_batovp_alarm_th(struct sc8577_device *sc, int threshold)
{
	int reg_val = val2reg(SC8577_VBAT_OVP_ALM, threshold);

	mca_log_debug("th:%d reg_val:%d\n", threshold, reg_val);

	return sc8577_field_write(sc, VBAT_OVP_ALM, reg_val);
}
/* SC8577_BUS_OCP_ALM */
static int sc8577_enable_busocp_alarm(struct sc8577_device *sc, bool enable)
{
	int ret = 0;
	/* sc8577 not support */
	return ret;
}

static int sc8577_set_adc_scanrate(struct sc8577_device *sc, bool oneshot)
{
	return sc8577_field_write(sc, ADC_RATE, oneshot);
}

static int sc8577_set_adc_scan(struct sc8577_device *sc, int channel, bool enable)
{
	enum sc8577_fields adc_dis_field = CP_TSBAT_ADC_DIS;
	switch(channel) {
	case ADC_IBUS:
		adc_dis_field = IBUS_ADC_DIS;
		break;
	case ADC_VBUS:
		adc_dis_field = VBUS_ADC_DIS;
		break;
	case ADC_VAC1:
		adc_dis_field = VAC1_ADC_DIS;
		break;
	case ADC_VAC2:
		adc_dis_field = VAC2_ADC_DIS;
		break;
	case ADC_VOUT:
		adc_dis_field = VOUT_ADC_DIS;
		break;
	case ADC_VBAT:
		adc_dis_field = VBAT_ADC_DIS;
		break;
	case ADC_TSBAT:
		adc_dis_field = CP_TSBAT_ADC_DIS;
		break;
	case ADC_TDIE:
		adc_dis_field = CP_TDIE_ADC_DIS;
		break;
	default:
		break;
	}

	return sc8577_field_write(sc, adc_dis_field, !enable);
}

/* SC8577_SYNC_FUNCTION_EN */
static int sc8577_enable_parallel_func(struct sc8577_device *sc, bool enable)
{
	return sc8577_field_write(sc, SYNC_EN, !enable);
}

/* SC8577_REG_RST */
static int sc8577_set_reg_reset(struct sc8577_device *sc)
{
	return sc8577_field_write(sc, REG_RST, true);
}

/* SC8577_MODE */
static int sc8577_set_operation_mode(struct sc8577_device *sc, int operation_mode)
{
	int ret = 0;
	u8 val;
	int mode = MCA_EVENT_CP_MODE_DEFAULT;

	switch (operation_mode) {
	case CP_MODE_FORWARD_2_1:
		val = SC8577_SET_2_1_MODE;
		sc->work_mode = CP_MODE_DIV2;
		break;
	case CP_MODE_FORWARD_1_1:
		val = SC8577_SET_1_1_MODE;
		sc->work_mode = CP_MODE_DIV1;
		break;
	case CP_MODE_REVERSE_1_1:
		val = SC8577_SET_1_1_MODE;
		sc->work_mode = CP_MODE_DIV1;
		break;
	default:
		mca_log_info("%s operation mode error%d\n", sc->log_tag, operation_mode);
		return -1;
	break;
	}
	sc->operation_mode = val;

	ret = sc8577_init_protection(sc, sc->work_mode);
	if(ret < 0) {
		mca_log_err("%s init protection fail\n", sc->log_tag);
	}
	mca_log_info("%s set operation mode %d regval %d work_mode %d\n", sc->log_tag, operation_mode, val, sc->work_mode);

	switch (sc->work_mode) {
	case CP_MODE_DIV2:
		mode = MCA_EVENT_CP_MODE_FORWARD_4;
		break;
	default:
		mode = MCA_EVENT_CP_MODE_DEFAULT;
		break;
	}

	mca_event_block_notify(MCA_EVENT_TYPE_CP_INFO, MCA_EVENT_CP_MODE_CHANGE, &mode);
	ret = sc8577_field_write(sc, MODE, val);
	if(ret < 0) {
		mca_log_err("%s set operation mode[%s] fail\n",
			sc->log_tag, val == SC8577_SET_2_1_MODE ? "2_1" : "1_1");
	}
	return ret;
}

static int sc8577_get_operation_mode(struct sc8577_device *sc, int *operation_mode)
{
	int ret = 0;
	int val;

	ret = sc8577_field_read(sc, MODE, &val);
	if (ret) {
		mca_log_info("%s get operation mode fail\n", sc->log_tag);
		return ret;
	}

	*operation_mode = val;

	return ret;
}

static int sc8577_get_int_stat(struct sc8577_device *sc, int channel, bool *enable)
{
	int ret = 0;
	enum sc8577_fields field = VOUT_OK_CHG_FLAG;
	int val = 0;

	switch (channel) {
	case VOUT_OK_CHG_STAT:
		field = VOUT_OK_CHG_FLAG;
		break;
	case VOUT_INSERT_STAT:
		field = VBAT_INSERT_STAT;
		break;
	case VBUS_PRESENT_STAT:
		field = CP_VBUS_PRESENT_STAT;
		break;
	case VWPC_PRESENT_STAT:
		field = VAC2_INSERT_STAT;
		break;
	case VUSB_PRESENT_STAT:
		field = VAC1_INSERT_STAT;
		break;
	default:
		*enable = 0;
		break;
	}

	ret = sc8577_field_read(sc, field, &val);
	if(ret < 0) {
		mca_log_err("%s get int stat fail\n", sc->log_tag);
		return ret;
	}
	*enable = !!val;

	return ret;
}

/* SC8577_ACDRV_MANUAL_EN */
static int sc8577_enable_acdrv_manual(struct sc8577_device *sc, bool enable)
{
	return sc8577_field_write(sc, ACDRV_MANUAL_EN, enable);
}

/* SC8577_OVPGATE_EN */
static int sc8577_enable_ovpgate(struct sc8577_device *sc, bool enable)
{
	return sc8577_field_write(sc, ACDRV1_EN, enable);
}

static int sc8577_get_ovpgate_status(struct sc8577_device *sc, bool *enable)
{
	int temp = 0;
	int ret = 0;

	ret = sc8577_field_read(sc, ACDRV1_STAT, &temp);
	*enable = !!temp;

	return ret;
}

#define MANUAL_GATE_MASK 0x38
static int sc8577_enable_acdrv_manual_ovpgate_wpcgate(struct sc8577_device *sc, bool enable)
{
	int ret = 0;

	ret |= sc8577_field_write(sc, ACDRV_MANUAL_EN, enable);
	ret |= sc8577_field_write(sc, ACDRV1_EN, enable);
	ret |= sc8577_field_write(sc, ACDRV2_EN, enable);

	return ret;
}

/* SC8577_SS_TIMEOUT */
static int sc8577_set_ss_timeout(struct sc8577_device *sc, u8 val)
{
	return sc8577_field_write(sc, SS_TIMEOUT, val);
}

/* SC8577_WD_TIMEOUT_SET */
#ifdef DEBUG_CODE
static int sc8577_set_wdt(struct sc8577_device *sc, int ms)
{
	int reg_val = val2reg(SC8577_WD_TIMEOUT, ms);

	mca_log_debug("th:%d reg_val:%d\n", threshold, reg_val);

	return sc8577_field_write(sc, WD_TIMEOUT, reg_val);
}
#endif

/* OTHER */
static int sc8577_set_batovp_alarm_int_mask(struct sc8577_device *sc, u8 mask)
{
	return sc8577_field_write(sc, VBAT_OVP_ALM_MASK, mask);
}

static int sc8577_set_busocp_alarm_int_mask(struct sc8577_device *sc, u8 mask)
{
	return sc8577_field_write(sc, IBUS_OCP_MASK, mask);
}

static int sc8577_set_ucp_fall_dg(struct sc8577_device *sc, u8 val)
{
	return sc8577_field_write(sc, IBUS_UCP_FALL_DEG, val);
}

static int sc8577_tsbat_flt_dis(struct sc8577_device *sc, bool disable)
{
	return sc8577_field_write(sc, TSBAT_FLT_DIS, disable);
}

static int sc8577_set_tdie_alm_th(struct sc8577_device *sc, int threshold)
{
	int reg_val = val2reg(SC8577_TDIE_ALM_TH, threshold);

	mca_log_debug("th:%d reg_val:%d\n", threshold, reg_val);

	return sc8577_field_write(sc, TDIE_ALM, reg_val);
}

static int cp_charge_detect_device(struct sc8577_device *sc)
{
	int ret = 0;
	int data;
	int retry_cnt = 0;

	while (retry_cnt < ERROR_RECOVERY_COUNT) {
		ret = sc8577_field_read(sc, DEVICE_ID, &data);
		if (ret < 0) {
			retry_cnt++;
			msleep(100);
			mca_log_info(" %s failed to read device id, retry count = %d\n", sc->log_tag, retry_cnt);
		} else
			break;
	}

	if (retry_cnt == ERROR_RECOVERY_COUNT && ret < 0) {
		mca_log_info("%s failed to detect CP device. retry %d times\n", sc->log_tag, retry_cnt);
		return ret;
	}

	mca_log_info("%s sucess read device id = %x\n", sc->log_tag, data);
	if (data == SC8577_DEVICE_ID)
		sc->chip_vendor = SC8577;
	else {
		mca_log_info("%s device_id is invalid\n",  sc->log_tag);
		return -1;
	}

	return ret;
}

/**
 * @brief
 *
 * @param sc
 * @param val SC8577_SW_FREQ_XXXX
 * @return int
 */
static int sc8577_set_switch_freq(struct sc8577_device *sc, u8 val)
{
	return sc8577_field_write(sc, FSW_SET, val);
}

/* add the regs you want to dump here */
static unsigned int sc8577_reg_list[] = {
	0x00, /* VBAT_OVP */
	0x05, /* CTRL1 */
	0x06, /* VBUS_OVP */
	0x08, /* IBUS_OCP */
	0x0B, /* TDIE ALM*/
	0x0E, /* VAC_CTRL*/
	0x0F, /* CTRL2 */
	0x10, /* CTRL3 */
	0x11, /* CTRL4 */
	0x12, /* CTRL5 */
	0x13, /* STAT1 */
	0x14, /* STAT2 */
	0x15, /* STAT3 */
	0x16, /* STAT4 */
	0x17, /* STAT5 */
	0x37, /* TDIE_ADC*/
	0x38, /* TDIE_ADC*/
	0x40, /* CONFIG2 */
	0x42, /* PMID2OUT_OVP_UVP */
};

enum cp_reg_idx {
	VBAT_OVP_REG = 0,
	IBAT_OCP_REG,
	VUSB_OVP_REG,
	VWPC_OVP_REG,
	VOUT_VBUS_OVP_REG,
	IBUS_OCP_REG,
	IBUS_UCP_REG,
	PMID2OUT_OVP_REG,
	PMID2OUT_UVP_REG,
	CONVERTER_STATE_REG,
	CTRL1_REG,
	CTRL2_REG,
	CTRL3_REG,
	CTRL4_REG,
	CTRL5_REG,
	INT_STAT_REG,
	INT_FLAG_REG,
	FLT_FLAG_REG,
	DEVICE_ID_REG,
	FAULT_STATUS_REG,
	CP_REG_MAX,
};

static struct stat_mca_notify stat_notify[] = {
	{VBAT_OVP_STAT, MCA_EVENT_CP_VBAT_OVP, "VBAT OVP"},
	{VOUT_OVP_STAT, MCA_EVENT_MAX, "VOUT OVP"},
	{VBUS_OVP_STAT, MCA_EVENT_CP_VBUS_OVP, "VBUS OVP"},
	{IBUS_OCP_STAT, MCA_EVENT_CP_IBUS_OCP, "IBUS OCP"},
	{IBUS_UCP_FALL_STAT, MCA_EVENT_CP_IBUS_UCP, "IBUS UCP"},
	{PIN_DIAG_FAIL_STAT, MCA_EVENT_CP_CBOOT_FAIL, "CBOOT FAIL"},
	{VAC1_OVP_STAT, MCA_EVENT_CP_VBUS_OVP, "VAC1 OVP"},
	{VAC2_OVP_STAT, MCA_EVENT_CP_VWPC_OVP, "VAC2 OVP"},
	{VAC1_INSERT_STAT, MCA_EVENT_MAX, "VAC1 INSERT"},
	{VAC2_INSERT_STAT, MCA_EVENT_MAX, "VAC2 INSERT"},
	{CP_VBUS_PRESENT_STAT, MCA_EVENT_MAX, "VBUS PRESENT"},
	{SS_TIMEOUT_STAT, MCA_EVENT_MAX , "SS TIMEOUT"},
	{WD_TIMEOUT_STAT, MCA_EVENT_MAX, "WD TIMEOUT"},
	{VBUS_ERRORHI_STAT, MCA_EVENT_MAX, "VBUS ERRORHI"},
	{VBUS_ERRORLO_STAT, MCA_EVENT_MAX, "VBUS ERRORLO"},
	{PMID2OUT_OVP_FLAG, MCA_EVENT_CP_PMID2OUT_OVP, "PMID2OUT OVP"},
	{PMID2OUT_UVP_FLAG, MCA_EVENT_CP_PMID2OUT_UVP, "PMID2OUT UVP"},
	{TDIE_ALM_STAT ,MCA_EVENT_CP_TSHUT_FLAG , "TSHUT_STAT"},
	{VBATOVP_FLAG ,MCA_EVENT_CP_VBAT_OVP , "VBAT OVP FLG"},
};

static struct stat_mca_notify stat_notify_other[] = {
	{TDIE_ALM_STAT ,MCA_EVENT_CP_TSHUT_FLAG , "TSHUT_STAT"},
};

#define STAT_FIELD_MAX   ARRAY_SIZE(stat_notify)
#define STAT_FIELD_MAX_OTHER   ARRAY_SIZE(stat_notify_other)
static void sc8577_abnormal_charging_judge(struct sc8577_device *sc, struct stat_mca_notify *stat_notify, int stat_max_num)
{
	int i = 0;
	int stat = 0;
	int ret = 0;
	int val[2] = {0};
	if (!sc)
		return;

	for(i = 0; i < stat_max_num; i++) {
		ret = sc8577_field_read(sc, stat_notify[i].field, &stat);
		if(ret < 0) {
			mca_log_err("%s get stat fail\n", sc->log_tag);
		} else if (stat == true) {
			mca_log_info("%s %s\n", sc->log_tag, stat_notify[i].err_str);
			if(stat_notify[i].event != MCA_EVENT_MAX) {
				if(stat_notify[i].event == MCA_EVENT_CP_TSHUT_FLAG)
					sc8577_get_adc_data(sc, TDIE_ALM, val);
				mca_event_block_notify(MCA_EVENT_TYPE_CP_INFO, stat_notify[i].event, val);
			}
		}
	}
}

#define CP_MAX_REG_NUM			ARRAY_SIZE(sc8577_reg_list)
static int sc8577_dump_important_regs(struct sc8577_device *sc, union cp_propval *val)
{
	int ret = 0;
	u8 reg[CP_MAX_REG_NUM] = { 0 };
	int i = 0;
	int sw_stat;
	int len = 0, idx = 0, idx_total = 0, len_sysfs = 0;
	char buf_tmp[256] = {0,};

	for (i = 0; i < CP_MAX_REG_NUM; i++) {
		cp_read_byte(sc->client, sc8577_reg_list[i], &reg[i]);
		len = scnprintf(buf_tmp + strlen(buf_tmp), PAGE_SIZE - idx,
			"[0x%02X]=0x%02X,", sc8577_reg_list[i], reg[i]);
		idx += len;

		if (((i + 1) % 8 == 0) || ((i + 1) == CP_MAX_REG_NUM)) {
			mca_log_info("%s %s\n", sc->log_tag, buf_tmp);
			if (val) {
				len_sysfs = scnprintf(val->strval + strlen(val->strval),
					PAGE_SIZE - idx_total, "%s\n", buf_tmp);
			}

			memset(buf_tmp, 0x0, sizeof(buf_tmp));
			idx_total += len_sysfs;
			idx = 0;
		}
	}
	ret = sc8577_field_read(sc, CP_SWITCHING_STAT, &sw_stat);
	if(ret < 0) {
		mca_log_info(" %s failed to read cp switching status:%d\n", sc->log_tag, ret);
		return 0;
	}

	if (sw_stat == SC8577_CP_NOT_SWITCHGING) {
		mca_log_info(" %s cp switching stop, enter abnormal charging judge\n", sc->log_tag);
		sc8577_abnormal_charging_judge(sc, stat_notify, STAT_FIELD_MAX);
	} else {
		sc8577_abnormal_charging_judge(sc, stat_notify_other, STAT_FIELD_MAX_OTHER);
	}

	return 0;
}

/* SC8577_CHG_EN */
static int sc8577_enable_charge(struct sc8577_device *sc, bool enable)
{
	int ret = 0;
	int vbus_value = 0, vout_value = 0, value = 0;
	int vbus_hi = 0, vbus_low = 0;
	mca_log_info("%d\n",enable);

	if (!enable) {
		ret |= sc8577_field_write(sc, CHG_EN, !!enable);

		return ret;
	} else {
		ret = sc8577_get_adc_data(sc, ADC_VBUS, &vbus_value);
		ret |= sc8577_get_adc_data(sc, ADC_VOUT, &vout_value);
		if(vout_value != 0) {
			mca_log_info("vbus/vout:%d / %d = %d \n", vbus_value, vout_value, vbus_value*100/vout_value);
		}

		ret |= sc8577_field_read(sc, MODE, &value);
		mca_log_info("mode:%d %s \n", value, (value == 0 ?"2:1":(value == 1 ?"1:1":"else")));

		ret |= sc8577_field_read(sc, VBUS_ERRORLO_STAT, &vbus_low);
		ret |= sc8577_field_read(sc, VBUS_ERRORHI_STAT, &vbus_hi);
		mca_log_info("high:%d  low:%d \n", vbus_hi, vbus_low);

		ret |= sc8577_field_write(sc, CHG_EN, !!enable);

		disable_irq(sc->irq);

		mdelay(300);

		ret |= sc8577_field_read(sc, CP_SWITCHING_STAT, &value);
		if (!value) {
			mca_log_info("enable fail\n");
			sc8577_dump_important_regs(sc, NULL);
		} else {
			mca_log_info("enable success\n");
		}

		enable_irq(sc->irq);
	}

	return ret;
}

static int sc8577_init_int_src(struct sc8577_device *sc)
{
	int ret = 0;

	ret = sc8577_set_batovp_alarm_int_mask(sc, SC8577_BAT_OVP_ALM_INT_EN);
	if (ret) {
		mca_log_info(" %s failed to set alarm mask:%d\n", sc->log_tag, ret);
		return ret;
	}
	ret = sc8577_set_busocp_alarm_int_mask(sc, SC8577_BUS_OCP_ALM_INT_EN);
	if (ret) {
		mca_log_info("%s failed to set alarm mask:%d\n", sc->log_tag, ret);
		return ret;
	}

	return ret;
}

static int sc8577_init_protection(struct sc8577_device *cp, int work_mode)
{
	int ret = 0;

	ret = sc8577_enable_batovp(cp, true);
	ret = sc8577_enable_batocp(cp, false);
	ret = sc8577_enable_busocp(cp, true);
	ret = sc8577_enable_busucp(cp, true);
	ret = sc8577_enable_pmid2outovp(cp, true);
	ret = sc8577_enable_pmid2outuvp(cp, true);
	ret = sc8577_enable_batovp_alarm(cp, true);
	ret = sc8577_enable_busocp_alarm(cp, true);
	ret = sc8577_set_batovp_th(cp, cp->cfg.bat_ovp_th);
	ret = sc8577_set_batocp_th(cp, BAT_OCP_TH);
	ret = sc8577_set_batovp_alarm_th(cp, cp->cfg.bat_ovp_alarm_th);
	ret = sc8577_set_busovp_th(cp, cp->cfg.bus_ovp_th[work_mode]);
	ret = sc8577_set_usbovp_th(cp, cp->cfg.usb_ovp_th[work_mode]);
	ret = sc8577_set_busocp_th(cp, cp->cfg.bus_ocp_th[work_mode]);
	ret = sc8577_set_tdie_alm_th(cp, cp->cfg.tdie_alm_th);

	ret = sc8577_set_wpcovp_th(cp, cp->cfg.wpc_ovp_th);
	ret = sc8577_set_outovp_th(cp, cp->cfg.out_ovp_th);
	ret = sc8577_set_pmid2outuvp_th(cp, cp->cfg.pmid2out_uvp_th);
	ret = sc8577_set_pmid2outovp_th(cp, cp->cfg.pmid2out_ovp_th);

	return ret;
}

static int sc8577_init_adc(struct sc8577_device *cp)
{
	sc8577_set_adc_scanrate(cp, false);
	sc8577_set_adc_scan(cp, ADC_IBUS, true);
	sc8577_set_adc_scan(cp, ADC_VBUS, true);
	sc8577_set_adc_scan(cp, ADC_VAC1, true);
	sc8577_set_adc_scan(cp, ADC_VAC2, true);
	sc8577_set_adc_scan(cp, ADC_VOUT, true);
	sc8577_set_adc_scan(cp, ADC_VBAT, true);
	sc8577_set_adc_scan(cp, ADC_TSBAT, true);
	sc8577_set_adc_scan(cp, ADC_TDIE, true);
	sc8577_enable_adc(cp, false);

	return 0;
}

static int sc8577_init_device(struct sc8577_device *cp)
{
	int ret = 0;
	int retry_cnt = 0;

	while (retry_cnt < ERROR_RECOVERY_COUNT)	{
		sc8577_enable_acdrv_manual_ovpgate_wpcgate(cp, false);
		ret |= sc8577_set_ss_timeout(cp, SC8577_SS_TIMEOUT_10000MS);
		ret |= sc8577_set_ucp_fall_dg(cp, SC8577_BUS_UCP_FALL_DG_5MS);
		ret |= sc8577_init_adc(cp);
		ret |= sc8577_init_int_src(cp);
		ret |= sc8577_set_operation_mode(cp, CP_MODE_FORWARD_1_1);
		ret |= sc8577_tsbat_flt_dis(cp, true);

		switch (cp->cp_role) {
		case SC8577_MASTER:
			sc8577_set_switch_freq(cp, SC8577_SW_FREQ_500KHZ);
			break;
		case SC8577_SLAVE:
			sc8577_set_switch_freq(cp, SC8577_SW_FREQ_600KHZ);
			break;
		default:
			break;
		}

		if (ret < 0) {
			retry_cnt++;
		} else {
			mca_log_err("%s success to init CP device\n", cp->log_tag);
			break;
		}
	}

	return ret;
}

static int cp_get_adc_data(struct sc8577_device *sc, int channel,  u32 *result)
{
	int ret = 0;

	ret = sc8577_get_adc_data(sc, channel, result);

	if (ret)
		mca_log_info(" %s failed get ADC value\n", sc->log_tag);


	return ret;
}

static int ops_cp_get_int_stat(int channel, bool *result, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_get_int_stat(sc, channel, result);

	if (ret)
		mca_log_info("failed get int stat %d\n", channel);

	return ret;
}

static int cp_enable_adc(struct sc8577_device *sc, bool enable)
{
	int ret = 0;

	ret = sc8577_enable_adc(sc, enable);

	if (ret)
		mca_log_info("%s failed to enable/disable ADC\n", sc->log_tag);

	return ret;
}

static int ops_cp_dump_register(void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_dump_important_regs(sc, NULL);

	if (ret)
		mca_log_info("%s failed dump registers ret=%d\n", sc->log_tag, ret);

	return ret;
}

static int ops_cp_get_chip_vendor(int *chip_id, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;

	*chip_id = sc->chip_vendor;
	mca_log_info("%s %d\n", sc->log_tag, *chip_id);

	return 0;
}

static int ops_cp_enable_charge(bool enable, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_enable_charge(sc, enable);

	if (ret)
		mca_log_err("%s failed enable cp charge\n", sc->log_tag);

	return ret;
}

static int ops_cp_get_charge_enable(bool *enabled, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_check_charge_enabled(sc, enabled);

	if (ret)
		mca_log_err("%s failed get enable cp charge status ret =%d\n", sc->log_tag, ret);


	return ret;
}

static int ops_cp_get_vbus(u32 *val, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = cp_get_adc_data(sc, ADC_VBUS, val);

	return ret;
}

static int ops_cp_get_vusb(u32 *val, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = cp_get_adc_data(sc, ADC_VAC1, val);

	return ret;

}

static int ops_cp_get_ibus(u32 *val, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = cp_get_adc_data(sc, ADC_IBUS, val);

	return ret;
}

static int ops_cp_get_vbatt(u32 *val, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = cp_get_adc_data(sc, ADC_VBAT, val);

	return ret;
}

static int ops_cp_set_mode(int value, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_set_operation_mode(sc, value);

	if (ret)
		mca_log_err("%s failed set cp charge mode\n", sc->log_tag);

	return ret;
}

static int ops_cp_set_default_mode(void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_set_operation_mode(sc, CP_MODE_FORWARD_1_1);

	if (ret)
		mca_log_err("%s failed set cp default charge mode\n", sc->log_tag);

	return ret;
}

static int ops_cp_get_mode(int *mode, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_get_operation_mode(sc, mode);
	if (ret)
		mca_log_err("%s failed to get div_mode\n", sc->log_tag);

	return ret;
}

static int ops_cp_device_init(int value, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_init_device(sc);

	if (ret)
		mca_log_err("%s failed init cp init device\n", sc->log_tag);

	return ret;
}

static int ops_cp_enable_adc(bool enable, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = cp_enable_adc(sc, enable);

	return ret;
}



static int ops_cp_get_bypass_support(bool *enabled, void *chg_dev)
{
	*enabled = true;
	mca_log_info("%s %d\n", __func__, *enabled);

	return 0;
}

static int ops_enable_acdrv_manual(bool enable, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_enable_acdrv_manual(sc, enable);

	if (ret)
		mca_log_info("%s failed to set cp acdrv manual %s\n",
			sc->log_tag, enable ? "enable" : "disable");


	return ret;
}

static int ops_cp_enable_ovpgate(bool enable, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_enable_acdrv_manual(sc, !enable);
	if (ret)
		mca_log_info("%s failed to set cp acdrv manual %s\n",
			sc->log_tag, enable ? "enable" : "disable");

	ret = sc8577_enable_ovpgate(sc, enable);

	if (ret)
		mca_log_info("%s failed set cp ovpgate %s\n",
			sc->log_tag, enable ? "enable" : "disable");

	return ret;
}

static int ops_cp_get_ovpgate_status(bool *enable, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_get_ovpgate_status(sc, enable);

	if (ret)
		mca_log_info("%s failed enable cp acdrv manual\n", sc->log_tag);

	return ret;

}

static int ops_cp_get_present(bool *present, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	*present  = sc->chip_ok;
	return ret;
}

static int ops_cp_get_battery_temmperature(u32 *val, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = cp_get_adc_data(sc, ADC_TSBAT, val);
	return ret;

}

static int ops_cp_get_battery_present(bool *present, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_get_int_stat(sc, VOUT_INSERT_STAT, present);

	return ret;
}

static int ops_cp_get_errorhl_stat(int *stat, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;
	int val = 0;

	ret = sc8577_get_status(sc, &val);
	*stat = CP_PMID_ERROR_OK;
	if (val & BIT(ERROR_VBUS_HIGH)) {
		*stat = CP_PMID_ERROR_HIGH;
	} else if (val & BIT(ERROR_VBUS_LOW)) {
		*stat = CP_PMID_ERROR_LOW;
	}

	mca_log_info("%s val: 0x%02x, stat: %d", sc->log_tag, val, *stat);
	return ret;
}

static int ops_cp_enable_busucp(bool en, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = sc8577_enable_busucp(sc, en);
	if (ret)
		mca_log_err("%s failed to [%d] busucp ret=%d\n", sc->log_tag, en, ret);

	return ret;
}

static int ops_cp_get_tdie(int *val, void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = cp_get_adc_data(sc, ADC_TDIE, val);
	return ret;
}

static int ops_cp_check_iic_ok(void *chg_dev)
{
	struct sc8577_device *sc = (struct sc8577_device *)chg_dev;
	int ret = 0;

	ret = cp_charge_detect_device(sc);
	if (ret < 0)
		mca_log_err("cp iic error\n");

	return ret;
}

static struct platform_class_cp_ops sc8577_chg_ops = {
	.cp_set_enable = ops_cp_enable_charge,
	.cp_get_enabled = ops_cp_get_charge_enable,
	.cp_get_bus_voltage = ops_cp_get_vbus,
	.cp_get_bus_current = ops_cp_get_ibus,
	.cp_get_battery_voltage = ops_cp_get_vbatt,
	.cp_get_battery_temperature = ops_cp_get_battery_temmperature,
	.cp_get_battery_present = ops_cp_get_battery_present,
	.cp_set_mode = ops_cp_set_mode,
	.cp_set_default_mode = ops_cp_set_default_mode,
	.cp_get_mode = ops_cp_get_mode,
	.cp_device_init = ops_cp_device_init,
	.cp_enable_adc = ops_cp_enable_adc,
	.cp_get_bypass_support = ops_cp_get_bypass_support,
	.cp_dump_register = ops_cp_dump_register,
	.cp_get_chip_vendor = ops_cp_get_chip_vendor,
	.cp_enable_acdrv_manual = ops_enable_acdrv_manual,
	.cp_enable_ovpgate = ops_cp_enable_ovpgate,
	.cp_get_ovpgate_status = ops_cp_get_ovpgate_status,
	.cp_get_present = ops_cp_get_present,
	.cp_get_usb_voltage = ops_cp_get_vusb,
	.cp_get_int_stat = ops_cp_get_int_stat,
	.cp_get_errorhl_stat = ops_cp_get_errorhl_stat,
	.cp_enable_busucp = ops_cp_enable_busucp,
	.cp_get_tdie = ops_cp_get_tdie,
	.cp_check_iic_ok = ops_cp_check_iic_ok,
};

static void sc8577_irq_handler(struct work_struct *work)
{
	struct sc8577_device *sc = container_of(work, struct sc8577_device, irq_handle_work.work);
	bool usb_present;

	sc8577_get_int_stat(sc, VUSB_PRESENT_STAT, &usb_present);
	mca_log_info("%s usb_present = %d, hanler %s\n", sc->log_tag, usb_present, __func__);
	if (!sc->wls_sleep_usb_insert) {
		if (usb_present) {
			mca_event_block_notify(MCA_EVENT_TYPE_CP_INFO, MCA_EVENT_CP_VUSB_INSERT, NULL);
			sc->wls_sleep_usb_insert = true;
		}
	} else {
		if (!usb_present) {
			mca_event_block_notify(MCA_EVENT_TYPE_CP_INFO, MCA_EVENT_CP_VUSB_OUT, NULL);
			sc->wls_sleep_usb_insert = false;
		}
	}

	mca_log_info("%s hanler %s\n", sc->log_tag, __func__);
	sc8577_dump_important_regs(sc, NULL);
}

static irqreturn_t sc8577_int_isr(int irq, void *private)
{
	struct sc8577_device *sc = private;

	mca_log_info("%s %s\n", sc->log_tag, __func__);

	schedule_delayed_work(&sc->irq_handle_work, 0);

	return IRQ_HANDLED;
}

static int sc8577_parse_dt(struct sc8577_device *sc)
{
	struct device_node *np = sc->dev->of_node;
	int ret = 0;

	if (!np) {
		mca_log_err("device tree info missing\n");
		return -1;
	}

	mca_parse_dts_u32(np, "ic_role", &sc->cp_role, 0);

	if (sc->cp_role == SC8577_SLAVE)
		strscpy(sc->log_tag, "[1]", sizeof("[1]"));
	else
		strscpy(sc->log_tag, "[0]", sizeof("[0]"));

	sc->irq_gpio = of_get_named_gpio(np, "cp-int", 0);
	if (!gpio_is_valid(sc->irq_gpio)) {
		mca_log_err("%s failed to parse irq_gpio\n", sc->log_tag);
		return -1;
	}

	sc->nlpm_gpio = of_get_named_gpio(np, "cp-nlpm-gpio", 0);
	if (!gpio_is_valid(sc->nlpm_gpio)) {
		mca_log_err("%s failed to parse  sc858_nlpm_gpio\n", sc->log_tag);
		return -1;
	}

	mca_parse_dts_u32(np, "tdie-alm-threshold", &sc->cfg.tdie_alm_th, TDIE_ALM_TH);
	mca_parse_dts_u32(np, "bat-ovp-threshold", &sc->cfg.bat_ovp_th, BAT_OVP_TH);
	mca_parse_dts_u32(np, "bat-ovp-alarm-threshold", &sc->cfg.bat_ovp_alarm_th, BAT_OVP_ALARM_TH);
	mca_parse_dts_u32_array(np, "bus-ovp-threshold", sc->cfg.bus_ovp_th, CP_MODE_DIV_MAX);
	mca_log_info("%s bus_ovp [%d,%d]\n", sc->log_tag, sc->cfg.bus_ovp_th[0], sc->cfg.bus_ovp_th[1]);
	mca_parse_dts_u32_array(np, "usb-ovp-threshold", sc->cfg.usb_ovp_th, CP_MODE_DIV_MAX);
	mca_log_info("%s usb_ovp [%d,%d]\n", sc->log_tag, sc->cfg.usb_ovp_th[0], sc->cfg.usb_ovp_th[1]);
	mca_parse_dts_u32_array(np, "bus-ocp-threshold", sc->cfg.bus_ocp_th, CP_MODE_DIV_MAX);
	mca_log_info("%s bus_ocp [%d,%d]\n", sc->log_tag, sc->cfg.bus_ocp_th[0], sc->cfg.bus_ocp_th[1]);
	mca_parse_dts_u32(np, "wpc-ovp-threshold", &sc->cfg.wpc_ovp_th, WPC_OVP_TH);
	mca_parse_dts_u32(np, "out-ovp-threshold", &sc->cfg.out_ovp_th, OUT_OVP_TH);
	mca_parse_dts_u32(np, "pmid2-uvp-threshold", &sc->cfg.pmid2out_uvp_th, PMID2OUT_UVP_TH);
	mca_parse_dts_u32(np, "pmid2-ovp-threshold", &sc->cfg.pmid2out_ovp_th, PMID2OUT_OVP_TH);

	return ret;
}

static int sc8577_register_irq(struct sc8577_device *sc)
{
	int ret = 0;

	ret = devm_gpio_request(sc->dev, sc->irq_gpio, dev_name(sc->dev));
	if (ret < 0) {
		mca_log_info(" %s failed to master request gpio\n", sc->log_tag);
		return -1;
	}

	sc->irq = gpio_to_irq(sc->irq_gpio);
	if (sc->irq_gpio < 0) {
		mca_log_info("%s failed to master get gpio_irq\n", sc->log_tag);
		return -1;
	}

	ret = request_irq(sc->irq, sc8577_int_isr,
		IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
		dev_name(sc->dev), sc);
	if (ret < 0) {
		mca_log_info("%s failed to master request irq\n", sc->log_tag);
		return -1;
	}

	enable_irq_wake(sc->irq);

	return 0;
}

static int sc8577_init_gpio(struct sc8577_device *sc)
{
	int ret = 0;

	if (sc->cp_role != SC8577_MASTER)
		return ret;

	ret = devm_gpio_request(sc->dev, sc->nlpm_gpio, dev_name(sc->dev));
	if (ret)
		mca_log_info("%s unable to request nlpm gpio [%d]\n", sc->log_tag, sc->nlpm_gpio);
	else {
		ret = gpio_direction_output(sc->nlpm_gpio, 1);
		if (ret)
			mca_log_info("%s unable to set direction for nlpm gpio[%d]\n", sc->log_tag, sc->nlpm_gpio);
		msleep(400);
	}

	return ret;
}

static int sc8577_register_platform(struct sc8577_device *sc)
{
	sc8577_set_reg_reset(sc);
	platform_class_cp_register_ops(sc->cp_role, &sc8577_chg_ops, sc);
	sc8577_enable_parallel_func(sc, true);

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 9))
static int sc8577_probe(struct i2c_client *client)
#else
static int sc8577_probe(struct i2c_client *client, const struct i2c_device_id *id)
#endif
{
	struct device *dev = &client->dev;
	struct sc8577_device *sc;
	int ret = 0;
	int i = 0;

	sc = devm_kzalloc(dev, sizeof(*sc), GFP_KERNEL);
	if (!sc)
		return -ENOMEM;

	sc->client = client;
	sc->dev = dev;

	sc->regmap = devm_regmap_init_i2c(client,
							&sc8577_regmap_config);
	if (IS_ERR(sc->regmap)) {
		mca_log_err("Failed to initialize regmap\n");
		ret = PTR_ERR(sc->regmap);
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(sc8577_reg_fields); i++) {
		const struct reg_field *reg_fields = sc8577_reg_fields;

		sc->rmap_fields[i] =
			devm_regmap_field_alloc(sc->dev,
						sc->regmap,
						reg_fields[i]);
		if (IS_ERR(sc->rmap_fields[i])) {
			mca_log_err("cannot allocate regmap field\n");
			ret = PTR_ERR(sc->rmap_fields[i]);
			return ret;
		}
	}

	i2c_set_clientdata(client, sc);

	ret = sc8577_parse_dt(sc);
	if (ret) {
		mca_log_err(" %s failed to parse DTS\n", sc->log_tag);
		return ret;
	}

	ret = sc8577_init_gpio(sc);
	ret = cp_charge_detect_device(sc);
	if (ret) {
		mca_log_err("%s failed to detect device\n", sc->log_tag);
		return ret;
	}

	INIT_DELAYED_WORK(&sc->irq_handle_work, sc8577_irq_handler);
	ret = sc8577_register_irq(sc);
	if (ret) {
		mca_log_err("%s failed to int irq\n", sc->log_tag);
		return ret;
	}

	ret = sc8577_register_platform(sc);
	ret = sc8577_init_device(sc);
	if (ret) {
		mca_log_err("%s failed to init sc8577\n", sc->log_tag);
		return ret;
	}

	sc->chip_ok = true;
#ifdef CONFIG_DEBUG_FS
	reg_info.address = 0x00;
	reg_info.count = 1;

	if (sc->cp_role == SC8577_SLAVE)
		mca_debugfs_create_group("sc857x_01", cp_debugfs_field_tbl, CP_DEBUGFS_ATTRS_SIZE, sc);
	else
		mca_debugfs_create_group("sc857x_00", cp_debugfs_field_tbl, CP_DEBUGFS_ATTRS_SIZE, sc);
#endif
	mca_log_err("%s probe success %d\n", sc->log_tag, ret);

	return 0;
}

static int sc8577_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sc8577_device *sc = i2c_get_clientdata(client);
	int ret = 0;

	mca_log_info("%s sc8577 suspend!\n", sc->log_tag);
	ret = cp_enable_adc(sc, false);
	if (ret)
		mca_log_err("%s failed to disable ADC\n", sc->log_tag);

	ret = enable_irq_wake(sc->irq);

	return ret;
}

static int sc8577_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sc8577_device *sc = i2c_get_clientdata(client);
	int ret = 0;

	mca_log_info("%s sc8577 resume!\n", sc->log_tag);
	ret = disable_irq_wake(sc->irq);

	return ret;
}

static const struct dev_pm_ops sc8577_pm_ops = {
	.suspend	= sc8577_suspend,
	.resume		= sc8577_resume,
};

static void sc8577_remove(struct i2c_client *client)
{
	struct sc8577_device *sc = i2c_get_clientdata(client);
	int ret = 0;

	ret = cp_enable_adc(sc, false);
	if (ret)
		mca_log_err("%s failed to disable ADC\n", sc->log_tag);
}

static void sc8577_shutdown(struct i2c_client *client)
{
	struct sc8577_device *sc = i2c_get_clientdata(client);
	int ret = 0;

	ret = cp_enable_adc(sc, false);
	ret |= gpio_direction_output(sc->nlpm_gpio, 0);
	if (ret)
		mca_log_err("%s unable to reset adc and nlpm fail\n", sc->log_tag);

	mca_log_info("%s sc8577 shutdown!\n", sc->log_tag);
}

static const struct of_device_id sc8577_of_match[] = {
	{ .compatible = "sc8577"},
	{ .compatible = "sc8577_master"},
	{ .compatible = "sc8577_slave"},
	{},
};
MODULE_DEVICE_TABLE(of, sc8577_of_match);

static struct i2c_driver sc8577_driver = {
	.driver = {
		.name = "sc8577_charger_pump",
		.of_match_table = sc8577_of_match,
		.pm = &sc8577_pm_ops,
	},
	.probe = sc8577_probe,
	.remove = sc8577_remove,
	.shutdown = sc8577_shutdown,
};
module_i2c_driver(sc8577_driver);

MODULE_AUTHOR("jiaomenglong <jiaomenglong@xiaomi.com>");
MODULE_DESCRIPTION("southchip SC8581 driver");
MODULE_LICENSE("GPL v2");
