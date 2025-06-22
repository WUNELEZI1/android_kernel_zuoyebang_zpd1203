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

#ifndef _DP_HW_COMMON_H_
#define _DP_HW_COMMON_H_

#include "dpu_osal.h"

/* #define FPGA */
#define ASIC

#ifdef __KERNEL__

#include <soc/xring/xr_usbdp_event.h>
#include <soc/xring/display/dp_audio_interface.h>

#else

struct dp_audio_fmt_info {
	u32 samp_rate;
	u16 bit_width;
	u16 ch_num;
};

enum dp_hpd_event {
	DP_HPD_EVENT_PLUG_OUT = 0,
	DP_HPD_EVENT_PLUG_IN,
	DP_HPD_EVENT_IRQ,
	DP_HPD_EVENT_MAX,
};

enum dp_orientation {
	DP_ORIENTATION_OBSERVE = 0,
	DP_ORIENTATION_RESERVE = 1,
	DP_ORIENTATION_MAX,
};

enum dp_combo_mode {
	DP_COMBO_MODE_DP2,
	DP_COMBO_MODE_DP4,
	DP_COMBO_MODE_MAX,
};

#endif

/**
 * hardware restriction, max ipi clock is 208Mhz.
 * we use 4 pixel mode default, so max pixel clock of hardware restriction
 * is 208Mhz * 4 = 832Mhz
 *
 * F1 dp specification designed restriction, max resolution is 4k@60hz.
 * max pixel clock for 4k@60hz in CTA-861-G is 594Mhz.
 *
 * here we use 594Mhz to limit DP video stream pixel clock.
 */
#define DP_MAX_IPI_CLK_KHZ                                  594000

#define BITS_MASK(bit_start, bit_num) \
	((BIT((bit_num)) - 1) << (bit_start))

#define MERGE_32(byte0, byte1, byte2, byte3) \
	((((u32)(byte0) << 24) & 0xFF000000) | \
	(((u32)(byte1) << 16) & 0xFF0000) | \
	(((u32)(byte2) << 8) & 0xFF00) | \
	((u32)(byte3) & 0xFF))

#define DP_REG_READ DPU_REG_READ
#define DP_REG_WRITE(hw, offset, val) \
	DPU_REG_WRITE(hw, offset, val, true)
#define DP_BIT_WRITE(hw, offset, val, shift, len) \
	DPU_BIT_WRITE(hw, offset, val, shift, len, true)
#define DP_READ_POLL_TIMEOUT DPU_READ_POLL_TIMEOUT

enum dp_bpc {
	DP_BPC_6                       = 6,
	DP_BPC_8                       = 8,
	DP_BPC_10                      = 10,
	DP_BPC_12                      = 12,
	DP_BPC_16                      = 16,
};

#define DP_TIMING_POLARITY_ACTIVE_HIGH                  1
#define DP_TIMING_POLARITY_ACTIVE_LOW                   0
#define DP_TIMING_INTERLACED                            1
#define DP_TIMING_PROGRESSIVE                           0

struct dp_display_timing {
	u8 interlaced;
	u8 h_sync_polarity;
	u16 h_active;
	u16 h_front_porch;
	u16 h_sync_width;
	u16 h_back_porch;
	u16 h_blank;
	u16 h_total;
	u8 v_sync_polarity;
	u16 v_active;
	u16 v_front_porch;
	u16 v_sync_width;
	u16 v_back_porch;
	u16 v_blank;
	u16 v_total;
	u32 pixel_clock_khz;
	u32 fps_thousand;
	u32 color_depth;
};

enum dp_link_rate {
	DP_LINK_RATE_RBR          = 0,
	DP_LINK_RATE_HBR          = 1,
	DP_LINK_RATE_HBR2         = 2,
	DP_LINK_RATE_HBR3         = 3,
	DP_LINK_RATE_MAX          = 3,
};

enum dp_lane_count {
	DP_LANE_COUNT_1           = 1,
	DP_LANE_COUNT_2           = 2,
	DP_LANE_COUNT_4           = 4,
	DP_LANE_COUNT_MAX         = 4,
};

enum dp_custom_pattern {
	DP_CUSTOM_PATTERN_CUS,
	DP_CUSTOM_PATTERN_PLTPAT,
};

#define DP_MAX_EXT_SDP_COUNT                  4
#define DP_SDP_PAYLOAD32_SIZE                 8

struct dp_sdp_packet {
	u32 header;
	u32 payload[DP_SDP_PAYLOAD32_SIZE];
};

const static char *link_rate_str_table[] = {"1.62Gbps", "2.7Gbps", "5.4Gbps", "8.1Gbps"};

static inline const char *to_link_rate_str(enum dp_link_rate rate)
{
	if (rate >= DP_LINK_RATE_RBR && rate <= DP_LINK_RATE_HBR3)
		return link_rate_str_table[rate];
	else
		return "unknown rate";
}

#endif
