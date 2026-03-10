// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*
 *
 * Filename:
 * ---------
 *   mtk_charger.c
 *
 * Project:
 * --------
 *   Android_Software
 *
 * Description:
 * ------------
 *   This Module defines functions of Battery charging
 *
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/power_supply.h>
#include <linux/pm_wakeup.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/scatterlist.h>
#include <linux/suspend.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/reboot.h>
#include <asm/setup.h>
#include "mtk_charger.h"
#include "mtk_battery.h"
#include <tcpm.h>
#include "middleware/battery_secret_manager/inc/battery_secret_class.h"

/* sw jeita */
#define JEITA_TEMP_ABOVE_T6_CV	4100000
#define JEITA_TEMP_T5_TO_T6_CV	4100000
#define JEITA_TEMP_T4_TO_T5_CV	4480000
#define JEITA_TEMP_T3_TO_T4_CV	4480000
#define JEITA_TEMP_T2_TO_T3_CV	4500000
#define JEITA_TEMP_T1_TO_T2_CV	4500000
#define JEITA_TEMP_T0_TO_T1_CV	4500000
#define JEITA_TEMP_BELOW_T0_CV	4500000
#define NORMAL_JEITA_TEMP_CV	4480000
#define JEITA_CV_COMPENSATION	200000  //uV

#define TEMP_T6_THRES  60
#define TEMP_T6_THRES_MINUS_X_DEGREE 58
#define TEMP_T5_THRES  45
#define TEMP_T5_THRES_MINUS_X_DEGREE 43
#define TEMP_T4_THRES  35
#define TEMP_T4_THRES_MINUS_X_DEGREE 33
#define TEMP_T3_THRES  15
#define TEMP_T3_THRES_MINUS_X_DEGREE 13
#define TEMP_T2_THRES  10
#define TEMP_T2_THRES_PLUS_X_DEGREE 8
#define TEMP_T1_THRES  5
#define TEMP_T1_THRES_PLUS_X_DEGREE 3
#define TEMP_T0_THRES  0
#define TEMP_T0_THRES_PLUS_X_DEGREE  -2
#define TEMP_NEG_10_THRES  -10
#define TEMP_NEG_10_THRES_PLUS_X_DEGREE  -8

#define JEITA_CURRENT_T5_TO_T6	2935000
#define JEITA_CURRENT_T4_TO_T5	6000000
#define JEITA_CURRENT_T3_TO_T4	6000000
#define JEITA_CURRENT_T2_TO_T3	4696000
#define JEITA_CURRENT_T1_TO_T2	2935000
#define JEITA_CURRENT_T0_TO_T1	1174000
#define JEITA_CURRENT_BELOW_T0	587000
#define JEITA_INPUT_CURRENT_MAX	3000000
#define SINGLE_CELL_MODE_IEOC	240000	//uA

/* ffc charger cycle reduce battery voltage algorithm */
#define MAX_BATTERY_ID	3
#define NORMAL_TEMP_REGION	2
#define FCC_JEITA_STEP_MAX	3
#define BATTERY_CYCLE_STEP_MAX	4

int sw_jeita_fcc_cv[BATTERY_CYCLE_STEP_MAX][FCC_JEITA_STEP_MAX] = {
	/* cv1,     cv2,    cv3 uV */
	{4250000, 4500000, 4530000},/*0-99cls*/
	{4220000, 4470000, 4510000},/*100-299cls*/
	{4220000, 4460000, 4500000},/*300-799cls*/
	{4220000, 4460000, 4500000} /*>800cls*/
};

int sw_jeita_fcc_cc[BATTERY_CYCLE_STEP_MAX][FCC_JEITA_STEP_MAX] = {
	/* cc1,     cc2,    cc3 uA */
	{5950000, 5400000, 4696000},/*0-99cls*/
	{5950000, 5400000, 4696000},/*100-299cls*/
	{5950000, 5400000, 4696000},/*300-799cls*/
	{4800000, 4320000, 3759000} /*>800cls*/
};

int sw_jeita_fcc_ieoc[NORMAL_TEMP_REGION][MAX_BATTERY_ID] = {
	/*COS,     NVT,    XWD uA*/
	{1174000, 1292000, 1292000},/* TEMP_T3_TO_T4 */
	{1468000, 1584000, 1762000} /* TEMP_T4_TO_T5 */
};

int sw_jeita_normal_ieoc[MAX_BATTERY_ID] = {
	/*COS,   NVT,    XWD uA*/
	420000, 480000, 420000
};

static void select_jeita_ieoc(struct mtk_charger *info)
{
	struct sw_jeita_data *sw_jeita;
	int ieoc;
	int id = 0;
	int ret = 0;

	sw_jeita = &info->sw_jeita;
	sw_jeita->pre_sm = sw_jeita->sm;

	ret = lc_get_battery_id();
	if (ret <= 0 || ret > MAX_BATTERY_ID) {
		chr_err("[SW_JEITA] get batt_id fail,use battery0 id\n");
		id = 1;
	} else {
		chr_info("[SW_JEITA] get batt_id succ\n");
		id = ret;
	}
	chr_info("[SW_JEITA] battery_id = %d\n", id);
	if ((sw_jeita->sm == TEMP_T3_TO_T4 || sw_jeita->sm == TEMP_T4_TO_T5) && info->bat.ffc) {
		ieoc = sw_jeita_fcc_ieoc[sw_jeita->sm - 4][id - 1];
	} else {
		ieoc = sw_jeita_normal_ieoc[id - 1];
		//charger_dev_set_eoc_current(info->chg1_dev, ieoc);
	}
	if (info->enable_single_cell_mode) {
		ieoc = SINGLE_CELL_MODE_IEOC;
		//charger_dev_set_eoc_current(info->chg1_dev, ieoc);
	}
  	charger_dev_set_eoc_current(info->chg1_dev, ieoc);
	info->bat.iterm = ieoc / 1000;
	chr_info("[SW_JEITA] ieoc = %d\n", ieoc);
}

static void select_fcc_jeita_cc_cv(struct mtk_charger *info)
{
	struct sw_jeita_data *sw_jeita;
	struct power_supply *bat_psy;
	union power_supply_propval val;
	int vbat_low_step = 5000; //uV
	int vbat = 0;
	int cycle_count_step = 0;
	int ret;

	sw_jeita = &info->sw_jeita;
	sw_jeita->pre_sm = sw_jeita->sm;

	vbat = get_battery_voltage(info) * 1000;
	chr_info("[SW_JEITA] vbat = %d\n", vbat);

	bat_psy = power_supply_get_by_name("battery");
	if (!IS_ERR_OR_NULL(bat_psy)) {
		ret = power_supply_get_property(bat_psy, POWER_SUPPLY_PROP_CYCLE_COUNT, &val);
		if (ret < 0) {
			chr_err("%s: failed to get bat cycle, ret=%d\n", __func__, ret);
		} else {
			info->bat.cycle = val.intval;
		}
	}

	if (info->bat.cycle < 100) {
		cycle_count_step = 0;
	} else if (info->bat.cycle < 300) {
		cycle_count_step = 1;
	} else if (info->bat.cycle < 800) {
		cycle_count_step = 2;
	} else if (info->bat.cycle >= 800) {
		cycle_count_step = 3;
	}

	if (info->state_jeita == 0) {
		if (vbat < sw_jeita_fcc_cv[cycle_count_step][0]) {
			info->state_jeita = 1;
		} else if (vbat < sw_jeita_fcc_cv[cycle_count_step][1]) {
			info->state_jeita = 2;
		} else if (vbat < sw_jeita_fcc_cv[cycle_count_step][2]) {
			info->state_jeita = 3;
		}
	}

	if(info->sw_jeita.sm == TEMP_T3_TO_T4 ||
		info->sw_jeita.sm == TEMP_T4_TO_T5) {
		if ((vbat < sw_jeita_fcc_cv[cycle_count_step][0]) && (info->state_jeita == 1)) {
			info->setting.fcc_cv= sw_jeita_fcc_cv[cycle_count_step][2];
			sw_jeita->cc = sw_jeita_fcc_cc[cycle_count_step][0];
			info->state_jeita = 2;
		} else if ((vbat >= (sw_jeita_fcc_cv[cycle_count_step][0] - vbat_low_step)
			&& vbat < sw_jeita_fcc_cv[cycle_count_step][1])
			&& (info->state_jeita == 2)) {
			info->setting.fcc_cv = sw_jeita_fcc_cv[cycle_count_step][2];
			sw_jeita->cc = sw_jeita_fcc_cc[cycle_count_step][1];
			info->state_jeita = 3;
		} else if ((vbat >= (sw_jeita_fcc_cv[cycle_count_step][1] - vbat_low_step)
			&& vbat < sw_jeita_fcc_cv[cycle_count_step][2])
			&& (info->state_jeita == 3)) {
			info->setting.fcc_cv = sw_jeita_fcc_cv[cycle_count_step][2];
			sw_jeita->cc = sw_jeita_fcc_cc[cycle_count_step][2];
		}
		sw_jeita->cv = sw_jeita_fcc_cv[cycle_count_step][2];
	} else if (info->sw_jeita.sm == TEMP_T2_TO_T3) {
		info->setting.fcc_cv = JEITA_TEMP_T2_TO_T3_CV;
		sw_jeita->cc = JEITA_CURRENT_T2_TO_T3;
	}
	chr_info("[SW_JEITA] state_jeita = %d, cycle_count_step = %d, fcc_cv = %d, fcc_cc = %d\n", info->state_jeita, cycle_count_step, info->setting.fcc_cv, sw_jeita->cc);
}

void sw_jeita_get_bat_info(struct mtk_charger *info)
{
	struct sw_jeita_data *sw_jeita;
	struct battery_info *bat = &(info->bat);
	struct power_supply *bat_psy;
	union power_supply_propval val;
	int pd_type = MTK_CAP_TYPE_UNKNOWN;
	int ret;

	sw_jeita = &info->sw_jeita;
	sw_jeita->pre_sm = sw_jeita->sm;
	bat->temp = info->battery_temp;
	bat->uisoc = get_uisoc(info);

	bat_psy = power_supply_get_by_name("battery");
	if (!IS_ERR_OR_NULL(bat_psy)) {
		ret = power_supply_get_property(bat_psy, POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT, &val);
		if (ret < 0) {
			chr_err("%s: failed to get bat cycle, ret=%d\n", __func__, ret);
		} else {
			info->bat.thermal_lv = val.intval;
		}
	}

	pd_type = adapter_dev_get_property(info->adapter_dev[PD], CAP_TYPE);
	if ((sw_jeita->sm >= TEMP_T2_TO_T3 && sw_jeita->sm <= TEMP_T4_TO_T5) &&
		info->ta_status[PD] == TA_ATTACH &&
		pd_type ==  MTK_PD_APDO &&
		info->batt_verify &&
		info->bat.thermal_lv <= 10) {
		if (bat->uisoc < 90 && bat->ffc == false) {
			bat->ffc = true;
		}
	} else {
			bat->ffc = false;
	}
	if (info->enable_single_cell_mode) {
		bat->ffc = false;
	}
	if (bat->ffc_disable)
		bat->ffc = false;
	chr_info("[SW_JEITA] ffc_disable = %d, ffc = %d, pd_type = %d, thermal_lv = %d, single_cell_mode = %d\n", info->bat.ffc_disable, bat->ffc, pd_type, info->bat.thermal_lv, info->enable_single_cell_mode);
}

static void sw_jeita_prevent_battery_ovp(struct mtk_charger *info)
{
	struct sw_jeita_data *sw_jeita;
	int batt_voltage = 0;

	sw_jeita = &info->sw_jeita;
	sw_jeita->pre_sm = sw_jeita->sm;

	batt_voltage = get_battery_voltage(info) * 1000;
	chr_info("[SW_JEITA] %s: vbat = %d\n", __func__, batt_voltage);

	if (batt_voltage >= (info->data.jeita_temp_above_t6_cv + 100000)) { //batt_voltage >= (4.1+0.01)V
		sw_jeita->charging = false;
		sw_jeita->cv = NORMAL_JEITA_TEMP_CV;
	} else {
		sw_jeita->cv = info->data.jeita_temp_t5_to_t6_cv;
	}
}

void do_sw_jeita_state_machine(struct mtk_charger *info)
{
	struct sw_jeita_data *sw_jeita;

	sw_jeita = &info->sw_jeita;
	sw_jeita->pre_sm = sw_jeita->sm;
	sw_jeita->charging = true;

	/* JEITA battery temp Standard */
	if (info->battery_temp >= info->data.temp_t6_thres) {  //60
		chr_info("[SW_JEITA] Battery Over high Temperature(%d) !!\n",
			info->data.temp_t6_thres);   //60

		sw_jeita->sm = TEMP_ABOVE_T6;
		sw_jeita->charging = false;
	} else if (info->battery_temp > info->data.temp_t5_thres) {   //45
		/* control 45 degree to normal behavior */
		if ((sw_jeita->sm == TEMP_ABOVE_T6)
		    && (info->battery_temp
			>= info->data.temp_t6_thres_minus_x_degree)) {        //58
			chr_info("[SW_JEITA] Battery Temperature between %d and %d,not allow charging yet!!\n",
				info->data.temp_t6_thres_minus_x_degree,          //58
				info->data.temp_t6_thres);                        //60

			sw_jeita->charging = false;
		} else {
			chr_info("[SW_JEITA] Battery Temperature between %d and %d !!\n",
				info->data.temp_t5_thres,                         //45
				info->data.temp_t6_thres);                        //60
			sw_jeita->charging = true;
			sw_jeita->sm = TEMP_T5_TO_T6;
		}
	}else if (info->battery_temp > info->data.temp_t4_thres) {   //35
		/* control 45 degree to normal behavior */
		if ((sw_jeita->sm == TEMP_T5_TO_T6)
		    && (info->battery_temp
			>= info->data.temp_t5_thres_minus_x_degree)) {  //43
			chr_info("[SW_JEITA] Battery Temperature between %d and %d\n",
				info->data.temp_t5_thres_minus_x_degree,      //43
				info->data.temp_t5_thres);                    //45
			sw_jeita->charging = true;
		} else {
			chr_info("[SW_JEITA] Battery Temperature between %d and %d !!\n",
				info->data.temp_t4_thres,                     //35
				info->data.temp_t5_thres);                    //45
			sw_jeita->charging = true;
			sw_jeita->sm = TEMP_T4_TO_T5;
		}
	} else if (info->battery_temp > info->data.temp_t3_thres) {  //15
		/* control 20 degree to normal behavior */
		if ((sw_jeita->sm == TEMP_T4_TO_T5)
		    && (info->battery_temp
			>= info->data.temp_t4_thres_minus_x_degree))        //33
		{
			chr_info("[SW_JEITA] Battery Temperature between %d and %d !!\n",
				info->data.temp_t4_thres_minus_x_degree,       //33
				info->data.temp_t4_thres);                    //35
		} else {
			chr_info("[SW_JEITA] Battery Temperature between %d and %d !!\n",
				info->data.temp_t3_thres,                      //15
				info->data.temp_t4_thres);                     //35
			sw_jeita->sm = TEMP_T3_TO_T4;
		}
	} else if (info->battery_temp >= info->data.temp_t2_thres) {  //10 
		/* control 15 degree to normal behavior */
		if ((sw_jeita->sm == TEMP_T3_TO_T4)
		     && (info->battery_temp
			 >= info->data.temp_t3_thres_minus_x_degree)) {       //13
			chr_info("[SW_JEITA] Battery Temperature between %d and %d !!\n",
				info->data.temp_t3_thres_minus_x_degree,         //13
				info->data.temp_t3_thres);                       //15
		} else {
			chr_info("[SW_JEITA] Battery Temperature between %d and %d !!\n",
				info->data.temp_t2_thres,                        //10
				info->data.temp_t3_thres);                       //15
			sw_jeita->sm = TEMP_T2_TO_T3;
		}
	}else if (info->battery_temp >= info->data.temp_t1_thres) {  //5
		/* control 15 degree to normal behavior */
		if ((sw_jeita->sm == TEMP_T2_TO_T3)
		     && (info->battery_temp
			 >= info->data.temp_t2_thres_plus_x_degree)) {       //8
			chr_info("[SW_JEITA] Battery Temperature between %d and %d !!\n",
				info->data.temp_t2_thres_plus_x_degree,         //8
				info->data.temp_t2_thres);                       //10
		} else {
			chr_info("[SW_JEITA] Battery Temperature between %d and %d !!\n",
				info->data.temp_t1_thres,                        //5
				info->data.temp_t2_thres);                       //10
			sw_jeita->sm = TEMP_T1_TO_T2;
		}
	} else if (info->battery_temp >= info->data.temp_t0_thres) {  //0
		/* control 10 degree to normal behavior */
		if ((sw_jeita->sm == TEMP_T1_TO_T2)
		    && (info->battery_temp
			>= info->data.temp_t1_thres_plus_x_degree)) {       //3
			chr_info("[SW_JEITA] Battery Temperature between %d and %d !!\n",
				info->data.temp_t1_thres_plus_x_degree,         //3
				info->data.temp_t1_thres);                      //5
			} else {
				chr_info("[SW_JEITA] Battery Temperature between %d and %d\n",
					info->data.temp_t0_thres,
					info->data.temp_t1_thres);
				sw_jeita->charging = true;
				sw_jeita->sm = TEMP_T0_TO_T1;
			}
	} else if (info->battery_temp >= info->data.temp_neg_10_thres) {  //-10
		/* control 0 degree to normal behavior */
		if ((sw_jeita->sm == TEMP_T0_TO_T1)
		    && (info->battery_temp
			>= info->data.temp_t0_thres_plus_x_degree)) {             //-2
			chr_info("[SW_JEITA] Battery Temperature between %d and %d\n",
				info->data.temp_t0_thres_plus_x_degree,
				info->data.temp_t0_thres);
			sw_jeita->charging = true;
		} else if((sw_jeita->sm == TEMP_BELOW_NEG_10) && (info->battery_temp
			<= info->data.temp_neg_10_thres_plus_x_degree)){
			chr_info("[SW_JEITA] Battery Temperature between %d and %d ,not allow charging!!\n",
				info->data.temp_neg_10_thres,                       //-10
				info->data.temp_neg_10_thres_plus_x_degree);        //-8
			sw_jeita->charging = false;
		} else {
			chr_info("[SW_JEITA] Battery Temperature between %d and %d \n",
				info->data.temp_neg_10_thres,                       //-10
				info->data.temp_t0_thres);                          //0
			sw_jeita->charging = true;
			sw_jeita->sm = TEMP_BELOW_T0;
		}
	} else {
		chr_info("[SW_JEITA] Battery below low Temperature(%d) !!\n",
			info->data.temp_neg_10_thres);
		sw_jeita->sm = TEMP_BELOW_NEG_10;
		sw_jeita->charging = false;
	}

	//sw_jeita_get_bat_info(info);
    if (info->bat.ffc) {
        select_fcc_jeita_cc_cv(info);
    } else {
        info->state_jeita = 0;
		/* set CV after temperature changed */
		/* In normal range, we adjust CV dynamically */
		if (sw_jeita->sm != TEMP_T2_TO_T3) {
			if ((sw_jeita->sm == TEMP_ABOVE_T6) || (sw_jeita->sm == TEMP_T5_TO_T6))
				sw_jeita_prevent_battery_ovp(info);
			else if (sw_jeita->sm == TEMP_T4_TO_T5)
				sw_jeita->cv = info->data.jeita_temp_t4_to_t5_cv;
			else if (sw_jeita->sm == TEMP_T3_TO_T4)
				sw_jeita->cv = info->data.jeita_temp_t3_to_t4_cv;
			else if (sw_jeita->sm == TEMP_T2_TO_T3)
				sw_jeita->cv = info->data.jeita_temp_t2_to_t3_cv;
			else if (sw_jeita->sm == TEMP_T1_TO_T2)
				sw_jeita->cv = info->data.jeita_temp_t1_to_t2_cv;
			else if (sw_jeita->sm == TEMP_T0_TO_T1)
				sw_jeita->cv = info->data.jeita_temp_t0_to_t1_cv;
			else if (sw_jeita->sm == TEMP_BELOW_T0)
				sw_jeita->cv = info->data.jeita_temp_below_t0_cv;
			else
				sw_jeita->cv = info->data.battery_cv;
		} else {
			sw_jeita->cv = info->data.jeita_temp_t2_to_t3_cv;
		}
		sw_jeita->cv -= (info->cycle_fv * 1000);
	}

	if(info->is_basic)
		info->state_jeita = 0;

	info->pe50_start_flag = info->bat.ffc;
	if (info->smart_fv > 0) {
		if (sw_jeita->sm >= TEMP_T5_TO_T6) {
			chr_info("[%s]is high tmep, ignore smart_fv\n", __func__);
		} else if ((sw_jeita->sm == TEMP_T4_TO_T5) && (sw_jeita->cv <= NORMAL_JEITA_TEMP_CV)) {//in nomal cycle fv
			sw_jeita->cv = min((sw_jeita->cv / 1000), (NORMAL_JEITA_TEMP_CV / 1000 - info->smart_fv)) * 1000;
		} else {
			sw_jeita->cv -= (info->smart_fv * 1000);
		}
	}

	select_jeita_ieoc(info);

	if (sw_jeita->cv != JEITA_TEMP_T5_TO_T6_CV && sw_jeita->sm > TEMP_T2_TO_T3) { //ffc charging cut-off too early, set cv+= 100 mV
		charger_dev_set_constant_voltage(info->chg1_dev, sw_jeita->cv + JEITA_CV_COMPENSATION);
	} else {
		charger_dev_set_constant_voltage(info->chg1_dev, sw_jeita->cv);
	}

	chr_info("[SW_JEITA]tmp:%d jeita_cv:%d jeita_cc:%d\n",
			info->battery_temp, sw_jeita->cv, sw_jeita->cc);
}

int jeita_current_limit(struct mtk_charger *info)
{
	if(IS_ERR_OR_NULL(info)) {
		return false;
	}

	if (info->enable_sw_jeita) {
		if (IS_ENABLED(CONFIG_USBIF_COMPLIANCE)
			&& info->chr_type == POWER_SUPPLY_TYPE_USB) {
			chr_debug("USBIF & STAND_HOST skip current check\n");
		} else {
			/* Normal jeita icl set 3000ma,final icl use chg_type icl */
			if (info->sw_jeita.sm == TEMP_BELOW_T0) {
				info->jeita_input_current_limit = JEITA_INPUT_CURRENT_MAX;
				info->jeita_charging_current_limit = info->data.jeita_current_below_t0;
			}else if (info->sw_jeita.sm == TEMP_T0_TO_T1) {
				info->jeita_input_current_limit = JEITA_INPUT_CURRENT_MAX;
				info->jeita_charging_current_limit = info->data.jeita_current_t0_to_t1;
			} else if (info->sw_jeita.sm == TEMP_T1_TO_T2) {
				info->jeita_input_current_limit = JEITA_INPUT_CURRENT_MAX;
				info->jeita_charging_current_limit = info->data.jeita_current_t1_to_t2;
			} else if (info->sw_jeita.sm == TEMP_T2_TO_T3) {
				info->jeita_input_current_limit = JEITA_INPUT_CURRENT_MAX;
				info->jeita_charging_current_limit = info->data.jeita_current_t2_to_t3;
			} else if (info->sw_jeita.sm == TEMP_T3_TO_T4) {
				info->jeita_input_current_limit = JEITA_INPUT_CURRENT_MAX;
				info->jeita_charging_current_limit = info->data.jeita_current_t3_to_t4;
			} else if (info->sw_jeita.sm == TEMP_T4_TO_T5) {
				info->jeita_input_current_limit = JEITA_INPUT_CURRENT_MAX;
				info->jeita_charging_current_limit = info->data.jeita_current_t4_to_t5;
			}else if (info->sw_jeita.sm == TEMP_T5_TO_T6) {
				info->jeita_input_current_limit = JEITA_INPUT_CURRENT_MAX;
				info->jeita_charging_current_limit = info->data.jeita_current_t5_to_t6;
			} else {
				info->jeita_input_current_limit = JEITA_INPUT_CURRENT_MAX;
				info->jeita_charging_current_limit = 0;
			}
		}
	}

	chr_info("%s: jeita_input_current_limit: %d, jeita_charging_current_limit: %d\n",
				__func__,
				info->jeita_input_current_limit,
				info->jeita_charging_current_limit);

	return true;
}

void lc_jeita_parse_dt(struct mtk_charger *info,
				struct device *dev)
{
	struct device_node *np = dev->of_node;
	u32 val = 0;

	/* sw jeita */
	info->enable_sw_jeita = of_property_read_bool(np, "enable_sw_jeita");
	if (of_property_read_u32(np, "jeita_temp_above_t6_cv", &val) >= 0)
		info->data.jeita_temp_above_t6_cv = val;
	else {
		chr_err("use default JEITA_TEMP_ABOVE_T6_CV:%d\n",
			JEITA_TEMP_ABOVE_T6_CV);
		info->data.jeita_temp_above_t6_cv = JEITA_TEMP_ABOVE_T6_CV;
	}
	if (of_property_read_u32(np, "jeita_temp_t5_to_t6_cv", &val) >= 0)
                info->data.jeita_temp_t5_to_t6_cv = val;
    else {
                chr_err("use default JEITA_TEMP_T5_TO_T6_CV:%d\n",
                        JEITA_TEMP_T5_TO_T6_CV);
                info->data.jeita_temp_t5_to_t6_cv = JEITA_TEMP_T5_TO_T6_CV;
    }
	if (of_property_read_u32(np, "jeita_temp_t4_to_t5_cv", &val) >= 0)
                info->data.jeita_temp_t4_to_t5_cv = val;
    else {
                chr_err("use default JEITA_TEMP_T4_TO_T5_CV:%d\n",
                        JEITA_TEMP_T4_TO_T5_CV);
                info->data.jeita_temp_t4_to_t5_cv = JEITA_TEMP_T4_TO_T5_CV;
    }

	if (of_property_read_u32(np, "jeita_temp_t3_to_t4_cv", &val) >= 0)
		info->data.jeita_temp_t3_to_t4_cv = val;
	else {
		chr_err("use default JEITA_TEMP_T3_TO_T4_CV:%d\n",
			JEITA_TEMP_T3_TO_T4_CV);
		info->data.jeita_temp_t3_to_t4_cv = JEITA_TEMP_T3_TO_T4_CV;
	}

	if (of_property_read_u32(np, "jeita_temp_t2_to_t3_cv", &val) >= 0)
		info->data.jeita_temp_t2_to_t3_cv = val;
	else {
		chr_err("use default JEITA_TEMP_T2_TO_T3_CV:%d\n",
			JEITA_TEMP_T2_TO_T3_CV);
		info->data.jeita_temp_t2_to_t3_cv = JEITA_TEMP_T2_TO_T3_CV;
	}

	if (of_property_read_u32(np, "jeita_temp_t1_to_t2_cv", &val) >= 0)
		info->data.jeita_temp_t1_to_t2_cv = val;
	else {
		chr_err("use default JEITA_TEMP_T1_TO_T2_CV:%d\n",
			JEITA_TEMP_T1_TO_T2_CV);
		info->data.jeita_temp_t1_to_t2_cv = JEITA_TEMP_T1_TO_T2_CV;
	}

	if (of_property_read_u32(np, "jeita_temp_t0_to_t1_cv", &val) >= 0)
		info->data.jeita_temp_t0_to_t1_cv = val;
	else {
		chr_err("use default JEITA_TEMP_T0_TO_T1_CV:%d\n",
			JEITA_TEMP_T0_TO_T1_CV);
		info->data.jeita_temp_t0_to_t1_cv = JEITA_TEMP_T0_TO_T1_CV;
	}

	if (of_property_read_u32(np, "jeita_temp_below_t0_cv", &val) >= 0)
		info->data.jeita_temp_below_t0_cv = val;
	else {
		chr_err("use default JEITA_TEMP_BELOW_T0_CV:%d\n",
			JEITA_TEMP_BELOW_T0_CV);
		info->data.jeita_temp_below_t0_cv = JEITA_TEMP_BELOW_T0_CV;
	}

	if (of_property_read_u32(np, "temp_t6_thres", &val) >= 0)
			info->data.temp_t6_thres = val;
	else {
			chr_err("use default TEMP_T6_THRES:%d\n",
						TEMP_T6_THRES);
			info->data.temp_t6_thres = TEMP_T6_THRES;
	}

	if (of_property_read_u32(np, "temp_t6_thres_minus_x_degree", &val) >= 0)
				info->data.temp_t6_thres_minus_x_degree = val;
	else {
				chr_err("use default TEMP_T6_THRES_MINUS_X_DEGREE:%d\n",
						TEMP_T6_THRES_MINUS_X_DEGREE);
				info->data.temp_t6_thres_minus_x_degree =
										TEMP_T6_THRES_MINUS_X_DEGREE;
	}

	if (of_property_read_u32(np, "temp_t5_thres", &val) >= 0)
			info->data.temp_t5_thres = val;
 	else {
			chr_err("use default TEMP_T5_THRES:%d\n",
 	 	 	 	 	 	TEMP_T5_THRES);
 	 	 	info->data.temp_t5_thres = TEMP_T5_THRES;
 	}

 	if (of_property_read_u32(np, "temp_t5_thres_minus_x_degree", &val) >= 0)
 	 	 	 	info->data.temp_t5_thres_minus_x_degree = val;
 	else {
 	 	 	 	chr_err("use default TEMP_T5_THRES_MINUS_X_DEGREE:%d\n",
						TEMP_T5_THRES_MINUS_X_DEGREE);
 	 	 	 	info->data.temp_t5_thres_minus_x_degree =
 	 	 	 	 	 	 	 	 	 	TEMP_T5_THRES_MINUS_X_DEGREE;
	}

	if (of_property_read_u32(np, "temp_t4_thres", &val) >= 0)
		info->data.temp_t4_thres = val;
	else {
		chr_err("use default TEMP_T4_THRES:%d\n",
			TEMP_T4_THRES);
		info->data.temp_t4_thres = TEMP_T4_THRES;
	}

	if (of_property_read_u32(np, "temp_t4_thres_minus_x_degree", &val) >= 0)
		info->data.temp_t4_thres_minus_x_degree = val;
	else {
		chr_err("use default TEMP_T4_THRES_MINUS_X_DEGREE:%d\n",
			TEMP_T4_THRES_MINUS_X_DEGREE);
		info->data.temp_t4_thres_minus_x_degree =
					TEMP_T4_THRES_MINUS_X_DEGREE;
	}

	if (of_property_read_u32(np, "temp_t3_thres", &val) >= 0)
		info->data.temp_t3_thres = val;
	else {
		chr_err("use default TEMP_T3_THRES:%d\n",
			TEMP_T3_THRES);
		info->data.temp_t3_thres = TEMP_T3_THRES;
	}

	if (of_property_read_u32(np, "temp_t3_thres_minus_x_degree", &val) >= 0)
		info->data.temp_t3_thres_minus_x_degree = val;
	else {
		chr_err("use default TEMP_T3_THRES_MINUS_X_DEGREE:%d\n",
			TEMP_T3_THRES_MINUS_X_DEGREE);
		info->data.temp_t3_thres_minus_x_degree =
					TEMP_T3_THRES_MINUS_X_DEGREE;
	}

	if (of_property_read_u32(np, "temp_t2_thres", &val) >= 0)
		info->data.temp_t2_thres = val;
	else {
		chr_err("use default TEMP_T2_THRES:%d\n",
			TEMP_T2_THRES);
		info->data.temp_t2_thres = TEMP_T2_THRES;
	}

	if (of_property_read_u32(np, "temp_t2_thres_plus_x_degree", &val) >= 0)
		info->data.temp_t2_thres_plus_x_degree = val;
	else {
		chr_err("use default TEMP_T2_THRES_PLUS_X_DEGREE:%d\n",
			TEMP_T2_THRES_PLUS_X_DEGREE);
		info->data.temp_t2_thres_plus_x_degree =
					TEMP_T2_THRES_PLUS_X_DEGREE;
	}

	if (of_property_read_u32(np, "temp_t1_thres", &val) >= 0)
		info->data.temp_t1_thres = val;
	else {
		chr_err("use default TEMP_T1_THRES:%d\n",
			TEMP_T1_THRES);
		info->data.temp_t1_thres = TEMP_T1_THRES;
	}

	if (of_property_read_u32(np, "temp_t1_thres_plus_x_degree", &val) >= 0)
		info->data.temp_t1_thres_plus_x_degree = val;
	else {
		chr_err("use default TEMP_T1_THRES_PLUS_X_DEGREE:%d\n",
			TEMP_T1_THRES_PLUS_X_DEGREE);
		info->data.temp_t1_thres_plus_x_degree =
					TEMP_T1_THRES_PLUS_X_DEGREE;
	}

	if (of_property_read_u32(np, "temp_t0_thres", &val) >= 0)
		info->data.temp_t0_thres = val;
	else {
		chr_err("use default TEMP_T0_THRES:%d\n",
			TEMP_T0_THRES);
		info->data.temp_t0_thres = TEMP_T0_THRES;
	}

	if (of_property_read_u32(np, "temp_t0_thres_plus_x_degree", &val) >= 0)
		info->data.temp_t0_thres_plus_x_degree = val;
	else {
		chr_err("use default TEMP_T0_THRES_PLUS_X_DEGREE:%d\n",
			TEMP_T0_THRES_PLUS_X_DEGREE);
		info->data.temp_t0_thres_plus_x_degree =
					TEMP_T0_THRES_PLUS_X_DEGREE;
	}
	if (of_property_read_u32(np, "temp_neg_10_thres", &val) >= 0)
		info->data.temp_neg_10_thres = val;
	else {
		chr_err("use default TEMP_NEG_10_THRES:%d\n",
			TEMP_NEG_10_THRES);
		info->data.temp_neg_10_thres =
					TEMP_NEG_10_THRES;
	}
	if (of_property_read_u32(np, "temp_neg_10_thres_plus_x_degree", &val) >= 0)
		info->data.temp_neg_10_thres_plus_x_degree = val;
	else {
		chr_err("use default TEMP_NEG_10_THRES_PLUS_X_DEGREE:%d\n",
			TEMP_NEG_10_THRES_PLUS_X_DEGREE);
		info->data.temp_neg_10_thres_plus_x_degree =
					TEMP_NEG_10_THRES_PLUS_X_DEGREE;
	}

#if 0
	if (of_property_read_u32(np, "temp_neg_10_thres", &val) >= 0)
		info->data.temp_neg_10_thres = val;
	else {
		chr_err("use default TEMP_NEG_10_THRES:%d\n",
			TEMP_NEG_10_THRES);
		info->data.temp_neg_10_thres = TEMP_NEG_10_THRES;
	}
#endif

	/*get jeita current from dts*/
	if (of_property_read_u32(np, "jeita_current_t5_to_t6", &val) >= 0)
				info->data.jeita_current_t5_to_t6 = val;
	else {
				chr_err("use default JEITA_CURRENT_T5_TO_T6:%d\n",
							JEITA_CURRENT_T5_TO_T6);
				info->data.jeita_current_t5_to_t6 = JEITA_CURRENT_T5_TO_T6;
	}

	if (of_property_read_u32(np, "jeita_current_t4_to_t5", &val) >= 0)
				info->data.jeita_current_t4_to_t5 = val;
	else {
				chr_err("use default JEITA_CURRENT_T4_TO_T5:%d\n",
							JEITA_CURRENT_T4_TO_T5);
				info->data.jeita_current_t4_to_t5 = JEITA_CURRENT_T4_TO_T5;
	}

	if (of_property_read_u32(np, "jeita_current_t3_to_t4", &val) >= 0)
				info->data.jeita_current_t3_to_t4 = val;
	else {
				chr_err("use default JEITA_CURRENT_T3_TO_T4:%d\n",
							JEITA_CURRENT_T3_TO_T4);
				info->data.jeita_current_t3_to_t4 = JEITA_CURRENT_T3_TO_T4;
	}

	if (of_property_read_u32(np, "jeita_current_t2_to_t3", &val) >= 0)
				info->data.jeita_current_t2_to_t3 = val;
	else {
				chr_err("use default JEITA_CURRENT_T2_TO_T3:%d\n",
							JEITA_CURRENT_T2_TO_T3);
				info->data.jeita_current_t2_to_t3 = JEITA_CURRENT_T2_TO_T3;
	}

	if (of_property_read_u32(np, "jeita_current_t1_to_t2", &val) >= 0)
				info->data.jeita_current_t1_to_t2 = val;
	else {
				chr_err("use default JEITA_CURRENT_T1_TO_T2:%d\n",
							JEITA_CURRENT_T1_TO_T2);
				info->data.jeita_current_t1_to_t2 = JEITA_CURRENT_T1_TO_T2;
	}

	if (of_property_read_u32(np, "jeita_current_t0_to_t1", &val) >= 0)
				info->data.jeita_current_t0_to_t1 = val;
	else {
				chr_err("use default JEITA_CURRENT_T0_TO_T1:%d\n",
							JEITA_CURRENT_T0_TO_T1);
				info->data.jeita_current_t0_to_t1 = JEITA_CURRENT_T0_TO_T1;
	}

	if (of_property_read_u32(np, "jeita_current_below_t0", &val) >= 0)
				info->data.jeita_current_below_t0 = val;
	else {
				chr_err("use default JEITA_CURRENT_BELOW_T0:%d\n",
							JEITA_CURRENT_BELOW_T0);
				info->data.jeita_current_below_t0 = JEITA_CURRENT_BELOW_T0;
	}

}