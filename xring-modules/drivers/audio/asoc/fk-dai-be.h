/* SPDX-License-Identifier: GPL-2.0-only */
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

#ifndef __FK_DAI_BE_H
#define __FK_DAI_BE_H

#include "../acore/fk-acore-api.h"
#include <soc/xring/display/dp_audio_interface.h>
#include "../common/fk-common-ctrl.h"

#define PORT_RATE_48KHZ          48000
#define PORT_RATE_16KHZ          16000
#define PORT_PHYSICAL_WIDTH_BIT  32
#define PORT_BITS_16             16
#define PORT_BITS_24             24
#define PORT_BITS_32             32
#define PORT_CHANNELS_16         16
#define PORT_CHANNELS_8          8
#define PORT_CHANNELS_6          6
#define PORT_CHANNELS_4          4
#define PORT_CHANNELS_2          2
#define PORT_CHANNELS_1          1

/* Bit formats */
enum pcm_format {
	PCM_FORMAT_INVALID = -1,
	PCM_FORMAT_S16_LE = 0,  /* 16-bit signed */
	PCM_FORMAT_S32_LE,      /* 32-bit signed */
	PCM_FORMAT_S8,          /* 8-bit signed */
	PCM_FORMAT_S24_LE,      /* 24-bits in 4-bytes */
	PCM_FORMAT_S24_3LE,     /* 24-bits in 3-bytes */
	PCM_FORMAT_MAX,
};

/*struct for dp*/
struct fk_disp_audio_edid_blk {
	unsigned int audio_data_blk_size; /* in bytes */
	unsigned int spk_alloc_data_blk_size; /* in bytes */
	struct dp_audio_cta_blk dp_blk;
};
/*end for dp*/

enum i2s_port {
/*AUDIF i2s*/
	FK_XR_I2S_BEGIN = 0X0,
	FK_XR_I2S0_RX = FK_XR_I2S_BEGIN,
	FK_XR_I2S0_TX,
	FK_XR_I2S1_RX,
	FK_XR_I2S1_TX,
	FK_XR_I2S2_RX,
	FK_XR_I2S2_TX,
	FK_XR_I2S3_RX,
	FK_XR_I2S3_TX,
	FK_XR_I2S4_RX,
	FK_XR_I2S4_TX,
/*ADSP I2S*/
	FK_XR_I2S5_RX,
	FK_XR_I2S5_TX,
	FK_XR_I2S6_RX,
	FK_XR_I2S6_TX,
	FK_XR_I2S7_RX,
	FK_XR_I2S7_TX,
	FK_XR_I2S8_RX,
	FK_XR_I2S8_TX,
	FK_XR_I2S9_RX,
	FK_XR_I2S9_TX,
/*VAD_I2S*/
	FK_XR_VAD_I2S_TX,
/*mutil_i2s0 for DP*/
	FK_XR_MI2S0_RX,
/* AUDIO USB */
	FK_XR_AUDIO_USB_RX,
	FK_XR_AUDIO_USB_TX,
	FK_XR_I2S_MAX,
};

enum tdm_port {
/*AUDIF i2s*/
	FK_XR_TDM_BEGIN = 0X20,
	FK_XR_TDM0_RX = FK_XR_TDM_BEGIN,
	FK_XR_TDM0_TX,
	FK_XR_TDM1_RX,
	FK_XR_TDM1_TX,
	FK_XR_TDM2_RX,
	FK_XR_TDM2_TX,
	FK_XR_TDM3_RX,
	FK_XR_TDM3_TX,
	FK_XR_TDM4_RX,
	FK_XR_TDM4_TX,
	FK_XR_TDM5_RX,
	FK_XR_TDM5_TX,
	FK_XR_TDM6_RX,
	FK_XR_TDM6_TX,
	FK_XR_TDM7_RX,
	FK_XR_TDM7_TX,
	FK_XR_TDM8_RX,
	FK_XR_TDM8_TX,
	FK_XR_TDM9_RX,
	FK_XR_TDM9_TX,
	FK_XR_TDM7_8_RX,
	FK_XR_TDM7_8_TX,
	FK_XR_TDM_MAX,
};

struct dai_format *fk_aif_dev_cfg_get(int id);
int dp_plugged_cb(void *priv_data, bool plugged);

#endif //__FK_DAI_BE_H
