// SPDX-License-Identifier: GPL-2.0
/*
 * mca_connector_antiburn.c
 *
 * connector antiburn driver
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
#include <linux/of_gpio.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <mca/common/mca_event.h>
#include <mca/common/mca_log.h>
#include <mca/common/mca_parse_dts.h>
#include <linux/iio/consumer.h>
#include <linux/debugfs.h>
#include <mca/common/mca_sysfs.h>
#include <linux/ktime.h>
#include <mca/platform/platform_buckchg_class.h>
#include <mca/common/mca_hwid.h>
#include <mca/protocol/protocol_class.h>
#include <mca/protocol/protocol_pd_class.h>
#include <mca/platform/platform_cp_class.h>
#include <mca/protocol/protocol_qc_class.h>
#include <mca/common/mca_charge_mievent.h>
#include <mca/common/mca_workqueue.h>
#include "inc/mca_connector_antiburn.h"
#include <soc/xring/xr_pmic.h>
#include <mca/platform/platform_sc6601a_cid_class.h>
#include <mca/strategy/strategy_class.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_connector_antiburn"
#endif

#define PROJECT_HW_UDP 0X00
#define PROJECT_HW_PREP0 0X01
#define PROJECT_HW_P00 0X02
#define PROJECT_HW_P01 0X03
#define PROJECT_HW_P10 0X04
#define PROJECT_HW_P11 0X05
#define PROJECT_HW_P12 0X06
#define PROJECT_HW_P20 0X07
#define PROJECT_HW_MP 0X08

#define CONNECTOR_ANTIBURN_WORK_INTERVAL_FAST 1000
#define CONNECTOR_ANTIBURN_WORK_INTERVAL_NORMAL 5000
#define CONNECTOR_ANTIBURN_WORK_DELAY_CHECK 300000 // 5min

#define CONNECTOR_ANTIBURN_TRIGGER_TEMP 65
#define CONNECTOR_ANTIBURN_RECHARGE_TEMP 55
#define CONNECTOR_ANTIBURN_TEMPERATURE_INCREASE_RATE 4
#define NTC_SCALE_TEMP 1000
#define COMBINED_SENSOR_BOARD_CONNECTOR_ANTIBURN_TRIGGER_TEMP 60
#define COMBINED_RATE_CONNECTOR_ANTIBURN_TRIGGER_TEMP 35
#define THERMAL_SENSOR_BOARD_TRIGGER_TEMP 50

static struct connector_antiburn *g_conn;
#define XRING_PMIC_GPIO_NUM          3

int connector_antiburn_is_triggered(void)
{
	if(!g_conn)
		return 0;

	return g_conn->triggered;
}
EXPORT_SYMBOL(connector_antiburn_is_triggered);

static void connector_temp_uevent(int temp)
{
	char event[MCA_EVENT_NOTIFY_SIZE] = { 0 };
	int len = 0;
	struct mca_event_notify_data event_data;

	mca_log_info("connector temp uevent notify\n");

	len = snprintf(event, MCA_EVENT_NOTIFY_SIZE,
		"POWER_SUPPLY_CONNECTOR_TEMP=%d", temp);
	event_data.event = event;
	event_data.event_len = len;
	mca_event_report_uevent(&event_data);
}

static void adapter_reset_vsafe0V_uevent(int is_reset_vsafe0V)
{
	char event[MCA_EVENT_NOTIFY_SIZE] = { 0 };
	int len = 0;
	struct mca_event_notify_data event_data;

	mca_log_info("adapter vbus drop 0V notify\n");

	len = snprintf(event, MCA_EVENT_NOTIFY_SIZE,
		"POWER_SUPPLY_ADAPTER_RESET_VSAFE0V=%d", is_reset_vsafe0V);
	event_data.event = event;
	event_data.event_len = len;
	mca_event_report_uevent(&event_data);
}

static void connector_ntc_alarm_uevent(int ntc_alarm)
{
	char event[MCA_EVENT_NOTIFY_SIZE] = { 0 };
	int len = 0;
	struct mca_event_notify_data event_data;

	mca_log_info("ntc alarm notify\n");

	len = snprintf(event, MCA_EVENT_NOTIFY_SIZE,
		"POWER_SUPPLY_NTC_ALARM=%d", ntc_alarm);
	event_data.event = event;
	event_data.event_len = len;
	mca_event_report_uevent(&event_data);
}

static int connector_antiburn_get_temp_increase_rate(struct connector_antiburn *conn)
{
	static ktime_t last_time;
	static int last_connector_temp[CONNECTOR_PROP_TEMP_MAX];
	ktime_t current_time, time_gap;
	int temp_gap[CONNECTOR_PROP_TEMP_MAX] = {0, 0};
	int ret = 0;
	int i = 0;

	if (conn->reset_rate) {
		last_time = ktime_get();
		for (i = 0; i < CONNECTOR_PROP_TEMP_MAX; i++) {
			last_connector_temp[i] = conn->temperature[i];
		}
		conn->reset_rate = false;
	}

	current_time = ktime_get();
	time_gap = ktime_to_ms(ktime_sub(current_time, last_time));
	if (time_gap <= 0) {
		mca_log_err("timestamp is error, time_gap is %ld, current time is %ld, last time is %ld\n", time_gap, current_time, last_time);
		for (i = 0; i < CONNECTOR_PROP_TEMP_MAX; i++) {
			conn->temp_increase_rate[i] = 0;
		}
		ret = -1;
		return ret;
	}

	for (i = 0; i < CONNECTOR_PROP_TEMP_MAX; i++) {
		temp_gap[i] = conn->temperature[i] - last_connector_temp[i];
		conn->temp_increase_rate[i] = temp_gap[i] * 1000 / time_gap;
		last_connector_temp[i] = conn->temperature[i];
	}

	last_time = current_time;
	return ret;
}

static int connector_antiburn_get_temperature(struct connector_antiburn *conn, int index)
{
	int connector_temp = 25;

	if (conn->fake_connector_temp[index]) {
		connector_temp = conn->fake_connector_temp[index];
		return connector_temp;
	}

	switch (index) {
	case CONNECTOR_PROP_TEMP_1:
		platform_class_buckchg_ops_get_bus_tsns(MAIN_BUCK_CHARGER, &connector_temp);
		break;
	case CONNECTOR_PROP_TEMP_2:
		if (conn->isvalid_thermal_zone) {
			thermal_zone_get_temp(conn->tzd_conn2, &connector_temp);
			mca_log_info("get thermal zone temp\n");
		}
		else {
			platform_class_buckchg_ops_get_bus_tsns(MAIN_BUCK_CHARGER, &connector_temp);
			mca_log_info("get subpmic tbus temp\n");
		}
		break;
	default:
		mca_log_err("invaid ntc\n");
		break;
	}
	mca_log_info("CONNECTOR_PROP_TEMP read %d, index is %d\n", connector_temp, index);
	if (index == CONNECTOR_PROP_TEMP_2 && conn->isvalid_thermal_zone)
		connector_temp /= NTC_SCALE_TEMP;

	return connector_temp;
}

static void connector_antiburn_ensure_vbus_sense5V(struct connector_antiburn *conn)
{
	int i = 0;
	int MAX_COUNT = 5;
	int target_volt = 5000;
	int cmd = USBPD_UVDM_RESET_VSAFE0V;
	int bus_volt = 0;
	int target_curr = 1500;
	int real_type = XM_CHARGER_TYPE_UNKNOW;
	int sense_vbus_default = 6000;
	int vbus_disable_volt = 3600;
	unsigned int data;

	platform_class_cp_enable_adc(CP_ROLE_MASTER, true);
	for (i = 0; i < MAX_COUNT; i++) {
		protocol_class_get_adapter_type(ADAPTER_PROTOCOL_PD, &real_type);
		mca_log_err("real_type is %d, bus_volt is %d", real_type, bus_volt);
		switch (real_type) {
		case XM_CHARGER_TYPE_HVDCP2:
		case XM_CHARGER_TYPE_HVDCP3:
		case XM_CHARGER_TYPE_HVDCP3_B:
		case XM_CHARGER_TYPE_HVDCP3P5:
			platform_class_cp_get_bus_voltage(CP_ROLE_MASTER, &bus_volt);
			if (bus_volt < sense_vbus_default) {
				platform_class_cp_enable_adc(CP_ROLE_MASTER, false);
				return;
			}
			protocol_class_qc_set_volt(TYPEC_PORT_0, target_volt);
			msleep(200);
			break;
		case XM_CHARGER_TYPE_PD:
		case XM_CHARGER_TYPE_PPS:
		case XM_CHARGER_TYPE_PD_VERIFY:
			protocol_class_pd_request_vdm_cmd(TYPEC_PORT_0, cmd, &data, 0);
			msleep(200);
			platform_class_cp_get_bus_voltage(CP_ROLE_MASTER, &bus_volt);
			if (bus_volt < sense_vbus_default) {
				if (bus_volt < vbus_disable_volt) {
					conn->is_reset_vsafe0V = 1;
					adapter_reset_vsafe0V_uevent(conn->is_reset_vsafe0V);
				}
				platform_class_cp_enable_adc(CP_ROLE_MASTER, false);
				return;
			}
			protocol_class_set_adapter_volt_and_curr(ADAPTER_PROTOCOL_PD, target_volt, target_curr);
			break;
		default:
			return;
		}

	}
	platform_class_cp_enable_adc(CP_ROLE_MASTER, false);
}

#define VBUS_UV_THRESHOLD 4100000
#define VBUS_DEALY_TIME 50
static void connector_antiburn_check_status(struct connector_antiburn *conn)
{
	int connector_temp = 25;
	int temp_increase_rate = 0;
	int otg_enable;
	int i;
	int vbus_uv;
	static int last_temp0 = 0;
	bool adc_en = false;
	int fake_temp_set = 0;

	for (i = 0; i < CONNECTOR_PROP_TEMP_MAX; i++) {
		connector_temp = max(connector_temp, conn->temperature[i]);
		temp_increase_rate = max(temp_increase_rate, conn->temp_increase_rate[i]);
	}

	if (conn->support_base_flip)
		connector_temp = min(conn->temperature[0], conn->temperature[1]);

	protocol_class_pd_get_otg_plugin_status(TYPEC_PORT_0, &conn->otg_plugin_status);
	platform_class_buckchg_ops_get_online(MAIN_BUCK_CHARGER, &conn->usb_online);
	platform_class_buckchg_ops_get_cid_status(MAIN_BUCK_CHARGER, &conn->cid_status);
	platform_class_buckchg_ops_get_adc_enable(MAIN_BUCK_CHARGER, &adc_en);

	for (i = 0; i < CONNECTOR_PROP_TEMP_MAX; i++) {
		if(conn->fake_connector_temp[i])
			fake_temp_set += 1;
	}

	mca_log_info("connector_temp:%d triggered:%d, cid_status:%d, otg_plugin_status: %d, usb_online:%d, adc:%d, fake:%d\n",
		connector_temp, conn->triggered, conn->cid_status, conn->otg_plugin_status, conn->usb_online, adc_en, fake_temp_set);

	if(!adc_en && !fake_temp_set)
		return;

	if ((connector_temp >= conn->trigger_temp ||
		(connector_temp >= conn->comb_sensorboard_con_trigger_temp && conn->thermal_board_temp <= conn->max_thermal_board_temp) ||
		(connector_temp >= conn->comb_rate_conn_trigger_temp && temp_increase_rate >= conn->max_temp_increase_rate &&
		last_temp0 != 0)) && !conn->triggered) {
		mca_log_err("conn->usb_online is %d, conn->otg_plugin_status is %d",conn->usb_online, conn->otg_plugin_status);
		mca_log_err("conn_therm is %d, conn_therm2 %d, temp_increase_rate is %d, thermal_board_temp is %d\n",conn->temperature[0], conn->temperature[1], temp_increase_rate, conn->thermal_board_temp);
		conn->triggered = 1;
		conn->ntc_alarm = 1;
		connector_temp_uevent(connector_temp * 10);
		connector_ntc_alarm_uevent(conn->ntc_alarm);
		mca_event_block_notify(MCA_EVENT_TYPE_HW_INFO, MCA_EVENT_CONN_ANTIBURN_CHANGE, NULL);
		mca_charge_mievent_report(CHARGE_DFX_ANTI_BURN_TRIGGERED, &connector_temp, 1);
		msleep(200);
		connector_antiburn_ensure_vbus_sense5V(conn);
		if (conn->otg_plugin_status == 1) {
			otg_enable = 0;
			otg_enable = (conn->en_src << 16) | (conn->otg_boost_src << 8) | otg_enable;
			platform_class_buckchg_ops_set_boost_enable(MAIN_BUCK_CHARGER, otg_enable);
		}
		if (conn->support_hw_antiburn) {
			xr_pmic_gpio_set_value(XRING_PMIC_GPIO_NUM, 1);
			__pm_relax(conn->anti_wake_lock);
			mca_log_err("triggering hw anti burn");
			msleep(VBUS_DEALY_TIME);
			(void)platform_class_buckchg_ops_get_bus_volt(MAIN_BUCK_CHARGER, &vbus_uv);
			if (vbus_uv >= VBUS_UV_THRESHOLD)
				mca_charge_mievent_report(CHARGE_DFX_ANTIBURN_ERR, NULL, 0);
		}
	} else if (connector_temp <= conn->recharge_temp && temp_increase_rate < conn->max_temp_increase_rate && conn->triggered && !conn->cid_status) {
		conn->triggered = 0;
		conn->is_reset_vsafe0V = 0;
		mca_log_err("recovery antiburn conn->usb_online is %d, conn->otg_plugin_status is %d, conn->cid_status is %d\n",conn->usb_online, conn->otg_plugin_status, conn->cid_status);
		adapter_reset_vsafe0V_uevent(conn->is_reset_vsafe0V);
		if (conn->support_hw_antiburn) {
			xr_pmic_gpio_set_value(XRING_PMIC_GPIO_NUM, 0);
			__pm_relax(conn->anti_wake_lock);
			mca_log_err("close hw anti burn");
		}
		mca_event_block_notify(MCA_EVENT_TYPE_HW_INFO, MCA_EVENT_CONN_ANTIBURN_CHANGE, NULL);
		mca_charge_mievent_set_state(MIEVENT_STATE_END, CHARGE_DFX_ANTI_BURN_TRIGGERED);
	}

	last_temp0 = conn->temperature[0];

	if (conn->ntc_alarm == 1 && conn->cid_status == 0) {
		conn->ntc_alarm = 0;
		connector_ntc_alarm_uevent(conn->ntc_alarm);
	}
	if (conn->otg_plugin_status == 1 || conn->usb_online == 1)
		conn->monitor_interval = CONNECTOR_ANTIBURN_WORK_INTERVAL_FAST;//interval 1s
	else
		conn->monitor_interval = CONNECTOR_ANTIBURN_WORK_INTERVAL_NORMAL;//interval 5s
}

static int mca_connector_antiburn_process_event(int event, int value, void *data)
{
	struct connector_antiburn *conn = data;
	int ret = 0;

	if (conn == NULL)
		return -EINVAL;

	mca_log_info("receive event %d, value %d\n", event, value);
	switch (event) {
	case MCA_EVENT_USB_CONNECT:
	case MCA_EVENT_WIRELESS_CONNECT:
	case MCA_EVENT_OTG_CONNECT:
		if (conn->triggered)
			break;
		if (conn->reset_rate != 1)
			conn->reset_rate = 1;
		ret = platform_class_buckchg_ops_adc_enable(MAIN_BUCK_CHARGER, true);
		cancel_delayed_work_sync(&conn->monitor_work);
		mca_queue_delayed_work(&conn->monitor_work, msecs_to_jiffies(2000));
		break;
	case MCA_EVENT_USB_DISCONNECT:
	case MCA_EVENT_WIRELESS_DISCONNECT:
	case MCA_EVENT_OTG_DISCONNECT:
	case MCA_EVENT_CID_DISCONNECT:
		if (conn->triggered) {
			ret = platform_class_buckchg_ops_adc_enable(MAIN_BUCK_CHARGER, true);
			mca_log_info("anti trigger\n");
		} else {
			ret = platform_class_buckchg_ops_adc_enable(MAIN_BUCK_CHARGER, false);
			cancel_delayed_work_sync(&conn->monitor_work);
		}
		break;
	default:
		break;
	}
	return 0;
}

static void connector_antiburn_monitor_workfunc(struct work_struct *work)
{
	struct connector_antiburn *conn = container_of(work,
		struct connector_antiburn, monitor_work.work);
	int interval = conn->monitor_interval;
	int i;

	for (i = 0; i < CONNECTOR_PROP_TEMP_MAX; i++)
		conn->temperature[i] = connector_antiburn_get_temperature(conn, i);
	connector_antiburn_get_temp_increase_rate(conn);
	connector_antiburn_check_status(conn);

	mca_queue_delayed_work(&conn->monitor_work, msecs_to_jiffies(interval));
}


static void connector_antiburn_parse_dt(struct connector_antiburn *conn)
{
	struct device_node *np = conn->dev->of_node;

	mca_parse_dts_u32(np, "trigger_temp",
		&conn->trigger_temp, CONNECTOR_ANTIBURN_TRIGGER_TEMP);
	mca_parse_dts_u32(np, "recharge_temp",
		&conn->recharge_temp, CONNECTOR_ANTIBURN_RECHARGE_TEMP);
	mca_parse_dts_u32(np, "support_soft_antiburn",
		&conn->support_soft_antiburn, 1);
	mca_parse_dts_u32(np, "support_hw_antiburn",
		&conn->support_hw_antiburn, 1);
	mca_parse_dts_u32(np, "use_double_ntc",
		&conn->use_double_ntc, 0);
	mca_parse_dts_u32(np, "antiburn_otg_detect",
		&conn->otg_detect_en, 1);
	mca_parse_dts_u32(np, "monitor_interval",
		&conn->monitor_interval, CONNECTOR_ANTIBURN_WORK_INTERVAL_FAST);
	mca_parse_dts_u32(np, "max_temp_increase_rate",
		&conn->max_temp_increase_rate, CONNECTOR_ANTIBURN_TEMPERATURE_INCREASE_RATE);
	mca_parse_dts_string(np, "thermal-zone-name", &conn->thermal_zone_name);
	mca_parse_dts_string(np, "thermal-zone-name2", &conn->thermal_zone_name2);
	//mca_parse_dts_string(np, "thermal-board-temp-name", &conn->thermal_board_temp_name);
	mca_parse_dts_u32(np, "comb_sensorboard_con_trigger_temp",
		&conn->comb_sensorboard_con_trigger_temp, COMBINED_SENSOR_BOARD_CONNECTOR_ANTIBURN_TRIGGER_TEMP);
	mca_parse_dts_u32(np, "comb_rate_conn_trigger_temp",
		&conn->comb_rate_conn_trigger_temp, COMBINED_RATE_CONNECTOR_ANTIBURN_TRIGGER_TEMP);
	mca_parse_dts_u32(np, "max_thermal_board_temp",
		&conn->max_thermal_board_temp, THERMAL_SENSOR_BOARD_TRIGGER_TEMP);
	mca_parse_dts_u32(np, "otg_boost_src",
		&conn->otg_boost_src, BOOST_SRC_EXTERNAL);
	mca_parse_dts_u32(np, "en_src",
		&conn->en_src, OTG_EN_BOOST);
	conn->support_base_flip = of_property_read_bool(np, "support-base-flip");
}

static ssize_t antiburn_sysfs_show(struct device *dev,
		struct device_attribute *attr, char *buf);
static ssize_t antiburn_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct mca_sysfs_attr_info antiburn_sysfs_field_tbl[] = {
	mca_sysfs_attr_rw(antiburn_sysfs, 0664, CONNECTOR_PROP_TEMP_1, connector_temp_1),
	mca_sysfs_attr_rw(antiburn_sysfs, 0664, CONNECTOR_PROP_TEMP_2, connector_temp_2),
	mca_sysfs_attr_rw(antiburn_sysfs, 0664, CONNECTOR_PROP_RESET_VSAFE0V, reset_vsafe0V),
	mca_sysfs_attr_rw(antiburn_sysfs, 0664, CONNECTOR_PROP_NTC_ALARM, ntc_alarm),

};

#define ANTIBURN_SYSFS_ATTRS_SIZE   ARRAY_SIZE(antiburn_sysfs_field_tbl)

static struct attribute *antiburn_sysfs_attrs[ANTIBURN_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group antiburn_sysfs_attr_group = {
	.attrs = antiburn_sysfs_attrs,
};

static ssize_t antiburn_sysfs_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mca_sysfs_attr_info *attr_info;
	ssize_t count = 0;
	struct connector_antiburn *conn = dev_get_drvdata(dev);
	int temp;

	attr_info = mca_sysfs_lookup_attr(attr->attr.name,
		antiburn_sysfs_field_tbl, ANTIBURN_SYSFS_ATTRS_SIZE);
	if (!attr_info)
		return -1;

	if (!conn) {
		mca_log_err("%s dev_driverdata is null\n", __func__);
		return -1;
	}
	switch (attr_info->sysfs_attr_name) {
	case CONNECTOR_PROP_TEMP_1:
	case CONNECTOR_PROP_TEMP_2:
		temp = connector_antiburn_get_temperature(conn, attr_info->sysfs_attr_name) * 10;
		count = scnprintf(buf, PAGE_SIZE, "%d\n", temp);
		break;
	case CONNECTOR_PROP_RESET_VSAFE0V:
		temp = conn->is_reset_vsafe0V;
		count = scnprintf(buf, PAGE_SIZE, "%d\n", temp);
		break;
	case CONNECTOR_PROP_NTC_ALARM:
		temp = conn->ntc_alarm;
		count = scnprintf(buf, PAGE_SIZE, "%d\n", temp);
		break;
	default:
		break;
	}
	return count;
}

static ssize_t antiburn_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mca_sysfs_attr_info *attr_info;
	struct connector_antiburn *conn = dev_get_drvdata(dev);
	int val;

	attr_info = mca_sysfs_lookup_attr(attr->attr.name,
		antiburn_sysfs_field_tbl, ANTIBURN_SYSFS_ATTRS_SIZE);
	if (!attr_info)
		return -1;

	if (!conn) {
		mca_log_err("%s dev_driverdata is null\n", __func__);
		return -1;
	}

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	switch (attr_info->sysfs_attr_name) {
	case CONNECTOR_PROP_TEMP_1:
	case CONNECTOR_PROP_TEMP_2:
		conn->fake_connector_temp[attr_info->sysfs_attr_name] = val / 10;
		mca_log_err("set the %d ntc = %d\n", attr_info->sysfs_attr_name, val);
		break;
	default:
		break;
	}

	return count;
}

static int antiburn_sysfs_create_group(struct device *dev)
{
	mca_sysfs_init_attrs(antiburn_sysfs_attrs, antiburn_sysfs_field_tbl,
		ANTIBURN_SYSFS_ATTRS_SIZE);
	return mca_sysfs_create_link_group(SYSFS_DEV_5, "connector",
	 dev, &antiburn_sysfs_attr_group);
}

static int connector_antiburn_thermal_notifier_event(struct notifier_block *nb,
			unsigned long chg_event, void *val)
{
	struct connector_antiburn *conn = container_of(nb, struct connector_antiburn, thermal_board_nb);

	switch (chg_event) {
	case MCA_EVENT_THERMAL_BOARD_TEMP_CHANGE:
		conn->thermal_board_temp  = *(int *)val/NTC_SCALE_TEMP;
		mca_log_info("get thermal_board_temp: %d\n", conn->thermal_board_temp);
		break;
	default:
		mca_log_info("not supported thermal board notifier event: %lu\n", chg_event);
		break;
	}
	return NOTIFY_DONE;
}

static int connector_antiburn_dump_log_head(void *data, char *buf, int size)
{
	return snprintf(buf, size, "port_temp port_temp1 shell_temp ");
}

static int connector_antiburn_dump_log_context(void *data, char *buf, int size)
{
	struct connector_antiburn *conn = (struct connector_antiburn *)data;
	int temp1;
	int temp2 = -1;

	if (!data)
		return snprintf(buf, size, "%-10d%-11d%-11d", -1, -1, -1);

	temp1 = connector_antiburn_get_temperature(conn, CONNECTOR_PROP_TEMP_1);
	if (conn->use_double_ntc)
		temp2 = connector_antiburn_get_temperature(conn, CONNECTOR_PROP_TEMP_2);

	return snprintf(buf, size, "%-10d%-11d%-11d",
		temp1, temp2, conn->thermal_board_temp);
}

static struct mca_log_charge_log_ops g_connector_antiburn_log_ops = {
	.dump_log_head = connector_antiburn_dump_log_head,
	.dump_log_context = connector_antiburn_dump_log_context,
};

static int connector_antiburn_probe(struct platform_device *pdev)
{
	struct connector_antiburn *conn;
	static int probe_cnt;
	const struct mca_hwid *hwid = mca_get_hwid_info();
	int online = 0;
	int ret = 0;
	int hwid_info = PROJECT_HW_P20;

	mca_log_info("probe_cnt = %d\n", ++probe_cnt);

	conn = devm_kzalloc(&pdev->dev, sizeof(*conn), GFP_KERNEL);
	if (!conn) {
		mca_log_err("out of memory\n");
		goto err;
	}
	conn->dev = &pdev->dev;
	platform_set_drvdata(pdev, conn);

	connector_antiburn_parse_dt(conn);

	conn->tzd_conn2 = thermal_zone_get_zone_by_name(conn->thermal_zone_name2);
	if (IS_ERR(conn->tzd_conn2)) {
			mca_log_err("thermal zone get conn_therm2 failed\n");
			goto err;
	}

	if (hwid == NULL)
		goto err;

	if (hwid->major_version == 0 && hwid->minor_version == 0)
		hwid_info = PROJECT_HW_P00;
	else if (hwid->major_version == 0 && hwid->minor_version == 1)
		hwid_info = PROJECT_HW_P01;
	else if (hwid->major_version == 1 && hwid->minor_version == 0)
		hwid_info = PROJECT_HW_P10;
	else if (hwid->major_version == 1 && hwid->minor_version == 1)
		hwid_info = PROJECT_HW_P11;
	else if (hwid->major_version == 1 && hwid->minor_version == 2)
		hwid_info = PROJECT_HW_P12;
	else if (hwid->major_version == 2 && hwid->minor_version == 0)
		hwid_info = PROJECT_HW_P20;
	else if (hwid->major_version == 9 && hwid->minor_version == 0)
		hwid_info = PROJECT_HW_MP;
	else if (hwid->major_version == 0 && hwid->minor_version == 9)
		hwid_info = PROJECT_HW_UDP;
	else
		hwid_info = PROJECT_HW_P20;

	if ((hwid->platform_version == 2) || ((hwid->platform_version == 1) && (hwid_info >= PROJECT_HW_P11)))
		conn->isvalid_thermal_zone = true;
	else
		conn->isvalid_thermal_zone = false;
	mca_log_info("NTC2 use thermal zone = %d, hwid = P%d%d, hwid_value = 0x%x\n",
		conn->isvalid_thermal_zone, hwid->major_version, hwid->minor_version, hwid_info);

	platform_class_buckchg_ops_get_online(MAIN_BUCK_CHARGER, &online);
	conn->anti_wake_lock = wakeup_source_register(conn->dev, "anti_wakelock");
	if (!conn->anti_wake_lock)
		mca_log_err("anti reg wakelock failed\n");

	(void)mca_strategy_ops_register(STRATEGY_FUNC_TYPE_CONNECTOR_ANTIBURN, mca_connector_antiburn_process_event, NULL, NULL, conn);

	conn->thermal_board_nb.notifier_call = connector_antiburn_thermal_notifier_event;
	mca_event_block_notify_register(MCA_EVENT_TYPE_THERMAL_TEMP, &conn->thermal_board_nb);
	conn->triggered = 0;

	INIT_DELAYED_WORK(&conn->monitor_work, connector_antiburn_monitor_workfunc);

	if (online) {
		ret = platform_class_buckchg_ops_adc_enable(MAIN_BUCK_CHARGER, true);
		conn->reset_rate = 1;
		cancel_delayed_work_sync(&conn->monitor_work);
		mca_queue_delayed_work(&conn->monitor_work, msecs_to_jiffies(10000));
		mca_log_info("start schedule anti work\n");
	}

	g_conn = conn;

	mca_log_charge_log_register(MCA_CHARGE_LOG_ID_USCP,
		&g_connector_antiburn_log_ops, conn);
	antiburn_sysfs_create_group(conn->dev);
	mca_log_err("probe OK");
	return 0;
err:
	devm_kfree(&pdev->dev, conn);
	return -EPROBE_DEFER;
}

static const struct of_device_id match_table[] = {
	{.compatible = "xiaomi,connector_antiburn"},
	{},
};

static int connector_antiburn_remove(struct platform_device *pdev)
{
	struct connector_antiburn *conn = platform_get_drvdata(pdev);

	cancel_delayed_work(&conn->monitor_work);

	mca_event_block_notify_unregister(MCA_EVENT_TYPE_THERMAL_TEMP, &conn->thermal_board_nb);
	devm_kfree(&pdev->dev, conn);
	return 0;
}

static void connector_antiburn_shutdown(struct platform_device *pdev)
{

}

static struct platform_driver connector_antiburn_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "connector_antiburn",
		.of_match_table = match_table,
	},
	.probe = connector_antiburn_probe,
	.remove = connector_antiburn_remove,
	.shutdown = connector_antiburn_shutdown,
};

static int __init connector_antiburn_init(void)
{
	return platform_driver_register(&connector_antiburn_driver);
}
module_init(connector_antiburn_init);

static void __exit connector_antiburn_exit(void)
{
	platform_driver_unregister(&connector_antiburn_driver);
}
module_exit(connector_antiburn_exit);

MODULE_DESCRIPTION("Connector antiburn");
MODULE_AUTHOR("muxinyi1@xiaomi.com");
MODULE_LICENSE("GPL v2");
