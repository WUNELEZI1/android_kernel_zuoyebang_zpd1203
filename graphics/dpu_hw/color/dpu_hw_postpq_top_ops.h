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

#ifndef _DPU_HW_POSTPQ_TOP_OPS_H_
#define _DPU_HW_POSTPQ_TOP_OPS_H_

#include "dpu_hw_color.h"

void dpu_hw_postpq_top_pipe_init(struct dpu_hw_blk *hw,
		struct postpq_hw_init_cfg *cfg);
void dpu_hw_postpq_top_enable(struct dpu_hw_blk *hw,
		struct postpq_feature_ctrl *ctrl);
void dpu_hw_post_scaler_output_size_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);
void dpu_hw_post_rgb2yuv_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);
void dpu_hw_post_yuv2rgb_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);
void dpu_hw_post_dither_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);
void dpu_hw_csc_matrix_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);
void dpu_hw_post_dither_proc_loc_get(struct dpu_hw_blk *hw, void *loc);
void dpu_hw_post_proc_loc_get(struct dpu_hw_blk *hw, void *loc);
void dpu_hw_postpq_top_flush(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_POSTPQ_TOP_OPS_H_ */
