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
#include <linux/miscdevice.h>
#include <linux/compat.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/model_protection.h>

#include "crypto_internal.h"

#if IS_ENABLED(CONFIG_XRING_DEBUG)
int crypto_log_level = CRYPTO_LOG_LEVEL_DEF;
module_param(crypto_log_level, int, 0644);
MODULE_PARM_DESC(crypto_log_level, "Control log level (0=none, 1=info, ...)");
#endif

#define MODEL_HEADER_SIZE        96
#define MODEL_KEY_SIZE           32

struct model_key_data {
	uint32_t model_header_size;
	u8 model_header_test[MODEL_HEADER_SIZE];
	uint32_t model_key_size;
	u8 model_key_test[MODEL_KEY_SIZE];
};

#define HPC_CRYPTO_GET_MODEL_KEY \
	_IOWR('X', 0, struct model_key_data)

#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
static int mitee_get_model_key(u8 *model_header_test, u8 *model_key_buffer, uint32_t *buffer_size)
{
	int ret;
	errno_t errno;
	size_t key_size = MODEL_KEY_SIZE;
	void *mitee_model_key = kmalloc(MODEL_KEY_SIZE, GFP_KERNEL);

	ret = get_model_key(model_header_test, MODEL_HEADER_SIZE, (u8 *)mitee_model_key, &key_size);
	if (ret != 0) {
		cryptoerr("get_model_key failed\n");
		goto exit;
	}

	if (key_size != MODEL_KEY_SIZE) {
		cryptoerr("Wrong model key size\n");
		ret = -EINVAL;
		goto exit;
	}

	errno = memcpy_s(model_key_buffer, *buffer_size, mitee_model_key, key_size);
	if (errno != EOK) {
		ret = -EINVAL;
		cryptoerr("memcpy failed\n");
		goto exit;
	}
	*buffer_size = (uint32_t)key_size;

exit:
	kfree(mitee_model_key);

	return ret;
}
#else
static inline int mitee_get_model_key(u8 *model_header_test, u8 *model_key_buffer, uint32_t *buffer_size)
{
	return -ENODEV;
}
#endif

static int invoke_get_model_key(void *argp)
{
	int ret = 0;
	struct model_key_data data;

	if (copy_from_user(&data, argp, sizeof(struct model_key_data))) {
		cryptoerr("copy_from_user failed");
		return -EINVAL;
	}

	ret = mitee_get_model_key(data.model_header_test, data.model_key_test, &data.model_key_size);
	if (ret != 0) {
		cryptoerr("get_model_key failed");
		return ret;
	}

	if (copy_to_user(argp, &data, sizeof(struct model_key_data))) {
		cryptoerr("copy_to_user failed");
		return -EINVAL;
	}

	return ret;
}

static long hpc_crypto_ioctl(struct file *filp, unsigned int ioctl,
		unsigned long arg)
{
	cryptoinfo("entry\n");
	int ret = 0;

	if (!filp->private_data) {
		cryptoerr("filp->private_data is null");
		return -EPERM;
	}

	switch (ioctl) {
	case HPC_CRYPTO_GET_MODEL_KEY:
		ret = invoke_get_model_key((void __user *)arg);
		break;

	default:
		cryptoerr("Unsupported ioctl\n");
		ret = -EINVAL;
	}
	cryptoinfo("exit\n");
	return ret;
}

static int hpc_crypto_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	return ret;
}

static int hpc_crypto_release(struct inode *inode, struct file *file)
{
	int ret = 0;
	return ret;
}

static const struct file_operations misc_fops = {  //在结构体中引入相关函数
	.open = hpc_crypto_open,
	.release = hpc_crypto_release,
	.unlocked_ioctl = hpc_crypto_ioctl,
	.compat_ioctl = compat_ptr_ioctl,
};

struct miscdevice misc_dev = {
	.minor = MISC_DYNAMIC_MINOR, //次设备号
	.name = "hpc-mitee-crypto",   //设备节点的名字
	.fops = &misc_fops
};

static int hpc_mitee_crypto_cdev_probe(struct platform_device *pdev)
{
	int ret = 0;

	cryptoinfo("entry\n");
	ret = misc_register(&misc_dev);
	if (ret != 0)
		cryptoerr("register crypto cdev failed: %d\n", ret);
	cryptoinfo("exit\n");
	return ret;
}

static int hpc_mitee_crypto_cdev_remove(struct platform_device *pdev)
{
	cryptoinfo("entry\n");
	misc_deregister(&misc_dev);
	cryptoinfo("exit\n");
	return 0;
}

static const struct of_device_id hpc_mitee_crypto_cdev_id_table[] = {
	{ .compatible = "xring,hpc-mitee-crypto-cdev" },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, hpc_mitee_crypto_cdev_id_table);

static struct platform_driver hpc_mitee_crypto_cdev_driver = {
	.probe = hpc_mitee_crypto_cdev_probe,
	.remove = hpc_mitee_crypto_cdev_remove,
	.driver = {
		.name = "hpc-mitee-crypto-cdev",
		.of_match_table = of_match_ptr(hpc_mitee_crypto_cdev_id_table),
	},
};
module_platform_driver(hpc_mitee_crypto_cdev_driver);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC MITEE CRYPTO Driver");
MODULE_LICENSE("GPL v2");
