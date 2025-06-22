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

#ifndef _VEU_CMDLIST_NODE_H_
#define _VEU_CMDLIST_NODE_H_

#include <linux/fs.h>
#include "veu_cmdlist_common.h"

#define CMDLIST_RDMA_CFG_RDY(id) (1 << (id))
#define CMDLIST_PREPQ_CFG_RDY(id) (1 << ((id) + 10))
#define CMDLIST_CMPS_CFG_RDY(id) (1 << ((id) + 20))
#define CMDLIST_WB_CFG_RDY(id) (1 << ((id) + 23))

#define CMDLIST_FRM_ID(x) ((u8)((x) >> 32))
#define MAKE_NODE_ID(x, y) ((s64)(x) << 32 | (y) | 1ll << 48)

enum CMDLIST_WAIT_EVENT {
	WB0_RELOAD = 0x10,
	RDMA_RELOAD = 0x40,
	PREPQ_RELOAD = 0x80,
	WB0_SLICE_DONE = 0x100,
};

enum CMDLIST_NODE_TYPE {
	NORMAL_TYPE = 0,
	LAST_TYPE = 1,
	PENDING_TYPE = 2,
	NOP_TYPE,
};

/* 128bit align */
struct cmdlist_item {
	/* cmdlist to veu config register address */
	union {
		u32 cfg_info;
		struct {
			u32 cfg_addr : 19;
			u32 cfg_strobe : 12;
			u32 last_row : 1;
		};
	};
	/* modules_cfg_data0[31:0] */
	u32 data0 : 32;
	/* modules_cfg_data1[31:0] */
	u32 data1 : 32;
	/* modules_cfg_data2[31:0] */
	u32 data2 : 32;
};

/* 128bit align */
struct cmdlist_header {
	union {
		struct {
			u64 reserve0 : 59;
			u64 sw_event : 4;
			u64 rdma_reload : 1;
			u64 prepq_reload : 1;
			u64 rdma_slice_done : 1;
			u64 reserve1 : 62;
		} rch;
		struct {
			/**
			 * Indicates the starting address of the next node,
			 * and the normal node is valid;
			 */
			u64 next_node_addr : 39;
			u64 reserve0 : 1;
			/* 128bit as the unit */
			u64 node_length : 16;
			/**
			 * 0-nml_node; 1-last_node; 2-pending_node: need sw
			 * clear pending, and config cont_addr
			 */
			u64 node_type : 2;
			/* value = 1: next cmdlist_node_info is security */
			u64 next_node_secu : 1;
			u64 wait_event_l : 5;
			u64 wait_event_h : 59;
			u64 reserve1 : 5;
		};
	};
};

/**
 * cmdlist_node - in order to manage cmdlist node
 * @frame_head: head in frame channel
 * @header_addr: physical addr of node buffer
 * @header: virtual addr of node header
 * @items: virtual addr of node data
 * @id: node id
 * @node_size: the real size of this node
 * @node_length: the actual length of this node
 * @current_index: write pos for items
 * @last_cfg_info: last cfg info of cmdlist_item
 * @cfg_rdy: cmdlist_xxx_cfg_rdy
 */
struct cmdlist_node {
	struct list_head frame_head;

	dma_addr_t header_addr;
	struct cmdlist_header *header;
	struct cmdlist_item *items;

	s64 id;

	u32 node_size;
	u32 node_length;
	u32 current_index;

	union {
		u32 last_cfg_info;
		struct {
			u32 last_addr : 19;
			u32 last_strobe : 12;
			u32 last_row : 1;
		};
	};

	u32 cfg_rdy;
};

/**
 * cmdlist_node_creat - create a node for cmdlist_ch(ch_id)
 * @node_len: length of current cmdlist node
 * @reg_num: reg num for current cmdlist node to hold
 */
void cmdlist_node_prepare(int *node_len, int reg_num);

/**
 * cmdlist_node_creat - create a node for cmdlist_ch(ch_id)
 * @base: cmdlist private data
 * @nod_type: the type of this node
 * @nod_len: the length of this node buffer
 * @blk_id: index of cmdlist node
 * @wait_event: which HW event this node should wait
 */
s64 cmdlist_node_create(struct device *dev, struct cmdlist_base *base, u8 nod_type,
		u16 nod_len, s64 blk_id, u64 wait_event);

/**
 * cmdlist_node_del - delete a node for cmdlist_ch(id)
 * @dev: device for veu_dev
 * @base: cmdlist private data
 * @id: index of cmdlist_frame
 */
int cmdlist_node_delete(struct device *dev, struct cmdlist_base *base, s64 id);

/**
 * get_cmdlist_node - check and get node by node_id
 * @base: cmdlist private data
 * @id: id of a node
 */
struct cmdlist_node *get_cmdlist_node(struct cmdlist_base *base, s64 id);

/**
 * cmdlist_node_config - fill node buffer with reg_addr and reg_value
 * @base: cmdlist private data
 * @id: id of a node
 * @reg_addr: reg offset on veu_base
 * @value: value of reg
 */
int cmdlist_node_config(struct cmdlist_base *base, s64 id, u32 reg_addr, u32 value);

/**
 * cmdlist_cfg_rdy_config - set cmdlist_xxx_cfg_rdy of this node
 * @base: base of cmdlist, contain addr and frame info
 * @cfg_rdy: cmdlist_xxx_cfg_rdy
 */
int cmdlist_cfg_rdy_config(struct cmdlist_base *base, uint32_t cfg_rdy);

#endif /* _VEU_CMDLIST_NODE_H_ */
