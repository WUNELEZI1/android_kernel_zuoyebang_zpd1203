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
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include "hpc_internal.h"

#define HPC_MDR_EXP_TYPE_MAX             3
#define HMODID                           0
#define GET_LOG_LEVEL  _IOWR('M', 0, unsigned long)
#define SET_LOG_LEVEL  _IOWR('M', 1, unsigned long)
#define GET_AINR_DUMP_REF  _IOWR('M', 2, struct hpc_ainr_dump_ref_msg)
#define GET_LOG_INFO   _IOWR('M', 3, struct hpc_log_info)
#define SET_LOG_RDP    _IOWR('M', 4, unsigned long)
#define GET_LOG_DROP_PATH    _IOWR('M', 5, unsigned long)
#define LOG_LEVEL_MAX  0xFFFFFFFF
#define PATH_MAXLEN         128

struct hpc_notifer_block {
	uint32_t event;
	void *arg;
	hpc_notifier_cb cb;
	struct list_head node;
};

struct irq_num_record {
	int wdt0;
	int lockup;
	int sysrst;
};

static char LOG_DROP_PATH[PATH_MAXLEN];
static struct irq_num_record irq_record = {0};

static struct mdr_exception_info_s g_hexp[HPC_MDR_EXP_TYPE_MAX] = {
	[0] = {
		.e_modid            = (u32)MODID_NPU_WDT0_EXCEPTION,
		.e_modid_end        = (u32)MODID_NPU_WDT0_EXCEPTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_WAIT,
		.e_notify_core_mask = MDR_NPU,
		.e_reset_core_mask  = MDR_NPU,
		.e_from_core        = MDR_NPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = NPU_S_EXCEPTION,
		.e_exce_subtype     = NPU_S_WDT0,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "NPU",
		.e_desc             = "NPU wdt0 exception",
	},
	[1] = {
		.e_modid            = (u32)MODID_NPU_LOCKUP_EXCEPTION,
		.e_modid_end        = (u32)MODID_NPU_LOCKUP_EXCEPTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_WAIT,
		.e_notify_core_mask = MDR_NPU,
		.e_reset_core_mask  = MDR_NPU,
		.e_from_core        = MDR_NPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = NPU_S_EXCEPTION,
		.e_exce_subtype     = NPU_S_LOCKUP,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "NPU",
		.e_desc             = "NPU lockup exception",
	},
	[2] = {
		.e_modid            = (u32)MODID_NPU_SYSRST_EXCEPTION,
		.e_modid_end        = (u32)MODID_NPU_SYSRST_EXCEPTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_WAIT,
		.e_notify_core_mask = MDR_NPU,
		.e_reset_core_mask  = MDR_NPU,
		.e_from_core        = MDR_NPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = NPU_S_EXCEPTION,
		.e_exce_subtype     = NPU_S_SYSRST,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "NPU",
		.e_desc             = "NPU sysrst exception",
	},
};

static void hmdr_npu_dump(u32 modid, u32 etype, u64 coreid, char *log_path,
				pfn_cb_dump_done pfn_cb, void *data)
{
	struct hpc_mdr_device *hmdev = NULL;

	strncpy(LOG_DROP_PATH, log_path, PATH_MAXLEN - 1);
	LOG_DROP_PATH[PATH_MAXLEN - 1] = '\0';
	hpcdbg("log_path: %s\n", log_path);
	hpcdbg("LOG_DROP_PATH: %s\n", LOG_DROP_PATH);

	hpcdbg("entry %s, data ptr is %p\n", __func__, data);
	hmdev = (struct hpc_mdr_device *)data;
	if (!hmdev) {
		hpcerr("get hmdr dev failed, modid 0x%x\n", modid);
		return;
	}
#if IS_ENABLED(CONFIG_XRING_DEBUG)
	kill_fasync(&hmdev->fasync_q, SIGRTMAX, POLL_IN);
#endif

	hpcdbg("mdr_npu_dump\n");
}

static void hmdr_npu_reset(u32 modid, u32 etype, u64 coreid, void *data)
{
	struct hpc_mdr_device *hmdev = NULL;
	int ret;

	hpcinfo("entry\n");
	hpcdbg("entry %s, data ptr is %p\n", __func__, data);

	set_npu_excp_true();
	hmdev = (struct hpc_mdr_device *)data;
	if (!hmdev) {
		hpcerr("get hmdr dev failed, modid 0x%x\n", modid);
		return;
	}

	ret = hdev_resource_release(hmdev->hdev);
	if (ret != 0)
		hpcerr("hdev resource release failed: %d\n", ret);

	hpcdbg("reset npu-mcu ok\n");
}

static int hpc_register_mdr(struct hpc_mdr_device *hmdev)
{
	int ret = 0;
	struct hpc_device *hdev = NULL;
	struct hpc_mem_info *mem = NULL;
	struct hpc_log_info *log_info = NULL;
	struct device *dev = NULL;
	int i;

	hpcinfo("entry\n");
	hpcdbg("struct hpc_log_info size %lu\n", sizeof(struct hpc_log_info));
	hmdev->mdr_npu_ops.ops_dump = hmdr_npu_dump;
	hmdev->mdr_npu_ops.ops_reset = hmdr_npu_reset;
	hmdev->mdr_npu_ops.ops_data = hmdev;

	for (i = 0; i < HPC_MDR_EXP_TYPE_MAX; i++) {
		ret = mdr_register_exception(&g_hexp[i]);
		if (ret == 0) {
			hpcerr("exception %d register fail, ret %d.\n", i, ret);
			return ret;
		}
	}

	ret = mdr_register_module_ops(MDR_NPU, &hmdev->mdr_npu_ops, &hmdev->mdr_retinfo);//npu没有mdr的unregister
	if (ret < 0) {
		hpcerr("module register fail, ret %d.\n", ret);
		ret = 0;
	}
	hdev = hmdev->hdev;

	dev = &hdev->pdev->dev;
	mem = &hdev->hbm[HBM_HMDR_TYPE];
	mem->pa = DFX_MEM_NPU_ADDR;
	mem->size = DFX_MEM_NPU_SIZE;
	mem->va = mdr_map(mem->pa, mem->size);

	hpcdbg("hmdrmem memory va %pK, pa %pK\n", mem->va, (void *)mem->pa);
	hmdev->mdr_retinfo.log_addr = mem->pa;
	hmdev->mdr_retinfo.log_len = mem->size;

	log_info = &hmdev->g_hli;
	log_info->writep = 0;
	log_info->readp = 0;

	hpcdbg("hmdrmem: fd:%d, size:0x%x, prot:%d, da:%pK, pa:%pK\n",
		hdev->hbm[HBM_HMDR_TYPE].fd, hdev->hbm[HBM_HMDR_TYPE].size,
		hdev->hbm[HBM_HMDR_TYPE].prot, (void *)(long)hdev->hbm[HBM_HMDR_TYPE].da,
		(void *)hdev->hbm[HBM_HMDR_TYPE].pa);

	hpcinfo("end\n");

	return ret;
}

static irqreturn_t hpc_exception_isr(int irq, void *dev_id)
{
	struct hpc_mdr_device *hmdev = (struct hpc_mdr_device *)dev_id;
	struct hpc_device *hdev = hmdev->hdev;
	int ret = 0;

	hpcdbg("exception interrupt occurred on IRQ %d\n", irq);

	set_npu_excp_true();
	ret = hdev_shutdown_atomic(hmdev->hdev);
	if (ret != 0) {
		hpcerr("hpc is already shutdown, ret:%d", ret);
		return IRQ_HANDLED;
	}

	if (hdev_booted(hdev)) {
		if (hmdev->p_hli != NULL) {
			hmdev->g_hli.log_drop = hmdev->p_hli->log_drop;
			hmdev->g_hli.writep = hmdev->p_hli->writep;//g_hli为全局变量
		} else {
			hpcerr("hmdev->p_hli is NULL, happen error");
		}
	}

	//npu exception happen. move kill_signal to mdr_dump
	if (irq == irq_record.wdt0) {
		hpcdbg("exception interrupt occurred on IRQ %d  MODID_NPU_WDT0_EXCEPTION\n", irq);
		mdr_system_error(MODID_NPU_WDT0_EXCEPTION, 0, 0);
	} else if (irq == irq_record.lockup) {
		hpcdbg("exception interrupt occurred on IRQ %d  MODID_NPU_LOCKUP_EXCEPTION\n", irq);
		mdr_system_error(MODID_NPU_LOCKUP_EXCEPTION, 0, 0);
	} else if (irq == irq_record.sysrst) {
		hpcdbg("exception interrupt occurred on IRQ %d  MODID_NPU_SYSRST_EXCEPTION\n", irq);
		mdr_system_error(MODID_NPU_SYSRST_EXCEPTION, 0, 0);
	} else {
		hpcerr("Error: irq Value does not match any record.\n");
	}

	return IRQ_HANDLED;
}

static int hpc_register_exception_irq(struct hpc_mdr_device *hmdev)
{
	int ret = 0;
	int irq_line;
	struct platform_device *pdev;
	struct device *dev;

	pdev = hmdev->hdev->pdev;
	dev = &pdev->dev;

	irq_line = platform_get_irq(pdev, 0);
	if (irq_line < 0) {
		dev_err(dev, "Failed to get wdt0 interrupt\n");
		return irq_line;
	}
	dev_info(dev, "wdt0_irq = %d\n", irq_line);
	irq_record.wdt0 = irq_line;
	ret = devm_request_irq(dev, irq_line, hpc_exception_isr, 0, "wdt0", hmdev);
	if (ret) {
		dev_err(dev, "Failed to request wdt0 interrupt\n");
		return ret;
	}

	irq_line = platform_get_irq(pdev, 1);
	if (irq_line < 0) {
		dev_err(dev, "Failed to get lockup interrupt\n");
		return irq_line;
	}
	dev_info(dev, "lockup_irq = %d\n", irq_line);
	irq_record.lockup = irq_line;
	ret = devm_request_irq(dev, irq_line, hpc_exception_isr, 0, "lockup", hmdev);
	if (ret) {
		dev_err(dev, "Failed to request lockup interrupt\n");
		return ret;
	}

	irq_line = platform_get_irq(pdev, 2);
	if (irq_line < 0) {
		dev_err(dev, "Failed to get sysrst interrupt\n");
		return irq_line;
	}
	dev_info(dev, "sysrst_irq = %d\n", irq_line);
	irq_record.sysrst = irq_line;
	ret = devm_request_irq(dev, irq_line, hpc_exception_isr, 0, "sysrst", hmdev);
	if (ret) {
		dev_err(dev, "Failed to request sysrst interrupt\n");
		return ret;
	}

	return ret;
}

int hpc_flush_log_handler(void *arg, void *data)
{
	struct hpc_mdr_device *hmdev;

	hpcdbg("entry\n");
	hmdev = (struct hpc_mdr_device *)arg;
	hmdev->g_hli.writep = hmdev->p_hli->writep;
#if IS_ENABLED(CONFIG_XRING_DEBUG)
	kill_fasync(&hmdev->fasync_q, SIGRTMAX, POLL_IN);
#endif
	hpcdbg("end\n");

	return 0;
}

int hpc_ainr_dump_ref_handler(void *arg, void *data)
{
	struct hpc_mdr_device *hmdev;
	struct hpc_ainr_dump_ref_msg *msg;

	hpcdbg("entry\n");

	hmdev = (struct hpc_mdr_device *)arg;
	msg = (struct hpc_ainr_dump_ref_msg *)data;
	memcpy(&hmdev->ainr_dump_ref_msg, msg, sizeof(struct hpc_ainr_dump_ref_msg));
	hmdev->ainr_dump_msg_ready = 1;
	wake_up_interruptible(&hmdev->wait);

	hpcdbg("end\n");

	return 0;
}

int hpc_cancel_pid_all_handler(void *arg, void *data)
{
	struct hpc_mdr_device *hmdev;
	struct hpc_device *hdev;

	hpcdbg("entry\n");

	hmdev = (struct hpc_mdr_device *)arg;
	hdev = hmdev->hdev;
	hdev->cancel_all_flag = true;
	wake_up(&hdev->cancel_all_wait);

	hpcdbg("end\n");

	return 0;
}

#if IS_ENABLED(CONFIG_XRING_DEBUG)
static int hpc_set_log_level(struct hpc_mdr_device *hmdev, unsigned long log_level)
{
	int ret = 0;
	struct hpc_device *hdev = NULL;
	struct hpc_event_msg msg;

	hpcdbg("entry\n");
	hpcdbg("log_level = 0x%lx\n", log_level);
	if (!hmdev)
		goto out;
	hmdev->g_hli.log_level = log_level;
	hdev = hmdev->hdev;
	if (atomic_read(&hdev->power) == 0)
		goto out;
	msg.len = sizeof(msg) - sizeof(msg.len);
	msg.type = HE_LOG_LEVEL_SET;
	msg.msg.hllm.log_level = log_level;
	ret = hpc_send_event(hdev, &msg);
	if (ret < 0) {
		hpcerr("hpc send event failed: %d\n", ret);
		return ret;
	}
	hpcdbg("end\n");

out:
	return ret;
}

static int hpc_get_log_level(struct hpc_mdr_device *hmdev, unsigned long *log_level)
{
	struct hpc_log_info *log_info;
	struct hpc_device *hdev = NULL;

	hpcdbg("entry\n");
	if (!hmdev)
		goto out;
	hdev = hmdev->hdev;
	if (atomic_read(&hdev->power) == 0) {
		*log_level = hmdev->g_hli.log_level;
		goto out;
	}
	mutex_lock(&hdev->lock);
	log_info = hmdev->p_hli;
	*log_level = log_info->log_level;
	mutex_unlock(&hdev->lock);
	hpcdbg("end\n");

out:
	return 0;
}

static ssize_t pressure_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long log_level;
	struct hpc_device *hdev = dev_get_drvdata(dev);
	struct hpc_mdr_device *hmdev;
	int ret;

	hpcinfo("entry\n");
	if (kstrtoul(buf, 0, &log_level))
		return -EAGAIN;
	if (log_level > LOG_LEVEL_MAX) {
		hpcerr("log_level is llegal para, %lu\n", log_level);
		return -EAGAIN;
	}
	hpcinfo("log_level: %lu\n", log_level);
	hmdev = hdev->hmdev;
	ret = hpc_set_log_level(hmdev, log_level);
	if (ret < 0)
		hpcerr("hpc set log level fail\n");

	hpcinfo("end\n");
	return count;
}

static DEVICE_ATTR_WO(pressure);

static ssize_t state_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned long log_level;
	struct hpc_device *hdev = dev_get_drvdata(dev);
	struct hpc_mdr_device *hmdev;
	int ret;

	hpcinfo("entry\n");
	hmdev = hdev->hmdev;
	ret = hpc_get_log_level(hmdev, &log_level);
	if (ret != 0)
		hpcerr("hpc set log level fail\n");


	return snprintf(buf, 8, "%lu\n", log_level);
}

static DEVICE_ATTR_RO(state);

static struct attribute *hmdr_attributes[] = {
	&dev_attr_state.attr,
	&dev_attr_pressure.attr,
	NULL
};

static const struct attribute_group hmdr_attr_group = {
	.name = "hmdr_log",
	.attrs = hmdr_attributes,
};

static inline struct hpc_mdr_device *miscdev_to_hmdev(struct miscdevice *miscdev)
{
	return container_of(miscdev, struct hpc_mdr_device, miscdev);
}

static int hmdr_cdev_open(struct inode *inode, struct file *filp)
{
	struct hpc_mdr_device *hmdev = miscdev_to_hmdev(filp->private_data);

	hpcinfo("entry\n");
	filp->private_data = hmdev;
	hpcinfo("end\n");
	return 0;
}

static ssize_t hmdr_cdev_read(struct file *filp,
		char __user *buf, size_t len, loff_t *pos)
{
	int ret = 0;

	return ret;
}

static ssize_t hmdr_cdev_write(struct file *filp,
		const char __user *buf, size_t len, loff_t *pos)
{
	int ret = 0;

	return ret;
}

static __poll_t hmdr_cdev_poll(struct file *filp, struct poll_table_struct *poll_table)
{
	struct hpc_mdr_device *hmdev;
	__poll_t mask = 0;

	hpcdbg("entry\n");
	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -EPERM;
	}
	hmdev = filp->private_data;

	poll_wait(filp, &hmdev->wait, poll_table);

	if (hmdev->ainr_dump_msg_ready)
		mask |= POLLIN;
	else
		mask = 0;

	hpcdbg("end\n");
	return mask;
}


static int hmdr_cdev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct hpc_mdr_device *hmdev;
	struct mdr_register_module_result *mdr_retinfo;
	unsigned long size;
	unsigned long long pfn;
	int ret = 0;

	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -EPERM;
	}
	hmdev = filp->private_data;
	mdr_retinfo = &hmdev->mdr_retinfo;
	size = vma->vm_end - vma->vm_start;

	if ((vma->vm_pgoff + vma_pages(vma) > mdr_retinfo->log_len >> PAGE_SHIFT)
			|| (vma->vm_pgoff + vma_pages(vma) < vma->vm_pgoff)) {
		hpcerr("vma size is over hpc mdr size\n");
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	pfn = (mdr_retinfo->log_addr >> PAGE_SHIFT) + vma->vm_pgoff;
	ret = remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot);
	if (ret < 0) {
		hpcerr("remap_pfn_range fail, ret%d.\n", ret);
		return ret;
	}

	return 0;
}

static long hmdr_cdev_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	int ret = 0;
	unsigned long log_level = 0;
	unsigned long readp = 0;
	struct hpc_mdr_device *hmdev;
	struct hpc_device *hdev = NULL;
	struct mdr_register_module_result *mdr_retinfo;

	hpcdbg("entry\n");
	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -EPERM;
	}
	hmdev = filp->private_data;
	mdr_retinfo = &hmdev->mdr_retinfo;
	hdev = hmdev->hdev;

	switch (cmd) {
	case SET_LOG_LEVEL:
		if (copy_from_user(&log_level, (unsigned long __user *)arg, sizeof(unsigned long)))
			return -EFAULT;

		ret = hpc_set_log_level(hmdev, log_level);
		if (ret < 0)
			hpcerr("hpc set log level fail\n");

		break;
	case GET_AINR_DUMP_REF:
		if (copy_to_user((unsigned long __user *)arg, &hmdev->ainr_dump_ref_msg, sizeof(struct hpc_ainr_dump_ref_msg)))
			hpcerr("get hpc ainr_dump_ref_msg fail\n");

		hmdev->ainr_dump_msg_ready = 0;
		break;
	case GET_LOG_INFO:
		hpcdbg("hmdev->g_hli.log_drop =%d, hmdev->g_hli.wdt_enable  =%d\n", hmdev->g_hli.log_drop, hmdev->g_hli.wdt_enable);
		if (copy_to_user((unsigned long __user *)arg, &hmdev->g_hli, sizeof(struct hpc_log_info))) {
			hpcerr("get hpc log info fail\n");
			return -EFAULT;
		}
		break;
	case SET_LOG_RDP:
		if (copy_from_user(&readp, (unsigned int __user *)arg, sizeof(unsigned long))) {
			hpcerr("hpc set log readp fail\n");
			return -EFAULT;
		}
		mutex_lock(&hdev->lock);
		hmdev->g_hli.readp = readp;
		if (hdev_booted(hdev)) {
			if (hmdev->p_hli)
				hmdev->p_hli->readp = readp;
		}
		mutex_unlock(&hdev->lock);
		break;
	case GET_LOG_DROP_PATH:
		hpcdbg("enter GET_LOG_DROP_PATH !!!LOG_DROP_PATH = %s", LOG_DROP_PATH);
		if (copy_to_user((unsigned long __user *)arg, LOG_DROP_PATH, sizeof(LOG_DROP_PATH))) {
			hpcerr("get LOG_DROP_PATH fail\n");
			return -EFAULT;
		}
		break;
	default:
		return -ENOTTY;
	}

	hpcdbg("end\n");
	return ret;
}

static int hmdr_cdev_fasync(int fd, struct file *filp, int mode)
{
	int ret = 0;
	struct hpc_mdr_device *hmdev;

	hpcinfo("entry\n");
	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -EPERM;
	}
	hmdev = filp->private_data;
	ret = fasync_helper(fd, filp, mode, &hmdev->fasync_q);
	hpcinfo("end\n");

	return ret;
}

static int hmdr_cdev_release(struct inode *inode, struct file *filp)
{
	int ret = 0;

	hpcinfo("entry\n");
	filp->private_data = NULL;
	hpcinfo("end\n");

	return ret;
}

static const struct file_operations hmdev_fops = {
	.owner = THIS_MODULE,
	.open = hmdr_cdev_open,
	.read = hmdr_cdev_read,
	.write = hmdr_cdev_write,
	.poll = hmdr_cdev_poll,
	.mmap = hmdr_cdev_mmap,
	.unlocked_ioctl = hmdr_cdev_ioctl,
	.compat_ioctl = compat_ptr_ioctl,
	.fasync = hmdr_cdev_fasync,
	.release = hmdr_cdev_release,
};

static int hmdr_miscdev_init(struct hpc_mdr_device *hmdev)
{
	struct miscdevice *miscdev = &hmdev->miscdev;
	int ret;

	miscdev->parent = &hmdev->hdev->pdev->dev;
	miscdev->minor = MISC_DYNAMIC_MINOR;
	miscdev->name = "hpc-mdr";
	miscdev->fops = &hmdev_fops;
	ret = misc_register(miscdev);
	if (ret != 0)
		hpcerr("register hpc mdr miscdev failed: %d\n", ret);

	return ret;
}

static void hmdr_miscdev_destroy(struct hpc_mdr_device *hmdev)
{
	misc_deregister(&hmdev->miscdev);
}
#else
static int hmdr_miscdev_init(struct hpc_mdr_device *hmdev)
{
	return 0;
}
static void hmdr_miscdev_destroy(struct hpc_mdr_device *hmdev)
{

}
#endif

int hpc_event_notifier_register(struct hpc_device *hdev, u32 event,
		void *arg, hpc_notifier_cb cb)
{
	struct hpc_mdr_device *hmdev = hdev->hmdev;
	struct hpc_notifer_block *nb = NULL, *tmp = NULL;

	list_for_each_entry_safe(nb, tmp, &hmdev->nh, node) {
		if (nb->event == event) {
			hpcerr("event has been registered\n");
			return -EINVAL;
		}
	}

	nb = devm_kzalloc(&hdev->pdev->dev, sizeof(*nb), GFP_KERNEL);
	if (!nb) {
		hpcerr("alloc hpc notifier block memory failed\n");
		return -ENOMEM;
	}

	nb->arg = arg;
	nb->event = event;
	nb->cb = cb;
	list_add_tail(&nb->node, &hmdev->nh);

	return 0;
}
EXPORT_SYMBOL_GPL(hpc_event_notifier_register);

int hpc_event_notifier_unregister(struct hpc_device *hdev, u32 event)
{
	struct hpc_mdr_device *hmdev = hdev->hmdev;
	struct hpc_notifer_block *nb = NULL, *tmp = NULL;

	list_for_each_entry_safe(nb, tmp, &hmdev->nh, node) {
		if (nb->event == event) {
			list_del(&nb->node);
			devm_kfree(&hdev->pdev->dev, nb);
			return 0;
		}
	}

	hpcerr("hpc notifier block not find\n");

	return -ENODEV;
}
EXPORT_SYMBOL_GPL(hpc_event_notifier_unregister);

int hpc_send_event(struct hpc_device *hdev, void *msg)
{
	struct hpc_mdr_device *hmdev = NULL;
	int ret;

	hpcinfo("entry\n");
	if (!msg || !hdev)
		return -EFAULT;

	if (!hdev_booted(hdev))
		return -ENODEV;

	hmdev = hdev->hmdev;
	if (!hmdev->tx_ch) {
		hpcerr("mdr tx channel not initialized\n");
		return -ENODEV;
	}

	ret = mbox_send_message(hmdev->tx_ch, (void *)msg);
	if (ret < 0)
		hpcerr("send msg failed: %d\n", ret);

	hpcinfo("end\n");
	return ret;
}
EXPORT_SYMBOL_GPL(hpc_send_event);

static void hmdr_mbox_rx_callback(struct mbox_client *cl, void *msg)
{
	struct hpc_device *hdev = dev_get_drvdata(cl->dev);
	struct hpc_mdr_device *hmdev = hdev->hmdev;
	struct hpc_event_msg *em = (struct hpc_event_msg *)msg;
	struct hpc_notifer_block *nb = NULL, *tmp = NULL;
	int ret;

	list_for_each_entry_safe(nb, tmp, &hmdev->nh, node) {
		if (nb->event != em->type)
			continue;

		if (!nb->cb) {
			hpcerr("event cb is NULL\n");
			return;
		}

		ret = nb->cb(nb->arg, (void *)&em->msg);
		if (ret != 0)
			hpcerr("event cb process failed: %d\n", ret);

		return;
	}
}

static int hmdr_mbox_init(struct hpc_mdr_device *hmdev)
{
	struct device *dev = &hmdev->hdev->pdev->dev;
	struct mbox_client *cl;

	hpcinfo("entry\n");
	if (!of_get_property(dev->of_node, "mbox-names", NULL))
		return -EFAULT;

	cl = &hmdev->cl;
	cl->dev = dev;
	cl->tx_block = true;
	cl->tx_tout = 0;
	cl->knows_txdone = false;
	cl->rx_callback = hmdr_mbox_rx_callback;

	hmdev->tx_ch = mbox_request_channel_byname(cl, "mdr-tx");
	if (IS_ERR(hmdev->tx_ch)) {
		hpcerr("mailbox tx channel request failed\n");
		return PTR_ERR(hmdev->tx_ch);
	}

	hmdev->rx_ch = mbox_request_channel_byname(cl, "mdr-rx");
	if (IS_ERR(hmdev->rx_ch)) {
		hpcerr("mailbox rx channel request failed\n");
		mbox_free_channel(hmdev->tx_ch);
		return PTR_ERR(hmdev->rx_ch);
	}
	hpcinfo("end\n");

	return 0;
}

static void hmdr_mbox_deinit(struct hpc_mdr_device *hmdev)
{
	hpcinfo("entry\n");
	if (hmdev->tx_ch)
		mbox_free_channel(hmdev->tx_ch);
	if (hmdev->rx_ch)
		mbox_free_channel(hmdev->rx_ch);
	hmdev->tx_ch = NULL;
	hmdev->rx_ch = NULL;
	hpcinfo("end\n");
}

int hmdr_init(struct hpc_device *hdev)
{
	struct hpc_mdr_device *hmdev = NULL;
	struct device *dev = &hdev->pdev->dev;
	int ret;

	hpcinfo("entry\n");
	hmdev = devm_kzalloc(dev, sizeof(*hmdev), GFP_KERNEL);
	if (!hmdev) {
		hpcerr("hpc mdr device zalloc failed\n");
		return -ENOMEM;
	}

	init_waitqueue_head(&hmdev->wait);
	hmdev->ainr_dump_msg_ready = 0;
	hmdev->hdev = hdev;
	hdev->hmdev = hmdev;
	INIT_LIST_HEAD(&hmdev->nh);
	ret = hmdr_miscdev_init(hmdev);
	if (ret != 0)
		return ret;

	ret = hmdr_mbox_init(hmdev);
	if (ret != 0) {
		hpcerr("mdr mbox init failed: %d\n", ret);
		hmdr_miscdev_destroy(hmdev);
		return ret;
	}

	ret = hpc_register_mdr(hmdev);
	if (ret != 0) {
		hpcerr("mdr register npu failed: %d\n", ret);
		hmdr_miscdev_destroy(hmdev);
		return ret;
	}

	ret = hpc_register_exception_irq(hmdev);
	if (ret != 0) {
		hpcerr("hmdr register exception isr failed: %d\n", ret);
		hmdr_miscdev_destroy(hmdev);
		return ret;
	}

	ret = hpc_event_notifier_register(hdev, HE_LOG_FLUSH, hmdev, hpc_flush_log_handler);
	if (ret != 0) {
		hpcerr("hpc notifier register log_flush failed: %d\n", ret);
		hmdr_miscdev_destroy(hmdev);
		return ret;
	}

	ret = hpc_event_notifier_register(hdev, HE_AINR_DUMP_REF, hmdev, hpc_ainr_dump_ref_handler);
	if (ret != 0) {
		hpcerr("hpc notifier register dump_ainr_ref failed: %d\n", ret);
		hmdr_miscdev_destroy(hmdev);
		return ret;
	}

	ret = hpc_event_notifier_register(hdev, HE_CANCEL_TPID_ALL, hmdev, hpc_cancel_pid_all_handler);
	if (ret != 0) {
		hpcerr("hpc notifier register cancel_pid failed: %d\n", ret);
		hmdr_miscdev_destroy(hmdev);
		return ret;
	}

#if IS_ENABLED(CONFIG_XRING_DEBUG)
	ret = sysfs_create_group(&dev->kobj, &hmdr_attr_group);
	if (ret < 0)
		dev_err(dev, "attr group create failed\n");
	else
		dev_info(dev, "attr group create success!\n");
#endif

	hpcinfo("end\n");

	return 0;
}
EXPORT_SYMBOL_GPL(hmdr_init);

int hmdr_exit(struct hpc_device *hdev)
{
	struct hpc_mem_info *mem = NULL;
	struct hpc_mdr_device *hmdev = hdev->hmdev;

	hpcinfo("entry\n");
	hpc_event_notifier_unregister(hdev, HE_LOG_FLUSH);
	hpc_event_notifier_unregister(hdev, HE_AINR_DUMP_REF);
	hmdr_mbox_deinit(hmdev);
	hmdr_miscdev_destroy(hmdev);
	hdev->hmdev = NULL;
#if IS_ENABLED(CONFIG_XRING_DEBUG)
	struct device *dev = &hdev->pdev->dev;
	sysfs_remove_group(&dev->kobj, &hmdr_attr_group);
#endif
	mem = &hdev->hbm[HBM_HMDR_TYPE];
	mdr_unmap(mem->va);
	hpcinfo("end\n");

	return 0;
}
EXPORT_SYMBOL_GPL(hmdr_exit);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC MDR Driver");
MODULE_LICENSE("GPL v2");
