// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Description: fastbootlog save driver
 */

#define pr_fmt(fmt)	"[xr_dfx][fastbootlog_save]:%s:%d " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include <soc/xring/dfx_switch.h>

#define FASTBOOTLOG_PROC_NAME	"fastbootlog"
#define MAX_MESSAGE_LENGTH	0x200

typedef struct _ddr_logbuf_header {
	u32 magic_num;               // logbuf header magic num
	u32 total_size;              // logbuf total_size except header
	u32 current_start_offset;    // current start log offset
	u32 last_start_offset;       // last start log offset
	u32 current_offset;          // current offset
} __packed ddr_logbuf_header_t;

static void *g_fastbootlog_addr;
static size_t g_fastbootlog_size;

static void fastbootlog_print(void);
static void fastbootlog_dump(char *start, u32 size);

static int proc_show(struct seq_file *m, void *v)
{
	return 0;
}

static int fastbootlog_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show, NULL);
}

static ssize_t fastbootlog_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	ssize_t copy_size;

	if (ppos == NULL) {
		pr_err("ppos is null\n");
		return 0;
	}

	if (buf == NULL) {
		pr_err("buf is null\n");
		return 0;
	}

	if (*ppos >= (loff_t)g_fastbootlog_size) {
		pr_err("ppos too large\n");
		return 0;
	}

	copy_size = (ssize_t)min(size, (size_t)(g_fastbootlog_size - *ppos));
	if (copy_to_user(buf, (char *)g_fastbootlog_addr + *ppos, copy_size)) {
		pr_err("copy to user failed\n");
		return -EFAULT;
	}

	*ppos += copy_size;

	return copy_size;
}

static struct proc_ops fastbootlog_proc_fops = {
	.proc_open	= fastbootlog_open,
	.proc_read	= fastbootlog_read,
	.proc_release	= single_release,
};

int fastbootlog_init(void)
{
	struct proc_dir_entry *de;

	g_fastbootlog_size = DFX_MEM_FASTBOOTLOG_SIZE;
	g_fastbootlog_addr = ioremap_wc(DFX_MEM_FASTBOOTLOG_ADDR, DFX_MEM_FASTBOOTLOG_SIZE);
	if (!g_fastbootlog_addr) {
		pr_err("fastbootlog ioremap error\n");
		return -ENOMEM;
	}

	de = proc_create(FASTBOOTLOG_PROC_NAME, 0440, NULL, &fastbootlog_proc_fops);
	if (!de) {
		pr_err("fastbootlog proc created failed\n");
		return -ENOENT;
	}

	fastbootlog_print();

	pr_info("fastbootlog init success\n");

	return 0;
}

void fastbootlog_exit(void)
{
	iounmap(g_fastbootlog_addr);
	pr_info("fastbootlog module exit\n");
}

/*
 * fastbootlog_print
 * @brief: print current boot fastbootlog
 */
static void fastbootlog_print(void)
{
	ddr_logbuf_header_t *header = (ddr_logbuf_header_t *)g_fastbootlog_addr;
	u32 start = header->current_start_offset;
	u32 end = header->current_offset;
	char *p_start = NULL;
	u32 len;

	if (!is_switch_on(SWITCH_FASTBOOTLOG_PRINT)) {
		pr_info("fastbootlog print switch off\n");
		return;
	}

	if (start > DFX_MEM_FASTBOOTLOG_SIZE || end > DFX_MEM_FASTBOOTLOG_SIZE) {
		pr_err("wrong start[0x%x] or end[0x%x] value\n", start, end);
		return;
	}

	pr_notice("print fastbootlog:\n");
	pr_notice("fastbootlog>>>>>>>>>>>>>>>>>>>>>start\n");
	p_start = (char *)header + start + sizeof(ddr_logbuf_header_t);
	if (start < end) {
		len = end - start;
		fastbootlog_dump(p_start, len);
	} else if (start > end) {
		len = DFX_MEM_FASTBOOTLOG_SIZE - start - sizeof(ddr_logbuf_header_t);
		fastbootlog_dump(p_start, len);
		p_start = (char *)header + sizeof(ddr_logbuf_header_t);
		len = end;
		fastbootlog_dump(p_start, len);
	} else {
		pr_err("fastbootlog is empty\n");
		return;
	}
	pr_notice("fastbootlog>>>>>>>>>>>>>>>>>>>>>end\n");
}

#ifdef pr_fmt
#undef pr_fmt
#define pr_fmt(fmt) fmt
#endif

/*
 * fastbootlog_dump
 * @brief: print log from start addr
 * @param start: start addr
 * @param size: size of log
 */
static void fastbootlog_dump(char *start, u32 size)
{
	u32 i;
	char *p = start;

	if (p == NULL) {
		pr_err("[xr_dfx][fastbootlog_save]:%s:%d null start pointer\n", __func__, __LINE__);
		return;
	}

	if (size >= DFX_MEM_FASTBOOTLOG_SIZE) {
		pr_err("[xr_dfx][fastbootlog_save]:%s:%d size %u is too large\n", __func__, __LINE__, size);
		return;
	}

	for (i = 0; i < size; i++) {
		if (start[i] == '\0')
			start[i] = ' ';
		if (start[i] == '\n') {
			start[i] = '\0';
			pr_notice("%s", p);
			start[i] = '\n';
			p = &start[i + 1];
		}
	}
}
