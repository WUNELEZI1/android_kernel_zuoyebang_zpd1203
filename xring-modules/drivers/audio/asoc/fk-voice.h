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

#include <linux/init.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/timer.h>
#include <asm/dma.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <sound/tlv.h>
#include <sound/pcm_params.h>

#include "fk-dai-fe.h"
#include "../acore/fk-acore-api.h"
#include "../acore/fk-acore-define.h"

#define  PCIE_BAR0_1_addr 0x00000000D2400000
#define  PCIE_BAR2_3_addr 0x00000000C4000000

struct fk_voice {
	struct snd_pcm_substream *playback_substream;
	struct snd_pcm_substream *capture_substream;

	uint32_t samp_rate;
	uint32_t channel_mode;

	int playback_prepare;
	int capture_prepare;

	int playback_start;
	int capture_start;

	uint16_t playback_session_id;
	uint16_t capture_session_id;

	struct voice_pcie_bar bar_info;
	enum speech_band band;
	int volume;
	bool ul_mute;
	bool dl_mute;
};

static inline unsigned int pcie_reg_read32(const void __iomem *reg)
{
	return __raw_readl(reg);
}

