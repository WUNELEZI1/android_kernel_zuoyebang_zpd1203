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

#ifndef _VEU_STRIPE_H_
#define _VEU_STRIPE_H_

#include "veu_drv.h"

/**
 * @veu_stripe_config - veu stripe param config
 * @veu_dev: core data for veu driver
 * @return:0 on success, -1 on failure
 */
int veu_stripe_config(struct veu_data *veu_dev);
#endif
