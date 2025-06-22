/* SPDX-License-Identifier: GPL-2.0 */
/*
 * X-Ring IO-PGTBALE-API to IOMMU-API.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __XRING_IO_PGTABLE_H
#define __XRING_IO_PGTABLE_H

#include <linux/io-pgtable.h>
#include <linux/vmalloc.h>

#define xring_io_pgtable_err(fmt, ...) \
	pr_err("[SMMU] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define xring_io_pgtable_warn(fmt, ...) \
	pr_warn("[SMMU] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define xring_io_pgtable_info(fmt, ...) \
	pr_info("[SMMU] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define xring_io_pgtable_debug(fmt, ...) \
	pr_debug("[SMMU] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define XRING_ARM_64_LPAE_S1 ((unsigned int)-1)
#define XRING_ARM_64_LPAE_S2 ((unsigned int)-2)
#define ARM_LPAE_PTE_CONTIGUOUS	 (((arm_lpae_iopte)1) << 52)

typedef u64 xring_iopte;

#define NUM_PGD_PAGES 1
#define NUM_PMD_PAGES 8
#define NUM_PAGE_ENTRY 512

struct xring_io_pgtable {
	struct io_pgtable	iop;
	xring_iopte		*pgd;
	xring_iopte		*pmds;
	int			bits_per_level;
	int			pte_pg_index;
	struct page		**pages;
	unsigned long		nr_pages;
	dma_addr_t		base;
	dma_addr_t		end;
	struct device		*iommu_dev;
	spinlock_t		lock;
};

/* Struct accessors */
#define iof_pgtable_to_data(x)						\
	container_of((x), struct xring_io_pgtable, iop)

#define iof_pgtable_ops_to_pgtable(x)					\
	container_of((x), struct io_pgtable, ops)

#define iof_pgtable_ops_to_data(x)					\
	iof_pgtable_to_data(iof_pgtable_ops_to_pgtable(x))

#define iopte_pgd_index(iova)	((iova >> PUD_SHIFT) & 0x7)
#define iopte_pmd_index(iova)	((iova >> PMD_SHIFT) & 0x1ff)
#define iopte_pte_index(iova)	((iova >> PAGE_SHIFT) & 0x1ff)
#define PGTABLE_MASK		(~((xring_iopte)(1 << 2) - 1))
#define PADDR_MASK		(~((xring_iopte)(1 << PAGE_SHIFT) - 1))
#define iopte_pmd_offset(pmds, base, iova)	 (pmds + (iova >> PMD_SHIFT))
#define GET_PMD_NUM(addr, end)		\
({	\
	unsigned long __pmd_num = 0, __next_addr = 0, __start_addr = addr;	\
	do {							\
		__pmd_num++;					\
		__next_addr = pmd_addr_end(__start_addr, end);	\
	} while (__start_addr = __next_addr, __start_addr < end);	\
	__pmd_num ? __pmd_num : 0;	\
})

struct io_pgtable_ops *xring_alloc_io_pgtable_ops(enum io_pgtable_fmt fmt,
		struct io_pgtable_cfg *cfg, void *cookie);

#endif /* __XRING_IO_PGTABLE_H */
