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
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/usb/audio.h>
#include <linux/usb/audio-v2.h>
#include <linux/uaccess.h>
#include <linux/input.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/core.h>
#include <sound/asound.h>
#include <linux/usb.h>
#include <linux/iommu.h>
#include <linux/dma-mapping.h>
#include <linux/dma-map-ops.h>
#include <linux/platform_device.h>
#include <linux/usb.h>
#include <linux/usb/audio.h>
#include <linux/usb/audio-v2.h>
#include <linux/usb/audio-v3.h>


#include "../../../common/sound/usb/mixer.h"
#include "../../../common/sound/usb/usbaudio.h"
#include "../../../common/sound/usb/card.h"
#include "../../../common/sound/usb/endpoint.h"
#include "../../../common/sound/usb/helper.h"
#include "../../../common/sound/usb/pcm.h"
#include "../../../common/sound/usb/quirks.h"
#include "../../../common/sound/usb/power.h"
#include "../../../common/sound/usb/media.h"
#include "../../../common/sound/usb/implicit.h"

#include "fk-audio-log.h"
#include "fk-audio-usb.h"
#include "../acore/fk-acore-utils.h"
#include "../acore/fk-acore.h"

#define SUBSTREAM_FLAG_DATA_EP_STARTED	0
#define SUBSTREAM_FLAG_SYNC_EP_STARTED	1
#define STREAM_DIR_MAX 2
#define MAX_ID_ELEMS	256

#define ep_type_name(type) \
	(type == SND_USB_ENDPOINT_TYPE_DATA ? "data" : "sync")

union uac23_clock_source_desc {
	struct uac_clock_source_descriptor v2;
	struct uac3_clock_source_descriptor v3;
};

union uac23_clock_selector_desc {
	struct uac_clock_selector_descriptor v2;
	struct uac3_clock_selector_descriptor v3;
};

union uac23_clock_multiplier_desc {
	struct uac_clock_multiplier_descriptor v2;
	struct uac_clock_multiplier_descriptor v3;
};


#define GET_VAL(p, proto, field) \
	((proto) == UAC_VERSION_3 ? (p)->v3.field : (p)->v2.field)

/*
 *
 * config function: snd_usb_ctl_msg(
 *		usb device : struct usb_device *dev
 *		pipe : usb_sndctrlpipe(chip->dev, 0)
 *				#define usb_sndctrlpipe(dev, endpoint)	\
 *					((PIPE_CONTROL << 30) | __create_pipe(dev, endpoint))
 *		request :	#define UAC_SET_			0x00
 *					#define UAC__CUR			0x1
 *					#define UAC_SET_CUR			(UAC_SET_ | UAC__CUR)
 *		requesttype :	USB_RECIP_INTERFACE | USB_TYPE_CLASS | USB_DIR_OUT
 *						#define USB_RECIP_INTERFACE		0x01
 *						#define USB_TYPE_CLASS			(0x01 << 5)
 *						#define USB_DIR_OUT			0		// to device
 *		value : (cval->control << 8) | channel + cval->idx_off
 *		index : get_iface_desc(mixer->hostif)->bInterfaceNumber | (cval->head.id << 8)
 *		buf :
 *			static int get_abs_value(struct usb_mixer_elem_info *cval, int val)
 *			{
 *				if (val < 0)
 *					return cval->min;
 *				if (!cval->res)
 *					cval->res = 1;
 *				val *= cval->res;
 *				val += cval->min;
 *				if (val > cval->max)
 *					return cval->max;
 *				return val;
 *			}
 *			static int convert_bytes_value(struct usb_mixer_elem_info *cval, int val)
 *			{
 *				switch (cval->val_type) {
 *				case USB_MIXER_BOOLEAN:
 *					return !!val;
 *				case USB_MIXER_INV_BOOLEAN:
 *					return !val;
 *				case USB_MIXER_S8:
 *				case USB_MIXER_U8:
 *					return val & 0xff;
 *				case USB_MIXER_S16:
 *				case USB_MIXER_U16:
 *					return val & 0xffff;
 *			}
 *			return 0; // not reached
 *		}
 *			buf[0] = value_set & 0xff;
 *			buf[1] = (value_set >> 8) & 0xff;
 *			buf[2] = (value_set >> 16) & 0xff;
 *			buf[3] = (value_set >> 24) & 0xff;
 *		size : calc by val_type
 *				switch (cval->val_type) {
 *					case USB_MIXER_S32:
 *					case USB_MIXER_U32:
 *						return 4;
 *					case USB_MIXER_S16:
 *					case USB_MIXER_U16:
 *						return 2;
 *					default:
 *						return 1;
 *				}
 *				enum {
 *					USB_MIXER_BOOLEAN,
 *					USB_MIXER_INV_BOOLEAN,
 *					USB_MIXER_S8,
 *					USB_MIXER_U8,
 *					USB_MIXER_S16,
 *					USB_MIXER_U16,
 *					USB_MIXER_S32,
 *					USB_MIXER_U32,
 *					USB_MIXER_BESPOKEN,
 *				};
 */
struct usb_items_info {
	uint8_t mixer_type; //mixer_ctrl_type
	uint8_t control;
	uint8_t channels;
	uint8_t idx_off;
	uint8_t bInterfaceNumber;
	uint8_t head_id;
	uint8_t val_type;
	uint8_t res; //int
	uint32_t min; //int
	uint32_t max; //int
};

struct aud_usb_dev {
	struct platform_device *pdev;
	struct usb_device *udev;
	struct snd_usb_audio *chip;
	unsigned int card_num;

	struct snd_soc_dai *dai;

	/* status */
	atomic_t connected;
	int jack_init;
	int jack_type;
	struct snd_soc_jack hs_jack;
	struct input_dev *input;

	/* interface specific */
	struct usb_interface *intf;
	uint8_t intf0_num;
	uint8_t alt0_setting;
	struct aud_usb_offload_info info[STREAM_DIR_MAX];

	uint8_t items_num;
	struct usb_ctrl_param item_param[MAX_ELEMS_NUM+1];
};

/****************** static parameter begin **********************/

static wait_queue_head_t ausb_wait_que;
static atomic_t ausb_wait;

static struct aud_usb_dev audev[SNDRV_CARDS];
/* mode: 0-offload 1-no offload */
static atomic_t usb_mode;
static usb2adu_cb usb_cb;
static void *usb_priv;

/****************** static parameter end ***********************/


/****************** function define begin **********************/
static void audio_usb_ep_info(int card_id);
static void audio_usb_disconnect(struct snd_usb_audio *chip);

/****************** function define end ************************/

static void *find_uac_clock_desc(struct usb_host_interface *iface, int id,
				 bool (*validator)(void *, int, int),
				 u8 type, int proto)
{
	void *cs = NULL;

	while ((cs = snd_usb_find_csint_desc(iface->extra, iface->extralen,
					     cs, type))) {
		if (validator(cs, id, proto))
			return cs;
	}

	return NULL;
}

static bool validate_clock_source(void *p, int id, int proto)
{
	union uac23_clock_source_desc *cs = p;

	return GET_VAL(cs, proto, bClockID) == id;
}

static bool validate_clock_selector(void *p, int id, int proto)
{
	union uac23_clock_selector_desc *cs = p;

	return GET_VAL(cs, proto, bClockID) == id;
}

static bool validate_clock_multiplier(void *p, int id, int proto)
{
	union uac23_clock_multiplier_desc *cs = p;

	return GET_VAL(cs, proto, bClockID) == id;
}

static union uac23_clock_source_desc
	*aud_usb_find_clock_source(struct snd_usb_audio *chip, int id, int proto)
{
	u8 type;
	union uac23_clock_source_desc *source_desc = NULL;

	type = (proto == UAC_VERSION_3 ? (UAC3_CLOCK_SOURCE) : (UAC2_CLOCK_SOURCE));
	source_desc = (union uac23_clock_source_desc *)find_uac_clock_desc(chip->ctrl_intf,
		id, validate_clock_source, type, proto);
	return source_desc;
}

static union uac23_clock_selector_desc
	*aud_usb_find_clock_selector(struct snd_usb_audio *chip, int id, int proto)
{
	u8 type;
	union uac23_clock_selector_desc *selector_desc = NULL;

	type = (proto == UAC_VERSION_3 ? (UAC3_CLOCK_SELECTOR) : (UAC2_CLOCK_SELECTOR));
	selector_desc = (union uac23_clock_selector_desc *)find_uac_clock_desc(chip->ctrl_intf,
		id, validate_clock_selector, type, proto);
	return selector_desc;
}

static union uac23_clock_multiplier_desc
	*aud_usb_find_clock_multiplier(struct snd_usb_audio *chip, int id, int proto)
{
	u8 type;
	union uac23_clock_multiplier_desc *multi_desc = NULL;

	type = (proto == UAC_VERSION_3 ? (UAC3_CLOCK_MULTIPLIER) : (UAC2_CLOCK_MULTIPLIER));
	multi_desc = (union uac23_clock_multiplier_desc *)find_uac_clock_desc(chip->ctrl_intf,
		id, validate_clock_multiplier, type, proto);
	return multi_desc;
}

void snd_usb_ctl_msg_quirk(struct usb_device *dev, unsigned int pipe,
			   __u8 request, __u8 requesttype, __u16 value,
			   __u16 index, void *data, __u16 size)
{
	struct snd_usb_audio *chip = dev_get_drvdata(&dev->dev);

	if (!chip || (requesttype & USB_TYPE_MASK) != USB_TYPE_CLASS)
		return;

	if (chip->quirk_flags & QUIRK_FLAG_CTL_MSG_DELAY)
		msleep(20);
	else if (chip->quirk_flags & QUIRK_FLAG_CTL_MSG_DELAY_1M)
		usleep_range(1000, 2000);
	else if (chip->quirk_flags & QUIRK_FLAG_CTL_MSG_DELAY_5M)
		usleep_range(5000, 6000);
}

int snd_usb_ctl_msg(struct usb_device *dev, unsigned int pipe, __u8 request,
		    __u8 requesttype, __u16 value, __u16 index, void *data,
		    __u16 size)
{
	int err;
	void *buf = NULL;
	int timeout;

	if (usb_pipe_type_check(dev, pipe))
		return -EINVAL;

	if (size > 0) {
		buf = kmemdup(data, size, GFP_KERNEL);
		if (!buf)
			return -ENOMEM;
	}

	if (requesttype & USB_DIR_IN)
		timeout = USB_CTRL_GET_TIMEOUT;
	else
		timeout = USB_CTRL_SET_TIMEOUT;

	err = usb_control_msg(dev, pipe, request, requesttype,
			      value, index, buf, size, timeout);

	if (size > 0) {
		memcpy(data, buf, size);
		kfree(buf);
	}

	snd_usb_ctl_msg_quirk(dev, pipe, request, requesttype,
			      value, index, data, size);

	return err;
}

static int uac_clock_selector_get_val(struct snd_usb_audio *chip, int selector_id)
{
	unsigned char buf;
	int ret;

	ret = snd_usb_ctl_msg(chip->dev, usb_rcvctrlpipe(chip->dev, 0),
			      UAC2_CS_CUR,
			      USB_RECIP_INTERFACE | USB_TYPE_CLASS | USB_DIR_IN,
			      UAC2_CX_CLOCK_SELECTOR << 8,
			      snd_usb_ctrl_intf(chip) | (selector_id << 8),
			      &buf, sizeof(buf));

	if (ret < 0)
		return ret;

	return buf;
}

static int uac_clock_selector_set_val(struct snd_usb_audio *chip, int selector_id,
					unsigned char pin)
{
	int ret;

	ret = snd_usb_ctl_msg(chip->dev, usb_sndctrlpipe(chip->dev, 0),
			      UAC2_CS_CUR,
			      USB_RECIP_INTERFACE | USB_TYPE_CLASS | USB_DIR_OUT,
			      UAC2_CX_CLOCK_SELECTOR << 8,
			      snd_usb_ctrl_intf(chip) | (selector_id << 8),
			      &pin, sizeof(pin));
	if (ret < 0)
		return ret;

	if (ret != sizeof(pin)) {
		AUD_LOG_INFO(AUD_COMM,
			"setting selector (id %d) unexpected length %d",
			selector_id, ret);
		return -EINVAL;
	}

	ret = uac_clock_selector_get_val(chip, selector_id);
	if (ret < 0)
		return ret;

	if (ret != pin) {
		AUD_LOG_INFO(AUD_COMM,
			"setting selector (id %d) to %x failed (current: %d)",
			selector_id, pin, ret);
		return -EINVAL;
	}

	return ret;
}

static bool uac_clock_source_is_valid_quirk(struct snd_usb_audio *chip,
					    const struct audioformat *fmt,
					    int source_id)
{
	bool ret = false;
	int count;
	unsigned char data;
	struct usb_device *dev = chip->dev;
	union uac23_clock_source_desc *cs_desc;

	AUD_LOG_INFO(AUD_COMM, "find clock source");
	cs_desc = aud_usb_find_clock_source(chip, source_id, fmt->protocol);
	if (!cs_desc)
		return false;

	if (fmt->protocol == UAC_VERSION_2) {
		/*
		 * Assume the clock is valid if clock source supports only one
		 * single sample rate, the terminal is connected directly to it
		 * (there is no clock selector) and clock type is internal.
		 * This is to deal with some Denon DJ controllers that always
		 * reports that clock is invalid.
		 */
		if (fmt->nr_rates == 1 &&
		    (fmt->clock & 0xff) == cs_desc->v2.bClockID &&
		    (cs_desc->v2.bmAttributes & 0x3) !=
				UAC_CLOCK_SOURCE_TYPE_EXT)
			return true;
	}

	/*
	 * MOTU MicroBook IIc
	 * Sample rate changes takes more than 2 seconds for this device. Clock
	 * validity request returns false during that period.
	 */
	if (chip->usb_id == USB_ID(0x07fd, 0x0004)) {
		count = 0;

		while ((!ret) && (count < 50)) {
			int err;

			msleep(100);

			AUD_LOG_INFO(AUD_COMM, "check usb id. count=%d", count);
			err = snd_usb_ctl_msg(dev, usb_rcvctrlpipe(dev, 0), UAC2_CS_CUR,
					      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
					      UAC2_CS_CONTROL_CLOCK_VALID << 8,
					      snd_usb_ctrl_intf(chip) | (source_id << 8),
					      &data, sizeof(data));
			if (err < 0) {
				AUD_LOG_ERR(AUD_COMM,
					"cannot get clock validity for id %d",
					source_id);
				return false;
			}

			ret = !!data;
			count++;
		}
	}

	return ret;
}

static bool uac_clock_source_is_valid(struct snd_usb_audio *chip,
				      const struct audioformat *fmt,
				      int source_id)
{
	int err;
	unsigned char data;
	struct usb_device *dev = chip->dev;
	u32 bmControls;
	union uac23_clock_source_desc *cs_desc;

	cs_desc = aud_usb_find_clock_source(chip, source_id, fmt->protocol);
	if (!cs_desc)
		return false;

	if (fmt->protocol == UAC_VERSION_3)
		bmControls = le32_to_cpu(cs_desc->v3.bmControls);
	else
		bmControls = cs_desc->v2.bmControls;

	/* If a clock source can't tell us whether it's valid, we assume it is */
	if (!uac_v2v3_control_is_readable(bmControls,
			UAC2_CS_CONTROL_CLOCK_VALID)) {
		AUD_LOG_INFO(AUD_COMM, "check readable");
		return true;
	}

	err = snd_usb_ctl_msg(dev, usb_rcvctrlpipe(dev, 0), UAC2_CS_CUR,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
			      UAC2_CS_CONTROL_CLOCK_VALID << 8,
			      snd_usb_ctrl_intf(chip) | (source_id << 8),
			      &data, sizeof(data));

	if (err < 0) {
		AUD_LOG_INFO(AUD_COMM,
			"cannot get clock validity for id %d",
			source_id);
		return false;
	}

	if (data)
		return true;
	else
		return uac_clock_source_is_valid_quirk(chip, fmt, source_id);
}

static int __uac_clock_find_source(struct snd_usb_audio *chip,
				   const struct audioformat *fmt, int entity_id,
				   unsigned long *visited, bool validate)
{
	union uac23_clock_source_desc *source;
	union uac23_clock_selector_desc *selector;
	union uac23_clock_multiplier_desc *multiplier;
	int ret, i, cur, err, pins, clock_id;
	const u8 *sources;
	int proto = fmt->protocol;

	entity_id &= 0xff;

	if (test_and_set_bit(entity_id, visited)) {
		AUD_LOG_INFO(AUD_COMM,
			"recursive clock topology detected, id %d.",
			entity_id);
		return -EINVAL;
	}

	/* first, see if the ID we're looking at is a clock source already */
	source = aud_usb_find_clock_source(chip, entity_id, proto);
	if (source) {
		entity_id = GET_VAL(source, proto, bClockID);
		if (validate && !uac_clock_source_is_valid(chip, fmt,
								entity_id)) {
			AUD_LOG_INFO(AUD_COMM,
				"clock source %d is not valid, cannot use",
				entity_id);
			return -ENXIO;
		}
		AUD_LOG_INFO(AUD_COMM, "entity id 0x%x", entity_id);
		return entity_id;
	}

	AUD_LOG_INFO(AUD_COMM, "find clock selector");

	selector = aud_usb_find_clock_selector(chip, entity_id, proto);
	if (selector) {
		pins = GET_VAL(selector, proto, bNrInPins);
		clock_id = GET_VAL(selector, proto, bClockID);
		sources = GET_VAL(selector, proto, baCSourceID);
		cur = 0;

		if (pins == 1) {
			ret = 1;
			goto find_source;
		}

		AUD_LOG_INFO(AUD_COMM, "get clock selector");
		/* the entity ID we are looking at is a selector.
		 * find out what it currently selects.
		 */
		ret = uac_clock_selector_get_val(chip, clock_id);
		if (ret < 0) {
			if (!chip->autoclock)
				return ret;
			goto find_others;
		}

		/* Selector values are one-based */

		if (ret > pins || ret < 1) {
			AUD_LOG_INFO(AUD_COMM,
				"selector reported illegal value, id %d, ret %d",
				clock_id, ret);

			if (!chip->autoclock)
				return -EINVAL;
			goto find_others;
		}

find_source:
		AUD_LOG_INFO(AUD_COMM, "find source");
		cur = ret;
		ret = __uac_clock_find_source(chip, fmt,
			sources[ret - 1], visited, validate);
		if (ret > 0) {
			/* Skip setting clock selector again for some devices */
			if (chip->quirk_flags & QUIRK_FLAG_SKIP_CLOCK_SELECTOR)
				return ret;
			err = uac_clock_selector_set_val(chip, entity_id, cur);
			if (err < 0) {
				if (pins == 1) {
					AUD_LOG_INFO(AUD_COMM,
						"selector returned an error. id %d, ret %d",
						clock_id, err);
					return ret;
				}
				return err;
			}
		}

		if (!validate || ret > 0 || !chip->autoclock)
			return ret;

find_others:
		AUD_LOG_INFO(AUD_COMM, "find other source");
		/* The current clock source is invalid, try others. */
		for (i = 1; i <= pins; i++) {
			if (i == cur)
				continue;

			ret = __uac_clock_find_source(chip, fmt,
						      sources[i - 1],
						      visited, true);
			if (ret < 0)
				continue;

			AUD_LOG_INFO(AUD_COMM, "set clock selector(%d)", i);
			err = uac_clock_selector_set_val(chip, entity_id, i);
			if (err < 0)
				continue;

			AUD_LOG_INFO(AUD_COMM,
				"found and selected valid clock source %d",
				ret);
			return ret;
		}

		return -ENXIO;
	}

	/* FIXME: multipliers only act as pass-thru element for now */
	multiplier = aud_usb_find_clock_multiplier(chip, entity_id, proto);
	if (multiplier)
		return __uac_clock_find_source(chip, fmt,
					       GET_VAL(multiplier, proto, bCSourceID),
					       visited, validate);

	AUD_LOG_INFO(AUD_COMM, "end");
	return -EINVAL;
}

int audio_usb_clock_find_source(struct snd_usb_audio *chip,
			      const struct audioformat *fmt, bool validate)
{
	DECLARE_BITMAP(visited, 256);

	memset(visited, 0, sizeof(visited));

	switch (fmt->protocol) {
	case UAC_VERSION_2:
	case UAC_VERSION_3:
		return __uac_clock_find_source(chip, fmt, fmt->clock, visited,
					       validate);
	default:
		return -EINVAL;
	}
}

bool audio_usb_pcm_has_fixed_rate(struct snd_usb_substream *subs)
{
	const struct audioformat *fp;
	struct snd_usb_audio *chip;
	int rate = -1;

	if (!subs || !subs->stream)
		return false;
	chip = subs->stream->chip;
	if (!chip)
		return false;

	if (!(chip->quirk_flags & QUIRK_FLAG_FIXED_RATE))
		return false;

	list_for_each_entry(fp, &subs->fmt_list, list) {
		if (fp->rates & SNDRV_PCM_RATE_CONTINUOUS)
			return false;

		if (fp->nr_rates < 1)
			continue;

		if (fp->nr_rates > 1)
			return false;

		if (rate < 0) {
			rate = fp->rate_table[0];
			continue;
		}

		if (rate != fp->rate_table[0])
			return false;
	}

	return true;
}

void usb2aud_cb_register(usb2adu_cb cb, void *priv)
{
	AUD_LOG_INFO(AUD_COMM, "enter");
	usb_cb = cb;
	usb_priv = priv;
}
EXPORT_SYMBOL(usb2aud_cb_register);

void usb2aud_cb_unregister(void)
{
	AUD_LOG_DBG(AUD_COMM, "enter");
	usb_cb = NULL;
	usb_priv = NULL;
}
EXPORT_SYMBOL(usb2aud_cb_unregister);

static int convert_bytes_value(struct usb_mixer_elem_info *cval, int val)
{
	switch (cval->val_type) {
	case USB_MIXER_BOOLEAN:
		return !!val;
	case USB_MIXER_INV_BOOLEAN:
		return !val;
	case USB_MIXER_S8:
	case USB_MIXER_U8:
		return val & 0xff;
	case USB_MIXER_S16:
	case USB_MIXER_U16:
		return val & 0xffff;
	}
	return 0; /* not reached */
}

static int uac2_ctl_value_size(int val_type)
{
	switch (val_type) {
	case USB_MIXER_S32:
	case USB_MIXER_U32:
		return 4;
	case USB_MIXER_S16:
	case USB_MIXER_U16:
		return 2;
	default:
		return 1;
	}
	return 0; /* unreachable */
}

static int get_abs_value(struct usb_mixer_elem_info *cval, int val)
{
	if (val < 0)
		return cval->min;
	if (!cval->res)
		cval->res = 1;
	val *= cval->res;
	val += cval->min;
	if (val > cval->max)
		return cval->max;
	return val;
}

static int usb_mixer_set_ctl_value(struct usb_mixer_elem_info *cval,
	int request, int validx, struct usb_ctrl_param *ctrl_param, int value_set)
{
	struct snd_usb_audio *chip = cval->head.mixer->chip;
	unsigned char buf[4];
	int idx = 0, val_len;

	validx += cval->idx_off;

	AUD_LOG_INFO(AUD_COMM, "validx=%d off=%d",
		validx, cval->idx_off);

	if (cval->head.mixer->protocol == UAC_VERSION_1) {
		val_len = cval->val_type >= USB_MIXER_S16 ? 2 : 1;
	} else { /* UAC_VERSION_2/3 */
		val_len = uac2_ctl_value_size(cval->val_type);

		if (request != UAC_SET_CUR) {
			AUD_LOG_INFO(AUD_COMM, "RANGE setting not yet supported");
			return -EINVAL;
		}

		request = UAC2_CS_CUR;
	}

	value_set = convert_bytes_value(cval, value_set);
	buf[0] = value_set & 0xff;
	buf[1] = (value_set >> 8) & 0xff;
	buf[2] = (value_set >> 16) & 0xff;
	buf[3] = (value_set >> 24) & 0xff;

	AUD_LOG_INFO(AUD_COMM, "type=%d value_set=%x", cval->val_type, value_set);
	idx = get_iface_desc(cval->head.mixer->hostif)->bInterfaceNumber
		| (cval->head.id << 8);
	AUD_LOG_INFO(AUD_COMM, "set ctl value:req=%#x wValue=%#x, wIndex=%#x, type=%d",
		request, validx, idx, cval->val_type);
	AUD_LOG_INFO(AUD_COMM, "set ctl value:pipe=%x data = %#x/%#x",
		usb_sndctrlpipe(chip->dev, 0), buf[0], buf[1]);

	/* update snd_usb_ctl_msg parameter info */
	ctrl_param->pipe = usb_sndctrlpipe(chip->dev, 0);
	ctrl_param->request = request;
	ctrl_param->request_type = USB_RECIP_INTERFACE | USB_TYPE_CLASS | USB_DIR_OUT;
	ctrl_param->value = validx;
	ctrl_param->index = idx;
	ctrl_param->buf = value_set;
	ctrl_param->size = val_len;

	return 0;
}

static int usb_set_mix_value(struct usb_mixer_elem_info *cval, int channel,
			     int index, struct usb_ctrl_param *ctrl_param, int value)
{
	int err;
	unsigned int read_only = (channel == 0) ?
		cval->master_readonly :
		cval->ch_readonly & (1 << (channel - 1));

	if (read_only) {
		AUD_LOG_INFO(AUD_COMM,
			"channel %d of control %d is read_only",
			channel, cval->control);
		return 0;
	}

	AUD_LOG_INFO(AUD_COMM,
		"channel %d of control %d value %d",
		channel, cval->control, value);

	err = usb_mixer_set_ctl_value(cval, UAC_SET_CUR,
		(cval->control << 8) | channel, ctrl_param, value);
	if (err < 0)
		return err;
	cval->cached |= 1 << channel;
	cval->cache_val[index] = value;
	return 0;
}

static int audio_usb_ctrl_put(struct snd_usb_audio *chip,
	struct usb_mixer_elem_info *cval,
	struct usb_ctrl_param *ctrl_param,
	int val)
{
	int c;
	int cnt = 0;
	int abs_val;

	if (cval->cmask) {
		for (c = 0; c < MAX_CHANNELS; c++) {
			if (!(cval->cmask & (1 << c)))
				continue;

			abs_val = get_abs_value(cval, val);
			usb_set_mix_value(cval, c + 1, cnt, ctrl_param, abs_val);
			cnt++;
			ctrl_param++;

			AUD_LOG_INFO(AUD_COMM, "mask=0x%x cnt=%d val=%d",
				cval->cmask, cnt, abs_val);
		}
	} else {
		/* master channel */
		abs_val = get_abs_value(cval, val);
		usb_set_mix_value(cval, 0, 0, ctrl_param, abs_val);
		cnt++;

		AUD_LOG_INFO(AUD_COMM, "mask=0x%x val=%d", cval->cmask, abs_val);
	}

	return cnt;
}

void audio_usb_mode_set(int usb_offload)
{
	int card_id = -1;
	int i = 0;

	/* set usb no-offload mode */
	if (usb_offload) {
		AUD_LOG_INFO(AUD_COMM, "offload mode");
		atomic_set(&usb_mode, 1);
	} else {
		AUD_LOG_INFO(AUD_COMM, "no-offload mode");
		atomic_set(&usb_mode, 0);
	}

	for (i = 0; i < SNDRV_CARDS; i++) {
		if (atomic_read(&audev[i].connected)) {
			card_id = i;
			AUD_LOG_INFO(AUD_COMM, "usb card(%d) connect before mode config", card_id);
			if (audev[card_id].chip)
				audio_usb_disconnect(audev[card_id].chip);
		}
	}
}
EXPORT_SYMBOL(audio_usb_mode_set);

void audio_usb_status_notify_complete(int cmd)
{
	AUD_LOG_INFO(AUD_COMM, "usb cmd(%d) RSP", cmd);

	atomic_set(&ausb_wait, 0);
	wake_up(&ausb_wait_que);
}

/************************************************************************
 * function: audio_usb_pcm_format_get
 * description: get pcm format value by bit width
 * parameter:
 *		bit_width : bit width value
 * return:
 *		snd_pcm_format_t
 ************************************************************************/
static int audio_usb_status_notify(struct snd_usb_audio *chip, int cmd)
{
	int ret = EOK;
	struct aud_usb_dev *auDev = NULL;
	int card_num;
	int i;

	struct xring_aud_msg usb_status_msg = {0};
	struct usb_mixer_info *mixer_info;

	AUD_LOG_INFO(AUD_COMM, "cmd is 0x%x.", cmd);

	card_num = chip->card->number;
	if (card_num >= SNDRV_CARDS) {
		AUD_LOG_ERR(AUD_COMM, "Invalid card number(%d)", card_num);
		return -EINVAL;
	}
	auDev = &audev[card_num];

	/* fill usb status notify message */
	usb_status_msg.header.scene_id = (uint8_t)ADSP_SCENE_NONE;
	usb_status_msg.header.ins_id = 0;
	usb_status_msg.header.cmd_id = ADSP_HW_MODULE_CTRL;
	if (cmd == ADSP_SET_USB_CONNECT) {
		usb_status_msg.header.func_id = ADSP_SET_USB_CONNECT;
		usb_status_msg.header.len = sizeof(struct xring_aud_msg_header) +
			sizeof(struct usb_mixer_info);

		if (usb_status_msg.header.len > MBX_BLK_SIZE) {
			AUD_LOG_ERR(AUD_COMM, "Invalid message length(%d)",
				usb_status_msg.header.len);
			return -EINVAL;
		}

		mixer_info = (struct usb_mixer_info *)&usb_status_msg.data[0];
		mixer_info->ctrl_num = auDev->items_num;
		AUD_LOG_INFO(AUD_CORE, "items number %d.", mixer_info->ctrl_num);

		for (i = 0; i < mixer_info->ctrl_num; i++) {
			if (i >= MAX_ELEMS_NUM) {
				AUD_LOG_INFO(AUD_CORE, "items number max");
				break;
			}

			mixer_info->ctrl_msg[i].ctrl_type = auDev->item_param[i].ctrl_type;
			mixer_info->ctrl_msg[i].pipe = auDev->item_param[i].pipe;
			mixer_info->ctrl_msg[i].request = auDev->item_param[i].request;
			mixer_info->ctrl_msg[i].request_type =
				auDev->item_param[i].request_type;
			mixer_info->ctrl_msg[i].value = auDev->item_param[i].value;
			mixer_info->ctrl_msg[i].index = auDev->item_param[i].index;
			mixer_info->ctrl_msg[i].buf = auDev->item_param[i].buf;
			mixer_info->ctrl_msg[i].size = auDev->item_param[i].size;

			AUD_LOG_INFO(AUD_CORE, "items(%d) type(%x) pipe(%x) request(%x)",
				i, mixer_info->ctrl_msg[i].ctrl_type,
				mixer_info->ctrl_msg[i].pipe,
				mixer_info->ctrl_msg[i].request);
			AUD_LOG_INFO(AUD_CORE, "request_type(%x) value(%x) index(%x)",
				mixer_info->ctrl_msg[i].request_type,
				mixer_info->ctrl_msg[i].value,
				mixer_info->ctrl_msg[i].index);
			AUD_LOG_INFO(AUD_CORE, "buf(%x) size(%x)", mixer_info->ctrl_msg[i].buf,
				mixer_info->ctrl_msg[i].size);
		}
	} else {
		usb_status_msg.header.func_id = ADSP_SET_USB_DISCONNECT;
		usb_status_msg.header.len = sizeof(struct xring_aud_msg_header);
		usb_status_msg.header.result = 0;
	}

	/* send audio usb status message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&usb_status_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send start message failure.");
		return ret;
	}

	atomic_set(&ausb_wait, 1);
	/* wait for dsp startup ack */
	ret = wait_event_timeout(ausb_wait_que,
		!atomic_read(&ausb_wait), WAIT_TIMEOUT_SHORT);
	if (!ret && atomic_read(&ausb_wait))
		AUD_LOG_INFO(AUD_CORE, "wait usb notify complete.");

	return ret;
}

/************************************************************************
 * function: audio_usb_pcm_format_get
 * description: get pcm format value by bit width
 * parameter:
 *		bit_width : bit width value
 * return:
 *		snd_pcm_format_t
 ************************************************************************/
int audio_usb_pcm_fmt_get(uint32_t bit_width, struct snd_usb_substream *subs)
{
	struct audioformat *fp = NULL;
	snd_pcm_format_t fmt;

	list_for_each_entry(fp, &subs->fmt_list, list) {
		if (fp->fmt_bits == bit_width) {
			pcm_for_each_format(fmt)
				if (fp->formats & pcm_format_to_bits(fmt)) {
					AUD_LOG_INFO(AUD_COMM, "fmt(%d) format(%llx)",
						fmt, fp->formats);
					return (int)fmt;
				}
		}
	}

	switch (bit_width) {
	case 8:
		return SNDRV_PCM_FORMAT_S8;
	case 24:
		return SNDRV_PCM_FORMAT_S24_3LE;
	case 32:
		return SNDRV_PCM_FORMAT_S32_LE;
	default:
	case 16:
		return SNDRV_PCM_FORMAT_S16_LE;
	}
}

static struct snd_usb_substream *find_audio_usb_format(
	int card_id, int dir, struct dai_format dai_fmt,
	struct snd_pcm_hw_params *params)
{
	struct snd_usb_stream *as = NULL;
	struct snd_usb_audio *chip = NULL;
	struct snd_usb_substream *subs = NULL;
	const struct audioformat *found = NULL;

	struct snd_interval *rate = NULL;
	struct snd_interval *channels = NULL;
	unsigned int bits_fmt;

	if (dir >= STREAM_DIR_MAX) {
		AUD_LOG_INFO(AUD_COMM, "stream dir(%d) unexpected", dir);
		return NULL;
	}

	chip = audev[card_id].chip;
	if (chip == NULL) {
		AUD_LOG_INFO(AUD_COMM, "chip(%d) is NULL.", card_id);
		return NULL;
	}

	if (params == NULL) {
		AUD_LOG_INFO(AUD_COMM, "params is NULL.");
		return NULL;
	}

	rate = hw_param_interval(params, SNDRV_PCM_HW_PARAM_RATE);
	channels = hw_param_interval(params, SNDRV_PCM_HW_PARAM_CHANNELS);
	rate->min = rate->max = dai_fmt.samples;
	channels->min = channels->max = dai_fmt.channels;

	list_for_each_entry(as, &chip->pcm_list, list) {
		subs = &as->substream[dir];

		bits_fmt = (unsigned int)audio_usb_pcm_fmt_get(dai_fmt.bit_width, subs);
		params_set_format(params, bits_fmt);

		AUD_LOG_INFO(AUD_COMM, "fmt list:%d fmt:%d rate:%d channels:%d",
			as->fmt_type, params_format(params),
			params_rate(params), params_channels(params));

		if ((subs->pcm_substream != NULL) && (subs->pcm_substream->pcm != NULL))
			AUD_LOG_INFO(AUD_COMM, "pcm stream name(%s) index(%d).",
				(char *)&subs->pcm_substream->pcm->name, as->pcm_index);
		found = snd_usb_find_format(&subs->fmt_list, params_format(params),
			params_rate(params), params_channels(params), true, subs);
		if (found != NULL) {
			AUD_LOG_INFO(AUD_COMM, "audio format match. iface=%d altsetting=%d",
				(int)found->iface, (int)found->altsetting);
			subs->cur_audiofmt = found;
			return subs;
		}
	}

	AUD_LOG_INFO(AUD_COMM, "audio format unmatch");

	return NULL;
}

int audio_usb_offload_param_prepare(int dir,
	struct dai_format dai_fmt)
{
	int ret = 0;
	int card_id = -1;
	int i = 0;
	struct snd_usb_substream *subs = NULL;
	struct snd_usb_endpoint *data_ep = NULL;
	struct snd_usb_endpoint *sync_ep = NULL;

	struct snd_pcm_hw_params params;

	if (dir >= STREAM_DIR_MAX) {
		AUD_LOG_INFO(AUD_COMM, "stream dir(%d) unexpected", dir);
		return -1;
	}

	for (i = 0; i < SNDRV_CARDS; i++) {
		if (atomic_read(&audev[i].connected)) {
			card_id = i;
			break;
		}
	}

	if (card_id < 0) {
		AUD_LOG_INFO(AUD_COMM, "audio usb not connect");
		return -1;
	}

	subs = find_audio_usb_format(card_id, dir, dai_fmt, &params);
	if (subs == NULL) {
		AUD_LOG_INFO(AUD_COMM, "subs unmatch");
		return -1;
	}

	ret = snd_usb_hw_params(subs, &params);
	if (ret < 0) {
		AUD_LOG_INFO(AUD_COMM, "audio usb parameter fail");
		return -1;
	}

	data_ep = subs->data_endpoint;
	if (data_ep == NULL) {
		AUD_LOG_INFO(AUD_COMM, "data ep empty");
		return -1;
	}

	AUD_LOG_INFO(AUD_COMM, "dir=%d pipe=0x%x packets=%d", dir, data_ep->pipe,
			data_ep->urb[0].packets);
	AUD_LOG_INFO(AUD_COMM, "intf=%d alts=%d nurbs=%d max_packet_size=%d",
		data_ep->iface, data_ep->altsetting,
		data_ep->nurbs, data_ep->maxpacksize);

	audev[card_id].info[dir].intf_num = data_ep->iface;
	audev[card_id].info[dir].alt_setting = data_ep->altsetting;
	audev[card_id].info[dir].idle_intf_num = audev[card_id].intf0_num;
	audev[card_id].info[dir].alt_setting = audev[card_id].alt0_setting;
	audev[card_id].info[dir].nurbs = data_ep->nurbs;
	audev[card_id].info[dir].max_packet_size = data_ep->maxpacksize;
	audev[card_id].info[dir].pipe = data_ep->pipe;
	audev[card_id].info[dir].packet_num = data_ep->urb[0].packets;

	sync_ep = subs->sync_endpoint;
	if (sync_ep) {
		audev[card_id].info[dir].sync_pipe = sync_ep->pipe;
		audev[card_id].info[dir].sync_interval = sync_ep->syncinterval;
		AUD_LOG_INFO(AUD_COMM, "sync_pipe=0x%x datainterval=0x%x syncinterval=0x%x",
			sync_ep->pipe, sync_ep->datainterval, sync_ep->syncinterval);
	}

	return ret;
}

int audio_usb_offload_param_get(struct aud_usb_offload_info *info,
	int dir, struct dai_format dai_fmt)
{
	int ret = 0;
	int card_id = -1;
	struct snd_usb_audio *chip = NULL;
	struct snd_usb_substream *subs = NULL;
	struct snd_usb_endpoint *data_ep = NULL;
	struct snd_pcm_hw_params params;
	const struct audioformat *fmt;
	int i = 0;

	for (i = 0; i < SNDRV_CARDS; i++) {
		if (atomic_read(&audev[i].connected)) {
			card_id = i;
			break;
		}
	}

	if (card_id < 0) {
		AUD_LOG_INFO(AUD_COMM, "audio usb not connect");
		return -1;
	}

	chip = audev[card_id].chip;
	if ((chip == NULL) || (chip->card == NULL)) {
		AUD_LOG_INFO(AUD_COMM, "chip or card(%d) is NULL.", card_id);
		return -1;
	}

	ret = snd_usb_lock_shutdown(chip);
	if (ret < 0)
		return ret;

	subs = find_audio_usb_format(card_id, dir, dai_fmt, &params);
	if (subs == NULL) {
		AUD_LOG_INFO(AUD_COMM, "subs unmatch");
		ret = -1;
		goto out;
	}

	ret = snd_usb_hw_params(subs, &params);
	if (ret < 0) {
		AUD_LOG_INFO(AUD_COMM, "audio usb parameter fail");
		ret = -1;
		goto out;
	}

	data_ep = subs->data_endpoint;
	if (data_ep == NULL) {
		AUD_LOG_INFO(AUD_COMM, "data ep empty");
		ret = -1;
		goto out;
	}

	if (audio_usb_pcm_has_fixed_rate(subs))
		AUD_LOG_INFO(AUD_COMM, "use fix rate");
	else
		AUD_LOG_INFO(AUD_COMM, "no use fix rate");

	/* get format attributes for sample rate config */
	fmt = data_ep->cur_audiofmt;
	if (fmt && (fmt->protocol == UAC_VERSION_1)) {
		AUD_LOG_INFO(AUD_COMM, "attributes=0x%x", fmt->attributes);
		info->attributes = (uint8_t)fmt->attributes;
	} else
		info->attributes = 0;

	AUD_LOG_INFO(AUD_COMM, "intf=%d alts=%d nurbs=%d max_packet_size=%d",
		data_ep->iface, data_ep->altsetting,
		data_ep->nurbs, data_ep->maxpacksize);
	AUD_LOG_INFO(AUD_COMM, "dir=%d pipe=0x%x packets=%d", dir, data_ep->pipe,
		data_ep->urb[0].packets);
	info->intf_num = data_ep->iface;
	info->alt_setting = data_ep->altsetting;
	info->nurbs = data_ep->nurbs;
	info->max_packet_size = data_ep->maxpacksize;
	info->pipe = data_ep->pipe;
	info->packet_num = data_ep->urb[0].packets;
	info->datainterval = data_ep->datainterval;

	info->idle_intf_num = audev[card_id].intf0_num;
	info->idle_alt_setting = audev[card_id].alt0_setting;

	if (subs->sync_endpoint) {
		info->sync_pipe = subs->sync_endpoint->pipe;
		info->sync_interval = subs->sync_endpoint->syncinterval;
		AUD_LOG_INFO(AUD_COMM, "sync ep: pipe(0x%x) datainterval(0x%x) syncinterval(0x%x)",
			subs->sync_endpoint->pipe, subs->sync_endpoint->datainterval,
			subs->sync_endpoint->syncinterval);
	}

	AUD_LOG_INFO(AUD_COMM, "intf0=%d alt0=%d intr_num=%d alt_setting=%d interval=%d",
		info->idle_intf_num, info->idle_alt_setting,
		info->intf_num, info->alt_setting, info->datainterval);

	info->clock_id = (int32_t)audio_usb_clock_find_source(audev[card_id].chip,
			subs->cur_audiofmt, true);
	if (info->clock_id < 0) {
		/* We did not find a valid clock, but that might be
		 * because the current sample rate does not match an
		 * external clock source. Try again without validation
		 * and we will do another validation after setting the
		 * rate.
		 */
		AUD_LOG_INFO(AUD_COMM, "try find clock again");
		info->clock_id = audio_usb_clock_find_source(audev[card_id].chip,
			subs->cur_audiofmt, false);

		/* Hardcoded sample rates */
		if (audev[card_id].chip->quirk_flags & QUIRK_FLAG_IGNORE_CLOCK_SOURCE)
			info->clock_id = 0;

		if (info->clock_id < 0)
			info->clock_id = 0;
	}
	info->protocol = subs->cur_audiofmt->protocol;
	info->ctrl_intf = snd_usb_ctrl_intf(audev[card_id].chip);
	AUD_LOG_INFO(AUD_COMM, "clock_id(0x%x) protocol(0x%x) ctrl_intf(0x%x)",
		info->clock_id, info->protocol, info->ctrl_intf);

	if (audev[card_id].jack_type == SND_JACK_MICROPHONE)
		info->dev_type = 1;
	else if (audev[card_id].jack_type == SND_JACK_HEADPHONE)
		info->dev_type = 2;
	else
		info->dev_type = 0;

	AUD_LOG_INFO(AUD_COMM, "usb device type(%d)", info->dev_type);

out:
	snd_usb_unlock_shutdown(chip);
	return ret;
}

/* audio usb info: begin */
int audio_usb_connect_set(bool val)
{
	return 0;
}

int audio_usb_info_query(void)
{
	int ret = 0;
	int i = 0;

	AUD_LOG_INFO(AUD_COMM, "as follow:");

	for (i = 0; i < SNDRV_CARDS; i++) {
		if (atomic_read(&audev[i].connected)) {
			AUD_LOG_INFO(AUD_COMM, "  : card[%d] usb connect.", i);
			audio_usb_ep_info(i);

			AUD_LOG_INFO(AUD_COMM, "    : intf(%d)(%d).",
				audev[i].info[0].intf_num, audev[i].info[1].intf_num);
			AUD_LOG_INFO(AUD_COMM, "    : alt_setting(%d)(%d).",
				audev[i].info[0].alt_setting, audev[i].info[1].alt_setting);
			AUD_LOG_INFO(AUD_COMM, "    : nurbs(%d)(%d).",
				audev[i].info[0].nurbs, audev[i].info[1].nurbs);
			AUD_LOG_INFO(AUD_COMM, "    : packet_num(%d)(%d).",
				audev[i].info[0].packet_num, audev[i].info[1].packet_num);
			AUD_LOG_INFO(AUD_COMM, "    : pipe(%x)(%x).",
				audev[i].info[0].pipe, audev[i].info[1].pipe);
			AUD_LOG_INFO(AUD_COMM, "    : max_packet_size(%x)(%x).",
				audev[i].info[0].max_packet_size, audev[i].info[1].max_packet_size);
		}
	}

	return ret;
}

/* audio usb info: end */

static void audio_usb_ep_info(int card_id)
{
	int i = 0;
	struct snd_usb_audio *chip = NULL;
	struct snd_usb_stream *as = NULL;
	struct snd_usb_substream *subs = NULL;

	AUD_LOG_INFO(AUD_COMM, "card[%d]:", card_id);

	chip = audev[card_id].chip;
	AUD_LOG_INFO(AUD_COMM, "connect status:%d", atomic_read(&audev[card_id].connected));
	if (chip) {
		list_for_each_entry(as, &chip->pcm_list, list) {
			for (i = 0; i < STREAM_DIR_MAX; i++) {
				subs = &as->substream[i];
				if (subs->data_endpoint) {
					AUD_LOG_INFO(AUD_COMM,
						"strm[%d]: rate(%d) chs(%d) format(%s) p_bytes(%d)",
						i, subs->data_endpoint->cur_rate,
						subs->data_endpoint->cur_channels,
						snd_pcm_format_name(subs->data_endpoint->cur_format),
						subs->data_endpoint->cur_frame_bytes);
				}
			}
		}
	}
}

static void audio_usb_mixer_info(struct snd_usb_audio *chip)
{
	struct usb_mixer_interface *mixer;
	struct usb_mixer_elem_info *cval;
	struct usb_mixer_elem_list *list;
	int unitid;
	struct aud_usb_dev *auDev = NULL;
	int card_num, item_idx;

	int unmute = 0;
	int volume = 100;
	int cnt = 1;

	card_num = chip->card->number;
	if (card_num >= SNDRV_CARDS) {
		AUD_LOG_ERR(AUD_COMM, "Invalid card number(%d)", card_num);
		return;
	}

	AUD_LOG_INFO(AUD_COMM, "card number(%d)", card_num);
	auDev = &audev[card_num];

	list_for_each_entry(mixer, &chip->mixer_list, list) {
		AUD_LOG_INFO(AUD_COMM,
			"USB Mixer: usb_id=0x%08x, ctrlif=%i, ctlerr=%i",
				chip->usb_id, get_iface_desc(mixer->hostif)->bInterfaceNumber,
				mixer->ignore_ctl_error);
		AUD_LOG_INFO(AUD_COMM, "Card: %s", chip->card->longname);
		for (unitid = 0; unitid < MAX_ID_ELEMS; unitid++) {
			for_each_mixer_elem(list, mixer, unitid) {
				AUD_LOG_INFO(AUD_COMM, "  Unit: %i", list->id);
				if (list->kctl)
					AUD_LOG_INFO(AUD_COMM,
						    "    Control: name=\"%s\", index=%i",
						    list->kctl->id.name,
						    list->kctl->id.index);

				cval = mixer_elem_list_to_info(list);
				if (!cval)
					continue;

				AUD_LOG_INFO(AUD_COMM, "  Info:id=%i,ctrl=%i,cmask=0x%x,chs=%i",
					cval->head.id, cval->control, cval->cmask, cval->channels);
				AUD_LOG_INFO(AUD_COMM, "  Vol:min=%i,max=%i,dBmin=%i,dBmax=%i,res=%d",
					cval->min, cval->max, cval->dBmin, cval->dBmax, cval->res);

				auDev->items_num += cnt;
				if (auDev->items_num >= MAX_ELEMS_NUM)
					break;

				item_idx = auDev->items_num - 1;
				if (strstr(list->kctl->id.name, "Capture")) {
					if (cval->control == UAC_FU_MUTE) {
						auDev->item_param[item_idx].ctrl_type = CAPTURE_SWITCH;
						cnt = audio_usb_ctrl_put(chip, cval, &auDev->item_param[item_idx], unmute);
					} else {
						volume = (cval->max - cval->min) / cval->res;
						auDev->item_param[item_idx].ctrl_type = CAPTURE_VOLUME;
						cnt = audio_usb_ctrl_put(chip, cval, &auDev->item_param[item_idx], volume);
					}
				} else {
					if (cval->control == UAC_FU_MUTE) {
						auDev->item_param[item_idx].ctrl_type = PLAYBACK_SWITCH;
						cnt = audio_usb_ctrl_put(chip, cval, &auDev->item_param[item_idx], unmute);
					} else {
						volume = (cval->max - cval->min) / cval->res;
						auDev->item_param[item_idx].ctrl_type = PLAYBACK_VOLUME;
						cnt = audio_usb_ctrl_put(chip, cval, &auDev->item_param[item_idx], volume);
					}
				}
			}
		}
	}

	AUD_LOG_INFO(AUD_COMM, "Total mixer items number(%d)", auDev->items_num);
}

static void audio_usb_connect(struct snd_usb_audio *chip)
{
	struct usb_interface *intf;
	struct usb_host_interface *alts;
	struct snd_usb_stream *as;
	struct snd_usb_substream *subs;
	struct audioformat *fp;
	int card_num = 0;
	int num_fmt = 0;

	struct acore_client *ac = NULL;

	if (!chip || !chip->card) {
		AUD_LOG_ERR(AUD_COMM, "chip is null");
		return;
	}

	if ((usb_cb != NULL) && (chip->dev != NULL)) {
		if (usb_cb((void *)chip->dev) == true) {
			AUD_LOG_INFO(AUD_COMM, "offload mode");
			atomic_set(&usb_mode, 0);
		} else {
			AUD_LOG_INFO(AUD_COMM, "no-offload mode");
			atomic_set(&usb_mode, 1);
			return;
		}
	}

	AUD_LOG_INFO(AUD_COMM, "set audio usbconnect. intf num=%d", chip->num_interfaces);

	card_num = chip->card->number;
	if (card_num >= SNDRV_CARDS) {
		AUD_LOG_ERR(AUD_COMM, "Invalid card number(%d)", card_num);
		return;
	}

	/* get usb interface */
	intf = chip->intf[chip->num_interfaces - 1];
	if (intf == NULL) {
		AUD_LOG_ERR(AUD_COMM, "intf is null");
		return;
	}

	AUD_LOG_INFO(AUD_COMM, "intf: %s. card_number:%d",
		dev_name(&intf->dev), chip->card->number);

	/* update audio usb device info */
	audev[card_num].udev = interface_to_usbdev(intf);
	audev[card_num].chip = chip;
	audev[card_num].card_num = chip->card->number;

	alts = &intf->altsetting[0];
	audev[card_num].intf0_num =
		get_iface_desc(alts)->bInterfaceNumber;
	audev[card_num].alt0_setting =
		get_iface_desc(alts)->bAlternateSetting;
	audev[card_num].intf = intf;

	AUD_LOG_INFO(AUD_COMM, "intf0_num: %d. alt0_setting:%d",
		audev[card_num].intf0_num,
		audev[card_num].alt0_setting);

	list_for_each_entry(as, &chip->pcm_list, list) {
		AUD_LOG_INFO(AUD_COMM, "fmt list: type %d", as->fmt_type);

		subs = &as->substream[0];
		AUD_LOG_INFO(AUD_COMM, "as0: index(%d) dir(%d) ep(%d).",
			as->pcm_index, subs->direction, subs->endpoint);
		if (subs->cur_audiofmt)
			AUD_LOG_INFO(AUD_COMM, "subs0 fmt: channels(%d) type(%d) iface(%d) altsetting(%d).",
				subs->cur_audiofmt->channels,
				subs->cur_audiofmt->fmt_type,
				subs->cur_audiofmt->iface,
				subs->cur_audiofmt->altsetting);

		if (subs->sync_endpoint)
			AUD_LOG_INFO(AUD_COMM, "subs0 sync ep pipe=0x%x", subs->sync_endpoint->pipe);

		AUD_LOG_INFO(AUD_COMM, "subs0 format num=%d", subs->num_formats);
		num_fmt = subs->num_formats;

		list_for_each_entry(fp, &subs->fmt_list, list) {
			AUD_LOG_INFO(AUD_COMM, "fp num index:%d", num_fmt);

			if ((num_fmt > 0) && fp) {
				num_fmt--;
				AUD_LOG_INFO(AUD_COMM, "out fp:%p", fp);

				AUD_LOG_INFO(AUD_COMM,
					"subs[0]: endpoint(%d) iface(%d) altsetting(%d)",
					fp->endpoint, fp->iface, fp->altsetting);
				AUD_LOG_INFO(AUD_COMM,
					"subs[0]: rate max(%d) min(%d)",
					fp->rate_max, fp->rate_min);
				AUD_LOG_INFO(AUD_COMM,
					"subs[0]: fmt_type(%d) fmt_bits(%d) formats(%llx) channels(%d)",
					fp->fmt_type, fp->fmt_bits, fp->formats, fp->channels);
				audev[card_num].jack_type |= SND_JACK_HEADPHONE;
			} else
				break;
		}

		subs = &as->substream[1];
		AUD_LOG_INFO(AUD_COMM, "as1: index(%d) dir(%d) ep(%d).",
			as->pcm_index, subs->direction, subs->endpoint);
		if (subs->cur_audiofmt)
			AUD_LOG_INFO(AUD_COMM, "subs1 fmt: channels(%d) type(%d) iface(%d) altsetting(%d).",
				subs->cur_audiofmt->channels,
				subs->cur_audiofmt->fmt_type,
				subs->cur_audiofmt->iface,
				subs->cur_audiofmt->altsetting);

		if (subs->sync_endpoint)
			AUD_LOG_INFO(AUD_COMM, "subs0 sync ep pipe=0x%x", subs->sync_endpoint->pipe);

		AUD_LOG_INFO(AUD_COMM, "subs1 format num=%d", subs->num_formats);
		num_fmt = subs->num_formats;

		list_for_each_entry(fp, &subs->fmt_list, list) {
			AUD_LOG_INFO(AUD_COMM, "fp num index:%d", num_fmt);

			if ((num_fmt > 0) && fp) {
				num_fmt--;
				AUD_LOG_INFO(AUD_COMM, "in fp:%p", fp);
				AUD_LOG_INFO(AUD_COMM,
					"subs[1]: endpoint(%d) iface(%d) altsetting(%d)",
					fp->endpoint, fp->iface, fp->altsetting);
				AUD_LOG_INFO(AUD_COMM,
					"subs[1]: rate max(%d) min(%d)",
					fp->rate_max, fp->rate_min);
				AUD_LOG_INFO(AUD_COMM,
					"subs[1]: fmt_type(%d) fmt_bits(%d) formats(%llx) channels(%d)",
					fp->fmt_type, fp->fmt_bits, fp->formats, fp->channels);
				audev[card_num].jack_type |= SND_JACK_MICROPHONE;
			} else
				break;
		}
	}

	AUD_LOG_INFO(AUD_COMM, "prepare jack report.");

#ifdef AUDIO_PLUGIN_REPORT
	if (!audev[card_num].jack_init) {
		int ret = 0;

		AUD_LOG_INFO(AUD_COMM, "create audio usb input device.");

		/* alloc audio usb input event */
		audev[card_num].input = devm_input_allocate_device(&audev[card_num].pdev->dev);
		if (!audev[card_num].input) {
			AUD_LOG_ERR(AUD_COMM, "new card(%d) input not create.", card_num);
			return;
		}

		audev[card_num].input->name = "Audio-USB-Jack";
		audev[card_num].input->phys = "ALSA";
		audev[card_num].input->dev.parent = &audev[card_num].pdev->dev;

		input_set_capability(audev[card_num].input,
			EV_SW, SW_HEADPHONE_INSERT);

		input_set_capability(audev[card_num].input,
			EV_SW, SW_MICROPHONE_INSERT);

		ret = input_register_device(audev[card_num].input);
		if (ret) {
			AUD_LOG_ERR(AUD_COMM, "register input fail.");
			return;
		}

		audev[card_num].jack_init = 1;
	}

	if (audev[card_num].input) {
		int report_update = 0;

		if (audev[card_num].jack_type & SND_JACK_HEADPHONE) {
			input_report_switch(audev[card_num].input, SW_HEADPHONE_INSERT, true);
			report_update = 1;
			AUD_LOG_INFO(AUD_COMM, "SND_JACK_HEADPHONE in report.");

			if (audev[card_num].jack_type & SND_JACK_MICROPHONE) {
				input_report_switch(audev[card_num].input, SW_MICROPHONE_INSERT, true);
				report_update = 1;
				AUD_LOG_INFO(AUD_COMM, "SND_JACK_MICROPHONE in report.");
			}
		}

		if (report_update) {
			AUD_LOG_DBG(AUD_COMM, "jack plug-in.");

			ac = (struct acore_client *)fk_acore_get_acore_client();
			if (!ac) {
				AUD_LOG_INFO(AUD_CORE, "audio client is NULL");
				return;
			}
			ac->usb_conn = 1;

			if (fk_adsp_vote_register(AUDIO_USB_CONN_SESS) < 0)
				AUD_LOG_INFO(AUD_COMM, "register usb connect power up fail.");
			else {
				input_sync(audev[card_num].input);

				atomic_set(&audev[card_num].connected, 1);

				audio_usb_mixer_info(chip);

				audio_usb_status_notify(chip, ADSP_SET_USB_CONNECT);
			}
		}
	}
#else
		AUD_LOG_DBG(AUD_COMM, "jack plug-in.");

		ac = (struct acore_client *)fk_acore_get_acore_client();
		if (!ac) {
			AUD_LOG_INFO(AUD_CORE, "audio client is NULL");
			return;
		}
		ac->usb_conn = 1;

		if (fk_adsp_vote_register(AUDIO_USB_CONN_SESS) < 0)
			AUD_LOG_INFO(AUD_COMM, "register usb connect power up fail.");
		else {
			atomic_set(&audev[card_num].connected, 1);

			audio_usb_mixer_info(chip);

			audio_usb_status_notify(chip, ADSP_SET_USB_CONNECT);
		}
#endif

	AUD_LOG_INFO(AUD_COMM, "completed.");
}

static void audio_usb_disconnect(struct snd_usb_audio *chip)
{
	int card_num;
	struct usb_interface *intf;

	struct acore_client *ac = NULL;

	if (!chip || !chip->card) {
		AUD_LOG_ERR(AUD_COMM, "chip null");
		return;
	}

	if (!atomic_read(&usb_mode))
		AUD_LOG_INFO(AUD_COMM, "offload mode.");
	else {
		AUD_LOG_INFO(AUD_COMM, "no-offload mode.");
		atomic_set(&usb_mode, 0);
		return;
	}

	card_num = chip->card->number;
	if (card_num >= SNDRV_CARDS) {
		AUD_LOG_ERR(AUD_COMM, "invalid card number(%d)", card_num);
		return;
	}

	AUD_LOG_INFO(AUD_COMM, "set audio usb card disconnect. number:%d.",
		chip->num_interfaces);

	/* get usb interface */
	intf = chip->intf[chip->num_interfaces - 1];
	if (intf == NULL) {
		AUD_LOG_ERR(AUD_COMM, "intf is null");
		return;
	}

	AUD_LOG_INFO(AUD_COMM, "intf: %s: card: %d",
		dev_name(&intf->dev), card_num);

	if (atomic_read(&audev[card_num].connected) == 0)
		return;

	atomic_set(&audev[card_num].connected, 0);

#ifdef AUDIO_PLUGIN_REPORT
	if (audev[card_num].input) {
		int report_update = 0;

		if (audev[card_num].jack_type & SND_JACK_HEADPHONE) {
			input_report_switch(audev[card_num].input, SW_HEADPHONE_INSERT, false);
			report_update = 1;
			AUD_LOG_INFO(AUD_COMM, "SND_JACK_HEADPHONE out report.");
		}

		if (audev[card_num].jack_type & SND_JACK_MICROPHONE) {
			input_report_switch(audev[card_num].input, SW_MICROPHONE_INSERT, false);
			report_update = 1;
			AUD_LOG_INFO(AUD_COMM, "SND_JACK_MICROPHONE out report.");
		}

		if (report_update)
			input_sync(audev[card_num].input);
	}
#endif

	/* trigger audio usb disconnect */
	audio_usb_status_notify(chip, ADSP_SET_USB_DISCONNECT);
	audev[card_num].items_num = 0;
	atomic_set(&audev[card_num].connected, 0);
	audev[card_num].jack_type = 0;

	ac = (struct acore_client *)fk_acore_get_acore_client();
	if (!ac) {
		AUD_LOG_INFO(AUD_CORE, "audio client is NULL");
		return;
	}

	ac->usb_conn = 0;
	if (fk_adsp_vote_unregister(AUDIO_USB_CONN_SESS) < 0)
		AUD_LOG_DBG(AUD_COMM, "register usb disconnect power down.");

	AUD_LOG_INFO(AUD_COMM, "completed.");
}

static struct snd_usb_platform_ops usb_plat_ops = {
	.connect_cb = audio_usb_connect,
	.disconnect_cb = audio_usb_disconnect,
};

int fk_dai_audio_usb_dai_probe(struct snd_soc_dai *dai)
{
	int i;

	AUD_LOG_INFO(AUD_COMM, "enter");

	for (i = 0; i < SNDRV_CARDS; i++)
		audev[i].dai = dai;

	return 0;
}

static int audio_usb_dev_probe(struct platform_device *pdev)
{
	int ret = 0;
	int i;

	/* init usb to audio callback */
	usb_cb = NULL;
	usb_priv = NULL;

	ret = snd_usb_register_platform_ops(&usb_plat_ops);
	if (ret < 0)
		AUD_LOG_ERR(AUD_COMM,
			"register audio usb OP failure. ret = %d", ret);

	for (i = 0; i < SNDRV_CARDS; i++) {
		audev[i].pdev = pdev;
		audev[i].dai = NULL;
		audev[i].jack_init = 0;
		audev[i].jack_type = 0;
		audev[i].input = NULL;
	}

	/* init wait condition */
	init_waitqueue_head(&ausb_wait_que);
	atomic_set(&ausb_wait, 0);

	/* init usb no-offload mode */
	atomic_set(&usb_mode, 1);

	return ret;
}

static int audio_usb_dev_remove(struct platform_device *pdev)
{
	int ret = 0;

	return ret;
}

static const struct of_device_id fk_audio_usb_dt_match[] = {
	{ .compatible = "xring,fk-audio-usb", },
	{},
};

static struct platform_driver fk_audio_usb_driver = {
	.driver = {
		.name = "fk-audio-usb",
		.owner = THIS_MODULE,
		.of_match_table = fk_audio_usb_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = audio_usb_dev_probe,
	.remove = audio_usb_dev_remove,
};

int fk_audio_usb_init(void)
{
	return platform_driver_register(&fk_audio_usb_driver);
}

void fk_audio_usb_exit(void)
{
	platform_driver_unregister(&fk_audio_usb_driver);
}

MODULE_DESCRIPTION("Generic XR AUDIO USB driver");
MODULE_LICENSE("Dual BSD/GPL");


