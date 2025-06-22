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

#ifndef _DPU_HW_POST_PIPE_TOP_H_
#define _DPU_HW_POST_PIPE_TOP_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_hw_post_pipe_top_ops.h"

/**
 * dpu_hw_post_pipe_top_ops - the virtual function table of hw_post_pipe_top class
 */
struct dpu_hw_post_pipe_top_ops {
	/**
	 * port_config - post pipe output port config
	 * @hw: the post_pipe_top hardware pointer
	 * @cfg: config info
	 */
	void (*port_config)(struct dpu_hw_blk *hw, struct post_pipe_cfg *cfg);

	/**
	 * rc_enable - RC module enable or disable
	 * @hw: the post_pipe_top hardware pointer
	 * @width: panel witdth
	 * @height: panel height
	 * @enable: enable or disable
	 */
	void (*rc_enable)(struct dpu_hw_blk *hw,
			u16 width, u16 height, bool enable);

	/**
	 * rc_config - RC module config
	 * @hw: the post_pipe_top hardware pointer
	 * @cfg: config info
	 */
	void (*rc_config)(struct dpu_hw_blk *hw, void *cfg);

	/**
	 * rc_stream_config - RC stream config
	 * @hw: the post_pipe_top hardware pointer
	 * @cfg: config info
	 */
	void (*rc_stream_config)(struct dpu_hw_blk *hw, void *cfg);

	/**
	 * status_dump - dump hw status for debug
	 * @hw: the post_pipe_top hardware pointer
	 */
	void (*status_dump)(struct dpu_hw_blk *hw);

	/**
	 * status_clear - clear post_pipe_top raw status
	 * @hw: the post_pipe_top hardware pointer
	 */
	void (*status_clear)(struct dpu_hw_blk *hw);

	/**
	 * crc_dump - dump postpipe crc register
	 * @hw: the post_pipe_top hardware pointer
	 * @dsc_crc_selected: if true to choose dsc crc, false to choose postpipe crc
	 *
	 * @return: crc value on success, -1 on failure
	 */
	int32_t (*crc_dump)(struct dpu_hw_blk *hw, bool dsc_crc_selected);
};

/**
 * dpu_hw_post_pipe_top - post_pipe_top hardware object
 * @hw: the post_pipe_top hardware info
 * @blk_cap: the post_pipe_top hardware resource and capability
 * @ops: the post_pipe_top function table
 */
struct dpu_hw_post_pipe_top {
	struct dpu_hw_blk hw;
	const struct dpu_post_pipe_top_cap *blk_cap;

	struct dpu_hw_post_pipe_top_ops ops;
};

static inline struct dpu_hw_post_pipe_top *to_dpu_hw_post_pipe_top(struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_post_pipe_top, hw);
}

/**
 * dpu_hw_post_pipe_top_init - init one post_pipe_top hw block with its capability
 * @post_pipe_top_cap: the post_pipe_top capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_post_pipe_top_init(struct dpu_post_pipe_top_cap *post_pipe_top_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_post_pipe_top_deinit - deinit one post_pipe_top hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_post_pipe_top_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_POST_PIPE_TOP_H_ */
