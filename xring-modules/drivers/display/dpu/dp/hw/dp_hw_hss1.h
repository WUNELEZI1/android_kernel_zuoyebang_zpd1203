/* SPDX-License-Identifier: GPL-2.0-only */
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

#ifndef _DP_HW_HSS1_H_
#define _DP_HW_HSS1_H_

#include "dp_hw_hss1_ops.h"
#include "dp_parser.h"

struct dp_hw_hss1_ops {
	/**
	 * power_on - power on hw_hss1
	 * @hw: hardware memory address
	 *
	 * turn on the clock gate of dptx and sctrl, clear reset state
	 */
	int (*power_on)(struct dpu_hw_blk *hw);

	/**
	 * power_off - power off hw_hss1
	 * @hw: hardware memory address
	 *
	 * turn off the clock gate of dptx and sctrl, enter reset state
	 */
	void (*power_off)(struct dpu_hw_blk *hw);

	/**
	 * ipi_clock_gate_on - turn on ipi clock gate
	 * @hw: hardware memory address
	 */
	int (*ipi_clock_gate_on)(struct dpu_hw_blk *hw);

	/**
	 * ipi_clock_gate_off - turn off ipi clock gate
	 * @hw: hardware memory address
	 */
	void (*ipi_clock_gate_off)(struct dpu_hw_blk *hw);

	/**
	 * set_ipi_clock_div_ratio - set ipi clock div ratio
	 * @hw: hardware memory address
	 * @div_ratio: should be in range [0x1, 0x40]
	 */
	int (*set_ipi_clock_div_ratio)(struct dpu_hw_blk *hw, u32 div_ratio);
};

/**
 * dp_hw_hss1 - handle of hss1 hardware module
 * @hw: hardware memory address
 * @ops: hss1 supported operations
 */
struct dp_hw_hss1 {
	struct dpu_hw_blk hw;

	struct dp_hw_hss1_ops *ops;
};

struct dp_hw_hss1 *dp_hw_hss1_init(void);
void dp_hw_hss1_deinit(struct dp_hw_hss1 *hw_hss1);

#endif
