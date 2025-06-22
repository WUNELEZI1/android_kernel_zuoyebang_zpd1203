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

#include "osal.h"
#include "litexdm.h"
#include "pipeline_hw_res.h"
#include "dpu_comp_mgr.h"
#include "dpu_conn_mgr.h"
#include "panel_mgr.h"
#include "dpu_lcd_type.h"
#include "dpu_log.h"
#include "dpu_res_mgr.h"

static struct dpu_ops g_dpu_ops;
static bool g_is_dpu_inited;
static bool g_power_state;

static int32_t dpu_scene_power_on(struct dpu_frame *frame)
{
	int32_t ret;

	if (g_power_state) {
		dpu_pr_info("dpu has already powered on\n");
		return 0;
	}

	frame->scene_id = g_dpu_ops.scene_id;
	frame->profile_id = g_dpu_ops.profile_id;
	frame->volt_level = g_dpu_ops.volt_level;

	ret = comp_mgr_power_on(frame);
	if (ret) {
		dpu_pr_err("scene:%u profile_id:%u comp_mgr_on failed\n",
				g_dpu_ops.scene_id, g_dpu_ops.profile_id);
		return -1;
	}

	g_power_state = true;
	return 0;
}

static int32_t dpu_scene_enable(struct dpu_frame *frame)
{
	int32_t ret;

	frame->scene_id = g_dpu_ops.scene_id;

	ret = comp_mgr_enable(frame);
	if (ret) {
		dpu_pr_err("scene:%u comp_mgr_on failed\n", frame->scene_id);
		return -1;
	}

	return 0;
}

static int32_t dpu_scene_power_off(struct dpu_frame *frame)
{
	int32_t ret;

	frame->scene_id = g_dpu_ops.scene_id;

	if (!g_power_state) {
		dpu_pr_info("dpu has already powered off\n");
		return 0;
	}

	ret = comp_mgr_power_off(frame);
	g_power_state = false;
	return ret;
}

static int32_t dpu_scene_present(struct dpu_frame *frame)
{
	if (!g_power_state) {
		dpu_pr_info("dpu not powered on\n");
		return 0;
	}

	return comp_mgr_present(frame);
}

static int32_t dpu_get_panel_info(struct panel_base_info *out_pinfo)
{
	return comp_mgr_get_panel_info(out_pinfo, g_dpu_ops.scene_id);
}

static int32_t dpu_scene_ops_handle(char *ops_type, void *data)
{
	if (!g_power_state) {
		dpu_pr_info("dpu has powered off\n");
		return 0;
	}
	return 0;
}

static int32_t dpu_dump_info(bool debug_en)
{
	return composer_dump(g_dpu_ops.scene_id, debug_en);
}

static void init_dpu_ops(void)
{
	g_dpu_ops.power_on = dpu_scene_power_on;
	g_dpu_ops.power_off = dpu_scene_power_off;
	g_dpu_ops.enable = dpu_scene_enable;
	g_dpu_ops.present = dpu_scene_present;
	g_dpu_ops.get_panel_info = dpu_get_panel_info;
	g_dpu_ops.ops_handle = dpu_scene_ops_handle;
	g_dpu_ops.dump_info = dpu_dump_info;
}

uint32_t dpu_get_boot_up_profile()
{
	struct panel_drv_private *priv;
	struct panel_dev *panel;
	uint32_t bootup_profile;

	panel = get_panel_dev(PANEL_PRIMARY);
	if (!panel) {
		dpu_pr_err("failed to get panel dev, set bootup profile to PROFILE_0\n");
		return DPU_PROFILE_0;
	}

	priv = to_panel_priv(platform_get_drvdata(panel->pdev));
	bootup_profile = priv->base.bootup_profile;
	dpu_pr_info("get bootup profile %d\n", bootup_profile);

	return bootup_profile;
}

int32_t litexdm_init(void)
{
	int32_t ret;

	dpu_pr_info("+\n");
	if (g_is_dpu_inited)
		return 0;

	init_dpu_ops();

	ret = dpu_res_mgr_init();
	dpu_check_and_return(ret != 0, ret, "res mgr init failed\n");

	ret = dpu_comp_mgr_init();
	dpu_check_and_return(ret != 0, ret, "dpu_comp_mgr_init failed\n");

	ret = dpu_conn_mgr_init();
	dpu_check_and_return(ret != 0, ret, "dpu_conn_mgr_init failed\n");

	ret = panel_init();
	dpu_check_and_return(ret != 0, ret, "panel_init failed\n");

	ret = dpu_get_lcd_type();
	dpu_check_and_return(ret != 0, ret, "get_lcd_type failed\n");

	g_dpu_ops.profile_id = 0;
	g_dpu_ops.volt_level = 0;
	g_dpu_ops.scene_id = dpu_get_scene_id_from_lcd();

	g_is_dpu_inited = true;
	dpu_pr_info("-\n");
	return ret;
}

int32_t litexdm_factory_init(void)
{
	int32_t ret;

	dpu_pr_info("+\n");
	if (g_is_dpu_inited)
		return 0;

	init_dpu_ops();

	ret = dpu_res_mgr_init();
	dpu_check_and_return(ret != 0, ret, "res mgr init failed\n");

	ret = dpu_comp_mgr_init();
	dpu_check_and_return(ret != 0, ret, "dpu_comp_mgr_init failed\n");

	ret = dpu_conn_mgr_init();
	dpu_check_and_return(ret != 0, ret, "dpu_conn_mgr_init failed\n");

	ret = panel_init();
	dpu_check_and_return(ret != 0, ret, "panel_init failed\n");

#ifdef DPU_FACTORY_DEBUG_WITH_PANEL
	ret = dpu_get_lcd_type();
	dpu_check_and_return(ret != 0, ret, "dpu_get_lcd_type failed\n");
#else
	ret = dpu_get_fake_lcd_type();
	dpu_check_and_return(ret != 0, ret, "dpu_get_fake_lcd_type failed\n");
#endif

	g_is_dpu_inited = true;
	dpu_pr_info("-\n");
	return ret;
}

struct dpu_ops *get_dpu_ops(void)
{
	return (g_is_dpu_inited) ? &g_dpu_ops : NULL;
}
