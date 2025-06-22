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

#ifndef _VEU_CMDLIST_COMMON_H_
#define _VEU_CMDLIST_COMMON_H_

#include <linux/types.h>
#include <linux/genalloc.h>
#include <linux/spinlock.h>

/**
 * cmdlist_frame - in order to manage cmdlist node
 * @list: connect each node for current frame
 * @lock: protect list
 * @frm_id: frame_id
 */
struct cmdlist_frame {
	struct list_head list;

	rwlock_t lock;
	u32 frm_id;
};

/**
 * cmdlist_base - the must args for cmdlist module
 * @veu_base: veu base virtual addr
 * @veu_cmdlist_base: veu cmdlist base virtual addr
 * @ctl_top_offset: ctl_top offset of veu base
 * @mem_align: align size when node pool alloc
 * @frame: frame for organizing all cmdlist node
 */
struct cmdlist_base {
	void __iomem *veu_base;
	void __iomem *veu_cmdlist_base;

	u32 ctl_top_offset;
	u32 mem_align;

	struct cmdlist_frame frame;
};

#endif /* _VEU_CMDLIST_COMMON_H_ */
