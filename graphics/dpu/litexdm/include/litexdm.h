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

#ifndef _LITEXDM_H_
#define _LITEXDM_H_

#include "osal.h"
#include "dpu_hw_format.h"
#include "dpu_hw_common.h"
#include "DpuBaseType.h"

#if defined(DPU_FACTORY_VERSION)
#define DPU_POSTPIPE_CRC_EN 1
#else
#define DPU_POSTPIPE_CRC_EN 0
#endif

/* define dpu lowpower features */
enum DPU_LP_CTRL {
        DPU_LP_ALL_DISABLE = 0,
        DPU_LP_TOP_AUTO_CG_ENABLE = BIT(0),
        DPU_LP_PCLK_AUTO_CG_ENABLE = BIT(1),
        DPU_LP_AUTO_CG_ENABLE = BIT(2),
        DPU_LP_SRAM_LP_ENABLE = BIT(3),
        DPU_LP_HW_DVFS_ENABLE = BIT(4),
        DPU_LP_DOZE_ENABLE = BIT(5),
        DPU_LP_HW_PLL2_CTRL_ENABLE = BIT(6),
        DPU_LP_IDLE_CTRL_ENABLE = BIT(7),
        DPU_LP_FRAME_POWER_CTRL_ENABLE = BIT(8),
};

struct dpu_ops {
	/**
	 * power_on - dpu power on
	 *
	 * @return 0 on success, -1 on failure
	 */
	int32_t (*power_on)(struct dpu_frame *frame);
	/**
	 * power_off - dpu power off
	 *
	 * @return 0 on success, -1 on failure
	 */
	int32_t (*power_off)(struct dpu_frame *frame);
	/**
	 * enable - dpu some module to enable or init
	 * @frame: dpu frame info
	 *
	 * @return 0 on success, -1 on failure
	 */
	int32_t (*enable)(struct dpu_frame *frame);
	/**
	 * present - dpu present ops, for online and offline
	 * @frame: dpu frame info
	 *
	 * @return 0 on success, -1 on failure
	 */
	int32_t (*present)(struct dpu_frame *frame);

	/**
	 * set_backlight - set backlight ops
	 * @bl_lvl: bl level
	 *
	 * @return 0 on success, -1 on failure
	 */
	int32_t (*set_backlight)(uint32_t bl_lvl);

	/**
	 * get_panel_info - DpuDxe get panel info(xres/yres)
	 * @out_pinfo: pinfo need be assigned
	 *
	 * @return 0 on success, -1 on failure
	 */
	int32_t (*get_panel_info)(struct panel_base_info *out_pinfo);
	/**
	 * ops_handle - encapsulates some tool interfaces, such as mipi tx/rx
	 * @ops_type: ops type, used to detect func
	 * @data: preserve data
	 */
	int32_t (*ops_handle)(char *ops_type, void *data);
	/**
	 * dump_info - dump some reg value
	 * @debug_en: debug_en flag, if false just dump crc reg
	 *
	 * @return crc value on success, 0 on failure
	 */
	int32_t (*dump_info)(bool debug_en);

	/**
	 * @brief This parameter is not required by default!
	 *
	 * Notice: If you want to update profile_id and scene_id，
	 * it needs to be updated before the other interface calls!
	 * That's how it works.
	 *
	 */
	uint32_t profile_id;
	uint32_t volt_level;
	uint32_t scene_id;
};

/**
 * dpu_get_boot_up_profile - get dpu boot up profile
 *
 * return: boot up profile from panel dts
 */
uint32_t dpu_get_boot_up_profile(void);
/**
 * litexdm_init - litexdm init func, will be call firstly
 *
 * @return 0 on success, -1 on failure
 */
int32_t litexdm_init(void);
/**
 * litexdm_factory_init - litexdm factory init func, will be call firstly
 *
 * @return 0 on success, -1 on failure
 */
int32_t litexdm_factory_init(void);

/**
 * get_dpu_ops - Get the dpu ops object
 *
 * @return struct dpu_ops* on success, NULL on failure
 */
struct dpu_ops *get_dpu_ops(void);

#endif
