// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pm_opp.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <soc/xring/vote_mng.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <trace/hooks/power.h>
#include <trace/hooks/cpufreq.h>
#include <linux/sched.h>
#include <linux/pm_qos.h>

#define CREATE_TRACE_POINTS
#include "xr_trace.h"
#undef CREATE_TRACE_POINTS
#include "xr_cpufreq.h"

#define CPUFREQ_OFF_NAME	"xr_cpufreq_off"
#define XR_INIT_GOV(cpu)	(arch_scale_cpu_capacity(cpu) != SCHED_CAPACITY_SCALE ? "performance" : "powersave")
#define XR_TURBO_ATTR_INDEX	1

#define cpufreq_attr_available_freq(_name)	  \
struct freq_attr cpufreq_freq_attr_##_name##_freqs =     \
__ATTR_RO(_name##_frequencies)

static struct cpufreq_driver xr_cpufreq_driver;

struct private_data {
	struct list_head node;

	cpumask_var_t cpus;
	struct device *cpu_dev;
	struct device *pdev;
	struct opp_table *opp_table;
	struct cpufreq_frequency_table *freq_table;
	uint32_t boost_freq;
	uint32_t unboost_max_freq;
	bool have_static_opps;
	struct vote_mng *mng;
	int opp_token;
};

static LIST_HEAD(priv_list);
static unsigned int cpu_version = BIT(1);
static unsigned int vote_mng_status;


static uint32_t get_boost_freq(struct cpufreq_policy *policy)
{
	struct private_data *priv = policy->driver_data;

	return priv->boost_freq;
}

static uint32_t get_unboost_max_freq(struct cpufreq_policy *policy)
{
	struct private_data *priv = policy->driver_data;

	return priv->unboost_max_freq;
}

/*********************************************************************
 *                          SYSFS INTERFACE                          *
 *********************************************************************/

static ssize_t scaling_xr_boost_frequencies_show(struct cpufreq_policy *policy, char *buf)
{
	ssize_t count = 0;

	if (cpufreq_boost_enabled())
		count += sprintf(&buf[count], "%d ", get_boost_freq(policy));

	count += sprintf(&buf[count], "\n");

	return count;
}
cpufreq_attr_available_freq(scaling_xr_boost);

static struct freq_attr *xr_cpufreq_dt_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
	NULL,   /* Extra space for boost-attr if required */
};

static struct private_data *cpufreq_dt_find_data(int cpu)
{
	struct private_data *priv = NULL;

	list_for_each_entry(priv, &priv_list, node) {
		if (cpumask_test_cpu(cpu, priv->cpus))
			return priv;
	}

	return NULL;
}

#define DEFAULT_CHN		0
static int xr_cpufreq_target(struct cpufreq_policy *policy, unsigned int index)
{
	struct private_data *priv = policy->driver_data;
	uint32_t freq = (uint32_t)policy->freq_table[index].frequency;

	/* freq should be converted form KHz to MHz */
	if (vote_mng_status) {
		vote_mng_vote_dmd(priv->mng, DEFAULT_CHN, freq / 1000);
		pr_debug("%s: cpu=%u, index=%u, freq=%u\n", __func__,
				cpumask_first(priv->cpus), index, freq);
	}

	return 0;
}

#define CPU_CLUSTER_MAX		5
static void __maybe_unused cpu_profile_get(uint8_t cluster, uint32_t *freq_KHz,
			    uint16_t *logic_volt_mv, uint16_t *mem_volt_mv)
{
	struct arm_smccc_res res = { 0 };

	if (cluster >= CPU_CLUSTER_MAX) {
		pr_debug("%s: invalid cluster%u\n", __func__, cluster);
		return;
	}

	arm_smccc_smc(FID_BL31_CPU_PROFILE_GET, cluster, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0) {
		pr_debug("%s:arm_smccc_smc failed\n", __func__);
		return;
	}

	*freq_KHz = res.a1;
	*logic_volt_mv = res.a2;
	*mem_volt_mv = res.a3;
}

static void android_vh_freq_qos_update_request(void *unsed,
			struct freq_qos_request *req, int value)
{
	struct task_struct *task = current;
	struct cpufreq_policy *policy = NULL;
	unsigned int cpu = -1;
	struct cpumask tmp_mask;

	if (req == NULL || task == NULL || !trace_freq_qos_update_enabled())
		return;

	cpumask_copy(&tmp_mask, cpu_online_mask);

	for_each_cpu(cpu, &tmp_mask) {
		policy = cpufreq_cpu_get(cpu);
		if (policy == NULL)
			continue;

		if (req->qos == &(policy->constraints)) {
			if (req->type == FREQ_QOS_MAX)
				trace_freq_qos_update(value, task->pid,
						      task->comm, policy->cpu, "max");

			if (req->type == FREQ_QOS_MIN)
				trace_freq_qos_update(value, task->pid,
						      task->comm, policy->cpu, "min");
			cpufreq_cpu_put(policy);
			break;
		}
		cpufreq_cpu_put(policy);
		cpumask_andnot(&tmp_mask, &tmp_mask, policy->cpus);
	}
}

static void android_vh_freq_table_limits(void *used, struct cpufreq_policy *policy,
						unsigned int min_freq, unsigned int max_freq)
{
	if (policy->cpuinfo.max_freq > max_freq)
		policy->cpuinfo.max_freq = max_freq;
}

static void android_vh_cpufreq_resolve_freq(void *used, struct cpufreq_policy *policy,
						uint32_t *target_freq, uint32_t old_freq)
{
	uint32_t unboost_max_freq = get_unboost_max_freq(policy);

	if (!cpufreq_boost_enabled() && *target_freq > unboost_max_freq)
		*target_freq = unboost_max_freq;
}


static int xr_cpufreq_init(struct cpufreq_policy *policy)
{
	struct private_data *priv;
	struct device *cpu_dev;
	unsigned int transition_latency;
	int ret = 0;
	struct cpufreq_frequency_table *pos, *table;


	priv = cpufreq_dt_find_data(policy->cpu);
	if (!priv) {
		pr_err("failed to find data for cpu%d\n", policy->cpu);
		return -ENODEV;
	}
	cpu_dev = priv->cpu_dev;

	transition_latency = dev_pm_opp_get_max_transition_latency(cpu_dev);
	if (!transition_latency)
		transition_latency = CPUFREQ_ETERNAL;

	cpumask_copy(policy->cpus, priv->cpus);
	policy->driver_data = priv;
	policy->freq_table = priv->freq_table;
	policy->suspend_freq = dev_pm_opp_get_suspend_opp_freq(cpu_dev) / 1000;
	policy->cpuinfo.transition_latency = transition_latency;
	policy->dvfs_possible_from_any_cpu = true;
	policy->fast_switch_possible = true;
	strncpy(policy->last_governor, XR_INIT_GOV(policy->cpu), CPUFREQ_NAME_LEN);
	priv->unboost_max_freq = 0;
	priv->boost_freq = 0;

	/*boot freq & uboot_max_freq*/
	table = policy->freq_table;

	cpufreq_for_each_valid_entry(pos, table) {
		if (pos->flags & CPUFREQ_BOOST_FREQ) {
			priv->boost_freq = pos->frequency;
			continue;
		}

		if (priv->unboost_max_freq < pos->frequency)
			priv->unboost_max_freq = pos->frequency;
	}

	/* Support turbo/boost mode */
	if (policy_has_boost_freq(policy)) {
		/* This gets disabled by core on driver unregister */
		ret = cpufreq_enable_boost_support();
		if (ret)
			return ret;

		xr_cpufreq_dt_attr[XR_TURBO_ATTR_INDEX] = &cpufreq_freq_attr_scaling_xr_boost_freqs;
	}

	return 0;
}

static int xr_cpufreq_online(struct cpufreq_policy *policy)
{
	/* We did light-weight tear down earlier, nothing to do here */
	return 0;
}

static int xr_cpufreq_offline(struct cpufreq_policy *policy)
{
	/*
	 * Preserve policy->driver_data and don't free resources on light-weight
	 * tear down.
	 */
	return 0;
}

static int xr_cpufreq_exit(struct cpufreq_policy *policy)
{
	return 0;
}

static unsigned int xr_cpufreq_get(unsigned int cpu)
{
	struct cpufreq_policy *policy = cpufreq_cpu_get_raw(cpu);
	struct private_data *priv;
	struct vote_mng_mode2_result vote_result = {
		.arb_ch = 0,
		.max = 0,
		.min = 0,
		.dmd = 0,
	};
	unsigned int index, freq;

	if (policy == NULL) {
		pr_err("%s: No policy associated to cpu: %u\n", __func__, cpu);
		return 0;
	}

	priv = policy->driver_data;

	if (vote_mng_status)
		vote_mng_mode2_result_dump(priv->mng, &vote_result);

	if (vote_result.dmd) {
		freq = vote_result.dmd * 1000;
		index = cpufreq_frequency_table_target(policy,
						    freq, CPUFREQ_RELATION_H);
		freq = policy->freq_table[index].frequency;
		return clamp(freq, policy->cpuinfo.min_freq,
				      policy->cpuinfo.max_freq);
	}

	pr_debug("%s: cur=%u, min=%u, max=%u\n", __func__, policy->cur, policy->min, policy->max);
	return max(policy->cur, policy->min);
}

static unsigned int xr_cpufreq_hw_fast_switch(struct cpufreq_policy *policy,
						unsigned int target_freq)
{
	struct private_data *priv = policy->driver_data;

	/* freq should be converted form KHz to MHz */
	if (vote_mng_status) {
		vote_mng_vote_dmd(priv->mng, DEFAULT_CHN, target_freq / 1000);
		pr_debug("fast_switch policy:%d target_freq:%d\n", policy->cpu, target_freq);

		return target_freq;
	}

	return 0;
}

static void xr_unboost_verify_limits(struct cpufreq_policy_data *policy_data)
{
	uint32_t unboost_max_freq = 0;
	struct cpufreq_frequency_table *pos;

	cpufreq_for_each_valid_entry(pos, policy_data->freq_table) {
		if (pos->flags & CPUFREQ_BOOST_FREQ)
			continue;

		if (unboost_max_freq < pos->frequency)
			unboost_max_freq = pos->frequency;
	}

	if (policy_data->max > unboost_max_freq)
		policy_data->max = unboost_max_freq;

	if (policy_data->min > unboost_max_freq)
		policy_data->min = unboost_max_freq;
}

static int xr_cpufreq_verify(struct cpufreq_policy_data *policy_data)
{
	int ret = 0;

	if (!policy_data->freq_table)
		return -ENODEV;

	if (!cpufreq_boost_enabled())
		xr_unboost_verify_limits(policy_data);

	ret = cpufreq_generic_frequency_table_verify(policy_data);
	if (ret)
		return ret;

	return 0;
}

#define BOOST_CHN 1
static void xr_cpufreq_adjust(unsigned int cpu, unsigned int min_freq)
{
	struct cpufreq_policy *policy = NULL;
	struct private_data *priv = NULL;

	policy = cpufreq_cpu_get_raw(cpu);
	if (policy != NULL) {
		priv = policy->driver_data;
		if (vote_mng_status) {
			vote_mng_vote_dmd(priv->mng, BOOST_CHN, min_freq / 1000);
			pr_debug("policy:%d target_freq:%u\n", cpu, min_freq);
		}
	}
}

static struct cpufreq_driver xr_cpufreq_driver = {
	.flags = CPUFREQ_NEED_INITIAL_FREQ_CHECK |
		 CPUFREQ_IS_COOLING_DEV |
		 CPUFREQ_HAVE_GOVERNOR_PER_POLICY,
	.driver_data = (void *)xr_cpufreq_adjust,
	.verify = xr_cpufreq_verify,
	.target_index = xr_cpufreq_target,
	.get = xr_cpufreq_get,
	.init = xr_cpufreq_init,
	.exit = xr_cpufreq_exit,
	.online = xr_cpufreq_online,
	.offline = xr_cpufreq_offline,
	.register_em = cpufreq_register_em_with_opp,
	.fast_switch = xr_cpufreq_hw_fast_switch,
	.name = "xr_cpufreq",
	.attr = xr_cpufreq_dt_attr,
	.suspend = cpufreq_generic_suspend,
	.boost_enabled = false,
};

static inline int cpu_to_cluster(int cpu)
{
	return topology_cluster_id(cpu);
}

#define NAME_LEN	32
static int xr_cpufreq_early_init(struct device *dev, int cpu)
{
	struct private_data *priv;
	struct device *cpu_dev;
	struct vote_mng *mng;
	char name[NAME_LEN];
	int ret;

	/* Check if this CPU is already covered by some other policy */
	if (cpufreq_dt_find_data(cpu))
		return 0;

	cpu_dev = get_cpu_device(cpu);
	if (!cpu_dev)
		return -EPROBE_DEFER;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	if (!alloc_cpumask_var(&priv->cpus, GFP_KERNEL))
		return -ENOMEM;

	cpumask_set_cpu(cpu, priv->cpus);
	priv->cpu_dev = cpu_dev;
	priv->pdev = dev;

	/* Get OPP-sharing information from "operating-points-v2" bindings */
	ret = dev_pm_opp_of_get_sharing_cpus(cpu_dev, priv->cpus);
	if (ret)
		goto out;

	ret = dev_pm_opp_set_supported_hw(cpu_dev, &cpu_version, 1);
	if (ret < 0)
		goto out;

	priv->opp_token = ret;
	/*
	 * Initialize OPP tables for all priv->cpus. They will be shared by
	 * all CPUs which have marked their CPUs shared with OPP bindings.
	 *
	 * For platforms not using operating-points-v2 bindings, we do this
	 * before updating priv->cpus. Otherwise, we will end up creating
	 * duplicate OPPs for the CPUs.
	 *
	 * OPPs might be populated at runtime, don't fail for error here unless
	 * it is -EPROBE_DEFER.
	 */
	ret = dev_pm_opp_of_cpumask_add_table(priv->cpus);
	if (!ret)
		priv->have_static_opps = true;
	else if (ret == -EPROBE_DEFER)
		goto out;

	/*
	 * The OPP table must be initialized, statically or dynamically, by this
	 * point.
	 */
	ret = dev_pm_opp_get_opp_count(cpu_dev);
	if (ret <= 0) {
		dev_err(cpu_dev, "OPP table can't be empty\n");
		ret = -ENODEV;
		goto out;
	}

	ret = dev_pm_opp_init_cpufreq_table(cpu_dev, &priv->freq_table);
	if (ret) {
		dev_err(cpu_dev, "failed to init cpufreq table: %d\n", ret);
		goto out;
	}

	snprintf(name, sizeof(name), "freq_mng%u", cpu_to_cluster(cpu));
	mng = vote_mng_get(dev, name);
	if (IS_ERR_OR_NULL(mng)) {
		dev_err(cpu_dev, "failed get %s, %ld\n",
				name, PTR_ERR(mng));
		ret = -EINVAL;
		goto out;
	}

	priv->mng = mng;

	list_add(&priv->node, &priv_list);
	return 0;

out:
	if (priv->opp_token)
		dev_pm_opp_put_supported_hw(priv->opp_token);

	if (priv->have_static_opps)
		dev_pm_opp_of_cpumask_remove_table(priv->cpus);

	free_cpumask_var(priv->cpus);
	return ret;
}

static void xr_cpufreq_release(void)
{
	struct private_data *priv = NULL;
	struct private_data *tmp = NULL;

	list_for_each_entry_safe(priv, tmp, &priv_list, node) {
		dev_pm_opp_free_cpufreq_table(priv->cpu_dev, &priv->freq_table);
		if (priv->have_static_opps)
			dev_pm_opp_of_cpumask_remove_table(priv->cpus);

		if (priv->opp_token)
			dev_pm_opp_put_supported_hw(priv->opp_token);

		if (priv->mng)
			vote_mng_put(priv->pdev, priv->mng);

		free_cpumask_var(priv->cpus);
		list_del(&priv->node);
	}
}

static int xr_cpufreq_probe(struct platform_device *pdev)
{
	int cpu = -1;
	int ret;

	/* Request resources early so we can return in case of -EPROBE_DEFER */
	for_each_possible_cpu(cpu) {
		ret = xr_cpufreq_early_init(&pdev->dev, cpu);
		if (ret)
			goto err;
	}

	ret = of_property_read_u32((&pdev->dev)->of_node,
					"vote-mng-status",
					&vote_mng_status);
	if (ret) {
		dev_err(&pdev->dev, "failed to get vote_mng status: %d\n", ret);
		vote_mng_status = 0;
	}

	if (vote_mng_status == 0)
		strcpy(xr_cpufreq_driver.name, CPUFREQ_OFF_NAME);

	ret = cpufreq_register_driver(&xr_cpufreq_driver);
	if (ret) {
		dev_err(&pdev->dev, "failed register driver: %d\n", ret);
		goto err;
	}

	register_trace_android_vh_freq_qos_update_request(
					android_vh_freq_qos_update_request, NULL);
	register_trace_android_vh_freq_table_limits(android_vh_freq_table_limits, NULL);
	register_trace_android_vh_cpufreq_resolve_freq(android_vh_cpufreq_resolve_freq, NULL);

	ret = xr_cpufreq_qos_init((&pdev->dev)->of_node);
	if (ret)
		pr_err("xr_cpufreq_qos_init failed: %d\n", ret);

	return 0;
err:
	xr_cpufreq_release();
	return ret;
}

static int xr_cpufreq_remove(struct platform_device *pdev)
{
	xr_cpufreq_qos_exit();
	unregister_trace_android_vh_freq_qos_update_request(
					android_vh_freq_qos_update_request, NULL);
	unregister_trace_android_vh_freq_table_limits(android_vh_freq_table_limits, NULL);
	unregister_trace_android_vh_cpufreq_resolve_freq(android_vh_cpufreq_resolve_freq, NULL);

	cpufreq_unregister_driver(&xr_cpufreq_driver);
	xr_cpufreq_release();
	return 0;
}

static const struct of_device_id xr_cpufreq_match_table[] = {
	{.compatible = "xring,xr-cpufreq"},
	{}
};

static struct platform_driver xr_cpufreq_platdrv = {
	.driver = {
		.name	= "xr_cpufreq",
		.of_match_table = xr_cpufreq_match_table,
	},
	.probe		= xr_cpufreq_probe,
	.remove		= xr_cpufreq_remove,
};

static int __init xr_cpufreq_drv_init(void)
{
	int ret;

	ret = platform_driver_register(&xr_cpufreq_platdrv);
	if (ret) {
		pr_err("platdrv register failed, %d\n", ret);
		return ret;
	}

	return 0;
}
module_init(xr_cpufreq_drv_init);

static void __exit xr_cpufreq_drv_exit(void)
{
	platform_driver_unregister(&xr_cpufreq_platdrv);
}
module_exit(xr_cpufreq_drv_exit);

MODULE_AUTHOR("Fang Xiang <fangxiang3@xiaomi.com>");
MODULE_DESCRIPTION("XRing cpufreq driver");
MODULE_LICENSE("GPL");
