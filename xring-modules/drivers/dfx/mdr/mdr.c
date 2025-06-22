// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */
#include "mdr_print.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <uapi/linux/sched/types.h>
#include <linux/slab.h>

#include "mdr_field_core.h"
#include "mdr_syserr_core.h"
#include "mdr.h"
#include "mdr_adapter_ap.h"
#include <soc/xring/xr-pmic-spmi.h>

int g_mdr_fpga_flag = -1;
static unsigned long long g_pmu_reset_reg;
static unsigned long long g_pmu_subtype_reg;
static struct mdr_data *g_data;

unsigned long long get_pmu_reset_reg(void)
{
	return g_pmu_reset_reg;
}

unsigned long long get_pmu_subtype_reg(void)
{
	return g_pmu_subtype_reg;
}

/*
 * If the sub exception reason already present in the register is retained, save_value is true.
 * the sub exception reason already present in the register is not retained, save_value is false.
 *
 */
void set_subtype_exception(unsigned int subtype, bool save_value)
{
	unsigned int value = 0;
	uintptr_t pmu_reset_reg;
	int ret = 0;

	if (g_mdr_fpga_flag == FPGA) {
		pmu_reset_reg = get_pmu_subtype_reg();
		if (pmu_reset_reg)
			value = readl((char *)pmu_reset_reg);
	} else {
		ret = xr_pmic_reg_read(PMIC_REBOOT_SUBREASON_ADDR, &value);
		if (ret)
			pr_err("read pmic reg failed, ret=%d\n", ret);
	}

	if (save_value == false)
		value &= (PMU_RESET_REG_MASK);

	subtype &= (RST_FLAG_MASK);
	value |= subtype;
	if (g_mdr_fpga_flag == FPGA) {
		pmu_reset_reg = get_pmu_subtype_reg();
		if (pmu_reset_reg) {
			pr_info("value = 0x%x\n", value);
			writel(value, (char *)pmu_reset_reg);
		}
	} else {
		ret = xr_pmic_reg_write(PMIC_REBOOT_SUBREASON_ADDR, value);
		if (ret)
			pr_err("write pmic reg failed,ret=%d\n", ret);
	}

	pr_info("[0x%x]\n", (u8)value);
	return;

}
EXPORT_SYMBOL(set_subtype_exception);

void set_reboot_reason(unsigned int reboot_reason)
{
	unsigned int value = 0;
	uintptr_t pmu_reset_reg;
	int ret = 0;

	reboot_reason &= (RST_FLAG_MASK);

	if (g_mdr_fpga_flag == FPGA) {
		pmu_reset_reg = get_pmu_reset_reg();
		if (pmu_reset_reg)
			writel(reboot_reason, (char *)pmu_reset_reg);
	} else {
		ret = xr_pmic_reg_read(PMIC_REBOOT_REASON_ADDR, &value);
		if (ret)
			pr_err("read pmic reg failed, ret=%d\n", ret);

		value &= (PMU_RESET_REG_MASK);
		value |= reboot_reason;
		ret = xr_pmic_reg_write(PMIC_REBOOT_REASON_ADDR, value);
		if (ret)
			pr_err("write pmic reg failed,ret=%d\n", ret);
	}

	pr_info("[0x%x]\n", (u8)reboot_reason);
}
EXPORT_SYMBOL(set_reboot_reason);

unsigned int get_reboot_reason(void)
{
	unsigned int value = 0;
	uintptr_t pmu_reset_reg;
	int ret = 0;

	if (g_mdr_fpga_flag == FPGA) {
		pmu_reset_reg = get_pmu_reset_reg();
		if (pmu_reset_reg)
			value = readl((char *)pmu_reset_reg);
	} else {
		ret = xr_pmic_reg_read(PMIC_REBOOT_REASON_ADDR, &value);
		if (ret)
			pr_err("read pmic reg failed, ret=%d\n", ret);
	}
	value &= RST_FLAG_MASK;

	pr_info("[0x%x]\n", value);
	return value;
}
EXPORT_SYMBOL(get_reboot_reason);

void record_exce_type(struct mdr_exception_info_s *e_info)
{
	if (!e_info) {
		MDR_PRINT_ERR("einfo is null\n");
		return;
	}
	set_reboot_reason(e_info->e_exce_type);
	set_subtype_exception(e_info->e_exce_subtype, false);
}

static int mdr_dt_parse(struct mdr_data *data)
{
	struct device_node *np = NULL;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "xring,mdr");
	if (!np) {
		MDR_PRINT_ERR("NOT FOUND device node 'xring,mdr'!\n");
		return -ENXIO;
	}
	ret = of_property_read_u32(np, "fpga_flag", &data->fpga_flag);
	if (ret) {
		MDR_PRINT_ERR("failed to get fpga_flag resource.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "reset_reg_addr", &data->fpga_reset_reg_addr);
	if (ret) {
		MDR_PRINT_ERR("failed to get reset_reg_addr.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "sub_reset_reg_addr", &data->fpga_sub_reset_reg_addr);
	if (ret) {
		MDR_PRINT_ERR("failed to get sub_reset_reg_addr.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "lpcore_nmi_addr", &data->lpcore_nmi_addr);
	if (ret) {
		MDR_PRINT_ERR("failed to get lpcore_nmi_addr.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "mdr_phymem_addr", &data->reserved_mdr_phymem_addr);
	if (ret) {
		MDR_PRINT_ERR("failed to get mdr_phymem_addr.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "mdr_phymem_size", &data->reserved_mdr_phymem_size);
	if (ret) {
		MDR_PRINT_ERR("failed to get mdr_phymem_size.\n");
		return ret;
	}

	return 0;
}

int get_pmu_reset_base_addr(struct mdr_data *data)
{

	if (data->fpga_flag == FPGA) {
		g_mdr_fpga_flag = FPGA;
		g_pmu_reset_reg = (unsigned long long)mdr_map(data->fpga_reset_reg_addr,
				sizeof(u32));
		if (!g_pmu_reset_reg) {
			MDR_PRINT_ERR("get pmu reset reg error\n");
			return -1;
		}

		g_pmu_subtype_reg = (unsigned long long)mdr_map(data->fpga_sub_reset_reg_addr,
				sizeof(u32));
		if (!g_pmu_subtype_reg) {
			MDR_PRINT_ERR("get pmu subtype reg error\n");
			return -1;
		}
	}
	return 0;
}

static void set_mdrdata(struct mdr_data *data)
{
	g_data = data;
}

struct mdr_data *get_mdrdata(void)
{
	return g_data;
}

int mdr_init(void)
{
	int ret = 0;
	struct mdr_data *data = kzalloc(sizeof(struct mdr_data), GFP_KERNEL);

	if (!data)
		return -ENOMEM;

	ret = mdr_dt_parse(data);
	if (ret) {
		MDR_PRINT_ERR("mdr dt parse fail.\n");
		kfree(data);
		return ret;
	}

	set_mdrdata(data);

	MDR_PRINT_START();
	mdr_syserr_init();
	mdr_field_init();

	ret = get_pmu_reset_base_addr(data);
	if (ret) {
		MDR_PRINT_ERR("get_pmu_reset_base_addr fail.\n");
		kfree(data);
		return ret;
	}

	mdr_ap_adapter_init();

	ret = mdr_netlink_init();
	if (ret)
		MDR_PRINT_ERR("mdr_netlink_init failed\n");

	ret = mdr_dump_node_init();
	if (ret)
		MDR_PRINT_ERR("mdr_dump_node_init failed\n");

	MDR_PRINT_END();
	MDR_PRINT_ERR("success\n");
	return 0;
}
