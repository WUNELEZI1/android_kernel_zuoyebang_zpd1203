// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 *
 * Description: gic routing driver
 */

#include <linux/bits.h>
#include <linux/cpuhotplug.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdesc.h>
#include <linux/irqnr.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

#include <linux/irqchip/arm-gic-v3.h>
#include <trace/hooks/gic_v3.h>

#include <linux/tracepoint.h>
#include <trace/events/irq.h>

#define TAG	"xring gic intr routing: "

#define NUM_CLASS_CPUS	NR_CPUS
#define GIC_INTERRUPT_ROUTING_MODE	BIT(31)
#define GICD_ICLAR2		0xE008
#define GICD_TYPER_1_OF_N	BIT(25)
#define MAX_IRQS		1020U
#define GIC_CLASS_0_CPU		0
#define GIC_V3_NAME		"GICv3"

extern int irq_do_set_affinity(struct irq_data *data,
			       const struct cpumask *dest, bool force);

struct gic_intr_routing_data {
	struct irq_chip *gic_chip;
	cpumask_t gic_routing_class0_cpus;
	cpumask_t gic_routing_class1_cpus;
	cpumask_t class0_active_cpus;
	enum cpuhp_state gic_affinity_cpuhp_state;
	bool gic_supports_1_of_N;
	bool gic_1_of_N_init_done;
	bool gic_enable_class0;
};

static struct gic_intr_routing_data gic_routing_data;

static DEFINE_SPINLOCK(gic_class_lock);
static DEFINE_SPINLOCK(gic_init_lock);

static DECLARE_BITMAP(active_gic_class0, MAX_IRQS);
static DECLARE_BITMAP(gic_saved_class0, MAX_IRQS);

static bool gicd_typer_1_of_N_supported(void __iomem *base)
{
	return !(readl_relaxed(base + GICD_TYPER) & GICD_TYPER_1_OF_N);
}

static void gic_enable_1_of_N(void __iomem *base)
{
	gic_routing_data.gic_supports_1_of_N =
		gicd_typer_1_of_N_supported(base);
}

static bool is_gic_chip(struct irq_desc *desc, struct irq_chip *gic_chip)
{
	struct irq_data *data = irq_desc_get_irq_data(desc);
	struct irq_chip *chip = irq_data_get_irq_chip(data);

	if (!chip)
		return false;

	if (gic_chip)
		return (gic_chip == chip);
	else
		return !strcmp(chip->name, GIC_V3_NAME);
}

static bool need_affinity_setting(struct irq_desc *desc,
					struct irq_chip *gic_chip,
					bool check_saved_class)
{
	bool need_affinity;
	struct irq_data *data = irq_desc_get_irq_data(desc);
	u32 irq = data->hwirq - 32;

	if (irq >= MAX_IRQS)
		return false;

	need_affinity = is_gic_chip(desc, gic_chip);

	if (!need_affinity)
		return false;
	if (check_saved_class &&
		!test_bit(irq, active_gic_class0))
		need_affinity = (cpumask_weight(desc->irq_common_data.affinity) > 1);

	return need_affinity;
}

static bool gic_need_class_update(u32 irq, bool is_class0)
{
	pr_debug(TAG "class_set: %d is_class0: %d test-class0: %d\n",
		test_bit(irq, gic_saved_class0), is_class0,
		test_bit(irq, active_gic_class0));

	if (test_bit(irq, gic_saved_class0) &&
		(is_class0 == !!test_bit(irq, active_gic_class0)))
		return false;

	return true;
}

void gic_do_class_update(void __iomem *base, u32 irq,
			  bool is_class0)
{
	void __iomem *reg = base + GICD_ICLAR2 + (irq / 16) * 4;
	unsigned int val, offset, class_bits_val = 0;

	if (is_class0)
		class_bits_val = 0x2;

	val = readl_relaxed(reg);
	offset = (irq % 16) << 1;
	val &= ~(0x3 << offset);
	val |= class_bits_val << offset;
	writel_relaxed(val, reg);
	pr_debug(TAG "Set class of hwirq: %d class: %#x\n", (irq + 32), val);
}

void gic_routing_set_affinity(unsigned int irq, unsigned int enable)
{
	struct irq_chip *gic_chip = NULL;
	struct irq_desc *desc;
	unsigned long hwirq, flags;
	int err;
	bool affinity_setting = false;
	cpumask_t affinity = { CPU_BITS_NONE };
	struct irq_data *d;

	if (READ_ONCE(gic_routing_data.gic_1_of_N_init_done)) {
		/* do memory barrier here */
		smp_rmb();
		gic_chip = READ_ONCE(gic_routing_data.gic_chip);
	}

	cpus_read_lock();
	local_irq_save(flags);
	desc = irq_to_desc(irq);
	if (!desc)
		goto out_cpus_lock;

	raw_spin_lock(&desc->lock);
	affinity_setting = need_affinity_setting(
		desc, gic_chip, true);
	if (!affinity_setting)
		goto out_desc_lock;

	d = irq_desc_get_irq_data(desc);
	hwirq = d->hwirq;

	spin_lock(&gic_class_lock);
	if (enable) {
		set_bit(hwirq - 32, gic_saved_class0);
	} else if (!gic_routing_data.gic_enable_class0) {
		clear_bit(hwirq - 32, gic_saved_class0);
	} else {
		spin_unlock(&gic_class_lock);
		pr_warn_once(TAG "Not Support disable specific 1 of N SPI\n");
		goto out_desc_lock;
	}
	spin_unlock(&gic_class_lock);

	cpumask_copy(&affinity, cpu_online_mask);

	err = irq_do_set_affinity(d, &affinity, false);
	if (err)
		pr_warn_ratelimited(
			TAG "IRQ%u: affinity set failed(%d).\n",
			d->irq, err);

out_desc_lock:
	raw_spin_unlock(&desc->lock);
out_cpus_lock:
	local_irq_restore(flags);
	cpus_read_unlock();
}

static void gic_v3_set_affinity(void *unused, struct irq_data *d,
				  const struct cpumask *mask_val, u64 *affinity,
				  bool force, void __iomem *base,
				  void __iomem *rbase, u64 redist_stride)
{
	const struct cpumask *cpu_affinity = mask_val;
	bool is_class0 = false;
	u32 irq = d->hwirq - 32;
	bool need_class_update = false;
	void __iomem *reg = base + GICD_IROUTER + d->hwirq * 8;

	pr_debug(TAG "irq : %lu mask: %*pb\n",
		d->hwirq, cpumask_pr_args(cpu_affinity));

	if (!gic_routing_data.gic_1_of_N_init_done) {
		spin_lock(&gic_init_lock);
		if (!gic_routing_data.gic_1_of_N_init_done) {
			gic_enable_1_of_N(base);
			WRITE_ONCE(gic_routing_data.gic_chip, d->chip);
			/* Order readers of .gic_chip */
			smp_wmb();
			WRITE_ONCE(gic_routing_data.gic_1_of_N_init_done,
				   true);
		}
		spin_unlock(&gic_init_lock);
	}

	if (!gic_routing_data.gic_supports_1_of_N)
		return;

	/* Do not set InterruptRouting for single CPU affinity mask */
	if (cpumask_weight(cpu_affinity) <= 1 && (cpumask_weight(cpu_online_mask) > 1 ||
		!test_bit(irq, gic_saved_class0)))
		goto clear_class;

	spin_lock(&gic_class_lock);
	if (gic_routing_data.gic_enable_class0 && !test_bit(irq, gic_saved_class0))
		set_bit(irq, gic_saved_class0);

	if (!test_bit(irq, gic_saved_class0)) {
		clear_bit(irq, gic_saved_class0);
		clear_bit(irq, active_gic_class0);
		spin_unlock(&gic_class_lock);
		return;
	}

	is_class0 = true;
	*affinity |= GIC_INTERRUPT_ROUTING_MODE;

	need_class_update = gic_need_class_update(irq, is_class0);
	set_bit(irq, active_gic_class0);

	if (need_class_update) {
		writeq_relaxed(*affinity, reg);
		gic_do_class_update(base, irq, is_class0);
	}
	spin_unlock(&gic_class_lock);

	return;

clear_class:
	spin_lock(&gic_class_lock);
	clear_bit(irq, gic_saved_class0);
	clear_bit(irq, active_gic_class0);
	spin_unlock(&gic_class_lock);
}

static int gic_update_cpu_class(unsigned long class0_cpu_mask)
{
	struct arm_smccc_res res = { 0 };

	arm_smccc_smc(FID_BL31_GIC_SET_CLASS, class0_cpu_mask,
		0, 0, 0, 0, 0, 0, &res);

	if (res.a0 != 0) {
		pr_err(TAG "Failed to set GIC CPU class in ATF.\n");
		return -EINVAL;
	}

	return 0;
}

static int gic_affinity_cpu_online(unsigned int cpu)
{
	if (cpumask_intersects(&gic_routing_data.gic_routing_class0_cpus, cpu_online_mask))
		gic_update_cpu_class(cpumask_bits(
				&gic_routing_data.gic_routing_class0_cpus)[0]);
	else
		gic_update_cpu_class(cpumask_bits(
				&gic_routing_data.gic_routing_class1_cpus)[0]);

	if (cpumask_test_cpu(cpu,
		&gic_routing_data.gic_routing_class0_cpus)) {
		if (cpumask_intersects(&gic_routing_data.class0_active_cpus,
			&gic_routing_data.gic_routing_class1_cpus))
			cpumask_clear(&gic_routing_data.class0_active_cpus);
		cpumask_set_cpu(cpu,
			&gic_routing_data.class0_active_cpus);
	}
	return 0;
}

static int gic_affinity_cpu_offline(unsigned int cpu)
{
	if (!cpumask_test_cpu(cpu,
		&gic_routing_data.gic_routing_class0_cpus))
		return 0;

	cpumask_clear_cpu(cpu, &gic_routing_data.class0_active_cpus);
	if (cpumask_empty(&gic_routing_data.class0_active_cpus)) {
		gic_update_cpu_class(
			cpumask_bits(&gic_routing_data.gic_routing_class1_cpus)[0]);
		cpumask_and(&gic_routing_data.class0_active_cpus,
			&gic_routing_data.gic_routing_class1_cpus, cpu_online_mask);
	}

	return 0;
}

static ssize_t show_enable_class0(struct kobject *kobj,
				  struct kobj_attribute *attr, char *buf)
{
	unsigned int enable = 0;
	ssize_t len = 0;

	if (gic_routing_data.gic_enable_class0)
		enable = 1;

	len += sysfs_emit_at(buf, len, "%u\n", enable);

	return len;
}

static ssize_t store_enable_class0(struct kobject *kobj,
				  struct kobj_attribute *attr, const char *buf,
				  size_t count)
{
	unsigned int enable;
	struct irq_desc *desc;
	int i, ret = 0;

	ret = kstrtouint(buf, 10, &enable);
	if (ret != 0 || enable > 1) {
		pr_err(TAG "gic class0 spi input error, ret = %d\n", ret);
		return -EINVAL;
	}

	WRITE_ONCE(gic_routing_data.gic_enable_class0, !!enable);

	for_each_irq_desc(i, desc)
		gic_routing_set_affinity(i, enable);

	return count;
}

static ssize_t show_class0_spi(struct kobject *kobj,
				  struct kobj_attribute *attr, char *buf)
{
	unsigned int irq;
	ssize_t len = 0;
	unsigned long flags;

	if (bitmap_empty(active_gic_class0, MAX_IRQS)) {
		len += sysfs_emit_at(buf, len,
				"There are not GIC class0 SPIs\n");
		return len;
	}

	len += sysfs_emit_at(buf, len, "hwirq:\n");

	spin_lock_irqsave(&gic_class_lock, flags);
	for_each_set_bit(irq, active_gic_class0, MAX_IRQS) {
		len += sysfs_emit_at(buf, len, "%u\n", irq + 32);
		if (len >= PAGE_SIZE) {
			spin_unlock_irqrestore(&gic_class_lock, flags);
			pr_warn_once(TAG "active_gic_class0 exceeds PAGE_SIZE\n");
			return -EFBIG;
		}
	}
	spin_unlock_irqrestore(&gic_class_lock, flags);

	return len;
}

static ssize_t store_class0_spi(struct kobject *kobj,
				  struct kobj_attribute *attr, const char *buf,
				  size_t count)
{
	unsigned int irq;
	int ret = 0;

	ret = kstrtouint(buf, 10, &irq);
	if (ret != 0) {
		pr_err(TAG "gic class0 spi input error, ret = %d\n", ret);
		return -EINVAL;
	}

	gic_routing_set_affinity(irq, 1);

	return count;
}

static ssize_t show_cpu_class(struct kobject *kobj,
				  struct kobj_attribute *attr, char *buf)
{
	ssize_t len = 0;

	len += sysfs_emit_at(buf, len, "Class0 CPUs: %*pbl\n",
			cpumask_pr_args(&gic_routing_data.gic_routing_class0_cpus));

	len += sysfs_emit_at(buf, len, "Class1 CPUs: %*pbl\n",
			cpumask_pr_args(&gic_routing_data.gic_routing_class1_cpus));

	len += sysfs_emit_at(buf, len, "Active Class0 CPUs: %*pbl\n",
			cpumask_pr_args(&gic_routing_data.class0_active_cpus));

	if (len >= PAGE_SIZE) {
		pr_warn_once(TAG "gic_class_cpu exceeds PAGE_SIZE\n");
		return -EFBIG;
	}

	return len;
}

static ssize_t store_cpu_class(struct kobject *kobj,
				  struct kobj_attribute *attr, const char *buf,
				  size_t count)
{
	unsigned int mask_val, cpu;
	cpumask_t mask = { CPU_BITS_NONE };
	int ret = 0;
	unsigned long flags;

	ret = kstrtouint(buf, 16, &mask_val);
	if (ret != 0 || mask_val > cpumask_bits(cpu_possible_mask)[0]) {
		pr_err(TAG "cpumask input error, ret = %d\n", ret);
		return -EINVAL;
	}

	for_each_possible_cpu(cpu)
		if (mask_val & (1U << cpu))
			cpumask_set_cpu(cpu, &mask);

	spin_lock_irqsave(&gic_class_lock, flags);
	cpus_read_lock();
	if (cpumask_intersects(&mask, cpu_online_mask))
		gic_update_cpu_class(mask_val);
	else
		gic_update_cpu_class(~mask_val & cpumask_bits(cpu_possible_mask)[0]);

	cpumask_copy(&gic_routing_data.gic_routing_class0_cpus, &mask);
	cpumask_andnot(&gic_routing_data.gic_routing_class1_cpus, cpu_possible_mask,
			&gic_routing_data.gic_routing_class0_cpus);

	if (cpumask_intersects(&gic_routing_data.gic_routing_class0_cpus, cpu_online_mask))
		cpumask_and(&gic_routing_data.class0_active_cpus,
					&gic_routing_data.gic_routing_class0_cpus,
					cpu_online_mask);
	else
		cpumask_and(&gic_routing_data.class0_active_cpus,
					&gic_routing_data.gic_routing_class1_cpus,
					cpu_online_mask);
	cpus_read_unlock();
	spin_unlock_irqrestore(&gic_class_lock, flags);

	return count;
}

static ssize_t store_no_class_spi(struct kobject *kobj,
				  struct kobj_attribute *attr, const char *buf,
				  size_t count)
{
	unsigned int irq;
	int ret = 0;

	ret = kstrtouint(buf, 10, &irq);
	if (ret != 0) {
		pr_err(TAG "gic spi input error, ret = %d\n", ret);
		return -EINVAL;
	}

	gic_routing_set_affinity(irq, 0);

	return count;
}

#define SYS_RW_MODE	0644
#define SYS_WO_MODE	0200
static struct kobj_attribute gic_enable_class0 = __ATTR(gic_enable_class0,
				  SYS_RW_MODE,
				  show_enable_class0,
				  store_enable_class0);

static struct kobj_attribute gic_class0_spi = __ATTR(gic_class0_spi,
				  SYS_RW_MODE,
				  show_class0_spi,
				  store_class0_spi);

static struct kobj_attribute gic_no_class_spi = __ATTR(gic_no_class_spi,
				  SYS_WO_MODE,
				  NULL,
				  store_no_class_spi);

static struct kobj_attribute gic_cpu_class = __ATTR(gic_cpu_class,
				  SYS_RW_MODE,
				  show_cpu_class,
				  store_cpu_class);

static struct attribute *gic_routing_attrs[] = {
	&gic_enable_class0.attr,
	&gic_class0_spi.attr,
	&gic_no_class_spi.attr,
	&gic_cpu_class.attr,
	NULL
};

static const struct attribute_group gic_routing_attr_group = {
	.attrs = gic_routing_attrs,
};

int gic_routing_sysfs_init(void)
{
	int err = 0;
	struct kobject *gic_kobj;
	struct device *dev_root = bus_get_dev_root(&cpu_subsys);

	if (!dev_root) {
		pr_err(TAG "Get CPU Subsys dev_root failed\n");
		return -ENODEV;
	}
	gic_kobj = kobject_create_and_add("gic_routing", &dev_root->kobj);
	put_device(dev_root);
	if (IS_ERR_OR_NULL(gic_kobj)) {
		pr_err(TAG "gic_routing sysfs node create error, errorcode = %ld\n",
			(long)gic_kobj);
		return (long)gic_kobj;
	}

	err = sysfs_create_group(gic_kobj, &gic_routing_attr_group);
	if (err) {
		pr_err(TAG "gic routing sysfs create group error, errorcode = %d\n",
			err);
		kobject_put(gic_kobj);
	}
	return err;
}

static int gic_intr_routing_probe(struct platform_device *pdev)
{
	u32 class0_cpu_mask;
	int cpu, ret = 0;
	struct arm_smccc_res res = {0};

	ret = of_property_read_u32(pdev->dev.of_node,
				   "xring,gic-class0-cpus",
				   &class0_cpu_mask);
	if (ret) {
		dev_err(&pdev->dev, "failed to get class0_cpu_mask\n");
		return ret;
	}

	for_each_possible_cpu(cpu)
		if (class0_cpu_mask & (1U << cpu))
			cpumask_set_cpu(cpu, &gic_routing_data.gic_routing_class0_cpus);

	cpumask_andnot(&gic_routing_data.gic_routing_class1_cpus, cpu_possible_mask,
			&gic_routing_data.gic_routing_class0_cpus);

	arm_smccc_smc(FID_BL31_GIC_E1NWF, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		dev_err(&pdev->dev, "Failed to enable GIC 1 of N wakeup functionality in ATF\n");
		return -EINVAL;
	}

	ret = gic_update_cpu_class(
			cpumask_bits(&gic_routing_data.gic_routing_class0_cpus)[0]);
	if (ret < 0)
		return ret;

	WRITE_ONCE(gic_routing_data.gic_enable_class0, false);

	ret = register_trace_android_rvh_gic_v3_set_affinity(
		gic_v3_set_affinity, NULL);

	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register gic_v3_set_affinity hook\n");
		return ret;
	}

	cpus_read_lock();
	cpumask_and(&gic_routing_data.class0_active_cpus,
		&gic_routing_data.gic_routing_class0_cpus,
		cpu_online_mask);

	ret = cpuhp_setup_state_nocalls_cpuslocked(
		CPUHP_AP_ONLINE_DYN, "gic_affinity_setting:online",
		gic_affinity_cpu_online, gic_affinity_cpu_offline);
	cpus_read_unlock();

	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register CPUHP state\n");
		return ret;
	}
	gic_routing_data.gic_affinity_cpuhp_state = ret;

	ret = gic_routing_sysfs_init();
	if (ret < 0) {
		cpuhp_remove_state_nocalls(CPUHP_AP_ONLINE_DYN);
		dev_err(&pdev->dev, "Failed to create sys node\n");
		return ret;
	}

	dev_info(&pdev->dev, "GIC Interrupt Routing Driver Registered\n");

	return 0;
}

static const struct of_device_id gic_intr_routing_of_match[] = {
	{ .compatible = "xring,gic-intr-routing"},
	{}
};
MODULE_DEVICE_TABLE(of, gic_intr_routing_of_match);

static struct platform_driver gic_intr_routing_driver = {
	.probe = gic_intr_routing_probe,
	.driver = {
		.name = "gic_routing",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(gic_intr_routing_of_match),
	},
};

static int __init gic_routing_init(void)
{
	return platform_driver_register(&gic_intr_routing_driver);
}
module_init(gic_routing_init);

MODULE_AUTHOR("Ding Li <liding7@xiaomi.com>");
MODULE_DESCRIPTION("XRing GIC Interrupt Routing Driver");
MODULE_LICENSE("GPL v2");
