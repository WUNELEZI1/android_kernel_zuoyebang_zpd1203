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

#include <linux/of_gpio.h>
#include <linux/irqdesc.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/of.h>

#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>
#include <soc/xring/trace_hook_set.h>
#include <video/mipi_display.h>

#include "dsi_panel_event_notifier.h"
#include "dpu_cont_display.h"
#include "dpu_backlight.h"
#include "dsi_panel.h"
#include "dsi_host.h"
#include "dpu_log.h"
#include "backlight_ktz8866.h"
#include "dpu_trace.h"
#include "dpu_hw_ulps.h"

#ifdef MI_DISPLAY_MODIFY
#include "mi_disp_parser.h"
#include "mi_panel_id.h"
#include "mi_dsi_display.h"
#endif

#define CMD_SET_MIN_SIZE 7
#define UPDATE_INFO_MEMBER 3
#define FLAT_MODE_INFO_MEMBER 3
#define MAX_FPS_LIST_SIZE 32
#define PANEL_POWER_STATE_SIZE 2
#define ESD_CHECK_PERIOD_MS 5000
#define ESD_RECHECK_TE_TIMEOUT_MS 300
#define READ_BACK_BUFFER_SIZE 256
#define PREFERRED_MODE_INDEX 0

static struct dsi_cmd_mgr *g_dsi_cmd_mgr;

inline bool dsi_cmd_type_is_read(u8 type)
{
	switch (type) {
	case MIPI_DSI_DCS_READ:
	case MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM:
		return true;
	default:
		return false;
	}
}

u32 dsi_basic_cmd_flag_get(struct dsi_cmd_desc *cmd, u8 transfer_type,
		u32 cur_index, u32 num_cmds)
{
	u32 flag = 0;
	bool is_read_pkt;

	if (unlikely(!cmd || !num_cmds || cur_index >= num_cmds)) {
		DSI_ERROR("get invalid paramters, %pK, cur: %d, num:%d\n",
				cmd, cur_index, num_cmds);
		return flag;
	}

	if (transfer_type == USE_CMDLIST)
		return flag;

	is_read_pkt = dsi_cmd_type_is_read(cmd->msg.type);
	if (num_cmds == 1) {
		flag |= DSI_CMD_FLAG_EXIT_ULPS;
		flag |= DSI_CMD_FLAG_ENTER_ULPS;
		flag |= DSI_CMD_FLAG_GET_MIPI_STATE;
		flag |= DSI_CMD_FLAG_WAIT_CRI_AVAILABLE;
		if (is_read_pkt)
			flag |= DSI_CMD_FLAG_EARLY_CLEAR_MIPI_STATE;
		else
			flag |= DSI_CMD_FLAG_CLEAR_MIPI_STATE;
	} else if (cur_index == 0) {
		flag |= DSI_CMD_FLAG_EXIT_ULPS;
		flag |= DSI_CMD_FLAG_GET_MIPI_STATE;
		flag |= DSI_CMD_FLAG_WAIT_CRI_AVAILABLE;
		if (!is_read_pkt)
			flag |= DSI_CMD_FLAG_CRI_HOLD;
	} else if (cur_index == (num_cmds - 1)) {
		flag |= DSI_CMD_FLAG_CLEAR_MIPI_STATE;
		flag |= DSI_CMD_FLAG_ENTER_ULPS;
		if (!is_read_pkt)
			flag |= DSI_CMD_FLAG_CRI_UNHOLD;
	}

	DSI_DEBUG("get basic flag:0x%x\n", flag);
	return flag;
}

static void dsi_cmd_set_dump(struct dsi_cmd_set *cmd_set)
{
	struct dsi_cmd_desc *cmd;
	int i, j;

	if (!cmd_set) {
		DSI_ERROR("invalid parameter\n");
		return;
	}

	DSI_DEBUG("cmd set %s info:\n", dsi_panel_cmd_set_name_get(cmd_set->type));
	DSI_DEBUG("type:%d\n", cmd_set->type);
	DSI_DEBUG("port_index:%d\n", cmd_set->port_index);
	DSI_DEBUG("payload_size:%d\n", cmd_set->payload_size);
	DSI_DEBUG("priority_level:%d\n", cmd_set->priority_level);
	DSI_DEBUG("delete_after_usage:%d\n", cmd_set->delete_after_usage);
	DSI_DEBUG("num_cmds:%d\n", cmd_set->num_cmds);
	for (i = 0; i < cmd_set->num_cmds; ++i) {
		cmd = &cmd_set->cmds[i];
		DSI_DEBUG("cmd[%d] tx_len: %zu\n", i, cmd->msg.tx_len);
		for (j = 0; j < cmd->msg.tx_len; j++)
			DSI_DEBUG("cmd[%d] tx_buf[%d]:0x%x\n", i, j, ((u8 *)cmd->msg.tx_buf)[j]);
	}
	DSI_DEBUG("num_infos:%d\n", cmd_set->num_infos);
	for (i = 0; i < cmd_set->num_infos; ++i) {
		DSI_DEBUG("info[%d] addr:0x%x, index:%d, len:%d\n",
				i, cmd_set->infos[i].addr,
				cmd_set->infos[i].index,
				cmd_set->infos[i].len);
	}

}

struct dsi_cmd_set *dsi_cmd_set_duplicate(struct dsi_cmd_set *cmd_set)
{
	struct dsi_cmd_set *new_cmd_set = NULL;
	struct dsi_cmd_desc *cmd, *cmds = NULL;
	struct dsi_cmd_set_update_info *infos = NULL;
	uint8_t *tx_buf, *rx_buf;
	int i, ret;

	new_cmd_set = kzalloc(sizeof(struct dsi_cmd_set), GFP_KERNEL);
	if (unlikely(!new_cmd_set))
		return ERR_PTR(-ENOMEM);

	cmds = kcalloc(cmd_set->num_cmds, sizeof(*cmd_set->cmds), GFP_KERNEL);
	if (unlikely(!cmds)) {
		DSI_ERROR("cmds alloc failed!\n");
		ret = -ENOMEM;
		goto error_with_release_cmd_set;
	}

	// dsi_cmd_set_dump(cmd_set);

	memcpy(new_cmd_set, cmd_set, sizeof(struct dsi_cmd_set));
	/* duplicate cmds */
	for (i = 0; i < cmd_set->num_cmds; ++i) {
		cmd = &cmds[i];
		memcpy(cmd, &cmd_set->cmds[i], sizeof(struct dsi_cmd_desc));
		tx_buf = NULL;
		rx_buf = NULL;
		cmd->msg.tx_buf = NULL;
		cmd->msg.rx_buf = NULL;
		if (likely(cmd->msg.tx_len)) {
			tx_buf = kzalloc(cmd->msg.tx_len, GFP_KERNEL);
			if (unlikely(!tx_buf)) {
				ret = -ENOMEM;
				goto error_with_release_cmds;
			}
			memcpy(tx_buf, (const void *)cmd_set->cmds[i].msg.tx_buf, cmd->msg.tx_len);
		}

		if (unlikely(cmd->msg.rx_len)) {
			rx_buf = kzalloc(cmd->msg.rx_len, GFP_KERNEL);
			if (unlikely(!rx_buf)) {
				ret = -ENOMEM;

				kfree(tx_buf);
				goto error_with_release_cmds;
			}
			memcpy(rx_buf, cmd_set->cmds[i].msg.rx_buf, cmd->msg.rx_len);
		}
		cmd->msg.tx_buf = tx_buf;
		cmd->msg.rx_buf = rx_buf;
	}

	if (cmd_set->num_infos) {
		infos = kcalloc(cmd_set->num_infos, sizeof(*cmd_set->infos), GFP_KERNEL);
		if (unlikely(!infos)) {
			DSI_ERROR("infos alloc failed!\n");
			ret = -ENOMEM;
			goto error_with_release_cmds;
		}
		memcpy(infos, cmd_set->infos, sizeof(*cmd_set->infos) * cmd_set->num_infos);
	}

	new_cmd_set->cmds = cmds;
	new_cmd_set->num_infos = cmd_set->num_infos;
	new_cmd_set->infos = infos;
	/* delete in __dsi_panel_cmd_set_send */
	new_cmd_set->delete_after_usage = true;
	// dsi_cmd_set_dump(new_cmd_set);
	return new_cmd_set;

error_with_release_cmds:
	while (--i >= 0) {
		kfree(cmds[i].msg.tx_buf);
		kfree(cmds[i].msg.rx_buf);
	}
	kfree(cmds);
error_with_release_cmd_set:
	kfree(new_cmd_set);
	return ERR_PTR(ret);
}

static void dsi_panel_cmds_free(struct dsi_cmd_set *cmd_set)
{
	struct dsi_cmd_desc *cmd;
	int i;

	for (i = 0; i < cmd_set->num_cmds; i++) {
		cmd = &cmd_set->cmds[i];
		kfree(cmd->msg.tx_buf);
		kfree(cmd->msg.rx_buf);
	}

	kfree(cmd_set->cmds);
	cmd_set->cmds = NULL;
	cmd_set->num_cmds = 0;
}

static void dsi_cmd_set_content_free(struct dsi_cmd_set *cmd_set)
{
	if (cmd_set) {
		dsi_panel_cmds_free(cmd_set);
		kfree(cmd_set->infos);
		cmd_set->infos = NULL;
		cmd_set->num_infos = 0;
	}
}

void dsi_cmd_set_delete(struct dsi_cmd_set *cmd_set)
{
	if (cmd_set) {
		dsi_cmd_set_content_free(cmd_set);
		kfree(cmd_set);
	}
}

enum dsi_cmd_set_priority cmd_sets_priority[DSI_CMD_SET_MAX] = {
	PRIORITY_CRITICAL,
	PRIORITY_CRITICAL,
	PRIORITY_HIGH,
	PRIORITY_HIGH,
	PRIORITY_HIGH,
	PRIORITY_LOW,
	PRIORITY_LOW,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_LOW,
#ifdef MI_DISPLAY_MODIFY
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
	PRIORITY_NORMAL,
#endif
};

static int dsi_cmd_mgr_init(struct dsi_panel *panel)
{
	struct dsi_cmd_mgr *dsi_cmd_mgr;
	int ret = 0;

	if (!panel) {
		DSI_ERROR("failed to init dsi cmd mgr\n");
		return -EINVAL;
	}

	dsi_cmd_mgr = kzalloc(sizeof(struct dsi_cmd_mgr), GFP_KERNEL);
	if (!dsi_cmd_mgr) {
		DSI_ERROR("dsi_cmd_mgr alloc failed!\n");
		return -ENOMEM;
	}

	dsi_cmd_mgr->front = 0;
	dsi_cmd_mgr->rear = 0;
	dsi_cmd_mgr->num_cmd_set = 0;
	dsi_cmd_mgr->header_size = 0;
	dsi_cmd_mgr->payload_size = 0;
	dsi_cmd_mgr->transfered_payload_size = 0;

	mutex_init(&dsi_cmd_mgr->cmd_mgr_lock);
	dsi_cmd_mgr->panel = panel;
	g_dsi_cmd_mgr = dsi_cmd_mgr;

	return ret;
}

void dsi_cmd_mgr_deinit(void)
{
	if (g_dsi_cmd_mgr) {
		mutex_destroy(&g_dsi_cmd_mgr->cmd_mgr_lock);

		kfree(g_dsi_cmd_mgr);

		g_dsi_cmd_mgr = NULL;
	}
}


char *dsi_panel_esd_mode_name_get(int esd_mode)
{
	switch (esd_mode) {
	case DSI_PANEL_ESD_MODE_ERR_INT:
		return "err_int";
	case DSI_PANEL_ESD_MODE_REG_READ:
		return "reg_read";
	case DSI_PANEL_ESD_MODE_ERR_INT_AND_REG_READ:
		return "err_int_and_reg_read";
	case DSI_PANEL_ESD_MODE_PANEL_TE:
		return "panel_te";
	case DSI_PANEL_ESD_MODE_SW_SUCCESS:
		return "always_success";
	case DSI_PANEL_ESD_MODE_SW_FAILURE:
		return "always_failure";
	default:
		return "invalid ESD mode";
	}
}

void dsi_panel_mode_update(struct dsi_panel *panel,
		struct dsi_panel_mode *new_mode)
{
	if (!panel || !new_mode) {
		DSI_ERROR("get invalid parameters, %pK, %pK\n", panel, new_mode);
		return;
	}

	panel->panel_info.current_mode = new_mode;
}

inline void dsi_panel_lock(struct dsi_panel *panel)
{
	static int count;

	DSI_DEBUG("request panel lock %d\n", count);
	mutex_lock(&panel->panel_lock);
	count++;
}

inline void dsi_panel_unlock(struct dsi_panel *panel)
{
	static int count;

	DSI_DEBUG("release panel lock %d\n", count);
	mutex_unlock(&panel->panel_lock);
	count++;
}

int dsi_panel_status_reg_read(struct dsi_panel *panel, int port_index)
{
	struct dsi_panel_esd_info *esd_info;
	struct dsi_cmd_set *cmd_set;
	struct dsi_cmd_desc *cmds;
	u8 rx_buf[READ_BACK_BUFFER_SIZE];
	int i, ret = 0, index = 0;
	u32 transfer_flag = 0;

	esd_info = &panel->panel_info.esd_info;
	cmd_set = &esd_info->panel_status_cmd;
	cmds = cmd_set->cmds;

	for (i = 0; i < cmd_set->num_cmds; ++i) {
		memset(rx_buf, 0x0, READ_BACK_BUFFER_SIZE);
		cmds[i].msg.flags |= MIPI_DSI_MSG_UNICAST_COMMAND;
		cmds[i].port_index = port_index;
		cmds[i].msg.rx_buf = rx_buf;
		cmds[i].msg.rx_len = esd_info->status_length[i];
		if (esd_info->panel_status_cmd.cmd_state == DSI_CMD_SET_STATE_LP)
			cmds[i].msg.flags |= MIPI_DSI_MSG_USE_LPM;

		transfer_flag = dsi_basic_cmd_flag_get(&cmds[i], USE_CPU, i, cmd_set->num_cmds);
		ret = dsi_host_cmd_transfer(panel->host, &cmds[i], USE_CPU, transfer_flag);
		if (ret)
			DSI_ERROR("failed to read %d-th cmd\n", i);

		memcpy(esd_info->status_buf + index, rx_buf, esd_info->status_length[i]);
		index += esd_info->status_length[i];
	}

	for (i = 0; i < index; ++i)
		DSI_DEBUG("get panel status[%d] = 0x%x\n", i, esd_info->status_buf[i]);
	return 0;

}

bool dsi_panel_status_value_validate(struct dsi_panel *panel)
{
	struct dsi_panel_esd_info *esd_info;
	int i;

	esd_info = &panel->panel_info.esd_info;

	for (i = 0; i < esd_info->status_length_total; ++i) {
		DSI_DEBUG("status_buf[%d] = 0x%x, status_value[%d] = 0x%x\n",
				i, esd_info->status_buf[i],
				i, esd_info->status_value[i]);
		if (esd_info->status_buf[i] != esd_info->status_value[i]) {
			DSI_ERROR("status value validate failed, 0x%x, 0x%x\n",
					esd_info->status_buf[i],
					esd_info->status_value[i]);
			return false;
		}
	}

	return true;
}

int dsi_panel_esd_attack_trigger(struct dsi_panel *panel)
{
	struct reset_ctrl *reset_ctrl;

	if (!panel) {
		DSI_ERROR("get invalid parameter, %pK", panel);
		return -EINVAL;
	}

	if (!dsi_panel_initialized(panel)) {
		DSI_INFO("panel is uninitialized, do nothing\n");
		return 0;
	}

	reset_ctrl = &panel->panel_info.reset_ctrl;

	if (!gpio_is_valid(reset_ctrl->gpio)) {
		DSI_ERROR("get invalid reset gpio %d\n", reset_ctrl->gpio);
		return -EINVAL;
	}

	gpio_set_value(reset_ctrl->gpio, 0);
	DSI_INFO("trigger esd attack by pull down the reset gpio output\n");
	return 0;
}

int dsi_panel_esd_irq_ctrl(struct dsi_panel *panel, bool enable)
{
	struct dsi_panel_esd_info *esd_info;
	struct irq_desc *desc;
	int i;

	if (unlikely(!panel)) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	trace_dsi_panel_esd_irq_ctrl("esd irq enabled");

	esd_info = &panel->panel_info.esd_info;
	for (i = 0; i < esd_info->err_irq_gpio_num; ++i) {
		if (enable && !esd_info->err_irq_enabled) {
			desc = irq_to_desc(esd_info->err_irq[i]);
			/* TODO: make sure that irq is level trigger */
			// if (!irq_settings_is_level(desc))
			// desc->istate &= ~IRQS_PENDING;

			enable_irq_wake(esd_info->err_irq[i]);
			enable_irq(esd_info->err_irq[i]);
			esd_info->err_irq_enabled = true;
			DSI_DEBUG("%s = %d is enabled\n",
					esd_info->err_irq_gpio_name[i],
					esd_info->err_irq_gpio[i]);
		} else if (!enable && esd_info->err_irq_enabled) {
			disable_irq_wake(esd_info->err_irq[i]);
			disable_irq_nosync(esd_info->err_irq[i]);
			esd_info->err_irq_enabled = false;
			DSI_DEBUG("%s = %d is disabled\n",
					esd_info->err_irq_gpio_name[i],
					esd_info->err_irq_gpio[i]);
		}


	}

	return 0;
}

bool dsi_panel_initialized(struct dsi_panel *panel)
{
	return panel->initialized;
}

int dsi_panel_power_mgr_init(struct dsi_panel_power_mgr **power_mgr)
{
	struct dsi_panel_power_mgr *dsi_panel_power_mgr;
	int ret = 0;

	if (*power_mgr) {
		POWER_ERROR("dsi panel power manager has been initialized\n");
		return -EINVAL;
	}

	dsi_panel_power_mgr = kzalloc(sizeof(struct dsi_panel_power_mgr), GFP_KERNEL);
	if (!dsi_panel_power_mgr) {
		DSI_ERROR("dsi_panel_power_mgr alloc failed!\n");
		return -ENOMEM;
	}

	dsi_panel_power_mgr->count = 0;
	mutex_init(&dsi_panel_power_mgr->lock);

	*power_mgr = dsi_panel_power_mgr;

	return ret;
}

void dsi_panel_power_mgr_deinit(struct dsi_panel_power_mgr *power_mgr)
{
	if (!power_mgr) {
		PERF_ERROR("invalid parameter\n");
		return;
	}

	mutex_destroy(&power_mgr->lock);
	kfree(power_mgr);
	power_mgr = NULL;
}

int dsi_panel_cmd_set_update(struct dsi_cmd_set *cmd_set,
		u8 info_index, u8 *data_buf, u32 data_len)
{
	struct dsi_cmd_set_update_info *info;
	size_t tx_len;
	u8 *tx_buf;
	int i;

	if (!cmd_set || !cmd_set->infos || !data_buf || !data_len) {
		DSI_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	if (!cmd_set->num_cmds || !cmd_set->num_infos) {
		DSI_ERROR("num cmds:%d, num infos:%d\n",
				cmd_set->num_cmds, cmd_set->num_infos);
		return -EINVAL;
	}

	info = &cmd_set->infos[info_index];
	if (info->index >= cmd_set->num_cmds) {
		DSI_ERROR("cmd index:%d, num cmds:%d\n",
				info->index, cmd_set->num_cmds);
		return -EINVAL;
	}

	tx_buf = (u8 *)cmd_set->cmds[info->index].msg.tx_buf;
	tx_len = cmd_set->cmds[info->index].msg.tx_len;
	if (!tx_buf) {
		DSI_ERROR("invalid tx buffer");
		return -EINVAL;
	}

	if (tx_buf[0] != info->addr || tx_len < info->len) {
		DSI_ERROR("buf[0]:0x%X, info addr:0x%X, tx len:%zu, info len:%d",
				tx_buf[0], info->addr, tx_len, info->len);
		return -EINVAL;
	}

	memcpy(&tx_buf[1], data_buf, info->len);
	for (i = 0; i < tx_len; i++)
		DSI_DEBUG("tx_buf[%d]: 0x%x", i, tx_buf[i]);

	return 0;

}

static int dsi_panel_cmd_set_gamma_cfg_update(struct dsi_panel *panel,
		enum dsi_cmd_set_type type)
{
	struct dsi_panel_flat_mode_info *flat_mode_info;
	struct dsi_cmd_set_update_info *info;
	struct dsi_panel_mode *current_mode;
	struct dsi_cmd_set *cmd_set;
	u8 gamma;
	int i, j;

	if (!panel || !panel->panel_info.current_mode) {
		DSI_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	current_mode = panel->panel_info.current_mode;
	cmd_set = &current_mode->cmd_sets[type];
	if (!cmd_set->num_infos || !current_mode->flat_mode_info_num) {
		DSI_DEBUG("no update info or flat mode info %d, %d\n",
				cmd_set->num_infos,
				current_mode->flat_mode_info_num);
		return 0;
	}

	/* TODO: update flat mode status when flat mode feature is ready */
	for (i = 0; i < cmd_set->num_infos; ++i) {
		info = &cmd_set->infos[i];
		for (j = 0; j < current_mode->flat_mode_info_num; ++j) {
			flat_mode_info = &current_mode->flat_mode_infos[i];
			if (flat_mode_info->mipi_addr == info->addr) {
				gamma = flat_mode_info->flat_off_gamma;
				dsi_panel_cmd_set_update(cmd_set, i,
						&gamma, sizeof(gamma));
				DSI_DEBUG("mipi addr: %d, gamma: %u\n",
						info->addr, gamma);
			}
			DSI_DEBUG("%d %d\n", flat_mode_info->mipi_addr, info->addr);
		}
	}

	return 0;
}

int dsi_panel_timing_switch(struct dsi_panel *panel)
{
	int ret = 0;

	if (!panel) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

#ifdef MI_DISPLAY_MODIFY
	mi_dsi_panel_timing_switch(panel);
#endif
	dsi_panel_cmd_set_gamma_cfg_update(panel, DSI_CMD_SET_TIMING_SWITCH);
	ret = dsi_panel_cmd_set_send(panel,
			DSI_CMD_SET_TIMING_SWITCH,
			NULL,
			USE_CMDLIST);
	if (ret)
		DSI_ERROR("failed to send timing switch command rc:%d\n", ret);

	DSI_DEBUG("switch panel timing success\n");
	return ret;
}

int dsi_panel_post_timing_switch(struct dsi_panel *panel)
{
	int ret = 0;

	if (!panel) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	dsi_panel_cmd_set_gamma_cfg_update(panel, DSI_CMD_SET_POST_TIMING_SWITCH);

	ret = dsi_panel_cmd_set_send(panel,
			DSI_CMD_SET_POST_TIMING_SWITCH,
			NULL,
			USE_CMDLIST);
	if (ret)
		DSI_ERROR("failed to send post timing switch command rc:%d\n",
				ret);

	DSI_DEBUG("post switch panel timing success\n");
	return ret;
}

static void dsi_panel_convert_to_panel_mode(struct drm_display_mode *drm_mode,
		struct dsi_panel_mode *panel_mode)
{
	if (!drm_mode) {
		DSI_ERROR("invalid parameter\n");
		return;
	}

	panel_mode->timing.framerate = drm_mode_vrefresh(drm_mode);

	panel_mode->timing.hdisplay = drm_mode->hdisplay;
	panel_mode->timing.hbp = drm_mode->htotal - drm_mode->hsync_end;
	panel_mode->timing.hpw = drm_mode->hsync_end - drm_mode->hsync_start;
	panel_mode->timing.hfp = drm_mode->hsync_start - drm_mode->hdisplay;
	panel_mode->timing.h_skew = drm_mode->hskew;

	panel_mode->timing.vdisplay = drm_mode->vdisplay;
	panel_mode->timing.vbp = drm_mode->vtotal - drm_mode->vsync_end;
	panel_mode->timing.vpw = drm_mode->vsync_end - drm_mode->vsync_start;
	panel_mode->timing.vfp = drm_mode->vsync_start - drm_mode->vdisplay;
}

static bool dsi_panel_mode_timing_compare(struct dsi_panel_mode *mode1,
		struct dsi_panel_mode *mode2)
{
	if (!mode1 && !mode2)
		return true;

	if (!mode1 || !mode2)
		return false;

	if (mode1->timing.hdisplay == mode2->timing.hdisplay &&
			mode1->timing.vdisplay == mode2->timing.vdisplay &&
			mode1->timing.framerate == mode2->timing.framerate &&
			mode1->timing.hbp == mode2->timing.hbp &&
			mode1->timing.hpw == mode2->timing.hpw &&
			mode1->timing.hfp == mode2->timing.hfp &&
			mode1->timing.h_skew == mode2->timing.h_skew &&
			mode1->timing.vbp == mode2->timing.vbp &&
			mode1->timing.vpw == mode2->timing.vpw &&
			mode1->timing.vfp == mode2->timing.vfp) {
		return true;
	}

	return false;
}

int dsi_panel_mode_search(struct dsi_panel *panel,
		struct drm_display_mode *drm_mode,
		struct dsi_panel_mode **result)
{
	struct dsi_panel_mode *mode, target_mode;
	struct dsi_panel_info *panel_info;
	int ret = -EINVAL;
	int i;

	if (!panel || !drm_mode || !panel->panel_info.modes) {
		DSI_ERROR("inalid parameters\n");
		return ret;
	}

	panel_info = &panel->panel_info;
	dsi_panel_convert_to_panel_mode(drm_mode, &target_mode);

	/* adjust hdisplay for sbs mode */
	if (panel_info->host_info.sbs_en)
		target_mode.timing.hdisplay /= DSI_PORT_MAX;

	for (i = 0; i < panel->panel_info.num_modes; i++) {
		mode = &panel->panel_info.modes[i];
		if (dsi_panel_mode_timing_compare(mode, &target_mode)) {
			*result = mode;
			ret = 0;
			DSI_DEBUG("set mode success:\n");
			dsi_panel_timing_dump(mode->cur_timing);
			break;
		}
	}

	return ret;
}

static int __dsi_panel_cmd_set_send(struct dsi_panel *panel,
		struct dsi_cmd_set *cmd_set, u8 transfer_type)
{
	struct dsi_cmd_desc *cmd_descs;
	struct dsi_cmd_desc *cmd_desc;
	enum dsi_cmd_set_state state;
	const char *cmd_set_name;
	int num_cmds, port_index;
	uint16_t flags = 0;
	int i, ret = 0;
	u32 transfer_flag;

	port_index = cmd_set->port_index;
	cmd_descs = cmd_set->cmds;
	num_cmds = cmd_set->num_cmds;
	state = cmd_set->cmd_state;
	cmd_set_name = dsi_panel_cmd_set_name_get(cmd_set->type);

	if (state == DSI_CMD_SET_STATE_LP)
		flags = MIPI_DSI_MSG_USE_LPM;

	DSI_DEBUG("send %s with %d cmds begin\n", cmd_set_name, num_cmds);
	for (i = 0; i < num_cmds; i++) {
		cmd_desc = &cmd_descs[i];
		cmd_desc->port_index = port_index;
		cmd_desc->msg.flags |= flags;
		transfer_flag = dsi_basic_cmd_flag_get(cmd_desc, transfer_type, i, num_cmds);
		if (transfer_type == USE_CPU &&
				(cmd_set->type == DSI_CMD_SET_ON ||
				cmd_set->type == DSI_CMD_SET_MI_ROUND_CORNER_PORT0_ON ||
				cmd_set->type == DSI_CMD_SET_MI_ROUND_CORNER_PORT1_ON ||
				cmd_set->type == DSI_CMD_SET_OFF)) {
			transfer_flag |= DSI_CMD_FLAG_WAIT_CRI_AVAILABLE;
			transfer_flag &= (~DSI_CMD_FLAG_CRI_HOLD);
			transfer_flag &= (~DSI_CMD_FLAG_CRI_UNHOLD);
		}

		DSI_DEBUG("get flag:0x%x, 0x%hx\n", transfer_flag, cmd_desc->msg.flags);
		ret = dsi_host_cmd_transfer(panel->host, cmd_desc, transfer_type, transfer_flag);
		if (ret)
			DSI_ERROR("failed to send %s %d-th cmd, ret=%d\n",
					cmd_set_name, i, ret);

		/* post wait for cmdlist send is useless */
		if (unlikely(transfer_type == USE_CPU && cmd_desc->post_wait_us))
			usleep_range(cmd_desc->post_wait_us,
					(cmd_desc->post_wait_us + 10));
	}

	DSI_DEBUG("send %s with %d cmds success\n", cmd_set_name, num_cmds);
	return ret;
}

int dsi_panel_cmd_set_send(struct dsi_panel *panel, enum dsi_cmd_set_type type,
		struct dsi_cmd_set *cmd_set, u8 transfer_type)
{
	struct dsi_cmd_set *new_cmd_set;
	struct dsi_cmd_desc *cmd_descs;
	const char *cmd_set_name;
	int num_cmds;
	int ret = 0;

	cmd_set_name = cmd_set ? dsi_panel_cmd_set_name_get(cmd_set->type) :
			dsi_panel_cmd_set_name_get(type);
	if (unlikely(!panel || (!cmd_set && !panel->panel_info.current_mode))) {
		DSI_ERROR("invalid parameter, send %s failed\n",
				cmd_set_name);
		return -EINVAL;
	}

	if (unlikely(panel->panel_info.is_fake_panel)) {
		DSI_INFO("bypass send %s for fake panel\n", cmd_set_name);
		return 0;
	}

	if (!cmd_set)
		cmd_set = &panel->panel_info.current_mode->cmd_sets[type];

	cmd_descs = cmd_set->cmds;
	num_cmds = cmd_set->num_cmds;
	if (IS_ERR_OR_NULL(cmd_descs) || unlikely(!num_cmds)) {
		DSI_DEBUG("no commands to be send, num cmds = %d\n", num_cmds);
		goto error;
	}

	if (transfer_type == USE_CMDLIST && panel->cmd_mgr_enabled) {
		/* delete after use */
		new_cmd_set = dsi_cmd_set_duplicate(cmd_set);
		if (IS_ERR_OR_NULL(new_cmd_set)) {
			DSI_ERROR("failed to duplicate %s\n",
					dsi_panel_cmd_set_name_get(cmd_set->type));
			return -EAGAIN;
		}
		new_cmd_set->delete_after_usage = true;
		ret = dsi_cmd_mgr_push(new_cmd_set, transfer_type);
	} else {
		cmd_set->delete_after_usage = false;
		ret = __dsi_panel_cmd_set_send(panel, cmd_set, transfer_type);
		/* For cpu send, reduce the quota in this frame immediately. */
		if (panel->cmd_mgr_enabled)
			dsi_cmd_mgr_quota_update(cmd_set);
	}

	if (ret)
		DSI_ERROR("failed to send %s, ret = %d\n", cmd_set_name, ret);
error:
	DSI_DEBUG("cmd set:%s, tansfer_type:%s, cmd_mgr:%s\n", cmd_set_name,
			transfer_type == USE_CPU ? "cpu" : "cmdlist",
			panel->cmd_mgr_enabled ? "enabled" : "disable");
	return ret;
}

inline void dsi_cmd_mgr_lock(struct dsi_cmd_mgr *dsi_cmd_mgr)
{
	static int count;

	DSI_DEBUG("request dsi_cmd_mgr lock %d\n", count);
	mutex_lock(&dsi_cmd_mgr->cmd_mgr_lock);
	count++;
}

inline void dsi_cmd_mgr_unlock(struct dsi_cmd_mgr *dsi_cmd_mgr)
{
	static int count;

	DSI_DEBUG("release dsi_cmd_mgr lock %d\n", count);
	mutex_unlock(&dsi_cmd_mgr->cmd_mgr_lock);
	count++;
}

static void dsi_cmd_mgr_status_dump(void)
{
	struct dsi_cmd_mgr *dsi_cmd_mgr = g_dsi_cmd_mgr;

	if (unlikely(!dsi_cmd_mgr)) {
		DSI_ERROR("invalid parameters, %pK\n", dsi_cmd_mgr);
		return;
	}

	DSI_DEBUG("dsi_cmd_mgr current status:\n");
	DSI_DEBUG("num_cmd_set: %d, payload_size: %d, header_size: %d\n",
			dsi_cmd_mgr->num_cmd_set, dsi_cmd_mgr->payload_size,
			dsi_cmd_mgr->header_size);
	DSI_DEBUG("front: %d, rear: %d\n", dsi_cmd_mgr->front, dsi_cmd_mgr->rear);
	DSI_DEBUG("transfered payload: %d, header: %d\n",
			dsi_cmd_mgr->transfered_payload_size,
			dsi_cmd_mgr->transfered_header_size);
}

static bool dsi_cmd_mgr_is_empty_locked(void)
{
	struct dsi_cmd_mgr *dsi_cmd_mgr = g_dsi_cmd_mgr;

	if (!dsi_cmd_mgr) {
		DSI_ERROR("invalid parameters, %pK\n", dsi_cmd_mgr);
		return true;
	}

	return dsi_cmd_mgr->front == dsi_cmd_mgr->rear;
}

static struct dsi_cmd_queue_node *dsi_cmd_mgr_front_get_locked(void)
{
	struct dsi_cmd_mgr *dsi_cmd_mgr = g_dsi_cmd_mgr;
	struct dsi_cmd_queue_node *cmd_set_node = NULL;

	if (!dsi_cmd_mgr) {
		DSI_ERROR("invalid parameters, %pK\n", dsi_cmd_mgr);
		return NULL;
	}

	if (!dsi_cmd_mgr_is_empty_locked())
		cmd_set_node = &dsi_cmd_mgr->queue[dsi_cmd_mgr->front];

	return cmd_set_node;
}

static bool dsi_cmd_mgr_is_full_locked(void)
{
	struct dsi_cmd_mgr *dsi_cmd_mgr = g_dsi_cmd_mgr;
	bool is_queue_full;

	if (!dsi_cmd_mgr) {
		DSI_ERROR("invalid parameters, %pK\n", dsi_cmd_mgr);
		return true;
	}

	is_queue_full = (dsi_cmd_mgr->front ==
			(dsi_cmd_mgr->rear + 1) % DSI_HDR_MEM_MAX);
	if (unlikely(is_queue_full))
		DSI_WARN("queue full, front:%d, rear:%d, payload_size:%d, header_size:%d\n",
				dsi_cmd_mgr->front,
				dsi_cmd_mgr->rear,
				dsi_cmd_mgr->payload_size,
				dsi_cmd_mgr->header_size);

	return is_queue_full;
}

static inline bool dsi_cmd_priority_cmp(const struct dsi_cmd_queue_node *a,
		const struct dsi_cmd_queue_node *b)
{
	if (a->cmd_set->priority_level != b->cmd_set->priority_level)
		return a->cmd_set->priority_level < b->cmd_set->priority_level;
	else
		return a->num_delayed_frame > b->num_delayed_frame;
}

void dsi_cmd_mgr_quota_reset(void)
{
	struct dsi_cmd_mgr *dsi_cmd_mgr = g_dsi_cmd_mgr;
	u8 front, rear;

	if (!dsi_cmd_mgr) {
		DSI_ERROR("invalid parameters, %pK\n", dsi_cmd_mgr);
		return;
	}

	dsi_cmd_mgr_lock(dsi_cmd_mgr);
	dsi_cmd_mgr->transfered_header_size = 0;
	dsi_cmd_mgr->transfered_payload_size = 0;
	if (!dsi_cmd_mgr_is_empty_locked()) {
		front = dsi_cmd_mgr->front;
		rear = dsi_cmd_mgr->rear;
		while (rear != front) {
			dsi_cmd_mgr->queue[rear].num_delayed_frame++;
			rear = (rear - 1 + DSI_HDR_MEM_MAX) % DSI_HDR_MEM_MAX;
		}
	}
	dsi_cmd_mgr_unlock(dsi_cmd_mgr);
}

int dsi_cmd_mgr_quota_update(struct dsi_cmd_set *cmd_set)
{
	struct dsi_cmd_mgr *dsi_cmd_mgr = g_dsi_cmd_mgr;
	struct dsi_panel *panel;
	u32 pld_size, hdr_size, frame_pld_size, frame_hdr_size;

	if (!dsi_cmd_mgr || !cmd_set) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", dsi_cmd_mgr, cmd_set);
		return -EINVAL;
	}

	if (cmd_set->type == DSI_CMD_SET_ON || cmd_set->type == DSI_CMD_SET_OFF) {
		DSI_DEBUG("skip update quota for on/off cmd set\n");
		return 0;
	}

	panel = dsi_cmd_mgr->panel;
	dsi_cmd_mgr_lock(dsi_cmd_mgr);

	pld_size = dsi_cmd_mgr->payload_size;
	hdr_size = dsi_cmd_mgr->header_size;
	dsi_cmd_mgr->transfered_payload_size += cmd_set->payload_size;
	dsi_cmd_mgr->transfered_header_size += cmd_set->num_cmds;
	/* Too many command sets need to be sent, and some command sets will be delayed to the next frame. */
	frame_pld_size = pld_size + dsi_cmd_mgr->transfered_payload_size;
	frame_hdr_size = hdr_size + dsi_cmd_mgr->transfered_header_size;
	if (frame_pld_size > DSI_PLD_MEM_MAX || frame_hdr_size > DSI_HDR_MEM_MAX)
		DSI_WARN("exceed the hardware capacity, frame pld/hdr size: %d/%d\n",
				frame_pld_size, frame_hdr_size);

	DSI_DEBUG("tranfered pld/hdr size: %d/%d\n",
			dsi_cmd_mgr->transfered_payload_size,
			dsi_cmd_mgr->transfered_header_size);
	dsi_cmd_mgr_unlock(dsi_cmd_mgr);
	return 0;
}

int dsi_cmd_mgr_push(struct dsi_cmd_set *cmd_set, u8 transfer_type)
{
	struct dsi_cmd_mgr *dsi_cmd_mgr = g_dsi_cmd_mgr;
	struct dsi_cmd_queue_node *queue, new_node;
	struct dsi_panel *panel;
	u32 pld_size, hdr_size, frame_pld_size, frame_hdr_size;
	int i;

	if (!dsi_cmd_mgr || !cmd_set) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", dsi_cmd_mgr, cmd_set);
		return -EINVAL;
	}

	panel = dsi_cmd_mgr->panel;
	cmd_set->priority_level = cmd_sets_priority[cmd_set->type];
	dsi_cmd_mgr_lock(dsi_cmd_mgr);
	/* The cmd mgr queue is full ? */
	if (dsi_cmd_mgr_is_full_locked()) {
		DSI_ERROR("failed to push cmd %s, queue is full\n",
				dsi_panel_cmd_set_name_get(cmd_set->type));
		dsi_cmd_mgr_unlock(dsi_cmd_mgr);
		return -ENOMEM;
	}

	/* Get new payload size and header size. */
	queue = dsi_cmd_mgr->queue;
	pld_size = dsi_cmd_mgr->payload_size + cmd_set->payload_size;
	hdr_size = dsi_cmd_mgr->header_size + cmd_set->num_cmds;
	/* Too many command sets need to be sent, and some command sets will be delayed to the next frame. */
	frame_pld_size = pld_size + dsi_cmd_mgr->transfered_payload_size;
	frame_hdr_size = hdr_size + dsi_cmd_mgr->transfered_header_size;
	if (frame_pld_size > DSI_PLD_MEM_MAX || frame_hdr_size > DSI_HDR_MEM_MAX)
		DSI_WARN("exceed the hardware capacity, frame pld/hdr size: %d/%d\n",
				frame_pld_size, frame_hdr_size);

	if (dsi_cmd_mgr_is_empty_locked()) {
		queue[dsi_cmd_mgr->rear].cmd_set = cmd_set;
		queue[dsi_cmd_mgr->rear].transfer_type = transfer_type;
		queue[dsi_cmd_mgr->rear].num_delayed_frame = 0;
	} else {
		/* Insert new cmd set by it's priority. */
		i = dsi_cmd_mgr->rear;
		new_node.cmd_set = cmd_set;
		new_node.num_delayed_frame = 0;
		while (i != dsi_cmd_mgr->front &&
				dsi_cmd_priority_cmp(&new_node,
						&queue[(i - 1 + DSI_HDR_MEM_MAX) %
								DSI_HDR_MEM_MAX])) {
			queue[i] = queue[(i - 1 + DSI_HDR_MEM_MAX) %
					DSI_HDR_MEM_MAX];
			i = (i - 1 + DSI_HDR_MEM_MAX) % DSI_HDR_MEM_MAX;
		}
		queue[i].cmd_set = cmd_set;
		queue[i].transfer_type = transfer_type;
		queue[i].num_delayed_frame = 0;
	}

	dsi_cmd_mgr->rear = (dsi_cmd_mgr->rear + 1) % DSI_HDR_MEM_MAX;
	dsi_cmd_mgr->num_cmd_set++;
	dsi_cmd_mgr->payload_size = pld_size;
	dsi_cmd_mgr->header_size = hdr_size;
	DSI_DEBUG("push cmd:%s, priority:%d\n",
			dsi_panel_cmd_set_name_get(cmd_set->type),
			cmd_set->priority_level);
	dsi_cmd_mgr_status_dump();

	dsi_cmd_mgr_unlock(dsi_cmd_mgr);
	return 0;
}

static struct dsi_cmd_queue_node *dsi_cmd_mgr_pop_locked(void)
{
	struct dsi_cmd_mgr *dsi_cmd_mgr = g_dsi_cmd_mgr;
	struct dsi_cmd_queue_node *node = NULL;

	if (!dsi_cmd_mgr) {
		DSI_ERROR("invalid parameters, %pK\n", dsi_cmd_mgr);
		return NULL;
	}

	if (!dsi_cmd_mgr_is_empty_locked()) {
		node = &dsi_cmd_mgr->queue[dsi_cmd_mgr->front];
		dsi_cmd_mgr->front = (dsi_cmd_mgr->front + 1) % DSI_HDR_MEM_MAX;
		dsi_cmd_mgr->num_cmd_set--;
		dsi_cmd_mgr->payload_size -= node->cmd_set->payload_size;
		dsi_cmd_mgr->header_size -= node->cmd_set->num_cmds;
		// dsi_cmd_mgr_status_dump();
	}

	return node;
}

int dsi_cmd_mgr_flush(void)
{
	u32 transfered_payload, transfered_header;
	struct dsi_cmd_mgr *dsi_cmd_mgr = g_dsi_cmd_mgr;
	struct dsi_cmd_queue_node *next_node;
	struct dsi_panel *panel;
	bool updated = false;
	const char *cmd_set_name;
	int ret = 0;

	if (!dsi_cmd_mgr) {
		DSI_ERROR("invalid parameters, %pK\n", dsi_cmd_mgr);
		return -EINVAL;
	}

	dsi_cmd_mgr_lock(dsi_cmd_mgr);
	panel = dsi_cmd_mgr->panel;
	transfered_payload = dsi_cmd_mgr->transfered_payload_size;
	transfered_header = dsi_cmd_mgr->transfered_header_size;

	next_node = dsi_cmd_mgr_front_get_locked();
	if (next_node)
		updated = true;

	/* Send the CMDs in the queue in order of priority until the hardware buffer is exhausted. */
	while (next_node && (transfered_payload + next_node->cmd_set->payload_size) <= DSI_PLD_MEM_MAX &&
			(transfered_header + next_node->cmd_set->num_cmds) <= DSI_HDR_MEM_MAX) {
		/* Update cmd mgr info first, cause the duplicated cmd set will be deleted after used. */
		dsi_cmd_set_dump(next_node->cmd_set);
		cmd_set_name = dsi_panel_cmd_set_name_get(next_node->cmd_set->type);
		dsi_cmd_mgr_pop_locked();
		transfered_header += next_node->cmd_set->num_cmds;
		transfered_payload += next_node->cmd_set->payload_size;
		/* Send and delete the duplicated cmd set. */
		DSI_DEBUG("flush:%s, priority:%d, current quota:%d/%d(hdr/pld)\n",
				cmd_set_name,
				next_node->cmd_set->priority_level,
				transfered_header, transfered_payload);
		ret = __dsi_panel_cmd_set_send(panel, next_node->cmd_set, next_node->transfer_type);
		if (ret) {
			DSI_ERROR("failed to send cmd set, ret: %d\n", ret);
			goto exit;
		}

		if (next_node->cmd_set->delete_after_usage) {
			dsi_cmd_set_delete(next_node->cmd_set);
			DSI_DEBUG("delete %s after use\n", cmd_set_name);
		}
		next_node = dsi_cmd_mgr_front_get_locked();
		DSI_DEBUG("flush num cmds %d\n", transfered_header);
	}

exit:
	/* Updated the transfered payload size */
	dsi_cmd_mgr->transfered_payload_size = transfered_payload;
	dsi_cmd_mgr->transfered_header_size = transfered_header;
	if (updated)
		dsi_cmd_mgr_status_dump();

	dsi_cmd_mgr_unlock(dsi_cmd_mgr);
	return 0;
}

static int dsi_panel_backlight_cmd_set_send(struct dsi_panel *panel,
		u32 brightness, u8 transfer_type)
{
	enum dsi_cmd_set_type type = DSI_CMD_SET_BACKLIGHT;
	struct dsi_cmd_set *cmd_set;
	struct dsi_cmd_desc cmd = {0};
	u8 tx_buf[3] = {0x51, 0x00, 0x00};
#ifdef MI_DISPLAY_MODIFY
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;
#endif
	int ret = 0;

	if (unlikely(!panel->panel_info.current_mode)) {
		DSI_ERROR("current_mode is null\n");
		return -EINVAL;
	}

	cmd_set = &panel->panel_info.current_mode->cmd_sets[type];
	cmd_set->cmd_state = DSI_CMD_SET_STATE_HS;
	cmd_set->type = type;
	cmd_set->num_infos = 0;
	cmd_set->num_cmds = 1;
	cmd_set->port_index = DSI_PORT_MAX;
	cmd_set->cmds = &cmd;
	cmd_set->payload_size = 3;

	tx_buf[1] = (brightness & 0xFF00) >> 8;
	tx_buf[2] = brightness & 0xFF;
	cmd.msg.channel = 0;
	cmd.msg.type = 0x39;
	cmd.msg.flags = MIPI_DSI_MSG_USE_LPM;
	cmd.msg.tx_len = 3;
	cmd.msg.tx_buf = (const void *)tx_buf;
	cmd.msg.rx_len = 0;

	ret = dsi_panel_cmd_set_send(panel, type, cmd_set, transfer_type);
	if (ret)
		DSI_ERROR("failed to update DSI_CMD_SET_BACKLIGHT\n");

#ifdef MI_DISPLAY_MODIFY
	if (mi_cfg->xeq_enabled) {
		ret = dsi_panel_cmd_set_send(panel, DSI_CMD_SET_MI_XEQ_SYNC, NULL, transfer_type);
		if (ret)
			DSI_ERROR("failed to send 6C cmd\n");
	}
#endif

	return ret;
}

int dsi_panel_backlight_set_locked(struct dsi_panel *panel,
		u32 brightness, u8 transfer_type)
{
	int ret;

#ifdef MI_DISPLAY_MODIFY
	ret = mi_dsi_panel_backlight_prepare(panel, brightness);
	if (ret) {
		DSI_DEBUG("skip set panel backlight %d, ret %d\n", brightness, ret);
		return 0;
	}
#endif

	switch (panel->panel_info.bl_setting_type) {
	case DSI_PANEL_BL_UPDATE_METHOD_CMD:
		ret = dsi_panel_backlight_cmd_set_send(panel, brightness, transfer_type);
		if (ret)
			DSI_ERROR("filed to setting backlight by cmd\n");

		break;
	case DSI_PANEL_BL_UPDATE_METHOD_I2C:
		ret = ktz8866_backlight_update_status(brightness);
		if (ret)
			DSI_ERROR("filed to setting backlight by i2c\n");

		break;
	default:
		DSI_ERROR("get invalid backlight setting type\n");
		ret = -EINVAL;
		break;
	}

	if (!ret) {
#ifndef MI_DISPLAY_MODIFY
		DSI_INFO("set panel backlight as %d\n", brightness);
#endif
		panel->panel_info.bl_config.current_brightness = brightness;
	}
	if (brightness < 1000)
		set_sr_keypoint(SR_KEYPOINT_DPU_BACKLIGHT_0);
	else if (brightness < 2000)
		set_sr_keypoint(SR_KEYPOINT_DPU_BACKLIGHT_1000);
	else if (brightness < 3000)
		set_sr_keypoint(SR_KEYPOINT_DPU_BACKLIGHT_2000);
	else if (brightness < 4000)
		set_sr_keypoint(SR_KEYPOINT_DPU_BACKLIGHT_3000);
	else
		set_sr_keypoint(SR_KEYPOINT_DPU_BACKLIGHT_4000);

	return ret;
}

int dsi_panel_doze(struct dsi_panel *panel)
{
	int ret = 0;

	if (!panel) {
		DSI_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);

#ifdef MI_DISPLAY_MODIFY
	mi_dsi_panel_set_doze(panel);
#endif
	dsi_panel_event_notifier_trigger(DRM_PANEL_EVENT_BLANK_LP, panel);

	DPU_DEBUG("dsi panel enter doze");

	dsi_panel_unlock(panel);
	return ret;
}

int dsi_panel_doze_suspend(struct dsi_panel *panel)
{
	int ret = 0;

	if (!panel) {
		DSI_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);

#ifdef MI_DISPLAY_MODIFY
	mi_dsi_panel_set_doze_suspend(panel);
#endif
	dsi_panel_event_notifier_trigger(DRM_PANEL_EVENT_BLANK, panel);

	DPU_DEBUG("dsi panel enter doze suspend");

	dsi_panel_unlock(panel);
	return ret;
}

int dsi_panel_exit_lp(struct dsi_panel *panel)
{
	int ret = 0;

	if (!panel) {
		DSI_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);

#ifdef MI_DISPLAY_MODIFY
	mi_dsi_panel_exit_lp_locked(panel);
#endif
	dsi_panel_event_notifier_trigger(DRM_PANEL_EVENT_UNBLANK, panel);

	DPU_DEBUG("dsi panel exit low power");

	dsi_panel_unlock(panel);
	return ret;
}

int dsi_panel_always_on_display(struct dsi_panel *panel, bool enable)
{
	enum dsi_cmd_set_type type;
	int ret = 0;

	if (!panel) {
		DSI_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);

	type = enable ? DSI_CMD_SET_AOD_ON : DSI_CMD_SET_AOD_OFF;
	ret = dsi_panel_cmd_set_send(panel, type, NULL, USE_CPU);
	if (ret) {
		DSI_ERROR("failed to %s aod\n", enable ? "enter" : "exit");
		goto exit;
	}

	DPU_DEBUG("panel %s aod\n", enable ? "enter" : "exit");
exit:
	dsi_panel_unlock(panel);
	return ret;
}

static __maybe_unused int dsi_panel_te_scanline_update(struct dsi_panel *panel)
{
	struct dsi_cmd_set_update_info *info;
	struct dsi_panel_mode *current_mode;
	struct dsi_cmd_set *cmd_set;
	u8 te_scanline[2];
	int i;

	if (!panel || !panel->panel_info.current_mode) {
		DSI_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	current_mode = panel->panel_info.current_mode;
	cmd_set = &current_mode->cmd_sets[DSI_CMD_SET_ON];
	if (!cmd_set->num_infos || panel->panel_info.te_scanline < 0) {
		DSI_DEBUG("no update info or te_scanline %d\n",
				cmd_set->num_infos);
		return 0;
	}

	for (i = 0; i < cmd_set->num_infos; ++i) {
		info = &cmd_set->infos[i];
		if (info->addr == MIPI_DCS_SET_TE_SCANLINE) {
			te_scanline[0] = (panel->panel_info.te_scanline & 0xFF00) >> 8;
			te_scanline[1] = panel->panel_info.te_scanline & 0xFF;
			dsi_panel_cmd_set_update(cmd_set, i,
					te_scanline, sizeof(te_scanline));
			DSI_DEBUG("mipi addr: 0x%x, te_scanline: 0x%x\n",
					info->addr, (panel->panel_info.te_scanline & 0xFFFF));
		}
	}

	return 0;
}

int dsi_panel_enable(struct dsi_panel *panel)
{
	int ret = 0;

	if (!panel) {
		DSI_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);
	/* clear the panel dead pending status */
	atomic_set(&panel->panel_recovery_pending, 0);

	dsi_panel_te_scanline_update(panel);

	ret = dsi_panel_cmd_set_send(panel, DSI_CMD_SET_ON, NULL, USE_CPU);
	if (ret) {
		DSI_ERROR("failed to send DSI_CMD_SET_ON cmds\n");
		goto error;
	}

#ifdef MI_DISPLAY_MODIFY
	mi_dsi_panel_enable(panel);
#endif

	panel->initialized = true;
	panel->panel_dead = false;
	dsi_panel_unlock(panel);
	return 0;
error:
	panel->initialized = false;
	dsi_panel_unlock(panel);
	return ret;
}

int dsi_panel_disable(struct dsi_panel *panel)
{
	int ret = 0;

	if (!panel) {
		DSI_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);
	panel->initialized = false;
	ret = dsi_panel_cmd_set_send(panel, DSI_CMD_SET_OFF, NULL, USE_CPU);
	if (ret) {
		DSI_ERROR("failed to send DSI_CMD_SET_OFF cmds\n");
		goto error;
	}

#ifdef MI_DISPLAY_MODIFY
	mi_dsi_panel_disable(panel);
#endif

	dsi_panel_event_notifier_trigger(DRM_PANEL_EVENT_BLANK, panel);
	panel->panel_dead = false;
error:
	dsi_panel_unlock(panel);
	return ret;
}

void dsi_panel_timing_dump(struct dsi_panel_timing *panel_timing)
{
	if (!panel_timing) {
		DSI_ERROR("get null panel timing\n");
		return;
	}

	DSI_DEBUG("--------------------Panel Timing--------------------\n");
	DSI_DEBUG("Frame Rate:%d\n", panel_timing->framerate);
	DSI_DEBUG("Hact:%d\n", panel_timing->hdisplay);
	DSI_DEBUG("Vact:%d\n", panel_timing->vdisplay);
	DSI_DEBUG("HPW:%d\n", panel_timing->hpw);
	DSI_DEBUG("HFP:%d\n", panel_timing->hfp);
	DSI_DEBUG("HBP:%d\n", panel_timing->hbp);
	DSI_DEBUG("H Skew:%d\n", panel_timing->h_skew);
	DSI_DEBUG("VPW:%d\n", panel_timing->vpw);
	DSI_DEBUG("VFP:%d\n", panel_timing->vfp);
	DSI_DEBUG("VBP:%d\n", panel_timing->vbp);
	DSI_DEBUG("--------------------Panel Timing--------------------\n");
}

int dsi_panel_mode_validate(struct dsi_panel *panel,
		struct drm_display_mode *drm_mode)
{
	return 0;
}

static void __maybe_unused dsi_drm_mode_dump(struct drm_display_mode *drm_mode)
{
	DSI_DEBUG("drm mode name:%s\n", drm_mode->name);
	DSI_DEBUG("mode flags:%x\n", drm_mode->flags);

	DSI_DEBUG("hdisplay:%d\n", drm_mode->hdisplay);
	DSI_DEBUG("hsync_start:%d\n", drm_mode->hsync_start);
	DSI_DEBUG("hsync_end:%d\n", drm_mode->hsync_end);
	DSI_DEBUG("htotal:%d\n", drm_mode->htotal);
	DSI_DEBUG("hskew:%d\n", drm_mode->hskew);
	DSI_DEBUG("vdisplay:%d\n", drm_mode->vdisplay);
	DSI_DEBUG("vsync_start:%d\n", drm_mode->vsync_start);
	DSI_DEBUG("vsync_end:%d\n", drm_mode->vsync_end);
	DSI_DEBUG("vtotal:%d\n", drm_mode->vtotal);
	DSI_DEBUG("clock:%d\n", drm_mode->clock);
}

static int dsi_panel_convert_to_drm_mode(struct dsi_panel_mode *panel_mode,
		struct drm_display_mode *drm_mode)
{
	struct dsi_panel_timing *timing = &panel_mode->timing;

	memset(drm_mode, 0, sizeof(*drm_mode));

	drm_mode->hdisplay = timing->hdisplay;
	drm_mode->hsync_start = drm_mode->hdisplay + timing->hfp;
	drm_mode->hsync_end = drm_mode->hsync_start + timing->hpw;
	drm_mode->htotal = drm_mode->hsync_end + timing->hbp;
	drm_mode->hskew = timing->h_skew;

	drm_mode->vdisplay = timing->vdisplay;
	drm_mode->vsync_start = drm_mode->vdisplay + timing->vfp;
	drm_mode->vsync_end = drm_mode->vsync_start + timing->vpw;
	drm_mode->vtotal = drm_mode->vsync_end + timing->vbp;

	drm_mode->clock = drm_mode->htotal * drm_mode->vtotal * timing->framerate;
	drm_mode->clock /= 1000;

	drm_mode->flags |= (DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC);

	return 0;
}

int dsi_panel_get_drm_mode(struct dsi_panel *panel, u32 index,
		struct drm_display_mode *drm_mode)
{
	struct dsi_panel_info *panel_info;
	struct dsi_panel_mode *panel_mode;
	struct dsi_panel_timing *timing;

	if (!panel) {
		DSI_ERROR("invalid params\n");
		return -EINVAL;
	}

	panel_info = &panel->panel_info;

	panel_mode = &panel_info->modes[index];
	dsi_panel_convert_to_drm_mode(panel_mode, drm_mode);

	/* update timing for dual mipi */
	timing = panel_mode->cur_timing;
	if (panel_info->host_info.sbs_en) {
		drm_mode->hdisplay *= DSI_PORT_MAX;
		drm_mode->hsync_start = drm_mode->hdisplay + timing->hfp;
		drm_mode->hsync_end = drm_mode->hsync_start + timing->hpw;
		drm_mode->htotal = drm_mode->hsync_end + timing->hbp;

		drm_mode->clock = drm_mode->htotal * drm_mode->vtotal * timing->framerate;
		drm_mode->clock /= 1000;
	}

	drm_mode->width_mm = panel_info->physical_info.width_mm;
	drm_mode->height_mm = panel_info->physical_info.height_mm;
	/* set mode name */
	snprintf(drm_mode->name, DRM_DISPLAY_MODE_LEN, "%dx%dx%d",
			drm_mode->hdisplay, drm_mode->vdisplay,
			drm_mode_vrefresh(drm_mode));

	/* set the firset mode in dtsi as preferred mode */
	if (index == PREFERRED_MODE_INDEX) {
		drm_mode->type |= DRM_MODE_TYPE_PREFERRED;
		DSI_DEBUG("set drm mode %s as prefered\n", drm_mode->name);
	}

	return 0;
}

static int dsi_panel_parse_physical_info(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	struct dsi_panel_physical_info *physical_info =
			&panel_info->physical_info;
	const char *panel_type;
	int ret = 0;

	ret = of_property_read_string(np, "dsi-panel-physical-type",
			(const char **)&panel_type);

	DSI_DEBUG("get dsi-panel-physical-type: %s\n", panel_type);

	if (ret) {
		physical_info->type = DSI_PANEL_TYPE_LCD;
		DSI_WARN("set default panel physical type: lcd\n");
	} else if (panel_type && !strcmp(panel_type, "lcd"))
		physical_info->type = DSI_PANEL_TYPE_LCD;
	else if (panel_type && !strcmp(panel_type, "ltps"))
		physical_info->type = DSI_PANEL_TYPE_LTPS;
	else if (panel_type && !strcmp(panel_type, "ltpo"))
		physical_info->type = DSI_PANEL_TYPE_LTPO;
	else {
		DSI_WARN("invalid panel type %s, set as default lcd\n",
				panel_type);
		physical_info->type = DSI_PANEL_TYPE_LCD;
	}

	/**
	 * kernel is not friendly to floating-point arithmetic,
	 * the width_mm/height_mm parsed in here is multiplied by 10,
	 * the caller needs to divide by 10 when using it.
	 */
	ret = of_property_read_u32(np,
			"dsi-panel-physical-width-dimension",
			&physical_info->width_mm);
	if (ret) {
		DSI_WARN("dsi panel physical width is not defined, set as default 0\n");
		physical_info->width_mm = 0;
	} else {
		DSI_DEBUG("dsi panel physical width:%d\n",
				physical_info->width_mm);
	}

	ret = of_property_read_u32(np,
			"dsi-panel-physical-height-dimension",
			&physical_info->height_mm);
	if (ret) {
		DSI_ERROR("dsi panel physical height is not defined\n");
		physical_info->height_mm = 0;
		ret = 0;
	} else {
		DSI_DEBUG("dsi panel physical height:%d\n",
				physical_info->height_mm);
	}

	return 0;
}

static int dsi_panel_parse_bl_setting_type(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	const char *type;
	int ret = 0;

	ret = of_property_read_string(np,
			"dsi-panel-backlight-setting-type",
			(const char **)&type);

	if (ret) {
		panel_info->bl_setting_type = DSI_PANEL_BL_UPDATE_METHOD_CMD;
		DSI_WARN("get default backlight setting type: cmd\n");
		ret = 0;
		goto error;
	} else if (!strcmp(type, "cmd")) {
		panel_info->bl_setting_type = DSI_PANEL_BL_UPDATE_METHOD_CMD;
	} else if (!strcmp(type, "i2c")) {
		panel_info->bl_setting_type = DSI_PANEL_BL_UPDATE_METHOD_I2C;
	} else {
		panel_info->bl_setting_type = DSI_PANEL_BL_UPDATE_METHOD_CMD;
		DSI_ERROR("invalid backlight setting type, set as cmd: %s\n",
				type);
		goto error;
	}

	DSI_DEBUG("get panel backlight setting type:%s\n", type);
error:
	return ret;
}

static int dsi_panel_common_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	int ret = 0;

	panel_info->name = NULL;

	ret = of_property_read_string(np, "dsi-panel-name",
		(const char **)&panel_info->name);
	if (ret) {
		panel_info->name = "dsi_panel_default";
		DSI_WARN("set dsi-panel-name as default: dsi_panel_default\n");
	} else {
		if (!strcmp(panel_info->name, "fake_dsi_panel"))
			panel_info->is_fake_panel = true;
		else
			panel_info->is_fake_panel = false;
		DSI_INFO("dsi-panel-name: %s, is_fake_panel: %d\n",
		panel_info->name, panel_info->is_fake_panel);
	}

	ret = of_property_read_u32(np, "dsi-panel-id",
		&panel_info->panel_id);
	if (ret) {
		panel_info->panel_id = 0;
		DSI_WARN("set dsi panel id as default: 0\n");
	} else {
		DSI_DEBUG("get panel id: %d\n", panel_info->panel_id);
	}

	ret = of_property_read_u32(np, "dsi-panel-idle-policy",
		&panel_info->idle_policy);
	if (ret) {
		panel_info->idle_policy = 0;
		DSI_WARN("set dsi panel idle policy as default: 0\n");
	} else {
		DSI_DEBUG("get idle policy: 0x%x\n", panel_info->idle_policy);
	}

	ret = of_property_read_u32(np, "dsi-panel-lowpower-ctrl",
		&panel_info->lowpower_ctrl);
	if (ret) {
		panel_info->lowpower_ctrl = 0;
		DSI_WARN("set dsi panel lowpower ctrl as default: 0\n");
	} else {
		DSI_DEBUG("get lowpower ctrl: 0x%x\n", panel_info->lowpower_ctrl);
	}

	ret = of_property_read_u32(np, "dsi-panel-frame-power-mask",
		&panel_info->frame_power_mask);
	if (ret) {
		panel_info->frame_power_mask = 0;
		DSI_WARN("set dsi panel frame power mask as default: 0\n");
	} else {
		DSI_DEBUG("get frame power mask: 0x%x\n", panel_info->frame_power_mask);
	}

	ret = of_property_read_u32(np, "dsi-panel-frame-power-max-refresh-rate",
		&panel_info->frame_power_max_refresh_rate);
	if (ret) {
		panel_info->frame_power_max_refresh_rate = 0;
		DSI_WARN("set dsi panel frame power max refresh rate as default: 0\n");
	} else {
		DSI_DEBUG("get frame power max refresh rate: 0x%x\n",
				panel_info->frame_power_max_refresh_rate);
	}

	ret = of_property_read_u32(np, "dsi-panel-hw-ulps-max-refresh-rate",
		&panel_info->hw_ulps_max_refresh_rate);
	if (ret) {
		panel_info->hw_ulps_max_refresh_rate = 0;
		DSI_DEBUG("set hw_ulps_max_refresh_rate as default: 0\n");
	} else {
		DSI_DEBUG("get hw_ulps_max_refresh_rate: 0x%x\n",
				panel_info->hw_ulps_max_refresh_rate);
	}

	ret = of_property_read_u32(np, "dsi-panel-boot-up-profile",
		&panel_info->boot_up_profile);
	if (ret) {
		panel_info->boot_up_profile = 0;
		DSI_WARN("set dsi panel boot up profile as default: 0\n");
	} else {
		DSI_DEBUG("get boot up profile: %d\n", panel_info->boot_up_profile);
	}

	ret = dsi_panel_parse_bl_setting_type(panel_info, np);
	if (ret)
		DSI_ERROR("failed to get panel backlight setting type\n");

	ret = dsi_panel_parse_physical_info(panel_info, np);
	if (ret)
		DSI_ERROR("failed to get panel physical type\n");

	ret = dpu_backlight_parse_bl_info(&panel_info->bl_config, np);
	if (ret) {
		DSI_ERROR("failed to get panel backlight info\n");
		ret = 0;
	}

	panel_info->support_aod = of_property_read_bool(np, "dsi-panel-aod-enable");

	panel_info->partial_update_enable = of_property_read_bool(np, "partial-update-enabled");

	DSI_DEBUG("get aod support:%d, partial_update_enable:%d\n",
		panel_info->support_aod, panel_info->partial_update_enable);

	return ret;
}

static int dsi_panel_num_modes_get(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	const char *fps_list_name = "dsi-panel-supported-fps-list";
	struct device_node *timings_np, *child_timing_np;
	int num_timings = 0, num_modes = 0, ret = 0;
	int fps_list_size;

	panel_info->num_timings = 0;
	panel_info->num_modes = 0;

	timings_np = of_find_node_by_name(np, "dsi-display-timings");
	if (!timings_np) {
		DSI_ERROR("failed to find display timing nodes\n");
		ret = -EINVAL;
		goto exit;
	}

	num_timings = of_get_available_child_count(timings_np);
	if (num_timings <= 0 || num_timings > DSI_TIMING_MAX) {
		DSI_ERROR("get %d timings, max timing num %d\n",
				num_timings, DSI_TIMING_MAX);
		return -EINVAL;
	}

	num_modes = num_timings;
	for_each_available_child_of_node(timings_np, child_timing_np) {
		fps_list_size = of_property_count_u32_elems(child_timing_np,
				fps_list_name);
		if (fps_list_size < 0 || fps_list_size > MAX_FPS_LIST_SIZE) {
			DSI_DEBUG("fps_list_size undefined or invalid %d\n",
					fps_list_size);
			of_node_put(child_timing_np);
			continue;
		}

		num_modes += fps_list_size;
		DSI_DEBUG("get fps_list_size %d, current num modes %d\n",
				fps_list_size, num_modes);
		if (num_modes > DSI_MODE_MAX) {
			DSI_ERROR("get %d modes, max modes num %d\n",
					num_modes, DSI_MODE_MAX);
			ret = -EINVAL;
			of_node_put(child_timing_np);
			goto exit;
		}
		of_node_put(child_timing_np);
	}


	panel_info->num_timings = num_timings;
	panel_info->num_modes = num_modes;
	DSI_INFO("get %d timings, %d modes\n", num_timings, num_modes);

exit:
	of_node_put(timings_np);
	return ret;
}

static int dsi_panel_timing_parse(enum dsi_ctrl_mode ctrl_mode,
		struct dsi_panel_timing *timing, struct device_node *np)
{
	int ret = 0;

	ret = of_property_read_u32(np, "dsi-panel-frame-rate",
		&timing->framerate);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-frame-rate!\n");
		return ret;
	}

	if (ctrl_mode == DSI_CMD_MODE) {
		ret = of_property_read_u32(np, "dsi-panel-te-rate",
			&timing->te_rate);
		if (ret) {
			DSI_ERROR("failed to get dsi-panel-te-rate!\n");
			return ret;
		}
	} else {
		timing->te_rate = timing->framerate;
	}

	ret = of_property_read_u32(np, "dsi-panel-base-rate",
		&timing->base_rate);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-base-rate!\n");
		timing->base_rate = timing->framerate;
	}

	ret = of_property_read_u32(np, "dsi-panel-h-front-porch",
		&timing->hfp);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-h-front-porch!\n");
		return ret;
	}

	ret = of_property_read_u32(np, "dsi-panel-h-back-porch",
		&timing->hbp);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-h-back-porch!\n");
		return ret;
	}

	ret = of_property_read_u32(np, "dsi-panel-h-pulse-width",
		&timing->hpw);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-h-pulse-width!\n");
		return ret;
	}

	ret = of_property_read_u32(np, "dsi-panel-h-sync-skew",
		&timing->h_skew);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-h-sync-skew!\n");
		return ret;
	}

	ret = of_property_read_u32(np, "dsi-panel-v-back-porch",
		&timing->vbp);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-v-back-porch!\n");
		return ret;
	}

	ret = of_property_read_u32(np, "dsi-panel-v-front-porch",
		&timing->vfp);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-v-front-porch!\n");
		return ret;
	}

	ret = of_property_read_u32(np, "dsi-panel-v-pulse-width",
		&timing->vpw);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-v-pulse-width!\n");
		return ret;
	}

	return 0;
}

static int dsi_panel_timing_bak_parse(struct dsi_panel_mode *mode,
		struct device_node *np,  struct dsi_panel_mode *def_mode)
{
	int i, ret = 0;
	u32 tmp_array[DSI_MIPI_DYN_FREQ_MAX] = {0};

	if (mode == def_mode) {
		mode->timing_baks_num =
				of_property_count_u32_elems(np, "dsi-panel-h-front-porch-bak");
	} else {
		mode->timing_baks_num = def_mode->timing_baks_num;
	}
	if (mode->timing_baks_num > DSI_MIPI_DYN_FREQ_MAX) {
		DSI_DEBUG("failed to count dsi-panel-h-front-porch-bak\n");
		mode->timing_baks_num = 0;
		goto error;
	}

	DSI_DEBUG("panel timing baks num %d\n", mode->timing_baks_num);
	if (mode->timing_baks_num == 0)
		/* there is no dsi-panel-h-front-porch-bak node if doesnt support
		 * mipi dynamic frequency
		 */
		goto error;

	for (i = 0; i < mode->timing_baks_num; i++)
		mode->timing_baks[i] = mode->timing;

	if (mode == def_mode) {
		ret = of_property_read_u32_array(np, "dsi-panel-h-front-porch-bak",
				tmp_array, mode->timing_baks_num);
		if (ret) {
			DSI_ERROR("failed to read dsi-panel-h-front-porch-bak\n");
			goto error;
		}
		for (i = 0; i < mode->timing_baks_num; i++) {
			mode->timing_baks[i].hfp = tmp_array[i];
			DSI_DEBUG("timing_baks[%d].hfp %d\n", i, mode->timing_baks[i].hfp);
		}
	} else {
		for (i = 0; i < mode->timing_baks_num; i++) {
			mode->timing_baks[i].hfp = def_mode->timing_baks[i].hfp;
			DSI_DEBUG("timing_baks[%d].hfp %d\n", i, mode->timing_baks[i].hfp);
		}
	}

error:
	return ret;
}

int dsi_panel_get_cmd_pkt_count(const char *cmd_data, u32 length,
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
			DSI_ERROR("command format error\n");
			return -EINVAL;
		}

		length -= packet_length;
		cmd_data += packet_length;
		count++;
	}

	*pkt_count = count;
	DSI_DEBUG("get cmd pkt count:%d\n", count);

	return 0;
}

int dsi_panel_create_cmd_packets(const char *data, u32 length,
		u32 count, struct dsi_cmd_desc *cmd)
{
	int rc = 0;
	int i, j;
	u8 *payload;

	if (length < CMD_SET_MIN_SIZE) {
		DSI_ERROR("invalid params\n");
		return -EINVAL;
	}

	for (i = 0; i < count; i++) {
		u32 size;

		cmd[i].msg.type = data[0];
		cmd[i].msg.channel = data[2];
		cmd[i].msg.flags |= data[3];
		cmd[i].msg.tx_len = ((data[5] << 8) | (data[6]));
		/* convert ms to us */
		cmd[i].post_wait_us = data[4] * 1000;

		size = cmd[i].msg.tx_len * sizeof(u8);

		if (size + CMD_SET_MIN_SIZE > length) {
			DSI_ERROR("invalid params\n");
			return -EINVAL;
		}

		payload = kzalloc(size, GFP_KERNEL);
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
		kfree(cmd->msg.tx_buf);
	}

	return rc;
}

int dsi_panel_cmds_parse(struct dsi_cmd_set *cmd_set,
		struct device_node *np, const char *cmd_name)
{
	int raw_data_size = 0;
	char *raw_data;
	u32 num_cmds = 0;
	int ret = 0;
	int i;

	if (!np || !cmd_name) {
		DSI_ERROR("invalid params\n");
		ret = -EINVAL;
		goto error;
	}

	/* get cmd_set set source data */
	raw_data_size = of_property_count_u8_elems(np, cmd_name);
	if (raw_data_size < 1) {
		DSI_DEBUG("empty cmd:%s\n", cmd_name);
		ret = -EINVAL;
		goto error;
	}
	DSI_DEBUG("get cmd data size:%d\n", raw_data_size);

	raw_data = kcalloc(raw_data_size, sizeof(u8), GFP_KERNEL);
	if (!raw_data)
		return -ENOMEM;

	ret = of_property_read_u8_array(np, cmd_name, raw_data, raw_data_size);
	if (ret) {
		DSI_ERROR("failed to get cmd set: %s raw data\n", cmd_name);
		goto error_release_raw_data;
	}

	ret = dsi_panel_get_cmd_pkt_count(raw_data, raw_data_size, &num_cmds);
	if (ret) {
		DSI_ERROR("failed to get command packet count!\n");
		goto error_release_raw_data;
	}

	DSI_DEBUG("%s: num_cmds: %d, sizeof(*cmds): %lu, total alloc size: %lu\n", cmd_name, num_cmds,
			sizeof(*cmd_set->cmds), num_cmds * sizeof(*cmd_set->cmds));
	cmd_set->cmds = kcalloc(num_cmds, sizeof(*cmd_set->cmds), GFP_KERNEL);
	if (!cmd_set->cmds)
		goto error_release_raw_data;

	cmd_set->num_cmds = num_cmds;
	ret = dsi_panel_create_cmd_packets(raw_data, raw_data_size,
			num_cmds, cmd_set->cmds);
	if (ret) {
		DSI_ERROR("failed to create cmd packets\n");
		goto error_release_cmds;
	}

	cmd_set->payload_size = 0;
	for (i = 0; i < cmd_set->num_cmds; i++) {
		cmd_set->payload_size += cmd_set->cmds[i].msg.tx_len;
		if (((uint8_t *)cmd_set->cmds[i].msg.tx_buf)[0] == MIPI_DCS_SET_TE_SCANLINE)
			DSI_DEBUG("mipi addr 0x44 index: %d\n", i);
	}
	DSI_DEBUG("cmd %s payload_size %d\n", cmd_name, cmd_set->payload_size);

	kfree(raw_data);
	return 0;

error_release_cmds:
	kfree(cmd_set->cmds);
error_release_raw_data:
	kfree(raw_data);
error:
	cmd_set->cmds = NULL;
	cmd_set->num_cmds = 0;
	return ret;
}


static int dsi_panel_cmd_update_infos_parse(
		struct dsi_cmd_set *cmd_set, struct device_node *np,
		const char *cmd_update_name)
{
	struct dsi_cmd_set_update_info *infos;
	int data_length, num_infos;
	u32 *data_buffer = NULL;
	int i, ret = 0;

	if (!cmd_update_name) {
		DSI_DEBUG("get null cmd update name\n");
		ret = -EINVAL;
		goto error;
	}

	data_length = of_property_count_u32_elems(np, cmd_update_name);
	DSI_DEBUG("%s: data_length: %d\n", cmd_update_name, data_length);
	if (data_length <= 0) {
		DSI_DEBUG("%s not defined\n", cmd_update_name);
		goto error;
	}

	if (data_length % UPDATE_INFO_MEMBER) {
		DSI_ERROR("[%s] invalid data lenght:%d\n",
				cmd_update_name, data_length);
		ret = -EINVAL;
		goto error;
	}

	DSI_DEBUG("data_length: %d, sizeof(*data_buffer): %lu",
			data_length, sizeof(*data_buffer));
	data_buffer = kcalloc(data_length, sizeof(*data_buffer), GFP_KERNEL);
	if (!data_buffer) {
		ret = -ENOMEM;
		goto error;
	}

	num_infos = data_length / UPDATE_INFO_MEMBER;
	DSI_DEBUG("num_infos: %d, sizeof(*infos): %lu",
			num_infos, sizeof(*infos));
	infos = kcalloc(num_infos, sizeof(*infos), GFP_KERNEL);
	if (!infos) {
		ret = ENOMEM;
		goto error_release_buffer;
	}

	ret = of_property_read_u32_array(np, cmd_update_name,
			data_buffer, data_length);
	if (ret) {
		DSI_ERROR("failed to get u32 array, prop name:%s\n",
				cmd_update_name);
		goto error_release_infos;
	}

	DSI_DEBUG("get cmd set update:%s\n", cmd_update_name);
	for (i = 0; i < num_infos; i++) {
		infos[i].addr = data_buffer[i * UPDATE_INFO_MEMBER];
		infos[i].index = data_buffer[i * UPDATE_INFO_MEMBER + 1];
		infos[i].len = data_buffer[i * UPDATE_INFO_MEMBER + 2];
		DSI_DEBUG("[%d-th info] index:%d, addr:0x%x, len:%d\n", i,
			infos[i].index, infos[i].addr, infos[i].len);
	}

	cmd_set->num_infos = num_infos;
	cmd_set->infos = infos;
	kfree(data_buffer);
	return 0;

error_release_infos:
	kfree(infos);
error_release_buffer:
	kfree(data_buffer);
error:
	cmd_set->num_infos = 0;
	cmd_set->infos = NULL;
	return ret;
}

static inline void dsi_panel_mode_destroy(struct dsi_panel_mode *mode)
{
	struct dsi_cmd_set *cmd_set;
	int i;

	if (!mode || mode->is_flattened_mode)
		return;

	for (i = 0; i < DSI_CMD_SET_MAX; i++) {
		cmd_set = &mode->cmd_sets[i];
		dsi_cmd_set_content_free(cmd_set);
	}

	kfree(mode->flat_mode_infos);
}

void dsi_panel_modes_destroy(struct dsi_panel *panel)
{
	struct dsi_panel_info *panel_info;
	struct dsi_panel_mode *modes;
	int i;

	if (!panel)
		return;

	panel_info = &panel->panel_info;
	modes = panel_info->modes;

	for (i = 0; i < panel_info->num_modes; i++)
		dsi_panel_mode_destroy(&modes[i]);

	kfree(panel_info->modes);
}

/* The cmd set name should be added here when adding a new cmd set.
 * Note: DSI_CMD_SET_DAMAGE_AREA and DSI_CMD_SET_BACKLIGHT not parsed from DTSI,
 * generated dynamically.
 */
const char *cmd_set_names[DSI_CMD_SET_MAX] = {
	"dsi-panel-on-command",
	"dsi-panel-off-command",
	"dsi-panel-roi-command(generated dynamically)",
	"dsi-panel-timing-switch-command",
	"dsi-panel-post-timing-switch-command",
	"dsi-panel-flat-mode-on-command",
	"dsi-panel-flat-mode-off-command",
	"dsi-panel-backlight-command(generated dynamically)",
	"dsi-panel-aod-on-command",
	"dsi-panel-aod-off-command",
	"dsi-panel-status-command",
#ifdef MI_DISPLAY_MODIFY
	"mi,dsi-panel-dimmingon-command",
	"mi,dsi-panel-dimmingoff-command",
	"mi,dsi-panel-apldimming-command",
	"mi,dsi-panel-hbm-on-command",
	"mi,dsi-panel-hbm-off-command",
	"mi,dsi-panel-nolp-command",
	"mi,dsi-panel-doze-hbm-command",
	"mi,dsi-panel-doze-lbm-command",
	"mi,dsi-panel-doze-hbm-nolp-command",
	"mi,dsi-panel-doze-lbm-nolp-command",
	"mi,dsi-panel-flat-mode-on-command",
	"mi,dsi-panel-flat-mode-off-command",
	"mi,dsi-panel-flat-mode-on-sec-command",
	"mi,dsi-panel-flat-mode-off-sec-command",
	"mi,dsi-panel-flat-mode-read-pre-command",
	"mi,dsi-panel-flat-mode-off-read-pre-command",
	"mi,dsi-panel-dc-on-command",
	"mi,dsi-panel-dc-off-command",
	"mi,dsi-panel-round-corner-on-command",
	"mi,dsi-panel-round-corner-on-port0-command",
	"mi,dsi-panel-round-corner-on-port1-command",
	"mi,dsi-panel-round-corner-off-command",
	"mi,dsi-panel-timing-switch-sec-command",
	"mi,dsi-panel-doze-to-off-command",
	"mi,dsi-panel-doze-param-read-state",
	"mi,dsi-panel-doze-param-read-end-state",
	"mi,dsi-panel-status-offset-command",
	"mi,dsi-panel-status-after-command",
	"mi,dsi-panel-build-id-read-command",
	"mi,dsi-panel-build-id-sub-write1-command",
	"mi,dsi-panel-build-id-sub-write2-command",
	"mi,dsi-panel-build-id-sub-read-command",
	"mi,dsi-panel-cell-id-read-command",
	"mi,dsi-panel-cell-id-read-pre-tx-command",
	"mi,dsi-panel-cell-id-read-after-tx-command",
	"mi,dsi-panel-wp-read-command",
	"mi,dsi-panel-wp-read-pre-tx-command",
	"mi,dsi-panel-flatmode-status-command",
	"mi,dsi-panel-flatmode-status-offset-command",
	"mi,dsi-panel-flatmode-status-offset-end-command",
	"mi,dsi-panel-timing-switch-from-auto-mode-command",
	"mi,dsi-panel-timing-switch-from-skip-mode-command",
	"mi,dsi-panel-timing-switch-from-normal-mode-command",
	"mi,dsi-panel-demura-hbm-command",
	"mi,dsi-panel-demura-lbm-command",
	"mi,dsi-panel-csc-by-temper-comp-off-mode-command",
	"mi,dsi-panel-csc-by-temper-comp-32-36-mode-command",
	"mi,dsi-panel-csc-by-temper-comp-36-40-mode-command",
	"mi,dsi-panel-csc-by-temper-comp-40-mode-command",
	"mi,dsi-panel-csc-by-temper-comp-45-mode-command",
	"mi,dsi-panel-xeq-sync-command"
#endif
};

/* The cmd set state name should be added here when adding a new cmd set.
 * Note: DSI_CMD_SET_DAMAGE_AREA and DSI_CMD_SET_BACKLIGHT not parsed from DTSI,
 * generated dynamically.
 */
const char *cmd_set_state_names[DSI_CMD_SET_MAX] = {
	"dsi-panel-on-command-state",
	"dsi-panel-off-command-state",
	"dsi-panel-roi-command-state(generated dynamically)",
	"dsi-panel-timing-switch-command-state",
	"dsi-panel-post-timing-switch-command-state",
	"dsi-panel-flat-mode-on-command-state",
	"dsi-panel-flat-mode-off-command-state",
	"dsi-panel-backlight-command-state(generated dynamically)",
	"dsi-panel-aod-on-command-state",
	"dsi-panel-aod-off-command-state",
	"dsi-panel-status-command-state",
#ifdef MI_DISPLAY_MODIFY
	"mi,dsi-panel-dimmingon-command-state",
	"mi,dsi-panel-dimmingoff-command-state",
	"mi,dsi-panel-apldimming-command-state",
	"mi,dsi-panel-hbm-on-command-state",
	"mi,dsi-panel-hbm-off-command-state",
	"mi,dsi-panel-nolp-command-state",
	"mi,dsi-panel-doze-hbm-command-state",
	"mi,dsi-panel-doze-lbm-command-state",
	"mi,dsi-panel-doze-hbm-nolp-command-state",
	"mi,dsi-panel-doze-lbm-nolp-command-state",
	"mi,dsi-panel-flat-mode-on-command-state",
	"mi,dsi-panel-flat-mode-off-command-state",
	"mi,dsi-panel-flat-mode-on-sec-command-state",
	"mi,dsi-panel-flat-mode-off-sec-command-state",
	"mi,dsi-panel-flat-mode-read-pre-command-state",
	"mi,dsi-panel-flat-mode-off-read-pre-command-state",
	"mi,dsi-panel-dc-on-command-state",
	"mi,dsi-panel-dc-off-command-state",
	"mi,dsi-panel-round-corner-on-command-state",
	"mi,dsi-panel-round-corner-on-port0-command-state",
	"mi,dsi-panel-round-corner-on-port1-command-state",
	"mi,dsi-panel-round-corner-off-command-state",
	"mi,dsi-panel-timing-switch-sec-command-state",
	"mi,dsi-panel-doze-to-off-command-state",
	"mi,dsi-panel-doze-param-read-state-state",
	"mi,dsi-panel-doze-param-read-end-state-state",
	"mi,dsi-panel-status-offset-command-state",
	"mi,dsi-panel-status-after-command-state",
	"mi,dsi-panel-build-id-read-command-state",
	"mi,dsi-panel-build-id-sub-write1-command-state",
	"mi,dsi-panel-build-id-sub-write2-command-state",
	"mi,dsi-panel-build-id-sub-read-command-state",
	"mi,dsi-panel-cell-id-read-command-state",
	"mi,dsi-panel-cell-id-read-pre-tx-command-state",
	"mi,dsi-panel-cell-id-read-after-tx-command-state",
	"mi,dsi-panel-wp-read-command-state",
	"mi,dsi-panel-wp-read-pre-tx-command-state",
	"mi,dsi-panel-flatmode-status-command-state",
	"mi,dsi-panel-flatmode-status-offset-command-state",
	"mi,dsi-panel-flatmode-status-offset-end-command-state",
	"mi,dsi-panel-timing-switch-from-auto-mode-command-state",
	"mi,dsi-panel-timing-switch-from-skip-mode-command-state",
	"mi,dsi-panel-timing-switch-from-normal-mode-command-state",
	"mi,dsi-panel-demura-hbm-command-state",
	"mi,dsi-panel-demura-lbm-command-state",
	"mi,dsi-panel-csc-by-temper-comp-off-mode-command-state",
	"mi,dsi-panel-csc-by-temper-comp-32-36-mode-command-state",
	"mi,dsi-panel-csc-by-temper-comp-36-40-mode-command-state",
	"mi,dsi-panel-csc-by-temper-comp-40-mode-command-state",
	"mi,dsi-panel-csc-by-temper-comp-45-mode-command-state",
	"mi,dsi-panel-xeq-sync-command-state"
#endif
};

/* The cmd set update name should be added here when adding a new cmd set update.
 * Note: DSI_CMD_SET_DAMAGE_AREA and DSI_CMD_SET_BACKLIGHT not parsed from DTSI,
 * generated dynamically.
 */
const char *cmd_set_update_names[DSI_CMD_SET_MAX] = {
	"dsi-panel-on-command-update",
	(const char *)NULL,
	(const char *)NULL,
	"dsi-panel-timing-switch-command-update",
	"dsi-panel-post-timing-switch-command-update",
	"dsi-panel-flat-mode-on-command-update",
	"dsi-panel-flat-mode-off-command-update",
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	"dsi-panel-status-command-update",
#ifdef MI_DISPLAY_MODIFY
	"mi,dsi-panel-dimmingon-command-update",
	"mi,dsi-panel-dimmingoff-command-update",
	"mi,dsi-panel-apldimming-command-update",
	"mi,dsi-panel-hbm-on-command-update",
	"mi,dsi-panel-hbm-off-command-update",
	"mi,dsi-panel-nolp-command-update",
	"mi,dsi-panel-doze-hbm-command-update",
	"mi,dsi-panel-doze-lbm-command-update",
	"mi,dsi-panel-doze-hbm-nolp-command-update",
	"mi,dsi-panel-doze-lbm-nolp-command-update",
	"mi,dsi-panel-flat-mode-on-command-update",
	"mi,dsi-panel-flat-mode-off-command-update",
	"mi,dsi-panel-flat-mode-on-sec-command-update",
	"mi,dsi-panel-flat-mode-off-sec-command-update",
	"mi,dsi-panel-flat-mode-read-pre-command-update",
	"mi,dsi-panel-flat-mode-off-read-pre-command-update",
	"mi,dsi-panel-dc-on-command-update",
	"mi,dsi-panel-dc-off-command-update",
	"mi,dsi-panel-round-corner-on-command-update",
	"mi,dsi-panel-round-corner-on-port0-command-update",
	"mi,dsi-panel-round-corner-on-port1-command-update",
	"mi,dsi-panel-round-corner-off-command-update",
	"mi,dsi-panel-timing-switch-sec-command-update",
	"mi,dsi-panel-doze-to-off-command-update",
	"mi,dsi-panel-doze-param-read-state-update",
	"mi,dsi-panel-doze-param-read-end-state-update",
	"mi,dsi-panel-status-offset-command-update",
	"mi,dsi-panel-status-after-command-update",
	"mi,dsi-panel-build-id-read-command-update",
	"mi,dsi-panel-build-id-sub-write1-command-update",
	"mi,dsi-panel-build-id-sub-write2-command-update",
	"mi,dsi-panel-build-id-sub-read-command-update",
	"mi,dsi-panel-cell-id-read-command-update",
	"mi,dsi-panel-cell-id-read-pre-tx-command-update",
	"mi,dsi-panel-cell-id-read-after-tx-command-update",
	"mi,dsi-panel-wp-read-command-update",
	"mi,dsi-panel-wp-read-pre-tx-command-update",
	"mi,dsi-panel-flatmode-status-command-update",
	"mi,dsi-panel-flatmode-status-offset-command-update",
	"mi,dsi-panel-flatmode-status-offset-end-command-update",
	"mi,dsi-panel-timing-switch-from-auto-mode-command-update",
	"mi,dsi-panel-timing-switch-from-skip-mode-command-update",
	"mi,dsi-panel-timing-switch-from-normal-mode-command-update",
	"mi,dsi-panel-demura-hbm-command-update",
	"mi,dsi-panel-demura-lbm-command-update",
	"mi,dsi-panel-csc-by-temper-comp-off-mode-command-update",
	"mi,dsi-panel-csc-by-temper-comp-32-36-mode-command-update",
	"mi,dsi-panel-csc-by-temper-comp-36-40-mode-command-update",
	"mi,dsi-panel-csc-by-temper-comp-40-mode-command-update",
	"mi,dsi-panel-csc-by-temper-comp-45-mode-command-update",
	"mi,dsi-panel-xeq-sync-command-update"
#endif
};

/* The cmd set ctrl index name should be added here when adding a new cmd set update.
 * Note: DSI_CMD_SET_DAMAGE_AREA and DSI_CMD_SET_BACKLIGHT not parsed from DTSI,
 * generated dynamically.
 */
const char *cmd_set_port_index_names[DSI_CMD_SET_MAX] = {
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
#ifdef MI_DISPLAY_MODIFY
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	"mi,dsi-panel-round-corner-on-port0-command-port",
	"mi,dsi-panel-round-corner-on-port1-command-port",
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL,
	(const char *)NULL
#endif
};

const char *dsi_panel_cmd_set_name_get(int type)
{
	if (type < 0 || type >= DSI_CMD_SET_MAX)
		return "invalid cmd set type";
	return cmd_set_names[type];
}

static int dsi_panel_cmd_sets_parse(struct dsi_cmd_set *cmd_sets,
		struct device_node *np)
{
	struct dsi_cmd_set *cmd_set;
	u32 i, ret = 0;

	if (!cmd_sets || !np) {
		DSI_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	for (i = DSI_CMD_SET_ON; i < DSI_CMD_SET_MAX; i++) {
		cmd_set = &cmd_sets[i];
		ret = dsi_panel_cmd_single_parse(cmd_set, i, np);
		if (ret) {
			DSI_ERROR("failed to parse cmd set: %s\n",
					cmd_set_state_names[i]);
			continue;
		}
	}
	return 0;
}

static void dsi_panel_cmd_port_index_info_parse(
		struct dsi_cmd_set *cmd_set, struct device_node *np,
		const char *cmd_port_index_name)
{
	int port_index, ret, i;

	if (!cmd_port_index_name) {
		DSI_DEBUG("get null cmd ctrl index name\n");
		port_index = DSI_PORT_MAX;
		goto exit;
	}

	ret = of_property_read_u32(np, cmd_port_index_name, &port_index);
	if (ret) {
		DSI_DEBUG("%s property is null\n", cmd_port_index_name);
		port_index = DSI_PORT_MAX;
		goto exit;
	}
	if (port_index < 0 || port_index > DSI_PORT_MAX) {
		DSI_INFO("get invald %s:%d, ret = %d\n",
				cmd_port_index_name, port_index, ret);
		port_index = DSI_PORT_MAX;
	}

exit:
	cmd_set->port_index = port_index;
	for (i = 0; i < cmd_set->num_cmds; ++i)
		cmd_set->cmds[i].port_index = cmd_set->port_index;

	DSI_DEBUG("get cmd port_index %s: %d\n", cmd_port_index_name, port_index);
}

int dsi_panel_cmd_single_parse(struct dsi_cmd_set *cmd_set,
		enum dsi_cmd_set_type cmd_type, struct device_node *np)
{
	const char *cmd_state = NULL;
	u32 ret = 0;

	DSI_DEBUG("cmd set:%s\n", cmd_set_names[cmd_type]);

	/* parse single cmd set */
	ret = dsi_panel_cmds_parse(cmd_set, np, cmd_set_names[cmd_type]);
	if (ret) {
		DSI_DEBUG("empty cmd set:%s\n", cmd_set_names[cmd_type]);
		return 0;
	}
	cmd_set->type = cmd_type;

	/* parse single cmd set state */
	ret = of_property_read_string(np, cmd_set_state_names[cmd_type],
			(const char **)&cmd_state);
	if (ret) {
		DSI_WARN("set cmd: %s state as default lp mode\n",
				cmd_set_names[cmd_type]);
		cmd_set->cmd_state = DSI_CMD_SET_STATE_LP;
	} else if (!strcmp(cmd_state, "dsi_hs_mode")) {
		cmd_set->cmd_state = DSI_CMD_SET_STATE_HS;
	} else {
		cmd_set->cmd_state = DSI_CMD_SET_STATE_LP;
	}
	DSI_DEBUG("get cmd state:%s\n", cmd_state);

	ret = dsi_panel_cmd_update_infos_parse(cmd_set, np,
			cmd_set_update_names[cmd_type]);
	if (ret)
		DSI_DEBUG("get empty update info\n");

	dsi_panel_cmd_port_index_info_parse(cmd_set, np,
			cmd_set_port_index_names[cmd_type]);
	cmd_set->delete_after_usage = false;
	return 0;
}

static void dsi_panel_frame_duration_calculate(struct dsi_panel_mode *mode)
{
	mode->frame_duration_ms = (999 + mode->timing.framerate) /
			mode->timing.framerate;
	if (mode->vrr_type == DSI_VRR_SKIP_FRAME)
		mode->te_duration_ms = (999 + mode->timing.te_rate) / mode->timing.te_rate;
	else
		mode->te_duration_ms = mode->frame_duration_ms;

	mode->frame_timeout_ms = 3 * mode->frame_duration_ms;
	DPU_DEBUG("framerate:%d, frame_duration_ms:%d, te_duration_ms: %d, frame_timeout_ms: %d\n",
			mode->timing.framerate, mode->frame_duration_ms,
			mode->te_duration_ms, mode->frame_timeout_ms);
}

static int dsi_panel_mode_flatten(struct dsi_panel_mode *modes, u32 num_modes,
		int *src_index, struct device_node *child_timing_np)
{
	const char *fps_list_name = "dsi-panel-supported-fps-list";
	struct dsi_panel_mode *src_mode, *flattened_mode;
	u32 src_fps, dest_fps, src_vfp, tgt_vfp, src_total;
	u32 fps_list_size, tgt_index;
	u32 *fps_list;
	int ret, i;

	fps_list_size = of_property_count_u32_elems(child_timing_np,
		fps_list_name);
	if (fps_list_size <= 0 || fps_list_size > MAX_FPS_LIST_SIZE) {
		DSI_DEBUG("no fps list to be flatten, %d\n", fps_list_size);
		return 0;
	}

	fps_list = kzalloc(fps_list_size * sizeof(*fps_list), GFP_KERNEL);
	if (!fps_list)
		return -ENOMEM;
	ret = of_property_read_u32_array(child_timing_np,
			fps_list_name, fps_list, fps_list_size);
	if (ret) {
		DSI_ERROR("failed to get fps list, ret = %d\n", ret);
		goto error;
	}

	src_mode = &modes[*src_index];
	src_vfp = src_mode->cur_timing->vfp;
	src_fps = src_mode->cur_timing->framerate;
	src_total = PANEL_TIMING_V_TOTAL(src_mode->cur_timing);
	tgt_index = *src_index;
	for (i = 0; i < fps_list_size; ++i) {
		dest_fps = fps_list[i];
		if (dest_fps == src_fps)
			continue;

		/* Only support src_fpc >= dest_fps */
		if (src_fps < dest_fps) {
			DSI_DEBUG("not supported tgt fps\n");
			continue;
		}

		ret = dsi_display_front_porch_calculate(src_fps, dest_fps,
				src_total, src_vfp, &tgt_vfp);
		if (ret) {
			DSI_ERROR("failed to get new vfp, ret = %d\n", ret);
			goto error;
		}

		tgt_index++;
		/* Update the flattened mode parameters */
		flattened_mode = &modes[tgt_index];
		memcpy(flattened_mode, src_mode, sizeof(*src_mode));
		flattened_mode->timing.framerate = dest_fps;
		flattened_mode->timing.vfp = tgt_vfp;
		flattened_mode->index = tgt_index;
		flattened_mode->is_flattened_mode = true;
		dsi_panel_frame_duration_calculate(flattened_mode);

		DSI_INFO("parse %d-th mode (fps = %d) success\n",
		flattened_mode->index,
		flattened_mode->timing.framerate);
	}

	DSI_DEBUG("src_index: %d, tgt_index: %d\n", *src_index, tgt_index);
	*src_index = tgt_index;
error:
	kfree(fps_list);
	return ret;
}

static int dsi_panel_vrr_parse(struct dsi_panel_mode *modes, u32 num_modes,
		int *index, struct device_node *child_timing_np)
{
	struct dsi_panel_mode *mode = &modes[*index];
	int ret;

	ret = of_property_read_u32(child_timing_np, "dsi-panel-vrr-type",
			&mode->vrr_type);
	if (ret) {
		DSI_DEBUG("set default vrr type as none\n");
		mode->vrr_type = DSI_VRR_NONE;
		ret = 0;
	}
	DSI_DEBUG("get vrr type:%d\n", mode->vrr_type);

	switch (mode->vrr_type) {
	case DSI_VRR_SKIP_FRAME:
		if (mode->timing.te_rate < mode->timing.framerate) {
			DSI_DEBUG("get invalid te_rate %d, set it as %d\n",
					mode->timing.te_rate, mode->timing.framerate);
			mode->timing.te_rate = mode->timing.framerate;
			ret = 0;
		}
		DSI_DEBUG("get dsi-panel-te-rate: %d\n", mode->timing.te_rate);
		mode->skip_frame_num =
				mode->timing.te_rate / mode->timing.framerate - 1;
		DSI_DEBUG("get skip_frame_num: %d\n", mode->skip_frame_num);
		break;
	case DSI_VRR_LONG_V:
		ret = dsi_panel_mode_flatten(modes, num_modes, index, child_timing_np);
		if (ret) {
			DSI_DEBUG("failed to flatten mode %d\n", ret);
			goto error;
		}
		break;
	default:
		break;
	}

error:
	return ret;
}

static int dsi_panel_flat_mode_info_parse(struct dsi_panel_mode *mode,
		struct device_node *np)
{
	struct dsi_panel_flat_mode_info *flat_mode_infos, *flat_mode_info;
	const char *gamma_cfg_name = "dsi-panel-flat-mode-gamma-cfg";
	int data_length, num_cfgs;
	int ret, i;
	u32 mipi_addr, flat_on_gamma, flat_off_gamma;

	data_length = of_property_count_u32_elems(np, gamma_cfg_name);
	if (data_length < 0 || data_length % FLAT_MODE_INFO_MEMBER) {
		DSI_DEBUG("dsi-panel-flat-mode-gamma-cfg undefined or invald %d\n",
				data_length);
		return 0;
	}

	num_cfgs = data_length / FLAT_MODE_INFO_MEMBER;
	if (!num_cfgs) {
		DSI_INFO("get empty gamma cfg\n");
		return 0;
	}
	DSI_DEBUG("get %d flat mode gamma cfg\n", num_cfgs);
	flat_mode_infos = kcalloc(num_cfgs, sizeof(*flat_mode_infos), GFP_KERNEL);
	if (!flat_mode_infos)
		return -ENOMEM;

	for (i = 0; i < num_cfgs; ++i) {
		flat_mode_info = &flat_mode_infos[i];
		ret = of_property_read_u32_index(np, gamma_cfg_name,
				i * FLAT_MODE_INFO_MEMBER, &mipi_addr);
		if (ret) {
			DSI_ERROR("failed to parse %d mipi_addr\n", i);
			goto error;
		}

		ret = of_property_read_u32_index(np, gamma_cfg_name,
				i * FLAT_MODE_INFO_MEMBER + 1, &flat_on_gamma);
		if (ret) {
			DSI_ERROR("failed to parse %d flat_on_gamma\n", i);
			goto error;
		}

		ret = of_property_read_u32_index(np, gamma_cfg_name,
				i * FLAT_MODE_INFO_MEMBER + 2, &flat_off_gamma);
		if (ret) {
			DSI_ERROR("failed to parse %d flat_off_gamma\n", i);
			goto error;
		}

		flat_mode_info->mipi_addr = mipi_addr;
		flat_mode_info->flat_on_gamma = flat_on_gamma;
		flat_mode_info->flat_off_gamma = flat_off_gamma;
		DSI_DEBUG("%d-th mode (fps = %d), %d-info, 0x%02X, 0x%02X, 0x%02X\n",
				mode->index, mode->timing.framerate, i,
				mipi_addr, flat_on_gamma, flat_off_gamma);
	}

	mode->flat_mode_info_num = num_cfgs;
	mode->flat_mode_infos = flat_mode_infos;
	return 0;
error:
	mode->flat_mode_info_num = 0;
	mode->flat_mode_infos = NULL;
	kfree(flat_mode_infos);
	return ret;
}

static int dsi_panel_modes_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	struct device_node *timings_np, *child_timing_np;
	struct dsi_panel_mode *modes, *mode, *max_fps_mode;
	int index = 0, ret = 0;
	u32 hdisplay, vdisplay;
	u32 num_modes;
	int i;

	panel_info->vrr_enable = of_property_read_bool(np, "dsi-panel-vrr-enable");
	DSI_DEBUG("panel_info->vrr_enable: %d\n", panel_info->vrr_enable);

	ret = dsi_panel_num_modes_get(panel_info, np);
	if (ret) {
		DSI_ERROR("failed to get the number of modes\n");
		goto error;
	}

	num_modes = panel_info->num_modes;
	modes = kcalloc(num_modes, sizeof(*panel_info->modes), GFP_KERNEL);
	if (!modes)
		return -ENOMEM;

	ret = of_property_read_u32(np, "dsi-panel-width",
		&hdisplay);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-width!\n");
		goto error_release_modes;
	}

	ret = of_property_read_u32(np, "dsi-panel-height",
		&vdisplay);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-height!\n");
		goto error_release_modes;
	}

	timings_np = of_find_node_by_name(np, "dsi-display-timings");
	if (!timings_np) {
		DSI_ERROR("failed to find timings node\n");
		ret = -EFAULT;
		goto error_release_modes;
	}
	for_each_available_child_of_node(timings_np, child_timing_np) {
		mode = &modes[index];
		mode->cur_timing = &mode->timing;
		mode->timing.hdisplay = hdisplay;
		mode->timing.vdisplay = vdisplay;
		ret = dsi_panel_timing_parse(panel_info->host_info.ctrl_mode,
			&mode->timing, child_timing_np);
		if (ret) {
			DSI_ERROR("failed to get panel timing\n");
			goto error_put_node;
		}

		ret = dsi_panel_timing_bak_parse(mode, child_timing_np, &modes[0]);
		if (ret)
			DSI_DEBUG("failed to get panel timing bak\n");

		ret = dsi_panel_cmd_sets_parse(mode->cmd_sets, child_timing_np);
		if (ret) {
			DSI_ERROR("failed to parse panel cmd sets\n");
			goto error_put_node;
		}

		mode->index = index;
		mode->is_flattened_mode = false;
		ret = dsi_panel_flat_mode_info_parse(mode, child_timing_np);
		if (ret) {
			DSI_ERROR("failed to parse flat mode info\n");
			goto error_put_node;
		}

		ret = dsi_panel_vrr_parse(modes, num_modes,
				&index, child_timing_np);
		if (ret) {
			DSI_ERROR("failed to parse panel vrr info\n");
			goto error_put_node;
		}

		dsi_panel_frame_duration_calculate(mode);
#ifdef MI_DISPLAY_MODIFY
		ret = mi_dsi_panel_parse_sub_timing(&mode->mi_timing, child_timing_np);
		if (ret)
			DSI_DEBUG("failed to parse panel mi timing, ret=%d\n", ret);
#endif
		index++;
		DSI_DEBUG("parse %d-th mode (fps = %d) success\n",
				mode->index, mode->timing.framerate);
		of_node_put(child_timing_np);
	}

	max_fps_mode = NULL;
	for (i = 0; i < index; ++i) {
		if (!max_fps_mode) {
			max_fps_mode = &modes[i];
			continue;
		}

		if (max_fps_mode->timing.framerate < modes[i].timing.framerate)
			max_fps_mode = &modes[i];
	}

	panel_info->max_fps_mode = max_fps_mode;
	panel_info->modes = modes;
	panel_info->num_modes = index;
	DSI_DEBUG("get %d timings, %d modes\n", panel_info->num_timings,
			panel_info->num_modes);
	of_node_put(timings_np);
	return 0;

error_put_node:
	of_node_put(child_timing_np);
	of_node_put(timings_np);
error_release_modes:
	for (i = 0; i < index; i++)
		dsi_panel_mode_destroy(&modes[i]);

	kfree(modes);
error:
	panel_info->num_modes = 0;
	panel_info->modes = NULL;
	return ret;
}

static int dsi_panel_video_config_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	const char *burst_mode;
	int ret = 0;

	ret = of_property_read_string(np, "dsi-burst-mode", (const char **)&burst_mode);
	if (ret) {
		DSI_WARN("failed to get dsi-burst-mode, set as default: " \
				"non burst with sync pulses\n");
		panel_info->host_info.burst_mode_type =
				NON_BURST_WITH_SYNC_PULSES;
		ret = 0;
	} else if (!strcmp(burst_mode, "non_burst_sync_events")) {
		panel_info->host_info.burst_mode_type =
				NON_BURST_WITH_SYNC_EVENTS;
	} else if (!strcmp(burst_mode, "non_burst_sync_pulses")) {
		panel_info->host_info.burst_mode_type =
				NON_BURST_WITH_SYNC_PULSES;
	} else if (!strcmp(burst_mode, "burst_mode")) {
		panel_info->host_info.burst_mode_type =
				BURST_MODE;
	} else {
		DSI_ERROR("get invalid transmit mode, , set as default: " \
				"non burst with sync pulses\n");
		panel_info->host_info.burst_mode_type =
				NON_BURST_WITH_SYNC_PULSES;
	}

	DSI_INFO("get dsi-burst-mode %s\n", burst_mode);
	return ret;
}

static int dsi_panel_ctrl_mode_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	const char *work_mode;
	int ret = 0;

	ret = of_property_read_string(np,
			"dsi-panel-type", (const char **)&work_mode);

	if (ret) {
		DSI_WARN("failed to parse dsi-panel-type, set as default: cmd mode\n");
		panel_info->host_info.ctrl_mode = DSI_CMD_MODE;
		return 0;
	} else if (!strcmp(work_mode, "dsi_video_mode")) {
		panel_info->host_info.ctrl_mode = DSI_VIDEO_MODE;
		ret = dsi_panel_video_config_parse(panel_info, np);
		if (ret) {
			DSI_ERROR("failed to parse panel video mode config\n");
			ret = 0;
		}
	} else if (!strcmp(work_mode, "dsi_cmd_mode")) {
		panel_info->host_info.ctrl_mode = DSI_CMD_MODE;
	} else {
		DSI_ERROR("invalid panel op mode\n");
		return -EINVAL;
	}

	DSI_DEBUG("get dsi-panel-type: %s\n", work_mode);
	return 0;
}

static int dsi_panel_phy_type_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	const char *phy_type;
	int ret = 0;

	ret = of_property_read_string(np,
			"dsi-panel-phy-type", (const char **)&phy_type);

	if (ret) {
		DSI_WARN("set default panel phy type: dphy\n");
		panel_info->host_info.phy_info.phy_type = DSI_PHY_TYPE_DPHY;
		ret = 0;
		goto exit;
	} else if (!strcmp(phy_type, "dphy")) {
		panel_info->host_info.phy_info.phy_type = DSI_PHY_TYPE_DPHY;
	} else if (!strcmp(phy_type, "cphy")) {
		panel_info->host_info.phy_info.phy_type = DSI_PHY_TYPE_CPHY;
	} else {
		DSI_ERROR("invalid panel phy type:%s, set is as default dphy\n",
				phy_type);
		panel_info->host_info.phy_info.phy_type = DSI_PHY_TYPE_DPHY;
	}

	DSI_DEBUG("get panel phy type:%d(%s)\n",
			panel_info->host_info.phy_info.phy_type, phy_type);
exit:
	return ret;
}

static int dsi_panel_clk_type_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	const char *clk_type;
	int ret = 0;

	ret = of_property_read_string(np,
			"dsi-panel-clk-type", (const char **)&clk_type);

	if (ret) {
		DSI_WARN("set default panel clk type: dphy\n");
		panel_info->host_info.phy_info.clk_type =
				DSI_PHY_CONTINUOUS_CLK;
		ret = 0;
	} else if (!strcmp(clk_type, "continuous")) {
		panel_info->host_info.phy_info.clk_type =
				DSI_PHY_CONTINUOUS_CLK;
	} else if (!strcmp(clk_type, "non_continuous")) {
		panel_info->host_info.phy_info.clk_type =
				DSI_PHY_NON_CONTINUOUS_CLK;
	} else {
		DSI_WARN("invalid panel phy type:%s, set is as default non continus\n",
				clk_type);
		panel_info->host_info.phy_info.clk_type =
				DSI_PHY_NON_CONTINUOUS_CLK;
	}

	DSI_DEBUG("get panel clk type:%d(%s)\n",
			panel_info->host_info.phy_info.clk_type, clk_type);
	return ret;
}

static int dsi_panel_lane_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	u32 i, lane_num;
	int ret = 0;
	struct phy_info *phy_info = &panel_info->host_info.phy_info;

	ret = of_property_read_u32(np, "dsi-panel-lane-number", &lane_num);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-lane-number!\n");
		goto error;
	}
	switch (lane_num) {
	case 1:
		phy_info->lane_num = DSI_PHY_1LANE;
		break;
	case 2:
		phy_info->lane_num = DSI_PHY_2LANE;
		break;
	case 3:
		phy_info->lane_num = DSI_PHY_3LANE;
		break;
	default:
		phy_info->lane_num = DSI_PHY_4LANE;
		break;
	}

	DSI_DEBUG("get phy lane_num: %d\n", lane_num);

	ret = of_property_read_u32(np, "dsi-panel-lane-rate", &phy_info->lane_rate);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-lane-rate!\n");
		goto error;
	}
	phy_info->cur_lane_rate = phy_info->lane_rate;

	DSI_DEBUG("lane_rate: %d\n", phy_info->lane_rate);

	/*
	 * there is no dsi-panel-lane-rate-bak node if doesnt support
	 * mipi dynamic frequency
	 */
	phy_info->lane_rate_baks_num =
			of_property_count_u32_elems(np, "dsi-panel-lane-rate-bak");
	if (phy_info->lane_rate_baks_num > DSI_MIPI_DYN_FREQ_MAX) {
		DSI_DEBUG("failed to count dsi-panel-lane-rate-bak\n");
		phy_info->lane_rate_baks_num = 0;
		ret = 0;
	} else {
		DSI_DEBUG("dsi-panel-lane-rate-bak count is %d\n",
				phy_info->lane_rate_baks_num);

		ret = of_property_read_u32_array(np, "dsi-panel-lane-rate-bak",
				phy_info->lane_rate_baks, phy_info->lane_rate_baks_num);
		if (ret) {
			DSI_DEBUG("failed to get dsi-panel-lane-rate-bak\n");
			phy_info->lane_rate_baks_num = 0;
			ret = 0;
		} else {
			for (i = 0; i < phy_info->lane_rate_baks_num; i++)
				DSI_DEBUG("lane-rate-bak%d: %d\n", i, phy_info->lane_rate_baks[i]);
		}
	}

error:
	return ret;
}

static void __maybe_unused dsi_panel_dsc_info_dump(struct dsc_parms *dsc)
{
	if (!dsc) {
		DSI_ERROR("invalid parameter\n");
		return;
	}

	DSI_DEBUG("dsc_en:%d\n", dsc->dsc_en);
	if (!dsc->dsc_en)
		return;

	DSI_DEBUG("dsc_version_major:%d\n", dsc->dsc_version_major);
	DSI_DEBUG("dsc_version_minor:%d\n", dsc->dsc_version_minor);
	DSI_DEBUG("pic_height:%d\n", dsc->pic_height);
	DSI_DEBUG("pic_width:%d\n", dsc->pic_width);
	DSI_DEBUG("slice_height:%d\n", dsc->slice_height);
	DSI_DEBUG("slice_width:%d\n", dsc->slice_width);
	DSI_DEBUG("bits_per_component:%d\n", dsc->bits_per_component);
	DSI_DEBUG("bits_per_pixel:%d\n", dsc->bits_per_pixel);
}

static int dsi_panel_parse_custom_pps(struct dsc_parms *dsc,
		struct device_node *np)
{
	int pps_size, ret, i;
	u8 *customized_pps;
	const char *name = "dsi-panel-dsc-pps-table";

	pps_size = of_property_count_u8_elems(np, name);
	if (pps_size < 1) {
		DSI_INFO("no customized pps table size\n");
		return 0;
	}

	if (pps_size != DSC_PPS_TABLE_BYTES_SIZE) {
		DSI_ERROR("get invalid pps table size %d\n", pps_size);
		return -EINVAL;
	}

	customized_pps = kcalloc(pps_size, sizeof(u8), GFP_KERNEL);
	if (!customized_pps)
		return -ENOMEM;

	ret = of_property_read_u8_array(np, name, customized_pps, pps_size);
	if (ret) {
		DSI_ERROR("failed to get customized pps table\n");
		goto error;
	}

	DSI_DEBUG("get customized pps table\n");
	for (i = 0; i < DSC_PPS_TABLE_BYTES_SIZE; i++)
		DSI_DEBUG("PPS[%d]: 0x%x\n", i, customized_pps[i]);

	dsc->customized_pps_table = customized_pps;
	return 0;
error:
	kfree(customized_pps);
	return -EINVAL;
}

static int dsi_panel_dsc_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	struct dsc_parms *dsc = &panel_info->host_info.dsc_cfg;
	const char *compression_mode;
	u32 version;
	int ret;

	dsc->dsc_en = false;
	ret = of_property_read_string(np, "dsi-panel-dsc-compression-mode",
			(const char **)&compression_mode);
	DSI_DEBUG("ret = %d, compression mode = %s\n", ret, compression_mode);
	if (ret || strcmp(compression_mode, "dsc")) {
		DSI_INFO("panel not support dsc\n");
		goto exit;
	}

	ret = of_property_read_u32(np, "dsi-panel-dsc-version",
			&version);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-dsc-version!\n");
		goto dsc_parse_err;
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
		DSI_ERROR("dsc version (0x%x) is out of range.\n", version);
		goto dsc_parse_err;
	}

	ret = of_property_read_u32(np, "dsi-panel-dsc-pic-height",
			&dsc->pic_height);
	if (ret) {
		DSI_ERROR("failed to get dsc-pic-heigh!\n");
		goto dsc_parse_err;
	}

	ret = of_property_read_u32(np, "dsi-panel-dsc-pic-width",
			&dsc->pic_width);
	if (ret) {
		DSI_ERROR("failed to get dsc-pic-heigh!\n");
		goto dsc_parse_err;
	}

	ret = of_property_read_u32(np, "dsi-panel-dsc-slice-height",
			&dsc->slice_height);
	if (ret) {
		DSI_ERROR("failed to get dsc-sliceh!\n");
		goto dsc_parse_err;
	}

	ret = of_property_read_u32(np, "dsi-panel-dsc-slice-width",
			&dsc->slice_width);
	if (ret) {
		DSI_ERROR("failed to get dsc-slicew!\n");
		goto dsc_parse_err;
	}

	ret = of_property_read_u8(np, "dsi-panel-dsc-bpc", &dsc->bits_per_component);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-dsc-bpc!\n");
		goto dsc_parse_err;
	}

	/* bits per pixel after dsc compression */
	ret = of_property_read_u8(np, "dsi-panel-dsc-bpp", &dsc->bits_per_pixel);
	if (ret) {
		DSI_ERROR("failed to get dsi-panel-dsc-bpp!\n");
		goto dsc_parse_err;
	}

	ret = dsi_panel_parse_custom_pps(dsc, np);
	if (ret) {
		DSI_ERROR("failed to get customized pps table!\n");
		goto dsc_parse_err;
	}

	dsc->dsc_en = true;
exit:
	dsi_panel_dsc_info_dump(dsc);
	return 0;

dsc_parse_err:
	DSI_ERROR("parse dsc info error, set dsc_en as false\n");
	dsc->dsc_en = false;
	return ret;
}

static int dsi_panel_color_format_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	enum dsi_pixel_format color_fmt;
	bool dsc_en;
	int ret = 0;
	int bpp;

	ret = of_property_read_u32(np, "dsi-panel-bpp", &bpp);
	if (ret) {
		DSI_WARN("failed to get dsi-panel-bpp, set as default 24\n");
		bpp = 24;
		ret = 0;
	} else {
		DSI_DEBUG("get panel bpp %d!\n", bpp);
	}

	switch (bpp) {
	case 18:
		color_fmt = DSI_FMT_RGB666;
		break;
	case 30:
		color_fmt = DSI_FMT_RGB101010;
		break;
	default:
		/* if DSC is enabled, the color format should be RGB888 */
		color_fmt = DSI_FMT_RGB888;
	}

	dsc_en = panel_info->host_info.dsc_cfg.dsc_en;
	panel_info->host_info.bpp = bpp;
	panel_info->host_info.color_fmt = dsc_en ? DSI_FMT_DSC : color_fmt;
	DSI_DEBUG("get color format:%d\n",
			panel_info->host_info.color_fmt);

	return ret;
}

static int dsi_panel_host_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	int i;
	u8 port_mask;
	int ret = 0;
	struct dsi_panel_host_info *host_info = &panel_info->host_info;

	ret = dsi_panel_ctrl_mode_parse(panel_info, np);
	if (ret)
		DSI_ERROR("failed to get panel op mode\n");

	ret = dsi_panel_dsc_parse(panel_info, np);
	if (ret)
		DSI_ERROR("failed to get dsc info\n");

	/* The color format should be parsed after parsing the DSC info.*/
	ret = dsi_panel_color_format_parse(panel_info, np);
	if (ret)
		DSI_ERROR("failed to get panel color format !\n");

	ret = dsi_panel_phy_type_parse(panel_info, np);
	if (ret)
		DSI_ERROR("failed to get panel phy type\n");

	ret = dsi_panel_clk_type_parse(panel_info, np);
	if (ret)
		DSI_ERROR("failed to get panel clk type\n");

	ret = dsi_panel_lane_parse(panel_info, np);
	if (ret)
		DSI_ERROR("failed to get lane info\n");

	ret = of_property_read_u32(np, "dsi-panel-pixel-clock",
			&host_info->pixel_clk_rate);
	if (ret)
		DSI_ERROR("failed to get dsi-panel-pixel-clock!\n");
	host_info->cur_pixel_clk_rate = host_info->pixel_clk_rate;

	ret = of_property_read_u32(np, "dsi-panel-pixel-clock-div",
			&host_info->pixel_clk_div);
	if (ret)
		host_info->pixel_clk_div = 0;

	DSI_DEBUG("get dsi pixel_clk_rate %d, div %d\n",
		host_info->pixel_clk_rate, host_info->pixel_clk_div);

	host_info->pixel_clk_rate_baks_num =
			of_property_count_u32_elems(np, "dsi-panel-pixel-clock-bak");
	if (host_info->pixel_clk_rate_baks_num > DSI_MIPI_DYN_FREQ_MAX) {
		DSI_DEBUG("failed to get dsi-panel-pixel-clock-bak!\n");
		host_info->pixel_clk_rate_baks_num = 0;
	} else {
		DSI_DEBUG("dsi-panel-pixel-clock-bak count %d\n",
				host_info->pixel_clk_rate_baks_num);

		ret = of_property_read_u32_array(np, "dsi-panel-pixel-clock-bak",
				host_info->pixel_clk_rate_baks,
				host_info->pixel_clk_rate_baks_num);
		if (ret)
			DSI_DEBUG("failed to get dsi-panel-pixel-clock-bak!\n");

		for (i = 0; i < host_info->pixel_clk_rate_baks_num; i++)
			DSI_DEBUG("dsi-panel-pixel-clock-baks[%d] %d\n",
					i, host_info->pixel_clk_rate_baks[i]);
	}

	ret = of_property_read_u32(np, "dsi-panel-ipi-pll-sel",
			&panel_info->host_info.ipi_pll_sel);
	if (ret) {
		panel_info->host_info.ipi_pll_sel = DSI_PIXEL_CLOCK_PHY_PLL;
		DSI_DEBUG("default use phy pll\n");
	}
	DSI_DEBUG("ipi pixel clock use %d\n", panel_info->host_info.ipi_pll_sel);

	ret = of_property_read_u32(np, "dsi-panel-phy-amplitude",
			&panel_info->host_info.phy_info.phy_amplitude);
	if (ret)
		panel_info->host_info.phy_info.phy_amplitude = PHY_AMPLITUDE_200MV;

	ret = of_property_read_u32(np, "dsi-panel-phy-oa-setr",
			&panel_info->host_info.phy_info.phy_oa_setr);
	if (ret)
		panel_info->host_info.phy_info.phy_oa_setr = 0;

	ret = of_property_read_u32(np, "dsi-panel-phy-eqa",
			&panel_info->host_info.phy_info.phy_eqa);
	if (ret)
		panel_info->host_info.phy_info.phy_eqa = 0;

	ret = of_property_read_u32(np, "dsi-panel-phy-eqb",
			&panel_info->host_info.phy_info.phy_eqb);
	if (ret)
		panel_info->host_info.phy_info.phy_eqb = 0;



	ret = of_property_read_u32(np, "dsi-panel-virtual-channel",
			&host_info->virtual_channel);
	if (ret) {
		DSI_DEBUG("failed to get dsi-panel-virtual-channel, set as default 0!\n");
		host_info->virtual_channel = 0;
	} else {
		DSI_DEBUG("virtual_channel %d\n", host_info->virtual_channel);
	}

	host_info->bta_en = of_property_read_bool(np, "dsi-panel-bta-enable");
	DSI_DEBUG("bta_en %d!\n", host_info->bta_en);

	host_info->eotp_en = of_property_read_bool(np, "dsi-panel-eotp-enable");
	DSI_DEBUG("eotp_en %d!\n", host_info->eotp_en);

	ret = of_property_read_u8(np, "dsi-panel-port-mask", &port_mask);
	if (ret) {
		DSI_WARN("failed to get port mask, set it as default 0x1(dsi-0)\n");
		port_mask = BIT(DSI_PORT_0);
	}

	host_info->port_mask = port_mask;
	host_info->sbs_en = false;
	if ((port_mask & BIT(DSI_PORT_0)) && (port_mask & BIT(DSI_PORT_1)))
		host_info->sbs_en = true;

	DSI_DEBUG("amplitude %d, oa_setr %d, eqa %d, eqb %d, port_mask %d, sbs %d\n",
		panel_info->host_info.phy_info.phy_amplitude,
		panel_info->host_info.phy_info.phy_oa_setr,
		panel_info->host_info.phy_info.phy_eqa,
		panel_info->host_info.phy_info.phy_eqb,
		host_info->port_mask,
		host_info->sbs_en);

	return ret;
}

static int dsi_panel_power_state_parse(struct device_node *np,
		const char *state_seq_name, int seq_index,
		struct dsi_panel_power_state *state)
{
	u32 level, delay_ms;
	int ret;

	ret = of_property_read_u32_index(np, state_seq_name, seq_index << 1,
			&level);
	if (ret) {
		DSI_ERROR("index-%d failed to parse state level\n", seq_index);
		return ret;
	}

	ret = of_property_read_u32_index(np, state_seq_name,
			(seq_index << 1) + 1, &delay_ms);
	if (ret) {
		DSI_ERROR("index-%d failed to parse state delay_ms\n", seq_index);
		return ret;
	}

	state->level = level;
	state->delay_ms = delay_ms;
	DSI_DEBUG("index-%d %s, level: %d, delay_ms: %d\n", seq_index,
			state_seq_name, level, delay_ms);
	return 0;
}

static int dsi_panel_reset_ctrl_parse(struct reset_ctrl *reset_ctrl,
		struct device_node *np, const char *gpio_name,
		const char *state_seq_name)
{
	u32 state_num, data_length = 0;
	u32 state_seq_size;
	int ret = 0;
	int i;

	reset_ctrl->gpio = of_get_named_gpio(np, gpio_name, 0);
	if (!gpio_is_valid(reset_ctrl->gpio)) {
		DSI_ERROR("%s is not set\n", gpio_name);
		reset_ctrl->state_num = 0;
		goto error;
	}

	DSI_DEBUG("get reset gpio number:%d\n", reset_ctrl->gpio);

	/* parse state sequence */
	data_length = of_property_count_u32_elems(np, state_seq_name);
	if (!data_length || (data_length % 2)) {
		DSI_ERROR("invalid %s state sequence\n", state_seq_name);
		ret = -EINVAL;
		goto error;
	}

	DSI_DEBUG("get %d u32 raw data\n", data_length);
	state_num = data_length >> 1;
	state_seq_size = state_num * sizeof(struct dsi_panel_power_state);
	reset_ctrl->state_sequence = kzalloc(state_seq_size, GFP_KERNEL);
	if (!reset_ctrl->state_sequence) {
		ret = -ENOMEM;
		goto error;
	}

	for (i = 0; i < state_num; i++) {
		ret = dsi_panel_power_state_parse(np, state_seq_name, i,
				&reset_ctrl->state_sequence[i]);
		if (ret) {
			DSI_ERROR("failed to parse %d state sequence\n", i);
			goto error_release_sequence;
		}
	}

	ret = of_property_read_u32(np, "dsi-panel-delay-ms-before-reset-off",
			&reset_ctrl->delay_ms_before_reset_off);
	if (ret) {
		reset_ctrl->delay_ms_before_reset_off = 0;
		ret = 0;
		DSI_DEBUG("set delay_ms_before_reset_off as default 0\n");
	}

	DSI_DEBUG("get delay_ms_before_reset_off: %d\n",
			reset_ctrl->delay_ms_before_reset_off);

	reset_ctrl->reset_after_lp11 = of_property_read_bool(np,
			"dsi-panel-reset-after-lp11");
	DSI_DEBUG("reset_after_lp11:%d\n", reset_ctrl->reset_after_lp11);
	reset_ctrl->state_num = state_num;
	return 0;

error_release_sequence:
	reset_ctrl->state_num = 0;
	kfree(reset_ctrl->state_sequence);
error:
	return ret;
}

static void dsi_panel_reset_ctrl_release(struct reset_ctrl *reset_ctrl)
{
	if (gpio_is_valid(reset_ctrl->gpio))
		gpio_free(reset_ctrl->gpio);

	kfree(reset_ctrl->state_sequence);
	reset_ctrl->state_sequence = NULL;
	reset_ctrl->state_num = 0;
}

static void dsi_panel_gpio_release(struct dsi_panel *panel)
{
	struct dsi_panel_power_supply *power_supply_infos, *power_supply_info;
	struct dsi_panel_esd_info *esd_info;
	u32 supply_info_num;
	int i;


	power_supply_infos = panel->panel_info.power_seq.power_supply_infos;
	supply_info_num = panel->panel_info.power_seq.supply_info_num;

	for (i = 0; i < supply_info_num; i++) {
		power_supply_info = &power_supply_infos[i];
		if (power_supply_info->power_type != DSI_PANEL_POWER_TYPE_GPIO)
			continue;
		gpio_free(power_supply_info->gpio_id);
	}

	dsi_panel_reset_ctrl_release(&panel->panel_info.reset_ctrl);

	esd_info = &panel->panel_info.esd_info;
	for (i = 0; i < esd_info->err_irq_gpio_num; ++i) {
		if (gpio_is_valid(esd_info->err_irq_gpio[i]))
			gpio_free(esd_info->err_irq_gpio[i]);
	}
}

static int dsi_panel_gpio_request(struct dsi_panel *panel)
{
	struct dsi_panel_power_supply *power_supply_infos, *power_supply_info;
	struct reset_ctrl *reset_ctrl;
	struct dsi_panel_esd_info *esd_info;
	u32 supply_info_num;
	int i, ret;

	power_supply_infos = panel->panel_info.power_seq.power_supply_infos;
	supply_info_num = panel->panel_info.power_seq.supply_info_num;

	for (i = 0; i < supply_info_num; i++) {
		power_supply_info = &power_supply_infos[i];
		if (power_supply_info->power_type != DSI_PANEL_POWER_TYPE_GPIO)
			continue;
		ret = gpio_request(power_supply_info->gpio_id,
				power_supply_info->name);
		if (ret) {
			DSI_WARN("failed to request reset_gpio, ret = %d\n",
					ret);
			goto error;
		}
	}

	reset_ctrl = &panel->panel_info.reset_ctrl;
	if (gpio_is_valid(reset_ctrl->gpio)) {
		ret = gpio_request(reset_ctrl->gpio, "reset_gpio");
		if (ret) {
			DSI_WARN("failed to request reset_gpio, ret = %d\n",
					ret);
			goto error;
		}
	}

	esd_info = &panel->panel_info.esd_info;
	for (i = 0; i < esd_info->err_irq_gpio_num; ++i) {
		if (!gpio_is_valid(esd_info->err_irq_gpio[i]))
			continue;

		ret = gpio_request(esd_info->err_irq_gpio[i],
				esd_info->err_irq_gpio_name[i]);
		if (ret) {
			DSI_WARN("failed to request %s, ret = %d\n",
					esd_info->err_irq_gpio_name[i], ret);
			goto error;
		}
	}

	return 0;
error:
	while (--i > 0) {
		power_supply_info = &power_supply_infos[i];
		if (power_supply_info->power_type == DSI_PANEL_POWER_TYPE_GPIO)
			gpio_free(power_supply_info->gpio_id);
	}
	return ret;
}

static int dsi_panel_hdr_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	struct dsi_panel_hdr_caps *hdr_info;
	int index;
	int ret = 0;

	hdr_info = &panel_info->hdr_info;
	hdr_info->hdr_enabled = of_property_read_bool(np,
			"dsi-panel-hdr-enabled");

	DSI_DEBUG("get hdr enabled:%d\n", hdr_info->hdr_enabled);

	if (hdr_info->hdr_enabled) {
		for (index = 0; index < COLOR_PRIMATIES_COORDS; index++) {
			ret = of_property_read_u32_index(np,
				"dsi-panel-hdr-color-primaries",
				index, &hdr_info->color_primaries[index]);
			if (ret)
				DSI_DEBUG("dsi panel peak brightness is not defined\n");
		}

		ret = of_property_read_u32(np,
				"dsi-panel-peak-brightness",
				&hdr_info->peak_brightness);
		if (ret) {
			DSI_ERROR("dsi panel peak brightness is not defined\n");
			hdr_info->hdr_enabled = false;
			goto error;
		}

		ret = of_property_read_u32(np,
				"dsi-panel-blackness-level",
				&hdr_info->blackness_level);
		if (ret) {
			DSI_ERROR("dsi panel blackness level is not defined\n");
			hdr_info->hdr_enabled = false;
			goto error;
		}

		DSI_DEBUG("get hdr peak brightness:%d, backness level:%d\n",
				hdr_info->peak_brightness,
				hdr_info->blackness_level);
	}

error:
	return ret;
}

static  int dsi_panel_supply_seq_parse(struct device_node *np,
		const char *seq_name, const char *type_name, int index,
		struct dsi_panel_power_supply *power_supply_info,
		struct device *panel_dev)
{
	const char *power_name, *power_type;
	int ret;

	ret = of_property_read_string_index(np, seq_name, index, &power_name);
	if (ret) {
		DSI_ERROR("failed to parse power_name, index: %d\n", index);
		return ret;
	}
	DSI_DEBUG("index-%d power_name: %s\n", index, power_name);

	ret = of_property_read_string_index(np, type_name, index, &power_type);
	if (ret) {
		DSI_ERROR("failed to parse power_type, index: %d\n", index);
		power_supply_info->power_type = DSI_PANEL_POWER_TYPE_INVALID;
		return ret;
	}
	DSI_DEBUG("index-%d power_type: %s\n", index, power_type);

	if (!strcmp(power_type, "GPIO")) {
		power_supply_info->power_type = DSI_PANEL_POWER_TYPE_GPIO;
		power_supply_info->gpio_id =
				of_get_named_gpio(np, power_name, 0);
		if (!gpio_is_valid(power_supply_info->gpio_id)) {
			DSI_ERROR("index-%d invalid gpio: %s\n",
					index, power_name);
			return -EINVAL;
		}

		DSI_DEBUG("index-%d gpio id %d\n", index,
				power_supply_info->gpio_id);
	} else if (!strcmp(power_type, "LDO")) {
		power_supply_info->power_type = DSI_PANEL_POWER_TYPE_LDO;
		power_supply_info->regulator =
				regulator_get(panel_dev, power_name);
		if (IS_ERR_OR_NULL(power_supply_info->regulator)) {
			DSI_ERROR("index-%d failed to get regulator\n", index);
			return -EINVAL;
		}
		DSI_DEBUG("index-%d get ldo %s success\n", index, power_name);
		/* Vote regulator when continus display */
		if (is_cont_display_enabled()) {
			ret = regulator_enable(power_supply_info->regulator);
			if (ret) {
				DSI_ERROR("index-%d enable regulator failed\n",
						index);
				return ret;
			}
			DSI_DEBUG("index-%d enable regulator success\n", index);
		}
	} else {
		power_supply_info->power_type = DSI_PANEL_POWER_TYPE_INVALID;
		DSI_ERROR("index-%d get invalid type %s\n", index, power_type);
		return ret;
	}

	power_supply_info->name = power_name;
	return ret;
}

static int dsi_panel_supply_state_parse(struct device_node *np,
		const char *on_state_name, const char *off_state_name,
		int index, struct dsi_panel_power_supply *power_supply_info)
{
	int ret;

	ret = dsi_panel_power_state_parse(np, on_state_name, index,
			&power_supply_info->on_state);
	if (ret) {
		DSI_ERROR("index-%d failed to parse %s\n", index, on_state_name);
		return ret;
	}

	ret = dsi_panel_power_state_parse(np, off_state_name, index,
			&power_supply_info->off_state);
	if (ret) {
		DSI_ERROR("index-%d failed to parse %s\n", index, off_state_name);
		return ret;
	}

	return 0;
}


static int dsi_panel_power_supply_parse(struct dsi_panel_power_seq *io_seq,
		struct device_node *np, struct device *panel_dev)
{
	struct dsi_panel_power_supply *power_supply_infos, *power_supply_info;
	const char *seq_name = "dsi-panel-power-seq";
	const char *type_name = "dsi-panel-power-type";
	const char *on_state_name = "dsi-panel-power-on-state";
	const char *off_state_name = "dsi-panel-power-off-state";
	int seq_size, type_size, on_state_size, off_state_size;
	int seq_idx;
	int ret = 0;

	seq_size = of_property_count_strings(np, seq_name);
	DSI_DEBUG("get power supply seq_size: %d\n", seq_size);
	type_size = of_property_count_strings(np, type_name);
	DSI_DEBUG("get power supply type_size: %d\n", type_size);
	on_state_size = of_property_count_u32_elems(np, on_state_name);
	if (on_state_size < 0 || on_state_size % PANEL_POWER_STATE_SIZE) {
		DSI_ERROR("get invalid on_state_size %d\n", on_state_size);
		return -EINVAL;
	}
	on_state_size >>= 1;
	DSI_DEBUG("get on_state_size: %d\n", on_state_size);

	off_state_size = of_property_count_u32_elems(np, off_state_name);
	if (off_state_size < 0 || off_state_size % PANEL_POWER_STATE_SIZE) {
		DSI_ERROR("get invalid off_state_size %d\n", off_state_size);
		return -EINVAL;
	}
	off_state_size >>= 1;
	DSI_DEBUG("get off_state_size: %d\n", off_state_size);
	if (seq_size != type_size || seq_size != on_state_size ||
			seq_size != off_state_size) {
		DSI_ERROR("get invalid power info size, " \
				"seq_size: %d," \
				"type_size: %d," \
				"on_state_size: %d," \
				"off_state_size: %d\n",
				seq_size, type_size,
				on_state_size, off_state_size);
		return -EINVAL;
	}

	power_supply_infos = kcalloc(seq_size, sizeof(*power_supply_info), GFP_KERNEL);
	if (!power_supply_infos)
		return -ENOMEM;

	for (seq_idx = 0; seq_idx < seq_size; ++seq_idx) {
		power_supply_info = &power_supply_infos[seq_idx];
		ret = dsi_panel_supply_seq_parse(np, seq_name, type_name,
				seq_idx, power_supply_info, panel_dev);
		if (ret) {
			DSI_ERROR("failed to parse %d-th power_type\n", seq_idx);
			goto error;
		}

		ret = dsi_panel_supply_state_parse(np, on_state_name,
				off_state_name, seq_idx, power_supply_info);
		if (ret) {
			DSI_ERROR("failed to parse %d-th power state\n", seq_idx);
			goto error;
		}
	}

	io_seq->supply_info_num = seq_size;
	io_seq->power_supply_infos = power_supply_infos;
	return ret;
error:
	io_seq->supply_info_num = 0;
	io_seq->power_supply_infos = NULL;
	kfree(power_supply_infos);
	return ret;
}

static int dsi_panel_pinctrl_info_parse(struct dsi_panel_info *panel_info,
		struct device *panel_dev)
{
	struct dsi_panel_pinctrl_info *pinctrl_info;
	int ret = 0;

	pinctrl_info = &panel_info->pinctrl_info;

	pinctrl_info->pinctrl_ptr = pinctrl_get(panel_dev);
	if (IS_ERR_OR_NULL(pinctrl_info->pinctrl_ptr)) {
		DSI_ERROR("failed to get pinctrl handle\n");
		ret = PTR_ERR(pinctrl_info->pinctrl_ptr);
		goto error;
	}

	pinctrl_info->work_state = pinctrl_lookup_state(pinctrl_info->pinctrl_ptr,
			"work");
	if (IS_ERR_OR_NULL(pinctrl_info->work_state)) {
		DSI_ERROR("failed to get pinctrl work state\n");
		ret = PTR_ERR(pinctrl_info->work_state);
		goto error_with_release_pinctrl;
	} else {
		DSI_DEBUG("get default pinctrl state success\n");
	}

	pinctrl_info->sleep_state = pinctrl_lookup_state(pinctrl_info->pinctrl_ptr,
			"sleep");
	if (IS_ERR_OR_NULL(pinctrl_info->sleep_state)) {
		DSI_ERROR("failed to get pinctrl sleep state\n");
		ret = PTR_ERR(pinctrl_info->sleep_state);
		goto error_with_release_pinctrl;
	} else {
		DSI_DEBUG("get pinctrl state success\n");
	}

	if (panel_info->host_info.ctrl_mode == DSI_CMD_MODE) {
		pinctrl_info->te_work_state = pinctrl_lookup_state(
			pinctrl_info->pinctrl_ptr, "te_work");
		if (IS_ERR_OR_NULL(pinctrl_info->te_work_state)) {
			DSI_ERROR("failed to get pinctrl te_work state\n");
			ret = PTR_ERR(pinctrl_info->te_work_state);
			goto error_with_release_pinctrl;
		} else {
			DSI_DEBUG("get te_work state success\n");
		}

		pinctrl_info->te_gpio_state = pinctrl_lookup_state(
			pinctrl_info->pinctrl_ptr, "te_gpio");
		if (IS_ERR_OR_NULL(pinctrl_info->te_gpio_state)) {
			DSI_ERROR("failed to get pinctrl te_gpio state\n");
			ret = PTR_ERR(pinctrl_info->te_gpio_state);
			goto error_with_release_pinctrl;
		} else {
			DSI_DEBUG("get te_gpio state success\n");
		}

	}

	DSI_DEBUG("parse panel regulator success\n");
	return 0;

error_with_release_pinctrl:
	pinctrl_put(pinctrl_info->pinctrl_ptr);
error:
	return ret;
}

static int dsi_panel_esd_mode_parse(struct dsi_panel_esd_info *esd_info,
		struct device_node *np)
{
	const char *mode;
	int ret;

	ret = of_property_read_string(np, "dsi-panel-esd-mode",
			(const char **)&mode);
	if (ret) {
		DSI_ERROR("failed to parse esd check mode\n");
		return ret;
	}

	if (!strcmp(mode, "err_int")) {
		esd_info->esd_mode = DSI_PANEL_ESD_MODE_ERR_INT;
	} else if (!strcmp(mode, "reg_read")) {
		esd_info->esd_mode = DSI_PANEL_ESD_MODE_REG_READ;
	} else if (!strcmp(mode, "panel_te")) {
		esd_info->esd_mode = DSI_PANEL_ESD_MODE_PANEL_TE;
	} else if (!strcmp(mode, "err_int_and_reg_read")) {
		esd_info->esd_mode = DSI_PANEL_ESD_MODE_ERR_INT_AND_REG_READ;
	} else {
		DSI_ERROR("get invalid esd check mode: %s\n", mode);
		return -EINVAL;
	}

	DSI_DEBUG("get esd mode %s, %d\n", mode, esd_info->esd_mode);
	return 0;
}

static int dsi_panel_err_irq_parse(int *gpio_num, int *flags,
		int *err_irq, const char *gpio_name,
		const char *gpio_flag_name, struct device_node *np)
{
	int ret = 0;

	*gpio_num = of_get_named_gpio(np, gpio_name, 0);
	ret = of_property_read_u32(np, gpio_flag_name, flags);
	if (gpio_is_valid(*gpio_num) && !ret) {
		*err_irq = gpio_to_irq(*gpio_num);
		gpio_direction_input(*gpio_num);
	} else {
		return ret;
	}

	DSI_DEBUG("get error irq gpio: %d, flag: 0x%x\n", *gpio_num, *flags);
	return ret;
}

int dsi_panel_esd_irq_parse(struct dsi_panel_esd_info *esd_info,
		struct device_node *np)
{
	const char *gpio_0_name = "dsi-panel-esd-err-irq-gpio-0";
	const char *gpio_1_name = "dsi-panel-esd-err-irq-gpio-1";
	const char *gpio_0_flag_name = "dsi-panel-erd-err-irq-gpio-0-flag";
	const char *gpio_1_flag_name = "dsi-panel-esd-err-irq-gpio-1-flag";
	int index = 0;
	int ret;

	ret = dsi_panel_err_irq_parse(&esd_info->err_irq_gpio[index],
			&esd_info->err_irq_gpio_flag[index],
			&esd_info->err_irq[index],
			gpio_0_name, gpio_0_flag_name, np);
	if (ret) {
		DSI_ERROR("fialed to parse %s err irq gpio\n", gpio_0_name);
		goto error;
	}

	esd_info->err_irq_gpio_name[index] = gpio_0_name;
	++index;
	ret = dsi_panel_err_irq_parse(&esd_info->err_irq_gpio[index],
			&esd_info->err_irq_gpio_flag[index],
			&esd_info->err_irq[index],
			gpio_1_name, gpio_1_flag_name, np);
	if (ret) {
		DSI_DEBUG("%s err irq gpio no used\n", gpio_1_name);
		ret = 0;
		goto error;
	}

	esd_info->err_irq_gpio_name[index] = gpio_1_name;
	++index;
	DSI_DEBUG("get %d err irq config success\n", index);
error:
	esd_info->err_irq_gpio_num = index;
	return ret;
}

static int dsi_panel_esd_status_parse(const char *status_name,
		u32 **status, u32 target_len, struct device_node *np)
{
	u32 length;
	int ret;
	int i;

	length = of_property_count_u32_elems(np, status_name);
	if (length != target_len) {
		DSI_ERROR("get unmatch size, length %d, target_len %d\n",
				length, target_len);
		return -EINVAL;
	}

	*status = kcalloc(length, sizeof(u32), GFP_KERNEL);
	if (!(*status)) {
		DSI_ERROR("failed to alloc memory for %s\n", status_name);
		return -ENOMEM;
	}

	ret = of_property_read_u32_array(np, status_name,
			*status, length);
	if (ret) {
		DSI_ERROR("failed to read %s array\n", status_name);
		goto error;
	}

	DSI_DEBUG("get %s\n", status_name);
	for (i = 0; i < target_len; ++i)
		DSI_DEBUG("target status[%d] 0x%x\n", i, (*status)[i]);

	return 0;
error:
	kfree(*status);
	*status = NULL;
	return ret;
}

int dsi_panel_esd_reg_read_parse(struct dsi_panel_esd_info *esd_info,
		struct device_node *np)
{
	u32 status_length_total;
	int ret, i;

	/* TODO: implement this function */

	/* default 5s, may updated when debugfs is config esd priod */
	esd_info->period = ESD_CHECK_PERIOD_MS;

	ret = dsi_panel_cmd_single_parse(&esd_info->panel_status_cmd,
			DSI_CMD_SET_PANEL_STATUS, np);
	if (ret) {
		DSI_ERROR("failed to parse cmd set %s",
				cmd_set_names[DSI_CMD_SET_PANEL_STATUS]);
		return -EINVAL;
	}

	ret = dsi_panel_esd_status_parse("dsi-panel-status-length",
			&esd_info->status_length,
			esd_info->panel_status_cmd.num_cmds, np);
	if (ret) {
		DSI_ERROR("invalid panel status length\n");
		goto error_with_release_status_cmd_set;
	}

	status_length_total = 0;
	for (i = 0; i < esd_info->panel_status_cmd.num_cmds; ++i)
		status_length_total += esd_info->status_length[i];
	if (!status_length_total) {
		DSI_ERROR("invalid status_length_total\n");
		goto error_with_release_status_len;
	}
	DSI_DEBUG("get status_length_total %d\n", status_length_total);

	ret = dsi_panel_esd_status_parse("dsi-panel-status-value",
			&esd_info->status_value,
			status_length_total, np);
	if (ret) {
		DSI_ERROR("invalid dsi-panel-status-value\n");
		goto error_with_release_status_len;
	}

	esd_info->status_buf = kcalloc(status_length_total,
			sizeof(unsigned char), GFP_KERNEL);
	esd_info->status_length_total = status_length_total;

	DSI_DEBUG("get esd reg read config success\n");
	return 0;
error_with_release_status_len:
	kfree(esd_info->status_length);
error_with_release_status_cmd_set:
	dsi_panel_cmds_free(&esd_info->panel_status_cmd);
	return ret;
}

static void dsi_panel_esd_te_parse(struct dsi_panel_esd_info *esd_info,
		struct device_node *np)
{
	esd_info->te_gpio = of_get_named_gpio(np, "dsi-panel-te-gpio", 0);
	if (!gpio_is_valid(esd_info->te_gpio))
		DSI_ERROR("get invalid te_gpio: %d\n", esd_info->te_gpio);
	DSI_DEBUG("get te_gpio %d\n", esd_info->te_gpio);
}

static int dsi_panel_esd_parse(struct dsi_panel_info *panel_info,
		struct device_node *np)
{
	struct dsi_panel_esd_info *esd_info;
	int ret;

	esd_info = &panel_info->esd_info;

	esd_info->enable = of_property_read_bool(np,
			"dsi-panel-esd-check-enable");

	/* Requires te gpio to differentiate the miss te problem caused by AP/panel */
	if (panel_info->host_info.ctrl_mode == DSI_CMD_MODE)
		dsi_panel_esd_te_parse(esd_info, np);

	if (!esd_info->enable) {
		DSI_INFO("esd check is diabled\n");
		return 0;
	}

	ret = dsi_panel_esd_mode_parse(esd_info, np);
	if (ret) {
		DSI_ERROR("failed to parse esd mode\n");
		goto error;
	}

	/* parse all esd check mode info to support mode switch in debugfs */
	if (esd_info->esd_mode == DSI_PANEL_ESD_MODE_ERR_INT ||
			esd_info->esd_mode == DSI_PANEL_ESD_MODE_ERR_INT_AND_REG_READ) {
		ret = dsi_panel_esd_irq_parse(esd_info, np);
		if (ret)
			DSI_ERROR("failed to parse esd gpio info\n");
	}

	if (esd_info->esd_mode == DSI_PANEL_ESD_MODE_REG_READ ||
			esd_info->esd_mode == DSI_PANEL_ESD_MODE_ERR_INT_AND_REG_READ) {
		ret = dsi_panel_esd_reg_read_parse(esd_info, np);
		if (ret)
			DSI_ERROR("failed to parse esd reg read info\n");
	}

	return 0;
error:
	esd_info->enable = false;
	return ret;
}

static int dsi_panel_info_parse(struct dsi_panel *panel)
{
	struct dsi_panel_info *panel_info = &panel->panel_info;
	struct device_node *np;
	int ret = 0;

	np = panel->mipi_device.dev.of_node;
	if (!np) {
		DSI_ERROR("panel deivce node is null\n");
		return -EINVAL;
	}

	ret = dsi_panel_common_parse(panel_info, np);
	if (ret)
		DSI_ERROR("failed to parse panel common info\n");

	ret = dsi_panel_hdr_parse(panel_info, np);
	if (ret)
		DSI_ERROR("failed to parse panel hdr info\n");

	ret = dsi_panel_host_parse(panel_info, np);
	if (ret) {
		DSI_ERROR("failed to get host info\n");
		return ret;
	}

	ret = dsi_panel_modes_parse(panel_info, np);
	if (ret) {
		DSI_ERROR("failed to parse panel mode info\n");
		return ret;
	}

	if (panel->panel_info.is_asic && !panel_info->is_fake_panel) {
		ret = dsi_panel_pinctrl_info_parse(panel_info, &panel->mipi_device.dev);
		if (ret)
			DSI_ERROR("failed to get pinctrl\n");
	} else {
		DSI_INFO("bypass the pmic&pinctrl parse for FPGA and fake panel\n");
	}

	if (!panel_info->is_fake_panel) {
		ret = dsi_panel_power_supply_parse(&panel_info->power_seq, np,
				&panel->mipi_device.dev);
		if (ret)
			DSI_ERROR("failed to parse panel power supply\n");

		/* parse reset ctrl */
		ret = dsi_panel_reset_ctrl_parse(&panel_info->reset_ctrl, np,
				"dsi-panel-reset-gpio",
				"dsi-panel-reset-state");
		if (ret)
			DSI_ERROR("failed to parse reset gpio ctrl\n");

		ret = dsi_panel_esd_parse(panel_info, np);
		if (ret)
			DSI_ERROR("failed to parse esd info, ret: %d\n", ret);

		ret = dsi_panel_gpio_request(panel);
		if (ret)
			DSI_ERROR("failed to request panel gpio\n");

	} else {
		DSI_INFO("bypass the panel IO parse for fake panel\n");
	}

	return ret;
}

int dsi_panel_mipi_freq_update(struct dsi_panel *panel, u8 freq_id)
{
	int i;
	struct dsi_panel_info *panel_info = &panel->panel_info;

	dsi_panel_lock(panel);

	if (freq_id > panel_info->host_info.phy_info.lane_rate_baks_num ||
			freq_id > panel_info->host_info.pixel_clk_rate_baks_num) {
		DSI_ERROR("freq_id %d is over lane rate baks num\n", freq_id);
		dsi_panel_unlock(panel);
		return -EINVAL;
	}

	for (i = 0; i < panel_info->num_modes; i++) {
		if (freq_id > 0 && freq_id <= panel_info->modes[i].timing_baks_num) {
			panel_info->modes[i].cur_timing =
					&panel_info->modes[i].timing_baks[freq_id-1];
		} else {
			panel_info->modes[i].cur_timing = &panel_info->modes[i].timing;
		}
	}

	if (freq_id == 0) {
		panel_info->host_info.phy_info.cur_lane_rate =
				panel_info->host_info.phy_info.lane_rate;
	} else {
		panel_info->host_info.phy_info.cur_lane_rate =
				panel_info->host_info.phy_info.lane_rate_baks[freq_id-1];
	}

	if (freq_id == 0) {
		panel_info->host_info.cur_pixel_clk_rate =
				panel_info->host_info.pixel_clk_rate;
	} else {
		panel_info->host_info.cur_pixel_clk_rate =
				panel_info->host_info.pixel_clk_rate_baks[freq_id-1];
	}

	dsi_panel_unlock(panel);

	return 0;
}

int dsi_panel_reset(struct dsi_panel_info *panel_info)
{
	struct dsi_panel_power_state *gpio_state;
	struct reset_ctrl *reset_ctrl;
	int ret = 0;
	int i;

	if (!panel_info) {
		DSI_ERROR("invalid parameter\n");
		ret = -EINVAL;
		goto error;
	}

	reset_ctrl = &panel_info->reset_ctrl;
	if (!gpio_is_valid(reset_ctrl->gpio)) {
		DSI_ERROR("reset gpio is invalid\n");
		ret = -EINVAL;
		goto error;
	}

	DSI_DEBUG("reset gpio is valid, start process %d state\n",
			reset_ctrl->state_num);

	if (reset_ctrl->state_num > 0) {
		ret = gpio_direction_output(reset_ctrl->gpio,
				reset_ctrl->state_sequence[0].level);
		if (ret) {
			DSI_ERROR("failed to set reset gpio ret=%d\n", ret);
			goto error;
		}

		for (i = 0; i < reset_ctrl->state_num; i++) {
			gpio_state = &reset_ctrl->state_sequence[i];
			gpio_set_value(reset_ctrl->gpio, gpio_state->level);

			if (gpio_state->delay_ms)
				mdelay(gpio_state->delay_ms);

			DSI_DEBUG("%d-th state with level:%d, delay_ms:%d\n",
					i,
					gpio_state->level,
					gpio_state->delay_ms);
		};
	}

error:
	return ret;
}

static int dsi_panel_pinctrl_work_state_set(struct dsi_panel_info *panel_info)
{
	struct dsi_panel_pinctrl_info *pinctrl_info;
	int ret = 0;

	pinctrl_info = &panel_info->pinctrl_info;
	if (IS_ERR_OR_NULL(pinctrl_info->pinctrl_ptr)) {
		DSI_INFO("get null pinctrl");
		return ret;
	}

	ret = pinctrl_select_state(pinctrl_info->pinctrl_ptr,
			pinctrl_info->work_state);
	if (ret)
		DSI_ERROR("faile to set pinctrl to work state\n");
	else
		DSI_DEBUG("set pinctrl as work state\n");

	return ret;
}

static int dsi_panel_pinctrl_sleep_state_set(struct dsi_panel_info *panel_info)
{
	struct dsi_panel_pinctrl_info *pinctrl_info;
	int ret = 0;

	pinctrl_info = &panel_info->pinctrl_info;
	if (IS_ERR_OR_NULL(pinctrl_info->pinctrl_ptr)) {
		DSI_INFO("get null pinctrl");
		return 0;
	}

	ret = pinctrl_select_state(pinctrl_info->pinctrl_ptr,
			pinctrl_info->sleep_state);
	if (ret)
		DSI_ERROR("faile to set pinctrl to work state\n");
	else
		DSI_DEBUG("set pinctrl as sleep state\n");

	return ret;
}

static int dsi_panel_power_state_set(struct dsi_panel_power_supply *info,
	bool is_on_state)
{
	struct dsi_panel_power_state *state;
	int ret = 0;

	state = is_on_state ? &info->on_state : &info->off_state;
	switch (info->power_type) {
	case DSI_PANEL_POWER_TYPE_GPIO:
		ret = gpio_direction_output(info->gpio_id, state->level);
		if (ret)
			DSI_ERROR("failed to set gpio %s\n", info->name);
		break;
	case DSI_PANEL_POWER_TYPE_LDO:
		if (state->level)
			ret = regulator_enable(info->regulator);
		else
			ret = regulator_disable(info->regulator);
		if (ret)
			DSI_ERROR("failed to set regulator %s\n", info->name);
		break;
	default:
		DSI_ERROR("power_type is invalid, %s\n", info->name);
		ret = -EINVAL;
	}

	if (!ret) {
		mdelay(state->delay_ms);
		DSI_DEBUG("set %s as level %d, delay_ms: %d\n", info->name,
				state->level, state->delay_ms);
	}
	return ret;
}

static int dsi_panel_power_on(struct dsi_panel *panel)
{
	struct dsi_panel_power_seq *power_seq;
	struct dsi_panel_power_supply *power_supply_info;
	struct dsi_panel_info *panel_info;
	int supply_info_num, ret;
	int i = 0;

	if (!panel) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	panel_info = &panel->panel_info;
	if (panel_info->is_fake_panel) {
		DSI_INFO("bypass the panel power_on operaction for fake panel\n");
		return 0;
	}

	ret = dsi_panel_pinctrl_work_state_set(panel_info);
	if (ret) {
		DSI_ERROR("failed to set pinctrl to work state, ret = %d\n", ret);
		return -EINVAL;
	}

	power_seq = &panel_info->power_seq;
	supply_info_num = power_seq->supply_info_num;
	for (i = 0; i < supply_info_num; ++i) {
		power_supply_info = &power_seq->power_supply_infos[i];
		ret = dsi_panel_power_state_set(power_supply_info, true);
		if (ret) {
			DSI_ERROR("failed to set %s as power on\n",
					power_supply_info->name);
			goto error;
		}
	}

	if (panel_info->bl_setting_type == DSI_PANEL_BL_UPDATE_METHOD_I2C)
			ktz8866_power_on();

	if (!panel_info->reset_ctrl.reset_after_lp11) {
		ret = dsi_panel_reset(panel_info);
		if (ret) {
			DSI_ERROR("failed to reset panel\n");
			goto error;
		}
	}

	return 0;
error:
	while (--i >= 0) {
		power_supply_info = &power_seq->power_supply_infos[i];
		dsi_panel_power_state_set(power_supply_info, false);
	}
	(void)dsi_panel_pinctrl_sleep_state_set(panel_info);
	return ret;
}

static int dsi_panel_power_off(struct dsi_panel *panel)
{
	struct dsi_panel_info *panel_info;
	struct dsi_panel_power_supply *power_supply_info;
	struct dsi_panel_power_seq *power_seq;
	int supply_info_num, i;
	int ret = 0;
#ifdef MI_DISPLAY_MODIFY
	struct mi_dsi_panel_cfg *mi_cfg;
#endif

	if (!panel) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	panel_info = &panel->panel_info;
	if (panel_info->is_fake_panel) {
		DSI_INFO("bypass the panel power_off operaction for fake panel\n");
		return 0;
	}

#ifdef MI_DISPLAY_MODIFY
	mi_cfg = &panel->mi_panel.mi_cfg;
	if (mi_cfg->reset_post_off_delay) {
		usleep_range(mi_cfg->reset_post_off_delay * 1000,
				(mi_cfg->reset_post_off_delay * 1000 + 10));
	}
#endif

	if (panel_info->reset_ctrl.delay_ms_before_reset_off)
		mdelay(panel_info->reset_ctrl.delay_ms_before_reset_off);

	ret = gpio_direction_output(panel_info->reset_ctrl.gpio, 0);
	if (ret)
		DSI_ERROR("failed to set reset gpio %d as 0 ret=%d\n",
				panel_info->reset_ctrl.gpio, ret);

	power_seq = &panel_info->power_seq;
	supply_info_num = power_seq->supply_info_num;

	if (panel_info->bl_setting_type == DSI_PANEL_BL_UPDATE_METHOD_I2C)
		ktz8866_power_off();

	for (i = supply_info_num - 1; i >= 0; --i) {
		power_supply_info = &power_seq->power_supply_infos[i];
		ret = dsi_panel_power_state_set(power_supply_info, false);
		if (ret)
			DSI_ERROR("failed to set %s as power off\n",
					power_supply_info->name);
	}

	ret = dsi_panel_pinctrl_sleep_state_set(panel_info);
	if (ret)
		DSI_ERROR("failed to set pinctrl to sleep state, ret = %d\n", ret);

	return ret;
}

void dsi_panel_power_get(struct drm_connector *connector)
{
	struct dsi_panel_power_mgr *power_mgr;
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;
	int ret;

	if (!connector) {
		DSI_ERROR("invalid parameters, %pK\n", connector);
		return;
	}

	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;
	power_mgr = display->dsi_panel_power_mgr;

	DSI_DEBUG("dsi panel power count is %d\n", power_mgr->count);

	mutex_lock(&power_mgr->lock);
	power_mgr->count++;
	if (power_mgr->count == 1) {
		if (!is_cont_display_enabled()) {
			ret = dsi_panel_power_on(display->panel);
			if (ret) {
				DSI_ERROR("panel power on failed\n");
				goto exit;
			}
		}
		DSI_DEBUG("dsi panel power on\n");
	}

exit:
	mutex_unlock(&power_mgr->lock);
}

void dsi_panel_power_put(struct drm_connector *connector)
{
	struct dsi_panel_power_mgr *power_mgr;
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;

	if (!connector) {
		DSI_ERROR("invalid parameters, %pK\n", connector);
		return;
	}

	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;
	power_mgr = display->dsi_panel_power_mgr;

	if (WARN_ON(power_mgr->count == 0))
		return;

	DSI_DEBUG("dsi panel power count is %d\n", power_mgr->count);

	mutex_lock(&power_mgr->lock);
	power_mgr->count--;
	if (power_mgr->count == 0) {
		dsi_panel_power_off(display->panel);
		DSI_DEBUG("dsi panel power off\n");
	}
	mutex_unlock(&power_mgr->lock);
}

bool is_dsi_panel_powered_on(struct dsi_panel_power_mgr *power_mgr)
{
	if (!power_mgr) {
		DSI_ERROR("invalid parameters\n");
		return false;
	}

	if (power_mgr->count > 0)
		return true;
	else
		return false;
}

static void dsi_panel_device_release(struct device *dev)
{
	DSI_INFO("release dsi panel device\n");
}

struct dsi_panel *dsi_panel_create(struct device_node *dev_node, bool is_asic,
		const char *type)
{
	struct dsi_backlight_config *bl_config;
	struct dsi_panel *panel;
	struct device *panel_dev;
	int ret;

	if (!dev_node) {
		DSI_ERROR("invalid panel dev_node\n");
		return ERR_PTR(-EINVAL);
	}

	panel = kzalloc(sizeof(*panel), GFP_KERNEL);
	if (!panel)
		return ERR_PTR(-ENOMEM);

	panel->panel_info.is_asic = is_asic;
	panel_dev = &panel->mipi_device.dev;
	panel_dev->of_node = dev_node;
	panel_dev->release = dsi_panel_device_release;
	/* TODO: Support dual-panel later */
	ret = dev_set_name(panel_dev, "dsi_panel");
	if (ret)
		DSI_ERROR("failed to set panel device name\n");

	ret = device_register(panel_dev);
	if (ret)
		DSI_ERROR("failed to regist panel device\n");

	ret = dsi_panel_info_parse(panel);
	if (ret)
		DSI_ERROR("failed to parse panel info, ret=%d\n", ret);

	mutex_init(&panel->panel_lock);
	panel->panel_info.current_mode = NULL;
	panel->panel_info.old_mode = NULL;
	panel->panel_info.te_scanline = -1;
	drm_panel_init(&panel->drm_panel, &panel->mipi_device.dev,
			NULL, DRM_MODE_CONNECTOR_DSI);
	drm_panel_add(&panel->drm_panel);

	backlight_ktz8866_init();

#ifdef MI_DISPLAY_MODIFY
	panel->mi_panel.type = type;
	mi_dsi_panel_init(panel);
#endif

	ret = dsi_cmd_mgr_init(panel);
	if (ret)
		DSI_ERROR("failed to init dsi_cmd_mgr, ret = %d\n", ret);

	atomic_set(&panel->panel_recovery_pending, 0);
	panel->panel_dead = false;
	panel->panel_info.esd_info.err_irq_enabled = false;
	panel->cmd_mgr_enabled = false;
	bl_config = &panel->panel_info.bl_config;
	if (is_cont_display_enabled()) {
		panel->initialized = true;
		if (panel->panel_info.num_modes <= PREFERRED_MODE_INDEX) {
			DSI_WARN("invalid preferred mode index %d, num_modes %d\n",
					PREFERRED_MODE_INDEX,
					panel->panel_info.num_modes);
		} else {
			dsi_panel_mode_update(panel,
					&panel->panel_info.modes[PREFERRED_MODE_INDEX]);
			panel->panel_info.old_mode = panel->panel_info.current_mode;
		}
		bl_config->current_brightness = bl_config->brightness_init_level;
	} else {
		panel->initialized = false;
		bl_config->current_brightness = 0;
	}

	return panel;
}

static inline void dsi_panel_esd_info_release(struct dsi_panel_esd_info *esd_info)
{
	kfree(esd_info->status_length);
	kfree(esd_info->status_value);
	kfree(esd_info->status_buf);
}

void dsi_panel_destroy(struct dsi_panel *panel)
{
	if (!panel)
		return;

	dsi_cmd_mgr_deinit();

#ifdef MI_DISPLAY_MODIFY
	mi_dsi_panel_deinit(panel);
#endif

	dsi_panel_modes_destroy(panel);

	dsi_panel_gpio_release(panel);
	kfree(panel->panel_info.power_seq.power_supply_infos);
	if (!IS_ERR_OR_NULL(panel->panel_info.pinctrl_info.pinctrl_ptr))
		pinctrl_put(panel->panel_info.pinctrl_info.pinctrl_ptr);

	kfree(panel->panel_info.host_info.dsc_cfg.customized_pps_table);

	dsi_panel_esd_info_release(&panel->panel_info.esd_info);

	drm_panel_remove(&panel->drm_panel);

	device_unregister(&panel->mipi_device.dev);
	kfree(panel);
}
