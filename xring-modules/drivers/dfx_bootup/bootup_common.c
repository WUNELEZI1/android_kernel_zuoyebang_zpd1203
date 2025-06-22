// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)	"[xr_dfx][bootup]:%s:%d " fmt, __func__, __LINE__

#include <linux/string.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <soc/xring/dfx_bootup.h>
#include <soc/xring/xr-pmic-spmi.h>

#define FPGA 1
#define PMIC_DFX_BOOTUP_READ_ERR	0xFF

static void *g_bootup_keypoint_addr;
static int g_bootup_fpga_flag;

struct bootup_inf {
	u32 bootupid;
	char *name;
};

static struct bootup_inf boot_up_list[] = {
	{STAGE_KERNEL_STAGE_ONE_START, "kernel stage1 init start"},
	{STAGE_KERNEL_WDT_INIT_OK, "kernel wdt init ok"},
	{STAGE_KERNEL_STAGE_TWO_START, "kernel stage2 init start"},
	{STAGE_ANDROID_ZYGOTE_START, "android zygote start"},
	{STAGE_ANDROID_BOOT_SUCCESS, "android launcher start"}
};

static char *get_bootup_stage_name(u32 bootupid)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(boot_up_list); i++) {
		if (boot_up_list[i].bootupid == bootupid)
			return boot_up_list[i].name;
	}

	return "NA";
}

/*
 * set bootup_keypoint, record last_bootup_keypoint,
 */
int set_bootup_status(u32 value)
{
	int ret;
	u32 out;

	if (value < STAGE_KERNEL_STAGE_ONE_START || value > STAGE_ANDROID_BOOT_SUCCESS) {
		pr_err("[%u] parameter is invalid\n", value);
		return -EINVAL;
	}

	if (g_bootup_fpga_flag == FPGA) {
		if (!g_bootup_keypoint_addr) {
			pr_err("bootup addr fail to init\n");
			return -1;
		}
		writel(value, g_bootup_keypoint_addr);
		out = readl(g_bootup_keypoint_addr);
	} else {
		ret = xr_pmic_reg_write(PMIC_BOOTUP_KEYPOINT_ADDR, (int)value);
		if (ret) {
			pr_err("[%u] error: %d\n", value, ret);
			return ret;
		}
		ret = xr_pmic_reg_read(PMIC_BOOTUP_KEYPOINT_ADDR, &out);
		if (ret) {
			pr_err("read back error: %d\n", ret);
			return ret;
		}
	}
	pr_info("%s, bootup[%u]\n", get_bootup_stage_name(out), out);
	return 0;
}
EXPORT_SYMBOL(set_bootup_status);

u32 get_bootup_status(void)
{
	u32 value = 0;
	int ret;

	if (g_bootup_fpga_flag == FPGA) {
		if (!g_bootup_keypoint_addr)
			return -1;
		value = readl(g_bootup_keypoint_addr);
	} else {
		ret = xr_pmic_reg_read(PMIC_BOOTUP_KEYPOINT_ADDR, &value);
		if (ret) {
			pr_err(" error: %d\n", ret);
			return PMIC_DFX_BOOTUP_READ_ERR;
		}
	}
	pr_info("value is %u\n", value);
	return value;
}
EXPORT_SYMBOL(get_bootup_status);

int bootup_keypoint_addr_init(void)
{
	int ret;
	struct device_node *np = NULL;
	u32 bootup_keypoint_addr;

	np = of_find_compatible_node(NULL, NULL, "xring,mdr");
	if (np == NULL) {
		pr_err("NOT FOUND device node 'xring,mdr'!\n");
		return -ENXIO;
	}
	ret = of_property_read_u32(np, "fpga_flag", &g_bootup_fpga_flag);
	if (ret) {
		pr_err("failed to get fpga_flag resource\n");
		return ret;
	}

	if (g_bootup_fpga_flag == FPGA) {
		ret = of_property_read_u32(np, "bootup_keypoint_addr", &bootup_keypoint_addr);
		if (ret) {
			pr_err("failed to get bootup_keypoint_addr resource\n");
			return ret;
		}
		g_bootup_keypoint_addr = memremap(bootup_keypoint_addr, sizeof(int), MEMREMAP_WB);
		if (!g_bootup_keypoint_addr) {
			pr_err("get bootup_keypoint_addr error\n");
			return -1;
		}
	}

	return 0;
}
EXPORT_SYMBOL(bootup_keypoint_addr_init);
