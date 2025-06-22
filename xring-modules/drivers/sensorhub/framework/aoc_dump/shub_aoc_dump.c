// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/of_reserved_mem.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <dt-bindings/xring/platform-specific/sensorhub_ddr_layout.h>
#include <soc/xring/sensorhub/shub_boot_prepare.h>

#define TYPE 0x7D
#define GET_AOC_DUMP_MEM _IO(TYPE, 1)
#define DEVICE_NAME "shub_aoc_dump"

struct shub_sensor_shm {
	uint64_t phy_addr;
	uint32_t size;
};

static struct shub_sensor_shm g_aoc_dump_shm;

static int shub_sensor_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int shub_sensor_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long page;
	unsigned long start = (unsigned long)vma->vm_start;
	unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);
	int ret;

	if (g_aoc_dump_shm.size == 0 || size != g_aoc_dump_shm.size) {
		pr_err("mmap aoc dump mem size error : %lu, %u\n", size, g_aoc_dump_shm.size);
		return -EINVAL;
	}

	page = g_aoc_dump_shm.phy_addr;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	ret = remap_pfn_range(vma, start, page >> PAGE_SHIFT, size,
							vma->vm_page_prot);

	pr_info("mmap aoc dump mem done : %d\n", ret);

	return ret;
}

static long shub_sensor_ioctl(struct file *filp, unsigned int cmd,
							  unsigned long arg)
{
	int err = 0;

	if (!filp)
		return -EBADF;
	if (!arg)
		return -EFAULT;

	switch (cmd) {
	case GET_AOC_DUMP_MEM:
		if (g_aoc_dump_shm.phy_addr == 0)
			return -ENXIO;
		err = copy_to_user((void *)arg, &g_aoc_dump_shm, sizeof(g_aoc_dump_shm));
		break;
	default:
		break;
	}

	return err;
}

static const struct file_operations g_shub_sensor_dev_fops = {
	.owner = THIS_MODULE,
	.open = shub_sensor_open,
	.mmap = shub_sensor_mmap,
	.unlocked_ioctl = shub_sensor_ioctl,
};

static struct miscdevice g_shub_sensor_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &g_shub_sensor_dev_fops,
};

static int aoc_mem_init(void)
{
	struct device_node *np = NULL;
	struct reserved_mem *rsv_mem = NULL;

	np = of_find_node_by_name(NULL, "rsv_mem_aoc_dump");
	if (!np) {
		pr_err("rsv_mem_aoc_dump not found\n");
		return -ENXIO;
	}

	rsv_mem = of_reserved_mem_lookup(np);
	if (!rsv_mem) {
		pr_err("find aoc rsv mem fail\n");
		return -ENXIO;
	}

	g_aoc_dump_shm.phy_addr = rsv_mem->base;
	g_aoc_dump_shm.size = rsv_mem->size;
	pr_info("find aoc rsv mem success\n");

	return 0;
}

static int shub_aoc_dump_init(void)
{
	int ret;

	g_aoc_dump_shm.phy_addr = 0;
	g_aoc_dump_shm.size = 0;

	ret = aoc_mem_init();
	if (ret != 0)
		return ret;

	ret = misc_register(&g_shub_sensor_misc);
	if (ret != 0) {
		pr_err("misc register fail, ret = %d\n", ret);
		return ret;
	}

	return ret;
}

static void shub_aoc_dump_exit(void)
{
	misc_deregister(&g_shub_sensor_misc);
}

module_init(shub_aoc_dump_init);
module_exit(shub_aoc_dump_exit);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("XRing AOC Dump");
MODULE_LICENSE("GPL v2");
