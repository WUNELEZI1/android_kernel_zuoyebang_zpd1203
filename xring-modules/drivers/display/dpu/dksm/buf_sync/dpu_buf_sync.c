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
#include "dpu_buf_sync.h"
#include "dpu_cmdlist_frame_mgr.h"

/* release lock_info listener begin */
static const char *dpu_frame_lock_info_get_name(struct timeline_listener *listener)
{
	struct dpu_frame_lock_info *lock_info =
			(struct dpu_frame_lock_info *)listener->listener_data;

	return lock_info->name;
}

static bool dpu_frame_lock_info_is_signaled(struct timeline_listener *listener, u64 tl_val)
{
	bool ret = tl_val > listener->pt_value;

	if (ret)
		IOMMU_DEBUG("signal=%d tl_val=%llu, listener=0x%pK, listener->pt_value=%llu",
				ret, tl_val, listener, listener->pt_value);

	return ret;
}

static int dpu_frame_lock_info_handle_signal(struct timeline_listener *listener)
{
	struct dpu_buf_sync *node = NULL;
	struct dpu_buf_sync *_node_ = NULL;
	struct dpu_frame_lock_info *frame_lock_info =
			(struct dpu_frame_lock_info *)listener->listener_data;

	if (frame_lock_info->cmdlist_frame_id != 0) {
		cmdlist_frame_destroy(frame_lock_info->cmdlist_frame_id);
		IOMMU_DEBUG("unlock cmdlist_frame_id: 0x%llx ", frame_lock_info->cmdlist_frame_id);
		frame_lock_info->cmdlist_frame_id = 0;
	}

	if (frame_lock_info->state) {
		drm_atomic_state_put(frame_lock_info->state);
		IOMMU_DEBUG("unlock state: %p ", frame_lock_info->state);
		frame_lock_info->state = NULL;
	}

	list_for_each_entry_safe(node, _node_, &(frame_lock_info->layerbuf_list), list_node) {
		list_del(&node->list_node);
		IOMMU_DEBUG("buffer unlock dmabuf: 0x%llx, frame_no: %llu, rch_id: %u\n",
				(u64)(uintptr_t)node->dmabuf,
				node->frame_no, node->rch_id);
		if (node->dmabuf) {
			dma_buf_put(node->dmabuf);
			node->dmabuf = NULL;
		}
		kfree(node);
	}

	return 0;
}

static void dpu_frame_lock_info_handle_release(struct timeline_listener *listener)
{
	kfree(listener->listener_data);
	listener->listener_data = NULL;
}

static struct timeline_listener_ops g_frame_lock_listener_ops = {
	.get_listener_name = dpu_frame_lock_info_get_name,
	.is_signaled = dpu_frame_lock_info_is_signaled,
	.handle_signal = dpu_frame_lock_info_handle_signal,
	.release = dpu_frame_lock_info_handle_release,
};
/* release lock_info listener end */

struct dpu_frame_lock_info *dpu_buf_sync_create(struct dpu_timeline *timeline,
		struct list_head *buffer_lock, u64 sync_pt, u64 frame_no)
{
	struct dpu_frame_lock_info *lock_info = NULL;
	struct timeline_listener *listener = NULL;
	struct dpu_buf_sync *node = NULL;
	struct dpu_buf_sync *_node_ = NULL;

	if (!timeline) {
		DPU_ERROR("timeline input error is null");
		return NULL;
	}

	lock_info = kzalloc(sizeof(*lock_info), GFP_KERNEL);
	if (!lock_info) {
		DPU_ERROR("kzalloc lock_info fail");
		return NULL;
	}

	INIT_LIST_HEAD(&(lock_info->layerbuf_list));
	spin_lock_init(&(lock_info->layerbuf_spinlock));
	(void)snprintf(lock_info->name, SYNC_NAME_SIZE, "buf_sync_frame_no%llu", frame_no);

	listener = dpu_timeline_alloc_listener(&g_frame_lock_listener_ops, lock_info, sync_pt);
	if (!listener) {
		DPU_ERROR("alloc lock_info listener fail, sync_pt=%llu", sync_pt);
		kfree(lock_info);
		return NULL;
	}
	dpu_timeline_add_listener(timeline, listener);

	if (buffer_lock) {
		list_for_each_entry_safe(node, _node_, buffer_lock, list_node) {
			list_del(&node->list_node);
			list_add_tail(&node->list_node, &(lock_info->layerbuf_list));
		}
	}

	return lock_info;
}
