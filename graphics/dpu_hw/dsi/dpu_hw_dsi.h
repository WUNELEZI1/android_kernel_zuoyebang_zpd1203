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

#include "dpu_hw_common.h"

#ifndef _DPU_HW_DSI_H_
#define _DPU_HW_DSI_H_

/**
 * Custom MIPI DSI message flags.
 * BIT(0) and BIT(1) are already defined in drm_mipi_dsi.h
 */
#define MIPI_DSI_MSG_REQ_ACK	BIT(0)
#define MIPI_DSI_MSG_USE_LPM	BIT(1)
#define MIPI_DSI_MSG_BATCH_COMMAND BIT(6)
#define MIPI_DSI_MSG_UNICAST_COMMAND BIT(7)

#define MIPI_DSI_VIDEO_MODE   (1)
#define MIPI_DSI_COMMAND_MODE (0)

#define CMD_WAITE_DATA_ENABLE     (1)
#define CMD_WAITE_DATA_DISABLE    (0)

#define TMG_TE_FROM_GPIO      (0)
#define TMG_TE_FROM_DSI       (1)

#define MIPI_INIT_STEP_0      (0)
#define MIPI_INIT_STEP_1      (1)
#define MIPI_INIT_STEP_2      (2)
#define MIPI_INIT_STEP_3      (3)
#define MIPI_INIT_STEP_4      (4)
#define MIPI_INIT_STEP_5      (5)

#define DSI_HDR_MEM_MAX           (64)
#define DSI_PLD_MEM_MAX           (1280)

enum dsi_ctrl_cri_mode {
	CRI_MODE_PKT_STACK = 0,
	CRI_MODE_PLD_MERGE,
};

enum dsi_debug {
	DSI_SET_PHY_AMPLITUDE,
	DSI_SET_PHY_EQ,
	DSI_GET_PHY_EQ_PARMS,
	DSI_DEBUG_MAX,
};

enum dsi_phy_amplitude {
	PHY_AMPLITUDE_100MV,
	PHY_AMPLITUDE_175MV,
	PHY_AMPLITUDE_225MV,
	PHY_AMPLITUDE_200MV,
	PHY_AMPLITUDE_237MV,
	PHY_AMPLITUDE_MAX,
};

enum dsi_phy_ppi_width {
	PHY_PPI_WIDTH_8BIT,
	PHY_PPI_WIDTH_16BIT,
	PHY_PPI_WIDTH_32BIT,
	PHY_PPI_WIDTH_MAX,
};

enum dsi_phy_lane {
	DSI_PHY_1LANE = 0,
	DSI_PHY_2LANE = 1,
	DSI_PHY_3LANE = 2,
	DSI_PHY_4LANE = 3,
	DSI_PHY_LANE_MAX,
};

/**
 * enum dsi_phy_type - DSI phy types
 * @DSI_PHY_TYPE_DPHY: type for dsi DPHY
 * @DSI_PHY_TYPE_CPHY: type for dsi CPHY
 */
enum dsi_phy_type {
	DSI_PHY_TYPE_DPHY,
	DSI_PHY_TYPE_CPHY
};

enum dsi_phy_clk_type {
	DSI_PHY_CONTINUOUS_CLK,
	DSI_PHY_NON_CONTINUOUS_CLK,
};

enum dsi_manual_mode {
	DSI_IPI_MANUAL_MODE,
	DSI_IPI_AUTO_MODE,
	DSI_IPI_MODE_MAX,
};

enum dsi_pixel_format {
	DSI_FMT_RGB565,
	DSI_FMT_RGB666,
	DSI_FMT_RGB888,
	DSI_FMT_RGB101010,
	DSI_FMT_RGB121212,
	DSI_FMT_YUV422,
	DSI_FMT_YUV420,
	DSI_FMT_YUV422_LOOSELY,
	DSI_FMT_RGB_LOOSELY,
	DSI_FMT_DSC,
	DSI_FMT_MAX,
};

enum dsi_pixel_depth {
	DSI_DEPTH_565,
	DSI_DEPTH_666,
	/* DSC use this */
	DSI_DEPTH_888,
	DSI_DEPTH_101010,
	DSI_DEPTH_121212,
	DSI_DEPTH_MAX,
};

enum dsi_hibernate_type {
	DSI_HIBERNATE_TYPE_LP11,
	DSI_HIBERNATE_TYPE_ULPS,
	DSI_HIBERNATE_TYPE_MAX,
};

/* dsi host engine work mode */
enum dsi_ctrl_mode {
	DSI_VIDEO_MODE = 0,
	DSI_CMD_MODE
};

/* dsi host video mode type */
enum video_mode_type {
	NON_BURST_WITH_SYNC_PULSES = 0x0,
	NON_BURST_WITH_SYNC_EVENTS,
	BURST_MODE,
	VID_MODE_TYPE_MAX,
};

/* dsi select phy */
enum phy_select {
	SNPS_CDPHY_ASIC = 0x0,
	SNPS_CDPHY_FPGA,
	XILINX_DPHY,
};

/**
 * struct partial_parms - paritial update parmeters
 * @xs: x start, h_active start
 * @ys: y start, v_active start
 */
struct partial_parms {
	u32 width;
	u32 height;
	u32 xs;
	u32 ys;
};

struct dsi_msg {
	u8 channel;
	u8 type;
	u16 flags;

	size_t tx_len;
	const void *tx_buf;

	size_t rx_len;
	void *rx_buf;
};

struct dsi_cmds {
	u32 count;
	u8 transfer_type;
	struct dsi_msg *msg;
};

/* sctrl, phy: for phy pll calculate */
struct freq_range_params {
	u32 freq;
	u32 vco_cntrl;
	u32 cpibas_cntrl;
	/* phy pll calculate temporary parameter */
	u8  p;
	u8 gmp_cntrl;
};

/* dsi controller PHY regiter */
struct dsi_ctrl_phy {
	enum dsi_phy_ppi_width ppi_width;
	/* lane number */
	enum dsi_phy_lane lanes_num;
	/* CPHY or DPHY */
	enum dsi_phy_type phy_type;
	/*for phy_lp divid */
	int sys_clk;
	/* for lp transfer speed MHz */
	int phy_lp_speed;
	/* lane_rate: for hs transfer speed: bps */
	u64 phy_hs_speed;
	/* continuous or non-continuous clock */
	enum dsi_phy_clk_type clk_type;
};

/* dsi_phy_cfg - dsi phy config data structure */
struct dsi_phy_cfg {
	/* 0: SNPS_CDPHY_ASIC, 1: SNSP_CDPH_FPGA, 2: XILINX_DPHY, */
	enum phy_select phy_sel;
	/* initial step */
	u8 initial_step;
	/* CPHY or DPHY */
	enum dsi_phy_type phy_type;
	/* ppi interface width: 8, 16, 32 */
	enum dsi_phy_ppi_width ppi_width;
	/* lane number */
	enum dsi_phy_lane lanes_num;
	/*for phy_lp divid */
	int sys_clk;
	/* for lp transfer speed MHz */
	int phy_lp_speed;
	/* lane_rate: for hs transfer speed: bps */
	int phy_hs_speed;
	/* continuous or non-continuous clock */
	enum dsi_phy_clk_type clk_type;
	/* phy amplitude */
	enum dsi_phy_amplitude phy_amplitude;
	/* phy oa setr, should read back at runing */
	u8 phy_oa_setr;
	/* phy eq parmater : eqa */
	u8 phy_eqa;
	/* phy eq parmater : eqb */
	u8 phy_eqb;
};

struct dsi_ipi_format {
	enum dsi_pixel_format color_fmt;
	enum dsi_pixel_depth color_depth;
};

/* dsi controller DSI regiter */
struct dsi_general_parms {
	/* for bta function: 0 - disable, 1 - enable */
	u8  bta_en;
	/* for eotp pkt send: 0 - disable, 1 - enable */
	u8  eotp_en;
	/* virtual channel: tx_vcid */
	u8  virtual_channel;
	u16 scrambling_seed;
	u8  scrambling_en;
	u8  vfp_hs_en;
	u8  vbf_hs_en;
	u8  vsa_hs_en;
	u8  hfp_hs_en;
	u8  hbp_hs_en;
	u8  hsa_hs_en;
	/**
	 * video mode transmission type: Burst_Mode,
	 * Non-burst Mode with sync pulses, Non-burst with sync events
	 */
	u8  vid_mode_type;
	u8  max_rt_pkt_size;
	u8  tear_effect;
};

struct dsi_sync_timing {
	u32 hsa;
	u32 hbp;
	u32 hfp;
	u32 hact;
	u32 vsa;
	u32 vbp;
	u32 vfp;
	u32 vact;
	u32 vtotal;
	u32 htotal;
};

struct dsi_tear_parms {
	u8 tear_scanline;
	u8 tear_on;
	u8 te_type;
	u8 auto_tear_bta_disable;
};

/* dsi_ctrl_cfg - dsi controller config data structure */
struct dsi_ctrl_cfg {
	/* 1: DIRECT_WRITE(true), 0: CMDLIST_WRITE(false) */
	u8 cmdlist_flag;
	/* cmdlist node id for hw ulps */
	s64 cmdlist_node_id;
	/* for bta function: 0 - disable, 1 - enable */
	u8  bta_en;
	/* for eotp pkt send: 0 - disable, 1 - enable */
	u8  eotp_en;
	/* virtual channel: tx_vcid */
	u8  virtual_channel;
	/* video mode or command mode */
	enum dsi_ctrl_mode ctrl_mode;
	/* dsi controller timing calculate mode */
	u8 manual_mode_en;
	/* dsi phy clock calculate */
	u8 auto_calc_en;
	/* frame rate */
	u8 fps;
	/* dsi vg test: 0 - disable, 1 - enable */
	u8 vg_en;
	/* dsi vg pattern: 0 - Color-bar, 1 - R, 2 - G, 3 - B */
	u8 vg_dpmode;
	/* ipi clock */
	u32 ipi_clk;
	/* ipi clock div */
	u32 ipi_clk_div;
	/* phy parameters */
	struct dsi_phy_cfg phy_parms;
	/* dsi controller and system controller parameters */
	//struct dsi_general_parms dsi_parms;
	/* video mode type: burst or non-burst */
	enum video_mode_type video_mode_type;
	/* pixel format: rgb888, dsc, and so on */
	enum dsi_pixel_format pixel_fomat;
	/* hsync and vsync timing informations */
	struct dsi_sync_timing timing;
	/* tear information */
	struct dsi_tear_parms tear;
};

/* cri: dsi controller command intercace */
struct cri_rx_hdr {
	/* word count msb */
	u8 wc_msb;
	/* word count lsb */
	u8 wc_lsb;
	/* virtural channel */
	u8 virtual_channel;
	/* data type for packet */
	u8 data_type;
};

struct cri_tx_hdr {
	/* 0- Short Pkt, 1- Long Pkt */
	u8 cmd_hdr_long;
	/* 0- Write, 1- Read */
	u8 cmd_hdr_rd;
	/* 0- Hight-speed, 1- Low-power */
	u8 cmd_tx_mode;
	/* word count msb */
	u8 wc_msb;
	/* word count lsb */
	u8 wc_lsb;
	/* virtural channel */
	u8 virtual_channel;
	/* data type for packet */
	u8 data_type;
};

struct dsi_ctrl_hw_blk {
	struct dpu_hw_blk *ctrl_blk;
	struct dpu_hw_blk *sctrl_blk;
	struct dpu_hw_blk *phy_blk;

	struct dpu_hw_blk *dctrl_blk;
	struct dpu_hw_blk *dsctrl_blk;
};

#endif
