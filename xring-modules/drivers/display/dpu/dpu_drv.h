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

#ifndef _DPU_DRV_H_
#define _DPU_DRV_H_

#include <drm/drm_device.h>
#include <drm/drm_print.h>
#include <drm/drm_file.h>
#include <display/xring_dpu_drm.h>
#include "dpu_log.h"

struct dpu_kms;

/**
 * dpu_drm_device - the dpu drm device structure
 * @drm_dev: the base drm device
 * @dpu_kms: the dpu kms pointer
 * @event_list: dpu drm event list
 * @event_list_lock: protect event list
 */
struct dpu_drm_device {
	struct drm_device drm_dev;

	struct dpu_kms *dpu_kms;

	struct list_head event_list;
	struct mutex event_list_lock;
};

/**
 * struct dpu_drm_event - dpu drm event data structure
 * @base: drm event base
 * @event: drm event information
 */
struct dpu_drm_pending_event {
	struct drm_pending_event base;
	struct dpu_drm_event_res event;
};

/**
 * struct dpu_drm_event - dpu drm event data structure
 * @file: pointer to drm_file
 * @event: requested drm event information
 * @head: node head in event_list
 */
struct dpu_drm_event {
	struct drm_file *file;
	struct dpu_drm_event_req event;
	struct list_head head;
};

static inline struct dpu_drm_device *to_dpu_drm_dev(
		struct drm_device *drm_dev)
{
	return container_of(drm_dev, struct dpu_drm_device, drm_dev);
}

/**
 * dpu_drm_event_notify - notify userspace with dpu event
 * @dev: pointer to drm device
 * @event: drm event to be notified
 * @data: payload of the event
 */
void dpu_drm_event_notify(struct drm_device *dev,
		struct drm_event *event, u8 *data);

#endif /* _DPU_DRV_H_ */
