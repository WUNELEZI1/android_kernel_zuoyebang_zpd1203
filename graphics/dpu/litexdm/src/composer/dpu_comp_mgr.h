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

#ifndef _DPU_COMP_MGR_H_
#define _DPU_COMP_MGR_H_

#include "dpu_list.h"
#include "dpu_pipeline.h"
#include "platform_device.h"
#include "dpu_common_info.h"
#include "pipeline_hw_res.h"
#include "litexdm.h"
#include "dpu_post_color.h"

/**
 * composer - composer object, include rch/mixer/pq
 * @pinfo: base panel info. Variables in pinfo cannot be changed
 * @id: logical composer id
 * @scene_id: scene id
 * @hw_intr: intr hw ops
 * @hw_mixer: mixer hw ops
 * @hw_ctl_top: dpu ctl top hw ops
 * @hw_scene_ctl: scene_ctl hw ops
 * @hw_post_pipe_top: post pipe top hw ops
 * @lm_width: composer output width
 * @lm_height: composer output height
 * @postpq: color postpq struct
 * @hw_dsc: dsc hw ops
 * @dsc_cfg: dsc config parameters from panel
 * @hw_wb_top: dpu wb top hw ops
 * @hw_wb: dpu wb core hw ops
 * @parent_dev: comp_mgr dev
 * @power_on: composer power on ops
 * @power_off: composer power off ops
 * @enable: composer enable ops
 * @present: composer present config ops
 */
struct composer {
	struct dpu_panel_info *pinfo;
	uint32_t id;
	uint32_t scene_id;

	struct dpu_power_mgr pwr_mgr;

	struct dpu_hw_intr *hw_intr;
	struct dpu_hw_mixer *hw_mixer;
	struct dpu_hw_ctl_top *hw_ctl_top;
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dpu_hw_post_pipe_top *hw_post_pipe_top;

	uint16_t lm_width;
	uint16_t lm_height;
	struct color_postpq postpq;

	struct dpu_hw_dsc *hw_dsc;
	struct dpu_dsc_config dsc_cfg;
	struct dpu_hw_wb_top *hw_wb_top;
	struct dpu_hw_wb *hw_wb;
	struct dpu_hw_wb_scaler *hw_wb_scaler;
	struct dpu_hw_postpq_top *hw_pq_top;

	struct platform_device *parent_dev;
	int32_t (*power_on)(struct composer *comp);
	int32_t (*power_off)(struct composer *comp);
	int32_t (*enable)(struct composer *comp);
	int32_t (*present)(struct composer *comp, struct dpu_frame *frame);
};

/**
 * comp_mgr - composer mgr object
 * @base: base device
 * @comp_list: comp list parsed from pipeline_hw_res
 * @pdev: device object
 * @panel_list: panels mounted on comp_mgr
 */
struct comp_mgr {
	struct dpu_device_base base;
	struct composer *comp_list[MIXER_CNT];
	struct platform_device *pdev;
	struct list_head panel_list;
};

/**
 * comp_mgr_power_on - composer mgr power on
 *
 * @frame: dpu frame
 * @return 0 on success, -1 on failure
 */
int32_t comp_mgr_power_on(struct dpu_frame *frame);

/**
 * comp_mgr_power_off - composer mgr power off
 *
 * @frame: dpu frame
 * @return 0 on success, -1 on failure
 */
int32_t comp_mgr_power_off(struct dpu_frame *frame);

/**
 * comp_mgr_enable - composer mgr init basic module
 *
 * @frame: dpu frame
 * @return 0 on success, -1 on failure
 */
int32_t comp_mgr_enable(struct dpu_frame *frame);

/**
 * comp_mgr_present - composer mgr present config
 *
 * @frame: dpu frame
 * @return 0 on success, -1 on failure
 */
int32_t comp_mgr_present(struct dpu_frame *frame);

/**
 * comp_mgr_get_panel_info - get panel info
 *
 * @pinfo: base panel info
 * @scene_id: scene_id
 * @return 0 on success, -1 on failure
 */
int32_t comp_mgr_get_panel_info(struct panel_base_info *pinfo, uint32_t scene_id);

/**
 * dpu_comp_mgr_init - init comp mgr
 *
 * @return 0 on success, -1 on failure
 */
int32_t dpu_comp_mgr_init(void);

/**
 * register_composer - register composer
 *
 * @next_pdev: rear dev object
 * @pinfo: panel info
 * @return 0 on success, -1 on failure
 */
int32_t register_composer(struct platform_device *next_pdev, struct dpu_panel_info *pinfo);

void unregister_composer(void);

int32_t dpu_post_pipe_top_crc_read(struct composer *comp);
int32_t composer_dump(uint32_t scene_id, bool debug_en);
void dpu_update_comp_dsc_cfg(struct composer *comp);
struct comp_mgr *comp_mgr_get(void);
#endif
