// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/init.h>
#include <linux/initrd.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/major.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/highmem.h>
#include <linux/mutex.h>
#include <linux/pagemap.h>
#include <linux/radix-tree.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/backing-dev.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#define XRING_RAMDISK_MAJOR 60

/*
 * Each block ramdisk device has a radix_tree rd_pages of pages that stores
 * the pages containing the block device's contents.
 */
struct xrd_device {
	struct gendisk *xrd_disk;
	spinlock_t xrd_lock;
	struct radix_tree_root xrd_pages;
	u64	xrd_nr_pages;
	void *vaddr_ptr;
	u64 mem_size;
};
static struct xrd_device *g_xrd;

/*
 * Look up and return a xrd's page for a given sector.
 */
static struct page *xrd_lookup_page(sector_t sector)
{
	pgoff_t idx;
	struct page *page = NULL;

	rcu_read_lock();
	idx = sector >> PAGE_SECTORS_SHIFT; /* sector to page index */
	page = radix_tree_lookup(&g_xrd->xrd_pages, idx);
	rcu_read_unlock();

	return page;
}

/*
 * Look up and return a xrd's page for a given sector.
 * If one does not exist, allocate an empty page, and insert that. Then
 * return it.
 */
static struct page *xrd_insert_page(sector_t sector)
{
	pgoff_t idx;
	struct page *page = NULL;
	void *cur_vaddr = NULL;

	page = xrd_lookup_page(sector);
	if (page)
		return page;

	idx = sector >> PAGE_SECTORS_SHIFT;
	cur_vaddr = g_xrd->vaddr_ptr + (PAGE_SIZE * idx);
	page = virt_to_page(cur_vaddr);
	if (!page)
		return NULL;

	if (radix_tree_preload(GFP_NOIO))
		return NULL;

	spin_lock(&g_xrd->xrd_lock);
	if (radix_tree_insert(&g_xrd->xrd_pages, idx, page)) {
		pr_err("radix_tree_insert failed!\n");
		page = radix_tree_lookup(&g_xrd->xrd_pages, idx);
		WARN_ON(!page);
	} else {
		g_xrd->xrd_nr_pages++;
	}
	spin_unlock(&g_xrd->xrd_lock);

	radix_tree_preload_end();

	return page;
}

static void xrd_free_pages(void)
{
	struct radix_tree_iter iter = {0};
	void **slot = NULL;
	unsigned long index;

	radix_tree_for_each_slot(slot, &g_xrd->xrd_pages, &iter, 0) {
		index = iter.index;
		radix_tree_delete(&g_xrd->xrd_pages, index);
	}
}

/*
 * rw_xrd_setup must be called before copy_to_xrd.
 */
static int rw_xrd_setup(sector_t sector, size_t n)
{
	unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
	size_t copy;

	copy = min_t(size_t, n, PAGE_SIZE - offset);
	if (!xrd_insert_page(sector))
		return -ENOSPC;
	if (copy < n) {
		sector += copy >> SECTOR_SHIFT;
		if (!xrd_insert_page(sector))
			return -ENOSPC;
	}
	return 0;
}

/*
 * Copy n bytes from src to the xrd starting at sector. Does not sleep.
 */
static void copy_to_xrd(const void *src, sector_t sector, size_t n)
{
	struct page *page = NULL;
	void *dst = NULL;
	unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
	size_t copy;

	copy = min_t(size_t, n, PAGE_SIZE - offset);
	page = xrd_lookup_page(sector);
	WARN_ON(!page);

	dst = kmap_atomic(page);
	memcpy(dst + offset, src, copy);
	kunmap_atomic(dst);

	if (copy < n) {
		src += copy;
		sector += copy >> SECTOR_SHIFT;
		copy = n - copy;
		page = xrd_lookup_page(sector);
		WARN_ON(!page);

		dst = kmap_atomic(page);
		memcpy(dst, src, copy);
		kunmap_atomic(dst);
	}
}

/*
 * Copy n bytes to dst from the xrd starting at sector. Does not sleep.
 */
static void copy_from_xrd(void *dst, sector_t sector, size_t n)
{
	struct page *page = NULL;
	void *src = NULL;
	unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
	size_t copy;

	copy = min_t(size_t, n, PAGE_SIZE - offset);
	page = xrd_lookup_page(sector);
	if (page) {
		src = kmap_atomic(page);
		memcpy(dst, src + offset, copy);
		kunmap_atomic(src);
	} else {
		memset(dst, 0, copy);
	}

	if (copy < n) {
		dst += copy;
		sector += copy >> SECTOR_SHIFT;
		copy = n - copy;
		page = xrd_lookup_page(sector);
		if (page) {
			src = kmap_atomic(page);
			memcpy(dst, src, copy);
			kunmap_atomic(src);
		} else {
			memset(dst, 0, copy);
		}
	}
}

/*
 * Process a single bvec of a bio.
 */
static int xrd_do_bvec(struct page *page,
			unsigned int len, unsigned int off, unsigned int op,
			sector_t sector)
{
	void *mem;
	int err = 0;

	err = rw_xrd_setup(sector, len);
	if (err)
		goto out;

	mem = kmap_atomic(page);
	if (!op_is_write(op)) {
		copy_from_xrd(mem + off, sector, len);
		flush_dcache_page(page);
	} else {
		flush_dcache_page(page);
		copy_to_xrd(mem + off, sector, len);
	}
	kunmap_atomic(mem);

out:
	return err;
}

static void xrd_submit_bio(struct bio *bio)
{
	sector_t sector = bio->bi_iter.bi_sector;
	struct bio_vec bvec = {0};
	struct bvec_iter iter = {0};

	g_xrd = bio->bi_bdev->bd_disk->private_data;
	bio_for_each_segment(bvec, bio, iter) {
		unsigned int len = bvec.bv_len;
		int err;

		/* Don't support un-aligned buffer */
		WARN_ON_ONCE((bvec.bv_offset & (SECTOR_SIZE - 1)) ||
				(len & (SECTOR_SIZE - 1)));

		err = xrd_do_bvec(bvec.bv_page, len, bvec.bv_offset,
				  bio_op(bio), sector);

		if (err) {
			if (err == -ENOMEM && bio->bi_opf & REQ_NOWAIT) {
				bio_wouldblock_error(bio);
				return;
			}
			bio_io_error(bio);
			return;
		}
		sector += len >> SECTOR_SHIFT;
	}

	bio_endio(bio);
}

static const struct block_device_operations xrd_fops = {
	.owner =		THIS_MODULE,
	.submit_bio =		xrd_submit_bio,
};

/*
 * And now the modules code and kernel interface.
 */
MODULE_LICENSE("GPL");
MODULE_ALIAS_BLOCKDEV_MAJOR(XRING_RAMDISK_MAJOR);
MODULE_ALIAS("rd");

static int get_reserve_mem_info(unsigned long long *base_addr, unsigned long long *rmem_size)
{
	struct device_node *node;
	int ret = 0;
	unsigned long long reg[2];

	node = of_find_node_by_path("/reserved-memory/reserve-ramdisk");
	if (!node) {
		pr_err("xring_ramdisk: Failed to find reserve memory node for xring-ramdisk!\n");
		return -ENODEV;
	}
	ret = of_property_read_u64_array(node, "reg", reg, 2);
	if (ret) {
		pr_err("xring_ramdisk: Failed to get reserve memory reg!\n");
		return ret;
	}
	*base_addr = reg[0];
	*rmem_size = reg[1];
	pr_err("base_addr: %llx\n", *base_addr);
	pr_err("rmem_size: %llx\n", *rmem_size);
	return ret;
}

static int xrd_alloc(void)
{
	struct gendisk *disk;
	char buf[DISK_NAME_LEN] = {0};
	unsigned long long base_addr = 0;
	unsigned long long rmem_size = 0;
	int err = -ENOMEM;

	if (g_xrd)
		return -EEXIST;

	g_xrd = kzalloc(sizeof(*g_xrd), GFP_KERNEL);
	if (!g_xrd)
		return -ENOMEM;

	get_reserve_mem_info(&base_addr, &rmem_size);
	if (rmem_size == 0) {
		pr_err("xring_ramdisk: Failed to get reserve memory from device tree!\n");
		return -ENOSPC;
	}
	g_xrd->vaddr_ptr = ioremap_cache(base_addr, rmem_size);
	g_xrd->mem_size = (u64)rmem_size;
	if (g_xrd->vaddr_ptr == NULL)
		pr_err("ioremap failed\n");
	else
		pr_err("ioremap success!\n");

	spin_lock_init(&g_xrd->xrd_lock);
	INIT_RADIX_TREE(&g_xrd->xrd_pages, GFP_ATOMIC);

	snprintf(buf, DISK_NAME_LEN, "xr_ram");

	g_xrd->xrd_disk = blk_alloc_disk(NUMA_NO_NODE);
	disk = g_xrd->xrd_disk;
	if (!disk) {
		kfree(g_xrd);
		g_xrd = NULL;
		return -ENOMEM;
	}

	disk->major		= XRING_RAMDISK_MAJOR;
	disk->first_minor	= 0;
	disk->minors		= 1;
	disk->fops		= &xrd_fops;
	disk->private_data	= g_xrd;
	strscpy(disk->disk_name, buf, DISK_NAME_LEN);
	set_capacity(disk, rmem_size >> 9);

	blk_queue_physical_block_size(disk->queue, PAGE_SIZE);

	/* Tell the block layer that this is not a rotational device */
	blk_queue_flag_set(QUEUE_FLAG_NONROT, disk->queue);
	blk_queue_flag_clear(QUEUE_FLAG_ADD_RANDOM, disk->queue);
	err = add_disk(disk);
	if (err)
		return err;

	return 0;
}

static void xrd_del_one(void)
{
	if (g_xrd->xrd_disk != NULL) {
		del_gendisk(g_xrd->xrd_disk);
		put_disk(g_xrd->xrd_disk);
	}
	xrd_free_pages();
	iounmap(g_xrd->vaddr_ptr);
	g_xrd->vaddr_ptr = NULL;
	kfree(g_xrd);
	g_xrd = NULL;
}

static struct kobject *kob;

static ssize_t cmd_show(struct kobject *kobjs, struct kobj_attribute *attr, char *buf)
{
	pr_info("cmd show\n");
	return 0;
}

static void invalid_dcache_range(u64 start, u64 end)
{
	u64 value;
	u64 tmp;

	__asm__ __volatile__ (
		"mrs %3, ctr_el0\n"
		"ubfx %3, %3, #16, #4\n"
		"mov %2, #4\n"
		"lsl %2, %2, %3\n"  /* cache line size */

		/* val1 <- minimal cache line size in cache system */
		"sub %3, %2, #1\n"
		"bic %0, %0, %3\n"
		"1:	dc civac, %0\n" /* clean & invalidate data or unified cache */
		"add %0, %0, %2\n"
		"cmp %0, %1\n"
		"b.lo 1b\n"
		"dsb sy\n"
		: "+r"(start), "+r"(end), "+r"(value), "+r"(tmp)
	);
}

static ssize_t cmd_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	pr_info("cmd store!\n");
	if (strncmp(buf, "1", 1) == 0) {
		invalid_dcache_range((u64)(g_xrd->vaddr_ptr), (u64)(g_xrd->vaddr_ptr) + g_xrd->mem_size - 1);
		pr_info("xr_ram: clean and invalid dcache!\n");
	}
	return count;
}

static struct kobj_attribute cmd_attr = __ATTR(cmd, 0660, cmd_show, cmd_store);

static int create_sysfs(void)
{
	int ret = 0;

	kob = kobject_create_and_add("xr_ram_cmd", kernel_kobj->parent);
	if (!kob) {
		pr_err("xr_ram: failed to create sysfs entry\n");
		ret = -ENOMEM;
	}
	if (sysfs_create_file(kob, &cmd_attr.attr)) {
		pr_err("xr_ram: failed to create cmd file\n");
		kobject_put(kob);
		ret = -ENOMEM;
	}
	return ret;
}

static int __init xring_ramdisk_init(void)
{
	int err;

	if (__register_blkdev(XRING_RAMDISK_MAJOR, "xring_ramdisk", NULL))
		return -EIO;
	err = xrd_alloc();
	if (err)
		goto out_free;

	err = create_sysfs();
	if (err)
		goto out_free;

	pr_info("xring_ramdisk: module loaded\n");
	invalid_dcache_range((u64)(g_xrd->vaddr_ptr), (u64)(g_xrd->vaddr_ptr) + g_xrd->mem_size - 1);
	return 0;

out_free:
	unregister_blkdev(XRING_RAMDISK_MAJOR, "xring_ramdisk");
	xrd_del_one();
	pr_info("xring_ramdisk: module NOT loaded !!!\n");
	return err;
}

static void __exit xring_ramdisk_exit(void)
{
	unregister_blkdev(XRING_RAMDISK_MAJOR, "xring_ramdisk");
	xrd_del_one();
	kobject_put(kob);

	pr_info("xring_ramdisk: module unloaded\n");
}

module_init(xring_ramdisk_init);
module_exit(xring_ramdisk_exit);
