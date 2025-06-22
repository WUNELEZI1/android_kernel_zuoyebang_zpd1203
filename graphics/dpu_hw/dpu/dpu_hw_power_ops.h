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

#ifndef _DPU_HW_POWER_OPS_H_
#define _DPU_HW_POWER_OPS_H_

#include "dpu_osal.h"
#include "dpu_hw_init_module_ops.h"

enum dpu_partition_type {
	DPU_PARTITION_0,
	DPU_PARTITION_1,
	DPU_PARTITION_2,
	DPU_PARTITION_MAX_NUM,
};

#define DPU_ALL_PARTITIONS  (BIT(DPU_PARTITION_MAX_NUM) - 1)

enum dsi_ipi_clk_mode {
	DSI_IPI_MAIN_PORT0,
	DSI_IPI_MAIN_PORT1,
	DSI_IPI_MAIN_DUAL_PORT,
	DSI_IPI_BACKUP_PORT0,
	DSI_IPI_BACKUP_PORT1,
	//DSI_IPI_BACKUP_PORT1_DP_PLL,
	DSI_IPI_BACKUP_DUAL_PORT,
	DSI_IPI_MAX,
};

/**
 * dsi_pixel_clock_mode - dsi panel pixel clock mode: from PHY or from PLL
 * @DSI_PIXEL_CLOCK_DPU_PLL: from DPU_PLL
 * @DSI_PIXEL_CLOCK_PHY_PLL: from DSI PHY
 * @DSI_PIXEL_CLOCK_MAX:  invalid parameter
 */
enum dsi_pixel_clock_mode {
	DSI_PIXEL_CLOCK_PHY_PLL = 0,
	DSI_PIXEL_CLOCK_DPU_PLL = 1,
	DSI_PIXEL_CLOCK_MAX,
};

#define DPU_PLL_MIN  (24000000UL)
#define DPU_PLL_MAX  (200000000UL)

/**
 * dpu_hw_power_up - dpu power up ops(include M1/dpu/dsi)
 */
void dpu_hw_power_up(struct dpu_hw_init_cfg *cfg);

/**
 * dpu_hw_power_down - dpu power down ops(include M1/dpu/dsi)
 */
void dpu_hw_power_down(void);

/**
 * dsi_power_up - dsi power up ops
 */
void dsi_hw_power_up(void);

/**
 * dsi_hw_power_down - dsi power down ops
 */
void dsi_hw_power_down(void);

/**
 * dpu_hw_set_media1_clk - set media1 clk
 */
void dpu_hw_set_media1_clk(void);

/**
 * dpu_dsi_clk_ctrl - Control clk used to control ipi clock inside dpu
 *
 * @dsi_idx: physical dsi idx of primary panel
 * @enable: bool
 */
void dpu_dsi_clk_ctrl(u32 dsi_idx, bool enable);

/**
 * dsi_clk_dpu_dsi_ipi_gt_off - dsi ipi clock off
 * @port: select dsi0 or dsi1
 */
void dsi_clk_dpu_dsi_ipi_gt_off(u8 port);

/**
 * dsi_clk_dpu_dsi_ipi_gt_on - dsi ipi clock on
 * @port: select dsi0 or dsi1
 */
void dsi_clk_dpu_dsi_ipi_gt_on(u8 port);

/**
 * dpu_dsi_ipi_gt_on - dpu ipi clock on
 * @port: select dsi0 or dsi1
 */
void dpu_dsi_ipi_gt_on(u8 port);

/**
 * dpu_dsi_ipi_gt_off - dpu ipi clock on
 * @port: select dsi0 or dsi1
 */
void dpu_dsi_ipi_gt_off(u8 port);

/**
 * dsi_hw_set_main_ipi_mux - dsi ipi clk mux setting
 * @mode: select dsi0 or dsi1
 */
void dsi_hw_set_main_ipi_mux(enum dsi_ipi_clk_mode mode);

/**
 * dsi_set_ipi_mux_backup - dsi ipi clk mux setting
 * @mode: select dsi0 or dsi1
 */
void dsi_set_ipi_mux_backup(enum dsi_ipi_clk_mode mode);

/**
 * dsi_hw_set_backup_ipi_gt_div - dsi ipi clk pre gate and div
 * @div:ipi divider
 * @mode: port0 , port1 or dual-port
 */
void dsi_hw_set_backup_ipi_gt_div(u8 div, enum dsi_ipi_clk_mode mode);

#endif
