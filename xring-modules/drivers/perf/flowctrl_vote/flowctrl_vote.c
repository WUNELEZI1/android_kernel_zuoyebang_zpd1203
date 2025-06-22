// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/units.h>
#include <linux/miscdevice.h>
#include <linux/compiler_attributes.h>
#include <linux/arm-smccc.h>
#include <soc/xring/flowctrl_vote.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

#undef pr_fmt
#define pr_fmt(fmt) "[flowctrl_vote]:%s:%d " fmt, __func__, __LINE__

static bool init_flag;

static int flowctrl_vote_dev_request(unsigned int vote_id, unsigned int vote_bw)
{
	struct arm_smccc_res res = {0xff};

	pr_info("flowctrl_request vote_id:%d, vote_bw:%d\n", vote_id, vote_bw);

	if (!init_flag) {
		pr_err("flowctrl vote dev is not initialized\n");
		return -EINVAL;
	}

	if (vote_id >= VOTE_MAX_MODE) {
		pr_err("flowctrl vote_id=%u is wrong, should be smaller than %u\n",
			vote_id, VOTE_MAX_MODE);
		return -EINVAL;
	}

	arm_smccc_smc(FID_BL31_PERF_FLOWCTRL_VOTE, vote_id, vote_bw, 0, 0, 0, 0, 0, &res);

	return 0;
}

static int flowctrl_vote_dev_open(__maybe_unused struct inode *inode, struct file *file)
{
	union flowctrl_vote_info *vote_data = NULL;

	if (file == NULL) {
		pr_err("ddr vote dev file is NULL\n");
		return -EFAULT;
	}

	vote_data = kzalloc(sizeof(union flowctrl_vote_info), GFP_KERNEL);
	if (vote_data == NULL)
		return -ENOMEM;

	file->private_data = vote_data;

	pr_info("flowctrl vote open success\n");

	return 0;
}

static int flowctrl_vote_dev_release(__maybe_unused struct inode *inode, struct file *file)
{
	union flowctrl_vote_info *vote_data = NULL;

	if (file == NULL) {
		pr_err("ddr vote dev file is NULL\n");
		return -EFAULT;
	}

	vote_data = (union flowctrl_vote_info *)file->private_data;
	if (vote_data == NULL) {
		pr_err("ddr vote private_data is NULL\n");
		return -EFAULT;
	}

	pr_info("flowctrl vote release success\n");

	kfree(vote_data);

	return 0;
}

static long flowctrl_vote_dev_ioctrl(struct file *file, unsigned int cmd, unsigned long data)
{
	union flowctrl_vote_info *vote_data = NULL;

	if (file == NULL) {
		pr_err("flowctrl vote dev file is NULL\n");
		return -EFAULT;
	}

	vote_data = (union flowctrl_vote_info *)file->private_data;
	if (vote_data == NULL) {
		pr_err("flowctrl vote private_data is NULL\n");
		return -EFAULT;
	}

	if (cmd != FLOWCTRL_VOTE_IOCTRL_CMD) {
		pr_err("ddr vote cmd=0x%x is wrong\n", cmd);
		return -EINVAL;
	}
	vote_data->value = data;
	return flowctrl_vote_dev_request(vote_data->data.vote_id, vote_data->data.vote_bw);
}

static const struct file_operations g_flowctrl_vote_dev_fops = {
	.owner = THIS_MODULE,
	.open = flowctrl_vote_dev_open,
	.release = flowctrl_vote_dev_release,
	.unlocked_ioctl = flowctrl_vote_dev_ioctrl,
	.llseek = no_llseek,
};

static struct miscdevice g_flowctrl_vote_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "flowctrl_vote",
	.fops = &g_flowctrl_vote_dev_fops,
};

static int flowctrl_vote_probe(struct platform_device *platform_dev)
{
	struct device *dev = &platform_dev->dev;
	struct device_node *dev_node = dev->of_node;
	int ret;

	if (dev_node == NULL) {
		dev_err(dev, "no device node\n");
		return -ENODEV;
	}

	ret = misc_register(&g_flowctrl_vote_miscdev);
	if (ret < 0) {
		dev_err(dev, "misc register fail\n");
		return ret;
	}

	init_flag = true;
	dev_info(dev, "flowctrl vote probe success\n");

	return 0;
}

static int flowctrl_vote_remove(struct platform_device *platform_dev)
{
	struct device *dev = &platform_dev->dev;

	init_flag = false;
	misc_deregister(&g_flowctrl_vote_miscdev);

	dev_info(dev, "flowctrl vote remove success\n");

	return 0;
}

static const struct of_device_id g_flowctrl_vote_of_match[] = {
	{
		.compatible = "xring,flowctrl_vote",
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_flowctrl_vote_of_match);

static struct platform_driver g_flowctrl_vote_driver = {
	.probe = flowctrl_vote_probe,
	.remove = flowctrl_vote_remove,
	.driver = {
		.name = "flowctrl_vote",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_flowctrl_vote_of_match),
	},
};
module_platform_driver(g_flowctrl_vote_driver);

MODULE_DESCRIPTION("X-RingTek flowctrl_vote driver");
MODULE_LICENSE("GPL v2");
