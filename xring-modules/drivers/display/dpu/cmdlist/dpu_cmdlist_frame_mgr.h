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

#ifndef _DPU_CMDLIST_FRAME_MGR_H_
#define _DPU_CMDLIST_FRAME_MGR_H_

#include <linux/types.h>
#include <linux/genalloc.h>

#include "dpu_exception.h"

/**
 * cmdlist delete type
 * DELETE_WITH_FRAME: delete cmdlist node with frame
 * USED_BY_FRAME: delete cmdlist node by user
 */
enum delete_type {
	DELETE_WITH_FRAME,
	USED_BY_FRAME,
};

/**
 * cmdlist_frame_create - create a frame for one frame
 */
s64 cmdlist_frame_create(void);

/**
 * cmdlist_frame_node_append - add a node to this frame
 * @frame_id: id of this frame
 * @node_id: id of this node
 * @flag: 0 - destory with frame, (other value) - self-define
 */
int cmdlist_frame_node_append(s64 frame_id, s64 *node_id, u32 flag);

/**
 * cmdlist_frame_commit - commit cfg to HW reg
 * @frame_id: id of this frame
 */
int cmdlist_frame_commit(s64 frame_id);

/**
 * cmdlist_frame_destroy - delete one frame
 * @frame_id: id of this frame
 */
int cmdlist_frame_destroy(s64 frame_id);

/**
 * cmdlist_frame_init - init cmdlist frame
 */
int cmdlist_frame_init(void);

/**
 * cmdlist_frame_deinit - release cmdlist frame
 */
void cmdlist_frame_deinit(void);

#endif
