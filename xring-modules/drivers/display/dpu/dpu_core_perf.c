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
#include <linux/types.h>
#include <linux/of.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/seq_file.h>
#include <linux/kstrtox.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mode.h>
#include <drm/drm_connector.h>

#include <soc/xring/flowctrl.h>
#include <soc/xring/ddr_devfreq.h>
#include <soc/xring/ddr_vote_dev.h>
#include <soc/xring/doze.h>
#include <dt-bindings/xring/platform-specific/pm/include/sys_doze_plat.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h>
#include "dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h"
#include <soc/xring/xr-clk-provider.h>

#include <ddr_dvfs_vote.h>
#include <soc/xring/xr-clk-provider.h>

#include "dpu_hw_tpc_module_ops.h"
#include "dpu_hw_common.h"
#include "dpu_core_perf.h"
#include "dpu_res_mgr.h"
#include "dpu_power_mgr.h"
#include "dpu_clk_mgr.h"
#include "dpu_hw_power_ops.h"
#include "dpu_crtc.h"
#include "dpu_cont_display.h"
#include "dpu_trace.h"
#include "dsi_encoder.h"

static struct dpu_core_perf *g_core_perf;

static u32 g_veu_last_ddr_bandwidth;
static u32 g_dpu_last_ddr_bandwidth;

static void get_dsi_dsc_info(struct drm_device *drm_dev,
		struct dsi_display_info **info)
{
	struct drm_connector_list_iter conn_iter;
	struct drm_connector *connector;

	drm_connector_list_iter_begin(drm_dev, &conn_iter);
	drm_for_each_connector_iter(connector, &conn_iter) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			PERF_DEBUG("get dsi dsc info\n");
			*info = dsi_display_info_get(connector);
			break;
		}
	}
	drm_connector_list_iter_end(&conn_iter);
}

void dpu_core_perf_clk_init(struct dpu_core_perf *core_perf)
{
	struct dsi_display_info *info;
	struct dpu_clk_mgr *clk_mgr;
	u32 init_profile;
	int ret;

	if (!core_perf) {
		PERF_ERROR("invalid parameters\n");
		return;
	}

	get_dsi_dsc_info(core_perf->dpu_kms->drm_dev, &core_perf->dsi_info);
	info = core_perf->dsi_info;
	if (!info) {
		PERF_INFO("invalid dsi display info\n");
		return;
	}

	init_profile = info->boot_up_profile;
	if (core_perf->curr_profile != DPU_CORE_PERF_INVALID_PROFILE_ID) {
		init_profile = core_perf->curr_profile;
		PERF_DEBUG("restore profile:%d\n", init_profile);
	}
	if (init_profile >= DPU_PROFILE_MAX_NUM) {
		DPU_ERROR("why boot up profile is %u\n", init_profile);
		init_profile = DPU_PROFILE_0;
	}

	ret = dpu_core_perf_update(core_perf, DPU_CORE_PERF_CHANNEL_NORMAL,
			init_profile, true, true);
	if (ret)
		DPU_WARN("fatal error, set init mclk rate failed\n");

	clk_mgr = core_perf->power_mgr->clk_mgr;
	if (info->dsc_en) {
		ret = dpu_clk_enable(clk_mgr, CLK_DPU_DSC0);
		if (ret)
			PERF_ERROR("failed to enable clk %s\n",
					dpu_get_clk_name(clk_mgr, CLK_DPU_DSC0));
	}
}

void dpu_core_perf_clk_deinit(struct dpu_core_perf *core_perf)
{
	struct dsi_display_info *info;
	struct dpu_clk_mgr *clk_mgr;

	if (!core_perf) {
		PERF_ERROR("invalid parameters\n");
		return;
	}

	clk_mgr = core_perf->power_mgr->clk_mgr;
	/* reset dpu core clk */
	dpu_clk_reset_clk_rate(clk_mgr);

	info = core_perf->dsi_info;
	if (!info) {
		PERF_INFO("invalid dsi display info\n");
		return;
	}

	if (info->dsc_en)
		dpu_clk_disable(clk_mgr, CLK_DPU_DSC0);

	PERF_DEBUG("clk deinit\n");
}

static int _do_dpu_core_perf_update(struct dpu_core_perf *core_perf,
		u32 profile_id)
{
	struct dpu_power_mgr *power_mgr = core_perf->dpu_kms->power_mgr;
	struct dpu_core_perf_params cfg_perf = {0};
	struct dpu_core_perf_params *curr_perf;

	cfg_perf.mclk_rate = core_perf->profiles[profile_id].mclk_rate;
	cfg_perf.aclk_rate = core_perf->profiles[profile_id].aclk_rate;

	dpu_clk_set_rate(power_mgr->clk_mgr, CLK_DPU_CORE0,
			cfg_perf.mclk_rate);
	core_perf->vote_perf = cfg_perf;

	PERF_DEBUG("core clk %lu, dsc0 clk %lu, axi clk %lu\n",
			dpu_clk_get_rate(power_mgr->clk_mgr, CLK_DPU_CORE0),
			dpu_clk_get_rate(power_mgr->clk_mgr, CLK_DPU_DSC0),
			dpu_clk_get_rate(power_mgr->clk_mgr, CLK_DPU_AXI0));

	curr_perf = &core_perf->curr_perf;
	curr_perf->mclk_rate = dpu_clk_get_rate(power_mgr->clk_mgr, CLK_DPU_CORE0);
	curr_perf->aclk_rate = dpu_clk_get_rate(power_mgr->clk_mgr, CLK_DPU_AXI0);

	trace__do_dpu_core_perf_update("clk update", curr_perf->aclk_rate, curr_perf->mclk_rate);

	if (cfg_perf.aclk_rate > curr_perf->aclk_rate + CLK_MARGIN)
		PERF_ERROR("current aclk rate(%lu) is lower than the required val(%lu)\n",
				curr_perf->aclk_rate, cfg_perf.aclk_rate);

	if (cfg_perf.mclk_rate > curr_perf->mclk_rate + CLK_MARGIN) {
		DPU_ERROR("current mclk rate(%lu) is lower than the required val(%lu)\n",
				curr_perf->mclk_rate, cfg_perf.mclk_rate);
		return -1;
	}

	return 0;
}

int dpu_core_perf_update(struct dpu_core_perf *core_perf, u32 channel_id,
		u32 profile_vote, bool vote_on, bool force)
{
	struct dpu_hw_dvfs_mgr *hw_dvfs_mgr;
	u32 profile = 0;
	int low_temp_status = 0;
	int ret = 0;
	int i;

	if (!core_perf) {
		DPU_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	if (channel_id >= DPU_CORE_PERF_CHANNEL_MAX_NUM) {
		DPU_ERROR("invalid channel id %d\n", channel_id);
		return -EINVAL;
	}

	if (profile_vote >= DPU_PROFILE_MAX_NUM) {
		DPU_WARN("invalid profile %u\n", profile_vote);
		return -EINVAL;
	}

	hw_dvfs_mgr = &core_perf->hw_dvfs_mgr;

	mutex_lock(&core_perf->perf_mutex);

	if (profile_vote == DPU_PROFILE_2 && media_dvs_is_low_temperature()) {
		profile_vote = DPU_PROFILE_1;
		low_temp_status = 1;
		PERF_INFO("enter low temperature status\n");
	}

	if (vote_on) {
		if (profile_vote != core_perf->vote_level[channel_id]) {
			core_perf->vote_level[channel_id] = profile_vote;

			PERF_DEBUG("update channel %d profile %u\n",
					channel_id, profile_vote);
		}
	} else {
		core_perf->vote_level[channel_id] = 0;
		PERF_DEBUG("channel %d vote_off\n", channel_id);
	}

	core_perf->vote_status[channel_id] = vote_on;

	for (i = DPU_CORE_PERF_CHANNEL_DEBUG + 1; i < DPU_CORE_PERF_CHANNEL_MAX_NUM; i++)
		if (core_perf->vote_status[channel_id])
			profile = max(profile, core_perf->vote_level[i]);

	// always set channel default when enable
	if (core_perf->vote_status[DPU_CORE_PERF_CHANNEL_DEBUG])
		profile = core_perf->vote_level[DPU_CORE_PERF_CHANNEL_DEBUG];

	if ((core_perf->curr_profile != profile) || force) {
		PERF_DEBUG("%s switch profile [%d] -> [%d]\n", force ? "force" : "",
				core_perf->curr_profile, profile);

		dpu_hw_dvfs_disable(hw_dvfs_mgr);
		ret = _do_dpu_core_perf_update(core_perf, profile);
		if (ret == 0)
			core_perf->curr_profile = profile;
		dpu_hw_dvfs_enable(hw_dvfs_mgr);
	} else {
		PERF_DEBUG("vote same profile[%d]\n", profile);
	}
	mutex_unlock(&core_perf->perf_mutex);

	return low_temp_status == 0 ? ret : -EFAULT;
}

static int dpu_common_ddr_bandwidth_update(u32 chn_id, u32 expect_ddr_bandwidth, bool block)
{
	unsigned long total_bandwidth = 0;
	unsigned long cur_bandwidth = 0;
	int ret_val = 0;
	int ret;

	if (chn_id == DPU_CH)
		total_bandwidth = expect_ddr_bandwidth + g_veu_last_ddr_bandwidth;
	else
		total_bandwidth = expect_ddr_bandwidth + g_dpu_last_ddr_bandwidth;

	ret = ddr_vote_dev_request(DDR_DVFS_HW_VOTE_DPU, VOTE_DMD_BANDWIDTH_MB_S,
			total_bandwidth);
	if (ret != 0) {
		DPU_DEBUG("ddr(MB/s) vote fail, total %lu, chn %u, last dpu %u, last veu %u\n",
			total_bandwidth, chn_id, g_dpu_last_ddr_bandwidth, g_veu_last_ddr_bandwidth);
		return -EINVAL;
	}

	if (block) {
		/* wait for 1000us timeout, poll current bandwidth per 100us */
		ret = read_poll_timeout(ddr_get_cur_bandwidth, ret_val,
			(ret_val == 0 && cur_bandwidth >= total_bandwidth),
			100, 1000, false, &cur_bandwidth);

		PERF_DEBUG("current ddr %lu MB/s\n", cur_bandwidth);

		if (ret != 0) {
			DPU_DEBUG("cur ddr(MB/s) %lu is lower than request %lu, chn %u, last dpu %u, last veu %u\n",
				cur_bandwidth, total_bandwidth, chn_id, g_dpu_last_ddr_bandwidth, g_veu_last_ddr_bandwidth);
			return -EINVAL;
		}
	}

	if (chn_id == DPU_CH)
		g_dpu_last_ddr_bandwidth = expect_ddr_bandwidth;
	else
		g_veu_last_ddr_bandwidth = expect_ddr_bandwidth;

	return 0;
}

int veu_ddr_bandwidth_update(u32 veu_expect_bandwidth)
{
	unsigned long max_bandwidth = 0;
	int ret = 0;

	PERF_DEBUG("vote ddr %u MB/s\n", veu_expect_bandwidth);

	if (g_veu_last_ddr_bandwidth == veu_expect_bandwidth) {
		PERF_DEBUG("same bandwidth\n");
		return 0;
	}

	if (ddr_get_max_bandwidth(&max_bandwidth)) {
		DPU_INFO("get max ddr bandwidth fail\n");
		max_bandwidth = veu_expect_bandwidth;
	}

	if (veu_expect_bandwidth > max_bandwidth) {
		DPU_DEBUG("%u exceed max ddr bandwidth %lu\n", veu_expect_bandwidth, max_bandwidth);
		veu_expect_bandwidth = max_bandwidth;
	}

	ret = dpu_common_ddr_bandwidth_update(VEU_CH, veu_expect_bandwidth, true);
	if (ret != 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL_GPL(veu_ddr_bandwidth_update);

int dpu_ddr_bandwidth_update(struct dpu_core_perf *core_perf, u32 bandwidth_cmd, u32 channel)
{
	unsigned long max_bandwidth = 0;
	u32 expect_bandwidth = 0;
	bool is_block = true;
	int ret = 0;

	if (!core_perf) {
		DPU_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	if (channel >= DPU_DDR_CHANNEL_MAX_NUM) {
		PERF_ERROR("invalid channel id %d\n", channel);
		return -EINVAL;
	}

	expect_bandwidth = bandwidth_cmd & ~BIT(31);
	is_block = ((bandwidth_cmd >> 31) == 0);

	PERF_DEBUG("vote ddr %u MB/s, isblock %d\n", expect_bandwidth, is_block);

	mutex_lock(&core_perf->ddr_mutex);
	core_perf->ddr_bandwidth[channel] = expect_bandwidth;

	// always set channel debug when enable
	if (channel != DPU_DDR_CHANNEL_DEBUG &&
		core_perf->ddr_bandwidth[DPU_DDR_CHANNEL_DEBUG] > 0) {
		expect_bandwidth = core_perf->ddr_bandwidth[DPU_DDR_CHANNEL_DEBUG];
		is_block = true;
		PERF_DEBUG("vote ddr debug %u MB/s\n", expect_bandwidth);
	}

	if (core_perf->curr_bandwidth == expect_bandwidth) {
		PERF_DEBUG("same bandwidth\n");
		mutex_unlock(&core_perf->ddr_mutex);
		return 0;
	}

	if (ddr_get_max_bandwidth(&max_bandwidth)) {
		PERF_DEBUG("get max ddr bandwidth fail\n");
		max_bandwidth = expect_bandwidth;
	}

	if (expect_bandwidth > max_bandwidth) {
		DPU_DEBUG("%u exceed max ddr bandwidth %lu\n", expect_bandwidth, max_bandwidth);
		expect_bandwidth = max_bandwidth;
	}

	ret = dpu_common_ddr_bandwidth_update(DPU_CH, expect_bandwidth, is_block);
	if (ret != 0) {
		mutex_unlock(&core_perf->ddr_mutex);
		return ret;
	}

	core_perf->curr_bandwidth = expect_bandwidth;
	mutex_unlock(&core_perf->ddr_mutex);

	return 0;
}

static void _do_dpu_doze_ctrl_cfg(bool enable)
{
	if (enable)
		sys_state_doz2nor_unvote(VOTER_DOZAP_DPU);
	else
		sys_state_doz2nor_vote(VOTER_DOZAP_DPU);
}

static void dpu_core_perf_doze_disable(void)
{
	struct dpu_doze_ctrl *doze_ctrl;
	unsigned long flags;

	if (!g_core_perf) {
		PERF_ERROR("g_core_perf uninitialized\n");
		return;
	}

	if (!is_dpu_lp_enabled(DPU_LP_DOZE_ENABLE))
		return;

	doze_ctrl = &g_core_perf->doze_ctrl;

	spin_lock_irqsave(&doze_ctrl->doze_lock, flags);
	doze_ctrl->doze_counter--;
	if (doze_ctrl->doze_status != DPU_DOZE_DISABLED) {
		_do_dpu_doze_ctrl_cfg(false);
		doze_ctrl->doze_status = DPU_DOZE_DISABLED;
	}
	spin_unlock_irqrestore(&doze_ctrl->doze_lock, flags);
}

static void dpu_core_perf_doze_enable(void)
{
	struct dpu_doze_ctrl *doze_ctrl;
	unsigned long flags;

	if (!g_core_perf) {
		PERF_ERROR("g_core_perf uninitialized\n");
		return;
	}

	if (!is_dpu_lp_enabled(DPU_LP_DOZE_ENABLE))
		return;

	doze_ctrl = &g_core_perf->doze_ctrl;

	spin_lock_irqsave(&doze_ctrl->doze_lock, flags);
	doze_ctrl->doze_counter++;
	if ((doze_ctrl->doze_counter == 0) &&
			(doze_ctrl->doze_status != DPU_DOZE_ENABLED)) {
		_do_dpu_doze_ctrl_cfg(true);
		doze_ctrl->doze_status = DPU_DOZE_ENABLED;
	}
	spin_unlock_irqrestore(&doze_ctrl->doze_lock, flags);
}

static void _do_dpu_hw_ppll2_ctrl_cfg(bool enable)
{
	if (enable)
		dpu_ppll2_vote_bypass(DPU_PPLL2_BYPASS);
	else
		dpu_ppll2_vote_bypass(DPU_PPLL2_UNBYPASS);
}

void dpu_core_perf_ppll2_disable(void)
{
	struct dpu_doze_ctrl *doze_ctrl;
	unsigned long flags;

	if (!g_core_perf) {
		PERF_ERROR("g_core_perf uninitialized\n");
		return;
	}

	if (!is_dpu_lp_enabled(DPU_LP_HW_PPLL2_CTRL_ENABLE))
		return;

	doze_ctrl = &g_core_perf->doze_ctrl;

	spin_lock_irqsave(&doze_ctrl->ppll2_lock, flags);
	doze_ctrl->ppll2_counter--;
	if (doze_ctrl->ppll2_status != DPU_DOZE_DISABLED) {
		_do_dpu_hw_ppll2_ctrl_cfg(false);
		doze_ctrl->ppll2_status = DPU_DOZE_DISABLED;
	}
	spin_unlock_irqrestore(&doze_ctrl->ppll2_lock, flags);
}

void dpu_core_perf_ppll2_enable(void)
{
	struct dpu_doze_ctrl *doze_ctrl;
	unsigned long flags;

	if (!g_core_perf) {
		PERF_ERROR("g_core_perf uninitialized\n");
		return;
	}

	if (!is_dpu_lp_enabled(DPU_LP_HW_PPLL2_CTRL_ENABLE))
		return;

	doze_ctrl = &g_core_perf->doze_ctrl;

	spin_lock_irqsave(&doze_ctrl->ppll2_lock, flags);
	doze_ctrl->ppll2_counter++;
	if ((doze_ctrl->ppll2_counter == 1) &&
			(doze_ctrl->ppll2_status != DPU_DOZE_ENABLED)) {
		_do_dpu_hw_ppll2_ctrl_cfg(true);
		doze_ctrl->ppll2_status = DPU_DOZE_ENABLED;
	}
	spin_unlock_irqrestore(&doze_ctrl->ppll2_lock, flags);
}

void dpu_core_perf_prepare(struct dpu_core_perf *perf,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_virt_pipeline *pipeline;
	u32 lowpower_ctrl;
	int index;

	if (!perf) {
		PERF_ERROR("invalid parameter\n");
		return;
	}

	if (!is_dpu_powered_on())
		return;

	index = drm_crtc_index(crtc);
	pipeline = &perf->dpu_kms->virt_pipeline[index];
	lowpower_ctrl = pipeline->power_ctrl.lowpower_ctrl;

	dpu_core_perf_clk_active();

	if (pipeline->is_offline) {
		PERF_DEBUG("offline display\n");
		dpu_hw_dvfs_disable(&perf->hw_dvfs_mgr);
		dpu_core_perf_ppll2_disable();
		dpu_core_perf_doze_disable();

		return;
	}

	if (need_enable_crtc(new_state)) {
		PERF_DEBUG("crtc %d on, lowpower_ctrl is 0x%x\n", index, lowpower_ctrl);
		if (!(lowpower_ctrl & DPU_LP_HW_DVFS_ENABLE))
			dpu_hw_dvfs_disable(&perf->hw_dvfs_mgr);

		if (!(lowpower_ctrl & DPU_LP_HW_PPLL2_CTRL_ENABLE)) {
			dpu_core_perf_ppll2_disable();
			PERF_DEBUG("disable ppll2\n");
		}

		if (!(lowpower_ctrl & DPU_LP_DOZE_ENABLE)) {
			dpu_core_perf_doze_disable();
			PERF_DEBUG("disable doze\n");
		}
	}

	if (need_disable_crtc(new_state)) {
		PERF_DEBUG("crtc %d off, lowpower_ctrl is 0x%x\n", index, lowpower_ctrl);
		if (lowpower_ctrl & DPU_LP_HW_DVFS_ENABLE)
			dpu_hw_dvfs_disable(&perf->hw_dvfs_mgr);

		if (lowpower_ctrl & DPU_LP_HW_PPLL2_CTRL_ENABLE) {
			dpu_core_perf_ppll2_disable();
			PERF_DEBUG("disable ppll2\n");
		}
	}
}

static bool _need_enable_encoder_tmg_doze(struct dpu_virt_pipeline *virt_pipeline,
		struct drm_crtc_state *new_state)
{
	if (virt_pipeline->sw_start_flag && !virt_pipeline->delay_mount_connector)
		return true;

	if (is_cont_display_enabled() && need_enable_crtc(new_state) &&
			is_primary_display(new_state))
		return true;

	return false;
}

void dpu_core_perf_finish(struct dpu_core_perf *perf,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct drm_connector *primary_connector;
	struct dsi_connector *dsi_connector;
	struct dpu_virt_pipeline *pipeline;
	struct dsi_encoder *dsi_encoder;
	struct drm_connector *connector;
	u32 lowpower_ctrl;
	int index;
	u32 mask;

	if (!perf) {
		PERF_ERROR("invalid parameter\n");
		return;
	}
	if (!is_dpu_powered_on())
		return;

	index = drm_crtc_index(crtc);
	pipeline = &perf->dpu_kms->virt_pipeline[index];
	lowpower_ctrl = pipeline->power_ctrl.lowpower_ctrl;

	if (pipeline->is_offline) {
		PERF_DEBUG("offline display\n");
		dpu_hw_dvfs_enable(&perf->hw_dvfs_mgr);
		dpu_core_perf_doze_enable();
		dpu_core_perf_ppll2_enable();

		dpu_core_perf_clk_deactive();
		return;
	}

	if (need_enable_crtc(new_state)) {
		PERF_DEBUG("crtc %d on, lowpower_ctrl is 0x%x\n", index, lowpower_ctrl);
		if (lowpower_ctrl & DPU_LP_HW_DVFS_ENABLE)
			dpu_hw_dvfs_enable(&perf->hw_dvfs_mgr);

		if (lowpower_ctrl & DPU_LP_HW_PPLL2_CTRL_ENABLE) {
			dpu_core_perf_ppll2_enable();
			PERF_DEBUG("enable ppll2\n");
		}
	}

	if (need_disable_crtc(new_state)) {
		PERF_DEBUG("crtc %d off, lowpower_ctrl is 0x%x\n", index, lowpower_ctrl);
		if (!(lowpower_ctrl & DPU_LP_HW_DVFS_ENABLE))
			dpu_hw_dvfs_enable(&perf->hw_dvfs_mgr);

		if (!(lowpower_ctrl & DPU_LP_DOZE_ENABLE)) {
			dpu_core_perf_doze_enable();
			PERF_DEBUG("enable doze\n");
		}

		if (!(lowpower_ctrl & DPU_LP_HW_PPLL2_CTRL_ENABLE)) {
			dpu_core_perf_ppll2_enable();
			PERF_DEBUG("enable ppll2\n");
		}
	}

	if (_need_enable_encoder_tmg_doze(pipeline, new_state)) {
		primary_connector = dsi_primary_connector_get(crtc->dev);
		for_each_connector_per_crtc(connector, new_state, mask) {
			if (connector == primary_connector) {
				dsi_connector = to_dsi_connector(connector);
				dsi_encoder = to_dsi_encoder(dsi_connector->encoder);

				dpu_core_perf_clk_active();
				dsi_encoder_tmg_doze_enable(dsi_encoder);
				dpu_core_perf_clk_deactive();

				PERF_DEBUG("enable tmg doze\n");
			}
		}
	}

	dpu_core_perf_clk_deactive();
}

static int dpu_core_perf_parse_dts(struct dpu_core_perf *perf,
		struct dpu_kms *dpu_kms)
{
	struct dpu_profile_info *item;
	struct drm_device *drm_dev;
	const __be32 *data;
	u32 len;
	u32 num;
	u32 i;

	PERF_DEBUG("Enter\n");

	drm_dev = dpu_kms->drm_dev;

	data = of_get_property(drm_dev->dev->of_node, "profiles", &len);
	if (!data) {
		PERF_ERROR("Failed to get data property\n");
		return -EINVAL;
	}

	num = len / (DPU_CORE_PERF_CLK_NUM * sizeof(__be32));
	PERF_INFO("profile num is %u\n", num);

	if (num == 0 || num > DPU_PROFILE_MAX_NUM) {
		PERF_ERROR("invalid profile num %u\n", num);
		return -EINVAL;
	}

	perf->profiles = kzalloc(num * sizeof(struct dpu_profile_info),
			GFP_KERNEL);
	if (!perf->profiles) {
		PERF_ERROR("malloc memory failed\n");
		return -ENOMEM;
	}

	perf->profile_num = num;
	for (i = 0; i < num; i++) {
		item = &perf->profiles[i];

		item->profile_id = i;
		item->mclk_rate = be32_to_cpu(data[i * DPU_CORE_PERF_CLK_NUM + 0]);
		item->aclk_rate = be32_to_cpu(data[i * DPU_CORE_PERF_CLK_NUM + 1]);
		item->pclk_rate = be32_to_cpu(data[i * DPU_CORE_PERF_CLK_NUM + 2]);
		item->dscclk0_rate = be32_to_cpu(data[i * DPU_CORE_PERF_CLK_NUM + 3]);
		item->dscclk1_rate = be32_to_cpu(data[i * DPU_CORE_PERF_CLK_NUM + 4]);

		PERF_DEBUG("profile %u:\n", item->profile_id);
		PERF_DEBUG("    mclk %lu\n", item->mclk_rate);
		PERF_DEBUG("    aclk %lu\n", item->aclk_rate);
		PERF_DEBUG("    pclk %lu\n", item->pclk_rate);
		PERF_DEBUG("    dsc0 %lu\n", item->dscclk0_rate);
		PERF_DEBUG("    dsc1 %lu\n", item->dscclk1_rate);
	}

	return 0;
}

void dpu_core_perf_update_lp_ctrl(struct dpu_core_perf *perf,
		u32 lp_ctrl)
{
	if (perf->perf_ctrl.lp_ctrl != lp_ctrl) {
		PERF_INFO("old lp_ctrl 0x%x, new lp_ctrl 0x%x\n",
				perf->perf_ctrl.lp_ctrl, lp_ctrl);
		PERF_INFO("need do dpu power off -> on to make them works\n");

		perf->perf_ctrl.lp_ctrl = lp_ctrl;
	}
}

bool is_dpu_lp_enabled(u32 feature)
{
	struct dpu_core_perf *perf = g_core_perf;

	/* sram lp depends on auto cg enable */
	if (feature == DPU_LP_SRAM_LP_ENABLE)
		return !!(perf->perf_ctrl.lp_ctrl & DPU_LP_AUTO_CG_ENABLE);
	else
		return !!(perf->perf_ctrl.lp_ctrl & feature);
}

static void dpu_core_perf_lp_ctrl_init(struct dpu_core_perf *perf)
{
	struct dpu_core_perf_ctrl *perf_ctrl = &perf->perf_ctrl;
	struct dpu_core_perf_tune *perf_tune = &perf_ctrl->perf_tune;

	perf_ctrl->lp_ctrl =
			DPU_LP_SRAM_LP_ENABLE |
			DPU_LP_AUTO_CG_ENABLE |
			DPU_LP_HW_DVFS_ENABLE |
			DPU_LP_DOZE_ENABLE |
			DPU_LP_TOP_AUTO_CG_ENABLE |
			DPU_LP_IDLE_CTRL_ENABLE |
			DPU_LP_HW_ULPS;

	PERF_INFO("lp_ctrl is 0x%x\n", perf_ctrl->lp_ctrl);

	perf_tune->min_mclk_rate = perf->profiles[0].mclk_rate;
	perf_tune->max_mclk_rate = perf->profiles[perf->profile_num - 1].mclk_rate;
	perf_tune->max_aclk_rate = perf->profiles[perf->profile_num - 1].aclk_rate;
}

void dpu_core_perf_sw_reset(struct dpu_core_perf *core_perf)
{
	int i;

	if (!core_perf) {
		PERF_ERROR("invalid parameter\n");
		return;
	}

	core_perf->curr_bandwidth = 0;
	core_perf->curr_profile = DPU_CORE_PERF_INVALID_PROFILE_ID;
	memset(&core_perf->vote_perf, 0, sizeof(struct dpu_core_perf_params));
	memset(&core_perf->curr_perf, 0, sizeof(struct dpu_core_perf_params));

	for (int i = 0; i < DPU_DDR_CHANNEL_MAX_NUM; i++)
		core_perf->ddr_bandwidth[i] = 0;

	for (i = 0; i < DPU_CORE_PERF_CHANNEL_MAX_NUM; i++) {
		core_perf->vote_level[i] = 0;
		core_perf->vote_status[i] = false;
	}
}

void dpu_core_perf_hw_init(struct dpu_core_perf *core_perf,
		struct dpu_power_ctrl_info *ctrl_info,
		u32 part_id)
{
	struct dpu_hw_init_cfg hw_init_cfg = {0};

	if (part_id >= DPU_PARTITION_MAX_NUM) {
		PERF_ERROR("invalid parameter %u\n", part_id);
		return;
	}

	/* disable auto cg to make sure tpc configure is effect,
	 * in discontinuous display case, must enable auto cg through perf_hw_init
	 * to avoid making other register lose effectiveness
	 */
	dpu_hw_auto_cg_disable(part_id);
	dpu_hw_tpc_module_cfg(part_id);

	if (is_cont_display_enabled()) {
		if (is_dpu_lp_enabled(DPU_LP_AUTO_CG_ENABLE))
			dpu_hw_auto_cg_enable(part_id);
		PERF_INFO("continues display enabled\n");
		return;
	}

	if (is_dpu_lp_enabled(DPU_LP_AUTO_CG_ENABLE))
		hw_init_cfg.auto_cg_cfg = ENABLE_AUTO_CG;
	else
		hw_init_cfg.auto_cg_cfg = DISABLE_AUTO_CG;

	if (is_dpu_lp_enabled(DPU_LP_SRAM_LP_ENABLE))
		hw_init_cfg.sram_lp_cfg = ENABLE_SRAM_LP;
	else
		hw_init_cfg.sram_lp_cfg = DISABLE_SRAM_LP;

	PERF_DEBUG("part_id %d: auto_cg %d, sram_lp %d\n",
			part_id,
			hw_init_cfg.auto_cg_cfg,
			hw_init_cfg.sram_lp_cfg);

	dpu_hw_do_hw_init(part_id, &hw_init_cfg);
}

void dpu_core_perf_hw_deinit(struct dpu_core_perf *core_perf,
		u32 part_id)
{
	struct dpu_top_cg_mgr *top_cg_mgr;

	if (!core_perf || part_id >= DPU_PARTITION_MAX_NUM) {
		PERF_ERROR("invalid parameter %pK, %u\n", core_perf, part_id);
		return;
	}

	if (part_id == DPU_PARTITION_0)
		return;

	top_cg_mgr = &core_perf->top_cg_mgr;

	dpu_hw_auto_cg_reset(part_id);
}

static int dpu_core_perf_doze_ctrl_init(struct dpu_core_perf *core_perf,
		struct dpu_kms *dpu_kms)
{
	struct dpu_doze_ctrl *doze_ctrl = &core_perf->doze_ctrl;

	spin_lock_init(&doze_ctrl->ppll2_lock);
	spin_lock_init(&doze_ctrl->doze_lock);
	doze_ctrl->ppll2_status = DPU_DOZE_UNKNOWN;
	doze_ctrl->doze_status = DPU_DOZE_UNKNOWN;

	return 0;
}

static void dpu_core_perf_doze_ctrl_deinit(struct dpu_core_perf *core_perf)
{

}

static void _dpu_top_cg_disable(struct dpu_top_cg_mgr *top_cg_mgr)
{
	unsigned long flags;

	spin_lock_irqsave(&top_cg_mgr->lock, flags);
	top_cg_mgr->vote_counter--;
	if (top_cg_mgr->status != DPU_AUTO_CG_DISABLED) {
		if (is_dpu_powered_on()) {
			dpu_hw_top_clk_auto_cg_cfg(DISABLE_AUTO_CG);
			PERF_DEBUG("disable auto cg\n");
			top_cg_mgr->status = DPU_AUTO_CG_DISABLED;
		} else {
			PERF_DEBUG("dpu is power down\n");
		}
	}
	spin_unlock_irqrestore(&top_cg_mgr->lock, flags);
}

void dpu_core_perf_top_cg_disable(void)
{
	if (!g_core_perf) {
		PERF_ERROR("g_core_perf is NULL\n");
		return;
	}

	if (!is_dpu_lp_enabled(DPU_LP_TOP_AUTO_CG_ENABLE))
		return;

	_dpu_top_cg_disable(&g_core_perf->top_cg_mgr);
}

static void _dpu_top_cg_enable(struct dpu_top_cg_mgr *top_cg_mgr)
{
	unsigned long flags;

	spin_lock_irqsave(&top_cg_mgr->lock, flags);
	top_cg_mgr->vote_counter++;
	if ((top_cg_mgr->vote_counter == 0) &&
			(top_cg_mgr->status != DPU_AUTO_CG_ENABLED)) {
		if (is_dpu_powered_on()) {
			dpu_hw_top_clk_auto_cg_cfg(ENABLE_AUTO_CG);
			PERF_DEBUG("enable auto cg\n");
			top_cg_mgr->status = DPU_AUTO_CG_ENABLED;
		} else {
			PERF_DEBUG("dpu is power down\n");
		}
	}
	spin_unlock_irqrestore(&top_cg_mgr->lock, flags);
}

void dpu_core_perf_top_cg_enable(void)
{
	if (!g_core_perf) {
		PERF_ERROR("g_core_perf is NULL\n");
		return;
	}

	if (!is_dpu_lp_enabled(DPU_LP_TOP_AUTO_CG_ENABLE))
		return;

	_dpu_top_cg_enable(&g_core_perf->top_cg_mgr);
}

void dpu_core_perf_clk_active(void)
{
	dpu_core_perf_top_cg_disable();
	dpu_core_perf_ppll2_disable();
}

void dpu_core_perf_clk_deactive(void)
{
	dpu_core_perf_ppll2_enable();
	dpu_core_perf_top_cg_enable();
}

static int dpu_top_cg_ctrl_init(struct dpu_core_perf *core_perf)
{
	struct dpu_top_cg_mgr *top_cg_mgr;

	if (!core_perf) {
		PERF_ERROR("invalid parameter %pK\n", core_perf);
		return -EINVAL;
	}

	top_cg_mgr = &core_perf->top_cg_mgr;
	spin_lock_init(&top_cg_mgr->lock);

	top_cg_mgr->vote_counter = 0;
	top_cg_mgr->status = DPU_AUTO_CG_UNKNOWN;

	return 0;
}

int dpu_core_perf_init(struct dpu_core_perf **core_perf, struct dpu_kms *dpu_kms)
{
	struct dpu_core_perf *perf = NULL;
	DPU_IOMEM dpu_base;
	int ret;

	if (!dpu_kms) {
		PERF_ERROR("invalid parameter %pK\n", dpu_kms);
		return -EINVAL;
	}

	perf = kzalloc(sizeof(*perf), GFP_KERNEL);
	if (!perf)
		return -ENOMEM;

	perf->dpu_kms = dpu_kms;
	perf->power_mgr = dpu_kms->power_mgr;
	mutex_init(&perf->perf_mutex);
	mutex_init(&perf->ddr_mutex);

	ret = dpu_core_perf_parse_dts(perf, dpu_kms);
	if (ret) {
		PERF_ERROR("failed to parse core perf strategy\n");
		goto err;
	}

	dpu_core_perf_lp_ctrl_init(perf);
	ret = dpu_hw_dvfs_mgr_init(&perf->hw_dvfs_mgr, dpu_kms);
	if (ret) {
		PERF_ERROR("failed to do hw dvfs mgr init\n");
		goto err;
	}

	ret = dpu_top_cg_ctrl_init(perf);
	if (ret) {
		PERF_ERROR("failed to do auto cg init\n");
		goto err_auto_cg;
	}

	dpu_base = dpu_kms->res_mgr_ctx->dpu_cap.dpu_mem.base;
	dpu_hw_init_module_init(dpu_base);

	ret = dpu_core_perf_doze_ctrl_init(perf, dpu_kms);
	if (ret) {
		PERF_ERROR("failed to do doze ctrl init\n");
		goto err_doze_ctrl;
	}

	dpu_core_perf_sw_reset(perf);

	*core_perf = perf;
	g_core_perf = perf;

	return 0;

err_doze_ctrl:
	dpu_hw_init_module_deinit();
err_auto_cg:
	dpu_hw_dvfs_mgr_deinit(&perf->hw_dvfs_mgr);
err:
	kfree(perf);
	return ret;
}

void dpu_core_perf_deinit(struct dpu_core_perf *core_perf)
{
	if (!core_perf) {
		PERF_ERROR("invalid parameter\n");
		return;
	}
	g_core_perf = NULL;
	dpu_core_perf_doze_ctrl_deinit(core_perf);
	dpu_hw_init_module_deinit();
	dpu_hw_dvfs_mgr_deinit(&core_perf->hw_dvfs_mgr);
	mutex_destroy(&core_perf->perf_mutex);
	mutex_destroy(&core_perf->ddr_mutex);

	kfree(core_perf->profiles);
	kfree(core_perf);
}
