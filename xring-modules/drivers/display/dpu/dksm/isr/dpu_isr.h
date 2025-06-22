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

#ifndef DPU_ISR_H
#define DPU_ISR_H

#include <linux/types.h>
#include <linux/notifier.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/atomic.h>

#define ISR_NAME_SIZE 256

#define DPU_SYNC_FRAME_NUM (4)

enum {
	INTS_FRM_TIMING_EOF = 0,
	INTS_FRM_TIMING_VSYNC,
	INTS_CFG_RDY_CLR_DONE,
	INTS_FRM_TIMING_UNDERFLOW,
	INTS_MAX,
};

struct listener_data {
	void *data;
	u64 notify_timestamp;
};

struct dpu_isr_listener_node {
	struct list_head list_node;

	struct listener_data listener_data;
	u32 listen_id;

	struct raw_notifier_head irq_nofitier;
};

struct dpu_isr {
	s32 irq_no;
	char irq_name[ISR_NAME_SIZE];
	void *parent;
	u32 unmask;
	atomic_t refcount;
	struct list_head list_node;

	void (*handle_func)(struct dpu_isr *isr_ctrl, u32 handle_event);
	irqreturn_t (*isr_fnc)(s32 irq, void *ptr);

	struct list_head isr_listener_list[INTS_MAX];
};

void dpu_isr_setup(struct dpu_isr *isr_ctrl);

s32 dpu_isr_notify_listener(struct dpu_isr *isr_ctrl, u32 listen_id, u64 timestamp);

s32 dpu_isr_unregister_listener(struct dpu_isr *isr_ctrl,
		struct notifier_block *nb, u32 listen_id);

s32 dpu_isr_register_listener(struct dpu_isr *isr_ctrl,
		struct notifier_block *nb, u32 listen_id, void *listener_data);

#endif
