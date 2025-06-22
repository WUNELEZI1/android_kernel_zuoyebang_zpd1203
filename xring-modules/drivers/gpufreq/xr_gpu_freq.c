// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/errno.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/devfreq.h>
#include <linux/pm_opp.h>
#include <governor.h>
#include <soc/xring/vote_mng.h>
#include <soc/xring/xr_gpufreq.h>
#include "xr_dynamic_governor.h"
#include <soc/xring/atf_shmem.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h>

#include <trace/events/power.h>

#define XR_GPU_CORE_DEFAULT_NUM		16

#define XR_GPUFREQ_VOTEMNG_CH		0U
#define XR_GPUFREQ_DEFAULT_GOVERNOR	"dynamic"
#define XR_GPUFREQ_DEFAULT_POLLING_MS	20U
#define MHZ				1000000U
#define XR_GPUFREQ_MAX_CORE_NUM         0xFFFF
#define XR_GPU_VFREQ_MAX_COUNT 4
#define XR_GPU_MIN_REAL_FREQ_INDEX 3
#define XR_GPU_VFREQ_LEVEL2 2
#define XR_GPU_VFREQ_LEVEL1 1
#define XR_GPU_VFREQ_LEVEL0 0

DEFINE_MUTEX(xr_gpufreq_mutex);

struct xr_gpufreq_vfreq {
	unsigned long vfreq; /* virtual freq */
	unsigned long rfreq; /* real freq */
	u32 core_num;
	struct list_head node;
};

struct xr_gpufreq_boost_freq {
	unsigned long freq_hz;
	struct list_head boost_node;
};

struct xr_gpufreq {
	unsigned long boost_enable; /* enable boost or not */
	unsigned long min_vfreq; /*min virtual freq */
	unsigned long max_vfreq; /*max virtual freq */
	unsigned long boost_freq_num; /* boost freq num */
	unsigned long vfreq_num; /* virtual freq num */
	struct device *dev;
	struct devfreq *df;
	struct vote_mng *vote;
	struct xr_gpufreq_ops *dev_ops;
	struct list_head vfreq_table;
	struct list_head boost_freq_table;
	unsigned long vfreq_table_value[XR_GPU_VFREQ_MAX_COUNT]; /*52M ~ 208M*/
};

static struct xr_gpufreq *g_gpufreq_data;

static int xr_gpufreq_find_real_freq(struct xr_gpufreq *gpufreq_data,
				     unsigned long vfreq,
				     unsigned long *rfreq,
				     u32 *core_num)
{
	struct xr_gpufreq_vfreq *vfreq_node = NULL;

	list_for_each_entry(vfreq_node, &gpufreq_data->vfreq_table, node) {
		if (vfreq_node->vfreq >= vfreq) {
			*rfreq = vfreq_node->rfreq;
			*core_num = vfreq_node->core_num;
			return 0;
		}
	}

	return -ENOMEM;
}

static unsigned long xr_gpufreq_smooth_freq(struct xr_gpufreq *gpufreq_data, unsigned long current_freq, unsigned long target_freq)
{
	unsigned long min_real_freq = gpufreq_data->vfreq_table_value[XR_GPU_MIN_REAL_FREQ_INDEX];

	/* smooth freq only when set freq to vfreq */
	if (target_freq < current_freq && target_freq < min_real_freq) {
		if (current_freq > min_real_freq)
			return min_real_freq;
		else if (current_freq == min_real_freq)
			return gpufreq_data->vfreq_table_value[XR_GPU_VFREQ_LEVEL2];
		else if (current_freq == gpufreq_data->vfreq_table_value[XR_GPU_VFREQ_LEVEL2])
			return gpufreq_data->vfreq_table_value[XR_GPU_VFREQ_LEVEL1];
		else if (current_freq == gpufreq_data->vfreq_table_value[XR_GPU_VFREQ_LEVEL1])
			return gpufreq_data->vfreq_table_value[XR_GPU_VFREQ_LEVEL0];
		pr_err("gpufreq targetfreq can not smaller than XR_GPU_VFREQ_LEVEL0");
	}
	return target_freq;
}

static int _xr_gpufreq_target(struct device *dev,
			      struct xr_gpufreq *gpufreq_data,
			      unsigned long *target_freq,
			      unsigned long *rfreq,
			      u32 flags)
{
	struct dev_pm_opp *opp;
	unsigned long nominal_freq;
	unsigned long real_freq;
	unsigned long old_freq;
	struct xr_gpufreq_ops *ops;
	u32 core_num = XR_GPU_CORE_DEFAULT_NUM;
	bool vfreq_status = false;
	int ret = 0;

	nominal_freq = *target_freq;

	ops = gpufreq_data->dev_ops;
	if (ops && ops->get_vfreq_status)
		vfreq_status = ops->get_vfreq_status(ops->data);
	if (!vfreq_status && nominal_freq <= gpufreq_data->max_vfreq)
		nominal_freq = gpufreq_data->max_vfreq + 1;

	opp = devfreq_recommended_opp(dev, &nominal_freq, 0);
	if (IS_ERR_OR_NULL(opp)) {
		dev_err(dev, "Failed to get opp (%d)\n", PTR_ERR_OR_ZERO(opp));
		return IS_ERR(opp) ? PTR_ERR(opp) : -ENODEV;
	}
	dev_pm_opp_put(opp);

	if (trace_clock_set_rate_enabled())
		trace_clock_set_rate("gpu nominal", nominal_freq, 0);

	nominal_freq = xr_gpufreq_smooth_freq(gpufreq_data, gpufreq_data->df->previous_freq, nominal_freq);
	if (trace_clock_set_rate_enabled())
		trace_clock_set_rate("gpu smooth", nominal_freq, 0);

	/* Only update if there is a change of frequency */
	old_freq = gpufreq_data->df->previous_freq;
	if (old_freq == nominal_freq) {
		*target_freq = nominal_freq;
		*rfreq = nominal_freq;
		if (vfreq_status)
			ret = xr_gpufreq_find_real_freq(gpufreq_data, nominal_freq, rfreq, &core_num);
		return 0;
	}

	real_freq = nominal_freq;

	if (vfreq_status) {
		if (ops && ops->update_coremask) {
			if (real_freq > gpufreq_data->max_vfreq) {
				ops->update_coremask(ops->data, XR_GPUFREQ_MAX_CORE_NUM);
			} else {
				ret = xr_gpufreq_find_real_freq(gpufreq_data, nominal_freq,
								&real_freq, &core_num);
				if (ret != 0)
					return -EINVAL;
				ops->update_coremask(ops->data, core_num);
			}
		}
	}

	*rfreq = real_freq;
	ret = vote_mng_vote_dmd(gpufreq_data->vote, XR_GPUFREQ_VOTEMNG_CH, real_freq / MHZ);
	if (ret != 0) {
		dev_err(dev, "vote gpu freq fail: %d\n", ret);
		return ret;
	}

	if (trace_clock_set_rate_enabled()) {
		trace_clock_set_rate("gpufreq", real_freq, 0);
		trace_clock_set_rate("gpucore", core_num, 0);
	}

	*target_freq = nominal_freq;

	return 0;
}

static int xr_gpufreq_target(struct device *dev,
			     unsigned long *target_freq,
			     u32 flags)
{
	struct xr_gpufreq *gpufreq_data = dev_get_drvdata(dev);
	struct xr_gpufreq_ops *ops;
	unsigned long rfreq;
	int ret;

	if (IS_ERR_OR_NULL(gpufreq_data))
		return 0;

	ret = _xr_gpufreq_target(dev, gpufreq_data, target_freq, &rfreq, flags);
	if (ret != 0)
		return ret;

	mutex_lock(&xr_gpufreq_mutex);
	ops = gpufreq_data->dev_ops;
	if (ops)
		ops->notify_change(ops->data, rfreq);
	mutex_unlock(&xr_gpufreq_mutex);

	return 0;
}

static void xr_gpufreq_suspend(void)
{
	if (g_gpufreq_data)
		devfreq_suspend_device(g_gpufreq_data->df);
}

static void xr_gpufreq_resume(void)
{
	static bool interval_init;
	struct devfreq *df;
	unsigned int polling_ms = XR_GPUFREQ_DEFAULT_POLLING_MS;

	if (g_gpufreq_data) {
		df = g_gpufreq_data->df;
		if (!interval_init) {
			devfreq_update_interval(df, &polling_ms);
			interval_init = true;
		}
		devfreq_resume_device(df);
	}
}

static u64 xr_gpufreq_get_freq(void)
{
	u64 freq_hz = 0;

	mutex_lock(&xr_gpufreq_mutex);
	if (g_gpufreq_data)
		freq_hz = g_gpufreq_data->df->previous_freq;
	mutex_unlock(&xr_gpufreq_mutex);

	return freq_hz;
}

static int xr_gpufreq_set_freq(u64 freq)
{
	unsigned long rfreq;
	int ret = -ENODEV;
	unsigned long target_freq = freq;
	struct xr_gpufreq_ops *ops;

	mutex_lock(&xr_gpufreq_mutex);
	if (g_gpufreq_data) {
		ret = _xr_gpufreq_target(g_gpufreq_data->dev, g_gpufreq_data, &target_freq, &rfreq, 0);
		ops = g_gpufreq_data->dev_ops;
		if (ops && ret == 0)
			ops->notify_change(ops->data, rfreq);
	}
	mutex_unlock(&xr_gpufreq_mutex);

	return ret;
}

int xr_gpufreq_ops_register(struct xr_gpufreq_ops *ops)
{
	int ret = -ENODEV;

	if (!ops) {
		pr_err("gpufreq register:ops is null\n");
		return -ENOMEM;
	}

	mutex_lock(&xr_gpufreq_mutex);
	if (g_gpufreq_data) {
		g_gpufreq_data->dev_ops = ops;
		ops->gpufreq_suspend = xr_gpufreq_suspend;
		ops->gpufreq_resume = xr_gpufreq_resume;
		ops->gpufreq_get_freq = xr_gpufreq_get_freq;
		ops->gpufreq_set_freq = xr_gpufreq_set_freq;
		ret = 0;
	}
	mutex_unlock(&xr_gpufreq_mutex);

	return ret;
}
EXPORT_SYMBOL(xr_gpufreq_ops_register);

void xr_gpufreq_ops_term(struct xr_gpufreq_ops *ops)
{
	if (!ops) {
		pr_err("gpufreq term:ops is null\n");
		return;
	}

	mutex_lock(&xr_gpufreq_mutex);
	if (g_gpufreq_data) {
		if (g_gpufreq_data->dev_ops == ops) {
			g_gpufreq_data->dev_ops = NULL;
			ops->gpufreq_suspend = NULL;
			ops->gpufreq_resume = NULL;
			ops->gpufreq_get_freq = NULL;
			ops->gpufreq_set_freq = NULL;
		}
	}
	mutex_unlock(&xr_gpufreq_mutex);
}
EXPORT_SYMBOL(xr_gpufreq_ops_term);

struct device *xr_get_gpufreq_device(void)
{
	return &(g_gpufreq_data->df->dev);
}
EXPORT_SYMBOL(xr_get_gpufreq_device);

static struct dynamic_gov_data g_devfreq_gov_data = {
	.cl_accel = 0,
	.vfreq_status = 0,
};

static int xr_gpufreq_get_dev_status(struct device *dev,
				     struct devfreq_dev_status *stat)
{
	struct xr_gpufreq *gpufreq_data = dev_get_drvdata(dev);
	struct xr_gpufreq_ops *ops;
	int ret = 0;
	u32 freq_mhz = 0;
	struct dynamic_gov_data *priv_data = &g_devfreq_gov_data;

	if (IS_ERR_OR_NULL(gpufreq_data))
		return 0;

	mutex_lock(&xr_gpufreq_mutex);
	ops = gpufreq_data->dev_ops;
	if (ops) {
		stat->busy_time = ops->get_utilization(ops->data);

		if (ops->get_cl_accel)
			priv_data->cl_accel = ops->get_cl_accel(ops->data);

		if (ops->get_vfreq_status)
			priv_data->vfreq_status = ops->get_vfreq_status(ops->data) > 0 ? 1 : 0;
	} else {
		ret = -ENODEV;
	}
	mutex_unlock(&xr_gpufreq_mutex);

	if (ret != 0) {
		dev_err(dev, "get gpufreq utilization fail: %d\n", ret);
		return ret;
	}

	/* if vfreq status is enabled get previous frequency, otherwise get real frequency */
	if (priv_data->vfreq_status) {
		stat->current_frequency = gpufreq_data->df->previous_freq;
	} else {
		ret = vote_mng_mode2_get_dmd(gpufreq_data->vote, XR_GPUFREQ_VOTEMNG_CH, &freq_mhz);
		if (ret != 0) {
			dev_err(dev, "get gpufreq current fail: %d\n", ret);
			return ret;
		}
		stat->current_frequency = (unsigned long)freq_mhz * MHZ;
	}

	stat->total_time = 100;
	stat->private_data = (void *)priv_data;

	return 0;
}

static struct xr_gpufreq_vfreq *
xr_gpufreq_vfreq_node_create(struct device *dev, struct device_node *np)
{
	u64 vfreq;
	u64 rfreq;
	u32 core_num;
	int ret;
	struct xr_gpufreq_vfreq *vfreq_node = NULL;

	ret = of_property_read_u64(np, "opp-hz", &vfreq);
	if (ret < 0) {
		dev_err(dev, "bad np for regs\n");
		return NULL;
	}

	ret = of_property_read_u64(np, "opp-hz-real", &rfreq);
	if (ret) {
		dev_err(dev, "bad arb_num\n");
		return NULL;
	}

	ret = of_property_read_u32(np, "opp-core-num", &core_num);
	if (ret) {
		dev_err(dev, "bad core_num\n");
		return NULL;
	}

	vfreq_node = devm_kzalloc(dev, sizeof(*vfreq_node), GFP_KERNEL);
	if (!vfreq_node)
		return NULL;
	vfreq_node->vfreq = vfreq;
	vfreq_node->rfreq = rfreq;
	vfreq_node->core_num = core_num;

	return vfreq_node;
}

static void xr_gpufreq_vfreq_table_init(struct device *dev, struct xr_gpufreq *gpufreq_data)
{
	unsigned long max_vfreq = 0;
	unsigned long min_vfreq = ~0;
	struct device_node *child;
	struct xr_gpufreq_vfreq *vfreq_node;
	struct device_node *opp_v2_np = dev_pm_opp_of_get_opp_desc_node(dev);

	for_each_available_child_of_node(opp_v2_np, child) {
		if (!of_find_property(child, "opp-type", NULL))
			continue;

		vfreq_node = xr_gpufreq_vfreq_node_create(dev, child);
		if (!vfreq_node) {
			dev_err(dev, "%s: fail to create vfreq\n", __func__);
			of_node_put(child);
			continue;
		}
		list_add_tail(&vfreq_node->node, &gpufreq_data->vfreq_table);

		if (min_vfreq > vfreq_node->vfreq)
			min_vfreq = vfreq_node->vfreq;

		if (max_vfreq < vfreq_node->vfreq)
			max_vfreq = vfreq_node->vfreq;
	}
	of_node_put(opp_v2_np);

	gpufreq_data->min_vfreq = min_vfreq;
	gpufreq_data->max_vfreq = max_vfreq;
	gpufreq_data->vfreq_num = 0;

	list_for_each_entry(vfreq_node, &gpufreq_data->vfreq_table, node) {
		pr_err("vfreq:%lu, rfreq:%lu, core_num:%d\n",
			vfreq_node->vfreq, vfreq_node->rfreq, vfreq_node->core_num);
		if (gpufreq_data->vfreq_num < XR_GPU_VFREQ_MAX_COUNT) {
			gpufreq_data->vfreq_table_value[gpufreq_data->vfreq_num] = vfreq_node->vfreq;
			gpufreq_data->vfreq_table_value[XR_GPU_MIN_REAL_FREQ_INDEX] = vfreq_node->rfreq;
		}
		gpufreq_data->vfreq_num++;
	}
}

static void xr_gpufreq_boost_freq_table_init(struct device *dev, struct xr_gpufreq *gpufreq_data)
{
	int ret;
	u64 tmp;
	struct xr_gpufreq_boost_freq *boost_freq;
	struct device_node *child;
	struct devfreq *df = gpufreq_data->df;
	struct device_node *opp_v2_np = dev_pm_opp_of_get_opp_desc_node(dev);

	if (!df) {
		pr_err("boost freq table init after devfreq\n");
		return;
	}

	gpufreq_data->boost_freq_num = 0;
	INIT_LIST_HEAD(&gpufreq_data->boost_freq_table);
	for_each_available_child_of_node(opp_v2_np, child) {
		if (!of_property_read_bool(child, "turbo-mode"))
			continue;

		boost_freq = devm_kzalloc(dev, sizeof(struct xr_gpufreq_boost_freq), GFP_KERNEL);
		if (!boost_freq)
			continue;

		ret = of_property_read_u64(child, "opp-hz", &tmp);
		if (ret < 0)
			pr_err("bad opp-hz para\n");
		else
			boost_freq->freq_hz = tmp;

		list_add_tail(&boost_freq->boost_node, &gpufreq_data->boost_freq_table);

		ret = dev_pm_opp_disable(dev, boost_freq->freq_hz);
		if (ret < 0)
			pr_err("boost freq %lu disable fail\n", boost_freq->freq_hz);

		gpufreq_data->boost_freq_num++;
	}
	of_node_put(opp_v2_np);

	mutex_lock(&gpufreq_data->df->lock);
	df->max_state -= gpufreq_data->boost_freq_num;
	mutex_unlock(&gpufreq_data->df->lock);
}

struct devfreq_dev_profile xr_gpufreq_profile = {
	/* it would be abnormal to enable devfreq monitor during initialization */
	.polling_ms     = 0,
	.target         = xr_gpufreq_target,
	.get_dev_status = xr_gpufreq_get_dev_status,
	.is_cooling_device = true,
};

static int xr_gpufreq_initial_freq(struct device *dev, struct vote_mng *vote)
{
	u32 freq_mhz = 0;
	struct dev_pm_opp *opp = NULL;
	unsigned long freq_hz = 0;
	int ret;

	ret = vote_mng_mode2_get_dmd(vote, XR_GPUFREQ_VOTEMNG_CH, &freq_mhz);
	if (ret != 0)
		dev_err(dev, "get gpufreq vote dme fail: %d\n", ret);
	else
		freq_hz = (unsigned long)freq_mhz * MHZ;

	opp = dev_pm_opp_find_freq_ceil(dev, &freq_hz);
	if (IS_ERR(opp)) {
		/* try to find an available frequency */
		freq_hz = 0;
		opp = dev_pm_opp_find_freq_ceil(dev, &freq_hz);
		if (IS_ERR(opp)) {
			dev_err(dev, "no available frequency exist\n");
			return -ENFILE;
		}
	}
	dev_pm_opp_put(opp);

	dev_info(dev, "initial frequency %lu\n", freq_hz);

	xr_gpufreq_profile.initial_freq = freq_hz;

	return 0;
}

static ssize_t gpufreq_usage_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret;
	uint32_t i;
	uint64_t prof_num, opp_cnt;
	uint32_t *freq;
	uint64_t *usage;
	char tmp_buf[PAGE_SIZE];
	u64 size = 0;
	uint32_t cnt = 0;

	ret = get_atf_shm_size(&size);
	if (ret != 0) {
		cnt = snprintf(buf, PAGE_SIZE, "atf_shemem get size fail\n");
		return cnt;
	}

	size = size > PAGE_SIZE ? PAGE_SIZE : size;

	ret = smc_shm_mode(FID_BL31_GPU_USAGE_GET, (char *)tmp_buf, size, TYPE_INOUT, 0);
	if (ret != 0) {
		cnt = snprintf(buf, PAGE_SIZE, "atf_shemem get usage fail\n");
		return cnt;
	}

	prof_num = ((unsigned int *)tmp_buf)[0];
	freq = (unsigned int *)tmp_buf + 1;
	usage = (uint64_t *)((unsigned int *)tmp_buf + 1 + prof_num);

	opp_cnt = dev_pm_opp_get_opp_count(dev->parent) - g_gpufreq_data->vfreq_num;
	prof_num = opp_cnt < prof_num ? opp_cnt : prof_num;
	for (i = 0; i < prof_num; ++i)
		cnt += snprintf(buf + cnt, PAGE_SIZE - cnt, "%u %llu\n", freq[i],
				usage[i] * 100 / LMS_TIMESTAMP_RATE);

	return cnt;
}
static DEVICE_ATTR_RO(gpufreq_usage);

static ssize_t boost_frequencies_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct xr_gpufreq_boost_freq *boost_freq;
	uint32_t cnt = 0;

	list_for_each_entry(boost_freq, &g_gpufreq_data->boost_freq_table, boost_node)
		cnt += snprintf(buf + cnt, PAGE_SIZE - cnt, "%lu ", boost_freq->freq_hz);

	cnt += snprintf(buf + cnt, PAGE_SIZE - cnt, "\n");
	return cnt;
}
static DEVICE_ATTR_RO(boost_frequencies);

static ssize_t boost_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", g_gpufreq_data->boost_enable);
}

static ssize_t boost_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long enable;
	struct xr_gpufreq_boost_freq *boost_freq;
	struct devfreq *df = to_devfreq(dev);

	ret = kstrtoul(buf, 0, &enable);
	if (ret < 0)
		return ret;

	enable = enable > 0 ? 1 : 0;
	mutex_lock(&xr_gpufreq_mutex);
	if (enable == g_gpufreq_data->boost_enable) {
		mutex_unlock(&xr_gpufreq_mutex);
		return count;
	}

	g_gpufreq_data->boost_enable = enable;

	list_for_each_entry(boost_freq, &g_gpufreq_data->boost_freq_table, boost_node) {
		if (!g_gpufreq_data->boost_enable) {
			ret = dev_pm_opp_disable(dev->parent, boost_freq->freq_hz);
			if (ret < 0)
				pr_err("boost freq %lu disable fail\n", boost_freq->freq_hz);
		} else {
			ret = dev_pm_opp_enable(dev->parent, boost_freq->freq_hz);
			if (ret < 0)
				pr_err("boost freq %lu enable fail\n", boost_freq->freq_hz);
		}
	}
	mutex_unlock(&xr_gpufreq_mutex);

	mutex_lock(&g_gpufreq_data->df->lock);

	if (enable)
		df->max_state += g_gpufreq_data->boost_freq_num;
	else
		df->max_state -= g_gpufreq_data->boost_freq_num;

	ret = update_devfreq(df);
	mutex_unlock(&g_gpufreq_data->df->lock);
	if (ret < 0)
		pr_err("update boost freq fail\n");

	return count;
}
static DEVICE_ATTR_RW(boost_enable);

static struct attribute *gpufreq_attrs[] = {
	&(dev_attr_gpufreq_usage.attr),
	&(dev_attr_boost_enable.attr),
	&(dev_attr_boost_frequencies.attr),
	NULL
};

static struct attribute_group g_gpufreq_group = {
	.attrs = gpufreq_attrs,
};

int xr_gpufreq_probe(struct platform_device *plat_dev)
{
	int opp_count;
	int ret;
	struct device *dev = &plat_dev->dev;
	struct xr_gpufreq *gpufreq_data;
	struct devfreq *df;

	gpufreq_data = devm_kzalloc(dev, sizeof(struct xr_gpufreq), GFP_KERNEL);
	if (IS_ERR_OR_NULL(gpufreq_data)) {
		dev_err(dev, "alloc gpufreq data failed\n");
		return -ENOMEM;
	}
	gpufreq_data->dev = dev;

	ret = dev_pm_opp_of_add_table(dev);
	if (ret != 0) {
		dev_err(dev, "Failed get opp table\n");
		return ret;
	}

	opp_count = dev_pm_opp_get_opp_count(dev);
	if (opp_count <= 0) {
		dev_err(dev, "opp is null %d\n", opp_count);
		return -ENOMEM;
	}

	gpufreq_data->vote = vote_mng_get(dev, "gpufreq_vote");
	if (!gpufreq_data->vote) {
		dev_err(dev, "Failed get vote mng\n");
		ret = -ENOMEM;
		goto err_out;
	}

	INIT_LIST_HEAD(&gpufreq_data->vfreq_table);
	xr_gpufreq_vfreq_table_init(dev, gpufreq_data);

	ret = xr_gpufreq_initial_freq(dev, gpufreq_data->vote);
	if (ret != 0) {
		dev_err(dev, "failed to initial freq\n");
		goto err_out;
	}

	df = devm_devfreq_add_device(dev,
				     &xr_gpufreq_profile,
				     XR_GPUFREQ_DEFAULT_GOVERNOR,
				     NULL);
	if (IS_ERR(df)) {
		ret = PTR_ERR(df);
		dev_err(dev, "Fail to add devfreq device(%d)", ret);
		goto err_out;
	}
	gpufreq_data->df = df;

	xr_gpufreq_boost_freq_table_init(dev, gpufreq_data);

	mutex_lock(&xr_gpufreq_mutex);
	g_gpufreq_data = gpufreq_data;
	mutex_unlock(&xr_gpufreq_mutex);

	dev_set_drvdata(&df->dev, gpufreq_data);
	platform_set_drvdata(plat_dev, gpufreq_data);

	ret = sysfs_create_group(&df->dev.kobj, &g_gpufreq_group);
	if (ret < 0)
		pr_err("gpufreq attrs create frail\n");

	return 0;

err_out:
	vote_mng_put(dev, gpufreq_data->vote);
	return ret;
}

static int xr_gpufreq_remove(struct platform_device *plat_dev)
{
	struct device *dev = &plat_dev->dev;
	struct xr_gpufreq *gpufreq_data = NULL;

	sysfs_remove_group(&g_gpufreq_data->df->dev.kobj, &g_gpufreq_group);

	mutex_lock(&xr_gpufreq_mutex);
	g_gpufreq_data = NULL;
	mutex_unlock(&xr_gpufreq_mutex);

	gpufreq_data = platform_get_drvdata(plat_dev);
	if (!gpufreq_data)
		return -EINVAL;

	vote_mng_put(dev, gpufreq_data->vote);

	return 0;
}

static const struct of_device_id g_xr_gpufreq_of_match[] = {
	{
		.compatible = "xring,gpufreq",
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_xr_gpufreq_of_match);

static struct platform_driver g_xr_gpufreq_driver = {
	.probe = xr_gpufreq_probe,
	.remove = xr_gpufreq_remove,
	.driver = {
		.name = "xr_gpufreq",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_xr_gpufreq_of_match),
	},
};
module_platform_driver(g_xr_gpufreq_driver);

MODULE_DESCRIPTION("XRing GPU Devfreq Driver");
MODULE_LICENSE("GPL v2");
