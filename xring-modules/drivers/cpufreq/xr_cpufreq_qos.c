// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023-2025, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cpufreq.h>
#include <linux/pm_qos.h>
#include <linux/cpumask.h>
#include <linux/of.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>
#include "xr_cpufreq.h"

#define DEVICE_NAME         "xr_cpufreq_qos"
#define BOOT_LIMIT_CFG      "boot-limit-cfg"
#define BOOT_LIMIT          "boot-limit"
#define SUSPEND_LIMIT_CFG   "suspend-limit-cfg"
#define SUSPEND_SCHED_UTIL   (550U)
#define MAX_CPUS          NR_CPUS

#define ADD_REQUEST       0
#define REMOVE_REQUEST    1

struct cluster_limit {
	struct cpumask mask;
	struct freq_qos_request qos_req;
	unsigned int freq;
};

static uint32_t boot_limit;
static struct cluster_limit *cluster_map;
static struct cluster_limit *suspend_cluster_map;
static uint32_t limit_count;
static uint32_t suspend_limit_count;
static struct mutex qos_mutex_lock;


static void limit_request(uint32_t type)
{
	ssize_t ret = 0;
	uint32_t index;
	int cpu = -1;
	struct cpufreq_policy *policy = 0;

	mutex_lock(&qos_mutex_lock);

	if ((type == ADD_REQUEST && boot_limit == 1) ||
	    (type == REMOVE_REQUEST && boot_limit == 0)) {
		mutex_unlock(&qos_mutex_lock);
		return;
	}

	for (index = 0; index < limit_count; ++index) {
		for_each_cpu(cpu, &cluster_map[index].mask) {
			policy  = cpufreq_cpu_get(cpu);
			if (policy)
				break;
		}

		if (policy) {
			if (type == ADD_REQUEST) {
				ret = freq_qos_add_request(&policy->constraints,
						&cluster_map[index].qos_req, FREQ_QOS_MAX,
						cluster_map[index].freq);
				boot_limit = ret >= 0 ? 1 : 0;
			} else if (type == REMOVE_REQUEST) {
				freq_qos_remove_request(&cluster_map[index].qos_req);
				boot_limit = 0;
			}

			cpufreq_cpu_put(policy);
		}

	}
	mutex_unlock(&qos_mutex_lock);
}

static ssize_t xr_cpufreq_qos_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned int value = 0;

	if (kstrtouint_from_user(buf, count, 10, &value))
		return -EINVAL;

	limit_request(value ? ADD_REQUEST : REMOVE_REQUEST);

	return count;
}

static const struct file_operations xr_cpufreq_qos_fops = {
	.owner = THIS_MODULE,
	.write = xr_cpufreq_qos_write,
};

static struct miscdevice xr_cpufreq_qos = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &xr_cpufreq_qos_fops,
};

static struct sched_attr xr_cpufreq_pm_attr = {
	.size = sizeof(struct sched_attr),
	.sched_policy = -1,
	.sched_priority = 0,
	.sched_flags = SCHED_FLAG_UTIL_CLAMP_MIN | SCHED_FLAG_KEEP_PARAMS,
	.sched_util_min = 0,
};

static int xr_cpufreq_pm_notifier(struct notifier_block *notify_block,
	unsigned long mode, void *_unused)
{
	int cpu = -1;
	int ret = 0;
	uint32_t index;
	struct cpufreq_policy *policy = 0;

	if (mode == PM_SUSPEND_PREPARE)
		xr_cpufreq_pm_attr.sched_util_min = SUSPEND_SCHED_UTIL;
	else if (mode == PM_POST_SUSPEND)
		xr_cpufreq_pm_attr.sched_util_min = 0;
	ret = sched_setattr_nocheck(current, &xr_cpufreq_pm_attr);
	if (ret)
		pr_err("sched_setattr_nocheck ret: %d\n", ret);

	for (index = 0; index < suspend_limit_count; ++index) {
		for_each_cpu(cpu, &suspend_cluster_map[index].mask) {
			policy  = cpufreq_cpu_get(cpu);
			if (policy)
				break;
		}

		if (policy) {
			if (mode == PM_SUSPEND_PREPARE) {
				ret = freq_qos_add_request(&policy->constraints,
						&suspend_cluster_map[index].qos_req, FREQ_QOS_MIN,
						suspend_cluster_map[index].freq);
			} else if (mode == PM_POST_SUSPEND) {
				freq_qos_remove_request(&suspend_cluster_map[index].qos_req);
			}
			cpufreq_cpu_put(policy);
		}
	}

	return ret;
}

static struct notifier_block cpufreq_pm_notifier_nb = {
	.notifier_call = xr_cpufreq_pm_notifier,
};

static int xr_cpufreq_qos_limit_cfg_init(struct device_node *node, uint32_t *limit_cnt,
					 struct cluster_limit **map, const char *propname)
{
	int ret;
	uint32_t cnt;
	int index = 0;
	uint32_t uint_mask = 0;

	if (!node)
		return -EINVAL;

	cnt = of_property_count_u32_elems(node, propname);
	if (cnt > MAX_CPUS || (cnt % 2))
		return -EINVAL;

	*limit_cnt = cnt / 2;
	*map = kcalloc(*limit_cnt, sizeof(**map), GFP_KERNEL);
	if (!(*map))
		return -ENOMEM;

	for (index = 0; index < *limit_cnt; ++index) {
		ret = of_property_read_u32_index(node, propname, 2 * index, &uint_mask);
		if (ret < 0)
			return ret;

		*cpumask_bits(&((*map)[index].mask)) = uint_mask;
		ret = of_property_read_u32_index(node, propname,
						 2 * index + 1, &((*map)[index].freq));
		if (ret < 0)
			return ret;
	}
	return 0;
}

int xr_cpufreq_qos_init(struct device_node *xr_node)
{
	int ret;
	uint32_t battery_state = 0;

	if (!xr_node)
		return -EINVAL;

	mutex_init(&qos_mutex_lock);

	ret = of_property_read_u32(xr_node, BOOT_LIMIT, &battery_state);
	if (ret < 0)
		return ret;

	ret = xr_cpufreq_qos_limit_cfg_init(xr_node, &limit_count, &cluster_map, BOOT_LIMIT_CFG);
	if (ret)
		return ret;

	ret = xr_cpufreq_qos_limit_cfg_init(xr_node, &suspend_limit_count,
					    &suspend_cluster_map, SUSPEND_LIMIT_CFG);
	if (ret)
		goto err_suspend_limit_cfg;

	ret = register_pm_notifier(&cpufreq_pm_notifier_nb);
	if (ret) {
		pr_err("register_pm_notifier failed: %d\n", ret);
		goto err_register_pm_notifier;
	}

	ret = misc_register(&xr_cpufreq_qos);
	if (ret) {
		pr_err("failed to register misc device\n");
		goto err_misc_register;
	}

	if (battery_state)
		limit_request(ADD_REQUEST);

	pr_info("misc device registered\n");

	return 0;

err_misc_register:
	unregister_pm_notifier(&cpufreq_pm_notifier_nb);
err_register_pm_notifier:
	kfree(suspend_cluster_map);
err_suspend_limit_cfg:
	kfree(cluster_map);

	pr_err("qos init failed\n");
	return ret;
}

void xr_cpufreq_qos_exit(void)
{
	unregister_pm_notifier(&cpufreq_pm_notifier_nb);
	misc_deregister(&xr_cpufreq_qos);
	pr_info("misc device unregistered\n");
	limit_request(REMOVE_REQUEST);
	mutex_destroy(&qos_mutex_lock);
	kfree(cluster_map);
	kfree(suspend_cluster_map);
}
