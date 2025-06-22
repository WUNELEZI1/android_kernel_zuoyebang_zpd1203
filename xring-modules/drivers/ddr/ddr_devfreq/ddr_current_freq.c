// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/types.h>
#include <linux/io.h>
#include <linux/units.h>
#include <linux/ioport.h>
#include <fcm_acpu_address_map.h>
#include <ddr_pub_res_define.h>

static void __iomem *g_current_freq_reg;

unsigned long ddr_get_current_freq(void)
{
	unsigned long freq_mbps;

	freq_mbps = (unsigned long)((readl(g_current_freq_reg) &
		LPIS_ACTRL_DDR_TYPE_FREQ_REG_FREQ_MASK) >> LPIS_ACTRL_DDR_TYPE_FREQ_REG_FREQ_SHIFT);
	return freq_mbps * HZ_PER_MHZ;
}

int ddr_current_freq_remap(struct device *dev)
{
	g_current_freq_reg = devm_ioremap(dev,
		(resource_size_t)(ACPU_LPIS_ACTRL + LPIS_ACTRL_DDR_TYPE_FREQ_REG), sizeof(unsigned int));
	if (g_current_freq_reg == NULL)
		return -ENOMEM;

	return 0;
}
