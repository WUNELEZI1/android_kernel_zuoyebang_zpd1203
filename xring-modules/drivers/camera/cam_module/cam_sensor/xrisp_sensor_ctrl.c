// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#define pr_fmt(fmt) "[XRISP_DRV][%14s] %s(%d): " fmt, "sensor_ctrl", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "sensor_ctrl", __func__, __LINE__

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/pinctrl/consumer.h>
#include <soc/xring/xr_timestamp.h>

#include "xrisp_sensor_dev.h"
#include "xrisp_sensor_ctrl.h"
#include "xrisp_csiphy_ctrl.h"
#include "xrisp_rproc_api.h"
#include "xrisp_log.h"
#include "cam_priv_led/xrisp_cam_privacy_led.h"
#include "soc/xring/sensorhub/app_router/shub_app_router.h"

#define OIS_WAIT_MAX_TIME_MS (1000)

int xrisp_notify_shub_ois_power_down(void)
{
	struct cam_pm_dev *cam_pm;
	int ret;
	int i;

	for (i = 0; i < CAM_SENSOR_NUM_MAX; i++) {
		cam_pm = xrisp_sensor_get_context(i);
		if (!cam_pm)
			continue;

		reinit_completion(&cam_pm->power_info.ois.complete);
	}
	ret = app_router_notify_exit(1);
	if (ret > 0)
		XRISP_PR_INFO("notify shub success");
	else
		XRISP_PR_ERROR("notify shub fail");

	return ret;
}

void xrisp_sensor_ois_init(struct cam_pm_dev *cam_pm)
{
	if (!cam_pm)
		return;
	cam_pm->power_info.ois.wait = false;
	cam_pm->power_info.ois.sensor_id = cam_pm->id;
	init_completion(&cam_pm->power_info.ois.complete);
}

static void xrisp_sensor_ois_deinit(struct cam_pm_dev *cam_pm)
{
	if (!cam_pm)
		return;
	cam_pm->power_info.ois.wait = false;
	reinit_completion(&cam_pm->power_info.ois.complete);
}

static struct cam_power_ctrl_ois *xrisp_sensor_convert_ois(struct cam_power_ctrl_gpio *gpio_seq)
{
	struct cam_power_ctrl_info *power_info = NULL;

	if (IS_ERR_OR_NULL(gpio_seq))
		return NULL;

	power_info = container_of(gpio_seq, struct cam_power_ctrl_info, gpio_seq);
	if (IS_ERR_OR_NULL(power_info))
		return NULL;

	return &power_info->ois;
}

static int xrisp_sensor_ois_set_wait(struct cam_power_ctrl_ois *ois, bool need_wait)
{
	if (IS_ERR_OR_NULL(ois))
		return -EINVAL;

	ois->wait = need_wait;
	XRISP_PR_INFO("camid:%d set ois set wait=%d", ois->sensor_id, ois->wait);
	return 0;
}

static void xrisp_sensor_ois_wait(struct cam_power_ctrl_gpio *gpio_seq)
{
	struct cam_power_ctrl_ois *ois = xrisp_sensor_convert_ois(gpio_seq);
	unsigned long timeout;

	if (IS_ERR_OR_NULL(ois)) {
		XRISP_PR_ERROR("ois is null");
		return;
	}

	if (!ois->wait)
		return;

	XRISP_PR_DEBUG("camid:%d ois wait start, boottime %llu, timestamp %llu", ois->sensor_id,
			   ktime_get_boottime(), xr_timestamp_gettime());

	timeout =
		wait_for_completion_timeout(&ois->complete, msecs_to_jiffies(OIS_WAIT_MAX_TIME_MS));
	if (timeout)
		XRISP_PR_INFO("camid:%d ois finish, wait %d ms, boottime %llu, timestamp %llu",
				  ois->sensor_id, OIS_WAIT_MAX_TIME_MS - jiffies_to_msecs(timeout),
				  ktime_get_boottime(), xr_timestamp_gettime());
	else
		XRISP_PR_ERROR("camid:%d ois timeout, boottime %llu, timestamp %llu",
				   ois->sensor_id, ktime_get_boottime(), xr_timestamp_gettime());

	ois->wait = false;
	reinit_completion(&ois->complete);
}

int xrisp_sensor_ois_complete(uint32_t sensor_id)
{
	struct cam_pm_dev *cam_pm;

	cam_pm = xrisp_sensor_get_context(sensor_id);
	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	if (cam_pm->id != sensor_id) {
		XRISP_PR_ERROR("cam_pm id:%d mismatch sensor_id:%d!", cam_pm->id, sensor_id);
		return -EINVAL;
	}

	complete(&cam_pm->power_info.ois.complete);
	XRISP_PR_INFO("camid:%d ois complete", sensor_id);
	return 0;
}

static int xrisp_sensor_request_gpio_table(struct cam_gpio_info *gpio_info, uint32_t enable)
{
	int ret = 0;
	struct gpio *req_tbl;
	uint32_t req_size;

	if (XRISP_CHECK_NULL_RETURN_INT(gpio_info)) {
		XRISP_PR_ERROR("gpio_info is null");
		return -ENOMEM;
	}
	req_tbl = gpio_info->gpio_req_tbl;
	if (XRISP_CHECK_NULL_RETURN_INT(req_tbl)) {
		XRISP_PR_ERROR("req_tbl is null");
		return -ENOMEM;
	}

	req_size = gpio_info->gpio_req_tbl_size;
	if (!req_size) {
		XRISP_PR_ERROR("Invalid gpio request table size:%d.", req_size);
		return -EINVAL;
	}

	if (enable)
		ret = gpio_request_array(req_tbl, req_size);
	else
		gpio_free_array(req_tbl, req_size);

	return ret;
}

static int xrisp_sensor_fill_vreg_params_one(struct cam_hw_info *hw_info, const char *name,
	struct cam_power_setting *power_setting)
{
	int i;
	uint32_t num_vreg;

	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(name)) {
		XRISP_PR_ERROR("name is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(power_setting)) {
		XRISP_PR_ERROR("power_setting is null");
		return -ENOMEM;
	}

	num_vreg = hw_info->num_rgltr;
	if (num_vreg <= 0 || num_vreg > CAM_MAX_REGULATOR) {
		XRISP_PR_ERROR("Invalid regulator number:%d.", num_vreg);
		return -EINVAL;
	}

	for (i = 0; i < num_vreg; i++) {
		if (!strcmp(hw_info->rgltr_nd_name[i], name)) {
			power_setting->seq_val = i;
			break;
		}
	}
	if (i == num_vreg)
		power_setting->seq_val = INVALID_VREG;

	return 0;
}

static int xrisp_sensor_fill_vreg_params(struct cam_hw_info *hw_info,
	struct cam_power_setting *power_settings,
	uint32_t power_setting_size)
{
	int ret = 0;
	int i, j;
	struct cam_seq_name regulator_tab[] = {
		{"cam_vdig", SENSOR_DVDD},
		{"cam_vana", SENSOR_AVDD},
		{"cam_bob1", SENSOR_BOB1},
		{"cam_vaf",  SENSOR_AF},
		{"cam_vois", SENSOR_OIS},
		{"cam_vio",  SENSOR_CAMIO},
		{"cam_bob2", SENSOR_BOB2},
		{"cam_apert", SENSOR_APERT},
	};

	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(power_settings)) {
		XRISP_PR_ERROR("power_settings is null");
		return -ENOMEM;
	}

	for (i = 0; i < power_setting_size; i++) {
		if (XRISP_CHECK_NULL_RETURN_INT(&power_settings[i])) {
			XRISP_PR_ERROR("power_settings[%d] is null", i);
			return -ENOMEM;
		}
		if (power_settings[i].seq_type < SENSOR_DVDD ||
			power_settings[i].seq_type >= SENSOR_SEQ_TYPE_MAX) {
			XRISP_PR_ERROR("Invalid power seq type:%d.", power_settings[i].seq_type);
			return -EINVAL;
		}

		for (j = 0; j < ARRAY_SIZE(regulator_tab); j++) {
			if (power_settings[i].seq_type == regulator_tab[j].seq_type) {
				ret = xrisp_sensor_fill_vreg_params_one(hw_info,
					regulator_tab[j].name,
					&power_settings[i]);
				if (ret) {
					XRISP_PR_ERROR("fill:%s regulator seq val failed.",
						regulator_tab[j].name);
					return ret;
				}
				break;
			}
		}
	}

	return ret;
}

static int xrisp_sensor_gpio_set(struct cam_power_ctrl_gpio *gpio_seq,
	enum xr_camera_power_seq_type seq_type, uint32_t enable)
{
	int ret = 0;

	if (XRISP_CHECK_NULL_RETURN_INT(gpio_seq)) {
		XRISP_PR_ERROR("gpio_seq is null");
		return -ENOMEM;
	}
	if (seq_type < SENSOR_DVDD || seq_type >= SENSOR_SEQ_TYPE_MAX) {
		XRISP_PR_ERROR("Invalid seq type:%d.", seq_type);
		return -EINVAL;
	}

	if (gpio_seq->valid[seq_type] == 1)
		gpio_set_value_cansleep(gpio_seq->gpio_num[seq_type], enable);

	return ret;
}

int xrisp_sensor_core_power_down(struct cam_hw_info *hw_info,
	struct cam_power_ctrl_gpio *gpio_seq,
	uint32_t power_setting_count,
	struct cam_power_setting *power_settings,
	uint32_t force)
{
	int ret;
	struct cam_pinctrl_info  *pinctrl_info;
	struct cam_power_setting *power_setting;
	uint32_t m;
	int idx;
	int vreg_idx = -1;
	int power_down_result = 0;

	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(gpio_seq)) {
		XRISP_PR_ERROR("gpio_seq is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(power_settings)) {
		XRISP_PR_ERROR("power_settings is null");
		return -ENOMEM;
	}

	if (!power_setting_count) {
		XRISP_PR_ERROR("Invalid power setting count:%d.", power_setting_count);
		return -EINVAL;
	}

	//Cci disconfig, then powerdown begin
	pinctrl_info = &hw_info->pinctrl_info;
	ret = pinctrl_select_state(pinctrl_info->pinctrl,
		pinctrl_info->gpio_state_cci_suspend);
	if (ret) {
		power_down_result = -EINVAL;
		XRISP_PR_ERROR("cci set pin to suspend failed.");
	}

	for (idx = 0; idx < power_setting_count; idx++) {
		if (force)
			m = power_setting_count - 1 - idx;
		else
			m = idx;
		power_setting = &power_settings[m];

		XRISP_PR_DEBUG("powerdown idx:%d seq_type:%d seq_val:0x%x.",
			m, power_setting->seq_type, power_setting->seq_val);
		switch (power_setting->seq_type) {
		case SENSOR_MCLK:
			/* soc supply clock */
			if (!gpio_seq->valid[SENSOR_MCLK]) {
				ret = xrisp_sensor_util_clk_set(
					hw_info, "cam_mclk", 0, CAM_NORMAL_VOTE);
				if (ret) {
					XRISP_PR_ERROR("cam mclk disable failed.");
					break;
				}
			/* oscillator supply clock */
			} else {
				ret = xrisp_sensor_util_regulator_set_by_name(
					hw_info, "cam_vclk", 0);
				if (ret)
					XRISP_PR_ERROR("cam mclk disable regulator failed.");

				ret = xrisp_sensor_gpio_set(gpio_seq, SENSOR_MCLK, 0);
				if (ret)
					XRISP_PR_ERROR("cam mclk set gpio failed.");
			}
			break;
		case SENSOR_DVDD:
		case SENSOR_AVDD:
		case SENSOR_BOB1:
		case SENSOR_AF:
		case SENSOR_OIS:
		case SENSOR_CAMIO:
		case SENSOR_BOB2:
		case SENSOR_APERT:
			if (power_setting->seq_type == SENSOR_OIS)
				xrisp_sensor_ois_wait(gpio_seq);

			if (!gpio_seq->valid[power_setting->seq_type]) {
				vreg_idx = power_setting->seq_val;
				if (vreg_idx == INVALID_VREG)
					break;
				if ((vreg_idx < 0) || (vreg_idx >= hw_info->num_rgltr)) {
					XRISP_PR_ERROR("invalid seq:%d val:%d",
						power_setting->seq_type,
						power_setting->seq_val);
					break;
				}

				ret = xrisp_sensor_util_regulator_set(hw_info->rgltr[vreg_idx],
					hw_info->rgltr_name[vreg_idx],
					hw_info->rgltr_min_volt[vreg_idx],
					hw_info->rgltr_max_volt[vreg_idx],
					hw_info->rgltr_op_mode[vreg_idx],
					hw_info->rgltr_delay[vreg_idx],
					0);
				if (ret) {
					XRISP_PR_ERROR("regulator:%d disable failed.",
						power_setting->seq_type);
					break;
				}
			} else {
				ret = xrisp_sensor_gpio_set(gpio_seq,
					power_setting->seq_type, 0);
				if (ret) {
					XRISP_PR_ERROR("regulator:%d disable failed by gpio",
						power_setting->seq_type);
					break;
				}
			}
			break;
		case SENSOR_RESET:
		case SENSOR_STANDBY:
		case SENSOR_CUSTOM_GPIO1:
		case SENSOR_CUSTOM_GPIO2:
			ret = xrisp_sensor_gpio_set(gpio_seq,
				power_setting->seq_type, 0);
			if (ret) {
				XRISP_PR_ERROR("gpio:%d set failed.",
					power_setting->seq_type);
				break;
			}
			break;
		case SENSOR_CLK_VDD:
			break;
		default:
			XRISP_PR_ERROR("invalid power seq type:%d.",
				power_setting->seq_type);
			break;
		}

		xtisp_sensor_util_msleep(power_setting->delay);
	}
	if (idx != power_setting_count)
		power_down_result = -EINVAL;

	ret = xrisp_sensor_request_gpio_table(&hw_info->gpio_info, 0);
	if (ret) {
		power_down_result = -EINVAL;
		XRISP_PR_ERROR("free gpio failed.");
	}

	ret = pinctrl_select_state(pinctrl_info->pinctrl,
		pinctrl_info->gpio_state_suspend);
	if (ret) {
		power_down_result = -EINVAL;
		XRISP_PR_ERROR("set pin to suspend failed.");
	}

	XRISP_PR_DEBUG("sensor power down.\n");
	return power_down_result;
}

static int xrisp_sensor_core_power_up(struct cam_hw_info *hw_info,
	struct cam_power_ctrl_gpio *gpio_seq,
	uint32_t power_setting_count,
	struct cam_power_setting *power_settings)
{
	int ret;
	struct cam_pinctrl_info  *pinctrl_info;
	struct cam_power_setting *power_setting;
	int idx;
	int vreg_idx = -1;

	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(gpio_seq)) {
		XRISP_PR_ERROR("gpio_seq is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(power_settings)) {
		XRISP_PR_ERROR("power_settings is null");
		return -ENOMEM;
	}

	if (!power_setting_count) {
		XRISP_PR_ERROR("Invalid power setting count:%d.", power_setting_count);
		return -EINVAL;
	}

	pinctrl_info = &hw_info->pinctrl_info;
	if (XRISP_CHECK_NULL_RETURN_INT(pinctrl_info->pinctrl)) {
		XRISP_PR_ERROR("pinctrl_info->pinctrl is null");
		return -ENOMEM;
	}
	ret = pinctrl_select_state(pinctrl_info->pinctrl,
		pinctrl_info->gpio_state_active);
	if (ret) {
		XRISP_PR_ERROR("set pin to activeve failed.");
		return -EINVAL;
	}

	ret = xrisp_sensor_request_gpio_table(&hw_info->gpio_info, 1);
	if (ret) {
		XRISP_PR_ERROR("request gpio failed.");
		goto pinctrl_suspend;
	}

	for (idx = 0; idx < power_setting_count; idx++) {
		power_setting = &power_settings[idx];
		if (!power_setting) {
			XRISP_PR_ERROR("Invalid power setting.");
			goto sensor_powerdown;
		}

		XRISP_PR_DEBUG("powerup idx:%d seq_type:%d seq_val:0x%x.",
			idx, power_setting->seq_type, power_setting->seq_val);
		switch (power_setting->seq_type) {
		case SENSOR_MCLK:
			/* soc supply clock */
			if (!gpio_seq->valid[SENSOR_MCLK]) {
				ret = xrisp_sensor_util_clk_set(hw_info,
					"cam_mclk", 1, CAM_NORMAL_VOTE);
				if (ret) {
					XRISP_PR_ERROR("cam mclk enable failed.");
					goto sensor_powerdown;
				}
			/* oscillator supply clock */
			} else {
				ret = xrisp_sensor_util_regulator_set_by_name(hw_info,
					"cam_vclk", 1);
				if (ret) {
					XRISP_PR_ERROR("cam mclk enable regulator failed.");
					goto sensor_powerdown;
				}
				ret = xrisp_sensor_gpio_set(gpio_seq, SENSOR_MCLK, 1);
				if (ret) {
					XRISP_PR_ERROR("cam mclk set gpio failed.");
					xrisp_sensor_util_regulator_set_by_name(hw_info,
						"cam_vclk", 0);
					goto sensor_powerdown;
				}
			}
			break;
		case SENSOR_DVDD:
		case SENSOR_AVDD:
		case SENSOR_BOB1:
		case SENSOR_AF:
		case SENSOR_OIS:
		case SENSOR_CAMIO:
		case SENSOR_BOB2:
		case SENSOR_APERT:
			if (!gpio_seq->valid[power_setting->seq_type]) {
				vreg_idx = power_setting->seq_val;
				if (vreg_idx == INVALID_VREG)
					break;
				if ((vreg_idx < 0) || (vreg_idx >= hw_info->num_rgltr)) {
					XRISP_PR_ERROR("Invalid seq:%d val:%d",
						power_setting->seq_type,
						power_setting->seq_val);
					goto sensor_powerdown;
				}

				ret = xrisp_sensor_util_regulator_set(hw_info->rgltr[vreg_idx],
					hw_info->rgltr_name[vreg_idx],
					hw_info->rgltr_min_volt[vreg_idx],
					hw_info->rgltr_max_volt[vreg_idx],
					hw_info->rgltr_op_mode[vreg_idx],
					hw_info->rgltr_delay[vreg_idx],
					1);
				if (ret) {
					XRISP_PR_ERROR("regulator:%d enable failed.",
						power_setting->seq_type);
					goto sensor_powerdown;
				}
			} else {
				ret = xrisp_sensor_gpio_set(gpio_seq,
					power_setting->seq_type, 1);
				if (ret) {
					XRISP_PR_ERROR("regulator:%d enable failed by gpio.",
						power_setting->seq_type);
					goto sensor_powerdown;
				}
			}
			break;
		case SENSOR_RESET:
		case SENSOR_STANDBY:
		case SENSOR_CUSTOM_GPIO1:
		case SENSOR_CUSTOM_GPIO2:
			ret = xrisp_sensor_gpio_set(gpio_seq,
				power_setting->seq_type, 1);
			if (ret) {
				XRISP_PR_ERROR("gpio:%d set failed.",
					power_setting->seq_type);
				goto sensor_powerdown;
			}
			break;
		case SENSOR_CLK_VDD:
			break;
		default:
			XRISP_PR_ERROR("Invalid power seq type:%d.", power_setting->seq_type);
			goto sensor_powerdown;
		}

		xtisp_sensor_util_msleep(power_setting->delay);
	}

	//powerup finish, then cci config
	ret = pinctrl_select_state(pinctrl_info->pinctrl,
		pinctrl_info->gpio_state_cci_active);
	if (ret) {
		XRISP_PR_ERROR("cci set pin to activeve failed.");
		goto cci_reconfig;
	}
	XRISP_PR_DEBUG("sensor core power up success.\n");
	return 0;

cci_reconfig:
	pinctrl_select_state(pinctrl_info->pinctrl, pinctrl_info->gpio_state_cci_suspend);

sensor_powerdown:
	for (idx--; idx >= 0; idx--) {
		power_setting = &power_settings[idx];

		XRISP_PR_INFO("power_setting free resource idx:%d seq_type:%d seq_val:0x%x.",
			idx, power_setting->seq_type, power_setting->seq_val);
		switch (power_setting->seq_type) {
		case SENSOR_MCLK:
			/* soc supply clock */
			if (!gpio_seq->valid[SENSOR_MCLK]) {
				ret = xrisp_sensor_util_clk_set(
					hw_info, "cam_mclk", 0, CAM_NORMAL_VOTE);
				if (ret) {
					XRISP_PR_ERROR("cam mclk disable failed.");
					break;
				}
			/* oscillator supply clock */
			} else {
				ret = xrisp_sensor_util_regulator_set_by_name(
					hw_info, "cam_vclk", 0);
				if (ret)
					XRISP_PR_ERROR("cam mclk disable regulator failed.");

				ret = xrisp_sensor_gpio_set(gpio_seq, SENSOR_MCLK, 0);
				if (ret)
					XRISP_PR_ERROR("cam mclk set gpio failed.");
			}
			break;
		case SENSOR_DVDD:
		case SENSOR_AVDD:
		case SENSOR_BOB1:
		case SENSOR_AF:
		case SENSOR_OIS:
		case SENSOR_CAMIO:
		case SENSOR_BOB2:
		case SENSOR_APERT:
			if (!gpio_seq->valid[power_setting->seq_type]) {
				vreg_idx = power_setting->seq_val;
				if (vreg_idx == INVALID_VREG)
					break;
				if ((vreg_idx < 0) || (vreg_idx >= hw_info->num_rgltr)) {
					XRISP_PR_ERROR("free resource invalid seq:%d val:%d",
						power_setting->seq_type,
						power_setting->seq_val);
					break;
				}

				ret = xrisp_sensor_util_regulator_set(hw_info->rgltr[vreg_idx],
					hw_info->rgltr_name[vreg_idx],
					hw_info->rgltr_min_volt[vreg_idx],
					hw_info->rgltr_max_volt[vreg_idx],
					hw_info->rgltr_op_mode[vreg_idx],
					hw_info->rgltr_delay[vreg_idx],
					0);
				if (ret) {
					XRISP_PR_ERROR("free resource regulator:%d disable failed.",
						power_setting->seq_type);
					break;
				}
			} else {
				ret = xrisp_sensor_gpio_set(gpio_seq,
					power_setting->seq_type, 0);
				if (ret) {
					XRISP_PR_ERROR("free resource regulator:%d disable failed by gpio",
						power_setting->seq_type);
					break;
				}
			}
			break;
		case SENSOR_RESET:
		case SENSOR_STANDBY:
		case SENSOR_CUSTOM_GPIO1:
		case SENSOR_CUSTOM_GPIO2:
			ret = xrisp_sensor_gpio_set(gpio_seq,
				power_setting->seq_type, 0);
			if (ret) {
				XRISP_PR_ERROR("free resource gpio:%d set failed.",
					power_setting->seq_type);
				break;
			}
			break;
		case SENSOR_CLK_VDD:
			break;
		default:
			XRISP_PR_ERROR("free resource invalid power seq type:%d.",
				power_setting->seq_type);
			break;
		}

		xtisp_sensor_util_msleep(power_setting->delay);
	}
	xrisp_sensor_request_gpio_table(&hw_info->gpio_info, 0);

pinctrl_suspend:
	if (pinctrl_select_state(pinctrl_info->pinctrl,
			pinctrl_info->gpio_state_suspend))
		XRISP_PR_ERROR("pinctrl suspend failed.");

	return -EINVAL;
}

static int xrisp_sensor_power_down_impl(struct xrisp_sensor_power_ctrl *ctrl,
	enum cam_power_type type)
{
	int ret;
	struct cam_pm_dev *cam_pm;
	struct cam_hw_info *hw_info;
	struct cam_power_ctrl_gpio   *gpio_seq;
	struct cam_power_ctrl_record *power_record;
	struct cam_power_setting *power_settings;
	struct cam_parklens_ctrl_t *parklens_s;
	int idx;

	if (XRISP_CHECK_NULL_RETURN_INT(ctrl)) {
		XRISP_PR_ERROR("ctrl is null");
		return -ENOMEM;
	}
	if (ctrl->sensor_id > CAM_SENSOR_NUM_MAX) {
		XRISP_PR_ERROR("sensor_id:%d out of range failed!", ctrl->sensor_id);
		return -EINVAL;
	}

	if (!ctrl->count) {
		XRISP_PR_ERROR("Invalid power setting!");
		return -EINVAL;
	}

	if ((type < POWER_TYPE_SENSOR) || (type >= POWER_TYPE_MAX)) {
		XRISP_PR_ERROR("Invalid power type:%d!", type);
		return -EINVAL;
	}

	cam_pm = xrisp_sensor_get_context(ctrl->sensor_id);
	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	if (cam_pm->id != ctrl->sensor_id) {
		XRISP_PR_ERROR("cam pm id:%d mismatch sensor_id:%d!", cam_pm->id, ctrl->sensor_id);
		return -EINVAL;
	}

	hw_info = cam_pm->hw_info;
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}

	gpio_seq = &cam_pm->power_info.gpio_seq;
	power_record = &cam_pm->power_info.power_record[type];

	mutex_lock(&cam_pm->mlock);
	if (atomic_read(&power_record->power_cnt) == 0) {
		XRISP_PR_INFO("type:%d is already powerdown.", type);
		mutex_unlock(&cam_pm->mlock);
		return 0;
	}

	if (atomic_dec_return(&power_record->power_cnt)) {
		if (atomic_read(&power_record->power_cnt) < 0)
			atomic_set(&power_record->power_cnt, 0);

		XRISP_PR_INFO("power count:%d.", atomic_read(&power_record->power_cnt));
		mutex_unlock(&cam_pm->mlock);
		return 0;
	}

	power_settings = kcalloc(ctrl->count, sizeof(struct cam_power_setting), GFP_KERNEL);
	if (!power_settings) {
		ret = -ENOMEM;
		goto pm_unlock;
	}

	for (idx = 0; idx < ctrl->count; idx++) {
		power_settings[idx].seq_type =
			ctrl->power_settings[idx].power_seq_type;
		power_settings[idx].delay =
			ctrl->power_settings[idx].delay_milli_seconds;
		power_settings[idx].seq_val = 0;
	}

	ret = xrisp_sensor_fill_vreg_params(hw_info,
		power_settings, ctrl->count);
	if (ret) {
		XRISP_PR_ERROR("fill regulator info failed.");
		goto power_settings_free;
	}

	if (!ctrl->ois_delay)
		xrisp_sensor_ois_deinit(cam_pm);

	parklens_s = &cam_pm->power_info.parklens;
	ret = xrisp_parklens_sensor_power_down(parklens_s, ctrl, cam_pm, power_settings);
	if (ret == PARKLENS_RETURN)
		goto pm_unlock;

	ret = xrisp_sensor_core_power_down(hw_info, gpio_seq,
		ctrl->count, power_settings, 0);
	if (ret) {
		XRISP_PR_ERROR("image sensor power down failed.");
		goto power_settings_free;
	}

	power_record->count = 0;
	kfree(power_record->power_settings);
	power_record->power_settings = NULL;
	kfree(power_settings);
	mutex_unlock(&cam_pm->mlock);

	XRISP_PR_DEBUG("camid:%d sensor power down success.", cam_pm->id);
	return ret;

power_settings_free:
	kfree(power_settings);

pm_unlock:
	power_record->count = 0;
	kfree(power_record->power_settings);
	power_record->power_settings = NULL;
	mutex_unlock(&cam_pm->mlock);

	return ret;
}

static int xrisp_sensor_power_up_impl(struct xrisp_sensor_power_ctrl *ctrl,
	enum cam_power_type type)
{
	int ret = 0;
	struct cam_pm_dev *cam_pm;
	struct cam_hw_info *hw_info;
	struct cam_power_ctrl_gpio   *gpio_seq;
	struct cam_power_ctrl_record *power_record;
	struct cam_power_setting *power_setting;
	struct cam_parklens_ctrl_t *parklens_s;
	int idx;

	if (XRISP_CHECK_NULL_RETURN_INT(ctrl)) {
		XRISP_PR_ERROR("ctrl is null");
		return -ENOMEM;
	}
	if (ctrl->sensor_id > CAM_SENSOR_NUM_MAX) {
		XRISP_PR_ERROR("sensor_id:%d out of range failed!", ctrl->sensor_id);
		return -EINVAL;
	}

	if (!ctrl->count) {
		XRISP_PR_ERROR("Invalid power setting!");
		return -EINVAL;
	}

	if ((type < POWER_TYPE_SENSOR) || (type >= POWER_TYPE_MAX)) {
		XRISP_PR_ERROR("Invalid power type:%d!", type);
		return -EINVAL;
	}

	cam_pm = xrisp_sensor_get_context(ctrl->sensor_id);
	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	if (cam_pm->id != ctrl->sensor_id) {
		XRISP_PR_ERROR("cam pm id:%d mismatch sensor_id:%d!", cam_pm->id, ctrl->sensor_id);
		return -EINVAL;
	}

	hw_info = cam_pm->hw_info;
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}

	gpio_seq = &cam_pm->power_info.gpio_seq;
	power_record = &cam_pm->power_info.power_record[type];
	mutex_lock(&cam_pm->mlock);
	if (atomic_read(&power_record->power_cnt)) {
		atomic_inc(&power_record->power_cnt);
		XRISP_PR_INFO("camid:%d already powerup count:%d.",
			cam_pm->id, atomic_read(&power_record->power_cnt));
		mutex_unlock(&cam_pm->mlock);
		return 0;
	}

	xrisp_sensor_ois_deinit(cam_pm);
	if (ctrl->ois_delay)
		xrisp_sensor_ois_set_wait(&cam_pm->power_info.ois, true);

	power_record->count = ctrl->count;
	power_record->power_settings = kcalloc(power_record->count,
		sizeof(struct cam_power_setting), GFP_KERNEL);
	if (!power_record->power_settings) {
		ret = -ENOMEM;
		goto pm_unlock;
	}

	power_setting = power_record->power_settings;
	for (idx = 0; idx < ctrl->count; idx++) {
		power_setting[idx].seq_type =
			ctrl->power_settings[idx].power_seq_type;
		power_setting[idx].delay =
			ctrl->power_settings[idx].delay_milli_seconds;
		power_setting[idx].seq_val = 0;
	}

	ret = xrisp_sensor_fill_vreg_params(hw_info,
		power_record->power_settings, power_record->count);
	if (ret) {
		XRISP_PR_ERROR("fill regulator info failed.");
		goto power_settings_free;
	}

	parklens_s = &cam_pm->power_info.parklens;
	if (xrisp_parklens_sensor_power_up(parklens_s) == PARKLENS_RETURN) {
		XRISP_PR_INFO("camid:%d parklens not power down, skip power up!", cam_pm->id);
		atomic_inc(&power_record->power_cnt);
		goto pm_unlock;
	}

	ret = xrisp_sensor_core_power_up(hw_info, gpio_seq,
		power_record->count, power_record->power_settings);
	if (ret) {
		XRISP_PR_ERROR("image sensor power up failed.");
		goto power_settings_free;
	}

	atomic_inc(&power_record->power_cnt);
	xrisp_parklens_state_reset(&cam_pm->power_info.parklens);
	mutex_unlock(&cam_pm->mlock);

	XRISP_PR_DEBUG("camid:%d sensor power up success, boottime %llu, timestamp %llu.",
			   cam_pm->id, ktime_get_boottime(), xr_timestamp_gettime());
	return ret;

power_settings_free:
	kfree(power_record->power_settings);
	power_record->power_settings = NULL;

pm_unlock:
	if (ret != PARKLENS_RETURN)
		xrisp_sensor_ois_deinit(cam_pm);
	mutex_unlock(&cam_pm->mlock);

	return ret;
}

int xrisp_sensor_power_up(struct xrisp_sensor_power_ctrl *ctrl)
{
	int ret, count = 0;
	struct cam_pm_dev *cam_pm = NULL;
	struct cam_hw_info *hw_info = NULL;
	enum cam_power_type type;
	bool is_aoc_cam = false;

	if (XRISP_CHECK_NULL_RETURN_INT(ctrl)) {
		XRISP_PR_ERROR("ctrl is null");
		return -ENOMEM;
	}
	type = ctrl->power_type;
	XRISP_PR_DEBUG("camid:%d power up start type:%d.", ctrl->sensor_id, type);
	if ((type < POWER_TYPE_SENSOR) || (type >= POWER_TYPE_MAX)) {
		XRISP_PR_ERROR("Invalid power type:%d!", type);
		return -EINVAL;
	}

	if (type == POWER_TYPE_SENSOR) {
		cam_pm = xrisp_sensor_get_context(ctrl->sensor_id);
		if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
			XRISP_PR_ERROR("cam_pm is null");
			return -ENOMEM;
		}
		if (cam_pm->id != ctrl->sensor_id) {
			XRISP_PR_ERROR("cam pm id:%d mismatch sensor_id:%d!",
				cam_pm->id, ctrl->sensor_id);
			return -EINVAL;
		}

		hw_info = cam_pm->hw_info;
		if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
			XRISP_PR_ERROR("hw_info is null");
			return -ENOMEM;
		}

		is_aoc_cam = hw_info->is_aoc_cam;
		if (xrisp_sensor_get_has_aoc_status()) {
			xrisp_power_sync_set(XRISP_AP_CAM_POWER_STATUS_MASK,
				XRISP_AOC_AP_POWER_MASKBIT);
			if (is_aoc_cam) {
				XRISP_PR_INFO("sensorhub aoc is poweron now!");
				while (xrisp_power_sync_get(XRISP_AOC_SH_POWER_MASKBIT)
					&& count < 60) {
					usleep_range(500, 500 + 100);
					count++;
				};
				xrisp_power_sync_set(XRISP_AP_CAM_FRONT_STATUS_MASK,
					XRISP_AOC_AP_FRONT_MASKBIT);
			}
		}
		ret = xrisp_csiphy_enable();
		if (ret) {
			if (xrisp_sensor_get_has_aoc_status()) {
				if (is_aoc_cam)
					xrisp_power_sync_clear(XRISP_AOC_AP_FRONT_MASKBIT);
				xrisp_power_sync_clear(XRISP_AOC_AP_POWER_MASKBIT);
			}
			XRISP_PR_ERROR("enable csiphy failed.");
			return ret;
		}
	}

	ret = xrisp_sensor_power_up_impl(ctrl, type);
	if (ret) {
		if (xrisp_sensor_get_has_aoc_status()) {
			if (is_aoc_cam)
				xrisp_power_sync_clear(XRISP_AOC_AP_FRONT_MASKBIT);
			xrisp_power_sync_clear(XRISP_AOC_AP_POWER_MASKBIT);
		}
		XRISP_PR_ERROR("image sensor power up failed.");
		goto csiphy_powerdown;
	}

	if (ctrl->need_parklens)
		xrisp_rproc_add_powerdown_wait_cond(ctrl->sensor_id);

	XRISP_PR_INFO("camid:%d type:%d aon flag:%d is_probe_event:%d power up success.",
		ctrl->sensor_id, type, ctrl->aon_boot_scene, ctrl->is_probe_event);

	if (type == POWER_TYPE_SENSOR && (!ctrl->aon_boot_scene && !ctrl->is_probe_event))
		xrisp_privacy_reminder_led_enable();

	return 0;

csiphy_powerdown:
	if (type == POWER_TYPE_SENSOR)
		xrisp_csiphy_disable();

	return ret;
}

int xrisp_sensor_power_down(struct xrisp_sensor_power_ctrl *ctrl)
{
	int ret, count = 0;
	enum cam_power_type type;
	struct cam_pm_dev *cam_pm;
	struct cam_hw_info *hw_info;

	if (XRISP_CHECK_NULL_RETURN_INT(ctrl)) {
		XRISP_PR_ERROR("ctrl is null");
		return -ENOMEM;
	}
	type = ctrl->power_type;
	XRISP_PR_DEBUG("camid:%d power down start tyep:%d.", ctrl->sensor_id, type);
	if ((type < POWER_TYPE_SENSOR) || (type >= POWER_TYPE_MAX)) {
		XRISP_PR_ERROR("Invalid power type:%d!", type);
		return -EINVAL;
	}

	cam_pm = xrisp_sensor_get_context(ctrl->sensor_id);
	if (cam_pm == NULL) {
		XRISP_PR_ERROR("sensor power down get cam_pm failed.");
		return -EINVAL;
	}
	if (cam_pm->id != ctrl->sensor_id) {
		XRISP_PR_ERROR("cam pm id:%d mismatch sensor_id:%d!",
			cam_pm->id, ctrl->sensor_id);
		return -EINVAL;
	}
	hw_info = cam_pm->hw_info;
	if (hw_info == NULL) {
		XRISP_PR_ERROR("sensor power down get hw_info failed.");
		return -EINVAL;
	}
	if (xrisp_sensor_get_has_aoc_status()) {
		if (hw_info->is_aoc_cam) {
			while (xrisp_power_sync_get(XRISP_AOC_SH_POWER_MASKBIT) && count < 3) {
				usleep_range(500, 500 + 100);
				count++;
			};
		}
	}

	ret = xrisp_sensor_power_down_impl(ctrl, type);
	if (ret != 0 && ret != PARKLENS_RETURN)
		XRISP_PR_ERROR("image sensor power down failed.");

	if (type == POWER_TYPE_SENSOR && ret != PARKLENS_RETURN) {
		ret = xrisp_csiphy_disable();
		if (ret)
			XRISP_PR_ERROR("disable csiphy failed.");
	}
	if (ret == PARKLENS_RETURN)
		ret = 0;
	XRISP_PR_INFO("camid:%d type:%d aon flag:%d is_probe_event:%d power down success.",
		ctrl->sensor_id, type, ctrl->aon_boot_scene, ctrl->is_probe_event);

	if (type == POWER_TYPE_SENSOR && (!ctrl->aon_boot_scene && !ctrl->is_probe_event))
		xrisp_privacy_reminder_led_disable();
	if (xrisp_sensor_get_has_aoc_status()) {
		if (hw_info->is_aoc_cam)
			xrisp_power_sync_clear(XRISP_AOC_AP_FRONT_MASKBIT);
		xrisp_power_sync_clear(XRISP_AOC_AP_POWER_MASKBIT);
	}

	return ret;
}

int xrisp_sensor_power_down_force(uint32_t sensor_id, enum cam_power_type type)
{
	int ret;
	struct cam_pm_dev *cam_pm;
	struct cam_hw_info *hw_info;
	struct cam_power_ctrl_gpio   *gpio_seq;
	struct cam_power_ctrl_record *power_record;

	if (sensor_id > CAM_SENSOR_NUM_MAX) {
		XRISP_PR_ERROR("sensor_id:%d out of range failed!", sensor_id);
		return -EINVAL;
	}

	if ((type < POWER_TYPE_SENSOR) || (type >= POWER_TYPE_MAX)) {
		XRISP_PR_ERROR("Invalid power type:%d!", type);
		return -EINVAL;
	}

	cam_pm = xrisp_sensor_get_context(sensor_id);
	if (cam_pm == NULL)
		return 0;

	if (cam_pm->id != sensor_id) {
		XRISP_PR_ERROR("cam id:%d mismatch sensor_id:%d!", cam_pm->id, sensor_id);
		return -EINVAL;
	}

	hw_info = cam_pm->hw_info;
	if (hw_info == NULL) {
		XRISP_PR_ERROR("sensor force power down get hw_info failed.");
		return -EINVAL;
	}
	gpio_seq = &cam_pm->power_info.gpio_seq;
	power_record = &cam_pm->power_info.power_record[type];

	if (atomic_read(&power_record->power_cnt) == 0) {
		ret = 0;
		goto power_settings_free;
	} else {
		XRISP_PR_DEBUG("cam id:%d type:%d power count:%d.",
			cam_pm->id, type, atomic_read(&power_record->power_cnt));
		atomic_set(&power_record->power_cnt, 0);
	}

	mutex_lock(&cam_pm->mlock);
	ret = xrisp_sensor_core_power_down(hw_info, gpio_seq,
		power_record->count, power_record->power_settings, 1);
	if (ret)
		XRISP_PR_ERROR("sensor force power down failed.");

	mutex_unlock(&cam_pm->mlock);

	if (type == POWER_TYPE_SENSOR) {
		xrisp_csiphy_disable_force();
		if (xrisp_sensor_get_has_aoc_status()) {
			xrisp_power_sync_clear_force(XRISP_AOC_AP_FRONT_MASKBIT);
			xrisp_power_sync_clear_force(XRISP_AOC_AP_POWER_MASKBIT);
		}
	}
power_settings_free:
	mutex_lock(&cam_pm->mlock);
	power_record->count = 0;
	kfree(power_record->power_settings);
	power_record->power_settings = NULL;
	mutex_unlock(&cam_pm->mlock);

	return ret;
}

