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
#ifndef _FK_PCM_ROUTING_
#define _FK_PCM_ROUTING_

#include <linux/init.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/bitops.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/tlv.h>
#include <sound/pcm_params.h>
#include <sound/hwdep.h>

#include "fk-dai-fe.h"
#include "fk-dai-be.h"
#include "../acore/fk-acore-api.h"

#ifndef FK_AUDIO_USB_OFFLOAD
#define FK_AUDIO_USB_OFFLOAD 1
#endif

enum {
	FK_FRONTEND_DAI_XR_MEDIA0 = 0,
	FK_FRONTEND_DAI_XR_MEDIA1,
	FK_FRONTEND_DAI_XR_MEDIA2,
	FK_FRONTEND_DAI_XR_MEDIA3,
	FK_FRONTEND_DAI_XR_MEDIA4,
	FK_FRONTEND_DAI_XR_MEDIA5,
	FK_FRONTEND_DAI_XR_MEDIA6,
	FK_FRONTEND_DAI_XR_MEDIA7,
	FK_FRONTEND_DAI_XR_VOICE,
	FK_FRONTEND_DAI_XR_LOOPBACK,
	FK_FRONTEND_DAI_XR_LSM,
	FK_FRONTEND_DAI_XR_MULTICHS,
	FK_FRONTEND_DAI_XR_SPATIAL,
	FK_FRONTEND_DAI_XR_MMAP,
	FK_FRONTEND_DAI_XR_KARAOKE,
	FK_FRONTEND_DAI_XR_INCALL,
	FK_FRONTEND_DAI_XR_HF,
	FK_FRONTEND_DAI_XR_ASR,
	FK_FRONTEND_DAI_MAX,
};

struct kctrl_item {
	const char *name;
	int val;
	int scene_id;
	int stream_id;
	int port_id;
	int port_dir;
};

struct map_item {
	const char *name;
	int scene_id;
	int stream_id;
	struct channel_map_info *map_info;
};
#endif //_FK_PCM_ROUTING_
