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

#include <linux/delay.h>
#include <drm/drm_atomic.h>
#include <uapi/linux/sched/types.h>

#include <soc/xring/ddr_vote_dev.h>
#include <ddr_dvfs_vote.h>
#include "dpu_cont_display.h"
#include "dpu_power_mgr.h"
#include "dpu_log.h"
#include "dpu_dbg.h"
#include "dpu_core_perf.h"
#include "dpu_trace.h"

static struct dpu_power_mgr *g_power_mgr;

static void _do_dpu_power_enable(u32 part_id)
{
	int ret;

	if (part_id == DPU_PARTITION_0) {
		ret = regulator_enable(g_power_mgr->media1);
		if (ret) {
			POWER_ERROR("failed to power on media1\n");
			return;
		}
	}

	ret = dpu_clk_enable_for_partition(g_power_mgr->clk_mgr, part_id);
	if (ret) {
		POWER_ERROR("failed to enable partition %u's clk\n", part_id);
		return;
	}

	ret = regulator_enable(g_power_mgr->dpu_partitions[part_id]);
	if (ret) {
		POWER_ERROR("failed to power on dpu partition_%u: %d\n",
				part_id, ret);
		return;
	}
}

static void _do_dpu_power_disable(u32 part_id)
{
	regulator_disable(g_power_mgr->dpu_partitions[part_id]);

	dpu_clk_disable_for_partition(g_power_mgr->clk_mgr, part_id);

	if (part_id == DPU_PARTITION_0)
		regulator_disable(g_power_mgr->media1);
}

static void _do_dpu_power_init(struct dpu_power_ctrl_info *ctrl_info, u32 part_id)
{
	struct dpu_core_perf *core_perf = g_power_mgr->dpu_kms->core_perf;

	if (part_id == DPU_PARTITION_0) {
		POWER_DEBUG("power_enable part %d\n", part_id);
		dpu_core_perf_clk_init(core_perf);
	}
	dpu_core_perf_hw_init(core_perf, ctrl_info, part_id);
}

static void _do_dpu_power_get(struct dpu_power_ctrl_info *ctrl_info,
		u32 part_id)
{
	POWER_DEBUG("part %d, count %d\n", part_id,
			g_power_mgr->power_count[part_id]);

	mutex_lock(&g_power_mgr->lock);
	dpu_core_perf_clk_active();
	_do_dpu_power_enable(part_id);
	if (!g_power_mgr->power_count[part_id]++)
		_do_dpu_power_init(ctrl_info, part_id);
	dpu_core_perf_clk_deactive();
	mutex_unlock(&g_power_mgr->lock);
}

static void dpu_power_get_internal(struct dpu_power_ctrl_info *ctrl_info)
{
	struct dpu_power_ctrl_info temp;
	u32 depend_mask;
	int i;

	for (i = DPU_PARTITION_0; i < DPU_PARTITION_MAX_NUM; i++) {
		if (BIT(i) & ctrl_info->part_mask) {
			depend_mask = g_power_mgr->depend_mask[i];
			if (depend_mask) {
				temp.part_mask = depend_mask;
				temp.scene_ctrl_id = ctrl_info->scene_ctrl_id;
				dpu_power_get_internal(&temp);
			}
			_do_dpu_power_get(ctrl_info, i);
		}
	}
}

void dpu_power_get(struct dpu_power_ctrl_info *ctrl_info)
{
	if (!g_power_mgr || !ctrl_info) {
		DPU_ERROR("invalid parameters %pK, %pK\n", g_power_mgr, ctrl_info);
		return;
	} else if (ctrl_info->scene_ctrl_id > SCENE_CTL_MAX) {
		DPU_ERROR("invalid scene ctrl id %d\n", ctrl_info->scene_ctrl_id);
		return;
	}

	if (g_power_mgr->suspend_block) {
		POWER_ERROR("dpu in suspend state\n");
		return;
	}

	dpu_power_get_internal(ctrl_info);
}

static void _do_dpu_power_early_deinit(u32 part_id)
{
	struct dpu_core_perf *core_perf = g_power_mgr->dpu_kms->core_perf;

	if (part_id == DPU_PARTITION_0) {
		POWER_DEBUG("power disable part %d\n", part_id);
		dpu_hw_tmg_mclk_auto_cg_dump();
		dpu_core_perf_clk_deinit(core_perf);
	}
}

static void _do_dpu_power_late_deinit(u32 part_id)
{
	struct dpu_core_perf *core_perf = g_power_mgr->dpu_kms->core_perf;

	if (!g_power_mgr->power_count[part_id]) {
		dpu_core_perf_hw_deinit(core_perf, part_id);
		if (g_power_mgr->power_off_cb[part_id])
			g_power_mgr->power_off_cb[part_id](g_power_mgr->cb_data[part_id]);
	}
}

static void _do_dpu_power_put(u32 part_id)
{
	POWER_DEBUG("part %d, count %d\n", part_id,
			g_power_mgr->power_count[part_id]);

	if (WARN_ON(!g_power_mgr->power_count[part_id]))
		return;

	mutex_lock(&g_power_mgr->lock);
	dpu_core_perf_clk_active();
	if (!--g_power_mgr->power_count[part_id])
		_do_dpu_power_early_deinit(part_id);
	_do_dpu_power_disable(part_id);
	_do_dpu_power_late_deinit(part_id);
	dpu_core_perf_clk_deactive();
	mutex_unlock(&g_power_mgr->lock);
}

static void dpu_power_put_internal(u32 part_mask)
{
	int i, depend_mask;

	for (i = DPU_PARTITION_MAX_NUM - 1; i >= DPU_PARTITION_0; i--) {
		if (BIT(i) & part_mask) {
			_do_dpu_power_put(i);
			depend_mask = g_power_mgr->depend_mask[i];
			if (depend_mask)
				dpu_power_put_internal(depend_mask);
		}
	}
}

void dpu_power_put(u32 part_mask)
{
	if (!g_power_mgr || !part_mask) {
		DPU_ERROR("invalid parameters %pK, %u\n", g_power_mgr, part_mask);
		return;
	}

	dpu_power_put_internal(part_mask);
}

static void client_power_enable(struct dpu_power_client *client, u32 part_mask)
{
	struct dpu_power_ctrl_info ctrl_info = {0};

	if (!client) {
		POWER_ERROR("invalid parameters\n");
		return;
	}

	if (!part_mask)
		return;

	ctrl_info.part_mask = part_mask;
	ctrl_info.scene_ctrl_id = client->sense_ctrl_id;

	mutex_lock(&client->lock);
	dpu_power_get(&ctrl_info);
	if ((part_mask & BIT(DPU_PARTITION_0)) &&
			!(client->power_mask & BIT(DPU_PARTITION_0)))
		client->power_status = DPU_POWER_ON;
	client->power_mask |= part_mask;
	mutex_unlock(&client->lock);
}

static void client_power_disable(struct dpu_power_client *client, u32 part_mask)
{
	if (!client) {
		POWER_ERROR("invalid parameters\n");
		return;
	}

	if (!part_mask)
		return;

	mutex_lock(&client->lock);
	dpu_power_put(part_mask);
	if ((client->power_mask & BIT(DPU_PARTITION_0)) &&
			(part_mask & BIT(DPU_PARTITION_0)))
		client->power_status = DPU_POWER_OFF;
	client->power_mask &= ~part_mask;
	mutex_unlock(&client->lock);
}

static bool is_client_powered_on(struct dpu_power_client *client)
{
	return client->power_status == DPU_POWER_ON;
}

struct dpu_power_client_ops default_client_power_ops = {
	.enable = client_power_enable,
	.disable = client_power_disable,
	.is_powered_on = is_client_powered_on,
};

static int dpu_power_client_init_add(struct dpu_power_client *client)
{
	if (!g_power_mgr || !client) {
		POWER_ERROR("invalid parameters: %pK, %pK\n",
				g_power_mgr, client);
		return -EINVAL;
	}

	client->power_status = DPU_POWER_OFF;
	client->ops = &default_client_power_ops;
	INIT_LIST_HEAD(&client->list);
	mutex_init(&client->lock);

	spin_lock(&g_power_mgr->client_lock);
	list_add_tail(&client->list, &g_power_mgr->clients);
	spin_unlock(&g_power_mgr->client_lock);

	return 0;
}

static void dpu_power_client_remove(struct dpu_power_client *client)
{
	if (!g_power_mgr || !client) {
		POWER_ERROR("invalid parameters: %pK, %pK\n", g_power_mgr, client);
		return;
	}

	WARN_ON(client->power_mask);
	mutex_destroy(&client->lock);

	spin_lock(&g_power_mgr->client_lock);
	list_del(&client->list);
	spin_unlock(&g_power_mgr->client_lock);
}

struct dpu_power_client *dpu_power_client_alloc(
		struct dpu_power_client_info *info)
{
	struct dpu_power_client *client;
	int ret;

	if (!info) {
		DPU_ERROR("info is NULL\n");
		return ERR_PTR(-EINVAL);
	} else if (info->sense_ctrl_id >= SCENE_CTL_MAX) {
		DPU_ERROR("invalid sense ctrl id %u\n", info->sense_ctrl_id);
		return ERR_PTR(-EINVAL);
	}

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (!client) {
		DPU_ERROR("failed alloc power client\n");
		return ERR_PTR(-ENOMEM);
	}

	client->sense_ctrl_id = info->sense_ctrl_id;
	ret = dpu_power_client_init_add(client);
	if (ret) {
		DPU_ERROR("failed to init power client, ret %d\n", ret);
		goto error;
	}

	POWER_DEBUG("init client, sense id %d\n", client->sense_ctrl_id);

	return client;

error:
	kfree(client);
	return ERR_PTR(-EINVAL);
}

void dpu_power_client_free(struct dpu_power_client *client)
{
	if (!client) {
		POWER_ERROR("invalid parameters\n");
		return;
	}

	POWER_DEBUG("remove client, sense id %d\n", client->sense_ctrl_id);
	dpu_power_client_remove(client);
	kfree(client);
}

void dpu_power_get_helper(u32 part_mask)
{
	struct dpu_power_ctrl_info ctrl_info = {0};

	ctrl_info.part_mask = part_mask;
	ctrl_info.scene_ctrl_id = SCENE_CTL_MAX;
	dpu_power_get(&ctrl_info);

	dpu_core_perf_clk_active();
}

void dpu_power_put_helper(u32 part_mask)
{
	dpu_power_put(part_mask);
	dpu_core_perf_clk_deactive();
}

int dpu_power_register_power_off_cb(u32 part_id, void *data,
	void (*cb)(void *data))
{
	if (!g_power_mgr || part_id >= DPU_PARTITION_MAX_NUM) {
		POWER_ERROR("invalid parameters %pK, %d\n", g_power_mgr, part_id);
		return -EINVAL;
	}

	if (g_power_mgr->power_off_cb[part_id]) {
		POWER_ERROR("part_id %d already has power off cb\n", part_id);
		return -EINVAL;
	}
	g_power_mgr->power_off_cb[part_id] = cb;
	g_power_mgr->cb_data[part_id] = data;

	return 0;
}

void dpu_power_unregister_power_off_cb(u32 part_id)
{
	if (!g_power_mgr || part_id >= DPU_PARTITION_MAX_NUM) {
		POWER_ERROR("invalid parameters %pK, %d\n", g_power_mgr, part_id);
		return;
	}

	g_power_mgr->power_off_cb[part_id] = NULL;
	g_power_mgr->cb_data[part_id] = NULL;
}

bool dpu_power_is_in_suspending(void)
{
	if (!g_power_mgr) {
		POWER_ERROR("power mgr uninitialized\n");
		return -EINVAL;
	}

	return g_power_mgr->suspend_block;
}

int dpu_power_resume(void)
{
	int ret = 0;

	if (!g_power_mgr) {
		POWER_ERROR("power mgr uninitialized\n");
		return -EINVAL;
	}

	g_power_mgr->suspend_block = false;

	return ret;
}

int dpu_power_suspend(void)
{
	int ret = 0;
	int i;

	if (!g_power_mgr) {
		POWER_ERROR("power mgr uninitialized\n");
		return -EINVAL;
	}

	g_power_mgr->suspend_block = true;

	for (i = DPU_PARTITION_0; i < DPU_PARTITION_MAX_NUM; i++)
		WARN_ON(g_power_mgr->power_count[i] > 0);

	return ret;
}

bool is_dpu_powered_on(void)
{
	if (!g_power_mgr) {
		POWER_ERROR("power mgr uninitialized\n");
		return false;
	}

	if (g_power_mgr->power_count[DPU_PARTITION_0] > 0)
		return true;
	else
		return false;
}

int dpu_power_mgr_init(struct dpu_power_mgr **mgr,
	struct drm_device *drm_dev)
{
	struct dpu_power_mgr *power_mgr = NULL;
	struct dpu_drm_device *dpu_drm_dev;
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };
	char supply[20];
	u32 depend_mask;
	int ret;
	int i;

	if (!drm_dev) {
		POWER_ERROR("invalid parameter %pK\n", drm_dev);
		return -EINVAL;
	}

	if (*mgr) {
		POWER_ERROR("power manager has been initialized\n");
		return -EINVAL;
	}

	power_mgr = kzalloc(sizeof(*power_mgr), GFP_KERNEL);
	if (!power_mgr)
		return -ENOMEM;

	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	power_mgr->drm_dev = drm_dev;
	power_mgr->dpu_kms = dpu_drm_dev->dpu_kms;

	depend_mask = 0;
	for (i = 0; i < DPU_PARTITION_MAX_NUM; i++) {
		snprintf(supply, sizeof(supply), "dpu%d", i);
		power_mgr->dpu_partitions[i] =
				devm_regulator_get(drm_dev->dev, supply);
		if (IS_ERR(power_mgr->dpu_partitions[i])) {
			POWER_ERROR("failed to get %s supply\n", supply);
			goto err;
		}

		power_mgr->depend_mask[i] = depend_mask;
		depend_mask |= BIT(i);
		power_mgr->power_count[i] = 0;
	}

	power_mgr->media1 = devm_regulator_get(drm_dev->dev, "media1");
	if (IS_ERR(power_mgr->media1)) {
		POWER_ERROR("failed to get media1 supply\n");
		goto err;
	}

	ret = dpu_clk_mgr_init(power_mgr);
	if (ret) {
		POWER_ERROR("failed to do clk mgr init\n");
		goto err;
	}

	kthread_init_worker(&power_mgr->power_worker);
	power_mgr->power_task = kthread_create(kthread_worker_fn,
				&power_mgr->power_worker, "dpu_power_worker");
	if (IS_ERR_OR_NULL(power_mgr->power_task)) {
		DPU_ERROR("failed to create dpu power thread\n");
		ret = PTR_ERR(power_mgr->power_task);
		goto err_power_thread;

	}
	ret = sched_setscheduler_nocheck(power_mgr->power_task, SCHED_FIFO, &param);
	if (ret < 0)
		DPU_ERROR("setscheduler failed\n");

	wake_up_process(power_mgr->power_task);

	mutex_init(&power_mgr->lock);
	spin_lock_init(&power_mgr->client_lock);
	INIT_LIST_HEAD(&power_mgr->clients);
	power_mgr->suspend_block = false;

	*mgr = power_mgr;
	g_power_mgr = power_mgr;

	return 0;

err_power_thread:
	kthread_stop(power_mgr->power_task);
err:
	kfree(power_mgr);

	return -EINVAL;
}

void dpu_power_mgr_deinit(struct dpu_power_mgr *power_mgr)
{
	if (!power_mgr) {
		PERF_ERROR("invalid parameter\n");
		return;
	}
	mutex_destroy(&power_mgr->lock);
	dpu_clk_mgr_deinit(power_mgr);
	kfree(power_mgr);
}
