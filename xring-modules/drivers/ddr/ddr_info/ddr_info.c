// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/units.h>
#include <linux/sysfs.h>

#define DDR_INFO_MAX_LEN	100
#undef pr_fmt
#define pr_fmt(fmt)		"ddr_info: " fmt

struct ddr_info {
	unsigned char rank_num;
	unsigned char vendor;
	unsigned char capacity;
	unsigned char type;
};

static struct ddr_info g_ddr_info;

static ssize_t ddr_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i;
	int cnt = 0;

	cnt += snprintf(buf + cnt, DDR_INFO_MAX_LEN, "Rank Num: %d\n", g_ddr_info.rank_num);
	for (i = 0; i < g_ddr_info.rank_num; i++)
		cnt += snprintf(buf + cnt, DDR_INFO_MAX_LEN, "Rank%d: %s\n",
			i, (g_ddr_info.type & (0x1 << i)) != 0 ? "x8" : "x16");

	cnt += snprintf(buf + cnt, DDR_INFO_MAX_LEN, "Manufacturer ID: 0x%x\n", g_ddr_info.vendor);
	cnt += snprintf(buf + cnt, DDR_INFO_MAX_LEN, "Capacity: %dGB\n", g_ddr_info.capacity);

	return cnt;
}

static DEVICE_ATTR_RO(ddr_info);

static struct attribute *g_ddr_info_attrs[] = {
	&dev_attr_ddr_info.attr,
	NULL,
};

static struct attribute_group g_ddr_info_attr_group = {
	.attrs = g_ddr_info_attrs,
};

static int ddr_info_proccess(struct device_node *ddr_info_node)
{
	int ret;
	void __iomem *ddr_info_addr = NULL;
	void __iomem *ddr_rank_num_addr = NULL;
	unsigned int ddr_info_reg, ddr_rank_num_reg;
	unsigned int ddr_info, ddr_rank_num;
	unsigned int vendor_shift, vendor_mask;
	unsigned int capacity_shift, capacity_mask;
	unsigned int type_shift, type_mask;
	unsigned int rank_num_shift, rank_num_mask;

	ret = of_property_read_u32_array(ddr_info_node, "ddr_info_reg", &ddr_info_reg, 1);
	if (ret) {
		pr_err("Failed to get ddr_info_reg!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_info_node, "ddr_rank_num_reg", &ddr_rank_num_reg, 1);
	if (ret) {
		pr_err("Failed to get ddr_rank_num_reg!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_info_node, "vendor_shift", &vendor_shift, 1);
	if (ret) {
		pr_err("Failed to get vendor_shift!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_info_node, "vendor_mask", &vendor_mask, 1);
	if (ret) {
		pr_err("Failed to get vendor_mask!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_info_node, "capacity_shift", &capacity_shift, 1);
	if (ret) {
		pr_err("Failed to get capacity_shift!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_info_node, "capacity_mask", &capacity_mask, 1);
	if (ret) {
		pr_err("Failed to get capacity_mask!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_info_node, "type_shift", &type_shift, 1);
	if (ret) {
		pr_err("Failed to get type_shift!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_info_node, "type_mask", &type_mask, 1);
	if (ret) {
		pr_err("Failed to get type_mask!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_info_node, "rank_num_shift", &rank_num_shift, 1);
	if (ret) {
		pr_err("Failed to get rank_num_shift!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_info_node, "rank_num_mask", &rank_num_mask, 1);
	if (ret) {
		pr_err("Failed to get rank_num_mask!\n");
		return -EINVAL;
	}

	ddr_info_addr = ioremap(ddr_info_reg, sizeof(unsigned int));
	if (!ddr_info_addr) {
		pr_err("Failed to ioremap ddr_info_addr!\n");
		return -EINVAL;
	}

	ddr_rank_num_addr = ioremap(ddr_rank_num_reg, sizeof(unsigned int));
	if (!ddr_rank_num_addr) {
		pr_err("Failed to ioremap ddr_rank_num_addr!\n");
		return -EINVAL;
	}

	ddr_info = readl(ddr_info_addr);
	iounmap(ddr_info_addr);
	pr_info("ddr_info = 0x%x\n", ddr_info);

	ddr_rank_num = readl(ddr_rank_num_addr);
	iounmap(ddr_rank_num_addr);

	/* vendor: bit[0:7], capacity: bit[8:13], type: bit[14:15] */
	g_ddr_info.vendor = (ddr_info & vendor_mask) >> vendor_shift;
	g_ddr_info.capacity = (ddr_info & capacity_mask) >> capacity_shift;
	g_ddr_info.type = (ddr_info & type_mask) >> type_shift;
	g_ddr_info.rank_num = ((ddr_rank_num & rank_num_mask) >> rank_num_shift) ? 2 : 1;

	return 0;
}

static int ddr_info_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;

	ret = ddr_info_proccess(dev->of_node);
	if (ret) {
		pr_err("failed to ddr_info_proccess!\n");
		return -EINVAL;
	}

	ret = sysfs_create_group(&dev->kobj, &g_ddr_info_attr_group);
	if (ret) {
		pr_err("failed to create sysfs group!\n");
		return -EINVAL;
	}

	pr_info("ddr info probe success!\n");
	return 0;
}

static int ddr_info_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	sysfs_remove_group(&dev->kobj, &g_ddr_info_attr_group);
	pr_info("ddr info remove success!\n");

	return 0;
}

static const struct of_device_id g_ddr_info_table[] = {
	{.compatible = "xring,ddr_info"},
	{},
};

MODULE_DEVICE_TABLE(of, g_ddr_info_table);

static struct platform_driver g_ddr_info_driver = {
	.probe = ddr_info_probe,
	.remove = ddr_info_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "ddr_info",
		.of_match_table = g_ddr_info_table,
	},
};

int ddr_info_init(void)
{
	return platform_driver_register(&g_ddr_info_driver);
}

void ddr_info_exit(void)
{
	platform_driver_unregister(&g_ddr_info_driver);
}
