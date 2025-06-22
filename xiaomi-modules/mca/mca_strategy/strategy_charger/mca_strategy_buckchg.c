// SPDX-License-Identifier: GPL-2.0
/*
 *mca_strategy_buckchg.c
 *
 * mca buck charger strategy driver
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
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <mca/common/mca_parse_dts.h>
#include <mca/common/mca_voter.h>
#include <mca/common/mca_log.h>
#include <mca/common/mca_event.h>
#include <mca/common/mca_charge_interface.h>
#include <mca/strategy/strategy_class.h>
#include <mca/platform/platform_buckchg_class.h>
#include <mca/platform/platform_cp_class.h>
#include <mca/protocol/protocol_class.h>
#include <mca/protocol/protocol_pd_class.h>
#include "inc/mca_strategy_buckchg.h"
#include <mca/strategy/strategy_fg_class.h>
#include <mca/strategy/strategy_wireless_class.h>
#include <mca/smartchg/smart_chg_class.h>
#include <mca/common/mca_hwid.h>
//#include "hwid.h"
#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/timekeeping.h>
#include <mca/common/mca_charge_mievent.h>
#include <mca/common/mca_workqueue.h>
#include <mca/common/mca_smem.h>
#include <mca/common/mca_event.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_strategy_buckchg"
#endif

#define CHECK_VBUS_9V_HIGH_TH		7600
#define CHECK_VBUS_5V_LOW_TH		6000
#define DEFAULT_PD_CURRENT_MA		500

static void strategy_buckchg_set_charge_volt(struct strategy_buckchg_dev *info, int target_volt);
static void strategy_buck_update_req_volt(struct strategy_buckchg_dev *info);
static void strategy_buckchg_exit_wireless_revchg(struct strategy_buckchg_dev *info);
static void strategy_wls_revchg_monitor_workfunc(struct work_struct *work);
static int strategy_buckchg_check_charger_change(struct strategy_buckchg_dev *info);
static void strategy_buckchg_sw_cv_start(struct strategy_buckchg_dev *info);
static void strategy_buckchg_sw_cv_stop(struct strategy_buckchg_dev *info);

static struct strategy_buckchg_dev *g_buckchg_info;

static int strategy_class_buckchg_ops_set_input_volt(struct strategy_buckchg_dev *info,
	int mv)
{
	int ret;

	ret = platform_class_buckchg_ops_set_input_volt_lmt(MAIN_BUCK_CHARGER, mv);
	if (info && info->support_multi_buck)
		ret |= platform_class_buckchg_ops_set_input_volt_lmt(AUX_BUCK_CHARGER, mv);

	return ret;
}

static int strategy_class_buckchg_ops_set_chg(struct strategy_buckchg_dev *info, bool en)
{
	int ret;

	ret = platform_class_buckchg_ops_set_chg(MAIN_BUCK_CHARGER, en);
	if (info && info->support_multi_buck)
		ret |= platform_class_buckchg_ops_set_chg(AUX_BUCK_CHARGER, en);

	return ret;
}

static int strategy_class_buckchg_ops_set_opt_fws(struct strategy_buckchg_dev *info, int mv)
{
	int ret;

	ret = platform_class_buckchg_ops_set_opt_fws(MAIN_BUCK_CHARGER, mv);
	if (info && info->support_multi_buck)
		ret |= platform_class_buckchg_ops_set_opt_fws(AUX_BUCK_CHARGER, mv);

	return ret;
}

static int strategy_class_buckchg_ops_adc_enable(struct strategy_buckchg_dev *info, bool en)
{
	int ret;

	ret = platform_class_buckchg_ops_adc_enable(MAIN_BUCK_CHARGER, en);
	if (info && info->support_multi_buck)
		ret |= platform_class_buckchg_ops_adc_enable(AUX_BUCK_CHARGER, en);

	return ret;
}

static int strategy_class_buckchg_ops_set_usb_aicl_cont_thd(struct strategy_buckchg_dev *info, int mv)
{
	int ret;

	ret = platform_class_buckchg_ops_set_usb_aicl_cont_thd(MAIN_BUCK_CHARGER, mv);
	if (info && info->support_multi_buck)
		ret |= platform_class_buckchg_ops_set_usb_aicl_cont_thd(AUX_BUCK_CHARGER, mv);

	return ret;
}

static void strategy_buckchg_parse_dt(struct strategy_buckchg_dev *info)
{
	u32 idata[REV_USBIN_TYPE_MAX] = { 0 };
	int ret = 0;

	mca_parse_dts_u32(info->dev->of_node, "support_multi_buck",
		&info->support_multi_buck, STATEGY_SUPPORT_MULTI_BUCK);
	mca_parse_dts_u32(info->dev->of_node, "ship_mode_chip",
		&info->ship_mode_chip, MAIN_BUCK_CHARGER);
	mca_parse_dts_u32(info->dev->of_node, "in_dcp",
		&info->in_dcp, CHARGE_DCP_INPUT_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "in_pd",
		&info->in_pd, CHARGE_DCP_INPUT_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "in_hvdcp",
		&info->in_hvdcp, CHARGE_DCP_INPUT_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "in_hvdcp3",
		&info->in_hvdcp3, CHARGE_DCP_INPUT_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "in_hvdcp3p5",
		&info->in_hvdcp3p5, CHARGE_DCP_INPUT_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "in_cdp",
		&info->in_cdp, CHARGE_CDP_INPUT_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "in_sdp",
		&info->in_sdp, CHARGE_SDP_CHARGE_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "in_float",
		&info->in_float, CHARGE_FLOAT_INPUT_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "chg_dcp",
		&info->chg_dcp, CHARGE_DCP_CHARGE_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "chg_pd",
		&info->chg_pd, CHARGE_DCP_CHARGE_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "chg_hvdcp",
		&info->chg_hvdcp, CHARGE_DCP_CHARGE_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "chg_hvdcp3",
		&info->chg_hvdcp3, CHARGE_DCP_CHARGE_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "chg_hvdcp3p5",
		&info->chg_hvdcp3p5, CHARGE_DCP_CHARGE_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "chg_cdp",
		&info->chg_cdp, CHARGE_CDP_CHARGE_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "chg_sdp",
		&info->chg_sdp, CHARGE_SDP_CHARGE_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "chg_float",
		&info->chg_float, CHARGE_FLOAT_CHARGE_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "chg_batt_auth_failed",
		&info->chg_batt_auth_failed, CHARGE_BATT_AUTH_FAIL_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "ffc_temp_low",
		&info->ffc_temp_low, ALLOW_FFC_TEMP_LOW_THR);
	mca_parse_dts_u32(info->dev->of_node, "ffc_temp_high",
		&info->ffc_temp_high, ALLOW_FFC_TEMP_HIGH_THR);
	mca_parse_dts_u32(info->dev->of_node, "pmic_fv_compensation",
		&info->pmic_fv_compensation, 0);
	mca_parse_dts_u32(info->dev->of_node, "mca_wire_use_sc_buck",
		&info->use_sc_buck, CHARGE_BATT_USE_SC6601A_BUCK);
	mca_parse_dts_u32(info->dev->of_node, "support_hw_bc12",
		&info->hw_bc12, DEFAULT_SUPPORT_HW_BC12);
	mca_parse_dts_u32(info->dev->of_node, "vbat_fg_to_pmic_ratio",
		&info->vbat_fg_to_pmic_ratio, VBAT_FG_TO_PMIC_RATIO_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "vote_buck_vterm_buf",
		&info->vote_buck_vterm_buf, VOTE_BUCK_VTERM_BUF_DEFAULT);
	mca_parse_dts_u32(info->dev->of_node, "vote_buck_iterm_buf",
		&info->vote_buck_iterm_buf, VOTE_BUCK_ITERM_BUF_DEFAULT);
	info->support_reverse_quick_charge = of_property_read_bool(info->dev->of_node, "support_reverse_quick_charge");
	mca_parse_dts_u32(info->dev->of_node, "sw_cv_vterm_th",
		&info->sw_cv_vterm_th, STATEGY_CHARGE_VTERM_LOW_TH);
	mca_parse_dts_u32(info->dev->of_node, "full_replug_ichg_limit",
	&info->full_replug_ichg_limit, 0);


	ret = mca_parse_dts_u32_array(info->dev->of_node, "rev_req_vadp", idata, REV_USBIN_TYPE_MAX);
	if (ret) {
		info->rev_req_vadp[REV_USBIN_TYPE_PPS] = STATEGY_USBIN_WLS_REV_CHG_VOLTAGE_DEFAULT;
		info->rev_req_vadp[REV_USBIN_TYPE_OTHER] = STATEGY_USBIN_WLS_REV_CHG_VOLTAGE_DEFAULT;
	} else
		memcpy(info->rev_req_vadp, idata, sizeof(idata));
	mca_log_debug("rev_req_vadp %d %d\n", info->rev_req_vadp[REV_USBIN_TYPE_PPS],
		info->rev_req_vadp[REV_USBIN_TYPE_OTHER]);

	ret = mca_parse_dts_u32_array(info->dev->of_node, "rev_vadp_valid_h", idata, REV_USBIN_TYPE_MAX);
	if (ret) {
		info->rev_vadp_valid_h[REV_USBIN_TYPE_PPS] = STATEGY_USBIN_WLS_REV_CHG_VOLTAGE_BUF_H;
		info->rev_vadp_valid_h[REV_USBIN_TYPE_OTHER] = STATEGY_USBIN_WLS_REV_CHG_VOLTAGE_BUF_H;
	} else
		memcpy(info->rev_vadp_valid_h, idata, sizeof(idata));
	mca_log_debug("rev_vadp_valid_h %d %d\n", info->rev_vadp_valid_h[REV_USBIN_TYPE_PPS],
		info->rev_vadp_valid_h[REV_USBIN_TYPE_OTHER]);

	ret = mca_parse_dts_u32_array(info->dev->of_node, "rev_vadp_valid_l", idata, REV_USBIN_TYPE_MAX);
	if (ret) {
		info->rev_vadp_valid_l[REV_USBIN_TYPE_PPS] = STATEGY_USBIN_WLS_REV_CHG_VOLTAGE_BUF_L;
		info->rev_vadp_valid_l[REV_USBIN_TYPE_OTHER] = STATEGY_USBIN_WLS_REV_CHG_VOLTAGE_BUF_L;
	} else
		memcpy(info->rev_vadp_valid_l, idata, sizeof(idata));
	mca_log_debug("rev_vadp_valid_l %d %d\n", info->rev_vadp_valid_l[REV_USBIN_TYPE_PPS],
		info->rev_vadp_valid_l[REV_USBIN_TYPE_OTHER]);
}

static int strategy_buckchg_enable(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = data;

	if (!data)
		return -1;
	mca_log_err("%d\n", effective_result);
	if (effective_result)
		return strategy_class_buckchg_ops_set_chg(info, true);
	else
		return strategy_class_buckchg_ops_set_chg(info, false);
}

static int strategy_buckchg_input_suspend(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = data;

	if (!data)
		return -1;
	mca_log_err("%d\n", effective_result);
	if (effective_result)
		mca_vote(info->input_limit_voter, "input_suspend", true, 0);
	else
		mca_vote(info->input_limit_voter, "input_suspend", false, 0);
	return 0;
}

static int strategy_buckchg_input_voltage(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = data;

	if (!data)
		return -1;

	info->hvdcp_allow_flag = effective_result;

	if (info->hvdcp_allow_flag) {
		info->proc_data.voltage = STATEGY_CHARGE_VBUS_9V;
		mca_vote(info->input_limit_voter, "volt_limit", info->buck_9v_in, info->buck_9v_in);
		mca_vote(info->charge_limit_voter, "volt_thermal_limit", info->buck_9v_ich, info->buck_9v_ich);
	} else {
		info->proc_data.voltage = STATEGY_CHARGE_VBUS_5V;
		mca_vote(info->input_limit_voter, "volt_limit", info->buck_5v_in, info->buck_5v_in);
		mca_vote(info->charge_limit_voter, "volt_thermal_limit", info->buck_5v_ich, info->buck_5v_ich);
	}

	return 0;
}

static int strategy_buckchg_process_multi_input_limit(struct strategy_buckchg_dev *info,
	int result)
{
	return 0;
}

static int strategy_buckchg_input_limit(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = data;
	struct timespec64 ts;
	int is_zero_speed = 0;
	int result = effective_result;

	if (!data)
		return -1;

	ktime_get_boottime_ts64(&ts);
	if ((u64)ts.tv_sec < 60) {
		// get_smem_battery_info(&is_zero_speed);
		if (is_zero_speed && effective_result) {
			result = effective_result < 1500 ? 1500 : effective_result;
			mca_log_err("is_zero_speed, result = %d, effective_result = %d\n", result, effective_result);
		}
	}

	mca_log_err("set input current %d\n", result);

	if (info->support_multi_buck)
		return strategy_buckchg_process_multi_input_limit(info, result);

	return platform_class_buckchg_ops_set_input_curr_lmt(MAIN_BUCK_CHARGER, result);
}

static int strategy_buckchg_process_multi_charge_limit(struct strategy_buckchg_dev *info,
	int result)
{
	return 0;
}

static int strategy_buckchg_charge_limit(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = data;

	if (!data)
		return -1;

	mca_log_err("set chg current %d\n", effective_result);

	effective_result *= info->vbat_fg_to_pmic_ratio;

	if (info->support_multi_buck)
		return strategy_buckchg_process_multi_charge_limit(info, effective_result);

	return platform_class_buckchg_ops_set_ichg(MAIN_BUCK_CHARGER, effective_result);
}

static int strategy_buckchg_process_multi_vterm(struct strategy_buckchg_dev *info,
	int result)
{
	return 0;
}

static int strategy_buckchg_set_vterm(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = NULL;
	int vterm = 0;

	if (!data)
		return -1;
	else {
		info = data;
		vterm = effective_result + info->pmic_fv_compensation;
	}

	vterm = (vterm / info->vbat_fg_to_pmic_ratio) + info->vote_buck_vterm_buf;

	if (info->support_multi_buck)
		return strategy_buckchg_process_multi_vterm(info, vterm);

	return platform_class_buckchg_ops_set_term_volt(MAIN_BUCK_CHARGER, vterm);
}

static int strategy_buckchg_process_multi_iterm(struct strategy_buckchg_dev *info,
	int result)
{
	return 0;
}

static int strategy_buckchg_set_iterm(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = data;

	if (!data)
		return -1;

	effective_result = (effective_result * info->vbat_fg_to_pmic_ratio) + info->vote_buck_iterm_buf;

	if (info->support_multi_buck)
		return strategy_buckchg_process_multi_iterm(info, effective_result);

	return platform_class_buckchg_ops_set_term_curr(MAIN_BUCK_CHARGER, effective_result);
}

static int strategy_buckchg_set_5v_input(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = data;

	if (!data)
		return -1;

	info->buck_5v_in = effective_result;

	if (info->proc_data.voltage != STATEGY_CHARGE_VBUS_5V)
		return 0;

	if (info->buck_5v_in)
		mca_vote(info->input_limit_voter, "volt_limit", true, effective_result);
	else
		mca_vote(info->input_limit_voter, "volt_limit", false, effective_result);

	return 0;
}

static int strategy_buckchg_set_5v_ichg(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = data;

	if (!data)
		return -1;

	info->buck_5v_ich = effective_result;

	if (info->proc_data.voltage != STATEGY_CHARGE_VBUS_5V)
		return 0;

	if (info->buck_5v_ich)
		mca_vote(info->charge_limit_voter, "volt_thermal_limit", true, effective_result);
	else
		mca_vote(info->charge_limit_voter, "volt_thermal_limit", false, effective_result);

	return 0;
}

static int strategy_buckchg_set_9v_input(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = data;

	if (!data)
		return -1;

	info->buck_9v_in = effective_result;

	if (info->proc_data.voltage != STATEGY_CHARGE_VBUS_9V)
		return 0;

	if (info->buck_9v_in)
		mca_vote(info->input_limit_voter, "volt_limit", true, effective_result);
	else
		mca_vote(info->input_limit_voter, "volt_limit", false, effective_result);

	return 0;
}

static int strategy_buckchg_set_9v_ichg(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct strategy_buckchg_dev *info = data;

	if (!data)
		return -1;

	info->buck_9v_ich = effective_result;

	if (info->proc_data.voltage != STATEGY_CHARGE_VBUS_9V)
		return 0;

	if (info->buck_9v_ich)
		mca_vote(info->charge_limit_voter, "volt_thermal_limit", true, effective_result);
	else
		mca_vote(info->charge_limit_voter, "volt_thermal_limit", false, effective_result);

	return 0;
}

static int strategy_buckchg_init_voter(struct strategy_buckchg_dev *info)
{
	/* buck charge */
	info->chg_enable_voter = mca_create_votable("chg_enable", MCA_VOTE_AND,
			strategy_buckchg_enable, STATEGY_CHARGE_DISENABLE, info);
	if (IS_ERR(info->chg_enable_voter))
		return -1;
	info->input_suppend_voter = mca_create_votable("input_suspend", MCA_VOTE_OR,
		strategy_buckchg_input_suspend, STATEGY_CHARGE_INPUT_SUSPEND, info);
	if (IS_ERR(info->input_suppend_voter))
		return -1;
	info->input_limit_voter = mca_create_votable("buck_input", MCA_VOTE_MIN,
		strategy_buckchg_input_limit, STATEGY_INPUT_DEFAULT_VALUE, info);
	if (IS_ERR(info->input_limit_voter))
		return -1;
	info->charge_limit_voter = mca_create_votable("buck_charge_curr", MCA_VOTE_MIN,
		strategy_buckchg_charge_limit, STATEGY_CHARGE_CURRENT_DEFAULT_VALUE, info);
	if (IS_ERR(info->charge_limit_voter))
		return -1;

	if (0)
		info->iterm_voter = mca_create_votable("term_curr", MCA_VOTE_MIN,
			strategy_buckchg_set_iterm, STATEGY_ITERM_DEFAULT_VALUE, info);
	else
		info->iterm_voter = mca_create_votable("term_curr", MCA_VOTE_MAX,
			strategy_buckchg_set_iterm, STATEGY_ITERM_DEFAULT_VALUE, info);
	if (IS_ERR(info->iterm_voter))
		return -1;

	info->vterm_voter = mca_create_votable("term_volt", MCA_VOTE_MIN,
		strategy_buckchg_set_vterm, STATEGY_VTERM_DEFAULT_VALUE, info);
	if (IS_ERR(info->vterm_voter))
		return -1;

	/* wire voter */
	info->input_voltage_voter = mca_create_votable("input_voltage", MCA_VOTE_AND,
		strategy_buckchg_input_voltage, 1, info);
	if (IS_ERR(info->input_voltage_voter))
		return -1;
	info->buck_5v_in_voter = mca_create_votable("buck_5v_in", MCA_VOTE_MIN,
		strategy_buckchg_set_5v_input, STATEGY_INPUT_5V_DEFAULT_VALUE, info);
	if (IS_ERR(info->buck_5v_in_voter))
		return -1;
	info->buck_5v_ich_voter = mca_create_votable("buck_5v_ich", MCA_VOTE_MIN,
		strategy_buckchg_set_5v_ichg, STATEGY_CHARGE_CURRENT_DEFAULT_VALUE, info);
	if (IS_ERR(info->buck_5v_ich_voter))
		return -1;
	info->buck_9v_in_voter = mca_create_votable("buck_9v_in", MCA_VOTE_MIN,
		strategy_buckchg_set_9v_input, STATEGY_INPUT_9V_DEFAULT_VALUE, info);
	if (IS_ERR(info->buck_9v_in_voter))
		return -1;
	info->buck_9v_ich_voter = mca_create_votable("buck_9v_ich", MCA_VOTE_MIN,
		strategy_buckchg_set_9v_ichg, STATEGY_CHARGE_CURRENT_DEFAULT_VALUE, info);
	if (IS_ERR(info->buck_9v_ich_voter))
		return -1;

	return 0;
}

static void strategy_buckchg_limit_full_replug_ichg(struct strategy_buckchg_dev *info, bool plugin)
{
	int rawsoc = 0;

	if(!info->full_replug_ichg_limit) {
		return;
	}

	strategy_class_fg_ops_get_rawsoc(&rawsoc);
	if(plugin && rawsoc >= FULL_REPLUG_LIMIT_RAWSOC_TH) {
		mca_vote(info->charge_limit_voter, "full_replug", true, info->full_replug_ichg_limit);
	} else if (rawsoc < FULL_REPLUG_LIMIT_RAWSOC_TH || info->proc_data.chg_status == MCA_BUCK_CHG_STS_CHARGE_DONE) {
		mca_vote(info->charge_limit_voter, "full_replug", false, 0);
	}
}

static void strategy_buckchg_resume_buck_ichg_limit(struct strategy_buckchg_dev *info)
{
	int rawsoc = 0;

	strategy_class_fg_ops_get_rawsoc(&rawsoc);
	if(rawsoc < BUCKCHG_OK_TO_HIGH_IBAT_RAWSOC_TH) {
		mca_vote(info->charge_limit_voter, "qc_done", false, 0);
	}
}

static void strategy_buckchg_start_charging(struct strategy_buckchg_dev *info)
{
	mca_log_info("start charging\n");
	strategy_buckchg_limit_full_replug_ichg(info, true);
	strategy_buck_update_req_volt(info);
	cancel_delayed_work_sync(&info->monitor_work);
	strategy_class_buckchg_ops_adc_enable(info, true);
	schedule_delayed_work(&info->monitor_work,  0);
}

static void strategy_buckchg_reset_charge_para(struct strategy_buckchg_dev *info)
{
	memset(&info->proc_data, 0, sizeof(info->proc_data));
	memset(&info->pwr_cap, 0, sizeof(info->pwr_cap));
	info->proc_data.voltage = STATEGY_CHARGE_VBUS_5V;
	info->is_non_compliant_qc = false;
	info->non_compliant_run_once = false;
	info->vbat_ov_count = 0;
	mca_vote(info->input_voltage_voter, "non_compliant_qc", false, 0);
	mca_vote(info->input_limit_voter, "non_compliant_qc", false, 0);

	mca_vote(info->chg_enable_voter, "online", true, STATEGY_CHARGE_DISENABLE);
	mca_vote(info->buck_5v_in_voter, "wire_chg_type",
		true, STATEGY_INPUT_DEFAULT_VALUE);

	mca_vote(info->buck_9v_in_voter, "wire_chg_type",
		true, STATEGY_INPUT_DEFAULT_VALUE);
	mca_vote(info->input_limit_voter, "icl_limit", true, STATEGY_INPUT_DEFAULT_VALUE);

	mca_vote(info->buck_9v_in_voter, "pmic_inductor", false, 0);
	mca_vote(info->buck_5v_in_voter, "pmic_inductor", false, 0);

	mca_vote(info->buck_5v_ich_voter, "wire_chg_type",
		true, STATEGY_INPUT_DEFAULT_VALUE);

	mca_vote(info->buck_9v_ich_voter, "wire_chg_type",
		true, STATEGY_CHARGE_CURRENT_DEFAULT_VALUE);
	mca_vote(info->input_voltage_voter, "real_type", true, 0);
	mca_vote(info->input_voltage_voter, "eoc_5v", false, 0);
	mca_vote(info->chg_enable_voter, "vbat_ovp", false, STATEGY_CHARGE_ENABLE);
	mca_vote(info->input_voltage_voter, "lpd", false, 0);
	cancel_delayed_work_sync(&info->csd_pulse_process_work);
	mca_vote(info->charge_limit_voter, "csd_pulse", false, 0);
	mca_vote(info->charge_limit_voter, "qc_done", false, 0);
	mca_vote(info->chg_enable_voter, "csd_pulse", false, STATEGY_CHARGE_DISENABLE);
	mca_vote(info->charge_limit_voter, "full_replug", false, 0);
	info->csd_flag = false;

	/*xring system abnormal use default ibus and ibat 500mA */
	if (info->use_sc_buck) {
		mca_vote(info->chg_enable_voter, "online", true, STATEGY_CHARGE_ENABLE);
		mca_vote(info->buck_5v_in_voter, "wire_chg_type",
				false, STATEGY_INPUT_DEFAULT_VALUE);
		mca_vote(info->buck_5v_ich_voter, "wire_chg_type",
				false, STATEGY_INPUT_DEFAULT_VALUE);
		mca_vote(info->buck_9v_in_voter, "wire_chg_type",
				false, STATEGY_INPUT_DEFAULT_VALUE);
		mca_vote(info->input_limit_voter, "icl_limit",
				false, STATEGY_INPUT_DEFAULT_VALUE);
		mca_vote(info->input_limit_voter, "subpmic_hw",
				true, MCA_WIRE_CHARGE_DEFAULT_IBUS_CURRENT);
		mca_vote(info->charge_limit_voter, "subpmic_hw",
				true, MCA_WIRE_CHARGE_DEFAULT_IBUS_CURRENT);
	}
}

static void strategy_buckchg_stop_charging(struct strategy_buckchg_dev *info)
{
	mca_log_info("stop charging\n");
	cancel_delayed_work_sync(&info->monitor_work);
	cancel_delayed_work_sync(&info->wls_revchg_monitor_work);
	cancel_delayed_work_sync(&info->check_pd_secret_work);
	strategy_buckchg_sw_cv_stop(info);
	strategy_buckchg_reset_charge_para(info);
	strategy_class_buckchg_ops_set_input_volt(info, STATEGY_CHARGE_INPUT_VOLT_DEFAULT);
	strategy_class_buckchg_ops_adc_enable(info, false);
	strategy_class_buckchg_ops_set_opt_fws(info, STATEGY_CHARGE_FWS_DEFAULT);
	strategy_class_buckchg_ops_set_usb_aicl_cont_thd(info, STATEGY_CHARGE_AICL_TH_4P4V);

	info->aicl_thd = 0;
	info->pdo_nums = 0;
}

static void strategy_buckchg_process_online_change(int value, struct strategy_buckchg_dev *info)
{
	if (value == info->proc_data.online)
		return;

	mca_event_block_notify(MCA_EVENT_TYPE_BATTERY_INFO, MCA_EVENT_BATTERY_STS_CHANGE, NULL);
	info->proc_data.online = value;
	if (value) {
		strategy_buckchg_start_charging(info);
	} else {
		info->pps_ptf = USBPD_BUCK_DPM_PORT_PPS_PTF_NOT_SUPPORTED;
		strategy_buckchg_stop_charging(info);
		info->proc_data.charge_done_force_5v = false;
		info->verify_process_end = 0;
	}
}

#define PDO_9V_VOLATGE 9000
static void stategy_buckchg_is_pdo_9v(struct strategy_buckchg_dev *info)
{
	if (info->proc_data.real_type != XM_CHARGER_TYPE_PD &&
		info->proc_data.real_type != XM_CHARGER_TYPE_PPS &&
		info->proc_data.real_type != XM_CHARGER_TYPE_PD_VERIFY)
		return;

	if (info->pdo_nums == 1 && info->pwr_cap.nums > 1)
		mca_event_block_notify(MCA_EVENT_CHARGE_STATUS, MCA_EVENT_USB_STS_CHANGE, NULL);
	info->pdo_nums = info->pwr_cap.nums;

	if (info->pwr_cap.nums == 0) {
		mca_log_info(" pwr_cap.nums is null\n");
		return;
	}

	for (int i = 0; i < info->pwr_cap.nums; i++) {
		if (info->pwr_cap.cap[i].max_voltage == info->pwr_cap.cap[i].min_voltage &&
			info->pwr_cap.cap[i].max_voltage == PDO_9V_VOLATGE) {
			info->proc_data.is_pd_9v = true;
			info->proc_data.curr_pd_pos = i;
			mca_log_info("pdo[%d] can support 9v: %d\n", i, info->proc_data.is_pd_9v);
			return;
		}
	}

	info->proc_data.is_pd_9v = false;
	info->proc_data.curr_pd_pos = 0;
	return;

}

static void strategy_buckchg_process_type_change(int value, struct strategy_buckchg_dev *info)
{
	struct timespec64 ts;

	if (value == info->proc_data.real_type) {
		if (value == XM_CHARGER_TYPE_UNKNOW)
			strategy_buckchg_check_charger_change(info);
		return;
	}

	info->proc_data.real_type = value;
	strategy_buckchg_check_charger_change(info);

	if (value == XM_CHARGER_TYPE_UNKNOW)
		return;

	if (info->proc_data.real_type == XM_CHARGER_TYPE_PD ||
		info->proc_data.real_type == XM_CHARGER_TYPE_PPS ||
		info->proc_data.real_type == XM_CHARGER_TYPE_PD_VERIFY) {
		protocol_class_get_adapter_power_cap(ADAPTER_PROTOCOL_PD, &info->pwr_cap);
		protocol_class_pd_get_suspend_support_status(TYPEC_PORT_0, &info->proc_data.pdsuspendsupported);
	}

	if (info->is_eu_model && info->proc_data.real_type == XM_CHARGER_TYPE_PPS) {
			info->proc_data.eu_start_time = ktime_get_boottime();
			mca_log_info("is_eu_model for PPS eu_start_time = %lld\n", info->proc_data.eu_start_time);
	}
	stategy_buckchg_is_pdo_9v(info);
	strategy_buck_update_req_volt(info);
	mod_delayed_work(system_wq, &info->monitor_work, 0);

	ktime_get_boottime_ts64(&ts);
	mca_log_info("verify_process_end: %d\n", info->verify_process_end);
	if (info->proc_data.real_type == XM_CHARGER_TYPE_PD) {
		if (!info->verify_process_end && (u64)ts.tv_sec > 60) {
			mca_vote(info->input_limit_voter, "icl_limit", true, STATEGY_INPUT_DEFAULT_VALUE);
			schedule_delayed_work(&info->check_pd_secret_work, msecs_to_jiffies(10000));
		}
	} else {
		mca_vote(info->input_limit_voter, "icl_limit", false, STATEGY_INPUT_DEFAULT_VALUE);
	}
}

static void strategy_buckchg_process_antiburn_change(int value, struct strategy_buckchg_dev *info)
{
	mca_log_info("value: %d\n", value);
	strategy_buckchg_set_charge_volt(info, STATEGY_CHARGE_VBUS_5V);
	if (value)
		strategy_buckchg_stop_charging(info);
	else if (info->proc_data.online)
		strategy_buckchg_start_charging(info);
}

static void strategy_buckchg_process_batt_btb_change(int value, struct strategy_buckchg_dev *info)
{
	if (value) {
		mca_vote(info->input_voltage_voter, "batt_miss", true, 0);
		mca_vote(info->input_limit_voter, "batt_miss", true, STAEGY_BATT_MISS_ICL);
#ifndef CONFIG_FACTORY_BUILD
		mca_vote(info->vterm_voter, "batt_miss", true, STAEGY_BATT_MISS_FV);
#endif
	} else {
		mca_vote(info->input_voltage_voter, "batt_miss", false, 0);
		mca_vote(info->input_limit_voter, "batt_miss", false, STATEGY_CHARGE_CURRENT_DEFAULT_VALUE);
		mca_vote(info->vterm_voter, "batt_miss", false, STATEGY_VTERM_DEFAULT_VALUE);
	}
}

static void strategy_buckchg_process_soc_limit_change(int value, struct strategy_buckchg_dev *info)
{
	if (value) {
		// use vote to control charge enable/disable
		mca_vote(info->chg_enable_voter, "soc_limit", true, STATEGY_CHARGE_DISENABLE);
		mca_vote(info->input_voltage_voter, "soc_limit", true, 0);
		mca_log_info("SOC limit triggered, stopping charge\n");
		mca_vote(info->charge_limit_voter, "csd_pulse", false, 0);
	} else {
		mca_vote(info->chg_enable_voter, "soc_limit", false, STATEGY_CHARGE_ENABLE);
		mca_vote(info->input_voltage_voter, "soc_limit", false, 0);
		mca_log_info("SOC limit released, starting charge\n");
		if (info->proc_data.online)
			mod_delayed_work(system_wq, &info->monitor_work, 0);
	}
}

static void strategy_buckchg_process_cap_change(struct strategy_buckchg_dev *info)
{
	if (info->proc_data.real_type == XM_CHARGER_TYPE_PD ||
		info->proc_data.real_type == XM_CHARGER_TYPE_PPS ||
		info->proc_data.real_type == XM_CHARGER_TYPE_PD_VERIFY) {
		protocol_class_get_adapter_power_cap(ADAPTER_PROTOCOL_PD, &info->pwr_cap);
		protocol_class_pd_get_suspend_support_status(TYPEC_PORT_0, &info->proc_data.pdsuspendsupported);
	}

	stategy_buckchg_is_pdo_9v(info);
	strategy_buck_update_req_volt(info);
	mod_delayed_work(system_wq, &info->monitor_work, 0);
}

static void strategy_buckchg_process_cancel_monitor_work(struct strategy_buckchg_dev *info)
{
	int ret;
	ret = cancel_delayed_work_sync(&info->monitor_work);
	return;
}

static void strategy_buckchg_process_csd_pulse(int value, struct strategy_buckchg_dev *info)
{
	int ffc_sts;

	ffc_sts = strategy_class_fg_get_fastcharge();

	mca_log_info("value: %d, ffc_sts: %d\n", value, ffc_sts);
	if (ffc_sts == true && value > 0)
		strategy_class_fg_set_fastcharge(false);

	if (value == 1 || value == 2) {
		mca_vote(info->charge_limit_voter, "csd_pulse", true, 1500);
		info->csd_flag = true;
		schedule_delayed_work(&info->csd_pulse_process_work, msecs_to_jiffies(75000));
	} else if (value == 3) {
		mca_vote(info->charge_limit_voter, "csd_pulse", true, 1500);
	} else if (value == 0) {
		cancel_delayed_work_sync(&info->csd_pulse_process_work);
		mca_vote(info->charge_limit_voter, "csd_pulse", false, 0);
		mca_vote(info->chg_enable_voter, "csd_pulse", false, STATEGY_CHARGE_DISENABLE);
		info->csd_flag = false;
	}
}

static void strategy_buckchg_cp_revert_handler(int auth_pos, struct strategy_buckchg_dev *info)
{
	static int last_otg_present = 0;
	static int last_pos = 1;
	bool otg_present;
	int otg_enable;
	int pos;

	info->cp_revert_auth = (auth_pos & 0xffff0000) >> 16;
	pos = auth_pos & 0xffff;

	protocol_class_pd_get_otg_plugin_status(TYPEC_PORT_0, &otg_present);
	mca_log_err("otg_present: %d, last_otg_present: %d\n", otg_present, last_otg_present);
	mca_log_err("cp_revert_auth: %d, pos: %d, last_pos: %d\n", info->cp_revert_auth, pos, last_pos);

	if (otg_present && last_pos == pos) {
		mca_log_err("same handler, ignore...");
		platform_class_cp_dump_register(CP_ROLE_MASTER);
		return;
	}

	if ((last_otg_present != otg_present) && otg_present)
		schedule_delayed_work(&info->source_status_monitor_work,
			msecs_to_jiffies(SOURCE_STATUS_MONITOR_INTERVAL));
	else if ((last_otg_present != otg_present) && !otg_present)
		cancel_delayed_work_sync(&info->source_status_monitor_work);

	last_otg_present = otg_present;
	last_pos = pos;

	if (otg_present) {
		if (pos == 1) {
			mca_log_err("start external boost");

			// enable external boost
			otg_enable = 1;
			otg_enable = (0 << 16) | (2 << 8) | otg_enable;
			platform_class_buckchg_ops_set_boost_enable(MAIN_BUCK_CHARGER, otg_enable);

			// close revert cp boost
			msleep(300);
			platform_class_cp_set_charging_enable(CP_ROLE_MASTER, false);
			platform_class_cp_enable_ovpgate(CP_ROLE_MASTER, false);
			platform_class_cp_enable_wpcgate(CP_ROLE_MASTER, false);
			platform_class_cp_set_qb(CP_ROLE_MASTER, false);
			platform_class_cp_set_mode(CP_ROLE_MASTER, CP_MODE_FORWARD_2_1);
		} else if (pos ==2) {
			mca_log_err("start revert 1_2 cp");

			// enable revert cp boost
			platform_class_cp_enable_acdrv_manual(CP_ROLE_MASTER, true);
			platform_class_cp_set_mode(CP_ROLE_MASTER, CP_MODE_REVERSE_1_2);
			platform_class_cp_enable_wpcgate(CP_ROLE_MASTER, false);
			platform_class_cp_enable_ovpgate(CP_ROLE_MASTER, true);
			platform_class_cp_set_qb(CP_ROLE_MASTER, true);
			platform_class_cp_set_charging_enable(CP_ROLE_MASTER, true);

			// close external boost
			msleep(400);
			otg_enable = 0;
			otg_enable = (0 << 16) | (2 << 8) | otg_enable;
			platform_class_buckchg_ops_set_boost_enable(MAIN_BUCK_CHARGER, otg_enable);
		}
	} else {
		mca_log_err("end revert 1_2 cp");

		platform_class_cp_set_charging_enable(CP_ROLE_MASTER, false);
		platform_class_cp_enable_ovpgate(CP_ROLE_MASTER, true);
		platform_class_cp_enable_wpcgate(CP_ROLE_MASTER, false);
		platform_class_cp_set_qb(CP_ROLE_MASTER, false);
		platform_class_cp_set_mode(CP_ROLE_MASTER, CP_MODE_FORWARD_2_1);

		// close otg
		msleep(400);
		otg_enable = 0;
		otg_enable = (0 << 16) | (2 << 8) | otg_enable;
		platform_class_buckchg_ops_set_boost_enable(MAIN_BUCK_CHARGER, otg_enable);

		last_otg_present = false;
		last_pos = 1;
		info->source_boost_status = 0;
	}
	platform_class_cp_dump_register(CP_ROLE_MASTER);
}

static int strategy_buckchg_process_event(int event, int value, void *data)
{
	struct strategy_buckchg_dev *info = data;

	if (!data)
		return -1;

	mca_log_info("receive event %d, value %d\n", event, value);
	switch (event) {
	case MCA_EVENT_USB_CONNECT:
	case MCA_EVENT_USB_DISCONNECT:
		strategy_buckchg_process_online_change(value, info);
		break;
	case MCA_EVENT_CHARGE_TYPE_CHANGE:
		strategy_buckchg_process_type_change(value, info);
		break;
	case MCA_EVENT_CHARGE_CAP_CHANGE:
		strategy_buckchg_process_cap_change(info);
		break;
	case MCA_EVENT_CHARGE_CANCEL_MONITOR_WORK:
		strategy_buckchg_process_cancel_monitor_work(info);
		break;
	case MCA_EVENT_CONN_ANTIBURN_CHANGE:
		strategy_buckchg_process_antiburn_change(value, info);
		break;
	case MCA_EVENT_BATT_BTB_CHANGE:
		strategy_buckchg_process_batt_btb_change(value, info);
		break;
	case MCA_EVENT_BATT_AUTH_PASS:
		mca_log_err("receive batt_auth event, value %d", value);
		info->batt_auth = 1;
		mca_vote(info->input_voltage_voter, "batt_auth", false, 0);
		mca_vote(info->charge_limit_voter, "batt_auth", false, info->chg_batt_auth_failed);
		if (info->proc_data.online)
			mod_delayed_work(system_wq, &info->monitor_work, 0);
		break;
	case MCA_EVENT_CHARGE_ABNORMAL:
		strategy_buckchg_stop_charging(info);
		break;
	case MCA_EVENT_CHARGE_RESTORE:
		(void)platform_class_buckchg_ops_get_online(MAIN_BUCK_CHARGER,
			&info->proc_data.online);
		if (info->proc_data.online)
			strategy_buckchg_start_charging(info);
		break;
	case MCA_EVENT_LPD_STATUS_CHANGE:
		if (value)
			mca_vote(info->input_voltage_voter, "lpd", true, 0);
		else
			mca_vote(info->input_voltage_voter, "lpd", false, 0);
		break;
	case MCA_EVENT_WIRELESS_REVCHG:
		mca_log_info("wireless revchg event %d\n", value);
		info->wls_revchg_en = value;
		if (value)
			strategy_wls_revchg_monitor_workfunc(&info->wls_revchg_monitor_work.work);
		else {
			cancel_delayed_work_sync(&info->wls_revchg_monitor_work);
			strategy_buckchg_exit_wireless_revchg(info);
		}
		break;
	case MCA_EVENT_CHARGE_VERIFY_PROCESS_END:
		mca_log_info("receive pd verify process end event, value %d\n", value);
		info->verify_process_end = value;
		if (info->verify_process_end)
			mca_vote(info->input_limit_voter, "icl_limit", false, STATEGY_INPUT_DEFAULT_VALUE);
		break;
	case MCA_EVENT_CC_SHORT_VBUS:
		if (value)
			mca_vote(info->input_voltage_voter, "cc_short_vbus", true, !value);
		else
			mca_vote(info->input_voltage_voter, "cc_short_vbus", false, !value);
		break;
	case MCA_EVENT_VBAT_OVP_CHANGE:
		if (value)
			mca_vote(info->chg_enable_voter, "vbat_ovp", true, STATEGY_CHARGE_DISENABLE);
		else
			mca_vote(info->chg_enable_voter, "vbat_ovp", false, STATEGY_CHARGE_ENABLE);
		break;
	case MCA_EVENT_CP_CBOOT_FAIL:
		if (value) {
			mca_vote(info->input_voltage_voter, "cp_cboot_short", true, !value);
			(void)platform_class_buckchg_ops_get_online(MAIN_BUCK_CHARGER,
														&info->proc_data.online);
			if (info->proc_data.online)
				strategy_buckchg_start_charging(info);
		} else {
			mca_vote(info->input_voltage_voter, "cp_cboot_short", false, !value);
		}
		break;
	case MCA_EVENT_PPS_PTF:
		info->pps_ptf = value;
		mca_log_info("set PPS_PTF %d\n", info->pps_ptf);
		if (info->pps_ptf == USBPD_BUCK_DPM_PORT_PPS_PTF_NOT_OVERTEMP)
			mca_vote(info->input_voltage_voter, "ptf", true, 1);
		break;
	case MCA_EVENT_IS_EU_MODEL:
		info->is_eu_model = value;
		mca_log_err("set buck is_eu_model %d\n", value);
		platform_class_buckchg_ops_set_eu_model(MAIN_BUCK_CHARGER, value);
		break;
	case MCA_EVENT_PLATE_SHOCK:
		if (value)
			mca_vote(info->input_limit_voter, "plate_shock", true, STAEGY_CHARGE_PLATE_SHOCK);
		else
			mca_vote(info->input_limit_voter , "plate_shock", false, 0);
		break;
	case MCA_EVENT_CSD_SEND_PULSE:
		strategy_buckchg_process_csd_pulse(value, info);
		break;
	case MCA_EVENT_CP_REVERT_CHANGE:
		if (info->support_reverse_quick_charge)
			strategy_buckchg_cp_revert_handler(value, info);
		break;
	case MCA_EVENT_OTG_CONNECT:
		mca_vote(info->input_suppend_voter, "otg_plug_status", true, 1);
		break;
	case MCA_EVENT_OTG_DISCONNECT:
		mca_vote(info->input_suppend_voter, "otg_plug_status", false, 0);
		break;
	default:
		break;
	}

	return 0;
}

static int strategy_buckchg_get_status(int status, void *value, void *data)
{
	struct strategy_buckchg_dev *info = (struct strategy_buckchg_dev *)data;
	int *cur_val = (int *)value;

	if (!info || !value)
		return -1;

	switch (status) {
	case STRATEGY_STATUS_TYPE_ONLINE:
		platform_class_buckchg_ops_get_online(MAIN_BUCK_CHARGER, cur_val);
		break;
	case STRATEGY_STATUS_TYPE_QC_TYPE:
		*cur_val = info->proc_data.qc_type;
		break;
	case STRATEGY_STATUS_TYPE_ENABLE:
		*cur_val = mca_get_effective_result(info->chg_enable_voter);
		break;
	case STRATEGY_STATUS_TYPE_VBUS:
		*cur_val = info->proc_data.vbus;
		break;
	case STRATEGY_STATUS_TYPE_IBUS:
		*cur_val = info->proc_data.ibus;
		break;
	default:
		return -1;
	}

	return 0;
}

#define USB_ICL_UNENUMERATED 100
static int strategy_buckchg_check_charger_change(struct strategy_buckchg_dev *info)
{
#ifdef CONFIG_FACTORY_BUILD
	return 0;
#endif
	static bool sdp_vote_completed = false;
	int charge_boot_mode = mca_log_get_charge_boot_mode();

	/* Release USB related vote if USB detach has been detected */
	if (info->proc_data.real_type == XM_CHARGER_TYPE_UNKNOW) {
		mca_vote(info->input_limit_voter, "usbicl", false, 0);
		mca_vote(info->input_limit_voter, "sdpicl", false, 0);
		sdp_vote_completed = false;
		mca_log_info("charger have been removed reset vote");
		return 0;
	}

	if (charge_boot_mode || !info->is_eu_model) {
		mca_log_info(" charge_boot_mode[%d] cancel sdp enumerated process", charge_boot_mode);
		return 0;
	}

	if (info->proc_data.real_type == XM_CHARGER_TYPE_SDP && !sdp_vote_completed) {
		mca_vote(info->input_limit_voter, "sdpicl", true, USB_ICL_UNENUMERATED);
		sdp_vote_completed = true;
		mca_log_info("sdp charger unenumerated");
	}

	if (info->proc_data.real_type != XM_CHARGER_TYPE_SDP && sdp_vote_completed) {
		mca_vote(info->input_limit_voter, "sdpicl", false, 0);
		mca_vote(info->input_limit_voter, "usbicl", false, 0);
		sdp_vote_completed = false;
		mca_log_info("sdp charger error detected reset vote%d\n", info->proc_data.real_type);
	}


	return 0;
}

#define USB_SUSPEND_ICL 0
#define USB_UNSUSPEND_ICL -1
#define USB_SUSPEND_ICL_DEFAULT 2
static int stategy_buckchg_set_usb_icl(int value, struct strategy_buckchg_dev *info)
{
#ifdef CONFIG_FACTORY_BUILD
	return 0;
#endif
	int icl_ma = value / 1000;//ua switch ma;
	static bool usb_suspend = false;
	int charge_boot_mode = mca_log_get_charge_boot_mode();

	if (charge_boot_mode || !info->is_eu_model) {
		mca_log_info("charge_boot_mode[%d] is_eu_model[%d] exit usb_icl",charge_boot_mode, info->is_eu_model);
		return 0;
	}
	mca_log_info("usb type[%d]icl_ma[%d]", info->proc_data.real_type, icl_ma);
	if (info->proc_data.real_type != XM_CHARGER_TYPE_SDP) {
		if (info->proc_data.real_type == XM_CHARGER_TYPE_PD ||
			info->proc_data.real_type == XM_CHARGER_TYPE_PPS) {
			if (!info->proc_data.pdsuspendsupported) {
				mca_log_info("USB suspend is not supported");
				return 0;
			} else {
				if (icl_ma == USB_SUSPEND_ICL_DEFAULT) {
					icl_ma = 0;
					mca_vote(info->input_limit_voter, "usbicl", true, icl_ma);
					mca_log_info("USB suspend for pd");
				} else if (icl_ma == 900 || icl_ma == 500 || icl_ma == 100) {
					mca_vote(info->input_limit_voter, "usbicl", false, 0);
					mca_log_info("USB Unsuspend for pd");
				} else
					mca_log_info("invalid ICL value for pd %d", icl_ma);
				return 0;
			}
		} else {
			mca_log_info("ICL setting is not allowed for usb type[%d]", info->proc_data.real_type);
			return 0;
		}
	}

	if (icl_ma == USB_SUSPEND_ICL_DEFAULT) {
		mca_log_info("USB Input Suspended");
		mca_vote(info->input_limit_voter, "usbicl", true, 0);
		usb_suspend = true;
		mca_event_block_notify(MCA_EVENT_TYPE_CHARGER_CONNECT, MCA_EVENT_USB_SUSPEND, &usb_suspend);
	} else if (icl_ma == USB_UNSUSPEND_ICL) {
		mca_log_info("USB Input UnSuspended");
		mca_vote(info->input_limit_voter, "usbicl", false, 0);
		if (usb_suspend) {
			usb_suspend = false;
			mca_event_block_notify(MCA_EVENT_TYPE_CHARGER_CONNECT, MCA_EVENT_USB_SUSPEND, &usb_suspend);
		}
	} else {
		if (info->proc_data.real_type == XM_CHARGER_TYPE_SDP &&
			(icl_ma == 900 || icl_ma == 500) ) {
			mca_vote(info->input_limit_voter, "sdpicl", true, icl_ma);
			mca_vote(info->input_limit_voter, "usbicl", true, icl_ma);
			mca_log_info("ICL for SDP set by HLOS is %d mA", icl_ma);
		} else {
			mca_log_info("Invalid ICL for SDP set by HLOS is %d mA", icl_ma);
		}

		if (usb_suspend) {
			usb_suspend = false;
			mca_event_block_notify(MCA_EVENT_TYPE_CHARGER_CONNECT, MCA_EVENT_USB_SUSPEND, &usb_suspend);
		}
	}

	return 0;
}
static int strategy_buckchg_set_config(int config, int value, void *data)
{
	struct strategy_buckchg_dev *info = (struct strategy_buckchg_dev *)data;

	if (!info)
		return -1;

	switch (config) {
	case STRATEGY_CONFIG_INPUT_CURRENT_LIMIT:
		stategy_buckchg_set_usb_icl(value, info);
		break;
	default:
		break;
	}
	return 0;
}

static void strategy_buckchg_update_aicl_cfg(struct strategy_buckchg_dev *info)
{
	int aicl_thd = STATEGY_CHARGE_AICL_TH_4P4V;
	int reg_aicl_thd = 0;

	if (info->proc_data.chg_status == MCA_BUCK_CHG_NO_CHARGING
		|| info->proc_data.chg_status == MCA_BUCK_CHG_STS_CHARGE_DONE) {
		aicl_thd = STATEGY_CHARGE_AICL_TH_4P1V;
		mca_log_info("discharging or done, should set aicl vth to 4.1V\n");
		goto update_aicl_cfg;
	}

	if (info->proc_data.real_type >= XM_CHARGER_TYPE_HVDCP3 &&
		info->proc_data.real_type <= XM_CHARGER_TYPE_HVDCP3P5)
		return;

	if (info->proc_data.vbat >= STATEGY_CHARGE_AICL_VBAT_TH)
		aicl_thd = STATEGY_CHARGE_AICL_TH_4P5V;

#ifdef CONFIG_FACTORY_BUILD
	aicl_thd = STATEGY_CHARGE_AICL_TH_4P1V;
#endif

update_aicl_cfg:
	platform_class_buckchg_ops_get_usb_aicl_cont_thd(MAIN_BUCK_CHARGER, &reg_aicl_thd);
	if (aicl_thd != info->aicl_thd || aicl_thd != reg_aicl_thd) {
		mca_log_info("vbat: %d, aicl_thd: %d, reg_aicl_thd: %d\n", info->proc_data.vbat, aicl_thd, reg_aicl_thd);
		info->aicl_thd = aicl_thd;
		strategy_class_buckchg_ops_set_usb_aicl_cont_thd(info, aicl_thd);
	}
}

static void strategy_buckchg_select_charg_para(struct strategy_buckchg_dev *info)
{
	int ibus_limit = CHARGE_SDP_INPUT_DEFAULT;
	int ibat_limit = CHARGE_SDP_CHARGE_DEFAULT;
	int real_type = info->proc_data.real_type;
	int count = 0;
	int pd_active;
#ifdef CONFIG_FACTORY_BUILD
	bool cc_attach = false;
#endif

	if (info->proc_data.voltage != STATEGY_CHARGE_VBUS_9V &&
		real_type >= XM_CHARGER_TYPE_HVDCP2 && real_type <= XM_CHARGER_TYPE_HVDCP3P5)
		real_type = XM_CHARGER_TYPE_DCP;

#ifdef CONFIG_FACTORY_BUILD
	protocol_class_pd_get_cc_status(TYPEC_PORT_0, &cc_attach);
	if (!cc_attach && (real_type == XM_CHARGER_TYPE_SDP || real_type == XM_CHARGER_TYPE_FLOAT))
		real_type = XM_CHARGER_TYPE_DCP;
	mca_log_err("cc_attach %d real_type %d\n", cc_attach, real_type);
#endif

	switch (real_type) {
	case XM_CHARGER_TYPE_UNKNOW:
		ibus_limit = 100;
		ibat_limit = 100;
		break;
	case XM_CHARGER_TYPE_SDP:
#ifdef CONFIG_FACTORY_BUILD
		ibus_limit = 600;
		ibat_limit = 600;
#else
		if (info->is_eu_model) {
				ibus_limit = info->in_sdp - 50;
		} else {
				ibus_limit = info->in_sdp;
		}
		ibat_limit = info->chg_sdp;
#endif
		break;
	case XM_CHARGER_TYPE_CDP:
		ibus_limit = info->in_cdp;
		ibat_limit = info->chg_cdp;
		break;
	case XM_CHARGER_TYPE_FLOAT:
	case XM_CHARGER_TYPE_OCP:
		ibus_limit = info->in_float;
		ibat_limit = info->chg_float;
		break;
	case XM_CHARGER_TYPE_DCP:
		if (info->smartchg_data.pwr_boost_state) {
			ibus_limit = CHARGE_DCP_INPUT_BOOST;
			ibat_limit = CHARGE_DCP_CHARGE_BOOST;
		} else {
			ibus_limit = info->in_dcp;
			ibat_limit = info->chg_dcp;
		}
		break;
	case XM_CHARGER_TYPE_HVDCP2:
		ibus_limit = info->in_hvdcp;
		ibat_limit = info->chg_hvdcp;
		break;
	case XM_CHARGER_TYPE_HVDCP3:
	case XM_CHARGER_TYPE_HVDCP3_B:
		ibus_limit = info->in_hvdcp3;
		ibat_limit = info->chg_hvdcp3;
		break;
	case XM_CHARGER_TYPE_HVDCP3P5:
		ibus_limit = info->in_hvdcp3p5;
		ibat_limit = info->chg_hvdcp3p5;
		break;
	case XM_CHARGER_TYPE_TYPEC:
		ibus_limit = info->in_dcp;
		ibat_limit = info->chg_dcp;
		break;
	case XM_CHARGER_TYPE_PD:
	case XM_CHARGER_TYPE_PPS:
	case XM_CHARGER_TYPE_PD_VERIFY:
		if (info->pps_ptf == USBPD_BUCK_DPM_PORT_PPS_PTF_NOT_OVERTEMP) {
			ibus_limit = CHARGE_PPS_PTF_INPUT_DEFAULT;
			ibat_limit = CHARGE_PPS_PTF_CHARGE_DEFAULT;
		} else {
			ibus_limit = min(info->in_pd, (unsigned int)info->pwr_cap.cap[info->proc_data.curr_pd_pos].max_current);
			ibat_limit = info->chg_pd;
			if (info->pwr_cap.cap[info->proc_data.curr_pd_pos].max_current == 0) {
				ibus_limit = min(info->in_pd, DEFAULT_PD_CURRENT_MA);
				mca_log_info("pdo broadcast abnormal %d \n", info->proc_data.curr_pd_pos);
			}
		}
		break;
	default:
		ibus_limit = info->in_sdp;
		ibat_limit = info->chg_sdp;
		break;
	};

	protocol_class_pd_get_pd_active(TYPEC_PORT_0, &pd_active);
	if (info->hw_bc12 && !pd_active) {
		do {
			protocol_class_get_bc12_adapter_detect_done(ADAPTER_PROTOCOL_BC12, &info->dpdm_detect_done);
			msleep(20);
			count++;
			mca_log_info("get dpdm_detect_done = %d, count  = %d\n", info->dpdm_detect_done, count);
		} while ((!info->dpdm_detect_done) && (count < 50));//wait 1s until bc1.2 det done
	} else
		info->dpdm_detect_done = true;

	if ((ibus_limit != info->proc_data.ibus_limit) && info->dpdm_detect_done){
		mca_vote(info->buck_5v_in_voter, "wire_chg_type", true, ibus_limit);
		mca_vote(info->buck_9v_in_voter, "wire_chg_type", true, ibus_limit);
		mca_vote(info->input_limit_voter, "subpmic_hw", false, 0); //avoid vote default vaule
		info->proc_data.ibus_limit = ibus_limit;
		mca_log_info("set ibus_limit = %d\n", ibus_limit);
	}

	if ((ibat_limit != info->proc_data.ibat_limit) && info->dpdm_detect_done) {
		mca_vote(info->buck_5v_ich_voter, "wire_chg_type", true, ibat_limit);
		mca_vote(info->buck_9v_ich_voter, "wire_chg_type", true, ibat_limit);
		mca_vote(info->charge_limit_voter, "subpmic_hw", false, 0); //avoid vote default vaule
		info->proc_data.ibat_limit = ibat_limit;
		mca_log_info("set ibat_limit = %d\n", ibat_limit);
	}

	mca_log_info("ibus_limit = %d, ibat_limit = %d, bc1.2 det process = %d, pd_active = %d\n",
				ibus_limit, ibat_limit, info->dpdm_detect_done, pd_active);
}

static void strategy_buckchg_set_charge_volt(struct strategy_buckchg_dev *info, int target_volt)
{
	int volt = target_volt;
	int ret = 0;

	if (!target_volt) {
		mca_log_info("target_volt = 0v is invalid\n");
		return;
	}

	switch (info->proc_data.real_type) {
	case XM_CHARGER_TYPE_HVDCP2:
	case XM_CHARGER_TYPE_HVDCP3:
	case XM_CHARGER_TYPE_HVDCP3_B:
	case XM_CHARGER_TYPE_HVDCP3P5:
		ret = platform_class_cp_check_iic_check(CP_ROLE_MASTER);
		if (ret) {
			volt = STATEGY_CHARGE_VBUS_5V;
		}
		platform_class_buckchg_ops_set_qc_volt(MAIN_BUCK_CHARGER, volt);
		break;
	case XM_CHARGER_TYPE_PD:
		if (info->pwr_cap.nums == 0) {
			mca_log_info("pwr_cap nums is null\n");
			return;
		}
		ret = platform_class_cp_check_iic_check(CP_ROLE_MASTER);
		if (ret) {
			volt = STATEGY_CHARGE_VBUS_5V;
		}
		(void)protocol_class_pd_set_fixed_volt(TYPEC_PORT_0, volt);
		break;
	case XM_CHARGER_TYPE_PPS:
	case XM_CHARGER_TYPE_PD_VERIFY:
		if (info->pwr_cap.nums == 0) {
			mca_log_info("pwr_cap nums is null\n");
			return;
		}
		ret = platform_class_cp_check_iic_check(CP_ROLE_MASTER);
		if (ret) {
			volt = STATEGY_CHARGE_VBUS_5V;
		}
		if(volt == STATEGY_CHARGE_VBUS_5V || volt == STATEGY_CHARGE_VBUS_9V || volt == STATEGY_CHARGE_VBUS_12V){
			(void)protocol_class_pd_set_fixed_volt(TYPEC_PORT_0, volt);
		}else{
			mca_log_info("set no fix pdo\n");
			protocol_class_set_adapter_volt_and_curr(ADAPTER_PROTOCOL_PPS, volt, CHARGE_PPS_INPUT_DEFAULT);
		}
		break;
	default:
		break;
	}
}

static void strategy_buck_update_req_volt(struct strategy_buckchg_dev *info)
{
	switch (info->proc_data.real_type) {
	case XM_CHARGER_TYPE_HVDCP2:
	case XM_CHARGER_TYPE_HVDCP3:
	case XM_CHARGER_TYPE_HVDCP3_B:
	case XM_CHARGER_TYPE_HVDCP3P5:
		mca_vote(info->input_voltage_voter, "real_type", true, 1);
		break;
	case XM_CHARGER_TYPE_PD:
	case XM_CHARGER_TYPE_PPS:
	case XM_CHARGER_TYPE_PD_VERIFY:
		if (info->proc_data.is_pd_9v)
			mca_vote(info->input_voltage_voter, "real_type", true, 1);
		else
			mca_vote(info->input_voltage_voter, "real_type", true, 0);
		break;
	default:
		mca_vote(info->input_voltage_voter, "real_type", true, 0);
		break;
	}
	mca_log_info("real_type = %d, input_voltage_voter effective_result: %d\n",
	info->proc_data.real_type, mca_get_effective_result(info->input_voltage_voter));
}

/*static int strategy_buckchg_check_online_msleep(int ms, struct strategy_buckchg_dev *info)
{
	int i, count;
	count = ms / 10;
	for (i = 0; i < count; i++) {
		if (!info->proc_data.online)
			return -1;
		usleep_range(9900, 11000);
	}
	return 0;
}*/

#define VBUS_9V_OVP_VOLTAGE 10000
#define VBUS_5V_OVP_VOLTAGE 7500
#define PMIC_INDUCTOR_SECURE_ICL 1000
static void strategy_buckchg_check_charge_volt(struct strategy_buckchg_dev *info)
{
	int target_volt = info->proc_data.voltage;
	int vbus = 0;

	if (info->wls_revchg_en) {
		mca_log_info("wireless reverse is charging, do not request volt\n");
		return;
	}

	if ((info->proc_data.real_type >= XM_CHARGER_TYPE_PD &&
		info->proc_data.real_type <= XM_CHARGER_TYPE_PD_VERIFY) && !info->pwr_cap.nums) {
		mca_log_info("pd pwr_cap nums is null\n");
		return;
	}

	platform_class_buckchg_ops_get_bus_volt(MAIN_BUCK_CHARGER, &vbus);
	vbus /= 1000;
	if ((target_volt == STATEGY_CHARGE_VBUS_9V && vbus < CHECK_VBUS_9V_HIGH_TH) ||
		(target_volt == STATEGY_CHARGE_VBUS_5V && vbus > CHECK_VBUS_5V_LOW_TH)) {
		mca_log_info("target_volt: %d, vbus: %d\n", target_volt, vbus);
		strategy_buckchg_set_charge_volt(info, target_volt);
	}

	platform_class_buckchg_ops_get_bus_volt(MAIN_BUCK_CHARGER, &vbus);
	vbus /= 1000;
	if (target_volt == STATEGY_CHARGE_VBUS_9V && vbus >= VBUS_9V_OVP_VOLTAGE) {
		mca_vote(info->buck_9v_in_voter, "pmic_inductor", true, PMIC_INDUCTOR_SECURE_ICL);
		mca_log_err("hvdcp_9v vbus_ovp decrease input current limit\n");
	} else {
		mca_vote(info->buck_9v_in_voter, "pmic_inductor", false, 0);
	}

	if (target_volt == STATEGY_CHARGE_VBUS_5V && vbus > VBUS_5V_OVP_VOLTAGE) {
		mca_vote(info->buck_5v_in_voter, "pmic_inductor", true, PMIC_INDUCTOR_SECURE_ICL);
		mca_log_err("lvdcp_5v vbus_ovp decrease input current limit\n");
	} else {
		mca_vote(info->buck_5v_in_voter, "pmic_inductor", false, 0);
	}

	if (info->is_non_compliant_qc && !info->non_compliant_run_once) {
		mca_log_err("non_compliant_qc: target_volt: %d\n", target_volt);
		strategy_buckchg_set_charge_volt(info, target_volt);
		mca_vote(info->input_limit_voter, "non_compliant_qc", true, 1500);
		strategy_class_buckchg_ops_set_chg(info, false);
		strategy_class_buckchg_ops_set_chg(info, true);
		mca_log_err("restart aicl and recover buck charging\n");
		platform_class_buckchg_ops_set_rerun_aicl(MAIN_BUCK_CHARGER, true);
		info->non_compliant_run_once = true;
	}
}

#define VBUS_QC_VOL_THRESHOLD_LOW 7200
#define NON_COMPLIANT_QC_IBUS_THR_LOW 200
static void strategy_buckchg_check_non_compliant_qc_charger(struct strategy_buckchg_dev *info)
{
	int target_volt = info->proc_data.voltage;
	int vbus = 0, ibus = 0;
	static int count;

	if (target_volt == STATEGY_CHARGE_VBUS_5V || info->is_non_compliant_qc)
		return;

	if (info->proc_data.chg_status == MCA_BUCK_CHG_STS_CHARGE_DONE) {
		mca_log_info("charge done, no need check non_compliant_qc_charger\n");
		return;
	}

	if (info->proc_data.real_type != XM_CHARGER_TYPE_HVDCP2 &&
		info->proc_data.real_type != XM_CHARGER_TYPE_HVDCP3) {
		mca_log_info("only qc2/qc3 need check\n");
		return;
	}

	platform_class_buckchg_ops_get_bus_volt(MAIN_BUCK_CHARGER, &vbus);
	vbus /= 1000;
	(void)platform_class_buckchg_ops_get_bus_curr(MAIN_BUCK_CHARGER, &info->proc_data.ibus);
	ibus = info->proc_data.ibus / 1000;
	if (target_volt == STATEGY_CHARGE_VBUS_9V && vbus < VBUS_QC_VOL_THRESHOLD_LOW
		&& ibus < NON_COMPLIANT_QC_IBUS_THR_LOW) {
		mca_log_info("target_volt: %d, vbus: %d\n", target_volt, vbus);
		count++;
	}
	if (count >= 3 && !info->is_non_compliant_qc) {
		info->is_non_compliant_qc = true;
		count = 0;
		mca_log_err("qc is non-compliant, set request vol to 5V\n");
		mca_vote(info->input_voltage_voter, "non_compliant_qc", true, 0);
	}
}

static int strategy_buckchg_update_charge_status(struct strategy_buckchg_dev *info)
{
	int main_chg_status = 0;
	int aux_chg_status = 0;
	int icl = 0;
	int ichg = 0;
	int suspend = 0;
	int charging_done = 0;
	int adsp_icl = 0;

	(void)platform_class_buckchg_ops_get_chg_status(MAIN_BUCK_CHARGER, &main_chg_status);
	(void)platform_class_buckchg_ops_get_chg_status(AUX_BUCK_CHARGER, &aux_chg_status);
	(void)platform_class_buckchg_ops_get_bus_curr(MAIN_BUCK_CHARGER, &info->proc_data.ibus);
	(void)platform_class_buckchg_ops_get_bus_volt(MAIN_BUCK_CHARGER, &info->proc_data.vbus);
	(void)strategy_class_fg_ops_get_voltage(&info->proc_data.vbat);
	(void)platform_class_buckchg_ops_get_input_curr_lmt(MAIN_BUCK_CHARGER, &adsp_icl);
	icl = mca_get_effective_result(info->input_limit_voter);
	ichg = mca_get_effective_result(info->charge_limit_voter);
	suspend = mca_get_effective_result(info->input_suppend_voter);

	switch (main_chg_status) {
	case MCA_BATT_CHGR_STATUS_INHIBIT:
	case MCA_BATT_CHGR_STATUS_TRICKLE:
	case MCA_BATT_CHGR_STATUS_PRECHARGE:
	case MCA_BATT_CHGR_STATUS_FULLON:
	case MCA_BATT_CHGR_STATUS_TAPER:
	case MCA_BATT_CHGR_STATUS_FAST_LINEAR:
		info->proc_data.chg_status = MCA_BUCK_CHG_STS_CHARGING;
		break;
	case MCA_BATT_CHGR_STATUS_TERMINATION:
		info->proc_data.chg_status =  MCA_BUCK_CHG_STS_CHARGE_DONE;
		break;
	case MCA_BATT_CHGR_STATUS_PAUSE:
	case MCA_BATT_CHGR_STATUS_CHARGING_DISABLED:
		info->proc_data.chg_status = MCA_BUCK_CHG_NO_CHARGING;
		break;
	default:
		info->proc_data.chg_status = MCA_BUCK_CHG_STS_NA;
		break;
	}

	charging_done = strategy_class_fg_ops_get_charging_done();
	if (charging_done)
		info->proc_data.chg_status =  MCA_BUCK_CHG_STS_CHARGE_DONE;

	mca_log_err("pmic_chg_status: %d, chg_status: %d, chg_en: [%d][%s], chg_type: %d, vbat: %d, ibat: %d, vbus: %d, ibus: %d\n",
		main_chg_status, info->proc_data.chg_status,
		mca_get_effective_result(info->chg_enable_voter), mca_get_effective_client(info->chg_enable_voter),
		info->proc_data.real_type, info->proc_data.vbat, info->proc_data.ibat, info->proc_data.vbus, info->proc_data.ibus);
	mca_log_err("ap_icl:[%s][%d], adsp_icl[%d], ichg:[%s][%d], input_suspend:[%s][%d]\n",
		mca_get_effective_client(info->input_limit_voter), icl, adsp_icl,
		mca_get_effective_client(info->charge_limit_voter), ichg,
		mca_get_effective_client(info->input_suppend_voter), suspend);
	return info->proc_data.chg_status;
}

static void strategy_buckchg_enable_fast_charge_mode(struct strategy_buckchg_dev *info, int soc)
{
	int batt_temp, fastcharge_mode, buck_jeita_ffc_size;
	int iterm = mca_get_effective_result(info->iterm_voter);
	int fcc = mca_get_effective_result(info->charge_limit_voter);
	int quick_charge_status = MCA_QUICK_CHG_STS_CHARGE_FAILED;

	if (info->proc_data.real_type == XM_CHARGER_TYPE_PD_VERIFY) {
		(void)mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_QUICK_CHARGE,
				STRATEGY_STATUS_TYPE_CHARGING, &quick_charge_status);
		(void)strategy_class_fg_ops_get_temperature(&batt_temp);
		batt_temp /= 10;

		mca_log_info("batt_temp = %d, soc =%d, fcc = %d, iterm = %d, quickchg_sts = %d\n",
			batt_temp, soc, fcc, iterm, quick_charge_status);

		mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_JEITA,
			STRATEGY_STATUS_TYPE_JEITA_FFC_SIZE, &buck_jeita_ffc_size);
		fastcharge_mode = strategy_class_fg_get_fastcharge();
		if (!fastcharge_mode && soc < ALLOW_START_FFC_BATT_SOC_THR
			&& batt_temp >= info->ffc_temp_low && batt_temp <= info->ffc_temp_high && info->batt_auth
			&& buck_jeita_ffc_size) {
			strategy_class_fg_set_fastcharge(true);
			mca_log_info("buck charger enable fast charge mode\n");
		} else if (batt_temp < info->ffc_temp_low
			|| batt_temp > info->ffc_temp_high) {
			strategy_class_fg_set_fastcharge(false);
			mca_log_info("buck charger disable fast charge mode\n");
		} else if (fastcharge_mode && soc >= ALLOW_START_FFC_BATT_SOC_THR
			&& fcc <= iterm && quick_charge_status != MCA_QUICK_CHG_STS_CHARGING) {
			strategy_class_fg_set_fastcharge(false);
			mca_log_info("buck charger disable fast charge mode by fcc too low\n");
		}
	}
}

static void strategy_buckchg_eoc_force_5v(struct strategy_buckchg_dev *info)
{
	mca_vote(info->input_voltage_voter, "eoc_5v", true, 0);
}

#define COLD_ZONE_LOW -100
#define COLD_ZONE_HIGH 50
#define HOT_ZONE_LOW 480
#define HOT_ZONE_HIGH 550
#define IS_BETWEEN(val, lval, rval)	((val >= lval) ? ((val <= rval) ? true :  false) : \
								((val >= rval) ? true : false))
static int strategy_buckchg_charge_abnormal_cold_or_hot_zone(struct strategy_buckchg_dev *info)
{
	int icl = 0;
	int chg_en = 0;
	int temp = 0;
	int vterm = 0;
	static int count = 0;

	strategy_class_fg_ops_get_temperature(&temp);
	if ( !IS_BETWEEN (temp, COLD_ZONE_LOW, COLD_ZONE_HIGH) &&
		!IS_BETWEEN (temp, HOT_ZONE_LOW, HOT_ZONE_HIGH)) {
		mca_log_info("temp: %d, not in cold or hot zone\n", temp);
		return 0;
	}

	icl = mca_get_effective_result(info->input_limit_voter);
	chg_en = mca_get_effective_result(info->chg_enable_voter);
	vterm = mca_get_effective_result(info->vterm_voter);
	if (!icl || !chg_en || info->proc_data.vbat >= vterm) {
		mca_log_info("user can stop charging icl: %d, chg_en: %d, vterm: %d, vbat: %d, temp:%d\n",
			icl, chg_en, vterm, info->proc_data.vbat, temp);
		return 0;
	}

	if (info->proc_data.chg_status == MCA_BUCK_CHG_NO_CHARGING) {
		/* temp ranges aren't overlaping, so using the same count variable is safe */
		if (IS_BETWEEN (temp, COLD_ZONE_LOW, COLD_ZONE_HIGH)) {
			count++;
			if (count > 3) {
				count = 0;
				mca_charge_mievent_report(CHARGE_DFX_LOW_TEMP_DISCHARGING, &temp, 1);
			}
		} else if (IS_BETWEEN (temp, HOT_ZONE_LOW, HOT_ZONE_HIGH)) {
			count++;
			if (count > 3) {
				count = 0;
				mca_charge_mievent_report(CHARGE_DFX_HIGH_TEMP_DISCHARGING, &temp, 1);
			}
		} else {
			count = 0;
		}
		mca_log_info("count: %d\n", count);
	} else {
		count = 0;
	}

	return 0;
}

#define STRATEGY_BUCKCHG_ENTER_QUICKCHG_TIME_MS 7000
#define VBAT_DROP_COUNT_TH 3
static void strategy_buckchg_monitor_workfunc(struct work_struct *work)
{
	struct strategy_buckchg_dev *info = container_of(work,
				struct strategy_buckchg_dev, monitor_work.work);
	int interval = CHARGE_MONITOR_WORK_NORMAL_INTERVAL;
	int quick_charge_status = MCA_QUICK_CHG_STS_CHARGE_FAILED;
	int input_suspned = 0;
	int chg_en = 0, system_soc;
	int vterm = 0;
	int jeita_hot_result = 1;
	ktime_t time_now;
	int verifed = 0;
	bool vbat_drop_exit_flag = false;
	static int vbat_drop_cnt = 0;
	int active_port = protocol_class_pd_get_port_num();

	system_soc = strategy_class_fg_ops_get_soc();
	if (system_soc <= ALLOW_QUICK_CHG_SOC_THR) {
		jeita_hot_result = mca_get_client_vote(info->chg_enable_voter, "jeita-hot");
		mca_log_info("jeita_hot vote value: %d\n", jeita_hot_result);
		switch (info->proc_data.real_type) {
		case XM_CHARGER_TYPE_HVDCP3_B:
		case XM_CHARGER_TYPE_HVDCP3P5:
		case XM_CHARGER_TYPE_PD_VERIFY:
			if (jeita_hot_result)
				mca_strategy_func_process(STRATEGY_FUNC_TYPE_QUICK_CHARGE,
					MCA_EVENT_CHARGE_ACTION, 0);
			break;
		case XM_CHARGER_TYPE_PPS:
			time_now = ktime_get_boottime();
			protocol_class_pd_get_pd_verifed(active_port, &verifed);
			if (info->is_eu_model) {
				mca_log_info("eu_start_time %lld, time_now %lld  delta = %lld\n",info->proc_data.eu_start_time , time_now,
					ktime_ms_delta(time_now, info->proc_data.eu_start_time));
				if (ktime_ms_delta(time_now, info->proc_data.eu_start_time) > STRATEGY_BUCKCHG_ENTER_QUICKCHG_TIME_MS) {
					mca_strategy_func_process(STRATEGY_FUNC_TYPE_QUICK_CHARGE,
					MCA_EVENT_CHARGE_ACTION, 0);
					info->proc_data.eu_start_time = time_now;
					mca_log_info("trigger MCA_EVENT_CHARGE_ACTION\n");
				}
			} else {
				if (jeita_hot_result && info->verify_process_end && !verifed) {
					mca_strategy_func_process(STRATEGY_FUNC_TYPE_QUICK_CHARGE,
						MCA_EVENT_CHARGE_ACTION, 0);
				}
				mca_log_info("verify_process_end: %d, verifed: %d\n", info->verify_process_end, verifed);
			}
			break;
		default:
			break;
		}

	} else {
		strategy_buckchg_enable_fast_charge_mode(info, system_soc);
	}

	(void)mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_QUICK_CHARGE,
				STRATEGY_STATUS_TYPE_CHARGING, &quick_charge_status);
	if (quick_charge_status == MCA_QUICK_CHG_STS_CHARGING)
		goto out;

	strategy_buckchg_check_charge_volt(info);
	strategy_buckchg_select_charg_para(info);
	strategy_buckchg_update_charge_status(info);
	strategy_buckchg_check_non_compliant_qc_charger(info);
	strategy_buckchg_update_aicl_cfg(info);
	strategy_buckchg_limit_full_replug_ichg(info, false);
	strategy_buckchg_resume_buck_ichg_limit(info);
	if (info->proc_data.chg_status == MCA_BUCK_CHG_STS_CHARGING) {
		if (info->proc_data.charge_done_force_5v) {
			info->proc_data.charge_done_force_5v = false;
			mca_vote(info->input_voltage_voter, "eoc_5v", false, 0);
		}
		// WA: rerun aicl if ibus too low
		if (info->proc_data.ibus < 150000 && mca_get_effective_result(info->input_limit_voter) >= 500) {
			mca_log_info("ibus abnormally low: %d, rerun aicl\n", info->proc_data.ibus);
			platform_class_buckchg_ops_set_rerun_aicl(MAIN_BUCK_CHARGER, true);
		}
	}
	if ((info->proc_data.chg_status == MCA_BUCK_CHG_STS_CHARGE_DONE) &&
		!info->proc_data.charge_done_force_5v) {
		strategy_buckchg_sw_cv_stop(info);
		strategy_buckchg_eoc_force_5v(info);
		info->proc_data.charge_done_force_5v = true;
	}
	if (!info->proc_data.chg_en) {
		mca_vote(info->chg_enable_voter, "online", true, STATEGY_CHARGE_ENABLE);
		info->proc_data.chg_en = STATEGY_CHARGE_ENABLE;
	}

	// WA: solve pmic abnormal status
	vterm = mca_get_effective_result(info->vterm_voter);
	input_suspned = mca_get_effective_result(info->input_suppend_voter);
	chg_en = mca_get_effective_result(info->chg_enable_voter);
	if (vterm >= info->sw_cv_vterm_th && !input_suspned) {
		if (chg_en && info->proc_data.chg_status == MCA_BUCK_CHG_NO_CHARGING) {
			strategy_class_buckchg_ops_set_chg(info, false);
			strategy_class_buckchg_ops_set_chg(info, true);
			mca_log_err("recover buck charging\n");
		}
	}
	strategy_buckchg_charge_abnormal_cold_or_hot_zone(info);

	if(info->proc_data.vbat <= vterm - (20 * info->vbat_fg_to_pmic_ratio)) {
		vbat_drop_cnt++;
	}
	vbat_drop_exit_flag = vbat_drop_cnt >= VBAT_DROP_COUNT_TH ? true : false;
	vbat_drop_cnt = vbat_drop_exit_flag ? 0 : vbat_drop_cnt;

	if (info->proc_data.chg_status == MCA_BUCK_CHG_STS_CHARGING &&
		!info->sw_cv_running && vterm >= info->sw_cv_vterm_th &&
		info->proc_data.vbat >= vterm - CHARGE_SW_CV_VBAT_ALARM_DELTA) {
		mca_log_err("vbat: %d, vterm: %d, start sw_cv_work\n", info->proc_data.vbat, vterm);
		strategy_buckchg_sw_cv_start(info);
	} else if (info->sw_cv_running && (!chg_en || vbat_drop_exit_flag)) {
		mca_log_err("vbat: %d, vterm: %d, chg_en: %d, stop sw_cv_work\n", info->proc_data.vbat, vterm, chg_en);
		strategy_buckchg_sw_cv_stop(info);
	}

	if (quick_charge_status == MCA_QUICK_CHG_STS_CHARGE_DONE)
		interval = CHARGE_MONITOR_WORK_FAST_INTERVAL;

out:
	schedule_delayed_work(&info->monitor_work, msecs_to_jiffies(interval));
}

static void strategy_buckchg_sw_cv_start(struct strategy_buckchg_dev *info)
{
	info->sw_cv_running = true;
	mca_vote(info->charge_limit_voter, "sw_cv", false, 0);
	mca_queue_delayed_work(&info->sw_cv_work, msecs_to_jiffies(CHARGE_SW_CV_WORK_FAST_INTERVAL));
}

static void strategy_buckchg_sw_cv_stop(struct strategy_buckchg_dev *info)
{
	mca_log_err("chg_status: %d, stop sw_cv_work\n", info->proc_data.chg_status);
	cancel_delayed_work_sync(&info->sw_cv_work);
	mca_vote(info->charge_limit_voter, "sw_cv", false, 0);
	info->sw_cv_running = false;
	info->vbat_ov_count = 0;
}

#define FCC_STEP 50
#define FV_STEP 5
static void strategy_buckchg_sw_cv_workfunc(struct work_struct *work)
{
	struct strategy_buckchg_dev *info = container_of(work,
		struct strategy_buckchg_dev, sw_cv_work.work);
	static int sw_cv_volt_delta_map[][2] = {
		{1000, 5},
		{0, 2}
	};
	int interval = CHARGE_SW_CV_WORK_NORMAL_INTERVAL;
	int vbat, ibat;
	int vterm = mca_get_effective_result(info->vterm_voter);
	int iterm = mca_get_effective_result(info->iterm_voter);
	int fcc = mca_get_effective_result(info->charge_limit_voter);
	int volt_delta = sw_cv_volt_delta_map[0][1];

	strategy_class_fg_ops_get_voltage(&info->proc_data.vbat);
	strategy_class_fg_ops_get_current(&info->proc_data.ibat);
	vbat = info->proc_data.vbat;
	ibat = -info->proc_data.ibat / 1000;
	mca_log_info("vbat: %d, ibat: %d, vterm: %d, iterm: %d, fcc: %d\n",
		vbat, ibat, vterm, iterm, fcc);

	for (int i = 0; i < sizeof(sw_cv_volt_delta_map) / sizeof(sw_cv_volt_delta_map[0]); i++) {
		if (ibat > sw_cv_volt_delta_map[i][0]) {
			volt_delta = sw_cv_volt_delta_map[i][1];
			break;
		}
	}

	if (vbat >= vterm - volt_delta) {
		interval = CHARGE_SW_CV_WORK_FAST_INTERVAL;
		if (ibat - FCC_STEP > iterm) {
			if (fcc - ibat >= 2 * FCC_STEP)
				mca_vote(info->charge_limit_voter, "sw_cv", true, ibat / FCC_STEP * FCC_STEP);
			else
				mca_vote(info->charge_limit_voter, "sw_cv", true, fcc - FCC_STEP);
		}
	} else {
		interval = CHARGE_SW_CV_WORK_NORMAL_INTERVAL;
	}

	if (vbat >= (vterm - 1)) {
		mca_log_err("WARNING: batt ov, reduce fv\n");
		++info->vbat_ov_count;
		if (info->vbat_ov_count == 1)
			platform_class_buckchg_ops_set_term_volt(MAIN_BUCK_CHARGER, vterm + info->pmic_fv_compensation - FV_STEP);
		else if (info->vbat_ov_count >= 2)
			platform_class_buckchg_ops_set_term_volt(MAIN_BUCK_CHARGER, vterm + info->pmic_fv_compensation - 2 * FV_STEP);
	}

	mca_queue_delayed_work(&info->sw_cv_work, msecs_to_jiffies(interval));
}

static int strategy_buckchg_wireless_revchg_msleep(int ms, struct strategy_buckchg_dev *info)
{
	int i, count;

	count = ms / 10;

	for (i = 0; i < count; i++) {
		if (!info->proc_data.online || !info->wls_revchg_en)
			return -1;
		usleep_range(9900, 11000);
	}

	return 0;
}

static void strategy_buckchg_exit_wireless_revchg(struct strategy_buckchg_dev *info)
{
	mca_vote(info->input_limit_voter, "wireless_revchg", false, 0);
	info->proc_data.wls_revchg_init_done = false;
	info->rev_icl_for_qc2 = false;
}

static int strategy_buckchg_process_wireless_revchg(struct strategy_buckchg_dev *info)
{
	int real_type = info->proc_data.real_type;
	int vbus = 0, cnt = 0;
	int rev_req_vadp = 0, req_volt_valid_h = 0, req_volt_valid_l = 0;
	int ret = 0;

	if (real_type < XM_CHARGER_TYPE_DCP || real_type > XM_CHARGER_TYPE_PD_VERIFY)
		return -1;

	if (real_type == XM_CHARGER_TYPE_PD) {
		if (strategy_buckchg_wireless_revchg_msleep(WLS_REVCHG_NORMAL_INTERVAL, info))
			return -1;
	} else if (real_type == XM_CHARGER_TYPE_DCP) {
		if (strategy_buckchg_wireless_revchg_msleep(WLS_REVCHG_SLOW_INTERVAL, info))
			return -1;
	} else if (real_type == XM_CHARGER_TYPE_HVDCP3) {
		if (strategy_buckchg_wireless_revchg_msleep(WLS_REVCHG_FAST_INTERVAL, info))
			return -1;
	}

	real_type = info->proc_data.real_type;
	mca_vote(info->input_limit_voter, "wireless_revchg", true, CHARGE_WLS_REVCHG_INPUT_DEFAULT);
	if (real_type == XM_CHARGER_TYPE_PPS || real_type == XM_CHARGER_TYPE_PD_VERIFY) {
		rev_req_vadp = info->rev_req_vadp[0];
		req_volt_valid_h = info->rev_vadp_valid_h[0] * 1000;
		req_volt_valid_l = info->rev_vadp_valid_l[0] * 1000;
	} else {
		rev_req_vadp = info->rev_req_vadp[1];
		req_volt_valid_h = info->rev_vadp_valid_h[1] * 1000;
		req_volt_valid_l = info->rev_vadp_valid_l[1] * 1000;
	}
	strategy_buckchg_set_charge_volt(info, rev_req_vadp);

	if (strategy_buckchg_wireless_revchg_msleep(300, info))
		return -1;
	(void)platform_class_buckchg_ops_get_bus_volt(MAIN_BUCK_CHARGER, &vbus);
	mca_log_info("chg_type: %d, req_vadp: %d, vbus: %d, rev_vadp_valid_h: %d, rev_vadp_valid_l: %d\n",
		real_type, rev_req_vadp, vbus, req_volt_valid_h, req_volt_valid_l);
	while (vbus < req_volt_valid_l || vbus > req_volt_valid_h) {
		if (cnt > 5)
			break;
		if (strategy_buckchg_wireless_revchg_msleep(300, info))
			return -1;
		(void)platform_class_buckchg_ops_get_bus_volt(MAIN_BUCK_CHARGER, &vbus);
		cnt++;
	}
	if (cnt > 5) {
		mca_log_err("request 9V vbus fail\n");
		return -1;
	}

	info->proc_data.wls_revchg_init_done = true;
	ret = mca_wireless_rev_set_wired_chg_ok(true);
	return ret;
}

static void strategy_wls_revchg_monitor_workfunc(struct work_struct *work)
{
	struct strategy_buckchg_dev *info = container_of(work,
				struct strategy_buckchg_dev, wls_revchg_monitor_work.work);
	int ret = 0;
	int interval = CHARGE_MONITOR_WORK_NORMAL_INTERVAL;

	if (!info->proc_data.wls_revchg_init_done)
		ret = strategy_buckchg_process_wireless_revchg(info);
	if (ret || !info->wls_revchg_en)
		goto err_out;
	else {
		if (info->proc_data.real_type == XM_CHARGER_TYPE_HVDCP2 && !info->rev_icl_for_qc2) {
			mca_vote(info->input_limit_voter, "wireless_revchg", true, CHARGE_WLS_REVCHG_INPUT_QC2);
			info->rev_icl_for_qc2 = true;
		}
		schedule_delayed_work(&info->wls_revchg_monitor_work,
			msecs_to_jiffies(interval));
		return;
	}

err_out:
	strategy_buckchg_exit_wireless_revchg(info);
}

static void strategy_csd_pulse_process_workfunc(struct work_struct *work)
{
	struct strategy_buckchg_dev *info = container_of(work,
				struct strategy_buckchg_dev, csd_pulse_process_work.work);

	if (info->csd_flag) {
		mca_vote(info->charge_limit_voter, "csd_pulse", false, 0);
		mca_vote(info->chg_enable_voter, "csd_pulse", true, STATEGY_CHARGE_DISENABLE);
		info->csd_flag = false;
		schedule_delayed_work(&info->csd_pulse_process_work, msecs_to_jiffies(75000));
		mca_log_info("vote fcc and discharge logic\n");
	} else {
		mca_vote(info->chg_enable_voter, "csd_pulse", false, STATEGY_CHARGE_DISENABLE);
		mca_strategy_func_process(STRATEGY_FUNC_TYPE_QUICK_CHARGE,
			MCA_EVENT_CSD_SEND_PULSE, 0);
		mca_log_info("resume charging logic\n");
	}
}

static void strategy_source_status_monitor_workfunc(struct work_struct *work)
{
	struct strategy_buckchg_dev *info = container_of(work,
				struct strategy_buckchg_dev, source_status_monitor_work.work);
	int batt_temp = 0;
	int system_soc = 0;
	int status = 0;

	(void)strategy_class_fg_ops_get_temperature(&batt_temp);
	system_soc = strategy_class_fg_ops_get_soc();
	batt_temp /= 10;
	mca_log_info("batt_temp = %d, thermal_board_temp =%d\n", batt_temp, info->thermal_board_temp);

	if (batt_temp < 0 || info->thermal_board_temp > 400 ||
		(info->cp_revert_auth && system_soc < 30) ||
		(!info->cp_revert_auth && system_soc < 70))
		status = 1;

	if (status != info->source_boost_status) {
		info->source_boost_status = status;
		protocol_class_pd_set_gear_shift(TYPEC_PORT_0, info->source_boost_status);
	}

	schedule_delayed_work(&info->source_status_monitor_work,
		msecs_to_jiffies(SOURCE_STATUS_MONITOR_INTERVAL));
}

static void strategy_buckchg_check_pdsecret_workfunc(struct work_struct *work)
{
	struct strategy_buckchg_dev *info = container_of(work,
				struct strategy_buckchg_dev, check_pd_secret_work.work);

	if (!info->verify_process_end) {
		mca_vote(info->input_limit_voter, "icl_limit", false, STATEGY_INPUT_DEFAULT_VALUE);
		info->verify_process_end = 1;
		mca_log_info("cancel icl_limit vote and pd_end flag when using pd charger\n");
	}
}

#define NTC_SCALE_BOARDTEMP 100
static int strategy_buckchg_thermal_notifier_cb(struct notifier_block *nb,
			unsigned long event, void *val)
{
	struct strategy_buckchg_dev *info = container_of(nb,
				struct strategy_buckchg_dev, thermal_board_nb);

	switch (event) {
	case MCA_EVENT_THERMAL_BOARD_TEMP_CHANGE:
		info->thermal_board_temp  = *(int *)val/NTC_SCALE_BOARDTEMP;
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}

static int strategy_buckchg_if_set_chg_cur(const char *user,
	char *value, void *data)
{
	int temp_value = 0;
	struct strategy_buckchg_dev *info = data;

	if (!user || !value || !data)
		return -1;

	if (kstrtoint(value, 0, &temp_value))
		return -1;

	if (temp_value)
		(void)mca_vote(info->charge_limit_voter, user, true, temp_value);
	else
		(void)mca_vote(info->charge_limit_voter, user, false, 0);

	return 0;
}

static int strategy_buckchg_if_get_chg_cur(char *buf, void *data)
{
	const char *client_str;
	int value;
	struct strategy_buckchg_dev *info = data;

	if (!buf || !data)
		return -1;

	client_str = mca_get_effective_client(info->charge_limit_voter);
	if (!client_str)
		return -1;
	value = mca_get_effective_result(info->charge_limit_voter);

	scnprintf(buf, MCA_CHARGE_IF_MAX_VALUE_BUFF, "eff_client:%s %d",
		client_str, value);

	return 0;
}


#define MTBF_CLIENT_SUBSTR_UPPER "MTBF\0"
#define MTBF_CLIENT_SUBSTR_LOW "mtbf\0"
#define MTBF_ACTIVE_TEST_CLIENT_SUBSTR "shell\0"
#define MTBF_TEST_MAX_CURRENT_MA 1500
static int strategy_buckchg_if_set_input_cur(const char *user,
	char *value, void *data)
{
	int temp_value = 0;
	struct strategy_buckchg_dev *info = data;

	if (!user || !value || !data)
		return -1;

	if (kstrtoint(value, 0, &temp_value))
		return -1;

	if (strstr(user, MTBF_ACTIVE_TEST_CLIENT_SUBSTR) != NULL) {
			if (temp_value > MTBF_TEST_MAX_CURRENT_MA)
				temp_value = MTBF_TEST_MAX_CURRENT_MA;
	}

	if (temp_value) {
		if (strstr(user, MTBF_CLIENT_SUBSTR_UPPER) != NULL ||
			strstr(user, MTBF_CLIENT_SUBSTR_LOW) != NULL ||
			strstr(user, MTBF_ACTIVE_TEST_CLIENT_SUBSTR) != NULL)
			mca_vote_override(info->input_limit_voter, user, true, temp_value);
		else
			(void)mca_vote(info->input_limit_voter, user, true, temp_value);
	} else {
		if (strstr(user, MTBF_CLIENT_SUBSTR_UPPER) != NULL || strstr(user, MTBF_CLIENT_SUBSTR_LOW) != NULL)
			mca_vote_override(info->input_limit_voter, user, false, 0);
		else
			(void)mca_vote(info->input_limit_voter, user, false, 0);
	}

	return 0;
}

static int strategy_buckchg_if_get_input_cur(char *buf, void *data)
{
	const char *client_str;
	int value;
	struct strategy_buckchg_dev *info = data;

	if (!buf || !data)
		return -1;

	client_str = mca_get_effective_client(info->input_limit_voter);
	if (!client_str)
		return -1;
	value = mca_get_effective_result(info->input_limit_voter);

	scnprintf(buf, MCA_CHARGE_IF_MAX_VALUE_BUFF, "eff_client:%s %d",
		client_str, value);

	return 0;
}

static int strategy_buckchg_if_set_chg_en(const char *user,
	unsigned int value, void *data)
{
	struct strategy_buckchg_dev *info = data;

	if (!user || !data)
		return -1;

	if (value)
		(void)mca_vote(info->chg_enable_voter, user,
			true, STATEGY_CHARGE_ENABLE);
	else
		(void)mca_vote(info->chg_enable_voter, user,
			true, STATEGY_CHARGE_DISENABLE);

	return 0;
}

static int strategy_buckchg_if_get_chg_en(char *buf, void *data)
{
	const char *client_str;
	int value;
	struct strategy_buckchg_dev *info = data;

	if (!buf || !data)
		return -1;

	client_str = mca_get_effective_client(info->chg_enable_voter);
	if (!client_str)
		return -1;
	value = mca_get_effective_result(info->chg_enable_voter);

	scnprintf(buf, MCA_CHARGE_IF_MAX_VALUE_BUFF, "eff_client:%s %d",
		client_str, value);

	return 0;
}

static int strategy_buckchg_if_get_input_suspend(char *buf, void *data)
{
	const char *client_str;
	int value;
	struct strategy_buckchg_dev *info = data;

	if (!buf || !data)
		return -1;

	client_str = mca_get_effective_client(info->input_suppend_voter);
	if (!client_str)
		return -1;
	value = mca_get_effective_result(info->input_suppend_voter);

	scnprintf(buf, MCA_CHARGE_IF_MAX_VALUE_BUFF, "eff_client:%s %d",
		client_str, value);

	return 0;
}

static int strategy_buckchg_if_set_input_suspend(const char *user,
	char *value, void *data)
{
	int temp_value = 0;
	struct strategy_buckchg_dev *info = data;

	if (!user || !value || !data)
		return -1;

	if (kstrtoint(value, 0, &temp_value))
		return -1;

	mca_log_err("set_input_suspend: %d\n", temp_value);
	if (temp_value)
		(void)mca_vote(info->input_suppend_voter, user, true, 1);
	else
		(void)mca_vote(info->input_suppend_voter, user, true, 0);

	return 0;
}

static int strategy_buckchg_if_set_ship_mode(const char *user,
	unsigned int val, void *data)
{
	int rc;
	struct strategy_buckchg_dev *info = data;
	mca_log_err("set shipmode chip:%d val:%d\n",info->ship_mode_chip, val);

	rc = platform_class_buckchg_ops_set_ship_mode(info->ship_mode_chip, !!val);
	if (rc < 0)
		return rc;

	return 0;
}

static int strategy_buckchg_if_get_ship_mode(bool *val, void *data)
{
	int rc;
	struct strategy_buckchg_dev *info = data;

	rc = platform_class_buckchg_ops_get_ship_mode(info->ship_mode_chip, val);
	if (rc < 0)
		return rc;

	return 0;
}

static struct mca_charge_if_ops g_strategy_buckchg_if_ops = {
	.type_name = "buck",
	.set_input_suspend = strategy_buckchg_if_set_input_suspend,
	.get_input_suspend = strategy_buckchg_if_get_input_suspend,
	.set_charge_enable = strategy_buckchg_if_set_chg_en,
	.get_charge_enable = strategy_buckchg_if_get_chg_en,
	.set_input_current_limit = strategy_buckchg_if_set_input_cur,
	.get_input_current_limit = strategy_buckchg_if_get_input_cur,
	.set_charge_current_limit = strategy_buckchg_if_set_chg_cur,
	.get_charge_current_limit = strategy_buckchg_if_get_chg_cur,
	.set_ship_mode_en = strategy_buckchg_if_set_ship_mode,
	.get_ship_mode_status = strategy_buckchg_if_get_ship_mode,
};

static int strategy_buckchg_soc_limit_sts_callback(void *data, int effective_result)
{
	struct strategy_buckchg_dev *info = (struct strategy_buckchg_dev *)data;

	if (!data)
		return -1;

	if (info->proc_data.online)
		strategy_buckchg_process_soc_limit_change(effective_result, info);
	else
		strategy_buckchg_process_soc_limit_change(0, info);

	mca_log_info("effective_result: %d\n", effective_result);

	return 0;
}

static int mca_charger_buckchg_pwr_boost_sts_callback(void *data, int enable)
{
	struct strategy_buckchg_dev *info = (struct strategy_buckchg_dev *)data;

	if (!data)
		return -1;

	info->smartchg_data.pwr_boost_state = enable;

	return 0;
}

static struct mca_smartchg_if_ops g_buck_smartchg_if_ops = {
	.type = MCA_SMARTCHG_IF_CHG_TYPE_BUCK,
	.data = NULL,
	.set_soc_limit_sts = strategy_buckchg_soc_limit_sts_callback,
	.set_pwr_boost_sts = mca_charger_buckchg_pwr_boost_sts_callback,
};

static int strategy_buckchg_class_probe(struct platform_device *pdev)
{
	struct strategy_buckchg_dev *info;
	int online;
	int ret;

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->dev = &pdev->dev;
	strategy_buckchg_parse_dt(info);
	ret = strategy_buckchg_init_voter(info);
	if (ret) {
		mca_log_err("init voter err\n");
		return -1;
	}

	mca_vote(info->input_voltage_voter, "batt_auth", true, 0);
	mca_vote(info->charge_limit_voter, "batt_auth", true, info->chg_batt_auth_failed);
	info->proc_data.voltage = STATEGY_CHARGE_VBUS_5V;
	info->hvdcp_allow_flag = 0;
	info->vbat_ov_count = 0;
	INIT_DELAYED_WORK(&info->monitor_work, strategy_buckchg_monitor_workfunc);
	INIT_DELAYED_WORK(&info->sw_cv_work, strategy_buckchg_sw_cv_workfunc);
	INIT_DELAYED_WORK(&info->wls_revchg_monitor_work, strategy_wls_revchg_monitor_workfunc);
	INIT_DELAYED_WORK(&info->csd_pulse_process_work, strategy_csd_pulse_process_workfunc);
	INIT_DELAYED_WORK(&info->source_status_monitor_work, strategy_source_status_monitor_workfunc);
	INIT_DELAYED_WORK(&info->check_pd_secret_work, strategy_buckchg_check_pdsecret_workfunc);
	(void)mca_strategy_ops_register(STRATEGY_FUNC_TYPE_BUCK_CHARGE,
		strategy_buckchg_process_event, strategy_buckchg_get_status, strategy_buckchg_set_config, info);
	g_strategy_buckchg_if_ops.data = info;
	(void)mca_charge_if_ops_register(&g_strategy_buckchg_if_ops);
	g_buck_smartchg_if_ops.data = info;
	(void)mca_smartchg_if_ops_register(&g_buck_smartchg_if_ops);
	info->thermal_board_nb.notifier_call = strategy_buckchg_thermal_notifier_cb;
	mca_event_block_notify_register(MCA_EVENT_TYPE_THERMAL_TEMP, &info->thermal_board_nb);

	platform_class_buckchg_ops_get_online(MAIN_BUCK_CHARGER, &online);
	if (online) {
		mca_log_info("avoid missing first usb connect event\n");
		mca_event_block_notify(MCA_EVENT_TYPE_CHARGER_CONNECT, MCA_EVENT_USB_CONNECT, NULL);
	}

	g_buckchg_info = info;
	mca_log_err("androidboot.mode=%d\n", mca_log_get_charge_boot_mode());

	mca_log_err("probe success\n");
	return 0;
}

static int strategy_buckchg_class_remove(struct platform_device *pdev)
{
	return 0;
}

static void strategy_buckchg_class_shutdown(struct platform_device *pdev)
{
}

static const struct of_device_id match_table[] = {
	{.compatible = "mca,strategy_buckchg"},
	{},
};

static struct platform_driver strategy_buckchg_class_driver = {
	.driver	= {
		.name = "strategy_buckchg_class",
		.owner = THIS_MODULE,
		.of_match_table = match_table,
	},
	.probe = strategy_buckchg_class_probe,
	.remove = strategy_buckchg_class_remove,
	.shutdown = strategy_buckchg_class_shutdown,
};

static int __init strategy_buckchg_class_init(void)
{
	return platform_driver_register(&strategy_buckchg_class_driver);
}
module_init(strategy_buckchg_class_init);

static void __exit strategy_buckchg_class_exit(void)
{
	platform_driver_unregister(&strategy_buckchg_class_driver);
}
module_exit(strategy_buckchg_class_exit);


MODULE_DESCRIPTION("strategy buckchg class");
MODULE_AUTHOR("liyuze1@xiaomi.com");
MODULE_LICENSE("GPL v2");
