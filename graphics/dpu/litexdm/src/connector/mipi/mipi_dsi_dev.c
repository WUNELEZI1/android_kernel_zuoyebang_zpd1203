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

#include "dpu_pipeline.h"
#include "dpu_conn_mgr.h"
#include "platform_device.h"
#include "pipeline_hw_res.h"
#include "dpu_hw_dsi.h"
#include "dsi_hw_ctrl_ops.h"
#include "dsi_hw_tmg_ops.h"
#include "dsi_hw_phy_ops.h"
#include "dpu_hw_post_pipe_top_ops.h"
#include "dsi_hw_ctrl.h"
#include "dpu_hw_power_ops.h"
#include "dpu_res_mgr.h"
#include "mipi_dsi_dev.h"
#include "dpu_log.h"
#include "panel_mgr.h"
#include "panel.h"
#include "dpu_lcd_type.h"
#include "dpu_comp_mgr.h"
#include "dpu_conn_mgr.h"

#define MIPI_INIT_STEP_0	  (0)
#define MIPI_INIT_STEP_1	  (1)
#define MIPI_INIT_STEP_2	  (2)
#define MIPI_INIT_STEP_3	  (3)
#define MIPI_INIT_STEP_4	  (4)
#define MIPI_INIT_STEP_5	  (5)

enum CONN_BLK_TYPE {
	BLK_PHY,
	BLK_DSI_SCTRL,
	BLK_DSI_CTRL,
	BLK_TMG,
};

void get_dsi_ctrl_hw(struct connector *connector, struct dsi_ctrl_hw_blk *hw)
{
	hw->ctrl_blk = &connector->dsi_hw_res.ctrl_blk;
	hw->dctrl_blk = &connector->dsi_hw_res.ctrl_blk;
	hw->sctrl_blk = &connector->dsi_hw_res.sctrl_blk;
	hw->dsctrl_blk = &connector->dsi_hw_res.sctrl_blk;
	hw->phy_blk = &connector->dsi_hw_res.phy_blk;
}

static inline void dsi_ctrl_get_dsi_msg(struct dsi_cmd_desc *cmd_desc,
		struct dsi_msg *msg)
{
	msg->channel = cmd_desc->msg.channel;
	msg->type = cmd_desc->msg.type;
	msg->flags = cmd_desc->msg.flags;

	msg->tx_len = cmd_desc->msg.tx_len;
	msg->tx_buf = cmd_desc->msg.tx_buf;

	msg->rx_len = cmd_desc->msg.rx_len;
	msg->rx_buf = cmd_desc->msg.rx_buf;
}

static int dsi_ctrl_read_cmd_desc(struct dsi_ctrl_hw_blk *hw,
		struct dsi_cmd_desc *cmd_desc)
{
	struct dsi_msg dsi_msg;
	const u8 *tx;
	int ret;

	dsi_ctrl_get_dsi_msg(cmd_desc, &dsi_msg);

	if (dsi_hw_ctrl_cri_is_busy(hw, DEFAULT_WAIT_TIMEOUT_MS)) {
		tx = dsi_msg.tx_buf;
		dpu_pr_err("wait cri busy timeout, cnt %zu, 0x%x\n",
			dsi_msg.tx_len, tx[0]);
	}

	ret = dsi_hw_ctrl_read_pkt_pre_tx(hw, &dsi_msg);
	if (ret) {
		dpu_pr_err("failed to send read_pkt_pre_tx\n");
		return -1;
	}

	ret = dsi_hw_ctrl_read_pkt(hw, &dsi_msg, DEFAULT_WAIT_TIMEOUT_MS);
	if (ret) {
		dpu_pr_err("failed to read pkt\n");
		return -1;
	}

	return 0;
}

static int dsi_ctrl_send_cmd_desc(struct dsi_ctrl_hw_blk *hw,
		struct dsi_cmd_desc *cmd_desc)
{
	struct dsi_msg dsi_msg;
	const u8 *tx;
	int ret;

	dsi_ctrl_get_dsi_msg(cmd_desc, &dsi_msg);

	if (dsi_hw_ctrl_cri_is_busy(hw, DEFAULT_WAIT_TIMEOUT_MS)) {
		tx = dsi_msg.tx_buf;
		dpu_pr_err("wait cri busy timeout, cnt %zu, 0x%x, 0x%x, 0x%x, 0x%x\n",
			dsi_msg.tx_len, tx[0], tx[1], tx[2], tx[3]);
	}

	ret = dsi_hw_ctrl_send_pkt(hw, &dsi_msg);
	if (ret) {
		dpu_pr_err("failed to send pkt\n");
		return ret;
	}

	return 0;
}

int dsi_ctrl_cmd_desc_transfer(struct dsi_ctrl_hw_blk *hw,
		struct dsi_cmd_desc *cmd_desc)
{
	int ret = -1;

	if (!cmd_desc->msg.rx_len && !cmd_desc->msg.tx_len) {
		dpu_pr_err("error message format\n");
		return -1;;
	}

	if (cmd_desc->msg.rx_len > 0) {
		ret = dsi_ctrl_read_cmd_desc(hw, cmd_desc);
		if (ret)
			dpu_pr_err("read message failed, ret=%d\n", ret);
	} else {
		ret = dsi_ctrl_send_cmd_desc(hw, cmd_desc);
		if (ret)
			dpu_pr_err("send message failed, ret=%d\n", ret);
	}

	return ret;
}

void display_to_dsi(struct connector *connector, struct dsi_ctrl_cfg *ctrl)
{
	struct dpu_panel_info *pinfo;
	struct panel_timing *timing;
	bool dsc_en = false;

	pinfo = connector->pinfo;
	timing = &connector->timing;
	dpu_check_and_no_retval(!timing, "timing is null\n");
	dpu_check_and_no_retval(!connector->pinfo, "connector->pinfo is null\n");
	dsc_en = connector->pinfo->dsc_cfg.dsc_en;

	ctrl->ctrl_mode = pinfo->display_type;
	ctrl->manual_mode_en = 0;
	ctrl->auto_calc_en = 1;
	ctrl->vg_en = 0;
	ctrl->vg_dpmode = 0;
	ctrl->fps = timing->framerate;
	ctrl->ipi_clk = pinfo->pixel_clk_rate;
	ctrl->ipi_clk_div = pinfo->pixel_clk_div;
	ctrl->phy_parms.lanes_num = pinfo->lane_num;
	ctrl->phy_parms.phy_type = pinfo->phy_type;
	ctrl->phy_parms.sys_clk = pinfo->sys_clk_rate;
	ctrl->phy_parms.phy_lp_speed = pinfo->phy_lp_speed;
	ctrl->phy_parms.phy_hs_speed = pinfo->phy_lane_rate;
	ctrl->phy_parms.clk_type = pinfo->phy_clk_type;
	ctrl->phy_parms.phy_sel = connector->phy_sel;
	ctrl->phy_parms.ppi_width = connector->ppi_width;
	ctrl->phy_parms.phy_amplitude = pinfo->phy_amplitude;
	ctrl->phy_parms.phy_oa_setr = pinfo->phy_oa_setr;
	ctrl->phy_parms.phy_eqa = pinfo->phy_eqa;
	ctrl->phy_parms.phy_eqb = pinfo->phy_eqb;

	ctrl->bta_en = pinfo->bta_en;
	ctrl->eotp_en = pinfo->eotp_en;
	ctrl->virtual_channel = 0;

	ctrl->video_mode_type = pinfo->burst_mode;
	ctrl->pixel_fomat = dsc_en ? DSI_FMT_DSC : DSI_FMT_RGB888;

	ctrl->timing.hsa = timing->hpw;
	ctrl->timing.hbp = timing->hbp;
	ctrl->timing.hfp = timing->hfp;
	ctrl->timing.hact = timing->hdisplay;

	ctrl->timing.vsa = timing->vpw;
	ctrl->timing.vbp = timing->vbp;
	ctrl->timing.vfp = timing->vfp;
	ctrl->timing.vact = timing->vdisplay;

	ctrl->tear.tear_scanline = 0;
	ctrl->tear.tear_on = 0;
	ctrl->tear.te_type = 0;
	ctrl->tear.auto_tear_bta_disable = 0;

	dpu_pr_debug("display_type: %d\n", pinfo->display_type);
	dpu_pr_debug("ipi_clk: %d\n", ctrl->ipi_clk);
	dpu_pr_debug("sys_clk: %d\n", ctrl->phy_parms.sys_clk);
	dpu_pr_debug("phy_lp_speed: %d\n", ctrl->phy_parms.phy_lp_speed);
	dpu_pr_debug("phy_hs_speed: %d\n", ctrl->phy_parms.phy_hs_speed);
	dpu_pr_debug("lane_num:%u\n", pinfo->lane_num);
	dpu_pr_debug("phy_type:%u\n", pinfo->phy_type);
	dpu_pr_debug("clk_type:%u\n", pinfo->phy_clk_type);
	dpu_pr_debug("video_mode_type:%u\n", pinfo->burst_mode);
	dpu_pr_info("bta_en:%u\n", pinfo->bta_en);
	dpu_pr_info("eotp_en:%u\n", pinfo->eotp_en);
}

static void display_to_tmg(struct connector *connector,
		struct dsi_tmg_cfg *tmg, struct dpu_hw_blk *hw)
{
	struct dpu_panel_info *pinfo;
	struct panel_timing *timing;
	bool dsc_en = false;

	pinfo = connector->pinfo;
	timing = &connector->timing;
	dpu_check_and_no_retval(!timing, "timing is null\n");

	dpu_check_and_no_retval(!connector->pinfo, "connector->pinfo is null\n");
	dsc_en = connector->pinfo->dsc_cfg.dsc_en;
	tmg->tmg_mode = pinfo->display_type;
	tmg->vg_en = 0;
	tmg->sbs_en = pinfo->dual_port;
	tmg->vg_dpmode = 0;
	tmg->partial_en = 0;
	tmg->cmd_wait_data_en  = CMD_WAITE_DATA_ENABLE;
	tmg->ipi_clk = pinfo->pixel_clk_rate;
	dpu_pr_debug("ipi_clk: %d\n", tmg->ipi_clk);

	tmg->fps_base = timing->framerate;
	tmg->fps_update = timing->framerate;
	tmg->current_base = 0;
	tmg->te_sel = TMG_TE_FROM_GPIO;
	tmg->base.base0.hsa = timing->hpw;
	tmg->base.base0.hbp = timing->hbp;
	tmg->base.base0.hfp = timing->hfp;
	tmg->base.base0.hact = timing->hdisplay;

	tmg->base.base0.vsa = timing->vpw;
	tmg->base.base0.vbp = timing->vbp;
	tmg->base.base0.vfp = timing->vfp;
	tmg->base.base0.vact = timing->vdisplay;

	tmg->format = dsc_en ? DSI_FMT_DSC : DSI_FMT_RGB888;

	tmg->vrr_mode = CMD_MODE_MANUAL_VRR;
	tmg->vrr_cmd.cmd_skip_te_next = 0;
	tmg->vrr_en = 1;

	tmg->vg.pat_sel = 1;
	tmg->vg.user_pattern = 0xFFFFFF;
	tmg->cmdlist_flag = true;

	if (pinfo->dual_port) {
		if (hw->blk_id == 0) {
			tmg->obuf.obufen = 0b0101;
		} else if(hw->blk_id == 1) {
			tmg->obuf.obufen = 0b1010;
		} else {
			tmg->obuf.obufen = 0b0101;
			dpu_pr_err("tmg port error, blk_id:%d\n", hw->blk_id);
		}
	} else {
		tmg->obuf.obufen = 0b1111;
	}

	tmg->obuf.core_fm_timing_en = 1;
}

static void dsi_flow_tmg_obuf_level_set(struct dsi_tmg_cfg *tmg)
{
	struct obuffer_parms *obuf = NULL;
	u32 obuf_depth;
	u64 urgent_obuf_depth;
	u64 vtotal, hdisplay;
	u64 bpp;
	u64 fps;

	obuf = &tmg->obuf;
	obuf_depth = GET_BIT_VAL(obuf->obufen, 0) * OBUF_MEMORY0_1_SIZE +
			GET_BIT_VAL(obuf->obufen, 1) * OBUF_MEMORY0_1_SIZE +
			GET_BIT_VAL(obuf->obufen, 2) * OBUF_MEMORY2_3_SIZE +
			GET_BIT_VAL(obuf->obufen, 3) * OBUF_MEMORY2_3_SIZE;
	obuf->core_obuf_lvl_thre_buf0 = obuf_depth / 4;
	obuf->core_obuf_lvl_thre_buf1 = obuf_depth / 2;
	obuf->core_obuf_lvl_thre_buf2 = obuf_depth * 3 / 4;
	obuf->core_obuf_th_buf = obuf_depth / 20;

	vtotal = tmg->base.base0.vsa + tmg->base.base0.vbp + tmg->base.base0.vfp + tmg->base.base0.vact;
	hdisplay = tmg->base.base0.hact;

	// bpp must be consistent with tmg format when dsc off
	bpp = (tmg->format == DSI_FMT_DSC) ? 8 : 24;
	fps = tmg->fps_base;

	urgent_obuf_depth = 60 * fps * vtotal * bpp * hdisplay /
			1000000 / 240;

	dpu_pr_info("urgent depth %llu, fps %llu, vtotal %llu, bpp %u, width %llu\n",
		urgent_obuf_depth, fps, vtotal, bpp, hdisplay);
	obuf->core_urgent_h_thre_buf = urgent_obuf_depth * 11 / 20;
	obuf->core_urgent_l_thre_buf = urgent_obuf_depth / 2;
	obuf->core_urgent_vld_en_buf = 1;

	obuf->core_dfc_h_thre_buf = urgent_obuf_depth;
	obuf->core_dfc_l_thre_buf = urgent_obuf_depth;

	dpu_pr_info(
		"core_urgent_vld_en_buf %u\n"
		"core_dfc_l_thre_buf %u\n"
		"core_dfc_h_thre_buf %u\n"
		"core_urgent_l_thre_buf %u\n"
		"core_urgent_h_thre_buf %u\n"
		"core_obuf_lvl_thre_buf0 %u\n"
		"core_obuf_lvl_thre_buf1 %u\n"
		"core_obuf_lvl_thre_buf2 %u\n"
		"core_obuf_th_buf %u\n"
		"obuf0_en: %u\n"
		"obuf1_en: %u\n"
		"obuf2_en: %u\n"
		"obuf3_en: %u\n",
		obuf->core_urgent_vld_en_buf,
		obuf->core_dfc_l_thre_buf,
		obuf->core_dfc_h_thre_buf,
		obuf->core_urgent_l_thre_buf,
		obuf->core_urgent_h_thre_buf,
		obuf->core_obuf_lvl_thre_buf0,
		obuf->core_obuf_lvl_thre_buf1,
		obuf->core_obuf_lvl_thre_buf2,
		obuf->core_obuf_th_buf,
		GET_BIT_VAL(obuf->obufen, 0),
		GET_BIT_VAL(obuf->obufen, 1),
		GET_BIT_VAL(obuf->obufen, 2),
		GET_BIT_VAL(obuf->obufen, 3));
}

static void dsi_tmg_init(struct connector *connector)
{
	struct dsi_tmg_cfg cfg = {0};
	struct dpu_hw_blk *hw_tmg;

	hw_tmg = &connector->hw_tmg;
	display_to_tmg(connector, &cfg, hw_tmg);
	dsi_flow_tmg_obuf_level_set(&cfg);
	dsi_hw_tmg_obufen_config(hw_tmg, cfg.obuf.obufen);
	dsi_hw_tmg_obuffer_level_config(hw_tmg, &cfg.obuf);
	dsi_hw_tmg_init(hw_tmg, &cfg);
	dsi_hw_tmg_enable(hw_tmg);

	if (connector->bind_connector) {
		hw_tmg = &connector->bind_connector->hw_tmg;
		display_to_tmg(connector, &cfg, hw_tmg);
		dsi_flow_tmg_obuf_level_set(&cfg);
		dsi_hw_tmg_obufen_config(hw_tmg, cfg.obuf.obufen);
		dsi_hw_tmg_obuffer_level_config(hw_tmg, &cfg.obuf);
		dsi_hw_tmg_init(hw_tmg, &cfg);
		dsi_hw_tmg_enable(hw_tmg);
	}
}

static int32_t dsi_ctrl_enable(struct connector *connector, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_ctrl_hw_blk hw = {0};
	struct dsi_ctrl_hw_blk hw_bind = {0};
	int32_t ret;

	get_dsi_ctrl_hw(connector, &hw);
	ret = dsi_hw_ctrl_enable(&hw, cfg);
	dsi_hw_sctrl_vg_en(hw.sctrl_blk, cfg);

	if (connector->bind_connector) {
		get_dsi_ctrl_hw(connector->bind_connector, &hw_bind);
		ret = dsi_hw_ctrl_enable(&hw_bind, cfg);
		dsi_hw_sctrl_vg_en(hw_bind.sctrl_blk, cfg);
	}
	return ret;
}

void mipi_dsi_init(struct platform_device *pdev, struct connector *connector, struct dsi_ctrl_cfg *ctrl_cfg)
{
	struct dsi_ctrl_hw_blk hw = {0};
	struct dsi_ctrl_hw_blk hw_bind = {0};
	int32_t ret;

	get_dsi_ctrl_hw(connector, &hw);
	ret = dsi_hw_init(&hw, ctrl_cfg);
	if (ret)
		dpu_pr_err("dsi ctrl init failled\n");

	if (connector->bind_connector) {
		get_dsi_ctrl_hw(connector->bind_connector, &hw_bind);
		ret = dsi_hw_init(&hw_bind, ctrl_cfg);
		if (ret)
			dpu_pr_err("dsi ctrl init failled\n");
	}

	if (pipeline_next_ops_handle(pdev, "need_reset_after_lp11", connector->id, NULL)) {
		dpu_pr_info("reset_after_lp11\n");
		pipeline_next_ops_handle(pdev, "panel_reset_flush", connector->id, NULL);
	}

	dsi_hw_wait_ready(&hw, ctrl_cfg);

	if (connector->bind_connector) {
		get_dsi_ctrl_hw(connector->bind_connector, &hw_bind);
		dsi_hw_wait_ready(&hw_bind, ctrl_cfg);
	}

	dpu_pr_info("mipi_dsi_init success!\n");
}

static int32_t mipi_dsi_on(struct platform_device *pdev, struct connector *connector, uint32_t scene_id)
{
	struct dsi_ctrl_cfg ctrl_cfg = {0};

	dpu_pr_debug("conn id:%u +\n", connector->id);

	pipeline_next_on(pdev, &connector->id);

	display_to_dsi(connector, &ctrl_cfg);

	mipi_dsi_init(pdev, connector, &ctrl_cfg);

	pipeline_next_ops_handle(pdev, "lcd_send_initial_cmd", connector->id, NULL);

	pipeline_next_ops_handle(pdev, "set_backlight", connector->id, NULL);

	dsi_ctrl_enable(connector, &ctrl_cfg);

	dsi_tmg_init(connector);

	dpu_pr_debug("conn id:%u -\n", connector->id);
	return 0;
}

static int32_t mipi_dsi_off(struct platform_device *pdev, struct connector *connector, uint32_t scene_id)
{
	dpu_pr_debug("conn id:%u+\n", connector->id);

	pipeline_next_ops_handle(pdev, "lcd_send_display_off_cmd", connector->id, NULL);
	pipeline_next_off(pdev, &connector->id);

	dpu_pr_debug("conn id:%u-\n", connector->id);
	return 0;
}

int32_t mipi_set_backlight(struct platform_device *pdev, uint32_t id, void *value)
{
	dpu_pr_debug("+\n");
	pipeline_next_ops_handle(pdev, "set_backlight", id, value);
	dpu_pr_debug("-\n");
	return 0;
}

static struct dpu_hw_blk *get_conn_hw_blk(enum CONN_BLK_TYPE blk_type, struct connector *connector)
{
	struct dpu_hw_blk *hw_blk = NULL;
	uint32_t dsi_index = connector->id;

	switch(blk_type) {
	case BLK_PHY:
		hw_blk = &connector->dsi_hw_res.phy_blk;
		hw_blk->iomem_base = (dsi_index == 0) ? 0xE8000000 : 0xE8100000;
		hw_blk->blk_id = dsi_index;
		hw_blk->blk_len = 0x40000;
		break;
	case BLK_DSI_SCTRL:
		hw_blk = &connector->dsi_hw_res.sctrl_blk;
		hw_blk->iomem_base = (dsi_index == 0) ? 0xE8080000 : 0xE8180000;
		hw_blk->blk_id = dsi_index;
		hw_blk->blk_len = 0x1000;
		break;
	case BLK_DSI_CTRL:
		hw_blk = &connector->dsi_hw_res.ctrl_blk;
		hw_blk->iomem_base = (dsi_index == 0) ? 0xE8081000 : 0xE8181000;
		hw_blk->blk_id = dsi_index;
		hw_blk->blk_len = 0x800;
		break;
	case BLK_TMG:
		hw_blk = &connector->hw_tmg;
		hw_blk->iomem_base = (dsi_index == 0) ? 0xE7051200 : 0xE7054200;
		hw_blk->blk_id = dsi_index;
		hw_blk->blk_len = 0x200;
		break;
	default:
		break;
	}

	dpu_pr_debug("blk_type:%u base:0x%x id:%u len:%u\n",
		blk_type, hw_blk->iomem_base, hw_blk->blk_id, hw_blk->blk_len);

	return hw_blk;
}

static int get_conn_phy_info(struct connector *connector)
{
	int32_t offset;
	void *fdt;
	int ret;
	const char *hw_platform;
	u32 ppi_width;

	if (!connector) {
		dpu_pr_err("connector is null\n");
		return -1;
	}
	fdt = dpu_get_fdt();
	if (!fdt) {
		dpu_pr_err("dpu_get_fdt fail\n");
		return -1;
	}

	offset = dpu_get_fdt_offset(fdt, PHY_DTS_PATH);
	if (offset < 0) {
		dpu_pr_err("get invalid offset, path:%a\n", PHY_DTS_PATH);
		return -1;
	}

	ret = dpu_dts_parse_string(fdt, offset, "hw-platform", &hw_platform);
	if (ret < 0) {
		dpu_pr_err("parse dsi hw-platform failed\n");
		return -1;
	} else if (!dpu_str_cmp(hw_platform, "xilinx")) {
		connector->phy_sel = XILINX_DPHY;
	} else if (!dpu_str_cmp(hw_platform, "snps_asic")) {
		connector->phy_sel = SNPS_CDPHY_ASIC;
	} else if (!dpu_str_cmp(hw_platform, "snps_fpga")) {
		connector->phy_sel = SNPS_CDPHY_FPGA;
	} else {
		dpu_pr_err("get invalid hw-platform:%s\n", hw_platform);
		return -1;
	}

	ret = dpu_dts_parse_u32(fdt, offset, "ppi-width", &ppi_width);
	if (ret) {
		dpu_pr_err("parse ppi-width failed\n");
		return -1;
	}

	switch (ppi_width) {
	case 8:
		connector->ppi_width = PHY_PPI_WIDTH_8BIT;
		break;
	case 16:
		connector->ppi_width = PHY_PPI_WIDTH_16BIT;
		break;
	case 32:
		connector->ppi_width = PHY_PPI_WIDTH_32BIT;
		break;
	default:
		dpu_pr_err("get invalid ppi_width:%s\n", ppi_width);
		return -1;
	}
	dpu_pr_info("phy hw-platform:%a, ppi-width:%u\n",
			hw_platform, ppi_width);

	return 0;
}

void get_mipi_hw_res(struct connector *connector)
{
	struct dpu_hw_blk *hw_blk = NULL;

	hw_blk = get_conn_hw_blk(BLK_PHY, connector);
	dpu_check_and_no_retval(!hw_blk, "get phy hw_blk failed\n");
	connector->dsi_hw_res.phy_blk = *hw_blk;

	hw_blk = get_conn_hw_blk(BLK_DSI_SCTRL, connector);
	dpu_check_and_no_retval(!hw_blk, "get dsi sctrl hw_blk failed\n");
	connector->dsi_hw_res.sctrl_blk = *hw_blk;

	hw_blk = get_conn_hw_blk(BLK_DSI_CTRL, connector);
	dpu_check_and_no_retval(!hw_blk, "get dsi ctl hw_blk failed\n");
	connector->dsi_hw_res.ctrl_blk = *hw_blk;

	hw_blk = get_conn_hw_blk(BLK_TMG, connector);
	dpu_check_and_no_retval(!hw_blk, "get tmg hw_blk failed\n");
	connector->hw_tmg = *hw_blk;

	if (get_conn_phy_info(connector))
		dpu_pr_err("failed to get phy info\n");

	if (connector->bind_connector)
		get_mipi_hw_res(connector->bind_connector);
}

void mipi_ops_set_up(struct connector *connector)
{
	get_mipi_hw_res(connector);

	connector->on = mipi_dsi_on;
	connector->off = mipi_dsi_off;
}

void mipi_ops_remove(struct connector *connector)
{
	connector->on = NULL;
	connector->off = NULL;
}

uint32_t dsi_ipi_clk_mode_get(struct dpu_panel_info *pinfo)
{
	enum dsi_ipi_clk_mode mode;

	if (pinfo->ipi_pll_sel == DSI_PIXEL_CLOCK_DPU_PLL) {
		if (pinfo->dual_port)
			mode = DSI_IPI_BACKUP_DUAL_PORT;
		else
			mode = (pinfo->connector_id == 0) ? DSI_IPI_BACKUP_PORT0 : DSI_IPI_BACKUP_PORT1;
	} else {
		if (pinfo->dual_port)
			mode = DSI_IPI_MAIN_DUAL_PORT;
		else
			mode = (pinfo->connector_id == 0) ? DSI_IPI_MAIN_PORT0 : DSI_IPI_MAIN_PORT1;
	}

	return mode;
}

int dsi_clk_div_cal(unsigned long rate, u32 *out_div, u64 *out_rate)
{
	u64 base_rate, rate_tmp;
	u32 div = 0;

	base_rate = rate;
	rate_tmp = base_rate;
	while (rate_tmp < DPU_PLL_MIN) {
		div += 1;
		rate_tmp = rate_tmp + base_rate;
	}

	dpu_pr_info("dest rate %llu, div %d, base rate %lu\n",
		rate_tmp, div, rate);

	if (rate_tmp > DPU_PLL_MAX) {
		dpu_pr_err("unsupported pixel clk rate(%llu)", rate_tmp);
		return -1;
	}

	*out_div = div;
	*out_rate = rate_tmp;
	return 0;
}
