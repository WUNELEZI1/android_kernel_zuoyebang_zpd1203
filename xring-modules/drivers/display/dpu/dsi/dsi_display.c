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

#include <linux/component.h>
#include <linux/backlight.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/of.h>

#include "dsi_panel_event_notifier.h"
#include "dpu_cont_display.h"
#include "dpu_module_drv.h"
#include "dpu_backlight.h"
#include "dsi_connector.h"
#include "dsi_encoder.h"
#include "dsi_display.h"
#include "dsi_power.h"
#include "dsi_host.h"
#include "dpu_log.h"
#include "dpu_kms.h"
#include "dpu_drv.h"
#include "dpu_flow_ctrl.h"
#include "dpu_obuf.h"
#include "backlight_ktz8866.h"
#include "dpu_power_mgr.h"
#include "dsi_power.h"
#include "dpu_trace.h"
#include "dpu_fence_core.h"
#include "dpu_power_helper.h"

#ifdef MI_DISPLAY_MODIFY
#include "mi_dsi_display.h"
#endif

#define CHIP_ID_GPIO_NUM 2
#define DSI_ESD_DISABLE_PERIOD 10000

/* Note: only for debug purpose, will be removed later */
bool g_power_off_dsi_panel = true;

static struct dsi_display *disp_slots[DSI_DISPLAY_MAX];

int dsi_display_finish(struct dsi_display *display)
{
	struct dsi_panel *panel;

	if (unlikely(!display)) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	panel = display->panel;
	panel->panel_info.old_mode = panel->panel_info.current_mode;
	DSI_DEBUG("%s-display update old mode as %d\n",
			dsi_display_type_name_get(display->display_type),
			panel->panel_info.old_mode->timing.framerate);
	return 0;
}

int dsi_display_mipi_cmd_state_get(struct dsi_display *display)
{
	int ret_val, ret = 0;
	int try_again = 2;
	u32 frame_timeout_ms;

	/* frame_timeout_ms = 3 * current vsync period */
	display_frame_timeout_get(display, &frame_timeout_ms);
	while (try_again--) {
		ret = read_poll_timeout(atomic_read, ret_val, (ret_val == 0),
				10, frame_timeout_ms * 1000, false,
				&display->mipi_cmd_state);
		if (!ret) {
			spin_lock(&display->mipi_cmd_lock);
			if (!atomic_read(&display->mipi_cmd_state)) {
				atomic_inc(&display->mipi_cmd_state);
				DPU_DEBUG("get mipi_cmd_state\n");
				spin_unlock(&display->mipi_cmd_lock);
				return 0;
			}
			spin_unlock(&display->mipi_cmd_lock);
			DSI_WARN("failed to get mipi_cmd_state, try again %d\n", try_again);
		} else {
			DSI_WARN("failed to wait mipi_cmd_state in %d ms\n", frame_timeout_ms);
			return -ETIMEDOUT;
		}
	}
	DSI_WARN("failed to get mipi_cmd_state, try again %d\n", try_again);
	return -ETIMEDOUT;
}

int dsi_display_mipi_cmd_state_clear(struct dsi_display *display)
{
	spin_lock(&display->mipi_cmd_lock);
	if (!atomic_read(&display->mipi_cmd_state)) {
		DSI_WARN("failed to clear mipi tx state\n");
		spin_unlock(&display->mipi_cmd_lock);
		return -EINVAL;
	}
	atomic_dec(&display->mipi_cmd_state);
	DPU_DEBUG("release mipi_cmd_state\n");
	spin_unlock(&display->mipi_cmd_lock);
	return 0;
}

static int __dsi_display_enable(struct dsi_display *display, bool dsi_panel_need_power_on)
{
	struct dsi_ctrl_cfg ctrl_cfg = {0};
	struct dsi_panel_info *panel_info;
	struct dsi_ctrl *ctrl;
	int ret = 0;
	int i;

	if (dsi_display_initialized_get(display)) {
		DSI_INFO("dsi display has already enabled by another thread, bypass\n");
		return 0;
	}
	panel_info = &display->panel->panel_info;
	dsi_display_ctrl_cfg_get(display, &ctrl_cfg);
	/* Init phy ready */
	display_for_each_active_port(i, display) {
		ctrl = display->port[i].ctrl;
		ret = dsi_ctrl_hw_init(ctrl, &ctrl_cfg);
		if (ret)
			DSI_ERROR("dsi ctrl-%d hw init failled\n", i);
		else
			DSI_INFO("dsi ctrl-%d hw init success\n", i);
	}

	if (dsi_panel_need_power_on && panel_info->reset_ctrl.reset_after_lp11) {
		ret = dsi_panel_reset(panel_info);
		if (ret)
			DSI_ERROR("failed to reset panel\n");
	}

	/* Auto calculation */
	display_for_each_active_port(i, display) {
		ctrl = display->port[i].ctrl;
		ret = dsi_ctrl_hw_wait_ready(ctrl, &ctrl_cfg);
		if (ret)
			DSI_ERROR("dsi ctrl-%d hw init failled\n", i);
		else
			DSI_INFO("dsi ctrl-%d hw init success\n", i);
	}

	if (dsi_panel_need_power_on) {
		ret = dsi_panel_enable(display->panel);
		if (ret) {
			DSI_ERROR("failed to disable cmd panel\n");
			goto error;
		}
		dsi_display_esd_ctrl_locked(display, true);
	}

	dsi_display_tmg_init(display);

	atomic_set(&display->mipi_dyn_freq.dyn_freq_state, DSI_MIPI_DYN_FREQ_POWERON);
	dsi_display_initialized_set(display, true);
error:
	return ret;
}

int dsi_display_cmd_transfer(struct dsi_display *display,
		struct dsi_cmd_desc *cmd_desc, u32 transfer_flag)
{
	int ret;

	if (!display || !cmd_desc) {
		DSI_ERROR("invalid paramters %pK, %pK\n", display, cmd_desc);
		return -EINVAL;
	}

	dsi_display_lock(display);
	dsi_panel_lock(display->panel);
	ret = dsi_display_cmd_transfer_locked(display, cmd_desc, transfer_flag);
	dsi_panel_unlock(display->panel);
	dsi_display_unlock(display);
	return ret;
}

static void dsi_display_cmd_power_prepare_locked(struct dsi_display *display)
{
	struct dsi_connector *dsi_conn;

	dsi_conn = to_dsi_connector(display->connector);
	/* dsi power up */
	dsi_power_get(display->connector);
	dpu_power_get_helper(BIT(DPU_PARTITION_0));
	DSI_DEBUG("display init state: %d, panel init state: %d\n",
			dsi_display_initialized_get(display), dsi_panel_initialized(display->panel));
	if (!dsi_display_initialized_get(display) && dsi_panel_initialized(display->panel))
		__dsi_display_enable(display, false);
}

static void dsi_display_cmd_power_unprepare_locked(struct dsi_display *display)
{
	/* unvote power on for dsi*/
	dpu_power_put_helper(BIT(DPU_PARTITION_0));
	if (dsi_power_put(display->connector))
		dsi_display_initialized_set(display, false);
}

int dsi_display_cmd_transfer_locked(struct dsi_display *display,
		struct dsi_cmd_desc *cmd_desc, u32 transfer_flag)
{
	struct dsi_connector *dsi_conn;
	int ret;

	if (!display || !cmd_desc) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", display, cmd_desc);
		return -EINVAL;
	}

	dsi_conn = to_dsi_connector(display->connector);
	if (dsi_conn->current_power_mode == DPU_POWER_MODE_OFF && !is_cont_display_enabled()) {
		DSI_WARN("cmd set send in power off is not supported, with cont_display_enabled = %d\n",
				is_cont_display_enabled());
		return 0;
	}

	dsi_display_cmd_power_prepare_locked(display);

	ret = dsi_host_cmd_transfer(&display->host, cmd_desc, USE_CPU, transfer_flag);
	if (ret)
		DSI_ERROR("failed to transfer cmd desc\n");

	dsi_display_cmd_power_unprepare_locked(display);

	return ret;

}

int dsi_display_cmd_set_send(struct dsi_display *display,
		enum dsi_cmd_set_type type,
		struct dsi_cmd_set *cmd_set,
		u8 transfer_type)
{
	struct dsi_connector *dsi_conn;
	int ret;

	if (unlikely(!display)) {
		DSI_ERROR("invalid parameters, %pK\n", display);
		return -EINVAL;
	}

	dsi_conn = to_dsi_connector(display->connector);
	dsi_display_lock(display);
	dsi_panel_lock(display->panel);
	ret = dsi_display_cmd_set_send_locked(display, type, cmd_set, transfer_type);
	dsi_panel_unlock(display->panel);
	dsi_display_unlock(display);

	return ret;
}

int dsi_display_cmd_set_send_locked(struct dsi_display *display,
		enum dsi_cmd_set_type type,
		struct dsi_cmd_set *cmd_set,
		u8 transfer_type)
{
	struct dsi_connector *dsi_conn;
	int ret;

	if (unlikely(!display)) {
		DSI_ERROR("invalid parameters, %pK\n", display);
		return -EINVAL;
	}

	dsi_conn = to_dsi_connector(display->connector);
	if (dsi_conn->current_power_mode == DPU_POWER_MODE_OFF && !is_cont_display_enabled()) {
		DSI_WARN("%s send in power off is not supported, with cont_display_enabled = %d\n",
				dsi_panel_cmd_set_name_get(type), is_cont_display_enabled());
		return 0;
	}

	if (dsi_conn->current_power_mode == DPU_POWER_MODE_DOZE_SUSPEND &&
			transfer_type == USE_CMDLIST) {
		DSI_ERROR("%s send by cmdlist is not supported in suspend\n",
				dsi_panel_cmd_set_name_get(type));
		return -ENOTSUPP;
	}

	dsi_display_cmd_power_prepare_locked(display);

	ret = dsi_panel_cmd_set_send(display->panel, type, cmd_set, transfer_type);
	if (ret)
		DSI_ERROR("failed to send cmd set, ret %d\n", ret);

	dsi_display_cmd_power_unprepare_locked(display);

	return ret;
}


const char *dsi_display_type_name_get(int display_type)
{
	switch (display_type) {
	case DSI_DISPLAY_PRIMARY:
		return "primary";
	case DSI_DISPLAY_SECONDARY:
		return "secondary";
	case DSI_DISPLAY_MAX:
		return "max";
	default:
		return "Unknown";
	}
}

static irqreturn_t dsi_display_te_irq_handler(int irq, void *data)
{
	struct dsi_display *display = (struct dsi_display *)data;

	if (!display)
		return IRQ_HANDLED;

	complete_all(&display->te_completion);
	return IRQ_HANDLED;
}

static int dsi_display_te_irq_register(struct dsi_display *display)
{
	struct dsi_panel *panel = display->panel;
	struct dsi_panel_esd_info *esd_info;
	int te_irq, ret;


	esd_info = &panel->panel_info.esd_info;
	if (!gpio_is_valid(esd_info->te_gpio)) {
		DSI_ERROR("invalid te_gpio %d\n", esd_info->te_gpio);
		return -EINVAL;
	}

	init_completion(&display->te_completion);
	te_irq = gpio_to_irq(esd_info->te_gpio);

	irq_set_status_flags(te_irq, IRQ_DISABLE_UNLAZY);

	ret = devm_request_irq(&display->pdev->dev, te_irq,
			dsi_display_te_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
			"TE_GPIO", display);
	if (ret) {
		DSI_ERROR("failed to request te irq, ret %d\n", ret);
		irq_clear_status_flags(te_irq, IRQ_DISABLE_UNLAZY);
		return ret;
	}

	disable_irq(te_irq);
	display->esd_te_irq_enabled = false;
	return ret;
}

static void dsi_display_te_irq_unregister(struct dsi_display *display)
{
	struct dsi_panel_esd_info *esd_info;
	int te_irq;

	esd_info = &display->panel->panel_info.esd_info;
	te_irq = gpio_to_irq(esd_info->te_gpio);
	free_irq(te_irq, display);
}

static void dsi_display_te_irq_ctrl(struct dsi_display *display, bool enable)
{
	struct dsi_panel_esd_info *esd_info;

	if (!display) {
		DSI_ERROR("invalid parameter\n");
		return;
	}

	esd_info = &display->panel->panel_info.esd_info;
	if (!display->esd_te_irq_enabled && enable) {
		enable_irq(gpio_to_irq(esd_info->te_gpio));
		display->esd_te_irq_enabled = true;
	}

	if (display->esd_te_irq_enabled && !enable) {
		disable_irq(gpio_to_irq(esd_info->te_gpio));
		display->esd_te_irq_enabled = false;
	}

	DSI_DEBUG("%s display te irq enabled = %d\n",
			dsi_display_type_name_get(display->display_type),
			display->esd_te_irq_enabled);
}

int dsi_display_te_check_timeout(struct dsi_display *display, u32 timeout_ms)
{
	struct dsi_panel_pinctrl_info *pinctrl_info;
	int ret = 0;

	if (!display) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	if (display->display_info.ctrl_mode == DSI_VIDEO_MODE)
		return 0;

	/* set pin function as gpio */
	pinctrl_info = &display->panel->panel_info.pinctrl_info;
	if (pinctrl_select_state(pinctrl_info->pinctrl_ptr,
			pinctrl_info->te_gpio_state))
		DSI_ERROR("failed to set te gpio pinctrl to work state\n");

	/* prepare */
	ret = dsi_display_te_irq_register(display);
	if (unlikely(ret)) {
		DSI_ERROR("regist TE to irq failed, %d\n", ret);
		return ret;
	}
	dsi_display_te_irq_ctrl(display, true);

	/* wait te irq */
	reinit_completion(&display->te_completion);
	if (!wait_for_completion_timeout(&display->te_completion,
			msecs_to_jiffies(timeout_ms))) {
		ret = -EINVAL;
		DSI_ERROR("check TE failed\n");
	} else {
		DSI_DEBUG("get te irq within: %dms\n", timeout_ms);
	}

	/* unprepare */
	dsi_display_te_irq_ctrl(display, false);
	dsi_display_te_irq_unregister(display);

	if (pinctrl_select_state(pinctrl_info->pinctrl_ptr,
			pinctrl_info->te_work_state))
		DSI_ERROR("failed to set te gpio pinctrl to work state\n");
	DPU_DEBUG("-\n");
	return ret;
}

static int dsi_display_status_reg_check(struct dsi_display *display)
{
	int port_index, ret;

	display_for_each_active_port(port_index, display) {
		/* read panel status register in port i */
		ret = dsi_panel_status_reg_read(display->panel, port_index);
		if (ret) {
			DSI_ERROR("failed to read status reg on port %d\n", port_index);
			return ret;
		}

		/* validate the read back result */
		ret = dsi_panel_status_value_validate(display->panel);
		if (!ret) {
			DSI_ERROR("validate read back result failed on port %d\n", port_index);
			ret = -EINVAL;
			return ret;
		}
	}

	DSI_DEBUG("status reg check success\n");
	return 0;
}

inline void dsi_display_lock(struct dsi_display *display)
{
	static int count;

	DSI_DEBUG("request display lock %d\n", count);
	mutex_lock(&display->display_lock);
	count++;
}

inline void dsi_display_unlock(struct dsi_display *display)
{
	static int count;

	DSI_DEBUG("release display lock %d\n", count);
	mutex_unlock(&display->display_lock);
	count++;
}

int dsi_display_status_check(struct dsi_display *display)
{
	struct dsi_panel_esd_info *esd_info;
	struct dsi_panel *panel;
	u32 wait_timeout;
	int ret = 0;

	if (unlikely(!display)) {
		DSI_ERROR("get invalid parameter, %pK", display);
		return -EINVAL;
	}

	panel = display->panel;
	dsi_display_lock(display);
	if (unlikely(!display->esd_check_enabled)) {
		DSI_INFO("%s-display esd check is disabled\n",
				dsi_display_type_name_get(display->display_type));
		dsi_display_unlock(display);
		return 0;
	}
	dsi_panel_lock(panel);

	if (atomic_read(&panel->panel_recovery_pending)) {
		DSI_INFO("%s display already in panel_recovery_pending\n",
		dsi_display_type_name_get(display->display_type));
		goto exit_with_release_panel_lock;
	}

	if (!dsi_panel_initialized(panel)) {
		DSI_DEBUG("panel is uninitialized\n");
		goto exit_with_release_panel_lock;
	}

	esd_info = &panel->panel_info.esd_info;
	if ((esd_info->esd_mode == DSI_PANEL_ESD_MODE_SW_SUCCESS) ||
			panel->panel_info.is_fake_panel) {
		DSI_DEBUG("sw_success esd mode or fake panel\n");
		ret = 0;
		goto exit_with_release_panel_lock;
	}

	if (esd_info->esd_mode == DSI_PANEL_ESD_MODE_SW_FAILURE) {
		ret = -EINVAL;
		goto exit_with_release_panel_lock;
	}

	DSI_DEBUG("%s display esd check begin\n",
			dsi_display_type_name_get(display->display_type));

	dsi_display_cmd_power_prepare_locked(display);
	ret = dsi_display_status_reg_check(display);
	dsi_display_cmd_power_unprepare_locked(display);
	if (!ret) {
		DSI_DEBUG("%s display status reg check success\n",
				dsi_display_type_name_get(display->display_type));
	} else {
		DSI_ERROR("%s display status reg check failed, recheck te\n",
				dsi_display_type_name_get(display->display_type));
		display_frame_timeout_get(display, &wait_timeout);
		ret = dsi_display_te_check_timeout(display, wait_timeout);
		if (ret)
			DSI_ERROR("%s display double check te failed\n",
					dsi_display_type_name_get(display->display_type));
	}

	if (ret)
		atomic_set(&panel->panel_recovery_pending, 1);

exit_with_release_panel_lock:
	dsi_panel_unlock(panel);
	dsi_display_unlock(display);
	return ret;
}

static inline int dsi_display_esd_irq_ctrl(struct dsi_display *display, bool enable)
{
	if (!display) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	dsi_panel_esd_irq_ctrl(display->panel, enable);
	return 0;
}

static void dsi_display_esd_work_ctrl(struct dsi_display *display,
		bool enable)
{	u32 period;

	if (enable) {
		period = display->panel->panel_info.esd_info.period;
		schedule_delayed_work(&display->esd_check_work,
				msecs_to_jiffies(period));
	} else {
		cancel_delayed_work_sync(&display->esd_check_work);
	}
}

void dsi_display_esd_ctrl(struct dsi_display *display, bool enable)
{
	if (unlikely(!display)) {
		DSI_ERROR("get invalid parameter\n");
		return;
	}

	dsi_display_lock(display);
	dsi_display_esd_ctrl_locked(display, enable);
	dsi_display_unlock(display);
}

void dsi_display_esd_ctrl_locked(struct dsi_display *display, bool enable)
{
	struct dsi_panel_esd_info *esd_info;
	struct dsi_connector_state *dsi_state;

	if (unlikely(!display)) {
		DSI_ERROR("get invalid parameter\n");
		return;
	}

	dsi_state = to_dsi_connector_state(display->connector->state);
	esd_info = &display->panel->panel_info.esd_info;
	if (!esd_info->enable) {
		DSI_DEBUG("esd is disabled return\n");
		return;
	}

	dsi_panel_lock(display->panel);
	if (enable == display->esd_check_enabled) {
		DSI_INFO("%s display esd check is already %s\n",
				dsi_display_type_name_get(display->display_type),
				enable ? "enabled" : "disabled");
		goto exit_with_release_lock;
	}

	if (enable && !dsi_panel_initialized(display->panel)) {
		DSI_INFO("panel is uninitialized, failed to enable esd\n");
		goto exit_with_release_lock;
	}

	switch (esd_info->esd_mode) {
	case DSI_PANEL_ESD_MODE_ERR_INT:
		dsi_display_esd_irq_ctrl(display, enable);
		break;
	case DSI_PANEL_ESD_MODE_ERR_INT_AND_REG_READ:
		dsi_display_esd_irq_ctrl(display, enable);
		dsi_display_esd_work_ctrl(display, enable);
		break;
	case DSI_PANEL_ESD_MODE_SW_SUCCESS:
	case DSI_PANEL_ESD_MODE_SW_FAILURE:
	case DSI_PANEL_ESD_MODE_PANEL_TE:
	case DSI_PANEL_ESD_MODE_REG_READ:
		dsi_display_esd_work_ctrl(display, enable);
		break;
	default:
		DSI_ERROR("get invalid esd mode %d\n", esd_info->esd_mode);
	}

	display->esd_check_enabled = enable;
exit_with_release_lock:
	dsi_panel_unlock(display->panel);
	DSI_DEBUG("%s display esd mode %s %s success\n",
			dsi_display_type_name_get(display->display_type),
			dsi_panel_esd_mode_name_get(esd_info->esd_mode),
			enable ? "enable" : "disable");
}

struct dsi_display *dsi_display_get(u8 index)
{
	if (index >= DSI_DISPLAY_MAX)
		return NULL;

	return disp_slots[index];
}

int dsi_display_post_init(void __iomem *dpu_iomem_ptr)
{
	struct dsi_display *display;
	struct dsi_ctrl *ctrl;
	struct dsi_tmg *tmg;
	int i, j;

	if (!dpu_iomem_ptr) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	for (i = 0; i < DSI_DISPLAY_MAX; i++) {
		display = dsi_display_get(i);
		if (IS_ERR_OR_NULL(display))
			continue;

		display_for_each_port(j, display) {
			tmg = display->port[j].tmg;
			ctrl = display->port[j].ctrl;
			tmg->tmg_hw.hw_blk.iomem_base = dpu_iomem_ptr;
			ctrl->ctrl_hw.dctrl_blk.iomem_base = dpu_iomem_ptr;
			ctrl->ctrl_hw.dsctrl_blk.iomem_base = dpu_iomem_ptr;
		}

		DSI_DEBUG("post init display-%d success\n", i);
	}

	return 0;
}

char *get_dpu_power_mode_name(int power_mode)
{
	switch (power_mode) {
	case DPU_POWER_MODE_ON:
		return "On";
	case DPU_POWER_MODE_DOZE:
		return "Doze";
	case DPU_POWER_MODE_DOZE_SUSPEND:
		return "Suspend";
	case DPU_POWER_MODE_OFF:
		return "Off";
	default:
		return "Unknown";
	}
}

static int dsi_display_tmg_cmd_vrr_cfg_updte(struct dsi_panel_info *panel_info,
		struct dsi_tmg_cfg *tmg_cfg)
{
	struct dsi_panel_mode *cur_mode = panel_info->current_mode;
	int ret = 0;

	tmg_cfg->vrr_cmd.cmd_man_mode = 0;
	tmg_cfg->vrr_cmd.cmd_auto_en = 0;
	switch (cur_mode->vrr_type) {
	case DSI_VRR_LONG_V:
		tmg_cfg->vrr_cmd.cmd_skip_te_next = 0;
		tmg_cfg->vrr_mode = TMG_VRR_LONG_V_MODE;
		break;
	case DSI_VRR_SKIP_FRAME:
		tmg_cfg->vrr_cmd.cmd_skip_te_next = cur_mode->skip_frame_num;
		tmg_cfg->vrr_mode = TMG_VRR_SKIP_FRAME_MODE;
		break;
	default:
		DSI_ERROR("unsupported vrr type %d\n", cur_mode->vrr_type);
		ret = -ENOTSUPP;
		break;
	}

	return ret;
}

static int dsi_display_tmg_vid_vrr_cfg_updte(struct dsi_panel_info *panel_info,
		struct dsi_tmg_cfg *tmg_cfg)
{
	struct dsi_panel_mode *cur_mode = panel_info->current_mode;
	int ret = 0;

	switch (cur_mode->vrr_type) {
	case DSI_VRR_LONG_V:
		tmg_cfg->vrr_vid.video_skip_frame_num = 0;
		tmg_cfg->vrr_mode = TMG_VRR_LONG_V_MODE;
		break;
	case DSI_VRR_SKIP_FRAME:
		tmg_cfg->vrr_vid.video_skip_frame_num = cur_mode->skip_frame_num;
		tmg_cfg->vrr_mode = TMG_VRR_SKIP_FRAME_MODE;
		break;
	case DSI_VRR_LONG_H:
		tmg_cfg->vrr_vid.video_skip_frame_num = 0;
		tmg_cfg->vrr_mode = TMG_VRR_LONG_H_MODE;
		break;
	default:
		DSI_ERROR("unsupported vrr type %d\n", cur_mode->vrr_type);
		ret = -ENOTSUPP;
		break;
	}

	return ret;
}

static void dsi_display_tmg_timing_cfg_update(struct dsi_sync_timing *tmg_timing,
		struct dsi_panel_timing *panel_timing)
{
	tmg_timing->hsa = panel_timing->hpw;
	tmg_timing->hbp = panel_timing->hbp;
	tmg_timing->hfp = panel_timing->hfp;
	tmg_timing->hact = panel_timing->hdisplay;
	tmg_timing->vsa = panel_timing->vpw;
	tmg_timing->vbp = panel_timing->vbp;
	tmg_timing->vfp = panel_timing->vfp;
	tmg_timing->vact = panel_timing->vdisplay;
}

static void dsi_display_tmg_vrr_cfg_dump(struct dsi_tmg_cfg *tmg_cfg)
{
	DSI_DEBUG("vrr_en: %d\n", tmg_cfg->vrr_en);
	DSI_DEBUG("vrr_mode: %d\n", tmg_cfg->vrr_mode);
	DSI_DEBUG("at_once: %d\n", tmg_cfg->at_once);
	DSI_DEBUG("vrr_cmd.cmd_skip_te_next: %d\n",
			tmg_cfg->vrr_cmd.cmd_skip_te_next);
	DSI_DEBUG("vrr_cmd.cmd_man_mode: %d\n", tmg_cfg->vrr_cmd.cmd_man_mode);
	DSI_DEBUG("vrr_cmd.cmd_auto_en: %d\n", tmg_cfg->vrr_cmd.cmd_auto_en);
	DSI_DEBUG("vrr_vid.video_em_unit_num: %d\n",
			tmg_cfg->vrr_vid.video_em_unit_num);
	DSI_DEBUG("vrr_vid.video_sync_opt: %d\n",
			tmg_cfg->vrr_vid.video_sync_opt);
	DSI_DEBUG("vrr_vid.video_em_next_num: %d\n",
			tmg_cfg->vrr_vid.video_em_next_num);
	DSI_DEBUG("vrr_vid.video_skip_frame_num: %d\n",
			tmg_cfg->vrr_vid.video_skip_frame_num);
	DSI_DEBUG("vrr_vid.video_frame_em_num: %d\n",
			tmg_cfg->vrr_vid.video_frame_em_num);
	DSI_DEBUG("vrr_vid.video_det_te_en: %d\n",
			tmg_cfg->vrr_vid.video_det_te_en);

	DSI_DEBUG("cmdlist_flag: %d\n", tmg_cfg->cmdlist_flag);
	DSI_DEBUG("fps_base: %d\n", tmg_cfg->fps_base);
	DSI_DEBUG("fps_update: %d\n", tmg_cfg->fps_update);
	DSI_DEBUG("current_base: %d\n", tmg_cfg->current_base);
	DSI_DEBUG("base.base0.hsa: %d\n", tmg_cfg->base.base0.hsa);
	DSI_DEBUG("base.base0.hbp: %d\n", tmg_cfg->base.base0.hbp);
	DSI_DEBUG("base.base0.hfp: %d\n", tmg_cfg->base.base0.hfp);
	DSI_DEBUG("base.base0.hact: %d\n", tmg_cfg->base.base0.hact);
	DSI_DEBUG("base.base0.vsa: %d\n", tmg_cfg->base.base0.vsa);
	DSI_DEBUG("base.base0.vbp: %d\n", tmg_cfg->base.base0.vbp);
	DSI_DEBUG("base.base0.vfp: %d\n", tmg_cfg->base.base0.vfp);
	DSI_DEBUG("base.base0.vact: %d\n", tmg_cfg->base.base0.vact);

	DSI_DEBUG("partial_en: %d\n", tmg_cfg->partial_en);
	if (tmg_cfg->partial_en) {
		DSI_DEBUG("xs: %d\n", tmg_cfg->partial.xs);
		DSI_DEBUG("xs: %d\n", tmg_cfg->partial.ys);
		DSI_DEBUG("width: %d\n", tmg_cfg->partial.width);
		DSI_DEBUG("height: %d\n", tmg_cfg->partial.height);
	}
}

int dsi_display_tmg_cfg_update(struct dsi_display *display, u8 transfer_type)
{
	struct dsi_panel_timing *panel_timing;
	struct dsi_panel_info *panel_info;
	struct dsi_tmg_cfg *tmg_cfg;
	int ret = 0;

	if (!display) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	tmg_cfg = &display->tmg_cfg;
	panel_info = &display->panel->panel_info;
	panel_timing = panel_info->current_mode->cur_timing;
	tmg_cfg->current_base = 0;
	tmg_cfg->cmd_wait_data_en = g_cmd_wait_data_en;
	dsi_display_tmg_timing_cfg_update(&tmg_cfg->base.base0, panel_timing);
	tmg_cfg->cmdlist_flag = (transfer_type == USE_CMDLIST) ? CMDLIST_WRITE : DIRECT_WRITE;

	/* Initialize the fps_base = panel_timing->framerate in first frame */
	tmg_cfg->fps_base = panel_timing->te_rate;

	tmg_cfg->fps_update = panel_timing->framerate;
	DPU_DEBUG("panel_timing->te_rate %d, tmg_cfg->fps_update %d\n",
		panel_timing->te_rate, tmg_cfg->fps_update);
	if (panel_info->current_mode->vrr_type == DSI_VRR_NONE) {
		DSI_DEBUG("dsi vrr disabled\n");
		goto exit;
	}

	tmg_cfg->vrr_en = 1;
	if (panel_info->host_info.ctrl_mode == DSI_VIDEO_MODE) {
		ret = dsi_display_tmg_vid_vrr_cfg_updte(panel_info, tmg_cfg);
		if (ret)
			DSI_ERROR("failed to update tmg video vrr config\n");
	} else {
		ret = dsi_display_tmg_cmd_vrr_cfg_updte(panel_info, tmg_cfg);
		if (ret)
			DSI_ERROR("failed to update tmg cmd vrr config\n");
	}

exit:
	dsi_display_tmg_vrr_cfg_dump(tmg_cfg);
	return ret;
}

static void dsi_display_get_obuf_depth_info(struct dsi_display *display,
		enum dsi_port_type port_id, struct obuf_depth_info *depth_info)
{
	struct dsi_panel_info *panel_info;
	struct dsi_panel_timing *timing;
	struct dsc_parms *dsc_cfg;

	if (!display || !depth_info) {
		DSI_INFO("invalid parameters, display:%p, depth_info:%p\n",
				display, depth_info);
		return;
	}

	panel_info = &display->panel->panel_info;
	timing = &panel_info->current_mode->timing;
	dsc_cfg = &display->panel->panel_info.host_info.dsc_cfg;

	depth_info->width = timing->hdisplay;
	depth_info->vtotal = timing->vdisplay + timing->vbp + timing->vfp + timing->vpw;
	depth_info->fps = timing->base_rate;
	depth_info->bpp = dsc_cfg->dsc_en ? dsc_cfg->bits_per_pixel : panel_info->host_info.bpp;
	depth_info->obufen = dpu_obuf_get_obufen((int)port_id);
}

void dsi_display_line_buf_level_cfg(struct dsi_display *display, int port_id)
{
	struct obuf_depth_info depth_info;
	struct dpu_flow_ctrl *flow_ctrl;

	flow_ctrl = dpu_flow_ctrl_get(to_dsi_connector(display->connector)->drm_dev);

	dsi_display_get_obuf_depth_info(display, port_id, &depth_info);
	/* configure obuf level threshold */
	dpu_flow_line_buf_level_cfg(flow_ctrl,
			&display->port[port_id].tmg->tmg_hw.hw_blk,
			FLOW_OBUF_LEVEL_DSI, &depth_info);
}

int dsi_display_panel_timing_switch(struct dsi_display *display)
{
	struct dsi_panel_info *panel_info;
	int ret;

	panel_info = &display->panel->panel_info;

	if (unlikely(!panel_info->current_mode)) {
		DSI_ERROR("get invalid current mode\n");
		return -EINVAL;
	}

	ret = dsi_panel_timing_switch(display->panel);
	if (ret) {
		DSI_ERROR("failed to switch panel timing\n");
		return ret;
	}

	ret = dsi_panel_post_timing_switch(display->panel);
	if (ret) {
		DSI_ERROR("failed to post switch panel timing\n");
		return ret;
	}

	return 0;
}

static void dsi_display_info_init(struct dsi_display *display,
		struct dsi_display_info *info)
{
	struct dsi_panel_info *panel_info;

	panel_info = &display->panel->panel_info;

	info->dsc_en = panel_info->host_info.dsc_cfg.dsc_en;
	info->is_dual_port = panel_info->host_info.sbs_en;
	info->dsi_id = display->port[0].enabled ? 0 : 1;
	info->hdisplay = panel_info->modes[0].timing.hdisplay;
	info->vdisplay = panel_info->modes[0].timing.vdisplay;
	info->width_mm = panel_info->physical_info.width_mm;
	info->height_mm = panel_info->physical_info.height_mm;
	info->tmg_mask = panel_info->host_info.port_mask;
	info->ctrl_mode = panel_info->host_info.ctrl_mode;
	info->pixel_clk_rate = panel_info->host_info.cur_pixel_clk_rate;
	info->ipi_pll_sel = panel_info->host_info.ipi_pll_sel;
	info->boot_up_profile = panel_info->boot_up_profile;
	info->lowpower_ctrl = panel_info->lowpower_ctrl;
	info->idle_policy = panel_info->idle_policy;
	info->frame_power_max_refresh_rate = panel_info->frame_power_max_refresh_rate;
	info->hw_ulps_max_refresh_rate = panel_info->hw_ulps_max_refresh_rate;
	info->frame_power_mask = panel_info->frame_power_mask;

	DSI_DEBUG("dsc_en: %d\n", info->dsc_en);
	DSI_DEBUG("is_dual_port: %d\n", info->is_dual_port);
	DSI_DEBUG("dsi_id: %d\n", info->dsi_id);
	DSI_DEBUG("hdisplay: %d\n", info->hdisplay);
	DSI_DEBUG("vdisplay: %d\n", info->vdisplay);
	DSI_DEBUG("width_mm: %d\n", info->width_mm);
	DSI_DEBUG("height_mm: %d\n", info->height_mm);
	DSI_DEBUG("tmg_mask: %d\n", info->tmg_mask);
	DSI_DEBUG("ctrl_mode: %d\n", info->ctrl_mode);
	DSI_DEBUG("pixel_clk_rate: %lu\n", info->pixel_clk_rate);
	DSI_DEBUG("boot up profile is %d\n", info->boot_up_profile);
	DSI_DEBUG("lowpower_ctrl: 0x%x\n", info->lowpower_ctrl);
	DSI_DEBUG("idle_policy: 0x%x\n", info->idle_policy);
	DSI_DEBUG("frame_power_mask: 0x%x\n", info->frame_power_mask);
	DSI_DEBUG("frame_power_mask_rr: %d\n", info->frame_power_max_refresh_rate);
	DSI_DEBUG("ipi_pll_sel: %d\n", info->ipi_pll_sel);
	DSI_DEBUG("hw_ulps_max_refresh_rate: %d\n", info->hw_ulps_max_refresh_rate);
}

struct dsi_panel_info *dsi_panel_info_get(
		struct drm_connector *connector)
{
	struct dsi_connector *dsi_connector;

	if (!connector) {
		DSI_ERROR("get invalid parameters\n");
		return ERR_PTR(-EINVAL);
	}

	dsi_connector = to_dsi_connector(connector);
	return &dsi_connector->display->panel->panel_info;
}

struct dsi_display_info *dsi_display_info_get(
		struct drm_connector *connector)
{
	struct dsi_connector *dsi_connector;

	if (!connector) {
		DSI_ERROR("get invalid parameters\n");
		return ERR_PTR(-EINVAL);
	}

	dsi_connector = to_dsi_connector(connector);
	return &dsi_connector->display->display_info;
}

int dsi_display_ctrl_mode_get(struct dsi_display *display,
		enum dsi_ctrl_mode *ctrl_mode)
{
	struct dsi_panel_info *panel_info;
	int ret = 0;

	if (!display || !display->panel) {
		DSI_ERROR("get invalid parameters\n");
		ret = -EINVAL;
		goto error;
	}

	panel_info = &display->panel->panel_info;
	*ctrl_mode = panel_info->host_info.ctrl_mode;

error:
	return ret;
}

static bool dsi_display_is_cmd_mode(struct dsi_display *display)
{
	enum dsi_ctrl_mode ctrl_mode;
	int ret;

	ret = dsi_display_ctrl_mode_get(display, &ctrl_mode);
	if (ret) {
		DSI_ERROR("failed to get dsi ctrl mode\n");
		return true;
	}

	if (ctrl_mode == DSI_VIDEO_MODE)
		return false;

	return true;
}

void dsi_display_cmd_flush(struct dsi_display *display)
{
	struct dsi_panel_timing *panel_timing;
	struct dpu_power_ctrl *power_ctrl = NULL;
	struct drm_crtc *crtc = NULL;
	struct dsi_connector *connector;

	connector = to_dsi_connector(display->connector);
	crtc = connector->encoder->crtc;
	if (!crtc)
		return;

	power_ctrl = get_power_ctrl_inst(crtc);
	if (!power_ctrl || !(power_ctrl->lowpower_ctrl & DPU_LP_HW_ULPS) ||
			!is_dpu_lp_enabled(DPU_LP_HW_ULPS))
		return;

	panel_timing = display->panel->panel_info.current_mode->cur_timing;
	if (panel_timing->framerate <= display->display_info.hw_ulps_max_refresh_rate) {
		dpu_force_refresh_event_notify(crtc->dev);
		DSI_INFO("cur frame rate:%u\n", panel_timing->framerate);
	}
}

int dsi_display_backlight_set(struct dsi_display *display,
		u32 backlight_level, u8 bl_transfer_type)
{
	struct dsi_connector_state *dsi_state;
	struct dsi_backlight_config config;
	struct dsi_connector *dsi_conn;
	struct dsi_panel *panel;
	u32 brightness;
	int ret = 0;

	if (!display) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	dsi_conn = to_dsi_connector(display->connector);
	dsi_state = to_dsi_connector_state(display->connector->state);
	panel = display->panel;
	if (panel->panel_info.is_fake_panel) {
		DSI_INFO("bypass the backlight set for fake panel\n");
		return 0;
	}

	/* Make sure the power state is consistent with the hardware state. */
	DSI_DEBUG("set display backlight %d with %s\n",
			backlight_level, bl_transfer_type == USE_CMDLIST ? "cmdlist" : "cpu");
	if (bl_transfer_type == USE_CPU) {
		dsi_display_lock(display);
		dsi_panel_lock(panel);
	}
	if (!dsi_panel_initialized(display->panel)) {
		DSI_WARN("failed to set backlight, panel is uninitialized\n");
		ret = 0;
		goto exit_with_release_lock;
	}

#ifdef MI_DISPLAY_MODIFY
	if (is_partial_aod_and_panel_initialized(panel) &&
			panel->mi_panel.mi_cfg.sf_transfer_power_mode != HAL_POWERMODE_ON &&
			panel->mi_panel.mi_cfg.fp_unlock_value != FINGERPRINT_UNLOCK_SUCCESS) {
		ret = mi_dsi_display_backlight_set_locked(display, backlight_level);
		goto exit_with_release_lock;
	}
#endif

	if (dsi_conn->current_power_mode == DPU_POWER_MODE_DOZE_SUSPEND) {
		DSI_WARN("failed to set backlight in doze suspend\n");
		ret = 0;
		goto exit_with_release_lock;
	}

	config = panel->panel_info.bl_config;
	brightness = backlight_level;
	/* 0 indicate the panel power off event */
	if (backlight_level) {
		if (brightness > config.brightness_max_level)
			brightness = config.brightness_max_level;
#ifdef MI_DISPLAY_MODIFY
		if (config.actual_brightness_max_level && brightness > config.actual_brightness_max_level)
			brightness = config.actual_brightness_max_level;
#endif
		if (brightness < config.brightness_min_level)
			brightness = config.brightness_min_level;
	}

	ret = dsi_panel_backlight_set_locked(panel, brightness,
			bl_transfer_type);
	if (ret)
		DSI_ERROR("failed to set panel backlight\n");

	if (!ret && backlight_level) {
		panel->panel_info.bl_config.brightness = brightness;
		DSI_DEBUG("update backlight as %d\n",
				panel->panel_info.bl_config.brightness);
	}
exit_with_release_lock:
	if (bl_transfer_type == USE_CPU) {
		dsi_panel_unlock(panel);
		dsi_display_unlock(display);
	}

	return ret;
}

static void dsi_display_ctrl_cfg_dump(struct dsi_ctrl_cfg *ctrl_cfg)
{
	if (!ctrl_cfg) {
		DSI_ERROR("invalid parameters\n");
		return;
	}

	DSI_DEBUG("ctrl_mode: %s\n",
			ctrl_cfg->ctrl_mode == DSI_VIDEO_MODE ? "video" : "cmd");
	DSI_DEBUG("fps: %d\n", ctrl_cfg->fps);
	DSI_DEBUG("phy_parms.phy_type: %s\n",
			ctrl_cfg->phy_parms.phy_type == DSI_PHY_TYPE_DPHY ? "DPHY" : "CPHY");
	DSI_DEBUG("phy_parms.ppi_width: %d\n", ctrl_cfg->phy_parms.ppi_width);
	DSI_DEBUG("phy_parms.lanes_num: %d\n", ctrl_cfg->phy_parms.lanes_num);
	DSI_DEBUG("phy_parms.sys_clk_rate: %d\n",
			ctrl_cfg->phy_parms.sys_clk);
	DSI_DEBUG("phy_parms.phy_lp_speed: %d\n",
			ctrl_cfg->phy_parms.phy_lp_speed);
	DSI_DEBUG("phy_parms.phy_hs_speed: %d\n",
			ctrl_cfg->phy_parms.phy_hs_speed);
	DSI_DEBUG("phy_parms.clk_type: %d\n", ctrl_cfg->phy_parms.clk_type);
	DSI_DEBUG("phy_parms.phy_sel: %d\n", ctrl_cfg->phy_parms.phy_sel);
	DSI_DEBUG("pixel_fomat: %d\n", ctrl_cfg->pixel_fomat);
	DSI_DEBUG("ipi_clk: %d\n", ctrl_cfg->ipi_clk);
	DSI_DEBUG("video_mode_type: %d\n", ctrl_cfg->video_mode_type);
}

int dsi_display_ctrl_cfg_get(struct dsi_display *display,
		struct dsi_ctrl_cfg *ctrl_cfg)
{
	struct dsi_panel_timing *max_fps_timing;
	struct dsi_panel_info *panel_info;
	int master_port_index = 0;
	struct dsi_phy *phy;

	if (!display || !ctrl_cfg || !display->panel) {
		DSI_ERROR("get invalid paramerters, display:%p, ctrl_cfg:%p",
				display, ctrl_cfg);
		return -EINVAL;
	}

	panel_info = &display->panel->panel_info;
	/* Initialize DSI with the highest speed timing. */
	max_fps_timing = panel_info->max_fps_mode->cur_timing;
	phy = display->port[master_port_index].phy;

	ctrl_cfg->ctrl_mode = panel_info->host_info.ctrl_mode;
	ctrl_cfg->manual_mode_en = 0;
	ctrl_cfg->auto_calc_en = 1;
	ctrl_cfg->vg_en = 0;
	ctrl_cfg->vg_dpmode = 0;

	ctrl_cfg->phy_parms.phy_type = panel_info->host_info.phy_info.phy_type;
	ctrl_cfg->phy_parms.ppi_width = phy->phy_hw.ppi_width;
	ctrl_cfg->phy_parms.lanes_num = panel_info->host_info.phy_info.lane_num;
	ctrl_cfg->ipi_clk = panel_info->host_info.cur_pixel_clk_rate;
	ctrl_cfg->ipi_clk_div = panel_info->host_info.pixel_clk_div;
	ctrl_cfg->phy_parms.phy_amplitude = panel_info->host_info.phy_info.phy_amplitude;
	ctrl_cfg->phy_parms.phy_oa_setr = panel_info->host_info.phy_info.phy_oa_setr;
	ctrl_cfg->phy_parms.phy_eqa = panel_info->host_info.phy_info.phy_eqa;
	ctrl_cfg->phy_parms.phy_eqb = panel_info->host_info.phy_info.phy_eqb;

	if (!display->is_asic) {
		ctrl_cfg->phy_parms.sys_clk = 20000000;
		ctrl_cfg->phy_parms.phy_lp_speed = 10000000;
	} else {
		ctrl_cfg->phy_parms.sys_clk = 208900000; // 208.9MHz
		ctrl_cfg->phy_parms.phy_lp_speed = 17400000; //17.4MHz
	}
	ctrl_cfg->phy_parms.phy_hs_speed =
			panel_info->host_info.phy_info.cur_lane_rate;

	ctrl_cfg->phy_parms.clk_type = panel_info->host_info.phy_info.clk_type;
	ctrl_cfg->phy_parms.phy_sel = phy->phy_hw.hw_platform;
	ctrl_cfg->bta_en = panel_info->host_info.bta_en;
	ctrl_cfg->eotp_en = panel_info->host_info.eotp_en;
	ctrl_cfg->virtual_channel =
			panel_info->host_info.virtual_channel;

	ctrl_cfg->video_mode_type =
			panel_info->host_info.burst_mode_type;

	ctrl_cfg->pixel_fomat = panel_info->host_info.color_fmt;

	ctrl_cfg->timing.hsa = max_fps_timing->hpw;
	ctrl_cfg->timing.hbp = max_fps_timing->hbp;
	ctrl_cfg->timing.hfp = max_fps_timing->hfp;
	ctrl_cfg->timing.hact = max_fps_timing->hdisplay;

	ctrl_cfg->timing.vsa = max_fps_timing->vpw;
	ctrl_cfg->timing.vbp = max_fps_timing->vbp;
	ctrl_cfg->timing.vfp = max_fps_timing->vfp;
	ctrl_cfg->timing.vact = max_fps_timing->vdisplay;

	dsi_display_ctrl_cfg_dump(ctrl_cfg);
	return 0;
}

static void dsi_display_tmg_cfg_dump(struct dsi_tmg_cfg *tmg_cfg)
{
	DSI_DEBUG("tmg_mode: %d\n", tmg_cfg->tmg_mode);
	DSI_DEBUG("partial_en: %d\n", tmg_cfg->partial_en);
	DSI_DEBUG("cmd_wait_data_en: %d\n", tmg_cfg->cmd_wait_data_en);
	DSI_DEBUG("sbs_en: %d\n", tmg_cfg->sbs_en);
	DSI_DEBUG("ipi_clk: %d\n", tmg_cfg->ipi_clk);
	DSI_DEBUG("te_sel: %d\n", tmg_cfg->te_sel);
	DSI_DEBUG("cmdlist_flag: %d\n", tmg_cfg->cmdlist_flag);
	DSI_DEBUG("fps_base: %d\n", tmg_cfg->fps_base);
	DSI_DEBUG("fps_update: %d\n", tmg_cfg->fps_update);
	DSI_DEBUG("current_base: %d\n", tmg_cfg->current_base);
	DSI_DEBUG("base.base0.hsa: %d\n", tmg_cfg->base.base0.hsa);
	DSI_DEBUG("base.base0.hbp: %d\n", tmg_cfg->base.base0.hbp);
	DSI_DEBUG("base.base0.hfp: %d\n", tmg_cfg->base.base0.hfp);
	DSI_DEBUG("base.base0.hact: %d\n", tmg_cfg->base.base0.hact);
	DSI_DEBUG("base.base0.vsa: %d\n", tmg_cfg->base.base0.vsa);
	DSI_DEBUG("base.base0.vbp: %d\n", tmg_cfg->base.base0.vbp);
	DSI_DEBUG("base.base0.vfp: %d\n", tmg_cfg->base.base0.vfp);
	DSI_DEBUG("base.base0.vact: %d\n", tmg_cfg->base.base0.vact);

	DSI_DEBUG("tmg_cfg->format: %d\n", tmg_cfg->format);
	DSI_DEBUG("tmg_cfg->vrr_mode: %d\n", tmg_cfg->vrr_mode);

	DSI_DEBUG("tmg_cfg->frame_power_en: %d\n", tmg_cfg->frame_power_en);
	DSI_DEBUG("tmg_cfg->frame_power_max_refresh_rate: %d\n",
			tmg_cfg->frame_power_max_refresh_rate);
}

int dsi_display_tmg_cfg_init(struct dsi_display *display,
		struct dsi_tmg_cfg *tmg_cfg)
{
	struct dsi_panel_info *panel_info;


	if (!display || !tmg_cfg) {
		DSI_INFO("get invalid paramerters, display:%p, tmg_cfg:%p",
				display, tmg_cfg);
		return -EINVAL;
	}

	panel_info = &display->panel->panel_info;
	tmg_cfg->tmg_mode = panel_info->host_info.ctrl_mode;
	tmg_cfg->vg_dpmode = 0;
	tmg_cfg->vg.pat_sel = 0;
	tmg_cfg->vg.user_pattern = 0x00FFFF;
	tmg_cfg->cmd_wait_data_en = g_cmd_wait_data_en;
	tmg_cfg->sbs_en = display->display_info.is_dual_port;
	tmg_cfg->ipi_clk = panel_info->host_info.cur_pixel_clk_rate;
	tmg_cfg->te_sel = panel_info->host_info.te_sel;

	tmg_cfg->format = panel_info->host_info.color_fmt;
	tmg_cfg->cmdlist_flag = DIRECT_WRITE;

	tmg_cfg->vrr_en = false;
	tmg_cfg->vrr_mode = CMD_MODE_MANUAL_VRR;

	/* core_mem_lp_auto_en_buf is config in low power init function */
	tmg_cfg->obuf.core_fm_timing_en = 1;
	tmg_cfg->obuf.split_en_buf = 0;

	if ((display->display_info.lowpower_ctrl & DPU_LP_FRAME_POWER_CTRL_ENABLE) &&
			is_dpu_lp_enabled(DPU_LP_FRAME_POWER_CTRL_ENABLE)) {
		tmg_cfg->frame_power_en = true;
		tmg_cfg->frame_power_max_refresh_rate =
				display->display_info.frame_power_max_refresh_rate;
	} else {
		tmg_cfg->frame_power_en = false;
		tmg_cfg->frame_power_max_refresh_rate = 0;
	}

	if ((display->display_info.lowpower_ctrl & DPU_LP_HW_ULPS) &&
			is_dpu_lp_enabled(DPU_LP_HW_ULPS)) {
		tmg_cfg->need_hw_ulps = true;
		tmg_cfg->hw_ulps_max_refresh_rate =
				panel_info->hw_ulps_max_refresh_rate;
	} else {
		tmg_cfg->need_hw_ulps = false;
		tmg_cfg->hw_ulps_max_refresh_rate = 0;
	}

	dsi_display_tmg_cfg_dump(tmg_cfg);

	return 0;
}

int dsi_display_check_ulps_state(struct dsi_display *display, bool enter)
{
	struct dsi_ctrl *ctrl;
	int ret = 0;
	int i;

	if (!display) {
		DSI_ERROR("invalid display parameters\n");
		return -EINVAL;
	}

	display_for_each_active_port(i, display) {
		ctrl = display->port[i].ctrl;
		ret = dsi_ctrl_check_ulps_state(ctrl, enter);
		DSI_INFO("dsi-%d check %s ulps state, ret=%d",
				i, enter ? "enter" : "exit", ret);
	}

	return ret;
}

int dsi_display_ulps_ctrl(struct dsi_display *display,
		u8 ulps_enable, u8 transfer_type, s64 node_id)
{
	struct dsi_ctrl_cfg ctrl_cfg = {0};
	struct dsi_ctrl *ctrl;
	int ret = 0;
	int i;

	if (!display) {
		DSI_ERROR("invalid display parameters\n");
		return -EINVAL;
	}

	display_for_each_active_port(i, display) {
		ctrl = display->port[i].ctrl;
		ctrl_cfg.cmdlist_flag = (transfer_type == USE_CMDLIST) ?
				CMDLIST_WRITE : DIRECT_WRITE;
		ctrl_cfg.cmdlist_node_id = node_id;
		if (ulps_enable)
			ret = dsi_ctrl_ulps_enter(ctrl, &ctrl_cfg);
		else
			ret = dsi_ctrl_ulps_exit(ctrl, &ctrl_cfg);
		if (ret)
			DSI_ERROR("dsi ctrl-%d ulps enter or exit failed\n", i);
	}

	DPU_DEBUG("dsi %s ulps by %s\n",
			ulps_enable ? "enter" : "exit",
			transfer_type == USE_CPU ? "cpu" : "cmdlist");

	return ret;
}

int dsi_display_debug(struct dsi_display *display,
	enum dsi_debug type, u8 parm_cnt, u8 *parm)
{
	struct dsi_ctrl *ctrl;
	int ret = 0;
	int i;

	if (!display) {
		DSI_ERROR("invalid display parameters\n");
		return -EINVAL;
	}

	display_for_each_active_port(i, display) {
		ctrl = display->port[i].ctrl;
		ret = dsi_ctrl_debug(ctrl, type, parm_cnt, parm);
		if (ret)
			DSI_ERROR("dsi ctrl-%d dsi debug error\n", i);
	}

	return ret;
}

u64 dsi_display_timestamp_get(struct dsi_display *display)
{
	u32 port;

	if (!display) {
		DSI_ERROR("get invalid paramerters\n");
		return 0;
	}
	if (display->display_info.is_dual_port)
		port = 0; /* use vsync timestamp of tmg0 for dual port */
	else
		port = display->display_info.dsi_id;

	return dsi_tmg_timestamp_get(display->port[port].tmg);
}

int dsi_display_damage_area_update(struct dsi_display *display)
{
	struct dsi_cmd_set *cmd_set;
	struct dsi_cmd_desc cmds[2] = {0};
	struct dsi_cmd_desc *cmd_x, *cmd_y;
	struct partial_parms par = {0};
	u8 tx_x[5] = {0};
	u8 tx_y[5] = {0};
	int ret = 0;

	par.xs = display->tmg_cfg.partial.xs;
	par.ys = display->tmg_cfg.partial.ys;
	par.width = display->tmg_cfg.partial.width;
	par.height = display->tmg_cfg.partial.height;

	tx_x[0] = MIPI_DCS_SET_COLUMN_ADDRESS;
	tx_x[1] = par.xs >> 8;
	tx_x[2] = par.xs & 0xff;
	tx_x[3] = (par.xs + par.width - 1) >> 8;
	tx_x[4] = (par.xs + par.width - 1) & 0xff;

	tx_y[0] = MIPI_DCS_SET_PAGE_ADDRESS;
	tx_y[1] = par.ys >> 8;
	tx_y[2] = par.ys & 0xff;
	tx_y[3] = (par.ys + par.height - 1) >> 8;
	tx_y[4] = (par.ys + par.height - 1) & 0xff;

	DSI_DEBUG("par: xs %d, ys %d, width %d, height %d\n",
		par.xs, par.ys, par.width, par.height);

	cmd_set = &display->panel->panel_info.current_mode->cmd_sets[DSI_CMD_SET_DAMAGE_AREA];
	cmd_set->cmd_state = DSI_CMD_SET_STATE_HS;
	cmd_set->type = DSI_CMD_SET_DAMAGE_AREA;
	cmd_set->num_infos = 0;
	cmd_set->num_cmds = 0;
	cmd_set->port_index = DSI_PORT_MAX;
	cmd_set->cmds = cmds;
	cmd_set->payload_size = 5;

	if (display->display_info.hdisplay != par.width) {
		cmd_x = &cmds[cmd_set->num_cmds++];
		cmd_x->msg.channel = 0;
		cmd_x->msg.type = 0x39;
		cmd_x->msg.flags &= ~MIPI_DSI_MSG_USE_LPM;
		cmd_x->msg.tx_len = 5;
		cmd_x->msg.tx_buf = (const void *)tx_x;
		cmd_x->msg.rx_len = 0;
		cmd_set->payload_size += 5;
	}

	cmd_y = &cmds[cmd_set->num_cmds++];
	cmd_y->msg.channel = 0;
	cmd_y->msg.type = 0x39;
	cmd_y->msg.flags &= ~MIPI_DSI_MSG_USE_LPM;
	cmd_y->msg.tx_len = 5;
	cmd_y->msg.tx_buf = (const void *)tx_y;
	cmd_y->msg.rx_len = 0;

	ret = dsi_panel_cmd_set_send(display->panel, cmd_set->type, cmd_set, USE_CMDLIST);
	if (ret)
		DSI_WARN("failed to send %s failed\n", dsi_panel_cmd_set_name_get(cmd_set->type));

	return 0;
}

static void dsi_display_mipi_freq_update(struct work_struct *dyn_freq_work)
{
	int i, ret;
	int state;
	struct dsi_mipi_dyn_freq *mipi_dyn_freq;
	struct dsi_display *display;
	struct dsi_ctrl_cfg ctrl_cfg = {0};
	struct dsi_ctrl *ctrl;
	struct dsi_tmg *tmg;
	struct dsi_display_info *dsi_info;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;
	struct dsi_connector *connector;
	struct dpu_hw_dvfs_mgr *dvfs_mgr;

	mipi_dyn_freq = container_of(dyn_freq_work,
			struct dsi_mipi_dyn_freq, dyn_freq_work);
	display = container_of(mipi_dyn_freq, struct dsi_display, mipi_dyn_freq);

	mutex_lock(&mipi_dyn_freq->dyn_freq_lock);
	state = atomic_read(&mipi_dyn_freq->dyn_freq_state);
	if (state != DSI_MIPI_DYN_FREQ_TRIG) {
		DSI_INFO("mipi dyn freq canceled, state %d\n", state);
		mutex_unlock(&mipi_dyn_freq->dyn_freq_lock);
		return;
	}
	atomic_set(&mipi_dyn_freq->dyn_freq_state, DSI_MIPI_DYN_FREQ_DOING);
	mutex_unlock(&mipi_dyn_freq->dyn_freq_lock);
	DSI_DEBUG("mipi dyn freq run begin\n");

	if (dpu_get_tui_state()) {
		DSI_INFO("mipi dyn freq not support in tui\n");
		goto end;
	}

	/* change mipi rate cfg */
	ret = dsi_panel_mipi_freq_update(display->panel,
			mipi_dyn_freq->request_freq_id);
	if (ret)
		goto end;

	/* disable ipi of dpu */
	dsi_info = &display->display_info;
	if (dsi_info->is_dual_port) {
		dpu_dsi_clk_ctrl(0, false);
		dpu_dsi_clk_ctrl(1, false);
	} else {
		dpu_dsi_clk_ctrl(dsi_info->dsi_id, false);
	}

	dsi_display_ctrl_cfg_get(display, &ctrl_cfg);
	display_for_each_active_port(i, display) {
		ctrl = display->port[i].ctrl;
		ret = dsi_ctrl_mipi_freq_update(ctrl, &ctrl_cfg);
		if (ret)
			DSI_ERROR("failed to update dsi ctrl-%d mipi freq\n", i);
	}

	dsi_display_tmg_cfg_update(display, USE_CPU);
	display_for_each_active_port(i, display) {
		tmg = display->port[i].tmg;
		ret = dsi_tmg_dyn_freq_update(tmg, &display->tmg_cfg);
		if (ret)
			DSI_ERROR("failed to update tmg-%d mipi freq\n", i);
	}

	/* enable ipi of dpu */
	if (dsi_info->is_dual_port) {
		dpu_dsi_clk_ctrl(0, true);
		dpu_dsi_clk_ctrl(1, true);
	} else {
		dpu_dsi_clk_ctrl(dsi_info->dsi_id, true);
	}

end:
	connector = to_dsi_connector(display->connector);
	dpu_drm_dev = container_of(connector->drm_dev, struct dpu_drm_device, drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	dvfs_mgr = &dpu_kms->core_perf->hw_dvfs_mgr;
	dpu_idle_enable_ctrl(true);
	dpu_hw_dvfs_enable(dvfs_mgr);
	dpu_core_perf_clk_deactive();

	atomic_set(&mipi_dyn_freq->dyn_freq_state, DSI_MIPI_DYN_FREQ_WAIT);
	DSI_INFO("mipi dyn freq run successfully\n");
	wake_up(&mipi_dyn_freq->dyn_freq_waitqueue);
}

static int dsi_display_dyn_freq_init(
		struct dsi_mipi_dyn_freq *mipi_dyn_freq)
{
	int ret = 0;

	memset(mipi_dyn_freq, 0, sizeof(*mipi_dyn_freq));
	mipi_dyn_freq->enable = true;
	mutex_init(&mipi_dyn_freq->dyn_freq_lock);
	mutex_init(&mipi_dyn_freq->commit_lock);
	atomic_set(&mipi_dyn_freq->force_refresh_flag, 0);
	atomic_set(&mipi_dyn_freq->dyn_freq_state, DSI_MIPI_DYN_FREQ_POWERON);
	atomic_set(&mipi_dyn_freq->commit_state, DPU_VIRT_PIPELINE_DONE);
	init_waitqueue_head(&mipi_dyn_freq->dyn_freq_waitqueue);

	mipi_dyn_freq->dyn_freq_workqueue =
			create_singlethread_workqueue("dsi_mipi_dyn_freq");
	if (IS_ERR_OR_NULL(mipi_dyn_freq->dyn_freq_workqueue)) {
		DSI_ERROR("failed to create dyn freq workqueue\n");
		ret = -EPERM;
		goto end;
	}

	INIT_WORK(&mipi_dyn_freq->dyn_freq_work, dsi_display_mipi_freq_update);

end:
	return ret;
}

static void dsi_display_dyn_freq_deinit(
		struct dsi_mipi_dyn_freq *mipi_dyn_freq)
{
	cancel_work_sync(&mipi_dyn_freq->dyn_freq_work);
	flush_workqueue(mipi_dyn_freq->dyn_freq_workqueue);
	destroy_workqueue(mipi_dyn_freq->dyn_freq_workqueue);
	mipi_dyn_freq->dyn_freq_workqueue = NULL;
}

inline void dsi_display_dyn_freq_work_trigger(struct dsi_display *display)
{
	struct dsi_mipi_dyn_freq *mipi_dyn_freq = &display->mipi_dyn_freq;
	int state;

	state = atomic_read(&mipi_dyn_freq->dyn_freq_state);
	if (state == DSI_MIPI_DYN_FREQ_POWERON) {
		atomic_set(&mipi_dyn_freq->dyn_freq_state, DSI_MIPI_DYN_FREQ_WAIT);
	} else if (state == DSI_MIPI_DYN_FREQ_ACCEPTED) {
		wake_up(&mipi_dyn_freq->dyn_freq_waitqueue);
	} else if (state == DSI_MIPI_DYN_FREQ_PREPARE) {
		atomic_set(&mipi_dyn_freq->dyn_freq_state, DSI_MIPI_DYN_FREQ_TRIG);
		queue_work(mipi_dyn_freq->dyn_freq_workqueue,
				&mipi_dyn_freq->dyn_freq_work);
	}
}

void dsi_display_dyn_freq_wait(struct drm_crtc_state *crtc_state,
		unsigned int msecs)
{
	struct drm_connector *connector;
	struct dsi_connector *dsi_connector;
	struct dsi_mipi_dyn_freq *mipi_dyn_freq;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_hw_dvfs_mgr *dvfs_mgr;
	u32 mask;
	int ret;
	int state;

	for_each_connector_per_crtc(connector, crtc_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_connector = to_dsi_connector(connector);
			mipi_dyn_freq = &dsi_connector->display->mipi_dyn_freq;
			dpu_drm_dev = container_of(dsi_connector->drm_dev,
					struct dpu_drm_device, drm_dev);
			dvfs_mgr = &dpu_drm_dev->dpu_kms->core_perf->hw_dvfs_mgr;

			mutex_lock(&mipi_dyn_freq->commit_lock);
			ret = wait_event_interruptible_timeout(
					mipi_dyn_freq->dyn_freq_waitqueue,
					atomic_read(&mipi_dyn_freq->dyn_freq_state) <= DSI_MIPI_DYN_FREQ_WAIT ||
					atomic_read(&mipi_dyn_freq->force_refresh_flag),
					msecs_to_jiffies(msecs));
			if (ret <= 0) {
				mutex_lock(&mipi_dyn_freq->dyn_freq_lock);
				state = atomic_read(&mipi_dyn_freq->dyn_freq_state);
				if (state == DSI_MIPI_DYN_FREQ_PREPARE ||
						state == DSI_MIPI_DYN_FREQ_TRIG) {
					atomic_set(&mipi_dyn_freq->dyn_freq_state, DSI_MIPI_DYN_FREQ_WAIT);
					dpu_idle_enable_ctrl(true);
					dpu_hw_dvfs_enable(dvfs_mgr);
					dpu_core_perf_clk_deactive();
				}
				mutex_unlock(&mipi_dyn_freq->dyn_freq_lock);
				DSI_INFO("wait dyn freq done exceeds %dms, state %d\n",
						msecs, state);
			}
			atomic_set(&mipi_dyn_freq->commit_state, DPU_VIRT_PIPELINE_RUN);
			mutex_unlock(&mipi_dyn_freq->commit_lock);
		}
	}
}

void dsi_display_dyn_freq_task_wakeup(struct drm_crtc_state *crtc_state)
{
	struct drm_connector *connector;
	struct dsi_connector *dsi_connector;
	struct dsi_mipi_dyn_freq *mipi_dyn_freq;
	u32 mask;

	for_each_connector_per_crtc(connector, crtc_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_connector = to_dsi_connector(connector);
			mipi_dyn_freq = &dsi_connector->display->mipi_dyn_freq;

			atomic_set(&mipi_dyn_freq->commit_state, DPU_VIRT_PIPELINE_DONE);
			if (atomic_read(&mipi_dyn_freq->force_refresh_flag))
				atomic_set(&mipi_dyn_freq->force_refresh_flag, 0);

			wake_up(&mipi_dyn_freq->dyn_freq_waitqueue);
		}
	}
}

int dsi_display_dyn_freq_request(struct dsi_display *display, u8 freq_id)
{
	struct dsi_mipi_dyn_freq *mipi_dyn_freq = &display->mipi_dyn_freq;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_virt_pipeline *virt_pipeline;
	struct dpu_kms *dpu_kms;
	struct dsi_connector *connector;
	struct dpu_hw_dvfs_mgr *dvfs_mgr;
	struct drm_crtc *crtc;
	int ret = 0;
	bool is_cmd_mode;
	int pipe;
	int state;

	if (!mipi_dyn_freq->enable) {
		DSI_INFO("mipi dyn freq not enable\n");
		return 0;
	}

	if (dpu_get_tui_state()) {
		DSI_INFO("mipi dyn freq not support in tui\n");
		return 0;
	}

	is_cmd_mode = dsi_display_is_cmd_mode(display);
	if (!is_cmd_mode) {
		DSI_INFO("mipi dyn freq only support in command mode\n");
		return 0;
	}

	mutex_lock(&mipi_dyn_freq->commit_lock);
	mutex_lock(&mipi_dyn_freq->dyn_freq_lock);
	state = atomic_read(&mipi_dyn_freq->dyn_freq_state);
	if (state != DSI_MIPI_DYN_FREQ_WAIT) {
		DSI_INFO("mipi dyn freq not ready, state %d\n", state);
		mutex_unlock(&mipi_dyn_freq->dyn_freq_lock);
		mutex_unlock(&mipi_dyn_freq->commit_lock);
		return -EBUSY;
	}
	atomic_set(&mipi_dyn_freq->dyn_freq_state, DSI_MIPI_DYN_FREQ_ACCEPTED);
	mutex_unlock(&mipi_dyn_freq->dyn_freq_lock);

	DSI_INFO("mipi dyn freq request, freq_id %d\n", freq_id);
	mipi_dyn_freq->request_freq_id = freq_id;

	connector = to_dsi_connector(display->connector);
	dpu_drm_dev = container_of(connector->drm_dev, struct dpu_drm_device, drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	dvfs_mgr = &dpu_kms->core_perf->hw_dvfs_mgr;
	dpu_core_perf_clk_active();
	dpu_hw_dvfs_disable(dvfs_mgr);
	dpu_idle_enable_ctrl(false);

	crtc = connector->encoder->crtc;
	if (!crtc) {
		DSI_INFO("crtc disconnected, dsi is powered off\n");
		goto power_off;
	}

	if (need_restart_dpu_display_after_idle(crtc)) {
		dpu_force_refresh_event_notify(crtc->dev);
		DSI_INFO("dyn freq notify force refresh event\n");
		mutex_unlock(&mipi_dyn_freq->commit_lock);

		ret = wait_event_interruptible_timeout(mipi_dyn_freq->dyn_freq_waitqueue,
				atomic_read(&mipi_dyn_freq->force_refresh_flag) == 0,
				msecs_to_jiffies(2000));
		if (ret <= 0) {
			DSI_DEBUG("failed to wait force refresh flag\n");
			ret = -EBUSY;
			mutex_lock(&mipi_dyn_freq->commit_lock);
			goto power_off;
		}
		mutex_lock(&mipi_dyn_freq->commit_lock);
	}

	pipe = drm_crtc_index(crtc);
	virt_pipeline = &dpu_kms->virt_pipeline[pipe];
	ret = wait_event_interruptible_timeout(mipi_dyn_freq->dyn_freq_waitqueue,
			(atomic_read(&virt_pipeline->vsync_flag) == 1 ||
			!is_dsi_powered_on()) &&
			atomic_read(&mipi_dyn_freq->commit_state) == DPU_VIRT_PIPELINE_DONE,
			msecs_to_jiffies(1000));
	if (ret <= 0) {
		DSI_DEBUG("failed to wait sw commit done\n");
		ret = -EBUSY;
		goto power_off;
	}

	if (!is_dsi_powered_on()) {
		DSI_INFO("dsi is powered off\n");
		goto power_off;
	}

	atomic_set(&mipi_dyn_freq->dyn_freq_state, DSI_MIPI_DYN_FREQ_PREPARE);
	DSI_DEBUG("mipi dyn freq request done\n");
	mutex_unlock(&mipi_dyn_freq->commit_lock);
	return 0;

power_off:
	dpu_idle_enable_ctrl(true);
	dpu_hw_dvfs_enable(dvfs_mgr);
	dpu_core_perf_clk_deactive();
	atomic_set(&mipi_dyn_freq->dyn_freq_state, DSI_MIPI_DYN_FREQ_WAIT);
	mutex_unlock(&mipi_dyn_freq->commit_lock);
	return ret;
}

int dsi_display_tmg_init(struct dsi_display *display)
{
	struct dsi_tmg *tmg;
	int ret;
	int i;

	dsi_display_tmg_cfg_update(display, USE_CPU);

	display_for_each_active_port(i, display) {
		tmg = display->port[i].tmg;
		/* config obufen */
		dsi_hw_tmg_obufen_config(&tmg->tmg_hw.hw_blk, dpu_obuf_get_obufen(i));

		/* config obuffer level threshold */
		dsi_display_line_buf_level_cfg(display, i);

		ret = dsi_tmg_hw_init(tmg, &display->tmg_cfg);
		if (ret)
			DSI_ERROR("failed to init %d-th tmg\n", i);
	}

	return 0;
}

int dsi_display_enable(struct dsi_display *display)
{
	struct dsi_connector_state *dsi_connector_state;
	int power_mode;
	int ret = 0;

	if (!display) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	dsi_connector_state = to_dsi_connector_state(display->connector->state);
	power_mode = dsi_connector_state->pm_changed ?
			dsi_connector_state->power_mode : DPU_POWER_MODE_ON;

	ret = dsi_display_set_power_mode_prepare(display, power_mode);
	if (ret)
		DSI_ERROR("failed to set power mode: ON\n");

	return ret;
}

static int dsi_display_ctrl_enable(struct dsi_display *display)
{
	struct dsi_ctrl_cfg ctrl_cfg = {0};
	struct dsi_ctrl *ctrl;
	int ret = 0;
	int i;

	if (!display) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	dsi_display_ctrl_cfg_get(display, &ctrl_cfg);
	display_for_each_active_port(i, display) {
		ctrl = display->port[i].ctrl;

		ret = dsi_ctrl_hw_init(ctrl, &ctrl_cfg);
		if (ret) {
			DSI_ERROR("dsi ctrl-%d hw init failled\n", i);
			return ret;
		}

		ret = dsi_ctrl_enable(ctrl, &ctrl_cfg);
		if (ret) {
			DSI_ERROR("dsi ctrl-%d enable failled\n", i);
			return ret;
		}
	}

	return 0;
}

int dsi_display_tmg_deinit(struct dsi_display *display)
{
	bool obufen_updated;
	struct dsi_tmg *tmg;
	int ret = 0;
	int i;

	display_for_each_active_port(i, display) {
		(void)dpu_obuf_check_obufen(i, &obufen_updated);
		dpu_obuf_update_obufen_cmt(i, obufen_updated);
		tmg = display->port[i].tmg;
		ret = dsi_tmg_disable(tmg);
		if (ret) {
			DSI_ERROR("failed to disable %d-th tmg\n", i);
			return ret;
		}
	}
	return 0;
}

int dsi_display_ctrl_mode_init(struct dsi_display *display)
{
	struct dsi_ctrl_cfg ctrl_cfg = {0};
	struct dsi_ctrl *ctrl;
	int ret = 0;
	int i;

	dsi_display_ctrl_cfg_get(display, &ctrl_cfg);

	display_for_each_active_port(i, display) {
		ctrl = display->port[i].ctrl;
		ret = dsi_ctrl_enable(ctrl, &ctrl_cfg);
		if (ret) {
			DSI_ERROR("dsi ctrl-%d enable failed\n", i);
			return ret;
		}
	}

	return 0;
}

static int __dsi_display_disable(struct dsi_display *display)
{
	bool is_cmd_mode;
	int ret = 0;

	is_cmd_mode = dsi_display_is_cmd_mode(display);
	if (!is_cmd_mode) {
		dsi_display_esd_ctrl_locked(display, false);
		ret = dsi_panel_disable(display->panel);
		if (ret) {
			DSI_ERROR("failed to disable video panel\n");
			goto error;
		}
	}

	dsi_display_tmg_deinit(display);

	atomic_set(&display->mipi_dyn_freq.dyn_freq_state, DSI_MIPI_DYN_FREQ_POWEROFF);

	DSI_DEBUG("dsi_display_disable, is_cmd_mode: %d\n", is_cmd_mode);
error:
	return ret;
}

int dsi_display_disable(struct dsi_display *display)
{
	struct dsi_connector_state *dsi_connector_state;
	int power_mode;
	int ret = 0;

	if (!display) {
		DSI_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_connector_state = to_dsi_connector_state(display->connector->state);
	power_mode = dsi_connector_state->pm_changed ?
			dsi_connector_state->power_mode : DPU_POWER_MODE_OFF;

	ret = dsi_display_set_power_mode_finish(display, power_mode);
	if (ret)
		DSI_ERROR("failed to set power mode: OFF\n");

	return ret;
}

static int dsi_display_disable_tmg(struct dsi_display *display)
{
	int ret = 0;
	int i;

	if (!display) {
		DSI_ERROR("invalid params\n");
		return -EINVAL;
	}

	display_for_each_active_port(i, display) {
		ret = dsi_tmg_disable(display->port[i].tmg);
		if (ret)
			DSI_ERROR("failed to disable %d-th tmg\n", i);
	}

	return ret;
}

int dsi_display_set_power_mode_prepare(struct dsi_display *display, int new_power_mode)
{
	struct dsi_connector_state *dsi_connector_state;
	struct dsi_connector *dsi_connector;
	int current_power_mode;
	int ret = 0;

	if (unlikely(!display)) {
		DSI_ERROR("get invalid paramerters\n");
		return -EINVAL;
	}

	dsi_connector = to_dsi_connector(display->connector);
	dsi_connector_state = to_dsi_connector_state(display->connector->state);
	current_power_mode = dsi_connector->current_power_mode;

	if (current_power_mode == new_power_mode) {
		DSI_WARN("dsi power already in: %s, skip\n",
				get_dpu_power_mode_name(current_power_mode));
		return 0;
	}

	dsi_display_lock(display);
	/* off -> on/doze */
	if (current_power_mode == DPU_POWER_MODE_OFF)
		ret = __dsi_display_enable(display, true);

	/* suspend -> on/doze, donot need to exit ulps */
	if (current_power_mode == DPU_POWER_MODE_DOZE_SUSPEND)
		ret = __dsi_display_enable(display, false);

	if (ret)
		goto exit;

	switch (new_power_mode) {
	case DPU_POWER_MODE_ON:
		if (current_power_mode == DPU_POWER_MODE_DOZE_SUSPEND ||
				current_power_mode == DPU_POWER_MODE_DOZE) {
			ret = dsi_panel_exit_lp(display->panel);
			if (ret)
				goto exit;
		} else {
			dsi_panel_event_notifier_trigger(DRM_PANEL_EVENT_UNBLANK, display->panel);
		}
		break;
	case DPU_POWER_MODE_DOZE:
		ret = dsi_panel_doze(display->panel);
		if (ret)
			goto exit;
		break;
	default:
		break;
	}

exit:
	dsi_connector->current_power_mode = new_power_mode;
	dsi_connector_state->pm_changed = false;
	dsi_display_unlock(display);
	DSI_INFO("display (%u), power mode transition from (%s) to (%s): %s\n",
			display->display_type, get_dpu_power_mode_name(current_power_mode),
			get_dpu_power_mode_name(new_power_mode),
			ret ? "failed" : "successful");

	return ret;
}

int dsi_display_set_power_mode_finish(struct dsi_display *display, int new_power_mode)
{
	struct dsi_connector_state *dsi_connector_state;
	struct dsi_connector *dsi_connector;
	int current_power_mode;
	int ret = 0;

	if (unlikely(!display)) {
		DSI_ERROR("get invalid paramerters\n");
		return -EINVAL;
	}

	dsi_connector = to_dsi_connector(display->connector);
	dsi_connector_state = to_dsi_connector_state(display->connector->state);
	current_power_mode = dsi_connector->current_power_mode;

	if (current_power_mode == new_power_mode) {
		DSI_ERROR("dsi power already in: %s, skip\n",
				get_dpu_power_mode_name(current_power_mode));
		return 0;
	}

	dsi_display_lock(display);
	switch (new_power_mode) {
	case DPU_POWER_MODE_DOZE_SUSPEND:
		if (current_power_mode == DPU_POWER_MODE_DOZE) {
			ret = dsi_panel_doze_suspend(display->panel);
			if (ret)
				goto exit;
			// require dpu power
			ret = dsi_display_disable_tmg(display);
			if (ret)
				goto exit;
		}

		break;
	case DPU_POWER_MODE_OFF:
		if (current_power_mode == DPU_POWER_MODE_DOZE_SUSPEND) {
			/* for send dsi panel disable cmd */
			if (!dsi_display_initialized_get(display))
				ret = dsi_display_ctrl_enable(display);
		} else {
			ret = __dsi_display_disable(display);
		}
		if (ret)
			goto exit;
		break;
	default:
		break;
	}

exit:
	dsi_connector->current_power_mode = new_power_mode;
	dsi_connector_state->pm_changed = false;
	dsi_display_unlock(display);
	DSI_INFO("display (%u), power from (%s) to (%s): %s\n",
			display->display_type,
			get_dpu_power_mode_name(current_power_mode),
			get_dpu_power_mode_name(new_power_mode),
			ret ? "failed" : "successful");

	return ret;
}

int dsi_display_mode_validate(struct dsi_display *display,
		struct drm_display_mode *mode)
{
	int ret = 0;

	ret = dsi_panel_mode_validate(display->panel, mode);
	if (ret) {
		DSI_ERROR("panel mode validation failed\n");
		return ret;
	}

	return ret;
}

int dsi_display_modes_get(struct drm_connector *connector,
		struct dsi_display *display)
{
	struct drm_display_mode *drm_mode, temp;
	struct dsi_panel_info *panel_info;
	int i, count = 0;
	u32 num_modes;

	if (!connector) {
		DSI_ERROR("invalid parameters\n");
		return 0;
	}

	panel_info = &display->panel->panel_info;
	num_modes = panel_info->num_modes;
	DSI_DEBUG("panel_info->vrr_enable:%d\n", panel_info->vrr_enable);
	for (i = 0; i < num_modes; i++) {
		dsi_panel_get_drm_mode(display->panel, i, &temp);

		/* if vrr is disabled, only probe the preferred mode */
		if (!panel_info->vrr_enable && !(temp.type & DRM_MODE_TYPE_PREFERRED))
			continue;

		drm_mode = drm_mode_duplicate(connector->dev, &temp);
		if (!drm_mode) {
			DSI_ERROR("failed to duplicate drm mode\n");
			return 0;
		}

		drm_mode_probed_add(connector, drm_mode);
		DSI_DEBUG("probe mode %s, skew %d, flag %d, type %d\n",
				drm_mode->name, drm_mode->hskew,
				drm_mode->flags, drm_mode->type);
		++count;
	}

	DSI_DEBUG("get mode count:%d\n", count);

	return count;
}

void dsi_display_modes_put(struct dsi_display *display)
{
	if (!display)
		return;

	dsi_panel_modes_destroy(display->panel);
}

int __maybe_unused dsi_display_front_porch_calculate(u32 src_fps, u32 dest_fps,
		u32 src_total, u32 src_fp, u32 *tgt_fp)
{
	s32 fp_gap, fps_gap, result_fp;

	if (!src_fps || !dest_fps || !tgt_fp || !src_total) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	/* Calculate front porch by source/target fps */
	if (src_fps < dest_fps) {
		DSI_ERROR("not supported src fps: %d, dest fps: %d\n",
				src_fps, dest_fps);
		return -EINVAL;
	}

	fps_gap = src_fps - dest_fps;
	fp_gap = mult_frac(src_total, fps_gap, dest_fps);
	fp_gap *= (src_fps > dest_fps ? 1 : -1);
	result_fp = src_fp + fp_gap;

	if (result_fp <= 0) {
		DSI_ERROR("failed to calculate new fp\n");
		return -EINVAL;
	}

	*tgt_fp = result_fp;
	DSI_DEBUG("src fps: %d, tgt fps: %d, src total: %d, src fp: %d, tgt fp: %d\n",
			src_fps, dest_fps, src_total, src_fp, result_fp);
	return 0;
}

int __maybe_unused dsi_display_skipped_te_cnt_calculate(u32 src_fps,
		u32 dest_fps, u32 *skip_te)
{
	if (!dest_fps || !skip_te) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	if (src_fps < dest_fps) {
		DSI_ERROR("invalid target fps\n");
		return -EINVAL;
	}

	*skip_te = (u32)mult_frac(1, src_fps, dest_fps);
	DSI_DEBUG("get skip_te:%d", *skip_te);

	return 0;
}

int dsi_display_mode_set(struct dsi_display *display,
		struct drm_display_mode *mode,
		struct drm_display_mode *adjusted_mode)
{
	struct dsi_panel_mode *panel_mode;
	struct dsi_panel *panel;
	int ret = 0;

	if (unlikely(!display || !mode || !adjusted_mode)) {
		DSI_ERROR("get invalid parameters, %pK, %pK, %pK\n",
				display, mode, adjusted_mode);
		return -EINVAL;
	}
	panel = display->panel;

	/**
	 * in order to find dsi_panel_mode by drm_display_mode,
	 * use mode insted of adjusted mode
	 */
	ret = dsi_panel_mode_search(panel, mode, &panel_mode);
	if (ret) {
		DSI_ERROR("failed to search panel mode by drm mode\n");
		goto error;
	}

	dsi_panel_mode_update(panel, panel_mode);
error:
	return ret;
}

static inline int dsi_display_phandle_num_get(struct device_node *dev_node,
		const char *name)
{
	int count = 0;

	count = of_property_count_u32_elems(dev_node, name);
	if (count < 0 || count > DSI_DISPLAY_MAX) {
		DSI_ERROR("failed to get %s count: %d\n", name, count);
		return -EINVAL;
	}

	return count;
}

static int dsi_display_port_create(struct dsi_display *display,
		struct device_node *dev_node)
{
	const char *ctrl_list_name = "dsi-ctrls";
	const char *phy_list_name = "dsi-phys";
	const char *tmg_list_name = "dsi-tmgs";
	u32 ctrl_count, phy_count, tmg_count;
	struct device_node *temp_node;
	int phy_id, ctrl_id, tmg_id, i;
	int ret = 0;

	ctrl_count = dsi_display_phandle_num_get(dev_node, ctrl_list_name);
	phy_count = dsi_display_phandle_num_get(dev_node, phy_list_name);
	tmg_count = dsi_display_phandle_num_get(dev_node, tmg_list_name);
	if (phy_count != ctrl_count || phy_count != tmg_count) {
		DSI_ERROR("get ctrl count=%d, phy count=%d, tmg count=%d\n",
				ctrl_count, phy_count, tmg_count);
		return -EINVAL;
	}

	DSI_DEBUG("get ctrl count=%d, phy count=%d, tmg count=%d\n",
			ctrl_count, phy_count, tmg_count);

	display->port_count = ctrl_count;
	display_for_each_port(i, display) {
		/* bind ctrl to dsi display port in here */
		temp_node = of_parse_phandle(dev_node, ctrl_list_name, i);
		display->port[i].ctrl = of_find_dsi_ctrl(temp_node);
		of_node_put(temp_node);
		if (IS_ERR_OR_NULL(display->port[i].ctrl)) {
			DSI_ERROR("failed to get dsi ctrl %d\n", i);
			ret = -EFAULT;
			goto error_release_tmg;
		}

		/* bind phy to dsi display port in here */
		temp_node = of_parse_phandle(dev_node, phy_list_name, i);
		display->port[i].phy = of_find_dsi_phy(temp_node);
		of_node_put(temp_node);
		if (IS_ERR_OR_NULL(display->port[i].phy)) {
			DSI_ERROR("failed to get dsi phy %d\n", i);
			ret = -EFAULT;
			goto error_release_tmg;
		}

		temp_node = of_parse_phandle(dev_node, tmg_list_name, i);
		display->port[i].tmg = dsi_tmg_create(display->pdev, temp_node);
		of_node_put(temp_node);
		if (IS_ERR_OR_NULL(display->port[i].tmg)) {
			DSI_ERROR("failed to get dsi tmg %d\n", i);
			ret = -EFAULT;
			goto error_release_tmg;
		}

		ctrl_id = dsi_ctrl_hw_blk_id_get(display->port[i].ctrl);
		phy_id = dsi_phy_id_get(display->port[i].phy);
		tmg_id = dsi_tmg_id_get(display->port[i].tmg);
		if (ctrl_id < 0 || phy_id < 0 || tmg_id < 0 ||
				ctrl_id != phy_id || ctrl_id != tmg_id) {
			DSI_ERROR("bad ctrl/phy/tmg hardware id, %d, %d, %d\n",
					ctrl_id, phy_id, tmg_id);
			ret = -EFAULT;
			goto error_release_tmg;
		}

		display->port[i].ctrl->ctrl_hw.ctrl_blks.phy_blk =
				&display->port[i].phy->phy_hw.hw_blk;

		display->port[i].dsi_id = ctrl_id;
		display->port[i].enabled = false;
		DSI_DEBUG("create dsi port %d with dsi id %d\n", i, ctrl_id);
	}
	return ret;

error_release_tmg:
	for (; i >= 0; i--)
		dsi_tmg_destroy(display->port[i].tmg);

	return ret;
}

static inline void dsi_display_port_destroy(struct dsi_display *display)
{
	int i;

	for (i = 0; i < display->port_count; i++) {
		if (display->port[i].tmg)
			dsi_tmg_destroy(display->port[i].tmg);
	}
}

static struct device_node *dsi_display_panel_node_get(
		struct device_node *dev_node)
{
	struct device_node *panel_np;
	const char *sel_lcd_name;
	int ret;

	ret = of_property_read_string(dev_node, "sel_lcd_name", &sel_lcd_name);
	if (ret) {
		DSI_ERROR("failed to parse sel_lcd_name\n");
		return ERR_PTR(-ret);
	}

	DSI_DEBUG("get sel_lcd_name:%s\n", sel_lcd_name);
	panel_np = of_find_node_by_name(dev_node, sel_lcd_name);
	return panel_np;
}

static int dsi_display_panel_create(struct dsi_display *display,
		struct device_node *dev_node)
{
	struct device_node *panel_node;
	u8 port_mask;
	int i;

	panel_node = dsi_display_panel_node_get(dev_node);
	if (IS_ERR_OR_NULL(panel_node)) {
		DSI_ERROR("invalid panel node: %pK\n", panel_node);
		return -EFAULT;
	}

	display->panel = dsi_panel_create(panel_node, display->is_asic,
			dsi_display_type_name_get(display->display_type));
	of_node_put(panel_node);
	if (IS_ERR_OR_NULL(display->panel)) {
		DSI_ERROR("create panel failed");
		return -EFAULT;
	}
	display->panel->display = display;

	/* update dsi port status */
	if (display->display_type == DSI_DISPLAY_PRIMARY)
		display->panel->panel_info.is_primary = true;
	else
		display->panel->panel_info.is_primary = false;

	port_mask = display->panel->panel_info.host_info.port_mask;
	for (i = 0; i < DSI_PORT_MAX; ++i) {
		if (port_mask & BIT(i)) {
			if (display->port_count  < i + 1) {
				DSI_ERROR("bad port count %d, mask:0x%x\n",
						display->port_count, port_mask);
				return -EINVAL;
			}
			display->port[i].enabled = true;
			DSI_DEBUG("dsi-%d is enabled\n", i);
		}
	}

	return 0;
}

static int dsi_display_resource_init(struct dsi_display *display)
{
	struct device_node *dev_node;
	int ret = 0;

	dev_node = display->pdev->dev.of_node;
	ret = dsi_display_port_create(display, dev_node);
	if (ret) {
		DSI_ERROR("failed to init display port\n");
		return ret;
	}

	ret = dsi_display_dyn_freq_init(&display->mipi_dyn_freq);
	if (ret) {
		DSI_ERROR("failed to init mipi dyn freq\n");
		return ret;
	}

	return 0;
}

/**
 * dsi_display_resource_deinit - deinitializes the display object resource
 * All the resources acquired during resource init will be released.
 * @display: Handle to the display
 * Returns: Zero on success
 */
static void dsi_display_resource_deinit(struct dsi_display *display)
{
	dsi_display_dyn_freq_deinit(&display->mipi_dyn_freq);
	dsi_display_port_destroy(display);
}

static void dsi_display_type_parse(struct dsi_display *display,
		struct platform_device *pdev)
{
	const char *display_type;

	/* parse display type primary/secondary for dual mipi dsi */
	display_type = of_get_property(pdev->dev.of_node, "type", NULL);
	if (!display_type) {
		display->display_type = DSI_DISPLAY_PRIMARY;
		DSI_DEBUG("get default display type: primary");
	} else {
		if (!strcmp(display_type, "primary")) {
			display->display_type = DSI_DISPLAY_PRIMARY;
		} else if (!strcmp(display_type, "secondary")) {
			display->display_type = DSI_DISPLAY_SECONDARY;
		} else {
			DSI_ERROR("get unknown display type\n");
			display->display_type = DSI_DISPLAY_MAX;
		}
	}
	DSI_DEBUG("get dispay type:%s\n", display_type);
}

void dsi_display_panel_dead_notify(struct dsi_display *display)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dsi_connector *dsi_conn;
	struct drm_device *drm_dev;
	struct dsi_panel *panel;
	struct drm_event event;

	if (!display) {
		DSI_ERROR("invalid pramter\n");
		return;
	}

	panel = display->panel;
	if (panel->panel_dead) {
		DSI_INFO("%s display already in panel_dead status\n",
				dsi_display_type_name_get(display->display_type));
		return;
	}

	display->panel->panel_dead = true;
	drm_dev = dev_get_drvdata(display->bl_device->dev.parent);
	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	/* TODO: add DRM_PRIVATE_EVENT_PRIMARY/SECDONARY_PANEL_DEAD */
	event.type = DRM_PRIVATE_EVENT_PANEL_DEAD;
	event.length = sizeof(bool);

	dsi_conn = to_dsi_connector(display->connector);

	dpu_drm_event_notify(drm_dev, &event, (u8 *)&display->panel->panel_dead);
	DSI_INFO("Panel status error, notify for %s display\n",
			dsi_display_type_name_get(display->display_type));
}

static void dsi_display_esd_check_work(struct work_struct *work)
{
	struct dsi_display *display;
	u32 period;
	int ret;

	display = container_of(to_delayed_work(work),
			struct dsi_display, esd_check_work);
	if (unlikely(!display)) {
		DSI_ERROR("failed to get %s display\n",
				dsi_display_type_name_get(display->display_type));
		return;
	}

	ret = dsi_display_status_check(display);
	if (!ret) {
		DSI_DEBUG("status reg check success, just returned\n");
		goto exit_with_release_lock;
	}

	if (ret) {
		dsi_display_panel_dead_notify(display);
		return;
	}

exit_with_release_lock:
	period = display->panel->panel_info.esd_info.period;
	DSI_DEBUG("%s display esd check success period: %d\n",
			dsi_display_type_name_get(display->display_type), period);
	schedule_delayed_work(&display->esd_check_work,
			msecs_to_jiffies(period));
}

static irqreturn_t dsi_display_esd_irq_handle(int irq, void *data)
{
	struct dsi_display *display = (struct dsi_display *)data;
	struct dsi_panel *panel;

	trace_dsi_display_esd_irq_handle("esd irq", irq);

	DSI_ERROR("esd irq %d is triggered\n", irq);
	if (unlikely(!display)) {
		DSI_ERROR("invalid parameter, %pK\n", display);
		return IRQ_HANDLED;
	}

	panel = display->panel;
	dsi_display_lock(display);
	if (unlikely(!display->esd_check_enabled)) {
		DSI_INFO("%s-display esd check is disabled\n",
				dsi_display_type_name_get(display->display_type));
		dsi_display_unlock(display);
		return IRQ_HANDLED;
	}
	dsi_display_esd_ctrl_locked(display, false);
	dsi_panel_lock(panel);
	display->esd_check_enabled = false;
	if (!dsi_panel_initialized(panel)) {
		DSI_WARN("panel is uninitialized, skip notify\n");
		goto exit_with_release_lock;
	}

	if (atomic_read(&display->panel->panel_recovery_pending)) {
		DSI_INFO("already in esd recovery process, returned\n");
		goto exit_with_release_lock;
	}

	if (!panel->panel_dead) {
		atomic_set(&display->panel->panel_recovery_pending, 1);
		DSI_DEBUG("panel first dead\n");
	} else {
		DSI_INFO("display%d already dead, skip notify\n",
				display->display_type);
		goto exit_with_release_lock;
	}

	DSI_DEBUG("panel get esd attack\n");
	dsi_display_panel_dead_notify(display);
exit_with_release_lock:
	dsi_panel_unlock(panel);
	dsi_display_unlock(display);
	return IRQ_HANDLED;
}

static void dsi_display_esd_irq_init(struct dsi_display *display)
{
	struct dsi_panel_esd_info *esd_info;
	int i, ret = 0;

	esd_info = &display->panel->panel_info.esd_info;
	for (i = 0; i < esd_info->err_irq_gpio_num; ++i) {
		if (!gpio_is_valid(esd_info->err_irq_gpio[i])) {
			DSI_ERROR("%s = %d is invalid\n",
					esd_info->err_irq_gpio_name[i],
					esd_info->err_irq_gpio[i]);
			continue;
		}

		ret = request_threaded_irq(esd_info->err_irq[i],
				NULL, dsi_display_esd_irq_handle,
				esd_info->err_irq_gpio_flag[i],
				esd_info->err_irq_gpio_name[i],
				display);
		if (ret) {
			DSI_ERROR("failed regist %s = %d, ret: %d\n",
					esd_info->err_irq_gpio_name[i],
					esd_info->err_irq_gpio[i], ret);
		} else {
			DSI_DEBUG("success regist %s = %d\n",
					esd_info->err_irq_gpio_name[i],
					esd_info->err_irq_gpio[i]);
			disable_irq(esd_info->err_irq[i]);
		}
	}
}

int dsi_display_esd_init(struct dsi_display *display)
{
	struct dsi_panel_esd_info *esd_info;
	struct dsi_panel_info *panel_info;

	panel_info = &display->panel->panel_info;
	esd_info = &panel_info->esd_info;
	if (!esd_info->enable) {
		DSI_DEBUG("esd is disabled return\n");
		return 0;
	}

	/* disable esd check untile the hwc is ready */
	display->esd_check_enabled = false;

	switch (esd_info->esd_mode) {
	case DSI_PANEL_ESD_MODE_SW_SUCCESS:
	case DSI_PANEL_ESD_MODE_SW_FAILURE:
	case DSI_PANEL_ESD_MODE_PANEL_TE:
	case DSI_PANEL_ESD_MODE_REG_READ:
		INIT_DELAYED_WORK(&display->esd_check_work,
				dsi_display_esd_check_work);
		break;
	case DSI_PANEL_ESD_MODE_ERR_INT:
		dsi_display_esd_irq_init(display);
		break;
	case DSI_PANEL_ESD_MODE_ERR_INT_AND_REG_READ:
		INIT_DELAYED_WORK(&display->esd_check_work,
				dsi_display_esd_check_work);
		dsi_display_esd_irq_init(display);
		break;
	default:
		DSI_ERROR("invalid esd mode: %s\n",
				dsi_panel_esd_mode_name_get(esd_info->esd_mode));
	}

	DSI_INFO("init esd check mode %s success\n",
			dsi_panel_esd_mode_name_get(esd_info->esd_mode));
	return 0;
}

/**
 * dsi_display_bind - bind dsi device with controlling device
 * @dev:        Pointer to base of platform device
 * @master:     Pointer to container of drm device
 * @data:       Pointer to private data
 *
 * Returns:     Zero on success
 */
static int dsi_display_bind(struct device *dev,
		struct device *master, void *data)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dsi_encoder *dsi_encoder;
	struct drm_connector *connector;
	struct device_node *dev_node;
	struct platform_device *pdev;
	struct dsi_display *display;
	struct drm_encoder *encoder;
	struct drm_device *drm_dev;
	struct dpu_kms *dpu_kms;
	int ret = 0;

	pdev = to_platform_device(dev);
	if (!dev || !pdev || !master) {
		DSI_ERROR("invalid param(s), dev %pK, pdev %pK, master %pK\n",
				dev, pdev, master);
		return -EINVAL;
	}

	drm_dev = dev_get_drvdata(master);
	display = platform_get_drvdata(pdev);
	if (!drm_dev || !display) {
		DSI_ERROR("invalid param(s), drm_dev %pK, display %pK\n",
				drm_dev, display);
		return -EINVAL;
	}

	dpu_drm_dev = container_of(drm_dev, struct dpu_drm_device, drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	display->is_asic = dpu_kms->is_asic;

	if (is_cont_display_enabled())
		dsi_display_initialized_set(display, true);
	else
		dsi_display_initialized_set(display, false);

	dev_node = display->pdev->dev.of_node;
	ret = dsi_display_panel_create(display, dev_node);
	if (ret) {
		DSI_ERROR("create panel failed, ret =%d\n", ret);
		return ret;
	}

	dsi_display_info_init(display, &display->display_info);
	dsi_display_tmg_cfg_init(display, &display->tmg_cfg);

	ret = dsi_host_init(&display->host, &display->pdev->dev);
	if (ret) {
		DSI_ERROR("failed to init host\n");
		goto error_release_panel;
	}
	display->panel->host = &display->host;

	encoder = dsi_encoder_create(drm_dev, display);
	if (IS_ERR_OR_NULL(encoder)) {
		DSI_ERROR("create encoder failed");
		goto error_release_host;
	}

	connector = dsi_connector_create(drm_dev, encoder, display);
	if (!connector) {
		DSI_ERROR("create dsi connector failed\n");
		dsi_encoder_destroy(encoder);
		goto error_release_encoder;
	}

	dsi_encoder = to_dsi_encoder(encoder);
	dsi_encoder->connector = connector;
	ret = drm_connector_attach_encoder(connector, encoder);
	if (ret) {
		DSI_ERROR("failed to attach drm_encoder\n");
		goto error_release_connector;
	}

	display->connector = connector;
	/* init backlight */
	ret = dpu_backlight_init(display, drm_dev, connector);
	if (ret) {
		DSI_ERROR("failed to init backlight\n");
		goto error_release_connector;
	}

	ret = dsi_power_mgr_init(display);
	if (ret) {
		DSI_ERROR("failed to do dsi power mgr init\n");
		goto error_release_backlight;
	}

	ret = dsi_panel_power_mgr_init(&display->dsi_panel_power_mgr);
	if (ret) {
		DSI_ERROR("failed to do dsi panel power mgr init\n");
		goto error_release_dsi_power_mgr;
	}

	ret = dsi_display_post_init(dpu_kms->res_mgr_ctx->dpu_cap.dpu_mem.base);
	if (ret) {
		DPU_ERROR("failed to post init dsi display, ret %d\n", ret);
		goto error_release_panel_power_mgr;
	}

	ret = dsi_display_esd_init(display);
	if (ret) {
		DPU_ERROR("failed to init esd, ret %d\n", ret);
		goto error_release_panel_power_mgr;
	}

#ifdef MI_DISPLAY_MODIFY
	ret = mi_disp_feature_attach_display(display,
			display->display_type, MI_INTF_DSI);
	if (ret) {
		DISP_ERROR("failed to attach %s display(%s intf)\n",
			dsi_display_type_name_get(display->display_type),
			get_disp_intf_type_name(MI_INTF_DSI));
		goto error_release_panel_power_mgr;
	}
#endif

	dpu_obuf_scene_trigger_dsi_port(
			display->display_info.is_dual_port ?
			DPU_OBUF_SCENE_DSI_PORT_DUAL : DPU_OBUF_SCENE_DSI_PORT_SINGLE);

	return 0;

error_release_panel_power_mgr:
	dsi_panel_power_mgr_deinit(display->dsi_panel_power_mgr);
error_release_dsi_power_mgr:
	dsi_power_mgr_deinit();
error_release_backlight:
	dpu_backlight_deinit(display);
error_release_connector:
	dsi_connector_destroy(connector);
error_release_encoder:
	dsi_encoder_destroy(encoder);
error_release_panel:
	dsi_panel_destroy(display->panel);
error_release_host:
	(void)dsi_host_deinit(&display->host);
	return ret;
}

static void dsi_display_unbind(struct device *dev,
		struct device *master, void *data)
{
	struct platform_device *pdev;
	struct dsi_display *display;
#ifdef MI_DISPLAY_MODIFY
	int rc = 0;
#endif

	if (!dev || !master) {
		DSI_ERROR("invalid param(s)\n");
		return;
	}

	pdev = to_platform_device(dev);
	display = platform_get_drvdata(pdev);
	if (!display) {
		DSI_ERROR("invalid display\n");
		return;
	}

	dsi_display_esd_ctrl(display, false);

#ifdef MI_DISPLAY_MODIFY
	rc = mi_disp_feature_detach_display(display,
			display->display_type, MI_INTF_DSI);
	if (rc) {
		DISP_ERROR("failed to detach %s display(%s intf)\n",
			dsi_display_type_name_get(display->display_type),
			get_disp_intf_type_name(MI_INTF_DSI));
	}
#endif
	dsi_power_mgr_deinit();

	dsi_panel_power_mgr_deinit(display->dsi_panel_power_mgr);

	dpu_backlight_deinit(display);

	display->panel->host = NULL;
	dsi_host_deinit(&display->host);

	dsi_panel_destroy(display->panel);

	DSI_DEBUG("dsi unbind\n");
}

static const struct component_ops dsi_display_comp_ops = {
	.bind = dsi_display_bind,
	.unbind = dsi_display_unbind,
};

static int dsi_display_init(struct dsi_display *display)
{
	struct platform_device *pdev = display->pdev;
	int ret = 0;

	mutex_init(&display->display_lock);
	spin_lock_init(&display->mipi_cmd_lock);
	atomic_set(&display->mipi_cmd_state, 0);

	/*  parse display type */
	dsi_display_type_parse(display, pdev);

	/* create port/tmg/panel object */
	dsi_display_lock(display);
	ret = dsi_display_resource_init(display);
	dsi_display_unlock(display);
	if (ret) {
		DSI_ERROR("failed to initialize resources, ret=%d\n", ret);
		goto exit;
	}
exit:
	return ret;
}

static void dsi_display_deinit(struct dsi_display *display)
{
	dsi_display_lock(display);
	dsi_display_resource_deinit(display);
	dsi_display_unlock(display);
}

static int dsi_display_device_probe(struct platform_device *pdev)
{
	struct dsi_display *display = NULL;
	int ret = 0;

	if (!pdev || !pdev->dev.of_node) {
		DSI_ERROR("pdev not found\n");
		return -ENODEV;
	}

	display = kzalloc(sizeof(*display), GFP_KERNEL);
	if (!display)
		return -ENOMEM;

	display->pdev = pdev;
	platform_set_drvdata(pdev, display);

	ret = dsi_display_init(display);
	if (ret) {
		DSI_ERROR("init display object failed\n");
		goto error;
	}

	disp_slots[display->display_type] = display;

	ret = component_add(&pdev->dev, &dsi_display_comp_ops);
	if (ret) {
		DSI_ERROR("component add failed, ret=%d\n", ret);
		goto error_deinit;
	}

	return 0;

error_deinit:
	dsi_display_deinit(display);
error:
	kfree(display);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

int dsi_display_device_remove(struct platform_device *pdev)
{
	struct dsi_display *display;
	int ret = 0;

	if (!pdev) {
		DSI_ERROR("Invalid device\n");
		return -EINVAL;
	}

	display = platform_get_drvdata(pdev);

	dsi_display_deinit(display);

	component_del(&pdev->dev, &dsi_display_comp_ops);
	platform_set_drvdata(pdev, NULL);

	kfree(display);

	return ret;
}

static const struct of_device_id dsi_display_dt_match[] = {
	{ .compatible = "xring,dsi-display" },
	{},
};

static struct platform_driver dsi_display_platform_driver = {
	.probe = dsi_display_device_probe,
	.remove = dsi_display_device_remove,
	.driver = {
		.name = "xring_dsi_display",
		.of_match_table = dsi_display_dt_match,
		.suppress_bind_attrs = true,
	},
};

int __init dsi_display_drv_register(void)
{
	int ret = 0;

#ifdef MI_DISPLAY_MODIFY
	mi_disp_feature_init();
#endif
	ret = dsi_phy_drv_register();
	if (ret) {
		DSI_ERROR("dsi phy driver failed to register\n");
		return ret;
	}

	ret = dsi_ctrl_drv_register();
	if (ret) {
		DSI_ERROR("dsi ctrl driver failed to register\n");
		return ret;
	}

	dsi_panel_event_notifier_init();

	ret = platform_driver_register(&dsi_display_platform_driver);
	if (ret) {
		DSI_ERROR("dsi display driver failed to register\n");
		return ret;
	}

	DSI_DEBUG("dsi disaply drv register success\n");

	return ret;
}

void __exit dsi_display_drv_unregister(void)
{
	platform_driver_unregister(&dsi_display_platform_driver);
	dsi_panel_event_notifier_deinit();
	dsi_ctrl_drv_unregister();
	dsi_phy_drv_unregister();
#ifdef MI_DISPLAY_MODIFY
	mi_disp_feature_deinit();
#endif
}
