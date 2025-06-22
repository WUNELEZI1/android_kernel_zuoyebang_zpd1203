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

#ifndef _DPU_MODULE_DRV_H_
#define _DPU_MODULE_DRV_H_

#include <linux/platform_device.h>

int __init dpu_wb_register(void);
void __exit dpu_wb_unregister(void);

int __init dp_drv_register(void);
void __exit dp_drv_unregister(void);

int __init dsi_phy_drv_register(void);
void __exit dsi_phy_drv_unregister(void);

int __init dsi_ctrl_drv_register(void);
void __exit dsi_ctrl_drv_unregister(void);

int __init dsi_display_drv_register(void);
void __exit dsi_display_drv_unregister(void);

#endif /* _DPU_MODULE_DRV_H_ */
