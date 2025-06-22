// SPDX-License-Identifier: GPL-2.0
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

#include <linux/types.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <drm/drm_file.h>
#include <soc/xring/doze.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h>
#include "dp_drm.h"
#include "dpu_log.h"
#include "dp_display.h"
#include "dp_hpd.h"
#include "dp_link.h"
#include "dpu_obuf.h"
#include "dpu_idle_helper.h"

#define HPD_EVENT_SIZE                                32
#define DP_DEVICE_SERVICE_IRQ_VECTOR                  0x201
#define DP_CP_IRQ                                     (1 << 2)
#define WAIT_SCREEN_OFF_TIMEOUT_US                    2000000
#define DP_UEVENT_REPORT_DELAY_MS                     10

static void dp_hpd_send_event(struct dp_display *display, bool is_hpd_high)
{
	struct drm_device *dev = display->drm_dev;
	struct dp_audio *audio = &display->audio;
	char event[HPD_EVENT_SIZE];
	char *envp[2];
	bool is_audio_exist;

	is_audio_exist = is_hpd_high &&
			display->panel->sink_caps.audio_blk.basic_audio_supported;

	/* notify HWC for dp plug event*/
	snprintf(event, HPD_EVENT_SIZE, "DP_HPD=%d, DP_AUDIO=%d", is_hpd_high, is_audio_exist);

	envp[0] = event;
	envp[1] = NULL;

	kobject_uevent_env(&dev->primary->kdev->kobj, KOBJ_CHANGE, envp);

	DP_INFO("send uevent: DP_HPD=%d, DP_AUDIO=%d\n", is_hpd_high, is_audio_exist);

	/* notify audio driver if sink supports audio */
	if (display->panel->sink_caps.audio_blk.basic_audio_supported
			&& audio->cb != NULL && audio->cb->audio_notify != NULL)
		audio->cb->audio_notify(audio->priv_data, is_hpd_high);
}

void dp_hpd_delayed_work_handle(struct kthread_work *work)
{
	struct kthread_delayed_work *delayed_work = container_of(work, struct kthread_delayed_work, work);
	struct dp_hpd_delayed_work *hpd_delayed_work = to_hpd_delayed_work(delayed_work);

	dp_hpd_send_event(hpd_delayed_work->display, hpd_delayed_work->is_hpd_high);
}

static int dp_handle_sink_request(struct dp_display *display)
{
	struct drm_dp_aux *aux = &display->aux->base;
	struct dp_link *link = display->ctrl->link;
	struct dp_hw_msgbus *hw_msgbus = display->hw_modules.hw_msgbus;
	struct dp_hw_sctrl *hw_sctrl = display->hw_modules.hw_sctrl;
	struct dp_hw_ctrl *hw_ctrl = display->hw_modules.hw_ctrl;
	struct dp_link_irq_vector *vec = &link->irq_vector;
	struct arm_smccc_res res = {0};
	int ret;

	/* check link status */
	if (!link->funcs->check_link_status(link))
		DP_WARN("link status is not ready\n");

	/* read and parse irq vector */
	ret = link->funcs->parser_sink_irq_vector(link);
	if (ret) {
		DP_WARN("failed to parser sink irq vector\n");
		return ret;
	}

	if (vec->cp_irq_flag) {
		ret = drm_dp_dpcd_writeb(aux, DP_DEVICE_SERVICE_IRQ_VECTOR,
				DP_CP_IRQ);
		if (ret != 1) {
			DP_ERROR("failed to read IRQ_VECTOR, ret %d\n", ret);
			return ret;
		}

		arm_smccc_smc(FID_BL31_DPU_DP_CP_IRQ_SET,
				0, 0, 0, 0, 0, 0, 0, &res);
		if (res.a0) {
			DP_ERROR("failed to set cp_irq\n");
			return -EBUSY;
		}
	}

	if (vec->auto_test_flag) {
		dp_hpd_send_event(display, false);

		/**
		 * in current implementation, we just set test lane count and link rate,
		 * instead of starting a new link training.
		 */
		if (vec->auto_test.test_training_flag) {
			u8 lane_count = vec->auto_test.test_lane_count;
			u8 link_rate = vec->auto_test.test_link_rate;

			hw_ctrl->ops->set_per_lane_power_mode(&hw_ctrl->hw, &hw_sctrl->hw,
					PHY_POWERDOWN_STATE_POWER_ON);
			ret = hw_ctrl->ops->set_lane_count_link_rate(&hw_ctrl->hw, &hw_sctrl->hw,
					&hw_msgbus->hw, lane_count, link_rate);
			if (ret) {
				DP_ERROR("failed to set lane count, link rate\n");
				return ret;
			}

			display->status.is_training_done = false;
			link->phy_status.lane_count = lane_count;
			link->phy_status.link_rate = link_rate;

			hw_ctrl->ops->enable_video_transfer(&hw_ctrl->hw, false);
			hw_ctrl->ops->set_pattern(&hw_ctrl->hw, TPS_IDLE);

			drm_dp_dpcd_writeb(&link->aux->base, DP_TEST_RESPONSE, DP_TEST_ACK);

			DP_INFO("irq reset lane count to be: %hhu, link rate to be: %s\n",
					lane_count, to_link_rate_str(link_rate));
		}

		if (vec->auto_test.test_pattern_flag) {
			dp_link_set_vswing_preemp(link);

			if (vec->auto_test.test_pattern == TPS_CUSTOMPAT)
				hw_ctrl->ops->set_80b_custom_pattern(&hw_ctrl->hw, DP_CUSTOM_PATTERN_CUS,
						vec->auto_test.test_cus_pattern_80b);

			hw_ctrl->ops->set_pattern(&hw_ctrl->hw, vec->auto_test.test_pattern);

			drm_dp_dpcd_writeb(&link->aux->base, DP_TEST_RESPONSE, DP_TEST_ACK);

			DP_INFO("irq req set vswing level: %d %d %d %d\n",
					link->phy_status.swing_level[0], link->phy_status.swing_level[1],
					link->phy_status.swing_level[2], link->phy_status.swing_level[3]);
			DP_INFO("irq req set preemp level: %d %d %d %d\n",
					link->phy_status.preemp_level[0], link->phy_status.preemp_level[1],
					link->phy_status.preemp_level[2], link->phy_status.preemp_level[3]);
			DP_INFO("irq req send pattern: %hhu\n", vec->auto_test.test_pattern);
		}
	}

	return 0;
}

static void dp_hpd_hardware_prepare(struct dp_hpd *hpd)
{
	struct dp_display *display = hpd->display;
	struct dp_hw_sctrl *hw_sctrl = display->hw_modules.hw_sctrl;
	struct dp_hw_ctrl *hw_ctrl = display->hw_modules.hw_ctrl;
	int ret;

#ifdef FPGA
	ret = display->power->funcs->on(display->power);
	if (ret) {
		DP_ERROR("failed to power on dptx modules\n");
		return;
	}
#endif

	ret = hw_ctrl->ops->reset_module(&hw_ctrl->hw, &hw_sctrl->hw,
			DPTX_MODULE_PHY | DPTX_MODULE_AUX | DPTX_MODULE_CONTROLLER);
	if (ret) {
		DP_ERROR("failed to reset dptx modules\n");
		return;
	}

	(void)hw_sctrl->ops->pll_request(&hw_sctrl->hw, DP_MAXPCLKREQ_PLL_ON_REF_CLK_ON);

	hw_sctrl->ops->aux_enable(&hw_sctrl->hw, true);

	hw_ctrl->ops->set_per_lane_power_mode(&hw_ctrl->hw, &hw_sctrl->hw,
		PHY_POWERDOWN_STATE_POWER_ON);

	hw_sctrl->ops->hpd_trigger(&hw_sctrl->hw, 1);
	hw_ctrl->ops->default_config(&hw_ctrl->hw);

	/* aux and video irq */
	dp_display_irq_enable(display, true);

	/* according synop spec. delay for AUX hardware startup */
	DPU_MDELAY(100);
}

static void dp_hpd_hardware_release(struct dp_hpd *hpd)
{
	struct dp_display *display = hpd->display;
	struct dp_hw_sctrl *hw_sctrl = display->hw_modules.hw_sctrl;
	struct dp_hw_ctrl *hw_ctrl = display->hw_modules.hw_ctrl;

	dp_display_irq_enable(display, false);

	hw_sctrl->ops->hpd_trigger(&hw_sctrl->hw, 0);
	hw_ctrl->ops->set_per_lane_power_mode(&hw_ctrl->hw, &hw_sctrl->hw,
			PHY_POWERDOWN_STATE_POWER_DOWN);

	hw_sctrl->ops->aux_enable(&hw_sctrl->hw, false);

	(void)hw_sctrl->ops->pll_request(&hw_sctrl->hw, DP_MAXPCLKREQ_PLL_OFF_REF_CLK_OFF);

#ifdef FPGA
	display->power->funcs->off(display->power);
#endif
}

static int dp_hpd_link_on(struct dp_hpd *hpd)
{
	struct dp_display *display = hpd->display;
	struct dp_panel *panel = display->panel;
	struct dp_ctrl *ctrl = display->ctrl;
	int ret;

	/* read sink capabilites and start link training */
	ret = panel->funcs->get_caps(panel, display->connector);
	if (ret) {
		DP_ERROR("failed to get dpcd caps, ret %d\n", ret);
		return ret;
	}

	/**
	 * If any lttprs exist on link, set to transparent mode explicitly.
	 * Although DP 1.4spec suggests lttprs to set transparent mode by default,
	 * some devices may behave wrong mode if the mode isn`t set explicitly by TX.
	 */
	if (drm_dp_lttpr_count(panel->lttpr_common_dpcd)) {
		if (drm_dp_dpcd_writeb(&panel->aux->base, DP_PHY_REPEATER_MODE,
				DP_PHY_REPEATER_MODE_TRANSPARENT) < 1)
			DP_WARN("failed to set phy repeater mode to transparent mode\n");
	}

	dp_display_prepare_link_param(display);

	return ctrl->funcs->start_link_training(ctrl);
}

static void dp_hpd_handle_plug_in(struct dp_hpd *hpd)
{
	struct dp_display *display = hpd->display;
	int ret;

	if (display->status.hpd == DP_HPD_EVENT_PLUG_IN) {
		DP_INFO("DP is already plugged in\n");
		return;
	}
	display->status.hpd = DP_HPD_EVENT_PLUG_IN;
	DP_INFO("DisplayPort is plugged in\n");

	/* dpu operation */
	dpu_idle_enable_ctrl(false);
	sys_state_doz2nor_vote(VOTER_DOZAP_DP);

	/* source hardware prepare */
	dp_hpd_hardware_prepare(hpd);

	display->power->is_lpm_enable = false;

	hpd->first_on = true;
	hpd->last_off = false;

	/* setup link connection */
	ret = dp_hpd_link_on(hpd);
	if (ret == 0) {
		display->status.is_training_done = true;
		dpu_obuf_scene_trigger_dp_plug(DPU_OBUF_SCENE_DP_PLUG_IN);
		dp_display_update_maxpclk(display);

		/* send event to userspace */
		hpd->delayed_work.display = display;
		hpd->delayed_work.is_hpd_high = true;
		kthread_queue_delayed_work(&hpd->worker, &hpd->delayed_work.base,
				msecs_to_jiffies(DP_UEVENT_REPORT_DELAY_MS));
	} else {
		display->status.is_training_done = false;
	}
}

static inline void dp_hpd_release_obuf(void)
{
	bool obufen_updated;

	(void)dpu_obuf_check_obufen(DPU_OBUF_CLIENT_DP, &obufen_updated);
	dpu_obuf_update_obufen_cmt(DPU_OBUF_CLIENT_DP, obufen_updated);
	dpu_obuf_update_obufen_done(DPU_OBUF_CLIENT_DP);
}

/**
 * wait screen-off frame commit done. Otherwise, the next plug-in and
 * link training may be affected by register configuration in screen-off commit.
 *
 * there are 3 conditions that no screen-off frame.
 * case 1: user doesn`t enable dp video stream
 *
 * case 2: DP is plugged out during screen-off state. In such case, DP screen-off
 * frame has been committed with primary screen off.
 *     case 2.1: DP is still displaying cause we keep send black frame before
 *     screen is off for 10s. Here we need to release DP recourse manually.
 *     case 2.2: DP video stream is really off after screen is off for 10s. Here
 *     we just need to release obuf. ipi clk and video stream is already off.
 *     case 2.3: DP enter to lp mode during screen-off state, but trained failed
 *     when resume from lp mode in encoder_enable().
 *
 * case 3: System is stuck. In such case, we release DP resource here in advance
 * to finish plug-out operation. The later screen-off commit will not release again.
 */
static void dp_hpd_wait_for_screen_off(struct dp_hpd *hpd)
{
	struct dp_display *display = hpd->display;
	int ret;

	/**
	 * first_on is not consumed means first frame is not committed,
	 * no video is playing, so there`s no need to wait screen-off frame commit.
	 * case 1 here.
	 */
	if (hpd->first_on)
		return;

	/**
	 * first_on is false and is_connector_enable is false means first frame is
	 * already committed and screen-off frame is also committed.
	 * case 2 here.
	 */
	if (!display->status.is_connector_enabled) {
		kthread_cancel_delayed_work_sync(&display->lp_delayed_work.base);
		/* case 2.2 and 2.3 */
		if (display->power->is_lpm_enable)
			return;

		/* case 2.1 */
		goto release;
	}

	ret = wait_for_completion_timeout(&hpd->off_completed,
			usecs_to_jiffies(WAIT_SCREEN_OFF_TIMEOUT_US));
	/* wait screen-off frame timeout, case 3 here. */
	if (!ret) {
		DP_INFO("wait dp screen-off timeout\n");
		goto release;
	}

	return;

release:
	mutex_lock(&hpd->lock);
	if (hpd->last_off) {
		/* consume last_off, to prevent repeated operation in screen-off commit in case 3 */
		hpd->last_off = false;

		dp_display_video_stream_off(display);
		dp_display_clear_video_info(display);
	}
	mutex_unlock(&hpd->lock);
}

static void dp_hpd_handle_plug_out(struct dp_hpd *hpd)
{
	struct dp_display *display = hpd->display;

	if (display->status.hpd == DP_HPD_EVENT_PLUG_OUT) {
		DP_INFO("DP is already plugged out\n");
		return;
	}
	DP_INFO("DisplayPort is plugged out\n");
	display->status.hpd = DP_HPD_EVENT_PLUG_OUT;

#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
	/* exit hdcp event */
	dp_hdcp_monitor_stop(display->hdcp);
#endif

	/* if hpd=1 uevent is not reported yet, cancel it */
	kthread_cancel_delayed_work_sync(&hpd->delayed_work.base);

	/* clear software state */
	display->status.is_training_done = false; /* should before send uevent */
	dpu_obuf_scene_trigger_dp_plug(DPU_OBUF_SCENE_DP_PLUG_OUT); /* should before send uevent */
	hpd->last_off = true;
	/* send event to userspace */
	reinit_completion(&hpd->off_completed);
	dp_hpd_send_event(display, false);
	dp_hpd_wait_for_screen_off(hpd);
	dp_hpd_release_obuf();

	/* hardware config */
	dp_hpd_hardware_release(hpd);

	/* dpu operation */
	dpu_idle_enable_ctrl(true);
	sys_state_doz2nor_unvote(VOTER_DOZAP_DP);
}

static void dp_hpd_handle_irq(struct dp_hpd *hpd)
{
	struct dp_display *display = hpd->display;
	int ret;

	if (display->status.hpd == DP_HPD_EVENT_PLUG_OUT) {
		DP_INFO("DisplayPort is not connected, irq is invalid\n");
		return;
	}
	DP_INFO("DisplayPort hpd irq is triggered\n");

	ret = dp_handle_sink_request(display);
	if (ret)
		DP_ERROR("Unable to handle sink request %d\n", ret);
}

void dp_hpd_handle_event(struct dp_hpd *hpd, enum dp_hpd_event hpd_event)
{
	switch (hpd_event) {
	case DP_HPD_EVENT_PLUG_IN:
		dp_hpd_handle_plug_in(hpd);
		break;
	case DP_HPD_EVENT_PLUG_OUT:
		dp_hpd_handle_plug_out(hpd);
		break;
	case DP_HPD_EVENT_IRQ:
		dp_hpd_handle_irq(hpd);
		break;
	default:
		DP_ERROR("unknown DP_HPD_EVENT_TYPE %d\n", hpd_event);
	}
}

int dp_hpd_init(struct dp_display *display, struct dp_hpd **hpd)
{
	struct dp_hpd *dp_hpd;
	int ret = 0;

	dp_hpd = kzalloc(sizeof(*dp_hpd), GFP_KERNEL);
	if (!dp_hpd)
		return -ENOMEM;

	init_completion(&dp_hpd->off_completed);
	mutex_init(&dp_hpd->lock);

	kthread_init_worker(&dp_hpd->worker);
	dp_hpd->worker_task = kthread_run(kthread_worker_fn, &dp_hpd->worker, "dp_hpd_uevent_worker");
	if (IS_ERR_OR_NULL(dp_hpd->worker_task)) {
		DP_ERROR("failed to create dp hpd uevent thread\n");
		ret = PTR_ERR(dp_hpd->worker_task);
		goto err_work;
	}

	kthread_init_delayed_work(&dp_hpd->delayed_work.base, dp_hpd_delayed_work_handle);

	dp_hpd->display = display;
	*hpd = dp_hpd;

	return 0;

err_work:
	kfree(dp_hpd);
	return ret;
}

void dp_hpd_deinit(struct dp_hpd *hpd)
{
	kthread_flush_worker(&hpd->worker);
	kthread_stop(hpd->worker_task);
	kfree(hpd);
}
