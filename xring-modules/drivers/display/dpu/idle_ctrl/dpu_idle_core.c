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

#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/input.h>
#include <drm/drm_file.h>
#include <soc/xring/xr_timestamp.h>

#include "dpu_idle_helper.h"
#include "dpu_idle_cmd.h"
#include "dpu_idle_utils.h"
#include "dpu_power_mgr.h"
#include "dpu_log.h"
#include "dpu_kms.h"
#include "dpu_crtc.h"

#define ALLOW_MIN_ENTRY_TIME_MS 40

/* If wants to print all duration, set g_idle_duration_ms to zero */
uint32_t g_idle_duration_ms = 50;
struct dpu_idle_ctx g_idle_ctx;

struct dpu_idle_ctx *dpu_get_idle_ctx(void)
{
	return &g_idle_ctx;
}

bool is_idle_enabled(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if ((idle_ctx->pinfo->lowpower_ctrl & DPU_LP_IDLE_CTRL_ENABLE) &&
			is_dpu_lp_enabled(DPU_LP_IDLE_CTRL_ENABLE))
		return true;
	return false;
}

bool is_dpu_power_safe_with_dpu_idle(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();
	bool status;

	if (!is_idle_enabled())
		return is_dpu_powered_on();

	mutex_lock(&idle_ctx->idle_lock);
	status = idle_ctx->active ? is_dpu_powered_on() : true;
	mutex_unlock(&idle_ctx->idle_lock);

	return status;
}

static void dpu_idle_wait_te_irq(struct dpu_idle_ctx *idle_ctx)
{
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;
	u32 wait_timeout;
	int ret;

	if (!idle_ctx->pinfo->is_dual_port)
		return;

	dsi_connector = to_dsi_connector(idle_ctx->conn);
	display = dsi_connector->display;
	if (!display) {
		DSI_ERROR("dsi_display is null\n");
		return;
	}

	display_frame_timeout_get(display, &wait_timeout);

	ret = dsi_display_te_check_timeout(display, wait_timeout);
	if (ret)
		DPU_WARN("wait TE timeout\n");
}

void dpu_idle_ulps_exit(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return;
	if (!(idle_ctx->idle_policy & IDLE_MIPI_ULPS))
		return;

	mutex_lock(&idle_ctx->idle_lock);
	if ((idle_ctx->active == false) && (idle_ctx->ulps_status == true)) {
		dsi_connector_ulps_exit(idle_ctx->conn);
		dpu_idle_wait_te_irq(idle_ctx);
		idle_ctx->ulps_status = false;
		IDLE_DEBUG("exit ulps\n");
	}
	mutex_unlock(&idle_ctx->idle_lock);
}

void dpu_idle_ulps_enter(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return;
	if (!(idle_ctx->idle_policy & IDLE_MIPI_ULPS))
		return;

	mutex_lock(&idle_ctx->idle_lock);
	if ((idle_ctx->active == false) && (idle_ctx->ulps_status == false)) {
		dsi_connector_ulps_enter(idle_ctx->conn);
		idle_ctx->ulps_status = true;
		IDLE_DEBUG("enter ulps\n");
	}
	mutex_unlock(&idle_ctx->idle_lock);
}

static int hw_dvfs_ctrl(struct dpu_idle_ctx *idle_ctx, bool enable)
{
	struct dpu_core_perf *core_perf;

	core_perf = idle_ctx->dpu_kms->core_perf;
	if (enable)
		dpu_hw_dvfs_enable(&core_perf->hw_dvfs_mgr);
	else
		dpu_hw_dvfs_disable(&core_perf->hw_dvfs_mgr);

	return 0;
}

void dpu_idle_vote_stop(enum idle_vote_ch vote_ch)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();
	ktime_t start_time;

	if (vote_ch == PRESENT_CH) {
		/* Notify idle-thread to exit immediately, cannot be contained by idle_lock */
		atomic_inc(&idle_ctx->need_active);
	}

	INIT_START_TIMING(start_time);
	mutex_lock(&idle_ctx->idle_lock);
	idle_ctx->vote_count++;
	if (!idle_ctx->active) {
		idle_ctx->ops.exit(idle_ctx);
		idle_ctx->active = true;
		idle_ctx->trace.exit_time = xr_timestamp_gettime();
		DPU_DEBUG("idle exit success\n");
	}
	mutex_unlock(&idle_ctx->idle_lock);

	IDLE_DEBUG("vote stop succ. vote_count:%d active:%d expire_count:%d\n",
			idle_ctx->vote_count, idle_ctx->active, idle_ctx->expire_count);
}

static void dpu_idle_restart(struct dpu_idle_ctx *idle_ctx)
{
	unsigned long flag;

	spin_lock_irqsave(&idle_ctx->spin_lock, flag);

	if (idle_ctx->active)
		idle_ctx->expire_count = IDLE_EXPIRE_FRAME_NUM;
	spin_unlock_irqrestore(&idle_ctx->spin_lock, flag);

	IDLE_DEBUG("vote_count:%d active:%d expire_count:%d\n",
			idle_ctx->vote_count, idle_ctx->active, idle_ctx->expire_count);
}

void dpu_idle_vote_restart(enum idle_vote_ch vote_ch)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();
	ktime_t start_time;

	if (idle_ctx->vote_count == 0)
		return;

	INIT_START_TIMING(start_time);
	mutex_lock(&idle_ctx->idle_lock);
	if (idle_ctx->need_restore_clk && vote_ch == PRESENT_CH) {
		dpu_core_perf_clk_deactive();
		hw_dvfs_ctrl(idle_ctx, true);
		idle_ctx->need_restore_clk = false;
		IDLE_DEBUG("deactive clk\n");
	}

	idle_ctx->vote_count--;
	if (idle_ctx->vote_count == 0) {
		if (dsi_connector_is_cmd_mode(idle_ctx->conn) || (vote_ch == PRESENT_CH))
			dpu_idle_restart(idle_ctx);
	}
	mutex_unlock(&idle_ctx->idle_lock);

	if (vote_ch == PRESENT_CH) {
		idle_ctx->restore_mode = NO_NEED_RESTORE;
		/* restore the state of need_active for new frame */
		atomic_set(&idle_ctx->need_active, 0);
		idle_ctx->trace.present_time = xr_timestamp_gettime();
	}
	IDLE_DEBUG("vote restart succ. vote count: %d\n", idle_ctx->vote_count);
}

/* The hw state will might been changed after enter idle state.
 * So should do some recovery in next new frame.
 */
static void update_restore_mode(struct dpu_idle_ctx *idle_ctx)
{
	/* Notify display effect thread to configure completely in first frame after idle exit */
	if ((idle_ctx->idle_policy & IDLE_P0_POWER_OFF) ||
		(idle_ctx->idle_policy & IDLE_P1_POWER_OFF) ||
		(idle_ctx->idle_policy & IDLE_P2_POWER_OFF))
		idle_ctx->restore_mode |= RESTORE_REGISITER;

	/* Everytime tmg enable need to notify dpu drv to config sw_start in next frame */
	if ((idle_ctx->idle_policy & IDLE_P0_POWER_OFF) ||
		(idle_ctx->idle_policy & IDLE_DSI_CLK_OFF))
		idle_ctx->restore_mode |= RESTART_DPU_DISPLAY;

	if (idle_ctx->restore_mode)
		IDLE_DEBUG("set restore mode:0x%x\n", idle_ctx->restore_mode);
}

static bool check_underflow_occur(struct dpu_idle_ctx *idle_ctx)
{
	struct drm_crtc_state *crtc_state;
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_crtc *dpu_crtc;

	if (!idle_ctx->crtc)
		return true;

	crtc_state = idle_ctx->crtc->state;
	if (!crtc_state) {
		IDLE_ERROR("crtc_state is null\n");
		return true;
	}
	dpu_crtc = to_dpu_crtc(idle_ctx->crtc);
	dpu_crtc_state = to_dpu_crtc_state(crtc_state);

	if ((dpu_crtc->underflow_frame_no > 0) &&
			(dpu_crtc_state->frame_no - dpu_crtc->underflow_frame_no < 10)) {
		IDLE_INFO("underflow just occur, return. cur frm:%llu underflow frm:%llu\n",
				dpu_crtc_state->frame_no, dpu_crtc->underflow_frame_no);
		return true;
	}
	IDLE_DEBUG("frame:%llu last underflow frm:%llu\n",
			dpu_crtc_state->frame_no, dpu_crtc->underflow_frame_no);
	return false;
}

static bool check_invalid_entry(struct dpu_idle_ctx *idle_ctx)
{
	u64 timestamp = xr_timestamp_gettime();
	u32 diff_ms;

	diff_ms = (timestamp - idle_ctx->trace.present_time) / 1000000;
	if (diff_ms < ALLOW_MIN_ENTRY_TIME_MS) {
		DPU_INFO("idle enter is so fast,return");
		return true;
	}

	if (check_underflow_occur(idle_ctx))
		return true;

	return false;
}

static void dpu_idle_work_handler(struct work_struct *work)
{
	struct dpu_idle_ctx *idle_ctx;
	ktime_t start_time;
	int ret;

	if (!work) {
		IDLE_ERROR("work is null\n");
		return;
	}

	idle_ctx = container_of(work, struct dpu_idle_ctx, idle_work);
	if (!idle_ctx) {
		IDLE_ERROR("idle_ctx is null\n");
		return;
	}

	if (!is_idle_policy_valid(idle_ctx))
		return;

	/* Avoid idle workqueue is scheduled after dpu power off */
	if (!is_dpu_powered_on()) {
		IDLE_INFO("DPU is power down\n");
		return;
	}

	if (check_invalid_entry(idle_ctx))
		return;

	mutex_lock(&idle_ctx->idle_lock);
	if (!idle_ctx->active) {
		IDLE_INFO("idle has already enter\n");
		goto exit;
	}

	if ((idle_ctx->vote_count > 0) || (atomic_read(&idle_ctx->need_active) > 0)) {
		IDLE_DEBUG("new frame come, need stop idle process\n");
		goto exit;
	}

	INIT_START_TIMING(start_time);

	if (!idle_ctx->need_restore_clk) {
		dpu_core_perf_clk_active();
		hw_dvfs_ctrl(idle_ctx, false);
		idle_ctx->need_restore_clk = true;
		IDLE_DEBUG("active clk\n");
	}

	ret = idle_ctx->ops.prepare(idle_ctx);
	if (ret == 0) {
		IDLE_DEBUG("idle prepare success\n");
		idle_ctx->ops.enter(idle_ctx);
		idle_ctx->dpu_kms->funcs->handle_intr_events(idle_ctx->dpu_kms, idle_ctx->crtc, INTR_SIMULATE_IDLE_ENTER);
		idle_ctx->active = false;
		idle_ctx->trace.enter_time = xr_timestamp_gettime();
		update_restore_mode(idle_ctx);
		DPU_DEBUG("idle enter success\n");
	} else {
		IDLE_INFO("idle prepare failed\n");
		goto exit;
	}

	IDLE_DEBUG("vote_count:%d active:%d expire_count:%d\n",
			idle_ctx->vote_count, idle_ctx->active, idle_ctx->expire_count);

exit:
	mutex_unlock(&idle_ctx->idle_lock);
}

void dpu_idle_task_commit(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();
	unsigned long flags;

	if (idle_ctx->vote_count > 0)
		return;

	spin_lock_irqsave(&idle_ctx->spin_lock, flags);
	if (idle_ctx->expire_count > 0) {
		idle_ctx->expire_count--;
		if (idle_ctx->expire_count == 0)
			queue_work(idle_ctx->idle_ctrl_wq, &idle_ctx->idle_work);
	}
	spin_unlock_irqrestore(&idle_ctx->spin_lock, flags);
}

void dpu_idle_set_idle_policy(u32 idle_policy)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!idle_ctx->ops.check_idle_policy(idle_policy)) {
		IDLE_INFO("idle_policy:0x%x is invalid\n", idle_policy);
		return;
	}
	if (idle_ctx->idle_policy == idle_policy)
		return;

	/* Exit idle firstly before setting new idle_policy */
	dpu_idle_vote_stop(CTRL_CH);
	idle_ctx->idle_policy = idle_policy;
	IDLE_INFO("idle_policy = 0x%x\n", idle_policy);
	dpu_idle_vote_restart(CTRL_CH);
}

static void dpu_idle_ops_register(struct dpu_idle_ctx *idle_ctx)
{
	idle_ctx->ops.init = dpu_cmd_idle_init;
	idle_ctx->ops.check_idle_policy = dpu_cmd_idle_check_idle_policy;
	idle_ctx->ops.get_basic_policy = dpu_cmd_idle_get_basic_policy;
	idle_ctx->ops.prepare = dpu_cmd_idle_prepare;
	idle_ctx->ops.enter = dpu_cmd_idle_enter;
	idle_ctx->ops.exit = dpu_cmd_idle_exit;
	idle_ctx->ops.deinit = dpu_cmd_idle_deinit;
}

static void dpu_idle_input_handler_event(struct input_handle *handle,
		unsigned int type, unsigned int code, int value)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if ((idle_ctx->vote_count > 0) || idle_ctx->active)
		return;

	queue_work(idle_ctx->early_up_wq, &idle_ctx->early_up_work);
}

static void dpu_idle_input_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static int dpu_idle_input_connect(struct input_handler *handler,
		struct input_dev *dev, const struct input_device_id *id)
{
	struct input_handle *handle;
	int ret = 0;

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = handler->name;
	ret = input_register_handle(handle);
	if (ret) {
		DPU_ERROR("failed to register input handle\n");
		goto error;
	}
	ret = input_open_device(handle);
	if (ret) {
		DPU_ERROR("failed to open input device\n");
		goto error_unregister;
	}
	return 0;
error_unregister:
	input_unregister_handle(handle);
error:
	kfree(handle);
	return ret;
}

void dpu_idle_input_handler_register(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();
	int ret;

	if (idle_ctx->input_handler && !idle_ctx->input_handler->private) {
		idle_ctx->input_handler->private = idle_ctx;

		ret = input_register_handler(idle_ctx->input_handler);
		if (ret) {
			DPU_ERROR("input_handler_register failed, ret= %d\n", ret);
			kfree(idle_ctx->input_handler);
		}
	}
}

void dpu_idle_input_handler_unregister(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (idle_ctx->input_handler && idle_ctx->input_handler->private) {
		input_unregister_handler(idle_ctx->input_handler);
		idle_ctx->input_handler->private = NULL;
	}
}

static const struct input_device_id g_dpu_idle_input_ids[] = {
	/* multi-touch touchscreen */
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT |
			INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT_MASK(EV_ABS) },
		.absbit = { [BIT_WORD(ABS_MT_POSITION_X)] =
			BIT_MASK(ABS_MT_POSITION_X) |
			BIT_MASK(ABS_MT_POSITION_Y)
		},
	},
	{ },
};

static int dpu_idle_input_handler_init(struct dpu_idle_ctx *idle_ctx)
{
	struct input_handler *input_handler = NULL;

	input_handler = kzalloc(sizeof(struct input_handler), GFP_KERNEL);
	if (!input_handler) {
		DPU_ERROR("input_handler alloc failed\n");
		return -ENOMEM;
	}
	input_handler->event = dpu_idle_input_handler_event;
	input_handler->connect = dpu_idle_input_connect;
	input_handler->disconnect = dpu_idle_input_disconnect;
	input_handler->name = "dpu_idle";
	input_handler->id_table = g_dpu_idle_input_ids;

	idle_ctx->input_handler = input_handler;

	return 0;
}

static void dpu_idle_early_up_work(struct work_struct *work)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	mutex_lock(&idle_ctx->idle_lock);
	if (!idle_ctx->active) {
		idle_ctx->ops.exit(idle_ctx);
		idle_ctx->active = true;
		idle_ctx->trace.exit_time = xr_timestamp_gettime();
		DPU_DEBUG("idle early exit success\n");
	}
	mutex_unlock(&idle_ctx->idle_lock);
}

int dpu_idle_ctx_init(struct dpu_kms *dpu_kms)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();
	int ret;

	mutex_init(&idle_ctx->idle_lock);
	spin_lock_init(&idle_ctx->spin_lock);
	INIT_WORK(&idle_ctx->idle_work, dpu_idle_work_handler);
	INIT_WORK(&idle_ctx->early_up_work, dpu_idle_early_up_work);

	idle_ctx->dpu_kms = dpu_kms;
	idle_ctx->drm_dev = dpu_kms->drm_dev;
	idle_ctx->res_mgr_ctx = dpu_kms->res_mgr_ctx;
	idle_ctx->intr_ctx = dpu_kms->virt_ctrl.intr_ctx;
	idle_ctx->power_mgr = dpu_kms->power_mgr;
	idle_ctx->mem_pool = dpu_kms->mem_pool;

	idle_ctx->conn = dsi_primary_connector_get(idle_ctx->drm_dev);
	if (IS_ERR_OR_NULL(idle_ctx->conn)) {
		IDLE_ERROR("get primary connector failed\n");
		ret = -EINVAL;
		goto exit;
	}
	idle_ctx->pinfo = dsi_display_info_get(idle_ctx->conn);
	if (IS_ERR_OR_NULL(idle_ctx->pinfo)) {
		IDLE_ERROR("get primary panel info failed\n");
		ret = -EINVAL;
		goto exit;
	}
	idle_ctx->idle_ctrl_wq = alloc_workqueue("idle_ctrl_workqueue", WQ_HIGHPRI, 1);
	if (!idle_ctx->idle_ctrl_wq) {
		IDLE_ERROR("alloc idle workqueue failed\n");
		ret = -ENOMEM;
		goto exit;
	}

	idle_ctx->early_up_wq = alloc_workqueue("early_up_wq", WQ_HIGHPRI, 1);
	if (!idle_ctx->early_up_wq) {
		IDLE_ERROR("alloc early_up_wq failed\n");
		ret = -ENOMEM;
		goto exit;
	}
	ret = dpu_idle_input_handler_init(idle_ctx);
	if (ret) {
		DPU_ERROR("input handler failed\n");
		goto exit;
	}
	dpu_idle_ops_register(idle_ctx);

	idle_ctx->idle_policy = idle_ctx->pinfo->idle_policy;
	idle_ctx->active = true;
	idle_ctx->expire_count = 0; // will start counting after the first frame
	idle_ctx->restore_mode = NO_NEED_RESTORE;
	idle_ctx->vote_count = 0;

	/* Last step : call sub idle_module init */
	ret = idle_ctx->ops.init(idle_ctx);
	if (ret) {
		IDLE_ERROR("sub idle_module init failed\n");
		goto exit;
	}
	IDLE_DEBUG("vote_count:%d active:%d expire_count:%d\n",
		idle_ctx->vote_count, idle_ctx->active, idle_ctx->expire_count);
	IDLE_INFO("idle init succ\n");
	return 0;

exit:
	if (idle_ctx->idle_ctrl_wq)
		destroy_workqueue(idle_ctx->idle_ctrl_wq);
	if (idle_ctx->early_up_wq)
		destroy_workqueue(idle_ctx->early_up_wq);
	kfree(idle_ctx->input_handler);
	idle_ctx->input_handler = NULL;
	return ret;
}

void dpu_idle_ctx_deinit(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	idle_ctx->ops.deinit(idle_ctx);
	flush_workqueue(idle_ctx->idle_ctrl_wq);
	flush_workqueue(idle_ctx->early_up_wq);
	destroy_workqueue(idle_ctx->idle_ctrl_wq);
	destroy_workqueue(idle_ctx->early_up_wq);
	mutex_destroy(&idle_ctx->idle_lock);
	kfree(idle_ctx->input_handler);
	idle_ctx->input_handler = NULL;

	kfree(idle_ctx);
}
