// SPDX-License-Identifier: GPL-2.0-only
/*
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

#include "hw_reg/veu_hw_cmdlist_reg.h"
#include "veu_cmdlist_hw_ops.h"
#include "../../veu_defs.h"

void cmdlist_frame_first_node_addr_setup(void __iomem *base, u8 ch_id, u64 phy_addr)
{
	u32 addr;

	addr = (u32)phy_addr & ~0xf;
	WRITE_REG(base + CMDLIST_CH_START_ADDRL_OFFSET + (ch_id << 2), addr);

	addr = (u32)(phy_addr >> 32);
	WRITE_REG(base + CMDLIST_CH_START_ADDRH_OFFSET + (ch_id << 2), addr);

	VEU_DBG("ch: %d 0x%llx", ch_id, phy_addr);
}

void cmdlist_config_enable(void __iomem *base, u8 ch_id, bool enable)
{
	WRITE_REG(base + (ch_id << 2), enable ? 3 : 0);

	VEU_DBG("ch: %d, enable: %d", ch_id, enable);
}
