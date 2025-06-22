// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2012 Altera Corporation
 * Copyright (c) 2011 Picochip Ltd., Jamie Iles
 *
 * Modified from mach-picoxcell/time.c
 */
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/sched_clock.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/clocksource.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include "dw_apb_timer.h"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)  "XR_APBTIMER:%s:%d " fmt, __func__, __LINE__

#define DW_APB_TIMER_RATING 300
#define MAP_SIZE 4
#define TIMER_BASE_NAME_SIZE 30
static int num_called;

static int __init timer_get_base_and_rate(struct device_node *np,
					void __iomem **base, u32 *rate)
{
	struct clk *timer_clk;
	struct clk *pclk;
	int ret = 0;

	*base = of_iomap(np, 0);

	if (!*base) {
		pr_err("Unable to map regs for %pOFn", np);
		return !*base;
	}

	/*
	 * Not all implementations use a peripheral clock, so don't panic
	 * if it's not present
	 */
	pclk = of_clk_get_by_name(np, "pclk_peri_timer_ns");
	if (!IS_ERR(pclk)) {
		if (clk_prepare_enable(pclk)) {
			pr_warn("pclk for %pOFn is present, but could not be activated\n",
				np);
		}
		timer_clk = of_clk_get(np, 1);
		if (IS_ERR(timer_clk)) {
			ret = PTR_ERR(timer_clk);
			pr_err("fail to get timer_clk!\n");
			goto out_pclk_disable;
		}
	} else {
		timer_clk = of_clk_get(np, 0);
		if (IS_ERR(timer_clk)) {
			pr_info("Clock not found\n");
			ret = of_property_read_u32(np, "clock-frequency", rate);
			if (ret) {
				pr_err("failed to get clock-frequency, ret = %d\n", ret);
				goto out_pclk_disable;
			}
			return ret;
		}
	}

	ret = of_property_read_u32(np, "clock-frequency", rate);
	if (ret) {
		pr_err("failed to get clock-frequency, ret = %d\n", ret);
		goto out_pclk_disable;
	}
	pr_debug("first, rate:%u\n", *rate);
	ret = clk_prepare_enable(timer_clk);
	if (ret)
		goto out_timer_clk_put;

	*rate = clk_get_rate(timer_clk);
	if (!(*rate)) {
		ret = -EINVAL;
		goto out_timer_clk_disable;
	}
	pr_debug("finanly, rate:%u\n", *rate);
	return 0;

out_timer_clk_disable:
	clk_disable_unprepare(timer_clk);
out_timer_clk_put:
	clk_put(timer_clk);
out_pclk_disable:
	if (!IS_ERR(pclk)) {
		clk_disable_unprepare(pclk);
		clk_put(pclk);
	}
	iounmap(*base);
	return ret;
}



static int __init add_clockevent(struct device_node *event_timer)
{
	void __iomem *iobase;
	struct dw_apb_clock_event_device *ced;
	u32 irq, rate, rating;
	int ret = 0;

	irq = irq_of_parse_and_map(event_timer, 0);
	if (irq == 0) {
		pr_err("No IRQ for clock event timer");
		return !irq;
	}

	ret = timer_get_base_and_rate(event_timer, &iobase, &rate);
	if (ret)
		return ret;
	ret = of_property_read_u32(event_timer, "rating", &rating);
	if (ret != 0)
		ced = dw_apb_clockevent_init(-1, event_timer->name, DW_APB_TIMER_RATING,
		iobase, irq, rate);
	else
		ced = dw_apb_clockevent_init(-1, event_timer->name, rating,
		iobase, irq, rate);
	if (!ced)
		return -EINVAL;

	dw_apb_clockevent_register(ced);

	return 0;
}

static void __iomem *sched_io_base;
static u32 sched_rate;

static int __init add_clocksource(struct device_node *source_timer)
{
	void __iomem *iobase;
	struct dw_apb_clocksource *cs;
	u32 rate;
	int ret = 0;

	ret = timer_get_base_and_rate(source_timer, &iobase, &rate);
	if (ret)
		return ret;

	cs = dw_apb_clocksource_init(DW_APB_TIMER_RATING, source_timer->name, iobase, rate);
	if (!cs)
		return -EINVAL;

	dw_apb_clocksource_start(cs);
	dw_apb_clocksource_register(cs);

	/*
	 * Fallback to use the clocksource as sched_clock if no separate
	 * timer is found. sched_io_base then points to the current_value
	 * register of the clocksource timer.
	 */
	sched_io_base = iobase + 0x04;
	sched_rate = rate;

	return 0;
}

#ifdef CONFIG_ARM
static unsigned long dw_apb_delay_timer_read(void)
{
	return ~readl_relaxed(sched_io_base);
}

static struct delay_timer dw_apb_delay_timer = {
	.read_current_timer	= dw_apb_delay_timer_read,
};
#endif

static int __init dw_apb_timer_init(struct device_node *timer)
{
	int ret = 0;

	switch (num_called) {
	case 1:
		pr_debug("%s: found clocksource timer\n", __func__);
		ret = add_clocksource(timer);
		if (ret)
			return ret;
#ifdef CONFIG_ARM
		dw_apb_delay_timer.freq = sched_rate;
		register_current_timer_delay(&dw_apb_delay_timer);
#endif
		break;
	default:
		pr_debug("%s: found clockevent timer\n", __func__);
		ret = add_clockevent(timer);
		if (ret)
			return ret;
		break;
	}

	num_called++;

	return 0;
}

static const struct of_device_id dw_apb_timer[] = {
	{ .compatible = "xring,dw-apb-timer",  },
	{ },
};
MODULE_DEVICE_TABLE(of, dw_apb_timer);

static int crg_config(struct device_node *np)
{
	int ret = 0;
	u32 clk_address;
	void __iomem *clk_regs;
	u32 clk_bit;
	struct reset_control	*rst;
	struct reset_control	*prst;

	prst = of_reset_control_get_exclusive(np, "prst");
	if (!IS_ERR(prst)) {
		ret = reset_control_deassert(prst);
		if (ret < 0) {
			pr_err("Couldn't deassert prst control: %d\n", ret);
			return ret;
		}
	}

	rst = of_reset_control_get_exclusive(np, "rst");
	if (IS_ERR(rst))  {
		pr_err("Couldn't get rst control: %d\n", ret);
		return PTR_ERR(rst);
	}
	ret = reset_control_deassert(rst);
	if (ret < 0) {
		pr_err("Couldn't deassert rst control: %d\n", ret);
		return ret;
	}

	/* clk enable */
	ret = of_property_read_u32(np, "clk_address", &clk_address);
	if (!ret) {
		ret = of_property_read_u32(np, "clk_bit", &clk_bit);
		if (ret) {
			pr_err("failed to get clk_bit, ret = %d\n", ret);
			return ret;
		}
		clk_regs = ioremap(clk_address, MAP_SIZE);
		if (IS_ERR(clk_regs)) {
			pr_err("err = %ld\n", PTR_ERR(clk_regs));
			return -EFAULT;
		}
		writel((1 << clk_bit), clk_regs);
		iounmap(clk_regs);
	}

	return 0;
}

static int __init dw_timer_probe(void)
{
	struct device_node *node = NULL;
	const struct of_device_id *match = NULL;
	int ret = 0;

	pr_info("DTS probe entence...\n");

	/* find all device nodes via device node id */
	for_each_matching_node_and_match(node, dw_apb_timer, &match) {
		if (node) {
			if (!of_device_is_available(node)) {
				pr_info("node(%s) status is disable, Skip initialization\n",
					node->name);
				continue;
			}
			ret = crg_config(node);
			if (ret)
				pr_err("failed to config timer_crg\n");
			pr_info("Matching %s\n", node->name);
			dw_apb_timer_init(node);
		}
		if (match)
			pr_info("device_id compatible: %s\n", match->compatible);
	}
	return 0;
}

module_init(dw_timer_probe);

MODULE_SOFTDEP("pre: xr_ip_reset");
MODULE_AUTHOR("Jacob Pan (jacob.jun.pan@intel.com)");
MODULE_DESCRIPTION("Synopsys DesignWare APB Timers");
MODULE_LICENSE("GPL v2");
