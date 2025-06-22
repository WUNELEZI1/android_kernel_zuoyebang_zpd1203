/*
 * fts.c
 *
 * FTS Capacitive touch screen controller (FingerTipS)
 *
 * Copyright (C) 2016, STMicroelectronics Limited.
 * Authors: AMG(Analog Mems Group)
 *
 *		marco.cali@st.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
 * PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
 */

/*!
* \file fts.c
* \brief It is the main file which contains all the most important functions generally used by a device driver the driver
*/
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spi.h>
#include <linux/completion.h>
#ifdef CONFIG_SECURE_TOUCH
#include <linux/atomic.h>
#include <linux/sysfs.h>
#include <linux/hardirq.h>
#endif

#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>

#include <linux/notifier.h>
#include <linux/backlight.h>
#if defined(CONFIG_DRM)
#include <soc/xring/display/panel_event_notifier.h>
#include <drm/drm_panel.h>
#elif defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif
#include <linux/fb.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/pinctrl/consumer.h>
#ifdef KERNEL_ABOVE_2_6_38
#include <linux/input/mt.h>
#endif

#include "fts.h"
#include "fts_lib/ftsCompensation.h"
#include "fts_lib/ftsCore.h"
#include "fts_lib/ftsIO.h"
#include "fts_lib/ftsError.h"
#include "fts_lib/ftsFlash.h"
#include "fts_lib/ftsFrame.h"
#include "fts_lib/ftsGesture.h"
#include "fts_lib/ftsTest.h"
#include "fts_lib/ftsTime.h"
#include "fts_lib/ftsTool.h"
#include <linux/power_supply.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/time64.h>
#include <linux/version.h>
#include <soc/xring/doze.h>
#include <dt-bindings/xring/platform-specific/pm/include/sys_doze_plat.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h>
#include <linux/mutex.h>
#include "tui_common.h"
#include "spi-xr.h"

#if defined(CONFIG_DRM)
static struct device_node *panel_node;
#endif
/**
 * Event handler installer helpers
 */
#define event_id(_e)	 (EVT_ID_##_e>>4)
#define handler_name(_h) fts_##_h##_event_handler

#define install_handler(_i, _evt, _hnd) \
do { \
	_i->event_dispatch_table[event_id(_evt)] = handler_name(_hnd); \
} while (0)

#ifdef KERNEL_ABOVE_2_6_38
#define TYPE_B_PROTOCOL
#endif
extern SysInfo systemInfo;
#ifdef GESTURE_MODE
extern struct mutex gestureMask_mutex;
#endif

char tag[8] = "[ FTS ]\0";
/* buffer which store the input device name assigned by the kernel	*/
char fts_ts_phys[64];
extern spinlock_t fts_int;
struct fts_ts_info *fts_info;
static bool in_tui_state;
static struct mutex tui_mutex;
static int fts_init_sensing(struct fts_ts_info *info);
static irqreturn_t fts_event_handler(int irq, void *ts_info);
static int fts_enable_reg(struct fts_ts_info *info, bool enable);
static int fts_chip_initialization(struct fts_ts_info *info, int init_type);
static int fts_mode_handler(struct fts_ts_info *info, int force);

static ssize_t fts_fwupdate_show(struct device *dev,
								 struct device_attribute *attr, char *buf)
{
	struct fts_ts_info *info = dev_get_drvdata(dev);

	/*fwupdate_stat: ERROR code Returned by flashProcedure. */
	return snprintf(buf, PAGE_SIZE, "{ %08X }\n", info->fwupdate_stat);
}

static ssize_t fts_fwupdate_store(struct device *dev,
								  struct device_attribute *attr,
								  const char *buf, size_t count)
{
	int ret, mode[2];
	char path[101];
	struct fts_ts_info *info = dev_get_drvdata(dev);

	/* by default(if not specified by the user) set the force = 0 and keep_cx to 1 */
	mode[0] = 0;
	mode[1] = 1;

	/* reading out firmware upgrade parameters */
	ret = sscanf(buf, "%100s %d %d", path, &mode[0], &mode[1]);
	if(ret < 0)
		logError(0, "failed to reading out firmware upgrade parameters\n");
	logError(0, "%s fts_fwupdate_store: mode = %s \n", tag, path);

	ret = flashProcedure(path, mode[0], mode[1]);

	info->fwupdate_stat = ret;

	if (ret < OK)
		logError(1, "%s  %s Unable to upgrade firmware! ERROR %08X\n",
				 tag, __func__, ret);
	return count;
}

static DEVICE_ATTR(fwupdate, (S_IRUGO | S_IWUSR | S_IWGRP), fts_fwupdate_show,
				   fts_fwupdate_store);
static struct attribute *fts_attr_group[] = {
	&dev_attr_fwupdate.attr,
	NULL,
};


static const char *fts_get_config(struct fts_ts_info *info);

#ifndef FW_UPDATE_ON_PROBE

/**
*	Function called by the delayed workthread executed after the probe in order to perform the fw update flow
*	@see  fts_fw_update()
*/
static void fts_fw_update_auto(struct work_struct *work)
{
	struct delayed_work *fwu_work =
		container_of(work, struct delayed_work, work);
	struct fts_ts_info *info =
		container_of(fwu_work, struct fts_ts_info, fwu_work);
	fts_fw_update(info, NULL, 0);
}
#endif

void release_all_touches(struct fts_ts_info *info)
{
	unsigned int type = MT_TOOL_FINGER;
	int i;

	for (i = 0; i < TOUCH_ID_MAX; i++) {
		input_mt_slot(info->input_dev, TOUCH_ID_MAX - 1 - i);
		input_mt_report_slot_state(info->input_dev, type, 0);
		input_report_abs(info->input_dev, ABS_MT_TRACKING_ID, -1);
		info->last_x[i] = info->last_y[i] = 0;
	}
	input_sync(info->input_dev);
	info->touch_id = 0;
	info->temp_touch_id = 0;
	info->touch_skip = 0;
	info->width_major = 0;
	info->width_minor = 0;
	info->orientation = 0;
	logError(0, "%s release all touches\n", tag);
}

static const char *fts_get_config(struct fts_ts_info *info)
{
	struct fts_hw_platform_data *pdata = info->board;
	int i = 0, ret = 0;

	if (!info->lockdown_is_ok) {
		logError(1, "%s can't read lockdown info", tag);
		return pdata->default_fw_name;
	}

	ret |= fts_enableInterrupt();

	for (i = 0; i < pdata->config_array_size; i++) {
		if (info->lockdown_info[1] == pdata->config_array[i].tp_vendor) {
			if (pdata->config_array[i].tp_module != U8_MAX &&
				info->lockdown_info[7] == pdata->config_array[i].tp_module) {
				break;
			} else if (pdata->config_array[i].tp_module == U8_MAX)
				break;
		}
	}

	if (i >= pdata->config_array_size) {
		logError(1, "%s can't find right config, i:%d, array_size:%d", tag, i, pdata->config_array_size);
		return pdata->default_fw_name;
	}

	logError(0, "%s Choose config %d: %s", tag, i,
			 pdata->config_array[i].fts_cfg_name);
	pdata->current_index = i;
	return pdata->config_array[i].fts_cfg_name;
}

static int fts_chip_initialization(struct fts_ts_info *info, int init_type)
{
	int ret2 = 0;
	int retry;
	int initretrycnt = 0;

	for (retry = 0; retry <= RETRY_INIT_BOOT; retry++) {
		ret2 = production_test_initialization(init_type);
		if (ret2 == OK)
			break;
		initretrycnt++;
		logError(1,
				 "%s initialization cycle count = %04d - ERROR %08X \n",
				 tag, initretrycnt, ret2);
		fts_chip_powercycle(info);
	}

	if (ret2 < OK) {
		logError(1, "%s fts initialization failed 3 times \n", tag);
	}

	return ret2;
}

int fts_chip_powercycle(struct fts_ts_info *info)
{
	int error = 0;

	logError(0, "%s %s: Power Cycle Starting... \n", tag, __func__);
	logError(0, "%s %s: Disabling IRQ... \n", tag, __func__);

	fts_disableInterruptNoSync();

	if (info->vdd_reg) {
		error = regulator_disable(info->vdd_reg);
		if (error < 0) {
			logError(1, "%s %s: Failed to disable DVDD regulator\n",
					 tag, __func__);
		}
	}

	if (info->avdd_reg) {
		error = regulator_disable(info->avdd_reg);
		if (error < 0) {
			logError(1, "%s %s: Failed to disable AVDD regulator\n",
					 tag, __func__);
		}
	}

	if (info->avddold_reg) {
		error = regulator_disable(info->avddold_reg);
		if (error < 0) {
			logError(1, "%s %s: Failed to disable AVDD SOURCE regulator\n",
					 tag, __func__);
		}
	}

	if (info->board->reset_gpio != GPIO_NOT_DEFINED)
		gpio_set_value(info->board->reset_gpio, 0);
	else
		mdelay(300);

	if (info->vdd_reg) {
		error = regulator_enable(info->vdd_reg);
		if (error < 0) {
			logError(1, "%s %s: Failed to enable DVDD regulator\n",
					 tag, __func__);
		}
	}


	mdelay(1);

	if (info->avdd_reg) {
		error = regulator_enable(info->avdd_reg);
		if (error < 0) {
			logError(1, "%s %s: Failed to enable AVDD regulator\n",
					 tag, __func__);
		}
	}

	mdelay(1);

	if (info->avddold_reg) {
		error = regulator_enable(info->avddold_reg);
		if (error < 0) {
			logError(1, "%s %s: Failed to enable AVDD SOURCE regulator\n",
					 tag, __func__);
		}
	}

	mdelay(5);

	if (info->board->reset_gpio != GPIO_NOT_DEFINED) {
		mdelay(10);
		gpio_set_value(info->board->reset_gpio, 1);
	}

	release_all_touches(info);

	logError(0, "%s %s: Power Cycle Finished! ERROR CODE = %08x\n", tag,
			 __func__, error);
	setSystemResetedUp(1);
	setSystemResetedDown(1);
	return error;
}

#if defined(CONFIG_DRM)
/**
 * pointer panel_node initlized function, used to checkout panel(config) from devices
 * tree ,later will be passed to drm_notifyXXX function.
 * @param np node contains the panel
 * @return pointer to panel node if panel truly exists, otherwise negative number
 */

static int fts_ts_check_panel_node(struct device_node *np)
{
	int i;
	int count;
	struct device_node *node;

	count = of_count_phandle_with_args(np, "dsi-panel", NULL);
	if (count <= 0)
		return -ENODEV;

	panel_node = NULL;
	for (i = 0; i < count; i++) {
		node = of_parse_phandle(np, "dsi-panel", i);
		if (!IS_ERR_OR_NULL(node)) {
			panel_node = node;
			return 0;
		}
	}

	return -ENODEV;
}

static void fts_drm_panel_notifier_callback(enum xring_panel_event_tag notifier_tag,
		 struct xring_panel_event_notification *notification, void *client_data)
{
	struct fts_ts_info *info = client_data;

	if (!notification) {
		logError(1, "%s %s: Invalid notification\n", tag, __func__);
		return;
	}

	switch (notification->type) {
	case DRM_PANEL_EVENT_UNBLANK:
		if (!info->resume_bit) {
			flush_workqueue(info->event_wq);
			queue_work(info->event_wq, &info->resume_work);
		}
		break;
	case DRM_PANEL_EVENT_BLANK:
		flush_workqueue(info->event_wq);
		queue_work(info->event_wq, &info->suspend_work);
		break;
	case DRM_PANEL_EVENT_FPS_CHANGE:
		break;
	default:
		logError(1, "%s %s: notification serviced :%d\n",
				tag, __func__,
				notification->type);
		break;
	}

}

#endif


static int fts_gpio_setup(int gpio, bool config, int dir, int state)
{
	int retval = 0;
	unsigned char buf[16];

	if (config) {
		if (!fts_info->gpio_has_request) {
			snprintf(buf, 16, "fts_gpio_%u\n", gpio);
			retval = gpio_request(gpio, buf);
			if (retval) {
				logError(1, "%s %s: Failed to get gpio %d (code: %d)",
						 tag, __func__, gpio, retval);
				return retval;
			}
		}

		if (dir == 0)
			retval = gpio_direction_input(gpio);
		else
			retval = gpio_direction_output(gpio, state);
		if (retval) {
			logError(1, "%s %s: Failed to set gpio %d direction",
					 tag, __func__, gpio);
			return retval;
		}
	} else {
		gpio_free(gpio);
	}

	return retval;
}

/**
 * Setup the IRQ and RESET (if present) gpios.
 * If the Reset Gpio is present it will perform a cycle HIGH-LOW-HIGH in order to assure that the IC has been reset properly
 */
static int fts_set_gpio(struct fts_ts_info *info, bool alway_output_low)
{
	int retval;
	struct fts_hw_platform_data *bdata = info->board;

	retval = fts_gpio_setup(bdata->irq_gpio, true, alway_output_low ? 1 : 0, 0);
	if (retval < 0) {
		logError(1, "%s %s: Failed to configure irq GPIO\n", tag,
				 __func__);
		goto err_gpio_irq;
	}

	if (bdata->reset_gpio >= 0) {
		retval = fts_gpio_setup(bdata->reset_gpio, true, 1, alway_output_low ? 0 : 1);
		if (retval < 0) {
			logError(1, "%s %s: Failed to configure reset GPIO\n",
					 tag, __func__);
			goto err_gpio_reset;
		}
	}
	info->gpio_has_request = true;
	return OK;

err_gpio_reset:
	fts_gpio_setup(bdata->irq_gpio, false, 0, 0);
	bdata->reset_gpio = GPIO_NOT_DEFINED;
err_gpio_irq:
	return retval;
}

static int fts_pinctrl_init(struct fts_ts_info *info)
{
	int retval = 0;

	info->ts_pinctrl = devm_pinctrl_get(info->dev);
	if (IS_ERR_OR_NULL(info->ts_pinctrl)) {
		retval = PTR_ERR(info->ts_pinctrl);
		dev_err(info->dev, "Target does not use pinctrl %d\n", retval);
		goto err_pinctrl_get;
	}

	info->pinctrl_state_active
		= pinctrl_lookup_state(info->ts_pinctrl, "default");
	if (IS_ERR_OR_NULL(info->pinctrl_state_active)) {
		retval = PTR_ERR(info->pinctrl_state_active);
		dev_err(info->dev, "Can not lookup %s pinstate %d\n",
			"default", retval);
		goto err_pinctrl_lookup;
	}

	info->pinctrl_state_suspend
		= pinctrl_lookup_state(info->ts_pinctrl, "sleep");
	if (IS_ERR_OR_NULL(info->pinctrl_state_suspend)) {
		retval = PTR_ERR(info->pinctrl_state_suspend);
		dev_err(info->dev, "Can not lookup %s pinstate %d\n",
			"sleep", retval);
		goto err_pinctrl_lookup;
	}

	info->pinctrl_state_secure
		= pinctrl_lookup_state(info->ts_pinctrl, "secure");
	if (IS_ERR_OR_NULL(info->pinctrl_state_secure)) {
		retval = PTR_ERR(info->pinctrl_state_secure);
		dev_err(info->dev, "Can not lookup %s pinstate %d\n",
			"secure", retval);
		goto err_pinctrl_lookup;
	}

	return 0;
err_pinctrl_lookup:
	devm_pinctrl_put(info->ts_pinctrl);
err_pinctrl_get:
	info->ts_pinctrl = NULL;
	return retval;
}


/**
 * From the name of the power regulator get/put the actual regulator structs (copying their references into fts_ts_info variable)
 * @param info pointer to fts_ts_info which contains info about the device and its hw setup
 * @param get if 1, the regulators are get otherwise they are put (released) back to the system
 * @return OK if success or an error code which specify the type of error encountered
 */
static int fts_get_reg(struct fts_ts_info *info, bool get)
{
	int retval;
	const struct fts_hw_platform_data *bdata = info->board;

	if (!get) {
		retval = 0;
		goto regulator_put;
	}

	if ((bdata->vdd_reg_name != NULL) && (*bdata->vdd_reg_name != 0)) {
		info->vdd_reg = regulator_get(info->dev, bdata->vdd_reg_name);
		if (IS_ERR(info->vdd_reg)) {
			logError(1, "%s %s: Failed to get pullup regulator\n",
					 tag, __func__);
			retval = PTR_ERR(info->vdd_reg);
			goto regulator_put;
		}
	}

	if ((bdata->avdd_reg_name != NULL) && (*bdata->avdd_reg_name != 0)) {
		info->avdd_reg = regulator_get(info->dev, bdata->avdd_reg_name);
		if (IS_ERR(info->avdd_reg)) {
			logError(1,
					 "%s %s: Failed to get bus power regulator\n",
					 tag, __func__);
			retval = PTR_ERR(info->avdd_reg);
			goto regulator_put;
		}
	}
	return OK;

regulator_put:
	if (info->vdd_reg) {
		regulator_put(info->vdd_reg);
		info->vdd_reg = NULL;
	}

	if (info->avdd_reg) {
		regulator_put(info->avdd_reg);
		info->avdd_reg = NULL;
	}

	return retval;
}

static int fts_enable_reg(struct fts_ts_info *info, bool enable)
{
	int retval = 0;
	const struct fts_hw_platform_data *bdata = info->board;

	if (bdata->platform_flag == FPGA_PLATFORM) {
		if (!enable) {
			retval = 0;
			return retval;
		}
		retval = gpio_request(bdata->avdd_gpio, "AVDD-GPIO");
		if (retval) {
			logError(1, "%s %s: Failed to get gpio %d (code: %d)",
					 tag, __func__, bdata->avdd_gpio, retval);
			return retval;
		}

		retval = gpio_request(bdata->vdd_gpio, "VDD-GPIO");
		if (retval) {
			logError(1, "%s %s: Failed to get gpio %d (code: %d)",
					tag, __func__, bdata->vdd_gpio, retval);
			return retval;
		}

		mdelay(2);
		retval = gpio_direction_output(bdata->vdd_gpio, 1);
		if (retval) {
			logError(1, "%s %s: Failed to set gpio %d direction",
					 tag, __func__, bdata->vdd_gpio);
		}

		mdelay(3);
		retval = gpio_direction_output(bdata->avdd_gpio, 1);
		if (retval) {
			logError(1, "%s %s: Failed to set gpio %d direction",
					 tag, __func__, bdata->avdd_gpio);
		}

		return OK;
	} else {
		logError(0, "%s %s: enable power regulator:%d\n",
			 tag, __func__, enable);
		if (!enable) {
			retval = 0;
			goto disable_pwr_reg;
		}

		if (info->vdd_reg) {
			retval = regulator_enable(info->vdd_reg);
			if (retval < 0) {
				logError(1, "%s %s: Failed to enable bus regulator\n",
						 tag, __func__);
				goto exit;
			}
		}

		if (info->avdd_reg) {
			retval = regulator_enable(info->avdd_reg);
			if (retval < 0) {
				logError(1, "%s %s: Failed to enable power regulator\n",
						 tag, __func__);
				goto disable_bus_reg;
			}
		}

		return OK;

	disable_pwr_reg:
		if (info->avdd_reg)
			regulator_disable(info->avdd_reg);

	disable_bus_reg:
		if (info->vdd_reg)
			regulator_disable(info->vdd_reg);
	exit:
		return retval;
	}
}

static int fts_mode_handler(struct fts_ts_info *info, int force)
{
	int res = OK;
	int ret = OK;
	u8 settings[4] = { 0 };

	info->mode = MODE_NOTHING;
	logError(0, "%s %s: Mode Handler starting... \n", tag, __func__);
	switch (info->resume_bit) {
	case 0:	/* screen down */
		logError(0, "%s %s: Screen OFF...\n", tag, __func__);
		/* do sense off in order to avoid the flooding of the fifo with
		 * touch events if someone is touching the panel during suspend
		 */
		logError(0, "%s %s: Sense OFF!\n", tag, __func__);
		/* for speed reason (no need to check echo in this case and
		 * interrupt can be enabled) */
		ret = setScanMode(SCAN_MODE_ACTIVE, 0x00);
		res |= ret;	/* to avoid warning unsused ret variable when
				 * all the features are disabled */
		break;

	case 1:
		logError(0, "%s %s: Screen ON... \n", tag, __func__);

#ifdef GLOVE_MODE
		if ((info->glove_enabled == FEAT_ENABLE && isSystemResettedUp())
			|| force == 1) {
			logError(0, "%s %s: Glove Mode setting... \n", tag,
					 __func__);
			settings[0] = info->glove_enabled;
			ret = setFeatures(FEAT_SEL_GLOVE, settings, 1);
			if (ret < OK) {
				logError(1,
						 "%s %s: error during setting GLOVE_MODE! ERROR %08X\n",
						 tag, __func__, ret);
			}
			res |= ret;

			if (ret >= OK && info->glove_enabled == FEAT_ENABLE) {
				fromIDtoMask(FEAT_SEL_GLOVE,
							 (u8 *)&info->mode,
							 sizeof(info->mode));
				logError(1, "%s %s: GLOVE_MODE Enabled! \n",
						 tag, __func__);
			} else {
				logError(1, "%s %s: GLOVE_MODE Disabled! \n",
						 tag, __func__);
			}

		}
#endif

#ifdef COVER_MODE
		if ((info->cover_enabled == FEAT_ENABLE && isSystemResettedUp())
			|| force == 1) {
			logError(0, "%s %s: Cover Mode setting... \n", tag,
					 __func__);
			settings[0] = info->cover_enabled;
			ret = setFeatures(FEAT_SEL_COVER, settings, 1);
			if (ret < OK) {
				logError(1,
						 "%s %s: error during setting COVER_MODE! ERROR %08X\n",
						 tag, __func__, ret);
			}
			res |= ret;

			if (ret >= OK && info->cover_enabled == FEAT_ENABLE) {
				fromIDtoMask(FEAT_SEL_COVER,
							 (u8 *)&info->mode,
							 sizeof(info->mode));
				logError(1, "%s %s: COVER_MODE Enabled! \n",
						 tag, __func__);
			} else {
				logError(1, "%s %s: COVER_MODE Disabled! \n",
						 tag, __func__);
			}

		}
#endif
#ifdef CHARGER_MODE
		if ((info->charger_enabled > 0 && isSystemResettedUp())
			|| force == 1) {
			logError(0, "%s %s: Charger Mode setting... \n", tag,
					 __func__);

			settings[0] = info->charger_enabled;
			ret = setFeatures(FEAT_SEL_CHARGER, settings, 1);
			if (ret < OK) {
				logError(1,
						 "%s %s: error during setting CHARGER_MODE! ERROR %08X\n",
						 tag, __func__, ret);
			}
			res |= ret;

			if (ret >= OK && info->charger_enabled == FEAT_ENABLE) {
				fromIDtoMask(FEAT_SEL_CHARGER,
							 (u8 *)&info->mode,
							 sizeof(info->mode));
				logError(1, "%s %s: CHARGER_MODE Enabled! \n",
						 tag, __func__);
			} else {
				logError(1, "%s %s: CHARGER_MODE Disabled! \n",
						 tag, __func__);
			}

		}
#endif

#ifdef GRIP_MODE
		if ((info->grip_enabled == FEAT_ENABLE && isSystemResettedUp())
			|| force == 1) {
			logError(0, "%s %s: Grip Mode setting... \n", tag,
					 __func__);
			settings[0] = info->grip_enabled;
			ret = setFeatures(FEAT_SEL_GRIP, settings, 1);
			if (ret < OK) {
				logError(1,
						 "%s %s: error during setting GRIP_MODE! ERROR %08X\n",
						 tag, __func__, ret);
			}
			res |= ret;

			if (ret >= OK && info->grip_enabled == FEAT_ENABLE) {
				fromIDtoMask(FEAT_SEL_GRIP, (u8 *)&info->mode,
							 sizeof(info->mode));
				logError(1, "%s %s: GRIP_MODE Enabled! \n", tag,
						 __func__);
			} else {
				logError(1, "%s %s: GRIP_MODE Disabled! \n",
						 tag, __func__);
			}

		}
#endif
		settings[0] = 0x01;
		logError(0, "%s %s: Sense ON!\n", tag, __func__);
		res |= setScanMode(SCAN_MODE_ACTIVE, settings[0]);
		info->mode |= (SCAN_MODE_ACTIVE << 24);
		setSystemResetedUp(0);
		break;

	default:
		logError(1,
				 "%s %s: invalid resume_bit value = %d! ERROR %08X \n",
				 tag, __func__, info->resume_bit, ERROR_OP_NOT_ALLOW);
		res = ERROR_OP_NOT_ALLOW;
	}

	logError(0, "%s %s: Mode Handler finished! res = %08X mode = %08X \n",
			 tag, __func__, res, info->mode);
	return res;

}

static irqreturn_t fts_event_handler(int irq, void *ts_info)
{
	struct fts_ts_info *info = ts_info;
	int error = 0, count = 0;
	unsigned char regAdd = FIFO_CMD_READALL;
	unsigned char data[FIFO_EVENT_SIZE * FIFO_DEPTH] = {0};
	unsigned char eventId;
	const unsigned char EVENTS_REMAINING_POS = 7;
	const unsigned char EVENTS_REMAINING_MASK = 0x1F;
	unsigned char events_remaining = 0;
	unsigned char *evt_data;
	static char pre_id[3];
	event_dispatch_handler_t event_handler;
	static struct task_struct *touch_task = NULL;
	struct sched_param par = { .sched_priority = MAX_RT_PRIO - 1};
	int irq_gpio = info->board->irq_gpio;
	int io_status_check_cnt = 0;

	mutex_lock(&tui_mutex);
	if (in_tui_state) {
		mutex_unlock(&tui_mutex);
		return IRQ_HANDLED;
	}
	if (touch_task == NULL) {
		touch_task = current;
		sched_setscheduler_nocheck(touch_task, SCHED_FIFO, &par);
	}

	info->irq_status = true;
	error = fts_writeReadU8UX(regAdd, 0, 0, data, FIFO_EVENT_SIZE,
							  DUMMY_FIFO);
	events_remaining = data[EVENTS_REMAINING_POS] & EVENTS_REMAINING_MASK;
	events_remaining = (events_remaining > FIFO_DEPTH - 1) ?
					   FIFO_DEPTH - 1 : events_remaining;
	/*Drain the rest of the FIFO, up to 31 events*/
	if (error == OK && events_remaining > 0) {
		error = fts_writeReadU8UX(regAdd, 0, 0, &data[FIFO_EVENT_SIZE],
								  FIFO_EVENT_SIZE * events_remaining,
								  DUMMY_FIFO);
	}
	if (error != OK) {
		logError(1,
				 "Error (%d) while reading from FIFO in fts_event_handler",
				 error);
	} else {
		for (count = 0; count < events_remaining + 1; count++) {
			evt_data = &data[count * FIFO_EVENT_SIZE];
			if (pre_id[0] == EVT_ID_USER_REPORT	&&
				pre_id[1] == 0x02 &&
				pre_id[2] == 0x18) {
				pre_id[0] = 0;
				pre_id[1] = 0;
				pre_id[2] = 0;
				continue;
			}
			if (evt_data[0] == EVT_ID_NOEVENT)
				break;
			eventId = evt_data[0] >> 4;
			/*Ensure event ID is within bounds*/
			if (eventId < NUM_EVT_ID) {
				event_handler = info->event_dispatch_table[eventId];
				event_handler(info, (evt_data));
				pre_id[0] = evt_data[0];
				pre_id[1] = evt_data[1];
				pre_id[2] = evt_data[2];
			}
		}
	}
	info->irq_status = false;
	input_sync(info->input_dev);
	pm_relax(info->dev);

	/* wait irq_gpio high */
	while (!gpio_get_value(irq_gpio) && io_status_check_cnt < 100) {
		io_status_check_cnt++;
	}
	mutex_unlock(&tui_mutex);
	return IRQ_HANDLED;
}


void fts_input_report_key(struct fts_ts_info *info, int key_code)
{
	mutex_lock(&info->input_report_mutex);
	input_report_key(info->input_dev, key_code, 1);
	input_sync(info->input_dev);
	input_report_key(info->input_dev, key_code, 0);
	input_sync(info->input_dev);
	mutex_unlock(&info->input_report_mutex);
}

#ifdef GESTURE_MODE
/**
 * Event handler for gesture events (EVT_TYPE_USER_GESTURE)
 * Handle gesture events and simulate the click on a different button for any gesture detected (@link gesture_opt Gesture IDs @endlink)
 */
static void fts_gesture_event_handler(struct fts_ts_info *info,
									  unsigned char *event)
{
	int value;
	int needCoords = 0;

	logError(0,
			 "%s  gesture event data: %02X %02X %02X %02X %02X %02X %02X %02X\n",
			 tag, event[0], event[1], event[2], event[3], event[4],
			 event[5], event[6], event[7]);
	if (event[0] == EVT_ID_USER_REPORT && event[1] == EVT_TYPE_USER_GESTURE) {
		needCoords = 1;
		switch (event[2]) {
		case GEST_ID_DBLTAP:
			if (!info->gesture_enabled)
				goto gesture_done;
			value = KEY_WAKEUP;
			logError(1, "%s %s: double tap ! \n", tag, __func__);
			needCoords = 0;
			break;

		case GEST_ID_AT:
			value = KEY_WWW;
			logError(0, "%s %s: @ ! \n", tag, __func__);
			break;

		case GEST_ID_C:
			value = KEY_C;
			logError(0, "%s %s: C ! \n", tag, __func__);
			break;

		case GEST_ID_E:
			value = KEY_E;
			logError(0, "%s %s: e ! \n", tag, __func__);
			break;

		case GEST_ID_F:
			value = KEY_F;
			logError(0, "%s %s: F ! \n", tag, __func__);
			break;

		case GEST_ID_L:
			value = KEY_L;
			logError(0, "%s %s: L ! \n", tag, __func__);
			break;

		case GEST_ID_M:
			value = KEY_M;
			logError(0, "%s %s: M ! \n", tag, __func__);
			break;

		case GEST_ID_O:
			value = KEY_O;
			logError(0, "%s %s: O ! \n", tag, __func__);
			break;

		case GEST_ID_S:
			value = KEY_S;
			logError(0, "%s %s: S ! \n", tag, __func__);
			break;

		case GEST_ID_V:
			value = KEY_V;
			logError(0, "%s %s:  V ! \n", tag, __func__);
			break;

		case GEST_ID_W:
			value = KEY_W;
			logError(0, "%s %s:  W ! \n", tag, __func__);
			break;

		case GEST_ID_Z:
			value = KEY_Z;
			logError(0, "%s %s:  Z ! \n", tag, __func__);
			break;

		case GEST_ID_RIGHT_1F:
			value = KEY_RIGHT;
			logError(0, "%s %s:  -> ! \n", tag, __func__);
			break;

		case GEST_ID_LEFT_1F:
			value = KEY_LEFT;
			logError(0, "%s %s:  <- ! \n", tag, __func__);
			break;

		case GEST_ID_UP_1F:
			value = KEY_UP;
			logError(0, "%s %s:  UP ! \n", tag, __func__);
			break;

		case GEST_ID_DOWN_1F:
			value = KEY_DOWN;
			logError(0, "%s %s:  DOWN ! \n", tag, __func__);
			break;

		case GEST_ID_CARET:
			value = KEY_APOSTROPHE;
			logError(0, "%s %s:  ^ ! \n", tag, __func__);
			break;

		case GEST_ID_LEFTBRACE:
			value = KEY_LEFTBRACE;
			logError(0, "%s %s:  < ! \n", tag, __func__);
			break;

		case GEST_ID_RIGHTBRACE:
			value = KEY_RIGHTBRACE;
			logError(0, "%s %s:  > ! \n", tag, __func__);
			break;

		default:
			logError(0, "%s %s:  No valid GestureID! \n", tag,
					 __func__);
			goto gesture_done;

		}

		if (needCoords == 1)
			readGestureCoords(event);

		fts_input_report_key(info, value);

gesture_done:
		return;
	} else {
		logError(1, "%s %s: Invalid event passed as argument! \n", tag,
				 __func__);
	}

}
#endif

#ifndef I2C_INTERFACE
/* configure manually SPI4 because when no fw is running the chip use
 * SPI3 by default */

static int fts_spi4_mode_set(struct fts_ts_info *info)
{
	u8 cmd[1] = {0x00};
	int error;

	logError(1, "%s Setting SPI4 mode...\n", tag);
	cmd[0] = 0x10;
	error = fts_writeU8UX(FTS_CMD_HW_REG_W, ADDR_SIZE_HW_REG,
						  ADDR_GPIO_DIRECTION, cmd, 1);
	if (error < OK) {
		logError(1, "%s can not set gpio dir ERROR %08X\n",
				 tag, error);
		return error;
	}

	cmd[0] = 0x02;
	error = fts_writeU8UX(FTS_CMD_HW_REG_W, ADDR_SIZE_HW_REG,
						  ADDR_GPIO_PULLUP, cmd, 1);
	if (error < OK) {
		logError(1, "%s can not set gpio pull-up ERROR %08X\n",
				 tag, error);
		return error;
	}
#if defined(ALIX) || defined (SALIXP)
#if defined(ALIX)
	cmd[0] = 0x70;
#else
	cmd[0] = 0x07;
#endif
	error = fts_writeU8UX(FTS_CMD_HW_REG_W, ADDR_SIZE_HW_REG,
						  ADDR_GPIO_CONFIG_REG3, cmd, 1);
	if (error < OK) {
		logError(1, "%s can not set gpio config ERROR %08X\n",
				 tag, error);
		return error;
	}
#else
	cmd[0] = 0x07;
	error = fts_writeU8UX(FTS_CMD_HW_REG_W, ADDR_SIZE_HW_REG,
						  ADDR_GPIO_CONFIG_REG2, cmd, 1);
	if (error < OK) {
		logError(1, "%s can not set gpio config ERROR %08X\n",
				 tag, error);
		return error;
	}
#endif
	cmd[0] = 0x30;
	error = fts_writeU8UX(FTS_CMD_HW_REG_W, ADDR_SIZE_HW_REG,
						  ADDR_GPIO_CONFIG_REG0, cmd, 1);
	if (error < OK) {
		logError(1, "%s can not set gpio config ERROR %08X\n",
				 tag, error);
		return error;
	}

	cmd[0] = SPI4_MASK;
	error = fts_writeU8UX(FTS_CMD_HW_REG_W, ADDR_SIZE_HW_REG, ADDR_ICR, cmd,
						  1);
	if (error < OK) {
		logError(1, "%s can not set spi4 mode ERROR %08X\n",
				 tag, error);
		return error;
	}
	return OK;
}
#endif

int fts_fw_update(struct fts_ts_info *info, const char *fw_name, int force)
{

	u8 error_to_search[4] = {EVT_TYPE_ERROR_CRC_CX_HEAD, EVT_TYPE_ERROR_CRC_CX,
							 EVT_TYPE_ERROR_CRC_CX_SUB_HEAD, EVT_TYPE_ERROR_CRC_CX_SUB
							};
	int retval = 0;
	int retval1 = 0;
	int ret = 0;
	int crc_status = 0;
	int error = 0;
	int init_type = NO_INIT;
#ifdef PRE_SAVED_METHOD
	int keep_cx = 1;
#else
	int keep_cx = 0;
#endif

	logError(0, "%s Fw Auto Update is starting... \n", tag);
	ret = fts_crc_check();
	if (ret > OK) {
		logError(1, "%s %s: CRC Error or NO FW!\n", tag, __func__);
		crc_status = ret;
	} else {
		crc_status = 0;
		logError(0,
				 "%s %s: NO CRC Error or Impossible to read CRC register! \n",
				 tag, __func__);
	}

	if (fw_name == NULL) {
		fw_name = fts_get_config(info);
		if (fw_name == NULL)
			logError(1, "%s not found mached config!", tag);
	}

	if (fw_name) {
		if (force)
			retval = flashProcedure(fw_name, 1, keep_cx);
		else
			retval = flashProcedure(fw_name, crc_status, keep_cx);

		if ((retval & 0xFF000000) == ERROR_FLASH_PROCEDURE) {
			logError(1,
					 "%s %s: firmware update failed and retry! ERROR %08X\n",
					 tag, __func__, retval);
			fts_chip_powercycle(info);
			retval1 = flashProcedure(info->board->default_fw_name, crc_status, keep_cx);
			if ((retval1 & 0xFF000000) == ERROR_FLASH_PROCEDURE) {
				logError(1,
						 "%s %s: firmware update failed again!	ERROR %08X\n",
						 tag, __func__, retval1);
				logError(1, "%s Fw Auto Update Failed!\n", tag);
			}
		}
	}

	logError(0, "%s %s: Verifying if CX CRC Error...\n", tag, __func__,
			 ret);
	ret = fts_system_reset();
	if (ret >= OK) {
		ret = pollForErrorType(error_to_search, 4);
		if (ret < OK) {
			logError(0, "%s %s: No Cx CRC Error Found! \n", tag,
					 __func__);
			logError(0, "%s %s: Verifying if Panel CRC Error... \n",
					 tag, __func__);
			error_to_search[0] = EVT_TYPE_ERROR_CRC_PANEL_HEAD;
			error_to_search[1] = EVT_TYPE_ERROR_CRC_PANEL;
			ret = pollForErrorType(error_to_search, 2);
			if (ret < OK) {
				logError(0,
						 "%s %s: No Panel CRC Error Found! \n",
						 tag, __func__);
				init_type = NO_INIT;
			} else {
				logError(0,
						 "%s %s: Panel CRC Error FOUND! CRC ERROR = %02X\n",
						 tag, __func__, ret);
				init_type = SPECIAL_PANEL_INIT;
			}
		} else {
			logError(1,
					 "%s %s: Cx CRC Error FOUND! CRC ERROR = %02X\n",
					 tag, __func__, ret);

			logError(1,
					 "%s %s: Try to recovery with CX in fw file...\n",
					 tag, __func__, ret);
			flashProcedure(info->board->default_fw_name, CRC_CX, 0);
			logError(1, "%s %s: Refresh panel init data... \n", tag,
					 __func__, ret);
		}
	} else {
		logError(1,
				 "%s %s: Error while executing system reset! ERROR %08X\n",
				 tag, __func__, ret);
	}

	if (init_type != SPECIAL_FULL_PANEL_INIT) {
#ifdef PRE_SAVED_METHOD
		if (systemInfo.u8_cfgAfeVer != systemInfo.u8_cxAfeVer) {
			init_type = SPECIAL_FULL_PANEL_INIT;
			logError(1,
					 "%s %s: Different CX AFE Ver: %02X != %02X... Execute FULL Panel Init! \n",
					 tag, __func__, systemInfo.u8_cfgAfeVer,
					 systemInfo.u8_cxAfeVer);
		} else
#endif

			if (systemInfo.u8_cfgAfeVer != systemInfo.u8_panelCfgAfeVer) {
				init_type = SPECIAL_PANEL_INIT;
				logError(1,
						 "%s %s: Different Panel AFE Ver: %02X != %02X... Execute Panel Init! \n",
						 tag, __func__, systemInfo.u8_cfgAfeVer,
						 systemInfo.u8_panelCfgAfeVer);
			} else {
				init_type = NO_INIT;
			}
	}

	if (init_type != NO_INIT) {
		error = fts_chip_initialization(info, init_type);
		if (error < OK) {
			logError(1,
					 "%s %s Cannot initialize the chip ERROR %08X\n",
					 tag, __func__, error);
		}
	}

	error = fts_init_sensing(info);
	if (error < OK) {
		logError(1,
				 "%s Cannot initialize the hardware device ERROR %08X\n",
				 tag, error);
	}

	logError(1, "%s Fw Update Finished! error = %08X\n", tag, error);
	return error;
}

#define fts_motion_pointer_event_handler fts_enter_pointer_event_handler

static void fts_error_event_handler(struct fts_ts_info *info,
									unsigned char *event)
{
	int error = 0;
	logError(0,
			 "%s %s Received event %02X %02X %02X %02X %02X %02X %02X %02X\n",
			 tag, __func__, event[0], event[1], event[2], event[3],
			 event[4], event[5], event[6], event[7]);

	switch (event[1]) {
	case EVT_TYPE_ERROR_ESD: {
		release_all_touches(info);

		fts_chip_powercycle(info);

		error = fts_system_reset();
		error |= fts_mode_handler(info, 0);
		error |= fts_enableInterrupt();
		if (error < OK) {
			logError(1,
					 "%s %s Cannot restore the device ERROR %08X\n",
					 tag, __func__, error);
		}
	}
	break;
	case EVT_TYPE_ERROR_WATCHDOG: {
		dumpErrorInfo(NULL, 0);
		release_all_touches(info);
		error = fts_system_reset();
		error |= fts_mode_handler(info, 0);
		error |= fts_enableInterrupt();
		if (error < OK) {
			logError(1,
					 "%s %s Cannot reset the device ERROR %08X\n",
					 tag, __func__, error);
		}
	}
	break;

	}
}

/**
* Event handler for controller ready event (EVT_ID_CONTROLLER_READY)
* Handle controller events received after unexpected reset of the IC updating the resets flag and restoring the proper sensing status
*/
static void fts_controller_ready_event_handler(struct fts_ts_info *info,
		unsigned char *event)
{
	int error;
#ifdef DEBUG
	logError(1,
			 "%s %s Received event %02X %02X %02X %02X %02X %02X %02X %02X\n",
			 tag, __func__, event[0], event[1], event[2], event[3],
			 event[4], event[5], event[6], event[7]);
#endif
	release_all_touches(info);
	setSystemResetedUp(1);
	setSystemResetedDown(1);
	error = fts_mode_handler(info, 0);
	if (error < OK) {
		logError(1,
				 "%s %s Cannot restore the device status ERROR %08X\n",
				 tag, __func__, error);
	}
}

/**
* Event handler for status events (EVT_ID_STATUS_UPDATE)
* Handle status update events
*/
#ifdef DEBUG
static void fts_status_event_handler(struct fts_ts_info *info,
									 unsigned char *event)
{
	switch (event[1]) {

	case EVT_TYPE_STATUS_ECHO:
		logError(0,
				 "%s %s Echo event of command = %02X %02X %02X %02X %02X %02X\n",
				 tag, __func__, event[2], event[3], event[4], event[5],
				 event[6], event[7]);
		break;

	case EVT_TYPE_STATUS_FORCE_CAL:
		switch (event[2]) {
		case 0x00:
			logError(1,
					 "%s %s Continuous frame drop Force cal = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x01:
			logError(1,
					 "%s %s Mutual negative detect Force cal = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x02:
			logError(1,
					 "%s %s Mutual calib deviation Force cal = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x11:
			logError(1,
					 "%s %s SS negative detect Force cal = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x12:
			logError(1,
					 "%s %s SS negative detect Force cal in Low Power mode = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x13:
			logError(1,
					 "%s %s SS negative detect Force cal in Idle mode = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x20:
			logError(1,
					 "%s %s SS invalid Mutual Strength soft Force cal = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x21:
			logError(1,
					 "%s %s SS invalid Self Strength soft Force cal = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x22:
			logError(1,
					 "%s %s SS invalid Self Island soft Force cal = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x30:
			logError(1,
					 "%s %s MS invalid Mutual Strength soft Force cal = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x31:
			logError(1,
					 "%s %s MS invalid Self Strength soft Force cal = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		default:
			logError(1,
					 "%s %s Force cal = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);

		}
		break;

	case EVT_TYPE_STATUS_FRAME_DROP:
		switch (event[2]) {
		case 0x01:
			logError(1,
					 "%s %s Frame drop noisy frame = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x02:
			logError(1,
					 "%s %s Frame drop bad R = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		case 0x03:
			logError(1,
					 "%s %s Frame drop invalid processing state = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
			break;

		default:
			logError(1,
					 "%s %s Frame drop = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);

		}
		break;

	case EVT_TYPE_STATUS_SS_RAW_SAT:
		if (event[2] == 1)
			logError(1,
					 "%s %s SS Raw Saturated = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
		else
			logError(1,
					 "%s %s SS Raw No more Saturated = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
		break;

	case EVT_TYPE_STATUS_WATER:
		if (event[2] == 1)
			logError(1,
					 "%s %s Enter Water mode = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
		else
			logError(1,
					 "%s %s Exit Water mode = %02X %02X %02X %02X %02X %02X\n",
					 tag, __func__, event[2], event[3], event[4],
					 event[5], event[6], event[7]);
		break;
	case 0x0c:
		break;
	default:
		logError(1,
				 "%s %s Received unhandled status event = %02X %02X %02X %02X %02X %02X %02X %02X\n",
				 tag, __func__, event[0], event[1], event[2], event[3],
				 event[4], event[5], event[6], event[7]);
		break;
	}

}
#endif

static void fts_user_report_event_handler(struct fts_ts_info *info,
		unsigned char *event)
{

	switch (event[1]) {

#ifdef PHONE_KEY
	case EVT_TYPE_USER_KEY:
		fts_key_event_handler(info, event);
		break;
#endif

	case EVT_TYPE_USER_PROXIMITY:
		if (event[2] == 0) {
#ifdef DEBUG
			logError(1, "%s %s No proximity!\n", tag, __func__);
#endif
		} else {
#ifdef DEBUG
			logError(1, "%s %s Proximity Detected!\n", tag,
					 __func__);
#endif
		}
		break;

#ifdef GESTURE_MODE
	case EVT_TYPE_USER_GESTURE:
		fts_gesture_event_handler(info, event);
		break;
#endif
	default:
#ifdef DEBUG
		logError(1,
				 "%s %s Received unhandled user report event = %02X %02X %02X %02X %02X %02X %02X %02X\n",
				 tag, __func__, event[0], event[1], event[2], event[3],
				 event[4], event[5], event[6], event[7]);
#endif
		break;
	}

}

static void fts_nop_event_handler(struct fts_ts_info *info,
								  unsigned char *event)
{
	logError(0,
			 "%s %s Doing nothing for event = %02X %02X %02X %02X %02X %02X %02X %02X\n",
			 tag, __func__, event[0], event[1], event[2], event[3],
			 event[4], event[5], event[6], event[7]);
}

static void fts_enter_pointer_event_handler(struct fts_ts_info *info,
		unsigned char *event)
{
	unsigned char touchId;
	unsigned int touch_condition = 1, tool = MT_TOOL_FINGER;
	int x, y, distance, angle, major, minor;
	const struct fts_hw_platform_data *bdata = fts_info->board;
	u8 touchType;

	touchType = event[1] & 0x0F;
	touchId = (event[1] & 0xF0) >> 4;

	if (!bdata->support_super_resolution) {
		x = (((int)event[3] & 0x0F) << 8) | (event[2]);
		y = ((int)event[4] << 4) | ((event[3] & 0xF0) >> 4);
	} else {
		x = (((int)event[3]) << 8) | (event[2]);
		y = (((int)event[5]) << 8) | (event[4]);
	}
	distance = 0;
	angle = (signed char)event[5];
	major = (((event[0] & 0x0C) << 2) | ((event[6] & 0xF0) >> 4));
	minor = (((event[7] & 0xC0) >> 2) | (event[6] & 0x0F));
	if ((event[7] & 0x1f) >= 4) {
#ifdef DEBUG
		logError(1, "%s %s %02X %02X %02X %02X %02X %02X %02X %02X\n", tag, __func__,
				 event[0], event[1], event[2], event[3], event[4], event[5], event[6], event[7]);
#endif
	}
	if (x >= info->board->x_max)
		x = info->board->x_max;

	if (y >= info->board->y_max)
		y = info->board->y_max;
	if (info->board->swap_x)
		x = info->board->x_max - x;
	if (info->board->swap_y)
		y = info->board->y_max - y;
	input_mt_slot(info->input_dev, TOUCH_ID_MAX - 1 - touchId);
	switch (touchType) {

	case TOUCH_TYPE_FINGER:
	/*logError(0, "%s  %s : It is a finger!\n",tag,__func__); */
	case TOUCH_TYPE_GLOVE:
	/*logError(0, "%s  %s : It is a glove!\n",tag,__func__); */
	case TOUCH_TYPE_PALM:
		/*logError(0, "%s  %s : It is a palm!\n",tag,__func__); */
		tool = MT_TOOL_FINGER;
		touch_condition = 1;
		if (test_bit(touchId, &info->temp_touch_id)) {
#ifdef DEBUG
			logError(1, "%s  %s : NOTE: same slot in one irq, slot = %d !\n", tag, __func__, touchId);
#endif
			input_sync(info->input_dev);
		}
		__set_bit(touchId, &info->touch_id);
		__set_bit(touchId, &info->temp_touch_id);
		break;

	case TOUCH_TYPE_HOVER:
		tool = MT_TOOL_FINGER;
		touch_condition = 0;
		__set_bit(touchId, &info->touch_id);
		__set_bit(touchId, &info->temp_touch_id);
		distance = DISTANCE_MAX;
		break;

	case TOUCH_TYPE_INVALID:
	default:
#ifdef DEBUG
		logError(1, "%s  %s : Invalid touch type = %d ! No Report...\n",
				 tag, __func__, touchType);
#endif
		break;
	}

	input_mt_report_slot_state(info->input_dev, tool, 1);
	input_report_key(info->input_dev, BTN_TOUCH, touch_condition);
	if (touch_condition)
		input_report_key(info->input_dev, BTN_TOOL_FINGER, 1);

	/*input_report_abs(info->input_dev, ABS_MT_TRACKING_ID, touchId); */
	input_report_abs(info->input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(info->input_dev, ABS_MT_POSITION_Y, y);
	if ((info->last_x[touchId] != x) || (info->last_y[touchId] != y)) {
		input_report_abs(info->input_dev, ABS_MT_TOUCH_MAJOR, major * 16);
		input_report_abs(info->input_dev, ABS_MT_TOUCH_MINOR, minor * 16);
		input_report_abs(info->input_dev, ABS_MT_ORIENTATION, angle);
		input_report_abs(info->input_dev, ABS_MT_DISTANCE, distance);
	}
	info->last_x[touchId] = x;
	info->last_y[touchId] = y;

	return;
}

/**
* Event handler for leave event (EVT_ID_LEAVE_POINT )
* Report to the linux input system that one touch left the display
*/
static void fts_leave_pointer_event_handler(struct fts_ts_info *info,
		unsigned char *event)
{
	unsigned char touchId = 0;
	unsigned int tool = MT_TOOL_FINGER;
	unsigned int touch_condition = 0;
	u8 touchType;

	touchType = event[1] & 0x0F;
	touchId = (event[1] & 0xF0) >> 4;
	input_mt_slot(info->input_dev, TOUCH_ID_MAX - 1 - touchId);

	switch (touchType) {

	case TOUCH_TYPE_FINGER:
		/*logError(0, "%s  %s : It is a finger!\n",tag,__func__); */
	case TOUCH_TYPE_GLOVE:
		/*logError(0, "%s  %s : It is a glove!\n",tag,__func__); */
	case TOUCH_TYPE_PALM:
		/*logError(0, "%s  %s : It is a palm!\n",tag,__func__); */
		tool = MT_TOOL_FINGER;
		touch_condition = 0;
		if (test_bit(touchId, &info->temp_touch_id)) {
#ifdef DEBUG
			logError(1, "%s  %s : NOTE: down and up in one irq, slot = %d !\n", tag, __func__, touchId);
#endif
			input_sync(info->input_dev);
		}
		__clear_bit(touchId, &info->touch_id);
		__clear_bit(touchId, &info->temp_touch_id);
		break;
	case TOUCH_TYPE_HOVER:
		tool = MT_TOOL_FINGER;
		touch_condition = 1;
		__clear_bit(touchId, &info->touch_id);
		__clear_bit(touchId, &info->temp_touch_id);
		break;

	case TOUCH_TYPE_INVALID:
	default:
#ifdef DEBUG
		logError(1, "%s  %s : Invalid touch type = %d ! No Report...\n",
				 tag, __func__, touchType);
#endif
		return;

	}
	__clear_bit(touchId, &info->sleep_finger);
	input_mt_report_slot_state(info->input_dev, tool, 0);

	if (info->touch_id == 0) {
		input_report_key(info->input_dev, BTN_TOUCH, touch_condition);
		if (!touch_condition)
			input_report_key(info->input_dev, BTN_TOOL_FINGER, 0);
#ifdef CONFIG_FTS_BOOST
			lpm_disable_for_dev(false, EVENT_INPUT);
#endif
	}
	info->last_x[touchId] = info->last_y[touchId] = 0;
	input_report_abs(info->input_dev, ABS_MT_TRACKING_ID, -1);
}

const char *fts_get_limit(struct fts_ts_info *info)
{
	struct fts_hw_platform_data *pdata = info->board;
	int i = 0, ret = 0;

	if (!info->lockdown_is_ok) {
		logError(1, "%s can't read lockdown info", tag);
		return LIMITS_FILE;
	}

	ret |= fts_enableInterrupt();

	for (i = 0; i < pdata->config_array_size; i++) {
		if (info->lockdown_info[1] == pdata->config_array[i].tp_vendor) {
			if (pdata->config_array[i].tp_module != U8_MAX &&
				info->lockdown_info[7] == pdata->config_array[i].tp_module) {
				break;
			} else if (pdata->config_array[i].tp_module == U8_MAX)
				break;
		}
	}

	if (i >= pdata->config_array_size) {
		logError(1, "%s can't find right limit", tag);
		return LIMITS_FILE;
	}

	logError(1, "%s Choose limit file %d: %s", tag, i,
			 pdata->config_array[i].fts_limit_name);
	pdata->current_index = i;
	return pdata->config_array[i].fts_limit_name;
}

static void fts_resume_work(struct work_struct *work)
{
	struct fts_ts_info *info;
	int ret;

	info = container_of(work, struct fts_ts_info, resume_work);

	pr_info("%s ++\n", __func__);

	info->resume_bit = 1;

	ret = fts_system_reset();

	release_all_touches(info);

	fts_mode_handler(info, 0);

	fts_enableInterrupt();

	pr_info("%s --\n", __func__);
}

static void fts_suspend_work(struct work_struct *work)
{
	struct fts_ts_info *info;

	info = container_of(work, struct fts_ts_info, suspend_work);

	pr_info("%s ++\n", __func__);

	info->resume_bit = 0;

	fts_mode_handler(info, 0);

	release_all_touches(info);

	fts_disableInterrupt();

	pr_info("%s --\n", __func__);
}


static int parse_dt(struct device *dev, struct fts_hw_platform_data *bdata)
{
	int retval;
	const char *name;
	struct device_node *temp = NULL, *np = dev->of_node;
	struct fts_config_info *config_info;
	u32 temp_val;

	if (of_property_read_bool(np, "fpga-platform"))
		bdata->platform_flag = FPGA_PLATFORM;

	bdata->irq_gpio = of_get_named_gpio(np, "fts,irq-gpio", 0);
	logError(0, "%s irq_gpio = %d\n", tag, bdata->irq_gpio);

	if (bdata->platform_flag == FPGA_PLATFORM) {
		bdata->avdd_gpio = of_get_named_gpio(np, "fts,avdd-gpio", 0);
		bdata->vdd_gpio = of_get_named_gpio(np, "fts,vdd-gpio", 0);
		logError(0, "%s avdd_gpio = %d\n", tag, bdata->avdd_gpio);
		logError(0, "%s vdd_gpio = %d\n", tag, bdata->vdd_gpio);
	}

	retval = of_property_read_string(np, "fts,pwr-reg-name", &name);
	if (retval == -EINVAL)
		bdata->avdd_reg_name = NULL;
	else if (retval < 0)
		return retval;
	else {
		bdata->avdd_reg_name = name;
		logError(0, "%s pwr_reg_name = %s\n", tag, name);
	}

	retval = of_property_read_string(np, "fts,bus-reg-name", &name);
	if (retval == -EINVAL)
		bdata->vdd_reg_name = NULL;
	else if (retval < 0)
		return retval;
	else {
		bdata->vdd_reg_name = name;
		logError(0, "%s bus_reg_name = %s\n", tag, name);
	}

	if (of_property_read_bool(np, "fts,reset-gpio-enable")) {
		bdata->reset_gpio = of_get_named_gpio(np,
							"fts,reset-gpio", 0);
		logError(0, "%s reset_gpio =%d\n", tag, bdata->reset_gpio);
	} else {
		bdata->reset_gpio = GPIO_NOT_DEFINED;
	}

	retval = of_property_read_u32(np, "fts,irq-flags", &temp_val);
	if (retval < 0)
		return retval;
	else
		bdata->irq_flags = temp_val;
	retval = of_property_read_u32(np, "fts,x-max", &temp_val);
	if (retval < 0)
		bdata->x_max = X_AXIS_MAX;
	else
		bdata->x_max = temp_val;

	retval = of_property_read_u32(np, "fts,y-max", &temp_val);
	if (retval < 0)
		bdata->y_max = Y_AXIS_MAX;
	else
		bdata->y_max = temp_val;
	retval = of_property_read_string(np, "fts,default-fw-name",
									 &bdata->default_fw_name);
	bdata->swap_x =
		of_property_read_bool(np, "fts,swap-x");
	bdata->swap_y =
		of_property_read_bool(np, "fts,swap-y");
	retval = of_property_read_u32(np, "fts,support-super-resolution", &temp_val);
	if (retval < 0)
		return retval;
	else
		bdata->support_super_resolution = temp_val;

	retval =
		of_property_read_u32(np, "fts,config-array-size",
							 (u32 *)&bdata->config_array_size);

	if (retval) {
		logError(1, "%s Unable to get array size\n", tag);
		return retval;
	}

	bdata->config_array = devm_kzalloc(dev, bdata->config_array_size *
									   sizeof(struct fts_config_info),
									   GFP_KERNEL);

	if (!bdata->config_array) {
		logError(1, "%s Unable to allocate memory\n", tag);
		return -ENOMEM;
	}

	config_info = bdata->config_array;
	for_each_child_of_node(np, temp) {
		retval = of_property_read_u32(temp, "fts,tp-vendor", &temp_val);

		if (retval) {
			logError(0, "%s Unable to read tp vendor\n", tag);
		} else {
			config_info->tp_vendor = (u8) temp_val;
			logError(0, "%s %s:tp vendor: %u", tag, __func__,
					 config_info->tp_vendor);
		}
		retval = of_property_read_u32(temp, "fts,tp-color", &temp_val);
		if (retval) {
			logError(0, "%s Unable to read tp color\n", tag);
		} else {
			config_info->tp_color = (u8) temp_val;
			logError(0, "%s %s:tp color: %u", tag, __func__,
					 config_info->tp_color);
		}

		retval = of_property_read_u32(temp, "fts,tp-module", &temp_val);
		if (retval) {
			logError(0, "%s Unable to read tp module\n", tag);
			config_info->tp_module = U8_MAX;
		} else {
			config_info->tp_module = (u8) temp_val;
			logError(0, "%s %s:tp module: %u", tag, __func__,
					 config_info->tp_module);
		}

		retval =
			of_property_read_u32(temp, "fts,tp-hw-version", &temp_val);

		if (retval) {
			logError(0, "%s Unable to read tp hw version\n", tag);
		} else {
			config_info->tp_hw_version = (u8) temp_val;
			logError(0, "%s %s:tp color: %u", tag, __func__,
					 config_info->tp_hw_version);
		}

		retval = of_property_read_string(temp, "fts,fw-name",
										 &config_info->fts_cfg_name);

		if (retval && (retval != -EINVAL)) {
			config_info->fts_cfg_name = NULL;
			logError(0, "%s Unable to read cfg name\n", tag);
		} else {
			logError(0, "%s %s:fw_name: %s", tag, __func__,
					 config_info->fts_cfg_name);
		}
		retval = of_property_read_string(temp, "fts,limit-name",
										 &config_info->fts_limit_name);

		if (retval && (retval != -EINVAL)) {
			config_info->fts_limit_name = NULL;
			logError(0, "%s Unable to read limit name\n", tag);
		} else {
			logError(0, "%s %s:limit_name: %s", tag, __func__,
					 config_info->fts_limit_name);
		}
		config_info++;
	}
	return OK;
}

static int fts_init(struct fts_ts_info *info)
{
	int error, retry;
	u8 readData[2];

	error = fts_system_reset();
#ifdef I2C_INTERFACE
	if (error < OK && isI2cError(error)) {
#else
	if (error < OK) {
#endif
		logError(1, "%s Cannot reset the device! ERROR %08X\n", tag, error);
		logError(1, "%s check chip_id to judge there is no fw or no panel\n", tag);
#ifdef SPI4_WIRE
		error = fts_spi4_mode_set(info);
		if (error < OK) {
			logError(1, "%s change spi4 mode error\n", tag);
			return error;
		}
		msleep(1);
#endif
		logError(1, "%s Reading chip id\n", tag);
		for (retry = 0; retry < 3; retry++) {
			msleep(10); /* wait for the GPIO to stabilize */
			error = fts_writeReadU8UX(FTS_CMD_HW_REG_R, ADDR_SIZE_HW_REG, ADDR_DCHIP_ID, readData, 2, DUMMY_FIFO);
			logError(1, "%s chip_id0:0x%x,chip_id1:0x%x\n", tag, readData[0], readData[1]);
			if ((readData[0] == DCHIP_ID_0) && (readData[1] == DCHIP_ID_1))
				break;
		}
		if (retry == 3) {
			logError(1, "%s read chip error,no panel\n", tag);
			return ERROR_OP_NOT_ALLOW;
		} else {
			logError(1, "%s read chip ok, maybe no fw,force update fw\n", tag);
			return OK;
		}
	} else {
		msleep(10);
		if (error == (ERROR_TIMEOUT | ERROR_SYSTEM_RESET_FAIL)) {
			logError(1, "%s Setting default Sys INFO! \n", tag);
			error = defaultSysInfo(0);
		} else {
			error = readSysInfo(0);
			if (error < OK) {
				if (!isI2cError(error))
					error = OK;
				logError(1,
						 "%s Cannot read Sys Info! ERROR %08X\n",
						 tag, error);
			}
		}
	}

	return error;
}

static int fts_interrupt_install(struct fts_ts_info *info)
{
	int i, error = 0;

	info->event_dispatch_table =
		kzalloc(sizeof(event_dispatch_handler_t) * NUM_EVT_ID, GFP_KERNEL);

	if (!info->event_dispatch_table) {
		logError(1, "%s OOM allocating event dispatch table\n", tag);
		return -ENOMEM;
	}

	for (i = 0; i < NUM_EVT_ID; i++)
		info->event_dispatch_table[i] = fts_nop_event_handler;

	install_handler(info, ENTER_POINT, enter_pointer);
	install_handler(info, LEAVE_POINT, leave_pointer);
	install_handler(info, MOTION_POINT, motion_pointer);
	install_handler(info, ERROR, error);
	install_handler(info, CONTROLLER_READY, controller_ready);
#ifdef DEBUG
	install_handler(info, STATUS_UPDATE, status);
#endif
	install_handler(info, USER_REPORT, user_report);

	/* disable interrupts in any case */
	error = fts_disableInterrupt();
	logError(0, "%s Interrupt Mode\n", tag);
	if (request_threaded_irq(info->client->irq, NULL, fts_event_handler, info->board->irq_flags,
							 "xiaomi_tp" FTS_TS_DRV_NAME, info)) {
		logError(1, "%s Request irq failed\n", tag);
		kfree(info->event_dispatch_table);
		error = -EBUSY;
	} else {
		disable_irq(info->client->irq);
	}
	return error;
}

/**
*	Clean the dispatch table and the free the IRQ.
*	This function is called when the driver need to be removed
*/
static void fts_interrupt_uninstall(struct fts_ts_info *info)
{

	fts_disableInterrupt();

	kfree(info->event_dispatch_table);

	free_irq(info->client->irq, info);

}

static int fts_init_sensing(struct fts_ts_info *info)
{
	int error = 0;
	error |= fts_interrupt_install(info);
	error |= fts_mode_handler(info, 0);
	if (error < OK) {
		logError(1, "%s %s Init after Probe error (ERROR = %08X)\n",
				 tag, __func__, error);
		return error;
	}
	error |= fts_enableInterrupt();

	return error;
}
#if IS_ENABLED(CONFIG_XRING_MITEE_TUI_SUPPORT)
static int fts_tpd_enter_tui(void)
{
	int ret;
	//thp_off();
	fts_disableInterrupt();
	mutex_lock(&tui_mutex);
	ret = sys_state_doz2nor_vote(VOTER_DOZAP_TP_GPIO);
	if (ret) {
		logError(1, "%s %s doz2nor vote failed!\n",
				 tag, __func__);
		goto err_doze2nor_vote;
	}
	ret = xr_tui_resource_request(fts_info->client);
	if (ret) {
		logError(1, "%s %s xr_spi enable master clk failed!\n",
				 tag, __func__);
		goto err_xr_tui_resource_req;
	}
	ret = pinctrl_select_state(fts_info->ts_pinctrl, fts_info->pinctrl_state_secure);
	if (ret) {
		logError(1, "%s %s pinctrl select state failed!\n",
				 tag, __func__);
		goto err_pinctrl_select_int;
	}

	in_tui_state = true;
	mutex_unlock(&tui_mutex);
	return ret;
err_pinctrl_select_int:
	xr_tui_resource_release(fts_info->client);
err_xr_tui_resource_req:
	sys_state_doz2nor_unvote(VOTER_DOZAP_TP_GPIO);
err_doze2nor_vote:
	mutex_unlock(&tui_mutex);
	return ret;
}

static int fts_tpd_exit_tui(void)
{
	int ret;

	mutex_lock(&tui_mutex);
	if (!in_tui_state) {
		ret = -1;
		goto exit_fts_tpd_exit_tui;
	}

	ret = pinctrl_select_state(fts_info->ts_pinctrl, fts_info->pinctrl_state_active);
	if (ret) {
		logError(1, "%s %s pinctrl select state failed!\n",
				 tag, __func__);
		goto exit_fts_tpd_exit_tui;
	}

	ret = xr_tui_resource_release(fts_info->client);
	if (ret) {
		logError(1, "%s %s xr_spi disable master clk failed!\n",
				 tag, __func__);
		goto exit_fts_tpd_exit_tui;
	}

	ret = sys_state_doz2nor_unvote(VOTER_DOZAP_TP_GPIO);
	if (ret) {
		logError(1, "%s %s doz2nor unvote failed!\n",
				 tag, __func__);
		goto exit_fts_tpd_exit_tui;
	}

	in_tui_state = false;
	fts_enableInterrupt();

exit_fts_tpd_exit_tui:
	mutex_unlock(&tui_mutex);

	return ret;
}

static enum tpd_vendor_id_t fts_get_vendor_id(void)
{
	return ST_VENDOR_ID;
}
#endif

/**
 * Probe function, called when the driver it is matched with a device with the same name compatible name
 * This function allocate, initialize and define all the most important function and flow that are used by the driver to operate with the IC.
 * It allocates device variables, initialize queues and schedule works, registers the IRQ handler, suspend/resume callbacks, registers the device to the linux input subsystem etc.
 */
#ifdef I2C_INTERFACE
static int fts_probe(struct i2c_client *client, const struct i2c_device_id *idp)
{
#else
static int fts_probe(struct spi_device *client)
{
#endif

	struct fts_ts_info *info = NULL;
	int error = 0;
	struct device_node *dp = client->dev.of_node;
	int retval;
	int skip_5_1 = 0;
	u16 bus_type;
#ifdef CONFIG_TOUCH_FACTORY_BUILD
	int res = 0;
	u8 gesture_cmd[6] = {0xA2, 0x03, 0x00, 0x00, 0x00, 0x03};
#endif
	logError(0, "%s %s: driver spi ver: %s\n", tag, __func__,
			 FTS_TS_DRV_VERSION);
#ifdef I2C_INTERFACE
	logError(1, "%s I2C interface... \n", tag);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		logError(1, "%s Unsupported I2C functionality\n", tag);
		error = -EIO;
		goto ProbeErrorExit_0;
	}

	logError(0, "%s i2c address: %x \n", tag, client->addr);
	bus_type = BUS_I2C;
#else
	logError(0, "%s SPI interface... \n", tag);
	client->mode = SPI_MODE_0;
#ifndef SPI4_WIRE
	client->mode |= SPI_3WIRE;
#endif
	client->max_speed_hz = SPI_CLOCK_FREQ;
	client->bits_per_word = 8;
	if (spi_setup(client) < 0) {
		logError(1, "%s Unsupported SPI functionality\n", tag);
		error = -EIO;
		goto ProbeErrorExit_0;
	}
	bus_type = BUS_SPI;
#endif

	logError(0, "%s SET Device driver INFO: \n", tag);

	info = kzalloc(sizeof(struct fts_ts_info), GFP_KERNEL);
	if (!info) {
		logError(1,
				 "%s Out of memory... Impossible to allocate struct info!\n",
				 tag);
		error = -ENOMEM;
		goto ProbeErrorExit_0;
	}

	fts_info = info;
	info->client = client;
	info->dev = &info->client->dev;
	dev_set_drvdata(info->dev, info);
	mutex_init(&tui_mutex);
	if (dp) {
		info->board =
			devm_kzalloc(&client->dev,
						 sizeof(struct fts_hw_platform_data),
						 GFP_KERNEL);
		if (!info->board) {
			logError(1, "%s ERROR:info.board kzalloc failed \n",
					 tag);
			error = -ENOMEM;
			goto ProbeErrorExit_1;
		}
		parse_dt(&client->dev, info->board);
	}

#if defined(CONFIG_DRM)
	error = fts_ts_check_panel_node(dp);
	if (!panel_node) {
		logError(1, "Failed to check panel\n");
	} else {
		info->notifier_cookie = xring_panel_event_notifier_register(XRING_PANEL_EVENT_TAG_PRIMARY,
			XRING_PANEL_EVENT_CLIENT_SECONDARY_TOUCH, panel_node,
		&fts_drm_panel_notifier_callback, (void *)info);
		if (!info->notifier_cookie) {
		logError(1, "Failed to register for panel events\n");
		}
	}
#endif

	logError(0, "%s SET GPIOS: \n", tag);
	info->gpio_has_request = false;
	retval = fts_set_gpio(info, true);
	if (retval < 0) {
		logError(1, "%s %s: ERROR Failed to set up GPIO's\n", tag,
				 __func__);
		error = retval;
		goto ProbeErrorExit_1;
	}

	error = fts_pinctrl_init(info);

	if (!error && info->ts_pinctrl) {
		error =
			pinctrl_select_state(info->ts_pinctrl,
                     info->pinctrl_state_active);

		if (error < 0) {
			dev_err(&client->dev,
			"%s: Failed to select %s pinstate %d\n",
			__func__, PINCTRL_STATE_ACTIVE, error);
		}
	} else {
		dev_err(&client->dev, "%s: Failed to init pinctrl\n", __func__);
	}

	logError(0, "%s SET Regulators: \n", tag);
	retval = fts_get_reg(info, true);
	if (retval < 0) {
		logError(1, "%s ERROR: %s: Failed to get regulators\n", tag,
				 __func__);
		error = retval;
		goto ProbeErrorExit_2;
	}

	retval = fts_enable_reg(info, true);
	if (retval < 0) {
		logError(1, "%s %s: ERROR Failed to enable regulators\n", tag,
				 __func__);
		error = retval;
		goto ProbeErrorExit_3;
	}

	mdelay(3);
	retval = fts_set_gpio(info, false);
	if (retval < 0) {
		logError(1, "%s %s: ERROR Failed to set up GPIO's\n", tag,
				 __func__);
		error = retval;
		goto ProbeErrorExit_3_1;
	}

	info->client->irq = gpio_to_irq(info->board->irq_gpio);
	logError(0, "%s gpio_num:%d, irq:%d\n", tag, info->board->irq_gpio, info->client->irq);

	logError(0, "%s SET Event Handler: \n", tag);

	info->event_wq =
		alloc_workqueue("fts-event-queue",
						WQ_UNBOUND | WQ_HIGHPRI | WQ_CPU_INTENSIVE, 1);
	if (!info->event_wq) {
		logError(1, "%s ERROR: Cannot create work thread\n", tag);
		error = -ENOMEM;
		goto ProbeErrorExit_4;
	}

	info->irq_wq =
		alloc_workqueue("fts-irq-queue",
						WQ_UNBOUND | WQ_HIGHPRI | WQ_CPU_INTENSIVE, 1);
	if (!info->irq_wq) {
		logError(1, "%s ERROR: Cannot create irq work thread\n", tag);
		error = -ENOMEM;
		goto ProbeErrorExit_4;
	}

	info->fps_wq =
		alloc_workqueue("fts-fps-queue",
						WQ_UNBOUND | WQ_HIGHPRI | WQ_CPU_INTENSIVE, 1);
	if (!info->fps_wq) {
		logError(1, "%s ERROR: Cannot create fps thread\n", tag);
		error = -ENOMEM;
		goto ProbeErrorExit_4;
	}

	mutex_init(&info->fod_mutex);
	INIT_WORK(&info->resume_work, fts_resume_work);
	INIT_WORK(&info->suspend_work, fts_suspend_work);
	init_completion(&info->tp_reset_completion);

	logError(0, "%s SET Input Device Property: \n", tag);
	info->dev = &info->client->dev;
	info->input_dev = input_allocate_device();
	if (!info->input_dev) {
		logError(1, "%s ERROR: No such input device defined! \n", tag);
		error = -ENODEV;
		goto ProbeErrorExit_5;
	}
	info->input_dev->dev.parent = &client->dev;
	info->input_dev->name = FTS_TS_DRV_NAME;
	snprintf(fts_ts_phys, sizeof(fts_ts_phys), "%s/input0",
			 info->input_dev->name);
	info->input_dev->phys = fts_ts_phys;
	info->input_dev->id.bustype = bus_type;
	info->input_dev->id.vendor = 0x0001;
	info->input_dev->id.product = 0x0002;
	info->input_dev->id.version = 0x0100;
	input_set_drvdata(info->input_dev, info);

	__set_bit(EV_SYN, info->input_dev->evbit);
	__set_bit(EV_KEY, info->input_dev->evbit);
	__set_bit(EV_ABS, info->input_dev->evbit);
	__set_bit(BTN_TOUCH, info->input_dev->keybit);
	__set_bit(BTN_TOOL_FINGER, info->input_dev->keybit);
	/*__set_bit(BTN_TOOL_PEN, info->input_dev->keybit);*/

	input_mt_init_slots(info->input_dev, TOUCH_ID_MAX, INPUT_MT_DIRECT);

	/*input_mt_init_slots(info->input_dev, TOUCH_ID_MAX); */

	input_set_abs_params(info->input_dev, ABS_MT_POSITION_X, X_AXIS_MIN,
						 info->board->x_max - 1, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_POSITION_Y, Y_AXIS_MIN,
						 info->board->y_max - 1, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_TOUCH_MAJOR, AREA_MIN,
						 info->board->x_max, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_TOUCH_MINOR, AREA_MIN,
						 info->board->y_max, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_WIDTH_MINOR, AREA_MIN,
						 AREA_MAX, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_WIDTH_MAJOR, AREA_MIN,
						 AREA_MAX, 0, 0);

#ifdef FTS_FOD_AREA_REPORT
	/*input_set_abs_params(info->input_dev, ABS_MT_PRESSURE, PRESSURE_MIN, PRESSURE_MAX, 0, 0);*/
	input_set_abs_params(info->input_dev, ABS_MT_ORIENTATION, -90, 90, 0, 0);
#endif
	input_set_abs_params(info->input_dev, ABS_MT_DISTANCE, DISTANCE_MIN,
						 DISTANCE_MAX, 0, 0);

#ifdef GESTURE_MODE
	input_set_capability(info->input_dev, EV_KEY, KEY_WAKEUP);

	input_set_capability(info->input_dev, EV_KEY, KEY_M);
	input_set_capability(info->input_dev, EV_KEY, KEY_O);
	input_set_capability(info->input_dev, EV_KEY, KEY_E);
	input_set_capability(info->input_dev, EV_KEY, KEY_W);
	input_set_capability(info->input_dev, EV_KEY, KEY_C);
	input_set_capability(info->input_dev, EV_KEY, KEY_L);
	input_set_capability(info->input_dev, EV_KEY, KEY_F);
	input_set_capability(info->input_dev, EV_KEY, KEY_V);
	input_set_capability(info->input_dev, EV_KEY, KEY_S);
	input_set_capability(info->input_dev, EV_KEY, KEY_Z);
	input_set_capability(info->input_dev, EV_KEY, KEY_WWW);

	input_set_capability(info->input_dev, EV_KEY, KEY_LEFT);
	input_set_capability(info->input_dev, EV_KEY, KEY_RIGHT);
	input_set_capability(info->input_dev, EV_KEY, KEY_UP);
	input_set_capability(info->input_dev, EV_KEY, KEY_DOWN);

	input_set_capability(info->input_dev, EV_KEY, KEY_F1);
	input_set_capability(info->input_dev, EV_KEY, KEY_F2);
	input_set_capability(info->input_dev, EV_KEY, KEY_F3);
	input_set_capability(info->input_dev, EV_KEY, KEY_F4);
	input_set_capability(info->input_dev, EV_KEY, KEY_F5);

	input_set_capability(info->input_dev, EV_KEY, KEY_LEFTBRACE);
	input_set_capability(info->input_dev, EV_KEY, KEY_RIGHTBRACE);
#endif

#ifdef PHONE_KEY
	/*KEY associated to the touch screen buttons */
	input_set_capability(info->input_dev, EV_KEY, KEY_HOMEPAGE);
	input_set_capability(info->input_dev, EV_KEY, KEY_BACK);
	input_set_capability(info->input_dev, EV_KEY, KEY_MENU);
#endif
	mutex_init(&(info->input_report_mutex));
#ifdef GESTURE_MODE
	mutex_init(&gestureMask_mutex);
#endif

	spin_lock_init(&fts_int);

	/* register the multi-touch input device */
	error = input_register_device(info->input_dev);
	if (error) {
		logError(1, "%s ERROR: No such input device\n", tag);
		error = -ENODEV;
		goto ProbeErrorExit_5_1;
	}

	skip_5_1 = 1;
	/* track slots */
	info->touch_id = 0;
	info->temp_touch_id = 0;
#ifdef STYLUS_MODE
	info->stylus_id = 0;
#endif

	/* init feature switches (by default all the features are disable, if one feature want to be enabled from the start, set the corresponding value to 1) */
	info->gesture_enabled = 0;
	info->glove_enabled = 0;
	info->charger_enabled = 0;
	info->cover_enabled = 0;
	info->grip_enabled = 0;
	info->grip_pixel_def = 30;
	info->grip_pixel = info->grip_pixel_def;

	info->resume_bit = 1;
	info->lockdown_is_ok = false;

	mutex_init(&info->charge_lock);
#ifdef CONFIG_FTS_BL_CB
	info->bl_notifier = fts_bl_noti_block;
#endif
	logError(0, "%s Init Core Lib: \n", tag);
	initCore(info);
	/* init hardware device */
	logError(0, "%s Device Initialization: \n", tag);
	error = fts_init(info);
	if (error < OK) {
		logError(1, "%s Cannot initialize the device ERROR %08X\n", tag,
				 error);
		error = -ENODEV;
		goto ProbeErrorExit_6;
	}

#ifdef CONFIG_I2C_BY_DMA
	/*dma buf init*/
	info->dma_buf = (struct fts_dma_buf *)kzalloc(sizeof(*info->dma_buf), GFP_KERNEL);
	if (!info->dma_buf) {
		logError(1, "%s %s:Error alloc mem failed!", tag, __func__);
		goto ProbeErrorExit_7;
	}
	mutex_init(&info->dma_buf->dmaBufLock);
	info->dma_buf->rdBuf = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!info->dma_buf->rdBuf) {
		logError(1, "%s %s:Error alloc mem failed!", tag, __func__);
		goto ProbeErrorExit_7;
	}
	info->dma_buf->wrBuf = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!info->dma_buf->wrBuf) {
		logError(1, "%s %s:Error alloc mem failed!", tag, __func__);
		goto ProbeErrorExit_7;
	}
#endif
	error = fts_get_lockdown_info(info->lockdown_info, info);

	if (error < OK)
		logError(1, "%s can't get lockdown info", tag);
	else {
		logError(0,
				 "%s Lockdown:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\n",
				 tag, info->lockdown_info[0], info->lockdown_info[1],
				 info->lockdown_info[2], info->lockdown_info[3],
				 info->lockdown_info[4], info->lockdown_info[5],
				 info->lockdown_info[6], info->lockdown_info[7]);
		info->lockdown_is_ok = true;
	}

#ifdef FTS_FW_UPDATE
#ifdef FW_UPDATE_ON_PROBE
	logError(0, "%s FW Update and Sensing Initialization: \n", tag);
	error = fts_fw_update(info, NULL, 0);
	if (error < OK) {
		logError(1,
				 "%s Cannot execute fw upgrade the device ERROR %08X\n",
				 tag, error);
		error = -ENODEV;
		goto ProbeErrorExit_7;
	}
#else
	logError(0, "%s SET Auto Fw Update: \n", tag);
	info->fwu_workqueue =
		alloc_workqueue("fts-fwu-queue",
						WQ_UNBOUND | WQ_HIGHPRI | WQ_CPU_INTENSIVE, 1);
	if (!info->fwu_workqueue) {
		logError(1, "%s ERROR: Cannot create fwu work thread\n", tag);
		goto ProbeErrorExit_7;
	}
	INIT_DELAYED_WORK(&info->fwu_work, fts_fw_update_auto);
#endif
#else
	error = fts_init_sensing(info);
	if (error < OK) {
		logError(1,
				 "%s Cannot initialize the hardware device ERROR %08X\n",
				 tag, error);
	}
#endif
	info->sensor_scan = true;

	logError(0, "%s SET Device File Nodes: \n", tag);
	/* sysfs stuff */
	info->attrs.attrs = fts_attr_group;
	error = sysfs_create_group(&client->dev.kobj, &info->attrs);
	if (error) {
		logError(1, "%s ERROR: Cannot create sysfs structure!\n", tag);
		error = -ENODEV;
		goto ProbeErrorExit_7;
	}

	error = fts_proc_init();
	if (error < OK)
		logError(1, "%s Error: can not create /proc file! \n", tag);

	device_init_wakeup(&client->dev, 1);
	init_completion(&info->pm_resume_completion);
#ifdef CONFIG_FTS_BL_CB
	if (backlight_register_notifier(&info->bl_notifier) < 0) {
		logError(1, "%s register bl_notifier failed!\n", tag);
	}
#endif

#ifndef FW_UPDATE_ON_PROBE
	queue_delayed_work(info->fwu_workqueue, &info->fwu_work,
					   msecs_to_jiffies(EXP_FN_WORK_DELAY_MS));
#endif
#if IS_ENABLED(CONFIG_XRING_MITEE_TUI_SUPPORT)
	register_tpd_tui_request(fts_tpd_enter_tui, fts_tpd_exit_tui,
				fts_get_vendor_id);
#endif
	logError(1, "%s Probe Finished! \n", tag);
	return OK;

ProbeErrorExit_7:
	if (info->tp_selftest_proc)
		remove_proc_entry("tp_selftest", NULL);
	info->tp_selftest_proc = NULL;
#ifdef CONFIG_I2C_BY_DMA
	if (info->dma_buf)
		kfree(info->dma_buf);
	if (info->dma_buf->rdBuf)
		kfree(info->dma_buf->rdBuf);
	if (info->dma_buf->wrBuf)
		kfree(info->dma_buf->wrBuf);
#endif
ProbeErrorExit_6:
#if defined(CONFIG_DRM)
	if (panel_node && info->notifier_cookie) {
		of_node_put(panel_node);
		panel_node = NULL;
		xring_panel_event_notifier_unregister(info->notifier_cookie);
	}
#endif
	input_unregister_device(info->input_dev);
#ifdef CONFIG_FTS_POWERSUPPLY_CB
	power_supply_unreg_notifier(&info->power_supply_notifier);
#endif

ProbeErrorExit_5_1:
	if (skip_5_1 != 1)
		input_free_device(info->input_dev);

ProbeErrorExit_5:
	destroy_workqueue(info->event_wq);
	destroy_workqueue(info->fps_wq);

ProbeErrorExit_4:
	fts_gpio_setup(info->board->irq_gpio, false, 0, 0);
	fts_gpio_setup(info->board->reset_gpio, false, 0, 0);

ProbeErrorExit_3_1:
	fts_enable_reg(info, false);

ProbeErrorExit_3:
	fts_get_reg(info, false);

ProbeErrorExit_2:
ProbeErrorExit_1:
	kfree(info);

ProbeErrorExit_0:
	logError(1, "%s Probe Failed!\n", tag);

	return error;
}

/**
 * Clear and free all the resources associated to the driver.
 * This function is called when the driver need to be removed.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
static int fts_remove(struct spi_device *client)
{
#else
static void fts_remove(struct spi_device *client)
{
#endif

	struct fts_ts_info *info = dev_get_drvdata(&(client->dev));

	fts_proc_remove();
	if (info->tp_lockdown_info_proc)
		remove_proc_entry("tp_lockdown_info", NULL);
	if (info->tp_selftest_proc)
		remove_proc_entry("tp_selftest", NULL);
	if (info->tp_data_dump_proc)
		remove_proc_entry("tp_data_dump", NULL);
	if (info->tp_fw_version_proc)
		remove_proc_entry("tp_fw_version", NULL);
	info->tp_lockdown_info_proc = NULL;
	info->tp_selftest_proc = NULL;
	info->tp_data_dump_proc = NULL;
	info->tp_fw_version_proc = NULL;

	/* sysfs stuff */
	sysfs_remove_group(&client->dev.kobj, &info->attrs);
	/* remove interrupt and event handlers */
	fts_interrupt_uninstall(info);
#ifdef CONFIG_FTS_BL_CB
	backlight_unregister_notifier(&info->bl_notifier);
#endif
#if defined(CONFIG_DRM)
	if (panel_node && info->notifier_cookie) {
		of_node_put(panel_node);
		panel_node = NULL;
		xring_panel_event_notifier_unregister(info->notifier_cookie);
	}
#endif
	/* unregister the device */
	input_unregister_device(info->input_dev);

	/* Remove the work thread */
	destroy_workqueue(info->event_wq);
	destroy_workqueue(info->fps_wq);
#ifndef FW_UPDATE_ON_PROBE
	destroy_workqueue(info->fwu_workqueue);
#endif
	device_destroy(info->fts_tp_class, DCHIP_ID_0);
	/*
		class_destroy(info->fts_tp_class);
		info->fts_tp_class = NULL;
	*/
	if (info->debugfs)
		debugfs_remove(info->debugfs);

	fts_enable_reg(info, false);
	fts_get_reg(info, false);
	fts_gpio_setup(info->board->irq_gpio, false, 0, 0);
	fts_gpio_setup(info->board->reset_gpio, false, 0, 0);
	if (info->board->platform_flag == FPGA_PLATFORM) {
		fts_gpio_setup(info->board->avdd_gpio, false, 0, 0);
		fts_gpio_setup(info->board->vdd_gpio, false, 0, 0);
	}
	fts_info = NULL;
#ifdef CONFIG_SECURE_TOUCH
	fts_secure_remove(info);
#endif
	/* free all */
	kfree(info);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
	return OK;
#endif
}

static struct of_device_id fts_of_match_table[] = {
	{
		.compatible = "st,spi",
	},
	{},
};

#ifdef I2C_INTERFACE
static const struct i2c_device_id fts_device_id[] = {
	{FTS_TS_DRV_NAME, 0},
	{}
};

static struct i2c_driver fts_i2c_driver = {
	.driver = {
		.name = FTS_TS_DRV_NAME,
		.of_match_table = fts_of_match_table,
	},
	.probe = fts_probe,
	.remove = fts_remove,
	.id_table = fts_device_id,
};
#else
static struct spi_driver fts_spi_driver = {
	.driver = {
		.name = FTS_TS_DRV_NAME,
		.of_match_table = fts_of_match_table,
		.owner = THIS_MODULE,
	},
	.probe = fts_probe,
	.remove = fts_remove,
};
#endif

static int __init fts_driver_init(void)
{
#ifdef I2C_INTERFACE
	return i2c_add_driver(&fts_i2c_driver);
#else
	return spi_register_driver(&fts_spi_driver);
#endif
}

static void __exit fts_driver_exit(void)
{
#ifdef I2C_INTERFACE
	i2c_del_driver(&fts_i2c_driver);
#else
	spi_unregister_driver(&fts_spi_driver);
#endif

}

MODULE_DESCRIPTION("STMicroelectronics MultiTouch IC Driver");
MODULE_AUTHOR("STMicroelectronics");
MODULE_LICENSE("GPL");

late_initcall(fts_driver_init);
module_exit(fts_driver_exit);
