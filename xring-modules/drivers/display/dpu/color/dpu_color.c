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
#include "dpu_log.h"
#include "dpu_crtc.h"
#include "dpu_kms.h"

static void color_pre_hist_get(struct drm_crtc *crtc)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	dpu_pre_color_hist_get(crtc->dev, &dpu_kms->color->pre_color);
}

static int color_pre_hist_event_cb(struct drm_crtc *crtc)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_pre_color *color;
	struct dpu_kms *dpu_kms;

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	color = &dpu_kms->color->pre_color;

	if (!color->hist_event.enable)
		return 0;

	if (color->hist_event.hist_read_mode == HIST_READ_ONCE) {
		if (atomic_read(&color->hist_event.read_cnt) <= 0)
			return 0;

		atomic_dec(&color->hist_event.read_cnt);
	}

	dpu_crtc_intr_event_queue(crtc, color_pre_hist_get);
	return 0;
}

static int color_pre_hist_event_enable(struct drm_crtc *crtc,
		bool enable, void *event)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_pre_color *color;
	struct dpu_kms *dpu_kms;

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	color = &dpu_kms->color->pre_color;

	atomic_set(&color->hist_event.read_cnt, 0);
	color->hist_event.hist_read_mode = HIST_READ_DISABLE;
	color->hist_event.enable = false;

	DPU_COLOR_DEBUG("%s pre hist event\n", enable ? "enable" : "disable");

	return 0;
}

int dpu_color_pre_hist_event_init(struct drm_device *dev, void *event)
{
	struct dpu_crtc_event *color_event = event;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;

	dpu_drm_dev = to_dpu_drm_dev(dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	if (!dpu_kms->color)
		return -EINVAL;

	switch (color_event->event_type) {
	case DRM_PRIVATE_EVENT_VCHN0_HISTOGRAM:
		color_event->intr_id = INTR_ONLINE0_FRM_TIMING_EOF;
		color_event->enable = color_pre_hist_event_enable;
		color_event->cb = color_pre_hist_event_cb;
		break;

	default:
		DPU_WARN("unknow hist event type %d", color_event->event_type);
		return -ENOENT;
	}

	return 0;
}

int dpu_color_init(struct dpu_color **color, struct drm_device *dev)
{
	struct dpu_color *dpu_color;

	if (*color) {
		DPU_ERROR("dpu color has been initialized\n");
		return -EINVAL;
	}

	dpu_color = kzalloc(sizeof(*dpu_color), GFP_KERNEL);
	if (!dpu_color)
		return -ENOMEM;

	dpu_color->dev = dev;
	dpu_color->pre_color.hw_hist = NULL;
	dpu_color->pre_color.hist_event.enable = 0;
	dpu_color->pre_color.hist_event.hist_blob = drm_property_create_blob(dev,
			sizeof(struct dpu_hist_data), NULL);
	if (IS_ERR(dpu_color->pre_color.hist_event.hist_blob))
		dpu_color->pre_color.hist_event.hist_blob = NULL;

	*color = dpu_color;

	return 0;
}

void dpu_color_deinit(struct dpu_color *color)
{
	if (!color)
		return;

	/**
	 * hist_blob would be free in drm_mode_config_cleanup,
	 * it would be called before kms->deinit.
	 */
	color->pre_color.hist_event.hist_blob = NULL;
	kfree(color);
}
