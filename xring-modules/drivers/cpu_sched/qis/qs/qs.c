// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#define pr_fmt(fmt) "qs: " fmt

#include <linux/compiler.h>
#include <linux/kobject.h>
#include <linux/module.h>

static ssize_t sched_enable_store(struct kobject *kobj,
				  struct kobj_attribute *attr,
				  const char *buf, size_t count)
{
	return count;
}

static ssize_t sched_enable_show(struct kobject *kobj,
				 struct kobj_attribute *attr,
				 char *buf)
{
	return 0;
}

#define SYS_MODE	0640
static struct kobj_attribute sched_enable_attr = __ATTR(sched_enable,
							SYS_MODE,
							sched_enable_show,
							sched_enable_store);

static struct attribute *qs_attrs[] = {
	&sched_enable_attr.attr,
	NULL,
};

static struct attribute_group qs_attr_group = {
	.attrs = qs_attrs,
};

int sl_init(void);

static int __init qs_init(void)
{
	int ret = 0;
	struct kobject *qs_kobj = NULL;

	qs_kobj = kobject_create_and_add("qos_sched", kernel_kobj);
	if (qs_kobj == NULL)
		return -ENOMEM;

	ret = sysfs_create_group(qs_kobj, &qs_attr_group);
	if (ret) {
		kobject_put(qs_kobj);
		pr_err("qs init: failed to create sys node\n");
		return -EINVAL;
	}

	ret = sl_init();
	if (ret < 0) {
		pr_err("speed lock probe failed\n");
		goto err_exit;
	}

	pr_info("qos sched and speed lock probe successfully\n");
	return ret;

err_exit:
	sysfs_remove_group(qs_kobj, &qs_attr_group);
	kobject_put(qs_kobj);
	return ret;
}

module_init(qs_init);

MODULE_AUTHOR("Shaohua Fan <fanshaohua@xiaomi.com>");
MODULE_DESCRIPTION("XRing QOS sched Driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xr-qi sched-walt");
