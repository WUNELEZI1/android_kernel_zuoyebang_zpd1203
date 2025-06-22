// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/err.h>
#include <linux/module.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include "hpc_internal.h"

int hpc_smc_set_boot_info(struct hpc_mem_info *hbm, int num)
{
	struct arm_smccc_res res = {0};
	struct hpc_mem_info *info;
	int type;

	if (num > HBM_TYPE_MAX) {
		hpcerr("num %d out of bound, max %d\n", num, HBM_TYPE_MAX);
		return -EFAULT;
	}

	for (type = 0; type < num; type++) {
		info = &hbm[type];
		arm_smccc_smc(FID_BL31_HPC_BOOT_INFO, type, info->size,
			info->pa, 0, 0, 0, 0, &res);
		if (res.a0 != 0) {
			hpcerr("set %d boot info failed\n", type);
			return -EINVAL;
		}
	}

	return 0;
}
EXPORT_SYMBOL_GPL(hpc_smc_set_boot_info);

int hpc_smc_boot(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_HPC_BOOT, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		hpcerr("hpc smc boot failed: %ld\n", res.a0);

	return res.a0;
}
EXPORT_SYMBOL_GPL(hpc_smc_boot);

int hpc_smc_shutdown(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_HPC_SHUTDOWN, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		hpcerr("hpc smc shutdown failed: %ld\n", res.a0);

	return res.a0;
}
EXPORT_SYMBOL_GPL(hpc_smc_shutdown);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC ATF Driver");
MODULE_LICENSE("GPL v2");
