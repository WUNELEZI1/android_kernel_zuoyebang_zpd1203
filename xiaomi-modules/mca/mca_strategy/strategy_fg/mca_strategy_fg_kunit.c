// SPDX-License-Identifier: GPL-2.0
/*
 *mca_strategy_fg_kunit.c
 *
 * Kunit test case about strategy fg
 *
 * Copyright (c) 2023-2023 Xiaomi Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <kunit/test.h>
#include <mca/common/mca_log.h>
#include "inc/strategy_fg.h"

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_strategy_fg_kunit"
#endif

static  void strategy_fg_record_volt_mean_test(struct kunit *test)
{
	struct strategy_fg fg;
	int vol_data[7] = {0, 3400, 3500, 3600, 3700, 3800, 3900};

	fg.fg_init_flag = 1;

	for (int i = 0; i < 7; i++) {
		fg.batt_voltage = vol_data[i];
		strategy_fg_record_volt_mean(&fg);
		mca_log_info("vol %d, batt_voltage_mean %d", fg.batt_voltage, fg.batt_voltage_mean);
		if (i == 0)
			KUNIT_EXPECT_EQ(test, true,  fg.batt_voltage_mean == 0);
		else
			KUNIT_EXPECT_EQ(test, true,  fg.batt_voltage_mean >= 3400 &&  fg.batt_voltage_mean <= 3900);
		}
}
static struct kunit_case mca_strategy_fg_test_case[] = {
	KUNIT_CASE(strategy_fg_record_volt_mean_test),
	{}
};
static struct kunit_suite mca_strategy_fg_test_suite = {
	.name = "mca_strategy_fg_test",
	.test_cases = mca_strategy_fg_test_case,
};

kunit_test_suite(mca_strategy_fg_test_suite);

MODULE_DESCRIPTION("Strategy Fg  Kunit");
MODULE_LICENSE("liupengfei9@xiaomi.com");
MODULE_LICENSE("GPL v2");
