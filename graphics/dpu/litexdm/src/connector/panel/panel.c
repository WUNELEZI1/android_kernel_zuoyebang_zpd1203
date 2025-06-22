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

#include "platform_device.h"
#include "dpu_pipeline.h"
#include "panel_mgr.h"
#include "dpu_log.h"
#include "panel.h"
#include "dsi_hw_ctrl_ops.h"
#include "dsi_hw_ctrl.h"
#include "panel_backlight_ktz8866.h"
#include "panel_aw37504.h"
#include "mipi_dsi_dev.h"

static int panel_pinctrl_work_state_set(struct panel_seq_ctrl *seq_ctrl)
{
	int ret, i;
	uint32_t gpio_id;

	for (i = 0; i < seq_ctrl->seq_num; i++) {
		if (GPIO_TYPE != seq_ctrl->seq_info[i].io_type)
			continue;

		gpio_id = seq_ctrl->seq_info[i].id;
		ret = gpio_ioc_prepare(gpio_id, PINCTRL_FUNC0_GPIO, SINGLE_FUNC_IOC_NO_PULL);
		if (ret) {
			dpu_pr_err("failed to set gpio %d config as work state\n",
					gpio_id);
			return -1;
		}

		gpio_set_direction_output_value(gpio_id, 0, PANEL_IO_DELAY_1MS);
		dpu_pr_debug("set gpio %d as work state success\n", gpio_id);
	}

	return 0;
}

static void panel_pinctrl_sleep_state_set(struct panel_seq_ctrl *seq_ctrl)
{
	int i;
	uint32_t gpio_id;

	for (i = 0; i < seq_ctrl->seq_num; i++) {
		if (GPIO_TYPE != seq_ctrl->seq_info[i].io_type)
			continue;

		gpio_id = seq_ctrl->seq_info[i].id;
		gpio_pull_status_set(gpio_id, SINGLE_FUNC_IOC_PULL_DOWN);
		gpio_set_direction_input_value(gpio_id, 0);
		dpu_pr_info("set gpio %d as sleep state success\n", gpio_id);
	}

	return;
}

static int panel_pinctrl_state_set(struct panel_drv_private *priv,
		bool is_work_state)
{
	struct panel_seq_ctrl *power_on_seq_ctrl;
	struct panel_seq_ctrl *power_off_seq_ctrl;
	struct panel_seq_ctrl *reset_on_seq_ctrl;
	struct panel_seq_ctrl *reset_off_seq_ctrl;
	int ret;

	dpu_pr_debug("+\n");
	dpu_check_and_return(!priv, -1, "priv is null\n");

	if (!priv->base.is_asic) {
		dpu_pr_debug("bypass the IOC operation in FPGA\n");
		return 0;
	}

	power_on_seq_ctrl = &priv->power_on_seq_ctrl;
	power_off_seq_ctrl = &priv->power_off_seq_ctrl;
	reset_on_seq_ctrl = &priv->reset_on_seq_ctrl;
	reset_off_seq_ctrl = &priv->reset_off_seq_ctrl;

	if (is_work_state) {
		ret = panel_pinctrl_work_state_set(reset_on_seq_ctrl);
		if (ret) {
			dpu_pr_err("set te reset_on_seq_ctrl work error\n");
			return -1;
		}

		ret = panel_pinctrl_work_state_set(power_on_seq_ctrl);
		if (ret) {
			dpu_pr_err("set te power_on_seq_ctrl work error\n");
			return -1;
		}

		ret = gpio_func_set(priv->lcd_te, PINCTRL_FUNC1_TE);
		if (ret) {
			dpu_pr_err("set te gpio %d as work error\n",
					priv->lcd_te);
			return -1;
		}
		dpu_pr_debug("set all ioc as work state success\n");
	} else {
		panel_pinctrl_sleep_state_set(reset_off_seq_ctrl);
		panel_pinctrl_sleep_state_set(power_off_seq_ctrl);

		ret = gpio_ioc_prepare(priv->lcd_te,
				PINCTRL_FUNC0_GPIO, SINGLE_FUNC_IOC_PULL_DOWN);
		if (ret)
			dpu_pr_err("set te gpio %d as sleep state error\n",
					priv->lcd_te);

		gpio_set_direction_input_value(priv->lcd_te, 0);
		dpu_pr_debug("set all ioc as sleep state\n");
	}

	return 0;
}

extern int dsi_panel_cmd_set_send(struct panel_drv_private *priv, enum dsi_cmd_set_type type);

static void send_power_sequence(struct panel_seq_ctrl *power_seq_ctrl)
{
	struct io_seq_info *seq_info;
	int i, j;

	if(!power_seq_ctrl) {
		dpu_pr_info("the current has no reset gpio \n");
		return;
	}

	for (i = 0; i < power_seq_ctrl->seq_num; i++) {
		seq_info = &power_seq_ctrl->seq_info[i];
		if (seq_info->io_type == GPIO_TYPE) {
			for (j = 0; j < seq_info->ops_num; j++)
				gpio_set_direction_output_value(seq_info->id,
						seq_info->seq[j].level, seq_info->seq[j].delay);
		} else if (seq_info->io_type == LDO_TYPE) {
			for (j = 0; j < seq_info->ops_num; j++)
				ldo_set_direction_output_value(seq_info->id,
						seq_info->seq[j].level, seq_info->seq[j].delay);
		}
	}
}

static int32_t panel_power_on(struct platform_device *pdev)
{
	struct panel_drv_private *priv;
	int ret = 0;

	dpu_pr_debug("+\n");
	priv = to_panel_priv(platform_get_drvdata(pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");

	if (priv->base.is_asic) {
		ret = panel_pinctrl_state_set(priv, true);
		if (ret) {
			dpu_pr_err("failed to set io state as work\n");
			return ret;
		}
	} else {
		dpu_pr_info("bypass the panel_pinctrl_state_set for FPGA\n");
	}

	send_power_sequence(&priv->power_on_seq_ctrl);

	if (BACKLIGHT_SET_BY_I2C == priv->base.backlight_setting_type) {
		dpu_mdelay(1);
		if(!dpu_str_cmp(priv->base.bias_ic_name, "AW37504"))
			aw37504_config();
		ktz8866_power_on();
	}

	if (!priv->reset_after_lp11) {
		dpu_pr_info("reset before lp11\n");
		send_power_sequence(&priv->reset_on_seq_ctrl);
	}
	dpu_pr_debug("-\n");
	dpu_pr_info("panel power on\n");
	return ret;
}

static int32_t panel_reset_flush(struct platform_device *pdev, uint32_t id, void *value)
{
	struct panel_drv_private *priv;
	int ret = 0;

	dpu_pr_debug("+\n");
	priv = to_panel_priv(platform_get_drvdata(pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");

	send_power_sequence(&priv->reset_on_seq_ctrl);

	dpu_pr_debug("-\n");
	return ret;
}

static int32_t panel_power_off(struct platform_device *pdev)
{
	struct panel_drv_private *priv;

	dpu_pr_debug("+\n");
	priv = to_panel_priv(platform_get_drvdata(pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");

	send_power_sequence(&priv->reset_off_seq_ctrl);

	send_power_sequence(&priv->power_off_seq_ctrl);

	if (priv->base.is_asic)
		panel_pinctrl_state_set(priv, false);
	else
		dpu_pr_info("bypass the panel_pinctrl_state_set for FPGA\n");
	dpu_pr_debug("-\n");
	dpu_pr_info("panel power off\n");
	return 0;
}

static void dsi_panel_backlight_cmd_set_send(struct panel_drv_private *priv,
		unsigned int backlight_level)
{
	enum dsi_cmd_set_type type = DSI_CMD_SET_BACKLIGHT;
	struct dsi_cmd_desc cmd = {0};
	struct dsi_cmd_set *cmd_set;
	u8 tx_buf[3];

	tx_buf[0] = 0x51;
	tx_buf[1] = (backlight_level & 0xFF00) >> 8;
	tx_buf[2] = backlight_level & 0xFF;

	cmd.msg.channel = 0;
	cmd.msg.type = 0x39;
	cmd.msg.tx_buf = tx_buf;
	cmd.msg.flags = MIPI_DSI_MSG_USE_LPM;
	cmd.msg.tx_len = 3;
	cmd.msg.tx_buf = (void *)tx_buf;
	cmd.msg.rx_len = 0;

	cmd_set = &priv->cmd_sets[type];
	cmd_set->cmd_state = DSI_CMD_SET_STATE_LP;
	cmd_set->type = type;
	cmd_set->num_cmds = 1;
	cmd_set->cmds = &cmd;

	dsi_panel_cmd_set_send(priv, DSI_CMD_SET_BACKLIGHT);
	if (priv->panel_xeq_enabled)
		dsi_panel_cmd_set_send(priv, DSI_CMD_SET_XEQ_SYNC);
}

static int32_t panel_set_backlight(struct platform_device *pdev, uint32_t id, void *value)
{
	struct panel_drv_private *priv;

	dpu_pr_debug("+\n");

	priv = to_panel_priv(platform_get_drvdata(pdev));

	dpu_check_and_return(!priv, -1, "priv is null\n");
	if (priv->is_fake_panel) {
		dpu_pr_info("virtual panel return\n");
		return 0;
	}

	if (priv->base.backlight_setting_type == BACKLIGHT_SET_BY_I2C)
		ktz8866_set_backlight(priv->base.brightness_init_level);
	else
		dsi_panel_backlight_cmd_set_send(priv, priv->base.brightness_init_level);
	dpu_pr_info("panel set backlight as %d\n", priv->base.brightness_init_level);
	return 0;
}

static int32_t panel_check_lcd_status(struct platform_device *pdev, uint32_t id, void *value)
{
	return 0;
}

static int32_t lcd_send_initial_cmd(struct platform_device *pdev, uint32_t id, void *value)
{
	struct panel_drv_private *priv;

	priv = to_panel_priv(platform_get_drvdata(pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");

	return dsi_panel_cmd_set_send(priv, DSI_CMD_SET_ON);
}

static int32_t lcd_send_display_off_cmd(struct platform_device *pdev, uint32_t id, void *value)
{
	struct panel_drv_private *priv;
	
	dpu_pr_debug("+\n");
	priv = to_panel_priv(platform_get_drvdata(pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");

	return dsi_panel_cmd_set_send(priv, DSI_CMD_SET_OFF);
}

static int32_t need_reset_after_lp11(struct platform_device *pdev, uint32_t id, void *value)
{
	struct panel_drv_private *priv;

	dpu_pr_debug("+\n");
	priv = to_panel_priv(platform_get_drvdata(pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");

	dpu_pr_debug("-\n");
	return priv->reset_after_lp11;
}

static struct ops_handle_table g_priv_handle[] = {
	{"set_backlight", panel_set_backlight},
	{"check_lcd_status", panel_check_lcd_status},
	{"lcd_send_initial_cmd", lcd_send_initial_cmd},
	{"lcd_send_display_off_cmd", lcd_send_display_off_cmd},
	{"panel_reset_flush", panel_reset_flush},
	{"need_reset_after_lp11", need_reset_after_lp11},
};

static int32_t panel_of_device_setup(struct platform_device *pdev)
{
	struct panel_drv_private *priv;
	struct dpu_panel_info *pinfo;
	int ret = 0;

	priv = to_panel_priv(platform_get_drvdata(pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");
	dpu_pr_debug("parse %a info\n", pdev->name);
	if (!dpu_str_cmp(pdev->name, "panel_offline")) {
		dpu_pr_debug("parse panel_offline info\n");
		pinfo = &priv->base;
		pinfo->connector_id = CONNECTOR_WB0;
		pinfo->external_connector_id = CONNECTOR_WB1;
		pinfo->xres = 1440;
		pinfo->yres = 3200;
		pinfo->bpp = 4;

		priv->is_fake_panel = true;
	} else {
		dpu_pr_debug("parse %a info\n", pdev->name);
		ret = dsi_panel_info_parse(priv, pdev->name);
		if (ret) {
			dpu_pr_err("parse panel info failed, ret = %d\n", ret);
			return ret;
		}
	}

	return ret;
}

static void panel_of_device_remove(struct platform_device *pdev)
{
	struct panel_drv_private *priv;

	dpu_pr_debug("+\n");
	priv = to_panel_priv(platform_get_drvdata(pdev));
	dpu_check_and_no_retval(!priv, "priv is null\n");
	if (dpu_str_cmp(pdev->name, "panel_offline")) {
		dpu_pr_debug("release %a info\n", pdev->name);
		dsi_panel_info_release(priv);
	}

	dpu_pr_debug("-\n");
}

int32_t prepare_panel_data(struct platform_device *pdev, struct panel_dev *panel_dev)
{
	dpu_pr_debug("+\n");
	dpu_check_and_return(!panel_dev, -1, "panel dev is null\n");
	dpu_check_and_return(!pdev, -1, "platform_device is null\n");

	if (panel_of_device_setup(pdev)) {
		dpu_pr_err("Device initialization is failed!\n");
		return -1;
	}

	panel_dev->on = panel_power_on;
	panel_dev->off = panel_power_off;
	panel_dev->handle_table = g_priv_handle;
	panel_dev->ops_size = ARRAY_SIZE(g_priv_handle);

	dpu_pr_debug("-\n");
	return 0;
}

void unprepare_panel_data(struct platform_device *pdev)
{
	dpu_pr_debug("+\n");
	dpu_check_and_no_retval(!pdev, "platform_device is null\n");

	panel_of_device_remove(pdev);
	dpu_pr_debug("-\n");
}

int dsi_panel_cmd_read(struct panel_drv_private *priv,
		struct dsi_cmd_desc *cmd, u8 *rx_buf, u32 rx_len)
{
	struct dsi_ctrl_hw_blk hw = {0};
	struct connector *connector;
	int i, ret = 0;

	if (!cmd || !rx_buf || !rx_len) {
		dpu_pr_err("get invalid parameters\n");
		return -1;
	}

	cmd->msg.flags |= MIPI_DSI_MSG_UNICAST_COMMAND;
	cmd->msg.rx_buf = rx_buf;
	cmd->msg.rx_len = rx_len;

	connector = get_connector(priv->base.connector_id);
	dpu_check_and_return(!connector, -1, "connector is null\n");
	get_dsi_ctrl_hw(connector, &hw);
	ret = dsi_ctrl_cmd_desc_transfer(&hw, cmd);
	if (ret) {
		dpu_pr_err("panel failed to read cmd\n");
		cmd->msg.rx_buf = NULL;
		cmd->msg.rx_len = 0;
		return ret;
	}

	for (i = 0; i < rx_len; ++i) {
		dpu_pr_debug("read reg 0x%x: 0x%x\n",
			((u8 *)(cmd->msg.tx_buf))[0], ((u8 *)(cmd->msg.rx_buf))[i]);
	}

	cmd->msg.rx_buf = NULL;
	cmd->msg.rx_len = 0;
	return 0;
}

int dsi_panel_cmd_set_send(struct panel_drv_private *priv, enum dsi_cmd_set_type type)
{
	struct dsi_ctrl_hw_blk hw = {0};
	struct dsi_ctrl_hw_blk hw_bind = {0};
	struct dsi_cmd_desc *cmd_descs;
	struct dsi_cmd_desc *cmd_desc;
	enum dsi_cmd_set_state state;
	struct connector *connector;
	int num_cmds;
	int ret = 0;
	int i;

	if (!priv) {
		dpu_pr_err("priv is null\n");
		return -1;
	}

	cmd_descs = priv->cmd_sets[type].cmds;
	num_cmds = priv->cmd_sets[type].num_cmds;
	state = priv->cmd_sets[type].cmd_state;

	if (!cmd_descs) {
		dpu_pr_debug("no commands to be send, num cmds = %d\n", num_cmds);
		goto error;
	}

	connector = get_connector(priv->base.connector_id);
	dpu_check_and_return(!connector, -1, "connector is null\n");

	get_dsi_ctrl_hw(connector, &hw);
	if (connector->bind_connector)
		get_dsi_ctrl_hw(connector->bind_connector, &hw_bind);

	if (state == DSI_CMD_SET_STATE_LP)
		dpu_pr_debug("transfer cmd with low power mode\n");

	dpu_pr_debug("num cmds:%d\n", num_cmds);
	for (i = 0; i < num_cmds; i++) {
		cmd_desc = &cmd_descs[i];

		/* set low power mode flag */
		if (state == DSI_CMD_SET_STATE_LP)
			cmd_desc->msg.flags |= MIPI_DSI_MSG_USE_LPM;

		ret = dsi_ctrl_cmd_desc_transfer(&hw, cmd_desc);
		if (ret) {
			dpu_pr_err("panel failed to send cmd set:%d\n", type);
			return ret;
		}
		if (connector->bind_connector) {
			ret = dsi_ctrl_cmd_desc_transfer(&hw_bind, cmd_desc);
			if (ret) {
				dpu_pr_err("panel failed to send bind cmd set:%d\n", type);
				return ret;
			}
		}

		if (cmd_desc->post_wait_us)
			dpu_udelay(cmd_desc->post_wait_us);
	}
error:
	return ret;
}
