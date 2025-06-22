// SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _PANEL_DEV_H_
#define _PANEL_DEV_H_

#define PINCTRL_FUNC0_GPIO 0
#define PINCTRL_FUNC1_TE 1
#define PANEL_IO_DELAY_0MS 0
#define PANEL_IO_DELAY_1MS 1
#define PANEL_IO_DELAY_2MS 2

struct platform_device;
struct panel_dev;

/**
 * prepare_panel_data - prepare panel data
 *
 * @pdev: platform_device
 * @panel_dev: panel dev data
 * @return 0 on success, -1 on failure
 */
int32_t prepare_panel_data(struct platform_device *pdev, struct panel_dev *panel_dev);
void unprepare_panel_data(struct platform_device *pdev);

#endif