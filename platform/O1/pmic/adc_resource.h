// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __ADC_RESOURCE_H__
#define __ADC_RESOURCE_H__

enum adc_ctrl_cal_setup {
	ADC_NO_CAL = 0,
	ADC_EN_CAL,
};

enum adc_ctrl_mode {
	ADC_MODE_NORMAL =	0,
	ADC_MODE_CHOPPE,
};

enum adc_ch_type {
	ADC_CH_0		= 0x0,
	ADC_CH_1		= 0x1,
	ADC_CH_2		= 0x2,
	ADC_CH_3		= 0x3,
	ADC_CH_4		= 0x4,
	ADC_CH_5		= 0x5,
	ADC_CH_6		= 0x6,
	ADC_CH_7		= 0x7,
	ADC_CH_8		= 0x8,
	ADC_CH_9		= 0x9,
	ADC_CH_A		= 0xa,
	ADC_CH_B		= 0xb,
};

#endif
