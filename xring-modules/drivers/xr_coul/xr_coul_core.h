/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#ifndef _XR_COUL_CORE_H_
#define _XR_COUL_CORE_H_

#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/suspend.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include <soc/xring/xr_coul_interface.h>
#include "xr_battery_profile.h"

#define coul_debug(fmt, args...) \
	pr_debug("[coul_log] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_info(fmt, args...) \
	pr_info("[coul_log] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_warn(fmt, args...) \
	pr_warn("[coul_log] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_err(fmt, args...) \
	pr_err("[coul_log] [%s:%d]" fmt, __func__, __LINE__, ## args)

#define IS_IC_OPS_NULL(_chip, _name) \
	(IS_ERR_OR_NULL(_chip) || IS_ERR_OR_NULL(_chip->core_ic_ops->_name))

#define chip_lock() mutex_lock(&chip->soc_mutex)
#define chip_unlock() mutex_unlock(&chip->soc_mutex)

#define NV_SAVE_SUCCESS         1
#define NV_SAVE_FAIL            0
#define NV_READ_SUCCESS         1
#define NV_READ_FAIL            0

#define TENTH                   10

#define DEFAULT_TEMP            250 /* 25 degrees celsius,set for SFT/UDP */
#define ERR_TEMP               -400 /* -40 degrees celsius,set for SFT/UDP */

#define PERMILLAGE              1000

#define LOW_INT_STATE_RUNNING   1
#define LOW_INT_STATE_SLEEP     0
#define LOW_INT_TEMP_THRED      (-50) /* -5 degrees celsius */
#define BATTERY_CC_WARNING_LEV  10

#define PERCENT                 100

#define COUL_CHG_STATE_UNKNOW   0
#define COUL_CHG_STATE_START    1
#define COUL_CHG_STATE_STOP     2
#define COUL_CHG_STATE_FULL     3

#define UA_PER_MA               1000
#define SEC_PER_HOUR            3600

#define BATTERY_VOL_2_PERCENT   3350

#define VTH_SOC_LEVEL           3
#define WINDOW_LEN              10

struct coul_ic_ops {
	int (*get_cc)(void);
	void (*set_cc)(int cc_uah);
	int (*get_fifo_depth)(void);
	int (*get_nv_read_flag)(void);
	void (*set_nv_save_flag)(int nv_flag);
	int (*get_use_saved_ocv_flag)(void);
	int (*get_ocv_update_fcc_flag)(void);
	int (*set_ocv_update_fcc_flag)(int val);
	void (*set_ocv)(int ocv);
	int (*get_ocv)(void);
	void (*clear_ocv)(void);
	void (*set_ocv_temp)(int ocv_temp);
	int (*get_ocv_temp)(void);
	void (*set_low_vol_alert_thr)(int vol_mv);
	unsigned int (*read_coul_time)(void);
	void (*rst_coul_time)(void);
	void (*rst_cc)(void);
	void (*cali_adc)(void);
	int (*get_batt_vol)(void);
	int (*get_batt_cur)(void);
	void (*get_batt_vol_fifo)(int *data);
	void (*get_batt_cur_fifo)(int *data);
	int (*get_batt_cur_from_fifo)(short fifo_order);
	void (*dump_reg)(void);
	void (*lp_enable)(void);
	int (*get_lp_compensate_cc)(void);
	void (*set_last_soc)(short soc);
	void (*read_last_soc)(short *soc);
	void (*rst_last_soc_flag)(void);
	void (*read_last_soc_flag)(bool *valid);
	void (*set_ocv_level)(int level);
	int (*read_ocv_level)(void);
	void (*set_curr_in_alert_thr)(int ma);
	void (*set_curr_out_alert_thr)(int ma);
	int (*get_adc_tbatt)(void);
	int (*regval_convert_uv)(unsigned int reg_val);
	int (*is_new_battery)(int *new);
};

struct vth_soc_para {
	int soc;
	int vol;
};

struct coul_core_device {
	bool kdata_ready;
	int hal_read_type;
	struct mutex data_lock;
	wait_queue_head_t coul_wait_q;

	struct coul_ic_ops *core_ic_ops;
	struct device *dev;
	struct device *cdev;

	struct mutex soc_mutex;

	int core_init_flag;
	int soc_work_interval;
	int soc_filter_fifo[WINDOW_LEN];
	int soc_filter_count;
	int soc_filter_sum;
	int work_init_flag;
	int resume_cc;
	unsigned int batt_board_id;
	struct delayed_work calc_soc_work;
	struct delayed_work ui_soc_work;
	struct power_supply *battery_psy;

	int vth_soc_en;
	struct vth_soc_para vth_soc_data[VTH_SOC_LEVEL];

	int shutdown_soc_en;
	int delta_soc;
	unsigned int low_vbatt_filter_cnt;
	int v_poweroff_low_temp;
	unsigned int v_poweroff;
	unsigned int v_poweroff_sleep;
	int nv_readable_flag;

	struct coul_k2u_data k2u_data;
	struct coul_common_data common_data;
};

void coul_chg_start(struct coul_core_device *chip);
void coul_chg_stop(struct coul_core_device *chip);
void coul_chg_full(struct coul_core_device *chip);
struct coul_core_device *get_coul_core_dev(void);
int coul_core_init(void);
void coul_core_exit(void);
#endif
