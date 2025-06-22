// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#include "xr_coul_xp2210.h"
#include "xr_coul_core.h"
#include "xr_coul_nv.h"
#include "xr_battery_profile.h"
#include "xr_coul_sysfs.h"
#include <soc/xring/xr_coul_interface.h>

int __init coul_init(void)
{
	int ret;

	ret = coul_nv_data_initial();
	if (ret) {
		pr_err("%s coul_nv_data_initial fail, ret:%d\n", __func__, ret);
		goto err1;
	}

	ret = coul_ic_init();
	if (ret) {
		pr_err("%s coul_ic_init fail, ret:%d\n", __func__, ret);
		goto err1;
	}

	ret = battery_profile_init();
	if (ret) {
		pr_err("%s battery_profile_init fail, ret:%d\n", __func__, ret);
		goto err2;
	}

	ret = coul_core_init();
	if (ret) {
		pr_err("%s coul_core_init fail, ret:%d\n", __func__, ret);
		goto err3;
	}

	coul_intf_init();

	coul_sysfs_init();

	return 0;
err3:
	battery_profile_exit();
err2:
	coul_ic_exit();
err1:
	return 0;
}

void __exit coul_exit(void)
{
	coul_sysfs_exit();

	coul_intf_exit();

	coul_core_exit();

	battery_profile_exit();

	coul_ic_exit();

	coul_nv_data_exit();
}

fs_initcall(coul_init);

module_exit(coul_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("XRing Technologies Co., Ltd");
MODULE_DESCRIPTION("xr coul init driver");
