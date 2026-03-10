#define pr_fmt(fmt)	"[usbpd-pm]: %s: " fmt, __func__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/usb/usbpd.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/iio/iio.h>
#include <linux/iio/consumer.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>
#include <linux/pmic-voter.h>
#include <linux/qti_power_supply.h>
#include "bq28z610.h"
#include "pd_policy_manager.h"

enum {
	PM_ALGO_RET_OK,
	PM_ALGO_RET_CHG_DISABLED,
#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
	PM_ALGO_SMART_CHG_ENABLED,
	PM_ALGO_SMART_CHG_DISABLED,
#endif
	PM_ALGO_RET_TAPER_DONE,
};

static struct pdpm_config pm_config = {
	.bat_volt_lp_lmt = BAT_VOLT_LOOP_LMT,
	.bat_curr_lp_lmt = BAT_CURR_LOOP_LMT,
	.fc2_taper_current = 2300,
	.fc2_steps = 1,
	.min_adapter_volt_required = 10000,
	.min_adapter_curr_required = 2000,
	.min_vbat_for_cp = 3000,
};

static struct usbpd_pm *g_pdpm = NULL;
static int fc2_taper_timer;
static int ibus_lmt_change_timer;

static int fast_work_time = PM_WORK_RUN_FAST;
module_param_named(fast_work_time, fast_work_time, int, 0600);

static int stable_work_time = PM_WORK_RUN_STABLE;
module_param_named(stable_work_time, stable_work_time, int, 0600);

static int fast_tune_step = 5;
module_param_named(fast_tune_step, fast_tune_step, int, 0600);

static int stable_tune_step = 1;
module_param_named(stable_tune_step, stable_tune_step, int, 0600);

enum cp_iio_type {
	CP_MASTER,
	SMB,
};

#define BQ25960_IIO_CHANNEL_OFFSET      6

enum cp_iio_channels {
	CHARGE_PUMP_SC_PRESENT,
	CHARGE_PUMP_SC_CHARGING_ENABLED,
	CHARGE_PUMP_SC_BUS_VOLTAGE,
	CHARGE_PUMP_SC_BUS_CURRENT,
	CHARGE_PUMP_SC_ADC_CONTROL,
	CHARGE_PUMP_SC_STATUS,
	CHARGE_PUMP_BQ_PRESENT = BQ25960_IIO_CHANNEL_OFFSET,
	CHARGE_PUMP_BQ_CHARGING_ENABLED,
	CHARGE_PUMP_BQ_BUS_VOLTAGE,
	CHARGE_PUMP_BQ_BUS_CURRENT,
	CHARGE_PUMP_BQ_ADC_CONTROL,
	CHARGE_PUMP_BQ_STATUS,
};

static const char * const cp_iio_chan_name[] = {
	[CHARGE_PUMP_SC_PRESENT] = "sc_present",
	[CHARGE_PUMP_SC_CHARGING_ENABLED] = "sc_charging_enabled",
	[CHARGE_PUMP_SC_BUS_VOLTAGE] = "sc_bus_voltage",
	[CHARGE_PUMP_SC_BUS_CURRENT] = "sc_bus_current",
	[CHARGE_PUMP_SC_ADC_CONTROL] = "sc_adc_control",
	[CHARGE_PUMP_SC_STATUS] = "sc_status",
	[CHARGE_PUMP_BQ_PRESENT] = "bq_present",
	[CHARGE_PUMP_BQ_CHARGING_ENABLED] = "bq_charging_enabled",
	[CHARGE_PUMP_BQ_BUS_VOLTAGE] = "bq_bus_voltage",
	[CHARGE_PUMP_BQ_BUS_CURRENT] = "bq_bus_current",
	[CHARGE_PUMP_BQ_ADC_CONTROL] = "bq_adc_control",
	[CHARGE_PUMP_BQ_STATUS] = "bq_status",
};

enum smb_iio_channels {
	SMB_PD_ACTIVE,
	SMB_INPUT_SUSPEND,
	SMB_SW_CHARGING_ENABLED,
	SMB_APDO_VOLT,
	SMB_APDO_CURR,
};

static const char * const smb_iio_chan_name[] = {
	[SMB_PD_ACTIVE] = "pd_active",
	[SMB_INPUT_SUSPEND] = "input_suspend",
	[SMB_SW_CHARGING_ENABLED] = "sw_charging_enabled",
	[SMB_APDO_VOLT] = "apdo_volt",
	[SMB_APDO_CURR] = "apdo_curr",
};

static bool is_cp_chan_valid(struct usbpd_pm *chip, enum cp_iio_channels chan)
{
	int rc;

	if (IS_ERR(chip->cp_iio[chan]))
		return false;

	if (!chip->cp_iio[chan]) {
		chip->cp_iio[chan] = iio_channel_get(chip->dev, cp_iio_chan_name[chan]);
		if (IS_ERR(chip->cp_iio[chan])) {
			rc = PTR_ERR(chip->cp_iio[chan]);
			chip->cp_iio[chan] = NULL;
			pr_err("Failed to get IIO channel %s, rc=%d\n", cp_iio_chan_name[chan], rc);
			return false;
		}
	}

	return true;
}

static bool is_smb_chan_valid(struct usbpd_pm *chip, enum smb_iio_channels chan)
{
	int rc;

	if (IS_ERR(chip->smb_iio[chan]))
		return false;

	if (!chip->smb_iio[chan]) {
		chip->smb_iio[chan] = iio_channel_get(chip->dev, smb_iio_chan_name[chan]);
		if (IS_ERR(chip->smb_iio[chan])) {
			rc = PTR_ERR(chip->smb_iio[chan]);
			chip->smb_iio[chan] = NULL;
			pr_err("Failed to get IIO channel %s, rc=%d\n", smb_iio_chan_name[chan], rc);
			return false;
		}
	}

	return true;
}

static int usbpd_get_iio_channel(struct usbpd_pm *chg, enum cp_iio_type type, int channel, int *val)
{
	struct iio_channel *iio_chan_list;
	int rc;

	switch (type) {
	case CP_MASTER:
		if (!chg->bq25960_flag) {
			if (!is_cp_chan_valid(chg, channel))
				return -ENODEV;
			iio_chan_list = chg->cp_iio[channel];
		} else {
			if (!is_cp_chan_valid(chg, channel + BQ25960_IIO_CHANNEL_OFFSET))
				return -ENODEV;
			iio_chan_list = chg->cp_iio[channel + BQ25960_IIO_CHANNEL_OFFSET];
		}
		break;
	case SMB:
		if (!is_smb_chan_valid(chg, channel))
			return -ENODEV;
		iio_chan_list = chg->smb_iio[channel];
		break;
	default:
		pr_err_ratelimited("iio_type %d is not supported\n", type);
		return -EINVAL;
	}

	rc = iio_read_channel_processed(iio_chan_list, val);

	return rc < 0 ? rc : 0;
}

static int usbpd_set_iio_channel(struct usbpd_pm *chg, enum cp_iio_type type, int channel, int val)
{
	struct iio_channel *iio_chan_list;
	int rc;

	switch (type) {
	case CP_MASTER:
		if (!chg->bq25960_flag) {
			if (!is_cp_chan_valid(chg, channel))
				return -ENODEV;
			iio_chan_list = chg->cp_iio[channel];
		} else {
			if (!is_cp_chan_valid(chg, channel + BQ25960_IIO_CHANNEL_OFFSET))
				return -ENODEV;
			iio_chan_list = chg->cp_iio[channel + BQ25960_IIO_CHANNEL_OFFSET];
		}
		break;
	case SMB:
		if (!is_smb_chan_valid(chg, channel))
			return -ENODEV;
		iio_chan_list = chg->smb_iio[channel];
		break;
	default:
		pr_err_ratelimited("iio_type %d is not supported\n", type);
		return -EINVAL;
	}

	rc = iio_write_channel_raw(iio_chan_list, val);

	return rc < 0 ? rc : 0;
}

static void usbpd_check_usb_psy(struct usbpd_pm *pdpm)
{
	if (!pdpm->usb_psy) {
		pdpm->usb_psy = power_supply_get_by_name("usb");
		if (!pdpm->usb_psy)
			pr_err("usb psy not found!\n");
	}
}

static int usbpd_check_batt_psy(struct usbpd_pm *pdpm)
{
	if (!pdpm->batt_psy) {
		pdpm->batt_psy = power_supply_get_by_name("battery");
		if (!pdpm->batt_psy) {
			pr_err("batt psy not found!\n");
			return -ENODEV;
		}
	}

    return 0;
}

static int pd_get_input_suspend(struct usbpd_pm *pdpm, int *input_suspend)
{
	union power_supply_propval pval = {0,};
	int rc;

	rc = usbpd_get_iio_channel(pdpm, SMB, SMB_INPUT_SUSPEND, &pval.intval);
	if (!rc)
		*input_suspend = pval.intval;

	return rc;
}

static bool is_cool_charge(struct usbpd_pm *pdpm)
{
	if (pdpm->batt_temp < (COOL_HYS_THRESHOLDS - 30))
		return true;

	return false;
}

static bool is_warm_charge(struct usbpd_pm *pdpm)
{
	if (pdpm->batt_temp > (WARM_HYS_THRESHOLDS + 30))
		return true;

	return false;
}

static int usbpd_get_effective_fcc_val(struct usbpd_pm *pdpm)
{
	int effective_fcc_val = 0;

	if (!pdpm->fcc_votable)
		pdpm->fcc_votable = find_votable("FCC");

	if (!pdpm->fcc_votable)
		return -EINVAL;

	effective_fcc_val = get_effective_result(pdpm->fcc_votable);
	effective_fcc_val = effective_fcc_val / 1000;

	return effective_fcc_val;

}

static int pd_get_gauge_cv(struct usbpd_pm *pdpm)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = -EINVAL;

	if (IS_ERR_OR_NULL(pdpm)) {
		pr_err("pdpm is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_VOLTAGE_MAX, &val);
	if (rc < 0) {
		pr_err("Get gauge cv failed, rc: %d\n", rc);
		return rc;
	} else {
		val.intval = val.intval / 1000; //uV to mV
	}

	return val.intval;
}

static int pd_get_fg_vbat(struct usbpd_pm *pdpm)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = -EINVAL;

	if (IS_ERR_OR_NULL(pdpm)) {
		pr_err("pdpm is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
	if (rc < 0) {
		pr_err("Get gauge vbat failed, rc: %d\n", rc);
		return rc;
	} else {
		val.intval = val.intval / 1000; //uV to mV
	}

	return val.intval;
}

static int pd_get_fg_rsoc(struct usbpd_pm *pdpm)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = -EINVAL;

	if (IS_ERR_OR_NULL(pdpm)) {
		pr_err("pdpm is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("bq28z610 is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_CAPACITY_ALERT_MAX, &val);
	if (rc < 0) {
		pr_err("Get gauge rsoc failed, rc: %d\n", rc);
		return rc;
	}

	return val.intval;
}

static int usbpd_select_pdo_5v(void)
{
	int ret = -EINVAL;

	if (!g_pdpm->pd)
		g_pdpm->pd = usbpd_get_pd_lobal();
	if (IS_ERR_OR_NULL(g_pdpm) || IS_ERR_OR_NULL(g_pdpm->pd)) {
		pr_err("g_pdpm maybe is err or null\n");
		return ret;
	}

	ret = usbpd_select_pdo(g_pdpm->pd, 1, 0, 0);
	if (ret < 0) {
		pr_err("pdo set 5V fail, ret: %d\n", ret);
		return ret;
	}

	pr_debug("successful\n");
	return ret;
}

static bool pd_disable_cp_by_jeita_status(struct usbpd_pm *pdpm)
{
	int rc;
	int input_suspend = 0;

	rc = usbpd_check_batt_psy(pdpm);
	if (rc < 0) {
		return rc;
	}

	rc = pd_get_input_suspend(pdpm, &input_suspend);
	if (rc < 0) {
		pr_err("Failed to get input suspend, rc = %d\n", rc);
		return false;
	}

	if (input_suspend) {
		return true;
	} else {
		if (pdpm->batt_temp >= JEITA_WARM_THR && !pdpm->jeita_triggered) {
			pr_info("jeita upper limit reached,batt_temp:%d\n", pdpm->batt_temp);
			pdpm->jeita_triggered = true;
			return true;
		} else if (pdpm->batt_temp <= JEITA_COOL_NOT_ALLOW_CP_THR && !pdpm->jeita_triggered) {
			pr_info("jeita lower limit reached,batt_temp:%d\n", pdpm->batt_temp);
			pdpm->jeita_triggered = true;
			return true;
		} else if (pdpm->cool_warm_done) {
			if (((pdpm->vbat < (pm_config.bat_volt_lp_lmt - 100)) && (pdpm->batt_temp <= COOL_HYS_THRESHOLDS)) ||
			((pdpm->vbat < (BATT_WARM_CHG_VOLT - 100)) && (pdpm->batt_temp >= WARM_HYS_THRESHOLDS)) ||
			((pdpm->batt_temp < WARM_HYS_THRESHOLDS) && (pdpm->batt_temp > COOL_HYS_THRESHOLDS))) {
				pr_info("after cool and warm state recharge,batt_temp:%d,vbat_volt=%d,bat_volt_lp_lmt=%d,cool_warm_done=%d\n",
						pdpm->batt_temp, pdpm->vbat,pm_config.bat_volt_lp_lmt,pdpm->cool_warm_done);
				pdpm->cool_warm_done = false;
				return false;
			} else {
				return true;
			}
		} else if ((pdpm->batt_temp <= (JEITA_WARM_THR - JEITA_HYSTERESIS))
				&& (pdpm->batt_temp >= (JEITA_COOL_NOT_ALLOW_CP_THR + JEITA_HYSTERESIS))
				&& pdpm->jeita_triggered) {
			pr_info("jeita returned to normal, batt_temp:%d\n", pdpm->batt_temp);
			pdpm->jeita_triggered = false;
			return false;
		} else {
			return pdpm->jeita_triggered;
		}
	}
}

static int usbpd_pm_enable_cp(struct usbpd_pm *pdpm, bool enable)
{
	int ret;
	union power_supply_propval val = {0,};

	val.intval = enable;
	ret = usbpd_set_iio_channel(pdpm, CP_MASTER, CHARGE_PUMP_SC_CHARGING_ENABLED, val.intval);
	if (ret < 0)
		pr_err("enable cp fail\n");

	pr_info("enable cp:%d\n", enable);

	return ret;
}

static int usbpd_pm_cp_adc_control(struct usbpd_pm *pdpm, bool enable)
{
	int ret;
	union power_supply_propval val = {0,};

	val.intval = enable;
	ret = usbpd_set_iio_channel(pdpm, CP_MASTER, CHARGE_PUMP_SC_ADC_CONTROL, val.intval);
	if (ret < 0)
		pr_err("adc control fail\n");

	pr_info("adc control:%d\n", enable);

	return ret;
}

static int usbpd_pm_check_cp_enabled(struct usbpd_pm *pdpm)
{
	int ret;
	union power_supply_propval val = {0,};

	ret = usbpd_get_iio_channel(pdpm, CP_MASTER, CHARGE_PUMP_SC_CHARGING_ENABLED, &val.intval);
	if (!ret)
		pdpm->cp_charging = !!val.intval;

	pr_info("cp charging is %d, ret=%d\n",val.intval, ret);

	return ret;
}

static int usbpd_pm_enable_sw(struct usbpd_pm *pdpm, bool en)
{
	union power_supply_propval val = {0,};
	int ret;

	val.intval = en;
	ret = usbpd_set_iio_channel(pdpm, SMB, SMB_SW_CHARGING_ENABLED, val.intval);
	if (!ret) {
		pdpm->sw_charging = en;
	}

	pr_info("set battey charging en = %d, ret = %d\n", en, ret);

	return ret;
}

static int usbpd_pm_check_sw_enabled(struct usbpd_pm *pdpm)
{
	int ret;
	union power_supply_propval val = {0,};

	ret = usbpd_get_iio_channel(pdpm, SMB, SMB_SW_CHARGING_ENABLED, &val.intval);
	if (!ret) {
		pdpm->sw_charging = !!val.intval;
	}

	pr_info("check sw enabled en = %d, ret = %d\n", pdpm->sw_charging, ret);

	return ret;
}

static int usbpd_pm_get_pd_active(struct usbpd_pm *pdpm, int *pd_active)
{
	int ret = 0;
	union power_supply_propval pval;

	ret = usbpd_get_iio_channel(pdpm, SMB, SMB_PD_ACTIVE, &pval.intval);
	if (!ret) {
		*pd_active = pval.intval;
	}

	pr_info("pd active:%d, ret = %d\n", *pd_active, ret);

	return ret;
}

static void usbpd_pm_update_info(struct usbpd_pm *pdpm)
{
	int ret;
	union power_supply_propval val = {0,};

	if (!pdpm->batt_psy) {
		pdpm->batt_psy = power_supply_get_by_name("battery");
		if (!pdpm->batt_psy) {
			pr_err("Failed to get battery psy.\n");
			return;
		}
	}

	ret = power_supply_get_property(pdpm->batt_psy, POWER_SUPPLY_PROP_CURRENT_NOW, &val);
	if (!ret)
		pdpm->ibat = (int)(val.intval / 1000);

	ret = power_supply_get_property(pdpm->batt_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
	if (!ret)
		pdpm->vbat = (int)(val.intval / 1000);

	ret = power_supply_get_property(pdpm->batt_psy, POWER_SUPPLY_PROP_TEMP, &val);
	if (!ret)
		pdpm->batt_temp = val.intval;

	ret = power_supply_get_property(pdpm->batt_psy, POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT, &val);
	if (!ret)
		pdpm->thermal_level = val.intval;

	ret = usbpd_get_iio_channel(pdpm, CP_MASTER, CHARGE_PUMP_SC_BUS_VOLTAGE, &val.intval);
	if (!ret)
		pdpm->vbus = val.intval;

	ret = usbpd_get_iio_channel(pdpm, CP_MASTER, CHARGE_PUMP_SC_BUS_CURRENT, &val.intval);
	if (!ret)
		pdpm->ibus= val.intval;

	ret = usbpd_get_iio_channel(pdpm, CP_MASTER, CHARGE_PUMP_SC_CHARGING_ENABLED, &val.intval);
	if (!ret)
		pdpm->cp_charging = val.intval;

	ret = power_supply_get_property(pdpm->batt_psy, POWER_SUPPLY_PROP_CAPACITY, &val);
	if (!ret)
		pdpm->soc = val.intval;

}

static int usbpd_pm_is_src_caps_update(struct usbpd_pm *pdpm)
{
	if (IS_ERR(pdpm->pd)) {
		return -ENODEV;
	}

	if (pdpm->src_cap_id != usbpd_get_src_cap_id(pdpm->pd)) {
		pdpm->src_cap_id = usbpd_get_src_cap_id(pdpm->pd);
		return true;
	}

	return false;
}

static void usbpd_pm_evaluate_src_caps(struct usbpd_pm *pdpm)
{
	int ret;
	int i;

	if (!pdpm->pd) {
		pdpm->pd = usbpd_get_pd_lobal();
		if (!pdpm->pd) {
			pr_err("couldn't get usbpd device\n");
			return;
		}
	}

	ret = usbpd_fetch_pdo(pdpm->pd, pdpm->pdo);
	if (ret) {
		pr_err("Failed to fetch pdo info\n");
		return;
	}

	pdpm->apdo_max_volt = pm_config.min_adapter_volt_required;
	pdpm->apdo_max_curr = pm_config.min_adapter_curr_required;

	for (i = 0; i < PDO_MAX_NUM; i++) {
		if (pdpm->pdo[i].type == PD_SRC_PDO_TYPE_AUGMENTED
			&& pdpm->pdo[i].pps && pdpm->pdo[i].pos) {
			if (pdpm->pdo[i].max_volt_mv >= pdpm->apdo_max_volt && pdpm->pdo[i].curr_ma >= pdpm->apdo_max_curr) {
				pdpm->apdo_max_volt = pdpm->pdo[i].max_volt_mv;
				pdpm->apdo_max_curr = pdpm->pdo[i].curr_ma;
				pdpm->apdo_selected_pdo = pdpm->pdo[i].pos;
				pdpm->pps_supported = true;
				break;
			}
		}
	}

	if (pdpm->pps_supported) {
		if (pdpm->apdo_max_curr > MAX_POWER_PPS_CURR_THR) {
			pr_info("PPS apdo_max_curr(%d) > MAX_POWER_PPS_CURR_THR(%d)\n", pdpm->apdo_max_curr, MAX_POWER_PPS_CURR_THR);
			pdpm->apdo_max_curr = MAX_POWER_PPS_CURR_THR;
		}

		usbpd_set_iio_channel(pdpm, SMB, SMB_APDO_VOLT,pdpm->apdo_max_volt);
		usbpd_set_iio_channel(pdpm, SMB, SMB_APDO_CURR,pdpm->apdo_max_curr);
		pr_info("PPS supported, preferred APDO pos:%d, max volt:%d, current:%d\n",
				pdpm->apdo_selected_pdo, pdpm->apdo_max_volt, pdpm->apdo_max_curr);

		if (pdpm->apdo_max_curr <= LOW_POWER_PPS_CURR_THR)
			pdpm->apdo_max_curr = XIAOMI_LOW_POWER_PPS_CURR_MAX;

		usbpd_pm_is_src_caps_update(pdpm);
	} else {
		pr_info("Not qualified PPS adapter\n");
	}
}

static int pd_update_iterm(struct usbpd_pm *pdpm)
{
	union power_supply_propval val = {0, };
	struct power_supply *psy_fg = NULL;
	int rc = -EINVAL;

	if (IS_ERR_OR_NULL(pdpm)) {
		pr_err("pdpm is err or null\n");
		return rc;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (IS_ERR_OR_NULL(psy_fg)) {
		pr_err("chip is err or null\n");
		return rc;
	}

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT, &val);
	if (rc < 0) {
		pr_err("Get charger iterm failed, rc: %d\n", rc);
		return rc;
	} else {
		pdpm->iterm = val.intval/1000; //uA to mA
	}

	pr_debug("term: %d\n", pdpm->iterm);
	return pdpm->iterm;
}

static void update_gauge_info(struct usbpd_pm *pdpm)
{
	int rsoc = 0, fg_vbat = 0, terger_cv = 0, iterm = 0;
	if (IS_ERR_OR_NULL(pdpm)) {
		pr_err("pdpm is err or null\n");
		return;
	}

	terger_cv = pd_get_gauge_cv(pdpm);
	if (terger_cv > 0) {
		pdpm->fg_cv = terger_cv;
	}

	fg_vbat =  pd_get_fg_vbat(pdpm);
	if (fg_vbat > 0) {
		pdpm->fg_vbat = fg_vbat;
	}

	rsoc = pd_get_fg_rsoc(pdpm);
	if (rsoc >= 0) {
		pdpm->fg_rsoc = rsoc;
	}

	iterm = pd_update_iterm(pdpm);
	if(iterm >= 0){
		if (pdpm->iterm >= pm_config.fc2_taper_current) {
			pdpm->iterm = pdpm->iterm + ITERM_RISE_HYS_MA;//mA
		} else {
			pdpm->iterm = pm_config.fc2_taper_current;
		}
	}

	return;
}
static int usbpd_pm_fc2_charge_algo(struct usbpd_pm *pdpm)
{
	int steps = 0, sw_ctrl_steps = 0;
	int step_vbat = 0, step_ibus = 0, step_ibat = 0;
	int fcc_vote_val = 0, effective_fcc_taper = 0;
	int fcc_ibatt_diff = 0 ,sicl_ibus_diff = 0;
	int taper_hys_mv = TAPER_HYS_MV;
	static int ibus_limit, fcc_limit;
	static int cool_warm_overcharge_timer;
	int time_delta = 0;
	int ret;
	union power_supply_propval val = {0,};
	struct bq_fg_chip *bq = NULL;
	struct power_supply *psy_fg = NULL;
#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
	static int g_smartchg_val = 0;
	static int g_nightchg_val = 0;
	static int g_endurance_pro_val = 0;
#endif

	time_delta = ktime_ms_delta(ktime_get(), pdpm->entry_cv_time);
	fcc_vote_val = usbpd_get_effective_fcc_val(pdpm);
	fcc_limit = min(fcc_vote_val, pm_config.bat_curr_lp_lmt);

	ret = usbpd_get_iio_channel(pdpm, SMB, SMB_APDO_CURR,&val.intval);
	if(ret < 0) {
		ibus_limit = fcc_limit >> 1;
		pr_err("get pdo_curr err, ibus_limit = %d\n", ibus_limit);
	} else {
		ibus_limit = (fcc_limit >> 1) + IBUS_RISE_HYS_MA;
		ibus_limit = min(ibus_limit, val.intval);
		pr_info("ibus_limit = %d\n", ibus_limit);
	}

#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
	pdpm->chg_disable = find_votable("CHG_DISABLE");
	if (pdpm->chg_disable) {
		g_smartchg_val = get_client_vote(pdpm->chg_disable, SMART_CHARGING_VOTER);
		g_nightchg_val = get_client_vote(pdpm->chg_disable, NIGHT_CHARGING_VOTER);
		g_endurance_pro_val = get_client_vote(pdpm->chg_disable, ENDURANCE_PRO_VOTER);
	}
	pr_info("ctrl val: smart chg = %d, night chg = %d, endurance pro = %d\n",
				g_smartchg_val, g_nightchg_val, g_endurance_pro_val);

	if (g_smartchg_val || g_nightchg_val || g_endurance_pro_val) {
		pr_info("PD is ctrl\n");
		pdpm->ctrl_flag = true;
		return PM_ALGO_SMART_CHG_DISABLED;
	} else if (!g_smartchg_val && pdpm->ctrl_flag) {
		pdpm->ctrl_flag = false;
		pr_info("release ctrl\n");
		return PM_ALGO_SMART_CHG_ENABLED;
	}
#endif
	update_gauge_info(pdpm);
	/* reduce bus current in cv loop */
	if ((pdpm->fg_vbat > (pdpm->fg_cv - taper_hys_mv))) {
		if (ibus_lmt_change_timer++ > IBUS_CHANGE_TIMEOUT) {
			ibus_lmt_change_timer = 0;
			ibus_limit = ibus_limit - 100;
			effective_fcc_taper = fcc_vote_val - TAPER_DECREASE_STEP_MA;
			if (pdpm->fcc_votable) {
				if(effective_fcc_taper >= pdpm->iterm) {
					effective_fcc_taper = effective_fcc_taper * 1000;
					vote(pdpm->fcc_votable, TAPER_FCC_VOTER, true, effective_fcc_taper);
					pr_info("set taper fcc to : %d uA\n", effective_fcc_taper);
				}
			}
		}
	} else if ((pdpm->vbat < pm_config.bat_volt_lp_lmt - 250) &&
				(pdpm->fg_vbat < pdpm->fg_cv - 250)) {
		ibus_lmt_change_timer = 0;
	} else {
		ibus_lmt_change_timer = 0;
	}

	/* battery voltage loop*/
	if (pdpm->vbat > pm_config.bat_volt_lp_lmt)
		step_vbat = -pm_config.fc2_steps;
	else if (pdpm->vbat < pm_config.bat_volt_lp_lmt - 10)
		step_vbat = pm_config.fc2_steps;;

	/* battery charge current loop*/
	if (pdpm->ibat < fcc_limit)
		step_ibat = pm_config.fc2_steps;
	else if (pdpm->ibat > fcc_limit + 100)
		step_ibat = -pm_config.fc2_steps;

	/* bus current loop*/
	if (pdpm->ibus < ibus_limit - 80)
		step_ibus = pm_config.fc2_steps;
	else if (pdpm->ibus > ibus_limit - 80)
		step_ibus = -pm_config.fc2_steps;

	pr_info("vbat:%d,rsoc:%d,lmt:%d; ibat:%d,lmt:%d; ibus:%d,lmt:%d,fg_vbat:%d,fg_cv:%d,iterm:%d",
			pdpm->vbat, pdpm->fg_rsoc, pm_config.bat_volt_lp_lmt, pdpm->ibat, fcc_limit, pdpm->ibus, ibus_limit, pdpm->fg_vbat, pdpm->fg_cv, pdpm->iterm);

	sw_ctrl_steps = min(min(step_vbat, step_ibus), step_ibat);
	sw_ctrl_steps = min(sw_ctrl_steps, pm_config.fc2_steps);

	/* check if cp disabled due to other reason*/
	usbpd_pm_check_cp_enabled(pdpm);
	if (!pdpm->cp_charging) {
		pr_err("cp disabled due to other reason, return\n");
		return PM_ALGO_RET_CHG_DISABLED;
	}

	pdpm->is_temp_out_fc2_range = pd_disable_cp_by_jeita_status(pdpm);
	psy_fg = power_supply_get_by_name("bq28z610");
	if (!IS_ERR_OR_NULL(psy_fg)) {
		bq = power_supply_get_drvdata(psy_fg);
		if (!bq){
			pr_err("get fg drv data  failed\n");
                } else if (bq->irregular_batt) {
			pr_info("disable cp, irregular_batt:%d\n", bq->irregular_batt);
			return PM_ALGO_RET_CHG_DISABLED;
		}
	} else {
		pr_err("get fg psy failed\n");
        }

	if (pdpm->thermal_level >= MAX_THERMAL_LEVEL || pdpm->is_temp_out_fc2_range) {
		pr_info("system thermal level too high or batt temp is out of fc2 range\n");
		return PM_ALGO_RET_CHG_DISABLED;
	} else if (!pdpm->cp_charging) {
		pr_err("cp_charging:%d\n", pdpm->cp_charging);
		return PM_ALGO_RET_CHG_DISABLED;
	}

	/*check overcharge when it is cool*/
	if ((pdpm->vbat > pm_config.bat_volt_lp_lmt && is_cool_charge(pdpm))
        || (pdpm->vbat > BATT_WARM_CHG_VOLT && is_warm_charge(pdpm))) {
		if (cool_warm_overcharge_timer++ > TAPER_TIMEOUT) {
			pr_info("cool warm overcharge\n");
			cool_warm_overcharge_timer = 0;
			pdpm->cool_warm_done = true;
			return PM_ALGO_RET_CHG_DISABLED;
		}
	} else {
		cool_warm_overcharge_timer = 0;
	}

	/* charge pump taper charge */
	if (((pdpm->fg_vbat > (pdpm->fg_cv - taper_hys_mv)) && pdpm->ibat <= pdpm->iterm) ||
			pdpm->fg_rsoc >= 99 ||
			pdpm->fg_vbat > (pdpm->fg_cv - 10)) {
		if (fc2_taper_timer++ > TAPER_TIMEOUT) {
			fc2_taper_timer = 0;
			if (effective_fcc_taper > PD_FFC_TAPER_MAX_CURRENT) {
				effective_fcc_taper = PD_FFC_TAPER_MAX_CURRENT;
			} else if (effective_fcc_taper < (pdpm->iterm * 1000)) {
				effective_fcc_taper = pdpm->iterm * 1000;
			}
			pr_info("charge pump taper charging done, set taper current: %duA\n", effective_fcc_taper);
			vote(pdpm->fcc_votable, JEITA_FCC_TAPER_VOTER, true, effective_fcc_taper);
			return PM_ALGO_RET_TAPER_DONE;
		}
	} else {
		fc2_taper_timer = 0;
	}

	steps = sw_ctrl_steps;

	if(pdpm->ibat > 0 && pdpm->ibus > 0 && time_delta < QUICK_RAISE_VOLT_INTERVAL_S) {
		fcc_ibatt_diff = (pdpm->ibat > fcc_limit) ? (pdpm->ibat - fcc_limit) : (fcc_limit - pdpm->ibat);
		sicl_ibus_diff = (pdpm->ibus > ibus_limit) ? (pdpm->ibus - ibus_limit) : (ibus_limit - pdpm->ibus);
		pr_err("fcc_ibatt_diff:%d sicl_ibus_diff:%d\n", fcc_ibatt_diff, sicl_ibus_diff);
		if (fcc_ibatt_diff > 1200 && sicl_ibus_diff > 500) {
			pdpm->pm_work_time_ms = fast_work_time;
			steps = steps * fast_tune_step;
		} else if (fcc_ibatt_diff > 500 && fcc_ibatt_diff <= 1200
			&& sicl_ibus_diff <= 500 && sicl_ibus_diff > 250) {
			pdpm->pm_work_time_ms = stable_work_time;
			steps = steps * stable_tune_step;
		}
	}
	pr_info("step_vbat:%d step_ibat:%d step_ibus:%d sw_ctrl_steps:%d steps:%d\n",
		step_vbat, step_ibat, step_ibus, sw_ctrl_steps, steps);
	pdpm->request_vbus += steps * 20;

	/* 11V3A PD charger cause the battery current over 6A, reduce the request_vbus */
	if (pdpm->request_vbus > (pdpm->apdo_max_volt == 11000 ? 10000 : pdpm->apdo_max_volt))
		pdpm->request_vbus = (pdpm->apdo_max_volt == 11000 ? 10000 : pdpm->apdo_max_volt);

	return PM_ALGO_RET_OK;
}

static const unsigned char *pm_str[] = {
	"PD_PM_STATE_ENTRY",
	"PD_PM_STATE_FC2_ENTRY",
	"PD_PM_STATE_FC2_ENTRY_1",
	"PD_PM_STATE_FC2_ENTRY_2",
	"PD_PM_STATE_FC2_ENTRY_3",
	"PD_PM_STATE_FC2_TUNE",
#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
	"PD_PM_STATE_HOLD",
#endif
	"PD_PM_STATE_FC2_EXIT",
};

static void usbpd_pm_move_state(struct usbpd_pm *pdpm, enum pm_state state)
{
	pr_info("state change:%s -> %s\n", pm_str[pdpm->state], pm_str[state]);
	pdpm->state = state;
}

static int usbpd_pm_sm(struct usbpd_pm *pdpm)
{
	int ret;
	int effective_fcc_val = 0;
	static int tune_vbus_retry;
	static bool stop_sw;
	static bool recover;
	static int fcc_lmt, ibus_lmt, retry_count;
	struct bq_fg_chip *bq = NULL;
	struct power_supply *psy_fg = NULL;
	struct power_supply *psy_batt = NULL;
	union power_supply_propval pval = {0, };
#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
	static int g_status = 0;
#endif

	switch (pdpm->state) {
	case PD_PM_STATE_ENTRY:
		stop_sw = false;
		recover = false;
		pdpm->pm_work_time_ms = fast_work_time;
		pdpm->select_fix_pdo = PD_SELECT_DEFAULT;
		pdpm->is_temp_out_fc2_range = pd_disable_cp_by_jeita_status(pdpm);

		usbpd_pm_enable_sw(pdpm, true);
		psy_fg = power_supply_get_by_name("bq28z610");
		if (!IS_ERR_OR_NULL(psy_fg)) {
			bq = power_supply_get_drvdata(psy_fg);
			if (!bq)
				pr_err("get fg drv data  failed\n");
		} else
			pr_err("get fg psy failed\n");

		psy_batt = power_supply_get_by_name("battery");
		if (!IS_ERR_OR_NULL(psy_batt)) {
			ret = power_supply_get_property(psy_batt, POWER_SUPPLY_PROP_STATUS, &pval);
			if (ret < 0) {
				pr_err("Failed to set status gauge to full , ret=%d\n", ret);
			}
		} else {
			pr_err("get batt psy failed\n");
		}

		effective_fcc_val = usbpd_get_effective_fcc_val(pdpm);
		if (effective_fcc_val > 0) {
			fcc_lmt = min(pm_config.bat_curr_lp_lmt, effective_fcc_val);
			ibus_lmt = fcc_lmt >> 1;
			pr_info("ibus_lmt:%d\n", ibus_lmt);
		}

		if (pdpm->vbat < pm_config.min_vbat_for_cp) {
			usbpd_select_pdo(pdpm->pd, 2, 0, 0);
			pr_info("batt_volt %d is too low for cp, waiting...\n", pdpm->vbat);
		} else if (pdpm->vbat > pm_config.bat_volt_lp_lmt - 100) {
			pr_info("batt_volt %d is too high for cp, charging with switch charger\n", pdpm->vbat);
			usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_EXIT);
		} else if (pdpm->thermal_level >= MAX_THERMAL_LEVEL || pdpm->is_temp_out_fc2_range) {
			pr_info("thermal:%d too high or batt temp:%d is out of fc2 range, status:%d, waiting...\n", pdpm->thermal_level, pdpm->batt_temp, pval.intval);
			if (pval.intval != POWER_SUPPLY_STATUS_FULL) {
				usbpd_select_pdo(pdpm->pd, 2, 0, 0);
			}
		} else if (bq && bq->irregular_batt) {
			pr_info("set 5v vbus, irregular_batt:%d\n", bq->irregular_batt);
			usbpd_select_pdo(pdpm->pd, 1, 0, 0);
		} else if (pdpm->soc >= 90) {
			pr_info("batt_soc %d is too high for cp, charging with switch charger\n", pdpm->soc);
			usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_EXIT);
		} else {
			pr_info("batt_volt %d is ok, start ffc charging\n", pdpm->vbat);
			usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_ENTRY);
		}
		break;

	case PD_PM_STATE_FC2_ENTRY:
		usbpd_select_pdo(pdpm->pd, 1, 0, 0);
		usbpd_pm_enable_sw(pdpm, false);
		usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_ENTRY_1);
		retry_count = 0;
		break;

	case PD_PM_STATE_FC2_ENTRY_1:
		if (retry_count >= 1)
			pdpm->request_vbus += STEP_MV;
		else
			pdpm->request_vbus = pdpm->vbat * 2 + BUS_VOLT_INIT_UP;

		if (usbpd_pm_is_src_caps_update(pdpm)) {
			usbpd_pm_evaluate_src_caps(pdpm);
		}

		pdpm->request_ibus = pdpm->apdo_max_curr;

		usbpd_select_pdo(pdpm->pd, pdpm->apdo_selected_pdo, pdpm->request_vbus * 1000, pdpm->request_ibus * 1000);
		pr_info("entry1 request_vbus:%d, request_ibus:%d\n", pdpm->request_vbus, pdpm->request_ibus);
		usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_ENTRY_2);
		tune_vbus_retry = 0;
		break;

	case PD_PM_STATE_FC2_ENTRY_2:
		pr_info("tune adapter vbus_volt %d, vbat_volt %d tune_vbus_retry=%d\n", pdpm->vbus, pdpm->vbat, tune_vbus_retry);
		if (pdpm->vbus < (pdpm->vbat * 2 + BUS_VOLT_INIT_UP - 50)) {
			tune_vbus_retry++;
			pdpm->request_vbus += STEP_MV;
			usbpd_select_pdo(pdpm->pd, pdpm->apdo_selected_pdo, pdpm->request_vbus * 1000, pdpm->request_ibus * 1000);
			pr_info("entry2 add request_vbus:%d, request_ibus:%d\n", pdpm->request_vbus, pdpm->request_ibus);
		} else if (pdpm->vbus > (pdpm->vbat * 2 + BUS_VOLT_INIT_UP + 200)) {
			tune_vbus_retry++;
			pdpm->request_vbus -= STEP_MV;
			usbpd_select_pdo(pdpm->pd, pdpm->apdo_selected_pdo, pdpm->request_vbus * 1000, pdpm->request_ibus * 1000);
			pr_info("entry2 reduce request_vbus:%d, request_ibus:%d\n", pdpm->request_vbus, pdpm->request_ibus);
		} else {
			pr_info("adapter volt tune ok, retry %d times\n", tune_vbus_retry);
			usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_ENTRY_3);
			break;
		}
		if (tune_vbus_retry > TUNE_VBUS_RETRY_MAX) {
			if (retry_count < 1) {
				usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_ENTRY_1);
				retry_count++;
				pr_info("Failed to tune adapter volt into valid range, retry again\n");
			} else {
				pr_info("Failed to tune adapter volt into valid range, charge with switching charger\n");
				usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_EXIT);
			}
		}
		break;

	case PD_PM_STATE_FC2_ENTRY_3:
		if (!pdpm->cp_charging) {
			usbpd_pm_enable_cp(pdpm, true);
			msleep(30);
			usbpd_pm_check_cp_enabled(pdpm);
		}

		if (pdpm->cp_charging) {
			pdpm->entry_cv_time  = ktime_get();
			usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_TUNE);
			ibus_lmt_change_timer = 0;
			fc2_taper_timer = 0;
		}
		break;

	case PD_PM_STATE_FC2_TUNE:
		if (usbpd_pm_is_src_caps_update(pdpm)) {
			usbpd_pm_evaluate_src_caps(pdpm);
		}

		ret = usbpd_pm_fc2_charge_algo(pdpm);
#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
		g_status = ret;
#endif
		if (ret == PM_ALGO_RET_TAPER_DONE) {
			pr_info("move to PD_PM_STATE_FC2_EXIT\n");
			usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_EXIT);
			break;
		} else if (ret == PM_ALGO_RET_CHG_DISABLED) {
			pr_info("move to PD_PM_STATE_FC2_EXIT, will try to recover flash charging:%d\n", ret);
			recover = true;
			usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_EXIT);
			break;
		}
#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
		else if ((ret == PM_ALGO_SMART_CHG_DISABLED) || (ret == PM_ALGO_SMART_CHG_ENABLED)) {
			pr_info("because smart ctrl, cp enter hold state = %d\n", ret);
			usbpd_pm_move_state(pdpm, PD_PM_STATE_HOLD);
			break;
		}
#endif
		else {
			usbpd_select_pdo(pdpm->pd, pdpm->apdo_selected_pdo, pdpm->request_vbus * 1000, pdpm->request_ibus * 1000);
			pr_info("request_vbus:%d, request_ibus:%d\n", pdpm->request_vbus, pdpm->request_ibus);
		}
		break;

#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
	case PD_PM_STATE_HOLD:
		pr_info("enter HOLD state\n");
		if (g_status == PM_ALGO_SMART_CHG_DISABLED) {
			usbpd_select_pdo(pdpm->pd, 1, 0, 0);

			if (pdpm->fcc_votable)
				vote(pdpm->fcc_votable, TAPER_FCC_VOTER, false, 0);

			if (!stop_sw && !pdpm->sw_charging) {
				usbpd_pm_enable_sw(pdpm, true);
			}

			if (stop_sw && pdpm->sw_charging)
				usbpd_pm_enable_sw(pdpm, false);

			usbpd_pm_check_sw_enabled(pdpm);

			if (pdpm->cp_charging) {
				usbpd_pm_enable_cp(pdpm, false);
				usbpd_pm_check_cp_enabled(pdpm);
			}

			usbpd_pm_move_state(pdpm, PD_PM_STATE_FC2_TUNE);
			pr_info("keep HOLD state\n");
		} else if (g_status == PM_ALGO_SMART_CHG_ENABLED) {
			usbpd_pm_move_state(pdpm, PD_PM_STATE_ENTRY);
			pr_info("switch PD_PM_STATE_ENTRY state\n");
		}
		break;
#endif

	case PD_PM_STATE_FC2_EXIT:
		if (pdpm->select_fix_pdo == PD_SELECT_DEFAULT) {
			pr_info("set pdo fixed:9V\n");
			usbpd_select_pdo(pdpm->pd, 2, 0, 0);
			pdpm->select_fix_pdo = PD_SELECT_FIXED_9V;
		}

		if (pdpm->fcc_votable)
			vote(pdpm->fcc_votable, TAPER_FCC_VOTER, false, 0);

		if (!stop_sw && !pdpm->sw_charging) {
			usbpd_pm_enable_sw(pdpm, true);
		}

		if (stop_sw && pdpm->sw_charging)
			usbpd_pm_enable_sw(pdpm, false);

		usbpd_pm_check_sw_enabled(pdpm);

		if (pdpm->cp_charging) {
			usbpd_pm_enable_cp(pdpm, false);
			usbpd_pm_check_cp_enabled(pdpm);
		}

		if (recover) {
			pdpm->select_fix_pdo = PD_SELECT_DEFAULT;
			usbpd_pm_move_state(pdpm, PD_PM_STATE_ENTRY);
		}
		break;

	default:
		pdpm->select_fix_pdo = PD_SELECT_DEFAULT;
		usbpd_pm_move_state(pdpm, PD_PM_STATE_ENTRY);
		break;
	}

	return false;
}

static void usbpd_pm_workfunc(struct work_struct *work)
{
	struct usbpd_pm *pdpm = container_of(work, struct usbpd_pm, pm_work.work);

	usbpd_pm_update_info(pdpm);

	if (!usbpd_pm_sm(pdpm) && pdpm->pd_active)
		schedule_delayed_work(&pdpm->pm_work, msecs_to_jiffies(pdpm->pm_work_time_ms));
}

static void usbpd_pm_disconnect(struct usbpd_pm *pdpm)
{
	cancel_delayed_work_sync(&pdpm->pm_work);

	if (pdpm->fcc_votable)
		vote(pdpm->fcc_votable, TAPER_FCC_VOTER, false, 0);

	pdpm->pps_supported = false;
	pdpm->jeita_triggered = false;
	pdpm->is_temp_out_fc2_range = false;
	pdpm->cool_warm_done = false;
	pdpm->apdo_selected_pdo = 0;
#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
	pdpm->ctrl_flag = false;
#endif

	memset(&pdpm->pdo, 0, sizeof(pdpm->pdo));

	usbpd_pm_enable_sw(pdpm, true);
	usbpd_pm_cp_adc_control(pdpm, false);
	usbpd_pm_move_state(pdpm, PD_PM_STATE_ENTRY);
}

static void usbpd_pm_pd_contact(struct usbpd_pm *pdpm, bool connected)
{
	pdpm->pd_active = connected;

	pr_info("pd_active = %d\n", connected);

	if (connected) {
		usbpd_pm_evaluate_src_caps(pdpm);
		if (pdpm->pps_supported) {
			usbpd_pm_cp_adc_control(pdpm, true);
			schedule_delayed_work(&pdpm->pm_work, 0);
		}
	} else {
		usbpd_pm_disconnect(pdpm);
	}
}

static int cycle_count_reduce_voltage(struct usbpd_pm *pdpm)
{
	int ret = 0;
	union power_supply_propval pval = {0, };

		ret = power_supply_get_property(pdpm->batt_psy, POWER_SUPPLY_PROP_VOLTAGE_MAX, &pval);
	if (ret < 0 || pval.intval == 0) {
		if(ret < 0)
			pr_err("voltage_max is not ready fail ret = %d\n", ret);
		else
			pr_err("voltage_max is 0, set to fixed(%d) \n", BAT_VOLT_LOOP_LMT);
		pm_config.bat_volt_lp_lmt = BAT_VOLT_LOOP_LMT;
	} else {
		pm_config.bat_volt_lp_lmt = BAT_VOLT_LOOP_LMT;
		pr_info("bat_volt_lp_lmt is %dmV, val is %duV\n", pm_config.bat_volt_lp_lmt, pval.intval);
	}

	return 0;
}

static void usbpd_pm_psy_change_work(struct work_struct *work)
{
	struct bq_fg_chip *bq = NULL;
	struct power_supply *psy_fg = NULL;
	union power_supply_propval pval;
	struct power_supply *psy_usb = NULL;
	struct usbpd_pm *pdpm = container_of(work, struct usbpd_pm, usb_psy_change_work);
	int ret = 0;
	int pd_active = 0;

	ret = usbpd_pm_get_pd_active(pdpm, &pd_active);
	if (ret) {
		pr_err("Failed to get usb pd active state\n");
		goto out;
	}

	psy_fg = power_supply_get_by_name("bq28z610");
	if (!IS_ERR_OR_NULL(psy_fg)) {
		bq = power_supply_get_drvdata(psy_fg);
		if (!bq) {
			pr_err("get fg drv data  failed\n");
                } else {
			if (bq->irregular_batt && pd_active == QTI_POWER_SUPPLY_PD_ACTIVE) {
				psy_usb = power_supply_get_by_name("usb");
				if (!IS_ERR_OR_NULL(psy_usb)) {
					if (!power_supply_get_property(psy_usb, POWER_SUPPLY_PROP_VOLTAGE_NOW, &pval)) {
						pr_info("get vbus voltage:%dmV\n", pval.intval / 1000);
						if (pval.intval > HVDCP_MIN_VOL_UV) {
							pr_info("force 5v pd, irregular_batt:%d\n", bq->irregular_batt);
							usbpd_select_pdo(pdpm->pd, 1, 0, 0);
						}
					} else
						pr_err("get usb psy voltage prop failed\n");
				} else
					pr_err("get usb psy failed\n");
			}
		}
	} else {
		pr_err("get fg psy failed\n");
        }

	if (!pdpm->pd_active && (pd_active == QTI_POWER_SUPPLY_PD_PPS_ACTIVE)) {
		usbpd_pm_pd_contact(pdpm, true);
	} else if (pdpm->pd_active && pd_active != QTI_POWER_SUPPLY_PD_PPS_ACTIVE) {
		usbpd_pm_pd_contact(pdpm, false);
	}

out:
	pdpm->psy_change_running = false;
}

static int usbpd_pm_psy_notifier_cb(struct notifier_block *nb,
			unsigned long event, void *data)
{
	struct usbpd_pm *pdpm = container_of(nb, struct usbpd_pm, nb);
	struct power_supply *psy = data;
	unsigned long flags;
	static bool first_flag = true;
	int ret = 0;

	if (event != PSY_EVENT_PROP_CHANGED)
		return NOTIFY_OK;

	if (first_flag) {
		if (is_cp_chan_valid(pdpm, CHARGE_PUMP_SC_PRESENT)) {
			pdpm->bq25960_flag = false;
			first_flag = false;
			pr_info("charge pump is SC853x\n");
		} else if (is_cp_chan_valid(pdpm, BQ25960_IIO_CHANNEL_OFFSET)) {
			pdpm->bq25960_flag = true;
			first_flag = false;
			pr_info("[charge pump is BQ25960\n");
		} else {
			first_flag = true;
			pr_info("can't find charge pump\n");
		}
	}

	usbpd_check_usb_psy(pdpm);
	ret = usbpd_check_batt_psy(pdpm);
	if (!ret) {
		if (psy == pdpm->batt_psy) {
			ret = cycle_count_reduce_voltage(pdpm);
			if (ret) {
				pr_err("Failed to set cycle count reduce voltage\n");
			}
		}
	}

	if (psy != pdpm->usb_psy) {
		return NOTIFY_OK;
	} else {
		spin_lock_irqsave(&pdpm->psy_change_lock, flags);
		if (!pdpm->psy_change_running) {
			pdpm->psy_change_running = true;
			schedule_work(&pdpm->usb_psy_change_work);
		}
		spin_unlock_irqrestore(&pdpm->psy_change_lock, flags);
	}

	return NOTIFY_OK;
}

static int usbpd_iio_init(struct usbpd_pm *pdpm)
{
	pr_err("usbpd_iio_init start\n");

	pdpm->cp_iio = devm_kcalloc(pdpm->dev, ARRAY_SIZE(cp_iio_chan_name), sizeof(*pdpm->cp_iio), GFP_KERNEL);
	if (!pdpm->cp_iio)
		return -ENOMEM;
	pdpm->smb_iio = devm_kcalloc(pdpm->dev, ARRAY_SIZE(smb_iio_chan_name), sizeof(*pdpm->smb_iio), GFP_KERNEL);
	if (!pdpm->smb_iio)
		return -ENOMEM;

	pr_err("usbpd_iio_init end\n");

	return 0;
}

static int pdo_5v_vote_callback(struct votable *votable,
			void *data, int pdo_5v_working_flag, const char *client)
{
	pr_info("client: %s pdo_5v_working_flag: %d\n", client, pdo_5v_working_flag);
	if(pdo_5v_working_flag) {
		pr_info("set pdo to 5V\n");
		usbpd_select_pdo_5v();
	} else {
		pr_info("needn't set pdo to 5V\n");
	}

	return 0;
}

static int usbpd_pm_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct usbpd_pm *pdpm;
	struct power_supply *usb_psy = NULL;

	pr_err("enter\n");

	usb_psy = power_supply_get_by_name("usb");
	if (IS_ERR_OR_NULL(usb_psy)) {
		return -EPROBE_DEFER;
	}

	pdpm = kzalloc(sizeof(struct usbpd_pm), GFP_KERNEL);
	if (!pdpm)
		return -ENOMEM;

	pdpm->dev = dev;

	platform_set_drvdata(pdev, pdpm);
	spin_lock_init(&pdpm->psy_change_lock);
	usbpd_check_usb_psy(pdpm);
	usbpd_iio_init(pdpm);

	INIT_WORK(&pdpm->usb_psy_change_work, usbpd_pm_psy_change_work);
	INIT_DELAYED_WORK(&pdpm->pm_work, usbpd_pm_workfunc);

	pdpm->nb.notifier_call = usbpd_pm_psy_notifier_cb;
	power_supply_reg_notifier(&pdpm->nb);
	usbpd_pm_psy_notifier_cb(&pdpm->nb, PSY_EVENT_PROP_CHANGED, pdpm->usb_psy);
	pdpm->pdo_5v_votable = create_votable("PDO_5V", VOTE_SET_ANY, pdo_5v_vote_callback, pdpm);
	if (IS_ERR_OR_NULL(pdpm->pdo_5v_votable)) {
		pr_err("create votable PDO_5V fail\n");
		destroy_votable(pdpm->pdo_5v_votable);
	}

	g_pdpm = pdpm;
	pr_err("success\n");

	return ret;
}

static int usbpd_pm_remove(struct platform_device *pdev)
{
	struct usbpd_pm *pdpm = platform_get_drvdata(pdev);

	power_supply_unreg_notifier(&pdpm->nb);
	cancel_delayed_work(&pdpm->pm_work);
	cancel_work_sync(&pdpm->usb_psy_change_work);

	return 0;
}

static const struct of_device_id usbpd_pm_of_match[] = {
	{ .compatible = "xiaomi,usbpd-pm", },
	{},
};

static struct platform_driver usbpd_pm_driver = {
	.driver = {
		.name = "usbpd_pm",
		.owner = THIS_MODULE,
		.of_match_table = usbpd_pm_of_match,
	},
	.probe = usbpd_pm_probe,
	.remove = usbpd_pm_remove,
};

static int __init usbpd_pm_init(void)
{
	return platform_driver_register(&usbpd_pm_driver);
}
late_initcall(usbpd_pm_init);

static void __exit usbpd_pm_exit(void)
{
	return platform_driver_unregister(&usbpd_pm_driver);
}
module_exit(usbpd_pm_exit);

MODULE_DESCRIPTION("USB Power Delivery Policy Driver");
MODULE_LICENSE("GPL v2");
