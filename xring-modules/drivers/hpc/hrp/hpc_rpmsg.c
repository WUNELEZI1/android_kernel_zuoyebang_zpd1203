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
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/rpmsg.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/genalloc.h>
#include "hpc_internal.h"
#include <linux/jiffies.h>

#if IS_ENABLED(CONFIG_XRING_DEBUG)
int hpc_log_level = HPC_LOG_LEVEL_INFO;
module_param(hpc_log_level, int, 0644);
MODULE_PARM_DESC(hpc_log_level, "Control log level (0=none, 1=info, ...)");
#endif

struct hpc_rpmsg_device {
	struct rpmsg_device *rpdev;
	struct hpc_device *hdev;
	struct miscdevice miscdev;
	struct mutex mutex;
	struct list_head epts;
	struct hpc_pid_info *pid_info;
};

struct hpc_ept_device {
	struct rpmsg_endpoint *ept;
	void *priv;
	struct rpmsg_channel_info chinfo;
	wait_queue_head_t readq;
	struct list_head rvq;
	struct list_head mem;
	struct list_head node;
	struct mutex mutex;
	struct mutex map_lock;
	struct file *filp;
	pid_t pid;
	atomic_t fd_closed;
	atomic_t if_memmap;
};

struct hpc_rpc_msg {
	void *data;
	int len;
	struct list_head node;
};

struct pid_record {
	pid_t pid;
	bool flag;
	atomic_t count;
	struct list_head list;
};

struct hpc_pid_info {
	atomic_t total_count;
	struct list_head head;
	struct mutex mutex;
};

struct npu_msg_header {
	uint32_t process_id;
	uint32_t thread_id;
	uint32_t cmd;
	uint32_t msg_id;
};

struct npu_cancel_all_stream_para {
	struct npu_msg_header header;
	uint32_t pid;
};

#define HDEV_DMA_BUFF_MAP \
	_IOWR('X', 0, struct hpc_rpmsg_mem_info)

#define HDEV_DMA_BUFF_UNMAP \
	_IOWR('X', 1, struct hpc_rpmsg_mem_info)

#define HDEV_RPMSG_FD_CLOSE \
	_IO('X', 2)

#define WAIT_TIMEOUT              5000
#define NPU_IPC_CMD_CANCEL_PID    100
#define NPU_IPC_CMD_CANCEL_MSGID  0x1001

struct hpc_rpmsg_device *g_hrpdev;
struct hpc_pid_info *g_pid_info;

bool hpc_rpmsg_is_pid_not_in_epts(struct hpc_rpmsg_device *rdev, pid_t pid)
{
	struct hpc_ept_device *ept;
	bool is_not_in_epts = true;

	if (!rdev) {
		hpcerr("invalid rdev (NULL)\n");
		return true;
	}

	mutex_lock(&rdev->mutex);

	list_for_each_entry(ept, &rdev->epts, node) {
		if (ept->pid == pid) {
			is_not_in_epts = false;
			break;
		}
	}

	mutex_unlock(&rdev->mutex);

	return is_not_in_epts;
}

static inline void hpc_pid_info_init(struct hpc_pid_info *info)
{
	atomic_set(&info->total_count, 0);
	INIT_LIST_HEAD(&info->head);
	mutex_init(&info->mutex);
}

int hpc_pid_info_add(struct hpc_pid_info *info, pid_t pid)
{
	struct pid_record *entry;
	int new_count = 0;

	if (!info) {
		hpcerr("invalid info (NULL)\n");
		return -EINVAL;
	}

	mutex_lock(&info->mutex);

	list_for_each_entry(entry, &info->head, list) {
		if (entry->pid == pid) {
			new_count = atomic_inc_return(&entry->count);
			hpcdbg("pid %d already exists, inc count to %d\n",
					pid, new_count);

			mutex_unlock(&info->mutex);
			return new_count;
		}
	}

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry) {
		hpcerr("kzalloc failed\n");
		mutex_unlock(&info->mutex);
		return -ENOMEM;
	}

	entry->pid  = pid;
	entry->flag = false;
	atomic_set(&entry->count, 1);
	list_add_tail(&entry->list, &info->head);
	atomic_inc(&info->total_count);

	new_count = 1;

	hpcdbg("pid %d added, total_count now %d\n",
		pid, atomic_read(&info->total_count));

	mutex_unlock(&info->mutex);
	return new_count;
}

int hpc_pid_info_del(struct hpc_pid_info *info, pid_t pid)
{
	struct pid_record *entry, *tmp;
	int ret = -EINVAL;

	if (!info) {
		hpcerr("invalid info (NULL)\n");
		return -EINVAL;
	}

	mutex_lock(&info->mutex);

	list_for_each_entry_safe(entry, tmp, &info->head, list) {
		if (entry->pid == pid) {
			list_del(&entry->list);
			kfree(entry);
			atomic_dec(&info->total_count);

			hpcdbg("pid %d removed, total_count now %d\n",
			       pid, atomic_read(&info->total_count));

			ret = 0;
			break;
		}
	}

	if (ret < 0)
		hpcdbg("pid %d not found\n", pid);

	mutex_unlock(&info->mutex);
	return ret;
}


bool hpc_pid_info_find(struct hpc_pid_info *info, pid_t pid)
{
	struct pid_record *entry;
	bool found = false;

	if (!info) {
		hpcerr("invalid info (NULL)\n");
		return false;
	}

	if (atomic_read(&info->total_count) == 0) {
		hpcdbg("total_count=0, no pid found\n");
		return false;
	}

	mutex_lock(&info->mutex);

	list_for_each_entry(entry, &info->head, list) {
		if (entry->pid == pid) {
			found = true;
			hpcdbg("found pid %d\n", pid);
			break;
		}
	}

	if (!found)
		hpcdbg("pid %d not in list\n", pid);

	mutex_unlock(&info->mutex);
	return found;
}


void hpc_pid_info_destroy(struct hpc_pid_info *info)
{
	struct pid_record *entry, *tmp;

	if (!info)
		return;

	atomic_set(&info->total_count, 0);
	mutex_lock(&info->mutex);

	list_for_each_entry_safe(entry, tmp, &info->head, list) {
		list_del(&entry->list);
		kfree(entry);
	}

	mutex_unlock(&info->mutex);
}

static ssize_t hrpdev_ept_write(struct hpc_ept_device *heptdev, const char *buf, size_t len)
{
	struct hpc_rpmsg_device *hrpdev;
	struct hpc_virtio_device *hvdev;
	int ret = 0;
	void *kbuf;

	if (!heptdev || !heptdev->priv)
		return -EPIPE;

	hrpdev = (struct hpc_rpmsg_device *)heptdev->priv;
	kbuf = kzalloc(len, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	memcpy(kbuf, buf, len);

	hvdev = hrpdev->hdev->hvdev;
	if (!hvdev) {
		ret = -EPIPE;
		goto free_kbuf;
	}

	mutex_lock(&hvdev->lock);
	if (atomic_read(&hvdev->power) == 0) {
		ret = -EFAULT;
		mutex_unlock(&hvdev->lock);
		goto free_kbuf;
	}

	ret = rpmsg_sendto(heptdev->ept, kbuf, len, heptdev->chinfo.dst);
	mutex_unlock(&hvdev->lock);

free_kbuf:
	kfree(kbuf);
	return ret < 0 ? ret : len;
}

void send_npu_cancel_all_stream(struct hpc_ept_device *heptdev, uint32_t pid)
{
	struct npu_cancel_all_stream_para *msg;
	ssize_t ret;

	hpcdbg("entry\n");

	if (!heptdev) {
		hpcerr("heptdev pointer is null\n");
		return;
	}

	msg = kmalloc(sizeof(struct npu_cancel_all_stream_para), GFP_KERNEL);
	if (!msg) {
		hpcerr("Failed to allocate memory for npu_cancel_all_stream_para\n");
		return;
	}

	msg->header.process_id = current->pid;
	msg->header.thread_id = current->tgid;
	msg->header.cmd = NPU_IPC_CMD_CANCEL_PID;
	msg->header.msg_id = NPU_IPC_CMD_CANCEL_MSGID;
	msg->pid = pid;

	ret = hrpdev_ept_write(heptdev, (const char *)msg, sizeof(struct npu_cancel_all_stream_para));
	if (ret < 0)
		hpcerr("hrpdev_ept_write failed: %ld\n", ret);

	kfree(msg);
	hpcdbg("end\n");
}

static inline struct hpc_device *rpdev_to_hdev(struct rpmsg_device *rpdev)
{
	struct device *dev = rpdev->dev.parent->parent->parent;
	struct platform_device *pdev = to_platform_device(dev);

	return platform_get_drvdata(pdev);
}

static inline struct hpc_rpmsg_device *rpdev_to_hrpdev(struct rpmsg_device *rpdev)
{
	return rpdev->ept->priv;
}

static inline struct hpc_rpmsg_device *miscdev_to_hrpdev(struct miscdevice *miscdev)
{
	return container_of(miscdev, struct hpc_rpmsg_device, miscdev);
}

static int hrpdev_buf_map(struct hpc_device *hdev,
		struct hpc_ept_device *heptdev, void __user *argp)
{
	struct hpc_rpmsg_mem_node *mem = NULL;
	int ret = 0;

	mutex_lock(&heptdev->map_lock);
	mem = kzalloc(sizeof(*mem), GFP_KERNEL);
	if (!mem) {
		hpcerr("malloc rpmsg mem node failed\n");
		ret = -ENOMEM;
		goto put_lock;
	}

	ret = copy_from_user(&mem->info, argp, sizeof(mem->info));
	if (ret != 0) {
		hpcerr("copy from user for mem info failed: %d\n", ret);
		goto put_buf;
	}

	switch (mem->info.type) {
	case HPC_MEM_BUF_DMA:
		ret = hrpdev_dma_buf_map(hdev, mem);
		if (ret != 0) {
			hpcerr("%d buf map mem failed: %d\n", mem->info.type, ret);
			if (ret != -EINVAL)
				mem->info.heap_status = 1;
			if (copy_to_user(argp, &mem->info, sizeof(mem->info)))
				hpcerr("copy_to_user failed");
			goto put_buf;
		}
		break;
	case HPC_MEM_BUF_OCM:
		ret = hrpdev_ocm_buf_map(hdev, mem);
		break;
	default:
		hpcerr("unsupported type\n");
		ret = -EINVAL;
	}

	if (ret != 0) {
		hpcerr("%d buf map mem failed: %d\n", mem->info.type, ret);
		goto put_buf;
	}

	mem->info.heap_status = 0;
	ret = copy_to_user(argp, &mem->info, sizeof(mem->info));
	if (ret != 0) {
		hpcerr("copy_to_user failed: %d\n", ret);
		goto put_copy_to_user;
	}

	list_add_tail(&mem->node, &heptdev->mem);

	mutex_unlock(&heptdev->map_lock);

	return 0;

put_copy_to_user:
	(void)hrpdev_dma_buf_unmap(hdev, mem);

put_buf:
	kfree(mem);

put_lock:
	mutex_unlock(&heptdev->map_lock);

	return ret;
}

static int hrpdev_buf_unmap_all(struct hpc_device *hdev,
		struct hpc_ept_device *heptdev)
{
	struct hpc_rpmsg_mem_node *mem = NULL, *tmp = NULL;
	int ret = 0;

	hpcdbg("entry\n");
	mutex_lock(&heptdev->map_lock);
	list_for_each_entry_safe(mem, tmp, &heptdev->mem, node) {
		switch (mem->info.type) {
		case HPC_MEM_BUF_DMA:
			ret = hrpdev_dma_buf_unmap(hdev, mem);
			break;
		case HPC_MEM_BUF_OCM:
			ret = hrpdev_ocm_buf_unmap(hdev, mem);
			break;
		default:
			hpcerr("unsupported type\n");
			ret = -EINVAL;
		}

		if (ret != 0) {
			hpcerr("hrpdev buf %d unmap mem failed: %d\n", mem->info.type, ret);
			goto put_lock;
		}

		list_del(&mem->node);
		kfree(mem);
	}

put_lock:
	mutex_unlock(&heptdev->map_lock);
	hpcdbg("end\n");

	return ret;
}

static int hrpdev_buf_unmap(struct hpc_device *hdev,
		struct hpc_ept_device *heptdev, void __user *argp)
{
	struct hpc_rpmsg_mem_info info;
	struct hpc_rpmsg_mem_node *mem = NULL, *tmp = NULL;
	int ret;

	hpcdbg("entry\n");
	mutex_lock(&heptdev->map_lock);
	ret = copy_from_user(&info, argp, sizeof(info));
	if (ret != 0) {
		hpcerr("copy from user for mem info failed: %d\n", ret);
		goto put_lock;
	}

	list_for_each_entry_safe(mem, tmp, &heptdev->mem, node) {
		if ((mem->info.fd != info.fd) || (mem->info.size != info.size))
			continue;

		switch (mem->info.type) {
		case HPC_MEM_BUF_DMA:
			ret = hrpdev_dma_buf_unmap(hdev, mem);
			break;
		case HPC_MEM_BUF_OCM:
			ret = hrpdev_ocm_buf_unmap(hdev, mem);
			break;
		default:
			hpcerr("unsupported type\n");
			ret = -EINVAL;
		}

		if (ret != 0) {
			hpcerr("hrpdev buf %d unmap mem failed: %d\n", mem->info.type, ret);
			goto put_lock;
		}

		list_del(&mem->node);
		kfree(mem);
		break;
	}

put_lock:
	mutex_unlock(&heptdev->map_lock);
	hpcdbg("end\n");

	return ret;
}

static int heptdev_rpmsg_cb(struct rpmsg_device *rpdev, void *data, int len,
		void *priv, u32 addr)
{
	struct hpc_ept_device *heptdev;
	struct hpc_rpc_msg *kbuf;
	int ret = 0;

	heptdev = (struct hpc_ept_device *)priv;
	if (!heptdev->ept) {
		hpcerr("ept is not created\n");
		return -EPIPE;
	}

	kbuf = kzalloc(sizeof(struct hpc_rpc_msg), GFP_KERNEL);
	if (!kbuf) {
		hpcerr("alloc rx msg desc failed\n");
		ret = -ENOMEM;
		goto out;
	}

	kbuf->data = kzalloc(len, GFP_KERNEL);
	if (!kbuf->data) {
		hpcerr("alloc rx msg buffer failed\n");
		ret = -ENOMEM;
		goto put_data;
	}

	hpcdbg("get msg from dest 0x%x, src 0x%x", heptdev->ept->addr, addr);
	memcpy(kbuf->data, data, len);
	kbuf->len = len;
	mutex_lock(&heptdev->mutex);
	list_add_tail(&kbuf->node, &heptdev->rvq);
	mutex_unlock(&heptdev->mutex);
	wake_up_interruptible(&heptdev->readq);

	return 0;

put_data:
	kfree(kbuf);

out:
	return ret;
}

static int heptdev_init(struct hpc_rpmsg_device *hrpdev, struct file *filp)
{
	struct rpmsg_device *rpdev = hrpdev->rpdev;
	struct hpc_ept_device *heptdev;
	int ret;

	get_device(&rpdev->dev);

	heptdev = devm_kzalloc(&rpdev->dev, sizeof(*heptdev), GFP_KERNEL);
	if (!heptdev) {
		hpcerr("hpc ept device zalloc failed\n");
		ret = -ENOMEM;
		goto err_put_dev;
	}

	heptdev->filp = filp;
	heptdev->pid = current->tgid;
	atomic_set(&heptdev->fd_closed, 0);
	atomic_set(&heptdev->if_memmap, 0);
	strscpy(heptdev->chinfo.name, rpdev->id.name, RPMSG_NAME_SIZE);

	heptdev->chinfo.src = RPMSG_ADDR_ANY;
	heptdev->chinfo.dst = rpdev->dst;
	heptdev->ept = rpmsg_create_ept(rpdev, heptdev_rpmsg_cb, heptdev, heptdev->chinfo);
	if (!heptdev->ept) {
		hpcerr("create ept fail\n");
		ret = -ENOMEM;
		goto err_put_eptdev;
	}

	hpcdbg("ept addr 0x%x\n", heptdev->ept->addr);
	heptdev->ept->priv = heptdev;
	list_add_tail(&heptdev->node, &hrpdev->epts);
	INIT_LIST_HEAD(&heptdev->rvq);
	INIT_LIST_HEAD(&heptdev->mem);
	init_waitqueue_head(&heptdev->readq);
	heptdev->priv = hrpdev;
	filp->private_data = heptdev;
	mutex_init(&heptdev->mutex);
	mutex_init(&heptdev->map_lock);

	return 0;

err_put_eptdev:
	devm_kfree(&rpdev->dev, heptdev);
err_put_dev:
	put_device(&rpdev->dev);

	return ret;
}


static void heptdev_free_rvq_buf(struct hpc_ept_device *heptdev)
{
	struct hpc_rpc_msg *msg = NULL;

	mutex_lock(&heptdev->mutex);
	while (!list_empty(&heptdev->rvq)) {
		msg = list_first_entry(&heptdev->rvq, struct hpc_rpc_msg, node);
		if (msg) {
			kfree(msg->data);
			list_del(&msg->node);
			kfree(msg);
			msg = NULL;
		}
	}
	mutex_unlock(&heptdev->mutex);
}

static void heptdev_deinit(struct hpc_rpmsg_device *hrpdev, struct hpc_ept_device *heptdev)
{
	struct rpmsg_device *rpdev = hrpdev->rpdev;
	struct file *filp = NULL;
	int ret;

	if (!heptdev || !heptdev->ept)
		return;

	filp = heptdev->filp;
	hpcdbg("destroy ept addr 0x%x\n", heptdev->ept->addr);
	rpmsg_destroy_ept(heptdev->ept);
	heptdev_free_rvq_buf(heptdev);
	heptdev->ept = NULL;
	filp->private_data = NULL;

	wake_up_interruptible_all(&heptdev->readq);
	hpcdbg("wake up all readq in heptdev\n");
	heptdev->priv = NULL;
	list_del(&heptdev->node);

	ret = hrpdev_buf_unmap_all(hrpdev->hdev, heptdev);
	if (ret != 0)
		hpcerr("buf unmap all failed: %d\n", ret);

	mutex_destroy(&heptdev->map_lock);
	mutex_destroy(&heptdev->mutex);
	devm_kfree(&rpdev->dev, heptdev);
	heptdev = NULL;
	put_device(&rpdev->dev);
}

static int hrpdev_open(struct inode *inode, struct file *filp)
{
	struct hpc_rpmsg_device *hrpdev = miscdev_to_hrpdev(filp->private_data);
	struct hpc_ept_device *heptdev = NULL, *heptmp = NULL;
	int ret = 0;

	if (hpc_pid_info_find(g_pid_info, current->tgid))
		return -EIO;

	hpcdbg("entry\n");
	mutex_lock(&hrpdev->mutex);
	if (!hrpdev->rpdev) {
		hpcerr("rpdev not probe\n");
		ret = -ENODEV;
		goto out;
	}
	list_for_each_entry_safe(heptdev, heptmp, &hrpdev->epts, node) {
		if (filp->private_data == heptdev) {
			hpcerr("hpc rpmsg device already create\n");
			ret = -EBUSY;
			goto out;
		}
	}

	ret = heptdev_init(hrpdev, filp);
	if (ret != 0)
		hpcerr("heptdev_init failed: %d\n", ret);

out:
	mutex_unlock(&hrpdev->mutex);
	hpcdbg("end\n");
	return ret;
}

static int hrpdev_release(struct inode *inode, struct file *filp)
{
	struct hpc_ept_device *heptdev = NULL;
	struct hpc_rpmsg_device *hrpdev = g_hrpdev;
	struct hpc_ept_device *heptnode = NULL, *heptmp = NULL;
	int ret = 0, find = 0;
	pid_t tmp_pid = 0;
	long timeout = 0;

	hpcdbg("entry\n");
	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -EPERM;
	}
	mutex_lock(&hrpdev->mutex);
	heptdev = filp->private_data;
	list_for_each_entry_safe(heptnode, heptmp, &hrpdev->epts, node) {
		if (heptnode == heptdev) {
			find = 1;
			break;
		}
	}

	if (!find) {
		hpcerr("ept device is not created\n");
		ret = -ENOMEM;
		goto out;
	}

	tmp_pid = heptdev->pid;
	if ((atomic_read(&heptdev->fd_closed) != 1) &&
		(atomic_read(&heptdev->if_memmap) != 0)) {
		if ((atomic_read(&hrpdev->hdev->power) != 0) &&
			hpc_pid_info_add(hrpdev->pid_info, heptdev->pid) == 1) {
			send_npu_cancel_all_stream(heptdev, (uint32_t)heptdev->pid);
		}

		hpcdbg("begin wait cancel_all_wait, %d", hrpdev->hdev->cancel_all_flag);
		timeout = msecs_to_jiffies(WAIT_TIMEOUT);
		wait_event_timeout(hrpdev->hdev->cancel_all_wait,
							 hrpdev->hdev->cancel_all_flag,
							 timeout);

		hpcdbg(" end wait cancel_all_wait, %d", hrpdev->hdev->cancel_all_flag);
	}

	heptdev_deinit(hrpdev, heptdev);
out:
	mutex_unlock(&hrpdev->mutex);
	filp->private_data = NULL;
	if (hpc_rpmsg_is_pid_not_in_epts(hrpdev, tmp_pid)) {
		hpc_pid_info_del(hrpdev->pid_info, tmp_pid);
		if (hrpdev->hdev->cancel_all_flag == true)
			hrpdev->hdev->cancel_all_flag = false;
	}
	hpcdbg("end\n");

	return ret;
}

static ssize_t hrpdev_read(struct file *filp, char __user *buf,
		size_t len, loff_t *pos)
{
	struct hpc_ept_device *heptdev = NULL;
	struct hpc_rpc_msg *msg;
	ssize_t ret = 0;
	size_t mlen;

	if (is_npu_excp_true())
		return -EIO;

	hpcdbg("entry\n");
	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -EPERM;
	}
	heptdev = filp->private_data;
	if (!heptdev->priv) {
		hpcdbg("heptdev has been unregistered, return %d\n", -EPIPE);
		return ret;
	}

	if (hpc_pid_info_find(g_pid_info, heptdev->pid))
		return -EIO;

	if (list_empty(&heptdev->rvq)) {
		if (wait_event_interruptible(heptdev->readq,
				!heptdev->ept || !list_empty(&heptdev->rvq)))
			return -ERESTARTSYS;
		if (!heptdev->ept) {
			hpcdbg("heptdev->ept is null and return with %d\n", -EPIPE);
			return -EPIPE;
		}
		if (!list_empty(&heptdev->rvq))
			hpcdbg("heptdev->rvq is not empty and go on");
	}

	mutex_lock(&heptdev->mutex);
	msg = list_first_entry(&heptdev->rvq,
					struct hpc_rpc_msg, node);
	if (!msg || !msg->data) {
		hpcerr("get response msg failed\n");
		ret = -ENOMEM;
		mutex_unlock(&heptdev->mutex);
		goto out;
	}
	list_del(&msg->node);
	mutex_unlock(&heptdev->mutex);
	mlen = msg->len < len ? msg->len : len;
	ret = copy_to_user((void __user *)buf, (void *)msg->data, mlen);
	if (ret != 0) {
		hpcerr("copy_to_user failed: %ld\n", ret);
		ret = -ENOMEM;
		goto out;
	}

	kfree(msg->data);
	msg->data = NULL;
	kfree(msg);
	msg = NULL;
out:
	hpcdbg("end\n");
	return ret < 0 ? ret : mlen;
}

static ssize_t hrpdev_write(struct file *filp, const char __user *buf,
		size_t len, loff_t *pos)
{
	struct hpc_ept_device *heptdev = NULL;
	struct hpc_rpmsg_device *hrpdev = NULL;
	struct hpc_virtio_device *hvdev = NULL;
	int ret = 0;
	unsigned long length = 0;
	void *kbuf;

	if (is_npu_excp_true())
		return -EIO;

	hpcdbg("entry\n");
	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -EPERM;
	}
	heptdev = filp->private_data;
	if (!heptdev->ept) {
		hpcerr("ept is not created\n");
		return -EPIPE;
	}

	if (hpc_pid_info_find(g_pid_info, heptdev->pid))
		return -EIO;

	hrpdev = (struct hpc_rpmsg_device *)heptdev->priv;
	kbuf = kzalloc(len, GFP_KERNEL);
	if (!kbuf) {
		hpcerr("alloc tx buffer failed\n");
		return -ENOMEM;
	}

	length = copy_from_user(kbuf, (const void __user *)buf, len);
	if (length != 0) {
		hpcerr("copy msg from user failed: length %lu, len %lu\n", length, len);
		ret = -EFAULT;
		goto free_kbuf;
	}

	if (!heptdev->priv) {
		hpcdbg("heptdev has been unregistered, return %d\n", -EPIPE);
		ret = -EPIPE;
		goto free_kbuf;
	}

	hvdev = hrpdev->hdev->hvdev;

	if (!hvdev) {
		ret = -EPIPE;
		goto free_kbuf;
	}

	mutex_lock(&hvdev->lock);

	if (atomic_read(&hvdev->power) == 0) {
		hpcerr("hvdev->power is equal to 0\n");
		mutex_unlock(&hvdev->lock);
		ret = -EFAULT;
		goto free_kbuf;
	}

	ret = rpmsg_sendto(heptdev->ept, kbuf, len, heptdev->chinfo.dst);

	mutex_unlock(&hvdev->lock);

free_kbuf:
	kfree(kbuf);
	hpcdbg("end\n");
	return ret < 0 ? ret : len;
}

static long hrpdev_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	struct hpc_ept_device *heptdev = NULL;
	struct hpc_rpmsg_device *hrpdev = NULL;
	void __user *argp = (void __user *)arg;
	int ret = 0;

	if (is_npu_excp_true())
		return -EIO;

	hpcdbg("entry\n");
	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -EPERM;
	}
	heptdev = filp->private_data;
	if (!heptdev->priv) {
		hpcdbg("heptdev has been unregistered, return %d\n", -EPIPE);
		return -EPIPE;
	}

	if (hpc_pid_info_find(g_pid_info, heptdev->pid))
		return -EIO;

	hrpdev = heptdev->priv;

	switch (cmd) {
	case HDEV_DMA_BUFF_MAP:
		ret = hrpdev_buf_map(hrpdev->hdev, heptdev, argp);
		if (ret != 0)
			hpcerr("buf map failed: %d\n", ret);

		atomic_inc(&heptdev->if_memmap);
		break;
	case HDEV_DMA_BUFF_UNMAP:
		ret = hrpdev_buf_unmap(hrpdev->hdev, heptdev, argp);
		if (ret != 0)
			hpcerr("buf unmap failed: %d\n", ret);

		atomic_dec(&heptdev->if_memmap);
		break;
	case HDEV_RPMSG_FD_CLOSE:
		atomic_set(&heptdev->fd_closed, 1);
		hpcdbg("HDEV_RPMSG_FD_CLOSE called, set g_rpmsg_fd_closed to 1\n");
		break;

	default:
		hpcerr("Unsupported ioctl\n");
		ret = -EINVAL;
	}
	hpcdbg("end\n");

	return ret;
}

static const struct file_operations hrpdev_fops = {
	.owner = THIS_MODULE,
	.open = hrpdev_open,
	.read = hrpdev_read,
	.release = hrpdev_release,
	.write = hrpdev_write,
	.unlocked_ioctl = hrpdev_ioctl,
	.compat_ioctl = compat_ptr_ioctl,
};

static int hrpdev_miscdev_init(struct hpc_rpmsg_device *hrpdev)
{
	struct miscdevice *miscdev = &hrpdev->miscdev;
	int ret;

	miscdev->parent = &hrpdev->rpdev->dev;
	miscdev->minor = MISC_DYNAMIC_MINOR;
	miscdev->name = "hpc-rpmsg";
	miscdev->fops = &hrpdev_fops;
	ret = misc_register(miscdev);
	if (ret != 0)
		hpcerr("register hpc rpmsg miscdev failed: %d\n", ret);

	return ret;
}

void hrpdev_miscdev_destroy(struct hpc_rpmsg_device *hrpdev)
{
	misc_deregister(&hrpdev->miscdev);
}

static int hpc_rpmsg_probe(struct rpmsg_device *rpdev)
{
	struct hpc_device *hdev = NULL;
	struct hpc_rpmsg_device *hrpdev = NULL;
	int ret;

	hpcdbg("entry\n");
	hrpdev = g_hrpdev;
	hdev = rpdev_to_hdev(rpdev);
	hrpdev->rpdev = rpdev;
	hrpdev->hdev = hdev;
	rpdev->ept->priv = hrpdev;
	hpcdbg("this ept is not for send or recevie msg, ");
	hpcdbg("just use to create name service channel\n");
	hdev_rpmsg_complete(hdev);
	ret = hpc_dma_alloc(hdev);
	if (ret) {
		hpcdbg("DMA allocation failed\n");
		return ret;
	}
	hpcdbg("end\n");

	return 0;
}

void hpc_rpmsg_remove(struct rpmsg_device *rpdev)
{
	struct hpc_rpmsg_device *hrpdev = rpdev_to_hrpdev(rpdev);
	struct hpc_ept_device *heptdev = NULL, *heptmp = NULL;

	hpcdbg("entry\n");

	mutex_lock(&hrpdev->mutex);
	hpc_pid_info_destroy(hrpdev->pid_info);
	hpc_dma_free(hrpdev->hdev);
	list_for_each_entry_safe(heptdev, heptmp, &hrpdev->epts, node) {
		heptdev_deinit(hrpdev, heptdev);
	}
	hdev_boot_completion_reinit(hrpdev->hdev);
	hrpdev->rpdev = NULL;

	mutex_unlock(&hrpdev->mutex);

	hpcdbg("end\n");
}

static int hpc_rpmsg_cb(struct rpmsg_device *rpdev, void *data, int len,
		void *priv, u32 addr)
{
	return 0;
}

static const struct rpmsg_device_id hpc_rpmsg_id_table[] = {
	{ .name = "hpc,rpmsg" },
	{ /* Sentinel */ },
};

static struct rpmsg_driver hpc_rpmsg_driver = {
	.probe = hpc_rpmsg_probe,
	.remove = hpc_rpmsg_remove,
	.callback = hpc_rpmsg_cb,
	.id_table = hpc_rpmsg_id_table,
	.drv = {
		.name = "hpc-rpmsg",
	},
};

static int hpc_rpmsg_pdev_remove(struct platform_device *pdev)
{
	struct hpc_rpmsg_device *hrpdev = platform_get_drvdata(pdev);

	hpcinfo("entry\n");
	unregister_rpmsg_driver(&hpc_rpmsg_driver);

	mutex_destroy(&hrpdev->mutex);
	hpc_pid_info_destroy(hrpdev->pid_info);
	hrpdev_miscdev_destroy(hrpdev);
	hpcinfo("end\n");
	return 0;
}

static int hpc_rpmsg_pdev_probe(struct platform_device *pdev)
{
	struct hpc_rpmsg_device *hrpdev = NULL;
	int ret;

	hpcinfo("entry\n");
	hrpdev = devm_kzalloc(&pdev->dev, sizeof(*hrpdev), GFP_KERNEL);
	if (!hrpdev) {
		hpcerr("hpc rpmsg device zalloc failed\n");
		return -ENOMEM;
	}
	g_hrpdev = hrpdev;
	ret = hrpdev_miscdev_init(hrpdev);
	if (ret != 0) {
		hpcerr("register hrpdev failed\n");
		return ret;
	}

	platform_set_drvdata(pdev, hrpdev);
	mutex_init(&hrpdev->mutex);
	INIT_LIST_HEAD(&hrpdev->epts);
	hrpdev->pid_info = devm_kzalloc(&pdev->dev, sizeof(struct hpc_pid_info), GFP_KERNEL);
	if (!hrpdev->pid_info) {
		hpcerr("hpchrpdev->pid_info zalloc failed\n");
		return -ENOMEM;
	}
	hpc_pid_info_init(hrpdev->pid_info);
	g_pid_info = hrpdev->pid_info;

	ret = register_rpmsg_driver(&hpc_rpmsg_driver);
	if (ret != 0) {
		hpcerr("register rpmsg driver failed\n");
		return ret;
	}

	hpcinfo("end\n");

	return 0;
}

static const struct of_device_id hpc_rpmsg_pdev_id_table[] = {
	{ .compatible = "xring,hpc-rpmsg-pdev" },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, hpc_rpmsg_pdev_id_table);

static struct platform_driver hpc_rpmsg_pdev_driver = {
	.probe = hpc_rpmsg_pdev_probe,
	.remove = hpc_rpmsg_pdev_remove,
	.driver = {
		.name = "hpc-rpmsg-pdev",
		.of_match_table = of_match_ptr(hpc_rpmsg_pdev_id_table),
	},
};
module_platform_driver(hpc_rpmsg_pdev_driver);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC Rpmsg Driver");
MODULE_LICENSE("GPL v2");
