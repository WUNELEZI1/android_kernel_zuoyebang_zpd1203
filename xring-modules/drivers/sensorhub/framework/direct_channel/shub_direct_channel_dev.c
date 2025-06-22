// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/genalloc.h>
#include <linux/slab.h>
#include <dt-bindings/xring/platform-specific/sensorhub_ddr_layout.h>

#define TYPE 0x7B
#define GET_BIAS _IO(TYPE, 1)
#define DEVICE_NAME "direct_channel"
static void __iomem *g_shmem_addr_base;
static struct gen_pool *g_shm_pool;
static struct mutex g_pool_lock;
static DEFINE_MUTEX(g_direct_channel_lock);

struct shm_node {
	unsigned long shm;
	size_t size;
};

static int direct_channel_open(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static int direct_channel_open_lock(struct inode *inode, struct file *filp)
{
	int res;

	mutex_lock(&g_direct_channel_lock);
	res = direct_channel_open(inode, filp);
	mutex_unlock(&g_direct_channel_lock);

	return res;
}

static int direct_channel_release(struct inode *inode, struct file *filp)
{
	struct shm_node *node;

	if (!filp->private_data)
		return 0;
	node = (struct shm_node *)filp->private_data;

	mutex_lock(&g_pool_lock);
	gen_pool_free(g_shm_pool, node->shm, node->size);
	mutex_unlock(&g_pool_lock);

	pr_info("gen_pool_free %zu", node->size);
	kfree(filp->private_data);
	filp->private_data = NULL;
	return 0;
}

static int direct_channel_release_lock(struct inode *inode, struct file *filp)
{
	int res;

	mutex_lock(&g_direct_channel_lock);
	res = direct_channel_release(inode, filp);
	mutex_unlock(&g_direct_channel_lock);

	return res;
}

static ssize_t direct_channel_write(struct file *filp,
				    const char __user *buffer,
				    size_t write_size, loff_t *pos)
{
	struct shm_node *node;
	int err;

	if (!filp->private_data)
		return -ENOMEM;
	node = (struct shm_node *)filp->private_data;
	if (write_size > node->size)
		return -ENOMEM;
	err = copy_from_user((void *)node->shm, buffer, write_size);
	if (err)
		return err;
	return write_size;
}

static ssize_t direct_channel_write_lock(struct file *filp,
					 const char __user *buffer,
					 size_t write_size, loff_t *pos)
{
	int res;

	mutex_lock(&g_direct_channel_lock);
	res = direct_channel_write(filp, buffer, write_size, pos);
	mutex_unlock(&g_direct_channel_lock);

	return res;
}

static int direct_channel_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long page;
	unsigned long start = (unsigned long)vma->vm_start;
	unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);
	int ret;
	unsigned long shm;
	struct shm_node *node;

	if (filp->private_data)
		return -EEXIST;
	if (size == 0)
		return -EINVAL;

	node = kzalloc(sizeof(struct shm_node), GFP_KERNEL);
	if (!node)
		return -ENOMEM;

	mutex_lock(&g_pool_lock);
	shm = (uintptr_t)gen_pool_alloc(g_shm_pool, size);
	mutex_unlock(&g_pool_lock);

	if (!shm) {
		kfree(node);
		return -ENOMEM;
	}
	memset((void *)shm, 0, size);
	node->shm = shm;
	node->size = size;

	pr_info("gen_pool_alloc %lu\n", size);
	filp->private_data = node;
	page = SHUB_NS_DDR_SDC_MEM_ADDR + (shm - (uintptr_t)g_shmem_addr_base);
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	ret = remap_pfn_range(vma, start, page >> PAGE_SHIFT, size,
			      vma->vm_page_prot);

	if (ret) {
		mutex_lock(&g_pool_lock);
		gen_pool_free(g_shm_pool, shm, size);
		mutex_unlock(&g_pool_lock);
		kfree(node);
		filp->private_data = NULL;
		return ret;
	}

	return 0;
}

static int direct_channel_mmap_lock(struct file *filp,
				    struct vm_area_struct *vma)
{
	int res;

	mutex_lock(&g_direct_channel_lock);
	res = direct_channel_mmap(filp, vma);
	mutex_unlock(&g_direct_channel_lock);

	return res;
}

static long direct_channel_ioctl(struct file *filp, unsigned int cmd,
				 unsigned long arg)
{
	unsigned long bias;
	struct shm_node *node;
	int err = 0;

	if (!filp)
		return -EFAULT;
	if (!arg)
		return -EINVAL;

	switch (cmd) {
	case GET_BIAS:
		if (!filp->private_data)
			return -ENXIO;
		node = (struct shm_node *)filp->private_data;
		bias = node->shm - (uintptr_t)g_shmem_addr_base;
		err = copy_to_user((void *)arg, &bias, sizeof(bias));
		break;
	default:
		break;
	}
	return err;
}

static long direct_channel_ioctl_lock(struct file *filp, unsigned int cmd,
				      unsigned long arg)
{
	long res;

	mutex_lock(&g_direct_channel_lock);
	res = direct_channel_ioctl(filp, cmd, arg);
	mutex_unlock(&g_direct_channel_lock);

	return res;
}

static const struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = direct_channel_open_lock,
	.write = direct_channel_write_lock,
	.release = direct_channel_release_lock,
	.mmap = direct_channel_mmap_lock,
	.unlocked_ioctl = direct_channel_ioctl_lock,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dev_fops,
};

static int __init dev_init(void)
{
	int ret;

	mutex_init(&g_pool_lock);
	g_shmem_addr_base = ioremap_wc((ssize_t)SHUB_NS_DDR_SDC_MEM_ADDR,
				       (unsigned long)SHUB_NS_DDR_SDC_MEM_SIZE);
	if (IS_ERR_OR_NULL(g_shmem_addr_base)) {
		pr_err("ioremap err\n");
		return -ENOMEM;
	}
	g_shm_pool = gen_pool_create(PAGE_SHIFT, -1);
	if (IS_ERR_OR_NULL(g_shm_pool)) {
		pr_err("gen_pool_create failed\n");
		ret = -ENOMEM;
		goto unmap;
	}

	ret = gen_pool_add(g_shm_pool, (uintptr_t)g_shmem_addr_base,
			   SHUB_NS_DDR_SDC_MEM_SIZE, -1);
	if (ret != 0) {
		ret = -ENOMEM;
		pr_err("gen_pool_add failed\n");
		goto destroy_pool;
	}

	ret = misc_register(&misc);
	if (!ret) {
		pr_info("direct channel register success %d\n", ret);
		return 0;
	}

destroy_pool:
	gen_pool_destroy(g_shm_pool);
	g_shm_pool = NULL;
unmap:
	iounmap(g_shmem_addr_base);
	mutex_destroy(&g_pool_lock);

	return ret;
}
static void __exit dev_exit(void)
{
	gen_pool_destroy(g_shm_pool);
	iounmap(g_shmem_addr_base);
	misc_deregister(&misc);
	mutex_destroy(&g_pool_lock);
}
module_init(dev_init);
module_exit(dev_exit);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("XRing Direct Channel");
MODULE_LICENSE("GPL v2");
