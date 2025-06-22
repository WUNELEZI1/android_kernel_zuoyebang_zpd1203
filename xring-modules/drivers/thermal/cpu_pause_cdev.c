// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "%s:%s " fmt, KBUILD_MODNAME, __func__

#include <linux/module.h>
#include <linux/thermal.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/of_device.h>
#include <linux/suspend.h>
#include <linux/cpumask.h>
#include <soc/xring/walt.h>

enum cpu_pause_levels {
	CDEV_PAUSE_CPU_NONE,
	CDEV_PAUSE_CPU_GROUP,
	CDEV_PAUSE_CPU_MAX,
};

#define MAX_RETRY_CYCLE 5

struct cpu_pause_cdev {
	struct list_head node;
	cpumask_t cpu_mask;
	enum cpu_pause_levels cur_state;
	enum cpu_pause_levels max_state;
	enum cpu_pause_levels req_state;
	struct device_node *np;
	char cdev_name[THERMAL_NAME_LENGTH];
	struct work_struct reg_work;
	struct work_struct exec_work;
	struct thermal_cooling_device *cdev;
};

static DEFINE_MUTEX(cpus_pause_lock);
static LIST_HEAD(cpu_pause_cdev_list);

static enum cpuhp_state cpu_hp_flag;

static int cpu_pause_set_cur_state(struct thermal_cooling_device *cdev,
				   unsigned long level)
{
	struct cpu_pause_cdev *pause_cdev = cdev->devdata;

	if (level >= CDEV_PAUSE_CPU_MAX)
		return -EINVAL;

	mutex_lock(&cpus_pause_lock);

	if (level)
		pause_cdev->req_state = CDEV_PAUSE_CPU_GROUP;
	else
		pause_cdev->req_state = CDEV_PAUSE_CPU_NONE;

	queue_work(system_highpri_wq, &pause_cdev->exec_work);

	mutex_unlock(&cpus_pause_lock);

	return 0;
}

static int cpu_pause_get_cur_state(struct thermal_cooling_device *cdev,
				   unsigned long *level)
{
	struct cpu_pause_cdev *pause_cdev = cdev->devdata;

	*level = pause_cdev->cur_state;

	return 0;
}

static int cpu_pause_get_max_state(struct thermal_cooling_device *cdev,
				   unsigned long *level)
{
	struct cpu_pause_cdev *pause_cdev = cdev->devdata;

	*level = pause_cdev->max_state;

	return 0;
}

static struct thermal_cooling_device_ops cpu_pause_cooling_ops = {
	.get_max_state = cpu_pause_get_max_state,
	.get_cur_state = cpu_pause_get_cur_state,
	.set_cur_state = cpu_pause_set_cur_state,
};

static int cpu_resume_execute(struct cpu_pause_cdev *pause_cdev)
{
	int ret;
	cpumask_t unpause_cpus;

	cpumask_copy(&unpause_cpus, &pause_cdev->cpu_mask);
	pr_debug("cpu unpause:%*pbl\n", cpumask_pr_args(&unpause_cpus));

	mutex_unlock(&cpus_pause_lock);
	ret = walt_resume_cpus(&unpause_cpus, PAUSE_THERMAL);
	mutex_lock(&cpus_pause_lock);
	if (ret != 0)
		pr_err("resuming cpu:%*pbl fail, err:%d\n",
		       cpumask_pr_args(&pause_cdev->cpu_mask), ret);

	return ret;
}

static int cpu_pause_execute(struct cpu_pause_cdev *pause_cdev)
{
	int ret;
	cpumask_t pause_cpus;

	cpumask_copy(&pause_cpus, &pause_cdev->cpu_mask);
	pr_debug("cpu pause:%*pbl\n", cpumask_pr_args(&pause_cdev->cpu_mask));

	mutex_unlock(&cpus_pause_lock);
	ret = walt_pause_cpus(&pause_cpus, PAUSE_THERMAL);
	mutex_lock(&cpus_pause_lock);
	if (ret != 0)
		pr_err("pausing cpu:%*pbl fail, err:%d\n",
		       cpumask_pr_args(&pause_cdev->cpu_mask), ret);

	return ret;
}

static void cpu_pause_execute_cdev(struct work_struct *work)
{
	int ret;
	int retry_cnt = MAX_RETRY_CYCLE;
	struct cpu_pause_cdev *pause_cdev =
		container_of(work, struct cpu_pause_cdev, exec_work);

	mutex_lock(&cpus_pause_lock);

	if (pause_cdev->req_state == pause_cdev->cur_state)
		goto out;
retry:
	if (pause_cdev->req_state == CDEV_PAUSE_CPU_NONE) {
		ret = cpu_resume_execute(pause_cdev);
		if (ret >= 0)
			pause_cdev->cur_state = CDEV_PAUSE_CPU_NONE;
	} else {
		ret = cpu_pause_execute(pause_cdev);
		if (ret >= 0)
			pause_cdev->cur_state = CDEV_PAUSE_CPU_GROUP;
	}

	if (pause_cdev->req_state != pause_cdev->cur_state) {
		pr_err("cpu pause state mismatch: req %u cur %u\n",
			  pause_cdev->req_state, pause_cdev->cur_state);
		retry_cnt--;
		if (retry_cnt >= 0)
			goto retry;
	}
out:
	mutex_unlock(&cpus_pause_lock);
}

static void cpu_pause_register_cdev(struct work_struct *work)
{
	struct cpu_pause_cdev *pause_cdev =
			container_of(work, struct cpu_pause_cdev, reg_work);
	cpumask_t cpus_online;

	cpumask_and(&cpus_online,
			&pause_cdev->cpu_mask,
			cpu_online_mask);
	if (!cpumask_equal(&pause_cdev->cpu_mask, &cpus_online)) {
		pr_err("pause cpumask %*pbl not match cpu_online_mask %*pbl!\n",
			cpumask_pr_args(&pause_cdev->cpu_mask),
			cpumask_pr_args(cpu_online_mask));
		return;
	}

	pause_cdev->cdev = thermal_of_cooling_device_register(
					pause_cdev->np,
					pause_cdev->cdev_name,
					pause_cdev,
					&cpu_pause_cooling_ops);

	if (IS_ERR(pause_cdev->cdev)) {
		pr_err("cpu pause cdev register failed, %s ret:%ld\n",
			pause_cdev->cdev_name, PTR_ERR(pause_cdev->cdev));
		pause_cdev->cdev = NULL;
		return;
	}
	pr_debug("[%s] cdev registered.\n", pause_cdev->cdev_name);
}

static int cpu_pause_startup(unsigned int cpu)
{
	struct cpu_pause_cdev *pause_cdev;
	int ret = 0;

	pr_debug("online entry CPU:%d\n", cpu);

	mutex_lock(&cpus_pause_lock);
	list_for_each_entry(pause_cdev, &cpu_pause_cdev_list, node) {
		if (cpumask_test_cpu(cpu, &pause_cdev->cpu_mask)
			&& !pause_cdev->cdev)
			queue_work(system_highpri_wq,
				   &pause_cdev->reg_work);
	}
	mutex_unlock(&cpus_pause_lock);
	pr_debug("online exit CPU:%d\n", cpu);
	return ret;
}

static int cpu_pause_probe(struct platform_device *pdev)
{
	int ret, cpu;
	struct device_node *subsys_np = NULL;
	struct device *cpu_dev = NULL;
	struct cpu_pause_cdev *pause_cdev = NULL;
	struct device_node *np = pdev->dev.of_node;
	struct of_phandle_iterator opi;
	const char *alias = NULL;

	INIT_LIST_HEAD(&cpu_pause_cdev_list);

	for_each_available_child_of_node(np, subsys_np) {
		pause_cdev = devm_kzalloc(&pdev->dev,
				sizeof(*pause_cdev), GFP_KERNEL);

		if (!pause_cdev) {
			pr_err("kzalloc cpu pause cdev fail:no-mem!\n");
			of_node_put(subsys_np);
			return -ENOMEM;
		}
		of_phandle_iterator_init(&opi, subsys_np, "xring,cpus", NULL, 0);
		while (of_phandle_iterator_next(&opi) == 0) {
			for_each_possible_cpu(cpu) {
				cpu_dev = get_cpu_device(cpu);
				if (cpu_dev && cpu_dev->of_node
						== opi.node) {
					cpumask_set_cpu(cpu, &pause_cdev->cpu_mask);
					break;
				}
			}
		}

		if (cpumask_empty(&pause_cdev->cpu_mask))
			continue;

		ret = of_property_read_string(subsys_np,
				"xring,cdev-alias", &alias);
		if (ret) {
			pr_err("cpu pause lost alias, %s!\n", subsys_np->name);
			continue;
		}

		strscpy(pause_cdev->cdev_name, alias,
				THERMAL_NAME_LENGTH);

		pause_cdev->cur_state = CDEV_PAUSE_CPU_NONE;
		pause_cdev->max_state = CDEV_PAUSE_CPU_GROUP;
		pause_cdev->req_state = CDEV_PAUSE_CPU_NONE;
		pause_cdev->cdev = NULL;
		pause_cdev->np = subsys_np;

		INIT_WORK(&pause_cdev->reg_work, cpu_pause_register_cdev);
		INIT_WORK(&pause_cdev->exec_work, cpu_pause_execute_cdev);
		list_add(&pause_cdev->node, &cpu_pause_cdev_list);
	}

	ret = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN, "cpu pause cdev startup",
				cpu_pause_startup, NULL);
	if (ret < 0)
		return ret;
	cpu_hp_flag = ret;

	return 0;
}

static int cpu_pause_remove(struct platform_device *pdev)
{
	struct cpu_pause_cdev *pause_cdev = NULL;
	struct cpu_pause_cdev *next = NULL;

	if (cpu_hp_flag) {
		cpuhp_remove_state_nocalls(cpu_hp_flag);
		cpu_hp_flag = 0;
	}

	mutex_lock(&cpus_pause_lock);
	list_for_each_entry_safe(pause_cdev, next,
			&cpu_pause_cdev_list, node) {
		if (pause_cdev->cur_state == CDEV_PAUSE_CPU_GROUP) {
			pause_cdev->req_state = CDEV_PAUSE_CPU_NONE;
			queue_work(system_highpri_wq, &pause_cdev->exec_work);
		}

		if (pause_cdev->cdev)
			thermal_cooling_device_unregister(
					pause_cdev->cdev);
		list_del(&pause_cdev->node);
	}

	mutex_unlock(&cpus_pause_lock);

	return 0;
}

static const struct of_device_id cpu_pause_match[] = {
	{ .compatible = "xring,cpu-pause", },
	{},
};

static struct platform_driver cpu_pause_driver = {
	.probe		= cpu_pause_probe,
	.remove		= cpu_pause_remove,
	.driver		= {
		.name		= "xring-cpupause-cdev",
		.of_match_table = cpu_pause_match,
	},
};

int xr_cpu_pause_cdev_init(void)
{
	int ret;

	ret = platform_driver_register(&cpu_pause_driver);
	if (ret)
		pr_err("%s fail\n", __func__);

	return ret;
}

void xr_cpu_pause_cdev_exit(void)
{
	platform_driver_unregister(&cpu_pause_driver);
}
