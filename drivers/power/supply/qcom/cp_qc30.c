/*
 * State machine for qc3 when it works on cp
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#define pr_fmt(fmt)	"[QC-CHG]: %s: " fmt, __func__
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/pmic-voter.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>
#include <linux/iio/iio.h>
#include <linux/iio/consumer.h>
#include <linux/qti_power_supply.h>
#include "bq28z610.h"
#include "cp_qc30.h"
#include "smb5-lib.h"

#ifdef pr_debug
#undef pr_debug
#define pr_debug pr_err
#endif

#define BATT_MAX_CHG_VOLT         4460
#define BATT_FAST_CHG_CURR        5400
#define	BUS_OVP_THRESHOLD         12000
#define	BUS_OVP_ALARM_THRESHOLD   9500

#define BUS_VOLT_INIT_UP          200

#define BAT_VOLT_LOOP_LMT         BATT_MAX_CHG_VOLT
#define BAT_CURR_LOOP_LMT         BATT_FAST_CHG_CURR
#define BUS_VOLT_LOOP_LMT         BUS_OVP_THRESHOLD

#define VOLT_UP                   true
#define VOLT_DOWN                 false

#define ADC_ERR                   1
#define CP_ENABLE_FAIL            2
#define TAPER_DONE                1

static struct sys_config sys_config = {
	.bat_volt_lp_lmt		= BAT_VOLT_LOOP_LMT,
	.ffc_bat_volt_lmt		= BAT_VOLT_LOOP_LMT,
	.bat_curr_lp_lmt		= BAT_CURR_LOOP_LMT/* + 1000*/,
	.bus_volt_lp_lmt		= BUS_VOLT_LOOP_LMT,
	.bus_curr_lp_lmt		= BAT_CURR_LOOP_LMT >> 1,

	.ibus_minus_deviation_val = 400,
	.ibus_plus_deviation_val = 200,
	.ibat_minus_deviation_val = 600,
	.ibat_plus_deviation_val = 500,

	.fc2_taper_current		= 2200,
	.flash2_policy.down_steps	= -1,
	.flash2_policy.volt_hysteresis	= 50,

	.min_vbat_start_flash2		= 3000,
	.cp_sec_enable			= false,
};

struct cp_qc30_data {
	struct device *dev;
	int			iterm;
	int			fg_vbat;
	int			fg_cv;
	int			bat_volt_max;
	int			ffc_bat_volt_max;
	int			bat_curr_max;
	int			bus_volt_max;
	int			bus_curr_max;
	bool		cp_sec_enable;

	/* notifiers */
	struct notifier_block	nb;

	struct delayed_work	qc3_pm_work;

	struct iio_channel	**cp_iio;
	struct iio_channel	**smb_iio;
	int qc3_work_time;
	bool bq25960_flag;
};

static int debug_mask = 21;

module_param_named(debug_mask, debug_mask, int, 0600);

static pm_t pm_state;
static int fc2_taper_timer;
static int ibus_lmt_change_timer;

enum cp_iio_type {
	CP_MASTER,
	SMB,
};

#define BQ25960_IIO_CHANNEL_OFFSET      5
enum cp_iio_channels {
	CHARGE_PUMP_SC_PRESENT,
	CHARGE_PUMP_SC_CHARGING_ENABLED,
	CHARGE_PUMP_SC_BUS_VOLTAGE,
	CHARGE_PUMP_SC_BUS_CURRENT,
	CHARGE_PUMP_SC_ADC_CTROL,
	CHARGE_PUMP_BQ_PRESENT = BQ25960_IIO_CHANNEL_OFFSET,
	CHARGE_PUMP_BQ_CHARGING_ENABLED,
	CHARGE_PUMP_BQ_BUS_VOLTAGE,
	CHARGE_PUMP_BQ_BUS_CURRENT,
	CHARGE_PUMP_BQ_ADC_CTROL,
};

static const char * const cp_iio_chan_name[] = {
	[CHARGE_PUMP_SC_PRESENT] = "sc_present",
	[CHARGE_PUMP_SC_CHARGING_ENABLED] = "sc_charging_enabled",
	[CHARGE_PUMP_SC_BUS_VOLTAGE] = "sc_bus_voltage",
	[CHARGE_PUMP_SC_BUS_CURRENT] = "sc_bus_current",
	[CHARGE_PUMP_SC_ADC_CTROL] = "sc_adc_control",
	[CHARGE_PUMP_BQ_PRESENT] = "bq_present",
	[CHARGE_PUMP_BQ_CHARGING_ENABLED] = "bq_charging_enabled",
	[CHARGE_PUMP_BQ_BUS_VOLTAGE] = "bq_bus_voltage",
	[CHARGE_PUMP_BQ_BUS_CURRENT] = "bq_bus_current",
	[CHARGE_PUMP_BQ_ADC_CTROL] = "bq_adc_control",
};

enum smb_iio_channels {
	REAL_TYPE,
	HVDCP3_TYPE,
	CP_INPUT_SUSPEND,
	SW_CHARGING_ENABLED,
	DP_DM,
	BATTERY_CHARGING_LIMITED,
};

static const char * const smb_iio_chan_name[] = {
	[REAL_TYPE] = "real_type",
	[HVDCP3_TYPE] = "hvdcp3_type",
	[CP_INPUT_SUSPEND] = "cp_input_suspend",
	[SW_CHARGING_ENABLED] = "sw_charging_enabled",
	[DP_DM] = "battery_dp_dm",
	[BATTERY_CHARGING_LIMITED] = "battery_charging_limited",
};

static bool is_cp_chan_valid(struct cp_qc30_data *chip,
		enum cp_iio_channels chan)
{
	int rc;
	if (IS_ERR(chip->cp_iio[chan]))
		return false;
	if (!chip->cp_iio[chan]) {
		chip->cp_iio[chan] = iio_channel_get(chip->dev,
					cp_iio_chan_name[chan]);
		if (IS_ERR(chip->cp_iio[chan])) {
			rc = PTR_ERR(chip->cp_iio[chan]);
			// if (rc == -EPROBE_DEFER)
				chip->cp_iio[chan] = NULL;
			pr_err("Failed to get IIO channel %s, rc=%d\n",
				cp_iio_chan_name[chan], rc);
			return false;
		}
	}
	return true;
}

static bool is_smb_chan_valid(struct cp_qc30_data *chip,
		enum smb_iio_channels chan)
{
	int rc;
	if (IS_ERR(chip->smb_iio[chan]))
		return false;
	if (!chip->smb_iio[chan]) {
		chip->smb_iio[chan] = iio_channel_get(chip->dev,
					smb_iio_chan_name[chan]);
		if (IS_ERR(chip->smb_iio[chan])) {
			rc = PTR_ERR(chip->smb_iio[chan]);
			// if (rc == -EPROBE_DEFER)
				chip->smb_iio[chan] = NULL;
			pr_err("Failed to get IIO channel %s, rc=%d\n",
				smb_iio_chan_name[chan], rc);
			return false;
		}
	}
	return true;
}

static int cp_qc30_get_iio_channel(struct cp_qc30_data *chg,
	    enum cp_iio_type type, int channel, int *val)
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

static int cp_q30_set_iio_channel(struct cp_qc30_data *chg,
		enum cp_iio_type type, int channel, int val)
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

static struct power_supply *cp_get_sw_psy(void)
{
	if (!pm_state.sw_psy)
		pm_state.sw_psy = power_supply_get_by_name("battery");

	return pm_state.sw_psy;
}

static struct power_supply *cp_get_usb_psy(void)
{
	if (!pm_state.usb_psy)
		pm_state.usb_psy = power_supply_get_by_name("usb");

	return pm_state.usb_psy;
}

static int cp_get_effective_fcc_val(pm_t pm_state)
{
	int effective_fcc_val = 0;

	if (!pm_state.fcc_votable)
		pm_state.fcc_votable = find_votable("FCC");

	if (!pm_state.fcc_votable)
		return -EINVAL;

	effective_fcc_val = get_effective_result(pm_state.fcc_votable);
	effective_fcc_val = effective_fcc_val / 1000;
	pr_info("effective_fcc_val: %d\n", effective_fcc_val);
	return effective_fcc_val;
}

static int cp_get_effective_usb_icl_val(void)
{
	int effective_usb_icl_val = 0;

	if (!pm_state.usb_icl_votable)
		pm_state.usb_icl_votable = find_votable("USB_ICL");

	if (!pm_state.usb_icl_votable) {
		pr_err("[%s] find votable: USB_ICL failed!\n", __func__);
		return -EINVAL;
	}

	effective_usb_icl_val = get_effective_result(pm_state.usb_icl_votable);
	pr_info("effective_usb_icl_val: %d voted by:%s\n", effective_usb_icl_val, get_effective_client(pm_state.usb_icl_votable));
	return effective_usb_icl_val;
}


static int cp_get_gauge_cv(struct cp_qc30_data *chip)
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
		pr_err("Get gauge cv failed, rc: %d\n", rc);
		return rc;
	} else {
		val.intval = val.intval / 1000; //uV to mV
	}

	return val.intval;
}

static int cp_get_fg_vbat(struct cp_qc30_data *chip)
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

	rc = power_supply_get_property(psy_fg, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
	if (rc < 0) {
		pr_err("Get gauge vbat failed, rc: %d\n", rc);
		return rc;
	} else {
		val.intval = val.intval / 1000; //uV to mV
	}

	return val.intval;
}

static int cp_get_fg_iterm(struct cp_qc30_data *chip)
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
		pr_err("Get gauge vbat failed, rc: %d\n", rc);
		return rc;
	} else {
		val.intval = val.intval / 1000; //uA to mA
	}

	return val.intval;
}

static void cp_update_batt_info(void)
{
	int ret;
	struct power_supply *psy;
	union power_supply_propval val = {0,};

	psy = cp_get_sw_psy();
	if (!psy)
		return;

	ret = power_supply_get_property(psy, POWER_SUPPLY_PROP_CURRENT_NOW, &val);
	if (!ret)
		pm_state.ibat_now = (int)(val.intval / 1000);

	ret = power_supply_get_property(psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
	if (!ret)
		pm_state.vbat_volt = (int)(val.intval/1000);

	pr_info("ibat:%d vbat_from_qg:%d \n", pm_state.ibat_now, pm_state.vbat_volt);
}

static int qc3_get_bms_fastcharge_mode(void)
{
	union power_supply_propval pval = {0,};
	int rc;
	struct power_supply *psy;

	psy = cp_get_sw_psy();

	rc = power_supply_get_property(psy,
				POWER_SUPPLY_PROP_CHARGE_TYPE, &pval);
	if (rc < 0) {
		pr_info("Couldn't get fastcharge mode:%d\n", rc);
		return 0;
	}

	pm_state.bms_fastcharge_mode = pval.intval;

	return pval.intval;
}

/* get thermal level from battery power supply property */
static int qc3_get_batt_current_thermal_level(int *level)
{
	int rc;
	struct power_supply *psy;
	union power_supply_propval val = {0,};

	psy = cp_get_sw_psy();
	if (!psy)
		return 0;

	rc = power_supply_get_property(psy,
			POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT, &val);

	if (rc < 0) {
		pr_info("Couldn't get themal level:%d\n", rc);
		return rc;
	}

	pr_debug("val.intval: %d\n", val.intval);

	*level = val.intval;
	return rc;
}

/* get input suspend */
static int cp_qc30_get_input_suspend(struct cp_qc30_data *chip, int *input_suspend)
{
	union power_supply_propval pval = {0,};
	int rc;

	rc = cp_qc30_get_iio_channel(chip, SMB,
		CP_INPUT_SUSPEND, &pval.intval);
	if (!rc) {
		*input_suspend = pval.intval;
		pr_info("get input suspend:%d, rc = %d\n", *input_suspend, rc);
	}

	return rc;
}

/* determine whether to disable cp according to jeita status */
static bool qc3_disable_cp_by_jeita_status(struct cp_qc30_data *chip)
{
	int batt_temp = 0, cp_input_suspend = 0;
	int ret;
	struct power_supply *psy;
	union power_supply_propval val = {0,};

	ret = cp_qc30_get_input_suspend(chip, &cp_input_suspend);
	if (ret < 0) {
		pr_err("Failed to get input suspend, ret = %d\n", ret);
		return false;
	}

	psy = cp_get_sw_psy();
	if (!psy)
		return false;

	ret = power_supply_get_property(psy, POWER_SUPPLY_PROP_TEMP, &val);
	if (ret < 0) {
		pr_info("Couldn't get batt temp prop:%d\n", ret);
		return false;
	}

	batt_temp = val.intval;
	pr_debug("batt_temp: %d\n", batt_temp);

	if (cp_input_suspend) {
		return true;
	} else {
		if (batt_temp >= JEITA_WARM_THR && !pm_state.jeita_triggered) {
			pm_state.jeita_triggered = true;
			pr_err("warm batt_temp: %d\n", batt_temp);
			return true;
		} else if (batt_temp <= JEITA_COOL_NOT_ALLOW_CP_THR
				&& !pm_state.jeita_triggered) {
			pm_state.jeita_triggered = true;
			pr_err("cool batt_temp: %d\n", batt_temp);
			return true;
		} else if ((batt_temp <= (JEITA_WARM_THR - JEITA_HYSTERESIS))
					&& (batt_temp >= (JEITA_COOL_NOT_ALLOW_CP_THR + JEITA_HYSTERESIS))
				&& pm_state.jeita_triggered) {
			pm_state.jeita_triggered = false;
			pr_err("return to normal batt_temp: %d\n", batt_temp);
			return false;
		} else {
			return pm_state.jeita_triggered;
		}
	}
}

static void cp_get_batt_capacity(void)
{
	int ret;
	struct power_supply *psy;
	union power_supply_propval val = {0,};

	psy = cp_get_sw_psy();
	if (!psy)
		return;

	ret = power_supply_get_property(psy,
			POWER_SUPPLY_PROP_CAPACITY, &val);
	if (!ret)
		pm_state.capacity = val.intval;
	pr_info("capacity:%d\n", pm_state.capacity);
}

static void cp_update_fc_status(struct cp_qc30_data *chip)
{
	int ret;
	union power_supply_propval val = {0,};

	ret = cp_qc30_get_iio_channel(chip, CP_MASTER,
		 CHARGE_PUMP_SC_BUS_VOLTAGE, &val.intval);
	if (!ret)
		pm_state.charge_pump.vbus_volt = val.intval;
	ret = cp_qc30_get_iio_channel(chip, CP_MASTER,
		 CHARGE_PUMP_SC_BUS_CURRENT, &val.intval);
	if (!ret)
		pm_state.charge_pump.ibus_curr = val.intval;
/*
	ret = cp_qc30_get_iio_channel(chip, CP_MASTER,
		CHARGE_PUMP_SC_VBUS_PRESENT, &val.intval);
	if (!ret)
		pm_state.charge_pump.vbus_pres = val.intval;
*/
	cp_update_batt_info();
	pm_state.charge_pump.ibat_curr = pm_state.ibat_now;
	pm_state.charge_pump.vbat_volt = pm_state.vbat_volt;

	ret = cp_qc30_get_iio_channel(chip, CP_MASTER,
		CHARGE_PUMP_SC_CHARGING_ENABLED, &val.intval);
	if (!ret)
		pm_state.charge_pump.charge_enabled = val.intval;
}

static int cp_enable_charge_pump_adc(struct cp_qc30_data *chip, bool enable)
{
	int ret;
	union power_supply_propval val = {0,};

	val.intval = enable;
	ret = cp_q30_set_iio_channel(chip, CP_MASTER,
		CHARGE_PUMP_SC_ADC_CTROL, val.intval);
	pr_debug("cp enable adc %d,ret is %d \n", val.intval, ret);

	return ret;
}

static int cp_enable_charge_pump(struct cp_qc30_data *chip, bool enable)
{
	int ret;
	union power_supply_propval val = {0,};

	val.intval = enable;
	ret = cp_q30_set_iio_channel(chip, CP_MASTER,
		CHARGE_PUMP_SC_CHARGING_ENABLED, val.intval);
	pr_debug("cp enable %d,ret is %d \n", val.intval, ret);

	return ret;
}

static int cp_enable_sw(struct cp_qc30_data *chip, bool enable)
{
	union power_supply_propval val = {0,};
	int ret;

	val.intval = enable;
	ret = cp_q30_set_iio_channel(chip, SMB,
			SW_CHARGING_ENABLED, val.intval);
	if (ret < 0) {
		pr_err("set battey charging fail\n");
	}
	pr_err("set sw charging en = %d\n", enable);

	return ret;
}

static int cp_check_charge_pump_enabled(struct cp_qc30_data *chip)
{
	int ret;
	union power_supply_propval val = {0,};

	ret = cp_qc30_get_iio_channel(chip, CP_MASTER,
		CHARGE_PUMP_SC_CHARGING_ENABLED, &val.intval);
	if (!ret)
		pm_state.charge_pump.charge_enabled = !!val.intval;

	pr_debug("pm_state.charge_pump.charge_enabled: %d\n",
		pm_state.charge_pump.charge_enabled);
	return ret;
}

static int cp_check_sw_enabled(struct cp_qc30_data *chip)
{
	int ret;
	union power_supply_propval val = {0,};

	ret = cp_qc30_get_iio_channel(chip, SMB,
		SW_CHARGING_ENABLED, &val.intval);
	if (!ret)
		pm_state.sw_chager.charge_enabled = !!val.intval;

	pr_debug("battery charging enabled: %d\n",
			pm_state.sw_chager.charge_enabled);
	return ret;
}

static int cp_limit_sw(struct cp_qc30_data *chip, bool enable)
{
	int ret = 0;
	union power_supply_propval val = {0,};

	val.intval = enable;
	ret = cp_q30_set_iio_channel(chip, SMB,
		BATTERY_CHARGING_LIMITED, val.intval);
	if (ret < 0) {
		pr_err("cp_limit_sw fail\n");
	}

	return ret;
}

static int cp_check_sw_limited(struct cp_qc30_data *chip)
{
	int ret;
	union power_supply_propval val = {0,};

	ret = cp_qc30_get_iio_channel(chip, SMB,
		BATTERY_CHARGING_LIMITED, &val.intval);
	if (!ret)
		pm_state.sw_chager.charge_limited = !!val.intval;

	pr_debug("pm_state.sw_chager.charge_limited: %d\n",
			pm_state.sw_chager.charge_limited);
	return ret;
}

static void cp_update_sw_status(struct cp_qc30_data *chip)
{
	cp_check_sw_enabled(chip);
	cp_check_sw_limited(chip);
}

static int cp_tune_vbus_volt(struct cp_qc30_data *chip, bool up)
{
	int ret;
	union power_supply_propval val = {0,};

	if (up)
		val.intval = QTI_POWER_SUPPLY_DP_DM_DP_PULSE;
	else
		val.intval = QTI_POWER_SUPPLY_DP_DM_DM_PULSE;

	ret = cp_q30_set_iio_channel(chip, SMB, DP_DM, val.intval);

	pr_info("tune adapter voltage %s %s\n", up ? "up" : "down",
			ret ? "fail" : "successfully");

	return ret;
}

static int cp_reset_vbus_volt(struct cp_qc30_data *chip)
{
	int ret = 0;
	int qc3p5_reset_vbus_retry = 0;
	int val = 0;
	if (pm_state.usb_type == QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3P5) {
		cp_update_fc_status(chip);
		pr_err("vbat=%d,vbus=%d\n", pm_state.charge_pump.vbat_volt, pm_state.charge_pump.vbus_volt);
		while (pm_state.charge_pump.vbus_volt > 6550) {
			cp_tune_vbus_volt(chip, VOLT_DOWN);
			cp_update_fc_status(chip);
			qc3p5_reset_vbus_retry++;
			usleep_range(10000, 10010);
			pr_err("qc3p5_reset_vbus_retry=%d,vbus=%d\n", qc3p5_reset_vbus_retry, pm_state.charge_pump.vbus_volt);
			if (qc3p5_reset_vbus_retry > 300) {
				pr_err("Failed to reset qc3.5 adapter volt to 6.55v or less\n");
				break;
			}
		}
	} else {
		val = QTI_POWER_SUPPLY_DP_DM_FORCE_5V;
		ret = cp_q30_set_iio_channel(chip, SMB, DP_DM, val);
		pr_info("reset vbus volt %s\n", ret ? "fail" : "successfully");
	}

	return ret;
}

static int cp_get_usb_type(struct cp_qc30_data *chip)
{
	int ret;
	int real_type;

	ret = cp_qc30_get_iio_channel(chip, SMB, REAL_TYPE, &real_type);
	if (!ret) {
		pm_state.usb_type = real_type;
		pr_info("cp_get_usb_type:%d\n", real_type);
	}

	return ret;
}
static int get_usb_online(void)
{
	int ret;
	struct power_supply *psy;
	union power_supply_propval val = {0,};

	psy = cp_get_usb_psy();
	if (!psy)
		return -ENODEV;

	ret = power_supply_get_property(psy,
			POWER_SUPPLY_PROP_ONLINE, &val);
	if (!ret)
		pm_state.usb_online = val.intval;

	return ret;
}

static int cp_get_usb_present(void)
{
	int ret;
	struct power_supply *psy;
	union power_supply_propval val = {0,};

	psy = cp_get_usb_psy();
	if (!psy)
		return -ENODEV;

	ret = power_supply_get_property(psy,
			POWER_SUPPLY_PROP_PRESENT, &val);
	if (!ret)
		pm_state.usb_present = val.intval;

	return ret;
}

static int cp_get_qc_hvdcp3_type(struct cp_qc30_data *chip)
{
	int ret;
	int hvdcp3_type;

	ret = cp_qc30_get_iio_channel(chip, SMB, HVDCP3_TYPE, &hvdcp3_type);
	if (!ret) {
		pm_state.hvdcp3_type = hvdcp3_type;
		pr_info("cp_get_qc_hvdcp3_type:%d\n", hvdcp3_type);
	}

	return ret;
}

#define TAPER_TIMEOUT	3
#define IBUS_CHANGE_TIMEOUT  5
static int cp_flash2_charge(struct cp_qc30_data *chip, unsigned int port)
{
	static int ibus_limit,ibat_limit;
	int thermal_level = 0;
	uint16_t effective_fcc_val = cp_get_effective_fcc_val(pm_state);
	uint16_t effective_ibus_val = effective_fcc_val/2;
	int fg_vbat = 0, fg_cv = 0, fg_iterm = 0;

	if (ibus_limit == 0)
		ibus_limit = pm_state.ibus_lmt_curr;
	ibus_limit = min(effective_ibus_val, pm_state.ibus_lmt_curr);

	qc3_get_bms_fastcharge_mode();
	if (pm_state.bms_fastcharge_mode == POWER_SUPPLY_CHARGE_TYPE_FAST) {
		if (pm_state.usb_type == QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3P5)
			sys_config.bat_volt_lp_lmt = 4460;
		else
			sys_config.bat_volt_lp_lmt = sys_config.ffc_bat_volt_lmt;
	}

	qc3_get_batt_current_thermal_level(&thermal_level);
	pm_state.is_temp_out_fc2_range = qc3_disable_cp_by_jeita_status(chip);
	ibat_limit = min(effective_fcc_val, sys_config.bat_curr_lp_lmt);

	fg_cv = cp_get_gauge_cv(chip);
	if (fg_cv > 0) {
		chip->fg_cv = fg_cv;
	}

	fg_vbat = cp_get_fg_vbat(chip);
	if (fg_vbat > 0) {
		chip->fg_vbat = fg_vbat;
	}

	fg_iterm = cp_get_fg_iterm(chip);
	if (fg_iterm > 0) {
		chip->iterm = fg_iterm;
	}

	pr_info("vbus=%d; ibus_lmt=%d,ibus:%d; vbat_lmt=%d,vbat=%d; ibat_lmt=%d,ibat=%d,fg_cv=%d,fg_vbat=%d,iterm=%d\n",
				pm_state.charge_pump.vbus_volt,
				ibus_limit,
				pm_state.charge_pump.ibus_curr,
				sys_config.bat_volt_lp_lmt,
				pm_state.charge_pump.vbat_volt,
				ibat_limit,
				pm_state.charge_pump.ibat_curr,
				chip->fg_cv,
				chip->fg_vbat,
				chip->iterm);

	if (pm_state.charge_pump.vbus_volt <= 9500
		&& pm_state.charge_pump.ibus_curr < ibus_limit - sys_config.ibus_minus_deviation_val
		&& pm_state.charge_pump.ibat_curr < ibat_limit - sys_config.ibat_minus_deviation_val
		&& pm_state.charge_pump.vbat_volt < sys_config.bat_volt_lp_lmt - 50) {
			pr_err("cp_tune_vbus_volt up\n");
			cp_tune_vbus_volt(chip, VOLT_UP);
	}

	if (pm_state.charge_pump.vbat_volt > sys_config.bat_volt_lp_lmt
		|| pm_state.charge_pump.ibat_curr > ibat_limit + sys_config.ibat_plus_deviation_val
		|| pm_state.charge_pump.ibus_curr > ibus_limit + sys_config.ibus_plus_deviation_val) {
		pr_err("cp_tune_vbus_volt down\n");
		cp_tune_vbus_volt(chip, VOLT_DOWN);
	}

	cp_check_charge_pump_enabled(chip);

	/* battery overheat, stop charge */
	if (!pm_state.charge_pump.charge_enabled) {
		return -CP_ENABLE_FAIL;
	} else if (thermal_level >= MAX_THERMAL_LEVEL || pm_state.is_temp_out_fc2_range) {
		pr_info("thermal level (%d) too high or batt temp is out of fc2 range, pm_state.is_temp_out_fc2_range:%d\n",
				thermal_level, pm_state.is_temp_out_fc2_range);
		return CP_ENABLE_FAIL;
	}

	if (chip->iterm > sys_config.fc2_taper_current) {
		chip->iterm = chip->iterm + 400;
	} else {
		chip->iterm = sys_config.fc2_taper_current;
	}

	if (pm_state.charge_pump.vbat_volt > sys_config.bat_volt_lp_lmt - 100 ||
			chip->fg_vbat > chip->fg_cv - HVDCP3_TAPER_HYS_MV) {
		if (fc2_taper_timer++ > TAPER_TIMEOUT) {
			fc2_taper_timer = 0;
			pr_err("cp taper charging done\n");
			return TAPER_DONE;
		}
	} else {
		fc2_taper_timer = 0;
	}

	return 0;
}

const unsigned char *pm_state_str[] = {
	"CP_STATE_ENTRY",
	"CP_STATE_DISCONNECT",
	"CP_STATE_SW_ENTRY",
	"CP_STATE_SW_ENTRY_2",
	"CP_STATE_SW_LOOP",
	"CP_STATE_FLASH2_ENTRY",
	"CP_STATE_FLASH2_ENTRY_1",
	"CP_STATE_FLASH2_ENTRY_3",
	"CP_STATE_FLASH2_TUNE",
	"CP_STATE_FLASH2_DELAY",
	"CP_STATE_STOP_CHARGE",
};

static void cp_move_state(pm_sm_state_t state)
{
	pr_debug("pm_state change:%s -> %s\n",
	pm_state_str[pm_state.state], pm_state_str[state]);
	pm_state.state_log[pm_state.log_idx] = pm_state.state;
	pm_state.log_idx++;
	pm_state.log_idx %= PM_STATE_LOG_MAX;
	pm_state.state = state;
}

static void cp_statemachine(struct cp_qc30_data *chip, unsigned int port)
{
	int ret;
	static int tune_vbus_retry, tune_vbus_count, retry_enable_cp_count;
	int thermal_level = 0, usb_icl_value = 0;
	static bool recovery;
	int target_vbus_volt;
	int i = 0;
	struct bq_fg_chip *bq = NULL;
	struct power_supply *psy_fg = NULL;

	psy_fg = power_supply_get_by_name("bq28z610");
	if (!IS_ERR_OR_NULL(psy_fg)) {
		bq = power_supply_get_drvdata(psy_fg);
		if (!bq)
			pr_err("%s: get fg psy drv data failed\n", __func__);
	} else {
		pr_err("%s: get fg psy failed\n", __func__);
        }

	get_usb_online();
	if (!pm_state.usb_online) {
		pm_state.state = CP_STATE_DISCONNECT;
		recovery = true;
		pr_info("vbus disconnected\n");
	} else if (pm_state.state == CP_STATE_DISCONNECT) {
		pr_info("vbus connected\n");
		recovery = true;
		pm_state.jeita_triggered = false;
		pm_state.is_temp_out_fc2_range = false;
		cp_enable_charge_pump_adc(chip, true);
		cp_move_state(CP_STATE_ENTRY);
	}

	switch (pm_state.state) {
	case CP_STATE_DISCONNECT:
		if (!pm_state.fcc_votable)
			pm_state.fcc_votable = find_votable("FCC");
		if (pm_state.fcc_votable)
			vote(pm_state.fcc_votable, QC3P5_BQ_TAPER_FCC_VOTER, false, 0);
		if (pm_state.charge_pump.charge_enabled) {
			cp_enable_charge_pump(chip, false);
			cp_check_charge_pump_enabled(chip);
		}
		if (!pm_state.sw_chager.charge_enabled || pm_state.sw_chager.charge_limited) {
			cp_reset_vbus_volt(chip);
			cp_enable_sw(chip, true);
			cp_update_sw_status(chip);
		}

		tune_vbus_count = 0;
		retry_enable_cp_count = 0;
		pm_state.usb_type = 0;
		pm_state.sw_fc2_init_fail = false;
		pm_state.sw_near_cv = false;
		sys_config.bat_curr_lp_lmt = HVDCP3_CLASS_A_BAT_CURRENT_MA;
		sys_config.bus_curr_lp_lmt = HVDCP3_CLASS_A_BUS_CURRENT_MA;
		pm_state.ibus_lmt_curr = HVDCP3_CLASS_A_BUS_CURRENT_MA;
		sys_config.ibus_minus_deviation_val = 450;
		sys_config.ibat_minus_deviation_val = 400;
		sys_config.ibus_plus_deviation_val = 100;
		sys_config.ibat_plus_deviation_val = 350;

		cp_enable_charge_pump_adc(chip, false);
		break;

	case CP_STATE_ENTRY:
		chip->qc3_work_time = 100; /* accelerate voltage regulation at the beginning of QC3.0 charging*/
		if (!pm_state.fcc_votable)
			pm_state.fcc_votable = find_votable("FCC");
		if (pm_state.fcc_votable)
			vote(pm_state.fcc_votable, QC3P5_BQ_TAPER_FCC_VOTER, false, 0);
		cp_get_usb_type(chip);
		cp_get_batt_capacity();
		qc3_get_batt_current_thermal_level(&thermal_level);
		pm_state.is_temp_out_fc2_range = qc3_disable_cp_by_jeita_status(chip);
		pr_info("is_temp_out_fc2_range:%d\n", pm_state.is_temp_out_fc2_range);

		if (pm_state.usb_type == QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3
			|| pm_state.usb_type == QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3P5) {
			pr_info("vbus_volt:%d\n", pm_state.charge_pump.vbus_volt);
			//cp_reset_vbus_volt(chip);
			msleep(100);
			if (bq && bq->irregular_batt) {
				pr_info("cp_qc3 charge stop, irregular_batt:%d\n", bq->irregular_batt);
				cp_move_state(CP_STATE_SW_ENTRY);
			}
			if (thermal_level >= MAX_THERMAL_LEVEL || pm_state.is_temp_out_fc2_range) {
				cp_move_state(CP_STATE_SW_ENTRY);
				pr_info("thermal too high or batt temp out of range or slowly charging, waiting...\n");
			} else if (pm_state.charge_pump.vbat_volt < sys_config.min_vbat_start_flash2) {
				cp_move_state(CP_STATE_SW_ENTRY);
			} else if (pm_state.charge_pump.vbat_volt > sys_config.bat_volt_lp_lmt - 250
					|| pm_state.capacity >= HIGH_CAPACITY_TRH) {
				pm_state.sw_near_cv = true;
				cp_move_state(CP_STATE_SW_ENTRY);
			} else {
				cp_move_state(CP_STATE_FLASH2_ENTRY);
			}
		}
		break;

	case CP_STATE_SW_ENTRY:
		if (!pm_state.fcc_votable)
				pm_state.fcc_votable = find_votable("FCC");
		if (pm_state.fcc_votable)
				vote(pm_state.fcc_votable, QC3P5_BQ_TAPER_FCC_VOTER,false, 0);
		//cp_reset_vbus_volt(chip);

		pr_err("vbat=%d,vbus=%d\n", pm_state.charge_pump.vbat_volt, pm_state.charge_pump.vbus_volt);
		pr_info("enable sw charger\n");
		cp_enable_sw(chip, true);
		usb_icl_value = cp_get_effective_usb_icl_val();
		if (pm_state.usb_icl_votable && (usb_icl_value < QC3_MAIN_CHARGER_ICL))
			vote(pm_state.usb_icl_votable, MAIN_CHG_VOTER, true, QC3_MAIN_CHARGER_ICL);

		if (pm_state.charge_pump.charge_enabled) {
			cp_enable_charge_pump(chip, false);
			cp_check_charge_pump_enabled(chip);
		}

		if (!pm_state.charge_pump.charge_enabled)
			cp_move_state(CP_STATE_SW_ENTRY_2);
		break;

	case CP_STATE_SW_ENTRY_2:
        pr_info("sw check enable\n");
		cp_update_sw_status(chip);
		if (pm_state.sw_chager.charge_enabled)
			cp_move_state(CP_STATE_SW_LOOP);
		break;

	case CP_STATE_SW_LOOP:
		pr_err("vbat=%d, vbus=%d\n", pm_state.charge_pump.vbat_volt, pm_state.charge_pump.vbus_volt);
		qc3_get_batt_current_thermal_level(&thermal_level);
		if (retry_enable_cp_count >= 5) {
			pr_info("retry_enable_cp_count=%d\n", retry_enable_cp_count);
			break;
		}

		pm_state.is_temp_out_fc2_range = qc3_disable_cp_by_jeita_status(chip);
		if (thermal_level < MAX_THERMAL_LEVEL && !pm_state.is_temp_out_fc2_range && recovery) {
			if (tune_vbus_count >= 2) {
				pr_info("unsupport qc3, use sw charging\n");
				break;
			}
			tune_vbus_count++;
			pr_info("thermal or batt temp recovery...\n");
			recovery = false;
		} else if (thermal_level >= MAX_THERMAL_LEVEL || pm_state.is_temp_out_fc2_range) {
			pr_info("thermal(%d) too high or batt temp out of range\n", thermal_level);
		}
		cp_get_batt_capacity();
		if (pm_state.charge_pump.vbat_volt > sys_config.bat_volt_lp_lmt - 250
					|| pm_state.capacity >= HIGH_CAPACITY_TRH) {
			pm_state.sw_near_cv = true;
		}
		if (!pm_state.sw_near_cv && !recovery) {
			if (pm_state.charge_pump.vbat_volt > sys_config.min_vbat_start_flash2) {
				pr_info("battery volt: %d is ok, proceeding to flash charging...\n",
					pm_state.charge_pump.vbat_volt);
				usb_icl_value = cp_get_effective_usb_icl_val();
				if (pm_state.usb_icl_votable && (usb_icl_value > QC3_CHARGER_ICL))
					vote(pm_state.usb_icl_votable, MAIN_CHG_VOTER, true, QC3_CHARGER_ICL);
				cp_move_state(CP_STATE_FLASH2_ENTRY);
			}
		}
		break;

	case CP_STATE_FLASH2_ENTRY:
		if (!pm_state.sw_chager.charge_limited) {
			cp_limit_sw(chip, true);
			cp_update_sw_status(chip);
		}

		if (pm_state.sw_chager.charge_limited) {
			cp_move_state(CP_STATE_FLASH2_ENTRY_1);
			tune_vbus_retry = 0;
		}

		cp_get_qc_hvdcp3_type(chip);
		if (pm_state.hvdcp3_type == QC_HVDCP3_CLASSB_27W) {
			sys_config.bat_curr_lp_lmt = HVDCP3_CLASS_B_BAT_CURRENT_MA;
			sys_config.bus_curr_lp_lmt = HVDCP3_CLASS_B_BUS_CURRENT_MA;
			pm_state.ibus_lmt_curr = sys_config.bus_curr_lp_lmt;
		} else {
			sys_config.bat_curr_lp_lmt = HVDCP3_CLASS_A_BAT_CURRENT_MA;
			sys_config.bus_curr_lp_lmt = HVDCP3_CLASS_A_BUS_CURRENT_MA;
			pm_state.ibus_lmt_curr = sys_config.bus_curr_lp_lmt;
		}
		break;

	case CP_STATE_FLASH2_ENTRY_1:
		cp_update_fc_status(chip);
		pr_err("entry1:vbat=%d,vbus=%d\n", pm_state.charge_pump.vbat_volt, pm_state.charge_pump.vbus_volt);
		target_vbus_volt = (pm_state.charge_pump.vbat_volt * debug_mask) / 10 + BUS_VOLT_INIT_UP - 50;
		if (pm_state.charge_pump.vbus_volt < target_vbus_volt) {
			if ((target_vbus_volt - pm_state.charge_pump.vbus_volt) > 2600) {
				for (i = 0; i < 15; i++) {
					cp_tune_vbus_volt(chip, VOLT_UP);
					msleep(45);
					tune_vbus_retry++;
				}
			} else {
				cp_tune_vbus_volt(chip, VOLT_UP);
				tune_vbus_retry++;
			}
		} else {
			pr_err("entry1:voltage tuned above expected voltage, retry %d times\n", tune_vbus_retry);
			cp_move_state(CP_STATE_FLASH2_ENTRY_3);
			break;
		}

		if (tune_vbus_retry > 23) {
			pr_err("entry1:Failed to tune adapter volt into valid range, charge with switching charger\n");
			pm_state.sw_fc2_init_fail = true;
			recovery = true;
			cp_move_state(CP_STATE_SW_ENTRY);
		}
		if (bq && bq->irregular_batt) {
			pr_info("cp_qc3 charge reset, irregular_batt:%d\n", bq->irregular_batt);
			cp_move_state(CP_STATE_SW_ENTRY);
		}
		break;

	case CP_STATE_FLASH2_ENTRY_3:
		if (pm_state.charge_pump.vbus_volt >
				((pm_state.charge_pump.vbat_volt * debug_mask) / 10 + BUS_VOLT_INIT_UP + 400)) {
			cp_tune_vbus_volt(chip, VOLT_DOWN);
			cp_update_fc_status(chip);
			pr_err("entry3:vbus volt is too high:%d, vbat:%d wait it down\n", pm_state.charge_pump.vbus_volt, pm_state.charge_pump.vbat_volt);
			/* voltage is too high, wait for voltage down, keep charge disabled to discharge */
		} else {
			pr_err("entry3:vbat volt is ok, enable flash charging, charge_enabled=%d\n", pm_state.charge_pump.charge_enabled);
			if (!pm_state.charge_pump.charge_enabled) {
				cp_enable_charge_pump(chip, true);
				cp_check_charge_pump_enabled(chip);
			}

			if (pm_state.charge_pump.charge_enabled) {
				if (retry_enable_cp_count > 0)
					retry_enable_cp_count = 0;
				cp_move_state(CP_STATE_FLASH2_TUNE);
				if (pm_state.usb_type == QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3P5) {
					cp_enable_sw(chip, false);
					cp_update_sw_status(chip);
				}
			} else {
				retry_enable_cp_count++;
				if (retry_enable_cp_count < 5)
					cp_move_state(CP_STATE_FLASH2_ENTRY_3);
				else
					cp_move_state(CP_STATE_SW_ENTRY);
			}
			ibus_lmt_change_timer = 0;
			fc2_taper_timer = 0;
		}
		break;

	case CP_STATE_FLASH2_TUNE:
		ret = cp_flash2_charge(chip, port);
		if (ret == -ADC_ERR) {
			pr_err("Move to stop charging:%d\n", ret);
			cp_move_state(CP_STATE_STOP_CHARGE);
			break;
		} else if (ret == -CP_ENABLE_FAIL || ret == TAPER_DONE) {
			pr_err("Move to switch charging:%d\n", ret);
			cp_move_state(CP_STATE_ENTRY);
			break;
		} else if (ret == CP_ENABLE_FAIL) {
			tune_vbus_count = 0;
			pr_err("Move to switch charging, will try to recover to flash charging:%d\n",
					ret);
			recovery = true;
			cp_move_state(CP_STATE_SW_ENTRY);
		} else {// normal tune adapter output
			cp_move_state(CP_STATE_FLASH2_DELAY);
		}
		break;

	case CP_STATE_FLASH2_DELAY:
		chip->qc3_work_time = 1000;
		cp_move_state(CP_STATE_FLASH2_TUNE);
		break;

	case CP_STATE_STOP_CHARGE:
		pr_err("Stop charging\n");
		if (pm_state.charge_pump.charge_enabled) {
			cp_enable_charge_pump(chip, false);
			cp_check_charge_pump_enabled(chip);
		}
		if (pm_state.sw_chager.charge_enabled) {
			cp_enable_sw(chip, false);
			cp_update_sw_status(chip);
		}
		break;

	default:
		pr_err("No state defined! Move to stop charging\n");
		cp_move_state(CP_STATE_STOP_CHARGE);
		break;
	}
}

static void cp_workfunc(struct work_struct *work)
{
	struct cp_qc30_data *chip = container_of(work, struct cp_qc30_data,
					qc3_pm_work.work);

	cp_get_usb_type(chip);
	cp_update_sw_status(chip);
	cp_update_fc_status(chip);
	cp_statemachine(chip, 0);

	cp_get_usb_present();

	if (pm_state.usb_type == QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3)
		schedule_delayed_work(&chip->qc3_pm_work, msecs_to_jiffies(chip->qc3_work_time));
}

static int cp_qc30_notifier_call(struct notifier_block *nb,
		unsigned long ev, void *v)
{
	struct power_supply *psy = v;
	static bool usb_hvdcp3_on;
	struct cp_qc30_data *chip = container_of(nb, struct cp_qc30_data, nb);
	static bool first_flag = true;

	if (ev != PSY_EVENT_PROP_CHANGED)
		return NOTIFY_OK;

	if (first_flag) {
		if (is_cp_chan_valid(chip, CHARGE_PUMP_SC_PRESENT)) {
			chip->bq25960_flag = false;
			first_flag = false;
			pr_err("[cp_qc_psy_notifier_cb] SC8541\n");
		} else if (is_cp_chan_valid(chip, BQ25960_IIO_CHANNEL_OFFSET)) {
			chip->bq25960_flag = true;
			first_flag = false;
			pr_err("[cp_qc_psy_notifier_cb] BQ25960\n");
		} else {
			first_flag = true;
			pr_err("qc3.0 can't find charge pump\n");
		}
	}

	if (strcmp(psy->desc->name, "usb") == 0) {
		cp_get_usb_type(chip);
		if (pm_state.usb_type == QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3 || pm_state.usb_type == QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3P5) {
			if (!delayed_work_pending(&chip->qc3_pm_work))
				schedule_delayed_work(&chip->qc3_pm_work, 3 * HZ);
			usb_hvdcp3_on = true;
		} else if (pm_state.usb_type == POWER_SUPPLY_TYPE_UNKNOWN && usb_hvdcp3_on == true) {
			cancel_delayed_work(&chip->qc3_pm_work);
			schedule_delayed_work(&chip->qc3_pm_work, 0);
			pr_info("pm_state.usb_type: %d\n", pm_state.usb_type);
			usb_hvdcp3_on = false;
		}
	}

	return NOTIFY_OK;
}

static int cp_qc30_register_notifier(struct cp_qc30_data *chip)
{
	int rc;

	chip->nb.notifier_call = cp_qc30_notifier_call;
	rc = power_supply_reg_notifier(&chip->nb);
	if (rc < 0) {
		pr_err("Couldn't register psy notifier rc = %d\n", rc);
		return rc;
	}

	return 0;
}

static int cp_qc30_parse_dt(struct cp_qc30_data *chip)
{
	struct device_node *node = chip->dev->of_node;
	int rc = 0;

	if (!node) {
		pr_err("device tree node missing\n");
		return -EINVAL;
	}

	rc = of_property_read_u32(node,
			"mi,qc3-bat-volt-max", &chip->bat_volt_max);
	if (rc < 0)
		pr_err("qc3-bat-volt-max property missing, use default val\n");
	else
		sys_config.bat_volt_lp_lmt = chip->bat_volt_max;

	rc = of_property_read_u32(node,
			"mi,qc3-ffc-bat-volt-max", &chip->ffc_bat_volt_max);
	if (rc < 0)
		pr_err("qc3-ffc-bat-volt-max property missing, use default\n");
	else
		sys_config.ffc_bat_volt_lmt = chip->ffc_bat_volt_max;

	rc = of_property_read_u32(node,
			"mi,qc3-bat-curr-max", &chip->bat_curr_max);
	if (rc < 0)
		pr_err("qc3-bat-curr-max property missing, use default val\n");
	else
		sys_config.bat_curr_lp_lmt = chip->bat_curr_max;

	rc = of_property_read_u32(node,
			"mi,qc3-bus-volt-max", &chip->bus_volt_max);
	if (rc < 0)
		pr_err("qc3-bus-volt-max property missing, use default val\n");
	else
		sys_config.bus_volt_lp_lmt = chip->bus_volt_max;

	rc = of_property_read_u32(node,
			"mi,qc3-bus-curr-max", &chip->bus_curr_max);
	if (rc < 0)
		pr_err("qc3-bus-curr-max property missing, use default val\n");
	else
		sys_config.bus_curr_lp_lmt = chip->bus_curr_max;

	chip->cp_sec_enable = of_property_read_bool(node,
				"mi,cp-sec-enable");

	sys_config.cp_sec_enable = chip->cp_sec_enable;

	return rc;
}

static int cp_qc30_iio_init(struct cp_qc30_data *chip)
{
	pr_err("cp_qc30_iio_init start\n");

	chip->cp_iio = devm_kcalloc(chip->dev,
		ARRAY_SIZE(cp_iio_chan_name), sizeof(*chip->cp_iio), GFP_KERNEL);
	if (!chip->cp_iio)
		return -ENOMEM;
	chip->smb_iio = devm_kcalloc(chip->dev,
		ARRAY_SIZE(smb_iio_chan_name), sizeof(*chip->smb_iio), GFP_KERNEL);
	if (!chip->smb_iio)
		return -ENOMEM;

	pr_err("cp_qc30_iio_init end\n");

	return 0;
}

static int cp_qc30_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct cp_qc30_data *chip;

	pr_info("%s enter\n", __func__);

	chip = devm_kzalloc(dev, sizeof(struct cp_qc30_data), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->dev = dev;
	ret = cp_qc30_parse_dt(chip);
	if (ret < 0) {
		pr_err("Couldn't parse device tree rc=%d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, chip);

	pm_state.state = CP_STATE_DISCONNECT;
	pm_state.usb_type = POWER_SUPPLY_TYPE_UNKNOWN;
	pm_state.ibus_lmt_curr = sys_config.bus_curr_lp_lmt;
	chip->qc3_work_time = 100;

	cp_qc30_iio_init(chip);

	INIT_DELAYED_WORK(&chip->qc3_pm_work, cp_workfunc);

	cp_qc30_register_notifier(chip);
	pr_info("charge pump qc3 probe success\n");

	return ret;
}

static int cp_qc30_remove(struct platform_device *pdev)
{
	struct cp_qc30_data *chip = platform_get_drvdata(pdev);

	cancel_delayed_work(&chip->qc3_pm_work);

	return 0;
}

static const struct of_device_id cp_qc30_of_match[] = {
	{ .compatible = "xiaomi,cp-qc30", },
	{},
};

static struct platform_driver cp_qc30_driver = {
	.driver = {
		.name = "cp-qc30",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(cp_qc30_of_match),
	},
	.probe = cp_qc30_probe,
	.remove = cp_qc30_remove,
};

static int __init cp_qc30_init(void)
{
	return platform_driver_register(&cp_qc30_driver);
}

late_initcall(cp_qc30_init);

static void __exit cp_qc30_exit(void)
{
	return platform_driver_unregister(&cp_qc30_driver);
}
module_exit(cp_qc30_exit);

MODULE_LICENSE("GPL");
