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

#ifndef _DPU_FENCE_H_
#define _DPU_FENCE_H_

#include <linux/kernel.h>
#include <linux/dma-fence.h>

#define FENCE_NAME_LEN 32

enum dpu_fence_type {
	DPU_FENCE_PRESENT,
	DPU_FENCE_RELEASE,
	DPU_FENCE_START,
	DPU_FENCE_MAX
};

enum dpu_fence_forward_type {
	DPU_FENCE_FORWARD_PREPARE = 1,
	DPU_FENCE_FORWARD_DONE = 2,
};

/**
 * dpu_fence_ctx - the fence info for maintaining the running state
 *                 in the life cycle of a specific fence for one display
 */
struct dpu_fence_ctx {
	/* the basic info of fence_ctx */
	u64 context;
	struct kref refcount;
	enum dpu_fence_type type;
	char name[FENCE_NAME_LEN];

	/* the timeline info for maintaining the running rules */
	u64 commit_seqno;
	u64 signaled_seqno;
	u64 unexpected_count;
	u64 expired_count;

	/* the related member of fence list */
	struct list_head fence_list;
	struct list_head forward_list;
	spinlock_t list_lock;
	spinlock_t lock;

	/* the head for debug purpose */
	struct list_head head;
};

/* dpu_fence - the dpu fence inherit from dma_fence */
struct dpu_fence {
	struct dma_fence base;
	struct dpu_fence_ctx *ctx;
	char name[FENCE_NAME_LEN];
	struct list_head head;
	ktime_t timestamp;
};

static inline struct dpu_fence *to_dpu_fence(struct dma_fence *fence)
{
	return container_of(fence, struct dpu_fence, base);
}

/* The APIs for fence releated operations */
void dpu_fence_forward(struct dpu_fence_ctx *ctx, enum dpu_fence_forward_type type);
void dpu_fence_signal(struct dpu_fence_ctx *ctx, bool force);
int dpu_fence_wait(struct dma_fence *fence, bool intr, uint32_t timeout_ms);
int dpu_fence_create(struct dpu_fence_ctx *ctx, u32 offset);

/* The APIs to create/destroy/dump fence_ctx */
int dpu_fence_ctx_init(struct dpu_fence_ctx **ctx,
		enum dpu_fence_type type, int index);
void dpu_fence_ctx_deinit(struct dpu_fence_ctx *ctx);
void dpu_fence_ctx_state_dump(struct seq_file *s);

#endif /* _DPU_FENCE_H_ */
