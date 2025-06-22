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

#include "veu_cmdlist_common.h"
#include "veu_cmdlist_node.h"
#include "../veu_drv.h"
#include "../veu_defs.h"
#include "../veu_utils.h"

void cmdlist_write(struct veu_data *veu_dev, u32 node_index, u32 reg_offset, u32 val)
{
	veu_check_and_void_return(!veu_dev, "veu_dev is null");

	if (veu_dev->node_id[node_index] < 0) {
		VEU_ERR("invalid node id!");
		return;
	}

	VEU_DBG("veu set reg addr: 0x%x, value: 0x%x",
		veu_dev->addr + reg_offset, val);
	cmdlist_node_config(&veu_dev->cmdlist->base, veu_dev->node_id[node_index], reg_offset, val);
}
