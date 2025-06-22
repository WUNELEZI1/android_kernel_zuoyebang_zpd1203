/* SPDX-License-Identifier: GPL-2.0-only
 *
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

/* audio pinctrl */
enum audio_pinctrl_e {
	PDM0_FUNC,
	PDM1_FUNC,
	CDC_CLK_FUNC,
	I2S0_FUNC,
	I2S1_FUNC,
	I2S2_FUNC,
	I2S3_FUNC,
	I2S4_FUNC,
	I2S5_FUNC,
	I2S6_FUNC,
	I2S7_FUNC,
	I2S8_FUNC,
	I2S9_FUNC,
	I2S10_FUNC,
	DEVICE_INT_FUNC,
	PINCTRL_FUNC_MAX,
};

enum audio_pinctrl_s {
	PIN_INIT,
	PIN_DEFAULT,
	PIN_SLEEP,
	PIN_STATE_MAX,
};

int fk_audio_pinctrl_switch_state(u8 func_type, u8 action);
int fk_audio_port_pinctrl_func_set(int port_id, bool en);

