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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/version.h>

#include "veu_drv.h"
#include "veu_defs.h"
#include "veu_utils.h"
#include "veu_res.h"
#include "veu_uapi.h"

#define DTS_VEU_NAME "xring,veu"

struct veu_data g_veu_data;
struct device *g_dma_dev;

static int veu_open(struct inode *inode, struct file *file)
{
	struct veu_data *priv = NULL;
	int ret;

	VEU_INFO("enter");
	veu_check_and_return(!file, -1, "file is null");
	veu_check_and_return(!inode, -1, "inode is null");

	priv = &g_veu_data;
	/* for other ioctls to get veu data */
	file->private_data = priv;
	mutex_lock(&priv->lock);
	if (!priv->power_count) {
		ret = priv->on(priv);
		if (ret) {
			mutex_unlock(&priv->lock);
			VEU_ERR("veu on fail");
			return -1;
		}
	}
	priv->power_count++;
	VEU_INFO("veu power count %d", priv->power_count);
	mutex_unlock(&priv->lock);
	VEU_INFO("exit");

	return 0;
}

static int veu_release(struct inode *inode, struct file *file)
{
	struct veu_data *priv = NULL;
	int ret;

	VEU_INFO("enter");
	veu_check_and_return(!file, -1, "file is null");
	veu_check_and_return(!inode, -1, "inode is null");

	priv = (struct veu_data *)(file->private_data);
	mutex_lock(&priv->lock);
	priv->power_count--;
	if (priv->power_count == 0) {
		ret = priv->off(priv);
		if (ret) {
			mutex_unlock(&priv->lock);
			VEU_ERR("veu off fail");
			return -1;
		}
	}
	VEU_INFO("veu power count %d", priv->power_count);
	mutex_unlock(&priv->lock);
	VEU_INFO("exit");

	return 0;
}

static long veu_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
	struct veu_data *priv = NULL;
	int ret = 0;

	VEU_DBG("enter");
	veu_check_and_return(!filp, -1, "filp is null");

	priv = (struct veu_data *)(filp->private_data);
	veu_check_and_return(!priv, -1, "priv is null");

	if (!mutex_trylock(&priv->lock)) {
		VEU_INFO("veu is occupied, return");
		return -1;
	}
	switch (cmd) {
	case VEU_PROCESS:
		ret = veu_process(filp, arg);
		if (ret)
			VEU_ERR("veu process fail");
		break;
	case VEU_SET_CLK_RATE:
		ret = veu_set_core_clk(filp, arg);
		if (ret)
			VEU_ERR("veu set core clk rate fail");
		break;
	default:
		VEU_ERR("unknown cmd, check it! cmd is %u", cmd);
		break;

	}
	mutex_unlock(&priv->lock);
	VEU_DBG("exit");

	return ret;
}

static const struct file_operations veu_fops = {
	.owner = THIS_MODULE,
	.open = veu_open,
	.release = veu_release,
	.compat_ioctl = veu_ioctl,
	.unlocked_ioctl = veu_ioctl,
};

static int veu_chrdev_register(struct veu_data *priv)
{
	int ret;

	VEU_INFO("enter");
	veu_check_and_return(!priv, -1, "priv is null");

	priv->chr_major = register_chrdev(0, VEU_DEV_NAME, &veu_fops);
	if (priv->chr_major < 0) {
		VEU_ERR("register chrdev fail");
		return -ENXIO;
	}

	priv->chr_class = class_create("veu");

	if (IS_ERR_OR_NULL(priv->chr_class)) {
		VEU_ERR("class create fail");
		ret = PTR_ERR(priv->chr_class);
		unregister_chrdev(priv->chr_major, VEU_DEV_NAME);
		return ret;
	}

	priv->chr_dev = device_create(
		priv->chr_class, 0, MKDEV(priv->chr_major, 0), NULL, VEU_DEV_NAME);
	if (IS_ERR_OR_NULL(priv->chr_dev)) {
		VEU_ERR("dev create fail");
		ret = PTR_ERR(priv->chr_dev);
		class_destroy(priv->chr_class);
		unregister_chrdev(priv->chr_major, VEU_DEV_NAME);
		return ret;
	}

	dev_set_drvdata(priv->chr_dev, priv);
	VEU_INFO("exit");

	return 0;
}

static int veu_chrdev_unregister(struct veu_data *priv)
{
	VEU_INFO("enter");
	veu_check_and_return(!priv, -1, "priv is null");

	if (priv->chr_major > 0) {
		device_destroy(priv->chr_class, MKDEV(priv->chr_major, 0));
		class_destroy(priv->chr_class);
		unregister_chrdev(priv->chr_major, VEU_DEV_NAME);
	}
	VEU_INFO("exit");

	return 0;
}

static void __iomem *veu_iomem_remap(struct platform_device *pdev, const char *name)
{
	void __iomem *iomem_ptr;
	struct resource *res;
	unsigned long size;

	if (!pdev || !name) {
		VEU_ERR("invalid parameters: %pK, %s", pdev, name);
		return ERR_PTR(-EINVAL);
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!res) {
		VEU_ERR("failed to get memory resource: %s", name);
		return ERR_PTR(-EINVAL);
	}

	size = resource_size(res);

	iomem_ptr = devm_ioremap(&pdev->dev, res->start, size);
	if (!iomem_ptr) {
		VEU_ERR("failed to ioremap: %s", name);
		return ERR_PTR(-ENOMEM);
	}

	return iomem_ptr;
}

static uint32_t veu_iomem_size(struct platform_device *pdev, const char *name)
{
	struct resource *res = NULL;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!res) {
		VEU_ERR("failed to get memory resource: %s", name);
		return 0;
	}

	return resource_size(res);
}

static int veu_parse_dts(struct platform_device *pdev)
{
	static const char *base_name = "veu_base";
	struct veu_data *priv = &g_veu_data;
	struct device_node *np = NULL;
	struct resource *res = NULL;
	void __iomem *iomem_ptr;
	uint32_t size;

	priv->of_dev = &pdev->dev;

	np = of_find_compatible_node(NULL, NULL, DTS_VEU_NAME);
	if (!np) {
		VEU_ERR("veu device node not found: %s", DTS_VEU_NAME);
		return -1;
	}

	iomem_ptr = veu_iomem_remap(pdev, base_name);
	if (IS_ERR_OR_NULL(iomem_ptr))
		return -ENXIO;

	size = veu_iomem_size(pdev, base_name);
	if (!size)
		return -ENXIO;

	res =  platform_get_resource_byname(pdev, IORESOURCE_MEM, base_name);

	priv->base = iomem_ptr;

	priv->addr = (uint32_t)res->start;

	priv->iomem_len = size;
	VEU_INFO("veu base 0x%pK, size 0x%x", priv->base, priv->iomem_len);

	priv->veu_irq = of_irq_get_byname(np, "veu_irq");
	veu_check_and_return(priv->veu_irq <= 0, -1, "fail to get veu irq");
	VEU_INFO("veu irq no is 0x%x", priv->veu_irq);

	return 0;
}

static int veu_probe(struct platform_device *pdev)
{
	struct veu_mmu_object *rdma_obj = NULL;
	struct veu_mmu_object *wdma_obj = NULL;
	struct veu_data *priv = NULL;
	int ret = 0;

	VEU_INFO("enter");

	veu_check_and_return(!pdev, -ENODEV, "pdev is null");

	g_dma_dev = &pdev->dev;
	priv = &g_veu_data;
	if (priv->device_initialized) {
		VEU_INFO("veu driver has already been initialized before");
		return 0;
	}

	memset(priv, 0, sizeof(*priv));

	// create character device for veu
	ret = veu_chrdev_register(priv);
	veu_check_and_return(ret, -EINVAL, "chrdev register fail");

	// parse dts config
	ret = veu_parse_dts(pdev);
	if (ret) {
		VEU_ERR("parse dts fail");
		goto parse_dts_fail;
	}

	veu_callback_register(priv);

	priv->pdev = pdev;
	platform_set_drvdata(pdev, priv);
	dev_set_drvdata(&(pdev->dev), priv);

	ret = veu_power_mgr_init(priv->of_dev, &priv->power_mgr);
	if (ret) {
		VEU_ERR("power mgr init fail");
		goto init_power_mgr_fail;
	}

	ret = veu_clk_mgr_init(priv->of_dev, &priv->clk_mgr);
	if (ret) {
		VEU_ERR("clk init fail");
		goto init_clk_mgr_fail;
	}

	priv->clk_rate_level = PROFILE_LVL_0;
	priv->clk_rate_lock_level = PROFILE_LVL_INVALID;

	ret = veu_init_isr(priv);
	if (ret) {
		VEU_ERR("init isr fail");
		goto init_isr_fail;
	}

	veu_mmu_mgr_ctx_init(priv->of_dev);

	rdma_obj = kzalloc(sizeof(*rdma_obj), GFP_KERNEL);
	if (!rdma_obj) {
		VEU_ERR("init rdma obj fail");
		goto alloc_obj_fail;
	}
	priv->rdma_obj = rdma_obj;

	wdma_obj = kzalloc(sizeof(*wdma_obj), GFP_KERNEL);
	if (!wdma_obj) {
		VEU_ERR("init wdma obj fail");
		kfree(priv->rdma_obj);
		priv->rdma_obj = NULL;
		goto alloc_obj_fail;
	}
	priv->wdma_obj = wdma_obj;

	priv->power_count = 0;
	priv->lock_power = false;
	mutex_init(&priv->lock);

	ret = dma_set_mask_and_coherent(priv->of_dev, DMA_BIT_MASK(64));
	if (ret != 0) {
		VEU_ERR("dma set mask and coherent failed %d!", ret);
		kfree(priv->rdma_obj);
		priv->rdma_obj = NULL;
		kfree(priv->wdma_obj);
		priv->wdma_obj = NULL;
		goto err_out;
	}

	veu_cmdlist_init(priv);

	priv->device_initialized = true;

	VEU_INFO("exit");

	return 0;

err_out:
alloc_obj_fail:
	veu_mmu_mgr_ctx_cleanup();
	veu_free_isr(priv);
init_isr_fail:
	veu_clk_mgr_deinit(priv->clk_mgr);
init_clk_mgr_fail:
	veu_power_mgr_deinit(priv->power_mgr);
init_power_mgr_fail:
parse_dts_fail:
	veu_chrdev_unregister(priv);
	mutex_destroy(&priv->lock);
	return -EINVAL;
}

static int veu_remove(struct platform_device *pdev)
{
	struct veu_data *priv = NULL;

	VEU_INFO("enter");
	priv = (struct veu_data *)platform_get_drvdata(pdev);
	veu_check_and_return(!pdev, -EINVAL, "priv is null");

	if (priv->device_initialized) {
		kfree(priv->rdma_obj);
		priv->rdma_obj = NULL;
		kfree(priv->wdma_obj);
		priv->wdma_obj = NULL;
		veu_mmu_mgr_ctx_cleanup();
		veu_cmdlist_deinit(priv);
		veu_free_isr(priv);
		veu_power_mgr_deinit(priv->power_mgr);
		veu_clk_mgr_deinit(priv->clk_mgr);
		veu_chrdev_unregister(priv);
		mutex_destroy(&priv->lock);
		priv->power_count = 0;
		priv->lock_power = false;
		priv->device_initialized = false;
	}
	VEU_INFO("exit");

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int veu_pm_suspend(struct device *dev)
{
	struct veu_data *priv = NULL;

	VEU_INFO("Enter");

	veu_check_and_return(!dev, -EINVAL, "dev is null");

	priv = dev_get_drvdata(dev);
	veu_check_and_return(!priv, -EINVAL, "priv is null");

	mutex_lock(&priv->lock);

	/*
	 * isp/vpu dont close veu during suspend, so
	 * power_count will remain, veu driver do power off
	 * and keep power_count as before
	 */
	VEU_INFO("veu power count %d", priv->power_count);
	if (priv->power_count > 0)
		priv->off(priv);

	mutex_unlock(&priv->lock);

	VEU_INFO("Exit");
	return 0;
}

static int veu_pm_resume(struct device *dev)
{
	struct veu_data *priv = NULL;

	VEU_INFO("Enter");

	veu_check_and_return(!dev, -EINVAL, "dev is null");

	priv = dev_get_drvdata(dev);
	veu_check_and_return(!priv, -EINVAL, "priv is null");

	mutex_lock(&priv->lock);

	/*
	 * isp/vpu may use veu as soon as system resume,
	 * so veu should do power on if power count is not zero,
	 * which means veu is being used before suspend
	 */
	VEU_INFO("veu power count %d", priv->power_count);
	if (priv->power_count > 0)
		priv->on(priv);

	mutex_unlock(&priv->lock);

	VEU_INFO("Exit");
	return 0;
}
#endif

static const struct dev_pm_ops veu_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(veu_pm_suspend, veu_pm_resume)
};

static const struct of_device_id of_match_veu[] = {
	{.compatible = "xring,veu", .data = NULL},
	{},
};

MODULE_DEVICE_TABLE(of, of_match_veu);

static struct platform_driver veu_driver = {
	.probe = veu_probe,
	.remove = veu_remove,
	.driver = {
		.name = "veu",
		.owner = THIS_MODULE,
		.of_match_table = of_match_veu,
		.pm = &veu_pm_ops,
	}
};

int veu_drv_register(void)
{
	int ret;

	VEU_INFO("enter");
	ret = platform_driver_register(&veu_driver);
	veu_check_and_return(ret, -EINVAL, "veu drv register fail");
	VEU_INFO("exit");

	return ret;
}

void veu_drv_unregister(void)
{
	VEU_INFO("enter");
	platform_driver_unregister(&veu_driver);
	VEU_INFO("exit");
}

