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

#ifndef _DSI_ENCODER_H_
#define _DSI_ENCODER_H_

#include <drm/drm_encoder.h>

#include "dpu_hw_format.h"
#include "dsi_display.h"
#include "dsi_tmg.h"

#define to_dsi_encoder(x) \
		container_of(x, struct dsi_encoder, base)

/**
 * dsi_encoder - dsi encoder
 * @base: the drm encoder instanse
 * @display: the dsi display phandle
 * @connector: the drm connector phandle
 */
struct dsi_encoder {
	struct drm_encoder base;
	struct drm_connector *connector;

	struct dsi_display *display;
};

/**
 * dsi_encoder_pack_fmt_get - get pack format for post pipe top
 * @encoder: the phandle of drm encoder
 *
 * Return: the pack format
 */
enum pack_data_format dsi_encoder_pack_fmt_get(struct drm_encoder *encoder);

/**
 * dsi_encoder_create - create the dsi encoder object
 * @drm_dev: the pointer of drm device
 * @display: the dsi display
 *
 * Return: valid pointer on success, invalid pointer on failure
 */
struct drm_encoder *dsi_encoder_create(struct drm_device *drm_dev,
		struct dsi_display *display);

/**
 * dsi_encoder_destroy - destroy the dsi encoder instance and release
 * reserved resources.
 * @drm_encoder: the drm encoder phandle
 */
void dsi_encoder_destroy(struct drm_encoder *drm_encoder);

int dsi_encoder_tmg_enable(struct dsi_encoder *dsi_encoder);

int dsi_encoder_tmg_disable(struct dsi_encoder *dsi_encoder);
int dsi_encoder_tmg_doze_enable(struct dsi_encoder *dsi_encoder);
int dsi_encoder_tmg_status_dump(struct dsi_encoder *dsi_encoder);

/**
 * dsi_encoder_tmg_mask_get - get the mask of enabled timing engine
 * @encoder: the phandle of drm encoder
 * @tmg_mask: the output result pointer
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_encoder_tmg_mask_get(struct drm_encoder *encoder, u32 *tmg_mask);

#endif
