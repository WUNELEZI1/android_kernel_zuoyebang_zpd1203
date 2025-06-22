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

#ifndef _DSI_HW_CTRL_H_
#define _DSI_HW_CTRL_H_

#include "dpu_hw_dsi.h"

#define SYS_CLK_ASIC           (208900000UL)
#define SYS_CLK_FPGA           (30000000UL)
#define LPTX_CLK_ASIC          (17400000UL)
#define LPTX_CLK_FPGA          (15000000UL)
#define DESKEW_RATE_1G5        (1500000000UL)

#define DSI_IPI_MANUAL_MODE_EN    (0x0)
#define DSI_IPI_AUTO_MODE_EN      (0x1)

#define PHY_LP2HS_TIME            (0xFF)
#define PHY_HS2LP_TIME            (0xFF)
#define PHY_MAX_RD_TIME           (0xFF)
#define PHY_ESC_CMD_TIME          (0xFF)
#define PHY_ESC_BYTE_TIME         (0xFF)
#define IPI_DEPTH_BITS565         (0x2)
#define IPI_DEPTH_BITS6           (0x3)
#define IPI_DEPTH_BITS8           (0x5)
#define IPI_DEPTH_BITS10          (0x6)
#define IPI_DEPTH_BITS12          (0x7)
#define IPI_FORMAT_RGB            (0x0)
#define IPI_FORMAT_YUV422         (0x1)
#define IPI_FORMAT_YUV420         (0x3)
#define IPI_FORMAT_YUV422_LOOSELY (0x4)
#define IPI_FORMAT_RGB_LOOSELY    (0x5)
#define IPI_FORMAT_DSC            (0xB)

#define CRI_SEND_BY_HS            (0x0)
#define CRI_SEND_BY_LP            (0x1)
#define CRI_READ_CMD_HDR          (0x1)
#define CRI_WRITE_CMD_HDR         (0x0)
#define CRI_SEND_LONG_PKT         (0x1)
#define CRI_SEND_SHORT_PKT        (0x0)
#define CRI_DATA_AVAIL            (0x40000)
#define CRI_RD_DATA_AVAIL         (0x40000)
#define CRI_BUSY                  (0x10000)
#define CRI_RX_DATA_TYPE          (0x3F)
#define CRI_RX_WC_LSB             (0xFF00)
#define CRI_RX_WC_MSB             (0xFF0000)
#define PRI_BUSY                  BIT(24)
#define IPI_BUSY                  BIT(8)
#define CRI_WR_FIFOS_NOT_EMPTY    BIT(17)

#define MAX_RPS_SIZE              (0x100)
#define MAX_RT_PKT_SIZE           (0xFF)
#define CRI_HDR_FIFO_MAX          (0x40)
#define CRI_PLD_FIFO_MAX          (0x500)
#define DSI_FIFO_ACTIVE_WAIT      (200)

#define DSI_AUTO_CAL_TIMEOUT_US   (1000)
#define DSI_AUTO_CAL_DELAY_MS     (7)
#define DSI_MODE_CHANGE_DELAY_US  (200)

#define DSI_SET_CRI_PACKETS_STACK_MODE  (0)
#define DSI_SET_CRI_PAYLOAD_MERGE_MODE  (1)
#define DSI_SET_CRI_HOLD          (1)
#define DSI_SET_CRI_SEND          (0)

/* for ULPS and LP11 */
#define DSI_HIBERNATE_UL          (0x0)
#define DSI_HIBERNATE_LP11        (0x1)
#define DSI_HIB_WAKE_UP_TIME      (0x4F00)

#define DSI_ULPS_ENTER            (1)
#define DSI_ULPS_EXIT             (0)

#define DSI_DESKEW_MAX_100US      (100000)
#define POWER_2_15                (32768)

/* MIPI DSI Processor-to-Peripheral transaction types */
enum data_type{
	MIPI_DSI_V_SYNC_START                         = 0x01,
	MIPI_DSI_V_SYNC_END                           = 0x11,
	MIPI_DSI_H_SYNC_START                         = 0x21,
	MIPI_DSI_H_SYNC_END                           = 0x31,
	MIPI_DSI_COMPRESSION_MODE                     = 0x07,
	MIPI_DSI_END_OF_TRANSMISSION                  = 0x08,
	MIPI_DSI_COLOR_MODE_OFF                       = 0x02,
	MIPI_DSI_COLOR_MODE_ON                        = 0x12,
	MIPI_DSI_SHUTDOWN_PERIPHERAL                  = 0x22,
	MIPI_DSI_TURN_ON_PERIPHERAL                   = 0x32,
	MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM          = 0x03,
	MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM          = 0x13,
	MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM          = 0x23,
	MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM         = 0x04,
	MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM         = 0x14,
	MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM         = 0x24,
	MIPI_DSI_DCS_SHORT_WRITE                      = 0x05,
	MIPI_DSI_DCS_SHORT_WRITE_PARAM                = 0x15,
	MIPI_DSI_DCS_READ                             = 0x06,
	MIPI_DSI_EXECUTE_QUEUE                        = 0x16,
	MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE       = 0x37,
	MIPI_DSI_NULL_PACKET                          = 0x09,
	MIPI_DSI_BLANKING_PACKET                      = 0x19,
	MIPI_DSI_GENERIC_LONG_WRITE                   = 0x29,
	MIPI_DSI_DCS_LONG_WRITE                       = 0x39,
	MIPI_DSI_PICTURE_PARAMETER_SET                = 0x0a,
	MIPI_DSI_COMPRESSED_PIXEL_STREAM              = 0x0b,
	MIPI_DSI_LOOSELY_PACKED_PIXEL_STREAM_YCBCR20  = 0x0c,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR24          = 0x1c,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR16          = 0x2c,
	MIPI_DSI_PACKED_PIXEL_STREAM_30               = 0x0d,
	MIPI_DSI_PACKED_PIXEL_STREAM_36               = 0x1d,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR12          = 0x3d,
	MIPI_DSI_PACKED_PIXEL_STREAM_16               = 0x0e,
	MIPI_DSI_PACKED_PIXEL_STREAM_18               = 0x1e,
	MIPI_DSI_PIXEL_STREAM_3BYTE_18                = 0x2e,
	MIPI_DSI_PACKED_PIXEL_STREAM_24               = 0x3e,
};

/* MIPI DSI Peripheral-to-Processor transaction types */
enum dcs_read{
	MIPI_DSI_RX_ACKNOWLEDGE_AND_ERROR_REPORT       = 0x02,
	MIPI_DSI_RX_END_OF_TRANSMISSION                = 0x08,
	MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_1BYTE  = 0x11,
	MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_2BYTE  = 0x12,
	MIPI_DSI_RX_GENERIC_LONG_READ_RESPONSE         = 0x1a,
	MIPI_DSI_RX_DCS_LONG_READ_RESPONSE             = 0x1c,
	MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_1BYTE      = 0x21,
	MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_2BYTE      = 0x22,
};

/* MIPI DCS commands */
enum dsc_send{
	MIPI_DCS_NOP                     = 0x00,
	MIPI_DCS_SOFT_RESET              = 0x01,
	MIPI_DCS_GET_COMPRESSION_MODE    = 0x03,
	MIPI_DCS_GET_DISPLAY_ID          = 0x04,
	MIPI_DCS_GET_ERROR_COUNT_ON_DSI  = 0x05,
	MIPI_DCS_GET_RED_CHANNEL         = 0x06,
	MIPI_DCS_GET_GREEN_CHANNEL       = 0x07,
	MIPI_DCS_GET_BLUE_CHANNEL        = 0x08,
	MIPI_DCS_GET_DISPLAY_STATUS      = 0x09,
	MIPI_DCS_GET_POWER_MODE          = 0x0A,
	MIPI_DCS_GET_ADDRESS_MODE        = 0x0B,
	MIPI_DCS_GET_PIXEL_FORMAT        = 0x0C,
	MIPI_DCS_GET_DISPLAY_MODE        = 0x0D,
	MIPI_DCS_GET_SIGNAL_MODE         = 0x0E,
	MIPI_DCS_GET_DIAGNOSTIC_RESULT   = 0x0F,
	MIPI_DCS_ENTER_SLEEP_MODE        = 0x10,
	MIPI_DCS_EXIT_SLEEP_MODE         = 0x11,
	MIPI_DCS_ENTER_PARTIAL_MODE      = 0x12,
	MIPI_DCS_ENTER_NORMAL_MODE       = 0x13,
	MIPI_DCS_GET_IMAGE_CHECKSUM_RGB  = 0x14,
	MIPI_DCS_GET_IMAGE_CHECKSUM_CT   = 0x15,
	MIPI_DCS_EXIT_INVERT_MODE        = 0x20,
	MIPI_DCS_ENTER_INVERT_MODE       = 0x21,
	MIPI_DCS_SET_GAMMA_CURVE         = 0x26,
	MIPI_DCS_SET_DISPLAY_OFF         = 0x28,
	MIPI_DCS_SET_DISPLAY_ON          = 0x29,
	MIPI_DCS_SET_COLUMN_ADDRESS      = 0x2A,
	MIPI_DCS_SET_PAGE_ADDRESS        = 0x2B,
	MIPI_DCS_WRITE_MEMORY_START      = 0x2C,
	MIPI_DCS_WRITE_LUT               = 0x2D,
	MIPI_DCS_READ_MEMORY_START       = 0x2E,
	/* MIPI DCS 1.02 - MIPI_DCS_SET_PARTIAL_AREA before that */
	MIPI_DCS_SET_PARTIAL_ROWS        = 0x30,
	MIPI_DCS_SET_PARTIAL_COLUMNS     = 0x31,
	MIPI_DCS_SET_SCROLL_AREA         = 0x33,
	MIPI_DCS_SET_TEAR_OFF            = 0x34,
	MIPI_DCS_SET_TEAR_ON             = 0x35,
	MIPI_DCS_SET_ADDRESS_MODE        = 0x36,
	MIPI_DCS_SET_SCROLL_START        = 0x37,
	MIPI_DCS_EXIT_IDLE_MODE          = 0x38,
	MIPI_DCS_ENTER_IDLE_MODE         = 0x39,
	MIPI_DCS_SET_PIXEL_FORMAT        = 0x3A,
	MIPI_DCS_WRITE_MEMORY_CONTINUE   = 0x3C,
	MIPI_DCS_SET_3D_CONTROL     =      0x3D,
	MIPI_DCS_READ_MEMORY_CONTINUE    = 0x3E,
	MIPI_DCS_GET_3D_CONTROL          = 0x3F,
	MIPI_DCS_SET_VSYNC_TIMING        = 0x40,
	MIPI_DCS_SET_TEAR_SCANLINE       = 0x44,
	MIPI_DCS_GET_SCANLINE            = 0x45,
	/* **** Only for MIPI DCS 1.3 **** */
	MIPI_DCS_SET_DISPLAY_BRIGHTNESS  = 0x51,
	MIPI_DCS_GET_DISPLAY_BRIGHTNESS  = 0x52,
	MIPI_DCS_WRITE_CONTROL_DISPLAY   = 0x53,
	MIPI_DCS_GET_CONTROL_DISPLAY     = 0x54,
	MIPI_DCS_WRITE_POWER_SAVE        = 0x55,
	MIPI_DCS_GET_POWER_SAVE          = 0x56,
	MIPI_DCS_SET_CABC_MIN_BRIGHTNESS = 0x5E,
	MIPI_DCS_GET_CABC_MIN_BRIGHTNESS = 0x5F,
	/* **** Only for MIPI DCS 1.3 **** */
	MIPI_DCS_READ_DDB_START          = 0xA1,
	MIPI_DCS_READ_PPS_START          = 0xA2,
	MIPI_DCS_READ_DDB_CONTINUE       = 0xA8,
	MIPI_DCS_READ_PPS_CONTINUE       = 0xA9,
};

enum ipi_mode_ctrl {
	IPI_IDLE_MODE = 0x0,
	IPI_AUTO_CAL_MODE,
	/* for CRI/PRI/UCS */
	IPI_COMMAND_MODE,
	IPI_VIDEO_MODE,
	/* command mode */
	IPI_DATA_STREAM_MODE,
	IPI_MODE_CTRL_MAX,
};

enum debug_clk {
	DEBUG_IPI_CLK = 0x0,
	DEBUG_SYS_CLK,
	DEBUG_HSTX_CLK,
	DEBUG_HSRX_CLK,
	DEBUG_LPTX_CLK,
	DEBUG_GP_CLK,
	DEBUG_CLK_MAX,
};

struct dsi_phy_state {
	u8 phy_l3_ulps_active_not;
	u8 phy_l2_ulps_active_not;
	u8 phy_l1_ulps_active_not;
	u8 phy_l0_ulps_active_not;
	u8 phy_clk_ulps_active_not;
	u8 phy_l3_stop_state;
	u8 phy_l2_stop_state;
	u8 phy_l1_stop_state;
	u8 phy_l0_stop_state;
	u8 phy_clk_stop_state;
	u8 phy_direction;
};

struct dsi_phy_timing {
	u32 lp2hs_time;
	u32 hs2lp_time;
	u32 max_rd_time;
	u32 esc_cmd_time;
	u32 esc_byte_time;
	u32 ipi_ratio;
	u16 sys_ratio;
	u32 cal_time;
	u16 wakeup_time;
	u8  ulps_clk_lanes;
	u8  ulps_data_lanes;
	u32 to_hstx_value;
	u32 to_hstx_rdy_value;
	u32 to_lprx_value;
	u32 to_lprx_rdy_value;
	u32 to_lprx_trig_value;
	u32 to_lptx_ulps_value;
	u32 to_bta_value;
};

void dsi_hw_sctrl_init_clk_set(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
void dsi_hw_sctrl_init_phy_pre(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
void dsi_hw_sctrl_init_phy(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_hw_sctrl_pll_clk_sel - enable/disable sctrl pll clock
 * @hw: the pointer of sctrl hw blk
 * @enable: enable/disable
 */
void dsi_hw_sctrl_pll_clk_sel(struct dpu_hw_blk *hw, bool enable);

/**
 * dsi_hw_clk_debug - enable/disable sctrl debug dsi clock function
 * @hw: the pointer of sctrl hw blk
 */
void dsi_hw_clk_debug(struct dpu_hw_blk *hw);

/**
 * dsi_hw_sctrl_debug - sctrl debug interface
 * @hw: the pointer of sctrl hw blk
 */
void dsi_hw_sctrl_debug(struct dpu_hw_blk *hw);

#endif /* _DSI_HW_CTRL_H_ */
