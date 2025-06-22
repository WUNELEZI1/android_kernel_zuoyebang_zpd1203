// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include "mdr_print.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include "mdr.h"

static void *g_mem_addr;
static void *g_dfx_reservedmem_addr;

struct exp_sub {
	unsigned int exception;
	unsigned char category_name[MDR_REBOOTREASONWORD_MAXLEN];
	unsigned char subtype_name[MDR_REBOOTREASONWORD_MAXLEN];
	unsigned int subtype_num;
};

#undef __REBOOT_REASON_MAP
#define __REBOOT_REASON_MAP(x, y) { #x, x, #y, y },

struct reboot_reason_word reboot_reason_map[] = {
	#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_reboot_reason_map.h"
};

#undef __REBOOT_REASON_MAP

#undef __MDR_MODID_MAP
#define __MDR_MODID_MAP(x) { #x, x },

struct modid_word modid_map[] = {
	#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_modid_map.h"
};

#undef __MDR_MODID_MAP

#undef __AP_PANIC_SUBTYPE_MAP
#define __AP_PANIC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __AP_PMU_SUBTYPE_MAP
#define __AP_PMU_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __AP_WDT_SUBTYPE_MAP
#define __AP_WDT_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __GPU_EXC_SUBTYPE_MAP
#define __GPU_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __SHUB_EXC_SUBTYPE_MAP
#define __SHUB_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __ISP_EXC_SUBTYPE_MAP
#define __ISP_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __NPU_EXC_SUBTYPE_MAP
#define __NPU_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __VDSP_EXC_SUBTYPE_MAP
#define __VDSP_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __DPU_EXC_SUBTYPE_MAP
#define __DPU_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __VPU_EXC_SUBTYPE_MAP
#define __VPU_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __LPCORE_EXC_SUBTYPE_MAP
#define __LPCORE_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __XRSE_EXC_SUBTYPE_MAP
#define __XRSE_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __XCTRL_CPU_EXC_SUBTYPE_MAP
#define __XCTRL_CPU_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __XCTRL_DDR_EXC_SUBTYPE_MAP
#define __XCTRL_DDR_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __WCN_EXC_SUBTYPE_MAP
#define __WCN_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __TEEOS_EXC_SUBTYPE_MAP
#define __TEEOS_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

#undef __ASP_EXC_SUBTYPE_MAP
#define __ASP_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z},

struct exp_sub exp_subtype_map[] = {
	#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_reboot_subtype_map.h"
};

#undef __AP_PANIC_SUBTYPE_MAP
#undef __AP_PMU_SUBTYPE_MAP
#undef __AP_WDT_SUBTYPE_MAP
#undef __GPU_EXC_SUBTYPE_MAP
#undef __SHUB_EXC_SUBTYPE_MAP
#undef __ISP_EXC_SUBTYPE_MAP
#undef __VDSP_EXC_SUBTYPE_MAP
#undef __DPU_EXC_SUBTYPE_MAP
#undef __VPU_EXC_SUBTYPE_MAP
#undef __LPCORE_EXC_SUBTYPE_MAP
#undef __XRSE_EXC_SUBTYPE_MAP
#undef __XCTRL_CPU_EXC_SUBTYPE_MAP
#undef __XCTRL_DDR_EXC_SUBTYPE_MAP
#undef __WCN_EXC_SUBTYPE_MAP
#undef __TEEOS_EXC_SUBTYPE_MAP
#undef __ASP_EXC_SUBTYPE_MAP

struct core_inf {
	unsigned int coreid;
	char *name;
};

static struct core_inf core_inf_list[] = {
	{MDR_AP, "AP"},
	{MDR_CP, "CP"},
	{MDR_TEEOS, "TEEOS"},
	{MDR_AUDIO, "AUDIO"},
	{MDR_LPM3, "LPM3"},
	{MDR_SHUB, "SENSORHUB"},
	{MDR_ISP, "ISP"},
	{MDR_IVP, "IVP"},
	{MDR_UFS, "UFS"},
	{MDR_CLK, "CLK"},
	{MDR_MISEE, "MISEE"},
	{MDR_NPU, "NPU"},
	{MDR_XRSE, "XRSE"},
	{MDR_VDSP, "VDSP"},
	{MDR_GPU, "GPU"},
	{MDR_VENC, "VENC"},
	{MDR_VDEC, "VDEC"},
	{MDR_WCN, "WCN"},
	{MDR_DPU, "DPU"}
};

char *get_core_name(unsigned int coreid)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(core_inf_list); i++) {
		if (core_inf_list[i].coreid == coreid)
			return core_inf_list[i].name;
	}

	return "NA";
}

char *get_reboot_reason_name(unsigned int exec)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(reboot_reason_map); i++) {
		if (reboot_reason_map[i].num == exec)
			return (char *)reboot_reason_map[i].name;
	}

	return "NA";
}

char *get_reboot_subtype_name(unsigned int exec, unsigned int subtype)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(exp_subtype_map); i++) {
		if ((exp_subtype_map[i].exception == exec)
			&& (exp_subtype_map[i].subtype_num == subtype))
			return (char *)exp_subtype_map[i].subtype_name;
	}

	return "NA";
}

struct reboot_reason_word *get_reboot_reason_map(void)
{
	return reboot_reason_map;
}
EXPORT_SYMBOL(get_reboot_reason_map);

u32 get_reboot_reason_map_size(void)
{
	return (u32)ARRAY_SIZE(reboot_reason_map);
}
EXPORT_SYMBOL(get_reboot_reason_map_size);

u32 get_exception_subtype_map_size(void)
{
	return ARRAY_SIZE(exp_subtype_map);
}

const struct modid_word *get_modid_map(void)
{
	return modid_map;
}

u32 get_modid_map_size(void)
{
	return ARRAY_SIZE(modid_map);
}

static u32 get_category_value(u32 e_exce_type)
{
	u32 i;
	u32 category = 0;
	const struct reboot_reason_word *reboot_reason_map = get_reboot_reason_map();

	if (!reboot_reason_map) {
		pr_err("reboot_reason_map is NULL\n");
		return 0;
	}

	for (i = 0; i < get_reboot_reason_map_size(); i++) {
		if (reboot_reason_map[i].num == e_exce_type) {
			category = reboot_reason_map[i].category_num;
			break;
		}
	}
	return category;
}

char *mdr_get_category_name(u32 e_exce_type, u32 subtype)
{
	int i, category;
	const struct reboot_reason_word *reboot_reason_map = get_reboot_reason_map();

	if (reboot_reason_map == NULL) {
		pr_err("reboot_reason_map is NULL\n");
		return NULL;
	}

	category = get_category_value(e_exce_type);
	if (category == SUBTYPE) {
		for (i = 0; (unsigned int)i < get_exception_subtype_map_size(); i++) {
			if (exp_subtype_map[i].exception == e_exce_type &&
					exp_subtype_map[i].subtype_num == subtype) {
				return (char *)exp_subtype_map[i].category_name;
			}
		}
	} else {
		for (i = 0; (unsigned int)i < get_reboot_reason_map_size(); i++) {
			if (reboot_reason_map[i].num == e_exce_type)
				return (char *)reboot_reason_map[i].category_name;
		}
	}

	return MDR_CATEGORY_TYPE;
}

void *get_mdrmem_addr(void)
{
	return g_mem_addr;
}

u64 get_dfx_reservedmem_addr(void)
{
	return (u64)g_dfx_reservedmem_addr;
}

int mdr_mem_init(void)
{
	void __iomem *mem_addr = NULL;
	struct device_node *np = NULL;
	struct device_node *node = NULL;
	struct resource res;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "xring,memory_dump");
	if (!np) {
		pr_err("There is no device!\n");
		return -EINVAL;
	}

	node = of_parse_phandle(np, "memory-region", 0);
	if (!node) {
		pr_err("No %s specified!\n", "memory-region");
		return -EINVAL;
	}

	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		pr_err("Failed to get resource from memdump_reserve\n");
		return ret;
	}

	mem_addr = ioremap(res.start, MEMDUMP_SIZE);
	if (!mem_addr) {
		pr_err("Unable to map I/O memory!\n");
		return -EINVAL;
	}

	g_dfx_reservedmem_addr = mem_addr;
	g_mem_addr = mem_addr + MDRLOG_OFFSET;

	return 0;
}
