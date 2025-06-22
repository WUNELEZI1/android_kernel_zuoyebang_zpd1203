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

#ifndef _DPU_COMP_DEV_H_
#define _DPU_COMP_DEV_H_

struct composer;

/**
 * composer_enable - initialize the front module
 *
 * @comp: composer object
 * @return 0 on success, -1 on failure
 */
int32_t composer_enable(struct composer *comp);
#endif
