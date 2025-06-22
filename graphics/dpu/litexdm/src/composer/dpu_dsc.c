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

#include "dpu_log.h"
#include "platform_device.h"
#include "dpu_dsc.h"
#include "dpu_conn_mgr.h"
#include "dpu_dsc_algorithm.h"

void dpu_update_comp_dsc_cfg(struct composer *comp)
{
	struct dsc_caps_src dsc_caps_src;

	dpu_check_and_no_retval(!comp, "comp is null\n");
	if (comp->dsc_cfg.valid)
		return;

	dpu_check_and_no_retval(!comp->pinfo, "comp->pinfo is null\n");
	if (!comp->pinfo->dsc_cfg.dsc_en) {
		comp->dsc_cfg.valid = false;
		return;
	}

	dpu_check_and_no_retval(!comp->hw_dsc, "comp->hw_dsc is null\n");
	comp->hw_dsc->ops.get_caps(&comp->hw_dsc->hw, &dsc_caps_src);

	dpu_dsc_get_config(&comp->pinfo->dsc_cfg, NULL, &dsc_caps_src,
			 &comp->dsc_cfg);

	comp->dsc_cfg.dual_port = comp->pinfo->dual_port;
}
