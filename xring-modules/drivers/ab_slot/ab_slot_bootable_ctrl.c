// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/sys_ctrl_por_reg.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/version.h>

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": %s:%d: " fmt, __func__, __LINE__

#define SYS_CTRL_POR_REG_USEC0	(ACPU_LMS_SYS_CTRL + SYS_CTRL_POR_REG_SC_REG_USEC0)
#define SLOTA_BOOTABLE_BIT	13
#define SLOTB_BOOTABLE_BIT	14
#define SLOTA_BOOTABLE_MASK	(1UL << 29)
#define SLOTB_BOOTABLE_MASK	(1UL << 30)
#define SLOT_A			0
#define SLOT_B			1
#define SLOT_BOOTABLE		1
#define SLOT_UNBOOTABLE		0
#define MAX_NUM_SLOTS		2

#define DEVICE_NAME		"slot_bootable"
#define CLASS_NAME		"ab_slot"
#define DEVICE_MAJOR		0
#define DEVICE_MINOR		0
#define DEVICE_COUNT		1
#define MAX_ARGS_LEN		32

static void __iomem *bootable_ctrl_reg;
static dev_t bootable_ctrl_dev;
static struct cdev bootable_ctrl_cdev;
static struct class *bootable_ctrl_class;
static struct device *bootable_ctrl_device;

static u8 get_slot_bootable(u32 slot)
{
	u8 bootable;

	if (slot == SLOT_A)
		bootable = (((readl(bootable_ctrl_reg) & (1 << SLOTA_BOOTABLE_BIT)) >>
			   SLOTA_BOOTABLE_BIT) == 0);
	else
		bootable = (((readl(bootable_ctrl_reg) & (1 << SLOTB_BOOTABLE_BIT)) >>
			   SLOTB_BOOTABLE_BIT) == 0);

	return bootable;
}

static void set_slot_bootable(u32 slot, u32 bootable)
{
	u32 val = !bootable;

	if (slot == SLOT_A)
		writel((val << SLOTA_BOOTABLE_BIT) | SLOTA_BOOTABLE_MASK, bootable_ctrl_reg);
	else
		writel((val << SLOTB_BOOTABLE_BIT) | SLOTB_BOOTABLE_MASK, bootable_ctrl_reg);
}

static int ab_slot_bootable_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int ab_slot_bootable_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t ab_slot_bootable_read(struct file *filp, char __user *buf,
				     size_t count, loff_t *f_pos)
{
	u8 bootable_array[MAX_NUM_SLOTS] = { 0 };
	int ret;

	if (*f_pos >= sizeof(bootable_array)) {
		pr_info("nothing to be read\n");
		return 0;
	}
	bootable_array[SLOT_A] = get_slot_bootable(SLOT_A);
	bootable_array[SLOT_B] = get_slot_bootable(SLOT_B);
	count = sizeof(bootable_array);
	ret = copy_to_user(buf, bootable_array, count);
	if (ret) {
		pr_err("copy_to_user failed, ret = %d\n", ret);
		/* set count to zero since copied error */
		return 0;
	}
	*f_pos = sizeof(bootable_array);
	return count;
}

static ssize_t ab_slot_bootable_write(struct file *filp, const char __user *buf,
				      size_t count, loff_t *f_pos)
{
	int ret;
	char args[MAX_ARGS_LEN] = { 0 };
	char *arg1;
	char *arg2;
	char *buffer;
	char *delim = " ";
	u32 slot;
	u32 bootable;

	if (count > MAX_ARGS_LEN) {
		pr_err("count(%zu) out of range\n", count);
		return -EINVAL;
	}

	ret = copy_from_user(args, buf, count);
	if (ret) {
		pr_err("copy args from user failed, ret = %d\n", ret);
		return -EFAULT;
	}

	pr_info("args = %s\n", args);
	buffer = (char *)args;
	arg1 = strsep(&buffer, delim);
	if (!arg1 || (strncmp(arg1, "0", strlen("0")) &&
		strncmp(arg1, "1", strlen("1")))) {
		pr_err("arg1 is invalid\n");
		return -EINVAL;
	}
	arg2 = strsep(&buffer, delim);
	if (!arg2 || (strncmp(arg2, "0", strlen("0")) &&
		strncmp(arg2, "1", strlen("1")))) {
		pr_err("arg1 is invalid\n");
		return -EINVAL;
	}
	ret = kstrtouint(arg1, 10, &slot);
	if (ret) {
		pr_err("arg1 kstrtouint failed, ret = %d\n", ret);
		return -EFAULT;
	}
	ret = kstrtouint(arg2, 10, &bootable);
	if (ret) {
		pr_err("arg2 kstrtouint failed, ret = %d\n", ret);
		return -EFAULT;
	}

	set_slot_bootable(slot, bootable);

	return count;
}

static const struct file_operations ab_slot_bootable_fops = {
	.owner = THIS_MODULE,
	.open = ab_slot_bootable_open,
	.release = ab_slot_bootable_release,
	.read = ab_slot_bootable_read,
	.write = ab_slot_bootable_write,
};

static int __init ab_slot_bootable_ctrl_init(void)
{
	int ret;

	pr_info("ab_slot_bootable_ctrl init");

	bootable_ctrl_reg = ioremap(SYS_CTRL_POR_REG_USEC0, sizeof(u32));
	if (IS_ERR(bootable_ctrl_reg)) {
		pr_err("failed to ioremap bootable_ctrl_reg\n");
		return PTR_ERR(bootable_ctrl_reg);
	}

	ret = alloc_chrdev_region(&bootable_ctrl_dev, DEVICE_MINOR, DEVICE_COUNT, DEVICE_NAME);
	if (ret < 0) {
		pr_err("failed to alloc_chrdev_region\n");
		iounmap(bootable_ctrl_reg);
		return ret;
	}
	cdev_init(&bootable_ctrl_cdev, &ab_slot_bootable_fops);
	bootable_ctrl_cdev.owner = THIS_MODULE;

	ret = cdev_add(&bootable_ctrl_cdev, bootable_ctrl_dev, DEVICE_COUNT);
	if (ret < 0) {
		pr_err("failed to add cdev\n");
		unregister_chrdev_region(bootable_ctrl_dev, DEVICE_COUNT);
		iounmap(bootable_ctrl_reg);
		return ret;
	}

	/* create device class */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0)
	bootable_ctrl_class = class_create(bootable_ctrl_cdev.owner, CLASS_NAME);
#else
	bootable_ctrl_class = class_create(CLASS_NAME);
#endif
	if (IS_ERR(bootable_ctrl_class)) {
		pr_err("failed to create device class\n");
		cdev_del(&bootable_ctrl_cdev);
		unregister_chrdev_region(bootable_ctrl_dev, DEVICE_COUNT);
		iounmap(bootable_ctrl_reg);
		return PTR_ERR(bootable_ctrl_class);
	}

	/* create device node */
	bootable_ctrl_device = device_create(bootable_ctrl_class, NULL,
					     bootable_ctrl_dev, NULL, DEVICE_NAME);
	if (IS_ERR(bootable_ctrl_device)) {
		pr_err("failed to create device node\n");
		class_destroy(bootable_ctrl_class);
		cdev_del(&bootable_ctrl_cdev);
		unregister_chrdev_region(bootable_ctrl_dev, DEVICE_COUNT);
		iounmap(bootable_ctrl_reg);
		return PTR_ERR(bootable_ctrl_device);
	}

	return 0;
}
static void __exit ab_slot_bootable_ctrl_exit(void)
{
	pr_info("ab_slot_bootable_ctrl exit\n");
	device_destroy(bootable_ctrl_class, bootable_ctrl_dev);
	class_destroy(bootable_ctrl_class);
	cdev_del(&bootable_ctrl_cdev);
	unregister_chrdev_region(bootable_ctrl_dev, DEVICE_COUNT);
	iounmap(bootable_ctrl_reg);
}

module_init(ab_slot_bootable_ctrl_init);
module_exit(ab_slot_bootable_ctrl_exit);

MODULE_DESCRIPTION("Xring AB Slot Bootable Control Driver");
MODULE_LICENSE("GPL v2");
