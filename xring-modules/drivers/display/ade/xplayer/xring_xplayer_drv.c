// SPDX-License-Identifier: GPL-2.0-only
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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/dma-buf.h>
#include <display/xring_xplayer_uapi.h>
#include "xring_xplayer_drv.h"

#define WFD_SOURCE_FIFO_SIZE 16

static struct platform_device *g_xplayer_device;

static void xplayer_sink_work(struct work_struct *arg)
{
	struct xplayer_iommu_format_info *iommu_info;
	struct dma_buf_attachment *attach;
	struct xplayer_sink_work *work;
	struct xplayer_data *priv;
	struct sg_table *sgt;
	struct dma_buf *buf;
	int tlb_index;

	priv = platform_get_drvdata(xplayer_get_device());
	work = container_of(arg, struct xplayer_sink_work, work);
	buf = work->data.buf;
	tlb_index = work->data.tlb_index;
	iommu_info = &(work->data.iommu_info);

	attach = dma_buf_attach(buf, &priv->pdev->dev);
	if (IS_ERR(attach)) {
		XRING_XPLAYER_ERR("fail to attach dma-buf");
		return;
	}

	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		XRING_XPLAYER_ERR("fail to map dma-buf to get sgt, errno %ld", PTR_ERR(sgt));
		dma_buf_detach(buf, attach);
		return;
	}

	dpu_wfd_sink_set_sg_table(buf, sgt, tlb_index, iommu_info);

	dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(buf, attach);
}
static int xplayer_slice_init(uint32_t flags)
{
	struct xplayer_data *priv;
	int ret;

	priv = platform_get_drvdata(xplayer_get_device());
	if (flags & (XPLAYER_FLAG_EVENT | XPLAYER_FLAG_SOURCE)) {
		ret = kfifo_alloc(&priv->info_work.data,
				sizeof(struct xplayer_source_data) * WFD_SOURCE_FIFO_SIZE,
				GFP_KERNEL);
		if (ret) {
			XRING_XPLAYER_ERR("kfifo alloc fail ret is %d\n", ret);
			return -ENOMEM;
		}
	}

	if (flags & XPLAYER_FLAG_SINK) {
		priv->wq = create_workqueue("xplayer_wq");
		if (!priv->wq) {
			XRING_XPLAYER_ERR("fail to create workqueue");
			return -1;
		}
		dpu_wfd_sink_init();
	}

	if (flags & XPLAYER_FLAG_EVENT)
		dpu_wfd_event_register(xplayer_notify_event);

	atomic_set(&priv->released, 0);
	priv->flags = flags;

	return 0;
}

static void xplayer_slice_deinit(void)
{
	struct xplayer_data *priv;

	dpu_wfd_event_unregister();

	priv = platform_get_drvdata(xplayer_get_device());
	priv->flags = 0;
	if (priv->wq) {
		flush_workqueue(priv->wq);
		destroy_workqueue(priv->wq);
		priv->wq = NULL;
	}


	atomic_set(&priv->released, 1);
	wake_up_interruptible(&priv->info_work.wq);
	kfifo_free(&priv->info_work.data);
	dpu_wfd_sink_deinit();
}

static int xring_xplayer_open(struct inode *inode, struct file *filp)
{
	struct xplayer_data *priv;

	XRING_XPLAYER_DBG("enter");
	xplayer_check_and_return(!filp, -1, "filp is null");
	xplayer_check_and_return(!inode, -1, "inode is null");

	priv = platform_get_drvdata(xplayer_get_device());
	if (atomic_read(&priv->unique_opened))
		return -EBUSY;

	atomic_set(&priv->unique_opened, 1);
	filp->private_data = priv;

	XRING_XPLAYER_DBG("exit");

	return 0;
}

static int xring_xplayer_release(struct inode *inode, struct file *filp)
{
	struct xplayer_data *priv;

	XRING_XPLAYER_DBG("enter");
	xplayer_check_and_return(!filp, -1, "filp is null");
	xplayer_check_and_return(!inode, -1, "inode is null");

	xplayer_slice_deinit();
	priv = platform_get_drvdata(xplayer_get_device());
	atomic_set(&priv->unique_opened, 0);

	XRING_XPLAYER_DBG("exit");

	return 0;
}

static long xring_xplayer_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
	int ret = 0;

	XRING_XPLAYER_DBG("enter");
	xplayer_check_and_return(!filp, -1, "filp is null");

	switch (cmd) {
	case XPLAYER_SLICE_INIT:
		return xplayer_slice_init(arg);
	case XPLAYER_SLICE_DEINIT:
		xplayer_slice_deinit();
		break;
	case XPLAYER_SLICE_PRESENT_INDEX:
		dpu_wfd_sink_set_present_index(arg);
		break;
	default:
		XRING_XPLAYER_WARN("unknown cmd, check it! cmd is %u", cmd);
		break;
	}
	XRING_XPLAYER_DBG("exit");

	return ret;
}

static ssize_t xring_xplayer_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	struct xplayer_source_data tmp;
	struct xplayer_data *priv;
	size_t fifo_ret;
	int ret = 0;
	int fd;

	if (len != sizeof(struct xplayer_info)) {
		XRING_XPLAYER_ERR("reading length not match");
		return -EINVAL;
	}

	priv = platform_get_drvdata(xplayer_get_device());
	if (atomic_read(&priv->released)) {
		XRING_XPLAYER_WARN("released");
		return -ENOMEM;
	}

	if (kfifo_is_empty(&priv->info_work.data)) {
		if (filp->f_flags & O_NONBLOCK)
			return 0;

		if (wait_event_interruptible(priv->info_work.wq,
				!kfifo_is_empty(&priv->info_work.data) ||
				atomic_read(&priv->released)) ||
				atomic_read(&priv->released))
			return 0;
	}

	fifo_ret = kfifo_out_locked(&priv->info_work.data, &tmp, sizeof(tmp),
			&priv->info_work.lock);
	if (fifo_ret != sizeof(tmp))
		return -ENOMEM;

	if (tmp.info.type == XPLAYER_INFO_ENC_SLICE) {
		/* get fd for buf */
		fd = dma_buf_fd(tmp.buf, O_CLOEXEC);
		if (fd < 0) {
			XRING_XPLAYER_ERR("couldn't get fd from dma_buf");
			return -ENOEXEC;
		}

		get_dma_buf(tmp.buf);
		tmp.info.slice_info.fd = fd;

		XRING_XPLAYER_DBG("dma_buf: %p, fd: %d, offset: %d, length: %d, slice_index: %d\n",
				tmp.buf, tmp.info.slice_info.fd, tmp.info.slice_info.offset,
				tmp.info.slice_info.length, tmp.info.slice_info.slice_index);
	}

	ret = copy_to_user((struct xplayer_info __user *)buf, &tmp.info,
			sizeof(struct xplayer_info));
	if (ret) {
		XRING_XPLAYER_ERR("copy to user failed ret:%d", ret);
		return -ENOMEM;
	}

	return sizeof(tmp.info);

}

static const struct file_operations xplayer_fops = {
	.owner = THIS_MODULE,
	.open = xring_xplayer_open,
	.read = xring_xplayer_read,
	.release = xring_xplayer_release,
	.compat_ioctl = xring_xplayer_ioctl,
	.unlocked_ioctl = xring_xplayer_ioctl,
};

static int xring_xplayer_chrdev_register(struct xplayer_data *priv)
{
	int ret;

	XRING_XPLAYER_DBG("enter");
	xplayer_check_and_return(!priv, -1, "priv is null");

	priv->chr_major = register_chrdev(0, XPLAYER_DEV_NAME, &xplayer_fops);
	if (priv->chr_major < 0) {
		XRING_XPLAYER_ERR("register chrdev fail");
		return -ENXIO;
	}

	priv->chr_class = class_create(XPLAYER_DEV_NAME);
	if (IS_ERR_OR_NULL(priv->chr_class)) {
		XRING_XPLAYER_ERR("class create fail");
		ret = PTR_ERR(priv->chr_class);
		unregister_chrdev(priv->chr_major, XPLAYER_DEV_NAME);
		return ret;
	}

	priv->chr_dev = device_create(priv->chr_class,
			0, MKDEV(priv->chr_major, 0), priv, XPLAYER_DEV_NAME);
	if (IS_ERR_OR_NULL(priv->chr_dev)) {
		XRING_XPLAYER_ERR("dev create fail");
		ret = PTR_ERR(priv->chr_dev);
		class_destroy(priv->chr_class);
		unregister_chrdev(priv->chr_major, XPLAYER_DEV_NAME);
		return ret;
	}

	INIT_WORK(&priv->sink_work.work, xplayer_sink_work);
	init_waitqueue_head(&priv->info_work.wq);
	spin_lock_init(&priv->info_work.lock);

	XRING_XPLAYER_DBG("exit");

	return 0;
}

static int xring_xplayer_chrdev_unregister(struct xplayer_data *priv)
{
	XRING_XPLAYER_DBG("enter");
	xplayer_check_and_return(!priv, -1, "priv is null");

	if (priv->chr_major > 0) {
		device_destroy(priv->chr_class, MKDEV(priv->chr_major, 0));
		class_destroy(priv->chr_class);
		unregister_chrdev(priv->chr_major, XPLAYER_DEV_NAME);
	}
	XRING_XPLAYER_DBG("exit");

	return 0;
}

static int xring_xplayer_probe(struct platform_device *pdev)
{
	struct xplayer_data *priv = NULL;
	int ret = 0;

	XRING_XPLAYER_DBG("enter");

	xplayer_check_and_return(!pdev, -ENODEV, "pdev is null");

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	xplayer_check_and_return(!priv, -EINVAL, "priv is null");

	/* create character device for xplayer */
	ret = xring_xplayer_chrdev_register(priv);
	xplayer_check_and_return(ret, -EINVAL, "chrdev register fail");

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret)
		XRING_XPLAYER_ERR("dma_set_mask_and_coherent fail\n");

	priv->pdev = pdev;
	platform_set_drvdata(pdev, priv);

	XRING_XPLAYER_DBG("exit");

	return 0;
}

static int xring_xplayer_remove(struct platform_device *pdev)
{
	struct xplayer_data *priv = NULL;

	XRING_XPLAYER_DBG("enter");

	xplayer_check_and_return(!pdev, -ENODEV, "pdev is null");

	priv = (struct xplayer_data *)platform_get_drvdata(pdev);
	xplayer_check_and_return(!priv, -EINVAL, "priv is null");

	xring_xplayer_chrdev_unregister(priv);

	XRING_XPLAYER_DBG("exit");

	return 0;
}

static struct platform_driver xring_xplayer_driver = {
	.probe = xring_xplayer_probe,
	.remove = xring_xplayer_remove,
	.driver = {
		.name = "xplayer",
		.owner = THIS_MODULE,
	}
};

int xplayer_drv_register(void)
{
	int ret;

	XRING_XPLAYER_DBG("enter");

	g_xplayer_device = platform_device_alloc("xplayer", -1);
	xplayer_check_and_return(!g_xplayer_device, -1, "Failed to allocate device for xplayer");

	ret = platform_device_add(g_xplayer_device);
	if (ret != 0) {
		XRING_XPLAYER_ERR("Failed to add xplayer device\n");
		platform_device_put(g_xplayer_device);
		return -1;
	}

	ret = platform_driver_register(&xring_xplayer_driver);
	xplayer_check_and_return(ret, -EINVAL, "xplayer register fail");
	XRING_XPLAYER_DBG("exit");

	return ret;
}

void xplayer_drv_unregister(void)
{
	XRING_XPLAYER_DBG("enter");
	platform_device_unregister(g_xplayer_device);
	platform_driver_unregister(&xring_xplayer_driver);
	XRING_XPLAYER_DBG("exit");
}

struct platform_device *xplayer_get_device(void)
{
	return g_xplayer_device;
}
