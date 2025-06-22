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

#include "dpu_clk_mgr.h"
#include "dpu_power_mgr.h"

#define DPU_RESET_CLK_DPU_CORE0 26112000

struct dpu_clk_rate {
	u32 clk_type;
	unsigned long rate;
};

/**
 * All clock gates inside DPU is stored here.
 * Clks that part_id is DPU_PARTITION_0/1/2 will be controled by partition.
 * Others will be controled solely.
 */
static const struct dpu_clock g_dpu_clks[] = {
	[CLK_DPU_CORE0] = {CLK_DPU_CORE0, "clk_dpu_core0", DPU_PARTITION_0},
	[CLK_DPU_CORE1] = {CLK_DPU_CORE1, "clk_dpu_core1", DPU_PARTITION_1},
	[CLK_DPU_CORE2] = {CLK_DPU_CORE2, "clk_dpu_core2", DPU_PARTITION_1},
	[CLK_DPU_CORE3] = {CLK_DPU_CORE3, "clk_dpu_core3", DPU_PARTITION_2},
	[CLK_DPU_AXI0] = {CLK_DPU_AXI0, "clk_dpu_axi0", DPU_PARTITION_0},
	[CLK_DPU_AXI1] = {CLK_DPU_AXI1, "clk_dpu_axi1", DPU_PARTITION_1},
	[CLK_DPU_AXI2] = {CLK_DPU_AXI2, "clk_dpu_axi2", DPU_PARTITION_1},
	[CLK_DPU_AXI3] = {CLK_DPU_AXI3, "clk_dpu_axi3", DPU_PARTITION_2},
	[CLK_DPU_BUS_DATA] = {CLK_DPU_BUS_DATA, "clk_disp_bus_data", DPU_PARTITION_0},
	[CLK_DPU_PCLK] = {CLK_DPU_PCLK, "pclk_dpu_cfg", DPU_PARTITION_0},
	[CLK_DPU_BUS_CFG] = {CLK_DPU_BUS_CFG, "clk_disp_bus_cfg", DPU_PARTITION_0},
	[CLK_DSI_CFG] = {CLK_DSI_CFG, "pclk_dsi_cfg", DPU_PARTITION_0},
	[CLK_DPU_DSC0] = {CLK_DPU_DSC0, "clk_dpu_dsc0", DPU_PARTITION_MAX_NUM},
	[CLK_DPU_DSC1] = {CLK_DPU_DSC1, "clk_dpu_dsc1", DPU_PARTITION_MAX_NUM},
};

static struct dpu_clk_rate g_dpu_reset_clk_rates[] = {
	{CLK_DPU_CORE0, DPU_RESET_CLK_DPU_CORE0},
};

static struct clk *dpu_get_clk(struct dpu_clk_mgr *clk_mgr,
		u32 clk_type)
{
	if (!clk_mgr || clk_type >= DPU_CLK_MAX_NUM) {
		CLK_ERROR("invalid parameter %pK, %u\n", clk_mgr, clk_type);
		return NULL;
	}

	return clk_mgr->dpu_clk[clk_type];
}

const char *dpu_get_clk_name(struct dpu_clk_mgr *clk_mgr, u32 clk_type)
{
	if (!clk_mgr || clk_type >= DPU_CLK_MAX_NUM) {
		CLK_ERROR("invalid parameter %pK, %u\n", clk_mgr, clk_type);
		return "invalid clk";
	}

	return g_dpu_clks[clk_type].clk_name;
}

void dpu_clk_set_rate(struct dpu_clk_mgr *clk_mgr,
		u32 clk_type, unsigned long rate)
{
	struct clk *clk = dpu_get_clk(clk_mgr, clk_type);
	int ret;

	if (clk) {
		ret = clk_set_rate(clk, rate);
		if (ret < 0)
			CLK_ERROR("failed to set clk %s(clk_type %u) to %lu, err:%d\n",
					dpu_get_clk_name(clk_mgr, clk_type),
					clk_type,
					rate,
					ret);
		else
			CLK_DEBUG("set clk %s(clk_type %u) rate to %lu success\n",
					dpu_get_clk_name(clk_mgr, clk_type),
					clk_type,
					rate);
	}
}

void dpu_clk_reset_clk_rate(struct dpu_clk_mgr *clk_mgr)
{

	struct dpu_clk_rate *clk_rate = g_dpu_reset_clk_rates;
	int i;

	for (i = 0; i < ARRAY_SIZE(g_dpu_reset_clk_rates); i++)
		dpu_clk_set_rate(clk_mgr, clk_rate[i].clk_type, clk_rate[i].rate);
}

int dpu_clk_enable(struct dpu_clk_mgr *clk_mgr, u32 clk_type)
{
	struct clk *clk;
	int ret = 0;

	clk = dpu_get_clk(clk_mgr, clk_type);
	if (clk) {
		ret = clk_prepare_enable(clk);
		if (ret)
			CLK_ERROR("failed to enable %s clk_type %u\n",
					dpu_get_clk_name(clk_mgr, clk_type),
					clk_type);
		else
			CLK_DEBUG("enable clk %s, clk_type is %u\n",
					dpu_get_clk_name(clk_mgr, clk_type),
					clk_type);
	}

	return ret;
}

void dpu_clk_disable(struct dpu_clk_mgr *clk_mgr, u32 clk_type)
{
	struct clk *clk;

	clk = dpu_get_clk(clk_mgr, clk_type);
	if (clk)
		clk_disable_unprepare(clk);

	CLK_DEBUG("disable clk %s, clk_type is %u\n",
			dpu_get_clk_name(clk_mgr, clk_type),
			clk_type);
}

void dpu_clk_ctrl(struct dpu_clk_mgr *clk_mgr, u32 clk_type, bool enable)
{
	if (enable)
		dpu_clk_enable(clk_mgr, clk_type);
	else
		dpu_clk_disable(clk_mgr, clk_type);
}

unsigned long dpu_clk_get_rate(struct dpu_clk_mgr *clk_mgr,
		u32 clk_type)
{
	unsigned long clk_rate = 0;
	struct clk *clk = dpu_get_clk(clk_mgr, clk_type);

	if (clk) {
		clk_rate = clk_get_rate(clk);
		CLK_DEBUG("get clk %s(clk_type %u) rate %lu\n",
				dpu_get_clk_name(clk_mgr, clk_type),
				clk_type,
				clk_rate);
	}

	return clk_rate;
}

static int _partition_clk_enable(struct dpu_clk_mgr *clk_mgr,
		u32 part_id)
{
	int ret;
	int i;

	for (i = 0; i < DPU_CLK_MAX_NUM; i++) {
		if (g_dpu_clks[i].part_id == part_id) {
			ret = dpu_clk_enable(clk_mgr, i);
			if (ret) {
				CLK_ERROR("failed to enable clk %s(clk_type %u)\n",
						dpu_get_clk_name(clk_mgr, i),
						i);
				goto end;
			}
		}
	}

	return 0;
end:
	i--;
	for (; i >= 0; i--) {
		if (g_dpu_clks[i].part_id == part_id)
			dpu_clk_disable(clk_mgr, i);
	}

	return ret;
}

int dpu_clk_enable_for_partition(struct dpu_clk_mgr *clk_mgr, u32 part_id)
{
	int ret = 0;

	if (!clk_mgr || part_id >= DPU_PARTITION_MAX_NUM) {
		CLK_ERROR("invalid parameter %pK, %u\n", clk_mgr, part_id);
		return -EINVAL;
	}

	CLK_DEBUG("part_id: %d, clk count %d\n",
			part_id, clk_mgr->clk_count[part_id]);

	mutex_lock(&clk_mgr->lock);
	clk_mgr->clk_count[part_id]++;
	if (clk_mgr->clk_count[part_id] == 1) {
		ret = _partition_clk_enable(clk_mgr, part_id);
		if (ret)
			clk_mgr->clk_count[part_id]--;
	}
	mutex_unlock(&clk_mgr->lock);

	return ret ? -1 : 0;
}

static void _partition_clk_disable(struct dpu_clk_mgr *clk_mgr, u32 part_id)
{
	u32 i;

	for (i = 0; i < DPU_CLK_MAX_NUM; i++)
		if (g_dpu_clks[i].part_id == part_id)
			dpu_clk_disable(clk_mgr, i);
}

void dpu_clk_disable_for_partition(struct dpu_clk_mgr *clk_mgr, u32 part_id)
{
	if (!clk_mgr || part_id >= DPU_PARTITION_MAX_NUM) {
		CLK_ERROR("invalid parameter %pK, %u\n", clk_mgr, part_id);
		return;
	}

	CLK_DEBUG("part_id: %d, clk count %d\n",
			part_id, clk_mgr->clk_count[part_id]);
	mutex_lock(&clk_mgr->lock);
	clk_mgr->clk_count[part_id]--;
	if (clk_mgr->clk_count[part_id] == 0)
		_partition_clk_disable(clk_mgr, part_id);
	mutex_unlock(&clk_mgr->lock);
}

int dpu_clk_set_debug(struct dpu_clk_mgr *clk_mgr, u32 clk_type,
		unsigned long rate)
{
	int ret;

	if (clk_type >= DPU_CLK_MAX_NUM) {
		CLK_ERROR("Error: invalid clk type %d\n", clk_type);
		return -1;
	}

	CLK_INFO("set clk %s(%d), rate %lu\n", dpu_get_clk_name(clk_mgr, clk_type),
			clk_type, rate);

	ret = dpu_clk_enable(clk_mgr, clk_type);
	if (ret) {
		CLK_ERROR("failed to enable clk %s(id %d)\n",
				dpu_get_clk_name(clk_mgr, clk_type),
				clk_type);
		return -1;
	}

	dpu_clk_set_rate(clk_mgr, clk_type, rate);

	dpu_clk_disable(clk_mgr, clk_type);

	return 0;
}

int dpu_clk_mgr_init(struct dpu_power_mgr *power_mgr)
{
	u32 i;
	struct dpu_clk_mgr *clk_mgr;

	if (!power_mgr) {
		CLK_ERROR("power_mgr is NULL\n");
		return -EINVAL;
	}

	clk_mgr = kzalloc(sizeof(*clk_mgr), GFP_KERNEL);
	if (!clk_mgr) {
		CLK_ERROR("alloc clk_mgr failed\n");
		return -ENOMEM;
	}

	for (i = 0; i < DPU_CLK_MAX_NUM; i++) {
		clk_mgr->dpu_clk[i] =
				devm_clk_get(power_mgr->drm_dev->dev,
						dpu_get_clk_name(clk_mgr, i));
		if (IS_ERR(clk_mgr->dpu_clk[i])) {
			CLK_ERROR("failed to get clk:%s\n", dpu_get_clk_name(clk_mgr, i));
			goto err;
		}
	}

	mutex_init(&clk_mgr->lock);
	power_mgr->clk_mgr = clk_mgr;
	return 0;
err:
	kfree(clk_mgr);
	return -EINVAL;
}

void dpu_clk_mgr_deinit(struct dpu_power_mgr *power_mgr)
{
	if (!power_mgr) {
		CLK_ERROR("power_mgr is NULL\n");
		return;
	}

	mutex_destroy(&power_mgr->clk_mgr->lock);
	kfree(power_mgr->clk_mgr);
	power_mgr->clk_mgr = NULL;
}
