// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#define DEBUG
#include <linux/kernel.h>
#include <linux/module.h>
// #include <linux/power_supply.h>
#include <linux/regmap.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
// #include <linux/usb/typec.h>
// #include <linux/usb/ucsi_glink.h>
#include "fsa4480-i2c.h"
// #include <linux/iio/consumer.h>
// #include "../asoc/fk-pcm.h"
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/usb.h>
#include <linux/pinctrl/consumer.h>

// #include "../../mca/mca_protocol/protocol_pd/external_tcpc/inc/tcpci_core.h"
// #include "../../mca/mca_protocol/protocol_pd/external_tcpc/inc/tcpm.h"
// #include "../../drivers/power/supply/charger_class.h"
#define USE_POWER_SUPPLY_NOTIFIER    0
#define USE_TCPC_NOTIFIER            1
#define FSA4480_I2C_NAME	"fsa4480-driver"

#define AUDIO_SW_REG_ID		     0x00
#define AUDIO_SW_REG_SWITCH_SETTINGS 0x04
#define AUDIO_SW_REG_SWITCH_CONTROL  0x05
#define AUDIO_SW_REG_SWITCH_STATUS0  0x06
#define AUDIO_SW_REG_SWITCH_STATUS1  0x07
#define AUDIO_SW_REG_SLOW_L	     0x08
#define AUDIO_SW_REG_SLOW_R	     0x09
#define AUDIO_SW_REG_SLOW_MIC	     0x0A
#define AUDIO_SW_REG_SLOW_SENSE	     0x0B
#define AUDIO_SW_REG_SLOW_GND	     0x0C
#define AUDIO_SW_REG_DELAY_L_R	     0x0D
#define AUDIO_SW_REG_DELAY_L_MIC     0x0E
#define AUDIO_SW_REG_DELAY_L_SENSE   0x0F
#define AUDIO_SW_REG_DELAY_L_AGND    0x10
#define AUDIO_SW_REG_FUNCTION_ENABLE 0x12
#define AUDIO_SW_REG_JACK_STATUS     0x17
#define AUDIO_SW_REG_DETECTION_INT   0x18
#define AUDIO_SW_REG_RESET	     0x1E
#define AUDIO_SW_REG_CURRENT_SOURCE  0x1F
#define AUDIO_SW_REG_INVALID	     0xFFFFFFFF

struct fsa4480_priv {
	struct regmap *regmap;
	struct device *dev;
	struct pinctrl *pinctrl_t;
	struct dentry *debug_root;
	// struct tcpc_device *tcpc_dev;
	struct notifier_block nb;
	struct iio_channel *iio_ch;
	atomic_t usbc_mode;
	struct work_struct usbc_analog_work;
	struct blocking_notifier_head fsa4480_notifier;
	struct mutex notification_lock;
	u32 lpd_use;
	int dev_id;
	struct pinctrl_state *pinctrl_st_default;
	struct pinctrl_state *pinctrl_st_active;
};

struct fsa4480_reg_val {
	u16 reg;
	u8 val;
};

static const struct regmap_config fsa4480_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = AUDIO_SW_REG_CURRENT_SOURCE,
};

// static const struct fsa4480_reg_val fsa_reg_i2c_defaults[] = {
// 	{AUDIO_SW_REG_SLOW_L, 0x00},
// 	{AUDIO_SW_REG_SLOW_R, 0x00},
// 	{AUDIO_SW_REG_SLOW_MIC, 0x00},
// 	{AUDIO_SW_REG_SLOW_SENSE, 0x00},
// 	{AUDIO_SW_REG_SLOW_GND, 0x00},
// 	{AUDIO_SW_REG_DELAY_L_R, 0x00},
// 	{AUDIO_SW_REG_DELAY_L_MIC, 0x00},
// 	{AUDIO_SW_REG_DELAY_L_SENSE, 0x00},
// 	{AUDIO_SW_REG_DELAY_L_AGND, 0x09},
// 	{AUDIO_SW_REG_SWITCH_SETTINGS, 0x98},
// };


static const struct fsa4480_reg_val fsa_reg_i2c_defaults[] = {
    {AUDIO_SW_REG_SWITCH_SETTINGS, 0xf8},
    {AUDIO_SW_REG_SWITCH_CONTROL, 0x18},
    {AUDIO_SW_REG_SLOW_L, 0x00},
    {AUDIO_SW_REG_SLOW_R, 0x00},
    {AUDIO_SW_REG_SLOW_MIC, 0x00},
    {AUDIO_SW_REG_SLOW_SENSE, 0x00},
    {AUDIO_SW_REG_SLOW_GND, 0x00},
    {AUDIO_SW_REG_DELAY_L_R, 0x00},
    {AUDIO_SW_REG_DELAY_L_MIC, 0x00},
    {AUDIO_SW_REG_DELAY_L_SENSE, 0x00},
    {AUDIO_SW_REG_DELAY_L_AGND, 0x00},
    {AUDIO_SW_REG_FUNCTION_ENABLE, 0x48},
    {AUDIO_SW_REG_CURRENT_SOURCE, 0x07},
};

enum audio_switch_mode {
	AUDIO_SW_MODE_NOT_CONNECTED = 0x1,
	AUDIO_SW_MODE_3POLE = 0x2,
	AUDIO_SW_MODE_4POLE_1 = 0x4, // 4 Pole audio accessory with SBU1 connected to MIC, SBU2 to AGND
	AUDIO_SW_MODE_4POLE_0 = 0x8, // 4 Pole audio accessory with SBU2 connected to MIC, SBU1 to AGND
	AUDIO_SW_MODE_USB,
	AUDIO_SW_MODE_USBDP,
	AUDIO_SW_MODE_HEADSET,
	AUDIO_SW_MODE_MAX,
};
struct fsa4480_priv *globol_priv_data = NULL;
static struct timer_list sw_enable_timer;
void accdet_madera_report(int which, bool attached);

int get_type_c_hph_direction(void)
{
    u32 jack_status;
    regmap_read(globol_priv_data->regmap, AUDIO_SW_REG_JACK_STATUS, &jack_status);
    pr_info("%s:jack status is 0x%x\n",__func__,jack_status);
    return jack_status;
}

static void dump_register(void)
{
	int adr = 0, value = 0;
	pr_info("%s:======================\n",__func__);
	pr_info("%s:dump hl5281 reg\n",__func__);
	if (!globol_priv_data)
	{
		pr_info("%s:globol_priv_data is NULL !!!!!!!!\n",__func__);
		return;
	}

	for (adr = 0; adr <= AUDIO_SW_REG_CURRENT_SOURCE; adr++) {
		regmap_read(globol_priv_data->regmap, adr, &value);
		pr_info("(0x%x)=0x%x",adr,value);
	}
	pr_info("%s:======================\n",__func__);
}

#define ET_DBG_TYPE_MODE          0
#define ET_DBG_REG_MODE           1
#define ET_DBG_TYPE_MODE_LIUCHENG     3
#define ET_DBG_TYPE_MODE_LIUCHENG_HEADSET     4
enum SWITCH_STATUS {
	SWITCH_STATUS_INVALID = 0,
	SWITCH_STATUS_NOT_CONNECTED,
	SWITCH_STATUS_USB_MODE,
	SWITCH_STATUS_HEADSET_MODE,
	SWITCH_STATUS_MAX
};

enum SWITCH_STATUS audio_sw_get_switch_mode(void)
{
	uint val = 0;
	enum SWITCH_STATUS state = SWITCH_STATUS_INVALID;
	regmap_read(globol_priv_data->regmap, AUDIO_SW_REG_SWITCH_STATUS0, &val);

	switch (val & 0xf) {
	case 0x0:
		state = SWITCH_STATUS_NOT_CONNECTED;
		break;
	case 0x5:
		state = SWITCH_STATUS_USB_MODE;
		break;
	case 0xA:
		state = SWITCH_STATUS_HEADSET_MODE;
		break;
	default:
		state = SWITCH_STATUS_INVALID;
		break;
	}

	return state;
}

char *switch_status_string[SWITCH_STATUS_MAX] = {
	"switch invalid",
	"switch not connected",
	"switch usb mode",
	"switch headset mode",
};

static void fsa4480_usbc_update_settings(struct fsa4480_priv *fsa_priv,
		u32 switch_control, u32 switch_enable)
{
	u32 prev_control, prev_enable;
	pr_info("%s:enter",__func__);
	if (!fsa_priv->regmap) {
		dev_err(fsa_priv->dev, "%s: regmap invalid\n", __func__);
		return;
	}

	regmap_read(fsa_priv->regmap, AUDIO_SW_REG_SWITCH_CONTROL, &prev_control);
	regmap_read(fsa_priv->regmap, AUDIO_SW_REG_SWITCH_SETTINGS, &prev_enable);
	if (prev_control == switch_control && prev_enable == switch_enable) {
		dev_dbg(fsa_priv->dev, "%s: settings unchanged\n", __func__);
		return;
	}

	regmap_write(fsa_priv->regmap, AUDIO_SW_REG_SWITCH_SETTINGS, 0x80);
	regmap_write(fsa_priv->regmap, AUDIO_SW_REG_SWITCH_CONTROL, switch_control);
	/* FSA4480 chip hardware requirement */
	usleep_range(50, 55);
	regmap_write(fsa_priv->regmap, AUDIO_SW_REG_SWITCH_SETTINGS, switch_enable);
}

static int fsa4480_usbc_event_changed_ucsi(struct fsa4480_priv *fsa_priv,
				      unsigned long evt, void *ptr)
{
	struct device *dev;
	int rc,acc=0;
	pr_info("%s:enter evt:%ld",__func__,evt);
	if (evt == 55)
	{
		acc = ((struct ucsi_glink_constat_info *)ptr)->acc;
	}
	// else if (evt == TCP_NOTIFY_TYPEC_STATE)
	// {
	// 	struct tcp_notify *noti = ptr;
	// 	pr_info("%s:evt:TCP_NOTIFY_TYPEC_STATE new_state:%d",__func__,noti->typec_state.new_state);
	// 	if (noti->typec_state.new_state == TYPEC_ATTACHED_AUDIO)
	// 	{
	// 		pr_info("%s:new_state:TYPEC_ATTACHED_AUDIO",__func__);
	// 		acc = TYPEC_ACCESSORY_AUDIO;
	// 	}else if (noti->typec_state.new_state == TYPEC_UNATTACHED)
	// 	{
	// 		pr_info("%s:new_state:TYPEC_UNATTACHED",__func__);
	// 		acc = TYPEC_ACCESSORY_NONE;
	// 	}else{
	// 		pr_err("%s:new_state:%d set default to TYPEC_ACCESSORY_NONE",__func__,noti->typec_state.new_state);
	// 		acc = TYPEC_ACCESSORY_NONE;
	// 	}
	// }else{
	// 	pr_info("%s:evt!=TCP_NOTIFY_TYPEC_STATE ignore",__func__);
	// 	return 0;
	// }
	if (!fsa_priv)
		return -EINVAL;

	dev = fsa_priv->dev;
	if (!dev)
		return -EINVAL;

	dev_dbg(dev, "%s: USB change event received, acc mode %d, usbc mode %d, expected %d\n",
			__func__, acc, fsa_priv->usbc_mode.counter,
			TYPEC_ACCESSORY_AUDIO);
	if (atomic_read(&(fsa_priv->usbc_mode)) == acc){
		pr_info("%s() acc mode no change return\n", __func__);
		return 0;
	}
	atomic_set(&(fsa_priv->usbc_mode), acc);
	switch (acc) {
	case TYPEC_ACCESSORY_AUDIO:
		pr_info("%s() TYPEC_ACCESSORY_AUDIO disable uart\n", __func__);
		if (!IS_ERR_OR_NULL(fsa_priv->pinctrl_st_active)) {
			rc = pinctrl_select_state(fsa_priv->pinctrl_t, fsa_priv->pinctrl_st_active);
			if (rc) {
				dev_err(fsa_priv->dev, "select state: %s failed","active");
			}
		}
		regmap_write(globol_priv_data->regmap, AUDIO_SW_REG_CURRENT_SOURCE, 0x07);
		pr_info("%s() write AUDIO_SW_REG_FUNCTION_ENABLE 0x49\n", __func__);
		regmap_write(globol_priv_data->regmap, AUDIO_SW_REG_SWITCH_SETTINGS, 0x9F);
		regmap_write(globol_priv_data->regmap, AUDIO_SW_REG_SWITCH_CONTROL, 0x00);
		regmap_write(globol_priv_data->regmap, AUDIO_SW_REG_FUNCTION_ENABLE, 0x49);
		mod_timer(&sw_enable_timer, jiffies + (int)(0.05 * HZ));
		pr_info("%s: sw delay 50ms \n", __func__);
		break;
	case TYPEC_ACCESSORY_NONE:
		pr_info("%s() TYPEC_ACCESSORY_NONE enable uart\n", __func__);
		if (!IS_ERR_OR_NULL(fsa_priv->pinctrl_st_default)) {
			rc = pinctrl_select_state(fsa_priv->pinctrl_t, fsa_priv->pinctrl_st_default);
			if (rc) {
				dev_err(fsa_priv->dev, "select state: %s failed","default");
			}
		}
		dev_dbg(dev, "%s: queueing usbc_analog_work\n",__func__);
		// pm_stay_awake(fsa_priv->dev);
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
	pr_info("%s:enter",__func__);
	if (fsa_priv == NULL)
		return -EINVAL;

	dev = fsa_priv->dev;
	if (!dev)
		return -EINVAL;
	fsa4480_usbc_event_changed_ucsi(fsa_priv, evt, ptr);
	pr_info("%s:exit",__func__);
	return 0;
}


static int fsa4480_usbc_analog_setup_switches_ucsi(
						struct fsa4480_priv *fsa_priv)
{
	int rc = 0;
	int mode;
	struct device *dev;
	pr_info("%s:enter",__func__);
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
	u32 int_status = 0;
	u32 jack_status = 0;
	if(mode != TYPEC_ACCESSORY_NONE){
		regmap_read(globol_priv_data->regmap, AUDIO_SW_REG_DETECTION_INT, &int_status);
		jack_status = get_type_c_hph_direction();
		pr_info("%s()int_status:%x,jack_status:%x\n", __func__,int_status,jack_status);
		if (!(int_status & (1 << 2)))
		{
			pr_info("%s() int_status check err!!!!!!!\n", __func__);
			// return 0;
		}
	}
	pr_info("%s() >>>>>>>>>>>>>>>>>>>>>>\n", __func__);
	// dump_register();
	switch (mode) {
	/* add all modes FSA should notify for in here */
	case TYPEC_ACCESSORY_AUDIO:
		/* activate switches */
		switch (jack_status)
		{
		case AUDIO_SW_MODE_3POLE:
			pr_info("%s: 3-pole detect\n", __func__);
			// fsa4480_usbc_update_settings(fsa_priv, 0x00, 0x9F);
			accdet_madera_report(jack_status,true);
			break;
		case AUDIO_SW_MODE_4POLE_1:
			pr_info("%s: Audio Plug In AUDIO_SW_MODE_4POLE_1\n", __func__);
			// fsa4480_usbc_update_settings(fsa_priv, 0x00, 0x9F);
			accdet_madera_report(jack_status,true);
			break;
		case AUDIO_SW_MODE_4POLE_0:
			pr_info("%s: Audio Plug In AUDIO_SW_MODE_4POLE_0\n", __func__);
			// fsa4480_usbc_update_settings(fsa_priv, 0x07, 0x9F);
			accdet_madera_report(jack_status,true);
			break;
		default:
			pr_err("%s() jack_status check err reset to usb\n", __func__);
			accdet_madera_report(jack_status,false);
			fsa4480_usbc_update_settings(fsa_priv, 0x18, 0x98);
			atomic_set(&(globol_priv_data->usbc_mode), TYPEC_ACCESSORY_NONE);
			break;
		}
		/* notify call chain on event */
		blocking_notifier_call_chain(&fsa_priv->fsa4480_notifier,
					     mode, NULL);
		break;
	case TYPEC_ACCESSORY_NONE:
		pr_info("%s: TYPEC_ACCESSORY_NONE detect\n", __func__);
		/* notify call chain on event */
		blocking_notifier_call_chain(&fsa_priv->fsa4480_notifier,
				TYPEC_ACCESSORY_NONE, NULL);

		/* deactivate switches */
		if (fsa_priv->lpd_use)
				fsa4480_usbc_update_settings(fsa_priv, 0x18, 0xF8);
		else
				fsa4480_usbc_update_settings(fsa_priv, 0x18, 0x98);
		accdet_madera_report(jack_status,false);
		break;
	default:
		/* ignore other usb connection modes */
		pr_err("%s: ignore other usb connection modes\n", __func__);
		break;
	}
	dump_register();
	pr_info("%s() <<<<<<<<<<<<<<<<<<<<<<<<<n", __func__);
	mutex_unlock(&fsa_priv->notification_lock);
	return rc;
}

static int fsa4480_usbc_analog_setup_switches(struct fsa4480_priv *fsa_priv)
{
	// linshi
	int rc = 0;
	rc = fsa4480_usbc_analog_setup_switches_ucsi(fsa_priv);
	return rc;
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
	pr_info("%s:enter",__func__);
	if (!client)
		return -EINVAL;

	fsa_priv = (struct fsa4480_priv *)i2c_get_clientdata(client);
	if (!fsa_priv)
		return -EINVAL;

	rc = blocking_notifier_chain_register
				(&fsa_priv->fsa4480_notifier, nb);

	dev_dbg(fsa_priv->dev, "%s: registered notifier for %s\n",
		__func__, node->name);
	if (rc)
		return rc;

	/*
	 * as part of the init sequence check if there is a connected
	 * USB C analog adapter
	 */
	if (atomic_read(&(fsa_priv->usbc_mode)) == TYPEC_ACCESSORY_AUDIO) {
		dev_dbg(fsa_priv->dev, "%s: analog adapter already inserted\n",
			__func__);
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
	pr_info("%s:enter",__func__);
	if (!client)
		return -EINVAL;

	fsa_priv = (struct fsa4480_priv *)i2c_get_clientdata(client);
	if (!fsa_priv)
		return -EINVAL;
	fsa4480_usbc_update_settings(fsa_priv, 0x18, 0x98);
	rc = blocking_notifier_chain_unregister
			(&fsa_priv->fsa4480_notifier, nb);
	return rc;
}
EXPORT_SYMBOL_GPL(fsa4480_unreg_notifier);

static int fsa4480_validate_display_port_settings(struct fsa4480_priv *fsa_priv)
{
	u32 switch_status = 0;
	pr_info("%s:enter",__func__);
	regmap_read(fsa_priv->regmap, AUDIO_SW_REG_SWITCH_STATUS1, &switch_status);

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
	pr_info("%s:enter",__func__);
	if (!client)
		return -EINVAL;

	fsa_priv = (struct fsa4480_priv *)i2c_get_clientdata(client);
	if (!fsa_priv)
		return -EINVAL;
	if (!fsa_priv->regmap)
		return -EINVAL;

	switch (event) {
	case FSA_MIC_GND_SWAP:
		regmap_read(fsa_priv->regmap, AUDIO_SW_REG_SWITCH_CONTROL,
				&switch_control);
		if ((switch_control & 0x07) == 0x07)
			switch_control = 0x0;
		else
			switch_control = 0x7;
		fsa4480_usbc_update_settings(fsa_priv, switch_control, 0x9F);
		break;
	case FSA_USBC_ORIENTATION_CC1:
		fsa4480_usbc_update_settings(fsa_priv, 0x18, 0xF8);
		return fsa4480_validate_display_port_settings(fsa_priv);
	case FSA_USBC_ORIENTATION_CC2:
		fsa4480_usbc_update_settings(fsa_priv, 0x78, 0xF8);
		return fsa4480_validate_display_port_settings(fsa_priv);
	case FSA_USBC_DISPLAYPORT_DISCONNECTED:
		if (fsa_priv->lpd_use)
			fsa4480_usbc_update_settings(fsa_priv, 0x18, 0xF8);
		else
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
	pr_info("%s:enter",__func__);
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
	pr_info("%s:enter",__func__);
	for (i = 0; i < ARRAY_SIZE(fsa_reg_i2c_defaults); i++)
		regmap_write(regmap, fsa_reg_i2c_defaults[i].reg,
				   fsa_reg_i2c_defaults[i].val);
}
static void sw_enable_switch_handler(struct timer_list *t)
{
	int ret = 0;
	pr_info("%s:enter",__func__);
	ret = queue_work(system_freezable_wq, &globol_priv_data->usbc_analog_work);
	if (!ret)
		pr_info("%s, queue work return: %d!\n", __func__, ret);
}



int audio_sw_switch_mode(enum SWITCH_STATUS val)
{
	pr_info("%s:enter",__func__);
	if (val == SWITCH_STATUS_HEADSET_MODE) {
		pr_info("%s:switch to headset",__func__);
		// dump_register();
		fsa4480_usbc_update_settings(globol_priv_data, 0x00, 0x9F);	// switch to headset
		accdet_madera_report(AUDIO_SW_MODE_4POLE_0,true);
		// dump_register();
	} else if (val == SWITCH_STATUS_USB_MODE) {
		pr_info("%s:switch to USB",__func__);
		// dump_register();
		fsa4480_usbc_update_settings(globol_priv_data, 0x18, 0x98); // switch to USB
		accdet_madera_report(AUDIO_SW_MODE_NOT_CONNECTED,false);
		// dump_register();
	}
	return 0;
}

int audio_sw_switch_mode_liucheng(enum SWITCH_STATUS val)
{
	pr_info("%s:enter",__func__);
	if (val == SWITCH_STATUS_HEADSET_MODE) {
		pr_info("%s:switch to headset",__func__);
		struct ucsi_glink_constat_info minfo;
		minfo.acc=TYPEC_ACCESSORY_AUDIO;
		fsa4480_usbc_event_changed(&globol_priv_data->nb,55,(void *)&minfo);

	} else if (val == SWITCH_STATUS_USB_MODE) {
		pr_info("%s:switch to USB",__func__);
		struct ucsi_glink_constat_info minfo;
		minfo.acc=TYPEC_ACCESSORY_NONE;
		fsa4480_usbc_event_changed(&globol_priv_data->nb,55,(void *)&minfo);
	}
	pr_info("%s:exit",__func__);
	return 0;
}

static ssize_t sysfs_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf, u32 type)
{
	int value = 0;
	char *mode = "Unknown mode";
	ssize_t ret_size = 0;

	switch (type) {
	case ET_DBG_TYPE_MODE:
		value = audio_sw_get_switch_mode();
		mode = switch_status_string[value];
		ret_size += snprintf(buf, 50, "%s: %d \n", mode, value);
		break;
	case ET_DBG_TYPE_MODE_LIUCHENG:
		value = audio_sw_get_switch_mode();
		mode = switch_status_string[value];
		ret_size += snprintf(buf, 50, "%s: %d \n", mode, value);
		audio_sw_switch_mode_liucheng(SWITCH_STATUS_USB_MODE);
		break;
	case ET_DBG_TYPE_MODE_LIUCHENG_HEADSET:
		value = audio_sw_get_switch_mode();
		mode = switch_status_string[value];
		ret_size += snprintf(buf, 50, "%s: %d \n", mode, value);
		audio_sw_switch_mode_liucheng(SWITCH_STATUS_HEADSET_MODE);
		break;
	case ET_DBG_REG_MODE:
		dump_register();
	    break;
	default:
		pr_warn("%s: invalid type %d\n", __func__, type);
		break;
	}
	return ret_size;
}

static ssize_t sysfs_set(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count, u32 type)
{
	int err;
	unsigned long value;
	pr_info("%s:enter",__func__);
	err = kstrtoul(buf, 10, &value);
	if (err) {
		pr_warn("%s: get data of type %d failed\n", __func__, type);
		return err;
	}

	pr_info("%s: set type %d, data %ld\n", __func__, type, value);
	switch (type) {
		case ET_DBG_TYPE_MODE:
			audio_sw_switch_mode((enum SWITCH_STATUS)value);
			break;
		case ET_DBG_TYPE_MODE_LIUCHENG:
			audio_sw_switch_mode_liucheng(SWITCH_STATUS_USB_MODE);
			break;
		case ET_DBG_TYPE_MODE_LIUCHENG_HEADSET:
			audio_sw_switch_mode_liucheng(SWITCH_STATUS_HEADSET_MODE);
			break;
		default:
			pr_warn("%s: invalid type %d\n", __func__, type);
			break;
	}
	return count;
}

#define AUDIO_SW_DEVICE_SHOW(_name, _type) static ssize_t \
show_##_name(struct device *dev, \
			  struct device_attribute *attr, char *buf) \
{ \
	return sysfs_show(dev, attr, buf, _type); \
}

#define AUDIO_SW_DEVICE_SET(_name, _type) static ssize_t \
set_##_name(struct device *dev, \
			 struct device_attribute *attr, \
			 const char *buf, size_t count) \
{ \
	return sysfs_set(dev, attr, buf, count, _type); \
}

#define AUDIO_SW_DEVICE_SHOW_SET(name, type) \
AUDIO_SW_DEVICE_SHOW(name, type) \
AUDIO_SW_DEVICE_SET(name, type) \
static DEVICE_ATTR(name, S_IWUSR | S_IRUGO, show_##name, set_##name);

AUDIO_SW_DEVICE_SHOW_SET(audio_sw_switch_mode_liucheng_headset, ET_DBG_TYPE_MODE_LIUCHENG_HEADSET);
AUDIO_SW_DEVICE_SHOW_SET(audio_sw_switch_mode_liucheng, ET_DBG_TYPE_MODE_LIUCHENG);
AUDIO_SW_DEVICE_SHOW_SET(audio_sw_switch_mode, ET_DBG_TYPE_MODE);
AUDIO_SW_DEVICE_SHOW_SET(audio_sw_reg, ET_DBG_REG_MODE);

static struct attribute *audio_sw_attrs[] = {
	&dev_attr_audio_sw_switch_mode_liucheng_headset.attr,
	&dev_attr_audio_sw_switch_mode_liucheng.attr,
	&dev_attr_audio_sw_switch_mode.attr,
	&dev_attr_audio_sw_reg.attr,
	NULL
};

static const struct attribute_group audio_sw_group = {
	.attrs = audio_sw_attrs,
};



//debugfs
static int audio_sw_mode_set_liucheg(void *data, u64 val)
{
	struct fsa4480_priv *priv = data;
	dev_info(priv->dev,"%s:enter",__func__);
	dev_info(priv->dev,"%s:val:%llx",__func__,val);
	audio_sw_switch_mode_liucheng((enum SWITCH_STATUS)val);
	return 0;
}


static int audio_sw_mode_get_liucheg(void *data, u64 *val)
{
	struct fsa4480_priv *priv = data;
	dev_info(priv->dev,"%s:enter",__func__);
	audio_sw_switch_mode_liucheng(SWITCH_STATUS_USB_MODE);
	*val = audio_sw_get_switch_mode();
	return 0;
}

static int audio_sw_mode_get_liucheg_handset(void *data, u64 *val)
{
	struct fsa4480_priv *priv = data;
	dev_info(priv->dev,"%s:enter",__func__);
	audio_sw_switch_mode_liucheng(SWITCH_STATUS_HEADSET_MODE);
	*val = audio_sw_get_switch_mode();
	return 0;
}



static int audio_sw_mode_set(void *data, u64 val)
{
	struct fsa4480_priv *priv = data;
	dev_info(priv->dev,"%s:enter",__func__);
	dev_info(priv->dev,"%s:val:%llx",__func__,val);
	audio_sw_switch_mode((enum SWITCH_STATUS)val);
	return 0;
}

static int audio_sw_mode_get(void *data, u64 *val)
{
	struct fsa4480_priv *priv = data;
	dev_info(priv->dev,"%s:enter",__func__);
	dump_register();
	*val = audio_sw_get_switch_mode();
	audio_sw_switch_mode(SWITCH_STATUS_USB_MODE);
	dump_register();
	return 0;
}

static int audio_sw_mode_get_2(void *data, u64 *val)
{
	struct fsa4480_priv *priv = data;
	dev_info(priv->dev,"%s:enter",__func__);
	dump_register();
	*val = audio_sw_get_switch_mode();
	audio_sw_switch_mode(SWITCH_STATUS_HEADSET_MODE);
	dump_register();
	return 0;
}


DEFINE_DEBUGFS_ATTRIBUTE(audio_sw_mode_fops, audio_sw_mode_get,
			 audio_sw_mode_set, "%llu\n");
DEFINE_DEBUGFS_ATTRIBUTE(audio_sw_mode_fops2, audio_sw_mode_get_2,
			 audio_sw_mode_set, "%llu\n");
DEFINE_DEBUGFS_ATTRIBUTE(audio_sw_mode_fops_test_usb, audio_sw_mode_get_liucheg,
			 audio_sw_mode_set_liucheg, "%llu\n");
DEFINE_DEBUGFS_ATTRIBUTE(audio_sw_mode_fops_test_handset, audio_sw_mode_get_liucheg_handset,
			 audio_sw_mode_set_liucheg, "%llu\n");
#define AUDIO_SWITCH_MODULE_NAME "audio_sw"
static void audio_switch_debugfs_create(struct fsa4480_priv *priv)
{
	struct dentry *root = NULL;

	root = debugfs_create_dir(AUDIO_SWITCH_MODULE_NAME, usb_debug_root);
	if (!root) {
		dev_err(priv->dev, "failed to create dir\n");
		return;
	}

	debugfs_create_file("mode", 0644, root, priv, &audio_sw_mode_fops);
	debugfs_create_file("mode_handset", 0644, root, priv, &audio_sw_mode_fops2);
	debugfs_create_file("test_mode_usb", 0644, root, priv, &audio_sw_mode_fops_test_usb);
	debugfs_create_file("test_mode_handset", 0644, root, priv, &audio_sw_mode_fops_test_handset);
	priv->debug_root = root;
}

static void audio_switch_debugfs_destroy(struct fsa4480_priv *priv)
{
	if (priv->debug_root) {
		debugfs_remove(priv->debug_root);
		priv->debug_root = NULL;
	}
}

static int fsa4480_probe(struct i2c_client *i2c)
{
	struct fsa4480_priv *fsa_priv;
	u32 lpd_use;
	int rc = 0;
	pr_info("%s:enter",__func__);
	fsa_priv = devm_kzalloc(&i2c->dev, sizeof(*fsa_priv),
				GFP_KERNEL);
	if (!fsa_priv)
		return -ENOMEM;

	memset(fsa_priv, 0, sizeof(struct fsa4480_priv));
	globol_priv_data = fsa_priv;
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
	regmap_read(fsa_priv->regmap, AUDIO_SW_REG_ID, &fsa_priv->dev_id);
	pr_info("%s: dev_id:%x\n", __func__, fsa_priv->dev_id);

	rc = sysfs_create_group(&fsa_priv->dev->kobj, &audio_sw_group);
	if (rc) {
		pr_err("%s: create attr error %d\n", __func__, rc);
	}
	audio_switch_debugfs_create(fsa_priv);
	// pr_info("%s: register_tcp_dev_notifier\n", __func__);
	// fsa_priv->tcpc_dev = tcpc_dev_get_by_name("type_c_port0");
	// if (!fsa_priv->tcpc_dev) {
	// 	rc = -EPROBE_DEFER;
	// 	pr_err("%s get tcpc device type_c_port0 fail \n", __func__);
	// 	goto err_data;
	// }
	fsa_priv->nb.notifier_call = fsa4480_usbc_event_changed;
	fsa_priv->nb.priority = 0;
	// rc = register_tcp_dev_notifier(fsa_priv->tcpc_dev, &fsa_priv->nb, TCP_NOTIFY_TYPE_USB);

	if (rc) {
		dev_err(fsa_priv->dev,
			"%s: ucsi glink notifier registration failed: %d\n",
			__func__, rc);
		goto err_data;
	}

	rc = of_property_read_u32(fsa_priv->dev->of_node,
			"mi,lpd-use-sbu_h", &lpd_use);
	if (lpd_use) {
		fsa_priv->lpd_use = 1;
		regmap_write(fsa_priv->regmap, AUDIO_SW_REG_SWITCH_SETTINGS, 0xF8);
		dev_err(fsa_priv->dev, "%s: mi lpd use\n", __func__);
	} else {
		fsa_priv->lpd_use = 0;
		dev_err(fsa_priv->dev, "%s: mi lpd not use\n", __func__);
	}

	mutex_init(&fsa_priv->notification_lock);
	i2c_set_clientdata(i2c, fsa_priv);

	/* get pinctrl handle */
	fsa_priv->pinctrl_t = devm_pinctrl_get(fsa_priv->dev);
	if (IS_ERR_OR_NULL(fsa_priv->pinctrl_t)) {
		rc = PTR_ERR(fsa_priv->pinctrl_t);
		dev_info(fsa_priv->dev, "get audio pinctrl fail, ret = %d.", rc);
	}else{
		dev_info(fsa_priv->dev, "get fsa_priv pinctrl success.");
		fsa_priv->pinctrl_st_default = pinctrl_lookup_state(fsa_priv->pinctrl_t, "default");
		if (IS_ERR_OR_NULL(fsa_priv->pinctrl_st_default)) {
			dev_err(fsa_priv->dev, "find state: %s failed", "default");
			goto err_data;
		}
		fsa_priv->pinctrl_st_active = pinctrl_lookup_state(fsa_priv->pinctrl_t, "active");
		if (IS_ERR_OR_NULL(fsa_priv->pinctrl_st_active)) {
			dev_err(fsa_priv->dev, "find state: %s failed", "active");
			goto err_data;
		}
	}
	INIT_WORK(&fsa_priv->usbc_analog_work,
		  fsa4480_usbc_analog_work_fn);

	BLOCKING_INIT_NOTIFIER_HEAD(&fsa_priv->fsa4480_notifier);

	pr_info("%s(), setup enable timer", __func__);
	timer_setup(&sw_enable_timer, sw_enable_switch_handler, 0);
	return 0;

err_data:
	return rc;
}

static void fsa4480_remove(struct i2c_client *i2c)
{
	struct fsa4480_priv *fsa_priv =
			(struct fsa4480_priv *)i2c_get_clientdata(i2c);

	if (!fsa_priv)
		return;

	globol_priv_data = NULL;
	audio_switch_debugfs_destroy(fsa_priv);
	fsa4480_usbc_update_settings(fsa_priv, 0x18, 0x98);

	cancel_work_sync(&fsa_priv->usbc_analog_work);
	pm_relax(fsa_priv->dev);
	mutex_destroy(&fsa_priv->notification_lock);
	dev_set_drvdata(&i2c->dev, NULL);

}

static const struct of_device_id fsa4480_i2c_dt_match[] = {
	{
		.compatible = "xring,hl5281-i2c",
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
//MODULE_SOFTDEP("pre: madera_codec_cs47l92");
//MODULE_SOFTDEP("pre: external_tcpc");
