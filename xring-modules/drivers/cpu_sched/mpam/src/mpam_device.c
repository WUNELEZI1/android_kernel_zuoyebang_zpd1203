// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "mpam_partid_map_mgr.h"
#include "mpam_user_interface.h"
#include <linux/of.h>

#define TAG "xring-mpam: "

/* MPAM isn't available until all the MSC have been probed. */
static int s_fw_num_msc;
static u32 s_mpam_num_msc;

static u16 s_mpam_partid_max = 0xffffU;
static bool s_partid_max_published;

bool mpam_partid_check(u16 partid)
{
	bool ret = true;

	if (!s_partid_max_published || partid > s_mpam_partid_max)
		ret = false;

	return ret;
}

int mpam_get_max_partid(void)
{
	return s_partid_max_published ? s_mpam_partid_max : -EPERM;
}

/*
 * Return positive New max partid if input is valid
 * else return negative.
 * THIS FUNCTION NOT SUPPORT PARALLELISM
 */
int mpam_update_max_partid(u16 device_max_partid)
{
	if (s_partid_max_published)
		return -EPERM;

	if (device_max_partid == 0) {
		pr_err(TAG "Input partid_max zero\n");
		return -EINVAL;
	}

	if (device_max_partid < s_mpam_partid_max)
		s_mpam_partid_max = device_max_partid;

	return s_mpam_partid_max;
}

static int mpam_msc_init(struct mpam_msc *msc)
{
	int ret = 0;

	mutex_lock(&msc->lock);
	if (msc->probed) {
		dev_warn(mpam_msc_dev(msc), "MPAM MSC %d has been inited!\n", msc->id);
		goto unlock_out;
	}

	ret = mpam_msc_hw_probe(msc);
	if (ret)
		goto unlock_out;

	ret = mpam_msc_register_irqs(msc);
	if (ret)
		goto unlock_out;

	ret = mpam_msc_sysfs_interface_init(msc);
	if (ret)
		mpam_msc_unregister_irq(msc);

unlock_out:
	mutex_unlock(&msc->lock);
	return ret;
}

int mpam_enable_once(void)
{
	int err = 0;

	s_partid_max_published = true;

	err = mpam_map_mgr_init();
	if (err)
		return err;

	err = mpam_sysfs_interface_init();
	if (err)
		return err;

	return 0;
}

static int platform_mpam_probe(struct platform_device *pdev)
{
	struct mpam_msc *msc = NULL;
	struct resource *res = NULL;
	int ret = 0;

	msc = devm_kzalloc(&pdev->dev, sizeof(*msc), GFP_KERNEL);
	if (IS_ERR_OR_NULL(msc)) {
		dev_err(&pdev->dev, "alloc error, errorcode=%ld\n",
			(long)msc);
		return -ENOMEM;
	}

	msc->id = s_mpam_num_msc++;
	msc->pdev = pdev;

	mutex_init(&msc->lock);
	spin_lock_init(&msc->part_sel_lock);

	msc->mapped_hwpage =
		devm_platform_get_and_ioremap_resource(pdev, 0, &res);
	if (IS_ERR_OR_NULL(msc->mapped_hwpage)) {
		ret = PTR_ERR(msc->mapped_hwpage);
		dev_err(&pdev->dev, "Failed to map MPAM base address\n");
		return ret;
	}

	msc->mapped_hwpage_sz = res->end - res->start;

	ret = mpam_msc_init(msc);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, msc);

	if (s_mpam_num_msc == s_fw_num_msc) {
		ret = mpam_enable_once();
		if (ret)
			pr_err(TAG "MPAM Enable Failed, errcode=%d\n", ret);
	}
	return 0;
}

static int mpam_dt_count_msc(void)
{
	struct device_node *np = NULL;
	int count = 0;

	for_each_compatible_node(np, NULL, "xring,mpam")
		++count;

	return count;
}

static const struct of_device_id g_mpam_of_match[] = {
	{
		.compatible = "xring,mpam",
	},
	{},
};

static struct platform_driver g_mpam_driver = {
	.probe = platform_mpam_probe,
	.driver = { .name = "mpam",
		    .owner = THIS_MODULE,
		    .of_match_table = g_mpam_of_match },
};

static int __init mpam_device_init(void)
{
	int ret = 0;

	s_fw_num_msc = mpam_dt_count_msc();
	if (s_fw_num_msc <= 0) {
		pr_err(TAG "No MSC devices found in firmware\n");
		return -ENODEV;
	}

	ret = mpam_core_function_check();
	if (ret)
		return ret;

	s_mpam_num_msc = 0;

	return platform_driver_register(&g_mpam_driver);
}

module_init(mpam_device_init);

MODULE_AUTHOR("Yimin Lou <louyimin@xiaomi.com>");
MODULE_DESCRIPTION("X-ring DynamIQ Shared Unit MPAM driver");
MODULE_LICENSE("GPL v2");
