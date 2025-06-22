/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DP_HPD_H_
#define _DP_HPD_H_

#include <linux/workqueue.h>
#include <linux/completion.h>

struct dp_hpd_delayed_work {
	struct dp_display *display;
	bool is_hpd_high;

	struct kthread_delayed_work base;
};

#define to_hpd_delayed_work(x) \
container_of(x, struct dp_hpd_delayed_work, base)

/**
 * dp_hpd - dp hpd structure
 * @display: the pointer of dp display
 * @dp_worker: dp hpd worker pointer
 * @hpd_work: dp hpd work
 * @hpd_irq: hpd irq number
 * @off_completed: used to wait dp screen-off frame
 * @first_on: indicate first on after plug-in
 * @last_off: indicate last off before plug-out
 * @lock: to protect hardware config shared with hpd and frame commit
 */
struct dp_hpd {
	struct dp_display *display;

	/**
	 * task, worker and work for hpd uevent report
	 */
	struct task_struct *worker_task;
	struct kthread_worker worker;
	struct dp_hpd_delayed_work delayed_work;

	int hpd_irq;

	struct completion off_completed;
	bool first_on;
	bool last_off;

	struct mutex lock;
};

/**
 * dp_hpd_handle_event - function for handling dp hpd irq
 * @hpd: the pointer of dp hpd
 * @hpd_event: hpd is high or low
 */
void dp_hpd_handle_event(struct dp_hpd *hpd, enum dp_hpd_event hpd_event);

/**
 * dp_hpd_init - init dp hpd module
 * @display: dp display pointer
 * @hpd: the returned dp hpd instance pointer
 *
 * Return: zero on success, -errno on failure
 */
int dp_hpd_init(struct dp_display *display, struct dp_hpd **hpd);

/**
 * dp_hpd_deinit - deinit dp hpd module
 * @hpd: the dp hpd instance pointer
 */
void dp_hpd_deinit(struct dp_hpd *hpd);

#endif /* _DP_HPD_H_ */
