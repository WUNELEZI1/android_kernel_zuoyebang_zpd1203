// SPDX-License-Identifier: GPL-2.0
/*
 * XRING Page Table implementation
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/swap.h>
#include <linux/gfp.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <linux/pgtable.h>
#include <soc/xring/xring_mem_adapter.h>
#include <linux/android_debug_symbols.h>

#include "xring_mem_adapter.h"

static bool xr_pgattr_change_is_safe(u64 old, u64 new)
{
	pteval_t mask = PTE_PXN | PTE_RDONLY | PTE_WRITE | PTE_NG;

	/* creating or taking down mappings is always safe */
	if (old == 0 || new == 0)
		return true;

	/* live contiguous mappings may not be manipulated at all */
	if ((old | new) & PTE_CONT)
		return false;

	/* Transitioning from Non-Global to Global is unsafe */
	if (old & ~new & PTE_NG)
		return false;

	if (((old & PTE_ATTRINDX_MASK) == PTE_ATTRINDX(MT_NORMAL) ||
			(old & PTE_ATTRINDX_MASK) == PTE_ATTRINDX(MT_NORMAL_TAGGED)) &&
		    ((new & PTE_ATTRINDX_MASK) == PTE_ATTRINDX(MT_NORMAL) ||
			(new & PTE_ATTRINDX_MASK) == PTE_ATTRINDX(MT_NORMAL_TAGGED)))
		mask |= PTE_ATTRINDX_MASK;

	return ((old ^ new) & ~mask) == 0;
}

static void _set_pmd(pmd_t *pmdp, pmd_t pmd)
{
	WRITE_ONCE(*pmdp, pmd);

	if (pmd_valid(pmd)) {
		dsb(ishst);
		isb();
	}
}

static int _pmd_set_huge(pmd_t *pmdp, phys_addr_t phys, pgprot_t prot)
{
	pmd_t new_pmd = pfn_pmd(__phys_to_pfn(phys), mk_pmd_sect_prot(prot));

	if (!xr_pgattr_change_is_safe(READ_ONCE(pmd_val(*pmdp)),
				pmd_val(new_pmd)))
		return 0;

	VM_BUG_ON(phys & ~PMD_MASK);
	_set_pmd(pmdp, new_pmd);
	return -1;
}

static inline void _pmd_populate(pmd_t *pmdp, phys_addr_t ptep,
				pmdval_t prot)
{
	_set_pmd(pmdp, __pmd(__phys_to_pmd_val(ptep) | prot));
}

static void _set_pud(pud_t *pudp, pud_t pud)
{
	WRITE_ONCE(*pudp, pud);

	if (pud_valid(pud)) {
		dsb(ishst);
		isb();
	}
}

static int _pud_set_huge(pud_t *pudp, phys_addr_t phys, pgprot_t prot)
{
	pud_t new_pud = pfn_pud(__phys_to_pfn(phys), mk_pud_sect_prot(prot));

	/* Only allow permission changes for now */
	if (!xr_pgattr_change_is_safe(READ_ONCE(pud_val(*pudp)),
				pud_val(new_pud)))
		return 0;

	VM_BUG_ON(phys & ~PUD_MASK);
	_set_pud(pudp, new_pud);
	return -1;
}

static inline void _pud_populate(pud_t *pudp, phys_addr_t pmdp, pudval_t prot)
{
	_set_pud(pudp, __pud(__phys_to_pud_val(pmdp) | prot));
}

static void change_pte_range(pmd_t *pmd, unsigned long phys,
			unsigned long addr, unsigned long end, pgprot_t prot)
{
	pte_t *pte = NULL;
	unsigned long pfn = __phys_to_pfn(phys);
	struct mm_struct *init_mm = (struct mm_struct *)android_debug_symbol(ADS_INIT_MM);

	if (pmd_none(*pmd) || pmd_sect(*pmd)) {
		void *pte_ptr = NULL;
		phys_addr_t pte_phys;

		spin_unlock(&init_mm->page_table_lock);
		pte_ptr = (void *)__get_free_page(GFP_KERNEL | __GFP_ZERO);
		pte_phys = __pa(pte_ptr);
		spin_lock(&init_mm->page_table_lock);

		if (pmd_sect(*pmd)) {
			unsigned long pfn = pmd_pfn(*pmd);
			int i = 0;

			do {
				set_pte(pte_ptr, pfn_pte(pfn, PAGE_KERNEL));
				pfn++;
				pte_ptr++;
				i++;
			} while (i < PTRS_PER_PTE);
		}
		_pmd_populate(pmd, pte_phys, PMD_TYPE_TABLE);
		flush_tlb_all();
	}

	pte = pte_offset_kernel(pmd, addr);

	do {
		set_pte(pte, pfn_pte(pfn, prot));
		phys += PAGE_SIZE;
		pfn++;
		pte++;
		addr += PAGE_SIZE;
	} while (addr != end);
}

static void change_pmd_range(pud_t *pud, unsigned long phys,
			unsigned long addr, unsigned long end, pgprot_t prot)
{
	pmd_t *pmd = NULL;
	unsigned long next;
	struct mm_struct *init_mm = (struct mm_struct *)android_debug_symbol(ADS_INIT_MM);

	if (pud_none(*pud) || pud_sect(*pud)) {
		void *pmd_ptr = NULL;
		phys_addr_t pmd_phys;

		spin_unlock(&init_mm->page_table_lock);
		pmd_ptr = (void *)__get_free_page(GFP_KERNEL | __GFP_ZERO);
		pmd_phys = __pa(pmd_ptr);
		spin_lock(&init_mm->page_table_lock);

		if (pud_sect(*pud)) {
			unsigned long addr = pud_pfn(*pud) << PAGE_SHIFT;
			pgprot_t prot = __pgprot(pud_val(*pud) ^ addr);
			int i = 0;

			do {
				_set_pmd(pmd_ptr, __pmd(addr | pgprot_val(prot)));
				addr += PMD_SIZE;
				pmd_ptr++;
				i++;
			} while (i < PTRS_PER_PMD);
		}
		_pud_populate(pud, pmd_phys, PUD_TYPE_TABLE);
		flush_tlb_all();
	}

	pmd = pmd_offset(pud, addr);

	do {
		next = pmd_addr_end(addr, end);
		if (((addr | next | phys) & ~PMD_MASK) == 0 && pmd_sect(*pmd))
			_pmd_set_huge(pmd, phys, prot);
		else
			change_pte_range(pmd, phys, addr, next, prot);

		phys += next - addr;
		pmd++;
		addr = next;
	} while (addr != end);
}

static void change_pud_range(pgd_t *pgd, unsigned long phys,
			unsigned long addr, unsigned long end, pgprot_t prot)
{
	p4d_t *p4d;
	pud_t *pud;
	unsigned long next;

	p4d = p4d_offset(pgd, addr);
	pud = pud_offset(p4d, addr);

	do {
		next = pud_addr_end(addr, end);
		if (((addr | next | phys) & ~PUD_MASK) == 0 && pud_sect(*pud))
			_pud_set_huge(pud, phys, prot);
		else
			change_pmd_range(pud, phys, addr, next, prot);

		phys += next - addr;
		pud++;
		addr = next;
	} while (addr != end);

}

void xring_change_secpage_range(phys_addr_t phys, unsigned long addr,
				unsigned long size, pgprot_t prot)
{
	pgd_t *pgd = NULL;
	unsigned long next;
	unsigned long end = addr + size;
	struct mm_struct *init_mm = (struct mm_struct *)android_debug_symbol(ADS_INIT_MM);

	spin_lock(&init_mm->page_table_lock);

	pgd = pgd_offset(init_mm, addr);
	do {
		next = pgd_addr_end(addr, end);
		change_pud_range(pgd, phys, addr, next, prot);
		phys += next - addr;
		pgd++;
		addr = next;
	} while (addr != end);

	spin_unlock(&init_mm->page_table_lock);
}
EXPORT_SYMBOL(xring_change_secpage_range);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring MEM ADAPTER Driver");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(MINIDUMP);
