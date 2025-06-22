/* SPDX-License-Identifier: GPL-2.0-or-later
 *
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
#ifndef __OCM_INTERNEL__
#define __OCM_INTERNEL__

#include <linux/types.h>
#include <soc/xring/ocm_wrapper.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>

#define ocmerr(fmt, ...) \
	pr_err("[%s][E]: " fmt, __func__, ##__VA_ARGS__)

#define ocmwarn(fmt, ...) \
	pr_warn("[%s][W]: " fmt, __func__, ##__VA_ARGS__)

#define ocminfo(fmt, ...) \
	pr_info("[%s][I]: " fmt, __func__, ##__VA_ARGS__)

#define ocmdbg(fmt, ...) \
	pr_debug("[%s][D]: " fmt, __func__, ##__VA_ARGS__)


#endif
