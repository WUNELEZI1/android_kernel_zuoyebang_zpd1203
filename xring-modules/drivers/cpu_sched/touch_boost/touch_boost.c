// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/bitmap.h>
#include <linux/cpu.h>
#include <linux/hrtimer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <trace/events/irq.h>
#include "../walt/walt.h"
#include <soc/xring/xr_cpuidle.h>

#define CREATE_TRACE_POINTS
#include "touch_boost_trace.h"

#define GIC_V3_NAME "GICv3"
#define MODE_RW 0640
#define IRQ_ARG_NUM 4  /* irq_type, spi or irq, lim_ns, cpumask */

/* SPI is hwirq, IRQ is virq */
#define MIN_SPI 32
#define MAX_SPI 2047
#define MIN_IRQ 0
#define MAX_IRQ 1023

#define BOOST_CPUIDLE_DEPTH 0
#define UNBOOST_CPUIDLE_DEPTH 3

enum irq_type_t {
	IRQ_TYPE_SPI,
	IRQ_TYPE_IRQ,
	IRQ_TYPE_NUM,
};

enum boost_type_t {
	BOOST_TYPE_IDLE,
	BOOST_TYPE_FREQ,
	BOOST_TYPE_NUM,
};

struct irq_config_t {
	int tag[IRQ_TYPE_NUM];
	u32 lim_ns;
	u32 freq_boost_cpumask;
};

struct boost_handle_t {
	struct device *dev;
	int cpu;

	struct hrtimer stop_timers[BOOST_TYPE_NUM];
	bool boosted[BOOST_TYPE_NUM];
	enum boost_type_t boost_type;

	int trigger_irq;
	struct __call_single_data csd;
};

static struct irq_chip *g_gic_chip;
static DEFINE_PER_CPU(struct boost_handle_t, g_boost_handle);
static DECLARE_BITMAP(g_spi_bitmap, MAX_SPI + 1);
static DECLARE_BITMAP(g_irq_bitmap, MAX_IRQ + 1);
static bool g_disable_spi_match = true;
static u32 g_irq_num_max;  /* decided by dts */
static struct irq_config_t *g_irq_config;

static DEFINE_MUTEX(config_mutex);

static struct irq_config_t *find_slot(enum irq_type_t irq_type, int tag_val)
{
	int i;

	for (i = 0; i < g_irq_num_max; ++i) {
		if (g_irq_config[i].tag[irq_type] == tag_val)
			return &g_irq_config[i];
	}

	return NULL;
}

static int update_slot(
	struct device *dev, enum irq_type_t irq_type, struct irq_config_t *new_slot)
{
	struct irq_config_t *empty_slot = NULL;
	int i;

	for (i = 0; i < g_irq_num_max; ++i) {
		/* record the first empty slot */
		if (!empty_slot && g_irq_config[i].lim_ns == 0)
			empty_slot = &g_irq_config[i];

		if (g_irq_config[i].tag[irq_type] == new_slot->tag[irq_type]) {
			/* slot found, update it */
			g_irq_config[i] = new_slot->lim_ns ? *new_slot : (struct irq_config_t){0};
			return 0;
		}
	}

	if (new_slot->lim_ns) {
		if (empty_slot == NULL) {
			dev_err(dev, "Cannot config more than %u irqs!\n", g_irq_num_max);
			return -ENOMEM;
		}
		/* insert to the empty slot */
		*empty_slot = *new_slot;
		return 0;
	}

	dev_err(dev, "Failed to find a slot to clear!\n");
	return -1;
}

static int update_config(struct device *dev, u32 args[IRQ_ARG_NUM])
{
	enum irq_type_t irq_type = args[0];
	u32 spi_or_irq = args[1];
	u32 lim_ns = args[2];
	u32 freq_boost_cpumask = args[3];

	struct irq_config_t new_slot = {0};
	u32 min_spi_or_irq, max_spi_or_irq;
	int ret;

	if (irq_type < 0 || irq_type >= IRQ_TYPE_NUM) {
		dev_err(dev, "Invalid irq type!\n");
		return -EINVAL;
	}

	if (irq_type == IRQ_TYPE_SPI) {
		min_spi_or_irq = MIN_SPI;
		max_spi_or_irq = MAX_SPI;
	} else {
		min_spi_or_irq = MIN_IRQ;
		max_spi_or_irq = MAX_IRQ;
	}

	if (spi_or_irq < min_spi_or_irq || spi_or_irq > max_spi_or_irq) {
		dev_err(dev, "%s must be in [%d ... %d]\n",
			irq_type == IRQ_TYPE_SPI ? "SPI" : "IRQ",
			min_spi_or_irq, max_spi_or_irq);
		return -EINVAL;
	}

	new_slot.tag[irq_type] = spi_or_irq;
	new_slot.lim_ns = lim_ns;
	new_slot.freq_boost_cpumask = freq_boost_cpumask;

	if (lim_ns) {
		ret = update_slot(dev, irq_type, &new_slot);
		if (ret)
			return ret;

		smp_wmb();  /* ensure slot updated, then set bit */

		if (irq_type == IRQ_TYPE_SPI)
			set_bit(spi_or_irq, g_spi_bitmap);
		else
			set_bit(spi_or_irq, g_irq_bitmap);
	} else {
		if (irq_type == IRQ_TYPE_SPI)
			clear_bit(spi_or_irq, g_spi_bitmap);
		else
			clear_bit(spi_or_irq, g_irq_bitmap);

		smp_wmb();  /* ensure bit cleared, then clear slot */

		ret = update_slot(dev, irq_type, &new_slot);
	}

	g_disable_spi_match = bitmap_empty(g_spi_bitmap, MAX_SPI);

	return ret;
}

static ssize_t config_show(
	struct device *dev, struct device_attribute __maybe_unused *attr, char *buf)
{
	char *buf_p = buf;
	int i, irq_type;

	mutex_lock(&config_mutex);
	for (i = 0; i < g_irq_num_max; ++i) {
		if (g_irq_config[i].lim_ns == 0)
			continue;

		irq_type = g_irq_config[i].tag[IRQ_TYPE_SPI] ? IRQ_TYPE_SPI : IRQ_TYPE_IRQ;
		buf_p += scnprintf(
			buf_p, PAGE_SIZE - (buf_p - buf),
			"%s %d lim_ns %u freq_boost_cpumask 0x%x\n",
			irq_type == IRQ_TYPE_SPI ? "SPI" : "IRQ",
			g_irq_config[i].tag[irq_type], g_irq_config[i].lim_ns,
			g_irq_config[i].freq_boost_cpumask);
	}
	mutex_unlock(&config_mutex);

	return strlen(buf);
}

static ssize_t config_store(
	struct device *dev, struct device_attribute __maybe_unused *attr,
	const char *buf, size_t count)
{
	char *tmp_buf = NULL;
	char *p = NULL;
	char *token = NULL;
	u32 arg[IRQ_ARG_NUM];
	int i;
	int ret = 0;

	tmp_buf = kstrdup(buf, GFP_KERNEL);
	if (!tmp_buf)
		return -ENOMEM;

	p = tmp_buf;

	for (i = 0; i < IRQ_ARG_NUM; ++i) {
		token = strsep(&p, " ");
		if (!token) {
			dev_err(dev, "Failed: no arg[%d]\n", i);
			goto end_free;
		}
		ret = kstrtou32(token, 0, &arg[i]);
		if (ret) {
			dev_err(dev, "Failed: arg[%d] is not u32\n", i);
			goto end_free;
		}
	}

	mutex_lock(&config_mutex);
	ret = update_config(dev, arg);
	mutex_unlock(&config_mutex);

end_free:
	kfree(tmp_buf);
	return count;
}
static DEVICE_ATTR(config, MODE_RW, config_show, config_store);

static struct attribute *g_touch_boost_attrs[] = {
	&dev_attr_config.attr,
	NULL,
};

static struct attribute_group g_touch_boost_attr_group = {
	.attrs = g_touch_boost_attrs,
};

static struct irq_config_t *get_configured_global(int irq)
{
	if (!test_bit(irq, g_irq_bitmap))
		return NULL;
	/* do memory barrier */
	smp_rmb();
	return find_slot(IRQ_TYPE_IRQ, irq);
}

static struct irq_config_t *get_configured_spi(int irq)
{
	struct irq_data *data = irq_get_irq_data(irq);
	struct irq_chip *chip = irq_data_get_irq_chip(data);

	if (g_gic_chip) {
		if (g_gic_chip == chip)
			goto is_gic_v3;
		else
			return NULL;
	}

	if (!chip)
		return NULL;

	if (strcmp(chip->name, GIC_V3_NAME) == 0) {
		g_gic_chip = chip;
		goto is_gic_v3;
	}

	return NULL;

is_gic_v3:
	if (!test_bit(data->hwirq, g_spi_bitmap))
		return NULL;

	/* do memory barrier */
	smp_rmb();
	return find_slot(IRQ_TYPE_SPI, data->hwirq);
}

static void cpufreq_boost_set(int cpu, bool boost)
{
	struct rq *rq = cpu_rq(cpu);
	unsigned long flags;

	raw_spin_lock_irqsave(&rq->__lock, flags);
	xresgov_run_callback(cpu, boost ? WALT_IRQ_BOOST : WALT_IRQ_UNBOOST);
	raw_spin_unlock_irqrestore(&rq->__lock, flags);
}

static void unboost_cpu(struct hrtimer *timer, enum boost_type_t boost_type)
{
	struct boost_handle_t *handle = container_of(
		timer, struct boost_handle_t, stop_timers[boost_type]);
	int cpu = handle->cpu;
	bool *boosted = &handle->boosted[boost_type];

	if (*boosted) {
		*boosted = false;
		if (boost_type == BOOST_TYPE_IDLE)
			cpuidle_set_allowed_depth(cpu, UNBOOST_CPUIDLE_DEPTH, IDLE_CLIENT_BOOST);
		else
			cpufreq_boost_set(cpu, false);
	} else {
		dev_err_ratelimited(
			handle->dev, "cpu%d not %s_boosted but stop_timer active\n",
			cpu, boost_type == BOOST_TYPE_IDLE ? "idle" : "freq");
	}

	trace_touch_boost_end(cpu, boost_type);
}

static enum hrtimer_restart idle_timer_callback(struct hrtimer *timer)
{
	unboost_cpu(timer, BOOST_TYPE_IDLE);
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart freq_timer_callback(struct hrtimer *timer)
{
	unboost_cpu(timer, BOOST_TYPE_FREQ);
	return HRTIMER_NORESTART;
}

static void boost_cpu(void *info)
{
	struct irq_config_t *configured_irq = (struct irq_config_t *)info;
	int cpu = smp_processor_id();
	struct boost_handle_t *handle = &per_cpu(g_boost_handle, cpu);
	enum boost_type_t boost_type = handle->boost_type;
	bool *boosted = &handle->boosted[boost_type];
	ktime_t ktime;

	ktime = ktime_set(0, configured_irq->lim_ns);
	hrtimer_start(&handle->stop_timers[boost_type], ktime, HRTIMER_MODE_REL_PINNED);

	if (!*boosted) {
		*boosted = true;
		if (boost_type == BOOST_TYPE_IDLE)
			cpuidle_set_allowed_depth(cpu, BOOST_CPUIDLE_DEPTH, IDLE_CLIENT_BOOST);
		else
			cpufreq_boost_set(cpu, true);
	}

	if (!trace_touch_boost_start_enabled())
		return;

	trace_touch_boost_start(
		configured_irq->tag[IRQ_TYPE_SPI], handle->trigger_irq, configured_irq->lim_ns,
		cpu, boost_type);
}

static void trace_irqentry_callback(void *data, int irq, struct irqaction *action)
{
	struct irq_config_t *configured_irq = NULL;
	struct boost_handle_t *handle = NULL;
	int cpu = smp_processor_id();

	configured_irq = get_configured_global(irq);
	if (unlikely(!configured_irq && !g_disable_spi_match))
		configured_irq = get_configured_spi(irq);
	if (!configured_irq)
		return;

	handle = &per_cpu(g_boost_handle, cpu);
	handle->trigger_irq = irq;

	/* always idle boost current cpu */
	handle->boost_type = BOOST_TYPE_IDLE;
	boost_cpu(configured_irq);

	if (!configured_irq->freq_boost_cpumask) {
		/* freq boost current cpu */
		handle->boost_type = BOOST_TYPE_FREQ;
		boost_cpu(configured_irq);
		return;
	}

	/* freq boost by freq_boost_cpumask */
	for_each_possible_cpu(cpu)
		if ((1 << cpu) & configured_irq->freq_boost_cpumask) {
			handle = &per_cpu(g_boost_handle, cpu);
			handle->trigger_irq = irq;
			handle->boost_type = BOOST_TYPE_FREQ;
			handle->csd.func = boost_cpu;
			handle->csd.info = configured_irq;
			smp_call_function_single_async(cpu, &handle->csd);
		}
}

static int parse_dts_config(struct device *dev)
{
	struct device_node *np = dev->of_node;
	u32 *arg_array = NULL;
	u32 i;
	int arg_num, ret;

	ret = of_property_read_u32(np, "irq_num_max", &g_irq_num_max);
	if (ret) {
		dev_err(dev, "Failed to get irq_num_max in dts!\n");
		return ret;
	}

	if (g_irq_num_max == 0) {
		dev_err(dev, "irq_num_max in dts cannot be 0!\n");
		return -EINVAL;
	}

	g_irq_config = devm_kzalloc(dev, g_irq_num_max * sizeof(struct irq_config_t), GFP_KERNEL);
	if (!g_irq_config)
		return -ENOMEM;

	arg_num = of_property_count_u32_elems(np, "irq_config");
	if (arg_num < 0 || arg_num % IRQ_ARG_NUM) {
		dev_err(dev, "Bad irq_config in dts!\n");
		return -EINVAL;
	}

	if (arg_num == 0)
		return 0;

	arg_array = kcalloc(arg_num, sizeof(u32), GFP_KERNEL);
	if (!arg_array)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "irq_config", arg_array, arg_num);
	if (ret) {
		dev_err(dev, "Failed to get u32 array irq_config in dts!\n");
		goto free_array;
	}

	for (i = 0; i < arg_num; i += IRQ_ARG_NUM) {
		if (arg_array[i] == IRQ_TYPE_IRQ) {
			ret = of_get_named_gpio(np, "boost-gpio", arg_array[i+1]);
			if (ret < 0)
				continue;

			arg_array[i+1] = gpio_to_irq(ret);
		}

		update_config(dev, arg_array + i);
	}

	ret = 0;

free_array:
	kfree(arg_array);
	return ret;
}

static int touch_boost_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct boost_handle_t *handle = NULL;
	int cpu;
	int ret = 0;

	ret = parse_dts_config(dev);
	if (ret)
		return ret;

	for_each_possible_cpu(cpu) {
		handle = &per_cpu(g_boost_handle, cpu);
		handle->dev = dev;
		handle->cpu = cpu;

		hrtimer_init(
			&handle->stop_timers[BOOST_TYPE_IDLE],
			CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED);
		handle->stop_timers[BOOST_TYPE_IDLE].function = idle_timer_callback;

		hrtimer_init(
			&handle->stop_timers[BOOST_TYPE_FREQ],
			CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED);
		handle->stop_timers[BOOST_TYPE_FREQ].function = freq_timer_callback;
	}

	ret = register_trace_irq_handler_entry(trace_irqentry_callback, NULL);
	if (ret) {
		dev_err(dev, "Failed to register irq handler entry!\n");
		return ret;
	}

	ret = sysfs_create_group(&dev->kobj, &g_touch_boost_attr_group);
	if (ret) {
		dev_err(dev, "Failed to create sysfs group!\n");
		unregister_trace_irq_handler_entry(trace_irqentry_callback, NULL);
		return ret;
	}

	return 0;
}

static const struct of_device_id g_touch_boost_of_match[] = {
	{
		.compatible = "xring,touch_boost",
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_touch_boost_of_match);

static struct platform_driver g_touch_boost_driver = {
	.probe = touch_boost_probe,
	.driver = {
		.name = "touch_boost",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_touch_boost_of_match),
	},
};

static int __init touch_boost_init(void)
{
	return platform_driver_register(&g_touch_boost_driver);
}
module_init(touch_boost_init);

MODULE_AUTHOR("Aoyang Yan <yanaoyang@xiaomi.com>");
MODULE_DESCRIPTION("XRing L3 Touch Boost Driver");
MODULE_LICENSE("GPL v2");
