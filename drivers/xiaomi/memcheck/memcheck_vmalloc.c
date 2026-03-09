// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/vmalloc.h>

#include "memcheck_ioctl.h"
#include "memcheck_vmalloc.h"

#define MAX_VMALLOC_SIZE (700 * 1024 * 1024)

static struct list_head *mmcheck_vmap_area_list;
static struct mutex *mmcheck_vmap_purge_lock;
static spinlock_t *mmcheck_vmap_area_lock;

static void vmalloc_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_puts(s, "Vmalloc info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%15s %50s %10s\n", "Size", "PC", "Pages");
	} else {
		memcheck_info("Vmalloc info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%15s %50s %10s\n", "Size", "PC", "Pages");
	}
}

static void vmalloc_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

void vmap_lock(void)
{
	mutex_lock(mmcheck_vmap_purge_lock);
	spin_lock(mmcheck_vmap_area_lock);
}

void vmap_unlock(void)
{
	spin_unlock(mmcheck_vmap_area_lock);
	mutex_unlock(mmcheck_vmap_purge_lock);
}

static int vmalloc_info_show(struct seq_file *s, void *d)
{
	struct vmap_area *va = NULL;
	struct vm_struct *v = NULL;
	unsigned long old_ns = ktime_get();

	vmalloc_info_print_header(s);

	vmap_lock();
	list_for_each_entry(va, mmcheck_vmap_area_list, list) {
		v = va->vm;
		if (!v || !(v->flags & VM_ALLOC))
			continue;
		if (s)
			seq_printf(s, "%15ld %50pS %10d\n", v->size, v->caller, v->nr_pages);
		else
			memcheck_info("%15ld %50pS %10d\n", v->size, v->caller, v->nr_pages);
	}
	vmap_unlock();

	memcheck_info("take %llu ns", ktime_get() - old_ns);
	vmalloc_info_print_foot(s);

	return 0;
}

void memcheck_vmalloc_info_show(void)
{
	u64 total = vmalloc_nr_pages() * PAGE_SIZE;

	memcheck_info("Total Vmalloc usage is %llu\n", total);
	if (total < MAX_VMALLOC_SIZE)
		return;

	if (!mmcheck_vmap_area_list)
		return ;
	if (!mmcheck_vmap_purge_lock)
		return ;
	if (!mmcheck_vmap_area_lock)
		return ;

	vmalloc_info_show(NULL, NULL);
}
EXPORT_SYMBOL(memcheck_vmalloc_info_show);

static int vmalloc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, vmalloc_info_show, pde_data(inode));
}

static const struct proc_ops vmalloc_info_fops = {
	.proc_open = vmalloc_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int memcheck_vmalloc_createfs(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *entry = NULL;

	MEMCHECK_DEBUG_LOOKUP(vmap_area_list, struct list_head);
	MEMCHECK_DEBUG_LOOKUP(vmap_purge_lock, struct mutex);
	MEMCHECK_DEBUG_LOOKUP(vmap_area_lock, spinlock_t);

	if (!mmcheck_vmap_area_list)
		return -EFAULT;
	if (!mmcheck_vmap_purge_lock)
		return -EFAULT;
	if (!mmcheck_vmap_area_lock)
		return -EFAULT;

	entry = proc_create("vmalloc_info", 0444, parent, &vmalloc_info_fops);
	if (!entry)
		memcheck_err("Failed to create vmalloc debug info\n");

	return (!entry ? -EFAULT : 0);
}
