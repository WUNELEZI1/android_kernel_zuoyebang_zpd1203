// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#include <linux/module.h>
#include <soc/xring/xr_coul_interface.h>
#include "xr_coul_core.h"

#define coul_intf_debug(fmt, args...) \
	pr_debug("[coul_intf] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_intf_info(fmt, args...) \
	pr_info("[coul_intf] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_intf_warn(fmt, args...) \
	pr_warn("[coul_intf] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_intf_err(fmt, args...) \
	pr_err("[coul_intf] [%s:%d]" fmt, __func__, __LINE__, ## args)

static int coul_intf_init_flag;
static struct mutex coul_intf_lock;

static void coul_intf_lock_f(void)
{
	if (!coul_intf_init_flag)
		return;
	mutex_lock(&coul_intf_lock);
}

static void coul_intf_unlock_f(void)
{
	if (!coul_intf_init_flag)
		return;
	mutex_unlock(&coul_intf_lock);
}

 /* get charge status */
int coul_intf_get_chg_status(void)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (!chip)
		return -EPERM;

	return chip->common_data.charging_state;
}
EXPORT_SYMBOL_GPL(coul_intf_get_chg_status);

 /* check wheather coul is ready 1: OK 0:not ready */
int coul_intf_init_state(void)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (chip)
		return 1;

	return 0;
}
EXPORT_SYMBOL_GPL(coul_intf_init_state);

int coul_intf_read_batt_fcc_design(void)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (!chip)
		return -EPERM;

	return chip->k2u_data.batt_data.fcc;
}
EXPORT_SYMBOL_GPL(coul_intf_read_batt_fcc_design);

/* battery vbat max vol */
int coul_intf_read_vbatt_max(void)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (!chip)
		return -EPERM;

	return chip->k2u_data.batt_data.vbatt_max;
}
EXPORT_SYMBOL_GPL(coul_intf_read_vbatt_max);

static void charger_event_process(struct coul_core_device *chip,
	unsigned int event)
{
	switch (event) {
	case COUL_CHG_START_EVENT:
		coul_info("receive charge start event = 0x%x\n", event);
		/* record soc and cc value */
		chip_lock();
		coul_chg_start(chip);
		chip_unlock();
		break;

	case COUL_CHG_STOP_EVENT:
		coul_info("receive charge stop event = 0x%x\n", event);
		chip_lock();
		coul_chg_stop(chip);
		chip_unlock();
		break;

	case COUL_CHG_FULL_EVENT:
		coul_info("receive charge full event = 0x%x\n", (int)event);
		chip_lock();
		coul_chg_full(chip);
		chip_unlock();
		break;

	default:
		chip->common_data.charging_state = COUL_CHG_STATE_UNKNOW;
		coul_err("unknown event %d\n", (int)event);
		break;
	}
}

void coul_intf_resp_charger_event(unsigned int event)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (!chip)
		return;

	coul_intf_lock_f();
	charger_event_process(chip, event);
	coul_intf_unlock_f();
}
EXPORT_SYMBOL_GPL(coul_intf_resp_charger_event);

int coul_intf_read_chg_cycle(void)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (!chip)
		return -EPERM;

	return chip->common_data.charge_cycles / PERCENT;
}
EXPORT_SYMBOL_GPL(coul_intf_read_chg_cycle);

int coul_intf_read_batt_vol_uv(void)
{
	int vbat_uv = 0;
	struct coul_core_device *chip = get_coul_core_dev();

	if ((!chip) || IS_IC_OPS_NULL(chip, get_batt_vol)) {
		coul_err("NULL point\n");
		return -EPERM;
	}

	vbat_uv = chip->core_ic_ops->get_batt_vol();

	return vbat_uv;
}
EXPORT_SYMBOL_GPL(coul_intf_read_batt_vol_uv);

int coul_intf_read_batt_cap(void)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (!chip) {
		coul_err("error, chip is NULL, return default exist\n");
		return -EPERM;
	}

	coul_debug("ui_soc = %d\n", chip->common_data.ui_soc);

	return chip->common_data.ui_soc;
}
EXPORT_SYMBOL_GPL(coul_intf_read_batt_cap);

int coul_intf_read_batt_curr(void)
{
	int cur;
	struct coul_core_device *chip = get_coul_core_dev();

	if ((!chip) || IS_IC_OPS_NULL(chip, get_batt_cur)) {
		coul_err("NULL point\n");
		return -EPERM;
	}

	cur = chip->core_ic_ops->get_batt_cur();
	return cur;
}
EXPORT_SYMBOL_GPL(coul_intf_read_batt_curr);

int coul_intf_read_batt_rm(void)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (!chip) {
		coul_err("NULL point\n");
		return -EPERM;
	}

	return chip->common_data.batt_ruc / PERMILLAGE;
}

int coul_intf_read_batt_fcc(void)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (!chip) {
		coul_err("NULL point\n");
		return -EPERM;
	}

	return chip->common_data.batt_fcc / PERMILLAGE;
}
EXPORT_SYMBOL_GPL(coul_intf_read_batt_fcc);

int coul_intf_read_batt_cc(void)
{
	int cc;
	struct coul_core_device *chip = get_coul_core_dev();

	if ((!chip) || IS_IC_OPS_NULL(chip, get_cc)) {
		coul_err("NULL point\n");
		return -EPERM;
	}

	cc = chip->core_ic_ops->get_cc();
	return cc;
}
EXPORT_SYMBOL_GPL(coul_intf_read_batt_cc);

int coul_get_tbatt(void)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (!chip) {
		coul_err("error, chip is NULL, return default temp\n");
		return DEFAULT_TEMP;
	}
	return chip->common_data.batt_temp;
}
EXPORT_SYMBOL_GPL(coul_get_tbatt);

int coul_intf_is_batt_exist(void)
{
	struct coul_core_device *chip = get_coul_core_dev();

	if (!chip) {
		coul_err("NULL point\n");
		return 0;
	}
	return chip->k2u_data.batt_exist;
}
EXPORT_SYMBOL_GPL(coul_intf_is_batt_exist);

int coul_intf_init(void)
{
	coul_intf_init_flag = 1;
	mutex_init(&coul_intf_lock);
	return 0;
}

void coul_intf_exit(void)
{
	coul_intf_init_flag = 0;
	mutex_destroy(&coul_intf_lock);
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("XRing Technologies Co., Ltd");
MODULE_DESCRIPTION("xr coul interface");
