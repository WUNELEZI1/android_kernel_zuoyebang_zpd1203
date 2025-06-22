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

#ifndef _DPU_COLOR_INTERNAL_H_
#define _DPU_COLOR_INTERNAL_H_

#include "osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_color.h"
#include "XRDpuColor.h"

/* post pq hw modules */
enum postpq_modules {
	POSTPQ_PRE_PROC,
	POSTPQ_ACAD,
	POSTPQ_TOP,
	POSTPQ_DPP_3DLUT,
	POSTPQ_DPP_GAMMA,
	POSTPQ_HIST,
	POSTPQ_SCALER,
	POSTPQ_RC,
	POSTPQ_MODULE_MAX,
};

/**
 * color_hist_event - the histogram event data structure
 * @enable: enable or disable hist statistics
 * @hist_read_mode: 0 for reading hist data on each eof, 1 for reading once
 * @read_cnt: used in read mode 1
 * @hist_blob: hist data blob
 */
struct color_hist_event {
	bool enable;
	u32 hist_read_mode;
	u32 read_cnt;
};
#endif /* _DPU_COLOR_INTERNAL_H_ */
