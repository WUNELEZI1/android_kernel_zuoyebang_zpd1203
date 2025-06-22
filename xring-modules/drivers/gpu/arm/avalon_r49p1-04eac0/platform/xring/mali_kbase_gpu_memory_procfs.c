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
#include <mali_kbase_trace_gpu_mem.h>
#include <linux/dma-buf.h>

#if IS_ENABLED(CONFIG_PROC_FS)
static struct proc_dir_entry *g_xgpu_root;

struct kbase_tgid_mem_info {
	int tgid;
	int gl_pages;
	int pss_pages;
	int rss_pages;
	struct rb_node tgid_mem_node;
};

int kbasep_gpu_memory_procs_egl(struct kbase_device *kbdev, struct rb_root *root, bool is_pss)
{
	struct rb_node *node = NULL;
	struct kbase_dma_buf *buf_node;
	int total_size = 0;

	lockdep_assert_held(&kbdev->dma_buf_lock);

	if (!root) {
		dev_err(kbdev->dev, "Invalid rb root\n");
		return 0;
	}

	// traverse all the dmabuf node in rb tree
	for (node = rb_first(root); node; node = rb_next(node)) {
		buf_node = rb_entry(node, struct kbase_dma_buf, dma_buf_node);

		if (!buf_node || !buf_node->dma_buf) {
			dev_err(kbdev->dev,"Invalid buffer node or dma_buf\n");
			continue;
		}

		if (is_pss) {
			// When the used_count is 1, means it's this process firstly use this dmabuf
			if (buf_node->used_count == 1) {
				total_size += buf_node->dma_buf->size;
			}
		} else {
			// Calculate rss by sum up all dmabuf node
			total_size += buf_node->dma_buf->size;
		}
	}
	return PAGE_ALIGN(total_size) >> PAGE_SHIFT;
}

static int kbasep_gpu_memory_proc_show(struct seq_file *sfile, void *data)
{
	struct list_head *entry;
	const struct list_head *kbdev_list;
	struct rb_root tgid_root = RB_ROOT;
	struct rb_node *node;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = list_entry(entry, struct kbase_device, entry);
		struct kbase_context *kctx = NULL;

		mutex_lock(&kbdev->dma_buf_lock);
		seq_printf(sfile, "%-16s  %10d  %10d\n",
				kbdev->devname,
				atomic_read(&(kbdev->memdev.used_pages)),
				kbasep_gpu_memory_procs_egl(kbdev, &kbdev->dma_buf_root, false));
		mutex_unlock(&kbdev->dma_buf_lock);

		mutex_lock(&kbdev->kctx_list_lock);
		list_for_each_entry(kctx, &kbdev->kctx_list, kctx_list_link) {
			struct kbase_tgid_mem_info *tgid_info = NULL;
			struct rb_node **new_node = &(tgid_root.rb_node);
			struct rb_node *parent = NULL;

			while (*new_node) {
				struct kbase_tgid_mem_info *info = rb_entry(*new_node, struct kbase_tgid_mem_info, tgid_mem_node);
				parent = *new_node;

				if (kctx->tgid == info->tgid) {
					tgid_info = info;
					break;
				} else if (kctx->tgid < info->tgid) {
					new_node = &((*new_node)->rb_left);
				} else {
					new_node = &((*new_node)->rb_right);
				}
			}

			if (!tgid_info) {
				// Create new tgid node
				tgid_info = kzalloc(sizeof(struct kbase_tgid_mem_info), GFP_KERNEL);
				if (!tgid_info) {
					dev_err(kbdev->dev,"Failed to alloc memory for tgid info\n");
					continue;
				}
				tgid_info->tgid = kctx->tgid;
				rb_link_node(&tgid_info->tgid_mem_node, parent, new_node);
				rb_insert_color(&tgid_info->tgid_mem_node, &tgid_root);
			}

			mutex_lock(&kbdev->dma_buf_lock);
			// Sum up kctx's dmabuffer belongs to same tgid
			tgid_info->gl_pages += atomic_read(&(kctx->used_pages));
			tgid_info->pss_pages = kbasep_gpu_memory_procs_egl(kbdev, &kctx->kprcs->dma_buf_root, true);
			tgid_info->rss_pages = kbasep_gpu_memory_procs_egl(kbdev, &kctx->kprcs->dma_buf_root, false);
			mutex_unlock(&kbdev->dma_buf_lock);
		}
		mutex_unlock(&kbdev->kctx_list_lock);
	}

	// show memory info for each tgid
	for (node = rb_first(&tgid_root); node; node = rb_next(node)) {
		struct kbase_tgid_mem_info *tgid_info = rb_entry(node, struct kbase_tgid_mem_info, tgid_mem_node);
		seq_printf(sfile, "%-16d  %10d  %10d  %10d\n",
			   tgid_info->tgid,
			   tgid_info->gl_pages,
			   tgid_info->pss_pages,
			   tgid_info->rss_pages);
	}

	// release tgid nodes
	do {
		node = rb_first(&tgid_root);
		if (node) {
			rb_erase(node, &tgid_root);
			struct kbase_tgid_mem_info *tgid_info = rb_entry(node, struct kbase_tgid_mem_info, tgid_mem_node);
			kfree(tgid_info);
		}
	} while (node);

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
