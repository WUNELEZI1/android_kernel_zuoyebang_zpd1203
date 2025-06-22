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
#include "xrp_hw.h"

int hpc_smc_xrp0_pwon(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_HPC_XRP_PWON, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("hpc xrp0 power on failed: %ld\n", res.a0);

	return res.a0;
}

int hpc_smc_xrp0_halt(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_HPC_XRP_HALT, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("hpc xrp0 halt failed: %ld\n", res.a0);

	return res.a0;
}

int hpc_smc_xrp0_release(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_HPC_XRP_RELEASE, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("hpc xrp0 release failed: %ld\n", res.a0);

	return res.a0;
}

int hpc_smc_xrp0_pwoff(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_HPC_XRP_PWOFF, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("hpc xrp0 power off failed: %ld\n", res.a0);

	return res.a0;
}

int hpc_smc_xrp0_disreset(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_HPC_XRP_DISRESET, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("hpc xrp0 direset failed: %ld\n", res.a0);

	return res.a0;
}

int hpc_smc_xrp0_reset(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_HPC_XRP_RESET, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("hpc xrp0 reset failed: %ld\n", res.a0);

	return res.a0;
}


MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("XRP ATF Driver");
MODULE_LICENSE("GPL v2");
