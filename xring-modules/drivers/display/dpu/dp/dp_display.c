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

#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include "dp_display.h"
#include "dp_parser.h"
#include "dpu_log.h"
#include "dp_drm.h"
#include "dpu_flow_ctrl.h"
#include "dpu_obuf.h"
#include "dpu_power_mgr.h"
#if IS_ENABLED(CONFIG_SWITCH_FSA4480)
#include <soc/xring/fsa4480-i2c.h>
#endif

static struct dp_display *disp_slots[MAX_DP_DISPLAY_NUM] = {NULL};

struct dp_display *dp_display_get(u8 index)
{
	if (index >= MAX_DP_DISPLAY_NUM) {
		DP_ERROR("invalid display index: %u, max: %u", index, MAX_DP_DISPLAY_NUM);
		return NULL;
	}

	return disp_slots[index];
}

int dp_handle_hpd_event(enum dp_hpd_event hpd_event)
{
	struct dp_display *display;

	display = dp_display_get(DP_DEVICE_ID_0);
	if (display == NULL) {
		DP_ERROR("dp device is not prepared yet\n");
		return -EBUSY;
	}

	if (hpd_event < 0 || hpd_event >= DP_HPD_EVENT_MAX) {
		DP_ERROR("invalid parameter of hpd_event %d\n", hpd_event);
		return -EINVAL;
	}

	if (display->power->state != DP_POWER_STATE_ON) {
		DP_ERROR("displayport hardware module is in power-off state\n");
		return -EBUSY;
	}

	dp_hpd_handle_event(display->hpd, hpd_event);

	return 0;
}

int dp_manage_power(bool power_on, enum dp_orientation orientation, unsigned int usb_combo_mode)
{
	struct dp_display *display;
	enum dp_combo_mode combo_mode;
#if IS_ENABLED(CONFIG_SWITCH_FSA4480)
	enum fsa_function event = FSA_USBC_DISPLAYPORT_DISCONNECTED;
	int rc = 0;
#endif

	display = dp_display_get(DP_DEVICE_ID_0);
	if (display == NULL) {
		DP_ERROR("dp device is not prepared yet\n");
		return -EBUSY;
	}

	if (!power_on) {
		display->power->funcs->off(display->power);
		return 0;
	}

	if (orientation != DP_ORIENTATION_OBSERVE
			&& orientation != DP_ORIENTATION_RESERVE) {
		DP_ERROR("invalid parameter of orientation %d", orientation);
		return -EINVAL;
	}

	if (usb_combo_mode == DP_MODE) {
		combo_mode = DP_COMBO_MODE_DP4;
	} else if (usb_combo_mode == USBDP_MODE) {
		combo_mode = DP_COMBO_MODE_DP2;
	} else {
		DP_ERROR("invalid parameter of combo mode %u", usb_combo_mode);
		return -EINVAL;
	}

	display->status.combo_mode = combo_mode;
	display->status.orientation = orientation;
	DP_INFO("set dp combo mode: %s, orientation: %s\n",
			combo_mode == DP_COMBO_MODE_DP2 ? "combo" : " dp only",
			orientation == DP_ORIENTATION_OBSERVE ? "observe" : "reserve");
#if IS_ENABLED(CONFIG_SWITCH_FSA4480)
	if (orientation == DP_ORIENTATION_OBSERVE)
		event = FSA_USBC_ORIENTATION_CC1;
	else
		event = FSA_USBC_ORIENTATION_CC2;
	DP_INFO("orientation=%d, event=%d\n", orientation, event);
	rc = fsa4480_switch_event_set(event);
	if (rc)
		DP_ERROR("failed to configure fsa4480 i2c device (%d)\n", rc);
#endif

	dp_hw_sctrl_set_g_combo_mode(combo_mode);
	dp_hw_sctrl_set_g_orientation(orientation);

	display->power->funcs->on(display->power);

	return 0;
}

static irqreturn_t dp_display_irq_handler(int irq, void *data)
{
	struct dp_display *display = data;
	struct dp_intr_ops *intr_item;
	unsigned long flags;

	spin_lock_irqsave(&display->lock, flags);
	list_for_each_entry(intr_item, &display->irq_list, list) {
		if (intr_item->is_triggered(intr_item->data)) {
			intr_item->handler(intr_item->data);
			intr_item->clear(intr_item->data);
		}
	}
	spin_unlock_irqrestore(&display->lock, flags);

	return IRQ_HANDLED;
}

void dp_display_irq_register(struct dp_display *display,
		struct dp_intr_ops *ops)
{
	unsigned long flags;

	spin_lock_irqsave(&display->lock, flags);
	list_add_tail(&ops->list, &display->irq_list);
	spin_unlock_irqrestore(&display->lock, flags);
}

void dp_display_irq_unregister(struct dp_display *display,
		struct dp_intr_ops *ops)
{
	unsigned long flags;

	spin_lock_irqsave(&display->lock, flags);
	list_del_init(&ops->list);
	spin_unlock_irqrestore(&display->lock, flags);
}

void dp_display_irq_enable(struct dp_display *display, bool enable)
{
	struct dp_intr_ops *intr_item;
	unsigned long flags;

	spin_lock_irqsave(&display->lock, flags);
	list_for_each_entry(intr_item, &display->irq_list, list)
		intr_item->enable(intr_item->data, enable);
	spin_unlock_irqrestore(&display->lock, flags);
}

static int dp_display_intr_init(struct dp_display *display)
{
	int ret;

	ret = request_irq(display->parser->dptx_irq, &dp_display_irq_handler,
			0, "dptx_irq", display);
	if (ret) {
		DP_ERROR("failed to request dptx irq, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static void dp_display_intr_deinit(struct dp_display *display)
{
	disable_irq(display->parser->dptx_irq);
	free_irq(display->parser->dptx_irq, display);
}

/* video stream interrupt operation */
static int dp_display_video_irq_enable(void *data, bool enable)
{
	struct dp_display *display = data;
	struct dp_hw_ctrl *hw_ctrl = display->hw_modules.hw_ctrl;

	hw_ctrl->ops->enable_intr(&hw_ctrl->hw, DP_INTR_VIDEO_FIFO_OVERFLOW_0, enable);
	hw_ctrl->ops->enable_intr(&hw_ctrl->hw, DP_INTR_VIDEO_FIFO_UNDERFLOW_0, enable);
	hw_ctrl->ops->enable_intr(&hw_ctrl->hw, DP_INTR_AUDIO_FIFO_OVERFLOW_0, enable);

	return 0;
}

static bool dp_display_video_irq_is_triggered(void *data)
{
	struct dp_display *display = data;
	struct dp_hw_ctrl *hw_ctrl = display->hw_modules.hw_ctrl;
	bool is_triggered = false;
	bool ret = false;

	if (!display->status.is_connector_enabled)
		return false;

	is_triggered = hw_ctrl->ops->get_intr_state(&hw_ctrl->hw,
			DP_INTR_VIDEO_FIFO_OVERFLOW_0);
	atomic_set(&display->stream_overflow_flag, is_triggered ? 1 : 0);
	ret |= is_triggered;

	is_triggered = hw_ctrl->ops->get_intr_state(&hw_ctrl->hw,
			DP_INTR_VIDEO_FIFO_UNDERFLOW_0);
	atomic_set(&display->stream_underflow_flag, is_triggered ? 1 : 0);
	ret |= is_triggered;

	is_triggered = hw_ctrl->ops->get_intr_state(&hw_ctrl->hw,
			DP_INTR_AUDIO_FIFO_OVERFLOW_0);
	atomic_set(&display->audio_overflow_flag, is_triggered ? 1 : 0);
	ret |= is_triggered;

	return ret;
}

static int dp_display_video_irq_clear(void *data)
{
	struct dp_display *display = data;
	struct dp_hw_ctrl *hw_ctrl = display->hw_modules.hw_ctrl;

	hw_ctrl->ops->clear_intr_state(&hw_ctrl->hw,
			DP_INTR_VIDEO_FIFO_OVERFLOW_0 | DP_INTR_VIDEO_FIFO_UNDERFLOW_0);

	atomic_set(&display->stream_overflow_flag, 0);
	atomic_set(&display->stream_underflow_flag, 0);
	atomic_set(&display->audio_overflow_flag, 0);

	return 0;
}

static int dp_display_video_irq_handler(void *data)
{
	struct dp_display *display = data;

	DP_WARN("dp video stream fifo overflow: %d, underflow: %d, audio overflow: %d\n",
			atomic_read(&display->stream_overflow_flag),
			atomic_read(&display->stream_underflow_flag),
			atomic_read(&display->audio_overflow_flag));

	return 0;
}

int dp_display_tmg_state_get(struct dp_display *display)
{
	struct dp_hw_tmg *hw_tmg = display->hw_modules.hw_tmg;

	/**
	 * if encoder is enable, update underflow_count and state to be the latest
	 * status, if encoder is disable, remain underflow_count and state unchanged
	 */
	if (display->status.is_connector_enabled)
		hw_tmg->ops->get_state(&hw_tmg->hw,
				&display->status.tmg_underflow_count,
				&display->status.tmg_state);

	return 0;
}

void dp_display_get_obuf_depth_info(struct dp_display *display,
		struct obuf_depth_info *depth_info)
{
	struct dp_display_timing *timing;

	if (!display || !depth_info) {
		DP_ERROR("invalid parameter, display:%p, depth_info:%p\n",
				display, depth_info);
		return;
	}

	timing = &display->panel->video_info.timing;

	depth_info->width = timing->h_active;
	depth_info->vtotal = timing->v_total;
	depth_info->fps = timing->fps_thousand / 1000;
	depth_info->bpp = timing->color_depth;
	depth_info->obufen = dpu_obuf_get_obufen(DPU_OBUF_CLIENT_DP);
}

bool dp_display_check_tmg_timing(struct dp_display_timing *timing, bool dsc_en)
{
	u32 h_blank;

	h_blank = dsc_en ? DIV_ROUND_UP(timing->h_active + timing->h_blank, 6) -
			DIV_ROUND_UP(timing->h_active, 6) : timing->h_blank;

	return h_blank >= DP_TMG_MIN_H_BLANK;
}

/* according link training result to calculate the max pixel clock allowed */
void dp_display_update_maxpclk(struct dp_display *display)
{
	struct dp_link_phy_status *phy_status = &display->ctrl->link->phy_status;
	bool dsc_en = display->panel->sink_caps.dsc_caps.dsc_supported &&
			display->module_ctrl.dsc_enable;
	bool fec_en = phy_status->fec_en;
	u32 link_rate_mbps = phy_status->link_rate_mbps;
	u32 lane_count = phy_status->lane_count;
	u32 color_depth = dsc_en ? 8 : 24;
	u64 maxpclk_khz;

	if (fec_en)
		maxpclk_khz = (64000 * (u64)lane_count * link_rate_mbps * (1000 - 24)) /
				(color_depth * 80 * 1000);
	else
		maxpclk_khz = (64000 * (u64)lane_count * link_rate_mbps) / (color_depth * 80);

	display->status.maxpclk_khz = maxpclk_khz;
}

void dp_display_prepare_link_param(struct dp_display *display)
{
	struct dp_link_caps_sink *link_caps = &display->panel->sink_caps.link_caps;
	struct dp_link_phy_param *dst_param = &display->ctrl->link->phy_param;

	/* device-tree pre-defined */
	dp_link_reset_to_default_param(display->ctrl->link);

	/* sink caps limitation */
	dst_param->fec_en &= link_caps->fec_supported;
	dst_param->ssc_en &= link_caps->ssc_supported;
	dst_param->enhance_frame_en &= link_caps->enhance_frame_supported;
	dst_param->eq_pattern = link_caps->tps4_supported ? TPS_TPS4 :
			(link_caps->tps3_supported ? TPS_TPS3 : TPS_TPS2);
	dst_param->init_lane_count = min_t(u8,
			dst_param->init_lane_count, link_caps->max_lane_count);
	dst_param->init_link_rate = min_t(u8,
			dst_param->init_link_rate, link_caps->max_link_rate);

	dst_param->init_lane_count = min_t(u8,
			dst_param->init_lane_count,
			display->status.combo_mode == DP_COMBO_MODE_DP4 ? 4 : 2);
}

void dp_display_prepare_video_info(struct dp_display *display,
		struct drm_display_mode *mode, struct dpu_dsc_config *dsc_cfg)
{
	struct dp_panel_video_info *video_info = &display->panel->video_info;
	struct dp_panel_dsc_info *dsc_info = &display->panel->dsc_info;
	struct dp_link_phy_status *phy_status = &display->ctrl->link->phy_status;

	/* get dsc info */
	dsc_info->dsc_config = dsc_cfg;
	dsc_info->dsc_en = dsc_cfg->valid;

	/* if vg or tmg self test is on, turn off dsc */
	if (display->module_ctrl.tmg_test_index != 0 ||
			display->module_ctrl.vg_test_index != 0) {
		dsc_info->dsc_config = NULL;
		dsc_info->dsc_en = false;
		DP_INFO("self test is on, turn off dsc automatically\n");
	}

	/* get link info */
	video_info->link_info.fec_en = phy_status->fec_en;
	video_info->link_info.lane_count = phy_status->lane_count;
	video_info->link_info.link_clk_khz = phy_status->link_clk_khz;
	video_info->link_info.link_rate_mbps = phy_status->link_rate_mbps;

	/* get timing */
	display->panel->funcs->set_timing(display->panel, mode);
}

void dp_display_clear_video_info(struct dp_display *display)
{
	display->panel->funcs->clear_video_info(display->panel);
}

#ifdef ASIC
static void dp_display_ipi_clock_off(struct dp_display *display)
{
	clk_disable_unprepare(display->ipi_clk);
}

static int dp_display_ipi_clock_on(struct dp_display *display,
		struct dp_display_timing *timing, bool dsc_en)
{
	struct dp_hw_hss1 *hw_hss1 = display->power->hw_hss1;
	u64 htotal, htotal_compressed;
	u64 base_rate, rate;
	u32 div_ratio;
	int ret;

	htotal = timing->h_active + timing->h_blank;
	htotal_compressed = DIV_ROUND_UP(htotal, 6);

	base_rate = (u64)timing->pixel_clock_khz * 1000 * 1000;
	if (dsc_en)
		base_rate = base_rate / htotal * htotal_compressed;
	base_rate = base_rate / 4000;  /* ipi interface is 4 pixel mode */

	/* adjust rate and rate_div to suit ipi clock limitation */
	div_ratio = 1;
	rate = base_rate;
	while (rate < DP_IPI_CLOCK_HZ_MIN) {
		div_ratio += 1;
		rate += base_rate;
	}

	DP_DEBUG("PLL rate: %llu, div_ratio: %u, ipi rate: %llu, pixel_clk:%llukhz\n",
			rate, div_ratio, base_rate, base_rate / 250);
	if (rate > DP_IPI_CLOCK_HZ_MAX) {
		DP_ERROR("unsupported pixel clock rate (%llu)\n", rate);
		return -EINVAL;
	}

	hw_hss1->ops->ipi_clock_gate_off(&hw_hss1->hw);

	ret = clk_set_rate(display->ipi_clk, rate);
	if (ret) {
		DP_ERROR("failed to set ipi clock rate\n");
		return -EBUSY;
	}

	ret = clk_prepare_enable(display->ipi_clk);
	if (ret) {
		DP_ERROR("failed to enable ipi clock\n");
		return -EBUSY;
	}

	hw_hss1->ops->ipi_clock_gate_on(&hw_hss1->hw);

	hw_hss1->ops->set_ipi_clock_div_ratio(&hw_hss1->hw, div_ratio);

	return 0;
}
#elif defined FPGA

static void dp_display_ipi_clock_off(struct dp_display *display)
{}

static int dp_display_ipi_clock_on(struct dp_display *display,
		struct dp_display_timing *timing, bool dsc_en)
{
	return 0;
}
#endif

void dp_display_line_buf_level_cfg(struct dp_display *display)
{
	struct dpu_flow_ctrl *flow_ctrl;
	struct obuf_depth_info depth_info;

	flow_ctrl = dpu_flow_ctrl_get(display->drm_dev);
	dp_display_get_obuf_depth_info(display, &depth_info);

	/* configure obuf level threshold */
	dpu_flow_line_buf_level_cfg(flow_ctrl, &display->hw_modules.hw_tmg->hw,
			FLOW_OBUF_LEVEL_DP, &depth_info);
}

void dp_display_video_stream_self_test(struct dp_display *display)
{
	struct dp_hw_sctrl *hw_sctrl = display->hw_modules.hw_sctrl;
	struct dp_hw_tmg *hw_tmg = display->hw_modules.hw_tmg;
	bool tmg_self_test_en = display->module_ctrl.tmg_test_index != 0;
	bool vg_self_test_en = display->module_ctrl.vg_test_index != 0;
	struct dp_hw_sctrl_vg_config vg_config = {
		.bpc = display->panel->video_info.bpc,
		.timing = &display->panel->video_info.timing,
	};

	hw_tmg->ops->enable_self_test(&hw_tmg->hw,
			display->module_ctrl.tmg_test_index, tmg_self_test_en);

	/* configure vg self test */
	hw_sctrl->ops->enable_self_test(&hw_sctrl->hw, &vg_config,
			display->module_ctrl.vg_test_index, vg_self_test_en);

	if (!tmg_self_test_en && !vg_self_test_en)
		DP_INFO("* DP self test is off *\n");
}

void dp_display_video_stream_on(struct dp_display *display)
{
	struct dp_hw_tmg *hw_tmg = display->hw_modules.hw_tmg;
	struct dp_panel *panel = display->panel;
	struct dp_ctrl *ctrl = display->ctrl;
	struct dp_panel_video_info *video_info = &panel->video_info;
	struct dp_panel_dsc_info *dsc_info = &panel->dsc_info;

	dpu_power_get_helper(BIT(DPU_PARTITION_0));

	/* set ipi clock frequency */
	if (dp_display_ipi_clock_on(display, &video_info->timing, dsc_info->dsc_en)) {
		DP_ERROR("failed to set ipi clock\n");
		return;
	}

	/* configure dptx video stream */
	panel->funcs->config_video(panel);

	/* configure tmg */
	dp_hw_tmg_obufen_config(&hw_tmg->hw, dpu_obuf_get_obufen(DPU_OBUF_CLIENT_DP));
	dp_display_line_buf_level_cfg(display);
	hw_tmg->ops->config(&hw_tmg->hw, &video_info->timing, video_info->bpc,
			dsc_info->dsc_en);

	hw_tmg->ops->enable(&hw_tmg->hw, true);
	ctrl->funcs->enable_video(ctrl, true);
	dp_display_video_irq_enable(display, true);

	/* update status */
	display->status.timing = &video_info->timing;
	display->status.is_dsc_enabled = dsc_info->dsc_en;

	DP_INFO("* DP video stream is on *\n");
}

void dp_display_send_black_frame(struct dp_display *display)
{
	struct dp_hw_tmg *hw_tmg = display->hw_modules.hw_tmg;

	dp_hw_tmg_send_black_frame(&hw_tmg->hw);

	DP_INFO("* DP self test (black frame) is on *\n");
}

void dp_display_video_stream_off(struct dp_display *display)
{
	struct dp_hw_tmg *hw_tmg = display->hw_modules.hw_tmg;
	struct dp_ctrl *ctrl = display->ctrl;
	bool obufen_updated;

	display->status.timing = NULL;
	display->status.is_dsc_enabled = false;
	display->status.is_hdcp_encryption_enabled = false;

	dp_display_video_irq_enable(display, false);
	ctrl->funcs->enable_video(ctrl, false);

	/**
	 * case 1: plug out event happened, obufen_updated will be true here
	 * case 2: screen-off frame committed, obufen_updated will be false here
	 */
	(void)dpu_obuf_check_obufen(DPU_OBUF_CLIENT_DP, &obufen_updated);
	dpu_obuf_update_obufen_cmt(DPU_OBUF_CLIENT_DP, obufen_updated);

	hw_tmg->ops->enable(&hw_tmg->hw, false);

	dp_display_ipi_clock_off(display);

	dpu_power_put_helper(BIT(DPU_PARTITION_0));

	DP_INFO("* DP video stream is off *\n");
}

static int dp_display_hw_modules_init(struct dp_display *display)
{
	struct dp_hw_modules *modules = &display->hw_modules;

	modules->hw_ctrl = dp_hw_ctrl_init(&display->parser->ctrl_cap);
	if (IS_ERR_OR_NULL(modules->hw_ctrl)) {
		DP_ERROR("failed to init dp hw ctrl\n");
		goto err_hw_ctrl;
	}

	modules->hw_msgbus = dp_hw_msgbus_init(&display->parser->msgbus_cap);
	if (IS_ERR_OR_NULL(modules->hw_msgbus)) {
		DP_ERROR("failed to init dp hw msgbus\n");
		goto err_hw_msgbus;
	}

	modules->hw_sctrl = dp_hw_sctrl_init(&display->parser->sctrl_cap);
	if (IS_ERR_OR_NULL(modules->hw_sctrl)) {
		DP_ERROR("failed to init dp hw sctrl\n");
		goto err_hw_sctrl;
	}

	modules->hw_tmg = dp_hw_tmg_init(&display->parser->tmg_cap);
	if (IS_ERR_OR_NULL(modules->hw_tmg)) {
		DP_ERROR("failed to init dp hw tmg\n");
		goto err_hw_tmg;
	}

	return 0;

err_hw_tmg:
	dp_hw_sctrl_deinit(modules->hw_sctrl);
err_hw_sctrl:
	dp_hw_msgbus_deinit(modules->hw_msgbus);
err_hw_msgbus:
	dp_hw_ctrl_deinit(modules->hw_ctrl);
err_hw_ctrl:
	return -ENOMEM;
}

static void dp_display_hw_modules_deinit(struct dp_display *display)
{
	struct dp_hw_modules *modules = &display->hw_modules;

	dp_hw_tmg_deinit(modules->hw_tmg);
	dp_hw_sctrl_deinit(modules->hw_sctrl);
	dp_hw_msgbus_deinit(modules->hw_msgbus);
	dp_hw_ctrl_deinit(modules->hw_ctrl);
}

static void dp_display_lp_delayed_work_handle(struct kthread_work *work)
{
	struct kthread_delayed_work *delayed_work = container_of(work, struct kthread_delayed_work, work);
	struct dp_lp_delayed_work *lp_delayed_work = to_dp_lp_delayed_work(delayed_work);
	struct dp_display *display;
	struct dp_power *power;

	if (lp_delayed_work->display == NULL) {
		DP_ERROR("display is NULL\n");
		return;
	}

	display = lp_delayed_work->display;
	power = display->power;

	if (display->status.is_connector_enabled) {
		DP_INFO("dp video stream is already on\n");
		return;
	}

	/* if device is in charging state, skip */

	if (display->audio.is_audio_playing) {
		DP_DEBUG("dp audio is playing\n");
		return;
	}

	if (display->power->is_lpm_enable) {
		DP_INFO("dp power is already in low power mode\n");
		return;
	}

	drm_dp_dpcd_writeb(&display->aux->base, DP_SET_POWER, DP_SET_POWER_D3);

	dp_display_video_stream_off(display);

	power->funcs->lpm_enable(power);

	DP_INFO("dp lp mode enter\n");
}

void dp_display_lp_exit(struct dp_display *display)
{
	struct dp_power *power = display->power;
	struct dp_ctrl *ctrl = display->ctrl;
	struct dp_panel_video_info *video_info = &display->panel->video_info;
	struct dp_link_phy_status *phy_status = &ctrl->link->phy_status;
	int ret = 0;

	/* power isn`t in low power mode */
	if (!power->is_lpm_enable)
		return;

	drm_dp_dpcd_writeb(&display->aux->base, DP_SET_POWER, DP_SET_POWER_D0);

	power->funcs->lpm_disable(power);

	/* if link is not ready, retraining */
	if (!dp_link_check_link_status(ctrl->link)) {
		ret = ctrl->funcs->start_link_training(ctrl);
		if (ret == 0) {
			video_info->link_info.fec_en = phy_status->fec_en;
			video_info->link_info.lane_count = phy_status->lane_count;
			video_info->link_info.link_clk_khz = phy_status->link_clk_khz;
			video_info->link_info.link_rate_mbps = phy_status->link_rate_mbps;
		}
	}

	dp_display_video_stream_on(display);
	DP_INFO("dp lp mode exit\n");
}

static int dp_display_work_init(struct dp_display *display)
{
	int ret;

	kthread_init_worker(&display->lp_worker);

	display->lp_worker_task = kthread_run(kthread_worker_fn,
			&display->lp_worker, "dp_display_lp_worker");
	if (IS_ERR_OR_NULL(display->lp_worker_task)) {
		DPU_ERROR("failed to create dp display lp thread\n");
		ret = PTR_ERR(display->lp_worker_task);
		return ret;
	}

	display->lp_delayed_work.display = display;
	kthread_init_delayed_work(&display->lp_delayed_work.base,
			dp_display_lp_delayed_work_handle);

	return 0;
}

static void dp_display_work_deinit(struct dp_display *display)
{
	kthread_flush_worker(&display->lp_worker);
	kthread_stop(display->lp_worker_task);
}

int dp_display_init(struct platform_device *pdev, struct dp_display *display)
{
	int ret;

	spin_lock_init(&display->lock);
	INIT_LIST_HEAD(&display->irq_list);

	ret = dp_parser_init(pdev, &display->parser);
	if (ret) {
		DP_ERROR("failed to init dp parser\n");
		goto err_parser;
	}

	ret = dp_display_hw_modules_init(display);
	if (ret) {
		DP_ERROR("failed to init dp hw modules\n");
		goto err_hw_modules;
	}

	ret = dp_display_intr_init(display);
	if (ret) {
		DP_ERROR("failed to register dp interrupt\n");
		goto err_intr;
	}

	ret = dp_aux_init(display, &display->aux);
	if (ret) {
		DP_ERROR("failed to register dp aux\n");
		goto err_aux;
	}

	ret = dp_hpd_init(display, &display->hpd);
	if (ret) {
		DP_ERROR("failed to register dp hpd\n");
		goto err_hpd;
	}

	ret = dp_power_init(display, &display->power);
	if (ret) {
		DP_ERROR("failed to init dp power\n");
		goto err_power;
	}

	ret = dp_panel_init(display, &display->panel);
	if (ret) {
		DP_ERROR("failed to init dp panel\n");
		goto err_panel;
	}

	ret = dp_ctrl_init(display, &display->ctrl);
	if (ret) {
		DP_ERROR("failed to init dp ctrl\n");
		goto err_ctrl;
	}

	ret = dp_display_work_init(display);
	if (ret) {
		DP_ERROR("failed to init dp work\n");
		goto err_work;
	}

#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
	ret = dp_hdcp_init(display, &display->hdcp);
	if (ret) {
		DP_ERROR("failed to init dp hdcp\n");
		goto err_hdcp;
	}
#endif

	display->module_ctrl.dsc_enable = false;

	display->video_intr_ops.data          = display;
	display->video_intr_ops.enable        = dp_display_video_irq_enable;
	display->video_intr_ops.is_triggered  = dp_display_video_irq_is_triggered;
	display->video_intr_ops.clear         = dp_display_video_irq_clear;
	display->video_intr_ops.handler       = dp_display_video_irq_handler;
	dp_display_irq_register(display, &display->video_intr_ops);

	return 0;

#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
err_hdcp:
#endif
err_work:
	dp_ctrl_deinit(display->ctrl);
err_ctrl:
	dp_panel_deinit(display->panel);
err_panel:
	dp_power_deinit(display->power);
err_power:
	dp_hpd_deinit(display->hpd);
err_hpd:
	dp_aux_deinit(display->aux);
err_aux:
	dp_display_intr_deinit(display);
err_intr:
	dp_display_hw_modules_deinit(display);
err_hw_modules:
	dp_parser_deinit(display->parser);
err_parser:
	return ret;
}

void dp_display_deinit(struct dp_display *display)
{
	dp_display_irq_unregister(display, &display->video_intr_ops);
#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
	dp_hdcp_deinit(display->hdcp);
#endif
	dp_display_work_deinit(display);
	dp_ctrl_deinit(display->ctrl);
	dp_panel_deinit(display->panel);
	dp_power_deinit(display->power);
	dp_hpd_deinit(display->hpd);
	dp_aux_deinit(display->aux);
	dp_display_intr_deinit(display);
	dp_display_hw_modules_deinit(display);
	dp_parser_deinit(display->parser);
}

static struct xr_dp_ops gdp_ops = {
	.manage_power = dp_manage_power,
	.hpd_evt_notifier = dp_handle_hpd_event,
};

static int dp_probe(struct platform_device *pdev)
{
	struct dp_display *dp_display;
	struct clk *ipi_clk = NULL;
	int ret;

	dp_display = kzalloc(sizeof(*dp_display), GFP_KERNEL);
	if (!dp_display)
		return -ENOMEM;

	dp_display->xr_usb = xr_usb_port_of_get(pdev->dev.of_node);
	if (!dp_display->xr_usb) {
		DP_INFO("cannot get xrusb, retry\n");
		ret = -EPROBE_DEFER;
		goto err_usbdp_get;
	}

	ret = xr_usbdp_ops_register(dp_display->xr_usb, &gdp_ops);
	if (ret) {
		DP_ERROR("register xrusb dp failed\n");
		goto err_usbdp_ops_register;
	}

	ret = dp_display_init(pdev, dp_display);
	if (ret) {
		DP_ERROR("failed to init dp display, ret %d\n", ret);
		goto err_disp_init;
	}

	ipi_clk = devm_clk_get(&pdev->dev, "clk_dp_pll");
	if (IS_ERR(ipi_clk)) {
		DP_ERROR("failed to get dp ipi clock\n");
		ret = PTR_ERR(ipi_clk);
		goto err_clk;
	}
	dp_display->ipi_clk = ipi_clk;

	disp_slots[dp_display->parser->index] = dp_display;
	/**
	 * platform_set_drvdata should be called before component_add.
	 * if dp driver is the last one of components which called component_add,
	 * component_add will call dp_component_ops.bind (dp_bind).
	 * dp_bind use platform_get_drvdata to get dp_display. if
	 * this line is set after dp_drm_init, dp_bind will get a NULL pointer.
	 */
	platform_set_drvdata(pdev, dp_display);

	ret = dp_drm_init(pdev);
	if (ret) {
		DP_DEBUG("failed to init dp drm, ret %d\n", ret);
		goto err_drm_init;
	}

	return 0;

err_drm_init:
	disp_slots[dp_display->parser->index] = NULL;
	platform_set_drvdata(pdev, NULL);
err_clk:
	dp_display_deinit(dp_display);
err_disp_init:
	xr_usbdp_ops_unregister(dp_display->xr_usb);
err_usbdp_ops_register:
	xr_usb_port_of_put(dp_display->xr_usb);
	dp_display->xr_usb = NULL;
err_usbdp_get:
	kfree(dp_display);
	return ret;
}

static int dp_remove(struct platform_device *pdev)
{
	struct dp_display *dp_display;

	dp_display = platform_get_drvdata(pdev);

	disp_slots[dp_display->parser->index] = NULL;

	dp_drm_deinit(pdev);

	dp_display_deinit(dp_display);

	xr_usbdp_ops_unregister(dp_display->xr_usb);

	xr_usb_port_of_put(dp_display->xr_usb);

	dp_display->xr_usb = NULL;

	platform_set_drvdata(pdev, NULL);

	kfree(dp_display);

	return 0;
}

static const struct of_device_id dp_of_match[] = {
	{.compatible = "xring,dp-display"},
	{},
};
MODULE_DEVICE_TABLE(of, dp_of_match);

static struct platform_driver dp_driver = {
	.probe = dp_probe,
	.remove = dp_remove,
	.driver = {
		.name = "xring-dp-display",
		.of_match_table = dp_of_match,
		.suppress_bind_attrs = true,
	},
};

int __init dp_drv_register(void)
{
	return platform_driver_register(&dp_driver);
}

void __exit dp_drv_unregister(void)
{
	platform_driver_unregister(&dp_driver);
}
