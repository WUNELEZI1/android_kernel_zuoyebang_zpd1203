// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "rproc_vdev", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "rproc_vdev", __func__, __LINE__

#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/dma-direct.h> /* XXX: pokes into bus_dma_range */
#include <linux/virtio_ids.h>
#include <linux/virtio_ring.h>
#include <linux/of_reserved_mem.h>
#include <linux/workqueue.h>
#include "xrisp_rproc.h"
#include "xrisp_log.h"

/**
 * xrisp_rproc_check_carveout_da() - Check specified carveout da configuration
 * @rproc: handle of a remote processor
 * @mem: pointer on carveout to check
 * @da: area device address
 * @len: associated area size
 *
 * This function is a helper function to verify requested device area (couple
 * da, len) is part of specified carveout.
 * If da is not set (defined as FW_RSC_ADDR_ANY), only requested length is
 * checked.
 *
 * Return: 0 if carveout matches request else error
 */
static int xrisp_rproc_check_carveout_da(struct rproc *rproc,
				   struct rproc_mem_entry *mem, u32 da, u32 len)
{
	struct device *dev = &rproc->dev;
	int delta;

	/* Check requested resource length */
	if (len > mem->len) {
		XRISP_DEV_ERROR(dev, "Registered carveout doesn't fit len request\n");
		return -EINVAL;
	}

	if (da != FW_RSC_ADDR_ANY && mem->da == FW_RSC_ADDR_ANY) {
		/* Address doesn't match registered carveout configuration */
		return -EINVAL;
	} else if (da != FW_RSC_ADDR_ANY && mem->da != FW_RSC_ADDR_ANY) {
		delta = da - mem->da;

		/* Check requested resource belongs to registered carveout */
		if (delta < 0) {
			XRISP_DEV_ERROR(dev,
				"Registered carveout doesn't fit da request\n");
			return -EINVAL;
		}

		if (delta + len > mem->len) {
			XRISP_DEV_ERROR(dev,
				"Registered carveout doesn't fit len request\n");
			return -EINVAL;
		}
	}

	return 0;
}

int xrisp_rproc_alloc_vring(struct xrisp_rproc_vdev *rvdev, int i)
{

	struct cam_rproc *xrisp_rproc = rvdev->xrisp_rproc;
	struct device *dev = xrisp_rproc->dev;
	struct xrisp_rproc_vring *rvring = &rvdev->vring[i];
	struct fw_rsc_vdev *rsc;
	int ret, notifyid;
	struct rproc_mem_entry *mem;
	size_t size;

	/* actual size of vring (in bytes) */
	size = PAGE_ALIGN(vring_size(rvring->len, rvring->align));

	rsc = (void *)xrisp_rproc->rproc->table_ptr + rvdev->rsc_offset;

	/* Search for pre-registered carveout */
	mem = xrisp_rproc_find_carveout_by_name(xrisp_rproc->rproc, "vdev%dvring%d", rvdev->index,
					  i);
	if (mem) {
		if (xrisp_rproc_check_carveout_da(xrisp_rproc->rproc, mem, rsc->vring[i].da, size))
			return -ENOMEM;
	} else {
		XRISP_DEV_ERROR(dev, "Can't allocate memory entry structure\n");
		return -ENOMEM;
	}

	/*
	 * Assign an rproc-wide unique index for this vring
	 * TODO: assign a notifyid for rvdev updates as well
	 * TODO: support predefined notifyids (via resource table)
	 */
	ret = idr_alloc(&xrisp_rproc->notifyids, rvring, 0, 0, GFP_KERNEL);
	if (ret < 0) {
		XRISP_DEV_ERROR(dev, "idr_alloc failed: %d\n", ret);
		return ret;
	}
	notifyid = ret;

	/* Potentially bump max_notifyid */
	if (notifyid > xrisp_rproc->max_notifyid)
		xrisp_rproc->max_notifyid = notifyid;

	rvring->notifyid = notifyid;

	/* Let the rproc know the notifyid of this vring.*/
	rsc->vring[i].notifyid = notifyid;
	return 0;
}

static int
xrisp_rproc_parse_vring(struct xrisp_rproc_vdev *rvdev, struct fw_rsc_vdev *rsc, int i)
{
	struct cam_rproc *xrisp_rproc = rvdev->xrisp_rproc;
	struct device *dev = xrisp_rproc->dev;
	struct fw_rsc_vdev_vring *vring = &rsc->vring[i];
	struct xrisp_rproc_vring *rvring = &rvdev->vring[i];

	XRISP_DEV_DEBUG(dev, "vdev rsc: vring%d: da 0x%x, qsz %d, align %d\n",
		i, vring->da, vring->num, vring->align);

	/* verify queue size and vring alignment are sane */
	if (!vring->num || !vring->align) {
		XRISP_DEV_ERROR(dev, "invalid qsz (%d) or alignment (%d)\n",
			vring->num, vring->align);
		return -EINVAL;
	}

	rvring->len = vring->num;
	rvring->align = vring->align;
	rvring->rvdev = rvdev;

	return 0;
}

void xrisp_rproc_free_vring(struct xrisp_rproc_vring *rvring)
{
	struct cam_rproc *xrisp_rproc = rvring->rvdev->xrisp_rproc;
	int idx = rvring - rvring->rvdev->vring;
	struct fw_rsc_vdev *rsc;

	idr_remove(&xrisp_rproc->notifyids, rvring->notifyid);

	/*
	 * At this point rproc_stop() has been called and the installed resource
	 * table in the remote processor memory may no longer be accessible. As
	 * such and as per rproc_stop(), rproc->table_ptr points to the cached
	 * resource table (rproc->cached_table).  The cached resource table is
	 * only available when a remote processor has been booted by the
	 * remoteproc core, otherwise it is NULL.
	 *
	 * Based on the above, reset the virtio device section in the cached
	 * resource table only if there is one to work with.
	 */
	if (xrisp_rproc->rproc->table_ptr) {
		rsc = (void *)xrisp_rproc->rproc->table_ptr + rvring->rvdev->rsc_offset;
		rsc->vring[idx].da = 0;
		rsc->vring[idx].notifyid = -1;
	}
}

static int xrisp_rproc_vdev_do_prepare(struct rproc_subdev *subdev)
{
	struct xrisp_rproc_vdev *rvdev = container_of(subdev, struct xrisp_rproc_vdev, subdev);
	struct cam_rproc *xrisp_rproc = rvdev->xrisp_rproc;
	struct rproc_mem_entry *mem;
	struct cam_rproc_mem *cam_mem;
	struct xrisp_rproc_rsc_table *rsc_ptr;

	/*
	 * Handle vdev buffer rsc for nuttx simple addrenv,
	 * let our firmware know where vbuffer is.
	 */
	rsc_ptr = (struct xrisp_rproc_rsc_table *)xrisp_rproc->rproc->table_ptr;
	mem = xrisp_rproc_find_carveout_by_name(xrisp_rproc->rproc, "vdev%dbuffer", rvdev->index);
	if (!mem || !mem->priv) {
		XRISP_PR_ERROR("can not fine vdev buffer %d\n", rvdev->index);
		return -ENOMEM;
	}
	cam_mem = mem->priv;
	if (!cam_mem->avail) {
		XRISP_PR_ERROR("vdev buffer %d invalid\n", rvdev->index);
		return -ENOMEM;
	}
	rsc_ptr->rpmsg_vbuffer.da = mem->da;
	rsc_ptr->rpmsg_vbuffer.pa = cam_mem->pa;
	rsc_ptr->rpmsg_vbuffer.len = mem->len;

	XRISP_PR_INFO("vdev buffer pa 0x%pK da 0x%x len 0x%x",
		      (void *)(uint64_t)rsc_ptr->rpmsg_vbuffer.pa, rsc_ptr->rpmsg_vbuffer.da,
		      rsc_ptr->rpmsg_vbuffer.len);

	return 0;
}

static int xrisp_rproc_vdev_do_start(struct rproc_subdev *subdev)
{
	struct xrisp_rproc_vdev *rvdev = container_of(subdev, struct xrisp_rproc_vdev, subdev);

	return xrisp_rproc_add_virtio_dev(rvdev, rvdev->id);
}

static void xrisp_rproc_vdev_do_stop(struct rproc_subdev *subdev, bool crashed)
{
	struct xrisp_rproc_vdev *rvdev = container_of(subdev, struct xrisp_rproc_vdev, subdev);
	int ret;

	if (rvdev->xrisp_rproc) {
		ret = cancel_work_sync(&rvdev->xrisp_rproc->kick_work);
		XRISP_PR_INFO("work cancel ret=%d", ret);
	}

	ret = device_for_each_child(&rvdev->dev, NULL, xrisp_rproc_remove_virtio_dev);
	if (ret)
		XRISP_DEV_WARN(&rvdev->dev, "can't remove vdev child device: %d\n", ret);
}

/**
 * xrisp_rproc_rvdev_release() - release the existence of a rvdev
 *
 * @dev: the subdevice's dev
 */
static void xrisp_rproc_rvdev_release(struct device *dev)
{
	struct xrisp_rproc_vdev *rvdev = container_of(dev, struct xrisp_rproc_vdev, dev);

	of_reserved_mem_device_release(dev);
	// dma_release_coherent_memory(dev);

	kfree(rvdev);
}

static int copy_dma_range_map(struct device *to, struct device *from)
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

/**
 * xrisp_rproc_handle_vdev() - handle a vdev fw resource
 * @rproc: the remote processor
 * @ptr: the vring resource descriptor
 * @offset: offset of the resource entry
 * @avail: size of available data (for sanity checking the image)
 *
 * This resource entry requests the host to statically register a virtio
 * device (vdev), and setup everything needed to support it. It contains
 * everything needed to make it possible: the virtio device id, virtio
 * device features, vrings information, virtio config space, etc...
 *
 * Before registering the vdev, the vrings are allocated from non-cacheable
 * physically contiguous memory. Currently we only support two vrings per
 * remote processor (temporary limitation). We might also want to consider
 * doing the vring allocation only later when ->find_vqs() is invoked, and
 * then release them upon ->del_vqs().
 *
 * Note: @da is currently not really handled correctly: we dynamically
 * allocate it using the DMA API, ignoring requested hard coded addresses,
 * and we don't take care of any required IOMMU programming. This is all
 * going to be taken care of when the generic iommu-based DMA API will be
 * merged. Meanwhile, statically-addressed iommu-based firmware images should
 * use RSC_DEVMEM resource entries to map their required @da to the physical
 * address of their base CMA region (ouch, hacky!).
 *
 * Return: 0 on success, or an appropriate error code otherwise
 */

int xrisp_rproc_handle_vdev(struct rproc *rproc, void *ptr,
			     int offset, int avail)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct fw_rsc_vdev *rsc = ptr;
	struct device *dev = xrisp_rproc->dev;
	struct xrisp_rproc_vdev *rvdev;
	int i, ret;
	char name[16];
	struct xrisp_rproc_rsc_table *rsc_ptr;

	XRISP_PR_INFO("handle vdev rsc");

	rsc_ptr = (struct xrisp_rproc_rsc_table *)rproc->table_ptr;

	/* make sure resource isn't truncated */
	if (struct_size(rsc, vring, rsc->num_of_vrings) + rsc->config_len >
			avail) {
		XRISP_DEV_ERROR(dev, "vdev rsc is truncated\n");
		return -EINVAL;
	}

	/* make sure reserved bytes are zeroes */
	if (rsc->reserved[0] || rsc->reserved[1]) {
		XRISP_DEV_ERROR(dev, "vdev rsc has non zero reserved bytes\n");
		return -EINVAL;
	}

	XRISP_DEV_DEBUG(dev, "vdev rsc: id %d, dfeatures 0x%x, cfg len %d, %d vrings\n",
		rsc->id, rsc->dfeatures, rsc->config_len, rsc->num_of_vrings);

	rvdev = kzalloc(sizeof(*rvdev), GFP_KERNEL);
	if (!rvdev)
		return -ENOMEM;

	/* we currently support only two vrings per rvdev */
	if (rsc->num_of_vrings > ARRAY_SIZE(rvdev->vring)) {
		XRISP_DEV_ERROR(dev, "too many vrings: %d\n", rsc->num_of_vrings);
		kfree(rvdev);
		return -EINVAL;
	}

	/* Our firmware should handle the rsc as RSC_VDEV */
	rsc_ptr->rpmsg_vdev_hdr.type = RSC_VDEV;

	kref_init(&rvdev->refcount);

	rvdev->id = rsc->id;
	rvdev->xrisp_rproc = xrisp_rproc;
	rvdev->index = xrisp_rproc->nb_vdev++;

	/* Initialise vdev subdevice */
	snprintf(name, sizeof(name), "vdev%dbuffer", rvdev->index);
	rvdev->dev.parent = xrisp_rproc->dev;
	rvdev->dev.release = xrisp_rproc_rvdev_release;
	dev_set_name(&rvdev->dev, "%s#%s", dev_name(rvdev->dev.parent), name);
	dev_set_drvdata(&rvdev->dev, rvdev);

	ret = device_register(&rvdev->dev);
	if (ret) {
		put_device(&rvdev->dev);
		kfree(rvdev);
		return ret;
	}

	ret = copy_dma_range_map(&rvdev->dev, xrisp_rproc->dev);
	if (ret)
		goto free_rvdev;

	/* Make device dma capable by inheriting from parent's capabilities */
	set_dma_ops(&rvdev->dev, get_dma_ops(xrisp_rproc->dev));

	ret = dma_coerce_mask_and_coherent(&rvdev->dev,
					   dma_get_mask(xrisp_rproc->dev));
	if (ret) {
		XRISP_DEV_WARN(dev,
			 "Failed to set DMA mask %llx. Trying to continue... %x\n",
			 dma_get_mask(xrisp_rproc->dev), ret);
	}

	/* parse the vrings */
	for (i = 0; i < rsc->num_of_vrings; i++) {
		ret = xrisp_rproc_parse_vring(rvdev, rsc, i);
		if (ret)
			goto free_rvdev;
	}

	/* remember the resource offset*/
	rvdev->rsc_offset = offset;

	/* allocate the vring resources */
	for (i = 0; i < rsc->num_of_vrings; i++) {
		ret = xrisp_rproc_alloc_vring(rvdev, i);
		if (ret)
			goto unwind_vring_allocations;
	}

	list_add_tail(&rvdev->node, &xrisp_rproc->rvdevs);

	rvdev->subdev.prepare = xrisp_rproc_vdev_do_prepare;
	rvdev->subdev.start = xrisp_rproc_vdev_do_start;
	rvdev->subdev.stop = xrisp_rproc_vdev_do_stop;

	rproc_add_subdev(rproc, &rvdev->subdev);

	return 0;

unwind_vring_allocations:
	for (i--; i >= 0; i--)
		xrisp_rproc_free_vring(&rvdev->vring[i]);
free_rvdev:
	device_unregister(&rvdev->dev);
	kfree(rvdev);
	return ret;
}

void xrisp_rproc_vdev_release(struct kref *ref)
{
	struct xrisp_rproc_vdev *rvdev = container_of(ref, struct xrisp_rproc_vdev, refcount);
	struct xrisp_rproc_vring *rvring;
	struct cam_rproc *xrisp_rproc = rvdev->xrisp_rproc;
	int id;

	XRISP_PR_INFO("put kref");

	for (id = 0; id < ARRAY_SIZE(rvdev->vring); id++) {
		rvring = &rvdev->vring[id];
		xrisp_rproc_free_vring(rvring);
	}

	rproc_remove_subdev(xrisp_rproc->rproc, &rvdev->subdev);
	list_del(&rvdev->node);
	device_unregister(&rvdev->dev);
}
