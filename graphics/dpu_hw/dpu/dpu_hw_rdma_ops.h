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

#ifndef _DPU_HW_RDMA_OPS_H_
#define _DPU_HW_RDMA_OPS_H_

#include "dpu_hw_common.h"
#include "dpu_hw_mixer_ops.h"

#define RDMA_LAYER_PLANAR_NUM_MAX    3

/* the axi port index for rdma connection */
enum rdma_axi_port {
	RDMA_AXI_0 = 0,
	RDMA_AXI_1,
	RDMA_AXI_2,
	RDMA_AXI_MAX,
	RDMA_AXI_DEFAULT = RDMA_AXI_MAX,
};

/* rdma_extender_mode - used to setup rdma horizontal direction extender mode */
enum rdma_extender_mode {
	EXTENDER_MODE_NONE = 0,
	EXTENDER_MODE_V,
	EXTENDER_MODE_LEFT,
	EXTENDER_MODE_RIGHT,
};

/**
 * rdma_layer_stride_type - the read dma stride type for layer
 * @STRIDE_Y_RGB: the first componet-Y and RGB format.
 * @STRIDE_UV: the second componet-UV of YUV_P2 & P3.
 * @STRIDE_MAX: indicate stride sum number.
 */
enum rdma_layer_stride_type {
	STRIDE_Y_RGB = 0,
	STRIDE_UV,
	STRIDE_MAX,
};

enum rdma_layer_plane_type {
	PLANE_Y,
	PLANE_U,
	PLANE_V,
};

struct rch_channel_param {
	u32 roi_enable;
	u32 format_common;
	u32 sram_part_size;
	u32 rch_y_offset_value;
	u32 rch_mode_value;
};

struct rdma_pipe_cfg {
	/**
	 * @is_offline: indicate current rdma work pipeline.
	 */
	bool is_offline;

	/**
	 * @offline_wb_id: indicate which wb is connect when rdma work at offline.
	 */
	enum dpu_wb_id offline_wb_id;

	/**
	 * @axi_port: indicate rdma connect which axi port,
	 *          rdma0~5 has AXI0/1, rdma6~9 only have AXI0 port
	 */
	enum rdma_axi_port axi_port;

	/**
	 * @mixer_id: indicate current rdma connector mixer id.
	 */
	enum mixer_hw_id mixer_id;
};

/* rdma_buffer_level - rdma line buffer level infomation */
struct rdma_line_buf_level {
	u32 rdma_lvl_down0;
	u32 rdma_lvl_up0;
	u32 rdma_lvl_down1;
	u32 rdma_lvl_up1;
	u32 rdma_lvl_down2;
	u32 rdma_lvl_up2;
	u32 rdma_lvl_down3;
	u32 rdma_lvl_up3;
	u32 rdma_urgent_up;
	u32 rdma_urgent_down;
	u32 prefetch_line;
};

/* rdma_qos_level - rdma qos level infomation */
struct rdma_qos_level {
	u32 rdma_arqos_level0;
	u32 rdma_arqos_level1;
	u32 rdma_arqos_level2;
	u32 rdma_arqos_level3;

	u32 obuf_arqos_level0;
	u32 obuf_arqos_level1;
	u32 obuf_arqos_level2;
	u32 obuf_arqos_level3;
};

struct rdma_layer_cfg {
	/**
	 * @extender_mode: indicate rdma H or V extender mode
	 */
	enum rdma_extender_mode extender_mode;

	/**
	 * @is_afbc: indicate current rdma input layer mode.
	 */
	bool is_afbc;

	u64 layer_addrs[RDMA_LAYER_PLANAR_NUM_MAX];

	u16 layer_strides[STRIDE_MAX];

	bool is_fbc_split_mode;
	bool is_fbc_yuv_transform;
	bool is_fbc_32x8;
	bool is_fbc_tile_mode;

	enum protected_status drm_enable_status;

	u16 lbuf_size;
	u16 lbuf_base_addr;
	u16 left_lbuf_size;
	u16 right_lbuf_size;
	bool lbuf_mem_map;

	/**
	 * @crop_area: layer crop area(x1,y1,x2,y2)
	 * @img_size: layer source image size.
	 * @roi_area: layer roi area(x1,y1,x2,y2)
	 */
	struct dpu_rect crop_area;
	struct dpu_rect_v2 img_size;

	/**
	 * @roi0_area: layer roi0 area(x1,y1,x2,y2)
	 * @roi1_area: layer roi1 area(x1,y1,x2,y2)
	 */
	struct dpu_rect roi0_area;
	struct dpu_rect roi1_area;
	bool roi0_en;
	bool roi1_en;

	/* drm fb format */
	u32 drm_format;
	/**
	 * @pixel_format: layer pixel format
	 * @plane_count: layer plane count
	 * @is_yuv: is yuv format or not
	 * @is_narrow_yuv: for yuv format, value range.
	 * @is_uv_swap: swap u panel and v panel, R channel and B channel
	 */
	u8 pixel_format;
	u8 plane_count;
	bool is_yuv;
	bool is_narrow_yuv;
	bool is_uv_swap;

	enum flip_mode layer_flip_mode;
	enum rot_mode layer_rot_mode;

	/**
	 * @layer_mixer_y_offset: indicate layer y offset at mixer
	 */
	u16 layer_mixer_y_offset;

	/**
	 * @slice_line_num: indicate the line number of one slice
	 * @slice_cnt: slice count, support 1 - 4
	 */
	u16 slice_line_num;
	u8 slice_cnt;

	/**
	 * @qos_cfg: rdma and obuf qos level
	 */
	struct rdma_qos_level qos_cfg;
};

/**
 * dpu_hw_rdma_channel_default_refine - refine rdma channel default value.
 * @channel_param: the rdma common param for extender mode
 */
void dpu_hw_rdma_channel_default_refine(struct rch_channel_param *channel_param);

/**
 * dpu_hw_rdma_common_config - update_channel_param_param.
 * @hw: the rdma hardware block pointer
 * @channel_param: the rdma common param for extender mode
 * @layer_cfg: single plane layer param
 * @pipe_cfg: single plane pipe param.
 */
void dpu_hw_rdma_channel_config(struct dpu_hw_blk *hw,
		struct rch_channel_param *channel_param,
		struct rdma_layer_cfg *layer_cfg,
		struct rdma_pipe_cfg *pipe_cfg);

/**
 * dpu_hw_rdma_layer_config - setup rdma_path input layer configure.
 * @hw: the rdma hardware block pointer
 * @channel_param: the rdma common param for extender mode
 * @layer_cfg: rdma_path input layer configure.
 */
void dpu_hw_rdma_layer_config(struct dpu_hw_blk *hw,
		struct rch_channel_param *channel_param,
		struct rdma_layer_cfg *layer_cfg);

/**
 * dpu_hw_rdma_reset - reset the rdma registers
 * @hw: the rdma hardware block pointer
 */
void dpu_hw_rdma_reset(struct dpu_hw_blk *hw);

/**
 * dpu_hw_rdma_path_status_dump - dump rdma_path status for debug
 * @hw: the rdma_path hardware pointer
 */
void dpu_hw_rdma_path_status_dump(struct dpu_hw_blk *hw);

/**
 * dpu_hw_rdma_top_status_dump - dump rdma_top status for debug
 * @hw: the rdma_top hardware pointer
 */
void dpu_hw_rdma_top_status_dump(struct dpu_hw_blk *hw);

/**
 * dpu_hw_rdma_status_clear - clear hw intr status
 * @hw: the rch hardware pointer
 */
void dpu_hw_rdma_status_clear(struct dpu_hw_blk *hw);

#if defined(CONFIG_XRING_DEBUG)
/**
 * dpu_hw_rdma_path_dump_to_file - clear hw intr status
 * @hw: the rch hardware pointer
 * @f: the file pointer
 */
void dpu_hw_rdma_path_dump_to_file(struct dpu_hw_blk *hw, struct file *f);
#elif defined(__KERNEL__)
static inline void dpu_hw_rdma_path_dump_to_file(struct dpu_hw_blk *hw, struct file *f)
{
}
#else
static inline void dpu_hw_rdma_path_dump_to_file(void)
{
}
#endif

/**
 * dpu_hw_rch_rdma_qos_config - config rch rdma qos
 * @hw: the rch hardware pointer
 * @rdma_qos: the rdma qos level structure
 */
void dpu_hw_rch_rdma_qos_config(struct dpu_hw_blk *hw, struct rdma_qos_level rdma_qos);

/**
 * dpu_hw_rch_line_buf_level_config - config rch rdma buffer level
 * @hw: the rch hardware pointer
 * @rdma_lbuf_level: the rdma line buffer level structure
 */
void dpu_hw_rch_line_buf_level_config(struct dpu_hw_blk *hw, struct rdma_line_buf_level rdma_lbuf_level);

/**
 * dpu_hw_rdma_path_dbg_irq_enable - unmask rdma dbg irq
 * @hw: the rch hardware pointer
 * @enable: mask or unmask the irq
 */
void dpu_hw_rdma_path_dbg_irq_enable(struct dpu_hw_blk *hw, bool enable);

/**
 * dpu_hw_rdma_path_dbg_irq_clear - clear rdma dbg irq
 * @hw: the rch hardware pointer
 */
void dpu_hw_rdma_path_dbg_irq_clear(struct dpu_hw_blk *hw);

/**
 * dpu_hw_rdma_path_dbg_irq_dump - dump rdma dbg irq raw status
 * @hw: the rch hardware pointer
 *
 * Returns: the value of rdma dbg irq raw status
 */
u32 dpu_hw_rdma_path_dbg_irq_dump(struct dpu_hw_blk *hw);

/**
 * dpu_hw_rdma_path_dbg_irq_status - dump rdma dbg irq status
 * @hw: the rch hardware pointer
 *
 * Returns: the value of rdma dbg irq status
 */
u32 dpu_hw_rdma_path_dbg_irq_status(struct dpu_hw_blk *hw);

#endif
