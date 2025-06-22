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

#include <ApbGpio.h>
#include <Protocol/FdtProtocol.h>
#include <libfdt.h>
#include <Library/MiscIdLib.h>

#include "panel_mgr.h"
#include "dpu_log.h"
#include "panel.h"

#define MAX_LCD_ID_SIZE 32
#define MAX_LCD_NAME_SIZE 256
#define READ_BACK_BUFFER_SIZE 256
#define MAX_RETRY_TIMES 5

enum LCD_ID_VALUE {
	PULL_DOWN_0=0,
	PULL_UP_1,
	FLOAT_2,
	ERROR_VALUE,
};

static const char *get_lcd_info_by_name(void *fdt, int32_t offset)
{
	const char *lcd_name;
	int32_t ret;

	ret = dpu_dts_parse_string(fdt, offset, "lcd_name", &lcd_name);
	if (ret < 0 || !lcd_name)
		goto error;

	dpu_pr_debug("get lcd name: %a\n", lcd_name);
	return lcd_name;
error:
	dpu_pr_err("get lcd name failed, return fake_dsi_panel\n");
	return "fake_dsi_panel";
}

static int get_lcd_id_from_dts(void *fdt, int32_t offset)
{
	const char *prop_name = "lcd_id";
	uint32_t lcd_id = 0;
	int ret;

	ret = dpu_dts_parse_u32(fdt, offset, prop_name, &lcd_id);
	if (ret < 0) {
		dpu_pr_err("failed to get lcd_id from dtsi, return -1\n");
		/* default invalid panel id */
		return -1;
	}

	dpu_pr_debug("get lcd_id from dtsi: %d\n", lcd_id);
	return lcd_id;
}

static uint32_t get_pin_state(uint32_t lcd_gpio_id)
{
	uint32_t pull_up_value = FLOAT_2;
	uint32_t pull_down_value = FLOAT_2;
	int ret;

	ret = gpio_ioc_prepare(lcd_gpio_id, PINCTRL_FUNC0_GPIO,
			SINGLE_FUNC_IOC_PULL_UP);
	if (ret) {
		dpu_pr_err("failed to prepare for lcd gpio\n");
		goto error;
	}
	gpio_get_direction_input_value(lcd_gpio_id, (int *)&pull_up_value);

	ret = gpio_ioc_prepare(lcd_gpio_id, PINCTRL_FUNC0_GPIO,
			SINGLE_FUNC_IOC_PULL_DOWN);
	if (ret) {
		dpu_pr_err("failed to prepare for lcd gpio\n");
		goto error;
	}
	gpio_get_direction_input_value(lcd_gpio_id, (int *)&pull_down_value);
	ret = gpio_ioc_prepare(lcd_gpio_id, PINCTRL_FUNC0_GPIO,
			SINGLE_FUNC_IOC_NO_PULL);
	if (ret) {
		dpu_pr_err("failed to prepare for lcd gpio\n");
		goto error;
	}
	dpu_pr_info("get gpio_id & pull_up_value & pull_down_value: %d & %d & %d\n",
			lcd_gpio_id, pull_up_value, pull_down_value);
	if ((pull_up_value == pull_down_value) && (pull_up_value == 0)) {
		return PULL_DOWN_0;
	} else if ((pull_up_value == pull_down_value) && (pull_up_value == 1)) {
		return PULL_UP_1;
	} else {
		return FLOAT_2;
	}

error:
	dpu_pr_err("get lcd_gpio_id failed, return error\n");
	return ERROR_VALUE;
}

static const char *get_lcd_info_by_gpio(void *fdt, int32_t offset)
{
	uint32_t lcd_gpio_id, vddio_id;
	int lcd_id, target_lcd_id;
	int ret;

	ret = dpu_dts_parse_u32(fdt, offset, "lcd_vddio_id", &vddio_id);
	if (ret) {
		dpu_pr_err("failed to get lcd_vddio_id, ret = %d\n", ret);
		goto error;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "lcd_gpio_id", &lcd_gpio_id);
	if (ret) {
		dpu_pr_err("failed to get lcd_gpio_id, ret = %d\n", ret);
		goto error;
	}

	ret = ldo_set_direction_output_value(vddio_id, PULL_UP,
			PANEL_IO_DELAY_2MS);
	if (ret) {
		dpu_pr_err("failed to enable vddio\n");
		goto error;
	}
	lcd_id = get_pin_state(lcd_gpio_id);
	ldo_set_direction_output_value(vddio_id, PULL_DOWN, 0);
	gpio_ioc_prepare(lcd_gpio_id, PINCTRL_FUNC0_GPIO,
			SINGLE_FUNC_IOC_PULL_DOWN);

	/* TODO: if support multi panel on same boardid,
	* should loop read lcd id and check right panel
	*/

	target_lcd_id = get_lcd_id_from_dts(fdt, offset);
	dpu_pr_info("get gpio_id & lcd_id & target_id: %d & %d & %d\n",
			lcd_gpio_id, lcd_id, target_lcd_id);
	if (lcd_id == target_lcd_id) {
		return get_lcd_info_by_name(fdt, offset);
	} else {
		dpu_pr_info("unmatched lcd_id: %d, target_lcd_id: %d, "
				"return fake_dsi_panel\n",
				lcd_id, target_lcd_id);
		return "fake_dsi_panel";
	}

error:
	dpu_pr_err("get lcd_gpio_id failed, return fake_dsi_panel\n");
	return "fake_dsi_panel";
}

extern int dsi_panel_cmd_read(struct panel_drv_private *priv,
		struct dsi_cmd_desc *cmd, u8 *rx_buf, u32 rx_len);

static int32_t panel_read_build_id(enum PANEL_ID panel_id, uint32_t *build_id)
{
	struct panel_build_id_config *id_config;
	struct panel_drv_private *priv;
	struct panel_dev *panel_dev;
	struct dsi_cmd_set *cmd_set;
	int ret = 0;

	*build_id = PANEL_BUILD_MAX;

	panel_dev = get_panel_dev(panel_id);
	dpu_check_and_return(!panel_dev, -1, "failed to get panel dev\n");
	priv = to_panel_priv(platform_get_drvdata(panel_dev->pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");

	id_config = &priv->id_config;
	cmd_set = &id_config->id_cmd;
	if (!id_config->id_cmds_rlen || !cmd_set->cmds || !cmd_set->num_cmds) {
		dpu_pr_err("invliad id config\n");
		return -1;
	}

	ret = dsi_panel_cmd_read(priv, &cmd_set->cmds[0],
			&id_config->build_id, id_config->id_cmds_rlen);
	if (ret) {
		dpu_pr_err("failed to read panel build id\n");
		return -1;
	}

	*build_id = (uint32_t)id_config->build_id;
	return 0;
}

static int32_t get_panel_build_id_info(void *fdt, int32_t offset,
		const char **build_id_name,uint32_t *build_id,
		uint32_t *build_id_size)
{
	int32_t id_size, id_name_size, ret;

	*build_id_size = 0;
	id_size = dpu_dts_get_u32_array_size(fdt, offset, "build_id");
	id_name_size = dpu_dts_get_string_array_size(fdt, offset, "build_id_name");
	if (id_size <= 0 || id_name_size <= 0 ||
			id_size != id_name_size ||
			id_name_size > MAX_LCD_ID_SIZE) {
		dpu_pr_err("build_id or build_id_name is invalid %d, %d\n",
				id_size, id_name_size);
		return -1;
	}

	ret = dpu_dts_parse_u32_array(fdt, offset, "build_id", build_id);
	if (ret < 0) {
		dpu_pr_info("parse build_id array property failed\n");
		return -1;
	}

	ret = dpu_dts_parse_string_array(fdt, offset, "build_id_name", build_id_name, id_name_size);
	if (ret < 0 ) {
		dpu_pr_err("failed to get build_id_name array, ret=%d\n", ret);
		return -1;
	}

	*build_id_size = id_size;
	return 0;
}

static int g_select_scene_id = SCENE_ID_0;

int32_t dpu_get_scene_id_from_lcd(void)
{
	return g_select_scene_id;
}

static int32_t dsi_minimal_enable(void)
{
	static struct comp_mgr *comp_mgr;
	struct dpu_power_mgr pwr_mgr;
	uint32_t scene_id, conn_id;
	struct connector *connector;
	int ret = 0;

	scene_id = dpu_get_scene_id_from_lcd();
	conn_id = get_conn_id_from_scene_id(scene_id);
	dpu_pr_debug("scene id:%u, conn id:%u +\n", scene_id, conn_id);

	connector = get_connector(conn_id);
	conn_mgr_prepare(scene_id);

	/* dsi power on */
	pwr_mgr.pinfo = connector->pinfo;
	pwr_mgr.profile_id = 0;
	pwr_mgr.volt_level = 0;
	if (is_dsi_connector(scene_id)) {
		ret = dsi_power_on(&pwr_mgr);
		dpu_check_and_return(ret != 0, -1, "dsi power on fail\n");
	}

	/* panel power on & dsi init */
	comp_mgr = comp_mgr_get();
	pipeline_next_ops_handle(comp_mgr->pdev, "minimal_init", connector->id, NULL);

	dpu_pr_debug("scene id:%u, conn id:%u -\n", scene_id, conn_id);
	return 0;
}

static void dsi_minimal_disable(void)
{
	uint32_t scene_id, conn_id;
	struct connector *connector;
	static struct comp_mgr *comp_mgr;

	scene_id = dpu_get_scene_id_from_lcd();
	conn_id = get_conn_id_from_scene_id(scene_id);
	dpu_pr_debug("scene id:%u, conn id:%u +\n", scene_id, conn_id);

	connector = get_connector(conn_id);
	comp_mgr = comp_mgr_get();
	pipeline_next_ops_handle(comp_mgr->pdev, "minimal_deinit",
			connector->id, (void *)&scene_id);

	conn_mgr_unprepare(scene_id);
	dpu_pr_debug("scene id:%u, conn id:%u -\n", scene_id, conn_id);
}

static const char *get_lcd_info_by_build_id(void *fdt, int32_t offset)
{
	const char *build_id_name[MAX_LCD_ID_SIZE], *sel_lcd_info = "fake_dsi_panel";
	uint32_t target_build_id, build_id_size = 0;
	uint32_t build_id[MAX_LCD_ID_SIZE];
	const char *base_panel_name;
	bool matched = false;
	int ret, i, default_index;

	/* get base panel name */
	base_panel_name = get_lcd_info_by_name(fdt, offset);
	if (!dpu_str_cmp(base_panel_name, "fake_dsi_panel")) {
		dpu_pr_err("get invalid base base_panel_name:%a", base_panel_name);
		return "fake_dsi_panel";
	}

	/* get base config and create toplogic of dsi */
	ret = register_panel(PANEL_PRIMARY, base_panel_name);
	if (ret) {
		dpu_pr_err("register panel:%a failed\n", base_panel_name);
		return "fake_dsi_panel";
	}

	/* init dsi and panel by base config */
	ret = dsi_minimal_enable();
	if (ret) {
		dpu_pr_err("failed to minimal init dsi\n");
		sel_lcd_info = "fake_dsi_panel";
		goto error_with_unregister_panel;
	}

	/* parse build id table */
	ret = get_panel_build_id_info(fdt, offset, build_id_name,
			build_id, &build_id_size);
	if (ret) {
		dpu_pr_err("failed to get panel build id info\n");
		sel_lcd_info = "fake_dsi_panel";
		goto error_with_deinit;
	}

	/* read panel build id */
	for (i = 0; i < MAX_RETRY_TIMES; ++i) {
		ret = panel_read_build_id(PANEL_PRIMARY, &target_build_id);
		if (ret) {
			dpu_pr_err("failed to read build id in %d times\n", i);
			sel_lcd_info = "fake_dsi_panel";
			target_build_id = PANEL_BUILD_MAX;
		} else {
			dpu_pr_warn("read build id:0x%x\n", target_build_id);
			break;
		}
	}

	/* select panel from build id table */
	for (i = 0; i < build_id_size; ++i) {
		dpu_pr_debug("build_id[%d]&target_build_id&build_id_name[%d]:0x%x&0x%x&%a\n",
				i, i, build_id[i], target_build_id, build_id_name[i]);
		if (build_id[i] == target_build_id) {
			dpu_pr_warn("get build_id[%d]&target_build_id&build_id_name[%d]:0x%x&0x%x&%a\n",
					i, i, build_id[i], target_build_id, build_id_name[i]);
			sel_lcd_info = build_id_name[i];
			matched = true;
		}
	}

	default_index = 0;
	if (!matched && (target_build_id != PANEL_BUILD_MAX)) {
		for (i = 0; i < build_id_size; ++i) {
			if (build_id[i] > build_id[default_index])
				default_index = i;
		}
		sel_lcd_info = build_id_name[default_index];
		dpu_pr_err("failed to get matched panel, set %a as default\n", sel_lcd_info);
	}

error_with_deinit:
	dsi_minimal_disable();
error_with_unregister_panel:
	unregister_panel(PANEL_PRIMARY);
	return sel_lcd_info;
}

typedef const char *(*detect_func)(void *fdt, int32_t offset);
static detect_func g_detect_func[] = {
	get_lcd_info_by_name,
	get_lcd_info_by_gpio,
	get_lcd_info_by_build_id,
};

static const char *dpu_get_lcd_name_by_detect_type(void *fdt, int32_t offset)
{
	uint32_t detect_type;
	int ret;

	ret = dpu_dts_parse_u32(fdt, offset, "lcd_detect_type", &detect_type);
	if (ret) {
		dpu_pr_err("parse lcd_detect_type failed, "
				"default detect by name\n");
		return g_detect_func[0](fdt, offset);
	}

	dpu_pr_info("get detect_type: %d\n", detect_type);
	if (detect_type < ARRAY_SIZE(g_detect_func))
		return g_detect_func[detect_type](fdt, offset);

	dpu_pr_warn("failed to get lcd_detect_type, default detect lcd by name\n");
	return g_detect_func[0](fdt, offset);
}

static void dpu_update_lcd_name_by_hw_id(void *fdt, int32_t offset, const char **lcd_name)
{
	int32_t hw_id_size, hw_id_name_size;
	EFI_STATUS Status;
	UINT32 BoardId;
	const char *hw_id_name[MAX_LCD_ID_SIZE];
	uint32_t hw_id[MAX_LCD_ID_SIZE];
	int32_t ret, i;

	/* Bypass update lcd_name if hw_id/hw_id name is not configured */
	if (!dpu_dts_read_bool(fdt, offset, "hw_id") ||
			!dpu_dts_read_bool(fdt, offset, "hw_id_name")) {
		dpu_pr_debug("hw_id or hw_id_name is not configered\n");
		return;
	}

	/* Check that the hw_id size matches the hw_id_name size. */
	hw_id_size = dpu_dts_get_u32_array_size(fdt, offset, "hw_id");
	hw_id_name_size = dpu_dts_get_string_array_size(fdt, offset, "hw_id_name");
	if (hw_id_size <= 0 || hw_id_name_size <= 0 ||
			hw_id_size != hw_id_name_size ||
			hw_id_name_size > MAX_LCD_ID_SIZE) {
		dpu_pr_err("hw_id or hw_id_name is invalid %d, %d\n",
				hw_id_size, hw_id_name_size);
		return;
	}

	dpu_pr_debug("get hw_id_size %d, hw_id_name_size %d\n", hw_id_size, hw_id_name_size);

  	Status = GetBoardId(&BoardId);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("failed to get board id, ret=%d\n", Status);
		return;
	}

	ret = dpu_dts_parse_string_array(fdt, offset, "hw_id_name", hw_id_name, hw_id_name_size);
	if (ret < 0 ) {
		dpu_pr_err("failed to get hw_id_name array, ret=%d\n", ret);
		return;
	}

	ret = dpu_dts_parse_u32_array(fdt, offset, "hw_id", hw_id);
	if (ret < 0) {
		dpu_pr_info("parse hw_id array property failed\n");
		return;
	}

	/* Identify panel by product sub lot id */
	for (i = 0; i < hw_id_size; ++i) {
		dpu_pr_debug("BoardId:0x%x, hw_id[%d]:0x%x\n", BoardId, i, hw_id[i]);
		if ((BoardId & 0xFF) == hw_id[i]) {
			*lcd_name = hw_id_name[i];
			dpu_pr_info("get compatiable panel:%a by hw_id:%u\n",
					hw_id_name[i], hw_id[i]);
		}
	}
}

int32_t dpu_get_lcd_type(void)
{
	struct panel_drv_private *priv;
	uint32_t scene_id = SCENE_ID_0;
	void *fdt;
	int32_t offset;
	struct panel_dev *panel;
	int32_t ret;
	const char *lcd_name;
	char temp[MAX_LCD_NAME_SIZE];

	fdt = dpu_get_fdt();
	if (!fdt) {
		goto err;
	}

	offset = dpu_get_fdt_offset(fdt, DSI_DTS_PATH);
	if (offset < 0) {
		goto err;
	}
	ret = dpu_dts_parse_u32(fdt, offset, "scene_id", &scene_id);
	if (ret) {
		dpu_pr_warn("parse scene id: %u failed\n", scene_id);
	}
	dpu_pr_info("get scene_id: %d\n", scene_id);
	g_select_scene_id = scene_id;

	/* detect panel is connected */
	lcd_name = dpu_get_lcd_name_by_detect_type(fdt, offset);
	dpu_pr_warn("panel %a selected\n", lcd_name);

	/* if panel is connected, identify panel by hw id */
	if (dpu_str_cmp(lcd_name, "fake_dsi_panel"))
		dpu_update_lcd_name_by_hw_id(fdt, offset, &lcd_name);

	ret = register_panel(PANEL_PRIMARY, lcd_name);
	if (ret) {
		dpu_pr_err("register panel: %a failed\n", lcd_name);
		goto err_with_update_sel_lcd_name;
	}
	/* update */
	panel = get_panel_dev(PANEL_PRIMARY);
	dpu_check_and_return(!panel, -1, "failed to get panel dev\n");
	priv = to_panel_priv(platform_get_drvdata(panel->pdev));

	dpu_str_cpy(temp, lcd_name, MAX_LCD_NAME_SIZE);
	dpu_dts_update_string_prop(fdt, offset, "sel_lcd_name", temp);

#ifdef DPU_FACTORY_DEBUG_WITH_PANEL
	ret = register_panel(PANEL_OFFLINE, "panel_offline");
	if (ret)
		dpu_pr_err("register panel: panel_offline failed\n");
#endif
	return 0;
err_with_update_sel_lcd_name:
	dpu_dts_update_string_prop(fdt, offset, "sel_lcd_name", "fake_dsi_panel");
err:
	ret = register_panel(PANEL_PRIMARY, "fake_dsi_panel");
	if (ret)
		dpu_pr_err("register panel_fake failed\n");

	return ret;
}

int32_t dpu_get_fake_lcd_type(void)
{
	int32_t ret;

	ret = register_panel(PANEL_PRIMARY, "fake_dsi_panel");
	if (ret)
		dpu_pr_err("register panel_fake failed\n");

	ret = register_panel(PANEL_OFFLINE, "panel_offline");
	if (ret)
		dpu_pr_err("register panel: panel_offline failed\n");

	return ret;
}

