/* SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#ifndef _VEU_RES_H_
#define _VEU_RES_H_

#include <linux/regulator/consumer.h>
#include <linux/device.h>
#include "veu_drv.h"

struct veu_data;

#define VEU_INT 0x80
#define VEU_FUNC_IRQ_RAW 0x0
#define VEU_FUNC_IRQ_MASK 0x4
#define VEU_FUNC_IRQ_STATUS 0x8

enum VEU_REGULATOR {
	MEDIA1_SUBSYS = 0,
	DPU0,
	VEU_TOP_SUBSYS,
	VEU_REGULATOR_NUM_MAX,
};

enum VEU_CLK {
	VEU_MCLK = 0,
	VEU_ACLK,
	VEU_PCLK,
	DISP_BUS_DATA,
	DISP_BUS_CFG,
	DPU_PCLK,
	VEU_CLK_NUM_MAX,
};

/**
 * veu_clk_mgr - veu clock manager
 * @veu_clk: clocks for veu
 */
struct veu_clk_mgr {
	struct clk *veu_clk[VEU_CLK_NUM_MAX];
	struct mutex lock;
};

/**
 * veu_power_mgr - veu power manager
 * @veu_supply: regulator for veu
 * @power_count: power count for regulator polls
 */
struct veu_power_mgr {
	struct regulator *veu_supply[VEU_REGULATOR_NUM_MAX];

	atomic_t power_count;
};

/**
 * veu_clk_mgr_init - init veu clock
 * @veu_dev: pointer of veu device
 * @clk_mgr: pointer of clk mgr
 *
 * Return: zero on success, -ERRNO on failure
 */
int veu_clk_mgr_init(struct device *dev, struct veu_clk_mgr **mgr);

/**
 * veu_clk_mgr_deinit - deinit veu clock
 * @clk_mgr: pointer of clk mgr
 */
void veu_clk_mgr_deinit(struct veu_clk_mgr *clk_mgr);

/**
 * veu_power_mgr_init - init veu power mgr
 * @dev: pointer of veu of_device
 * @mgr: pointer of power mgr
 *
 * Return: zero on success, -ERRNO on failure
 */
int veu_power_mgr_init(struct device *dev, struct veu_power_mgr **mgr);

/**
 * veu_power_mgr_deinit - deinit veu power mgr
 * @power_mgr: pointer of power mgr
 */
void veu_power_mgr_deinit(struct veu_power_mgr *power_mgr);

int veu_init_isr(struct veu_data *veu_dev);
void veu_free_isr(struct veu_data *veu_dev);

void veu_callback_register(struct veu_data *veu_dev);


#endif /* _VEU_RES_H_ */
