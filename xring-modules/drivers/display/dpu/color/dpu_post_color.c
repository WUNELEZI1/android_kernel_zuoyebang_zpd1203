// SPDX-License-Identifier: GPL-2.0-only
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

#include <display/xring_dpu_color.h>
#include "dpu_post_color.h"
#include "dpu_crtc.h"
#include "dpu_kms.h"
#include "dpu_log.h"
#include "dpu_hw_intr_ops.h"
#include "dpu_power_mgr.h"

static void color_postpq_prop_node_attach(struct color_postpq *postpq,
		struct postpq_property_node *pnode, u32 feature, u32 blob_sz,
		struct drm_property *property)
{
	pnode->feature = feature;
	pnode->prop_id = property->base.id;
	pnode->prop_flags = property->flags;
	pnode->blob_size = blob_sz;
	INIT_LIST_HEAD(&pnode->dirty_head);
	INIT_LIST_HEAD(&pnode->history_head);

	list_add_tail(&pnode->feature_head, &postpq->feature_list);
}

static void color_crtc_create_blob_property(struct drm_crtc *crtc,
		struct color_postpq *postpq, char *name, u32 feature, u32 blob_sz)
{
	struct postpq_property_node *pnode;
	struct drm_property *property;

	pnode = kzalloc(sizeof(*pnode), GFP_KERNEL);
	if (!pnode)
		return;

	property = drm_property_create(crtc->dev, DRM_MODE_PROP_BLOB, name, 0);
	if (!property) {
		DPU_ERROR("drm_property_create fail: %s\n", name);
		kfree(pnode);
		return;
	}

	drm_object_attach_property(&crtc->base, property, 0);

	color_postpq_prop_node_attach(postpq, pnode, feature, blob_sz, property);
}

static void color_crtc_create_range_property(struct drm_crtc *crtc,
		struct color_postpq *postpq, char *name, u32 feature,
		uint64_t min, uint64_t max)
{
	struct postpq_property_node *pnode;
	struct drm_property *property;

	pnode = kzalloc(sizeof(*pnode), GFP_KERNEL);
	if (!pnode)
		return;

	property = drm_property_create_range(crtc->dev, 0,
			name, min, max);
	if (!property) {
		DPU_ERROR("drm_property_create_range fail: %s\n", name);
		kfree(pnode);
		return;
	}

	drm_object_attach_property(&crtc->base, property, 0);

	color_postpq_prop_node_attach(postpq, pnode, feature, 0, property);
}

static void postpq_ltm_create_property(
		struct drm_crtc *crtc, struct color_postpq *postpq)
{
	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_LTM",
			DPU_CRTC_POSTPQ_LTM, sizeof(struct dpu_ltm_cfg));

	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_MUL_MATRIX",
		DPU_CRTC_POSTPQ_MUL_MATRIX, sizeof(struct dpu_post_matrix_3x4_cfg));

	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_LTM_EN",
			DPU_CRTC_POSTPQ_LTM_EN, 0, 1);
	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_MUL_MATRIX_EN",
			DPU_CRTC_POSTPQ_MUL_MATRIX_EN, 0, 1);
}

static void postpq_acad_create_property(
		struct drm_crtc *crtc, struct color_postpq *postpq)
{
	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_ACAD",
			DPU_CRTC_POSTPQ_ACAD, sizeof(struct dpu_acad_cfg));

	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_ACAD_EN",
			DPU_CRTC_POSTPQ_ACAD_EN, 0, 1);
}

static void postpq_dpp_3dlut_create_property(
		struct drm_crtc *crtc, struct color_postpq *postpq)
{
	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_3DLUT",
			DPU_CRTC_POSTPQ_3DLUT, sizeof(struct dpu_lut_3d_cfg));

	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_3DLUT_EN",
			DPU_CRTC_POSTPQ_3DLUT_EN, 0, 1);
}

static void postpq_pq_top_create_property(
		struct drm_crtc *crtc, struct color_postpq *postpq)
{
	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_CSC_MATRIX",
			DPU_CRTC_POSTPQ_CSC_MATRIX, sizeof(struct dpu_post_matrix_3x3_cfg));
	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_DITHER",
			DPU_CRTC_POSTPQ_DITHER, sizeof(struct dpu_dither_cfg));
	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_R2Y",
			DPU_CRTC_POSTPQ_R2Y, sizeof(struct dpu_csc_matrix_cfg));
	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_Y2R",
			DPU_CRTC_POSTPQ_Y2R, sizeof(struct dpu_csc_matrix_cfg));

	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_DITHER_LITE_EN",
			DPU_CRTC_POSTPQ_DITHER_LITE_EN, 0, 1);
	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_R2Y_EN",
			DPU_CRTC_POSTPQ_R2Y_EN, 0, 1);
	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_Y2R_EN",
			DPU_CRTC_POSTPQ_Y2R_EN, 0, 1);
	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_CSC_MATRIX_EN",
			DPU_CRTC_POSTPQ_CSC_MATRIX_EN, 0, 1);
	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_DITHER_EN",
			DPU_CRTC_POSTPQ_DITHER_EN, 0, 1);

}

static void postpq_dpp_gamma_create_property(
		struct drm_crtc *crtc, struct color_postpq *postpq)
{
	color_crtc_create_blob_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_USR_GAMMA",
			DPU_CRTC_POSTPQ_USR_GAMMA, sizeof(struct dpu_gamma_cfg));

	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_USR_GAMMA_EN",
			DPU_CRTC_POSTPQ_USR_GAMMA_EN, 0, 1);
}

static void postpq_hist_create_property(
		struct drm_crtc *crtc, struct color_postpq *postpq)
{
	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_HIST",
			DPU_CRTC_POSTPQ_HIST, sizeof(struct dpu_hist_cfg));

	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_HIST_READ_MODE",
			DPU_CRTC_POSTPQ_HIST_READ_MODE, 0, HIST_READ_MODE_MAX);
}

static void postpq_scaler_create_property(
		struct drm_crtc *crtc, struct color_postpq *postpq)
{
	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_SCALER",
			DPU_CRTC_POSTPQ_SCALER, sizeof(struct dpu_2d_scaler_cfg));

	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_SCALER_EN",
			DPU_CRTC_POSTPQ_SCALER_EN, 0, 1);
}

static void postpq_rc_create_property(
		struct drm_crtc *crtc, struct color_postpq *postpq)
{
	color_crtc_create_blob_property(crtc, postpq, "DPU_CRTC_POSTPQ_RC",
			DPU_CRTC_POSTPQ_RC, sizeof(struct dpu_rc_cfg));

	color_crtc_create_range_property(crtc, postpq,
			"DPU_CRTC_POSTPQ_RC_REGION",
			DPU_CRTC_POSTPQ_RC_REGION, 0, U64_MAX);
}

typedef void (*postpq_prop_create_func_t)(struct drm_crtc *crtc,
		struct color_postpq *postpq);
static postpq_prop_create_func_t postpq_prop_create_func[POSTPQ_MODULE_MAX];

#define postpq_prop_create_funcs_init(func) \
do { \
	func[POSTPQ_PRE_PROC] = postpq_ltm_create_property; \
	func[POSTPQ_ACAD] = postpq_acad_create_property; \
	func[POSTPQ_TOP] = postpq_pq_top_create_property;\
	func[POSTPQ_DPP_3DLUT] = postpq_dpp_3dlut_create_property; \
	func[POSTPQ_DPP_GAMMA] = postpq_dpp_gamma_create_property; \
	func[POSTPQ_HIST] = postpq_hist_create_property; \
	func[POSTPQ_SCALER] = postpq_scaler_create_property; \
	func[POSTPQ_RC] = postpq_rc_create_property; \
} while (0)

/* module commit funcs defined here */
static void postpq_usr_gamma_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_gamma *gamma = postpq->hw_gamma;

	if (!gamma)
		return;

	if (!postpq->usrgma_tag.switched)
		postpq->usrgma_tag.pong = !postpq->usrgma_tag.pong;

	gamma->ops.set_gamma(&gamma->hw, hw_cfg);
}

static void postpq_usr_gamma_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_gamma *gamma = postpq->hw_gamma;
	u32 feature = BIT(DPU_CRTC_POSTPQ_USR_GAMMA_EN);

	if (!gamma)
		return;

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
		gamma->ops.enable(&gamma->hw, true);
	} else {
		postpq->enabled_features &= ~feature;
		gamma->ops.enable(&gamma->hw, false);
	}
}

static void postpq_3dlut_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_3dlut *lut_3d = postpq->hw_3dlut;

	if (!lut_3d)
		return;

	if (!postpq->lut3d_tag.switched)
		postpq->lut3d_tag.pong = !postpq->lut3d_tag.pong;

	lut_3d->ops.set_3dlut(&lut_3d->hw, hw_cfg);
}

static void postpq_3dlut_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_3dlut *lut_3d = postpq->hw_3dlut;
	u32 feature = BIT(DPU_CRTC_POSTPQ_3DLUT_EN);

	if (!lut_3d)
		return;

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
		lut_3d->ops.enable(&lut_3d->hw, true);
	} else {
		postpq->enabled_features &= ~feature;
		lut_3d->ops.enable(&lut_3d->hw, false);
	}
}

static void postpq_r2y_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top)
		return;

	postpq_top->ops.set_r2y(&postpq_top->hw, hw_cfg);
}

static void postpq_r2y_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_R2Y_EN);

	if (hw_cfg->value > 0)
		postpq->enabled_features |= feature;
	else
		postpq->enabled_features &= ~feature;
}

static void postpq_scaler_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;
	struct dpu_hw_post_scaler *scaler = postpq->hw_scaler;

	if (!postpq_top || !scaler)
		return;

	postpq_top->ops.set_scaler_output_size(&postpq_top->hw, hw_cfg);
	scaler->ops.set(&scaler->hw, hw_cfg);
}

static void postpq_scaler_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;
	struct dpu_hw_post_scaler *scaler = postpq->hw_scaler;
	u32 feature = BIT(DPU_CRTC_POSTPQ_SCALER_EN);

	if (!postpq_top || !scaler)
		return;

	postpq_top->ops.set_scaler_output_size(&postpq_top->hw, hw_cfg);

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
		scaler->ops.enable(&scaler->hw, true);
	} else {
		postpq->enabled_features &= ~feature;
		scaler->ops.enable(&scaler->hw, false);
	}
}

static void postpq_dither_lite_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_DITHER_LITE_EN);

	if (hw_cfg->value > 0)
		postpq->enabled_features |= feature;
	else
		postpq->enabled_features &= ~feature;
}

static void postpq_y2r_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top)
		return;

	postpq_top->ops.set_y2r(&postpq_top->hw, hw_cfg);
}

static void postpq_y2r_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_Y2R_EN);

	if (hw_cfg->value > 0)
		postpq->enabled_features |= feature;
	else
		postpq->enabled_features &= ~feature;
}

static void postpq_csc_matrix_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top)
		return;

	postpq_top->ops.set_csc_matrix(&postpq_top->hw, hw_cfg);
}

static void postpq_csc_matrix_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_CSC_MATRIX_EN);

	if (hw_cfg->value > 0)
		postpq->enabled_features |= feature;
	else
		postpq->enabled_features &= ~feature;
}

static void postpq_dither_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top)
		return;

	postpq_top->ops.set_dither(&postpq_top->hw, hw_cfg);
}

static void postpq_dither_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_DITHER_EN);

	if (hw_cfg->value > 0)
		postpq->enabled_features |= feature;
	else
		postpq->enabled_features &= ~feature;
}

static void postpq_ltm_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpre_proc *pre_proc = postpq->hw_pre_proc;

	if (!pre_proc)
		return;

	pre_proc->ops.set_eotf_oetf(&pre_proc->hw, hw_cfg);
}

static void postpq_ltm_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_LTM_EN);

	if (hw_cfg->value > 0)
		postpq->enabled_features |= feature;
	else
		postpq->enabled_features &= ~feature;
}

static void postpq_acad_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_acad *acad = postpq->hw_acad;

	if (!acad)
		return;

	acad->ops.set_acad(&acad->hw, hw_cfg);
}

static void postpq_acad_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_acad *acad = postpq->hw_acad;
	u32 feature = BIT(DPU_CRTC_POSTPQ_ACAD_EN);

	if (!acad)
		return;

	if (hw_cfg->value > 0) {
		if (!(postpq->enabled_features & feature))
			DPU_COLOR_INFO("enable acad");

		postpq->enabled_features |= feature;
	} else {
		if (postpq->enabled_features & feature)
			DPU_COLOR_INFO("disable acad");

		postpq->enabled_features &= ~feature;
		acad->ops.enable(&acad->hw, false);
	}
}

static void postpq_mul_matrix_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpre_proc *pre_proc = postpq->hw_pre_proc;

	if (!pre_proc)
		return;

	pre_proc->ops.set_mul_matrix(&pre_proc->hw, hw_cfg);
}

static void postpq_mul_matrix_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_MUL_MATRIX_EN);

	if (hw_cfg->value)
		postpq->enabled_features |= feature;
	else
		postpq->enabled_features &= ~feature;
}

static void postpq_hist_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_hist *hist = postpq->hw_hist;

	if (!hist)
		return;

	hist->ops.set_hist(&hist->hw, hw_cfg);
}

static void postpq_hist_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_hist *hist = postpq->hw_hist;
	u32 feature = BIT(DPU_CRTC_POSTPQ_HIST_READ_MODE);
	u32 read_mode;

	if (!hist)
		return;

	read_mode = hw_cfg->value;

	if (postpq->hist_event.hist_read_mode != read_mode ||
			need_restore_register_after_idle(crtc))
		hist->ops.enable_update(&hist->hw);

	postpq->hist_event.hist_read_mode = read_mode;

	if (read_mode == HIST_READ_DISABLE) {
		postpq->enabled_features &= ~feature;
		hist->ops.enable(&hist->hw, false);
		postpq->hist_event.enable = false;
	} else {
		postpq->enabled_features |= feature;
		hist->ops.enable(&hist->hw, true);
		postpq->hist_event.enable = true;
		if (postpq->hist_event.hist_read_mode == HIST_READ_ONCE)
			atomic_inc(&postpq->hist_event.read_cnt);
	}
}

static void postpq_rc_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_pipe_top *hw_post_pipe_top;
	struct dpu_crtc *dpu_crtc;

	dpu_crtc = to_dpu_crtc(crtc);
	hw_post_pipe_top = dpu_crtc->hw_post_pipe_top;

	if (!hw_post_pipe_top)
		return;

	hw_post_pipe_top->ops.rc_stream_config(&hw_post_pipe_top->hw,
			hw_cfg->data);
	hw_post_pipe_top->ops.rc_config(&hw_post_pipe_top->hw,
			hw_cfg->data);
}

static void postpq_rc_conifg_commit(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_pipe_top *hw_post_pipe_top;
	struct dpu_crtc *dpu_crtc;

	dpu_crtc = to_dpu_crtc(crtc);
	hw_post_pipe_top = dpu_crtc->hw_post_pipe_top;

	if (!hw_post_pipe_top)
		return;

	hw_post_pipe_top->ops.rc_config(&hw_post_pipe_top->hw,
			hw_cfg->data);
}

static void postpq_rc_enable(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg)
{
	if (hw_cfg->value > 0) {
		postpq->enabled_features |= BIT(DPU_CRTC_POSTPQ_RC_REGION);
		postpq->rc_region.x = hw_cfg->value;
		postpq->rc_region.y = hw_cfg->value >> 16;
		postpq->rc_region.w = hw_cfg->value >> 32;
		postpq->rc_region.h = hw_cfg->value >> 48;
	} else {
		postpq->enabled_features &= ~BIT(DPU_CRTC_POSTPQ_RC_REGION);
		postpq->rc_region.x = 0;
		postpq->rc_region.y = 0;
		postpq->rc_region.w = 0;
		postpq->rc_region.h = 0;
	}
}

typedef void (*postpq_feature_commit_func_t)(struct drm_crtc *crtc,
		struct color_postpq *postpq, struct postpq_hw_cfg *hw_cfg);
static postpq_feature_commit_func_t
	postpq_feature_commit_func[DPU_CRTC_POSTPQ_FEATURE_MAX];

#define postpq_feature_commit_funcs_init(func) \
do { \
	func[DPU_CRTC_POSTPQ_USR_GAMMA_EN] = postpq_usr_gamma_enable; \
	func[DPU_CRTC_POSTPQ_USR_GAMMA] = postpq_usr_gamma_commit; \
	func[DPU_CRTC_POSTPQ_3DLUT_EN] = postpq_3dlut_enable; \
	func[DPU_CRTC_POSTPQ_3DLUT] = postpq_3dlut_commit; \
	func[DPU_CRTC_POSTPQ_R2Y_EN] = postpq_r2y_enable; \
	func[DPU_CRTC_POSTPQ_R2Y] = postpq_r2y_commit; \
	func[DPU_CRTC_POSTPQ_SCALER_EN] = postpq_scaler_enable; \
	func[DPU_CRTC_POSTPQ_SCALER] = postpq_scaler_commit; \
	func[DPU_CRTC_POSTPQ_DITHER_LITE_EN] = postpq_dither_lite_enable; \
	func[DPU_CRTC_POSTPQ_Y2R_EN] = postpq_y2r_enable; \
	func[DPU_CRTC_POSTPQ_Y2R] = postpq_y2r_commit; \
	func[DPU_CRTC_POSTPQ_CSC_MATRIX_EN] = postpq_csc_matrix_enable; \
	func[DPU_CRTC_POSTPQ_CSC_MATRIX] = postpq_csc_matrix_commit; \
	func[DPU_CRTC_POSTPQ_DITHER_EN] = postpq_dither_enable; \
	func[DPU_CRTC_POSTPQ_DITHER] = postpq_dither_commit; \
	func[DPU_CRTC_POSTPQ_LTM_EN] = postpq_ltm_enable; \
	func[DPU_CRTC_POSTPQ_LTM] = postpq_ltm_commit; \
	func[DPU_CRTC_POSTPQ_MUL_MATRIX_EN] = postpq_mul_matrix_enable; \
	func[DPU_CRTC_POSTPQ_MUL_MATRIX] = postpq_mul_matrix_commit; \
	func[DPU_CRTC_POSTPQ_ACAD_EN] = postpq_acad_enable; \
	func[DPU_CRTC_POSTPQ_ACAD] = postpq_acad_commit; \
	func[DPU_CRTC_POSTPQ_HIST_READ_MODE] = postpq_hist_enable; \
	func[DPU_CRTC_POSTPQ_HIST] = postpq_hist_commit; \
	func[DPU_CRTC_POSTPQ_RC_REGION] = postpq_rc_enable; \
	func[DPU_CRTC_POSTPQ_RC] = postpq_rc_commit; \
} while (0)

static void postpq_feature_enable(struct drm_crtc *crtc, struct color_postpq *postpq)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;
	struct dpu_hw_postpre_proc *pre_proc = postpq->hw_pre_proc;
	struct dpu_hw_post_pipe_top *hw_post_pipe_top;
	struct postpq_feature_ctrl ctrl_info;
	struct dpu_crtc_state *cstate;
	struct dpu_crtc *dpu_crtc;

	ctrl_info.lp_enable = is_dpu_lp_enabled(DPU_LP_SRAM_LP_ENABLE);
	ctrl_info.enabled_features = postpq->enabled_features;

	if (postpq_top)
		postpq_top->ops.feature_enable(&postpq_top->hw, &ctrl_info);

	if (pre_proc)
		pre_proc->ops.feature_enable(&pre_proc->hw, &ctrl_info);

	dpu_crtc = to_dpu_crtc(crtc);
	cstate = to_dpu_crtc_state(crtc->state);
	hw_post_pipe_top = dpu_crtc->hw_post_pipe_top;

	if (hw_post_pipe_top) {
		if ((postpq->enabled_features & BIT(DPU_CRTC_POSTPQ_RC_REGION)) &&
				cstate->damage_y < postpq->rc_region.y) {
			hw_post_pipe_top->ops.rc_enable(&hw_post_pipe_top->hw,
					crtc->state->mode.vdisplay, crtc->state->mode.hdisplay, true);
		} else {
			hw_post_pipe_top->ops.rc_enable(&hw_post_pipe_top->hw,
					crtc->state->mode.vdisplay, crtc->state->mode.hdisplay, false);
		}
	}

	postpq->lut3d_tag.switched = false;
	postpq->usrgma_tag.switched = false;
}

static void postpq_usr_gamma_flush(struct color_postpq *postpq)
{
	struct dpu_hw_post_gamma *gamma = postpq->hw_gamma;

	if (!gamma)
		return;

	gamma->ops.flush(&gamma->hw, false);
}

static void postpq_3dlut_flush(struct color_postpq *postpq)
{
	struct dpu_hw_post_3dlut *lut_3d = postpq->hw_3dlut;

	if (!lut_3d)
		return;

	lut_3d->ops.flush(&lut_3d->hw, false);
}

static void postpq_top_flush(struct color_postpq *postpq)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top)
		return;

	postpq_top->ops.flush(&postpq_top->hw);
}

static void postpq_pre_proc_flush(struct color_postpq *postpq)
{
	struct dpu_hw_postpre_proc *postpre_proc = postpq->hw_pre_proc;

	if (!postpre_proc)
		return;

	postpre_proc->ops.flush(&postpre_proc->hw);
}

static void postpq_acad_flush(struct color_postpq *postpq)
{
	struct dpu_hw_post_acad *acad = postpq->hw_acad;

	if (!acad)
		return;

	acad->ops.flush(&acad->hw);
}

static void postpq_hist_flush(struct color_postpq *postpq)
{
	struct dpu_hw_post_hist *hist = postpq->hw_hist;

	if (!hist)
		return;

	hist->ops.flush(&hist->hw);
}

static void postpq_scaler_flush(struct color_postpq *postpq)
{
	struct dpu_hw_post_scaler *scaler = postpq->hw_scaler;

	if (!scaler)
		return;

	scaler->ops.flush(&scaler->hw);
}

typedef void (*postpq_module_flush_func_t)(struct color_postpq *postpq);
static postpq_module_flush_func_t
	postpq_module_flush_func[POSTPQ_MODULE_MAX];

#define postpq_module_flush_funcs_init(func) \
do { \
	func[POSTPQ_DPP_GAMMA] = postpq_usr_gamma_flush; \
	func[POSTPQ_DPP_3DLUT] = postpq_3dlut_flush; \
	func[POSTPQ_TOP] = postpq_top_flush; \
	func[POSTPQ_PRE_PROC] = postpq_pre_proc_flush; \
	func[POSTPQ_ACAD] = postpq_acad_flush; \
	func[POSTPQ_HIST] = postpq_hist_flush; \
	func[POSTPQ_SCALER] = postpq_scaler_flush;\
} while (0)

static void postpq_top_init(struct color_postpq *postpq,
		struct postpq_hw_init_cfg *cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top) {
		DPU_COLOR_DEBUG("postpq_top is null\n");
		return;
	}

	postpq_top->ops.init(&postpq_top->hw, cfg);
}

typedef void (*postpq_module_init_func_t)(struct color_postpq *postpq,
		struct postpq_hw_init_cfg *cfg);
static postpq_module_init_func_t postpq_module_init_func[POSTPQ_MODULE_MAX];

#define postpq_module_init_funcs_init(func) ((func)[(POSTPQ_TOP)] = (postpq_top_init))


static struct color_postpq *color_postpq_get(struct drm_crtc *crtc)
{
	struct dpu_crtc *dpu_crtc = NULL;

	if (!crtc) {
		DPU_ERROR("invalid crtc %pK\n", crtc);
		return NULL;
	}

	dpu_crtc = to_dpu_crtc(crtc);

	return dpu_crtc->postpq;
}

static int color_postpq_create_properties(struct drm_crtc *crtc)
{
	struct color_postpq *postpq = color_postpq_get(crtc);
	int i;

	if (!postpq) {
		DPU_ERROR("invalid postpq %pK\n", postpq);
		return -EINVAL;
	}

	for (i = 0; i < POSTPQ_MODULE_MAX; i++)
		if (postpq_prop_create_func[i])
			postpq_prop_create_func[i](crtc, postpq);

	return 0;
}

static int color_postpq_set_prop(struct drm_crtc *crtc,
		struct color_postpq_prop *prop,
		struct postpq_property_node *prop_node,
		uint64_t value)
{
	struct drm_property_blob *blob;

	if (!prop)
		return -EINVAL;

	if (prop->flags & DRM_MODE_PROP_BLOB) {
		blob = drm_property_lookup_blob(crtc->dev, value);
		if (!blob) {
			DPU_ERROR("feature %d: cannot find blob %lld\n",
					prop_node->feature, value);
			return -EINVAL;
		}

		if (blob->length != prop_node->blob_size) {
			DPU_ERROR("feature %d: blob %lld length %zu different from expected %u\n",
					prop_node->feature, value, blob->length, prop_node->blob_size);
			drm_property_blob_put(blob);
			return -EINVAL;
		}

		prop->blob_ptr = blob;
		prop->value = value;
	} else {
		prop->value = value;
	}

	return 0;
}

static int color_postpq_set_property(struct drm_crtc *crtc,
		struct drm_crtc_state *state,
		struct drm_property *property, uint64_t val)
{
	struct color_postpq *postpq = color_postpq_get(crtc);
	struct dpu_crtc_state *cstate;
	struct color_postpq_prop *prop;
	struct color_postpq_state *pstate;
	struct postpq_property_node *prop_node;
	bool found = false;
	int ret;

	if (!postpq || !state || !property) {
		DPU_ERROR("invalid postpq %pK state %pK property %pK\n",
				postpq, state, property);
		return -EINVAL;
	}

	list_for_each_entry(prop_node, &postpq->feature_list, feature_head) {
		if (property->base.id == prop_node->prop_id) {
			found = true;
			break;
		}
	}

	if (!found || prop_node->feature >= DPU_CRTC_POSTPQ_FEATURE_MAX) {
		DPU_WARN("not find feature %d, prop_id %d", prop_node->feature, prop_node->prop_id);
		return -ENOENT;
	}

	cstate = to_dpu_crtc_state(state);
	pstate = &cstate->postpq_state;
	prop = &pstate->prop[prop_node->feature];

	DPU_COLOR_DEBUG("set feature%d's property value %llu\n",
			prop_node->feature, val);

	prop->flags = property->flags;
	ret = color_postpq_set_prop(crtc, prop, prop_node, val);

	if (!ret)
		pstate->dirty[prop_node->feature] = 1;

	return 0;
}

static int color_postpq_check_properties(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	return 0;
}

static int color_postpq_destroy_state(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct color_postpq *postpq = color_postpq_get(crtc);
	struct dpu_crtc_state *cstate;
	struct color_postpq_prop *prop;
	struct color_postpq_state *pstate;
	int i;

	if (!postpq || !state) {
		DPU_ERROR("invalid postpq %pK state %pK\n",
				postpq, state);
		return -EINVAL;
	}

	cstate = to_dpu_crtc_state(state);
	pstate = &cstate->postpq_state;
	for (i = 0; i < DPU_CRTC_POSTPQ_FEATURE_MAX; i++) {
		prop = &pstate->prop[i];
		if (prop->blob_ptr)
			drm_property_blob_put(prop->blob_ptr);
	}
	memset(pstate->prop, 0, sizeof(pstate->prop));

	return 0;
}

static void color_postpq_ping_pong_reset(struct drm_crtc *crtc)
{
	struct color_postpq *postpq = color_postpq_get(crtc);

	postpq->lut3d_tag.pong = 0;
	postpq->usrgma_tag.pong = 0;
}

static int color_postpq_clear(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct color_postpq *postpq = color_postpq_get(crtc);
	struct dpu_crtc_state *cstate;
	struct color_postpq_prop *prop;
	struct color_postpq_state *pstate;
	struct postpq_property_node *prop_node, *next;

	if (!postpq || !state) {
		DPU_ERROR("invalid postpq %pK state %pK\n",
				postpq, state);
		return -EINVAL;
	}

	cstate = to_dpu_crtc_state(state);
	pstate = &cstate->postpq_state;
	list_for_each_entry(prop_node, &postpq->feature_list, feature_head) {
		if (prop_node->blob_ptr) {
			drm_property_blob_put(prop_node->blob_ptr);
			prop_node->blob_ptr = NULL;
		}
		prop_node->prop_val = 0;
		prop_node->is_stored = 0;

		prop = &pstate->prop[prop_node->feature];
		if (prop->blob_ptr)
			drm_property_blob_put(prop->blob_ptr);
	}
	postpq->enabled_features = 0;
	memset(&postpq->lut3d_tag, 0, sizeof(postpq->lut3d_tag));
	memset(&postpq->usrgma_tag, 0, sizeof(postpq->usrgma_tag));
	memset(pstate->prop, 0, sizeof(pstate->prop));

	list_for_each_entry_safe(prop_node, next, &postpq->history_list, history_head) {
		list_del(&prop_node->history_head);
	}

	return 0;
}

static void color_postpq_hw_cfg_populate(
		struct postpq_property_node *prop_node,
		struct drm_crtc_state *state,
		struct postpq_hw_cfg *cfg)
{
	struct drm_property_blob *blob = prop_node->blob_ptr;
	struct dpu_crtc_state *cstate;

	memset(cfg, 0, sizeof(*cfg));
	cstate = to_dpu_crtc_state(state);

	if (prop_node->prop_flags & DRM_MODE_PROP_BLOB) {
		if (!blob)
			return;

		cfg->len = blob->length;
		cfg->data = blob->data;
	} else {
		cfg->value = prop_node->prop_val;
	}

	cfg->lm_width = cstate->lm_width;
	cfg->lm_height = cstate->lm_height;

	cfg->panel_width = state->mode.hdisplay;
	cfg->panel_height = state->mode.vdisplay;
}

static void color_postpq_commit_feature(struct drm_crtc *crtc,
		struct color_postpq *postpq)
{
	struct postpq_property_node *prop_node;
	struct postpq_hw_cfg hw_cfg;

	if (need_restore_register_after_idle(crtc)) {
		list_for_each_entry(prop_node, &postpq->history_list, history_head) {
			color_postpq_hw_cfg_populate(prop_node, crtc->state, &hw_cfg);

			if (postpq_feature_commit_func[prop_node->feature])
				postpq_feature_commit_func[prop_node->feature](crtc,
						postpq, &hw_cfg);
		}
	} else {
		list_for_each_entry(prop_node, &postpq->dirty_list, dirty_head) {
			color_postpq_hw_cfg_populate(prop_node, crtc->state, &hw_cfg);

			if (postpq_feature_commit_func[prop_node->feature])
				postpq_feature_commit_func[prop_node->feature](crtc,
						postpq, &hw_cfg);
		}
	}

	postpq_feature_enable(crtc, postpq);
}

static int to_postpq_module_idx(u32 feature)
{
	int idx = -1;

	switch (feature) {
	case DPU_CRTC_POSTPQ_LTM_EN:
	case DPU_CRTC_POSTPQ_LTM:
	case DPU_CRTC_POSTPQ_MUL_MATRIX_EN:
	case DPU_CRTC_POSTPQ_MUL_MATRIX:
		idx = POSTPQ_PRE_PROC;
		break;
	case DPU_CRTC_POSTPQ_3DLUT_EN:
	case DPU_CRTC_POSTPQ_3DLUT:
		idx = POSTPQ_DPP_3DLUT;
		break;
	case DPU_CRTC_POSTPQ_USR_GAMMA_EN:
	case DPU_CRTC_POSTPQ_USR_GAMMA:
		idx = POSTPQ_DPP_GAMMA;
		break;
	case DPU_CRTC_POSTPQ_HIST_READ_MODE:
	case DPU_CRTC_POSTPQ_HIST:
		idx = POSTPQ_HIST;
		break;
	case DPU_CRTC_POSTPQ_ACAD_EN:
	case DPU_CRTC_POSTPQ_ACAD:
		idx = POSTPQ_ACAD;
		break;
	case DPU_CRTC_POSTPQ_SCALER_EN:
	case DPU_CRTC_POSTPQ_SCALER:
		idx = POSTPQ_SCALER;
		break;
	case DPU_CRTC_POSTPQ_R2Y_EN:
	case DPU_CRTC_POSTPQ_R2Y:
	case DPU_CRTC_POSTPQ_DITHER_LITE_EN:
	case DPU_CRTC_POSTPQ_Y2R_EN:
	case DPU_CRTC_POSTPQ_Y2R:
	case DPU_CRTC_POSTPQ_CSC_MATRIX_EN:
	case DPU_CRTC_POSTPQ_CSC_MATRIX:
	case DPU_CRTC_POSTPQ_DITHER_EN:
	case DPU_CRTC_POSTPQ_DITHER:
		idx = POSTPQ_TOP;
		break;

	default:
		break;
	}

	return idx;
}

static void color_postpq_flush(struct drm_crtc *crtc, struct color_postpq *postpq)
{
	struct postpq_property_node *prop_node;
	bool dirty[POSTPQ_MODULE_MAX];
	int i = 0;
	int idx;

	memset(dirty, 0, sizeof(dirty));

	if (need_restore_register_after_idle(crtc)) {
		list_for_each_entry(prop_node, &postpq->history_list, history_head) {
			idx = to_postpq_module_idx(prop_node->feature);
			if (idx >= 0)
				dirty[idx] = true;
		}
	} else {
		list_for_each_entry(prop_node, &postpq->dirty_list, dirty_head) {
			idx = to_postpq_module_idx(prop_node->feature);
			if (idx >= 0)
				dirty[idx] = true;
		}
	}

	for (i = 0; i < POSTPQ_MODULE_MAX; i++) {
		if (postpq_module_flush_func[i] && dirty[i]) {
			DPU_COLOR_DEBUG("pq module:%d flush\n", i);
			postpq_module_flush_func[i](postpq);
		}
	}
}

static void color_postpq_update_property(struct drm_crtc *dpu_crtc,
		struct postpq_property_node *prop_node,
		struct color_postpq_prop *prop)
{
	if (prop->flags & DRM_MODE_PROP_BLOB) {
		if (prop_node->blob_ptr)
			drm_property_blob_put(prop_node->blob_ptr);

		prop_node->blob_ptr = prop->blob_ptr;
		drm_property_blob_get(prop_node->blob_ptr);
	}
	prop_node->prop_val = prop->value;
}

static int color_postpq_commit(struct drm_crtc *crtc)
{
	struct color_postpq *postpq = color_postpq_get(crtc);
	struct postpq_property_node *prop_node, *next;
	struct color_postpq_state *pstate;
	struct color_postpq_prop *prop;
	struct dpu_crtc_state *cstate;
	struct dpu_crtc *dpu_crtc;

	if (!postpq) {
		DPU_ERROR("invalid postpq %pK\n", postpq);
		return -EINVAL;
	}

	dpu_crtc = to_dpu_crtc(crtc);
	cstate = to_dpu_crtc_state(crtc->state);
	pstate = &cstate->postpq_state;

	list_for_each_entry(prop_node, &postpq->feature_list, feature_head) {
		if (cstate->postpq_state.dirty[prop_node->feature]) {
			prop = &pstate->prop[prop_node->feature];
			color_postpq_update_property(crtc, prop_node, prop);
			list_add_tail(&prop_node->dirty_head, &postpq->dirty_list);
			if (!prop_node->is_stored) {
				list_add_tail(&prop_node->history_head, &postpq->history_list);
				prop_node->is_stored = true;
			}
		}
	}

	color_postpq_commit_feature(crtc, postpq);

	color_postpq_flush(crtc, postpq);

	list_for_each_entry_safe(prop_node, next, &postpq->dirty_list, dirty_head) {
		list_del(&prop_node->dirty_head);
	}

	return 0;
}
#define SKIP_RESTOER_POSTPQ_FEATURES(prop_feature) \
	((prop_feature == DPU_CRTC_POSTPQ_USR_GAMMA) || \
		(prop_feature == DPU_CRTC_POSTPQ_3DLUT) || \
		(prop_feature == DPU_CRTC_POSTPQ_ACAD) || \
		(prop_feature == DPU_CRTC_POSTPQ_ACAD_EN))

static int color_postpq_restore(struct drm_crtc *crtc)
{
	struct color_postpq *postpq = color_postpq_get(crtc);
	struct postpq_property_node *prop_node;
	bool dirty[POSTPQ_MODULE_MAX];
	struct postpq_hw_cfg hw_cfg;
	int i = 0;
	int idx;

	if (!postpq) {
		DPU_ERROR("invalid postpq %pK\n", postpq);
		return -EINVAL;
	}

	memset(dirty, 0, sizeof(dirty));

	DPU_COLOR_DEBUG("restore postpq effect\n");

	list_for_each_entry(prop_node, &postpq->history_list, history_head) {
		if (SKIP_RESTOER_POSTPQ_FEATURES(prop_node->feature))
			continue;

		if (postpq_feature_commit_func[prop_node->feature]) {
			color_postpq_hw_cfg_populate(prop_node, crtc->state, &hw_cfg);

			idx = to_postpq_module_idx(prop_node->feature);
			if (idx >= 0)
				dirty[idx] = true;

			if (prop_node->feature == DPU_CRTC_POSTPQ_RC) {
				postpq_rc_conifg_commit(crtc, postpq, &hw_cfg);
				continue;
			}

			postpq_feature_commit_func[prop_node->feature](crtc,
					postpq, &hw_cfg);
		}
	}

	postpq_feature_enable(crtc, postpq);

	for (i = 0; i < POSTPQ_MODULE_MAX; i++) {
		if (postpq_module_flush_func[i] && dirty[i]) {
			if (i == POSTPQ_DPP_3DLUT) {
				if (!postpq->lut3d_tag.pong)
					continue;
				else
					postpq->lut3d_tag.switched = true;
			}
			if (i == POSTPQ_DPP_GAMMA) {
				if (!postpq->usrgma_tag.pong)
					continue;
				else
					postpq->usrgma_tag.switched = true;
			}

			postpq_module_flush_func[i](postpq);
		}
	}

	return 0;
}

static int color_postpq_get_property(struct drm_crtc *crtc,
		struct drm_property *property, uint64_t *val)
{
	struct postpq_property_node *prop_node;
	struct color_postpq *postpq = color_postpq_get(crtc);
	bool found = false;

	if (!postpq || !property || !val) {
		DPU_ERROR("invalid postpq %pK, property %pK, val %pK\n",
			  postpq, property, val);
		return -EINVAL;
	}

	*val = 0;

	list_for_each_entry(prop_node, &postpq->feature_list, feature_head) {
		if (property->base.id == prop_node->prop_id) {
			*val = prop_node->prop_val;
			found = true;
			break;
		}
	}

	return found ? 0 : -ENOENT;
}

static void color_postpq_module_init(struct color_postpq *postpq,
		struct postpq_hw_init_cfg *cfg)
{
	int i = 0;

	for (i = 0; i < POSTPQ_MODULE_MAX; i++) {
		if (postpq_module_init_func[i]) {
			postpq_module_init_func[i](postpq, cfg);
			postpq_module_flush_func[i](postpq);
		}
	}
}

int color_postpq_pipe_init(struct drm_crtc *crtc)
{
	struct color_postpq *postpq = color_postpq_get(crtc);
	struct postpq_hw_init_cfg cfg;
	u32 lm_width, lm_height;

	if (!postpq) {
		DPU_ERROR("invalid postpq %pK\n", postpq);
		return -EINVAL;
	}

	lm_width = to_dpu_crtc_state(crtc->state)->lm_width;
	lm_height = to_dpu_crtc_state(crtc->state)->lm_height;

	if (lm_width && lm_height) {
		cfg.hdisplay = lm_width;
		cfg.vdisplay = lm_height;
	} else {
		cfg.hdisplay = crtc->state->mode.hdisplay;
		cfg.vdisplay = crtc->state->mode.vdisplay;
	}

	color_postpq_module_init(postpq, &cfg);

	DPU_COLOR_DEBUG("postpq pipe init, display wxh = %d x %d\n",
			cfg.hdisplay, cfg.vdisplay);
	return 0;
}

static void color_postpq_status_dump(struct drm_crtc *crtc)
{
	struct color_postpq *postpq = color_postpq_get(crtc);
	struct dpu_hw_post_scaler *hw_scaler = postpq->hw_scaler;

	if (hw_scaler && (g_dpu_postpq_debug & BIT(DPU_CRTC_POSTPQ_SCALER)))
		hw_scaler->ops.status_dump(&hw_scaler->hw);
}

static void dpu_crtc_color_hist_get(struct drm_crtc *crtc)
{
	struct color_postpq *postpq = color_postpq_get(crtc);
	struct dpu_hw_post_hist *hist = postpq->hw_hist;
	struct dpu_hist_data *hist_data;
	struct drm_event event;

	if (!hist)
		return;

	hist_data = (struct dpu_hist_data *)postpq->hist_event.hist_blob->data;
	memset(hist_data->data, 0, sizeof(hist_data->data));

	DPU_POWER_PROTECT_BEGIN();
	hist->ops.get_hist(&hist->hw, 0, HIST_DATA_SIZE, hist_data);
	DPU_POWER_PROTECT_END();

	event.length = sizeof(u32);
	event.type = DRM_PRIVATE_EVENT_HISTOGRAM;
	dpu_drm_event_notify(crtc->dev,
			&event, (u8 *)(&postpq->hist_event.hist_blob->base.id));

	DPU_COLOR_DEBUG("sent hist blob id 0x%x\n",
			postpq->hist_event.hist_blob->base.id);
}

static int color_postpq_hist_event_cb(struct drm_crtc *crtc)
{
	struct color_postpq *postpq = color_postpq_get(crtc);

	if (!postpq->hist_event.enable)
		return 0;

	if (postpq->hist_event.hist_read_mode == HIST_READ_ONCE) {
		if (atomic_read(&postpq->hist_event.read_cnt) <= 0)
			return 0;

		atomic_dec(&postpq->hist_event.read_cnt);
	}

	dpu_crtc_intr_event_queue(crtc, dpu_crtc_color_hist_get);

	return 0;
}

static int color_postpq_hist_event_enable(struct drm_crtc *crtc,
		bool enable, void *event)
{
	struct color_postpq *postpq = color_postpq_get(crtc);

	atomic_set(&postpq->hist_event.read_cnt, 0);
	postpq->hist_event.hist_read_mode = HIST_READ_DISABLE;
	postpq->hist_event.enable = false;

	DPU_COLOR_DEBUG("%s hist event\n", enable ? "enable" : "disable");
	return 0;
}

static int color_postpq_event_init(struct drm_crtc *crtc, void *event)
{
	struct dpu_crtc_event *color_event = event;

	if (!crtc || !color_event)
		return -EINVAL;

	switch (color_event->event_type) {
	case DRM_PRIVATE_EVENT_HISTOGRAM:
		color_event->intr_id = INTR_ONLINE0_FRM_TIMING_EOF;
		color_event->enable = color_postpq_hist_event_enable;
		color_event->cb = color_postpq_hist_event_cb;
		break;

	default:
		DPU_WARN("unknow color event type %d", color_event->event_type);
		return -ENOENT;
	}

	return 0;
}

struct color_postpq_funcs postpq_funcs = {
	.create_properties = color_postpq_create_properties,
	.set_property = color_postpq_set_property,
	.get_property = color_postpq_get_property,
	.check_properties = color_postpq_check_properties,
	.destroy_state = color_postpq_destroy_state,
	.commit = color_postpq_commit,
	.pipe_init = color_postpq_pipe_init,
	.hw_restore = color_postpq_restore,
	.status_dump = color_postpq_status_dump,
	.event_init = color_postpq_event_init,
	.clear = color_postpq_clear,
	.ping_pong_reset = color_postpq_ping_pong_reset,
};

int dpu_crtc_color_postpq_init(struct drm_crtc *crtc)
{
	struct dpu_crtc *dpu_crtc;
	struct color_postpq *postpq;

	DPU_COLOR_DEBUG("crtc postpq init\n");

	if (!crtc) {
		DPU_ERROR("invalid drm_crtc, %pK\n", crtc);
		return -EINVAL;
	}

	dpu_crtc = to_dpu_crtc(crtc);
	if (!dpu_crtc) {
		DPU_ERROR("invalid dpu_crtc, %pK\n", dpu_crtc);
		return -EINVAL;
	}

	if (dpu_crtc->postpq)
		goto exit;

	postpq = kzalloc(sizeof(*postpq), GFP_KERNEL);
	if (!postpq) {
		DPU_ERROR("alloc postpq fail\n");
		return -ENOMEM;
	}

	postpq->hist_event.enable = 0;
	postpq->hist_event.hist_blob = drm_property_create_blob(crtc->dev,
			sizeof(struct dpu_hist_data), NULL);
	if (IS_ERR(postpq->hist_event.hist_blob))
		postpq->hist_event.hist_blob = NULL;

	INIT_LIST_HEAD(&postpq->feature_list);
	INIT_LIST_HEAD(&postpq->dirty_list);
	INIT_LIST_HEAD(&postpq->history_list);
	postpq->funcs = &postpq_funcs;

	postpq_prop_create_funcs_init(postpq_prop_create_func);
	postpq_feature_commit_funcs_init(postpq_feature_commit_func);
	postpq_module_flush_funcs_init(postpq_module_flush_func);
	postpq_module_init_funcs_init(postpq_module_init_func);
	dpu_crtc->postpq = postpq;

exit:
	return 0;
}

void dpu_crtc_color_postpq_deinit(struct drm_crtc *crtc)
{
	struct dpu_crtc *dpu_crtc;
	struct color_postpq *postpq;
	struct postpq_property_node *prop_node, *next;

	DPU_COLOR_DEBUG("crtc postpq deinit\n");

	if (!crtc) {
		DPU_ERROR("invalid drm_crtc, %pK\n", crtc);
		return;
	}

	dpu_crtc = to_dpu_crtc(crtc);
	if (!dpu_crtc) {
		DPU_ERROR("invalid dpu_crtc, %pK\n", dpu_crtc);
		return;
	}

	postpq = dpu_crtc->postpq;
	if (!postpq) {
		DPU_ERROR("invalid postpq, %pK\n", postpq);
		return;
	}

	if (postpq->hist_event.hist_blob)
		drm_property_blob_put(postpq->hist_event.hist_blob);

	list_for_each_entry_safe(prop_node, next, &postpq->feature_list,
				feature_head) {
		if (prop_node->prop_flags & DRM_MODE_PROP_BLOB
		    && prop_node->blob_ptr)
			drm_property_blob_put(prop_node->blob_ptr);

		list_del(&prop_node->feature_head);
		kfree(prop_node);
	}

	kfree(postpq);
	dpu_crtc->postpq = NULL;
}
