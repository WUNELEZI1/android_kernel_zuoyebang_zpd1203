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

#ifndef _DPU_HW_PREPIPE_TOP_OPS_H_
#define _DPU_HW_PREPIPE_TOP_OPS_H_

#include "dpu_hw_color.h"

/* TODO: set prepipe sbs_en for rch h-extender mode */
void dpu_hw_prepipe_top_feature_enable(struct dpu_hw_blk *hw,
		struct prepq_feature_ctrl *ctrl);
void dpu_hw_prepipe_top_dither_lite_enable(struct dpu_hw_blk *hw,
		bool enable);
void dpu_hw_prepipe_rgb2yuv_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg);
void dpu_hw_prepipe_yuv2rgb_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg);
/* TODO: set non pre alpha param */
void dpu_hw_prepipe_non_prealpha_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg);
void dpu_hw_prepipe_top_flush(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_PREPIPE_TOP_OPS_H_ */
