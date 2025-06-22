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
#ifndef __DT_BINDINGS_XR_THERMAL_CDEV_H__
#define __DT_BINDINGS_XR_THERMAL_CDEV_H__

struct xr_gpuhp_cdev_ops {
	void *data;
	int (*update_hotplug_coremask)(void *data, u32 core_num);
};

int xr_gpuhp_cdev_ops_register(struct xr_gpuhp_cdev_ops *ops);
void xr_gpuhp_cdev_ops_unregister(void);
#endif
