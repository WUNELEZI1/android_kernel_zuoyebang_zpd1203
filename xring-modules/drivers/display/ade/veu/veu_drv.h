/* SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _VEU_DRV_H_
#define _VEU_DRV_H_

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/interrupt.h>

#include "display/xring_veu.h"
#include "display/xring_dpu_color.h"
#include "display/xring_stripe_common.h"
#include "cmdlist/veu_cmdlist.h"
#include "veu_res.h"
#include "veu_dbg.h"
#include "veu_dmmu.h"

extern struct device *g_dma_dev;

#define VEU_DEV_NAME "veu"
#define VEU_BASE_NODE_INDEX    0

struct veu_irq {
	const char *irq_name;
	irqreturn_t (*isr_fnc)(int irq, void *ptr);
};

enum AFBC_SUPERBLOCK_LAYOUT {
	SUPERBLOCK_16x16 = 0,
	SUPERBLOCK_16x8,
	SUPERBLOCK_32x8,
	SUPERBLOCK_32x4,
};

enum AFBC_TILE_MODE {
	TILE_LINE = 0,
	TILE_8x8,
};

struct veu_2dscaler_param {
	bool scaler_en;
	struct dpu_2d_scaler_cfg scaler_cfg;
};

struct veu_dither_param {
	bool dither_en;
	struct dpu_dither_cfg dither_cfg;
};

struct veu_task {
	struct veu_layer input_layer;
	struct veu_layer output_layer;
};

struct afbcd_param {
	uint32_t fbc_tile_type;
	uint32_t fbc_superblock_layout;
	uint32_t fbc_yuv_transform;
	uint32_t fbc_split_mode;
};

struct veu_stripe_param {
	struct display_rch_stripe_info layer_stripe_param;
	struct display_wb_stripe_info wb_stripe_param;
	struct veu_2dscaler_param scaler;
	struct veu_dither_param dither;
};

struct veu_stripe {
	int count;
	struct veu_stripe_param stripe_param[VEU_STRIPE_NUM_MAX];
};

struct veu_data_copy {
	struct veu_task veu_layer_task;
	uint64_t veu_stripe_ptr;
};

/**
 * veu_rdma_param - config data for rdma
 * @layer: input layer
 * @lbuf_cllc_rect: layer src rect need align when calc lbuf size
 * @bbox_start_align: crop area of rdma after alignment
 * @bbox_end_align: crop area of rdma after alignment
 * @lbuf_mem_size: share mem size of rdma
 * @afbcd: rdma afbc format decode param
 * @format: format which rdma fetch from ddr
 * @is_afbc_support: whether specific format is afbc_supported by rdma
 * @is_yuv: whether specific format is yuv format
 * @need_uv_swap: NV21&NV21_10b need swap uv plane
 * @plane_count: reserved, currently not used
 * @bpp: reserved, currently not used
 * @phy_addr: input buffer address
 */
struct veu_rdma_param {
	struct veu_layer layer;

	struct veu_rect_ltrb lbuf_calc_rect;
	uint32_t bbox_start_align;
	uint32_t bbox_end_align;
	uint32_t lbuf_mem_size;

	struct afbcd_param afbcd;

	uint32_t format;
	bool is_afbc_support;
	bool is_yuv;
	bool need_uv_swap;
	uint32_t plane_count;
	uint32_t bpp;

	uint64_t phy_addr;
};

struct afbce_param {
	uint32_t fbc_tile_split_en;
	uint32_t fbc_tile_wide;
	uint32_t fbc_tile_hd_mode_on;
	uint32_t fbc_yuv_transform_en;
	uint32_t fbc_default_color_en;
	uint32_t fbc_copy_mode_en;
};

/**
 * veu_wdma_param - config data for wdma
 * @layer: output layer
 * @format: format which wdma fetch from ddr
 * @is_afbc_support: whether specific format is afbc_supported by rdma
 * @is_yuv: whether specific format is yuv format
 * @need_uv_swap: NV21&NV21_10b need swap uv plane
 * @plane_count: reserved, currently not used
 * @bpp: reserved, currently not used
 * @buf_size: size of writeback buffer
 * @phy_addr: writeback buffer address
 * @afbce: writeback afbc format encoder param
 * @afbc_header_size: afbc header buffer size
 * @out_width: writeback output original image width
 * @out_height: writeback output original image height
 * @out_subframe_ltopx: h-start point of the output subimage on original image
 * @out_subframe_ltopy: v-start point of the output subimage on original image
 * @out_subframe_width: output subimage width for stripe
 * @out_subframe_height: output subimage height for stripe
 * @rot_mode: output layer rotation param
 * @flip_mode: output layer flip param
 */
struct veu_wdma_param {
	struct veu_layer layer;

	uint32_t format;
	bool is_afbc_support;
	bool is_yuv;
	bool need_uv_swap;
	uint32_t plane_count;
	uint32_t bpp;

	/* for uft test only */
	uint64_t buf_size;
	uint64_t phy_addr;

	struct afbce_param afbce;
	uint32_t afbc_header_size;

	uint32_t out_width;
	uint32_t out_height;
	uint32_t out_subframe_ltopx;
	uint32_t out_subframe_ltopy;
	uint32_t out_subframe_width;
	uint32_t out_subframe_height;

	uint32_t vpu_crop_width;
	uint32_t vpu_crop_height;
	uint32_t rot_mode;
	uint32_t flip_mode;
};

/**
 * veu_pipe_param - config data for veu pipe
 * @format_in: rdma output format
 * @is_yuv_in: intput layer yuv format flag
 * @format_out: output format after veu pipe csc and dfc
 * @is_yuv_out: output layer yuv format flag
 * @pipe_en: veu pipe module enable mask
 * @scaler_en: veu pipe scaler enable flag
 * @fmt_cvt_alpha: dfc alpha matrix param
 * @is_yuv_10bit: 10bit yuv format flag
 * @dither_cfg: veu pipe dither config param
 * @dither_en: veu pipe dither enable flag
 * @crop1_in: veu pipe first crop module input rect
 * @crop1_out: veu pipe first crop module output rect
 * @crop2_in: veu pipe second crop module input rect
 * @crop2_out: veu pipe second crop module output rect
 */
struct veu_pipe_param {
	uint32_t format_in;
	bool is_yuv_in;
	uint32_t format_out;
	bool is_yuv_out;

	uint32_t pipe_en;
	bool scaler_en;

	uint32_t fmt_cvt_alpha[3];
	bool is_yuv_10bit;
	uint32_t color_space;

	struct dpu_dither_cfg dither_cfg;
	bool dither_en;

	struct veu_rect_ltrb crop1_in;
	struct veu_rect_ltrb crop1_out;

	struct veu_rect_ltrb crop2_in;
	struct veu_rect_ltrb crop2_out;
};

/**
 * @veu_data - core data for veu driver
 * @pdev: platform device for veu driver
 * @of_dev: device for veu driver
 * @chr_major: device num for veu device
 * @chr_class: class veu device belong to
 * @chr_dev: character device for veu driver
 * @device_initialized: prevent repeated drive probe
 * @clk_mgr: manage veu clock object
 * @power_mgr: manage veu regulator supplies
 * @base: base addr after ioremap
 * @addr: addr same as memory map for debug
 * @iomem_len: length of iomem space of veu
 * @rdma_obj: mmu param of rdma
 * @wdma_obj: mmu param of wdma
 * @enable_cmdlist: decide register config use cpu or cmdlist
 * @cmdlist: veu cmdlist device
 * @node_id: array of cmdlist node id
 * @veu_irq: irq for veu, get from device tree
 * @veu_irq_info: irq info for veu
 * @frm_done_wq: wait_queue for frm_done irq, indicate writeback finish
 * @frm_done_flag: flag for frm_done irq, set 1 when frm_done irq comes
 * @cfg_rdy_clr_wq: reserved, currently not used
 * @cfg_rdy_clr_flag: reserved, currently not used
 * @power_count: refcount for veu power status
 * @power_sem: protect for veu power on/off
 * @rdma_param: rdma config data
 * @wdma_param: wdma config data
 * @pipe_param: pq config data
 * @veu_stripe: veu stripe config data
 * @lock: protect multi-call on veu
 */
struct veu_data {
	struct device *of_dev;
	struct platform_device *pdev;

	int chr_major;
	struct class *chr_class;
	struct device *chr_dev;

	bool device_initialized;
	int clk_rate_lock_level;
	int clk_rate_level;

	struct veu_clk_mgr *clk_mgr;
	struct veu_power_mgr *power_mgr;

	void __iomem *base;
	uint32_t addr;
	uint32_t iomem_len;

	struct veu_mmu_object *rdma_obj;
	struct veu_mmu_object *wdma_obj;

	bool enable_cmdlist;
	struct veu_cmdlist *cmdlist;
	long node_id[MAX_STRIPE_COUNT];

	uint32_t veu_irq;
	struct veu_irq veu_irq_info;
	wait_queue_head_t frm_done_wq;
	int frm_done_flag;
	wait_queue_head_t cfg_rdy_clr_wq;
	atomic_t cfg_rdy_clr_flag;

	int power_count;
	bool lock_power;

	int (*on)(struct veu_data *priv);
	int (*off)(struct veu_data *priv);
	int (*set_core_clk_rate)(struct veu_data *veu_dev, uint32_t level);
	void (*set_reg)(struct veu_data *veu_dev, uint32_t node_index,
			uint32_t offset, uint32_t val);

	struct veu_rdma_param rdma_param;
	struct veu_wdma_param wdma_param;
	struct veu_pipe_param pipe_param;

	struct veu_stripe *veu_stripe;

	struct mutex lock;
};

/**
 * @veu_drv_register - veu driver register
 * @return:0 on success, other on failure
 */
int veu_drv_register(void);

/**
 * @veu_drv_unregister - veu driver unregister
 * @return:0 on success, other on failure
 */
void veu_drv_unregister(void);

#endif /* _VEU_DRV_H_ */
