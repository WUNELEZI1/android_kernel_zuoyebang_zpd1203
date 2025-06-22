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

#ifndef _DPU_COMMON_INFO_H_
#define _DPU_COMMON_INFO_H_

#include "osal.h"
#include "dpu_hw_dsc_ops.h"
#include "dpu_hw_dsi.h"
#include "dpu_intr.h"
#include "litexdm.h"

#define ASIC 0
#define FPGA 1

enum backlight_set_type {
	BACKLIGHT_SET_BY_I2C,
	BACKLIGHT_SET_BY_DSI,
	BACKLIGHT_SET_MAX,
};

/**
 * dpu_panel_info - base panel info
 * @connector_id: connector id,maybe dsi or wb
 * @external_connector_id: alternative connector id for this panel or virtual panel
 * @type: diplay type, cmd or video
 * @product_type: product type, fold panel
 * @xres: horizontal resolution
 * @yres: vertical resolution
 * @width: physical width
 * @height: physical height
 * @bpp: byte per pixel for panel
 * @bl_type: bl set type, default mipi
 * @bl_min: min backlight level supported
 * @bl_max: max backlight level supported
 * @bl_default: default backlight level
 * @fps: frames per second,default 60hz
 * @is_asic: the hardware platform is fpga or asic
 * @dsc_cfg: panel dsc cfg
 * @dsi_intr_ids: pointer to dsi connector interrupt ids
 * @wb_intr_ids: pointer to wb connector interrupt ids
 * @brightness_max_level：panel max brightness
 * @brightness_min_level：panel min brightness
 * @brightness_init_level：panel initial brightness
 * @backlight_setting_type：set backlight type i2c , dsi and so on
 * @bootup_profile: the panel's bootup profile
 * @lowpower_ctrl：the panel's lowpower ctrl
 * @ipi_pll_sel: ipi pll
 */
struct dpu_panel_info {
	const char *name;
	uint32_t panel_id;
	uint32_t connector_id;
	int32_t external_connector_id;
	uint32_t port_mask;
	enum dsi_ctrl_mode display_type;
	uint32_t product_type;
	uint32_t xres;
	uint32_t yres;
	uint32_t width;
	uint32_t height;
	uint32_t bpp;
	uint32_t bl_type;
	uint32_t bl_min;
	uint32_t bl_max;
	uint32_t bl_default;
	uint32_t fps;
	bool is_asic;
	uint32_t phy_lp_speed;
	uint32_t phy_lane_rate;
	uint32_t pixel_clk_rate;
	uint32_t pixel_clk_div;
	uint32_t phy_amplitude;
	uint32_t phy_oa_setr;
	uint32_t phy_eqa;
	uint32_t phy_eqb;
	uint32_t sys_clk_rate;

	bool bta_en;
	bool eotp_en;

	enum dsi_phy_type phy_type;
	enum dsi_phy_clk_type phy_clk_type;
	enum video_mode_type burst_mode;
	uint32_t lane_num;

	bool dual_port;
	bool dsc_en;
	struct dsc_parms dsc_cfg;

	struct dsi_intr_state *dsi_intr_ids;
	struct wb_intr_state *wb_intr_ids;

	/* backlight info */
	u32 brightness_max_level;
	u32 brightness_min_level;
	u32 brightness_init_level;
	enum backlight_set_type backlight_setting_type;
	char *bias_ic_name;

	u32 bootup_profile;
	u32 lowpower_ctrl;
	u32 ipi_pll_sel;
};

#endif
