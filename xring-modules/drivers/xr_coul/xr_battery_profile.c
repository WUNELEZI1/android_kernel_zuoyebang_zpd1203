// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "xr_battery_profile.h"

#define batt_profile_debug(fmt, args...) \
	pr_debug("[batt_profile] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define batt_profile_info(fmt, args...) \
	pr_info("[batt_profile] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define batt_profile_warn(fmt, args...) \
	pr_warn("[batt_profile] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define batt_profile_err(fmt, args...) \
	pr_err("[batt_profile] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define BATT_PROFILE_DEBUG 1
#ifdef BATT_PROFILE_DEBUG
#define bat_debug_log(fmt, args...) batt_profile_info(fmt, ## args)
#else
#define bat_debug_log(fmt, args...) do {} while (0)
#endif

#define MAX_TEMP_LEVEL_NUM 6
/* static variable definition */
static struct coul_battery_profile **batt_profile;
static bool batt_profile_init_state;
static unsigned int batt_profile_num;
static int batt_temp_level[MAX_TEMP_LEVEL_NUM] = { -20, -10, 0, 25, 40, 60 };

static int get_batt_sn_dat(struct device_node *np,
	struct coul_battery_profile *pdat)
{
	int ret;

	ret = of_property_read_string(np,
		"batt_sn", (const char **)(&pdat->batt_sn));
	if (ret) {
		batt_profile_err("get sn_auth failed\n");
		return -EINVAL;
	}
	bat_debug_log("batt_sn is %s\n", pdat->batt_sn);
	return 0;
}

static int get_batt_basic_para(struct device_node *np,
	struct coul_battery_profile *pdat)
{
	int ret;

	ret = of_property_read_u32(np, "fcc_desgin", &(pdat->battery_data.fcc));
	if (ret) {
		batt_profile_err("get fcc_desgin failed\n");
		return -EINVAL;
	}
	bat_debug_log("fcc_desgin is %u\n", pdat->battery_data.fcc);

	ret = of_property_read_string(np, "auth_type",
		(const char **)(&pdat->auth_type));
	if (ret) {
		batt_profile_err("get batt_identify_type failed!\n");
		return -EINVAL;
	}
	bat_debug_log("auth_type is %s\n", pdat->auth_type);

	if (!strncmp(pdat->auth_type,
		BATT_AUTH_BY_SN, strlen(BATT_AUTH_BY_SN))) {
		ret = get_batt_sn_dat(np, pdat);
	} else {
		batt_profile_err("has no matched auth type\n");
		return -EINVAL;
	}
	if (ret) {
		batt_profile_err("get batt_identify_para failed!\n");
		return -EINVAL;
	}
	ret = of_property_read_u32(np, "rbatt_mohm",
		&(pdat->battery_data.default_rbatt_mohm));
	if (ret) {
		batt_profile_err("get rbatt_mohm failed\n");
		return -EINVAL;
	}
	bat_debug_log("rbatt_mohm is %d\n", pdat->battery_data.default_rbatt_mohm);
	/* vbat_max */
	ret = of_property_read_u32(np, "vbat_max", &(pdat->battery_data.vbatt_max));
	if (ret) {
		batt_profile_err("get vbat_max failed\n");
		return -EINVAL;
	}
	bat_debug_log("vbat_max is %d\n", pdat->battery_data.vbatt_max);

	/* vbat_min */
	ret = of_property_read_u32(np, "vbat_min", &(pdat->battery_data.vbatt_min));
	if (ret) {
		batt_profile_err("get vbat_min failed\n");
		return -EINVAL;
	}
	bat_debug_log("vbat_min is %d\n", pdat->battery_data.vbatt_min);

	/* low_temp_opt_flag */
	ret = of_property_read_u32(np, "low_temp_opt_flag",
			&(pdat->battery_data.low_temp_opt_flag));
	if (ret)
		batt_profile_err("get low_temp_opt_flag failed\n");
	bat_debug_log("low_temp_opt_flag is %d\n",
		pdat->battery_data.low_temp_opt_flag);

	/* vbat_low_temp_soc_three */
	ret = of_property_read_u32(np, "vbat_low_temp_soc_three",
			&(pdat->battery_data.vbatt_low_temp_soc_three));
	if (ret)
		batt_profile_err("get vbat_low_temp_soc_three failed\n");
	bat_debug_log("vbat_low_temp_soc_three is %d\n",
		pdat->battery_data.vbatt_low_temp_soc_three);

	/* full_curr */
	ret = of_property_read_u32(np, "full_curr", &(pdat->full_curr));
	if (ret) {
		pdat->full_curr = FULL_CURR_DEFAULT;
		batt_profile_err("get full_curr failed,using default:%d\n", FULL_CURR_DEFAULT);
	}
	bat_debug_log("full_curr is %d\n", pdat->full_curr);
	return 0;
}

static int get_batt_fcc_temp(struct device_node *np,
	struct coul_battery_profile *pdat)
{
	int ret, i;

	pdat->battery_data.fcc_temp_lut.cols = TEMP_SAMPLING_POINTS;
	for (i = 0; i < pdat->battery_data.fcc_temp_lut.cols; i++) {
		pdat->battery_data.fcc_temp_lut.x[i] = batt_temp_level[i];
		ret = of_property_read_u32_index(np, "fcc_temp",
			i, (unsigned int *)(&(pdat->battery_data.fcc_temp_lut.y[i])));
		if (ret) {
			batt_profile_err("get fcc_temp[%d] failed\n", i);
			return -EINVAL;
		}
	}

	return 0;
}

static int get_batt_pc_coef_lut_para(struct device_node *np,
	struct coul_battery_profile *pdat)
{
	int ret, i, j;

	ret = of_property_read_u32(np, "pc_coef_rows",
		&(pdat->battery_data.pc_coef_lut.rows));
	if (ret) {
		batt_profile_err("get pc_coef_rows failed\n");
		return -EINVAL;
	}

	if (pdat->battery_data.pc_coef_lut.rows >= MAX_COEF_COL_SIZE) {
		batt_profile_err("pc_coef_rows is %d over max size:%d\n",
			pdat->battery_data.pc_coef_lut.rows,
			MAX_COEF_COL_SIZE);
		return -EINVAL;
	}

	bat_debug_log("pc_coef_rows is %d\n", pdat->battery_data.pc_coef_lut.rows);

	ret = of_property_read_u32(np, "pc_coef_cols",
		&(pdat->battery_data.pc_coef_lut.cols));
	if (ret) {
		batt_profile_err("get pc_coef_cols failed\n");
		return -EINVAL;
	}

	if (pdat->battery_data.pc_coef_lut.cols >= MAX_COEF_ROW_SIZE) {
		batt_profile_err("pc_coef_cols is %d over max size:%d\n",
			pdat->battery_data.pc_coef_lut.cols,
			MAX_COEF_ROW_SIZE);
		return -EINVAL;
	}

	bat_debug_log("pc_coef_cols is %d\n", pdat->battery_data.pc_coef_lut.cols);

	for (i = 0; i < pdat->battery_data.pc_coef_lut.rows; i++) {
		ret = of_property_read_u32(np, "pc_coef_row_entries",
			&(pdat->battery_data.pc_coef_lut.row_entries[i]));
		if (ret) {
			batt_profile_err("get pc_coef_row_entries[%d] failed\n", i);
			return -EINVAL;
		}
		bat_debug_log("pc_coef_row_entries is %d\n",
			pdat->battery_data.pc_coef_lut.row_entries[i]);
	}

	for (i = 0; i < pdat->battery_data.pc_coef_lut.cols; i++) {
		ret = of_property_read_u32(np, "pc_coef_percent",
			&(pdat->battery_data.pc_coef_lut.percent[i]));
		if (ret) {
			batt_profile_err("get pc_coef_percent[%d] failed\n", i);
			return -EINVAL;
		}
		bat_debug_log("pc_coef_percent is %d\n", pdat->battery_data.pc_coef_lut.percent[i]);
	}

	for (i = 0; i < pdat->battery_data.pc_coef_lut.cols; i++) {
		for (j = 0; j < pdat->battery_data.pc_coef_lut.rows; j++) {
			ret = of_property_read_u32(np, "pc_coef_sf",
				&(pdat->battery_data.pc_coef_lut.coef[j][i]));
			if (ret) {
				batt_profile_err("get pc_coef_sf[%d] failed\n",
					j * pdat->battery_data.pc_coef_lut.cols + i);
				return -EINVAL;
			}
			bat_debug_log("pc_coef_sf[%d] is %d\n",
				j * pdat->battery_data.pc_coef_lut.cols + i,
				pdat->battery_data.pc_coef_lut.coef[j][i]);
		}
	}

	return 0;
}

static int get_batt_fcc_coef_para(struct device_node *np,
	struct coul_battery_profile *pdat)
{
	int i, ret;

	ret = of_property_read_u32(np, "fcc_coef_cols",
		&(pdat->battery_data.fcc_coef_lut.cols));
	if (ret) {
		batt_profile_err("get fcc_coef_cols failed\n");
		return -EINVAL;
	}

	if (pdat->battery_data.fcc_coef_lut.cols >= MAX_LUT_SIZE) {
		batt_profile_err("fcc_coef_cols is %d, over max size:%d\n",
			pdat->battery_data.fcc_coef_lut.cols,
			MAX_LUT_SIZE);
	}

	bat_debug_log("fcc_coef_cols is %d\n", pdat->battery_data.fcc_coef_lut.cols);

	for (i = 0; i < pdat->battery_data.fcc_coef_lut.cols; i++) {
		ret = of_property_read_u32_index(np, "fcc_coef_x",
			i, (unsigned int *)(&(pdat->battery_data.fcc_coef_lut.x[i])));
		if (ret) {
			batt_profile_err("get fcc_coef_x[%d] failed\n", i);
			return -EINVAL;
		}
		bat_debug_log("fcc_coef_x[%d] is %d\n", i, pdat->battery_data.fcc_coef_lut.x[i]);
		ret = of_property_read_u32_index(np, "fcc_coef_y",
			i, (unsigned int *)(&(pdat->battery_data.fcc_coef_lut.y[i])));
		if (ret) {
			batt_profile_err("get fcc_coef_y[%d] failed\n", i);
			return -EINVAL;
		}
		bat_debug_log("fcc_coef_y[%d] is %d\n", i, pdat->battery_data.fcc_coef_lut.y[i]);
	}
	/* pc_coef_lut */
	ret = get_batt_pc_coef_lut_para(np, pdat);
	if (ret) {
		batt_profile_err("get get_pc_coef_lut_dts failed\n");
		return -EINVAL;
	}

	return 0;
}

static int get_batt_rbat_coef_para(struct device_node *np,
	struct coul_battery_profile *pdat)
{
	int ret, i, j;

	ret = of_property_read_u32(np, "rbatt_coef_rows",
		&(pdat->battery_data.rbatt_coef_lut.rows));
	if (ret) {
		batt_profile_err("get rbatt_coef_rows failed\n");
		return -EINVAL;
	}

	if (pdat->battery_data.rbatt_coef_lut.rows >= MAX_COEF_ROW_SIZE) {
		batt_profile_err("rbatt_coef_rows is %d, over max size:%d\n",
			pdat->battery_data.rbatt_coef_lut.rows,
			MAX_COEF_ROW_SIZE);
		return -EINVAL;
	}

	bat_debug_log("rbatt_coef_rows is %d\n", pdat->battery_data.rbatt_coef_lut.rows);

	ret = of_property_read_u32(np, "rbatt_coef_cols",
		&(pdat->battery_data.rbatt_coef_lut.cols));
	if (ret) {
		batt_profile_err("get rbatt_coef_cols failed\n");
		return -EINVAL;
	}

	if (pdat->battery_data.rbatt_coef_lut.cols > MAX_TEMP_LEVEL_NUM) {
		batt_profile_err("rbatt_coef_cols is %d > max size:%d\n",
			pdat->battery_data.rbatt_coef_lut.cols,
			MAX_TEMP_LEVEL_NUM);
			return -EINVAL;
	}

	bat_debug_log("rbatt_coef_cols is %d\n", pdat->battery_data.rbatt_coef_lut.cols);

	for (i = 0; i < pdat->battery_data.rbatt_coef_lut.rows; i++) {
		ret = of_property_read_u32_index(np, "rbatt_coef_percent", i,
			(unsigned int *)(&(pdat->battery_data.rbatt_coef_lut.percent[i])));
		if (ret) {
			batt_profile_err("get rbatt_coef_percent[%d] failed\n", i);
			return -EINVAL;
		}
		bat_debug_log("rbatt_coef_percent[%d] is %d\n",
			i, pdat->battery_data.rbatt_coef_lut.percent[i]);
	}
	for (i = 0; i < pdat->battery_data.rbatt_coef_lut.cols; i++)
		pdat->battery_data.rbatt_coef_lut.row_entries[i] = batt_temp_level[i];

	for (i = 0; i < pdat->battery_data.rbatt_coef_lut.cols; i++) {
		for (j = 0; j < pdat->battery_data.rbatt_coef_lut.rows; j++) {
			ret = of_property_read_u32_index(np,
			  "rbatt_coef_sf", j * TEMP_SAMPLING_POINTS + i,
			  (unsigned int *)(&(pdat->battery_data.rbatt_coef_lut.coef[j][i])));
			if (ret) {
				batt_profile_err("get rbatt_coef_sf[%d] failed\n",
					j * TEMP_SAMPLING_POINTS + i);
				return -EINVAL;
			}
			bat_debug_log("rbatt_coef_sf[%d] is %d\n",
				j * TEMP_SAMPLING_POINTS + i,
				pdat->battery_data.rbatt_coef_lut.coef[j][i]);
		}
	}
	return 0;
}

static int get_batt_pc_temp_ocv_para(struct device_node *np,
	struct coul_battery_profile *pdat)
{
	int ret, i, j;

	ret = of_property_read_u32(np, "pc_temp_ocv_rows",
		&(pdat->battery_data.pc_temp_ocv_lut.rows));
	if (ret) {
		batt_profile_err("get pc_temp_ocv_rows failed\n");
		return -EINVAL;
	}

	if (pdat->battery_data.pc_temp_ocv_lut.rows >= MAX_COEF_ROW_SIZE) {
		batt_profile_err("pc_temp_ocv_rows is %d > max size:%d\n",
			pdat->battery_data.pc_temp_ocv_lut.rows,
			MAX_COEF_ROW_SIZE);
		return -EINVAL;
	}

	bat_debug_log("pc_temp_ocv_rows is %d\n", pdat->battery_data.pc_temp_ocv_lut.rows);

	ret = of_property_read_u32(np, "pc_temp_ocv_cols",
		&(pdat->battery_data.pc_temp_ocv_lut.cols));
	if (ret) {
		batt_profile_err("get pc_temp_ocv_cols failed\n");
		return -EINVAL;
	}

	if (pdat->battery_data.pc_temp_ocv_lut.cols > MAX_TEMP_LEVEL_NUM) {
		batt_profile_err("pc_temp_ocv_cols is %d > max size:%d\n",
			pdat->battery_data.pc_temp_ocv_lut.cols,
			MAX_TEMP_LEVEL_NUM);
			return -EINVAL;
	}

	bat_debug_log("pc_temp_ocv_cols is %d\n", pdat->battery_data.pc_temp_ocv_lut.cols);

	for (i = 0; i < pdat->battery_data.pc_temp_ocv_lut.rows - 1; i++) {
		ret = of_property_read_u32_index(np,
		 "pc_temp_ocv_percent", i,
		 (unsigned int *)(&(pdat->battery_data.pc_temp_ocv_lut.percent[i])));
		if (ret) {
			batt_profile_err("get pc_temp_ocv_percent[%d] failed\n", i);
			return -EINVAL;
		}
		bat_debug_log("pc_temp_ocv_percent[%d] is %d\n",
			i, pdat->battery_data.pc_temp_ocv_lut.percent[i]);
	}
	for (i = 0; i < pdat->battery_data.pc_temp_ocv_lut.cols; i++)
		pdat->battery_data.pc_temp_ocv_lut.temp[i] = batt_temp_level[i];

	for (i = 0; i < pdat->battery_data.pc_temp_ocv_lut.cols; i++) {
		for (j = 0; j < pdat->battery_data.pc_temp_ocv_lut.rows; j++) {
			ret = of_property_read_u32_index(np, "pc_temp_ocv_ocv",
			  j * TEMP_SAMPLING_POINTS + i, (unsigned int *)(
			  &(pdat->battery_data.pc_temp_ocv_lut.ocv[j][i])));
			if (ret) {
				batt_profile_err("get pc_temp_ocv_ocv[%d] failed\n",
					j * TEMP_SAMPLING_POINTS + i);
				return -EINVAL;
			}
			bat_debug_log("pc_temp_ocv_ocv[%d][%d] = %d\n", j, i,
				pdat->battery_data.pc_temp_ocv_lut.ocv[j][i]);
		}
	}
	return 0;
}

static int get_batt_data(struct device_node *np, struct coul_battery_profile *pdat)
{
	int ret;

	ret = get_batt_basic_para(np, pdat);
	if (ret)
		return -EINVAL;

	/* fcc_temp */
	ret = get_batt_fcc_temp(np, pdat);
	if (ret)
		return -EINVAL;

	/* fcc_coef */
	ret = get_batt_fcc_coef_para(np, pdat);
	if (ret)
		return -EINVAL;

	/* rbat_coef */
	ret = get_batt_rbat_coef_para(np, pdat);
	if (ret)
		return -EINVAL;

	/* pc_temp_ocv */
	ret = get_batt_pc_temp_ocv_para(np, pdat);
	if (ret)
		return -EINVAL;

	return ret;
}

static int batt_data_initial(struct platform_device *pdev, struct device_node *np)
{
	int retval;
	unsigned int i;
	struct device_node *bat_node = NULL;

	/* alloc memory to store pointers(point to battery data) */
	batt_profile = kcalloc(batt_profile_num,
		sizeof(struct coul_battery_profile *), GFP_KERNEL);
	if (!batt_profile)
		return -EINVAL;

	for (i = 0; i < batt_profile_num; i++) {
		batt_profile[i] = kcalloc(1,
			sizeof(struct coul_battery_profile), GFP_KERNEL);
		if (!batt_profile[i])
			goto free_pre_batt_data;

		bat_node = of_parse_phandle(np, "batt_name", i);
		if (!bat_node) {
			batt_profile_err("get bat_node failed\n");
			goto free_batt_data;
		}

		retval = get_batt_data(bat_node, batt_profile[i]);
		if (retval) {
			batt_profile_err("get_dat[%d] failed\n", i);
			goto free_batt_data;
		}
	}
	return 0;

free_batt_data:
	i += 1;
free_pre_batt_data:
	if (batt_profile) {
		while (i) {
			i -= 1;
			kfree(batt_profile[i]);
		}
		kfree(batt_profile);
	}
	batt_profile = NULL;
	return -EINVAL;
}

int get_battery_batt_sn(unsigned char *name, unsigned int name_size)
{
	/*TODO read batt_sn from battery*/
	return 0;
}

static int get_battery_profile_by_batt_sn(unsigned int id_index)
{
	int ret;
	unsigned char batt_sn[ID_SN_SIZE] = {0};

	if (id_index >= batt_profile_num)
		return -EINVAL;

	ret = get_battery_batt_sn(batt_sn, ID_SN_SIZE);
	if (ret) {
		batt_profile_err("get batt_sn from ic fail!\n");
		return -EINVAL;
	}

	batt_profile_info("batt_sn from ic is %s\n", batt_sn);

	if (!strncmp(batt_profile[id_index]->batt_sn, batt_sn, strlen(batt_sn)))
		return 0;

	return -EINVAL;
}

struct coul_battery_data *get_battery_profile(void)
{
	unsigned int i;
	int ret;

	if (!batt_profile_init_state) {
		batt_profile_err("battery param is invalid\n");
		return NULL;
	}

	for (i = 0; i < batt_profile_num; i++) {
		if (!strncmp(batt_profile[i]->auth_type,
			BATT_AUTH_BY_SN, strlen(BATT_AUTH_BY_SN))) {
			ret = get_battery_profile_by_batt_sn(i);
			if (!ret)
				break;
		} else
			batt_profile_err("batt_identify_type error\n");
	}

	if (i == batt_profile_num) {
		batt_profile_err("cannot find batt id\n");
		return NULL;
	} else
		return &batt_profile[i]->battery_data;

}

static int battery_data_probe(struct platform_device *pdev)
{
	int retval;
	unsigned int i;
	struct device_node *np = NULL;

	/* get device node for battery module */
	batt_profile_init_state = false;
	np = pdev->dev.of_node;
	if (!np) {
		batt_profile_err("get device node failed\n");
		return -EINVAL;
	}

	/* get numeber of types */
	for (i = 0;; ++i) {
		if (!of_parse_phandle(np, "batt_name", i))
			break;
	}
	if (!i) {
		batt_profile_err("bat_data_size is zero\n");
		return -EINVAL;
	}
	batt_profile_num = i;
	batt_profile_info("bat_data_size = %u\n", batt_profile_num);

	retval = batt_data_initial(pdev, np);
	if (retval)
		return retval;

	batt_profile_init_state = true;
	batt_profile_info("probe ok\n");
	return 0;
}

static int battery_data_remove(struct platform_device *pdev)
{
	unsigned int i;

	if (batt_profile) {
		for (i = 0; i < batt_profile_num; ++i)
			kfree((batt_profile[i]));

		kfree(batt_profile);
	}
	batt_profile = NULL;
	return 0;
}

static const struct of_device_id bat_match_table[] = {
	{
		.compatible = "xring,battery",
		.data = NULL,
	},
	{
	},
};

static struct platform_driver bat_driver = {
	.probe = battery_data_probe,
	.remove = battery_data_remove,
	.driver = {
		.name = "battery",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_match_table),
	},
};

int battery_profile_init(void)
{
	return platform_driver_register(&bat_driver);
}

void battery_profile_exit(void)
{
	platform_driver_unregister(&bat_driver);
}

MODULE_AUTHOR("XRing Technologies Co., Ltd");
MODULE_DESCRIPTION("xring battery profile driver");
MODULE_LICENSE("GPL");
