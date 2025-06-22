/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sc8577.h
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

#ifndef __SC8577_H__
#define __SC8577_H__

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/err.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include <mca/common/mca_event.h>
#include <linux/debugfs.h>
#include <linux/bitops.h>
#include <linux/math64.h>
#include <linux/version.h>
#include <linux/regmap.h>

#define SC8577_ROLE_MASTER		0
#define SC8577_ROLE_SLAVE		1
#define ERROR_RECOVERY_COUNT	5

#define BAT_OCP_TH 8000
#define WPC_OVP_TH  22000
#define OUT_OVP_TH 10000
#define PMID2OUT_UVP_TH 100
#define PMID2OUT_OVP_TH 600
#define BAT_OVP_TH 9300
#define BAT_OVP_ALARM_TH 9200
#define TDIE_ALM_TH 700

#define I2C_RETRY_CNT 5

enum  sc8577_adc_ch {
	ADC_IBUS,
	ADC_VBUS,
	ADC_VAC1,
	ADC_VAC2,
	ADC_VOUT,
	ADC_VBAT,
	ADC_RESERVED1,
	ADC_RESERVED2,
	ADC_TSBAT,
	ADC_TDIE,
	ADC_MAX_NUM,
};

enum cp_number {
	SC8577_MASTER,
	SC8577_SLAVE,
};

enum cp_model {
	SC8577,
};

enum chg_mode {
	CP_MODE_DIV2,
	CP_MODE_DIV1,
	CP_MODE_DIV_MAX,
};


/************************************************************************/
#define SC8577_DEVICE_ID                0x75

#define SC8577_VBAT_OVP_MIN             7000
#define SC8577_VBAT_OVP_MAX             10810
#define SC8577_VBAT_OVP_STEP            30

#define SC8577_VBUS_OVP_MIN             7000
#define SC8577_VBUS_OVP_MAX             13350
#define sc8577_VBUS_OVP_STEP            50

#define SC8577_IBUS_OCP_MIN             1000
#define SC8577_IBUS_OCP_MAX             8750
#define SC8577_IBUS_OCP_STEP            250

#define SC8577_REG18                    0x18
#define SC8577_REG25                    0x25
#define SC8577_REGMAX                   0x55

/* mode */
#define CP_FORWARD_4_TO_1    0
#define CP_FORWARD_2_TO_1    1
#define CP_FORWARD_1_TO_1    2
#define SC8577_SET_2_1_MODE 0
#define SC8577_SET_1_1_MODE 1

/* ss_timeout */
#define SC8577_SS_TIMEOUT_DISABLE	  0
#define SC8577_SS_TIMEOUT_20MS      1
#define SC8577_SS_TIMEOUT_40MS      2
#define SC8577_SS_TIMEOUT_80MS     3
#define SC8577_SS_TIMEOUT_160MS    4
#define SC8577_SS_TIMEOUT_320MS    5
#define SC8577_SS_TIMEOUT_1280MS   6
#define SC8577_SS_TIMEOUT_10000MS   7

/* deglitch time */
#define SC8577_BUS_UCP_FALL_DG_10US     0
#define SC8577_BUS_UCP_FALL_DG_5MS     1
#define SC8577_BUS_UCP_FALL_DG_40MS    2
#define SC8577_BUS_UCP_FALL_DG_640MS    3

/* INT MASK */
#define SC8577_BAT_OVP_ALM_INT_EN   0
#define SC8577_BAT_OVP_ALM_INT_DIS    1
#define SC8577_BUS_OCP_ALM_INT_EN   0
#define SC8577_BUS_OCP_ALM_INT_DIS    1

/* switch frequncy in forward 2:1 mode */
#define SC8577_SW_FREQ_300KHZ	0
#define SC8577_SW_FREQ_350KHZ	1
#define SC8577_SW_FREQ_400KHZ	2
#define SC8577_SW_FREQ_500KHZ	3
#define SC8577_SW_FREQ_600KHZ	4
#define SC8577_SW_FREQ_700KHZ	5
#define SC8577_SW_FREQ_800KHZ	6
#define SC8577_SW_FREQ_1000KHZ	7

/* switch state */
#define SC8577_CP_NOT_SWITCHGING 0
#define SC8577_CP_SWITCHGING 1

enum sc8577_reg_range {
	SC8577_VBAT_OVP,
	SC8577_VBAT_OVP_ALM,
	SC8577_VOUT_OVP,
	SC8577_VBUS_OVP_MODE_1_1,
	SC8577_VBUS_OVP_MODE_2_1,
	SC8577_IBUS_OCP,
	SC8577_VAC1_OVP,
	SC8577_VAC2_OVP,
	SC8577_PMID2OUT_OVP,
	SC8577_PMID2OUT_UVP,
	SC8577_WD_TIMEOUT,
	SC8577_TDIE_ALM_TH,
};

enum sc8577_error_stata {
	ERROR_VBUS_HIGH = 0,
	ERROR_VBUS_LOW,
	ERROR_VBUS_OVP,
	ERROR_IBUS_OCP,
	ERROR_VBAT_OVP,
	ERROR_IBAT_OCP,
};

enum sc8577_fields {
	VBAT_OVP_DIS, VBAT_OVP, /*reg00*/
	VBAT_OVP_ALM_DIS, VBAT_OVP_ALM, /*reg01*/
	IBUS_UCP_DIS, IBUS_UCP_TH, VBUS_IN_RANGE_DIS, /*reg05*/
	BUS_PD_EN, VBUS_OVP, /*reg06*/
	IBUS_OCP_DIS, IBUS_OCP, /*reg08*/
	TSHUT_DIS, TDIE_ALM_DIS, TSBAT_FLT_DIS, /*reg0A*/
	TDIE_ALM, /*reg0B*/
	TSBAT_FLT, /*reg0D*/
	VAC1_OVP, VAC2_OVP, VAC1_PD_EN, VAC2_PD_EN, /*reg0E*/
	REG_RST, CHG_EN, MODE, ACDRV1_STAT, ACDRV2_STAT, /*reg0F*/
	FSW_SET, WD_TIMEOUT, WD_TIMEOUT_DIS, /*reg10*/
	OTG_EN, SS_TIMEOUT, IBUS_UCP_FALL_DEG, /*reg11*/
	VOUT_OVP_DIS, VOUT_OVP, FREQ_SHIFT, MS, /*reg12*/
	VBAT_OVP_STAT, VBAT_OVP_ALM_STAT, VOUT_OVP_STAT, VBUS_OVP_STAT, /*reg13*/
	IBUS_OCP_STAT, IBUS_UCP_FALL_STAT, PIN_DIAG_FAIL_STAT, /*reg14*/
	VAC1_OVP_STAT, VAC2_OVP_STAT, VBAT_INSERT_STAT, VAC1_INSERT_STAT,
	VAC2_INSERT_STAT, CP_VBUS_PRESENT_STAT, ACRB1_CFG_STAT, ACRB2_CFG_STAT, /*reg15*/
	ADC_DONE_STAT, SS_TIMEOUT_STAT, TSBAT_FLT_STAT,
	TSHUT_FLT_STAT, TDIE_ALM_STAT, WD_TIMEOUT_STAT, /*reg16*/
	CP_SWITCHING_STAT, VBUS_ERRORHI_STAT, VBUS_ERRORLO_STAT, /*reg17*/
	VBATOVP_FLAG, VBATOVP_ALM_FLAG, VOUT_OVP_FLAG, VBUS_OVP_FLAG, /*reg18*/
	IBUS_OCP_FLAG, IBUS_UCP_FALL_FLAG, PIN_DIAG_FAIL_FLAG, /*reg19*/
	VAC1_OVP_FLAG, VAC2_OVP_FLAG, VBAT_INSERT_FLAG, VAC1_INSERT_FLAG,
	VAC2_INSERT_FLAG, VBUS_PRESENT_FLAG, ACRB1_CFG_FLAG, ACRB2_CFG_FLAG, /*reg1A*/
	ADC_DONE_FLAG, SS_TIMEOUT_FLAG, TSBAT_FLT_FLAG,
	TSHUT_FLAG, TDIE_ALM_FLAG, WD_TIMEOUT_FLAG, /*reg1B*/
	VBUS_ERRORHI_FLAG, VBUS_ERRORLO_FLAG, /*reg1C*/
	VBAT_OVP_MASK, VBAT_OVP_ALM_MASK, VOUT_OVP_MASK, VBUS_OVP_MASK, /*reg1D*/
	IBUS_OCP_MASK, IBUS_UCP_FALL_MASK, PIN_DIAG_FAIL_MASK, /*reg1E*/
	VAC1_OVP_MASK, VAC2_OVP_MASK, VBAT_INSERT_MASK, VAC1_INSERT_MASK,
	VAC2_INSERT_MASK, VBUS_PRESENT_MASK, ACRB1_CFG_MASK, ACRB2_CFG_MASK, /*reg1F*/
	ADC_DONE_MASK, SS_TIMEOUT_MASK, TSBAT_FLT_MASK,
	TSHUT_MASK, TDIE_ALM_MASK, WD_TIMEOUT_MASK, /*reg20*/
	VBUS_ERRORHI_MASK, VBUS_ERRORLO_MASK, /*reg21*/
	DEVICE_ID, /*reg22*/
	CP_ADC_EN, ADC_RATE, ADC_FREEZE, IBUS_ADC_DIS, VBUS_ADC_DIS, /*reg23*/
	VAC1_ADC_DIS, VAC2_ADC_DIS, VOUT_ADC_DIS, VBAT_ADC_DIS,
	CP_TSBAT_ADC_DIS, CP_TDIE_ADC_DIS, /*reg24*/
	POR_FLAG, ACDRV_MANUAL_EN, ACDRV1_EN, ACDRV2_EN,
	IBUS_UCP_RISE_FLAG, IBUS_UCP_RISE_MASK, VOUT_OK_CHG_FLAG, VOUT_OK_CHG_MASK, /*reg40*/
	SS_TIMEOUT_DIS, VBUS_OVP_DIS, PMID2OUT_OVP_DIS, PMID2OUT_UVP_DIS,
	ACRB1_DET_DIS, ACRB2_DET_DIS, VBAT_SNS_DIS, /*reg41*/
	PMID2OUT_UVP, PMID2OUT_OVP, PMID2OUT_UVP_FLAG, PMID2OUT_OVP_FLAG, /*reg42*/
	PMID2OUT_UVP_MASK, PMID2OUT_OVP_MASK, VBUS_TH_CHG_EN_STAT, VOUT_TH_SW_REGN_STAT, /*reg43*/
	VAC_UVLO_FALL_DG, DEVICE_VER, SYNC_EN, ACDRV_BYPASS_EN, /*reg50*/
	WORK_MODE, /*reg54*/
	VBUS_FORCE_ON_PD, VBUS_FORCE_OFF_PD, /*reg55*/
	CP_F_MAX_FIELDS,
};

struct stat_mca_notify {
	enum sc8577_fields field;
	enum mca_event_notify_list event;
	char* err_str;
};

struct reg_range {
	u32 min;
	u32 max;
	u32 step;
	u32 offset;
	const u32 *table;
	u16 num_table;
	bool round_up;
};

#define SC8577_CHG_RANGE(_min, _max, _step, _offset, _ru) \
{ \
	.min = _min, \
	.max = _max, \
	.step = _step, \
	.offset = _offset, \
	.round_up = _ru, \
}

#define SC8577_CHG_RANGE_T(_table, _ru) \
	{ .table = _table, .num_table = ARRAY_SIZE(_table), .round_up = _ru, }

struct sc8577_cfg {
	unsigned int bat_ovp_th;
	unsigned int bat_ovp_alarm_th;
	unsigned int wpc_ovp_th;
	unsigned int out_ovp_th;
	unsigned int pmid2out_uvp_th;
	unsigned int pmid2out_ovp_th;
	unsigned int tdie_alm_th;
	unsigned int bus_ovp_th[CP_MODE_DIV_MAX];
	unsigned int usb_ovp_th[CP_MODE_DIV_MAX];
	unsigned int bus_ocp_th[CP_MODE_DIV_MAX];
};

struct sc8577_device {
	struct i2c_client *client;
	struct device *dev;
	struct device *sysfs_dev;
	struct charger_device *chg_dev;
	struct power_supply *cp_psy;
	struct power_supply_desc psy_desc;
	struct regmap *regmap;
	struct regmap_field *rmap_fields[CP_F_MAX_FIELDS];

	struct sc8577_cfg cfg;
	bool chip_ok;
	char log_tag[25];
	int work_mode;
	int operation_mode;
	int chip_vendor;
	u8 adc_mode;
	unsigned int revision;
	unsigned int product_cfg;
	int cp_role;
	bool wls_sleep_usb_insert;

	struct delayed_work irq_handle_work;
	int irq_gpio;
	int irq;
	int nlpm_gpio;
};

union cp_propval {
	unsigned int uintval;
	int intval;
	char strval[PAGE_SIZE];
};

extern int subpmic_dev_notify(int event, bool plug);
extern int register_subpmic_device_notifier(struct notifier_block *nb);
#endif /* __SC8577_H__ */