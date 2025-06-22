// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */

#include <linux/module.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include "xr_bob_regulator.h"
#include "xr_regulator_internal.h"
#include "xsp_ffa.h"

int xr_regulator_ffa_direct_message(struct device *dev, unsigned long fid,
		unsigned long id, unsigned long para, unsigned long *data)
{
	struct xsp_ffa_msg msg;
	int ret = 0;

	memset(&msg, 0, sizeof(msg));

	msg.fid = fid;
	msg.data0 = id;
	msg.data1 = para;

	ret = xrsp_ffa_direct_message(&msg);
	if (ret != 0) {
		dev_err(dev, "ffa message failed, ret = %d\n", ret);
		return ret;
	}
	if (msg.ret != 0) {
		dev_err(dev, "%s: error: ret = %#lx\n", __func__, msg.ret);
		return -EINVAL;
	}

	if (data != NULL)
		*data = msg.data1;

	return 0;
}

static int xr_regulator_init(void)
{
	int ret;

	ret = xring_regulator_ip_init();
	if (ret)
		pr_err("xring_regulator_ip_init fail, ret=%d\n", ret);

	ret = xring_regulator_bob_init();
	if (ret)
		pr_err("xring_regulator_bob_init fail, ret=%d\n", ret);

	ret = xring_regulator_pmic_init();
	if (ret)
		pr_err("xring_regulator_ip_init fail, ret=%d\n", ret);

	return 0;
}

static void xr_regulator_exit(void)
{
	xring_regulator_pmic_exit();

	xring_regulator_bob_exit();

	xring_regulator_ip_exit();
}

module_init(xr_regulator_init);

module_exit(xr_regulator_exit);

MODULE_DESCRIPTION("X-Ring PMIC/IP/Buckboost Regulator Driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xring_i2c");
MODULE_SOFTDEP("pre: trace_hook");
MODULE_SOFTDEP("pre: xr_timestamp");
MODULE_SOFTDEP("pre: xsp1");
