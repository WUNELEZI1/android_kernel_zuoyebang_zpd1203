// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#define pr_fmt(fmt) "QG-K: %s: " fmt, __func__

#include <linux/debugfs.h>
#include <linux/alarmtimer.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/iio/iio.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>
#include <linux/uaccess.h>
#include <linux/pmic-voter.h>
#include <linux/poll.h>
#include <linux/iio/consumer.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>
#include "gauge-iio.h"
#include "qg-iio.h"
#include "charger_partition.h"
#include "lc_notify.h"
#include "lc_adaptive_poweroff_voltage.h"

static int shutdown_delay_voltage = SHUTDOWN_DELAY_VOLMAX;
static int shutdown_force_voltage = SHUTDOWN_DELAY_VOLMIN;
static int volt_count;
static int detect_done;

#define DEBUG_BATT_TYPE "Debug Board"
static const char *qg_get_battery_type(struct qpnp_qg *chip)
{
	return DEBUG_BATT_TYPE;
}

static ssize_t battery_type_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct qpnp_qg *chip = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%s\n", qg_get_battery_type(chip));
}
static DEVICE_ATTR_RO(battery_type);

static struct attribute *qg_attrs[] = {
	&dev_attr_battery_type.attr,
	NULL,
};
ATTRIBUTE_GROUPS(qg);

static bool is_batt_available(struct qpnp_qg *chip)
{
	if (chip->batt_psy)
		return true;

	chip->batt_psy = power_supply_get_by_name("battery");
	if (!chip->batt_psy)
		return false;

	/* batt_psy is initialized, set the fcc and fv */
	//qg_notify_charger(chip);

	return true;
}

static bool is_gague_available(struct qpnp_qg *chip)
{
	if (IS_ERR_OR_NULL(chip->fg_psy)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			return false;
		}
	}

	return true;
}

bool is_chan_valid(struct qpnp_qg *chip,
		enum qg_ext_iio_channels chan)
{
	int rc;

	if (IS_ERR(chip->ext_iio_chans[chan]))
		return false;

	if (!chip->ext_iio_chans[chan]) {
		chip->ext_iio_chans[chan] = devm_iio_channel_get(chip->dev,
					qg_ext_iio_chan_name[chan]);
		if (IS_ERR(chip->ext_iio_chans[chan])) {
			rc = PTR_ERR(chip->ext_iio_chans[chan]);
			if (rc == -EPROBE_DEFER)
				chip->ext_iio_chans[chan] = NULL;

			pr_err("Failed to get IIO channel %s, rc=%d\n",
				qg_ext_iio_chan_name[chan], rc);
			return false;
		}
	}

	return true;
}

int qg_read_iio_chan(struct qpnp_qg *chip,
	enum qg_ext_iio_channels chan, int *val)
{
	int rc;

	if (is_chan_valid(chip, chan)) {
		rc = iio_read_channel_processed(
				chip->ext_iio_chans[chan], val);
		return (rc < 0) ? rc : 0;
	}

	return -EINVAL;
}

int qg_write_iio_chan(struct qpnp_qg *chip,
	enum qg_ext_iio_channels chan, int val)
{
	if (is_chan_valid(chip, chan))
		return iio_write_channel_raw(chip->ext_iio_chans[chan],
						val);

	return -EINVAL;
}

int qg_read_int_iio_chan(struct iio_channel *iio_chan_list, int chan_id,
			int *val)
{
	int rc;

	do {
		if (iio_chan_list->channel->channel == chan_id) {
			rc = iio_read_channel_processed(iio_chan_list,
							val);
			return (rc < 0) ? rc : 0;
		}
	} while (iio_chan_list++);

	return -ENOENT;
}

#define DEBUG_BATT_ID_LOW 6000
#define DEBUG_BATT_ID_HIGH 8500
static bool is_debug_batt_id(struct qpnp_qg *chip)
{
	return false;
}

static int qg_set_fast_charge_mode(struct qpnp_qg *chip, bool enable)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			enable = false;
			return 0;
		}
	}

	val.intval = enable;

	rc = power_supply_set_property(chip->fg_psy, POWER_SUPPLY_PROP_CURRENT_BOOT,
				       &val);
	if (rc < 0) {
		pr_err("Failed to set fast_charge_mode, rc=%d\n", rc);
		return rc;
	}

	return 0;
}

static int qg_set_battery_capacity(struct qpnp_qg *chip, int soc)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			soc = 50;
			return 0;
		}
	}

	val.intval = soc;

	rc = power_supply_set_property(chip->fg_psy, POWER_SUPPLY_PROP_CAPACITY,
				       &val);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc=%d\n", rc);
		return rc;
	}

	return 0;
}

static int qg_get_battery_capacity(struct qpnp_qg *chip, int *soc)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*soc = 50;
			return 0;
		}
	}

	rc = power_supply_get_property(chip->fg_psy, POWER_SUPPLY_PROP_CAPACITY,
				       &val);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc=%d\n", rc);
		return rc;
	}

	*soc = val.intval;

	return 0;
}

static int qg_get_battery_capacity_real(struct qpnp_qg *chip, int *rsoc)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*rsoc = 50;
			return 0;
		}
	}

	rc = power_supply_get_property(chip->fg_psy,
			POWER_SUPPLY_PROP_CAPACITY_ALERT_MAX, &val);
	if (rc < 0) {
		pr_err("Failed to get raw_soc, rc=%d\n", rc);
		return rc;
	}

	*rsoc = val.intval;

	return 0;
}

static int qg_get_battery_capacity_raw(struct qpnp_qg *chip, int *raw_soc)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*raw_soc = 50;
			return 0;
		}
	}

	rc = power_supply_get_property(chip->fg_psy,
			POWER_SUPPLY_PROP_CAPACITY_ALERT_MIN, &val);
	if (rc < 0) {
		pr_err("Failed to get raw_soc, rc=%d\n", rc);
		return rc;
	}

	*raw_soc = val.intval;

	return 0;
}

int qg_get_battery_voltage(struct qpnp_qg *chip, int *vbat_uv)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*vbat_uv = 3700000;
			return 0;
		}
	}

#if 0
	rc = power_supply_get_property(chip->fg_psy,
				       POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc=%d\n", rc);
		return rc;
	}
#endif
	if (!chip->usb_psy) {
		chip->usb_psy = power_supply_get_by_name("usb");
	}

	if (IS_ERR_OR_NULL(chip->batt_psy)) {
		rc = power_supply_get_property(chip->fg_psy,
				       POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
		if (rc < 0) {
			pr_err("Failed to get capacity, rc=%d\n", rc);
			return rc;
		}
	} else {
		rc = power_supply_get_property(chip->usb_psy,
				       POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN, &val);
		if (rc < 0) {
			pr_err("Failed to get capacity, rc=%d\n", rc);
			return rc;
		} else {
			val.intval = val.intval/100;
		}
	}

	*vbat_uv = val.intval;

	return 0;
}

int qg_get_battery_current(struct qpnp_qg *chip, int *ibat_ua)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*ibat_ua = 0;
			return 0;
		}
	}

	rc = power_supply_get_property(chip->fg_psy,
				       POWER_SUPPLY_PROP_CURRENT_NOW, &val);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc=%d\n", rc);
		return rc;
	}

	*ibat_ua = val.intval;

	return 0;
}

int qg_set_battery_temp(struct qpnp_qg *chip, int temp)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			temp = 250;
			return 0;
		}
	}

	val.intval = temp;

	rc = power_supply_set_property(chip->fg_psy, POWER_SUPPLY_PROP_TEMP,
				       &val);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc=%d\n", rc);
		return rc;
	}

	return 0;
}

int qg_get_battery_temp(struct qpnp_qg *chip, int *temp)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*temp = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(chip->fg_psy, POWER_SUPPLY_PROP_TEMP,
				       &val);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc=%d\n", rc);
		return rc;
	}

	*temp = val.intval;

	return 0;
}

int qg_get_charge_full(struct qpnp_qg *chip, int *temp)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*temp = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(chip->fg_psy,
				       POWER_SUPPLY_PROP_CHARGE_FULL, &val);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc=%d\n", rc);
		return rc;
	}

	*temp = val.intval;

	return 0;
}

int qg_get_charge_full_design(struct qpnp_qg *chip, int *temp)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*temp = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(
		chip->fg_psy, POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, &val);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc=%d\n", rc);
		return rc;
	}

	*temp = val.intval;

	return 0;
}

int qg_set_cycle_count(struct qpnp_qg *chip, int cycle_count)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			cycle_count = 50;
			return 0;
		}
	}

	val.intval = cycle_count;

	rc = power_supply_set_property(chip->fg_psy,
				       POWER_SUPPLY_PROP_CYCLE_COUNT, &val);
	if (rc < 0) {
		pr_err("Failed to set cycle_count, rc=%d\n", rc);
		return rc;
	}

	return 0;
}

int qg_get_cycle_count(struct qpnp_qg *chip, int *cycle_count)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*cycle_count = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(chip->fg_psy,
				       POWER_SUPPLY_PROP_CYCLE_COUNT, &val);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc=%d\n", rc);
		return rc;
	}

	*cycle_count = val.intval;

	return 0;
}

int ttf_get_time_to_full(struct qpnp_qg *chip, int *temp)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*temp = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(
		chip->fg_psy, POWER_SUPPLY_PROP_TIME_TO_FULL_NOW, &val);
	if (rc < 0) {
		pr_err("Failed to get time full, rc=%d\n", rc);
		return rc;
	}

	*temp = val.intval;

	return 0;
}

int ttf_get_time_to_empty(struct qpnp_qg *chip, int *temp)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*temp = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(
		chip->fg_psy, POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG, &val);
	if (rc < 0) {
		pr_err("Failed to get time_to_empty, rc=%d\n", rc);
		return rc;
	}

	*temp = val.intval;

	return 0;
}

static int qg_get_soh(struct qpnp_qg *chip, int *val)
{
	union power_supply_propval pval = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*val = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(
		chip->fg_psy, POWER_SUPPLY_PROP_CHARGE_EMPTY, &pval);
	if (rc < 0) {
		pr_err("Failed to get soh, rc=%d\n", rc);
		return rc;
	}

	*val = pval.intval;

	return 0;
}

static int qg_get_fg1_df_check(struct qpnp_qg *chip, int *val)
{
	union power_supply_propval pval = {0,};
	int rc = 0;
	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*val = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(
		chip->fg_psy, POWER_SUPPLY_PROP_ENERGY_AVG, &pval);

	if (rc < 0) {
		pr_err("Failed to get fg1_df_check, rc=%d\n", rc);
		return rc;
	}

	*val = pval.intval;

	return 0;
}

static int qg_get_fg1_chemid(struct qpnp_qg *chip, int *val)
{
	union power_supply_propval pval = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*val = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(
		chip->fg_psy, POWER_SUPPLY_PROP_ENERGY_NOW, &pval);
	if (rc < 0) {
		pr_err("Failed to get fg1_chemid, rc=%d\n", rc);
		return rc;
	}

	*val = pval.intval;

	return 0;
}

static int qg_get_pack_vendor(struct qpnp_qg *chip, int *val)
{
	union power_supply_propval pval = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*val = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(
		chip->fg_psy, POWER_SUPPLY_PROP_ENERGY_EMPTY, &pval);
	if (rc < 0) {
		pr_err("Failed to get pack_vendor, rc=%d\n", rc);
		return rc;
	}

	*val = pval.intval;

	return 0;
}

static int qg_get_soc_decimal_rate(struct qpnp_qg *chip, int *val)
{
	union power_supply_propval pval = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*val = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(
		chip->fg_psy, POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN, &pval);
	if (rc < 0) {
		pr_err("Failed to get soh, rc=%d\n", rc);
		return rc;
	}

	*val = pval.intval;

	return 0;
}

static int qg_get_soc_decimal(struct qpnp_qg *chip, int *val)
{
	union power_supply_propval pval = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*val = 250;
			return 0;
		}
	}

	rc = power_supply_get_property(
		chip->fg_psy, POWER_SUPPLY_PROP_ENERGY_FULL, &pval);
	if (rc < 0) {
		pr_err("Failed to get soh, rc=%d\n", rc);
		return rc;
	}

	*val = pval.intval;

	return 0;
}

static int calculate_delta_time(ktime_t *time_stamp, int *delta_time_s)
{
	ktime_t now_time;

	now_time = ktime_get_seconds();
	*delta_time_s = now_time - *time_stamp;
	if(*delta_time_s < 0)
		*delta_time_s = 0;

	return 0;
}

#define LOW_TBAT_THRESHOLD		    150
#define CHANGE_SOC_TIME_LIMIT_10S	10
#define CHANGE_SOC_TIME_LIMIT_20S	20
#define CHANGE_SOC_TIME_LIMIT_60S	60
#define HEAVY_DISCHARGE_CURRENT		1000
#define FORCE_TO_FULL_SOC		    95
#define MIN_DISCHARGE_CURRENT		(-25)
#define MIN_CHARGING_CURRENT		25
static void battery_soc_smooth_tracking_new(struct qpnp_qg *chip)
{
	static int system_soc, last_system_soc, raw_soc;
	int soc_changed = 0, unit_time = 20, delta_time = 0, soc_delta = 0;
	//static struct timespec last_change_time;
	static ktime_t last_change_time;

	static int firstcheck = 0;
	int change_delta = 0, rc = 0;
	union power_supply_propval prop = {0, };
	int charging_status = 0, charge_type = 0;

	rc = qg_read_iio_chan(chip, USB_REAL_TYPE, &prop.intval);
	if (rc < 0)
		pr_err("Failed to get real type , rc=%d\n", rc);
	else
		chip->charge_type = prop.intval;

	if (!chip->batt_psy)
		chip->batt_psy = power_supply_get_by_name("battery");
	if (chip->batt_psy) {
		rc = power_supply_get_property(chip->batt_psy,
						POWER_SUPPLY_PROP_STATUS, &prop);
		if (rc < 0)
			pr_err("Failed to get charger status, rc=%d\n", rc);
		else
			charging_status = prop.intval;
	}
	raw_soc = chip->param.batt_raw_soc *100;

	/*Map system_soc value according to raw_soc */
	if (raw_soc >= 9700)
		system_soc = 100;
	else {
		system_soc = ((raw_soc + 96) / 97);
		if (system_soc > 99)
			system_soc = 99;
	}
	pr_debug("smooth_tracking_new:charging_status:%d raw_soc:%d system_soc:%d\n",
		charging_status, raw_soc, system_soc);
	/*Get the initial value for the first time */
	if (!firstcheck) {
		last_change_time = ktime_get_seconds();
		last_system_soc = system_soc;
		firstcheck = 1;
	}
	if ((charging_status == POWER_SUPPLY_STATUS_DISCHARGING || charging_status == POWER_SUPPLY_STATUS_NOT_CHARGING)
		&& !chip->param.batt_rmc && chip->param.batt_temp < LOW_TBAT_THRESHOLD && last_system_soc > 1) {
		unit_time = 50;
	}

	pr_debug("ktime:%d ktime_boot:%d, last_change_time=%d\n",
			ktime_get_seconds(), ktime_get_boottime_seconds(), last_change_time);
	/*If the soc jump, will smooth one cap every 10S */
	qg_get_battery_voltage(chip, &rc);
	chip->param.batt_volt = rc;
	soc_delta = abs(system_soc - last_system_soc);
	if (soc_delta > 1 || (chip->param.batt_volt < 3300 && system_soc > 0)) {
		calculate_delta_time(&last_change_time, &change_delta);
		delta_time = change_delta / unit_time;
		pr_debug("change_delta:%d unit_time:%d\n", change_delta, unit_time);
		if (delta_time < 0) {
			last_change_time = ktime_get_seconds();
			delta_time = 0;
		}
		soc_changed = min(1, delta_time);
		rc = qg_get_battery_current(chip, &chip->param.batt_curr);
		if (soc_changed) {
			if ((chip->param.batt_curr > 0 || charging_status == POWER_SUPPLY_STATUS_CHARGING) && (system_soc > last_system_soc))
				system_soc = last_system_soc + soc_changed;
			else if ((chip->param.batt_curr< 0 || charging_status == POWER_SUPPLY_STATUS_DISCHARGING ||
				(charging_status == POWER_SUPPLY_STATUS_CHARGING && charge_type == POWER_SUPPLY_TYPE_USB))
				&& (system_soc < last_system_soc))
				system_soc = last_system_soc - soc_changed;
			else
				system_soc = last_system_soc;
			pr_debug("soc_changed:%d charging_status:%d\n", soc_changed, charging_status);
		} else
			system_soc = last_system_soc;
	}

	/*Avoid mismatches between charging status and soc changes  */
	if(charging_status == POWER_SUPPLY_STATUS_DISCHARGING && (system_soc > last_system_soc))
		system_soc = last_system_soc;

	if ((ktime_get_seconds() - last_change_time < unit_time) && soc_delta == 1) {
		system_soc = last_system_soc;
	}

	pr_debug("smooth_new:sys_soc:%d last_sys_soc:%d soc_delta:%d" ,
		system_soc, last_system_soc, soc_delta);

	if (system_soc != last_system_soc) {
		last_change_time = ktime_get_seconds();
		last_system_soc = system_soc;
	}
	chip->param.batt_soc = system_soc;
}

static int qg_get_charge_counter(struct qpnp_qg *chip, int *charge_counter)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			*charge_counter = 50;
			return 0;
		}
	}

	rc = power_supply_get_property(
		chip->fg_psy, POWER_SUPPLY_PROP_CHARGE_COUNTER, &val);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc=%d\n", rc);
		return rc;
	}

	*charge_counter = val.intval;

	return 0;
}

static void shutdown_delayed_workfunc(struct work_struct *work)
{
	int rc, vbat_mv, ibat_ma;
	union power_supply_propval pval;
	struct power_supply *fg_psy = NULL;
	struct qpnp_qg *chip = container_of(work, struct qpnp_qg, shutdown_delay_work.work);

	fg_psy = power_supply_get_by_name("bq28z610");
	if (fg_psy) {
		rc = power_supply_get_property(fg_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &pval);
		vbat_mv = pval.intval / 1000;
		rc |= power_supply_get_property(fg_psy, POWER_SUPPLY_PROP_CURRENT_NOW, &pval);
		ibat_ma = pval.intval / 1000;
		pr_info("vbat:%dmV, ibat:%dmA\n", vbat_mv, ibat_ma);
		if (!rc) {
			if (vbat_mv <= shutdown_delay_voltage) {
				chip->vbat_low_cnt++;
				if (chip->vbat_low_cnt >= 4) {
					chip->vbat_low_cnt = 0;
					chip->shutdown_delay = true;
					pr_info("vbat force shutdown_delay true\n");
				} else
					schedule_delayed_work(&chip->shutdown_delay_work, msecs_to_jiffies(1000));
			} else {
				chip->vbat_low_cnt = 0;
				pr_info("reset vbat_low_cnt\n");
			}
			if (ibat_ma < SHUTDOWN_DELAY_CURMAX) {
				chip->ibat_ocp_cnt++;
				if (chip->ibat_ocp_cnt >= 4) {
					chip->ibat_ocp_cnt = 0;
					chip->shutdown_delay = true;
					pr_info("ibat force shutdown_delay true\n");
				} else
					schedule_delayed_work(&chip->shutdown_delay_work, msecs_to_jiffies(1000));
			} else {
				chip->ibat_ocp_cnt = 0;
				pr_info("reset ibat_ocp_cnt\n");
			}
		} else {
			chip->vbat_low_cnt = 0;
			chip->ibat_ocp_cnt = 0;
			pr_info("reset vbat_low_cnt and ibat_ocp_cnt\n");
		}
	} else {
		chip->vbat_low_cnt = 0;
		chip->ibat_ocp_cnt = 0;
		pr_info("clear vbat_low_cnt and ibat_ocp_cnt\n");
	}
}

static int lc_charger_chain_notify(struct notifier_block *notifier, unsigned long event, void *data)
{
	struct poweroff_voltage_config *poweroff_vol_conf;
	if (event == CHARGER_EVENT_SHUTDOWN_VOLTAGE_CHANGED) {
		poweroff_vol_conf = (struct poweroff_voltage_config *)data;
		if (IS_ERR_OR_NULL(poweroff_vol_conf))
			return NOTIFY_OK;
		shutdown_force_voltage = poweroff_vol_conf->poweroff_voltage;
		shutdown_delay_voltage = poweroff_vol_conf->shutdown_delay_voltage;
		pr_info("get evt:%lu reset shutdown_force_voltage:%d shutdown_delay_voltage:%d\n",
			event, shutdown_force_voltage, shutdown_delay_voltage);
	}
	return NOTIFY_OK;
}

static struct notifier_block lc_charger_chain_nb = {
    .notifier_call = lc_charger_chain_notify,
};

int qg_get_prop_shutdown_delay(struct qpnp_qg *chip, int *capacity)
{
	union power_supply_propval psp = {0,};
	int vbat_mv = 0, ibat_ma = 0;
	int rc;
	static bool last_shutdown_delay;

	if (chip->shutdown_delay_enable){
		if (*capacity <= 1){
			rc = qg_get_battery_voltage(chip, &psp.intval);
			if (rc < 0)
				pr_err("Couldn't read batt voltage rc=%d\n",rc);
			else
				vbat_mv = psp.intval / 1000;
			rc = qg_get_battery_current(chip, &psp.intval);
			if (rc < 0)
				pr_err("Couldn't read batt current rc=%d\n",rc);
			else
				ibat_ma = psp.intval / 1000;
			if ((chip->charge_status == POWER_SUPPLY_STATUS_CHARGING) && chip->shutdown_delay) {
				chip->shutdown_delay = false;
				*capacity = 1;
			} else {
				if (vbat_mv > shutdown_delay_voltage)
					*capacity = 1;
				if (vbat_mv < shutdown_delay_voltage || ibat_ma < SHUTDOWN_DELAY_CURMAX) {
					if (!chip->shutdown_delay && (chip->charge_status == POWER_SUPPLY_STATUS_DISCHARGING || chip->charge_status == POWER_SUPPLY_STATUS_NOT_CHARGING)) {
						if (!delayed_work_pending(&chip->shutdown_delay_work)) {
							pr_info("start shutdown delayed work\n");
							schedule_delayed_work(&chip->shutdown_delay_work, msecs_to_jiffies(1000));
						}
					} else {
						chip->vbat_low_cnt = 0;
						chip->ibat_ocp_cnt = 0;
						cancel_delayed_work(&chip->shutdown_delay_work);
						pr_info("cancel work,shutdown_delay:%d,charge_status:%d\n", chip->shutdown_delay, chip->charge_status);
					}
					*capacity = 1;
				} else {
					chip->shutdown_delay = false;
					*capacity = 0;
					chip->vbat_low_cnt = 0;
					chip->ibat_ocp_cnt = 0;
					cancel_delayed_work(&chip->shutdown_delay_work);
					pr_info("cancel work,vbat_mv:%d,ibat_ma:%d\n", vbat_mv, ibat_ma);
				}
			}
			if (chip->shutdown_delay)
				pr_info("shutdown_delay true\n");
			else
				pr_info("shutdown_delay false\n");
		} else {
			chip->shutdown_delay = false;
			chip->vbat_low_cnt = 0;
			chip->ibat_ocp_cnt = 0;
			cancel_delayed_work(&chip->shutdown_delay_work);
			pr_info("cancel work,capacity:%d\n", *capacity);
		}
	} else {
		chip->shutdown_delay = false;
		chip->vbat_low_cnt = 0;
		chip->ibat_ocp_cnt = 0;
		cancel_delayed_work(&chip->shutdown_delay_work);
		pr_info("cancel work,shutdown_delay_enable:%d\n", chip->shutdown_delay_enable);
	}

	if (*capacity == 0)
		*capacity = 1;

	if (last_shutdown_delay != chip->shutdown_delay) {
		last_shutdown_delay = chip->shutdown_delay;
		power_supply_changed(chip->qg_psy);
	}

	if (chip->shutdown_delay)
		pr_info("shutdown_delay in QG is %d \n",chip->shutdown_delay);

	return 0;
}

static int qg_get_batt_id(void)
{
	int batt_id = 0;
	struct power_supply *psy_fg = NULL;
	union power_supply_propval pval = { 0 };

	psy_fg = power_supply_get_by_name("bq28z610");
	if (!IS_ERR_OR_NULL(psy_fg)) {
		if (!power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_CALIBRATE, &pval))
			batt_id = pval.intval;
		else
			pr_err("%s: get fg psy authentic prop failed\n", __func__);
	} else
		pr_err("%s: get fg psy failed\n", __func__);

	return batt_id;
}

#define SIN_COS_FFC_LOW_TEMP_CHG_TERM_CURRENT	-950
#define COS_LOW_TEMP_FFC_BATT_FULL_CURRENT		1350000
#define SIN_ATL_FFC_LOW_TEMP_CHG_TERM_CURRENT	-1150
#define ATL_LOW_TEMP_FFC_BATT_FULL_CURRENT		1450000
#define DBL_SWD_FFC_LOW_TEMP_CHG_TERM_CURRENT	-1350
#define DBL_ATL_FFC_LOW_TEMP_CHG_TERM_CURRENT	-1500
#define SIN_COS_FFC_HIGH_TEMP_CHG_TERM_CURRENT	-1350
#define COS_FFC_HIGH_TEMP_CHG_TERM_CURRENT		1550000
#define SIN_ATL_FFC_HIGH_TEMP_CHG_TERM_CURRENT	-1450
#define ATL_FFC_HIGH_TEMP_CHG_TERM_CURRENT		1650000
#define DBL_SWD_FFC_HIGH_TEMP_CHG_TERM_CURRENT	-1700
#define DBL_ATL_FFC_HIGH_TEMP_CHG_TERM_CURRENT	-2050
static int qg_get_ffc_iterm_for_chg(struct qpnp_qg *chip)
{
	int rc = 0, batt_temp = 0, ffc_chg_iterm = 0;
	int batt_id = 0;

	batt_id = qg_get_batt_id();
	rc = qg_get_battery_temp(chip, &batt_temp);
	if (rc < 0){
		pr_err("Failed to get battery-temp, rc = %d\n", rc);
		return rc;
	}
	pr_err("%s: batt_id:%d \n", __func__, batt_id);
	if (batt_temp < 350) {
		if (batt_id == 1) {
			ffc_chg_iterm = SIN_COS_FFC_LOW_TEMP_CHG_TERM_CURRENT;
		} else if (batt_id == 2) {
			ffc_chg_iterm = SIN_ATL_FFC_LOW_TEMP_CHG_TERM_CURRENT;
		} else if (batt_id == 3) {
			ffc_chg_iterm = DBL_SWD_FFC_LOW_TEMP_CHG_TERM_CURRENT;
		} else if (batt_id == 4) {
			ffc_chg_iterm = DBL_ATL_FFC_LOW_TEMP_CHG_TERM_CURRENT;
		}
	} else {
		if (batt_id == 1) {
			ffc_chg_iterm = SIN_COS_FFC_HIGH_TEMP_CHG_TERM_CURRENT;
		} else if (batt_id == 2) {
			ffc_chg_iterm = SIN_ATL_FFC_HIGH_TEMP_CHG_TERM_CURRENT;
		} else if (batt_id == 3) {
			ffc_chg_iterm = DBL_SWD_FFC_HIGH_TEMP_CHG_TERM_CURRENT;
		} else if (batt_id == 4) {
			ffc_chg_iterm = DBL_ATL_FFC_HIGH_TEMP_CHG_TERM_CURRENT;
		}
	}

	return ffc_chg_iterm;
}

static int qg_iio_write_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan, int val1,
			    int val2, long mask)
{
	struct qpnp_qg *chip = iio_priv(indio_dev);
	int rc = 0;

	switch (chan->channel) {
	case PSY_IIO_CAPACITY:
		rc = qg_set_battery_capacity(chip, val1);
		break;
	case PSY_IIO_CHARGE_FULL:
		break;
	case PSY_IIO_CHARGE_FULL_DESIGN:
		break;
	case PSY_IIO_BATT_FULL_CURRENT:
		chip->keep_ffc_iterm = val1;
		break;
	case PSY_IIO_FASTCHARGE_MODE:
		chip->fastcharge_mode_enabled = val1;
		rc = qg_set_fast_charge_mode(chip, chip->fastcharge_mode_enabled);
		break;
	case PSY_IIO_TEMP:
		rc = qg_set_battery_temp(chip, val1);
		break;
	case PSY_IIO_CYCLE_COUNT:
		rc = qg_set_cycle_count(chip, val1);
		break;
	case PSY_IIO_CLEAR_SOH:
		break;
	default:
		pr_debug("Unsupported QG IIO chan %d\n", chan->channel);
		rc = -EINVAL;
		break;
	}

	if (rc < 0)
		pr_err_ratelimited("Couldn't write IIO channel %d, rc = %d\n",
				   chan->channel, rc);

	return rc;
}

static int qg_iio_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan, int *val1,
			   int *val2, long mask)
{
	struct qpnp_qg *chip = iio_priv(indio_dev);
	//int64_t temp = 0;
	int rc = 0;

	*val1 = 0;

	switch (chan->channel) {
	case PSY_IIO_CAPACITY:
		rc = qg_get_battery_capacity(chip, val1);
		//if (chip->param.batt_soc >= 0)
		//	*val1 = chip->param.batt_soc;
		if (*val1 <= 1)
			qg_get_prop_shutdown_delay(chip, val1);
		break;
	case PSY_IIO_CAPACITY_RAW:
		rc = qg_get_battery_capacity_raw(chip, val1);
		chip->sys_soc = *val1;
		break;
	case PSY_IIO_REAL_CAPACITY:
		rc = qg_get_battery_capacity_real(chip, val1);
		break;
	case PSY_IIO_VOLTAGE_NOW:
		rc = qg_get_battery_voltage(chip, val1);
		break;
	case PSY_IIO_CURRENT_NOW:
		rc = qg_get_battery_current(chip, val1);
		break;
	case PSY_IIO_TEMP:
		rc = qg_get_battery_temp(chip, val1);
		break;
	case PSY_IIO_RESISTANCE_ID:
		*val1 = 100000;
		break;
	case PSY_IIO_VOLTAGE_MAX:
		//*val1 = chip->bp.float_volt_uv;
		break;
	case PSY_IIO_BATT_FULL_CURRENT:
		/*if (chip->fastcharge_mode_enabled)
				*val1 = qg_get_ffc_iterm_for_qg(chip);
			else
				*val1 = chip->dt.iterm_ma * 1000;
			*/
		break;
	case PSY_IIO_CHARGE_COUNTER:
		rc = qg_get_charge_counter(chip, val1);
		break;
	case PSY_IIO_CHARGE_FULL:
		rc = qg_get_charge_full(chip, val1);
		break;
	case PSY_IIO_CHARGE_FULL_DESIGN:
		rc = qg_get_charge_full_design(chip, val1);
		break;
	case PSY_IIO_CYCLE_COUNT:
		rc = qg_get_cycle_count(chip, val1);
		break;
	case PSY_IIO_TIME_TO_FULL_NOW:
		rc = ttf_get_time_to_full(chip, val1);
		break;
	case PSY_IIO_TIME_TO_EMPTY_AVG:
		rc = ttf_get_time_to_empty(chip, val1);
		break;
	case PSY_IIO_VOLTAGE_AVG:
		//rc = qg_get_vbat_avg(chip, val1);
		break;
	case PSY_IIO_CURRENT_AVG:
		//rc = qg_get_ibat_avg(chip, val1);
		break;
	case PSY_IIO_POWER_NOW:
		//rc = qg_get_power(chip, val1, false);
		break;
	case PSY_IIO_POWER_AVG:
		//rc = qg_get_power(chip, val1, true);
		break;
	case PSY_IIO_DEBUG_BATTERY:
		*val1 = is_debug_batt_id(chip);
		break;
	case PSY_IIO_FASTCHARGE_MODE:
		*val1 = chip->fastcharge_mode_enabled;
		break;
	case PSY_IIO_SOC_REPORTING_READY:
		*val1 = chip->soc_reporting_ready;
		break;
	case PSY_IIO_SOC_DECIMAL:
		rc = qg_get_soc_decimal(chip, val1);
		break;
	case PSY_IIO_SOC_DECIMAL_RATE:
		rc = qg_get_soc_decimal_rate(chip, val1);
		break;
	case PSY_IIO_SHUTDOWN_DELAY:
		*val1 = chip->shutdown_delay;
		break;
	case PSY_IIO_BATT_AGE_LEVEL:
	case PSY_IIO_ESR_ACTUAL:
	case PSY_IIO_ESR_NOMINAL:
		*val1 = 0;
		break;
	case PSY_IIO_SOH:
		rc = qg_get_soh(chip, val1);
		break;
	case PSY_IIO_CLEAR_SOH:
		break;
	case PSY_IIO_FFC_CHG_TERMINATION_CURRENT:
		*val1 = qg_get_ffc_iterm_for_chg(chip);
		break;
	case PSY_IIO_FG1_DF_CHECK:
		rc = qg_get_fg1_df_check(chip, val1);
		break;
	case PSY_IIO_FG1_CHEMID:
		rc = qg_get_fg1_chemid(chip, val1);
		break;
	case PSY_IIO_PACK_VENDOR:
		rc = qg_get_pack_vendor(chip, val1);
		break;
	default:
		pr_err("Unsupported QG IIO chan %d\n", chan->channel);
		rc = -EINVAL;
		break;
	}

	if (rc < 0) {
		pr_err_ratelimited("Couldn't read IIO channel %d, rc = %d\n",
				   chan->channel, rc);
		return rc;
	}

	return IIO_VAL_INT;
}

static int qg_iio_fwnode_xlate(struct iio_dev *indio_dev,
			       const struct fwnode_reference_args *iiospec)
{
	struct qpnp_qg *chip = iio_priv(indio_dev);
	struct iio_chan_spec *iio_chan = chip->iio_chan;
	int i;

	for (i = 0; i < ARRAY_SIZE(qg_iio_psy_channels); i++, iio_chan++)
		if (iio_chan->channel == iiospec->args[0])
			return i;

	return -EINVAL;
}

static const struct iio_info qg_iio_info = {
	.read_raw = qg_iio_read_raw,
	.write_raw = qg_iio_write_raw,
	.fwnode_xlate = qg_iio_fwnode_xlate,
};

static int qg_parse_dt(struct qpnp_qg *chip)
{
	struct device_node *node = chip->dev->of_node;

	if (!node)  {
		pr_err("Failed to find device-tree node\n");
		return -ENXIO;
	}


	if (of_property_read_bool(node, "qcom,shutdown-delay-enable"))
		chip->shutdown_delay_enable = true;
	else
		chip->shutdown_delay_enable = false;

	return 0;
}

static ssize_t qg_device_read(struct file *file, char __user *buf, size_t count,
			  loff_t *ppos)
{
	return 0;
}

static ssize_t qg_device_write(struct file *file, const char __user *buf,
			size_t count, loff_t *ppos)
{
	return 0;
}

static unsigned int qg_device_poll(struct file *file, poll_table *wait)
{
	return 0;
}

static int qg_device_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int qg_device_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations qg_fops = {
	.owner		= THIS_MODULE,
	.open		= qg_device_open,
	.release	= qg_device_release,
	.read		= qg_device_read,
	.write		= qg_device_write,
	.poll		= qg_device_poll,
};

static int qg_register_device(struct qpnp_qg *chip)
{
	int rc;

	rc = alloc_chrdev_region(&chip->dev_no, 0, 1, "qg");
	if (rc < 0) {
		pr_err("Failed to allocate chardev rc=%d\n", rc);
		return rc;
	}

	cdev_init(&chip->qg_cdev, &qg_fops);
	rc = cdev_add(&chip->qg_cdev, chip->dev_no, 1);
	if (rc < 0) {
		pr_err("Failed to cdev_add rc=%d\n", rc);
		goto unregister_chrdev;
	}

	chip->qg_class = class_create(THIS_MODULE, "qg");
	if (IS_ERR_OR_NULL(chip->qg_class)) {
		pr_err("Failed to create qg class\n");
		rc = -EINVAL;
		goto delete_cdev;
	}
	chip->qg_device = device_create(chip->qg_class, NULL, chip->dev_no, NULL, "qg");
	if (IS_ERR(chip->qg_device)) {
		pr_err("Failed to create qg_device\n");
		rc = -EINVAL;
		goto destroy_class;
	}

	return 0;

destroy_class:
	class_destroy(chip->qg_class);
delete_cdev:
	cdev_del(&chip->qg_cdev);
unregister_chrdev:
	unregister_chrdev_region(chip->dev_no, 1);
	return rc;
}

#define MONITOR_SOC_WAIT_MS		    1000
#define MONITOR_SOC_WAIT_PER_MS		8000
static void soc_monitor_work(struct work_struct *work)
{
	int rc, batt_ua;
	struct qpnp_qg *chip = container_of(work,
				struct qpnp_qg,
				soc_monitor_work.work);

	pr_info("%s: monitor_work stopped to use due to fg\n", __func__);
	return ;

	/* Update battery information */
	rc = qg_get_battery_current(chip, &batt_ua);
	if (rc < 0)
		pr_err("failed to get battery current, rc = %d\n", rc);
	chip->param.batt_ma = batt_ua / 1000;

	rc = qg_get_battery_temp(chip, &chip->param.batt_temp);
	if (rc < 0)
		pr_err("failed to get battery temperature, rc = %d\n", rc);

	rc = qg_get_battery_capacity(chip, &chip->param.batt_raw_soc);
	if (rc < 0)
		pr_err("failed to get battery capacity, rc = %d\n", rc);

	if (chip->soc_reporting_ready)
		battery_soc_smooth_tracking_new(chip);

	pr_debug("soc:%d, raw_soc:%d, c:%d, s:%d\n",
			chip->param.batt_soc, chip->param.batt_raw_soc,
			chip->param.batt_ma, chip->charge_status);

	schedule_delayed_work(&chip->soc_monitor_work, msecs_to_jiffies(MONITOR_SOC_WAIT_PER_MS));

	return;

}

static int get_fv_eea(int *volt)
{
	struct votable *fv_votable;

	fv_votable = find_votable("FV");
	if (!fv_votable) {
		pr_err("%s failed to get fv_votable\n", __func__);
		return -EINVAL;
	}

	*volt = get_effective_result(fv_votable);

	return 0;

}

static int set_charge_status_eea(struct qpnp_qg *chip, int status)
{
	union power_supply_propval val = {0,};
	int rc = 0;

	if(IS_ERR_OR_NULL(chip->batt_psy)){
		chip->batt_psy = power_supply_get_by_name("battery");
		if(IS_ERR_OR_NULL(chip->batt_psy)){
			pr_err("%s: get batt psy failed \n", __func__);
			return -ENODEV;
		}
	}

	val.intval = status;

	rc = power_supply_set_property(chip->batt_psy, POWER_SUPPLY_PROP_STATUS, &val);
	if (rc < 0) {
		pr_err("%s, Failed to set charge_status, rc=%d\n", __func__, rc);
		return rc;
	}

	return 0;
}

#define	EEA_CHARGE_FULL "EEA_CHARGE_FULL"
static int input_current_limit_eea(struct qpnp_qg *chip, bool en)
{

	if(en){
		vote(chip->icl_votable, EEA_CHARGE_FULL, true, 1000000);
		vote(chip->fcc_main_votable, FCC_STEPPER_VOTER, false, 0);
		vote(chip->fcc_main_votable, EEA_CHARGE_FULL, true, 0);
		vote(chip->fcc_votable, EEA_CHARGE_FULL, true, 0);
	}else{
		vote(chip->icl_votable, EEA_CHARGE_FULL, false, 1000000);
		vote(chip->fcc_main_votable, EEA_CHARGE_FULL, false, 0);
		vote(chip->fcc_votable, EEA_CHARGE_FULL, false, 0);
	}

	return 0;
}

static int charge_enable_eea(struct qpnp_qg *chip, bool enable)
{
	bool chg_dis = false;

	chg_dis = get_effective_result(chip->chg_dis_votable);
	if(enable){
		vote(chip->chg_dis_votable, EEA_CHARGE_FULL, true, 0);
		msleep(50);
		vote(chip->chg_dis_votable, EEA_CHARGE_FULL, false, 0);
	}else{
		vote(chip->chg_dis_votable, EEA_CHARGE_FULL, true, 0);
	}

	return 0;
}

#define EEA_RECHARGE_SOC		95
#define RECHARGE_COUNT          5
#define CHARGE_FULL_COUNT       3
#define POWER_SUPPLY_STATUS_INVAILD -1
static int last_batt_full = 0;
static int handle_recharge_eea(struct qpnp_qg *chip)
{
	struct battery_info *bat = &(chip->bat);
	union power_supply_propval pval = {0, };
	int rc = 0;

	rc = qg_get_battery_temp(chip, &bat->temp);
	rc |= qg_get_battery_capacity(chip, &bat->uisoc);
	rc |= qg_get_battery_voltage(chip, &bat->volt);
	rc |= qg_get_battery_current(chip, &bat->curr);
	if (rc < 0){
		pr_err("failed to get battery info, rc = %d\n", rc);
		return rc;
	}

	rc = get_fv_eea(&bat->fv);
	if (rc < 0){
		pr_err("failed to get fv, rc = %d\n", rc);
		return rc;
	}

	pval.intval = bat->fv;
	rc = power_supply_set_property(chip->batt_psy,	POWER_SUPPLY_PROP_VOLTAGE_MAX, &pval);
	if (rc < 0) {
		pr_err("Failed to set voltage_max property on batt_psy, rc=%d\n",rc);
		return rc;
	}

	rc = power_supply_get_property(chip->batt_psy, POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT, &pval);
	if (rc < 0){
		pr_err("Couldn't get chg_term_current, rc=%d\n", rc);
		return rc;
	}
	bat->iterm = -pval.intval;
	bat->ffc = chip->fastcharge_mode_enabled;
	bat->bbc_charge_done = chip->charge_done;


	if(chip->charge_status == POWER_SUPPLY_STATUS_DISCHARGING || chip->charge_type == POWER_SUPPLY_TYPE_UNKNOWN){
		bat->charge_full = 0;
		input_current_limit_eea(chip, false);
		charge_enable_eea(chip, true);
		if (chip->fake_full_status_flag) {
			chip->fake_full_status_flag = false;
			set_charge_status_eea(chip, POWER_SUPPLY_STATUS_INVAILD);
			pr_info("%s: adapter pluged out, reset charge_full flag \n", __func__);
		}
		return rc;
	}

	/* Fix not recharge in case soc < 99 while charge done by high temp */
	if(chip->charge_status == POWER_SUPPLY_STATUS_FULL) {
		pr_err("%s: charge status is set full by jeita.\n", __func__);
		chip->fake_full_status_flag = true;
		bat->charge_full = 1;
	}

	if (bat->charge_full) {
		bat->full_cnt = 0;
		if (bat->uisoc < EEA_RECHARGE_SOC && (-100 <= bat->temp && bat->temp <= 450)) {//not in charging pause with jeita
			bat->recharge_cnt++;
		} else {
			bat->recharge_cnt = 0;
			if(bat->uisoc >= EEA_RECHARGE_SOC && chip->prp_is_enable == false){
				pr_err("%s: keep full and limit ibus between full and recharge unless replenish happend \n", __func__);
				chip->fake_full_status_flag = true;
				set_charge_status_eea(chip, POWER_SUPPLY_STATUS_FULL);
				input_current_limit_eea(chip, true);
			}
		}

		if ((bat->recharge_cnt > RECHARGE_COUNT)) {
			bat->recharge_cnt = 0;
			bat->charge_full = false;
			input_current_limit_eea(chip, false);
			charge_enable_eea(chip, true);
			if (chip->fake_full_status_flag) {
				chip->fake_full_status_flag = false;
				set_charge_status_eea(chip, POWER_SUPPLY_STATUS_INVAILD);
			}
		}

	}else{
		bat->recharge_cnt = 0;

		if((bat->uisoc >= 99 && bat->bbc_charge_done) || bat->uisoc >= 100)
			bat->full_cnt++;
		else{
			bat->full_cnt = 0;
			if (chip->fake_full_status_flag) {
				chip->fake_full_status_flag = false;
				set_charge_status_eea(chip, POWER_SUPPLY_STATUS_INVAILD);
			}
		}

		if (bat->full_cnt >= CHARGE_FULL_COUNT) {
			bat->full_cnt = 0;
			bat->charge_full = true;
			bat->recharge = false;
			if(bat->uisoc >= 100)
				charge_enable_eea(chip, false);
		}
	}


	if(last_batt_full != bat->charge_full){
		pr_info("%s:full=%d, full_cnt=%d, recharge_cnt=%d\n",
				__func__, bat->charge_full, bat->full_cnt, bat->recharge_cnt);
		last_batt_full = bat->charge_full;
	}


	return rc;

}

#define CHARGE_FULL_CURR_BUFFER  	50000
#define CHARGE_FULL_VOLT_BUFFER  	20000
#define XM81_SWD		3
static bool is_replenish_done(struct qpnp_qg *chip)
{
	int iterm;
	int batt_volt, batt_curr;
	static int full_cnt;
	int rc = 0;
	//union power_supply_propval val;

	iterm = chip->batt_id == XM81_SWD ? 550000 : 480000;   //uA
	rc = qg_get_battery_voltage(chip, &batt_volt);
	rc |= qg_get_battery_current(chip, &batt_curr);
	if (rc < 0){
		pr_err("failed to get battery info, rc = %d\n", rc);
		return rc;
	}

	if((batt_volt >= chip->rp_fv + CHARGE_FULL_VOLT_BUFFER) && (batt_curr <= iterm + CHARGE_FULL_CURR_BUFFER)){
		full_cnt ++;
	}else{
		full_cnt = 0;
	}
	pr_info("%s: eff_fv:%d ~ %d, iterm:%d ~ %d, batt_volt:%d, batt_curr:%d, full_cnt:%d \n", __func__,
		chip->rp_fv, CHARGE_FULL_VOLT_BUFFER, iterm, CHARGE_FULL_CURR_BUFFER, batt_volt, batt_curr, full_cnt);
	if(full_cnt > CHARGE_FULL_COUNT){
		return true;
	}

	return false;
}

#define POWER_REPLENISH_ICL     1000000
static int set_fv_fcc_cofig_for_replenish(struct qpnp_qg *chip)
{
	int batt_temp, cycle_count = chip->cycle_count;
	int rc = 0;
	int replenish_fcc = 0;

	rc = qg_get_battery_temp(chip, &batt_temp);
	if (rc < 0){
		pr_err("failed to get battery info, rc = %d\n", rc);
		return rc;
	}

	chip->batt_id = qg_get_batt_id();
	replenish_fcc = chip->batt_id == XM81_SWD ? 570000 : 500000;   //uA

	switch (batt_temp)
	{
	case 201 ... 350:
		if(is_between(1, 100, cycle_count)){
			chip->rp_fv = 4490000;
		}else if(is_between(101, 300, cycle_count)){
			chip->rp_fv = 4480000;
		}else if(is_between(301, 800, cycle_count)){
			chip->rp_fv = 4470000;
		}else if(is_between(801, 2000, cycle_count)){
			chip->rp_fv = 4450000;
		}
		break;
	case 351 ... 400:
		if(is_between(1, 100, cycle_count)){
			chip->rp_fv = 4480000;
		}else if(is_between(101, 300, cycle_count)){
			chip->rp_fv = 4470000;
		}else if(is_between(301, 800, cycle_count)){
			chip->rp_fv = 4460000;
		}else if(is_between(801, 2000, cycle_count)){
			chip->rp_fv = 4440000;
		}
		break;
	case 401 ... 450:
		if(is_between(1, 100, cycle_count)){
			chip->rp_fv = 4470000;
		}else if(is_between(101, 300, cycle_count)){
			chip->rp_fv = 4460000;
		}else if(is_between(301, 800, cycle_count)){
			chip->rp_fv = 4450000;
		}else if(is_between(801, 2000, cycle_count)){
			chip->rp_fv = 4430000;
		}
		break;
	default:
		pr_err("%s: not support power replenish \n", __func__);
		break;
	}

	pr_info("%s: temp:%d, cycle_count:%d, rp_fv:%d \n", __func__, batt_temp, cycle_count, chip->rp_fv);

	input_current_limit_eea(chip, false);

	vote(chip->icl_votable, POWER_REPLENISH_VOTER, true, POWER_REPLENISH_ICL);
	vote(chip->fcc_votable, POWER_REPLENISH_VOTER, true, replenish_fcc);
	vote(chip->fcc_main_votable, FCC_STEPPER_VOTER, false, replenish_fcc);
	vote(chip->fcc_main_votable, POWER_REPLENISH_VOTER, true, replenish_fcc);

	return rc;
}

#define JEITA_FCC_TAPER_VOTER      "JEITA_FCC_TAPER_VOTER"
static void replenish_work_func(struct work_struct *work)
{
	struct qpnp_qg *chip = container_of(work,
				struct qpnp_qg,
				replenish_work.work);

	chip->prp_is_enable = true;
	pr_info("%s: replenish_en:%d, work_count:%d, replenish_done:%d \n", __func__, chip->prp_is_enable, rp_work_count, is_replenish_done(chip));
	if(rp_work_count == 1){
		vote(chip->chg_dis_votable, JEITA_FCC_TAPER_VOTER, false, 0);
		charge_enable_eea(chip, true);
		set_fv_fcc_cofig_for_replenish(chip);
		set_charge_status_eea(chip, POWER_SUPPLY_STATUS_CHARGING);
	}else if(is_replenish_done(chip)){
		pr_err("power replenish done, reset replenish status\n");
		set_charge_status_eea(chip, POWER_SUPPLY_STATUS_FULL);
		charge_enable_eea(chip, false);
		//vote(chip->icl_votable, EEA_CHARGE_FULL, true, 1000000);
		goto reset;
	}

	if(chip->charge_status == POWER_SUPPLY_STATUS_DISCHARGING || chip->charge_type == POWER_SUPPLY_TYPE_UNKNOWN){
		pr_err("adapter plug out, reset replenish status\n");
		chip->prp_is_enable = false;
		set_charge_status_eea(chip, POWER_SUPPLY_STATUS_INVAILD);
		goto reset;
	}

	rp_work_count ++;
	schedule_delayed_work(&chip->replenish_work, msecs_to_jiffies(REPLENISH_LOOP_WAIT_S));// 3s

	return;

reset:
	rp_work_count = 0;
	cancel_delayed_work(&chip->replenish_work);
	vote(chip->icl_votable, POWER_REPLENISH_VOTER, false, 0);
	vote(chip->fv_votable, POWER_REPLENISH_VOTER, false, 0);
	vote(chip->fcc_main_votable, POWER_REPLENISH_VOTER, false, 0);
	vote(chip->fcc_votable, POWER_REPLENISH_VOTER, false, 0);
	return;
}

static int handle_power_replenish(struct qpnp_qg *chip)
{
	int rc = 0;

	if(chip->is_eu_mode && chip->cycle_count <= 300){
		pr_debug("%s: cycle_count is less than 300, not support replenish for EEA \n", __func__);
		return rc;
	}

	pr_debug("%s: charge_done:%d, replenish_en:%d \n", __func__, chip->charge_status, chip->prp_is_enable);
	if(chip->prp_is_enable == false){
		schedule_delayed_work(&chip->replenish_work, msecs_to_jiffies(REPLENISH_START_WAIT_S)); //5 mins
	}

	return rc;
}

#define CIS_ALERT 			"CIS_ALERT"
#define JEITA_VOTER			"JEITA_VOTER"
#define BATT_PROFILE_VOTER	"BATT_PROFILE_VOTER"
static bool cis_handle_done = false;
static int last_jeita_fcc = 0;
static int handle_battery_cis(struct qpnp_qg *chip)
{
	union power_supply_propval val = {0,};
	int rc = 0;
	int cis_alert_status = 0;
	int eff_fv, eff_fcc;
	int cis_fcc;

	if (!is_gague_available(chip)) {
		chip->fg_psy = power_supply_get_by_name("bq28z610");
		if (!chip->fg_psy) {
			pr_err("%s: get gauge psy failed \n", __func__);
			return -EINVAL;
		}
	}

	rc = power_supply_get_property(chip->fg_psy, POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MIN,
				       &val);
	if (rc < 0) {
		pr_err("Failed to get cis alert, rc=%d\n", rc);
		return rc;
	}

	cis_alert_status = val.intval;

	if(cis_alert_status &&
		(chip->charge_status == POWER_SUPPLY_STATUS_DISCHARGING
		|| chip->charge_type == POWER_SUPPLY_TYPE_UNKNOWN)){
			pr_info("adapter plug out while cis_alert, reset cis status \n");
			cis_handle_done = false;
			vote(chip->fcc_votable, CIS_ALERT, false, 0);
			vote(chip->fcc_main_votable, CIS_ALERT, false, 100000);//100ma
			return rc;
	}

	rc = power_supply_get_property(chip->fg_psy, POWER_SUPPLY_PROP_VOLTAGE_MAX,
				       &val);
	if (rc < 0) {
		pr_err("Failed to get cis alert, rc=%d\n", rc);
		return rc;
	}

	eff_fv = val.intval;
	eff_fcc = get_client_vote_locked(chip->fcc_votable, JEITA_VOTER);
	cis_fcc = get_client_vote_locked(chip->fcc_votable, CIS_ALERT);
	if(cis_fcc < 0)
		cis_fcc = eff_fcc;
	pr_info("%s: eff_fv:%d, eff_fcc:%d, cis_fcc:%d \n", __func__, eff_fv, eff_fcc, cis_fcc);

	if(cis_alert_status >= 3 && eff_fcc != last_jeita_fcc){
		if(chip->fastcharge_mode_enabled)
			vote(chip->fcc_main_votable, CIS_ALERT, true, 100000);//100ma
		vote(chip->fcc_votable, CIS_ALERT, false, 0);
		msleep(20);
		vote(chip->fcc_votable, CIS_ALERT, true, (4 * eff_fcc / 5));
		cis_handle_done = true;
		last_jeita_fcc = eff_fcc;
	}else if(cis_alert_status < 3 && cis_handle_done) {
		pr_info("cis alert is cancled, reset cis status \n");
		cis_handle_done = false;
		vote(chip->fcc_main_votable, CIS_ALERT, false, 100000);//100ma
		vote(chip->fcc_votable, CIS_ALERT, false, 0);
	}

	pr_info("%s: cis_alert_status:%d, cis_handle_done:%d \n", __func__, cis_alert_status, cis_handle_done);
	return rc;
}

#define MCA_EVENT_NOTIFY_SIZE 128
static void connector_protect_uevent(struct qpnp_qg *chip, int val)
{
	int len;
	static char uevent_string[][MCA_EVENT_NOTIFY_SIZE + 1] = { 0 };
	static char *envp[] = {
		uevent_string[0],
		NULL,
	};

	len = snprintf(uevent_string[0], sizeof(uevent_string[0]), "POWER_SUPPLY_CONNECTOR_TEMP=%d", val);

	kobject_uevent_env(&chip->dev->kobj, KOBJ_CHANGE, envp);
}

static void vbus_detect_workfunc(struct work_struct *work) {
	int rc = 0;
	union power_supply_propval val = {0, };
	struct qpnp_qg *chip = container_of(work, struct qpnp_qg, vbus_detect_work.work);

	volt_count ++;
	if (!chip->usb_psy) {
		chip->usb_psy = power_supply_get_by_name("usb");
	}

	if (!IS_ERR_OR_NULL(chip->usb_psy)) {
		rc = power_supply_get_property(chip->usb_psy,
						POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
		pr_info("vbus = %d, volt_count = %d\n", val.intval, volt_count);
		if (rc < 0) {
			pr_debug("Failed to get vbus, rc=%d\n", rc);
		} else if (val.intval <= 5500000) {
			if (!is_gague_available(chip)) {
				pr_err("fg-psy not available\n");
			} else {
				val.intval = 1;
				rc = power_supply_set_property(chip->fg_psy,
							POWER_SUPPLY_PROP_VOLTAGE_BOOT, &val);
				pr_debug("vbus ctrl gpio set high by connector_protect\n");
			}
			detect_done = 1;
			return;
		}
	}
	if (volt_count < 5)
		schedule_delayed_work(&chip->vbus_detect_work, msecs_to_jiffies(200));
	else
		detect_done = 1;
}

static void connector_protect_workfunc(struct work_struct *work) {
	int rc;
	int connector_temp1 = 0;
	int connector_temp2 = 0;
	int temp_max;
	union power_supply_propval prop = {0, };
	struct qpnp_qg *chip = container_of(work, struct qpnp_qg, connector_protect_work.work);

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("Point maybe null\n");
		goto last_temp;
	}

	rc = qg_read_iio_chan(chip, CONN_TEMP, &prop.intval);
	if (rc < 0)
		pr_err("Failed to get conn_temp, rc=%d\n", rc);
	else
		connector_temp1 = prop.intval;

	rc = qg_read_iio_chan(chip, SMB1390_TEMP, &prop.intval);
	if (rc < 0)
		pr_err("Failed to get smb1390_temp, rc=%d\n", rc);
	else
		connector_temp2 = prop.intval;

	if (connector_temp1 < 0 && connector_temp2 < 0) {
		pr_info("Not JP version, cancel connector_protect\n");
		return;
	}

	if (!is_gague_available(chip)) {
		pr_err("fg-psy not available\n");
		goto last_temp;
	}

	temp_max = connector_temp1;
	if (connector_temp1 < connector_temp2)
		temp_max = connector_temp2;

	if(chip->last_temp1 != connector_temp1 || chip->last_temp2 != connector_temp2){
		pr_info("temp1=%d, temp2=%d, max=%d, last1=%d, last2=%d, protect=%d, thermal=%d, event=%d\n",
			connector_temp1, connector_temp2, temp_max, chip->last_temp1, chip->last_temp2,
			chip->protect, chip->thermal_board_temp, chip->charger_plugin_event);
	}

	if (!chip->protect && temp_max > 650) {
		rc = qg_write_iio_chan(chip, DISABLE_OTG, 1);
		if (rc < 0)
			pr_err("Failed to disable otg, rc=%d\n", rc);
		prop.intval = 1;
		rc = power_supply_set_property(chip->fg_psy, POWER_SUPPLY_PROP_SCOPE, &prop);
		if (IS_ERR_OR_NULL(chip->pdo_5v_votable)) {
			chip->pdo_5v_votable = find_votable("PDO_5V");
		}
		if (IS_ERR_OR_NULL(chip->pdo_5v_votable)) {
			pr_err("find votable pdo_5v_votable fail\n");
		} else {
			vote(chip->pdo_5v_votable, PDO_5V_VOTER, true, 1);
			vote(chip->pdo_5v_votable, PDO_5V_VOTER, false, 0);
		}
		volt_count = 0;
		detect_done = 0;
		schedule_delayed_work(&chip->vbus_detect_work, msecs_to_jiffies(500));
		msleep(400);
		connector_protect_uevent(chip, temp_max);
		chip->protect = true;
	} else if(detect_done && chip->protect && connector_temp1 <= 550 &&
		connector_temp2 <= 550 && chip->charger_plugin_event == 2) {
		prop.intval = 0;
		rc = power_supply_set_property(chip->fg_psy, POWER_SUPPLY_PROP_SCOPE, &prop);
		rc |= power_supply_set_property(chip->fg_psy, POWER_SUPPLY_PROP_VOLTAGE_BOOT, &prop);
		if (rc < 0)
			pr_err("Failed to cancel protect, rc=%d\n", rc);
		pr_info("cancel protect\n");
		chip->protect = false;
		goto last_temp;
	}

	schedule_delayed_work(&chip->connector_protect_work, msecs_to_jiffies(1000));

last_temp:
	chip->last_temp1 = connector_temp1;
	chip->last_temp2 = connector_temp2;

}

static void qg_status_change_work(struct work_struct *work)
{
	struct qpnp_qg *chip =
		container_of(work, struct qpnp_qg, qg_status_change_work);
	union power_supply_propval prop = {0, };
	int rc = 0, val, batt_temp;

	if (!is_batt_available(chip)) {
		pr_err("batt-psy not available\n");
		goto out;
	}

	chip->fcc_main_votable = find_votable("FCC_MAIN");
	if (chip->fcc_main_votable == NULL) {
		rc = -EINVAL;
		pr_err("Couldn't find FCC Main votable rc=%d\n", rc);
		goto out;
	}

	chip->fcc_votable = find_votable("FCC");
	if (chip->fcc_votable == NULL) {
		rc = -EINVAL;
		pr_err("Couldn't find FCC votable rc=%d\n", rc);
		goto out;
	}

	chip->icl_votable = find_votable("USB_ICL");
	if (chip->icl_votable == NULL) {
		rc = -EINVAL;
		pr_err("Couldn't find USB_ICL votable rc=%d\n", rc);
		goto out;
	}

	chip->chg_dis_votable = find_votable("CHG_DISABLE");
	if (chip->chg_dis_votable == NULL) {
		pr_err("Couldn't find CHG_DIS votable \n");
		goto out;
	}

	chip->fv_votable = find_votable("FV");
	if(!chip->fv_votable)
	{
		pr_err("Failed to find FV_votable \n");
		goto out;
	}

	prop.intval = get_effective_result(chip->fv_votable);

	chip->is_eu_mode = get_eu_mode();
	pr_debug("%s: is_eu_mode:%d \n", __func__, chip->is_eu_mode);

	rc = power_supply_set_property(chip->batt_psy,
			POWER_SUPPLY_PROP_VOLTAGE_MAX, &prop);
	if (rc < 0) {
		pr_err("Failed to set voltage_max property on batt_psy, rc=%d\n",rc);
	}

	rc = qg_read_iio_chan(chip, USB_REAL_TYPE, &prop.intval);
	if (rc < 0)
		pr_err("Failed to get charge-type, rc=%d\n", rc);
	else
		chip->charge_type = prop.intval;


	rc = power_supply_get_property(chip->batt_psy,
			POWER_SUPPLY_PROP_STATUS, &prop);
	if (rc < 0)
		pr_err("Failed to get charger status, rc=%d\n", rc);
	else
		chip->charge_status = prop.intval;

	rc = qg_read_iio_chan(chip, CHARGE_DONE, &val);
	if (rc < 0)
		pr_err("Failed to get charge done status, rc=%d\n", rc);
	else
		chip->charge_done = val;

	rc = qg_get_cycle_count(chip, &val);
	if (rc < 0)
		pr_err("Failed to get cycle_count, rc=%d\n", rc);
	else
		chip->cycle_count = val;

	rc = qg_get_battery_temp(chip, &batt_temp);
	if (rc < 0)
		pr_err("Failed to get batt_temp, rc=%d\n", rc);

	if(chip->is_eu_mode){
		rc = handle_recharge_eea(chip);
	}

	if(chip->charge_type == POWER_SUPPLY_TYPE_USB_PD
		&& chip->charge_status == POWER_SUPPLY_STATUS_FULL
		&& is_between(201, 451, batt_temp)) {
		rc = handle_power_replenish(chip);
	}

	rc = handle_battery_cis(chip);

	pr_info("%s, charge_type=%d, charge_status=%d charge_done=%d, cycle_count:%d \n",__func__,
			chip->charge_type, chip->charge_status, chip->charge_done, chip->cycle_count);

out:
	pm_relax(chip->dev);
}

static int qg_notifier_cb(struct notifier_block *nb, unsigned long event,
			  void *data)
{
	struct power_supply *psy = data;
	struct qpnp_qg *chip = container_of(nb, struct qpnp_qg, nb);

	if (event != PSY_EVENT_PROP_CHANGED)
		return NOTIFY_OK;

	if (work_pending(&chip->qg_status_change_work))
		return NOTIFY_OK;

	if ((strcmp(psy->desc->name, "battery") == 0) ||
	    (strcmp(psy->desc->name, "parallel") == 0) ||
	    (strcmp(psy->desc->name, "usb") == 0) ||
	    (strcmp(psy->desc->name, "dc") == 0) ||
	    (strcmp(psy->desc->name, "charge_pump_master") == 0)) {
		/*
		 * We cannot vote for awake votable here as that takes
		 * a mutex lock and this is executed in an atomic context.
		 */
		pm_stay_awake(chip->dev);
		schedule_work(&chip->qg_status_change_work);
	}

	return NOTIFY_OK;
}

static int qpnp_smb5_notifier_event_callback(struct notifier_block *qpnp_smb5_nb,
			unsigned long chg_event, void *val)
{
	struct qpnp_qg *chip = container_of(qpnp_smb5_nb, struct qpnp_qg, qpnp_smb5_nb);

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("Point maybe null\n");
		return -EINVAL;
	}

	switch (chg_event) {
		/*charger_plugin_event 1:plugin 2:plugout*/
		case QPNP_SMB5_TYPEC_PLUGIN_EVENT:
			chip->charger_plugin_event = *(int *)val;
			pr_info("fg get charger_plugin_event: %d\n", chip->charger_plugin_event);
			if (!chip->protect && chip->charger_plugin_event == 1) {
				schedule_delayed_work(&chip->connector_protect_work, msecs_to_jiffies(0));
				pr_info("schedule connector_protect_work\n");
			} else if (!chip->protect && chip->charger_plugin_event == 2) {
				cancel_delayed_work(&chip->connector_protect_work);
				chip->last_temp1 = 1000;
				chip->last_temp2 = 1000;
				pr_info("cancel connector_protect_work\n");
			}
			break;
		default:
			pr_debug("fg not supported charger notifier event: %d\n", chg_event);
		break;
	}

	return NOTIFY_DONE;
}

static int qg_psy_get_property(struct power_supply *psy,
			       enum power_supply_property psp,
			       union power_supply_propval *pval)
{
	if (psp == POWER_SUPPLY_PROP_TYPE)
		pval->intval = POWER_SUPPLY_TYPE_MAINS;

	return 0;
}

static enum power_supply_property qg_psy_props[] = {
	POWER_SUPPLY_PROP_TYPE,
};

static const struct power_supply_desc qg_psy_desc = {
	.name = "bms",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.properties = qg_psy_props,
	.num_properties = ARRAY_SIZE(qg_psy_props),
	.get_property = qg_psy_get_property,
};

static int qg_init_psy(struct qpnp_qg *chip)
{
	struct power_supply_config qg_psy_cfg = {};
	int rc = 0;

	qg_psy_cfg.drv_data = chip;
	chip->qg_psy = devm_power_supply_register(chip->dev, &qg_psy_desc,
						  &qg_psy_cfg);
	if (IS_ERR_OR_NULL(chip->qg_psy)) {
		pr_err("Failed to register qg_psy, rc = %d\n",
		       PTR_ERR(chip->qg_psy));
		return -ENODEV;
	}

	chip->nb.notifier_call = qg_notifier_cb;
	rc = power_supply_reg_notifier(&chip->nb);
	if (rc < 0)
		pr_err("Failed to register psy notifier rc = %d\n", rc);

	/*register qpnp_smb5_notifier*/
	chip->qpnp_smb5_nb.notifier_call = qpnp_smb5_notifier_event_callback;
	qpnp_smb5_reg_notifier(&chip->qpnp_smb5_nb);
	if (rc < 0) {
		pr_err("Couldn't register plug notifier rc = %d\n", rc);
		return rc;
	}

	return rc;
}

static int qg_init_iio_psy(struct qpnp_qg *chip, struct platform_device *pdev)
{
	struct iio_dev *indio_dev = chip->indio_dev;
	struct iio_chan_spec *chan;
	int qg_num_iio_channels = ARRAY_SIZE(qg_iio_psy_channels);
	int rc, i;

	chip->iio_chan = devm_kcalloc(chip->dev, qg_num_iio_channels,
				      sizeof(*chip->iio_chan), GFP_KERNEL);
	if (!chip->iio_chan)
		return -ENOMEM;

	chip->int_iio_chans = devm_kcalloc(chip->dev, qg_num_iio_channels,
					   sizeof(*chip->int_iio_chans),
					   GFP_KERNEL);
	if (!chip->int_iio_chans)
		return -ENOMEM;

	chip->ext_iio_chans =
		devm_kcalloc(chip->dev, ARRAY_SIZE(qg_ext_iio_chan_name),
			     sizeof(*chip->ext_iio_chans), GFP_KERNEL);
	if (!chip->ext_iio_chans)
		return -ENOMEM;

	indio_dev->info = &qg_iio_info;
	indio_dev->dev.parent = chip->dev;
	indio_dev->dev.of_node = chip->dev->of_node;
	indio_dev->name = "qpnp,qg";
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = chip->iio_chan;
	indio_dev->num_channels = qg_num_iio_channels;

	for (i = 0; i < qg_num_iio_channels; i++) {
		chip->int_iio_chans[i].indio_dev = indio_dev;
		chan = &chip->iio_chan[i];
		chip->int_iio_chans[i].channel = chan;
		chan->address = i;
		chan->channel = qg_iio_psy_channels[i].channel_num;
		chan->type = qg_iio_psy_channels[i].type;
		chan->datasheet_name = qg_iio_psy_channels[i].datasheet_name;
		chan->extend_name = qg_iio_psy_channels[i].datasheet_name;
		chan->info_mask_separate = qg_iio_psy_channels[i].info_mask;
	}

	rc = devm_iio_device_register(chip->dev, indio_dev);
	if (rc)
		pr_err("Failed to register QG IIO device, rc=%d\n", rc);

	return rc;
}

static int gauge_iio_suspend(struct device *dev)
{
	//struct qpnp_qg *chip = dev_get_drvdata(dev);

	return 0;
}

static int gauge_iio_resume(struct device *dev)
{
	//struct qpnp_qg *chip = dev_get_drvdata(dev);

	return 0;
}

static const struct dev_pm_ops gauge_iio_pm_ops = {
	.suspend = gauge_iio_suspend,
	.resume = gauge_iio_resume,
};

static int gauge_iio_probe(struct platform_device *pdev)
{
	struct qpnp_qg *chip;
	struct iio_dev *indio_dev;
	int rc = 0;
	static int probe_count = 0;

	probe_count ++;

	pr_err("%s: start, count:%d \n", __func__, probe_count);
	indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*chip));
	if (!indio_dev)
		return -ENOMEM;

	chip = iio_priv(indio_dev);
	chip->indio_dev = indio_dev;
	chip->dev = &pdev->dev;
	platform_set_drvdata(pdev, chip);

	chip->soc_reporting_ready = true;
	chip->fake_full_status_flag = false;
	chip->charger_plugin_event = 0;
	chip->last_temp1 = 1000;
	chip->last_temp2 = 1000;
	chip->protect = false;
	chip->thermal_board_temp = 250;
	INIT_WORK(&chip->qg_status_change_work, qg_status_change_work);
	INIT_DELAYED_WORK(&chip->soc_monitor_work, soc_monitor_work);
	INIT_DELAYED_WORK(&chip->replenish_work, replenish_work_func);
	INIT_DELAYED_WORK(&chip->shutdown_delay_work, shutdown_delayed_workfunc);
	INIT_DELAYED_WORK(&chip->connector_protect_work, connector_protect_workfunc);
	INIT_DELAYED_WORK(&chip->vbus_detect_work, vbus_detect_workfunc);
	lc_charger_notifier_register(&lc_charger_chain_nb);

	rc = qg_parse_dt(chip);
	if (rc < 0) {
		pr_err("Failed to parse DT, rc=%d\n", rc);
		return rc;
	}

	rc = qg_register_device(chip);
	if (rc < 0) {
		pr_err("Failed to register QG char device, rc=%d\n", rc);
		return rc;
	}

	rc = qg_init_iio_psy(chip, pdev);
	if (rc < 0) {
		pr_err("Failed to initialize QG IIO PSY, rc=%d\n", rc);
		goto fail_votable;
	}

	rc = qg_init_psy(chip);
	if (rc < 0) {
		pr_err("Failed to initialize QG PSY, rc=%d\n", rc);
		goto fail_votable;
	}

	rc = sysfs_create_groups(&chip->dev->kobj, qg_groups);
	if (rc < 0) {
		pr_err("Failed to create sysfs files rc=%d\n", rc);
		goto fail_votable;
	}
	pr_err("%s: successed \n", __func__);
	return rc;

fail_votable:
	pr_err("%s: failed, count:%d \n", __func__, probe_count);
	device_destroy(chip->qg_class, chip->dev_no);
	cdev_del(&chip->qg_cdev);
	unregister_chrdev_region(chip->dev_no, 1);
	return rc;
}

static int gauge_iio_remove(struct platform_device *pdev)
{
	struct qpnp_qg *chip = platform_get_drvdata(pdev);

	sysfs_remove_groups(&chip->dev->kobj, qg_groups);
	device_destroy(chip->qg_class, chip->dev_no);
	cdev_del(&chip->qg_cdev);
	unregister_chrdev_region(chip->dev_no, 1);

	return 0;
}

static void gauge_iio_shutdown(struct platform_device *pdev)
{
	//struct qpnp_qg *chip = platform_get_drvdata(pdev);
}

static const struct of_device_id match_table[] = {
	{
		.compatible = "qcom,pm7250b-qg",
	},
	{},
};

static struct platform_driver gauge_iio_driver = {
	.driver		= {
		.name		= "qcom,qpnp-qg",
		.of_match_table	= match_table,
		.pm		= &gauge_iio_pm_ops,
	},
	.probe		= gauge_iio_probe,
	.remove		= gauge_iio_remove,
	.shutdown	= gauge_iio_shutdown,
};
module_platform_driver(gauge_iio_driver);

MODULE_DESCRIPTION("Third-apply IIO");
MODULE_LICENSE("GPL");
