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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#include "fk-audio-test.h"
#include "fk-audio-simulate.h"
#include <soc/xring/ipc/xr_ipc.h>
#include <soc/xring/ipc/xr_rproc.h>
#include <soc/xring/ipc/xr_ipc_test.h>

#define audio_err(fmt, args...)	pr_err("[audif]" fmt "\n", ##args)
#define audio_info(fmt, args...)	pr_info("[audif]" fmt "\n", ##args)
#define audio_debug(fmt, args...)	pr_debug("[audif]" fmt "\n", ##args)

struct audio_callback_t audio_test_cb;

static void audio_ipc_recv_callback_test(void *msg, int len, void *priv)
{
	unsigned int *recv_msg;
	int ret;
	int i;

	recv_msg = kcalloc(1, sizeof(*recv_msg), GFP_KERNEL);
	if (!recv_msg)
		return;

	ret = memcpy_s((unsigned int *)recv_msg,
			(MBOX_DATA_BASE_COUNT * sizeof(*recv_msg)),
			msg, (sizeof(*recv_msg)));
	if (ret) {
		audio_err("%s: memcpy_s failed", __func__);

		kfree(recv_msg);
		return;
	}

	for (i = 0; i < len; i++) {
		audio_info("receive msg[%d] = 0x%08x",
			i, recv_msg[i]);
	}

	kfree(recv_msg);
}

static void xr_audio_test_cb_get(void)
{
	audio_test_cb.callback = audio_ipc_recv_callback_test;
	audio_test_cb.priv = NULL;
}

static long xr_audio_test_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct xring_aud_msg send_msg;
	int ret;

	send_msg.header.scene_id = 0x1;
	send_msg.header.ins_id = 0x1;
	send_msg.header.cmd_id = 0x0;

	pr_info("xring_aud_msg len: %d\n", sizeof(struct xring_aud_msg));
	switch (cmd) {
	case AUDIO_TEST_CASE_000:
		xr_audio_test_cb_get();
		ret = fk_simulate_recv_register(audio_test_cb.callback, NULL);
		break;
	case AUDIO_TEST_CASE_001:
		ret = fk_simulate_send_sync((unsigned int *)&send_msg,
				sizeof(struct xring_aud_msg));
		break;
	case AUDIO_TEST_CASE_002:
		ret = fk_simulate_send_async((unsigned int *)&send_msg,
				sizeof(struct xring_aud_msg),
					NULL, NULL);
		break;
	case AUDIO_TEST_CASE_003:
		ret = 0;
		break;
	case AUDIO_TEST_CASE_004:
		ret = 0;
		break;
	case AUDIO_TEST_CASE_005:

		ret = 0;
		break;
	case AUDIO_TEST_CASE_006:
		fk_simulate_dsp_send_sync((void *)&send_msg);
		ret = 0;
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static int xr_audio_test_dev_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int xr_audio_test_dev_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations xr_audio_test_dev_fops = {
	.owner = THIS_MODULE,
	.open = xr_audio_test_dev_open,
	.release = xr_audio_test_dev_release,
	.unlocked_ioctl = xr_audio_test_dev_ioctl,
};

static struct cdev xr_audio_test_cdev;
struct class *xr_audio_test_class;
static int raw_major;
static DEFINE_IDA(minors);

int xr_audio_test_init(void)
{
	int ret;
	dev_t devno;
	struct class *xr_audio_test_class;
	int minor;

	xr_audio_test_class = class_create(THIS_MODULE, "xr_audio_test_class");
	if (IS_ERR(xr_audio_test_class)) {
		audio_err("%s: class_create failed", __func__);
		return PTR_ERR(xr_audio_test_class);
	}

	ret = alloc_chrdev_region(&devno, 0, 1, "xr_audio_test_dev");
	if (ret < 0) {
		audio_err("%s: alloc_chrdev_region failed", __func__);
		return ret;
	}

	raw_major = MAJOR(devno);

	minor = ida_simple_get(&minors, 0, 0, GFP_KERNEL);
	if (minor < 0) {
		audio_err("%s: ida_simple_get failed", __func__);
		return minor;
	}

	devno = MKDEV(raw_major, minor);
	cdev_init(&xr_audio_test_cdev, &xr_audio_test_dev_fops);
	xr_audio_test_cdev.owner = THIS_MODULE;

	ret = cdev_add(&xr_audio_test_cdev, devno, 1);
	if (ret < 0) {
		audio_err("%s: cdev_add failed", __func__);
		unregister_chrdev_region(devno, 1);
		return ret;
	}

	//callback func

	device_create(xr_audio_test_class, NULL, devno, NULL, "xr_audio_test_dev");

	return 0;
}

void xr_audio_test_exit(void)
{
	cdev_del(&xr_audio_test_cdev);
	unregister_chrdev_region(xr_audio_test_cdev.dev, 1);
}

MODULE_DESCRIPTION("XRING AUDIO COMMON TEST");
MODULE_LICENSE("Dual BSD/GPL");
