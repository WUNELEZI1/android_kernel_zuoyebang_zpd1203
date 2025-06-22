// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "%s:%s " fmt, KBUILD_MODNAME, __func__

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/device.h>

enum {
	CDEV_HOTPLUG_CPU_ONLINE,
	CDEV_HOTPLUG_CPU_OFFLINE,
	CDEV_HOTPLUG_CPU_MAX,
};

struct cpu_hp_cdev {
	struct list_head node;
	int cpu;
	/* cpu hp level, 0: cpu need online; 1: cpu need offline */
	unsigned int cur_state;
	unsigned int max_state;
	/* cpu state, false: cpu offline; true: cpu online */
	bool cpu_state;
	struct device_node *np;
	char cdev_name[THERMAL_NAME_LENGTH];
	struct work_struct reg_work;
	struct thermal_cooling_device *cdev;
};
static DEFINE_MUTEX(cpu_hp_lock);
static LIST_HEAD(cpu_hp_cdev_list);
static enum cpuhp_state cpu_hp_flag;
static void cpu_hotplug_execute_cdev(struct cpu_hp_cdev *hp_cdev);

static int cpu_hp_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	struct cpu_hp_cdev *hp_cdev = cdev->devdata;

	/* request state should be less than max_level */
	if (state > hp_cdev->max_state || hp_cdev->cpu == -1)
		return -EINVAL;

	/* check if the old cooling action is same as new cooling action */
	if (state == hp_cdev->cur_state)
		return 0;

	mutex_lock(&cpu_hp_lock);
	hp_cdev->cur_state = state;
	mutex_unlock(&cpu_hp_lock);
	cpu_hotplug_execute_cdev(hp_cdev);

	return 0;
}

static int cpu_hp_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	struct cpu_hp_cdev *hp_cdev = cdev->devdata;

	mutex_lock(&cpu_hp_lock);
	*state = hp_cdev->cur_state;
	mutex_unlock(&cpu_hp_lock);

	return 0;
}

static int cpu_hp_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	struct cpu_hp_cdev *hp_cdev = cdev->devdata;

	*state = hp_cdev->max_state;
	return 0;
}

static struct thermal_cooling_device_ops cpu_hot_cooling_ops = {
	.get_max_state = cpu_hp_get_max_state,
	.get_cur_state = cpu_hp_get_cur_state,
	.set_cur_state = cpu_hp_set_cur_state,
};

static void cpu_hotplug_execute_cdev(struct cpu_hp_cdev *hp_cdev)
{
	int ret = 0, cpu = 0;

	mutex_lock(&cpu_hp_lock);
	cpu = hp_cdev->cpu;

	if (hp_cdev->cur_state == CDEV_HOTPLUG_CPU_OFFLINE) {
		/* exit when cpu already offline */
		if (hp_cdev->cpu_state == false)
			goto out;
		mutex_unlock(&cpu_hp_lock);
		ret = remove_cpu(cpu);
		mutex_lock(&cpu_hp_lock);
		if (ret < 0)
			pr_err("cpuhp cpu offline error, cpu%d ret %d\n", cpu, ret);
		else
			hp_cdev->cpu_state = false;
	} else {
		/* exit when cpu already online */
		if (hp_cdev->cpu_state == true)
			goto out;
		mutex_unlock(&cpu_hp_lock);
		ret = add_cpu(cpu);
		mutex_lock(&cpu_hp_lock);
		if (ret < 0)
			pr_err("cpuhp cpu online error, cpu%d ret %d\n", cpu, ret);
		else
			hp_cdev->cpu_state = true;
	}
out:
	mutex_unlock(&cpu_hp_lock);
}

static void cpu_hotplug_register_cdev(struct work_struct *work)
{
	struct cpu_hp_cdev *hp_cdev =
			container_of(work, struct cpu_hp_cdev, reg_work);

	hp_cdev->cdev = thermal_of_cooling_device_register(
					hp_cdev->np,
					hp_cdev->cdev_name,
					hp_cdev,
					&cpu_hot_cooling_ops);
	if (IS_ERR(hp_cdev->cdev)) {
		pr_err("cpuhp cdev register failed, %s ret:%ld\n",
			hp_cdev->cdev_name, PTR_ERR(hp_cdev->cdev));
		hp_cdev->cdev = NULL;
		return;
	}
	pr_debug("[%s] cdev registered.\n", hp_cdev->cdev_name);
}

static int cpu_hotplug_startup(unsigned int cpu)
{
	struct cpu_hp_cdev *hp_cdev = NULL;
	int ret = 0;

	mutex_lock(&cpu_hp_lock);
	list_for_each_entry(hp_cdev, &cpu_hp_cdev_list, node) {
		if (cpu != hp_cdev->cpu)
			continue;

		if (hp_cdev->cdev) {
			/* report an error if cpuhp cur state is offline when cpu startup */
			if (hp_cdev->cur_state == CDEV_HOTPLUG_CPU_OFFLINE) {
				pr_err("cpuhp state not match actual state:%d\n",
					cpu);
				ret = NOTIFY_BAD;
			}
		} else {
			queue_work(system_highpri_wq, &hp_cdev->reg_work);
		}

		break;
	}
	mutex_unlock(&cpu_hp_lock);

	return ret;
}

static int cpu_hotplug_probe(struct platform_device *pdev)
{
	int ret, cpu = 0;
	struct device_node *dev_phandle = NULL;
	struct device_node *subsys_np = NULL;
	struct device *cpu_dev = NULL;
	struct cpu_hp_cdev *hp_cdev = NULL;
	struct device_node *np = pdev->dev.of_node;
	const char *alias = NULL;

	INIT_LIST_HEAD(&cpu_hp_cdev_list);
	for_each_available_child_of_node(np, subsys_np) {
		hp_cdev = devm_kzalloc(&pdev->dev,
				sizeof(*hp_cdev), GFP_KERNEL);
		if (!hp_cdev) {
			of_node_put(subsys_np);
			return -ENOMEM;
		}
		hp_cdev->cpu = -1;
		hp_cdev->cur_state = CDEV_HOTPLUG_CPU_ONLINE;
		hp_cdev->max_state = CDEV_HOTPLUG_CPU_OFFLINE;
		hp_cdev->cpu_state = true;
		hp_cdev->cdev = NULL;
		hp_cdev->np = subsys_np;

		dev_phandle = of_parse_phandle(subsys_np, "xring,cpu", 0);
		for_each_possible_cpu(cpu) {
			cpu_dev = get_cpu_device(cpu);
			if (cpu_dev && cpu_dev->of_node == dev_phandle) {
				hp_cdev->cpu = cpu;
				break;
			}
		}
		if (hp_cdev->cpu == -1)
			continue;

		ret = of_property_read_string(subsys_np,
				"xring,cdev-alias", &alias);
		if (ret)
			snprintf(hp_cdev->cdev_name, THERMAL_NAME_LENGTH, "cpu-hotplug%d",
					hp_cdev->cpu);
		else
			strscpy(hp_cdev->cdev_name, alias,
					THERMAL_NAME_LENGTH);
		INIT_WORK(&hp_cdev->reg_work,
				cpu_hotplug_register_cdev);
		list_add(&hp_cdev->node, &cpu_hp_cdev_list);
	}

	ret = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN, "cpu hotplug cdev startup",
				cpu_hotplug_startup, NULL);
	if (ret < 0)
		return ret;
	cpu_hp_flag = ret;

	return 0;
}

static int cpu_hotplug_remove(struct platform_device *pdev)
{
	struct cpu_hp_cdev *hp_cdev = NULL;
	struct cpu_hp_cdev *next = NULL;
	int ret = 0;

	if (cpu_hp_flag) {
		cpuhp_remove_state_nocalls(cpu_hp_flag);
		cpu_hp_flag = 0;
	}

	mutex_lock(&cpu_hp_lock);
	list_for_each_entry_safe(hp_cdev, next, &cpu_hp_cdev_list, node) {
		if (!hp_cdev->cdev) {
			list_del(&hp_cdev->node);
			continue;
		}
		thermal_cooling_device_unregister(hp_cdev->cdev);
		if (hp_cdev->cur_state == CDEV_HOTPLUG_CPU_OFFLINE) {
			hp_cdev->cur_state = CDEV_HOTPLUG_CPU_ONLINE;
			ret = add_cpu(hp_cdev->cpu);
			if (ret)
				pr_err("cpuhp cpu online fail, cpu%d ret %d\n",
						hp_cdev->cpu, ret);
		}
		list_del(&hp_cdev->node);
	}
	mutex_unlock(&cpu_hp_lock);

	return 0;
}

static const struct of_device_id cpu_hotplug_match[] = {
	{ .compatible = "xring,cpu-hotplug", },
	{},
};

static struct platform_driver cpu_hotplug_driver = {
	.probe		= cpu_hotplug_probe,
	.remove         = cpu_hotplug_remove,
	.driver		= {
		.name = "xring-cpu-hotplug-cdev",
		.of_match_table = cpu_hotplug_match,
	},
};

int xr_cpu_hotplug_cdev_init(void)
{
	int ret;

	ret = platform_driver_register(&cpu_hotplug_driver);
	if (ret)
		pr_err("%s fail\n", __func__);

	return ret;
}

void xr_cpu_hotplug_cdev_exit(void)
{
	platform_driver_unregister(&cpu_hotplug_driver);
}
