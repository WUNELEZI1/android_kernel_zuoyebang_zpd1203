/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __FK_AUDIO_USB_H
#define __FK_AUDIO_USB_H

#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/core.h>
#include <sound/asound.h>
#include <sound/soc-jack.h>
#include <sound/jack.h>

#include "../acore/fk-acore-api.h"

/****************** define callback function begin *************/
typedef bool (*usb2adu_cb)(void *priv);

/****************** define callback function begin *************/

struct aud_usb_offload_info {
	uint8_t intf_num;
	uint8_t alt_setting;
	uint8_t idle_intf_num;
	uint8_t idle_alt_setting;
	uint16_t nurbs; /* urb numbers */
	int32_t packet_num; /* number of packets per urb */
	uint32_t pipe; /* the data io pipe */
	uint16_t max_packet_size; /* max packet size in bytes */
	uint8_t datainterval;      /* log_2 of data packet interval */
	int32_t clock_id;
	uint8_t protocol;
	uint8_t ctrl_intf;
	uint32_t sync_pipe; /* sync endpoint io pipe */
	uint8_t sync_interval; /* sync endpoint interval */
	uint8_t attributes; /* ep format attributes */
	uint8_t dev_type; /* device type, capture&playbcak */
};

void usb2aud_cb_register(usb2adu_cb cb, void *priv);
void usb2aud_cb_unregister(void);

int audio_usb_pcm_format_get(uint32_t bit_width);
int audio_usb_offload_param_prepare(int dir,
	struct dai_format dai_fmt);
int audio_usb_offload_param_get(struct aud_usb_offload_info *info,
	int dir, struct dai_format dai_fmt);
int audio_usb_connect_set(bool val);
int audio_usb_info_query(void);

int fk_dai_audio_usb_dai_probe(struct snd_soc_dai *dai);

void audio_usb_status_notify_complete(int cmd);

#endif //__FK_AUDIO_USB_H

