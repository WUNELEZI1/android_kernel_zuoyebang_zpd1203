// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "cbm_debugfs", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "cbm_debugfs", __func__, __LINE__

#include <linux/fs.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "xrisp_cbm.h"
#include "xrisp_debug.h"
#include "xrisp_cbm_api.h"
#include "xrisp_log.h"

static bool debug_is_map = true;
static uint32_t region_type;
static uint32_t region_id;

void pr_cam_buf(struct cam_buf_element *cbm_buf, enum cam_buf_action action)
{
	struct cam_buf_handle *hdl;

	if (!cbm_buf)
		return;
	cbm_buf_lock(cbm_buf);
	hdl = (struct cam_buf_handle *)&cbm_buf->buf_handle;
	XRISP_PR_DEBUG("%s buf%d hdl: 0x%016llx region: %d "
		"fd: %d inode %lu buf 0x%pK "
		"is_internal : %s is_imported : %s size: %zu "
		"mapio: %s iova: 0x%08llx ~ 0x%08llx "
		"sfd: %d, iomap_type: %d",
		 BUF_ACTION_STR(action), hdl->idx,
		 cbm_buf->buf_handle,
		 cbm_buf->iomap.region == NULL ? 0 :
		 cbm_buf->iomap.region->region_id,
		 cbm_buf->fd, cbm_buf->i_ino, cbm_buf->buf,
		 cbm_buf->is_internal ? "Y" : "N",
		 cbm_buf->is_imported ? "Y" : "N",
		 cbm_buf->size,
		 cbm_buf->mapio ? "Y" : "N", cbm_buf->iova,
		 (cbm_buf->iova + cbm_buf->size - 1),
		 cbm_buf->sfd, cbm_buf->iomap.map_type);
	cbm_buf_unlock(cbm_buf);
}

int cbt_buf_info_show(struct seq_file *m, void *data)
{

	struct cam_buf_element *cbm_buf;
	struct cam_buf_handle *hdl;

	cbm_buf = m->private;
	if (!cbm_buf) {
		seq_puts(m, "null buf");
		XRISP_PR_ERROR("null buf");
		return -EINVAL;
	}

	cbm_buf_lock(cbm_buf);
	hdl = (struct cam_buf_handle *)&cbm_buf->buf_handle;

	seq_printf(m, "idx:%d handle: 0x%016llx region: %d fd: 0x%08x\n",
		   hdl->idx, cbm_buf->buf_handle, cbm_buf->region_id, hdl->fd);
	seq_printf(m, "fd: %d is_internal : %s is_imported : %s size: %zu\n",
		  cbm_buf->fd, cbm_buf->is_internal ? "Y" : "N",
		  cbm_buf->is_imported ? "Y" : "N", cbm_buf->size);
	seq_printf(m, "mapio: %s iova: %pad mapk: %s kva: 0x%pK\n",
		   cbm_buf->mapio ? "Y" : "N", &cbm_buf->iova,
		   cbm_buf->mapk ? "Y" : "N", (void *)cbm_buf->kva);
	cbm_buf_unlock(cbm_buf);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(cbt_buf_info);

int cbt_bitmap_show(struct seq_file *m, void *data)
{
	mutex_lock(&g_cam_buf_mgr.lock_tbl);
	seq_printf(m, "buf table bit map: %*pbl\n", (int)g_cam_buf_mgr.bits, g_cam_buf_mgr.bitmap);
	mutex_unlock(&g_cam_buf_mgr.lock_tbl);
	return 0;
}
DEFINE_SHOW_ATTRIBUTE(cbt_bitmap);

int iova_region_bitmap_show(struct seq_file *m, void *data)
{
	mutex_lock(&g_cam_buf_mgr.lock_region);
	seq_printf(m, "iova region bit map: %*pbl\n",
		   (int)g_cam_buf_mgr.region_num, g_cam_buf_mgr.bitmap_region);
	mutex_unlock(&g_cam_buf_mgr.lock_region);
	return 0;
}
DEFINE_SHOW_ATTRIBUTE(iova_region_bitmap);


static int iova_region_alloc(void *data, u64 *v)
{
	int ret = 0;

	ret = cbm_iova_region_alloc(region_type);

	if (ret < 0) {
		XRISP_PR_ERROR("alloc iova region fail");
		return -EINVAL;
	}

	*v = ret;

	return 0;
}

static int iova_region_free(void *data, u64 v)
{
	int ret = 0;

	ret = cbm_iova_region_free(v);
	if (ret) {
		XRISP_PR_ERROR("free iova region fail");
		return ret;
	}

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(iova_region_fops, iova_region_alloc, iova_region_free, "%llx\n");

static int cbt_buf_alloc(void *data, u64 v)
{
	int ret = 0;
	struct cam_buf_request req;
	struct cam_buf_desc desc;

	if (v <= 0) {
		XRISP_PR_ERROR("error alloc size");
		return -EINVAL;
	}

	req.size = v;
	req.flags = CAM_BUF_KERNEL_ACCESS |
		    CAM_BUF_USER_READ_WRITE;
	req.buf_region = region_id;
	if (debug_is_map)
		req.flags |= CAM_BUF_HW_READ_WRITE;

	ret = cbm_buf_get(&req, &desc);
	if (ret) {
		XRISP_PR_ERROR("alloc buf fail");
		return ret;
	}

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(cbt_buf_alloc_fops, NULL, cbt_buf_alloc, "%llx\n");


static int cbt_buf_map(void *data, u64 v)
{
	int ret = 0;
	struct cam_buf_map map;
	struct cam_buf_desc desc;

	if (v <= 0) {
		XRISP_PR_ERROR("error alloc size");
		return -EINVAL;
	}

	map.fd = v;
	map.flags = CAM_BUF_KERNEL_ACCESS |
			CAM_BUF_HW_READ_WRITE;
	map.buf_region = region_id;

	ret = cbm_buf_map(&map, &desc);
	if (ret) {
		XRISP_PR_ERROR("map buf fail");
		return ret;
	}

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(cbt_buf_map_fops, NULL, cbt_buf_map, "%llx\n");

static int cbt_buf_free(void *data, u64 v)
{
	struct cam_buf_desc kdesc;

	kdesc.buf_handle = v;
	return cbm_buf_put(&kdesc);
}
DEFINE_DEBUGFS_ATTRIBUTE(cbt_buf_free_fops, NULL, cbt_buf_free, "%llx\n");

static int sec_buf_alloc(void *data, u64 v)
{
	int ret = 0;
	struct cam_buf_request req;
	struct cam_buf_desc desc;
	struct task_struct *task = get_current();

	XRISP_PR_INFO("task id %d", task->tgid);

	if (v <= 0) {
		XRISP_PR_ERROR("error alloc size");
		return -EINVAL;
	}

	req.size = v;
	req.flags =
		SEC_BUF_TYPE_PROTECT | CAM_BUF_HW_READ_WRITE | CAM_BUF_USER_READ_WRITE;
	req.buf_region = 0;

	ret = cbm_buf_get(&req, &desc);
	if (ret) {
		XRISP_PR_ERROR("alloc buf fail");
		return ret;
	}
	XRISP_PR_INFO("sec buf alloc success, hdl=0x%llx, iova=0x%llx", desc.buf_handle,
		desc.iovaddr);
	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(sec_buf_alloc_fops, NULL, sec_buf_alloc, "%llx\n");

static int cbt_buf_open(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	if (!inode->i_private)
		return -EINVAL;
	filp->private_data = inode->i_private;
	return 0;
}

static int cbt_buf_release(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	filp->private_data = NULL;
	return 0;
}

static ssize_t cbt_buf_read(struct file *file, char __user *buf,
			size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	ssize_t ret = 0;
	struct cam_buf_element *cbm_buf = file->private_data;
	struct cam_buf_cache cache;

	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;

	cbm_buf_lock(cbm_buf);
	if (!cbm_buf->mapk) {
		cbm_buf_unlock(cbm_buf);
		debugfs_file_put(dentry);
		return -EFAULT;
	}
	cache.buf_handle = cbm_buf->buf_handle;
	cbm_buf_unlock(cbm_buf);

	cache.ops = CAM_BUF_CPU_ACCESS_BEGIN;
	cbm_dma_buf_cache_ops(&cache);

	cbm_buf_lock(cbm_buf);
	ret = simple_read_from_buffer(buf, len, ppos,
		(void *)cbm_buf->kva, cbm_buf->size);
	cbm_buf_unlock(cbm_buf);

	cache.ops = CAM_BUF_CPU_ACCESS_END;
	cbm_dma_buf_cache_ops(&cache);

	debugfs_file_put(dentry);
	return ret;
}

static ssize_t cbt_buf_write(struct file *file, const char __user *buf,
			 size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	ssize_t ret = 0;
	struct cam_buf_element *cbm_buf = file->private_data;
	struct cam_buf_cache cache;

	cbm_buf_lock(cbm_buf);
	if (*ppos >= cbm_buf->size) {
		cbm_buf_unlock(cbm_buf);
		return -ENOSPC;
	}
	ret = debugfs_file_get(dentry);
	if (unlikely(ret)) {
		cbm_buf_unlock(cbm_buf);
		return ret;
	}
	if (!cbm_buf->mapk) {
		cbm_buf_unlock(cbm_buf);
		debugfs_file_put(dentry);
		return -EFAULT;
	}
	cache.buf_handle = cbm_buf->buf_handle;
	cbm_buf_unlock(cbm_buf);

	cache.ops = CAM_BUF_CPU_ACCESS_BEGIN;
	cbm_dma_buf_cache_ops(&cache);

	cbm_buf_lock(cbm_buf);
	ret = simple_write_to_buffer((void *)cbm_buf->kva,  cbm_buf->size,
					ppos, buf, len);
	cbm_buf_unlock(cbm_buf);

	cache.ops = CAM_BUF_CPU_ACCESS_END;
	cbm_dma_buf_cache_ops(&cache);

	debugfs_file_put(dentry);
	return ret;
}

loff_t cbt_buf_llseek(struct file *file, loff_t offset, int whence)
{
	XRISP_PR_INFO("");
	return -ESPIPE;
}

__maybe_unused
static const struct file_operations cbt_buf_rw_fops = {
	.owner	 = THIS_MODULE,
	.open	 = cbt_buf_open,
	.release = cbt_buf_release,
	.read	 = cbt_buf_read,
	.write	 = cbt_buf_write,
	.llseek	 = cbt_buf_llseek,
};

#ifdef CAM_BUF_DEBUG
int cbt_debugfs_install(struct dentry *entry, struct cam_buf_element *cbm_buf)
{
	int ret = 0;
	char name[64];
	struct cam_buf_handle *hdl;
	bool mapk;

	if (!entry || !cbm_buf)
		return -EINVAL;

	hdl = (struct cam_buf_handle *)&cbm_buf->buf_handle;
	ret = snprintf(name, 64, "buf%d.0x%016llx.0x%08lx",
			hdl->idx, cbm_buf->buf_handle,
			(unsigned long)hdl->fd);
	// XRISP_PR_DEBUG("install debugfs to create %s", name);
	if (ret >= 64) {
		XRISP_PR_ERROR("too long buf name");
		return ret;
	}
	cbm_buf->entry = debugfs_create_dir(name, entry);
	if (IS_ERR_OR_NULL(cbm_buf->entry)) {
		XRISP_PR_ERROR("debugfs creat %s dir fail", name);
		cbm_buf->entry = NULL;
		return -EINVAL;
	}
	debugfs_create_file("info", 0200, cbm_buf->entry, cbm_buf, &cbt_buf_info_fops);
	cbm_buf_lock(cbm_buf);
	mapk = cbm_buf->mapk;
	cbm_buf_unlock(cbm_buf);
	if (mapk)
		debugfs_create_file("data", 0600, cbm_buf->entry, cbm_buf, &cbt_buf_rw_fops);
	return 0;
}

void cbt_debugfs_uninstall(struct cam_buf_element *cbm_buf)
{

	if (!cbm_buf)
		return;
	// XRISP_PR_DEBUG("uninstall debugfs");
	debugfs_remove(cbm_buf->entry);
}

int cbm_debugfs_init(void)
{
	struct dentry *root;
	int ret = 0;

	root = xrisp_debugfs_get_root();

	g_cam_buf_mgr.debugfs = debugfs_create_dir("cbm", root);
	if (IS_ERR_OR_NULL(g_cam_buf_mgr.debugfs)) {
		ret = PTR_ERR(g_cam_buf_mgr.debugfs);
		XRISP_PR_ERROR("debugfs creat dir fail ret = %d", ret);
		g_cam_buf_mgr.debugfs = NULL;
		return ret;
	}
	root = g_cam_buf_mgr.debugfs;
	debugfs_create_bool("is_map", 0600, root, &debug_is_map);
	debugfs_create_file("alloc", 0444, root, &g_cam_buf_mgr, &cbt_buf_alloc_fops);
	debugfs_create_file("sec_alloc", 0444, root, &g_cam_buf_mgr, &sec_buf_alloc_fops);
	debugfs_create_file("map", 0444, root, &g_cam_buf_mgr, &cbt_buf_map_fops);
	debugfs_create_file("free", 0444, root, &g_cam_buf_mgr, &cbt_buf_free_fops);
	debugfs_create_file("buf_bitmap", 0444, root, NULL, &cbt_bitmap_fops);
	debugfs_create_file("region_bitmap", 0444, root, NULL, &iova_region_bitmap_fops);
	debugfs_create_file("region", 0600, root, NULL, &iova_region_fops);
	debugfs_create_u32("region_type", 0600, root, &region_type);
	debugfs_create_u32("cur_region", 0600, root, &region_id);
	g_cam_buf_mgr.cbt_entry = debugfs_create_dir("dma-buf", root);
	if (IS_ERR_OR_NULL(g_cam_buf_mgr.cbt_entry)) {
		ret = PTR_ERR(g_cam_buf_mgr.cbt_entry);
		XRISP_PR_ERROR("debugfs creat cbt dir fail ret = %d", ret);
		g_cam_buf_mgr.cbt_entry = NULL;
		return ret;
	}
	return 0;
}

void cbm_debugfs_exit(void)
{
	debugfs_remove(g_cam_buf_mgr.debugfs);
}

#else /* CAM_BUF_DEBUG */


int cbm_debugfs_init(void)
{
	return 0;
}
void cbm_debugfs_exit(void){}

int cbt_debugfs_install(struct dentry *entry, struct cam_buf_element *cbm_buf)
{
	return 0;
}
void cbt_debugfs_uninstall(struct cam_buf_element *cbm_buf) {}

#endif /* CAM_BUF_DEBUG */
