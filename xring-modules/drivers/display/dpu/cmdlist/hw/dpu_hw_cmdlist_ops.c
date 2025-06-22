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

#include "dpu_hw_cmdlist_ops.h"
#include "dpu_hw_cmdlist_reg.h"
#include "dpu_cmdlist_common.h"
#include "dpu_log.h"

static inline u32 merge_bits(u32 origin_value, u32 val, u32 shift, u32 len)
{
	u32 mask = ((1 << len) - 1) << shift;

	origin_value &= ~mask;
	return origin_value | (mask & (val << shift));
}

void cmdlist_first_addr_setup(u8 ch_id, u64 phy_addr)
{
	struct cmdlist_base *cmdlist;
	u32 addr;

	cmdlist = get_cmdlist_instance();

	addr = (u32)phy_addr & ~0xf;
	cmdlist_outp32(cmdlist, cmdlist->cmdlist_offset +
			CMDLIST_CH_START_ADDRL_OFFSET + (ch_id << 2), addr);

	addr = (u32)(phy_addr >> 32);
	cmdlist_outp32(cmdlist, cmdlist->cmdlist_offset +
			CMDLIST_CH_START_ADDRH_OFFSET + (ch_id << 2), addr);

	DPU_CMDLIST_DEBUG("ch: %d 0x%llx\n", ch_id, phy_addr);
}

void cmdlist_first_layer_y_setup(u8 ch_id, u16 y, u32 qos)
{
	struct cmdlist_base *cmdlist;
	u32 offset;
	u32 val;

	cmdlist = get_cmdlist_instance();
	offset = cmdlist->cmdlist_offset + CMDLIST_CH_ARCACHE_OFFSET + (ch_id << 2);

	val = 0x0;
	val = merge_bits(val, y, CMDLIST_CH_Y_FIRST_SHIFT, CMDLIST_CH_Y_FIRST_LENGTH);
	val = merge_bits(val, qos, CMDLIST_CH_ARQOS_SHIFT, CMDLIST_CH_ARQOS_LENGTH);
	cmdlist_outp32(cmdlist, offset, val);

	DPU_CMDLIST_DEBUG("ch: %d, y: %d\n", ch_id, y);
}

void cmdlist_config_enable(u8 ch_id, bool enable)
{
	struct cmdlist_base *cmdlist;

	cmdlist = get_cmdlist_instance();
	cmdlist_outp32(cmdlist, cmdlist->ctl_top_offset +
			(ch_id << 2), enable ? 3 : 0);

	DPU_CMDLIST_DEBUG("ch: %d, enable: %d\n", ch_id, enable);
}

void cmdlist_outp32(struct cmdlist_base *cmdlist, u32 offset, u32 val)
{
	writel(val, cmdlist->dpu_base + offset);
	DPU_REG_DEBUG("[W] addr: 0x%08x, val: 0x%08x\n",
			cmdlist->dpu_phy_addr + offset, val);
}

u32 cmdlist_inp32(u32 offset)
{
	struct cmdlist_base *cmdlist;
	u32 val;

	cmdlist = get_cmdlist_instance();
	val = readl(cmdlist->dpu_base + offset);

	return val;
}

void cmdlist_qos_config(u8 ch_id, u32 qos)
{
	struct cmdlist_base *cmdlist;
	u32 offset;
	u32 value;

	cmdlist = get_cmdlist_instance();
	offset = cmdlist->cmdlist_offset + CMDLIST_CH_ARCACHE_OFFSET + (ch_id << 2);
	value = 0x0;
	value = merge_bits(value, qos, CMDLIST_CH_ARQOS_SHIFT, CMDLIST_CH_ARQOS_LENGTH);
	cmdlist_outp32(cmdlist, offset, value);
}

void cmdlist_dbg_enable(bool enable)
{
	struct cmdlist_base *cmdlist;
	u32 offset;
	u32 val;

	cmdlist = get_cmdlist_instance();
	offset = cmdlist->cmdlist_offset + CMDLIST_PSLVERR_INT_MSK_OFFSET;
	val = 0x0;
	if (enable)
		val = merge_bits(val, 0x1FFF, CMDLIST_CH_CLR_TIMEOUT_INT_MSK_SHIFT,
				CMDLIST_CH_CLR_TIMEOUT_INT_MSK_LENGTH);
	cmdlist_outp32(cmdlist, offset, val);
}

void cmdlist_dbg_clear(void)
{
	struct cmdlist_base *cmdlist;
	u32 offset;
	u32 val;

	cmdlist = get_cmdlist_instance();
	offset = cmdlist->cmdlist_offset + CMDLIST_PSLVERR_INTS_OFFSET;
	val = 0x0;
	val = merge_bits(val, 0x1FFF, CMDLIST_CH_CLR_TIMEOUT_INTS_SHIFT,
			CMDLIST_CH_CLR_TIMEOUT_INTS_LENGTH);
	cmdlist_outp32(cmdlist, offset, val);
}

u32 cmdlist_dbg_save(void)
{
	struct cmdlist_base *cmdlist;
	u32 offset;
	u32 val;

	cmdlist = get_cmdlist_instance();
	offset = cmdlist->cmdlist_offset + CMDLIST_PSLVERR_INT_RAW_OFFSET;
	val = cmdlist_inp32(offset);

	return val;
}
