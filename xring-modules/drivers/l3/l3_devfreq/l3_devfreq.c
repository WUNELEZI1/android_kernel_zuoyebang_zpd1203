// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/devfreq.h>
#include <linux/pm_opp.h>
#include <linux/units.h>
#include <linux/arm-smccc.h>
#include <governor.h>
#include <soc/xring/vote_mng.h>
#include <soc/xring/atf_shmem.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h>
#define CREATE_TRACE_POINTS
#include "l3_devfreq_trace.h"

#define VOTE_MAX_VALUE 0xFFFF
#define CLUSTER_L3 4

struct l3_dev_data {
	struct devfreq *devfreq;
	struct devfreq_dev_profile dev_profile;
	struct vote_mng *vote_mng;
	u32 dmd_channel;
	u32 max_channel;
	unsigned long last_dmd_freq;
	unsigned long last_max_freq;
};

enum {
	DMDVOTE = 0,
	MAXVOTE,
	VOTE_MNG_END,
};

static u32 g_vote_mng_regs[VOTE_MNG_END];

static void vote_mng_reg_cfg(struct l3_dev_data *l3_dev_data,
			     u32 addr, unsigned long freq_hz)
{
	struct vote_mng *vote_mng = l3_dev_data->vote_mng;
	u32 freq_mhz;

	freq_mhz = (u32)(freq_hz / HZ_PER_MHZ);
	if (freq_mhz > VOTE_MAX_VALUE)
		freq_mhz = VOTE_MAX_VALUE;

	g_vote_mng_regs[addr] = freq_mhz;
	if (addr == DMDVOTE)
		vote_mng_vote_dmd(vote_mng, l3_dev_data->dmd_channel, freq_mhz);
	else
		vote_mng_vote_max(vote_mng, l3_dev_data->max_channel, freq_mhz);
}

static void vote_mng_init(struct device *dev, struct l3_dev_data *l3_dev_data)
{
	unsigned long max_freq;

	dev_pm_opp_find_freq_ceil(dev, &max_freq);
	vote_mng_reg_cfg(l3_dev_data, DMDVOTE, max_freq);
	vote_mng_reg_cfg(l3_dev_data, MAXVOTE, max_freq);
}

static ssize_t range_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u-%u\n", g_vote_mng_regs[DMDVOTE], g_vote_mng_regs[MAXVOTE]);
}
static DEVICE_ATTR_RO(range);

static unsigned long l3_get_current_freq(struct device *dev)
{
	unsigned long freq_khz;
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_CPU_PROFILE_GET, CLUSTER_L3, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0) {
		dev_err(dev, "Failed to read L3 actual freq.\n");
		return 0;
	}
	freq_khz = (unsigned long)res.a1;
	return freq_khz * HZ_PER_KHZ;
}

static ssize_t actual_freq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned long freq;

	freq = l3_get_current_freq(dev);
	return sprintf(buf, "%lu\n", freq);
}
static DEVICE_ATTR_RO(actual_freq);

#define TEN_MS_PER_S 100

static ssize_t stat_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u32 prof_num, i;
	u32 *freq;
	u64 shm_size = 0;
	u64 *usage;
	ssize_t len = 0;
	char *kernel_buffer = NULL;
	int ret;

	ret = get_atf_shm_size(&shm_size);
	if (ret != 0) {
		dev_err(dev, "Failed in get_atf_shm_size\n");
		return -EINVAL;
	}

	shm_size = min(shm_size, PAGE_SIZE);
	if (shm_size == 0) {
		dev_err(dev, "Fail: shm_size == 0\n");
		return -EINVAL;
	}

	kernel_buffer = kzalloc(shm_size, GFP_KERNEL);
	if (!kernel_buffer)
		return -ENOMEM;

	ret = smc_shm_mode(
		FID_BL31_CPU_USAGE_GET, kernel_buffer, shm_size, TYPE_OUT, CLUSTER_L3);
	if (ret != 0) {
		dev_err(dev, "Failed in smc_shm_mode\n");
		kfree(kernel_buffer);
		return -EPERM;
	}

	prof_num = ((unsigned int *)kernel_buffer)[0];
	freq = (unsigned int *)kernel_buffer + 1;
	usage = (u64 *)((unsigned int *)kernel_buffer + 1 + prof_num);

	for (i = 0; i < prof_num; ++i)
		len += scnprintf(buf + len, PAGE_SIZE - len, "%u %llu\n",  /* unit: kHz 10ms */
				 freq[i], usage[i] * TEN_MS_PER_S / LMS_TIMESTAMP_RATE);

	kfree(kernel_buffer);
	return len;
}
static DEVICE_ATTR_RO(stat);

static struct attribute *g_l3_attrs[] = {
	&dev_attr_range.attr,		/* RO */
	&dev_attr_actual_freq.attr,	/* RO */
	&dev_attr_stat.attr,		/* RO */
	NULL,
};

static struct attribute_group g_l3_attr_group = {
	.attrs = g_l3_attrs,
};

static int l3_devfreq_target(struct device *dev, unsigned long *freq, u32 flags)
{
	struct platform_device *platform_dev = container_of(dev, struct platform_device, dev);
	struct l3_dev_data *l3_dev_data = platform_get_drvdata(platform_dev);
	struct dev_pm_opp *opp = NULL;
	struct dev_pm_opp *max_opp = NULL;
	unsigned long max_freq;

	if (l3_dev_data == NULL) {
		dev_err(dev, "%s %d, invalid pointer\n", __func__, __LINE__);
		return -EINVAL;
	}

	max_freq = dev_pm_qos_read_value(dev, DEV_PM_QOS_MAX_FREQUENCY);
	if (max_freq != PM_QOS_MAX_FREQUENCY_DEFAULT_VALUE)
		max_freq *= HZ_PER_KHZ;

	opp = devfreq_recommended_opp(dev, freq, flags);
	if (IS_ERR(opp)) {
		dev_err(dev, "Failed to get Operating Point\n");
		return PTR_ERR(opp);
	}
	dev_pm_opp_put(opp);
	max_opp = devfreq_recommended_opp(dev, &max_freq, DEVFREQ_FLAG_LEAST_UPPER_BOUND);
	if (IS_ERR(max_opp)) {
		dev_err(dev, "Failed to get max Operating Point\n");
		return PTR_ERR(max_opp);
	}
	dev_pm_opp_put(max_opp);
	*freq = min(*freq, max_freq);

	if (*freq != l3_dev_data->last_dmd_freq) {
		dev_dbg(dev, "set down threshold:%lu\n", *freq);
		vote_mng_reg_cfg(l3_dev_data, DMDVOTE, *freq);
		l3_dev_data->last_dmd_freq = *freq;
	}

	if (max_freq != l3_dev_data->last_max_freq) {
		dev_dbg(dev, "set up threshold:%lu\n", max_freq);
		vote_mng_reg_cfg(l3_dev_data, MAXVOTE, max_freq);
		l3_dev_data->last_max_freq = max_freq;
	}

	trace_l3_devfreq_range(*freq, max_freq);

	return 0;
}

static int l3_devfreq_get_dev_status(struct device *dev, struct devfreq_dev_status *stat)
{
	return 0;
}

static int l3_devfreq_probe(struct platform_device *platform_dev)
{
	struct device *dev = &platform_dev->dev;
	struct device_node *np = dev->of_node;
	struct devfreq *devfreq = NULL;
	struct l3_dev_data *l3_dev_data = NULL;
	struct devfreq_dev_profile *dev_profile = NULL;
	int ret;

	l3_dev_data = devm_kzalloc(dev, sizeof(struct l3_dev_data), GFP_KERNEL);

	dev_profile = &l3_dev_data->dev_profile;
	dev_profile->target = l3_devfreq_target;
	dev_profile->get_dev_status = l3_devfreq_get_dev_status;

	l3_dev_data->vote_mng = vote_mng_get(dev, NULL);
	if (IS_ERR_OR_NULL(l3_dev_data->vote_mng)) {
		dev_err(dev, "get vote mng failed\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "dmd-channel", &l3_dev_data->dmd_channel);
	if (ret) {
		dev_err(dev, "bad dmd_channel\n");
		goto err_remove_vote_mng;
	}
	ret = of_property_read_u32(np, "max-channel", &l3_dev_data->max_channel);
	if (ret) {
		dev_err(dev, "bad max_channel\n");
		goto err_remove_vote_mng;
	}

	if (dev_pm_opp_of_add_table(dev) != 0) {
		dev_err(dev, "Failed to add opp table\n");
		goto err_remove_vote_mng;
	}

	vote_mng_init(dev, l3_dev_data);
	dev_profile->initial_freq = l3_get_current_freq(dev);
	platform_set_drvdata(platform_dev, l3_dev_data);
	devfreq = devm_devfreq_add_device(dev, dev_profile, "performance", NULL);
	if (IS_ERR_OR_NULL(devfreq)) {
		dev_err(dev, "Failed to init l3 devfreq\n");
		dev_pm_opp_of_remove_table(dev);
		goto err_remove_vote_mng;
	}

	l3_dev_data->devfreq = devfreq;

	ret = sysfs_create_group(&devfreq->dev.kobj, &g_l3_attr_group);
	if (ret) {
		dev_err(dev, "Failed to create sysfs group!\n");
		return ret;
	}

	return 0;

err_remove_vote_mng:
	vote_mng_put(dev, l3_dev_data->vote_mng);
	l3_dev_data->vote_mng = NULL;
	return -ENODEV;
}

static const struct of_device_id g_l3_devfreq_of_match[] = {
	{
		.compatible = "xring,l3_devfreq",
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_l3_devfreq_of_match);

static struct platform_driver g_l3_devfreq_driver = {
	.probe = l3_devfreq_probe,
	.driver = {
		.name = "l3_devfreq",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_l3_devfreq_of_match),
	},
};

static int __init l3_devfreq_init(void)
{
	return platform_driver_register(&g_l3_devfreq_driver);
}
module_init(l3_devfreq_init);

MODULE_AUTHOR("Aoyang Yan <yanaoyang@xiaomi.com>");
MODULE_DESCRIPTION("XRing L3 Devfreq Driver");
MODULE_LICENSE("GPL v2");
