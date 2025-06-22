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

#ifndef _DP_DRM_H_
#define _DP_DRM_H_

#include <display/xring_dpu_color.h>
#include <drm/drm_device.h>
#include <drm/drm_connector.h>
#include <drm/drm_encoder.h>
#include "dpu_drv.h"
#include "dpu_kms.h"
#include "dpu_intr_helper.h"

/**
 * dp_intr_context - dp interrupt context
 * @intr_pipe: the point of dpu intr pipe
 * @base: the point of dpu connector intr context
 * @underflow_work: work for underflow intr
 * @underflow_frame_no: the frame no has underflow
 */
struct dp_intr_context {
	struct dpu_intr_pipe *intr_pipe;

	struct dpu_connector_intr_context base;
	struct work_struct underflow_work;
	u64 underflow_frame_no;
};

/**
 * dp_connector - the dp connector structure
 * @base: the drm connector
 * @index: the index of connector
 * @display: the pointer of dp display
 * @encoder: the attached encoder for this connector
 * @intr_ctx: the interrupt context of dp connector
 * @hdr_caps_property: hdr capabilities property
 * @hdr_caps_blob: hdr capabilities blob
 * @hdr_metadata_property: hdr metadata property
 */
struct dp_connector {
	struct drm_connector base;
	u32 index;

	struct dp_display *display;
	struct drm_encoder *encoder;

	struct dp_intr_context intr_ctx;

	struct drm_property *hdr_caps_property;
	struct drm_property_blob *hdr_caps_blob;
	struct drm_property *hdr_metadata_property;
	struct drm_property *dp_protect_property;
};

#define to_dp_connector(x) container_of(x, struct dp_connector, base)

/**
 * dp_connector_state - the dp connector state structure
 * @base: the drm connector state
 * @metadata_update: if metadata is updated
 * @metadata: metadata pointer to record userspace buffer
 * @static_metadata: static metadata data
 * @dynamic_metadata: dynamic metadata data
 */
struct dp_connector_state {
	struct drm_connector_state base;

	bool metadata_update;
	u64 protect_enable;

	struct dp_hdr_metadata metadata;
	struct dp_hdr_static_metadata static_metadata;
	struct dp_hdr_dynamic_metadata dynamic_metadata;
};

#define to_dp_connector_state(x) \
		container_of(x, struct dp_connector_state, base)

/**
 * dp_encoder - the dp encoder structure
 * @base: the drm encoder
 * @display: the pointer of dp display
 */
struct dp_encoder {
	struct drm_encoder base;
	struct dp_display *display;
};

#define to_dp_encoder(x) container_of(x, struct dp_encoder, base)

/**
 * dp_drm_init - init dp drm
 * @pdev: the pointer of platform device
 *
 * Return: zero on success, -errno on failure
 */
int dp_drm_init(struct platform_device *pdev);

/**
 * dp_drm_deinit - deinit dp drm
 * @pdev: the pointer of platform device
 */
void dp_drm_deinit(struct platform_device *pdev);

/**
 * dp_interrupt_register - register dp interrupts
 * @connector: the pointer of dp drm connector
 * @intr_type: the dpu interrupt type for this connector
 *
 * Return: zero on success, -errno on failure
 */
int dp_interrupt_register(struct drm_connector *connector, u32 intr_type);

/**
 * dp_interrupt_register - register dp interrupts
 * @connector: the pointer of dp drm connector
 * @intr_type: the dpu interrupt type for this connector
 */
void dp_interrupt_unregister(struct drm_connector *connector, u32 intr_type);

/**
 * dp_connector_update_hdr_caps_property - update hdr caps blob property
 * @connector: the pointer of dp drm connector
 */
void dp_connector_update_hdr_caps_property(struct drm_connector *connector);

/*
 * dp_connector_pack_fmt_get - get post pipe pack data format
 * @connector: the pointer of dp drm connector
 */
enum pack_data_format dp_connector_pack_fmt_get(struct drm_connector *connector);

/*
 * dpu_dp_irq_status_dump - dump dp irq status
 * @connector: the pointer of dp drm connector
 */
void dpu_dp_irq_status_dump(struct drm_connector *connector);

#endif /* _DP_DRM_H_ */
