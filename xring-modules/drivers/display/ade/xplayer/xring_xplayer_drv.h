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

#ifndef _XRING_XPLAYER_DRV_H_
#define _XRING_XPLAYER_DRV_H_

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#include "xring_xplayer_utils.h"

#define XPLAYER_DEV_NAME "xplayer"

#define xplayer_check_and_return(condition, ret, msg, ...) \
	do { \
		if (condition) { \
			pr_err(msg, ##__VA_ARGS__);\
			return ret; \
		} \
	} while (0)

#define xplayer_check_and_void_return(condition, msg, ...) \
	do { \
		if (condition) { \
			pr_err(msg, ##__VA_ARGS__);\
			return; \
		} \
	} while (0)

typedef void (*xplayer_notify_event_func)(enum xplayer_event);

/**
 * @xplayer_data - core data for xplayer driver
 * @pdev: platform device for xplayer driver
 * @unique_opened: only one user
 * @released: released
 * @flags: flags for xplayer
 * @chr_major: device num for xplayer device
 * @chr_class: class xplayer device belong to
 * @chr_dev: character device for xplayer driver
 * @wq: workqueue for xplayer work
 * @info_work: work for xplayer service
 * @sink_work: work for xplayer sink
 */
struct xplayer_data {
	struct platform_device *pdev;

	atomic_t unique_opened;
	atomic_t released;
	uint32_t flags;

	int chr_major;
	struct class *chr_class;
	struct device *chr_dev;

	struct workqueue_struct *wq;
	struct xplayer_info_work info_work;
	struct xplayer_sink_work sink_work;
};

/**
 * @xplayer_drv_register - xplayer driver register
 * @return:0 on success, other on failure
 */
int xplayer_drv_register(void);

/**
 * @xplayer_drv_unregister - xplayer driver unregister
 */
void xplayer_drv_unregister(void);

/**
 * @xplayer_get_device - get xplayer device
 */
struct platform_device *xplayer_get_device(void);

extern int dpu_wfd_sink_init(void);
extern int dpu_wfd_sink_deinit(void);
extern void dpu_wfd_sink_set_sg_table(struct dma_buf *buf, struct sg_table *sgt,
		int tlb_index, struct xplayer_iommu_format_info *iommu_info);
extern void dpu_wfd_event_register(xplayer_notify_event_func func);
extern void dpu_wfd_event_unregister(void);
extern void dpu_wfd_sink_set_present_index(u8 slice_index);
extern void dpu_wfd_sink_get_cmdlist_addr(dma_addr_t phy_addr[]);
extern void dpu_wfd_sink_get_cmdlist_config(uint32_t phy_addr[], int *tlb_index);

#endif /* _XRING_XPLAYER_DRV_H_ */
