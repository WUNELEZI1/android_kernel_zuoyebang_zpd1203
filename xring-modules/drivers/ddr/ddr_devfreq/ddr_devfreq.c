// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
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
#include <linux/version.h>
#include <governor.h>
#include <soc/xring/vote_mng.h>
#include <soc/xring/ddr_devfreq.h>
#include <soc/xring/securelib/securec.h>
#include <ddr_soc_define.h>
#include <ddr_dvfs_vote.h>
#include <xctrl_ddr_msg_define.h>
#include "ddr_current_freq_internal.h"

#define FREQ_SHOW_MAX_LEN		100
#define DDR_DEVFREQ_NAME		"ddr_devfreq"
#define BANDWIDTH_DATA_WORD_SIZE	2
#define DDR_BD_PERCENT			100
#define VOTE_SHOW_MAX_LEN		100
#undef pr_fmt
#define pr_fmt(fmt)			DDR_DEVFREQ_NAME ": " fmt

struct ddr_devfreq_data {
	unsigned int bytes_per_sec_per_bps;
	unsigned int bd_utilization;
	unsigned long min_freq;
	unsigned long max_freq;
	struct vote_mng *vote_mng;
	struct vote_mng *vote_mng_dvfs_ok;
	struct vote_mng *vote_mng_freq_duration;
	struct devfreq *devfreq;
	bool is_initialized;
};

static struct ddr_devfreq_data g_ddr_devfreq_data = {
	.min_freq = 0,
	.max_freq = PM_QOS_MAX_FREQUENCY_DEFAULT_VALUE,
	.vote_mng = NULL,
	.vote_mng_dvfs_ok = NULL,
	.devfreq = NULL,
	.is_initialized = false,
};

static const struct of_device_id g_ddr_devfreq_of_match[] = {
	{
		.compatible = "xring,ddr_devfreq",
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_ddr_devfreq_of_match);

int get_dvfs_ok_tout_info(char *buf, unsigned int buf_size)
{
	union kc_msg msg = { .pt_msg.cmd_id = KC_CMD_DVFS_OK_INFO, };
	int ret, cnt;
	unsigned long freq_mbps;

	if (buf == NULL) {
		pr_err("dvfs_ok:buf null\n");
		return -EFAULT;
	}

	if (buf_size < DVFS_OK_BUF_LEN) {
		pr_err("dvfs_ok:buf_size=%u,min_size=%u\n", buf_size, DVFS_OK_BUF_LEN);
		return -EFAULT;
	}

	if (!g_ddr_devfreq_data.is_initialized) {
		pr_err("dvfs_ok:not initialized\n");
		return -EFAULT;
	}

	if (g_ddr_devfreq_data.devfreq == NULL) {
		pr_err("dvfs_ok:devfreq invalid pointer\n");
		return -EINVAL;
	}

	if (g_ddr_devfreq_data.devfreq->freq_table == NULL) {
		pr_err("dvfs_ok:freq_table invalid pointer\n");
		return -EINVAL;
	}

	ret = vote_mng_msg_send(g_ddr_devfreq_data.vote_mng_dvfs_ok,
		(unsigned int *)&msg, VOTE_MNG_MSG_SIZE, VOTE_MNG_MSG_SYNC);
	if (ret != 0) {
		pr_err("dvfs_ok:vote mng send fail:%d\n", ret);
		return -EACCES;
	}

	if (msg.dvfs_ok_msg.freq_id >= g_ddr_devfreq_data.devfreq->max_state) {
		pr_err("dvfs_ok:freq_id=%u is err,max=%u\n",
			msg.dvfs_ok_msg.freq_id, g_ddr_devfreq_data.devfreq->max_state - 1);
		return -EINVAL;
	}

	freq_mbps = g_ddr_devfreq_data.devfreq->freq_table[msg.dvfs_ok_msg.freq_id] / HZ_PER_MHZ;
	cnt = snprintf(buf, VOTE_SHOW_MAX_LEN,
		"isp_cnt=%u,dpu_cnt=%u,last_timestamp=0x%llx,last_isp=%u,last_dpu=%u,last_freq=%lu\n",
		msg.dvfs_ok_msg.isp_dvfs_ok_tout_cnt, msg.dvfs_ok_msg.dpu_dvfs_ok_tout_cnt,
		msg.dvfs_ok_msg.last_tout_timestamp, msg.dvfs_ok_msg.last_isp_dvfs_ok,
		msg.dvfs_ok_msg.last_dpu_dvfs_ok, freq_mbps);

	return cnt;
}
EXPORT_SYMBOL(get_dvfs_ok_tout_info);

static ssize_t dvfs_ok_tout_show(__maybe_unused struct device *dev,
	__maybe_unused struct device_attribute *attr, char *buf)
{
	int cnt;

	cnt = get_dvfs_ok_tout_info(buf, DVFS_OK_BUF_LEN);
	if (cnt > 0)
		dev_err(dev, "%s\n", buf);

	return cnt;
}
static DEVICE_ATTR_RO(dvfs_ok_tout);

static ssize_t ddr_freq_duration_show(__maybe_unused struct device *dev, __maybe_unused struct device_attribute *attr, char *buf)
{
	unsigned int freq_time[SPEC_PROFILE_NUM_MAX] = {0};
	unsigned int freq_msg[VOTE_MNG_MSG_SIZE] = {0};
	struct vote_mng *mng = NULL;
	struct devfreq *devfreq = NULL;
	struct ddr_devfreq_data *ddr_devfreq_data = NULL;
	unsigned long *freq_table = NULL;
	unsigned int freq_num, freq_idx, cpy_size;
	int ret, cnt;

	devfreq = to_devfreq(dev);
	if (devfreq == NULL) {
		dev_err(dev, "freq_dur: devfreq null\n");
		return -EINVAL;
	}

	ddr_devfreq_data = devfreq->data;
	if (ddr_devfreq_data == NULL) {
		dev_err(dev, "freq_dur: ddr_devfreq_data null\n");
		return -EINVAL;
	}

	freq_table = devfreq->freq_table;
	if (freq_table == NULL) {
		dev_err(dev, "freq_dur: freq_table null\n");
		return 0;
	}

	mng = ddr_devfreq_data->vote_mng_freq_duration;
	if (mng == NULL) {
		dev_err(dev, "freq_dur: freq duration mng null\n");
		return -EINVAL;
	}

	freq_num = devfreq->max_state;
	if (freq_num > SPEC_PROFILE_NUM_MAX) {
		dev_err(dev, "freq_dur: freq_num err: %u\n", freq_num);
		return -EINVAL;
	}
	for (freq_idx = 0; freq_idx < freq_num; freq_idx += VOTE_MNG_MSG_SIZE) {
		freq_msg[0] = freq_idx;
		cpy_size = freq_num - freq_idx < VOTE_MNG_MSG_SIZE ? freq_num - freq_idx : VOTE_MNG_MSG_SIZE;
		ret = vote_mng_msg_send(mng, freq_msg, VOTE_MNG_MSG_SIZE, VOTE_MNG_MSG_SYNC);
		if (ret < 0) {
			dev_err(dev, "freq_dur: freq %u send xctrl_ddr fail %d!\n", freq_idx, ret);
			return ret;
		}
		ret = memcpy_s(freq_time + freq_idx, (freq_num - freq_idx) * sizeof(unsigned int),
			freq_msg, cpy_size * sizeof(unsigned int));
		if (ret != EOK) {
			dev_err(dev, "freq_dur: freq %u send xctrl_ddr fail %d!\n", freq_idx, ret);
			return ret;
		}
	}
	cnt = 0;
	for (freq_idx = 0; freq_idx < freq_num; freq_idx++)
		cnt += snprintf_s(buf + cnt, FREQ_SHOW_MAX_LEN, FREQ_SHOW_MAX_LEN, "%lu %u\n", /* unit: 10ms */
			freq_table[freq_idx], freq_time[freq_idx]);
	return cnt;
}
static DEVICE_ATTR_RO(ddr_freq_duration);

static struct attribute *g_ddr_devfreq_attrs[] = {
	&dev_attr_dvfs_ok_tout.attr,
	&dev_attr_ddr_freq_duration.attr,
	NULL,
};

static struct attribute_group g_ddr_devfreq_attr_group = {
	.attrs = g_ddr_devfreq_attrs,
};

static unsigned long calc_vote_value(unsigned long freq_bps)
{
	unsigned long freq_mbps = freq_bps / HZ_PER_MHZ;

	if (freq_mbps > VOTE_MAX_VALUE)
		freq_mbps = VOTE_MAX_VALUE;

	return freq_mbps;
}

static int ddr_devfreq_target(struct device *dev, unsigned long *freq, u32 flags)
{
	struct platform_device *platform_dev = container_of(dev, struct platform_device, dev);
	struct ddr_devfreq_data *ddr_devfreq_data = platform_get_drvdata(platform_dev);
	struct devfreq *ddr_devfreq = NULL;
	unsigned long min_freq, max_freq;
	struct dev_pm_opp *opp = NULL;

	if (ddr_devfreq_data == NULL) {
		dev_err(dev, "devfreq data invalid pointer\n");
		return -EINVAL;
	}

	if (!ddr_devfreq_data->is_initialized)
		return 0;

	ddr_devfreq = ddr_devfreq_data->devfreq;
	if (ddr_devfreq == NULL) {
		dev_err(dev, "devfreq invalid pointer\n");
		return -EINVAL;
	}

	devfreq_get_freq_range(ddr_devfreq, &min_freq, &max_freq);

	if (max_freq != ddr_devfreq_data->max_freq) {
		vote_mng_vote_max(ddr_devfreq_data->vote_mng,
			DDR_DVFS_HW_VOTE_KERNEL, calc_vote_value(max_freq));
		ddr_devfreq_data->max_freq = max_freq;
		dev_info(dev, "vote max %ldbps\n", max_freq);
	}

	if (min_freq != ddr_devfreq_data->min_freq) {
		vote_mng_vote_min(ddr_devfreq_data->vote_mng,
			DDR_DVFS_HW_VOTE_KERNEL, calc_vote_value(min_freq));
		ddr_devfreq_data->min_freq = min_freq;
		dev_info(dev, "vote min %ldbps\n", min_freq);
	}

	*freq = ddr_get_current_freq();
	opp = devfreq_recommended_opp(dev, freq, flags);
	if (IS_ERR(opp)) {
		dev_err(dev, "%s %d, failed to get opp\n", __func__, __LINE__);
		return PTR_ERR(opp);
	}
	dev_pm_opp_put(opp);

	return 0;
}

static int ddr_devfreq_get_dev_status(struct device *dev, struct devfreq_dev_status *stat)
{
	return 0;
}

static int ddr_devfreq_get_cur_freq(struct device *dev, unsigned long *freq)
{
	*freq = ddr_get_current_freq();
	return 0;
}

int ddr_get_cur_bandwidth(unsigned long *bandwidth_mbytes_ps)
{
	unsigned long freq;

	if (bandwidth_mbytes_ps == NULL) {
		pr_err("bandwidth_mbytes_ps is null\n");
		return -EFAULT;
	}

	if (!g_ddr_devfreq_data.is_initialized) {
		pr_err("ddr devfreq is not initialized\n");
		return -EFAULT;
	}

	freq = ddr_get_current_freq();
	*bandwidth_mbytes_ps = freq / HZ_PER_MHZ * g_ddr_devfreq_data.bytes_per_sec_per_bps *
		g_ddr_devfreq_data.bd_utilization / DDR_BD_PERCENT;

	return 0;
}
EXPORT_SYMBOL(ddr_get_cur_bandwidth);

int ddr_get_max_bandwidth(unsigned long *bandwidth_mbytes_ps)
{
	unsigned long freq_mbps;
	struct devfreq *devfreq = NULL;
	unsigned long *freq_table = NULL;

	if (bandwidth_mbytes_ps == NULL) {
		pr_err("bandwidth_mbytes_ps is null\n");
		return -EFAULT;
	}

	if (!g_ddr_devfreq_data.is_initialized) {
		pr_err("ddr devfreq is not initialized\n");
		return -EFAULT;
	}

	devfreq = g_ddr_devfreq_data.devfreq;
	freq_table = devfreq->freq_table;
	freq_mbps = freq_table[devfreq->max_state - 1] / HZ_PER_MHZ;
	*bandwidth_mbytes_ps = freq_mbps * g_ddr_devfreq_data.bytes_per_sec_per_bps *
		g_ddr_devfreq_data.bd_utilization / DDR_BD_PERCENT;

	return 0;
}
EXPORT_SYMBOL(ddr_get_max_bandwidth);

static struct devfreq_dev_profile g_ddr_devfreq_dev_profile = {
	.polling_ms = 0,
	.target = ddr_devfreq_target,
	.get_dev_status = ddr_devfreq_get_dev_status,
	.get_cur_freq = ddr_devfreq_get_cur_freq,
	.freq_table = NULL,
	.max_state = 0,
};

static int ddr_prepare_devfreq_data(struct platform_device *platform_dev,
	struct ddr_devfreq_data *ddr_devfreq_data)
{
	struct device *dev = &platform_dev->dev;
	struct devfreq *ddr_devfreq = NULL;
	struct device_node *dev_node = dev->of_node;
	int ret;

	if (dev_node == NULL) {
		dev_err(dev, "no device node\n");
		return -ENODEV;
	}

	ret = of_property_read_u32_array(dev_node, "bandwidth_data",
		&ddr_devfreq_data->bytes_per_sec_per_bps, BANDWIDTH_DATA_WORD_SIZE);
	if (ret != 0) {
		dev_err(dev, "no bandwidth_data\n");
		return -EINVAL;
	}

	ddr_devfreq_data->vote_mng = vote_mng_get(dev, "vote_mng_ddr_devfreq");
	if (ddr_devfreq_data->vote_mng == NULL) {
		dev_err(dev, "get devfreq vote mng failed\n");
		return -ENOMEM;
	}

	ddr_devfreq_data->vote_mng_freq_duration = vote_mng_get(dev, "vote_mng_ddr_freq_duration");
	if (ddr_devfreq_data->vote_mng_freq_duration == NULL) {
		dev_err(dev, "get freq_duration vote mng failed\n");
		ret = -ENOMEM;
		goto err_vote_mng_freq_duration;
	}

	ddr_devfreq_data->vote_mng_dvfs_ok = vote_mng_get(dev, "vote_mng_dvfs_ok");
	if (ddr_devfreq_data->vote_mng_dvfs_ok == NULL) {
		dev_err(dev, "get dvfs_ok vote mng failed\n");
		ret = -ENOMEM;
		goto err_vote_mng_dvfs_ok;
	}
	if (dev_pm_opp_of_add_table(dev) != 0) {
		ddr_devfreq = NULL;
	} else {
		g_ddr_devfreq_dev_profile.initial_freq = ddr_get_current_freq();
		ddr_devfreq = devm_devfreq_add_device(dev,
			&g_ddr_devfreq_dev_profile, "powersave", (void *)ddr_devfreq_data);
	}
	if (IS_ERR_OR_NULL(ddr_devfreq)) {
		dev_err(dev, "Failed to add table or device\n");
		ret = -ENODEV;
		goto err_ddr_devfreq;
	}
	ddr_devfreq_data->devfreq = ddr_devfreq;
	ddr_devfreq_data->is_initialized = true;
	dev_info(dev, "Ready\n");
	return 0;

err_ddr_devfreq:
	vote_mng_put(dev, ddr_devfreq_data->vote_mng_dvfs_ok);
err_vote_mng_dvfs_ok:
	vote_mng_put(dev, ddr_devfreq_data->vote_mng_freq_duration);
err_vote_mng_freq_duration:
	vote_mng_put(dev, ddr_devfreq_data->vote_mng);
	return ret;
}

static int ddr_devfreq_probe(struct platform_device *platform_dev)
{
	struct ddr_devfreq_data *ddr_devfreq_data = &g_ddr_devfreq_data;
	struct device *dev = &platform_dev->dev;
	int ret;

	ret = ddr_current_freq_remap(dev);
	if (ret != 0) {
		dev_err(dev, "ioremap fail\n");
		return ret;
	}

	platform_set_drvdata(platform_dev, ddr_devfreq_data);
	ret = ddr_prepare_devfreq_data(platform_dev, ddr_devfreq_data);
	if (ret != 0) {
		dev_err(dev, "prepare data fail\n");
		platform_set_drvdata(platform_dev, NULL);
		return ret;
	}

	ret = sysfs_create_group(&ddr_devfreq_data->devfreq->dev.kobj, &g_ddr_devfreq_attr_group);
	if (ret != 0) {
		dev_err(dev, "failed to create sysfs group!\n");
		return ret;
	}

	ret = dev_pm_qos_update_request(&ddr_devfreq_data->devfreq->user_min_freq_req,
		PM_QOS_MAX_FREQUENCY_DEFAULT_VALUE);
	if (ret < 0)
		dev_err(dev, "init min fail\n");
	ret = dev_pm_qos_update_request(&ddr_devfreq_data->devfreq->user_max_freq_req,
		PM_QOS_MAX_FREQUENCY_DEFAULT_VALUE);
	if (ret < 0)
		dev_err(dev, "init max fail\n");
	return 0;
}

static int ddr_devfreq_remove(struct platform_device *platform_dev)
{
	struct ddr_devfreq_data *ddr_devfreq_data = NULL;
	struct device *dev = &platform_dev->dev;

	ddr_devfreq_data = platform_get_drvdata(platform_dev);
	if (ddr_devfreq_data == NULL)
		return -EINVAL;
	if (ddr_devfreq_data->devfreq == NULL)
		return -EINVAL;

	sysfs_remove_group(&ddr_devfreq_data->devfreq->dev.kobj, &g_ddr_devfreq_attr_group);
	vote_mng_put(dev, ddr_devfreq_data->vote_mng_dvfs_ok);
	vote_mng_put(dev, ddr_devfreq_data->vote_mng_freq_duration);
	vote_mng_put(dev, ddr_devfreq_data->vote_mng);
	devfreq_remove_device(ddr_devfreq_data->devfreq);
	platform_set_drvdata(platform_dev, NULL);
	ddr_devfreq_data->devfreq = NULL;
	ddr_devfreq_data->is_initialized = false;

	return 0;
}

static struct platform_driver g_ddr_devfreq_driver = {
	.probe = ddr_devfreq_probe,
	.remove = ddr_devfreq_remove,
	.driver = {
		.name = DDR_DEVFREQ_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_ddr_devfreq_of_match),
	},
};

int ddr_devfreq_init(void)
{
	return platform_driver_register(&g_ddr_devfreq_driver);
}

void ddr_devfreq_exit(void)
{
	platform_driver_unregister(&g_ddr_devfreq_driver);
}
