// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#define pr_fmt(fmt) "xsee: " fmt

#include <linux/cpufreq.h>
#include <linux/device.h>
#include <linux/kstrtox.h>
#include <linux/limits.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <soc/xring/vote_mng.h>

#include "dvfs_gov.h"

enum {
	RECENT_POLICY,
	MAX_POLICY,
	AVG_POLICY,
};

#define POLICY_MAX	AVG_POLICY
#define INTVL_MAX	20000
#define IPC_MAX		U16_MAX
#define DELAY_MAX	U16_MAX
#define DOWN_STEP_MAX	U16_MAX
#define BOOST_MAX	0xf
#define MAP_MAX_LEN	7
#define KHZ_PER_MHZ	1000

enum xsee_type {
	XSEE_DEV,
	XSEE_NODE,
};

struct xsee_spec {
	enum xsee_type type;
};

enum {
	ENABLE,
	POLICY,
	INTVL,
	DOWN_STEP,
	PARA_NUM,
	MAP = PARA_NUM,
	BOOST,
	PARA_TYPE_NUM,
};

struct xsee_map {
	u16 ipc;
	u16 limit;
};

enum {
	LIMIT_FREQ,
	UP_DELAY,
	DOWN_DELAY,
	MAP_TYPE_NUM,
};

struct xsee_node {
	u32 id;
	int paras[PARA_NUM];
	struct xsee_map maps[MAP_TYPE_NUM][MAP_MAX_LEN];
	u32 lens[MAP_TYPE_NUM];
};

static struct mutex g_mutex;
static int g_boost_bitmap;
static int g_enable_bitmap;

static int xsee_paras_store(struct xsee_node *node, unsigned long value, int type)
{
	struct vote_msg msg = {0};
	int ret = 0;
	int *para = NULL;

	if (type == BOOST)
		para = &g_boost_bitmap;
	else
		para = &node->paras[type];

	mutex_lock(&g_mutex);
	if (*para != value) {
		if (type == BOOST && g_enable_bitmap == 0) {
			*para = value;
			goto out;
		} else {
			msg.dev_type = XSEE;
			msg.msg_type = SET_TYPE;
			msg.cluster_id = (u8)node->id;
			msg.para_type = type;
			msg.value = (u16)value;
			if (type == ENABLE)
				msg.value2 = (u16)g_boost_bitmap;

			ret = dvfs_gov_msg_trans(&msg);
			if (ret < 0) {
				pr_err("vote msg failed, err:%d\n", ret);
				goto out;
			}
			*para = value;
			if (type != ENABLE)
				goto out;

			if (value == 1)
				g_enable_bitmap |= (1 << node->id);
			else
				g_enable_bitmap &= ~(1 << node->id);
		}
	}
out:
	mutex_unlock(&g_mutex);
	return ret;
}

#define SYS_STORE(name, sys_type, max_val) \
static ssize_t name##_store(struct device *dev,\
			    struct device_attribute *attr,\
			    const char *buf,\
			    size_t count)\
{\
	struct xsee_node *node = NULL;\
	unsigned long value;\
	int ret = 0;\
	if (kstrtoul(buf, 0, &value) != 0) { \
		dev_err(dev, "parse %s failed\n", #name);\
		return -EINVAL;\
	} \
	if (value > max_val) { \
		dev_err(dev, "invalid value[%lu] for %s\n", value, #name);\
		return -EINVAL;\
	} \
	node = dev_get_drvdata(dev);\
	ret = xsee_paras_store(node, value, sys_type);\
	return ret < 0 ? ret : count;\
}

SYS_STORE(enable, ENABLE, 1)
SYS_STORE(boost, BOOST, BOOST_MAX)
SYS_STORE(policy, POLICY, POLICY_MAX)
SYS_STORE(interval, INTVL, INTVL_MAX)
SYS_STORE(down_step, DOWN_STEP, DOWN_STEP_MAX)

static int xsee_paras_show(struct xsee_node *node, int type)
{
	struct vote_msg msg = {0};
	int *para = NULL;
	int ret;

	if (type == BOOST)
		para = &g_boost_bitmap;
	else
		para = &node->paras[type];

	mutex_lock(&g_mutex);
	if (*para == -1) {
		msg.dev_type = XSEE;
		msg.para_type = type;
		msg.msg_type = GET_TYPE;
		msg.cluster_id = (u8)node->id;
		ret = dvfs_gov_msg_trans(&msg);
		if (ret < 0) {
			pr_err("vote mng query failed, err:%d\n", ret);
			goto out;
		}
		*para = msg.value;
	}
out:
	mutex_unlock(&g_mutex);
	return *para;
}

#define SYS_SHOW(name, sys_type) \
static ssize_t name##_show(struct device *dev,\
			   struct device_attribute *attr,\
			   char *buf)\
{\
	struct xsee_node *node = NULL;\
	int para;\
	node = dev_get_drvdata(dev);\
	para = xsee_paras_show(node, sys_type);\
	return scnprintf(buf, PAGE_SIZE, "%s:%d\n", #name, para);\
}

SYS_SHOW(enable, ENABLE)
SYS_SHOW(boost, BOOST)
SYS_SHOW(policy, POLICY)
SYS_SHOW(interval, INTVL)
SYS_SHOW(down_step, DOWN_STEP)

#define SYS_MAP_STORE(name, sys_type, max_val) \
static ssize_t name##_store(struct device *dev,\
			    struct device_attribute *attr,\
			    const char *buf,\
			    size_t count)\
{\
	struct xsee_node *node = NULL;\
	struct vote_msg msg = {0};\
	char *p = NULL;\
	char *token = NULL;\
	struct xsee_map *map = NULL;\
	u16 data[MAP_MAX_LEN * 2] = {0};\
	unsigned int value;\
	unsigned int max_value;\
	int ret = 0;\
	u32 cnt = 0;\
	p = kstrdup(buf, GFP_KERNEL);\
	if (p == NULL)\
		return -EINVAL;\
	token = strsep(&p, ": ");\
	while (token != NULL) { \
		ret = kstrtouint(token, 0, &value);\
		if (ret)\
			return -EINVAL;\
		max_value = (cnt % 2 == 0) ? IPC_MAX : max_val;\
		if (value > max_value) { \
			dev_err(dev, "invalid value[%d]\n", value);\
			return -EINVAL;\
		} \
		data[cnt] = (u16)value;\
		cnt++;\
		token = strsep(&p, ": ");\
	} \
	if (cnt % 2 != 0 || cnt > MAP_MAX_LEN * 2) { \
		dev_err(dev, "invalid format or too large for map\n");\
		return -EINVAL;\
	} \
	node = dev_get_drvdata(dev);\
	msg.dev_type = XSEE;\
	msg.data_size = cnt * sizeof(u16);\
	msg.para_type = MAP;\
	msg.msg_type = SET_TYPE;\
	msg.cluster_id = node->id;\
	msg.value = sys_type;\
	mutex_lock(&g_mutex);\
	ret = dvfs_gov_data_send(&msg, (u8 *)data);\
	if (ret < 0) { \
		dev_err(dev, "vote data failed, err:%d\n", ret);\
	} else { \
		map = node->maps[sys_type];\
		memcpy(map, data, cnt * sizeof(u16));\
		node->lens[sys_type] = cnt / 2;\
	} \
	mutex_unlock(&g_mutex);\
	return ret < 0 ? ret : count;\
}

SYS_MAP_STORE(limit_freq, LIMIT_FREQ, 100)
SYS_MAP_STORE(up_delay, UP_DELAY, DELAY_MAX)
SYS_MAP_STORE(down_delay, DOWN_DELAY, DELAY_MAX)

static int map_show(char *buf, struct xsee_map *map, u32 len)
{
	int i;
	int cnt = 0;

	for (i = 0; i < len; i++) {
		cnt += scnprintf(buf + cnt, PAGE_SIZE - cnt,
				 "%d %d\n", map->ipc, map->limit);
		map++;
	}

	return cnt;
}

#define SYS_MAP_SHOW(name, sys_type) \
static ssize_t name##_show(struct device *dev,\
			   struct device_attribute *attr,\
			   char *buf)\
{\
	struct xsee_node *node = NULL;\
	struct vote_msg msg = {0};\
	struct xsee_map *map = NULL;\
	u16 data[MAP_MAX_LEN * 2] = {0};\
	int ret = 0;\
	u32 map_len = 0;\
	u32 size;\
	node = dev_get_drvdata(dev);\
	map = node->maps[sys_type];\
	mutex_lock(&g_mutex);\
	map_len = node->lens[sys_type];\
	if (map_len > 0) { \
		ret = map_show(buf, map, map_len);\
		goto out;\
	} \
	msg.dev_type = XSEE;\
	msg.para_type = MAP;\
	msg.msg_type = GET_TYPE;\
	msg.cluster_id = node->id;\
	msg.value = sys_type;\
	ret = dvfs_gov_data_receive(&msg, (u8 *)data, sizeof(data));\
	if (ret < 0) { \
		dev_err(dev, "vote data failed, err:%d\n", ret);\
	} else { \
		size = msg.data_size;\
		memcpy(map, data, size);\
		map_len = size / sizeof(struct xsee_map);\
		node->lens[sys_type] = map_len;\
		ret = map_show(buf, map, map_len);\
	} \
out:\
	mutex_unlock(&g_mutex);\
	return ret;\
}

SYS_MAP_SHOW(limit_freq, LIMIT_FREQ)
SYS_MAP_SHOW(up_delay, UP_DELAY)
SYS_MAP_SHOW(down_delay, DOWN_DELAY)

static DEVICE_ATTR(boost, NODE_MODE_RW,
		   boost_show, boost_store);
static DEVICE_ATTR(enable, NODE_MODE_RW,
		   enable_show, enable_store);
static DEVICE_ATTR(policy, NODE_MODE_RW,
		   policy_show, policy_store);
static DEVICE_ATTR(interval, NODE_MODE_RW, interval_show, interval_store);
static DEVICE_ATTR(down_step, NODE_MODE_RW, down_step_show, down_step_store);
static DEVICE_ATTR(limit_freq, NODE_MODE_RW,
		   limit_freq_show, limit_freq_store);
static DEVICE_ATTR(up_delay, NODE_MODE_RW,
		   up_delay_show, up_delay_store);
static DEVICE_ATTR(down_delay, NODE_MODE_RW,
		   down_delay_show, down_delay_store);

static struct attribute *xsee_attrs[] = {
	&dev_attr_boost.attr,
	&dev_attr_enable.attr,
	&dev_attr_policy.attr,
	&dev_attr_interval.attr,
	&dev_attr_down_step.attr,
	&dev_attr_limit_freq.attr,
	&dev_attr_up_delay.attr,
	&dev_attr_down_delay.attr,
	NULL,
};

static struct attribute_group xsee_attr_group = {
	.attrs = xsee_attrs,
};

static int xsee_node_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xsee_node *node = NULL;
	u32 cluster_id;

	if (of_property_read_u32(dev->of_node, "cluster_id", &cluster_id)) {
		dev_err(dev, "cluster_id is misssing\n");
		return -EINVAL;
	}
	if (cluster_id >= get_cluster_num()) {
		dev_err(dev, "invalid cluster id[%u]\n", cluster_id);
		return -EINVAL;
	}
	node = devm_kzalloc(dev, sizeof(struct xsee_node), GFP_KERNEL);
	if (node == NULL)
		return -ENOMEM;

	node->id = cluster_id;
	node->paras[ENABLE] = 0;
	node->paras[POLICY] = -1;
	node->paras[INTVL] = -1;
	node->paras[DOWN_STEP] = -1;
	node->lens[LIMIT_FREQ] = 0;
	node->lens[UP_DELAY] = 0;
	node->lens[DOWN_DELAY] = 0;

	platform_set_drvdata(pdev, node);
	(void)sysfs_create_group(&dev->kobj, &xsee_attr_group);

	return 0;
}

static int xsee_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const struct xsee_spec *spec = of_device_get_match_data(dev);
	int ret = 0;

	if (spec == NULL) {
		dev_err(dev, "xsee spec is empty\n");
		return -EINVAL;
	}
	if (spec->type == XSEE_DEV) {
		mutex_init(&g_mutex);
		if (of_get_available_child_count(dev->of_node))
			of_platform_populate(dev->of_node, NULL, NULL, dev);
	} else if (spec->type == XSEE_NODE) {
		ret = xsee_node_probe(pdev);
	} else {
		dev_err(dev, "Invalid xsee spec type:%u\n", spec->type);
		ret = -EINVAL;
	}

	return ret;
}

static const struct xsee_spec spec[] = {
	[0] = { XSEE_DEV },
	[1] = { XSEE_NODE },
};

static const struct of_device_id match_table[] = {
	{
		.compatible = "xring,xsee",
		.data = &spec[0],
	},
	{
		.compatible = "xring,xsee-node",
		.data = &spec[1],
	},
	{},
};

static struct platform_driver g_xsee_driver = {
	.probe = xsee_probe,
	.driver = {
		.name = "xsee",
		.owner = THIS_MODULE,
		.of_match_table = match_table,
	},
};

int xsee_init(void)
{
	return platform_driver_register(&g_xsee_driver);
}

MODULE_AUTHOR("Shaohua Fan <fanshaohua@xiaomi.com>");
MODULE_DESCRIPTION("XRing System Energy Engine Driver");
MODULE_LICENSE("GPL v2");
