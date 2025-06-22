// SPDX-License-Identifier: GPL-2.0
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

#include <linux/fs.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/sync_file.h>
#include "dpu_log.h"
#include "dpu_acquire_fence.h"
#include "dpu_fence_utils.h"
#include "dpu_trace.h"

#define ACQUIRE_SYNC_FENCE_DRIVER_NAME "acquire_sync"

static const char *acquire_sync_fence_get_driver_name(struct dma_fence *fence)
{
	return ACQUIRE_SYNC_FENCE_DRIVER_NAME;
}

static bool acquire_sync_fence_enable_signaling(struct dma_fence *fence)
{
	return true;
}

static void acquire_sync_fence_release(struct dma_fence *fence)
{
	DPU_SYNC_DEBUG("release fence %s seqno:%llu\n",
		fence->ops->get_driver_name(fence),
		fence->seqno);

	kfree_rcu(fence, rcu);
}

static bool acquire_sync_fence_signaled(struct dma_fence *fence)
{
	DPU_SYNC_DEBUG("drv:%s timeline:%s seqno:%llu signaled",
		fence->ops->get_driver_name(fence),
		fence->ops->get_timeline_name(fence),
		fence->seqno);
	return true;
}

static void acquire_sync_fence_value_str(struct dma_fence *fence, char *str, s32 size)
{
	int ret;

	ret = snprintf(str, size, "%llu", fence->seqno);
	if (ret < 0)
		DPU_ERROR("snprintf of fence->seqno failed!\n");
}

static struct dma_fence_ops acquire_sync_ops = {
	.get_driver_name = acquire_sync_fence_get_driver_name,
	.get_timeline_name = acquire_sync_fence_get_driver_name,
	.enable_signaling = acquire_sync_fence_enable_signaling,
	.signaled = acquire_sync_fence_signaled,
	.wait = dma_fence_default_wait,
	.release = acquire_sync_fence_release,
	.fence_value_str = acquire_sync_fence_value_str,
	.timeline_value_str = acquire_sync_fence_value_str,
};

s32 dpu_acquire_fence_create_fd(struct dma_fence *fence, spinlock_t *lock, s32 value)
{
	s32 fence_fd = -1;

	if (!fence)
		return -1;

	dma_fence_init(fence, &acquire_sync_ops, lock, dma_fence_context_alloc(1), value);

	fence_fd = fence_get_fence_fd(fence);
	if (fence_fd < 0) {
		DPU_ERROR("get_unused_fd_flags failed error:%d\n", fence_fd);
		dma_fence_put(fence);
		return -1;
	}

	return fence_fd;
}

struct dma_fence *alloc_dpu_acquire_fence(spinlock_t *lock, s32 value)
{
	struct dma_fence *fence = kzalloc(sizeof(struct dma_fence), GFP_KERNEL);

	if (!fence)
		return NULL;

	dma_fence_init(fence, &acquire_sync_ops, lock, dma_fence_context_alloc(1), value);

	return fence;
}

s32 dpu_acquire_fence_wait(struct dma_fence *fence, long timeout)
{
	s32 rc = 0;
	char buffer[128];

	if (!fence)
		return rc;

	rc = dma_fence_wait_timeout(fence, false, msecs_to_jiffies(timeout));

	snprintf(buffer, sizeof(buffer), "drv:%s timeline:%s seqno:%llu",
			fence->ops->get_driver_name(fence),
			fence->ops->get_timeline_name(fence),
			fence->seqno);

	if (rc > 0) {
		DPU_SYNC_DEBUG("signaled %s\n", buffer);
		rc = 0;
	} else if (rc == 0) {
		DPU_ERROR("timeout %s\n", buffer);
		rc = -ETIMEDOUT;
	}

	trace_dpu_kms_wait_for_fences(buffer);

	return rc;
}

/**
 * @brief This function is used to wait effective fence fd,
 *  such as the GPU graphics buffer
 *
 * @param fence_fd Must be the same thread of fd
 * @param timeout
 * @return s32
 */
s32 dpu_acquire_fence_wait_fd(s32 fence_fd, long timeout)
{
	s32 rc = 0;
	struct dma_fence *fence = NULL;

	if (fence_fd < 0)
		return 0;

	fence = sync_file_get_fence(fence_fd);
	if (!fence) {
		DPU_ERROR("fence_fd=%d sync_file_get_fence failed!\n", fence_fd);
		return -EINVAL;
	}

	rc = dpu_acquire_fence_wait(fence, timeout);
	if (rc != 0)
		DPU_ERROR("wait for fence_fd=%d fail", fence_fd);

	dma_fence_put(fence);
	return rc;
}

void dpu_acquire_fence_signal_release(struct dma_fence *fence)
{
	if (!fence)
		return;

	if (!dma_fence_is_signaled_locked(fence))
		DPU_INFO("no signal drv:%s timeline:%s seqno:%llu, but release!\n",
			fence->ops->get_driver_name(fence),
			fence->ops->get_timeline_name(fence),
			fence->seqno);

	/* ref match fence_create_init */
	dma_fence_put(fence);
}

s32 dpu_acquire_fence_signal(s32 fence_fd)
{
	s32 rc = 0;
	struct dma_fence *fence = NULL;

	if (fence_fd < 0)
		return 0;

	fence = sync_file_get_fence(fence_fd);
	if (!fence) {
		DPU_ERROR("fence_fd=%d, sync_file_get_fence failed!\n", fence_fd);
		return -EINVAL;
	}

	if (dma_fence_is_signaled_locked(fence)) {
		/* ref match fence_create_init */
		dma_fence_put(fence);
	} else {
		DPU_INFO("signal err drv:%s timeline:%s seqno:%llu\n",
			fence->ops->get_driver_name(fence),
			fence->ops->get_timeline_name(fence),
			fence->seqno);
		rc = -1;
	}
	dma_fence_put(fence);

	return rc;
}
