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
#include <linux/irqreturn.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include <linux/platform_device.h>
#include "xrp_internal.h"
#include "xrp_kernel_defs.h"
#include <linux/interrupt.h>
#include <linux/delay.h>
#include "xrp_hw.h"

#define XRP_MDR_EXP_TYPE_MAX			2
#define VDSP_SET_LOG_LEVEL				_IOWR('V', 1, unsigned long)
#define VDSP_QUERY_STATUS				_IOWR('V', 2, unsigned long)
#define VDSP_GET_LOG_DROP_PATH			_IOWR('V', 3, unsigned long)
#define DRIVER_NAME						"xrp-mdr"
#define VDSP_MDR_ADDR					(0x102B0000)
#define VDSP_MDR_SIZE					(0x20000)
#define VDSP_GPIO_LOG_INT_FLAG			(0xAA)
#define EXIT_VDSP_GPIO_INTERRUPT		(1)
#define PATH_MAXLEN						128

struct irq_num_record {
	int wdt1_vdsp;
	int gpio_ns_vdsp;
};

static char LOG_DROP_PATH[PATH_MAXLEN];
static struct irq_num_record irq_record = {0};

static struct mdr_exception_info_s g_hexp[XRP_MDR_EXP_TYPE_MAX] = {
	[0] = {
		.e_modid            = (u32)MODID_VDSP_WDT1_EXCEPTION,
		.e_modid_end        = (u32)MODID_VDSP_WDT1_EXCEPTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_WAIT,
		.e_notify_core_mask = MDR_VDSP,
		.e_reset_core_mask  = MDR_VDSP,
		.e_from_core        = MDR_VDSP,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = VDSP_S_EXCEPTION,
		.e_exce_subtype     = VDSP_S_WDT1,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "VDSP",
		.e_desc             = "VDSP wdt1 exception",
	},
	[1] = {
		.e_modid            = (u32)MODID_VDSP_GPIO_NS_EXCEPTION,
		.e_modid_end        = (u32)MODID_VDSP_GPIO_NS_EXCEPTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_WAIT,
		.e_notify_core_mask = MDR_VDSP,
		.e_reset_core_mask  = MDR_VDSP,
		.e_from_core        = MDR_VDSP,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = VDSP_S_EXCEPTION,
		.e_exce_subtype     = VDSP_S_GPIO_NS,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "VDSP",
		.e_desc             = "VDSP exception",
	},
};

static int hpc_smc_xrp_readreg(bool use_exit_interrupt)
{
	struct arm_smccc_res res = {0};

	if (use_exit_interrupt) {
		res.a2 = EXIT_VDSP_GPIO_INTERRUPT;
		arm_smccc_smc(FID_BL31_HPC_XRP_READREG, 1, 1, 0, 0, 0, 0, 0, &res);
	} else
		arm_smccc_smc(FID_BL31_HPC_XRP_READREG, 0, 0, 0, 0, 0, 0, 0, &res);

	return res.a1;
}

static int xmdr_cdev_fasync(int fd, struct file *filp, int mode)
{
	int ret;
	struct xrp_mdr_device *xmdev;

	xmdev = filp->private_data;
	xrp_print(LOG_LEVEL_INFO, "%s start, xmdev %pK\n", __func__, xmdev);
	ret = fasync_helper(fd, filp, mode, &xmdev->fasync_q);
	xrp_print(LOG_LEVEL_DEBUG, "%s fd = %d, &xmdev->fasync_q = %pK\n", __func__, fd, &xmdev->fasync_q);
	xrp_print(LOG_LEVEL_INFO, "%s fasync end\n", __func__);
	return ret;
}

void print_mbox_send_task_msg_t(struct mbox_msg_t msg)
{
	xrp_print(LOG_LEVEL_DEBUG, "msg_len: %u\n", msg.msg_len);
	xrp_print(LOG_LEVEL_DEBUG, "msg_type: %u\n", msg.msg_type);
	xrp_print(LOG_LEVEL_DEBUG, "cv_type: %u\n", msg.cv_type);
	xrp_print(LOG_LEVEL_DEBUG, "msg_id: %u\n", msg.msg_id);
	xrp_print(LOG_LEVEL_DEBUG, "priority: %u\n", msg.priority);
}

void print_xrp_mdr_mbox_send_setlog_msg_s(struct xrp_mdr_mbox_send_setlog_msg_s msg)
{
	xrp_print(LOG_LEVEL_DEBUG, "msg_len: %u\n", msg.msg_len);
	xrp_print(LOG_LEVEL_DEBUG, "msg_type: %u\n", msg.msg_type);
	xrp_print(LOG_LEVEL_DEBUG, "event_msg len: %u\n", msg.event_msg.len);

	if (msg.event_msg.type == HE_LOG_FLUSH) {
		xrp_print(LOG_LEVEL_DEBUG, "event_msg type: HE_LOG_FLUSH\n");
		xrp_print(LOG_LEVEL_DEBUG, "readp_offset: %u\n", msg.event_msg.msg.hlfm.readp_offset);
		xrp_print(LOG_LEVEL_DEBUG, "wirtep_offset: %u\n", msg.event_msg.msg.hlfm.wirtep_offset);
	} else if (msg.event_msg.type == HE_LOG_LEVEL_SET) {
		xrp_print(LOG_LEVEL_DEBUG, "event_msg type: HE_LOG_LEVEL_SET\n");
		xrp_print(LOG_LEVEL_DEBUG, "log_level: %u\n", msg.event_msg.msg.hllm.log_level);
	}
}

void print_xrp_mdr_mbox_send_query_status(struct xrp_mdr_mbox_send_query_msg_s msg)
{
	xrp_print(LOG_LEVEL_DEBUG, "msg_len: %u\n", msg.msg_len);
	xrp_print(LOG_LEVEL_DEBUG, "msg_type: %u\n", msg.msg_type);

	switch (msg.status_msg.status) {
	case DEFAULT:
		xrp_print(LOG_LEVEL_DEBUG, "status: DEFAULT\n");
		break;
	case POWEROFF:
		xrp_print(LOG_LEVEL_DEBUG, "Status: POWEROFF\n");
		break;
	case WORK:
		xrp_print(LOG_LEVEL_DEBUG, "status: WORK\n");
		break;
	case IDLE:
		xrp_print(LOG_LEVEL_DEBUG, "status: IDLE\n");
		break;
	case HANG:
		xrp_print(LOG_LEVEL_DEBUG, "status: HANG\n");
		break;
	case STATUS_MAX:
		xrp_print(LOG_LEVEL_DEBUG, "status: MAX\n");
		break;
	default:
		xrp_print(LOG_LEVEL_ERROR, "status: Unknown\n");
	}
}

int xrp_send_event(struct xrp_device *xdev, void *msg)
{
	int ret;
	struct vdsp_mbox *vmbox;

	xrp_print(LOG_LEVEL_DEBUG, "xrp send event entry %s\n", __func__);
	if (!msg || !xdev) {
		xrp_print(LOG_LEVEL_ERROR, "msg || xdev is null\n");
		return -EFAULT;
	}

	vmbox = xdev->xtdev->vmbox;
	if (!vmbox->tx_ch) {
		xrp_print(LOG_LEVEL_ERROR, "mdr tx channel not initialized\n");
		return -ENODEV;
	}

	ret = mbox_send_message(vmbox->tx_ch, msg);
	if (ret < 0)
		xrp_print(LOG_LEVEL_ERROR, "send msg failed: %d\n", ret);

	xrp_print(LOG_LEVEL_DEBUG, "exit %s ret = %d\n", __func__, ret);
	return ret;
}

static int xrp_set_log_level(struct xrp_mdr_device *xmdev, unsigned long log_level)
{
	int ret = 0;

	xrp_print(LOG_LEVEL_INFO, "xrp set log level enter %s\n", __func__);
	xmdev->log_level = log_level;
	xrp_print(LOG_LEVEL_DEBUG, "by service modify log_level, value is %d\n", xmdev->log_level);

	return ret;
}

int xrp_query_vdsp_status(struct xrp_mdr_device *xmdev, unsigned long *query_status)
{
	int ret = 0;
	struct xrp_device *xdev;
	struct xboot_dev *xbdev;

	xrp_print(LOG_LEVEL_DEBUG, "query status enter %s\n", __func__);
	xdev = xmdev->xdev;
	xbdev = xdev->xtdev->xbdev;

	if (atomic_read(&xbdev->power) == 0) {
		xrp_print(LOG_LEVEL_ERROR, "%s power_atomic is 0, vdsp is in poweroff, wait poweron\n", __func__);
		xrp_print(LOG_LEVEL_ERROR, "vdsp_status is POWEROFF\n");
		*query_status = POWEROFF;
		return ret;
	}

	*query_status = WORK;
	xrp_print(LOG_LEVEL_DEBUG, "vdsp_status is poweron, in work\n");
	xrp_print(LOG_LEVEL_DEBUG, "query status end %s\n", __func__);
	return ret;
}

static long xmdr_cdev_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	int ret = 0;
	unsigned long log_level = 0;
	struct xrp_mdr_device *xmdev;
	unsigned long query_status = DEFAULT;

	xrp_print(LOG_LEVEL_INFO, "%s cdev ioctl entry\n", __func__);
	xmdev = filp->private_data;
	switch (cmd) {
	case VDSP_SET_LOG_LEVEL:
		if (copy_from_user(&log_level, (unsigned long __user *)arg, sizeof(unsigned long)))
			return -EFAULT;
		ret = xrp_set_log_level(xmdev, log_level);
		if (ret < 0)
			xrp_print(LOG_LEVEL_ERROR, "xrp set log level fail\n");
		xrp_print(LOG_LEVEL_INFO, "set log level ret = %d\n", ret);
		break;
	case VDSP_QUERY_STATUS:
		if (copy_from_user(&query_status, (unsigned long __user *)arg, sizeof(unsigned long)))
			return -EFAULT;
		ret = xrp_query_vdsp_status(xmdev, &query_status);
		if (ret < 0) {
			xrp_print(LOG_LEVEL_ERROR, "Failed to query_vdsp_status\n");
			return -EFAULT;
		}
		if (copy_to_user((unsigned long __user *)arg, &query_status, sizeof(unsigned long)))
			return -EFAULT;

		xrp_print(LOG_LEVEL_INFO, "xrp_query_vdsp_status ret = %d\n", ret);
		break;
	case VDSP_GET_LOG_DROP_PATH:
		xrp_print(LOG_LEVEL_DEBUG, "enter VDSP_GET_LOG_DROP_PATH !!!LOG_DROP_PATH = %s", LOG_DROP_PATH);
		if (copy_to_user((unsigned long __user *)arg, LOG_DROP_PATH, sizeof(LOG_DROP_PATH))) {
			xrp_print(LOG_LEVEL_ERROR, "get VDSP_GET_LOG_DROP_PATH fail\n");
			return -EFAULT;
		}
		break;
	default:
		return -ENOTTY;
	}
	xrp_print(LOG_LEVEL_INFO, "%s end ret = %d\n", __func__, ret);
	return ret;
}

static int xmdr_cdev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct xrp_mdr_device *xmdev;
	struct mdr_register_module_result *mdr_retinfo;
	unsigned long size;
	unsigned long long pfn;
	int ret;

	xrp_print(LOG_LEVEL_INFO, "%s cdev mmap entry\n", __func__);

	xmdev = filp->private_data;
	mdr_retinfo = &xmdev->mdr_retinfo;
	size = vma->vm_end - vma->vm_start;

	if ((vma->vm_pgoff + vma_pages(vma) > mdr_retinfo->log_len >> PAGE_SHIFT)
			|| (vma->vm_pgoff + vma_pages(vma) < vma->vm_pgoff)) {
		xrp_print(LOG_LEVEL_ERROR, "%s vma size is over xrp mdr size\n", __func__);
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	pfn = (mdr_retinfo->log_addr >> PAGE_SHIFT) + vma->vm_pgoff;
	ret = remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot);
	if (ret < 0) {
		xrp_print(LOG_LEVEL_ERROR, "remap_pfn_range fail, ret%d.\n", ret);
		return ret;
	}

	xrp_print(LOG_LEVEL_INFO, "map 0x%llx to 0x%lx, size: 0x%lx\n",
		mdr_retinfo->log_addr + vma->vm_pgoff, vma->vm_start, size);
	xrp_print(LOG_LEVEL_INFO, "%s cdev mmap end\n", __func__);
	return 0;
}

static ssize_t xmdr_cdev_write(struct file *filp,
		const char __user *buf, size_t len, loff_t *pos)
{
	int ret = 0;
	return ret;
}

static ssize_t xmdr_cdev_read(struct file *filp,
		char __user *buf, size_t len, loff_t *pos)
{
	int ret = 0;
	return ret;
}

static int xmdr_cdev_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct xboot_dev *xbdev;
	struct xrp_mdr_device *xmdev = container_of(filp->private_data, struct xrp_mdr_device, miscdev);

	xrp_print(LOG_LEVEL_INFO, "%s cdev open entry\n", __func__);
	filp->private_data = xmdev;
	xbdev = xmdev->xdev->xtdev->xbdev;
	if (xbdev == NULL) {
		xrp_print(LOG_LEVEL_ERROR, "xbdev is NULL\n");
		return -EINVAL;
	}
	xrp_print(LOG_LEVEL_INFO, "%s xbdev is %pK\n", __func__, xbdev);

	if (xmdev->is_first_open_mdr) {
		xrp_print(LOG_LEVEL_INFO, "is first open xrp mdr\n");
		xmdev->is_first_open_mdr = false;
	}

	xmdev->open_count++;
	xrp_print(LOG_LEVEL_INFO, "%s, device open count is %d\n", __func__, xmdev->open_count);
	xrp_print(LOG_LEVEL_INFO, "%s cdev open end\n", __func__);
	return ret;
}

static int xmdr_cdev_release(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct xrp_mdr_device *xmdev = filp->private_data;

	xrp_print(LOG_LEVEL_INFO, "%s cdev release start\n", __func__);
	xmdev->open_count--;
	xrp_print(LOG_LEVEL_INFO, "%s, device open count is %d\n", __func__, xmdev->open_count);

	if (xmdev->open_count == 0) {
		xmdev->is_first_open_mdr = true;
		xrp_print(LOG_LEVEL_INFO, "release xmdr_cdev\n");
	}
	xrp_print(LOG_LEVEL_INFO, "%s cdev release end\n", __func__);
	return ret;
}

static const struct file_operations xmdev_fops = {
	.owner = THIS_MODULE,
	.open = xmdr_cdev_open,
	.read = xmdr_cdev_read,
	.write = xmdr_cdev_write,
	.mmap = xmdr_cdev_mmap,
	.unlocked_ioctl = xmdr_cdev_ioctl,
	.compat_ioctl = compat_ptr_ioctl,
	.fasync = xmdr_cdev_fasync,
	.release = xmdr_cdev_release,
};

int xdev_resource_release(struct xrp_task_device *xrp_task_dev)
{
	int ret = 0;

	vdsp_exception_resource_release(xrp_task_dev);
	return ret;
}

static void xmdr_vdsp_dump(u32 modid, u32 etype, u64 coreid, char *log_path, pfn_cb_dump_done pfn_cb, void *data)
{
	struct xrp_mdr_device *xmdev = NULL;

	xrp_print(LOG_LEVEL_INFO, "xmdr dump enter %s, data = %p\n", __func__, data);
	xmdev = (struct xrp_mdr_device *)data;
	if (!xmdev) {
		xrp_print(LOG_LEVEL_ERROR, "get xrp_mdr dev failed, modid 0x%x\n", modid);
		return;
	}


	strncpy(LOG_DROP_PATH, log_path, PATH_MAXLEN - 1);
	LOG_DROP_PATH[PATH_MAXLEN - 1] = '\0';

	xrp_print(LOG_LEVEL_DEBUG, "log_path: %s\n", log_path);
	xrp_print(LOG_LEVEL_DEBUG, "LOG_DROP_PATH: %s\n", LOG_DROP_PATH);

	kill_fasync(&xmdev->fasync_q, SIGUSR2, POLL_IN);

	xrp_print(LOG_LEVEL_INFO, "xmdr dump end %s\n", __func__);
}

static void xmdr_vdsp_reset(u32 modid, u32 etype, u64 coreid, void *data)
{
	int ret;
	struct xrp_mdr_device *xmdev = NULL;
	struct xrp_task_device *xrp_task_dev = NULL;

	xrp_print(LOG_LEVEL_INFO, "xmdr reset enter %s, data = %p\n", __func__, data);
	xmdev = (struct xrp_mdr_device *)data;
	if (!xmdev) {
		xrp_print(LOG_LEVEL_ERROR, "get xrp_mdr dev failed, modid 0x%x\n", modid);
		return;
	}
	xrp_task_dev = xmdev->xdev->xtdev;

	xrp_print(LOG_LEVEL_INFO, "%s xmdev %pK, xrp_task_dev %pK\n", __func__, xmdev, xrp_task_dev);
	ret = xdev_resource_release(xrp_task_dev);
	if (ret != 0)
		xrp_print(LOG_LEVEL_ERROR, "xdev resource release failed: %d\n", ret);

	xrp_print(LOG_LEVEL_INFO, "xmdr reset end %s\n", __func__);
}

int alloc_and_map_special_mdr_mem(struct xrp_task_device *xtdev,
	struct xrp_mdr_mem_info *mem, bool if_alloc, bool if_map)
{
	int ret = 0;
	struct xrp_device *xdev;
	struct xrp_mdr_device *xmdev;
	void *vdsp_mdr_va;

	xrp_print(LOG_LEVEL_DEBUG, "map special mdr mem entry %s\n", __func__);
	xdev = xtdev->xdev;
	xmdev = xdev->xmdev;
	xrp_print(LOG_LEVEL_DEBUG, "begin malloc and map mdr_log memory, addr is %08x, size is %08x\n",
		VDSP_MDR_ADDR, VDSP_MDR_SIZE);
	vdsp_mdr_va = allocate_and_map_vdsp_memory(
								xtdev,
								VDSP_MDR_SIZE,
								VDSP_MDR_ADDR,
								(dma_addr_t *)&mem->pa,
								GFP_KERNEL,
								if_alloc,
								if_map);
	if (!vdsp_mdr_va) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to allocate and map vdsp mdr memory\n");
		return -1;
	}

	xmdev->vdsp_mdr_vaddr_base = vdsp_mdr_va;
	mem->va = xmdev->vdsp_mdr_vaddr_base;
	mem->iova = VDSP_MDR_ADDR;
	mem->size = VDSP_MDR_SIZE;
	xrp_print(LOG_LEVEL_DEBUG, "mem->va %pK, mem->iova %lu, mem->pa %llx\n", mem->va, mem->iova, mem->pa);
	xrp_print(LOG_LEVEL_DEBUG, "map special mdr mem end %s\n", __func__);

	return ret;
}

void free_and_unmap_special_mdr_mem(struct xrp_task_device *xtdev, bool if_free, bool if_unmap)
{
	struct xrp_device *xdev;
	struct xrp_mdr_device *xmdev;
	struct xrp_mdr_mem_info *mem;

	xrp_print(LOG_LEVEL_DEBUG, "unmap special mdr meme entry %s\n", __func__);
	xdev = xtdev->xdev;
	xmdev = xdev->xmdev;
	mem = &xdev->xmm;

	free_and_unmap_special_memory(xtdev, VDSP_MDR_SIZE, mem->iova, mem->va,
		(dma_addr_t *)&mem->pa, if_free, if_unmap);

	xmdev->vdsp_mdr_vaddr_base = NULL;

	xrp_print(LOG_LEVEL_DEBUG, "unmap special mdr meme end %s\n", __func__);
}

static int xrp_register_mdr(struct xrp_mdr_device *xmdev)
{
	int ret = 0;
	int i;

	xrp_print(LOG_LEVEL_INFO, "xrp register mdr entry %s\n", __func__);

	xmdev->mdr_vdsp_ops.ops_dump = xmdr_vdsp_dump;
	xmdev->mdr_vdsp_ops.ops_reset = xmdr_vdsp_reset;
	xmdev->mdr_vdsp_ops.ops_data = xmdev;
	for (i = 0; i < XRP_MDR_EXP_TYPE_MAX; i++) {
		ret = mdr_register_exception(&g_hexp[i]);
		if (ret == 0)
			xrp_print(LOG_LEVEL_ERROR, "exception %d register fail, ret %d.\n", i, ret);
	}

	ret = mdr_register_module_ops(MDR_VDSP, &xmdev->mdr_vdsp_ops, &xmdev->mdr_retinfo);
	if (ret < 0) {
		xrp_print(LOG_LEVEL_ERROR, "module register fail, ret %d.\n", ret);
		ret = 0;
	}
	xrp_print(LOG_LEVEL_INFO, "log_addr= %llx, log_len=%x",
		xmdev->mdr_retinfo.log_addr, xmdev->mdr_retinfo.log_len);

	xmdev->mdr_retinfo.log_addr = (u64)DFX_MEM_VDSP_ADDR;
	xmdev->mdr_retinfo.log_len = DFX_MEM_VDSP_SIZE;
	return ret;
}

static int xrp_unregister_mdr(void)
{
	int ret = 0;

	xrp_print(LOG_LEVEL_INFO, "xrp unregister mdr entry %s\n", __func__);
	mdr_unregister_exception((u32)MODID_VDSP_WDT1_EXCEPTION);
	mdr_unregister_exception((u32)MODID_VDSP_GPIO_NS_EXCEPTION);
	mdr_unregister_module_ops(MDR_VDSP);
	xrp_print(LOG_LEVEL_INFO, "xrp unregister mdr end %s\n", __func__);
	return ret;
}

int xrp_flush_log_handler(void *arg, void *data)
{
	struct xrp_mdr_device *xmdev;
	struct xrp_log_info *info;

	xrp_print(LOG_LEVEL_DEBUG, "xrp flush log entry %s\n", __func__);
	xmdev = (struct xrp_mdr_device *)arg;
	info = xmdev->xdev->xmm.va;

	kill_fasync(&xmdev->fasync_q, SIGUSR2, POLL_IN);
	xrp_print(LOG_LEVEL_DEBUG, "%s, &xmdev->fasync_q = %pK, SIGUSR2\n", __func__, &xmdev->fasync_q);

	xrp_print(LOG_LEVEL_DEBUG, "xrp flush log exit %s\n", __func__);
	return 0;
}

static int xrp_event_notifier_unregister(struct xrp_device *xdev, struct xrp_mdr_device *xmdev, u32 event)
{
	struct xrp_notifer_block *nb = NULL, *tmp = NULL;

	list_for_each_entry_safe(nb, tmp, &xmdev->nh, node) {
		if (nb->event == event) {
			list_del(&nb->node);
			devm_kfree(&xdev->pdev->dev, nb);
			return 0;
		}
	}
	xrp_print(LOG_LEVEL_ERROR, "xrp notifier block not find\n");
	return -ENODEV;
}

void print_xrp_notifier_block(struct xrp_notifer_block *block)
{
	xrp_print(LOG_LEVEL_DEBUG, "event: %u\n", block->event);
	xrp_print(LOG_LEVEL_DEBUG, "arg: %pK\n", block->arg);
	xrp_print(LOG_LEVEL_DEBUG, "cb: %pK\n", block->cb);
}


static int xrp_event_notifier_register(struct xrp_device *xdev, u32 event,
		void *arg, xrp_notifier_cb cb)
{
	struct xrp_mdr_device *xmdev = xdev->xmdev;
	struct xrp_notifer_block *nb = NULL, *tmp = NULL;

	list_for_each_entry_safe(nb, tmp, &xmdev->nh, node) {
		if (nb->event == event) {
			xrp_print(LOG_LEVEL_ERROR, "event %d has been registered\n", event);
			return -EINVAL;
		}
	}
	nb = devm_kzalloc(&xdev->pdev->dev, sizeof(*nb), GFP_KERNEL);
	if (!nb)
		return -ENOMEM;

	nb->arg = arg;
	nb->event = event;
	nb->cb = cb;

	print_xrp_notifier_block(nb);
	list_add_tail(&nb->node, &xmdev->nh);
	xrp_print(LOG_LEVEL_INFO, "&nb->node %pK, &xmdev->nh %pK\n", &nb->node, &xmdev->nh);
	return 0;
}

static irqreturn_t xrp_exception_isr(int irq, void *dev_id)
{
	struct xrp_mdr_device *xmdev = (struct xrp_mdr_device *)dev_id;
	struct xrp_task_device *xrp_task_dev = xmdev->xdev->xtdev;
	uint32_t reg_value = 0;

	xrp_print(LOG_LEVEL_DEBUG, "xrp exception interrupt occurred on IRQ %d\n", irq);

	reg_value = hpc_smc_xrp_readreg(false);
	if (reg_value == VDSP_GPIO_LOG_INT_FLAG) {
		xrp_print(LOG_LEVEL_DEBUG, "%s--->xrp_flush_log_handler\n", __func__);
		xrp_flush_log_handler((void *)xmdev, NULL);
		hpc_smc_xrp_readreg(true);
		return IRQ_HANDLED;
	}

	xrp_task_dev->hw_ops->reset(xrp_task_dev->hw_arg);
	set_vdsp_excp_true();
	xrp_print(LOG_LEVEL_DEBUG, "set_vdsp_excp_true happen\n");

	if (irq == irq_record.wdt1_vdsp)
		mdr_system_error(MODID_VDSP_WDT1_EXCEPTION, 0, 0);
	else if (irq == irq_record.gpio_ns_vdsp)
		mdr_system_error(MODID_VDSP_GPIO_NS_EXCEPTION, 0, 0);
	else
		xrp_print(LOG_LEVEL_ERROR, "Error: irq Value does not match any record.\n");

	xrp_print(LOG_LEVEL_DEBUG, "mdr_system_error happen\n");

	hpc_smc_xrp_readreg(true);
	return IRQ_HANDLED;
}

static int xrp_register_exception_irq(struct xrp_mdr_device *xmdev)
{
	int ret = 0;
	int irq_line;
	struct platform_device *pdev;
	struct device *dev;

	pdev = xmdev->xdev->pdev;
	dev = &pdev->dev;

	irq_line = platform_get_irq(pdev, 0);
	if (irq_line < 0) {
		dev_err(dev, "Failed to get wdt1_vdsp interrupt\n");
		return irq_line;
	}
	dev_info(dev, "_wdt1_vdsp irq = %d\n", irq_line);
	irq_record.wdt1_vdsp = irq_line;
	ret = devm_request_irq(dev, irq_line, xrp_exception_isr, 0, "wdt1_vsdp", xmdev);
	if (ret) {
		dev_err(dev, "Failed to request wdt1_vdsp interrupt\n");
		return ret;
	}

	irq_line = platform_get_irq(pdev, 1);
	if (irq_line < 0) {
		dev_err(dev, "Failed to get gpio_ns_vdsp interrupt\n");
		return irq_line;
	}
	dev_info(dev, "gpio_ns_vdsp irq = %d\n", irq_line);
	irq_record.gpio_ns_vdsp = irq_line;
	ret = devm_request_irq(dev, irq_line, xrp_exception_isr, 0, "gpio_ns_vdsp", xmdev);
	if (ret) {
		dev_err(dev, "Failed to request gpio_ns_vdsp interrupt\n");
		return ret;
	}

	return ret;
}

static int xmdr_miscdev_init(struct xrp_mdr_device *xmdev)
{
	int ret = 0;
	struct miscdevice *miscdev = &xmdev->miscdev;

	miscdev->parent = &xmdev->xdev->pdev->dev;
	miscdev->minor = MISC_DYNAMIC_MINOR;
	miscdev->name = DRIVER_NAME;
	miscdev->fops = &xmdev_fops;
	ret = misc_register(miscdev);
	if (ret != 0)
		xrp_print(LOG_LEVEL_ERROR, "register xrp mdr miscdev failed: %d\n", ret);
	return ret;
}

static void xmdr_miscdev_destroy(struct xrp_mdr_device *xmdev)
{
	misc_deregister(&xmdev->miscdev);
	xrp_print(LOG_LEVEL_INFO, "unregister xrp mdr miscdev\n");
}

int xmdr_init(struct xrp_device *xdev)
{
	int ret = 0;
	struct xrp_mdr_device *xmdev = NULL;
	struct device *dev = &xdev->pdev->dev;

	xrp_print(LOG_LEVEL_INFO, "entry %s xmdr init.\n", __func__);
	//log_level = get_log_level();
	xrp_print(LOG_LEVEL_INFO, "log_level = %d\n", log_level);
	xmdev = devm_kzalloc(dev, sizeof(*xmdev), GFP_KERNEL);
	if (!xmdev) {
		xrp_print(LOG_LEVEL_INFO, "xrp mdr device kzalloc failed\n");
		return -ENOMEM;
	}
	xdev->xmdev = xmdev;
	xmdev->xdev = xdev;

	INIT_LIST_HEAD(&xmdev->nh);
	ret = xmdr_miscdev_init(xmdev);
	if (ret != 0) {
		return ret;
		xrp_print(LOG_LEVEL_ERROR, "xmdr_miscdev_init failed: %d\n", ret);
	}

	ret = xrp_register_mdr(xmdev);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "xmdr register npu failed: %d\n", ret);
		xmdr_miscdev_destroy(xmdev);
		return ret;
	}

	xmdev->is_first_open_mdr = true;

	ret = xrp_register_exception_irq(xmdev);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "xmdr register exception isr failed: %d\n", ret);
		xmdr_miscdev_destroy(xmdev);
		return ret;
	}

	ret = xrp_event_notifier_register(xdev, HE_LOG_FLUSH, xmdev, xrp_flush_log_handler);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "xrp notifier register log_flush failed: %d\n", ret);
		xmdr_miscdev_destroy(xmdev);
		return ret;
	}
	xrp_print(LOG_LEVEL_INFO, "%s xmdr init end.\n", __func__);
	return 0;
}

int xmdr_exit(struct xrp_device *xdev)
{
	int ret = 0;
	struct xrp_mdr_device *xmdev = xdev->xmdev;

	xrp_print(LOG_LEVEL_INFO, "%s mdr entry.\n", __func__);
	ret = xrp_event_notifier_unregister(xdev, xmdev, HE_LOG_FLUSH);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "xrp notifier register log_flush failed: %d\n", ret);
		return ret;
	}

	xrp_unregister_mdr();
	xrp_print(LOG_LEVEL_INFO, "xmdr_miscdev_destroy end\n");

	xmdr_miscdev_destroy(xmdev);
	xdev->xmdev = NULL;
	xrp_print(LOG_LEVEL_INFO, "xmdr_miscdev_destroy end\n");

	xrp_print(LOG_LEVEL_INFO, "%s mdr end.\n", __func__);
	return 0;
}

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("xrp MDR Driver");
MODULE_LICENSE("GPL v2");
