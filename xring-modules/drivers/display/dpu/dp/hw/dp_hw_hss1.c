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

#include <linux/slab.h>
#include "dp_hw_hss1.h"

/* HSS1_CRG reg configuration */
#define HSS1_CRG_ADDRESS                                 0xE1A83000
#define HSS1_CRG_SIZE                                    4096

#define LPIS_ACTRL_ADDRESS                               0xE1508000
#define LPIS_ACTRL_SIZE                                  4096

static struct dp_hw_hss1_ops dp_hw_hss1_ops = {
	.power_on                 = dp_hw_hss1_power_on,
	.power_off                = dp_hw_hss1_power_off,
	.ipi_clock_gate_on        = dp_hw_hss1_ipi_clock_gate_on,
	.ipi_clock_gate_off       = dp_hw_hss1_ipi_clock_gate_off,
	.set_ipi_clock_div_ratio  = dp_hw_hss1_set_ipi_clock_div_ratio,
};

struct dp_hw_hss1 *dp_hw_hss1_init(void)
{
	struct dp_hw_hss1 *hw_hss1;

	hw_hss1 = kzalloc(sizeof(*hw_hss1), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_hss1))
		return ERR_PTR(-ENOMEM);

	hw_hss1->hw.iomem_base = ioremap(HSS1_CRG_ADDRESS, HSS1_CRG_SIZE);
	hw_hss1->hw.base_addr = HSS1_CRG_ADDRESS;
	hw_hss1->hw.blk_len = HSS1_CRG_SIZE;
	hw_hss1->ops = &dp_hw_hss1_ops;

	return hw_hss1;
}

void dp_hw_hss1_deinit(struct dp_hw_hss1 *hw_hss1)
{
	iounmap(hw_hss1->hw.iomem_base);
	kfree(hw_hss1);
}
