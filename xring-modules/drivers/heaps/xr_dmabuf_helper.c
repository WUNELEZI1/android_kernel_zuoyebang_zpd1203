// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Carveout heap exporter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on dma heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/kthread.h>
#include <linux/iommu.h>
#include <linux/dmaengine.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <uapi/linux/dma-heap.h>
#include <linux/proc_fs.h>
#include <linux/iommu.h>
#include <linux/list.h>
#include <linux/fdtable.h>
#include <linux/sched.h>
#include <linux/version.h>

#include "asm/page-def.h"
#include "soc/xring/xr_dmabuf_helper.h"
#include "xr_dmabuf_helper_internal.h"
#include "soc/xring/xring_mem_adapter.h"
#include "xr_dmabuf_pagepool.h"
#include "xring_heaps.h"

struct xr_dmabuf_helper_list {
	struct list_head head;
	struct mutex lock;
};

static unsigned long long dmabuf_alloc_warn_threshold = 1 << 29;
static struct xr_dmabuf_helper_list xdh_list;
static atomic64_t dmabuf_kernel_account[XR_DMABUF_KERNEL_TYPE];
static const char *xr_dmabuf_kernel_type_strs[XR_DMABUF_KERNEL_TYPE] = {
	"isp_rproc",
	"isp_ca",
	"gpu",
	"npu",
	"mitee_tui",
};

int xr_dmabuf_helper_register(struct xr_dmabuf_helper_ops *ops)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (!ops)
		return -EINVAL;

	INIT_LIST_HEAD(&ops->list_node);
	mutex_lock(&xdh_list.lock);
	list_for_each_safe(pos, n, &xdh_list.head) {
		if (pos == &ops->list_node) {
			mutex_unlock(&xdh_list.lock);
			xrheap_info("this xr_dmabuf_helper_ops already registered\n");
			return 0;
		}
	}
	list_add(&ops->list_node, &xdh_list.head);
	mutex_unlock(&xdh_list.lock);
	return 0;
}
EXPORT_SYMBOL(xr_dmabuf_helper_register);

int xr_dmabuf_kernel_account(struct dma_buf *dmabuf, enum xr_dmabuf_kernel_type type)
{
	if (!dmabuf || IS_ERR(dmabuf) || type >= XR_DMABUF_KERNEL_TYPE) {
		xrheap_info("invalid dmabuf or type[%d]\n", type);
		return -EINVAL;
	}
	atomic64_add(dmabuf->size, &dmabuf_kernel_account[type]);

	return 0;
}
EXPORT_SYMBOL_GPL(xr_dmabuf_kernel_account);

int xr_dmabuf_kernel_unaccount(struct dma_buf *dmabuf, enum xr_dmabuf_kernel_type type)
{
	if (!dmabuf || IS_ERR(dmabuf) || type >= XR_DMABUF_KERNEL_TYPE) {
		xrheap_info("invalid dmabuf or type[%d]\n", type);
		return -EINVAL;
	}
	atomic64_sub(dmabuf->size, &dmabuf_kernel_account[type]);

	return 0;
}
EXPORT_SYMBOL_GPL(xr_dmabuf_kernel_unaccount);

size_t xr_iterate_fd(struct files_struct *files, unsigned int n,
			size_t (*f)(const void *, struct file *, unsigned int), const void *p)
{
	struct fdtable *fdt = NULL;
	size_t res = 0;
	struct file *file = NULL;

	if (!files)
		return 0;

	spin_lock(&files->file_lock);
	for (fdt = files_fdtable(files); n < fdt->max_fds; n++) {
		file = rcu_dereference_check_fdtable(files, fdt->fd[n]);
		if (!file)
			continue;
		res += f(p, file, n);
	}
	spin_unlock(&files->file_lock);
	return res;
}

static size_t each_dmabuf_not_show(const void *data, struct file *f, unsigned int fd)
{
	struct dma_buf *dmabuf = NULL;

	if (!is_dma_buf_file(f))
		return 0;

	dmabuf = (struct dma_buf *)f->private_data;
	if (!dmabuf)
		return 0;

	return dmabuf->size;
}

static size_t each_dmabuf(const void *data, struct file *f, unsigned int fd)
{
	const struct task_struct *task = data;
	struct dma_buf *dmabuf = NULL;

	if (!is_dma_buf_file(f))
		return 0;

	dmabuf = (struct dma_buf *)f->private_data;
	if (!dmabuf)
		return 0;

	xrheap_info("%16s %8d %8u %16s %32s %8lukB %8lu\n",
		task->comm, task->pid, fd, dmabuf->exp_name, dmabuf->name ? dmabuf->name : "",
		dmabuf->size >> 10, f->f_inode->i_ino);
	return dmabuf->size;
}

static void read_dmabuf(void)
{
	struct task_struct *task = NULL;
	size_t tasksize;
	int i;

	xrheap_info("Detail dmabuf process info is below:\n");
	xrheap_info("        taskname      pid       fd         heapname %32s size(kB)    inode\n", "bufname");

	rcu_read_lock();
	for_each_process(task) {
		if (task->flags & PF_KTHREAD)
			continue;
		task_lock(task);
		tasksize = xr_iterate_fd(task->files, 0, each_dmabuf_not_show, (void *)task);
		if (tasksize)
			xrheap_info("%16s %8d %8u %16s %8lukB\n",
				task->comm, task->pid, 0, "TOTAL", tasksize >> 10);
		if (tasksize > dmabuf_alloc_warn_threshold)
			xr_iterate_fd(task->files, 0, each_dmabuf, (void *)task);
		task_unlock(task);
	}
	rcu_read_unlock();

	xrheap_info("dmabuf kernel account size is below:\n");
	for (i = 0; i < XR_DMABUF_KERNEL_TYPE; i++)
		xrheap_info("[type=%d] %s TOTAL %llukB\n", i, xr_dmabuf_kernel_type_strs[i],
			atomic64_read(&dmabuf_kernel_account[i]) >> 10);
}


int xr_dmabuf_get_meminfo(u64 *alloc_size, u64 *buddy_consumed, u64 *cma_consumed, u64 *pool_cached, bool dump)
{
	struct xr_dmabuf_helper_ops *ops = NULL;
	struct xr_dmabuf_helper_ops *next = NULL;

	if (!alloc_size || !buddy_consumed || !cma_consumed || !pool_cached)
		return -EINVAL;

	*alloc_size = 0;
	*buddy_consumed = 0;
	*cma_consumed = 0;
	*pool_cached = 0;

	mutex_lock(&xdh_list.lock);
	list_for_each_entry_safe(ops, next, &xdh_list.head, list_node) {
		u64 alloc = 0;
		u64 buddy = 0;
		u64 cma = 0;
		u64 pool = 0;

		if (ops->get_alloc_size)
			alloc = ops->get_alloc_size(ops->priv);

		if (ops->get_buddy_consumed)
			buddy = ops->get_buddy_consumed(ops->priv);

		if (ops->get_cma_consumed)
			cma = ops->get_cma_consumed(ops->priv);

		if (ops->get_pool_cached)
			pool = ops->get_pool_cached(ops->priv);

		*alloc_size += alloc;
		*buddy_consumed += buddy;
		*cma_consumed += cma;
		*pool_cached += pool;
		if (dump)
			xrheap_info("%s alloc: %lldkB, from buddy: %lldkB, from cma: %lldkB, cached in pool: %lldkB\n",
				ops->name, alloc >> 10, buddy >> 10, cma >> 10, pool >> 10);
	}
	mutex_unlock(&xdh_list.lock);

	if (dump) {
		xrheap_info("dmabuf total alloc: %llukB, from buddy: %llukB, from cma: %llukB, cached in pool: %llukB\n",
			(*alloc_size) >> 10, (*buddy_consumed) >> 10,
			(*cma_consumed) >> 10, (*pool_cached) >> 10);
	}
	return 0;
}

void show_dmabuf_detail(bool verbose)
{
	u64 alloc_size, buddy_consumed, cma_consumed, pool_cached;

	xrheap_info("Xring-meminfo dmabuf:\n");
	xr_dmabuf_get_meminfo(&alloc_size, &buddy_consumed, &cma_consumed, &pool_cached, true);
	read_dmabuf();
}
EXPORT_SYMBOL(show_dmabuf_detail);

int xr_dma_buf_begin_cpu_access_partial(void *arg)
{
	struct xr_dmabuf_helper_cpu_access xdhca;
	struct dma_buf *dmabuf = NULL;
	int ret;

	if (copy_from_user(&xdhca, (void __user *)arg,
		sizeof(struct xr_dmabuf_helper_cpu_access))) {
		xrheap_err("%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	dmabuf = dma_buf_get(xdhca.dmabuf_fd);
	if (IS_ERR(dmabuf)) {
		xrheap_err("%s fd is not dmabuf\n", __func__);
		return -EFAULT;
	}

	ret = dma_buf_begin_cpu_access_partial(dmabuf, xdhca.direction, xdhca.offset, xdhca.len);
	dma_buf_put(dmabuf);
	return ret;
}

int xr_dma_buf_end_cpu_access_partial(void *arg)
{
	struct xr_dmabuf_helper_cpu_access xdhca;
	struct dma_buf *dmabuf = NULL;
	int ret;

	if (copy_from_user(&xdhca, (void __user *)arg,
		sizeof(struct xr_dmabuf_helper_cpu_access))) {
		xrheap_err("%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	dmabuf = dma_buf_get(xdhca.dmabuf_fd);
	if (IS_ERR(dmabuf)) {
		xrheap_err("%s fd is not dmabuf\n", __func__);
		return -EFAULT;
	}

	ret = dma_buf_end_cpu_access_partial(dmabuf, xdhca.direction, xdhca.offset, xdhca.len);
	dma_buf_put(dmabuf);
	return ret;
}

long long xr_dmabuf_get_headersize(struct dma_buf *dmabuf)
{
	struct xr_heap_buffer_header *header;
	long long headersize;

	if (!dmabuf || !dmabuf->priv) {
		xrheap_err("xr_dmabuf_helper: input parameter error\n");
		return -EINVAL;
	}

	header = (struct xr_heap_buffer_header *)(dmabuf->priv);
	headersize = header->header_size;

	return headersize;
}
EXPORT_SYMBOL(xr_dmabuf_get_headersize);

static int dmabuf_set_headersize(void *data)
{
	struct xr_dmabuf_helper_args *helper_data = data;
	struct xr_heap_buffer_header *header;
	int fd = helper_data->dmabuf_fd;
	struct dma_buf *dmabuf = NULL;
	long long header_size;

	fd = helper_data->dmabuf_fd;
	header_size = helper_data->header.header_size;
	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf))
		return -EFAULT;

	if (header_size > dmabuf->size) {
		xrheap_err("fd %d header_size %lld is larger than dmabuf size %ld\n",
			fd, header_size, dmabuf->size);
		dma_buf_put(dmabuf);
		return -EINVAL;
	}

	header = (struct xr_heap_buffer_header *)(dmabuf->priv);
	header->header_size = header_size;

	dma_buf_put(dmabuf);

	return 0;
}

u32 xr_dmabuf_helper_get_sfd(struct dma_buf *dmabuf)
{
	struct xr_dmabuf_helper_ops *next = NULL;
	struct xr_dmabuf_helper_ops *ops = NULL;
	const char *name;
	u32 sfd = 0;

	name = dmabuf->exp_name;
	mutex_lock(&xdh_list.lock);
	list_for_each_entry_safe(ops, next, &xdh_list.head, list_node) {
		if (!strcmp(name, ops->name) && ops->get_sfd)
			sfd = ops->get_sfd(dmabuf);
	}
	mutex_unlock(&xdh_list.lock);
	return sfd;
}
EXPORT_SYMBOL(xr_dmabuf_helper_get_sfd);

static int dmabuf_helper_get_sfd(void *data)
{
	struct xr_dmabuf_helper_args *helper_data = data;
	int fd = helper_data->dmabuf_fd;
	struct dma_buf *dmabuf = NULL;

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf))
		return -EFAULT;

	helper_data->sfd = xr_dmabuf_helper_get_sfd(dmabuf);
	dma_buf_put(dmabuf);

	return 0;
}

static int dmabuf_change_mem_prot(void *data)
{
	struct xr_dmabuf_helper_args *helper_data = data;
	struct xr_dmabuf_helper_ops *next = NULL;
	struct xr_dmabuf_helper_ops *ops = NULL;
	int fd = helper_data->dmabuf_fd;
	struct dma_buf *dmabuf = NULL;
	const char *name;
	int ret = -EINVAL;

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf))
		return -EFAULT;

	name = dmabuf->exp_name;
	mutex_lock(&xdh_list.lock);
	list_for_each_entry_safe(ops, next, &xdh_list.head, list_node) {
		if (!strcmp(name, ops->name) && ops->change_mem_prot) {
			ret = ops->change_mem_prot(dmabuf);
			break;
		}
	}
	mutex_unlock(&xdh_list.lock);
	dma_buf_put(dmabuf);
	if (ret < 0) {
		xrheap_err("failed\n");
		return -EINVAL;
	}

	return 0;
}

static int dmabuf_prepare_alloc(void *data)
{
	struct xr_dmabuf_helper_args *helper_data = data;
	struct xr_dmabuf_helper_ops *next = NULL;
	struct xr_dmabuf_helper_ops *ops = NULL;
	u32 scene = helper_data->scene;
	u32 action = helper_data->action;
	u32 size = helper_data->size;
	int ret = 0;

	xrheap_info("Xring-dmabuf prepare alloc\n");

	mutex_lock(&xdh_list.lock);
	list_for_each_entry_safe(ops, next, &xdh_list.head, list_node) {
		if (scene == ops->scene && ops->pre_alloc)
			ret = ops->pre_alloc(ops->priv, (size_t)size, action);
	}
	mutex_unlock(&xdh_list.lock);

	return ret;
}

static int dmabuf_dio_alloc(void *data)
{
	struct xr_dmabuf_helper_args *helper_data = data;
	struct xr_dmabuf_helper_ops *next = NULL;
	struct xr_dmabuf_helper_ops *ops = NULL;
	u32 scene = helper_data->scene;
	size_t len = helper_data->size;
	int file_fd = helper_data->file_fd;
	loff_t offset = helper_data->offset;
	loff_t ppos = helper_data->ppos;
	int dmabuf_fd = helper_data->dmabuf_fd;
	int ret = 0;

	xrheap_info("Xring-dmabuf dio alloc\n");

	len = PAGE_ALIGN(len);
	if (!len)
		return -EFAULT;

	list_for_each_entry_safe(ops, next, &xdh_list.head, list_node) {
		if (scene == ops->scene && ops->dio_alloc)
			ret = ops->dio_alloc(file_fd, dmabuf_fd, offset, ppos, len);
	}

	return ret;
}

static long xr_dmabuf_helper_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct xr_dmabuf_helper_args xr_dmabuf_helper_data;
	int ret = 0;

	if (cmd == XR_HELPER_IOCTL_HEADERSIZE || cmd == XR_HELPER_IOCTL_GETSFD ||
		cmd == XR_HELPER_IOCTL_MEMPROT || cmd == XR_HELPER_IOCTL_PREALLOC ||
		cmd == XR_HELPER_IOCTL_DMABUF_DIO_ALLOC)
		if (copy_from_user(&xr_dmabuf_helper_data, (void __user *)arg,
			sizeof(struct xr_dmabuf_helper_args))) {
			xrheap_err("copy_from_user failed\n");
			return -EFAULT;
		}

	switch (cmd) {
	case XR_HELPER_IOCTL_HEADERSIZE:
		ret = dmabuf_set_headersize(&xr_dmabuf_helper_data);
		break;
	case XR_HELPER_IOCTL_GETSFD:
		ret = dmabuf_helper_get_sfd(&xr_dmabuf_helper_data);
		break;
	case XR_HELPER_IOCTL_MEMPROT:
		ret = dmabuf_change_mem_prot(&xr_dmabuf_helper_data);
		break;
	case XR_HELPER_IOCTL_PREALLOC:
		ret = dmabuf_prepare_alloc(&xr_dmabuf_helper_data);
		break;
	case XR_HELPER_IOCTL_BEGIN_CPU_ACCESS_PARTIAL:
		ret = xr_dma_buf_begin_cpu_access_partial((void *)arg);
		break;
	case XR_HELPER_IOCTL_END_CPU_ACCESS_PARTIAL:
		ret = xr_dma_buf_end_cpu_access_partial((void *)arg);
		break;
	case XR_HELPER_IOCTL_DMABUF_DIO_ALLOC:
		ret = dmabuf_dio_alloc(&xr_dmabuf_helper_data);
		break;
	default:
		xrheap_info("cmd: %u not supported\n", cmd);
		ret = -ENOTTY;
		break;
	}

	if (cmd == XR_HELPER_IOCTL_GETSFD) {
		if (copy_to_user((void __user *)arg, &xr_dmabuf_helper_data,
				sizeof(struct xr_dmabuf_helper_args)) != 0) {
			xrheap_err("copy_to_user failed\n");
			ret = -EFAULT;
		}
	}

	return ret;
}

static const struct file_operations xr_dmabuf_helper_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = xr_dmabuf_helper_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = xr_dmabuf_helper_ioctl,
#endif
};

static struct miscdevice xr_dmabuf_helper = {
	.minor          = MISC_DYNAMIC_MINOR,
	.name           = "xr_dmabuf_helper",
	.fops           = &xr_dmabuf_helper_fops,
};

static int xr_dmabuf_helper_probe(struct platform_device *pdev)
{
	int ret;

	ret = misc_register(&xr_dmabuf_helper);
	if (ret < 0) {
		xrheap_err("xr_dmabuf_helper: misc_register fail, ret=%d\n", ret);
		return -1;
	}

	xr_dmabuf_helper.parent = &pdev->dev;
	mutex_init(&xdh_list.lock);
	INIT_LIST_HEAD(&xdh_list.head);

	return 0;
}

static int xr_dmabuf_helper_remove(struct platform_device *pdev)
{
	misc_deregister(&xr_dmabuf_helper);
	return 0;
}

static const struct of_device_id xr_dmabuf_helper_match_table[] = {
	{ .compatible = "xring,dmabuf-helper" },
	{}
};

MODULE_DEVICE_TABLE(of, xr_dmabuf_helper_match_table);

static struct platform_driver xr_dmabuf_helper_platform_driver = {
	.driver = {
		.name = "xr_dmabuf_helper",
		.of_match_table = xr_dmabuf_helper_match_table,
	},
	.probe = xr_dmabuf_helper_probe,
	.remove = xr_dmabuf_helper_remove,
};

int xr_dmabuf_helper_init(void)
{
	platform_driver_register(&xr_dmabuf_helper_platform_driver);
	return 0;
}
EXPORT_SYMBOL(xr_dmabuf_helper_init);

void xr_dmabuf_helper_exit(void)
{
	platform_driver_unregister(&xr_dmabuf_helper_platform_driver);
}
EXPORT_SYMBOL(xr_dmabuf_helper_exit);

module_param(dmabuf_alloc_warn_threshold, ullong, 0644);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring Driver");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(MINIDUMP);
