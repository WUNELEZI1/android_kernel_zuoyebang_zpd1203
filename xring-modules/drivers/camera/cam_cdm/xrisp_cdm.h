/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_CDM_H_
#define _XRISP_CDM_H_

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>
#include "cam_fence/xrisp_fence.h"
#include "cam_sysfs/xrisp_sysfs.h"

#define XRING_EVENT_QUEUE_LEN 250
struct xrisp_cdm_data {
	struct device *dev;
	struct v4l2_device *v4l2_dev;
	struct video_device *vdev;
	struct v4l2_fh *eventq;
	atomic_t dev_opened;
	struct mutex dev_lock;
	struct fence_manager fence_dev;
};

//extern void cam_pm_resource_close(void);

#endif
