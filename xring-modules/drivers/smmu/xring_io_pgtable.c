// SPDX-License-Identifier: GPL-2.0
/*
 * IO_PAGETABLE API for ARM architected SMMUv3 implementations.
 *
 * Copyright (C) 2015 ARM Limited
 *
 * Author: Will Deacon <will.deacon@arm.com>
 *
 * This driver is powered by bad coffee and bombay mix.
 */

#include <linux/io-pgtable-arm.h>
#include <linux/sizes.h>
#include <linux/types.h>
#include <linux/gfp.h>
#include <linux/dma-mapping.h>

#include "xring_io_pgtable.h"
#include "xring_iommu_iova_cookie.h"
#include "xring_smmu.h"

extern bool dumpmapunmap;

static xring_iopte xring_prot_to_pte(struct xring_io_pgtable *data, int prot)
{
	xring_iopte pte = ARM_LPAE_PTE_nG
		| ARM_LPAE_PTE_AF
		| ARM_LPAE_PTE_TYPE_PAGE;

	if (prot & IOMMU_MMIO)
		pte |= (ARM_LPAE_MAIR_ATTR_IDX_DEV
				<< ARM_LPAE_PTE_ATTRINDX_SHIFT);
	if (prot & IOMMU_CACHE)
		pte |= (ARM_LPAE_MAIR_ATTR_IDX_CACHE
				<< ARM_LPAE_PTE_ATTRINDX_SHIFT) | ARM_LPAE_PTE_SH_IS;
	else
		pte |= ARM_LPAE_PTE_SH_OS;

	if (prot & IOMMU_NOEXEC)
		pte |= ARM_LPAE_PTE_XN;

	if (!(prot & IOMMU_WRITE))
		pte |= ARM_LPAE_PTE_AP_RDONLY;

	if (!(prot & IOMMU_PRIV))
		pte |= ARM_LPAE_PTE_AP_UNPRIV;

	return pte;
}

static int xring_io_pgtbable_map(struct io_pgtable_ops *ops, unsigned long iova,
		phys_addr_t paddr, size_t size, int prot)
{
	struct xring_io_pgtable *data = iof_pgtable_ops_to_data(ops);
	unsigned long nptes = size >> PAGE_SHIFT;
	struct page *page = NULL;
	xring_iopte *pmd_ptep = iopte_pmd_offset(data->pmds, data->base, iova);
	xring_iopte *ptep = NULL, *tmp_ptep = NULL;
	xring_iopte pte;
	unsigned long end = iova + size;
	unsigned long pmd_num = GET_PMD_NUM(iova, end);
	unsigned long pte_page_currut = (iova >> PMD_SHIFT) + data->pte_pg_index;
	unsigned long pte_index = iopte_pte_index(iova);
	unsigned long flags;

	if (iova >= (IOVA_DEFAULT_ADDR + IOVA_DEFAULT_SIZE)) {
		xring_io_pgtable_err("map failed! iova error, input iova:0x%lx exceeds 0x%llx\n",
					iova, data->end);
		return -EINVAL;
	}

#ifdef CONFIG_XRING_SMMU_CONTIGUOUS
	unsigned long next = pmd_cont_addr_end(iova, end);
	/* 2m Fill in the page table */
	if (((iova | next | paddr) & ~PMD_MASK) == 0) {
		for (unsigned long i = 0; i < pmd_num; i++) {
			spin_lock_irqsave(&data->lock, flags);
			if (((*(pmd_ptep + i) & ARM_LPAE_PTE_TYPE_MASK) == ARM_LPAE_PTE_TYPE_PAGE) &&
				(data->pages[pte_page_currut + i] != 0)) {
				__free_page(data->pages[pte_page_currut + i]);
				data->pages[pte_page_currut + i] = NULL;
				*(pmd_ptep + i) = 0;
			}
			spin_unlock_irqrestore(&data->lock, flags);
		}

		pte = xring_prot_to_pte(data, prot);
		paddr &= PADDR_MASK;
		pte &= ~ARM_LPAE_PTE_TYPE_MASK;
		pte |= ARM_LPAE_PTE_TYPE_BLOCK;
		if (((iova | next | paddr) & ~CONT_PMD_MASK) == 0)
			pte |= ARM_LPAE_PTE_CONTIGUOUS;

		for (unsigned long i = 0; i < pmd_num; i++, paddr += PMD_SIZE)
			*(pmd_ptep + i) = pte | paddr;

		return 0;
	}
#endif

	/* 4k fill in the page table */
	for (int i = 0; i < pmd_num; i++) {
		if ((*(pmd_ptep + i) != 0) && (data->pages[pte_page_currut + i] != 0))
			continue;
		page = alloc_page(GFP_KERNEL | __GFP_ZERO); //alloc pte pages
		if (!page) {
			xring_io_pgtable_err("i:%d alloc pte memory failed!\n", i);
			return -ENOMEM;
		}
		/* clean cache */
		dma_sync_single_for_device(data->iommu_dev, page_to_phys(page), PAGE_SIZE, DMA_TO_DEVICE);

		spin_lock_irqsave(&data->lock, flags);
		ptep = pmd_ptep + i;
		if (*ptep != 0) {
			spin_unlock_irqrestore(&data->lock, flags);
			__free_page(page);
			continue;
		}
		pte = page_to_phys(page) | ARM_LPAE_PTE_TYPE_PAGE;
		*ptep = pte;
		data->pages[pte_page_currut + i] = page;
		spin_unlock_irqrestore(&data->lock, flags);
	}

	ptep = vmap(&data->pages[pte_page_currut], pmd_num, VM_MAP, pgprot_writecombine(PAGE_KERNEL));

	tmp_ptep = ptep;
	ptep += pte_index;
	pte = xring_prot_to_pte(data, prot);
	paddr &= PADDR_MASK;

#ifdef CONFIG_XRING_SMMU_CONTIGUOUS
	next = pte_cont_addr_end(iova, end);
	if (((iova | next | paddr) & ~CONT_PTE_MASK) == 0)
		pte |= ARM_LPAE_PTE_CONTIGUOUS;
#endif

	for (unsigned long i = 0; i < nptes; i++, paddr += PAGE_SIZE) {
		*(ptep + i) = pte | paddr;
		if (dumpmapunmap)
			if (strstr(dev_name(data->iommu_dev), "media2_smmu")) {
				dev_err(data->iommu_dev, "SMMU TEST LOG, %s, iova:0x%lx, paddr:0x%llx\n"
						"ptep + %lu:%p, *(ptep + %lu):0x%llx\n",
						__func__, iova + i * PAGE_SIZE, paddr, i, ptep + i, i, *(ptep + i));
			}
	}
	dsb(sy);

	vunmap(tmp_ptep);

	return 0;
}

static int xring_io_pgtable_map_pages(struct io_pgtable_ops *ops, unsigned long iova,
		phys_addr_t paddr, size_t pgsize, size_t pgcount,
		int prot, gfp_t gfp, size_t *mapped)
{
	int ret = xring_io_pgtbable_map(ops, iova, paddr, pgsize * pgcount, prot);

	if (!ret)
		*mapped = pgsize * pgcount;

	return ret;
}

static int xring_prepopulate_pgtables(struct xring_io_pgtable *data,
		struct io_pgtable_cfg *cfg, void *cookie)
{
	struct page *page, **pages;
	struct xring_iommu_dma_cookie *iova_cookie = ((struct arm_smmu_domain *)cookie)->iova_cookie;
	unsigned long iova_size = ALIGN(iova_cookie->iova_default_size, PUD_SIZE);
	int pmd_num = iova_size >> PUD_SHIFT;
	int nr_pages = 0, i = 0;
	unsigned long pte_num = pmd_num * NUM_PAGE_ENTRY;
	int pmd_pg_index;
	dma_addr_t iova_start = IOVA_DEFAULT_ADDR;

	pages = kmalloc(sizeof(*pages) * (NUM_PGD_PAGES + pmd_num + pte_num), GFP_KERNEL | __GFP_ZERO);
	if (!pages)
		pages = vmalloc(sizeof(*pages) * (NUM_PGD_PAGES + pmd_num + pte_num));

	if (!pages) {
		xring_io_pgtable_err("alloc page mem failed\n");
		return -ENOMEM;
	}

	/* alloc pgd table memory */
	page = alloc_page(GFP_KERNEL | __GFP_ZERO);
	if (!page) {
		xring_io_pgtable_err("alloc pgd memory failed\n");
		goto out_free_pages_arr;
	}
	/* clean cache */
	dma_sync_single_for_device(cfg->iommu_dev, page_to_phys(page), PAGE_SIZE, DMA_TO_DEVICE);

	pages[nr_pages++] = page;
	pmd_pg_index = nr_pages;

	/* map uncached */
	data->pgd = vmap(&page, 1, VM_MAP, pgprot_writecombine(PAGE_KERNEL));
	if (!data->pgd) {
		xring_io_pgtable_err("map-uncached pgd memory failed\n");
		goto out_free_pages;
	}

	/* alloc pmd memory and remap uncached */
	for (i = iopte_pgd_index(iova_start); i < pmd_num; i++) {
		xring_iopte pte, *ptep;

		page = alloc_page(GFP_KERNEL | __GFP_ZERO);
		if (!page) {
			xring_io_pgtable_err("i:%d alloc pmd memory failed\n", i);
			goto out_unmap_pgd_pages;
		}
		/* clean cache */
		dma_sync_single_for_device(cfg->iommu_dev, page_to_phys(page), PAGE_SIZE, DMA_TO_DEVICE);

		pte = page_to_phys(page) | ARM_LPAE_PTE_TYPE_PAGE;
		ptep = (xring_iopte *)data->pgd + i;
		*ptep = pte;

		pages[nr_pages++] = page;
	}

	data->pmds = vmap(&pages[pmd_pg_index], nr_pages - pmd_pg_index, VM_MAP, pgprot_writecombine(PAGE_KERNEL));
	if (!data->pmds) {
		xring_io_pgtable_err("data->pmds vmap failed!\n");
		goto out_unmap_pgd_pages;
	}

	data->pte_pg_index = nr_pages;
	data->pages = pages;
	data->nr_pages = NUM_PGD_PAGES + pmd_num + pte_num;
	data->base = IOVA_DEFAULT_ADDR;
	data->end = iova_cookie->iova_default_size + IOVA_DEFAULT_ADDR;
	data->iommu_dev = cfg->iommu_dev;
	spin_lock_init(&data->lock);

	return 0;

out_unmap_pgd_pages:
	vunmap(data->pgd);
out_free_pages:
	for (i = 0; i < nr_pages; i++)
		__free_page(pages[i]);
out_free_pages_arr:
	kvfree(pages);
	return -ENOMEM;
}

static int xring_lpae_init_pgtable_s1(struct io_pgtable_cfg *cfg,
		struct xring_io_pgtable *data)
{
	u64 reg;
	typeof(&cfg->arm_lpae_s1_cfg.tcr) tcr = &cfg->arm_lpae_s1_cfg.tcr;

	/* restrict according to the fast map requirements */
	cfg->ias = 39;
	cfg->pgsize_bitmap = SZ_4K;

	if (cfg->quirks & ~(IO_PGTABLE_QUIRK_ARM_NS |
			    IO_PGTABLE_QUIRK_ARM_TTBR1 |
			    IO_PGTABLE_QUIRK_ARM_OUTER_WBWA))
		return -EINVAL;

	/* TCR */
	if (cfg->coherent_walk) {
		tcr->sh = ARM_LPAE_TCR_SH_IS;
		tcr->irgn = ARM_LPAE_TCR_RGN_WBWA;
		tcr->orgn = ARM_LPAE_TCR_RGN_WBWA;
		if (cfg->quirks & IO_PGTABLE_QUIRK_ARM_OUTER_WBWA)
			return -EINVAL;
	} else {
		tcr->sh = ARM_LPAE_TCR_SH_OS;
		tcr->irgn = ARM_LPAE_TCR_RGN_NC;
		if (!(cfg->quirks & IO_PGTABLE_QUIRK_ARM_OUTER_WBWA))
			tcr->orgn = ARM_LPAE_TCR_RGN_NC;
		else
			tcr->orgn = ARM_LPAE_TCR_RGN_WBWA;
	}

	tcr->tg = ARM_LPAE_TCR_TG0_4K;

	switch (cfg->oas) {
	case 32:
		tcr->ips = ARM_LPAE_TCR_PS_32_BIT;
		break;
	case 36:
		tcr->ips = ARM_LPAE_TCR_PS_36_BIT;
		break;
	case 40:
		tcr->ips = ARM_LPAE_TCR_PS_40_BIT;
		break;
	case 42:
		tcr->ips = ARM_LPAE_TCR_PS_42_BIT;
		break;
	case 44:
		tcr->ips = ARM_LPAE_TCR_PS_44_BIT;
		break;
	case 48:
		tcr->ips = ARM_LPAE_TCR_PS_48_BIT;
		break;
	case 52:
		tcr->ips = ARM_LPAE_TCR_PS_52_BIT;
		break;
	default:
		return -EINVAL;
	}

	tcr->tsz = 64ULL - cfg->ias;

	/* MAIRs */
	reg = (ARM_LPAE_MAIR_ATTR_NC
	       << ARM_LPAE_MAIR_ATTR_SHIFT(ARM_LPAE_MAIR_ATTR_IDX_NC)) |
	      (ARM_LPAE_MAIR_ATTR_WBRWA
	       << ARM_LPAE_MAIR_ATTR_SHIFT(ARM_LPAE_MAIR_ATTR_IDX_CACHE)) |
	      (ARM_LPAE_MAIR_ATTR_DEVICE
	       << ARM_LPAE_MAIR_ATTR_SHIFT(ARM_LPAE_MAIR_ATTR_IDX_DEV)) |
	      (ARM_LPAE_MAIR_ATTR_INC_OWBRWA
	       << ARM_LPAE_MAIR_ATTR_SHIFT(ARM_LPAE_MAIR_ATTR_IDX_INC_OCACHE));

	cfg->arm_lpae_s1_cfg.mair = reg;
	return 0;
}

static size_t xring_io_pgtable_unmap_pages(struct io_pgtable_ops *ops, unsigned long iova,
		size_t pgsize, size_t pgcount, struct iommu_iotlb_gather *gather)
{
	struct xring_io_pgtable *data = iof_pgtable_ops_to_data(ops);
	struct io_pgtable *iop = &data->iop;
	size_t size = pgsize * pgcount;
	unsigned long nptes = size >> PAGE_SHIFT;
	struct page **pages = NULL;
	xring_iopte *ptep = NULL, *tmp_ptep = NULL;
	unsigned long end = iova + size;
	unsigned long pmd_num = GET_PMD_NUM(iova, end);
	unsigned long pte_page_currut = (iova >> PMD_SHIFT) + data->pte_pg_index;
	unsigned long pte_index = iopte_pte_index(iova);

	if (iova >= (IOVA_DEFAULT_ADDR + IOVA_DEFAULT_SIZE)) {
		xring_io_pgtable_err("map failed! iova error, input iova:0x%lx exceeds 0x%llx\n",
					iova, data->end);
		return -EINVAL;
	}

	if (dumpmapunmap)
		if (strstr(dev_name(data->iommu_dev), "media2_smmu")) {
			dev_err(data->iommu_dev, "SMMU TEST LOG, %s, iova:0x%lx, size:0x%zx\n",
					__func__, iova, size);
		}

#ifdef CONFIG_XRING_SMMU_CONTIGUOUS
	xring_iopte *pmd_ptep = iopte_pmd_offset(data->pmds, data->base, iova);
	unsigned long next = pmd_cont_addr_end(iova, end);
	phys_addr_t paddr = (*pmd_ptep) & PADDR_MASK;
	/* 2m clear the page table */
	if (((iova | next | paddr) & ~PMD_MASK) == 0) {
		memset(pmd_ptep, 0, sizeof(*pmd_ptep) * pmd_num);
		/* flush tlb */
		io_pgtable_tlb_flush_all(iop);
		return size;
	}

	if (((*pmd_ptep) & ARM_LPAE_PTE_TYPE_MASK) == ARM_LPAE_PTE_TYPE_BLOCK) {
		memset(pmd_ptep, 0, sizeof(*pmd_ptep) * 1);
		/* flush tlb */
		io_pgtable_tlb_flush_all(iop);
		return PMD_SIZE;
	}
#endif

	/* 4k clear the page table */
	for (unsigned long i = 0; i < pmd_num; i++)
		if (WARN_ON(!data->pages[pte_page_currut + i])) {
			xring_io_pgtable_err("page is null\n"
					"data->pte_pg_index:%d, i:0x%lx, pmd_num:%lu, iova:0x%lx, pte_index:%lu\n"
					"pte_page_currut:%lu, size:0x%zx, nptes:0x%lx\n",
					data->pte_pg_index, i, pmd_num, iova, pte_index,
					pte_page_currut, size, nptes);
			return 0;
		}

	pages = &data->pages[pte_page_currut];
	ptep = vmap(pages, pmd_num, VM_MAP, pgprot_writecombine(PAGE_KERNEL));
	if (!ptep) {
		xring_io_pgtable_err("ptep vmap failed\n"
				"data->pte_pg_index:%d, pmd_num:%lu, iova:0x%lx, pte_index:%lu\n"
				"pte_page_currut:%lu, size:0x%zx, nptes:0x%lx, pages:%p, ptep:%p\n",
				data->pte_pg_index, pmd_num, iova, pte_index,
				pte_page_currut, size, nptes, pages, ptep);
		return 0;
	}

	tmp_ptep = ptep;
	ptep += pte_index;
	memset(ptep, 0, sizeof(*ptep) * nptes);
	dsb(sy);
	vunmap(tmp_ptep);

	/* flush tlb */
	io_pgtable_tlb_flush_all(iop);

	return size;
}

static phys_addr_t xring_io_pgtable_iova_to_phys(struct io_pgtable_ops *ops,
				  unsigned long iova)
{
	struct xring_io_pgtable *data = iof_pgtable_ops_to_data(ops);
	xring_iopte *ptep = NULL;
	unsigned long pte_page_currut = (iova >> PMD_SHIFT) + data->pte_pg_index;
	unsigned long pte_index = iopte_pte_index(iova);
	phys_addr_t phys = 0;

	if (iova >= data->end) {
		xring_io_pgtable_err("map failed! iova error, input iova:0x%lx exceeds 0x%llx\n",
					iova, data->end);
		return -EINVAL;
	}

#ifdef CONFIG_XRING_SMMU_CONTIGUOUS
	/* 2m map */
	xring_iopte *pmd_ptep = iopte_pmd_offset(data->pmds, data->base, iova);

	phys = (*pmd_ptep);
	if (((*pmd_ptep) & ARM_LPAE_PTE_TYPE_MASK) == ARM_LPAE_PTE_TYPE_BLOCK)
		return phys & PADDR_MASK;
#endif

	/* 4k map */
	ptep = vmap(&data->pages[pte_page_currut], 1, VM_MAP, pgprot_writecombine(PAGE_KERNEL));
	if (!ptep) {
		xring_io_pgtable_err("ptep vmap failed\n"
				"iova:0x%lx, pte_page_currut:0x%lx\n"
				" data->pte_pg_index:%d, pte_index:%lu\n"
				" data->pages[pte_page_currut]:%llx, ptep:%llx\n",
				iova, pte_page_currut,
				data->pte_pg_index, pte_index,
				(uint64_t)data->pages[pte_page_currut], (uint64_t)ptep);
		return 0;
	}
	phys = *(ptep + pte_index);
	vunmap(ptep);

	return phys & PADDR_MASK;
}

static struct io_pgtable *xring_io_pgtable_alloc(struct io_pgtable_cfg *cfg, void *cookie)
{
	struct xring_io_pgtable *data;
	int pg_shift;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return NULL;

	if (xring_lpae_init_pgtable_s1(cfg, data))
		goto out_free_data;

	/* alloc all page table memory */
	if (xring_prepopulate_pgtables(data, cfg, cookie))
		goto out_free_data;

	pg_shift = __ffs(cfg->pgsize_bitmap);
	data->bits_per_level = pg_shift - ilog2(sizeof(arm_lpae_iopte));

	data->iop.ops = (struct io_pgtable_ops) {
		.map_pages = xring_io_pgtable_map_pages,
		.unmap_pages = xring_io_pgtable_unmap_pages,
		.iova_to_phys = xring_io_pgtable_iova_to_phys,
		// .unmap_pages_walk = xring_io_pgtable_unmap_pages_walk,
	};

	/* Ensure the empty pgd is visible before any actual TTBR write */
	wmb();

	/* TTBR */
	cfg->arm_lpae_s1_cfg.ttbr = page_to_phys(data->pages[0]);
	return &data->iop;

out_free_data:
	kfree(data);
	return NULL;
}

static void xring_io_pgtable_free(struct io_pgtable *iop)
{
	int i;
	struct xring_io_pgtable *data = iof_pgtable_to_data(iop);

	vunmap(data->pgd);
	vunmap(data->pmds);
	for (i = 0; i < data->nr_pages; i++) {
		if (data->pages[i])
			__free_page(data->pages[i]);
	}
	kvfree(data->pages);
	kfree(data);
}

static struct io_pgtable_init_fns xring_io_pgtable_arm_64_s1_init_fns = {
	.alloc = xring_io_pgtable_alloc,
	.free = xring_io_pgtable_free,
};

struct io_pgtable_ops *xring_alloc_io_pgtable_ops(enum io_pgtable_fmt fmt,
		struct io_pgtable_cfg *cfg, void *cookie)
{
	struct io_pgtable *iop;
	const struct io_pgtable_init_fns *fns;

	if (fmt < IO_PGTABLE_NUM_FMTS)
		return alloc_io_pgtable_ops(fmt, cfg, cookie);
	else if (fmt == XRING_ARM_64_LPAE_S1)
		fns = &xring_io_pgtable_arm_64_s1_init_fns;
	else {
		xring_io_pgtable_err("Invalid io-pgtable fmt:%d\n", fmt);
		return NULL;
	}

	iop = fns->alloc(cfg, cookie);
	if (!iop)
		return NULL;

	iop->fmt = fmt;
	iop->cookie = cookie;
	iop->cfg = *cfg;

	return &iop->ops;
}
