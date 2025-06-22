// SPDX-License-Identifier: GPL-2.0
/*
 * SMMU pagetable dump.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#include <linux/dma-map-ops.h>
#include <linux/device/bus.h>
#include <linux/debugfs.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/genalloc.h>
#include <linux/iova.h>
#include <linux/io-pgtable.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/rwlock.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <asm/cacheflush.h>
#include <asm-generic/errno-base.h>

#include "xring_smmu.h"
#include "xring_smmu_dump_pgtable.h"
#include "xring_smmu_debugfs.h"
#include "xring_iommu_iova_cookie.h"

static struct dentry *g_smmu_debug_dir[SMMU_INFO_MAX];
static struct xring_smmu_debugfs_info g_smmu_info[SMMU_INFO_MAX];
static uint g_smmu_info_num;
bool dumpmapunmap;

static int xring_smmu_parse_input_find_smmudev_index(char *cmd, uint64_t *smmu_input_info,
		int *para_num)
{
	int j = 0, ret;
	char *token, *cur = cmd;
	char *delim = " ";

	token = strsep(&cur, delim);
	xring_smmu_debug("token:%s\n", token);
	for (j = 0; j < g_smmu_info_num; j++) {
		if (strstr(g_smmu_info[j].name, token)) {
			xring_smmu_err("find input_name:%s, smmu_info[%d].name:%s",
				token, j, g_smmu_info[j].name);
			break;
		}
		if ((!strstr(g_smmu_info[j].name, token)) && (j == (g_smmu_info_num - 1))) {
			xring_smmu_err("Err: smmu debugfs node for %s subsystem not found\n",
				token);
			for (j = 0; j < g_smmu_info_num; j++)
				xring_smmu_err("smmu_info[%d].name:%s", j, g_smmu_info[j].name);
			return -ENAVAIL;
		}
		xring_smmu_debug("input_name:%s, smmu_info[%d].name:%s", token, j, g_smmu_info[j].name);
	}

	token = strsep(&cur, delim);
	while (token != NULL) {
		ret = kstrtoull(token, 0, &smmu_input_info[(*para_num)++]);
		if (ret) {
			xring_smmu_err("Failed to convert string to number!\n");
			return -ENAVAIL;
		}
		delim = " ";
		token = strsep(&cur, delim);
		xring_smmu_debug("**************token:%s\n", token);
		xring_smmu_debug("**************smmu_input_info[%d]:0x%llx\n",
			(*para_num) - 1, smmu_input_info[(*para_num) - 1]);
	}

	xring_smmu_debug("**************ok\n");
	return j;
}

static ssize_t xring_smmu_strtab_debug_write(struct file *filp,
		const char __user *buf, size_t len, loff_t *off)
{
	int info_index = 0, smmu_index = 0;
	char cmd[SMMU_NAME_LEN];
	uint64_t smmu_input_info[SMMU_INFO_MAX];

	if (!len || len > sizeof(cmd))
		return -EINVAL;

	if (copy_from_user(cmd, buf, len))
		return -EFAULT;

	xring_smmu_debug("%s, len:0x%lx\n", cmd, len);

	smmu_index = xring_smmu_parse_input_find_smmudev_index(cmd,
			smmu_input_info, &info_index);
	if (smmu_index < 0)
		return -EINVAL;

	xring_smmu_debug("smmu_index:%d, info_index:%d\n", smmu_index, info_index);

	switch (info_index) {
	case DUMP_STE:
		xring_smmu_dump_ste(g_smmu_info[smmu_index].smmu, smmu_input_info[0]);
		break;
	case DUMP_CD:
		xring_smmu_dump_cd(g_smmu_info[smmu_index].smmu, smmu_input_info[0],
			smmu_input_info[1]);
		break;
	case DUMP_STRTAB:
		xring_smmu_dump_va_range(g_smmu_info[smmu_index].smmu, smmu_input_info[0],
			smmu_input_info[1], smmu_input_info[2], smmu_input_info[3]);
		break;
	default:
		xring_smmu_err("Input parameter error:\n"
			"dump strtab: sid ssid va entries\ndump iova:sid\n");
		return -EFAULT;
	}

	return len;
}

static ssize_t xring_smmu_strtab_debug_read(struct file *filp,
		char __user *buf, size_t len, loff_t *off)
{
	return len;
}

static const struct file_operations xring_smmu_strtab_debug_fops = {
	.read = xring_smmu_strtab_debug_read,
	.write = xring_smmu_strtab_debug_write,
};

static ssize_t xring_smmu_iova_debug_read(struct file *filp,
		char __user *buf, size_t len, loff_t *off)
{
	return len;
}

static ssize_t xring_smmu_iova_debug_write(struct file *filp,
		const char __user *buf, size_t len, loff_t *off)
{
	struct iommu_domain *domain = NULL;
	int info_index = 0, smmu_index = 0;
	char cmd[SMMU_NAME_LEN];
	uint64_t smmu_input_info[SMMU_INFO_MAX];

	if (!len || len > sizeof(cmd))
		return -EINVAL;

	if (copy_from_user(cmd, buf, len))
		return -EFAULT;

	smmu_index = xring_smmu_parse_input_find_smmudev_index(cmd,
				smmu_input_info, &info_index);
	if (smmu_index < 0)
		return len;

	xring_smmu_debug("smmu_index:%d, info_index:%d\n", smmu_index, info_index);

	domain = arm_smmu_get_iommu_domain_by_sid(g_smmu_info[smmu_index].smmu,
				smmu_input_info[0]);

	if (!domain)
		return -ENAVAIL;

	xring_smmu_iova_debug(domain);
	return len;
}

static const struct file_operations xring_smmu_iova_debug_fops = {
	.read = xring_smmu_iova_debug_read,
	.write = xring_smmu_iova_debug_write,
};

int xring_smmu_debugfs_init(struct arm_smmu_device *smmu)
{
	struct dentry *root;
	int ret = 0;
	char name[SMMU_NAME_LEN];

	dumpmapunmap = false;
	snprintf(name, sizeof(name),
			"xring_smmu_debug_%s", dev_name(smmu->dev));
	root = debugfs_create_dir(name, NULL);
	if (IS_ERR(root))
		return PTR_ERR(root);

	snprintf(name, sizeof(name), "%s", dev_name(smmu->dev));
	xring_smmu_debug("smmu:%p, dev_name:%s\n", smmu, dev_name(smmu->dev));

	g_smmu_debug_dir[g_smmu_info_num] = root;
	debugfs_create_bool("dumpmapunmap", 0600, root, &dumpmapunmap);
	debugfs_create_file("strtab", 0644, root, NULL, &xring_smmu_strtab_debug_fops);
	debugfs_create_file("iova", 0644, root, NULL, &xring_smmu_iova_debug_fops);

	g_smmu_info[g_smmu_info_num].smmu = smmu;
	memcpy(g_smmu_info[g_smmu_info_num].name, name, sizeof(name));
	xring_smmu_debug("g_smmu_info[%d].smmu:%p, g_smmu_info[%d].name:%s\n",
		g_smmu_info_num, g_smmu_info[g_smmu_info_num].smmu,
		g_smmu_info_num, g_smmu_info[g_smmu_info_num].name);

	g_smmu_info_num++;
	return ret;
}
EXPORT_SYMBOL(xring_smmu_debugfs_init);

void xring_smmu_debugfs_exit(void)
{
	int i;

	for (i = 0; i < g_smmu_info_num; i++)
		debugfs_remove(g_smmu_debug_dir[i]);

	memset(g_smmu_info, 0, sizeof(struct xring_smmu_debugfs_info) * SMMU_INFO_MAX);
	g_smmu_info_num = 0;
}
EXPORT_SYMBOL(xring_smmu_debugfs_exit);
