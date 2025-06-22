// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#define pr_fmt(fmt) "memlat: " fmt

#include <linux/device.h>
#include <linux/kstrtox.h>
#include <linux/limits.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <dt-bindings/xring/memlat_define.h>

#include "dvfs_gov.h"

#define INTVAL_MIN		2000 /* us */
#define INTVAL_MAX		12000 /* us */
#define ENABLE_MAX		1
#define IPM_THRES_MAX		U16_MAX
#define FE_STALL_THRES_MAX	U16_MAX
#define BE_STALL_THRES_MAX	U16_MAX
#define FREQ_SCALE_CEIL_MAX	U16_MAX
#define FREQ_SCALE_FLOOR_MAX	U16_MAX
#define FREQ_SCALE_COEFF_MAX	U8_MAX

enum {
	ENABLE,
	IPM_THRES,
	FE_STALL_THRES,
	BE_STALL_THRES,
	FREQ_SCALE_CEIL,
	FREQ_SCALE_FLOOR,
	FREQ_SCALE_COEFF,
	INTVAL,
	PARA_TYPE_NUM = FREQ_SCALE_COEFF + 1,
};

enum memlat_type {
	MEMLAT_DEV,
	MEMLAT_MEM,
	MEMLAT_MON,
};

struct memlat_spec {
	enum memlat_type type;
};

struct memlat_mon {
	u8 type;
	u8 cluster_id;
	u16 paras[PARA_TYPE_NUM];
};

struct memlat_mem {
	u32 type;
	struct memlat_mon *mon;
};

struct memlat {
	struct mutex mutex;
	struct memlat_mem mems[MEM_NUM];
	u16 interval_us;
	u8 inited;
};

static struct memlat g_memlat;

static ssize_t interval_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf,
			      size_t count)
{
	struct vote_msg msg = {0};
	unsigned long interval_us;
	int ret = 0;

	if (kstrtoul(buf, 0, &interval_us)) {
		dev_err(dev, "parse interval failed\n");
		return -EINVAL;
	}
	if (interval_us > INTVAL_MAX || interval_us < INTVAL_MIN) {
		dev_err(dev, "invalid value[%lu] for interval sys node\n",
			interval_us);
		return -EINVAL;
	}
	mutex_lock(&g_memlat.mutex);
	if (g_memlat.interval_us != interval_us) {
		msg.dev_type = MEMLAT;
		msg.value = interval_us;
		msg.para_type = INTVAL;
		msg.msg_type = SET_TYPE;
		ret = dvfs_gov_msg_trans((struct vote_msg *)&msg);
		if (ret < 0)
			dev_err(dev, "dvfs gov msg send failed, err:%d\n", ret);
		else
			g_memlat.interval_us = interval_us;
	}
	mutex_unlock(&g_memlat.mutex);

	return ret < 0 ? ret : count;
}

static ssize_t interval_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct vote_msg msg = {0};
	u32 interval_us = 0U;
	int ret;

	mutex_lock(&g_memlat.mutex);
	if (g_memlat.interval_us == 0) {
		msg.dev_type = MEMLAT;
		msg.para_type = INTVAL;
		msg.msg_type = GET_TYPE;
		ret = dvfs_gov_msg_trans((struct vote_msg *)&msg);
		if (ret < 0) {
			dev_err(dev, "vote mng msg send failed, err:%d\n", ret);
			mutex_unlock(&g_memlat.mutex);
			return ret;
		}
		g_memlat.interval_us = msg.value;
	}
	interval_us = g_memlat.interval_us;
	mutex_unlock(&g_memlat.mutex);

	return snprintf(buf, PAGE_SIZE, "%s:%dus\n", "interval", interval_us);
}

static inline int memlat_vote_msg_func(u8 para_type, u8 msg_type, u16 value,
				       struct memlat_mon *mon,
				       struct vote_msg *msg)
{
	msg->dev_type = MEMLAT;
	msg->para_type = para_type;
	msg->msg_type = msg_type;
	msg->mem_type = mon->type;
	msg->cluster_id = mon->cluster_id;

	if (msg_type == SET_TYPE)
		msg->value = value;

	return dvfs_gov_msg_trans((struct vote_msg *)msg);
}

#define SYS_STORE(name, sys_type) \
static ssize_t name##_store(struct device *dev,\
			    struct device_attribute *attr,\
			    const char *buf,\
			    size_t count)\
{\
	struct memlat_mon *mon = NULL;\
	struct vote_msg msg = {0};\
	unsigned long value;\
	int ret = 0;\
	if (kstrtoul(buf, 0, &value) != 0) { \
		dev_err(dev, "parse %s failed\n", #name);\
		return -EINVAL;\
	} \
	if (value > sys_type##_MAX) {\
		dev_err(dev, "invalid value[%lu] for %s\n", value, #name);\
		return -EINVAL;\
	} \
	mon = dev_get_drvdata(dev);\
	mutex_lock(&g_memlat.mutex);\
	if (mon->paras[sys_type] != value) { \
		ret = memlat_vote_msg_func(sys_type, SET_TYPE, value,\
					   mon, &msg);\
		if (ret < 0)\
			dev_err(dev, "vote msg func failed, err:%d\n", ret);\
		else\
			mon->paras[sys_type] = value;\
	} \
	mutex_unlock(&g_memlat.mutex);\
	return ret < 0 ? ret : count;\
}

#define SYS_SHOW(name, sys_type) \
static ssize_t name##_show(struct device *dev,\
			   struct device_attribute *attr,\
			   char *buf)\
{\
	struct memlat_mon *mon = NULL;\
	struct vote_msg msg = {0};\
	u32 value;\
	int ret;\
	mon = dev_get_drvdata(dev);\
	mutex_lock(&g_memlat.mutex);\
	if (mon->paras[sys_type] == 0 && sys_type != ENABLE) { \
		ret = memlat_vote_msg_func(sys_type, GET_TYPE, 0, mon, &msg);\
		if (ret < 0) { \
			dev_err(dev, "vote mng query failed, err:%d\n", ret);\
			mutex_unlock(&g_memlat.mutex);\
			return ret;\
		} \
		mon->paras[sys_type] = msg.value;\
	} \
	value = mon->paras[sys_type];\
	mutex_unlock(&g_memlat.mutex);\
	return snprintf(buf, PAGE_SIZE, "%s:%d\n", #name, value);\
}

SYS_STORE(enable, ENABLE)
SYS_SHOW(enable, ENABLE)
SYS_STORE(ipm_thres, IPM_THRES)
SYS_SHOW(ipm_thres, IPM_THRES)
SYS_STORE(fe_stall_thres, FE_STALL_THRES)
SYS_SHOW(fe_stall_thres, FE_STALL_THRES)
SYS_STORE(be_stall_thres, BE_STALL_THRES)
SYS_SHOW(be_stall_thres, BE_STALL_THRES)
SYS_STORE(freq_scale_ceil, FREQ_SCALE_CEIL)
SYS_SHOW(freq_scale_ceil, FREQ_SCALE_CEIL)
SYS_STORE(freq_scale_floor, FREQ_SCALE_FLOOR)
SYS_SHOW(freq_scale_floor, FREQ_SCALE_FLOOR)
SYS_STORE(freq_scale_coeff, FREQ_SCALE_COEFF)
SYS_SHOW(freq_scale_coeff, FREQ_SCALE_COEFF)

static DEVICE_ATTR(interval, NODE_MODE_RW,
		   interval_show, interval_store);
static DEVICE_ATTR(enable, NODE_MODE_RW,
		   enable_show, enable_store);
static DEVICE_ATTR(ipm_thres, NODE_MODE_RW,
		   ipm_thres_show, ipm_thres_store);
static DEVICE_ATTR(fe_stall_thres, NODE_MODE_RW,
		   fe_stall_thres_show, fe_stall_thres_store);
static DEVICE_ATTR(be_stall_thres, NODE_MODE_RW,
		   be_stall_thres_show, be_stall_thres_store);
static DEVICE_ATTR(freq_scale_ceil, NODE_MODE_RW,
		   freq_scale_ceil_show, freq_scale_ceil_store);
static DEVICE_ATTR(freq_scale_floor, NODE_MODE_RW,
		   freq_scale_floor_show, freq_scale_floor_store);
static DEVICE_ATTR(freq_scale_coeff, NODE_MODE_RW,
		   freq_scale_coeff_show, freq_scale_coeff_store);

static struct attribute *memlat_attrs[] = {
	&dev_attr_enable.attr,
	&dev_attr_ipm_thres.attr,
	&dev_attr_fe_stall_thres.attr,
	&dev_attr_be_stall_thres.attr,
	&dev_attr_freq_scale_ceil.attr,
	&dev_attr_freq_scale_floor.attr,
	&dev_attr_freq_scale_coeff.attr,
	NULL,
};

static struct attribute_group memlat_attr_group = {
	.attrs = memlat_attrs,
};

static int memlat_mon_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct memlat_mem *mem = NULL;
	struct memlat_mon *mon = NULL;
	u32 cluster_id;

	mem = dev_get_drvdata(dev->parent);
	if (mem == NULL) {
		dev_err(dev, "memlat mem is not inited\n");
		return -EINVAL;
	}
	if (of_property_read_u32(dev->of_node, "cluster_id", &cluster_id)) {
		dev_err(dev, "cluster_id is misssing\n");
		return -EINVAL;
	}
	if (cluster_id >= get_cluster_num()) {
		dev_err(dev, "invalid cluster id[%u]\n", cluster_id);
		return -EINVAL;
	}
	mon = &mem->mon[cluster_id];
	mon->type = mem->type;
	mon->cluster_id = cluster_id;
	platform_set_drvdata(pdev, mon);
	(void)sysfs_create_group(&dev->kobj, &memlat_attr_group);

	return 0;
}

static int memlat_mem_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct memlat_mem *mem = NULL;
	u32 count;
	u32 type;

	if (of_property_read_u32(dev->of_node, "dev_type", &type)) {
		dev_err(dev, "memlat dev type is misssing\n");
		return -EINVAL;
	}
	if (type >= MEM_NUM)
		return -EINVAL;

	mem = &g_memlat.mems[type];
	mem->type = type;
	count = of_get_available_child_count(dev->of_node);
	if (count != get_cluster_num()) {
		dev_err(dev, "Invalid memlat mon count[%d]\n", count);
		return -EINVAL;
	}
	platform_set_drvdata(pdev, mem);
	mem->mon = devm_kzalloc(dev, sizeof(struct memlat_mon) * count, GFP_KERNEL);
	if (mem->mon == NULL) {
		dev_err(dev, "alloc memlat_mon failed\n");
		return -ENOMEM;
	}
	if (of_get_available_child_count(dev->of_node))
		of_platform_populate(dev->of_node, NULL, NULL, dev);

	return 0;
}

static int memlat_dev_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	if (g_memlat.inited) {
		dev_err(dev, "only one memlat device allowed\n");
		return -ENODEV;
	}
	g_memlat.inited = 1U;
	mutex_init(&g_memlat.mutex);
	(void)sysfs_create_file(&dev->kobj, &dev_attr_interval.attr);
	if (of_get_available_child_count(dev->of_node))
		of_platform_populate(dev->of_node, NULL, NULL, dev);

	return 0;
}

static int memlat_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const struct memlat_spec *spec = of_device_get_match_data(dev);
	int ret = 0;

	if (spec == NULL) {
		dev_err(dev, "memlat spec is empty\n");
		return -EINVAL;
	}

	switch (spec->type) {
	case MEMLAT_DEV:
		ret = memlat_dev_probe(pdev);
		break;
	case MEMLAT_MEM:
		ret = memlat_mem_probe(pdev);
		break;
	case MEMLAT_MON:
		ret = memlat_mon_probe(pdev);
		break;
	default:
		dev_err(dev, "Invalid memlat spec type:%u\n", spec->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static const struct memlat_spec spec[] = {
	[0] = { MEMLAT_DEV },
	[1] = { MEMLAT_MEM },
	[2] = { MEMLAT_MON },
};

static const struct of_device_id g_memlat_match_table[] = {
	{
		.compatible = "xring,memlat",
		.data = &spec[0],
	},
	{
		.compatible = "xring,memlat-mem",
		.data = &spec[1],
	},
	{
		.compatible = "xring,memlat-mon",
		.data = &spec[2],
	},
	{},
};

static struct platform_driver g_memlat_driver = {
	.probe = memlat_probe,
	.driver = {
		.name = "memlat",
		.owner = THIS_MODULE,
		.of_match_table = g_memlat_match_table,
	},
};

int memlat_init(void)
{
	return platform_driver_register(&g_memlat_driver);
}

MODULE_AUTHOR("Shaohua Fan <fanshaohua@xiaomi.com>");
MODULE_DESCRIPTION("XRing Memlat Driver");
MODULE_LICENSE("GPL v2");
