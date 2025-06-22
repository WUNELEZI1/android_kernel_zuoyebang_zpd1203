// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/kernel.h>
#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/platform-specific/sensorhub_dts.h>
#include <dt-bindings/xring/platform-specific/sensorhub_ddr_layout.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <linux/io.h>
#include "shub_private_dts.h"

#define     WLDO_28681c_SUPPLY      0
#define     WLDO_28661d_SUPPLY      1
#define     MPMIC_SUPPLY            2
#define     IO_SUPPLY               3

static int aoc_cam_mpmic_dts_parse(struct device_node *aoc_front_root,
	struct mpmic_rgltr_dev_s *mpmic_dev, struct mpmic_param_s *mpmic_param)
{
	struct device_node *child = NULL;
	const char *strings;
	uint32_t val = 0, ret = 0, count = 0;

	child = of_find_node_by_name(aoc_front_root, "aoc_mpmic");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("aoc_mpmic node not found!\n");
		return ret;
	}
	count = of_property_count_strings(child, "ldo_name");
	if (count <= 0) {
		pr_err("line:%d,no ldo_name found.", __LINE__);
		count = 0;
		return -EINVAL;
	}
	mpmic_dev->mpmic_num = count;
	mpmic_dev->is_mpmic_used = true;
	for (int i = 0; i < count; i++) {
		if (of_property_read_string(child, "ldo_name", &strings)) {
			pr_err("of_property_read_string :ldo_name, failed!");
			return -EINVAL;
		}
		memcpy_toio(mpmic_dev->mpmic.ldo_name, strings, strlen(strings));
		if (of_property_read_u32(child, "type", &val)) {
			pr_err("of_property_read_u32 type failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.type);
		if (of_property_read_u32(child, "vset_addr", &val)) {
			pr_err("of_property_read_u32 vset_addr failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.vset_addr);
		if (of_property_read_u32(child, "vset_read_addr", &val)) {
			pr_err("of_property_read_u32 vset_read_addr failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.vset_read_addr);
		if (of_property_read_u32(child, "power_en_addr", &val)) {
			pr_err("of_property_read_u32 power_en_addr failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.power_en_addr);
		if (of_property_read_u32(child, "power_ramp_addr", &val)) {
			pr_err("of_property_read_u32 power_ramp_addr failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.power_ramp_addr);
		if (of_property_read_u32(child, "regval_def", &val)) {
			pr_err("of_property_read_u32 regval_def failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.regval_def);
		if (of_property_read_u32(child, "voltval_def_mv", &val)) {
			pr_err("of_property_read_u32 voltval_def_mv failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.voltval_def_mv);
		if (of_property_read_u32(child, "vset_step_mv", &val)) {
			pr_err("of_property_read_u32 vset_step_mv failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.vset_step_mv);
		if (of_property_read_u32(child, "voltval_min_mv", &val)) {
			pr_err("of_property_read_u32 voltval_min_mv failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.voltval_min_mv);
		if (of_property_read_u32(child, "voltval_max_mv", &val)) {
			pr_err("of_property_read_u32 voltval_max_mv failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.voltval_max_mv);
		if (of_property_read_u32(child, "voltval_set_mv", &val)) {
			pr_err("of_property_read_u32 voltval_set_mv failed!\n");
			return -EINVAL;
		}
		writew((uint16_t)val, &mpmic_dev->mpmic.voltval_set_mv);
	}
	if (of_property_read_u32(child, "pwr_en_read_bit_shift", &val)) {
		pr_err("of_property_read_u32 pwr_en_read_bit_shift failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &mpmic_param->pwr_en_read_bit_shift);
	if (of_property_read_u32(child, "pwr_en_bit_shift", &val)) {
		pr_err("of_property_read_u32 pwr_en_bit_shift failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &mpmic_param->pwr_en_bit_shift);
	if (of_property_read_u32(child, "vset_valid_addr", &val)) {
		pr_err("of_property_read_u32 vset_valid_addr failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &mpmic_param->vset_valid_addr);
	if (of_property_read_u32(child, "rampup_valid_addr", &val)) {
		pr_err("of_property_read_u32 rampup_valid_addr failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &mpmic_param->rampup_valid_addr);
	if (of_property_read_u32(child, "rampdn_valid_addr", &val)) {
		pr_err("of_property_read_u32 rampdn_valid_addr failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &mpmic_param->rampdn_valid_addr);

	return 0;
}

static int aoc_cam_wldo_dts_parse(struct device_node *aoc_front_root,
	struct cam_wldo_info_s  *wldo_dev)
{
	struct device_node *child = NULL;
	struct device_node *node = NULL;
	const char *subldo_name;
	const char *strings;
	uint32_t val = 0, type;
	int count, i, ret = 0;

	child = of_find_node_by_name(aoc_front_root, "aoc_wldo");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("aoc_wldo node not found!\n");
		return ret;
	}
	if (of_property_read_string(child, "wldo_name", &strings)) {
		pr_err("of_property_read_string :wldo_name, failed!");
		return -EINVAL;
	}
	memcpy_toio(wldo_dev->wldo_info.wldo_name, strings, strlen(strings));
	if (of_property_read_string(child, "i2c_mst_name", &strings)) {
		pr_err("of_property_read_string :i2c_mst_name, failed!");
		return -EINVAL;
	}
	memcpy_toio(wldo_dev->wldo_info.i2c_mst_name, strings, strlen(strings));
	if (of_property_read_u32(child, "i2c_mst_port", &val)) {
		pr_err("of_property_read_u32 i2c_mst_port failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &wldo_dev->wldo_info.i2c_mst_port);
	if (of_property_read_u32(child, "wldo_chipid", &val)) {
		pr_err("of_property_read_u32 wldo_chipid failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &wldo_dev->wldo_info.wldo_chipid);
	if (of_property_read_u32(child, "wldo_chipid_addr", &val)) {
		pr_err("of_property_read_u32 wldo_chipid_addr failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &wldo_dev->wldo_info.wldo_chipid_addr);
	if (of_property_read_u32(child, "wldo_en_addr", &val)) {
		pr_err("of_property_read_u32 wldo_en_addr failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &wldo_dev->wldo_info.wldo_en_addr);
	if (of_property_read_u32(child, "wldo_slave_addr", &val)) {
		pr_err("of_property_read_u32 wldo_slave_addr failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &wldo_dev->wldo_info.wldo_slave_addr);
	if (of_property_read_u32(child, "wldo_en_io_num", &val))
		pr_err("of_property_read_u32 wldo_en_io_num failed!,maby not used!\n");
	else
		writew((uint16_t)val, &wldo_dev->wldo_info.en_gpio);
	if (of_property_read_u32(child, "wldo_i2c_freq",
		&wldo_dev->wldo_info.wldo_i2c_freq)) {
		pr_err("of_property_read_u32 wldo_i2c_freq failed!\n");
		return -EINVAL;
	}
	if (of_property_read_u32(child, "delay_after_rst", &val))
		pr_err("of_property_read_u32 delay_after_rst failed,maby not used!\n");
	else
		writew((uint16_t)val, &wldo_dev->wldo_info.delay_after_rst);
	count = of_property_count_strings(child, "subldo-names");
	if (count <= 0) {
		pr_err("no subldo-names found.");
		count = 0;
		return -EINVAL;
	}
	wldo_dev->wldo_info.sub_ldo_num = count;
	if (of_property_read_u32(child, "type", &type)) {
		pr_err("of_property_read_u32 type failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)type, &wldo_dev->wldo_info.type);
	for (i = 0; i < count; i++) {
		ret = of_property_read_string_index(child, "subldo-names", i, &subldo_name);
		if (ret) {
			pr_err("of_property_read_string_index :subldo-names, failed!");
			return -EINVAL;
		}
		node = of_find_node_by_name(child, subldo_name);
		if (node == NULL) {
			ret = -ENODEV;
			pr_err("[%s] node not found!\n", subldo_name);
			return ret;
		}
		if (of_property_read_string(node, "sub_ldo_name", &strings)) {
			pr_err("of_property_read_string :sub_ldo_name, failed!");
			return -EINVAL;
		}
		memcpy_toio(wldo_dev->subldo_info[i].sub_ldo_name, strings, strlen(strings));
		writew((uint16_t)type, &wldo_dev->subldo_info[i].type);
		if (of_property_read_u32(node, "ldo_num", &val)) {
			pr_err("get ldo_num[%d] failed.", i);
			return -EINVAL;
		}
		writew((uint16_t)val, &wldo_dev->subldo_info[i].ldo_num);
		if (of_property_read_u32(node, "ldo_regaddr", &val)) {
			pr_err("get ldo_regaddr[%d] failed.", i);
			return -EINVAL;
		}
		writew((uint16_t)val, &wldo_dev->subldo_info[i].ldo_regaddr);
		if (of_property_read_u32(node, "ldo_volt_mv",
			&wldo_dev->subldo_info[i].ldo_volt_mv)) {
			pr_err("get ldo_volt_mv[%d] failed.", i);
			return -EINVAL;
		}
	}
	return 0;
}

static int aoc_cam_csi_clk_dts_parse(struct device_node *aoc_front_root,
	struct csi_clk_info_s *csi_clk_info)
{
	struct device_node *child = NULL;
	uint32_t val = 0;
	int ret = 0;

	child = of_find_node_by_name(aoc_front_root, "csi_clk");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("csi_clk node not found!\n");
		return ret;
	}
	if (of_property_read_u32(child, "csi_clk_id", &val)) {
		pr_err("of_property_read_u32 csi_clk_id failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &csi_clk_info->csi_clk_id);
	if (of_property_read_u32(child, "csi_clk_normal",
		&csi_clk_info->csi_clk_normal)) {
		pr_err("of_property_read_u32 csi_clk_normal failed!\n");
		return -EINVAL;
	}
	if (of_property_read_u32(child, "csi_clk_low",
		&csi_clk_info->csi_clk_low)) {
		pr_err("of_property_read_u32 csi_clk_low failed!\n");
		return -EINVAL;
	}
	return 0;
}

static int aoc_cam_seq_dts_parse(struct device_node *aoc_front_root,
	struct cam_seq_s *cam_power_seq)
{
	struct device_node *child = NULL;
	int count, i, ret = 0;

	child = of_find_node_by_name(aoc_front_root, "aoc_pu_seq");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("aoc_pu_seq node not found!\n");
		return ret;
	}
	count = of_property_count_strings(child, "seq-names");
	if (count <= 0) {
		pr_err("no seq-names found.");
		count = 0;
		return -EINVAL;
	}
	cam_power_seq[0].seq_num = count;
	for (i = 0; i < count; i++) {
		if (of_property_read_u32_index(child,
			"seq-table", i, &cam_power_seq[0].cam_seq[i].seq_type)) {
			pr_err("get seq table:%d failed.", i);
			return -EINVAL;
		}
		if (of_property_read_u32_index(child,
			"seq-delay", i, &cam_power_seq[0].cam_seq[i].delay_ms)) {
			pr_err("get seq delay:%d failed.", i);
			return -EINVAL;
		}
		if (of_property_read_u32_index(child,
			"seq-val", i, &cam_power_seq[0].cam_seq[i].config_val)) {
			pr_err("get seq val:%d failed.", i);
			return -EINVAL;
		}
	}
	child = of_find_node_by_name(aoc_front_root, "aoc_pd_seq");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("aoc_pu_seq node not found!\n");
		return ret;
	}
	count = of_property_count_strings(child, "seq-names");
	if (count <= 0) {
		pr_err("no seq-names found.");
		count = 0;
		return -EINVAL;
	}
	cam_power_seq[1].seq_num = count;
	for (i = 0; i < count; i++) {
		if (of_property_read_u32_index(child,
			"seq-table", i, &cam_power_seq[1].cam_seq[i].seq_type)) {
			pr_err("get seq table:%d failed.", i);
			return -EINVAL;
		}
		if (of_property_read_u32_index(child,
			"seq-delay", i, &cam_power_seq[1].cam_seq[i].delay_ms)) {
			pr_err("get seq delay:%d failed.", i);
			return -EINVAL;
		}
		if (of_property_read_u32_index(child,
			"seq-val", i, &cam_power_seq[1].cam_seq[i].config_val)) {
			pr_err("get seq val:%d failed.", i);
			return -EINVAL;
		}
	}
	return 0;
}

static int aoc_cam_gpio_dts_parse(struct device_node *aoc_front_root,
	struct cam_power_io_s *cam_io)
{
	struct device_node *child = NULL;
	uint32_t val = 0;
	const char *strings;
	int count, i, ret = 0;

	child = of_find_node_by_name(aoc_front_root, "aoc_mclk");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("aoc_mclk node not found!\n");
		return ret;
	}
	if (of_property_read_string(child, "mclk_name", &strings)) {
		pr_err("of_property_read_string :mclk_name, failed!");
		return -EINVAL;
	}
	memcpy_toio(cam_io->gpio_mclk.name, strings, strlen(strings));
	if (of_property_read_u32(child, "mclk_rate",
		&cam_io->gpio_mclk.clk_rate)) {
		pr_err("of_property_read_u32 mclk_rate failed!\n");
		return -EINVAL;
	}
	if (of_property_read_u32(child, "mclk_io_num", &val)) {
		pr_err("of_property_read_u32 mclk_io_num failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &cam_io->gpio_mclk.gpio_num);
	if (of_property_read_u32(child, "mclk_id", &val)) {
		pr_err("of_property_read_u32 mclk_id failed!\n");
		return -EINVAL;
	}
	writew((uint16_t)val, &cam_io->gpio_mclk.id);
	if (of_property_read_u32(child, "is_osc_clk", &val)) {
		pr_err("of_property_read_u32 is_osc_clk failed!\n");
		return -EINVAL;
	}
	writeb((uint8_t)val, &cam_io->gpio_mclk.is_osc_clk);
	child = of_find_node_by_name(aoc_front_root, "aoc_gpio");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("aoc_gpio node not found!\n");
		return ret;
	}
	count = of_property_count_strings(child, "gpio-names");
	if (count <= 0) {
		pr_err("no gpio-names found.");
		count = 0;
		return -EINVAL;
	}
	cam_io->gpio_count = count;
	for (i = 0; i < count; i++) {
		if (of_property_read_string_index(child,
			"gpio-names", i, &strings)) {
			pr_err("get gpio-names :%d failed.", i);
			return -EINVAL;
		}
		memcpy_toio(cam_io->gpios[i].name, strings, strlen(strings));
		if (of_property_read_u32_index(child, "gpio-nums", i, &val)) {
			pr_err("get gpio-nums :%d failed.", i);
			return -EINVAL;
		}
		writew((uint16_t)val, &cam_io->gpios[i].gpio_num);
		if (of_property_read_u32_index(child, "gpio-mode", i, &val)) {
			pr_err("get gpio-mode :%d failed.", i);
			return -EINVAL;
		}
		writew((uint16_t)val, &cam_io->gpios[i].mode);
		if (of_property_read_u32_index(child, "gpio-def-val", i, &val)) {
			pr_err("get gpio-def-val :%d failed.", i);
			return -EINVAL;
		}
		writeb((uint8_t)val, &cam_io->gpios[i].is_low_def);
	}
	return 0;
}

static int aoc_cam_power_map_dts_parse(struct device_node *aoc_front_root,
	struct cam_map_s *cam_power_map)
{
	struct device_node *child = NULL;
	struct device_node *node = NULL;
	const char *strings;
	int idx = 0, ret = 0;

	child = of_find_node_by_name(aoc_front_root, "aoc_cam_seq_map");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("aoc_cam_seq_map node not found!\n");
		return ret;
	}
	while (idx < SH_AOC_SEQ_MAX) {
		node = of_get_next_available_child(child, node);
		if (node == NULL)
			break;
		ret = of_property_read_u32(node, "power_seq",
			&cam_power_map->power_map[idx].power_seq);
		if (ret < 0) {
			pr_err("power_seq read failed, ret[%d]!\n", ret);
			return ret;
		}
		ret = of_property_read_u32(node, "type",
			&cam_power_map->power_map[idx].power_type);
		if (ret < 0) {
			pr_err("power_type read failed, ret[%d]!\n", ret);
			return ret;
		}
		if (cam_power_map->power_map[idx].power_type == IO_SUPPLY) {
			ret = of_property_read_u32(node, "gpio_num",
				&cam_power_map->power_map[idx].gpio_num);
			if (ret < 0) {
				pr_err("gpio_num read failed, ret[%d]!\n", ret);
				return ret;
			}
		} else if (cam_power_map->power_map[idx].power_type == WLDO_28681c_SUPPLY ||
			cam_power_map->power_map[idx].power_type == WLDO_28661d_SUPPLY) {
			ret = of_property_read_string(node, "sub_ldo_name",
				&strings);
			if (ret < 0) {
				pr_err("sub_ldo_name read failed, ret[%d]!\n", ret);
				return ret;
			}
			memcpy_toio(cam_power_map->power_map[idx].sub_ldo_name,
				strings, strlen(strings));
		}
		idx++;
	}
	cam_power_map->map_num = idx;
	return 0;
}

int shub_parse_aoc_dts(struct device_node *parent, struct sh_bsp_dts *dts_mem_block)
{
	struct sh_dts_aoc_cam_power_info *aoc_power_info = &dts_mem_block->aoc_power_info;
	struct device_node *aoc_root = NULL;
	const char *strings;
	int ret = 0;

	aoc_root = of_find_node_by_name(parent, "aoc_front");
	if (aoc_root == NULL) {
		ret = -ENODEV;
		pr_err("%s node not found!\n", "aoc_front");
		return ret;
	}
	if (!of_device_is_available(aoc_root)) {
		ret = -EINVAL;
		pr_err("%s node not enabled!\n", "aoc_front");
		return ret;
	}
	ret = aoc_cam_mpmic_dts_parse(aoc_root, &aoc_power_info->mpmic_dev,
		&aoc_power_info->mpmic_param);
	if (ret) {
		pr_err("aoc_cam_mpmic_dts_parse failed,ret = %d!\n", ret);
		return ret;
	}
	ret = aoc_cam_wldo_dts_parse(aoc_root, &aoc_power_info->wldo_dev);
	if (ret) {
		pr_err("aoc_cam_wldo_dts_parse failed,ret = %d!\n", ret);
		return ret;
	}
	ret = aoc_cam_csi_clk_dts_parse(aoc_root, &aoc_power_info->csi_clk_info);
	if (ret) {
		pr_err("aoc_cam_csi_clk_dts_parse failed,ret = %d!\n", ret);
		return ret;
	}
	ret = aoc_cam_gpio_dts_parse(aoc_root, &aoc_power_info->cam_io);
	if (ret) {
		pr_err("aoc_cam_gpio_dts_parse failed,ret = %d!\n", ret);
		return ret;
	}
	ret = aoc_cam_seq_dts_parse(aoc_root, aoc_power_info->cam_power_seq);
	if (ret) {
		pr_err("aoc_cam_seq_dts_parse failed,ret = %d!\n", ret);
		return ret;
	}
	ret = aoc_cam_power_map_dts_parse(aoc_root, &aoc_power_info->cam_power_map);
	if (ret) {
		pr_err("aoc_cam_power_map_dts_parse failed,ret = %d!\n", ret);
		return ret;
	}
	if (of_property_read_string(aoc_root, "sensor_type", &strings)) {
		pr_err("of_property_read_string :sensor_type, failed!");
		return -EINVAL;
	}
	memcpy_toio(aoc_power_info->sensor_type, strings, strlen(strings));
	pr_info("%s success!\n", __func__);
	return 0;
}
EXPORT_SYMBOL(shub_parse_aoc_dts);
