// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "power_sync", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "power_sync", __func__, __LINE__

#include <linux/slab.h>
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <asm/io.h>
#include "xrisp_log.h"
#include "xrisp_sensor_dev.h"
#include "xrisp_sensor_utils.h"

struct cam_power_sync_info *g_sync_info;

struct cam_power_sync_info *xrisp_power_sync_get_context(void)
{
	return g_sync_info;
}

/* maskbit means: bit0: AP_power, bit1: SH_power, bit3: Front_cam */
static int xrisp_power_sync_reg_set(struct cam_power_sync_info *sync_info,
	uint8_t status, uint8_t maskbit)
{
	uint32_t  val;

	if (XRISP_CHECK_NULL_RETURN_INT(sync_info)) {
		XRISP_PR_ERROR("sync_info is null\n");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(sync_info->baseaddr)) {
		XRISP_PR_ERROR("sync_info->baseaddr is null\n");
		return -ENOMEM;
	}

	val = status & (1UL << maskbit);
	val |= ((1UL << maskbit) << XRISP_AP_CAM_ACTRL_BMRW_SHIFT);
	writel(val, sync_info->baseaddr);
	XRISP_PR_INFO("set ap camera power status =[0x%x]\n", val);

	return 0;
}

int xrisp_power_sync_get(uint8_t maskbit)
{


	struct cam_power_sync_info *sync_info;

	sync_info = xrisp_power_sync_get_context();
	if (XRISP_CHECK_NULL_RETURN_INT(sync_info)) {
		XRISP_PR_ERROR("sync_info is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(sync_info->baseaddr)) {
		XRISP_PR_ERROR("sync_info->baseaddr is null");
		return -ENOMEM;
	}

	XRISP_PR_INFO("get ap camera power[%ld] status:0x%lx.",
		(1UL << maskbit),
		readl(sync_info->baseaddr) & (1UL << maskbit));

	return (readl(sync_info->baseaddr) & (1UL << maskbit));
}

int xrisp_power_sync_set(uint8_t status, uint8_t bitmask)
{
	int ret = 0;
	struct cam_power_sync_info *sync_info;

	sync_info = xrisp_power_sync_get_context();
	if (XRISP_CHECK_NULL_RETURN_INT(sync_info)) {
		XRISP_PR_ERROR("sync_info is null");
		return -ENOMEM;
	}

	mutex_lock(&sync_info->mlock);
	if (atomic_read(&sync_info->power_cnt[bitmask])) {
		atomic_inc(&sync_info->power_cnt[bitmask]);
		XRISP_PR_INFO("ap camera power already set count:%d.",
			atomic_read(&sync_info->power_cnt[bitmask]));
		mutex_unlock(&sync_info->mlock);
		return 0;
	}

	ret = xrisp_power_sync_reg_set(sync_info, status, bitmask);
	if (ret) {
		XRISP_PR_ERROR("Set ap camera power status failed.\n");
		goto sync_unlock;
	}

	atomic_inc(&sync_info->power_cnt[bitmask]);
	mutex_unlock(&sync_info->mlock);

	return ret;

sync_unlock:
	mutex_unlock(&sync_info->mlock);

	return ret;
}

int xrisp_power_sync_clear(uint8_t bitmask)
{
	int ret = 0;
	struct cam_power_sync_info *sync_info;

	sync_info = xrisp_power_sync_get_context();
	if (XRISP_CHECK_NULL_RETURN_INT(sync_info)) {
		XRISP_PR_ERROR("sync_info is null");
		return -ENOMEM;
	}

	mutex_lock(&sync_info->mlock);
	if (atomic_dec_return(&sync_info->power_cnt[bitmask])) {
		if (atomic_read(&sync_info->power_cnt[bitmask]) < 0)
			atomic_set(&sync_info->power_cnt[bitmask], 0);

		XRISP_PR_INFO("ap camera power count:%d.", atomic_read(&sync_info->power_cnt[bitmask]));
		mutex_unlock(&sync_info->mlock);
		return 0;
	}

	ret = xrisp_power_sync_reg_set(sync_info, XRISP_AP_CAM_POWER_STATUS_IDLE, bitmask);
	if (ret)
		XRISP_PR_ERROR("Clear ap camera power status failed, ret:%d", ret);

	mutex_unlock(&sync_info->mlock);

	return ret;
}

int xrisp_power_sync_clear_force(uint8_t bitmask)
{
	int ret = 0;
	struct cam_power_sync_info *sync_info;

	sync_info = xrisp_power_sync_get_context();
	if (XRISP_CHECK_NULL_RETURN_INT(sync_info)) {
		XRISP_PR_ERROR("sync_info is null");
		return -ENOMEM;
	}

	mutex_lock(&sync_info->mlock);
	if (atomic_read(&sync_info->power_cnt[bitmask]) > 0) {
		ret = xrisp_power_sync_reg_set(sync_info, XRISP_AP_CAM_POWER_STATUS_IDLE, bitmask);
		if (ret)
			XRISP_PR_ERROR("Force clear ap camera power status failed, ret:%d", ret);

		XRISP_PR_INFO("Force clear ap camera power status count:%d.",
			atomic_read(&sync_info->power_cnt[bitmask]));
		atomic_set(&sync_info->power_cnt[bitmask], 0);
	}
	mutex_unlock(&sync_info->mlock);

	return 0;
}

int xrisp_power_sync_init(struct device *dev)
{
	struct cam_power_sync_info *sync_info;

	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}

	sync_info = devm_kzalloc(dev, sizeof(struct cam_power_sync_info), GFP_KERNEL);
	if (unlikely(sync_info == NULL)) {
		XRISP_PR_ERROR("alloc sync info failed, no memory!");
		return -ENOMEM;
	}

	sync_info->dev      = dev;
	sync_info->baseaddr = ioremap(XRISP_AP_CAM_POWER_STATUS_BASE,
		XRISP_AP_CAM_POWER_STATUS_SIZE);
	if (unlikely(sync_info->baseaddr == NULL)) {
		XRISP_PR_ERROR("aoc reg iomap failed");
		return -ENOMEM;
	}

	g_sync_info = sync_info;
	mutex_init(&sync_info->mlock);
	for (int i = 0; i < XRISP_AOC_POWER_SYNC_BIT_MAX; i++)
		atomic_set(&sync_info->power_cnt[i], 0);

	/* clear camera power status */
	xrisp_power_sync_reg_set(sync_info, XRISP_AP_CAM_POWER_STATUS_IDLE,
	XRISP_AOC_AP_POWER_MASKBIT);
	xrisp_power_sync_reg_set(sync_info, XRISP_AP_CAM_POWER_STATUS_IDLE,
	XRISP_AOC_AP_FRONT_MASKBIT);

	return 0;
}

int xrisp_power_sync_exit(void)
{
	struct cam_power_sync_info *sync_info;

	sync_info = xrisp_power_sync_get_context();
	if (XRISP_CHECK_NULL_RETURN_INT(sync_info)) {
		XRISP_PR_ERROR("sync_info is null");
		return -ENOMEM;
	}

	if (likely(sync_info->baseaddr))
		iounmap(sync_info->baseaddr);

	return 0;
};
