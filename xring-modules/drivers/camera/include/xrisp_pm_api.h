/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRISP_PM_API_H__
#define __XRISP_PM_API_H__
#include <linux/kernel.h>

#define PM_RGLTR_NUM_MAX (5)

enum isp_rgltr_type {
	ISP_FE_CORE0 = 0,
	ISP_FE_CORE1,
	ISP_FE_CORE2,
	ISP_BE,
	ISP_PE,
	ISP_REGULATOR_MAX,
};

/* cam pinctrl */
enum cam_clk_pinctrl_state {
	CAMT_CLK_DEFAULT, //PAD_GPIO_224
	CAMT_CLK_SLEEP,
	CAMF_CLK_DEFAULT, //PAD_GPIO_226
	CAMF_CLK_SLEEP,
	PINCTRL_CLK_MAX = CAMF_CLK_SLEEP,
	I3C4_SCL_DEFAULT, //PAD_GPIO_177
	I3C4_SCL_SLEEP,
	I3C4_SDA_DEFAULT, //PAD_GPIO_178
	I3C4_SDA_SLEEP,
	I3C5_SCL_DEFAULT, //PAD_GPIO_182
	I3C5_SCL_SLEEP,
	I3C5_SDA_DEFAULT, //PAD_GPIO_183
	I3C5_SDA_SLEEP,
	I3C6_SCL_DEFAULT, //PAD_GPIO_188
	I3C6_SCL_SLEEP,
	I3C6_SDA_DEFAULT, //PAD_GPIO_189
	I3C6_SDA_SLEEP,
	I3C7_SCL_DEFAULT, //PAD_GPIO_185
	I3C7_SCL_SLEEP,
	I3C7_SDA_DEFAULT, //PAD_GPIO_186
	I3C7_SDA_SLEEP,
	I2C22_SCL_DEFAULT,
	I2C22_SCL_SLEEP,
	I2C22_SDA_DEFAULT,
	I2C22_SDA_SLEEP,
	GPIO_087_DEFAULT,
	GPIO_087_SLEEP,
	GPIO_088_DEFAULT,
	GPIO_088_SLEEP,
	GPIO_096_DEFAULT,
	GPIO_096_SLEEP,
	GPIO_132_DEFAULT,
	GPIO_132_SLEEP,
	GPIO_151_DEFAULT,
	GPIO_151_SLEEP,
	GPIO_192_DEFAULT,
	GPIO_192_SLEEP,
	GPIO_195_DEFAULT,
	GPIO_195_SLEEP,
	GPIO_207_DEFAULT,
	GPIO_207_SLEEP,
	GPIO_213_DEFAULT,
	GPIO_213_SLEEP,
	GPIO_218_DEFAULT,
	GPIO_218_SLEEP,
	PINCTRL_STATE_MAX,
};

int xrisp_be_ocm_link(void);
int xrisp_be_ocm_unlink(void);
int xrisp_be_ocm_unlink_reap(void);

int xrisp_store_pipergltr_set(u8 rgltr_set[], int rgltr_num);
int xrisp_boot_pipe_rgltr_enable(void);

int xrisp_pipe_rgltr_enable(u8 rgltr_on_set[], int rgltr_num);
int xrisp_pipe_rgltr_disable(u8 rgltr_off_set[], int rgltr_num);
void xrisp_pipe_rgltr_release(void);

int xrisp_csi_rgltr_enable(void);
int xrisp_csi_rgltr_disable(void);

int xrisp_get_rgltr_open_cnt(int type);

int xrisp_pinctrl_switch_state(u8 state);
#endif /* __XRISP_PM_API_H__ */
