/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#ifndef __XRING_CPUIDLE_CUSTOM_H__
#define __XRING_CPUIDLE_CUSTOM_H__

#include <linux/cpuidle.h>



#if IS_ENABLED(CONFIG_XRING_SCHED_WALT)
extern struct cpumask __cpu_halt_mask;
extern struct cpumask __cpu_partial_halt_mask;

#define cpu_halt_mask ((struct cpumask *)&__cpu_halt_mask)
#define cpu_partial_halt_mask ((struct cpumask *)&__cpu_partial_halt_mask)

#define cpu_halted(cpu) cpumask_test_cpu((cpu), cpu_halt_mask)
#define cpu_partial_halted(cpu) cpumask_test_cpu((cpu), cpu_partial_halt_mask)
#endif

#define custom_idle_attr_ro(_name) \
static struct custom_idle_attr attr_custom_##_name = \
	__ATTR(_name, 0444, show_##_name, NULL)
#define custom_idle_attr_rw(_name) \
static struct custom_idle_attr attr_custom_##_name = \
	__ATTR(_name, 0644, show_##_name, store_##_name)


struct custom_idle_attr {
	struct attribute attr;
	ssize_t (*show)(struct cpuidle_driver *driver, struct cpuidle_device *device, int val, char *buf);
	ssize_t (*store)(struct cpuidle_driver *driver, struct cpuidle_device *device, int val, const char *buf, size_t cnt);
};

enum reason {
	ORIGIN,
	PREDICT,
	LATENCY,
	HALTED,
	LIMITED,
	IPI,
	REASON_MAX
};

struct state_config {
	unsigned int residency_us;
	unsigned int latency_us;
};

struct custom_kobj {
	struct kobject kobj;
	struct cpuidle_device *dev;
	struct cpuidle_driver *drv;
	int idx;
};

struct custom_config {
	struct state_config state[CPUIDLE_STATE_MAX];
	struct custom_kobj *st_kobj[CPUIDLE_STATE_MAX];
	struct custom_kobj *cpu_kobj;
};

struct custom_stat {
	unsigned int state_map[CPUIDLE_STATE_MAX][CPUIDLE_STATE_MAX];
	unsigned int reason_cnt[CPUIDLE_STATE_MAX][REASON_MAX];
};

struct ret_mode {
	int vpu;
	int wfi;
	int wfe;
};

struct custom_device {
	atomic_t ipi_pending;
	int cluster;
	int reason;
	struct custom_stat stat;
	struct ret_mode ret;
};

DECLARE_PER_CPU(struct custom_config, custom_idle_config);
DECLARE_PER_CPU(struct custom_device, custom_idle_device);

int custom_get_allowed_depth(struct cpuidle_driver *drv,
			     struct cpuidle_device *dev);
void custom_collect_stat(struct cpuidle_driver *drv,
			 struct cpuidle_device *dev,
			 int selected, int duration_us);
void custom_clear_stat(int cpu);
bool custom_forced_state(void);
void custom_set_reason(enum reason reason);
int custom_create_state_attr(struct cpuidle_driver *drv,
			     struct cpuidle_device *dev,
			     const struct attribute_group **cpu_group,
			     const struct attribute_group **state_group);
void custom_remove_state_attr(struct cpuidle_driver *drv,
			      struct cpuidle_device *dev,
			      const struct attribute_group **cpu_group,
			      const struct attribute_group **state_group);
int custom_create_root_attr(const struct attribute_group **groups);
void custom_remove_root_attr(const struct attribute_group **groups);


extern int xr_menu_governor_init(void);
extern int risk_governor_init(void);

#endif
