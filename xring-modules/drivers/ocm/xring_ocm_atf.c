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
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include "xring_ocm_internal.h"

int ocm_buf_set_attr(struct ocm_buffer *buf, enum ocm_buf_attr attr)
{
	struct arm_smccc_res res = {0};
	u32 right_shift_buf_addr = buf->addr >> PAGE_SHIFT;
	u32 right_shift_buf_size = buf->size >> PAGE_SHIFT;

	arm_smccc_smc(FID_BL31_OCM_SET_ATTR, right_shift_buf_addr, right_shift_buf_size, attr, 0, 0, 0, 0, &res);

	if (res.a0 != 0)
		ocmerr("ocm buf smc set attr failed: %lu\n", res.a0);

	return res.a0;
}
EXPORT_SYMBOL_GPL(ocm_buf_set_attr);

int ocm_buf_clr_attr(struct ocm_buffer *buf, enum ocm_buf_attr attr)
{
	struct arm_smccc_res res = {0};
	u32 right_shift_buf_addr = buf->addr >> PAGE_SHIFT;
	u32 right_shift_buf_size = buf->size >> PAGE_SHIFT;

	arm_smccc_smc(FID_BL31_OCM_CLR_ATTR, right_shift_buf_addr, right_shift_buf_size, attr, 0, 0, 0, 0, &res);

	if (res.a0 != 0)
		ocmerr("ocm buf smc clr attr failed: %lu\n", res.a0);

	return res.a0;
}
EXPORT_SYMBOL_GPL(ocm_buf_clr_attr);
