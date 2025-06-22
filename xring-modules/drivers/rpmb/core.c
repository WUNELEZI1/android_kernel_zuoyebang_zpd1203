// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright(c) 2015 - 2019 Intel Corporation. All rights reserved.
 * Copyright(c) 2021 - 2022 Linaro Ltd.
 * Copyright(c) 2023 - 2024 X-Ring technologies Inc.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/version.h>

#include <soc/xring/rpmb.h>
#include "rpmb-cdev.h"

static DEFINE_IDA(rpmb_ida);

/**
 * rpmb_dev_get() - increase rpmb device ref counter
 * @rdev: rpmb device
 */
struct rpmb_dev *rpmb_dev_get(struct rpmb_dev *rdev)
{
	return get_device(&rdev->dev) ? rdev : NULL;
}
EXPORT_SYMBOL_GPL(rpmb_dev_get);

/**
 * rpmb_dev_put() - decrease rpmb device ref counter
 * @rdev: rpmb device
 */
void rpmb_dev_put(struct rpmb_dev *rdev)
{
	put_device(&rdev->dev);
}
EXPORT_SYMBOL_GPL(rpmb_dev_put);

/**
 * rpmb_get_capacity() - returns the capacity of the rpmb device
 * @rdev: rpmb device
 * @region: rpmb region
 *
 * Return:
 * *        0 on success
 * *        < 0 if the operation fails
 */
int rpmb_get_capacity(struct rpmb_dev *rdev, u8 region)
{
	int err;

	if (!rdev)
		return -EINVAL;

	mutex_lock(&rdev->lock);
	err = -EOPNOTSUPP;
	if (rdev->ops && rdev->ops->get_capacity)
		err = rdev->ops->get_capacity(rdev->dev.parent, region);
	mutex_unlock(&rdev->lock);

	return err;
}
EXPORT_SYMBOL_GPL(rpmb_get_capacity);

/**
 * rpmb_program_key() - program the RPMB access key
 * @rdev: rpmb device
 * @region: rpmb region
 * @len: size of request frame
 * @rlen: size of response frame
 * @req: request frame
 * @resp: response frame
 *
 * A successful programming of the key implies it has been set by the
 * driver and can be used.
 *
 * Return:
 * *        0 on success
 * *        < 0 if the operation fails
 */
int rpmb_program_key(struct rpmb_dev *rdev, u8 region,
		     int len, u8 *req, int rlen, u8 *resp)
{
	int err;

	if (!rdev || !len || !req || !rlen || !resp)
		return -EINVAL;

	mutex_lock(&rdev->lock);
	err = -EOPNOTSUPP;
	if (rdev->ops && rdev->ops->program_key)
		err = rdev->ops->program_key(rdev->dev.parent, region,
					     len, req, rlen, resp);
	mutex_unlock(&rdev->lock);

	return err;
}
EXPORT_SYMBOL_GPL(rpmb_program_key);

/**
 * rpmb_get_write_counter() - returns the write counter of the rpmb device
 * @rdev: rpmb device
 * @region: rpmb region
 * @len: size of request frame
 * @rlen: size of response frame
 * @req: request frame
 * @resp: response frame
 *
 * Return:
 * *        0 on success
 * *        < 0 if the operation fails
 */
int rpmb_get_write_counter(struct rpmb_dev *rdev, u8 region,
			   int len, u8 *req, int rlen, u8 *resp)
{
	int err;

	if (!rdev || !len || !req || !rlen || !resp)
		return -EINVAL;

	mutex_lock(&rdev->lock);
	err = -EOPNOTSUPP;
	if (rdev->ops && rdev->ops->get_write_counter)
		err = rdev->ops->get_write_counter(rdev->dev.parent, region,
						   len, req, rlen, resp);
	mutex_unlock(&rdev->lock);

	return err;
}
EXPORT_SYMBOL_GPL(rpmb_get_write_counter);

/**
 * rpmb_write_blocks() - write data to RPMB device
 * @rdev: rpmb device
 * @region: rpmb region
 * @len: size of request frame
 * @rlen: size of response frame
 * @req: request frame
 * @resp: response frame
 *
 * Write a series of blocks to the RPMB device.
 *
 * Return:
 * *        0 on success
 * *        < 0 if the operation fails
 */
int rpmb_write_blocks(struct rpmb_dev *rdev, u8 region,
		      int len, u8 *req, int rlen, u8 *resp)
{
	int err;

	if (!rdev || !len || !req || !rlen || !resp)
		return -EINVAL;

	mutex_lock(&rdev->lock);
	err = -EOPNOTSUPP;
	if (rdev->ops && rdev->ops->write_blocks)
		err = rdev->ops->write_blocks(rdev->dev.parent, region,
					      len, req, rlen, resp);
	mutex_unlock(&rdev->lock);

	return err;
}
EXPORT_SYMBOL_GPL(rpmb_write_blocks);

/**
 * rpmb_read_blocks() - read data from RPMB device
 * @rdev: rpmb device
 * @region: rpmb region
 * @len: size of request frame
 * @rlen: size of response frame
 * @req: request frame
 * @resp: response frame
 *
 * Read a series of blocks from the RPMB device.
 *
 * Return:
 * *        0 on success
 * *        < 0 if the operation fails
 */
int rpmb_read_blocks(struct rpmb_dev *rdev, u8 region,
		     int len, u8 *req, int rlen, u8 *resp)
{
	int err;

	if (!rdev || !len || !req || !rlen || !resp)
		return -EINVAL;

	mutex_lock(&rdev->lock);
	err = -EOPNOTSUPP;
	if (rdev->ops && rdev->ops->read_blocks)
		err = rdev->ops->read_blocks(rdev->dev.parent, region,
					     len, req, rlen, resp);
	mutex_unlock(&rdev->lock);

	return err;
}
EXPORT_SYMBOL_GPL(rpmb_read_blocks);

static void rpmb_dev_release(struct device *dev)
{
	struct rpmb_dev *rdev = to_rpmb_dev(dev);

	ida_simple_remove(&rpmb_ida, rdev->id);
	kfree(rdev);
}

struct class rpmb_class = {
	.name = "rpmb",
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	.owner = THIS_MODULE,
#endif
	.dev_release = rpmb_dev_release,
};

/**
 * rpmb_dev_find_device() - return first matching rpmb device
 * @data: data for the match function
 * @match: the matching function
 *
 * Return: matching rpmb device or NULL on failure
 */
static
struct rpmb_dev *rpmb_dev_find_device(const void *data,
				      int (*match)(struct device *dev,
						   const void *data))
{
	struct device *dev;

	dev = class_find_device(&rpmb_class, NULL, data, match);

	return dev ? to_rpmb_dev(dev) : NULL;
}

static int match_by_parent(struct device *dev, const void *data)
{
	const struct device *d = data;

	return dev->parent == d;
}

/**
 * rpmb_dev_find_by_device() - retrieve rpmb device from the parent device
 *     Should be called with lock held
 * @parent: parent device of the rpmb device
 *
 * Return: NULL if there is no rpmb device associated with the parent device
 */
struct rpmb_dev *rpmb_dev_find_by_device(struct device *parent)
{
	if (!parent)
		return NULL;

	return rpmb_dev_find_device(parent, match_by_parent);
}
EXPORT_SYMBOL_GPL(rpmb_dev_find_by_device);

/**
 * rpmb_dev_find_by_name() - retrieve rpmb device from name
 * @name: name of the rpmb device
 *
 * Return: NULL if there is no rpmb device associated with the name
 */
struct rpmb_dev *rpmb_dev_find_by_name(const char *name)
{
	struct device *dev;

	if (!name)
		return NULL;

	dev = class_find_device_by_name(&rpmb_class, name);

	return dev ? to_rpmb_dev(dev) : NULL;
}
EXPORT_SYMBOL_GPL(rpmb_dev_find_by_name);

/**
 * rpmb_dev_unregister() - unregister RPMB partition from the RPMB subsystem
 * @rdev: the rpmb device to unregister
 * Return:
 * *        0 on success
 * *        -EINVAL on wrong parameters
 */
static int rpmb_dev_unregister(struct rpmb_dev *rdev)
{
	if (!rdev)
		return -EINVAL;

	mutex_lock(&rdev->lock);
	rpmb_cdev_del(rdev);
	device_del(&rdev->dev);
	mutex_unlock(&rdev->lock);

	rpmb_dev_put(rdev);

	return 0;
}

/**
 * rpmb_dev_unregister_by_device() - unregister RPMB partition
 *     from the RPMB subsystem
 * @dev: the parent device of the rpmb device
 * Return:
 * *        0 on success
 * *        -EINVAL on wrong parameters
 * *        -ENODEV if a device cannot be find.
 */
int rpmb_dev_unregister_by_device(struct device *dev)
{
	struct rpmb_dev *rdev;

	if (!dev)
		return -EINVAL;

	rdev = rpmb_dev_find_by_device(dev);
	if (!rdev) {
		dev_warn(dev, "no disk found %s\n", dev_name(dev->parent));
		return -ENODEV;
	}

	rpmb_dev_put(rdev);

	return rpmb_dev_unregister(rdev);
}
EXPORT_SYMBOL_GPL(rpmb_dev_unregister_by_device);

/**
 * rpmb_dev_get_drvdata() - driver data getter
 * @rdev: rpmb device
 *
 * Return: driver private data
 */
void *rpmb_dev_get_drvdata(const struct rpmb_dev *rdev)
{
	return dev_get_drvdata(&rdev->dev);
}
EXPORT_SYMBOL_GPL(rpmb_dev_get_drvdata);

/**
 * rpmb_dev_set_drvdata() - driver data setter
 * @rdev: rpmb device
 * @data: data to store
 */
void rpmb_dev_set_drvdata(struct rpmb_dev *rdev, void *data)
{
	dev_set_drvdata(&rdev->dev, data);
}
EXPORT_SYMBOL_GPL(rpmb_dev_set_drvdata);

/**
 * rpmb_dev_register - register RPMB partition with the RPMB subsystem
 * @dev: storage device of the rpmb device
 * @region: RPMB region within the physical device
 * @ops: device specific operations
 *
 * Return: a pointer to rpmb device
 */
struct rpmb_dev *rpmb_dev_register(struct device *dev,
				   const struct rpmb_ops *ops)
{
	struct rpmb_dev *rdev;
	int id;
	int ret;

	if (!dev || !ops)
		return ERR_PTR(-EINVAL);

	rdev = kzalloc(sizeof(*rdev), GFP_KERNEL);
	if (!rdev)
		return ERR_PTR(-ENOMEM);

	id = ida_simple_get(&rpmb_ida, 0, 0, GFP_KERNEL);
	if (id < 0) {
		ret = id;
		goto exit;
	}

	mutex_init(&rdev->lock);
	rdev->ops = ops;
	rdev->id = id;

	dev_set_name(&rdev->dev, "rpmb%d", id);
	rdev->dev.class = &rpmb_class;
	rdev->dev.parent = dev;

	rpmb_cdev_prepare(rdev);

	ret = device_register(&rdev->dev);
	if (ret)
		goto exit;

	ret = rpmb_cdev_add(rdev);
	if (ret)
		goto exit;

	dev_dbg(&rdev->dev, "registered device\n");

	return rdev;

exit:
	if (id >= 0)
		ida_simple_remove(&rpmb_ida, id);
	kfree(rdev);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(rpmb_dev_register);

static int __init rpmb_init(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
	int ret;
#endif

	ida_init(&rpmb_ida);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	class_register(&rpmb_class);
#else
	ret = class_register(&rpmb_class);
	if (ret) {
		pr_err("rpmb: register class fail: %d\n", ret);
		ida_destroy(&rpmb_ida);
		return ret;
	}
#endif

	return rpmb_cdev_init();
}

static void __exit rpmb_exit(void)
{
	rpmb_cdev_exit();
	class_unregister(&rpmb_class);
	ida_destroy(&rpmb_ida);
}

subsys_initcall(rpmb_init);
module_exit(rpmb_exit);

MODULE_AUTHOR("Intel Corporation");
MODULE_DESCRIPTION("RPMB class");
MODULE_LICENSE("GPL v2");
