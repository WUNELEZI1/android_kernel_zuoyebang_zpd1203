// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "mdr_print.h"
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <soc/xring/logbuf.h>
#include "dt-bindings/xring/platform-specific/dfx_memory_layout.h"

#include "mdr.h"

static int proc_show(struct seq_file *m, void *v)
{
	return 0;
}

static int mdr_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show, NULL);
}

static ssize_t mdr_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	u64 log_addr = get_logbuf_addr();
	u64 dfx_reservedmem_addr = get_dfx_reservedmem_addr();

	if (ppos == NULL) {
		pr_err("ppos is null\n");
		return -EINVAL;
	}

	if (buf == NULL) {
		pr_err("buf is null\n");
		return -EINVAL;
	}

	if (log_addr == 0) {
		pr_err("log_addr is null\n");
		return -EINVAL;
	}

	if (dfx_reservedmem_addr == 0) {
		pr_err("dfx_reservedmem_addr is null\n");
		return -EINVAL;
	}

	if (copy_to_user(buf, (void *)dfx_reservedmem_addr, DFX_RESERVED_MEM_SIZE)) {
		pr_err("copy to user failed\n");
		return -EFAULT;
	}

	buf += DFX_RESERVED_MEM_SIZE;
	if (copy_to_user(buf, (void *)log_addr, KERNELLOGSIZE)) {
		pr_err("copy to user failed\n");
		return -EFAULT;
	}

	return size;
}

static const struct proc_ops mdr_proc_fops = {
	.proc_open      = mdr_open,
	.proc_read      = mdr_read,
	.proc_release   = single_release,
};

int mdr_dump_node_init(void)
{
	struct proc_dir_entry *de;

	de = proc_create(MDR_DUMP_NODE, 0440, NULL, &mdr_proc_fops);
	if (!de) {
		pr_err("mdr proc created failed\n");
		return -ENOENT;
	}

	return 0;
}
