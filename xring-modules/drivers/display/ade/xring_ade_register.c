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

#include "veu/veu_drv.h"
#include "xplayer/xring_xplayer_drv.h"
#include "dpu_helper/dpu_drv.h"

static int __init xring_ade_register(void)
{
	int ret;

	ret = veu_drv_register();
	veu_check_and_return(ret, -EINVAL, "veu register fail");

	ret = xplayer_drv_register();
	xplayer_check_and_return(ret, -EINVAL, "xplayer register fail");

	return ret;
}

static void __exit xring_ade_unregister(void)
{
	veu_drv_unregister();
	xplayer_drv_unregister();
}

MODULE_LICENSE("GPL v2");
module_init(xring_ade_register);
module_exit(xring_ade_unregister);
