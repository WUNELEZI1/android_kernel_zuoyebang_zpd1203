/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#ifndef RELEASE_FENCE_H
#define RELEASE_FENCE_H

#include <linux/dma-fence.h>
#include "dpu_timeline.h"

struct dpu_release_fence {
	struct dma_fence base;
	struct dpu_timeline *timeline;

	char name[SYNC_NAME_SIZE];
};

static inline struct dpu_timeline *dpu_release_fence_get_timeline(struct dma_fence *fence)
{
	struct dpu_release_fence *dpu_fence = container_of(fence, struct dpu_release_fence, base);

	return dpu_fence->timeline;
}

/**
 * @brief
 *  return fence fd, if create fence succ, will return >= 0,
 *  else return < 0.
 */
s32 dpu_release_fence_create(struct dpu_timeline *timeline, u64 *sync_pt, u64 frame_no);

#endif
