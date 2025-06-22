// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "dfx", __func__, __LINE__

#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <linux/debugfs.h>
#include "xrisp_debug.h"
#include "xrisp_rproc_api.h"
#include "xrisp_pm_api.h"
#include "xrisp_log.h"

#define R82_AP_IRQ_NUM 248
#define WDT_IRQ_NUM    324

void __iomem *base;
char reg_out_buf[30];
static uint32_t irq;
static int arr_irq[33];
struct irq_register_list {
	int irq;
	struct list_head list;
};

struct manage_regdump {
	struct dentry *regdump_debugfs;
	struct dentry *blob_dentry;
	void __iomem *dump_base;
	uint32_t dump_address;
	uint32_t dump_length;
	uint32_t reg_address;
	uint32_t reg_value;
	struct list_head irq_sign_list;
};

static struct manage_regdump g_regdump = {
	.regdump_debugfs = NULL,
	.blob_dentry = NULL,
	.dump_base = NULL,
	.dump_length = 0,
	.dump_address = 0,
	.reg_address = 0,
	.reg_value = 0,
};

__maybe_unused int xrisp_reg_dump(phys_addr_t reg_addr, uint32_t len)
{
	static struct debugfs_blob_wrapper blob_buf;

	if (g_regdump.blob_dentry != NULL) {
		debugfs_remove(g_regdump.blob_dentry);
		g_regdump.blob_dentry = NULL;
	}

	if (g_regdump.dump_base != NULL) {
		XRISP_PR_INFO("last alloc memory free");
		iounmap(g_regdump.dump_base);
		g_regdump.dump_base = NULL;
	}

	g_regdump.dump_base = ioremap(reg_addr, len);
	if (g_regdump.dump_base == NULL) {
		XRISP_PR_ERROR("reg_addr 0x%llx ioremap failed", reg_addr);
		return -EINVAL;
	}
	XRISP_PR_INFO("reg_addr map va=0x%pK da=0x%llx", g_regdump.dump_base, reg_addr);

	blob_buf.data = g_regdump.dump_base;
	blob_buf.size = len;
	g_regdump.blob_dentry = debugfs_create_blob("dump_file", 0200,
						    g_regdump.regdump_debugfs, &blob_buf);
	if (IS_ERR_OR_NULL(g_regdump.blob_dentry)) {
		XRISP_PR_ERROR("blob file creation failed");
		g_regdump.blob_dentry = NULL;
		goto err_blob;
	}
	return 0;

err_blob:
	iounmap(g_regdump.dump_base);
	g_regdump.dump_base = NULL;
	return -EINVAL;
}

static void dump_free_memory(void)
{
	if (g_regdump.blob_dentry != NULL) {
		debugfs_remove(g_regdump.blob_dentry);
		g_regdump.blob_dentry = NULL;
	}

	if (g_regdump.dump_base != NULL) {
		XRISP_PR_INFO("free iomap memory 0x%pK", g_regdump.dump_base);
		iounmap(g_regdump.dump_base);
		g_regdump.dump_base = NULL;
	}
}

static ssize_t dfx_dump_reg_ops(struct file *file, const char __user *user_buf,
				size_t count, loff_t *ppos)
{
	int ret = 0;
	char cmd[10];

	if (!count || count > sizeof(cmd))
		return -EINVAL;

	if ((g_regdump.dump_address & 3) || (g_regdump.dump_length & 3)) {
		XRISP_PR_ERROR("input address or length need to multiple of 4, addr 0x%x ,len %d",
		       g_regdump.dump_address, g_regdump.dump_length);
		return -EINVAL;
	}

	ret = copy_from_user(cmd, user_buf, count);
	if (ret)
		return -EFAULT;

	if (!strncmp(cmd, "dump", count - 1)) {
		ret = xrisp_reg_dump(g_regdump.dump_address, g_regdump.dump_length);
		if (ret)
			return -EINVAL;
	} else if (!strncmp(cmd, "free", count - 1)) {
		dump_free_memory();
	} else {
		XRISP_PR_INFO("Unrecognized option\n");
		ret = -EINVAL;
	}

	return count;
}

static const struct file_operations dump_regs_ops = {
	.open = simple_open,
	.write = dfx_dump_reg_ops,
};

static ssize_t dfx_reg_result_show(struct file *file, char __user *user_buf, size_t count,
				   loff_t *ppos)
{
	uint32_t get_val;

	if (*ppos >= sizeof(reg_out_buf))
		return 0;

	if (g_regdump.reg_address == 0 || (g_regdump.reg_address & 3)) {
		XRISP_PR_ERROR("input reg addr error, addr = 0x%x", g_regdump.reg_address);
		return -EINVAL;
	}

	memset(reg_out_buf, 0, sizeof(reg_out_buf));

	if (g_regdump.reg_value == 0) {/* read */
		base = ioremap(g_regdump.reg_address, 4);

		get_val = readl_relaxed(base);
		XRISP_PR_INFO("read: 0x%x = 0x%x\n", g_regdump.reg_address, get_val);
		scnprintf(reg_out_buf, sizeof(reg_out_buf), "read 0x%x = 0x%x\n",
			  g_regdump.reg_address, get_val);

		iounmap(base);
	} else {/* write */
		base = ioremap(g_regdump.reg_address, 4);

		writel_relaxed(g_regdump.reg_value, base);
		XRISP_PR_INFO("write: 0x%x = 0x%x\n", g_regdump.reg_address,
			g_regdump.reg_value);
		scnprintf(reg_out_buf, sizeof(reg_out_buf), "write 0x%x = 0x%x\n",
			  g_regdump.reg_address, g_regdump.reg_value);

		g_regdump.reg_value = 0;
		iounmap(base);
	}

	return simple_read_from_buffer(user_buf, count, ppos, reg_out_buf,
				       sizeof(reg_out_buf));
}

static const struct file_operations reg_wr_ops = {
	.owner = THIS_MODULE,
	.read = dfx_reg_result_show,
};

static irqreturn_t xring_irq(int irq, void *dev_id)
{
	XRISP_PR_INFO("into irq, irq_num=%d\n", irq);
	return IRQ_HANDLED;
}

int irq_node_add(int irq_num)
{
	struct irq_register_list *irq_node = NULL;

	irq_node = kzalloc(sizeof(struct irq_register_list), GFP_KERNEL);
	if (irq_node == NULL)
		return -ENOMEM;
	irq_node->irq = irq_num;
	XRISP_PR_INFO("soft_irq[%d] add to list\n", irq_num);
	list_add_tail(&irq_node->list, &g_regdump.irq_sign_list);
	return 0;
}

void irq_node_delete(int irq_num)
{
	struct irq_register_list *entry = NULL;
	struct irq_register_list *temp = NULL;

	list_for_each_entry_safe(entry, temp, &g_regdump.irq_sign_list, list) {
		if (entry->irq == irq_num) {
			XRISP_PR_INFO("soft_irq[%d] delete to list\n", irq_num);
			list_del_init(&entry->list);
			kfree(entry);
		}
	}
}

void irq_sign_free(void)
{
	struct irq_register_list *entry = NULL;
	struct irq_register_list *temp = NULL;

	list_for_each_entry_safe(entry, temp, &g_regdump.irq_sign_list, list) {
		XRISP_PR_INFO("soft_irq[%d] unregister, free\n", entry->irq);
		free_irq(entry->irq, NULL);
		list_del_init(&entry->list);
		kfree(entry);
	}
}

static ssize_t dfx_irq_sign_ops(struct file *file, const char __user *user_buf,
				size_t count, loff_t *ppos)
{
	int ret = 0;
	int sign_mode = 0;
	unsigned long irqflags;
	char name_buf[20];
	uint32_t irq_dts_num = 0;

	if (kstrtouint_from_user(user_buf, count, 10, &sign_mode)) {
		XRISP_PR_ERROR("input sign_mode read error: %d", sign_mode);
		return -EINVAL;
	}

	if (irq > 32 || arr_irq[irq] == 0) {
		XRISP_PR_ERROR("input irq num invalid irq=%d", irq);
		return -EINVAL;
	}

	if (irq <= 31)
		irq_dts_num = irq + R82_AP_IRQ_NUM;
	else if (irq == 32)
		irq_dts_num = WDT_IRQ_NUM;
	else
		return -EINVAL;

	if (sign_mode == 1) { /* irq register */
		memset(name_buf, 0, sizeof(name_buf));
		snprintf(name_buf, sizeof(name_buf), "xring_irq%d", irq);
		irqflags = IRQF_TRIGGER_RISING | IRQF_ONESHOT;
		ret = request_threaded_irq(arr_irq[irq], NULL, xring_irq, irqflags,
					   name_buf, NULL);
		if (ret) {
			XRISP_PR_ERROR("num[%d], soft_irq[%d], dts_irq[%d] register error , ret = %d",
			       irq, arr_irq[irq], irq_dts_num, ret);
			return -EINVAL;
		}
		if (irq_node_add(arr_irq[irq]))
			return -ENOMEM;

		XRISP_PR_INFO("num[%d], soft_irq=%d, irq_dts=%d register", irq, arr_irq[irq],
			irq_dts_num);
	} else if (sign_mode == 2) { /* irq unregister */
		XRISP_PR_INFO("num[%d], soft_irq=%d, irq_dts=%d unregister", irq, arr_irq[irq],
			irq_dts_num);
		free_irq(arr_irq[irq], NULL);
		irq_node_delete(arr_irq[irq]);
	} else {
		XRISP_PR_ERROR("input sign_mode invalid");
		return -EINVAL;
	}

	return count;
}

static const struct file_operations irq_sign_ops = {
	.write = dfx_irq_sign_ops,
};

static ssize_t dfx_be_ocm_set(struct file *file, const char __user *user_buf, size_t count,
			      loff_t *ppos)
{
	ssize_t ret = 0;
	char cmd[10];

	if (!count || count > sizeof(cmd))
		return -EINVAL;

	ret = copy_from_user(cmd, user_buf, count);
	if (ret)
		return -EFAULT;

	if (!strncmp(cmd, "start", count - 1)) {
		ret = xrisp_be_ocm_link();
	} else if (!strncmp(cmd, "stop", count - 1)) {
		ret = xrisp_be_ocm_unlink();
	} else {
		XRISP_PR_INFO("Unrecognized option\n");
		ret = -EINVAL;
	}
	XRISP_PR_INFO("be_ocm %s", cmd);
	return ret < 0 ? ret : count;
}

static const struct file_operations be_ocm_fops = {
	.write = dfx_be_ocm_set,
};

static ssize_t dfx_mcu_rate_set(struct file *file, const char __user *user_buf,
				size_t count, loff_t *ppos)
{
	int ret = 0;
	int rate;

	if (kstrtouint_from_user(user_buf, count, 10, &rate)) {
		XRISP_PR_ERROR("input rate read error: %d", rate);
		return -EINVAL;
	}

	ret = xrisp_mcu_set_rate(rate);

	return ret < 0 ? ret : count;
}

static const struct file_operations mcu_rate_fops = {
	.write = dfx_mcu_rate_set,
};

int xring_dfx_get_irq(void)
{
	struct device_node *np;
	const int irq_len = 33;
	int i;

	np = of_find_node_by_path("/soc/xrisp_cdm");
	if (!np) {
		XRISP_PR_ERROR("xrisp_cdm node not found\n");
		return -EINVAL;
	}

	for (i = 0; i < irq_len; i++) {
		arr_irq[i] = irq_of_parse_and_map(np, i);
		if (!arr_irq[i]) {
			XRISP_PR_INFO("get irq error %d", arr_irq[i]);
			return -EINVAL;
		}
	}

	return 0;
}

int xring_regdump_init(void)
{
	struct dentry *debugfs_file = NULL;

	if (xring_dfx_get_irq()) {
		XRISP_PR_ERROR("get irq error");
		return -EINVAL;
	}

	INIT_LIST_HEAD(&g_regdump.irq_sign_list);

	g_regdump.regdump_debugfs =
		debugfs_create_dir("regdump_dfx", xrisp_debugfs_get_root());
	if (IS_ERR_OR_NULL(g_regdump.regdump_debugfs)) {
		XRISP_PR_INFO("regdump_dfx dir create failed %ld\n",
			PTR_ERR(g_regdump.regdump_debugfs));
		return -EINVAL;
	}
	debugfs_file = debugfs_create_file("dump_ops", 0600, g_regdump.regdump_debugfs,
					   NULL, &dump_regs_ops);
	if (IS_ERR_OR_NULL(debugfs_file)) {
		XRISP_PR_INFO("dump_ops debugfs create failed %ld\n", PTR_ERR(debugfs_file));
		goto debugfs_err;
	}

	debugfs_file = debugfs_create_file("reg_result", 0400, g_regdump.regdump_debugfs,
					   NULL, &reg_wr_ops);
	if (IS_ERR_OR_NULL(debugfs_file)) {
		XRISP_PR_INFO("reg_result debugfs create failed %ld\n", PTR_ERR(debugfs_file));
		goto debugfs_err;
	}

	debugfs_file = debugfs_create_file("irq_sign", 0600, g_regdump.regdump_debugfs,
					   NULL, &irq_sign_ops);
	if (IS_ERR_OR_NULL(debugfs_file)) {
		XRISP_PR_INFO("irq_sign debugfs create failed %ld\n", PTR_ERR(debugfs_file));
		goto debugfs_err;
	}

	debugfs_create_u32("reg_addr", 0600, g_regdump.regdump_debugfs,
			   &g_regdump.reg_address);
	debugfs_create_u32("reg_val", 0600, g_regdump.regdump_debugfs,
			   &g_regdump.reg_value);

	debugfs_create_u32("dump_addr", 0600, g_regdump.regdump_debugfs,
			   &g_regdump.dump_address);
	debugfs_create_u32("dump_len", 0600, g_regdump.regdump_debugfs,
			   &g_regdump.dump_length);

	debugfs_create_u32("irq_num", 0600, g_regdump.regdump_debugfs, &irq);

	/* be_ocm link/unlink */
	debugfs_file = debugfs_create_file("be_ocm", 0600, g_regdump.regdump_debugfs,
					   NULL, &be_ocm_fops);

	debugfs_file = debugfs_create_file("mcu_rate", 0600, g_regdump.regdump_debugfs,
					   NULL, &mcu_rate_fops);

	XRISP_PR_INFO("regdump probe success");
	return 0;

debugfs_err:
	debugfs_remove(g_regdump.regdump_debugfs);
	XRISP_PR_INFO("regdump probe failed");
	return -EINVAL;
}

void xring_regdump_exit(void)
{
	irq_sign_free();
	dump_free_memory();
	debugfs_remove(g_regdump.regdump_debugfs);
}
