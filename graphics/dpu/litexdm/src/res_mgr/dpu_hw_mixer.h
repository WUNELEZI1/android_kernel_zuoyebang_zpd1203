// SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _DPU_HW_MIXER_H_
#define _DPU_HW_MIXER_H_

#include "osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_cap.h"
#include "dpu_hw_mixer_ops.h"

/**
 * dpu_hw_mixer_ops - the virtual function table of mixer class
 */
struct dpu_hw_mixer_ops {
	/**
	 * enable - enable or disable the mixer module
	 * @hw: the mixer hardware pointer
	 * @enable: true for enabling or false for disabling
	 */
	void (*enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * output_size_config - config the mixer output size
	 * @hw: the mixer hardware pointer
	 * @width: the output width
	 * @height: the output height
	 * @cfg_method: use cmdlist cfg or acpu cfg
	 */
	void (*output_size_config)(struct dpu_hw_blk *hw,
			uint16_t width, uint16_t height, bool cfg_method);

	/**
	 * bg_color_config - config the background color
	 * @hw: the mixer hardware pointer
	 * @bg_color: pointer of background color data
	 * @cfg_method: use cmdlist cfg or acpu cfg
	 */
	void (*bg_color_config)(struct dpu_hw_blk *hw,
			struct mixer_color_cfg *bg_color, bool cfg_method);

	/**
	 * rch_layer_config - config the rch layer data
	 * @hw: the mixer hardware pointer
	 * @rdma_cfg: pointer of rdma layer config
	 */
	void (*rch_layer_config)(struct dpu_hw_blk *hw,
			struct mixer_dma_layer_cfg *rdma_cfg);

	/**
	 * solid_layer_config - config the solid layer data
	 * @hw: the mixer hardware pointer
	 * @solid_cfg: pointer of solid layer config
	 */
	void (*solid_layer_config)(struct dpu_hw_blk *hw,
			struct mixer_solid_layer_cfg *solid_cfg);

	/**
	 * reset - reset the mixer hardware registers
	 * @hw: the mixer hardware pointer
	 * @cfg_method: use cmdlist cfg or acpu cfg
	 */
	void (*reset)(struct dpu_hw_blk *hw, bool cfg_method);
};

/**
 * dpu_hw_mixer - mixer hardware object
 * @hw: the mixer hardware info
 * @blk_cap: the mixer hardware resource and capability
 * @ops: the mixer function table
 */
struct dpu_hw_mixer {
	struct dpu_hw_blk hw;
	const struct dpu_mixer_cap *blk_cap;

	struct dpu_hw_mixer_ops ops;
};

static inline struct dpu_hw_mixer *to_dpu_hw_mixer(struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_mixer, hw);
}

/**
 * dpu_hw_mixer_init - init one mixer hw block with its capability
 * @mixer_cap: the mixer capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_mixer_init(struct dpu_mixer_cap *mixer_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_mixer_deinit - deinit one mixer hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_mixer_deinit(struct dpu_hw_blk *hw);

#endif /* DPU_HW_MIXER_H */
