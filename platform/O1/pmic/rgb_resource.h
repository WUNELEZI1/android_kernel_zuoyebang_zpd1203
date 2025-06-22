// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __RGB_RESOURCE_H__
#define __RGB_RESOURCE_H__


#define CHECK_RGB_ID(rgb_id)           ((rgb_id) < MPMIC_RGB_ID_CNT)
#define CHECK_RGB_MODE(rgb_mode)       ((rgb_mode) < RGB_MODE_CNT)
#define CHECK_RGB_CURRENT(current)     ((current) < RGB_CURRENT_CNT)
#define CHECK_RGB_PWM(pwm)             ((pwm) < RGB_PWM_FREQ_CNT)
#define CHECK_RGB_DIMMING(dimming)     ((dimming) < RGB_DIMMING_CNT)
#define CHECK_RGB_TOFF(toff)           ((toff) < RGB_TOFF_CNT)
#define CHECK_RGB_TON_TR_TF(ton_tr_tf) ((ton_tr_tf) < RGB_TON_TR_TF_CNT)


#define GET_RGB_CURRENT_ADDR(rgb_id)  (unsigned short)(TOP_REG_INTF_RGB_CFG2 + (rgb_id))
#define GET_RGB_PWM_ADDR(rgb_id)      (unsigned short)(TOP_REG_RGB1_PWM_FRE_REG + 5 * (rgb_id))
#define GET_RGB_DIMMING_ADDR(rgb_id)  (unsigned short)(TOP_REG_RGB1_DIMMING_DUTY_REG + 5 * (rgb_id))
#define GET_RGB_TON_TOFF_ADDR(rgb_id) (unsigned short)(TOP_REG_RGBL_TON_TOFF_REG + 5 * (rgb_id))
#define GET_RGB_TR1_TR2_ADDR(rgb_id)  (unsigned short)(TOP_REG_RGBL_TR1_TR2_REG + 5 * (rgb_id))
#define GET_RGB_TF1_TF2_ADDR(rgb_id)  (unsigned short)(TOP_REG_RGBL_TF1_TF2_REG + 5 * (rgb_id))

#define RGB_DIMMING_CNT               128

enum rgb_current {
	RGB_CURRENT_01_mA = 0x0,
	RGB_CURRENT_03_mA = 0x1,
	RGB_CURRENT_05_mA = 0x2,
	RGB_CURRENT_07_mA = 0x3,
	RGB_CURRENT_09_mA = 0x4,
	RGB_CURRENT_11_mA = 0x5,
	RGB_CURRENT_13_mA = 0x6,
	RGB_CURRENT_NONE  = 0x7,
	RGB_CURRENT_CNT   = 0x8,
};

enum rgb_pwm_freq {
	RGB_PWM_FREQ_125_mHz = 0x0,
	RGB_PWM_FREQ_250_mHz = 0x1,
	RGB_PWM_FREQ_500_mHz = 0x2,
	RGB_PWM_FREQ_1_Hz    = 0x3,
	RGB_PWM_FREQ_2_Hz    = 0x4,
	RGB_PWM_FREQ_4_Hz    = 0x5,
	RGB_PWM_FREQ_128_Hz  = 0x6,
	RGB_PWM_FREQ_256_Hz  = 0x7,
	RGB_PWM_FREQ_CNT     = 0x8,
};

enum rgb_toff {
	RGB_TOFF_0250_ms = 0x0,
	RGB_TOFF_0750_ms = 0x1,
	RGB_TOFF_1250_ms = 0x2,
	RGB_TOFF_1750_ms = 0x3,
	RGB_TOFF_2250_ms = 0x4,
	RGB_TOFF_2750_ms = 0x5,
	RGB_TOFF_3250_ms = 0x6,
	RGB_TOFF_3750_ms = 0x7,
	RGB_TOFF_4250_ms = 0x8,
	RGB_TOFF_4750_ms = 0x9,
	RGB_TOFF_5250_ms = 0xa,
	RGB_TOFF_5750_ms = 0xb,
	RGB_TOFF_6250_ms = 0xc,
	RGB_TOFF_6750_ms = 0xd,
	RGB_TOFF_7250_ms = 0xe,
	RGB_TOFF_7750_ms = 0xf,
	RGB_TOFF_CNT     = 0x10,
};

enum rgb_ton_tr_tf {
	RGB_TON_TR_TF_0125_ms = 0x0,
	RGB_TON_TR_TF_0375_ms = 0x1,
	RGB_TON_TR_TF_0625_ms = 0x2,
	RGB_TON_TR_TF_0875_ms = 0x3,
	RGB_TON_TR_TF_1125_ms = 0x4,
	RGB_TON_TR_TF_1375_ms = 0x5,
	RGB_TON_TR_TF_1625_ms = 0x6,
	RGB_TON_TR_TF_1875_ms = 0x7,
	RGB_TON_TR_TF_2125_ms = 0x8,
	RGB_TON_TR_TF_2375_ms = 0x9,
	RGB_TON_TR_TF_2625_ms = 0xa,
	RGB_TON_TR_TF_2875_ms = 0xb,
	RGB_TON_TR_TF_3125_ms = 0xc,
	RGB_TON_TR_TF_3375_ms = 0xd,
	RGB_TON_TR_TF_3625_ms = 0xe,
	RGB_TON_TR_TF_3875_ms = 0xf,
	RGB_TON_TR_TF_CNT     = 0x10,
};

enum rgb_mode {
	RGB_BREATH_MODE   = 0x0,
	RGB_REGISTER_MODE = 0x1,
	RGB_FLASH_MODE    = 0x2,
	RGB_MODE_CNT      = 0x3,
};

enum rgb_id {
	MPMIC_RGB1       = 0x0,
	MPMIC_RGB2       = 0x1,
	MPMIC_RGB3       = 0x2,
	MPMIC_RGB_ID_CNT = 0x3,
};

enum rgb_signal {
	RGB_DISABLE_SIGNAL = 0x0,
	RGB_ENABLE_SIGNAL  = 0x1,
};

#endif
