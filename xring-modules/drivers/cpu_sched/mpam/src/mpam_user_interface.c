// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "mpam_user_interface.h"
#include "mpam_partid_map_mgr.h"
#include <linux/cpu.h>

#define TAG "xring-mpam-interface: "

#define MODE_RO 0440
#define MODE_RW 0640

static struct kobject *mpam_kobj;

static ssize_t mpam_partid_map_cpu_store(struct kobject *kobj,
					 struct kobj_attribute *attr, const char *buf,
					 size_t count)
{
	int cpu_id = 0;
	u16 partid = 0;
	int ret = 0;

	ret = sscanf_s(buf, "%d %u", &cpu_id, &partid);
	if (ret != 2) {
		pr_err(TAG "cpu partid input error, ret = %d\n", ret);
		return -EINVAL;
	}

	mpam_set_cpu_partid(cpu_id, partid);

	return count;
}

static ssize_t mpam_partid_map_cgroup_store(struct kobject *kobj,
					    struct kobj_attribute *attr,
					    const char *buf, size_t count)
{
	u16 partid = 0;
	char str[MPAM_STRING_MAX] = { 0 };
	int ret = 0;
	int cgroup_id = 0;

	ret = sscanf_s(buf, "%s %u", str, ARRAY_SIZE(str), &partid);
	if (ret != 2) {
		pr_err(TAG "cgroup partid input error, ret = %d\n", ret);
		return -EINVAL;
	}

	ret = mpam_get_cgroup_subsys_id(str, &cgroup_id);
	if (ret) {
		pr_err(TAG "get cgroup subsys id error, errcode = %d\n", ret);
		return ret;
	}

	if (cgroup_id > 0) {
		mpam_set_cgroup_partid(cgroup_id, partid);
	} else {
		pr_err(TAG "path = %s, cgroup id = %d is not valid\n", str,
		       cgroup_id);
		return -EINVAL;
	}

	return count;
}

static ssize_t mpam_partid_map_task_store(struct kobject *kobj,
					  struct kobj_attribute *attr, const char *buf,
					  size_t count)
{
	int pid = 0;
	u16 partid = 0;
	int ret = 0;

	ret = sscanf_s(buf, "%d %u", &pid, &partid);
	if (ret != 2) {
		pr_err(TAG "task partid input error, ret = %d\n", ret);
		return -EINVAL;
	}

	ret = mpam_set_task_partid(pid, partid);
	if (ret) {
		pr_err(TAG "set task partid error, errorcode=%d\n", ret);
		return ret;
	}
	return count;
}

static ssize_t mpam_enable_store(struct kobject *kobj, struct kobj_attribute *attr,
				 const char *buf, size_t count)
{
	int err = 0;
	int enable = 0;

	err = kstrtoint(buf, 0, &enable);
	if (err) {
		pr_err(TAG "set enable error, input=%s, errorcode=%d\n", buf,
		       err);
		return err;
	}

	err = mpam_set_cpu_enable(enable);
	if (err)
		return err;
	else
		return count;
}

static ssize_t mpam_enable_show(struct kobject *kobj, struct kobj_attribute *attr,
			 char *buf)
{
	int ret = 0;

	ret = mpam_get_cpu_enable_status();
	return sprintf_s(buf, PAGE_SIZE, "%d\n", ret);
}

static ssize_t mpam_partid_max_show(struct kobject *kobj, struct kobj_attribute *attr,
			     char *buf)
{
	return sprintf_s(buf, PAGE_SIZE - 1, "%u\n", mpam_get_max_partid());
}

static ssize_t mpam_partid_select_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	u16 partid = 0;
	int ret;
	struct mpam_msc *msc = NULL;

	msc = (struct mpam_msc *)dev_get_drvdata(dev);

	ret = mpam_get_partid_sel(msc, &partid);
	if (ret)
		return ret;

	return sprintf_s(buf, PAGE_SIZE - 1, "%u\n", partid);
}

static ssize_t mpam_partid_select_store(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t count)
{
	u16 reg = 0;
	int err = 0;
	struct mpam_msc *msc = NULL;

	msc = (struct mpam_msc *)dev_get_drvdata(dev);

	err = kstrtou16(buf, 0, &reg);
	if (err) {
		dev_err(dev, "partid Input err, errcode = %d\n", err);
		return err;
	}

	err = mpam_set_partid_sel(msc, reg);
	if (err)
		return err;

	return count;
}

static ssize_t mpam_cpbm_show(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	struct mpam_msc *msc = NULL;
	u32 reg = 0;
	int ret = 0;

	msc = (struct mpam_msc *)dev_get_drvdata(dev);
	ret = mpam_get_cpor_part_config(msc, &reg);
	if (ret)
		return ret;

	return sprintf_s(buf, PAGE_SIZE - 1, "0x%x\n", reg);
}

static ssize_t mpam_cpbm_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret = 0;
	u32 reg = 0;
	struct mpam_msc *msc = NULL;

	ret = kstrtouint(buf, 0, &reg);
	if (ret) {
		dev_err(dev, "cpbm input error %s, errorcode=%d\n", buf, ret);
		return ret;
	}

	msc = (struct mpam_msc *)dev_get_drvdata(dev);
	ret = mpam_set_cpor_part_config(msc, reg);
	if (ret)
		return ret;

	return count;
}

static ssize_t mpam_mbw_prop_show(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	int ret = 0;
	u32 reg = 0;
	struct mpam_msc *msc = NULL;

	msc = (struct mpam_msc *)dev_get_drvdata(dev);

	ret = mpam_get_mbw_part_config(msc, &reg);
	if (ret)
		return ret;

	return sprintf_s(buf, PAGE_SIZE - 1, "0x%x\n", reg);
}

static ssize_t mpam_mbw_prop_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int ret = 0;
	u32 reg = 0;
	struct mpam_msc *msc = NULL;

	msc = (struct mpam_msc *)dev_get_drvdata(dev);
	ret = kstrtouint(buf, 0, &reg);
	if (ret) {
		dev_err(dev, "set mbw prop input %s error, errorcode=%d\n", buf,
			ret);
		return ret;
	}

	ret = mpam_set_mbw_part_config(msc, reg);
	if (ret)
		return ret;

	return count;
}

static ssize_t mpam_msc_lock_show(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct mpam_msc *msc = NULL;
	int lock_status = 0;

	msc = (struct mpam_msc *)dev_get_drvdata(dev);
	lock_status = mpam_get_msc_lock(msc);

	if (lock_status < 0)
		dev_err(dev, "get lock status error, errorcode = %d\n",
			lock_status);

	return sprintf_s(buf, PAGE_SIZE - 1, "%d\n", lock_status);
}

static ssize_t mpam_msc_lock_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct mpam_msc *msc = NULL;
	int err = 0;

	msc = (struct mpam_msc *)dev_get_drvdata(dev);
	err = mpam_set_msc_lock(msc);
	if (err)
		dev_err(dev, "set lock Error, errorcode = %d\n", err);
	return count;
}


static DEVICE_ATTR(mpam_cpbm, MODE_RW, mpam_cpbm_show, mpam_cpbm_store);
static DEVICE_ATTR(mpam_mbw_prop, MODE_RW, mpam_mbw_prop_show,
		   mpam_mbw_prop_store);
static DEVICE_ATTR(mpam_partid_select, MODE_RW, mpam_partid_select_show,
		   mpam_partid_select_store);
static DEVICE_ATTR(mpam_msc_lock, MODE_RW, mpam_msc_lock_show,
		   mpam_msc_lock_store);

static struct attribute *mpam_msc_attrs[] = {
	&dev_attr_mpam_partid_select.attr, &dev_attr_mpam_msc_lock.attr,
	NULL
};
static const struct attribute_group mpam_msc_group = {
	.attrs = mpam_msc_attrs,
};

static struct kobj_attribute mpam_partid_max_attr =
	__ATTR_RO_MODE(mpam_partid_max, MODE_RO);
static struct kobj_attribute mpam_partid_map_cpu_attr =
	__ATTR_WO(mpam_partid_map_cpu);
static struct kobj_attribute mpam_partid_map_cgroup_attr =
	__ATTR_WO(mpam_partid_map_cgroup);
static struct kobj_attribute mpam_partid_map_task_attr =
	__ATTR_WO(mpam_partid_map_task);
static struct kobj_attribute mpam_enable = __ATTR_RW_MODE(mpam_enable, MODE_RW);

static struct attribute *mpam_partid_map_attrs[] = {
	&mpam_partid_max_attr.attr,
	&mpam_partid_map_cpu_attr.attr,
	&mpam_partid_map_cgroup_attr.attr,
	&mpam_partid_map_task_attr.attr,
	&mpam_enable.attr,
	NULL
};

const struct attribute_group mpam_partid_map_group = {
	.attrs = mpam_partid_map_attrs,
};

int mpam_msc_sysfs_interface_init(struct mpam_msc *msc)
{
	int ret = 0;
	struct device *dev = &msc->pdev->dev;

	lockdep_assert_held(&msc->lock);

	ret = device_add_group(dev, &mpam_msc_group);
	if (ret) {
		dev_err(dev,
			"Create mpam msc base interface Error, errorcode = %d\n",
			ret);
		goto group_err;
	}

	if (mpam_has_feature(mpam_feat_cpor_part, &msc->prop)) {
		ret = device_create_file(dev, &dev_attr_mpam_cpbm);
		if (ret) {
			dev_err(dev,
				"Create mpam msc cpbm interface Error, errorcode = %d\n",
				ret);
			goto cpbm_err;
		}
	}

	if (mpam_has_feature(mpam_feat_mbw_prop, &msc->prop)) {
		ret = device_create_file(dev, &dev_attr_mpam_mbw_prop);
		if (ret) {
			dev_err(dev,
				"Create mpam msc mbw prop interface Error, errorcode = %d\n",
				ret);
			goto prop_err;
		}
	}
	return 0;
prop_err:
	if (mpam_has_feature(mpam_feat_cpor_part, &msc->prop))
		device_remove_file(dev, &dev_attr_mpam_cpbm);
cpbm_err:
	device_remove_group(dev, &mpam_msc_group);
group_err:
	return ret;
}

int mpam_sysfs_interface_init(void)
{
	int err = 0;
	struct device *dev_root = bus_get_dev_root(&cpu_subsys);

	if (!dev_root) {
		pr_err(TAG "Get CPU Subsys dev_root failed\n");
		return -ENODEV;
	}
	mpam_kobj = kobject_create_and_add("mpam", &dev_root->kobj);
	put_device(dev_root);
	if (IS_ERR_OR_NULL(mpam_kobj)) {
		pr_err(TAG "mpam sysfs node create error, errorcode = %ld\n",
		       (long)mpam_kobj);
		return (long)mpam_kobj;
	}

	err = sysfs_create_group(mpam_kobj, &mpam_partid_map_group);
	if (err) {
		pr_err(TAG "mpam sysfs create group error, errorcode = %d\n",
		       err);
		kobject_put(mpam_kobj);
	}
	return err;
}
