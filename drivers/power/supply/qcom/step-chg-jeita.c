// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#define pr_fmt(fmt) "QCOM-STEPCHG: %s: " fmt, __func__

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/pmic-voter.h>
#include <linux/iio/consumer.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>
#include <linux/qti_power_supply.h>
#include "step-chg-jeita.h"
#include "battery-profile-loader.h"
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/timekeeping.h>
#include "charger_partition.h"
#include "bq28z610.h"
#include "smb5-lib.h"
#include "smb5-reg.h"

#define is_between(left, right, value) \
		(((left) >= (right) && (left) >= (value) \
			&& (value) >= (right)) \
		|| ((left) <= (right) && (left) <= (value) \
			&& (value) < (right)))

struct step_chg_cfg {
	struct step_chg_jeita_param	param;
	struct range_data		fcc_cfg[MAX_STEP_CHG_ENTRIES];
};

struct jeita_fcc_cfg {
	struct step_chg_jeita_param	param;
	struct range_data		fcc_cfg[MAX_STEP_CHG_ENTRIES];
};

struct jeita_fv_cfg {
	struct step_chg_jeita_param	param;
	struct range_data		fv_cfg[MAX_STEP_CHG_ENTRIES];
};

struct step_chg_info {
	struct device   *dev;
	bool            step_chg_enable;
	bool            sw_jeita_enable;
	bool            jeita_arb_en;
	bool            config_is_read;
	bool            step_chg_cfg_valid;
	bool            sw_jeita_cfg_valid;
	bool            soc_based_step_chg;
	bool            ocv_based_step_chg;
	bool            vbat_avg_based_step_chg;
	bool            batt_missing;
	bool            taper_fcc;
	bool            jeita_fcc_scaling;
	int             jeita_fcc_index;
	int             jeita_fv_index;
	int             step_index;
	int             get_config_retry_count;
	int             jeita_last_update_temp;
	int             jeita_fcc_scaling_temp_threshold[2];
	long            jeita_max_fcc_ua;
	long            jeita_fcc_step_size;
	int             batt_id;
	int             chg_term_current;
	int             targer_fv_uv;
	int             fv_drop_time;
	int             final_fv_uv;
	int             final_fcc_ua;
	int             targer_fcc_ua;
	int             flag_fcc_taper;
	int             fg_vbat_uv;
	int             cp_enable;
	int             fg_ibat;
	int             smart_batt;
	int             batt_soc;
	int             fg_rsoc;
	int             fg_temp;
	int             high_soc_plugin;
	int             is_need_get_timer;

	struct step_chg_cfg      *step_chg_config;
	struct jeita_fcc_cfg     *jeita_fcc_config;
	struct jeita_fv_cfg      *jeita_fv_config;

	struct votable           *fcc_votable;
	struct votable           *fv_votable;
	struct votable           *usb_icl_votable;
	struct votable           *chg_disable_votable;
	struct votable           *awake_votable;
	struct wakeup_source     *step_chg_ws;
	struct power_supply      *batt_psy;
	struct power_supply      *usb_psy;
	struct power_supply      *dc_psy;
	struct delayed_work      status_change_work;
	struct delayed_work      get_config_work;
	struct delayed_work      fcc_taper_work;
	struct notifier_block    nb;
	struct iio_channel      *iio_chans;
	struct iio_channel      **iio_chan_list_qg;

	/*charger_plugin_event 0:none 1:plugin 2:plugout*/
	int                     charger_plugin_event;
	struct notifier_block   qpnp_smb5_nb;
};

static struct step_chg_info *the_chip;
static int high_temp = 0;
static int low_temp = 0;

struct range_data step_chg_ffc_ranges[MAX_STEP_CHG_ENTRIES] = {
	{3600000,4250000,6000000},// step chg for cycle_count 50
	{4250000,4560000,5400000},
	{3600000,4200000,6000000},// step chg for cycle_count 200
	{4200000,4540000,5400000},
	{3600000,4170000,6000000},// step chg for cycle_count 400
	{4170000,4520000,5400000},
	{3600000,4150000,4800000},// step chg for cycle_count 900
	{4150000,4510000,4320000},
};

struct range_data step_chg_eu_range1[MAX_STEP_CHG_ENTRIES] = {
	{3600000,4250000,6000000},// step chg for cycle_count 50
	{4250000,4490000,5400000},
	{3600000,4250000,6000000},// step chg for cycle_count 200
	{4250000,4480000,5400000},
	{3600000,4250000,6000000},// step chg for cycle_count 400
	{4250000,4470000,5400000},
	{3600000,4250000,4800000},// step chg for cycle_count 900
	{4250000,4450000,4320000},
};

struct range_data step_chg_eu_range2[MAX_STEP_CHG_ENTRIES] = {
	{3600000,4250000,6000000},// step chg for cycle_count 50
	{4250000,4480000,5400000},
	{3600000,4250000,6000000},// step chg for cycle_count 200
	{4250000,4470000,5400000},
	{3600000,4250000,6000000},// step chg for cycle_count 400
	{4250000,4460000,5400000},
	{3600000,4250000,4800000},// step chg for cycle_count 900
	{4250000,4440000,4320000},
};


struct range_data step_chg_eu_range3[MAX_STEP_CHG_ENTRIES] = {
	{3600000,4250000,6000000},// step chg for cycle_count 50
	{4250000,4470000,5400000},
	{3600000,4250000,6000000},// step chg for cycle_count 200
	{4250000,4460000,5400000},
	{3600000,4250000,6000000},// step chg for cycle_count 400
	{4250000,4450000,5400000},
	{3600000,4250000,4800000},// step chg for cycle_count 900
	{4250000,4430000,4320000},
};

struct range_data step_chg_normal_range1[MAX_STEP_CHG_ENTRIES] = {
	{3600000,4490000,5400000},// step chg for cycle_count 50
	{3600000,4480000,5400000},// step chg for cycle_count 200
	{3600000,4470000,5400000},// step chg for cycle_count 400
	{3600000,4450000,4320000},// step chg for cycle_count 900
};

struct range_data step_chg_normal_range2[MAX_STEP_CHG_ENTRIES] = {
	{3600000,4480000,5400000},// step chg for cycle_count 50
	{3600000,4470000,5400000},// step chg for cycle_count 200
	{3600000,4460000,5400000},// step chg for cycle_count 400
	{3600000,4440000,4320000},// step chg for cycle_count 900
};

struct range_data step_chg_normal_range3[MAX_STEP_CHG_ENTRIES] = {
	{3600000,4470000,5400000},// step chg for cycle_count 50
	{3600000,4460000,5400000},// step chg for cycle_count 200
	{3600000,4450000,5400000},// step chg for cycle_count 400
	{3600000,4430000,4320000},// step chg for cycle_count 900
};

struct range_data cycle_count_ffc_ranges[MAX_STEP_CHG_ENTRIES] = {
	{0,      100,      4560000},
	{100,    300,      4540000},
	{300,    800,      4520000},
	{800,    10000,    4510000},
};

struct range_data cycle_count_ffc_range1[MAX_STEP_CHG_ENTRIES] = {
	{0,      100,      4530000},
	{100,    300,      4530000},
	{300,    800,      4530000},
	{800,    10000,    4530000},
};

struct range_data cycle_count_eu_range1[MAX_STEP_CHG_ENTRIES] = {
	{0,      100,      4490000},
	{100,    300,      4480000},
	{300,    800,      4470000},
	{800,    10000,    4450000},
};

struct range_data cycle_count_eu_range2[MAX_STEP_CHG_ENTRIES] = {
	{0,      100,      4480000},
	{100,    300,      4470000},
	{300,    800,      4460000},
	{800,    10000,    4440000},
};

struct range_data cycle_count_eu_range3[MAX_STEP_CHG_ENTRIES] = {
	{0,      100,      4470000},
	{100,    300,      4460000},
	{300,    800,      4450000},
	{800,    10000,    4430000},
};

struct range_data jeita_ffc_ranges[MAX_STEP_CHG_ENTRIES] = {
	{(-100), 0,      687000},
	{0,      50,     1300000},
	{50,     100,    2000000},
	{100,    150,    4809000},
	{150,    200,    5496000},
	{200,    350,    6000000},
	{350,    450,    6000000},
	{450,    550,    1717500},
};

struct range_data jeita_eu_ranges[MAX_STEP_CHG_ENTRIES] = {
	{(-100), 0,      687000},
	{0,      50,     1300000},
	{50,     100,    2000000},
	{100,    150,    4809000},
	{150,    200,    5496000},
	{200,    350,    6000000},
	{350,    450,    6000000},
	{450,    550,    1717500},
};

struct range_data jeita_normal_ranges[MAX_STEP_CHG_ENTRIES] = {
	{(-100), 0,      687000},
	{0,      50,     1300000},
	{50,     100,    2000000},
	{100,    150,    4809000},
	{150,    200,    5496000},
	{200,    350,    5400000},
	{350,    450,    5400000},
	{450,    550,    1717500},
};

static bool is_batt_available(struct step_chg_info *chip)
{
	if (!chip->batt_psy)
		chip->batt_psy = power_supply_get_by_name("battery");

	if (!chip->batt_psy)
		return false;

	return true;
}

static const char * const step_chg_ext_iio_chan[] = {
	[STEP_QG_RESISTANCE_ID] = "resistance_id",
	[STEP_QG_VOLTAGE_NOW] = "voltage_now",
	[STEP_QG_TEMP] = "temp",
	[STEP_QG_CAPACITY] = "capacity",
	[STEP_QG_VOLTAGE_OCV] = "voltage_ocv",
	[STEP_QG_VOLTAGE_AVG] = "voltage_avg",
};

static bool is_bms_available(struct step_chg_info *chip)
{
	int rc = 0;
	struct iio_channel **iio_list;

	if (IS_ERR(chip->iio_chan_list_qg))
		return false;

	if (!chip->iio_chan_list_qg) {
		iio_list = get_ext_channels(chip->dev, step_chg_ext_iio_chan,
				ARRAY_SIZE(step_chg_ext_iio_chan));
		if (IS_ERR(iio_list)) {
			rc = PTR_ERR(iio_list);
			if (rc != -EPROBE_DEFER) {
				dev_err(chip->dev, "Failed to get channels, %d\n",
						rc);
				chip->iio_chan_list_qg = ERR_PTR(-EINVAL);
			}
			return false;
		}
		chip->iio_chan_list_qg = iio_list;
	}

	return true;
}

static bool is_usb_available(struct step_chg_info *chip)
{
	if (!chip->usb_psy)
		chip->usb_psy = power_supply_get_by_name("usb");

	if (!chip->usb_psy)
		return false;

	return true;
}

static bool is_input_present(struct step_chg_info *chip)
{
	int rc = 0, input_present = 0;
	union power_supply_propval pval = {0, };

	if (!chip->usb_psy)
		chip->usb_psy = power_supply_get_by_name("usb");
	if (chip->usb_psy) {
		rc = power_supply_get_property(chip->usb_psy,
				POWER_SUPPLY_PROP_PRESENT, &pval);
		if (rc < 0)
			pr_err("Couldn't read USB Present status, rc=%d\n", rc);
		else
			input_present |= pval.intval;
	}

	if (!chip->dc_psy)
		chip->dc_psy = power_supply_get_by_name("dc");
	if (chip->dc_psy) {
		rc = power_supply_get_property(chip->dc_psy,
				POWER_SUPPLY_PROP_PRESENT, &pval);
		if (rc < 0)
			pr_err("Couldn't read DC Present status, rc=%d\n", rc);
		else
			input_present |= pval.intval;
	}

	if (input_present)
		return true;

	return false;
}

static int read_range_data_from_node(struct device_node *node,
		const char *prop_str, struct range_data *ranges,
		int max_threshold, u32 max_value)
{
	int rc = 0, i, length, per_tuple_length, tuples;

	if (!node || !prop_str || !ranges) {
		pr_err("Invalid parameters passed\n");
		return -EINVAL;
	}

	rc = of_property_count_elems_of_size(node, prop_str, sizeof(u32));
	if (rc < 0) {
		pr_err("Count %s failed, rc=%d\n", prop_str, rc);
		return rc;
	}

	length = rc;
	per_tuple_length = sizeof(struct range_data) / sizeof(u32);
	if (length % per_tuple_length) {
		pr_err("%s length (%d) should be multiple of %d\n",
				prop_str, length, per_tuple_length);
		return -EINVAL;
	}
	tuples = length / per_tuple_length;

	if (tuples > MAX_STEP_CHG_ENTRIES) {
		pr_err("too many entries(%d), only %d allowed\n",
				tuples, MAX_STEP_CHG_ENTRIES);
		return -EINVAL;
	}

	rc = of_property_read_u32_array(node, prop_str,
			(u32 *)ranges, length);
	if (rc) {
		pr_err("Read %s failed, rc=%d\n", prop_str, rc);
		return rc;
	}

	for (i = 0; i < tuples; i++) {
		if (ranges[i].low_threshold >
				ranges[i].high_threshold) {
			pr_err("%s thresholds should be in ascendant ranges\n",
						prop_str);
			rc = -EINVAL;
			goto clean;
		}

		if (i != 0) {
			if (ranges[i - 1].high_threshold >
					ranges[i].low_threshold) {
				pr_err("%s thresholds should be in ascendant ranges\n",
							prop_str);
				rc = -EINVAL;
				goto clean;
			}
		}

		if (ranges[i].low_threshold > max_threshold)
			ranges[i].low_threshold = max_threshold;
		if (ranges[i].high_threshold > max_threshold)
			ranges[i].high_threshold = max_threshold;
		if (ranges[i].value > max_value)
			ranges[i].value = max_value;
	}

	return rc;
clean:
	memset(ranges, 0, tuples * sizeof(struct range_data));
	return rc;
}

static int step_chg_read_iio_prop(struct step_chg_info *chip,
		enum iio_type type, int iio_chan, int *val)
{
	struct iio_channel *iio_chan_list;
	int rc;

	switch (type) {
	case MAIN:
		if (!chip->iio_chans)
			return -ENODEV;
		iio_chan_list = &chip->iio_chans[iio_chan];
		break;
	case QG:
		if (IS_ERR_OR_NULL(chip->iio_chan_list_qg))
			return -ENODEV;
		iio_chan_list = chip->iio_chan_list_qg[iio_chan];
		break;
	default:
		pr_err_ratelimited("iio_type %d is not supported\n", type);
		return -EINVAL;
	}

	rc = iio_read_channel_processed(iio_chan_list, val);
	return (rc < 0) ? rc : 0;
}

static int step_chg_write_iio_prop(struct step_chg_info *chip,
		enum iio_type type, int iio_chan, int val)
{
	struct iio_channel *iio_chan_list;

	switch (type) {
	case MAIN:
		if (!chip->iio_chans)
			return -ENODEV;
		iio_chan_list = &chip->iio_chans[iio_chan];
		break;
	default:
		pr_err_ratelimited("iio_type %d is not supported\n", type);
		return -EINVAL;
	}

	return iio_write_channel_raw(iio_chan_list, val);
}

static int get_step_chg_jeita_setting_from_profile(struct step_chg_info *chip)
{
	struct device_node *batt_node, *profile_node;
	u32 max_fv_uv, max_fcc_ma;
	const char *batt_type_str;
	const __be32 *handle;
	int batt_id_ohms, rc, hysteresis[2] = {0};
	u32 jeita_scaling_min_fcc_ua = 0;

	handle = of_get_property(chip->dev->of_node,
			"qcom,battery-data", NULL);
	if (!handle) {
		pr_debug("ignore getting sw-jeita/step charging settings from profile\n");
		return 0;
	}

	batt_node = of_find_node_by_phandle(be32_to_cpup(handle));
	if (!batt_node) {
		pr_err("Get battery data node failed\n");
		return -EINVAL;
	}

	if (!is_bms_available(chip))
		return -ENODEV;

	rc = step_chg_read_iio_prop(chip, QG, STEP_QG_RESISTANCE_ID,
			&batt_id_ohms);
	if (rc < 0)
		pr_err("Failed to read batt_id rc=%d\n", rc);

	/* bms_psy has not yet read the batt_id */
	if (batt_id_ohms < 0)
		return -EBUSY;

	profile_node = of_batterydata_get_best_profile(batt_node,
					batt_id_ohms / 1000, NULL);
	if (IS_ERR(profile_node))
		return PTR_ERR(profile_node);

	if (!profile_node) {
		pr_err("Couldn't find profile\n");
		return -ENODATA;
	}

	rc = of_property_read_string(profile_node, "qcom,battery-type",
					&batt_type_str);
	if (rc < 0) {
		pr_err("battery type unavailable, rc:%d\n", rc);
		return rc;
	}
	pr_debug("battery: %s detected, getting sw-jeita/step charging settings\n",
					batt_type_str);

	rc = of_property_read_u32(profile_node, "qcom,max-voltage-uv",
					&max_fv_uv);
	if (rc < 0) {
		pr_err("max-voltage_uv reading failed, rc=%d\n", rc);
		return rc;
	}

	rc = of_property_read_u32(profile_node, "qcom,fastchg-current-ma",
					&max_fcc_ma);
	if (rc < 0) {
		pr_err("max-fastchg-current-ma reading failed, rc=%d\n", rc);
		return rc;
	}
	chip->jeita_max_fcc_ua = max_fcc_ma * 1000;

	chip->taper_fcc = of_property_read_bool(profile_node, "qcom,taper-fcc");

	chip->soc_based_step_chg =
		of_property_read_bool(profile_node, "qcom,soc-based-step-chg");
	if (chip->soc_based_step_chg) {
		chip->step_chg_config->param.psy_prop =
				POWER_SUPPLY_PROP_CAPACITY;
		chip->step_chg_config->param.iio_prop = STEP_QG_CAPACITY;
		chip->step_chg_config->param.prop_name = "SOC";
		chip->step_chg_config->param.rise_hys = 0;
		chip->step_chg_config->param.fall_hys = 0;
	}

	chip->ocv_based_step_chg =
		of_property_read_bool(profile_node, "qcom,ocv-based-step-chg");
	if (chip->ocv_based_step_chg) {
		chip->step_chg_config->param.psy_prop =
				POWER_SUPPLY_PROP_VOLTAGE_OCV;
		chip->step_chg_config->param.iio_prop = STEP_QG_VOLTAGE_OCV;
		chip->step_chg_config->param.prop_name = "OCV";
		chip->step_chg_config->param.rise_hys = 0;
		chip->step_chg_config->param.fall_hys = 0;
		chip->step_chg_config->param.use_bms = true;
	}

	chip->vbat_avg_based_step_chg =
				of_property_read_bool(profile_node,
				"qcom,vbat-avg-based-step-chg");
	if (chip->vbat_avg_based_step_chg) {
		chip->step_chg_config->param.psy_prop =
				POWER_SUPPLY_PROP_VOLTAGE_AVG;
		chip->step_chg_config->param.iio_prop = STEP_QG_VOLTAGE_AVG;
		chip->step_chg_config->param.prop_name = "VBAT_AVG";
		chip->step_chg_config->param.rise_hys = 0;
		chip->step_chg_config->param.fall_hys = 0;
		chip->step_chg_config->param.use_bms = true;
	}

	chip->step_chg_cfg_valid = true;
	rc = read_range_data_from_node(profile_node,
			"qcom,step-chg-ranges",
			chip->step_chg_config->fcc_cfg,
			chip->soc_based_step_chg ? 100 : max_fv_uv,
			max_fcc_ma * 1000);
	if (rc < 0) {
		pr_debug("Read qcom,step-chg-ranges failed from battery profile, rc=%d\n",
					rc);
		chip->step_chg_cfg_valid = false;
	}

	chip->sw_jeita_cfg_valid = true;
	rc = read_range_data_from_node(profile_node,
			"qcom,jeita-fcc-ranges",
			chip->jeita_fcc_config->fcc_cfg,
			BATT_HOT_DECIDEGREE_MAX, max_fcc_ma * 1000);
	if (rc < 0) {
		pr_debug("Read qcom,jeita-fcc-ranges failed from battery profile, rc=%d\n",
					rc);
		chip->sw_jeita_cfg_valid = false;
	}

	rc = of_property_read_u32_array(profile_node,
			"qcom,step-jeita-hysteresis", hysteresis, 2);
	if (!rc) {
		chip->jeita_fcc_config->param.rise_hys = hysteresis[0];
		chip->jeita_fcc_config->param.fall_hys = hysteresis[1];
		pr_debug("jeita-fcc-hys: rise_hys=%u, fall_hys=%u\n",
			hysteresis[0], hysteresis[1]);
	}

	rc = read_range_data_from_node(profile_node,
			"qcom,jeita-fv-ranges",
			chip->jeita_fv_config->fv_cfg,
			BATT_HOT_DECIDEGREE_MAX, max_fv_uv);
	if (rc < 0) {
		pr_debug("Read qcom,jeita-fv-ranges failed from battery profile, rc=%d\n",
					rc);
		chip->sw_jeita_cfg_valid = false;
	}

	if (of_property_read_bool(profile_node, "qcom,jeita-fcc-scaling")) {

		rc = of_property_read_u32_array(profile_node,
				"qcom,jeita-fcc-scaling-temp-threshold",
				chip->jeita_fcc_scaling_temp_threshold, 2);
		if (rc < 0)
			pr_debug("Read jeita-fcc-scaling-temp-threshold from battery profile, rc=%d\n",
				rc);

		rc = of_property_read_u32(profile_node,
			"qcom,jeita-scaling-min-fcc-ua",
			&jeita_scaling_min_fcc_ua);
		if (rc < 0)
			pr_debug("Read jeita-scaling-min-fcc-ua from battery profile, rc=%d\n",
				rc);

		if ((jeita_scaling_min_fcc_ua &&
			(jeita_scaling_min_fcc_ua < chip->jeita_max_fcc_ua)) &&
			(chip->jeita_fcc_scaling_temp_threshold[0] <
			chip->jeita_fcc_scaling_temp_threshold[1])) {
			/*
			 * Calculate jeita-fcc-step-size =
			 *	(difference-in-fcc) / ( difference-in-temp)
			 */
			chip->jeita_fcc_step_size = div_s64(
			(chip->jeita_max_fcc_ua - jeita_scaling_min_fcc_ua),
			(chip->jeita_fcc_scaling_temp_threshold[1] -
				chip->jeita_fcc_scaling_temp_threshold[0]));

			if (chip->jeita_fcc_step_size > 0)
				chip->jeita_fcc_scaling = true;
		}

		pr_debug("jeita-fcc-scaling: enabled = %d, jeita-fcc-scaling-temp-threshold = [%d, %d], jeita-scaling-min-fcc-ua = %ld, jeita-scaling-max_fcc_ua = %ld,jeita-fcc-step-size = %ld\n",
			chip->jeita_fcc_scaling,
			chip->jeita_fcc_scaling_temp_threshold[0],
			chip->jeita_fcc_scaling_temp_threshold[1],
			jeita_scaling_min_fcc_ua, chip->jeita_max_fcc_ua,
			chip->jeita_fcc_step_size
			);
	}

	return rc;
}

static void get_config_work(struct work_struct *work)
{
	struct step_chg_info *chip = container_of(work,
			struct step_chg_info, get_config_work.work);
	int i, rc;

	chip->config_is_read = false;
	rc = get_step_chg_jeita_setting_from_profile(chip);

	if (rc < 0) {
		if (rc == -ENODEV || rc == -EBUSY) {
			if (chip->get_config_retry_count++
					< GET_CONFIG_RETRY_COUNT) {
				pr_debug("bms is not ready, retry: %d\n",
						chip->get_config_retry_count);
				goto reschedule;
			}
		}
	}

	chip->config_is_read = true;

	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++)
		pr_debug("step-chg-cfg: %duV(SoC) ~ %duV(SoC), %duA\n",
			chip->step_chg_config->fcc_cfg[i].low_threshold,
			chip->step_chg_config->fcc_cfg[i].high_threshold,
			chip->step_chg_config->fcc_cfg[i].value);
	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++)
		pr_debug("jeita-fcc-cfg: %ddecidegree ~ %ddecidegre, %duA\n",
			chip->jeita_fcc_config->fcc_cfg[i].low_threshold,
			chip->jeita_fcc_config->fcc_cfg[i].high_threshold,
			chip->jeita_fcc_config->fcc_cfg[i].value);
	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++)
		pr_debug("jeita-fv-cfg: %ddecidegree ~ %ddecidegre, %duV\n",
			chip->jeita_fv_config->fv_cfg[i].low_threshold,
			chip->jeita_fv_config->fv_cfg[i].high_threshold,
			chip->jeita_fv_config->fv_cfg[i].value);

	return;

reschedule:
	schedule_delayed_work(&chip->get_config_work,
			msecs_to_jiffies(GET_CONFIG_DELAY_MS));

}

static int get_val(struct range_data *range, int rise_hys, int fall_hys,
		int current_index, int threshold, int *new_index, int *val)
{
	int i;

	*new_index = -EINVAL;

	/*
	 * If the threshold is lesser than the minimum allowed range,
	 * return -ENODATA.
	 */
	if (threshold < range[0].low_threshold)
		return -ENODATA;

	/* First try to find the matching index without hysteresis */
	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++) {
		if (!range[i].high_threshold && !range[i].low_threshold) {
			/* First invalid table entry; exit loop */
			break;
		}

		if (is_between(range[i].low_threshold,
			range[i].high_threshold, threshold)) {
			*new_index = i;
			*val = range[i].value;
			break;
		}
	}

	/*
	 * If nothing was found, the threshold exceeds the max range for sure
	 * as the other case where it is lesser than the min range is handled
	 * at the very beginning of this function. Therefore, clip it to the
	 * max allowed range value, which is the one corresponding to the last
	 * valid entry in the battery profile data array.
	 */
	if (*new_index == -EINVAL) {
		if (i == 0) {
			/* Battery profile data array is completely invalid */
			return -ENODATA;
		}

		*new_index = (i - 1);
		*val = range[*new_index].value;
	}

	/*
	 * If we don't have a current_index return this
	 * newfound value. There is no hysterisis from out of range
	 * to in range transition
	 */
	if (current_index == -EINVAL)
		return 0;

	/*
	 * Check for hysteresis if it in the neighbourhood
	 * of our current index.
	 */
	if ((*new_index == current_index + 1) && (*new_index < 5)) {
		if (threshold <
			(range[*new_index].low_threshold + rise_hys)) {
			/*
			 * Stay in the current index, threshold is not higher
			 * by hysteresis amount
			 */
			*new_index = current_index;
			*val = range[current_index].value;
		}
	} else if ((*new_index == current_index - 1) && (*new_index >= 5)) {
		if (threshold >
			range[*new_index].high_threshold - fall_hys) {
			/*
			 * stay in the current index, threshold is not lower
			 * by hysteresis amount
			 */
			*new_index = current_index;
			*val = range[current_index].value;
		}
	}
	return 0;
}

static void taper_fcc_step_chg(struct step_chg_info *chip, int index,
					int current_voltage)
{
	u32 current_fcc, target_fcc;

	if (index < 0) {
		pr_err("Invalid STEP CHG index\n");
		return;
	}

	current_fcc = get_effective_result(chip->fcc_votable);
	target_fcc = chip->step_chg_config->fcc_cfg[index].value;

	pr_info("index: %d, current_fcc: %d, target_fcc: %d, current_voltage: %d",
				index, current_fcc, target_fcc, current_voltage);
	if (index == 0) {
		pr_info("index = %d, enable target_fcc\n", index);
		vote(chip->fcc_votable, STEP_CHG_VOTER, true, target_fcc);
	} else if (current_voltage >
		(chip->step_chg_config->fcc_cfg[index - 1].high_threshold +
		chip->step_chg_config->param.rise_hys)) {
		/*
		 * Ramp down FCC in pre-configured steps till the current index
		 * FCC configuration is reached, whenever the step charging
		 * control parameter exceeds the high threshold of previous
		 * step charging index configuration.
		 */
		pr_info("current_voltage: %d, high_threshold: %d, rise_hys: %d, target_fcc: %d, current_fcc: %d, step: %d",
					current_voltage,
					chip->step_chg_config->fcc_cfg[index - 1].high_threshold,
					chip->step_chg_config->param.rise_hys,
					target_fcc,
					current_fcc,
					TAPERED_STEP_CHG_FCC_REDUCTION_STEP_MA);
		vote(chip->fcc_votable, STEP_CHG_VOTER, true, max(target_fcc,
			current_fcc - TAPERED_STEP_CHG_FCC_REDUCTION_STEP_MA));
	} else if ((current_fcc >
		chip->step_chg_config->fcc_cfg[index - 1].value) &&
		(current_voltage >
		chip->step_chg_config->fcc_cfg[index - 1].low_threshold +
		chip->step_chg_config->param.fall_hys)) {
		/*
		 * In case the step charging index switch to the next higher
		 * index without FCCs saturation for the previous index, ramp
		 * down FCC till previous index FCC configuration is reached.
		 */
		pr_info("current_voltage: %d, low_threshold: %d, fall_hys: %d, target_fcc: %d, current_fcc: %d, value: %d step: %d",
					current_voltage,
					chip->step_chg_config->fcc_cfg[index - 1].low_threshold,
					chip->step_chg_config->param.fall_hys,
					target_fcc,
					current_fcc,
					chip->step_chg_config->fcc_cfg[index - 1].value,
					TAPERED_STEP_CHG_FCC_REDUCTION_STEP_MA);
		vote(chip->fcc_votable, STEP_CHG_VOTER, true,
			max(chip->step_chg_config->fcc_cfg[index - 1].value,
			current_fcc - TAPERED_STEP_CHG_FCC_REDUCTION_STEP_MA));
	}
}

int get_gauge_fastcharg_mode(struct step_chg_info *chip)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = 0;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_CURRENT_BOOT, &val);
	if (rc < 0) {
		pr_err("Get fastcharg mode failed, rc=%d\n", rc);
		return 0;
	}

	return val.intval;
}

int set_gauge_fastcharg_mode(struct step_chg_info *chip, bool enable)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = 0;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	val.intval = !!enable;
	rc = power_supply_set_property(psy_fg, POWER_SUPPLY_PROP_CURRENT_BOOT, &val);
	if (rc < 0) {
		pr_err("Set fastcharg mode failed, rc=%d\n", rc);
		return 0;
	}

	return rc;
}

int check_pd_active(struct step_chg_info *chip)
{
	union power_supply_propval val = {0, };
	int rc = 0;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return 0;
	}

	rc = step_chg_read_iio_prop(chip, MAIN, PSY_IIO_PD_ACTIVE, &val.intval);
	if (rc < 0) {
		pr_err("Get fastcharge mode status failed, rc=%d\n", rc);
		return 0;
	}

	return val.intval;
}

int check_cp_enable(struct step_chg_info *chip)
{
	union power_supply_propval val = {0, };
	struct power_supply *cp_psy = NULL;
	int rc = 0;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	cp_psy = power_supply_get_by_name("ti-cp-standalone");
	if (IS_ERR_OR_NULL(cp_psy)) {
		cp_psy = power_supply_get_by_name("sc-cp-standalone");
		if (IS_ERR_OR_NULL(cp_psy)) {
			pr_err("chip is err or null\n");
			return rc;
		}
	}

	rc = power_supply_get_property(cp_psy, POWER_SUPPLY_PROP_ONLINE, &val);
	if (rc < 0) {
		pr_err("Get charger pump online failed, rc=%d\n", rc);
		return 0;
	}

	return val.intval;
}

static int jeita_get_gauge_cv(struct step_chg_info *chip)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = -EINVAL;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_VOLTAGE_MAX, &val);
	if (rc < 0) {
		pr_err("Get cv failed, rc: %d\n", rc);
		return rc;
	}

	return val.intval;
}

static int jeita_set_gauge_cv(struct step_chg_info *chip, int cv)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = -EINVAL;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	if (cv < 0) {
		pr_err("Einval cv (%d)\n", cv);
		return rc;
	} else {
		val.intval = cv;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_set_property(psy_fg, POWER_SUPPLY_PROP_VOLTAGE_MAX, &val);
	if (rc < 0) {
		pr_err("Set cv failed, rc: %d\n", rc);
		return rc;
	}

	pr_info("cv: %d(%d)", cv, val.intval);
	return val.intval;
}

static int jeita_get_gauge_iterm(struct step_chg_info *chip)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = -EINVAL;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT, &val);
	if (rc < 0) {
		pr_err("Get charger iterm failed, rc: %d\n", rc);
		return rc;
	}

	return val.intval;
}

static int jeita_set_gauge_iterm(struct step_chg_info *chip, int iterm)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = -EINVAL;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	if (iterm < 0) {
		pr_err("Einval iterm (%d)\n", iterm);
		return rc;
	} else {
		val.intval = iterm;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_set_property(psy_fg, POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT, &val);
	if (rc < 0) {
		pr_err("Set charger iterm failed, rc: %d\n", rc);
		return rc;
	}

	pr_info("iterm: %d(%d)", iterm, val.intval);
	return val.intval;
}

static int jeita_get_gauge_rsoc(struct step_chg_info *chip)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = 0;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_CAPACITY_ALERT_MAX, &val);
	if (rc < 0) {
		pr_err("Get fg rsoc failed, rc=%d\n", rc);
		return 0;
	}

	return val.intval;
}

static int jeita_get_gauge_cis(struct step_chg_info *chip)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = 0;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MIN, &val);
	if (rc < 0) {
		pr_err("Get vbat failed, rc=%d\n", rc);
		return 0;
	}

	return val.intval; //uV
}

static int jeita_get_gauge_vbat(struct step_chg_info *chip)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = 0;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
	if (rc < 0) {
		pr_err("Get vbat failed, rc=%d\n", rc);
		return 0;
	}

	return val.intval;   //uV
}

static int jeita_get_gauge_ibat(struct step_chg_info *chip)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = -EINVAL;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_CURRENT_NOW, &val);
	if (rc < 0) {
		pr_err("Get ibat failed, rc: %d\n", rc);
		return rc;
	}

	return val.intval;
}

int jeita_step_set_charger_done(struct step_chg_info *chip, bool enable)
{
	union power_supply_propval val = {0, };
	int rc = -EINVAL;

	pr_err("enter\n");
	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	if (IS_ERR_OR_NULL(chip->batt_psy)){
		chip->batt_psy = power_supply_get_by_name("battery");
	}

	if (IS_ERR_OR_NULL(chip->batt_psy)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	if (IS_ERR_OR_NULL(chip->chg_disable_votable)) {
		chip->chg_disable_votable = find_votable("CHG_DISABLE");
	}

	if (IS_ERR_OR_NULL(chip->chg_disable_votable)) {
		pr_err("Couldn't find chg_disable_votable\n");
		return rc;
	}

	if (enable) {
		val.intval = POWER_SUPPLY_STATUS_FULL;
	} else {
		val.intval = -EINVAL;
	}

	if (chip->fg_temp < 450) {
		rc = power_supply_set_property(chip->batt_psy, POWER_SUPPLY_PROP_STATUS, &val);
		if (rc < 0) {
			pr_err("Failed to set status gauge to full , rc=%d\n", rc);
		}
	} else {
		pr_info("Temp is too hight(%d), needn't set charger status to full\n", chip->fg_temp);
	}

	rc = vote(chip->chg_disable_votable, JEITA_FCC_TAPER_VOTER, enable, 0);
	if (rc < 0) {
		pr_err("Failed to set JEITA_FCC_TAPER_VOTER , rc=%d\n", rc);
		return rc;
	}

	pr_info("enable: %d, rc: %d\n", enable, rc);
	return rc;
}

int set_fg_full(struct step_chg_info *chip)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = 0;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	if (chip->fg_temp >= 450) {
		pr_info("Temp to high(%d), needn't set fg status to full\n", chip->fg_temp);
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	rc = power_supply_set_property(psy_fg, POWER_SUPPLY_PROP_CHARGE_CONTROL_END_THRESHOLD, &val);
	if (rc < 0) {
		pr_err("Failed to set status gauge to full , rc=%d\n", rc);
		return 0;
	}

	return val.intval;   //uV
}

static int config_step_chg_cycle_count(struct step_chg_info *chip)
{
	int rc, i, pd_active, gauge_ffc_mode;
	bool fastcharge_mode, is_eu_mode;
	int cycle_count, cis_status, batt_temp = 0;
	const char *final_fv_client;
	union power_supply_propval val = {0, };

	if(IS_ERR_OR_NULL(chip)) {
		pr_err("Point is err or null\n");
		return -EINVAL;
	}

	is_eu_mode = get_eu_mode();

	rc = power_supply_get_property(chip->batt_psy,
				POWER_SUPPLY_PROP_CYCLE_COUNT, &val);
	if (rc < 0) {
		pr_err("cycle_count is not ready fail rc = %d\n", rc);
				return rc;
	}
	cycle_count = val.intval;

	/* set and clear fast charge mode when soft jeita trigger and clear */
	rc = step_chg_read_iio_prop(chip, MAIN, PSY_IIO_SMB_FASTCHARGE_MODE, &val.intval);
	if (rc < 0) {
		pr_err("Couldn't read fastcharge mode fail rc=%d\n", rc);
		return rc;
	}
	fastcharge_mode = !!val.intval;

	pd_active = check_pd_active(chip);

	rc = power_supply_get_property(chip->batt_psy, POWER_SUPPLY_PROP_TEMP, &val);
	if (rc < 0) {
		pr_err("cycle_count is not ready fail rc = %d\n", rc);
				return rc;
	}
	batt_temp = val.intval;

	rc = power_supply_get_property(chip->batt_psy, POWER_SUPPLY_PROP_CAPACITY, &val);
	if (rc < 0) {
		pr_err("Couldn't read batt_soc fail rc = %d\n", rc);
				return rc;
	}

	chip->batt_soc = val.intval;
	if (chip->batt_soc >= 95) {
		pr_err("soc:%d is more than 95, do not set fastcharge mode\n", chip->batt_soc);
	}

	if (pd_active == QTI_POWER_SUPPLY_PD_ACTIVE || pd_active == QTI_POWER_SUPPLY_PD_PPS_ACTIVE) {
		if ((batt_temp >= BATT_WARM_THRESHOLD || batt_temp <= BATT_COOL_THRESHOLD)
					&& fastcharge_mode) {
			pr_err("batt_temp: %d disable fastcharge mode\n", batt_temp);
			val.intval = false;
			rc = step_chg_write_iio_prop(chip, MAIN, PSY_IIO_SMB_FASTCHARGE_MODE, val.intval);
			if (rc < 0) {
				pr_err("Set fastcharge mode failed, rc=%d\n", rc);
				return rc;
			}
			fastcharge_mode = false;
		} else if ((batt_temp < BATT_WARM_THRESHOLD - chip->jeita_fv_config->param.fall_hys)
					&& (batt_temp > BATT_COOL_THRESHOLD + chip->jeita_fv_config->param.rise_hys)
						&& !fastcharge_mode && (chip->batt_soc < 95)) {
			pr_err("batt_temp:%d enable fastcharge mode\n", batt_temp);
			val.intval = true;
			rc = step_chg_write_iio_prop(chip, MAIN, PSY_IIO_SMB_FASTCHARGE_MODE, val.intval);
			if (rc < 0) {
				pr_err("Set fastcharge mode failed, rc=%d\n", rc);
				return rc;
			}
		}
	} else {
		if (fastcharge_mode) {
			val.intval = false;
			rc = step_chg_write_iio_prop(chip, MAIN, PSY_IIO_SMB_FASTCHARGE_MODE, val.intval);
			if (rc < 0) {
				pr_err("Set fastcharge mode failed, rc=%d\n", rc);
				return rc;
			}
			fastcharge_mode = false;
			pr_err("disable fastcharge mode\n");
		}
	}

	if(chip->jeita_fv_index < 0){
		chip->jeita_fv_index = 0;
		pr_err("jeita_fv_index is not ready, reset jeita_fv_index to 0 \n");
	}

	if(fastcharge_mode){
		pr_debug("%s: config the cycle_count for FFC, batt_temp:%d, fv_index:%d \n", __func__, batt_temp, chip->jeita_fv_index);
		if(is_between(200, 450, batt_temp)){
			for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
				chip->jeita_fv_config->fv_cfg[i].low_threshold = cycle_count_ffc_ranges[i].low_threshold;
				chip->jeita_fv_config->fv_cfg[i].high_threshold = cycle_count_ffc_ranges[i].high_threshold;
				chip->jeita_fv_config->fv_cfg[i].value = cycle_count_ffc_ranges[i].value;
			}

			for(i = chip->jeita_fv_index; i < chip->jeita_fv_index + 2; i++){
				chip->step_chg_config->fcc_cfg[i].low_threshold = step_chg_ffc_ranges[i].low_threshold;
				chip->step_chg_config->fcc_cfg[i].high_threshold = step_chg_ffc_ranges[i].high_threshold;
				chip->step_chg_config->fcc_cfg[i].value = step_chg_ffc_ranges[i].value;
			}
		}
	}else{
		if(is_eu_mode){
			pr_debug("%s: config the cycle_count for EU, batt_temp:%d, fv_index:%d \n", __func__, batt_temp, chip->jeita_fv_index);
			if(is_between(200, 350, batt_temp)){
				for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
					chip->jeita_fv_config->fv_cfg[i].low_threshold = cycle_count_eu_range1[i].low_threshold;
					chip->jeita_fv_config->fv_cfg[i].high_threshold = cycle_count_eu_range1[i].high_threshold;
					chip->jeita_fv_config->fv_cfg[i].value = cycle_count_eu_range1[i].value;
				}

				for(i = chip->jeita_fv_index; i < chip->jeita_fv_index + 2; i++){
					chip->step_chg_config->fcc_cfg[i].low_threshold = step_chg_eu_range1[i].low_threshold;
					chip->step_chg_config->fcc_cfg[i].high_threshold = step_chg_eu_range1[i].high_threshold;
					chip->step_chg_config->fcc_cfg[i].value = step_chg_eu_range1[i].value;
				}
			}else if(is_between(350, 400, batt_temp)){
				for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
					chip->jeita_fv_config->fv_cfg[i].low_threshold = cycle_count_eu_range2[i].low_threshold;
					chip->jeita_fv_config->fv_cfg[i].high_threshold = cycle_count_eu_range2[i].high_threshold;
					chip->jeita_fv_config->fv_cfg[i].value = cycle_count_eu_range2[i].value;
				}

				for(i = chip->jeita_fv_index; i < chip->jeita_fv_index + 2; i++){
					chip->step_chg_config->fcc_cfg[i].low_threshold = step_chg_eu_range2[i].low_threshold;
					chip->step_chg_config->fcc_cfg[i].high_threshold = step_chg_eu_range2[i].high_threshold;
					chip->step_chg_config->fcc_cfg[i].value = step_chg_eu_range2[i].value;
				}
			}else if(is_between(400, 450, batt_temp)){
				for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
					chip->jeita_fv_config->fv_cfg[i].low_threshold = cycle_count_eu_range3[i].low_threshold;
					chip->jeita_fv_config->fv_cfg[i].high_threshold = cycle_count_eu_range3[i].high_threshold;
					chip->jeita_fv_config->fv_cfg[i].value = cycle_count_eu_range3[i].value;
				}

				for(i = chip->jeita_fv_index; i < chip->jeita_fv_index + 2; i++){
					chip->step_chg_config->fcc_cfg[i].low_threshold = step_chg_eu_range3[i].low_threshold;
					chip->step_chg_config->fcc_cfg[i].high_threshold = step_chg_eu_range3[i].high_threshold;
					chip->step_chg_config->fcc_cfg[i].value = step_chg_eu_range3[i].value;
				}
			}else if(is_between(-100, 200, batt_temp)){
				for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
					chip->jeita_fv_config->fv_cfg[i].low_threshold = cycle_count_ffc_range1[i].low_threshold;
					chip->jeita_fv_config->fv_cfg[i].high_threshold = cycle_count_ffc_range1[i].high_threshold;
					chip->jeita_fv_config->fv_cfg[i].value = cycle_count_ffc_range1[i].value;
				}
			}
		}else{
			pr_debug("%s: config the cycle_count for NORMAL, batt_temp:%d, fv_index:%d \n", __func__, batt_temp, chip->jeita_fv_index);
			if(is_between(200, 350, batt_temp)){
				for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
					chip->jeita_fv_config->fv_cfg[i].low_threshold = cycle_count_eu_range1[i].low_threshold;
					chip->jeita_fv_config->fv_cfg[i].high_threshold = cycle_count_eu_range1[i].high_threshold;
					chip->jeita_fv_config->fv_cfg[i].value = cycle_count_eu_range1[i].value;
				}

				for(i = chip->jeita_fv_index; i < chip->jeita_fv_index + 1; i++){
					chip->step_chg_config->fcc_cfg[i].low_threshold = step_chg_normal_range1[i].low_threshold;
					chip->step_chg_config->fcc_cfg[i].high_threshold = step_chg_normal_range1[i].high_threshold;
					chip->step_chg_config->fcc_cfg[i].value = step_chg_normal_range1[i].value;
				}
			}else if(is_between(350, 400, batt_temp)){
				for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
					chip->jeita_fv_config->fv_cfg[i].low_threshold = cycle_count_eu_range2[i].low_threshold;
					chip->jeita_fv_config->fv_cfg[i].high_threshold = cycle_count_eu_range2[i].high_threshold;
					chip->jeita_fv_config->fv_cfg[i].value = cycle_count_eu_range2[i].value;
				}

				for(i = chip->jeita_fv_index; i < chip->jeita_fv_index + 1; i++){
					chip->step_chg_config->fcc_cfg[i].low_threshold = step_chg_normal_range2[i].low_threshold;
					chip->step_chg_config->fcc_cfg[i].high_threshold = step_chg_normal_range2[i].high_threshold;
					chip->step_chg_config->fcc_cfg[i].value = step_chg_normal_range2[i].value;
				}
			}else if(is_between(400, 450, batt_temp)){
				for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
					chip->jeita_fv_config->fv_cfg[i].low_threshold = cycle_count_eu_range3[i].low_threshold;
					chip->jeita_fv_config->fv_cfg[i].high_threshold = cycle_count_eu_range3[i].high_threshold;
					chip->jeita_fv_config->fv_cfg[i].value = cycle_count_eu_range3[i].value;
				}

				for(i = chip->jeita_fv_index; i < chip->jeita_fv_index + 1; i++){
					chip->step_chg_config->fcc_cfg[i].low_threshold = step_chg_normal_range3[i].low_threshold;
					chip->step_chg_config->fcc_cfg[i].high_threshold = step_chg_normal_range3[i].high_threshold;
					chip->step_chg_config->fcc_cfg[i].value = step_chg_normal_range3[i].value;
				}
			}else if(is_between(-100, 200, batt_temp)){
				for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
					chip->jeita_fv_config->fv_cfg[i].low_threshold = cycle_count_ffc_range1[i].low_threshold;
					chip->jeita_fv_config->fv_cfg[i].high_threshold = cycle_count_ffc_range1[i].high_threshold;
					chip->jeita_fv_config->fv_cfg[i].value = cycle_count_ffc_range1[i].value;
				}
			}
		}
	}

	pr_debug("step1: cycle_count is %d, fv_index:%d, is_eu_mode:%d, fast_chg_mode:%d, pd_active:%d, batt_temp:%d \n",
							cycle_count,
							chip->jeita_fv_index,
							is_eu_mode,
							fastcharge_mode,
							pd_active,
							batt_temp);
	rc = get_val(chip->jeita_fv_config->fv_cfg,
			1,
			0,
			chip->jeita_fv_index,
			cycle_count,
			&chip->jeita_fv_index,
			&chip->targer_fv_uv);
	if (rc < 0){
		pr_err("parse the fv_comfig failed \n");
		chip->targer_fv_uv = JEITA_HIGH_TEMP_FV_UV;
	}

	if(!fastcharge_mode && !is_eu_mode){
		for (i = chip->jeita_fv_index; i < chip->jeita_fv_index + 1; i++)
			pr_debug("step-chg-cfg: %duV(SoC) ~ %duV(SoC), %duA\n",
				chip->step_chg_config->fcc_cfg[i].low_threshold,
				chip->step_chg_config->fcc_cfg[i].high_threshold,
				chip->step_chg_config->fcc_cfg[i].value);
	}else{
		for (i = chip->jeita_fv_index; i < chip->jeita_fv_index + 2; i++)
			pr_debug("step-chg-cfg: %duV(SoC) ~ %duV(SoC), %duA\n",
				chip->step_chg_config->fcc_cfg[i].low_threshold,
				chip->step_chg_config->fcc_cfg[i].high_threshold,
				chip->step_chg_config->fcc_cfg[i].value);
	}

	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++)
		pr_debug("jeita-fcc-cfg: %ddecidegree ~ %ddecidegre, %duA\n",
			chip->jeita_fcc_config->fcc_cfg[i].low_threshold,
			chip->jeita_fcc_config->fcc_cfg[i].high_threshold,
			chip->jeita_fcc_config->fcc_cfg[i].value);
	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++)
		pr_debug("jeita-fv-cfg: %d cycle ~ %d cycle, %duV\n",
			chip->jeita_fv_config->fv_cfg[i].low_threshold,
			chip->jeita_fv_config->fv_cfg[i].high_threshold,
			chip->jeita_fv_config->fv_cfg[i].value);

	//dynamics fv
	if (IS_ERR_OR_NULL(chip->fv_votable)) {
		pr_err("get FV voter fail, set fv to %duV\n", NORMAL_FV_MAX_VOLTAGE);
		chip->final_fv_uv = NORMAL_FV_MAX_VOLTAGE;
	} else {
		chip->final_fv_uv = get_effective_result_locked(chip->fv_votable);
		final_fv_client = get_effective_client_locked(chip->fv_votable);
	}

	chip->fg_vbat_uv = jeita_get_gauge_vbat(chip);
	gauge_ffc_mode = get_gauge_fastcharg_mode(chip);
	chip->cp_enable = check_cp_enable(chip);
	pr_info("final_fv:%d by %s, targer_fv_uv:%d, fg_vbat_uv:%d, gauge_ffc_mode:%d, cp_enable:%d, batt_temp:%d, plugin =%d, smart_batt:%d\n",
				chip->final_fv_uv,
				final_fv_client,
				chip->targer_fv_uv,
				chip->fg_vbat_uv,
				gauge_ffc_mode,
				chip->cp_enable,
				batt_temp,
				chip->charger_plugin_event,
				chip->smart_batt);

	if (batt_temp >= 450) {
		pr_info("temp too high set fv to %duV\n", JEITA_HIGH_TEMP_FV_UV);
		chip->final_fv_uv = JEITA_HIGH_TEMP_FV_UV;
		chip->targer_fv_uv = JEITA_HIGH_TEMP_TARGER_FV_UV;
	} else {
		if(chip->smart_batt > 0 && chip->smart_batt <= 40) {
			chip->targer_fv_uv = chip->targer_fv_uv - (chip->smart_batt * 1000); //uV
		}
		chip->final_fv_uv =  FFC_FV_MAX_VOLTAGE;
	}

	cis_status = jeita_get_gauge_cis(chip);
	if(cis_status >= 3){
		chip->targer_fv_uv = chip->targer_fv_uv - 20000; //uV
		pr_info("%s: cis alert! set fv to %duV\n", __func__, chip->targer_fv_uv);
	}

	if (chip->cp_enable && !gauge_ffc_mode) {
		pr_debug("gauge set fastcharger mode to enable\n");
		set_gauge_fastcharg_mode(chip, true);
	} else if (!chip->cp_enable && gauge_ffc_mode) {
		pr_debug("gauge set fastcharger mode to disable\n");
		set_gauge_fastcharg_mode(chip, false);
	}

	pr_debug("final_fv: %d by %s, targer_fv:%d\n", chip->final_fv_uv, final_fv_client, chip->targer_fv_uv);
	return 0;
}

__maybe_unused static int handle_step_chg_config(struct step_chg_info *chip)
{
	union power_supply_propval pval = {0, };
	int rc = 0, fcc_ua = 0, current_index;
	struct timespec64 ts;

	ktime_get_boottime_ts64(&ts);
	if((u64)ts.tv_sec < 60) {
		pr_err("tv_sec(%d) < 60\n", (u64)ts.tv_sec);
		return 0;
	}

	if (!chip->step_chg_enable || !chip->step_chg_cfg_valid) {
		if (chip->fcc_votable)
			vote(chip->fcc_votable, STEP_CHG_VOTER, false, 0);
		pr_err("step_chg_enable: %d, step_chg_cfg_valid:%d\n",
					chip->step_chg_enable, chip->step_chg_cfg_valid);
	}

	if (chip->step_chg_config->param.use_bms) {
		rc = step_chg_read_iio_prop(chip, QG,
			chip->step_chg_config->param.iio_prop, &pval.intval);
		if (rc < 0)
			pr_err("Failed to read IIO prop %d rc=%d\n",
				chip->step_chg_config->param.iio_prop, rc);
	} else {
		rc = power_supply_get_property(chip->batt_psy,
				chip->step_chg_config->param.psy_prop, &pval);
	}

	if (rc < 0) {
		pr_err("Couldn't read %s property rc=%d\n",
			chip->step_chg_config->param.prop_name, rc);
		return rc;
	}

	current_index = chip->step_index;
	rc = get_val(chip->step_chg_config->fcc_cfg,
			chip->step_chg_config->param.rise_hys,
			chip->step_chg_config->param.fall_hys,
			chip->step_index,
			pval.intval,
			&chip->step_index,
			&fcc_ua);
	if (rc < 0) {
		/* remove the vote if no step-based fcc is found */
		if (chip->fcc_votable)
			vote(chip->fcc_votable, STEP_CHG_VOTER, false, 0);
	}

	/* Do not drop step-chg index, if input supply is present */
	if (is_input_present(chip)) {
		if (chip->step_index < current_index)
			chip->step_index = current_index;
	} else {
		chip->step_index = 0;
	}

	if (!chip->fcc_votable) {
		chip->fcc_votable = find_votable("FCC");
	}
	if (!chip->fcc_votable) {
		pr_err("Couldn't find fcc votable\n");
		return -EINVAL;
	}

	if (chip->taper_fcc) {
		taper_fcc_step_chg(chip, chip->step_index, pval.intval);
	} else {
		fcc_ua = chip->step_chg_config->fcc_cfg[chip->step_index].value;
		vote(chip->fcc_votable, STEP_CHG_VOTER, true, fcc_ua);
	}

	pr_info("%s = %d Step-FCC = %duA taper-fcc: %d\n",
		chip->step_chg_config->param.prop_name, pval.intval,
		get_client_vote(chip->fcc_votable, STEP_CHG_VOTER),
		chip->taper_fcc);

	return 0;
}

static void handle_jeita_fcc_scaling(struct step_chg_info *chip)
{
	union power_supply_propval pval = {0, };
	int fcc_ua = 0, temp_diff = 0, rc;
	bool first_time_entry;

	if (chip->jeita_fcc_config->param.use_bms) {
		rc = step_chg_read_iio_prop(chip, QG,
			chip->jeita_fcc_config->param.iio_prop, &pval.intval);
		if (rc < 0)
			pr_err("Failed to read IIO prop %d rc=%d\n",
				chip->jeita_fcc_config->param.iio_prop, rc);
	} else {
		rc = power_supply_get_property(chip->batt_psy,
				chip->jeita_fcc_config->param.psy_prop, &pval);
	}

	if (rc < 0) {
		pr_err("Couldn't read %s property rc=%d\n",
				chip->jeita_fcc_config->param.prop_name, rc);
		return;
	}

	/* Skip mitigation if temp is not within min-max thresholds */
	if (!is_between(chip->jeita_fcc_scaling_temp_threshold[0],
		chip->jeita_fcc_scaling_temp_threshold[1], pval.intval)) {
		pr_debug("jeita-fcc-scaling : Skip jeita scaling, temp out of range temp = %d\n",
			pval.intval);
		chip->jeita_last_update_temp = pval.intval;
		vote(chip->fcc_votable, JEITA_FCC_SCALE_VOTER, false, 0);
		return;
	}

	/*
	 * We determine this is the first time entry to jeita-fcc-scaling if
	 * jeita_last_update_temp is not within entry/exist thresholds.
	 */
	first_time_entry = !is_between(chip->jeita_fcc_scaling_temp_threshold[0]
		, chip->jeita_fcc_scaling_temp_threshold[1],
		chip->jeita_last_update_temp);

	/*
	 * VOTE on FCC only when temp is within hys or if this the very first
	 * time we crossed the entry threshold.
	 */
	if (first_time_entry ||
		((pval.intval > (chip->jeita_last_update_temp +
			chip->jeita_fcc_config->param.rise_hys)) ||
		(pval.intval < (chip->jeita_last_update_temp -
			chip->jeita_fcc_config->param.fall_hys)))) {

		/*
		 * New FCC step is calculated as :
		 *	fcc_ua = (max-fcc - ((current_temp - min-temp) *
		 *			jeita-step-size))
		 */
		temp_diff = pval.intval -
				chip->jeita_fcc_scaling_temp_threshold[0];
		fcc_ua = div_s64((chip->jeita_max_fcc_ua -
			(chip->jeita_fcc_step_size * temp_diff)), 100) * 100;

		vote(chip->fcc_votable, JEITA_FCC_SCALE_VOTER, true, fcc_ua);
		pr_info("jeita-fcc-scaling: first_time_entry = %d, max_fcc_ua = %ld, voted_fcc_ua = %d, temp_diff = %d, prev_temp = %d, current_temp = %d\n",
			first_time_entry, chip->jeita_max_fcc_ua, fcc_ua,
			temp_diff, chip->jeita_last_update_temp, pval.intval);

		chip->jeita_last_update_temp = pval.intval;
	} else {
		pr_info("jeita-fcc-scaling: Skip jeita mitigation temp within first_time_entry = %d, hys temp = %d, last_updated_temp = %d\n",
			first_time_entry, pval.intval,
			chip->jeita_last_update_temp);
	}
}

static int config_jeita_fcc(struct step_chg_info *chip)
{
	int rc,i;
	bool fastcharge_mode, is_eu_mode;
	union power_supply_propval val = {0, };

	rc = step_chg_read_iio_prop(chip, MAIN, PSY_IIO_SMB_FASTCHARGE_MODE, &val.intval);
	if (rc < 0) {
		pr_err("Couldn't read fastcharge mode fail rc=%d\n", rc);
		return rc;
	}

	fastcharge_mode = !!val.intval;
	is_eu_mode = get_eu_mode();

	if(fastcharge_mode){
		pr_debug("%s: config the jeita for FFC \n", __func__);
		for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
			chip->jeita_fcc_config->fcc_cfg[i].low_threshold = jeita_ffc_ranges[i].low_threshold;
			chip->jeita_fcc_config->fcc_cfg[i].high_threshold = jeita_ffc_ranges[i].high_threshold;
			chip->jeita_fcc_config->fcc_cfg[i].value = jeita_ffc_ranges[i].value;
		}
	}else{
		if(is_eu_mode){
			pr_debug("%s: config the jeita for EU \n", __func__);
			for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
				chip->jeita_fcc_config->fcc_cfg[i].low_threshold = jeita_eu_ranges[i].low_threshold;
				chip->jeita_fcc_config->fcc_cfg[i].high_threshold = jeita_eu_ranges[i].high_threshold;
				chip->jeita_fcc_config->fcc_cfg[i].value = jeita_eu_ranges[i].value;
			}
		}else{
			pr_debug("%s: config the jeita for NORMAL \n", __func__);
			for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++){
				chip->jeita_fcc_config->fcc_cfg[i].low_threshold = jeita_normal_ranges[i].low_threshold;
				chip->jeita_fcc_config->fcc_cfg[i].high_threshold = jeita_normal_ranges[i].high_threshold;
				chip->jeita_fcc_config->fcc_cfg[i].value = jeita_normal_ranges[i].value;
			}
		}
	}

	pr_debug("%s: fastcharge_mode: %d, is_eu_mode: %d \n", __func__, fastcharge_mode, is_eu_mode);
	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++)
		pr_debug("%s: jeita-fcc-cfg: %d decidegree ~ %d decidegre, %d uA\n", __func__,
			chip->jeita_fcc_config->fcc_cfg[i].low_threshold,
			chip->jeita_fcc_config->fcc_cfg[i].high_threshold,
			chip->jeita_fcc_config->fcc_cfg[i].value);

	return 0;
}

#define DT_FCC_TEMP_15_20 4
#define DT_FCC_TEMP_20_35 5
#define DT_FCC_TEMP_35_45 6
static int config_chg_term_current(struct step_chg_info *chip, int fastcharge_mode, int batt_temp)
{
	struct bq_fg_chip *gm;
	struct power_supply *psy_fg = NULL;
	int rc, batt_id;
	int iterm = ERR_CHG_TERM_CURRENT;
	int final_iterm = -200; //mA
	union power_supply_propval pval = {0, };

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("%s: get fg psy failed\n", __func__);
		return iterm;
	}

	gm = power_supply_get_drvdata(psy_fg);
	if (IS_ERR_OR_NULL(gm)){
		pr_err("%s: get fg psy drv data failed\n", __func__);
		return iterm;
	}
	batt_id = gm->cell_supplier;

	if (fastcharge_mode) {
		if(chip->jeita_fcc_index == DT_FCC_TEMP_15_20){
			if (batt_id == 3){
				iterm = -550;
			} else {
				iterm = -480;
			}
		} else if(chip->jeita_fcc_index == DT_FCC_TEMP_20_35) {
			if(batt_id == 3) {// swd-lwn
				iterm = -1306;
			} else if(batt_id == 2) {// nvt-atl
				iterm = -1306;
			} else if(batt_id == 1) {// gy-cos
				iterm = -1100;
			} else {
				iterm = -1306;
				pr_info("batt_id is not allowed, set iterm to %d\n", iterm);
			}
		}else if (chip->jeita_fcc_index == DT_FCC_TEMP_35_45){
			if(batt_temp < 400){
				if(batt_id == 3) {// swd-lwn
					iterm = -1648;
				} else if(batt_id == 2) {// nvt-atl
					iterm = -2062;
				} else if(batt_id == 1) {// gy-cos
					iterm = -1648;
				}else {
					iterm = -2062;
					pr_info("batt_id is not allowed, set iterm to %d\n", iterm);
				}
			} else {
				if(batt_id == 3) {// swd-lwn
					iterm = -2060;
				} else if(batt_id == 2) {// nvt-atl
					iterm = -2680;
				} else if(batt_id == 1) {// gy-cos
					iterm = -2268;
				} else {
					iterm = -2680;
					pr_info("batt_id is not allowed, set iterm to %d\n", iterm);
				}
			}
		}
	} else if (batt_id == 3){
		iterm = -550;
	} else {
		iterm = -480;
	}

	if(IS_ERR_OR_NULL(chip->batt_psy)){
		chip->batt_psy = power_supply_get_by_name("battery");
		if(IS_ERR_OR_NULL(chip->batt_psy)){
			pr_err("%s: null pointer , return \n", __func__);
			return iterm;
		}
	}

	rc = power_supply_get_property(chip->batt_psy, POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT, &pval);
	if (rc < 0){
		pr_err("Couldn't get chg_term_current, rc=%d\n", rc);
	}

	pr_info("batt_id: %d, batt_temp: %d, fast_chg_mode: %d, target_term_current: %d, register_iterm: %d\n",
				batt_id, batt_temp, fastcharge_mode, iterm, pval.intval);

	if (final_iterm != pval.intval || rc < 0) {
		pval.intval = final_iterm;
		rc = power_supply_set_property(chip->batt_psy, POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT, &pval);
		if (rc < 0)
			pr_err("Couldn't set chg_term_current, rc=%d\n", rc);
	}

	return iterm;
}

static int handle_jeita(struct step_chg_info *chip)
{
	union power_supply_propval pval = {0, };
	union power_supply_propval val = {-22, };
	int rc = 0, data, ret, i, ret1 = 0;
	bool fastcharge_mode;
	int temp,chg_term_current = 0;
	struct timespec64 ts;
	int last_temp_range;
	int old_fv = 0;
	const char *final_fcc_client;
	bool is_eu_mode = false;
	int taper_max_current;

	if (!is_batt_available(chip) || !is_bms_available(chip))
	{
		pr_err("batt_psy or bms init failed\n");
		return -EINVAL;
	}

	rc = step_chg_read_iio_prop(chip, MAIN, PSY_IIO_SW_JEITA_ENABLED,
			&data);
	if (rc < 0) {
		chip->sw_jeita_enable = false;
		pr_err("Failed to read jeita_enabled rc=%d\n", rc);
	} else {
		chip->sw_jeita_enable = data;
	}

	/* Handle jeita-fcc-scaling if enabled */
	if (chip->jeita_fcc_scaling)
		handle_jeita_fcc_scaling(chip);

	if (!chip->sw_jeita_enable || !chip->sw_jeita_cfg_valid) {
		if (chip->fcc_votable) {
			vote(chip->fcc_votable, JEITA_VOTER, false, 0);
			vote(chip->fcc_votable, JEITA_LOW_TEMP_VOTER, false, 0);
			vote(chip->fcc_votable, JEITA_HIGH_TEMP_VOTER, false, 0);
                }
		if (chip->fv_votable)
			vote(chip->fv_votable, JEITA_VOTER, false, 0);
		if (chip->usb_icl_votable)
			vote(chip->usb_icl_votable, JEITA_VOTER, false, 0);
		pr_info("sw_jeita_enable: %d, sw_jeita_cfg_valid: %d, return\n", chip->sw_jeita_enable, chip->sw_jeita_cfg_valid);
		return 0;
	}

	if (chip->is_need_get_timer) {
		ktime_get_boottime_ts64(&ts);
		if((u64)ts.tv_sec < 60 && chip->high_soc_plugin == 0 && chip->fg_rsoc >= 90) {
			pr_debug("tv_sec: %d < 60\n", (u64)ts.tv_sec);
			chip->high_soc_plugin = 1;
			chip->is_need_get_timer = 0;
			vote(chip->fcc_votable, JEITA_FCC_TAPER_VOTER, true, FFC_HIGHT_SOC_LIMIT_CURRENT);
			//return 0;
		} else if ((u64)ts.tv_sec > 60) {
			chip->is_need_get_timer = 0;
		}
	}

	if (chip->jeita_fcc_config->param.use_bms) {
		rc = step_chg_read_iio_prop(chip, QG,
			chip->jeita_fcc_config->param.iio_prop, &pval.intval);
		if (rc < 0)
			pr_err("Failed to read IIO prop %d rc=%d\n",
				chip->jeita_fcc_config->param.iio_prop, rc);
	} else {
		rc = power_supply_get_property(chip->batt_psy,
				chip->jeita_fcc_config->param.psy_prop, &pval);
	}

	if (rc < 0) {
		pr_err("Couldn't read %s property rc=%d\n",
				chip->jeita_fcc_config->param.prop_name, rc);
		return rc;
	}
	temp = pval.intval;
	last_temp_range = chip->jeita_fcc_index;

	rc = config_jeita_fcc(chip);
	if (rc < 0){
		pr_err("Couldn't config the jeita, rc=%d\n",rc);
		chip->targer_fcc_ua = 0;
	}

	rc = get_val(chip->jeita_fcc_config->fcc_cfg,
			chip->jeita_fcc_config->param.rise_hys,
			chip->jeita_fcc_config->param.fall_hys,
			chip->jeita_fcc_index,
			temp,
			&chip->jeita_fcc_index,
			&chip->targer_fcc_ua);
	if (rc < 0) {
		pr_err("Fail get val ,set targer_fcc_ua: 0\n");
		chip->targer_fcc_ua = 0;
	}

	if (chip->fg_vbat_uv >= 4200000 && chip->fg_vbat_uv < 4530000 ) {
		for(i = 0; i < MAX_STEP_CHG_ENTRIES; i++) {
			if (is_between(0, 50, temp)) {
				chip->targer_fcc_ua = 1030500;
			} else if (is_between(50, 100, temp)) {
				chip->targer_fcc_ua = 1374000;
			} else if (is_between(100, 150, temp)) {
				chip->targer_fcc_ua = 3435000;
			}
			pr_debug("fg_vbat_uv = %d, targer_fcc_ua = %d\n", chip->fg_vbat_uv, chip->targer_fcc_ua);
		}
	}

	if (!chip->fcc_votable)
		chip->fcc_votable = find_votable("FCC");
	if (!chip->fcc_votable) {
		/* changing FCC is a must */
		pr_err("Couldn't find fcc votable, return\n");
		return -EINVAL;
	}

	pr_debug("targer_fcc_ua: %d\n", chip->targer_fcc_ua);
	vote(chip->fcc_votable, JEITA_VOTER, chip->targer_fcc_ua ? true : false, chip->targer_fcc_ua);

	if(chip->jeita_fcc_index == DT_FCC_TEMP_45_60){
		chip->final_fv_uv = JEITA_HIGH_TEMP_FV_UV;
		chip->targer_fv_uv = JEITA_HIGH_TEMP_FV_UV - (FFC_FV_FALL_HYS * 20);
		pr_info("%s: battery temp(%d) is too high, limit fv to %d uV \n", __func__, temp, chip->final_fv_uv);
	}

	chip->fv_votable = find_votable("FV");
	if (!chip->fv_votable) {
		pr_err("Couldn't find fv_votable\n");
		goto update_time;
	}

	if (!chip->usb_icl_votable) {
		chip->usb_icl_votable = find_votable("USB_ICL");
	}

	if (!chip->usb_icl_votable) {
		pr_err("Couldn't find usb_icl_votable\n");
		goto set_jeita_fv;
	}

	if (last_temp_range == -EINVAL && (temp <=
			chip->jeita_fcc_config->fcc_cfg[0].low_threshold + chip->jeita_fcc_config->param.rise_hys))
	{
		pr_info("Battery Temperature(%d) is not in vaild range(%d ~ %d), not allow charging yet!!\n",
			temp, chip->jeita_fcc_config->fcc_cfg[0].low_threshold,
			chip->jeita_fcc_config->fcc_cfg[0].low_threshold + chip->jeita_fcc_config->param.rise_hys);
		chip->jeita_fcc_index = last_temp_range;
		chip->jeita_fv_index = last_temp_range;
		vote(chip->fcc_votable, JEITA_VOTER, true, 0);
	}

	if (last_temp_range == 8 && (temp >=
			chip->jeita_fcc_config->fcc_cfg[7].high_threshold - chip->jeita_fcc_config->param.fall_hys))
	{
		pr_info("Battery Temperature(%d) between %d and %d,not allow charging yet!!\n",
			temp, chip->jeita_fcc_config->fcc_cfg[7].high_threshold,
			chip->jeita_fcc_config->fcc_cfg[7].high_threshold - chip->jeita_fcc_config->param.fall_hys);
		chip->jeita_fcc_index = last_temp_range;
		chip->jeita_fv_index = last_temp_range;
		vote(chip->fcc_votable, JEITA_VOTER, true, 0);
	}

	/* 低温充电控制逻辑 */
	if ((temp < -100)||(low_temp == 2 && is_between(-100, -80, temp)))
	{
		vote(chip->chg_disable_votable, JEITA_LOW_TEMP_VOTER, true, 0);
		pr_info("Critical low_temp = %d, temp = %d°C, charging stopped!\n", low_temp, temp);
		low_temp = 2;
	} else if ((temp < 0)||(low_temp == 1 && is_between(0, 20, temp))){
		vote(chip->chg_disable_votable, JEITA_LOW_TEMP_VOTER, false, 0);
		vote(chip->fcc_votable, JEITA_LOW_TEMP_VOTER, true, BATT_LOWTEMP_CURRENT);
		pr_info("Critical low_temp = %d, temp = %d°C, limit charging\n", low_temp, temp);
		low_temp = 1;
	}

	if(temp >= 560){
		vote(chip->chg_disable_votable, JEITA_HIGH_TEMP_VOTER, true, 0);
		high_temp = 1;
	} else if(is_between(450, 550, temp)){
		high_temp = 2;
	}

	if(temp >= 20 && temp <= 540){
		vote(chip->chg_disable_votable, JEITA_HIGH_TEMP_VOTER, false, 0);
		vote(chip->chg_disable_votable, JEITA_LOW_TEMP_VOTER, false, 0);
		vote(chip->fcc_votable, JEITA_LOW_TEMP_VOTER, false, 0);
		high_temp = 0;
		low_temp = 0;
	}

	if((high_temp == 2) && (temp <
			chip->jeita_fcc_config->fcc_cfg[6].high_threshold - chip->jeita_fcc_config->param.fall_hys)){

		pr_info("Battery Temperature(%d) has been between %d and %d, force recharge !!\n",
			temp, chip->jeita_fcc_config->fcc_cfg[6].high_threshold,
			chip->jeita_fcc_config->fcc_cfg[6].high_threshold - chip->jeita_fcc_config->param.fall_hys);
		rc = power_supply_set_property(chip->batt_psy, POWER_SUPPLY_PROP_STATUS, &val);
		if (rc < 0) {
			pr_err("Failed to set status gauge to full , rc=%d\n", rc);
		}
		pval.intval = 0;
		rc = step_chg_write_iio_prop(chip, MAIN, PSY_IIO_FORCE_RECHARGE, pval.intval);
		if (rc < 0) {
			pr_err("Set FORCE_RECHARGE failed, rc=%d\n", rc);
			return rc;
		}
		high_temp = 0;
	}

	rc = step_chg_read_iio_prop(chip, MAIN, PSY_IIO_SMB_FASTCHARGE_MODE, &pval.intval);
	if (rc < 0) {
		pr_err("Couldn't read fastcharge mode fail rc=%d\n", rc);
		return rc;
	}
	fastcharge_mode = !!pval.intval;

	chg_term_current = config_chg_term_current(chip, fastcharge_mode, temp);
	chg_term_current = (chg_term_current * (-1000)) + FFC_TAPER_ITERM_RISE_HYS; //uA
	if (jeita_get_gauge_iterm(chip) != chg_term_current) {
		ret = jeita_set_gauge_iterm(chip, chg_term_current);
		if (ret < 0) {
			pr_err("set iterm to fg fail\n");
		}
	}

	if (jeita_get_gauge_cv(chip) != chip->targer_fv_uv) {
		ret = jeita_set_gauge_cv(chip, chip->targer_fv_uv);
		if (ret < 0) {
			pr_err("set iterm to fg fail\n");
		}
	}

	//fcc taper charger
	chip->final_fcc_ua = get_effective_result_locked(chip->fcc_votable);
	final_fcc_client = get_effective_client_locked(chip->fcc_votable);
	if (chip->fg_vbat_uv > FFC_FV_LIMIT_VOLTAGE  && chip->final_fcc_ua > FFC_FCC_LIMIT_CURRENT) {
		vote(chip->fcc_votable, CYCLE_COUNT_VOTER, true , FFC_FCC_LIMIT_CURRENT);
	} else if (chip->fg_vbat_uv < (FFC_FV_LIMIT_VOLTAGE - FFC_FV_FALL_HYS)) {
		vote(chip->fcc_votable, CYCLE_COUNT_VOTER, false , 0);
	}

	chip->fg_ibat = jeita_get_gauge_ibat(chip);
	chip->fg_rsoc = jeita_get_gauge_rsoc(chip);
	is_eu_mode = get_eu_mode();
	chip->fg_temp = temp;
	pr_info("temp:%d, FCM:%d, fg_rsoc:%d, fg_vbat_uv(%d), targer_fcc_ua(%d):%d, fv_uv(%d):%d, iterm_uA:%d, ibat:%d, cp_enable:%d, final_fcc_ua:%d by %s, flag_fcc_taper:%d, is_eu_mode:%d\n",
				temp,
				fastcharge_mode,
				chip->fg_rsoc,
				chip->fg_vbat_uv,
				chip->jeita_fcc_index,
				chip->targer_fcc_ua,
				chip->final_fv_uv,
				chip->targer_fv_uv,
				chg_term_current,
				chip->fg_ibat,
				chip->cp_enable,
				chip->final_fcc_ua,
				final_fcc_client,
				chip->flag_fcc_taper,
				is_eu_mode);

	if (IS_ERR_OR_NULL(chip->chg_disable_votable)) {
		chip->chg_disable_votable = find_votable("CHG_DISABLE");
	}

	if (chip->fg_vbat_uv >= chip->targer_fv_uv && temp >= 450) {
		vote(chip->chg_disable_votable, HIGH_TEMP_OV_VOTER, true, 0);
	} else if (chip->fg_vbat_uv < chip->targer_fv_uv - FFC_FV_FALL_HYS) {
		vote(chip->chg_disable_votable, HIGH_TEMP_OV_VOTER, false, 0);
	}

	if (chip->charger_plugin_event == 2 || chip->cp_enable) {
		if (chip->flag_fcc_taper != 0) {
			pr_debug("taper fcc plugout cancel vote\n");
			jeita_step_set_charger_done(chip, false);
			vote(chip->fcc_votable, JEITA_FCC_TAPER_VOTER, false , 0);
			chip->flag_fcc_taper = 0;
			cancel_delayed_work_sync(&chip->fcc_taper_work);
		}
	} else if (!chip->cp_enable) {
		if (chip->high_soc_plugin == 1 && chip->flag_fcc_taper == 0) {
			taper_max_current = chip->batt_soc >= 95 ? HIGH_SOC_MAX_CURRENT : FFC_TAPER_MAX_CURRENT;
			if (chip->final_fcc_ua < taper_max_current) {
				chip->final_fcc_ua = chip->final_fcc_ua + FFC_TAPER_STEP1 * 5;
			} else {
				chip->high_soc_plugin = 0;
			}
			pr_info("jeita climb fcc: %d\n", chip->final_fcc_ua);
			vote(chip->fcc_votable, JEITA_FCC_TAPER_VOTER, true, chip->final_fcc_ua);
		}

		if ((chip->fg_vbat_uv > chip->targer_fv_uv - FFC_FV_FALL_HYS) && (chip->fg_ibat > chg_term_current)) {
			if (!chip->flag_fcc_taper) {
				chip->high_soc_plugin = 0;
				chip->flag_fcc_taper = 1;
				if (chip->final_fcc_ua > FFC_TAPER_MAX_CURRENT) {
					chip->final_fcc_ua = FFC_TAPER_MAX_CURRENT;
				}
				vote(chip->fcc_votable, JEITA_FCC_TAPER_VOTER, true , chip->final_fcc_ua);
				schedule_delayed_work(&chip->fcc_taper_work, msecs_to_jiffies(FFC_TAPER_DELAY_MS));
			} else {
				if(chg_term_current <= chip->final_fcc_ua - FFC_TAPER_STEP) {
					chip->final_fcc_ua = chip->final_fcc_ua - FFC_TAPER_STEP;
				} else {
					chip->final_fcc_ua = chip->final_fcc_ua - FFC_TAPER_STEP1;
				}

				if (chip->fg_vbat_uv >= chip->targer_fv_uv) {
					chip->flag_fcc_taper = 2;
					goto _set_done;
				} else if (chip->fg_vbat_uv >= chip->targer_fv_uv - FFC_TAPER_OV_FAIL_HYS) {
					pr_info("Jeita taper fcc protect: fg_vbat_uv(%d) >= targer_fv_uv(%d)\n", chip->fg_vbat_uv, chip->targer_fv_uv);
					chip->final_fcc_ua = chip->final_fcc_ua - FFC_BATT_OV_RISE_HYS;
					if (chip->final_fcc_ua <= chg_term_current) {
						pr_info("Jeita taper fcc protect, set fg to full\n");
						set_fg_full(chip);
						chip->final_fcc_ua = chg_term_current;
					}
				}

				if (chip->fg_ibat <= chg_term_current + FFC_FG_FULL_RISE_HYS &&
						chip->fg_rsoc != 100 &&
						chip->fg_vbat_uv > chip->targer_fv_uv - FFC_FV_FALL_HYS * 2) {
					pr_info("set fg to full befor charge full\n");
					set_fg_full(chip);
				}
				pr_info("Jeita fcc taper current: %d\n", chip->final_fcc_ua);
				vote(chip->fcc_votable, JEITA_FCC_TAPER_VOTER, true , chip->final_fcc_ua);
			}
		} else if (chip->flag_fcc_taper >= 3) {
			if (temp >= 450) {
				if (chip->fg_vbat_uv <= JEITA_HIGH_TEMP_RECHG_UV) {
					/*Jeita fcc recharge of hight temp*/
					rc = jeita_step_set_charger_done(chip, false);
					if (rc < 0) {
						pr_err("Set recharge fail, retry");
					} else {
						chip->flag_fcc_taper = 0;
						vote(chip->fcc_votable, JEITA_FCC_TAPER_VOTER, false , 0);
						vote(chip->chg_disable_votable, EEA_CHARGE_FULL, false, 0);
					}
				}
			} else if (!is_eu_mode && chip->fg_rsoc <= 97) {
				/*Jeita fcc recharge*/
				rc = jeita_step_set_charger_done(chip, false);
				if (rc < 0) {
					pr_err("Set recharge fail, retry");
				} else {
					chip->flag_fcc_taper = 0;
					vote(chip->fcc_votable, JEITA_FCC_TAPER_VOTER, false , 0);
					vote(chip->chg_disable_votable, EEA_CHARGE_FULL, false, 0);
				}
			} else if (is_eu_mode && chip->batt_soc < 95) {
				/*Jeita fcc recharge for erp, cancle dis_chg vote*/
				rc = vote(chip->chg_disable_votable, JEITA_FCC_TAPER_VOTER, false, 0);
				if (rc < 0) {
					pr_err("Erp cancle dis_chg vote fail, retry");
				} else {
					chip->flag_fcc_taper = 0;
					vote(chip->fcc_votable, JEITA_FCC_TAPER_VOTER, false , 0);
				}
			}
		} else if ((chip->fg_vbat_uv >= chip->targer_fv_uv - FFC_FV_FALL_HYS) &&
				(chip->fg_ibat >= 0 && chip->fg_ibat <= chg_term_current)) {
			if (chip->flag_fcc_taper < 2) {
				chip->flag_fcc_taper = chip->flag_fcc_taper + 1;
				/*Jeita fcc taper ready:*/
			} else if (chip->flag_fcc_taper == 2) {
_set_done:
				pr_info("charge done\n");
				cancel_delayed_work_sync(&chip->fcc_taper_work);
				rc = jeita_step_set_charger_done(chip, true);
				if (chip->fg_rsoc != 100) {
					ret1 = set_fg_full(chip);
				}
				if (rc < 0 || ret1 < 0) {
					pr_err("Set charge done fail, retry");
				} else {
					chip->flag_fcc_taper = chip->flag_fcc_taper + 1;
				}
			}
		} else {
			/* reset flag_fcc_taper */
			chip->flag_fcc_taper = 0;
		}
	}

	/*
	 * If JEITA float voltage is same as max-vfloat of battery then
	 * skip any further VBAT specific checks.
	 */

	rc = power_supply_get_property(chip->batt_psy,
				POWER_SUPPLY_PROP_VOLTAGE_MAX, &pval);
	if (rc || (pval.intval == chip->final_fv_uv)) {
		vote(chip->usb_icl_votable, JEITA_VOTER, false, 0);
		goto set_jeita_fv;
	}

	/*
	 * Suspend USB input path if battery voltage is above
	 * JEITA VFLOAT threshold.
	 */
	if (chip->jeita_arb_en && chip->final_fv_uv > 0) {
		rc = power_supply_get_property(chip->batt_psy,
				POWER_SUPPLY_PROP_VOLTAGE_NOW, &pval);
		if (!rc && (pval.intval > chip->final_fv_uv)) {
			pr_err("%s jeita set icl to 0, ret:%d, val:%d, final_fv_uv:%d\n", __func__, rc, pval.intval, chip->final_fv_uv);
			vote(chip->usb_icl_votable, JEITA_VOTER, true, 0);
		} else if (pval.intval < (chip->final_fv_uv - JEITA_SUSPEND_HYST_UV)) {
			pr_info("%s jeita disalbe vote,ret:%d, val:%d, final_fv_uv:%d\n", __func__, rc, pval.intval, chip->final_fv_uv);
			vote(chip->usb_icl_votable, JEITA_VOTER, false, 0);
		}
	}

set_jeita_fv:
	if (chip->fv_votable) {
		old_fv = get_client_vote(chip->fv_votable, BATT_PROFILE_VOTER);
		if(chip->final_fv_uv > old_fv){
			pr_debug("Should cancel fv vote first \n");
			vote(chip->fv_votable, JEITA_VOTER, false, 0);
			vote(chip->fv_votable, BATT_PROFILE_VOTER, false, 0);
		}
		vote(chip->fv_votable, JEITA_VOTER, chip->final_fv_uv ? true : false, chip->final_fv_uv);
	} else {
		pr_err("fv votable is null, don't set fv vote\n");
	}

update_time:
	return 0;
}

static int handle_battery_insertion(struct step_chg_info *chip)
{
	int rc;
	union power_supply_propval pval = {0, };

	rc = power_supply_get_property(chip->batt_psy,
			POWER_SUPPLY_PROP_PRESENT, &pval);
	if (rc < 0) {
		pr_err("Get battery present status failed, rc=%d\n", rc);
		return rc;
	}

	if (chip->batt_missing != (!pval.intval)) {
		chip->batt_missing = !pval.intval;
		pr_debug("battery %s detected\n",
				chip->batt_missing ? "removal" : "insertion");
		if (chip->batt_missing) {
			chip->step_chg_cfg_valid = false;
			chip->sw_jeita_cfg_valid = false;
			chip->get_config_retry_count = 0;
		} else {
			/*
			 * Get config for the new inserted battery, delay
			 * to make sure BMS has read out the batt_id.
			 */
			schedule_delayed_work(&chip->get_config_work,
				msecs_to_jiffies(WAIT_BATT_ID_READY_MS));
		}
	}

	return rc;
}

static void handle_irregular_battery(void)
{
	union power_supply_propval pval;
	struct bq_fg_chip *bq = NULL;
	struct power_supply *psy_fg = NULL;
	struct power_supply *psy_usb = NULL;

	psy_usb = power_supply_get_by_name("usb");
	if (IS_ERR_OR_NULL(psy_usb)) {
		pr_err("get usb psy failed\n");
		return;
	}

	if (power_supply_get_property(psy_usb, POWER_SUPPLY_PROP_ONLINE, &pval)) {
		pr_err("get usb psy online prop failed\n");
		return;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("get fg psy failed\n");
		return;
	}

	bq = power_supply_get_drvdata(psy_fg);
	if (IS_ERR_OR_NULL(bq)) {
		pr_err("get fg drv data failed\n");
		return;
	}

	if (pval.intval && bq->irregular_batt) {
		if (power_supply_get_property(psy_usb, POWER_SUPPLY_PROP_VOLTAGE_NOW, &pval)) {
			pr_err("get usb psy voltage prop failed\n");
			return;
		}
		pr_info("real_charger_type:%d, vbus:%dmV\n", g_chg->real_charger_type, pval.intval / 1000);
		if (g_chg->real_charger_type == QTI_POWER_SUPPLY_TYPE_USB_HVDCP && pval.intval > HVDCP_MIN_VOL_UV) {
			pr_info("force 5v hvdcp, irregular_batt:%d\n", bq->irregular_batt);
			smblib_force_vbus_voltage(g_chg, FORCE_5V_BIT);
		}
		if (bq->irregular_batt == IRREGULATOR_BATT1) {
			pr_info("non standard battery1, force ibus 2A\n");
			vote(g_chg->usb_icl_votable, NON_STD_BATT_VOTER, true, 2000000);
		} else if (bq->irregular_batt == IRREGULATOR_BATT2) {
			pr_info("non standard battery2, force ibus 2A\n");
			vote(g_chg->usb_icl_votable, NON_STD_BATT_VOTER, true, 2000000);
		} else if (bq->irregular_batt == IRREGULATOR_BATT3) {
			pr_info("non standard battery3, force ibus 500mA\n");
			vote(g_chg->usb_icl_votable, NON_STD_BATT_VOTER, true, 500000);
		}
	}
}

static void status_change_work(struct work_struct *work)
{
	struct step_chg_info *chip = container_of(work,
			struct step_chg_info, status_change_work.work);
	int rc = 0;
	union power_supply_propval prop = {0, };

	handle_irregular_battery();
	if (!is_batt_available(chip) || !is_bms_available(chip))
		goto exit_work;

	handle_battery_insertion(chip);

	rc = config_step_chg_cycle_count(chip);
	if (rc < 0)
		pr_err("Couldn't config step cycle count rc = %d\n", rc);

	/* skip elapsed_us debounce for handling battery temperature */
	rc = handle_jeita(chip);
	if (rc < 0)
		pr_err("Couldn't handle sw jeita rc = %d\n", rc);

	/* Remove stale votes on USB removal */
	if (is_usb_available(chip)) {
		prop.intval = 0;
		power_supply_get_property(chip->usb_psy,
				POWER_SUPPLY_PROP_PRESENT, &prop);
		if (!prop.intval) {
			if (chip->usb_icl_votable)
				vote(chip->usb_icl_votable, JEITA_VOTER,
						false, 0);
		}
	}

	schedule_delayed_work(&chip->status_change_work, msecs_to_jiffies(JEITA_WORK_MS));
exit_work:
	__pm_relax(chip->step_chg_ws);
}

static void fcc_taper_work(struct work_struct *work)
{
	struct step_chg_info *chip = container_of(work,
			struct step_chg_info, fcc_taper_work.work);

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("Chip point maybe null\n");
		return;
	}

	if (chip->charger_plugin_event == 1) {
		pr_err("schedule fcc tapaer work\n");
		schedule_delayed_work(&chip->status_change_work, 0);
		schedule_delayed_work(&chip->fcc_taper_work, msecs_to_jiffies(FFC_TAPER_DELAY_MS));
	} else {
		pr_info("Event needn't handle\n");
	}
	return;
}

static int step_chg_notifier_call(struct notifier_block *nb,
		unsigned long ev, void *v)
{
	struct power_supply *psy = v;
	struct step_chg_info *chip = container_of(nb, struct step_chg_info, nb);

	if (ev != PSY_EVENT_PROP_CHANGED)
		return NOTIFY_OK;

	if ((strcmp(psy->desc->name, "battery") == 0)
			|| (strcmp(psy->desc->name, "usb") == 0)) {
		__pm_stay_awake(chip->step_chg_ws);
		handle_jeita(chip);
		schedule_delayed_work(&chip->status_change_work, 0);
	}

	if ((strcmp(psy->desc->name, "bms") == 0)) {
		if (!chip->config_is_read)
			schedule_delayed_work(&chip->get_config_work, 0);
	}

	return NOTIFY_OK;
}

static int step_chg_register_notifier(struct step_chg_info *chip)
{
	int rc;

	chip->nb.notifier_call = step_chg_notifier_call;
	rc = power_supply_reg_notifier(&chip->nb);
	if (rc < 0) {
		pr_err("Couldn't register psy notifier rc = %d\n", rc);
		return rc;
	}

	return 0;
}
int jeita_set_eea_chg_dis_vote(struct step_chg_info *chip, bool enable)
{
	int ret = -EINVAL;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("Maybe chip is err or null\n");
		return ret;
	}

	if (IS_ERR_OR_NULL(chip->chg_disable_votable)) {
		chip->chg_disable_votable = find_votable("CHG_DISABLE");
	}

	if (IS_ERR_OR_NULL(chip->chg_disable_votable)) {
		pr_err("Couldn't find chg_disable_votable\n");
		return ret;
	}

	ret = vote(chip->chg_disable_votable, EEA_CHARGE_FULL, enable, 0);
	if (ret < 0) {
		pr_err("Failed to set JEITA_FCC_TAPER_VOTER , ret=%d\n", ret);
		return ret;
	}

	pr_info("enable: %d, ret: %d\n", enable, ret);
	return ret;
}

static int updata_awake_voteable(struct step_chg_info *chip)
{
	int ret = 1;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("chip maybe is err or null\n");
		return -EINVAL;
	}

	if (IS_ERR_OR_NULL(chip->awake_votable)) {
		chip->awake_votable = find_votable("AWAKE");
	}

	if (IS_ERR_OR_NULL(chip->awake_votable)) {
		pr_err("find awake_votable fail\n");
		return -EINVAL;
	}

	return ret;
}

static int qpnp_smb5_notifier_event_callback(struct notifier_block *qpnp_smb5_nb,
			unsigned long chg_event, void *val)
{
	struct step_chg_info *chip = container_of(qpnp_smb5_nb, struct step_chg_info, qpnp_smb5_nb);
	int ret = 0;

	if (IS_ERR_OR_NULL(chip)) {
		pr_err("Point maybe null\n");
		return -EINVAL;
	}

	switch (chg_event) {
		/*charger_plugin_event 0:none 1:plugin 2:plugout*/
		case QPNP_SMB5_USB_PLUGIN_EVENT:
			chip->charger_plugin_event = *(int *)val;
			pr_info("Jeita get charger_plugin_event: %d\n", chip->charger_plugin_event);
			if (chip->charger_plugin_event == 2) {
				pr_info("Jeita reflash status\n");
				jeita_step_set_charger_done(chip, false);
				jeita_set_eea_chg_dis_vote(chip, false);
				vote(chip->fcc_votable, JEITA_FCC_TAPER_VOTER, false , 0);
				chip->flag_fcc_taper = 0;
				chip->high_soc_plugin = 0;
				cancel_delayed_work_sync(&chip->fcc_taper_work);
				ret = updata_awake_voteable(chip);
				if (ret > 0) {
					vote(chip->awake_votable, JEITA_AWAKE_VOTER, false, 0);
				}
			} else if (chip->charger_plugin_event == 1) {
				ret = updata_awake_voteable(chip);
				if (ret > 0) {
					vote(chip->awake_votable, JEITA_AWAKE_VOTER, true, 0);
				}
				chip->fg_rsoc = jeita_get_gauge_rsoc(chip);
				if (chip->fg_rsoc >= 90) {
					chip->high_soc_plugin = 1;
					vote(chip->fcc_votable, JEITA_FCC_TAPER_VOTER, true, FFC_HIGHT_SOC_LIMIT_CURRENT);
				}
			}
			break;
		case QPNP_SMB5_SMART_BATT_EVENT:
			chip->smart_batt = *(int *)val;
			pr_info("Jeita get smart_batt_event: %d\n", chip->smart_batt);
			break;
		default:
			pr_debug("Jeita not supported charger notifier event: %d\n", chg_event);
		break;
    }

	return NOTIFY_DONE;
}


static int plugin_register_notifier(struct step_chg_info *chip)
{
	int rc;

	chip->qpnp_smb5_nb.notifier_call = qpnp_smb5_notifier_event_callback;
	rc = qpnp_smb5_reg_notifier(&chip->qpnp_smb5_nb);
	if (rc < 0) {
		pr_err("Couldn't register plug notifier rc = %d\n", rc);
		return rc;
	}

	return 0;
}

int qcom_step_chg_init(struct device *dev, bool step_chg_enable,
	bool sw_jeita_enable, bool jeita_arb_en, struct iio_channel *iio_chans)
{
	int rc;
	struct step_chg_info *chip;

	if (the_chip) {
		pr_err("Already initialized\n");
		return -EINVAL;
	}

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->step_chg_ws = wakeup_source_register(dev, "qcom-step-chg");
	if (!chip->step_chg_ws)
		return -EINVAL;

	chip->dev = dev;
	chip->step_chg_enable = step_chg_enable;
	chip->sw_jeita_enable = sw_jeita_enable;
	chip->jeita_arb_en = jeita_arb_en;
	chip->step_index = -EINVAL;
	chip->jeita_fcc_index = -EINVAL;
	chip->jeita_fv_index = -EINVAL;
	chip->iio_chans = iio_chans;
	chip->iio_chan_list_qg = NULL;

	chip->step_chg_config = devm_kzalloc(dev,
			sizeof(struct step_chg_cfg), GFP_KERNEL);
	if (!chip->step_chg_config)
		return -ENOMEM;

	chip->step_chg_config->param.psy_prop = POWER_SUPPLY_PROP_VOLTAGE_NOW;
	chip->step_chg_config->param.iio_prop = STEP_QG_VOLTAGE_NOW;
	chip->step_chg_config->param.prop_name = "VBATT";
	chip->step_chg_config->param.rise_hys = 100000;
	chip->step_chg_config->param.fall_hys = 100000;

	chip->jeita_fcc_config = devm_kzalloc(dev,
			sizeof(struct jeita_fcc_cfg), GFP_KERNEL);
	chip->jeita_fv_config = devm_kzalloc(dev,
			sizeof(struct jeita_fv_cfg), GFP_KERNEL);
	if (!chip->jeita_fcc_config || !chip->jeita_fv_config)
		return -ENOMEM;

	chip->jeita_fcc_config->param.psy_prop = POWER_SUPPLY_PROP_TEMP;
	chip->jeita_fcc_config->param.iio_prop = STEP_QG_TEMP;
	chip->jeita_fcc_config->param.prop_name = "BATT_TEMP";
	chip->jeita_fcc_config->param.rise_hys = 10;
	chip->jeita_fcc_config->param.fall_hys = 20;
	chip->jeita_fv_config->param.psy_prop = POWER_SUPPLY_PROP_TEMP;
	chip->jeita_fv_config->param.iio_prop = STEP_QG_TEMP;
	chip->jeita_fv_config->param.prop_name = "BATT_TEMP";
	chip->jeita_fv_config->param.rise_hys = 10;
	chip->jeita_fv_config->param.fall_hys = 20;

	chip->targer_fv_uv = FFC_FV_MAX_VOLTAGE;
	chip->final_fv_uv = FFC_FV_MAX_VOLTAGE;
	chip->final_fcc_ua = FFC_TAPER_MAX_CURRENT;
	chip->targer_fcc_ua = 0;
	chip->charger_plugin_event = 0;
	chip->flag_fcc_taper = 0;
	chip->fv_drop_time = 0;
	chip->cp_enable = 0;
	chip->fg_ibat = 0;
	chip->smart_batt = 0;
	chip->batt_soc = 50;
	chip->fg_rsoc = 50;
	chip->fg_temp = 250;
	chip->is_need_get_timer = 1;

	INIT_DELAYED_WORK(&chip->status_change_work, status_change_work);
	INIT_DELAYED_WORK(&chip->get_config_work, get_config_work);
	INIT_DELAYED_WORK(&chip->fcc_taper_work, fcc_taper_work);

	rc = step_chg_register_notifier(chip);
	if (rc < 0) {
		pr_err("Couldn't register psy notifier rc = %d\n", rc);
		goto release_wakeup_source;
	}

	rc = plugin_register_notifier(chip);
	if (rc < 0) {
		pr_err("Couldn't register psy notifier rc = %d\n", rc);
	}

	schedule_delayed_work(&chip->get_config_work, msecs_to_jiffies(GET_CONFIG_DELAY_MS));

	the_chip = chip;

	return 0;

release_wakeup_source:
	wakeup_source_unregister(chip->step_chg_ws);
	return rc;
}

void qcom_step_chg_deinit(void)
{
	struct step_chg_info *chip = the_chip;

	if (!chip)
		return;

	cancel_delayed_work_sync(&chip->status_change_work);
	cancel_delayed_work_sync(&chip->get_config_work);
	cancel_delayed_work_sync(&chip->fcc_taper_work);
	power_supply_unreg_notifier(&chip->nb);
	qpnp_smb5_unreg_notifier(&chip->qpnp_smb5_nb);
	wakeup_source_unregister(chip->step_chg_ws);
	the_chip = NULL;
}
