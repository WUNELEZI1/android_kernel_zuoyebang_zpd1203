// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/msm_sysstats.h>

#include "memcheck_ioctl.h"
#include "memcheck_gpumem.h"


static bool gpu_print_enforce = false;
module_param(gpu_print_enforce, bool, 0644);
static int gpumem_info_show(struct seq_file *s, void *data);

static void gpumem_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_puts(s, "Process KGSL info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%5s\t%5s\t%20s\t%20s\t%20s\t%20s\n",
					"PID", "PNAME", "KGSL_CUR_MEMORY", "DMABUF_CUR_MEMORY",
					"PROCESS_PRIVATE_PTR", "KGSL_PAGETABLE_ADDRESS");
	} else {
		memcheck_info("Process KGSL info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%5s\t%5s\t%20s\t%20s\t%20s\t%20s\n",
					"PID", "PNAME", "KGSL_CUR_MEMORY", "DMABUF_CUR_MEMORY",
					"PROCESS_PRIVATE_PTR", "KGSL_PAGETABLE_ADDRESS");
	}
}

static void gpumem_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

static void show_val_kb(struct seq_file *m, const char *s, unsigned long num)
{
	seq_put_decimal_ull_width(m, s, TO_KB(num), 8);
	seq_write(m, " kB\n", 4);
}

void memcheck_gpumem_hook_meminfo(struct seq_file *s)
{
	if (memcheck_kgsl_memstat == NULL) {
		memcheck_err("memcheck_kgsl_memstat is NULL\n");
		return;
	}

	if (s)
		show_val_kb(s, "GpuTotal:       ",
					memcheck_kgsl_memstat("coherent") +
					memcheck_kgsl_memstat("page_alloc") +
					memcheck_kgsl_memstat("secure"));

}

unsigned long memcheck_kgsl_get_total(void)
{
	unsigned long total = 0;

	if (memcheck_kgsl_memstat == NULL) {
		memcheck_err("memcheck_kgsl_memstat is NULL\n");
		return total;
	}

	total = memcheck_kgsl_memstat("coherent") +
			memcheck_kgsl_memstat("page_alloc") +
			memcheck_kgsl_memstat("secure");

	return total;
}

void memcheck_gpumem_info_show(void)
{
	gpumem_info_show(NULL, NULL);
}

int gpumem_walk_process_privates_cb(void *data)
{
	struct memcheck_gpumem_pinfo *p_data = (struct memcheck_gpumem_pinfo *)data;

	if (p_data == NULL)
		return -EFAULT;

	if (p_data->seq) {
		seq_printf(p_data->seq, "%5d\t%5s\t%15lu kB\t%15lu kB\t",  p_data->pid, p_data->comm,
									TO_KB(p_data->kgsl_mem), TO_KB(p_data->kgsl_dmabuf));
		if (p_data->print_enforced) {
			seq_printf(p_data->seq, "%20ps\t%20ps\n", p_data->private_base, p_data->pagetable);
		} else {
			seq_printf(p_data->seq, "%20p\t%20p\n", p_data->private_base, p_data->pagetable);
		}
	} else {
		memcheck_info("%5d\t%5s\t%15lu kB\t%15lu kB\t",  p_data->pid, p_data->comm,
									TO_KB(p_data->kgsl_mem), TO_KB(p_data->kgsl_dmabuf));
		if (p_data->print_enforced) {
			memcheck_info("%20ps\t%20ps\n", p_data->private_base, p_data->pagetable);
		} else {
			memcheck_info("%20p\t%20p\n", p_data->private_base, p_data->pagetable);
		}
	}
	return 0;
}

static int gpumem_info_show(struct seq_file *s, void *data)
{
	unsigned long old_ns = ktime_get();
	struct memcheck_gpumem_pinfo cb_args;

	cb_args.seq = s;
	cb_args.print_enforced = gpu_print_enforce;

	if (memcheck_kgsl_memstat == NULL) {
		memcheck_err("memcheck_kgsl_memstat is NULL\n");
		return -EFAULT;
	}
	if (memcheck_walk_process_privates == NULL) {
		memcheck_err("memcheck_walk_process_privates is NULL\n");
		return -EFAULT;
	}

	if (cb_args.seq) {
		seq_printf(s, "GpuTotal: %zu kB\n",
					TO_KB(memcheck_kgsl_memstat("coherent") +
					memcheck_kgsl_memstat("page_alloc") +
					memcheck_kgsl_memstat("secure")));
		seq_printf(s, "page_alloc: %zu kB, secure: %zu kB, coherent: %zu kB\n",
					TO_KB(memcheck_kgsl_memstat("page_alloc")),
					TO_KB(memcheck_kgsl_memstat("secure")),
					TO_KB(memcheck_kgsl_memstat("coherent")));
	} else {
		memcheck_info("GpuTotal: %zu kB\n",
						TO_KB(memcheck_kgsl_memstat("coherent") +
						memcheck_kgsl_memstat("page_alloc") +
						memcheck_kgsl_memstat("secure")));
		memcheck_info("page_alloc: %zu kB, secure: %zu kB, coherent: %zu kB\n",
						TO_KB(memcheck_kgsl_memstat("page_alloc")),
						TO_KB(memcheck_kgsl_memstat("secure")),
						TO_KB(memcheck_kgsl_memstat("coherent")));
	}

	gpumem_info_print_header(s);
	memcheck_walk_process_privates(gpumem_walk_process_privates_cb, &cb_args);
	gpumem_info_print_foot(s);

	memcheck_info("take %llu ns", ktime_get() - old_ns);

	return 0;
}

static int gpumem_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, gpumem_info_show, pde_data(inode));
}

static const struct proc_ops gpumem_info_fops = {
	.proc_open = gpumem_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int memcheck_gpumem_createfs(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create("gpumem_process_info", 0444, parent, &gpumem_info_fops);
	if (!entry)
		memcheck_err("Failed to create gpu buffer debug info\n");

	return (!entry ? -EFAULT : 0);
}
