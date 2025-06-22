// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)    KBUILD_MODNAME ": " fmt

#include <linux/version.h>
#include <linux/cpu.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/arm-smccc.h>
#include <trace/events/ipi.h>
#include <trace/events/power.h>
#include <trace/hooks/cpuidle.h>
#include <soc/xring/xr_cpuidle.h>
#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

#include "custom.h"

#define CREATE_TRACE_POINTS
#include "trace.h"

#define CLUSTER_MAX	4

int client_depth[NR_CPUS][IDLE_CLIENT_MAX];

DEFINE_PER_CPU(struct custom_config, custom_idle_config);
DEFINE_PER_CPU(struct custom_device, custom_idle_device);

static struct custom_kobj root_kobj;
static unsigned int allowed_depth[NR_CPUS];
static struct cpumask cluster_map[CLUSTER_MAX];
static bool suspend_in_progress;

static DEFINE_MUTEX(lock);

static ssize_t store_residency(struct cpuidle_driver *drv,
			       struct cpuidle_device *dev,
			       int idx, const char *buf, size_t count)
{
	struct custom_device *device = per_cpu_ptr(&custom_idle_device, dev->cpu);
	struct custom_config *config;
	unsigned int val;
	int cpu;

	if (sscanf(buf, "%u\n", &val) != 1)
		return -EINVAL;

	mutex_lock(&lock);
	config = per_cpu_ptr(&custom_idle_config, dev->cpu);
	config->state[idx].residency_us = val;

	/* also set the residency for the CPUs in the same cluster */
	if (device->cluster < CLUSTER_MAX) {
		for_each_cpu(cpu, &cluster_map[device->cluster]) {
			config = per_cpu_ptr(&custom_idle_config, cpu);
			config->state[idx].residency_us = val;
		}
	}
	mutex_unlock(&lock);

	return count;
}

static ssize_t show_residency(struct cpuidle_driver *drv,
			      struct cpuidle_device *dev,
			      int idx, char *buf)
{
	struct custom_config *config = per_cpu_ptr(&custom_idle_config, dev->cpu);

	return sprintf(buf, "%u\n", config->state[idx].residency_us);
}

static ssize_t store_latency(struct cpuidle_driver *drv,
			     struct cpuidle_device *dev,
			     int idx, const char *buf, size_t count)
{
	struct custom_device *device = per_cpu_ptr(&custom_idle_device, dev->cpu);
	struct custom_config *config;
	unsigned int val;
	int cpu;

	if (sscanf(buf, "%u\n", &val) != 1)
		return -EINVAL;


	mutex_lock(&lock);
	config = per_cpu_ptr(&custom_idle_config, dev->cpu);
	config->state[idx].latency_us = val;

	/* also set the latency for the CPUs in the same cluster */
	if (device->cluster < CLUSTER_MAX) {
		for_each_cpu(cpu, &cluster_map[device->cluster]) {
			config = per_cpu_ptr(&custom_idle_config, cpu);
			config->state[idx].latency_us = val;
		}
	}
	mutex_unlock(&lock);

	return count;
}

static ssize_t show_latency(struct cpuidle_driver *drv,
			    struct cpuidle_device *dev,
			    int idx, char *buf)
{
	struct custom_config *config = per_cpu_ptr(&custom_idle_config, dev->cpu);

	return sprintf(buf, "%u\n", config->state[idx].latency_us);
}

static ssize_t show_stat(struct cpuidle_driver *drv,
			 struct cpuidle_device *dev,
			 int idx, char *buf)
{
	struct custom_device *device = per_cpu_ptr(&custom_idle_device, dev->cpu);
	ssize_t count = 0;
	int selected, actual, reason;

	for (selected = 0; selected < drv->state_count; selected++) {
		count += scnprintf(buf + count, PAGE_SIZE - count, "%2u:", selected);

		for (actual = 0; actual < drv->state_count; actual++)
			count += scnprintf(buf + count, PAGE_SIZE - count, " %10u",
				device->stat.state_map[selected][actual]);


		count += scnprintf(buf + count, PAGE_SIZE - count, " (");
		for (reason = 0; reason < REASON_MAX; reason++)
			count += scnprintf(buf + count, PAGE_SIZE - count, " %10u",
				device->stat.reason_cnt[selected][reason]);
		count += scnprintf(buf + count, PAGE_SIZE - count, ")\n");
	}

	return count;
}

static ssize_t store_stat(struct cpuidle_driver *drv,
			  struct cpuidle_device *dev,
			  int idx, const char *buf, size_t count)
{
	custom_clear_stat(dev->cpu);

	return count;
}

static void __cpu_set_ret_mode(void *arg)
{
	struct custom_device *device = (struct custom_device *)arg;
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FIC_BL31_CPU_SET_RET_MODE,
		device->ret.vpu, device->ret.wfe, device->ret.wfi, 0, 0, 0, 0,
		&res);
	if (res.a0 != 0)
		pr_err("failed to set retention mode!\n");
}

static void __cpu_get_ret_mode(void *arg)
{
	struct custom_device *device = (struct custom_device *)arg;
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FIC_BL31_CPU_GET_RET_MODE, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("failed to get retention mode!\n");

	device->ret.vpu = res.a1;
	device->ret.wfe = res.a2;
	device->ret.wfi = res.a3;
}

static ssize_t show_ret_mode(struct cpuidle_driver *drv,
			struct cpuidle_device *dev,
			int idx, char *buf)
{
	struct custom_device *device = per_cpu_ptr(&custom_idle_device, dev->cpu);

	smp_call_function_single(dev->cpu, __cpu_get_ret_mode, device, true);

	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n",
			device->ret.vpu, device->ret.wfe, device->ret.wfi);
}

static ssize_t store_ret_mode(struct cpuidle_driver *drv,
			struct cpuidle_device *dev,
			int idx, const char *buf, size_t count)
{
	struct custom_device *device = per_cpu_ptr(&custom_idle_device, dev->cpu);
	unsigned int vpu, wfe, wfi;

	if (sscanf(buf, "%u %u %u\n", &vpu, &wfe, &wfi) != 3)
		return -EINVAL;

	device->ret.vpu = vpu;
	device->ret.wfe = wfe;
	device->ret.wfi = wfi;

	smp_call_function_single(dev->cpu, __cpu_set_ret_mode, device, true);

	return count;
}

static ssize_t store_allowed_depth(struct cpuidle_driver *drv,
					  struct cpuidle_device *dev,
					  int idx, const char *buf, size_t count)
{
	unsigned int val;

	if (sscanf(buf, "%u\n", &val) != 1)
		return -EINVAL;

	client_depth[dev->cpu][IDLE_CLIENT_USER] = val;

	return count;
}

static ssize_t show_allowed_depth(struct cpuidle_driver *drv,
					 struct cpuidle_device *dev,
					 int idx, char *buf)
{
	return sprintf(buf, "%u\n", allowed_depth[dev->cpu]);
}


custom_idle_attr_rw(residency);
custom_idle_attr_rw(latency);
custom_idle_attr_rw(stat);
custom_idle_attr_rw(ret_mode);
custom_idle_attr_rw(allowed_depth);

static struct attribute *custom_state_attrs[] = {
	&attr_custom_residency.attr,
	&attr_custom_latency.attr,
	NULL
};
ATTRIBUTE_GROUPS(custom_state);

static struct attribute *custom_cpu_attrs[] = {
	&attr_custom_stat.attr,
	&attr_custom_ret_mode.attr,
	&attr_custom_allowed_depth.attr,
	NULL
};
ATTRIBUTE_GROUPS(custom_cpu);

static struct attribute *custom_attrs[] = {
	NULL
};
ATTRIBUTE_GROUPS(custom);

#define to_state_dev(k) container_of(k, struct custom_kobj, kobj)->dev
#define to_state_drv(k) container_of(k, struct custom_kobj, kobj)->drv
#define to_state_idx(k) container_of(k, struct custom_kobj, kobj)->idx
#define to_cattr(a) container_of(a, struct custom_idle_attr, attr)

static ssize_t show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	struct cpuidle_device *dev = to_state_dev(kobj);
	struct cpuidle_driver *drv = to_state_drv(kobj);
	int idx = to_state_idx(kobj);
	struct custom_idle_attr *cattr = to_cattr(attr);
	ssize_t ret = -EIO;

	if (cattr->show)
		ret = cattr->show(drv, dev, idx, buf);

	return ret;
}

static ssize_t store(struct kobject *kobj, struct attribute *attr,
		     const char *buf, size_t count)
{
	struct cpuidle_device *dev = to_state_dev(kobj);
	struct cpuidle_driver *drv = to_state_drv(kobj);
	int idx = to_state_idx(kobj);
	struct custom_idle_attr *cattr = to_cattr(attr);
	ssize_t ret = -EIO;

	if (cattr->store)
		ret = cattr->store(drv, dev, idx, buf, count);

	return ret;
}

static const struct sysfs_ops sysfs_ops = {
	.show	= show,
	.store	= store,
};

static struct kobj_type ktype_custom_state = {
	.sysfs_ops	= &sysfs_ops,
	.default_groups = custom_state_groups,
};

static struct kobj_type ktype_custom_cpu = {
	.sysfs_ops	= &sysfs_ops,
	.default_groups = custom_cpu_groups,
};

static struct kobj_type ktype_custom = {
	.sysfs_ops	= &sysfs_ops,
	.default_groups = custom_groups,
};

/**
 * custom_create_state_attr - create the attributes for each of the cpu idle states
 * @drv: cpu idle driver
 * @dev: cpu idle device
 * @cpu_group: additional cpu attributes if a governor needs
 * @state_group: additional state attributes if a governor needs
 */
int custom_create_state_attr(struct cpuidle_driver *drv,
			     struct cpuidle_device *dev,
			     const struct attribute_group **cpu_group,
			     const struct attribute_group **state_group)
{
	struct custom_config *config = per_cpu_ptr(&custom_idle_config, dev->cpu);
	struct custom_kobj *st_kobj;
	int ret;
	int i;

	/* cpu attributes */
	config->cpu_kobj = kzalloc(sizeof(struct custom_kobj), GFP_KERNEL);
	if (config->cpu_kobj == NULL)
		return -ENOMEM;

	config->cpu_kobj->dev = dev;
	config->cpu_kobj->drv = drv;

	ret = kobject_init_and_add(&config->cpu_kobj->kobj, &ktype_custom_cpu,
				   &root_kobj.kobj, "cpu%u", dev->cpu);
	if (ret)
		goto cpu_release;

	if (cpu_group) {
		ret = sysfs_create_groups(&config->cpu_kobj->kobj, cpu_group);
		if (ret)
			goto cpu_release;
	}

	/* state attributes */
	allowed_depth[dev->cpu] = drv->state_count;
	for (i = 0; i < IDLE_CLIENT_MAX; i++)
		client_depth[dev->cpu][i] = drv->state_count;

	for (i = 0; i < drv->state_count; i++) {
		st_kobj = kzalloc(sizeof(struct custom_kobj), GFP_KERNEL);
		if (!st_kobj) {
			ret = -ENOMEM;
			goto state_release;
		}

		st_kobj->idx = i;
		st_kobj->dev = dev;
		st_kobj->drv = drv;

		ret = kobject_init_and_add(&st_kobj->kobj, &ktype_custom_state,
					   &config->cpu_kobj->kobj, "state%u", i);
		if (ret) {
			kobject_put(&st_kobj->kobj);
			kfree(st_kobj);
			goto state_release;
		}

		// reset state residency and latency to the default
		config->state[i].residency_us = drv->states[i].target_residency;
		config->state[i].latency_us = drv->states[i].exit_latency;
		config->st_kobj[i] = st_kobj;

		if (state_group) {
			ret = sysfs_create_groups(&st_kobj->kobj, state_group);
			if (ret) {
				kobject_put(&st_kobj->kobj);
				kfree(st_kobj);
				config->st_kobj[i] = NULL;
				goto state_release;
			}
		}
	}

	return 0;

state_release:
	for (i = i - 1; i >= 0; i--) {
		kobject_put(&config->st_kobj[i]->kobj);
		kfree(config->st_kobj[i]);
		config->st_kobj[i] = NULL;
	}
cpu_release:
	kobject_put(&config->cpu_kobj->kobj);
	kfree(config->cpu_kobj);
	config->cpu_kobj = NULL;

	return ret;
}

/**
 * custom_remove_state_attr - remove the attributes created for each cpu
 * @drv: cpu idle driver
 * @dev: cpu idle device
 * @cpu_group: additional cpu attributes if a governor needs
 * @state_group: additional state attributes if a governor needs
 */
void custom_remove_state_attr(struct cpuidle_driver *drv,
			      struct cpuidle_device *dev,
			      const struct attribute_group **cpu_group,
			      const struct attribute_group **state_group)
{
	struct custom_config *config = per_cpu_ptr(&custom_idle_config, dev->cpu);
	int i;

	for (i = 0; i < drv->state_count; i++) {
		if (config->st_kobj[i]) {
			sysfs_remove_groups(&config->st_kobj[i]->kobj, state_group);
			kobject_put(&config->st_kobj[i]->kobj);
			kfree(config->st_kobj[i]);
			config->st_kobj[i] = NULL;
		}
	}

	if (config->cpu_kobj) {
		sysfs_remove_groups(&config->cpu_kobj->kobj, cpu_group);
		kobject_put(&config->cpu_kobj->kobj);
		kfree(config->cpu_kobj);
		config->cpu_kobj = NULL;
	}
}

/**
 * custom_create_root_attr - create attributes in the root directory
 * @groups: group of attributes
 */
int custom_create_root_attr(const struct attribute_group **groups)
{
	return sysfs_create_groups(&root_kobj.kobj, groups);
}

/**
 * custom_remove_root_attr - remove the root attributes created
 * @groups: group of attributes
 */
void custom_remove_root_attr(const struct attribute_group **groups)
{
	sysfs_remove_groups(&root_kobj.kobj, groups);
}

int custom_get_allowed_depth(struct cpuidle_driver *drv,
			     struct cpuidle_device *dev)
{
	int depth = drv->state_count;
	int cpu = dev->cpu;

	for (int i = 0; i < IDLE_CLIENT_MAX; i++)
		depth = min(depth, client_depth[cpu][i]);

	allowed_depth[cpu] = depth;

	return depth;
}

void cpuidle_set_allowed_depth(unsigned int cpu, unsigned int depth, int client)
{
	if (unlikely(client >= IDLE_CLIENT_MAX)) {
		pr_err("invalid client %d\n", client);
		return;
	}

	if (cpu < num_possible_cpus())
		client_depth[cpu][client] = depth;
}
EXPORT_SYMBOL_GPL(cpuidle_set_allowed_depth);

/**
 * custom_collect_stat - collect the idle statistics
 * @drv: cpuidle driver
 * @dev: cpuidle device
 * @selected: the idle state selected
 * @duration_us: the actual idle length in micro seconds
 */
void custom_collect_stat(struct cpuidle_driver *drv,
			 struct cpuidle_device *dev,
			 int selected, int duration_us)
{
	struct custom_config *config = per_cpu_ptr(&custom_idle_config, dev->cpu);
	struct custom_device *device = per_cpu_ptr(&custom_idle_device, dev->cpu);
	int i, actual = 0;

	/* find out the idle depth matches the actual sleep length */
	for (i = 0; i < drv->state_count; i++) {
		if (duration_us < config->state[i].residency_us)
			break;

		actual = i;
	}

	/* record */
	if (selected < drv->state_count) {
		device->stat.state_map[selected][actual]++;
		if (device->reason < REASON_MAX)
			device->stat.reason_cnt[selected][device->reason]++;
	}

	trace_idle_stat(dev->cpu, selected, actual, device->reason);
}

/**
 * custom_clear_stat - clear the idle statistics of a given cpu
 * @cpu: cpu id
 */
void custom_clear_stat(int cpu)
{
	struct custom_device *device = per_cpu_ptr(&custom_idle_device, cpu);

	memset(&device->stat, 0, sizeof(device->stat));
}

/**
 * custom_forced_state - check if the last idle state selected is determined
 * by the constraints.
 */
bool custom_forced_state(void)
{
	struct custom_device *device = this_cpu_ptr(&custom_idle_device);
	bool forced = false;

	/**
	 * interrupts can still wake up CPU even if it's halted,
	 * governor need to find the interrupt pattern as the normal cases
	 */
	if (device->reason != PREDICT &&
	    device->reason != ORIGIN &&
	    device->reason != HALTED)
		forced = true;

	return forced;
}

/**
 * custom_set_reason - sets the reason to select the idle state. governor can
 * later check the value to tell if the last idle state is still its own
 * decision.
 * @reason: reason to select the idle state
 */
void custom_set_reason(enum reason reason)
{
	struct custom_device *device = this_cpu_ptr(&custom_idle_device);

	device->reason = reason;
}

/**
 * custom_suspend_trace - function to track the entrance and exit of system suspend
 */
static void custom_suspend_trace(void *unused, const char *action,
				 int event, bool start)
{
	if (start && !strcmp("dpm_suspend_late", action)) {
		suspend_in_progress = true;

		return;
	}

	if (!start && !strcmp("dpm_resume_early", action))
		suspend_in_progress = false;
}

/**
 * custom_ipi_raise - function to track the occurance of IPI events
 */
static void custom_ipi_raise(void *ignore, const struct cpumask *mask, const char *unused)
{
	int cpu;
	struct custom_device *custom;

	if (suspend_in_progress)
		return;

	for_each_cpu_and(cpu, mask, cpu_possible_mask) {
		custom = per_cpu_ptr(&custom_idle_device, cpu);

		atomic_set(&custom->ipi_pending, true);
	}
}

/**
 * custom_ipi_entry - function to track the IPI entry
 */
static void custom_ipi_entry(void *ignore, const char *unused)
{
	struct custom_device *custom;

	if (suspend_in_progress)
		return;

	custom = this_cpu_ptr(&custom_idle_device);

	atomic_set(&custom->ipi_pending, false);
}

/**
 * custom_idle_enter - function to track the entrance of idle events.
 * it will over-write the state to enter if necessary
 */
static void custom_idle_enter(void *unused, int *state, struct cpuidle_device *dev)
{
	struct custom_device *device = this_cpu_ptr(&custom_idle_device);

	if (*state == 0)
		return;

	/* Restrict to WFI state if there is an IPI pending on current CPU */
	if (atomic_read(&device->ipi_pending)) {
		*state = 0;
		device->reason = IPI;
	}
}

static int custom_init(void)
{
	int cpu;
	int cluster;
	struct custom_device *dev;
	struct custom_config *cfg;
	struct device *root;
	int ret;

	root = bus_get_dev_root(&cpu_subsys);
	if (!root)
		return -ENODEV;

	ret = kobject_init_and_add(&root_kobj.kobj, &ktype_custom,
				   &root->kobj, "xring_idle");
	if (ret)
		return ret;

	put_device(root);

	for_each_possible_cpu(cpu) {
		dev = per_cpu_ptr(&custom_idle_device, cpu);
		cfg = per_cpu_ptr(&custom_idle_config, cpu);

		memset(dev, 0, sizeof(struct custom_device));
		memset(cfg, 0, sizeof(struct custom_config));

		cluster = cpu_topology[cpu].cluster_id;
		if (cluster < 0)
			return -EINVAL;

		if (cluster < CLUSTER_MAX) {
			cpumask_set_cpu(cpu, &cluster_map[cluster]);
			dev->cluster = cluster;
		}
	}

	ret = register_trace_suspend_resume(custom_suspend_trace, NULL);
	if (ret)
		return ret;

	ret = register_trace_ipi_raise(custom_ipi_raise, NULL);
	if (ret)
		return ret;

	ret = register_trace_ipi_entry(custom_ipi_entry, NULL);
	if (ret)
		return ret;

	ret = register_trace_prio_android_vh_cpu_idle_enter(custom_idle_enter, NULL, INT_MIN);
	if (ret)
		return ret;

	return 0;
}

static void custom_exit(void)
{
	kobject_put(&root_kobj.kobj);

	unregister_trace_suspend_resume(custom_suspend_trace, NULL);
	unregister_trace_ipi_raise(custom_ipi_raise, NULL);
	unregister_trace_ipi_entry(custom_ipi_entry, NULL);
	unregister_trace_android_vh_cpu_idle_enter(custom_idle_enter, NULL);
}

static int __init cpuidle_init(void)
{
	int ret;

	ret = custom_init();
	if (ret)
		goto error;

#if IS_ENABLED(CONFIG_XRING_CPUIDLE_MENU_GOVERNOR)
	ret = xr_menu_governor_init();
	if (ret)
		goto error;
#endif

	return 0;

error:
	custom_exit();

	return ret;
}
module_init(cpuidle_init);

MODULE_DESCRIPTION("XRing CPU Idle Governor");
MODULE_LICENSE("GPL");
#if IS_ENABLED(CONFIG_XRING_SCHED_WALT)
MODULE_SOFTDEP("pre: sched-walt");
#endif
