/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DPU_IDLE_CORE_H_
#define _DPU_IDLE_CORE_H_

#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <drm/drm_device.h>

#include "dpu_intr_core.h"
#include "dsi_connector.h"

struct dpu_idle_ctx;

#define ONE_FRAME_TIME_MS 300
#define IDLE_EXPIRE_FRAME_NUM 8

extern uint32_t g_idle_duration_ms;

struct dpu_idle_ops {
	/**
	 * init - init func for idle ctrl
	 * @idle_ctx: struct dpu_idle_ctx object
	 */
	int (*init)(struct dpu_idle_ctx *idle_ctx);

	/**
	 * check_idle_policy - check idle policy valid
	 * cmd mode and video mode have different hw constraints in idle policy
	 * @idle_policy: idle policy
	 *
	 * Return: true for success, false for failure
	 */
	bool (*check_idle_policy)(u32 idle_policy);

	/**
	 * get_basic_policy - get basic policy
	 *
	 * Return: u32 basic_policy
	 */
	u32 (*get_basic_policy)(void);

	/**
	 * prepare - prepare idle for enter idle
	 *
	 * @idle_ctx: idle_ctx object
	 * Return: 0 on success, else on failure
	 */
	int (*prepare)(struct dpu_idle_ctx *idle_ctx);

	/**
	 * enter - enter idle status
	 *
	 * @idle_ctx: idle_ctx object
	 */
	void (*enter)(struct dpu_idle_ctx *idle_ctx);

	/**
	 * exit - exit idle status
	 *
	 * @idle_ctx: idle_ctx object
	 */
	void (*exit)(struct dpu_idle_ctx *idle_ctx);

	/**
	 * deinit - deinit idle object
	 *
	 * @idle_ctx: idle_ctx object
	 */
	void (*deinit)(struct dpu_idle_ctx *idle_ctx);

	/**
	 * status_dump - dump idle exception info
	 * @idle_ctx: idle_ctx
	 */
	void (*status_dump)(struct dpu_idle_ctx *idle_ctx);

	/**
	 * postprocess - post process of exit idle
	 * @idle_ctx: idle_ctx
	 */
	void (*postprocess)(struct dpu_idle_ctx *idle_ctx);
};

enum idle_mode {
	IDLE_NONE                   = 0,
	IDLE_DSC_SELF_FLUSH         = BIT(0), /* self flush depends on dsc-wdma and dsc-rdma，for video panel */
	IDLE_ISR_OFF                = BIT(1), /* close first-level interrupt response */
	IDLE_MIPI_ULPS              = BIT(2), /* enter ulps between frames */
	IDLE_P0_CLK_OFF             = BIT(3), /* clks in dpu-part0 */
	IDLE_P1_CLK_OFF             = BIT(4), /* clks in dpu-part1, no use */
	IDLE_P2_CLK_OFF             = BIT(5), /* clks in dpu-part2, no use */
	IDLE_DSI_CLK_OFF            = BIT(6), /* clk_dpu_dsi 0/1 */
	IDLE_P0_POWER_OFF           = BIT(7),
	IDLE_P1_POWER_OFF           = BIT(8),
	IDLE_P2_POWER_OFF           = BIT(9),
	IDLE_DSI_POWER_OFF          = BIT(10), /* no use */
	IDLE_DPU_CLK_RATE_ADJUST    = BIT(11), /* no use */
	IDLE_DDR_RATE_ADJUST        = BIT(12), /* no use */
};

enum idle_restore_policy {
	NO_NEED_RESTORE             = 0,
	RESTORE_REGISITER           = BIT(0), /* The reg of P1/P2 has been clear, restore registers */
	RESTART_DPU_DISPLAY         = BIT(1), /* DPU has been suspended, need restart dpu */
};

enum idle_policy_level {
	NORMAL = 0,
	BASIC,
};

/**
 * Both the PRESENT_CH and the CTRL_CH can control the entry and exit of IDLE,
 * but only the PRESENT_CH can restore the state of restore_mode
 */
enum idle_vote_ch {
	PRESENT_CH, /* present commit channel */
	CTRL_CH, /* control channel, such as vsync enable evevt from hwc */
	VOTE_CH_NUM,
};

struct idle_trace {
	u32 enter_frame_no;
	u64 enter_time;
	u64 exit_time;
	u64 present_time;
};

/**
 * dpu_idle_ctx - dpu idle manager object
 *
 * @idle_policy: bit mask of idle_mode
 * @vote_count: vote count for control idle entry
 * @restore_mode: state used to judge restore display effect config
 * @active: Indicates the current idle state, which needs to be managed by mutex_lock
 *          Set to false when entering idle and true when exiting idle
 * @idle_lock: mutex lock for @active
 * @need_restore_clk: whether need restore clk config, like top cg, doze etc
 * @expire_count: When the value exceeds the threshold,
 *                the system enters idle and uses spin_lock to manage the value
 * @spin_lock: spin lock for @expire_count
 * @need_active: used to notify the idle thread to end the
 *               idle process as soon as possible (no need lock to protect)
 * @idle_work: struct work_struct for idle ctrl
 * @idle_ctrl_wq: struct workqueue_struct for idle ctrl
 * @drm_dev: drm device
 * @res_mgr_ctx: struct dpu_res_mgr_ctx
 * @intr_ctx: struct dpu_intr_context
 * @power_mgr: struct dpu_power_mgr
 * @conn: connector for primary panel
 * @crtc: crtc for primary panel
 * @dpu_kms: point of dpu kms
 * @mem_pool: point of dpu mem pool
 * @hw_scene_ctl: common hw res of scene ctrl for primary panel
 * @pinfo: fixed panel info
 * @ops: dpu_idle_ops
 * @priv: private object
 * @trace: record idle trace
 * @ulps_status: record ulps status
 */
struct dpu_idle_ctx {
	u32 idle_policy;
	u32 vote_count;
	u32 restore_mode;

	bool active;
	struct mutex idle_lock;

	bool need_restore_clk;

	u32 expire_count;
	spinlock_t spin_lock;

	atomic_t need_active;

	struct work_struct idle_work;
	struct workqueue_struct *idle_ctrl_wq;

	struct work_struct early_up_work;
	struct workqueue_struct *early_up_wq;
	struct drm_device *drm_dev;
	struct dpu_res_mgr_ctx *res_mgr_ctx;
	struct dpu_intr_context *intr_ctx;
	struct dpu_power_mgr *power_mgr;
	struct drm_connector *conn;
	struct drm_crtc *crtc;
	struct dpu_kms *dpu_kms;
	struct dksm_mem_pool *mem_pool;

	/* common hw resource */
	struct dpu_hw_scene_ctl *hw_scene_ctl;

	/* const panel info */
	struct dsi_display_info *pinfo;

	struct dpu_idle_ops ops;
	void *priv;

	struct idle_trace trace;

	bool ulps_status;
	struct input_handler *input_handler;
};

struct dpu_idle_ctx *dpu_get_idle_ctx(void);

/**
 * dpu_idle_vote_stop - exit idle status before dpu hardware operation
 *
 * @vote_ch: vote channel
 */
void dpu_idle_vote_stop(enum idle_vote_ch vote_ch);

/**
 * dpu_idle_vote_restart - restart idle func after dpu hardware operation
 *
 * @vote_ch: vote channel
 */
void dpu_idle_vote_restart(enum idle_vote_ch vote_ch);

/**
 * dpu_idle_task_commit - try to enter idle status in vsync isr
 */
void dpu_idle_task_commit(void);

/**
 * dpu_idle_ctx_init - init idle helper for user
 *
 * @dpu_kms: dpu_kms object
 * Return 0 on succ, else on fail
 */
int dpu_idle_ctx_init(struct dpu_kms *dpu_kms);

/**
 * dpu_idle_ctx_deinit - deinit idle helper for user
 */
void dpu_idle_ctx_deinit(void);

static inline bool is_idle_policy_valid(struct dpu_idle_ctx *idle_ctx)
{
	return idle_ctx->ops.check_idle_policy(idle_ctx->idle_policy);
}

bool is_dpu_power_safe_with_dpu_idle(void);

/**
 * dpu_idle_set_idle_policy - set idle policy
 *
 * @idle_policy: idle policy
 */
void dpu_idle_set_idle_policy(u32 idle_policy);

/**
 * dpu_idle_ulps_exit - exit ulps when dpu is in cmd idle status
 */
void dpu_idle_ulps_exit(void);

bool is_idle_enabled(void);

/**
 * dpu_idle_ulps_enter - enter ups when dpu is in cmd idle status
 */
void dpu_idle_ulps_enter(void);

void dpu_idle_input_handler_register(void);

void dpu_idle_input_handler_unregister(void);

#endif
