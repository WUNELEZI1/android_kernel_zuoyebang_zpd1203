/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __QG_IIO_H
#define __QG_IIO_H

#include <linux/iio/iio.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>
#include "smb5-lib.h"

#define is_between(left, right, value) \
		(((left) >= (right) && (left) >= (value) \
			&& (value) >= (right)) \
		|| ((left) <= (right) && (left) <= (value) \
			&& (value) < (right)))

#define POWER_REPLENISH_VOTER		"POWER_REPLENISH_VOTER"
#define FCC_STEPPER_VOTER		"FCC_STEPPER_VOTER"
#define PDO_5V_VOTER			"PDO_5V_VOTER"

#define REPLENISH_LOOP_WAIT_S (3 * 1000)
#define REPLENISH_START_WAIT_S (300 * 1000)
static int rp_work_count = 0;

struct qg_iio_channels {
	const char *datasheet_name;
	int channel_num;
	enum iio_chan_type type;
	long info_mask;
};

struct qg_dt {
	int			vbatt_empty_mv;
	int			vbatt_empty_cold_mv;
	int			vbatt_low_mv;
	int			vbatt_low_cold_mv;
	int			vbatt_cutoff_mv;
	int			iterm_ma;
	int			s2_fifo_length;
	int			s2_vbat_low_fifo_length;
	int			s2_acc_length;
	int			s2_acc_intvl_ms;
	int			sleep_s2_fifo_length;
	int			sleep_s2_acc_length;
	int			sleep_s2_acc_intvl_ms;
	int			fast_chg_s2_fifo_length;
	int			ocv_timer_expiry_min;
	int			ocv_tol_threshold_uv;
	int			s3_entry_fifo_length;
	int			s3_entry_ibat_ua;
	int			s3_exit_ibat_ua;
	int			delta_soc;
	int			rbat_conn_mohm;
	int			ignore_shutdown_soc_secs;
	int			shutdown_temp_diff;
	int			cold_temp_threshold;
	int			esr_qual_i_ua;
	int			esr_qual_v_uv;
	int			esr_disable_soc;
	int			esr_min_ibat_ua;
	int			shutdown_soc_threshold;
	int			min_sleep_time_secs;
	int			sys_min_volt_mv;
	int			fvss_vbat_mv;
	int			tcss_entry_soc;
	int			esr_low_temp_threshold;
	bool			hold_soc_while_full;
	bool			linearize_soc;
	bool			cl_disable;
	bool			cl_feedback_on;
	bool			esr_disable;
	bool			esr_discharge_enable;
	bool			qg_ext_sense;
	bool			use_cp_iin_sns;
	bool			use_s7_ocv;
	bool			qg_sleep_config;
	bool			qg_fast_chg_cfg;
	bool			fvss_enable;
	bool			multi_profile_load;
	bool			tcss_enable;
	bool			bass_enable;
	int			*dec_rate_seq;
	int			dec_rate_len;
};

#define BATT_MA_AVG_SAMPLES	8
struct batt_params {
	bool			update_now;
	int			batt_raw_soc;
	int			batt_soc;
	int			samples_num;
	int			samples_index;
	int			batt_ma_avg_samples[BATT_MA_AVG_SAMPLES];
	int			batt_ma_avg;
	int			batt_ma_prev;
	int			batt_ma;
	int			batt_mv;
	int			batt_temp;
	int			batt_rmc;/* Remaining capacity */
	int			batt_volt;
	int			batt_curr;
	ktime_t		last_soc_change_time;
};

struct battery_info {
	u32 	volt;
	u32 	cycle;
	u32 	iterm;
	u32 	fv;
	u32 	uisoc;
	u32 	rsoc;
	int 	temp;
	int 	curr;
	u32 	full_cnt;
	u32 	recharge_cnt;
	u32 	thermal_lv;
	bool 	charge_full;
	bool 	recharge;
	bool 	ffc;
	bool 	ffc_disable;
	bool 	bbc_charge_done;
	bool 	bbc_charge_enable;
};

struct qpnp_qg {
	struct device		*dev;
	struct iio_dev		*indio_dev;
	struct iio_chan_spec	*iio_chan;
	struct iio_channel	*int_iio_chans;
	struct iio_channel	**ext_iio_chans;
	struct class		*qg_class;
	struct device		*qg_device;
	struct cdev		qg_cdev;
	dev_t			dev_no;
	struct votable *fv_votable;
	struct votable *fcc_main_votable;
	struct votable *fcc_votable;
	struct votable *icl_votable;
	struct votable *chg_dis_votable;
	struct votable *pdo_5v_votable;

	struct work_struct	qg_status_change_work;
	struct batt_params	param;
	struct delayed_work	soc_monitor_work;
	struct delayed_work 	replenish_work;
	struct delayed_work	shutdown_delay_work;
	struct delayed_work 	connector_protect_work;
	struct delayed_work 	vbus_detect_work;
	int vbat_low_cnt;
	int ibat_ocp_cnt;

	struct notifier_block	nb;
	struct notifier_block   qpnp_smb5_nb;

	struct power_supply	*fg_psy;
	struct power_supply	*qg_psy;
	struct power_supply	*batt_psy;
	struct power_supply	*usb_psy;
	struct power_supply	*dc_psy;
	struct power_supply	*parallel_psy;
	struct power_supply	*cp_psy;

	struct qg_dt		dt;
	struct battery_info 	bat;
	int			sys_soc;
	int			charge_status;
	int			charge_type;
	int			soh;
	bool			charge_done;
	bool			charge_full;
	bool			keep_ffc_iterm;
	bool			fastcharge_mode_enabled;
	int			soc_reporting_ready;
	bool			shutdown_delay;
	bool			shutdown_delay_enable;
	int 			is_eu_mode;
	bool 			prp_is_enable;
	int 			cycle_count;
	bool			fake_full_status_flag;
	int			charger_plugin_event;
	int			last_temp1;
	int			last_temp2;
	int			protect;
	int			thermal_board_temp;
	int			rp_fv;
	int			batt_id;
};

#define QG_IIO_CHAN(_name, _num, _type, _mask)		\
	{						\
		.datasheet_name = _name,		\
		.channel_num = _num,			\
		.type = _type,				\
		.info_mask = _mask,			\
	},

#define QG_CHAN_VOLT(_name, _num)			\
	QG_IIO_CHAN(_name, _num, IIO_VOLTAGE,		\
		BIT(IIO_CHAN_INFO_PROCESSED))

#define QG_CHAN_CUR(_name, _num)			\
	QG_IIO_CHAN(_name, _num, IIO_CURRENT,		\
		BIT(IIO_CHAN_INFO_PROCESSED))

#define QG_CHAN_RES(_name, _num)			\
	QG_IIO_CHAN(_name, _num, IIO_RESISTANCE,	\
		BIT(IIO_CHAN_INFO_PROCESSED))

#define QG_CHAN_TEMP(_name, _num)			\
	QG_IIO_CHAN(_name, _num, IIO_TEMP,		\
		BIT(IIO_CHAN_INFO_PROCESSED))

#define QG_CHAN_POW(_name, _num)			\
	QG_IIO_CHAN(_name, _num, IIO_POWER,		\
		BIT(IIO_CHAN_INFO_PROCESSED))

#define QG_CHAN_ENERGY(_name, _num)			\
	QG_IIO_CHAN(_name, _num, IIO_ENERGY,		\
		BIT(IIO_CHAN_INFO_PROCESSED))

#define QG_CHAN_INDEX(_name, _num)			\
	QG_IIO_CHAN(_name, _num, IIO_INDEX,		\
		BIT(IIO_CHAN_INFO_PROCESSED))

#define QG_CHAN_ACT(_name, _num)			\
	QG_IIO_CHAN(_name, _num, IIO_ACTIVITY,		\
		BIT(IIO_CHAN_INFO_PROCESSED))

#define QG_CHAN_TSTAMP(_name, _num)			\
	QG_IIO_CHAN(_name, _num, IIO_TIMESTAMP,		\
		BIT(IIO_CHAN_INFO_PROCESSED))

#define QG_CHAN_COUNT(_name, _num)			\
	QG_IIO_CHAN(_name, _num, IIO_COUNT,		\
		BIT(IIO_CHAN_INFO_PROCESSED))

static const struct qg_iio_channels qg_iio_psy_channels[] = {
	QG_CHAN_ENERGY("capacity", PSY_IIO_CAPACITY)
	QG_CHAN_ENERGY("capacity_raw", PSY_IIO_CAPACITY_RAW)
	QG_CHAN_ENERGY("real_capacity", PSY_IIO_REAL_CAPACITY)
	QG_CHAN_TEMP("temp", PSY_IIO_TEMP)
	QG_CHAN_VOLT("voltage_now", PSY_IIO_VOLTAGE_NOW)
	QG_CHAN_VOLT("voltage_ocv", PSY_IIO_VOLTAGE_OCV)
	QG_CHAN_CUR("current_now", PSY_IIO_CURRENT_NOW)
	QG_CHAN_ENERGY("charge_counter", PSY_IIO_CHARGE_COUNTER)
	QG_CHAN_RES("resistance", PSY_IIO_RESISTANCE)
	QG_CHAN_RES("resistance_id", PSY_IIO_RESISTANCE_ID)
	QG_CHAN_ACT("soc_reporting_ready", PSY_IIO_SOC_REPORTING_READY)
	QG_CHAN_RES("resistance_capacitive", PSY_IIO_RESISTANCE_CAPACITIVE)
	QG_CHAN_INDEX("debug_battery", PSY_IIO_DEBUG_BATTERY)
	QG_CHAN_VOLT("voltage_min", PSY_IIO_VOLTAGE_MIN)
	QG_CHAN_VOLT("voltage_max", PSY_IIO_VOLTAGE_MAX)
	QG_CHAN_CUR("batt_full_current", PSY_IIO_BATT_FULL_CURRENT)
	QG_CHAN_INDEX("batt_profile_version", PSY_IIO_BATT_PROFILE_VERSION)
	QG_CHAN_COUNT("cycle_count", PSY_IIO_CYCLE_COUNT)
	QG_CHAN_ENERGY("charge_full", PSY_IIO_CHARGE_FULL)
	QG_CHAN_ENERGY("charge_full_design", PSY_IIO_CHARGE_FULL_DESIGN)
	QG_CHAN_TSTAMP("time_to_full_avg", PSY_IIO_TIME_TO_FULL_AVG)
	QG_CHAN_TSTAMP("time_to_full_now", PSY_IIO_TIME_TO_FULL_NOW)
	QG_CHAN_TSTAMP("time_to_empty_avg", PSY_IIO_TIME_TO_EMPTY_AVG)
	QG_CHAN_RES("esr_actual", PSY_IIO_ESR_ACTUAL)
	QG_CHAN_RES("esr_nominal", PSY_IIO_ESR_NOMINAL)
	QG_CHAN_INDEX("soh", PSY_IIO_SOH)
	QG_CHAN_INDEX("clear_soh", PSY_IIO_CLEAR_SOH)
	QG_CHAN_ENERGY("cc_soc", PSY_IIO_CC_SOC)
	QG_CHAN_ACT("fg_reset", PSY_IIO_FG_RESET)
	QG_CHAN_VOLT("voltage_avg", PSY_IIO_VOLTAGE_AVG)
	QG_CHAN_CUR("current_avg", PSY_IIO_CURRENT_AVG)
	QG_CHAN_POW("power_avg", PSY_IIO_POWER_AVG)
	QG_CHAN_POW("power_now", PSY_IIO_POWER_NOW)
	QG_CHAN_ACT("scale_mode_en", PSY_IIO_SCALE_MODE_EN)
	QG_CHAN_INDEX("batt_age_level", PSY_IIO_BATT_AGE_LEVEL)
	QG_CHAN_ACT("fg_type", PSY_IIO_FG_TYPE)
	QG_CHAN_ENERGY("soc_decimal", PSY_IIO_SOC_DECIMAL)
	QG_CHAN_ENERGY("soc_decimal_rate", PSY_IIO_SOC_DECIMAL_RATE)
	QG_CHAN_ACT("shutdown_delay", PSY_IIO_SHUTDOWN_DELAY)
	QG_CHAN_ACT("fastcharge_mode", PSY_IIO_FASTCHARGE_MODE)
	QG_CHAN_ACT("ffc_chg_termination_current", PSY_IIO_FFC_CHG_TERMINATION_CURRENT)
	QG_CHAN_ENERGY("fg1_df_check",PSY_IIO_FG1_DF_CHECK)
	QG_CHAN_ENERGY("fg1_chemid",PSY_IIO_FG1_CHEMID)
	QG_CHAN_ENERGY("pack_vendor",PSY_IIO_PACK_VENDOR)
};

enum qg_ext_iio_channels {
	INPUT_CURRENT_LIMITED = 0,
	RECHARGE_SOC,
	FORCE_RECHARGE,
	CHARGE_DONE,
	PARALLEL_CHARGING_ENABLED,
	CP_CHARGING_ENABLED,
	USB_REAL_TYPE,
	DISABLE_OTG,
	CONN_TEMP,
	SMB1390_TEMP,
};

static const char * const qg_ext_iio_chan_name[] = {
	[INPUT_CURRENT_LIMITED]		= "input_current_limited",
	[RECHARGE_SOC]			= "recharge_soc",
	[FORCE_RECHARGE]		= "force_recharge",
	[CHARGE_DONE]			= "charge_done",
	[PARALLEL_CHARGING_ENABLED]	= "parallel_charging_enabled",
	[CP_CHARGING_ENABLED]		= "cp_charging_enabled",
	[USB_REAL_TYPE]			= "real_type",
	[DISABLE_OTG]			= "disable_otg",
	[CONN_TEMP]			= "conn_temp",
	[SMB1390_TEMP]			= "smb1390_temp",
};

#endif
