// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/err.h>
#include <linux/cpu.h>
#include <trace/hooks/psci.h>

#include <soc/xring/walt.h>
#include "walt.h"
#include "trace.h"

/*
 * prop[0] - Utility bus base adderss for core register,
 * prop[1] - Utility bus amu offset,
 * prop[2] - Utility bus percpu stride of core register.
 */
#define PROP_MAX_NUM			(3)
#define AMU_VAL_INVAL			((u64)(0xdeadbeafdeadbeaf))
#define AMU_VAL_INVAL_32		((u32)(0xdeadbeaf))

#define read_corecnt()		read_sysreg_s(SYS_AMEVCNTR0_CORE_EL0)
#define read_constcnt()		read_sysreg_s(SYS_AMEVCNTR0_CONST_EL0)

struct cpu_counter {
	void __iomem *reg;
	spinlock_t lock;
	u64 total_cycle[MAXCNT];
	u64 prev_cycle[MAXCNT];
	u64 saved_cycle[MAXCNT];
	bool offline;
};
DEFINE_PER_CPU(struct cpu_counter, xr_cpu_counter);

static inline bool is_val_valid(u64 val)
{
	if (!val)
		return false;

	if (val == AMU_VAL_INVAL)
		return false;

	if ((u32)(val >> 32) == AMU_VAL_INVAL_32 ||
			(u32)(val & 0xffffffff) == AMU_VAL_INVAL_32)
		return false;

	return true;
}

u64 xr_get_cpu_cycle_counter(int cpu, int evt)
{
	struct cpu_counter *counter;
	u64 cycle_ret, val, prev, saved;
	unsigned long flags;
	u32 offset = evt * 0x8;

	counter = &per_cpu(xr_cpu_counter, cpu);

	spin_lock_irqsave(&counter->lock, flags);

	prev = counter->prev_cycle[evt];
	saved = counter->saved_cycle[evt];
	val = readq_relaxed(counter->reg + offset);
	if (!is_val_valid(val) || counter->offline)
		val = saved;

	if (val < prev) {
		if (val != saved)
			WARN_ONCE(1, "cpu=%d val=%llu prev=%llu saved=%llu\n", cpu,
					val, prev, saved);

		val = prev;
	}

	counter->total_cycle[evt] += val - prev;
	counter->prev_cycle[evt] = val;

	cycle_ret = counter->total_cycle[evt];

	spin_unlock_irqrestore(&counter->lock, flags);

	trace_xr_get_cpu_cycle(cpu, evt, val, prev, saved, cycle_ret);

	return cycle_ret;
}

static void xr_save_cpu_cycle_counter(int cpu, bool hp)
{
	struct cpu_counter *counter;
	unsigned long flags;

	counter = &per_cpu(xr_cpu_counter, cpu);

	spin_lock_irqsave(&counter->lock, flags);

	if (hp)
		counter->offline = true;

	counter->saved_cycle[CORECNT] = read_corecnt();
	counter->saved_cycle[CONSTCNT] = read_constcnt();

	spin_unlock_irqrestore(&counter->lock, flags);
}

static int cpuhp_walt_amu_offline(unsigned int cpu)
{
	xr_save_cpu_cycle_counter(cpu, true);

	return 0;
}

static int cpuhp_walt_amu_online(unsigned int cpu)
{
	struct cpu_counter *counter;
	unsigned long flags;

	counter = &per_cpu(xr_cpu_counter, cpu);

	spin_lock_irqsave(&counter->lock, flags);

	counter->offline = false;

	spin_unlock_irqrestore(&counter->lock, flags);

	return 0;
}

static void walt_amu_psci_cpu_suspend(void *unused, u32 state, bool *deny)
{
	int cpu = smp_processor_id();

	xr_save_cpu_cycle_counter(cpu, false);
}

int walt_amu_register_hooks(void)
{
	int ret;

	ret = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN, "walt_amu:online",
			cpuhp_walt_amu_online, cpuhp_walt_amu_offline);
	if (ret < 0)
		return ret;

	register_trace_android_rvh_psci_cpu_suspend(walt_amu_psci_cpu_suspend, NULL);

	return ret;
}

int walt_init_amu(void)
{
	struct cpu_counter *counter;
	struct device_node *cpu_np;
	int cpu = 0, ret;
	u32 raddr;

	for_each_possible_cpu(cpu) {
		cpu_np = of_cpu_device_node_get(cpu);
		if (cpu_np == NULL) {
			ret = -ENODEV;
			goto failed;
		}

		ret = of_property_read_u32(cpu_np, "xring,amu", &raddr);
		of_node_put(cpu_np);
		if (ret != 0) {
			pr_err("cpu%d has no amu addr!", cpu);
			goto failed;
		}

		counter = &per_cpu(xr_cpu_counter, cpu);
		/* ioremap percpu amu cpu cycle register address */
		counter->reg = ioremap(raddr, MAXCNT * sizeof(u64));
		if (!counter->reg) {
			ret = -EIO;
			goto failed;
		}

		memset(counter->total_cycle, 0, MAXCNT * sizeof(u64));
		memset(counter->prev_cycle, 0, MAXCNT * sizeof(u64));
		memset(counter->saved_cycle, 0, MAXCNT * sizeof(u64));
		spin_lock_init(&(counter->lock));
		counter->offline = false;
	}

	if (!xr_get_cpu_cycle_counter(smp_processor_id(), CORECNT)) {
		ret = -EINVAL;
		goto failed;
	}

	ret = walt_amu_register_hooks();
	if (ret < 0)
		goto failed;

	return 0;

failed:
	for_each_possible_cpu(cpu) {
		counter = &per_cpu(xr_cpu_counter, cpu);
		if (counter->reg)
			iounmap(counter->reg);
	}

	return ret;
}
