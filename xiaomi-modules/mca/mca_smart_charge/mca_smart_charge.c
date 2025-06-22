// SPDX-License-Identifier: GPL-2.0
/*
 * mca_smart_charge.c
 *
 * smart_charge driver
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/version.h>
#include <mca/protocol/protocol_class.h>
#include <mca/strategy/strategy_class.h>
#include <mca/strategy/strategy_fg_class.h>
#include <mca/strategy/strategy_soc_limit.h>
#include <mca/common/mca_event.h>
#include <mca/common/mca_log.h>
#include <mca/common/mca_parse_dts.h>
#include <mca/common/mca_voter.h>
#include <mca/common/mca_sysfs.h>
#include <mca/common/mca_panel.h>
#include <mca/smartchg/smart_chg_class.h>
#include "inc/mca_smart_charge.h"
#include <mca/common/mca_charge_mievent.h>
#include <mca/common/mca_hwid.h>
#include "hwid.h"

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_smart_charge"
#endif
#ifndef BIT
#define BIT(x) (1 << (x))
#endif

#define SMART_CHG_SOCLMT_TRIG_DEFAULT 80
#define SMART_CHG_SOCLMT_CANCEL_DEFAULT 5
#define SMART_CHARGE_DELTA_FV_DEFAULT_VALUE 0
#define SMART_CHARGE_DELTA_ICHG_DEFAULT_VALUE 0
#define SMART_CHARGE_DTPT_DELTA_VALUE 15
#define SMART_CHARGE_FAKE_SCENE_DEFAULT -1
#define MAX_MMAP_SIZE    4096

enum mca_smart_charge_ioctrl_cmd {
	MCA_SMARTCHG_CMD_DATA = 1,
	MCA_SMARTCHG_CMD_INVALID,
};

#define MCA_SMARTCHG_MAGIC 0x10
#define SMARTCHG_DATA_CMD _IOC(_IOC_READ|_IOC_WRITE, MCA_SMARTCHG_MAGIC, MCA_SMARTCHG_CMD_DATA, PAGE_SIZE)

static struct smart_charge_info *global_smartchg_info;
static struct mca_smartchg_if_ops *g_mca_smartchg_if_ops[MCA_SMARTCHG_IF_CHG_TYPE_END];
static void smart_charge_report_scene(struct smart_charge_info *info, int scene);

static struct mca_smartchg_if_ops *mca_smartchg_if_get_ops(unsigned int type)
{
	return g_mca_smartchg_if_ops[type];
}

int mca_smartchg_if_ops_register(struct mca_smartchg_if_ops *ops)
{
	if (!ops) {
		mca_log_err("ops invalid %d\n");
		return -1;
	}

	if (ops->type < 0 || ops->type >= MCA_SMARTCHG_IF_CHG_TYPE_END) {
		mca_log_err("ops->type invalid %d\n", ops->type);
		return -1;
	}


	g_mca_smartchg_if_ops[ops->type] = ops;
	return  0;
}
EXPORT_SYMBOL(mca_smartchg_if_ops_register);

static ssize_t smart_charge_sysfs_show(struct device *dev,
		struct device_attribute *attr, char *buf);
static ssize_t smart_charge_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct mca_sysfs_attr_info smartchg_sysfs_field_tbl[] = {
	mca_sysfs_attr_rw(smart_charge_sysfs, 0664, MCA_PROP_SMARTCHG, smart_chg),
	mca_sysfs_attr_rw(smart_charge_sysfs, 0664, MCA_PROP_SMARTCHG_FV, smart_fv),
	mca_sysfs_attr_rw(smart_charge_sysfs, 0664, MCA_PROP_SMARTCHG_ICHG, smart_ichg),
	mca_sysfs_attr_rw(smart_charge_sysfs, 0664, MCA_PROP_SMARTBATT, smart_batt),
	mca_sysfs_attr_rw(smart_charge_sysfs, 0664, MCA_PROP_SMARTNIGHT, smart_night),
	mca_sysfs_attr_ro(smart_charge_sysfs, 0440, MCA_PROP_POSTURE, posture),
	mca_sysfs_attr_rw(smart_charge_sysfs, 0664, MCA_PROP_SCENE, scene),
	mca_sysfs_attr_rw(smart_charge_sysfs, 0664, MCA_PROP_BOARD_TEMP, board_temp),
	mca_sysfs_attr_rw(smart_charge_sysfs, 0664, MCA_PROP_SMART_SIC_MODE, smart_sic_mode),
};

#define SMARTCHG_SYSFS_ATTRS_SIZE   ARRAY_SIZE(smartchg_sysfs_field_tbl)

static struct attribute *smartchg_sysfs_attrs[SMARTCHG_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group smartchg_sysfs_attr_group = {
	.attrs = smartchg_sysfs_attrs,
};

void mca_smartchg_set_scene(int scene)
{
	struct smart_charge_info *info = global_smartchg_info;

	if (!info) {
		mca_log_err("info is null\n");
		return;
	}

	mca_log_info("set scene: %d:%d => %d\n", info->scene, info->fake_scene, scene);
	if (scene >= SMART_CHG_SCENE_NORMAL && scene < SMART_CHG_SCENE_INDEX_MAX) {
		smart_charge_report_scene(info, scene);
		info->scene = scene;
	} else if (scene >= SMART_CHG_SCENE_REDIR_START && scene < SMART_CHG_SCENE_REDIR_INDEX_MAX) {
		info->fake_scene = scene - SMART_CHG_SCENE_REDIR_START;
	} else if (scene == SMART_CHARGE_FAKE_SCENE_DEFAULT) {
		info->fake_scene = SMART_CHARGE_FAKE_SCENE_DEFAULT;
	}

	cancel_delayed_work_sync(&info->smart_sense_chg_work);
	schedule_delayed_work(&info->smart_sense_chg_work, 0);
}
EXPORT_SYMBOL(mca_smartchg_set_scene);

int mca_smartchg_get_scene(void)
{
	struct smart_charge_info *info = global_smartchg_info;

	if (!info) {
		mca_log_err("info is null\n");
		return 0;
	}

	if (info->fake_scene != SMART_CHARGE_FAKE_SCENE_DEFAULT)
		return info->fake_scene;

	return info->scene;
}
EXPORT_SYMBOL(mca_smartchg_get_scene);

void mca_smartchg_set_board_temp(int board_temp)
{
	struct smart_charge_info *info = global_smartchg_info;

	if (!info) {
		mca_log_err("info is null\n");
		return;
	}

	mca_log_info("set board_temp: %d\n", board_temp);
	info->board_temp = board_temp;
}
EXPORT_SYMBOL(mca_smartchg_set_board_temp);

int mca_smartchg_get_board_temp(void)
{
	struct smart_charge_info *info = global_smartchg_info;

	if (!info) {
		mca_log_err("info is null\n");
		return 0;
	}

	return info->board_temp;
}
EXPORT_SYMBOL(mca_smartchg_get_board_temp);

int mca_smartchg_is_extreme_cold_enabled(void)
{
	if (global_smartchg_info)
		return global_smartchg_info->smart_chg_data.smart_chgcfg[SMART_CHG_EXTREME_COLD].enable == 1;
	return false;
}
EXPORT_SYMBOL(mca_smartchg_is_extreme_cold_enabled);

int mca_smartchg_get_limit_soc(void)
{
	struct smart_charge_info *info = global_smartchg_info;

	if (!info) {
		mca_log_err("info is null\n");
		return 0;
	}
	if (info->soc_limit_enable)
		return info->soc_limit;
	return 0;
}
EXPORT_SYMBOL(mca_smartchg_get_limit_soc);

static void smart_charge_report_scene(struct smart_charge_info *info, int scene)
{
	static int last_report_scene;
	int report_scene = 0;
	int len = 0;
	char event[MCA_EVENT_NOTIFY_SIZE] = { 0 };
	struct mca_event_notify_data event_data = { 0 };

	if (scene != info->scene) {
		switch (scene) {
		case SMART_CHG_SCENE_PHONE:
		case SMART_CHG_SCENE_VIDEO:
		case SMART_CHG_SCENE_VIDEOCHAT:
			report_scene = scene;
			break;
		default:
			report_scene = 0;
			break;
		}

		if (report_scene != last_report_scene) {
			mca_log_info("report scene: %d => %d\n", last_report_scene, report_scene);
			last_report_scene = report_scene;

			len = snprintf(event, MCA_EVENT_NOTIFY_SIZE, "MCA_SMARTCHG_SCENE=%d", report_scene);
			event_data.event = event;
			event_data.event_len = len;
			mca_event_report_uevent(&event_data);
		}
	}
}

static void smart_charge_handle_sense_chg(struct smart_charge_info *info)
{
	int posture_st = SMART_CHG_POSTURE_UNKNOW;
	int scene = SMART_CHG_SCENE_NORMAL;
	int sic_mode = SMART_CHG_SIC_MODE_BALANCED;
	bool active_stat = false;
	int len = 0;
	char event[MCA_EVENT_NOTIFY_SIZE] = { 0 };
	struct mca_event_notify_data event_data = { 0 };

	scene = info->fake_scene != SMART_CHARGE_FAKE_SCENE_DEFAULT ? info->fake_scene : info->scene;
	posture_st = info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].use_fake_value ?
		info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].fake_value :
		info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].func_value;

	if (!info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].enable ||
		posture_st <= SMART_CHG_POSTURE_UNKNOW || posture_st >= SMART_CHG_POSTURE_INDEX_MAX)
		return;

	switch (scene) {
	case SMART_CHG_SCENE_PHONE:
		active_stat = true;
		if (posture_st >= SMART_CHG_POSTURE_DESKTOP && posture_st <= SMART_CHG_POSTURE_HOLDER)
			sic_mode = SMART_CHG_SIC_MODE_SUPERCHG;
		else if (posture_st >= SMART_CHG_POSTURE_ONEHAND && posture_st <= SMART_CHG_POSTURE_TWOHAND_V)
			sic_mode = SMART_CHG_SIC_MODE_MIDDLE;
		else if (posture_st == SMART_CHG_POSTURE_ANS_CALL)
			sic_mode = SMART_CHG_SIC_MODE_BALANCED;
		break;
	case SMART_CHG_SCENE_VIDEO:
	case SMART_CHG_SCENE_VIDEOCHAT:
	case SMART_CHG_SCENE_PER_VIDEO:
		active_stat = true;
		if (posture_st >= SMART_CHG_POSTURE_DESKTOP && posture_st <= SMART_CHG_POSTURE_HOLDER)
			sic_mode = SMART_CHG_SIC_MODE_SUPERCHG;
		else if (posture_st >= SMART_CHG_POSTURE_ONEHAND && posture_st <= SMART_CHG_POSTURE_ANS_CALL)
			sic_mode = SMART_CHG_SIC_MODE_BALANCED;
		break;
	case SMART_CHG_SCENE_CLASS0:
	case SMART_CHG_SCENE_PER_CLASS0:
		if (info->support_sensechg_v2) {
			active_stat = true;
			if (posture_st >= SMART_CHG_POSTURE_DESKTOP && posture_st <= SMART_CHG_POSTURE_HOLDER)
				sic_mode = SMART_CHG_SIC_MODE_SUPERCHG;
			else if (posture_st >= SMART_CHG_POSTURE_ONEHAND && posture_st <= SMART_CHG_POSTURE_ANS_CALL)
				sic_mode = SMART_CHG_SIC_MODE_BALANCED;
		} else {
			mca_log_err("unsupport scene: %d\n", info->scene);
			return;
		}
		break;
	default:
		mca_log_err("unsupport scene: %d\n", info->scene);
		return;
	}

	if (info->screen_state == 0)
		sic_mode /= 2;

	mca_log_info("active_stat: %d, online: %d, screen_state: %d, scene: %d, posture: %d, smart_sic_mode: %d -> %d\n",
		active_stat, info->online, info->screen_state, scene, posture_st, info->smart_sic_mode, sic_mode);
	if (sic_mode != info->smart_sic_mode) {
		info->smart_sic_mode = sic_mode;
		if (active_stat && info->online) {
			len = snprintf(event, MCA_EVENT_NOTIFY_SIZE,
				"POWER_SUPPLY_SMART_SIC_MODE=%d", info->smart_sic_mode);
			event_data.event = event;
			event_data.event_len = len;
			mca_event_report_uevent(&event_data);
		}
	}
}

//static void smartcharging_handle_soc_limit_process(struct smart_charge_info *info,
//	bool enable, int soc_limit_thre)
//{
//	int curr_soc = 0;
//	static struct mca_smartchg_if_ops *if_ops;

//	if (enable) {
//		curr_soc = strategy_class_fg_ops_get_soc();
//		if (curr_soc >= soc_limit_thre) {
//			mca_log_info("soc_limit_thre = %d\n", soc_limit_thre);
//			info->soc_limit_enable = true;
//			for (int i = 0; i < MCA_SMARTCHG_IF_CHG_TYPE_END; i++) {
//				if_ops = mca_smartchg_if_get_ops(i);
//				if (!if_ops || !if_ops->set_soc_limit_sts)
//					continue;
//				if_ops->set_soc_limit_sts(if_ops->data, info->soc_limit_enable);
//			}
//		} else{
//			cancel_delayed_work_sync(&info->smart_soc_limit_work);
//			schedule_delayed_work(&info->smart_soc_limit_work, msecs_to_jiffies(1000));
//		}
//	} else {
//		mca_log_info("disable soc limit\n");
//		cancel_delayed_work_sync(&info->smart_soc_limit_work);
//		info->soc_limit_enable = true;
//		for (int i = 0; i < MCA_SMARTCHG_IF_CHG_TYPE_END; i++) {
//			if_ops = mca_smartchg_if_get_ops(i);
//			if (!if_ops || !if_ops->set_soc_limit_sts)
//				continue;
//			if_ops->set_soc_limit_sts(if_ops->data, info->soc_limit_enable);
//		}
//	}
//}

static void smart_charge_set_soc_limit(bool enable)
{
	static struct mca_smartchg_if_ops *if_ops;

	for (int i = 0; i < MCA_SMARTCHG_IF_CHG_TYPE_END; i++) {
		if_ops = mca_smartchg_if_get_ops(i);
		if (!if_ops || !if_ops->set_soc_limit_sts)
			continue;
		if_ops->set_soc_limit_sts(if_ops->data, enable);
	}
}

#define SMART_CHG_SOC_HYS	2
static void smart_charge_soc_limit_workfunc(struct work_struct *work)
{
	struct smart_charge_info *info = container_of(work,
		struct smart_charge_info, smart_soc_limit_work.work);
	int curr_soc = 0;

	if (!info->online) {
		mca_log_err("adapter is plugout, don't handle soc limit\n");
		return;
	}

	if (info->soc_limit) {
		curr_soc = strategy_class_fg_ops_get_soc();
		mca_log_info("curr_soc = %d, soc_limit_thre = %d\n", curr_soc, info->soc_limit);
		if (curr_soc >= info->soc_limit) {
			if (info->soc_limit_enable == false) {
				info->soc_limit_enable = true;
				smart_charge_set_soc_limit(info->soc_limit_enable);
			}

			if (curr_soc >= info->soc_limit + SMART_CHG_SOC_HYS) {
				if (info->smart_chg_data.smart_chgcfg[SMART_CHG_ENDURANCE_PRO].enable && !info->ignore_upload.endurance_pro) {
					mca_charge_mievent_report(CHARGE_DFX_SMART_ENDURANCE_SOC_ERR, &curr_soc, 1);
					info->ignore_upload.endurance_pro = true;
				}
				if (info->smart_chg_data.smart_chgcfg[SMART_CHG_NAVIGATION].enable && !info->ignore_upload.navigation) {
					mca_charge_mievent_report(CHARGE_DFX_SMART_NAVIGATION_SOC_ERR, &curr_soc, 1);
					info->ignore_upload.endurance_pro = true;
				}
			}

		} else if (curr_soc <= info->soc_limit - SMART_CHG_SOCLMT_CANCEL_DEFAULT) {
			if (info->soc_limit_enable == true) {
				info->soc_limit_enable = false;
				smart_charge_set_soc_limit(info->soc_limit_enable);
			}
		}
		schedule_delayed_work(&info->smart_soc_limit_work, msecs_to_jiffies(1000));
	} else {
		mca_log_info("disable soc limit\n");
		info->soc_limit_enable = false;
		smart_charge_set_soc_limit(info->soc_limit_enable);
	}
	mca_log_info("soc_limit_enable = %d\n", info->soc_limit_enable);
}

/*to handle navigation and smart night algorithm conflict*/
static void smartcharging_handle_algorithm_conflict(struct smart_charge_info *info)
{
	bool smart_night = false;
	bool smart_navigation = false;
	bool smart_endurance_pro = false;
	uint32_t soc_upper_thre = SMART_CHG_SOCLMT_TRIG_DEFAULT;

	smart_night = info->smart_chg_data.smart_chgcfg[SMART_CHG_NIGHT].enable;
	smart_navigation = info->smart_chg_data.smart_chgcfg[SMART_CHG_NAVIGATION].enable;
	smart_endurance_pro = info->smart_chg_data.smart_chgcfg[SMART_CHG_ENDURANCE_PRO].enable;

	/*endurance protect is prior to all others, then navigation is prior to smart night charge */
	if (smart_endurance_pro) {
		soc_upper_thre = info->smart_chg_data.smart_chgcfg[SMART_CHG_ENDURANCE_PRO].func_value;
		mca_log_info("smart_endurance %d, soc_upper_thre:%d", smart_endurance_pro, soc_upper_thre);
	} else if (smart_navigation) {
		soc_upper_thre = info->smart_chg_data.smart_chgcfg[SMART_CHG_NAVIGATION].func_value;
		mca_log_info("smart_navigation %d, soc_upper_thre:%d", smart_navigation, soc_upper_thre);
	} else if (smart_night) {
		soc_upper_thre = info->smart_chg_data.smart_chgcfg[SMART_CHG_NIGHT].func_value;
		mca_log_info("smart_night %d, soc_upper_thre:%d", smart_night, soc_upper_thre);
	} else {
		soc_upper_thre = 0;
	}
	info->soc_limit = soc_upper_thre;
	cancel_delayed_work_sync(&info->smart_soc_limit_work);
	schedule_delayed_work(&info->smart_soc_limit_work, 0);
	//smartcharging_handle_soc_limit_process(info, !!soc_upper_thre, info->soc_limit);
}

static void smartcharging_handle_wls_super(struct smart_charge_info *info)
{
	static struct mca_smartchg_if_ops *if_ops;

	for (int i = 0; i < MCA_SMARTCHG_IF_CHG_TYPE_END; i++) {
		if_ops = mca_smartchg_if_get_ops(i);
		if (!if_ops || !if_ops->set_wls_super_sts)
			continue;
		if_ops->set_wls_super_sts(if_ops->data, info->smart_chg_data.smart_chgcfg[SMART_CHG_WLS_SUPER].enable);
	}
}

static void smartcharging_handle_power_boost(struct smart_charge_info *info, int type)
{
	static struct mca_smartchg_if_ops *if_ops;

	for (int i = 0; i < MCA_SMARTCHG_IF_CHG_TYPE_END; i++) {
		if_ops = mca_smartchg_if_get_ops(i);
		if (!if_ops || !if_ops->set_pwr_boost_sts)
			continue;
		if (type == SMART_CHG_OUTDOOR_PWR_BOOST)
			if_ops->set_pwr_boost_sts(if_ops->data, info->smart_chg_data.smart_chgcfg[SMART_CHG_OUTDOOR].enable);
		else if (type == SMART_CHG_TRAVELWAIT_PWR_BOOST)
			if_ops->set_pwr_boost_sts(if_ops->data, info->smart_chg_data.smart_chgcfg[SMART_CHG_TRAVELWAIT].enable);
	}
}

static void smartcharging_handle_wls_quiet(struct smart_charge_info *info)
{
	static struct mca_smartchg_if_ops *if_ops;

	for (int i = 0; i < MCA_SMARTCHG_IF_CHG_TYPE_END; i++) {
		if_ops = mca_smartchg_if_get_ops(i);
		if (!if_ops || !if_ops->set_wls_quiet_sts)
			continue;
		if_ops->set_wls_quiet_sts(if_ops->data, info->smart_chg_data.smart_chgcfg[SMART_CHG_WLS_QUIET].enable);
	}
}

static void smartcharging_handle_controlmessage(void)
{
	struct smart_charge_info *info = global_smartchg_info;
	uint32_t smart_header = info->smart_chg_control.AsUINT32;
	int soc = strategy_class_fg_ops_get_soc();

	if (!info)
		return;

	smart_header = (smart_header & 0xFFFE);
	smart_header = (smart_header - 1) & smart_header;
	mca_log_info("smart_header: %d", smart_header);
	if (!smart_header) {
		info->smart_chg_data.ret_code = false;
		mca_log_err("input correct\n");
	} else {
		info->smart_chg_data.ret_code = true;
		mca_log_err("error input invalid\n");
		return;
	}

	mca_log_info("smart_chg_control: enable: %d, func_type: 0x%04x, func_value: %d\n",
		info->smart_chg_control.enable, info->smart_chg_control.func_type << 1,
		info->smart_chg_control.func_value);

	if (info->smart_chg_control.navigation) {
		info->smart_chg_data.smart_chgcfg[SMART_CHG_NAVIGATION].enable = info->smart_chg_control.enable;
		info->smart_chg_data.smart_chgcfg[SMART_CHG_NAVIGATION].func_value = info->smart_chg_control.soc_limit;
		smartcharging_handle_algorithm_conflict(info);
		if (info->smart_chg_control.enable)
			mca_charge_mievent_report(CHARGE_DFX_SMART_NAVIGATION_TRIGGERED, &soc, 1);
		else
			mca_charge_mievent_set_state(MIEVENT_STATE_END, CHARGE_DFX_SMART_NAVIGATION_TRIGGERED);

		info->ignore_upload.navigation = soc >= info->smart_chg_control.soc_limit ? true : false;

	} else if (info->smart_chg_control.endurance_pro) {
		info->smart_chg_data.smart_chgcfg[SMART_CHG_ENDURANCE_PRO].enable = info->smart_chg_control.enable;
		info->smart_chg_data.smart_chgcfg[SMART_CHG_ENDURANCE_PRO].func_value = info->smart_chg_control.soc_limit;
		smartcharging_handle_algorithm_conflict(info);
		if (info->smart_chg_control.enable)
			mca_charge_mievent_report(CHARGE_DFX_SMART_ENDURANCE_TRIGGERED, &soc, 1);
		else
			mca_charge_mievent_set_state(MIEVENT_STATE_END, CHARGE_DFX_SMART_ENDURANCE_TRIGGERED);

		info->ignore_upload.endurance_pro = soc >= info->smart_chg_control.soc_limit ? true : false;

	} else if (info->smart_chg_control.outdoor) {
		info->smart_chg_data.smart_chgcfg[SMART_CHG_OUTDOOR].enable = info->smart_chg_control.enable;
		info->smart_chg_data.smart_chgcfg[SMART_CHG_OUTDOOR].func_value = 0;
		smartcharging_handle_power_boost(info, SMART_CHG_OUTDOOR_PWR_BOOST);
	} else if (info->smart_chg_control.wls_super_chg) {
		info->smart_chg_data.smart_chgcfg[SMART_CHG_WLS_SUPER].enable = info->smart_chg_control.enable;
		info->smart_chg_data.smart_chgcfg[SMART_CHG_WLS_SUPER].func_value = 0;
		smartcharging_handle_wls_super(info);
	} else if (info->smart_chg_control.lowfast) {
		info->smart_chg_data.smart_chgcfg[SMART_CHG_LOWFAST].enable = info->smart_chg_control.enable;
		info->smart_chg_data.smart_chgcfg[SMART_CHG_LOWFAST].func_value = 0;
	} else if (info->smart_chg_control.wls_quiet) {
		info->smart_chg_data.smart_chgcfg[SMART_CHG_WLS_QUIET].enable = info->smart_chg_control.enable;
		info->smart_chg_data.smart_chgcfg[SMART_CHG_WLS_QUIET].func_value = 0;
		smartcharging_handle_wls_quiet(info);
	} else if (info->smart_chg_control.sense_chg) {
		info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].enable = info->smart_chg_control.enable;
		if (info->smart_chg_control.func_value > SMART_CHG_POSTURE_FAKE_CLEAR) {
			mca_log_info("debug sense charge, set value: %d\n", info->smart_chg_control.func_value);
			info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].use_fake_value = true;
			info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].fake_value =
				info->smart_chg_control.func_value - SMART_CHG_POSTURE_FAKE_CLEAR;
		} else if (info->smart_chg_control.func_value == SMART_CHG_POSTURE_FAKE_CLEAR) {
			mca_log_info("debug sense charge, cancel fake value: %d\n", info->smart_chg_control.func_value);
			info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].use_fake_value = false;
			info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].fake_value = 0;
		} else {
			info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].func_value = info->smart_chg_control.func_value;
		}
		smart_charge_handle_sense_chg(info);
	} else if (info->smart_chg_control.extreme_cold) {
		info->smart_chg_data.smart_chgcfg[SMART_CHG_EXTREME_COLD].enable = info->smart_chg_control.enable;
		info->smart_chg_data.smart_chgcfg[SMART_CHG_EXTREME_COLD].func_value = 0;
	} else if (info->smart_chg_control.travel_wait) {
		info->smart_chg_data.smart_chgcfg[SMART_CHG_TRAVELWAIT].enable = info->smart_chg_control.enable;
		info->smart_chg_data.smart_chgcfg[SMART_CHG_TRAVELWAIT].func_value = 0;
		smartcharging_handle_power_boost(info, SMART_CHG_TRAVELWAIT_PWR_BOOST);
	}
}

static int smartcharging_receive_control_message(int val)
{
	struct smart_charge_info *info = global_smartchg_info;

	info->smart_chg_control.AsUINT32 = val;
	mca_log_info("smart_chg value is %08x\n", val);
	smartcharging_handle_controlmessage();
	return 0;
}

static int get_smart_chg_message(void)
{
	struct smart_charge_info *info = global_smartchg_info;
	uint32_t control_header = 0;
	int index = 0;

	control_header |= (info->smart_chg_data.ret_code & 0x01);

	for (index = SMART_CHG_NAVIGATION; index < SMART_CHG_MAX; index++) {
		if (info->smart_chg_data.smart_chgcfg[index].enable)
			control_header |= BIT(index);
		else
			control_header &= ~BIT(index);
	}
	mca_log_info("control_header = %d\n", control_header);
	return control_header;
}

static int handle_smart_batt_message(int val)
{
	struct smart_charge_info *info = global_smartchg_info;
	int vote_value;

	info->smart_batt_val = val;
	vote_value = info->smart_batt_val * info->cell_type;

	mca_log_info("smart_batt_val is %d vote_value %d\n", val, vote_value);
	mca_vote(info->smartchg_delta_fv_voter, "smart_batt", true, vote_value);

	return 0;
}

static int get_smart_batt_message(void)
{
	struct smart_charge_info *info = global_smartchg_info;

	return info->smart_batt_val;
}

static int handle_smart_night_message(int val)
{
	struct smart_charge_info *info = global_smartchg_info;

	info->smart_night_val = val;
	mca_log_info("smart_night_val is %d\n", !!val);
	if (val)
		strategy_class_fg_ops_get_rsoc(&info->night_enable_rsoc);
	if (!!val == 1 || !!val == 0) {
		info->smart_chg_data.smart_chgcfg[SMART_CHG_NIGHT].enable = val;
		info->smart_chg_data.smart_chgcfg[SMART_CHG_NIGHT].func_value = SMART_CHG_SOCLMT_TRIG_DEFAULT;
		smartcharging_handle_algorithm_conflict(info);
	}

	return 0;
}

static int get_smart_night_message(void)
{
	struct smart_charge_info *info = global_smartchg_info;

	return info->smart_night_val;

}

static void smart_charge_plugin_or_plugout_reset(struct smart_charge_info *info)
{

	if (info->online)
		mca_vote(info->smartchg_delta_ichg_voter, "batt_health", true, info->smart_delta_ichg);
	else
		mca_vote(info->smartchg_delta_ichg_voter, "batt_health", false, info->smart_delta_ichg);
}

static void smart_charge_csd_send_pulse(struct smart_charge_info *info)
{
	int soc = 0, rsoc = 0;
	int batt_temp = 0;
	bool smart_night = false;

	soc = strategy_class_fg_ops_get_soc();
	strategy_class_fg_ops_get_rsoc(&rsoc);
	(void)strategy_class_fg_ops_get_temperature(&batt_temp);
	batt_temp /= 10;
	smart_night = info->smart_chg_data.smart_chgcfg[SMART_CHG_NIGHT].enable;

	if (!info->pulse_mode) {
		if ((soc >= 50 && soc <= 80) && (batt_temp >= 20 && batt_temp <= 45)) {
			if (info->plugin_rsoc <= 45 && !info->screen_state && info->cycle_count > 100 && (info->cycle_count % 25 == 0) && rsoc == 50) {
				info->pulse_mode = 1;
				mca_event_block_notify(MCA_EVENT_CHARGE_STATUS, MCA_EVENT_CSD_SEND_PULSE, &info->pulse_mode);
			} else if (smart_night == true) {
				if (info->night_enable_rsoc <= 45 && rsoc == 50) {
					info->pulse_mode = 2;
					mca_event_block_notify(MCA_EVENT_CHARGE_STATUS, MCA_EVENT_CSD_SEND_PULSE, &info->pulse_mode);
				} else if (info->night_enable_rsoc > 45 && rsoc == 76) {
					info->pulse_mode = 3;
					mca_event_block_notify(MCA_EVENT_CHARGE_STATUS, MCA_EVENT_CSD_SEND_PULSE, &info->pulse_mode);
				}
			}
		}
	} else if ((info->cycle_count % 25 != 0) && smart_night == false) {
		info->pulse_mode = 0;
		mca_event_block_notify(MCA_EVENT_CHARGE_STATUS, MCA_EVENT_CSD_SEND_PULSE, &info->pulse_mode);
	}
	mca_log_info("soc: %d, rsoc: %d, plugin_rsoc: %d, night_enable_rsoc: %d, batt_temp: %d, cycle: %d, night: %d, pulse_mode: %d",
		soc, rsoc, info->plugin_rsoc, info->night_enable_rsoc, batt_temp, info->cycle_count, smart_night, info->pulse_mode);
}

static void smart_charge_workfunc(struct work_struct *work)
{
	struct smart_charge_info *info = container_of(work,
		struct smart_charge_info, smart_charge_work.work);
	int delta_ichg = 0;
	const struct mca_hwid *hwid = mca_get_hwid_info();

	//To-Do
	//Battery status monitoring work_func
	if (!info->online) {
		mca_log_err("adapter is plugout\n");
		return;
	}
	//battery health4.0 cyclecount decrease ichg
	strategy_class_fg_ops_get_cyclecount(&info->cycle_count);
	for (int i = 0; i < CC_ICHG_MAX_GROUP; i++) {
		if (info->cycle_count <= info->ichg_cc_table[i].cc_max) {
			delta_ichg = info->ichg_cc_table[i].delta_ichg;
			break;
		}
	}

	if (delta_ichg != info->smart_delta_ichg) {
		info->smart_delta_ichg = delta_ichg;
		mca_vote(info->smartchg_delta_ichg_voter, "batt_health", true, delta_ichg);
		mca_log_info("cycle_count %d smart_delta_ichg %d\n", info->cycle_count, info->smart_delta_ichg);
	}
	//csd1.0 feature:check battery swelling
	if (hwid && hwid->country_version == CountryCN && info->support_csd)
		smart_charge_csd_send_pulse(info);

	schedule_delayed_work(&info->smart_charge_work, msecs_to_jiffies(5000));
}

static void smart_charge_sense_chg_workfunc(struct work_struct *work)
{
	struct smart_charge_info *info = container_of(work,
		struct smart_charge_info, smart_sense_chg_work.work);

	if (!info)
		return;

	smart_charge_handle_sense_chg(info);
}

static int mca_smart_charge_process_event(int event, int value, void *data)
{
	struct smart_charge_info *info = data;

	if (!info)
		return -1;

	mca_log_info("receive event %d, value %d\n", event, value);
	switch (event) {
	case MCA_EVENT_USB_CONNECT:
	case MCA_EVENT_WIRELESS_CONNECT:
		info->online = 1;
		strategy_class_fg_ops_get_rsoc(&info->plugin_rsoc);
		smart_charge_plugin_or_plugout_reset(info);
		cancel_delayed_work_sync(&info->smart_charge_work);
		schedule_delayed_work(&info->smart_charge_work, 0);
		cancel_delayed_work_sync(&info->smart_soc_limit_work);
		schedule_delayed_work(&info->smart_soc_limit_work, 0);
		break;
	case MCA_EVENT_USB_DISCONNECT:
	case MCA_EVENT_WIRELESS_DISCONNECT:
		info->online = 0;
		smart_charge_plugin_or_plugout_reset(info);
		cancel_delayed_work_sync(&info->smart_charge_work);
		cancel_delayed_work_sync(&info->smart_soc_limit_work);
		//memset(&info->smart_chg_data, 0, sizeof(info->smart_chg_data));
		memset(&info->smart_chg_control, 0, sizeof(info->smart_chg_control));
		info->soc_limit_enable = false;
		info->ignore_upload.AsUINT32 = 0;
		smart_charge_set_soc_limit(info->soc_limit_enable);
		break;
	case MCA_EVENT_BATTERY_DTPT:
		if (value)
			mca_vote(info->smartchg_delta_fv_voter, "dtpt", true, SMART_CHARGE_DTPT_DELTA_VALUE);
		else
			mca_vote(info->smartchg_delta_fv_voter, "dtpt", false, 0);
		break;
	default:
		break;
	}

	return 0;
}

static int smart_charge_fv_control(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct smart_charge_info *info = data;
	static struct mca_smartchg_if_ops *if_ops;

	if (!data)
		return -1;

	for (int i = 0; i < MCA_SMARTCHG_IF_CHG_TYPE_END; i++) {
		if_ops = mca_smartchg_if_get_ops(i);
		if (!if_ops || !if_ops->set_delta_fv) {
			mca_log_info(" %d %p\n", i, if_ops);
			continue;
		}
		if_ops->set_delta_fv(if_ops->data, effective_result);
	}
	info->delta_fv = effective_result;
	mca_log_err("smartchg_delta_fv %d\n", effective_result);

	return 0;
}

static int smart_charge_ichg_control(struct mca_votable *votable,
	void *data, int effective_result, const char *effective_client)
{
	struct smart_charge_info *info = data;
	static struct mca_smartchg_if_ops *if_ops;

	if (!data)
		return -1;

	if (effective_result)
		info->delta_ichg = effective_result;
	else
		info->delta_ichg = 0;

	mca_log_err("smartchg_delta_ichg %d\n", effective_result);
	for (int i = 0; i < MCA_SMARTCHG_IF_CHG_TYPE_END; i++) {
		if_ops = mca_smartchg_if_get_ops(i);
		if (!if_ops || !if_ops->set_delta_ichg)
			continue;
		if_ops->set_delta_ichg(if_ops->data, effective_result);
	}

	return 0;
}

static int smart_charge_init_voter(struct smart_charge_info *info)
{
	info->smartchg_delta_fv_voter = mca_create_votable("smartchg_delta_fv", MCA_VOTE_MAX,
		smart_charge_fv_control, SMART_CHARGE_DELTA_FV_DEFAULT_VALUE, info);
	if (IS_ERR(info->smartchg_delta_fv_voter)) {
		mca_log_err("smartchg_delta_fv voteable failed\n");
		return -1;
	}

	info->smartchg_delta_ichg_voter = mca_create_votable("smartchg_delta_ichg", MCA_VOTE_MAX,
		smart_charge_ichg_control, SMART_CHARGE_DELTA_ICHG_DEFAULT_VALUE, info);
	if (IS_ERR(info->smartchg_delta_ichg_voter)) {
		mca_log_err("smartchg_delta_ichg voteable failed\n");
		return -1;
	}

	return 0;
}

static ssize_t smart_charge_sysfs_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mca_sysfs_attr_info *attr_info;
	ssize_t count = 0;
	struct smart_charge_info *info = dev_get_drvdata(dev);
	int val = 0;

	attr_info = mca_sysfs_lookup_attr(attr->attr.name,
		smartchg_sysfs_field_tbl, SMARTCHG_SYSFS_ATTRS_SIZE);
	if (!attr_info)
		return -1;

	if (!info) {
		mca_log_err("%s dev_driverdata is null\n", __func__);
		return -1;
	}

	switch (attr_info->sysfs_attr_name) {
	case MCA_PROP_SMARTCHG:
		val = get_smart_chg_message();
		count = scnprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case MCA_PROP_SMARTBATT:
		val = get_smart_batt_message();
		count = scnprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case MCA_PROP_SMARTNIGHT:
		val = get_smart_night_message();
		count = scnprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case MCA_PROP_SMARTCHG_FV:
		val = mca_get_client_vote(info->smartchg_delta_fv_voter, "smart_chg");
		val =  val < 0 ? 0  : val;
		count = scnprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case MCA_PROP_SMARTCHG_ICHG:
		val = mca_get_client_vote(info->smartchg_delta_ichg_voter, "smart_chg");
		val =  val < 0 ? 0  : val;
		count = scnprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case MCA_PROP_POSTURE:
		val = info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].use_fake_value ?
			info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].fake_value :
			info->smart_chg_data.smart_chgcfg[SMART_CHG_SENSE_CHG].func_value;
		count = scnprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case MCA_PROP_SCENE:
		val = mca_smartchg_get_scene();
		count = scnprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case MCA_PROP_BOARD_TEMP:
		count = scnprintf(buf, PAGE_SIZE, "%d\n", info->board_temp);
		break;
	case MCA_PROP_SMART_SIC_MODE:
		count = scnprintf(buf, PAGE_SIZE, "%d\n", info->smart_sic_mode);
		break;
	default:
		break;
	}

	return count;
}

static ssize_t smart_charge_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mca_sysfs_attr_info *attr_info;
	struct smart_charge_info *info = dev_get_drvdata(dev);
	int val;

	attr_info = mca_sysfs_lookup_attr(attr->attr.name,
		smartchg_sysfs_field_tbl, SMARTCHG_SYSFS_ATTRS_SIZE);
	if (!attr_info)
		return -1;

	if (!info) {
		mca_log_err("%s dev_driverdata is null\n", __func__);
		return -1;
	}

	if (kstrtoint(buf, 0, &val))
		return -EINVAL;

	switch (attr_info->sysfs_attr_name) {
	case MCA_PROP_SMARTCHG:
		smartcharging_receive_control_message(val);
		break;
	case MCA_PROP_SMARTBATT:
		handle_smart_batt_message(val);
		break;
	case MCA_PROP_SMARTNIGHT:
		handle_smart_night_message(val);
		break;
	case MCA_PROP_SMARTCHG_FV:
		val = val * info->cell_type;
		mca_vote(info->smartchg_delta_fv_voter, "smart_chg", true, val);
		break;
	case MCA_PROP_SMARTCHG_ICHG:
		mca_vote(info->smartchg_delta_ichg_voter, "smart_chg", true, val);
		break;
	case MCA_PROP_SCENE:
		mca_smartchg_set_scene(val);
		break;
	case MCA_PROP_BOARD_TEMP:
		mca_smartchg_set_board_temp(val);
		break;
	default:
		break;
	}

	mca_log_info("set the %d value = %d\n", attr_info->sysfs_attr_name, val);

	return count;
}

static int smart_charge_sysfs_create_group(struct device *dev)
{
	mca_sysfs_init_attrs(smartchg_sysfs_attrs, smartchg_sysfs_field_tbl,
		SMARTCHG_SYSFS_ATTRS_SIZE);
	return mca_sysfs_create_link_group("charger", "smart_charge",
	 dev, &smartchg_sysfs_attr_group);
}

static void smart_charge_sysfs_remove_group(struct device *dev)
{
	mca_sysfs_remove_link_group("charger", "smart_charge",
		dev, &smartchg_sysfs_attr_group);
}

static int smart_charge_handle_baa_data(struct smart_charge_info *info)
{
	struct smart_basp_header *basp_header = (struct smart_basp_header *)info->mmap_addr;
	struct smart_batt_spec *pbatt_spec = NULL;
	static struct mca_smartchg_if_ops *if_ops;
	int offset = 0;

	if (!basp_header) {
		mca_log_err("mmap_addr is NULL\n");
		return -1;
	}

#if 0
	int *raw = (int *)(info->mmap_addr);
	for (int i = 0; i < basp_header->total_len / sizeof(int) - 10; i += 10) {
		mca_log_err("raw[%d]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
			i, raw[i], raw[i + 1], raw[i + 2], raw[i + 3], raw[i + 4],
			raw[i + 5], raw[i + 6], raw[i + 7], raw[i + 8], raw[i + 9]);
	}
#endif

	// update buck para
	offset = sizeof(struct smart_basp_header);
	if_ops = mca_smartchg_if_get_ops(MCA_SMARTCHG_IF_CHG_TYPE_JEITA);
	if (!if_ops || !if_ops->update_baa_para) {
		mca_log_err("jeita update_baa_para is null\n");
	} else {
		mca_log_err("jeita update_baa_para offset: %d, size: %d/%d\n",
			offset, basp_header->jeita_ffc_term_size, basp_header->jeita_normal_term_size);
		if_ops->update_baa_para(if_ops->data, info->mmap_addr + offset,
			basp_header->jeita_ffc_term_size, basp_header->jeita_normal_term_size);
	}

	// get wired para offset
	offset += basp_header->jeita_ffc_term_size * sizeof(struct smart_batt_jeita_term_para);
	offset += basp_header->jeita_normal_term_size * sizeof(struct smart_batt_jeita_term_para);

	// update wired para
	if_ops = mca_smartchg_if_get_ops(MCA_SMARTCHG_IF_CHG_TYPE_QC);
	if (!if_ops || !if_ops->update_baa_para) {
		mca_log_err("wired qc update_baa_para is null\n");
	} else {
		mca_log_err("wired qc update_baa_para offset: %d, size: %d/%d\n",
			offset, basp_header->wired_ffc_size, basp_header->wired_normal_size);
		if_ops->update_baa_para(if_ops->data, info->mmap_addr + offset,
			basp_header->wired_ffc_size, basp_header->wired_normal_size);
	}

	// get wls para offset
	for (int i = 0; i < basp_header->wired_ffc_size; i++) {
		pbatt_spec = (struct smart_batt_spec *)(info->mmap_addr + offset);
		offset += offsetof(struct smart_batt_spec, steps) + pbatt_spec->step_size * sizeof(struct smart_batt_spec_curve);
	}
	for (int i = 0; i < basp_header->wired_normal_size; i++) {
		pbatt_spec = (struct smart_batt_spec *)(info->mmap_addr + offset);
		offset += offsetof(struct smart_batt_spec, steps) + pbatt_spec->step_size * sizeof(struct smart_batt_spec_curve);
	}

	// update wls para
	if_ops = mca_smartchg_if_get_ops(MCA_SMARTCHG_IF_CHG_TYPE_WL_QC);
	if (!if_ops || !if_ops->update_baa_para) {
		mca_log_err("wls qc update_baa_para is null\n");
	} else {
		mca_log_err("wls qc update_baa_para offset: %d, size: %d/%d\n",
			offset, basp_header->wls_ffc_size, basp_header->wls_normal_size);
		if_ops->update_baa_para(if_ops->data, info->mmap_addr + offset,
			basp_header->wls_ffc_size, basp_header->wls_normal_size);
	}

	return 0;
}

static int smart_charge_cal_checksum(struct smart_charge_info *info)
{
	struct smart_basp_header *basp_header = NULL;
	uint32_t checksum = 0;
	size_t i = 0;
	size_t total_len;
	char *temp_buff = info->mmap_addr;
	size_t buffer_size = info->mmap_size;

	if (!temp_buff) {
		 mca_log_err("mmap_addr is NULL\n");
		return -1;
	}

	basp_header = kmalloc(sizeof(struct smart_basp_header), GFP_KERNEL);
	if (!basp_header) {
		mca_log_err("Alloc header failed\n");
		return -1;
	}
	memcpy(basp_header, temp_buff, sizeof(struct smart_basp_header));

	mca_log_err("header: %d, %d, %d; %d, %d; %d, %d; %d, %d\n",
		basp_header->checksum, basp_header->type, basp_header->total_len,
		basp_header->jeita_ffc_term_size, basp_header->jeita_normal_term_size,
		basp_header->wired_ffc_size, basp_header->wired_normal_size,
		basp_header->wls_ffc_size, basp_header->wls_normal_size);

	total_len = basp_header->total_len;
	if ((total_len > buffer_size && info->map_flag) || total_len < sizeof(struct smart_basp_header)) {
		mca_log_err("Invalid total_len: %zu\n", total_len);
		kfree(basp_header);
		return -1;
	}

	for (i = sizeof(int); i < total_len; i++)
		checksum += (uint8_t)temp_buff[i];

	mca_log_err("cal checksum %d, origin checksum %d\n", checksum, basp_header->checksum);
	if (checksum != basp_header->checksum) {
		kfree(basp_header);
		return -1;
	}

	kfree(basp_header);
	return 0;
}

static void smart_charge_handle_mmap_data(struct smart_charge_info *info)
{
	if (!info->mmap_addr || (!info->mmap_size && info->map_flag)) {
		mca_log_err("Invalid mmap buffer\n");
		goto out;
	}
	if (smart_charge_cal_checksum(info)) {
		mca_log_err("checksum failed\n");
		goto out;
	}

	if (info->enable_fv_dec_by_cc)
		smart_charge_handle_baa_data(info);

out:
	info->map_flag = 0;
	if (info->mmap_addr) {
		kfree(info->mmap_addr);
		info->mmap_addr = NULL;
		info->mmap_size = 0;
	}
}

static int smart_charge_file_open(struct inode *inode, struct file *filp)
{
	struct smart_charge_info *info = NULL;

	info = container_of(inode->i_cdev, struct smart_charge_info, pri_dev);
	if (info->mmap_addr)
		return 0;

	info->mmap_addr = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!info->mmap_addr)
		return -1;

	mca_log_err("malloc memery succ\n");
	filp->private_data = info;

	return 0;
}

static int smart_charge_file_close(struct inode *inode, struct file *filp)
{
	struct smart_charge_info *info = NULL;

	info = container_of(inode->i_cdev, struct smart_charge_info, pri_dev);
	if (!info) {
		mca_log_err("Failed to get smart_charge_info from cdev\n");
		return -EINVAL;
	}

	mutex_lock(&info->data_lock);
	if (info && info->mmap_addr)
		smart_charge_handle_mmap_data(info);
	mutex_unlock(&info->data_lock);
	mca_log_err("release memery succ\n");

	return 0;
}

static int smart_charge_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct smart_charge_info *info = filp->private_data;
	unsigned long size = vma->vm_end - vma->vm_start;
	int ret;

	if (size > MAX_MMAP_SIZE) {
		mca_log_err("mmap size %lu exceeds limit %d\n", size, MAX_MMAP_SIZE);
		return -EINVAL;
	}


	if (!info || !info->mmap_addr || info->map_flag) {
		mca_log_err("map addr error\n");
		return -1;
	}
	if (vma->vm_end - vma->vm_start > PAGE_SIZE) {
		mca_log_err("map addr len error\n");
		return -1;
	}

	info->mmap_size = size;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	ret = remap_pfn_range(vma, vma->vm_start, virt_to_phys(info->mmap_addr)>>PAGE_SHIFT,
		vma->vm_end - vma->vm_start, vma->vm_page_prot);

	mca_log_err("vma start 0x%x, prot %d\n", vma->vm_start, vma->vm_page_prot);
	info->map_flag = 1;

	if (ret)
		mca_log_err("remap pfn failed %d\n", ret);

	return ret;
}

static long smart_charge_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	struct smart_charge_info *info = file->private_data;
	int ret;
	int err = 0;

#if (KERNEL_VERSION(5, 0, 0) > LINUX_VERSION_CODE)
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
#else
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok((void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok((void *)arg, _IOC_SIZE(cmd));
#endif
	if (err)
		return -EFAULT;

	switch (cmd) {
	case SMARTCHG_DATA_CMD:
		if (!info->mmap_addr) {
			mca_log_err("mmap addr is null\n");
			return -EINVAL;
		}
		ret = copy_from_user(info->mmap_addr, (void __user *)arg, PAGE_SIZE);
		mca_log_info("data ret = %d\n", ret);
		if (ret)
			return -EINVAL;
		break;
	default:
		mca_log_err("invalid cmd\n");
		return -EINVAL;
	}
	return 0;
}

static int smart_charge_panel_notifier_cb(struct notifier_block *nb,
			unsigned long event, void *val)
{
	struct smart_charge_info *info = container_of(nb, struct smart_charge_info, panel_nb);
	int tmp = 0;

	switch (event) {
	case MCA_EVENT_PANEL_SCREEN_STATE_CHANGE:
		tmp = *(int *)val;
		mca_log_info("update screen_state: %d => %d\n", info->screen_state, tmp);
		info->screen_state = tmp;
		cancel_delayed_work_sync(&info->smart_sense_chg_work);
		schedule_delayed_work(&info->smart_sense_chg_work, 0);
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}

static const struct file_operations g_smt_chg_file_ops = {
	.owner = THIS_MODULE,
	.open = smart_charge_file_open,
	.release = smart_charge_file_close,
	.mmap = smart_charge_mmap,
#ifdef CONFIG_COMPAT
	.compat_ioctl = smart_charge_ioctl,
#endif /* CONFIG_COMPAT */
	.unlocked_ioctl = smart_charge_ioctl,
};

static int smart_charge_parse_dt(struct smart_charge_info *info)
{
	struct device_node *node = info->dev->of_node;
	int len = 0;
	int ret = 0;
	int batt_ichg_cfg[CC_ICHG_MAX_GROUP * SMARTCHG_MODE_PARA_MAX];

	(void)mca_parse_dts_u32(node, "cell_type", &info->cell_type, MCA_CELL_TYPE_1S);
	mca_parse_dts_u32(node, "enable_fv_dec_by_cc", &info->enable_fv_dec_by_cc, 0);
	mca_parse_dts_u32(node, "support_csd", &info->support_csd, 0);
	info->support_sensechg_v2 = of_property_read_bool(node, "support_sensechg_v2");
	info->support_cc_ichg = of_property_read_bool(node, "support_cc_ichg");
	if (info->support_cc_ichg) {
		len = mca_parse_dts_u32_count(node, "cc_ichg_cfg", CC_ICHG_MAX_GROUP, SMARTCHG_MODE_PARA_MAX);
		if (len < 0) {
			mca_log_err("parse cc_ichg_cfg failed\n");
			return -1;
		}

		ret = mca_parse_dts_u32_array(node, "cc_ichg_cfg", batt_ichg_cfg, CC_ICHG_MAX_GROUP * SMARTCHG_MODE_PARA_MAX);
		if (ret) {
			mca_log_err(" parse cc_ichg_cfg failed\n");
			return -1;
		}

		for (int row = 0; row < (len / SMARTCHG_MODE_PARA_MAX); row++) {
			int col = row * SMARTCHG_MODE_PARA_MAX;

			info->ichg_cc_table[row].cc_max = batt_ichg_cfg[col + SMARTCHG_MODE_CYCLECOUNT];
			info->ichg_cc_table[row].delta_ichg = batt_ichg_cfg[col + SMARTCHG_MODE_DELTA_ICHG];
			mca_log_err("%d, %d\n", info->ichg_cc_table[row].cc_max, info->ichg_cc_table[row].delta_ichg);
		}
	}

	return 0;

}

static int smart_charge_probe(struct platform_device *pdev)
{
	struct smart_charge_info *info;
	static int probe_cnt;
	int ret = 0, online = 0, wireless_online = 0;

	mca_log_info("probe_cnt = %d\n", ++probe_cnt);
	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info) {
		mca_log_err("out of memory\n");
		return -ENOMEM;
	}

	mutex_init(&info->data_lock);
	info->dev = &pdev->dev;
	platform_set_drvdata(pdev, info);
	global_smartchg_info = info;
	ret = smart_charge_parse_dt(info);
	if (ret) {
		mca_log_err("parse dt failed\n");
		return -1;
	}
	smart_charge_sysfs_create_group(info->dev);
	ret = smart_charge_init_voter(info);
	if (ret) {
		mca_log_err("init voter err\n");
		return -1;
	}

	(void)mca_strategy_ops_register(STRATEGY_FUNC_TYPE_SMARTCHG, mca_smart_charge_process_event, NULL, NULL, info);

	info->panel_nb.notifier_call = smart_charge_panel_notifier_cb;
	mca_event_block_notify_register(MCA_EVENT_TYPE_PANEL, &info->panel_nb);

	INIT_DELAYED_WORK(&info->smart_charge_work, smart_charge_workfunc);
	INIT_DELAYED_WORK(&info->smart_sense_chg_work, smart_charge_sense_chg_workfunc);
	INIT_DELAYED_WORK(&info->smart_soc_limit_work, smart_charge_soc_limit_workfunc);

	(void)mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_BUCK_CHARGE,
		STRATEGY_STATUS_TYPE_ONLINE, &online);
	(void)mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_BASIC_WIRELESS,
		STRATEGY_STATUS_TYPE_ONLINE, &wireless_online);
	ret = alloc_chrdev_region(&info->dev_num, 0, 1, "smart_charge");
	if (ret) {
		mca_log_err("alloc chrdev num failed\n");
		return ret;
	}

	cdev_init(&info->pri_dev, &g_smt_chg_file_ops);
	info->pri_dev.owner = THIS_MODULE;
	ret = cdev_add(&info->pri_dev, info->dev_num, 1);
	if (ret) {
		mca_log_err("cdev add failed\n");
		goto error1;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 9))
	info->smart_charge_class = class_create("smart_charge");
#else
	info->smart_charge_class = class_create(THIS_MODULE, "smart_charge");
#endif
	if (!info->smart_charge_class) {
		mca_log_err("create smart charge class failed\n");
		goto error2;
	}

	info->sys_dev = device_create(info->smart_charge_class, NULL, info->dev_num,
		NULL, "smart_charge_cdev");
	if (!info->sys_dev) {
		mca_log_err("create smart charge device failed\n");
		goto error3;
	}

	if (online || wireless_online) {
		info->online = 1;
		schedule_delayed_work(&info->smart_charge_work, 0);
	}

	info->fake_scene = SMART_CHARGE_FAKE_SCENE_DEFAULT;

	mca_log_info("done\n");
	return 0;

error3:
	class_destroy(info->smart_charge_class);
error2:
	cdev_del(&info->pri_dev);
error1:
	unregister_chrdev_region(info->dev_num, 1);

	return -1;
}

static int smart_charge_remove(struct platform_device *pdev)
{
	struct smart_charge_info *info = platform_get_drvdata(pdev);

	unregister_chrdev_region(info->dev_num, 1);
	cdev_del(&info->pri_dev);
	mutex_destroy(&info->data_lock);
	device_destroy(info->smart_charge_class, info->dev_num);
	class_destroy(info->smart_charge_class);
	smart_charge_sysfs_remove_group(&pdev->dev);
	mca_event_block_notify_unregister(MCA_EVENT_TYPE_PANEL, &info->panel_nb);

	return 0;
}

static const struct of_device_id match_table[] = {
	{.compatible = "xiaomi,smart_charge"},
	{},
};

static struct platform_driver smart_charge_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "smart_charge",
		.of_match_table = match_table,
	},
	.probe = smart_charge_probe,
	.remove = smart_charge_remove,
};

static int __init smart_charge_init(void)
{
	return platform_driver_register(&smart_charge_driver);
}
module_init(smart_charge_init);

static void __exit smart_charge_exit(void)
{
	platform_driver_unregister(&smart_charge_driver);
}
module_exit(smart_charge_exit);

MODULE_DESCRIPTION("smart charge driver");
MODULE_AUTHOR("liuzhengqing@xiaomi.com");
MODULE_LICENSE("GPL v2");
