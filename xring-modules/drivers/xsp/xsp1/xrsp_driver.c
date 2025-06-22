// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 * Description: xring rust secure partition driver
 *              This file implement the FF-A ABI used when communicating with secure partition by FF-A.
 *              This file is divided into the following sections:
 *              1. FF-A Driver initialization.
 *              2. probe ffa driver by xrsp uuid
 *              3. Convert between struct xsp_ffa_msg and struct ffa_send_direct_data
 * Modify time: 2024-07-06
 */

#include "arm_ffa.h"
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/scatterlist.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include "../xsp_ffa.h"

struct xrsp_ffa {
	int is_inited;
	struct ffa_device *dev;
	const struct ffa_ops *ops;
	struct ffa_device_id device_id;
	struct mutex mutex;
};

static struct xrsp_ffa xrsp_ffa = {
	.dev = NULL,
	.ops = NULL,
	/* <0xe0786148 0xe311f8e7 0x2005ebc 0x1bc5d5a5>; */
	.device_id = { UUID_INIT(0x486178e0, 0xe7f8, 0x11e3, 0xbc, 0x5e, 0x00,
				 0x02, 0xa5, 0xd5, 0xc5, 0x1b) },
};

static void xrsp_ffa_remove(struct ffa_device *ffa_dev)
{
	xrsp_ffa.is_inited = 0;
	mutex_destroy(&xrsp_ffa.mutex);
}

static int xrsp_ffa_probe(struct ffa_device *ffa_dev)
{
	const struct ffa_ops *ffa_ops = NULL;
	int rc = 0;

	ffa_ops = ffa_dev->ops;
	if (IS_ERR_OR_NULL(ffa_ops)) {
		rc = PTR_ERR(ffa_ops);
		pr_err("error, ffa ops is NULL %d", rc);
		return rc;
	}

	xrsp_ffa.dev = ffa_dev;
	xrsp_ffa.ops = ffa_ops;
	xrsp_ffa.is_inited = 1;

	mutex_init(&xrsp_ffa.mutex);

	return rc;
}

static struct ffa_driver xrsp_ffa_driver = {
	.name = "xsp1",
	.probe = xrsp_ffa_probe,
	.remove = xrsp_ffa_remove,
	.id_table = &xrsp_ffa.device_id,
};


int xrsp_ffa_direct_message(struct xsp_ffa_msg *msg)
{
	int ret;

	if (!msg) {
		pr_err("error, xsp ffa msg is NULL\n");
		return -EFAULT;
	}

	if (!xrsp_ffa.is_inited) {
		pr_err("error, xrsp not be inited\n");
		return -EAGAIN;
	}

	struct ffa_send_direct_data data = {
		.data0 = msg->fid,
		.data1 = msg->data0,
		.data2 = msg->data1,
		.data3 = msg->data2,
		.data4 = msg->data3,
	};

	ret = xrsp_ffa.ops->msg_ops->sync_send_receive(xrsp_ffa.dev, &data);
	if (ret) {
		pr_err("error, xrsp ffa direct msg %d\n", ret);
		return ret;
	}

	msg->ret = data.data0;
	msg->data0 = data.data1;
	msg->data1 = data.data2;
	msg->data2 = data.data3;
	msg->data3 = data.data4;
	return 0;
}
EXPORT_SYMBOL_GPL(xrsp_ffa_direct_message);

static int xrsp_ffa_register(void)
{
	int ret;

	if (!IS_REACHABLE(CONFIG_XRING_ARM_FFA_TRANSPORT)) {
		pr_err("error, arm ffa not enable, register xrsp ffa fail\n");
		return -EOPNOTSUPP;
	}

	ret = ffa_register(&xrsp_ffa_driver);
	if (ret != 0)
		pr_err("error, register xrsp ffa fail %d\n", ret);
	else
		pr_info("register xrsp ffa success\n");

	return ret;
}

static void xrsp_ffa_unregister(void)
{
	if (!IS_REACHABLE(CONFIG_XRING_ARM_FFA_TRANSPORT))
		pr_info("arm ffa not enable, no need unregister xrsp\n");

	ffa_unregister(&xrsp_ffa_driver);

	pr_info("unregister xrsp ffa success\n");
}

static int __init xrsp_driver_init(void)
{
	return xrsp_ffa_register();
}

/* This is called when the module is removed. */
static void __exit xrsp_driver_exit(void)
{
	xrsp_ffa_unregister();
}

MODULE_AUTHOR("securityAP");
MODULE_DESCRIPTION("X-ring rust sp driver");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xr_ffa-module");
module_init(xrsp_driver_init);
module_exit(xrsp_driver_exit);
