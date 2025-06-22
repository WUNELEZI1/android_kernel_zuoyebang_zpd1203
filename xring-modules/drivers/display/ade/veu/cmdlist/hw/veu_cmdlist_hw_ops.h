/* SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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
#include <linux/types.h>
#include <linux/iopoll.h>

#define WRITE_REG(addr, val) writel(val, addr)

#define READ_REG(addr) readl(addr)

/**
 * cmdlist_first_addr_setup - set cmdlist_ch(ch_id) first node addr
 * @base: the mapped address of veu_cmdlist
 * @ch_id: cmdlist channel id, veu always 1
 * @phy_addr: first_node physical addr
 */
void cmdlist_frame_first_node_addr_setup(void __iomem *base, u8 ch_id, u64 phy_addr);

/**
 * cmdlist_config_enable - set cmdlist config enable
 * @base: the mapped address of veu_ctl_top
 * @ch_id: cmdlist channel id, range 0-12
 * @enable: enable cmdlist config
 */
void cmdlist_config_enable(void __iomem *base, u8 ch_id, bool enable);
