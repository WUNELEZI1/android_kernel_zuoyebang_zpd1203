// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include "xr_coul_nv.h"
#include "xr_coul_xp2210.h"
#include <soc/xring/xr_nv_interface.h>

#define PMU_NV_ADDR_CMDLINE_MAX_LEN 30

static struct coul_cali_params priv_coul_cali_params = {
	DEFAULT_V_OFF_A, 0, DEFAULT_C_OFF_A, 0, 0
};

static struct mutex coul_nv_lock;

static struct coul_nv_data priv_coul_nv_data;

static void set_coul_cali_params(struct coul_cali_params *cali_params)
{
	if (!cali_params)
		return;

	if (cali_params->v_offset_a)
		priv_coul_cali_params.v_offset_a = cali_params->v_offset_a;
	if (cali_params->v_offset_b)
		priv_coul_cali_params.v_offset_b = cali_params->v_offset_b;
	if (cali_params->c_offset_a)
		priv_coul_cali_params.c_offset_a = cali_params->c_offset_a;
	if (cali_params->c_offset_b)
		priv_coul_cali_params.c_offset_b = cali_params->c_offset_b;

	priv_coul_cali_params.chip_id = cali_params->chip_id;
}

static int coul_nv_read_cali(void)
{
	int ret;
	static struct nv_info_struct coul_nv_info;

	struct coul_cali_params cali_params = {
		DEFAULT_V_OFF_A, DEFAULT_V_OFF_B,
		DEFAULT_C_OFF_A, DEFAULT_C_OFFSET_B
	};

	coul_nv_info.operation = NV_READ;
	coul_nv_info.item_id = NV_UEFI_COUL_CHANNEL0;

	ret = nv_read_write_item(&coul_nv_info);
	if (ret) {
		coul_err("nv read item failed, ret:%d\n", ret);
	} else {
		mutex_lock(&coul_nv_lock);
		memcpy(&priv_coul_nv_data, coul_nv_info.buf.nv_data, sizeof(priv_coul_cali_params));

		cali_params.v_offset_a = (priv_coul_nv_data.v_offset_a == 0) ?
			DEFAULT_V_OFF_A : priv_coul_nv_data.v_offset_a;
		cali_params.v_offset_b = (priv_coul_nv_data.v_offset_b == 0) ?
			DEFAULT_V_OFF_B : priv_coul_nv_data.v_offset_b;

		cali_params.c_offset_a = (priv_coul_nv_data.c_offset_a == 0) ?
			DEFAULT_C_OFF_A : priv_coul_nv_data.c_offset_a;
		cali_params.c_offset_b = (priv_coul_nv_data.c_offset_b == 0) ?
			DEFAULT_C_OFFSET_B : priv_coul_nv_data.c_offset_b;

		cali_params.chip_id = priv_coul_nv_data.chip_id;
		mutex_unlock(&coul_nv_lock);

		set_coul_cali_params(&cali_params);
	}

	coul_info("finala/b value, v_a=%d,v_b=%d, c_a=%d, c_b=%d, chip_id:0x%llx.\n",
		cali_params.v_offset_a, cali_params.v_offset_b,
		cali_params.c_offset_a, cali_params.c_offset_b, cali_params.chip_id);

	return ret;
}

int coul_nv_write_cali(struct coul_cali_params *cali_params)
{
	int ret;
	static struct nv_info_struct coul_nv_info;

	if (!cali_params)
		return -EINVAL;

	coul_nv_info.operation = NV_READ;
	coul_nv_info.item_id = NV_UEFI_COUL_CHANNEL0;
	ret = nv_read_write_item(&coul_nv_info);
	if (ret) {
		coul_err("nv read item failed, ret:%d\n", ret);
		return ret;
	}

	coul_nv_info.operation = NV_WRITE;

	memcpy(coul_nv_info.buf.nv_data, cali_params, sizeof(struct coul_cali_params));

	ret = nv_read_write_item(&coul_nv_info);
	if (ret)
		coul_err("nv write item failed, ret:%d\n", ret);

	set_coul_cali_params(cali_params);

	coul_info("finala/b value, v_offset_a=%d,v_offset_b=%d, c_offset_a=%d, c_offset_b=%d, chip_id:0x%llx.\n",
		cali_params->v_offset_a, cali_params->v_offset_b,
		cali_params->c_offset_a, cali_params->c_offset_b, cali_params->chip_id);

	return ret;
}

static void coul_nv_print_nv_info(struct coul_nv_info *nv_info)
{
	int i = 0;

	if (nv_info == NULL)
		return;

	coul_info("charge_cycles:%d\n", nv_info->charge_cycles);
	coul_info("limit_fcc:%d\n", nv_info->limit_fcc);
	coul_info("qmax:%d\n", nv_info->qmax);

	for (i = 0; i < NV_MAX_TEMPS_SIZE; i++)
		coul_info("temp[%d]:%d, fcc[%d]:%d\n", i, nv_info->temp[i], i, nv_info->fcc[i]);

	coul_info("ocv_voltage_onboot:%d\n", nv_info->ocv_voltage_onboot);
	coul_info("ocv_current_onboot:%d\n", nv_info->ocv_current_onboot);

}

static int coul_nv_read_backup(void)
{
	int ret;
	static struct nv_info_struct coul_nv_info;

	coul_nv_info.operation = NV_READ;

	coul_nv_info.item_id = NV_UEFI_COUL_CHANNEL1;

	ret = nv_read_write_item(&coul_nv_info);
	if (ret) {
		coul_err("nv read item failed, ret:%d\n", ret);
	} else {
		mutex_lock(&coul_nv_lock);
		memcpy(&priv_coul_nv_data.nv_data,
			coul_nv_info.buf.nv_data, sizeof(priv_coul_nv_data.nv_data));
		coul_nv_print_nv_info(&priv_coul_nv_data.nv_data);
		mutex_unlock(&coul_nv_lock);
	}

	return ret;
}

static int coul_nv_write_backup_data(struct coul_nv_info *nv_info)
{
	int ret;
	static struct nv_info_struct coul_nv_info_s;

	if (!nv_info) {
		coul_err("NULL point\n");
		return -EINVAL;
	}

	coul_nv_info_s.operation = NV_READ;
	coul_nv_info_s.item_id = NV_UEFI_COUL_CHANNEL1;

	ret = nv_read_write_item(&coul_nv_info_s);
	if (ret)
		coul_err("nv read item failed, ret:%d\n", ret);

	coul_nv_info_s.operation = NV_WRITE;

	memcpy(coul_nv_info_s.buf.nv_data, nv_info, sizeof(struct coul_nv_info));

	ret = nv_read_write_item(&coul_nv_info_s);
	if (ret)
		coul_err("nv write item failed, ret:%d\n", ret);

	coul_nv_print_nv_info(nv_info);

	return ret;
}

void get_coul_cali_params(struct coul_cali_params *cali_params)
{
	u64 chip_id;

	if (!cali_params)
		return;

	if (coul_get_chipid(&chip_id) || chip_id != priv_coul_cali_params.chip_id)
		return;

	memcpy(cali_params, &priv_coul_cali_params, sizeof(priv_coul_cali_params));
}

/* get NV info from fastboot send */
int coul_nv_params_initial(struct coul_core_device *chip)
{
	struct coul_nv_data *pdata = NULL;

	if (!chip) {
		coul_err("NULL point\n");
		return -EINVAL;
	}

	pdata = kzalloc(sizeof(struct coul_nv_data), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	mutex_lock(&coul_nv_lock);
	memcpy(pdata, &priv_coul_nv_data, sizeof(priv_coul_nv_data));

	memcpy(&chip->common_data.nv_info, &priv_coul_nv_data.nv_data,
		sizeof(priv_coul_nv_data.nv_data));
	mutex_unlock(&coul_nv_lock);

	chip->common_data.charge_cycles = (int)pdata->nv_data.charge_cycles;
	chip->common_data.qmax = pdata->nv_data.qmax;
	chip->common_data.batt_limit_fcc = pdata->nv_data.limit_fcc;

	coul_debug("charge_cycles:%d, limit_fcc:%d, qmax:%d\n",
		pdata->nv_data.charge_cycles, pdata->nv_data.limit_fcc, pdata->nv_data.qmax);

	for (int i = 0; i < NV_MAX_TEMPS_SIZE; i++)
		coul_debug("nv_temp[%d] : %d\n", i, pdata->nv_data.temp[i]);

	for (int i = 0; i < NV_MAX_TEMPS_SIZE; i++)
		coul_debug("nv_fcc[%d] : %d\n", i, pdata->nv_data.fcc[i]);

	coul_debug("nv_ocv_voltage_onboot=%d, nv_ocv_current_onboot=%d\n",
		pdata->nv_data.ocv_voltage_onboot, pdata->nv_data.ocv_current_onboot);

	kfree(pdata);

	return 0;
}

int coul_nv_params_deinitial(struct coul_core_device *chip)
{
	struct coul_nv_data *pdata = NULL;

	if (!chip) {
		coul_err("NULL point\n");
		return -EINVAL;
	}

	pdata = kzalloc(sizeof(struct coul_nv_data), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	chip->common_data.charge_cycles = 0;
	chip->common_data.qmax = 0;
	chip->common_data.batt_limit_fcc = 0;

	coul_nv_write_backup_data(&pdata->nv_data);

	kfree(pdata);

	coul_info("success!!!\n");
	return 0;
}

int store_nv_data(struct coul_core_device *chip)
{
	if (!chip) {
		coul_err("chip is null\n");
		return -EINVAL;
	}

	coul_nv_write_backup_data(&chip->common_data.nv_info);

	return 0;
}

int coul_nv_data_initial(void)
{
	mutex_init(&coul_nv_lock);
	coul_nv_read_cali();
	coul_nv_read_backup();

	return 0;
}

int coul_nv_data_exit(void)
{
	mutex_destroy(&coul_nv_lock);
	return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("XRing Technologies Co., Ltd");
MODULE_DESCRIPTION("xr coul nv driver");
