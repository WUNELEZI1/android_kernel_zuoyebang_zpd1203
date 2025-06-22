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
#include<linux/dmaengine.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <uapi/linux/dma-heap.h>
#include <linux/proc_fs.h>
#include <linux/iommu.h>
#include "fk-audio-cma.h"
#include "fk-audio-log.h"

static LIST_HEAD(audio_memory_dma_buf_list);
static DEFINE_MUTEX(audio_memory_dma_buf_lock);

MODULE_IMPORT_NS(DMA_BUF);

dma_addr_t audio_memory_phys;
char *audio_memory_vaddr;

dma_addr_t get_audio_memory_phys(void)
{
	AUD_LOG_DBG(AUD_COMM, "audio_memory_phys =0x%llx", audio_memory_phys);
	return audio_memory_phys;
}
EXPORT_SYMBOL(get_audio_memory_phys);

char *get_audio_memory_vaddr(void)
{
	AUD_LOG_DBG(AUD_COMM, "audio_memory_vaddr =0x%llx", (uint64_t)audio_memory_vaddr);
	return audio_memory_vaddr;
}
EXPORT_SYMBOL(get_audio_memory_vaddr);

static void audio_memory_dump_sg_info(struct sg_table *sgt)
{
	struct scatterlist *sgl = NULL;
	int i = 0;

	for_each_sg(sgt->sgl, sgl, sgt->nents, i) {
		AUD_LOG_INFO(AUD_COMM, "sg phys: 0x%llx, dma_address: 0x%llx, dma_len: 0x%x",
			sg_phys(sgl), sg_dma_address(sgl), sg_dma_len(sgl));
	}
}

void fk_audio_buf_free(struct audio_memory_dma_buf *priv)
{
	dma_heap_buffer_free(priv->dma_buf);
	kfree(priv);
}
EXPORT_SYMBOL(fk_audio_buf_free);

char *fk_audio_buffer_alloc(struct device *dev, size_t size, dma_addr_t *dma_handle)
{
	void *va = dma_alloc_coherent(dev, size, dma_handle, GFP_KERNEL);

	if (!va) {
		AUD_LOG_ERR(AUD_COMM, "dma_alloc_coherent failed");
		return NULL;
	}
	return va;
}
EXPORT_SYMBOL(fk_audio_buffer_alloc);
void fk_audio_buffer_free(struct device *dev, size_t size, void *vaddr, dma_addr_t dma_handle)
{
	dma_free_coherent(dev, size, vaddr, dma_handle);
}
EXPORT_SYMBOL(fk_audio_buffer_free);

static struct audio_memory_dma_buf *audio_memory_map(struct device *dev, int fd)
{
	struct audio_memory_dma_buf *priv = NULL;
	int ret = 0;

	AUD_LOG_DBG_LIM(AUD_COMM, "audio_memory");
	priv = kzalloc(sizeof(struct audio_memory_dma_buf), GFP_KERNEL);
	if (!priv) {
		AUD_LOG_ERR(AUD_COMM, "audio_memory: alloc mem for priv fail");
		return NULL;
	}

	priv->dma_buf = dma_buf_get(fd);
	if (IS_ERR(priv->dma_buf)) {
		AUD_LOG_ERR(AUD_COMM, "audio_memory: dma_buf_get fail");
		goto fail_free;
	}

	priv->attachment = dma_buf_attach(priv->dma_buf, dev);
	if (IS_ERR(priv->attachment)) {
		AUD_LOG_ERR(AUD_COMM, "audio_memory: dma_buf_attach fail");
		goto fail_put;
	}

	priv->sgt = dma_buf_map_attachment(priv->attachment, DMA_BIDIRECTIONAL);
	if (IS_ERR(priv->sgt)) {
		AUD_LOG_ERR(AUD_COMM, "audio_memory: dma_buf_map_attachment fail");
		goto fail_detach;
	}

	audio_memory_dump_sg_info(priv->sgt);

	/* map to kernel and access the mem */
	ret = dma_buf_vmap(priv->dma_buf, &priv->map);
	if (ret != 0) {
		AUD_LOG_ERR(AUD_COMM, "audio_memory: dma_buf_vmap fail");
		goto fail_unmap;
	}

	priv->fd = fd;
	priv->buf = (char *)priv->map.vaddr;
	priv->len = sg_dma_len(priv->sgt->sgl);
	priv->src_dma_addr = sg_dma_address(priv->sgt->sgl);
	audio_memory_phys = priv->src_dma_addr;
	audio_memory_vaddr = priv->buf;
	memset(priv->buf, 0, priv->len);
	AUD_LOG_DBG_LIM(AUD_COMM, "priv->len=%d priv->buf=0x%llx",
		priv->len, (uint64_t)priv->buf);
	return priv;

fail_unmap:
	dma_buf_unmap_attachment(priv->attachment, priv->sgt, DMA_BIDIRECTIONAL);
fail_detach:
	dma_buf_detach(priv->dma_buf, priv->attachment);
fail_put:
	dma_buf_put(priv->dma_buf);
fail_free:
	kfree(priv);

	return NULL;
}

static long audio_memory_ioctl(struct file *file, unsigned int ioctl,
			unsigned long arg)
{
	long ret = 0;
	int dmabuf_fd = 0;
	struct miscdevice *misc_dev = file->private_data;
	struct audio_memory_dma_buf *priv = NULL;

	AUD_LOG_INFO_LIM(AUD_COMM, "ioctl = %d", ioctl);
	if (copy_from_user(&dmabuf_fd, (int __user *)arg,
			sizeof(dmabuf_fd))) {
		AUD_LOG_ERR(AUD_COMM, "audio_memory: copy_from_user fail");
		return -1;
	}
	AUD_LOG_INFO(AUD_COMM, "dmabuf_fd = %d", dmabuf_fd);
	priv = audio_memory_map(misc_dev->parent, dmabuf_fd);

	return ret;
}

static const struct file_operations audio_memory_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = audio_memory_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = audio_memory_ioctl,
#endif
};

static struct miscdevice audio_memory_misc = {
	.minor          = MISC_DYNAMIC_MINOR,
	.name           = "audio_memory",
	.fops           = &audio_memory_fops,
};

static int fk_audio_cma_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = misc_register(&audio_memory_misc);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_COMM, "misc_register fail, ret=%d", ret);
		return -1;
	}
	audio_memory_misc.parent = &pdev->dev;

	/*
	 * don't check return value here, sometimes user forget to insmod
	 * carveout_heap.ko or don't need to test carveout heap, here will not
	 * cause error and will continue other test
	 */

	mutex_init(&audio_memory_dma_buf_lock);
	return ret;
}

static int fk_audio_cma_remove(struct platform_device *pdev)
{
	misc_deregister(&audio_memory_misc);
	return 0;
}


static const struct of_device_id fk_audio_cma_dt_match[] = {
	{.compatible = "xring,fk-audio-cma"},
	{}
};

static struct platform_driver fk_audio_cma_driver = {
	.probe  = fk_audio_cma_probe,
	.remove = fk_audio_cma_remove,
	.driver = {
		.name = "fk-audio-cma",
		.owner = THIS_MODULE,
		.of_match_table = fk_audio_cma_dt_match,
		.suppress_bind_attrs = true,
	},
};

int __init fk_audio_cma_init(void)
{
	return platform_driver_register(&fk_audio_cma_driver);
}

void fk_audio_cma_exit(void)
{
	platform_driver_unregister(&fk_audio_cma_driver);
}

/* Module information */
MODULE_DESCRIPTION("XRING AUDIO COMMON CMA");
MODULE_LICENSE("Dual BSD/GPL");
