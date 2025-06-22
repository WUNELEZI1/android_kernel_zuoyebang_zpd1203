// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 XiaoMi, Inc. All rights reserved.
 */
#define pr_fmt(fmt)	"mi-disp-parse:[%s:%d] " fmt, __func__, __LINE__
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include "mi_disp_print.h"
#include "dsi_panel.h"
#include "mi_panel_id.h"

#define SMEM_SW_DISPLAY_GAMMA_TABLE 499
#define DEFAULT_MAX_BRIGHTNESS_CLONE ((1 << 14) - 1)

int mi_dsi_panel_parse_esd_gpio_config(struct dsi_panel *panel)
{
	int rc = 0;
	struct device_node *np;
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;

	np = panel->mipi_device.dev.of_node;
	mi_cfg->esd_err_irq_gpio = of_get_named_gpio(
			np, "mi,esd-err-irq-gpio", 0);
	if (gpio_is_valid(mi_cfg->esd_err_irq_gpio)) {
		mi_cfg->esd_err_irq = gpio_to_irq(mi_cfg->esd_err_irq_gpio);
		rc = gpio_request(mi_cfg->esd_err_irq_gpio, "esd_err_irq_gpio");
		if (rc)
			DISP_ERROR("Failed to request esd irq gpio %d, rc=%d\n",
				mi_cfg->esd_err_irq_gpio, rc);
		else
			gpio_direction_input(mi_cfg->esd_err_irq_gpio);
	}

	rc = of_property_read_u32(np, "mi,esd-err-irq-gpio-flag", &mi_cfg->esd_err_irq_flags);
	if (mi_cfg->esd_err_irq_flags)
		DISP_INFO("mi,esd-err-irq-gpio-flag is defined\n");

	return rc;
}

static void mi_dsi_panel_parse_round_corner_config(struct dsi_panel *panel)
{
	int rc = 0;
	struct device_node *np;
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;

	np = panel->mipi_device.dev.of_node;
	mi_cfg->ddic_round_corner_enabled =
			of_property_read_bool(np, "mi,ddic-round-corner-enabled");
	if (mi_cfg->ddic_round_corner_enabled)
		DISP_INFO("mi,ddic-round-corner-enabled is defined\n");

	rc = of_property_read_u32(np, "mi,ddic-round-corner-port-mode",
			&mi_cfg->ddic_round_corner_port_mode);

	DISP_INFO("mi,ddic-round-corner-mode is %d port mode\n",
			mi_cfg->ddic_round_corner_port_mode);
}

static void mi_dsi_panel_parse_flat_config(struct dsi_panel *panel)
{
	struct device_node *np;
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;

	np = panel->mipi_device.dev.of_node;
	mi_cfg->flat_sync_te = of_property_read_bool(np, "mi,flat-need-sync-te");
	if (mi_cfg->flat_sync_te)
		DISP_INFO("mi,flat-need-sync-te is defined\n");
	else
		DISP_DEBUG("mi,flat-need-sync-te is undefined\n");
	mi_cfg->flatmode_default_on_enabled = of_property_read_bool(np,
				"mi,flatmode-default-on-enabled");
	if (mi_cfg->flatmode_default_on_enabled)
		DISP_INFO("flat mode is  default enabled\n");

#ifdef CONFIG_FACTORY_BUILD
	mi_cfg->flat_sync_te = false;
#endif

	mi_cfg->flat_update_flag = of_property_read_bool(np, "mi,flat-update-flag");
	if (mi_cfg->flat_update_flag)
		DISP_INFO("mi,flat-update-flag is defined\n");
	else
		DISP_DEBUG("mi,flat-update-flag is undefined\n");
}

static int mi_dsi_panel_parse_dc_config(struct dsi_panel *panel)
{
	int rc = 0;
	struct device_node *np;
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;
	const char *string;

	np = panel->mipi_device.dev.of_node;
	mi_cfg->dc_feature_enable = of_property_read_bool(np, "mi,dc-feature-enabled");
	if (!mi_cfg->dc_feature_enable) {
		DISP_DEBUG("mi,dc-feature-enabled not defined\n");
		return rc;
	}
	DISP_INFO("mi,dc-feature-enabled is defined\n");

	rc = of_property_read_string(np, "mi,dc-feature-type", &string);
	if (rc) {
		DISP_ERROR("mi,dc-feature-type not defined!\n");
		return -EINVAL;
	}
	if (!strcmp(string, "crc_skip_backlight")) {
		mi_cfg->dc_type = TYPE_CRC_SKIP_BL;
	} else {
		DISP_ERROR("No valid mi,dc-feature-type string\n");
		return -EINVAL;
	}
	DISP_INFO("mi, dc type is %s\n", string);

	mi_cfg->dc_update_flag = of_property_read_bool(np, "mi,dc-update-flag");
	if (mi_cfg->dc_update_flag)
		DISP_INFO("mi,dc-update-flag is defined\n");
	else
		DISP_DEBUG("mi,dc-update-flag not defined\n");

	rc = of_property_read_u32(np, "mi,dsi-panel-dc-threshold", &mi_cfg->dc_threshold);
	if (rc) {
		mi_cfg->dc_threshold = 440;
		DISP_INFO("default dc threshold is %d\n", mi_cfg->dc_threshold);
	} else {
		DISP_INFO("dc threshold is %d\n", mi_cfg->dc_threshold);
	}

	return rc;
}

static int mi_dsi_panel_parse_backlight_config(struct dsi_panel *panel)
{
	int rc = 0;
#ifdef CONFIG_FACTORY_BUILD
	u32 val = 0;
#endif
	struct device_node *np;
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;

	np = panel->mipi_device.dev.of_node;
	rc = of_property_read_u32(np, "mi,panel-on-dimming-delay", &mi_cfg->panel_on_dimming_delay);
	if (rc) {
		mi_cfg->panel_on_dimming_delay = 0;
		DISP_INFO("mi,panel-on-dimming-delay not specified\n");
	} else {
		DISP_INFO("mi,panel-on-dimming-delay is %d\n", mi_cfg->panel_on_dimming_delay);
	}

	rc = of_property_read_u32(np, "mi,doze-hbm-dbv-level", &mi_cfg->doze_hbm_dbv_level);
	if (rc) {
		mi_cfg->doze_hbm_dbv_level = 0;
		DISP_INFO("mi,doze-hbm-dbv-level not specified\n");
	} else {
		DISP_INFO("mi,doze-hbm-dbv-level is %d\n", mi_cfg->doze_hbm_dbv_level);
	}

	rc = of_property_read_u32(np, "mi,doze-lbm-dbv-level", &mi_cfg->doze_lbm_dbv_level);
	if (rc) {
		mi_cfg->doze_lbm_dbv_level = 0;
		DISP_INFO("mi,doze-lbm-dbv-level not specified\n");
	} else {
		DISP_INFO("mi,doze-lbm-dbv-level is %d\n", mi_cfg->doze_lbm_dbv_level);
	}
	rc = of_property_read_u32(np, "mi,max-brightness-clone", &mi_cfg->max_brightness_clone);
	if (rc)
		mi_cfg->max_brightness_clone = DEFAULT_MAX_BRIGHTNESS_CLONE;
	DISP_INFO("max_brightness_clone=%d\n", mi_cfg->max_brightness_clone);

	rc = of_property_read_u32(np, "mi,max-brightness-normal", &mi_cfg->bl_normal_max);
	if (rc)
		mi_cfg->bl_normal_max = 0;
	DISP_INFO("bl_normal_max=%d\n", mi_cfg->bl_normal_max);
#ifdef CONFIG_FACTORY_BUILD
	rc = of_property_read_u32(np, "mi,dsi-panel-fac-brightness-max-level", &val);
	if (rc) {
		rc = 0;
		DSI_DEBUG("[%s] factory brigheness-max-level unspecified\n", panel->panel_info.name);
	} else {
		panel->panel_info.bl_config.brightness_max_level = val;
	}
	DISP_INFO("brightness_max_level is %d\n", panel->panel_info.bl_config.brightness_max_level);
#endif

	mi_cfg->disable_ic_dimming = of_property_read_bool(np, "mi,disable-ic-dimming-flag");
	if (mi_cfg->disable_ic_dimming)
		DISP_INFO("disable_ic_dimming\n");

	rc = of_property_read_u32(np, "mi,panel-hbm-backlight-threshold", &mi_cfg->hbm_backlight_threshold);
	if (rc)
		mi_cfg->hbm_backlight_threshold = 8192;
	DISP_INFO("panel hbm backlight threshold %d\n", mi_cfg->hbm_backlight_threshold);

	mi_cfg->count_hbm_by_backlight = of_property_read_bool(np, "mi,panel-count-hbm-by-backlight-flag");
	if (mi_cfg->count_hbm_by_backlight)
		DISP_INFO("panel count hbm by backlight\n");

	mi_cfg->hwc_set_doze_brightness = of_property_read_bool(np, "mi,hwc-set-doze-brightness");
	if (mi_cfg->hwc_set_doze_brightness)
		DISP_INFO("hwc set doze brightness\n");
	mi_cfg->xeq_enabled = of_property_read_bool(np, "mi,dsi-panel-xeq-enabled");
	if (mi_cfg->xeq_enabled)
		DISP_INFO("panel xeq enabled\n");
	return 0;
}

static int mi_dsi_panel_parse_demura_config(struct dsi_panel *panel)
{
	int rc = 0;
	struct device_node *np;
	struct panel_demura_info *demura_info = &panel->mi_panel.mi_cfg.demura_info;

	np = panel->mipi_device.dev.of_node;
	rc = of_property_read_u32(np, "mi,panel-demura-change-dbv-threshold",
			&demura_info->demura_change_dbv_threshold);
	if (rc)
		demura_info->demura_change_dbv_threshold = 0;

	DISP_INFO("demura_change_dbv_threshold is %d\n",
			demura_info->demura_change_dbv_threshold);

	return rc;
}

static int mi_dsi_panel_parse_ic_dimming_config(struct dsi_panel *panel)
{
	int rc = 0;
	struct device_node *np;
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;

	np = panel->mipi_device.dev.of_node;
	rc = of_property_read_u32(np, "mi,panel-ic-dimming-dbv-threshold",
			&mi_cfg->ic_dimming_dbv_threshold);
	if (rc)
		mi_cfg->ic_dimming_dbv_threshold = 0;
	DISP_INFO("ic_dimming_dbv_threshold is %d\n",
			mi_cfg->ic_dimming_dbv_threshold);

	return rc;
}

int mi_dsi_panel_parse_config(struct dsi_panel *panel)
{
	int rc = 0;
	struct device_node *np;
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;

	np = panel->mipi_device.dev.of_node;
	rc = of_property_read_u64(np, "mi,panel-id", &mi_cfg->mi_panel_id);
	if (rc) {
		mi_cfg->mi_panel_id = 0;
		DISP_INFO("mi,panel-id not specified\n");
	} else {
		DISP_INFO("mi,panel-id is 0x%llx (%s)\n",
			mi_cfg->mi_panel_id, mi_get_panel_id_name(mi_cfg->mi_panel_id));
	}

	mi_cfg->panel_build_id_read_needed =
		of_property_read_bool(np, "mi,panel-build-id-read-needed");
	if (mi_cfg->panel_build_id_read_needed) {
		rc = dsi_panel_parse_build_id_read_config(panel);
		if (rc) {
			mi_cfg->panel_build_id_read_needed = false;
			DISP_ERROR("[%s] failed to get panel build id read infos, rc=%d\n",
					panel->panel_info.name, rc);
		}
	}
	rc = dsi_panel_parse_cell_id_read_config(panel);
	if (rc) {
		DISP_ERROR("[%s] failed to get panel cell id read infos, rc=%d\n",
			panel->panel_info.name, rc);
	}
	rc = dsi_panel_parse_wp_reg_read_config(panel);
	if (rc) {
		DISP_ERROR("[%s] failed to get panel wp read infos, rc=%d\n",
			panel->panel_info.name, rc);
	}

	rc = of_property_read_u32(np, "mi,panel-reset-post-off-delay",
			&mi_cfg->reset_post_off_delay);
	if (rc) {
		mi_cfg->reset_post_off_delay = 0;
		DISP_INFO("mi,panel-reset-post-off-delay not specified\n");
	} else {
		DISP_INFO("mi,panel-reset-post-off-delay is %d\n",
				mi_cfg->reset_post_off_delay);
	}

	mi_cfg->fps_change_notify_enabled =
			of_property_read_bool(np, "mi,fps-change-notify-enabled");
	DISP_INFO("fps_change_notify_enabled = %d\n", mi_cfg->fps_change_notify_enabled);

	mi_dsi_panel_parse_round_corner_config(panel);
	mi_dsi_panel_parse_flat_config(panel);
	mi_dsi_panel_parse_dc_config(panel);
	mi_dsi_panel_parse_backlight_config(panel);
	mi_dsi_panel_parse_demura_config(panel);
	mi_dsi_panel_parse_ic_dimming_config(panel);

	return rc;
}
