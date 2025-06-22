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

#ifndef _DP_PARSER_H_
#define _DP_PARSER_H_

#include <linux/types.h>
#include <linux/platform_device.h>
#include "dpu_hw_parser.h"

#define BLK_NAME_LEN 32

/**
 * dp_blk_cap - dp block cap structure
 * @name: the block name
 * @padd: the physical address
 * @len: the memory length
 * @vaddr: the virtual address
 */
struct dp_blk_cap {
	char name[BLK_NAME_LEN];
	u32 paddr;
	u32 len;

	void __iomem *vaddr;
};

/**
 * dp_parser - the dp parser structure
 * @index: the dp index
 * @dptx_irq: the dptx irq number
 * @ctrl_cap: the blk cap of dp ctrl
 * @sctrl_cap: the blk cap of dp sctrl
 * @tmg_cap: the blk cap of dp tmg
 * @msgbus_cap: the blk cap of dp message bus
 */
struct dp_parser {
	u32 index;
	int dptx_irq;

	struct dp_blk_cap ctrl_cap;
	struct dp_blk_cap sctrl_cap;
	struct dp_blk_cap tmg_cap;
	struct dp_blk_cap msgbus_cap;
};

/**
 * dp_parser_init - init dp parser
 * @pdev: the platform device pointer
 * @parser: the returned pointer of dp parser
 *
 * Return: zero on success, -errno of failure
 */
int dp_parser_init(struct platform_device *pdev, struct dp_parser **parser);

/**
 * dp_parser_deinit - deinit dp parser
 * @parser: the pointer of dp parser
 */
void dp_parser_deinit(struct dp_parser *parser);

#endif /* _DP_PARSER_H_ */
