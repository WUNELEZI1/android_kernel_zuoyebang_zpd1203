// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */
#include <linux/module.h>
#include "ddr_devfreq/ddr_devfreq_internal.h"
#include "ddr_vote_dev/ddr_vote_dev_internal.h"
#include "ddr_flux/ddr_flux_internal.h"
#include "ddr_info/ddr_info_internal.h"
#undef pr_fmt
#define pr_fmt(fmt)		"ddr_main: " fmt

static int ddr_dev_init(void)
{
	int ret;
	int ret_final = 0;

	/* user + eng */
	ret = ddr_devfreq_init();
	if (ret != 0) {
		pr_err("ddr_devfreq_init fail, ret = %d\n", ret);
		ret_final = ret;
	}

	ret = ddr_vote_init();
	if (ret != 0) {
		pr_err("ddr_vote_init fail, ret = %d\n", ret);
		ret_final = ret;
	}

	ret = ddr_flux_init();
	if (ret != 0) {
		pr_err("ddr_flux_init fail, ret = %d\n", ret);
		ret_final = ret;
	}

	ret = ddr_info_init();
	if (ret != 0) {
		pr_err("ddr_info_init fail, ret = %d\n", ret);
		ret_final = ret;
	}
	return ret_final;
}

static void ddr_dev_exit(void)
{
	ddr_info_exit();
	ddr_flux_exit();
	ddr_vote_exit();
	ddr_devfreq_exit();
}

module_init(ddr_dev_init);
module_exit(ddr_dev_exit);

MODULE_DESCRIPTION("X-Ring DDR Driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xr_vote_mng");
MODULE_SOFTDEP("pre: perf-actuator");
