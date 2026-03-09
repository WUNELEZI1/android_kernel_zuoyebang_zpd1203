// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/miscdevice.h>
#include <linux/proc_fs.h>

#include "memcheck_ioctl.h"
#include "memcheck_process_mem.h"
#include "memcheck_cma.h"
#include "memcheck_dma-buf.h"
#include "memcheck_slab.h"
#include "memcheck_vmalloc.h"

static long memcheck_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	memcheck_err("%ud, %lu\n", cmd, arg);
	if (cmd < MEMCHECK_IOC_MIN || cmd > MEMCHECK_IOC_MAX)
		return -EINVAL;

	switch (cmd) {
	case MEMCHECK_PMEM_INFO:
		memcheck_top_pmem_info(arg);
		break;
	case MEMCHECK_CMA_INFO:
		memcheck_cma_info_show();
		break;
	case MEMCHECK_DMABUF_INFO:
		memcheck_dmabuf_info_show();
		break;
	case MEMCHECK_SLAB_INFO:
		memcheck_slab_info_show();
		break;
	case MEMCHECK_VMALLOC_INFO:
		memcheck_vmalloc_info_show();
		break;
	case MEMCHECK_GPU_INFO:
		break;
	default:
		memcheck_err("invaild cmd %d not support\n", cmd);
		ret = -EINVAL;
		break;
	}
	return ret;
}


static const struct file_operations memcheck_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = memcheck_ioctl,
	.llseek = noop_llseek,
};

static struct miscdevice memcheck_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "memcheck_ioctl",
	.fops = &memcheck_fops,
};

int memcheck_ioctl_init(void)
{
	int ret = 0;

	ret = misc_register(&memcheck_miscdev);
	if (!ret) {
		memcheck_err("memcheck_ioctl_init failed\n");
		return ret;
	}
	memcheck_info("memcheck_ioctl_init success\n");

	return ret;
}