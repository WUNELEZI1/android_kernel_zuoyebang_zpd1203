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

#ifndef _DPU_CMDLIST_COMMON_H_
#define _DPU_CMDLIST_COMMON_H_

#include <linux/types.h>
#include <linux/genalloc.h>
#include <linux/spinlock.h>

#include "dksm_mem_pool.h"

#define MAX_CHANNEL_NODE 128

/**
 * cmdlist_channel - in order to manage cmdlist node
 * @nodes: connect each node
 * @bitw_list: bit write list
 * @lock: protect nodes
 * @ch_id: id(index) of channel
 * @next_node_id: next reserve node id
 */
struct cmdlist_channel {
	void *nodes[MAX_CHANNEL_NODE];
	struct list_head bitw_list;
	spinlock_t lock;
	u8 ch_id;
	u8 next_node_id;
};

/**
 * cmdlist_frame_channel - in order to manage cmdlist frame
 * @list: cmdlist_frame list
 * @lock: protect frames
 * @ch_id: id(index) of frame_channel
 */
struct cmdlist_frame_channel {
	struct list_head list;
	spinlock_t lock;
	u8 ch_id;
};

/**
 * cmdlist_base - the must args for cmdlist module
 * @dpu_base: dpu base virtual addr
 * @dpu_phy_addr: dpu phy addr which used for debug
 * @cmdlist_offset: dpu cmdlist offset of dpu base
 * @channel_count: the num of cmdlist channel
 * @ctl_top_offset: ctl_top offset of dpu base
 * @mem_align: align size when node pool alloc
 * @node_pool: the pointer of dksm_mem_pool struct
 * @frame_channel: in order to manage a frame
 * @cmdlist_channels: to manage per cmdlist channel
 * @initialized: is cmdlist initialized ?
 */
struct cmdlist_base {
	void __iomem *dpu_base;
	u32 dpu_phy_addr;

	u32 cmdlist_offset;
	u32 channel_count;
	u32 ctl_top_offset;

	u32 mem_align;
	struct dksm_mem_pool *node_pool;

	struct cmdlist_frame_channel frame_channel;
	struct cmdlist_channel *cmdlist_channels;

	bool initialized;
};

inline struct cmdlist_base *get_cmdlist_instance(void);

#endif
