// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#include <linux/of.h>
#include "xr_coul_dts.h"
#include "xr_coul_nv.h"

#define ZERO_V_MAX      3200
#define ZERO_V_MIN      2800

#define LOW_INT_VOL_COUNT       3
#define DEFAULT_SOC_AT_TERM     100
#define DEFAULT_DISCHG_OCV_SOC  5

static void get_multi_ocv_open_flag(struct coul_core_device *chip,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "multi_ocv_open",
		&chip->k2u_data.multi_ocv_enabled);
	if (ret)
		chip->k2u_data.multi_ocv_enabled = 0;
	coul_debug("multi_ocv_open: flag is %d\n", chip->k2u_data.multi_ocv_enabled);
}

static void get_poweroff_vol_mv(struct coul_core_device *chip,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "v_poweroff", &chip->v_poweroff);
	if (ret)
		chip->v_poweroff = BATTERY_NORMAL_POWEROFF_VOL;

	ret = of_property_read_u32(np, "sleep_poweroff_vol_mv",
		&chip->v_poweroff_sleep);
	if (ret)
		chip->v_poweroff_sleep = BATTERY_VOL_2_PERCENT;

	ret = of_property_read_u32(np, "v_poweroff_low_temp",
		&chip->v_poweroff_low_temp);
	if (ret)
		chip->v_poweroff_low_temp = BATTERY_NORMAL_POWEROFF_VOL;

	coul_debug("poweroff = %u, poweroff_sleep = %u, low_temp_vol = %d\n",
		chip->v_poweroff, chip->v_poweroff_sleep, chip->v_poweroff_low_temp);
}

static void get_uuc_vol_dts(struct coul_core_device *chip,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "uuc_zero_vol", &chip->k2u_data.uuc_zero_vol);
	if (ret)
		chip->k2u_data.uuc_zero_vol = ZERO_V_MAX;

	if (chip->k2u_data.uuc_zero_vol < ZERO_V_MIN)
		chip->k2u_data.uuc_zero_vol = ZERO_V_MIN;

	coul_info("uuc_zero_vol:%d\n", chip->k2u_data.uuc_zero_vol);
}

static void get_psy_dts_info(struct coul_core_device *chip)
{
	int ret;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "xring,coul_psy");
	if (!np) {
		chip->batt_board_id = BAT_BOARD_FPGA;
		return;
	}
	ret = of_property_read_u32(np, "battery_board_type", &chip->batt_board_id);
	if (ret) {
		chip->batt_board_id = BAT_BOARD_PRODUCT;
		coul_err("dts:get board type fail\n");
	}

	coul_err("dts:get board type is %u\n", chip->batt_board_id);
}

static void get_r_pcb_dts(struct coul_core_device *chip,
	const struct device_node *np)
{
	int ret;
	unsigned int pcb_mohm = DEFAULT_RPCB;

	ret = of_property_read_u32(np, "pcb_mohm", &pcb_mohm);
	if (ret)
		coul_err("error:get pcb_mohm value failed\n");
	chip->k2u_data.r_pcb = (int)pcb_mohm;
	coul_info("dts:get pcb_mohm = %u\n", pcb_mohm);
}

static void get_vth_soc_dts_info(struct coul_core_device *chip,
	const struct device_node *np)
{
	int i;
	unsigned int array_size = sizeof(chip->vth_soc_data);
	unsigned int int_size = sizeof(int);

	if (of_property_read_u32(np,
		"vth_soc_en", (u32 *)&chip->vth_soc_en)) {
		chip->vth_soc_en = 0;
		coul_err("get vth_soc_en value failed, used default value!\n");
		return;
	}

	if (of_property_read_u32_array(np, "vth_soc_para",
			(u32 *)&chip->vth_soc_data[0], array_size / int_size)) {
		chip->vth_soc_en = 0;
		coul_err("get vth_soc_para fail!!\n");
		return;
	}

	for (i = 0; i < VTH_SOC_LEVEL; i++)
		coul_debug("chip->vth_soc_data[%d]:%d,%d\n", i,
			chip->vth_soc_data[i].soc,
			chip->vth_soc_data[i].vol);
}

static void get_soc_dts_info(struct coul_core_device *chip,
	const struct device_node *np)
{
	int ret;
	unsigned int shutdown_soc_en = 0;
	unsigned int delta_soc = 0;
	unsigned int soc_at_term = DEFAULT_SOC_AT_TERM;

	ret = of_property_read_u32(np, "shutdown_soc_en", &shutdown_soc_en);
	if (ret)
		coul_err("dts:can not get shutdown_soc_en, use default : %u\n",
			shutdown_soc_en);
	chip->shutdown_soc_en = (int)shutdown_soc_en;
	coul_info("dts:get shutdown_soc_en = %u\n", shutdown_soc_en);

	ret = of_property_read_u32(np, "delta_soc", &delta_soc);
	if (ret)
		coul_err("dts:can not get delta_soc,use default: %u!\n",
			delta_soc);
	chip->delta_soc = (int)delta_soc;
	coul_info("dts:get delta_soc = %u\n", delta_soc);

	ret = of_property_read_u32(np, "soc_at_term", &soc_at_term);
	if (ret)
		coul_err("dts:can not get soc_at_term,use default : %u\n",
			soc_at_term);
	chip->k2u_data.soc_at_term = (int)soc_at_term;
	coul_info("dts:get soc_at_term = %u\n", soc_at_term);
}

static void get_low_vol_filter_cnt(struct coul_core_device *chip,
	const struct device_node *np)
{
	int ret;
	unsigned int low_vbatt_filter_cnt = LOW_INT_VOL_COUNT;

	ret = of_property_read_u32(np, "low_vbatt_filter_cnt",
		&low_vbatt_filter_cnt);
	if (ret)
		coul_err("dts:get low_vbatt_filter_cnt fail, use default limit value\n");
	chip->low_vbatt_filter_cnt = low_vbatt_filter_cnt;
	coul_debug("low_vbatt_filter_cnt = %u\n", chip->low_vbatt_filter_cnt);
}

void coul_core_init_dts(struct coul_core_device *chip)
{
	struct device_node *np = NULL;

	if (!chip) {
		coul_err("chip is null\n");
		return;
	}
	if (!chip->dev) {
		coul_err("chip->dev is null\n");
		return;
	}
	np = chip->dev->of_node;
	if (!np) {
		coul_err("np is null\n");
		return;
	}

	get_multi_ocv_open_flag(chip, np);
	get_poweroff_vol_mv(chip, np);
	get_uuc_vol_dts(chip, np);
	get_psy_dts_info(chip);
	get_r_pcb_dts(chip, np);
	get_soc_dts_info(chip, np);
	get_vth_soc_dts_info(chip, np);
	get_low_vol_filter_cnt(chip, np);
}

MODULE_AUTHOR("XRing Technologies Co., Ltd");
MODULE_DESCRIPTION("xring coul dts");
MODULE_LICENSE("GPL");
