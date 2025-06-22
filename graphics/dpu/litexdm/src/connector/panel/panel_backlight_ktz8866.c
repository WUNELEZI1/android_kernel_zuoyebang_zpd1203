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

#include <Library/BoardProjectInfo.h>
#include "panel_backlight_ktz8866.h"
#include "dpu_log.h"
#include "osal.h"

static void ktz8866_i2c_write(uint8_t reg, uint8_t value)
{
	uint32_t id = I2C_BUS_9;
	uint32_t id1 = I2C_BUS_3;
	uint32_t id2 = I2C_BUS_11;
	uint16_t addr = 0x11;
	uint32_t reg_len = 1;
	uint32_t data_len = 1;
	uint8_t buf_r[32] = {0};
	uint8_t buf_r1[32] = {0};
	uint8_t buf_w[32] = {0};
	int ret = 0;

	EFI_ProjectInfoType project = EFI_PROJECTINFO_TYPE_UNKNOWN;
	project = BoardProjectType();

	buf_w[0] = value;

	if(project == EFI_PROJECTINFO_TYPE_O81A) {
		ret = i2c_write(id1, addr, reg, reg_len, buf_w,  data_len);
		if(ret)
			dpu_pr_err("I2C_BUS_3 write reg %x value %x failed\n", reg, value);
		ret = i2c_write(id2, addr, reg, reg_len, buf_w,  data_len);
		if(ret)
			dpu_pr_err("I2C_BUS_11 write reg %x value %x failed\n", reg, value);

		buf_r[0] = 0;
		buf_r1[0] = 0;
		i2c_read(id1, addr, reg, reg_len, buf_r,data_len);
		i2c_read(id2, addr, reg, reg_len, buf_r1,data_len);
	} else {
		i2c_write(id, addr, reg, reg_len, buf_w,  data_len);

		buf_r[0] = 0;
		i2c_read(id, addr, reg, reg_len, buf_r,data_len);
	}

}

void ktz8866_set_backlight(unsigned int backlight_level)
{
	uint8_t value;

	dpu_pr_debug("+\n");

	value = (backlight_level >> 8) & 0x7;
	ktz8866_i2c_write(KTZ8866_DISP_BB_LSB, value);

	value = backlight_level & 0xFF;
	ktz8866_i2c_write(KTZ8866_DISP_BB_MSB, value);

	ktz8866_i2c_write(KTZ8866_DISP_BL_ENABLE, 0x7F);

	dpu_pr_debug("-\n");
}

void ktz8866_power_on(void)
{
	dpu_pr_debug("+\n");

	EFI_STATUS Status;
	EFI_ProjectInfoType project = EFI_PROJECTINFO_TYPE_UNKNOWN;
	Status = BoardProjectInfoInit();
	if (Status != EFI_SUCCESS)
		dpu_pr_err("BoardProjectInfoInit failed:%r\n", Status);
	project = BoardProjectType();

	if(project == EFI_PROJECTINFO_TYPE_O81A) {
		ktz8866_i2c_write(KTZ8866_DISP_BC1, 0x42);
		ktz8866_i2c_write(KTZ8866_DISP_FULL_CURRENT, 0xF9);
		ktz8866_i2c_write(KTZ8866_DISP_BC2, 0xCD);
		/* set bl as 0 */
		ktz8866_i2c_write(KTZ8866_DISP_BB_LSB, 0);
		ktz8866_i2c_write(KTZ8866_DISP_BB_MSB, 0);
		/* enable the bl device */
		ktz8866_i2c_write(KTZ8866_DISP_BL_ENABLE, 0x7F);
	} else {
		ktz8866_i2c_write(KTZ8866_DISP_BC1, 0x52);
		ktz8866_i2c_write(KTZ8866_DISP_FULL_CURRENT, 0x91);
		ktz8866_i2c_write(KTZ8866_DISP_BC2, 0xCD);
		/* set the voltage of vsp/vsn to 6v */
		ktz8866_i2c_write(KTZ8866_DISP_BIAS_VPOS, 0x28);
		ktz8866_i2c_write(KTZ8866_DISP_BIAS_VNEG, 0x28);
		ktz8866_i2c_write(KTZ8866_DISP_BIAS_CONF1, 0x9F);
		/* set bl as 0 */
		ktz8866_i2c_write(KTZ8866_DISP_BB_LSB, 0);
		ktz8866_i2c_write(KTZ8866_DISP_BB_MSB, 0);
		/* enable the bl device */
		ktz8866_i2c_write(KTZ8866_DISP_BL_ENABLE, 0x7F);
	}
	dpu_pr_debug("-\n");
}

void ktz8866_power_off(void)
{
	dpu_pr_debug("+\n");

	ktz8866_i2c_write(KTZ8866_DISP_BL_ENABLE, 0x0);

	dpu_pr_debug("-\n");
}
