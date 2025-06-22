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
#include <linux/version.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_probe_helper.h>
#include <drm/display/drm_dp_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_atomic.h>
#include <drm/drm_vblank.h>
#include <drm/drm_property.h>
#include "dpu_module_drv.h"
#include "dp_display.h"
#include "dp_drm.h"
#include "dp_aux.h"
#include "dpu_crtc.h"
#include "dpu_trace.h"
#include "dpu_obuf.h"

#define DP_LP_ENTER_DELAY_MS 10000

static const struct drm_display_mode fail_modes[] = {
	{ DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
		25175, 640, 656, 752, 800, 0, 480, 490, 492, 525, 0,
		DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC) },
};

enum pack_data_format dp_connector_pack_fmt_get(struct drm_connector *connector)
{
	struct dp_connector *dp_connector;
	enum dp_bpc bpc;

	if (!connector) {
		DP_ERROR("parameter of connector is NULL\n");
		return POSTPIPE_PACK_FMT_RGB888;
	}

	dp_connector = to_dp_connector(connector);
	bpc = dp_connector->display->panel->video_info.bpc;

	switch (bpc) {
	case DP_BPC_6:
		return POSTPIPE_PACK_FMT_RGB666;
	case DP_BPC_8:
		return POSTPIPE_PACK_FMT_RGB888;
	case DP_BPC_10:
		return POSTPIPE_PACK_FMT_RGB101010;
	default:
		return POSTPIPE_PACK_FMT_RGB888;
	}
}

static void dp_underflow_work(struct work_struct *work)
{
	struct dp_intr_context *dp_intr_ctx;
	struct dpu_intr_pipe *intr_pipe;

	dp_intr_ctx = container_of(work, struct dp_intr_context, underflow_work);
	if (!dp_intr_ctx) {
		DPU_ERROR("dp_intr_ctx is NULL\n");
		return;
	}

	intr_pipe = dp_intr_ctx->intr_pipe;
	if (!intr_pipe->crtc) {
		DPU_DEBUG("crtc is null\n");
		return;
	}

	DPU_WARN("pipe %d, %s(frame_no %lld): underflow\n", drm_crtc_index(intr_pipe->crtc),
			intr_pipe->connector->name,
			dp_intr_ctx->underflow_frame_no);
}

static void dp_interrupt_init(struct dp_connector *dp_conn)
{
	INIT_WORK(&dp_conn->intr_ctx.underflow_work, dp_underflow_work);
}

static void dp_interrupt_deinit(struct dp_connector *dp_conn)
{
	/* do dp interrupt deinit */
}

void dpu_dp_irq_status_dump(struct drm_connector *connector)
{
	struct dpu_connector_intr_context *conn_intr_ctx;
	struct dp_intr_context *dp_intr_ctx;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;

	if (!connector) {
		DPU_ERROR("invalid parameters\n");
		return;
	}

	dpu_drm_dev = to_dpu_drm_dev(connector->dev);
	intr_ctx = dpu_drm_dev->dpu_kms->virt_ctrl.intr_ctx;
	dp_intr_ctx = &to_dp_connector(connector)->intr_ctx;
	conn_intr_ctx = &dp_intr_ctx->base;

	dpu_online_irq_status_dump(intr_ctx, conn_intr_ctx);
}

static void dp_irq_handler(int intr_id, void *data)
{
	struct dpu_connector_intr_context *conn_intr_ctx;
	struct dp_intr_context *dp_intr_ctx;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;
	struct dpu_crtc_state *cstate = NULL;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_intr_pipe *intr_pipe;
	struct dp_connector *dp_connector;
	enum dpu_intr_type intr_type;
	struct dpu_kms *dpu_kms;
	struct drm_crtc *crtc;
	int pipe_id;

	intr_ctx = data;
	intr_type = dpu_interrupt_id_to_type(intr_id);
	if (intr_type >= DPU_INTR_TYPE_MAX)
		return;

	intr_pipe = &intr_ctx->intr_pipe[intr_type];
	crtc = intr_pipe->crtc;
	if (!crtc) {
		DPU_ISR_DEBUG("crtc is null\n");
		return;
	}

	pipe_id = drm_crtc_index(crtc);
	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	dpu_kms->funcs->handle_intr_events(dpu_kms, crtc, intr_id);
	pipeline = &dpu_kms->virt_pipeline[pipe_id];
	dp_connector = to_dp_connector(intr_pipe->connector);
	dp_intr_ctx = &dp_connector->intr_ctx;
	conn_intr_ctx = &dp_intr_ctx->base;
	cstate = to_dpu_crtc_state(crtc->state);

	if (intr_id == conn_intr_ctx->intr_id[DPU_VSYNC_INT]) {
		dpu_intr_online_vsync_handler(pipeline, ktime_get_mono_fast_ns());

		if (dp_connector->display->panel->sink_caps.hdr_caps.caps_valid)
			dp_panel_send_metadata_intr_handler(dp_connector->display->panel);
		trace_dp_irq_handler("vsync", intr_id);
	} else if (intr_id == conn_intr_ctx->intr_id[DPU_UNDERFLOW_INT]) {
		dpu_intr_online_underflow_handler(intr_ctx, pipeline, intr_id);
		dp_intr_ctx->underflow_frame_no = cstate->frame_no;
		queue_work(pipeline->pipe_wq, &dp_intr_ctx->underflow_work);
		trace_dp_irq_handler("underflow", intr_id);
	} else if (intr_id == conn_intr_ctx->intr_id[DPU_VSTART_INT]) {
		dpu_intr_online_vstart_handler(pipeline);
		trace_dp_irq_handler("vstart_id", intr_id);
	} else if (intr_id == conn_intr_ctx->intr_id[DPU_CFG_RDY_CLR_INT]) {
		DPU_ISR_DEBUG("cfg_ready\n");
		dpu_intr_online_cfg_rdy_clr_handler(pipeline);
		trace_dp_irq_handler("cfg_ready", intr_id);
	} else if (intr_id == conn_intr_ctx->intr_id[DPU_EOF_INT]) {
		atomic_set(&pipeline->vstart_flag, 0);
	}
}

int dp_interrupt_register(struct drm_connector *connector, u32 intr_type)
{
	struct dpu_connector_intr_context *conn_intr_ctx;
	struct dp_intr_context *dp_intr_ctx;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;

	dp_intr_ctx = &to_dp_connector(connector)->intr_ctx;
	conn_intr_ctx = &dp_intr_ctx->base;
	dpu_drm_dev = to_dpu_drm_dev(connector->dev);
	intr_ctx = dpu_drm_dev->dpu_kms->virt_ctrl.intr_ctx;

	if (intr_type >= DPU_INTR_TYPE_MAX) {
		DPU_ERROR("invalid intr_type %d\n", intr_type);
		return -EINVAL;
	}

	dp_intr_ctx->intr_pipe = &intr_ctx->intr_pipe[intr_type];
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_CFG_RDY_CLR_INT);
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_VSYNC_INT);
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_VSTART_INT);
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_UNDERFLOW_INT);
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_EOF_INT);

	return dpu_intr_online_irq_register(connector, conn_intr_ctx, dp_irq_handler);
}

void dp_interrupt_unregister(struct drm_connector *connector, u32 intr_type)
{
	struct dp_intr_context *dp_intr_ctx;
	struct dp_connector *dp_connector;

	dp_connector = to_dp_connector(connector);
	dp_intr_ctx = &dp_connector->intr_ctx;

	dpu_intr_online_irq_unregister(connector, &dp_intr_ctx->base);
}

static int dp_connector_create_hdr_properties(struct drm_connector *connector)
{
	struct dp_connector *dp_conn = to_dp_connector(connector);
	struct drm_property_blob *blob;
	struct drm_property *property;

	/* hdr caps blob */
	blob = drm_property_create_blob(connector->dev,
			sizeof(struct dp_hdr_caps_sink), NULL);
	if (!blob) {
		DP_ERROR("failed to create hdr_caps blob\n");
		return -ENOMEM;
	}
	dp_conn->hdr_caps_blob = blob;

	/* hdr caps property */
	property = drm_property_create(connector->dev,
			DRM_MODE_PROP_BLOB, "dp_hdr_caps", 0);
	if (!property) {
		DP_ERROR("failed to create dp_hdr_caps property\n");
		return -ENOMEM;
	}

	drm_object_attach_property(&connector->base, property,
			dp_conn->hdr_caps_blob->base.id);
	dp_conn->hdr_caps_property = property;

	/* hdr metadata property, including static and dynamic */
	property = drm_property_create_range(connector->dev, 0,
			"dp_hdr_metadata", 0, ~0);
	if (!property) {
		DP_ERROR("failed to create dp_hdr_metadata\n");
		return -ENOMEM;
	}

	drm_object_attach_property(&connector->base, property, 0);
	dp_conn->hdr_metadata_property = property;

	return 0;
}

void dp_connector_update_hdr_caps_property(struct drm_connector *connector)
{
	struct dp_connector *dp_conn = to_dp_connector(connector);
	struct dp_hdr_caps_sink *hdr_caps = &dp_conn->display->panel->sink_caps.hdr_caps;

	if (hdr_caps->caps_valid)
		memcpy(dp_conn->hdr_caps_blob->data, (void *)hdr_caps,
				dp_conn->hdr_caps_blob->length);
	else
		memset(dp_conn->hdr_caps_blob->data, 0, dp_conn->hdr_caps_blob->length);
}

static int dp_connector_get_modes(struct drm_connector *connector)
{
	struct drm_display_mode *mode = NULL;
	struct dp_connector *dp_connector;
	struct dp_display *dp_display;
	struct drm_device *dev;
	struct edid *edid;
	int ret;

	if (!connector) {
		DP_ERROR("invalid parameters, %pK\n", connector);
		return -EINVAL;
	}

	dev = connector->dev;
	dp_connector = to_dp_connector(connector);
	dp_display = dp_connector->display;
	edid = dp_display->panel->edid;

	if (edid) {
		ret = drm_connector_update_edid_property(connector, edid);
		if (ret) {
			DP_ERROR("failed to update EDID property, ret %d\n", ret);
			return -EINVAL;
		}

		return drm_add_edid_modes(connector, edid);
	}

	mode = drm_mode_duplicate(dev, &fail_modes[0]);
	if (mode)
		drm_mode_probed_add(connector, mode);

	return 1;
}

static int dp_connector_detect_ctx(struct drm_connector *connector,
		struct drm_modeset_acquire_ctx *ctx, bool force)
{
	enum drm_connector_status status = connector_status_unknown;
	struct dp_connector *dp_connector;
	struct dp_display *dp_display;

	if (!connector || !ctx) {
		DP_ERROR("invalid parameters, %pK, %pK\n", connector, ctx);
		return -EINVAL;
	}

	dp_connector = to_dp_connector(connector);
	dp_display = dp_connector->display;

	status = dp_display->status.is_training_done
			? connector_status_connected
			: connector_status_disconnected;

	return (int)status;
}

static enum drm_mode_status dp_connector_mode_valid(
		struct drm_connector *connector,
		struct drm_display_mode *mode)
{
	struct dp_connector *dp_connector;
	struct dp_display *display;

	if (!connector || !mode) {
		DP_ERROR("invalid parameters, %pK, %pK\n", connector, mode);
		return MODE_ERROR;
	}

	dp_connector = to_dp_connector(connector);
	display = dp_connector->display;

	if (mode->clock >= display->status.maxpclk_khz || mode->clock > DP_MAX_IPI_CLK_KHZ)
		return MODE_ERROR;

	return MODE_OK;
}

static int dp_connector_atomic_check(struct drm_connector *connector,
		struct drm_atomic_state *state)
{
	DP_DEBUG("dp connector atomic_check\n");

	if (!connector || !state) {
		DP_ERROR("invalid parameters, %pK, %pK\n", connector, state);
		return -EINVAL;
	}

	return 0;
}

static void dp_connector_destroy(struct drm_connector *connector)
{
	struct dp_connector *dp_conn;

	if (!connector) {
		DP_ERROR("invalid parameters, %pK\n", connector);
		return;
	}

	dp_conn = to_dp_connector(connector);

	dp_interrupt_deinit(dp_conn);

	drm_connector_unregister(connector);
	drm_connector_cleanup(connector);

	kfree(dp_conn);
}

static int dp_connector_fill_modes(struct drm_connector *connector,
		uint32_t max_width, uint32_t max_height)
{
	int mode_count;

	if (!connector) {
		DP_ERROR("invalid parameters, %pK\n", connector);
		return 0;
	}

	mode_count = drm_helper_probe_single_connector_modes(connector,
			max_width, max_height);

	return mode_count;
}

static struct drm_connector_state *dp_connector_atomic_duplicate_state(
		struct drm_connector *connector)
{
	struct dp_connector_state *dp_conn_state;

	if (!connector) {
		DP_ERROR("invalid parameters, %pK\n", connector);
		return NULL;
	}

	dp_conn_state = kzalloc(sizeof(*dp_conn_state),
			GFP_KERNEL);
	if (!dp_conn_state)
		return NULL;

	__drm_atomic_helper_connector_duplicate_state(connector,
			&dp_conn_state->base);

	return &dp_conn_state->base;
}

static void dp_connector_atomic_destroy_state(
		struct drm_connector *connector,
		struct drm_connector_state *state)
{
	struct dp_connector_state *dp_conn_state;

	if (!connector || !state) {
		DP_ERROR("invalid parameters, %pK, %pK\n",
				connector, state);
		return;
	}

	dp_conn_state = to_dp_connector_state(state);

	__drm_atomic_helper_connector_destroy_state(
			&dp_conn_state->base);

	kfree(dp_conn_state);
}

static void dp_connector_reset(struct drm_connector *connector)
{
	struct dp_connector *dp_conn;
	struct dp_connector_state *dp_conn_state;

	if (!connector) {
		DP_ERROR("invalid parameters, %pK\n", connector);
		return;
	}

	dp_conn = to_dp_connector(connector);

	if (connector->state) {
		dp_connector_atomic_destroy_state(connector,
				connector->state);
		connector->state = NULL;
	}

	dp_conn_state = kzalloc(sizeof(*dp_conn_state),
			GFP_KERNEL);
	if (!dp_conn_state)
		return;

	__drm_atomic_helper_connector_reset(connector,
			&dp_conn_state->base);
}

static int dp_connector_set_hdr_metadata(struct dp_connector_state *state,
		void __user *uptr)
{
	if (uptr == NULL) {
		DP_ERROR("invalid user pointer, %p\n", uptr);
		return -EINVAL;
	}

	if (copy_from_user(&state->metadata, uptr, sizeof(struct dp_hdr_metadata))) {
		DP_ERROR("failed to copy userspace hdr metadata\n");
		return -ENOMEM;
	}

	if (!state->metadata.buffer) {
		DP_ERROR("metadata buffer is NULL\n");
		return -EINVAL;
	}

	switch (state->metadata.type) {
	case DP_STATIC_METADATA:
		DP_DEBUG("parse static metadata");
		if (state->metadata.size != sizeof(struct dp_hdr_static_metadata)) {
			DP_ERROR("static metadata size is invalid: %d\n",
					state->metadata.size);
			return -EINVAL;
		}

		if (copy_from_user(&state->static_metadata,
				(void __user *)state->metadata.buffer, state->metadata.size)) {
			DP_ERROR("failed to copy userspace hdr static metadata\n");
			return -EINVAL;
		}
		state->metadata_update = true;
		break;
	case DP_DYNAMIC_METADATA_HDR10_PLUS:
		DP_DEBUG("parse dynamic metadata");
		if (state->metadata.size > DP_MAX_METADATA_SIZE - 4) {
			DP_ERROR("dynamic metadata size is beyond capabilities: %d\n",
					DP_MAX_METADATA_SIZE - 4);
			return -EINVAL;
		}

		state->dynamic_metadata.data[0] = 0x04;
		state->dynamic_metadata.data[1] = 0x00;
		state->dynamic_metadata.data[2] = state->metadata.size;
		state->dynamic_metadata.data[3] = 0x00;

		if (copy_from_user(&(state->dynamic_metadata.data[4]),
				(void __user *)state->metadata.buffer, state->metadata.size)) {
			DP_ERROR("failed to copy userspace hdr static metadata\n");
			return -EINVAL;
		}
		state->dynamic_metadata.size = state->metadata.size + 4;
		state->metadata_update = true;
		break;
	default:
		DP_ERROR("unknown metadata type\n");
		return -EINVAL;
	}

	return 0;
}

static int dp_connector_atomic_set_property(
		struct drm_connector *connector,
		struct drm_connector_state *state,
		struct drm_property *property,
		uint64_t val)
{
	struct dp_connector_state *dp_conn_state;
	struct dp_connector *dp_conn;
	int ret;

	if (!connector || !state || !property) {
		DP_ERROR("invalid parameters, %pK, %pK, %pK\n",
				connector, state, property);
		return -EINVAL;
	}

	dp_conn_state = to_dp_connector_state(state);
	dp_conn = to_dp_connector(connector);

	if (property == dp_conn->hdr_metadata_property) {
		ret = dp_connector_set_hdr_metadata(dp_conn_state,
				(void *)(uintptr_t)val);
		if (ret) {
			DP_ERROR("failed to set hdr metadata\n");
			return -EINVAL;
		}
	} else if (property == dp_conn->dp_protect_property) {
		dp_conn_state->protect_enable = val;
	} else {
		DP_ERROR("unknow property %s: %lld\n", property->name, val);
		return -EINVAL;
	}

	DPU_PROPERTY_DEBUG("set property %s: %lld\n", property->name, val);

	return 0;
}

static int dp_connector_atomic_get_property(
		struct drm_connector *connector,
		const struct drm_connector_state *state,
		struct drm_property *property,
		uint64_t *val)
{
	struct dp_connector_state *dp_conn_state;
	struct dp_connector *dp_conn;

	if (!connector || !state || !property || !val) {
		DP_ERROR("invalid parameters, %pK, %pK, %pK, %pK\n",
				connector, state, property, val);
		return -EINVAL;
	}

	dp_conn = to_dp_connector(connector);
	dp_conn_state = to_dp_connector_state(state);

	if (property == dp_conn->hdr_caps_property) {
		DP_DEBUG("get hdr caps property");
		*val = dp_conn->hdr_caps_blob->base.id;
	} else if (property == dp_conn->hdr_metadata_property) {
		DP_DEBUG("get hdr_metadata_property");
		*val = 0;
	} else if (property == dp_conn->dp_protect_property) {
		*val = 0;
	} else {
		DP_ERROR("unknow property %s: %lld\n", property->name, *val);
		return -EINVAL;
	}

	DPU_PROPERTY_DEBUG("set property %s: %lld\n", property->name, *val);

	return 0;
}

static void dp_encoder_disable(struct drm_encoder *encoder)
{
	struct dp_encoder *dp_encoder;
	struct dp_display *display;
	struct dp_hpd *hpd;

	DP_DEBUG("dp encoder disable\n");

	if (!encoder) {
		DP_ERROR("invalid parameters, %pK\n", encoder);
		return;
	}

	dp_encoder = to_dp_encoder(encoder);
	display = dp_encoder->display;
	hpd = display->hpd;

	if (!display->status.is_connector_enabled) {
		DP_ERROR("dp connector is already disabled\n");
		return;
	}
	display->status.is_connector_enabled = false;

	/* update tmg state before video stream is off */
	dp_display_tmg_state_get(display);

	/* hardware constraint: special vfp value, need disable dpu top auto cg */
	if (display->panel->video_info.timing.v_front_porch ==
			DP_TIMING_VFP_NEED_DISABLE_DPU_TOP_AUTO_CG_VAL)
		dpu_core_perf_top_cg_enable();

	dp_display_send_black_frame(display);

	kthread_queue_delayed_work(&display->lp_worker,
			&display->lp_delayed_work.base, msecs_to_jiffies(DP_LP_ENTER_DELAY_MS));

	mutex_lock(&hpd->lock);
	if (hpd->last_off) {
		dp_display_video_stream_off(display);

		/* clear video info and dsc info */
		dp_display_clear_video_info(display);

		kthread_cancel_delayed_work_sync(&display->lp_delayed_work.base);

		hpd->last_off = false;
	}
	mutex_unlock(&hpd->lock);
}

static void dp_encoder_enable(struct drm_encoder *encoder)
{
	struct dp_display *display;
	struct dpu_crtc *crtc;
	struct dp_panel_video_info *video_info;
	struct dp_panel_dsc_info *dsc_info;

	DP_DEBUG("dp encoder enable\n");

	if (!encoder) {
		DP_ERROR("invalid parameters, %pK\n", encoder);
		return;
	}

	display = to_dp_encoder(encoder)->display;
	crtc = to_dpu_crtc(encoder->crtc);
	video_info = &display->panel->video_info;
	dsc_info = &display->panel->dsc_info;

	if (display->status.is_connector_enabled) {
		DP_ERROR("dp connector is already enabled");
		return;
	}

	if (!display->status.is_training_done) {
		DP_ERROR("dp link is not ready, training_done: %d\n",
				display->status.is_training_done);
		return;
	}

	if (display->hpd->first_on) {
		/* prepare video and dsc info */
		dp_display_prepare_video_info(display,
				&encoder->crtc->state->mode, &crtc->dsc_cfg);

		if (!dp_display_check_tmg_timing(&video_info->timing, dsc_info->dsc_en))
			DP_ERROR("timing is unsupported for timing engine hardware module\n");

		dp_display_video_stream_on(display);

		/* consume the flag provided by hpd */
		display->hpd->first_on = false;
	}

	kthread_cancel_delayed_work_sync(&display->lp_delayed_work.base);
	dp_display_lp_exit(display);

	dp_display_video_stream_self_test(display);

	/* hardware constraint: special vfp value, need disable dpu top auto cg */
	if (video_info->timing.v_front_porch ==
			DP_TIMING_VFP_NEED_DISABLE_DPU_TOP_AUTO_CG_VAL)
		dpu_core_perf_top_cg_disable();

	display->status.is_connector_enabled = true;
}

static int dp_encoder_atomic_check(struct drm_encoder *encoder,
		struct drm_crtc_state *crtc_state,
		struct drm_connector_state *conn_state)
{
	DP_DEBUG("dp encoder check\n");

	if (!encoder || !crtc_state || !conn_state) {
		DP_ERROR("invalid parameters, %pK, %pK, %pK\n",
				encoder, crtc_state, conn_state);
		return -EINVAL;
	}

	return 0;
}

static void dp_encoder_destroy(struct drm_encoder *encoder)
{
	struct dp_encoder *dp_enc;

	if (!encoder) {
		DP_ERROR("invalid parameters, %pK\n", encoder);
		return;
	}

	dp_enc = to_dp_encoder(encoder);

	drm_encoder_cleanup(encoder);

	kfree(dp_enc);
}

static const struct drm_connector_helper_funcs dp_connector_helper_ops = {
	.get_modes = dp_connector_get_modes,
	.detect_ctx = dp_connector_detect_ctx,
	.mode_valid = dp_connector_mode_valid,
	.atomic_check = dp_connector_atomic_check,
};

static const struct drm_connector_funcs dp_connector_ops = {
	.reset = dp_connector_reset,
	.destroy = dp_connector_destroy,
	.fill_modes = dp_connector_fill_modes,
	.atomic_duplicate_state = dp_connector_atomic_duplicate_state,
	.atomic_destroy_state = dp_connector_atomic_destroy_state,
	.atomic_set_property = dp_connector_atomic_set_property,
	.atomic_get_property = dp_connector_atomic_get_property,
};

static const struct drm_encoder_helper_funcs dp_encoder_helper_funcs = {
	.disable = dp_encoder_disable,
	.enable = dp_encoder_enable,
	.atomic_check = dp_encoder_atomic_check,
};

static const struct drm_encoder_funcs dp_encoder_funcs = {
	.destroy = dp_encoder_destroy,
};

static void dp_connector_pre_commit(struct drm_atomic_state *state, void *data)
{
	struct drm_connector *drm_connector = data;
	struct dp_connector_state *dp_conn_state;
	struct dp_connector *dp_conn;
	struct dp_display *display;
	bool obufen_updated = false;
	u32 obufen;

	dp_conn = to_dp_connector(drm_connector);
	display = dp_conn->display;

	obufen = dpu_obuf_check_obufen(DPU_OBUF_CLIENT_DP, &obufen_updated);
	/* obufen = 0 only allowed to be updated in disp off cmt (encoder_disable) */
	if (obufen_updated && obufen != 0) {
		dpu_obuf_update_obufen_cmt(DPU_OBUF_CLIENT_DP, obufen_updated);
		dp_hw_tmg_obufen_config(&display->hw_modules.hw_tmg->hw, obufen);
		dp_display_line_buf_level_cfg(display);
	}

	dp_conn_state = to_dp_connector_state(drm_connector->state);

#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
	int ret = 0;
	/* trigger hdcp event while displaying first frame */
	if (display->status.is_connector_enabled &&
			(display->status.hpd == DP_HPD_EVENT_PLUG_IN)) {
		ret = dp_hdcp_monitor_start(display->hdcp);
		if (ret)
			DP_WARN("hdcp start failed, cannot do encryption\n");
	}

#endif
}

static void dp_connector_post_commit(struct drm_atomic_state *state, void *data)
{
	struct drm_connector *drm_connector = data;
	struct dp_connector *dp_conn;
	struct dp_connector_state *dp_conn_state;

	DP_DEBUG("dp connector id: %d\n", drm_connector->index);

	dp_conn = to_dp_connector(drm_connector);
	dp_conn_state = to_dp_connector_state(drm_connector->state);

	if (dp_conn_state->metadata_update) {
		dp_panel_config_hdr_commit(dp_conn->display->panel, dp_conn_state);
		dp_conn_state->metadata_update = false;
	}

	if (dp_conn_state->protect_enable)
		dp_conn_state->protect_enable = false;

	DP_DEBUG("dp connector commit exit\n");
}

static struct drm_property *dp_connector_create_range_property(
		struct drm_connector *connector,
		char *name, uint64_t min, uint64_t max)
{
	struct drm_property *prop;

	prop = drm_property_create_range(connector->dev, 0,
			name, min, max);
	if (!prop)
		return NULL;

	drm_object_attach_property(&connector->base, prop, 0);

	return prop;
}

static int dp_connector_create_properties(struct drm_connector *connector)
{
	struct dp_connector *dp_conn;
	int ret;

	if (!connector) {
		DP_ERROR("invalid parameters, %pK\n", connector);
		return -EINVAL;
	}

	dp_conn = to_dp_connector(connector);

	ret = dp_connector_create_hdr_properties(connector);
	if (ret)
		return ret;

	dp_conn->dp_protect_property = dp_connector_create_range_property(
			connector, "dp_protect_enable", 0, 1);
	if (!dp_conn->dp_protect_property)
		return -ENOMEM;

	return 0;
}

static struct drm_connector *dp_connector_init(struct drm_device *drm_dev,
		struct dp_display *display)
{
	struct dp_encoder *dp_enc;
	struct dp_connector *dp_conn;
	int ret;

	dp_enc = kzalloc(sizeof(*dp_enc), GFP_KERNEL);
	if (!dp_enc)
		return ERR_PTR(-ENOMEM);

	dp_conn = kzalloc(sizeof(*dp_conn), GFP_KERNEL);
	if (!dp_conn) {
		kfree(dp_enc);
		return ERR_PTR(-ENOMEM);
	}

	dp_enc->base.possible_crtcs = BIT(MAX_CRTC_COUNT) - 1;

	drm_encoder_helper_add(&dp_enc->base,
			&dp_encoder_helper_funcs);

	ret = drm_encoder_init(drm_dev, &dp_enc->base,
			&dp_encoder_funcs,
			DRM_MODE_ENCODER_TMDS, NULL);
	if (ret) {
		DP_ERROR("failed to init drm encoder\n");
		goto fail;
	}

	ret = drm_connector_init(drm_dev, &dp_conn->base,
			&dp_connector_ops,
			DRM_MODE_CONNECTOR_DisplayPort);
	if (ret) {
		DP_ERROR("failed to init drm connector\n");
		goto connector_fail;
	}

	dp_conn->base.helper_private = &dp_connector_helper_ops;

	ret = drm_connector_attach_encoder(&dp_conn->base,
			&dp_enc->base);
	if (ret) {
		DP_ERROR("failed to attach encoder\n");
		goto attach_fail;
	}

	ret = dp_connector_create_properties(&dp_conn->base);
	if (ret) {
		DPU_ERROR("failed to create properties, ret %d\n", ret);
		goto attach_fail;
	}

	ret = dpu_kms_post_commit_cb_register(drm_dev,
			&dp_connector_post_commit, &dp_conn->base);
	if (ret) {
		DP_ERROR("failed to register post commit cb, ret %d\n", ret);
		goto attach_fail;
	}

	dp_interrupt_init(dp_conn);

	ret = dpu_kms_pre_commit_cb_register(drm_dev,
			&dp_connector_pre_commit, &dp_conn->base);
	if (ret) {
		DP_ERROR("failed to register pre commit cb, ret %d\n", ret);
		goto pre_cb_fail;
	}

	dp_conn->index = display->parser->index;
	dp_conn->encoder = &dp_enc->base;
	dp_conn->display = display;
	dp_enc->display = display;

	return &dp_conn->base;

pre_cb_fail:
	dpu_kms_post_commit_cb_unregister(drm_dev, &dp_connector_post_commit);
attach_fail:
	drm_connector_cleanup(&dp_conn->base);
connector_fail:
	drm_encoder_cleanup(&dp_enc->base);
fail:
	kfree(dp_conn);
	kfree(dp_enc);
	return ERR_PTR(ret);
}

static int dp_bind(struct device *dev, struct device *master, void *data)
{
	struct drm_connector *drm_dp_conn;
	struct dp_display *dp_display;
	struct drm_device *drm_dev;
	struct drm_dp_aux *drm_aux;

	DP_DEBUG("dp bind\n");

	dp_display = platform_get_drvdata(to_platform_device(dev));
	drm_dev = dev_get_drvdata(master);

	drm_aux = &dp_display->aux->base;
	drm_aux->name = "dp_aux";
	drm_aux->dev = dev;
	drm_aux->drm_dev = drm_dev;
	drm_aux->transfer = dp_aux_transfer;
	drm_dp_aux_register(drm_aux);

	drm_dp_conn = dp_connector_init(drm_dev, dp_display);
	if (IS_ERR_OR_NULL(drm_dp_conn)) {
		DP_ERROR("failed to init dp connector\n");
		return PTR_ERR(drm_dp_conn);
	}

	dp_display->drm_dev = drm_dev;
	dp_display->connector = drm_dp_conn;

	return 0;
}

static void dp_unbind(struct device *dev, struct device *master, void *data)
{
	struct dp_display *dp_display;

	DP_DEBUG("dp unbind\n");

	dp_display = platform_get_drvdata(to_platform_device(dev));

	drm_dp_aux_unregister(&dp_display->aux->base);

	dp_display->connector = NULL;

}

static const struct component_ops dp_component_ops = {
	.bind = dp_bind,
	.unbind = dp_unbind,
};

int dp_drm_init(struct platform_device *pdev)
{
	return component_add(&pdev->dev, &dp_component_ops);
}

void dp_drm_deinit(struct platform_device *pdev)
{
	component_del(&pdev->dev, &dp_component_ops);
}
