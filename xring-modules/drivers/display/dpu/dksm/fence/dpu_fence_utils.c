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
#include <linux/fs.h>
#include "dpu_log.h"
#include "dpu_fence_utils.h"

s32 fence_get_fence_fd(struct dma_fence *fence)
{
	s32 fd;
	struct sync_file *sync_file = NULL;

	if (unlikely(!fence)) {
		DPU_ERROR("fence is null\n");
		return -1;
	}

	fd = get_unused_fd_flags(O_CLOEXEC);
	if (fd < 0) {
		DPU_ERROR("fail to get unused fd\n");
		return fd;
	}

	sync_file = sync_file_create(fence);
	if (!sync_file) {
		put_unused_fd(fd);
		DPU_ERROR("failed to create sync file\n");
		return -ENOMEM;
	}

	fd_install(fd, sync_file->file);

	return fd;
}

s32 fence_signal_fence_fd(s32 fence_fd)
{
	struct dma_fence *fence = NULL;

	if (fence_fd < 0)
		return 0;

	fence = sync_file_get_fence(fence_fd);
	if (!fence) {
		DPU_ERROR("fence_fd=%d sync_file_get_fence failed!\n", fence_fd);
		return -EINVAL;
	}

	if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags))
		return 0;

	if (dma_fence_signal_timestamp(fence, ktime_get()) != 0)
		DPU_ERROR("fence_fd %d signal timestamp fail", fence_fd);

	dma_fence_put(fence);

	DPU_SYNC_DEBUG("signal fence: 0x%llx", (u64)(intptr_t)fence);

	return 0;
}

