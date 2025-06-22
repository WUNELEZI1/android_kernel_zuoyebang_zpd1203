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

#ifndef _DP_PANEL_H_
#define _DP_PANEL_H_

#include <linux/types.h>
#include <linux/version.h>
#include <drm/display/drm_dp_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_modes.h>
#include "dpu_hw_dsc_ops.h"
#include "dp_hw_ctrl.h"
#include "dp_hw_sctrl.h"
#include "dp_aux.h"
#include "dp_link.h"

#define SINK_DSC_CAP_SIZE                          0x10

struct dp_panel;
struct dp_connector_state;

/**
 * struct hdr_state - hdr state
 * @ext_sdp_on: if ext_sdp module is on
 * @power_off_flag: if ext_sdp module need to be turned off at next frame
 * @repeat_times: the number of times that sending the same sdp
 * @lock: used for hdr_state protection
 */
struct hdr_state {
	bool ext_sdp_on;
	bool power_off_flag;
	int repeat_times;
	spinlock_t lock;
};

struct dp_link_caps_sink {
	u8 max_lane_count;
	u8 max_link_rate;
	bool fec_supported;
	bool ssc_supported;
	bool tps3_supported;
	bool tps4_supported;
	bool enhance_frame_supported;
};

/**
 * dp_panel_caps - panel capabilities
 * @dpcd_version: sink dpcd version
 * @max_ext_sdp_cnt: sink supported maximum chained ext_sdp packets count
 * @link_caps: sink link capabilities
 * @dsc_caps: sink dsc capabilities
 * @audio_blk: audio cta data block
 * @hdr_caps: sink hdr capabilities
 */
struct dp_panel_caps {
	u8 dpcd_version;
	u32 max_ext_sdp_cnt;
	struct dp_link_caps_sink link_caps;
	struct dsc_caps_sink dsc_caps;
	struct dp_audio_cta_blk audio_blk;
	struct dp_hdr_caps_sink hdr_caps;
};

/* link information, used for video stream configuration */
struct dp_panel_link_info {
	enum dp_lane_count lane_count;
	u32 link_rate_mbps;
	u32 link_clk_khz;
	bool fec_en;
};

/**
 * dp_panel_video_info - video information for display
 * @timing: vertical and horizontal blank, active, front/back porch and so on
 * @link_info: link information, including lane count, link rate
 * @bpc: bits per component
 * @pixel_format: only support RGB now
 * @colorimetry: DP_COLORIMETRY_BT601 / DP_COLORIMETRY_BT709
 * @dynamic_range: DYNAMIC_RANGE_CEA / DYNAMIC_RANGE_VESA
 */
struct dp_panel_video_info {
	struct dp_display_timing timing;
	struct dp_panel_link_info link_info;
	enum dp_bpc bpc;
	enum pixel_format pixel_format;
	enum dp_colorimetry_space colorimetry;
	enum dynamic_range_type dynamic_range;
};

/**
 * dp_panel_dsc_info - dsc information for display
 * @dsc_en: if dsc function enables
 * @dsc_config: dsc configuration, valid when dsc_en = true
 */
struct dp_panel_dsc_info {
	bool dsc_en;
	struct dpu_dsc_config *dsc_config;
};

struct dp_panel_funcs {
	/**
	 * get_caps - get sink capabilities, stored in @panel->sink_caps
	 * @panel: the pointer of dp panel
	 * @conn: drm connector
	 *
	 * Returns 0 on success or a negative error code on failure.
	 */
	int (*get_caps)(struct dp_panel *panel, struct drm_connector *conn);

	/**
	 * clear_cpas - clear panel state
	 * @panel: the pointer of dp panel
	 * @conn: drm connector
	 */
	void (*clear_caps)(struct dp_panel *panel, struct drm_connector *conn);

	/**
	 * config_video - configure video information for enabling video stream
	 * @panel: the pointer of dp panel
	 *
	 * this function will configure necessary video information, including
	 * timing, msa, tu, init_threshold, dptx dsc for enabling video stream.
	 * this function will also turn on/off fec according sink caps,
	 * and turn on/off dsc according configuration.
	 * call ctrl->funcs->enable_video() to start video transmission
	 *
	 * Returns 0 on success or a negative error code on failure.
	 */
	void (*config_video)(struct dp_panel *panel);

	/**
	 * set_timing - set display timing
	 * @panel: the pointer of dp panel
	 * @mode: drm display mode
	 *
	 * timing will be get from @mode or be calculated by dsc_info or video_info
	 */
	void (*set_timing)(struct dp_panel *panel, struct drm_display_mode *mode);

	/**
	 * clear_video_info - clear video information
	 * @panel: the pointer of dp panel
	 */
	void (*clear_video_info)(struct dp_panel *panel);
};

/**
 * dp_panel - the structure to parse sink caps, config video info and dsc info
 * @sink_caps: sink capabilities
 * @dpcd_caps: sink capabilities raw data
 * @dsc_dpcd: sink dsc capabilities raw data
 * @fec_dpcd: sink fec status raw data
 * @lttpr_common_dpcd: sink lttpr common capabilities raw data
 * @edid: sink basic edid block, extension block is indicated by @edid->extensions
 * @video_info: video information including timing, msa ...
 * @dsc_info: dsc config and dsc enabled flag
 * @hdr_state: hdr state
 * @display: pointer of dp_display
 * @aux: the handle of aux chanel transaction
 * @hw_ctrl: the handle of dp controller hardware operation
 * @hw_sctrl: the handle of dptx sctrl hardware operation
 * @funcs: the dp panel supported functions
 */
struct dp_panel {
	struct dp_panel_caps sink_caps;
	u8 dpcd_caps[DP_RECEIVER_CAP_SIZE];
	u8 dsc_dpcd[SINK_DSC_CAP_SIZE];
	u8 fec_dpcd;
	u8 lttpr_common_dpcd[DP_LTTPR_COMMON_CAP_SIZE];
	struct edid *edid;

	struct dp_panel_video_info video_info;
	struct dp_panel_dsc_info dsc_info;
	struct hdr_state hdr_state;

	struct dp_display *display;
	struct dp_aux *aux;
	struct dp_hw_ctrl *hw_ctrl;
	struct dp_hw_sctrl *hw_sctrl;

	const struct dp_panel_funcs *funcs;
};

/**
 * dp_panel_get_color_depth - get dp panel color depth
 * @bpc: bits per component
 * @pixel_format: pixel format
 *
 * Return: color depth calculation results
 */
u32 dp_panel_get_color_depth(enum dp_bpc bpc, enum pixel_format pixel_format);

/**
 * dp_panel_send_metadata_intr_handler - handle metadata sending interrupt operation
 * @panel: the pointer of panel
 *
 * stop or power off ext_sdp sending according repeat time when vsync interrupt
 * trigger.
 */
void dp_panel_send_metadata_intr_handler(struct dp_panel *panel);

/**
 * dp_panel_config_hdr_commit - handle metadata sending in commit operation
 * @panel: the pointer of panel
 * @dp_conn_state: dp connector state
 *
 * start to sending dynamic or static metadata
 */
int dp_panel_config_hdr_commit(struct dp_panel *panel,
		struct dp_connector_state *dp_conn_state);

int dp_panel_init(struct dp_display *display, struct dp_panel **panel);
void dp_panel_deinit(struct dp_panel *panel);

#endif
