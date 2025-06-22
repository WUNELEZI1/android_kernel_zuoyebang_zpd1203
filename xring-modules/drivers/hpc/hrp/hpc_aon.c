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
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/firmware.h>
#include <linux/dma-heap.h>
#include <linux/sysfs.h>
#include <linux/mm.h>
#include <linux/string.h>
#include "hpc_internal.h"
#include "hpc_tee_ca.h"
#include <soc/xring/xr_dmabuf_helper.h>
#include <linux/dma-mapping.h>

#define HADEV_BOOT  _IO('X', 0)
#define HADEV_EXIT  _IO('X', 1)
//AON_FW_SHMEM_SIZE needs to be 4k aligned
#define AON_FW_SHMEM_SIZE    0x00100000
#define AON_FW_RUNTIME_SIZE  0x00A00000
#define HAON_LOG_LEVEL_MAX   0xFF
#define HAON_LOG_CHECKCODE1  0x11223344
#define HAON_LOG_CHECKCODE2  0xFFEEDDCC

static inline struct hpc_aon_device *miscdev_to_hadev(struct miscdevice *miscdev)
{
	return container_of(miscdev, struct hpc_aon_device, miscdev);
}

static int haon_alloc_sec_mem(struct hpc_aon_device *hadev)
{
	struct hpc_device *hdev = hadev->hdev;
	struct dma_buf *dma_buf = NULL;
	uint32_t img_size = hadev->cipherimg_size;
	u32 sfd;
	int ret = 0;

	dma_buf = dma_heap_buffer_alloc(hdev->system_heap, img_size, O_RDWR, 0);
	if (IS_ERR(dma_buf)) {
		hpcerr("alloc imgbuf_src fail, ret = %ld(%pe)", PTR_ERR(dma_buf), dma_buf);
		return -ENOMEM;
	}
	xr_dmabuf_kernel_account(dma_buf, XR_DMABUF_KERNEL_NPU);
	hadev->imgbuf_src = dma_buf;
	sfd = xr_dmabuf_helper_get_sfd(hadev->imgbuf_src);
	if (!sfd) {
		hpcerr("get imgbuf_src sfd failed");
		ret = -EINVAL;
		goto free_imgsrc_buf;
	}
	hadev->imgsrc_sfd = (int)sfd;

	dma_buf = dma_heap_buffer_alloc(hdev->system_heap, AON_FW_RUNTIME_SIZE, O_RDWR, 0);
	if (IS_ERR(dma_buf)) {
		hpcerr("alloc imgbuf_run fail, ret = %ld(%pe)", PTR_ERR(dma_buf), dma_buf);
		ret = -ENOMEM;
		goto free_imgsrc_buf;
	}
	xr_dmabuf_kernel_account(dma_buf, XR_DMABUF_KERNEL_NPU);
	hadev->imgbuf_run = dma_buf;
	hadev->imgrun_size = AON_FW_RUNTIME_SIZE;
	sfd = xr_dmabuf_helper_get_sfd(hadev->imgbuf_run);
	if (!sfd) {
		hpcerr("get imgbuf_run sfd failed");
		ret = -EINVAL;
		goto free_imgrun_buf;
	}
	hadev->imgrun_sfd = (int)sfd;

	return ret;

free_imgrun_buf:
	xr_dmabuf_kernel_unaccount(hadev->imgbuf_run, XR_DMABUF_KERNEL_NPU);
	dma_heap_buffer_free(hadev->imgbuf_run);
	hadev->imgbuf_run = NULL;
free_imgsrc_buf:
	xr_dmabuf_kernel_unaccount(hadev->imgbuf_src, XR_DMABUF_KERNEL_NPU);
	dma_heap_buffer_free(hadev->imgbuf_src);
	hadev->imgbuf_src = NULL;

	return ret;
}

static void haon_free_sec_mem(struct hpc_aon_device *hadev)
{
	if (hadev->imgbuf_run) {
		xr_dmabuf_kernel_unaccount(hadev->imgbuf_run, XR_DMABUF_KERNEL_NPU);
		dma_heap_buffer_free(hadev->imgbuf_run);
		hadev->imgbuf_run = NULL;
	}
	if (hadev->imgbuf_src) {
		xr_dmabuf_kernel_unaccount(hadev->imgbuf_src, XR_DMABUF_KERNEL_NPU);
		dma_heap_buffer_free(hadev->imgbuf_src);
		hadev->imgbuf_src = NULL;
	}
}

static void haon_free_share_mem(struct hpc_aon_device *hadev)
{
	if (hadev->imgshm_va) {
		dma_buf_vunmap(hadev->imgshm_dmabuf, &hadev->imgshm_map);
		dma_buf_put(hadev->imgshm_dmabuf);
		xr_dmabuf_kernel_unaccount(hadev->imgshm_dmabuf, XR_DMABUF_KERNEL_NPU);
		dma_heap_buffer_free(hadev->imgshm_dmabuf);
	}
	hadev->imgshm_va = NULL;
	hadev->imgshm_size = 0;
}

static int haon_alloc_share_mem(struct hpc_aon_device *hadev)
{
	int ret = 0;

	hadev->imgshm_dmabuf = dma_heap_buffer_alloc(hadev->dma_sysheap, AON_FW_SHMEM_SIZE, O_RDWR, 0);
	if (IS_ERR(hadev->imgshm_dmabuf)) {
		hadev->imgshm_dmabuf = NULL;
		hpcerr("dma_heap_buffer_alloc fail");
		ret = -ENOBUFS;
		goto exit;
	}
	xr_dmabuf_kernel_account(hadev->imgshm_dmabuf, XR_DMABUF_KERNEL_NPU);
	get_dma_buf(hadev->imgshm_dmabuf);

	ret = dma_buf_vmap(hadev->imgshm_dmabuf, &hadev->imgshm_map);
	if (ret) {
		hpcerr("dma buf vmap fail ret = %d", ret);
		goto buf_release;
	}
	hadev->imgshm_va = hadev->imgshm_map.vaddr;
	hadev->imgshm_size = AON_FW_SHMEM_SIZE;

	hpcinfo("shmem dmabuf size[0x%zx]", hadev->imgshm_dmabuf->size);

	return ret;

buf_release:
	dma_buf_put(hadev->imgshm_dmabuf);
	xr_dmabuf_kernel_unaccount(hadev->imgshm_dmabuf, XR_DMABUF_KERNEL_NPU);
	dma_heap_buffer_free(hadev->imgshm_dmabuf);
exit:
	return ret;
}

static int haon_request_fw(struct hpc_aon_device *hadev)
{
	struct device *dev = &hadev->hdev->pdev->dev;
	const struct firmware *firmware_p;
	int ret = 0;

	ret = request_firmware(&firmware_p, "sec_npuaoc.img", dev);
	if (ret < 0) {
		hpcerr("request sec_npuaoc fw failed: %d\n", ret);
		return ret;
	}
	hpcinfo("request sec_npuaoc fw success\n");
	memset(hadev->imgshm_va, 0x0, hadev->imgshm_size);
	memcpy(hadev->imgshm_va, firmware_p->data, firmware_p->size);
	hadev->cipherimg_size = firmware_p->size;
	hpcinfo("cipherimg_size[0x%x]\n", hadev->cipherimg_size);
	release_firmware(firmware_p);
	hpcinfo("memcpy sec_npuaoc fw success\n");

	return ret;
}

static int haon_load_fw(struct hpc_aon_device *hadev)
{
	int ret = 0;

	hpcinfo("entry\n");
	if (hadev->aon_enable == HAON_ENABLE) {
		hpcinfo("hpc aon has been enabled");
		return ret;
	}

	ret = haon_alloc_share_mem(hadev);
	if (ret < 0) {
		hpcerr("alloc share memory for image failed\n");
		return ret;
	}

	ret = haon_request_fw(hadev);
	if (ret != 0) {
		hpcerr("request fw failed\n");
		goto free_share_mem;
	}

	ret = haon_alloc_sec_mem(hadev);
	if (ret < 0) {
		hpcerr("alloc sec memory for image failed\n");
		goto free_share_mem;
	}

	ret = hree_ca_load_aonfw(hadev);
	if (ret != 0) {
		hpcerr("hree ca load fw failed\n");
		goto free_sec_mem;
	}

	haon_free_share_mem(hadev);

	hadev->aon_enable = HAON_ENABLE;

	hpcinfo("end\n");
	return ret;

free_sec_mem:
	haon_free_sec_mem(hadev);

free_share_mem:
	haon_free_share_mem(hadev);
	hadev->aon_enable = HAON_DISENABLE;
	return ret;
}

static void haon_release_fw(struct hpc_aon_device *hadev)
{
	hpcinfo("entry\n");
	hadev->aon_enable = HAON_DISENABLE;
	haon_free_share_mem(hadev);
	haon_free_sec_mem(hadev);
}

static int haon_cdev_open(struct inode *inode, struct file *filp)
{
	struct hpc_aon_device *hadev = miscdev_to_hadev(filp->private_data);

	hpcinfo("entry\n");
	filp->private_data = hadev;
	hpcinfo("end\n");

	return 0;
}

static ssize_t haon_cdev_read(struct file *filp,
		char __user *buf, size_t len, loff_t *pos)
{
	int ret = 0;
	return ret;
}

static ssize_t haon_cdev_write(struct file *filp,
		const char __user *buf, size_t len, loff_t *pos)
{
	int ret = 0;
	return ret;
}

static long haon_cdev_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	int ret = 0;
	struct hpc_aon_device *hadev;

	hpcinfo("entry\n");
	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -ENOENT;
	}
	hadev = filp->private_data;

	switch (cmd) {
	case HADEV_BOOT:
		ret = haon_load_fw(hadev);
		if (ret < 0)
			hpcerr("hadev load fw fail\n");
		break;
	case HADEV_EXIT:
		haon_release_fw(hadev);
		break;
	default:
		return -ENOTTY;
	}

	hpcinfo("end\n");
	return ret;
}

static int haon_cdev_release(struct inode *inode, struct file *filp)
{
	int ret = 0;

	hpcinfo("start\n");
	filp->private_data = NULL;
	hpcinfo("end\n");

	return ret;
}

static const struct file_operations hadev_fops = {
	.owner = THIS_MODULE,
	.open = haon_cdev_open,
	.read = haon_cdev_read,
	.write = haon_cdev_write,
	.unlocked_ioctl = haon_cdev_ioctl,
	.compat_ioctl = compat_ptr_ioctl,
	.release = haon_cdev_release,
};

static int haon_miscdev_init(struct hpc_aon_device *hadev)
{
	struct miscdevice *miscdev = &hadev->miscdev;
	int ret;

	miscdev->parent = &hadev->hdev->pdev->dev;
	miscdev->minor = MISC_DYNAMIC_MINOR;
	miscdev->name = "hpc-aon";
	miscdev->fops = &hadev_fops;
	ret = misc_register(miscdev);
	if (ret != 0)
		hpcerr("register hpc aon miscdev failed: %d\n", ret);
	return ret;
}

static void haon_miscdev_destroy(struct hpc_aon_device *hadev)
{
	misc_deregister(&hadev->miscdev);
}

int haon_sysheap_find(struct hpc_aon_device *hadev)
{
	int ret = 0;
	struct dma_heap *dma_sysheap = NULL; //system_heap

	dma_sysheap = dma_heap_find("system");
	if (IS_ERR_OR_NULL(dma_sysheap)) {
		dma_sysheap = NULL;
		hpcerr("system head not find, ret = %d(%pe)", ret, dma_sysheap);
		return -EINVAL;
	}
	hadev->dma_sysheap = dma_sysheap;
	return ret;
}

#if IS_ENABLED(CONFIG_XRING_DEBUG)
static ssize_t pressure_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long log_level;
	struct hpc_device *hdev = dev_get_drvdata(dev);
	struct hpc_mem_info *mem = NULL;
	struct haon_log_info *log_info = NULL;

	hpcinfo("entry\n");
	if (kstrtoul(buf, 0, &log_level))
		return -EAGAIN;
	if (log_level > HAON_LOG_LEVEL_MAX) {
		hpcerr("log_level is llegal para, %lu\n", log_level);
		return -EAGAIN;
	}

	mem = &hdev->hbm[HBM_HMDR_TYPE];
	if (!mem->va) {
		hpcerr("hmdr buffer va is null, then return");
		return -ENOMEM;
	}
	log_info = (struct haon_log_info *)mem->va;
	log_info->check_code1 = HAON_LOG_CHECKCODE1;
	log_info->log_level = log_level;
	hpcinfo("set log_level[%lu] success\n", log_level);

	hpcinfo("end\n");
	return count;
}
static DEVICE_ATTR_WO(pressure);

static ssize_t state_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned long log_level;
	struct hpc_device *hdev = dev_get_drvdata(dev);
	struct hpc_mem_info *mem = NULL;
	struct haon_log_info *log_info = NULL;

	hpcinfo("entry\n");

	mem = &hdev->hbm[HBM_HMDR_TYPE];
	if (!mem->va) {
		hpcerr("hmdr buffer va is null, then return");
		return 0;
	}
	log_info = (struct haon_log_info *)mem->va;
	log_level = log_info->log_level;
	hpcinfo("get log_level[%lu] success\n", log_level);

	return snprintf(buf, 8, "%lu\n", log_level);
}
static DEVICE_ATTR_RO(state);

static struct attribute *haon_attributes[] = {
	&dev_attr_state.attr,
	&dev_attr_pressure.attr,
	NULL
};

static const struct attribute_group haon_attr_group = {
	.name = "aon_npufw_log",
	.attrs = haon_attributes,
};
#endif

int haon_init(struct hpc_device *hdev)
{
	struct hpc_aon_device *hadev = NULL;
	struct device *dev = &hdev->pdev->dev;
	int ret;

	hpcinfo("entry\n");
	hadev = devm_kzalloc(dev, sizeof(*hadev), GFP_KERNEL);
	if (!hadev) {
		hpcerr("hpc aon device zalloc failed\n");
		return -ENOMEM;
	}
	hadev->aon_enable = HAON_DISENABLE;
	hadev->hdev = hdev;
	hdev->hadev = hadev;
	ret = haon_miscdev_init(hadev);
	if (ret != 0)
		return ret;
	ret = haon_sysheap_find(hadev);
	if (ret != 0)
		return ret;
#if IS_ENABLED(CONFIG_XRING_DEBUG)
	ret = sysfs_create_group(&dev->kobj, &haon_attr_group);
	if (ret < 0)
		dev_err(dev, "attr group create failed\n");
	else
		dev_info(dev, "attr group create success!\n");
#endif
	hpcinfo("end\n");

	return 0;
}
EXPORT_SYMBOL_GPL(haon_init);

int haon_exit(struct hpc_device *hdev)
{
	struct hpc_aon_device *hadev = hdev->hadev;

	hpcinfo("entry\n");
	haon_miscdev_destroy(hadev);
	hdev->hadev = NULL;
#if IS_ENABLED(CONFIG_XRING_DEBUG)
	struct device *dev = &hdev->pdev->dev;
	sysfs_remove_group(&dev->kobj, &haon_attr_group);
#endif
	hpcinfo("end\n");
	return 0;
}
EXPORT_SYMBOL_GPL(haon_exit);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC AON Driver");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
MODULE_IMPORT_NS(DMA_BUF);
