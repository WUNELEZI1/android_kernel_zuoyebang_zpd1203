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

#ifndef TIMELINE_LISTENER_H
#define TIMELINE_LISTENER_H

#include <linux/slab.h>
#include <linux/types.h>

struct timeline_listener_ops;

/* each timeline maybe have a lot of listener_server */
struct timeline_listener {
	struct list_head list_node;

	u64 pt_value;
	u64 notify_timestamp;
	void *listener_data;
	const struct timeline_listener_ops *ops;
};

struct timeline_listener_ops {
	/**
	 * @brief get listener name: MUST implement the interface
	 *
	 */
	const char* (*get_listener_name)(struct timeline_listener *listener);
	/**
	 * @brief The signal function: MUST implement the interface
	 *
	 */
	bool (*is_signaled)(struct timeline_listener *listener, u64 tl_val);
	s32 (*handle_signal)(struct timeline_listener *listener);
	void (*release)(struct timeline_listener *listener);
};

#endif /* DKMD_DISP_TIMELINE_LISTENER_H */
