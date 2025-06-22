// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#define MODULE_NAME "dse"
#define pr_fmt(fmt) MODULE_NAME ": " fmt

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/arm-smccc.h>
#include <linux/cpu.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <soc/xring/dse.h>

/* EL3 external registers */
#define CLUSTER_CONTROL_OFFSET 0x000000
#define CLUSTERPPMCR_OFFSET (CLUSTER_CONTROL_OFFSET + 0x0080)
#define CLUSTERMPMMCR_OFFSET (CLUSTER_CONTROL_OFFSET + 0x0088)

#define MODE_RO 0440
#define MODE_RW 0640

#define CPU_NUM 10
u64 g_sysreg_backup[CPU_NUM][SYSREG_NUM];
DECLARE_BITMAP(g_cpus_sysreg_has_backup[CPU_NUM], SYSREG_NUM);

static u64 read_extreg64(struct device *dev, u32 offset)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_CPU_REG_READ_EXTREG, offset, 64, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		dev_err(dev, "Read extreg64 fail. Offset: 0x%x\n", offset);
		return -EINVAL;
	}
	return (u64)res.a1;
}

static int write_extreg64(struct device *dev, u32 offset, u64 val)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_CPU_REG_WRITE_EXTREG, offset, 64, val, 0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		dev_err(dev, "Write extreg64 fail. Offset: 0x%x, val: 0x%llx\n", offset, val);
		return -EINVAL;
	}
	return 0;
}

#define EXTREG64_SHOW_FUNC(reg_name) \
static ssize_t reg_name##_show( \
	struct device *dev, struct device_attribute __maybe_unused *attr, char *buf) \
{ \
	return sprintf(buf, "0x%llx\n", read_extreg64(dev, reg_name##_OFFSET)); \
}

#define EXTREG64_STORE_FUNC(reg_name) \
static ssize_t reg_name##_store( \
	struct device *dev, struct device_attribute __maybe_unused *attr, \
	const char *buf, size_t count) \
{ \
	u64 input; \
	int ret; \
\
	ret = kstrtoull(buf, 0, &input); \
	if (ret != 0) { \
		dev_err(dev, "input must be u64.\n"); \
		return -EINVAL; \
	} \
\
	write_extreg64(dev, reg_name##_OFFSET, input); \
	return count; \
}

EXTREG64_SHOW_FUNC(CLUSTERPPMCR);
EXTREG64_STORE_FUNC(CLUSTERPPMCR);
static DEVICE_ATTR(CLUSTERPPMCR, MODE_RW, CLUSTERPPMCR_show, CLUSTERPPMCR_store);

EXTREG64_SHOW_FUNC(CLUSTERMPMMCR);
EXTREG64_STORE_FUNC(CLUSTERMPMMCR);
static DEVICE_ATTR(CLUSTERMPMMCR, MODE_RW, CLUSTERMPMMCR_show, CLUSTERMPMMCR_store);

/* EL3 system registers */
static u64 read_sysreg64(enum sysreg_id id)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_CPU_REG_READ_SYSREG, id, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		pr_err("Failed to read from sysreg id %u.\n", id);
		return 0;
	}
	return (u64)res.a1;
}

static void write_sysreg64(enum sysreg_id id, u64 val)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_CPU_REG_WRITE_SYSREG, id, val, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("Failed to write to sysreg id %u, val 0x%llx\n", id, val);
}

struct func_args_t {
	enum sysreg_id id;
	u64 value;
};

static void read_sysreg_handle(void *info)
{
	struct func_args_t *args = (struct func_args_t *)info;

	args->value = read_sysreg64(args->id);
}
static void write_sysreg_handle(void *info)
{
	struct func_args_t *args = (struct func_args_t *)info;

	write_sysreg64(args->id, args->value);
}

u64 read_dse_sysreg(int cpu_id, enum sysreg_id id)
{
	int ret;
	struct func_args_t args = {id, 0};

	if (cpu_id < 0 || cpu_id >= CPU_NUM || id >= SYSREG_NUM)
		return 0;

	cpus_read_lock();
	ret = smp_call_function_single(cpu_id, read_sysreg_handle, (void *)&args, true);
	cpus_read_unlock();
	if (ret == 0)
		return args.value;

	return g_sysreg_backup[cpu_id][id];
}
EXPORT_SYMBOL(read_dse_sysreg);

void write_dse_sysreg(int cpu_id, enum sysreg_id id, u64 val)
{
	int ret;
	struct func_args_t args = {id, val};

	if (cpu_id < 0 || cpu_id >= CPU_NUM || id >= SYSREG_NUM)
		return;

	cpus_read_lock();
	ret = smp_call_function_single(cpu_id, write_sysreg_handle, (void *)&args, true);
	if (ret != 0) {
		g_sysreg_backup[cpu_id][id] = val;
		set_bit(id, g_cpus_sysreg_has_backup[cpu_id]);
	}
	cpus_read_unlock();
}
EXPORT_SYMBOL(write_dse_sysreg);

static int cpu_online_set_dse_sysreg(unsigned int cpu)
{
	enum sysreg_id id;

	for_each_set_bit(id, g_cpus_sysreg_has_backup[cpu], SYSREG_NUM) {
		write_sysreg64(id, g_sysreg_backup[cpu][id]);
		clear_bit(id, g_cpus_sysreg_has_backup[cpu]);
	}

	return 0;
}

static int cpu_offline_save_dse_sysreg(unsigned int cpu)
{
	enum sysreg_id id;

	id = SYSREG_ID(CPUMPMMCR);
	g_sysreg_backup[cpu][id] = read_sysreg64(id);

	id = SYSREG_ID(CPUMPMMTUNE);
	g_sysreg_backup[cpu][id] = read_sysreg64(id);

	return 0;
}

struct node_args_t {
	char *show_buf;
	const char *store_buf;
	enum sysreg_id id;
	size_t count;
};

static void sysreg_show(void *info)
{
	struct node_args_t *args = (struct node_args_t *)info;
	u64 value;

	value = read_sysreg64(args->id);
	args->count = sprintf(args->show_buf, "0x%llx\n", value);
}
static void sysreg_store(void *info)
{
	struct node_args_t *args = (struct node_args_t *)info;
	u64 input;
	int ret;

	ret = kstrtoull(args->store_buf, 0, &input);
	if (ret != 0) {
		pr_err("input must be u64.\n");
		return;
	}

	write_sysreg64(args->id, input);
}

#define SYSREG_SHOW_FUNC(cpu_id, reg_name) \
static ssize_t reg_name##_c##cpu_id##_show( \
	struct device *dev, struct device_attribute __maybe_unused *attr, char *buf) \
{ \
	struct node_args_t args = {buf, NULL, SYSREG_ID(reg_name)}; \
\
	smp_call_function_single(cpu_id, sysreg_show, (void *)&args, true); \
	return args.count; \
}

#define SYSREG_STORE_FUNC(cpu_id, reg_name) \
static ssize_t reg_name##_c##cpu_id##_store( \
	struct device *dev, struct device_attribute __maybe_unused *attr, \
	const char *buf, size_t count) \
{ \
	struct node_args_t args = {NULL, buf, SYSREG_ID(reg_name)}; \
\
	smp_call_function_single(cpu_id, sysreg_store, (void *)&args, true); \
	return count; \
}

#define MAKE_SYSREG_RO(cpu_id, reg_name) \
SYSREG_SHOW_FUNC(cpu_id, reg_name); \
static DEVICE_ATTR( \
	reg_name##_c##cpu_id, MODE_RO, \
	reg_name##_c##cpu_id##_show, NULL)

#define MAKE_SYSREG_RW(cpu_id, reg_name) \
SYSREG_SHOW_FUNC(cpu_id, reg_name); \
SYSREG_STORE_FUNC(cpu_id, reg_name); \
static DEVICE_ATTR( \
	reg_name##_c##cpu_id, MODE_RW, \
	reg_name##_c##cpu_id##_show, reg_name##_c##cpu_id##_store)

/* All sysfs nodes */
#define MAKE_CPUL_REGS(cpu_id) \
MAKE_SYSREG_RW(cpu_id, CPUPPMCR); \
MAKE_SYSREG_RO(cpu_id, CPUMPMMCR); \
MAKE_SYSREG_RO(cpu_id, CPUMPMMTUNE);  /* maybe exist */ \
MAKE_SYSREG_RW(cpu_id, CPUECTLR); \
MAKE_SYSREG_RW(cpu_id, CMPXECTLR)

#define MAKE_CPUM_REGS(cpu_id) \
MAKE_SYSREG_RW(cpu_id, CPUPPMCR); \
MAKE_SYSREG_RO(cpu_id, CPUMPMMCR); \
MAKE_SYSREG_RW(cpu_id, CPUPPMPDPCR); \
MAKE_SYSREG_RO(cpu_id, CPUMPMMTUNE);  /* maybe exist */ \
MAKE_SYSREG_RW(cpu_id, CPUPDPTUNE);  /* maybe exist */ \
MAKE_SYSREG_RW(cpu_id, CPUPDPTUNE2);  /* maybe exist */ \
MAKE_SYSREG_RW(cpu_id, CPUECTLR); \
MAKE_SYSREG_RW(cpu_id, CPUECTLR2)

#define MAKE_CPUB_REGS(cpu_id) \
MAKE_SYSREG_RW(cpu_id, CPUPPMCR); \
MAKE_SYSREG_RO(cpu_id, CPUMPMMCR); \
MAKE_SYSREG_RW(cpu_id, CPUPPMPDPCR); \
MAKE_SYSREG_RO(cpu_id, CPUMPMMTUNE); \
MAKE_SYSREG_RW(cpu_id, CPUPDPTUNE); \
MAKE_SYSREG_RW(cpu_id, CPUPDPTUNE2); \
MAKE_SYSREG_RW(cpu_id, CPUECTLR); \
MAKE_SYSREG_RW(cpu_id, CPUECTLR2)

MAKE_CPUL_REGS(0);
MAKE_CPUL_REGS(1);
MAKE_CPUM_REGS(2);
MAKE_CPUM_REGS(3);
MAKE_CPUM_REGS(4);
MAKE_CPUM_REGS(5);
MAKE_CPUM_REGS(6);
MAKE_CPUM_REGS(7);
MAKE_CPUB_REGS(8);
MAKE_CPUB_REGS(9);

#define ADD_CPUL_ATTRS(cpu_id) \
&dev_attr_CPUPPMCR_c##cpu_id.attr, \
&dev_attr_CPUMPMMCR_c##cpu_id.attr, \
&dev_attr_CPUMPMMTUNE_c##cpu_id.attr, \
&dev_attr_CPUECTLR_c##cpu_id.attr, \
&dev_attr_CMPXECTLR_c##cpu_id.attr

#define ADD_CPUM_ATTRS(cpu_id) \
&dev_attr_CPUPPMCR_c##cpu_id.attr, \
&dev_attr_CPUMPMMCR_c##cpu_id.attr, \
&dev_attr_CPUPPMPDPCR_c##cpu_id.attr, \
&dev_attr_CPUMPMMTUNE_c##cpu_id.attr, \
&dev_attr_CPUPDPTUNE_c##cpu_id.attr, \
&dev_attr_CPUPDPTUNE2_c##cpu_id.attr, \
&dev_attr_CPUECTLR_c##cpu_id.attr, \
&dev_attr_CPUECTLR2_c##cpu_id.attr

#define ADD_CPUB_ATTRS(cpu_id) \
&dev_attr_CPUPPMCR_c##cpu_id.attr, \
&dev_attr_CPUMPMMCR_c##cpu_id.attr, \
&dev_attr_CPUPPMPDPCR_c##cpu_id.attr, \
&dev_attr_CPUMPMMTUNE_c##cpu_id.attr, \
&dev_attr_CPUPDPTUNE_c##cpu_id.attr, \
&dev_attr_CPUPDPTUNE2_c##cpu_id.attr, \
&dev_attr_CPUECTLR_c##cpu_id.attr, \
&dev_attr_CPUECTLR2_c##cpu_id.attr

static struct attribute *g_dse_attrs[] = {
	&dev_attr_CLUSTERPPMCR.attr,
	&dev_attr_CLUSTERMPMMCR.attr,
	ADD_CPUL_ATTRS(0),
	ADD_CPUL_ATTRS(1),
	ADD_CPUM_ATTRS(2),
	ADD_CPUM_ATTRS(3),
	ADD_CPUM_ATTRS(4),
	ADD_CPUM_ATTRS(5),
	ADD_CPUM_ATTRS(6),
	ADD_CPUM_ATTRS(7),
	ADD_CPUB_ATTRS(8),
	ADD_CPUB_ATTRS(9),
	NULL,
};

static struct attribute_group g_dse_attr_group = {
	.attrs = g_dse_attrs,
};

static int dse_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret = 0;

	ret = cpuhp_setup_state_nocalls(
		CPUHP_AP_ONLINE_DYN, "dse:online",
		cpu_online_set_dse_sysreg, cpu_offline_save_dse_sysreg);
	if (ret < 0) {
		dev_err(dev, "cpuhp_setup_state_nocalls failed\n");
		return ret;
	}

	ret = sysfs_create_group(&dev->kobj, &g_dse_attr_group);
	if (ret)
		dev_err(dev, "Failed in sysfs_create_group!\n");

	return ret;
}

static const struct of_device_id g_dse_of_match[] = {
	{
		.compatible = "xring,dse",
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_dse_of_match);

static struct platform_driver g_dse_driver = {
	.probe = dse_probe,
	.driver = {
		.name = "dse",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_dse_of_match),
	},
};

static int __init dse_init(void)
{
	return platform_driver_register(&g_dse_driver);
}
module_init(dse_init);

MODULE_AUTHOR("Aoyang Yan <yanaoyang@xiaomi.com>");
MODULE_DESCRIPTION("XRing DSE Driver");
MODULE_LICENSE("GPL v2");
