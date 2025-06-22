// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/backlight.h>
#include <drm/drm_device.h>

#include "dpu_power_mgr.h"
#include "dpu_backlight.h"
#include "dsi_display.h"
#include "dpu_log.h"
#include "dsi_power.h"

#define BL_NODE_NAME_SIZE 32

static int dpu_backlight_device_update_status(struct backlight_device *bd)
{
	struct dsi_display *display;
	int brightness;
	int ret;

	brightness = bd->props.brightness;

	display = bl_get_data(bd);
	if (display == NULL || display->panel == NULL)
		return -EINVAL;

	ret = dsi_display_backlight_set(display, brightness, USE_CPU);
	if (ret)
		DPU_ERROR("unable to set backlight\n");

	return ret;
}

static int dpu_backlight_device_get_brightness(struct backlight_device *bd)
{
	struct dsi_display *display;
	int brightness;

	display = bl_get_data(bd);
	if (display == NULL || display->panel == NULL)
		return -EINVAL;

	brightness = display->panel->panel_info.bl_config.current_brightness;
	return brightness;
}

static const struct backlight_ops dpu_backlight_device_ops = {
	.update_status = dpu_backlight_device_update_status,
	.get_brightness = dpu_backlight_device_get_brightness,
};

int dpu_backlight_init(struct dsi_display *display, struct drm_device *dev,
		struct drm_connector *drm_dsi_conn)
{
	struct dsi_backlight_config bl_config;
	char bl_node_name[BL_NODE_NAME_SIZE];
	struct backlight_properties props;

	bl_config = display->panel->panel_info.bl_config;
	memset(&props, 0, sizeof(props));
	props.type = BACKLIGHT_RAW;
	props.max_brightness = bl_config.brightness_max_level;
	/* if uefi display is diabled, the brightness is 0 */
	props.brightness = bl_config.current_brightness;

	snprintf(bl_node_name, BL_NODE_NAME_SIZE, "panel%u-backlight",
			drm_dsi_conn->connector_type_id - 1);

	display->bl_device = backlight_device_register(bl_node_name, dev->dev, display,
			&dpu_backlight_device_ops, &props);
	if (IS_ERR_OR_NULL(display->bl_device)) {
		DSI_ERROR("Failed to register backlight: %ld\n",
				PTR_ERR(display->bl_device));
		return -ENODEV;
	}

	display->panel->bl_device = display->bl_device;
	return 0;
}

void dpu_backlight_deinit(struct dsi_display *display)
{
	backlight_device_unregister(display->bl_device);
	display->panel->bl_device = NULL;
}

int dpu_backlight_parse_bl_info(struct dsi_backlight_config *config,
		struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "dsi-panel-brightness-min-level",
		&config->brightness_min_level);
	if (ret) {
		config->brightness_min_level = 0;
		DSI_ERROR("failed to parse min brightness, set as default 0\n");
	}

	ret = of_property_read_u32(np, "dsi-panel-brightness-max-level",
		&config->brightness_max_level);
	if (ret) {
		config->brightness_max_level = DSI_BL_LEVEL_MAX;
		DSI_ERROR("failed to parse max brightness, set as default %d\n",
				DSI_BL_LEVEL_MAX);
	} else if (config->brightness_max_level >= DSI_BL_LEVEL_MAX) {
		DSI_ERROR("get invalid max brightness %d, set as default %d\n",
				config->brightness_max_level, DSI_BL_LEVEL_MAX);
		config->brightness_max_level = DSI_BL_LEVEL_MAX;
#ifdef MI_DISPLAY_MODIFY
	} else {
		ret = of_property_read_u32(np, "dsi-panel-actual-brightness-max-level",
			&config->actual_brightness_max_level);
		if (!ret) {
			DSI_INFO("parse max brightness %d, actual max brightness %d\n",
				config->brightness_max_level, config->actual_brightness_max_level);
		}
#endif
	}

	ret = of_property_read_u32(np, "dsi-panel-brightness-init-level",
		&config->brightness_init_level);
	if (ret) {
		DSI_INFO("brigheness-init-level 40% max level as default\n");
		config->brightness_init_level =
			(config->brightness_max_level * 40) / 100;
		ret = 0;
	}

	return ret;
}
