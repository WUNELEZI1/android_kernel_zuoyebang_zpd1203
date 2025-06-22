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

#include "../common/fk-common-ctrl.h"
#include "../common/fk-audio-log.h"
#include "../acore/fk-acore-utils.h"
#include "../common/fk-audio-pinctrl.h"

#include "fk-pcm-routing.h"
#include "../common/audio-trace.h"

#define DRV_NAME "fk-pcm-routing"

//#define AUDIO_ROUTES_RESERVE

static int fk_kctrl_val_get_by_name(const char *name);
static int fk_kctrl_val_set_by_name(const char *name, int val);
static struct map_item *fk_channel_map_get_by_name
	(const char *name);
static int fk_routing_update_by_name(const char *name, int val);
static int fk_aif_format_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo);
static int fk_aif_format_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol);
static int fk_aif_format_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol);
static int fk_routing_ec_reference(const char *name, int val);
static int fk_channel_map_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo);
static int fk_channel_map_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol);
static int fk_channel_map_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol);


#define FK_AIF_FMT_CONTROL(xname, port_id)				\
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname,	\
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,			\
	.info = fk_aif_format_info, .get = fk_aif_format_get,	\
	.put = fk_aif_format_put, .private_value = port_id}

static const struct snd_kcontrol_new fk_aif_fmt_controls[] = {
	/* audio i2s interface format kcontrol */
	FK_AIF_FMT_CONTROL("FK_XR_I2S0_RX", FK_XR_I2S0_RX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S0_TX", FK_XR_I2S0_TX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S1_RX", FK_XR_I2S1_RX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S1_TX", FK_XR_I2S1_TX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S2_RX", FK_XR_I2S2_RX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S2_TX", FK_XR_I2S2_TX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S3_RX", FK_XR_I2S3_RX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S3_TX", FK_XR_I2S3_TX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S4_RX", FK_XR_I2S4_RX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S4_TX", FK_XR_I2S4_TX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S5_RX", FK_XR_I2S5_RX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S5_TX", FK_XR_I2S5_TX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S6_RX", FK_XR_I2S6_RX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S6_TX", FK_XR_I2S6_TX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S7_RX", FK_XR_I2S7_RX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S7_TX", FK_XR_I2S7_TX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S8_RX", FK_XR_I2S8_RX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S8_TX", FK_XR_I2S8_TX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S9_RX", FK_XR_I2S9_RX),
	FK_AIF_FMT_CONTROL("FK_XR_I2S9_TX", FK_XR_I2S9_TX),
	/* audio vad i2s interface format kcontrol */
	FK_AIF_FMT_CONTROL("FK_XR_VAD_I2S_TX", FK_XR_VAD_I2S_TX),
	/*audio mi2s interface format kcontrol*/
	FK_AIF_FMT_CONTROL("FK_XR_MI2S0_RX", FK_XR_MI2S0_RX),
	/* audio usb interface format kcontrol */
	FK_AIF_FMT_CONTROL("FK_XR_AUDIO_USB_RX", FK_XR_AUDIO_USB_RX),
	FK_AIF_FMT_CONTROL("FK_XR_AUDIO_USB_TX", FK_XR_AUDIO_USB_TX),
	/* audio tdm interface format kcontrol */
	FK_AIF_FMT_CONTROL("FK_XR_TDM0_RX", FK_XR_TDM0_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM0_TX", FK_XR_TDM0_TX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM1_RX", FK_XR_TDM1_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM1_TX", FK_XR_TDM1_TX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM2_RX", FK_XR_TDM2_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM2_TX", FK_XR_TDM2_TX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM3_RX", FK_XR_TDM3_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM3_TX", FK_XR_TDM3_TX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM4_RX", FK_XR_TDM4_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM4_TX", FK_XR_TDM4_TX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM5_RX", FK_XR_TDM5_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM5_TX", FK_XR_TDM5_TX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM6_RX", FK_XR_TDM6_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM6_TX", FK_XR_TDM6_TX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM7_RX", FK_XR_TDM7_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM7_TX", FK_XR_TDM7_TX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM8_RX", FK_XR_TDM8_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM8_TX", FK_XR_TDM8_TX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM9_RX", FK_XR_TDM9_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM9_TX", FK_XR_TDM9_TX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM7_8_RX", FK_XR_TDM7_8_RX),
	FK_AIF_FMT_CONTROL("FK_XR_TDM7_8_TX", FK_XR_TDM7_8_TX),
};

/* Default configuration of rx channel map */
static struct channel_map_info channel_map_cfg_rx[] = {
	[XR_MEDIA2] = {CHANNEL_NUM_MAX, {CHAN_1, CHAN_2, CHAN_NONE,
		CHAN_NONE, CHAN_NONE, CHAN_NONE, CHAN_NONE, CHAN_NONE}},
};

#define FK_CHANNEL_MAP_CONTROL(xname)				\
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname,	\
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,			\
	.info = fk_channel_map_info, .get = fk_channel_map_get,	\
	.put = fk_channel_map_put, .private_value = 0}

static const struct snd_kcontrol_new fk_channel_map_controls[] = {
	FK_CHANNEL_MAP_CONTROL("XR_media2 Out Channel Map"),
};

static struct map_item map_item_talbe[] = {
	/* map table */
	/*XR_MEDIA2 playback*/
	{.name = "XR_media2 Out Channel Map", .scene_id = SCENE_PCM_PLAYBACK,
		.stream_id = XR_MEDIA2, .map_info = &channel_map_cfg_rx[XR_MEDIA2]},
};

static struct kctrl_item kctrl_item_talbe[] = {
	/* tx table */
	/*i2s0_tx*/
	{.name = "XR_media0 Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_I2S0_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_I2S0_TX, .port_dir = FK_BE_PORT_TX},
	/*i2s1_tx*/
	{.name = "XR_media0 Mixer XR_I2S1_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_I2S1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_I2S1_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_I2S1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_I2S1_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_I2S1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_I2S1_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_I2S1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_I2S1_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_I2S1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_I2S1_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_I2S1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_I2S1_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_I2S1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_I2S1_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_I2S1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_I2S1_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_I2S1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_I2S1_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_I2S1_TX, .port_dir = FK_BE_PORT_TX},
	/*i2s2_tx*/
	{.name = "XR_media0 Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_voicetrigger Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_LSM,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_I2S2_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_I2S2_TX, .port_dir = FK_BE_PORT_TX},

	/*i2s3_tx*/
	{.name = "XR_media0 Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_incall Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_INCALL_CAPTURE, .stream_id = XR_INCALL,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_I2S3_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_I2S3_TX, .port_dir = FK_BE_PORT_TX},

	/*i2s4_tx*/
	{.name = "XR_media0 Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_voicetrigger Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_LSM,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_incall Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_INCALL_CAPTURE, .stream_id = XR_INCALL,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_I2S4_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_I2S4_TX, .port_dir = FK_BE_PORT_TX},

	/*i2s9_tx*/
	{.name = "XR_voicetrigger Mixer XR_I2S9_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_LSM,
		.port_id = FK_XR_I2S9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_asr Mixer XR_I2S9_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_ASR,
		.port_id = FK_XR_I2S9_TX, .port_dir = FK_BE_PORT_TX},

	/*vad_i2s_tx*/
	{.name = "XR_voicetrigger Mixer XR_VAD_I2S_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_LSM,
		.port_id = FK_XR_VAD_I2S_TX, .port_dir = FK_BE_PORT_TX},

	/*tdm0_tx*/
	{.name = "XR_media0 Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM0_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM0_TX, .port_dir = FK_BE_PORT_TX},

	/*tdm1_tx*/
	{.name = "XR_media0 Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM1_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM1_TX, .port_dir = FK_BE_PORT_TX},
	/*tdm2_tx*/
	{.name = "XR_media0 Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM2_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM2_TX, .port_dir = FK_BE_PORT_TX},
	/*tdm3_tx*/
	{.name = "XR_media0 Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM3_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM3_TX, .port_dir = FK_BE_PORT_TX},
	/*tdm4_tx*/
	{.name = "XR_media0 Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM4_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM4_TX, .port_dir = FK_BE_PORT_TX},
	/*tdm5_tx*/
	{.name = "XR_media0 Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM5_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM5_TX, .port_dir = FK_BE_PORT_TX},
	/*tdm6_tx*/
	{.name = "XR_media0 Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM6_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM6_TX, .port_dir = FK_BE_PORT_TX},
	/*tdm7_tx*/
	{.name = "XR_media0 Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_voicetrigger Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_LSM,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_asr Mixer XR_TDM7_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_ASR,
		.port_id = FK_XR_TDM7_TX, .port_dir = FK_BE_PORT_TX},
	/*tdm8_tx*/
	{.name = "XR_media0 Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM8_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM8_TX, .port_dir = FK_BE_PORT_TX},
	/*tdm9_tx*/
	{.name = "XR_media0 Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_voicetrigger Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_LSM,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_incall Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_INCALL_CAPTURE, .stream_id = XR_INCALL,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_asr Mixer XR_TDM9_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_ASR,
		.port_id = FK_XR_TDM9_TX, .port_dir = FK_BE_PORT_TX},
	/*tdm7_8_tx*/
	{.name = "XR_media0 Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_TDM7_8_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_TDM7_8_TX, .port_dir = FK_BE_PORT_TX},
#ifdef FK_AUDIO_USB_OFFLOAD
	/*audio_usb_tx*/
	{.name = "XR_media0 Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media1 Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media2 Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media3 Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_PCM_CAPTURE, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media4 Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_COMPR_CAPTURE, .stream_id = XR_MEDIA4,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_media6 Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_VOIP_CAPTURE, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Voice Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_VOICE_CAPTURE, .stream_id = XR_VOICE,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Loopback Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_mmap Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_MMAP_CAPTURE, .stream_id = XR_MMAP,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_Karaoke Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_KARAOKE_CAPTURE, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
	{.name = "XR_hf Mixer XR_AUDIO_USB_TX", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_AUDIO_USB_TX, .port_dir = FK_BE_PORT_TX},
#endif
	/* rx table */
	/*i2s0_rx*/
	{.name = "XR_I2S0_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S0_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_CAPTURE, .stream_id = XR_HF,
		.port_id = FK_XR_I2S0_RX, .port_dir = FK_BE_PORT_RX},
	/*i2s1_rx*/
	{.name = "XR_I2S1_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S1_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S1_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S1_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S1_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S1_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S1_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S1_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S1_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S1_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S1_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_I2S1_RX, .port_dir = FK_BE_PORT_RX},
	/*i2s2_rx*/
	{.name = "XR_I2S2_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_media7", .val = 0,
		.scene_id = SCENE_HAPTIC, .stream_id = XR_MEDIA7,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_incall", .val = 0,
		.scene_id = SCENE_INCALL_PLAYBACK, .stream_id = XR_INCALL,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	/*i2s3_rx*/
	{.name = "XR_I2S3_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S3_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_I2S3_RX, .port_dir = FK_BE_PORT_RX},
	/*i2s4_rx*/
	{.name = "XR_I2S4_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S4_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S4_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S4_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S4_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S4_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S4_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S4_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S4_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S4_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S4_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_I2S4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_incall", .val = 0,
		.scene_id = SCENE_INCALL_PLAYBACK, .stream_id = XR_INCALL,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_I2S2_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_I2S2_RX, .port_dir = FK_BE_PORT_RX},

	/*mi2s0 for dp*/
	{.name = "XR_MI2S0_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_MI2S0_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_MI2S0_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_MI2S0_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_MI2S0_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_MI2S0_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_MI2S0_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_MI2S0_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_MI2S0_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_MI2S0_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_MI2S0_RX, .port_dir = FK_BE_PORT_RX},

	/*tdm0_rx*/
	{.name = "XR_TDM0_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM0_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM0_RX, .port_dir = FK_BE_PORT_RX},
	/*tdm1_rx*/
	{.name = "XR_TDM1_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM1_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM1_RX, .port_dir = FK_BE_PORT_RX},
	/*tdm2_rx*/
	{.name = "XR_TDM2_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM2_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM2_RX, .port_dir = FK_BE_PORT_RX},
	/*tdm3_rx*/
	{.name = "XR_TDM3_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM3_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM3_RX, .port_dir = FK_BE_PORT_RX},
	/*tdm4_rx*/
	{.name = "XR_TDM4_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM4_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM4_RX, .port_dir = FK_BE_PORT_RX},
	/*tdm5_rx*/
	{.name = "XR_TDM5_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM5_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM5_RX, .port_dir = FK_BE_PORT_RX},
	/*tdm6_rx*/
	{.name = "XR_TDM6_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM6_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM6_RX, .port_dir = FK_BE_PORT_RX},
	/*tdm7_rx*/
	{.name = "XR_TDM7_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_media7", .val = 0,
		.scene_id = SCENE_HAPTIC, .stream_id = XR_MEDIA7,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_incall", .val = 0,
		.scene_id = SCENE_INCALL_PLAYBACK, .stream_id = XR_INCALL,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM7_RX, .port_dir = FK_BE_PORT_RX},
	/*tdm8_rx*/
	{.name = "XR_TDM8_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM8_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM8_RX, .port_dir = FK_BE_PORT_RX},
	/*tdm9_rx*/
	{.name = "XR_TDM9_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_incall", .val = 0,
		.scene_id = SCENE_INCALL_PLAYBACK, .stream_id = XR_INCALL,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM9_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM9_RX, .port_dir = FK_BE_PORT_RX},
	/*tdm7_8_rx*/
	{.name = "XR_TDM7_8_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_TDM7_8_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_TDM7_8_RX, .port_dir = FK_BE_PORT_RX},

#ifdef FK_AUDIO_USB_OFFLOAD
	/*audio_usb_rx*/
	{.name = "XR_AUDIO_USB_RX Mixer XR_media0", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA0,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_media1", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA1,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_media2", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA2,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_media3", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_MEDIA3,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_media5", .val = 0,
		.scene_id = SCENE_COMPR_PLAYBACK, .stream_id = XR_MEDIA5,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_media6", .val = 0,
		.scene_id = SCENE_VOIP_PLAYBACK, .stream_id = XR_MEDIA6,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_spatial", .val = 0,
		.scene_id = SCENE_SPATIAL, .stream_id = XR_SPATIAL,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_Voice", .val = 0,
		.scene_id = SCENE_VOICE_PLAYBACK, .stream_id = XR_VOICE,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_Loopback", .val = 0,
		.scene_id = SCENE_LOOPBACK, .stream_id = 0,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_mmap", .val = 0,
		.scene_id = SCENE_MMAP_PLAYBACK, .stream_id = XR_MMAP,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_Karaoke", .val = 0,
		.scene_id = SCENE_KARAOKE_PLAYBACK, .stream_id = XR_KARAOKE,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_direct", .val = 0,
		.scene_id = SCENE_PCM_PLAYBACK, .stream_id = XR_DIRECT,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
	{.name = "XR_AUDIO_USB_RX Mixer XR_hf", .val = 0,
		.scene_id = SCENE_HF_PLAYBACK, .stream_id = XR_HF,
		.port_id = FK_XR_AUDIO_USB_RX, .port_dir = FK_BE_PORT_RX},
#endif
};

/***************************** function begin **************************************/

static int fk_aif_format_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 4;

	return 0;
}

static int fk_aif_format_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int port_id = (int)kcontrol->private_value;
	char *sname = (char *)&ucontrol->id.name;
	int idx = ucontrol->id.index;
	struct dai_format *aif_cfg = NULL;

	AUD_LOG_DBG(AUD_SOC, "name:%s port_id:%d idx:%d",
		sname, port_id, idx);

	aif_cfg = fk_aif_dev_cfg_get(port_id);
	if (aif_cfg == NULL)
		return 1;

	ucontrol->value.integer.value[0] = aif_cfg->samples;
	ucontrol->value.integer.value[1] = aif_cfg->bit_width;
	ucontrol->value.integer.value[2] = aif_cfg->channels;
	ucontrol->value.integer.value[3] = aif_cfg->pcm_fmt;

	AUD_LOG_DBG(AUD_SOC, "rate:%ld bits:%ld chs:%ld fmt:%ld",
		ucontrol->value.integer.value[0],
		ucontrol->value.integer.value[1],
		ucontrol->value.integer.value[2],
		ucontrol->value.integer.value[3]);

	return 0;
}

static int fk_aif_format_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int port_id = (int)kcontrol->private_value;
	char *sname = (char *)&ucontrol->id.name;
	int idx = ucontrol->id.index;
	struct dai_format *aif_cfg = NULL;

	AUD_LOG_INFO(AUD_SOC, "name:%s port_id:%d idx:%d",
		sname, port_id, idx);
	AUD_LOG_INFO(AUD_SOC, "rate:%ld bits:%ld chs:%ld fmt:%ld",
		ucontrol->value.integer.value[0],
		ucontrol->value.integer.value[1],
		ucontrol->value.integer.value[2],
		ucontrol->value.integer.value[3]);

	aif_cfg = fk_aif_dev_cfg_get(port_id);
	if (aif_cfg == NULL)
		return 1;

	aif_cfg->samples = ucontrol->value.integer.value[0];
	aif_cfg->bit_width = ucontrol->value.integer.value[1];
	aif_cfg->channels = ucontrol->value.integer.value[2];
	aif_cfg->pcm_fmt = ucontrol->value.integer.value[3];

	return 1;
}


static int fk_channel_map_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 17;

	return 0;
}

static int fk_channel_map_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	char *sname = (char *)&ucontrol->id.name;
	int idx = ucontrol->id.index;
	struct map_item *item = NULL;
	struct channel_map_info *map_info = NULL;
	int i;

	AUD_LOG_DBG(AUD_SOC, "name:%s idx:%d", sname, idx);

	item = fk_channel_map_get_by_name(kcontrol->id.name);
	if (!item) {
		AUD_LOG_INFO(AUD_SOC, "unfound");
		return 0;
	}

	map_info = item->map_info;
	ucontrol->value.integer.value[0] = map_info->num;
	for (i = 0; i < CHANNEL_NUM_MAX; i++)
		ucontrol->value.integer.value[1 + i] = map_info->info[i];

	return 0;
}

static int fk_channel_map_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	char *sname = (char *)&ucontrol->id.name;
	int idx = ucontrol->id.index;
	struct channel_map_info *map_info = NULL;
	struct map_item *item = NULL;
	int i;

	AUD_LOG_DBG(AUD_SOC, "name:%s idx:%d", sname, idx);

	item = fk_channel_map_get_by_name(kcontrol->id.name);
	if (item == NULL) {
		AUD_LOG_INFO(AUD_SOC, "unfound");
		return 1;
	}

	map_info = item->map_info;
	map_info->num = ucontrol->value.integer.value[0];

	if (map_info->num > CHANNEL_NUM_MAX) {
		AUD_LOG_INFO(AUD_SOC, "invalid channel map num");
		return 1;
	}

	for (i = 0; i < map_info->num; i++)
		map_info->info[i] = ucontrol->value.integer.value[1 + i];

	return 1;
}

static int fk_routing_get_single_mixer(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int val = 0;

	val = fk_kctrl_val_get_by_name(kcontrol->id.name);
	if (val < 0)
		ucontrol->value.integer.value[0] = 0;
	else
		ucontrol->value.integer.value[0] = val;

	return 0;
}

static int fk_routing_put_single_mixer(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct snd_soc_dapm_update *update = NULL;
	struct snd_soc_dapm_widget *widget =
		snd_soc_dapm_kcontrol_widget(kcontrol);

#ifdef ATRACE_MACRO
	AUD_TRACE("enter. name(%s)", kcontrol->id.name);
#endif

	if (ucontrol->value.integer.value[0]) {
		fk_kctrl_val_set_by_name(kcontrol->id.name, 1);
		/* update BE route info */
		ret = fk_routing_update_by_name(kcontrol->id.name, 1);
		if (!ret)
			snd_soc_dapm_mixer_update_power(widget->dapm, kcontrol, 1,
				update);
		else {
			AUD_LOG_ERR(AUD_SOC, "enable audio route failed");
			return -1;
		}
	} else {
		fk_kctrl_val_set_by_name(kcontrol->id.name, 0);
		/* update BE route info */
		ret = fk_routing_update_by_name(kcontrol->id.name, 0);
		if (!ret)
			snd_soc_dapm_mixer_update_power(widget->dapm, kcontrol, 0,
				update);
		else {
			AUD_LOG_ERR(AUD_SOC, "disable audio route failed");
			return -1;
		}
	}

	AUD_LOG_DBG(AUD_SOC, "name:%s val:%ld", kcontrol->id.name,
		ucontrol->value.integer.value[0]);

#ifdef ATRACE_MACRO
	AUD_TRACE("exit");
#endif

	return 1;
}

static int fk_routing_get_ec_single_mixer(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int val = 0;

	val = fk_kctrl_val_get_by_name(kcontrol->id.name);
	if (val < 0)
		ucontrol->value.integer.value[0] = 0;
	else
		ucontrol->value.integer.value[0] = val;

	return 0;
}

static int fk_routing_put_ec_single_mixer(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_dapm_update *update = NULL;
	struct snd_soc_dapm_widget *widget =
		snd_soc_dapm_kcontrol_widget(kcontrol);
	int val = ucontrol->value.integer.value[0];

	AUD_LOG_DBG(AUD_SOC, "ucontrol->value.integer.value[0]=%ld", ucontrol->value.integer.value[0]);
	if (ucontrol->value.integer.value[0]) {
		fk_kctrl_val_set_by_name(kcontrol->id.name, val);
		snd_soc_dapm_mixer_update_power(widget->dapm, kcontrol, 1,
			update);
		/*enable EC reference */
		fk_routing_ec_reference(kcontrol->id.name, val);
	} else {
		fk_kctrl_val_set_by_name(kcontrol->id.name, val);
		snd_soc_dapm_mixer_update_power(widget->dapm, kcontrol, 0,
			update);

		/* disable EC reference */
		fk_routing_ec_reference(kcontrol->id.name, val);
	}
	AUD_LOG_DBG(AUD_SOC, "name:%s complete", kcontrol->id.name);

	return 1;
}


static const struct snd_kcontrol_new i2s0_rx_mixer_controls[] = {
	/* audio rx stream to i2s0*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to i2s0 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to i2s0 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to i2s0 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_I2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new i2s1_rx_mixer_controls[] = {
	/* audio rx stream to i2s1*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to i2s1 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to i2s1 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to i2s1 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_I2S1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new i2s2_rx_mixer_controls[] = {
	/* audio rx stream to i2s2*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media7", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to i2s2 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to i2s2 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to i2s2 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* incall rx stream to i2s2 */
	SOC_SINGLE_EXT("XR_incall", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* hf rx stream to i2s2 */
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_I2S2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new i2s3_rx_mixer_controls[] = {
	/* audio rx stream to i2s3*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to i2s3 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to i2s3 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to i2s3 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_I2S3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new i2s4_rx_mixer_controls[] = {
	/* audio rx stream to i2s4*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to i2s4 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to i2s4 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to i2s4 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* incall rx stream to i2s4 */
	SOC_SINGLE_EXT("XR_incall", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* hf rx stream to i2s4 */
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_I2S4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

#ifdef AUDIO_ROUTES_RESERVE
static const struct snd_kcontrol_new tdm0_rx_mixer_controls[] = {
	/* audio rx stream to tdm0*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm0 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm0 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm0 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* direct rx stream to tdm0*/
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new tdm1_rx_mixer_controls[] = {
	/* audio rx stream to tdm1*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm1 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm1 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm1 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* direct rx stream to tdm1*/
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM1_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new tdm2_rx_mixer_controls[] = {
	/* audio rx stream to tdm2*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm2 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm2 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm2 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* direct rx stream to tdm2*/
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM2_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new tdm3_rx_mixer_controls[] = {
	/* audio rx stream to tdm3*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm3 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm3 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm3 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* direct rx stream to tdm3*/
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM3_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new tdm4_rx_mixer_controls[] = {
	/* audio rx stream to tdm4*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm4 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm4 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm4 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* direct rx stream to tdm5*/
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM4_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new tdm5_rx_mixer_controls[] = {
	/* audio rx stream to tdm5*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm5 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm5 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm5 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* direct rx stream to tdm5*/
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM5_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new tdm6_rx_mixer_controls[] = {
	/* audio rx stream to tdm6*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm6 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm6 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm6 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* direct rx stream to tdm6*/
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM6_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};
#endif

static const struct snd_kcontrol_new tdm7_rx_mixer_controls[] = {
	/* audio rx stream to tdm7*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media7", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm7 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm7 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm7 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* incall rx stream to tdm7 */
	SOC_SINGLE_EXT("XR_incall", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* direct rx stream to tdm7*/
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM7_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new tdm8_rx_mixer_controls[] = {
	/* audio rx stream to tdm8*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm8 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm8 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm8 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* direct rx stream to tdm8*/
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new tdm9_rx_mixer_controls[] = {
	/* audio rx stream to tdm9*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm9 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm9 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm9 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* incall rx stream to tdm9 */
	SOC_SINGLE_EXT("XR_incall", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* direct rx stream to tdm9*/
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM9_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new tdm7_8_rx_mixer_controls[] = {
	/* audio rx stream to tdm7_8*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* voice rx stream to tdm7_8 */
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* loopback rx stream to tdm7_8 */
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* mmap rx stream to tdm7_8 */
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_TDM7_8_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

#ifdef FK_AUDIO_USB_OFFLOAD
static const struct snd_kcontrol_new audio_usb_rx_mixer_controls[] = {
	/* audio rx stream to audio usb*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Voice", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Loopback", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_hf", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};
#endif

static const struct snd_kcontrol_new mi2s0_rx_mixer_controls[] = {
	/* mi2s0 for dp*/
	SOC_SINGLE_EXT("XR_media0", SND_SOC_NOPM,
	FK_XR_MI2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media1", SND_SOC_NOPM,
	FK_XR_MI2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media2", SND_SOC_NOPM,
	FK_XR_MI2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media3", SND_SOC_NOPM,
	FK_XR_MI2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media5", SND_SOC_NOPM,
	FK_XR_MI2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_media6", SND_SOC_NOPM,
	FK_XR_MI2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_spatial", SND_SOC_NOPM,
	FK_XR_MI2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_mmap", SND_SOC_NOPM,
	FK_XR_MI2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_Karaoke", SND_SOC_NOPM,
	FK_XR_MI2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_direct", SND_SOC_NOPM,
	FK_XR_MI2S0_RX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new xr_ul0_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S1_TX", SND_SOC_NOPM,
	FK_XR_I2S1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
#ifdef AUDIO_ROUTES_RESERVE
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_ul1_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S1_TX", SND_SOC_NOPM,
	FK_XR_I2S1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
#ifdef AUDIO_ROUTES_RESERVE
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_ul2_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S1_TX", SND_SOC_NOPM,
	FK_XR_I2S1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
#ifdef AUDIO_ROUTES_RESERVE
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_ul3_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S1_TX", SND_SOC_NOPM,
	FK_XR_I2S1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
#ifdef AUDIO_ROUTES_RESERVE
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_ul4_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S1_TX", SND_SOC_NOPM,
	FK_XR_I2S1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
#ifdef AUDIO_ROUTES_RESERVE
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_ul6_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S1_TX", SND_SOC_NOPM,
	FK_XR_I2S1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
#ifdef AUDIO_ROUTES_RESERVE
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_voice_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S1_TX", SND_SOC_NOPM,
	FK_XR_I2S1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
#ifdef AUDIO_ROUTES_RESERVE
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_karaoke_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S1_TX", SND_SOC_NOPM,
	FK_XR_I2S1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
#ifdef AUDIO_ROUTES_RESERVE
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_lp_tx_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S1_TX", SND_SOC_NOPM,
	FK_XR_I2S1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
#ifdef AUDIO_ROUTES_RESERV
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_mmap_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S1_TX", SND_SOC_NOPM,
	FK_XR_I2S1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
#ifdef AUDIO_ROUTES_RESERVE
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_incall_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
};

static const struct snd_kcontrol_new xr_hf_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S0_TX", SND_SOC_NOPM,
	FK_XR_I2S0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S2_TX", SND_SOC_NOPM,
	FK_XR_I2S2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S3_TX", SND_SOC_NOPM,
	FK_XR_I2S3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	/* tdm tx */
#ifdef AUDIO_ROUTES_RESERVE
	SOC_SINGLE_EXT("XR_TDM0_TX", SND_SOC_NOPM,
	FK_XR_TDM0_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM1_TX", SND_SOC_NOPM,
	FK_XR_TDM1_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM2_TX", SND_SOC_NOPM,
	FK_XR_TDM2_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM3_TX", SND_SOC_NOPM,
	FK_XR_TDM3_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM4_TX", SND_SOC_NOPM,
	FK_XR_TDM4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM5_TX", SND_SOC_NOPM,
	FK_XR_TDM5_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM6_TX", SND_SOC_NOPM,
	FK_XR_TDM6_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM8_TX", SND_SOC_NOPM,
	FK_XR_TDM8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_8_TX", SND_SOC_NOPM,
	FK_XR_TDM7_8_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#ifdef FK_AUDIO_USB_OFFLOAD
	SOC_SINGLE_EXT("XR_AUDIO_USB_TX", SND_SOC_NOPM,
	FK_XR_AUDIO_USB_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
#endif
};

static const struct snd_kcontrol_new xr_vt_tx_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_VAD_I2S_TX", SND_SOC_NOPM,
	FK_XR_VAD_I2S_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S9_TX", SND_SOC_NOPM,
	FK_XR_I2S9_TX, 5, 0, fk_routing_get_ec_single_mixer,
	fk_routing_put_ec_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 5, 0, fk_routing_get_ec_single_mixer,
	fk_routing_put_ec_single_mixer),
};

static const struct snd_kcontrol_new xr_asr_tx_mixer_controls[] = {
	SOC_SINGLE_EXT("XR_I2S4_TX", SND_SOC_NOPM,
	FK_XR_I2S4_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_TDM9_TX", SND_SOC_NOPM,
	FK_XR_TDM9_TX, 1, 0, fk_routing_get_single_mixer,
	fk_routing_put_single_mixer),
	SOC_SINGLE_EXT("XR_I2S9_TX", SND_SOC_NOPM,
	FK_XR_I2S9_TX, 5, 0, fk_routing_get_ec_single_mixer,
	fk_routing_put_ec_single_mixer),
	SOC_SINGLE_EXT("XR_TDM7_TX", SND_SOC_NOPM,
	FK_XR_TDM7_TX, 5, 0, fk_routing_get_ec_single_mixer,
	fk_routing_put_ec_single_mixer),
};


static const struct snd_soc_dapm_widget fk_widgets[] = {
	/* Frontend AIF */
	/* Widget name equals to Front-End DAI name<Need confirmation>,
	 * Stream name must contains substring of front-end dai name
	 */
	/*DL widgtes*/
	SND_SOC_DAPM_AIF_IN("XR_DL0", "XR_media0 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_DL1", "XR_media1 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_DL2", "XR_media2 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_DL3", "XR_media3 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_DL5", "XR_media5 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_DL6", "XR_media6 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_DL7", "XR_media7 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_DL8", "XR_spatial Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_Voice_DL", "XR_Voice DL", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_Loopback_DL", "XR_Loopback DL", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_Karaoke_DL", "XR_Karaoke Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_MDL", "XR_direct Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_MMAP_DL", "XR_mmap Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_INCALL_DL", "XR_incall Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_HF_DL", "XR_hf Playback", 0, 0, 0, 0),

	/*UL widgets*/
	SND_SOC_DAPM_AIF_OUT("XR_UL0", "XR_media0 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_UL1", "XR_media1 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_UL2", "XR_media2 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_UL3", "XR_media3 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_UL4", "XR_media4 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_UL6", "XR_media6 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_Voice_UL", "XR_Voice UL", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_Loopback_UL", "XR_Loopback UL", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_Karaoke_UL", "XR_Karaoke Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_voicetrigger_UL", "XR_voicetrigger", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_MMAP_UL", "XR_mmap Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_INCALL_UL", "XR_incall Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_HF_UL", "XR_hf Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_asr_UL", "XR_asr Capture", 0, 0, 0, 0),

	/*mixer*/
	SND_SOC_DAPM_MIXER("XR_media0 Mixer", SND_SOC_NOPM, 0, 0,
			   xr_ul0_mixer_controls,
			   ARRAY_SIZE(xr_ul0_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_media1 Mixer", SND_SOC_NOPM, 0, 0,
			   xr_ul1_mixer_controls,
			   ARRAY_SIZE(xr_ul1_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_media2 Mixer", SND_SOC_NOPM, 0, 0,
			   xr_ul2_mixer_controls,
			   ARRAY_SIZE(xr_ul2_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_media3 Mixer", SND_SOC_NOPM, 0, 0,
			   xr_ul3_mixer_controls,
			   ARRAY_SIZE(xr_ul3_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_media4 Mixer", SND_SOC_NOPM, 0, 0,
			   xr_ul4_mixer_controls,
			   ARRAY_SIZE(xr_ul4_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_media6 Mixer", SND_SOC_NOPM, 0, 0,
			   xr_ul6_mixer_controls,
			   ARRAY_SIZE(xr_ul6_mixer_controls)),

	SND_SOC_DAPM_MIXER("XR_Voice Mixer", SND_SOC_NOPM, 0, 0,
			   xr_voice_mixer_controls,
			   ARRAY_SIZE(xr_voice_mixer_controls)),

	SND_SOC_DAPM_MIXER("XR_Loopback Mixer", SND_SOC_NOPM, 0, 0,
		   xr_lp_tx_mixer_controls,
		   ARRAY_SIZE(xr_lp_tx_mixer_controls)),

	SND_SOC_DAPM_MIXER("XR_voicetrigger Mixer", SND_SOC_NOPM, 0, 0,
		   xr_vt_tx_mixer_controls,
		   ARRAY_SIZE(xr_vt_tx_mixer_controls)),

	SND_SOC_DAPM_MIXER("XR_mmap Mixer", SND_SOC_NOPM, 0, 0,
		   xr_mmap_mixer_controls,
		   ARRAY_SIZE(xr_mmap_mixer_controls)),

	SND_SOC_DAPM_MIXER("XR_Karaoke Mixer", SND_SOC_NOPM, 0, 0,
		   xr_karaoke_mixer_controls,
		   ARRAY_SIZE(xr_karaoke_mixer_controls)),

	SND_SOC_DAPM_MIXER("XR_incall Mixer", SND_SOC_NOPM, 0, 0,
		   xr_incall_mixer_controls,
		   ARRAY_SIZE(xr_incall_mixer_controls)),

	SND_SOC_DAPM_MIXER("XR_hf Mixer", SND_SOC_NOPM, 0, 0,
		   xr_hf_mixer_controls,
		   ARRAY_SIZE(xr_hf_mixer_controls)),

	SND_SOC_DAPM_MIXER("XR_asr Mixer", SND_SOC_NOPM, 0, 0,
		   xr_asr_tx_mixer_controls,
		   ARRAY_SIZE(xr_asr_tx_mixer_controls)),
};

static const struct snd_soc_dapm_widget fk_widgets_i2s[] = {
/*RX widgets*/
	SND_SOC_DAPM_AIF_OUT("XR_I2S1_RX", "XR_I2S1 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_I2S2_RX", "XR_I2S2 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_I2S3_RX", "XR_I2S3 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_MI2S0_RX", "XR_MI2S0 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM7_RX", "XR_TDM7 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM8_RX", "XR_TDM8 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM9_RX", "XR_TDM9 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM7_8_RX", "XR_TDM7_8 Playback", 0, 0, 0, 0),
#ifdef FK_AUDIO_USB_OFFLOAD
	SND_SOC_DAPM_AIF_OUT("XR_AUDIO_USB_RX", "XR_AUDIO_USB Playback", 0, 0, 0, 0),
#endif

#ifdef AUDIO_ROUTES_RESERVE
	SND_SOC_DAPM_AIF_OUT("XR_I2S0_RX", "XR_I2S0 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_I2S4_RX", "XR_I2S4 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_I2S5_RX", "XR_I2S5 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_I2S6_RX", "XR_I2S6 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_I2S7_RX", "XR_I2S7 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_I2S8_RX", "XR_I2S8 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_I2S9_RX", "XR_I2S9 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM0_RX", "XR_TDM0 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM1_RX", "XR_TDM1 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM2_RX", "XR_TDM2 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM3_RX", "XR_TDM3 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM4_RX", "XR_TDM4 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM5_RX", "XR_TDM5 Playback", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_OUT("XR_TDM6_RX", "XR_TDM6 Playback", 0, 0, 0, 0),
#endif
/*TX widgets*/
	SND_SOC_DAPM_AIF_IN("XR_I2S1_TX", "XR_I2S1 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_I2S2_TX", "XR_I2S2 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_I2S3_TX", "XR_I2S3 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_VAD_I2S_TX", "XR_VAD_I2S capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM7_TX", "XR_TDM7 capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM8_TX", "XR_TDM8 capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM9_TX", "XR_TDM9 capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM7_8_TX", "XR_TDM7_8 capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_I2S9_TX", "XR_I2S9 Capture", 0, 0, 0, 0),
#ifdef FK_AUDIO_USB_OFFLOAD
	SND_SOC_DAPM_AIF_IN("XR_AUDIO_USB_TX", "XR_AUDIO_USB Capture", 0, 0, 0, 0),
#endif

#ifdef AUDIO_ROUTES_RESERVE
	SND_SOC_DAPM_AIF_IN("XR_I2S0_TX", "XR_I2S0 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_I2S4_TX", "XR_I2S4 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_I2S5_TX", "XR_I2S5 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_I2S6_TX", "XR_I2S6 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_I2S7_TX", "XR_I2S7 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_I2S8_TX", "XR_I2S8 Capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM0_TX", "XR_TDM0 capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM1_TX", "XR_TDM1 capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM2_TX", "XR_TDM2 capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM3_TX", "XR_TDM3 capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM4_TX", "XR_TDM4 capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM5_TX", "XR_TDM5 capture", 0, 0, 0, 0),
	SND_SOC_DAPM_AIF_IN("XR_TDM6_TX", "XR_TDM6 capture", 0, 0, 0, 0),
#endif
/*RX mixer*/
	SND_SOC_DAPM_MIXER("XR_I2S1_RX Mixer", SND_SOC_NOPM, 0, 0,
				i2s1_rx_mixer_controls,
				ARRAY_SIZE(i2s1_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_I2S2_RX Mixer", SND_SOC_NOPM, 0, 0,
				i2s2_rx_mixer_controls,
				ARRAY_SIZE(i2s2_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_I2S3_RX Mixer", SND_SOC_NOPM, 0, 0,
				i2s3_rx_mixer_controls,
				ARRAY_SIZE(i2s3_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_MI2S0_RX Mixer", SND_SOC_NOPM, 0, 0,
				mi2s0_rx_mixer_controls,
				ARRAY_SIZE(mi2s0_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM7_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm7_rx_mixer_controls,
				ARRAY_SIZE(tdm7_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM8_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm8_rx_mixer_controls,
				ARRAY_SIZE(tdm8_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM9_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm9_rx_mixer_controls,
				ARRAY_SIZE(tdm9_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM7_8_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm7_8_rx_mixer_controls,
				ARRAY_SIZE(tdm7_8_rx_mixer_controls)),
#ifdef FK_AUDIO_USB_OFFLOAD
	SND_SOC_DAPM_MIXER("XR_AUDIO_USB_RX Mixer", SND_SOC_NOPM, 0, 0,
				audio_usb_rx_mixer_controls,
				ARRAY_SIZE(audio_usb_rx_mixer_controls)),
#endif

#ifdef AUDIO_ROUTES_RESERVE
	SND_SOC_DAPM_MIXER("XR_I2S0_RX Mixer", SND_SOC_NOPM, 0, 0,
				i2s0_rx_mixer_controls,
				ARRAY_SIZE(i2s0_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_I2S4_RX Mixer", SND_SOC_NOPM, 0, 0,
				i2s4_rx_mixer_controls,
				ARRAY_SIZE(i2s4_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM0_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm0_rx_mixer_controls,
				ARRAY_SIZE(tdm0_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM1_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm1_rx_mixer_controls,
				ARRAY_SIZE(tdm1_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM2_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm2_rx_mixer_controls,
				ARRAY_SIZE(tdm2_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM3_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm3_rx_mixer_controls,
				ARRAY_SIZE(tdm3_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM4_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm4_rx_mixer_controls,
				ARRAY_SIZE(tdm4_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM5_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm5_rx_mixer_controls,
				ARRAY_SIZE(tdm5_rx_mixer_controls)),
	SND_SOC_DAPM_MIXER("XR_TDM6_RX Mixer", SND_SOC_NOPM, 0, 0,
				tdm6_rx_mixer_controls,
				ARRAY_SIZE(tdm6_rx_mixer_controls)),
#endif
};

static const struct snd_soc_dapm_route fk_i2s[] = {
/* audio to I2S1_RX */
	{"XR_I2S1_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_I2S1_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_I2S1_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_I2S1_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_I2S1_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_I2S1_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_I2S1_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_I2S1_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_I2S1_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_I2S1_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_I2S1_RX", NULL, "XR_I2S1_RX Mixer"},
/* audio to I2S2_RX */
	{"XR_I2S2_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_I2S2_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_I2S2_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_I2S2_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_I2S2_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_I2S2_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_I2S2_RX Mixer", "XR_media7", "XR_DL7"},
	{"XR_I2S2_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_I2S2_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_I2S2_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_I2S2_RX Mixer", "XR_incall", "XR_INCALL_DL"},
	{"XR_I2S2_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_I2S2_RX", NULL, "XR_I2S2_RX Mixer"},
/* audio to I2S3_RX */
	{"XR_I2S3_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_I2S3_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_I2S3_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_I2S3_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_I2S3_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_I2S3_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_I2S3_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_I2S3_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_I2S3_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_I2S3_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_I2S3_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_I2S3_RX", NULL, "XR_I2S3_RX Mixer"},
/* audio to TDM7_RX */
	{"XR_TDM7_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM7_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM7_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM7_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM7_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM7_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM7_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM7_RX Mixer", "XR_media7", "XR_DL7"},
	{"XR_TDM7_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM7_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM7_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_TDM7_RX Mixer", "XR_incall", "XR_INCALL_DL"},
	{"XR_TDM7_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_TDM7_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM7_RX", NULL, "XR_TDM7_RX Mixer"},
/* audio to TDM8_RX */
	{"XR_TDM8_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM8_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM8_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM8_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM8_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM8_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM8_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM8_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM8_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM8_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_TDM8_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_TDM8_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM8_RX", NULL, "XR_TDM8_RX Mixer"},
/* audio to TDM9_RX */
	{"XR_TDM9_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM9_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM9_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM9_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM9_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM9_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM9_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM9_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM9_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM9_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_TDM9_RX Mixer", "XR_incall", "XR_INCALL_DL"},
	{"XR_TDM9_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_TDM9_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM9_RX", NULL, "XR_TDM9_RX Mixer"},
/* audio to TDM7_8_RX */
	{"XR_TDM7_8_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM7_8_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM7_8_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM7_8_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM7_8_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM7_8_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM7_8_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM7_8_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM7_8_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM7_8_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM7_8_RX", NULL, "XR_TDM7_8_RX Mixer"},
/* audio to AUDIO_USB_RX */
#ifdef FK_AUDIO_USB_OFFLOAD
	{"XR_AUDIO_USB_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_AUDIO_USB_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_AUDIO_USB_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_AUDIO_USB_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_AUDIO_USB_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_AUDIO_USB_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_AUDIO_USB_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_AUDIO_USB_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_AUDIO_USB_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_AUDIO_USB_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_AUDIO_USB_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_AUDIO_USB_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_AUDIO_USB_RX", NULL, "XR_AUDIO_USB_RX Mixer"},
#endif

/*audio to MI2S0_RX*/
	{"XR_MI2S0_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_MI2S0_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_MI2S0_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_MI2S0_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_MI2S0_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_MI2S0_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_MI2S0_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_MI2S0_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_MI2S0_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_MI2S0_RX", NULL, "XR_MI2S0_RX Mixer"},
#ifdef AUDIO_ROUTES_RESERVE
/* audio to I2S0_RX */
	{"XR_I2S0_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_I2S0_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_I2S0_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_I2S0_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_I2S0_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_I2S0_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_I2S0_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_I2S0_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_I2S0_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_I2S0_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_I2S0_RX", NULL, "XR_I2S0_RX Mixer"},
/* audio to I2S4_RX */
	{"XR_I2S4_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_I2S4_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_I2S4_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_I2S4_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_I2S4_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_I2S4_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_I2S4_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_I2S4_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_I2S4_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_I2S4_RX Mixer", "XR_incall", "XR_INCALL_DL"},
	{"XR_I2S4_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_I2S4_RX", NULL, "XR_I2S4_RX Mixer"},
/* audio to TDM0_RX */
	{"XR_TDM0_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM0_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM0_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM0_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM0_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM0_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM0_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM0_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM0_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_TDM0_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_TDM0_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM0_RX", NULL, "XR_TDM0_RX Mixer"},
/* audio to TDM1_RX */
	{"XR_TDM1_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM1_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM1_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM1_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM1_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM1_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM1_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM1_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM1_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_TDM1_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_TDM1_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM1_RX", NULL, "XR_TDM1_RX Mixer"},
/* audio to TDM2_RX */
	{"XR_TDM2_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM2_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM2_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM2_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM2_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM2_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM2_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM2_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM2_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_TDM2_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_TDM2_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM2_RX", NULL, "XR_TDM2_RX Mixer"},
/* audio to TDM3_RX */
	{"XR_TDM3_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM3_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM3_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM3_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM3_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM3_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM3_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM3_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM3_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_TDM4_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_TDM3_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM3_RX", NULL, "XR_TDM3_RX Mixer"},
/* audio to TDM4_RX */
	{"XR_TDM4_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM4_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM4_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM4_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM4_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM4_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM4_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM4_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM4_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_TDM4_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_TDM4_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM4_RX", NULL, "XR_TDM4_RX Mixer"},
/* audio to TDM5_RX */
	{"XR_TDM5_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM5_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM5_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM5_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM5_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM5_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM5_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM5_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM5_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_TDM5_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_TDM0_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM5_RX", NULL, "XR_TDM5_RX Mixer"},
/* audio to TDM6_RX */
	{"XR_TDM6_RX Mixer", "XR_media0", "XR_DL0"},
	{"XR_TDM6_RX Mixer", "XR_media1", "XR_DL1"},
	{"XR_TDM6_RX Mixer", "XR_media2", "XR_DL2"},
	{"XR_TDM6_RX Mixer", "XR_media3", "XR_DL3"},
	{"XR_TDM6_RX Mixer", "XR_media5", "XR_DL5"},
	{"XR_TDM6_RX Mixer", "XR_media6", "XR_DL6"},
	{"XR_TDM6_RX Mixer", "XR_spatial", "XR_DL8"},
	{"XR_TDM6_RX Mixer", "XR_mmap", "XR_MMAP_DL"},
	{"XR_TDM6_RX Mixer", "XR_Karaoke", "XR_Karaoke_DL"},
	{"XR_TDM6_RX Mixer", "XR_direct", "XR_MDL"},
	{"XR_TDM6_RX Mixer", "XR_hf", "XR_HF_DL"},
	{"XR_TDM6_RX", NULL, "XR_TDM6_RX Mixer"},
#endif
/* I2S1_TX to audio */
	{"XR_Voice Mixer", "XR_I2S1_TX", "XR_I2S1_TX"},
	{"XR_media0 Mixer", "XR_I2S1_TX", "XR_I2S1_TX"},
	{"XR_media1 Mixer", "XR_I2S1_TX", "XR_I2S1_TX"},
	{"XR_media2 Mixer", "XR_I2S1_TX", "XR_I2S1_TX"},
	{"XR_media3 Mixer", "XR_I2S1_TX", "XR_I2S1_TX"},
	{"XR_media4 Mixer", "XR_I2S1_TX", "XR_I2S1_TX"},
	{"XR_media6 Mixer", "XR_I2S1_TX", "XR_I2S1_TX"},
	{"XR_mmap Mixer", "XR_I2S1_TX", "XR_I2S1_TX"},
	{"XR_Karaoke Mixer", "XR_I2S1_TX", "XR_I2S1_TX"},
/* I2S2_TX to audio */
	{"XR_media0 Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
	{"XR_media1 Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
	{"XR_media2 Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
	{"XR_media3 Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
	{"XR_media4 Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
	{"XR_media6 Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
	{"XR_voicetrigger Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
	{"XR_mmap Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
	{"XR_Karaoke Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
	{"XR_hf Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
/* I2S3_TX to audio */
	{"XR_Voice Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_media0 Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_media1 Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_media2 Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_media3 Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_media4 Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_media6 Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_incall Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_mmap Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_Karaoke Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_hf Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
/* TDM7_TX to audio */
	{"XR_Voice Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_media0 Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_media1 Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_media2 Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_media3 Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_media4 Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_media6 Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_voicetrigger Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_mmap Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_Karaoke Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_hf Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_asr Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
/* TDM8_TX to audio */
	{"XR_Voice Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
	{"XR_media0 Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
	{"XR_media1 Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
	{"XR_media2 Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
	{"XR_media3 Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
	{"XR_media4 Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
	{"XR_media6 Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
	{"XR_mmap Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
	{"XR_Karaoke Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
	{"XR_hf Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
/* TDM9_TX to audio */
	{"XR_Voice Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_media0 Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_media1 Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_media2 Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_media3 Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_media4 Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_media6 Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_voicetrigger Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_incall Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_mmap Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_Karaoke Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_hf Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_asr Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
/* TDM7_8_TX to audio */
	{"XR_Voice Mixer", "XR_TDM7_8_TX", "XR_TDM7_8_TX"},
	{"XR_media0 Mixer", "XR_TDM7_8_TX", "XR_TDM7_8_TX"},
	{"XR_media1 Mixer", "XR_TDM7_8_TX", "XR_TDM7_8_TX"},
	{"XR_media2 Mixer", "XR_TDM7_8_TX", "XR_TDM7_8_TX"},
	{"XR_media3 Mixer", "XR_TDM7_8_TX", "XR_TDM7_8_TX"},
	{"XR_media4 Mixer", "XR_TDM7_8_TX", "XR_TDM7_8_TX"},
	{"XR_media6 Mixer", "XR_TDM7_8_TX", "XR_TDM7_8_TX"},
	{"XR_mmap Mixer", "XR_TDM7_8_TX", "XR_TDM7_8_TX"},
	{"XR_hf Mixer", "XR_TDM7_8_TX", "XR_TDM7_8_TX"},
/* AUDIO_USB_TX to audio */
#ifdef FK_AUDIO_USB_OFFLOAD
	{"XR_Voice Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
	{"XR_media0 Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
	{"XR_media1 Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
	{"XR_media2 Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
	{"XR_media3 Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
	{"XR_media4 Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
	{"XR_media6 Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
	{"XR_mmap Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
	{"XR_Karaoke Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
	{"XR_hf Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
#endif

#ifdef AUDIO_ROUTES_RESERVE
/* I2S0_TX to audio */
	{"XR_media0 Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
	{"XR_media1 Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
	{"XR_media2 Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
	{"XR_media3 Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
	{"XR_media4 Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
	{"XR_media6 Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
	{"XR_mmap Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
	{"XR_hf Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
	{"XR_Karaoke Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
/* I2S4_TX to audio */
	{"XR_media0 Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
	{"XR_media1 Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
	{"XR_media2 Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
	{"XR_media3 Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
	{"XR_media4 Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
	{"XR_media6 Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
	{"XR_voicetrigger Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
	{"XR_mmap Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
	{"XR_Karaoke Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
	{"XR_incall Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
	{"XR_hf Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
/* TDM0_TX to audio */
	{"XR_media0 Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
	{"XR_media1 Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
	{"XR_media2 Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
	{"XR_media3 Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
	{"XR_media4 Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
	{"XR_media6 Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
	{"XR_mmap Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
	{"XR_Karaoke Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
	{"XR_hf Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
/* TDM1_TX to audio */
	{"XR_media0 Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
	{"XR_media1 Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
	{"XR_media2 Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
	{"XR_media3 Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
	{"XR_media4 Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
	{"XR_media6 Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
	{"XR_mmap Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
	{"XR_Karaoke Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
	{"XR_hf Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
/* TDM2_TX to audio */
	{"XR_media0 Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
	{"XR_media1 Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
	{"XR_media2 Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
	{"XR_media3 Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
	{"XR_media4 Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
	{"XR_media6 Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
	{"XR_mmap Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
	{"XR_Karaoke Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
	{"XR_hf Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
/* TDM3_TX to audio */
	{"XR_media0 Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
	{"XR_media1 Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
	{"XR_media2 Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
	{"XR_media3 Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
	{"XR_media4 Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
	{"XR_media6 Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
	{"XR_mmap Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
	{"XR_Karaoke Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
	{"XR_hf Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
/* TDM4_TX to audio */
	{"XR_media0 Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
	{"XR_media1 Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
	{"XR_media2 Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
	{"XR_media3 Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
	{"XR_media4 Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
	{"XR_media6 Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
	{"XR_mmap Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
	{"XR_Karaoke Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
	{"XR_hf Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
/* TDM5_TX to audio */
	{"XR_media0 Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
	{"XR_media1 Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
	{"XR_media2 Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
	{"XR_media3 Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
	{"XR_media4 Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
	{"XR_media6 Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
	{"XR_mmap Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
	{"XR_Karaoke Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
	{"XR_hf Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
/* TDM6_TX to audio */
	{"XR_media0 Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
	{"XR_media1 Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
	{"XR_media2 Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
	{"XR_media3 Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
	{"XR_media4 Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
	{"XR_media6 Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
	{"XR_mmap Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
	{"XR_Karaoke Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
	{"XR_hf Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
#endif
	{"XR_Voice_UL", NULL, "XR_Voice Mixer"},
	{"XR_UL0", NULL, "XR_media0 Mixer"},
	{"XR_UL1", NULL, "XR_media1 Mixer"},
	{"XR_UL2", NULL, "XR_media2 Mixer"},
	{"XR_UL3", NULL, "XR_media3 Mixer"},
	{"XR_UL4", NULL, "XR_media4 Mixer"},
	{"XR_UL6", NULL, "XR_media6 Mixer"},
	{"XR_MMAP_UL", NULL, "XR_mmap Mixer"},
	{"XR_Karaoke_UL", NULL, "XR_Karaoke Mixer"},
	{"XR_INCALL_UL", NULL, "XR_incall Mixer"},
	{"XR_HF_UL", NULL, "XR_hf Mixer"},

#ifdef AUDIO_ROUTES_RESERVE
/* VOICE_DL to I2S0_RX */
	{"XR_I2S0_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_I2S0_RX", NULL, "XR_I2S0_RX Mixer"},
/* VOICE_DL to I2S2_RX */
	{"XR_I2S2_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_I2S2_RX", NULL, "XR_I2S2_RX Mixer"},
/* VOICE_DL to I2S4_RX */
	{"XR_I2S4_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_I2S4_RX", NULL, "XR_I2S4_RX Mixer"},
/* VOICE_DL to TDM0_RX */
	{"XR_TDM0_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM0_RX", NULL, "XR_TDM0_RX Mixer"},
/* VOICE_DL to TDM1_RX */
	{"XR_TDM1_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM1_RX", NULL, "XR_TDM1_RX Mixer"},
/* VOICE_DL to TDM2_RX */
	{"XR_TDM2_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM2_RX", NULL, "XR_TDM2_RX Mixer"},
/* VOICE_DL to TDM3_RX */
	{"XR_TDM3_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM3_RX", NULL, "XR_TDM3_RX Mixer"},
/* VOICE_DL to TDM4_RX */
	{"XR_TDM4_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM4_RX", NULL, "XR_TDM4_RX Mixer"},
/* VOICE_DL to TDM5_RX */
	{"XR_TDM5_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM5_RX", NULL, "XR_TDM5_RX Mixer"},
/* VOICE_DL to TDM6_RX */
	{"XR_TDM6_RX Mixer", "XR_Voice", "XR_Voice_DL"},
	{"XR_TDM6_RX", NULL, "XR_TDM6_RX Mixer"},
#endif

#ifdef AUDIO_ROUTES_RESERVE
/* I2S0_TX to VOICE_UL */
	{"XR_Voice Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
/* I2S2_TX to VOICE_UL */
	{"XR_Voice Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
/* I2S4_TX to VOICE_UL */
	{"XR_Voice Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
/* TDM_TX to VOICE_UL */
	{"XR_Voice Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
	{"XR_Voice Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
	{"XR_Voice Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
	{"XR_Voice Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
	{"XR_Voice Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
	{"XR_Voice Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
	{"XR_Voice Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
#endif

/* LOOPBACK_DL to I2S1_RX */
	{"XR_I2S1_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_I2S1_RX", NULL, "XR_I2S1_RX Mixer"},
/* LOOPBACK_DL to I2S3_RX */
	{"XR_I2S3_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_I2S3_RX", NULL, "XR_I2S3_RX Mixer"},
/* LOOPBACK_DL to TDM7_RX */
	{"XR_TDM7_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM7_RX", NULL, "XR_TDM7_RX Mixer"},
/* LOOPBACK_DL to TDM8_RX */
	{"XR_TDM8_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM8_RX", NULL, "XR_TDM8_RX Mixer"},
/* LOOPBACK_DL to TDM9_RX */
	{"XR_TDM9_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM9_RX", NULL, "XR_TDM9_RX Mixer"},
/* LOOPBACK_DL to TDM7_8_RX */
	{"XR_TDM7_8_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM7_8_RX", NULL, "XR_TDM7_8_RX Mixer"},

#ifdef AUDIO_ROUTES_RESERVE
/* LOOPBACK_DL to I2S0_RX */
	{"XR_I2S0_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_I2S0_RX", NULL, "XR_I2S0_RX Mixer"},
/* LOOPBACK_DL to I2S2_RX */
	{"XR_I2S2_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_I2S2_RX", NULL, "XR_I2S2_RX Mixer"},
/* LOOPBACK_DL to I2S4_RX */
	{"XR_I2S4_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_I2S4_RX", NULL, "XR_I2S4_RX Mixer"},
/* LOOPBACK_DL to TDM0_RX */
	{"XR_TDM0_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM0_RX", NULL, "XR_TDM0_RX Mixer"},
/* LOOPBACK_DL to TDM1_RX */
	{"XR_TDM1_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM1_RX", NULL, "XR_TDM1_RX Mixer"},
/* LOOPBACK_DL to TDM2_RX */
	{"XR_TDM2_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM2_RX", NULL, "XR_TDM2_RX Mixer"},
/* LOOPBACK_DL to TDM3_RX */
	{"XR_TDM3_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM3_RX", NULL, "XR_TDM3_RX Mixer"},
/* LOOPBACK_DL to TDM4_RX */
	{"XR_TDM4_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM4_RX", NULL, "XR_TDM4_RX Mixer"},
/* LOOPBACK_DL to TDM5_RX */
	{"XR_TDM5_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM5_RX", NULL, "XR_TDM5_RX Mixer"},
/* LOOPBACK_DL to TDM6_RX */
	{"XR_TDM6_RX Mixer", "XR_Loopback", "XR_Loopback_DL"},
	{"XR_TDM6_RX", NULL, "XR_TDM6_RX Mixer"},
#endif

/* I2S1_TX to LOOPBACK_UL*/
	{"XR_Loopback Mixer", "XR_I2S1_TX", "XR_I2S1_TX"},
/* I2S3_TX to LOOPBACK_UL*/
	{"XR_Loopback Mixer", "XR_I2S3_TX", "XR_I2S3_TX"},
	{"XR_Loopback Mixer", "XR_TDM7_TX", "XR_TDM7_TX"},
	{"XR_Loopback Mixer", "XR_TDM8_TX", "XR_TDM8_TX"},
	{"XR_Loopback Mixer", "XR_TDM9_TX", "XR_TDM9_TX"},
	{"XR_Loopback Mixer", "XR_TDM7_8_TX", "XR_TDM7_8_TX"},

#ifdef AUDIO_ROUTES_RESERVE
/* I2S0_TX to LOOPBACK_UL*/
	{"XR_Loopback Mixer", "XR_I2S0_TX", "XR_I2S0_TX"},
/* I2S2_TX to LOOPBACK_UL*/
	{"XR_Loopback Mixer", "XR_I2S2_TX", "XR_I2S2_TX"},
/* I2S4_TX to LOOPBACK_UL*/
	{"XR_Loopback Mixer", "XR_I2S4_TX", "XR_I2S4_TX"},
/* TDM_TX to LOOPBACK_UL */
	{"XR_Loopback Mixer", "XR_TDM0_TX", "XR_TDM0_TX"},
	{"XR_Loopback Mixer", "XR_TDM1_TX", "XR_TDM1_TX"},
	{"XR_Loopback Mixer", "XR_TDM2_TX", "XR_TDM2_TX"},
	{"XR_Loopback Mixer", "XR_TDM3_TX", "XR_TDM3_TX"},
	{"XR_Loopback Mixer", "XR_TDM4_TX", "XR_TDM4_TX"},
	{"XR_Loopback Mixer", "XR_TDM5_TX", "XR_TDM5_TX"},
	{"XR_Loopback Mixer", "XR_TDM6_TX", "XR_TDM6_TX"},
#endif
/* I2S9_TX TO audio */
	{"XR_voicetrigger Mixer", "XR_I2S9_TX", "XR_I2S9_TX"},
	{"XR_asr Mixer", "XR_I2S9_TX", "XR_I2S9_TX"},

/*VAD_I2S_TX to VT_UL*/
	{"XR_voicetrigger Mixer", "XR_VAD_I2S_TX", "XR_VAD_I2S_TX"},

#ifdef FK_AUDIO_USB_OFFLOAD
	/* AUDIO_USB_TX to LOOPBACK_UL*/
	{"XR_Loopback Mixer", "XR_AUDIO_USB_TX", "XR_AUDIO_USB_TX"},
#endif

	{"XR_Loopback_UL", NULL, "XR_Loopback Mixer"},
	{"XR_voicetrigger_UL", NULL, "XR_voicetrigger Mixer"},
	{"XR_asr_UL", NULL, "XR_asr Mixer"},
};

static int fk_kctrl_val_get_by_name(const char *name)
{
	int table_size = 0;
	int i = 0;

	table_size = sizeof(kctrl_item_talbe) / sizeof(struct kctrl_item);
	for (i = 0; i < table_size; i++) {
		if (!strcmp(name, kctrl_item_talbe[i].name))
			return kctrl_item_talbe[i].val;
	}

	AUD_LOG_DBG(AUD_SOC, "%s unfound.", name);

	return -1;
}

static int fk_kctrl_val_set_by_name(const char *name, int val)
{
	int table_size = 0;
	int i = 0;

	table_size = sizeof(kctrl_item_talbe) / sizeof(struct kctrl_item);
	for (i = 0; i < table_size; i++) {
		if (!strcmp(name, kctrl_item_talbe[i].name)) {
			kctrl_item_talbe[i].val = val;
			return 0;
		}
	}

	AUD_LOG_DBG(AUD_SOC, "%s unfound.", name);

	return -1;
}

static struct map_item *fk_channel_map_get_by_name(const char *name)
{
	int table_size = 0;
	int i;

	table_size = sizeof(map_item_talbe) / sizeof(struct map_item);
	for (i = 0; i < table_size; i++) {
		if (!strcmp(name, map_item_talbe[i].name))
			return (struct map_item *)&map_item_talbe[i];
	}

	AUD_LOG_DBG(AUD_SOC, "%s unfound.", name);
	return NULL;
}

static struct map_item *fk_channel_map_get_by_session
	(int scene_id, int stream_id)
{
	int table_size = 0;
	int i;

	table_size = sizeof(map_item_talbe) / sizeof(struct map_item);
	for (i = 0; i < table_size; i++) {
		if ((map_item_talbe[i].scene_id == scene_id) &&
			(map_item_talbe[i].stream_id == stream_id)) {
			return (struct map_item *)&map_item_talbe[i];
		}
	}

	AUD_LOG_DBG(AUD_SOC, "unfound.");
	return NULL;
}

static int fk_routing_update_by_name(const char *name, int val)
{
	int table_size = 0;
	int i = 0;
	struct kctrl_item *item = NULL;
	uint16_t session_id = 0;
	struct dai_format fmt = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2};
	struct dai_format *aif_cfg = NULL;
	struct map_item *item_map = NULL;
	int ret = 0;

	table_size = sizeof(kctrl_item_talbe) / sizeof(struct kctrl_item);
	for (i = 0; i < table_size; i++) {
		if (!strcmp(name, kctrl_item_talbe[i].name)) {
			item = (struct kctrl_item *)&kctrl_item_talbe[i];
			break;
		}
	}

	if (!item)
		return ret;

	/* update item value */
	item->val = val;

	aif_cfg = fk_aif_dev_cfg_get(item->port_id);
	if (aif_cfg != NULL) {
		fmt.samples = aif_cfg->samples;
		fmt.bit_width = aif_cfg->bit_width;
		fmt.channels = aif_cfg->channels;
		fmt.pcm_fmt = aif_cfg->pcm_fmt;
	}

	if (!fmt.samples || !fmt.bit_width || !fmt.channels) {
		AUD_LOG_ERR(AUD_SOC, "invalid values :samples(%u) bits(%u) channels(%u)",
		fmt.samples, fmt.bit_width, fmt.channels);
		return -EINVAL;
	}

	session_id = (item->scene_id << 8) | item->stream_id;
	AUD_LOG_INFO(AUD_SOC, "session id(0x%x) name(%s) en(%d)",
		session_id, name, val);

	if (val) {
		ret = fk_audio_port_pinctrl_func_set(item->port_id, true);
		ret = fk_acore_set_paths(session_id, item->port_id, fmt, true);
		/* todo acore set channel map info*/
		item_map = fk_channel_map_get_by_session(item->scene_id, item->stream_id);
		if (item_map)
			fk_acore_set_channel_map(session_id, item->port_id, item_map->map_info);
	} else {
		ret = fk_audio_port_pinctrl_func_set(item->port_id, false);
		ret = fk_acore_set_paths(session_id, item->port_id, fmt, false);
	}

	AUD_LOG_DBG(AUD_SOC, "session id(0x%x) samples(%u) bits(%u) channels(%u)",
		session_id, fmt.samples, fmt.bit_width, fmt.channels);

	return ret;
}

static int fk_ec_reference_config_port(int port_id)
{
	int ec_port = 0;

	switch (port_id) {
	case FK_XR_I2S0_TX:
	case FK_XR_TDM0_TX:
		ec_port = PORT_ID_AUDIFIIS0;
		break;
	case FK_XR_I2S1_TX:
	case FK_XR_TDM1_TX:
		ec_port = PORT_ID_AUDIFIIS1;
		break;
	case FK_XR_I2S2_TX:
	case FK_XR_TDM2_TX:
		ec_port = PORT_ID_AUDIFIIS2;
		break;
	case FK_XR_I2S3_TX:
	case FK_XR_TDM3_TX:
		ec_port = PORT_ID_AUDIFIIS3;
		break;
	case FK_XR_I2S4_TX:
	case FK_XR_TDM4_TX:
		ec_port = PORT_ID_AUDIFIIS4;
		break;
	case FK_XR_I2S5_TX:
	case FK_XR_TDM5_TX:
		ec_port = PORT_ID_IIS5;
		break;
	case FK_XR_I2S6_TX:
	case FK_XR_TDM6_TX:
		ec_port = PORT_ID_IIS6;
		break;
	case FK_XR_I2S7_TX:
	case FK_XR_TDM7_TX:
		ec_port = PORT_ID_IIS7;
		break;
	case FK_XR_I2S8_TX:
	case FK_XR_TDM8_TX:
		ec_port = PORT_ID_IIS8;
		break;
	case FK_XR_I2S9_TX:
	case FK_XR_TDM9_TX:
		ec_port = PORT_ID_IIS9;
		break;
	default:
		AUD_LOG_INFO(AUD_SOC, "no port for ec");
		break;
	}
	AUD_LOG_INFO(AUD_SOC, "ec port %d", ec_port);
	return ec_port;
}

static int fk_routing_ec_reference(const char *name, int val)
{
	int table_size = 0;
	int i = 0;
	struct kctrl_item *item = NULL;
	uint16_t session_id = 0;
	struct ec_ctrl kws_ec_info;
	int ret = 0;
	struct stream_params s_param;

	memset(&s_param, 0x0, sizeof(struct stream_params));
	memset(&kws_ec_info, 0x0, sizeof(struct ec_ctrl));

	table_size = sizeof(kctrl_item_talbe) / sizeof(struct kctrl_item);
	for (i = 0; i < table_size; i++) {
		if (!strcmp(name, kctrl_item_talbe[i].name)) {
			item = (struct kctrl_item *)&kctrl_item_talbe[i];
			break;
		}
	}

	if (!item)
		return ret;

	/* update item value */
	item->val = val;

	kws_ec_info.ec_type = fk_ec_type_from_val(val);
	kws_ec_info.port_id = fk_ec_reference_config_port(item->port_id);
	kws_ec_info.channel = fk_ec_ref_ch_query();
	kws_ec_info.bit_width = fk_ec_ref_bit_format_query();
	kws_ec_info.sample_rate = fk_ec_ref_sample_rate_query();

	session_id = (item->scene_id << 8) | item->stream_id;
	AUD_LOG_INFO(AUD_SOC, "session id(0x%x) name(%s) en(%d)",
		session_id, name, val);
	AUD_LOG_INFO(AUD_SOC, "ec_type(%d) port_id(%d) channel(%d) bit_width(%d) sample_rate(%d)",
		kws_ec_info.ec_type, kws_ec_info.port_id, kws_ec_info.channel,
		kws_ec_info.bit_width, kws_ec_info.sample_rate);

	s_param.type = PARAMS_KWS_EC_REF;
	s_param.params.kws_ec_ctrl = kws_ec_info;
	ret = fk_acore_set_params(session_id, &s_param);


	return ret;
}

static void snd_soc_dapm_add_routes_i2s(struct snd_soc_component *component)
{
	snd_soc_dapm_add_routes(&component->dapm, fk_i2s, ARRAY_SIZE(fk_i2s));
}

void fk_audio_dapm_ignore_suspend(struct snd_soc_component *component)
{
	struct snd_soc_dapm_context *dapm = &component->dapm;

	snd_soc_dapm_ignore_suspend(dapm, "XR_voicetrigger Mixer");
	snd_soc_dapm_ignore_suspend(dapm, "XR_VAD_I2S Capture");
	snd_soc_dapm_ignore_suspend(dapm, "XR_voicetrigger");
	snd_soc_dapm_ignore_suspend(dapm, "XR_voicetrigger_UL");
	snd_soc_dapm_ignore_suspend(dapm, "XR_VAD_I2S_TX");
}


static int fk_routing_probe(struct snd_soc_component *component)
{
	uint32_t items_num;

	/* add fe widgets */
	snd_soc_dapm_new_controls(&component->dapm, fk_widgets,
				ARRAY_SIZE(fk_widgets));
	/* add be widgets */
	snd_soc_dapm_new_controls(&component->dapm, fk_widgets_i2s,
		ARRAY_SIZE(fk_widgets_i2s));
	snd_soc_dapm_add_routes_i2s(component);
	snd_soc_dapm_new_widgets(component->dapm.card);

	/* add audio interface format ctrl */
	snd_soc_add_component_controls(component, fk_aif_fmt_controls,
		ARRAY_SIZE(fk_aif_fmt_controls));

	/* add audio channel map ctrl */
	snd_soc_add_component_controls(component, fk_channel_map_controls,
		ARRAY_SIZE(fk_channel_map_controls));

	/* add common ctrl */
	fk_common_controls_add(component);

	items_num = sizeof(kctrl_item_talbe) / sizeof(struct kctrl_item);
	fk_be_item_table_update((void *)&kctrl_item_talbe, items_num);

	fk_audio_dapm_ignore_suspend(component);

	return 0;
}
int fk_routing_pcm_prepare(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream)
{

	AUD_LOG_DBG(AUD_SOC, "enter");

	return 0;
}

int fk_routing_pcm_hw_params(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params)
{
	struct snd_interval *rate = hw_param_interval(params,
						SNDRV_PCM_HW_PARAM_RATE);
	struct snd_interval *channels = hw_param_interval(params,
						SNDRV_PCM_HW_PARAM_CHANNELS);
	struct snd_interval *samp_bits = hw_param_interval(params,
						SNDRV_PCM_HW_PARAM_SAMPLE_BITS);
	struct snd_interval *frame_bits = hw_param_interval(params,
						SNDRV_PCM_HW_PARAM_FRAME_BITS);

	AUD_LOG_DBG(AUD_SOC, "rate min=%d max=%d", rate->min, rate->max);
	AUD_LOG_DBG(AUD_SOC, "channels min=%d max=%d", channels->min, channels->max);
	AUD_LOG_DBG(AUD_SOC, "samp_bits min=%d max=%d", samp_bits->min, samp_bits->max);
	AUD_LOG_DBG(AUD_SOC, "frame_bits min=%d max=%d", frame_bits->min, frame_bits->max);

	return 0;
}

unsigned int fk_routing_pcm_read(struct snd_soc_component *component,
			     unsigned int reg)
{
	// AUD_LOG_DBG(AUD_SOC, "name(%s) reg=0x%x", component->name, reg);

	return 0;
}

int fk_routing_pcm_write(struct snd_soc_component *component,
			     unsigned int reg, unsigned int val)
{
	// AUD_LOG_DBG(AUD_SOC, "name(%s) reg=0x%x val=0x%x",
	// component->name, reg, val);

	return 0;
}

int fk_routing_pcm_construct(struct snd_soc_component *component,
			     struct snd_soc_pcm_runtime *rtd)
{
	// AUD_LOG_DBG(AUD_SOC, "rtd->num=%d", rtd->num);

	return 0;
}

void fk_routing_pcm_destruct(struct snd_soc_component *component,
			     struct snd_pcm *pcm)
{
	AUD_LOG_DBG(AUD_SOC, "pcm->id=%s", (char *)&pcm->id[0]);
}

const struct snd_soc_component_driver fk_soc_routing_component = {
	.name			= DRV_NAME,
	.prepare		= fk_routing_pcm_prepare,
	.hw_params		= fk_routing_pcm_hw_params,
	.probe			= fk_routing_probe,
	.pcm_construct	= fk_routing_pcm_construct,
	.pcm_destruct	= fk_routing_pcm_destruct,
	.read			= fk_routing_pcm_read,
	.write			= fk_routing_pcm_write,
};

static int fk_routing_pcm_probe(struct platform_device *pdev)
{
	AUD_LOG_INFO(AUD_SOC, "dev name %s", dev_name(&pdev->dev));
	return snd_soc_register_component(&pdev->dev,
				&fk_soc_routing_component,
				NULL, 0);
}

static int fk_routing_pcm_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id fk_pcm_routing_dt_match[] = {
	{.compatible = "xring,fk-pcm-routing"},
	{}
};
MODULE_DEVICE_TABLE(of, fk_pcm_routing_dt_match);

static struct platform_driver fk_routing_pcm_driver = {
	.driver = {
		.name = "fk-pcm-routing",
		.owner = THIS_MODULE,
		.of_match_table = fk_pcm_routing_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_routing_pcm_probe,
	.remove = fk_routing_pcm_remove,
};

int __init fk_soc_routing_platform_init(void)
{
	return platform_driver_register(&fk_routing_pcm_driver);
}

void fk_soc_routing_platform_exit(void)
{
	platform_driver_unregister(&fk_routing_pcm_driver);
}

MODULE_DESCRIPTION("FK routing platform driver");
MODULE_LICENSE("Dual BSD/GPL");
