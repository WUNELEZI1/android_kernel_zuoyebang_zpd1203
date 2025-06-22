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

#ifndef ACQUIRE_FENCE_H
#define ACQUIRE_FENCE_H

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/dma-fence.h>

#define ACQUIRE_FENCE_TIMEOUT_MSEC (4000)

/**
 * @brief Get a file descriptor with given fence on given value.
 *
 * @fence: [in] the fence to initialize
 * @lock:  [in] the irqsafe spinlock to use for locking this fence
 * @return: 0 for success, otherwise, fail
 */
s32 dpu_acquire_fence_create_fd(struct dma_fence *fence, spinlock_t *lock, s32 value);

/**
 * @brief create and init a fence file descriptor
 *
 * @param lock the irqsafe spinlock to use for locking this fence
 * @param value same as scene id
 * @return struct dma_fence* fence
 */
struct dma_fence *alloc_dpu_acquire_fence(spinlock_t *lock, s32 value);

/**
 * @brief sleep until the fence is signaled or timeout.
 *
 * @fence: fence struct
 * @timeout:  [in] expire time for fence wait in ms, or MAX_SCHEDULE_TIMEOUT
 * @return: 0 for success, otherwise, fail
 */
s32 dpu_acquire_fence_wait(struct dma_fence *fence, long timeout);

/**
 * @brief sleep until the fence is signaled or timeout.
 *
 * @fence_fd: [in] the fence fd to wait
 * @timeout:  [in] expire time for fence wait in ms, or MAX_SCHEDULE_TIMEOUT
 * @return: 0 for success, otherwise, fail
 */
s32 dpu_acquire_fence_wait_fd(s32 fence_fd, long timeout);

/**
 * @brief signal and release fence by struction
 *
 * @fence: fence struct
 */
void dpu_acquire_fence_signal_release(struct dma_fence *fence);

/**
 * @brief signal completion of a fence
 *
 * @fence_fd: [in] the fence to signal, fd must be the same with create pid
 * @return:   0 for success, otherwise, fail
 */
s32 dpu_acquire_fence_signal(s32 fence_fd);

#endif
