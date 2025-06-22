// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "cbm", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "cbm", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/dma-buf.h>
#include <linux/debugfs.h>
#include <linux/iommu.h>
#include <linux/bitmap.h>
#include <linux/mutex.h>
#include <linux/component.h>
#include <linux/of.h>
#include <linux/types.h>
#include <asm/errno.h>
#include <linux/err.h>
#include <linux/delay.h>
#include "xrisp_cbm.h"
#include "xrisp_cbm_api.h"
#include "xrisp_log.h"

struct cam_buf_manager g_cam_buf_mgr;

static int cbm_iommu_init(void)
{
	int ret = 0;
	struct device *dev = g_cam_buf_mgr.dev;

	g_cam_buf_mgr.i_group = iommu_group_get(dev);
	if (IS_ERR_OR_NULL(g_cam_buf_mgr.i_group)) {
		ret = PTR_ERR(g_cam_buf_mgr.i_group);
		XRISP_DEV_ERROR(dev, "get iommu group fail ret = %d(%pe)",
			      ret, g_cam_buf_mgr.i_group);
		g_cam_buf_mgr.i_group = NULL;
		return -EINVAL;
	}
	g_cam_buf_mgr.i_domain = iommu_get_domain_for_dev(dev);
	if (IS_ERR_OR_NULL(g_cam_buf_mgr.i_domain)) {
		ret = PTR_ERR(g_cam_buf_mgr.i_domain);
		XRISP_DEV_ERROR(dev, "get iommu domain fail ret = %d(%pe)",
			      ret, g_cam_buf_mgr.i_domain);
		g_cam_buf_mgr.i_domain = NULL;
		return -EINVAL;
	}

	return 0;
}

static void cbm_iommu_exit(void)
{

}

inline bool cbm_avlid(void)
{
	return g_cam_buf_mgr.avalid;
}

static int xrisp_cbm_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;

	XRISP_DEV_INFO(dev, "probe start");

	g_cam_buf_mgr.dev = dev;
	platform_set_drvdata(pdev, &g_cam_buf_mgr);

	ret = cbm_cbt_init();
	if (ret)
		goto exit;

	ret = dma_set_mask_and_coherent(dev, DMA_BIT_MASK(64));

	if (ret) {
		XRISP_DEV_ERROR(dev, "set dma mask fail ret = %d", ret);
		goto cbt_exit;
	}

	ret = cbm_iommu_init();
	if (ret)
		goto cbt_exit;

	ret = cbm_iova_region_init();
	if (ret)
		goto iommu_exit;

	ret = cbm_debugfs_init();
	if (ret)
		goto iova_region_exit;

	g_cam_buf_mgr.avalid = true;

	XRISP_DEV_INFO(dev, "probe done");

	return 0;

iova_region_exit:
	cbm_iova_region_exit();
iommu_exit:
	cbm_iommu_exit();
cbt_exit:
	cbm_cbt_exit();
exit:
	return ret;
}

static int xrisp_cbm_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	XRISP_DEV_INFO(dev, "remove start");

	cbm_buf_release_all();
	cbm_iova_region_exit();
	cbm_iommu_exit();
	cbm_cbt_exit();
	cbm_debugfs_exit();
	g_cam_buf_mgr.avalid = false;

	XRISP_DEV_INFO(dev, "remove done");

	return 0;
}

static const struct of_device_id cbm_match_table[] = {
	{ .compatible = "xring,cam-buf-manager", },
	{}
};

MODULE_DEVICE_TABLE(of, cbm_match_table);

static struct platform_driver xrisp_cbm_driver = {
	.probe = xrisp_cbm_probe,
	.remove = xrisp_cbm_remove,
	.driver = {
		.name = "cam-buf-manager",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(cbm_match_table),
	},
};

int xrisp_cbm_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&xrisp_cbm_driver);
	if (ret)
		XRISP_PR_ERROR("register driver failed");

	return ret;
}

void xrisp_cbm_exit(void)
{
	platform_driver_unregister(&xrisp_cbm_driver);
}


MODULE_AUTHOR("Zhen Bin<zhenbin@xiaomi.com>");
MODULE_DESCRIPTION("x-ring camera buf manager");
MODULE_LICENSE("GPL v2");
