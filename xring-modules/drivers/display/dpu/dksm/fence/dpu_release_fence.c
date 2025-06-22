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

#include <linux/file.h>
#include <linux/sync_file.h>
#include <linux/dma-buf.h>
#include "dpu_log.h"
#include "dpu_release_fence.h"
#include "dpu_fence_utils.h"
#include "dpu_cmdlist_frame_mgr.h"
#include "dpu_trace.h"

static const char *dpu_release_fence_get_driver_name(struct dma_fence *fence)
{
	struct dpu_release_fence *dpu_fence = container_of(fence, struct dpu_release_fence, base);

	return dpu_fence->name;
}

static const char *dpu_release_fence_get_timeline_name(struct dma_fence *fence)
{
	struct dpu_timeline *timeline = dpu_release_fence_get_timeline(fence);

	return timeline->name;
}

static bool dpu_release_fence_enable_signaling(struct dma_fence *fence)
{
	return true;
}

static bool dpu_release_fence_is_signaled_gt(struct dma_fence *fence)
{
	struct dpu_timeline *timeline = dpu_release_fence_get_timeline(fence);
	bool ret = ((s32)(timeline->pt_value - fence->seqno)) >= 0;

	if (g_debug_fence_timeline && ret)
		DPU_INFO("fence signal=%d at %s timeline_value=%llu fence->seqno:%llu",
				ret, timeline->name, timeline->pt_value, fence->seqno);

	return ret;
}

static void dpu_release_fence_release(struct dma_fence *fence)
{
	if (g_debug_fence_timeline)
		DPU_INFO("%s release for %s\n",
			fence->ops->get_timeline_name(fence),
			fence->ops->get_driver_name(fence));

	kfree_rcu(fence, rcu);
}

static void dpu_release_fence_value_str(struct dma_fence *fence, char *str, s32 size)
{
	int ret;

	ret = snprintf(str, size, "%llu", fence->seqno);
	if (ret < 0)
		DPU_ERROR("snprintf of fence->seqno failed!\n");
}

static void dpu_release_fence_timeline_value_str(struct dma_fence *fence, char *str, s32 size)
{
	int ret;
	struct dpu_timeline *timeline = dpu_release_fence_get_timeline(fence);

	ret = snprintf(str, size, "%llu", timeline->pt_value);
	if (ret < 0)
		DPU_ERROR("snprintf of timeline->pt_value failed!\n");
}

static struct dma_fence_ops g_dpu_release_fence_ops = {
	.get_driver_name = dpu_release_fence_get_driver_name,
	.get_timeline_name = dpu_release_fence_get_timeline_name,
	.enable_signaling = dpu_release_fence_enable_signaling,
	.signaled = dpu_release_fence_is_signaled_gt,
	.wait = dma_fence_default_wait,
	.release = dpu_release_fence_release,
	.fence_value_str = dpu_release_fence_value_str,
	.timeline_value_str = dpu_release_fence_timeline_value_str,
};

static void dpu_release_fence_init(struct dpu_release_fence *fence,
	struct dpu_timeline *timeline, u64 pt_value, u64 frame_no)
{
	int ret;

	fence->timeline = timeline;

	dma_fence_init(&fence->base, &g_dpu_release_fence_ops,
		 &timeline->value_lock, timeline->context, pt_value);

	ret = snprintf(fence->name, SYNC_NAME_SIZE, "frame%llu_fence_pt%llu",
			frame_no, pt_value);
	if (ret < 0)
		DPU_ERROR("snprintf of dpu_release_fence failed!\n");
}

/* release fence listener begin */
static const char *dpu_release_fence_get_name(struct timeline_listener *listener)
{
	struct dpu_release_fence *fence = (struct dpu_release_fence *)listener->listener_data;

	return dpu_release_fence_get_driver_name(&fence->base);
}

static bool dpu_release_fence_is_signaled(struct timeline_listener *listener, u64 tl_val)
{
	struct dpu_release_fence *dpu_release_fence =
			(struct dpu_release_fence *)listener->listener_data;
	struct dma_fence *fence = &dpu_release_fence->base;
	char buffer[128];

	if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags))
		return true;

	if (fence->ops->signaled && fence->ops->signaled(fence)) {
		dma_fence_signal_timestamp(fence, listener->notify_timestamp);
		snprintf(buffer, sizeof(buffer), "%s is signaled", dpu_release_fence->name);
		trace_dpu_kms_wait_for_fences(buffer);
		return true;
	}

	return false;
}

static void dpu_release_fence_handle_release(struct timeline_listener *listener)
{
	struct dpu_release_fence *dpu_fence = (struct dpu_release_fence *)listener->listener_data;

	dma_fence_put(&dpu_fence->base);
}

static struct timeline_listener_ops g_dpu_release_fence_listener_ops = {
	.get_listener_name = dpu_release_fence_get_name,
	.is_signaled = dpu_release_fence_is_signaled,
	.handle_signal = NULL,
	.release = dpu_release_fence_handle_release,
};
/* release fence listener end */

s32 dpu_release_fence_create(struct dpu_timeline *timeline, u64 *sync_pt, u64 frame_no)
{
	struct dpu_release_fence *fence = NULL;
	struct timeline_listener *listener = NULL;
	u64 next_value;
	s32 fd;

	if (!timeline || !sync_pt) {
		DPU_ERROR("timeline input error is null");
		return -1;
	}

	fence = kzalloc(sizeof(*fence), GFP_KERNEL);
	if (!fence) {
		DPU_ERROR("kzalloc fence fail");
		return -1;
	}
	next_value = timeline_get_next_value(timeline);
	dpu_release_fence_init(fence, timeline, next_value, frame_no);

	fd = fence_get_fence_fd(&fence->base);
	if (fd < 0) {
		timeline_dec_next_value(timeline);
		kfree(fence);
		return -1;
	}

	listener = dpu_timeline_alloc_listener(&g_dpu_release_fence_listener_ops, fence, next_value);
	if (!listener) {
		DPU_ERROR("alloc fence listener fail, sync_pt=%llu", next_value);
		timeline_dec_next_value(timeline);
		put_unused_fd(fd);
		kfree(fence);
		return -1;
	}
	dpu_timeline_add_listener(timeline, listener);

	if (g_debug_fence_timeline)
		DPU_INFO("fence created at val=%llu of %s timeline_value=%llu, next_value=%u",
			next_value, timeline->name, timeline->pt_value, timeline->next_value);

	*sync_pt = next_value;

	return fd;
}
