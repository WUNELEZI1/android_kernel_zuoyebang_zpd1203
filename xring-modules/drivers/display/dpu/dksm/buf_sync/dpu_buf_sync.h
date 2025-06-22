/* SPDX-License-Identifier: GPL-2.0 */
/*
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

#ifndef DPU_BUF_SYNC_H
#define DPU_BUF_SYNC_H

#include <drm/drm_atomic.h>
#include "dpu_timeline.h"

struct dpu_buf_sync {
	struct list_head list_node;

	u64 frame_no;
	s32 sharefd;
	u32 rch_id;
	struct dma_buf *dmabuf;
};

struct dpu_frame_lock_info {
	s64 cmdlist_frame_id;
	struct drm_atomic_state *state;

	spinlock_t layerbuf_spinlock;
	struct list_head layerbuf_list;

	char name[SYNC_NAME_SIZE];
};

struct dpu_frame_lock_info *dpu_buf_sync_create(struct dpu_timeline *timeline,
		struct list_head *buffer_lock, u64 sync_pt, u64 frame_no);

#endif
