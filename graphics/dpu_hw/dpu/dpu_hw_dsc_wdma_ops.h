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

#ifndef _DPU_HW_DSC_WDMA_OPS_H_
#define _DPU_HW_DSC_WDMA_OPS_H_

#include "dpu_hw_common.h"
#include "dpu_hw_format.h"

/* dscw data structure: Every 240bytes are added with 16bytes null data, align to 256 bytes */
#define DSC_WB_ALIGN(x) (ALIGN((x), 240) / 240 * 256 / 8)
#define DSC_WB_HSIZE(width, fmt) ALIGN(DSC_WB_ALIGN((width) * get_bit_per_pixel(fmt)), 16)
#define DSC_WB_BUF_SIZE(width, height, fmt) DSC_WB_HSIZE((width), (fmt)) * (height)

struct dsc_wr_axi_cfg {
	u8 burst_boundary;
	u8 boundary_en;
	u8 burst_len;
	u8 cache;
	u8 region;
	u8 osd;
	u8 nsaid;
	u8 qos;
};

struct dsc_wr_layer_cfg {
	u64 wdma_base_addr;
	u32 width;
	u32 height;
	u32 stride;
	enum pack_data_format format;
};

/**
 * dsc_wdma_layer_config - config layer info for dsc wdma
 *
 * @hw: hw res of dsc wdma
 * @layer_cfg: layer configure info
 */
void dsc_wdma_layer_config(struct dpu_hw_blk *hw, struct dsc_wr_layer_cfg *layer_cfg);

/**
 * dsc_wdma_axi_cfg - config axi info for dsc wdma
 *
 * @hw: hw res of dsc wdma
 * @axi_cfg: axi configure info
 */
void dsc_wdma_axi_cfg(struct dpu_hw_blk *hw, struct dsc_wr_axi_cfg *axi_cfg);

/**
 * dsc_wdma_mem_lp_ctl - control memory lowpower for dsc wdma
 *
 * @hw: hw res of dsc wdma
 * @auto_en: boolen state for enable auto mem lp
 */
void dsc_wdma_mem_lp_ctl(struct dpu_hw_blk *hw, u8 auto_en);

/**
 * dsc_wdma_secure_mode - config secure mode for dsc wdma
 *
 * @hw: hw res of dsc wdma
 * @secure_en: boolen state for secure mode
 */
void dsc_wdma_secure_mode(struct dpu_hw_blk *hw, u8 secure_en);

/**
 * dsc_wdma_qos_config - configure dsc wdma qos value
 *
 * @hw: hw res of dsc wdma
 * @qos: qos value
 */
void dsc_wdma_qos_config(struct dpu_hw_blk *hw, u32 qos);
#endif
