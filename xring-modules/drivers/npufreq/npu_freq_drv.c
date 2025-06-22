// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/devfreq.h>
#include <linux/pm_opp.h>
#include <linux/pm_qos.h>
#include <linux/units.h>
#include <soc/xring/vote_mng.h>
#include <governor.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/printk.h>

#ifdef NPU_FREQ_DEBUG
#define npu_freq_debug(fmt, ...)         \
	pr_err("[%s][D]: " fmt, __func__, ##__VA_ARGS__)
#else
#define npu_freq_debug(fmt, ...)
#endif

struct npu_device_data {
	struct devfreq *devfreq;
	struct vote_mng *npu_vote;
	const char *governor_name;
};

#define NPU_DEVFREQ_MAX_VOTE_CH	2
#define HZ_TO_MHZ(freq_hz)	((freq_hz) / 1000000)
#define MHZ_TO_HZ(freq_mhz)	((unsigned long)(freq_mhz) * 1000000)

static const struct of_device_id g_npu_freq_of_match[] = {
	{
		.compatible = "xring,npu_freq",
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_npu_freq_of_match);

static void npu_update_max_freq(struct device *dev, struct npu_device_data *npu_data)
{
	int ret;
	unsigned long min_freq = 0;
	unsigned long max_freq = 0;
	struct devfreq *devfreq = npu_data->devfreq;

	if (devfreq == NULL) {
		dev_err(dev, "npu devfreq data null\n");
		return;
	}

	devfreq_get_freq_range(devfreq, &min_freq, &max_freq);

	ret = vote_mng_vote_max(npu_data->npu_vote,
				NPU_DEVFREQ_MAX_VOTE_CH,
				HZ_TO_MHZ(max_freq));
	if (ret != 0)
		dev_err(dev, "vote npu freq max fail: %d\n", ret);
}

static int npu_set_target_freq(struct device *dev, unsigned long *freq, u32 flags)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct npu_device_data *npu_data = NULL;
	int ret;

	npu_data = platform_get_drvdata(pdev);
	if (npu_data == NULL) {
		dev_err(dev, "invalid pointer %d\n", __LINE__);
		return -EINVAL;
	}

	npu_freq_debug(" to %lu Mhz\n", HZ_TO_MHZ(*freq));

	npu_update_max_freq(dev, npu_data);

	ret = vote_mng_vote_min(npu_data->npu_vote, 0, HZ_TO_MHZ(*freq));
	if (ret != 0)
		dev_err(dev, "vote npu freq fail: %d\n", ret);

	return ret;
}

static int npu_get_cur_freq(struct device *dev, unsigned long *freq)
{
	struct vote_mng_mode2_result res;
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct npu_device_data *npu_data = NULL;

	npu_data = platform_get_drvdata(pdev);
	if (npu_data == NULL) {
		dev_err(dev, "invalid pointer %d\n", __LINE__);
		return -EINVAL;
	}

	if (vote_mng_mode2_result_dump(npu_data->npu_vote, &res) < 0)
		return -EINVAL;

	npu_freq_debug(": min-%d Mhz max-%d Mhz dmd-%d Mhz\n",
			res.min,
			res.max,
			res.dmd);

	*freq = MHZ_TO_HZ(res.max < res.min ? res.max : res.min);

	return 0;
}

static int npu_get_dev_status(struct device *dev, struct devfreq_dev_status *stat)
{
	pr_debug("%s %d\n", __func__, __LINE__);

	return 0;
}

static struct devfreq_dev_profile g_npu_devfreq_profile = {
	.initial_freq   = 0,
	.polling_ms     = 20,
	.target         = npu_set_target_freq,
	.get_dev_status = npu_get_dev_status,
	.get_cur_freq   = npu_get_cur_freq,
	.freq_table     = NULL,
	.max_state      = 0,
	.is_cooling_device = true,
};

static int npu_freq_probe(struct platform_device *plat_dev)
{
	struct device *dev = &plat_dev->dev;
	struct npu_device_data *npu_data = NULL;
	struct devfreq *devfreq = NULL;
	int ret;

	npu_data = kzalloc(sizeof(struct npu_device_data), GFP_KERNEL);
	if (npu_data == NULL)
		return -ENOMEM;

	ret = of_property_read_string(dev->of_node, "governor_name", &npu_data->governor_name);
	if (ret != 0) {
		dev_err(dev, "Failed get npu freq governor name\n");
		goto err_out;
	}

	ret = dev_pm_opp_of_add_table(dev);
	if (ret != 0) {
		dev_err(dev, "Failed get npu freq table\n");
		goto err_out;
	}

	npu_data->npu_vote = vote_mng_get(dev, "npu_freq_vote");
	if (!npu_data->npu_vote) {
		dev_err(dev, "Failed get npu vote mng\n");
		goto err_out;
	}

	devfreq = devm_devfreq_add_device(dev, &g_npu_devfreq_profile,
					  npu_data->governor_name, NULL);
	if (IS_ERR_OR_NULL(devfreq)) {
		dev_err(dev, "Failed to init npu devfreq\n");
		dev_pm_opp_of_remove_table(dev);
		goto err_out;
	}
	npu_data->devfreq = devfreq;

	platform_set_drvdata(plat_dev, npu_data);

	return 0;

err_out:
	kfree(npu_data);
	return -ENODEV;
}

static int npu_freq_remove(struct platform_device *plat_dev)
{
	struct device *dev = &plat_dev->dev;
	struct npu_device_data *npu_data = NULL;

	dev_pm_opp_of_remove_table(dev);

	npu_data = platform_get_drvdata(plat_dev);
	if (npu_data == NULL)
		return -EINVAL;

	devm_devfreq_remove_device(dev, npu_data->devfreq);
	platform_set_drvdata(plat_dev, NULL);
	vote_mng_put(dev, npu_data->npu_vote);

	kfree(npu_data);

	return 0;
}

static struct platform_driver g_npu_freq_driver = {
	.probe = npu_freq_probe,
	.remove = npu_freq_remove,
	.driver = {
		.name = "npu_freq",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_npu_freq_of_match),
	},
};
module_platform_driver(g_npu_freq_driver);

MODULE_SOFTDEP("pre: npu_freq_qos");
MODULE_AUTHOR("Hao Wang <wanghao69@xiaomi.com>");
MODULE_DESCRIPTION("XRing NPU Freq Driver");
MODULE_LICENSE("GPL v2");
