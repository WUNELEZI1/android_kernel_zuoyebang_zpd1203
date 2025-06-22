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

#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/memblock.h>

#include "dpu_cont_display.h"
#include "dsi_connector.h"
#include "dpu_power_mgr.h"
#include "dsi_power.h"
#include "dpu_log.h"

#define SAFE_FRAME_COUNT	4

static u32 g_cont_display_ctrl = 1;

void dpu_cont_display_status_get(struct drm_device *drm_dev)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct platform_device *pdev;
	struct device_node *np;
	int ret;

	if (!drm_dev) {
		DPU_ERROR("invalid parameters: %pK\n", drm_dev);
		return;
	}

	pdev = to_platform_device(drm_dev->dev);
	np = pdev->dev.of_node;

	dpu_drm_dev = to_dpu_drm_dev(drm_dev);

	if (g_cont_display_ctrl) {
		ret = of_property_read_u32(np, "fastboot-display-continuous", &g_cont_display_ctrl);
		if (ret)
			DPU_ERROR("failed to read fastboot-display-continuous!\n");
	}

	DPU_INFO("get fastboot-display-continuous:%d\n", g_cont_display_ctrl);
}

int dpu_cont_display_init(struct drm_device *drm_dev)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct drm_connector *drm_conn;
	struct dpu_kms *dpu_kms;

	if (!drm_dev) {
		DPU_ERROR("invalid parameters: %pK\n", drm_dev);
		return -EINVAL;
	}

	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	drm_conn = dsi_primary_connector_get(drm_dev);
	if (drm_conn == NULL) {
		DPU_ERROR("drm connector is NULL\n");
		return -EINVAL;
	}

	if (is_cont_display_enabled()) {
		DPU_INFO("cont display enabled\n");
		dsi_panel_power_get(drm_conn);
		dsi_power_get(drm_conn);
		dpu_power_get_helper(DPU_ALL_PARTITIONS);
		dpu_idle_enable_ctrl(false);
	}

	return 0;
}

bool is_cont_display_enabled(void)
{
	return g_cont_display_ctrl;
}

static void dpu_cont_display_disable(void)
{
	g_cont_display_ctrl = 0;
}

static void release_logo_buffer(void)
{
	phys_addr_t base_addr = 0;
	phys_addr_t rmem_size = 0;
	unsigned long long reg[2];
	struct device_node *np;
	uint32_t i, page_count;
	struct page *page;
	int ret;

	np = of_find_node_by_path("/reserved-memory/rsv_mem_dpu_logo@20000000");
	if (IS_ERR(np)) {
		DPU_ERROR("failed to get reserved-memory\n");
		return;
	}

	ret = of_property_read_u64_array(np, "reg", reg, 2);
	if (ret) {
		DPU_ERROR("failed to read reg property\n");
		return;
	}

	base_addr = reg[0];
	rmem_size = reg[1];

	memblock_free(__va(base_addr), rmem_size);

	page_count = rmem_size / PAGE_SIZE;
	for (i = 0; i < page_count; i++) {
		page = phys_to_page(base_addr);
		free_reserved_page(page);
		base_addr += PAGE_SIZE;
	}

	DPU_DEBUG("logo buffer is released\n");
}

static void _release_cont_display_res(struct dpu_power_mgr *power_mgr)
{
	struct drm_connector *drm_conn;

	drm_conn = dsi_primary_connector_get(power_mgr->drm_dev);
	if (drm_conn == NULL) {
		DPU_ERROR("drm connector is NULL\n");
		return;
	}

	dpu_cont_display_disable();
	dpu_power_put_helper(DPU_ALL_PARTITIONS);
	dsi_power_put(drm_conn);
	dsi_panel_power_put(drm_conn);
	release_logo_buffer();
	dpu_idle_enable_ctrl(true);
}

void dpu_cont_display_res_release(struct dpu_power_mgr *power_mgr)
{
	static u32 frame_count;

	if (!power_mgr) {
		DPU_ERROR("invalid params\n");
		return;
	}

	if (!is_cont_display_enabled())
		return;

	DPU_INFO("frame_count: %d\n", frame_count);

	if (++frame_count >= SAFE_FRAME_COUNT) {
		DPU_INFO("release cont display res\n");
		_release_cont_display_res(power_mgr);
	}
}

void dpu_cont_display_res_force_release(struct dpu_power_mgr *power_mgr)
{
	if (!power_mgr) {
		DPU_ERROR("invalid params\n");
		return;
	}

	if (!is_cont_display_enabled())
		return;

	DPU_INFO("force release cont display res\n");
	_release_cont_display_res(power_mgr);
}
