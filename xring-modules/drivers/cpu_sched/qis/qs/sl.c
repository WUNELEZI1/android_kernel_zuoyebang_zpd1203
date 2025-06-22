// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#define pr_fmt(fmt) "sl: " fmt

#include <linux/compiler.h>
#include <linux/kobject.h>
#include <linux/module.h>

static ssize_t sl_enable_store(struct kobject *kobj,
			       struct kobj_attribute *attr,
			       const char *buf, size_t count)
{
	return count;
}

static ssize_t sl_enable_show(struct kobject *kobj,
			      struct kobj_attribute *attr,
			      char *buf)
{
	return 0;
}

#define SYS_MODE	0640
static struct kobj_attribute sl_enable_attr = __ATTR(sl_enable,
						     SYS_MODE,
						     sl_enable_show,
						     sl_enable_store);

static struct attribute *sl_attrs[] = {
	&sl_enable_attr.attr,
	NULL,
};

static struct attribute_group sl_attr_group = {
	.attrs = sl_attrs,
};

int sl_init(void)
{
	int ret = 0;
	struct kobject *sl_kobj = NULL;

	sl_kobj = kobject_create_and_add("speedlock", kernel_kobj);
	if (sl_kobj == NULL)
		return -ENOMEM;

	ret = sysfs_create_group(sl_kobj, &sl_attr_group);
	if (ret) {
		kobject_put(sl_kobj);
		pr_err("speedlock init: failed to create sys node\n");
	}

	return ret;
}
