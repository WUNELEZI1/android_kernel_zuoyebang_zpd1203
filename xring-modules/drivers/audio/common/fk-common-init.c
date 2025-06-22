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
#include "fk-common-init.h"

int __init audio_common_init(void)
{
	fk_audio_cma_init();
	xr_audif_init();
#ifdef SIMULATE_COMMON
	fk_simulate_init();
#endif
	// xr_audio_test_init();
	fk_audio_ipc_init();
	fk_audio_pinctrl_init();
	fk_audio_tool_init();

	return 0;
}
void audio_common_exit(void)
{
	fk_audio_cma_exit();
	xr_audif_exit();
#ifdef SIMULATE_COMMON
	fk_simulate_exit();
#endif
	fk_audio_ipc_exit();
	fk_audio_pinctrl_exit();
	fk_audio_tool_exit();
	// xr_audio_test_exit();
}

// module_init(audio_common_init);
// module_exit(audio_common_exit);

