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
#include <linux/backlight.h>
#include <drm/drm_connector.h>

#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>
#include <soc/xring/trace_hook_set.h>

#include "dsi_power.h"
#include "dpu_hw_power_ops.h"
#include "dpu_cont_display.h"
#include "dsi_display.h"
#include "dsi_connector.h"

/* TODO: move power manager instance to dsi display */
static struct dsi_power_mgr *g_dsi_power_mgr;

static void set_clk_dsi_sys_rate(struct clk *clk_dsi_sys)
{
	unsigned long clk_rate = 0;
	int ret;

	DSI_DEBUG("clk info: marco: %s, name: %s, rate: %lu\n",
		"XR_CLK_DSI_SYS", "clk_dsi_sys", F1_CLK_DSI_SYS_208_M_9);

	ret = clk_set_rate(clk_dsi_sys, F1_CLK_DSI_SYS_208_M_9);

	if (ret < 0) {
		DSI_ERROR("failed to set clk_dsi_sys to %lu, err:%d\n",
			F1_CLK_DSI_SYS_208_M_9, ret);
		return;
	}

	clk_rate = clk_get_rate(clk_dsi_sys);
	DSI_DEBUG("get clk_dsi_sys rate: %lu\n", clk_rate);
}

static int dsi_backup_ipi_clk_set_rate(struct clk *clk_dpu_pll,
		unsigned long rate,
		enum dsi_ipi_clk_mode mode)
{
	unsigned long clk_rate = 0;
	u64 base_rate, rate_tmp;
	u32 div = 0;
	int ret;

	base_rate = rate;
	rate_tmp = base_rate;
	while (rate_tmp < DPU_PLL_MIN) {
		div += 1;
		rate_tmp = rate_tmp + base_rate;
	}

	DSI_DEBUG("dest rate %llu, div %d, base rate %lu\n",
		rate_tmp, div, rate);

	if (rate_tmp > DPU_PLL_MAX) {
		DSI_ERROR("unsupported pixel clk rate(%llu)", rate_tmp);
		return -ENOTSUPP;
	}

	ret = clk_set_rate(clk_dpu_pll, rate_tmp);
	if (ret < 0) {
		DSI_ERROR("failed to set clk_dpu_pll to %lu, err:%d\n",
			rate, ret);
		return ret;
	}

	clk_rate = clk_get_rate(clk_dpu_pll);
	DSI_DEBUG("get clk_dpu_pll rate: %lu\n", clk_rate);

	ret = clk_prepare_enable(clk_dpu_pll);
	if (ret < 0) {
		DSI_ERROR("failed to enable clk_dpu_pll clk\n");
		return ret;
	}

	dsi_hw_set_backup_ipi_gt_div(div, mode);

	dsi_set_ipi_mux_backup(mode);

	return 0;
}

static void dsi_power_down(void)
{
	struct dsi_display_info *dsi_info;

	set_sr_keypoint(SR_KEYPOINT_DPU_OFF);
	dsi_info = &g_dsi_power_mgr->display->display_info;

	clk_disable_unprepare(g_dsi_power_mgr->clk_dsi_sys);

	if (dsi_info->ipi_pll_sel == DSI_PIXEL_CLOCK_DPU_PLL) {
		DSI_DEBUG("ipi clk use dpu pll\n");
		clk_disable_unprepare(g_dsi_power_mgr->clk_dpu_pll);
	}

	if (dsi_info->is_dual_port) {
		dsi_clk_dpu_dsi_ipi_gt_off(0);
		dsi_clk_dpu_dsi_ipi_gt_off(1);
	} else {
		dsi_clk_dpu_dsi_ipi_gt_off(dsi_info->dsi_id);
	}

	dsi_hw_power_down();
}

static int dsi_ipi_clk_mode_get(u32 dsi_id, bool is_dual_port)
{
	struct dsi_display_info *dsi_info;
	enum dsi_ipi_clk_mode mode;

	dsi_info = &g_dsi_power_mgr->display->display_info;

	if (dsi_info->ipi_pll_sel == DSI_PIXEL_CLOCK_DPU_PLL) {
		DSI_DEBUG("use dsi backup ipi_clk mode %d\n", dsi_info->ipi_pll_sel);
		if (is_dual_port)
			mode = DSI_IPI_BACKUP_DUAL_PORT;
		else
			mode = (dsi_id == 0) ? DSI_IPI_BACKUP_PORT0 : DSI_IPI_BACKUP_PORT1;
	} else {
		if (is_dual_port)
			mode = DSI_IPI_MAIN_DUAL_PORT;
		else
			mode = (dsi_id == 0) ? DSI_IPI_MAIN_PORT0 : DSI_IPI_MAIN_PORT1;
	}

	return mode;
}

static void dsi_power_up(void)
{
	struct dsi_display_info *dsi_info;
	enum dsi_ipi_clk_mode mode;
	int ret = 0;

	set_sr_keypoint(SR_KEYPOINT_DPU_ON);
	dsi_info = &g_dsi_power_mgr->display->display_info;

	DSI_DEBUG("clk_prepare_enable clk_dsi_sys");
	ret = clk_prepare_enable(g_dsi_power_mgr->clk_dsi_sys);
	if (ret < 0) {
		DSI_ERROR("failed to enable clk_dsi_sys clk\n");
		return;
	}

	dsi_hw_power_up();

	set_clk_dsi_sys_rate(g_dsi_power_mgr->clk_dsi_sys);

	mode = dsi_ipi_clk_mode_get(dsi_info->dsi_id, dsi_info->is_dual_port);

	if (dsi_info->is_dual_port) {
		dsi_clk_dpu_dsi_ipi_gt_off(0);
		dsi_clk_dpu_dsi_ipi_gt_off(1);
	} else {
		dsi_clk_dpu_dsi_ipi_gt_off(dsi_info->dsi_id);
	}

	if (dsi_info->ipi_pll_sel == DSI_PIXEL_CLOCK_PHY_PLL)
		dsi_hw_set_main_ipi_mux(mode);
	else
		dsi_backup_ipi_clk_set_rate(g_dsi_power_mgr->clk_dpu_pll,
				dsi_info->pixel_clk_rate, mode);

	if (dsi_info->is_dual_port) {
		dsi_clk_dpu_dsi_ipi_gt_on(0);
		dsi_clk_dpu_dsi_ipi_gt_on(1);
	} else {
		dsi_clk_dpu_dsi_ipi_gt_on(dsi_info->dsi_id);
	}
}

void dsi_power_get(struct drm_connector *connector)
{
	struct dsi_power_mgr *power_mgr = g_dsi_power_mgr;
	struct dsi_connector *dsi_connector;

	if (!power_mgr || !connector) {
		DSI_ERROR("invalid parameters, %pK, %pK\n",
				power_mgr, connector);
		return;
	}

	dsi_connector = to_dsi_connector(connector);

	DSI_DEBUG("dsi power count is %d\n", power_mgr->count);

	mutex_lock(&power_mgr->lock);
	power_mgr->count++;
	if (power_mgr->count == 1) {
		if (!is_cont_display_enabled())
			dsi_power_up();
		DSI_DEBUG("dsi power on\n");
	}

	mutex_unlock(&power_mgr->lock);
}

void dsi_power_prepare_put(struct drm_connector *connector)
{
	struct dsi_power_mgr *power_mgr = g_dsi_power_mgr;
	struct dsi_connector *dsi_connector;
	int ret;

	if (!power_mgr || !connector) {
		DSI_ERROR("invalid parameters, %pK, %pK\n",
				power_mgr, connector);
		return;
	}

	dsi_connector = to_dsi_connector(connector);

	if (dsi_connector_is_cmd_mode(connector)) {
		dsi_display_esd_ctrl(dsi_connector->display, false);
		ret = dsi_panel_disable(dsi_connector->display->panel);
		if (ret) {
			DSI_ERROR("failed to disable cmd panel\n");
			return;
		}
	}
}

bool dsi_power_put(struct drm_connector *connector)
{
	struct dsi_power_mgr *power_mgr = g_dsi_power_mgr;
	struct dsi_connector *dsi_connector;
	bool is_power_down = false;
	int ret;

	if (!power_mgr || !connector) {
		DSI_ERROR("invalid parameters, %pK, %pK\n",
				power_mgr, connector);
		return is_power_down;
	}

	if (WARN_ON(power_mgr->count == 0))
		return is_power_down;

	dsi_connector = to_dsi_connector(connector);

	DSI_DEBUG("dsi power count is %d\n", power_mgr->count);

	mutex_lock(&power_mgr->lock);
	power_mgr->count--;
	if (power_mgr->count == 0) {
		/* cmd mode enter ulps for aod */
		if (dsi_connector_is_cmd_mode(connector)) {
			ret = dsi_display_ulps_ctrl(power_mgr->display, ENTER_ULPS, USE_CPU, 0);
			if (ret)
				DSI_INFO("ulps error when dsi power off, ret:%d\n", ret);
		}

		dsi_power_down();
		DSI_DEBUG("dsi power off\n");
		is_power_down = true;
	}
	mutex_unlock(&power_mgr->lock);
	return is_power_down;
}

bool is_dsi_powered_on(void)
{
	struct dsi_power_mgr *power_mgr = g_dsi_power_mgr;

	if (!power_mgr) {
		DSI_ERROR("invalid parameters\n");
		return false;
	}

	if (power_mgr->count > 0)
		return true;
	else
		return false;
}

int dsi_power_mgr_init(struct dsi_display *display)
{
	struct dsi_power_mgr *dsi_power_mgr;
	int ret = 0;

	if (!display) {
		DSI_ERROR("failed to init dsi power mgr\n");
		return -EINVAL;
	}

	dsi_power_mgr = kzalloc(sizeof(struct dsi_power_mgr), GFP_KERNEL);
	if (!dsi_power_mgr) {
		DSI_ERROR("dsi_power_mgr alloc failed!\n");
		return -ENOMEM;
	}

	dsi_power_mgr->count = 0;
	mutex_init(&dsi_power_mgr->lock);

	dsi_power_mgr->clk_dsi_sys =
			devm_clk_get(&display->pdev->dev, "clk_dsi_sys");
	if (IS_ERR(dsi_power_mgr->clk_dsi_sys)) {
		DSI_ERROR("failed to get clk_dsi_sys clk\n");
		ret = -EINVAL;
		goto error;
	}

	if (is_cont_display_enabled()) {
		DSI_DEBUG("clk_prepare_enable clk_dsi_sys");
		ret = clk_prepare_enable(dsi_power_mgr->clk_dsi_sys);
		if (ret < 0) {
			DSI_ERROR("failed to enable clk_dsi_sys clk\n");
			goto error_release_clk_dsi_sys;
		}
	}

	dsi_power_mgr->clk_dpu_pll =
			devm_clk_get(&display->pdev->dev, "clk_dpu_pll");
	if (IS_ERR(dsi_power_mgr->clk_dpu_pll)) {
		DSI_ERROR("failed to get clk_dpu_pll clk\n");
		ret = -EINVAL;
		goto error_unprepare_clk_dsi_sys;
	}

	if ((display->display_info.ipi_pll_sel == DSI_PIXEL_CLOCK_DPU_PLL) &&
			is_cont_display_enabled()) {
		DSI_DEBUG("clk_prepare_enable clk_dpu_pll");
		ret = clk_prepare_enable(dsi_power_mgr->clk_dpu_pll);
		if (ret < 0) {
			DSI_ERROR("failed to enable clk_dpu_pll clk\n");
			goto error_release_clk_dpu_pll;
		}
	}
	dsi_power_mgr->display = display;
	g_dsi_power_mgr = dsi_power_mgr;
	return 0;
error_release_clk_dpu_pll:
	devm_clk_put(&display->pdev->dev, dsi_power_mgr->clk_dpu_pll);
error_unprepare_clk_dsi_sys:
	clk_disable_unprepare(dsi_power_mgr->clk_dsi_sys);
error_release_clk_dsi_sys:
	devm_clk_put(&display->pdev->dev, dsi_power_mgr->clk_dsi_sys);
error:
	kfree(dsi_power_mgr);
	return ret;
}

void dsi_power_mgr_deinit(void)
{
	if (g_dsi_power_mgr) {
		mutex_destroy(&g_dsi_power_mgr->lock);

		kfree(g_dsi_power_mgr);

		g_dsi_power_mgr = NULL;
	}
}
