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

#include "dpu_color.h"
#include "dpu_plane.h"
#include "dpu_log.h"
#include "dpu_kms.h"
#include "dpu_crtc.h"
#include "dpu_power_mgr.h"

static int color_property_index_get(struct plane_color *color,
		struct drm_property *property)
{
	int i;
	int idx = -1;

	for (i = 0; i < PLANE_COLOR_PROP_MAX; i++) {
		if (color->property[i] == property) {
			idx = i;
			break;
		}
	}

	return idx;
}

static void color_create_range_property(struct drm_plane *plane,
		struct plane_color *color, char *name, u32 prop_index,
		uint64_t min, uint64_t max)
{
	struct drm_property *property;

	property = drm_property_create_range(plane->dev, 0,
			name, min, max);
	if (!property) {
		DPU_ERROR("drm_property_create_range fail: %s\n", name);
		return;
	}

	drm_object_attach_property(&plane->base, property, 0);

	color->property[prop_index] = property;
}

static void color_create_blob_property(struct drm_plane *plane,
		struct plane_color *color, char *name, u32 prop_index)
{
	struct drm_property *property;

	property = drm_property_create(plane->dev, DRM_MODE_PROP_BLOB, name, 0);
	if (!property) {
		DPU_ERROR("drm_property_create fail: %s\n", name);
		return;
	}

	drm_object_attach_property(&plane->base, property, 0);

	color->property[prop_index] = property;
}

static void dpu_pre_scaler_create_property(struct drm_plane *plane,
		struct plane_color *color)
{
	color_create_range_property(plane, color,
		"PLANE_COLOR_SCALER_EN",
		PLANE_COLOR_SCALER_EN, 0, 1);

	color_create_blob_property(plane, color,
		"PLANE_COLOR_SCALER", PLANE_COLOR_SCALER);
}

static void dpu_pre_csc_create_property(struct drm_plane *plane,
		struct plane_color *color)
{
	color_create_range_property(plane, color,
		"PLANE_COLOR_R2Y_EN",
		PLANE_COLOR_R2Y_EN, 0, 1);

	color_create_blob_property(plane, color,
		"PLANE_COLOR_R2Y", PLANE_COLOR_R2Y);

	color_create_range_property(plane, color,
		"PLANE_COLOR_Y2R_EN",
		PLANE_COLOR_Y2R_EN, 0, 1);

	color_create_blob_property(plane, color,
		"PLANE_COLOR_Y2R", PLANE_COLOR_Y2R);
}

static void dpu_pre_alpha_create_property(struct drm_plane *plane,
		struct plane_color *color)
{
	color_create_range_property(plane, color,
		"PLANE_COLOR_PREALPHA_EN",
		PLANE_COLOR_PREALPHA_EN, 0, 1);

	color_create_range_property(plane, color,
		"PLANE_COLOR_NON_PREALPHA_EN",
		PLANE_COLOR_NON_PREALPHA_EN, 0, 1);

	color_create_blob_property(plane, color,
		"PLANE_COLOR_NON_PREALPHA", PLANE_COLOR_NON_PREALPHA);
}

static void dpu_pre_tm_create_property(struct drm_plane *plane,
		struct plane_color *color)
{
	color_create_range_property(plane, color,
		"PLANE_COLOR_TM_EN",
		PLANE_COLOR_TM_EN, 0, 1);

	color_create_blob_property(plane, color,
		"PLANE_COLOR_TM", PLANE_COLOR_TM);
}

static void dpu_pre_hist_create_property(struct drm_plane *plane,
		struct plane_color *color)
{
	color_create_range_property(plane, color,
		"PLANE_COLOR_HIST_READ_MODE",
		PLANE_COLOR_HIST_READ_MODE, 0, HIST_READ_MODE_MAX);

	color_create_blob_property(plane, color,
		"PLANE_COLOR_HIST", PLANE_COLOR_HIST);
}

static void dpu_pre_3dlut_create_property(struct drm_plane *plane,
		struct plane_color *color)
{
	color_create_range_property(plane, color,
		"PLANE_COLOR_LUT3D_EN",
		PLANE_COLOR_LUT3D_EN, 0, 1);

	color_create_blob_property(plane, color,
		"PLANE_COLOR_LUT3D", PLANE_COLOR_LUT3D);
}

static void dpu_pre_dither_lite_create_property(struct drm_plane *plane,
		struct plane_color *color)
{
	color_create_range_property(plane, color,
		"PLANE_COLOR_DITHER_LITE_EN",
		PLANE_COLOR_DITHER_LITE_EN, 0, 1);
}

static int dpu_pre_color_create_properties(struct drm_plane *plane)
{
	struct dpu_plane *dpu_plane;

	dpu_plane = to_dpu_plane(plane);

	dpu_pre_scaler_create_property(plane, dpu_plane->color);
	dpu_pre_csc_create_property(plane, dpu_plane->color);
	dpu_pre_alpha_create_property(plane, dpu_plane->color);
	dpu_pre_tm_create_property(plane, dpu_plane->color);
	dpu_pre_hist_create_property(plane, dpu_plane->color);
	dpu_pre_3dlut_create_property(plane, dpu_plane->color);
	dpu_pre_dither_lite_create_property(plane, dpu_plane->color);

	return 0;
}

static int dpu_pre_color_set_property(struct drm_plane *plane,
		struct drm_plane_state *state,
		struct drm_property *property, uint64_t val)
{
	struct plane_color_state *color_state;
	struct drm_property_blob *blob = NULL;
	struct dpu_plane_state *plane_state;
	struct dpu_plane *dpu_plane;
	int prop_idx;

	dpu_plane = to_dpu_plane(plane);
	plane_state = to_dpu_plane_state(state);
	color_state = &plane_state->color_state;

	prop_idx = color_property_index_get(dpu_plane->color, property);
	if (prop_idx < 0)
		return -EINVAL;

	color_state->prop_value[prop_idx].value = val;
	if (property->flags & DRM_MODE_PROP_BLOB) {
		if (color_state->prop_value[prop_idx].blob)
			drm_property_blob_put(color_state->prop_value[prop_idx].blob);

		color_state->prop_value[prop_idx].blob = NULL;
		blob = drm_property_lookup_blob(plane->dev, val);
		if (!blob) {
			DPU_ERROR("color property %d: invalid blob %lld\n",
					prop_idx, val);
			return -EINVAL;
		}

		color_state->prop_value[prop_idx].blob = blob;
	}

	DPU_PROPERTY_DEBUG("set property %d - %s: %lld, %s: %d\n",
			prop_idx, property->name, val,
			blob ? "blob id" : "ret",
			blob ? blob->base.id : 0);
	return 0;
}

static int dpu_pre_color_get_property(struct drm_plane *plane,
		const struct drm_plane_state *state,
		struct drm_property *property, u64 *val)
{
	struct plane_color_state *color_state;
	struct dpu_plane *dpu_plane;
	int prop_idx;

	dpu_plane = to_dpu_plane(plane);
	color_state = &(to_dpu_plane_state(state)->color_state);

	prop_idx = color_property_index_get(dpu_plane->color, property);
	if (prop_idx < 0)
		return -EINVAL;

	*val = color_state->prop_value[prop_idx].value;

	DPU_PROPERTY_DEBUG("get property %d - %s: %lld\n", prop_idx, property->name, *val);
	return 0;
}

static int dpu_pre_color_check_properties(struct drm_plane *plane,
		struct drm_plane_state *state)
{
	return 0;
}

static int dpu_pre_color_duplicate_state(struct drm_plane_state *new_state,
		struct drm_plane_state *old_state)
{
	struct plane_color_state *old_color_state;
	struct plane_color_state *new_color_state;
	int i;

	old_color_state = &(to_dpu_plane_state(old_state)->color_state);
	new_color_state = &(to_dpu_plane_state(new_state)->color_state);

	for (i = 0; i < PLANE_COLOR_PROP_MAX; i++) {
		new_color_state->prop_value[i].value =
				old_color_state->prop_value[i].value;
		if (old_color_state->prop_value[i].blob) {
			new_color_state->prop_value[i].blob =
					old_color_state->prop_value[i].blob;
			drm_property_blob_get(new_color_state->prop_value[i].blob);
		}
	}

	return 0;
}

void dpu_pre_color_destroy_state(struct drm_plane *plane,
		struct drm_plane_state *state)
{
	struct plane_color_state *color_state;
	int i;

	color_state = &(to_dpu_plane_state(state)->color_state);

	for (i = 0; i < PLANE_COLOR_PROP_MAX; i++) {
		if (color_state->prop_value[i].blob) {
			drm_property_blob_put(color_state->prop_value[i].blob);
			color_state->prop_value[i].blob = NULL;
		}
	}
}

static void dpu_pre_scaler_commit(struct plane_color *color,
		struct plane_color_state *color_state,
		struct prepq_hw_cfg *hw_cfg)
{
	struct drm_property_blob *scaler_blob;
	bool scaler_enable;

	if (!color->hw_scaler)
		return;

	scaler_enable = color_state->prop_value[PLANE_COLOR_SCALER_EN].value;

	color->hw_scaler->ops.enable(&color->hw_scaler->hw, scaler_enable);

	if (scaler_enable) {
		scaler_blob = color_state->prop_value[PLANE_COLOR_SCALER].blob;
		if (!scaler_blob)
			return;

		hw_cfg->data = scaler_blob->data;
		hw_cfg->len = scaler_blob->length;
		color->hw_scaler->ops.set(&color->hw_scaler->hw, hw_cfg);
	}
}

static void dpu_pre_y2r_commit(struct plane_color *color,
		struct plane_color_state *color_state,
		struct prepq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(PLANE_COLOR_Y2R_EN);
	struct drm_property_blob *y2r_blob;
	struct dpu_hw_blk *hw;
	bool y2r_enable;

	if (!color->hw_prepipe_top)
		return;

	hw = &color->hw_prepipe_top->hw;
	if (!hw) {
		DPU_ERROR("invalid hw %pK\n", hw);
		return;
	}
	if (test_bit(PREPIPE_TOP_LITE, &hw->features)) {
		DPU_COLOR_DEBUG("ppt lite not support, id %d, offset 0x%x, features 0x%lx\n",
				hw->blk_id, hw->blk_offset, hw->features);
		return;
	}

	y2r_enable = color_state->prop_value[PLANE_COLOR_Y2R_EN].value;

	if (y2r_enable)
		color->enabled_features |= feature;
	else
		color->enabled_features &= ~feature;

	if (y2r_enable) {
		y2r_blob = color_state->prop_value[PLANE_COLOR_Y2R].blob;
		if (!y2r_blob)
			return;

		hw_cfg->data = y2r_blob->data;
		hw_cfg->len = y2r_blob->length;
		color->hw_prepipe_top->ops.set_y2r(&color->hw_prepipe_top->hw, hw_cfg);
	}
}

static void dpu_pre_r2y_commit(struct plane_color *color,
		struct plane_color_state *color_state,
		struct prepq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(PLANE_COLOR_R2Y_EN);
	struct drm_property_blob *r2y_blob;
	struct dpu_hw_blk *hw;
	bool r2y_enable;

	if (!color->hw_prepipe_top)
		return;

	hw = &color->hw_prepipe_top->hw;
	if (!hw) {
		DPU_ERROR("invalid hw %pK\n", hw);
		return;
	}
	if (test_bit(PREPIPE_TOP_LITE, &hw->features)) {
		DPU_COLOR_DEBUG("ppt lite not support, id %d, offset 0x%x, features 0x%lx\n",
				hw->blk_id, hw->blk_offset, hw->features);
		return;
	}

	r2y_enable = color_state->prop_value[PLANE_COLOR_R2Y_EN].value;

	if (r2y_enable)
		color->enabled_features |= feature;
	else
		color->enabled_features &= ~feature;

	if (r2y_enable) {
		r2y_blob = color_state->prop_value[PLANE_COLOR_R2Y].blob;
		if (!r2y_blob)
			return;

		hw_cfg->data = r2y_blob->data;
		hw_cfg->len = r2y_blob->length;
		color->hw_prepipe_top->ops.set_r2y(&color->hw_prepipe_top->hw, hw_cfg);
	}
}

static void dpu_pre_alpha_commit(struct plane_color *color,
		struct plane_color_state *color_state,
		struct prepq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(PLANE_COLOR_PREALPHA_EN);
	struct dpu_hw_blk *hw;
	bool alpha_enable;

	if (!color->hw_prepipe_top)
		return;

	hw = &color->hw_prepipe_top->hw;
	if (!hw) {
		DPU_ERROR("invalid hw %pK\n", hw);
		return;
	}
	if (test_bit(PREPIPE_TOP_LITE, &hw->features)) {
		DPU_COLOR_DEBUG("ppt lite not support, id %d, offset 0x%x, features 0x%lx\n",
				hw->blk_id, hw->blk_offset, hw->features);
		return;
	}

	alpha_enable = color_state->prop_value[PLANE_COLOR_PREALPHA_EN].value;

	if (alpha_enable)
		color->enabled_features |= feature;
	else
		color->enabled_features &= ~feature;
}

static void dpu_non_pre_alpha_commit(struct plane_color *color,
		struct plane_color_state *color_state,
		struct prepq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(PLANE_COLOR_NON_PREALPHA_EN);
	struct drm_property_blob *offset_blob;
	bool non_prealpha_enable;

	if (!color->hw_prepipe_top)
		return;

	non_prealpha_enable =
			color_state->prop_value[PLANE_COLOR_NON_PREALPHA_EN].value;

	if (non_prealpha_enable) {
		offset_blob = color_state->prop_value[PLANE_COLOR_NON_PREALPHA].blob;
		if (!offset_blob)
			return;

		hw_cfg->data = offset_blob->data;
		hw_cfg->len = offset_blob->length;
		color->enabled_features |= feature;
		color->hw_prepipe_top->ops.set_nonpre_alpha(&color->hw_prepipe_top->hw,
				hw_cfg);
	} else {
		color->enabled_features &= ~feature;
	}
}

static void dpu_tm_commit(struct plane_color *color,
		struct plane_color_state *color_state,
		struct prepq_hw_cfg *hw_cfg)
{
	struct drm_property_blob *tm_blob;
	bool tm_enable;

	if (!color->hw_tm)
		return;

	tm_enable = color_state->prop_value[PLANE_COLOR_TM_EN].value;

	if (!tm_enable) {
		color->hw_tm->ops.enable(&color->hw_tm->hw, false);
	} else {
		tm_blob = color_state->prop_value[PLANE_COLOR_TM].blob;
		if (!tm_blob)
			return;

		hw_cfg->data = tm_blob->data;
		hw_cfg->len = tm_blob->length;
		color->hw_tm->ops.set_tm(&color->hw_tm->hw, hw_cfg);
	}
}

static void dpu_pre_hist_commit(struct plane_color *color,
		struct plane_color_state *color_state,
		struct prepq_hw_cfg *hw_cfg)
{
	struct drm_property_blob *hist_blob;
	struct dpu_plane_state *pstate;
	struct dpu_color *colorp;
	u32 read_mode;

	if (!color->hw_hist)
		return;

	colorp = color->color_private;
	if (!colorp)
		return;

	pstate = container_of(color_state, struct dpu_plane_state, color_state);
	read_mode = color_state->prop_value[PLANE_COLOR_HIST_READ_MODE].value;

	if (read_mode != colorp->pre_color.hist_event.hist_read_mode ||
			need_restore_register_after_idle(pstate->base.crtc))
		color->hw_hist->ops.enable_update(&color->hw_hist->hw);

	colorp->pre_color.hist_event.hist_read_mode = read_mode;
	if (read_mode == HIST_READ_DISABLE) {
		color->hw_hist->ops.enable(&color->hw_hist->hw, false);
		colorp->pre_color.hist_event.enable = false;
	} else {
		hist_blob = color_state->prop_value[PLANE_COLOR_HIST].blob;
		if (!hist_blob)
			return;

		hw_cfg->data = hist_blob->data;
		hw_cfg->len = hist_blob->length;
		color->hw_hist->ops.set_hist(&color->hw_hist->hw, hw_cfg);
		color->hw_hist->ops.enable(&color->hw_hist->hw, true);

		colorp->pre_color.hist_event.enable = true;
		if (read_mode == HIST_READ_ONCE)
			atomic_inc(&colorp->pre_color.hist_event.read_cnt);
	}

	if (colorp && !colorp->pre_color.hw_hist)
		colorp->pre_color.hw_hist = color->hw_hist;

}

static void dpu_pre_3dlut_commit(struct plane_color *color,
		struct plane_color_state *color_state,
		struct prepq_hw_cfg *hw_cfg)
{
	struct drm_property_blob *lut3d_blob;
	bool lut3d_enable;

	if (!color->hw_3dlut)
		return;

	lut3d_enable = color_state->prop_value[PLANE_COLOR_LUT3D_EN].value;

	if (!lut3d_enable) {
		color->hw_3dlut->ops.enable(&color->hw_3dlut->hw, false);
	} else {
		lut3d_blob = color_state->prop_value[PLANE_COLOR_LUT3D].blob;
		if (!lut3d_blob)
			return;

		hw_cfg->data = lut3d_blob->data;
		hw_cfg->len = lut3d_blob->length;
		color->hw_3dlut->ops.set_3dlut(&color->hw_3dlut->hw, hw_cfg);
		color->hw_3dlut->ops.enable(&color->hw_3dlut->hw, true);
		color->hw_3dlut->ops.flush(&color->hw_3dlut->hw, false);
	}
}

static void dpu_pre_dither_lite_commit(struct plane_color *color,
		struct plane_color_state *color_state,
		struct prepq_hw_cfg *hw_cfg)
{
	bool dither_lite_en;

	if (!color->hw_prepipe_top)
		return;

	dither_lite_en = color_state->prop_value[PLANE_COLOR_DITHER_LITE_EN].value;
	color->hw_prepipe_top->ops.dither_lite_enable(&color->hw_prepipe_top->hw, dither_lite_en);
}

static void dpu_prepq_top_enable(struct plane_color *color, struct dpu_plane_state *plane_state)
{
	struct prepq_feature_ctrl ctrl_info;

	if (!color->hw_prepipe_top || !plane_state)
		return;

	ctrl_info.enabled_features = color->enabled_features;
	ctrl_info.left_right_reuse =
			(plane_state->extender_mode >= EXTENDER_MODE_LEFT) ? true : false;
	color->hw_prepipe_top->ops.feature_enable(&color->hw_prepipe_top->hw, &ctrl_info);
}

static int dpu_pre_color_commit(struct drm_plane *plane)
{
	struct dpu_plane_state *dpu_plane_state;
	struct plane_color_state *color_state;
	struct dpu_plane *dpu_plane;
	struct prepq_hw_cfg hw_cfg;

	memset(&hw_cfg, 0, sizeof(hw_cfg));
	dpu_plane = to_dpu_plane(plane);
	dpu_plane_state = to_dpu_plane_state(plane->state);
	color_state = &(dpu_plane_state->color_state);

	dpu_pre_scaler_commit(dpu_plane->color, color_state, &hw_cfg);
	dpu_pre_r2y_commit(dpu_plane->color, color_state, &hw_cfg);
	dpu_pre_y2r_commit(dpu_plane->color, color_state, &hw_cfg);
	dpu_pre_alpha_commit(dpu_plane->color, color_state, &hw_cfg);
	dpu_non_pre_alpha_commit(dpu_plane->color, color_state, &hw_cfg);
	dpu_tm_commit(dpu_plane->color, color_state, &hw_cfg);
	dpu_pre_hist_commit(dpu_plane->color, color_state, &hw_cfg);
	dpu_pre_3dlut_commit(dpu_plane->color, color_state, &hw_cfg);
	dpu_pre_dither_lite_commit(dpu_plane->color, color_state, &hw_cfg);
	dpu_prepq_top_enable(dpu_plane->color, dpu_plane_state);

	return 0;
}

struct plane_color_funcs color_funcs = {
	.create_properties = dpu_pre_color_create_properties,
	.set_property = dpu_pre_color_set_property,
	.get_property = dpu_pre_color_get_property,
	.check_properties = dpu_pre_color_check_properties,
	.duplicate_state = dpu_pre_color_duplicate_state,
	.destroy_state = dpu_pre_color_destroy_state,
	.commit = dpu_pre_color_commit,
};

void dpu_pre_color_hist_get(struct drm_device *dev,
		struct dpu_pre_color *pre_color)
{
	struct dpu_hw_pre_hist *hist;
	struct dpu_hist_data *hist_data;
	struct drm_event event;

	if (!pre_color)
		return;

	hist = pre_color->hw_hist;
	hist_data = (struct dpu_hist_data *)pre_color->hist_event.hist_blob->data;
	memset(hist_data->data, 0, sizeof(hist_data->data));

	DPU_POWER_PROTECT_BEGIN();
	hist->ops.get_hist(&hist->hw, 0, HIST_DATA_SIZE, hist_data);
	DPU_POWER_PROTECT_END();

	event.length = sizeof(u32);
	event.type = DRM_PRIVATE_EVENT_VCHN0_HISTOGRAM;
	dpu_drm_event_notify(dev, &event,
			(u8 *)(&pre_color->hist_event.hist_blob->base.id));

	DPU_COLOR_DEBUG("sent pre hist blob id 0x%x\n",
			pre_color->hist_event.hist_blob->base.id);
}

int dpu_pre_color_init(struct drm_plane *plane)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;
	struct dpu_plane *dpu_plane;
	struct plane_color *color;

	if (!plane) {
		DPU_ERROR("invalid parameters, %pK\n", plane);
		return -EINVAL;
	}

	dpu_drm_dev = to_dpu_drm_dev(plane->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	dpu_plane = to_dpu_plane(plane);
	if (dpu_plane->color)
		goto exit;

	color = kzalloc(sizeof(*color), GFP_KERNEL);
	if (!color) {
		DPU_ERROR("alloc color fail\n");
		return -ENOMEM;
	}

	color->funcs = &color_funcs;
	color->color_private = dpu_kms->color;

	dpu_plane->color = color;

	DPU_COLOR_DEBUG("done\n");
exit:
	return 0;
}

void dpu_pre_color_deinit(struct drm_plane *plane)
{
	struct dpu_plane *dpu_plane;
	struct plane_color *color;

	if (!plane) {
		DPU_ERROR("invalid plane, %pK\n", plane);
		return;
	}

	dpu_plane = to_dpu_plane(plane);
	color = dpu_plane->color;
	if (!color) {
		DPU_ERROR("invalid plane color, %pK\n", color);
		return;
	}

	kfree(color);
	dpu_plane->color = NULL;

	DPU_COLOR_DEBUG("done\n");
}
