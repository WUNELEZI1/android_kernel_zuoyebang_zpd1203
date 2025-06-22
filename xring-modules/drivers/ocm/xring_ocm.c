// SPDX-License-Identifier: GPL-2.0-or-later
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

#include <linux/miscdevice.h>
#include <linux/compat.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/virtio.h>
#include <linux/mm.h>
#include <linux/genalloc.h>
#include <linux/of.h>

#include "xring_ocm_internal.h"

#define OCM_ALLOC _IOWR('O', 1, struct ocm_buf_info)
#define OCM_FREQ  _IOWR('O', 2, enum ocm_clk_rate_e)
#define OCM_ACPU_ADDR                     0xE9820000

#define DPU_USED_ADDR (0x840000000)
#define DPU_USED_SIZE (4 * 1024 * 1024)

static const char *ocm_rate_prop_name[OCM_CLK_RATE_MAX] = {
	[OCM_CLK_RATE_080V] = "ocm-clock-rate-080",
	[OCM_CLK_RATE_070V] = "ocm-clock-rate-070",
	[OCM_CLK_RATE_065V] = "ocm-clock-rate-065",
	[OCM_CLK_RATE_060V] = "ocm-clock-rate-060",
};

enum ocm_clk_rate_e ocm_clk_value = OCM_CLK_RATE_060V;

struct ocm_clk_info_t {
	const char *clk_name;
	unsigned int *clk_init_rate;
	unsigned int clk_rate;
	struct clk *clk_p;
	unsigned int clk_enable;
	struct mutex clk_lock;
};

struct ocm_region {
	struct ocm_buffer buf;
	int ref_count;
	struct list_head node;
};

struct ocm_mem_heap {
	u64 base_addr;
	size_t size;
	struct gen_pool *pool;
	struct list_head head;
};

struct ocm_cdev {
	struct platform_device *pdev;
	struct ocm_mem_heap omp;
	struct idr ids_head;
	struct resource *reg;
	struct regulator *regulator;
	int boot_cnt;
	struct ocm_clk_info_t clk_info;
	struct miscdevice miscdev;
	struct mutex boot_lock;
};

static struct ocm_cdev ocmcdev = {0};
static struct wakeup_source *g_npu_sys_pm_ocm_wksrc;

static bool is_wksrc_active(void)
{
	if ((g_npu_sys_pm_ocm_wksrc != NULL) && (g_npu_sys_pm_ocm_wksrc->active))
		return true;

	ocmdbg("g_npu_sys_pm_ocm_wksrc->active is false\n");
	return false;
}



int ocm_clk_set_rate(enum ocm_clk_rate_e rate_e)
{
	int ret = 0;
	struct ocm_clk_info_t *clk_info_p = NULL;

	clk_info_p = &ocmcdev.clk_info;
	if (rate_e >= OCM_CLK_RATE_MAX) {
		ret = -EINVAL;
		ocmerr("ocm_clk_rate_e set error, rate_e : %d", rate_e);
		return ret;
	}
	if (IS_ERR(clk_info_p->clk_p) || clk_info_p->clk_p == NULL) {
		ret = -ENODEV;
		ocmerr("ocm clk ptr is error");
		return ret;
	}
	if (clk_info_p->clk_enable == 0) {
		ocm_clk_value = rate_e;
		ocminfo("clk_info_p is disable!!!");
		ocminfo("save ocm_clk_value: %d", ocm_clk_value);
	} else {
		clk_info_p->clk_rate = clk_info_p->clk_init_rate[rate_e];
		ret = clk_set_rate(clk_info_p->clk_p, (unsigned long)clk_info_p->clk_rate);
		if (ret) {
			ocmerr("set clk rate fail");
		} else {
			clk_info_p->clk_rate = clk_get_rate(clk_info_p->clk_p);
			ocminfo("set clk_info_p->clk_rate %u", clk_info_p->clk_rate);
		}
	}
	return ret;
}
EXPORT_SYMBOL_GPL(ocm_clk_set_rate);

unsigned int ocm_clk_get_rate(void)
{
	struct ocm_clk_info_t *clk_info_p = NULL;

	clk_info_p = &ocmcdev.clk_info;
	if (IS_ERR(clk_info_p->clk_p)) {
		ocmerr("clk_info is null");
		return 0;
	}
	if (clk_info_p->clk_enable == 0) {
		ocmerr("clk_info is disable");
		return 0;
	}
	clk_info_p->clk_rate = clk_get_rate(clk_info_p->clk_p);
	return clk_info_p->clk_rate;
}
EXPORT_SYMBOL_GPL(ocm_clk_get_rate);

int ocm_clk_get_stats(void)
{
	int ret = 0;
	struct ocm_clk_info_t *clk_info_p = &ocmcdev.clk_info;

	if (!IS_ERR(clk_info_p->clk_p) && clk_info_p->clk_enable) {
		ocmerr("ocm clk disable, please alloc ocm buf");
		ret = -ENODEV;
	}
	ocminfo("ocm clk status OK");
	return ret;
}
EXPORT_SYMBOL_GPL(ocm_clk_get_stats);

static int ocm_clk_enable(void)
{
	int ret = 0;
	struct ocm_clk_info_t *clk_info_p = &ocmcdev.clk_info;
	struct device *dev = &(ocmcdev.pdev->dev);

	if (clk_info_p->clk_p == NULL) {
		clk_info_p->clk_p = devm_clk_get(dev, clk_info_p->clk_name);
		ocminfo("ocm clk-name :%s", clk_info_p->clk_name);
	}
	if (IS_ERR(clk_info_p->clk_p)) {
		ocmerr("clp_p = %p", clk_info_p->clk_p);
		return -ENODEV;
	}
	mutex_lock(&clk_info_p->clk_lock);
	if (clk_info_p->clk_enable > 0) {
		clk_info_p->clk_enable++;
	} else {
		ret = clk_prepare_enable(clk_info_p->clk_p);
		if (ret) {
			ocmerr("enable clk fail");
			mutex_unlock(&clk_info_p->clk_lock);
			return ret;
		}
		clk_info_p->clk_enable = 1;
	}
	mutex_unlock(&clk_info_p->clk_lock);
	return ret;
}

static int ocm_clk_disable(void)
{
	int ret = 0;
	struct ocm_clk_info_t *clk_info_p = &ocmcdev.clk_info;

	mutex_lock(&clk_info_p->clk_lock);
	clk_info_p->clk_enable--;
	if (clk_info_p->clk_enable == 0) {
		clk_disable_unprepare(clk_info_p->clk_p);
		ocminfo("disable ocm_clk");
	}
	mutex_unlock(&clk_info_p->clk_lock);
	return ret;
}

static int ocm_clk_init(struct device *dev)
{
	int i;
	int ret = 0;
	struct ocm_clk_info_t *clk_info = NULL;
	struct device_node *np = dev->of_node;
	unsigned int *clk_init_rate =
		devm_kzalloc(dev, sizeof(unsigned int) * OCM_CLK_RATE_MAX, GFP_KERNEL);

	clk_info = &ocmcdev.clk_info;
	if (clk_init_rate == NULL)
		return -ENOMEM;

	ret = of_property_read_string(np, "clock-names", &clk_info->clk_name);
	if (ret != 0)
		return -EINVAL;
	ocminfo("clk_info->clk_name:%s\n", clk_info->clk_name);
	for (i = 0; i < OCM_CLK_RATE_MAX; i++) {
		ret = of_property_read_u32(np, ocm_rate_prop_name[i], &clk_init_rate[i]);
		if (ret != 0)
			return -EINVAL;
		ocminfo("clk_info->clk_init_rate[%d]: %d", i, clk_init_rate[i]);
	}
	clk_info->clk_init_rate = clk_init_rate;
	return 0;
}

struct ocm_buffer *ocm_buf_get(int fd)
{
	struct ocm_region *oregion = NULL;

	oregion = idr_find(&ocmcdev.ids_head, fd);
	if (oregion == NULL) {
		ocmerr("fd:%d is not in ocm\n", fd);
		return NULL;
	}
	oregion->ref_count++;
	ocminfo("fd: %d,ref_count: %d", fd, oregion->ref_count);
	return &oregion->buf;
}
EXPORT_SYMBOL_GPL(ocm_buf_get);

int npu_ocm_free(struct ocm_buffer *buf)  //NPU
{
	int ret = 0;
	bool is_in_pool;
	struct ocm_mem_heap *omp = &ocmcdev.omp;

	if (mutex_lock_interruptible(&ocmcdev.boot_lock))
		return -EFAULT;
	is_in_pool = gen_pool_has_addr(omp->pool,
				(unsigned long)buf->addr, buf->size);
	if (!is_in_pool) {
		ocminfo("buf is not in pool!\n");
		return -EFAULT;
	}

	gen_pool_free(omp->pool, (unsigned long)buf->addr, buf->size);
	ocmcdev.boot_cnt--;
	if (ocmcdev.boot_cnt == 0) {
		ocm_clk_value = OCM_CLK_RATE_060V;
		if (regulator_is_enabled(ocmcdev.regulator)) {
			ret = ocm_clk_disable();
			if (ret != 0)
				ocmerr("ocm_clk_disable failed:%d\n", ret);
			ret = regulator_disable(ocmcdev.regulator);
			if (ret != 0) {
				ocmerr("regulator_disable failed:%d\n", ret);
				return ret;
			}
			if (is_wksrc_active()) {
				__pm_relax(g_npu_sys_pm_ocm_wksrc);
				ocmerr("__pm_relax.g_npu_sys_pm_ocm_wksrc end\n");
			}
			ocminfo("power off\n");
		}
	}
	mutex_unlock(&ocmcdev.boot_lock);
	return ret;
}

void ocm_buf_put(int fd)
{
	int ret = 0;
	struct ocm_region *oregion = NULL;
	struct ocm_buffer *buf = NULL;

	oregion = idr_find(&ocmcdev.ids_head, fd);
	if (oregion == NULL) {
		ocmerr("fd:%d is not in ocm\n", fd);
		return;
	}
	oregion->ref_count--;
	ocminfo("fd: %d, ref_conut: %d", fd, oregion->ref_count);
	if (oregion->ref_count == 0) {
		buf = &oregion->buf;
		ret = ocm_buf_clr_attr(buf, XRING_NPU);
		if (ret != 0)
			ocmerr("ocm buf clear attr failed: %d\n", ret);
		ret = npu_ocm_free(buf);
		if (ret != 0)
			ocmerr("ocm_free failed: %d", ret);
		idr_remove(&ocmcdev.ids_head, fd);
		list_del(&oregion->node);
		kfree(oregion);
		oregion = NULL;
	}
}
EXPORT_SYMBOL_GPL(ocm_buf_put);

unsigned int ocm_query_avail_size(void)
{
	return DPU_USED_SIZE;
}
EXPORT_SYMBOL_GPL(ocm_query_avail_size);

int npu_ocm_alloc(struct ocm_buffer *buf)  //NPU 申请
{
	int ret = 0;
	struct ocm_mem_heap *omp = &ocmcdev.omp;

	if (mutex_lock_interruptible(&ocmcdev.boot_lock))
		return -EFAULT;
	if (ocmcdev.boot_cnt == 0) {
		if (!is_wksrc_active()) {
			__pm_stay_awake(g_npu_sys_pm_ocm_wksrc);
			ocmdbg("__pm_stay_awake.g_npu_sys_pm_ocm_wksrc end\n");
		}
		ret = regulator_enable(ocmcdev.regulator);
		if (ret != 0) {
			ocmerr("regulator_enable failed:%d\n", ret);
			goto power_off;
		}
		ocminfo("power on\n");
		ret = ocm_clk_enable();
		if (ret != 0) {
			ocmerr("ocm_clk_enable failed:%d\n", ret);
			goto close_clock;
		}
		ret = ocm_clk_set_rate(ocm_clk_value);
		if (ret != 0) {
			ocmerr("ocm_clk_set_rate failed:%d\n", ret);
			goto close_clock;
		}
	}
	buf->addr = gen_pool_alloc(omp->pool, buf->size);
	if (buf->addr == 0) {
		ocminfo("gen_pool_alloc failed! buf->size: %x\n", buf->size);
		ret = ocm_clk_disable();
		if (ret != 0)
			ocmerr("ocm_clk_disable failed:%d\n", ret);
		goto close_clock;
	}
	ocmcdev.boot_cnt++;
	mutex_unlock(&ocmcdev.boot_lock);
	return ret;

close_clock:
	ret = ocm_clk_disable();
	if (ret != 0)
		ocmerr("ocm_clk_disable failed:%d\n", ret);
power_off:
	if (ocmcdev.boot_cnt == 0) {
		regulator_disable(ocmcdev.regulator);
		if (is_wksrc_active()) {
			__pm_relax(g_npu_sys_pm_ocm_wksrc);
			ocmdbg("__pm_relax.g_npu_sys_pm_ocm_wksrc end\n");
		}
	}
	mutex_unlock(&ocmcdev.boot_lock);
	return -EFAULT;
}

int ocm_alloc(struct ocm_buffer *buf)  //DPU 申请
{
	int ret = 0;

	if (mutex_lock_interruptible(&ocmcdev.boot_lock))
		return -EFAULT;

	if (ocmcdev.boot_cnt == 0) {
		if (!is_wksrc_active()) {
			__pm_stay_awake(g_npu_sys_pm_ocm_wksrc);
			ocmerr("__pm_stay_awake.g_npu_sys_pm_ocm_wksrc end\n");
		}
		ret = regulator_enable(ocmcdev.regulator);
		if (ret != 0) {
			ocmerr("regulator_enable failed:%d\n", ret);
			goto power_off;
		}
		ocminfo("power on\n");
		ret = ocm_clk_enable();
		if (ret != 0) {
			ocmerr("ocm_clk_enable failed:%d\n", ret);
			goto close_clock;
		}
		ret = ocm_clk_set_rate(ocm_clk_value);
		if (ret != 0) {
			ocmerr("ocm_clk_set_rate failed:%d\n", ret);
			goto close_clock;
		}
	}
	buf->addr = DPU_USED_ADDR;
	ocmcdev.boot_cnt++;
	mutex_unlock(&ocmcdev.boot_lock);
	return ret;

close_clock:
	ret = ocm_clk_disable();
	if (ret != 0)
		ocmerr("ocm_clk_disable failed:%d\n", ret);

power_off:
	if (ocmcdev.boot_cnt == 0) {
		regulator_disable(ocmcdev.regulator);
		if (is_wksrc_active()) {
			__pm_relax(g_npu_sys_pm_ocm_wksrc);
			ocmdbg("__pm_relax.g_npu_sys_pm_ocm_wksrc end\n");
		}
	}
	mutex_unlock(&ocmcdev.boot_lock);
	return -EFAULT;
}
EXPORT_SYMBOL_GPL(ocm_alloc);

static int ocm_buf_alloc(struct ocm_buf_info *info)  //NPU  申请
{
	int ret = 0;
	struct ocm_region *oregion = kmalloc(sizeof(*oregion), GFP_KERNEL);
	struct ocm_mem_heap *omp = &ocmcdev.omp;
	struct list_head *head = &omp->head;
	struct ocm_buffer *buf = &oregion->buf;

	buf->size = info->size;
	ret = npu_ocm_alloc(buf);
	if (ret != 0)
		goto error;

	info->fd = idr_alloc(&ocmcdev.ids_head, oregion, 1, 16, GFP_KERNEL);
	if (info->fd < 0) {
		ocminfo("idr_alloc fail,info->fd : %d\n", info->fd);
		goto error;
	}
	ocminfo("info->fd : %d\n", info->fd);
	list_add(&oregion->node, head);
	return ret;
error:
	kfree(oregion);
	if (is_wksrc_active()) {
		__pm_relax(g_npu_sys_pm_ocm_wksrc);
		ocmdbg("__pm_relax.g_npu_sys_pm_ocm_wksrc end\n");
	}
	return -EFAULT;
}

int ocm_free(struct ocm_buffer *buf)  //DPU free
{
	int ret = 0;

	if (mutex_lock_interruptible(&ocmcdev.boot_lock))
		return -EFAULT;

	ocmcdev.boot_cnt--;
	if (ocmcdev.boot_cnt == 0) {
		ocm_clk_value = OCM_CLK_RATE_060V;
		if (regulator_is_enabled(ocmcdev.regulator)) {
			ret = ocm_clk_disable();
			if (ret != 0)
				ocmerr("ocm_clk_disable failed:%d\n", ret);

			ret = regulator_disable(ocmcdev.regulator);
			if (ret != 0)
				ocmerr("regulator_disable failed:%d\n", ret);

			if (is_wksrc_active()) {
				__pm_relax(g_npu_sys_pm_ocm_wksrc);
				ocmerr("__pm_relax.g_npu_sys_pm_ocm_wksrc end\n");
			}
			ocminfo("power off\n");
		}
	}
	ocmdbg("end\n");
	mutex_unlock(&ocmcdev.boot_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(ocm_free);

static int ocm_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	return ret;
}

static int ocm_release(struct inode *inode, struct file *filp)
{
	int ret = 0;

	return ret;
}

static long ocm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	struct ocm_buf_info info = {0};

	switch (cmd) {
	case OCM_ALLOC:
		ocmdbg("alloc star\n");
		ret = copy_from_user(&info, argp, sizeof(info));
		if (ret < 0)
			ocmerr("copy_from_user for alloc info failed:%d\n", ret);
		ret = ocm_buf_alloc(&info);
		if (ret != 0)
			ocmerr("com alloc mem failed:%d\n", ret);
		ret = copy_to_user(argp, &info, sizeof(info));
		if (ret < 0)
			ocmerr("copy_to_user for alloc info failed:%d\n", ret);
		ocmdbg("alloc end\n");
	break;
	case OCM_FREQ:
		ocmdbg("OCM_FREQ star\n");
		ret = copy_from_user(&ocm_clk_value, argp, sizeof(ocm_clk_value));
		if (ret != 0)
			ocmerr("copy_from_user for OCM_FREQ info failed:%d\n", ret);
		if (regulator_is_enabled(ocmcdev.regulator)) {
			ret = ocm_clk_set_rate(ocm_clk_value);
			if (ret != 0)
				ocmerr("OCM_FREQ setting failed:%d\n", ret);
		} else {
			ocminfo("save ocm_clk_value: %d\n", ocm_clk_value);
		}
		ocmdbg("OCM_FREQ end\n");
	break;
	default:
		ocmerr("Unsupported ioctl!!!\n");
		return -ENOTTY;
	}
	return ret;
}

static const struct file_operations ocm_fops = {
	.owner = THIS_MODULE,
	.open = ocm_open,
	.release = ocm_release,
	.unlocked_ioctl = ocm_ioctl,
};

static int ocmcdev_miscdev_init(struct ocm_cdev *ocmcdev)
{
	int ret = 0;
	struct miscdevice *miscdev = &ocmcdev->miscdev;

	miscdev->parent = &ocmcdev->pdev->dev;
	miscdev->minor = MISC_DYNAMIC_MINOR;
	miscdev->name = "ocm-buf";
	miscdev->fops = &ocm_fops;
	ret = misc_register(miscdev);
	if (ret != 0)
		ocmerr("register ocm cdev failed: %d\n", ret);
	return ret;
}

static void ocmcdev_miscdev_destroy(struct ocm_cdev *ocmcdev)
{
	misc_deregister(&ocmcdev->miscdev);
}

static int ocm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ocm_mem_heap *omp = &ocmcdev.omp;
	int ret = 0;

	ocmdbg("entry\n");
	ocmcdev.pdev = pdev;
	ret = ocmcdev_miscdev_init(&ocmcdev);
	if (ret != 0)
		return ret;
	ocmcdev.reg = platform_get_resource(ocmcdev.pdev, IORESOURCE_MEM, 0);
	if (ocmcdev.reg == NULL)
		return -EINVAL;
	omp->base_addr = ocmcdev.reg->start + DPU_USED_SIZE;
	omp->size = ocmcdev.reg->end - ocmcdev.reg->start - DPU_USED_SIZE;
	omp->pool = gen_pool_create(12, -1);
	if (!omp->pool) {
		ocmerr("%s gen pool create failed\n", dev_name(dev));
		ocmcdev_miscdev_destroy(&ocmcdev);
		return -EINVAL;
	}
	idr_init(&ocmcdev.ids_head);
	ret = ocm_clk_init(dev);
	if (ret != 0) {
		ocmerr("ocm_clk_init failed:%d\n", ret);
		goto out;
	}

	INIT_LIST_HEAD(&ocmcdev.omp.head);
	ret = gen_pool_add(omp->pool, omp->base_addr, omp->size, -1);
	if (ret != 0) {
		ocmerr("%s gen pool add failed: %d\n", dev_name(dev), ret);
		goto out;
	}

	ocmcdev.regulator = devm_regulator_get(dev, "ocm");
	if (IS_ERR_OR_NULL(ocmcdev.regulator)) {
		ocmerr("ocm regulator get failed\n");
		goto out;
	}
	g_npu_sys_pm_ocm_wksrc = wakeup_source_register(NULL, "npu_ocm_pm_wksrc");
	if (g_npu_sys_pm_ocm_wksrc == NULL) {
		ocmerr("%s:%d x_sys_pm_wksrc create err!\n", __func__, __LINE__);
		ret = -ENOMEM;
		goto out;
	}
	mutex_init(&ocmcdev.boot_lock);
	platform_set_drvdata(pdev, &ocmcdev);
	ocmdbg("end\n");
	return 0;
out:
	idr_destroy(&ocmcdev.ids_head);
	gen_pool_destroy(omp->pool);
	ocmcdev_miscdev_destroy(&ocmcdev);
	return -1;
}

static int ocm_remove(struct platform_device *pdev)
{
	struct ocm_cdev *ocmcdev = platform_get_drvdata(pdev);

	ocmdbg("entry\n");
	idr_destroy(&ocmcdev->ids_head);
	gen_pool_destroy(ocmcdev->omp.pool);
	ocmcdev_miscdev_destroy(ocmcdev);
	mutex_destroy(&ocmcdev->boot_lock);
	ocmdbg("end\n");
	return 0;
}

static const struct of_device_id ocm_id_table[] = {
	{ .compatible = "xring,ocm", },
	{},
	};

MODULE_DEVICE_TABLE(of, ocm_id_table);

static struct platform_driver ocm_driver = {
	.probe = ocm_probe,
	.remove = ocm_remove,
	.driver = {
		.name = "ocm-buf",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ocm_id_table),
	},
};

module_platform_driver(ocm_driver);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("OCM Memory Management Driver");
MODULE_LICENSE("GPL v2");
