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

#ifndef _VEU_CMDLIST_FRAME_MGR_H_
#define _VEU_CMDLIST_FRAME_MGR_H_

#include "veu_cmdlist_common.h"

/**
 * cmdlist_frame_create - create a frame for one frame
 * @base: cmdlist private data
 */
int cmdlist_frame_create(struct cmdlist_base *base);

/**
 * frame_commit - commit cfg to HW reg
 * @base: cmdlist private data
 */
int cmdlist_frame_commit(struct cmdlist_base *base);

/**
 * cmdlist_frame_init - init cmdlist frame
 * @base: cmdlist private data
 */
int cmdlist_frame_init(struct cmdlist_base *base);

/**
 * cmdlist_frame_deinit - release cmdlist frame
 * @base: cmdlist private data
 */
void cmdlist_frame_deinit(struct cmdlist_base *base);

/**
 * frame_destory - delete one frame
 * @dev: dev for veu_dev
 * @base: cmdlist private data
 */
int cmdlist_frame_destroy(struct device *dev, struct cmdlist_base *base);

#endif
