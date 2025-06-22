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

#ifndef _DPU_KMS_H_
#define _DPU_KMS_H_

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <drm/drm_device.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mode_object.h>
#include "dpu_drv.h"
#include "dpu_cap.h"
#include "dpu_res_mgr.h"
#include "dpu_intr_core.h"
#include "dpu_hw_rch.h"
#include "dpu_idle_helper.h"
#include "dpu_core_perf.h"
#include "dpu_tui.h"
#include "dpu_timeline.h"
#include "dpu_isr.h"

#define COPY_FD_TO_USER(user_ptr_val, fd) \
	copy_to_user(u64_to_user_ptr(user_ptr_val), fd, sizeof(int))

struct dpu_kms;

/* callback function info structure for every pipeline component */
struct dpu_pipe_cb_info {
	void *data;
	void (*commit)(struct drm_atomic_state *state, void *data);
	struct list_head list;
};

/**
 * dpu_virt_ctrl - dpu virtual controller structure
 * @hw_ctl_top: the hardware block pointer of control top
 * @intr_ctx: the interrupt context pointer
 * @hw_wb_top: the hardware block pointer of wb top
 */
struct dpu_virt_ctrl {
	struct dpu_hw_ctl_top *hw_ctl_top;
	struct dpu_intr_context *intr_ctx;
	struct dpu_hw_wb_top *hw_wb_top;
};

/* the atomic commit thread per pipeline */
struct dpu_commit_ctx {
	/* the commit related task data */
	struct task_struct *commit_task;
	struct kthread_worker commit_worker;
	struct kthread_work commit_work;

	/* the commit private data */
	struct drm_atomic_state *state;
	u32 commit_flag;
	struct mutex commit_lock;

	/* pre virtual pipeline need have own struct */
	struct dpu_isr isr;
	struct dpu_timeline timeline;
	struct dpu_timeline release_timeline;

	u64 ctx_frame_no;
};

/**
 * dpu_power_state - dpu power state
 * @dpu_need_power_on: need power on dpu
 * @dpu_need_power_off: need power off dpu
 * @dsi_need_power_on: need power on dsi, it's always false when dp/wb
 * @dsi_need_power_off: need power off dsi, it's always false when dp/wb
 * @dsi_panel_need_power_on: need power on dsi panel, it's always false when dp/wb
 * @dsi_panel_need_power_off: need power off dsi panel, it's always false when dp/wb
 */
struct dpu_power_state {
	bool dpu_need_power_on;
	bool dpu_need_power_off;
	bool dsi_need_power_on;
	bool dsi_need_power_off;
	bool dsi_panel_need_power_on;
	bool dsi_panel_need_power_off;
};

/**
 * dpu_power_ctrl - dpu power ctrl struct
 * @lowpower_ctrl: the display's lowpower switch information, see enum DPU_LP_CTRL bit filed
 * @crtc: the display's drm crtc
 * @desired_power_mask: the display's desired power mask
 * @actual_power_mask: the display's actual power mask at this time
 * @frame_power_mask: support frame power ctrl partition mask
 * @frame_ctrl_enabled: whether frame power ctrl is enabled or not
 * @need_restore: when the frame need to disable frame power ctrl, need restore regs
 *    to make sure hw work normally
 * @current_refresh_rate: current primary display refresh rate
 * @power_up_timeout: indicate whether dpu power up late or not
 * @frame_power_max_refresh_rate: max refresh rate that can enable frame power ctrl
 * @hw_ulps_max_refresh_rate: max refresh rate of hw ulps
 * @power_up_work: the work for the bottom half of the power-on interrupt
 * @power_down_work: the work for the bottom half of the power-down interrupt
 * @lock: the lock to protect power ctrl flow
 * @client: dpu power client point
 */
struct dpu_power_ctrl {
	u32 lowpower_ctrl;
	struct drm_crtc *crtc;

	u32 desired_power_mask;
	u32 actual_power_mask;
	u32 frame_power_mask;
	bool frame_ctrl_enabled;

	atomic_t need_restore;
	u32 current_refresh_rate;
	atomic_t power_up_timeout;

	u32 frame_power_max_refresh_rate;
	u32 hw_ulps_max_refresh_rate;

	struct kthread_work power_up_work;
	struct kthread_work power_down_work;

	/* protect mask */
	struct mutex lock;
	struct dpu_power_client *client;
};

/**
 * dpu_virt_pipe_running_state - dpu virt pipe running state
 * @DPU_VIRT_PIPELINE_DONE: isnt running
 * @DPU_VIRT_PIPELINE_RUN: is running
 */
enum dpu_virt_pipe_running_state {
	DPU_VIRT_PIPELINE_DONE = 0,
	DPU_VIRT_PIPELINE_RUN,
};

struct dpu_virt_pipe_state {
	u32 rch_mask;

	u32 old_vrefresh_rate;
	u32 old_connector_mask;

	u32 new_vrefresh_rate;
	u32 new_connector_mask;

	bool partial_update;
	u32 old_height;
	u32 new_height;

	struct dpu_hw_rch_top *rch_top[RCH_TOP_MAX];
	struct dpu_hw_rch *rch[RCH_MAX];
};

/**
 * dpu_virt_pipeline - dpu virtual pipeline structure
 * @dpu_kms: backpointer to the dpu_kms
 * @hw_sence_ctl: the hardware block pointer of scene control
 * @sw_clear_flag: the flag represent that sw clr is done
 * @sw_start_flag: the flag represent current frame is first frame, need do sw start
 * @updt_en: thg flag represent whether new frame is cfg ready
 * @is_offline: the flag represent whether is offline
 * @delay_mount_connector: scence ctrl need mount after active change
 * @underflow_flag: frm_timing_underflow intr event flag
 * @hw_hang: dpu hw hang, need do exception recovery
 *
 * @vstart_flag: tmg_vactive_start intr event flag
 * @vstart_wq: tmg_vactive_start event wait queue head
 *
 * @vsync_flag: tmg_vsync intr event flag
 *
 * @cfg_done_flag: cfg rdy clr intr event flag or wb done intr event flag
 * @cfg_done_wq: cfg rdy clr event wait queue head
 *
 * @rch_top: the hw handle of rch_top
 * @old_rch: currently working rch array
 * @new_rch: pending rch array to be committed
 * @post_pipe_crc_value: crc dump value
 * @commit_ctx: the info of commit_work
 * @power_state: the power state of display
 * @power_ctrl: the power ctrl struct for the virt pipe
 * @cfg_timestamp: the sw both cfg rdy timestamp
 * @frame_duration_ms: frame configuration waiting time
 * @cmdlist_frame_id: cmdlist frame id for pipeline
 * @actived_cmdlist_frame_id: actived cmdlist frame id for pipeline
 * @pipe_wq: the workqueue for the virt pipe
 * @working_state: working sw configuration
 * @pending_state: pending sw configuration
 *
 * @commit_state: sw commit state of the virt pipe
 */
struct dpu_virt_pipeline {
	struct dpu_kms *dpu_kms;
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	int index;
	bool sw_clear_flag;
	bool sw_start_flag;
	bool updt_en;
	bool is_offline;
	bool delay_mount_connector;
	atomic_t underflow_flag;
	atomic_t hw_hang;

	atomic_t vstart_flag;
	wait_queue_head_t vstart_wq;

	atomic_t vsync_flag;

	atomic_t cfg_done_flag;
	wait_queue_head_t cfg_done_wq;

	u32 post_pipe_crc_value;
	struct dpu_commit_ctx commit_ctx;

	struct dpu_power_state power_state;
	struct dpu_power_ctrl power_ctrl;

	ktime_t cfg_timestamp;
	u32 frame_duration_ms;

	s64 cmdlist_frame_id;
	s64 actived_cmdlist_frame_id;

	struct workqueue_struct *pipe_wq;

	struct dpu_virt_pipe_state working_state;
	struct dpu_virt_pipe_state pending_state;
};

/* the supported callback functions of dpu kms */
struct dpu_kms_funcs {
	/**
	 * init - initialize the dpu kms resources
	 * @dpu_kms: pointer of dpu_kms
	 *
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*init)(struct dpu_kms *dpu_kms);

	/**
	 * deinit - de-initialize the dpu kms resources
	 * @dpu_kms: pointer of dpu_kms
	 */
	void (*deinit)(struct dpu_kms *dpu_kms);

	/**
	 * check - check the dpu kms data before do preparation
	 * @dpu_kms: pointer of dpu_kms
	 * @state: pointer of the current atomic state
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*check)(struct dpu_kms *dpu_kms,
			struct drm_atomic_state *state);

	/**
	 * prepare - prepare dpu kms data before atomic commit
	 * @dpu_kms: pointer of dpu_kms
	 * @state: pointer of the current atomic state
	 */
	void (*prepare)(struct dpu_kms *dpu_kms,
			struct drm_atomic_state *state);

	/**
	 * pre_commit - commit dpu kms data before plane/crtc commit
	 * @dpu_kms: pointer of dpu_kms
	 * @state: pointer of the current atomic state
	 */
	void (*pre_commit)(struct dpu_kms *dpu_kms,
			struct drm_atomic_state *state);

	/**
	 * post_commit - commit dpu kms data after plane/crtc commit
	 * @dpu_kms: pointer of dpu_kms
	 * @state: pointer of the current atomic state
	 */
	void (*post_commit)(struct dpu_kms *dpu_kms,
			struct drm_atomic_state *state);

	/**
	 * finish - finish dpu kms committing
	 * @dpu_kms: pointer of dpu_kms
	 * @state: pointer of the current atomic state
	 */
	void (*finish)(struct dpu_kms *dpu_kms,
			struct drm_atomic_state *state);

	/**
	 * wait_for_hw_cfg_done - waiting for last configure take effect
	 * @dpu_kms: pointer of dpu_kms
	 * @state: pointer of the current atomic state
	 */
	void (*wait_for_hw_cfg_done)(struct dpu_kms *dpu_kms,
			struct drm_atomic_state *state);

	/**
	 * handle_intr_events - handle for cfg ready clear interrupt
	 * @dpu_kms: pointer of dpu_kms
	 * @crtc: pointer to drm crtc
	 * @intr_id: interrupt id
	 */
	void (*handle_intr_events)(struct dpu_kms *dpu_kms,
			struct drm_crtc *crtc, u32 intr_id);

	/**
	 * enable_event - enable kms event
	 * @dpu_kms: pointer of dpu_kms
	 * @obj: drm obj of crtc, plane, or connector
	 * @event_type: event in enum dpu_drm_event_type
	 * @enable: true for enabling event, or false for disabling event
	 */
	int (*enable_event)(struct dpu_kms *dpu_kms,
			struct drm_mode_object *obj,
			u32 event_type, bool enable);

	/**
	 * process_fences - prepare out-fence
	 * @crtc_state: pointer of the current atomic crtc state
	 * @commit_ctx: commit context info with fence timeline
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*process_fences)(struct drm_crtc_state *crtc_state,
			struct dpu_commit_ctx *commit_ctx);
};

/**
 * dpu_kms - the dpu kms structure
 * @drm_dev: the parent drm device pointer
 * @res_mgr_ctx: the resource manager context pointer
 * @virt_ctrl: the virtual ctrl instance
 * @virt_pipeline: the virtual pipeline instance
 * @pre_commit_cb_list: the pre-commit callback function list
 * @post_commit_cb_list: the post-commit callback function list
 * @lock: the spinlock for protecting dpu kms resources
 * @funcs: the supported functions for dpu kms
 * @flow_ctrl: the dpu flow control pointer
 * @power_mgr: the dpu power manager pointer
 * @color: the global private color instance
 * @idle_ctx: the idle context pointer
 * @core_perf: the dpu core perforcemance context pointer
 * @mem_pool: universal memory pool
 * @crc_dump_enable: to enable crc dump
 * @dpu_in_exceptional_state: dpu hardware is in exceptional state
 * @auto_recovery_en: switch for exception recovery when the
 *                         dpu works abnormally
 * @flush_bg_at_recovery: flush background color at recovery stage or not
 * @is_asic: the hardware platform is fpga or asic
 * @default_frame_timeout_ms: the default maximium frame duration in ms
 * @dbg_ctx: the dpu_dbg_ctx pointer
 * @suspend_state: save dpu state at pm suspend, restore at pm resume
 * @dsi_pm_mode_backup: save dsi power mode at pm suspend and restore at pm resume
 */
struct dpu_kms {
	struct drm_device *drm_dev;

	struct dpu_res_mgr_ctx *res_mgr_ctx;

	struct dpu_virt_ctrl virt_ctrl;
	struct dpu_virt_pipeline virt_pipeline[MAX_PIPELINE_COUNT];
	struct list_head pre_commit_cb_list;
	struct list_head post_commit_cb_list;

	struct dpu_kms_funcs *funcs;

	struct dpu_flow_ctrl *flow_ctrl;

	struct dpu_power_mgr *power_mgr;

	struct dpu_color *color;

	struct dpu_idle_ctx *idle_ctx;

	struct dpu_core_perf *core_perf;

	struct dksm_mem_pool *mem_pool;

	struct dpu_tui *tui;

	bool crc_dump_enable;

	bool dpu_in_exceptional_state;
	bool auto_recovery_en;

	bool flush_bg_at_recovery;
	bool is_asic;
	u32 default_frame_timeout_ms;

	struct dpu_dbg_ctx *dbg_ctx;

	struct drm_atomic_state *suspend_state;
	u32 dsi_pm_mode_backup;
};

static inline struct drm_connector *__find_connector(struct drm_device *dev,
		u32 *iter_mask)
{
	struct drm_connector *found = NULL;
	struct drm_connector *connector;
	struct drm_mode_config *config;
	unsigned long flags;

	if (!dev) {
		DPU_ERROR("invalid device pointer\n");
		return NULL;
	}

	config = &dev->mode_config;
	spin_lock_irqsave(&config->connector_list_lock, flags);
	list_for_each_entry(connector, &config->connector_list, head) {
		if (!((*iter_mask) & drm_connector_mask(connector)))
			continue;

		found = connector;
		*iter_mask &= ~drm_connector_mask(connector);
		break;
	}
	spin_unlock_irqrestore(&config->connector_list_lock, flags);

	return found;
}

/**
 * for_each_connector_per_crtc - iterate connector per one crtc
 * @connector: drm connector pointer for each connector instance
 * @crtc_state: drm crtc state pointer
 * @iter_mask: temporary u32 mask for iteration
 */
#define for_each_connector_per_crtc(connector, crtc_state, iter_mask) \
	for (iter_mask = crtc_state->connector_mask, \
		connector = __find_connector(crtc_state->crtc->dev, &iter_mask); \
		connector; connector = __find_connector( \
		crtc_state->crtc->dev, &iter_mask))

/**
 * is_clone_mode - check crtc mode
 * @state: the pointer of drm_atomic_state
 * @connector_state: the pointer of drm_connector_state
 *
 * Return: true for clone mode, otherwise false
 */
bool is_clone_mode(struct drm_atomic_state *state,
		struct drm_connector_state *connector_state);

/**
 * get_cloned_connector_type - get the type of the cloned connector
 * @state: the pointer of drm_atomic_state
 * @connector_state: the pointer of drm_connector_state
 * @id: the returned connector id
 *
 * Return: the enum value of connector type or -errno on failure case
 */
int get_cloned_connector_type(struct drm_atomic_state *state,
		struct drm_connector_state *connector_state, int *id);

/**
 * The pre_commit/post_commit callback register/unregister APIs
 * @drm_dev: the pointer of drm device
 * @commit: the callback function pointer
 * @data: the data pointer for callback
 *
 * Return: zero on success, -ERRNO on failure
 *
 * Note: The commit callback should be registered in initialized stage
 */
int dpu_kms_pre_commit_cb_register(struct drm_device *drm_dev,
		void (*commit)(struct drm_atomic_state *, void *), void *data);
void dpu_kms_pre_commit_cb_unregister(struct drm_device *drm_dev,
		void (*commit)(struct drm_atomic_state *, void *));
int dpu_kms_post_commit_cb_register(struct drm_device *drm_dev,
		void (*commit)(struct drm_atomic_state *, void *), void *data);
void dpu_kms_post_commit_cb_unregister(struct drm_device *drm_dev,
		void (*commit)(struct drm_atomic_state *, void *));

/**
 * dpu_kms_obj_init - initialize dpu kms object
 * @drm_dev: the pointer of drm device
 *
 * Return: zero on success, -ERRNO on failure
 */
int dpu_kms_obj_init(struct drm_device *drm_dev);

/**
 * dpu_kms_res_init - de-initialize dpu kms object
 * @drm_dev: the pointer of drm device
 */
void dpu_kms_obj_deinit(struct drm_device *drm_dev);

static inline bool need_enable_crtc(struct drm_crtc_state *new_state)
{
	return new_state->active && new_state->active_changed;
}

static inline bool need_disable_crtc(struct drm_crtc_state *new_state)
{
	return !new_state->active && new_state->active_changed;
}

void dpu_force_refresh_event_notify(struct drm_device *drm_dev);

void dpu_kms_fence_force_signal(struct drm_crtc *crtc, bool need_dump);

void dpu_virt_pipe_state_swap(struct dpu_virt_pipeline *pipeline);

void dpu_virt_pipe_partial_cfg_update(struct dpu_virt_pipeline *pipeline,
		u32 old_height,
		u32 new_height);

void dpu_kms_exception_recovery(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *new_crtc_state,
		struct drm_crtc *crtc);

#endif /* _DPU_KMS_H_ */
