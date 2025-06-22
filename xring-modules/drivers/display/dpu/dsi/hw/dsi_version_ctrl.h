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

#ifndef _DSI_VERSION_CTRL_H_
#define _DSI_VERSION_CTRL_H_

#include "dsi_ctrl_hw.h"
#include "dsi_phy_hw.h"
#include "dsi_tmg_hw.h"

/* regist dsi ctrl hw call back functions */
int dsi_ctrl_version_ctrl(struct dsi_ctrl_hw *ctrl);
int dsi_phy_version_ctrl(struct dsi_phy_hw *phy);
int dsi_tmg_version_ctrl(struct dsi_tmg_hw *tmg);

#endif
