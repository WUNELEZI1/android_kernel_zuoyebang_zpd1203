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

#ifndef _DPU_CORE_PERF_H_
#define _DPU_CORE_PERF_H_

#include "dpu_hw_dvfs.h"
#include "dpu_kms.h"
#include "dsi_display.h"
#include "dpu_hw_init_module_ops.h"
#include "dpu_power_mgr.h"

#define DPU_CORE_PERF_CLK_NUM                                  5
#define DPU_CORE_PERF_INVALID_PROFILE_ID                      -1
#define DP_TIMING_VFP_NEED_DISABLE_DPU_TOP_AUTO_CG_VAL         1

struct dpu_power_ctrl_info;

/**
 * Attention:
 * DPU_POWER_PROTECT_BEGIN and DPU_POWER_PROTECT_END used in work handler (struct work_struct),
 * and cannot be used in an interrupt context, because exist mutex lock,
 * and must be called in pairs.
 */
#define DPU_POWER_PROTECT_BEGIN() \
	dpu_power_get_helper(BIT(DPU_PARTITION_0) | BIT(DPU_PARTITION_1))

#define DPU_POWER_PROTECT_END() \
	dpu_power_put_helper(BIT(DPU_PARTITION_0) | BIT(DPU_PARTITION_1))

enum dpu_top_cg_status {
	DPU_AUTO_CG_DISABLED,
	DPU_AUTO_CG_ENABLED,
	DPU_AUTO_CG_UNKNOWN,
};

struct dpu_core_perf_params {
	unsigned long aclk_rate;
	unsigned long mclk_rate;
};

/**
 * dpu_profile_info - dpu profiles struct
 * @profile_id: dpu profile id info
 * @mclk_rate: current profile's mclk rate
 * @aclk_rate: current profile's aclk rate
 * @pclk_rate: current profile's pclk rate
 * @dscclk0: current profiles' dsc clk0's rate
 * @dscclk1: current profiles' dsc clk1's rate
 */
struct dpu_profile_info {
	u32 profile_id;
	unsigned long mclk_rate;
	unsigned long aclk_rate;
	unsigned long pclk_rate;
	unsigned long dscclk0_rate;
	unsigned long dscclk1_rate;
};

/**
 * dpu_core_perf_tune - current dpu profile's caps
 * @min_mclk_rate: dpu's min mclk rate
 * @max_mclk_rate: dpu's max mclk rate
 * @max_aclk_rate: dpu's max aclk rate
 */
struct dpu_core_perf_tune {
	unsigned long min_mclk_rate;
	unsigned long max_mclk_rate;
	unsigned long max_aclk_rate;
};

/**
 * dpu_core_perf_ctrl - used for dpu core perf ctrl
 * @lp_ctrl: dpu lowpower's switch
 * @perf_tune: dpu core perf tunning struct
 */
struct dpu_core_perf_ctrl {
	/* see enum DPU_LP_CTRL */
	u32 lp_ctrl;

	struct dpu_core_perf_tune perf_tune;
};

/* depends on dpu idle strategy */
struct dpu_core_perf_idle_strategy {
	u32 idle_expire_frame_count;
	u32 idle_mode;
};

/* define dpu lowpower features */
enum DPU_LP_CTRL {
	DPU_LP_ALL_DISABLE = 0,
	DPU_LP_TOP_AUTO_CG_ENABLE = BIT(0),
	DPU_LP_PCLK_AUTO_CG_ENABLE = BIT(1),
	DPU_LP_AUTO_CG_ENABLE = BIT(2),
	DPU_LP_SRAM_LP_ENABLE = BIT(3),
	DPU_LP_HW_DVFS_ENABLE = BIT(4),
	DPU_LP_DOZE_ENABLE = BIT(5),
	DPU_LP_HW_PPLL2_CTRL_ENABLE = BIT(6),
	DPU_LP_IDLE_CTRL_ENABLE = BIT(7),
	DPU_LP_FRAME_POWER_CTRL_ENABLE = BIT(8),
	DPU_LP_HW_ULPS = BIT(9),
};

#define DPU_LP_ALL_ENABLE	\
		(DPU_LP_TOP_AUTO_CG_ENABLE | \
		DPU_LP_PCLK_AUTO_CG_ENABLE | \
		DPU_LP_AUTO_CG_ENABLE | \
		DPU_LP_SRAM_LP_ENABLE | \
		DPU_LP_HW_DVFS_ENABLE | \
		DPU_LP_DOZE_ENABLE |  \
		DPU_LP_HW_PPLL2_CTRL_ENABLE | \
		DPU_LP_IDLE_CTRL_ENABLE |\
		DPU_LP_FRAME_POWER_CTRL_ENABLE)

/**
 * define dpu core perf's vote channel:
 * DPU_CORE_PERF_CHANNEL_DEBUG used for debug purpose
 * DPU_CORE_PERF_CHANNEL_NORMAL used for hwc or boot up scene
 */
enum dpu_core_perf_vote_channel {
	DPU_CORE_PERF_CHANNEL_DEBUG = 0,
	DPU_CORE_PERF_CHANNEL_NORMAL,
	DPU_CORE_PERF_CHANNEL_MAX_NUM,
};

/**
 * define dpu ddr bandwidth vote channel:
 * DPU_DDR_CHANNEL_DEBUG used for debug purpose
 * DPU_DDR_CHANNEL_NORMAL used for hwc scene
 */
enum {
	DPU_DDR_CHANNEL_DEBUG = 0,
	DPU_DDR_CHANNEL_NORMAL,
	DPU_DDR_CHANNEL_MAX_NUM,
};

/**
 * define dpu doze status
 */
enum {
	DPU_DOZE_ENABLED,
	DPU_DOZE_DISABLED,
	DPU_DOZE_UNKNOWN,
};

/**
 * define ddr vote channel for different module
 */
enum DDR_CHN_ID {
	DPU_CH = 0,
	VEU_CH,
};

/**
 * dpu_doze_ctrl: dpu doze ctrl struct
 * ppll2_status: whether dpu ppll2 feature enabled or disabled
 * ppll2_counter: the vote counter of ppll2 feature,
 *             1) from 0 to 1 will enable dpu ppll2 feature
 *             2) from 1 to 0 will disable dpu ppll2 feature
 * doze_status: whether dpu doze feature enabled or disabled
 * doze_counter: the vote counter of doze feature,
 *             1) from 0 to 1 will enable dpu doze feature
 *             2) from 1 to 0 will disable dpu doze feature
 * ppll2_lock: protect ppll2 vote_count and status
 * doze_lock: protect doze vote_count and status
 */
struct dpu_doze_ctrl {
	u32 ppll2_status;
	int ppll2_counter;

	u32 doze_status;
	u32 doze_counter;

	spinlock_t ppll2_lock;
	spinlock_t doze_lock;
};

/**
 * struct dpu_top_cg_mgr - dpu top auto clock gatting manager
 *		mananger structure
 * @vote_counter: vote counter for top cg
 * @status: current dpu top auto clock gating status
 * @lock: spin lock to protect the manager's members
 */
struct dpu_top_cg_mgr {
	int vote_counter;
	enum dpu_top_cg_status status;
	spinlock_t lock;
};

/**
 * dpu_core_perf: dpu core perforcemance context
 * @dpu_kms: the dpu_kms pointer
 * @power_mgr: dpu power mgr
 * @perf_ctrl: dpu core perf's lowpower switch
 * @profile_num: total dpu profile num
 * @profiles: each dpu profile's detail info
 * @dsi_display_info: used to get dsc info
 * @perf_mutex: core perf update lock
 * @hw_dvfs_mgr: manage dpu hw dvfs module
 * @top_cg_mgr: dpu top cg manager
 * @doze_ctrl: dpu doze ctrl struct
 * @vote_level: current vote level of each channel
 * @vote_status: current vote status of each channel
 * @vote_perf: the final dpu vote perf info
 * @curr_perf: the hardware real perf info
 * @curr_profile: current vote profile id
 * @ddr_mutex: ddr update lock
 * @ddr_bandwidth: current ddr of each channel
 * @curr_bandwidth: current band width
 */
struct dpu_core_perf {
	struct dpu_kms *dpu_kms;
	struct dpu_power_mgr *power_mgr;

	struct dpu_core_perf_ctrl perf_ctrl;

	u32 profile_num;
	struct dpu_profile_info *profiles;

	struct dsi_display_info *dsi_info;

	struct mutex perf_mutex;

	struct dpu_hw_dvfs_mgr hw_dvfs_mgr;
	struct dpu_top_cg_mgr top_cg_mgr;

	struct dpu_doze_ctrl doze_ctrl;

	u32 vote_level[DPU_CORE_PERF_CHANNEL_MAX_NUM];
	bool vote_status[DPU_CORE_PERF_CHANNEL_MAX_NUM];
	struct dpu_core_perf_params vote_perf;
	struct dpu_core_perf_params curr_perf;
	int curr_profile;

	struct mutex ddr_mutex;
	u32 ddr_bandwidth[DPU_DDR_CHANNEL_MAX_NUM];
	u32 curr_bandwidth;
};

/**
 * dpu_ddr_bandwidth_update - do ddr bandwidth vote for dpu
 * @core_perf: the dpu core perf pointer
 * @bandwidth_cmd: bit31: is block vote, bit30~0: the expect bandwidth
 * @channel: dpu ddr vote channel
 *
 * Returns: 0: success, others: fail to update
 */
int dpu_ddr_bandwidth_update(struct dpu_core_perf *core_perf, u32 bandwidth_cmd, u32 channel);

/**
 * veu_ddr_bandwidth_update - do ddr bandwidth vote for veu
 * @veu_expect_bandwidth: the bandwidth expect to vote
 *
 * Returns: 0: success, others: fail to update
 */
int veu_ddr_bandwidth_update(u32 veu_expect_bandwidth);

/**
 * dpu_core_perf_update - do one channel vote business
 * @dpu_kms: the dpu kms pointer
 * @channel_id: the vote channel, see enum dpu_core_perf_vote_channel
 * @profile_vote: the perf level to do vote
 * @vote_on: do vote on or cancel the channel's vote
 * @force: do force vote even if the same profile
 */
int dpu_core_perf_update(struct dpu_core_perf *core_perf, u32 channel_id,
		u32 profile_vote, bool vote_on, bool force);

/**
 * dpu_core_perf_crtc_update - update crtc(display)'s core perf requirement
 * @crtc: the crtc pointer
 * @vote_on: whether the crtc is vote on or off
 */
void dpu_core_perf_crtc_update(struct drm_crtc *crtc, bool vote_on);

/**
 * dpu_core_perf_prepare - do core perf prepare business
 * @perf: the core perf pointer
 * @crtc: the crtc to do perf prepare
 * @old_state: the crtc's old state
 * @new_state: the crtc's new state
 */
void dpu_core_perf_prepare(struct dpu_core_perf *perf,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state);

/**
 * dpu_core_perf_finish - do core perf finish business
 * @perf: the core perf pointer
 * @crtc: the crtc to do perf finish
 * @old_state: the crtc's old_state
 * @new_state: the crtc's new_state
 */
void dpu_core_perf_finish(struct dpu_core_perf *perf,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state);

/**
 * dpu_core_perf_hw_init - do dpu core perf hw init
 * @perf: the core perf pointer
 * @ctrl_info: the ctrl info used for hw init
 * @part_id: dpu partition id
 */
void dpu_core_perf_hw_init(struct dpu_core_perf *perf,
		struct dpu_power_ctrl_info *ctrl_info,
		u32 part_id);

/**
 * dpu_core_perf_hw_deinit - do dpu core perf hw deinit
 * @perf: the dpu core perf pointer
 * @part_id: dpu partition id
 */
void dpu_core_perf_hw_deinit(struct dpu_core_perf *perf,
		u32 part_id);

/**
 * dpu_core_perf_sw_reset - do core perf sw state reset
 * @dpu_kms: the dpu kms pointer
 */
void dpu_core_perf_sw_reset(struct dpu_core_perf *perf);

/**
 * dpu_core_perf_clk_init - mclk & dsc clk init
 * @core_perf: the dpu core perf pointer
 */
void dpu_core_perf_clk_init(struct dpu_core_perf *core_perf);

/**
 * dpu_core_perf_clk_deinit - reset mclk & dsc clk
 * @core_perf: the dpu core perf pointer
 */
void dpu_core_perf_clk_deinit(struct dpu_core_perf *core_perf);

/**
 * dpu_core_perf_update_lp_ctrl - update lowpower switch
 * @perf: the core perf pointer
 * @lp_ctrl: new lowpower switch
 */
void dpu_core_perf_update_lp_ctrl(struct dpu_core_perf *perf,
		u32 lp_ctrl);

/**
 * is_dpu_lp_enabled - check whether the lowpower feature is enabled
 * @feature: the lowpower feature to do check
 *
 * Returns: true if the lowpower feature is enabled, false otherwise
 */
bool is_dpu_lp_enabled(u32 feature);

/**
 * dpu_core_perf_top_cg_enable - helper function to vote off top auto clock gatting
 */
void dpu_core_perf_top_cg_enable(void);

/**
 * dpu_core_perf_top_cg_disable - helper function to vote on top auto clock gatting
 */
void dpu_core_perf_top_cg_disable(void);

/**
 * dpu_core_perf_clk_active - set dpu clk active
 */
void dpu_core_perf_clk_active(void);

/**
 * dpu_core_perf_clk_deactive - set dpu clk deactive
 */
void dpu_core_perf_clk_deactive(void);

/**
 * dpu_core_perf_ppll2_enable - vote enable dpu ppll2
 */
void dpu_core_perf_ppll2_enable(void);

/**
 * dpu_core_perf_ppll2_disable - vote disable dpu ppll2
 */
void dpu_core_perf_ppll2_disable(void);

/**
 * dpu_core_perf_init - do core_perf context initialise
 * @core_perf: out pointer of the initialised context
 * @dpu_kms: the dpu kms pointer
 *
 * Returns: 0: success, others failed
 */
int dpu_core_perf_init(struct dpu_core_perf **core_perf, struct dpu_kms *dpu_kms);

/**
 * dpu_core_perf_deinit - deinit core perf context
 * @core_perf: the core perf pointer
 */
void dpu_core_perf_deinit(struct dpu_core_perf *core_perf);

#endif
