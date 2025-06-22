/* SPDX-License-Identifier: GPL-2.0-only */
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

#ifndef _DPU_HW_CMDLIST_OPS_H_
#define _DPU_HW_CMDLIST_OPS_H_

#include <linux/types.h>
#include <linux/iopoll.h>
#include "dpu_cmdlist_common.h"

/**
 * cmdlist_outp32 - write cmdlist reg value
 * @cmdlist: the pointer of struct cmdlist_base
 * @offset: offset of reg
 * @val: the write value
 */
void cmdlist_outp32(struct cmdlist_base *cmdlist, u32 offset, u32 val);

/**
 * cmdlist_inp32 - read cmdlist reg value
 * @offset: offset of reg
 */
u32 cmdlist_inp32(u32 offset);

/**
 * cmdlist_first_addr_setup - set cmdlist_ch(ch_id) first node addr
 * @ch_id: cmdlist channel id, range 0-12
 * @phy_addr: first_node physical addr
 */
void cmdlist_first_addr_setup(u8 ch_id, u64 phy_addr);

/**
 * cmdlist_first_layer_y_setup - set the first layer y pos
 * @ch_id: cmdlist channel id, range 0-12
 * @y: layer y pos
 * @qos: qos value
 */
void cmdlist_first_layer_y_setup(u8 ch_id, u16 y, u32 qos);

/**
 * cmdlist_config_enable - set cmdlist config enable
 * @ch_id: cmdlist channel id, range 0-12
 * @enable: enable cmdlist config
 */
void cmdlist_config_enable(u8 ch_id, bool enable);

/**
 * cmdlist_qos_config - set cmdlist qos
 * @ch_id: cmdlist channel id, range 0-12
 * @qos: qos value
 */
void cmdlist_qos_config(u8 ch_id, u32 qos);
#endif
