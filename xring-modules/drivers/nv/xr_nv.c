// SPDX-License-Identifier: GPL-2.0
/*
 * nv driver for Xring
 *
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 *
 */
#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/crc32.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/debugfs.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/version.h>

#include "xr_nv.h"

static struct semaphore g_nv_sem;
static struct nve_struct *g_nve;
static bool g_nve_inited;
struct dentry *nve_debugfs_root;
static enum nv_item_id item_id = 1;
static u8 recover_mask;

static u32 calculate_crc32_nve_region_header(u8 *buf)
{
	return crc32(~0, buf, NVE_HEADER_NAME_LEN + sizeof(u32)) ^ ~0U;
}

static u32 calculate_crc32_nv_item(struct nv_item *buf)
{
	u32 len = buf->nv_header.valid_size + NV_ITEM_HEADER_SIZE - sizeof(u32);

	return crc32(~0, (u8 *)buf + sizeof(u32), len) ^ ~0U;
}

static int bio_request(struct block_device *bdev,
			sector_t sector,
			struct page *page,
			u32 page_order,
			u32 op,
			u32 op_flags)
{
	int ret = 0;
	struct bio *bio = NULL;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	bio = bio_alloc(GFP_KERNEL, 1 << page_order);
#else
	bio = bio_alloc(bdev, 1 << page_order, op | op_flags, GFP_KERNEL);
#endif
	if (!bio) {
		NV_LOG_ERR("Failed to allocate bio\n");
		ret = -ENOMEM;
		goto out;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	bio_set_dev(bio, bdev);
	bio_set_op_attrs(bio, op, op_flags);
#endif

	bio->bi_iter.bi_sector = sector;

	ret = bio_add_page(bio, page, PAGE_SIZE << page_order, 0);
	if (ret != PAGE_SIZE << page_order) {
		NV_LOG_ERR("Failed to bio_add_page\n");
		ret = -EIO;
		goto out;
	}

	ret = submit_bio_wait(bio);
	if (ret)
		NV_LOG_ERR("Failed to submit_bio_wait\n");

out:
	bio_put(bio);

	return ret;
}

static int bio_read(struct block_device *bdev,
		    loff_t offset,
		    size_t len,
		    u8 *buf)
{
	struct page *page = NULL;
	u32 blkbits = blksize_bits(bdev_logical_block_size(bdev));
	u32 sectors_per_block = 1 << (blkbits - SECTOR_SHIFT);
	sector_t sector_addr = (offset >> blkbits) * sectors_per_block;
	u32 offset_in_page = offset & ((1 << blkbits) - 1);
	u32 num_pages = (len + PAGE_SIZE - 1) / PAGE_SIZE;
	u32 page_order = get_order(num_pages * PAGE_SIZE);
	int ret;

	page = alloc_pages(GFP_KERNEL, page_order);
	if (!page) {
		NV_LOG_ERR("Failed to allocate page\n");
		ret = -ENOMEM;
		goto out;
	}

	ret = bio_request(bdev, sector_addr, page, page_order,
				REQ_OP_READ, 0);
	if (ret) {
		NV_LOG_ERR("bio_request failed, ret:%d\n", ret);
		goto out;
	}

	memcpy(buf, page_address(page) + offset_in_page, len);

out:
	__free_pages(page, page_order);

	return ret;
}

static int bio_write(struct block_device *bdev,
		     loff_t offset,
		     size_t len,
		     u8 *buf)
{
	struct page *page = NULL;
	u32 blkbits = blksize_bits(bdev_logical_block_size(bdev));
	u32 sectors_per_block = 1 << (blkbits - SECTOR_SHIFT);
	sector_t sector_addr = (offset >> blkbits) * sectors_per_block;
	u32 offset_in_page = offset & ((1 << blkbits) - 1);
	u32 num_pages = (len + PAGE_SIZE - 1) / PAGE_SIZE;
	u32 page_order = get_order(num_pages * PAGE_SIZE);
	int ret;

	page = alloc_pages(GFP_KERNEL, page_order);
	if (!page) {
		NV_LOG_ERR("Failed to allocate page\n");
		ret = -ENOMEM;
		goto out;
	}

	ret = bio_request(bdev, sector_addr, page, page_order,
				REQ_OP_READ, 0);
	if (ret) {
		NV_LOG_ERR("bio_request failed, ret:%d\n", ret);
		goto out;
	}

	memcpy(page_address(page) + offset_in_page, buf, len);

	ret = bio_request(bdev, sector_addr, page, page_order,
				REQ_OP_WRITE, REQ_SYNC | REQ_FUA);
	if (ret)
		NV_LOG_ERR("bio_request failed, ret:%d\n", ret);

out:
	__free_pages(page, page_order);

	return ret;
}

static int nve_read(loff_t offset, size_t len, u8 *buf)
{
	struct block_device *bdev = NULL;
	int ret;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	bdev = blkdev_get_by_path(NVE_PARTITION_NAME, FMODE_READ, NULL);
#else
	bdev = blkdev_get_by_path(NVE_PARTITION_NAME, FMODE_READ, NULL, NULL);
#endif
	if (IS_ERR(bdev)) {
		NV_LOG_ERR("open nvemem partition failed, ret:%ld\n", PTR_ERR(bdev));
		return -ENOMEM;
	}

	ret = bio_read(bdev, offset, len, buf);
	if (ret)
		NV_LOG_ERR("Failed to bio_read\n");

	return ret;
}

static int nve_write(loff_t offset, size_t len, u8 *buf)
{
	struct block_device *bdev = NULL;
	int ret;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	bdev = blkdev_get_by_path(NVE_PARTITION_NAME, FMODE_WRITE | FMODE_READ, NULL);
#else
	bdev = blkdev_get_by_path(NVE_PARTITION_NAME, FMODE_WRITE | FMODE_READ, NULL, NULL);
#endif
	if (IS_ERR(bdev)) {
		NV_LOG_ERR("open nvemem partition failed, ret:%ld\n", PTR_ERR(bdev));
		return -ENOMEM;
	}

	ret = bio_write(bdev, offset, len, buf);
	if (ret)
		NV_LOG_ERR("Failed to bio_write\n");

	return ret;
}

static void nv_read_item_from_ram(enum nv_item_id item_id, struct nv_item *buf)
{
	u8 *nv_data = g_nve->nve_region_ram + item_id * NV_ITEM_SIZE;

	memcpy(buf, nv_data, NV_ITEM_SIZE);
}

static void nv_write_item_to_ram(enum nv_item_id item_id, struct nv_item *buf)
{
	u8 *nv_data = g_nve->nve_region_ram + item_id * NV_ITEM_SIZE;

	memcpy(nv_data, buf, NV_ITEM_SIZE);
}

static int nv_read_item_from_region(u8 region_id, enum nv_item_id item_id, struct nv_item *buf)
{
	int ret;
	loff_t offset = region_id * NVE_REGION_SIZE + item_id * NV_ITEM_SIZE;

	ret = nve_read(offset, NV_ITEM_SIZE, (u8 *)buf);
	if (ret)
		NV_LOG_ERR("nve_read failed, ret:%d\n", ret);

	return ret;
}

static int nv_write_item_to_region(u8 region_id, enum nv_item_id item_id, struct nv_item *buf)
{
	int ret;
	loff_t offset = region_id * NVE_REGION_SIZE + item_id * NV_ITEM_SIZE;

	ret = nve_write(offset, NV_ITEM_SIZE, (u8 *)buf);
	if (ret)
		NV_LOG_ERR("nve_write failed, ret:%d\n", ret);

	return ret;
}

static int nv_read_backup_item(enum nv_item_id item_id, struct nv_item *buf)
{
	int ret;
	struct nv_item_header *nv_header = &buf->nv_header;

	ret = nv_read_item_from_region(NVE_BACKUP_REGION_ID, item_id, buf);
	if (ret) {
		NV_LOG_ERR("nve read item from region failed, ret:%d\n", ret);
		return ret;
	}

	if (nv_header->crc32 == calculate_crc32_nv_item(buf)) {
		NV_LOG_ERR("backup region nv item check success, item_id:%d\n", item_id);
		ret = nv_write_item_to_region(NVE_PRIMARY_REGION_ID, item_id, buf);
		if (ret)
			NV_LOG_ERR("nve write item to region failed, ret:%d\n", ret);

		return ret;
	}

	NV_LOG_ERR("backup region nv item check fail, item_id:%d\n", item_id);

	ret = nv_read_item_from_region(NVE_ORIGIN_REGION_ID, item_id, buf);
	if (ret) {
		NV_LOG_ERR("nve read item from region failed, ret:%d\n", ret);
		return ret;
	}

	if (nv_header->crc32 == calculate_crc32_nv_item(buf)) {
		NV_LOG_ERR("origin region nv item check success, item_id:%d\n", item_id);
		ret = nv_write_item_to_region(NVE_PRIMARY_REGION_ID, item_id, buf);
		if (ret) {
			NV_LOG_ERR("nve write item to region failed, ret:%d\n", ret);
			return ret;
		}

		ret = nv_write_item_to_region(NVE_BACKUP_REGION_ID, item_id, buf);
		if (ret) {
			NV_LOG_ERR("nve write item to region failed, ret:%d\n", ret);
			return ret;
		}

		return ret;
	}

	NV_LOG_ERR("origin region nv item check fail, item_id:%d\n", item_id);
	return -EBADMSG;
}

static int nv_read_item_from_device(enum nv_item_id item_id, struct nv_item *buf)
{
	int ret;
	struct nv_item_header *nv_header = &buf->nv_header;

	ret = nv_read_item_from_region(NVE_PRIMARY_REGION_ID, item_id, buf);
	if (ret) {
		NV_LOG_ERR("nve read item from region failed, ret:%d\n", ret);
		return ret;
	}

	if (nv_header->crc32 != calculate_crc32_nv_item(buf)) {
		/* judge is key item or not from origin region read item */
		ret = nv_read_item_from_region(NVE_ORIGIN_REGION_ID, item_id, buf);
		if (ret) {
			NV_LOG_ERR("nve read item from region failed, ret:%d\n", ret);
			return ret;
		}

		if (nv_header->key) {
			ret = nv_read_backup_item(item_id, buf);
			if (ret) {
				NV_LOG_ERR("nve read item from region failed, ret:%d\n", ret);
				return ret;
			}
		} else {
			NV_LOG_ERR("item crc32 check failed, item_id:%d\n", item_id);
			return -EBADMSG;
		}
	}

	return ret;
}

static int nv_write_item_to_device(enum nv_item_id item_id, struct nv_item *buf)
{
	int ret;
	struct nv_item_header *item_header = &buf->nv_header;

	ret = nv_write_item_to_region(NVE_PRIMARY_REGION_ID, item_id, buf);
	if (ret) {
		NV_LOG_ERR("nve write item to region failed, item_id:%d\n", item_id);
		return ret;
	}

	if (item_header->key) {
		ret = nv_write_item_to_region(NVE_BACKUP_REGION_ID, item_id, buf);
		if (ret) {
			NV_LOG_ERR("nve write item to region failed, item_id:%d\n", item_id);
			return ret;
		}
	}

	return ret;
}

static int nv_check_item_header(enum nv_item_id item_id, struct nv_item *buf)
{
	struct nv_item_header *item_header = &buf->nv_header;

	if (item_header->item_index != item_id) {
		NV_LOG_ERR("item_id check failed, item_id:%d\n",
			item_id);
		return -EINVAL;
	}

	if (item_header->valid_size > NV_DATA_SIZE) {
		NV_LOG_ERR("item_id:%d valid_size:%d excess the max\n",
			item_id, item_header->valid_size);
		return -EINVAL;
	}

	return 0;
}

static int nv_read_item(enum nv_item_id item_id, struct nv_item *buf)
{
	int ret;
	struct nv_item read_buf;
	struct nv_item_header *nv_header = &read_buf.nv_header;

	nv_read_item_from_ram(item_id, &read_buf);

	if (nv_header->crc32 != calculate_crc32_nv_item(&read_buf)) {
		NV_LOG_ERR("ram item crc check failed, item_id = %d\n", item_id);
		ret = nv_read_item_from_device(item_id, &read_buf);
		if (ret) {
			NV_LOG_ERR("nve read item from device failed, ret = %d\n", ret);
			return ret;
		}

		nv_write_item_to_ram(item_id, &read_buf);
	}

	*buf = read_buf;

	return 0;
}

static int nv_write_item(enum nv_item_id item_id, struct nv_item *buf)
{
	int ret;
	struct nv_item_header *nv_header = &buf->nv_header;

	ret = nv_check_item_header(item_id, buf);
	if (ret) {
		NV_LOG_ERR("item headercheck failed\n");
		return -EINVAL;
	}

	nv_header->crc32 = calculate_crc32_nv_item(buf);

	nv_write_item_to_ram(item_id, buf);

	ret = nv_write_item_to_device(item_id, buf);
	if (ret) {
		NV_LOG_ERR("item write to device failed, ret:%d\n", ret);
		return ret;
	}

	return ret;
}

static int nve_check_region(struct nve_region_header *nve_header, u8 region_id)
{
	int ret;
	u32 crc32;

	ret = nve_read(region_id * NVE_REGION_SIZE, NVE_HEADER_LEN, (u8 *)nve_header);
	if (ret) {
		NV_LOG_ERR("nve_read failed, ret = %d\n", ret);
		return ret;
	}

	crc32 = calculate_crc32_nve_region_header((u8 *)nve_header);
	if (crc32 != nve_header->crc32) {
		NV_LOG_ERR("region %d Crc32 check fail, crc32:%d nve_header->crc32:%d\n",
			region_id, crc32, nve_header->crc32);
		return -EIO;
	}

	return 0;
}

static int nve_check_primary_region(void)
{
	int ret;
	struct nve_region_header *nve_header = g_nve->nve_header;

	ret = nve_check_region(nve_header, NVE_PRIMARY_REGION_ID);
	if (!ret)
		return ret;

	ret = nve_check_region(nve_header, NVE_BACKUP_REGION_ID);
	if (ret)
		return -EBADMSG;

	NV_LOG_ERR("Nve backup region check success\n");
	ret = nve_write(NVE_PRIMARY_REGION_ID * NVE_REGION_SIZE,
			NVE_HEADER_LEN, (u8 *)nve_header);
	if (ret)
		NV_LOG_ERR("nve_write failed, ret = %d\n", ret);

	return ret;
}

static int nv_check_param(struct nv_info_struct *nv_info)
{
	struct nve_region_header *nve_header = g_nve->nve_header;

	if (!nv_info) {
		NV_LOG_ERR("param check failed, nv_info is NULL\n");
		return -EINVAL;
	}

	if (nv_info->operation > NV_WRITE) {
		NV_LOG_ERR("nv operation invalid\n");
		return -EINVAL;
	}

	if (nv_info->item_id <= NV_HEADER_ID || nv_info->item_id > nve_header->valid_items - 1) {
		NV_LOG_ERR("item_id:%d not less than 1 or more than valid_items:%d\n",
			nv_info->item_id, nve_header->valid_items - 1);
		return -EINVAL;
	}

	return 0;
}

int nv_read_write_item(struct nv_info_struct *nv_info)
{
	int ret;

	if (!g_nve_inited) {
		NV_LOG_ERR("nve not init, please wait\n");
		return -ENODEV;
	}

	if (down_interruptible(&g_nv_sem))
		return -EBUSY;

	ret = nv_check_param(nv_info);
	if (ret) {
		NV_LOG_ERR("nv_info param check failed\n");
		goto out;
	}

	if (nv_info->operation == NV_READ) {
		ret = nv_read_item(nv_info->item_id, &nv_info->buf);
		if (ret) {
			NV_LOG_ERR("nv read item failed\n");
			goto out;
		}
	} else {
		ret = nv_write_item(nv_info->item_id, &nv_info->buf);
		if (ret) {
			NV_LOG_ERR("nv write item failed\n");
			goto out;
		}
	}

out:
	up(&g_nv_sem);
	return ret;
}
EXPORT_SYMBOL(nv_read_write_item);

static int nv_write_item_dirct(u8 region_id, enum nv_item_id item_id, struct nv_item *buf)
{
	int ret;

	nv_write_item_to_ram(item_id, buf);

	ret = nv_write_item_to_region(region_id, item_id, buf);
	if (ret)
		NV_LOG_ERR("nve write item to region failed, ret:%d\n", ret);

	return ret;
}

static void util_dump_buffer_hex(u8 *buf, u32 len)
{
	u32 index = 0;
	char buffer[ITEM_ID_MAX_LEN] = {0};
	u8 i;
	ssize_t cnt;
	u32 offset;

	while (index < len) {
		offset = 0;
		for (i = 0; i <= 0xf; i++) {
			cnt = snprintf(buffer + offset, ITEM_ID_MAX_LEN - offset,
					"%2x ", buf[index]);
			offset += cnt;
			index++;

			if (index >= len)
				break;
		}

		pr_info("%s", buffer);
	}
}

static ssize_t nve_item_id_debug_read(struct file *file, char __user *user_buf,
					size_t count, loff_t *ppos)
{
	ssize_t cnt;
	char buf[ITEM_ID_MAX_LEN] = {0};

	if (*ppos != 0)
		return 0;

	cnt = sprintf(buf, "%d\n", item_id);

	cnt = simple_read_from_buffer(user_buf, count, ppos, buf, cnt);
	*ppos = cnt;

	return cnt;
}

static ssize_t nve_item_id_debug_write(struct file *file, const char __user *user_buf,
					size_t count, loff_t *ppos)
{
	int ret;
	char buf[ITEM_ID_MAX_LEN] = {0};
	enum nv_item_id item_id_tem;
	struct nve_region_header *nve_header = g_nve->nve_header;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	ret = kstrtou16(buf, 10, (u16 *)&item_id_tem);
	if (ret) {
		NV_LOG_ERR("kstrtou16 failed, ret:%d\n", ret);
		return ret;
	}

	if (item_id_tem <= NV_HEADER_ID || item_id_tem > nve_header->valid_items - 1) {
		NV_LOG_ERR("item_id:%d not less than 1 or more than valid_items:%d\n",
			item_id_tem, nve_header->valid_items - 1);
		return -EINVAL;
	}

	item_id = item_id_tem;

	return count;
}

static ssize_t nve_value_debug_read(struct file *file, char __user *user_buf,
					size_t count, loff_t *ppos)
{
	int ret;
	char buf[ITEM_ID_MAX_LEN] = {0};
	struct nv_info_struct nv_info;
	struct nv_item buffer = {0};
	ssize_t cnt = 0;
	int i = 0;
	u8 *item_buf = (u8 *)&nv_info.buf;

	if (*ppos != 0)
		return 0;

	nv_info.operation = NV_READ;
	nv_info.item_id = item_id;

	NV_LOG_INFO("nv read item, item_id:%d\n", item_id);
	ret = nv_read_write_item(&nv_info);
	if (ret) {
		NV_LOG_ERR("nv read item failed, ret:%d\n", ret);
		return ret;
	}

	util_dump_buffer_hex(item_buf, NV_ITEM_SIZE);

	switch (recover_mask) {
	case 0:
		goto out;
	case 1:
		/* erase nv item of primary region */
		ret = nv_write_item_dirct(NVE_PRIMARY_REGION_ID, item_id, &buffer);
		if (ret) {
			NV_LOG_ERR("nv write item failed, ret:%d\n", ret);
			return ret;
		}
		break;
	case 2:
		/* erase nv item of primary and backup region */
		for (i = NVE_PRIMARY_REGION_ID; i <= NVE_BACKUP_REGION_ID; i++) {
			ret = nv_write_item_dirct(i, item_id, &buffer);
			if (ret) {
				NV_LOG_ERR("nv write item failed, ret:%d\n", ret);
				return ret;
			}
		}
		break;
	case 3:
		/* erase nv item of primary and backup region */
		for (i = NVE_PRIMARY_REGION_ID; i <= NVE_BACKUP_REGION_ID; i++) {
			ret = nv_write_item_dirct(i, item_id, &buffer);
			if (ret) {
				NV_LOG_ERR("nv write item failed, ret:%d\n", ret);
				return ret;
			}
		}

		ret = nv_write_item_dirct(NVE_ORIGIN_REGION_ID, item_id, &buffer);
		if (ret) {
			NV_LOG_ERR("nv write item failed, ret:%d\n", ret);
			return ret;
		}
		break;
	default:
		NV_LOG_ERR("Invalid input parameters\n");
		return -EINVAL;
	}

	NV_LOG_INFO("nv read item for recover\n");
	ret = nv_read_write_item(&nv_info);
	if (ret) {
		NV_LOG_ERR("nv read item failed, ret:%d\n", ret);
		return ret;
	}

	util_dump_buffer_hex(item_buf, NV_ITEM_SIZE);

out:
	/* item_buf use 3 char */
	for (i = 0; i < NV_ITEM_SIZE; i++, item_buf++)
		cnt += sprintf(buf + i * 3, "%2x ", *item_buf);

	buf[i * 3] = '\n';

	cnt = simple_read_from_buffer(user_buf, count, ppos, buf, cnt + 1);
	*ppos = cnt;

	return cnt;
}

static ssize_t nve_value_debug_write(struct file *file, const char __user *user_buf,
					size_t count, loff_t *ppos)
{
	int ret;
	char buf[ITEM_ID_MAX_LEN] = {0};
	u8 nv_data;
	struct nv_info_struct nv_info;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	ret = kstrtou8(buf, 10, &nv_data);
	if (ret) {
		NV_LOG_ERR("kstrtou8 failed, ret:%d\n", ret);
		return ret;
	}

	nv_info.operation = NV_READ;
	nv_info.item_id = item_id;

	NV_LOG_INFO("nve read item, item_id:%d\n", item_id);
	ret = nv_read_write_item(&nv_info);
	if (ret) {
		NV_LOG_ERR("nv read item failed, ret:%d\n", ret);
		return ret;
	}

	util_dump_buffer_hex((u8 *)&nv_info.buf, NV_ITEM_SIZE);
	nv_info.buf.nv_data[0] = nv_data;

	nv_info.operation = NV_WRITE;
	ret = nv_read_write_item(&nv_info);
	if (ret) {
		NV_LOG_ERR("nv write item failed, ret:%d\n", ret);
		return ret;
	}

	NV_LOG_INFO("nve read item from primary region, item_id:%d\n", item_id);
	ret = nv_read_item_from_region(NVE_PRIMARY_REGION_ID, item_id, &nv_info.buf);
	if (ret) {
		NV_LOG_ERR("nve read item from region failed, ret:%d\n", ret);
		return ret;
	}

	util_dump_buffer_hex((u8 *)&nv_info.buf, NV_ITEM_SIZE);

	if (nv_info.buf.nv_header.key) {
		NV_LOG_INFO("nve read item from backup region, item_id:%d\n", item_id);
		ret = nv_read_item_from_region(NVE_BACKUP_REGION_ID, item_id, &nv_info.buf);
		if (ret) {
			NV_LOG_ERR("nve read item from region failed, ret:%d\n", ret);
			return ret;
		}

		util_dump_buffer_hex((u8 *)&nv_info.buf, NV_ITEM_SIZE);
	}

	return count;
}

static ssize_t nve_recover_mask_debug_read(struct file *file, char __user *user_buf,
					size_t count, loff_t *ppos)
{
	ssize_t cnt;
	char buf[ITEM_ID_MAX_LEN] = {0};

	if (*ppos != 0)
		return 0;

	cnt = sprintf(buf, "%d\n", recover_mask);

	cnt = simple_read_from_buffer(user_buf, count, ppos, buf, cnt);
	*ppos = cnt;

	return cnt;
}

static ssize_t nve_recover_mask_debug_write(struct file *file, const char __user *user_buf,
					size_t count, loff_t *ppos)
{
	int ret;
	u8 recover_mask_tem;
	char buf[ITEM_ID_MAX_LEN] = {0};

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	ret = kstrtou8(buf, 10, &recover_mask_tem);
	if (ret) {
		NV_LOG_ERR("kstrtou8 failed, ret:%d\n", ret);
		return ret;
	}

	if (recover_mask_tem > 4) {
		NV_LOG_ERR("Invalid recover_mask param\n");
		return -EINVAL;
	}

	recover_mask = recover_mask_tem;

	return count;
}

static const struct file_operations nve_item_id_debug_fops = {
	.read = nve_item_id_debug_read,
	.write = nve_item_id_debug_write,
};

static const struct file_operations nve_value_debug_fops = {
	.read = nve_value_debug_read,
	.write = nve_value_debug_write,
};

static const struct file_operations nve_recover_mask_debug_fops = {
	.read = nve_recover_mask_debug_read,
	.write = nve_recover_mask_debug_write,
};

static int nve_debugfs_init(void)
{
	struct dentry *debugfs_file = NULL;

	nve_debugfs_root = debugfs_create_dir("nve", NULL);
	if (IS_ERR(nve_debugfs_root))
		return PTR_ERR(nve_debugfs_root);

	debugfs_file = debugfs_create_file("item_id", 0644, nve_debugfs_root,
					   NULL, &nve_item_id_debug_fops);
	if (IS_ERR_OR_NULL(debugfs_file)) {
		NV_LOG_ERR("item_id debugfs create failed %ld\n", PTR_ERR(debugfs_file));
		goto debugfs_err;
	}

	debugfs_file = debugfs_create_file("value", 0644, nve_debugfs_root,
					   NULL, &nve_value_debug_fops);
	if (IS_ERR_OR_NULL(debugfs_file)) {
		NV_LOG_ERR("value debugfs create failed %ld\n", PTR_ERR(debugfs_file));
		goto debugfs_err;
	}

	debugfs_file = debugfs_create_file("recover_mask", 0644, nve_debugfs_root,
					   NULL, &nve_recover_mask_debug_fops);
	if (IS_ERR_OR_NULL(debugfs_file)) {
		NV_LOG_ERR("recover_mask debugfs create failed %ld\n", PTR_ERR(debugfs_file));
		goto debugfs_err;
	}

	return 0;

debugfs_err:
	debugfs_remove(nve_debugfs_root);
	return PTR_ERR(debugfs_file);
}

static int __init nv_init(void)
{
	int ret;

	sema_init(&g_nv_sem, 1);

	g_nve = kzalloc(sizeof(struct nve_struct), GFP_KERNEL);
	if (!g_nve) {
		ret = -ENOMEM;
		goto out;
	}

	g_nve->nve_header = kzalloc(sizeof(struct nve_region_header), GFP_KERNEL);
	if (!g_nve->nve_header) {
		ret = -ENOMEM;
		goto out;
	}

	g_nve->nve_region_ram = kzalloc((size_t)NVE_REGION_SIZE, GFP_KERNEL);
	if (!g_nve->nve_region_ram) {
		ret = -ENOMEM;
		goto out;
	}

	ret = nve_check_primary_region();
	if (ret) {
		NV_LOG_ERR("nve primary region check failed\n");
		goto out;
	}

	ret = nve_read(NVE_PRIMARY_REGION_ID * NVE_REGION_SIZE,
			g_nve->nve_header->valid_items * NV_ITEM_SIZE, (u8 *)g_nve->nve_region_ram);
	if (ret) {
		NV_LOG_ERR("nve_read failed, ret = %d\n", ret);
		goto out;
	}

	ret = nve_debugfs_init();
	if (ret)
		goto out;

	g_nve_inited = true;

	NV_LOG_INFO("nv init success\n");
	return 0;

out:
	if (g_nve) {
		kfree(g_nve->nve_region_ram);
		kfree(g_nve->nve_header);
		kfree(g_nve);
	}

	return 0;
}

static void __exit nv_exit(void)
{
	kfree(g_nve->nve_region_ram);
	kfree(g_nve->nve_header);
	kfree(g_nve);
	debugfs_remove(nve_debugfs_root);

	g_nve_inited = false;

	NV_LOG_INFO("nv exit\n");
}

module_init(nv_init);
module_exit(nv_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("NV driver");
