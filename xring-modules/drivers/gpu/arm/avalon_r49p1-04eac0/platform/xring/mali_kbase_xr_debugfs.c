// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#include <mali_kbase.h>
#include <device/mali_kbase_device.h>
#if IS_ENABLED(CONFIG_MALI_XR_SMART_GAMING)
#include <mali_kbase_xr_smart_gaming.h>
#endif
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
#include <mali_kbase_xr_mdr.h>
#endif
#include <mali_kbase_xr_dvfs.h>
#include <csf/ipa_control/mali_kbase_csf_ipa_control.h>
#if IS_ENABLED(CONFIG_DEBUG_FS)
#define MAX_PID_NAME_LEN 16
#define DVFS_MAX_TYPE_NUM 4

#if IS_ENABLED(CONFIG_MALI_XR_SMART_GAMING)
/**
 * kbasep_xr_fence_query_show - Show fence info
 * @sfile: The debugfs entry
 * @data: Data associated with the entry
 *
 * This function is called to get the contents of the @ fence_query debugfs
 * file. This is a count value of fence signal times.
 *
 * Return:
 * * 0 if successfully prints data in debugfs entry file
 * * -1 if it encountered an error
 */
static int kbasep_xr_fence_query_show(struct seq_file *sfile, void *data)
{
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = NULL;

		kbdev = list_entry(entry, struct kbase_device, entry);
		seq_printf(sfile, "%-16s %u %10llu\n",
				kbdev->devname,
				kbdev->gpu_dev_data.query_pid,
				kbdev->gpu_dev_data.fence_count);
	}
	kbase_device_put_list(kbdev_list);
	return 0;
}

/*
 *  File operations related to debugfs entry for fence query
 */
static int kbasep_xr_fence_query_debugfs_open(struct inode *in, struct file *file)
{
	return single_open(file, kbasep_xr_fence_query_show, NULL);
}

static ssize_t kbase_xr_fence_query_debugfs_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	unsigned long pid = 0;
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;
	char raw_str[MAX_PID_NAME_LEN];
	struct kbase_device *kbdev = NULL;

	CSTD_UNUSED(ppos);

	if (count >= MAX_PID_NAME_LEN)
		return -E2BIG;
	if (copy_from_user(raw_str, ubuf, count))
		return -EINVAL;
	raw_str[count] = '\0';
	if (kstrtoul(raw_str, 0, &pid))
		return -EINVAL;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		kbdev = list_entry(entry, struct kbase_device, entry);
		kbdev->gpu_dev_data.query_pid = pid;
	}
	kbase_device_put_list(kbdev_list);

	return count;
}

static const struct file_operations kbasep_fence_query_debugfs_fops = {
	.owner = THIS_MODULE,
	.open = kbasep_xr_fence_query_debugfs_open,
	.read = seq_read,
	.write = kbase_xr_fence_query_debugfs_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static void kbasep_xr_fence_query_debugfs_init(struct kbase_device *kbdev, struct dentry *dentry)
{
	debugfs_create_file("fence_query", 0644,
			dentry, kbdev,
			&kbasep_fence_query_debugfs_fops);
}

/**
 * kbasep_counters_show - Show counters value
 * @sfile: The debugfs entry
 * @data: Data associated with the entry
 *
 * This function is called to get the contents of the @ counter_query debugfs
 * file. This is a count value of counters.
 *
 * Return:
 * * 0 if successfully prints data in debugfs entry file
 * * -1 if it encountered an error
 */
static int kbasep_xr_counter_query_show(struct seq_file *sfile, void *data)
{
	int i = 0;
	u64 counter_value[XR_NUM_PERF_COUNTERS] = {0};

	kbase_xr_query_hwcounter(counter_value, XR_NUM_PERF_COUNTERS);
	for (i = 0; i < XR_NUM_PERF_COUNTERS; i++) {
		seq_printf(sfile, "counter %d value: %llu\n",
			i, counter_value[i]);
	}
	return 0;
}

/*
 *  File operations related to debugfs entry for counter query
 */
static int kbasep_xr_counter_query_debugfs_open(struct inode *in, struct file *file)
{
	return single_open(file, kbasep_xr_counter_query_show, NULL);
}

static const struct file_operations kbasep_counter_query_debugfs_fops = {
	.owner = THIS_MODULE,
	.open = kbasep_xr_counter_query_debugfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static void kbasep_xr_counter_query_debugfs_init(struct kbase_device *kbdev, struct dentry *dentry)
{
	debugfs_create_file("counter_query", 0444,
			dentry, NULL,
			&kbasep_counter_query_debugfs_fops);
}
#endif

#if IS_ENABLED(CONFIG_MALI_XR_MDR)
static void kbasep_xr_report_mdr(struct kbase_device *kbdev, struct kbase_context *kctx, u8 mdr_type)
{
	switch (mdr_type) {
	case 0:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_IRQ_FAULT, 0, 0);
		break;
	case 1:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_BUS_FAULT, 0, 0);
		break;
	case 2:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_FAULT, 0, 0);
		break;
	case 3:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_CS_FAULT, 0, 0);
		break;
	case 4:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_UNHANDLE_PAGE_FAULT, 0, 0);
		break;
	case 5:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_MCU_UNHANDLE_PAGE_FAULT, 0, 0);
		break;
	case 6:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_SOFT_RESET_TIME_OUT, 0, 0);
		break;
	case 7:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_HARD_RESET_TIME_OUT, 0, 0);
		break;
	case 8:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_BIT_STUCK, 0, 0);
		break;
	case 9:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_REGULATOR_ON_FAIL, 0, 0);
		break;
	case 10:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_REGULATOR_OFF_FAIL, 0, 0);
		break;
	case 11:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_FENCE_TIMEOUT, 0, 0);
		break;
	case 12:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_INTERRUPT_TIMEOUT, 0, 0);
		break;
	case 13:
		kbase_xr_mdr_system_error(kbdev, kctx, MODID_GPU_CS_FATAL, 0, 0);
		break;
	default:
		dev_err(kbdev->dev, "Unexpected mdr_type");
	}
}

/**
 * kbasep_xr_mdr_trigger_show - Trigger mdr report
 * @sfile: The debugfs entry
 * @data: Data associated with the entry
 *
 * This function is called to trigger the error report to mdr.
 *
 * Return:
 * * 0 if successfully prints data in debugfs entry file
 * * -1 if it encountered an error
 */
static int kbasep_xr_mdr_trigger_show(struct seq_file *sfile, void *data)
{
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;
	struct kbase_context *const kctx = sfile->private;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = NULL;

		kbdev = list_entry(entry, struct kbase_device, entry);
		seq_printf(sfile, "%-16s trigger mdr type %u\n",
				kbdev->devname,
				kbdev->gpu_dev_data.mdr_type);
		kbasep_xr_report_mdr(kbdev, kctx, kbdev->gpu_dev_data.mdr_type);
	}
	kbase_device_put_list(kbdev_list);
	return 0;
}

/*
 *  File operations related to debugfs entry for fence query
 */
static int kbasep_xr_mdr_trigger_debugfs_open(struct inode *in, struct file *file)
{
	return single_open(file, kbasep_xr_mdr_trigger_show, NULL);
}

static ssize_t kbase_xr_mdr_trigger_debugfs_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	unsigned long type = 0;
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;
	struct kbase_device *kbdev = NULL;
	char raw_str[MAX_PID_NAME_LEN];

	if (count >= MAX_PID_NAME_LEN)
		return -E2BIG;
	if (copy_from_user(raw_str, ubuf, count))
		return -EINVAL;
	raw_str[count] = '\0';
	if (kstrtoul(raw_str, 0, &type))
		return -EINVAL;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		kbdev = list_entry(entry, struct kbase_device, entry);
		if (type < MDR_GPU_TYPES_COUNT)
			kbdev->gpu_dev_data.mdr_type = type;
		dev_info(kbdev->dev, "MDR write %lu", type);
	}
	kbase_device_put_list(kbdev_list);

	return count;
}

static const struct file_operations kbasep_mdr_trigger_debugfs_fops = {
	.owner = THIS_MODULE,
	.open = kbasep_xr_mdr_trigger_debugfs_open,
	.read = seq_read,
	.write = kbase_xr_mdr_trigger_debugfs_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static void kbasep_xr_mdr_trigger_debugfs_init(struct kbase_device *kbdev, struct dentry *dentry)
{
	debugfs_create_file("mdr_trigger", 0644,
			dentry, kbdev,
			&kbasep_mdr_trigger_debugfs_fops);
}
#endif

#if IS_ENABLED(CONFIG_MALI_XR_MDR)
/**
 * kbasep_xr_mdr_info_show - Show mdr cumulative number of times and thresholds.
 * @sfile: The debugfs entry
 * @data: Data associated with the entry
 *
 * This function is called to Show mdr cumulative number of times and thresholds.
 *
 * Return:
 * * 0 if successfully prints data in debugfs entry file
 * * -1 if it encountered an error
 */
static int kbasep_xr_mdr_info_show(struct seq_file *sfile, void *data)
{
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;
	struct kbase_context *kctx = NULL;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = NULL;

		kbdev = list_entry(entry, struct kbase_device, entry);
		seq_printf(sfile, "Type:                             "
			"IRQ_FAULT  BUS_FAULT  GPU_FAULT  CS_FAULT  "
			"PAGE_FAULT  MCU_PAGE_FAULT  SOFT_RS_TM  "
			"HARD_RESET_TM  BIT_STUCK  PM_ON  PM_OFF  FENCE_TM  IRQ_TM  CS_FATAL\n");
		seq_printf(sfile, "Threshold:                        "
			"%2u         %2u         %2u         %2u         "
			"%2u           %2u             %2u         %2u              "
			"%2u        %2u      %2u      %2u         %2u         %2u\n",
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_IRQ_FAULT],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_BUS_FAULT],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_FAULT],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_CS_FAULT],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_UNHANDLE_PAGE_FAULT],
			kbdev->gpu_dev_data.mdr_threshold[MDR_MCU_UNHANDLE_PAGE_FAULT],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_SOFT_RESET_TIME_OUT],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_HARD_RESET_TIME_OUT],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_BIT_STUCK],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_REGULATOR_ON_FAIL],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_REGULATOR_OFF_FAIL],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_FENCE_TIMEOUT],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_INTERRUPT_TIMEOUT],
			kbdev->gpu_dev_data.mdr_threshold[MDR_GPU_CS_FATAL]);

		mutex_lock(&kbdev->kctx_list_lock);
		list_for_each_entry(kctx, &kbdev->kctx_list, kctx_list_link) {
			/* output the mdr info for each kctx
			 * opened on this device
			 */
			seq_printf(sfile, "%d %s-0x%pK:     "
				"%2u         %2u         %2u         %2u         "
				"%2u           %2u             %2u         %2u              "
				"%2u        %2u      %2u      %2u         %2u         %2u\n",
				kctx->tgid, "kctx", kctx,
				kctx->mdr_count[MDR_GPU_IRQ_FAULT],
				kctx->mdr_count[MDR_GPU_BUS_FAULT],
				kctx->mdr_count[MDR_GPU_FAULT],
				kctx->mdr_count[MDR_GPU_CS_FAULT],
				kctx->mdr_count[MDR_GPU_UNHANDLE_PAGE_FAULT],
				kctx->mdr_count[MDR_MCU_UNHANDLE_PAGE_FAULT],
				kctx->mdr_count[MDR_GPU_SOFT_RESET_TIME_OUT],
				kctx->mdr_count[MDR_GPU_HARD_RESET_TIME_OUT],
				kctx->mdr_count[MDR_GPU_BIT_STUCK],
				kctx->mdr_count[MDR_GPU_REGULATOR_ON_FAIL],
				kctx->mdr_count[MDR_GPU_REGULATOR_OFF_FAIL],
				kctx->mdr_count[MDR_GPU_FENCE_TIMEOUT],
				kctx->mdr_count[MDR_GPU_INTERRUPT_TIMEOUT],
				kctx->mdr_count[MDR_GPU_CS_FATAL]);
		}
		mutex_unlock(&kbdev->kctx_list_lock);
		mutex_lock(&kbdev->gpu_dev_data.mdr_lock);
		seq_printf(sfile, "mali0:                            "
			"%2u         %2u         %2u         %2u         "
			"%2u           %2u             %2u         %2u              "
			"%2u        %2u      %2u      %2u         %2u         %2u\n",
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_IRQ_FAULT],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_BUS_FAULT],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_FAULT],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_CS_FAULT],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_UNHANDLE_PAGE_FAULT],
			kbdev->gpu_dev_data.mdr_count[MDR_MCU_UNHANDLE_PAGE_FAULT],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_SOFT_RESET_TIME_OUT],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_HARD_RESET_TIME_OUT],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_BIT_STUCK],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_REGULATOR_ON_FAIL],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_REGULATOR_OFF_FAIL],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_FENCE_TIMEOUT],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_INTERRUPT_TIMEOUT],
			kbdev->gpu_dev_data.mdr_count[MDR_GPU_CS_FATAL]);
		mutex_unlock(&kbdev->gpu_dev_data.mdr_lock);
	}
	kbase_device_put_list(kbdev_list);
	return 0;
}

/*
 *  File operations related to debugfs entry for bugon
 */
static int kbasep_xr_mdr_info_debugfs_open(struct inode *in, struct file *file)
{
	return single_open(file, kbasep_xr_mdr_info_show, NULL);
}

static ssize_t kbase_xr_mdr_info_debugfs_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;
	struct kbase_device *kbdev = NULL;
	char raw_str[MAX_PID_NAME_LEN];
	char str1[MAX_PID_NAME_LEN];
	char str2[MAX_PID_NAME_LEN];
	unsigned long index = 0;
	unsigned long threshold = 0;

	if (count >= MAX_PID_NAME_LEN)
		return -E2BIG;
	if (copy_from_user(raw_str, ubuf, count))
		return -EINVAL;
	raw_str[count] = '\0';
	if (sscanf(raw_str, "%s %s", str1, str2) != 2)
		return -EINVAL;
	if (kstrtoul(str1, 0, &index))
		return -EINVAL;
	if (kstrtoul(str2, 0, &threshold))
		return -EINVAL;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		kbdev = list_entry(entry, struct kbase_device, entry);
		if (index < MDR_GPU_TYPES_COUNT)
			kbdev->gpu_dev_data.mdr_threshold[index] = threshold;
	}
	kbase_device_put_list(kbdev_list);

	return count;
}

static const struct file_operations kbasep_mdr_info_debugfs_fops = {
	.owner = THIS_MODULE,
	.open = kbasep_xr_mdr_info_debugfs_open,
	.read = seq_read,
	.write = kbase_xr_mdr_info_debugfs_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static void kbasep_xr_mdr_info_debugfs_init(struct kbase_device *kbdev, struct dentry *dentry)
{
	debugfs_create_file("mdr_info", 0644,
			dentry, kbdev,
			&kbasep_mdr_info_debugfs_fops);
}
#endif

#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
static void kbasep_xr_report_dvfs(struct seq_file *sfile, struct kbase_device *kbdev, u8 dvfs_type)
{
	int ret;
	u64 ori_gpu_rate = 0;
	u64 new_gpu_freq = 0;
	u64 freq = 0;
	u32 utilization = 0;
	struct xr_gpufreq_ops *gpufreq_ops = &kbdev->gpu_dev_data.gpufreq_ops;

	ori_gpu_rate = kbdev->csf.ipa_control.cur_gpu_rate;

	switch (dvfs_type) {
	case 0:
		ret = kbase_xr_register_dvfs_op(kbdev);
		seq_printf(sfile, "%-16s trigger dvfs type %u call dvfs result: %d\n",
				kbdev->devname,
				dvfs_type,
				ret);
		break;
	case 1:
		if (atomic_read(&kbdev->gpu_dev_data.gpufreq_ready) == 1)
			if (gpufreq_ops->gpufreq_get_freq)
				freq = gpufreq_ops->gpufreq_get_freq();
		seq_printf(sfile, "%-16s trigger dvfs type %u call dvfs result %llu\n",
				kbdev->devname,
				dvfs_type,
				freq);
		break;
	case 2:
		utilization = kbase_xr_get_utilisation(kbdev->dev);
		seq_printf(sfile, "%-16s trigger dvfs type %u call dvfs result %d\n",
				kbdev->devname,
				dvfs_type,
				utilization);
		break;
	case 3:
		kbase_xr_notify_change(kbdev->dev, 100);
		new_gpu_freq = kbdev->csf.ipa_control.cur_gpu_rate;
		seq_printf(sfile, "%-16s trigger dvfs type %u call dvfs result %llu\n",
				kbdev->devname,
				dvfs_type,
				new_gpu_freq);
		kbase_xr_notify_change(kbdev->dev, ori_gpu_rate);
		break;

	default:
		dev_err(kbdev->dev, "Unexpected dvfs_type");
	}
}

/**
 * kbasep_xr_dvfs_show - Show dvfs debug info
 * @sfile: The debugfs entry
 * @data: Data associated with the entry
 *
 * This function is called to show the dvfs debug report.
 *
 * Return:
 * * 0 if successfully prints data in debugfs entry file
 * * -1 if it encountered an error
 */
static int kbasep_xr_dvfs_show(struct seq_file *sfile, void *data)
{
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = NULL;

		kbdev = list_entry(entry, struct kbase_device, entry);
		kbasep_xr_report_dvfs(sfile, kbdev, kbdev->gpu_dev_data.dvfs_type);
		seq_printf(sfile, "%-16s trigger dvfs type %u\n",
				kbdev->devname,
				kbdev->gpu_dev_data.dvfs_type);
	}
	kbase_device_put_list(kbdev_list);
	return 0;
}

static int kbasep_xr_dvfs_debugfs_open(struct inode *in, struct file *file)
{
	return single_open(file, kbasep_xr_dvfs_show, NULL);
}

static ssize_t kbase_xr_dvfs_debugfs_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	unsigned long type = 0;
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;
	struct kbase_device *kbdev = NULL;
	char raw_str[MAX_PID_NAME_LEN];

	if (count >= MAX_PID_NAME_LEN)
		return -E2BIG;
	if (copy_from_user(raw_str, ubuf, count))
		return -EINVAL;
	raw_str[count] = '\0';
	if (kstrtoul(raw_str, 0, &type))
		return -EINVAL;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		kbdev = list_entry(entry, struct kbase_device, entry);
		if (type < DVFS_MAX_TYPE_NUM)
			kbdev->gpu_dev_data.dvfs_type = type;
		dev_info(kbdev->dev, "DVFS write %lu", type);
	}
	kbase_device_put_list(kbdev_list);
	return count;
}

static const struct file_operations kbasep_dvfs_debugfs_fops = {
	.owner = THIS_MODULE,
	.open = kbasep_xr_dvfs_debugfs_open,
	.read = seq_read,
	.write = kbase_xr_dvfs_debugfs_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static void kbasep_xr_dvfs_debugfs_init(struct kbase_device *kbdev, struct dentry *dentry)
{
	debugfs_create_file("dvfs", 0644,
			dentry, kbdev,
			&kbasep_dvfs_debugfs_fops);
}
#endif
/*
 *  Initialize debugfs entry for xr
 */
void kbasep_xr_debugfs_init(struct kbase_device *kbdev)
{
	struct dentry *dentry = debugfs_create_dir("xr", kbdev->mali_debugfs_directory);

	if (IS_ERR_OR_NULL(dentry))
		dev_err(kbdev->dev, "Couldn't create mali debugfs xr directory\n");

#if IS_ENABLED(CONFIG_MALI_XR_SMART_GAMING)
	kbasep_xr_fence_query_debugfs_init(kbdev, dentry);
	kbasep_xr_counter_query_debugfs_init(kbdev, dentry);
#endif
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
	kbasep_xr_mdr_trigger_debugfs_init(kbdev, dentry);
	kbasep_xr_mdr_info_debugfs_init(kbdev, dentry);
#endif
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
	kbasep_xr_dvfs_debugfs_init(kbdev, dentry);
#endif
}
#else
/*
 * Stub functions for when debugfs is disabled
 */
void kbasep_xr_debugfs_init(struct kbase_device *kbdev) {}
#endif
