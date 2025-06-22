// SPDX-License-Identifier: GPL-2.0-only
/*
 * Remote processor messaging transport (OMAP platform-specific bits)
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 * Copyright (C) 2011 Google, Inc.
 *
 * Ohad Ben-Cohen <ohad@wizery.com>
 * Brian Swetland <swetland@google.com>
 */

#include <linux/module.h>
#include <linux/dma-direct.h>
#include <linux/dma-map-ops.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/virtio_config.h>
#include <linux/virtio_ids.h>
#include <linux/virtio_ring.h>
#include <linux/err.h>
#include <linux/kref.h>
#include <linux/string.h>
#include "hpc_internal.h"

static DEFINE_IDA(hvdev_id);

static inline struct hpc_virtio_device *vdev_to_hvdev(struct virtio_device *vdev)
{
	return container_of(vdev->dev.parent, struct hpc_virtio_device, dev);
}

static int hpc_copy_dma_range_map(struct device *to, struct device *from)
{
	const struct bus_dma_region *map = from->dma_range_map, *new_map, *r;
	int num_ranges = 0;

	if (!map)
		return 0;

	for (r = map; r->size; r++)
		num_ranges++;

	new_map = kmemdup(map, array_size(num_ranges + 1, sizeof(*map)),
			  GFP_KERNEL);
	if (!new_map)
		return -ENOMEM;
	to->dma_range_map = new_map;

	return 0;
}

/* kick the remote processor, and let it know which virtqueue to poke at */
static bool hpc_virtio_notify(struct virtqueue *vq)
{
	struct hpc_vring *hvring = (struct hpc_vring *)vq->priv;
	struct hpc_virtio_device *hvdev = (struct hpc_virtio_device *)hvring->priv;
	int notifyid = hvring->notifyid;

	hpcdbg("kicking vq index: %d\n", notifyid);

	hvdev->ops->kick(hvdev, notifyid);
	return true;
}

/**
 * hpc_vq_interrupt() - tell remoteproc that a virtqueue is interrupted
 * @hvdev: handle to the hpc virtio device
 * @notifyid: index of the signalled virtqueue (unique per this @hvdev)
 *
 * This function should be called by the platform-specific hvdev driver,
 * when the remote processor signals that a specific virtqueue has pending
 * messages available.
 *
 * Return: IRQ_NONE if no message was found in the @notifyid virtqueue,
 * and otherwise returns IRQ_HANDLED.
 */
irqreturn_t hpc_vq_interrupt(struct hpc_virtio_device *hvdev, int notifyid)
{
	struct hpc_vring *hvring;
	int i;

	hpcdbg("vq index %d is interrupted\n", notifyid);

	for (i = 0; i < HPC_VRING_NUM_MAX; i++) {
		hvring = &hvdev->vring[i];
		if (hvring->vq != NULL && hvring->notifyid == notifyid)
			return vring_interrupt(0, hvring->vq);
	}

	return IRQ_NONE;
}
EXPORT_SYMBOL_GPL(hpc_vq_interrupt);

static void hpc_virtio_del_vqs(struct virtio_device *vdev)
{
	struct virtqueue *vq = NULL, *n = NULL;
	struct hpc_vring *hvring;

	list_for_each_entry_safe(vq, n, &vdev->vqs, list) {
		hvring = vq->priv;
		hvring->vq = NULL;
		vring_del_virtqueue(vq);
	}
}

static struct virtqueue *hpc_find_vq(struct virtio_device *vdev,
				    unsigned int id,
				    void (*callback)(struct virtqueue *vq),
				    const char *name, bool ctx)
{
	struct hpc_virtio_device *hvdev = vdev_to_hvdev(vdev);
	struct hpc_vring_share_para *hvrsp;
	struct hpc_vring *hvring;
	struct virtqueue *vq;
	size_t size;

	if (id >= ARRAY_SIZE(hvdev->vring))
		return ERR_PTR(-EINVAL);

	if (!name)
		return NULL;

	hvring = &hvdev->vring[id];
	hvrsp = &hvdev->hpc_vsp->hvrsp[id];

	/* zero vring */
	size = vring_size(hvrsp->num, hvrsp->align);
	memset((void *)hvrsp->va, 0, size);

	hpcdbg("vring%d: va 0x%llx qsz %ld notifyid %d\n",
		id, hvrsp->va, size, hvring->notifyid);

	vq = vring_new_virtqueue(id, hvrsp->num, hvrsp->align, vdev, false,
		ctx, (void *)hvrsp->va, hpc_virtio_notify, callback, name);
	if (!vq) {
		hpcerr("vring_new_virtqueue %s failed\n", name);
		return ERR_PTR(-ENOMEM);
	}

	hvring->vq = vq;
	vq->priv = hvring;

	return vq;
}

static int hpc_virtio_find_vqs(struct virtio_device *vdev, unsigned int nvqs,
				 struct virtqueue *vqs[],
				 vq_callback_t *callbacks[],
				 const char * const names[],
				 const bool *ctx,
				 struct irq_affinity *desc)
{
	int i, ret, queue_idx = 0;

	for (i = 0; i < nvqs; ++i) {
		if (!names[i]) {
			vqs[i] = NULL;
			continue;
		}

		vqs[i] = hpc_find_vq(vdev, queue_idx++, callbacks[i], names[i],
				    ctx ? ctx[i] : false);
		if (IS_ERR(vqs[i])) {
			ret = PTR_ERR(vqs[i]);
			goto error;
		}
	}

	return 0;

error:
	hpc_virtio_del_vqs(vdev);
	return ret;
}

static u8 hpc_virtio_get_status(struct virtio_device *vdev)
{
	struct hpc_virtio_device *hvdev = vdev_to_hvdev(vdev);
	struct hpc_vdev_share_para *hvsp = &hvdev->hpc_vsp->hvsp;

	return hvsp->status;
}

static void hpc_virtio_set_status(struct virtio_device *vdev, u8 status)
{
	struct hpc_virtio_device *hvdev = vdev_to_hvdev(vdev);
	struct hpc_vdev_share_para *hvsp = &hvdev->hpc_vsp->hvsp;

	hvsp->status = status;
	hpcdbg("status: %d\n", status);
}

static void hpc_virtio_reset(struct virtio_device *vdev)
{
	struct hpc_virtio_device *hvdev = vdev_to_hvdev(vdev);
	struct hpc_vdev_share_para *hvsp = &hvdev->hpc_vsp->hvsp;

	hvsp->status = 0;
	hpcdbg("reset !\n");
}

/* provide the vdev features as retrieved from the share memory */
static u64 hpc_virtio_get_features(struct virtio_device *vdev)
{
	struct hpc_virtio_device *hvdev = vdev_to_hvdev(vdev);
	struct hpc_vdev_share_para *hvsp = &hvdev->hpc_vsp->hvsp;

	return hvsp->dfeatures;
}

static int hpc_virtio_finalize_features(struct virtio_device *vdev)
{
	struct hpc_virtio_device *hvdev = vdev_to_hvdev(vdev);
	struct hpc_vdev_share_para *hvsp = &hvdev->hpc_vsp->hvsp;

	vring_transport_features(vdev);
	__virtio_clear_bit(vdev, VIRTIO_F_RING_PACKED);
	BUG_ON((u32)vdev->features != vdev->features);
	hvsp->gfeatures = vdev->features;

	return 0;
}

static void hpc_virtio_get(struct virtio_device *vdev, unsigned int offset,
			     void *buf, unsigned int len)
{
	struct hpc_virtio_device *hvdev = vdev_to_hvdev(vdev);
	struct hpc_vdev_share_para *hvsp = &hvdev->hpc_vsp->hvsp;
	void *cfg = (void *)&hvdev->hpc_vsp->hvqsp;

	if (offset + len > hvsp->config_len || offset + len < len) {
		hpcerr("access out of bounds\n");
		return;
	}

	memcpy(buf, cfg + offset, len);
}

static void hpc_virtio_set(struct virtio_device *vdev, unsigned int offset,
			     const void *buf, unsigned int len)
{
	struct hpc_virtio_device *hvdev = vdev_to_hvdev(vdev);
	struct hpc_vdev_share_para *hvsp = &hvdev->hpc_vsp->hvsp;
	void *cfg = (void *)&hvdev->hpc_vsp->hvqsp;

	if (offset + len > hvsp->config_len || offset + len < len) {
		hpcerr("access out of bounds\n");
		return;
	}

	memcpy(cfg + offset, buf, len);
}

static bool hpc_get_shm_region(struct virtio_device *vdev,
		struct virtio_shm_region *region, u8 id)
{
	struct hpc_virtio_device *hvdev = vdev_to_hvdev(vdev);
	struct hpc_vdev_share_para *vsp = &hvdev->hpc_vsp->hvsp;
	struct hpc_vq_share_para *vqsp = &hvdev->hpc_vsp->hvqsp;
	struct hpc_vring_share_para *vrsp;
	int i;

	region->addr = vqsp->va;
	region->len = 0;
	for (i = 0; i < vsp->num_vrings; i++) {
		vrsp = &hvdev->hpc_vsp->hvrsp[i];
		region->len += vrsp->num * vqsp->size;
	}

	return true;
}

static const struct virtio_config_ops hpc_virtio_config_ops = {
	.get_features	= hpc_virtio_get_features,
	.finalize_features = hpc_virtio_finalize_features,
	.find_vqs	= hpc_virtio_find_vqs,
	.del_vqs	= hpc_virtio_del_vqs,
	.reset		= hpc_virtio_reset,
	.set_status	= hpc_virtio_set_status,
	.get_status	= hpc_virtio_get_status,
	.get		= hpc_virtio_get,
	.set		= hpc_virtio_set,
	.get_shm_region = hpc_get_shm_region,
};

static int hvdev_init_hvring(struct hpc_virtio_device *hvdev,
		struct hpc_virtio_share_para *hpc_vsp, int i)
{
	struct hpc_vring *hvring = &hvdev->vring[i];
	struct hpc_vring_share_para *hvrsp = &hpc_vsp->hvrsp[i];
	int ret;

	if (!hvrsp->num || !hvrsp->align) {
		hpcerr("invalid qsz (%d) or alignment (%d)\n",
			hvrsp->num, hvrsp->align);
		return -EINVAL;
	}

	ret = ida_simple_get(&hvdev->notifyids, 0, 0, GFP_KERNEL);
	if (ret < 0) {
		hpcerr("ida_simple_get for hvring%d failed: %d\n",
				i, ret);
		return ret;
	}

	hvring->notifyid = ret;
	hvrsp->notifyid = (u32)ret;
	hvring->priv = (void *)hvdev;

	return 0;
}

static void hvdev_destroy_hvring(struct hpc_virtio_device *hvdev, int i)
{
	struct hpc_vring *hvring = &hvdev->vring[i];

	ida_simple_remove(&hvdev->notifyids, hvring->notifyid);
	hvring->notifyid = -1;
	hvring->priv = NULL;
}

static void hvdev_virito_dev_release(struct device *dev)
{
	struct virtio_device *vdev = dev_to_virtio(dev);
	struct hpc_virtio_device *hvdev = vdev_to_hvdev(vdev);

	hvdev->vdev = NULL;
	kfree(vdev);
	put_device(&hvdev->dev);
}

static int hvdev_init_vdev(struct hpc_virtio_device *hvdev, u32 id)
{
	struct virtio_device *vdev = NULL;
	struct device *dev = &hvdev->dev;
	int ret;

	vdev = kzalloc(sizeof(*vdev), GFP_KERNEL);
	if (!vdev) {
		hpcerr("kzalloc vdev failed\n");
		return -ENOMEM;
	}

	vdev->id.device = id;
	vdev->config = &hpc_virtio_config_ops;
	vdev->dev.parent = dev;
	vdev->dev.release = hvdev_virito_dev_release;
	get_device(dev);

	ret = register_virtio_device(vdev);
	if (ret != 0) {
		put_device(&vdev->dev);
		hpcerr("failed to register vdev: %d\n", ret);
		goto out;
	}

	hvdev->vdev = vdev;
	hpcdbg("registered %s (type %d)\n", dev_name(&vdev->dev), id);

out:
	return ret;
}

static void hvdev_destroy_vdev(struct hpc_virtio_device *hvdev)
{
	unregister_virtio_device(hvdev->vdev);
}

static int hvdev_virtio_boot(struct hpc_virtio_device *hvdev)
{
	struct hpc_vdev_share_para *hvsp = &hvdev->hpc_vsp->hvsp;
	int i, ret;

	if (hvsp->reserve[0] || hvsp->reserve[1]) {
		hpcerr("vdev share memory has non zero reserved bytes\n");
		return -EINVAL;
	}

	if (hvsp->num_vrings > ARRAY_SIZE(hvdev->vring)) {
		hpcerr("too many vrings: %d\n", hvsp->num_vrings);
		return -EINVAL;
	}

	hvsp->notifyid = hvdev->id;
	for (i = 0; i < hvsp->num_vrings; i++) {
		ret = hvdev_init_hvring(hvdev, hvdev->hpc_vsp, i);
		if (ret != 0) {
			hpcerr("init hvring%d failed: %d\n", i, ret);
			goto destroy_hvring;
		}
	}

	ret = hvdev_init_vdev(hvdev, hvsp->id);
	if (ret != 0) {
		hpcerr("init virtio device failed: %d\n", ret);
		goto destroy_hvring;
	}

	return 0;

destroy_hvring:
	for (i--; i >= 0; i--)
		hvdev_destroy_hvring(hvdev, i);

	return ret;
}

static void hpc_vdev_type_release(struct device *dev)
{
	struct hpc_virtio_device *hvdev = container_of(dev, struct hpc_virtio_device, dev);

	hpcdbg("releasing %s\n", hvdev->name);
	mutex_destroy(&hvdev->lock);
	ida_destroy(&hvdev->notifyids);
	if (hvdev->id >= 0)
		ida_simple_remove(&hvdev_id, hvdev->id);
	kfree_const(hvdev->name);
	kfree(hvdev);
}

static const struct device_type hvdev_type = {
	.name		= "hpc_vdev",
	.release	= hpc_vdev_type_release,
};

struct hpc_virtio_device *hpc_vdev_alloc(struct device *dev, const char *name,
	struct hpc_virtio_ops *ops, void *priv)
{
	struct hpc_virtio_device *hvdev = NULL;
	int ret;

	hpcdbg("alloc %s\n", name);
	if (!dev || !ops)
		return NULL;

	hvdev = kzalloc(sizeof(struct hpc_virtio_device), GFP_KERNEL);
	if (!hvdev)
		return NULL;

	hvdev->hpc_vsp = NULL;
	hvdev->vdev = NULL;
	hvdev->ops = ops;
	hvdev->priv = priv;
	device_initialize(&hvdev->dev);
	hvdev->dev.parent = dev;
	hvdev->dev.type = &hvdev_type;
	dev_set_drvdata(&hvdev->dev, hvdev);

	ida_init(&hvdev->notifyids);

	hvdev->name = kstrdup_const(name, GFP_KERNEL);
	if (!hvdev->name)
		goto put_device;

	hvdev->id = ida_simple_get(&hvdev_id, 0, USHRT_MAX, GFP_KERNEL);
	if (hvdev->id < 0) {
		hpcerr("ida_simple_get failed: %d\n", hvdev->id);
		goto put_device;
	}

	dev_set_name(&hvdev->dev, "hpc_vdev%d", hvdev->id);

	ret = hpc_copy_dma_range_map(&hvdev->dev, dev);
	if (ret)
		goto put_device;

	set_dma_ops(&hvdev->dev, get_dma_ops(dev));

	ret = dma_coerce_mask_and_coherent(&hvdev->dev, dma_get_mask(dev));
	if (ret) {
		hpcwarn("Failed to set DMA mask %llx. Trying to continue... %x\n",
			 dma_get_mask(dev), ret);
	}

	ret = device_add(&hvdev->dev);
	if (ret < 0) {
		hpcerr("hvdev device add failed\n");
		goto put_device;
	}

	atomic_set(&hvdev->power, 0);
	mutex_init(&hvdev->lock);
	hpcdbg("end alloc %s\n", name);

	return hvdev;

put_device:
	put_device(&hvdev->dev);
	return NULL;
}
EXPORT_SYMBOL_GPL(hpc_vdev_alloc);

void hpc_vdev_free(struct hpc_virtio_device *hvdev)
{
	device_del(&hvdev->dev);
	put_device(&hvdev->dev);
}
EXPORT_SYMBOL_GPL(hpc_vdev_free);

int hpc_vdev_boot(struct hpc_virtio_device *hvdev)
{
	struct hpc_vq_share_para *vqsp;
	int ret;

	hpcdbg("entry\n");
	if (!hvdev || !hvdev->hpc_vsp) {
		hpcerr("invalid hvdev or hpc_vsp\n");
		return -EINVAL;
	}

	vqsp = &hvdev->hpc_vsp->hvqsp;
	if (!vqsp->va) {
		hpcerr("vqueue va not init\n");
		return -EINVAL;
	}

	mutex_lock(&hvdev->lock);

	if (atomic_inc_return(&hvdev->power) > 1) {
		ret = 0;
		goto unlock_mutex;
	}

	ret = hvdev_virtio_boot(hvdev);
	if (ret)
		atomic_dec(&hvdev->power);

unlock_mutex:
	mutex_unlock(&hvdev->lock);
	hpcdbg("end\n");
	return ret;
}
EXPORT_SYMBOL_GPL(hpc_vdev_boot);

void hpc_vdev_shutdown(struct hpc_virtio_device *hvdev)
{
	struct hpc_vdev_share_para *hvsp = &hvdev->hpc_vsp->hvsp;
	int i;

	hpcdbg("entry\n");
	mutex_lock(&hvdev->lock);

	if (atomic_read(&hvdev->power) == 0)
		goto out;

	if (!atomic_dec_and_test(&hvdev->power))
		goto out;

	hvdev_destroy_vdev(hvdev);

	for (i = hvsp->num_vrings - 1; i >= 0; i--)
		hvdev_destroy_hvring(hvdev, i);

out:
	mutex_unlock(&hvdev->lock);
	hpcdbg("end\n");
}
EXPORT_SYMBOL_GPL(hpc_vdev_shutdown);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC Virtio Driver");
MODULE_LICENSE("GPL v2");
