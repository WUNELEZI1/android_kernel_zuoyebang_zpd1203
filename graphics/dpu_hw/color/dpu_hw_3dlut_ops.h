/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DPU_HW_POST_3DLUT_OPS_H_
#define _DPU_HW_POST_3DLUT_OPS_H_

#include "dpu_hw_color.h"

void dpu_hw_3dlut_enable(struct dpu_hw_blk *hw, bool enable);
void dpu_hw_post_3dlut_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);
void dpu_hw_pre_3dlut_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg);
void dpu_hw_3dlut_flush(struct dpu_hw_blk *hw, bool use_cpu);
void dpu_hw_3dlut_get(struct dpu_hw_blk *hw,
		void *data);

#endif /* _DPU_HW_POST_3DLUT_OPS_H_ */
