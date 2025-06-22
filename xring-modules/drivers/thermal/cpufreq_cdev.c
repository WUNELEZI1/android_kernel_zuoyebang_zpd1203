// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "%s:%s " fmt, KBUILD_MODNAME, __func__

#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_qos.h>
#include <linux/slab.h>
#include <linux/thermal.h>
#include <linux/workqueue.h>

struct cpufreq_cdev_device {
	struct list_head node;
	int cpu;
	unsigned long cur_state;
	unsigned long max_state;
	unsigned int *freq_table;
	struct freq_qos_request req;
	char cdev_name[THERMAL_NAME_LENGTH];
	struct cpufreq_policy *policy;
	struct thermal_cooling_device *cdev;
	struct work_struct reg_work;
};

static enum cpuhp_state cpu_hp_flag;
static LIST_HEAD(cpufreq_cdev_list);
static DEFINE_MUTEX(cpufreq_cdev_lock);

static int cpufreq_cdev_get_max_state(struct thermal_cooling_device *cdev,
					unsigned long *state)
{
	struct cpufreq_cdev_device *cpufreq_cdev = cdev->devdata;

	*state = cpufreq_cdev->max_state;
	return 0;
}

static unsigned int state_to_cpufreq(struct cpufreq_cdev_device *cpufreq_cdev,
					   unsigned long state)
{
	if (cpufreq_cdev->freq_table == NULL)
		return UINT_MAX;

	return cpufreq_cdev->freq_table[state];
}

static int cpufreq_cdev_get_state(struct thermal_cooling_device *cdev,
					unsigned long *state)
{
	struct cpufreq_cdev_device *cpufreq_cdev = cdev->devdata;

	*state = cpufreq_cdev->cur_state;
	return 0;
}

static int cpufreq_cdev_set_state(struct thermal_cooling_device *cdev,
					unsigned long state)
{
	struct cpufreq_cdev_device *cpufreq_cdev = cdev->devdata;
	int ret = 0;
	unsigned int freq;

	/* request state should be less than max_level */
	if (state > cpufreq_cdev->max_state)
		return -EINVAL;

	/* check if the old cooling action is same as new cooling action */
	if (state == cpufreq_cdev->cur_state)
		return 0;

	if (!freq_qos_request_active(&cpufreq_cdev->req)) {
		pr_err("cpufreq cdev freq_qos req not init!\n");
		return -EINVAL;
	}

	freq = state_to_cpufreq(cpufreq_cdev, state);
	pr_err("cdev:%s Limit:%u\n", cdev->type, freq);
	ret = freq_qos_update_request(&cpufreq_cdev->req, freq);
	if (ret < 0) {
		pr_err("update freq qos request fail, freq:%u cdev:%s err:%d\n",
			freq, cdev->type, ret);
		return ret;
	}

	cpufreq_cdev->cur_state = state;

	return 0;
}

static struct thermal_cooling_device_ops cpufreq_cdev_ops = {
	.set_cur_state = cpufreq_cdev_set_state,
	.get_cur_state = cpufreq_cdev_get_state,
	.get_max_state = cpufreq_cdev_get_max_state,
};

static void cpufreq_cdev_register(struct work_struct *work)
{
	struct cpufreq_cdev_device *cpufreq_cdev = container_of(work,
			struct cpufreq_cdev_device, reg_work);
	struct cpufreq_policy *policy = NULL;
	unsigned int *freq_table = NULL;
	int opp_count, i, j;

	policy = cpufreq_cpu_get(cpufreq_cdev->cpu);
	if (!policy) {
		pr_err("get cpufreq policy fail:%d\n", cpufreq_cdev->cpu);
		return;
	}
	opp_count = cpufreq_table_count_valid_entries(policy);
	if (opp_count == 0) {
		pr_err("get cpufreq table count fail, cpu:%d\n",
			cpufreq_cdev->cpu);
		goto exit;
	}

	freq_table = kcalloc(opp_count, sizeof(*freq_table), GFP_KERNEL);
	if (!freq_table)
		goto exit;

	for (i = 0; i < opp_count; i++) {
		j = i;
		if (policy->freq_table_sorted == CPUFREQ_TABLE_SORTED_ASCENDING)
			j = opp_count - i - 1;

		freq_table[i] =	policy->freq_table[j].frequency;
	}

	opp_count--;
	cpufreq_cdev->freq_table = freq_table;
	cpufreq_cdev->policy = policy;
	cpufreq_cdev->max_state = opp_count;
	cpufreq_cdev->cur_state = 0;
	/*
	 * Add a new requirement to constrain the frequency point corresponding
	 * to the minimum cooling state zero
	 */
	freq_qos_add_request(&policy->constraints,
			   &cpufreq_cdev->req, FREQ_QOS_MAX,
			   state_to_cpufreq(cpufreq_cdev, 0));
	cpufreq_cdev->cdev = thermal_cooling_device_register(cpufreq_cdev->cdev_name,
						cpufreq_cdev, &cpufreq_cdev_ops);
	if (IS_ERR(cpufreq_cdev->cdev)) {
		pr_err("cpufreq cooling device register failed, cdev_name %s ret %ld\n",
			cpufreq_cdev->cdev_name, PTR_ERR(cpufreq_cdev->cdev));
		goto exit;
	}

	pr_debug("CPUFreq cooling device %s registered\n", cpufreq_cdev->cdev_name);
	return;
exit:
	if (policy)
		cpufreq_cpu_put(policy);
	if (cpufreq_cdev->cdev)
		cpufreq_cdev->cdev = NULL;
	if (freq_qos_request_active(&cpufreq_cdev->req))
		freq_qos_remove_request(&cpufreq_cdev->req);
	kfree(freq_table);
	cpufreq_cdev->freq_table = NULL;
}

static int cpufreq_cdev_hp_startup(unsigned int online_cpu)
{

	struct cpufreq_cdev_device *cpufreq_cdev;

	mutex_lock(&cpufreq_cdev_lock);
	list_for_each_entry(cpufreq_cdev, &cpufreq_cdev_list, node) {
		if (cpufreq_cdev->cpu != online_cpu)
			continue;
		/* cpufreq cooling deivce already register */
		if (cpufreq_cdev->cdev)
			continue;
		queue_work(system_highpri_wq, &cpufreq_cdev->reg_work);
	}
	mutex_unlock(&cpufreq_cdev_lock);

	return 0;
}

static int cpufreq_cdev_probe(struct platform_device *pdev)
{
	struct cpufreq_cdev_device *cpufreq_cdev = NULL;
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct device *cpu_dev = NULL;
	int cpu = 0, ret;
	struct of_phandle_iterator opi;

	of_phandle_iterator_init(&opi, np, "xring,cpus", NULL, 0);
	while (of_phandle_iterator_next(&opi) == 0) {
		cpufreq_cdev = devm_kzalloc(dev, sizeof(*cpufreq_cdev), GFP_KERNEL);
		if (!cpufreq_cdev)
			return -ENOMEM;
		cpufreq_cdev->cpu = -1;
		for_each_possible_cpu(cpu) {
			cpu_dev = get_cpu_device(cpu);
			if (cpu_dev && (cpu_dev->of_node == opi.node)) {
				cpufreq_cdev->cpu = cpu;
				break;
			}
		}
		if (cpufreq_cdev->cpu == -1)
			continue;

		snprintf(cpufreq_cdev->cdev_name, THERMAL_NAME_LENGTH,
			"cpu%d", cpufreq_cdev->cpu);
		INIT_WORK(&cpufreq_cdev->reg_work,
			cpufreq_cdev_register);
		list_add(&cpufreq_cdev->node, &cpufreq_cdev_list);
	}

	ret = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN, "cpufreq cdev startup",
				cpufreq_cdev_hp_startup, NULL);
	if (ret < 0) {
		pr_err("cpuhp setup startup exec fail, %d!\n", ret);
		return ret;
	}
	cpu_hp_flag = ret;

	return 0;
}

static int cpufreq_cdev_remove(struct platform_device *pdev)
{
	struct cpufreq_cdev_device *cpufreq_cdev = NULL;

	mutex_lock(&cpufreq_cdev_lock);
	list_for_each_entry(cpufreq_cdev, &cpufreq_cdev_list, node) {
		if (!cpufreq_cdev->cdev)
			continue;
		thermal_cooling_device_unregister(cpufreq_cdev->cdev);
		if (freq_qos_request_active(&cpufreq_cdev->req))
			freq_qos_remove_request(&cpufreq_cdev->req);
		cpufreq_cdev->cdev = NULL;
		cpufreq_cpu_put(cpufreq_cdev->policy);
		kfree(cpufreq_cdev->freq_table);
		cpufreq_cdev->freq_table = NULL;
	}
	if (cpu_hp_flag) {
		cpuhp_remove_state_nocalls(cpu_hp_flag);
		cpu_hp_flag = 0;
	}
	mutex_unlock(&cpufreq_cdev_lock);
	return 0;
}

static const struct of_device_id cpufreq_cdev_match[] = {
	{.compatible = "xring,cpufreq-cdev"},
	{}
};

static struct platform_driver cpufreq_cdev_driver = {
	.probe          = cpufreq_cdev_probe,
	.remove         = cpufreq_cdev_remove,
	.driver         = {
		.name   = "xring-cpufreq-cdev",
		.of_match_table = cpufreq_cdev_match,
	},
};

int xr_cpufreq_cdev_init(void)
{
	int ret;

	ret = platform_driver_register(&cpufreq_cdev_driver);
	if (ret)
		pr_err("%s fail\n", __func__);

	return ret;
}

void xr_cpufreq_cdev_exit(void)
{
	platform_driver_unregister(&cpufreq_cdev_driver);
}
