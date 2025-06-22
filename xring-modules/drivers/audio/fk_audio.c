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

#include <linux/kernel.h>
#include <linux/module.h>
#include "asoc/platform_init.h"
#include "common/fk-common-init.h"
#include "adsp/adsp_driver_init.h"
#include "acore/fk-acore.h"


static int __init xring_audio_init(void)
{
	xring_adsp_driver_init();
	audio_common_init();
	fk_acore_init();
	audio_platform_init();

	return 0;
}

static void xring_audio_exit(void)
{
	xring_adsp_driver_exit();
	audio_common_exit();
	fk_acore_exit();
	audio_platform_exit();
}

module_init(xring_audio_init);
module_exit(xring_audio_exit);

MODULE_DESCRIPTION("Audio Platform driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_SOFTDEP("pre: xr_regulator");
MODULE_SOFTDEP("pre: xr_ipc");
MODULE_SOFTDEP("pre: xring_smartpa");
MODULE_SOFTDEP("pre: xring_codec");
MODULE_SOFTDEP("pre: madera_codec_cs47l92");
