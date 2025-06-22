// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "%s:%s " fmt, KBUILD_MODNAME, __func__

#include <linux/devfreq.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/pm_qos.h>
#include <linux/slab.h>
#include <linux/thermal.h>
#include <linux/workqueue.h>

#define MAX_RETRY_CYCLE 10
#define ONCE_RETRY_DELAY_TIME msecs_to_jiffies(2000)

struct gpufreq_cdev_device {
	int cur_state;
	int max_state;
	int retry_cycle;
	struct device_node *np;
	struct devfreq *gpufreq;
	struct device *dev;
	unsigned long *freq_table;
	struct dev_pm_qos_request req;
	struct delayed_work register_work;
	struct thermal_cooling_device *cdev;
};

static struct gpufreq_cdev_device *gpufreq_cdev_inst;

static int gpufreq_cdev_set_state(struct thermal_cooling_device *cdev,
					unsigned long state)
{
	struct gpufreq_cdev_device *gpufreq_cdev = cdev->devdata;
	unsigned long freq;
	int ret;

	/* Request state should be less than max_level */
	if (state > gpufreq_cdev->max_state)
		return -EINVAL;

	/* Check if the old cooling action is same as new cooling action */
	if (state == gpufreq_cdev->cur_state)
		return 0;

	freq = gpufreq_cdev->freq_table[state];
	pr_err("cdev:%s Limit:%lu\n", cdev->type, freq);

	ret = dev_pm_qos_update_request(&gpufreq_cdev->req, freq);
	if (ret < 0) {
		pr_err("Error placing qos request:%lu. cdev:%s err:%d\n",
				freq, cdev->type, ret);
		return ret;
	}
	gpufreq_cdev->cur_state = state;

	return 0;
}

static int gpufreq_cdev_get_state(struct thermal_cooling_device *cdev,
					unsigned long *state)
{
	struct gpufreq_cdev_device *gpufreq_cdev = cdev->devdata;

	*state = gpufreq_cdev->cur_state;
	return 0;
}

static int gpufreq_cdev_get_max_state(struct thermal_cooling_device *cdev,
					unsigned long *state)
{
	struct gpufreq_cdev_device *gpufreq_cdev = cdev->devdata;

	*state = gpufreq_cdev->max_state;
	return 0;
}

static struct thermal_cooling_device_ops gpufreq_cdev_ops = {
	.set_cur_state = gpufreq_cdev_set_state,
	.get_cur_state = gpufreq_cdev_get_state,
	.get_max_state = gpufreq_cdev_get_max_state,
};

static void gpufreq_cdev_work(struct work_struct *work)
{
	struct devfreq *gpufreq = NULL;
	unsigned long freq = ULONG_MAX;
	unsigned long *freq_table = NULL;
	struct dev_pm_opp *opp = NULL;
	unsigned int opp_count, i;
	int ret;
	struct gpufreq_cdev_device *cdev = container_of(work,
						struct gpufreq_cdev_device,
						register_work.work);

	gpufreq = devfreq_get_devfreq_by_node(cdev->np);
	if (IS_ERR(gpufreq)) {
		pr_debug("gpufreq not available:%ld\n", PTR_ERR(gpufreq));
		if (--cdev->retry_cycle)
			queue_delayed_work(system_highpri_wq,
					&cdev->register_work,
					ONCE_RETRY_DELAY_TIME);
		return;
	}

	/* dev.parent is the device using devfreq */
	cdev->dev = gpufreq->dev.parent;
	cdev->gpufreq = gpufreq;
	opp_count = dev_pm_opp_get_opp_count(cdev->dev);
	freq_table = kcalloc(opp_count, sizeof(*freq_table), GFP_KERNEL);
	if (!freq_table) {
		pr_err("kcalloc gpufreq table err:no-mem!\n");
		return;
	}

	for (i = 0; i < opp_count; i++) {
		opp = dev_pm_opp_find_freq_floor(cdev->dev, &freq);
		if (IS_ERR(opp)) {
			pr_err("find pm opp freq fail, freq %lu, ret %ld!\n",
				freq, PTR_ERR(opp));
			goto exit;
		}
		dev_pm_opp_put(opp);

		freq_table[i] = DIV_ROUND_UP(freq, 1000); //hz to khz
		freq--;
		pr_debug("%u. freq table:%lu\n", i, freq_table[i]);
	}
	cdev->max_state = opp_count-1;
	cdev->freq_table = freq_table;
	ret = dev_pm_qos_add_request(cdev->dev,
				     &cdev->req,
				     DEV_PM_QOS_MAX_FREQUENCY,
				     PM_QOS_MAX_FREQUENCY_DEFAULT_VALUE);
	if (ret < 0) {
		pr_err("pm qos add request err:%d!\n", ret);
		goto exit;
	}
	cdev->cdev = thermal_cooling_device_register("gpu", cdev, &gpufreq_cdev_ops);
	if (IS_ERR(cdev->cdev)) {
		pr_err("Cdev register failed for gpu, ret:%ld\n", PTR_ERR(cdev->cdev));
		cdev->cdev = NULL;
		goto pm_qos_exit;
	}
	pr_debug("[%s] cdev registered.\n", "gpu");
	return;
pm_qos_exit:
	dev_pm_qos_remove_request(&cdev->req);
exit:
	kfree(freq_table);
	cdev->freq_table = NULL;
}

static int gpufreq_cdev_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	gpufreq_cdev_inst = devm_kzalloc(dev, sizeof(*gpufreq_cdev_inst), GFP_KERNEL);
	if (!gpufreq_cdev_inst)
		return -ENOMEM;

	gpufreq_cdev_inst->np = of_parse_phandle(pdev->dev.of_node,
				"xring,gpufreq", 0);
	gpufreq_cdev_inst->retry_cycle = MAX_RETRY_CYCLE;

	INIT_DEFERRABLE_WORK(&gpufreq_cdev_inst->register_work, gpufreq_cdev_work);
	queue_delayed_work(system_highpri_wq, &gpufreq_cdev_inst->register_work, 0);

	return 0;
}

static int gpufreq_cdev_remove(struct platform_device *pdev)
{
	if (gpufreq_cdev_inst->cdev) {
		thermal_cooling_device_unregister(gpufreq_cdev_inst->cdev);
		dev_pm_qos_remove_request(&gpufreq_cdev_inst->req);
		kfree(gpufreq_cdev_inst->freq_table);
		gpufreq_cdev_inst->cdev = NULL;
	}

	return 0;
}

static const struct of_device_id gpufreq_cdev_match[] = {
	{.compatible = "xring,gpufreq-cdev"},
	{}
};

static struct platform_driver gpufreq_cdev_driver = {
	.probe          = gpufreq_cdev_probe,
	.remove         = gpufreq_cdev_remove,
	.driver         = {
		.name   = "xring-gpufreq-cdev",
		.of_match_table = gpufreq_cdev_match,
	},
};

static int __init gpufreq_cdev_init(void)
{
	return platform_driver_register(&gpufreq_cdev_driver);
}
module_init(gpufreq_cdev_init);

static void __exit gpufreq_cdev_exit(void)
{
	platform_driver_unregister(&gpufreq_cdev_driver);
}
module_exit(gpufreq_cdev_exit);

MODULE_DESCRIPTION("GPUFreq cooling driver");
MODULE_LICENSE("GPL");
