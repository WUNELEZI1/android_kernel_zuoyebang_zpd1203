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

#ifndef _VEU_CMDLIST_H_
#define _VEU_CMDLIST_H_

#include <linux/types.h>
#include <linux/device.h>

#include "veu_cmdlist_common.h"
#include "../veu_drv.h"

struct veu_data;

/**
 * veu_cmdlist_funcs - the virtual function table of cmdlist class
 */
struct cmdlist_funcs {
	/**
	 * prepare - calc total items and create node
	 * @veu_dev: veu device
	 * @blk_id:  blk_id for create cmdlist node
	 * @is_first_block: first block's node should create frame
	 * @is_last_block: last block's node has LAST_TYPE node_type
	 */
	void (*prepare)(struct veu_data *veu_dev, s64 blk_id, bool is_first_blk,
			bool is_last_blk);
	/**
	 * commit - commit cmdlist_node to hardware
	 * @veu_dev: veu device
	 */
	void (*commit)(struct veu_data *veu_dev);
};

/**
 * veu_cmdlist - cmdlist data hold by veu_data
 * @base: the must args for cmdlist module
 * @funcs: callbacks of cmdlist funcs
 */
struct veu_cmdlist {
	struct cmdlist_base base;
	struct cmdlist_funcs *funcs;
};

/**
 * veu_cmdlist_init - init cmdlist module from device tree
 * @veu_dev: veu device
 */
int veu_cmdlist_init(struct veu_data *veu_dev);

/**
 * veu_cmdlist_deinit - deinit cmdlist module
 * @veu_dev: veu device
 */
void veu_cmdlist_deinit(struct veu_data *veu_dev);

#endif /* _VEU_CMDLIST_H_ */
