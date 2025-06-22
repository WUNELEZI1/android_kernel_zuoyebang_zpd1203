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

#ifndef _DPU_POST_COLOR_H_
#define _DPU_POST_COLOR_H_

#include "dpu_color_internal.h"
#include "dpu_hw_postpq_top.h"
#include "dpu_hw_postpre_proc.h"
#include "dpu_hw_post_scaler.h"
#include "dpu_hw_post_3dlut.h"
#include "dpu_hw_post_acad.h"
#include "dpu_hw_post_gamma.h"
#include "dpu_hw_post_hist.h"
#include "dpu_hw_tm.h"

struct composer;

/* callback functions for registering the postPQ module to Mixer */
struct color_postpq_funcs {
	/**
	 * commit - commit properties to HW modules
	 * @comp: pointer to struct composer
	 * Return: zero on success, -1 on failure
	 */
	int (*commit)(struct composer *comp);

	/**
	 * pipe_init - init post pq pipe
	 * @comp: pointer to struct composer
	 * Return: zero on success, -1 on failure
	 */
	int (*pipe_init)(struct composer *comp);
};

/**
 * color_postpq - the postpq color data structure
 * @hw_gamma: point to post gamma object
 * @hw_3dlut: point to post 3dlut object
 * @hw_pq_top: point to post pq_top object
 * @hw_pre_proc: point to post pre proc object
 * @hw_acad: point to acad object
 * @hw_hist: point to post hist object
 * @hw_scaler: point to post scaler object
 * @feature_list: post pq feature list
 * @enabled_features: record all the features enabled
 * @funcs: the callback functions for postpq
 * @scaler: point to post scl param struct
 * @enable: postpq enable flag
 */
struct color_postpq {
	struct dpu_hw_post_pipe_top *hw_post_pipe_top;
	struct dpu_hw_post_gamma *hw_gamma;
	struct dpu_hw_post_3dlut *hw_3dlut;
	struct dpu_hw_postpq_top *hw_pq_top;
	struct dpu_hw_postpre_proc *hw_pre_proc;
	struct dpu_hw_post_acad *hw_acad;
	struct dpu_hw_post_hist *hw_hist;
	struct dpu_hw_post_scaler *hw_scaler;

	struct list_head feature_list;
	u32 enabled_features;
	struct color_postpq_funcs *funcs;

	struct color_hist_event hist_event;

	bool enable;
	bool dither_lite_en;
	bool r2y_en;
	bool y2r_en;
	dpu_ltm_param_t *ltm;
	dpu_acad_param_t *acad;
	dpu_matrix_3x4_param_t *mulmatrix;
	dpu_csc_matrix_param_t *r2y;
	dpu_csc_matrix_param_t *y2r;
	dpu_scl2d_param_t *scaler;
	dpu_lut3d_param_t *lut3d;
	dpu_matrix_3x3_param_t *cscmatrix;
	dpu_gamma_param_t *gma;
	dpu_dither_param_t *dither;
	dpu_hist_param_t *hist;
	dpu_rc_param_t *rc;
};

void dpu_mixer_color_postpq_init(struct color_postpq *postpq);
#endif
