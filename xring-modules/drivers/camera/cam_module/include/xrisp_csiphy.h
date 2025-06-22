/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc.
 */

#ifndef XRISP_CSIPHY_H
#define XRISP_CSIPHY_H

#include <linux/device.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include "xrisp_sensor_utils.h"

struct csiphy_dev {
	struct device        *dev;
	struct cam_hw_info   *hw_info;
	struct mutex         mlock;
	atomic_t             power_cnt;
};

#endif /* XRISP_CSIPHY_H */
