/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#ifndef _DPU_DSC_ALGORITHM_H_
#define _DPU_DSC_ALGORITHM_H_

#include "osal.h"
#include "dpu_hw_cap.h"
#include "dpu_hw_common.h"
#include "dpu_hw_dsc_ops.h"

/**
 * dpu_dsc_get_config - get configuration(pps table) of dsc hardware module
 * @in_params: parameters to determine pps table
 * @sink_cap: sink dsc capabilities
 * @src_cap: source dsc capabilities
 * @dsc_config: dsc configuration for dsc hardware module
 *
 * if @sink_cap is NULL, this function will calculate pps table according to
 * @in_params. if not, this function will compare @sink_cap and
 * @src_cap to determine some parameters in in_params.
 */
void dpu_dsc_get_config(struct dsc_parms *in_params,
		struct dsc_caps_sink *sink_cap, struct dsc_caps_src *src_cap,
		struct dpu_dsc_config *dsc_config);

#endif /* _DPU_DSC_HELPER_H_ */
