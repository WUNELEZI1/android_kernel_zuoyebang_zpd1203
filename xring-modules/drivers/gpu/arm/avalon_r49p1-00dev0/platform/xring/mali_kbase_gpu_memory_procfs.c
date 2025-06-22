// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <linux/proc_fs.h>
#include <mali_kbase.h>
#include <device/mali_kbase_device.h>
#include <mali_kbase_config.h>
#include <mali_kbase_defs.h>
#include <mali_kbase_gpu_memory_procfs.h>
#include <linux/kernel.h>

#if IS_ENABLED(CONFIG_PROC_FS)
static struct proc_dir_entry *g_xgpu_root;

static int kbasep_gpu_memory_proc_show(struct seq_file *sfile, void *data)
{
	struct list_head *entry;
	const struct list_head *kbdev_list;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = NULL;
		struct kbase_context *kctx;

		kbdev = list_entry(entry, struct kbase_device, entry);
		/* output the total memory usage and cap for this device */
		seq_printf(sfile, "%-16s  %10u\n",
				kbdev->devname,
				atomic_read(&(kbdev->memdev.used_pages)));
		mutex_lock(&kbdev->kctx_list_lock);
		list_for_each_entry(kctx, &kbdev->kctx_list, kctx_list_link) {
			/* output the memory usage and cap for each kctx
			 * opened on this device
			 */
			seq_printf(sfile, "%d %s-0x%pK %10u\n",
				kctx->tgid,
				"kctx",
				kctx,
				atomic_read(&(kctx->used_pages)));
		}
		mutex_unlock(&kbdev->kctx_list_lock);
	}
	kbase_device_put_list(kbdev_list);
	return 0;
}

/*
 *  File operations related to proc entry for gpu_memory
 */
static int kbasep_gpu_memory_procfs_open(struct inode *in, struct file *file)
{
	return single_open(file, kbasep_gpu_memory_proc_show, NULL);
}

/*
 *  It is mandatory to initialize proc_lseek,
 *  otherwise null pointer in kernel causing system reset
 */
static const struct proc_ops kbasep_gpu_memory_procfs_fops = {
	.proc_open = kbasep_gpu_memory_procfs_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

/*
 *  Initialize proc entry for gpu_memory
 */
struct proc_dir_entry *kbasep_gpu_memory_procfs_init(struct kbase_device *kbdev)
{
	struct proc_dir_entry *entry = NULL;

	g_xgpu_root = proc_mkdir("xgpu", NULL);
	if (!g_xgpu_root) {
		dev_err(kbdev->dev, "Can not create gpu directory\n");
		return NULL;
	}

	entry = proc_create("gpu_memory", 0444, g_xgpu_root, &kbasep_gpu_memory_procfs_fops);
	if (!entry) {
		remove_proc_entry("xgpu", NULL);
		dev_err(kbdev->dev, "Can not create gpu memory node\n");
		return NULL;
	}
	return entry;
}

void kbasep_gpu_memory_procfs_exit(void)
{
	if (g_xgpu_root) {
		remove_proc_entry("gpu_memory", g_xgpu_root);
		remove_proc_entry("xgpu", NULL);
		g_xgpu_root = NULL;
	}
}
#else
/*
 * Stub functions for when procfs is disabled
 */
struct proc_dir_entry *kbasep_gpu_memory_procfs_init(void) {
	return NULL;
}

void kbasep_gpu_memory_procfs_exit(void) {}
#endif
