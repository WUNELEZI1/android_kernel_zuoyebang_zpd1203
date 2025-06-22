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

#ifndef _DPU_CMDLIST_NODE_H_
#define _DPU_CMDLIST_NODE_H_

#include <linux/fs.h>

enum CMDLIST_WAIT_EVENT {
	RDMA_RELOAD = 0x10,
	PREPQ_RELOAD = 0x20,
	RDMA_SLICE_DONE = 0x40,
	WB0_SLICE_DONE = 0x80,
	WB1_SLICE_DONE = 0x100,
	WB_SLICE_DONE = WB0_SLICE_DONE | WB1_SLICE_DONE,
	WB0_RELOAD = RDMA_RELOAD,
	WB1_RELOAD = PREPQ_RELOAD,
	WB_RELOAD = WB0_RELOAD | WB1_RELOAD,
	CMPS_RELOAD = RDMA_SLICE_DONE,
	TMG_SOF = 0x200,
	TMG_EOF = 0x400,
	CRI_NO_BUSY0 = 0x1000,
	CRI_NO_BUSY1 = 0x2000,
	CRI_NO_BUSY = CRI_NO_BUSY0 | CRI_NO_BUSY1,
	PRI_NO_BUSY0 = 0x4000,
	PRI_NO_BUSY1 = 0x8000,
	PRI_NO_BUSY = PRI_NO_BUSY0 | PRI_NO_BUSY1
};

enum CMDLIST_NODE_TYPE {
	NORMAL_TYPE = 0,
	LAST_TYPE = 1,
	PENDING_TYPE = 2,
	NOP_TYPE = 4,
};

#define NODE_ITEM_ALIGN 2

#define CMDLIST_RDMA_CFG_RDY(id) (1 << (id))
#define CMDLIST_PREPQ_CFG_RDY(id) (1 << ((id) + 10))
#define CMDLIST_CMPS_CFG_RDY(id) (1 << ((id) + 20))
#define CMDLIST_WB_CFG_RDY(id) (1 << ((id) + 23))

#define CMDLIST_NODE_FLAG (0xA501ll << 48)
#define CMDLIST_NODE_USED_FLAG (1ll << 40)

#define CMDLIST_CH_ID(x) ((u8)((x) >> 32))
#define MAKE_NODE_ID(x, y) ((s64)(x) << 32 | (y) | CMDLIST_NODE_FLAG)
#define CMDLIST_INVALID_NODE_ID 0

#define IS_CMDLIST_NODE(id) (((id) & (0xffffll << 48)) == CMDLIST_NODE_FLAG)
#define CMDLIST_NODE_MARK_USED(id) ((id) |= (CMDLIST_NODE_USED_FLAG))
#define CMDLIST_NODE_UNUSED(id) !(id & CMDLIST_NODE_USED_FLAG)

/* 128bit align */
struct cmdlist_item {
	/* cmdlist to dpu config register address */
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
			u64 reserve0 : 59;
			u64 sw_event : 4;
			u64 wb0_reload : 1;
			u64 wb1_reload : 1;
			u64 cmps_reload : 1;
			u64 tmg_cfg_eof : 2;
			u64 tmg_cfg_in_done : 1;
			u64 reserve1 : 59;
		} cmps;

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
 * @header_addr: physical addr of node buffer
 * @header: virtual addr of node header
 * @items: virtual addr of node data
 * @id: node id
 * @node_size: the real size of this node
 * @node_length: the actual length of this node
 * @current_index: write pos for items
 * @last_cfg_info: last cfg info of cmdlist_item
 * @cfg_rdy: cmdlist_xxx_cfg_rdy
 * @layer_y_pos: layer y pos
 * @is_y_pos_set: is user set the layer y pos
 * @is_used: set true if node is used
 */
struct cmdlist_node {
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
	u32 layer_y_pos;
	bool is_y_pos_set;
	bool is_used;
};

/**
 * cmdlist_node_create - create a node for cmdlist_ch(ch_id)
 * @ch_id: index of cmdlist_channel
 * @nod_type: the type of this node
 * @nod_len: the length of this node buffer
 * @wait_event: which HW event this node should wait
 */
s64 cmdlist_node_create(u8 ch_id, u8 nod_type, u16 nod_len, u64 wait_event);

/**
 * cmdlist_node_delete - delete a node for cmdlist_ch(id)
 * @id: index of cmdlist_channel
 */
int cmdlist_node_delete(s64 id);

/**
 * get_cmdlist_node - check and get node by node_id
 * @id: id of a node
 */
struct cmdlist_node *get_cmdlist_node(s64 id);

/**
 * cmdlist_node_config - fill node buffer with reg_addr and reg_value
 * @id: id of a node
 * @reg_addr: reg offset of dpu_base
 * @value: value of reg
 */
int cmdlist_node_config(s64 id, u32 reg_addr, u32 value);

/**
 * cmdlist_node_bit_config - fill node buffer with reg_addr and reg_bit_value
 * @id: id of a node
 * @reg_addr: reg offset of dpu_base
 * @value: bit_value of reg
 * @loc: bit pos of this reg
 * @mask: mask of this bit_value
 * @base: virtual ptr of dpu base
 */
int cmdlist_node_bit_config(s64 id, u32 reg_addr, u32 value, u8 loc, u32 mask,
		void __iomem *base);

/**
 * cmdlist_node_same_config - to config same reg with diff value
 * @id: id of a node
 * @reg_addr: reg offset of dpu_base
 * @value: value of reg
 */
int cmdlist_node_same_config(s64 id, u32 reg_addr, u32 value);

/**
 * cmdlist_cfg_rdy_set - set cmdlist_xxx_cfg_rdy of this node
 * @id: id of a node
 * @cfg_rdy: cmdlist_xxx_cfg_rdy
 */
int cmdlist_cfg_rdy_set(s64 id, u32 cfg_rdy);

/**
 * cmdlist_wait_event_update - update wait_event of this node
 * @id: id of a node
 * @wait_event: which HW event this node should wait
 */
int cmdlist_wait_event_update(s64 id, u64 wait_event);

/**
 * cmdlist_node_type_update - update node type of this node
 * @id: id of a node
 * @node_type: which HW event this node should wait
 */
int cmdlist_node_type_update(s64 id, u8 node_type);

/**
 * cmdlist_cfg_rdy_set - set cmdlist_xxx_cfg_rdy of this node
 * @id: id of a node
 * @y: layer y pos
 */
int cmdlist_layer_y_pos_set(s64 id, u32 y);

/**
 * cmdlist_channel_init - init cmdlist channel
 */
int cmdlist_channel_init(void);

/**
 * cmdlist_channel_deinit - release cmdlist channel
 */
void cmdlist_channel_deinit(void);

#endif
