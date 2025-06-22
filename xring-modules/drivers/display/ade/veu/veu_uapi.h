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

#ifndef _VEU_HANDLER_H_
#define _VEU_HANDLER_H_

#include "veu_drv.h"

#define VEU_MAGIC 'V'

#define VEU_PROCESS      _IOWR(VEU_MAGIC, 0x1, struct veu_data_copy)
#define VEU_SET_CLK_RATE _IOWR(VEU_MAGIC, 0x2, uint32_t)

#define AFBC_ALIGN_SIZE 64
#define CSC_MATRIX_TABLE_SIZE 12

enum DDR_CHN_ID {
	DPU_CH = 0,
	VEU_CH,
};

enum VEU_IRQ_BIT {
	BIT_CMDLIST_ENTER_PEND = BIT(0),
	BIT_CMDLIST_FRM_DONE = BIT(1),
	BIT_CTL_SECU_CFG_RDADY_CLR = BIT(2),
	BIT_CTL_NML_CFG_READY_CLR = BIT(3),
	BIT_WB_SLICE_DONE = BIT(4),
	BIT_WB_FRM_DONE = BIT(5),
	BIT_PSLLV_ERR_IRQ = BIT(6),
};

// veu ioctl
/**
 * @veu_process - veu offline process
 * @filp: file descriptor provided by user space
 * @arg: data for process
 * @return:0 on success, -1 on failure
 */
int veu_process(struct file *filp, unsigned long arg);

/**
 * @veu_set_core_clk - veu clock rate level process
 * @filp: file descriptor provided by user space
 * @arg: data for process
 * @return:0 on success, -1 on failure
 */
int veu_set_core_clk(struct file *filp, unsigned long arg);

// rdma
int veu_rdma_config(struct veu_data *veu_dev, struct veu_layer *layer);
void veu_rdma_set_reg(struct veu_data *veu_dev);

// wdma
int veu_wdma_config(struct veu_data *veu_dev, struct veu_layer *layer);
void veu_wdma_set_reg(struct veu_data *veu_dev);

// veu ctrl
void veu_hw_init(struct veu_data *veu_dev);
void veu_ctrl_set_reg(struct veu_data *veu_dev);

// veu pipe
void veu_pipe_set_reg(struct veu_data *veu_dev);
int veu_pipe_config(struct veu_data *veu_dev,
	struct veu_layer *inputlayer, struct veu_layer *outputlayer);

// veu 2d scaler
void veu_2dscaler_set_reg(struct veu_data *veu_dev);
#endif /* _VEU_HANDLER_H_ */
