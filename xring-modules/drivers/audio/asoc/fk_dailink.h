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

#include <sound/soc.h>

/* FE dai-links */
SND_SOC_DAILINK_DEFS(xr_media0,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_media0")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-dsp.2")));
SND_SOC_DAILINK_DEFS(xr_media1,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_media1")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-dsp.1")));
SND_SOC_DAILINK_DEFS(xr_media2,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_media2")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-dsp.0")));
SND_SOC_DAILINK_DEFS(xr_media3,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_media3")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-dsp.0")));
SND_SOC_DAILINK_DEFS(xr_media4,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_media4")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-compress-dsp")));
SND_SOC_DAILINK_DEFS(xr_media5,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_media5")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-compress-dsp")));
SND_SOC_DAILINK_DEFS(xr_media6,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_media6")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-voip-dsp")));
SND_SOC_DAILINK_DEFS(xr_media7,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_media7")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-haptic")));
SND_SOC_DAILINK_DEFS(xr_voice,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_voice")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-voice-dsp")));
SND_SOC_DAILINK_DEFS(xr_loopback,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_Loopback")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-loopback-dsp")));
SND_SOC_DAILINK_DEFS(xr_lsm,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_lsm")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-lsm-dsp")));
SND_SOC_DAILINK_DEFS(xr_multichs,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_direct")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-dsp.3")));
SND_SOC_DAILINK_DEFS(xr_spatial,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_spatial")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-spatial")));
SND_SOC_DAILINK_DEFS(xr_mmap,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_mmap")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-mmap-dsp")));
SND_SOC_DAILINK_DEFS(xr_karaoke,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_Karaoke")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-karaoke")));
SND_SOC_DAILINK_DEFS(xr_incall,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_incall")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-incall-dsp")));
SND_SOC_DAILINK_DEFS(xr_hf,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_hf")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-hf-dsp")));
SND_SOC_DAILINK_DEFS(xr_asr,
	DAILINK_COMP_ARRAY(COMP_CPU("XR_asr")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-dsp.4")));

/* BE dai-links */
SND_SOC_DAILINK_DEFS(xr_i2s0_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audif-i2s.0")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s0_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audif-i2s.1")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s1_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audif-i2s.2")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s1_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audif-i2s.3")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s2_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audif-i2s.4")),
	DAILINK_COMP_ARRAY(COMP_CODEC("cs35l41b", "xring-spk"),
		COMP_CODEC("bot-pa", "xring-spk"), COMP_CODEC("top-pa", "xring-spk"),
		COMP_CODEC("haptic1", "xring-spk"), COMP_CODEC("haptic2", "xring-spk")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s2_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audif-i2s.5")),
	DAILINK_COMP_ARRAY(COMP_CODEC("cs35l41b", "xring-spk"),
		COMP_CODEC("bot-pa", "xring-spk"), COMP_CODEC("top-pa", "xring-spk")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s3_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audif-i2s.6")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s3_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audif-i2s.7")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s4_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audif-i2s.8")),
	DAILINK_COMP_ARRAY(COMP_CODEC("xring-codec", "xring-aif1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s4_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audif-i2s.9")),
	DAILINK_COMP_ARRAY(COMP_CODEC("xring-codec", "xring-aif1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
#ifdef AUDIO_ROUTES_RESERVE
SND_SOC_DAILINK_DEFS(xr_i2s5_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-adsp-i2s.10")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s5_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-adsp-i2s.11")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s6_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-adsp-i2s.12")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s6_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-adsp-i2s.13")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s7_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-adsp-i2s.14")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s7_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-adsp-i2s.15")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s8_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-adsp-i2s.16")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s8_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-adsp-i2s.17")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_i2s9_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-adsp-i2s.18")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
#endif
SND_SOC_DAILINK_DEFS(xr_i2s9_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-adsp-i2s.19")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_audio_usb_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-usb-rx")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_audio_usb_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-usb-tx")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
/*vad_i2s_tx*/
SND_SOC_DAILINK_DEFS(xr_vad_i2s_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-vad-i2s")),
	DAILINK_COMP_ARRAY(COMP_CODEC("xring-codec", "xring-aif1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
/*MI2S_RX*/
SND_SOC_DAILINK_DEFS(xr_mi2s0_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-mi2s0")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));

/* BE dai-links tdm */
#ifdef AUDIO_ROUTES_RESERVE
SND_SOC_DAILINK_DEFS(xr_tdm0_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.0")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm0_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.1")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm1_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.2")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm1_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.3")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm2_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.4")),
	DAILINK_COMP_ARRAY(COMP_CODEC("cs35l41b", "xring-spk"),
			COMP_CODEC("bot-pa", "xring-spk"), COMP_CODEC("top-pa", "xring-spk")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm2_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.5")),
	DAILINK_COMP_ARRAY(COMP_CODEC("cs35l41b", "xring-spk"),
			COMP_CODEC("bot-pa", "xring-spk"), COMP_CODEC("top-pa", "xring-spk")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm3_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.6")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm3_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.7")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm4_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.8")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm4_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.9")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm5_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.10")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm5_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.11")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm6_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.12")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm6_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.13")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
#endif
SND_SOC_DAILINK_DEFS(xr_tdm7_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.14")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bot-pa", "xring-spk"), COMP_CODEC("top-pa", "xring-spk"),
			COMP_CODEC("speaker3", "xring-spk"), COMP_CODEC("speaker4", "xring-spk"),
			COMP_CODEC("cs35l45_1", "xring-spk"), COMP_CODEC("cs35l45_2", "xring-spk"),
			COMP_CODEC("speaker6", "xring-spk"), COMP_CODEC("speaker8", "xring-spk"),
			COMP_CODEC("cs35l45_3", "xring-spk"), COMP_CODEC("cs35l45_4", "xring-spk"),
			COMP_CODEC("speaker5", "xring-spk"), COMP_CODEC("speaker7", "xring-spk"),
			COMP_CODEC("haptic1", "xring-spk"), COMP_CODEC("haptic2", "xring-spk")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm7_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.15")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm8_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.16")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm8_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.17")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm9_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.18")),
	DAILINK_COMP_ARRAY(COMP_CODEC("xring-codec", "xring-aif1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm9_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.19")),
	DAILINK_COMP_ARRAY(COMP_CODEC("xring-codec", "xring-aif1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm7_8_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.20")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bot-pa", "xring-spk"), COMP_CODEC("top-pa", "xring-spk"),
			COMP_CODEC("speaker3", "xring-spk"), COMP_CODEC("speaker4", "xring-spk"),
			COMP_CODEC("speaker5", "xring-spk"), COMP_CODEC("speaker6", "xring-spk"),
			COMP_CODEC("speaker7", "xring-spk"), COMP_CODEC("speaker8", "xring-spk"),
			COMP_CODEC("haptic1", "xring-spk"), COMP_CODEC("haptic2", "xring-spk")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));
SND_SOC_DAILINK_DEFS(xr_tdm7_8_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("fk-dai-audio-tdm.21")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bot-pa", "xring-spk"), COMP_CODEC("top-pa", "xring-spk"),
			COMP_CODEC("speaker3", "xring-spk"), COMP_CODEC("speaker4", "xring-spk"),
			COMP_CODEC("speaker5", "xring-spk"), COMP_CODEC("speaker6", "xring-spk"),
			COMP_CODEC("speaker7", "xring-spk"), COMP_CODEC("speaker8", "xring-spk"),
			COMP_CODEC("haptic1", "xring-spk"), COMP_CODEC("haptic2", "xring-spk")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("fk-pcm-routing")));

