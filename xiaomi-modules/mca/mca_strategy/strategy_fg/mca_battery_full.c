// SPDX-License-Identifier: GPL-2.0
/*
 * mca_battery_full.c
 *
 * mca battery full driver
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

#include <linux/delay.h>
#include <mca/common/mca_log.h>
#include <mca/common/mca_event.h>
#include <mca/common/mca_parse_dts.h>
#include <mca/platform/platform_buckchg_class.h>
#include <mca/platform/platform_fg_ic_ops.h>
#include <mca/strategy/strategy_fg_class.h>
#include "inc/strategy_fg.h"
#include "inc/mca_battery_full.h"

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_battery_full"
#endif

#define CURRENT_MONITOR_INTERVAL_DEFAULT 1000
#define SCREEN_AUDIO_STATUS_INTERVAL_DEFAULT 300000
#define CURRENT_MONITOR_CNT 5

static DEFINE_MUTEX(st_lock);

static void mca_battery_full_process_control(bool start, const char* reason)
{
	static bool start_last = false;

	if(start == start_last){
		if(reason)
			mca_log_err("%s, batt curr monitor already %s\n", reason, start ? "start" : "exit");
		return;
	}

	if(start) {
		platform_class_buckchg_ops_set_term(MAIN_BUCK_CHARGER, false);	// disable pmic term_en
		platform_class_buckchg_ops_set_chg(MAIN_BUCK_CHARGER, true);	// enable pmic chg_en
	} else {
		platform_class_buckchg_ops_set_term(MAIN_BUCK_CHARGER, true);	// enable pmic term_en
		platform_class_buckchg_ops_set_chg(MAIN_BUCK_CHARGER, false);	// disable pmic chg_en
	}
	start_last = start;
	if(reason)
		mca_log_err("%s, batt curr monitor %s\n", reason, start ? "start" : "exit");
}

static int mca_battery_full_open_co_mos(struct strategy_fg *fg)
{
	int retry_cnt = 0, co_en_flag = 0, ret = 0;

	co_en_flag = platform_fg_ops_get_co_status(FG_IC_MASTER);
	while(co_en_flag == MCA_BATTERY_FULL_CO_MOS_AUTOCTRL && retry_cnt <= 6) {
		ret = platform_fg_ops_set_co_mos(FG_IC_MASTER, true);	// open co charging path
		if(ret < 0) {
			mca_log_err("failed to open open charging path\n");
		} else {
			msleep(600);
			co_en_flag = platform_fg_ops_get_co_status(FG_IC_MASTER);
			if(co_en_flag == MCA_BATTERY_FULL_CO_MOS_AUTOCTRL) {
				retry_cnt++;
				mca_log_err("waiting for mos open! retry cnt:%d\n", retry_cnt);
			} else {
				mca_log_err("co charging path open, retry cnt:%d\n", retry_cnt);
				return 1;
			}
		}
	}

	return 0;
}

static int mca_battery_full_autoctrl_co_mos(struct strategy_fg *fg)
{
	int retry_cnt = 0, co_en_flag = 0, ret = 0;

	co_en_flag = platform_fg_ops_get_co_status(FG_IC_MASTER);
	while(co_en_flag == MCA_BATTERY_FULL_CO_MOS_OPEN && retry_cnt <= 6) {
		ret = platform_fg_ops_set_co_mos(FG_IC_MASTER, false);	// open co charging path
		if(ret < 0) {
			mca_log_err("failed to open open charging path\n");
		} else {
			msleep(600);
			co_en_flag = platform_fg_ops_get_co_status(FG_IC_MASTER);
			if(co_en_flag == MCA_BATTERY_FULL_CO_MOS_OPEN) {
				retry_cnt++;
				mca_log_err("waiting for mos autoctrl! retry cnt:%d\n", retry_cnt);
			} else {
				mca_log_err("co charging path autoctrl, retry cnt:%d\n", retry_cnt);
				return 1;
			}
		}
	}

	return 0;
}

static void mca_screen_and_audio_status_work(struct work_struct *work)
{
	struct strategy_fg *fg = container_of(work, struct strategy_fg, screen_and_audio_status_work.work);
	int interval_ms = 1000;

	if(MCA_BATTERY_FULL_EXIT == fg->battery_full_status)
		return;

	if(fg->screen_status || fg->audio_state) {
		fg->sa_status = true;
	} else {
		fg->sa_status = false;
	}

	mutex_lock(&st_lock);
	if(fg->sa_status_last!= fg->sa_status || fg->battery_full_status == MCA_BATTERY_FULL_ENTRY) {
		fg->battery_full_status = fg->sa_status ?
			MCA_BATTERY_FULL_SC_AU_WORKING : MCA_BATTERY_FULL_SC_AU_NOT_WORK;
		interval_ms = fg->sa_status ? (300 * 1000) : 1000;
		mca_log_err("full_st:%d, sa_st[%d %d]\n", fg->battery_full_status, fg->sa_status_last, fg->sa_status);
	}
	mutex_unlock(&st_lock);
	fg->sa_status_last = fg->sa_status;
	schedule_delayed_work(&fg->screen_and_audio_status_work, msecs_to_jiffies(interval_ms));
}

static void mca_battery_full_current_monitor_work(struct work_struct *work)
{
	struct strategy_fg *fg = container_of(work, struct strategy_fg, full_current_monitor_work.work);
	int ibat = 0;
	int vbat = 0;
	int volt_threshold = 0;
	enum co_mos_status co_en = MCA_BATTERY_FULL_CO_MOS_OPEN;
	int chg_fet = 0;
	unsigned int work_status = work_busy(&fg->screen_and_audio_status_work.work);

	if(!fg->cfg.support_full_curr_monitor)
		return;

	if (!(work_status & (WORK_BUSY_PENDING  | WORK_BUSY_RUNNING))) {
		schedule_delayed_work(&fg->screen_and_audio_status_work, 0);
		mca_log_info("sc_au dwork started\n");
	}

	mutex_lock(&st_lock);
	switch(fg->battery_full_status) {
	case MCA_BATTERY_FULL_ENTRY:
		if(fg->battery_full_status == MCA_BATTERY_FULL_SC_AU_WORKING) {
			if(mca_battery_full_autoctrl_co_mos(fg)) {
				mca_battery_full_process_control(true, "full start");
				fg->battery_full_status = MCA_BATTERY_FULL_KEEP;
			}

			if(fg->battery_full_status != MCA_BATTERY_FULL_KEEP) {
				fg->battery_full_status = MCA_BATTERY_FULL_EXIT;
			}
		}

		break;
	case MCA_BATTERY_FULL_SC_AU_NOT_WORK:
		co_en = platform_fg_ops_get_co_status(FG_IC_MASTER);
		if(co_en == MCA_BATTERY_FULL_CO_MOS_OPEN) {	// co mos already open
			mca_battery_full_process_control(false, "sa not work");
			fg->battery_full_status = MCA_BATTERY_FULL_KEEP;
		} else {
			mca_battery_full_process_control(false, "mos sa not work");
			if(mca_battery_full_open_co_mos(fg)) {
				fg->battery_full_status = MCA_BATTERY_FULL_KEEP;
			}
		}
		break;
	case MCA_BATTERY_FULL_SC_AU_WORKING:
		co_en = platform_fg_ops_get_co_status(FG_IC_MASTER);
		if(co_en == MCA_BATTERY_FULL_CO_MOS_AUTOCTRL) {	// co mos already in auto control
			mca_battery_full_process_control(true, "sc au working");
			fg->battery_full_status = MCA_BATTERY_FULL_KEEP;
		} else {
			if(mca_battery_full_autoctrl_co_mos(fg)) {
				mca_battery_full_process_control(true, "sa full start");
				fg->battery_full_status = MCA_BATTERY_FULL_KEEP;
			}
		}
		break;
	case MCA_BATTERY_FULL_KEEP:
		break;
	case MCA_BATTERY_FULL_EXIT:
		goto out;
		break;
	default:
		break;
	}
	mutex_unlock(&st_lock);
	if(fg->batt_cyclecount >= 0){
		for(int i = 0; i < CYCLECOUNT; i++) {
			if(fg->batt_cyclecount >= fg->cfg.full_volt_monitor[i][0]){
				volt_threshold = fg->cfg.full_volt_monitor[i][1];
				break;
			}
		}
	} else {
		mca_log_err("batt_cyclecount error\n");
	}

	/* monitor battery current voltage after full*/
	if(strategy_class_fg_ops_get_current(&ibat) || strategy_class_fg_ops_get_voltage(&vbat)) {
		mca_battery_full_process_control(false, "fg err exit`");
		goto out;
	}

	if(ibat >= 0 || vbat >= volt_threshold) {
		fg->full_current_count= 0;
	} else {
		fg->full_current_count++;
	}

	co_en = platform_fg_ops_get_co_status(FG_IC_MASTER);
	chg_fet = platform_fg_ops_get_chg_fet_status(FG_IC_MASTER);
	mca_log_info("co-en:%d, chgfet:%d, st:%d, ibat=%d, vbat=%d, vth=%d, cnt=%d, online=%d, rechg=%d, sa_st=%d\n",
			co_en, chg_fet, fg->battery_full_status, ibat, vbat, volt_threshold, fg->full_current_count,
			fg->power_present, fg->recharging, fg->sa_status);

	/* if battery still charging, enable pmic termination. */
	if(fg->full_current_count >= CURRENT_MONITOR_CNT) {
		fg->full_current_count = 0;
		fg->battery_full_status = MCA_BATTERY_FULL_ENTRY;
		mca_battery_full_process_control(false, "still charging");
		mca_log_err("full, but still charging. process end!\n");
	} else {
		schedule_delayed_work(&fg->full_current_monitor_work,
							msecs_to_jiffies(CURRENT_MONITOR_INTERVAL_DEFAULT));
	}
	return;
out:
	mutex_unlock(&st_lock);
	mca_battery_full_open_co_mos(fg);
	fg->battery_full_status = MCA_BATTERY_FULL_ENTRY;
	mca_battery_full_process_control(false, "err exit");
	cancel_delayed_work(&fg->full_current_monitor_work);
	cancel_delayed_work(&fg->screen_and_audio_status_work);
	return;
}

/* open fg co charging path, enable pmic term en cancel full_current_monitor_work */
int mca_battery_full_cancel_curr_monitor_work(void *data)
{
	struct strategy_fg *fg = (struct strategy_fg *)data;
	unsigned int work_status = work_busy(&fg->full_current_monitor_work.work);

	if (!(work_status & (WORK_BUSY_PENDING  | WORK_BUSY_RUNNING))) {
		mca_log_err("full_current_monitor_work is not running\n");
		return 0;
	}

	if(fg->power_present || !fg->recharging) {
		mca_log_err("online or not recharge.\n");
	}

	/* if plug out or start recharging, resume CO path and pmic termination. stop this work */
	fg->battery_full_status = false;

	cancel_delayed_work_sync(&fg->full_current_monitor_work);
	cancel_delayed_work_sync(&fg->screen_and_audio_status_work);

	mca_battery_full_open_co_mos(fg);
	platform_class_buckchg_ops_set_term(MAIN_BUCK_CHARGER, true);	// enable pmic term_en
	mca_log_err("stop full curret monitor work\n");

	return 0;
}

int mca_battery_full_parse_dt(void *data)
{
	struct strategy_fg *fg = (struct strategy_fg *)data;
	struct device_node *node = fg->dev->of_node;
	int ret,len;

	fg->cfg.support_full_curr_monitor = of_property_read_bool(node, "support-full-curr-monitor");
	if (fg->cfg.support_full_curr_monitor){
		len = mca_parse_dts_u32_count(node, "full-volt-monitor", CYCLECOUNT, VOLT_THRESHOLD);
		if (len < 0) {
			mca_log_err("parse full-volt-monitor failed\n");
			return -1;
		}
		ret = mca_parse_dts_u32_array(node, "full-volt-monitor", &fg->cfg.full_volt_monitor[0][0], len);
		if (ret < 0)
			return -1;
	}
	return 0;
}

int mca_battery_full_init(void *data)
{
	struct strategy_fg *fg = (struct strategy_fg *)data;
	fg->battery_full_status = MCA_BATTERY_FULL_ENTRY;

	if(!fg->cfg.support_full_curr_monitor)
		return 0;

	/* make sure co mos is opened */
	mca_battery_full_open_co_mos(fg);

	INIT_DELAYED_WORK(&fg->full_current_monitor_work, mca_battery_full_current_monitor_work);
	INIT_DELAYED_WORK(&fg->screen_and_audio_status_work, mca_screen_and_audio_status_work);

	return 0;
}

int mca_battery_full_shutdown(void *data)
{
	struct strategy_fg *fg = (struct strategy_fg *)data;

	/* make sure co mos is opened */
	mca_battery_full_open_co_mos(fg);

	return 0;
}
