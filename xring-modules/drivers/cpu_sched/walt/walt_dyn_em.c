// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/err.h>
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/energy_model.h>
#include <linux/sched/cputime.h>

#include "walt.h"

#ifdef pr_fmt
#undef pr_fmt
#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt
#endif

void xr_update_em_table(int cpu, int em)
{
	struct walt_sched_cluster *cluster;
	struct device *dev;
	struct em_perf_table *table;

	if (cpu_em[0] < 0 || cpu_em[0] >= WALT_NR_CPUS ||
			cpu_em[1] < EM_NORMAL || cpu_em[1] >= EM_MAX)
		return;

	cluster = cpu_cluster(cpu);
	dev = cluster->em_ctx[em].dev;
	table = cluster->em_ctx[em].table;
	if (!dev || !table) {
		pr_err("cpu%d em%d is invalid\n", cpu, em);
		return;
	}
	em_dev_update_perf_domain(dev, table);
}

static struct em_perf_table __rcu *alloc_em_table(struct em_perf_domain *pd)
{
	struct em_perf_table __rcu *table;
	int table_size;

	table_size = sizeof(struct em_perf_state) * pd->nr_perf_states;

	table = kzalloc(sizeof(*table) + table_size, GFP_KERNEL);
	if (!table)
		return NULL;

	kref_init(&table->kref);

	return table;
}

static void em_destroy_table_rcu(struct rcu_head *rp)
{
	struct em_perf_table __rcu *table;

	table = container_of(rp, struct em_perf_table, rcu);
	kfree(table);
}

static void em_release_table_kref(struct kref *kref)
{
	struct em_perf_table __rcu *table;

	/* It was the last owner of this table so we can free */
	table = container_of(kref, struct em_perf_table, kref);

	call_rcu(&table->rcu, em_destroy_table_rcu);
}

static void free_em_table(struct em_perf_table __rcu *table)
{
	kref_put(&table->kref, em_release_table_kref);
}

static int em_compute_costs(struct device *dev, struct em_perf_state *table,
		int nr_states)
{
	unsigned long prev_cost = ULONG_MAX;
	int i;

	/* Compute the cost of each performance state. */
	for (i = nr_states - 1; i >= 0; i--) {
		unsigned long power_res, cost;

		/* increase resolution of 'cost' precision */
		power_res = table[i].power * 10;
		cost = power_res / table[i].performance;

		table[i].cost = cost;

		if (table[i].cost >= prev_cost) {
			table[i].flags = EM_PERF_STATE_INEFFICIENT;
			dev_dbg(dev, "EM: OPP:%lu is inefficient\n",
				table[i].frequency);
		} else {
			prev_cost = table[i].cost;
		}
	}

	return 0;
}

static void normal_fixup_em_table(struct device *dev, struct em_perf_state *table,
		int nr_states)
{

}

static void avs_fixup_em_table(struct device *dev, struct em_perf_state *table,
		int nr_states)
{
	int i;
	struct em_perf_state *tmp = table;

	for (i = 0; i < nr_states; i++) {
		tmp->performance += 10;
		tmp->power += 10;
		tmp++;
	}
}

static void thermal_fixup_em_table(struct device *dev, struct em_perf_state *table,
		int nr_states)
{
	int i;
	struct em_perf_state *tmp = table;

	for (i = 0; i < nr_states; i++) {
		tmp->performance += 20;
		tmp->power += 20;
		tmp++;
	}
}

struct fixup_func_list {
	void (*fixup_em_table)(struct device *dev, struct em_perf_state *table,
			int nr_states);
} ffl[EM_MAX] = {
	{ .fixup_em_table = normal_fixup_em_table, },
	{ .fixup_em_table = avs_fixup_em_table, },
	{ .fixup_em_table = thermal_fixup_em_table, },
};

static void xr_create_new_em(struct dyn_em_ctx *ctx)
{
	struct em_perf_table __rcu *em_table;
	struct em_perf_state *table, *new_table;
	struct device *dev = ctx->dev;
	struct em_perf_domain *pd = ctx->pd;
	int ret, ps_size;

	em_table = alloc_em_table(pd);
	if (!em_table) {
		pr_err("em table alloc failed\n");
		return;
	}

	new_table = em_table->state;

	rcu_read_lock();
	/* Initialize data based on old table */
	table = em_perf_state_from_pd(pd);
	ps_size = sizeof(struct em_perf_state) * pd->nr_perf_states;
	memcpy(new_table, table, ps_size);
	ctx->fixup_em_table(dev, new_table, pd->nr_perf_states);
	rcu_read_unlock();

	ret = em_compute_costs(dev, table, pd->nr_perf_states);
	if (ret) {
		dev_warn(dev, "EM: compute costs failed %d\n", ret);
		free_em_table(em_table);
		return;
	}

	ctx->table = em_table;
}

void xr_init_dyn_em_table(void)
{
	struct walt_sched_cluster *cluster = NULL;
	int i;

	for_each_sched_cluster(cluster) {
		for (i = 0; i < EM_MAX; i++)
			xr_create_new_em(&cluster->em_ctx[i]);
	}
}

void xr_init_dyn_em_ctx(void)
{
	struct walt_sched_cluster *cluster = NULL;
	int i, cpu;

	for_each_sched_cluster(cluster) {
		for (i = 0; i < EM_MAX; i++) {
			cpu = cpumask_first(&cluster->cpus);
			cluster->em_ctx[i].dev = get_cpu_device(cpu);
			cluster->em_ctx[i].pd = em_cpu_get(cpu);
			cluster->em_ctx[i].fixup_em_table = ffl[i].fixup_em_table;
		}
	}
}

static int dyn_em_update_handler(struct ctl_table *table, int write,
				void __user *buffer, size_t *lenp,
				loff_t *ppos)
{
	int ret;
	int cpu_em[2] = {-1, -1};

	struct ctl_table tmp = {
		.data = &cpu_em,
		.maxlen = sizeof(cpu_em),
		.mode = table->mode,
	};

	if (!write) {
		ret = proc_dointvec(&tmp, write, buffer, lenp, ppos);
		return ret;
	}

	ret = proc_dointvec(&tmp, write, buffer, lenp, ppos);
	if (ret)
		return ret;

	pr_debug("%s: cpu %d, em %d\n", __func__, cpu_em[0], cpu_em[1]);
	if (cpu_em[0] < 0 || cpu_em[0] >= WALT_NR_CPUS ||
			cpu_em[1] < EM_NORMAL || cpu_em[1] >= EM_MAX)
		return ret;

	xr_update_em_table(cpu_em[0], cpu_em[1]);

	return ret;

}

static struct ctl_table dyn_em_table[] = {
	{
		.procname	= "xr_dyn_em",
		.maxlen		= sizeof(int) * 2,
		.mode		= 0644,
		.proc_handler	= dyn_em_update_handler,
	},
};

void xr_dyn_em_init(void)
{
	xr_init_dyn_em_ctx();
	xr_init_dyn_em_table();
	if (!register_sysctl("walt", dyn_em_table))
		pr_err("dyn em sysctl register failed\n");
}
