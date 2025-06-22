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

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "virtio", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "virtio", __func__, __LINE__

#include <linux/export.h>
#include <linux/of_reserved_mem.h>
#include <linux/remoteproc.h>
#include <linux/virtio.h>
#include <linux/virtio_config.h>
#include <linux/virtio_ids.h>
#include <linux/virtio_ring.h>
#include <linux/err.h>
#include <linux/kref.h>
#include <linux/slab.h>

#include "xrisp_rproc.h"
#include "xrisp_log.h"

#define MAX_RPMSG_NUM_BUFS	(512)
#define MAX_RPMSG_BUF_SIZE	(512)

/* kick the remote processor, and let it know which virtqueue to poke at */
static bool xrisp_rproc_virtio_notify(struct virtqueue *vq)
{
	struct xrisp_rproc_vring *rvring = vq->priv;
	struct cam_rproc *xrisp_rproc = rvring->rvdev->xrisp_rproc;
	int notifyid = rvring->notifyid;

	// XRISP_DEV_DEBUG(xrisp_rproc->dev, "kicking vq index: %d\n", notifyid);

	xrisp_rproc->rproc->ops->kick(xrisp_rproc->rproc, notifyid);
	return true;
}

/**
 * xrisp_rproc_vq_interrupt() - tell remoteproc that a virtqueue is interrupted
 * @rproc: handle to the remote processor
 * @notifyid: index of the signalled virtqueue (unique per this @rproc)
 *
 * This function should be called by the platform-specific rproc driver,
 * when the remote processor signals that a specific virtqueue has pending
 * messages available.
 *
 * Return: IRQ_NONE if no message was found in the @notifyid virtqueue,
 * and otherwise returns IRQ_HANDLED.
 */
irqreturn_t xrisp_rproc_vq_interrupt(struct cam_rproc *xrisp_rproc, int notifyid)
{
	struct rproc_vring *rvring;

	// XRISP_DEV_DEBUG(xrisp_rproc->dev, "vq index %d is interrupted\n", notifyid);

	rvring = idr_find(&xrisp_rproc->notifyids, notifyid);
	if (!rvring || !rvring->vq)
		return IRQ_NONE;

	return vring_interrupt(0, rvring->vq);
}

static struct virtqueue *rp_find_vq(struct virtio_device *vdev,
				    unsigned int id,
				    void (*callback)(struct virtqueue *vq),
				    const char *name, bool ctx)
{
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);
	struct cam_rproc *xrisp_rproc = vdev_to_xrisp_rproc(vdev);
	struct device *dev = xrisp_rproc->dev;
	struct rproc_mem_entry *mem;
	struct xrisp_rproc_vring *rvring;
	struct fw_rsc_vdev *rsc;
	struct virtqueue *vq;
	void *addr;
	int len, size;

	/* we're temporarily limited to two virtqueues per rvdev */
	if (id >= ARRAY_SIZE(rvdev->vring))
		return ERR_PTR(-EINVAL);

	if (!name)
		return NULL;

	/* Search allocated memory region by name */
	mem = xrisp_rproc_find_carveout_by_name(xrisp_rproc->rproc, "vdev%dvring%d",
						rvdev->index, id);
	if (!mem || !mem->va)
		return ERR_PTR(-ENOMEM);

	rvring = &rvdev->vring[id];
	addr = mem->va;
	len = rvring->len;

	/* zero vring */
	size = vring_size(len, rvring->align);
	memset(addr, 0, size);

	XRISP_DEV_DEBUG(dev, "vring%d: va %pK qsz %d notifyid %d\n",
		id, addr, len, rvring->notifyid);

	/*
	 * Create the new vq, and tell virtio we're not interested in
	 * the 'weak' smp barriers, since we're talking with a real device.
	 */
	vq = vring_new_virtqueue(id, len, rvring->align, vdev, false, ctx,
				 addr, xrisp_rproc_virtio_notify, callback, name);
	if (!vq) {
		XRISP_DEV_ERROR(dev, "vring_new_virtqueue %s failed\n", name);
		xrisp_rproc_free_vring(rvring);
		return ERR_PTR(-ENOMEM);
	}

	rvring->vq = vq;
	vq->priv = rvring;

	/* Update vring in resource table */
	rsc = (void *)xrisp_rproc->rproc->table_ptr + rvdev->rsc_offset;
	pr_rsc_table(xrisp_rproc);
	rsc->vring[id].da = mem->da;

	return vq;
}

static void __rproc_virtio_del_vqs(struct virtio_device *vdev)
{
	struct virtqueue *vq = NULL, *n = NULL;
	struct rproc_vring *rvring;

	list_for_each_entry_safe(vq, n, &vdev->vqs, list) {
		rvring = vq->priv;
		rvring->vq = NULL;
		vring_del_virtqueue(vq);
	}
}

static void rproc_virtio_del_vqs(struct virtio_device *vdev)
{
	__rproc_virtio_del_vqs(vdev);
}

static int rproc_virtio_find_vqs(struct virtio_device *vdev, unsigned int nvqs,
				 struct virtqueue *vqs[],
				 vq_callback_t *callbacks[],
				 const char *const names[],
				 const bool *ctx,
				 struct irq_affinity *desc)
{
	int i, ret, queue_idx = 0;

	for (i = 0; i < nvqs; ++i) {
		if (!names[i]) {
			vqs[i] = NULL;
			continue;
		}

		vqs[i] = rp_find_vq(vdev, queue_idx++, callbacks[i], names[i],
				    ctx ? ctx[i] : false);
		if (IS_ERR(vqs[i])) {
			ret = PTR_ERR(vqs[i]);
			goto error;
		}
	}

	return 0;

error:
	__rproc_virtio_del_vqs(vdev);
	return ret;
}

static u8 rproc_virtio_get_status(struct virtio_device *vdev)
{
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);
	struct fw_rsc_vdev *rsc;

	rsc = (void *)rvdev->xrisp_rproc->rproc->table_ptr + rvdev->rsc_offset;

	XRISP_DEV_DEBUG(&vdev->dev, "status: %d\n", rsc->status);

	return rsc->status;
}

static void rproc_virtio_set_status(struct virtio_device *vdev, u8 status)
{
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);
	struct fw_rsc_vdev *rsc;

	rsc = (void *)rvdev->xrisp_rproc->rproc->table_ptr + rvdev->rsc_offset;

	rsc->status = status;
	XRISP_DEV_DEBUG(&vdev->dev, "status: %d\n", status);
}

static void rproc_virtio_reset(struct virtio_device *vdev)
{
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);
	struct fw_rsc_vdev *rsc;

	rsc = (void *)rvdev->xrisp_rproc->rproc->table_ptr + rvdev->rsc_offset;

	rsc->status = 0;
	XRISP_DEV_DEBUG(&vdev->dev, "reset !\n");
}

/* provide the vdev features as retrieved from the firmware */
static u64 rproc_virtio_get_features(struct virtio_device *vdev)
{
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);
	struct fw_rsc_vdev *rsc;

	rsc = (void *)rvdev->xrisp_rproc->rproc->table_ptr + rvdev->rsc_offset;

	return rsc->dfeatures;
}

static void rproc_transport_features(struct virtio_device *vdev)
{
	/*
	 * Packed ring isn't enabled on remoteproc for now,
	 * because remoteproc uses vring_new_virtqueue() which
	 * creates virtio rings on preallocated memory.
	 */
	__virtio_clear_bit(vdev, VIRTIO_F_RING_PACKED);
}

static int rproc_virtio_finalize_features(struct virtio_device *vdev)
{
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);
	struct fw_rsc_vdev *rsc;

	rsc = (void *)rvdev->xrisp_rproc->rproc->table_ptr + rvdev->rsc_offset;

	/* Give virtio_ring a chance to accept features */
	vring_transport_features(vdev);

	/* Give virtio_rproc a chance to accept features. */
	rproc_transport_features(vdev);

	/* Make sure we don't have any features > 32 bits! */
	WARN_ON((u32)vdev->features != vdev->features);

	/*
	 * Remember the finalized features of our vdev, and provide it
	 * to the remote processor once it is powered on.
	 */
	rsc->gfeatures = vdev->features;

	return 0;
}

static void rproc_virtio_get(struct virtio_device *vdev, unsigned int offset,
			     void *buf, unsigned int len)
{
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);
	struct fw_rsc_vdev *rsc;
	void *cfg;

	rsc = (void *)rvdev->xrisp_rproc->rproc->table_ptr + rvdev->rsc_offset;
	cfg = &rsc->vring[rsc->num_of_vrings];

	if (offset + len > rsc->config_len || offset + len < len) {
		XRISP_DEV_ERROR(&vdev->dev, "access out of bounds\n");
		return;
	}

	memcpy(buf, cfg + offset, len);
}

static void rproc_virtio_set(struct virtio_device *vdev, unsigned int offset,
			     const void *buf, unsigned int len)
{
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);
	struct fw_rsc_vdev *rsc;
	void *cfg;

	rsc = (void *)rvdev->xrisp_rproc->rproc->table_ptr + rvdev->rsc_offset;
	cfg = &rsc->vring[rsc->num_of_vrings];

	if (offset + len > rsc->config_len || offset + len < len) {
		XRISP_DEV_ERROR(&vdev->dev, "access out of bounds\n");
		return;
	}

	memcpy(cfg + offset, buf, len);
}

static bool rproc_get_shm_region(struct virtio_device *vdev,
		struct virtio_shm_region *region, u8 id)
{
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);
	struct rproc_mem_entry *mem;

	XRISP_DEV_DEBUG(&vdev->dev, "get shm\n");

	mem = xrisp_rproc_find_carveout_by_name(rvdev->xrisp_rproc->rproc,
						"vdev%dbuffer", rvdev->index);

	region->addr = (u64)mem->va + XRISP_RPROC_RSC_SIZE;
	region->len = rvdev->vring->len * MAX_RPMSG_BUF_SIZE * 2;

	if (mem->len - XRISP_RPROC_RSC_SIZE < region->len) {
		region->addr = 0;
		region->len = 0;
		XRISP_DEV_ERROR(&vdev->dev, " shm out of bounds\n");
		return false;
	}

	return true;
}

static const struct virtio_config_ops xrisp_rproc_virtio_config_ops = {
	.get_features	= rproc_virtio_get_features,
	.finalize_features = rproc_virtio_finalize_features,
	.find_vqs	= rproc_virtio_find_vqs,
	.del_vqs	= rproc_virtio_del_vqs,
	.reset		= rproc_virtio_reset,
	.set_status	= rproc_virtio_set_status,
	.get_status	= rproc_virtio_get_status,
	.get		= rproc_virtio_get,
	.set		= rproc_virtio_set,
	.get_shm_region = rproc_get_shm_region,
};

/*
 * This function is called whenever vdev is released, and is responsible
 * to decrement the remote processor's refcount which was taken when vdev was
 * added.
 *
 * Never call this function directly; it will be called by the driver
 * core when needed.
 */
static void xrisp_rproc_virtio_dev_release(struct device *dev)
{
	struct virtio_device *vdev = dev_to_virtio(dev);
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);
	struct cam_rproc *xrisp_rproc = vdev_to_xrisp_rproc(vdev);

	kfree(vdev);

	kref_put(&rvdev->refcount, xrisp_rproc_vdev_release);

	put_device(xrisp_rproc->dev);
}

/**
 * xrisp_rproc_add_virtio_dev() - register an rproc-induced virtio device
 * @rvdev: the remote vdev
 * @id: the device type identification (used to match it with a driver).
 *
 * This function registers a virtio device. This vdev's partent is
 * the rproc device.
 *
 * Return: 0 on success or an appropriate error value otherwise
 */
int xrisp_rproc_add_virtio_dev(struct xrisp_rproc_vdev *rvdev, int id)
{
	struct cam_rproc *xrisp_rproc = rvdev->xrisp_rproc;
	struct device *dev = &rvdev->dev;
	struct virtio_device *vdev;
	int ret;

	if (xrisp_rproc->rproc->ops->kick == NULL) {
		ret = -EINVAL;
		XRISP_DEV_ERROR(dev, ".kick method not defined for %s\n",
			      xrisp_rproc->rproc->name);
		goto out;
	}

	/* Allocate virtio device */
	vdev = kzalloc(sizeof(*vdev), GFP_KERNEL);
	if (!vdev) {
		ret = -ENOMEM;
		goto out;
	}
	vdev->id.device	= id,
	vdev->config = &xrisp_rproc_virtio_config_ops,
	vdev->dev.parent = dev;
	vdev->dev.release = xrisp_rproc_virtio_dev_release;

	/*
	 * We're indirectly making a non-temporary copy of the rproc pointer
	 * here, because drivers probed with this vdev will indirectly
	 * access the wrapping rproc.
	 *
	 * Therefore we must increment the rproc refcount here, and decrement
	 * it _only_ when the vdev is released.
	 */
	get_device(xrisp_rproc->dev);

	/* Reference the vdev and vring allocations */
	kref_get(&rvdev->refcount);

	ret = register_virtio_device(vdev);
	if (ret) {
		put_device(&vdev->dev);
		XRISP_DEV_ERROR(dev, "failed to register vdev: %d\n", ret);
		goto out;
	}

	XRISP_DEV_INFO(dev, "registered %s (type %d)\n", dev_name(&vdev->dev), id);

out:
	return ret;
}

/**
 * xrisp_rproc_remove_virtio_dev() - remove an rproc-induced virtio device
 * @dev: the virtio device
 * @data: must be null
 *
 * This function unregisters an existing virtio device.
 *
 * Return: 0
 */
int xrisp_rproc_remove_virtio_dev(struct device *dev, void *data)
{
	struct virtio_device *vdev = dev_to_virtio(dev);

	unregister_virtio_device(vdev);
	return 0;
}
