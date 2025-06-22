// SPDX-License-Identifier: GPL-2.0
/*
 *subpmic_sc6601a_driver.c
 *
 * subpmic driver
 *
 * Copyright (c) 2024-2024 Xiaomi Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/power_supply.h>
#include <linux/regmap.h>
#include <linux/types.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/usb/phy.h>
#include <linux/acpi.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/kthread.h>
#include <linux/sched/prio.h>
#include <uapi/linux/sched/types.h>
#include <linux/platform_device.h>
#include <mca/common/mca_log.h>
#include <mca/common/mca_event.h>
#include <mca/protocol/protocol_pd_class.h>
#include "mca/platform/platform_buckchg_class.h"
#include <mca/common/mca_charge_interface.h>
#include <mca/platform/platform_cp_class.h>
#include <mca/platform/platform_wireless_class.h>
#include <xr-usb-repeater.h>
#include <xr_usbdp_event.h>
#include "inc/subpmic.h"
#include <mca/platform/platform_bc12_class.h>
#include <mca/common/mca_parse_dts.h>
#include <mca/common/mca_workqueue.h>
#include <mca/platform/platform_fg_ic_ops.h>
#include <mca/common/mca_parse_dts.h>
#include "../../../../mca_hardware_monitor/inc/mca_lpd_detect.h"
#include <mca/hardware/hw_connector_antiburn.h>

#ifdef CONFIG_ENABLE_BOOT_DEBUG
#include "../get_boot.h"
#endif
#ifdef CONFIG_MTK_CLASS
#include "mtk_charger_v5p10/charger_class.h"
#include "mtk_charger_v5p10/mtk_charger.h"
#endif /*CONFIG_MTK_CLASS */

#define CONFIG_SUBPMIC_CID

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_subpmic_sc6601a_drv"
#endif

#define CONFIG_SOUTHCHIP_CHARGER_CLASS

#define SUBPMIC_CHARGER_VERSION         "1.1.2"

#define BC12_TYPE_NONE                  0
#define SUBPMIC_BC_DETECT_VINDPM        5000

#define CUTOFF_BUCK_VBUS_GPIO_VAL        1
#define RESUME_BUCK_VBUS_GPIO_VAL        0

struct subpmic_chg_device *qc_device;

struct buck_init_data {
	u32 vsyslim;
	u32 batsns_en;
	u32 vbat;
	u32 ichg;
	u32 vindpm;
	u32 iindpm_dis;
	u32 iindpm;
	u32 ico_enable;
	u32 iindpm_ico;
	u32 vprechg;
	u32 iprechg;
	u32 iterm_en;
	u32 iterm;
	u32 rechg_dis;
	u32 rechg_dg;
	u32 rechg_volt;
	u32 vboost;
	u32 iboost;
	u32 conv_ocp_dis;
	u32 tsbat_jeita_dis;
	u32 ibat_ocp_dis;
	u32 vpmid_ovp_otg_dis;
	u32 vbat_ovp_buck_dis;
	u32 ibat_ocp;
	u32 chg_timer;
	u32 sc660x_pd_auth_en;
};

enum subpmic_chg_fields {
	F_VAC_OVP, F_VBUS_OVP,
	F_TSBUS_FLT,
	F_TSBAT_FLT,
	F_ACDRV_MANUAL_PRE, F_ACDRV_EN, F_ACDRV_MANUAL_EN, F_WD_TIME_RST, F_WD_TIMER,
	F_REG_RST, F_VBUS_PD, F_VAC_PD, F_CID_EN,
	F_ADC_EN, F_ADC_FREEZE, F_BATID_ADC_EN,
	F_EDL_ACTIVE_LEVEL,
	/******* charger *******/
	F_VSYS_MIN,     /* REG30 */
	F_BATSNS_EN, F_VBAT, /* REG31 */
	F_ICHG_CC, /* REG32 */
	F_VINDPM_VBAT, F_VINDPM_DIS, F_VINDPM, /* REG33 */
	F_IINDPM_DIS, F_IINDPM,  /* REG34 */
	F_FORCE_ICO, F_ICO_EN, F_IINDPM_ICO,  /* REG35 */
	F_VBAT_PRECHG, F_IPRECHG,    /* REG36 */
	F_TERM_EN, F_ITERM,  /* REG37 */
	F_RECHG_DIS, F_RECHG_DG, F_VRECHG,    /* REG38 */
	F_VBOOST, F_IBOOST,  /* REG39 */
	F_CONV_OCP_DIS, F_TSBAT_JEITA_DIS, F_IBAT_OCP_DIS, F_VPMID_OVP_OTG_DIS, F_VBAT_OVP_BUCK_DIS,    /* REG3A */
	F_T_BATFET_RST, F_T_PD_nRST, F_BATFET_RST_EN, F_BATFET_DLY, F_BATFET_DIS, F_nRST_SHIPMODE_DIS,   /* REG3B */
	F_HIZ_EN, F_PERFORMANCE_EN, F_DIS_BUCKCHG_PATH, F_DIS_SLEEP_FOR_OTG, F_QB_EN, F_BOOST_EN, F_CHG_EN,   /* REG3C */
	F_VBAT_TRACK, F_IBATOCP, F_VSYSOVP_DIS, F_VSYSOVP_TH,  /* REG3D */
	F_BAT_COMP, F_VCLAMP, F_JEITA_ISET_COOL, F_JEITA_VSET_WARM,    /* REG3E */
	F_TMR2X_EN, F_CHG_TIMER_EN, F_CHG_TIMER, F_TDIE_REG_DIS, F_TDIE_REG, F_PFM_DIS,  /* REG3F */
	F_BAT_COMP_OFF, F_VBAT_LOW_OTG, F_BOOST_FREQ, F_BUCK_FREQ, F_BAT_LOAD_EN, /* REG40 */
	/**
	 * F_VSYS_SHORT_STAT, F_VSLEEP_BUCK_STAT, F_VBAT_DPL_STAT, F_VBAT_LOW_BOOST_STAT, F_VBUS_GOOD_STAT,
	 * F_CHG_STAT, F_BOOST_OK_STAT, F_VSYSMIN_REG_STAT, F_QB_ON_STAT, F_BATFET_STAT,
	 * F_TDIE_REG_STAT, F_TSBAT_COOL_STAT, F_TSBAT_WARM_STAT, F_ICO_STAT, F_IINDPM_STAT, F_VINDPM_STAT,
	 */
	F_JEITA_COOL_TEMP, F_JEITA_WARM_TEMP, F_BOOST_NTC_HOT_TEMP, F_BOOST_NTC_COLD_TEMP, /* REG56 */
	F_TESTM_EN, /* REG5D */
	F_KEY_EN_OWN,   /* REG5E */
	/****** led ********/
	F_TRPT, F_FL_TX_EN, F_TLED2_EN, F_TLED1_EN, F_FLED2_EN, F_FLED1_EN,  /* reg80 */
	F_FLED1_BR, /* reg81 */
	F_FLED2_BR,/* reg82 */
	F_FTIMEOUT, F_FRPT, F_FTIMEOUT_EN,/* reg83 */
	F_TLED1_BR,/* reg84 */
	F_TLED2_BR,/* reg85 */
	F_PMID_FLED_OVP_DEG, F_VBAT_MIN_FLED, F_VBAT_MIN_FLED_DEG, F_LED_POWER,/* reg86 */
	/****** DPDPM ******/
	F_FORCE_INDET, F_AUTO_INDET_EN, F_HVDCP_EN, F_QC_EN,
	F_DP_DRIV, F_DM_DRIV, F_BC1_2_VDAT_REF_SET, F_BC1_2_DP_DM_SINK_CAP,
	F_QC2_V_MAX, F_QC3_PULS, F_QC3_MINUS, F_QC3_5_16_PLUS, F_QC3_5_16_MINUS, F_QC3_5_3_SEQ, F_QC3_5_2_SEQ,
	F_I2C_DPDM_BYPASS_EN, F_DPDM_PULL_UP_EN, F_WDT_TFCP_MASK, F_WDT_TFCP_FLAG, F_WDT_TFCP_RST, F_WDT_TFCP_CFG, F_WDT_TFCP_DIS,
	F_VBUS_STAT, F_BC1_2_DONE, F_DP_OVP, F_DM_OVP,
	F_DM_500K_PD_EN, F_DP_500K_PD_EN, F_DM_SINK_EN, F_DP_SINK_EN, F_DP_SRC_10UA,
	F_MAX_FIELDS,
};

enum ADC_MODULE {
	ADC_IBUS = 0,
	ADC_VBUS,
	ADC_VAC,
	ADC_VBATSNS,
	ADC_VBAT,
	ADC_IBAT,
	ADC_VSYS,
	ADC_TSBUS,
	ADC_TSBAT,
	ADC_TDIE,
	ADC_BATID,
};

static const u32 adc_step[] = {
	2500, 3750, 5000, 1250, 1250,
	1220, 1250, 9766, 9766, 5, 156,
};

enum {
	SUBPMIC_CHG_STATE_NO_CHG = 0,
	SUBPMIC_CHG_STATE_TRICK,
	SUBPMIC_CHG_STATE_PRECHG,
	SUBPMIC_CHG_STATE_CC,
	SUBPMIC_CHG_STATE_CV,
	SUBPMIC_CHG_STATE_TERM,
};

enum DPDM_DRIVE {
	DPDM_HIZ = 0,
	DPDM_20K_DOWN,
	DPDM_V0_6,
	DPDM_V1_8,
	DPDM_V2_7,
	DPDM_V3_3,
	DPDM_500K_DOWN,
};

enum DPDM_CAP {
	DPDM_CAP_SNK_50UA = 0,
	DPDM_CAP_SNK_100UA,
	DPDM_CAP_SRC_10UA,
	DPDM_CAP_SRC_250UA,
	DPDM_CAP_DISABLE,
};

struct chip_state {
	bool online;
	bool boost_good;
	int vbus_type;
	int chg_state;
	int vindpm;
};

enum {
	IRQ_HK = 0,
	IRQ_BUCK,
	IRQ_DPDM,
	IRQ_LED,
	IRQ_MAX,
};

enum LED_FLASH_MODULE {
	LED1_FLASH = 0,
	LED2_FLASH,
	LED_ALL_FLASH,
};

// start -10
static uint32_t temp_var[] = {
	628988, 593342, 559930, 528601, 499212, 471632, 445771, 421479, 398652, 377192,
	357011, 338005, 320121, 303286, 287433, 272499, 258426, 245159, 232649, 220847,
	209709, 199196, 189268, 179889, 171027, 162650, 154726, 147232, 140142, 133432,
	127080, 121065, 115368, 109969, 104852, 100000, 95398, 91032, 86889, 82856,
	79221, 75675, 72306, 69104, 66060, 63167, 60415, 57796, 55305, 52934,
	50676, 48528, 46482, 44532, 42674, 40903, 39213, 37601, 36062, 34595,
	33194, 31859, 30583, 29366, 28202, 27090, 26028, 25012, 24041, 23112,
	22224, 21374, 20560, 19780, 19036, 18322, 17640, 16986, 16360, 15759,
	15184, 14631, 14100, 13591, 13103, 12635, 12187, 11756, 11343, 10946,
	10565, 10199, 9847, 9508, 9184, 8872, 8572, 8283, 8000, 7738, 7481,
	7234, 6997, 6768, 6548, 6336, 6131, 5934, 5743, 5560, 5383,
};
// end 100

struct subpmic_chg_device {
	struct i2c_adapter *adp;
	struct device *dev;
	struct regmap *rmap;
	struct regmap_field *rmap_fields[F_MAX_FIELDS];
	struct buck_init_data buck_init;
	struct chip_state state;
	struct delayed_work led_work;
	enum LED_FLASH_MODULE led_index;
	struct completion flash_run;
	struct completion flash_end;
	struct mutex led_lock;
	bool led_work_running;
	unsigned long request_otg;
	int irq[IRQ_MAX];
	struct delayed_work bc12_timeout_work;
	struct delayed_work bc12_retry_work;
	int bc12_retry_cnt;
	struct mutex bc_detect_lock;
	bool bc_detect;
	struct mutex adc_read_lock;
	int qc_result;
	int qc_vbus;
	int now_vindpm;
	int vbus_type;
	int bc12_retry_type_last;
	int online;
	int chg_plugin;
	int switch_vbus_gpio;
	int otg_boost_src;
	int otg_boost_en;
	int otg_en;
	unsigned int status;
	struct delayed_work dump_work;
	struct work_struct qc_detect_work;
	struct delayed_work irq_work;
	struct work_struct adapter_in_work;
	struct work_struct adapter_out_work;
	struct delayed_work usb_type_work;
	struct notifier_block subpmic_nb;
	struct notifier_block cp_mode_change_nb;
	bool bc12_det_done;
	int iindpm_val;

	struct xring_usb *xr_usb;
	/* quirk for udp */
	bool xr_quirk_otg_boost;
	bool xr_quirk_dis_vac_detect;
	bool xr_quirk_dis_bc12_detect;
	u32 *store_target_cycle;
	u32 *store_target_soc;
	u32 *store_target_over10t_powercap;
	u32 *store_target_0_10t_powercap;
	u32 *store_target_min10_0_powercap;
	u32 *store_target_min20_min10_powercap;
	u32 *store_target_min20_powercap;
	int cycle_length;
	int soc_length;
	int powercap_over10t_length;
	int powercap_0_10t_length;
	int powercap_min10_0_length;
	int powercap_min20_min10_length;
	int powercap_min20_length;;
	int g_target_soc;
	int g_target_cycle;
	int g_target_power;
	struct power_supply *batt_psy;
	bool support_bcl;
};

static int sc6601_get_otg_en_status(void *data, int *otg_status);
static int subpmic_chg_force_dpdm(struct subpmic_chg_device *sc);
static int sc6601_get_vbus_mos_gpio(struct subpmic_chg_device *sc);

static int subpmic_chg_field_read(struct subpmic_chg_device *sc,
	enum subpmic_chg_fields field_id)
{
	int ret;
	int val;

	ret = regmap_field_read(sc->rmap_fields[field_id], &val);
	if (ret < 0) {
		mca_log_err("i2c field read failed\n");
		return ret;
	}
	return val;
}

static int subpmic_chg_field_write(struct subpmic_chg_device *sc,
	enum subpmic_chg_fields field_id, u8 val)
{
	int ret = 0;

	ret = regmap_field_write(sc->rmap_fields[field_id], val);
	if (ret < 0)
		mca_log_err("i2c field write failed\n");

	return ret;
}

static int subpmic_chg_bulk_read(struct subpmic_chg_device *sc, u16 reg,
	u8 *val, size_t count)
{
	int ret = 0;

	ret = regmap_bulk_read(sc->rmap, reg, val, count);
	if (ret < 0)
		mca_log_err("i2c bulk read failed\n");

	return ret;
}

static int subpmic_chg_bulk_write(struct subpmic_chg_device *sc, u16 reg,
	u8 *val, size_t count)
{
	int ret = 0;

	ret = regmap_bulk_write(sc->rmap, reg, val, count);
	if (ret < 0)
		mca_log_err("i2c bulk write failed\n");

	return ret;
}

static int subpmic_chg_write_byte(struct subpmic_chg_device *sc, u16 reg, u8 val)
{
	u8 temp = val;

	return subpmic_chg_bulk_write(sc, reg, &temp, 1);
}

static int subpmic_chg_read_byte(struct subpmic_chg_device *sc, u16 reg, u8 *val)
{
	return subpmic_chg_bulk_read(sc, reg, val, 1);
}

static const struct reg_field subpmic_chg_reg_fields[] = {
	[F_VAC_OVP]          = REG_FIELD(SUBPMIC_REG_VAC_VBUS_OVP, 4, 7),
	[F_VBUS_OVP]         = REG_FIELD(SUBPMIC_REG_VAC_VBUS_OVP, 0, 2),
	[F_TSBUS_FLT]        = REG_FIELD(SUBPMIC_REG_VAC_VBUS_OVP, 0, 7),
	[F_TSBAT_FLT]        = REG_FIELD(SUBPMIC_REG_VAC_VBUS_OVP, 0, 7),
	[F_ACDRV_MANUAL_PRE] = REG_FIELD(SUBPMIC_REG_HK_CTRL, 7, 7),
	[F_ACDRV_EN]         = REG_FIELD(SUBPMIC_REG_HK_CTRL, 5, 5),
	[F_ACDRV_MANUAL_EN]  = REG_FIELD(SUBPMIC_REG_HK_CTRL, 4, 4),
	[F_WD_TIME_RST]      = REG_FIELD(SUBPMIC_REG_HK_CTRL, 3, 3),
	[F_WD_TIMER]         = REG_FIELD(SUBPMIC_REG_HK_CTRL, 0, 2),
	[F_REG_RST]          = REG_FIELD(SUBPMIC_REG_HK_CTRL + 1, 7, 7),
	[F_VBUS_PD]          = REG_FIELD(SUBPMIC_REG_HK_CTRL + 1, 6, 6),
	[F_VAC_PD]           = REG_FIELD(SUBPMIC_REG_HK_CTRL + 1, 5, 5),
	[F_CID_EN]           = REG_FIELD(SUBPMIC_REG_HK_CTRL + 1, 3, 3),
	[F_VAC_PD]           = REG_FIELD(SUBPMIC_REG_HK_CTRL2, 5, 5),
	[F_ADC_EN]           = REG_FIELD(SUBPMIC_REG_HK_ADC_CTRL, 7, 7),
	[F_ADC_FREEZE]       = REG_FIELD(SUBPMIC_REG_HK_ADC_CTRL, 5, 5),
	[F_BATID_ADC_EN]     = REG_FIELD(SUBPMIC_REG_HK_ADC_CTRL, 3, 3),
	[F_EDL_ACTIVE_LEVEL] = REG_FIELD(0x27, 1, 1),
	/* Charger */
	/* REG30 */
	[F_VSYS_MIN]        = REG_FIELD(SUBPMIC_REG_VSYS_MIN, 0, 2),
	/* REG31 */
	[F_BATSNS_EN]       = REG_FIELD(SUBPMIC_REG_VBAT, 7, 7),
	[F_VBAT]            = REG_FIELD(SUBPMIC_REG_VBAT, 0, 6),
	/* REG32 */
	[F_ICHG_CC]         = REG_FIELD(SUBPMIC_REG_ICHG_CC, 0, 6),
	/* REG33 */
	[F_VINDPM_VBAT]     = REG_FIELD(SUBPMIC_REG_VINDPM, 5, 6),
	[F_VINDPM_DIS]      = REG_FIELD(SUBPMIC_REG_VINDPM, 4, 4),
	[F_VINDPM]          = REG_FIELD(SUBPMIC_REG_VINDPM, 0, 3),
	/* REG34 */
	[F_IINDPM_DIS]      = REG_FIELD(SUBPMIC_REG_IINDPM, 7, 7),
	[F_IINDPM]          = REG_FIELD(SUBPMIC_REG_IINDPM, 0, 5),
	/* REG35 */
	[F_FORCE_ICO]       = REG_FIELD(SUBPMIC_REG_ICO_CTRL, 7, 7),
	[F_ICO_EN]          = REG_FIELD(SUBPMIC_REG_ICO_CTRL, 6, 6),
	[F_IINDPM_ICO]      = REG_FIELD(SUBPMIC_REG_ICO_CTRL, 0, 5),
	/* REG36 */
	[F_VBAT_PRECHG]     = REG_FIELD(SUBPMIC_REG_PRECHARGE_CTRL, 6, 7),
	[F_IPRECHG]         = REG_FIELD(SUBPMIC_REG_PRECHARGE_CTRL, 0, 3),
	/* REG37 */
	[F_TERM_EN]         = REG_FIELD(SUBPMIC_REG_TERMINATION_CTRL, 7, 7),
	[F_ITERM]           = REG_FIELD(SUBPMIC_REG_TERMINATION_CTRL, 0, 4),
	/* REG38 */
	[F_RECHG_DIS]       = REG_FIELD(SUBPMIC_REG_RECHARGE_CTRL, 4, 4),
	[F_RECHG_DG]        = REG_FIELD(SUBPMIC_REG_RECHARGE_CTRL, 2, 3),
	[F_VRECHG]          = REG_FIELD(SUBPMIC_REG_RECHARGE_CTRL, 0, 1),
	/* REG39 */
	[F_VBOOST]          = REG_FIELD(SUBPMIC_REG_VBOOST_CTRL, 3, 7),
	[F_IBOOST]          = REG_FIELD(SUBPMIC_REG_VBOOST_CTRL, 0, 2),
	/* REG3A */
	[F_CONV_OCP_DIS]    = REG_FIELD(SUBPMIC_REG_PROTECTION_DIS, 4, 4),
	[F_TSBAT_JEITA_DIS] = REG_FIELD(SUBPMIC_REG_PROTECTION_DIS, 3, 3),
	[F_IBAT_OCP_DIS]    = REG_FIELD(SUBPMIC_REG_PROTECTION_DIS, 2, 2),
	[F_VPMID_OVP_OTG_DIS] = REG_FIELD(SUBPMIC_REG_PROTECTION_DIS, 1, 1),
	[F_VBAT_OVP_BUCK_DIS] = REG_FIELD(SUBPMIC_REG_PROTECTION_DIS, 0, 0),
	/* REG3B */
	[F_T_BATFET_RST]    = REG_FIELD(SUBPMIC_REG_RESET_CTRL, 5, 5),
	[F_BATFET_RST_EN]   = REG_FIELD(SUBPMIC_REG_RESET_CTRL, 3, 3),
	[F_BATFET_DLY]      = REG_FIELD(SUBPMIC_REG_RESET_CTRL, 2, 2),
	[F_BATFET_DIS]      = REG_FIELD(SUBPMIC_REG_RESET_CTRL, 1, 1),
	/* REG3C */
	[F_HIZ_EN]          = REG_FIELD(SUBPMIC_REG_CHG_CTRL, 7, 7),
	[F_PERFORMANCE_EN]  = REG_FIELD(SUBPMIC_REG_CHG_CTRL, 6, 6),
	[F_DIS_BUCKCHG_PATH] = REG_FIELD(SUBPMIC_REG_CHG_CTRL, 5, 5),
	[F_DIS_SLEEP_FOR_OTG] = REG_FIELD(SUBPMIC_REG_CHG_CTRL, 4, 4),
	[F_QB_EN]           = REG_FIELD(SUBPMIC_REG_CHG_CTRL, 2, 2),
	[F_BOOST_EN]        = REG_FIELD(SUBPMIC_REG_CHG_CTRL, 1, 1),
	[F_CHG_EN]          = REG_FIELD(SUBPMIC_REG_CHG_CTRL, 0, 0),
	/* REG3D */
	[F_VBAT_TRACK]      = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 1, 5, 5),
	[F_IBATOCP]         = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 1, 4, 4),
	[F_VSYSOVP_DIS]     = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 1, 2, 2),
	[F_VSYSOVP_TH]      = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 1, 0, 1),
	/* REG3E */
	[F_BAT_COMP]        = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 2, 5, 7),
	[F_VCLAMP]          = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 2, 2, 4),
	[F_JEITA_ISET_COOL] = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 2, 1, 1),
	[F_JEITA_VSET_WARM] = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 2, 0, 0),
	/* REG3F */
	[F_TMR2X_EN]        = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 3, 7, 7),
	[F_CHG_TIMER_EN]    = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 3, 6, 6),
	[F_CHG_TIMER]       = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 3, 4, 5),
	[F_TDIE_REG_DIS]    = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 3, 3, 3),
	[F_TDIE_REG]        = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 3, 1, 2),
	[F_PFM_DIS]         = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 3, 0, 0),
	/* REG40 */
	[F_BAT_COMP_OFF]    = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 4, 6, 7),
	[F_VBAT_LOW_OTG]    = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 4, 5, 5),
	[F_BOOST_FREQ]      = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 4, 3, 4),
	[F_BUCK_FREQ]       = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 4, 1, 2),
	[F_BAT_LOAD_EN]     = REG_FIELD(SUBPMIC_REG_CHG_CTRL + 4, 0, 0),
	/* REG56 */
	[F_JEITA_COOL_TEMP] = REG_FIELD(SUBPMIC_REG_JEITA_TEMP, 6, 7),
	[F_JEITA_WARM_TEMP] = REG_FIELD(SUBPMIC_REG_JEITA_TEMP, 4, 5),
	[F_BOOST_NTC_HOT_TEMP]  = REG_FIELD(SUBPMIC_REG_JEITA_TEMP, 2, 3),
	[F_BOOST_NTC_COLD_TEMP] = REG_FIELD(SUBPMIC_REG_JEITA_TEMP, 0, 0),
	/* REG5D */
	[F_TESTM_EN]            = REG_FIELD(SUBPMIC_REG_Internal, 0, 0),
	/* REG5E */
	[F_KEY_EN_OWN]          = REG_FIELD(SUBPMIC_REG_Internal + 1, 0, 0),
	/*LED*/
	/*REG80*/
	[F_TRPT]         = REG_FIELD(SUBPMIC_REG_LED_CTRL, 0, 2),
	[F_FL_TX_EN]     = REG_FIELD(SUBPMIC_REG_LED_CTRL, 3, 3),
	[F_TLED2_EN]     = REG_FIELD(SUBPMIC_REG_LED_CTRL, 4, 4),
	[F_TLED1_EN]     = REG_FIELD(SUBPMIC_REG_LED_CTRL, 5, 5),
	[F_FLED2_EN]     = REG_FIELD(SUBPMIC_REG_LED_CTRL, 6, 6),
	[F_FLED1_EN]     = REG_FIELD(SUBPMIC_REG_LED_CTRL, 7, 7),
	[F_FLED1_BR]     = REG_FIELD(SUBPMIC_REG_FLED1_BR_CTR, 0, 6),
	[F_FLED2_BR]     = REG_FIELD(SUBPMIC_REG_FLED2_BR_CTR, 0, 6),
	[F_FTIMEOUT]     = REG_FIELD(SUBPMIC_REG_FLED_TIMER, 0, 3),
	[F_FRPT]         = REG_FIELD(SUBPMIC_REG_FLED_TIMER, 4, 6),
	[F_FTIMEOUT_EN]  = REG_FIELD(SUBPMIC_REG_FLED_TIMER, 7, 7),
	[F_TLED1_BR]     = REG_FIELD(SUBPMIC_REG_TLED1_BR_CTR, 0, 6),
	[F_TLED2_BR]     = REG_FIELD(SUBPMIC_REG_TLED2_BR_CTR, 0, 6),
	[F_PMID_FLED_OVP_DEG] = REG_FIELD(SUBPMIC_REG_LED_PRO, 0, 1),
	[F_VBAT_MIN_FLED]     = REG_FIELD(SUBPMIC_REG_LED_PRO, 2, 4),
	[F_VBAT_MIN_FLED_DEG] = REG_FIELD(SUBPMIC_REG_LED_PRO, 5, 6),
	[F_LED_POWER]         = REG_FIELD(SUBPMIC_REG_LED_PRO, 7, 7),
	/* DPDM */
	/* REG90 */
	[F_FORCE_INDET]     = REG_FIELD(SUBPMIC_REG_DPDM_EN, 7, 7),
	[F_AUTO_INDET_EN]   = REG_FIELD(SUBPMIC_REG_DPDM_EN, 6, 6),
	[F_HVDCP_EN]        = REG_FIELD(SUBPMIC_REG_DPDM_EN, 5, 5),
	[F_QC_EN]           = REG_FIELD(SUBPMIC_REG_DPDM_EN, 0, 0),
	/* REG91 */
	[F_DP_DRIV]         = REG_FIELD(SUBPMIC_REG_DPDM_CTRL, 5, 7),
	[F_DM_DRIV]         = REG_FIELD(SUBPMIC_REG_DPDM_CTRL, 2, 4),
	[F_BC1_2_VDAT_REF_SET] = REG_FIELD(SUBPMIC_REG_DPDM_CTRL, 1, 1),
	[F_BC1_2_DP_DM_SINK_CAP] = REG_FIELD(SUBPMIC_REG_DPDM_CTRL, 0, 0),
	/* REG92 */
	[F_QC2_V_MAX]       = REG_FIELD(SUBPMIC_REG_DPDM_QC_CTRL, 0, 1),
	[F_QC3_PULS]       = REG_FIELD(SUBPMIC_REG_DPDM_QC_CTRL, 2, 2),
	[F_QC3_MINUS]       = REG_FIELD(SUBPMIC_REG_DPDM_QC_CTRL, 3, 3),
	[F_QC3_5_16_PLUS]       = REG_FIELD(SUBPMIC_REG_DPDM_QC_CTRL, 4, 4),
	[F_QC3_5_16_MINUS]       = REG_FIELD(SUBPMIC_REG_DPDM_QC_CTRL, 5, 5),
	[F_QC3_5_3_SEQ]       = REG_FIELD(SUBPMIC_REG_DPDM_QC_CTRL, 6, 6),
	[F_QC3_5_2_SEQ]       = REG_FIELD(SUBPMIC_REG_DPDM_QC_CTRL, 7, 7),
	/* REG94 */
	[F_VBUS_STAT]       = REG_FIELD(SUBPMIC_REG_DPDM_INT_FLAG, 5, 7),
	[F_BC1_2_DONE]      = REG_FIELD(SUBPMIC_REG_DPDM_INT_FLAG, 2, 2),
	[F_DP_OVP]          = REG_FIELD(SUBPMIC_REG_DPDM_INT_FLAG, 1, 1),
	[F_DM_OVP]          = REG_FIELD(SUBPMIC_REG_DPDM_INT_FLAG, 0, 0),
	/* REG9D */
	[F_DM_500K_PD_EN]   = REG_FIELD(SUBPMIC_REG_DPDM_CTRL_2, 7, 7),
	[F_DP_500K_PD_EN]   = REG_FIELD(SUBPMIC_REG_DPDM_CTRL_2, 6, 6),
	[F_DM_SINK_EN]      = REG_FIELD(SUBPMIC_REG_DPDM_CTRL_2, 5, 5),
	[F_DP_SINK_EN]      = REG_FIELD(SUBPMIC_REG_DPDM_CTRL_2, 4, 4),
	[F_DP_SRC_10UA]     = REG_FIELD(SUBPMIC_REG_DPDM_CTRL_2, 3, 3),
};

__maybe_unused
static int subpmic_chg_dump_regs(struct subpmic_chg_device *sc, char *buf)
{
	int ret = 0, reg = 0;
	u8 val = 0;
	int count = 0;

	for (reg = SUBPMIC_REG_DEVICE_ID; reg < SUBPMIC_REG_MAX; reg++) {
		ret = subpmic_chg_read_byte(sc, reg, &val);
		if (ret < 0)
			return ret;
		if (buf)
			count += snprintf(buf + count, PAGE_SIZE - count,
							"[0x%x] -> 0x%x\n", reg, val);
		mca_log_info("[0x%x] -> 0x%x\n", reg, val);
	}

	return count;
}

/**
 * DPDM Module
 */
__maybe_unused
static int subpmic_chg_set_dp_drive(struct subpmic_chg_device *sc, enum DPDM_DRIVE state)
{
	switch (state) {
	case DPDM_500K_DOWN:
		subpmic_chg_field_write(sc, F_DP_DRIV, DPDM_HIZ);
		subpmic_chg_field_write(sc, F_DP_500K_PD_EN, true);
		break;
	default:
		subpmic_chg_field_write(sc, F_DP_DRIV, state);
		break;
	}

	return 0;
}

__maybe_unused
static int subpmic_chg_set_dm_drive(struct subpmic_chg_device *sc, enum DPDM_DRIVE state)
{
	subpmic_chg_write_byte(sc, SUBPMIC_REG_DPDM_INTERNAL + 1, 0x00);
	switch (state) {
	case DPDM_500K_DOWN:
		subpmic_chg_field_write(sc, F_DM_DRIV, 0);
		subpmic_chg_field_write(sc, F_DM_500K_PD_EN, 1);
		break;
	case DPDM_V1_8:
		subpmic_chg_write_byte(sc, SUBPMIC_REG_DPDM_INTERNAL + 2, 0x2a);
		subpmic_chg_write_byte(sc, SUBPMIC_REG_DPDM_INTERNAL + 1, 0x0a);
		break;
	default:
		subpmic_chg_field_write(sc, F_DM_DRIV, state);
		break;
	}

	return 0;
}

static int set_otg_en(struct subpmic_chg_device *sc, bool en)
{
	if (en) {
		gpio_set_value(sc->otg_boost_en, 1);
		gpio_set_value(sc->otg_en, 1);
	} else {
		gpio_set_value(sc->otg_boost_en, 0);
		gpio_set_value(sc->otg_en, 0);
	}

	return 0;
}

__maybe_unused
static int subpmic_chg_set_dp_cap(struct subpmic_chg_device *sc, enum DPDM_CAP cap)
{
	switch (cap) {
	case DPDM_CAP_SNK_50UA:
		subpmic_chg_field_write(sc, F_DP_SINK_EN, true);
		subpmic_chg_field_write(sc, F_BC1_2_DP_DM_SINK_CAP, false);
		break;
	case DPDM_CAP_SNK_100UA:
		subpmic_chg_field_write(sc, F_DP_SINK_EN, true);
		subpmic_chg_field_write(sc, F_BC1_2_DP_DM_SINK_CAP, true);
		break;
	case DPDM_CAP_SRC_10UA:
		subpmic_chg_field_write(sc, F_DP_SINK_EN, false);
		subpmic_chg_field_write(sc, F_DP_SRC_10UA, true);
		break;
	case DPDM_CAP_SRC_250UA:
		subpmic_chg_field_write(sc, F_DP_SINK_EN, false);
		subpmic_chg_field_write(sc, F_DP_SRC_10UA, false);
		break;
	default:
		subpmic_chg_field_write(sc, F_DP_SINK_EN, false);
		break;
	}

	return 0;
}

__maybe_unused
static int subpmic_chg_set_dm_cap(struct subpmic_chg_device *sc, enum DPDM_CAP cap)
{
	switch (cap) {
	case DPDM_CAP_SNK_50UA:
		subpmic_chg_field_write(sc, F_DM_SINK_EN, true);
		subpmic_chg_field_write(sc, F_BC1_2_DP_DM_SINK_CAP, false);
		break;
	case DPDM_CAP_SNK_100UA:
		subpmic_chg_field_write(sc, F_DM_SINK_EN, true);
		subpmic_chg_field_write(sc, F_BC1_2_DP_DM_SINK_CAP, true);
		break;
	default:
		subpmic_chg_field_write(sc, F_DM_SINK_EN, false);
		break;
	}

	return 0;
}

__maybe_unused
static int subpmic_chg_auto_dpdm_enable(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_AUTO_INDET_EN, en);
}

/**
 * BUCK Module
 */
__maybe_unused
static int subpmic_chg_set_ico_enable(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_ICO_EN, en);
}

__maybe_unused
static int __subpmic_chg_get_chg_status(struct subpmic_chg_device *sc)
{
	int ret;
	u8 val;

	ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_CHG_INT_STAT + 1, &val);
	if (ret < 0)
		return ret;
	val >>= 5;
	val &= 0x7;

	return val;
}

__maybe_unused
static int subpmic_chg_set_vac_pd(struct subpmic_chg_device *sc, bool en)
{
	int ret = 0;

	if (en)
		ret = subpmic_chg_field_write(sc, F_VAC_PD, true);
	else
		ret = subpmic_chg_field_write(sc, F_VAC_PD, false);

	if (ret < 0)
		mca_log_err("set SUBPMIC_REG_HK_CTRL2 fail");

	return ret;
}

__maybe_unused
static int subpmic_chg_mask_buck_irq(struct subpmic_chg_device *sc, int irq_channel)
{
	int ret;
	u8 val[3] = {0};

	ret = subpmic_chg_bulk_read(sc, SUBPMIC_REG_CHG_INT_MASK, val, 3);
	if (ret < 0)
		return ret;

	val[0] |= irq_channel;
	val[1] |= irq_channel >> 8;
	val[2] |= irq_channel >> 16;

	return subpmic_chg_bulk_write(sc, SUBPMIC_REG_CHG_INT_MASK, val, 3);
}

__maybe_unused
static int subpmic_chg_unmask_buck_irq(struct subpmic_chg_device *sc, int irq_channel)
{
	int ret;
	u8 val[3] = {0};

	ret = subpmic_chg_bulk_read(sc, SUBPMIC_REG_CHG_INT_MASK, val, 3);
	if (ret < 0)
		return ret;

	val[0] &= ~(irq_channel);
	val[1] &= ~(irq_channel >> 8);
	val[2] &= ~(irq_channel >> 16);

	return subpmic_chg_bulk_write(sc, SUBPMIC_REG_CHG_INT_MASK, val, 3);
}

__maybe_unused
static int subpmic_chg_set_sys_volt(struct subpmic_chg_device *sc, int mv)
{
	int i = 0;

	if (mv < vsys_min[0])
		mv = vsys_min[0];
	if (mv > vsys_min[ARRAY_SIZE(vsys_min) - 1])
		mv = vsys_min[ARRAY_SIZE(vsys_min) - 1];
	for (i = 0; i < ARRAY_SIZE(vsys_min); i++) {
		if (mv <= vsys_min[i])
			break;
	}

	return subpmic_chg_field_write(sc, F_VSYS_MIN, i);
}

/***************************************************************************/
/**
 * Hourse Keeping Module
 */
__maybe_unused
static int subpmic_chg_set_adc_func(struct subpmic_chg_device *sc, int adc_channel, bool en)
{
	int ret;
	u8 val[2] = {0};

	ret = subpmic_chg_bulk_read(sc, SUBPMIC_REG_HK_ADC_CTRL, val, 2);
	if (ret < 0)
		return ret;
	val[0] = en ? val[0] | (adc_channel >> 8) : val[0] & ~(adc_channel >> 8);
	val[1] = en ? val[1] | adc_channel : val[1] & ~adc_channel;

	return subpmic_chg_bulk_write(sc, SUBPMIC_REG_HK_ADC_CTRL, val, 2);
}

__maybe_unused
static int subpmic_chg_get_adc(struct subpmic_chg_device *sc,
	enum ADC_MODULE id)
{
	u32 reg = SUBPMIC_REG_HK_IBUS_ADC + id * 2;
	u8 val[2] = {0};
	int ret = 0;

	ret = subpmic_chg_field_read(sc, F_ADC_EN);
	if (ret <= 0)
		return ret;
	if (id == ADC_BATID) {
		subpmic_chg_field_write(sc, F_BATID_ADC_EN, true);
		mdelay(100);
	}

	mutex_lock(&sc->adc_read_lock);
	subpmic_chg_field_write(sc, F_ADC_FREEZE, true);
	ret = subpmic_chg_bulk_read(sc, reg, val, sizeof(val));
	if (ret < 0) {
		goto OUT;
	}

	ret = val[1] + (val[0] << 8);
	if (id == ADC_TDIE) {
		ret = (440 - ret) / 2;
	} else if (id == ADC_BATID) {
		ret = 16233 * ret / 100000;
	} else if (id == ADC_TSBUS) {
		// get percentage
		ret = ret * adc_step[id] / 200000;
		ret = 100 * ret / (100 - ret) * 1000;
	} else if (id == ADC_TSBAT) {
		ret = ret * adc_step[id] / 100000;
		ret = 100 * ret / (100 - ret) * 1000;
	} else {
		ret *= adc_step[id];
	}

OUT:
	subpmic_chg_field_write(sc, F_ADC_FREEZE, false);
	mutex_unlock(&sc->adc_read_lock);
	return ret;
}

__maybe_unused
static int subpmic_chg_chip_reset(struct subpmic_chg_device *sc)
{
	return subpmic_chg_field_write(sc, F_REG_RST, true);
}

__maybe_unused
static int subpmic_chg_set_acdrv(struct subpmic_chg_device *sc, bool en)
{
	int ret;
	int cnt = 0;
	int from_ic;

	from_ic = subpmic_chg_field_read(sc, F_ACDRV_EN);
	do {
		if (cnt++ > 3) {
			mca_log_err("[ERR]set acdrv failed\n");
			return -EIO;
		}

		ret = subpmic_chg_field_write(sc, F_ACDRV_EN, en);
		if (ret < 0)
			continue;

		from_ic = subpmic_chg_field_read(sc, F_ACDRV_EN);
		if (from_ic < 0)
			continue;
	} while (en != from_ic);

	mca_log_info("acdrv set %d success\n", (int)en);
	return 0;
}

__maybe_unused
static int subpmic_chg_set_vac_ovp(struct subpmic_chg_device *sc, int mv)
{
	if (mv <= 6500)
		mv = 0;
	else if (mv <= 11000)
		mv = 1;
	else if (mv <= 12000)
		mv = 2;
	else if (mv <= 13000)
		mv = 3;
	else if (mv <= 14000)
		mv = 4;
	else if (mv <= 15000)
		mv = 5;
	else if (mv <= 16000)
		mv = 6;
	else
		mv = 7;
	return subpmic_chg_field_write(sc, F_VAC_OVP, mv);
}

__maybe_unused
static int subpmic_chg_set_vbus_ovp(struct subpmic_chg_device *sc, int mv)
{
	if (mv <= 6000)
		mv = 0;
	else if (mv <= 10000)
		mv = 1;
	else if (mv <= 12000)
		mv = 2;
	else
		mv = 3;
	return subpmic_chg_field_write(sc, F_VBUS_OVP, mv);
}

__maybe_unused
static int subpmic_chg_mask_hk_irq(struct subpmic_chg_device *sc, int irq_channel)
{
	u8 val = 0;

	subpmic_chg_read_byte(sc, SUBPMIC_REG_HK_INT_MASK, &val);
	val |= irq_channel;
	return subpmic_chg_write_byte(sc, SUBPMIC_REG_HK_INT_MASK, val);
}

__maybe_unused
static int subpmic_chg_unmask_hk_irq(struct subpmic_chg_device *sc, int irq_channel)
{
	u8 val = 0;

	subpmic_chg_read_byte(sc, SUBPMIC_REG_HK_INT_MASK, &val);
	val &= ~irq_channel;
	return subpmic_chg_write_byte(sc, SUBPMIC_REG_HK_INT_MASK, val);
}

/***************************************************************************/
/**
 * QC Module
 */
enum QC_STATUS {
	QC_NONE = 0,
	QC2_MODE,
	QC3_MODE,
	QC3_5_18W_MODE,
	QC3_5_27W_MODE,
	QC3_5_45W_MODE,
};

static int sc6601a_batfet_rst_en(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_BATFET_RST_EN, en);
}

static int subpmic_chg_hvdcp_detect(struct subpmic_chg_device *sc)
{
	int ret = 0;
	uint8_t dm = 0;

	ret = subpmic_chg_write_byte(sc, SUBPMIC_REG_DPDM_INTERNAL, 0xa0);
	if (ret < 0)
		return ret;

	udelay(1000);

	ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_DM_STAT, &dm);
	if (dm != 0x00)
		return -1;

	return 0;
}

static int subpmic_chg_get_online(struct subpmic_chg_device *sc, int *en);
static void qc_detect_workfunc(struct work_struct *work)
{
	struct subpmic_chg_device *sc = container_of(work,
				struct subpmic_chg_device, qc_detect_work);
	int vbus = 0;
	int cnt = 0;
	int vbus_online = false;

	mca_log_info("start\n");
	sc->qc_result = QC_NONE;
	cnt = 0;
	subpmic_chg_field_write(sc, F_QC2_V_MAX, 3);
	subpmic_chg_field_write(sc, F_QC_EN, true);

	do {
		msleep(200);
		subpmic_chg_get_online(sc, &vbus_online);
		if (!vbus_online)
			break;
		if (subpmic_chg_hvdcp_detect(sc) == 0)
			break;
	} while (cnt++ < 10);

	if (!vbus_online) {
		subpmic_chg_field_write(sc, F_QC_EN, false);
		goto out;
	}

	if (subpmic_chg_hvdcp_detect(sc) < 0) {
		subpmic_chg_field_write(sc, F_QC_EN, false);
		goto out;
	}
	msleep(50);
	sc->qc_result = QC2_MODE;
	goto out;

	subpmic_chg_field_write(sc, F_QC2_V_MAX, 2);
	// qc3 detect need delay 40-60ms
	msleep(120);
	// send 16 sequence
	subpmic_chg_field_write(sc, F_QC3_5_16_PLUS, true);
	msleep(130);
	vbus = subpmic_chg_get_adc(sc, ADC_VBUS) / 1000;
	if (vbus < 7500) {
		sc->qc_result = QC2_MODE;
		mca_log_info("QC2_MODE\n");
		goto out;
	}
	msleep(50);
	// send 16 sequence
	subpmic_chg_field_write(sc, F_QC3_5_16_MINUS, true);
	msleep(280);
	vbus = subpmic_chg_get_adc(sc, ADC_VBUS) / 1000;
	if (vbus < 5500) {
		sc->qc_result = QC3_MODE;
		mca_log_info("QC3_MODE\n");
		goto out;
	}
	// send 3 sequence
	subpmic_chg_field_write(sc, F_QC3_5_3_SEQ, true);
	msleep(100);
	vbus = subpmic_chg_get_adc(sc, ADC_VBUS) / 1000;
	if (vbus < 7500) {
		sc->qc_result = QC3_5_18W_MODE;
		mca_log_info("adapter support 18w\n");
	} else if (vbus < 8500) {
		sc->qc_result = QC3_5_27W_MODE;
		mca_log_info("adapter support 27w\n");
	} else if (vbus < 9500) {
		sc->qc_result = QC3_5_45W_MODE;
		mca_log_info("adapter support 45w\n");
	}
	// send 2 sequence
	subpmic_chg_field_write(sc, F_QC3_5_2_SEQ, true);
	msleep(60);
out:
	// set qc volt to qc2 5v , clear plus state
	subpmic_chg_write_byte(sc, SUBPMIC_REG_DPDM_QC_CTRL, 0x03);
	msleep(100);
	switch (sc->qc_result) {
	case QC2_MODE:
		sc->state.vbus_type = HVDCP_DETECED;
		break;
	case QC3_MODE:
		sc->state.vbus_type = QC3_DETECED;
		break;
	case QC3_5_18W_MODE:
	case QC3_5_27W_MODE:
	case QC3_5_45W_MODE:
		sc->state.vbus_type = QC3P5_DETECED;
		break;
	default:
		break;
	}

#if 0
	if (sc->qc_result != QC_NONE) {
		mca_log_info("force set qc2, qc_result = %d\n", sc->qc_result);
		subpmic_chg_field_write(sc, F_QC2_V_MAX, 0);
	}
#endif
	//charger_changed(sc->sc_charger);
	subpmic_dev_notify(SC6601_SUBPMIC_EVENT_DPDM_ALERT, true);
	mca_log_info("end\n");
}

// bringup adb port
__maybe_unused
static void usb_queue_device_on(struct subpmic_chg_device *sc)
{
	int ret = 0;
	int cc_orientation = 0;
	bool if_support = false;

	protocol_class_pd_get_usb_communication_support(TYPEC_PORT_0, &if_support);
	if (if_support || sc->vbus_type == XM_CHARGER_TYPE_SDP || sc->vbus_type == XM_CHARGER_TYPE_CDP) {
		ret = protocol_class_pd_get_typec_cc_orientation(TYPEC_PORT_0, &cc_orientation);
		if (ret)
			mca_log_err("failed to get cc orientation\n");
		if (cc_orientation != 0) {
			cc_orientation--;
			mca_log_info("cc_orientation = %d\n", cc_orientation);
		}
		ret = xr_usb_queue_device_on(sc->xr_usb, cc_orientation,
						USB_MODE, DP_HPD_EVENT_PLUG_OUT);
		if (ret)
			mca_log_err("failed to queue device on\n");
		else
			mca_log_err("queue device on\n");
	}
}

__maybe_unused
static void bc12_retry_workfunc(struct work_struct *work)
{
	struct subpmic_chg_device *sc = container_of(work,
			struct subpmic_chg_device, bc12_retry_work.work);
	int ret = 0;
	int pd_active = 0;

	/* retry work is scheduled seconds ago. if pd_active when get in bc12 retry work, quit work */
	protocol_class_pd_get_pd_active(TYPEC_PORT_0, &pd_active);
	if(pd_active)
		return;

	if(sc->bc12_retry_cnt && sc->bc12_retry_type_last == sc->vbus_type) {
		mca_log_info("bc12 retry. cnt = %d, type [%d %d]\n",
			sc->bc12_retry_cnt, sc->bc12_retry_type_last, sc->vbus_type);
		ret = subpmic_chg_force_dpdm(sc);
		if(ret) {
			mca_log_err("force dpdm failed\n");
		}
		mca_queue_delayed_work(&sc->bc12_retry_work, msecs_to_jiffies(30 * 1000));
	} else {
		mca_log_info("bc12 retry Done, cnt = %d, type [%d %d]\n",
			sc->bc12_retry_cnt, sc->bc12_retry_type_last, sc->vbus_type);
	}

	sc->bc12_retry_cnt--;
	sc->bc12_retry_cnt = sc->bc12_retry_cnt < 0 ? 0 : sc->bc12_retry_cnt;
}

__maybe_unused
static int subpmic_chg_qc_identify(struct subpmic_chg_device *sc)
{
	if (work_busy(&sc->qc_detect_work)) {
		mca_log_err("qc_detect work running\n");
		return -EBUSY;
	}
	sc->qc_vbus = 5000;
	mca_queue_work(&sc->qc_detect_work);
	return 0;
}

__maybe_unused
static int subpmic_chg_request_vbus(struct subpmic_chg_device *sc, int mv, int step)
{
	int count = 0, i;
	int ret = 0;

	count = (mv - sc->qc_vbus) / step;
	for (i = 0; i < abs(count); i++) {
		if (count > 0)
			ret |= subpmic_chg_field_write(sc, F_QC3_PULS, true);
		else
			ret |= subpmic_chg_field_write(sc, F_QC3_MINUS, true);
		mdelay(8);
	}
	if (ret >= 0)
		sc->qc_vbus = mv;
	return ret;
}

static int sc6601_get_vbus_mos_gpio(struct subpmic_chg_device *sc)
{
	return gpio_get_value(sc->switch_vbus_gpio);
}

static int sc6601_ctrl_vbus_path(struct subpmic_chg_device *sc, bool plug)
{
	int ret = 0;
	int g_val = 0;

	g_val = gpio_get_value(sc->switch_vbus_gpio);
	if (g_val == plug)
		return ret;

	ret = gpio_direction_output(sc->switch_vbus_gpio, plug);
	if (ret < 0) {
		mca_log_err("pull gpio failed\n");
		return ret;
	}

	g_val = gpio_get_value(sc->switch_vbus_gpio);
	mca_log_info("pull gpio success, get gpio value = %d, plug = %d\n", g_val, plug);
	return ret;
}

__maybe_unused
static int subpmic_chg_get_online(struct subpmic_chg_device *sc, int *en)
{
	bool usb_present;
	int en_temp = 0;
	int wls_online = 0, wls_otg_boost_en = 0;
	int pd_active = 0;

	(void)platform_class_cp_get_int_stat(CP_ROLE_MASTER, VUSB_PRESENT_STAT, &usb_present);
	platform_class_wireless_is_present(WIRELESS_ROLE_MASTER, &wls_online);
	protocol_class_pd_get_pd_active(TYPEC_PORT_0, &pd_active);
	sc6601_get_otg_en_status(sc, &wls_otg_boost_en);
	if (wls_otg_boost_en)
		return 0;

	en_temp = sc->state.online | usb_present;

	/* if buck vbus offline when we havn't cutoff buck vbus by pullup ovp mos gpio*/
	if(RESUME_BUCK_VBUS_GPIO_VAL == sc6601_get_vbus_mos_gpio(sc) && !sc->state.online && usb_present && !pd_active) {
		en_temp = 0;
	}

	*en = en_temp;
	if (!(sc->state.online | usb_present | wls_online))
		sc6601_ctrl_vbus_path(sc, RESUME_BUCK_VBUS_GPIO_VAL);
	mca_log_err("buck vbus online = %d, cp vbus online = %d, vbus online = %d, wls online = %d, pd_active = %d\n",
				sc->state.online, usb_present, *en, wls_online, pd_active);
	return 0;
}

__maybe_unused
static int subpmic_chg_get_vbus_type(struct subpmic_chg_device *sc, int *vbus_type)
{
	*vbus_type = sc->state.vbus_type;
	mca_log_info("vbus type = %d\n", *vbus_type);
	return 0;
}

__maybe_unused
static int subpmic_chg_is_charge_done(struct subpmic_chg_device *sc, bool *en)
{
	*en = __subpmic_chg_get_chg_status(sc) == SUBPMIC_CHG_STATE_TERM;
	return 0;
}

__maybe_unused
static int subpmic_chg_get_hiz_status(struct subpmic_chg_device *sc, int *en)
{
	int ret = 0;

	ret = subpmic_chg_field_read(sc, F_HIZ_EN);
	if (ret < 0)
		return ret;
	*en = ret;
	return 0;
}

__maybe_unused
static int subpmic_chg_get_input_volt_lmt(struct subpmic_chg_device *sc, uint32_t *mv)
{
	*mv = sc->state.vindpm;
	return 0;
}

__maybe_unused
static int subpmic_chg_get_input_curr_lmt(struct subpmic_chg_device *sc, uint32_t *ma)
{
	int ret;

	ret = subpmic_chg_field_read(sc, F_PERFORMANCE_EN);
	if (ret < 0)
		return ret;
	if (ret) {
		*ma = 0;
		return 0;
	}
	ret = subpmic_chg_field_read(sc, F_IINDPM);
	if (ret < 0)
		return ret;
	*ma = SUBPMIC_BUCK_IINDPM_STEP * ret + SUBPMIC_BUCK_IINDPM_OFFSET;
	return 0;
}

/* TODO MCA_QUICK_CHG_STS_CHARGING */
__maybe_unused
static int subpmic_chg_get_chg_status(struct subpmic_chg_device *sc,
	uint32_t *chg_status)
{
	int state = 0;

	state = __subpmic_chg_get_chg_status(sc);
	switch (state) {
	case SUBPMIC_CHG_STATE_CC:
		*chg_status = MCA_BATT_CHGR_STATUS_FULLON;
		break;
	case SUBPMIC_CHG_STATE_CV:
		*chg_status = MCA_BATT_CHGR_STATUS_TAPER;
		break;
	case SUBPMIC_CHG_STATE_PRECHG:
		*chg_status = MCA_BATT_CHGR_STATUS_PRECHARGE;
		break;
	case SUBPMIC_CHG_STATE_TRICK:
		*chg_status = MCA_BATT_CHGR_STATUS_TRICKLE;
		break;
	case SUBPMIC_CHG_STATE_TERM:
		*chg_status = MCA_BATT_CHGR_STATUS_TERMINATION;
		break;
	case SUBPMIC_CHG_STATE_NO_CHG:
		*chg_status = MCA_BATT_CHGR_STATUS_CHARGING_DISABLED;
		break;
	default:
		*chg_status = MCA_BATT_CHGR_STATUS_INVALID;
	}
	return 0;
}

__maybe_unused
static int subpmic_chg_get_chg_type(struct subpmic_chg_device *sc,
	uint32_t *chg_type)
{
	int state = 0;

	state = __subpmic_chg_get_chg_status(sc);
	switch (state) {
	case SUBPMIC_CHG_STATE_CC:
		*chg_type = POWER_SUPPLY_CHARGE_TYPE_FAST;
		break;
	case SUBPMIC_CHG_STATE_CV:
		*chg_type = POWER_SUPPLY_CHARGE_TYPE_TAPER_EXT;
		break;
	case SUBPMIC_CHG_STATE_PRECHG:
	case SUBPMIC_CHG_STATE_TRICK:
		*chg_type = POWER_SUPPLY_CHARGE_TYPE_TRICKLE;
		break;
	case SUBPMIC_CHG_STATE_TERM:
	case SUBPMIC_CHG_STATE_NO_CHG:
		*chg_type = POWER_SUPPLY_CHARGE_TYPE_NONE;
		break;
	default:
		*chg_type = POWER_SUPPLY_CHARGE_TYPE_UNKNOWN;
	}
	return 0;
}

__maybe_unused
static int subpmic_chg_get_otg_status(struct subpmic_chg_device *sc, bool *en)
{
	int ret = 0;

	ret = subpmic_chg_field_read(sc, F_BOOST_EN);
	if (ret < 0)
		return ret;
	*en = ret;
	return 0;
}

__maybe_unused
static int subpmic_chg_get_term_curr(struct subpmic_chg_device *sc, uint32_t *ma)
{
	int ret = 0;

	ret = subpmic_chg_field_read(sc, F_ITERM);
	if (ret < 0)
		return ret;
	*ma = ret * SUBPMIC_BUCK_ITERM_STEP + SUBPMIC_BUCK_ITERM_OFFSET;
	return ret;
}

__maybe_unused
static int subpmic_chg_get_term_volt(struct subpmic_chg_device *sc, uint32_t *mv)
{
	int ret = 0;

	ret = subpmic_chg_field_read(sc, F_VBAT);
	if (ret < 0)
		return ret;
	*mv = ret * SUBPMIC_BUCK_VBAT_STEP + SUBPMIC_BUCK_VBAT_OFFSET;

	return 0;
}

__maybe_unused
static int subpmic_chg_set_hiz(struct subpmic_chg_device *sc, bool en)
{
	mca_log_info("%s:set buck hiz = %d\n", __func__, en);
	return subpmic_chg_field_write(sc, F_HIZ_EN, en);
}

__maybe_unused
static int subpmic_chg_set_input_curr_lmt(struct subpmic_chg_device *sc, int ma)
{
	sc->iindpm_val = ma;
	if (ma < SUBPMIC_BUCK_IINDPM_MIN)
		ma = SUBPMIC_BUCK_IINDPM_MIN;
	if (ma > SUBPMIC_BUCK_IINDPM_MAX)
		ma = SUBPMIC_BUCK_IINDPM_MAX;
	ma = (ma - SUBPMIC_BUCK_IINDPM_OFFSET) / SUBPMIC_BUCK_IINDPM_STEP;

	mca_log_info("%s:set buck max icl = %d\n", __func__, ma);
	return subpmic_chg_field_write(sc, F_IINDPM, ma);
}

__maybe_unused
static int subpmic_chg_set_input_volt_lmt(struct subpmic_chg_device *sc, int mv)
{
	int i = 0, ret;

	mca_log_info("vbus mv = %d\n", mv);

	if (mv > 0)
		sc->now_vindpm = mv;

	mutex_lock(&sc->bc_detect_lock);
	if (sc->bc_detect)
		mv = SUBPMIC_BC_DETECT_VINDPM;
	else
		mv = sc->now_vindpm;

	mutex_unlock(&sc->bc_detect_lock);

	if (mv < vindpm[0])
		mv = vindpm[0];
	if (mv > vindpm[ARRAY_SIZE(vindpm) - 1])
		mv = vindpm[ARRAY_SIZE(vindpm) - 1];

	for (i = 0; i < ARRAY_SIZE(vindpm); i++) {
		if (mv <= vindpm[i])
			break;
	}
	ret = subpmic_chg_field_write(sc, F_VINDPM, i);
	if (ret < 0)
		return ret;

	sc->state.vindpm = vindpm[i];
	return 0;
}

__maybe_unused
static int subpmic_chg_set_ichg(struct subpmic_chg_device *sc, int ma)
{
	if (ma <= SUBPMIC_BUCK_ICHG_MIN)
		ma = SUBPMIC_BUCK_ICHG_MIN;
	else if (ma >= SUBPMIC_BUCK_ICHG_MAX)
		ma = SUBPMIC_BUCK_ICHG_MAX;

	ma = (ma - SUBPMIC_BUCK_ICHG_OFFSET) / SUBPMIC_BUCK_ICHG_STEP;
	return subpmic_chg_field_write(sc, F_ICHG_CC, ma);
}

__maybe_unused
static int subpmic_chg_get_ichg(struct subpmic_chg_device *sc, int *ma)
{
	int ret = 0;

	ret = subpmic_chg_field_read(sc, F_ICHG_CC);
	if (ret < 0)
		return ret;

	*ma = ret * SUBPMIC_BUCK_ICHG_STEP + SUBPMIC_BUCK_ICHG_OFFSET;
	return ret;
}

__maybe_unused
static int subpmic_chg_set_chg(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_CHG_EN, en);
}

static int subpmic_chg_set_wd_timeout(struct subpmic_chg_device *sc, int ms);

__maybe_unused
static int subpmic_chg_set_otg(struct subpmic_chg_device *sc, bool en)
{
	int ret;
	int cnt = 0;
	u8 boost_state;

	ret = subpmic_chg_set_chg(sc, !en);
	if (ret < 0)
		return ret;

	do {
		boost_state = 0;
		ret = subpmic_chg_field_write(sc, F_BOOST_EN, en ? true : false);
		if (ret < 0) {
			subpmic_chg_set_chg(sc, true);
			return ret;
		}
		mdelay(30);
		ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_CHG_INT_STAT + 1, &boost_state);
		if (ret < 0) {
			subpmic_chg_set_chg(sc, true);
			return ret;
		}
		if (cnt++ > 3) {
			subpmic_chg_set_chg(sc, true);
			return -EIO;
		}
	} while (en != (!!(boost_state & BIT(4))));
	mca_log_info("otg set success\n");
	return 0;
}

enum {
	SUBPMIC_NORMAL_USE_OTG = 0,
	SUBPMIC_LED_USE_OTG,
};

static int sc6601_set_otg_quirk(void *data, bool en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	int ret;
	int cnt = 0;
	u8 boost_state;
	u8 reg = 0;

	ret = subpmic_chg_field_write(sc, F_BATSNS_EN, en ? false : sc->buck_init.batsns_en);
	if (ret < 0)
		return ret;

	ret = subpmic_chg_read_byte(sc, SC6601_REG_HK_CTRL2, &reg);
	if (ret < 0) {
		mca_log_err("read SC6601_REG_HK_CTRL2 fail");
		subpmic_chg_set_chg(sc, true);
		return ret;
	}

	if (en) {
		reg |= HK_CTRL2_TSBUS_TSBAT_FLT_DIS | HK_CTRL2_TSHUT_DIS |
			HK_CTRL2_CID_EN;
		reg &= ~(HK_CTRL2_PMID_PD_EN | HK_CTRL2_VBUS_PD);
	} else {
		reg |= HK_CTRL2_PMID_PD_EN | HK_CTRL2_VBUS_PD;
		reg &= ~(HK_CTRL2_TSBUS_TSBAT_FLT_DIS | HK_CTRL2_TSHUT_DIS |
			HK_CTRL2_CID_EN);
	}
	ret = subpmic_chg_write_byte(sc, SC6601_REG_HK_CTRL2, reg);
	if (ret < 0) {
		mca_log_err("set SC6601_REG_HK_CTRL2 fail");
		subpmic_chg_set_chg(sc, true);
		return ret;
	}
	ret = subpmic_chg_field_write(sc, F_TSBAT_JEITA_DIS, en ? true : false);
	if (ret < 0) {
		subpmic_chg_set_chg(sc, true);
		return ret;
	}

	ret = subpmic_chg_set_chg(sc, !en);
	if (ret < 0)
		return ret;

	if (!en) {
		mca_log_info("otg clear success\n");
		return subpmic_chg_field_write(sc, F_QB_EN, false);
	}
	usleep_range(1000, 2000);
	do {
		boost_state = 0;
		ret = subpmic_chg_field_write(sc, F_BOOST_EN, true);
		if (ret < 0) {
			subpmic_chg_set_chg(sc, true);
			return ret;
		}
		mdelay(30);
		ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_CHG_INT_STAT + 1, &boost_state);
		if (cnt++ > 100) {
			mca_log_err("otg set fail\n");
			ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_CHG_CTRL, &reg);
			if (ret < 0)
				return ret;
			mca_log_info("read SC6601_REG_CHG_CTRL %02x\n", reg);
			subpmic_chg_set_chg(sc, true);
			return -EIO;
		}
	} while (en != (!!(boost_state & BIT(4))));

	ret = subpmic_chg_field_write(sc, F_QB_EN, true);
	if (ret < 0)
		return ret;

	mca_log_info("otg set success\n");

	return 0;
}

static int subpmic_chg_request_otg(struct subpmic_chg_device *sc, int index, bool en)
{
	int ret = 0;

	if (en && test_bit(index, &sc->request_otg)) {
		mca_log_err("already enable request_otg = %lx\n", sc->request_otg);
		return 0;
	}

	if (en)
		set_bit(index, &sc->request_otg);
	else
		clear_bit(index, &sc->request_otg);

	//mca_log_info("now request_otg = %lx\n", sc->request_otg);
	mca_log_err("now request_otg = %lx\n", sc->request_otg);
	if (index != SUBPMIC_LED_USE_OTG && en)
		ret = subpmic_chg_field_write(sc, F_QB_EN, true);
	else if (index != SUBPMIC_LED_USE_OTG && !en)
		ret = subpmic_chg_field_write(sc, F_QB_EN, false);

	if (!en && sc->request_otg)
		return 0;

	if (likely(!sc->xr_quirk_otg_boost))
		ret = subpmic_chg_set_otg(sc, en);
	else
		ret = sc6601_set_otg_quirk(sc, en);

	if (ret < 0) {
		if (en)
			clear_bit(index, &sc->request_otg);
		else
			set_bit(index, &sc->request_otg);
		return ret;
	}

	return 0;
}

__maybe_unused
static int subpmic_chg_normal_request_otg(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_request_otg(sc, SUBPMIC_NORMAL_USE_OTG, en);
}

__maybe_unused
static int subpmic_chg_set_otg_curr(struct subpmic_chg_device *sc, int ma)
{
	int i = 0;

	if (ma < boost_curr[0])
		ma = boost_curr[0];
	if (ma > boost_curr[ARRAY_SIZE(boost_curr) - 1])
		ma = boost_curr[ARRAY_SIZE(boost_curr) - 1];
	for (i = 0; i < ARRAY_SIZE(boost_curr); i++) {
		if (ma < boost_curr[i])
			break;
	}

	return subpmic_chg_field_write(sc, F_IBOOST, i);
}

__maybe_unused
static int subpmic_chg_set_otg_volt(struct subpmic_chg_device *sc, int mv)
{
	if (mv < SUBPMIC_BUCK_OTG_VOLT_MIN)
		mv = SUBPMIC_BUCK_OTG_VOLT_MIN;
	if (mv > SUBPMIC_BUCK_OTG_VOLT_MAX)
		mv = SUBPMIC_BUCK_OTG_VOLT_MAX;

	mv = (mv - SUBPMIC_BUCK_OTG_VOLT_OFFSET) / SUBPMIC_BUCK_OTG_VOLT_STEP;
	return subpmic_chg_field_write(sc, F_VBOOST, mv);
}

__maybe_unused
static int subpmic_chg_set_term(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_TERM_EN, en);
}

__maybe_unused
static int subpmic_chg_set_term_curr(struct subpmic_chg_device *sc, int ma)
{
	if (ma < SUBPMIC_BUCK_ITERM_MIN)
		ma = SUBPMIC_BUCK_ITERM_MIN;
	if (ma > SUBPMIC_BUCK_ITERM_MAX)
		ma = SUBPMIC_BUCK_ITERM_MAX;

	ma = (ma - SUBPMIC_BUCK_ITERM_OFFSET) / SUBPMIC_BUCK_ITERM_STEP;

	return subpmic_chg_field_write(sc, F_ITERM, ma);
}

__maybe_unused
static int subpmic_chg_set_term_volt(struct subpmic_chg_device *sc, int mv)
{
	if (mv < SUBPMIC_BUCK_VBAT_MIN)
		mv = SUBPMIC_BUCK_VBAT_MIN;
	if (mv > SUBPMIC_BUCK_VBAT_MAX)
		mv = SUBPMIC_BUCK_VBAT_MAX;

	mv = (mv - SUBPMIC_BUCK_VBAT_OFFSET) / SUBPMIC_BUCK_VBAT_STEP;

	return subpmic_chg_field_write(sc, F_VBAT, mv);
}

__maybe_unused
static int subpmic_chg_adc_enable(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_ADC_EN, en);
}

__maybe_unused
static int subpmic_chg_get_adc_enable(struct subpmic_chg_device *sc, bool *en)
{
	int ret = 0;

	ret |= subpmic_chg_field_read(sc, F_ADC_EN);
	if (ret < 0)
		return ret;

	*en = ret;
	return 0;
}

__maybe_unused
static int subpmic_chg_set_prechg_volt(struct subpmic_chg_device *sc, int mv)
{
	int i = 0, ret;

	if (mv < prechg_volt[0])
		mv = prechg_volt[0];
	if (mv > prechg_volt[ARRAY_SIZE(prechg_volt) - 1])
		mv = prechg_volt[ARRAY_SIZE(prechg_volt) - 1];
	for (i = 0; i < ARRAY_SIZE(prechg_volt); i++) {
		if (mv <= prechg_volt[i])
			break;
	}
	ret = subpmic_chg_field_write(sc, F_VBAT_PRECHG, i);
	if (ret < 0)
		return ret;

	return 0;
}

__maybe_unused
static int subpmic_chg_set_prechg_curr(struct subpmic_chg_device *sc, int ma)
{
	if (ma < SUBPMIC_BUCK_PRE_CURR_MIN)
		ma = SUBPMIC_BUCK_PRE_CURR_MIN;
	if (ma > SUBPMIC_BUCK_PRE_CURR_MAX)
		ma = SUBPMIC_BUCK_PRE_CURR_MAX;

	ma = (ma - SUBPMIC_BUCK_PRE_CURR_OFFSET) / SUBPMIC_BUCK_PRE_CURR_STEP;

	return subpmic_chg_field_write(sc, F_IPRECHG, ma);
}

__maybe_unused
static int subpmic_set_pd_shutdown_mode(struct subpmic_chg_device *sc, bool en)
{
	int ret = 0;
	uint8_t non_shutdown_mode_data[] = {0xA0, 0x01};
	struct i2c_msg msg = {
		.addr = 0x62,
		.flags = 0,
	};

	if (en) {
		msg.len = sizeof(non_shutdown_mode_data);
		msg.buf = non_shutdown_mode_data;
		ret = i2c_transfer(sc->adp, &msg, 1);
	}

	if (ret < 0)
		mca_log_err("i2c_transfer failed\n");

	return ret;
}

__maybe_unused
static int subpmic_xm_ctrl_low_power(struct subpmic_chg_device *sc, bool en)
{
	int ret = 0;

	if (en) {
		ret = subpmic_chg_adc_enable(sc, !en);
		ret |= subpmic_set_pd_shutdown_mode(sc, en);
	}
	return ret;
}

__maybe_unused
static int subpmic_chg_set_shipmode(struct subpmic_chg_device *sc, bool en)
{
	int ret = 0;

	ret = subpmic_xm_ctrl_low_power(sc, en);
	if (en) {
		ret = subpmic_chg_field_write(sc, F_BATFET_DLY, true);
		ret |= subpmic_chg_field_write(sc, F_BATFET_DIS, true);
	} else {
		ret |= subpmic_chg_field_write(sc, F_BATFET_DIS, false);
	}
	mca_log_info("set shipmode enable/disable = %d\n", en);
	return ret;
}

__maybe_unused
static int subpmic_chg_get_shipmode(struct subpmic_chg_device *sc, bool *en)
{
	int ret = 0;

	ret |= subpmic_chg_field_read(sc, F_BATFET_DIS);
	if (ret < 0)
		return ret;

	*en = ret;
	mca_log_info("get shipmode value = %d\n", *en);
	return 0;
}

__maybe_unused
static int subpmic_chg_set_rechg_vol(struct subpmic_chg_device *sc, int val)
{
	int i = 0;

	if (val < rechg_volt[0])
		val = rechg_volt[0];
	if (val > rechg_volt[ARRAY_SIZE(rechg_volt)-1])
		val = rechg_volt[ARRAY_SIZE(rechg_volt)-1];

	for (i = 0; i < ARRAY_SIZE(rechg_volt); i++) {
		if (val <= rechg_volt[i])
			break;
	}
	return subpmic_chg_field_write(sc, F_VRECHG, i);
}

static void bc12_timeout_workfunc(struct work_struct *work)
{
	int ret = 0;
	struct subpmic_chg_device *sc = container_of(work,
					struct subpmic_chg_device, bc12_timeout_work.work);

	mca_log_info("BC1.2 timeout\n");
	mutex_lock(&sc->bc_detect_lock);
	sc->bc_detect = false;
	mutex_unlock(&sc->bc_detect_lock);
	subpmic_chg_set_input_volt_lmt(sc, 0);

	/* Quirk for UDP */
	if (unlikely(sc->xr_quirk_dis_bc12_detect)) {
		if (test_bit(SUBPMIC_NORMAL_USE_OTG, &sc->request_otg)) {
			mca_log_info("Disable BC1.2 for otg set\n");
			return;
		}

		mca_log_info("Disable BC1.2 for quirk set\n");
		sc->state.vbus_type = SDP_DETECED;
	}
	//charger_changed(sc->sc_charger);
	subpmic_dev_notify(SC6601_SUBPMIC_EVENT_DPDM_ALERT, true);

	ret = subpmic_chg_force_dpdm(sc);
        if(ret) {
                mca_log_err("force dpdm failed\n");
        }
}

__maybe_unused
static int subpmic_chg_force_dpdm(struct subpmic_chg_device *sc)
{
	u8 val = 0;

	mutex_lock(&sc->bc_detect_lock);
	if (sc->bc_detect) {
		mutex_unlock(&sc->bc_detect_lock);
		return -EBUSY;
	}
	sc->bc_detect = true;
	mutex_unlock(&sc->bc_detect_lock);
	mca_queue_delayed_work(&sc->bc12_timeout_work, msecs_to_jiffies(300));
	subpmic_chg_set_input_volt_lmt(sc, 0);
	val = subpmic_chg_field_read(sc, F_FORCE_INDET);
	mca_log_info("force bit = %d\n", val);
	return subpmic_chg_field_write(sc, F_FORCE_INDET, true);
}

__maybe_unused
static int subpmic_chg_request_dpdm(struct subpmic_chg_device *sc, bool en)
{
	// struct subpmic_chg_device *sc = charger_get_private(charger);
	// todo
	return 0;
}

__maybe_unused
static int subpmic_chg_set_wd_timeout(struct subpmic_chg_device *sc, int ms)
{
	int i = 0;

	if (ms < subpmic_chg_wd_time[0])
		ms = subpmic_chg_wd_time[0];
	if (ms > subpmic_chg_wd_time[ARRAY_SIZE(subpmic_chg_wd_time) - 1])
		ms = subpmic_chg_wd_time[ARRAY_SIZE(subpmic_chg_wd_time) - 1];

	for (i = 0; i < ARRAY_SIZE(subpmic_chg_wd_time); i++) {
		if (ms <= subpmic_chg_wd_time[i])
			break;
	}
	return subpmic_chg_field_write(sc, F_WD_TIMER, i);
}

__maybe_unused
static int subpmic_chg_kick_wd(struct subpmic_chg_device *sc)
{
	return subpmic_chg_field_write(sc, F_WD_TIME_RST, true);
}

__maybe_unused
static int subpmic_chg_request_qc20(struct subpmic_chg_device *sc, int mv)
{
	int val = 0;

#if 0
	if (sc->qc_result != QC2_MODE)
		return -EIO;
#endif

	subpmic_chg_field_write(sc, F_QC_EN, true);
	if (mv == 5000)
		val = 3;
	else if (mv == 9000)
		val = 0;
	else if (mv == 12000)
		val = 1;

	return subpmic_chg_field_write(sc, F_QC2_V_MAX, val);

}

__maybe_unused
static int subpmic_chg_request_qc30(struct subpmic_chg_device *sc, int mv)
{
	if (sc->qc_result != QC3_MODE)
		return -EIO;

	return subpmic_chg_request_vbus(sc, mv, 200);
}

__maybe_unused
static int subpmic_chg_request_qc35(struct subpmic_chg_device *sc, int mv)
{
	if (sc->qc_result != QC3_5_18W_MODE &&
		sc->qc_result != QC3_5_27W_MODE &&
		sc->qc_result != QC3_5_45W_MODE)
		return -EIO;

	return subpmic_chg_request_vbus(sc, mv, 20);
}

__maybe_unused
static int subpmic_chg_set_safety_time(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_CHG_TIMER_EN, !!en);
}

__maybe_unused
static int subpmic_chg_is_safety_enable(struct subpmic_chg_device *sc, bool *en)
{
	int ret = 0;

	ret = subpmic_chg_field_read(sc, F_CHG_TIMER_EN);
	if (ret < 0)
		return ret;

	*en = !!ret;
	return 0;
}

__maybe_unused
static int subpmic_chg_led1_flash_enable(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_FLED1_EN, en);
}

__maybe_unused
static int subpmic_chg_led2_flash_enable(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_FLED2_EN, en);
}

__maybe_unused
static int subpmic_chg_led1_torch_enable(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_TLED1_EN, en);
}

__maybe_unused
static int subpmic_chg_led2_torch_enable(struct subpmic_chg_device *sc, bool en)
{
	return subpmic_chg_field_write(sc, F_TLED2_EN, en);
}

__maybe_unused
static int subpmic_chg_set_led1_flash_curr(struct subpmic_chg_device *sc, int curr)
{
	if (curr < SUBPMIC_LED_FLASH_CURR_MIN)
		curr = SUBPMIC_LED_FLASH_CURR_MIN;
	if (curr > SUBPMIC_LED_FLASH_CURR_MAX)
		curr = SUBPMIC_LED_FLASH_CURR_MAX;
	curr = (curr * 1000 - SUBPMIC_LED_FLASH_CURR_OFFSET) / SUBPMIC_LED_FLASH_CURR_STEP;

	return subpmic_chg_field_write(sc, F_FLED1_BR, curr);
}

__maybe_unused
static int subpmic_chg_set_led2_flash_curr(struct subpmic_chg_device *sc, int curr)
{
	if (curr < SUBPMIC_LED_FLASH_CURR_MIN)
		curr = SUBPMIC_LED_FLASH_CURR_MIN;
	if (curr > SUBPMIC_LED_FLASH_CURR_MAX)
		curr = SUBPMIC_LED_FLASH_CURR_MAX;
	curr = (curr * 1000 - SUBPMIC_LED_FLASH_CURR_OFFSET) / SUBPMIC_LED_FLASH_CURR_STEP;

	return subpmic_chg_field_write(sc, F_FLED2_BR, curr);
}

__maybe_unused
static int subpmic_chg_set_led1_torch_curr(struct subpmic_chg_device *sc, int curr)
{
	if (curr < SUBPMIC_LED_TORCH_CURR_MIN)
		curr = SUBPMIC_LED_TORCH_CURR_MIN;
	if (curr > SUBPMIC_LED_TORCH_CURR_MAX)
		curr = SUBPMIC_LED_TORCH_CURR_MAX;
	curr = (curr * 1000 - SUBPMIC_LED_TORCH_CURR_OFFSET) / SUBPMIC_LED_TORCH_CURR_STEP;

	return subpmic_chg_field_write(sc, F_TLED1_BR, curr);
}

__maybe_unused
static int subpmic_chg_set_led2_torch_curr(struct subpmic_chg_device *sc, int curr)
{
	if (curr < SUBPMIC_LED_TORCH_CURR_MIN)
		curr = SUBPMIC_LED_TORCH_CURR_MIN;
	if (curr > SUBPMIC_LED_TORCH_CURR_MAX)
		curr = SUBPMIC_LED_TORCH_CURR_MAX;
	curr = (curr * 1000 - SUBPMIC_LED_TORCH_CURR_OFFSET) / SUBPMIC_LED_TORCH_CURR_STEP;

	return subpmic_chg_field_write(sc, F_TLED2_BR, curr);
}

__maybe_unused
static int subpmic_chg_set_led_flash_timer(struct subpmic_chg_device *sc, int ms)
{
	int i = 0;

	if (ms < 0)
		return subpmic_chg_field_write(sc, F_FTIMEOUT_EN, false);

	subpmic_chg_field_write(sc, F_FTIMEOUT_EN, true);

	if (ms < led_time[0])
		ms = led_time[0];
	if (ms > led_time[ARRAY_SIZE(led_time) - 1])
		ms = led_time[ARRAY_SIZE(led_time) - 1];
	for (i = 0; i < ARRAY_SIZE(led_time); i++) {
		if (ms <= led_time[i])
			break;
	}
	return subpmic_chg_field_write(sc, F_FTIMEOUT, i);
}

__maybe_unused
static int subpmic_chg_set_led_flag_mask(struct subpmic_chg_device *sc, int mask)
{
	uint8_t val = 0;
	int ret;

	ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_LED_MASK, &val);
	if (ret < 0)
		return ret;
	val |= mask;
	return subpmic_chg_write_byte(sc, SUBPMIC_REG_LED_MASK, val);
}

__maybe_unused
static int subpmic_chg_set_led_flag_unmask(struct subpmic_chg_device *sc, int mask)
{
	uint8_t val = 0;
	int ret;

	ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_LED_MASK, &val);
	if (ret < 0)
		return ret;
	val &= ~mask;
	return subpmic_chg_write_byte(sc, SUBPMIC_REG_LED_MASK, val);
}

__maybe_unused
static int subpmic_chg_set_led_vbat_min(struct subpmic_chg_device *sc, int mv)
{
	if (mv <= SUBPMIC_LED_VBAT_MIN_MIN)
		mv = SUBPMIC_LED_VBAT_MIN_MIN;
	if (mv >= SUBPMIC_LED_VBAT_MIN_MAX)
		mv = SUBPMIC_LED_VBAT_MIN_MAX;
	mv = (mv - SUBPMIC_LED_VBAT_MIN_OFFSET) / SUBPMIC_LED_VBAT_MIN_STEP;
	return subpmic_chg_field_write(sc, F_VBAT_MIN_FLED, mv);
}

__maybe_unused
static int subpmic_chg_set_led_flash_curr(struct subpmic_chg_device *sc, int index, int ma)
{
	int ret = 0;

	switch (index) {
	case LED1_FLASH:
		ret = subpmic_chg_set_led1_flash_curr(sc, ma);
		break;
	case LED2_FLASH:
		ret = subpmic_chg_set_led2_flash_curr(sc, ma);
		break;
	case LED_ALL_FLASH:
		ret = subpmic_chg_set_led1_flash_curr(sc, ma);
		ret |= subpmic_chg_set_led2_flash_curr(sc, ma);
		break;
	default:
		ret = -1;
		break;
	}
	if (ret < 0)
		return ret;

	mutex_lock(&sc->led_lock);
	if (!sc->led_work_running) {
		sc->led_work_running = true;
		reinit_completion(&sc->flash_run);
		reinit_completion(&sc->flash_end);
		mca_queue_delayed_work(&sc->led_work, msecs_to_jiffies(0));
	}
	mutex_unlock(&sc->led_lock);
	mca_log_info("index : %d, curr : %d", index, ma);
	return 0;
}

__maybe_unused
static int subpmic_chg_set_led_flash_enable(struct subpmic_chg_device *sc, int index, bool en)
{
	subpmic_chg_set_led_flag_unmask(sc, SUBPMIC_LED_OVP_MASK);
	sc->led_index = index;
	if (en)
		complete(&sc->flash_run);

	mca_log_info("index : %d, en : %d", index, en);
	return 0;
}

__maybe_unused
static int subpmic_chg_set_led_torch(struct subpmic_chg_device *sc, int index, int ma, bool en)
{
	int ret;

	if (index == 1) {
		ret = subpmic_chg_set_led1_torch_curr(sc, ma);
		ret |= subpmic_chg_led1_torch_enable(sc, en);
	} else {
		ret = subpmic_chg_set_led2_torch_curr(sc, ma);
		ret |= subpmic_chg_led2_torch_enable(sc, en);
	}
	return ret;
}

static int subpmic_chg_enter_test_mode(struct subpmic_chg_device *sc, bool en)
{
	char str[] = "ENTERPRISE";
	uint8_t val;
	int ret, i;

	do {
		ret = subpmic_chg_read_byte(sc, 0xc1, &val);
		// not in test mode
		if (ret < 0 && !en) {
			mca_log_info("not in test mode\n");
			break;
		}
		// in test mode
		if (ret >= 0 && val == 0 && en) {
			mca_log_info("in test mode\n");
			break;
		}
		for (i = 0; i < (ARRAY_SIZE(str) - 1); i++) {
			ret = subpmic_chg_write_byte(sc, 0x5d, str[i]);
			if (ret < 0)
				return ret;
		}
	} while (true);

	return 0;
}

static int subpmic_chg_led_hw_init(struct subpmic_chg_device *sc)
{
	int ret;
	// select external source
	ret = subpmic_chg_field_write(sc, F_LED_POWER, 1);
	ret |= subpmic_chg_set_led_vbat_min(sc, 2800);
	// set flash timeout disable
	ret |= subpmic_chg_set_led_flash_timer(sc, SUBPMIC_DEFAULT_FLASH_TIMEOUT);
	ret |= subpmic_chg_set_led_flag_mask(sc, SUBPMIC_LED_OVP_MASK);
	return ret;
}

static int subpmic_pd_hw_init(struct subpmic_chg_device *sc)
{
	int ret = 0;
	uint8_t continue_time[] = {0xE7, 0x4E};
	uint8_t tx_discard_time[] = {0xEC, 0x00, 0x64};
#define SUBPMIC_PD_I2C_ADDR     0x62
	struct i2c_msg msg = {
		.addr = SUBPMIC_PD_I2C_ADDR,
		.flags = 0,
	};
	msg.len = sizeof(continue_time);
	msg.buf = continue_time;
	ret = i2c_transfer(sc->adp, &msg, 1);
	msg.len = sizeof(tx_discard_time);
	msg.buf = tx_discard_time;
	ret |= i2c_transfer(sc->adp, &msg, 1);

	return ret;
}

static int subpmic_chg_hw_init(struct subpmic_chg_device *sc)
{
	int ret = 0, i = 0;
	u8 val = 0;
	uint8_t tmp[3];
	uint32_t flag = 0;

	const struct {
		enum subpmic_chg_fields field;
		u8 val;
	} buck_init[] = {
		{F_BATSNS_EN,         sc->buck_init.batsns_en},
		{F_VBAT,              (sc->buck_init.vbat - SUBPMIC_BUCK_VBAT_OFFSET) / SUBPMIC_BUCK_VBAT_STEP},
		{F_ICHG_CC,           (sc->buck_init.ichg - SUBPMIC_BUCK_ICHG_OFFSET) / SUBPMIC_BUCK_ICHG_STEP},
		{F_IINDPM_DIS,        sc->buck_init.iindpm_dis},
		{F_VBAT_PRECHG,       sc->buck_init.vprechg},
		{F_IPRECHG,           (sc->buck_init.iprechg - SUBPMIC_BUCK_IPRECHG_OFFSET) / SUBPMIC_BUCK_IPRECHG_STEP},
		{F_TERM_EN,           sc->buck_init.iterm_en},
		{F_ITERM,             (sc->buck_init.iterm - SUBPMIC_BUCK_ITERM_OFFSET) / SUBPMIC_BUCK_ITERM_STEP},
		{F_RECHG_DIS,         sc->buck_init.rechg_dis},
		{F_RECHG_DG,          sc->buck_init.rechg_dg},
		{F_VRECHG,            sc->buck_init.rechg_volt},
		{F_CONV_OCP_DIS,      sc->buck_init.conv_ocp_dis},
		{F_TSBAT_JEITA_DIS,   0},
		{F_IBAT_OCP_DIS,      sc->buck_init.ibat_ocp_dis},
		{F_VPMID_OVP_OTG_DIS, sc->buck_init.vpmid_ovp_otg_dis},
		{F_VBAT_OVP_BUCK_DIS, sc->buck_init.vbat_ovp_buck_dis},
		{F_IBATOCP,           sc->buck_init.ibat_ocp},
		{F_CHG_TIMER,		  sc->buck_init.chg_timer},
		{F_QB_EN,             false},
		{F_ACDRV_MANUAL_EN,   true},
		{F_ICO_EN,            false},
		{F_EDL_ACTIVE_LEVEL,  false},
		{F_ACDRV_MANUAL_PRE,  true},
		{F_HVDCP_EN,          false},
	};

	ret = subpmic_chg_bulk_read(sc, SUBPMIC_REG_CHG_INT_FLG, tmp, 3);
	if (ret) {
		return ret;
	}
	flag = tmp[0] + (tmp[1] << 8) + (tmp[2] << 16);
	mca_log_info("Buck Flag:0x%x\n", flag);
	// reset all registers without flag registers
	ret = subpmic_chg_chip_reset(sc);
	// this need watchdog, if i2c operate failed, watchdog reset
	ret |= subpmic_chg_set_chg(sc, false);
	// set buck freq = 1M , boost freq = 1M
	ret |= subpmic_chg_write_byte(sc, SUBPMIC_REG_CHG_CTRL + 4, 0x8a);
	ret |= subpmic_chg_set_chg(sc, true);

	ret |= subpmic_chg_set_wd_timeout(sc, 0);
	// disable batfet rst fun
	ret = sc6601a_batfet_rst_en(sc, false);

	// clear flt && flag
	ret |= subpmic_chg_read_byte(sc, SUBPMIC_REG_HK_FLT_FLG, &val);
	ret |= subpmic_chg_read_byte(sc, SUBPMIC_REG_LED_FLAG, &val);
	ret |= subpmic_chg_read_byte(sc, SUBPMIC_REG_DPDM_INT_FLAG, &val);

	ret |= subpmic_chg_enter_test_mode(sc, true);
	ret |= subpmic_chg_read_byte(sc, 0xFC, &val);
	val |= BIT(5);
	ret |= subpmic_chg_write_byte(sc, 0xFC, val);

	ret |= subpmic_chg_read_byte(sc, 0xFD, &val);
	val |= BIT(6);
	ret |= subpmic_chg_write_byte(sc, 0xFD, val);
	ret |= subpmic_chg_read_byte(sc, 0xFD, &val);
	mca_log_err("0xFD = 0x%x\n", val);

	ret |= subpmic_chg_read_byte(sc, 0xF8, &val);
	val |= (0x07 << 1);
	ret |= subpmic_chg_write_byte(sc, 0xF8, val);
	ret |= subpmic_chg_read_byte(sc, 0xF8, &val);
	mca_log_err("0xF8 = 0x%x\n", val);

	ret |= subpmic_pd_hw_init(sc);
	ret |= subpmic_chg_enter_test_mode(sc, false);

	ret |= subpmic_chg_set_vac_ovp(sc, 14000);

	ret |= subpmic_chg_set_vbus_ovp(sc, 14000);

	ret |= subpmic_chg_set_ico_enable(sc, false);
#ifdef CONFIG_ENABLE_BOOT_DEBUG
	if (get_boot_mode() == BOOT_MODE_TEST) {
		ret |= subpmic_chg_set_acdrv(sc, false);
	} else
#endif /* CONFIG_ENABLE_BOOT_DEBUG */
	{
		ret |= subpmic_chg_set_acdrv(sc, true);
	}

	ret |= subpmic_chg_set_sys_volt(sc, 3500);
	ret |= subpmic_chg_mask_hk_irq(sc, SUBPMIC_HK_RESET_MASK |
					SUBPMIC_HK_ADC_DONE_MASK | SUBPMIC_HK_REGN_OK_MASK |
					SUBPMIC_HK_VAC_PRESENT_MASK);

	ret |= subpmic_chg_auto_dpdm_enable(sc, false);
#ifdef CONFIG_SUBPMIC_CID
	// only disable TSBAT
	ret |= subpmic_chg_write_byte(sc, SUBPMIC_REG_HK_ADC_CTRL + 1, 0x02);
	ret |= subpmic_chg_field_write(sc, F_CID_EN, true);
#endif /* CONFIG_SUBPMIC_CID */
	// mask qc int
	ret |= subpmic_chg_write_byte(sc, SUBPMIC_REG_QC3_INT_MASK, 0xFF);

	for (i = 0; i < ARRAY_SIZE(buck_init); i++) {
		ret |= subpmic_chg_field_write(sc,
					buck_init[i].field, buck_init[i].val);
	}

	ret |= subpmic_chg_led_hw_init(sc);

	/* set jeita, cool -> 5, warm -> 54.5 */
	ret |= subpmic_chg_field_write(sc, F_JEITA_COOL_TEMP, 0);
	ret |= subpmic_chg_field_write(sc, F_JEITA_WARM_TEMP, 3);

	ret |= subpmic_chg_write_byte(sc, 0x60, 0xb0);

	if (ret < 0)
		return ret;

	return 0;
}

/**
 * SOUTHCHIP CHARGER MANAGER
 */
#ifdef CONFIG_SOUTHCHIP_CHARGER_CLASS
__maybe_unused
static int sc_chg_get_chg_en(void *data, bool *en)
{
	int ret = 0;

	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	ret = subpmic_chg_field_read(sc, F_CHG_EN);
	if (ret < 0)
		return 0;

	*en = !!ret;
	mca_log_info("chg status is %s\n", *en ? "enable" : "disable");
	return ret;
}

static int sc6601_get_adc(struct subpmic_chg_device *sc, enum ADC_MODULE id)
{
	int ret = 0;

	switch (id) {
	case ADC_VBUS:
		ret = subpmic_chg_get_adc(sc, ADC_VBUS);
		mca_log_info("vbus = %d\n", ret);
		break;
	case ADC_VSYS:
		ret = subpmic_chg_get_adc(sc, ADC_VSYS) / 1000;
		break;
	case ADC_VBAT:
		ret = subpmic_chg_get_adc(sc, ADC_VBAT);
		mca_log_info("vbat = %d\n", ret);
		break;
	case ADC_VAC:
		ret = subpmic_chg_get_adc(sc, ADC_VAC) / 1000;
		mca_log_info("vac = %d\n", ret);
		break;
	case ADC_IBUS:
		ret = subpmic_chg_get_adc(sc, ADC_IBUS);
		mca_log_info("ibus = %d\n", ret);
		break;
	case ADC_IBAT:
		ret = subpmic_chg_get_adc(sc, ADC_IBAT);
		mca_log_info("ibat = %d\n", ret);
		break;
	case ADC_TSBUS:
		ret = subpmic_chg_get_adc(sc, ADC_TSBUS);
		break;
	case ADC_TSBAT:
		ret = subpmic_chg_get_adc(sc, ADC_TSBAT);
		break;
	case ADC_TDIE:
		ret = subpmic_chg_get_adc(sc, ADC_TDIE);
		break;
	default:
		ret = -1;
		break;
	}
	if (ret < 0) {
		mca_log_err("get adc data failed\n");
		return ret;
	}

	return ret;
}

static int sc6601_subpmic_bc12_det_en(int en, void *data)
{
	return 0;
}

static int sc6601_get_online(void *data, int *online)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_online(sc, online);
}

static int sc6601_get_vbus_type(int *vbus_type, void *data)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	int ret = 0;

	mca_log_info("vbus_type = %d\n", sc->state.vbus_type);
	switch (sc->state.vbus_type & 0xf) {
	case SDP_DETECED:
		*vbus_type = XM_CHARGER_TYPE_SDP;
		return XM_CHARGER_TYPE_SDP;
	case CDP_DETECED:
		*vbus_type = XM_CHARGER_TYPE_CDP;
		return XM_CHARGER_TYPE_CDP;
	case DCP_DETECED:
		*vbus_type = XM_CHARGER_TYPE_DCP;
		return XM_CHARGER_TYPE_DCP;
	case HVDCP_DETECED:
		*vbus_type = XM_CHARGER_TYPE_HVDCP2;
		return XM_CHARGER_TYPE_HVDCP2;
	case QC3_DETECED:
		*vbus_type = XM_CHARGER_TYPE_HVDCP2;
		return XM_CHARGER_TYPE_HVDCP2;
	case QC3P5_DETECED:
		*vbus_type = XM_CHARGER_TYPE_HVDCP2;
		return XM_CHARGER_TYPE_HVDCP2;
	case UNKNOWN_DETECED:
	case NON_STANDARD_DETECTED:
		*vbus_type = XM_CHARGER_TYPE_FLOAT;
		return XM_CHARGER_TYPE_FLOAT;
	default:
		*vbus_type = XM_CHARGER_TYPE_UNKNOW;
		return ret;
	}
}

static int sc6601_is_bc12_detect_done(void *data, bool *is_done)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	int ret = 0;
	*is_done = sc->bc12_det_done;
	return ret;
}

static int sc6601_is_charge_done(void *data, bool *charge_done)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_is_charge_done(sc, charge_done);
}

__maybe_unused
static int sc6601_get_hiz_status(void *data, int *hiz)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_hiz_status(sc, hiz);
}

__maybe_unused
static int sc_chg_get_ichg(void *data, int *ma)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_ichg(sc, ma);
}

__maybe_unused
static int sc6601_get_batt_id(void *data, int *id)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	*id = sc6601_get_adc(sc, ADC_BATID);
	return sc6601_get_adc(sc, ADC_BATID);
}

static int sc6601_get_die_temp(void *data, int *temp)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	*temp = sc6601_get_adc(sc, ADC_TDIE);
	return sc6601_get_adc(sc, ADC_TDIE);
}

static int sc6601_get_batt_tsns(void *data, int *tsns)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	u32 tsbat = 0;
	int i, tsbat_tem = 0;

	tsbat = sc6601_get_adc(sc, ADC_TSBAT);
	for (i = 0; i < ARRAY_SIZE(temp_var) - 1; i++) {
		if ((tsbat < temp_var[i]) && (tsbat >= temp_var[i+1])) {
			tsbat_tem = i - 10;
			break;
		}
	}

	*tsns = tsbat_tem;

	return tsbat_tem;
}

static int sc6601_get_bus_tsns(void *data, int *tsns)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	u32 tsbus = 0;
	int i, tsbus_tem = 0;

	tsbus = sc6601_get_adc(sc, ADC_TSBUS);
	for (i = 0; i < ARRAY_SIZE(temp_var) - 1; i++) {
		if ((tsbus < temp_var[i]) && (tsbus >= temp_var[i+1])) {
			tsbus_tem = i - 10;
			break;
		}
	}

	if (tsbus >= temp_var[0]) {
		tsbus_tem = -10;
	} else if (tsbus < temp_var[ARRAY_SIZE(temp_var) - 1]) {
		tsbus_tem = ARRAY_SIZE(temp_var) - 2 - 10;
	}

	*tsns = tsbus_tem;

	return tsbus_tem;
}

static int sc6601_get_sys_volt(void *data, int *vsys_min)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	*vsys_min = sc6601_get_adc(sc, ADC_VSYS) / 1000;
	return sc6601_get_adc(sc, ADC_VSYS) / 1000;
}

static int sc6601_get_batt_curr(void *data, int *curr)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	*curr = sc6601_get_adc(sc, ADC_IBAT) / 1000;
	return sc6601_get_adc(sc, ADC_IBAT) / 1000;
}

static int sc6601_get_batt_volt(void *data, int *volt)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	*volt = sc6601_get_adc(sc, ADC_VBAT) / 1000;
	return sc6601_get_adc(sc, ADC_VBAT) / 1000;
}

__maybe_unused
static int sc6601_get_batt_volt_sns(void *data, int *volt_sns)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	*volt_sns = sc6601_get_adc(sc, ADC_VBATSNS) / 1000;
	return sc6601_get_adc(sc, ADC_VBATSNS) / 1000;
}

static int sc6601_get_ac_volt(void *data, int *volt)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	*volt = sc6601_get_adc(sc, ADC_VAC);
	return *volt;
}

static int sc6601_set_wls_vdd_flag(void *data, bool en)
{
	int ret = 0;

	mca_log_info("set wls vdd flag: %d\n", en);
	return ret;
}

static int sc6601_get_bus_volt(void *data, int *bus_volt)
{
	int ret = 0;
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	/*for mca ops vbus uv*/
	*bus_volt = sc6601_get_adc(sc, ADC_VBUS);
	mca_log_info("vbus = %d\n", *bus_volt);
	return ret;
}

static int sc6601_get_bus_curr(void *data, int *bus_curr)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	*bus_curr = sc6601_get_adc(sc, ADC_IBUS);
	return *bus_curr;
}

static int sc6601_get_input_curr_lmt(void *data, int *curr_ma)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_input_curr_lmt(sc, curr_ma);
}

static int sc6601_get_input_volt_lmt(void *data, int *volt_mv)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_input_volt_lmt(sc, volt_mv);
}

static int sc6601_get_chg_status(void *data, int *chg_status)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_chg_status(sc, chg_status);
}

static int sc6601_get_chg_type(void *data, int *chg_type)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_chg_type(sc, chg_type);
}

static int sc6601_get_term_curr(void *data, int *term_curr_ma)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_term_curr(sc, term_curr_ma);
}

static int sc6601_get_term_volt(void *data, int *term_volt_mv)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_term_volt(sc, term_volt_mv);
}

__maybe_unused
static int sc_chg_set_hiz(void *data, bool en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_hiz(sc, en);
}

static int sc6601_set_input_curr_lmt(void *data, int ma)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	mca_log_info("set input current = %d mA\n", ma);
	return subpmic_chg_set_input_curr_lmt(sc, ma);
}

static int sc6601_set_input_volt_lmt(void *data, int mv)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_input_volt_lmt(sc, mv);
}

static int sc6601_set_ichg(void *data, int ma)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	mca_log_info("set ichg current = %d mA\n", ma);
	return subpmic_chg_set_ichg(sc, ma);
}

static int sc6601_set_chg(void *data, bool en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_chg(sc, en);
}

static int sc6601_pd_request_otg(void *data, bool en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	int ret = subpmic_chg_normal_request_otg(sc, en);

	if (ret < 0)
		return ret;
	return 0;
}

static int sc6601_set_otg_curr(void *data, int ma)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_otg_curr(sc, ma);
}

__maybe_unused
static int sc6601_set_qc_volt(void *data, int vbus)
{
	struct subpmic_chg_device *sc;
	int value;

	sc = qc_device;
	sc6601_get_vbus_type(&value, sc);
	if (value == XM_CHARGER_TYPE_HVDCP2)
		subpmic_chg_request_qc20(sc, vbus);
	else if (value == XM_CHARGER_TYPE_HVDCP3)
		subpmic_chg_request_qc30(sc, vbus);
	else if (value == XM_CHARGER_TYPE_HVDCP3P5)
		subpmic_chg_request_qc35(sc, vbus);
	else
		subpmic_chg_request_qc20(sc, vbus);

	mca_log_info("vbus = %d, qc type = %d\n", vbus, value);
	return 0;
}

static int sc6601_set_otg_volt(void *data, int mv)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_otg_volt(sc, mv);
}

static int sc6601_set_term(void *data, bool en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_term(sc, en);
}

static int sc6601_set_term_curr(void *data, int ma)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_term_curr(sc, ma);
}

static int sc6601_set_term_volt(void *data, int mv)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_term_volt(sc, mv);
}

static int sc6601_adc_enable(void *data, bool en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	int ret = 0;
	int antiburn = connector_antiburn_is_triggered();

	if (antiburn && !en) {
		mca_log_info("anti status = %d\n", antiburn);
		return ret;
	}

	return subpmic_chg_adc_enable(sc, en);
}

static int sc6601_get_adc_enable(void *data, bool *en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_adc_enable(sc, en);
}

static int sc6601_set_prechg_curr(void *data, int ma)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_prechg_curr(sc, ma);
}

static int sc6601_set_prechg_volt(void *data, int mv)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_prechg_volt(sc, mv);
}

static int sc6601_force_dpdm(void *data, int en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_force_dpdm(sc);
}

__maybe_unused
static int sc_chg_reset(void *data)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_chip_reset(sc);
}

static int sc6601_set_wd_timeout(void *data, int ms)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_wd_timeout(sc, ms);
}

static int sc6601_get_chg_real_type(void *data, int *real_type)
{
	struct subpmic_chg_device *sc = data;
	int ret = 0;
	int pd_active;
	int vbus_type;

	protocol_class_pd_get_pd_active(TYPEC_PORT_0, &pd_active);
	if (pd_active) {
		if (pd_active == QTI_POWER_SUPPLY_PD_ACTIVE)
			*real_type = XM_CHARGER_TYPE_PD;
		else if (pd_active == QTI_POWER_SUPPLY_PD_PPS_ACTIVE)
			*real_type = XM_CHARGER_TYPE_PPS;
	} else {
		sc6601_get_vbus_type(&vbus_type, sc);
		*real_type = vbus_type;
	}
	mca_log_info("real type is %d\n", *real_type);
	return ret;
}

static int sc6601_kick_wd(void *data)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_kick_wd(sc);
}

__maybe_unused
static int sc6601_set_vac_pd(void *data, bool en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	mca_log_info("set vac pd status = %d\n", en);
	return subpmic_chg_set_vac_pd(sc, en);
}

static int sc6601_buckchg_set_otg_en(void *data, bool en, int mv)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	if (en) {
		if (sc->otg_boost_src == 1) {    // otg boost src is gpio
			platform_class_cp_enable_ovpgate(CP_ROLE_MASTER, false);// 8581
			sc6601_set_vac_pd(sc, false);
			set_otg_en(sc, true);
		} else {
			sc6601_pd_request_otg(sc, true);
			sc6601_set_otg_volt(sc, mv);
		}
	} else {
		if (sc->otg_boost_src == 1) {    // otg boost src is gpio
			set_otg_en(sc, false);
			sc6601_set_vac_pd(sc, true);
			platform_class_cp_enable_ovpgate(CP_ROLE_MASTER, true);
		} else {
			sc6601_pd_request_otg(sc, false);
		}
	}

	return 0;
}

static int sc6601_get_otg_en_status(void *data, int *otg_status)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	int ret = 0;

	if (sc->otg_boost_src == 1) {
		ret = gpio_get_value(sc->otg_boost_en);
		mca_log_info("sc->otg_boost_gpio: %d\n", ret);
		if (ret < 0)
			return ret;
		*otg_status = ret;
	} else {
		ret = subpmic_chg_field_read(sc, F_BOOST_EN);
		if (ret < 0)
			return ret;
		*otg_status = ret;
	}

	return 0;
}

static int sc6601_get_otg_gate_enable_status(void *data, int *gate_status)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	int ret = 0;

	ret = gpio_get_value(sc->otg_en);
	mca_log_info("sc->otg_en: %d\n", ret);
	if (ret < 0)
		return ret;

	*gate_status = ret;
	return 0;
}

__maybe_unused
static int sc6601_set_ship_mode(void *data, bool en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_shipmode(sc, en);
}

__maybe_unused
static int sc6601_get_ship_mode(void *data, bool *en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_get_shipmode(sc, en);
}

__maybe_unused
static int sc_chg_set_rechg_vol(void *data, int mv)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_rechg_vol(sc, mv);
}

__maybe_unused
static int sc6601_qc_identify(void *data)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_qc_identify(sc);
}

__maybe_unused
static int sc_chg_set_safety_time(void *data, bool en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_set_safety_time(sc, en);
}

__maybe_unused
static int sc_chg_is_safety_time_en(void *data, bool *en)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;

	return subpmic_chg_is_safety_enable(sc, en);
}

static int sc6601_dump_regs(struct subpmic_chg_device *sc, char *buf)
{
	int ret = 0, reg = 0;
	u8 val = 0;
	int count = 0;

	for (reg = SUBPMIC_REG_DEVICE_ID; reg < SUBPMIC_REG_CHG_FLT_MASK; reg++) {
		ret = subpmic_chg_read_byte(sc, reg, &val);
		if (ret < 0)
			return ret;
		if (buf)
			count = snprintf(buf + count, PAGE_SIZE - count,
					"[0x%x] -> 0x%x\n", reg, val);
		mca_log_err("[0x%x] -> 0x%x\n", reg, val);
	}
	return count;
}

static void subpmic_power_irq_workfunc(struct work_struct *work)
{
	struct subpmic_chg_device *sc = container_of(work, struct subpmic_chg_device, irq_work.work);
	static int last_vbus_type = XM_CHARGER_TYPE_UNKNOW;
	static int last_plug_in;
	int ret = 0;
	int pd_active;
	int online;
	int vbus_type;

	//sc6601_request_dpdm(sc, true);

	ret = sc6601_get_vbus_type(&vbus_type, sc);
	sc->vbus_type = vbus_type;
	ret = sc6601_get_online(sc, &online);

	mca_queue_delayed_work(&sc->usb_type_work, msecs_to_jiffies(0));
	protocol_class_pd_get_pd_active(TYPEC_PORT_0, &pd_active);
	mca_log_info("vbus_type = %d, online = %d, pd_active = %d\n", sc->vbus_type, online, pd_active);

	if (!last_plug_in && online &&
		!pd_active &&
		sc->vbus_type == XM_CHARGER_TYPE_UNKNOW){
		msleep(500);
		mca_log_err("vbus gd is on\n");
		sc6601_set_input_volt_lmt(sc, sc->buck_init.vindpm);
		//sc6601_set_input_curr_lmt(sc, 100);
		//sc6601_request_dpdm(sc, true);
		usleep_range(5000, 6000);
		ret = sc6601_force_dpdm(sc, true);
		if (ret == -EIO)
			mca_queue_delayed_work(&sc->irq_work, msecs_to_jiffies(200));
	}

	if ((sc->status & SUBPMIC_STATUS_PLUGIN) &&
			!online &&
		(sc->vbus_type == XM_CHARGER_TYPE_UNKNOW ||
				sc->vbus_type == XM_CHARGER_TYPE_UNKNOW)) {
			//sc->vbus_type == SUBPMIC_VBUS_OTG)) {
		// adapter removed
		mca_log_err("adapter remove\n");

		//sc6601_request_dpdm(sc, false);

		sc->status &= ~SUBPMIC_STATUS_PLUGIN;
		mca_queue_work(&sc->adapter_out_work);
	} else if ((pd_active || sc->vbus_type != XM_CHARGER_TYPE_UNKNOW)
						&& online &&
				!(sc->status & SUBPMIC_STATUS_PLUGIN)) {
		// adapter in
		sc->status |= SUBPMIC_STATUS_PLUGIN;
		mca_log_err("adapter in\n");
		mca_queue_work(&sc->adapter_in_work);
	}

	if((sc->vbus_type != last_vbus_type && last_vbus_type == XM_CHARGER_TYPE_FLOAT) &&
		online && (sc->status & SUBPMIC_STATUS_PLUGIN)) {
		mca_log_err("slowly plugged in, update vbus type\n");
		mca_queue_work(&sc->adapter_in_work);
	}

	if (sc->vbus_type == XM_CHARGER_TYPE_DCP && online &&
			!pd_active && last_vbus_type != XM_CHARGER_TYPE_DCP) {
		mca_log_err("start qc identify\n");
		sc6601_qc_identify(sc);
	}

	sc6601_get_vbus_type(&vbus_type, sc);
	mca_log_err("after qc identify, get vbus_type = %d\n", vbus_type);
	last_vbus_type = sc->vbus_type;
	last_plug_in = online;
}

static void subpmic_power_adapter_in_workfunc(struct work_struct *work)
{
	struct subpmic_chg_device *sc = container_of(work,
									struct subpmic_chg_device, adapter_in_work);
	int pd_active;

	protocol_class_pd_get_pd_active(TYPEC_PORT_0, &pd_active);
	mca_log_err("pd_active = %d\n", pd_active);
	sc6601_adc_enable(sc, true);

	platform_class_buckchg_ops_set_vac_pd(MAIN_BUCK_CHARGER, false);

	//if (g_battmngr_noti->pd_msg.pd_active) {
	if (pd_active) {
		sc6601_set_input_volt_lmt(sc, sc->buck_init.vindpm);
		//sc6601_request_dpdm(sc, false);
		mca_log_err("PD plugged in\n");
	//} else if ((sc->vbus_type == SUBPMIC_VBUS_MAXC) ||
	} else if ((sc->vbus_type == XM_CHARGER_TYPE_HVDCP3) ||
			(sc->vbus_type == XM_CHARGER_TYPE_HVDCP3P5)) {
			sc6601_set_input_volt_lmt(sc, 8400);
			mca_log_err("HVDCP or Maxcharge adapter plugged in\n");
		//schedule_delayed_work(&bq->ico_work, msecs_to_jiffies(500));
	} else if (sc->vbus_type == XM_CHARGER_TYPE_DCP) {
		sc6601_set_input_volt_lmt(sc, sc->buck_init.vindpm);
		mca_log_err("usb dcp adapter plugged in\n");
	} else if (sc->vbus_type == XM_CHARGER_TYPE_SDP) {
		mca_log_err("host SDP plugged in\n");
		sc6601_set_input_volt_lmt(sc, sc->buck_init.vindpm);
	} else if (sc->vbus_type == XM_CHARGER_TYPE_CDP) {
		mca_log_err("host CDP plugged in\n");
		mca_log_err("pull DP high start\n");
		msleep(90);
		subpmic_chg_field_write(sc, F_DP_DRIV, 5);
		msleep(50);
		subpmic_chg_field_write(sc, F_DP_DRIV, 0);
		sc6601_set_input_volt_lmt(sc, sc->buck_init.vindpm);
		msleep(30);
		mca_log_err("pull DP high end\n");
		sc->bc12_det_done = true;
	} else if (sc->vbus_type == XM_CHARGER_TYPE_UNKNOW || sc->vbus_type == XM_CHARGER_TYPE_FLOAT) {
		mca_log_err("host FLOAT plugged in\n");
		sc6601_set_input_volt_lmt(sc, sc->buck_init.vindpm);
	} else {
		mca_log_err("other adapter plugged in,vbus_type is %d\n", sc->vbus_type);
		sc6601_set_input_volt_lmt(sc, sc->buck_init.vindpm);
	}

	cancel_delayed_work_sync(&sc->bc12_retry_work);
	// retry bc12
	if((sc->vbus_type == XM_CHARGER_TYPE_UNKNOW || sc->vbus_type == XM_CHARGER_TYPE_FLOAT ||
		(sc->vbus_type == XM_CHARGER_TYPE_DCP && !pd_active)) &&
		sc->bc12_retry_type_last == -1) {
			sc->bc12_retry_type_last = sc->vbus_type;
			mca_queue_delayed_work(&sc->bc12_retry_work, msecs_to_jiffies(3000));
	}

	// adb port
	usb_queue_device_on(sc);
}

static void subpmic_power_adapter_out_workfunc(struct work_struct *work)
{
	struct subpmic_chg_device *sc = container_of(work,
									struct subpmic_chg_device, adapter_out_work);
	int ret;

	sc->bc12_retry_cnt = 3;
	sc->bc12_retry_type_last = -1;

	ret = xr_usb_queue_off(sc->xr_usb);
	mca_log_err("queue off ret %d\n", ret);

	platform_class_buckchg_ops_set_vac_pd(MAIN_BUCK_CHARGER, true);

	sc6601_set_input_volt_lmt(sc, sc->buck_init.vindpm);
	// if (subpmic_power->vbus_type != SUBPMIC_VBUS_OTG)
	sc6601_adc_enable(sc, false);
}

static void sc6601_dump_regs_workfunc(struct work_struct *work)
{
	struct subpmic_chg_device *sc = container_of(work,
			struct subpmic_chg_device, dump_work.work);
	sc6601_dump_regs(sc, NULL);
	mca_queue_delayed_work(&sc->dump_work, msecs_to_jiffies(10000));
}
static int get_target_para_map(struct subpmic_chg_device *sc,struct device_node *np)
{
	int ret = 0;

	sc->support_bcl = of_property_read_bool(np, "support-bcl-powerlimit");
	if (!sc->support_bcl)
		return ret;

	sc->cycle_length = of_property_count_u32_elems(np, "target_cyclecount_map");
	if (sc->cycle_length < 0) {
		mca_log_info("BCL:parse cycle_length failed\n");
		return -1;
	} else {
		mca_log_info("BCL:parse cycle_length = %d\n", sc->cycle_length);
	}
	sc->store_target_cycle = devm_kcalloc(sc->dev, sc->cycle_length, sizeof(*sc->store_target_cycle), GFP_KERNEL);
	ret = of_property_read_u32_array(np, "target_cyclecount_map",
			(u32 *)sc->store_target_cycle, sc->cycle_length);

	if (ret)
		mca_log_info("BCL: parse store_target_cycle failed\n");
	else
		mca_log_info("BCL: parse store_target_cycle successful\n");

	sc->soc_length = of_property_count_u32_elems(np, "target_soc_map");
	if (sc->soc_length < 0) {
		mca_log_info("BCL:parse target_soc_map soc_length failed\n");
		return -1;
	} else {
		mca_log_info("BCL:parse target_soc_map soc_length = %d\n", sc->soc_length);
	}
	sc->store_target_soc = devm_kcalloc(sc->dev, sc->soc_length, sizeof(*sc->store_target_soc), GFP_KERNEL);
	ret = of_property_read_u32_array(np, "target_soc_map",
			(u32 *)sc->store_target_soc, sc->soc_length);
	if (ret)
		mca_log_info("BCL: parse store_target_soc failed\n");
	else
		mca_log_info("BCL: parse store_target_soc successful\n");

	/*> 10*/
	sc->powercap_over10t_length = of_property_count_u32_elems(np, "target_power_match_map_above_10");
	if (sc->powercap_over10t_length < 0) {
		mca_log_info("BCL:parse powercap_over10t_length failed\n");
		return -1;
	} else {
		mca_log_info("BCL:parse powercap_over10t_length = %d\n", sc->powercap_over10t_length);
	}
	sc->store_target_over10t_powercap = devm_kcalloc(sc->dev, sc->powercap_over10t_length, sizeof(*sc->store_target_over10t_powercap), GFP_KERNEL);
	ret = of_property_read_u32_array(np, "target_power_match_map_above_10",
		(u32 *)sc->store_target_over10t_powercap, sc->powercap_over10t_length);

	if (ret)
		mca_log_info("BCL: parse above_10 powercapfailed\n");
	else
		mca_log_info("BCL: parse above_10 powercap successful\n");

	/*0 to 10*/
	sc->powercap_0_10t_length = of_property_count_u32_elems(np, "target_power_match_map_0_10");
	if (sc->powercap_0_10t_length < 0) {
		mca_log_info("BCL:parse powercap_0_10t_length failed\n");
		return -1;
	} else {
		mca_log_info("BCL:parse powercap_0_10t_length = %d\n", sc->powercap_0_10t_length);
	}
	sc->store_target_0_10t_powercap = devm_kcalloc(sc->dev, sc->powercap_0_10t_length, sizeof(*sc->store_target_0_10t_powercap), GFP_KERNEL);
	ret = of_property_read_u32_array(np, "target_power_match_map_0_10",
		(u32 *)sc->store_target_0_10t_powercap, sc->powercap_0_10t_length);

	if (ret)
		mca_log_info("BCL: parse map_0_10 powercap failed\n");
	else
		mca_log_info("BCL: parse map_0_10 powercap successful\n");

	/*-10 to 0*/
	sc->powercap_min10_0_length = of_property_count_u32_elems(np, "target_power_match_map_minus10_0");
	if (sc->powercap_min10_0_length < 0) {
		mca_log_info("BCL:parse powercap_min10_0_length failed\n");
		return -1;
	} else {
		mca_log_info("BCL:parse powercap_min10_0_length = %d\n", sc->powercap_min10_0_length);
	}
	sc->store_target_min10_0_powercap = devm_kcalloc(sc->dev, sc->powercap_min10_0_length, sizeof(*sc->store_target_min10_0_powercap), GFP_KERNEL);
	ret = of_property_read_u32_array(np, "target_power_match_map_minus10_0",
		(u32 *)sc->store_target_min10_0_powercap, sc->powercap_min10_0_length);

	if (ret)
		mca_log_info("BCL: parse minus10_0 powercapfailed\n");
	else
		mca_log_info("BCL: parse minus10_0 powercap successful\n");

	/*-20 to -10*/
	sc->powercap_min20_min10_length = of_property_count_u32_elems(np, "target_power_match_map_minus20_minus10");
	if (sc->powercap_min20_min10_length < 0) {
		mca_log_info("BCL:parse powercap_min20_min10_length failed\n");
		return -1;
	} else {
		mca_log_info("BCL:parse powercap_min20_min10_length = %d\n", sc->powercap_min20_min10_length);
	}
	sc->store_target_min20_min10_powercap = devm_kcalloc(sc->dev, sc->powercap_min20_min10_length, sizeof(*sc->store_target_min20_min10_powercap), GFP_KERNEL);
	ret = of_property_read_u32_array(np, "target_power_match_map_minus20_minus10",
		(u32 *)sc->store_target_min20_min10_powercap, sc->powercap_min20_min10_length);

	if (ret)
		mca_log_info("BCL: parse minus20_minus10 powercapfailed\n");
	else
		mca_log_info("BCL: parse minus20_minus10 powercap successful\n");

	/*<-20*/
	sc->powercap_min20_length = of_property_count_u32_elems(np, "target_power_match_map_below_minus20");
	if (sc->powercap_min20_length < 0) {
		mca_log_info("BCL:parse powercap_min20_length failed\n");
		return -1;
	} else {
		mca_log_info("BCL:parse powercap_min20_length = %d\n", sc->powercap_min20_length);
	}
	sc->store_target_min20_powercap = devm_kcalloc(sc->dev, sc->powercap_min20_length, sizeof(*sc->store_target_min20_powercap), GFP_KERNEL);
	ret = of_property_read_u32_array(np, "target_power_match_map_below_minus20",
		(u32 *)sc->store_target_min20_powercap, sc->powercap_min20_length);

	if (ret)
		mca_log_info("BCL: parse below_minus20 powercapfailed\n");
	else
		mca_log_info("BCL: parse below_minus20 powercap successful\n");

	return ret;
}

__maybe_unused
static int get_target_soc(struct subpmic_chg_device *sc)
{
	int ret = 0;
	int i = 0;
	union power_supply_propval value;

	sc->batt_psy = power_supply_get_by_name("battery");
	if (!sc->batt_psy) {
		mca_log_err("BCL batt_psy is not find, return\n");
		return ret+1;
	}
	//ret |= platform_fg_ops_get_rsoc(FG_IC_MASTER, &capacity);
	ret = power_supply_get_property(
			sc->batt_psy, POWER_SUPPLY_PROP_CAPACITY, &value);
	for (i = 0; i < sc->soc_length; i++){
		if (value.intval == sc->store_target_soc[i]){
			return i+1;
		} else if (value.intval < sc->store_target_soc[i]) {
			break;
		}
	}
	mca_log_info("BCL:capacity = %d, get_target_soc i = %d\n", value.intval, i);
	return i;
}

__maybe_unused
static int get_target_cycle(struct subpmic_chg_device *sc)
{
	int ret = 0;
	int cyclecount;
	int i = 0;
	ret |= platform_fg_ops_get_cyclecount(FG_IC_MASTER, &cyclecount);
	for (i = 0; i < sc->cycle_length; i++){
		if (cyclecount <= sc->store_target_cycle[i]){
			break;
		}
	}
	mca_log_info("BCL: cyclecount = %d, get_target_cycle i = %d\n", cyclecount, i);
	return i+1;
}

__maybe_unused
static int get_batt_max_powercap(void *data, int *match_target_power)
{
	struct subpmic_chg_device *sc = (struct subpmic_chg_device *)data;
	int ret = 0;
	int g_temp = 0;
	int col = 0;
	int row = 0;

	if (!sc->support_bcl) {
		*match_target_power = 0;
		return ret;
	}

	ret |=platform_fg_ops_get_temp(FG_IC_MASTER, &g_temp);
	row = get_target_soc(sc);
	col = get_target_cycle(sc);
	if (g_temp > 100)
		*match_target_power = sc->store_target_over10t_powercap[(row-1)*12+col -1];
	else if ((g_temp > 0) && (g_temp <= 100))
		*match_target_power = sc->store_target_0_10t_powercap[(row-1)*12+col -1];
	else if ((g_temp > -100) && (g_temp <= 0))
		*match_target_power = sc->store_target_min10_0_powercap[(row-1)*12+col -1];
	else if ((g_temp > -200) && (g_temp <= -100))
		*match_target_power = sc->store_target_min20_min10_powercap[(row-1)*12+col -1];
	else if (g_temp <= -200)
		*match_target_power = sc->store_target_min20_powercap[(row-1)*12+col -1];

	mca_log_info("BCL:row = %d, col = %d, temp = %d， match_target_power = %d\n", row, col, g_temp, *match_target_power);
	return ret;
}

static void sc6601_subpmic_update_usb_type_work(struct work_struct *work)
{
	struct subpmic_chg_device *sc = container_of(work,
					struct subpmic_chg_device, usb_type_work.work);
	int real_type;
	int ret;
	static int last_real_type;
	int online = 0;
	int wls_online = 0;

	if (!sc) {
		mca_log_err("sc6601 device is null\n");
		return;
	}
	ret = sc6601_get_online(sc, &online);
	if (ret < 0) {
		mca_log_err("Failed to read usb_online rc=%d\n", ret);
		mca_queue_delayed_work(&sc->usb_type_work, msecs_to_jiffies(2000));
		return;
	}
	mca_log_info("online %d, sc->online =%d\n", online, sc->online);

	if (sc->online != online) {
		platform_class_wireless_is_present(WIRELESS_ROLE_MASTER, &wls_online);
		if (wls_online) {
			mca_log_info("wireless is online, ignore usb online\n");
			mca_queue_delayed_work(&sc->usb_type_work, msecs_to_jiffies(1000));
			return;
		}

		sc->online = online;
		if (online) {
			mca_log_info("send MCA_EVENT_USB_CONNECT\n");
			mca_event_block_notify(MCA_EVENT_TYPE_CHARGER_CONNECT, MCA_EVENT_USB_CONNECT, NULL);
		} else {
			mca_log_info("send MCA_EVENT_USB_DISCONNECT\n");
			mca_event_block_notify(MCA_EVENT_TYPE_CHARGER_CONNECT, MCA_EVENT_USB_DISCONNECT, NULL);
		}
	}

	ret = sc6601_get_chg_real_type(sc, &real_type);
	if (ret < 0) {
		mca_log_err("sc6601 subpmic failed to read USB_ADAP_TYPE rc=%d\n", ret);
		mca_queue_delayed_work(&sc->usb_type_work, msecs_to_jiffies(2000));
		return;
	}

	mca_log_info("usb_real_type: %d\n", real_type);
	if (last_real_type != real_type)
		last_real_type = real_type;
	else {
		mca_log_info("return can't call type changed work\n");
		return;
	}

	mca_log_info("call type change work: %d\n", real_type);
	mca_event_block_notify(MCA_EVENT_TYPE_CHARGE_TYPE, MCA_EVENT_CHARGE_TYPE_CHANGE, &real_type);
}

static int sc6601a_subpmic_get_lpd_enable(void *data, int *lpd_en)
{
	int ret = 0;

	ret = mca_lpd_get_reg(LPD_PROP_EN, lpd_en);
	if (ret < 0)
		mca_log_err("LPD get lpd_en value failed\n");
	else
		mca_log_info("LPD get lpd_en value = %d\n", *lpd_en);
	return ret;
}

static int sc6601a_subpmic_get_lpd_status(void *data, int *lpd_bliquid)
{
	int ret = 0;

	ret = mca_lpd_get_reg(LPD_PROP_STATUS, lpd_bliquid);
	if (ret < 0)
		mca_log_err("LPD get lpd status failed\n");
	else
		mca_log_info("LPD get lpd status value = %d\n", *lpd_bliquid);
	return ret;
}

static int sc6601a_subpmic_get_lpd_sbu1(void *data, int *lpd_sbu1)
{
	int ret = 0;

	ret = mca_lpd_get_reg(LPD_PROP_SBU1, lpd_sbu1);
	if (ret < 0)
		mca_log_err("LPD get sbu1 value failed\n");
	else
		mca_log_info("LPD get sbu1 value = %d\n", *lpd_sbu1);
	return ret;
}

static int sc6601a_subpmic_get_lpd_sbu2(void *data, int *lpd_sbu2)
{
	int ret = 0;

	ret = mca_lpd_get_reg(LPD_PROP_SBU2, lpd_sbu2);
	if (ret < 0)
		mca_log_err("LPD get sbu2 value failed\n");
	else
		mca_log_info("LPD get sbu2 value = %d\n", *lpd_sbu2);
	return ret;
}

static int sc6601a_subpmic_get_lpd_cc1(void *data, int *lpd_cc1)
{
	int ret = 0;

	ret = mca_lpd_get_reg(LPD_PROP_CC1, lpd_cc1);
	if (ret < 0)
		mca_log_err("LPD get cc1 value failed\n");
	else
		mca_log_info("LPD get cc1 value = %d\n", *lpd_cc1);
	return ret;
}

static int sc6601a_subpmic_get_lpd_cc2(void *data, int *lpd_cc2)
{
	int ret = 0;

	ret = mca_lpd_get_reg(LPD_PROP_CC2, lpd_cc2);
	if (ret < 0)
		mca_log_err("LPD get cc2 value failed\n");
	else
		mca_log_info("LPD get cc2 value = %d\n", *lpd_cc2);
	return ret;
}

static int sc6601a_subpmic_get_lpd_dp(void *data, int *lpd_dp)
{
	int ret = 0;

	ret = mca_lpd_get_reg(LPD_PROP_DP, lpd_dp);
	if (ret < 0)
		mca_log_err("LPD get dp value failed\n");
	else
		mca_log_info("LPD get dp value = %d\n", *lpd_dp);
	return ret;
}

static int sc6601a_subpmic_get_lpd_dm(void *data, int *lpd_dm)
{
	int ret = 0;

	ret = mca_lpd_get_reg(LPD_PROP_DM, lpd_dm);
	if (ret < 0)
		mca_log_err("LPD get dm value failed\n");
	else
		mca_log_info("LPD get dm value = %d\n", *lpd_dm);
	return ret;
}

static int sc6601a_subpmic_set_lpd_sbu1(void *data, int lpd_sbu1)
{
	return 0;
}

static int sc6601a_subpmic_set_lpd_control(void *data, int lpd_control)
{
	return 0;
}

static int sc6601a_subpmic_get_lpd_control(void *data, int *lpd_control)
{
	return 0;
}

static int sc6601a_subpmic_set_lpd_uart_control(void *data, int lpd_uart_control)
{
	return 0;
}

static int sc6601a_subpmic_get_lpd_uart_control(void *data, int *lpd_uart_control)
{
	return 0;
}

static struct platform_bc12_class_ops bc12_ops = {
	.bc12_det_en = sc6601_subpmic_bc12_det_en,
	.get_charge_type = sc6601_get_vbus_type,
	.bc12_det_done = sc6601_is_bc12_detect_done,
};

static struct platform_class_buckchg_ops sc6601a_ops = {
	.get_online = sc6601_get_online,
	.is_charge_done = sc6601_is_charge_done,
	.get_hiz_status = sc6601_get_hiz_status,
	.get_input_volt_lmt = sc6601_get_input_volt_lmt,
	.get_input_curr_lmt = sc6601_get_input_curr_lmt,
	.get_bus_curr = sc6601_get_bus_curr,
	.get_bus_volt = sc6601_get_bus_volt,
	.get_ac_volt = sc6601_get_ac_volt,
	.get_usb_sns_volt = sc6601_get_ac_volt,
	.get_batt_volt = sc6601_get_batt_volt,
	.get_batt_curr = sc6601_get_batt_curr,
	.get_sys_volt = sc6601_get_sys_volt,
	.get_bus_tsns = sc6601_get_bus_tsns,
	.get_batt_tsns = sc6601_get_batt_tsns,
	.get_die_temp = sc6601_get_die_temp,
	/*sc6601a not support get batt id
	.get_batt_id = sc6601_get_batt_id,
	*/
	.get_chg_status = sc6601_get_chg_status,
	.get_chg_type = sc6601_get_chg_type,
//	.get_otg_status = sc6601_get_otg_status,
	.get_term_curr = sc6601_get_term_curr,
	.get_term_volt = sc6601_get_term_volt,
	.set_input_curr_lmt = sc6601_set_input_curr_lmt,
	.set_input_volt_lmt = sc6601_set_input_volt_lmt,
	.set_ichg = sc6601_set_ichg,
	.set_chg = sc6601_set_chg,
//	.set_otg = sc6601_pd_request_otg,
	.set_otg_curr = sc6601_set_otg_curr,
	.set_otg_volt = sc6601_set_otg_volt,
	.set_qc_volt = sc6601_set_qc_volt,
	.set_term = sc6601_set_term,
	.set_term_curr = sc6601_set_term_curr,
	.set_term_volt = sc6601_set_term_volt,
	.adc_enable = sc6601_adc_enable,
	.get_adc_enable = sc6601_get_adc_enable,
	.set_prechg_volt = sc6601_set_prechg_volt,
	.set_prechg_curr = sc6601_set_prechg_curr,
	.force_dpdm = sc6601_force_dpdm,
	//.request_dpdm = sc6601_request_dpdm,
	.set_wd_timeout = sc6601_set_wd_timeout,
	.kick_wd = sc6601_kick_wd,
	//.enable_hvdcp = sc6601_qc_identify,
	.get_real_chg_type = sc6601_get_chg_real_type,
	.set_wls_input_curr_lmt = sc6601_set_input_curr_lmt,
	.get_wls_curr = sc6601_get_bus_curr,
	.set_wls_vdd_flag = sc6601_set_wls_vdd_flag,
	.set_ship_mode = sc6601_set_ship_mode,
	.get_ship_mode = sc6601_get_ship_mode,
	.set_vac_pd = sc6601_set_vac_pd,
	.set_otg_en = sc6601_buckchg_set_otg_en,
	.get_otg_boost_enable_status = sc6601_get_otg_en_status,
	.get_otg_gate_enable_status = sc6601_get_otg_gate_enable_status,
	.get_bcl_match_max_powercap = get_batt_max_powercap,
	.get_lpd_enable = sc6601a_subpmic_get_lpd_enable,
	.get_lpd_status = sc6601a_subpmic_get_lpd_status,
	.get_lpd_sbu1 = sc6601a_subpmic_get_lpd_sbu1,
	.get_lpd_sbu2 = sc6601a_subpmic_get_lpd_sbu2,
	.get_lpd_cc1 = sc6601a_subpmic_get_lpd_cc1,
	.get_lpd_cc2 = sc6601a_subpmic_get_lpd_cc2,
	.get_lpd_dp = sc6601a_subpmic_get_lpd_dp,
	.get_lpd_dm = sc6601a_subpmic_get_lpd_dm,
	.set_lpd_sbu1 = sc6601a_subpmic_set_lpd_sbu1,
	.set_lpd_control = sc6601a_subpmic_set_lpd_control,
	.get_lpd_control = sc6601a_subpmic_get_lpd_control,
	.set_lpd_uart_control = sc6601a_subpmic_set_lpd_uart_control,
	.get_lpd_uart_control = sc6601a_subpmic_get_lpd_uart_control,
};
#endif /* CONFIG_SOUTHCHIP_CHARGER_CLASS */

static int subpmic_chg_parse_dtb(struct subpmic_chg_device *sc, struct device_node *np)
{
	int ret, i;
	const struct {
		const char *name;
		u32 *val;
	} buck_data[] = {
		{"sc,vsys-limit",        &(sc->buck_init.vsyslim)},
		{"sc,batsnc-enable",     &(sc->buck_init.batsns_en)},
		{"sc,vbat",              &(sc->buck_init.vbat)},
		{"sc,charge-curr",       &(sc->buck_init.ichg)},
		{"sc,iindpm-disable",    &(sc->buck_init.iindpm_dis)},
		{"sc,input-curr-limit",  &(sc->buck_init.iindpm)},
		{"sc,ico-enable",        &(sc->buck_init.ico_enable)},
		{"sc,iindpm-ico",        &(sc->buck_init.iindpm_ico)},
		{"sc,precharge-volt",    &(sc->buck_init.vprechg)},
		{"sc,precharge-curr",    &(sc->buck_init.iprechg)},
		{"sc,term-en",           &(sc->buck_init.iterm_en)},
		{"sc,term-curr",         &(sc->buck_init.iterm)},
		{"sc,rechg-dis",         &(sc->buck_init.rechg_dis)},
		{"sc,rechg-dg",          &(sc->buck_init.rechg_dg)},
		{"sc,rechg-volt",        &(sc->buck_init.rechg_volt)},
		{"sc,boost-voltage",     &(sc->buck_init.vboost)},
		{"sc,boost-max-current", &(sc->buck_init.iboost)},
		{"sc,conv-ocp-dis",      &(sc->buck_init.conv_ocp_dis)},
		{"sc,tsbat-jeita-dis",   &(sc->buck_init.tsbat_jeita_dis)},
		{"sc,ibat-ocp-dis",      &(sc->buck_init.ibat_ocp_dis)},
		{"sc,vpmid-ovp-otg-dis", &(sc->buck_init.vpmid_ovp_otg_dis)},
		{"sc,vbat-ovp-buck-dis", &(sc->buck_init.vbat_ovp_buck_dis)},
		{"sc,ibat-ocp",          &(sc->buck_init.ibat_ocp)},
		{"sc,vindpm",            &(sc->buck_init.vindpm)},
		{"sc,chg-timer",         &(sc->buck_init.chg_timer)},
		{"sc,sc660x-pd-auth-en", &(sc->buck_init.sc660x_pd_auth_en)},
	};

	for (i = 0; i < ARRAY_SIZE(buck_data); i++) {
		ret = of_property_read_u32(np, buck_data[i].name,
									buck_data[i].val);
		if (ret < 0) {
			mca_log_err("not find property %s\n",
								buck_data[i].name);
			return ret;
		}
		mca_log_info("%s: %d\n", buck_data[i].name,
						(int)*buck_data[i].val);

	}
	return 0;
}

__maybe_unused
static irqreturn_t subpmic_chg_led_alert_handler(int irq, void *data)
{
	struct subpmic_chg_device *sc = data;
	int ret = 0;
	u8 val = 0;

	ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_LED_FLAG, &val);
	if (ret < 0)
		return ret;

	mca_log_info("LED Flag -> %x\n", val);

	if (val & SUBPMIC_LED_FLAG_FLASH_DONE) {
		mca_log_info("led flash done\n");
		complete(&sc->flash_end);
	}
	return IRQ_HANDLED;
}

static irqreturn_t subpmic_chg_dpdm_alert_handler(int irq, void *data)
{
	struct subpmic_chg_device *sc = data;
	int ret = 0;
	u8 val = 0, result = 0;

	ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_DPDM_INT_FLAG, &val);
	if (ret < 0)
		return ret;

	if (val & SUBPMIC_DPDM_BC12_DETECT_DONE) {
		cancel_delayed_work_sync(&sc->bc12_timeout_work);
		result = (val >> 5) & 0x7;
		switch (result) {
		case 1:
			sc->state.vbus_type = SDP_DETECED;
			break;
		case 2:
			sc->state.vbus_type = CDP_DETECED;
			break;
		case 3:
			sc->state.vbus_type = DCP_DETECED;
			break;
		case 4:
			sc->state.vbus_type = HVDCP_DETECED;
			break;
		case 5:
			sc->state.vbus_type = UNKNOWN_DETECED;
			break;
		case 6:
			sc->state.vbus_type = NON_STANDARD_DETECTED;
			break;
		default:
			break;
		}
		//sc->state.vbus_type = result;
		mutex_lock(&sc->bc_detect_lock);
		sc->bc_detect = false;
		mutex_unlock(&sc->bc_detect_lock);
		if (sc->buck_init.sc660x_pd_auth_en) {
			if (sc->state.vbus_type == DCP_DETECED) {
				subpmic_chg_set_input_curr_lmt(sc, 1500);
				mca_log_info("detect dcp type set ibus 1500mA\n");
			}
			else {
				subpmic_chg_set_input_curr_lmt(sc, 500);
				mca_log_info("set default ibus 500mA\n");
			}
		}
		subpmic_chg_set_input_volt_lmt(sc, 0);
		if (sc->state.vbus_type != CDP_DETECED)
			sc->bc12_det_done = true;
		mca_log_info("triger dpdm irq\n");
		//charger_changed(sc->sc_charger);
		subpmic_dev_notify(SC6601_SUBPMIC_EVENT_DPDM_ALERT, true);
	}
	return IRQ_HANDLED;
}

static irqreturn_t subpmic_chg_buck_alert_handler(int irq, void *data)
{
	struct subpmic_chg_device *sc = data;
	int ret;
	u8 val[3];
	u32 flt, state;

	ret = subpmic_chg_bulk_read(sc, SUBPMIC_REG_CHG_FLT_FLG, val, 2);
	if (ret < 0)
		return ret;

	flt = val[0] + (val[1] << 8);

	if (flt != 0)
		mca_log_err("Buck FAULT : 0x%x\n", flt);

	ret = subpmic_chg_bulk_read(sc, SUBPMIC_REG_CHG_INT_STAT, val, 3);
	if (ret < 0)
		return ret;

	state = val[0] + (val[1] << 8) + (val[2] << 16);
	mca_log_info("Buck State : 0x%x\n", state);

	sc->state.boost_good = !!(state & SUBPMIC_BUCK_FLAG_BOOST_GOOD);

	sc->state.chg_state = SUBPMIC_BUCK_GET_CHG_STATE(state);

	return IRQ_HANDLED;
}

static irqreturn_t subpmic_chg_hk_alert_handler(int irq, void *data)
{
	struct subpmic_chg_device *sc = data;
	static int last_online;
	int ret = 0;
	uint8_t val = 0;

	ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_HK_FLT_FLG, &val);
	if (ret < 0)
		goto out;
	if (val != 0) {
		mca_log_err("Hourse Keeping FAULT : 0x%x\n", val);
		if (val & SUBPMIC_HK_WD_TIMEOUT_MASK) {
			subpmic_chg_hw_init(sc);
			return IRQ_HANDLED;
		}
	}

	ret = subpmic_chg_read_byte(sc, SUBPMIC_REG_HK_INT_STAT, &val);
	if (ret < 0)
		goto out;

	if (likely(!sc->xr_quirk_dis_vac_detect))
		sc->state.online = (val & BIT(0)) && (val & BIT(1));
	else
		sc->state.online = !!(val & BIT(1));

	mca_log_info("online last:%d now:%d\n", last_online, sc->state.online);
	if (last_online && !sc->state.online) {
		mca_log_info("!!! plug out\n");
		// wait qc_work end
		cancel_work_sync(&sc->qc_detect_work);
		cancel_delayed_work_sync(&sc->bc12_retry_work);
		// relese qc
		subpmic_chg_field_write(sc, F_QC_EN, false);
		mutex_lock(&sc->bc_detect_lock);
		sc->bc_detect = false;
		mutex_unlock(&sc->bc_detect_lock);
		sc->state.vbus_type = BC12_TYPE_NONE;
		sc->bc12_det_done = false;
	}

	last_online = sc->state.online;
	mca_log_info("Hourse Keeping State : 0x%x\n", val);
	//charger_changed(sc->sc_charger);
	subpmic_dev_notify(SC6601_SUBPMIC_EVENT_HK_ALERT, true);
out:
	return IRQ_HANDLED;
}

static int subpmic_chg_request_irq_thread(struct subpmic_chg_device *sc)
{
	int i = 0, ret = 0;

	const struct {
		char *name;
		irq_handler_t hdlr;
	} subpmic_chg_chg_irqs[] = {
		{"Hourse Keeping", subpmic_chg_hk_alert_handler},
		{"Buck Charger", subpmic_chg_buck_alert_handler},
		{"DPDM", subpmic_chg_dpdm_alert_handler},
		//{"LED", subpmic_chg_led_alert_handler},
	};

	for (i = 0; i < ARRAY_SIZE(subpmic_chg_chg_irqs); i++) {
		ret = platform_get_irq_byname(to_platform_device(sc->dev),
				subpmic_chg_chg_irqs[i].name);
		if (ret < 0) {
			mca_log_err("failed to get irq %s\n", subpmic_chg_chg_irqs[i].name);
			return ret;
		}

		sc->irq[i] = ret;

		mca_log_info("%s irq = %d\n", subpmic_chg_chg_irqs[i].name, ret);
		ret = devm_request_threaded_irq(sc->dev, ret, NULL,
						subpmic_chg_chg_irqs[i].hdlr, IRQF_ONESHOT,
						dev_name(sc->dev), sc);
		if (ret < 0) {
			mca_log_err("failed to request irq %s\n", subpmic_chg_chg_irqs[i].name);
			return ret;
		}
	}

	return 0;
}

static void subpmic_chg_led_flash_done_workfunc(struct work_struct *work)
{
	struct subpmic_chg_device *sc = container_of(work,
					struct subpmic_chg_device, led_work.work);
	int ret = 0, now_vbus = 0;
	bool in_otg = false;
	int rc = 0;

	ret = subpmic_chg_get_otg_status(sc, &in_otg);
	if (ret < 0)
		return;

	if (in_otg)
		goto en_led_flash;
	// mask irq
	disable_irq(sc->irq[IRQ_HK]);
	disable_irq(sc->irq[IRQ_BUCK]);
	// must todo
	ret = subpmic_chg_set_chg(sc, false);
	/* wait adc update */
	msleep(100);
	ret |= subpmic_chg_field_write(sc, F_DIS_BUCKCHG_PATH, true);
	ret |= subpmic_chg_field_write(sc, F_DIS_SLEEP_FOR_OTG, true);
	// set otg volt curr
	now_vbus = subpmic_chg_get_adc(sc, ADC_VBUS) / 1000;
	mca_log_info("now vbus = %d\n", now_vbus);

	if (now_vbus < 0)
		goto err_set_acdrv;

	if (now_vbus == 0)
		ret |= subpmic_chg_set_otg_volt(sc, 5000);
	else
		ret |= subpmic_chg_set_otg_volt(sc, now_vbus);

	if (ret < 0)
		goto err_set_acdrv;

en_led_flash:
	ret = subpmic_chg_request_otg(sc, 1, true);
	if (ret < 0)
		goto err_set_otg;
	// wait flash en cmd
	// todo
	if (!wait_for_completion_timeout(&sc->flash_run,
		msecs_to_jiffies(60000))) {
		goto err_en_flash;
	}
	// open flash led
	switch (sc->led_index) {
	case LED1_FLASH:
		ret = subpmic_chg_led1_flash_enable(sc, true);
		break;
	case LED2_FLASH:
		ret = subpmic_chg_led2_flash_enable(sc, true);
		break;
	case LED_ALL_FLASH:
		ret = subpmic_chg_led1_flash_enable(sc, true);
		ret |= subpmic_chg_led2_flash_enable(sc, true);
		break;
	}

	if (ret < 0) {
		// Must close otg after otg set success
		goto err_en_flash;
	}

	rc = wait_for_completion_timeout(&sc->flash_end, msecs_to_jiffies(1000));
	if (!rc) {
		mca_log_err("flash timed out\n");
		goto err_en_flash;
	}

	switch (sc->led_index) {
	case LED1_FLASH:
		subpmic_chg_led1_flash_enable(sc, false);
		break;
	case LED2_FLASH:
		subpmic_chg_led2_flash_enable(sc, false);
		break;
	case LED_ALL_FLASH:
		subpmic_chg_led1_flash_enable(sc, false);
		subpmic_chg_led2_flash_enable(sc, false);
		break;
	}
err_en_flash:
	subpmic_chg_request_otg(sc, 1, false);
	if (in_otg)
		goto out;
err_set_otg:
	subpmic_chg_set_otg_volt(sc, 5000);
err_set_acdrv:
	// unmask irq
	subpmic_chg_field_write(sc, F_DIS_BUCKCHG_PATH, false);
	subpmic_chg_field_write(sc, F_DIS_SLEEP_FOR_OTG, false);
	subpmic_chg_set_led_flag_mask(sc, SUBPMIC_LED_OVP_MASK);
	mdelay(300);
	enable_irq(sc->irq[IRQ_HK]);
	enable_irq(sc->irq[IRQ_BUCK]);
	subpmic_chg_hk_alert_handler(0, sc);
	subpmic_chg_buck_alert_handler(0, sc);
out:
	mutex_lock(&sc->led_lock);
	sc->led_work_running = false;
	mutex_unlock(&sc->led_lock);

}

static int switch_vbus_path_gpio_init(struct subpmic_chg_device *sc)
{
	int ret = 0;

	sc->switch_vbus_gpio = of_get_named_gpio(sc->dev->of_node, "vbusp-gpio", 0);
	if (!gpio_is_valid(sc->switch_vbus_gpio)) {
		mca_log_err("failed to parse vbusp-gpio\n");
		return -EINVAL;
	}

	ret = gpio_request(sc->switch_vbus_gpio, "vbusp-gpio");
	if (ret) {
		mca_log_err("unable to request vbusp-gpio ret is %d\n", ret);
		return ret;
	}

	ret = gpio_direction_output(sc->switch_vbus_gpio, 0);
	if (ret)
		mca_log_err("unable to set direction for vbusp-gpio\n");

	return ret;
}

static int subpmic_notifier_call(struct notifier_block *nb,
			unsigned long event, void *data)
{
	struct subpmic_chg_device *sc =
		container_of(nb, struct subpmic_chg_device, subpmic_nb);
	bool plug;

	plug = *(bool *)data;
	mca_log_info("event %lu, data = %d\n", event, plug);

	switch (event) {
	case SC6601_SUBPMIC_EVENT_HK_ALERT:
		cancel_delayed_work_sync(&sc->irq_work);
		mca_queue_delayed_work(&sc->irq_work, 0);
		break;
	case SC6601_SUBPMIC_EVENT_DPDM_ALERT:
		cancel_delayed_work_sync(&sc->irq_work);
		mca_queue_delayed_work(&sc->irq_work, 0);

		/* overwrite the aicl value set by hardware when bc12 done */
		if(sc->iindpm_val){
			mca_log_info("overwrite bc12 iindpm, val = %d\n", sc->iindpm_val);
			subpmic_chg_set_input_curr_lmt(sc, sc->iindpm_val);
		}
		break;
	case SC6601_SUBPMIC_EVENT_CP_MODE:
	case SC6601_SUBPMIC_EVENT_BTB_CHANGE:
		sc6601_ctrl_vbus_path(sc, plug);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static int subpmic_cp_mode_change_cb(struct notifier_block *nb,
			unsigned long event, void *data)
{
	struct subpmic_chg_device *sc =
		container_of(nb, struct subpmic_chg_device, cp_mode_change_nb);
	int mode = 0;

	if (data)
		mode = *((int *)data);
	switch (event) {
	case MCA_EVENT_CP_MODE_CHANGE:
		if (mode == MCA_EVENT_CP_MODE_FORWARD_4)
			sc6601_ctrl_vbus_path(sc, CUTOFF_BUCK_VBUS_GPIO_VAL);
		else
			sc6601_ctrl_vbus_path(sc, RESUME_BUCK_VBUS_GPIO_VAL);
		break;
	case MCA_EVENT_CP_VUSB_OUT:
		cancel_delayed_work_sync(&sc->irq_work);
		mca_queue_delayed_work(&sc->irq_work, 0);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

SRCU_NOTIFIER_HEAD_STATIC(subpmic_notifier);

int subpmic_dev_notify(int event, bool plug_status)
{
	return srcu_notifier_call_chain(&subpmic_notifier, event, &plug_status);
}
EXPORT_SYMBOL(subpmic_dev_notify);

int register_subpmic_device_notifier(struct notifier_block *nb)
{
	return srcu_notifier_chain_register(&subpmic_notifier, nb);
}
EXPORT_SYMBOL(register_subpmic_device_notifier);

int unregister_subpmic_device_notifier(struct notifier_block *nb)
{
	return srcu_notifier_chain_unregister(&subpmic_notifier, nb);
}
EXPORT_SYMBOL(unregister_subpmic_device_notifier);

#ifdef CONFIG_ENABLE_SYSFS_DEBUG
static ssize_t subpmic_chg_show_regs(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct subpmic_chg_device *sc = dev_get_drvdata(dev);

	return subpmic_chg_dump_regs(sc, buf);
}
static int get_parameters(char *buf, unsigned long *param, int num_of_par)
{
	int cnt = 0;
	char *token = strsep(&buf, " ");

	for (cnt = 0; cnt < num_of_par; cnt++) {
		if (token) {
			if (kstrtoul(token, 0, &param[cnt]) != 0)
				return -EINVAL;

			token = strsep(&buf, " ");
		} else {
			return -EINVAL;
		}
	}

	return 0;
}

static ssize_t subpmic_chg_test_store_property(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct subpmic_chg_device *sc = dev_get_drvdata(dev);
	int ret/*, i*/;
	long val;

	ret = get_parameters((char *)buf, &val, 1);
	if (ret < 0) {
		dev_err(dev, "get parameters fail\n");
		return -EINVAL;
	}
	switch (val) {
	case 1: /* enable otg */
		subpmic_chg_request_otg(sc, 0, true);
		break;
	case 2: /* disenable otg */
		subpmic_chg_request_otg(sc, 0, false);
		break;
	case 3: /* open led1 flash mode */
		subpmic_chg_set_led_flash_curr(sc, LED1_FLASH, 300);
		break;
	case 4:
		subpmic_chg_set_led_flash_curr(sc, LED2_FLASH, 300);
		break;
	case 5:
		subpmic_chg_set_led_flash_curr(sc, LED_ALL_FLASH, 300);
		break;
	case 6:
		subpmic_chg_set_led_flash_enable(sc, LED1_FLASH, true);
		break;
	case 7:
		subpmic_chg_set_led_flash_enable(sc, LED2_FLASH, true);
		break;
	case 8:
		subpmic_chg_set_led_flash_enable(sc, LED_ALL_FLASH, true);
		break;
	case 9:
		subpmic_chg_set_led_flash_enable(sc, LED_ALL_FLASH, false);
		break;
	case 10:
		switch (subpmic_chg_qc_identify(sc)) {
		case QC2_MODE:
			subpmic_chg_request_qc20(sc, 9000);
			break;
		case QC3_MODE:
			subpmic_chg_request_qc30(sc, 9000);
			mdelay(2000);
			subpmic_chg_request_qc30(sc, 5000);
			break;
		case QC3_5_18W_MODE:
		case QC3_5_27W_MODE:
		case QC3_5_45W_MODE:
			subpmic_chg_request_qc35(sc, 9000);
			mdelay(2000);
			subpmic_chg_request_qc35(sc, 5000);
			break;
		default:
			break;
		}
		break;
	case 11:
		subpmic_chg_request_qc35(sc, 9000);
		break;
	case 12:
		subpmic_chg_field_write(sc, F_QC3_PULS, true);
		break;
	case 13:
		subpmic_chg_field_write(sc, F_QC3_MINUS, true);
		break;
	default:
		break;
	}

	return count;
}

static ssize_t subpmic_chg_test_show_property(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	ret = snprintf(buf, 256, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
		"1: otg enable", "2: otg disable",
		"3: en led1 flash", "4: dis led1 flash",
		"5: en led1 torch", "6: dis led2 torch", "7: enter shipmode");

	return ret;
}

static DEVICE_ATTR_OR(showregs, 0440, subpmic_chg_show_regs, NULL);
static DEVICE_ATTR_RW(test, 0660, subpmic_chg_test_show_property,
								subpmic_chg_test_store_property);
static void subpmic_chg_sysfs_file_init(struct device *dev)
{
	device_create_file(dev, &dev_attr_showregs);
	device_create_file(dev, &dev_attr_test);
}

#endif /* CONFIG_ENABLE_SYSFS_DEBUG */

static int subpmic_sc6601a_drv_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct subpmic_chg_device *sc;
	struct i2c_client *i2c = to_i2c_client(dev->parent);
	int i, ret;

	mca_log_info("(%s)\n", SUBPMIC_CHARGER_VERSION);
	sc = devm_kzalloc(dev, sizeof(*sc), GFP_KERNEL);

	if (!sc)
		return -ENOMEM;
	sc->rmap = dev_get_regmap(dev->parent, NULL);
	if (!sc->rmap) {
		dev_err(dev, "failed to get regmap\n");
		return -ENODEV;
	}
	sc->dev = dev;
	platform_set_drvdata(pdev, sc);
	sc->adp = i2c->adapter;

	for (i = 0; i < ARRAY_SIZE(subpmic_chg_reg_fields); i++) {
		sc->rmap_fields[i] = devm_regmap_field_alloc(dev,
							sc->rmap, subpmic_chg_reg_fields[i]);
		if (IS_ERR(sc->rmap_fields[i])) {
			dev_err(dev, "cannot allocate regmap field\n");
			return PTR_ERR(sc->rmap_fields[i]);
		}
	}

	sc->xr_usb = xr_usb_port_of_get(node);
	if (!sc->xr_usb) {
		mca_log_err("failed to get xr_usb\n");
		ret = -EPROBE_DEFER;
		goto err;
	}

	ret = switch_vbus_path_gpio_init(sc);
	if (ret < 0) {
		mca_log_err("init switch vbus path gpio failed\n");
		goto err;
	} else {
		mca_log_info("init switch vbus path gpio success\n");
	}

	ret = subpmic_chg_parse_dtb(sc, dev->of_node);
	if (ret < 0) {
		dev_err(dev, "dtb parse failed\n");
		goto err_1;
	}

	ret = subpmic_chg_hw_init(sc);
	if (ret < 0) {
		dev_err(dev, "hw init failed\n");
		goto err_1;
	}

	ret = subpmic_chg_request_irq_thread(sc);
	if (ret < 0) {
		dev_err(dev, "irq request failed\n");
		goto err_1;
	}

	ret = mca_parse_dts_u32(sc->dev->of_node, "otg_boost_src",	&sc->otg_boost_src, 0);
	if (ret)
		mca_log_err("%s failed to get otg_boost_src %d\n", __func__, sc->otg_boost_src);

	if (sc->otg_boost_src == 1) {	// otg boost src is gpio
		sc->otg_boost_en = of_get_named_gpio(sc->dev->of_node, "otg_boost_en", 0);
		if (!gpio_is_valid(sc->otg_boost_en))
			mca_log_err("failed to parse otg_boost_en gpio\n");
		ret = gpio_request(sc->otg_boost_en, "otg_boost_en");
		if (ret)
			mca_log_err("unable to request otg_boost_en gpio ret is %d\n", ret);
		else {
			ret = gpio_direction_output(sc->otg_boost_en, 0);
			if (ret)
				mca_log_err("unable to set direction for otg_boost_en gpio\n");
		}

		sc->otg_en = of_get_named_gpio(sc->dev->of_node, "otg_en", 0);
		if (!gpio_is_valid(sc->otg_en))
			mca_log_err("failed to parse otg_en gpio\n");
		ret = gpio_request(sc->otg_en, "otg_en");

		if (ret)
			mca_log_err("unable to request otg_en gpio ret is %d\n", ret);
		else {
			ret = gpio_direction_output(sc->otg_en, 0);
			if (ret)
				mca_log_err("unable to set direction for otg_en gpio\n");
		}
	}

	sc->xr_quirk_otg_boost = of_property_present(node, "sc,xr,quirk-otg-boost");
	sc->xr_quirk_dis_vac_detect = of_property_present(node, "sc,xr,quirk-dis-vac-detect");
	sc->xr_quirk_dis_bc12_detect = of_property_present(node, "sc,xr,quirk-dis-bc12-detect");

	ret = get_target_para_map(sc, node);
	if (ret)
		mca_log_err("BCL: parse target map failed\n");
	else
		mca_log_err("BCL: parse target successful\n");

	qc_device = sc;
	mutex_init(&sc->bc_detect_lock);
	mutex_init(&sc->adc_read_lock);
	mutex_init(&sc->led_lock);
	INIT_WORK(&sc->qc_detect_work, qc_detect_workfunc);
	INIT_DELAYED_WORK(&sc->led_work, subpmic_chg_led_flash_done_workfunc);
	INIT_DELAYED_WORK(&sc->bc12_timeout_work, bc12_timeout_workfunc);
	INIT_DELAYED_WORK(&sc->dump_work, sc6601_dump_regs_workfunc);
	INIT_DELAYED_WORK(&sc->irq_work, subpmic_power_irq_workfunc);
	INIT_DELAYED_WORK(&sc->bc12_retry_work, bc12_retry_workfunc);
	INIT_WORK(&sc->adapter_in_work, subpmic_power_adapter_in_workfunc);
	INIT_WORK(&sc->adapter_out_work, subpmic_power_adapter_out_workfunc);
	INIT_DELAYED_WORK(&sc->usb_type_work, sc6601_subpmic_update_usb_type_work);
	init_completion(&sc->flash_end);
	init_completion(&sc->flash_run);

	sc->subpmic_nb.notifier_call = subpmic_notifier_call;
	ret = register_subpmic_device_notifier(&sc->subpmic_nb);
	if (ret < 0) {
		mca_log_err("register subpmic notifier fail\n");
		return -EINVAL;
	}
	sc->cp_mode_change_nb.notifier_call = subpmic_cp_mode_change_cb;
	ret = mca_event_block_notify_register(MCA_EVENT_TYPE_CP_INFO,
		&sc->cp_mode_change_nb);
	if (ret)
		mca_log_err("register cp info failed\n");

	sc->bc12_retry_type_last = -1;
	sc->bc12_retry_cnt = 3;
	sc->request_otg = 0;
	sc->bc_detect = false;
	sc->led_work_running = false;

#ifdef CONFIG_ENABLE_SYSFS_DEBUG
	subpmic_chg_sysfs_file_init(sc->dev);
#endif /* CONFIG_ENABLE_SYSFS_DEBUG */
	//schedule_delayed_work(&sc->dump_work, msecs_to_jiffies(10000));
	mca_queue_delayed_work(&sc->usb_type_work, msecs_to_jiffies(0));

	cancel_delayed_work_sync(&sc->irq_work);
	mca_queue_delayed_work(&sc->irq_work, msecs_to_jiffies(5000));

	ret = platform_bc12_class_ops_register(BC12_MAIN_ROLE, &bc12_ops, sc);

	ret = platform_class_buckchg_ops_register(MAIN_BUCK_CHARGER, sc, &sc6601a_ops);
	subpmic_chg_hk_alert_handler(0, sc);
	subpmic_chg_buck_alert_handler(0, sc);

	mca_log_info("probe success\n");
	return 0;
err:
err_1:
	if (sc->xr_usb) {
		xr_usb_port_of_put(sc->xr_usb);
		sc->xr_usb = NULL;
	}

	mca_log_info("probe failed\n");
	return ret;
}

static int subpmic_chg_remove(struct platform_device *pdev)
{
	struct subpmic_chg_device *sc = platform_get_drvdata(pdev);
	int i = 0;

	for (i = 0; i < IRQ_MAX; i++)
		disable_irq(sc->irq[i]);

	if (sc->xr_usb) {
		xr_usb_port_of_put(sc->xr_usb);
		sc->xr_usb = NULL;
	}

	if (gpio_is_valid(sc->switch_vbus_gpio)) {
		gpio_free(sc->switch_vbus_gpio);
		mca_log_info("remove switch_vbus_gpio");
	}

	if (gpio_is_valid(sc->otg_boost_en)) {
		gpio_free(sc->otg_boost_en);
		mca_log_info("remove otg_boost_en gpio success");
	}

	if (gpio_is_valid(sc->otg_en)) {
		gpio_free(sc->otg_en);
		mca_log_info("remove otg_en gpio success");
	}

	return 0;
}

static void subpmic_chg_shutdown(struct platform_device *pdev)
{
	struct subpmic_chg_device *sc = platform_get_drvdata(pdev);

	subpmic_chg_chip_reset(sc);
	sc6601a_batfet_rst_en(sc, false);
}

static const struct of_device_id subpmic_chg_of_match[] = {
	{.compatible = "xm_subpmic_sc6601a,subpmic_drv",},
	{},
};

static struct platform_driver subpmic_chg_driver = {
	.driver = {
		.name = "subpmic_drv",
		.of_match_table = of_match_ptr(subpmic_chg_of_match),
	},
	.probe = subpmic_sc6601a_drv_probe,
	.remove = subpmic_chg_remove,
	.shutdown = subpmic_chg_shutdown,
};

module_platform_driver(subpmic_chg_driver);

MODULE_AUTHOR("tianye9@xiaomi.com>");
MODULE_DESCRIPTION("sc6601a driver");
MODULE_LICENSE("GPL v2");
