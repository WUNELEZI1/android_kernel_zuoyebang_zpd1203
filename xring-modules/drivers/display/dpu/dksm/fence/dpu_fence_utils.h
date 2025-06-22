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

#ifndef FENCE_UTILS_H
#define FENCE_UTILS_H

#include <linux/types.h>
#include <linux/dma-fence.h>

s32 fence_get_fence_fd(struct dma_fence *fence);
s32 fence_signal_fence_fd(s32 fence_fd);

#endif
