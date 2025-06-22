// SPDX-License-Identifier: GPL-2.0-only
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
#include <drm/drm_atomic_helper.h>
#include <drm/drm_plane.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_writeback.h>
#include "dpu_cmdlist_node.h"
#include "dpu_cmdlist_frame_mgr.h"
#include "dpu_hw_rch.h"
#include "dpu_format.h"
#include "dpu_stripe.h"
#include "dpu_hw_stripe.h"

#define NODE_ITEM_ALIGN 2

#define HW_STRIPE_MIXER_REG_NUM        8
#define LAYER_STRIPE_REG_NUM           38
#define WB_STRIPE_REG_NUM              100
#define SCALER_PHASE_LOW_32_BIT        0x00000000ffffffff
#define SCALER_PHASE_HIGH_1_BIT        0x0000000100000000

static bool g_stripe_enable;

static inline void cmdlist_node_length_align(u16 *node_len, u16 reg_num)
{
	*node_len += ALIGN(reg_num, NODE_ITEM_ALIGN) / NODE_ITEM_ALIGN;
}

void dpu_hw_wb_cmps_stripe_cmdlist_prepare(struct dpu_crtc *dpu_crtc,
		struct dpu_wb_connector *dpu_wb, s64 blk_id)
{
	struct dpu_crtc_state *dpu_crtc_state;
	u16 node_len = 0;

	if (!dpu_crtc || !dpu_wb) {
		DPU_ERROR("NULL dpu_crtc pointer or NULL dpu_wb pointer\n");
		return;
	}

	if (dpu_wb) {
		cmdlist_node_length_align(&node_len, WB_STRIPE_REG_NUM);
		dpu_crtc_state = to_dpu_crtc_state(dpu_crtc->base.state);

		cmdlist_node_length_align(&node_len, HW_STRIPE_MIXER_REG_NUM);

		dpu_wb->wb_slice_node_id[blk_id] = cmdlist_node_create(RCH_MAX +
			dpu_crtc->hw_mixer->hw.blk_id, NORMAL_TYPE, node_len, CMPS_RELOAD | WB1_RELOAD);
		dpu_crtc_state->cmps_slice_node_id[blk_id] = dpu_wb->wb_slice_node_id[blk_id];
	}

	DPU_CMDLIST_DEBUG("dpu_wb->wb_slice_node_id[%lld] = %lld\n", blk_id,
		dpu_wb->wb_slice_node_id[blk_id]);
}

static void dpu_hw_plane_stripe_calculate_valid_stripe(struct drm_plane *plane)
{
	struct display_rch_stripe_info plane_stripe_cfg;
	struct dpu_plane_state *dpu_plane_state;
	struct drm_property_blob *blob;
	u16 valid_stripe_count = 0;
	int i;

	if (!plane) {
		DPU_ERROR("NULL plane pointer Error\n");
		return;
	}

	dpu_plane_state = to_dpu_plane_state(plane->state);

	if (!dpu_plane_state) {
		DPU_ERROR("NULL dpu_plane_state pointer Error\n");
		return;
	}

	if (dpu_plane_state->plane_stripe_count <= MIN_STRIPE_COUNT) {
		dpu_plane_state->plane_valid_stripe_count = valid_stripe_count;
		return;
	}
	g_stripe_enable = true;
	/* get plane stripe para ptr */
	blob = drm_property_lookup_blob(plane->dev, dpu_plane_state->stripe_para_blob_id);
	if (dpu_plane_state->stripe_para_blob_ptr)
		drm_property_blob_put(dpu_plane_state->stripe_para_blob_ptr);

	dpu_plane_state->stripe_para_blob_ptr = blob;
	STRIPE_DEBUG("plane stripe blob_para == %p", blob);

	/* set cfg to cmdlist regs */
	for (i = 0; i < dpu_plane_state->plane_stripe_count; i++) {
		memcpy(&plane_stripe_cfg,
			(struct display_rch_stripe_info *)(blob->data) + i,
			sizeof(struct display_rch_stripe_info));

		if (plane_stripe_cfg.layer_crop_rect.w != 0)
			valid_stripe_count++;
	}
	STRIPE_DEBUG("cyrrect plane has %d valid stripe", valid_stripe_count);
	dpu_plane_state->plane_valid_stripe_count = valid_stripe_count;
}

void dpu_hw_plane_stripe_cmdlist_prepare(struct drm_plane *plane)
{
	struct dpu_plane *dpu_plane;
	struct dpu_plane_state *dpu_plane_state;
	u16 node_len = 0;
	int i;

	if (!plane) {
		DPU_ERROR("NULL plane pointer Error\n");
		return;
	}
	dpu_plane_state = to_dpu_plane_state(plane->state);

	if (!dpu_plane_state) {
		DPU_ERROR("NULL dpu_plane_state pointer Error\n");
		return;
	}
	dpu_hw_plane_stripe_calculate_valid_stripe(plane);
	for (i = 0; i < (dpu_plane_state->plane_valid_stripe_count - 1); i++) {
		dpu_plane = to_dpu_plane(plane);
		cmdlist_node_length_align(&node_len, LAYER_STRIPE_REG_NUM);

		dpu_plane->plane_slice_node_id[i] = cmdlist_node_create(dpu_plane->hw_rch->hw.blk_id,
				NORMAL_TYPE, node_len, RDMA_RELOAD | PREPQ_RELOAD);
		DPU_CMDLIST_INFO("dpu_plane->plane_slice_node_id[%d] = %lld\n", i,
				dpu_plane->plane_slice_node_id[i]);
		DPU_CMDLIST_INFO("one block config end");
	}
}

void dpu_hw_stripe_post_mixer_blks_cmdlist_prepare(struct dpu_crtc *dpu_crtc,
		struct drm_connector *connector)
{
	struct dpu_wb_connector *dpu_wb;
	struct dpu_wb_connector_state *dpu_wb_conn_state;
	int i;

	if (connector) {
		dpu_wb = to_dpu_wb_connector(drm_connector_to_writeback(connector));
		dpu_wb_conn_state = to_dpu_wb_connector_state(connector->state);
		if (!dpu_wb_conn_state) {
			DPU_ERROR("NULL pointer Error\n");
			return;
		}

		for (i = 0; i < (dpu_wb_conn_state->wb_stripe_count - 1); i++) {
			DPU_CMDLIST_INFO("dpu_hw_wb_stripe_prepare");
			dpu_hw_wb_cmps_stripe_cmdlist_prepare(dpu_crtc, dpu_wb, i);
		}
	}
}

void dpu_stripe_plane_cmdlist_append(struct drm_plane *plane, s64 frame_id)
{
	struct dpu_plane_state *dpu_pstate;
	struct dpu_plane *dpu_plane;
	u32 rch_id;
	int i;

	if (plane) {
		dpu_plane = to_dpu_plane(plane);
		dpu_pstate = to_dpu_plane_state(plane->state);
		if (!dpu_pstate) {
			DPU_ERROR("NULL pointer Error\n");
			return;
		}
		rch_id = dpu_plane->hw_rch->hw.blk_id;
		for (i = 0; i < (dpu_pstate->plane_valid_stripe_count - 1); i++) {
			cmdlist_cfg_rdy_set(dpu_plane->plane_slice_node_id[i],
					CMDLIST_RDMA_CFG_RDY(rch_id) |
					CMDLIST_PREPQ_CFG_RDY(rch_id));
			cmdlist_frame_node_append(frame_id, &dpu_plane->plane_slice_node_id[i],
				DELETE_WITH_FRAME);
		}
	}
}

void dpu_stripe_cmps_wb_cmdlist_append(struct drm_connector *connector, s64 frame_id, u32 cfg_rdy)
{
	struct dpu_wb_connector_state *dpu_wb_conn_state;
	struct drm_connector_state *conn_state;
	struct dpu_wb_connector *dpu_wb_conn;
	int i;

	if (connector) {
		dpu_wb_conn = to_dpu_wb_connector(drm_connector_to_writeback(connector));
		conn_state = connector->state;
		dpu_wb_conn_state = to_dpu_wb_connector_state(conn_state);
		if (!dpu_wb_conn_state) {
			DPU_ERROR("NULL pointer Error\n");
			return;
		}
		for (i = 0; i < (dpu_wb_conn_state->wb_stripe_count - 1); i++) {
			cmdlist_cfg_rdy_set(dpu_wb_conn->wb_slice_node_id[i], cfg_rdy);
			cmdlist_frame_node_append(frame_id, &dpu_wb_conn->wb_slice_node_id[i], DELETE_WITH_FRAME);
		}
	}
}

static int dpu_plane_create_stripe_para_properties(struct drm_plane *plane,
		struct dpu_plane *dpu_plane)
{
	struct drm_property *property;

	if (!plane || !dpu_plane) {
		DPU_ERROR("NULL drm_plane pointer or NULL dpu_plane pointer\n");
		return -EFAULT;
	}

	property = drm_property_create(plane->dev, DRM_MODE_PROP_BLOB,
			"plane_stripe_para_prop", 0);
	if (!property) {
		DPU_ERROR("drm_property_create fail: plane_stripe_para_prop\n");
		return -ENOMEM;
	}
	drm_object_attach_property(&plane->base, property, 0);
	dpu_plane->stripe_para_blob_prop = property;
	dpu_plane->stripe_para_blob_size = sizeof(struct display_rch_stripe_info) * MAX_STRIPE_COUNT;

	return 0;
}

static int dpu_plane_create_stripe_count_properties(struct drm_plane *plane,
		struct dpu_plane *dpu_plane)
{
	struct drm_property *prop;

	if (!plane || !dpu_plane) {
		DPU_ERROR("NULL drm_plane pointer or NULL dpu_plane pointer\n");
		return -EFAULT;
	}

	prop = drm_property_create_range(plane->dev, 0,
			"plane_stripe_count", 0, MAX_STRIPE_COUNT);
	if (!prop) {
		DPU_ERROR("create failed\n");
		return -ENOMEM;
	}

	drm_object_attach_property(&plane->base, prop, 0);
	dpu_plane->stripe_count_prop = prop;

	return 0;
}

int dpu_plane_create_stripe_properties(struct drm_plane *plane)
{
	struct dpu_plane *dpu_plane;

	if (!plane) {
		DPU_ERROR("invalid parameters, %pK\n", plane);
		return -EINVAL;
	}
	dpu_plane = to_dpu_plane(plane);
	if (dpu_plane_create_stripe_count_properties(plane, dpu_plane) != 0) {
		DPU_ERROR("create stripe count property fail\n");
		return -EINVAL;
	}
	if (dpu_plane_create_stripe_para_properties(plane, dpu_plane) != 0) {
		DPU_ERROR("create stripe para property fail\n");
		return -EINVAL;
	}
	return 0;
}

static int dpu_wb_create_stripe_para_properties(struct drm_writeback_connector *wb_conn,
		struct dpu_wb_connector *dpu_wb_conn)
{
	struct drm_property *property;

	if (!dpu_wb_conn || !wb_conn) {
		DPU_ERROR("NULL dpu_wb_connector pointer or NULL drm_writeback_connector pointer\n");
		return -EFAULT;
	}

	property = drm_property_create(wb_conn->base.dev, DRM_MODE_PROP_BLOB,
			"wb_stripe_para_prop", 0);
	if (!property) {
		DPU_ERROR("drm_property_create fail: wb_stripe_para_prop\n");
		return -ENOMEM;
	}
	drm_object_attach_property(&wb_conn->base.base, property, 0);
	dpu_wb_conn->stripe_para_blob_prop = property;
	dpu_wb_conn->stripe_para_blob_size = sizeof(struct display_wb_stripe_info) * MAX_STRIPE_COUNT;

	return 0;
}

static int dpu_wb_create_stripe_count_property(struct drm_writeback_connector *drm_wb_conn,
		struct dpu_wb_connector *dpu_wb_conn)
{
	struct drm_property *prop;

	if (!dpu_wb_conn || !drm_wb_conn) {
		DPU_ERROR("NULL dpu_wb_connector pointer or NULL drm_writeback_connector pointer\n");
		return -EFAULT;
	}

	prop = drm_property_create_range(drm_wb_conn->base.dev, 0,
			"wb_stripe_count", 0, MAX_STRIPE_COUNT);
	if (!prop) {
		DPU_ERROR("failed to create property\n");
		return -ENOMEM;
	}

	drm_object_attach_property(&drm_wb_conn->base.base, prop, 0);
	dpu_wb_conn->stripe_count_prop = prop;

	return 0;
}

int dpu_wb_create_stripe_properties(struct drm_writeback_connector *wb_conn)
{
	struct dpu_wb_connector *dpu_wb_conn;

	if (!wb_conn) {
		DPU_ERROR("invalid parameters, %pK\n", wb_conn);
		return -EINVAL;
	}

	dpu_wb_conn = to_dpu_wb_connector(wb_conn);
	if (dpu_wb_create_stripe_count_property(wb_conn, dpu_wb_conn) != 0)
		return -EINVAL;
	if (dpu_wb_create_stripe_para_properties(wb_conn, dpu_wb_conn) != 0)
		return -EINVAL;
	return 0;
}

static struct dpu_rect stripe_rect_to_dpu_rect(struct stripe_rect in_rect)
{
	struct dpu_rect out_rect;

	out_rect.x1 = in_rect.x;
	out_rect.y1 = in_rect.y;
	out_rect.x2 = in_rect.x + in_rect.w - 1;
	out_rect.y2 = in_rect.y + in_rect.h - 1;

	return out_rect;
}

static void dpu_stripe_crtc_mixer_input_commit(struct dpu_hw_stripe *hw_stripe,
		struct dpu_hw_mixer *hw_mixer, struct drm_plane *plane,
		struct drm_connector *connector, struct dpu_crtc_state *dpu_cstate)
{
	struct display_rch_stripe_info plane_stripe_cfg;
	const struct dpu_format_map *dpu_format_map;
	struct display_wb_stripe_info wb_stripe_cfg;
	struct drm_property_blob *connector_blob;
	struct dpu_plane_state *dpu_plane_state;
	struct dpu_wb_connector_state *wb_state;
	struct drm_connector_state *conn_state;
	struct drm_property_blob *plane_blob;
	struct dpu_rect_v2 cmps_stripe_area;
	struct mixer_blend_cfg blend_cfg;
	u32 zorder = 0;
	s64 node_id;
	u16 i;

	if (!g_stripe_enable)
		return;

	dpu_plane_state = to_dpu_plane_state(plane->state);
	if (dpu_plane_state->plane_stripe_count <= MIN_STRIPE_COUNT) {
		STRIPE_DEBUG("writeback stripe disable");
		return;
	}

	conn_state = connector->state;
	wb_state = to_dpu_wb_connector_state(conn_state);

	if (!wb_state) {
		DPU_ERROR("NULL pointer");
		return;
	}
	/* get plane stripe para ptr */
	plane_blob = drm_property_lookup_blob(plane->dev, dpu_plane_state->stripe_para_blob_id);
	if (dpu_plane_state->stripe_para_blob_ptr)
		drm_property_blob_put(dpu_plane_state->stripe_para_blob_ptr);
	dpu_plane_state->stripe_para_blob_ptr = plane_blob;

	/* get wb stripe para ptr */
	connector_blob = drm_property_lookup_blob(connector->dev, wb_state->stripe_para_blob_id);
	if (wb_state->stripe_para_blob_ptr)
		drm_property_blob_put(wb_state->stripe_para_blob_ptr);
	wb_state->stripe_para_blob_ptr = connector_blob;

	/* set cfg to cmdlist regs */
	for (i = 0; i < dpu_plane_state->plane_stripe_count; i++) {
		memcpy(&plane_stripe_cfg,
			(struct display_rch_stripe_info *)(plane_blob->data) + i,
			sizeof(struct display_rch_stripe_info));
		memcpy(&wb_stripe_cfg,
			(struct display_wb_stripe_info *)(connector_blob->data) + i,
			sizeof(struct display_wb_stripe_info));

		node_id = (i == 0) ?
			*hw_mixer->hw.cmd_node : dpu_cstate->cmps_slice_node_id[i - 1];
		STRIPE_DEBUG("dpu_cstate cmps node_id == %lld\n", node_id);

		zorder = TO_HW_ZORDER(plane->state->zpos);
		STRIPE_DEBUG("plane_stripe_cfg.layer_output_rect == x %d, y %d, w %d, h %d",
				plane_stripe_cfg.layer_output_rect.x,
				plane_stripe_cfg.layer_output_rect.y,
				plane_stripe_cfg.layer_output_rect.w,
				plane_stripe_cfg.layer_output_rect.h);
		if ((plane_stripe_cfg.layer_output_rect.w != 0) &&
			(plane_stripe_cfg.layer_output_rect.h != 0)) {
			cmps_stripe_area.x = plane_stripe_cfg.layer_output_rect.x -
						wb_stripe_cfg.wb_input_rect.x;
			cmps_stripe_area.y = plane_stripe_cfg.layer_output_rect.y -
						wb_stripe_cfg.wb_input_rect.y;
			cmps_stripe_area.w = plane_stripe_cfg.layer_output_rect.w;
			cmps_stripe_area.h = plane_stripe_cfg.layer_output_rect.h;
		} else {
			cmps_stripe_area.x = 0;
			cmps_stripe_area.y = 0;
			cmps_stripe_area.w = 0;
			cmps_stripe_area.h = 0;
		}
		STRIPE_DEBUG("cmps_stripe_area == x %d, y %d, w %d, h %d",
			cmps_stripe_area.x, cmps_stripe_area.y,
				cmps_stripe_area.w, cmps_stripe_area.h);
		dpu_format_map = dpu_format_get(FMT_MODULE_RCH,
				plane->state->fb->format->format,
				plane->state->fb->modifier);
		blend_cfg.blend_mode = plane->state->pixel_blend_mode;
		blend_cfg.has_pixel_alpha = dpu_format_map->has_alpha;
		blend_cfg.layer_alpha = (plane->state->alpha & 0xff);
		hw_stripe->ops.cmps_rch_layer_config(&hw_mixer->hw, node_id, &blend_cfg,
			&cmps_stripe_area, zorder, dpu_plane_state->rch_id);
	}
}

static void dpu_stripe_crtc_mixer_output_commit(struct dpu_hw_stripe *hw_stripe,
		struct dpu_hw_mixer *hw_mixer, struct drm_connector *connector,
		struct dpu_crtc_state *dpu_cstate)
{
	struct display_wb_stripe_info wb_stripe_cfg;
	struct dpu_wb_connector_state *wb_state;
	struct drm_connector_state *conn_state;
	struct drm_property_blob *blob_para;
	s64 node_id;
	u32 i;

	if (!g_stripe_enable)
		return;

	conn_state = connector->state;
	wb_state = to_dpu_wb_connector_state(conn_state);

	if (!wb_state || wb_state->wb_stripe_count <= MIN_STRIPE_COUNT) {
		DPU_ERROR("NULL pointer or writeback stripe disable");
		return;
	}

	/* get wb stripe para ptr */
	blob_para = drm_property_lookup_blob(connector->dev, wb_state->stripe_para_blob_id);
	if (wb_state->stripe_para_blob_ptr)
		drm_property_blob_put(wb_state->stripe_para_blob_ptr);
	wb_state->stripe_para_blob_ptr = blob_para;
	STRIPE_DEBUG("writeback stripe blob_para == %p", blob_para);

	/* set cfg to cmdlist regs */
	for (i = 0; i < wb_state->wb_stripe_count; i++) {
		memcpy(&wb_stripe_cfg,
			(struct display_wb_stripe_info *)(blob_para->data) + i,
			sizeof(struct display_wb_stripe_info));

		if (hw_mixer && hw_stripe) {
			node_id = (i == 0) ?
				*hw_mixer->hw.cmd_node : dpu_cstate->cmps_slice_node_id[i - 1];
			hw_stripe->ops.cmps_wb_stripe_dst_config(&hw_mixer->hw, node_id,
					wb_stripe_cfg.wb_input_rect.w, wb_stripe_cfg.wb_input_rect.h);
		}
	}
}

static enum rot_mode dpu_wb_get_rotation_info(u32 rotation)
{
	enum rot_mode dpu_rot_mode;
	u8 drm_rot_mode;

	drm_rot_mode = rotation & DRM_MODE_ROTATE_MASK;
	switch (drm_rot_mode) {
	case DRM_MODE_ROTATE_90:
		dpu_rot_mode = ROT_90;
		break;
	case DRM_MODE_ROTATE_180:
		dpu_rot_mode = ROT_180;
		break;
	case DRM_MODE_ROTATE_270:
		dpu_rot_mode = ROT_270;
		break;
	default:
		dpu_rot_mode = ROT_0;
		break;
	}

	return dpu_rot_mode;
}

static enum flip_mode dpu_wb_get_flip_info(u32 rotation)
{
	enum flip_mode dpu_flip_mode;
	u8 drm_flip_mode;

	drm_flip_mode = rotation & DRM_MODE_REFLECT_MASK;
	switch (drm_flip_mode) {
	case DRM_MODE_REFLECT_X:
		dpu_flip_mode = H_FLIP;
		break;
	case DRM_MODE_REFLECT_Y:
		dpu_flip_mode = V_FLIP;
		break;
	case DRM_MODE_REFLECT_MASK:
		dpu_flip_mode = HV_FLIP;
		break;
	default:
		dpu_flip_mode = NO_FLIP;
		break;
	}

	return dpu_flip_mode;
}

static void wb_stripe_top_config_from_state(struct wb_stripe_frame_cfg *stripe_frame_cfg,
		struct dpu_wb_connector_state *wb_state, struct drm_framebuffer *fb)
{
	const struct dpu_format_map *format_map;

	format_map = dpu_format_get(FMT_MODULE_WB, fb->format->format,
			fb->modifier);
	stripe_frame_cfg->is_offline_mode = true;
	stripe_frame_cfg->scl_en = wb_state->scaler_enable;
	stripe_frame_cfg->frame_flip_mode = dpu_wb_get_flip_info(wb_state->rotation);
	stripe_frame_cfg->frame_rot_mode = dpu_wb_get_rotation_info(wb_state->rotation);
	stripe_frame_cfg->output_format = format_map->dpu_format;
	stripe_frame_cfg->is_afbc = DPU_FMT_IS_AFBC(fb->modifier);
	stripe_frame_cfg->dither_en = wb_state->dither_enable;
}

static void wb_stripe_input_rect_config(struct wb_stripe_frame_cfg *stripe_frame_cfg,
		struct display_wb_stripe_info *stripe_param)
{
	/* input height */
	stripe_frame_cfg->input_height = stripe_param->wb_input_rect.h;
	/* input width */
	stripe_frame_cfg->input_width = stripe_param->wb_input_rect.w;
	STRIPE_DEBUG("wb_stripe_input_rect w:%d h:%d\n", stripe_frame_cfg->input_width,
			stripe_frame_cfg->input_height);
}

static void wb_stripe_output_rect_config(struct wb_stripe_frame_cfg *stripe_frame_cfg,
		struct display_wb_stripe_info *stripe_param)
{
	stripe_frame_cfg->output_original_width = stripe_param->wb_output_rect.w;
	stripe_frame_cfg->output_original_height = stripe_param->wb_output_rect.h;
	STRIPE_DEBUG("wb_stripe_output_rect w:%d h:%d", stripe_frame_cfg->output_original_width,
				stripe_frame_cfg->output_original_height);
}

static void wb_stripe_output_subframe_config(struct wb_stripe_frame_cfg *stripe_frame_cfg,
		struct display_wb_stripe_info *stripe_param)
{
	stripe_frame_cfg->output_subframe_rect.x = stripe_param->wb_output_rect.x;
	stripe_frame_cfg->output_subframe_rect.y = stripe_param->wb_output_rect.y;
	stripe_frame_cfg->output_subframe_rect.w = stripe_param->wb_output_rect.w;
	stripe_frame_cfg->output_subframe_rect.h = stripe_param->wb_output_rect.h;
}

static void wb_stripe_crop2_config(struct wb_stripe_frame_cfg *stripe_frame_cfg,
		struct display_wb_stripe_info *stripe_param)
{
	struct dpu_rect crop2_rect;

	if (stripe_param->wb_crop2_enable) {
		crop2_rect = stripe_rect_to_dpu_rect(stripe_param->wb_crop2_rect);
		stripe_frame_cfg->crop2_en = true;
		stripe_frame_cfg->crop2_area.x1 = crop2_rect.x1;
		stripe_frame_cfg->crop2_area.y1 = crop2_rect.y1;
		stripe_frame_cfg->crop2_area.x2 = crop2_rect.x2;
		stripe_frame_cfg->crop2_area.y2 = crop2_rect.y2;
		STRIPE_DEBUG("stripe_frame_cfg crop2_area == x %d, y %d, w %d", stripe_frame_cfg->crop2_area.x1,
			stripe_frame_cfg->crop2_area.y1, stripe_frame_cfg->crop2_area.x2);
	} else {
		stripe_frame_cfg->crop2_en = false;
	}
}

static void wb_stripe_crop1_config(struct wb_stripe_frame_cfg *stripe_frame_cfg,
		struct display_wb_stripe_info *stripe_param)
{
	struct dpu_rect crop1_rect;

	if (stripe_param->wb_crop1_enable) {
		crop1_rect = stripe_rect_to_dpu_rect(stripe_param->wb_crop1_rect);
		stripe_frame_cfg->crop1_en = true;
		stripe_frame_cfg->crop1_area.x1 = crop1_rect.x1;
		stripe_frame_cfg->crop1_area.y1 = crop1_rect.y1;
		stripe_frame_cfg->crop1_area.x2 = crop1_rect.x2;
		stripe_frame_cfg->crop1_area.y2 = crop1_rect.y2;
	} else {
		stripe_frame_cfg->crop1_en = false;
	}
}

static void wb_stripe_crop0_config(struct wb_stripe_frame_cfg *stripe_frame_cfg,
		struct display_wb_stripe_info *stripe_param)
{
	struct dpu_rect crop0_rect;

	if (stripe_param->wb_crop0_enable) {
		crop0_rect = stripe_rect_to_dpu_rect(stripe_param->wb_crop0_rect);
		stripe_frame_cfg->crop0_en = true;
		stripe_frame_cfg->crop0_area.x1 = crop0_rect.x1;
		stripe_frame_cfg->crop0_area.y1 = crop0_rect.y1;
		stripe_frame_cfg->crop0_area.x2 = crop0_rect.x2;
		stripe_frame_cfg->crop0_area.y2 = crop0_rect.y2;
	} else {
		stripe_frame_cfg->crop0_en = false;
	}
}

static void dpu_wb_stripe_scaler_config(struct dpu_1d_scaler_cfg *wb_stripe_1d_scaler_cfg,
		struct display_wb_stripe_info *wb_stripe_cfg)
{
	wb_stripe_1d_scaler_cfg->input_height = wb_stripe_cfg->wb_scaler_param.scaler_in_rect.h;
	wb_stripe_1d_scaler_cfg->input_width = wb_stripe_cfg->wb_scaler_param.scaler_in_rect.w;
	wb_stripe_1d_scaler_cfg->output_height = wb_stripe_cfg->wb_scaler_param.scaler_out_rect.h;
	wb_stripe_1d_scaler_cfg->output_width = wb_stripe_cfg->wb_scaler_param.scaler_out_rect.w;
	wb_stripe_1d_scaler_cfg->hor_delta_phase = wb_stripe_cfg->wb_scaler_param.scaler_hor_delta;
	wb_stripe_1d_scaler_cfg->ver_delta_phase = wb_stripe_cfg->wb_scaler_param.scaler_ver_delta;
	wb_stripe_1d_scaler_cfg->hor_init_phase_h1b = (wb_stripe_cfg->wb_scaler_param.scaler_hor_init_phase &
									SCALER_PHASE_HIGH_1_BIT) >> 32;
	wb_stripe_1d_scaler_cfg->hor_init_phase_l32b = wb_stripe_cfg->wb_scaler_param.scaler_hor_init_phase &
									SCALER_PHASE_LOW_32_BIT;
	wb_stripe_1d_scaler_cfg->ver_init_phase_h1b = (wb_stripe_cfg->wb_scaler_param.scaler_ver_init_phase &
									SCALER_PHASE_HIGH_1_BIT) >> 32;
	wb_stripe_1d_scaler_cfg->ver_init_phase_l32b = wb_stripe_cfg->wb_scaler_param.scaler_ver_init_phase &
									SCALER_PHASE_LOW_32_BIT;
}

static void dpu_stripe_wb_connector_atomic_commit(struct dpu_hw_stripe *hw_stripe,
		struct drm_connector *connector)
{
	struct dpu_1d_scaler_cfg wb_stripe_1d_scaler_cfg;
	struct drm_writeback_connector *drm_wb_conn;
	struct wb_stripe_frame_cfg stripe_frame_cfg;
	struct display_wb_stripe_info wb_stripe_cfg;
	struct dpu_wb_connector_state *wb_state;
	struct drm_connector_state *conn_state;
	struct dpu_hw_wb_scaler *hw_wb_scaler;
	struct dpu_wb_connector *dpu_wb_conn;
	struct drm_property_blob *blob_para;
	struct drm_writeback_job *job;
	struct drm_framebuffer *fb;
	struct dpu_hw_wb *hw_wb;
	unsigned long flags;
	s64 node_id;
	u32 i;

	STRIPE_DEBUG("stripe writeback commit\n");

	if (!g_stripe_enable) {
		STRIPE_DEBUG("stripe disable\n");
		return;
	}

	if (!connector) {
		DPU_ERROR("invalid parameters, %pK\n", connector);
		return;
	}

	conn_state = connector->state;
	wb_state = to_dpu_wb_connector_state(conn_state);

	drm_wb_conn = drm_connector_to_writeback(connector);
	spin_lock_irqsave(&drm_wb_conn->job_lock, flags);
	job = list_first_entry_or_null(&drm_wb_conn->job_queue,
			struct drm_writeback_job,
			list_entry);
	spin_unlock_irqrestore(&drm_wb_conn->job_lock, flags);
	if (!job) {
		DPU_ERROR("writeback job NULL");
		return;
	}

	fb = job->fb;
	if ((fb == NULL) || (fb->format == NULL)) {
		DPU_ERROR("writeback_job fb or fb->format is NULL");
		return;
	}

	if (wb_state->wb_stripe_count <= MIN_STRIPE_COUNT) {
		STRIPE_DEBUG("writeback stripe disable");
		return;
	}

	dpu_wb_conn = to_dpu_wb_connector(drm_connector_to_writeback(connector));
	hw_wb = dpu_wb_conn->hw_wb;

	if (hw_wb) {
		/* get wb stripe para ptr */
		blob_para = drm_property_lookup_blob(connector->dev, wb_state->stripe_para_blob_id);
		if (wb_state->stripe_para_blob_ptr)
			drm_property_blob_put(wb_state->stripe_para_blob_ptr);
		wb_state->stripe_para_blob_ptr = blob_para;

		/* set cfg to cmdlist regs */
		wb_stripe_top_config_from_state(&stripe_frame_cfg, wb_state, fb);

		for (i = 0; i < wb_state->wb_stripe_count; i++) {
			memcpy(&wb_stripe_cfg,
				(struct display_wb_stripe_info *)(blob_para->data) + i,
				sizeof(struct display_wb_stripe_info));

			if (i == 1) {
				STRIPE_DEBUG("wb_input_rect x:%d y:%d w:%d h:%d\n", wb_stripe_cfg.wb_input_rect.x,
					wb_stripe_cfg.wb_input_rect.y, wb_stripe_cfg.wb_input_rect.w,
					wb_stripe_cfg.wb_input_rect.h);
			}

			node_id = (i == 0) ?
					*hw_wb->hw.cmd_node : dpu_wb_conn->wb_slice_node_id[i - 1];
			STRIPE_DEBUG("dpu_wb_conn node_id == %lld\n", node_id);

			wb_stripe_input_rect_config(&stripe_frame_cfg, &wb_stripe_cfg);
			wb_stripe_output_rect_config(&stripe_frame_cfg, &wb_stripe_cfg);
			wb_stripe_output_subframe_config(&stripe_frame_cfg, &wb_stripe_cfg);
			wb_stripe_crop2_config(&stripe_frame_cfg, &wb_stripe_cfg);
			wb_stripe_crop1_config(&stripe_frame_cfg, &wb_stripe_cfg);
			wb_stripe_crop0_config(&stripe_frame_cfg, &wb_stripe_cfg);

			if (hw_wb && hw_stripe)
				hw_stripe->ops.wb_frame_stripe_config(&hw_wb->hw, node_id, &stripe_frame_cfg);

			if (wb_stripe_cfg.scaler_enable) {
				dpu_wb_stripe_scaler_config(&wb_stripe_1d_scaler_cfg, &wb_stripe_cfg);
				hw_wb_scaler = dpu_wb_conn->hw_wb_scaler;
				if (hw_wb_scaler && hw_stripe)
					hw_stripe->ops.wb_scaler_stripe_set(&hw_wb_scaler->hw,
						node_id, &wb_stripe_1d_scaler_cfg);
			}
		}
	}
}

static void dpu_stripe_crtc_mixer_commit(struct dpu_hw_stripe *hw_stripe,
		struct drm_atomic_state *state, struct drm_crtc *crtc)
{
	struct drm_plane *plane = NULL;
	struct dpu_crtc_state *dpu_cstate;
	struct dpu_hw_mixer *hw_mixer;
	struct dpu_crtc *dpu_crtc;
	struct drm_crtc_state *crtc_state;
	struct drm_connector *connector;
	u32 mask;

	dpu_crtc = to_dpu_crtc(crtc);
	if (!dpu_crtc) {
		DPU_ERROR("failed to get dpu crtc\n");
		return;
	}
	crtc_state = crtc->state;
	hw_mixer = dpu_crtc->hw_mixer;

	dpu_cstate = to_dpu_crtc_state(crtc->state);
	if (!dpu_cstate) {
		DPU_ERROR("failed to get dpu crtc state\n");
		return;
	}
	/* mixer commit */
	if (hw_mixer) {
		for_each_connector_per_crtc(connector, crtc_state, mask) {
			if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK) {
				drm_atomic_crtc_for_each_plane(plane, crtc) {
					dpu_stripe_crtc_mixer_input_commit(hw_stripe,
						hw_mixer, plane, connector, dpu_cstate);
				}
			}
		}

		for_each_connector_per_crtc(connector, crtc_state, mask) {
			if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK) {
				dpu_stripe_crtc_mixer_output_commit(hw_stripe, hw_mixer, connector, dpu_cstate);
				dpu_stripe_wb_connector_atomic_commit(hw_stripe, connector);
			}
		}
	}
}

static void dpu_rdma_stripe_scaler_config(struct dpu_2d_scaler_cfg *rdma_stripe_2d_scaler_cfg,
		struct display_rch_stripe_info *plane_stripe_cfg)
{
	rdma_stripe_2d_scaler_cfg->stripe_height_in = plane_stripe_cfg->layer_scaler_stripe.scaler_in_rect.h;
	rdma_stripe_2d_scaler_cfg->stripe_width_in = plane_stripe_cfg->layer_scaler_stripe.scaler_in_rect.w;
	rdma_stripe_2d_scaler_cfg->stripe_height_out = plane_stripe_cfg->layer_scaler_stripe.scaler_out_rect.h;
	rdma_stripe_2d_scaler_cfg->stripe_width_out = plane_stripe_cfg->layer_scaler_stripe.scaler_out_rect.w;
	rdma_stripe_2d_scaler_cfg->x_step = plane_stripe_cfg->layer_scaler_stripe.scaler_hor_delta;
	rdma_stripe_2d_scaler_cfg->y_step = plane_stripe_cfg->layer_scaler_stripe.scaler_ver_delta;
	rdma_stripe_2d_scaler_cfg->stripe_init_phase = plane_stripe_cfg->layer_scaler_stripe.scaler_hor_init_phase;
}

static void dpu_plane_set_stripe_config(struct dpu_hw_stripe *hw_stripe,
		struct dpu_hw_rch *hw_rch, struct drm_plane *plane,
		enum rdma_extender_mode mode)
{
	struct rdma_layer_stripe_cfg stripe_layer_cfg = {0};
	struct display_rch_stripe_info plane_stripe_cfg;
	struct dpu_2d_scaler_cfg stripe_pre_scaler_cfg;
	struct dpu_plane_state *dpu_plane_state;
	struct dpu_plane *dpu_plane;
	struct drm_property_blob *blob;
	struct plane_color *color;
	bool first_node_flag = true;
	u8 valid_node_idx = 0;
	s64 node_id;
	u32 i;

	if (!g_stripe_enable) {
		STRIPE_DEBUG("stripe disable\n");
		return;
	}

	dpu_plane = to_dpu_plane(plane);
	dpu_plane_state = to_dpu_plane_state(plane->state);
	if (dpu_plane_state->plane_stripe_count <= MIN_STRIPE_COUNT) {
		DPU_ERROR("writeback stripe disable");
		return;
	}

	/* get plane stripe para ptr */
	blob = drm_property_lookup_blob(plane->dev, dpu_plane_state->stripe_para_blob_id);
	if (dpu_plane_state->stripe_para_blob_ptr)
		drm_property_blob_put(dpu_plane_state->stripe_para_blob_ptr);

	dpu_plane_state->stripe_para_blob_ptr = blob;

	/* set cfg to cmdlist regs */
	for (i = 0; i < dpu_plane_state->plane_stripe_count; i++) {
		memcpy(&plane_stripe_cfg,
			(struct display_rch_stripe_info *)(blob->data) + i,
			sizeof(struct display_rch_stripe_info));

		STRIPE_DEBUG("plane_stripe_cfg crop_area == x %d, y %d, w %d, h %d",
				plane_stripe_cfg.layer_crop_rect.x, plane_stripe_cfg.layer_crop_rect.y,
				plane_stripe_cfg.layer_crop_rect.w, plane_stripe_cfg.layer_crop_rect.h);
		if (plane_stripe_cfg.layer_crop_rect.w != 0) {
			stripe_layer_cfg.crop_area.x1 = plane_stripe_cfg.layer_crop_rect.x;
			stripe_layer_cfg.crop_area.y1 = plane_stripe_cfg.layer_crop_rect.y;
			stripe_layer_cfg.crop_area.x2 = plane_stripe_cfg.layer_crop_rect.x +
				plane_stripe_cfg.layer_crop_rect.w - 1;
			stripe_layer_cfg.crop_area.y2 = plane_stripe_cfg.layer_crop_rect.y +
				plane_stripe_cfg.layer_crop_rect.h - 1;
			node_id = (first_node_flag) ? *hw_rch->hw.cmd_node :
				dpu_plane->plane_slice_node_id[valid_node_idx++];
			first_node_flag = false;
			if (hw_stripe)
				hw_stripe->ops.layer_stripe_config(&hw_rch->hw, node_id, &stripe_layer_cfg, mode);
		} else {
			stripe_layer_cfg.crop_area.x1 = 0;
			stripe_layer_cfg.crop_area.y1 = 0;
			stripe_layer_cfg.crop_area.x2 = 0;
			stripe_layer_cfg.crop_area.y2 = 0;
			continue;
		}

		/* pre scaler commit */
		if (dpu_plane->color && hw_stripe) {
			color = dpu_plane->color;
			if ((plane_stripe_cfg.layer_scaler_enable) && (dpu_plane->color->hw_scaler)) {
				dpu_rdma_stripe_scaler_config(&stripe_pre_scaler_cfg, &plane_stripe_cfg);
				color->hw_scaler->ops.enable(&color->hw_scaler->hw, true);
				hw_stripe->ops.layer_scaler_stripe_set(&color->hw_scaler->hw,
					node_id, &stripe_pre_scaler_cfg);
			}
		}
	}
}

static void dpu_stripe_plane_commit(struct dpu_hw_stripe *hw_stripe,
		struct drm_atomic_state *state, struct drm_plane *plane)
{
	struct dpu_plane_state *dpu_plane_state;
	struct dpu_plane *dpu_plane;
	struct dpu_hw_rch *hw_rch;
	enum rdma_extender_mode extender_mode;

	if (!g_stripe_enable)
		return;

	if (!plane || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				plane, state);
		return;
	}

	dpu_plane = to_dpu_plane(plane);
	if (!dpu_plane) {
		DPU_ERROR("invalid parameters, %pK\n", dpu_plane);
		return;
	}

	dpu_plane_state = to_dpu_plane_state(plane->state);
	if (!dpu_plane_state) {
		DPU_ERROR("invalid parameters, %pK\n", dpu_plane_state);
		return;
	}
	extender_mode = dpu_plane_state->extender_mode;

	hw_rch = dpu_plane->hw_rch;
	if (hw_rch && hw_stripe)
		dpu_plane_set_stripe_config(hw_stripe, hw_rch, plane, extender_mode);

}

void dpu_stripe_commit(struct drm_atomic_state *state)
{
	struct drm_plane_state *old_plane_state;
	struct drm_plane_state *new_plane_state;
	struct dpu_hw_stripe *hw_stripe;
	struct drm_crtc_state *old_state;
	struct drm_crtc_state *new_state;
	struct drm_plane *plane;
	struct drm_crtc *crtc;
	int i;

	hw_stripe = dpu_stripe_res_init();

	for_each_oldnew_crtc_in_state(state, crtc, old_state, new_state, i)
		dpu_stripe_crtc_mixer_commit(hw_stripe, state, crtc);

	for_each_oldnew_plane_in_state(state, plane, old_plane_state, new_plane_state, i)
		dpu_stripe_plane_commit(hw_stripe, state, plane);

	g_stripe_enable = false;
	dpu_stripe_res_deinit(hw_stripe);
}
