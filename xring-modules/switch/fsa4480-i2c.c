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
#include <soc/xring/fsa4480-i2c.h>
// #include <linux/iio/consumer.h>
// #include "../asoc/fk-pcm.h"
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/usb.h>
#include <linux/pinctrl/consumer.h>
#include <mca_protocol/protocol_pd/external_tcpc/inc/tcpci_core.h>
#include <mca_protocol/protocol_pd/external_tcpc/inc/tcpm.h>
#include <mca/protocol/protocol_pd_class.h>

#include <mca_hardware_monitor/inc/mca_lpd_detect.h>
#include <mca/common/mca_event.h>
#include <mca/common/mca_log.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <mca/protocol/protocol_class.h>
#include <mca/platform/platform_buckchg_class.h>
#include <mca/common/mca_workqueue.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_fsa4480_lpd"
#endif

#define FSA4480_OVP_MSK              0x01
#define FSA4480_OVP_FLG              0x02
#define FSA4480_FUNC_CTL0            0x12
#define FSA4480_DET_CTL0             0x13
#define FSA4480_DET_R_TH             0x15
#define FSA4480_FUNC_CTL1            0x16
#define FSA4480_SYS_FLG              0x18
#define FSA4480_SYS_MSK              0x19
#define FSA4480_DET_CTL1             0x31
#define FSA4480_DET_V_TH             0x32
/*detect vol*/
#define FSA4480_DETM_V_CCIN          0x38
#define FSA4480_DETM_V_DPR           0x39
#define FSA4480_DETM_V_DNL           0x3A
#define FSA4480_DETM_V_SBU1          0x3B
#define FSA4480_DETM_V_SBU2          0x3C
#define FSA4480_ALLOW_MAX_REG        0x3C

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
#define DELAY_INIT_TIME     (10 * HZ)

BLOCKING_NOTIFIER_HEAD(acore_notifier);

struct fsa4480_priv {
	struct regmap *regmap;
	struct device *dev;
	struct pinctrl *pinctrl_t;
	struct dentry *debug_root;
	struct tcpc_device *tcpc_dev;
	struct notifier_block nb;
	struct iio_channel *iio_ch;
	atomic_t usbc_mode;
	atomic_t orientation_status;
	atomic_t pd_connect_status;
	struct work_struct usbc_analog_work;
	struct work_struct delay_init_work;
	struct blocking_notifier_head fsa4480_notifier;
	struct mutex notification_lock;
	u32 lpd_use;
	u32 uart_mode;
	int dev_id;
	struct pinctrl_state *pinctrl_st_default;
	struct pinctrl_state *pinctrl_st_active;
	struct notifier_block lpd_nb;
	int gpio_irq;
	int lpd_irq;
	struct delayed_work lpd_irq_work;
	int start_det_flag;
	bool support_lpd;
};

struct fsa4480_reg_val {
	u16 reg;
	u8 val;
};

static const struct regmap_config fsa4480_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = FSA4480_ALLOW_MAX_REG,
};

static const struct fsa4480_reg_val fsa_reg_i2c_defaults[] = {
	{AUDIO_SW_REG_SWITCH_SETTINGS, 0x98},
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
struct fsa4480_priv *globol_priv_data;
static struct timer_list sw_enable_timer;
static struct timer_list delay_init_timer;
// static struct workqueue_struct *delay_init_workqueue;
int fsa4480_switch_event_internal(struct fsa4480_priv *fsa_priv,
			 enum fsa_function event);

static void madera_extcon_notify_event(int which, bool attached)
{
	struct madera_extcon_info data;

	data.which = which;
	data.attached = attached;

	blocking_notifier_call_chain(&acore_notifier, 1, &data);
}

int get_type_c_hph_direction(void)
{
	u32 jack_status;

	regmap_read(globol_priv_data->regmap, AUDIO_SW_REG_JACK_STATUS, &jack_status);
	pr_info("%s: jack status is 0x%x\n", __func__, jack_status);

	return jack_status;
}

void set_uart_pinctl(int active)
{
	int rc;

	if (globol_priv_data->uart_mode == 1) {
		pr_info("%s:uart_mode:%d test_mode disable uart!\n", __func__, globol_priv_data->uart_mode);
		pr_info("%s() TYPEC_ACCESSORY_AUDIO disable uart\n", __func__);
		if (!IS_ERR_OR_NULL(globol_priv_data->pinctrl_st_active)) {
			rc = pinctrl_select_state(globol_priv_data->pinctrl_t, globol_priv_data->pinctrl_st_active);
			if (rc)
				dev_err(globol_priv_data->dev, "select state: %s failed", "active");
		}
	} else {
		if (active) {
			pr_info("%s() TYPEC_ACCESSORY_AUDIO disable uart\n", __func__);
			if (!IS_ERR_OR_NULL(globol_priv_data->pinctrl_st_active)) {
				rc = pinctrl_select_state(globol_priv_data->pinctrl_t, globol_priv_data->pinctrl_st_active);
				if (rc)
					dev_err(globol_priv_data->dev, "select state: %s failed", "active");
			}
		} else {
			pr_info("%s() TYPEC_ACCESSORY_NONE enable uart\n", __func__);
			if (!IS_ERR_OR_NULL(globol_priv_data->pinctrl_st_default)) {
				rc = pinctrl_select_state(globol_priv_data->pinctrl_t, globol_priv_data->pinctrl_st_default);
				if (rc)
					dev_err(globol_priv_data->dev, "select state: %s failed", "default");
			}
		}
	}
}

static void dump_register(void)
{
	int adr = 0, value = 0;

	pr_info("%s:======================\n", __func__);
	pr_info("%s:dump hl5281 reg\n", __func__);

	if (!globol_priv_data) {
		pr_info("%s:globol_priv_data is NULL !!!!!!!!\n", __func__);
		return;
	}

	for (adr = 0; adr <= AUDIO_SW_REG_CURRENT_SOURCE; adr++) {
		regmap_read(globol_priv_data->regmap, adr, &value);
		pr_info("(0x%x)=0x%x", adr, value);
	}
	pr_info("%s:======================\n", __func__);
}

#define ET_DBG_TYPE_MODE          0
#define ET_DBG_REG_MODE           1
#define ET_DBG_UART_MODE          5
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

	pr_info("%s: +", __func__);

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
	struct tcp_notify *noti = ptr;
	int cc_orientation = -1;
	enum fsa_function event;
	int acc = 0;
	u32 g_val;

	if (!fsa_priv)
		return -EINVAL;

	dev = fsa_priv->dev;
	if (!dev)
		return -EINVAL;

	pr_info("%s:enter evt:%ld, typec_state.new_state:%d, pd_state.connected:%d",
			__func__, evt, noti->typec_state.new_state, noti->pd_state.connected);
	if (evt == 55) {
		acc = ((struct ucsi_glink_constat_info *)ptr)->acc;
	} else if (evt == TCP_NOTIFY_TYPEC_STATE) {
		if (noti->typec_state.new_state == TYPEC_ATTACHED_AUDIO) {
			pr_info("%s:new_state:TYPEC_ATTACHED_AUDIO", __func__);
			acc = TYPEC_ACCESSORY_AUDIO;
		} else if (noti->typec_state.new_state == TYPEC_UNATTACHED) {
			pr_info("%s:new_state:TYPEC_UNATTACHED", __func__);
			acc = TYPEC_ACCESSORY_NONE;
			/* DP add */
			atomic_set(&(fsa_priv->orientation_status), -1);
			if (atomic_read(&(fsa_priv->pd_connect_status)) == 1) {
				atomic_set(&(fsa_priv->pd_connect_status), 0);
				event = FSA_USBC_DISPLAYPORT_DISCONNECTED;
				fsa4480_switch_event_internal(fsa_priv, event);
				pr_info("%s: evt %ld, cc_orientation:%d, event:%d", __func__, evt, cc_orientation, event);
			}
		} else {
			pr_err("%s:new_state:%d set default to TYPEC_ACCESSORY_NONE", __func__, noti->typec_state.new_state);
			acc = TYPEC_ACCESSORY_NONE;
		}
	} else if (evt == TCP_NOTIFY_AMA_DP_STATE) {
		protocol_class_pd_get_typec_cc_orientation(TYPEC_PORT_0, &cc_orientation);
		switch (noti->ama_dp_state.active) {
		case 0:
			if (atomic_read(&(fsa_priv->orientation_status)) == cc_orientation &&
					atomic_read(&(fsa_priv->pd_connect_status)) == 0) {
				pr_info("%s: orientation and connect status no change return\n", __func__);
			} else {
				atomic_set(&(fsa_priv->orientation_status), cc_orientation);
				atomic_set(&(fsa_priv->pd_connect_status), 0);
				event = FSA_USBC_DISPLAYPORT_DISCONNECTED;
				fsa4480_switch_event_internal(fsa_priv, event);
				pr_info("%s: evt %ld, cc_orientation:%d, event:%d", __func__, evt, cc_orientation, event);
			}
			break;
		case 1:
			if (atomic_read(&(fsa_priv->orientation_status)) == cc_orientation &&
					atomic_read(&(fsa_priv->pd_connect_status)) == 1) {
				pr_info("%s: orientation and connect status no change return\n", __func__);
			} else {
				atomic_set(&(fsa_priv->orientation_status), cc_orientation);
				atomic_set(&(fsa_priv->pd_connect_status), 1);
				if (cc_orientation == 1)
					event = FSA_USBC_ORIENTATION_CC1;
				else
					event = FSA_USBC_ORIENTATION_CC2;
				fsa4480_switch_event_internal(fsa_priv, event);
				pr_info("%s: evt %ld, cc_orientation:%d, event:%d", __func__, evt, cc_orientation, event);
			}
			break;
		default:
			pr_info("%s: unknow active %d", __func__, noti->ama_dp_state.active);
			break;
		}
	} else {
		pr_info("%s: ignore evt %ld", __func__, evt);
		return 0;
	}

	dev_dbg(dev, "%s: USB change event received, acc mode %d, usbc mode %d, expected %d\n",
			__func__, acc, fsa_priv->usbc_mode.counter,	TYPEC_ACCESSORY_AUDIO);
	if (atomic_read(&(fsa_priv->usbc_mode)) == acc) {
		pr_info("%s() acc mode no change return\n", __func__);
		return 0;
	}
	atomic_set(&(fsa_priv->usbc_mode), acc);
	switch (acc) {
	case TYPEC_ACCESSORY_AUDIO:
		set_uart_pinctl(1);
		regmap_write(globol_priv_data->regmap, AUDIO_SW_REG_CURRENT_SOURCE, 0x07);
		pr_info("%s() write AUDIO_SW_REG_FUNCTION_ENABLE 0x49\n", __func__);
		regmap_write(globol_priv_data->regmap, AUDIO_SW_REG_SWITCH_SETTINGS, 0x9F);
		regmap_write(globol_priv_data->regmap, AUDIO_SW_REG_SWITCH_CONTROL, 0x00);
		regmap_read(globol_priv_data->regmap, AUDIO_SW_REG_FUNCTION_ENABLE, &g_val);
		if (g_val & 0x02)
			regmap_write(globol_priv_data->regmap, AUDIO_SW_REG_FUNCTION_ENABLE, 0x4B);
		else
			regmap_write(globol_priv_data->regmap, AUDIO_SW_REG_FUNCTION_ENABLE, 0x49);
		mod_timer(&sw_enable_timer, jiffies + (int)(0.05 * HZ));
		pr_info("%s: sw delay 50ms\n", __func__);
		break;
	case TYPEC_ACCESSORY_NONE:
		set_uart_pinctl(0);
		dev_dbg(dev, "%s: queueing usbc_analog_work\n", __func__);
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

	pr_info("%s: +", __func__);

	dev = fsa_priv->dev;
	if (!dev)
		return -EINVAL;

	fsa4480_usbc_event_changed_ucsi(fsa_priv, evt, ptr);
	pr_info("%s:  -", __func__);

	return 0;
}


static int fsa4480_usbc_analog_setup_switches_ucsi(
						struct fsa4480_priv *fsa_priv)
{
	int rc = 0;
	int mode;
	u32 int_status = 0;
	u32 jack_status = 0;
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
	if (mode != TYPEC_ACCESSORY_NONE) {
		regmap_read(globol_priv_data->regmap, AUDIO_SW_REG_DETECTION_INT, &int_status);
		jack_status = get_type_c_hph_direction();
		pr_info("%s()int_status:%x,jack_status:%x\n", __func__, int_status, jack_status);
		if (!(int_status & (1 << 2)))
			pr_info("%s() int_status check err!!!!!!!\n", __func__);
	}
	pr_info("%s() >>>>>>>>>>>>>>>>>>>>>>\n", __func__);
	// dump_register();
	switch (mode) {
	/* add all modes FSA should notify for in here */
	case TYPEC_ACCESSORY_AUDIO:
		/* activate switches */
		switch (jack_status) {
		case AUDIO_SW_MODE_3POLE:
			pr_info("%s: 3-pole detect\n", __func__);
			// fsa4480_usbc_update_settings(fsa_privfsa_priv, 0x00, 0x9F);
			madera_extcon_notify_event(jack_status, true);
			break;
		case AUDIO_SW_MODE_4POLE_1:
			pr_info("%s: Audio Plug In AUDIO_SW_MODE_4POLE_1\n", __func__);
			// fsa4480_usbc_update_settings(fsa_priv, 0x00, 0x9F);
			madera_extcon_notify_event(jack_status, true);
			break;
		case AUDIO_SW_MODE_4POLE_0:
			pr_info("%s: Audio Plug In AUDIO_SW_MODE_4POLE_0\n", __func__);
			// fsa4480_usbc_update_settings(fsa_priv, 0x07, 0x9F);
			madera_extcon_notify_event(jack_status, true);
			break;
		default:
			pr_err("%s() jack_status check err reset to AUDIO_SW_MODE_3POLE\n", __func__);
			pr_info("%s: 3-pole detect\n", __func__);
			fsa4480_usbc_update_settings(fsa_priv, 0x00, 0x9F);
			madera_extcon_notify_event(jack_status, true);
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
		madera_extcon_notify_event(jack_status, false);
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

	pr_info("%s: +", __func__);

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

	pr_info("%s: +", __func__);

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

	pr_info("%s: +", __func__);

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

	pr_info("%s: +", __func__);

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
		set_uart_pinctl(1);
		fsa4480_usbc_update_settings(fsa_priv, 0x18, 0xF8);
		return fsa4480_validate_display_port_settings(fsa_priv);
	case FSA_USBC_ORIENTATION_CC2:
		set_uart_pinctl(1);
		fsa4480_usbc_update_settings(fsa_priv, 0x78, 0xF8);
		return fsa4480_validate_display_port_settings(fsa_priv);
	case FSA_USBC_DISPLAYPORT_DISCONNECTED:
		set_uart_pinctl(0);
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

int fsa4480_switch_event_set(enum fsa_function event)
{
	int switch_control = 0;

	pr_info("%s:enter, event = %d", __func__, event);

	if (!globol_priv_data) {
		pr_err("%s:globol_priv_data is NULL !!!!!!!!\n", __func__);
		return -EINVAL;
	}
	switch (event) {
	case FSA_MIC_GND_SWAP:
		regmap_read(globol_priv_data->regmap, AUDIO_SW_REG_SWITCH_CONTROL,
				&switch_control);
		if ((switch_control & 0x07) == 0x07)
			switch_control = 0x0;
		else
			switch_control = 0x7;
		fsa4480_usbc_update_settings(globol_priv_data, switch_control, 0x9F);
		break;
	case FSA_USBC_ORIENTATION_CC1:
		set_uart_pinctl(1);
		fsa4480_usbc_update_settings(globol_priv_data, 0x18, 0xF8);
		return fsa4480_validate_display_port_settings(globol_priv_data);
	case FSA_USBC_ORIENTATION_CC2:
		set_uart_pinctl(1);
		fsa4480_usbc_update_settings(globol_priv_data, 0x78, 0xF8);
		return fsa4480_validate_display_port_settings(globol_priv_data);
	case FSA_USBC_DISPLAYPORT_DISCONNECTED:
		set_uart_pinctl(0);
		if (globol_priv_data->lpd_use)
			fsa4480_usbc_update_settings(globol_priv_data, 0x18, 0xF8);
		else
			fsa4480_usbc_update_settings(globol_priv_data, 0x18, 0x98);
		break;
	default:
		break;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(fsa4480_switch_event_set);

int fsa4480_switch_event_internal(struct fsa4480_priv *fsa_priv,
			 enum fsa_function event)
{
	int switch_control = 0;

	pr_info("%s:enter, event = %d", __func__, event);

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
		set_uart_pinctl(1);
		fsa4480_usbc_update_settings(fsa_priv, 0x18, 0xF8);
		return fsa4480_validate_display_port_settings(fsa_priv);
	case FSA_USBC_ORIENTATION_CC2:
		set_uart_pinctl(1);
		fsa4480_usbc_update_settings(fsa_priv, 0x78, 0xF8);
		return fsa4480_validate_display_port_settings(fsa_priv);
	case FSA_USBC_DISPLAYPORT_DISCONNECTED:
		set_uart_pinctl(0);
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

static void fsa4480_usbc_analog_work_fn(struct work_struct *work)
{
	struct fsa4480_priv *fsa_priv =
		container_of(work, struct fsa4480_priv, usbc_analog_work);

	pr_info("%s: +", __func__);
	if (!fsa_priv) {
		pr_err("%s: fsa container invalid\n", __func__);
		return;
	}
	fsa4480_usbc_analog_setup_switches(fsa_priv);
	pm_relax(fsa_priv->dev);
}

static void delay_init_work_callback(struct work_struct *work)
{
	int rc;
	u32 g_val;
#if USE_TCPC_NOTIFIER
	int acc = 0;
	struct fsa4480_priv *fsa_priv = globol_priv_data;

	if (fsa_priv && fsa_priv->tcpc_dev) {
		pr_info("%s() enter USE_TCPC_NOTIFIER\n", __func__);
		/* check tcpc status at startup */
		if (tcpm_inquire_typec_attach_state(fsa_priv->tcpc_dev) == TYPEC_ATTACHED_AUDIO) {
			/* Audio Plug in */
			pr_info("%s: Audio is Plug In status at startup\n", __func__);
			acc = TYPEC_ACCESSORY_AUDIO;
		} else {
			pr_info("%s: Audio is Plug Out status at startup\n", __func__);
			acc = TYPEC_ACCESSORY_NONE;
		}
		atomic_set(&(fsa_priv->usbc_mode), acc);
		switch (acc) {
		case TYPEC_ACCESSORY_AUDIO:
			set_uart_pinctl(1);
			regmap_write(fsa_priv->regmap, AUDIO_SW_REG_CURRENT_SOURCE, 0x07);
			pr_info("%s() write AUDIO_SW_REG_FUNCTION_ENABLE 0x49\n", __func__);
			regmap_write(fsa_priv->regmap, AUDIO_SW_REG_SWITCH_SETTINGS, 0x9F);
			regmap_write(fsa_priv->regmap, AUDIO_SW_REG_SWITCH_CONTROL, 0x00);
			regmap_read(fsa_priv->regmap, AUDIO_SW_REG_FUNCTION_ENABLE, &g_val);
			if (g_val & 0x02)
				regmap_write(fsa_priv->regmap, AUDIO_SW_REG_FUNCTION_ENABLE, 0x4B);
			else
				regmap_write(fsa_priv->regmap, AUDIO_SW_REG_FUNCTION_ENABLE, 0x49);
			mod_timer(&sw_enable_timer, jiffies + (int)(0.05 * HZ));
			pr_info("%s:TYPEC_ACCESSORY_AUDIO queueing usbc_analog_work delay 50ms\n", __func__);
			break;
		case TYPEC_ACCESSORY_NONE:
			set_uart_pinctl(0);
			pr_info("%s:TYPEC_ACCESSORY_NONE\n", __func__);
			// pm_stay_awake(fsa_priv->dev);
			// queue_work(system_freezable_wq, &fsa_priv->usbc_analog_work);
			break;
		}
		pr_info("%s:register_tcp_dev_notifier\n", __func__);
		fsa_priv->nb.notifier_call = fsa4480_usbc_event_changed;
		fsa_priv->nb.priority = 0;
		rc = register_tcp_dev_notifier(fsa_priv->tcpc_dev, &fsa_priv->nb, TCP_NOTIFY_TYPE_USB | TCP_NOTIFY_TYPE_MODE);
		if (rc)
			dev_err(fsa_priv->dev, "%s: ucsi glink notifier registration failed: %d\n", __func__, rc);

	}
#endif
}

static void fsa4480_update_reg_defaults(struct regmap *regmap)
{
	u8 i;

	pr_info("%s: +", __func__);

	for (i = 0; i < ARRAY_SIZE(fsa_reg_i2c_defaults); i++)
		regmap_write(regmap, fsa_reg_i2c_defaults[i].reg,
				   fsa_reg_i2c_defaults[i].val);
}

static void sw_enable_switch_handler(struct timer_list *t)
{
	int ret = 0;

	ret = queue_work(system_freezable_wq, &globol_priv_data->usbc_analog_work);
	if (!ret)
		pr_info("%s, queue work return: %d!\n", __func__, ret);
	else
		pr_info("%s, queue work success: %d!\n", __func__, ret);
}

static void delay_init_handler(struct timer_list *t)
{
	int ret = 0;

	// ret = queue_work(delay_init_workqueue, &globol_priv_data->delay_init_work);
	ret = queue_work(system_freezable_wq, &globol_priv_data->delay_init_work);
	if (!ret)
		pr_info("%s, queue work return: %d!\n", __func__, ret);
	else
		pr_info("%s, queue work success: %d!\n", __func__, ret);
}

int audio_sw_switch_mode(enum SWITCH_STATUS val)
{
	if (val == SWITCH_STATUS_HEADSET_MODE) {
		pr_info("%s:switch to headset", __func__);
		// dump_register();
		fsa4480_usbc_update_settings(globol_priv_data, 0x00, 0x9F);	// switch to headset
		madera_extcon_notify_event(AUDIO_SW_MODE_4POLE_0, true);
		// dump_register();
	} else if (val == SWITCH_STATUS_USB_MODE) {
		pr_info("%s:switch to USB", __func__);
		// dump_register();
		fsa4480_usbc_update_settings(globol_priv_data, 0x18, 0x98); // switch to USB
		madera_extcon_notify_event(AUDIO_SW_MODE_NOT_CONNECTED, false);
		// dump_register();
	}
	return 0;
}

int mca_set_lpd_enable(bool en)
{
	int ret = 0;
	u32 lpd_enable;

	if (en)
		ret |= regmap_write(globol_priv_data->regmap, FSA4480_FUNC_CTL0, 0x4A); //0x12 write 0x4A
	else
		ret |= regmap_write(globol_priv_data->regmap, FSA4480_FUNC_CTL0, 0x48); //0x12 write 0x48
	if (ret)
		mca_log_err("%s:failed", __func__);

	ret = regmap_read(globol_priv_data->regmap, FSA4480_FUNC_CTL0, &lpd_enable);
	mca_log_info("%s:is %d, 0x12 reg = 0x%x\n", __func__, en, lpd_enable);
	return ret;
}

int mca_get_lpd_enable(int *lpd_en_status)
{
	int ret = 0;
	u32 lpd_en_reg;

	ret = regmap_read(globol_priv_data->regmap, FSA4480_FUNC_CTL0, &lpd_en_reg);
	if (!ret) {
		*lpd_en_status = lpd_en_reg & BIT(1);
		mca_log_info("%s:get 0x12 reg = 0x%x, lpd_en = %d\n", __func__, lpd_en_reg, *lpd_en_status);
	} else {
		mca_log_err("%s:failed\n", __func__);
	}
	return ret;
}

int mca_get_lpd_trigger_status(int *lpd_tirgger_status)
{
	int ret = 0;
	u32 lpd_trigger_reg;

	ret = regmap_read(globol_priv_data->regmap, FSA4480_SYS_FLG, &lpd_trigger_reg);
	if (!ret) {
		*lpd_tirgger_status = lpd_trigger_reg & BIT(1);
		mca_log_info("%s:get 0x18 reg = 0x%x, lpd trigger = %d\n", __func__, lpd_trigger_reg, *lpd_tirgger_status);
	} else {
		*lpd_tirgger_status = 0;
		mca_log_err("%s:failed\n", __func__);
	}
	return ret;
}

int mca_check_lpd_ovp(void)
{
	int ret = 0;
	u32 lpd_ovp_flg;

	ret = regmap_read(globol_priv_data->regmap, FSA4480_OVP_FLG, &lpd_ovp_flg);
	if (!ret)
		mca_log_info("get lpd ovp flag = 0x%x\n", lpd_ovp_flg);
	else
		mca_log_err("get lpd ovp flag failed\n");
	return ret;
}

int mca_lpd_get_reg(enum lpd_attr_list LPD, int *reg)
{
	int ret = 0;

	switch (LPD) {
	case LPD_PROP_SBU1:
		regmap_read(globol_priv_data->regmap, FSA4480_DETM_V_SBU1, reg);
		mca_log_info("get sbu1 = %d\n", *reg);
		break;
	case LPD_PROP_SBU2:
		regmap_read(globol_priv_data->regmap, FSA4480_DETM_V_SBU2, reg);
		mca_log_info("get sbu2 = %d\n", *reg);
		break;
	case LPD_PROP_DP:
		regmap_read(globol_priv_data->regmap, FSA4480_DETM_V_DPR, reg);
		mca_log_info("get dp = %d\n", *reg);
		break;
	case LPD_PROP_DM:
		regmap_read(globol_priv_data->regmap, FSA4480_DETM_V_DNL, reg);
		mca_log_info("get dm = %d\n", *reg);
		break;
	case LPD_PROP_CC1:
	case LPD_PROP_CC2:
		regmap_read(globol_priv_data->regmap, FSA4480_DETM_V_CCIN, reg);
		mca_log_info("get cc = %d\n", *reg);
		break;
	case LPD_PROP_EN:
		mca_get_lpd_enable(reg);
		mca_log_info("get lpd en = %d\n", *reg);
		break;
	case LPD_PROP_STATUS:
		mca_get_lpd_trigger_status(reg);
		mca_log_info("get lpd status = %d\n", *reg);
		break;
	default:
		break;
	}
	return ret;
}
EXPORT_SYMBOL(mca_lpd_get_reg);

int mca_get_pin_val(void)
{
	int ret = 0;
	u32 sbu1_reg;
	u32 sbu2_reg;
	u32 dpr_reg;
	u32 dpl_reg;
	u32 cc_reg;
	u32 en_reg;
	u32 mult_pin_det;
	u32 inter_time;
	u32 current_source_en;
	u32 lpd_en_status;
	u32 volt_th;

	ret |= regmap_read(globol_priv_data->regmap, FSA4480_DETM_V_SBU1, &sbu1_reg);
	ret |= regmap_read(globol_priv_data->regmap, FSA4480_DETM_V_SBU2, &sbu2_reg);
	ret |= regmap_read(globol_priv_data->regmap, FSA4480_DETM_V_DPR, &dpr_reg);
	ret |= regmap_read(globol_priv_data->regmap, FSA4480_DETM_V_DNL, &dpl_reg);
	ret |= regmap_read(globol_priv_data->regmap, FSA4480_DETM_V_CCIN, &cc_reg);
	ret |= regmap_read(globol_priv_data->regmap, FSA4480_FUNC_CTL0, &en_reg);

	ret |= regmap_read(globol_priv_data->regmap, FSA4480_FUNC_CTL1, &mult_pin_det);
	ret |= regmap_read(globol_priv_data->regmap, FSA4480_DET_CTL1, &inter_time);
	ret |= regmap_read(globol_priv_data->regmap, FSA4480_DET_CTL0, &current_source_en);
	ret |= regmap_read(globol_priv_data->regmap, FSA4480_FUNC_CTL0, &lpd_en_status);
	ret |= regmap_read(globol_priv_data->regmap, FSA4480_DET_V_TH, &volt_th);

	if (ret) {
		mca_log_err("get reg failed\n");
		return ret;
	}

	mca_log_info("sbu1 = 0x%x, sbu2 = 0x%x, dpr = 0x%x, dpl = 0x%x, cc_in= 0x%x, en =0x%x\n",
		sbu1_reg, sbu2_reg, dpr_reg, dpl_reg, cc_reg, en_reg);

	mca_log_info("0X16  = 0x%x, 0X31  = 0x%x, 0X13  = 0x%x, 0X12  = 0x%x, 0X32  = 0x%x\n,",
				mult_pin_det, inter_time, current_source_en, lpd_en_status, volt_th);
	return ret;
}

int mca_lpd_init_reg(struct fsa4480_priv *fsa_priv)
{
	int ret = 0;

	ret |= regmap_write(fsa_priv->regmap, FSA4480_FUNC_CTL1, 0x05); //0x16 bit2 set1
	ret |= regmap_write(fsa_priv->regmap, FSA4480_DET_CTL1, 0x33); //0x31 write 0x2F
	ret |= regmap_write(fsa_priv->regmap, FSA4480_DET_V_TH, 0xD0); //0x32 write 0xD0 1.95V
	ret |= regmap_write(fsa_priv->regmap, FSA4480_DET_CTL0, 0x08); //0x13 write 0x08
	ret |= regmap_write(fsa_priv->regmap, FSA4480_SYS_MSK, 0x0D); //0x19 write 0x0D
	ret |= regmap_write(fsa_priv->regmap, FSA4480_OVP_MSK, 0x7F); //0x01 write 0x7F

	if (ret)
		mca_log_err("init 4480 lpd reg failed\n");
	else
		mca_log_info("init 4480 lpd reg successful\n");
	return ret;
}

static int mca_get_lpd_event_cb(struct notifier_block *nb,
			unsigned long event, void *data)
{
	struct fsa4480_priv *fsa_priv = container_of(nb, struct fsa4480_priv, lpd_nb);

	mca_log_info("%s, event = %lu\n", __func__, event);

	switch (event) {
	case MCA_EVENT_USB_CONNECT:
		mca_queue_delayed_work(&fsa_priv->lpd_irq_work, msecs_to_jiffies(2000));
		break;
	case MCA_EVENT_USB_DISCONNECT:
		globol_priv_data->start_det_flag = false;
		cancel_delayed_work_sync(&fsa_priv->lpd_irq_work);
		mca_set_lpd_enable(false);
		mca_get_pin_val();
		mca_log_info("end lpd\n");
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

irqreturn_t fsa4480_irq_handler(int irq, void *dev_id)
{
	int int_status_reg_18;
	int lpd_status;

	mca_log_info("%s: trigger\n", __func__);
	mca_check_lpd_ovp();
	mca_get_lpd_trigger_status(&int_status_reg_18);

	if (!globol_priv_data->start_det_flag)
		return IRQ_HANDLED;
	mca_get_pin_val();
	lpd_status = 1;
	mca_event_block_notify(MCA_EVENT_TYPE_HW_INFO, MCA_EVENT_LPD_STATUS_CHANGE, &lpd_status);
	return IRQ_HANDLED;
}

static int mca_init_gpio_irq(struct fsa4480_priv *fsa_priv)
{
	int ret = 0;

	fsa_priv->gpio_irq = of_get_named_gpio(fsa_priv->dev->of_node, "lpd-irq-gpio", 0);
	if (!gpio_is_valid(fsa_priv->gpio_irq)) {
		mca_log_err("%s:failed to parse lpd irq gpio\n", __func__);
		return -EINVAL;
	}

	ret = devm_gpio_request(fsa_priv->dev, fsa_priv->gpio_irq, dev_name(fsa_priv->dev));
	if (ret < 0) {
		mca_log_err("%s:lpd request gpio failed\n", __func__);
		return ret;
	}

	fsa_priv->lpd_irq = gpio_to_irq(fsa_priv->gpio_irq);
	if (fsa_priv->lpd_irq < 0) {
		mca_log_err("%s:lpd get gpio irq failed\n", __func__);
		return -EINVAL;
	}

	ret = devm_request_threaded_irq(fsa_priv->dev, fsa_priv->lpd_irq,
			NULL, fsa4480_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT, dev_name(fsa_priv->dev), fsa_priv);
	if (ret < 0) {
		mca_log_err("%s:lpd request irq failed\n", __func__);
		return ret;
	}
	return ret;
}

static void mca_lpd_irq_workfunc(struct work_struct *work)
{
	static int real_type = XM_CHARGER_TYPE_UNKNOW;
	int lpd_status_reg;

	platform_class_buckchg_ops_get_real_type(MAIN_BUCK_CHARGER, &real_type);
	mca_log_info("lpd get charge type = %d\n", real_type);

	if ((real_type == XM_CHARGER_TYPE_PD) || (real_type == XM_CHARGER_TYPE_PPS)
		|| (real_type == XM_CHARGER_TYPE_DCP) || (real_type == XM_CHARGER_TYPE_HVDCP2)) {
		globol_priv_data->start_det_flag = false;
		mca_check_lpd_ovp();
		mca_get_lpd_trigger_status(&lpd_status_reg);
		mca_set_lpd_enable(true);
		msleep(1000);
		mca_set_lpd_enable(false);
		globol_priv_data->start_det_flag = true;
		mca_check_lpd_ovp();
		mca_get_lpd_trigger_status(&lpd_status_reg);
		mca_set_lpd_enable(true);
		msleep(1000);
		mca_get_pin_val();
		mca_log_info("start lpd detect\n");
	} else
		mca_log_info("charge type don't match can't start lpd\n");
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
		ret_size += snprintf(buf, 50, "%s: %d\n", mode, value);
		break;
	case ET_DBG_REG_MODE:
		dump_register();
		break;
	case ET_DBG_UART_MODE:
		ret_size += snprintf(buf, 50, "uart_mode: %d\n", globol_priv_data->uart_mode);
		pr_info("%s: [read] uart_mode:%d\n",  __func__, globol_priv_data->uart_mode);
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

	pr_info("%s: +", __func__);
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
	case ET_DBG_UART_MODE:
		pr_info("%s: [write] uart_mode:%d\n", __func__, globol_priv_data->uart_mode);
		globol_priv_data->uart_mode = value;
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
	static DEVICE_ATTR(name, 0644, show_##name, set_##name);

AUDIO_SW_DEVICE_SHOW_SET(audio_sw_switch_mode, ET_DBG_TYPE_MODE);
AUDIO_SW_DEVICE_SHOW_SET(audio_sw_reg, ET_DBG_REG_MODE);
AUDIO_SW_DEVICE_SHOW_SET(uart_mode, ET_DBG_UART_MODE);
static struct attribute *audio_sw_attrs[] = {
	&dev_attr_audio_sw_switch_mode.attr,
	&dev_attr_uart_mode.attr,
	&dev_attr_audio_sw_reg.attr,
	NULL
};

static const struct attribute_group audio_sw_group = {
	.attrs = audio_sw_attrs,
};


#define AUDIO_SWITCH_MODULE_NAME "audio_sw"

static void audio_switch_debugfs_destroy(struct fsa4480_priv *priv)
{
	debugfs_remove(priv->debug_root);
	priv->debug_root = NULL;
}

int acore_reg_notifier(struct notifier_block *nb)
{
	int rc = 0;

	pr_info("%s: +", __func__);

	if (!globol_priv_data) {
		pr_err("%s:globol_priv_data is NULL !!!!!!!!\n", __func__);
		return -EINVAL;
	}
	rc = blocking_notifier_chain_register(&acore_notifier, nb);

	pr_info("%s: registered notifier\n", __func__);

	return rc;
}
EXPORT_SYMBOL_GPL(acore_reg_notifier);


int acore_unreg_notifier(struct notifier_block *nb)
{
	int rc = 0;

	pr_info("%s: +", __func__);
	if (!globol_priv_data) {
		pr_err("%s:globol_priv_data is NULL !!!!!!!!\n", __func__);
		return -EINVAL;
	}
	rc = blocking_notifier_chain_unregister(&acore_notifier, nb);
	return rc;
}
EXPORT_SYMBOL_GPL(acore_unreg_notifier);

static int fsa4480_probe(struct i2c_client *i2c)
{
	struct device *dev = &i2c->dev;
	struct device_node *node = dev->of_node;
	struct fsa4480_priv *fsa_priv;
	u32 lpd_use;
	u32 uart_mode;
	int rc = 0;
	int ret = 0;

	pr_info("%s: +", __func__);
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

	pr_info("%s: register_tcp_dev_notifier\n", __func__);
	fsa_priv->tcpc_dev = tcpc_dev_get_by_name("type_c_port0");
	if (!fsa_priv->tcpc_dev) {
		rc = -EPROBE_DEFER;
		pr_err("%s get tcpc device type_c_port0 fail\n", __func__);
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

	rc = of_property_read_u32(fsa_priv->dev->of_node, "mi,uart_mode", &uart_mode);
	if (uart_mode) {
		fsa_priv->uart_mode = uart_mode;
		dev_info(fsa_priv->dev, "%s: mi uart_mode:%d\n", __func__, fsa_priv->uart_mode);
	} else {
		fsa_priv->uart_mode = 0;
		dev_info(fsa_priv->dev, "%s: mi set default uart_mode:%d\n", __func__, fsa_priv->uart_mode);
	}
	mutex_init(&fsa_priv->notification_lock);
	i2c_set_clientdata(i2c, fsa_priv);
	atomic_set(&(fsa_priv->orientation_status), -1);
	atomic_set(&(fsa_priv->pd_connect_status), PD_CONNECT_NONE);

	/* get pinctrl handle */
	fsa_priv->pinctrl_t = devm_pinctrl_get(fsa_priv->dev);
	if (IS_ERR_OR_NULL(fsa_priv->pinctrl_t)) {
		rc = PTR_ERR(fsa_priv->pinctrl_t);
		dev_info(fsa_priv->dev, "get audio pinctrl fail, ret = %d.", rc);
	} else {
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

	/*for mca lpd use*/
	fsa_priv->support_lpd = of_property_read_bool(node, "support-external-lpd");
	if (!fsa_priv->support_lpd)
		goto EXIT_LPD_INIT;

	ret = mca_init_gpio_irq(fsa_priv);
	if (ret) {
		mca_log_err("request lpd irq gpio failed\n");
		goto err_data;
	}

	enable_irq_wake(fsa_priv->lpd_irq);
	if (!fsa_priv->lpd_irq) {
		mca_log_err("get lpd irq: failed %d\n", fsa_priv->lpd_irq);
		goto err_data;
	}

	ret = mca_lpd_init_reg(fsa_priv);
	if (ret)
		mca_log_err("init reg failed\n");
	else
		mca_log_info("init reg successful\n");

	fsa_priv->lpd_nb.notifier_call = mca_get_lpd_event_cb;
	ret = mca_event_block_notify_register(MCA_EVENT_TYPE_CHARGER_CONNECT, &fsa_priv->lpd_nb);
	if (ret)
		mca_log_err("register lpd notify failed\n");
	else
		mca_log_info("register lpd notify successful\n");
	INIT_DELAYED_WORK(&fsa_priv->lpd_irq_work, mca_lpd_irq_workfunc);
	/*end*/

EXIT_LPD_INIT:
	set_uart_pinctl(0);
	INIT_WORK(&fsa_priv->usbc_analog_work,
		  fsa4480_usbc_analog_work_fn);
	rc = sysfs_create_group(&i2c->dev.kobj, &audio_sw_group);
	if (rc)
		pr_err("%s: create attr error %d\n", __func__, rc);

	BLOCKING_INIT_NOTIFIER_HEAD(&fsa_priv->fsa4480_notifier);

	pr_info("%s(), setup enable timer", __func__);
	timer_setup(&sw_enable_timer, sw_enable_switch_handler, 0);
	/* delay 2s to register tcpc event change, after accdet init done */
	// delay_init_workqueue = create_singlethread_workqueue("delayInitQueue");
	INIT_WORK(&fsa_priv->delay_init_work, delay_init_work_callback);
	timer_setup(&delay_init_timer, delay_init_handler, 0);
	mod_timer(&delay_init_timer, jiffies + DELAY_INIT_TIME);

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
MODULE_SOFTDEP("pre: external_tcpc");
