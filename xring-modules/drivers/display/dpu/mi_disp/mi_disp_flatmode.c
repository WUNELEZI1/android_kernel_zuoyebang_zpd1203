// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 XiaoMi, Inc. All rights reserved.
 */

#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include "mi_disp_print.h"
#include "dsi_panel.h"
#include "mi_dsi_panel.h"
#include "mi_dsi_display.h"
#include "mi_disp_flatmode.h"

static bool mi_disp_parse_flatmode_status_len(struct device_node *np,
	char *prop_key, u32 **target, u32 cmd_cnt)
{
	int tmp;

	if (!of_find_property(np, prop_key, &tmp))
		return false;

	tmp /= sizeof(u32);
	if (tmp != cmd_cnt) {
		DISP_ERROR("request property(%d) do not match cmd num_cmds(%d)\n",
				tmp, cmd_cnt);
		return false;
	}

	*target = kcalloc(tmp, sizeof(u32), GFP_KERNEL);
	if (IS_ERR_OR_NULL(*target)) {
		DISP_ERROR("Error allocating memory for property\n");
		return false;
	}

	if (of_property_read_u32_array(np, prop_key, *target, tmp)) {
		DISP_ERROR("cannot get values from dts\n");
		kfree(*target);
		*target = NULL;
		return false;
	}

	return true;
}

static int mi_dsi_panel_parse_flatmode_configs(
		struct dsi_panel *panel, struct mi_panel_flatmode_config *config)
{
	int rc = 0;
	u32 tmp;
	u32 i, status_len;
	struct property *data;
	struct device_node *np;

	if (!panel || !config) {
		DISP_ERROR("invalid param!\n");
		rc = -EAGAIN;
		return rc;
	}

	np = panel->mipi_device.dev.of_node;
	config->flatmode_check_enabled = of_property_read_bool(np,
				"mi,panel-flatmode-status-check-enabled");
	if (!config->flatmode_check_enabled)
		goto error;

	dsi_panel_cmd_single_parse(&config->offset_cmd,
				DSI_CMD_SET_MI_FLATMODE_STATUS_OFFSET, np);
	if (!config->offset_cmd.num_cmds)
		DISP_INFO("flat mode status offset command not specified\n");

	dsi_panel_cmd_single_parse(&config->status_cmd,
				DSI_CMD_SET_MI_FLATMODE_STATUS, np);
	if (!config->status_cmd.num_cmds) {
		DISP_ERROR("flat mode status command parsing failed\n");
		rc = -EINVAL;
		goto error;
	}

	if (!mi_disp_parse_flatmode_status_len(np,
			"mi,dsi-panel-flatmode-status-read-length",
			&(config->status_cmds_rlen),
				config->status_cmd.num_cmds)) {
		DISP_ERROR("Invalid flat mode status read length\n");
		rc = -EINVAL;
		goto error1;
	}

	status_len = 0;
	for (i = 0; i < config->status_cmd.num_cmds; ++i)
		status_len += config->status_cmds_rlen[i];
	if (!status_len) {
		rc = -EINVAL;
		goto error2;
	}

	data = of_find_property(np,
			"mi,dsi-panel-flatmode-on-status-value", &tmp);
	tmp /= sizeof(u32);
	if (IS_ERR_OR_NULL(data) || tmp != status_len) {
		DISP_ERROR("error parse panel-status-value\n");
		rc = -EINVAL;
		goto error2;
	}

	config->status_value = kzalloc(sizeof(u32) * status_len, GFP_KERNEL);
	if (!(config->status_value)) {
		rc = -ENOMEM;
		goto error2;
	}
	config->return_buf = kcalloc(status_len, sizeof(unsigned char), GFP_KERNEL);
	if (!config->return_buf) {
		rc = -ENOMEM;
		goto error3;
	}
	config->status_buf = kzalloc(256, GFP_KERNEL);
	if (!config->status_buf) {
		rc = -ENOMEM;
		goto error4;
	}

	rc = of_property_read_u32_array(np,
		"mi,dsi-panel-flatmode-on-status-value",
		config->status_value, status_len);
	if (rc) {
		DISP_ERROR("error reading flat mode status values\n");
		memset(config->status_value, 0, status_len);
		goto error5;
	}

	dsi_panel_cmd_single_parse(&config->offset_end_cmd,
				DSI_CMD_SET_MI_FLATMODE_STATUS_OFFSET_END, np);
	if (!config->offset_end_cmd.num_cmds)
		DISP_INFO("flat mode status end command not specified\n");

	return rc;

error5:
	kfree(config->status_buf);
error4:
	kfree(config->return_buf);
error3:
	kfree(config->status_value);
error2:
	kfree(config->status_cmds_rlen);
error1:
	dsi_panel_destroy_cmd_packets(&config->status_cmd);
	dsi_panel_dealloc_cmd_packets(&config->status_cmd);
error:
	dsi_panel_destroy_cmd_packets(&config->offset_cmd);
	dsi_panel_dealloc_cmd_packets(&config->offset_cmd);
	config->flatmode_check_enabled = false;
	return rc;
}


static void mi_dsi_panel_flatmode_freemem(struct mi_panel_flatmode_config *config)
{
	if (config && config->flatmode_check_enabled) {
		dsi_panel_destroy_cmd_packets(&config->offset_cmd);
		dsi_panel_dealloc_cmd_packets(&config->offset_cmd);

		dsi_panel_destroy_cmd_packets(&config->status_cmd);
		dsi_panel_dealloc_cmd_packets(&config->status_cmd);

		dsi_panel_destroy_cmd_packets(&config->offset_end_cmd);
		dsi_panel_dealloc_cmd_packets(&config->offset_end_cmd);

		kfree(config->status_buf);
		kfree(config->return_buf);
		kfree(config->status_value);
		kfree(config->status_cmds_rlen);
	}
}

static int mi_flatmode_write_offset_cmd(struct dsi_display *display,
		struct mi_panel_flatmode_config *config, bool is_end)
{
	int rc = 0;
	struct dsi_cmd_set *cmd_sets = &config->offset_cmd;

	if (is_end)
		cmd_sets = &config->offset_end_cmd;
	if (cmd_sets->num_cmds != 0) {
		rc = mi_dsi_display_cmd_write(display, cmd_sets);
		if (rc)
			DISP_ERROR("send flatmode cmd failed!");
	}
	return rc;
}

static bool mi_flatmode_validate_status(struct dsi_display *display,
		struct mi_panel_flatmode_config *config)
{
	int i, num_cmds = 0, start = 0, len = 0, *lenp;
	struct dsi_cmd_desc *cmds;
	u16 flags = 0;

	lenp = config->status_cmds_rlen;
	num_cmds = config->status_cmd.num_cmds;
	cmds = config->status_cmd.cmds;
	memset(config->return_buf, 0x0, sizeof(*config->return_buf));

	if (config->status_cmd.cmd_state == DSI_CMD_SET_STATE_LP)
		flags = MIPI_DSI_MSG_USE_LPM;

	for (i = 0; i < num_cmds; ++i) {
		memset(config->status_buf, 0x0, 256);
		cmds[i].msg.flags |= flags;
		if (mi_dsi_display_cmd_read(display, cmds[i], config->status_buf, lenp[i]) != 0) {
			DISP_ERROR("mi_dsi_display_cmd_read fail\n");
			goto error;
		}

		memcpy(config->return_buf+start, config->status_buf, lenp[i]);
		start += lenp[i];
	}

	for (i = 0; i < num_cmds; i++)
		len += lenp[i];

	for (i = 0; i < len; ++i) {
		DISP_INFO(" flat mode return_buff[%d] = 0x%x, status_value[%d] = 0x%x",
						i, config->return_buf[i], i, config->status_value[i]);
		if (config->return_buf[i] != config->status_value[i])
			goto error;
	}

	return true;

error:
	return false;
}

int mi_dsi_panel_flatmode_validate_status(struct dsi_display *display, bool *status)
{
	int rc = 0;
	struct mi_panel_flatmode_config flatmode_config;

	if (!display || !display->panel || !status) {
		DISP_ERROR("Invalid ptr\n");
		return -EINVAL;
	}

	if (!display->panel->initialized) {
		DISP_WARN("[%s] panel not initialized!\n", display->panel->panel_info.name);
		return -ENODEV;
	}

	memset(&flatmode_config, 0, sizeof(flatmode_config));
	mi_dsi_panel_parse_flatmode_configs(display->panel, &flatmode_config);

	if (!flatmode_config.flatmode_check_enabled) {
		DISP_INFO("flatmode check not enabled\n");
		return rc;
	}

	rc = mi_flatmode_write_offset_cmd(display, &flatmode_config, false);
	if (rc)
		return -EINVAL;
	*status = mi_flatmode_validate_status(display, &flatmode_config);

	rc = mi_flatmode_write_offset_cmd(display, &flatmode_config, true);

	mi_dsi_panel_flatmode_freemem(&flatmode_config);

	return rc;
}
