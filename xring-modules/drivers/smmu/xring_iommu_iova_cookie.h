/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * X-Ring IOMMU IOVA manager.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __LINUX_XRING_IOMMU_IOVA_COOKIE_H
#define __LINUX_XRING_IOMMU_IOVA_COOKIE_H

#include <linux/genalloc.h>
#include <linux/iommu.h>

#define IOVA_DEFAULT_ADDR			0x1000
#define IOVA_DEFAULT_SIZE			0x1FFFFF000
#define IOVA_LIMIT_DEFAULT_SIZE			0xFFFFF000
#define IOVA_DEFAULT_PARA_NUM			2
#define DELAYED_FREE_WATERLINE			10
#define PINGPONG_SHIFT				35
#define PINGPONG_MASK				(1UL << PINGPONG_SHIFT)
#define IOVA_SIZE_MASK				(PINGPONG_MASK - 1)
#define WATCHDOG_CPU				0
#define WIFI_INTX_CPU				4
#define DELAYED_FREE_SCHED_PRI			97
#define IOVA_WATERLINE_PARA			100

enum iova_free_type {
	IMME_FREE, /* Immediately free */
	DELAYED_FREE, /* Delayed free */
};

struct xring_iommu_domain_data {
	unsigned long				iova_start;
	unsigned long				iova_size;
};

struct xring_iommu_iova_delay_free {
	unsigned long				pingpong;
	unsigned long				pages;
	unsigned long				waterline;
	bool					end;
	wait_queue_head_t			wait_q;
	struct task_struct			*task;
	struct mutex				mutex;
	spinlock_t				lock;
};

#define IOVA_MAX_NUM				5

#ifdef CONFIG_XRING_IOMMU_MAP
struct xring_iommu_dma_cookie {
	struct rb_root				iova_root;
	struct xring_iommu_domain_data		iova[IOVA_MAX_NUM];
	unsigned long				iova_pool_num;
	unsigned long				iova_align;
	unsigned long				iova_free;
	unsigned long				iova_total_size;
	unsigned long				iova_default_size;

	spinlock_t				lock;
	struct gen_pool				*iova_pool;
	struct xring_iommu_iova_delay_free	*delay_free;
	unsigned long				*free_size[IOVA_MAX_NUM];
};

int xring_smmu_init_domain_mapping(struct device *dev, struct iommu_domain *domain);
int xring_smmu_set_domain_mapping(struct device *dev, struct iommu_domain *domain);

unsigned long xring_iommu_iova_alloc(struct gen_pool *iova_pool,
		size_t size, unsigned long align);
unsigned long xring_iommu_realloc_iova(struct device *dev,
		struct iommu_domain *domain, size_t iova_size);
void xring_iommu_iova_free_imme(struct gen_pool *iova_pool,
		unsigned long iova, size_t size);
void xring_iommu_iova_free_delay(struct xring_iommu_dma_cookie *mapping,
		unsigned long iova, size_t size);
int xring_smmu_iova_debug(struct iommu_domain *domain);
#else
struct xring_iommu_dma_cookie {};
int xring_smmu_init_domain_mapping(struct device *dev, struct iommu_domain *domain)
{
	return 0;
}
int xring_smmu_set_domain_mapping(struct device *dev, struct iommu_domain *domain)
{
	return 0;
}
unsigned long xring_iommu_iova_alloc(struct gen_pool *iova_pool,
		size_t size, unsigned long align)
{
	return 0;
}
unsigned long xring_iommu_realloc_iova(struct device *dev,
		struct iommu_domain *domain, size_t iova_size)
{
	return 0;
}
void xring_iommu_iova_free_imme(struct gen_pool *iova_pool,
		unsigned long iova, size_t size)
{
}
void xring_iommu_iova_free_delay(struct xring_iommu_dma_cookie *mapping,
		unsigned long iova, size_t size)
{
}
int xring_smmu_iova_debug(struct iommu_domain *domain)
{
	return 0;
}
#endif

#endif /* __LINUX_XRING_IOMMU_IOVA_COOKIE_H */
