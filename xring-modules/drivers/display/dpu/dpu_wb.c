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

#include <linux/platform_device.h>
#include <linux/component.h>
#include <linux/of.h>
#include <linux/version.h>
#include <soc/xring/xr_dmabuf_helper.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_writeback.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_vblank.h>

#include "dpu_kms.h"
#include "dpu_sec_drm.h"
#include "dpu_wb.h"
#include "dpu_gem.h"
#include "dpu_module_drv.h"
#include "dpu_format.h"
#include "dpu_crtc.h"
#include "dksm_iommu.h"
#include "dpu_wfd.h"
#include "dpu_flow_ctrl.h"
#include "dpu_trace.h"

static u32 default_wb_id;

#define CROP_MAX_VALUE 4096
#define ALIGN_DOWN_TWO(number) ((number) & 0xfffe)

#define MIN_WB_OUTPUT_WIDTH_LIMIT 2048
#define LOG_OF_MAX_SCALE_RATIO 5
#define LOG_OF_MIN_SCALE_RATIO 4

#define AFBCE_TILE_16X16_WIDTH_HEIGHT_ALIGN 128
#define AFBCE_NON_TILE_16X16_WIDTH_HEIGHT_ALIGN 16

#define AFBCE_TILE_32X8_WIDTH_ALIGN 256
#define AFBCE_TILE_32X8_HEIGHT_ALIGN 64
#define AFBCE_NON_TILE_32X8_WIDTH_ALIGN 32
#define AFBCE_NON_TILE_32X8_HEIGHT_ALIGN 8

#define HEADER_SIZE_BEFORE_OVERALL_ALIGN(width, height) \
		((width) * (height) / 256 * 16)

#define CHECK_VALUE_ALIGN(value, align) ((value) % (align) ? false : true)


#ifdef PRODUCT_TYPE_ASIC
#define WB_DONE_DURATION_NS 500000000 // 500ms
#define WB_DONE_DURATION_S 2
#else
#define WB_DONE_DURATION_NS 0
#define WB_DONE_DURATION_S 5
#endif

static const u64 wb_format_modifiers[] = {
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_NONE,
			AFBC_NONE, AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_NONE,
			AFBC_NONE, AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_NONE,
			AFBC_NONE, AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_NONE,
			AFBC_NONE, AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_NONE,
			AFBC_SPLIT, AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_NONE,
			AFBC_SPLIT, AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_NONE,
			AFBC_SPLIT, AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_NONE,
			AFBC_SPLIT, AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_YTR,
			AFBC_NONE,  AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_YTR,
			AFBC_NONE,  AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_YTR,
			AFBC_NONE,  AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_YTR,
			AFBC_NONE,  AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_YTR,
			AFBC_SPLIT, AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_YTR,
			AFBC_SPLIT, AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_YTR,
			AFBC_SPLIT, AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_NONE,  AFBC_YTR,
			AFBC_SPLIT, AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_NONE,
			AFBC_NONE,  AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_NONE,
			AFBC_NONE,  AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_NONE,
			AFBC_NONE,  AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_NONE,
			AFBC_NONE,  AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_NONE,
			AFBC_SPLIT, AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_NONE,
			AFBC_SPLIT, AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_NONE,
			AFBC_SPLIT, AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_NONE,
			AFBC_SPLIT, AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_YTR,
			AFBC_NONE,  AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_YTR,
			AFBC_NONE,  AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_YTR,
			AFBC_NONE,  AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_YTR,
			AFBC_NONE,  AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_YTR,
			AFBC_SPLIT, AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_YTR,
			AFBC_SPLIT, AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_YTR,
			AFBC_SPLIT, AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_16x16, AFBC_TILED, AFBC_YTR,
			AFBC_SPLIT, AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_NONE,
			AFBC_NONE,  AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_NONE,
			AFBC_NONE,  AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_NONE,
			AFBC_NONE,  AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_NONE,
			AFBC_NONE,  AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_NONE,
			AFBC_SPLIT, AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_NONE,
			AFBC_SPLIT, AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_NONE,
			AFBC_SPLIT, AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_NONE,
			AFBC_SPLIT, AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_YTR,
			AFBC_NONE,  AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_YTR,
			AFBC_NONE,  AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_YTR,
			AFBC_NONE,  AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_YTR,
			AFBC_NONE,  AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_YTR,
			AFBC_SPLIT, AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_YTR,
			AFBC_SPLIT, AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_YTR,
			AFBC_SPLIT, AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_NONE,  AFBC_YTR,
			AFBC_SPLIT, AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_NONE,
			AFBC_NONE,  AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_NONE,
			AFBC_NONE,  AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_NONE,
			AFBC_NONE,  AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_NONE,
			AFBC_NONE,  AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_NONE,
			AFBC_SPLIT, AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_NONE,
			AFBC_SPLIT, AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_NONE,
			AFBC_SPLIT, AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_NONE,
			AFBC_SPLIT, AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_YTR,
			AFBC_NONE,  AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_YTR,
			AFBC_NONE,  AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_YTR,
			AFBC_NONE,  AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_YTR,
			AFBC_NONE,  AFBC_COLOR, AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_YTR,
			AFBC_SPLIT, AFBC_NONE,  AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_YTR,
			AFBC_SPLIT, AFBC_COLOR, AFBC_NONE),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_YTR,
			AFBC_SPLIT, AFBC_NONE,  AFBC_COPY),
	WB_AFBC_FORMAT(AFBC_32x8,  AFBC_TILED, AFBC_YTR,
			AFBC_SPLIT, AFBC_COLOR, AFBC_COPY),
};

struct wb_size {
	u16 w;
	u16 h;
};

static const struct drm_display_mode wb_default_mode[] = {
	{ DRM_MODE("1440x3200", DRM_MODE_TYPE_DRIVER, 302250, 1440, 1472,
			1490, 1550, 0, 3200, 3216, 3220, 3250, 0,
			DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC) },

	/* 4096x2160@60Hz */
	{ DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 693264, 4096, 5128,
			5160, 5200, 0, 2160, 2208, 2216, 2222, 0,
			DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC) },

	{ DRM_MODE("4096x4096", DRM_MODE_TYPE_DRIVER, 1058400, 4096, 4106,
			4150, 4200, 0, 4096, 4106, 4150, 4200, 0,
			DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC) },
};

static int dpu_wb_connector_get_modes(struct drm_connector *connector)
{
	struct drm_display_mode *mode = NULL;
	struct drm_device *dev;
	int i = 0;

	if (!connector) {
		DPU_ERROR("invalid parameters, %pK\n", connector);
		return -EINVAL;
	}

	dev = connector->dev;

	for (i = 0; i < ARRAY_SIZE(wb_default_mode); i++) {
		mode = drm_mode_duplicate(dev, &wb_default_mode[i]);
		if (mode)
			drm_mode_probed_add(connector, mode);
	}

	return ARRAY_SIZE(wb_default_mode);
}

static enum drm_mode_status dpu_wb_connector_mode_valid(
		struct drm_connector *connector,
		struct drm_display_mode *mode)
{
	enum drm_mode_status mode_status = MODE_OK;

	if (!connector || !mode) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				connector, mode);
		return MODE_ERROR;
	}

	return mode_status;
}

static int dpu_wb_online_src_rect_check(struct dpu_wb_connector_state *wb_state,
		struct drm_display_mode *mode)
{
	if ((wb_state->src_rect.w > mode->hdisplay) ||
			(wb_state->src_rect.h > mode->vdisplay)) {
		DPU_ERROR("online wb src_rect size exceeds the input size\n");
		return -EINVAL;
	}

	return 0;
}

int dpu_wb_connector_atomic_check(struct drm_connector *connector,
		struct drm_atomic_state *state)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector_state *wb_state;
	struct dpu_wb_connector *dpu_wb_conn;
	struct drm_display_mode *mode;
	bool is_online;
	int ret = 0;

	if (!connector || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				connector, state);
		return -EINVAL;
	}

	mode = &connector->state->crtc->mode;
	wb_state = to_dpu_wb_connector_state(connector->state);
	drm_wb_conn = drm_connector_to_writeback(connector);
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);

	is_online = is_clone_mode(state, connector->state);
	if (is_online && (wb_state->pos_type == WB_POS_TYPE_AFTER_POSTPQ ||
					wb_state->pos_type == WB_POS_TYPE_AFTER_RC ||
					wb_state->pos_type == WB_POS_TYPE_AFTER_MIXER)) {
		ret = dpu_wb_online_src_rect_check(wb_state, mode);
		if (ret) {
			DPU_ERROR("failed to do online wb src rect check, ret %d\n", ret);
			return ret;
		}
	}

	return ret;
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

static int dpu_get_afbc_header_size(bool is_tile, bool is_afbc_32x8,
		struct drm_framebuffer *fb)
{
	uint16_t aligned_height;
	uint16_t aligned_width;
	uint32_t header_size;
	uint16_t width;
	uint16_t height;

	width = fb->width;
	height = fb->height;

	if (!is_afbc_32x8) {
		aligned_width = is_tile ?
				ALIGN(width, AFBCE_TILE_16X16_WIDTH_HEIGHT_ALIGN) :
				ALIGN(width, AFBCE_NON_TILE_16X16_WIDTH_HEIGHT_ALIGN);
		aligned_height = is_tile ?
				ALIGN(height, AFBCE_TILE_16X16_WIDTH_HEIGHT_ALIGN) :
				ALIGN(height, AFBCE_NON_TILE_16X16_WIDTH_HEIGHT_ALIGN);
	} else {
		aligned_width = is_tile ?
				ALIGN(width, AFBCE_TILE_32X8_WIDTH_ALIGN) :
				ALIGN(width, AFBCE_NON_TILE_32X8_WIDTH_ALIGN);
		aligned_height = is_tile ?
				ALIGN(height, AFBCE_TILE_32X8_HEIGHT_ALIGN) :
				ALIGN(height, AFBCE_NON_TILE_32X8_HEIGHT_ALIGN);
	}
	header_size = HEADER_SIZE_BEFORE_OVERALL_ALIGN(aligned_width,
			aligned_height);
	header_size = ALIGN(header_size, PAGE_SIZE);

	DPU_DEBUG("afbc mode is_tile = %d is_32x8 = %d, dpu calc header size %u",
			is_tile, is_afbc_32x8, header_size);

	return header_size;
}

static int dpu_wb_scan2tile_buffer_format_bpp(enum dpu_wb_format wb_format)
{
	u8 bpp;

	switch (wb_format) {
	case WB_FORMAT_ARGB_2101010:
	case WB_FORMAT_ABGR_2101010:
	case WB_FORMAT_RGBA_1010102:
	case WB_FORMAT_BGRA_1010102:
	case WB_FORMAT_ARGB_8888:
	case WB_FORMAT_ABGR_8888:
	case WB_FORMAT_RGBA_8888:
	case WB_FORMAT_BGRA_8888:
	case WB_FORMAT_XRGB_8888:
	case WB_FORMAT_XBGR_8888:
	case WB_FORMAT_RGBX_8888:
	case WB_FORMAT_BGRX_8888:
	case WB_FORMAT_RGB_888:
	case WB_FORMAT_BGR_888:
	case WB_FORMAT_NV21_10:
	case WB_FORMAT_NV12_10:
		bpp = 32;
		break;
	default:
		bpp = 16;
		break;
	}

	return bpp;
}

static void dpu_wb_get_input_size(struct dpu_crtc_state *dpu_cstate,
		struct drm_display_mode *mode, struct dpu_wb_connector_state *wb_state,
		struct wb_size *input_size)
{
	u32 crtc_size_witdh;
	u32 crtc_size_height;

	switch (wb_state->pos_type) {
	case WB_POS_TYPE_AFTER_MIXER:
		get_crtc_size(dpu_cstate, &crtc_size_witdh, &crtc_size_height);
		input_size->w = crtc_size_witdh;
		input_size->h = crtc_size_height;
		break;
	case WB_POS_TYPE_AFTER_POSTPQ:
	case WB_POS_TYPE_AFTER_RC:
		input_size->w = mode->crtc_hdisplay;
		input_size->h = mode->crtc_vdisplay;
		break;
	case WB_POS_TYPE_BEFORE_PREPQ:
	case WB_POS_TYPE_AFTER_PREPQ:
		input_size->w = wb_state->prepq_wb.prepq_dst_width;
		input_size->h = wb_state->prepq_wb.prepq_dst_height;
		break;
	default:
		DPU_ERROR("invalid wb position type: %u\n", wb_state->pos_type);
		break;
	}
}

static void dpu_wb_scaler_commit(struct dpu_wb_connector *wb,
		struct dpu_wb_connector_state *wb_state)
{
	struct dpu_hw_wb_scaler *hw_wb_scaler;

	hw_wb_scaler = wb->hw_wb_scaler;
	if (!hw_wb_scaler) {
		DPU_ERROR("hw_wb_scaler is null");
		return;
	}

	if (wb_state->scaler_enable) {
		hw_wb_scaler->ops.enable(&hw_wb_scaler->hw, true);
		hw_wb_scaler->ops.set(&hw_wb_scaler->hw,
				(struct dpu_1d_scaler_cfg *)wb_state->scaler_blob_ptr->data);
	} else {
		hw_wb_scaler->ops.enable(&hw_wb_scaler->hw, false);
		return;
	}
	DPU_DEBUG("wb scaler_en=%d commit done\n", wb_state->scaler_enable);
}

static void dpu_wb_set_data_flow(struct wb_size *data_flow,
		u16 w, u16 h)
{
	data_flow->w = w;
	data_flow->h = h;
}

static void dpu_wb_set_input_config(struct wb_frame_cfg *wb_cfg,
		struct drm_connector *connector, struct drm_atomic_state *state,
		struct wb_size *data_flow)
{
	struct dpu_wb_connector_state *wb_state;
	struct drm_connector_state *conn_state;
	struct dpu_crtc_state *dpu_cstate;
	struct drm_display_mode *mode;
	struct wb_size input_size;
	bool is_online;

	dpu_cstate = to_dpu_crtc_state(connector->state->crtc->state);
	conn_state = connector->state;
	wb_state = to_dpu_wb_connector_state(conn_state);
	mode = &conn_state->crtc->mode;

	is_online = is_clone_mode(state, conn_state);

	dpu_wb_get_input_size(dpu_cstate, mode, wb_state, &input_size);
	wb_cfg->input_width = input_size.w;
	wb_cfg->input_height = input_size.h;

	dpu_wb_set_data_flow(data_flow, wb_cfg->input_width, wb_cfg->input_height);

	wb_cfg->is_offline_mode = !is_online;
	DPU_DEBUG("wb connector %d mode: %s\n", connector->index,
			wb_cfg->is_offline_mode ? "offline" : "online");
}

static void dpu_wb_set_crop0_config(struct wb_frame_cfg *wb_cfg,
		struct dpu_general_rect src_rect, struct wb_size *data_flow)
{
	if (src_rect.w && src_rect.h && ((src_rect.w != data_flow->w) ||
			(src_rect.h != data_flow->h))) {
		wb_cfg->crop0_en = true;

		/**
		 * hardware constraint:
		 * src_rect horizontal starting coordinates must be even
		 */
		if (src_rect.x % 2) {
			src_rect.x--;
			src_rect.w++;
		}

		wb_cfg->crop0_area.x1 = src_rect.x;
		wb_cfg->crop0_area.y1 = src_rect.y;
		wb_cfg->crop0_area.x2 = src_rect.x + src_rect.w - 1;
		wb_cfg->crop0_area.y2 = src_rect.y + src_rect.h - 1;
		dpu_wb_set_data_flow(data_flow, src_rect.w, src_rect.h);
	}
}

static void dpu_wb_set_1dscaler_config(struct wb_frame_cfg *wb_cfg,
		struct dpu_wb_connector_state *wb_state,
		struct wb_size *data_flow)
{
	struct dpu_1d_scaler_cfg *wb_scaler_cfg;
	struct drm_property_blob *blob;

	wb_cfg->scl_en = wb_state->scaler_enable;

	if (!wb_state->scaler_enable)
		return;

	if (!wb_state->scaler_blob_ptr) {
		DPU_ERROR("scaler_blob_ptr is NULL\n");
		return;
	}

	blob = wb_state->scaler_blob_ptr;
	wb_scaler_cfg = (struct dpu_1d_scaler_cfg *)(blob->data);

	dpu_wb_set_data_flow(data_flow, wb_scaler_cfg->output_width,
			wb_scaler_cfg->output_height);
}

static void dpu_wb_set_crop1_config(struct wb_frame_cfg *wb_cfg,
		struct drm_framebuffer *fb, struct wb_size *data_flow)
{
	const struct dpu_format_map *format_map;

	format_map = dpu_format_get(FMT_MODULE_WB, fb->format->format,
			fb->modifier);

	if (format_map->is_yuv && (data_flow->h % 2 || data_flow->w % 2)) {
		wb_cfg->crop1_en = true;
		wb_cfg->crop1_area.x1 = 0;
		wb_cfg->crop1_area.y1 = 0;

		dpu_wb_set_data_flow(data_flow, ALIGN_DOWN_TWO(data_flow->w),
				ALIGN_DOWN_TWO(data_flow->h));
		wb_cfg->crop1_area.x2 = data_flow->w - 1;
		wb_cfg->crop1_area.y2 = data_flow->h - 1;
	}
}

static void dpu_wb_set_dfc_config(struct wb_frame_cfg *wb_cfg,
	struct drm_framebuffer *fb)
{
	const struct dpu_format_map *format_map;

	format_map = dpu_format_get(FMT_MODULE_WB, fb->format->format,
			fb->modifier);

	wb_cfg->output_format = format_map->dpu_format;
	wb_cfg->is_yuv = format_map->is_yuv;
	wb_cfg->is_uv_swap = format_map->uv_swap;
	wb_cfg->plane_count = format_map->plane_count;
}

static void dpu_wb_set_rotation_flip_config(struct wb_frame_cfg *wb_cfg,
		u32 rotation, struct wb_size *data_flow)
{
	wb_cfg->frame_rot_mode = dpu_wb_get_rotation_info(rotation);
	wb_cfg->frame_flip_mode = dpu_wb_get_flip_info(rotation);

	switch (wb_cfg->frame_rot_mode) {
	case ROT_90:
	case ROT_270:
		dpu_wb_set_data_flow(data_flow, data_flow->h, data_flow->w);
		break;
	default:
		break;
	}
}

static void dpu_wb_set_afbce_config(struct wb_frame_cfg *wb_cfg,
		uint64_t modifier)
{
	wb_cfg->is_afbc = DPU_FMT_IS_AFBC(modifier);
	if (wb_cfg->is_afbc) {
		wb_cfg->is_afbc_32x8 = DPU_FMT_IS_AFBC_32x8(modifier);
		wb_cfg->is_afbc_split = DPU_FMT_HAS_AFBC_SPLIT(modifier);
		wb_cfg->is_afbc_yuv_transform = DPU_FMT_HAS_AFBC_Y2R(modifier);
		wb_cfg->is_afbc_tile_header_mode = DPU_FMT_HAS_AFBC_TILED(modifier);

		/**
		 * WB AFBCE copy mode & default color must be configured to be true to
		 * align with the GPU.
		 */
		wb_cfg->is_afbc_copy_mode = true;
		wb_cfg->is_afbc_default_color = true;
	}

}

static void dpu_wb_set_address_config(struct wb_frame_cfg *wb_cfg,
		struct drm_framebuffer *fb)
{
	struct dpu_gem_object *gem_obj;
	s64 afbc_header_page_count = 0;
	u64 afbc_header_size = 0;
	u64 dma_addr;

	gem_obj = to_dpu_gem_obj(fb->obj[0]);
	if (wb_cfg->is_afbc) {
		wb_cfg->plane_count = PLANE_TWO;

		afbc_header_page_count = xr_dmabuf_get_headersize(gem_obj->dmabuf);
		if (afbc_header_page_count > 0) {
			afbc_header_size = afbc_header_page_count * PAGE_SIZE;
		} else {
			if (afbc_header_page_count < 0)
				DPU_ERROR("get dmabuf afbc header size error!");

			/* If afbc scramble is off,
			 * wb needs to calculate the header size itself.
			 */
			afbc_header_size = dpu_get_afbc_header_size(
					wb_cfg->is_afbc_tile_header_mode,
					wb_cfg->is_afbc_32x8, fb);
		}
	}

	dma_addr = gem_obj->iova_dom ? gem_obj->iova_dom->iova : gem_obj->paddr;

	if (wb_cfg->plane_count == PLANE_ONE) {
		wb_cfg->wdma_base_addrs[WB_PLANAR_Y_RGB] = dma_addr +
				fb->offsets[WB_PLANAR_Y_RGB];
	} else if (wb_cfg->plane_count == PLANE_TWO) {
		if (wb_cfg->is_afbc) {
			wb_cfg->wdma_base_addrs[WB_PLANAR_Y_RGB] = dma_addr +
					fb->offsets[WB_PLANAR_Y_RGB];
			wb_cfg->wdma_base_addrs[WB_PLANAR_UV] = dma_addr +
					fb->offsets[WB_PLANAR_Y_RGB] + afbc_header_size;
		} else {
			wb_cfg->wdma_base_addrs[WB_PLANAR_Y_RGB] = dma_addr +
					fb->offsets[WB_PLANAR_Y_RGB];
			wb_cfg->wdma_base_addrs[WB_PLANAR_UV] = dma_addr +
					fb->offsets[WB_PLANAR_UV];
		}
	}
}

static void dpu_wb_set_original_output_size(struct wb_frame_cfg *wb_cfg,
		struct drm_framebuffer *fb)
{
	wb_cfg->output_original_width = fb->width;
	wb_cfg->output_original_height = fb->height;
}

static void dpu_wb_set_subframe_output_rect(struct wb_frame_cfg *wb_cfg,
		struct dpu_general_rect dst_rect)
{
	wb_cfg->output_subframe_rect.x = dst_rect.x;
	wb_cfg->output_subframe_rect.y = dst_rect.y;
	wb_cfg->output_subframe_rect.w = wb_cfg->is_yuv ? ALIGN_DOWN_TWO(dst_rect.w) : dst_rect.w;
	wb_cfg->output_subframe_rect.h = wb_cfg->is_yuv ? ALIGN_DOWN_TWO(dst_rect.h) : dst_rect.h;
}

static void dpu_wb_set_csc_config(struct wb_frame_cfg *wb_cfg,
		struct dpu_wb_connector_state *wb_state)
{
	struct dpu_csc_matrix_cfg *csc_cfg;
	struct drm_property_blob *blob;

	if (!wb_cfg->is_yuv)
		return;

	if (!wb_state->csc_blob_ptr) {
		DPU_ERROR("csc_blob_ptr is NULL\n");
		return;
	}

	blob = wb_state->csc_blob_ptr;
	csc_cfg = (struct dpu_csc_matrix_cfg *)(blob->data);
	memcpy(&wb_cfg->csc_cfg, csc_cfg, sizeof(struct dpu_csc_matrix_cfg));
}

static void dpu_wb_set_dither_config(struct wb_frame_cfg *wb_cfg,
		struct dpu_wb_connector_state *wb_state)
{
	struct dpu_dither_cfg *wb_dither_cfg;
	struct drm_property_blob *blob;

	wb_cfg->dither_en = wb_state->dither_enable;

	if (!wb_state->dither_enable)
		return;

	if (!wb_state->dither_blob_ptr) {
		DPU_ERROR("dither_blob_ptr is NULL\n");
		return;
	}

	blob = wb_state->dither_blob_ptr;
	wb_dither_cfg = (struct dpu_dither_cfg *)(blob->data);
	memcpy(&wb_cfg->dither_cfg, wb_dither_cfg, sizeof(struct dpu_dither_cfg));
}

static void dpu_wb_set_output_config(struct wb_frame_cfg *wb_cfg,
		struct drm_framebuffer *fb, struct dpu_general_rect dst_rect)
{
	dpu_wb_set_afbce_config(wb_cfg, fb->modifier);

	dpu_wb_set_address_config(wb_cfg, fb);

	if (!wb_cfg->is_afbc)
		wb_cfg->raw_format_stride = fb->pitches[0];

	dpu_wb_set_original_output_size(wb_cfg, fb);
	dpu_wb_set_subframe_output_rect(wb_cfg, dst_rect);
}

static enum dpu_hw_wb_position dpu_wb_get_input_position(
		enum dpu_hw_wb_pos_type pos_type, u32 scene_ctrl_blk_id,
		u8 prepq_id)
{
	enum dpu_hw_wb_position wb_position;

	switch (pos_type) {
	case WB_POS_TYPE_AFTER_MIXER:
		wb_position = GET_WB_POS_AFTER_MIXER(scene_ctrl_blk_id);
		break;
	case WB_POS_TYPE_AFTER_POSTPQ:
		wb_position = GET_WB_POS_AFTER_POSTPQ(scene_ctrl_blk_id);
		break;
	case WB_POS_TYPE_AFTER_RC:
		wb_position = WB_POS_AFTER_RC;
		break;
	case WB_POS_TYPE_BEFORE_PREPQ:
		wb_position = GET_WB_POS_BEFORE_PREPQ(prepq_id);
		break;
	case WB_POS_TYPE_AFTER_PREPQ:
		wb_position = GET_WB_POS_AFTER_PREPQ(prepq_id);
		break;

	/**
	 * Because atomic_check intercepts illegal writeback selection,
	 * it could not go to the default case
	 */
	default:
		wb_position = WB_POS_INVALID;
		break;
	}

	return wb_position;
}

static void dpu_wb_enable(struct dpu_wb_connector *dpu_wb_conn, bool enable,
		u8 prepq_id)
{
	struct dpu_wb_connector_state *dpu_wb_state;
	struct drm_connector_state *conn_state;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_hw_scene_ctl *scene_ctl;
	struct dpu_hw_ctl_top *hw_ctl_top;
	struct dpu_hw_wb_top *hw_wb_top;
	struct drm_connector *drm_conn;
	struct drm_crtc *drm_crtc;
	struct dpu_hw_wb *hw_wb;
	struct dpu_kms *kms;
	int wb_input_position;

	hw_wb = dpu_wb_conn->hw_wb;
	if (!hw_wb) {
		DPU_ERROR("hw_wb is null!\n");
		return;
	}

	drm_conn = &dpu_wb_conn->base.base;
	conn_state = drm_conn->state;
	drm_crtc = conn_state->crtc;
	if (!drm_crtc) {
		DPU_ERROR("drm_crtc is nullptr!\n");
		return;
	}

	dpu_drm_dev = to_dpu_drm_dev(drm_conn->dev);
	kms = dpu_drm_dev->dpu_kms;
	scene_ctl = kms->virt_pipeline[drm_crtc_index(drm_crtc)].hw_scene_ctl;
	hw_wb_top =  kms->virt_ctrl.hw_wb_top;
	dpu_wb_state = to_dpu_wb_connector_state(conn_state);

	scene_ctl->ops.wb_mount(&scene_ctl->hw, enable ? BIT(hw_wb->hw.blk_id) : 0);
	DPU_DEBUG("%s wb[%d] to scene ctl[%d]\n", enable ? "mount" : "unmount",
			hw_wb->hw.blk_id, scene_ctl->hw.blk_id);

	if (enable) {
		hw_ctl_top = kms->virt_ctrl.hw_ctl_top;
		wb_input_position = dpu_wb_get_input_position(
				dpu_wb_state->pos_type, scene_ctl->hw.blk_id, prepq_id);
		hw_ctl_top->ops.wb_input_position_config(&hw_ctl_top->hw,
				hw_wb->hw.blk_id, wb_input_position);

		hw_wb->ops.debug_enable(&hw_wb->hw, enable);
		hw_wb_top->ops.debug_enable(&hw_wb_top->hw, enable);
		hw_wb_top->ops.dbg_irq_enable(&hw_wb_top->hw, true);
	} else {
		if (dpu_wb_conn->is_online)
			return;

		scene_ctl->ops.rch_mount(&scene_ctl->hw, 0);
		scene_ctl->ops.cfg_ready_update(&scene_ctl->hw, NO_NEED_UPDT_EN,
				dpu_get_tui_state());

		dpu_wb_state->fb = NULL;
		DPU_DEBUG("clear dpu wb state\n");
	}
}

void dpu_wb_connector_atomic_commit(struct drm_connector *connector,
		struct drm_atomic_state *state)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector_state *wb_state;
	struct drm_connector_state *conn_state;
	struct dpu_wb_connector *dpu_wb_conn;
	struct framebuffer_info fb_info;
	struct wb_frame_cfg wb_cfg;
	struct drm_framebuffer *fb;
	struct dpu_hw_wb *hw_wb;
	struct wb_size data_flow;
	struct dpu_flow_ctrl *flow_ctrl;

	DPU_DEBUG("writeback commit\n");

	if (!connector || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				connector, state);
		return;
	}

	conn_state = connector->state;
	wb_state = to_dpu_wb_connector_state(conn_state);
	drm_wb_conn = drm_connector_to_writeback(connector);
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);
	hw_wb = dpu_wb_conn->hw_wb;

	if (!conn_state->writeback_job) {
		DPU_ERROR("writeback_job is NULL");
		return;
	}

	fb = conn_state->writeback_job->fb;
	if ((fb == NULL) || (fb->format == NULL)) {
		DPU_ERROR("writeback_job fb or fb->format is NULL");
		return;
	}

	dpu_sec_drm_wb_cfg(&wb_cfg, state, connector);
	dpu_wb_set_input_config(&wb_cfg, connector, state, &data_flow);

	dpu_wb_set_crop0_config(&wb_cfg, wb_state->src_rect, &data_flow);

	dpu_wb_set_1dscaler_config(&wb_cfg, wb_state, &data_flow);

	dpu_wb_set_dither_config(&wb_cfg, wb_state);

	dpu_wb_set_crop1_config(&wb_cfg, fb, &data_flow);

	dpu_wb_set_dfc_config(&wb_cfg, fb);

	dpu_wb_set_rotation_flip_config(&wb_cfg, wb_state->rotation, &data_flow);

	dpu_wb_set_output_config(&wb_cfg, fb, wb_state->dst_rect);

	dpu_wfd_source_config(&wb_cfg, wb_state, fb);

	dpu_wb_set_csc_config(&wb_cfg, wb_state);

	dpu_flow_qos_get(FLOW_QOS_WB_WDMA, &wb_cfg.is_offline_mode, &wb_cfg.qos);

	if (hw_wb) {
		hw_wb->ops.reset(&hw_wb->hw);
		hw_wb->ops.frame_config(&hw_wb->hw, &wb_cfg);

		dpu_gem_get_framebuffer_info(fb, &fb_info);
		dpu_flow_qos_get(FLOW_QOS_MMU, NULL, &fb_info.qos);

		hw_wb->ops.mmu_tbu_config(&hw_wb->hw, &fb_info, MODULE_WDMA);

		flow_ctrl = dpu_flow_ctrl_get(state->dev);
		dpu_flow_line_buf_level_cfg(flow_ctrl, &hw_wb->hw, FLOW_LBUF_LEVEL_WB, NULL);
	}

	dpu_wb_scaler_commit(dpu_wb_conn, wb_state);

	dpu_wb_enable(dpu_wb_conn, true, wb_state->prepq_wb.prepq_id);

	drm_writeback_queue_job(drm_wb_conn, conn_state);

	wb_state->fb = fb;

	DPU_DEBUG("writeback commit end\n");
}

enum drm_connector_status dpu_wb_connector_detect(
		struct drm_connector *connector, bool force)
{
	if (!connector) {
		DPU_ERROR("invalid parameters, %pK\n", connector);
		return -EINVAL;
	}

	return connector_status_connected;
}

static void dpu_wb_connector_destroy(struct drm_connector *connector)
{
	DPU_DEBUG("writeback destroy\n");

	if (!connector) {
		DPU_ERROR("invalid parameters, %pK\n", connector);
		return;
	}

	drm_connector_unregister(connector);
	drm_connector_cleanup(connector);
}

static int drm_atomic_replace_property_blob_from_id(struct drm_device *dev,
		struct drm_property_blob **blob,
		uint64_t blob_id,
		ssize_t expected_size,
		ssize_t expected_elem_size,
		bool *replaced)
{
	struct drm_property_blob *new_blob = NULL;

	if (blob_id != 0) {
		new_blob = drm_property_lookup_blob(dev, blob_id);
		if (new_blob == NULL) {
			DPU_ERROR("cannot find blob ID %llu\n", blob_id);
			return -EINVAL;
		}

		if (expected_size > 0 &&
		    new_blob->length != expected_size) {
			DPU_ERROR("[BLOB:%d] length %zu different from expected %zu\n",
					new_blob->base.id, new_blob->length, expected_size);
			drm_property_blob_put(new_blob);
			return -EINVAL;
		}
		if (expected_elem_size > 0 &&
		    new_blob->length % expected_elem_size != 0) {
			DPU_ERROR("[BLOB:%d] length %zu not divisible by element size %zu\n",
					new_blob->base.id, new_blob->length, expected_elem_size);
			drm_property_blob_put(new_blob);
			return -EINVAL;
		}
	}

	*replaced |= drm_property_replace_blob(blob, new_blob);
	drm_property_blob_put(new_blob);

	return 0;
}

static int dpu_wb_connector_atomic_set_property(
		struct drm_connector *connector,
		struct drm_connector_state *state,
		struct drm_property *property,
		uint64_t val)
{
	struct dpu_wb_connector_state *dpu_wb_conn_state;
	struct drm_writeback_connector *drm_wb_connector;
	struct dpu_wb_connector *dpu_wb_connector;
	struct dpu_general_rect wb_crop0;
	struct dpu_general_rect wb_dst_rect;
	struct dpu_prepq_writeback prepq_wb_pos;
	bool replaced = false;
	int ret;

	if (!connector || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", connector, state);
		return -EINVAL;
	}

	drm_wb_connector = drm_connector_to_writeback(connector);
	dpu_wb_connector = to_dpu_wb_connector(drm_wb_connector);
	dpu_wb_conn_state = to_dpu_wb_connector_state(state);

	if (property == dpu_wb_connector->pos_type_prop) {
		dpu_wb_conn_state->pos_type = val;
	} else if (property == dpu_wb_connector->rotation_prop) {
		dpu_wb_conn_state->rotation = val;
	} else if (property == dpu_wb_connector->src_rect_prop) {
		if (copy_from_user(&wb_crop0, (void __user *)(uintptr_t)val,
				sizeof(struct dpu_general_rect))) {
			DPU_ERROR("failed to copy wb src rect\n");
			return -EINVAL;
		}

		dpu_wb_conn_state->src_rect.x = wb_crop0.x;
		dpu_wb_conn_state->src_rect.y = wb_crop0.y;
		dpu_wb_conn_state->src_rect.w = wb_crop0.w;
		dpu_wb_conn_state->src_rect.h = wb_crop0.h;
	} else if (property == dpu_wb_connector->dst_rect_prop) {
		if (copy_from_user(&wb_dst_rect, (void __user *)(uintptr_t)val,
				sizeof(struct dpu_general_rect))) {
			DPU_ERROR("failed to copy wb dst rect\n");
			return -EINVAL;
		}

		dpu_wb_conn_state->dst_rect.x = wb_dst_rect.x;
		dpu_wb_conn_state->dst_rect.y = wb_dst_rect.y;
		dpu_wb_conn_state->dst_rect.w = wb_dst_rect.w;
		dpu_wb_conn_state->dst_rect.h = wb_dst_rect.h;
	} else if (property == dpu_wb_connector->scaler_enable_prop) {
		dpu_wb_conn_state->scaler_enable = val;
	} else if (property == dpu_wb_connector->scaler_blob_prop) {
		ret = drm_atomic_replace_property_blob_from_id(connector->dev,
				&dpu_wb_conn_state->scaler_blob_ptr,
				val,
				sizeof(struct dpu_1d_scaler_cfg), -1,
				&replaced);
		return ret;
	} else if (property == dpu_wb_connector->dither_enable_prop) {
		dpu_wb_conn_state->dither_enable = val;
	} else if (property == dpu_wb_connector->dither_blob_prop) {
		ret = drm_atomic_replace_property_blob_from_id(connector->dev,
				&dpu_wb_conn_state->dither_blob_ptr,
				val,
				sizeof(struct dpu_dither_cfg), -1,
				&replaced);
		return ret;
	} else if (property == dpu_wb_connector->csc_blob_prop) {
		ret = drm_atomic_replace_property_blob_from_id(connector->dev,
				&dpu_wb_conn_state->csc_blob_ptr,
				val,
				sizeof(struct dpu_csc_matrix_cfg), -1,
				&replaced);
		return ret;
	} else if (property == dpu_wb_connector->wfd_source_slice_prop) {
		dpu_wb_conn_state->wfd_source_slice_num = val;
	} else if (property == dpu_wb_connector->protected_buffer_enable_prop) {
		dpu_wb_conn_state->protected_buffer_enable = val;
	} else if (property == dpu_wb_connector->prepq_wb_prop) {
		if (copy_from_user(&prepq_wb_pos, (void __user *)(uintptr_t)val,
				sizeof(struct dpu_prepq_writeback))) {
			DPU_ERROR("failed to copy prepq writeback position\n");
			return -EINVAL;
		}

		dpu_wb_conn_state->prepq_wb.prepq_id = prepq_wb_pos.prepq_id;
		dpu_wb_conn_state->prepq_wb.prepq_dst_width = prepq_wb_pos.prepq_dst_width;
		dpu_wb_conn_state->prepq_wb.prepq_dst_height = prepq_wb_pos.prepq_dst_height;
	} else {
		DPU_ERROR("unknown property %s\n", property->name);
		return -EINVAL;
	}

	DPU_PROPERTY_DEBUG("set property %s: %lld\n", property->name, val);

	return 0;
}

static int dpu_wb_connector_atomic_get_property(
		struct drm_connector *connector,
		const struct drm_connector_state *state,
		struct drm_property *property,
		uint64_t *val)
{
	struct dpu_wb_connector_state *dpu_wb_conn_state;
	struct dpu_wb_connector *dpu_wb_connector;
	struct drm_writeback_connector *drm_wb_connector;

	if (!connector || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", connector, state);
		return -EINVAL;
	}

	drm_wb_connector = drm_connector_to_writeback(connector);
	dpu_wb_connector = to_dpu_wb_connector(drm_wb_connector);
	dpu_wb_conn_state = to_dpu_wb_connector_state(state);

	if (property == dpu_wb_connector->pos_type_prop)
		*val = dpu_wb_conn_state->pos_type;
	else if (property == dpu_wb_connector->rotation_prop)
		*val = dpu_wb_conn_state->rotation;
	else if (property == dpu_wb_connector->src_rect_prop)
		*val = 0;
	else if (property == dpu_wb_connector->dst_rect_prop)
		*val = 0;
	else if (property == dpu_wb_connector->scaler_enable_prop)
		*val = dpu_wb_conn_state->scaler_enable;
	else if (property == dpu_wb_connector->scaler_blob_prop)
		*val = dpu_wb_conn_state->scaler_blob_ptr ?
				dpu_wb_conn_state->scaler_blob_ptr->base.id : 0;
	else if (property == dpu_wb_connector->wfd_source_slice_prop)
		*val = dpu_wb_conn_state->wfd_source_slice_num;
	else if (property == dpu_wb_connector->csc_blob_prop)
		*val = dpu_wb_conn_state->csc_blob_ptr ?
				dpu_wb_conn_state->csc_blob_ptr->base.id : 0;
	else if (property == dpu_wb_connector->dither_enable_prop)
		*val = dpu_wb_conn_state->dither_enable;
	else if (property == dpu_wb_connector->dither_blob_prop)
		*val = dpu_wb_conn_state->dither_blob_ptr ?
				dpu_wb_conn_state->dither_blob_ptr->base.id : 0;
	else if (property == dpu_wb_connector->protected_buffer_enable_prop)
		*val = dpu_wb_conn_state->protected_buffer_enable;
	else if (property == dpu_wb_connector->prepq_wb_prop)
		*val = 0;
	else {
		DPU_ERROR("unknown property %s\n", property->name);
		return -EINVAL;
	}

	DPU_PROPERTY_DEBUG("get property %s: %lld\n", property->name, *val);
	return 0;
}

static struct drm_connector_state *dpu_wb_connector_atomic_duplicate_state(
		struct drm_connector *connector)
{
	struct dpu_wb_connector_state *new_state, *old_state;

	if (!connector || !connector->state) {
		DPU_ERROR("invalid parameters, %pK\n", connector);
		return NULL;
	}

	old_state = to_dpu_wb_connector_state(connector->state);

	new_state = kzalloc(sizeof(*new_state), GFP_KERNEL);
	if (!new_state)
		return NULL;

	__drm_atomic_helper_connector_duplicate_state(connector,
			&new_state->base);

	new_state->pos_type = old_state->pos_type;
	new_state->rotation = old_state->rotation;
	new_state->scaler_enable = old_state->scaler_enable;
	new_state->scaler_blob_ptr = old_state->scaler_blob_ptr;
	new_state->dither_enable = old_state->dither_enable;
	new_state->dither_blob_ptr = old_state->dither_blob_ptr;
	new_state->csc_blob_ptr = old_state->csc_blob_ptr;
	new_state->src_rect = old_state->src_rect;
	new_state->wfd_source_slice_num = old_state->wfd_source_slice_num;
	new_state->prepq_wb = old_state->prepq_wb;

	if (new_state->scaler_blob_ptr)
		drm_property_blob_get(new_state->scaler_blob_ptr);
	if (new_state->csc_blob_ptr)
		drm_property_blob_get(new_state->csc_blob_ptr);
	if (new_state->dither_blob_ptr)
		drm_property_blob_get(new_state->dither_blob_ptr);

	return &new_state->base;
}

static void dpu_wb_connector_atomic_destroy_state(
		struct drm_connector *connector,
		struct drm_connector_state *state)
{
	struct dpu_wb_connector_state *wb_state;

	DPU_DEBUG("wb destroy_state\n");

	if (!connector || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				connector, state);
		return;
	}

	wb_state = to_dpu_wb_connector_state(state);

	if (wb_state->scaler_blob_ptr)
		drm_property_blob_put(wb_state->scaler_blob_ptr);
	if (wb_state->csc_blob_ptr)
		drm_property_blob_put(wb_state->csc_blob_ptr);
	if (wb_state->dither_blob_ptr)
		drm_property_blob_put(wb_state->dither_blob_ptr);

	__drm_atomic_helper_connector_destroy_state(state);

	kfree(wb_state);
	wb_state = NULL;
}

static void dpu_wb_connector_reset(struct drm_connector *connector)
{
	struct dpu_wb_connector *dpu_wb_connector;
	struct drm_writeback_connector *drm_wb_connector;
	struct dpu_wb_connector_state *new_state;

	DPU_DEBUG("wb connector reset\n");

	if (!connector) {
		DPU_ERROR("invalid parameters, %pK\n", connector);
		return;
	}

	drm_wb_connector = drm_connector_to_writeback(connector);
	dpu_wb_connector = to_dpu_wb_connector(drm_wb_connector);

	if (connector->state) {
		dpu_wb_connector_atomic_destroy_state(connector,
				connector->state);
		connector->state = NULL;
	}

	new_state = kzalloc(sizeof(*new_state), GFP_KERNEL);
	if (!new_state)
		return;

	__drm_atomic_helper_connector_reset(connector, &new_state->base);

	new_state->pos_type = WB_POS_TYPE_INVALID;
	new_state->rotation = DRM_MODE_ROTATE_0;
	new_state->src_rect.x = 0;
	new_state->src_rect.y = 0;
	new_state->src_rect.w = 0;
	new_state->src_rect.h = 0;
	new_state->dst_rect.x = 0;
	new_state->dst_rect.y = 0;
	new_state->dst_rect.w = 0;
	new_state->dst_rect.h = 0;
	new_state->scaler_blob_ptr = NULL;
	new_state->scaler_enable = false;
	new_state->dither_blob_ptr = NULL;
	new_state->dither_enable = false;
	new_state->csc_blob_ptr = NULL;
	new_state->wfd_source_slice_num = 0;
	new_state->prepq_wb.prepq_id = 0;
	new_state->prepq_wb.prepq_dst_width = 0;
	new_state->prepq_wb.prepq_dst_height = 0;
}

static int dpu_online_wb_select_check(enum dpu_hw_wb_pos_type pos_type)
{
	int ret = 0;

	if (pos_type == WB_POS_TYPE_INVALID) {
		DPU_ERROR("invalid wb position type: %u\n", pos_type);
		ret = -EINVAL;
	}

	return ret;
}

static int dpu_offline_wb_select_check(enum dpu_hw_wb_pos_type pos_type)
{
	if (pos_type != WB_POS_TYPE_AFTER_MIXER) {
		DPU_ERROR("not supported wb position type: %u\n", pos_type);
		return -EINVAL;
	}

	return 0;
}

static int dpu_wb_position_type_check(struct drm_connector_state *conn_state)
{
	struct dpu_wb_connector_state *dpu_wb_conn_state;
	bool is_offline;
	int ret = 0;

	dpu_wb_conn_state = to_dpu_wb_connector_state(conn_state);
	is_offline = !is_clone_mode(conn_state->state, conn_state);

	if (is_offline) {
		/**
		 * offline writeback
		 * the default wb position type for offline mode only supports the selection
		 * after mixer.
		 */
		ret = dpu_offline_wb_select_check(dpu_wb_conn_state->pos_type);
		goto out;
	} else {
		/* online writeback */
		ret = dpu_online_wb_select_check(dpu_wb_conn_state->pos_type);
		goto out;
	}

out:
	return ret;
}

static int dpu_wb_afbce_superblock_check(struct drm_framebuffer *fb,
		struct drm_connector_state *conn_state)
{
	struct dpu_wb_connector_state *dpu_wb_conn_state;
	const struct dpu_format_map *format_map;
	bool is_afbc_32x8, is_afbc_split;
	u8 dpu_format;
	enum rot_mode rot_mode;
	enum flip_mode flip_mode;

	dpu_wb_conn_state = to_dpu_wb_connector_state(conn_state);

	format_map = dpu_format_get(FMT_MODULE_WB, fb->format->format,
			fb->modifier);
	dpu_format = format_map->dpu_format;

	if (!DPU_FMT_IS_AFBC(fb->modifier))
		return 0;

	is_afbc_32x8 = DPU_FMT_IS_AFBC_32x8(fb->modifier);
	is_afbc_split = DPU_FMT_HAS_AFBC_SPLIT(fb->modifier);

	if (dpu_format != WB_FORMAT_ABGR_2101010 &&
			dpu_format != WB_FORMAT_ABGR_8888 &&
			dpu_format != WB_FORMAT_BGR_888 &&
			dpu_format != WB_FORMAT_BGR_565 &&
			dpu_format != WB_FORMAT_NV12_8 &&
			dpu_format != WB_FORMAT_NV12_10) {
		DPU_ERROR("current format(%u) does not support afbce\n", dpu_format);
		return -EINVAL;
	}

	if (is_afbc_32x8 && is_afbc_split &&
			(dpu_format == WB_FORMAT_BGR_565 ||
			dpu_format == WB_FORMAT_NV12_8 ||
			dpu_format == WB_FORMAT_NV12_10)) {
		DPU_ERROR("current format(%u) does not support afbce tile size 32x4\n", dpu_format);
		return -EINVAL;
	}

	rot_mode = dpu_wb_get_rotation_info(dpu_wb_conn_state->rotation);
	flip_mode = dpu_wb_get_flip_info(dpu_wb_conn_state->rotation);

	if ((is_afbc_32x8 || is_afbc_split) && (rot_mode != ROT_0 || flip_mode != NO_FLIP)) {
		DPU_ERROR("only 16x16 tile afbce supports rotation of flip: %d, %d, %d, %d\n",
				is_afbc_32x8, is_afbc_split, rot_mode, flip_mode);
		return -EINVAL;
	}

	return 0;
}

static int dpu_wb_output_width_check(struct drm_framebuffer *fb,
		struct dpu_wb_connector_state *wb_state)
{
	struct dpu_1d_scaler_cfg *wb_scaler;
	const struct dpu_format_map *format_map;
	enum rot_mode rot_mode;
	bool need_check = false;
	int dst_width;

	format_map = dpu_format_get(FMT_MODULE_WB, fb->format->format,
			fb->modifier);

	rot_mode = dpu_wb_get_rotation_info(wb_state->rotation);

	dst_width = wb_state->src_rect.w;
	if (wb_state->scaler_enable && wb_state->scaler_blob_ptr) {
		wb_scaler = (struct dpu_1d_scaler_cfg *)wb_state->scaler_blob_ptr->data;
		dst_width = wb_scaler->output_width;
	}

	if (DPU_FMT_IS_AFBC(fb->modifier)) {
		/**
		 * When in afbce format, 32bpp format if tile size is 16X16,
		 * wb output width is limited to 2048
		 */
		if (!DPU_FMT_IS_AFBC_32x8(fb->modifier) &&
				!DPU_FMT_HAS_AFBC_SPLIT(fb->modifier) &&
				dpu_wb_scan2tile_buffer_format_bpp(format_map->dpu_format) == 32)
			need_check = true;
	} else {
		/**
		 * When in raw format, the 32bpp format is limited to 2048 output width
		 * if rotated 90 degrees or 270 degrees
		 */
		if ((rot_mode == ROT_90 || rot_mode == ROT_270) &&
				dpu_wb_scan2tile_buffer_format_bpp(format_map->dpu_format) == 32)
			need_check = true;
	}

	if (need_check && dst_width > MIN_WB_OUTPUT_WIDTH_LIMIT) {
		DPU_ERROR("wb output width: %d, exceeds the limit width: %d\n",
				dst_width, MIN_WB_OUTPUT_WIDTH_LIMIT);
		return -EINVAL;
	}

	return 0;
}

static int dpu_wb_output_address_check(struct drm_framebuffer *fb)
{
	const struct dpu_format_map *format_map;
	struct dpu_gem_object *gem_obj;
	u64 dma_addr;
	u64 addr;

	gem_obj = to_dpu_gem_obj(fb->obj[0]);
	dma_addr = gem_obj->iova_dom ? gem_obj->iova_dom->iova : gem_obj->paddr;

	format_map = dpu_format_get(FMT_MODULE_WB, fb->format->format,
			fb->modifier);

	addr = dma_addr + fb->offsets[WB_PLANAR_Y_RGB];
	if (!CHECK_VALUE_ALIGN(addr, 16)) {
		DPU_ERROR("address(%#llx) is not aligned with 16\n", addr);
		return -EINVAL;
	}
	if (format_map->plane_count == PLANE_TWO) {
		addr = dma_addr + fb->offsets[WB_PLANAR_UV];
		if (!CHECK_VALUE_ALIGN(addr, 16)) {
			DPU_ERROR("address(%#llx) is not aligned with 16\n", addr);
			return -EINVAL;
		}
	}

	return 0;
}

static int dpu_wb_offline_src_rect_check(struct dpu_wb_connector_state *wb_state,
		struct dpu_crtc_state *dpu_cstate)
{
	if ((wb_state->src_rect.w > dpu_cstate->lm_width) ||
			(wb_state->src_rect.h > dpu_cstate->lm_height)) {
		DPU_ERROR("offline wb src_rect size[%u,%u] exceeds the input size[%u,%u]\n",
				wb_state->src_rect.w, wb_state->src_rect.h,
				dpu_cstate->lm_width, dpu_cstate->lm_height);
		return -EINVAL;
	}

	return 0;
}

static int dpu_wb_scaler_check(struct drm_connector_state *conn_state)
{
	struct dpu_wb_connector_state *wb_state;
	struct dpu_1d_scaler_cfg *wb_scaler_cfg;

	wb_state = to_dpu_wb_connector_state(conn_state);

	if (!wb_state->scaler_blob_ptr)
		return 0;

	if (!wb_state->scaler_enable)
		return 0;

	if (!wb_state->scaler_blob_ptr->data) {
		DPU_ERROR("wb scaler blob data pointer is NULL\n");
		return -EINVAL;
	}

	wb_scaler_cfg = (struct dpu_1d_scaler_cfg *)(wb_state->scaler_blob_ptr->data);

	if (wb_scaler_cfg->output_width == 0 ||
			wb_scaler_cfg->input_width == 0 ||
			wb_scaler_cfg->output_height == 0 ||
			wb_scaler_cfg->input_height == 0) {
		DPU_ERROR("Invalid size: [%u,%u] -> [%u, %u]\n",
				wb_scaler_cfg->input_width, wb_scaler_cfg->input_height,
				wb_scaler_cfg->output_width, wb_scaler_cfg->output_height);
		return -EINVAL;
	}

	if (wb_scaler_cfg->output_width >
			(wb_scaler_cfg->input_width << LOG_OF_MAX_SCALE_RATIO)) {
		DPU_ERROR("Horizontal exceeds scl_up limit: %u -> %u\n",
				wb_scaler_cfg->input_width, wb_scaler_cfg->output_width);
		return -EINVAL;
	} else if (wb_scaler_cfg->output_width <
			(wb_scaler_cfg->input_width >> LOG_OF_MIN_SCALE_RATIO)) {
		DPU_ERROR("Horizontal exceeds scl_down limit: %u -> %u\n",
				wb_scaler_cfg->input_width, wb_scaler_cfg->output_width);
		return -EINVAL;
	}

	if (wb_scaler_cfg->output_height >
			(wb_scaler_cfg->input_height << LOG_OF_MAX_SCALE_RATIO)) {
		DPU_ERROR("Vertical exceeds scl_up limit: %u -> %u\n",
				wb_scaler_cfg->input_height, wb_scaler_cfg->output_height);
		return -EINVAL;
	} else if (wb_scaler_cfg->output_height <
			(wb_scaler_cfg->input_height >> LOG_OF_MIN_SCALE_RATIO)) {
		DPU_ERROR("Vertical exceeds scl_down limit: %u -> %u\n",
				wb_scaler_cfg->input_height, wb_scaler_cfg->output_height);
		return -EINVAL;
	}

	return 0;
}

static int dpu_wb_encoder_atomic_check(struct drm_encoder *encoder,
		struct drm_crtc_state *crtc_state,
		struct drm_connector_state *conn_state)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector_state *wb_state;
	struct dpu_wb_connector *dpu_wb_conn;
	const struct dpu_format_map *format_map;
	struct dpu_crtc_state *dpu_cstate;
	struct drm_connector *connector;
	struct drm_framebuffer *fb;
	bool is_online;
	int ret;

	if (!encoder || !crtc_state || !conn_state) {
		DPU_ERROR("invalid parameters, %pK, %pK, %pK\n",
				encoder, crtc_state, conn_state);
		return -EINVAL;
	}

	connector = conn_state->connector;
	drm_wb_conn = drm_connector_to_writeback(connector);
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);
	wb_state = to_dpu_wb_connector_state(conn_state);

	/* only power on */
	if (!conn_state->writeback_job) {
		DPU_DEBUG("no writeback job");
		return 0;
	}

	if (!conn_state->crtc) {
		DPU_ERROR("invalid parameters, crtc: %pK\n",
				conn_state->crtc);
		return -EINVAL;
	}

	if (!conn_state->crtc->state) {
		DPU_ERROR("invalid parameters, crtc_state: %pK\n",
				conn_state->crtc->state);
		return -EINVAL;
	}

	dpu_cstate = to_dpu_crtc_state(crtc_state);

	fb = conn_state->writeback_job->fb;
	if (!fb || !fb->format) {
		DPU_ERROR("writeback_job fb or fb->format is NULL");
		return -EINVAL;
	}

	format_map = dpu_format_get(FMT_MODULE_WB, fb->format->format,
			fb->modifier);
	if (!format_map) {
		DPU_ERROR("writeback invalid format: %u\n", fb->format->format);
		return -EINVAL;
	}

	is_online = is_clone_mode(conn_state->state, conn_state);
	if (!is_online) {
		ret = dpu_wb_offline_src_rect_check(wb_state, dpu_cstate);
		if (ret) {
			DPU_ERROR("failed to do wb src rect check, ret %d\n", ret);
			return ret;
		}
	}

	ret = dpu_wb_output_address_check(fb);
	if (ret) {
		DPU_ERROR("failed to do wb output address check, ret %d\n", ret);
		return ret;
	}

	ret = dpu_wb_afbce_superblock_check(fb, conn_state);
	if (ret) {
		DPU_ERROR("failed to do wb afbce superblock check, ret %d\n", ret);
		return ret;
	}

	ret = dpu_wb_output_width_check(fb, wb_state);
	if (ret) {
		DPU_ERROR("failed to do wb output width check, ret %d\n", ret);
		return ret;
	}

	ret = dpu_wb_position_type_check(conn_state);
	if (ret) {
		DPU_ERROR("failed to do wb input position check, ret %d\n", ret);
		return ret;
	}

	ret = dpu_wb_scaler_check(conn_state);
	if (ret) {
		DPU_ERROR("failed to do wb scaler check, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct drm_encoder_helper_funcs dpu_wb_encoder_helper_funcs = {
	.atomic_check = dpu_wb_encoder_atomic_check,
};

static const struct drm_connector_helper_funcs dpu_wb_conn_helper_funcs = {
	.get_modes = dpu_wb_connector_get_modes,
	.mode_valid = dpu_wb_connector_mode_valid,
	.atomic_check = dpu_wb_connector_atomic_check,
	.atomic_commit = dpu_wb_connector_atomic_commit,
};

static const struct drm_connector_funcs dpu_wb_connector_funcs = {
	.detect = dpu_wb_connector_detect,
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = dpu_wb_connector_destroy,
	.atomic_set_property = dpu_wb_connector_atomic_set_property,
	.atomic_get_property = dpu_wb_connector_atomic_get_property,
	.atomic_duplicate_state = dpu_wb_connector_atomic_duplicate_state,
	.atomic_destroy_state = dpu_wb_connector_atomic_destroy_state,
	.reset = dpu_wb_connector_reset,
};

bool dpu_wb_need_cfg_rdy_update(struct drm_connector *connector,
		bool is_offline)
{
	struct dpu_wb_connector_state *wb_state;

	if (!connector) {
		DPU_ERROR("invalid parameter\n");
		return false;
	}

	wb_state = to_dpu_wb_connector_state(connector->state);

	DPU_DEBUG("is_offline %d, fb state %d\n", is_offline, !!wb_state->fb);

	return is_offline ? !!wb_state->fb : true;
}

void dpu_wb_connector_job_clear(struct drm_crtc *crtc,
		struct drm_writeback_connector *drm_wb_conn)
{
	struct dpu_wb_connector *dpu_wb_conn;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;
	unsigned long flags;
	int ret;

	if (!crtc || !drm_wb_conn) {
		DPU_ERROR("invalid parameters %pK, %pK\n", crtc, drm_wb_conn);
		return;
	}

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	pipeline = &dpu_kms->virt_pipeline[drm_crtc_index(crtc)];

	spin_lock_irqsave(&drm_wb_conn->job_lock, flags);
	ret = list_empty(&drm_wb_conn->job_queue);
	spin_unlock_irqrestore(&drm_wb_conn->job_lock, flags);
	if (!ret) {
		dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);
		drm_writeback_signal_completion(drm_wb_conn, 0);

		if (pipeline->is_offline)
			dpu_kms_fence_force_signal(crtc, false);

		dpu_core_perf_clk_active();
		dpu_wb_enable(dpu_wb_conn, false, 0);
		dpu_core_perf_clk_deactive();
	}
}

static void dpu_wb_irq_handler(int intr_id, void *data)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector *dpu_wb_conn;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;
	struct dpu_intr_pipe *intr_pipe;
	enum dpu_intr_type intr_type;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_kms *dpu_kms;
	struct drm_crtc *crtc;
	bool is_online_wb;

	DPU_ISR_DEBUG("Enter, intr_id %d\n", intr_id);

	intr_ctx = data;
	intr_type = dpu_interrupt_id_to_type(intr_id);
	if (intr_type >= DPU_INTR_TYPE_MAX)
		return;

	intr_pipe = &intr_ctx->intr_pipe[intr_type];
	crtc = intr_pipe->connector->state->crtc;
	if (!crtc) {
		DPU_ISR_DEBUG("crtc is null\n");
		return;
	}

	is_online_wb = (intr_pipe->clone_connector) ? true : false;
	if (is_online_wb)
		drm_wb_conn = drm_connector_to_writeback(
				intr_pipe->clone_connector);
	else
		drm_wb_conn = drm_connector_to_writeback(
				intr_pipe->connector);

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	pipeline = &dpu_kms->virt_pipeline[drm_crtc_index(crtc)];
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);
	if (intr_id == dpu_wb_conn->intr_state.wb_done_id) {
		if (!is_online_wb) {
			trace_dpu_wb_irq_handler("wb done", intr_type);
			atomic_set(&pipeline->cfg_done_flag, 1);
			dpu_wfd_notify_event(XPLAYER_EVENT_FRAME_DONE);

			wake_up(&pipeline->cfg_done_wq);
		}
	}

	if (intr_id == dpu_wb_conn->intr_state.overflow_id) {
		DPU_ISR_DEBUG("wb overflow\n");
		dpu_wb_connector_job_clear(crtc, drm_wb_conn);
	}

	DPU_ISR_DEBUG("Exit, intr_id %d\n", intr_id);
}

static int dpu_wb_interrupt_id_get(struct drm_connector *connector, u32 type,
		enum dpu_intr_id *wb_intr_id_out,
		enum dpu_intr_id *wb_overflow_intr_id_out,
		bool *is_online_out)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector *dpu_wb_conn;
	enum dpu_intr_id wb_overflow_intr_id;
	enum dpu_intr_id wb_id;
	bool is_online;
	bool is_wb0;

	drm_wb_conn = drm_connector_to_writeback(connector);
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);
	is_online = dpu_wb_conn->is_online;
	if (!dpu_wb_conn->hw_wb) {
		DPU_ERROR("is_online %d, hw_wb is null\n", is_online);
		goto error_get;
	}

	is_wb0 = (dpu_wb_conn->hw_wb->hw.blk_id == WB0) ? true : false;

	switch (type) {
	case DPU_INTR_ONLINE0:
		wb_id = (is_wb0) ? INTR_ONLINE0_WB0_DONE : INTR_ONLINE0_WB1_DONE;
		wb_overflow_intr_id = (is_wb0) ? INTR_ONLINE0_WB0_OVERFLOW :
				INTR_ONLINE0_WB1_OVERFLOW;
		break;
	case DPU_INTR_ONLINE1:
		wb_id = (is_wb0) ? INTR_COMBO_WB0_DONE : INTR_COMBO_WB1_DONE;
		wb_overflow_intr_id = (is_wb0) ? INTR_COMBO_WB0_OVERFLOW :
				INTR_COMBO_WB1_OVERFLOW;
		break;
	case DPU_INTR_OFFLINE0:
		wb_id = (is_wb0) ? INTR_OFFLINE0_WB0_DONE : INTR_OFFLINE0_WB1_DONE;
		wb_overflow_intr_id = INTR_OFFLINE0_MAX;
		break;
	default:
		DPU_ERROR("incorrect dpu interrupt type %d\n", type);
		goto error_get;
	}

	*is_online_out = is_online;
	*wb_intr_id_out = wb_id;
	*wb_overflow_intr_id_out = wb_overflow_intr_id;

	return 0;

error_get:
	*wb_intr_id_out = INTR_OFFLINE0_MAX;
	*wb_overflow_intr_id_out = INTR_OFFLINE0_MAX;
	*is_online_out = false;
	return -EINVAL;
}

int dpu_wb_interrupt_register(struct drm_connector *connector, u32 type)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector *dpu_wb_conn;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;
	enum dpu_intr_id wb_overflow_intr_id;
	enum dpu_intr_id wb_done_intr_id;
	bool is_online;
	int ret;

	ret = dpu_wb_interrupt_id_get(connector, type, &wb_done_intr_id,
			&wb_overflow_intr_id, &is_online);
	if (ret) {
		DPU_ERROR("failed to get wb interrupt id\n");
		return -EINVAL;
	}

	dpu_drm_dev = to_dpu_drm_dev(connector->dev);
	intr_ctx = dpu_drm_dev->dpu_kms->virt_ctrl.intr_ctx;

	drm_wb_conn = drm_connector_to_writeback(connector);
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);

	ret = dpu_interrupt_register(intr_ctx, wb_done_intr_id, &dpu_wb_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register wb-done-%d interrupt, ret %d\n",
				wb_done_intr_id, ret);
		goto err_wb_done;
	}
	dpu_wb_conn->intr_state.wb_done_id = wb_done_intr_id;

	if (is_online) {
		ret = dpu_interrupt_register(intr_ctx, wb_overflow_intr_id,
			&dpu_wb_irq_handler);
		if (ret) {
			DPU_ERROR("failed to register wb-overflow-%d, ret %d\n",
					wb_overflow_intr_id, ret);
			goto err_cfg_ready;
		}
		dpu_wb_conn->intr_state.overflow_id = wb_overflow_intr_id;
	}

	return 0;

err_cfg_ready:
	dpu_interrupt_unregister(intr_ctx, wb_done_intr_id);
err_wb_done:
	return ret;
}

void dpu_wb_interrupt_unregister(struct drm_connector *connector, u32 type)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector *dpu_wb_conn;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;

	dpu_drm_dev = to_dpu_drm_dev(connector->dev);
	intr_ctx = dpu_drm_dev->dpu_kms->virt_ctrl.intr_ctx;

	drm_wb_conn = drm_connector_to_writeback(connector);
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);

	if (dpu_wb_conn->is_online)
		dpu_interrupt_unregister(intr_ctx,
				dpu_wb_conn->intr_state.overflow_id);

	dpu_interrupt_unregister(intr_ctx, dpu_wb_conn->intr_state.wb_done_id);
}

int dpu_wb_resource_reserve(struct drm_connector *connector, u32 pipe_id,
		unsigned long blk_id, bool is_clone, bool test_only)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector *dpu_wb_conn;
	struct dpu_res_mgr_ctx *res_mgr_ctx;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_hw_blk *wb_scale_hw_blk;
	struct dpu_hw_blk *wb_hw_blk;
	int ret;

	if (!connector) {
		DPU_ERROR("invalid parameter, %pK\n", connector);
		return -EINVAL;
	}

	drm_wb_conn = drm_connector_to_writeback(connector);
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);
	dpu_drm_dev = to_dpu_drm_dev(dpu_wb_conn->drm_dev);
	res_mgr_ctx = dpu_drm_dev->dpu_kms->res_mgr_ctx;

	if (dpu_wb_conn->hw_wb)
		return 0;

	wb_hw_blk = dpu_res_mgr_block_reserve_id(res_mgr_ctx, pipe_id,
			BLK_WB, blk_id, test_only);
	if (IS_ERR_OR_NULL(wb_hw_blk)) {
		DPU_ERROR("failed to reserve wb %lx\n", blk_id);
		return PTR_ERR(wb_hw_blk);
	}

	wb_scale_hw_blk = dpu_res_mgr_block_reserve_id(res_mgr_ctx, pipe_id,
			BLK_WB_SCALER, blk_id, test_only);
	if (IS_ERR_OR_NULL(wb_scale_hw_blk)) {
		DPU_ERROR("failed to reserve wb %lx\n", blk_id);
		ret = PTR_ERR(wb_scale_hw_blk);
		goto error_scale;
	}

	if (!test_only) {
		dpu_wb_conn->hw_wb = to_dpu_hw_wb(wb_hw_blk);
		RES_DEBUG("pipe[%u] reserved wb-%lu\n", pipe_id, blk_id);

		dpu_wb_conn->hw_wb_scaler = to_dpu_hw_wb_scaler(wb_scale_hw_blk);
		RES_DEBUG("pipe[%u] reserved hw_wb_scaler blk_id:%lu\n",
				pipe_id, blk_id);

		/* When in clone mode, it is online writeback */
		dpu_wb_conn->is_online = is_clone;
	}

	return 0;

error_scale:
	if (!test_only)
		dpu_res_mgr_block_release(res_mgr_ctx, wb_hw_blk);

	return ret;
}

void dpu_wb_resource_release(struct drm_connector *connector)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector *dpu_wb_conn;
	struct dpu_res_mgr_ctx *res_mgr_ctx;
	struct dpu_drm_device *dpu_drm_dev;

	if (!connector) {
		DPU_ERROR("invalid parameter, %pK\n", connector);
		return;
	}

	drm_wb_conn = drm_connector_to_writeback(connector);
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);
	dpu_drm_dev = to_dpu_drm_dev(dpu_wb_conn->drm_dev);
	res_mgr_ctx = dpu_drm_dev->dpu_kms->res_mgr_ctx;

	if (dpu_wb_conn->hw_wb) {
		dpu_res_mgr_block_release(res_mgr_ctx,
				&dpu_wb_conn->hw_wb->hw);
		dpu_wb_conn->hw_wb = NULL;
	}

	if (dpu_wb_conn->hw_wb_scaler) {
		dpu_res_mgr_block_release(res_mgr_ctx,
				&dpu_wb_conn->hw_wb_scaler->hw);
		dpu_wb_conn->hw_wb_scaler = NULL;
	}

	dpu_wb_conn->is_online = false;
}

static int dpu_wb_create_modifiers(struct drm_writeback_connector *wb_conn,
		u32 *wb_formats, u32 format_count)
{
	struct dpu_wb_connector *dpu_wb_conn;
	struct drm_format_modifier_blob *blob_data;
	struct drm_mode_config *config;
	struct drm_property_blob *blob;
	struct drm_format_modifier *mod;
	struct drm_device *drm_dev;
	size_t modifiers_size;
	size_t formats_size;
	size_t blob_size;
	u32 modifier_count;
	u32 i, j;

	if (!wb_conn || !wb_formats || !format_count) {
		DPU_ERROR("invalid parameters, %pK, %pK, %u\n",
				wb_conn, wb_formats, format_count);
		return -EINVAL;
	}

	dpu_wb_conn = to_dpu_wb_connector(wb_conn);
	drm_dev = dpu_wb_conn->drm_dev;
	config = &drm_dev->mode_config;
	if (config->fb_modifiers_not_supported)
		return 0;

	modifier_count = ARRAY_SIZE(wb_format_modifiers);
	formats_size = sizeof(u32) * format_count;
	modifiers_size = sizeof(struct drm_format_modifier) * modifier_count;
	blob_size = sizeof(struct drm_format_modifier_blob);

	BUILD_BUG_ON(sizeof(struct drm_format_modifier_blob) % 8);

	blob_size += ALIGN(formats_size, 8);
	blob_size += modifiers_size;

	blob = drm_property_create_blob(drm_dev, blob_size, NULL);
	if (IS_ERR(blob))
		return PTR_ERR(blob);

	blob_data = blob->data;
	blob_data->version = FORMAT_BLOB_CURRENT;
	blob_data->count_formats = format_count;
	blob_data->formats_offset = sizeof(struct drm_format_modifier_blob);
	blob_data->count_modifiers = modifier_count;
	blob_data->modifiers_offset =
			ALIGN(blob_data->formats_offset + formats_size, 8);

	memcpy((u32 *)(((char *)blob_data) + blob_data->formats_offset),
			wb_formats, formats_size);

	mod = (struct drm_format_modifier *)(((char *)blob_data)
		+ blob_data->modifiers_offset);

	for (i = 0; i < modifier_count; i++) {
		for (j = 0; j < format_count; j++)
			if (dpu_format_is_afbc_supported(FMT_MODULE_WB,
					wb_formats[j],
					wb_format_modifiers[i]))
				mod->formats |= 1ULL << j;

		mod->modifier = wb_format_modifiers[i];
		mod->offset = 0;
		mod->pad = 0;
		mod++;
	}

	drm_object_attach_property(&wb_conn->base.base,
			dpu_wb_conn->modifier_prop, blob->base.id);

	return 0;
}

/* create wb rotation bitmask property */
static int dpu_wb_create_rotation_property(
		struct drm_writeback_connector *wb_conn,
		unsigned int rotation,
		unsigned int supported_rotations)
{
	struct dpu_wb_connector *dpu_wb_conn;
	struct drm_property *prop;

	static const struct drm_prop_enum_list props[] = {
		{ __builtin_ffs(DRM_MODE_ROTATE_0) - 1,   "rotate-0" },
		{ __builtin_ffs(DRM_MODE_ROTATE_90) - 1,  "rotate-90" },
		{ __builtin_ffs(DRM_MODE_ROTATE_180) - 1, "rotate-180" },
		{ __builtin_ffs(DRM_MODE_ROTATE_270) - 1, "rotate-270" },
		{ __builtin_ffs(DRM_MODE_REFLECT_X) - 1,  "reflect-x" },
		{ __builtin_ffs(DRM_MODE_REFLECT_Y) - 1,  "reflect-y" },
	};

	dpu_wb_conn = to_dpu_wb_connector(wb_conn);

	WARN_ON((supported_rotations & DRM_MODE_ROTATE_MASK) == 0);
	WARN_ON(!is_power_of_2(rotation & DRM_MODE_ROTATE_MASK));
	WARN_ON(rotation & ~supported_rotations);

	prop = drm_property_create_bitmask(wb_conn->base.dev, 0, "rotation",
			props, ARRAY_SIZE(props), supported_rotations);
	if (!prop) {
		DPU_ERROR("drm_property_create_bitmask fail: rotation\n");
		return -ENOMEM;
	}

	drm_object_attach_property(&wb_conn->base.base, prop, rotation);
	dpu_wb_conn->rotation_prop = prop;

	return 0;
}

static int dpu_wb_create_rect_properties(
		struct drm_writeback_connector *wb_conn)
{
	struct dpu_wb_connector *dpu_wb_conn;
	struct drm_property *prop;

	dpu_wb_conn = to_dpu_wb_connector(wb_conn);

	prop = drm_property_create_range(wb_conn->base.dev, 0, "src_rect", 0, U64_MAX);
	drm_object_attach_property(&wb_conn->base.base, prop, 0);
	dpu_wb_conn->src_rect_prop = prop;

	prop = drm_property_create_range(wb_conn->base.dev, 0, "dst_rect", 0, U64_MAX);
	drm_object_attach_property(&wb_conn->base.base, prop, 0);
	dpu_wb_conn->dst_rect_prop = prop;

	return 0;
}

static int dpu_wb_create_scaler_properties(
		struct drm_writeback_connector *wb_conn,
		struct dpu_wb_connector *dpu_wb_conn)
{
	struct drm_property *property;

	property = drm_property_create_range(wb_conn->base.dev, 0,
			"wb_scaler_enable", 0, 1);
	if (!property) {
		DPU_ERROR("drm_property_create fail: wb_scaler_enable\n");
		return -ENOMEM;
	}

	drm_object_attach_property(&wb_conn->base.base, property, 0);
	dpu_wb_conn->scaler_enable_prop = property;

	property = drm_property_create(wb_conn->base.dev, DRM_MODE_PROP_BLOB,
			"wb_scaler_prop", 0);
	if (!property) {
		DPU_ERROR("drm_property_create fail: wb_scaler_enable\n");
		return -ENOMEM;
	}
	drm_object_attach_property(&wb_conn->base.base, property, 0);
	dpu_wb_conn->scaler_blob_prop = property;
	dpu_wb_conn->scaler_blob_size = sizeof(struct dpu_1d_scaler_cfg);

	return 0;
}

static int dpu_wb_create_position_property(struct drm_writeback_connector *wb_conn,
		struct dpu_wb_connector *dpu_wb_conn)
{
	struct drm_property *prop;

	const struct drm_prop_enum_list input_pos_list[] = {
		{ WB_POS_TYPE_AFTER_MIXER,  "WB_POS_TYPE_AFTER_MIXER" },
		{ WB_POS_TYPE_AFTER_POSTPQ, "WB_POS_TYPE_AFTER_POSTPQ" },
		{ WB_POS_TYPE_AFTER_RC,     "WB_POS_TYPE_AFTER_RC" },
		{ WB_POS_TYPE_BEFORE_PREPQ, "WB_POS_TYPE_BEFORE_PREPQ" },
		{ WB_POS_TYPE_AFTER_PREPQ,  "WB_POS_TYPE_AFTER_PREPQ" },
	};

	prop = drm_property_create_enum(wb_conn->base.dev, 0,
			"pos_type", input_pos_list, WB_POS_TYPE_MAX);
	if (!prop)
		return -ENOMEM;

	drm_object_attach_property(&wb_conn->base.base, prop, 0);
	dpu_wb_conn->pos_type_prop = prop;

	return 0;
}

static int dpu_wb_create_csc_property(
		struct drm_writeback_connector *conn,
		struct dpu_wb_connector *dpu_wb)
{
	struct drm_property *property;

	property = drm_property_create(conn->base.dev, DRM_MODE_PROP_BLOB,
			"wb_csc_prop", 0);
	if (!property) {
		DPU_ERROR("drm_property_create fail: wb_csc\n");
		return -ENOMEM;
	}
	drm_object_attach_property(&conn->base.base, property, 0);
	dpu_wb->csc_blob_prop = property;
	dpu_wb->csc_blob_size = sizeof(struct dpu_csc_matrix_cfg);

	return 0;
}

static int dpu_wb_create_dither_properties(
		struct drm_writeback_connector *conn,
		struct dpu_wb_connector *dpu_wb)
{
	struct drm_property *property;

	property = drm_property_create_range(conn->base.dev, 0,
			"wb_dither_enable", 0, 1);
	if (!property) {
		DPU_ERROR("drm_property_create fail: wb_dither_enable\n");
		return -ENOMEM;
	}

	drm_object_attach_property(&conn->base.base, property, 0);
	dpu_wb->dither_enable_prop = property;

	property = drm_property_create(conn->base.dev, DRM_MODE_PROP_BLOB,
			"wb_dither_prop", 0);
	if (!property) {
		DPU_ERROR("drm_property_create fail: wb_dither\n");
		return -ENOMEM;
	}
	drm_object_attach_property(&conn->base.base, property, 0);
	dpu_wb->dither_blob_prop = property;
	dpu_wb->dither_blob_size = sizeof(struct dpu_dither_cfg);

	return 0;
}

static int dpu_wb_create_prepq_position_properties(
		struct drm_writeback_connector *wb_conn)
{
	struct dpu_wb_connector *dpu_wb_conn;
	struct drm_property *prop;

	dpu_wb_conn = to_dpu_wb_connector(wb_conn);

	prop = drm_property_create_range(wb_conn->base.dev, 0, "prepq_wb_position", 0, U64_MAX);
	drm_object_attach_property(&wb_conn->base.base, prop, 0);

	dpu_wb_conn->prepq_wb_prop = prop;
	return 0;
}

static int dpu_wb_create_properties(struct drm_writeback_connector *wb_conn)
{
	struct dpu_wb_connector *dpu_wb_conn;
	struct drm_property *prop;
	int ret;

	DPU_DEBUG("wb create_properties\n");

	if (!wb_conn) {
		DPU_ERROR("invalid parameters, %pK\n", wb_conn);
		return -EINVAL;
	}

	dpu_wb_conn = to_dpu_wb_connector(wb_conn);

	ret = dpu_wb_create_position_property(wb_conn, dpu_wb_conn);

	ret = dpu_wb_create_rotation_property(wb_conn,
			DRM_MODE_ROTATE_0,
			DRM_MODE_ROTATE_MASK | DRM_MODE_REFLECT_MASK);

	ret = dpu_wb_create_rect_properties(wb_conn);

	prop = drm_property_create(wb_conn->base.dev,
			DRM_MODE_PROP_IMMUTABLE | DRM_MODE_PROP_BLOB,
			"IN_FORMATS", 0);
	if (!prop)
		return -ENOMEM;

	dpu_wb_conn->modifier_prop = prop;

	dpu_wb_create_scaler_properties(wb_conn, dpu_wb_conn);
	dpu_wb_create_dither_properties(wb_conn, dpu_wb_conn);
	dpu_wb_create_csc_property(wb_conn, dpu_wb_conn);

	dpu_wfd_source_create_slice_property(wb_conn, dpu_wb_conn);

	dpu_wb_create_sec_drm_properties(wb_conn);

	dpu_wb_create_prepq_position_properties(wb_conn);

	DPU_DEBUG("wb create_properties end\n");
	return 0;
}

static int dpu_wb_bind(struct device *dev,
		struct device *master, void *data)
{
	struct dpu_wb_connector *wb_conn;
	struct dpu_drm_device *dpu_drm_dev;
	u32 *wb_formats;
	u32 format_count;
	int ret;

	DPU_DEBUG("writeback bind\n");

	wb_conn = platform_get_drvdata(to_platform_device(dev));
	wb_conn->drm_dev = dev_get_drvdata(master);
	dpu_drm_dev = to_dpu_drm_dev(wb_conn->drm_dev);

	format_count = dpu_format_all_supported_get(FMT_MODULE_WB,
			&wb_formats);
	if (format_count <= 0) {
		ret = format_count;
		DPU_ERROR("failed to get wb formats, %d\n", ret);
		return ret;
	}

	ret = drm_writeback_connector_init(wb_conn->drm_dev,
			&wb_conn->base, &dpu_wb_connector_funcs,
			&dpu_wb_encoder_helper_funcs,
			wb_formats, format_count, BIT(MAX_CRTC_COUNT) - 1);

	if (ret) {
		DPU_ERROR("failed to init wb connector, %d\n", ret);
		goto error_init;
	}

	drm_connector_helper_add(&wb_conn->base.base,
			&dpu_wb_conn_helper_funcs);

	ret = dpu_wb_create_properties(&wb_conn->base);
	if (ret) {
		DPU_ERROR("failed to create wb properties, %d\n", ret);
		goto error;
	}

	ret = dpu_wb_create_modifiers(&wb_conn->base,
			wb_formats, format_count);
	if (ret) {
		DPU_ERROR("failed to create wb modifiers, %d\n", ret);
		goto error;
	}
	wb_conn->mid_enable = false;
	kfree(wb_formats);

	return 0;

error:
	drm_connector_cleanup(&wb_conn->base.base);
	drm_encoder_cleanup(&wb_conn->base.encoder);
	drm_property_blob_put(wb_conn->base.pixel_formats_blob_ptr);
error_init:
	kfree(wb_formats);
	return ret;
}

static void dpu_wb_unbind(struct device *dev,
		struct device *master, void *data)
{
	struct dpu_wb_connector *wb_conn;
	struct dpu_drm_device *dpu_drm_dev;

	DPU_DEBUG("writeback unbind\n");

	wb_conn = platform_get_drvdata(to_platform_device(dev));
	dpu_drm_dev = to_dpu_drm_dev(wb_conn->drm_dev);

	wb_conn->drm_dev = NULL;
	wb_conn->index = 0;
}

static const struct component_ops dpu_wb_component_ops = {
	.bind = dpu_wb_bind,
	.unbind = dpu_wb_unbind,
};

static int dpu_wb_probe(struct platform_device *pdev)
{
	struct dpu_wb_connector *wb_conn;
	int ret;

	wb_conn = devm_kzalloc(&pdev->dev, sizeof(*wb_conn), GFP_KERNEL);
	if (!wb_conn)
		return -ENOMEM;

	ret = of_property_read_u32(pdev->dev.of_node, "cell-index",
			&wb_conn->index);
	if (ret)
		wb_conn->index = default_wb_id++;

	platform_set_drvdata(pdev, wb_conn);

	return component_add(&pdev->dev, &dpu_wb_component_ops);
}

static int dpu_wb_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &dpu_wb_component_ops);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id dpu_wb_of_match[] = {
	{.compatible = "xring,wb"},
	{},
};
MODULE_DEVICE_TABLE(of, dpu_wb_of_match);

static struct platform_driver dpu_wb_driver = {
	.probe = dpu_wb_probe,
	.remove = dpu_wb_remove,
	.driver = {
		.name = "xring_wb",
		.of_match_table = dpu_wb_of_match,
		.suppress_bind_attrs = true,
	},
};

int __init dpu_wb_register(void)
{
	return platform_driver_register(&dpu_wb_driver);
}

void __exit dpu_wb_unregister(void)
{
	platform_driver_unregister(&dpu_wb_driver);
}
