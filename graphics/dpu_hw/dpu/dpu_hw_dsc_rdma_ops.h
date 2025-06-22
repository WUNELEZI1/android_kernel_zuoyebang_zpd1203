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

#ifndef _DPU_HW_DSC_RDMA_OPS_H_
#define _DPU_HW_DSC_RDMA_OPS_H_

#include "dpu_hw_common.h"
#include "dpu_hw_format.h"
struct dsc_rd_axi_cfg {
	u8 burst_split_en;
	u8 burst_split_unit;
	u8 burst_len;
	u8 qos;
};

struct dsc_rd_layer_cfg {
	u64 rdma_base_addr;
	u32 width;
	u32 height;
	u32 stride;
	enum pack_data_format format;
};

/**
 * dsc_rdma_layer_config - config layer info for dsc rdma
 *
 * @hw: hw res of dsc rdma
 * @layer_cfg: layer configure info
 */
void dsc_rdma_layer_config(struct dpu_hw_blk *hw, struct dsc_rd_layer_cfg *layer_cfg);

/**
 * dsc_rdma_axi_cfg - config axi info for dsc rdma
 *
 * @hw: hw res of dsc rdma
 * @axi_cfg: axi configure info
 */
void dsc_rdma_axi_cfg(struct dpu_hw_blk *hw, struct dsc_rd_axi_cfg *axi_cfg);

/**
 * dsc_rdma_secure_mode - config secure mode for dsc rdma
 *
 * @hw: hw res of dsc rdma
 * @secure_en: boolen state for secure mode
 */
void dsc_rdma_secure_mode(struct dpu_hw_blk *hw, u8 secure_en);

/**
 * dump_dsc_rdma_top_status - dump dsc rdma top status when dsc rdma works abnormally
 *
 * @hw: hw res of dsc rdma
 */
void dump_dsc_rdma_top_status(struct dpu_hw_blk *hw);

/**
 * dsc_rdma_qos_config - configure dsc rdma qos value
 *
 * @hw: hw res of dsc rdma
 * @qos: qos value
 */
void dsc_rdma_qos_config(struct dpu_hw_blk *hw, u32 qos);

/**
 * ocm_data_pipeline_ctrl - enable clk for ocm
 *
 * @enable: enable or disable ocm clk
 */
void ocm_data_pipeline_ctrl(bool enable);

#endif
