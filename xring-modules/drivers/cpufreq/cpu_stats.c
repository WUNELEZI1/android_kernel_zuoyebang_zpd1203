// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/spinlock.h>
#include <linux/threads.h>
#include <linux/device.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <asm/string.h>
#include <linux/module.h>
#include <linux/sched/cpufreq.h>
#include <linux/cpufreq.h>
#include <linux/cpuidle.h>
#include <linux/cpuhotplug.h>
#include <linux/sched/clock.h>
#include <trace/hooks/cpuidle.h>
#include <trace/hooks/cpufreq.h>
#include <linux/arm-smccc.h>
#include <linux/units.h>
#include <soc/xring/atf_shmem.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h>
#include <linux/minmax.h>

#define TAG	"xring cpu time stats: "

#define TIME_IN_IDLE_SIZE(x, y)		((x) * (y))
#define IDLE_IN_FREQ_SIZE(x)		x
#define TIME_IN_BUSY_SIZE(x)		x
#define OFFLINE_TIME_SIZE(x)		x
#define TMP_IDLE_TIME_SIZE(x)		x
#define TOTAL_STATE_TIME_SIZE(y)	y
#define FREQ_TABLE_SIZE(x)		x

#define TEN_MS_PER_S	100

struct cpu_stats {
	int last_freq;
	int last_state;
	unsigned int freq_num;
	unsigned int cpu;
	unsigned int is_idle;
	u64 last_state_time;
	u64 total_idle_time;
	u64 total_busy_time;
	u64 total_offline_time;
	u64 *time_in_idle;
	u64 *idle_time_in_freq;
	u64 *time_in_busy;
	u64 *offline_time;
	u64 *total_state_time;
	u64 *tmp_idle;
	unsigned int *freq_table;

	/*
	 * prevents CPU frequency transition and entry and exit of idle
	 * concurrent update time statistics.
	 */
	spinlock_t stats_lock;

	unsigned int reset_pending;
	u64 reset_time;
};

/* From native module of android kernel 5.15 */
struct cpufreq_stats {
	unsigned int total_trans;
	unsigned long long last_time;
	unsigned int max_state;
	unsigned int state_num;
	unsigned int last_index;
	u64 *time_in_state;
	unsigned int *freq_table;
	unsigned int *trans_table;

	/* Deferred reset */
	unsigned int reset_pending;
	unsigned long long reset_time;
};

static struct cpu_stats cpu_stats_data[NR_CPUS];
static int state_count;
static enum cpuhp_state hp_online;

static int cpu_stats_create_table(struct cpu_stats *stats, unsigned int cpu);

static void cpu_stats_reset_table(struct cpu_stats *stats)
{
	memset(stats->time_in_idle, 0, stats->freq_num * state_count * sizeof(u64));
	memset(stats->idle_time_in_freq, 0, stats->freq_num * sizeof(u64));
	memset(stats->time_in_busy, 0, stats->freq_num * sizeof(u64));
	memset(stats->offline_time, 0, stats->freq_num * sizeof(u64));
	memset(stats->total_state_time, 0, state_count * sizeof(u64));
	memset(stats->tmp_idle, 0, stats->freq_num * sizeof(u64));
	stats->total_idle_time = 0;
	stats->total_busy_time = 0;
	stats->total_offline_time = 0;
	stats->last_state_time = READ_ONCE(stats->reset_time);

	WRITE_ONCE(stats->reset_pending, 0);

	/* do memory barrier */
	smp_rmb();
}

static void cpu_stats_update(struct cpu_stats *stats,
				 u64 time)
{
	u64 cur_time, delta_time, tmp_time;
	int freq_index, state_index, i = 0;
	unsigned int is_idle;

	if (unlikely(READ_ONCE(stats->reset_pending)))
		cpu_stats_reset_table(stats);

	cur_time = local_clock();
	delta_time = cur_time - time;
	freq_index = READ_ONCE(stats->last_freq);
	state_index = READ_ONCE(stats->last_state);
	is_idle = READ_ONCE(stats->is_idle);

	if (is_idle == 1) {
		stats->tmp_idle[freq_index] += delta_time;
	} else if (is_idle >= 2) {
		stats->offline_time[freq_index] += delta_time;
		stats->idle_time_in_freq[freq_index] += delta_time;
		stats->total_offline_time += delta_time;
		stats->total_idle_time += delta_time;
	} else {
		if (state_index >= 0) {
			stats->time_in_idle[freq_index * state_count +
					state_index] += delta_time;
			stats->idle_time_in_freq[freq_index] += delta_time;
			stats->total_state_time[state_index] += delta_time;
			stats->total_idle_time += delta_time;
			for (i = 0; i < stats->freq_num; i++) {
				tmp_time = READ_ONCE(stats->tmp_idle[i]);
				stats->time_in_idle[i * state_count +
					state_index] += tmp_time;
				stats->idle_time_in_freq[freq_index] += tmp_time;
				stats->total_state_time[state_index] += tmp_time;
				stats->total_idle_time += tmp_time;
				stats->tmp_idle[i] = 0;
			}
		} else {
			stats->time_in_busy[freq_index] += delta_time;
			stats->total_busy_time += delta_time;
		}
	}

	stats->last_state_time = cur_time;
}

static int freq_table_get_index(struct cpufreq_stats *stats, unsigned int freq)
{
	int index;

	for (index = 0; index < stats->max_state; index++)
		if (stats->freq_table[index] == freq)
			return index;
	return -1;
}

static void android_rvh_cpufreq_transition(void *unused, struct cpufreq_policy *policy)
{
	unsigned int ret, cpu = 0;
	int freq_index = freq_table_get_index(policy->stats, policy->cur);
	unsigned long flags;

	for_each_cpu(cpu, policy->related_cpus) {
		if (!cpu_stats_data[cpu].time_in_idle) {
			if (cpufreq_cpu_get_raw(cpu)) {
				spin_lock_init(&cpu_stats_data[cpu].stats_lock);
				ret = cpu_stats_create_table(&cpu_stats_data[cpu], cpu);
			} else {
				continue;
			}
		}

		spin_lock_irqsave(&cpu_stats_data[cpu].stats_lock, flags);

		cpu_stats_update(&cpu_stats_data[cpu],
				  cpu_stats_data[cpu].last_state_time);

		if (freq_index < cpu_stats_data[cpu].freq_num)
			WRITE_ONCE(cpu_stats_data[cpu].last_freq, freq_index);
		else
			WRITE_ONCE(cpu_stats_data[cpu].last_freq, cpu_stats_data[cpu].freq_num - 1);

		spin_unlock_irqrestore(&cpu_stats_data[cpu].stats_lock, flags);
	}
}

static void android_vh_cpu_idle_enter(void *unused, int *state, struct cpuidle_device *dev)
{
	unsigned int cpu = dev->cpu;
	unsigned long flags;

	if (!cpu_stats_data[cpu].time_in_idle)
		return;

	spin_lock_irqsave(&cpu_stats_data[cpu].stats_lock, flags);

	if (READ_ONCE(cpu_stats_data[cpu].is_idle) < 2) {
		/* do memory barrier */
		smp_wmb();
		cpu_stats_update(&cpu_stats_data[cpu], cpu_stats_data[cpu].last_state_time);
		WRITE_ONCE(cpu_stats_data[cpu].is_idle, 1);
	}

	spin_unlock_irqrestore(&cpu_stats_data[cpu].stats_lock, flags);
}

static void android_vh_cpu_idle_exit(void *unused, int state, struct cpuidle_device *dev)
{
	unsigned int cpu = dev->cpu;
	unsigned long flags;

	if (!cpu_stats_data[cpu].time_in_idle)
		return;

	spin_lock_irqsave(&cpu_stats_data[cpu].stats_lock, flags);

	if (READ_ONCE(cpu_stats_data[cpu].is_idle) < 2) {
		/* do memory barrier */
		smp_wmb();
		if (state < state_count)
			WRITE_ONCE(cpu_stats_data[cpu].last_state, state);
		else
			WRITE_ONCE(cpu_stats_data[cpu].last_state, state_count - 1);

		WRITE_ONCE(cpu_stats_data[cpu].is_idle, 0);

		/* do memory barrier */
		smp_rmb();

		cpu_stats_update(&cpu_stats_data[cpu], cpu_stats_data[cpu].last_state_time);
		WRITE_ONCE(cpu_stats_data[cpu].last_state, -1);
	}

	spin_unlock_irqrestore(&cpu_stats_data[cpu].stats_lock, flags);
}

static int cpuhp_stats_online(unsigned int cpu)
{
	unsigned long flags;

	if (!cpu_stats_data[cpu].time_in_idle)
		return 0;

	spin_lock_irqsave(&cpu_stats_data[cpu].stats_lock, flags);

	cpu_stats_update(&cpu_stats_data[cpu], cpu_stats_data[cpu].last_state_time);
	WRITE_ONCE(cpu_stats_data[cpu].is_idle, 0);
	WRITE_ONCE(cpu_stats_data[cpu].last_state, -1);

	spin_unlock_irqrestore(&cpu_stats_data[cpu].stats_lock, flags);

	return 0;
}

static int cpuhp_stats_offline(unsigned int cpu)
{
	unsigned long flags;

	if (!cpu_stats_data[cpu].time_in_idle)
		return 0;

	spin_lock_irqsave(&cpu_stats_data[cpu].stats_lock, flags);

	cpu_stats_update(&cpu_stats_data[cpu], cpu_stats_data[cpu].last_state_time);
	WRITE_ONCE(cpu_stats_data[cpu].is_idle, 2);

	spin_unlock_irqrestore(&cpu_stats_data[cpu].stats_lock, flags);

	return 0;
}

static int cpu_vh_time_count_register(void)
{
	int ret = 0;

	ret = register_trace_android_vh_cpu_idle_enter(
		android_vh_cpu_idle_enter, NULL);
	if (ret < 0) {
		pr_err(TAG "register cpu_idle_enter hook failed, ret %d\n", ret);
		return ret;
	}

	ret = register_trace_android_vh_cpu_idle_exit(
		android_vh_cpu_idle_exit, NULL);
	if (ret < 0) {
		pr_err(TAG "register cpu_idle_exit hook failed, ret %d\n", ret);
		return ret;
	}

	ret = register_trace_android_rvh_cpufreq_transition(
		android_rvh_cpufreq_transition, NULL);
	if (ret < 0) {
		pr_err(TAG "register cpufreq_transition hook failed, ret %d\n", ret);
		return ret;
	}

	return ret;
}

static void cpu_vh_time_count_unregister(void)
{
	unregister_trace_android_vh_cpu_idle_enter(
		android_vh_cpu_idle_enter, NULL);
	unregister_trace_android_vh_cpu_idle_exit(
		android_vh_cpu_idle_exit, NULL);
}

/* Only when exiting cpu idle, the idle time will increase */
static ssize_t show_time(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	struct cpu *cpu_inst = container_of(dev, struct cpu, dev);
	unsigned int cpu = cpu_inst->dev.id;
	u64 time, busy_time, idle_time, offline_time;
	ssize_t len = 0;
	int i, j;
	unsigned long flags;
	unsigned int max_freq;
	struct cpufreq_policy *policy;

	policy = cpufreq_cpu_get(cpu);
	if (!policy)
		return 0;
	max_freq = policy->cpuinfo.max_freq;
	cpufreq_cpu_put(policy);

	if (!cpu_stats_data[cpu].time_in_idle)
		return len;

	len += scnprintf(buf + len, PAGE_SIZE - len, "   Freq  :  State\n");
	len += scnprintf(buf + len, PAGE_SIZE - len, "         : ");
	len += scnprintf(buf + len, PAGE_SIZE - len, "busy\t");
	len += scnprintf(buf + len, PAGE_SIZE - len, "idle\t");
	for (i = 0; i < state_count; i++) {
		if (len >= PAGE_SIZE)
			break;
		len += scnprintf(buf + len, PAGE_SIZE - len, "state%d\t", i);
	}
	len += scnprintf(buf + len, PAGE_SIZE - len, "offline\t");

	if (len >= PAGE_SIZE)
		return PAGE_SIZE;

	len += scnprintf(buf + len, PAGE_SIZE - len, "\n");

	spin_lock_irqsave(&cpu_stats_data[cpu].stats_lock, flags);

	cpu_stats_update(&cpu_stats_data[cpu], cpu_stats_data[cpu].last_state_time);

	for (i = 0; i < cpu_stats_data[cpu].freq_num; i++) {
		if (max_freq < cpu_stats_data[cpu].freq_table[i])
			continue;

		len += scnprintf(buf + len, PAGE_SIZE - len, "%9u ",
				  cpu_stats_data[cpu].freq_table[i]);
		busy_time = cpu_stats_data[cpu].time_in_busy[i];
		idle_time = cpu_stats_data[cpu].idle_time_in_freq[i];
		offline_time = cpu_stats_data[cpu].offline_time[i];

		len += scnprintf(buf + len, PAGE_SIZE - len, "%llu\t", nsec_to_clock_t(busy_time));
		len += scnprintf(buf + len, PAGE_SIZE - len, "%llu\t", nsec_to_clock_t(idle_time));

		for (j = 0; j < state_count; j++) {
			time = cpu_stats_data[cpu].time_in_idle[state_count * i + j];

			len += scnprintf(buf + len, PAGE_SIZE - len, "%llu\t",
				  nsec_to_clock_t(time));
		}
		len += scnprintf(
			buf + len, PAGE_SIZE - len, "%llu\t", nsec_to_clock_t(offline_time));
		len += scnprintf(buf + len, PAGE_SIZE - len, "\n");
	}

	len += scnprintf(buf + len, PAGE_SIZE - len, "    total ");
	len += scnprintf(buf + len, PAGE_SIZE - len, "%llu\t",
			  nsec_to_clock_t(cpu_stats_data[cpu].total_busy_time));
	len += scnprintf(buf + len, PAGE_SIZE - len, "%llu\t",
			  nsec_to_clock_t(cpu_stats_data[cpu].total_idle_time));
	for (i = 0; i < state_count; i++)
		len += scnprintf(buf + len, PAGE_SIZE - len, "%llu\t",
				nsec_to_clock_t(cpu_stats_data[cpu].total_state_time[i]));
	len += scnprintf(buf + len, PAGE_SIZE - len, "%llu\t",
			  nsec_to_clock_t(cpu_stats_data[cpu].total_offline_time));

	len += scnprintf(buf + len, PAGE_SIZE - len, "\n");

	spin_unlock_irqrestore(&cpu_stats_data[cpu].stats_lock, flags);

	if (len >= PAGE_SIZE) {
		pr_warn_once("cpu time table exceeds PAGE_SIZE. Disabling\n");
		return -EFBIG;
	}
	return len;
}

/* We don't care what is written to the attribute */
static ssize_t store_reset(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct cpu *cpu_inst = container_of(dev, struct cpu, dev);
	unsigned int cpu = cpu_inst->dev.id;
	unsigned long flags;

	if (!cpu_stats_data[cpu].time_in_idle)
		return count;

	spin_lock_irqsave(&cpu_stats_data[cpu].stats_lock, flags);

	WRITE_ONCE(cpu_stats_data[cpu].reset_time, local_clock());

	/* do memory barrier */
	smp_wmb();
	WRITE_ONCE(cpu_stats_data[cpu].reset_pending, 1);

	spin_unlock_irqrestore(&cpu_stats_data[cpu].stats_lock, flags);

	return count;
}


static ssize_t show_cpufreq_dvfs_time(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	struct cpu *cpu_inst = container_of(dev, struct cpu, dev);
	unsigned int cpu = cpu_inst->dev.id;
	unsigned int *freq, prof_num, cluster, i, j;
	u64 *usage, shm_size = 0;
	ssize_t len = 0;
	char *kernel_buffer = NULL;
	int ret;
	unsigned int max_freq;
	struct cpufreq_policy *policy;

	policy = cpufreq_cpu_get(cpu);
	if (!policy)
		return 0;

	max_freq = policy->cpuinfo.max_freq;
	cpufreq_cpu_put(policy);

	ret = get_atf_shm_size(&shm_size);
	if (ret != 0) {
		pr_err("atf log read: get shm size error\n");
		return -EINVAL;
	}

	shm_size = min(shm_size, PAGE_SIZE);
	if (shm_size == 0) {
		pr_err("atf log read: shmem is not allocated\n");
		return -EINVAL;
	}

	kernel_buffer = kzalloc(shm_size, GFP_KERNEL);
	if (!kernel_buffer)
		return -ENOMEM;

	cluster = topology_cluster_id(cpu);
	ret = smc_shm_mode(FID_BL31_CPU_USAGE_GET, (char *)kernel_buffer,
			  shm_size, TYPE_OUT, cluster);
	if (ret != 0) {
		pr_err("atf log read: shmem read error\n");
		len = 0;
		kfree(kernel_buffer);
		return -EPERM;
	}

	prof_num = ((unsigned int *)kernel_buffer)[0];
	freq = (unsigned int *)kernel_buffer + 1;
	usage = (u64 *)((unsigned int *)kernel_buffer + 1 + prof_num);
	len += snprintf(buf + len, PAGE_SIZE - len, "cpu_cluster[%u]:\n", cluster);

	for (i = 0; i < prof_num; i++) {
		for (j = 0; j < cpu_stats_data[cpu].freq_num; j++) {
			if (max_freq < freq[i])
				continue;

			if (freq[i] ==
			   cpu_stats_data[cpu].freq_table[j]) {
				len += snprintf(buf + len, PAGE_SIZE - len, "%u %llu\n",
						freq[i], usage[i] / (LMS_TIMESTAMP_RATE / 100));
			}
		}
	}

	if (len >= PAGE_SIZE) {
		pr_warn_once("cpufreq dvfs table exceeds PAGE_SIZE. Disabling\n");
		kfree(kernel_buffer);
		return -EFBIG;
	}

	kfree(kernel_buffer);
	return len;
}

#define SYS_RW_MODE	0644
#define SYS_RO_MODE	0444
static struct device_attribute time = __ATTR(time,
				  SYS_RW_MODE,
				  show_time,
				  store_reset);

static struct device_attribute cpufreq_dvfs_time = __ATTR(cpufreq_time,
				  SYS_RO_MODE,
				  show_cpufreq_dvfs_time,
				  NULL);

static struct attribute *cpustats_attrs[] = {
	&time.attr,
	&cpufreq_dvfs_time.attr,
	NULL
};

static const struct attribute_group cpustats_attr_group = {
	.attrs = cpustats_attrs,
	.name = "cpu_stats"
};

static int get_idle_state_count(void)
{
	struct device_node *cpu_node;
	int cpu, idle_states, state_count = -1;

	for_each_possible_cpu(cpu) {
		cpu_node = of_cpu_device_node_get(cpu);
		if (!cpu_node)
			continue;

		/* State 0 is not included in dts, so we add 1 */
		idle_states = of_count_phandle_with_args(cpu_node,
							  "cpu-idle-states", NULL);
		if (idle_states < 0)
			idle_states = 1;
		else
			idle_states += 1;

		state_count = max(idle_states, state_count);
		of_node_put(cpu_node);
	}

	return state_count;
}

static int cpu_stats_create_table(struct cpu_stats *stats, unsigned int cpu)
{
	unsigned int freq_count;
	struct cpufreq_policy *policy = cpufreq_cpu_get_raw(cpu);
	int i;

	if (!policy)
		return 0;

	freq_count = policy->stats->state_num;
	if (!freq_count)
		return -ENXIO;

	/* Allocate memory for time_in_idle in one go */
	stats->time_in_idle = kzalloc((TIME_IN_IDLE_SIZE(freq_count, state_count) +
				  IDLE_IN_FREQ_SIZE(freq_count) + TIME_IN_BUSY_SIZE(freq_count) +
				  TMP_IDLE_TIME_SIZE(freq_count) + TOTAL_STATE_TIME_SIZE(state_count) +
				  OFFLINE_TIME_SIZE(freq_count)) * sizeof(u64) +
				  FREQ_TABLE_SIZE(freq_count) * sizeof(unsigned int), GFP_ATOMIC);
	if (!stats->time_in_idle) {
		pr_err(TAG "CPU %u create time table failed\n", cpu);
		return -ENOMEM;
	}

	stats->idle_time_in_freq = stats->time_in_idle +
					  TIME_IN_IDLE_SIZE(freq_count, state_count);
	stats->time_in_busy = stats->idle_time_in_freq + IDLE_IN_FREQ_SIZE(freq_count);
	stats->total_state_time = stats->time_in_busy + TIME_IN_BUSY_SIZE(freq_count);
	stats->offline_time = stats->total_state_time + TOTAL_STATE_TIME_SIZE(state_count);
	stats->tmp_idle = stats->offline_time + OFFLINE_TIME_SIZE(freq_count);
	stats->freq_table = (unsigned int *)(stats->tmp_idle + TMP_IDLE_TIME_SIZE(freq_count));

	for (i = 0; i < FREQ_TABLE_SIZE(freq_count); i++)
		stats->freq_table[i] = policy->stats->freq_table[i];

	stats->cpu = cpu;
	stats->last_state_time = local_clock();
	stats->total_idle_time = 0;
	stats->total_busy_time = 0;
	stats->total_offline_time = 0;
	stats->last_freq = freq_table_get_index(policy->stats, policy->cur);
	stats->freq_num = freq_count;
	stats->last_state = -1;
	stats->is_idle = 0;

	return 0;
}

static void cpu_stats_free_table(struct cpu_stats *stats, unsigned int cpu)
{
	if (!stats->time_in_idle)
		return;

	pr_debug(TAG "CPU %u free time stats table\n", cpu);

	kfree(stats->time_in_idle);
}

static int __init cpu_time_stats_init(void)
{
	int cpu, ret = 0;
	struct device *dev = NULL;

	ret = get_idle_state_count();
	if (ret <= 0) {
		pr_err(TAG "Get idle state count failed\n");
		return ret;
	}
	state_count = ret;

	cpus_read_lock();
	for_each_online_cpu(cpu) {
		spin_lock_init(&cpu_stats_data[cpu].stats_lock);
		ret = cpu_stats_create_table(&cpu_stats_data[cpu], cpu);
		if (ret < 0) {
			pr_err(TAG "CPU %d failed to create time stats table\n", cpu);
			cpus_read_unlock();
			goto err_create_table;
		}
	}

	for_each_possible_cpu(cpu) {
		dev = get_cpu_device(cpu);
		if (!dev)
			continue;
		ret = sysfs_create_group(&dev->kobj, &cpustats_attr_group);

		if (ret < 0) {
			pr_err(TAG "CPU %d failed to create sys node\n", cpu);
			cpus_read_unlock();
			goto err_create_fsnode;
		}
	}

	ret = cpuhp_setup_state_nocalls_cpuslocked(CPUHP_AP_ONLINE_DYN,
						  "cpu_stats:online",
						  cpuhp_stats_online,
						  cpuhp_stats_offline);
	cpus_read_unlock();

	if (ret < 0) {
		pr_err(TAG "Setup hotplug state callbacks failed\n");
		goto err_create_fsnode;
	}
	hp_online = ret;
	ret = 0;

	ret = cpu_vh_time_count_register();

	if (ret < 0)
		goto err_register_hook;

	pr_debug(TAG "driver init success\n");
	goto out;

err_register_hook:
	cpu_vh_time_count_unregister();
	cpuhp_remove_state_nocalls(hp_online);
err_create_fsnode:
	for_each_possible_cpu(cpu) {
		dev = get_cpu_device(cpu);
		if (!dev)
			continue;
		if (!kernfs_find_and_get(dev->kobj.sd, cpustats_attr_group.name))
			continue;

		sysfs_remove_group(&dev->kobj, &cpustats_attr_group);
	}
err_create_table:
	for_each_possible_cpu(cpu)
		cpu_stats_free_table(&cpu_stats_data[cpu], cpu);
out:
	return ret;
}
module_init(cpu_time_stats_init);

MODULE_AUTHOR("Ding Li <liding7@xiaomi.com>");
MODULE_DESCRIPTION("XRing CPU Time Stats Driver");
MODULE_LICENSE("GPL v2");
