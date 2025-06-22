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

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <soc/xring/xr-clk-provider.h>
#include "dpu_hw_dvfs.h"
#include "dpu_core_perf.h"
#include "dpu_res_mgr.h"
#include "dpu_intr_core.h"
#include "dsi_encoder.h"
#include "dpu_trace.h"

static struct  dpu_hw_dvfs_mgr *g_dvfs_mgr;

int dpu_hw_dvfs_vote(bool enable)
{
	struct dpu_hw_dvfs_mgr *hw_dvfs_mgr = g_dvfs_mgr;
	struct task_struct *task;

	if (!hw_dvfs_mgr) {
		PERF_ERROR("hw dvfs mgr uninitialized\n");
		return -EINVAL;
	}

	task = get_current();

	PERF_INFO("%s vote hw_dvfs %d\n", task->comm, enable);

	if (enable)
		dpu_hw_dvfs_enable(hw_dvfs_mgr);
	else
		dpu_hw_dvfs_disable(hw_dvfs_mgr);

	return 0;
}
EXPORT_SYMBOL_GPL(dpu_hw_dvfs_vote);

static void _dump_hw_dvfs_status(struct dpu_hw_dvfs_mgr *dvfs_mgr, int flag)
{
	struct hdc_status status;
	struct hdc_cfg *cfg;

	dpu_hw_dvfs_status_dump(dvfs_mgr, &status, flag);
	cfg = &status.cfg;

	PERF_INFO("sw_clk_en:               0x%x\n", status.sw_clk_en);
	PERF_INFO("auto_cg_en:              0x%x\n", status.auto_cg_en);
	PERF_INFO("idle_dvfs_en:            0x%x\n", status.idle_dvfs_en);
	PERF_INFO("active_dvfs_en:          0x%x\n", status.active_dvfs_en);

	PERF_INFO("work_status:             0x%x\n", status.work_status);
	PERF_INFO("fail_counter:            0x%x\n", status.fail_counter);
	PERF_INFO("soft_clr_val:            0x%x\n", status.soft_clr_val);

	PERF_INFO("active_div_core:         0x%x\n", cfg->active_div_core);
	PERF_INFO("active_div_veu:          0x%x\n", cfg->active_div_veu);
	PERF_INFO("active_div_axi:          0x%x\n", cfg->active_div_axi);
	PERF_INFO("active_div_pclk:         0x%x\n", cfg->active_div_pclk);

	PERF_INFO("idle_div_core:           0x%x\n", cfg->idle_div_core);
	PERF_INFO("idle_div_veu:            0x%x\n", cfg->idle_div_veu);
	PERF_INFO("idle_div_axi:            0x%x\n", cfg->idle_div_axi);
	PERF_INFO("idle_div_pclk:           0x%x\n", cfg->idle_div_pclk);

	PERF_INFO("timeout_counter:         0x%x\n", cfg->timeout_counter);
	PERF_INFO("sync_counter:            0x%x\n", cfg->sync_counter);
	PERF_INFO("idle_volt:               0x%x\n", cfg->idle_volt);
	PERF_INFO("active_volt:             0x%x\n", cfg->active_volt);
	PERF_INFO("freq_div_counter:        0x%x\n", cfg->freq_div_counter);
	PERF_INFO("active_low_volt:         0x%x\n", cfg->active_low_volt);
	PERF_INFO("volt_scaling_disable:    0x%x\n", cfg->volt_scaling_disable);
}

static void hw_dvfs_work_handler(struct work_struct *work)
{
	struct dpu_hw_dvfs_mgr *dvfs_mgr;

	dvfs_mgr = container_of(work, struct dpu_hw_dvfs_mgr, hw_dvfs_work);
	if (!dvfs_mgr) {
		PERF_ERROR("dvfs_mgr is NULL\n");
		return;
	}

	PERF_ERROR("hw dvfs work failed, %llu\n", dvfs_mgr->fail_counter);
	DPU_POWER_PROTECT_BEGIN();

	_dump_hw_dvfs_status(dvfs_mgr, READ_SHADOW_VAL);

	dpu_hw_dvfs_disable(dvfs_mgr);

	msleep(DPU_HW_DVFS_TRY_AGAIN_DELAY);

	if (dvfs_mgr->fail_counter < DPU_HW_DVFS_MAX_ALLOWED_FAILED_TIMES)
		dpu_hw_dvfs_enable(dvfs_mgr);

	DPU_POWER_PROTECT_END();
}

static void dpu_hw_dvfs_intr_handler(int intr_id, void *data)
{
	struct dpu_hw_dvfs_mgr *dvfs_mgr;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;
	enum dpu_intr_type intr_type;

	intr_ctx = data;
	intr_type = dpu_interrupt_id_to_type(intr_id);
	if (intr_type >= DPU_INTR_TYPE_MAX)
		return;

	dpu_drm_dev = to_dpu_drm_dev(intr_ctx->drm_dev);
	dvfs_mgr = &dpu_drm_dev->dpu_kms->core_perf->hw_dvfs_mgr;

	if (intr_id == INTR_ONLINE0_DVFS_FAIL) {
		queue_work(dvfs_mgr->hw_dvfs_wq, &dvfs_mgr->hw_dvfs_work);
		dvfs_mgr->fail_counter++;
	}
}

static int dpu_hw_dvfs_int_register(struct dpu_intr_context *intr_ctx)
{
	int ret;

	ret = dpu_interrupt_register(intr_ctx, INTR_ONLINE0_DVFS_FAIL,
			&dpu_hw_dvfs_intr_handler);
	if (ret) {
		PERF_ERROR("faild to register hw dvfs int %d\n", ret);
		return ret;
	}
	return 0;
}

static void dpu_hw_dvfs_int_unregister(struct dpu_intr_context *intr_ctx)
{
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_DVFS_FAIL);
}

static void _do_dpu_hw_dvfs_enable(struct dpu_hw_dvfs_mgr *dvfs_mgr)
{
	struct dpu_core_perf *perf;

	perf = dvfs_mgr->dpu_kms->core_perf;
	dvfs_mgr->curr_profile = perf->curr_profile;

	dvfs_mgr->hw_dvs->ops->sw_to_hw_vote(&dvfs_mgr->hw_dvs->hw);
	dvfs_mgr->hw_glb->ops.hdc_config(&dvfs_mgr->hw_glb->hw,
			dvfs_mgr->curr_profile);

	dvfs_mgr->hw_glb->ops.hdc_enable(&dvfs_mgr->hw_glb->hw);
}

static void _do_dpu_hw_dvfs_disable(struct dpu_hw_dvfs_mgr *dvfs_mgr)
{
	u32 hw_volt;

	if (!dvfs_mgr) {
		PERF_ERROR("invalid parameter\n");
		return;
	}

	hw_volt = dvfs_mgr->hw_glb->ops.hdc_get_hw_vote_val(&dvfs_mgr->hw_glb->hw);
	dvfs_mgr->hw_glb->ops.hdc_disable(&dvfs_mgr->hw_glb->hw);

	dvfs_mgr->hw_dvs->ops->hw_to_sw_vote(&dvfs_mgr->hw_dvs->hw, hw_volt);
	dvfs_mgr->curr_profile = DPU_CORE_PERF_INVALID_PROFILE_ID;
}

void dpu_hw_dvfs_status_dump(struct dpu_hw_dvfs_mgr *dvfs_mgr,
		struct hdc_status *status, int flag)
{
	if (!dvfs_mgr || !status) {
		PERF_ERROR("invalid parameter %pK, %pK\n", dvfs_mgr, status);
		return;
	}

	dvfs_mgr->hw_glb->ops.hdc_dump(&dvfs_mgr->hw_glb->hw, status, flag);
}

void dpu_hw_dvfs_dvs_status_dump(struct dpu_hw_dvfs_mgr *dvfs_mgr, int cnt)
{
	if (!dvfs_mgr) {
		PERF_ERROR("invalid parameter\n");
		return;
	}

	dvfs_mgr->hw_dvs->ops->dvs_state_dump(&dvfs_mgr->hw_dvs->hw, cnt);
}

void dpu_hw_dvfs_enable(struct dpu_hw_dvfs_mgr *dvfs_mgr)
{
	struct dpu_core_perf *core_perf;
	struct dpu_virt_ctrl *virt_ctrl;

	if (!dvfs_mgr) {
		PERF_ERROR("invalid parameter\n");
		return;
	}

	virt_ctrl = &dvfs_mgr->dpu_kms->virt_ctrl;
	core_perf = dvfs_mgr->dpu_kms->core_perf;

	if (!is_dpu_lp_enabled(DPU_LP_HW_DVFS_ENABLE))
		return;

	PERF_DEBUG("enable_counter %d\n", dvfs_mgr->vote_counter);

	mutex_lock(&dvfs_mgr->dvfs_mutex);
	dvfs_mgr->vote_counter++;

	if ((dvfs_mgr->vote_counter == 1) &&
			(core_perf->curr_profile != DPU_CORE_PERF_INVALID_PROFILE_ID)) {
		PERF_DEBUG("enable hw dvfs\n");
		trace_dpu_hw_dvfs_enable("hw dvfs enabled");
		dpu_core_perf_clk_active();
		dpu_hw_dvfs_int_register(virt_ctrl->intr_ctx);
		_do_dpu_hw_dvfs_enable(dvfs_mgr);
		dpu_core_perf_clk_deactive();
		dvfs_mgr->status = DPU_HW_DVFS_ENABLED;
	}

	mutex_unlock(&dvfs_mgr->dvfs_mutex);
}

void dpu_hw_dvfs_disable(struct dpu_hw_dvfs_mgr *dvfs_mgr)
{
	struct dpu_virt_ctrl *virt_ctrl;

	if (!dvfs_mgr) {
		PERF_ERROR("invalid parameter\n");
		return;
	}

	virt_ctrl = &dvfs_mgr->dpu_kms->virt_ctrl;

	if (!is_dpu_lp_enabled(DPU_LP_HW_DVFS_ENABLE))
		return;

	PERF_DEBUG("vote_counter %d\n", dvfs_mgr->vote_counter);

	mutex_lock(&dvfs_mgr->dvfs_mutex);
	dvfs_mgr->vote_counter--;

	if ((dvfs_mgr->vote_counter == 0) &&
			(dvfs_mgr->status == DPU_HW_DVFS_ENABLED)) {
		PERF_DEBUG("disable hw dvfs\n");
		trace_dpu_hw_dvfs_disable("hw dvfs disabled");
		dpu_core_perf_clk_active();
		_do_dpu_hw_dvfs_disable(dvfs_mgr);
		dpu_hw_dvfs_int_unregister(virt_ctrl->intr_ctx);
		dpu_core_perf_clk_deactive();

		dvfs_mgr->status = DPU_HW_DVFS_DISABLED;
	}

	mutex_unlock(&dvfs_mgr->dvfs_mutex);
}

int dpu_hw_dvfs_mgr_init(struct dpu_hw_dvfs_mgr *dvfs_mgr,
		struct dpu_kms *dpu_kms)
{
	struct dpu_hw_blk *hw_blk;
	bool valid_avs_code = true;
	u8 avs_code;
	int i, ret;

	dvfs_mgr->dpu_kms = dpu_kms;
	dvfs_mgr->status = DPU_HW_DVFS_DISABLED;
	dvfs_mgr->hw_dvfs_wq = create_singlethread_workqueue("hw_dvfs");

	if (IS_ERR_OR_NULL(dvfs_mgr->hw_dvfs_wq)) {
		PERF_ERROR("failed to create workqueue\n");
		return -EINVAL;
	}

	hw_blk = dpu_res_mgr_block_reserve_id(
			dpu_kms->res_mgr_ctx,
			VIRTUAL_CTRL_PIPE_ID, BLK_DPU_GLB, DPU_GLB_0, false);
	if (IS_ERR_OR_NULL(hw_blk)) {
		PERF_ERROR("failed to reserve dpu glb blk\n");
		goto err_glb;
	}
	dvfs_mgr->hw_glb = to_dpu_hw_glb(hw_blk);

	INIT_WORK(&dvfs_mgr->hw_dvfs_work, hw_dvfs_work_handler);
	mutex_init(&dvfs_mgr->dvfs_mutex);

	dvfs_mgr->hw_dvs = dpu_hw_dvs_init();
	if (IS_ERR_OR_NULL(dvfs_mgr->hw_dvs)) {
		PERF_ERROR("failed to init dvs hw module\n");
		goto err_dvs;
	}

	for (i = DPU_PROFILE_0; i < DPU_PROFILE_MAX_NUM; i++) {
		ret = get_dpu_avs_volt(i, &avs_code);
		if (!ret && avs_code) {
			dvfs_mgr->avs_codes[i] = avs_code;
			PERF_INFO("profile %d, avs_code 0x%x\n", i, avs_code);
		} else {
			PERF_ERROR("failed to get profile %d's avs code %d\n", i, avs_code);
			valid_avs_code = false;
			break;
		}
	}

	if (valid_avs_code)
		dvfs_mgr->hw_glb->ops.hdc_update_avs_codes(dvfs_mgr->avs_codes);

	g_dvfs_mgr = dvfs_mgr;

	return 0;

err_dvs:
	dpu_res_mgr_block_release(dvfs_mgr->dpu_kms->res_mgr_ctx, &dvfs_mgr->hw_glb->hw);
err_glb:
	destroy_workqueue(dvfs_mgr->hw_dvfs_wq);
	mutex_destroy(&dvfs_mgr->dvfs_mutex);
	return -EINVAL;
}

void dpu_hw_dvfs_mgr_deinit(struct dpu_hw_dvfs_mgr *dvfs_mgr)
{
	g_dvfs_mgr = NULL;

	dpu_hw_dvs_deinit(dvfs_mgr->hw_dvs);
	dpu_res_mgr_block_release(dvfs_mgr->dpu_kms->res_mgr_ctx, &dvfs_mgr->hw_glb->hw);
	flush_workqueue(dvfs_mgr->hw_dvfs_wq);
	destroy_workqueue(dvfs_mgr->hw_dvfs_wq);
	mutex_destroy(&dvfs_mgr->dvfs_mutex);
}
