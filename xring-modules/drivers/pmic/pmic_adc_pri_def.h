/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __PMIC_ADC_PRI_DEF_H__
#define __PMIC_ADC_PRI_DEF_H__

#include <linux/iio/iio.h>
#include <dt-bindings/xring/platform-specific/pmic/adc_resource.h>

#define ADC_MAX_CHANNEL 0xc
#define COUL_BUSY_DELAY_NUM 3
#define ADC_DELAY_MS 5

struct xr_adc_channels {
	const char *datasheet_name;
	enum iio_chan_type type;
	long info_mask;
};

#define XR_ADC_CHAN(_dname, _type, _mask)			\
	{								\
		.datasheet_name = _dname,				\
		.type = _type,						\
		.info_mask = _mask,					\
	},								\

#define XR_ADC_CHAN_RAW(_dname)				\
	XR_ADC_CHAN(_dname, IIO_INDEX,			\
		BIT(IIO_CHAN_INFO_RAW))				\

static const struct xr_adc_channels adc_chans_pmic[ADC_MAX_CHANNEL] = {
	[ADC_CH_0]		= XR_ADC_CHAN_RAW("channel_0")
	[ADC_CH_1]		= XR_ADC_CHAN_RAW("channel_1")
	[ADC_CH_2]		= XR_ADC_CHAN_RAW("channel_2")
	[ADC_CH_3]		= XR_ADC_CHAN_RAW("channel_3")
	[ADC_CH_4]		= XR_ADC_CHAN_RAW("channel_4")
	[ADC_CH_5]		= XR_ADC_CHAN_RAW("channel_5")
	[ADC_CH_6]		= XR_ADC_CHAN_RAW("channel_6")
	[ADC_CH_7]		= XR_ADC_CHAN_RAW("channel_7")
	[ADC_CH_8]		= XR_ADC_CHAN_RAW("channel_8")
	[ADC_CH_9]		= XR_ADC_CHAN_RAW("channel_9")
	[ADC_CH_A]		= XR_ADC_CHAN_RAW("channel_a")
	[ADC_CH_B]		= XR_ADC_CHAN_RAW("channel_b")
};

#endif /* __PMIC_ADC_PRI_DEF_H__ */
