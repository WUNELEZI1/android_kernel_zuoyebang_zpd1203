// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#define pr_fmt(fmt) "qi: " fmt

#include <linux/compiler.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <soc/xring/perf_actuator.h>

struct task_conf_t {
	u32 qos;
	pid_t pid;
};

int register_qievent_notifier(struct notifier_block *nb)
{
	return 0;
}
EXPORT_SYMBOL_GPL(register_qievent_notifier);

int unregister_qievent_notifier(struct notifier_block *nb)
{
	return 0;
}
EXPORT_SYMBOL_GPL(unregister_qievent_notifier);

u32 get_qos_lvl(struct task_struct *task)
{
	return 0;
}
EXPORT_SYMBOL_GPL(get_qos_lvl);

bool is_qos_task(struct task_struct *task)
{
	return false;
}
EXPORT_SYMBOL_GPL(is_qos_task);

int get_proc_static_qos_lvl(struct task_struct *task)
{
	return 0;
}
EXPORT_SYMBOL_GPL(get_proc_static_qos_lvl);

int qi_intf_set_qos(void __user *uarg)
{
	return 0;
}

int qi_intf_get_qos_by_tid(void __user *uarg)
{
	return 0;
}

int qi_intf_get_qos_by_pid(void __user *uarg)
{
	return 0;
}

static ssize_t inherit_enable_store(struct kobject *kobj,
				    struct kobj_attribute *attr,
				    const char *buf, size_t count)
{
	return count;
}

static ssize_t inherit_enable_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	return 0;
}

#define SYS_MODE	0640
static struct kobj_attribute inherit_enable_attr = __ATTR(inherit_enable,
							  SYS_MODE,
							  inherit_enable_show,
							  inherit_enable_store);

static struct attribute *qi_attrs[] = {
	&inherit_enable_attr.attr,
	NULL,
};

static struct attribute_group qi_attr_group = {
	.attrs = qi_attrs,
};

#define CMD_SET_PID_QOS PERF_W_CMD(SET_PID_QOS, struct task_conf_t)
#define CMD_GET_PID_QOS PERF_RW_CMD(GET_PID_QOS, struct task_conf_t)
#define CMD_GET_TID_QOS PERF_RW_CMD(GET_TID_QOS, struct task_conf_t)

static int __init qi_init(void)
{
	int ret = 0;
	struct kobject *qi_kobj = NULL;

	qi_kobj = kobject_create_and_add("qos_inherit", kernel_kobj);
	if (!qi_kobj)
		return -ENOMEM;

	ret = sysfs_create_group(qi_kobj, &qi_attr_group);
	if (ret) {
		pr_err("qi init: failed to create sys node\n");
		kobject_put(qi_kobj);
		return ret;
	}

	register_perf_actuator(CMD_SET_PID_QOS, qi_intf_set_qos);
	register_perf_actuator(CMD_GET_PID_QOS, qi_intf_get_qos_by_pid);
	register_perf_actuator(CMD_GET_TID_QOS, qi_intf_get_qos_by_tid);

	return ret;
}

module_init(qi_init);

MODULE_AUTHOR("Shaohua Fan <fanshaohua@xiaomi.com>");
MODULE_DESCRIPTION("XRing QOS inherit Driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: perf-actuator");
