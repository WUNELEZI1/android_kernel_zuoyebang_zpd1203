// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 XiaoMi, Inc. All rights reserved.
 */

#define pr_fmt(fmt)	"mi-dsi-panel:[%s:%d] " fmt, __func__, __LINE__
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/rtc.h>
#include <linux/pm_wakeup.h>
#include <video/mipi_display.h>
#include "dsi_host.h"
#include "dsi_connector.h"
#include <display/xring_dpu_drm.h>
#include <soc/xring/display/panel_event_notifier.h>

#include "mi_disp.h"
#include "mi_dsi_panel.h"
#include "mi_disp_feature.h"
#include "mi_disp_print.h"
#include "mi_disp_parser.h"
#include "mi_dsi_display.h"
#include "mi_panel_id.h"
#include "dsi_panel_event_notifier.h"

void mi_dsi_update_backlight_in_aod(struct dsi_panel *panel,
			bool restore_backlight);

static u64 g_panel_id[MI_DISP_MAX];

typedef int (*mi_display_pwrkey_callback)(int);
//extern void mi_display_pwrkey_callback_set(mi_display_pwrkey_callback);

static int mi_panel_id_init(struct dsi_panel *panel)
{
	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	g_panel_id[mi_get_disp_id(panel->mi_panel.type)] = panel->mi_panel.mi_cfg.mi_panel_id;

	return 0;
}

enum mi_project_panel_id mi_get_panel_id_by_dsi_panel(struct dsi_panel *panel)
{
	if (!panel) {
		DISP_ERROR("invalid params\n");
		return PANEL_ID_INVALID;
	}

	return mi_get_panel_id(panel->mi_panel.mi_cfg.mi_panel_id);
}

enum mi_project_panel_id mi_get_panel_id_by_disp_id(int disp_id)
{
	if (!is_support_disp_id(disp_id)) {
		DISP_ERROR("Unsupported display id\n");
		return PANEL_ID_INVALID;
	}

	return mi_get_panel_id(g_panel_id[disp_id]);
}

int mi_dsi_panel_init(struct dsi_panel *panel)
{
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;

	mi_cfg->dsi_panel = panel;
	mutex_init(&mi_cfg->doze_lock);
	mi_cfg->disp_wakelock = wakeup_source_register(NULL, "disp_wakelock");
	if (!mi_cfg->disp_wakelock) {
		DISP_ERROR("doze_wakelock wake_source register failed");
		return -ENOMEM;
	}

	mi_dsi_panel_parse_config(panel);
	mi_panel_id_init(panel);
	atomic_set(&mi_cfg->brightness_clone, 0);

	//mi_display_pwrkey_callback_set(mi_display_powerkey_callback);

	return 0;
}

int mi_dsi_panel_deinit(struct dsi_panel *panel)
{
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;

	if (mi_cfg->disp_wakelock)
		wakeup_source_unregister(mi_cfg->disp_wakelock);

	return 0;
}

int mi_dsi_acquire_wakelock(struct dsi_panel *panel)
{
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;

	if (mi_cfg->disp_wakelock)
		__pm_stay_awake(mi_cfg->disp_wakelock);

	return 0;
}

int mi_dsi_release_wakelock(struct dsi_panel *panel)
{
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;

	if (mi_cfg->disp_wakelock)
		__pm_relax(mi_cfg->disp_wakelock);

	return 0;

}

u32 mi_get_panel_power_mode(struct dsi_panel *panel)
{
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;

	display = container_of(panel->host, struct dsi_display, host);
	dsi_connector = to_dsi_connector(display->connector);

	if (!dsi_connector) {
		DISP_ERROR("invalid parameters, %pK\n", dsi_connector);
		return -EINVAL;
	}
	return dsi_connector->current_power_mode;

}

bool is_aod_and_panel_initialized(struct dsi_panel *panel)
{
	u32 power_mode = mi_get_panel_power_mode(panel);

	if ((power_mode == DPU_POWER_MODE_DOZE || power_mode == DPU_POWER_MODE_DOZE_SUSPEND) &&
			panel->initialized) {
		return true;
	} else {
		return false;
	}
}

bool is_partial_aod_and_panel_initialized(struct dsi_panel *panel)
{
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;

	if (!mi_cfg->fullscreen_aod_status && is_aod_and_panel_initialized(panel))
		return true;
	else
		return false;
}

int mi_dsi_panel_esd_irq_ctrl(struct dsi_panel *panel,
				bool enable)
{
	int ret  = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);
	ret = mi_dsi_panel_esd_irq_ctrl_locked(panel, enable);
	dsi_panel_unlock(panel);

	return ret;
}

int mi_dsi_panel_esd_irq_ctrl_locked(struct dsi_panel *panel,
				bool enable)
{
	struct mi_dsi_panel_cfg *mi_cfg;
	struct irq_desc *desc;

	if (!panel || !panel->initialized) {
		DISP_ERROR("Panel not ready!\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;
	if (gpio_is_valid(mi_cfg->esd_err_irq_gpio)) {
		if (mi_cfg->esd_err_irq) {
			if (enable) {
				if (!mi_cfg->esd_err_enabled) {
					desc = irq_to_desc(mi_cfg->esd_err_irq);
					//if (!irq_settings_is_level(desc))
					//	desc->istate &= ~IRQS_PENDING;
					enable_irq_wake(mi_cfg->esd_err_irq);
					enable_irq(mi_cfg->esd_err_irq);
					mi_cfg->esd_err_enabled = true;
					DISP_INFO("[%s] esd irq is enable\n", panel->mi_panel.type);
				}
			} else {
				if (mi_cfg->esd_err_enabled) {
					disable_irq_wake(mi_cfg->esd_err_irq);
					disable_irq_nosync(mi_cfg->esd_err_irq);
					mi_cfg->esd_err_enabled = false;
					DISP_INFO("[%s] esd irq is disable\n", panel->mi_panel.type);
				}
			}
		}
	} else {
		DISP_INFO("[%s] esd irq gpio invalid\n", panel->mi_panel.type);
	}

	return 0;
}

static void mi_disp_set_dimming_delayed_work_handler(struct kthread_work *k_work)
{
	struct kthread_delayed_work *kdelayed_work =
			container_of(k_work, struct kthread_delayed_work, work);
	struct disp_delayed_work *delayed_work =
			container_of(kdelayed_work, struct disp_delayed_work, delayed_work);
	struct dsi_panel *panel = (struct dsi_panel *)(delayed_work->data);
	struct disp_feature_ctl ctl;

	memset(&ctl, 0, sizeof(struct disp_feature_ctl));
	ctl.feature_id = DISP_FEATURE_DIMMING;
	ctl.feature_val = FEATURE_ON;

	DISP_INFO("[%s] panel set backlight dimming on\n", panel->mi_panel.type);
	dsi_panel_unlock(panel);
	mi_dsi_acquire_wakelock(panel);
	mi_dsi_panel_set_disp_param(panel, &ctl);
	mi_dsi_release_wakelock(panel);
	dsi_panel_lock(panel);

	kfree(delayed_work);
}

int mi_dsi_panel_tigger_dimming_delayed_work(struct dsi_panel *panel)
{
	int disp_id = 0;
	struct disp_feature *df = mi_get_disp_feature();
	struct disp_display *dd_ptr;
	struct disp_delayed_work *delayed_work;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (panel->mi_panel.mi_cfg.disable_ic_dimming) {
		DISP_INFO("disable_ic_dimming is 1\n");
		return 0;
	}
	if (panel->mi_panel.mi_cfg.panel_on_dimming_delay == 0) {
		panel->mi_panel.mi_cfg.ic_dimming_by_feature = FEATURE_ON;
		return 0;
	}

	delayed_work = kzalloc(sizeof(*delayed_work), GFP_KERNEL);
	if (!delayed_work) {
		DISP_ERROR("failed to allocate delayed_work buffer\n");
		return -ENOMEM;
	}

	disp_id = mi_get_disp_id(panel->mi_panel.type);
	dd_ptr = &df->d_display[disp_id];

	kthread_init_delayed_work(&delayed_work->delayed_work,
			mi_disp_set_dimming_delayed_work_handler);
	delayed_work->dd_ptr = dd_ptr;
	delayed_work->wq = &dd_ptr->pending_wq;
	delayed_work->data = panel;

	return kthread_queue_delayed_work(dd_ptr->worker, &delayed_work->delayed_work,
				msecs_to_jiffies(panel->mi_panel.mi_cfg.panel_on_dimming_delay));
}


static void mi_disp_timming_switch_delayed_work_handler(struct kthread_work *k_work)
{
	struct kthread_delayed_work *kdelayed_work =
			container_of(k_work, struct kthread_delayed_work, work);
	struct disp_delayed_work *delayed_work =
			container_of(kdelayed_work, struct disp_delayed_work, delayed_work);
	struct dsi_panel *dsi_panel = (struct dsi_panel *)(delayed_work->data);

	mi_dsi_acquire_wakelock(dsi_panel);

	dsi_panel_lock(dsi_panel);
	if (dsi_panel->initialized) {
		dsi_panel_cmd_set_send(dsi_panel, DSI_CMD_SET_MI_FRAME_SWITCH_MODE_SEC, NULL, USE_CPU);
		DISP_INFO("DSI_CMD_SET_MI_FRAME_SWITCH_MODE_SEC\n");
	} else {
		DISP_ERROR("Panel not initialized, don't send DSI_CMD_SET_MI_FRAME_SWITCH_MODE_SEC\n");
	}
	dsi_panel_unlock(dsi_panel);

	mi_dsi_release_wakelock(dsi_panel);

	kfree(delayed_work);
}

int mi_dsi_panel_tigger_timming_switch_delayed_work(struct dsi_panel *panel)
{
	int disp_id = 0;
	struct disp_feature *df = mi_get_disp_feature();
	struct disp_display *dd_ptr;
	struct disp_delayed_work *delayed_work;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	delayed_work = kzalloc(sizeof(*delayed_work), GFP_KERNEL);
	if (!delayed_work) {
		DISP_ERROR("failed to allocate delayed_work buffer\n");
		return -ENOMEM;
	}

	disp_id = mi_get_disp_id(panel->mi_panel.type);
	dd_ptr = &df->d_display[disp_id];

	kthread_init_delayed_work(&delayed_work->delayed_work,
			mi_disp_timming_switch_delayed_work_handler);
	delayed_work->dd_ptr = dd_ptr;
	delayed_work->wq = &dd_ptr->pending_wq;
	delayed_work->data = panel;
	return kthread_queue_delayed_work(dd_ptr->worker, &delayed_work->delayed_work,
				msecs_to_jiffies(20));
}

bool is_dc_on_skip_backlight(struct dsi_panel *panel, u32 bl_lvl)
{
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;

	if (!mi_cfg->dc_feature_enable)
		return false;
	if (mi_cfg->feature_val[DISP_FEATURE_DC] == FEATURE_OFF)
		return false;
	if (mi_cfg->dc_type == TYPE_CRC_SKIP_BL && bl_lvl < mi_cfg->dc_threshold)
		return true;
	else
		return false;
}

bool is_backlight_set_skip(struct dsi_panel *panel, u32 bl_lvl)
{
	if (bl_lvl != 0 && is_dc_on_skip_backlight(panel, bl_lvl)) {
		DISP_DEBUG("[%s] skip set backlight %d due to DC on\n", panel->mi_panel.type, bl_lvl);
		return true;
	} else if (is_partial_aod_and_panel_initialized(panel) &&
		panel->mi_panel.mi_cfg.sf_transfer_power_mode == HAL_POWERMODE_DOZE &&
		((panel->mi_panel.mi_cfg.panel_state != PANEL_STATE_ON &&
		panel->mi_panel.mi_cfg.need_restore_last_bl != true) ||
		(panel->mi_panel.mi_cfg.panel_state == PANEL_STATE_ON &&
		panel->mi_panel.mi_cfg.need_restore_last_bl == true)) &&
		panel->mi_panel.mi_cfg.fp_unlock_value == FINGERPRINT_UNLOCK_SUCCESS) {
		panel->mi_panel.mi_cfg.need_restore_last_bl = true;
		DISP_INFO("[%s] skip set backlight %d due to fp already unlock success but panel in DOZE state\n",
				panel->mi_panel.type, bl_lvl);
		return true;
	} else {
		return false;
	}
}

void mi_dsi_panel_update_last_bl_level(struct dsi_panel *panel, int brightness)
{
	struct mi_dsi_panel_cfg *mi_cfg;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;

	if (mi_cfg->dimming_state == STATE_DIM_RESTORE && brightness > 0) {
		mi_dsi_panel_tigger_dimming_delayed_work(panel);
		mi_cfg->dimming_state = STATE_NONE;
	}

	atomic_set(&mi_cfg->last_bl_level, brightness);
	if (brightness != 0 && panel->mi_panel.mi_cfg.sf_transfer_power_mode == HAL_POWERMODE_ON)
		mi_cfg->last_no_zero_bl_level = brightness;
}

int mi_dsi_print_51_backlight_log(struct dsi_panel *panel,
		struct dsi_cmd_desc *cmd)
{
	u8 *buf = NULL;
	u32 bl_lvl = 0;
	int i = 0;
	struct mi_dsi_panel_cfg *mi_cfg;
	static int use_count = 20;

	if (!panel || !cmd) {
		DISP_ERROR("Invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;
	buf = (u8 *)cmd->msg.tx_buf;
	if (buf && buf[0] == MIPI_DCS_SET_DISPLAY_BRIGHTNESS) {
		if (cmd->msg.tx_len >= 3) {
			if (panel->panel_info.bl_config.bl_inverted_dbv)
				bl_lvl = (buf[1] << 8) | buf[2];
			else
				bl_lvl = buf[1] | (buf[2] << 8);

			//if (use_count-- > 0)
			DISP_TIME_INFO("[%s] set 51 backlight %d\n", panel->mi_panel.type, bl_lvl);

			if (!atomic_read(&mi_cfg->last_bl_level) || !bl_lvl)
				use_count = 20;
		}

		if (mi_get_backlight_log_mask() & BACKLIGHT_LOG_ENABLE) {
			DISP_INFO("[%s] [0x51 backlight debug] tx_len = %lu\n",
					panel->mi_panel.type, cmd->msg.tx_len);
			for (i = 0; i < cmd->msg.tx_len; i++) {
				DISP_INFO("[%s] [0x51 backlight debug] tx_buf[%d] = 0x%02X\n",
					panel->mi_panel.type, i, buf[i]);
			}

			if (mi_get_backlight_log_mask() & BACKLIGHT_LOG_DUMP_STACK)
				dump_stack();
		}
	}

	return 0;
}

int mi_dsi_update_51_mipi_cmd(struct dsi_panel *panel,
			enum dsi_cmd_set_type type, int bl_lvl)
{
	struct dsi_cmd_set_update_info *info;
	struct dsi_panel_mode *current_mode;
	struct dsi_cmd_set *cmd_set;
	u8 bl_buf[2] = {(bl_lvl >> 8) & 0xff, bl_lvl & 0xff};
	int size = 2;
	int i = 0;

	if (!panel || !panel->panel_info.current_mode) {
		DSI_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	DISP_INFO("[%s] bl_lvl = %d\n", panel->mi_panel.type, bl_lvl);

	if (type == DSI_CMD_SET_MI_APLDIMMING) {
		bl_buf[0] = bl_lvl & 0xff;
		size = 1;
	}

	current_mode = panel->panel_info.current_mode;
	cmd_set = &current_mode->cmd_sets[type];
	if (!cmd_set->num_infos) {
		DSI_DEBUG("no update info %d\n", cmd_set->num_infos);
		return 0;
	}
	for (i = 0; i < cmd_set->num_infos; ++i) {
		info = &cmd_set->infos[i];
		DISP_INFO("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
			panel->mi_panel.type, cmd_set_names[type],
			info->addr, info->index, info->len);
		if (info->addr != 0x51) {
			DISP_INFO("[%s] error mipi address (0x%02X)\n", panel->mi_panel.type, info->addr);
			continue;
		} else {
			if (size == 1)
				dsi_panel_cmd_set_update(cmd_set, i, bl_buf, size);
			else
				dsi_panel_cmd_set_update(cmd_set, i, bl_buf, sizeof(bl_buf));
			break;
		}
	}

	return 0;
}

void mi_dsi_panel_update_ic_dimming_by_bl(struct dsi_panel *panel, u32 bl_lvl)
{
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;

	if (mi_cfg->disable_ic_dimming || mi_cfg->panel_state != PANEL_STATE_ON
			|| mi_get_panel_id_by_dsi_panel(panel) != O2S_PANEL_PA ||
			panel->mi_panel.mi_cfg.sf_transfer_power_mode != HAL_POWERMODE_ON)
		return;
	if (mi_cfg->ic_dimming_by_feature == FEATURE_OFF)
		return;
	if (bl_lvl >= mi_cfg->ic_dimming_dbv_threshold &&
			mi_cfg->dimming_state != STATE_DIM_BLOCK &&
			mi_cfg->feature_val[DISP_FEATURE_DIMMING] == FEATURE_ON) {
		DISP_INFO("[%s] set dimming off\n", panel->mi_panel.type);
		dsi_panel_cmd_set_send(panel, DSI_CMD_SET_MI_DIMMINGOFF, NULL, USE_CPU);
		mi_cfg->feature_val[DISP_FEATURE_DIMMING] = FEATURE_OFF;
	} else if (bl_lvl < mi_cfg->ic_dimming_dbv_threshold &&
			mi_cfg->dimming_state != STATE_DIM_BLOCK &&
			mi_cfg->feature_val[DISP_FEATURE_DIMMING] == FEATURE_OFF)  {
		DISP_INFO("[%s] set dimming on\n", panel->mi_panel.type);
		dsi_panel_cmd_set_send(panel, DSI_CMD_SET_MI_DIMMINGON, NULL, USE_CPU);
		mi_cfg->feature_val[DISP_FEATURE_DIMMING] = FEATURE_ON;
	}
}

int mi_dsi_panel_backlight_prepare(struct dsi_panel *panel,
		u32 brightness)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel || (brightness > DSI_BL_LEVEL_MAX)) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}
	mi_cfg = &panel->mi_panel.mi_cfg;

	if (mi_get_backlight_log_mask() & BACKLIGHT_LOG_DUMP_STACK)
		dump_stack();

	if (is_backlight_set_skip(panel, brightness)) {
		mi_dsi_panel_update_last_bl_level(panel, brightness);
		return -1;
	}

	if (brightness == 0) {
		if (!mi_cfg->whether_backlight_zero) {
			mi_cfg->timestamp_backlight_zero = get_jiffies_64();
			if (mi_get_panel_power_mode(panel) == DPU_POWER_MODE_ON)
				mi_cfg->whether_backlight_zero = true;
		}
	}

	DISP_TIME_INFO("set panel backlight as %d\n", brightness);
	mi_dsi_panel_update_ic_dimming_by_bl(panel, brightness);

	// add hbm event notifier for o2s
	if (mi_get_panel_id_by_dsi_panel(panel) == O2S_PANEL_PA) {
		if (mi_cfg->bl_normal_max) {
			if ((brightness >= mi_cfg->bl_normal_max &&
				atomic_read(&mi_cfg->last_bl_level) < mi_cfg->bl_normal_max) ||
				(mi_cfg->need_restore_last_bl && brightness >= mi_cfg->bl_normal_max)) {
				dsi_panel_event_notifier_trigger(DRM_PANEL_EVENT_HBM_ON, panel);
			} else if ((brightness < mi_cfg->bl_normal_max &&
				atomic_read(&mi_cfg->last_bl_level) >= mi_cfg->bl_normal_max) ||
				(mi_cfg->need_restore_last_bl && brightness < mi_cfg->bl_normal_max)) {
				dsi_panel_event_notifier_trigger(DRM_PANEL_EVENT_HBM_OFF, panel);
			}
		}
	}

	mi_dsi_panel_update_last_bl_level(panel, brightness);

	mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
			MI_DISP_EVENT_51_BRIGHTNESS, sizeof(brightness), brightness);
	mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
			MI_DISP_EVENT_BACKLIGHT, sizeof(brightness), brightness);
	mi_dsi_panel_set_brightness_clone(panel, brightness);

	return rc;
}

int dsi_panel_alloc_cmd_packets(struct dsi_cmd_set *cmd,
		u32 packet_count)
{
	u32 size;

	size = packet_count * sizeof(*cmd->cmds);
	cmd->cmds = kzalloc(size, GFP_KERNEL);
	if (!cmd->cmds)
		return -ENOMEM;

	cmd->num_cmds = packet_count;
	return 0;
}

void dsi_panel_dealloc_cmd_packets(struct dsi_cmd_set *set)
{
	kfree(set->cmds);
}

void dsi_panel_destroy_cmd_packets(struct dsi_cmd_set *set)
{
	u32 i = 0;
	struct dsi_cmd_desc *cmd;

	for (i = 0; i < set->num_cmds; i++) {
		cmd = &set->cmds[i];
		kfree(cmd->msg.tx_buf);
	}
}

int mi_dsi_panel_parse_sub_timing(struct mi_mode_info *mi_mode,
				  struct device_node *np)
{
	int rc = 0;
	const char *ddic_mode;

	rc = of_property_read_u32(np, "dsi-panel-frame-rate",
		&mi_mode->timing_refresh_rate);
	if (rc) {
		DISP_ERROR("failed to get dsi-panel-frame-rate!\n");
		goto error;
	}

	rc = of_property_read_string(np, "mi,dsi-panel-ddic-mode",
			&ddic_mode);
	if (rc) {
		DISP_ERROR("failed to get mi,dsi-panel-ddic-mode!\n");
		goto error;
	}
	if (ddic_mode) {
		if (!strcmp(ddic_mode, "normal")) {
			mi_mode->ddic_mode = DDIC_MODE_NORMAL;
		} else if (!strcmp(ddic_mode, "idle")) {
			mi_mode->ddic_mode = DDIC_MODE_IDLE;
		} else if (!strcmp(ddic_mode, "auto")) {
			mi_mode->ddic_mode = DDIC_MODE_AUTO;
		} else if (!strcmp(ddic_mode, "qsync")) {
			mi_mode->ddic_mode = DDIC_MODE_QSYNC;
		} else if (!strcmp(ddic_mode, "diff")) {
			mi_mode->ddic_mode = DDIC_MODE_DIFF;
		} else if (!strcmp(ddic_mode, "test")) {
			mi_mode->ddic_mode = DDIC_MODE_TEST;
		} else {
			DISP_INFO("Unrecognized ddic mode, default is normal mode\n");
			mi_mode->ddic_mode = DDIC_MODE_NORMAL;
		}
	} else {
		DISP_DEBUG("Falling back ddic mode to default normal mode\n");
		mi_mode->ddic_mode = DDIC_MODE_NORMAL;
		mi_mode->sf_refresh_rate = mi_mode->timing_refresh_rate;
		mi_mode->ddic_min_refresh_rate = mi_mode->timing_refresh_rate;
	}

	if (mi_mode->ddic_mode != DDIC_MODE_NORMAL) {
		rc = of_property_read_u32(np, "mi,dsi-panel-sf-framerate",
				&mi_mode->sf_refresh_rate);
		if (rc) {
			DISP_ERROR("failed to read mi,dsi-panel-sf-framerate, rc=%d\n", rc);
			goto error;
		}
		rc = of_property_read_u32(np, "mi,dsi-panel-ddic-min-framerate",
					&mi_mode->ddic_min_refresh_rate);
		if (rc) {
			DISP_ERROR("failed to read mi,dsi-panel-ddic-min-framerate, rc=%d\n", rc);
			goto error;
		}
	}

	DISP_INFO("ddic_mode:%s, sf_refresh_rate:%d, ddic_min_refresh_rate:%d\n",
		get_ddic_mode_name(mi_mode->ddic_mode),
		mi_mode->sf_refresh_rate, mi_mode->ddic_min_refresh_rate);

error:
	return rc;
}

int mi_dsi_panel_write_cmd_set(struct dsi_panel *panel,
				struct dsi_cmd_set *cmd_sets)
{
	struct dsi_display *display;
	int rc = 0, i = 0;
	ssize_t len;
	struct dsi_cmd_desc *cmds;
	u32 count, transfer_flag;
	u32 port;

	enum dsi_cmd_set_state state;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}


	cmds = cmd_sets->cmds;
	count = cmd_sets->num_cmds;
	state = cmd_sets->cmd_state;
	port = cmd_sets->port_index;

	if (count == 0) {
		DISP_DEBUG("[%s] No commands to be sent for state\n", panel->mi_panel.type);
		goto error;
	}

	display = container_of(panel->host, struct dsi_display, host);
	for (i = 0; i < count; i++) {

		if (state == DSI_CMD_SET_STATE_LP)
			cmds->msg.flags |= MIPI_DSI_MSG_USE_LPM;

		cmds->port_index = port;
		transfer_flag = dsi_basic_cmd_flag_get(cmds, USE_CPU, i, count);
		len = dsi_display_cmd_transfer_locked(display, cmds, transfer_flag);
		if (len < 0) {
			rc = len;
			DISP_ERROR("failed to set cmds, rc=%d\n", rc);
		}
		if (cmds->post_wait_us)
			usleep_range(cmds->post_wait_us,
					((cmds->post_wait_us) + 10));
		cmds++;
	}
error:
	return rc;
}

int mi_dsi_panel_read_batch_number(struct dsi_panel *panel)
{
	int rc = 0;
	unsigned long mode_flags_backup = 0;
	u8 rdbuf[8];
	ssize_t read_len = 0;
	u8 read_batch_number = 0;

	int i = 0;
	struct panel_batch_info info[] = {
		{0x00, "P0.0"},
		{0x01, "P0.1"},
		{0x10, "P1.0"},
		{0x11, "P1.1"},
		{0x12, "P1.2"},
		{0x13, "P1.2"},
		{0x20, "P2.0"},
		{0x21, "P2.1"},
		{0x30, "MP"},
	};

	if (!panel || !panel->host) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}
	dsi_panel_lock(panel);

	mode_flags_backup = panel->mipi_device.mode_flags;
	panel->mipi_device.mode_flags |= MIPI_DSI_MODE_LPM;
	read_len = mipi_dsi_dcs_read(&panel->mipi_device, 0xDA, rdbuf, 1);
	panel->mipi_device.mode_flags = mode_flags_backup;
	if (read_len > 0) {
		read_batch_number = rdbuf[0];
		panel->mi_panel.mi_cfg.panel_batch_number = read_batch_number;
		for (i = 0; i < ARRAY_SIZE(info); i++) {
			if (read_batch_number == info[i].batch_number) {
				DISP_INFO("panel batch is %s\n", info[i].batch_name);
				break;
			}
		}
		rc = 0;
		panel->mi_panel.mi_cfg.panel_batch_number_read_done = true;
	} else {
		DISP_ERROR("failed to read panel batch number\n");
		panel->mi_panel.mi_cfg.panel_batch_number = 0;
		rc = -EAGAIN;
		panel->mi_panel.mi_cfg.panel_batch_number_read_done = false;
	}

	dsi_panel_unlock(panel);
	return rc;
}

int mi_dsi_panel_write_dsi_cmd_set(struct dsi_panel *panel,
			int type)
{
	int rc = 0;
	int i = 0, j = 0;
	u8 *tx_buf = NULL;
	u8 *buffer = NULL;
	int buf_size = 1024;
	u32 cmd_count = 0;
	int buf_count = 1024;
	struct dsi_cmd_desc *cmds;
	enum dsi_cmd_set_state state;
	struct dsi_panel_mode *mode;

	if (!panel || !panel->panel_info.current_mode || type < 0 || type >= DSI_CMD_SET_MAX) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	buffer = kzalloc(buf_size, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	dsi_panel_lock(panel);

	mode = panel->panel_info.current_mode;
	cmds = mode->cmd_sets[type].cmds;
	cmd_count = mode->cmd_sets[type].num_cmds;
	state = mode->cmd_sets[type].cmd_state;

	if (cmd_count == 0) {
		DISP_ERROR("[%s] No commands to be sent\n", cmd_set_names[type]);
		rc = -EAGAIN;
		goto error;
	}

	DISP_INFO("set cmds [%s], count (%d), state(%s)\n",
		cmd_set_names[type], cmd_count,
		(state == DSI_CMD_SET_STATE_LP) ? "dsi_lp_mode" : "dsi_hs_mode");

	for (i = 0; i < cmd_count; i++) {
		memset(buffer, 0, buf_size);
		buf_count = snprintf(buffer, buf_size, "%02zX", cmds->msg.tx_len);
		tx_buf = (u8 *)cmds->msg.tx_buf;
		for (j = 0; j < cmds->msg.tx_len ; j++) {
			buf_count += snprintf(buffer + buf_count,
					buf_size - buf_count, " %02X", tx_buf[j]);
		}
		DISP_DEBUG("[%d] %s\n", i, buffer);
		cmds++;
	}

	rc = dsi_panel_cmd_set_send(panel, type, NULL, USE_CPU);

error:
	dsi_panel_unlock(panel);
	kfree(buffer);
	return rc;
}

ssize_t mi_dsi_panel_show_dsi_cmd_set_type(struct dsi_panel *panel,
			char *buf, size_t size)
{
	ssize_t count = 0;
	int type = 0;

	if (!panel || !buf) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	count = snprintf(buf, size, "%s: dsi cmd_set name\n", "id");

	for (type = DSI_CMD_SET_ON; type < DSI_CMD_SET_MAX; type++) {
		count += snprintf(buf + count, size - count, "%02d: %s\n",
				     type, cmd_set_names[type]);
	}

	return count;
}

int mi_dsi_panel_set_doze_brightness(struct dsi_panel *panel,
			u32 doze_brightness, u8 transfer_type)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}
	dsi_panel_lock(panel);
	mi_dsi_panel_set_doze_brightness_lock(panel, doze_brightness, transfer_type);
	dsi_panel_unlock(panel);
	return rc;
}

int mi_dsi_panel_set_doze_brightness_lock(struct dsi_panel *panel,
			u32 doze_brightness, u8 transfer_type)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg;
	struct dsi_display *display;

	if (!dsi_panel_initialized(panel)) {
		DISP_ERROR("panel is not initialized!\n");
		rc = -EINVAL;
		return rc;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;

	display = container_of(panel->host, struct dsi_display, host);

	if (mi_cfg->panel_state == PANEL_STATE_ON
		|| mi_cfg->doze_brightness != doze_brightness) {
		if (doze_brightness == DOZE_BRIGHTNESS_HBM) {
			mi_cfg->panel_state = PANEL_STATE_DOZE_HIGH;
			rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_DOZE_HBM, NULL, transfer_type);
			if (rc) {
				DISP_ERROR("[%s] failed to send DOZE_HBM cmd, rc=%d\n",
					panel->mi_panel.type, rc);
			}
		} else if (doze_brightness == DOZE_BRIGHTNESS_LBM) {
			mi_cfg->panel_state = PANEL_STATE_DOZE_LOW;
			rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_DOZE_LBM, NULL, transfer_type);
			if (rc) {
				DISP_ERROR("[%s] failed to send DOZE_LBM cmd, rc=%d\n",
					panel->mi_panel.type, rc);
			}
		}
		mi_cfg->last_doze_brightness = mi_cfg->doze_brightness;
		mi_cfg->doze_brightness = doze_brightness;
		DISP_TIME_INFO("[%s] set doze brightness to %s\n",
			panel->mi_panel.type, get_doze_brightness_name(doze_brightness));
	} else {
		DISP_INFO("[%s] %s has been set, skip\n", panel->mi_panel.type,
			get_doze_brightness_name(doze_brightness));
	}
	return rc;
}

int mi_dsi_panel_get_doze_brightness(struct dsi_panel *panel,
			u32 *doze_brightness)
{
	struct mi_dsi_panel_cfg *mi_cfg;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);

	mi_cfg = &panel->mi_panel.mi_cfg;
	*doze_brightness =  mi_cfg->doze_brightness;

	dsi_panel_unlock(panel);

	return 0;
}

int mi_dsi_panel_get_brightness(struct dsi_panel *panel,
			u32 *brightness)
{
	struct mi_dsi_panel_cfg *mi_cfg;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);

	mi_cfg = &panel->mi_panel.mi_cfg;
	*brightness = atomic_read(&mi_cfg->last_bl_level);

	dsi_panel_unlock(panel);

	return 0;
}

int mi_dsi_panel_write_dsi_cmd(struct dsi_panel *panel,
			struct dsi_cmd_rw_ctl *ctl)
{
	struct dsi_cmd_set cmd_sets = {0};
	u32 packet_count = 0;
	int rc = 0;

	dsi_panel_lock(panel);

	if (!panel || !panel->initialized) {
		DISP_ERROR("Panel not initialized!\n");
		rc = -EAGAIN;
		goto exit_unlock;
	}

	if (!ctl->tx_len || !ctl->tx_ptr) {
		DISP_ERROR("[%s] invalid params\n", panel->mi_panel.type);
		rc = -EINVAL;
		goto exit_unlock;
	}

	rc = dsi_panel_get_cmd_pkt_count(ctl->tx_ptr, ctl->tx_len, &packet_count);
	if (rc) {
		DISP_ERROR("[%s] write dsi commands failed, rc=%d\n",
			panel->mi_panel.type, rc);
		goto exit_unlock;
	}

	DISP_DEBUG("[%s] packet-count=%d\n", panel->mi_panel.type, packet_count);

	rc = dsi_panel_alloc_cmd_packets(&cmd_sets, packet_count);
	if (rc) {
		DISP_ERROR("[%s] failed to allocate cmd packets, rc=%d\n",
			panel->mi_panel.type, rc);
		goto exit_unlock;
	}

	rc = dsi_panel_create_cmd_packets(ctl->tx_ptr, ctl->tx_len, packet_count,
				cmd_sets.cmds);
	if (rc) {
		DISP_ERROR("[%s] failed to create cmd packets, rc=%d\n",
			panel->mi_panel.type, rc);
		goto exit_free1;
	}

	if (ctl->tx_state == MI_DSI_CMD_LP_STATE) {
		cmd_sets.cmd_state = DSI_CMD_SET_STATE_LP;
	} else if (ctl->tx_state == MI_DSI_CMD_HS_STATE) {
		cmd_sets.cmd_state = DSI_CMD_SET_STATE_HS;
	} else {
		DISP_ERROR("[%s] command state unrecognized\n",
			panel->mi_panel.type);
		goto exit_free1;
	}
	cmd_sets.port_index = ctl->is_master_port ? 0 : 1;

	rc = mi_dsi_panel_write_cmd_set(panel, &cmd_sets);
	if (rc) {
		DISP_ERROR("[%s] failed to send cmds, rc=%d\n", panel->mi_panel.type, rc);
		goto exit_free2;
	}

exit_free2:
	if (ctl->tx_len && ctl->tx_ptr)
		dsi_panel_destroy_cmd_packets(&cmd_sets);
exit_free1:
	if (ctl->tx_len && ctl->tx_ptr)
		dsi_panel_dealloc_cmd_packets(&cmd_sets);
exit_unlock:
	dsi_panel_unlock(panel);
	return rc;
}

int mi_dsi_panel_set_brightness_clone(struct dsi_panel *panel,
			u32 brightness_clone)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg;
	int disp_id = MI_DISP_PRIMARY;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;

	if (brightness_clone > mi_cfg->max_brightness_clone)
		brightness_clone = mi_cfg->max_brightness_clone;

	atomic_set(&mi_cfg->brightness_clone, brightness_clone);

	disp_id = mi_get_disp_id(panel->mi_panel.type);
	mi_disp_feature_event_notify_by_type(disp_id,
			MI_DISP_EVENT_BRIGHTNESS_CLONE,
			sizeof(u32), atomic_read(&mi_cfg->brightness_clone));

	return rc;
}

int mi_dsi_panel_get_brightness_clone(struct dsi_panel *panel,
			u32 *brightness_clone)
{
	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	*brightness_clone = atomic_read(&panel->mi_panel.mi_cfg.brightness_clone);

	return 0;
}

int mi_dsi_panel_get_max_brightness_clone(struct dsi_panel *panel,
			u32 *max_brightness_clone)
{
	struct mi_dsi_panel_cfg *mi_cfg;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;
	*max_brightness_clone =  mi_cfg->max_brightness_clone;

	return 0;
}

void mi_dsi_update_backlight_in_aod(struct dsi_panel *panel, bool restore_backlight)
{
	int bl_lvl = 0;
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_panel.mi_cfg;
	struct mipi_dsi_device *dsi = &panel->mipi_device;

	if (restore_backlight) {
		bl_lvl = atomic_read(&mi_cfg->last_bl_level);
	} else {
		switch (mi_cfg->doze_brightness) {
		case DOZE_BRIGHTNESS_HBM:
			bl_lvl = mi_cfg->doze_hbm_dbv_level;
			break;
		case DOZE_BRIGHTNESS_LBM:
			bl_lvl = mi_cfg->doze_lbm_dbv_level;
			break;
		default:
			return;
		}
	}
	DISP_INFO("[%s] %s bl_lvl=%d\n",
			panel->mi_panel.type, __func__, bl_lvl);
	if (panel->panel_info.bl_config.bl_inverted_dbv)
		bl_lvl = (((bl_lvl & 0xff) << 8) | (bl_lvl >> 8));
	mipi_dsi_dcs_set_display_brightness(dsi, bl_lvl);

}

/* Note: Factory version need flat cmd send out immediately,
 * do not care it may lead panel flash.
 * Dev version need flat cmd send out send with te
 */
static int mi_dsi_send_flat_sync_with_te_locked(struct dsi_panel *panel,
			bool enable)
{
	int rc = 0;
#ifdef CONFIG_FACTORY_BUILD
	struct dsi_display *display;

	display = container_of(panel->host, struct dsi_display, host);
	if (enable) {
		rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_FLAT_MODE_ON, NULL, USE_CPU);
		rc |= dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_FLAT_MODE_SEC_ON, NULL, USE_CPU);
	} else {
		rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_FLAT_MODE_OFF, NULL, USE_CPU);
		rc |= dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_FLAT_MODE_SEC_OFF, NULL, USE_CPU);
	}
	DISP_INFO("send flat %s cmd immediately", enable ? "ON" : "OFF");
#else
	DISP_DEBUG("flat cmd should send out sync with te");
#endif
	mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
			MI_DISP_EVENT_FLAT_MODE, sizeof(int), enable ? 1 : 0);

	return rc;
}

int mi_dsi_panel_csc_by_temper_comp(struct dsi_panel *panel, int temp_val, bool is_possible_in_suspend)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}
	dsi_panel_lock(panel);
	rc = mi_dsi_panel_csc_by_temper_comp_locked(panel, temp_val, is_possible_in_suspend);
	dsi_panel_unlock(panel);
	return rc;
}

int mi_dsi_panel_csc_by_temper_comp_locked(struct dsi_panel *panel, int temp_val, bool is_possible_in_suspend)
{
	int rc = 0;
	int real_temp_val = temp_val;
	struct mi_dsi_panel_cfg *mi_cfg = NULL;
	struct dsi_display *display;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;
	display = container_of(panel->host, struct dsi_display, host);
	if (mi_get_panel_id_by_dsi_panel(panel) != O2S_PANEL_PA)
		return rc;

	if (panel->mi_panel.id_config.build_id < PANEL_P20) {
		rc = mi_dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_OFF_MODE, NULL, USE_CPU, is_possible_in_suspend);
		return rc;
	}
	if (temp_val < 32)
		rc = mi_dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_OFF_MODE, NULL, USE_CPU, is_possible_in_suspend);
	else if (temp_val >= 32 && temp_val < 36)
		rc = mi_dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_32_36_MODE, NULL, USE_CPU, is_possible_in_suspend);
	else if (temp_val >= 36 && temp_val < 40)
		rc = mi_dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_36_40_MODE, NULL, USE_CPU, is_possible_in_suspend);
	else if (temp_val >= 40 && temp_val < 45)
		rc = mi_dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_40_MODE, NULL, USE_CPU, is_possible_in_suspend);
	else
		rc = mi_dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_45_MODE, NULL, USE_CPU, is_possible_in_suspend);

	mi_cfg->real_dbi_state = real_temp_val;
	return rc;
}

int mi_dsi_panel_set_round_corner_locked(struct dsi_panel *panel,
			bool enable)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (panel->mi_panel.mi_cfg.ddic_round_corner_enabled) {
		DISP_INFO("[%s] ddic round corner feature is enabled\n", panel->mi_panel.type);

		if (enable) {
			if ((panel->mi_panel.mi_cfg.ddic_round_corner_port_mode | ROUND_CORNER_CMD_DUAL_PORT) == ROUND_CORNER_CMD_DUAL_PORT) {
				DISP_INFO("set ddic round corner dual port\n");
				rc = dsi_panel_cmd_set_send(panel, DSI_CMD_SET_MI_ROUND_CORNER_ON, NULL, USE_CPU);
				if (rc)
					DISP_ERROR("[%s] failed to send ROUND_CORNER(%s) cmds, rc=%d\n",
						panel->mi_panel.type, enable ? "On" : "Off", rc);
			}

			if ((panel->mi_panel.mi_cfg.ddic_round_corner_port_mode & ROUND_CORNER_CMD_PORT0) == ROUND_CORNER_CMD_PORT0) {
				DISP_INFO("set ddic round corner port0 On\n");
				rc = dsi_panel_cmd_set_send(panel, DSI_CMD_SET_MI_ROUND_CORNER_PORT0_ON, NULL, USE_CPU);
				if (rc)
					DISP_ERROR("[%s] failed to send ROUND_CORNER(%s) cmds, rc=%d\n",
						panel->mi_panel.type, enable ? "On" : "Off", rc);
			}

			if ((panel->mi_panel.mi_cfg.ddic_round_corner_port_mode & ROUND_CORNER_CMD_PORT1) == ROUND_CORNER_CMD_PORT1) {
				DISP_INFO("set ddic round corner port1 On\n");
				rc = dsi_panel_cmd_set_send(panel, DSI_CMD_SET_MI_ROUND_CORNER_PORT1_ON, NULL, USE_CPU);
				if (rc)
					DISP_ERROR("[%s] failed to send ROUND_CORNER(%s) cmds, rc=%d\n",
						panel->mi_panel.type, enable ? "On" : "Off", rc);
			}
		} else {
			rc = dsi_panel_cmd_set_send(panel, DSI_CMD_SET_MI_ROUND_CORNER_OFF, NULL, USE_CPU);
			if (rc)
				DISP_ERROR("[%s] failed to send ROUND_CORNER(%s) cmds, rc=%d\n",
					panel->mi_panel.type, enable ? "On" : "Off", rc);
		}
	} else {
		DISP_INFO("[%s] ddic round corner feature not enabled\n", panel->mi_panel.type);
	}

	return rc;
}

int mi_dsi_panel_set_round_corner(struct dsi_panel *panel, bool enable)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);

	rc = mi_dsi_panel_set_round_corner_locked(panel, enable);

	dsi_panel_unlock(panel);

	return rc;
}

int mi_dsi_panel_set_fp_unlock_state(struct dsi_panel *panel,
			u32 fp_unlock_value)
{
	int rc = 0;
	int update_bl = 0;
	struct mi_dsi_panel_cfg *mi_cfg = NULL;
	struct dsi_display *display;

	dsi_panel_lock(panel);

	if (!dsi_panel_initialized(panel)) {
		DISP_ERROR("panel is not initialized!\n");
		rc = -EINVAL;
		goto exit;
	}
	DISP_INFO("fp_unlock_value = %d\n", fp_unlock_value);
	mi_cfg = &panel->mi_panel.mi_cfg;
	if (fp_unlock_value != FINGERPRINT_UP)
		mi_cfg->fp_unlock_value = fp_unlock_value;
	display = container_of(panel->host, struct dsi_display, host);
	if (is_aod_and_panel_initialized(panel) &&
		!mi_cfg->fullscreen_aod_status &&
		(mi_cfg->panel_state == PANEL_STATE_DOZE_HIGH || mi_cfg->panel_state == PANEL_STATE_DOZE_LOW) &&
		fp_unlock_value == FINGERPRINT_UNLOCK_SUCCESS) {
		if (mi_get_panel_id(mi_cfg->mi_panel_id) == O2S_PANEL_PA) {
			if (mi_cfg->last_no_zero_bl_level < 412) {
				mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_NOLP, update_bl);
				rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_NOLP, NULL, USE_CPU);
				mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
						MI_DISP_EVENT_POWER, sizeof(int), MI_DISP_POWER_ON);
				mi_cfg->need_restore_last_bl = true;
				mi_cfg->panel_state = PANEL_STATE_ON;
			}
		}
	}

exit:
	dsi_panel_unlock(panel);

	return rc;
}

int mi_dsi_panel_set_dsi_port0_round_corner(struct dsi_panel *panel, bool enable)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);

	rc = mi_dsi_panel_set_round_corner_locked(panel, enable);

	dsi_panel_unlock(panel);

	return rc;
}

int mi_dsi_panel_set_dsi_port1_round_corner(struct dsi_panel *panel, bool enable)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);

	rc = mi_dsi_panel_set_round_corner_locked(panel, enable);

	dsi_panel_unlock(panel);

	return rc;
}

int mi_dsi_panel_set_dc_mode(struct dsi_panel *panel, bool enable, bool is_possible_in_suspend)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);
	rc = mi_dsi_panel_set_dc_mode_locked(panel, enable, is_possible_in_suspend);
	dsi_panel_unlock(panel);

	return rc;
}

int mi_dsi_panel_set_dc_mode_locked(struct dsi_panel *panel, bool enable, bool is_possible_in_suspend)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg  = NULL;
	struct dsi_display *display;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;
	display = container_of(panel->host, struct dsi_display, host);

	if (mi_cfg->dc_feature_enable) {
		if (enable) {
			rc = mi_dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_DC_ON, NULL, USE_CPU, is_possible_in_suspend);
			mi_cfg->real_dc_state = FEATURE_ON;
		} else {
			rc = mi_dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_DC_OFF, NULL, USE_CPU, is_possible_in_suspend);
			mi_cfg->real_dc_state = FEATURE_OFF;
		}

		if (rc)
			DISP_ERROR("failed to set DC mode: %d\n", enable);
		else
			DISP_INFO("DC mode: %s\n", enable ? "On" : "Off");

	} else {
		DISP_INFO("DC mode: TODO\n");
	}

	return rc;
}

int mi_dsi_panel_enable(struct dsi_panel *panel)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	mi_cfg = &panel->mi_panel.mi_cfg;

	if (mi_cfg->ddic_round_corner_enabled) {
#ifdef CONFIG_FACTORY_BUILD
		rc = mi_dsi_panel_set_round_corner_locked(panel, false);
		DISP_TIME_INFO("[%s] ddic round corner DSI_CMD_SET_MI_ROUND_CORNER_OFF\n",
				panel->mi_panel.type);
#else
		rc = mi_dsi_panel_set_round_corner_locked(panel, true);
		DISP_TIME_INFO("[%s] ddic round corner DSI_CMD_SET_MI_ROUND_CORNER_ON\n",
				panel->mi_panel.type);
#endif
		if (rc)
			DISP_ERROR("[%s] failed to send ROUND_CORNER cmds, rc=%d\n",
					panel->mi_panel.type, rc);
	} else {
		DISP_INFO("[%s] ddic round corner feature not enabled\n", panel->mi_panel.type);
	}
	if (mi_cfg->flatmode_default_on_enabled) {
		mi_cfg->feature_val[DISP_FEATURE_FLAT_MODE] = FEATURE_ON;
		mi_cfg->flat_cfg.cur_flat_state = FEATURE_ON;
	}
	if (mi_cfg->dc_feature_enable &&
		mi_cfg->feature_val[DISP_FEATURE_DC] == FEATURE_ON) {
		mi_dsi_panel_set_dc_mode_locked(panel, true, false);
	}

	if ((mi_get_panel_id_by_dsi_panel(panel) == O2S_PANEL_PA) &&
		(panel->mi_panel.id_config.build_id < PANEL_P20)) {
		rc = mi_dsi_panel_csc_by_temper_comp_locked(panel, 0, false);
	}

	if (mi_cfg->feature_val[DISP_FEATURE_DBI]) {
		if ((mi_get_panel_id_by_dsi_panel(panel) == O2S_PANEL_PA) &&
			(panel->mi_panel.id_config.build_id >= PANEL_P20)) {
			rc = mi_dsi_panel_csc_by_temper_comp_locked(panel, mi_cfg->feature_val[DISP_FEATURE_DBI], false);
			DISP_DEBUG("[%s] panel csc setting, rc=[%d]\n", panel->mi_panel.type, rc);
		}
	}

	mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
			MI_DISP_EVENT_POWER, sizeof(int), MI_DISP_POWER_ON);
	mi_cfg->panel_state = PANEL_STATE_ON;

	return rc;
}

int mi_dsi_panel_disable(struct dsi_panel *panel)
{
	int rc = 0;
	u64 jiffies_time = 0;
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	mi_cfg = &panel->mi_panel.mi_cfg;
	atomic_set(&mi_cfg->last_bl_level, 0);
	mi_cfg->fp_unlock_value = FINGERPRINT_UNLOCK_FAIL;
	mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
			MI_DISP_EVENT_POWER, sizeof(int), MI_DISP_POWER_OFF);
	mi_cfg->feature_val[DISP_FEATURE_HBM] = FEATURE_OFF;
	mi_cfg->feature_val[DISP_FEATURE_FLAT_MODE] = FEATURE_OFF;
	mi_cfg->dimming_state = STATE_NONE;
	mi_cfg->panel_state = PANEL_STATE_OFF;
	mi_cfg->is_peak_hdr = false;
	if (mi_cfg->whether_backlight_zero) {
		mi_cfg->whether_backlight_zero = false;
		jiffies_time = get_jiffies_64();
		if (jiffies_time - mi_cfg->timestamp_backlight_zero > HZ)
			DISP_ERROR("[%s] Backlight is 0 when the screen is on\n", panel->mi_panel.type);
	}

	return rc;
}

int mi_dsi_panel_timing_switch(struct dsi_panel *panel)
{
	struct xring_panel_event_notification notification = {0};
	struct dsi_panel_mode *current_mode = NULL;
	int ret = 0;

	if (!panel) {
		DISP_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	current_mode = panel->panel_info.current_mode;
	notification.type = DRM_PANEL_EVENT_FPS_CHANGE;
	notification.data.early_trigger = 0;
	notification.panel_node = panel->drm_panel.dev->of_node;
	if (current_mode->mi_timing.ddic_mode == DDIC_MODE_IDLE)
		notification.data.new_fps = current_mode->mi_timing.ddic_min_refresh_rate;
	else
		notification.data.new_fps = current_mode->cur_timing->framerate;
	DISP_TIME_INFO("fps change to %s:%d\n", get_ddic_mode_name(current_mode->mi_timing.ddic_mode),
			notification.data.new_fps);
	if (panel->mi_panel.mi_cfg.fps_change_notify_enabled)
		xring_panel_event_notifier_trigger(panel->panel_info.is_primary ? XRING_PANEL_EVENT_TAG_PRIMARY :
				XRING_PANEL_EVENT_TAG_SECONDARY, &notification);
	if (current_mode->mi_timing.ddic_mode == DDIC_MODE_AUTO)
		notification.data.new_fps = current_mode->mi_timing.ddic_min_refresh_rate;
	mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
			MI_DISP_EVENT_FPS, sizeof(notification.data.new_fps), notification.data.new_fps);
	mi_disp_feature_sysfs_notify(mi_get_disp_id(panel->mi_panel.type), MI_SYSFS_DYNAMIC_FPS);

	return ret;
}

bool mi_dsi_panel_need_tx_or_rx_cmd(u32 feature_id)
{
	switch (feature_id) {
	case DISP_FEATURE_SENSOR_LUX:
	case DISP_FEATURE_FOLD_STATUS:
	case DISP_FEATURE_FULLSCREEN_AOD_STATUS:
		return false;
	default:
		return true;
	}
}

int mi_dsi_panel_set_disp_param(struct dsi_panel *panel, struct disp_feature_ctl *ctl)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg = NULL;
	struct dsi_display *display;

	if (!panel || !ctl) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}


	display = container_of(panel->host, struct dsi_display, host);
	dsi_panel_lock(panel);

	DISP_TIME_INFO("[%s] feature: %s, value: %d\n", panel->mi_panel.type,
			get_disp_feature_id_name(ctl->feature_id), ctl->feature_val);


	mi_cfg = &panel->mi_panel.mi_cfg;

	if (!panel->initialized &&
		mi_dsi_panel_need_tx_or_rx_cmd(ctl->feature_id)) {
		if (ctl->feature_id == DISP_FEATURE_DC)
			mi_cfg->feature_val[DISP_FEATURE_DC] = ctl->feature_val;
		if (ctl->feature_id == DISP_FEATURE_DBI)
			mi_cfg->feature_val[DISP_FEATURE_DBI] = ctl->feature_val;
		DISP_WARN("[%s] panel not initialized!\n", panel->mi_panel.type);
		rc = -ENODEV;
		goto exit;
	}

	switch (ctl->feature_id) {
	case DISP_FEATURE_DIMMING:
		if (!mi_cfg->disable_ic_dimming) {
			if (mi_cfg->dimming_state != STATE_DIM_BLOCK) {
				mi_cfg->ic_dimming_by_feature = ctl->feature_val;
				if (ctl->feature_val == FEATURE_ON)
					rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_DIMMINGON, NULL, USE_CPU);
				else
					rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_DIMMINGOFF, NULL, USE_CPU);
				mi_cfg->feature_val[DISP_FEATURE_DIMMING] = ctl->feature_val;
			} else {
				DISP_INFO("skip dimming %s\n", ctl->feature_val ? "on" : "off");
			}
		} else {
			DISP_INFO("disable_ic_dimming is %d\n", mi_cfg->disable_ic_dimming);
		}
		break;
	case DISP_FEATURE_HBM:
		mi_cfg->feature_val[DISP_FEATURE_HBM] = ctl->feature_val;
#ifdef CONFIG_FACTORY_BUILD
		if (ctl->feature_val == FEATURE_ON) {
			rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_HBM_ON, NULL, USE_CPU);
			mi_cfg->dimming_state = STATE_DIM_BLOCK;
		} else {
			mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_HBM_OFF,
					atomic_read(&mi_cfg->last_bl_level));
			rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_HBM_OFF, NULL, USE_CPU);
			mi_cfg->dimming_state = STATE_DIM_RESTORE;
		}
#endif
		mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
				MI_DISP_EVENT_HBM, sizeof(ctl->feature_val), ctl->feature_val);
		break;
	case DISP_FEATURE_DOZE_BRIGHTNESS:
#ifdef CONFIG_FACTORY_BUILD
		if (dsi_panel_initialized(panel) &&
			is_aod_brightness(ctl->feature_val)) {
			if (ctl->feature_val == DOZE_BRIGHTNESS_HBM)
				rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_DOZE_HBM, NULL, USE_CPU);
			else
				rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_DOZE_LBM, NULL, USE_CPU);
		} else {
			rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_NOLP, NULL, USE_CPU);
			dsi_panel_unlock(panel);
			dsi_display_unlock(display);
			backlight_device_set_brightness(panel->bl_device, atomic_read(&mi_cfg->last_bl_level));
			dsi_display_lock(display);
			dsi_panel_lock(panel);
		}
#else
		if (is_aod_and_panel_initialized(panel) &&
			is_aod_brightness(ctl->feature_val)) {
			if (ctl->feature_val == DOZE_BRIGHTNESS_HBM)
				rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_DOZE_HBM, NULL, USE_CPU);
			else
				rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_DOZE_LBM, NULL, USE_CPU);
		} else {
			rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_NOLP, NULL, USE_CPU);
		}
#endif
		mi_cfg->feature_val[DISP_FEATURE_DOZE_BRIGHTNESS] = ctl->feature_val;
		break;
	case DISP_FEATURE_FLAT_MODE:
		if (!mi_cfg->flat_sync_te) {
			if (ctl->feature_val == FEATURE_ON) {
				DISP_INFO("flat mode on\n");
				//mi_dsi_update_flat_mode_on_cmd(panel, DSI_CMD_SET_MI_FLAT_MODE_ON);
				rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_FLAT_MODE_ON, NULL, USE_CPU);
				rc |= dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_FLAT_MODE_SEC_ON, NULL, USE_CPU);
			} else {
				DISP_INFO("flat mode off\n");
				rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_FLAT_MODE_OFF, NULL, USE_CPU);
				rc |= dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_FLAT_MODE_SEC_OFF, NULL, USE_CPU);
			}
			mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
				MI_DISP_EVENT_FLAT_MODE, sizeof(ctl->feature_val), ctl->feature_val);
		} else {
			rc = mi_dsi_send_flat_sync_with_te_locked(panel,
					ctl->feature_val == FEATURE_ON);
		}
		mi_cfg->feature_val[DISP_FEATURE_FLAT_MODE] = ctl->feature_val;
		break;
	case DISP_FEATURE_CRC:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_DC:
		if (mi_cfg->dc_feature_enable)
			rc = mi_dsi_panel_set_dc_mode_locked(panel, ctl->feature_val == FEATURE_ON, true);

		mi_cfg->feature_val[DISP_FEATURE_DC] = ctl->feature_val;
		mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
				MI_DISP_EVENT_DC, sizeof(ctl->feature_val), ctl->feature_val);
		break;
	case DISP_FEATURE_SENSOR_LUX:
		mi_cfg->feature_val[DISP_FEATURE_SENSOR_LUX] = ctl->feature_val;
		break;
	case DISP_FEATURE_FOLD_STATUS:
		mi_cfg->feature_val[DISP_FEATURE_FOLD_STATUS] = ctl->feature_val;
		break;
	case DISP_FEATURE_NATURE_FLAT_MODE:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_SPR_RENDER:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_COLOR_INVERT:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_DC_BACKLIGHT:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_GIR:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_DBI:
		if (mi_cfg->feature_val[DISP_FEATURE_DBI] == ctl->feature_val) {
			DISP_INFO("gray_level is the same, return\n");
			break;
		}

		if ((mi_get_panel_id_by_dsi_panel(panel) == O2S_PANEL_PA) &&
			(panel->mi_panel.id_config.build_id < PANEL_P20)) {
			rc = mi_dsi_panel_csc_by_temper_comp_locked(panel, 0, true);
		}

		if ((mi_get_panel_id_by_dsi_panel(panel) == O2S_PANEL_PA) &&
			(panel->mi_panel.id_config.build_id >= PANEL_P20)) {
			rc = mi_dsi_panel_csc_by_temper_comp_locked(panel, ctl->feature_val, true);
			DISP_DEBUG("[%s] panel csc setting, rc=[%d]\n", panel->mi_panel.type, rc);
		}
		mi_cfg->feature_val[DISP_FEATURE_DBI] = ctl->feature_val;
		break;
	case DISP_FEATURE_DDIC_ROUND_CORNER:
		rc = mi_dsi_panel_set_round_corner_locked(panel, ctl->feature_val == FEATURE_ON);
		mi_cfg->feature_val[DISP_FEATURE_DDIC_ROUND_CORNER] = ctl->feature_val;
		break;
	case DISP_FEATURE_HBM_BACKLIGHT:
		if (mi_cfg->bl_normal_max &&
				ctl->feature_val > mi_cfg->bl_normal_max &&
				atomic_read(&mi_cfg->last_bl_level) <= mi_cfg->bl_normal_max) {
			dsi_panel_event_notifier_trigger(DRM_PANEL_EVENT_HBM_ON, panel);
		} else if (mi_cfg->bl_normal_max &&
				ctl->feature_val <= mi_cfg->bl_normal_max &&
				atomic_read(&mi_cfg->last_bl_level) > mi_cfg->bl_normal_max) {
			dsi_panel_event_notifier_trigger(DRM_PANEL_EVENT_HBM_OFF, panel);
		}
		atomic_set(&mi_cfg->last_bl_level, ctl->feature_val);
		dsi_panel_unlock(panel);
		dsi_display_unlock(display);
		backlight_device_set_brightness(panel->bl_device, atomic_read(&mi_cfg->last_bl_level));
		dsi_display_lock(display);
		dsi_panel_lock(panel);
		break;
	case DISP_FEATURE_BACKLIGHT:
		break;
	case DISP_FEATURE_FULLSCREEN_AOD_STATUS:
		mi_cfg->feature_val[DISP_FEATURE_FULLSCREEN_AOD_STATUS] = ctl->feature_val;
		mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
			MI_DISP_EVENT_FULLSCREEN_AOD, sizeof(ctl->feature_val), ctl->feature_val);
		mi_cfg->fullscreen_aod_status = ctl->feature_val;
		break;
	case DISP_FEATURE_APL_DIMMING:
		mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_APLDIMMING, ctl->feature_val);
		rc = dsi_display_cmd_set_send_locked(display, DSI_CMD_SET_MI_APLDIMMING, NULL, USE_CPU);
		DISP_INFO("apl_dimming setting, ctl->feature_val=%d, rc=%d\n", ctl->feature_val, rc);
		break;
	default:
		DISP_ERROR("invalid feature argument: %d\n", ctl->feature_id);
		break;
	}
exit:
	dsi_panel_unlock(panel);
	return rc;
}

int mi_dsi_panel_get_disp_param(struct dsi_panel *panel,
			struct disp_feature_ctl *ctl)
{
	struct mi_dsi_panel_cfg *mi_cfg;
	int i = 0;

	if (!panel || !ctl) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (!is_support_disp_feature_id(ctl->feature_id)) {
		DISP_ERROR("unsupported disp feature id\n");
		return -EINVAL;
	}

	dsi_panel_lock(panel);
	mi_cfg = &panel->mi_panel.mi_cfg;
	for (i = DISP_FEATURE_DIMMING; i < DISP_FEATURE_MAX; i++) {
		if (i == ctl->feature_id) {
			ctl->feature_val =  mi_cfg->feature_val[i];
			DISP_INFO("%s: %d\n", get_disp_feature_id_name(ctl->feature_id),
				ctl->feature_val);
		}
	}
	dsi_panel_unlock(panel);

	return 0;
}

ssize_t mi_dsi_panel_show_disp_param(struct dsi_panel *panel,
			char *buf, size_t size)
{
	struct mi_dsi_panel_cfg *mi_cfg;
	ssize_t count = 0;
	int i = 0;

	if (!panel || !buf || !size) {
		DISP_ERROR("invalid params\n");
		return -EAGAIN;
	}

	count = snprintf(buf, size, "%40s: feature vaule\n", "feature name[feature id]");

	dsi_panel_lock(panel);
	mi_cfg = &panel->mi_panel.mi_cfg;
	for (i = DISP_FEATURE_DIMMING; i < DISP_FEATURE_MAX; i++) {
		count += snprintf(buf + count, size - count, "%36s[%02d]: %d\n",
				get_disp_feature_id_name(i), i, mi_cfg->feature_val[i]);

	}
	dsi_panel_unlock(panel);

	return count;
}

int dsi_panel_parse_build_id_read_config(struct dsi_panel *panel)
{
	struct drm_panel_build_id_config *id_config;
	struct device_node *np;
	int rc = 0;

	if (!panel) {
		DISP_ERROR("Invalid Params\n");
		return -EINVAL;
	}

	np = panel->mipi_device.dev.of_node;
	id_config = &panel->mi_panel.id_config;
	id_config->build_id = 0;
	rc = of_property_read_u32(np, "mi,panel-build-id-read-length",
			&id_config->id_cmds_rlen);
	if (rc) {
		id_config->id_cmds_rlen = 0;
		return -EINVAL;
	}
	dsi_panel_cmd_single_parse(&id_config->id_cmd,
			DSI_CMD_SET_MI_PANEL_BUILD_ID, np);
	if (!(id_config->id_cmd.num_cmds)) {
		DISP_ERROR("panel build id read command parsing failed\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_panel_parse_wp_reg_read_config(struct dsi_panel *panel)
{
	struct drm_panel_wp_config *wp_config;
	struct device_node *np;
	int rc = 0;

	if (!panel) {
		DISP_ERROR("Invalid Params\n");
		return -EINVAL;
	}

	np = panel->mipi_device.dev.of_node;
	wp_config = &panel->mi_panel.wp_config;
	if (!wp_config)
		return -EINVAL;

	dsi_panel_cmd_single_parse(&wp_config->wp_cmd,
			DSI_CMD_SET_MI_PANEL_WP_READ, np);
	if (!wp_config->wp_cmd.num_cmds) {
		DISP_ERROR("wp_info read command parsing failed\n");
		return -EINVAL;
	}

	dsi_panel_cmd_single_parse(&wp_config->pre_tx_cmd,
			DSI_CMD_SET_MI_PANEL_WP_READ_PRE_TX, np);
	if (!wp_config->pre_tx_cmd.num_cmds)
		DISP_INFO("wp_info pre command parsing failed\n");

	rc = of_property_read_u32(np, "mi,dsi-panel-wp-read-length",
				&wp_config->wp_cmds_rlen);
	if (rc || !wp_config->wp_cmds_rlen) {
		wp_config->wp_cmds_rlen = 0;
		return -EINVAL;
	}

	rc = of_property_read_u32(np, "mi,dsi-panel-wp-read-index",
				&wp_config->wp_read_info_index);
	if (rc || !wp_config->wp_read_info_index)
		wp_config->wp_read_info_index = 0;

	wp_config->return_buf = kcalloc(wp_config->wp_cmds_rlen,
		sizeof(unsigned char), GFP_KERNEL);
	if (!wp_config->return_buf)
		return -ENOMEM;

	return 0;
}

int dsi_panel_parse_cell_id_read_config(struct dsi_panel *panel)
{
	struct drm_panel_cell_id_config *cell_id_config;
	struct device_node *np;
	int rc = 0;

	if (!panel) {
		DISP_ERROR("Invalid Params\n");
		return -EINVAL;
	}

	np = panel->mipi_device.dev.of_node;
	cell_id_config = &panel->mi_panel.cell_id_config;
	if (!cell_id_config)
		return -EINVAL;

	dsi_panel_cmd_single_parse(&cell_id_config->cell_id_cmd,
			DSI_CMD_SET_MI_PANEL_CELL_ID_READ, np);
	if (!cell_id_config->cell_id_cmd.num_cmds) {
		DISP_ERROR("cell_id_info read command parsing failed\n");
		return -EINVAL;
	}

	dsi_panel_cmd_single_parse(&cell_id_config->pre_tx_cmd,
			DSI_CMD_SET_MI_PANEL_CELL_ID_READ_PRE_TX, np);
	if (!cell_id_config->pre_tx_cmd.num_cmds)
		DISP_INFO("cell_id_info pre command parsing failed\n");

	dsi_panel_cmd_single_parse(&cell_id_config->after_tx_cmd,
			DSI_CMD_SET_MI_PANEL_CELL_ID_READ_AFTER_TX, np);
	if (!cell_id_config->after_tx_cmd.num_cmds)
		DISP_INFO("cell_id_info after command parsing failed\n");

	rc = of_property_read_u32(np, "mi,dsi-panel-cell-id-read-length",
				&cell_id_config->cell_id_cmds_rlen);
	if (rc || !cell_id_config->cell_id_cmds_rlen) {
		cell_id_config->cell_id_cmds_rlen = 0;
		return -EINVAL;
	}

	rc = of_property_read_u32(np, "mi,dsi-panel-cell-id-read-index",
				&cell_id_config->cell_id_read_info_index);
	if (rc || !cell_id_config->cell_id_read_info_index)
		cell_id_config->cell_id_read_info_index = 0;

	cell_id_config->return_buf = kcalloc(cell_id_config->cell_id_cmds_rlen,
		sizeof(unsigned char), GFP_KERNEL);
	if (!cell_id_config->return_buf)
		return -ENOMEM;

	return 0;
}

int mi_dsi_panel_parse_dc_fps_config(struct dsi_panel *panel,
		struct dsi_panel_mode *mode)
{
	int rc;
	u32 dc_6c_cfg[2] = {0, 0};
	struct device_node *np;

	np = panel->mipi_device.dev.of_node;

	rc = of_property_read_u32_array(np, "mi,dsi-panel-dc-status-control-dc-6C-cfg",
		dc_6c_cfg, sizeof(dc_6c_cfg));
	if (rc)
		DISP_DEBUG("mi,dsi-panel-dc-status-control-dc-6C-cfg not defined rc=%d\n", rc);
	else
		DISP_INFO("FPS: x, dc 6C cfg: 0x%02X, 0x%02X\n", dc_6c_cfg[0], dc_6c_cfg[1]);

	return 0;
}

int mi_dsi_panel_demura_set_by_dbv(struct dsi_panel *panel, u32 bl_lvl)
{
	int rc = 0;
	struct panel_demura_info *demura_info;
	enum dsi_cmd_set_type demura_type;

	if (!panel)
		return -EINVAL;

	demura_info = &panel->mi_panel.mi_cfg.demura_info;

	if (bl_lvl == 0 || demura_info->demura_change_dbv_threshold == 0) {
		demura_info->demura_type = DSI_CMD_SET_MAX;
		return 0;
	}
	if (bl_lvl  >= demura_info->demura_change_dbv_threshold)
		demura_type = DSI_CMD_SET_MI_DEMURA_HBM;
	else
		demura_type = DSI_CMD_SET_MI_DEMURA_LBM;

	if ((demura_info->demura_type != demura_type) && (demura_type != DSI_CMD_SET_MAX)) {
		rc = dsi_panel_cmd_set_send(panel, demura_type, NULL, USE_CPU);
		demura_info->demura_type = demura_type;
		if (rc) {
			DISP_ERROR("(%s) send  cmds(%d) failed! bl_lvl(%d),rc(%d)\n",
					panel->panel_info.name, demura_type, bl_lvl, rc);
			return rc;
		}
		DISP_DEBUG("Send demura setttings (%d), bl_lvl = 0x%x\n",
				demura_type, bl_lvl);
	}

	return rc;
}

int mi_dsi_panel_set_doze(struct dsi_panel *panel)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg;

	mi_cfg = &panel->mi_panel.mi_cfg;
	if (mi_cfg->feature_val[DISP_FEATURE_FULLSCREEN_AOD_STATUS]) {
		if (mi_get_panel_id(mi_cfg->mi_panel_id) == O2S_PANEL_PA) {
			DISP_INFO("[%s] set dimming on\n", panel->mi_panel.type);
			rc = dsi_panel_cmd_set_send(panel, DSI_CMD_SET_MI_DIMMINGON, NULL, USE_CPU);
			mi_cfg->feature_val[DISP_FEATURE_DIMMING] = FEATURE_ON;
		}
	}
	mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
			MI_DISP_EVENT_POWER, sizeof(int), MI_DISP_POWER_LP1);

	return rc;
}

int mi_dsi_panel_set_doze_suspend(struct dsi_panel *panel)
{
	int rc = 0;

	mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
			MI_DISP_EVENT_POWER, sizeof(int), MI_DISP_POWER_LP2);

	return rc;
}

int mi_dsi_panel_set_nolp_locked(struct dsi_panel *panel)
{
	int rc = 0;
	int update_bl = 0;
	u32 doze_brightness = 0;
	struct mi_dsi_panel_cfg *mi_cfg;
	struct dsi_display *display;

	DISP_TIME_INFO("enter");

	mi_cfg = &panel->mi_panel.mi_cfg;
	doze_brightness = mi_cfg->doze_brightness;
	display = container_of(panel->host, struct dsi_display, host);

	if (mi_cfg->panel_state == PANEL_STATE_ON) {
		DISP_INFO("panel already PANEL_STATE_ON, skip nolp");
		if (mi_cfg->need_restore_last_bl && atomic_read(&mi_cfg->last_bl_level) != 0) {
			DISP_INFO("need restore last backlight %d", atomic_read(&mi_cfg->last_bl_level));
			mi_cfg->need_restore_last_bl = false;
			dsi_panel_unlock(panel);
			dsi_display_unlock(display);
			backlight_device_set_brightness(panel->bl_device, atomic_read(&mi_cfg->last_bl_level));
			dsi_display_lock(display);
			dsi_panel_lock(panel);
		}
		return rc;
	}

	if (doze_brightness == DOZE_TO_NORMAL)
		doze_brightness = mi_cfg->last_doze_brightness;

	switch (doze_brightness) {
	case DOZE_BRIGHTNESS_HBM:
		DISP_INFO("set doze_hbm_dbv_level in nolp");
		update_bl = mi_cfg->doze_hbm_dbv_level;
		break;
	case DOZE_BRIGHTNESS_LBM:
		DISP_INFO("set doze_lbm_dbv_level in nolp");
		update_bl = mi_cfg->doze_lbm_dbv_level;
		break;
	default:
		break;
	}

	if (mi_get_panel_id(mi_cfg->mi_panel_id) == O2S_PANEL_PA ||
		mi_get_panel_id(mi_cfg->mi_panel_id) == O80_PANEL_PB) {
		if (atomic_read(&mi_cfg->last_bl_level) == 0) {
			if (mi_cfg->last_no_zero_bl_level < update_bl)
				update_bl = atomic_read(&mi_cfg->last_bl_level);
		} else {
			update_bl = atomic_read(&mi_cfg->last_bl_level);
		}
		mi_dsi_panel_backlight_prepare(panel, update_bl);
		mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_NOLP, update_bl);
		rc = dsi_panel_cmd_set_send(panel, DSI_CMD_SET_NOLP, NULL, USE_CPU);
	}
	mi_cfg->need_restore_last_bl = false;

	return rc;
}

int mi_dsi_panel_exit_lp_locked(struct dsi_panel *panel)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_panel.mi_cfg;

	rc = mi_dsi_panel_set_nolp_locked(panel);

	if (mi_cfg->feature_val[DISP_FEATURE_FULLSCREEN_AOD_STATUS]) {
		if (mi_get_panel_id(mi_cfg->mi_panel_id) == O2S_PANEL_PA) {
			DISP_INFO("[%s] set dimming off\n", panel->mi_panel.type);
			rc = dsi_panel_cmd_set_send(panel, DSI_CMD_SET_MI_DIMMINGOFF, NULL, USE_CPU);
			mi_cfg->feature_val[DISP_FEATURE_DIMMING] = FEATURE_OFF;
		}
	}

	mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->mi_panel.type),
		MI_DISP_EVENT_POWER, sizeof(int), MI_DISP_POWER_ON);
	mi_cfg->panel_state = PANEL_STATE_ON;

	return rc;
}

int mi_dsi_panel_set_count_info(struct dsi_panel *panel, struct disp_count_info *count_info)
{
	int rc = 0;
	u32 power_mode = DPU_POWER_MODE_OFF;

	if (!panel || !count_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	DISP_TIME_INFO("[%s] count info type: %s, value: %d\n", panel->mi_panel.type,
		get_disp_count_info_type_name(count_info->count_info_type), count_info->count_info_val);

	switch (count_info->count_info_type) {
	case DISP_COUNT_INFO_POWERSTATUS:
		power_mode = mi_get_panel_power_mode(panel);
		DISP_INFO("sf power get:%d, last kernel power mode = %d\n", count_info->count_info_val, power_mode);
		panel->mi_panel.mi_cfg.sf_transfer_power_mode = count_info->count_info_val;
		if (count_info->count_info_val == HAL_POWERMODE_ON)
			panel->mi_panel.mi_cfg.dimming_state = STATE_DIM_RESTORE;
		break;
	case DISP_COUNT_INFO_SYSTEM_BUILD_VERSION:
		DISP_INFO("system build version:%s\n", count_info->tx_ptr);
		break;
	case DISP_COUNT_INFO_FRAME_DROP_COUNT:
		break;
	case DISP_COUNT_INFO_SWITCH_KERNEL_FUNCTION_TIMER:
		DISP_INFO("swith function timer:%d\n", count_info->count_info_val);
		break;
	default:
		break;
	}

	return rc;
}
