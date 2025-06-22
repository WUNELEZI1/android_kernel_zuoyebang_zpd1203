// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 * Description: arm amu driver
 * Modify time: 2024-01-17
 */

#define DRVNAME		"arm_amu"
#define pr_fmt(fmt)	DRVNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cpu_pm.h>
#include <linux/bug.h>
#include <linux/cpumask.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/of_device.h>
#include <linux/perf_event.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include "amu_hardware.h"

#define REG_CORE0_BASE_INDEX	0
#define REG_CORE_OFFSET_INDEX	1
#define REG_SIZE_INDEX		2
#define REG_INFO_MAX		3

#define GROUP_NUM		3
#define GROUP_COUNTER_MAX	16
#define COUNTER_MAX		(GROUP_NUM * GROUP_COUNTER_MAX)

#define CORE_NUM_MAX		NR_CPUS

#define TO_HARDWARE_ID(mpidr)		MPIDR_AFFINITY_LEVEL((mpidr), 1)

#define TO_GROUP_ID(event_id)		(((event_id) >> 8) & 0xFF)
#define TO_COUNTER_ID(event_id)		((event_id) & 0xFF)

#define TO_COUNTER_INDEX(group_id, counter_id)		\
	((group_id) * GROUP_COUNTER_MAX + (counter_id))

#define IS_TASK_DIMENSION(cpu)		((cpu) < 0)

#define AMU_EXT_ATTR(_name, _func, _config)				\
	(&((struct dev_ext_attribute[]) {				\
		{							\
			.attr = __ATTR(_name, 0444, _func, NULL),	\
			.var = (void *)_config				\
		}							\
	})[0].attr.attr)

#define AMU_COUNTER_NUM_ATTR(_name, _config)	\
	AMU_EXT_ATTR(_name, amu_counter_num_show, (unsigned long)_config)

#define AMU_EVENT_ATTR(_name, _config)		\
	AMU_EXT_ATTR(_name, amu_sysfs_event_show, (unsigned long)_config)

#define AMU_FORMAT_ATTR(_name, _config)		\
	AMU_EXT_ATTR(_name, amu_sysfs_format_show, (char *)_config)

struct amu_fake_events {
	DECLARE_BITMAP(used_mask, COUNTER_MAX);
};

struct amu_hw_events {
	void __iomem		*amu_base;
	struct perf_event	*events[COUNTER_MAX];
	DECLARE_BITMAP(used_mask, COUNTER_MAX);
};

struct amu_context {
	struct pmu			pmu;
	struct notifier_block		cpu_pm_nb;
	struct amu_hw_events		*hw_events_array[CORE_NUM_MAX];
	u32				counter_num[GROUP_NUM];
};

static ssize_t amu_counter_num_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf);

static ssize_t amu_sysfs_event_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf);

static ssize_t amu_sysfs_format_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf);

static struct attribute *amu_counter_num_attrs[] = {
	AMU_COUNTER_NUM_ATTR(g0_counter_num, 0),
	AMU_COUNTER_NUM_ATTR(g1_counter_num, 1),
	AMU_COUNTER_NUM_ATTR(g2_counter_num, 2),
	NULL,
};

static const struct attribute_group amu_counter_num_attr_group = {
	.attrs = amu_counter_num_attrs,
};

static struct attribute *amu_event_attrs[] = {
	AMU_EVENT_ATTR(cpu_cycles, 0x0),
	AMU_EVENT_ATTR(cnt_cycles, 0x1),
	AMU_EVENT_ATTR(inst_retired, 0x2),
	AMU_EVENT_ATTR(stall_backend_mem, 0x3),
	AMU_EVENT_ATTR(gear0_mpmm_athr_exceeded, 0x100),
	AMU_EVENT_ATTR(gear1_mpmm_athr_exceeded, 0x101),
	AMU_EVENT_ATTR(gear2_mpmm_athr_exceeded, 0x102),
	NULL,
};

static const struct attribute_group amu_events_attr_group = {
	.name = "events",
	.attrs = amu_event_attrs,
};

static struct attribute *amu_format_attrs[] = {
	AMU_FORMAT_ATTR(event, "config:0-15"),
	NULL,
};

static const struct attribute_group amu_format_attr_group = {
	.name = "format",
	.attrs = amu_format_attrs,
};

static const struct attribute_group *amu_attr_groups[] = {
	&amu_counter_num_attr_group,
	&amu_events_attr_group,
	&amu_format_attr_group,
	NULL,
};

static inline struct amu_context *to_amu_context(struct pmu *pmu)
{
	return container_of(pmu, struct amu_context, pmu);
}

static ssize_t amu_counter_num_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct pmu *pmu = dev_get_drvdata(dev);
	struct amu_context *amu_context = to_amu_context(pmu);
	struct dev_ext_attribute *eattr = container_of(attr,
					struct dev_ext_attribute, attr);
	unsigned long index = (unsigned long)eattr->var;
	u32 counter_num;

	if (index >= GROUP_NUM)
		counter_num = 0;
	else
		counter_num = amu_context->counter_num[index];

	return sysfs_emit(buf, "%u\n", counter_num);
}

static ssize_t amu_sysfs_event_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct dev_ext_attribute *eattr = container_of(attr,
					struct dev_ext_attribute, attr);
	return sysfs_emit(buf, "event=0x%lx\n", (unsigned long)eattr->var);
}

static ssize_t amu_sysfs_format_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct dev_ext_attribute *eattr = container_of(attr,
					struct dev_ext_attribute, attr);
	return sysfs_emit(buf, "%s\n", (char *)eattr->var);
}

static inline struct amu_hw_events *get_hw_events(
	struct amu_context *amu_context)
{
	u32 hwid = TO_HARDWARE_ID(read_cpuid_mpidr());

	return amu_context->hw_events_array[hwid];
}

static bool amu_validate_event(struct pmu *pmu,
	struct perf_event *event, struct amu_fake_events *fake_events)
{
	u32 index;

	/* perf event of amu is software event */
	if (event->pmu == pmu) {
		if (IS_TASK_DIMENSION(event->cpu))
			return true;

		index = TO_COUNTER_INDEX(event->hw.event_base, event->hw.idx);
		if (test_and_set_bit(index, fake_events->used_mask))
			return false;

		return true;
	}

	if (is_software_event(event))
		return true;

	return false;
}

/*
 * Make sure the group of events can be scheduled at once
 * on the PMU.
 */
static bool amu_validate_group(struct perf_event *event)
{
	struct perf_event *sibling, *leader = event->group_leader;
	struct amu_fake_events fake_events;

	if (event->group_leader == event)
		return true;

	memset(fake_events.used_mask, 0, sizeof(fake_events.used_mask));
	if (!amu_validate_event(event->pmu, leader, &fake_events))
		return false;
	for_each_sibling_event(sibling, leader) {
		if (!amu_validate_event(event->pmu, sibling, &fake_events))
			return false;
	}
	return amu_validate_event(event->pmu, event, &fake_events);
}

static bool amu_validate_event_id(struct amu_context *amu_context,
	struct perf_event *event)
{
	if (event->hw.event_base >= GROUP_NUM)
		return false;
	if (event->hw.idx >= amu_context->counter_num[event->hw.event_base])
		return false;

	return true;
}

static int amu_event_init(struct perf_event *event)
{
	struct amu_context *amu_context = to_amu_context(event->pmu);

	/* This is, of course, deeply driver-specific */
	if (event->attr.type != event->pmu->type)
		return -ENOENT;

	/* We don't support sampling */
	if (is_sampling_event(event)) {
		dev_dbg(amu_context->pmu.dev,
			"Can't support sampling events\n");
		return -EOPNOTSUPP;
	}

	/* does not support taken branch sampling */
	if (has_branch_stack(event)) {
		dev_dbg(amu_context->pmu.dev, "Can't support filtering\n");
		return -EINVAL;
	}

	event->hw.config_base = event->attr.config;  // event id
	event->hw.event_base = TO_GROUP_ID(event->hw.config_base);
	event->hw.idx = TO_COUNTER_ID(event->hw.config_base);
	if (!amu_validate_event_id(amu_context, event)) {
		dev_dbg(amu_context->pmu.dev,
			"validate event id fail, group %ld counter %d\n",
			event->hw.event_base, event->hw.idx);
		return -EINVAL;
	}

	if (!amu_validate_group(event)) {
		dev_dbg(amu_context->pmu.dev, "validate group fail\n");
		return -EINVAL;
	}

	return 0;
}

static inline u64 amu_read_counter(struct perf_event *event)
{
	struct amu_hw_events *hw_events;

	switch (event->hw.event_base) {
	case 0:
		return amu_group0_cnt_read(event->hw.idx);
	case 1:
		return amu_group1_cnt_read(event->hw.idx);
	case 2:
		hw_events = get_hw_events(to_amu_context(event->pmu));
		return amu_group2_cnt_read(hw_events->amu_base, event->hw.idx);
	default:
		break;
	}

	return 0;
}

static void amu_event_update(struct perf_event *event)
{
	u64 delta, prev_count, new_count;

	new_count = amu_read_counter(event);
	prev_count = local64_xchg(&event->hw.prev_count, new_count);
	delta = new_count - prev_count;  // correct even when overflow
	local64_add(delta, &event->count);
}

static void amu_start(struct perf_event *event, int pmu_flags)
{
	u64 new_count;

	/* We always reprogram the counter */
	if (pmu_flags & PERF_EF_RELOAD)
		WARN_ON(!(event->hw.state & PERF_HES_UPTODATE));

	new_count = amu_read_counter(event);
	local64_set(&event->hw.prev_count, new_count);
	event->hw.state = 0;
}

static void amu_stop(struct perf_event *event, int pmu_flags)
{
	if (event->hw.state & PERF_HES_STOPPED)
		return;
	amu_event_update(event);
	event->hw.state |= PERF_HES_STOPPED | PERF_HES_UPTODATE;
}

static bool amu_alloc_counter(struct perf_event *event)
{
	struct amu_hw_events *hw_events;
	u32 index;

	if (IS_TASK_DIMENSION(event->cpu))
		return true;


	index = TO_COUNTER_INDEX(event->hw.event_base, event->hw.idx);
	hw_events = get_hw_events(to_amu_context(event->pmu));
	if (test_and_set_bit(index, hw_events->used_mask))
		return false;
	hw_events->events[index] = event;

	return true;
}

static void amu_free_counter(struct perf_event *event)
{
	struct amu_hw_events *hw_events;
	u32 index;

	if (IS_TASK_DIMENSION(event->cpu))
		return;

	index = TO_COUNTER_INDEX(event->hw.event_base, event->hw.idx);
	hw_events = get_hw_events(to_amu_context(event->pmu));
	hw_events->events[index] = NULL;
	clear_bit(index, hw_events->used_mask);
}

static int amu_add(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;

	if (!amu_alloc_counter(event))
		return -EAGAIN;

	hwc->state = PERF_HES_STOPPED | PERF_HES_UPTODATE;

	if (flags & PERF_EF_START)
		amu_start(event, PERF_EF_RELOAD);

	/* Propagate our changes to the userspace mapping. */
	perf_event_update_userpage(event);

	return 0;
}

static void amu_del(struct perf_event *event, int flags)
{
	amu_stop(event, PERF_EF_UPDATE);
	amu_free_counter(event);
	perf_event_update_userpage(event);
}

static void amu_read(struct perf_event *event)
{
	if (event->hw.state & PERF_HES_STOPPED)
		return;
	amu_event_update(event);
}

#ifdef CONFIG_CPU_PM

static int cpu_pm_amu_notify(struct notifier_block *b, unsigned long cmd,
			     void *v)
{
	struct amu_context *amu_context =
		container_of(b, struct amu_context, cpu_pm_nb);
	struct amu_hw_events *hw_events = get_hw_events(amu_context);
	u32 index;

	switch (cmd) {
	case CPU_PM_ENTER:
		for_each_set_bit(index, hw_events->used_mask, COUNTER_MAX)
			amu_stop(hw_events->events[index], PERF_EF_UPDATE);
		break;
	case CPU_PM_EXIT:
	case CPU_PM_ENTER_FAILED:
		for_each_set_bit(index, hw_events->used_mask, COUNTER_MAX)
			amu_start(hw_events->events[index], PERF_EF_RELOAD);
		break;
	default:
		return NOTIFY_DONE;
	}

	return NOTIFY_OK;
}

static int cpu_pm_amu_register(struct amu_context *amu_context)
{
	amu_context->cpu_pm_nb.notifier_call = cpu_pm_amu_notify;
	return cpu_pm_register_notifier(&amu_context->cpu_pm_nb);
}

static void cpu_pm_amu_unregister(struct amu_context *amu_context)
{
	cpu_pm_unregister_notifier(&amu_context->cpu_pm_nb);
}
#else
static inline int cpu_pm_amu_register(struct amu_context *amu_context)
{
	return 0;
}
static inline void cpu_pm_amu_unregister(struct amu_context *amu_context) { }
#endif

static u64 of_get_cpu_mpidr(struct device_node *dn)
{
	const __be32 *cell;
	u64 hwid;

	cell = of_get_property(dn, "reg", NULL);
	if (!cell) {
		pr_err("%pOF: missing reg property\n", dn);
		return INVALID_HWID;
	}

	hwid = of_read_number(cell, of_n_addr_cells(dn));
	/*
	 * Non affinity bits must be set to 0 in the DT
	 */
	if (hwid & ~MPIDR_HWID_BITMASK) {
		pr_err("%pOF: invalid reg property\n", dn);
		return INVALID_HWID;
	}
	return hwid;
}

static void amu_init(struct amu_context *amu_context)
{
	u64 reg_val = read_amcgcr();

	amu_context->counter_num[0] = AMU_GROUP0_NUM(reg_val);
	amu_context->counter_num[1] = AMU_GROUP1_NUM(reg_val);
}

static int amu_dt_parse(struct device *dev, struct amu_context *amu_context)
{
	struct device_node *np = dev->of_node;
	u64 reg_info[REG_INFO_MAX];
	struct device_node *dn;
	u64 mpidr;
	u32 hwid;
	u64 physical_base;
	u64 size;
	struct amu_hw_events *hw_events;

	if (of_property_read_u32(np, "g2-counter-num",
		&amu_context->counter_num[2])) {
		dev_err(dev, "read prop g2-counter-num fail\n");
		return -EINVAL;
	}

	if (of_property_read_u64_array(np, "reg-param",
		reg_info, ARRAY_SIZE(reg_info))) {
		dev_err(dev, "read prop reg-param fail\n");
		return -EINVAL;
	}

	for_each_of_cpu_node(dn) {
		/* TODO: check whether cpu_logical_map is exported */
		mpidr = of_get_cpu_mpidr(dn);
		if (mpidr == INVALID_HWID)
			continue;

		hwid = TO_HARDWARE_ID(mpidr);
		if (hwid >= CORE_NUM_MAX) {
			dev_err(dev, "invalid hardware id %u\n", hwid);
			return -ENODEV;
		}

		physical_base = reg_info[REG_CORE0_BASE_INDEX] +
			reg_info[REG_CORE_OFFSET_INDEX] * hwid;
		size = reg_info[REG_SIZE_INDEX];
		hw_events = devm_kzalloc(dev, sizeof(*hw_events),
			GFP_KERNEL);
		if (!hw_events)
			return -ENOMEM;

		hw_events->amu_base = devm_ioremap(dev, physical_base, size);
		if (!hw_events->amu_base) {
			dev_err(dev,
				"fail to ioremap, base 0x%llx size 0x%llx\n",
				physical_base, size);
			return -ENODEV;
		}
		amu_context->hw_events_array[hwid] = hw_events;
	}

	return 0;
}

static struct amu_context *amu_alloc_context(struct platform_device *pdev)
{
	struct amu_context *amu_context;

	amu_context = devm_kzalloc(&pdev->dev, sizeof(*amu_context),
		GFP_KERNEL);
	if (!amu_context)
		return NULL;

	amu_context->pmu = (struct pmu) {
		/*
		 * There can only be a single PMU for perf_hw_context events
		 * which is assigned to core PMU. Hence use "perf_sw_context"
		 * for per-task profiling (which is not possible with
		 * perf_invalid_context).
		 */
		.task_ctx_nr	= perf_sw_context,
		.module		= THIS_MODULE,
		.event_init	= amu_event_init,
		.add		= amu_add,
		.del		= amu_del,
		.start		= amu_start,
		.stop		= amu_stop,
		.read		= amu_read,
		.attr_groups	= amu_attr_groups,
		.capabilities	= PERF_PMU_CAP_NO_INTERRUPT
					| PERF_PMU_CAP_NO_EXCLUDE,
	};

	return amu_context;
}

static int amu_device_probe(struct platform_device *pdev)
{
	int rc;
	struct amu_context *amu_context;

	amu_context = amu_alloc_context(pdev);
	if (!amu_context)
		return -ENOMEM;

	rc = amu_dt_parse(&pdev->dev, amu_context);
	if (rc)
		return rc;
	amu_init(amu_context);

	platform_set_drvdata(pdev, amu_context);

	rc = cpu_pm_amu_register(amu_context);
	if (rc)
		return rc;

	rc = perf_pmu_register(&amu_context->pmu, DRVNAME, -1);
	if (rc) {
		cpu_pm_amu_unregister(amu_context);
		return rc;
	}

	return 0;
}

static int amu_device_remove(struct platform_device *pdev)
{
	struct amu_context *amu_context = platform_get_drvdata(pdev);

	perf_pmu_unregister(&amu_context->pmu);
	cpu_pm_amu_unregister(amu_context);

	return 0;
}

static const struct of_device_id amu_of_match[] = {
	{ .compatible = "xring,arm-amu", },
	{},
};
MODULE_DEVICE_TABLE(of, amu_of_match);

static struct platform_driver amu_driver = {
	.driver = {
		.name = DRVNAME,
		.of_match_table = of_match_ptr(amu_of_match),
		.suppress_bind_attrs = true,
	},
	.probe = amu_device_probe,
	.remove = amu_device_remove,
};

module_platform_driver(amu_driver);

MODULE_DESCRIPTION("XRing arm amu driver");
MODULE_LICENSE("GPL v2");
