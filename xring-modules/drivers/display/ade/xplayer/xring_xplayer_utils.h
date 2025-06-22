/* SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _XRING_XPLAYER_UTILS_H
#define _XRING_XPLAYER_UTILS_H

#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <display/xring_xplayer_uapi.h>

extern int g_xplayer_msg_level;

#define XRING_XPLAYER_ERR(msg, ...) \
	do { if (g_xplayer_msg_level >= 0) \
		pr_err("[XPLAYER E]:%s %d: "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define XRING_XPLAYER_WARN(msg, ...) \
	do { if (g_xplayer_msg_level >= 1) \
		pr_warn("[XPLAYER W]:%s %d: "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define XRING_XPLAYER_INFO(msg, ...) \
	do { if (g_xplayer_msg_level >= 2) \
		pr_info("[XPLAYER I]:%s %d: "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define XRING_XPLAYER_DBG(msg, ...) \
	do { if (g_xplayer_msg_level >= 3) \
		pr_info("[XPLAYER D]:%s %d: "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

struct xplayer_source_data {
	struct xplayer_info info;
	struct dma_buf *buf;
};

struct xplayer_info_work {
	struct kfifo data;
	spinlock_t lock;
	wait_queue_head_t wq;
};

struct xplayer_sink_data {
	struct dma_buf *buf;
	int tlb_index;
	struct xplayer_iommu_format_info iommu_info;
};

struct xplayer_sink_work {
	struct xplayer_sink_data data;
	struct work_struct work;
};

void xplayer_notify_event(enum xplayer_event ev);

#endif /* _XRING_XPLAYER_UTILS_H */
