/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __STEP_CHG_H__
#define __STEP_CHG_H__

#include <linux/iio/consumer.h>
#include "smb5-iio.h"
#include "smb5-lib.h"

#define CYCLE_COUNT_VOTER          "CYCLE_COUNT_VOTER"
#define STEP_CHG_VOTER             "STEP_CHG_VOTER"
#define JEITA_VOTER                "JEITA_VOTER"
#define JEITA_FCC_SCALE_VOTER      "JEITA_FCC_SCALE_VOTER"
#define BATT_VERIFED_VOTER         "BATT_VERIFED_VOTER"
#define BATT_PROFILE_VOTER         "BATT_PROFILE_VOTER"
#define JEITA_FCC_TAPER_VOTER      "JEITA_FCC_TAPER_VOTER"
#define JEITA_AWAKE_VOTER          "JEITA_AWAKE_VOTER"
#define HIGH_TEMP_OV_VOTER         "HIGH_TEMP_OV_VOTER"

#define TAPERED_STEP_CHG_FCC_REDUCTION_STEP_MA   50000 /* 50 mA */
#define MAX_STEP_CHG_ENTRIES                     8
#define BATT_COOL_THRESHOLD                      150
#define BATT_WARM_THRESHOLD                      480
#define FFC_CHG_TERM_TEMP_THRESHOLD              350
#define SIN_COS_FFC_LOW_TEMP_CHG_TERM_CURRENT   -950
#define SIN_ATL_FFC_LOW_TEMP_CHG_TERM_CURRENT   -1150
#define DBL_SWD_FFC_LOW_TEMP_CHG_TERM_CURRENT   -1350
#define DBL_ATL_FFC_LOW_TEMP_CHG_TERM_CURRENT   -1500
#define SIN_COS_FFC_HIGH_TEMP_CHG_TERM_CURRENT  -1350
#define SIN_ATL_FFC_HIGH_TEMP_CHG_TERM_CURRENT  -1450
#define DBL_SWD_FFC_HIGH_TEMP_CHG_TERM_CURRENT  -1700
#define DBL_ATL_FFC_HIGH_TEMP_CHG_TERM_CURRENT  -2050
#define ERR_CHG_TERM_CURRENT                    -2680 //mA

#define BATT_ID_SIN_COS				1
#define BATT_ID_SIN_NVT				2
#define BATT_ID_DBL_SWD				3
#define BATT_ID_DBL_NVT				4

#define BATT_HOT_DECIDEGREE_MAX           600
#define GET_CONFIG_DELAY_MS               2000
#define GET_CONFIG_RETRY_COUNT            50
#define WAIT_BATT_ID_READY_MS             200
#define BATT_LOWTEMP_CURRENT              687000
#define BATT_UNVERIFED_CURRENT            3600000  //uA
#define FFC_FV_MAX_VOLTAGE                4780000  //uV
#define NORMAL_FV_MAX_VOLTAGE             4560000  //uV
#define FFC_FV_LIMIT_VOLTAGE              4230000  //uV
#define FFC_FV_FALL_HYS                   10000    //uV
#define FFC_FV_RISE_HYS                   10000    //uV
#define FFC_FCC_LIMIT_CURRENT             5400000  //uA
#define FFC_TAPER_MAX_CURRENT             3000000  //uA
#define HIGH_SOC_MAX_CURRENT              2000000  //uA
#define FFC_HIGHT_SOC_LIMIT_CURRENT       1000000   //uA
#define FFC_TAPER_STEP                    100000   //uA
#define FFC_TAPER_STEP1                   10000    //uA
#define FFC_TAPER_ITERM_RISE_HYS          80000    //uA
#define FFC_FG_FULL_RISE_HYS              100000   //uA
#define FFC_BATT_OV_RISE_HYS              200000   //uA
#define FFC_TAPER_DELAY_MS                2000     //2s
#define JEITA_WORK_MS                     5000     //5s
#define JEITA_HIGH_TEMP_FV_UV             4300000  //uV
#define JEITA_HIGH_TEMP_TARGER_FV_UV      4100000  //uV
#define JEITA_HIGH_TEMP_RECHG_UV          4000000  //uV
#define FFC_TAPER_OV_FAIL_HYS             2000     //uV
#define JEITA_SUSPEND_HYST_UV             50000
#define DT_FCC_TEMP_15_35                 4
#define DT_FCC_TEMP_35_48                 5
#define DT_FCC_TEMP_45_60                 7

struct step_chg_jeita_param {
	u32			psy_prop;
	u32			iio_prop;
	char			*prop_name;
	int			rise_hys;
	int			fall_hys;
	bool			use_bms;
};

struct range_data {
	int low_threshold;
	int high_threshold;
	u32 value;
};

extern struct smb_charger *g_chg;
int qcom_step_chg_init(struct device *dev, bool step_chg_enable,
	bool sw_jeita_enable, bool jeita_arb_en, struct iio_channel *iio_chans);
void qcom_step_chg_deinit(void);
#endif /* __STEP_CHG_H__ */
