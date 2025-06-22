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

#ifndef _DPU_WB_H_
#define _DPU_WB_H_

#include <linux/hrtimer.h>
#include <drm/drm_device.h>
#include <drm/drm_writeback.h>
#include <display/xring_dpu_drm.h>
#include "dpu_hw_wb.h"
#include "dpu_hw_wb_scaler.h"
#include "display/xring_stripe_common.h"

enum dpu_hw_wb_pos_type {
	WB_POS_TYPE_AFTER_MIXER,
	WB_POS_TYPE_AFTER_POSTPQ,
	WB_POS_TYPE_AFTER_RC,
	WB_POS_TYPE_BEFORE_PREPQ,
	WB_POS_TYPE_AFTER_PREPQ,
	WB_POS_TYPE_INVALID,
	WB_POS_TYPE_MAX = WB_POS_TYPE_INVALID,
};

struct wb_intr_state {
	/* registered interrupt index for wb connector */
	u32 wb_done_id;
	u32 overflow_id;
};

/**
 * dpu_wb_connector - the dpu wb connector structure
 * @base: the drm writeback connector
 * @index: the index of dpu connector
 * @is_online: flag of online wb
 * @wb_intr_state: the interrupt state of wb connector
 * @drm_dev: the drm device pointer
 * @hw_wb: the hardware handle pointer of writeback
 * @hw_wb_scaler: the hardware handle pointer of writeback 1d scaler
 * @pos_type_prop: pointer of wb input position type property
 * @modifier_prop: pointer of wb modifier property
 * @rotation_prop: pointer of wb rotation property
 * @src_rect_prop: pointer of wb src rect property
 * @dst_rect_prop: pointer of wb dst rect property
 * @stripe_count_prop: pointer of stripe count property
 * @stripe_para_blob_prop: pointer of wb stripe para blob property
 * @stripe_para_blob_size: wb stripe blob size property
 * @wb_slice_node_id: the array for saveing node id in stripe cmdlist config scence
 * @scaler_enable_prop: pointer of wb scaler enable property
 * @scaler_blob_prop: pointer of wb scaler blob property
 * @scaler_blob_size: pointer of wb blob size property
 * @wfd_source_slice_prop: pointor of wfd source slice number property
 * @prepq_wb_prop: pointer of prepq input or output wb position property
 */
struct dpu_wb_connector {
	struct drm_writeback_connector base;
	u32 index;
	bool is_online;
	bool mid_enable;
	struct wb_intr_state intr_state;

	struct drm_device *drm_dev;
	struct dpu_hw_wb *hw_wb;
	struct dpu_hw_wb_scaler *hw_wb_scaler;

	struct drm_property *pos_type_prop;
	struct drm_property *modifier_prop;
	struct drm_property *rotation_prop;

	struct drm_property *src_rect_prop;
	struct drm_property *dst_rect_prop;
	struct drm_property *scaler_enable_prop;
	struct drm_property *scaler_blob_prop;
	u32 scaler_blob_size;
	struct drm_property *dither_enable_prop;
	struct drm_property *dither_blob_prop;
	u32 dither_blob_size;
	struct drm_property *csc_blob_prop;
	u32 csc_blob_size;

	struct drm_property *protected_buffer_enable_prop;
	struct drm_property *stripe_count_prop;
	struct drm_property *stripe_para_blob_prop;
	u32 stripe_para_blob_size;

	s64 wb_slice_node_id[MAX_STRIPE_COUNT];
	struct drm_property *wfd_source_slice_prop;

	struct drm_property *prepq_wb_prop;
};

#define to_dpu_wb_connector(x) \
		container_of(x, struct dpu_wb_connector, base)

/**
 * dpu_wb_connector_state - the dpu wb connector state structure
 * @base: the drm connector state
 * @fb: the drm framebuffer for wb
 * @pos_type: the input position type
 * @rotation: the wb rotation bitmask
 * @src_rect: wb src rect area
 * @dst_rect: wb dst rect area
 * @wb_stripe_count: the count of wb stripe
 * @stripe_para_blob_id: wb stripe para blob id
 * @stripe_para_blob_ptr: pointer of wb stripe para
 * @scaler_enable: flag of wb scaler enable
 * @scaler_blob_ptr: pointer of wb scaler blob
 * @dither_enable: flag of wb dither enable
 * @dither_blob_ptr: pointer of wb dither blob
 * @csc_blob_ptr: pointer of wb csc blob
 * @wfd_source_slice_num: WFD source slice number
 * @prepq_wb: prepq input or output wb position
 */
struct dpu_wb_connector_state {
	struct drm_connector_state base;
	struct drm_framebuffer *fb;

	enum dpu_hw_wb_pos_type pos_type;
	u32 rotation;
	struct dpu_general_rect src_rect;
	struct dpu_general_rect dst_rect;

	bool protected_buffer_enable;
	u16 wb_stripe_count;
	u64 stripe_para_blob_id;
	struct drm_property_blob *stripe_para_blob_ptr;

	bool scaler_enable;
	struct drm_property_blob *scaler_blob_ptr;
	bool dither_enable;
	struct drm_property_blob *dither_blob_ptr;
	struct drm_property_blob *csc_blob_ptr;
	u16 wfd_source_slice_num;

	struct dpu_prepq_writeback prepq_wb;
};

#define to_dpu_wb_connector_state(x) \
		container_of(x, struct dpu_wb_connector_state, base)

/**
 * dpu_wb_interrupt_register - register interrupts for wb
 * @connector: drm connector pointer
 * @type: indicate which pipe need to be enabled
 *
 * Return: zero on success, -errno on failure
 */
int dpu_wb_interrupt_register(struct drm_connector *connector, u32 type);

/**
 * dpu_wb_interrupt_unregister - unregister interrupts for wb
 * @connector: drm connector pointer
 * @type: indicate which pipe need to be enabled
 */
void dpu_wb_interrupt_unregister(struct drm_connector *connector, u32 type);

/**
 * dpu_wb_resource_reserve - reserve hardware resource for wb
 * @connector: drm connector pointer
 * @pipe_id: the connected crtc index
 * @blk_id: the block id
 * @is_clone: is it clone mode
 * @test_only: is only for testing
 *
 * Return: zero on success, -errno on failure
 */
int dpu_wb_resource_reserve(struct drm_connector *connector, u32 pipe_id,
		unsigned long blk_id, bool is_clone, bool test_only);

/**
 * dpu_wb_resource_release - release the hardware resource for wb
 * @connector: drm connector pointer
 */
void dpu_wb_resource_release(struct drm_connector *connector);

/**
 * dpu_wb_connector_job_clear - clear wb job if wb broken down
 *
 * @crtc: the drm crtc bind to wb connector
 * @drm_wb_conn: pointer of struct drm_writeback_connector
 */
void dpu_wb_connector_job_clear(struct drm_crtc *crtc,
		struct drm_writeback_connector *drm_wb_conn);

/**
 * dpu_wb_need_cfg_rdy_update - check wb connector whether need cfg rdy update or not
 *@connector: the drm connector pointer
 *@is_offline: whether the connector is offline
 *
 * Return: ture for need cfg rdy update, false for not need
 */
bool dpu_wb_need_cfg_rdy_update(struct drm_connector *connector,
		bool is_offline);

#endif /* _DPU_WB_H_ */
