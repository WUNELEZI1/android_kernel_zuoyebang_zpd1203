// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/devfreq.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/of.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/version.h>
#include <linux/pm_qos.h>
#include <linux/jiffies.h>
#include <governor.h>
#include <soc/xring/securelib/securec.h>
#include "xr_dynamic_governor.h"
#include <soc/xring/xr_timestamp.h>

#include <trace/events/power.h>

#define DEFAULT_HISPEED_LOAD		90
#define DEFAULT_HISPEED_FREQ		533000000
#define DEFAULT_LOADING_WINDOW		10
#define DEFAULT_CL_ACCEL_FREQ		672000000UL
#define DEFAULT_TARGET_LOAD		85
#define DEFAULT_NTARGET_LOAD		1

#define LOAD_WIN_MAX			50
#define POLICY_INDEX_BUF_MAX		10
#define NTARGET_LOAD_MAX		40
#define POLICY_BUF_MAX			1024
#define TOKEN_MAX			2
#define POLICY_INDEX_MAX		20

#define PERCENT_UP_LIMIT		100
#define PERCENT_DN_LIMIT		0

#define STR_ENABLE		"enable"
#define STR_DISABLE		"disable"
#define DYNAMIC_DIR_NAME	"dynamic"

#define HIGH_BOOST_FREQ		"high boost freq"
#define MED_BOOST_FREQ		"med boost freq"
#define LOW_BOOST_FREQ		"low boost freq"
#define BOOST_DISABLE		"boost disable"

enum {
	POLICY_INDEX,
	HISPEED_LOAD,
	HISPEED_FREQ,
	LOAD_WIN,
	MAX_PARA
};

const char *policy_para_name[MAX_PARA] = {
	[POLICY_INDEX] = "policy   index",
	[HISPEED_LOAD] = "hispeed   load",
	[HISPEED_FREQ] = "hispeed   freq",
	[LOAD_WIN]     = "load    window"
};

struct dynamic_policy {
	unsigned long para[MAX_PARA];
	unsigned long *target_load;
	unsigned int ntarget_load;
	struct list_head node;
};

struct dynamic_data {
	unsigned long buf[LOAD_WIN_MAX];
	unsigned long buf_load[LOAD_WIN_MAX];
	unsigned long util;
	unsigned long normalized_util;
	unsigned long freq_max;
	unsigned long user_freq;
	unsigned long cl_low_accel_freq;
	unsigned long cl_med_accel_freq;
	unsigned long cl_high_accel_freq;
	unsigned int window_cnt;
	unsigned int window_idx;
	unsigned int cl_accel;
	unsigned int hispeed_clear_enable;
	u64 last_jiff;
	u64 util_sum;
	u64 jiff_sum;
	u64 buf_poling_time[LOAD_WIN_MAX];
	struct list_head policy_list;
	struct dynamic_policy *cur_policy;
};

#define SHOW_FUNC(object)					\
static ssize_t show_##object					\
(struct device *dev, struct device_attribute *attr __maybe_unused, char *buf) \
{								\
	struct devfreq *df = to_devfreq(dev);			\
	struct dynamic_data *data = NULL;			\
	int ret;						\
	mutex_lock(&df->lock);					\
	data = df->data;					\
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,		\
			 "%llu\n", (unsigned long long)data->object);	\
	mutex_unlock(&df->lock);				\
	return ret;						\
}

#define DYNAMIC_ATTR_RW(_name) \
	DEVICE_ATTR(_name, 0664, show_##_name, store_##_name)

#define DYNAMIC_ATTR_RO(_name) \
	DEVICE_ATTR(_name, 0444, show_##_name, NULL)

SHOW_FUNC(util)
SHOW_FUNC(normalized_util)
SHOW_FUNC(user_freq)
SHOW_FUNC(window_cnt)
SHOW_FUNC(jiff_sum)
SHOW_FUNC(util_sum)
SHOW_FUNC(hispeed_clear_enable)

static void devfreq_dynamic_freq_limit(struct devfreq *df, unsigned long *freq)
{
	struct dynamic_data *data = NULL;

	data = df->data;
	if (data == NULL)
		return;

	if (data->cl_accel != 0) {
		if (data->cl_accel == 3 && *freq < data->cl_low_accel_freq)
			*freq = data->cl_low_accel_freq;
		else if (data->cl_accel == 2 && *freq < data->cl_med_accel_freq)
			*freq = data->cl_med_accel_freq;
		else if (data->cl_accel == 1 && *freq < data->cl_high_accel_freq)
			*freq = data->cl_high_accel_freq;
	}

	if (data->user_freq != 0)
		*freq = data->user_freq;
}

static int devfreq_dynamic_func(struct devfreq *df, unsigned long *freq)
{
	u64 cur_jiff, jiff_diff;
	unsigned int i;
	unsigned long targetload;
	unsigned long util;
	unsigned long a;
	int err;
	struct devfreq_dev_status *stat;
	struct dynamic_data *data = NULL;
	struct dynamic_gov_data *priv_data = NULL;

	data = df->data;
	if (data == NULL)
		return -EINVAL;

	err = devfreq_update_stats(df);
	if (err)
		return err;

	stat = &df->last_status;

	priv_data = stat->private_data;

	if (priv_data == NULL)
		return -EINVAL;

	*freq = stat->current_frequency;

	if (unlikely(stat->total_time == 0 || *freq == 0)) {
		*freq = data->cur_policy->para[HISPEED_FREQ];
		goto check_barrier;
	}

	util = stat->busy_time * 100 / stat->total_time;

	trace_clock_set_rate("gpu cur frame load", util, 0);

	cur_jiff = xr_timestamp_count2ns(xr_timestamp_getcount()) / 1000;
	if (data->last_jiff == 0)
		jiff_diff = df->profile->polling_ms;
	else
		jiff_diff = cur_jiff - data->last_jiff;
	data->last_jiff = cur_jiff;

	data->util_sum += (*freq) * util * jiff_diff;
	data->jiff_sum += jiff_diff;
	data->cl_accel = priv_data->cl_accel;

	if (data->window_cnt >= data->cur_policy->para[LOAD_WIN]) {
		data->util_sum -= data->buf[data->window_idx] * data->buf_poling_time[data->window_idx];
		data->jiff_sum -= data->buf_poling_time[data->window_idx];
	} else {
		data->window_cnt++;
	}

	data->buf[data->window_idx] = (*freq) * util;
	data->buf_poling_time[data->window_idx] = jiff_diff;
	data->buf_load[data->window_idx] = util;

	data->window_idx = (data->window_idx + 1) % data->cur_policy->para[LOAD_WIN];
	a = div_u64(data->util_sum, data->jiff_sum);
	data->util = div64_u64(a, *freq);

	trace_clock_set_rate("gpu avg load_freq", a, 0);
	trace_clock_set_rate("gpu avg util", data->util, 0);

	if (data->freq_max != 0)
		data->normalized_util = div64_u64(a, data->freq_max);

	if (data->user_freq > 0)
		goto check_barrier;


	for (i = 0; i < data->cur_policy->ntarget_load - 1 &&
		*freq >= data->cur_policy->target_load[i + 1]; i += 2)
		;

	targetload = data->cur_policy->target_load[i];

	*freq = div_u64(a, targetload);
	trace_clock_set_rate("gpu tar load", targetload, 0);

	targetload = data->cur_policy->para[HISPEED_LOAD];
	if (util > targetload && data->cur_policy->para[HISPEED_FREQ] > *freq) {
		*freq = data->cur_policy->para[HISPEED_FREQ];
		/* clear history load */
		if (data->hispeed_clear_enable) {
			/* clear history load */
			data->jiff_sum = jiff_diff;
			data->util_sum = util * stat->current_frequency * jiff_diff;
			data->window_cnt = 1;
			a = div_u64(data->util_sum, data->jiff_sum);
			data->util = div64_u64(a, *freq);

			if (data->freq_max != 0)
				data->normalized_util = div64_u64(a, data->freq_max);
		}
	}

check_barrier:
	devfreq_dynamic_freq_limit(df, freq);

	return 0;
}

static void dynamic_refresh_load(struct devfreq *df,
				struct dynamic_policy *new_policy)
{
	unsigned long util_avg;
	unsigned int i;
	struct dynamic_data *data = df->data;

	if (data->cur_policy->para[LOAD_WIN] == new_policy->para[LOAD_WIN] ||
	    data->window_cnt == 0)
		return;

	util_avg = div_u64(data->util_sum, data->jiff_sum);
	if (data->window_cnt >= new_policy->para[LOAD_WIN]) {
		data->jiff_sum = data->jiff_sum / data->window_cnt * new_policy->para[LOAD_WIN];
		data->util_sum = util_avg * data->jiff_sum;
		data->window_cnt = new_policy->para[LOAD_WIN];
	}

	for (i = 0; i < new_policy->para[LOAD_WIN]; i++)
		data->buf[i] = util_avg;

	data->window_idx = 0;
}

static int policy_para_check(struct dynamic_policy *policy)
{
	unsigned int i;

	if (policy->para[LOAD_WIN] == 0 ||
	    policy->para[LOAD_WIN] > LOAD_WIN_MAX ||
	    policy->para[HISPEED_LOAD] == PERCENT_DN_LIMIT ||
	    policy->para[HISPEED_LOAD] > PERCENT_UP_LIMIT ||
	    policy->para[POLICY_INDEX] >= POLICY_INDEX_MAX)
		return -EINVAL;

#ifdef CONFIG_XR_DYNAMIC_GOV_DEBUG
	pr_err("%lu %lu %lu %lu\n",
		policy->para[LOAD_WIN],
		policy->para[HISPEED_LOAD],
		policy->para[HISPEED_FREQ],
		policy->para[POLICY_INDEX]
		);

	pr_err("ntarget_load %u\n", policy->ntarget_load);
	for (i = 0; i < policy->ntarget_load; i++)
		pr_err("target_load - %lu\n", policy->target_load[i]);
#endif

	for (i = 0; i < policy->ntarget_load; i += 2) {
		if (policy->target_load[i] > PERCENT_UP_LIMIT ||
		    policy->target_load[i] == PERCENT_DN_LIMIT)
			return -EINVAL;
	}

	return 0;
}

static int policy_sub_para_parse(const char *buf,
				 unsigned long *para,
				 unsigned int ntokens)
{
	const char *cp = buf;
	unsigned int i = 0;

	while (i < ntokens) {
		if (sscanf_s(cp, "%lu", &para[i++]) != 1)
			return -EINVAL;

		cp = strpbrk(cp, ":");
		if (cp == NULL)
			break;
		cp++;
	}

	return 0;
}

static inline bool policy_tokens_invalid(const unsigned int *ntokens_sub)
{
	return ((ntokens_sub[1] & 0x1) == 0 ||
		ntokens_sub[1] > NTARGET_LOAD_MAX ||
		ntokens_sub[0] != MAX_PARA);
}

static struct dynamic_policy *policy_create(const char *buf)
{
	int i;
	int ntokens = 1;
	const char *cp_sub[TOKEN_MAX] = {NULL};
	const char *cp = buf;
	unsigned int ntokens_sub[TOKEN_MAX] = { 1, 1 }; /* [0]:para_num, [1]:load num */
	struct dynamic_policy *policy = NULL;

	cp_sub[0] = buf;
	do {
		cp = strpbrk(cp, ",");
		if (!cp)
			break;
		cp_sub[ntokens] = ++cp;
		ntokens++;
	} while (ntokens < TOKEN_MAX);

	if (ntokens < TOKEN_MAX) {
		pr_err("ntokens not enough\n");
		goto err_parse;
	}

	for (i = 0; i < ntokens; i++) {
		cp = cp_sub[i];
		do {
			cp = strpbrk(cp + 1, ":");
			if (!cp)
				break;
			if (i != (ntokens - 1) && cp > cp_sub[i + 1])
				break;
			ntokens_sub[i]++;
		} while (cp);
	}

	if (policy_tokens_invalid(ntokens_sub)) {
		pr_err("sub ntokens err:%u,%u\n", ntokens_sub[0], ntokens_sub[1]);
		goto err_parse;
	}

	policy = kzalloc(sizeof(struct dynamic_policy), GFP_KERNEL);
	if (!policy)
		goto err_parse;

	if (policy_sub_para_parse(cp_sub[0], policy->para, ntokens_sub[0]) != 0) {
		pr_err("%s extract policy para fail\n", __func__);
		goto err_policy;
	}

	policy->ntarget_load = ntokens_sub[1];
	policy->target_load = kcalloc(ntokens_sub[1], sizeof(*policy->target_load), GFP_KERNEL);
	if (IS_ERR_OR_NULL(policy->target_load)) {
		pr_err("alloc target load fail\n");
		goto err_policy;
	}

	if (policy_sub_para_parse(cp_sub[1], policy->target_load, policy->ntarget_load) != 0) {
		pr_err("%s extract target load fail\n", __func__);
		goto err_target_load;
	}

	if (policy_para_check(policy) != 0) {
		pr_err("%s para check error\n", __func__);
		goto err_target_load;
	}

	return policy;

err_target_load:
	kfree(policy->target_load);
err_policy:
	kfree(policy);
err_parse:
	return NULL;
}

static void policy_destroy(struct dynamic_policy *policy)
{
	list_del(&policy->node);

	kfree(policy->target_load);

	kfree(policy);
}


static int policy_parse(struct devfreq *df,
			const char *buf,
			size_t count)
{
	int ret;
	struct dynamic_data *data = NULL;
	struct dynamic_policy *new_policy = NULL;
	struct dynamic_policy *policy = NULL;
	char local_buf[POLICY_BUF_MAX + 1] = {0};

	if (count >= POLICY_BUF_MAX)
		return -EINVAL;

	ret = strncpy_s(local_buf, POLICY_BUF_MAX, buf, count);
	if (ret != EOK)
		return -EINVAL;

	mutex_lock(&df->lock);
	data = df->data;
	new_policy = policy_create(local_buf);
	if (!new_policy) {
		pr_err("%s get policy fail\n", __func__);
		mutex_unlock(&df->lock);
		return -EINVAL;
	}

	list_for_each_entry(policy, &data->policy_list, node) {
		if (policy->para[POLICY_INDEX] == new_policy->para[POLICY_INDEX]) {
			if (data->cur_policy == policy) {
				dynamic_refresh_load(df, new_policy);
				data->cur_policy = new_policy;
			}

			/* free old policy */
			policy_destroy(policy);
			break;
		}
	}

	list_add_tail(&new_policy->node, &data->policy_list);
	mutex_unlock(&df->lock);

	return 0;
}

static ssize_t show_policy(struct device *dev,
			   struct device_attribute *attr __maybe_unused,
			   char *buf)
{
	int ret;
	unsigned int i;
	ssize_t count = 0;
	char *index_name = NULL;
	struct devfreq *df = to_devfreq(dev);
	struct dynamic_data *data = df->data;
	struct dynamic_policy *policy = NULL;

	mutex_lock(&df->lock);

	list_for_each_entry(policy, &data->policy_list, node) {
		index_name = (policy == data->cur_policy) ? "->" : "  ";
		for (i = 0; i < MAX_PARA; i++) {
			ret = snprintf_s(buf + count, PAGE_SIZE - count,
					 PAGE_SIZE - count - 1, "  %s:  %lu\n",
					 i == POLICY_INDEX ? index_name : policy_para_name[i],
					 policy->para[i]);
			if (ret < 0)
				goto err_ret;

			count += ret;
			if ((unsigned int)count >= PAGE_SIZE)
				goto err_ret;
		}

		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "  target load:   ");
		if (ret < 0)
			goto err_ret;

		count += ret;
		if ((unsigned int)count >= PAGE_SIZE)
			goto err_ret;

		if (IS_ERR_OR_NULL(policy->target_load))
			continue;

		for (i = 0; i < policy->ntarget_load - 1; i++) {
			ret = snprintf_s(buf + count, PAGE_SIZE - count,
					 PAGE_SIZE - count - 1, "%lu:",
					 policy->target_load[i]);
			if (ret < 0)
				goto err_ret;

			count += ret;
			if ((unsigned int)count >= PAGE_SIZE)
				goto err_ret;
		}
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1, "%lu\n",
				 policy->target_load[i]);
		if (ret < 0)
			goto err_ret;

		count += ret;
		if ((unsigned int)count >= PAGE_SIZE)
			goto err_ret;
	}

err_ret:
	mutex_unlock(&df->lock);
	return count;
}

static ssize_t store_policy(struct device *dev,
			    struct device_attribute *attr __maybe_unused,
			    const char *buf,
			    size_t count)
{
	int ret;
	struct devfreq *df = to_devfreq(dev);

	ret = policy_parse(df, buf, count);
	if (ret != 0)
		return -EINVAL;

	return count;
}

static ssize_t store_policy_index(struct device *dev,
				  struct device_attribute *attr __maybe_unused,
				  const char *buf,
				  size_t count)
{
	int ret;
	unsigned int policy_index;
	struct devfreq *df = to_devfreq(dev);
	struct dynamic_data *data = df->data;
	struct dynamic_policy *policy = NULL;
	char local_buf[POLICY_INDEX_BUF_MAX] = {0};

	if (count >= POLICY_INDEX_BUF_MAX)
		return -EINVAL;

	ret = strncpy_s(local_buf, POLICY_INDEX_BUF_MAX, buf, count);
	if (ret != EOK)
		return -EINVAL;

	ret = kstrtouint(local_buf, 10, &policy_index);
	if (ret != 0)
		return -EINVAL;

	if (data->cur_policy->para[POLICY_INDEX] == policy_index)
		return count;

	mutex_lock(&df->lock);
	list_for_each_entry(policy, &data->policy_list, node) {
		if (policy->para[POLICY_INDEX] == policy_index) {
			dynamic_refresh_load(df, policy);
			data->cur_policy = policy;
			break;
		}
	}
	mutex_unlock(&df->lock);
	return count;
}

static ssize_t show_policy_index(struct device *dev,
				 struct device_attribute *attr __maybe_unused,
				 char *buf)
{
	int ret;
	struct devfreq *df = to_devfreq(dev);
	struct dynamic_data *data = NULL;

	mutex_lock(&df->lock);
	data = df->data;
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			 "%lu\n", data->cur_policy->para[POLICY_INDEX]);
	mutex_unlock(&df->lock);

	return ret;
}

static ssize_t show_cl_accel(struct device *dev,
			     struct device_attribute *attr __maybe_unused,
			     char *buf)
{
	struct devfreq *df = to_devfreq(dev);
	struct dynamic_data *data = NULL;
	int ret;

	mutex_lock(&df->lock);
	data = df->data;
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			 "cl_accel:%s\n corresponding freq:%lu\n",
			 data->cl_accel == 3 ? HIGH_BOOST_FREQ : data->cl_accel == 2 ? MED_BOOST_FREQ : data->cl_accel == 1 ? LOW_BOOST_FREQ : BOOST_DISABLE,
			 data->cl_accel == 3 ? data->cl_high_accel_freq : data->cl_accel == 2 ? data->cl_med_accel_freq : data->cl_accel == 1 ? data->cl_low_accel_freq : 0);
	mutex_unlock(&df->lock);

	return ret;
}

static ssize_t store_cl_accel(struct device *dev,
			      struct device_attribute *attr __maybe_unused,
			      const char *buf,
			      size_t count)
{
	int ret;
	unsigned long freq;
	struct devfreq *df = to_devfreq(dev);
	struct dynamic_data *data = NULL;

	ret = kstrtoul(buf, 10, &freq);
	if (ret != 0) {
		pr_err("wrong input:%s\n", buf);
		return 0;
	}

	mutex_lock(&df->lock);
	data = df->data;
	data->cl_low_accel_freq = freq;
	data->cl_med_accel_freq = freq;
	data->cl_high_accel_freq = freq;
	ret = update_devfreq(df);
	if (ret == 0)
		ret = count;
	mutex_unlock(&df->lock);

	return ret;
}

static ssize_t store_user_freq(struct device *dev,
			       struct device_attribute *attr __maybe_unused,
			       const char *buf, size_t count)
{
	struct devfreq *df = to_devfreq(dev);
	struct dynamic_data *data = NULL;
	unsigned long input;
	int ret;

	ret = kstrtoul(buf, 10, &input);
	if (ret != 0)
		return -EINVAL;

	mutex_lock(&df->lock);
	data = df->data;
	if (data->freq_max != 0)
		input = min(data->freq_max, input);

	data->user_freq = input;
	ret = update_devfreq(df);
	if (ret == 0)
		ret = count;

	mutex_unlock(&df->lock);

	return ret;
}

static ssize_t store_hispeed_clear_enable(struct device *dev,
						struct device_attribute *attr __maybe_unused,
						const char *buf, size_t count)
{
	struct devfreq *df = to_devfreq(dev);
	struct dynamic_data *data = NULL;
	unsigned long input;
	int ret;

	ret = kstrtoul(buf, 10, &input);
	if (ret != 0)
		return -EINVAL;
	input = input > 0 ? 1 : 0;

	mutex_lock(&df->lock);
	data = df->data;
	data->hispeed_clear_enable = input;
	mutex_unlock(&df->lock);

	return count;
}


static ssize_t show_polling_history(struct device *dev,
				 struct device_attribute *attr __maybe_unused,
				 char *buf)
{
	unsigned long i, idx;
	int cnt = 0, ret;
	struct devfreq *df = to_devfreq(dev);
	struct dynamic_data *data = NULL;

	mutex_lock(&df->lock);
	data = df->data;
	idx = (data->window_idx + data->cur_policy->para[LOAD_WIN] - data->window_cnt) %
		data->cur_policy->para[LOAD_WIN];
	for (i = 0; i < data->window_cnt; i++) {
		ret = snprintf(buf + cnt, PAGE_SIZE - cnt,
				"win%lu KHz*load %lu, load %lu%%, polling %llu us\n", i, data->buf[idx] / 100000,
				data->buf_load[idx], data->buf_poling_time[idx]);
		if (ret < 0) {
			mutex_unlock(&df->lock);
			return ret;
		}

		cnt += ret;
		idx = (idx + 1) % data->cur_policy->para[LOAD_WIN];
	}
	mutex_unlock(&df->lock);

	return cnt;
}

static DYNAMIC_ATTR_RW(policy);
static DYNAMIC_ATTR_RW(policy_index);
static DYNAMIC_ATTR_RW(cl_accel);
static DYNAMIC_ATTR_RW(user_freq);
static DYNAMIC_ATTR_RW(hispeed_clear_enable);

static DYNAMIC_ATTR_RO(util);
static DYNAMIC_ATTR_RO(normalized_util);
static DYNAMIC_ATTR_RO(polling_history);
static DYNAMIC_ATTR_RO(window_cnt);
static DYNAMIC_ATTR_RO(jiff_sum);
static DYNAMIC_ATTR_RO(util_sum);

static struct attribute *dev_entries[] = {
	&dev_attr_policy.attr,
	&dev_attr_policy_index.attr,
	&dev_attr_cl_accel.attr,
	&dev_attr_user_freq.attr,
	&dev_attr_util.attr,
	&dev_attr_normalized_util.attr,
	&dev_attr_polling_history.attr,
	&dev_attr_window_cnt.attr,
	&dev_attr_jiff_sum.attr,
	&dev_attr_util_sum.attr,
	&dev_attr_hispeed_clear_enable.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.name = DYNAMIC_DIR_NAME,
	.attrs = dev_entries,
};

static void devfreq_dynamic_policy_dts_init(struct devfreq *df)
{
	int ret, count, i;
	unsigned int cl_low_accel_freq;
	unsigned int cl_med_accel_freq;
	unsigned int cl_high_accel_freq;
	const char *policy_str = NULL;
	struct dynamic_data *data = df->data;
	struct device *dev = df->dev.parent;
	struct device_node *np = dev->of_node;

	ret = of_property_read_u32(np, "cl-low-accel-freq", &cl_low_accel_freq);
	if (ret != 0)
		pr_err("%s: read cl-low-accel-freq fail\n", __func__);
	else
		data->cl_low_accel_freq = cl_low_accel_freq;

	ret = of_property_read_u32(np, "cl-med-accel-freq", &cl_med_accel_freq);
	if (ret != 0)
		pr_err("%s: read cl-med-accel-freq fail\n", __func__);
	else
		data->cl_med_accel_freq = cl_med_accel_freq;

	ret = of_property_read_u32(np, "cl-high-accel-freq", &cl_high_accel_freq);
	if (ret != 0)
		pr_err("%s: read cl-high-accel-freq fail\n", __func__);
	else
		data->cl_high_accel_freq = cl_high_accel_freq;

	count = of_property_count_strings(np, "policy");
	if (count <= 0) {
		pr_err("%s: count policy err %d\n", __func__, count);
		goto out;
	}

	count = min(count, POLICY_INDEX_MAX);
	for (i = 0; i < count; i++) {
		ret = of_property_read_string_index(np, "policy", i, &policy_str);
		if (ret != 0)
			continue;

		ret = policy_parse(df, policy_str, strlen(policy_str) + 1);
		if (ret != 0) {
			pr_err("%s: parse policy %s fail\n", __func__, policy_str);
			goto out;
		}
	}

out:
	of_node_put(np);
}

static int devfreq_dynamic_policy_init(struct devfreq *df)
{
	int err = -ENOMEM;
	struct dynamic_data *data = NULL;
	struct devfreq_dev_profile *profile = NULL;
	struct dynamic_policy *policy = NULL;

	if (df->data != NULL)
		goto err_out;

	if (IS_ERR_OR_NULL(df->profile))
		goto err_out;

	profile = df->profile;

	data = kzalloc(sizeof(struct dynamic_data), GFP_KERNEL);
	if (!data)
		goto err_out;

	policy = kzalloc(sizeof(struct dynamic_policy), GFP_KERNEL);
	if (!policy)
		goto err_data;

	policy->target_load = kzalloc(sizeof(*policy->target_load) * DEFAULT_NTARGET_LOAD, GFP_KERNEL);
	if (!policy->target_load)
		goto err_policy;

	policy->para[POLICY_INDEX] = 0;
	policy->para[HISPEED_LOAD] = DEFAULT_HISPEED_LOAD;
	policy->para[HISPEED_FREQ] = DEFAULT_HISPEED_FREQ;
	policy->para[LOAD_WIN] = DEFAULT_LOADING_WINDOW;
	policy->ntarget_load = DEFAULT_NTARGET_LOAD;
	data->cl_low_accel_freq = DEFAULT_CL_ACCEL_FREQ;
	data->cl_med_accel_freq = DEFAULT_CL_ACCEL_FREQ;
	data->cl_high_accel_freq = DEFAULT_CL_ACCEL_FREQ;

	if (profile->max_state > 0 && profile->freq_table != NULL)
		data->freq_max = profile->freq_table[profile->max_state - 1];

	*(policy->target_load) = DEFAULT_TARGET_LOAD;
	data->cur_policy = policy;
	data->hispeed_clear_enable = true;

	INIT_LIST_HEAD(&data->policy_list);
	list_add(&policy->node, &data->policy_list);

	df->data = data;
	devfreq_dynamic_policy_dts_init(df);

	err = sysfs_create_group(&df->dev.kobj, &dev_attr_group);
	if (err != 0)
		pr_err("%s: sysfs create err %d\n", __func__, err);

	return 0;

err_policy:
	kfree(policy);
err_data:
	kfree(data);
	df->data = NULL;
err_out:
	return err;
}

static void devfreq_dynamic_policy_exit(struct devfreq *df)
{
	struct dynamic_data *data = NULL;
	struct dynamic_policy *policy = NULL;

	data = df->data;
	if (IS_ERR_OR_NULL(data))
		return;

	sysfs_remove_group(&df->dev.kobj, &dev_attr_group);

	while (!list_empty(&data->policy_list)) {
		policy = list_first_entry(&data->policy_list,
					  struct dynamic_policy, node);

		policy_destroy(policy);
	}

	kfree(data);
	df->data = NULL;
}

void devfreq_gpu_gov_suspend(struct devfreq *df)
{
	if (!df)
		return;

	mutex_lock(&df->lock);
	update_devfreq(df);
	mutex_unlock(&df->lock);
	devfreq_monitor_suspend(df);
}

void devfreq_gpu_gov_resume(struct devfreq *df)
{
	struct dynamic_data *data = NULL;

	if (!df)
		return;

	mutex_lock(&df->lock);
	data = df->data;
	data->last_jiff = xr_timestamp_count2ns(xr_timestamp_getcount()) / 1000;
	mutex_unlock(&df->lock);
	devfreq_monitor_resume(df);
}

static int devfreq_dynamic_handler(struct devfreq *df,
				   unsigned int event,
				   void *data)
{
	int ret;

	switch (event) {
	case DEVFREQ_GOV_START:
		ret = devfreq_dynamic_policy_init(df);
		if (ret == 0) {
			devfreq_monitor_start(df);
			devfreq_monitor_suspend(df);
		}
		break;
	case DEVFREQ_GOV_STOP:
		devfreq_monitor_stop(df);
		devfreq_dynamic_policy_exit(df);
		break;
	case DEVFREQ_GOV_UPDATE_INTERVAL:
		devfreq_update_interval(df, (unsigned int *)data);
		break;
	case DEVFREQ_GOV_SUSPEND:
		devfreq_gpu_gov_suspend(df);
		break;
	case DEVFREQ_GOV_RESUME:
		devfreq_gpu_gov_resume(df);
		break;
	default:
		break;
	}

	return 0;
}

static struct devfreq_governor devfreq_dynamic = {
	.name = "dynamic",
	.flags = DEVFREQ_GOV_FLAG_IMMUTABLE,
	.attrs = DEVFREQ_GOV_ATTR_POLLING_INTERVAL,
	.get_target_freq = devfreq_dynamic_func,
	.event_handler = devfreq_dynamic_handler,
};

static int __init devfreq_dynamic_init(void)
{
	return devfreq_add_governor(&devfreq_dynamic);
}
subsys_initcall(devfreq_dynamic_init);

static void __exit devfreq_dynamic_exit(void)
{
	int ret;

	ret = devfreq_remove_governor(&devfreq_dynamic);
	if (ret != 0)
		pr_err("failed remove governor %d\n", ret);
}
module_exit(devfreq_dynamic_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("xring gpufreq governor of devfreq framework");
