// SPDX-License-Identifier: GPL-2.0
/*
 * X-Ring IOMMU IOVA manager.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#include <linux/dma-map-ops.h>
#include <linux/device/bus.h>
#include <linux/genalloc.h>
#include <linux/iova.h>
#include <linux/io-pgtable.h>
#include <linux/kthread.h>
#include <linux/pci.h>
#include <linux/rwlock.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <asm/cacheflush.h>
#include <asm-generic/errno-base.h>
#include <uapi/linux/sched/types.h>

#include "xring_smmu.h"
#include "xring_iommu_map.h"
#include "xring_iommu_iova_cookie.h"

static void xring_smmu_iova_delay_free(struct xring_iommu_dma_cookie *cookie)
{
	unsigned int pages_num, i, size, pong, j;
	unsigned long pingpong;
	unsigned long iova_start;
	unsigned long *free_size = NULL;

	pingpong = 1UL - cookie->delay_free->pingpong;

	for (i = 0; i < cookie->iova_pool_num; i++) {
		pages_num = DIV_ROUND_UP(cookie->iova[i].iova_size, PAGE_SIZE);
		iova_start = cookie->iova[i].iova_start;
		free_size = cookie->free_size[i];
		mutex_lock(&cookie->delay_free->mutex);
		for (j = 0; j < pages_num; j++) {
			size = free_size[j] & IOVA_SIZE_MASK;
			if (size == 0)
				continue;

			pong = (free_size[j] & PINGPONG_MASK) >> PINGPONG_SHIFT;
			if (pong != pingpong)
				continue;

			free_size[j] = 0;
			gen_pool_free(cookie->iova_pool,
				(iova_start + ((unsigned long)j << PAGE_SHIFT)),
				size);
		}
		mutex_unlock(&cookie->delay_free->mutex);
	}
	cookie->delay_free->end = true;
}

static int xring_smmu_iova_delayfree_thread(void *p)
{
	struct device *dev = (struct device *)p;
	struct iommu_domain *domain = NULL;
	struct xring_iommu_dma_cookie *cookie = NULL;
	DEFINE_WAIT(wait);

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, domain is null\n", __func__);
		return -ENOENT;
	}

	cookie = to_smmu_domain(domain)->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iova_cookie is null\n", __func__);
		return -ENOENT;
	}

	while (!kthread_should_stop()) {
		prepare_to_wait(&cookie->delay_free->wait_q, &wait,
			TASK_UNINTERRUPTIBLE);
		schedule();
		finish_wait(&cookie->delay_free->wait_q, &wait);
		xring_iommu_flush_tlb(dev, domain);
		xring_smmu_iova_delay_free(cookie);
	}
	return 0;
}

static void xring_smmu_init_iova_delay_free(struct device *dev,
				struct xring_iommu_dma_cookie *cookie)
{
	int ret;
	unsigned int pages_num, i = 0;
	struct cpumask sched_cpus;
	struct xring_iommu_iova_delay_free *delay_free = NULL;
	struct sched_param param;

	if (cookie->iova_free == IMME_FREE)
		return;

	delay_free = kzalloc(sizeof(*delay_free), GFP_KERNEL);
	if (!delay_free)
		goto out_err;

	do {
		pages_num = DIV_ROUND_UP(cookie->iova[i].iova_size, PAGE_SIZE);
		cookie->free_size[i] = vcalloc(pages_num, sizeof(u32));
		if (!cookie->free_size[i])
			goto out_free;
		i++;
	} while (i < cookie->iova_pool_num);

	init_waitqueue_head(&delay_free->wait_q);
	mutex_init(&delay_free->mutex);
	spin_lock_init(&delay_free->lock);
	delay_free->end = true;
	pages_num = DIV_ROUND_UP(cookie->iova_total_size, PAGE_SIZE);
	delay_free->waterline =
		((pages_num * IOVA_WATERLINE_PARA) / DELAYED_FREE_WATERLINE) /
			IOVA_WATERLINE_PARA;
	if (delay_free->waterline < DELAYED_FREE_WATERLINE) {
		dev_info(dev, "%s,iova_size %lu too small,disable delay free\n",
			__func__, cookie->iova_total_size);
		goto out_free;
	}
	cookie->delay_free = delay_free;

	delay_free->task = kthread_run(xring_smmu_iova_delayfree_thread, dev,
		"iova.%s", dev_name(dev));
	if (IS_ERR(delay_free->task)) {
		dev_err(dev, "%s, create delay free task err %d\n",
			__func__, IS_ERR(delay_free->task));
		goto out_stop;
	}

	cpumask_setall(&sched_cpus);
	cpumask_clear_cpu(WATCHDOG_CPU, &sched_cpus);
	cpumask_clear_cpu(WIFI_INTX_CPU, &sched_cpus);
	set_cpus_allowed_ptr(delay_free->task, &sched_cpus);

	/* set thread priority and schedule policy */
	param.sched_priority = DELAYED_FREE_SCHED_PRI;
	ret = sched_setscheduler(delay_free->task, SCHED_RR, &param);
	if (ret)
		dev_info(dev, "%s, task set priority error\n", __func__);

	dev_info(dev, "%s, iova delay free waterline %lu\n",
		__func__, delay_free->waterline);

	return;

out_stop:
	for (i = 0; i < cookie->iova_pool_num && cookie->free_size[i]; i++)
		kfree(cookie->free_size[i]);
	cookie->delay_free = NULL;
out_free:
	kfree(delay_free);
out_err:
	cookie->iova_free = IMME_FREE;
	dev_err(dev, "%s iova_free rollback imme free!\n", __func__);
}

static void xring_iommu_domain_swap(struct xring_iommu_domain_data *pre,
					struct xring_iommu_domain_data *cur)
{
	struct xring_iommu_domain_data *midtmp = NULL;

	midtmp = pre;
	pre = cur;
	cur = midtmp;
}

static int of_get_iova_info_smmu(struct device_node *np,
				struct xring_iommu_dma_cookie *cookie, bool flags)
{
	struct device_node *node = NULL;
	unsigned long count, size, i = 0;
	unsigned long end, total_size = 0;
	unsigned long iova_default_end = IOVA_DEFAULT_ADDR + IOVA_DEFAULT_SIZE;
	unsigned long default_size = IOVA_DEFAULT_SIZE;

	if (!flags) {
		cookie->iova[i].iova_start = IOVA_DEFAULT_ADDR;
		cookie->iova[i].iova_size = IOVA_DEFAULT_SIZE;
		cookie->iova_default_size = default_size;
		cookie->iova_align = PAGE_SIZE;
		cookie->iova_pool_num = 1;
		cookie->iova_free = IMME_FREE;
	}

	if (!np)
		return -ENODEV;

	node = of_get_child_by_name(np, "iova-info");
	if (!node) {
		xring_smmu_err("no iova-info, default cfg(0x%lx, 0x%lx)\n",
			cookie->iova[0].iova_start, cookie->iova[0].iova_size);
		return 0;
	}

	if (of_property_read_bool(node, "limit-4g")) {
		cookie->iova[i].iova_size = IOVA_LIMIT_DEFAULT_SIZE;
		default_size = IOVA_LIMIT_DEFAULT_SIZE;
		cookie->iova_default_size = default_size;
	}

	xring_smmu_debug("default_size:0x%lx, cookie->iova_default_size:0x%lx\n",
		default_size, cookie->iova_default_size);

	if (of_property_read_u32(node, "iova-free", (u32 *)&cookie->iova_free))
		xring_smmu_debug("read iova free is default\n");

	//iova resource
	count = of_property_count_u64_elems(node, "iova-reserve");
	if (!count) {
		xring_smmu_info("Err: no iova_reserve\n");
		return -EINVAL;
	}

	if (count % IOVA_DEFAULT_PARA_NUM)
		return -EINVAL;

	size = count / IOVA_DEFAULT_PARA_NUM;
	xring_smmu_debug("iova-reserve count:0x%lx, size:%lu\n",
		count, size);
	cookie->iova_pool_num = size;
	memset(cookie->iova, 0, sizeof(struct xring_iommu_domain_data) * size);
	for (i = 0; i < size; i++) {
		of_property_read_u64_index(node, "iova-reserve",
			IOVA_DEFAULT_PARA_NUM * i,
			(u64 *)&cookie->iova[i].iova_start);
		of_property_read_u64_index(node, "iova-reserve",
			IOVA_DEFAULT_PARA_NUM * i + 1,
			(u64 *)&cookie->iova[i].iova_size);
		total_size += cookie->iova[i].iova_size;
		end = cookie->iova[i].iova_start + cookie->iova[i].iova_size;
		xring_smmu_debug("cookie->iova[%lu].iova_start:0x%lx\n"
			"cookie->iova[%lu].iova_size:0x%lx\n",
			i, cookie->iova[i].iova_start, i, cookie->iova[i].iova_size);
		if (end > iova_default_end) {
			xring_smmu_err("Err: %d, iova reserved range overlaps\n"
				"please check\n", __LINE__);
			xring_smmu_err("end:0x%lx, iova_default_end:0x%lx\n", end, iova_default_end);
			return -EINVAL;
		}
		if (i == 0)
			continue;
		if (cookie->iova[i].iova_start < cookie->iova[i - 1].iova_start)
			xring_iommu_domain_swap(&cookie->iova[i],
				&cookie->iova[i - 1]);

		end = cookie->iova[i - 1].iova_start +
			cookie->iova[i - 1].iova_size;
		if (cookie->iova[i].iova_start < end) {
			xring_smmu_err("Err: %d, The incoming iova reserved range overlaps\n"
				"please check\n", __LINE__);
			return -EINVAL;
		}
	}

	if (cookie->iova[0].iova_start == 0) {
		cookie->iova[0].iova_start = IOVA_DEFAULT_ADDR;
		if (cookie->iova[0].iova_size < IOVA_DEFAULT_ADDR)
			cookie->iova[0].iova_size = 0;
		else
			cookie->iova[0].iova_size -= IOVA_DEFAULT_ADDR;
	}
	cookie->iova_total_size = default_size - total_size;

	return 0;
}

static unsigned long xring_iommu_iova_range_calculation(
			struct xring_iommu_domain_data *iova,
			unsigned long num, unsigned long iova_size)
{
	struct xring_iommu_domain_data *iova_tmp;
	uint i, j;
	unsigned long iova_start = IOVA_DEFAULT_ADDR;
	unsigned long pool_num = 1;

	if ((num == 1) && (iova[0].iova_start == iova_start)
		&& (iova[0].iova_size == iova_size))
		return num;

	iova_tmp = kzalloc(sizeof(struct xring_iommu_domain_data) *
		IOVA_MAX_NUM, GFP_KERNEL);
	if (!iova_tmp)
		return -ENOMEM;

	for (i = 0, j = 0; i < num && j < IOVA_MAX_NUM; i++) {
		if ((iova[i].iova_start - iova_start) != 0) {
			iova_tmp[j].iova_start = iova_start;
			iova_tmp[j].iova_size = iova[i].iova_start - iova_start;
			iova_size -= iova_tmp[j].iova_size;
			if (iova_size - iova[i].iova_size == 0)
				continue;
			j++;
			pool_num++;
		}

		iova_tmp[j].iova_start = iova[i].iova_start + iova[i].iova_size;
		iova_tmp[j].iova_size = iova_size - iova[i].iova_size;
		iova_size = iova_tmp[j].iova_size;
		iova_start = iova_tmp[j].iova_start;
	}

	memcpy(iova, iova_tmp, sizeof(struct xring_iommu_domain_data) * pool_num);

	kfree(iova_tmp);
	return pool_num;
}

struct gen_pool *iova_pool_setup(struct xring_iommu_domain_data *iova,
				unsigned long num, unsigned long align)
{
	struct gen_pool *pool = NULL;
	int ret;
	uint i = 0;

	pool = gen_pool_create(order_base_2(align), -1);/*lint !e666 */
	if (!pool) {
		xring_smmu_err("create gen pool failed!\n");
		return NULL;
	}

	for (i = 0; i < num; i++) {
		ret = gen_pool_add(pool, iova->iova_start, iova->iova_size, -1);
		if (ret) {
			xring_smmu_err("gen pool add failed!\n");
			gen_pool_destroy(pool);
			return NULL;
		}
		iova++;
	}

	return pool;
}

static struct xring_iommu_dma_cookie
	*xring_smmu_create_mapping_sized(struct device *dev)
{
	struct xring_iommu_dma_cookie *cookie;

	cookie = kzalloc(sizeof(*cookie), GFP_KERNEL);
	if (!cookie)
		goto err;

	if (of_get_iova_info_smmu(dev->of_node, cookie, false))
		goto err1;

	cookie->iova_pool_num = xring_iommu_iova_range_calculation(
		cookie->iova, cookie->iova_pool_num, cookie->iova_default_size);

	cookie->iova_pool = iova_pool_setup(cookie->iova,
		cookie->iova_pool_num, cookie->iova_align);
	if (!cookie->iova_pool) {
		xring_smmu_err("setup dev(%s) iova pool fail\n", dev_name(dev));
		goto err1;
	}

	return cookie;

err1:
	kfree(cookie);
err:
	return ERR_PTR(-ENOMEM);
}

void xring_iommu_iova_free_imme(struct gen_pool *iova_pool,
		unsigned long iova, size_t size)
{
	if (!iova_pool)
		return;

	gen_pool_free(iova_pool, iova, size);
}
EXPORT_SYMBOL(xring_iommu_iova_free_imme);

void xring_iommu_iova_free_delay(struct xring_iommu_dma_cookie *cookie,
		unsigned long iova, size_t size)
{
	unsigned long ping;
	unsigned long flag;
	unsigned long iova_end;
	unsigned int bgn_page, i;
	unsigned int page_num = size >> PAGE_SHIFT;
	unsigned long *free_size = NULL;
	struct xring_iommu_iova_delay_free *delay_free = NULL;

	for (i = 0; i < cookie->iova_pool_num; i++) {
		iova_end = cookie->iova[i].iova_start +
			cookie->iova[i].iova_size;
		if ((iova >= cookie->iova[i].iova_start) &&
			(iova + size) <= iova_end)
			break;
	}

	if (i == cookie->iova_pool_num) {
		xring_smmu_err("iova 0x%lx err\n", iova);
		for (i = 0; i < cookie->iova_pool_num; i++)
			xring_smmu_err("%d--iova_start:0x%lx, iova_size:0x%lx\n",
				i, cookie->iova[i].iova_start,
				cookie->iova[i].iova_size);
		return;
	}

	if (size > IOVA_SIZE_MASK) {
		xring_smmu_err("iova 0x%lx iova size 0x%lx err\n",
			iova, size);
		return;
	}

	delay_free = cookie->delay_free;
	if (!delay_free) {
		xring_smmu_err("delay free is null\n");
		return;
	}

	free_size = cookie->free_size[i];
	spin_lock_irqsave(&delay_free->lock, flag);
	ping = delay_free->pingpong;
	bgn_page = (iova - cookie->iova[i].iova_start) >> PAGE_SHIFT;
	if (free_size[bgn_page] != 0) {
		xring_smmu_err("iova 0x%lx free err, size 0x%lx\n",
			iova, size);
		return;
	}
	free_size[bgn_page] = (u32)size | (ping << PINGPONG_SHIFT);
	delay_free->pages += page_num;

	if (delay_free->pages >= delay_free->waterline) {
		if (delay_free->end) {
			delay_free->pages = 0;
			delay_free->pingpong = 1 - ping;
			delay_free->end = false;
			wake_up(&delay_free->wait_q);
		}
	}
	spin_unlock_irqrestore(&delay_free->lock, flag);
}
EXPORT_SYMBOL(xring_iommu_iova_free_delay);

static int __xring_smmu_set_reserve_pool(struct gen_pool *pool,
						size_t start, size_t size)
{
	struct gen_pool_chunk *chunk = NULL;
	unsigned long nbits, start_bit;
	unsigned long end = start + size - 1;
	int order = pool->min_alloc_order;

	if (size == 0)
		return 0;

	rcu_read_lock();
	list_for_each_entry_rcu(chunk, &pool->chunks, next_chunk) {
		if (end < chunk->start_addr || start < chunk->start_addr ||
			end > chunk->end_addr) {
			xring_smmu_info("please check iova res configuration\n"
				"iova_start:0x%zx iova_start:0x%zx\n", start, start + size);
			xring_smmu_info("chunk->start_addr:0x%lx, chunk->end_addr:0x%lx\n",
				chunk->start_addr, chunk->end_addr);
			rcu_read_unlock();
			return 0;
		}

		if (size > atomic_long_read(&chunk->avail))
			continue;

		if (start > chunk->end_addr)
			continue;

		nbits = (size + (1UL << order) - 1) >> order;
		start_bit = (start - chunk->start_addr) >> order;
		bitmap_set(chunk->bits, start_bit, nbits);
		break;
	}
	rcu_read_unlock();

	return 0;
}

static int xring_smmu_set_reserve_pool(struct xring_iommu_dma_cookie *cookie,
	struct xring_iommu_domain_data *iova, unsigned long num)
{
	int i, ret = 0;

	for (i = 0; i < num; i++) {
		ret = __xring_smmu_set_reserve_pool(cookie->iova_pool,
				iova[i].iova_start, iova[i].iova_size);
		if (ret)
			return ret;
	}

	return ret;
}

static void xring_smmu_printk_iova_genpool(struct gen_pool *pool)
{
	struct gen_pool_chunk *chunk = NULL;
	unsigned long nbits, nbytes, tmpbytes;
	int order = pool->min_alloc_order, i;

	rcu_read_lock();
	list_for_each_entry_rcu(chunk, &pool->chunks, next_chunk) {
		nbits = (chunk->end_addr - chunk->start_addr + 1) >> order;
		tmpbytes = BITS_TO_BYTES(nbits) % sizeof(long);
		if (tmpbytes)
			nbytes = BITS_TO_BYTES(nbits) / sizeof(long) + 1;
		else
			nbytes = BITS_TO_BYTES(nbits) / sizeof(long);
		xring_smmu_info("iova-debug: start_addr: 0x%lx, end_addr:0x%lx\n"
			"order:0x%x, nbits:0x%lx, nbytes:0x%lx\n",
			chunk->start_addr, chunk->end_addr, order, nbits, nbytes);
		xring_smmu_info("bit-map:\n");
		for (i = 0; i < nbytes; i++) {
			if (chunk->bits[i] == 0)
				continue;
			xring_smmu_info("%d nbytes: 0x%lx ", i, chunk->bits[i]);
		}
	}
	rcu_read_unlock();
}

unsigned long xring_iommu_iova_alloc(struct gen_pool *iova_pool, size_t size,
					unsigned long align)
{
	unsigned long iova;

	if (iova_pool->min_alloc_order >= 0) {
		if (align > (1UL << (unsigned long)iova_pool->min_alloc_order))
			WARN(1, "iommu domain cant align to 0x%lx\n", align);
	} else {
		pr_warn("The min_alloc_order of iova_pool is negative!\n");
		return 0;
	}

	iova = gen_pool_alloc(iova_pool, size);

	return iova;
}
EXPORT_SYMBOL(xring_iommu_iova_alloc);

unsigned long xring_iommu_realloc_iova(struct device *dev,
	struct iommu_domain *domain, size_t iova_size)
{
	unsigned long *free_size = NULL;
	unsigned long iova = 0;
	unsigned long pages_num = 0;
	unsigned long i = 0;
	unsigned long size = 0;
	unsigned long j = 0;
	struct xring_iommu_dma_cookie *cookie =
		to_smmu_domain(domain)->iova_cookie;

	might_sleep();
	mutex_lock(&cookie->delay_free->mutex);
	for (i = 0; i < cookie->iova_pool_num; i++) {
		free_size = cookie->free_size[i];
		pages_num = DIV_ROUND_UP(cookie->iova[i].iova_size, PAGE_SIZE);
		for (j = 0; j < pages_num; j++) {
			size = free_size[j] & IOVA_SIZE_MASK;
			if (size == iova_size)
				break;
		}
	}

	if (j >= pages_num && i == cookie->iova_pool_num) {
		mutex_unlock(&cookie->delay_free->mutex);
		dev_info(dev, "%s can't found free iova, pages %lu, size 0x%zx\n",
			__func__, cookie->delay_free->pages, iova_size);
		cookie->delay_free->pages = cookie->delay_free->waterline;
		goto try_alloc;
	}

	free_size[j] = 0;
	mutex_unlock(&cookie->delay_free->mutex);

	xring_iommu_flush_tlb(dev, domain);

	iova = cookie->iova[i].iova_start + ((unsigned long)j << PAGE_SHIFT);
	cookie->delay_free->pages = cookie->delay_free->waterline;

	return iova;

try_alloc:
	iova = xring_iommu_iova_alloc(cookie->iova_pool, iova_size,
		cookie->iova_align);

	return iova;
}
EXPORT_SYMBOL(xring_iommu_realloc_iova);

int xring_smmu_init_domain_mapping(struct device *dev, struct iommu_domain *domain)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct xring_iommu_dma_cookie *cookie = smmu_domain->iova_cookie;

	if (cookie) {
		dev_err(dev, "Iova cookie already present\n");
		return -EINVAL;
	}

	cookie = xring_smmu_create_mapping_sized(dev);
	if (IS_ERR(cookie))
		return -ENOMEM;

	cookie->iova_root = RB_ROOT;
	spin_lock_init(&cookie->lock);

	smmu_domain->iova_cookie = cookie;
	xring_smmu_init_iova_delay_free(dev, cookie);

	return 0;
}
EXPORT_SYMBOL(xring_smmu_init_domain_mapping);

int xring_smmu_set_domain_mapping(struct device *dev, struct iommu_domain *domain)
{
	struct xring_iommu_dma_cookie *fast = NULL;
	struct xring_iommu_dma_cookie *cookie = to_smmu_domain(domain)->iova_cookie;
	int ret;

	if (!cookie) {
		dev_err(dev, "%s, cookie is null!\n", __func__);
		return -ENODEV;
	}

	fast = kzalloc(sizeof(*fast), GFP_KERNEL);
	if (!fast)
		return -ENOMEM;

	if (of_get_iova_info_smmu(dev->of_node, fast, true)) {
		kfree(fast);
		return -ENODEV;
	}

	ret = xring_smmu_set_reserve_pool(cookie, fast->iova, fast->iova_pool_num);
	if (ret) {
		dev_err(dev, "share domain set reverse iova failed\n");
		kfree(fast);
		return -ENODEV;
	}

	kfree(fast);
	return 0;
}
EXPORT_SYMBOL(xring_smmu_set_domain_mapping);

int xring_smmu_iova_debug(struct iommu_domain *domain)
{
	struct xring_iommu_dma_cookie *cookie = to_smmu_domain(domain)->iova_cookie;

	if (!cookie) {
		xring_smmu_err("cookie is null!\n");
		return -ENODEV;
	}

	xring_smmu_printk_iova_genpool(cookie->iova_pool);

	return 0;
}
EXPORT_SYMBOL(xring_smmu_iova_debug);
