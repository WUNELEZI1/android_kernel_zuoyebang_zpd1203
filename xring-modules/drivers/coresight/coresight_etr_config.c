// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/types.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <linux/arm-smccc.h>
#include <soc/xring/coresight_etr_config.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/peri_crg.h>
#include <dt-bindings/xring/platform-specific/autofs_gt/mainbus_autogt.h>

#define HIMASK_ENABLE(shift)	(0x10001 << (shift))
#define HIMASK_DISABLE(shift)	(0x10000 << (shift))

static void *g_mem_addr;

/*
 * enable/disable atb_sh clk, send smc to bl31
 */
static int atb_sh_clk_safe_gate_en_dis(unsigned int enable)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_MNTN_ATB_SH_CLK, enable, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("smc return fail.\n");

	return res.a0;
}

/*
 * enable trace clk send to sensorhub subsystem, in case clk_debug_atb_sh not enable
 * @Input: true -> enable clk_debug_atb_sh gate
 * @Input: false -> disable clk_debug_atb_sh gate
 */
int toggle_coresight_atb_sh_clk_gate(bool enable)
{
	int ret = 0;

	ret = atb_sh_clk_safe_gate_en_dis(enable);
	if (ret)
		pr_err("config atb_sh clk gate fail.\n");

	return ret;
}
EXPORT_SYMBOL(toggle_coresight_atb_sh_clk_gate);

/*
 * Bypass mainbus autogate before config ETR,
 * and enable mainbus autogate after finish using ETR
 * @Input: true -> enable mainbus autogt
 * @Input: false -> bypass mainbus autogt
 */
void set_mainbus_autogt(bool enable)
{
	u32 reg_write_value = 0;

	if (!g_mem_addr) {
		pr_err("peri_crg remap addr is null!\n");
		return;
	}
	pr_info("mainbus autogt: 0x%x", readl(g_mem_addr));

	if (enable) {
		reg_write_value = HIMASK_DISABLE(MAINBUS_AUTOGT_ETR_NOC_DFX_BIT);
	} else {
	// bypass mainbus autogt
		reg_write_value = HIMASK_ENABLE(MAINBUS_AUTOGT_ETR_NOC_DFX_BIT);
	}

	writel(reg_write_value, g_mem_addr);
	udelay(1);
	pr_info("after config, mainbus autogt: 0x%x", readl(g_mem_addr));
}
EXPORT_SYMBOL(set_mainbus_autogt);


static int __init coresight_etr_config_init(void)
{
	void *mem_addr = NULL;
	struct device_node *node = NULL;
	u64 pericrg_reg[2];
	int ret;

	node = of_find_compatible_node(NULL, NULL, "xring,pericrg_syscon");
	if (!node) {
		pr_err("NOT FOUND device node 'xring,pericrg_syscon'!\n");
		return -EINVAL;
	}

	ret = of_property_read_u64_array(node, "reg", pericrg_reg, 2);
	if (ret) {
		pr_err("Failed to get reg from pericrg_syscon\n");
		return -ENODEV;
	}

	mem_addr = ioremap(pericrg_reg[0] + PERI_CRG_AUTOFSCTRL27, sizeof(int));
	if (!mem_addr) {
		pr_err("failed to map autofsctrl27 crg memory!\n");
		return -EINVAL;
	}

	g_mem_addr = mem_addr;
	pr_info("install coresight_etr_config.ko\n");
	return 0;
}

static void __exit coresight_etr_config_exit(void)
{
	iounmap(g_mem_addr);
	pr_info("remove coresight_etr_config.ko\n");
}


MODULE_AUTHOR("Zhijie Li <lizhijie5@xiaomi.com>");
MODULE_DESCRIPTION("XRing coresight_etr_config driver");
MODULE_LICENSE("GPL v2");
module_init(coresight_etr_config_init);
module_exit(coresight_etr_config_exit);
