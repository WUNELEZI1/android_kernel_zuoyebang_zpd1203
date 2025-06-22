// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */
#include <linux/module.h>
#include "xr_pmic_inner.h"

static int xr_pmic_init(void)
{
	int ret;

	ret = xr_pmic_rtc_init();
	if (ret)
		pr_err("xr_pmic_rtc_init fail, ret=%d\n", ret);

	ret = xr_pmic_adc_init();
	if (ret)
		pr_err("xr_pmic_adc_init fail, ret=%d\n", ret);

	ret = xr_pmic_mntn_init();
	if (ret)
		pr_err("xr_pmic_mntn_init fail, ret=%d\n", ret);

	ret = xr_pmic_led_init();
	if (ret)
		pr_err("xr_pmic_led_init fail, ret=%d\n", ret);

	ret = xr_pmic_key_init();
	if (ret)
		pr_err("xr_pmic_key_init fail, ret=%d\n", ret);

	return 0;
}

static void xr_pmic_exit(void)
{
	xr_pmic_key_exit();
	xr_pmic_led_exit();

	xr_pmic_mntn_exit();
	xr_pmic_adc_exit();
	xr_pmic_rtc_exit();
}

module_init(xr_pmic_init);
module_exit(xr_pmic_exit);

MODULE_DESCRIPTION("X-Ring PMIC Driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xr-pmic-spmi");
MODULE_SOFTDEP("pre: xr_vote_mng");
MODULE_SOFTDEP("pre: xsp1");
