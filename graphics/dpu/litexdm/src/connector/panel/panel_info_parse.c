// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#include "osal.h"
#include "panel_mgr.h"
#include "dpu_log.h"
#include "XRRegulator.h"
#include "pmic/pmic_regulator_def.h"
#include "panel.h"
#include "dpu_hw_power_ops.h"

enum {
	DSI0_ID = 0,
	DSI1_ID,
	DSI_NUM,
};

#define PANEL_IO_SEQ_PATTERN_SIZE 5

/* The cmd set name should be added here when adding a new cmd set.
 * Note: DSI_CMD_SET_BACKLIGHT not parsed from DTSI, generated dynamically.
 */
const char *cmd_set_names[DSI_CMD_SET_MAX] = {
	"dsi-panel-on-command",
	"dsi-panel-off-command",
	"dsi-panel-backlight-command(generated dynamically)",
	"dsi-panel-read-build-id-command",
	"mi,dsi-panel-xeq-sync-command",
};

/* The cmd set state name should be added here when adding a new cmd set.
 * Note: DSI_CMD_SET_BACKLIGHT not parsed from DTSI, generated dynamically.
 */
const char *cmd_set_state_names[DSI_CMD_SET_MAX] = {
	"dsi-panel-on-command-state",
	"dsi-panel-off-command-state",
	"dsi-panel-backlight-command-state(generated dynamically)",
	"dsi-panel-read-build-id-command-state",
	"mi,dsi-panel-xeq-sync-command-state",
};

int dsi_panel_offset_get(void *fdt, const char *panel_node_name)
{
	int offset;

	offset = dpu_get_subnode_offset(fdt, 0, panel_node_name);
	if (offset < 0) {
		dpu_pr_err("get invalid offset, path:%a\n", panel_node_name);
		return -1;
	}

	dpu_pr_debug("get panel_node_name:%a, offset:%d\n",
			panel_node_name, offset);
	return offset;
}

static void parse_port_mask(uint8_t port_mask, struct dpu_panel_info *base_info)
{
	if (port_mask == (TMG_DSI0 | TMG_DSI1)) {
		base_info->dual_port = true;
		base_info->connector_id = 0;
		base_info->external_connector_id = 1;
		base_info->xres *= 2;
	}

	if (port_mask == TMG_DSI0)
		base_info->connector_id = 0;
	else if (port_mask == TMG_DSI1)
		base_info->connector_id = 1;

	base_info->port_mask = port_mask;

	dpu_pr_info("port_mask:0x%x dual port:%d conn_id:%u ext_conn_id:%u\n",
			base_info->port_mask, base_info->dual_port,
			base_info->connector_id, base_info->external_connector_id);
}

static void parse_dsc_custom_pps(void *fdt, int32_t offset, struct dsc_parms *dsc)
{
	int pps_size, ret, i;
	u8 *customized_pps;

	dsc->customized_pps_table = NULL;
	pps_size = dpu_dts_get_u8_array_size(fdt, offset, "dsi-panel-dsc-pps-table");
	if (pps_size != DSC_PPS_TABLE_BYTES_SIZE) {
		dpu_pr_info("no customized pps table, size: %d\n", pps_size);
		return;
	}

	customized_pps = dpu_mem_alloc(DSC_PPS_TABLE_BYTES_SIZE *
			sizeof(*customized_pps));
	if (!customized_pps)
		return;

	ret = dpu_dts_parse_u8_array(fdt, offset, "dsi-panel-dsc-pps-table", customized_pps);
	if (ret) {
		dpu_pr_err("failed to get customized pps table\n");
		goto error;
	}

	dpu_pr_info("get customized pps table\n");
	for (i = 0; i < DSC_PPS_TABLE_BYTES_SIZE; i++) {
		dpu_pr_debug("PPS[%d]: 0x%x\n", i, customized_pps[i]);
	}

	dsc->customized_pps_table = customized_pps;
	return;
error:
	dpu_mem_free(customized_pps);
	return;
}

static int parse_dsc_params(void *fdt, int32_t offset, struct dpu_panel_info *base_info)
{
	struct dsc_parms *dsc = &base_info->dsc_cfg;
	const char *dsc_type;
	u32 version;
	int ret;

	dsc->dsc_en = false;
	base_info->dsc_en = false;
	ret = dpu_dts_parse_string(fdt, offset,
			"dsi-panel-dsc-compression-mode", &dsc_type);
	if (ret < 0 || dpu_str_cmp(dsc_type, "dsc")) {
		dpu_pr_info("panel not support dsc\n");
		return 0;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-dsc-version", &version);
	if (ret < 0) {
		dpu_pr_err("parse dsi-panel-dsc-version failed\n");
		return -1;
	}
	dsc->dsc_version_major = DSC_VERSION_MAJOR_V1;
	switch (version) {
	case DSC_VERSION_V11:
		dsc->dsc_version_minor = DSC_VERSION_MINOR_V1;
		break;
	case DSC_VERSION_V12A:
		dsc->dsc_version_minor = DSC_VERSION_MINOR_V2A;
		break;
	default:
		dpu_pr_err("dsc-version (0x%x) is out of range.\n", version);
		return -1;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-dsc-pic-height",
			&dsc->pic_height);
	if (ret) {
		dpu_pr_err("failed to get dsc-pic-heigh!\n");
		return -1;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-dsc-pic-width",
			&dsc->pic_width);
	if (ret) {
		dpu_pr_err("failed to get dsc-pic-width!\n");
		return -1;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-dsc-slice-height",
			&dsc->slice_height);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-dsc-slice-height!\n");
		return -1;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-dsc-slice-width",
			&dsc->slice_width);
	if (ret) {
		dpu_pr_err("failed to get dsc-slicew!\n");
		return -1;
	}

	ret = dpu_dts_parse_u8(fdt, offset, "dsi-panel-dsc-bpc", &dsc->bits_per_component);
	if (ret) {
		dpu_pr_err("failed to get dsc-bpc!\n");
		return -1;
	}

	/* bits per pixel after dsc compression */
	ret = dpu_dts_parse_u8(fdt, offset, "dsi-panel-dsc-bpp", &dsc->bits_per_pixel);
	if (ret) {
		dpu_pr_err("failed to get dsc-bpp!\n");
		return -1;
	}

	dsc->dsc_en = true;
	base_info->dsc_en = true;

	dpu_pr_info("dsc version:0x%x, bpc: %u, bpp: %u,"
			"pic: %u x %u, slice: %u x %u\n",
			version, dsc->bits_per_component, dsc->bits_per_pixel,
			dsc->pic_width, dsc->pic_height,
			dsc->slice_width, dsc->slice_height);

	parse_dsc_custom_pps(fdt, offset, dsc);
	return 0;
}

static int parse_backlight_info(void *fdt, int32_t offset,
		struct dpu_panel_info *base_info)
{
	const char *backlight_setting_type;
	const char *bias_ic_name;
	int ret = 0;

	ret = dpu_dts_parse_string(fdt, offset, "dsi-panel-backlight-setting-type", &backlight_setting_type);
	if (ret < 0 || !backlight_setting_type) {
		dpu_pr_warn("parse backlight_setting_type failed, set as default cmd\n");
		base_info->backlight_setting_type = BACKLIGHT_SET_BY_DSI;
	} else if (!dpu_str_cmp(backlight_setting_type, "i2c")) {
		base_info->backlight_setting_type = BACKLIGHT_SET_BY_I2C;
	} else if (!dpu_str_cmp(backlight_setting_type, "cmd")) {
		base_info->backlight_setting_type = BACKLIGHT_SET_BY_DSI;
	} else {
		dpu_pr_warn("parse backlight_setting_type failed, set as default cmd\n");
		base_info->backlight_setting_type = BACKLIGHT_SET_BY_DSI;
	}

	ret = dpu_dts_parse_string(fdt, offset, "dsi-panel-bias-ic-name", &bias_ic_name);
	if (ret < 0 || !bias_ic_name) {
		dpu_pr_warn("parse bias_ic_name failed, set as default name\n");
		base_info->bias_ic_name = "KTZ8866";
	} else if (!dpu_str_cmp(bias_ic_name, "AW37504")) {
		base_info->bias_ic_name = "AW37504";
	} else {
		dpu_pr_warn("parse bias_ic_name failed, set as default name\n");
		base_info->bias_ic_name = "KTZ8866";
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-brightness-max-level",
		&base_info->brightness_max_level);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-brightness-max-level,"
				"set as default 2047\n");
		base_info->brightness_max_level = 2047;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-brightness-min-level",
		&base_info->brightness_min_level);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-brightness-min-level,"
				"set as default 16\n");
		base_info->brightness_min_level = 16;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-brightness-init-level",
		&base_info->brightness_init_level);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-brightness-init-level,"
				"set as default 1023\n");
		base_info->brightness_init_level = 1023;
		ret = 0;
	}

	dpu_pr_info("bl_type:%a, max:%u, min:%u, init:%u\n",
			backlight_setting_type, base_info->brightness_max_level,
			base_info->brightness_min_level,
			base_info->brightness_init_level);

	return ret;
}

static int dsi_panel_video_config_parse(void *fdt, int32_t offset,
		struct panel_drv_private *priv)
{
	struct dpu_panel_info *base_info;
	const char *burst_mode;
	int ret = 0;

	base_info = &priv->base;
	ret = dpu_dts_parse_string(fdt, offset, "dsi-burst-mode", &burst_mode);
	if (ret < 0 || !burst_mode) {
		dpu_pr_err("parse dsi-burst-mod failed\n");
		return -1;
	}

	dpu_pr_info("dsi-burst-mode: %a\n", burst_mode);
	if (!dpu_str_cmp(burst_mode, "non_burst_sync_events")) {
		base_info->burst_mode = NON_BURST_WITH_SYNC_EVENTS;
	} else if (!dpu_str_cmp(burst_mode, "non_burst_sync_pulses")) {
		base_info->burst_mode = NON_BURST_WITH_SYNC_PULSES;
	} else if (!dpu_str_cmp(burst_mode, "burst_mode")) {
		base_info->burst_mode = BURST_MODE;
	} else {
		dpu_pr_err("invalid burst_mode:%a\n", burst_mode);
		ret = -1;
	}

	return ret;
}

static int dsi_panel_cmd_config_parse(void *fdt, int32_t offset,
		struct panel_drv_private *priv)
{
	int ret = 0;

	if (!priv->is_fake_panel && priv->base.is_asic) {
		ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-te-gpio-num",
			&priv->lcd_te);
		if (ret)
			dpu_pr_err("failed to get dsi-panel-te-gpio-num!\n");

		dpu_pr_info("te-gpio-num: %u\n", priv->lcd_te);
	} else {
		dpu_pr_info("skip parse dsi-panel-te-gpio-num on FPGA!\n");
	}

	return ret;
}

static int dsi_panel_common_parse(void *fdt, int32_t offset, struct panel_drv_private *priv)
{
	struct dpu_panel_info *base_info;
	struct connector *bind_connector = NULL;
	struct connector *connector;
	const char *display_type;
	int32_t dpu_offset;
	uint32_t asic_flag;
	u8 port_mask;
	int ret;

	base_info = &priv->base;

	dpu_offset = dpu_get_fdt_offset(fdt, DPU_DTS_PATH);
	if (dpu_offset < 0) {
		dpu_pr_err("get invalid dpu_offset, path:%a\n", DPU_DTS_PATH);
		return -1;
	}

	base_info->is_asic = true;
	ret = dpu_dts_parse_u32(fdt, dpu_offset, "asic-flag", &asic_flag);
	if (!ret && !asic_flag)
		base_info->is_asic = false;

	ret = dpu_dts_parse_string(fdt, offset, "dsi-panel-name", &base_info->name);
	if (ret < 0 || !base_info->name)
		dpu_pr_err("parse panel name failed\n");

	dpu_pr_info("dsi-panel-name:%a\n", base_info->name);

	if (!dpu_str_cmp(base_info->name, "fake_dsi_panel"))
		priv->is_fake_panel = true;
	else
		priv->is_fake_panel = false;
	dpu_pr_info("hw platform:%a\n", base_info->is_asic ? "asic" : "fpga");

	ret = dpu_dts_parse_string(fdt, offset, "dsi-panel-type", &display_type);
	if (ret < 0) {
		dpu_pr_err("parse dsi-panel-type failed\n");
		return -1;
	}

	if (!dpu_str_cmp(display_type, "dsi_cmd_mode")) {
		base_info->display_type = DSI_CMD_MODE;
		ret = dsi_panel_cmd_config_parse(fdt, offset, priv);
		if (ret)
			dpu_pr_err("failed to parse cmd config\n");

	} else if (!dpu_str_cmp(display_type, "dsi_video_mode")) {
		base_info->display_type = DSI_VIDEO_MODE;
		ret = dsi_panel_video_config_parse(fdt, offset, priv);
		if (ret)
			dpu_pr_err("failed to parse video config\n");
	} else {
		dpu_pr_err("invalid display type:%a\n", display_type);
		base_info->display_type = DSI_VIDEO_MODE;
		ret = dsi_panel_video_config_parse(fdt, offset, priv);
		if (ret)
			dpu_pr_err("failed to parse video config\n");
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-width", &base_info->xres);
	if (ret < 0)
	    dpu_pr_err("parse dsi-panel-width failed\n");

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-height", &base_info->yres);
	if (ret < 0)
	    dpu_pr_err("parse dsi-panel-height failed\n");

	dpu_pr_info("dsi-panel-type:%a, w x h = %u x %u\n",
			base_info->display_type ? "cmd" : "video",
			base_info->xres, base_info->yres);

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-id", &base_info->panel_id);
	if (ret < 0)
		dpu_pr_err("parse dsi-panel-id failed\n");

	dpu_pr_debug("dsi-panel-id:%u\n", base_info->panel_id);

	ret = parse_dsc_params(fdt, offset, base_info);
	if (ret)
		dpu_pr_err("failed to parse dsc params\n");

	ret = dpu_dts_parse_u8(fdt, offset, "dsi-panel-port-mask", &port_mask);
	if (ret < 0 || !port_mask) {
		dpu_pr_err("parse panel port-mask failed\n");
		return -1;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-boot-up-profile", &base_info->bootup_profile);
	if (ret < 0) {
		dpu_pr_err("parse boot up profile failed\n");
		base_info->bootup_profile = 0;
	}
	dpu_pr_info("boot up profile is %d\n", base_info->bootup_profile);

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-lowpower-ctrl", &base_info->lowpower_ctrl);
	if (ret < 0) {
		dpu_pr_err("parse lowpower ctrl failed\n");
		base_info->lowpower_ctrl = 0x0;
	}
	dpu_pr_info("lowpower ctrl is 0x%x\n", base_info->lowpower_ctrl);

	parse_port_mask(port_mask, base_info);

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-ipi-pll-sel", &base_info->ipi_pll_sel);
	if (ret < 0) {
		dpu_pr_debug("parse dsi-panel-ipi-pll-sel failed, use phy pll\n");
		base_info->ipi_pll_sel = DSI_PIXEL_CLOCK_PHY_PLL;
	}
	dpu_pr_info("use ipi pll:%d\n", base_info->ipi_pll_sel);

	connector = get_connector(base_info->connector_id);
	dpu_check_and_return(!connector, -1,"connector is null\n");

	if (base_info->external_connector_id > 0) {
		bind_connector = get_connector(base_info->external_connector_id);
		dpu_check_and_return(!bind_connector, -1,"bind_connector is null\n");
		connector->bind_connector = bind_connector;
	}

	return 0;
}

static int dsi_panel_parse_timing(void *fdt, int32_t parent_offset, struct panel_drv_private *priv)
{
	struct panel_timing *timing;
	struct connector *connector;
	int32_t offset;
	int ret;

	connector = get_connector(priv->base.connector_id);
	dpu_check_and_return(!connector, -1,"connector is null\n");

	timing = &connector->timing;

	if (priv->base.dual_port) {
		timing->hdisplay = priv->base.xres / 2;
		timing->vdisplay = priv->base.yres;
	} else {
		timing->hdisplay = priv->base.xres;
		timing->vdisplay = priv->base.yres;
	}

	offset = dpu_get_subnode_offset(fdt, parent_offset, "dsi-display-timings");
	if (offset < 0) {
		dpu_pr_err("get invalid offset, path:dsi-display-timings\n");
		return -1;
	}
	offset = dpu_get_subnode_offset(fdt, offset, "default_index_00");
	if (offset < 0) {
		dpu_pr_err("get invalid offset, path:default_index_00\n");
		return -1;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-frame-rate", &timing->framerate);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-frame-rate!\n");
		return ret;
	}
	dpu_pr_info("default_index_00 frame rate: %d\n", timing->framerate);

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-h-front-porch",
		&timing->hfp);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-h-front-porch!\n");
		return ret;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-h-back-porch",
		&timing->hbp);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-h-back-porch!\n");
		return ret;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-h-pulse-width",
		&timing->hpw);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-h-pulse-width!\n");
		return ret;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-h-sync-skew",
		&timing->h_skew);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-h-sync-skew!\n");
		return ret;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-v-back-porch",
		&timing->vbp);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-v-back-porch!\n");
		return ret;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-v-front-porch",
		&timing->vfp);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-v-front-porch!\n");
		return ret;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-v-pulse-width",
		&timing->vpw);
	if (ret) {
		dpu_pr_err("failed to get dsi-panel-v-pulse-width!\n");
		return ret;
	}

	return 0;
}

#define CMD_SET_MIN_SIZE 7
static int dsi_panel_get_cmd_pkt_count(const char *cmd_data, u32 length,
		u32 *pkt_count)
{
	u32 count = 0;
	u32 packet_length;
	u32 tmp;

	while (length >= CMD_SET_MIN_SIZE) {
		packet_length = CMD_SET_MIN_SIZE;
		tmp = ((cmd_data[5] << 8) | (cmd_data[6]));
		packet_length += tmp;
		if (packet_length > length) {
			dpu_pr_err("command format error\n");
			return -1;
		}

		length -= packet_length;
		cmd_data += packet_length;
		count++;
	}

	*pkt_count = count;
	dpu_pr_debug("get cmd pkt count:%d\n", count);

	return 0;
}

static int dsi_panel_create_cmd_packets(const char *data, u32 length,
		u32 count, struct dsi_cmd_desc *cmd)
{
	int rc = 0;
	int i, j;
	u8 *payload;

	for (i = 0; i < count; i++) {
		u32 size;

		cmd[i].msg.type = data[0];
		cmd[i].msg.channel = data[2];
		cmd[i].msg.tx_len = ((data[5] << 8) | (data[6]));
		/* convert ms to us */
		cmd[i].post_wait_us = data[4] * 1000;

		size = cmd[i].msg.tx_len * sizeof(u8);
		payload = dpu_mem_alloc(size);
		if (!payload) {
			rc = -ENOMEM;
			goto error_free_payloads;
		}

		for (j = 0; j < cmd[i].msg.tx_len; j++)
			payload[j] = data[7 + j];

		cmd[i].msg.tx_buf = payload;
		data += (7 + cmd[i].msg.tx_len);
	}

	return rc;

error_free_payloads:
	for (i = i - 1; i >= 0; i--) {
		cmd--;
		dpu_mem_free(cmd->msg.tx_buf);
	}

	return rc;
}

static int dsi_panel_parse_cmd_set(void *fdt, int32_t offset,
		struct dsi_cmd_set *cmd_set, const char *cmd_name,
		const char *cmd_state_name)
{
	const char *cmd_state = NULL;
	int raw_data_size = 0;
	char *raw_data;
	uint32_t num_cmds = 0;
	int ret = 0;

	if (!cmd_set || !cmd_name) {
		dpu_pr_err("invalid params\n");
		return -1;
	}

	/* get cmd_set set source data */
	raw_data_size = dpu_dts_get_u8_array_size(fdt, offset, cmd_name);
	if (raw_data_size < 1) {
		dpu_pr_debug("empty cmd:%a\n", cmd_name);
		return -1;
	}
	dpu_pr_debug("get cmd data size:%d\n", raw_data_size);

	raw_data = dpu_mem_alloc(raw_data_size * sizeof(uint8_t));
	if (!raw_data)
		return -1;

	ret = dpu_dts_parse_u8_array(fdt, offset, cmd_name, (uint8_t *)raw_data);
	if (ret) {
		dpu_pr_err("failed to get xring pdpu-dsi-panel-on-command\n");
		goto error_release_raw_data;
	}

	ret = dsi_panel_get_cmd_pkt_count(raw_data, raw_data_size, &num_cmds);
	if (ret) {
		dpu_pr_err("failed to get command packet count!\n");
		goto error_release_raw_data;
	}
	dpu_pr_debug("cmd set %a contains %d cmds\n", cmd_name, num_cmds);

	cmd_set->cmds = dpu_mem_alloc(num_cmds * sizeof(*cmd_set->cmds));
	if (!cmd_set->cmds)
		goto error_release_raw_data;

	cmd_set->num_cmds = num_cmds;

	ret = dsi_panel_create_cmd_packets(raw_data, raw_data_size,
			num_cmds, cmd_set->cmds);
	if (ret) {
		dpu_pr_err("failed to create cmd packets\n");
		goto error_release_cmds;
	}

	dpu_mem_free(raw_data);

	/* parse cmd set state */
	ret = dpu_dts_parse_string(fdt, offset, cmd_state_name, (const char **)&cmd_state);
	if (ret) {
		dpu_pr_debug("set cmd: %a state as default lp mode\n", cmd_name);
		cmd_set->cmd_state = DSI_CMD_SET_STATE_LP;
	} else if (!dpu_str_cmp(cmd_state, "dsi_hs_mode")) {
		cmd_set->cmd_state = DSI_CMD_SET_STATE_HS;
	} else {
		cmd_set->cmd_state = DSI_CMD_SET_STATE_LP;
	}

	dpu_pr_debug("get cmd state:%a\n", cmd_state);
	return 0;

error_release_cmds:
	dpu_mem_free(cmd_set->cmds);
	cmd_set->cmds = NULL;
error_release_raw_data:
	dpu_mem_free(raw_data);
	return ret;
}

static int dsi_panel_parse_cmd_sets(void *fdt, int32_t parent_offset,
		struct panel_drv_private *priv)
{
	struct dsi_cmd_set *cmd_set;
	const char *cmd_state_name = NULL;
	const char *cmd_name = NULL;
	int32_t offset;
	uint32_t i;
	int ret = 0;

	if (priv->is_fake_panel)
		return 0;

	offset = dpu_get_subnode_offset(fdt, parent_offset, "dsi-display-timings");
	if (offset < 0) {
		dpu_pr_err("get invalid offset, path:dsi-display-timings\n");
		return -1;
	}
	offset = dpu_get_subnode_offset(fdt, offset, "default_index_00");
	if (offset < 0) {
		dpu_pr_err("get invalid offset, path:default_index_00\n");
		return -1;
	}

	for (i = 0; i < DSI_CMD_SET_MAX; i++) {
		cmd_state_name = cmd_set_state_names[i];
		cmd_name = cmd_set_names[i];
		cmd_set = &priv->cmd_sets[i];

		if (!cmd_name || !cmd_state_name) {
			dpu_pr_err("%d-th cmd/cmd state name is null\n", i);
			continue;
		}

		/* parse single cmd set */
		ret = dsi_panel_parse_cmd_set(fdt, offset, cmd_set, cmd_name, cmd_state_name);
		if (ret)
			dpu_pr_debug("empty cmd set:%a\n", cmd_name);

		cmd_set->type = i;
		dpu_pr_debug("parse %d-th cmd set [%a] success\n", i, cmd_name);
	}

	return 0;
}

static int dsi_panel_parse_phy_param(void *fdt, int32_t offset, struct panel_drv_private *priv)
{
	struct dpu_panel_info *pinfo;
	const char *phy_clk_type;
	const char *phy_type;
	uint32_t lane_num;
	int ret;

	pinfo = &priv->base;

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-pixel-clock", &pinfo->pixel_clk_rate);
	if (ret) {
		dpu_pr_err("parse dsi-panel-pixel-clock failed\n");
		return -1;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-lane-number", &lane_num);
	if (ret) {
		dpu_pr_err("parse dsi-panel-lane-number failed\n");
		pinfo->lane_num = DSI_PHY_4LANE;
	}

	switch (lane_num) {
	case 1:
		pinfo->lane_num = DSI_PHY_1LANE;
		break;
	case 2:
		pinfo->lane_num = DSI_PHY_2LANE;
		break;
	case 3:
		pinfo->lane_num = DSI_PHY_3LANE;
		break;
	default:
		pinfo->lane_num = DSI_PHY_4LANE;
		break;
	}
	dpu_pr_debug("parse dsi-panel-lane-number:%u\n", pinfo->lane_num + 1);

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-lane-rate", &pinfo->phy_lane_rate);
	if (ret) {
		dpu_pr_err("parse dsi-panel-lane-rate failed\n");
		return -1;
	}

	if (!pinfo->is_asic) {
		pinfo->sys_clk_rate = 20000000;
		pinfo->phy_lp_speed = 10000000;
	} else {
		pinfo->sys_clk_rate = 208900000;
		pinfo->phy_lp_speed = 17400000;
	}

	ret = dpu_dts_parse_string(fdt, offset, "dsi-panel-phy-type", &phy_type);
	if (ret < 0) {
		dpu_pr_err("parse dsi-panel-phy-type failed\n");
		return -1;
	}

	if (!dpu_str_cmp(phy_type, "dphy")) {
		pinfo->phy_type = DSI_PHY_TYPE_DPHY;
	} else if (!dpu_str_cmp(phy_type, "cphy")) {
		pinfo->phy_type = DSI_PHY_TYPE_CPHY;
	} else {
		dpu_pr_err("invalid phy type:%a\n", phy_type);
		pinfo->phy_type = DSI_PHY_TYPE_DPHY;
	}

	ret = dpu_dts_parse_string(fdt, offset, "dsi-panel-clk-type", &phy_clk_type);
	if (ret < 0) {
		dpu_pr_err("parse dsi-panel-clk-type failed\n");
		return -1;
	}

	if (!dpu_str_cmp(phy_clk_type, "continuous")) {
		pinfo->phy_clk_type = DSI_PHY_CONTINUOUS_CLK;
	} else if (!dpu_str_cmp(phy_clk_type, "non_continuous")) {
		pinfo->phy_clk_type = DSI_PHY_NON_CONTINUOUS_CLK;
	} else {
		dpu_pr_err("invalid phy clk type:%a\n", phy_clk_type);
		pinfo->phy_clk_type = DSI_PHY_NON_CONTINUOUS_CLK;
	}

	dpu_pr_info("phy type:%a, clk_type:%a\n", phy_type, phy_clk_type);
	dpu_pr_info("sys:%u, pclk:%u, hs:%u, lp:%u\n",
			pinfo->sys_clk_rate,  pinfo->pixel_clk_rate,
			pinfo->phy_lane_rate, pinfo->phy_lp_speed);

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-pixel-clock-div",
		&pinfo->pixel_clk_div);
	if (ret) {
		dpu_pr_debug("unsupported parm: dsi-panel-pixel-clock-div \n");
		pinfo->pixel_clk_div = 0;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-phy-amplitude",
		&pinfo->phy_amplitude);
	if (ret) {
		dpu_pr_debug("unsupported parm: dsi-panel-phy-amplitude\n");
		pinfo->phy_amplitude = PHY_AMPLITUDE_200MV;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-phy-oa-setr",
		&pinfo->phy_oa_setr);
	if (ret) {
		dpu_pr_debug("unsupported parm: dsi-panel-phy-oa-setr\n");
		pinfo->phy_oa_setr = 0;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-phy-eqa",
		&pinfo->phy_eqa);
	if (ret) {
		dpu_pr_debug("unsupported parm: dsi-panel-phy-eqa\n");
		pinfo->phy_eqa = 0;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-phy-eqb",
		&pinfo->phy_eqb);
	if (ret) {
		dpu_pr_debug("unsupported parm: dsi-panel-phy-eqb\n");
		pinfo->phy_eqb = 0;
	}

	return 0;
}

static int parse_power_seq_info(void *fdt, int32_t offset,
		struct panel_seq_ctrl *seq_ctrl, const char *seq_name)
{
	uint32_t seq_idx;
	uint32_t *data;
	int data_num;
	int ret;
	int i, j;

	data_num = dpu_dts_get_u32_array_size(fdt, offset, seq_name);
	if (data_num < PANEL_IO_SEQ_PATTERN_SIZE) {
		dpu_pr_info("invalid %a property\n", seq_name);
		return 0;
	}

	data = dpu_mem_alloc(data_num * sizeof(uint32_t));
	if (!data) {
		dpu_pr_err("alloc failed, num:%u\n", data_num);
		return -1;
	}

	ret = dpu_dts_parse_u32_array(fdt, offset, seq_name, data);
	if (ret < 0) {
		dpu_pr_info("parse %a property failed\n", seq_name);
		return -1;
	}

	seq_idx = 0;
	seq_ctrl->seq_info = dpu_mem_alloc(sizeof(struct io_seq_info));
	for (i = 0; i < data_num;) {
		struct io_seq_info *tmp_info = dpu_mem_realloc(sizeof(struct io_seq_info) * seq_idx,
				sizeof(struct io_seq_info) * (seq_idx + 1), seq_ctrl->seq_info);
		if (!tmp_info) {
			dpu_pr_err("alloc io_seq_info failed\n");
			goto error;
		}
		seq_ctrl->seq_info = tmp_info;
		seq_ctrl->seq_info[seq_idx].io_type = data[i++];
		if (seq_ctrl->seq_info[seq_idx].io_type == 0) {
			dpu_pr_err("invalid io_type:%u\n", seq_ctrl->seq_info[seq_idx].io_type);
			goto error;
		}
		seq_ctrl->seq_info[seq_idx].id = data[i++];
		seq_ctrl->seq_info[seq_idx].ops_num = data[i++];
		if (seq_ctrl->seq_info[seq_idx].ops_num > MAX_IO_OPS_SEQ_NUM) {
			dpu_pr_err("invalid ops_num:%u\n", seq_ctrl->seq_info[seq_idx].ops_num);
			goto error;
		}

		dpu_pr_debug("seq[%u] io_type:%u id:%u ops_num:%u\n", seq_idx,
				seq_ctrl->seq_info[seq_idx].io_type,
				seq_ctrl->seq_info[seq_idx].id,
				seq_ctrl->seq_info[seq_idx].ops_num);

		for (j = 0; j < seq_ctrl->seq_info[seq_idx].ops_num; j++) {
			seq_ctrl->seq_info[seq_idx].seq[j].level = data[i++];
			seq_ctrl->seq_info[seq_idx].seq[j].delay = data[i++];
			dpu_pr_debug("seq[%u] level:%u delay:%u\n", seq_idx,
					seq_ctrl->seq_info[seq_idx].seq[j].level,
					seq_ctrl->seq_info[seq_idx].seq[j].delay);
		}

		seq_idx++;
	}

	seq_ctrl->seq_num = seq_idx + 1;
	dpu_mem_free(data);
	return 0;
error:
	dpu_mem_free(data);
	dpu_mem_free(seq_ctrl->seq_info);
	return -1;
}

static int dsi_panel_seq_info_parse(void *fdt, int32_t offset, struct panel_drv_private *priv)
{
	struct panel_seq_ctrl *power_on_seq_ctrl = &priv->power_on_seq_ctrl;
	struct panel_seq_ctrl *power_off_seq_ctrl = &priv->power_off_seq_ctrl;
	struct panel_seq_ctrl *reset_on_seq_ctrl = &priv->reset_on_seq_ctrl;
	struct panel_seq_ctrl *reset_off_seq_ctrl = &priv->reset_off_seq_ctrl;
	int ret;

	if (priv->is_fake_panel)
		return 0;

	ret = parse_power_seq_info(fdt, offset, power_on_seq_ctrl, "dsi-panel-power-on-seq");
	if (ret) {
		dpu_pr_err("parse dsi-panel-power-on-seq failed\n");
		return -1;
	}

	ret = parse_power_seq_info(fdt, offset, power_off_seq_ctrl, "dsi-panel-power-off-seq");
	if (ret) {
		dpu_pr_info("parse dsi-panel-power-off-seq failed\n");
		power_off_seq_ctrl->seq_num = 0;
	}

	ret = parse_power_seq_info(fdt, offset, reset_on_seq_ctrl, "dsi-panel-reset-on-seq");
	if (ret) {
		dpu_pr_info("parse dsi-panel-reset-on-seq failed\n");
		reset_on_seq_ctrl->seq_num = 0;
	}

	ret = parse_power_seq_info(fdt, offset, reset_off_seq_ctrl, "dsi-panel-reset-off-seq");
	if (ret) {
		dpu_pr_info("parse dsi-panel-reset-off-seq failed\n");
		reset_off_seq_ctrl->seq_num = 0;
	}

	priv->reset_after_lp11 = dpu_dts_read_bool(fdt, offset, "dsi-panel-reset-after-lp11");
	dpu_pr_info("get reset_after_lp11:%d\n", priv->reset_after_lp11);
	return 0;
}

static void dsi_panel_parse_build_id_config(void *fdt, int32_t offset,
		struct panel_drv_private *priv)
{
	struct panel_build_id_config *id_config;
	const char *cmd_name = "dsi-panel-read-build-id-command";
	int ret = 0;

	id_config = &priv->id_config;
	id_config->id_cmds_rlen = 0;
	if (priv->is_fake_panel)
		return;

	ret = dsi_panel_parse_cmd_set(fdt, offset, &id_config->id_cmd, cmd_name,
			"dsi-panel-read-build-id-command-state");
	if (ret) {
		dpu_pr_debug("empty cmd set:%a\n", cmd_name);
		id_config->id_cmd.num_cmds = 0;
		id_config->id_cmd.cmds = NULL;
		return;
	}

	id_config->id_cmd.type = DSI_CMD_SET_READ_BUILD_ID;
	ret = dpu_dts_parse_u32(fdt, offset, "dsi-panel-read-build-id-length",
			&id_config->id_cmds_rlen);
	if (ret)
		dpu_pr_err("failed to get build id length\n");
}
static void dsi_panel_parse_xeq_config(void *fdt, int32_t offset, struct panel_drv_private *priv)
{
	if (priv->is_fake_panel)
		return;
	priv->panel_xeq_enabled = dpu_dts_read_bool(fdt, offset, "mi,dsi-panel-xeq-enabled");
	return;
}
int dsi_panel_info_parse(struct panel_drv_private *priv,
		const char *panel_node_name)
{
	int32_t offset;
	void *fdt;
	int ret;

	fdt = dpu_get_fdt();
	if (!fdt) {
		dpu_pr_err("dpu_get_fdt fail\n");
		return -1;
	}

	offset = dsi_panel_offset_get(fdt, panel_node_name);
	if (offset < 0) {
		dpu_pr_err("get invalid panel node offset, name:%a\n", panel_node_name);
		return -1;
	}

	ret = dsi_panel_common_parse(fdt, offset, priv);
	if (ret) {
		dpu_pr_err("dsi_panel_common_parse failed\n");
		return -1;
	}

	if (!priv->is_fake_panel) {
		ret = dsi_panel_seq_info_parse(fdt, offset, priv);
		if (ret) {
			dpu_pr_err("dsi_panel_seq_info_parse failed\n");
			return -1;
		}
	} else {
		dpu_pr_info("fake panel no need gpio\n");
	}

	ret = dsi_panel_parse_timing(fdt, offset, priv);
	if (ret) {
		dpu_pr_err("dsi_panel_parse_timing failed\n");
		return -1;
	}

	ret = dsi_panel_parse_cmd_sets(fdt, offset, priv);
	if (ret) {
		dpu_pr_err("dsi_panel_parse_cmd_sets failed\n");
		return -1;
	}

	ret = dsi_panel_parse_phy_param(fdt, offset, priv);
	if (ret) {
		dpu_pr_err("dsi_panel_parse_phy_param failed\n");
		return -1;
	}

	ret = parse_backlight_info(fdt, offset, &priv->base);
	if (ret) {
		dpu_pr_err("parse_backlight_info failed\n");
		return -1;
	}

	dsi_panel_parse_build_id_config(fdt, offset, priv);


	dsi_panel_parse_xeq_config(fdt, offset, priv);

	priv->base.bta_en = dpu_dts_read_bool(fdt, offset,
			"dsi-panel-bta-enable");

	priv->base.eotp_en = dpu_dts_read_bool(fdt, offset,
			"dsi-panel-eotp-enable");

	dpu_pr_info("bta_en: %d, eotp_en: %d\n",
			priv->base.bta_en, priv->base.eotp_en);
	return 0;
}

static void dsi_panel_cmd_release(struct dsi_cmd_desc *cmd)
{
	if (cmd == NULL)
		return;

	if (cmd->msg.rx_buf)
		dpu_mem_free(cmd->msg.rx_buf);

	if (cmd->msg.tx_buf)
		dpu_mem_free(cmd->msg.tx_buf);
}

static void dsi_panel_cmd_set_release(struct dsi_cmd_set *cmd_set)
{
	uint32_t i;

	dpu_pr_debug("+\n");
	if (cmd_set->num_cmds == 0)
		return;

	for (i = 0; i < cmd_set->num_cmds; ++i)
		dsi_panel_cmd_release(&cmd_set->cmds[i]);

	dpu_mem_free(cmd_set->cmds);
	dpu_pr_debug("-\n");
}

static void dsi_panel_cmd_sets_release(struct panel_drv_private *priv)
{
	struct dsi_cmd_set *cmd_set;
	uint32_t i;

	dpu_pr_debug("+\n");
	if (priv->is_fake_panel)
		return;

	for (i = 0; i < DSI_CMD_SET_MAX; i++) {
		cmd_set = &priv->cmd_sets[i];
		dsi_panel_cmd_set_release(cmd_set);
	}
	dpu_pr_debug("-\n");
}

static void dsi_panel_dsc_params_release(struct dpu_panel_info *base_info)
{
	dpu_pr_debug("+\n");
	if (base_info->dsc_cfg.customized_pps_table)
		dpu_mem_free(base_info->dsc_cfg.customized_pps_table);
	dpu_pr_debug("-\n");
}

static inline void dsi_panel_single_power_seq_release(struct panel_seq_ctrl *power_seq)
{
	if (power_seq->seq_info)
		dpu_mem_free(power_seq->seq_info);
}
static void dsi_panel_power_seq_release(struct panel_drv_private *priv)
{
	dpu_pr_debug("+\n");
	dsi_panel_single_power_seq_release(&priv->power_on_seq_ctrl);
	dsi_panel_single_power_seq_release(&priv->power_off_seq_ctrl);
	dsi_panel_single_power_seq_release(&priv->reset_on_seq_ctrl);
	dsi_panel_single_power_seq_release(&priv->reset_off_seq_ctrl);
	dpu_pr_debug("-\n");
}

static void dsi_panel_build_id_config_release(struct panel_drv_private *priv)
{
	dpu_pr_debug("+\n");
	if (priv->is_fake_panel)
		return;

	dsi_panel_cmd_set_release(&priv->id_config.id_cmd);
	dpu_pr_debug("-\n");
}

void dsi_panel_info_release(struct panel_drv_private *priv)
{
	dpu_pr_debug("+\n");
	dsi_panel_build_id_config_release(priv);
	dsi_panel_cmd_sets_release(priv);
	dsi_panel_dsc_params_release(&priv->base);
	dsi_panel_power_seq_release(priv);
	dpu_pr_debug("-\n");
}
