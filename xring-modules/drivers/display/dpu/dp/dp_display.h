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

#ifndef _DP_DISPLAY_H_
#define _DP_DISPLAY_H_

#include <linux/clk.h>
#include <drm/drm_device.h>
#include <drm/drm_connector.h>
#include <soc/xring/xr_usbdp_event.h>
#include "dp_hw_ctrl.h"
#include "dp_hw_msgbus.h"
#include "dp_hw_sctrl.h"
#include "dp_hw_tmg.h"
#include "dp_parser.h"
#include "dp_aux.h"
#include "dp_hpd.h"
#include "dp_ctrl.h"
#include "dp_panel.h"
#include "dp_power.h"
#include "dp_hdcp.h"
#include "dp_audio.h"

#define MAX_DP_DISPLAY_NUM                1
#define DP_DEVICE_ID_0                    0

struct obuf_depth_info;
enum hdcp_run_info;

enum dp_test_tmg_mode {
	DP_TEST_TMG_RED = 1,
	DP_TEST_TMG_GREEN,
	DP_TEST_TMG_BLUE,
	DP_TEST_TMG_CUSTOM,
	DP_TEST_TMG_MAX
};

enum dp_test_vg_mode {
	DP_TEST_VG_COLOR_BAR = 1,
	DP_TEST_VG_COLOR_RED,
	DP_TEST_VG_COLOR_GREEN,
	DP_TEST_VG_COLOR_BLUE,
	DP_TEST_VG_MAX
};

/**
 * dp_intr_ops - dp internal interrupt structure
 * @data: private data pointer
 * @enable: enable callback pointer
 * @is_triggered: is triggered callback pointer
 * @clear: clear callback pointer
 * @handler: handler callback pointer
 * @list: operations list
 */
struct dp_intr_ops {
	void *data;
	int (*enable)(void *data, bool enable);
	bool (*is_triggered)(void *data);
	int (*clear)(void *data);
	int (*handler)(void *data);
	struct list_head list;
};

struct dp_hw_modules {
	struct dp_hw_ctrl *hw_ctrl;
	struct dp_hw_msgbus *hw_msgbus;
	struct dp_hw_sctrl *hw_sctrl;
	struct dp_hw_tmg *hw_tmg;
};

/**
 * dp_module_ctrl - structure for controlling dp module
 * @dsc_enable: change the default dsc enabled state
 * @tmg_test_index: tmg self test mode index, dp-dsc will be disabled if enable this option
 * @vg_test_index: vg self test mode index, dp-dsc will be disabled if enable this option
 */
struct dp_module_ctrl {
	bool dsc_enable;
	bool dp_hdcp_enable;

	u32 tmg_test_index;
	u32 vg_test_index;
};

/**
 * dp_display_status - dp display status structure
 * @orientation: the current dp orientation
 * @combo_mode: the current combo phy mode
 * @hpd: the current hpd state
 * @is_training_done: flag for checking training state
 * @is_connector_enabled: flag for checking connector state
 * @is_dsc_enabled: flag for checking dsc state
 * @timing: current display timing
 * @maxpclk_khz: the max pclk of the display timing in unit of kHz
 * @tmg_underflow_count: timing underflow frame count
 * @tmg_state: timing state
 * @hdcp_availabled: flag of hdcp enabled state
 * @is_hdcp_encryption_enabled: flag of hdcp encryption enabled state
 */
struct dp_display_status {
	enum dp_orientation orientation;
	enum dp_combo_mode combo_mode;

	u8 hpd;

	bool is_training_done;

	bool is_connector_enabled;
	bool is_dsc_enabled;
	struct dp_display_timing *timing;
	u64 maxpclk_khz;

	u32 tmg_underflow_count;
	u32 tmg_state;

	bool hdcp_availabled;
	bool is_hdcp_encryption_enabled;
};

struct dp_lp_delayed_work {
	struct dp_display *display;
	struct kthread_delayed_work base;
};

#define to_dp_lp_delayed_work(x) container_of(x, struct dp_lp_delayed_work, base)

/**
 * dp_display - dp display structure
 * @drm_dev: the drm device pointer
 * @connector: bind connector for this display
 * @parser: the dp parser pointer
 * @aux: the dp aux pointer
 * @hpd: the dp hpd pointer
 * @ctrl: the dp ctrl pointer
 * @panel: the dp panel pointer
 * @power: the dp power pointer
 * @hdcp: the dp hdcp pointer
 * @audio: to save audio info
 * @hw_modules: dp hardware modules pointers
 * @link_config: the link config of dp display
 * @module_ctrl: the module control config of dp display
 * @status: the status for dp display
 * @ipi_clk: handler to adjust ipi clock
 * @xr_usb: handler of xring usb
 * @lock: spin lock for dp display
 * @irq_list: the list head of irq
 * @video_intr_ops: the video interrupt operations
 * @stream_underflow_flag: flag for video stream underflow
 * @stream_overflow_flag: flag for video stream overflow
 * @audio_overflow_flag: flag for audio stream overflow
 */
struct dp_display {
	struct drm_device *drm_dev;
	struct drm_connector *connector;

	struct dp_parser *parser;
	struct dp_aux *aux;
	struct dp_hpd *hpd;
	struct dp_ctrl *ctrl;
	struct dp_panel *panel;
	struct dp_power *power;
	struct dp_hdcp *hdcp;
	struct dp_audio audio;
	struct dp_hw_modules hw_modules;

	struct dp_module_ctrl module_ctrl;
	struct dp_display_status status;

	struct clk *ipi_clk;

	struct xring_usb *xr_usb;

	spinlock_t lock;
	struct list_head irq_list;

	/**
	 * task, worker and work for dp low-power function
	 */
	struct task_struct *lp_worker_task;
	struct kthread_worker lp_worker;
	struct dp_lp_delayed_work lp_delayed_work;

	struct dp_intr_ops video_intr_ops;
	atomic_t stream_underflow_flag;
	atomic_t stream_overflow_flag;
	atomic_t audio_overflow_flag;
};

/**
 * dp_display_get - get dp display pointer
 * @index: the index of dp display
 *
 * Return: valid pointer on success, NULL on failure
 */
struct dp_display *dp_display_get(u8 index);

/**
 * dp_display_irq_enable - enable dp internal interrupt
 * @display: the pointer of dp display
 * @enable: enable or disable flag
 */
void dp_display_irq_enable(struct dp_display *display, bool enable);

/**
 * dp_display_irq_register - register dp internal interrupt
 * @display: the pointer of dp display
 * @ops: the operations of interrupt
 */
void dp_display_irq_register(struct dp_display *display, struct dp_intr_ops *ops);

/**
 * dp_display_irq_unregister - unregister dp internal interrupt
 * @display: the pointer of dp display
 * @ops: the operations of interrupt
 */
void dp_display_irq_unregister(struct dp_display *display, struct dp_intr_ops *ops);

/**
 * dp_display_tmg_state_get - get tmg state with dp display
 * @display: the pointer of dp display
 *
 * read tmg underflow count and tmg state, save in display->status. if dp
 * encoder is disable, tmg hardware is inaccessible, tmg underflow count and
 * tmg state will remain unchanged.
 *
 * Return: zero on success or -errno on failure
 */
int dp_display_tmg_state_get(struct dp_display *display);

/**
 * dp_display_send_black_frame - send black frame to monitor
 * @display: the pointer of dp display
 */
void dp_display_send_black_frame(struct dp_display *display);

/**
 * dp_display_video_stream_self_test - enable self test or not for video stream
 * @display: the pointer of display
 */
void dp_display_video_stream_self_test(struct dp_display *display);

/**
 * dp_display_video_stream_on - enable dp video stream
 * @display: the pointer of display
 */
void dp_display_video_stream_on(struct dp_display *display);

/**
 * dp_display_video_stream_off - disable dp video stream
 * @display: the pointer of display
 */
void dp_display_video_stream_off(struct dp_display *display);

/**
 * dp_display_check_tmg_timing - check if timing is valid for tmg
 * @timing: video stream timing information
 * @dsc_en: if dsc is enable or disable
 */
bool dp_display_check_tmg_timing(struct dp_display_timing *timing, bool dsc_en);

/**
 * dp_display_obuf_depth_info_get - get dp obuffer depth info
 * @display: the pointer of display
 * @depth_info: output of obuffer depth info
 */
void dp_display_obuf_depth_info_get(struct dp_display *display,
		struct obuf_depth_info *depth_info);

/**
 * dp_display_prepare_video_info - prepare video information
 * @display: the pointer of dp display
 * @mode: drm display mode
 * @dsc_cfg: dsc configuration
 *
 * prepare video information and dsc information for dp display
 */
void dp_display_prepare_video_info(struct dp_display *display,
		struct drm_display_mode *mode, struct dpu_dsc_config *dsc_cfg);

/**
 * dp_display_clear_video_info - clear video information
 * @display: the pointer of dp display
 */
void dp_display_clear_video_info(struct dp_display *display);

/**
 * dp_display_update_maxpclk - update max pclk of training result
 * @display: the pointer of dp display
 */
void dp_display_update_maxpclk(struct dp_display *display);

/**
 * dp_display_line_buf_level_cfg - configure obuffer level
 * @display: the pointer of dp display
 */
void dp_display_line_buf_level_cfg(struct dp_display *display);

/**
 * dp_display_prepare_link_param - prepare link parameters
 * link parameters come from: a) device-tree pre-defined
 * b) sink capabilities  c) debugfs input
 * The function choose which parameters are used finally
 */
void dp_display_prepare_link_param(struct dp_display *display);

/**
 * dp_display_lp_exit - exit low-power state and resume video playing
 * @display: the pointer of dp display
 */
void dp_display_lp_exit(struct dp_display *display);

#endif /* _DP_DISPLAY_H_ */
