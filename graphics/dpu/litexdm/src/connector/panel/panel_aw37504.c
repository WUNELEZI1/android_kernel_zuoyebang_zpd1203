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

#include "panel_aw37504.h"
#include "dpu_log.h"
#include "osal.h"

static void aw37504_i2c_write(uint8_t reg, uint8_t value)
{
	uint32_t id = I2C_BUS_3;
	uint16_t addr = 0x3e;
	uint32_t reg_len = 1;
	uint32_t data_len = 1;
	uint8_t buf_r[32] = {0};
	uint8_t buf_w[32] = {0};

	buf_w[0] = value;
	i2c_write(id, addr, reg, reg_len, buf_w,  data_len);
	dpu_pr_debug("AW37504Write 0x%x 0x%x \n", reg, value);

	buf_r[0] = 0;
	i2c_read(id, addr, reg, reg_len, buf_r,data_len);
	dpu_pr_debug("AW37504Read 0x%x 0x%x \n", reg, buf_r[0]);
}

void aw37504_config(void)
{
	dpu_pr_info("+\n");

	/* set the voltage of vsp/vsn to 6v */
	aw37504_i2c_write(AW37504_VOUTP, 0x14);
	aw37504_i2c_write(AW37504_VOUTN, 0x14);
	aw37504_i2c_write(AW37504_APPS, 0x5B);
	aw37504_i2c_write(AW37504_CTRL, 0x09);

	dpu_pr_info("-\n");
}
