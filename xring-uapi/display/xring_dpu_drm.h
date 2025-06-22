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

#ifndef __XRING_DPU_DRM_H__
#define __XRING_DPU_DRM_H__

#include <linux/types.h>
#include <drm/drm.h>

/* the maximum number of supported solid layer */
#define MAX_DPU_SOLID_LAYER_COUNT 16

/**
 * @SOLID_MODE_INSIDE: dpu solid mode inside
 * @SOLID_MODE_OUTSIDE: dpu solid mode outside
 */
#define SOLID_MODE_INSIDE 0
#define SOLID_MODE_OUTSIDE 1

/**
 * @DPU_BLEND_MODE_PREMULTI: This Layer has already been pre multiplied
 * @DPU_BLEND_MODE_COVERAGE: This layer is not pre multiplied
 * @DPU_BLEND_MODE_NONE: Do not mix, directly cover
 */
#define SOLID_BLEND_MODE_INVALID -1
#define SOLID_BLEND_MODE_PREMULTI 0
#define SOLID_BLEND_MODE_COVERAGE 1
#define SOLID_BLEND_MODE_NONE 2

/* the maximum number of supported roi in one layer */
#define MAX_ROI_COUNT 2

/**
 * dpu_general_color - dpu general color config structure
 * @r: color red
 * @g: color green
 * @b: color blue
 * @a: color alpha
 */
struct dpu_general_color {
	__u16 r;
	__u16 g;
	__u16 b;
	__u16 a;
};

/**
 * dpu_general_rect - dpu general rectangle structure
 * @x: the x coordinate of rectangle
 * @y: the y coordinate of rectangle
 * @w: the width of rectangle
 * @h: the height of rectangle
 */
struct dpu_general_rect {
	__u16 x;
	__u16 y;
	__u16 w;
	__u16 h;
};

/**
 * struct dpu_solid_layer_cfg - the config for one solid layer
 * @zpos: the zorder of solid layer
 * @blend_mode: the blend mode of solid layer, available values:
 *              SOLID_BLEND_MODE_PREMULTI
 *              SOLID_BLEND_MODE_COVERAGE
 *              SOLID_BLEND_MODE_NONE
 * @solid_mode: solid_mode to represent，available values:
 *              SOLID_MODE_INSIDE
 *              SOLID_MODE_OUTSIDE
 * @plane_alpha: Opacity of the plane with 0 as completely transparent and 0xffff as completely opaque.
 * @color: dpu general color config structure for solid
 * @rect: the general rectangle structure for solid
 */
struct dpu_solid_layer_cfg {
	__u16 zpos;
	__u16 blend_mode;
	__u16 solid_mode;
	__u16 plane_alpha;
	struct dpu_general_color color;
	struct dpu_general_rect rect;
};

/**
 * struct dpu_solid_layer - the all solid layer configs for one committing
 * @count: the total solid layer count
 * @cfg: the detailed solid layer config
 */
struct dpu_solid_layer {
	__u32 count;
	__u32 reserve;
	struct dpu_solid_layer_cfg cfg[MAX_DPU_SOLID_LAYER_COUNT];
};

/**
 * struct dpu_plane_roi - the all roi configs for one layer
 * @count: the total roi rect count
 * @roi: the detailed roi config
 */
struct dpu_plane_roi {
	__u32 count;
	__u32 reserve;
	struct dpu_general_rect roi[MAX_ROI_COUNT];
};

/**
 * struct dpu_sram_param - the sram param of dpu rdma
 * @total_sram_size: rdma path used total sram size
 * @left_sram_size: rdma path left/signal planar image used sram size
 * @right_sram_size: rdma path right planar image used sram size
 * @sram_base_addr: rdma path used sram base addr
 * @sram_mem_map: rdma path sram sw flag
 */
struct dpu_sram_param {
	__u16 total_sram_size;
	__u16 left_sram_size;
	__u16 right_sram_size;
	__u16 sram_base_addr;
	__u16 sram_mem_map;
};

/* dpu drm events */
enum dpu_drm_event_type {
	DRM_PRIVATE_EVENT_TYPE_MIN = 0xE0000000,
	DRM_PRIVATE_EVENT_HISTOGRAM,
	DRM_PRIVATE_EVENT_VCHN0_HISTOGRAM,
	DRM_PRIVATE_EVENT_VSYNC,
	DRM_PRIVATE_EVENT_IDLE,
	DRM_PRIVATE_EVENT_TUI,
	DRM_PRIVATE_EVENT_FORCE_REFRESH,
	DRM_PRIVATE_EVENT_PANEL_DEAD,
	DRM_PRIVATE_EVENT_TYPE_MAX,
};

/**
 * struct dpu_drm_event_req - event request
 * @object_id: drm object that event belongs to, e.g.: CRTC, CONNECTOR
 * @object_type: drm object type
 * @event_type: which event is to be enabled or disabled
 */
struct dpu_drm_event_req {
	__u32 object_id;
	__u32 object_type;
	enum dpu_drm_event_type event_type;
};

/**
 * struct dpu_drm_event_res - event result
 * @base: base of the event result
 *        base.type indicates event in dpu_drm_event_type
 *        base.length = sizeof(dpu_drm_event_res) + payload.length
 *
 * @data: payload of event result
 */
struct dpu_drm_event_res {
	struct drm_event base;
	__u8 data[];
};

/**
 * struct dpu_prepq_writeback - the specially configured values of prepq wb position
 * @prepq_id: prepq id
 * @prepq_dst_width: prepq dst width, be used to configure wb input width
 * @prepq_dst_height: prepq dst height, be used to configure wb input height
 */
struct dpu_prepq_writeback {
	__u8 prepq_id;
	__u16 prepq_dst_width;
	__u16 prepq_dst_height;
};

/**
 * Define extended power modes supported by the DPU connectors.
 */
#define DPU_POWER_MODE_ON              0x00
#define DPU_POWER_MODE_DOZE            0x01
#define DPU_POWER_MODE_DOZE_SUSPEND    0x02
#define DPU_POWER_MODE_OFF             0x03

#define DRM_DPU_REGISTER_EVENT         0x00
#define DRM_DPU_UNREGISTER_EVENT       0x01
#define DRM_DPU_BANDWIDTH              0x02
#define DRM_DPU_CLK_LEVEL              0x03
#define DRM_DPU_DRM_DEBUG_MASK         0x04

enum idle_type {
	CMD_IDLE = 0,
	VIDEO_IDLE,
};

#define DRM_IOCTL_DPU_REGISTER_EVENT   DRM_IOW((DRM_COMMAND_BASE + \
		DRM_DPU_REGISTER_EVENT), struct dpu_drm_event_req)
#define DRM_IOCTL_DPU_UNREGISTER_EVENT DRM_IOW((DRM_COMMAND_BASE + \
		DRM_DPU_UNREGISTER_EVENT), struct dpu_drm_event_req)

#define DRM_IOCTL_DPU_BANDWIDTH DRM_IOW((DRM_COMMAND_BASE + \
		DRM_DPU_BANDWIDTH), int)

#define DRM_IOCTL_DPU_CLK_LEVEL DRM_IOW((DRM_COMMAND_BASE + \
		DRM_DPU_CLK_LEVEL), int)

#define DRM_IOCTL_DPU_DRM_DEBUG_MASK DRM_IOW((DRM_COMMAND_BASE + \
		DRM_DPU_DRM_DEBUG_MASK), int)

#endif /* __XRING_DPU_DRM_H__ */
