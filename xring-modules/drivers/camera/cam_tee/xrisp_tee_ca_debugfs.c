// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "tee_debugfs", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "tee_debugfs", __func__, __LINE__

#include <linux/fs.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include "xrisp_debug.h"
#include "xrisp_rproc_api.h"
#include "xrisp_tee_ca.h"
#include "xrisp_tee_ca_api.h"
#include "xrisp_log.h"
#ifdef XRISP_AOC_DEBUG
#include <dt-bindings/xring/platform-specific/aoc_camera_common.h>
#endif

#ifdef XRISP_CA_DEBUG

#ifdef XRISP_AOC_DEBUG
static int xrisp_ca_dump_buffer_open(struct inode *inode, struct file *filp)
{
	if (!inode->i_private)
		return -EINVAL;
	filp->private_data = inode->i_private;

	return 0;
}

static int xrisp_ca_dump_buffer_relase(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static ssize_t xrisp_ca_dump_buffer_write(struct file *filp, const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	ssize_t ret;
	struct xrisp_ca_private *xrisp_ca = filp->private_data;
	char cmd[16];
	struct xrisp_ca_rsv_mem  *meminfo;
	struct xrisp_ca_buf_dentry *buf_dentry = NULL;
	struct debugfs_blob_wrapper *blob_buf = NULL;
	char *va = NULL;
	phys_addr_t pa;
	int len = 0;
	char name[32] = {0};
	char *memptr = NULL;

	if (!xrisp_ca)
		return -EINVAL;

	ret = copy_from_user(cmd, user_buf, count);
	if (ret)
		return ret;

	meminfo = &xrisp_ca->meminfo;
	meminfo->baseaddr = ioremap(meminfo->mem_phys, meminfo->size);
	if (IS_ERR(meminfo->baseaddr)) {
		ret = PTR_ERR(meminfo->baseaddr);
		XRISP_PR_ERROR("ioremap failure, ret=%zd\n", ret);
		meminfo->baseaddr = NULL;
		return ret;
	}
	XRISP_PR_INFO("aoc mem:0x%lx, size:0x%lx baseaddr:0x%lx.",
		(unsigned long)meminfo->mem_phys,
		meminfo->size, (unsigned long)meminfo->baseaddr);
	if (!strncmp(cmd, "header", count - 1)) {
		va = (char *)meminfo->baseaddr + CSIWDMA_HEADER_BASEADDR;
		pa = meminfo->mem_phys + CSIWDMA_HEADER_BASEADDR;
		len = CSIWDMA_HEADER_SIZE; // header: 8K
		buf_dentry = &meminfo->blob_tab[0];
		snprintf(name, sizeof(name), "%s", "header.bin");
		XRISP_PR_INFO("Dump header va:0x%lx pa:0x%lx size:0x%x.",
			(unsigned long)va, (unsigned long)pa, len);
	} else if (!strncmp(cmd, "raw0", count - 1)) {
		va = (char *)meminfo->baseaddr + PINGPONG_RAW_BUFFER0_ADDR;
		pa = meminfo->mem_phys + PINGPONG_RAW_BUFFER0_ADDR;
		len = PINGPONG_RAW_BUFFER_SIZE;
		buf_dentry = &meminfo->blob_tab[1];
		snprintf(name, sizeof(name), "%s", "raw_buffer0.bin");
		XRISP_PR_INFO("Dump RAW buf0 va:0x%lx pa:0x%lx size:0x%x.",
			(unsigned long)va, (unsigned long)pa, len);
	} else if (!strncmp(cmd, "raw1", count - 1)) {
		va = (char *)meminfo->baseaddr + PINGPONG_RAW_BUFFER1_ADDR;
		pa = meminfo->mem_phys + PINGPONG_RAW_BUFFER1_ADDR;
		len = PINGPONG_RAW_BUFFER_SIZE;
		buf_dentry = &meminfo->blob_tab[2];
		snprintf(name, sizeof(name), "%s", "raw_buffer1.bin");
		XRISP_PR_INFO("Dump RAW buf1 va:0x%lx pa:0x%lx size:0x%x.",
			(unsigned long)va, (unsigned long)pa, len);
	} else if (!strncmp(cmd, "y0", count - 1)) {
		va = (char *)meminfo->baseaddr + PINGPONG_Y_BUFFER0_ADDR;
		pa = meminfo->mem_phys + PINGPONG_Y_BUFFER0_ADDR;
		len = PINGPONG_Y_BUFFER_SIZE;
		buf_dentry = &meminfo->blob_tab[3];
		snprintf(name, sizeof(name), "%s", "y_buffer0.bin");
		XRISP_PR_INFO("Dump Y buf0 va:0x%lx pa:0x%lx size:0x%x.",
			(unsigned long)va, (unsigned long)pa, len);
	} else if (!strncmp(cmd, "y1", count - 1)) {
		va = (char *)meminfo->baseaddr + PINGPONG_Y_BUFFER1_ADDR;
		pa = meminfo->mem_phys + PINGPONG_Y_BUFFER1_ADDR;
		len = PINGPONG_Y_BUFFER_SIZE;
		buf_dentry = &meminfo->blob_tab[4];
		snprintf(name, sizeof(name), "%s", "y_buffer1.bin");
		XRISP_PR_INFO("Dump Y buf1 va:0x%lx pa:0x%lx size:0x%x.",
			(unsigned long)va, (unsigned long)pa, len);
	} else {
		XRISP_PR_ERROR("Unsupport dump cmd:%s.", cmd);
		goto mem_iounmap;
	}

	if (buf_dentry == NULL) {
		XRISP_PR_ERROR("get buf_dentry for %s failure.", cmd);
		goto mem_iounmap;
	}
	if (buf_dentry->blob_dentry != NULL) {
		XRISP_PR_ERROR("buf_dentry->blob_dentry:0x%lx.",
			(unsigned long)buf_dentry->blob_dentry);
		debugfs_remove(buf_dentry->blob_dentry);
		buf_dentry->blob_dentry = NULL;
	}

	memptr = kzalloc(len, GFP_KERNEL);
	if (memptr == NULL) {
		XRISP_PR_ERROR("alloc memory failed.");
		goto mem_iounmap;
	}

	memcpy_fromio(memptr, (void __iomem *)va, len);

	blob_buf = &buf_dentry->blob_buf;
	blob_buf->data = memptr;
	blob_buf->size = len;
	buf_dentry->blob_dentry = debugfs_create_blob(name, 0400,
					xrisp_ca->dentry, blob_buf);
	if (IS_ERR_OR_NULL(buf_dentry->blob_dentry)) {
		XRISP_PR_ERROR("Dump file:%s failure.", name);
		buf_dentry->blob_dentry = NULL;
		goto free_mem;
	}
	XRISP_PR_INFO("Dump file:%s success.", name);

free_mem:
	kfree(memptr);

mem_iounmap:
	if (meminfo->baseaddr) {
		iounmap(meminfo->baseaddr);
		meminfo->baseaddr = NULL;
	}

	return count;
}

static const struct file_operations xrisp_ca_dump_buffer_fops = {
	.owner = THIS_MODULE,
	.open = xrisp_ca_dump_buffer_open,
	.release = xrisp_ca_dump_buffer_relase,
	.write = xrisp_ca_dump_buffer_write,
	.llseek = generic_file_llseek,
};

static int xrisp_ca_dump_buffer_init(struct xrisp_ca_private *xrisp_ca)
{
	struct device_node *np = NULL;
	struct reserved_mem *rsv_mem = NULL;
	struct xrisp_ca_rsv_mem  *meminfo;

	if (!xrisp_ca)
		return -EINVAL;

	meminfo = &xrisp_ca->meminfo;
	np = of_find_node_by_name(NULL, "rsv_mem_isp_aoc");
	if (!np) {
		XRISP_PR_ERROR("no rsv_mem_isp_aoc found.");
		return -EINVAL;
	}

	rsv_mem = of_reserved_mem_lookup(np);
	if (!rsv_mem) {
		XRISP_PR_ERROR("find reserved memory for %s failure.", np->name);
		return -ENOMEM;
	}

	meminfo->mem_phys = rsv_mem->base;
	meminfo->size     = rsv_mem->size;
	debugfs_create_file("buf_dump", 0600, xrisp_ca->dentry,
		xrisp_ca, &xrisp_ca_dump_buffer_fops);

	return 0;
}
#endif

static int xrisp_ca_debug_switch_mode(uint32_t item, uint32_t mode)
{
	int ret = 0;

	if (xrisp_ca_context_build())
		return -EINVAL;

	if (mode)
		ret = xrisp_ca_switch_mode(item, TA_SECMODE_SAFE);
	else
		ret = xrisp_ca_switch_mode(item, TA_SECMODE_UNSAFE);

	xrisp_ca_context_unbuild();
	if (ret)
		return ret;

	return ret;
}

static int xrisp_ca_csiwdma_mode_get(void *data, u64 *val)
{
	struct xrisp_ca_private *xrisp_ca = data;

	*val = !!(xrisp_ca->mode & TA_SECMODE_CSIWDMA);

	return 0;
}

static int xrisp_ca_csiwdma_mode_set(void *data, u64 val)
{
	int ret;

	XRISP_PR_INFO("set csiwdma to %s start.", ((!!val) ? "sec" : "non-sec"));

	ret = xrisp_ca_debug_switch_mode(TA_SECMODE_CSIWDMA, !!val);
	if (ret) {
		XRISP_PR_ERROR("csiwdma switch to %s failure.", ((!!val) ? "sec" : "non-sec"));
		return ret;
	}

	XRISP_PR_INFO("set csiwdma to %s success.", ((!!val) ? "sec" : "non-sec"));
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(xrisp_ca_csiwdma_mode_fops, xrisp_ca_csiwdma_mode_get,
		xrisp_ca_csiwdma_mode_set, "%llu\n");

static int xrisp_ca_csi_mode_get(void *data, u64 *val)
{
	struct xrisp_ca_private *xrisp_ca = data;

	*val = !!(xrisp_ca->mode & TA_SECMODE_CSI);

	return 0;
}

static int xrisp_ca_csi_mode_set(void *data, u64 val)
{
	int ret;

	XRISP_PR_INFO("set csi to %s start.", ((!!val) ? "sec" : "non-sec"));

	ret = xrisp_ca_debug_switch_mode(TA_SECMODE_CSI, !!val);
	if (ret) {
		XRISP_PR_ERROR("csi switch to %s failure.", ((!!val) ? "sec" : "non-sec"));
		return ret;
	}

	XRISP_PR_INFO("set csi to %s success.", ((!!val) ? "sec" : "non-sec"));
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(xrisp_ca_csi_mode_fops, xrisp_ca_csi_mode_get,
		xrisp_ca_csi_mode_set, "%llu\n");

static int xrisp_ca_isp_mode_get(void *data, u64 *val)
{
	struct xrisp_ca_private *xrisp_ca = data;

	*val = !!(xrisp_ca->mode & TA_SECMODE_ISP);

	return 0;
}

static int xrisp_ca_isp_mode_set(void *data, u64 val)
{
	int ret;

	XRISP_PR_INFO("set isp to %s start.", ((!!val) ? "sec" : "non-sec"));

	ret = xrisp_ca_debug_switch_mode(TA_SECMODE_ISP, !!val);
	if (ret) {
		XRISP_PR_ERROR("isp switch to %s failure.", ((!!val) ? "sec" : "non-sec"));
		return ret;
	}

	XRISP_PR_INFO("set isp to %s success.", ((!!val) ? "sec" : "non-sec"));
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(xrisp_ca_isp_mode_fops, xrisp_ca_isp_mode_get,
		xrisp_ca_isp_mode_set, "%llu\n");

int xrisp_ca_debugfs_init(struct xrisp_ca_private *xrisp_ca)
{
	struct dentry *root;
	int ret = 0;

	if (!xrisp_ca)
		return -EINVAL;
	root = xrisp_debugfs_get_root();

	xrisp_ca->dentry = debugfs_create_dir("cam_ca", root);
	if (IS_ERR_OR_NULL(xrisp_ca->dentry))
		return -EINVAL;

	debugfs_create_file("isp_mode", 0600, xrisp_ca->dentry, xrisp_ca, &xrisp_ca_isp_mode_fops);
	debugfs_create_file("csi_mode", 0600, xrisp_ca->dentry, xrisp_ca, &xrisp_ca_csi_mode_fops);
	debugfs_create_file("csiwdma_mode", 0600, xrisp_ca->dentry, xrisp_ca,
		&xrisp_ca_csiwdma_mode_fops);

#ifdef XRISP_AOC_DEBUG
	ret = xrisp_ca_dump_buffer_init(xrisp_ca);
	if (ret) {
		XRISP_PR_ERROR("create debugfs for buf_dump failed, ret:%d", ret);
		return ret;
	}
#endif

	XRISP_PR_INFO("create debugfs for ca.");
	return ret;
}

void xrisp_ca_debugfs_exit(struct xrisp_ca_private *xrisp_ca)
{
	debugfs_remove(xrisp_ca->dentry);
}

#else /* XRISP_CA_DEBUG */

int xrisp_ca_debugfs_init(struct xrisp_ca_private *xrisp_ca)
{
	return 0;
}

void xrisp_ca_debugfs_exit(struct xrisp_ca_private *xrisp_ca)
{}

#endif /* XRISP_CA_DEBUG */
