// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/devfreq.h>
#include <linux/pm_opp.h>
#include <linux/pm_qos.h>
#include <linux/units.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/devfreq.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <governor.h>

#define MHZ	1000000U

#ifdef NPU_QOS_DEBUG
#define npu_qos_debug(fmt, ...)         \
	pr_err("[%s][D]: " fmt, __func__, ##__VA_ARGS__)
#else
#define npu_qos_debug(fmt, ...)
#endif

struct devfreq_npu_pm_qos_notifier_block {
	struct notifier_block nb;
	struct devfreq *df;
};

static struct freq_constraints g_npu_freq_cons;

static void npu_freq_qos_add_request(struct freq_qos_request *req, s32 value,
				     enum freq_qos_req_type type)
{
	if (!req)
		return;

	if (freq_qos_request_active(req))
		return;

	freq_qos_add_request(&g_npu_freq_cons, req, type, value);
}

void npu_freq_qos_add_min_request(struct freq_qos_request *req, s32 value)
{
	npu_freq_qos_add_request(req, value, FREQ_QOS_MIN);
}

void npu_freq_qos_add_max_request(struct freq_qos_request *req, s32 value)
{
	npu_freq_qos_add_request(req, value, FREQ_QOS_MAX);
}

int npu_freq_qos_update_request(struct freq_qos_request *req, s32 new_value)
{
	if (!req)
		return -EINVAL;

	npu_qos_debug(": %d\n", new_value);
	if (!freq_qos_request_active(req))
		return -EINVAL;

	return freq_qos_update_request(req, new_value);
}

void npu_freq_qos_remove_request(struct freq_qos_request *req)
{
	if (!req)
		return;

	if (!freq_qos_request_active(req))
		return;

	freq_qos_remove_request(req);
}

static int npu_freq_qos_min_open(struct inode *inode, struct file *filp)
{
	struct freq_qos_request *req;

	req = kzalloc(sizeof(*req), GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	npu_freq_qos_add_min_request(req, FREQ_QOS_MIN_DEFAULT_VALUE);
	filp->private_data = req;

	return 0;
}

static int npu_freq_qos_max_open(struct inode *inode, struct file *filp)
{
	struct freq_qos_request *req;

	req = kzalloc(sizeof(*req), GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	npu_freq_qos_add_max_request(req, FREQ_QOS_MAX_DEFAULT_VALUE);
	filp->private_data = req;

	return 0;
}

static int npu_freq_qos_release(struct inode *inode, struct file *filp)
{
	struct freq_qos_request *req = filp->private_data;

	filp->private_data = NULL;

	npu_freq_qos_remove_request(req);
	kfree(req);

	return 0;
}

static s32 npu_pm_qos_read_value(struct pm_qos_constraints *c)
{
	return READ_ONCE(c->target_value);
}

static s32 npu_freq_qos_read_value(struct freq_constraints *qos,
			enum freq_qos_req_type type)
{
	int ret;

	switch (type) {
	case FREQ_QOS_MIN:
		ret = IS_ERR_OR_NULL(qos) ?
			FREQ_QOS_MIN_DEFAULT_VALUE :
			npu_pm_qos_read_value(&qos->min_freq);
		break;
	case FREQ_QOS_MAX:
		ret = IS_ERR_OR_NULL(qos) ?
			FREQ_QOS_MAX_DEFAULT_VALUE :
			npu_pm_qos_read_value(&qos->max_freq);
		break;
	default:
		WARN_ON(1);
		ret = 0;
	}

	return ret;
}

static ssize_t npu_freq_qos_min_read(struct file *filp, char __user *buf,
				 size_t count, loff_t *f_pos)
{
	struct freq_qos_request *req = filp->private_data;
	int value;

	if (!req || !freq_qos_request_active(req)) {
		pr_err("%s invalid request\n", __func__);
		return -EINVAL;
	}

	value = npu_freq_qos_read_value(&g_npu_freq_cons, FREQ_QOS_MIN);

	return simple_read_from_buffer(buf, count, f_pos, &value, sizeof(s32));
}

static ssize_t npu_freq_qos_max_read(struct file *filp, char __user *buf,
				 size_t count, loff_t *f_pos)
{
	struct freq_qos_request *req = filp->private_data;
	int value;

	if (!req || !freq_qos_request_active(req))
		return -EINVAL;

	value = npu_freq_qos_read_value(&g_npu_freq_cons, FREQ_QOS_MAX);

	return simple_read_from_buffer(buf, count, f_pos, &value, sizeof(s32));
}

static ssize_t npu_freq_qos_write(struct file *filp, const char __user *buf,
				  size_t count, loff_t *f_pos)
{
	int ret;
	int value;

	if (count == sizeof(int)) {
		if (copy_from_user(&value, buf, sizeof(s32)))
			return -EFAULT;
	} else {
		ret = kstrtos32_from_user(buf, count, 16, &value);
		if (ret < 0)
			return ret;
	}

	ret = npu_freq_qos_update_request(filp->private_data, value);
	if (ret < 0)
		return ret;

	return count;
}

static const struct file_operations npu_freq_qos_min_fops = {
	.write = npu_freq_qos_write,
	.read = npu_freq_qos_min_read,
	.open = npu_freq_qos_min_open,
	.release = npu_freq_qos_release,
	.llseek = noop_llseek,
};

static struct miscdevice npu_freq_qos_min_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "npu_freq_qos_min",
	.fops = &npu_freq_qos_min_fops,
};

static const struct file_operations npu_freq_qos_max_fops = {
	.write = npu_freq_qos_write,
	.read = npu_freq_qos_max_read,
	.open = npu_freq_qos_max_open,
	.release = npu_freq_qos_release,
	.llseek = noop_llseek,
};

static struct miscdevice npu_freq_qos_max_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "npu_freq_qos_max",
	.fops = &npu_freq_qos_max_fops,
};

static int devfreq_npu_qos_func(struct devfreq *df, unsigned long *freq)
{
	int min_freq;
	int max_freq;

	min_freq = npu_freq_qos_read_value(&g_npu_freq_cons, FREQ_QOS_MIN);
	max_freq = npu_freq_qos_read_value(&g_npu_freq_cons, FREQ_QOS_MAX);

	npu_qos_debug("npu qos min_freq:%d max_freq:%d\n", min_freq, max_freq);

	if (min_freq > max_freq)
		*freq = max_freq;
	else
		*freq = min_freq;

	return 0;
}

static int devfreq_npu_qos_notifier_call(struct notifier_block *nb,
					 unsigned long type,
					 void *data)
{
	int ret;
	struct devfreq_npu_pm_qos_notifier_block *pq_nb =
		container_of(nb, struct devfreq_npu_pm_qos_notifier_block, nb);
	struct devfreq *df = NULL;

	df = pq_nb->df;
	if (IS_ERR_OR_NULL(df))
		return -EINVAL;

	mutex_lock(&df->lock);
	ret = update_devfreq(df);
	mutex_unlock(&df->lock);

	return ret;
}

static struct devfreq_npu_pm_qos_notifier_block g_npu_qos_dn_notifier = {
	.nb = {.notifier_call = devfreq_npu_qos_notifier_call},
	.df = NULL,
};

static void npu_freq_constraints_init(struct freq_constraints *qos)
{
	struct pm_qos_constraints *c;

	c = &qos->min_freq;
	plist_head_init(&c->list);
	c->type = PM_QOS_MAX;
	c->no_constraint_value = FREQ_QOS_MIN_DEFAULT_VALUE;
	c->default_value = FREQ_QOS_MIN_DEFAULT_VALUE;
	c->target_value = FREQ_QOS_MIN_DEFAULT_VALUE;
	c->notifiers = &qos->min_freq_notifiers;
	BLOCKING_INIT_NOTIFIER_HEAD(c->notifiers);

	c = &qos->max_freq;
	plist_head_init(&c->list);
	c->type = PM_QOS_MIN;
	c->no_constraint_value = FREQ_QOS_MAX_DEFAULT_VALUE;
	c->default_value = FREQ_QOS_MAX_DEFAULT_VALUE;
	c->target_value = FREQ_QOS_MAX_DEFAULT_VALUE;
	c->notifiers = &qos->max_freq_notifiers;
	BLOCKING_INIT_NOTIFIER_HEAD(c->notifiers);
}

static int npu_freq_qos_init(void)
{
	int ret;

	ret = misc_register(&npu_freq_qos_min_miscdev);
	if (ret < 0)
		pr_err("%s: %s setup failed\n", __func__,
		       npu_freq_qos_min_miscdev.name);
	ret = misc_register(&npu_freq_qos_max_miscdev);
	if (ret < 0)
		pr_err("%s: %s setup failed\n", __func__,
		       npu_freq_qos_max_miscdev.name);

	return ret;
}

static int devfreq_npu_qos_gov_init(struct devfreq *df)
{
	int ret;

	if (!df || g_npu_qos_dn_notifier.df) {
		dev_err(&df->dev, "Restart npu qos governor\n");
		return -EPERM;
	}

	g_npu_qos_dn_notifier.df = df;
	npu_freq_constraints_init(&g_npu_freq_cons);
	freq_qos_add_notifier(&g_npu_freq_cons, FREQ_QOS_MIN, &g_npu_qos_dn_notifier.nb);
	freq_qos_add_notifier(&g_npu_freq_cons, FREQ_QOS_MAX, &g_npu_qos_dn_notifier.nb);

	ret = npu_freq_qos_init();
	if (ret != 0)
		pr_err("%s: npu freq qps init fail %d\n", __func__, ret);

	return 0;
}

static void npu_freq_qos_exit(void)
{
	misc_deregister(&npu_freq_qos_min_miscdev);
	misc_deregister(&npu_freq_qos_max_miscdev);
}

static int devfreq_npu_qos_gov_stop(struct devfreq *df)
{
	if (!g_npu_qos_dn_notifier.df) {
		pr_err("%s: Restop npu governor\n", __func__);
		return -EPERM;
	}

	g_npu_qos_dn_notifier.df = NULL;
	npu_freq_qos_exit();

	return 0;
}

static int devfreq_npu_qos_handler(struct devfreq *devfreq,
				   unsigned int event, void *data)
{
	int ret = 0;

	switch (event) {
	case DEVFREQ_GOV_START:
		npu_qos_debug("npu qos gov start\n");
		ret = devfreq_npu_qos_gov_init(devfreq);
		break;
	case DEVFREQ_GOV_STOP:
		npu_qos_debug("npu qos gov stop\n");
		devfreq_npu_qos_gov_stop(devfreq);
		break;
	default:
		break;
	}

	return ret;
}

struct devfreq_governor devfreq_npu_qos = {
	.name = "npu_freq_qos",
	.get_target_freq = devfreq_npu_qos_func,
	.event_handler = devfreq_npu_qos_handler,
};

int __init devfreq_npu_qos_init(void)
{
	int ret;

	ret = devfreq_add_governor(&devfreq_npu_qos);
	if (ret != 0)
		pr_err("%s: npu freq qps add governor fail %d\n", __func__, ret);

	return ret;
}
subsys_initcall(devfreq_npu_qos_init);

static void __exit devfreq_npu_qos_exit(void)
{
	int ret;

	ret = devfreq_remove_governor(&devfreq_npu_qos);
	if (ret)
		pr_err("%s: failed remove governor %d\n", __func__, ret);
}
module_exit(devfreq_npu_qos_exit);

MODULE_SOFTDEP("post: npu_freq_drv");
MODULE_AUTHOR("Hao Wang <wanghao69@xiaomi.com>");
MODULE_DESCRIPTION("XRing NPU Freq QOS");
MODULE_LICENSE("GPL v2");
