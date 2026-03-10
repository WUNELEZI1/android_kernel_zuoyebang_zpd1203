// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */
/* ADD: AudioSwitch_Bringup */
#define DEBUG 1
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/regmap.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/usb/typec.h>
#include <linux/usb/ucsi_glink.h>
#include <linux/soc/qcom/fsa4480-i2c.h>
#include <linux/iio/consumer.h>
#include <linux/qti-regmap-debugfs.h>

#define FSA4480_I2C_NAME	"fsa4480-driver"

/* ADD: AudioSwitch_Bringup */
#define CONFIG_LCT_AUDIO_INFO   1
#define AUDIO_INFO_MAX_LEN     (64)
#define HL5281M_ID1_VALUE       0x40
#define HL5281M_ID2_VALUE       0x41
#define DIO4480_ID_VALUE        0xF1
#define DIO4485_ID_VALUE        0xF6
/* END AudioSwitch_Bringup */

#define FSA4480_DEV_ID          0x00
#define FSA4480_SWITCH_SETTINGS 0x04
#define FSA4480_SWITCH_CONTROL  0x05
#define FSA4480_SWITCH_STATUS1  0x07
#define FSA4480_SLOW_L          0x08
#define FSA4480_SLOW_R          0x09
#define FSA4480_SLOW_MIC        0x0A
#define FSA4480_SLOW_SENSE      0x0B
#define FSA4480_SLOW_GND        0x0C
#define FSA4480_DELAY_L_R       0x0D
#define FSA4480_DELAY_L_MIC     0x0E
#define FSA4480_DELAY_L_SENSE   0x0F
#define FSA4480_DELAY_L_AGND    0x10
#define FSA4480_FUNC_ENABLE     0x12  /* ADD: AudioSwitch_Bringup */
#if defined(CONFIG_RUST_DETECTION)
#define FSA4480_RES_DET_PIN       0x31
#define FSA4480_RES_SELECT_MODE   0x13
#define FSA4480_RES_DET_THRESHOLD 0x15
#define FSA4480_RES_DET_VAL       0x14
#define FSA4480_RES_DET_INTERVAL  0x16
#define FSA4480_DETECTION_INT     0x18
#define FSA4480_DET_V_TH          0x32
#define FSA4480_DETM_V_CCIN       0x38
#define FSA4480_DETM_V_SBU1       0x3B
#define FSA4480_DETM_V_SBU2       0x3C
#define FSA4480_V_MULTIPLE        9375 //二供转换电压倍数，（val *9375）/ 1000
#define FSA4480_V_MULTIPLE_2      1000
#define FSA4480_V_CCIN_TH         2000 //2V
#define FSA4480_V_SBU1_TH         500  //0.5V
#define FSA4480_V_SBU2_TH         500  //0.5V
#endif

#define FSA4480_RESET           0x1E

/* ADD: AudioSwitch_Bringup */
#define DIO4485_MIC_DET_STATUS  0x17
#define DIO4485_DELAY_T_SETTING 0x21
#define DIO4485_FREQ1_SETTING   0x4E
#define DIO4485_FREQ2_SETTING   0x50
#define DIO4485_FREQ3_SETTING   0x51
/* END AudioSwitch_Bringup */

struct fsa4480_priv {
	struct regmap *regmap;
	struct device *dev;
	struct power_supply *usb_psy;
	struct notifier_block nb;
	struct iio_channel *iio_ch;
	atomic_t usbc_mode;
	struct work_struct usbc_analog_work;
	struct blocking_notifier_head fsa4480_notifier;
	struct mutex notification_lock;
	u32 use_powersupply;
	u32 dev_id; /* ADD: AudioSwitch_Bringup */
};

#if defined(CONFIG_RUST_DETECTION)
static struct fsa4480_priv *global_fsa4480_data = NULL;
#endif

struct fsa4480_reg_val {
	u16 reg;
	u8 val;
};

static const struct regmap_config fsa4480_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
#if defined(CONFIG_RUST_DETECTION)
	.max_register = FSA4480_DETM_V_SBU2, //修改max寄存器值，不作修改的话如果注册的值超过这个值就不允许写入
#else
	.max_register = FSA4480_RESET,
#endif
};

static const struct fsa4480_reg_val fsa_reg_i2c_defaults[] = {
	{FSA4480_SLOW_L, 0x00},
	{FSA4480_SLOW_R, 0x00},
	{FSA4480_SLOW_MIC, 0x00},
	{FSA4480_SLOW_SENSE, 0x00},
	{FSA4480_SLOW_GND, 0x00},
	{FSA4480_DELAY_L_R, 0x00},
	{FSA4480_DELAY_L_MIC, 0x00},
	{FSA4480_DELAY_L_SENSE, 0x00},
	{FSA4480_DELAY_L_AGND, 0x09},
	{FSA4480_SWITCH_SETTINGS, 0x98},
};

/* ADD: AudioSwitch_Bringup */
#ifdef CONFIG_LCT_AUDIO_INFO
static char g_audio_switch_name[AUDIO_INFO_MAX_LEN];
int fsa4480_get_name(char *audio_switch_name)
{
	strncpy(audio_switch_name, g_audio_switch_name, strlen(g_audio_switch_name));
	return 0;
}
EXPORT_SYMBOL(fsa4480_get_name);
static int g_fsa4480_usbhs_state;
int fsa4480_flip_event(int *fsa4480_usbhs_state)
{
	*fsa4480_usbhs_state = g_fsa4480_usbhs_state;
	return 0;
}
EXPORT_SYMBOL(fsa4480_flip_event);
#endif
/* END AudioSwitch_Bringup */

static void fsa4480_usbc_update_settings(struct fsa4480_priv *fsa_priv,
		u32 switch_control, u32 switch_enable)
{
	u32 prev_control, prev_enable, cur_control, micdet_status; /* ADD: AudioSwitch_Bringup */

	if (!fsa_priv->regmap) {
		dev_err(fsa_priv->dev, "%s: regmap invalid\n", __func__);
		return;
	}

	regmap_read(fsa_priv->regmap, FSA4480_SWITCH_CONTROL, &prev_control);
	regmap_read(fsa_priv->regmap, FSA4480_SWITCH_SETTINGS, &prev_enable);
	if (prev_control == switch_control && prev_enable == switch_enable) {
		dev_dbg(fsa_priv->dev, "%s: settings unchanged\n", __func__);
		return;
	}

	/* ADD: AudioSwitch_Bringup */
	if (switch_enable == 0x98) {
		//regmap_write(fsa_priv->regmap, FSA4480_SWITCH_SETTINGS, 0x80); /* ADD: AudioSwitch_Bringup:NA */
		regmap_write(fsa_priv->regmap, FSA4480_SWITCH_CONTROL, switch_control);
		/* FSA4480 chip hardware requirement */
		usleep_range(50, 55);
		regmap_write(fsa_priv->regmap, FSA4480_SWITCH_SETTINGS, switch_enable);
		usleep_range(5000, 5500);
		g_fsa4480_usbhs_state = 0;
	} else if (switch_enable == 0x9F) {
		if (prev_control == 0x18) {
			regmap_write(fsa_priv->regmap, FSA4480_SWITCH_SETTINGS, switch_enable);
			regmap_write(fsa_priv->regmap, FSA4480_SWITCH_CONTROL, switch_control);
			regmap_write(fsa_priv->regmap, FSA4480_FUNC_ENABLE, 0x49);
			if (DIO4485_ID_VALUE == fsa_priv->dev_id) {
				usleep_range(350000, 351000);
			} else if ((HL5281M_ID1_VALUE == fsa_priv->dev_id) || (HL5281M_ID2_VALUE == fsa_priv->dev_id)) {
				usleep_range(50000, 51000);
			}
		} else {
			dev_dbg(fsa_priv->dev, "%s: Reverse insertion settings unchanged\n", __func__);
			if ((HL5281M_ID1_VALUE == fsa_priv->dev_id) || (HL5281M_ID2_VALUE == fsa_priv->dev_id)) {
				regmap_write(fsa_priv->regmap, FSA4480_SWITCH_SETTINGS, switch_enable);
				regmap_write(fsa_priv->regmap, FSA4480_SWITCH_CONTROL, switch_control);
				usleep_range(5000, 5500);
			}
		}
		regmap_read(fsa_priv->regmap, DIO4485_MIC_DET_STATUS, &micdet_status);
		if (DIO4485_ID_VALUE == fsa_priv->dev_id) {
			if (micdet_status == 0x1) {
				dev_dbg(fsa_priv->dev, "%s: The headset is not detected. Please detect it again.\n", __func__);
				if (switch_control == 0x0) {
					regmap_write(fsa_priv->regmap, FSA4480_RESET, 0x01);
					usleep_range(5000, 5500);
					regmap_write(fsa_priv->regmap, DIO4485_FREQ1_SETTING, 0x8F);
					regmap_write(fsa_priv->regmap, DIO4485_FREQ1_SETTING, 0x5A);
					regmap_write(fsa_priv->regmap, DIO4485_FREQ3_SETTING, 0x90);
					regmap_write(fsa_priv->regmap, DIO4485_FREQ2_SETTING, 0x45);
					usleep_range(5000, 5500);
					regmap_write(fsa_priv->regmap, FSA4480_DELAY_L_SENSE, 0x00);
					regmap_write(fsa_priv->regmap, FSA4480_DELAY_L_AGND, 0x00);
					regmap_write(fsa_priv->regmap, FSA4480_DELAY_L_MIC, 0x1F);
					regmap_write(fsa_priv->regmap, FSA4480_DELAY_L_R, 0x2F);
					regmap_write(fsa_priv->regmap, DIO4485_DELAY_T_SETTING, 0x2F);
					regmap_write(fsa_priv->regmap, FSA4480_FUNC_ENABLE, 0x08);
				}
				regmap_write(fsa_priv->regmap, FSA4480_SWITCH_CONTROL, switch_control);
				regmap_write(fsa_priv->regmap, FSA4480_SWITCH_SETTINGS, switch_enable);
				usleep_range(5000, 5500);
			}
		} else if ((HL5281M_ID1_VALUE == fsa_priv->dev_id) || (HL5281M_ID2_VALUE == fsa_priv->dev_id)) {
			if (micdet_status == 0x2) {
				regmap_write(fsa_priv->regmap, FSA4480_SWITCH_SETTINGS, switch_enable);
			} else if (micdet_status == 0x1) {
				dev_dbg(fsa_priv->dev, "%s: The headset is not detected. Please detect it again.\n", __func__);
				if (switch_control == 0x0) {
					regmap_write(fsa_priv->regmap, FSA4480_RESET, 0x01);
					usleep_range(5000, 5500);
					regmap_write(fsa_priv->regmap, FSA4480_FUNC_ENABLE, 0x48);
				}
				regmap_write(fsa_priv->regmap, FSA4480_SWITCH_CONTROL, switch_control);
				regmap_write(fsa_priv->regmap, FSA4480_SWITCH_SETTINGS, switch_enable);
				usleep_range(5000, 5500);
			}
		}
		regmap_read(fsa_priv->regmap, FSA4480_SWITCH_CONTROL, &cur_control);
		if (cur_control == 0x0) {
			g_fsa4480_usbhs_state = 1;
		} else if (cur_control == 0x7) {
			g_fsa4480_usbhs_state = 2;
		}
	}
	/* END AudioSwitch_Bringup */
}

static int fsa4480_usbc_event_changed_psupply(struct fsa4480_priv *fsa_priv,
				      unsigned long evt, void *ptr)
{
	struct device *dev = NULL;

	if (!fsa_priv)
		return -EINVAL;

	dev = fsa_priv->dev;
	if (!dev)
		return -EINVAL;
	if (fsa_priv->usb_psy && fsa_priv->usb_psy->desc && strcmp(fsa_priv->usb_psy->desc->name, "usb") == 0) {
		dev_dbg(dev, "%s: queueing usbc_analog_work\n",
			__func__);
		pm_stay_awake(fsa_priv->dev);
		queue_work(system_freezable_wq, &fsa_priv->usbc_analog_work);
	}

	return 0;
}

static int fsa4480_usbc_event_changed_ucsi(struct fsa4480_priv *fsa_priv,
				      unsigned long evt, void *ptr)
{
	struct device *dev;
	enum typec_accessory acc = ((struct ucsi_glink_constat_info *)ptr)->acc;

	if (!fsa_priv)
		return -EINVAL;

	dev = fsa_priv->dev;
	if (!dev)
		return -EINVAL;

	dev_dbg(dev, "%s: USB change event received, supply mode %d, usbc mode %ld, expected %d\n",
			__func__, acc, fsa_priv->usbc_mode.counter,
			TYPEC_ACCESSORY_AUDIO);

	switch (acc) {
	case TYPEC_ACCESSORY_AUDIO:
	case TYPEC_ACCESSORY_NONE:
		if (atomic_read(&(fsa_priv->usbc_mode)) == acc)
			break; /* filter notifications received before */
		atomic_set(&(fsa_priv->usbc_mode), acc);

		dev_dbg(dev, "%s: queueing usbc_analog_work\n",
			__func__);
		pm_stay_awake(fsa_priv->dev);
		queue_work(system_freezable_wq, &fsa_priv->usbc_analog_work);
		break;
	default:
		break;
	}

	return 0;
}

static int fsa4480_usbc_event_changed(struct notifier_block *nb_ptr,
				      unsigned long evt, void *ptr)
{
	struct fsa4480_priv *fsa_priv =
			container_of(nb_ptr, struct fsa4480_priv, nb);
	struct device *dev;

	if (!fsa_priv)
		return -EINVAL;

	dev = fsa_priv->dev;
	if (!dev)
		return -EINVAL;

	if (fsa_priv->use_powersupply)
		return fsa4480_usbc_event_changed_psupply(fsa_priv, evt, ptr);
	else
		return fsa4480_usbc_event_changed_ucsi(fsa_priv, evt, ptr);
}

static int fsa4480_usbc_analog_setup_switches_psupply(
						struct fsa4480_priv *fsa_priv)
{
	int rc = 0;
	union power_supply_propval mode;
	struct device *dev;

	if (!fsa_priv)
		return -EINVAL;
	dev = fsa_priv->dev;
	if (!dev)
		return -EINVAL;

	rc = iio_read_channel_processed(fsa_priv->iio_ch, &mode.intval);

	mutex_lock(&fsa_priv->notification_lock);
	/* get latest mode again within locked context */
	if (rc < 0) {
		dev_err(dev, "%s: Unable to read USB TYPEC_MODE: %d\n",
			__func__, rc);
		goto done;
	}

	/* ADD: AudioSwitch_Bringup */
	if (atomic_read(&(fsa_priv->usbc_mode)) == mode.intval)
		goto done; /* filter notifications received before */
	/* END AudioSwitch_Bringup */
	atomic_set(&(fsa_priv->usbc_mode), mode.intval);

	switch (mode.intval) {
	/* add all modes FSA should notify for in here */
	case TYPEC_ACCESSORY_AUDIO:
		/* ADD: AudioSwitch_Bringup */
		dev_dbg(dev, "%s: Audio Plug In \n", __func__);
		if (DIO4485_ID_VALUE == fsa_priv->dev_id) {
			regmap_write(fsa_priv->regmap, FSA4480_RESET, 0x01);
			msleep(5);
			regmap_write(fsa_priv->regmap, DIO4485_FREQ1_SETTING, 0x8F);
			regmap_write(fsa_priv->regmap, DIO4485_FREQ1_SETTING, 0x5A);
			regmap_write(fsa_priv->regmap, DIO4485_FREQ3_SETTING, 0x90);
			regmap_write(fsa_priv->regmap, DIO4485_FREQ2_SETTING, 0x45);
			regmap_write(fsa_priv->regmap, FSA4480_DELAY_L_SENSE, 0x00);
			regmap_write(fsa_priv->regmap, FSA4480_DELAY_L_AGND, 0x00);
			regmap_write(fsa_priv->regmap, FSA4480_DELAY_L_MIC, 0x1F);
			regmap_write(fsa_priv->regmap, FSA4480_DELAY_L_R, 0x2F);
			regmap_write(fsa_priv->regmap, DIO4485_DELAY_T_SETTING, 0x2F);
		}
		/* END AudioSwitch_Bringup */
		/* activate switches */
		fsa4480_usbc_update_settings(fsa_priv, 0x00, 0x9F);

		/* notify call chain on event */
		blocking_notifier_call_chain(&fsa_priv->fsa4480_notifier,
		mode.intval, NULL);
		break;
	case TYPEC_ACCESSORY_NONE:
		/* notify call chain on event */
		blocking_notifier_call_chain(&fsa_priv->fsa4480_notifier,
				TYPEC_ACCESSORY_NONE, NULL);

		/* deactivate switches */
		/* ADD: AudioSwitch_Bringup */
		dev_dbg(dev, "%s: Audio Plug Out \n", __func__);
		if ((HL5281M_ID1_VALUE == fsa_priv->dev_id) || (HL5281M_ID2_VALUE == fsa_priv->dev_id)) {
			regmap_write(fsa_priv->regmap, FSA4480_FUNC_ENABLE, 0x48);
		}
		/* END AudioSwitch_Bringup */
		fsa4480_usbc_update_settings(fsa_priv, 0x18, 0x98);
		break;
	default:
		/* ignore other usb connection modes */
		break;
	}

done:
	mutex_unlock(&fsa_priv->notification_lock);
	return rc;
}

static int fsa4480_usbc_analog_setup_switches_ucsi(
						struct fsa4480_priv *fsa_priv)
{
	int rc = 0;
	int mode;
	struct device *dev;

	if (!fsa_priv)
		return -EINVAL;
	dev = fsa_priv->dev;
	if (!dev)
		return -EINVAL;

	mutex_lock(&fsa_priv->notification_lock);
	/* get latest mode again within locked context */
	mode = atomic_read(&(fsa_priv->usbc_mode));

	dev_dbg(dev, "%s: setting GPIOs active = %d\n",
		__func__, mode != TYPEC_ACCESSORY_NONE);

	switch (mode) {
	/* add all modes FSA should notify for in here */
	case TYPEC_ACCESSORY_AUDIO:
		/* activate switches */
		fsa4480_usbc_update_settings(fsa_priv, 0x00, 0x9F);

		/* notify call chain on event */
		blocking_notifier_call_chain(&fsa_priv->fsa4480_notifier,
					     mode, NULL);
		break;
	case TYPEC_ACCESSORY_NONE:
		/* notify call chain on event */
		blocking_notifier_call_chain(&fsa_priv->fsa4480_notifier,
				TYPEC_ACCESSORY_NONE, NULL);

		/* deactivate switches */
		fsa4480_usbc_update_settings(fsa_priv, 0x18, 0x98);
		break;
	default:
		/* ignore other usb connection modes */
		break;
	}

	mutex_unlock(&fsa_priv->notification_lock);
	return rc;
}

static int fsa4480_usbc_analog_setup_switches(struct fsa4480_priv *fsa_priv)
{
	if (fsa_priv->use_powersupply)
		return fsa4480_usbc_analog_setup_switches_psupply(fsa_priv);
	else
		return fsa4480_usbc_analog_setup_switches_ucsi(fsa_priv);
}

/*
 * fsa4480_reg_notifier - register notifier block with fsa driver
 *
 * @nb - notifier block of fsa4480
 * @node - phandle node to fsa4480 device
 *
 * Returns 0 on success, or error code
 */
int fsa4480_reg_notifier(struct notifier_block *nb,
			 struct device_node *node)
{
	int rc = 0;
	struct i2c_client *client = of_find_i2c_device_by_node(node);
	struct fsa4480_priv *fsa_priv;

	if (!client)
		return -EINVAL;

	fsa_priv = (struct fsa4480_priv *)i2c_get_clientdata(client);
	if (!fsa_priv)
		return -EINVAL;

	rc = blocking_notifier_chain_register
				(&fsa_priv->fsa4480_notifier, nb);

	/* ADD: AudioSwitch_Bringup */
	dev_dbg(fsa_priv->dev, "%s: registered notifier for %s, addr 0x%x, rc %d\n",
		__func__, node->name, client->addr, rc);
	/* END AudioSwitch_Bringup */
	if (rc)
		return rc;

	/*
	 * as part of the init sequence check if there is a connected
	 * USB C analog adapter
	 */
	if (atomic_read(&(fsa_priv->usbc_mode)) == TYPEC_ACCESSORY_AUDIO) {
		dev_dbg(fsa_priv->dev, "%s: analog adapter already inserted\n",
			__func__);
		/* ADD: AudioSwitch_Bringup */
		atomic_set(&(fsa_priv->usbc_mode), TYPEC_ACCESSORY_NONE);
		/* END AudioSwitch_Bringup */
		rc = fsa4480_usbc_analog_setup_switches(fsa_priv);
	}

	return rc;
}
EXPORT_SYMBOL_GPL(fsa4480_reg_notifier);

/*
 * fsa4480_unreg_notifier - unregister notifier block with fsa driver
 *
 * @nb - notifier block of fsa4480
 * @node - phandle node to fsa4480 device
 *
 * Returns 0 on pass, or error code
 */
int fsa4480_unreg_notifier(struct notifier_block *nb,
			     struct device_node *node)
{
	int rc = 0;
	struct i2c_client *client = of_find_i2c_device_by_node(node);
	struct fsa4480_priv *fsa_priv;
	struct device *dev;
	union power_supply_propval mode;

	if (!client)
		return -EINVAL;

	fsa_priv = (struct fsa4480_priv *)i2c_get_clientdata(client);
	if (!fsa_priv)
		return -EINVAL;
	if (fsa_priv->use_powersupply) {
		dev = fsa_priv->dev;
		if (!dev)
			return -EINVAL;

		mutex_lock(&fsa_priv->notification_lock);
		/* get latest mode within locked context */

		rc = iio_read_channel_processed(fsa_priv->iio_ch, &mode.intval);

		if (rc < 0) {
			dev_dbg(dev, "%s: Unable to read USB TYPEC_MODE: %d\n",
				__func__, rc);
			mutex_unlock(&fsa_priv->notification_lock);
			return rc;
		}
		/* Do not reset switch settings for usb digital hs */
		if (mode.intval == TYPEC_ACCESSORY_AUDIO)
			fsa4480_usbc_update_settings(fsa_priv, 0x18, 0x98);
		rc = blocking_notifier_chain_unregister
					(&fsa_priv->fsa4480_notifier, nb);
		mutex_unlock(&fsa_priv->notification_lock);
	} else {
		fsa4480_usbc_update_settings(fsa_priv, 0x18, 0x98);
		rc = blocking_notifier_chain_unregister
				(&fsa_priv->fsa4480_notifier, nb);
	}
	return rc;
}
EXPORT_SYMBOL_GPL(fsa4480_unreg_notifier);

static int fsa4480_validate_display_port_settings(struct fsa4480_priv *fsa_priv)
{
	u32 switch_status = 0;

	regmap_read(fsa_priv->regmap, FSA4480_SWITCH_STATUS1, &switch_status);

	if ((switch_status != 0x23) && (switch_status != 0x1C)) {
		pr_err("AUX SBU1/2 switch status is invalid = %u\n",
				switch_status);
		return -EIO;
	}

	return 0;
}
/*
 * fsa4480_switch_event - configure FSA switch position based on event
 *
 * @node - phandle node to fsa4480 device
 * @event - fsa_function enum
 *
 * Returns int on whether the switch happened or not
 */
int fsa4480_switch_event(struct device_node *node,
			 enum fsa_function event)
{
	int switch_control = 0;
	struct i2c_client *client = of_find_i2c_device_by_node(node);
	struct fsa4480_priv *fsa_priv;

	if (!client)
		return -EINVAL;

	fsa_priv = (struct fsa4480_priv *)i2c_get_clientdata(client);
	if (!fsa_priv)
		return -EINVAL;
	if (!fsa_priv->regmap)
		return -EINVAL;

	switch (event) {
	case FSA_MIC_GND_SWAP:
		regmap_read(fsa_priv->regmap, FSA4480_SWITCH_CONTROL,
				&switch_control);
		if ((switch_control & 0x07) == 0x07)
			switch_control = 0x0;
		else
			switch_control = 0x7;
		fsa4480_usbc_update_settings(fsa_priv, switch_control, 0x9F);
		return 1; /* ADD: AudioSwitch_Bringup */
	case FSA_USBC_ORIENTATION_CC1:
		fsa4480_usbc_update_settings(fsa_priv, 0x18, 0xF8);
		return fsa4480_validate_display_port_settings(fsa_priv);
	case FSA_USBC_ORIENTATION_CC2:
		fsa4480_usbc_update_settings(fsa_priv, 0x78, 0xF8);
		return fsa4480_validate_display_port_settings(fsa_priv);
	case FSA_USBC_DISPLAYPORT_DISCONNECTED:
		fsa4480_usbc_update_settings(fsa_priv, 0x18, 0x98);
		break;
	default:
		break;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(fsa4480_switch_event);

static void fsa4480_usbc_analog_work_fn(struct work_struct *work)
{
	struct fsa4480_priv *fsa_priv =
		container_of(work, struct fsa4480_priv, usbc_analog_work);

	if (!fsa_priv) {
		pr_err("%s: fsa container invalid\n", __func__);
		return;
	}
	fsa4480_usbc_analog_setup_switches(fsa_priv);
	pm_relax(fsa_priv->dev);
}

static void fsa4480_update_reg_defaults(struct regmap *regmap)
{
	u8 i;

	for (i = 0; i < ARRAY_SIZE(fsa_reg_i2c_defaults); i++)
		regmap_write(regmap, fsa_reg_i2c_defaults[i].reg,
				   fsa_reg_i2c_defaults[i].val);
}

#if defined(CONFIG_RUST_DETECTION)
int rust_detection_workfunc_open(void)
{
	struct fsa4480_priv *fsa_priv = global_fsa4480_data;
	int reg_val_enable = 0, reg_val_pin = 0, reg_val_interval = 0, reg_val_select_mode = 0;
	int reg_val_ccin = 0, reg_val_sbu1 = 0, reg_val_sbu2 = 0;
	int val_result = 0, reg_val_det_int = 0, count_retry = 10, ret = 0;
	struct device *dev = NULL;

	if (!fsa_priv) {
		pr_err("%s: [UST DET] Failed because fsa4480_data is NULL\n", __func__);
		return -EINVAL;
	}

	dev = fsa_priv->dev;
	if (!dev) {
		pr_err("%s: [UST DET] dev is NULL\n", __func__);
		return -EINVAL;
	}

	if ((HL5281M_ID1_VALUE == fsa_priv->dev_id) || (HL5281M_ID2_VALUE == fsa_priv->dev_id) || (DIO4485_ID_VALUE == fsa_priv->dev_id)) {
		dev_dbg(dev, "%s: dev_id = 0x%x\n", __func__, fsa_priv->dev_id);
	} else {
		dev_err(dev, "%s: dev_id not matched\n", __func__);
		return -EINVAL;
	}

	ret |= regmap_read(fsa_priv->regmap, FSA4480_RES_DET_INTERVAL, &reg_val_interval);
	ret |= regmap_write(fsa_priv->regmap, FSA4480_RES_DET_INTERVAL, (0x4 | reg_val_interval)); //选择multi-pin mode， bit<2> = 1

	ret |= regmap_read(fsa_priv->regmap, FSA4480_RES_SELECT_MODE, &reg_val_select_mode);
	ret |= regmap_write(fsa_priv->regmap, FSA4480_RES_SELECT_MODE, (0x8 | reg_val_select_mode)); //选择电压模式，bit<3> = 1

	ret |= regmap_read(fsa_priv->regmap, FSA4480_RES_DET_PIN, &reg_val_pin);
	ret |= regmap_write(fsa_priv->regmap, FSA4480_RES_DET_PIN, (0x13 | reg_val_pin)); //选择需要检测的pin，bit<4>,bit<1>,bit<0> = 1

	ret |= regmap_write(fsa_priv->regmap, FSA4480_DET_V_TH, 0xFF); //阈值设为最大，所有pin都会检测

	ret |= regmap_read(fsa_priv->regmap, FSA4480_DETECTION_INT, &reg_val_det_int); //0x18 清除状态

	ret |= regmap_read(fsa_priv->regmap, FSA4480_FUNC_ENABLE, &reg_val_enable);
	ret |= regmap_write(fsa_priv->regmap, FSA4480_FUNC_ENABLE, (0x2 | reg_val_enable)); //打开检测使能，bit<1> = 1

	msleep(50);
	while (count_retry) {
		count_retry--;

		ret |= regmap_read(fsa_priv->regmap, FSA4480_DETECTION_INT, &reg_val_det_int);
		if (reg_val_det_int & 0x1) {
			break;
		} else {
			dev_err(dev, "%s: detection not complete, reg_val_det_int = 0x%x\n", __func__, reg_val_det_int);
			return -EINVAL;
		}
		if (count_retry == 0 || ret)
			break;
		mdelay(5);
	}
	ret |= regmap_read(fsa_priv->regmap, FSA4480_DETM_V_CCIN, &reg_val_ccin); //0x38的值
	ret |= regmap_read(fsa_priv->regmap, FSA4480_DETM_V_SBU1, &reg_val_sbu1); //0x3B的值
	ret |= regmap_read(fsa_priv->regmap, FSA4480_DETM_V_SBU2, &reg_val_sbu2); //0x3C的值
	if (ret) {
		dev_err(dev, "%s: get reg failed, ret = %d\n", __func__, ret);
		return -EINVAL;
	}
	if ((HL5281M_ID1_VALUE == fsa_priv->dev_id) || (HL5281M_ID2_VALUE == fsa_priv->dev_id)) {
		if (((((reg_val_ccin + 1) * FSA4480_V_MULTIPLE) / FSA4480_V_MULTIPLE_2) > FSA4480_V_CCIN_TH)
				&& ((((reg_val_sbu1 + 1) * FSA4480_V_MULTIPLE) / FSA4480_V_MULTIPLE_2) > FSA4480_V_SBU1_TH)
				&& ((((reg_val_sbu2 + 1) * FSA4480_V_MULTIPLE) / FSA4480_V_MULTIPLE_2) > FSA4480_V_SBU2_TH)) {
			dev_dbg(dev, "%s: HL5281M Moisture occur\n", __func__);
			val_result = 1;
			return val_result;
		}
	} else if (DIO4485_ID_VALUE == fsa_priv->dev_id) {
		if (((reg_val_ccin * 9) > FSA4480_V_CCIN_TH)
				&& ((reg_val_sbu1 * 9) > FSA4480_V_SBU1_TH)
				&& ((reg_val_sbu2 * 9) > FSA4480_V_SBU2_TH)) {
			dev_dbg(dev, "%s: DIO4485 Moisture occur\n", __func__);
			val_result = 1;
			return val_result;
		}
	}

	dev_dbg(dev, "%s: Moisture doesn’t occur\n", __func__);

	return val_result;
} EXPORT_SYMBOL_GPL(rust_detection_workfunc_open);
int rust_detection_workfunc_close(void)
{
	struct fsa4480_priv *fsa_priv = global_fsa4480_data;
	int reg_val_enable = 0, ret = 0;
	struct device *dev = NULL;

	if (!fsa_priv) {
		pr_err("%s: [UST DET] Failed because fsa4480_data is NULL\n", __func__);
		return -EINVAL;
	}

	dev = fsa_priv->dev;
	if (!dev) {
		pr_err("%s: [UST DET] dev is NULL\n", __func__);
		return -EINVAL;
	}

	if ((HL5281M_ID1_VALUE == fsa_priv->dev_id) || (HL5281M_ID2_VALUE == fsa_priv->dev_id) || (DIO4485_ID_VALUE == fsa_priv->dev_id)) {
		dev_dbg(dev, "%s: dev_id = 0x%x\n", __func__, fsa_priv->dev_id);
	} else {
		dev_err(dev, "%s: dev_id not matched\n", __func__);
		return -EINVAL;
	}

	ret |= regmap_read(fsa_priv->regmap, FSA4480_FUNC_ENABLE, &reg_val_enable);
	ret |= regmap_write(fsa_priv->regmap, FSA4480_FUNC_ENABLE, (0xFD & reg_val_enable)); //关闭检测使能，bit<1> = 0

	if (ret) {
		dev_err(dev, "%s: get reg failed, ret = %d\n", __func__, ret);
		return -EINVAL;
	}
	dev_dbg(dev, "%s: Moisture detection off\n", __func__);

	return 0;
} EXPORT_SYMBOL_GPL(rust_detection_workfunc_close);
#endif

static int fsa4480_probe(struct i2c_client *i2c,
			 const struct i2c_device_id *id)
{
	struct fsa4480_priv *fsa_priv;
	u32 use_powersupply = 0;
	int rc = 0;

	fsa_priv = devm_kzalloc(&i2c->dev, sizeof(*fsa_priv),
				GFP_KERNEL);
	if (!fsa_priv)
		return -ENOMEM;

	memset(fsa_priv, 0, sizeof(struct fsa4480_priv));
	fsa_priv->dev = &i2c->dev;

	fsa_priv->regmap = devm_regmap_init_i2c(i2c, &fsa4480_regmap_config);
	if (IS_ERR_OR_NULL(fsa_priv->regmap)) {
		dev_err(fsa_priv->dev, "%s: Failed to initialize regmap: %d\n",
			__func__, rc);
		if (!fsa_priv->regmap) {
			rc = -EINVAL;
			goto err_data;
		}
		rc = PTR_ERR(fsa_priv->regmap);
		goto err_data;
	}

	fsa4480_update_reg_defaults(fsa_priv->regmap);
	devm_regmap_qti_debugfs_register(fsa_priv->dev, fsa_priv->regmap);

	INIT_WORK(&fsa_priv->usbc_analog_work,
		  fsa4480_usbc_analog_work_fn);

	fsa_priv->nb.notifier_call = fsa4480_usbc_event_changed;
	fsa_priv->nb.priority = 0;
	rc = of_property_read_u32(fsa_priv->dev->of_node,
			"qcom,use-power-supply", &use_powersupply);
	if (rc || use_powersupply == 0) {
		dev_dbg(fsa_priv->dev,
			"%s: Looking up %s property failed or disabled\n",
			__func__, "qcom,use-power-supply");

		fsa_priv->use_powersupply = 0;
		rc = register_ucsi_glink_notifier(&fsa_priv->nb);
		if (rc) {
			dev_err(fsa_priv->dev,
			  "%s: ucsi glink notifier registration failed: %d\n",
			  __func__, rc);
			goto err_data;
		}
	} else {
		fsa_priv->use_powersupply = 1;
		fsa_priv->usb_psy = power_supply_get_by_name("usb");
		if (!fsa_priv->usb_psy) {
			rc = -EPROBE_DEFER;
			dev_dbg(fsa_priv->dev,
				"%s: could not get USB psy info: %d\n",
				__func__, rc);
			goto err_data;
		}

		fsa_priv->iio_ch = iio_channel_get(fsa_priv->dev, "typec_mode");
		if (!fsa_priv->iio_ch) {
			dev_err(fsa_priv->dev,
				"%s: iio_channel_get failed for typec_mode\n",
				__func__);
			goto err_supply;
		}
		rc = power_supply_reg_notifier(&fsa_priv->nb);
		if (rc) {
			dev_err(fsa_priv->dev,
				"%s: power supply reg failed: %d\n",
			__func__, rc);
			goto err_supply;
		}
	}

	/* ADD: AudioSwitch_Bringup */
	rc = regmap_read(fsa_priv->regmap, FSA4480_DEV_ID, &fsa_priv->dev_id);
#ifdef CONFIG_LCT_AUDIO_INFO
	strcpy(g_audio_switch_name, dev_name(&i2c->dev));
	if (rc) {
		strcat(g_audio_switch_name, "-fail");
	} else {
		if ((HL5281M_ID1_VALUE == fsa_priv->dev_id) || (HL5281M_ID2_VALUE == fsa_priv->dev_id)) {
			strcat(g_audio_switch_name, "-HL5281M");
		} else if (DIO4485_ID_VALUE == fsa_priv->dev_id) {
			strcat(g_audio_switch_name, "-DIO4485");
		} else if (DIO4480_ID_VALUE == fsa_priv->dev_id) {
			strcat(g_audio_switch_name, "-DIO4480");
		} else {
			strcat(g_audio_switch_name, "-unknown");
		}
		g_fsa4480_usbhs_state = 0;
	}
	dev_info(fsa_priv->dev, "%s: %s 0x%x\n", __func__, g_audio_switch_name, fsa_priv->dev_id);
#endif
	/* END AudioSwitch_Bringup */
#if defined(CONFIG_RUST_DETECTION)
	global_fsa4480_data = fsa_priv;
#endif

	mutex_init(&fsa_priv->notification_lock);
	i2c_set_clientdata(i2c, fsa_priv);

	BLOCKING_INIT_NOTIFIER_HEAD(&fsa_priv->fsa4480_notifier);

	return 0;

err_supply:
	power_supply_put(fsa_priv->usb_psy);
err_data:
	cancel_work_sync(&fsa_priv->usbc_analog_work);
	return rc;
}

static void fsa4480_remove(struct i2c_client *i2c)
{
	struct fsa4480_priv *fsa_priv =
			(struct fsa4480_priv *)i2c_get_clientdata(i2c);

	if (!fsa_priv)
		return;

	if (fsa_priv->use_powersupply) {
		/* deregister from PMI */
		power_supply_unreg_notifier(&fsa_priv->nb);
		power_supply_put(fsa_priv->usb_psy);
	} else {
		unregister_ucsi_glink_notifier(&fsa_priv->nb);
	}
	fsa4480_usbc_update_settings(fsa_priv, 0x18, 0x98);
	cancel_work_sync(&fsa_priv->usbc_analog_work);
	pm_relax(fsa_priv->dev);
	mutex_destroy(&fsa_priv->notification_lock);
	dev_set_drvdata(&i2c->dev, NULL);

}

/* ADD: AudioSwitch_Bringup */
static void fsa4480_shutdown(struct i2c_client *i2c)
{
	struct fsa4480_priv *fsa_priv =
			(struct fsa4480_priv *)i2c_get_clientdata(i2c);

	if (!fsa_priv)
		return;
	pr_info("%s poweroff reset switch.", __func__);
	regmap_write(fsa_priv->regmap, FSA4480_RESET, 0x01);
}
/* END AudioSwitch_Bringup */

static const struct of_device_id fsa4480_i2c_dt_match[] = {
	{
		.compatible = "qcom,fsa4480-i2c",
	},
	{}
};

static struct i2c_driver fsa4480_i2c_driver = {
	.driver = {
		.name = FSA4480_I2C_NAME,
		.of_match_table = fsa4480_i2c_dt_match,
		.probe_type = PROBE_PREFER_ASYNCHRONOUS,
	},
	.probe = fsa4480_probe,
	.remove = fsa4480_remove,
	.shutdown = fsa4480_shutdown, /* ADD: AudioSwitch_Bringup */
};

static int __init fsa4480_init(void)
{
	int rc;

	rc = i2c_add_driver(&fsa4480_i2c_driver);
	if (rc)
		pr_err("fsa4480: Failed to register I2C driver: %d\n", rc);

	return rc;
}
module_init(fsa4480_init);

static void __exit fsa4480_exit(void)
{
	i2c_del_driver(&fsa4480_i2c_driver);
}
module_exit(fsa4480_exit);

MODULE_DESCRIPTION("FSA4480 I2C driver");
MODULE_LICENSE("GPL");
