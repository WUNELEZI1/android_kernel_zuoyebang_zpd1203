// SPDX-License-Identifier: GPL-2.0-only
/*
 * HugeTLB for DMA-BUF
 *
 * Copyright (c) Xiaomi Technologies Co., Ltd. 2023. All rights reserved.
 */

#include <linux/mm.h>
#include <asm/pgalloc.h>
#include <asm/tlb.h>
#include <asm/tlbflush.h>
#include <linux/hugetlb_dmabuf.h>
#include <linux/hugetlb.h>
#include <linux/pfn_t.h>
#include <linux/mm_inline.h>
#include <linux/mmu_notifier.h>
#include <linux/security.h>
#include <linux/mman.h>

#include "internal.h"

#define DMABUF_64K_SHIFT		16
#define DMABUF_64K_SIZE			(1UL << DMABUF_64K_SHIFT)
#define DMABUF_64K_MASK			(~(DMABUF_64K_SIZE-1))
#define DMABUF_PMD_MAP			0

atomic_long_t dmabuf_hugetlb_pmd_map = ATOMIC_LONG_INIT(0);
EXPORT_SYMBOL(dmabuf_hugetlb_pmd_map);
atomic_long_t dmabuf_hugetlb_contpte_map = ATOMIC_LONG_INIT(0);
EXPORT_SYMBOL(dmabuf_hugetlb_contpte_map);
atomic_long_t dmabuf_hugetlb_pmd_zap = ATOMIC_LONG_INIT(0);
EXPORT_SYMBOL(dmabuf_hugetlb_pmd_zap);
atomic_long_t dmabuf_hugetlb_pmd_split = ATOMIC_LONG_INIT(0);
EXPORT_SYMBOL(dmabuf_hugetlb_pmd_split);

static int dmabuf_huge_remap_pte_range(struct mm_struct *mm, pmd_t *pmd,
			unsigned long addr, unsigned long end,
			unsigned long pfn, pgprot_t prot)
{
	pte_t *pte, *mapped_pte;
	spinlock_t *ptl;
	unsigned int nr_pages;
	int err = 0;

	mapped_pte = pte = pte_alloc_map_lock(mm, pmd, addr, &ptl);
	if (!pte)
		return -ENOMEM;
	arch_enter_lazy_mmu_mode();

	BUG_ON(!pte_none(*pte));
	if (!pfn_modify_allowed(pfn, prot)) {
		err = -EACCES;
		goto out;
	}

	nr_pages = (end - addr) >> PAGE_SHIFT;
	set_ptes(mm, addr, pte, pte_mkspecial(pfn_pte(pfn, prot)), nr_pages);
	atomic_long_inc(&dmabuf_hugetlb_contpte_map);

out:
	arch_leave_lazy_mmu_mode();
	pte_unmap_unlock(mapped_pte, ptl);
	return err;
}

static inline int dmabuf_huge_remap_pmd_range(struct mm_struct *mm, pud_t *pud,
			unsigned long addr, unsigned long end,
			unsigned long pfn, pgprot_t prot, unsigned int map_type)
{
	pmd_t *pmd;
	spinlock_t *ptl;
	pgtable_t pgtable;
	unsigned long next;
	int err = 0;

	pfn -= addr >> PAGE_SHIFT;
	pmd = pmd_alloc(mm, pud, addr);
	if (!pmd)
		return -ENOMEM;

	do {
		if (map_type == DMABUF_PMD_MAP) {
			pgtable = pte_alloc_one(mm);
			if (unlikely(!pgtable)) {
				return -ENOMEM;
			}

			ptl = pmd_lock(mm, pmd);
			pgtable_trans_huge_deposit(mm, pmd, pgtable);
			mm_inc_nr_ptes(mm);
			next = pmd_addr_end(addr, end);
			pmd_set_huge(pmd, ((pfn + (addr >> PAGE_SHIFT)) << PAGE_SHIFT), prot);
			spin_unlock(ptl);
			atomic_long_inc(&dmabuf_hugetlb_pmd_map);
		} else {
			next = pmd_addr_end(addr, end);
			err = dmabuf_huge_remap_pte_range(mm, pmd, addr, next,
					pfn + (addr >> PAGE_SHIFT), prot);
			if (err)
				return err;
		}
	} while (pmd++, addr = next, addr != end);
	return 0;
}

static inline int dmabuf_huge_remap_pud_range(struct mm_struct *mm, p4d_t *p4d,
			unsigned long addr, unsigned long end,
			unsigned long pfn, pgprot_t prot, unsigned int map_type)
{
	pud_t *pud;
	unsigned long next;
	int err;

	pfn -= addr >> PAGE_SHIFT;
	pud = pud_alloc(mm, p4d, addr);
	if (!pud)
		return -ENOMEM;
	do {
		next = pud_addr_end(addr, end);
		err = dmabuf_huge_remap_pmd_range(mm, pud, addr, next,
				pfn + (addr >> PAGE_SHIFT), prot, map_type);
		if (err)
			return err;
	} while (pud++, addr = next, addr != end);
	return 0;
}

static inline int dmabuf_huge_remap_p4d_range(struct mm_struct *mm, pgd_t *pgd,
			unsigned long addr, unsigned long end,
			unsigned long pfn, pgprot_t prot, unsigned int map_type)
{
	p4d_t *p4d;
	unsigned long next;
	int err;

	pfn -= addr >> PAGE_SHIFT;
	p4d = p4d_alloc(mm, pgd, addr);
	if (!p4d)
		return -ENOMEM;
	do {
		next = p4d_addr_end(addr, end);
		err = dmabuf_huge_remap_pud_range(mm, p4d, addr, next,
				pfn + (addr >> PAGE_SHIFT), prot, map_type);
		if (err)
			return err;
	} while (p4d++, addr = next, addr != end);
	return 0;
}

/*
 * Variant of remap_pfn_range that does not call track_pfn_remap.  The caller
 * must have pre-validated the caching bits of the pgprot_t.
 */
static int dmabuf_huge_remap_pfn_range_notrack(struct vm_area_struct *vma, unsigned long addr,
		unsigned long pfn, unsigned long size, pgprot_t prot, unsigned int map_type)
{
	pgd_t *pgd;
	unsigned long next;
	unsigned long end = addr + PAGE_ALIGN(size);
	struct mm_struct *mm = vma->vm_mm;
	int err;

	if (WARN_ON_ONCE(!PAGE_ALIGNED(addr)))
		return -EINVAL;

	/*
	 * Physically remapped pages are special. Tell the
	 * rest of the world about it:
	 *   VM_IO tells people not to look at these pages
	 *	(accesses can have side effects).
	 *   VM_PFNMAP tells the core MM that the base pages are just
	 *	raw PFN mappings, and do not have a "struct page" associated
	 *	with them.
	 *   VM_DONTEXPAND
	 *      Disable vma merging and expanding with mremap().
	 *   VM_DONTDUMP
	 *      Omit vma from core dump, even when VM_IO turned off.
	 *
	 * There's a horrible special case to handle copy-on-write
	 * behaviour that some programs depend on. We mark the "original"
	 * un-COW'ed pages by matching them up with "vma->vm_pgoff".
	 * See vm_normal_page() for details.
	 */

	if (is_cow_mapping(vma->vm_flags)) {
		if (addr != vma->vm_start || end != vma->vm_end)
			return -EINVAL;
		vma->vm_pgoff = pfn;
	}

	vm_flags_set(vma, VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP);
	if (map_type == DMABUF_PMD_MAP)
		vm_flags_set(vma, VM_DMABUF_HUGETLB);

	BUG_ON(addr >= end);
	pfn -= addr >> PAGE_SHIFT;
	pgd = pgd_offset(mm, addr);
	flush_cache_range(vma, addr, end);
	do {
		next = pgd_addr_end(addr, end);
		err = dmabuf_huge_remap_p4d_range(mm, pgd, addr, next,
				pfn + (addr >> PAGE_SHIFT), prot, map_type);
		if (err)
			return err;
	} while (pgd++, addr = next, addr != end);

	return 0;
}

int dmabuf_huge_remap_pfn_range(struct vm_area_struct *vma, unsigned long addr,
		    unsigned long pfn, unsigned long size, pgprot_t prot, unsigned int map_type)
{
	int err;

	err = track_pfn_remap(vma, &prot, pfn, addr, PAGE_ALIGN(size));
	if (err)
		return -EINVAL;

	err = dmabuf_huge_remap_pfn_range_notrack(vma, addr, pfn, size, prot, map_type);
	if (err)
		untrack_pfn(vma, pfn, PAGE_ALIGN(size), true);
	return err;
}
EXPORT_SYMBOL(dmabuf_huge_remap_pfn_range);

static void __split_dmabuf_huge_pmd_locked(struct vm_area_struct *vma, pmd_t *pmd,
		unsigned long haddr, bool freeze)
{
	struct mm_struct *mm = vma->vm_mm;
	pgtable_t pgtable;
	pmd_t old_pmd, _pmd;
	bool young, write, soft_dirty, uffd_wp = false, dirty = false;
	unsigned long addr;
	unsigned long pfn;
	int i;

	VM_BUG_ON(haddr & ~PMD_MASK);
	VM_BUG_ON_VMA(vma->vm_start > haddr, vma);
	VM_BUG_ON_VMA(vma->vm_end < haddr + PMD_SIZE, vma);
	VM_BUG_ON(!pmd_dmabuf_huge(*pmd));

	pgtable = pgtable_trans_huge_withdraw(mm, pmd);
	old_pmd = pmdp_invalidate(vma, haddr, pmd);

	pfn = pmd_pfn(old_pmd);

	if (pmd_dirty(old_pmd)) {
		dirty = true;
	}
	write = pmd_write(old_pmd);
	young = pmd_young(old_pmd);
	soft_dirty = pmd_soft_dirty(old_pmd);
	uffd_wp = pmd_uffd_wp(old_pmd);

	pmd_populate(mm, &_pmd, pgtable);

	for (i = 0, addr = haddr; i < PMD_SIZE/PAGE_SIZE; i++, addr += PAGE_SIZE) {
		pte_t entry, *pte;

		entry = pfn_pte(pfn + i, READ_ONCE(vma->vm_page_prot));
		if (write)
			entry = pte_mkwrite(entry, vma);
		if (!young)
			entry = pte_mkold(entry);
		/* NOTE: this may set soft-dirty too on some archs */
		if (dirty)
			entry = pte_mkdirty(entry);
		if (soft_dirty)
			entry = pte_mksoft_dirty(entry);
		if (uffd_wp)
			entry = pte_mkuffd_wp(entry);

		pte = pte_offset_kernel(&_pmd, addr);
		BUG_ON(!pte_none(*pte));
		set_pte_at(mm, addr, pte, pte_mkspecial(entry));
	}

	smp_wmb(); /* make pte visible before pmd */
	pmd_populate(mm, pmd, pgtable);
	atomic_long_inc(&dmabuf_hugetlb_pmd_split);
}

void __split_dmabuf_huge_pmd(struct vm_area_struct *vma, pmd_t *pmd,
		unsigned long address, bool freeze, struct folio *folio)
{
	spinlock_t *ptl;
	struct mmu_notifier_range range;

	mmu_notifier_range_init(&range, MMU_NOTIFY_CLEAR, 0, vma->vm_mm,
				address & PMD_MASK,
				(address & PMD_MASK) + PMD_SIZE);
	mmu_notifier_invalidate_range_start(&range);

	ptl = pmd_lock(vma->vm_mm, pmd);

	/*
	 * If caller asks to setup a migration entries, we need a page to check
	 * pmd against. Otherwise we can end up replacing wrong page.
	 */
	VM_BUG_ON(freeze && !folio);
	VM_WARN_ON_ONCE(folio && !folio_test_locked(folio));

	if (pmd_dmabuf_huge(*pmd)) {
		/*
		 * It's safe to call pmd_page when folio is set because it's
		 * guaranteed that pmd is present.
		 */
		if (folio && folio != page_folio(pmd_page(*pmd)))
			goto out;
		__split_dmabuf_huge_pmd_locked(vma, pmd, range.start, freeze);
	}

out:
	spin_unlock(ptl);
	mmu_notifier_invalidate_range_end(&range);
}

void zap_split_dmabuf_huge_pmd(struct vm_area_struct *vma, pmd_t *pmd,
		unsigned long address, bool freeze, struct folio *folio)
{
	__split_dmabuf_huge_pmd(vma, pmd, address, freeze, folio);
}

void split_dmabuf_huge_pmd_address(struct vm_area_struct *vma, unsigned long address,
		bool freeze, struct folio *folio)
{
	pmd_t *pmd = mm_find_pmd(vma->vm_mm, address);

	if (!pmd)
		return;

	__split_dmabuf_huge_pmd(vma, pmd, address, freeze, folio);
}

static inline void split_dmabuf_huge_pmd_if_needed(struct vm_area_struct *vma,
				unsigned long address)
{
	/*
	 * If the new address isn't hpage aligned and it could previously
	 * contain an hugepage: check if we need to split an huge pmd.
	 */
	if (!IS_ALIGNED(address, PMD_SIZE) &&
	    range_in_vma(vma, ALIGN_DOWN(address, PMD_SIZE),
			 ALIGN(address, PMD_SIZE)))
		split_dmabuf_huge_pmd_address(vma, address, false, NULL);
}

void vma_adjust_dmabuf_huge(struct vm_area_struct *vma,
			     unsigned long start,
			     unsigned long end,
			     long adjust_next)
{
	/* Check if we need to split start first. */
	split_dmabuf_huge_pmd_if_needed(vma, start);

	/* Check if we need to split end next. */
	split_dmabuf_huge_pmd_if_needed(vma, end);

	/*
	 * If we're also updating the vma->vm_next->vm_start,
	 * check if we need to split it.
	 */
	if (adjust_next > 0) {
		struct vm_area_struct *next = find_vma(vma->vm_mm, vma->vm_end);
		unsigned long nstart = next->vm_start;
		nstart += adjust_next;
		split_dmabuf_huge_pmd_if_needed(next, nstart);
	}
}

static inline unsigned long split_dmabuf_huge_pmd_range(struct vm_area_struct *vma, pud_t *pud,
				unsigned long addr, unsigned long end)
{
	pmd_t *pmd;
	unsigned long next;

	BUG_ON(!is_vm_dmabuf_hugetlb_page(vma));

	pmd = pmd_offset(pud, addr);
	do {
		next = pmd_addr_end(addr, end);
		if(pmd_dmabuf_huge(*pmd))
			__split_dmabuf_huge_pmd(vma, pmd, addr, false, NULL);
	} while (pmd++, addr = next, addr != end);

	return addr;
}

static inline unsigned long split_dmabuf_huge_pud_range(struct vm_area_struct *vma, p4d_t *p4d,
				unsigned long addr, unsigned long end)
{
	pud_t *pud;
	unsigned long next;

	pud = pud_offset(p4d, addr);
	do {
		next = pud_addr_end(addr, end);
		if (pud_none_or_clear_bad(pud))
			continue;
		next = split_dmabuf_huge_pmd_range(vma, pud, addr, next);
		cond_resched();
	} while (pud++, addr = next, addr != end);

	return addr;
}

static inline unsigned long split_dmabuf_huge_p4d_range(struct vm_area_struct *vma, pgd_t *pgd,
				unsigned long addr, unsigned long end)
{
	p4d_t *p4d;
	unsigned long next;

	p4d = p4d_offset(pgd, addr);
	do {
		next = p4d_addr_end(addr, end);
		if (p4d_none_or_clear_bad(p4d))
			continue;
		next = split_dmabuf_huge_pud_range(vma, p4d, addr, next);
	} while (p4d++, addr = next, addr != end);

	return addr;
}

void __split_dmabuf_huge_range(struct vm_area_struct *vma)
{
	pgd_t *pgd;
	unsigned long next;
	unsigned long addr = vma->vm_start;
	unsigned long end = vma->vm_end;

	pgd = pgd_offset(vma->vm_mm, addr);
	do {
		next = pgd_addr_end(addr, end);
		if (pgd_none_or_clear_bad(pgd))
			continue;
		next = split_dmabuf_huge_p4d_range(vma, pgd, addr, next);
	} while (pgd++, addr = next, addr != end);
}

void split_dmabuf_huge_range(struct vm_area_struct *vma)
{
	__split_dmabuf_huge_range(vma);
}

spinlock_t *__pmd_dmabuf_huge_lock(pmd_t *pmd, struct vm_area_struct *vma)
{
	spinlock_t *ptl;
	ptl = pmd_lock(vma->vm_mm, pmd);
	if (likely(pmd_dmabuf_huge(*pmd)))
		return ptl;
	spin_unlock(ptl);
	return NULL;
}

static inline void zap_dmabuf_hugetlb_deposited_table(struct mm_struct *mm, pmd_t *pmd)
{
	pgtable_t pgtable;

	pgtable = pgtable_trans_huge_withdraw(mm, pmd);
	pte_free(mm, pgtable);
	mm_dec_nr_ptes(mm);
}

int zap_dmabuf_huge_pmd(struct mmu_gather *tlb, struct vm_area_struct *vma,
		 pmd_t *pmd, unsigned long addr)
{
	pmd_t orig_pmd;
	spinlock_t *ptl;

	tlb_change_page_size(tlb, PMD_SIZE);
	ptl = __pmd_dmabuf_huge_lock(pmd, vma);
	if (!ptl)
		return 0;

	orig_pmd = pmdp_huge_get_and_clear_full(vma, addr, pmd,
						tlb->fullmm);
	tlb_remove_pmd_tlb_entry(tlb, pmd, addr);
	zap_dmabuf_hugetlb_deposited_table(tlb->mm, pmd);
	spin_unlock(ptl);
	atomic_long_inc(&dmabuf_hugetlb_pmd_zap);

	return 1;
}

static void take_dmabuf_hugetlb_rmap_locks(struct vm_area_struct *vma)
{
	if (vma->vm_file)
		i_mmap_lock_write(vma->vm_file->f_mapping);
	if (vma->anon_vma)
		anon_vma_lock_write(vma->anon_vma);
}

static void drop_dmabuf_hugetlb_rmap_locks(struct vm_area_struct *vma)
{
	if (vma->anon_vma)
		anon_vma_unlock_write(vma->anon_vma);
	if (vma->vm_file)
		i_mmap_unlock_write(vma->vm_file->f_mapping);
}

static inline int dmabuf_hugetlb_pmd_move_must_withdraw(spinlock_t *new_pmd_ptl,
					 spinlock_t *old_pmd_ptl)
{
	/*
	 * With split pmd lock we also need to move preallocated
	 * PTE page table if new_pmd is on different PMD page table.
	 */
	return (new_pmd_ptl != old_pmd_ptl);
}

static bool move_dmabuf_huge_pmd_locked(struct vm_area_struct *vma, unsigned long old_addr,
		  unsigned long new_addr, pmd_t *old_pmd, pmd_t *new_pmd)
{
	spinlock_t *old_ptl, *new_ptl;
	pmd_t pmd;
	struct mm_struct *mm = vma->vm_mm;
	bool force_flush = false;

	/*
	 * The destination pmd shouldn't be established, free_pgtables()
	 * should have release it.
	 */
	if (WARN_ON(!pmd_none(*new_pmd))) {
		VM_BUG_ON(pmd_dmabuf_huge(*new_pmd));
		return false;
	}

	/*
	 * We don't have to worry about the ordering of src and dst
	 * ptlocks because exclusive mmap_lock prevents deadlock.
	 */
	old_ptl = __pmd_dmabuf_huge_lock(old_pmd, vma);
	if (old_ptl) {
		new_ptl = pmd_lockptr(mm, new_pmd);
		if (new_ptl != old_ptl)
			spin_lock_nested(new_ptl, SINGLE_DEPTH_NESTING);
		pmd = pmdp_huge_get_and_clear(mm, old_addr, old_pmd);
		if (pmd_present(pmd))
			force_flush = true;
		VM_BUG_ON(!pmd_none(*new_pmd));

		if (dmabuf_hugetlb_pmd_move_must_withdraw(new_ptl, old_ptl)) {
			pgtable_t pgtable;
			pgtable = pgtable_trans_huge_withdraw(mm, old_pmd);
			pgtable_trans_huge_deposit(mm, new_pmd, pgtable);
		}

		//pmd = move_soft_dirty_pmd(pmd);
		set_pmd_at(mm, new_addr, new_pmd, pmd);
		if (force_flush)
			flush_tlb_range(vma, old_addr, old_addr + PMD_SIZE);
		if (new_ptl != old_ptl)
			spin_unlock(new_ptl);
		spin_unlock(old_ptl);
		return true;
	}
	return false;
}

bool move_dmabuf_huge_pmd(struct vm_area_struct *vma, unsigned long old_addr,
		  unsigned long new_addr, pmd_t *old_pmd, pmd_t *new_pmd, bool need_rmap_locks)
{
	bool moved = false;

	/* See comment in move_ptes() */
	if (need_rmap_locks)
		take_dmabuf_hugetlb_rmap_locks(vma);

	moved = move_dmabuf_huge_pmd_locked(vma, old_addr, new_addr, old_pmd,
			  new_pmd);

	if (need_rmap_locks)
		drop_dmabuf_hugetlb_rmap_locks(vma);

	return moved;
}

static unsigned long
dmabuf_hugetlb_get_unmapped_area_bottomup(struct file *file, unsigned long addr,
		unsigned long len, unsigned long pgoff, unsigned long flags)
{
	struct vm_unmapped_area_info info;

	info.flags = 0;
	info.length = len;
	info.low_limit = current->mm->mmap_base;
	info.high_limit = arch_get_mmap_end(addr, len, flags);
	info.align_mask = (len >= PMD_SIZE) ? (~PMD_MASK) : \
					((len >= DMABUF_64K_SIZE) ? (~DMABUF_64K_MASK) : 0);
	info.align_offset = 0;
	return vm_unmapped_area(&info);
}

static unsigned long
dmabuf_hugetlb_get_unmapped_area_topdown(struct file *file, unsigned long addr,
		unsigned long len, unsigned long pgoff, unsigned long flags)
{
	struct vm_unmapped_area_info info;

	info.flags = VM_UNMAPPED_AREA_TOPDOWN;
	info.length = len;
	info.low_limit = max(PAGE_SIZE, mmap_min_addr);
	info.high_limit = arch_get_mmap_base(addr, current->mm->mmap_base);
	info.align_mask = (len >= PMD_SIZE) ? (~PMD_MASK) : \
					((len >= DMABUF_64K_SIZE) ? (~DMABUF_64K_MASK) : 0);
	info.align_offset = 0;
	addr = vm_unmapped_area(&info);

	/*
	 * A failed mmap() very likely causes application failure,
	 * so fall back to the bottom-up function here. This scenario
	 * can happen with large stack limits and large mmap()
	 * allocations.
	 */
	if (unlikely(offset_in_page(addr))) {
		VM_BUG_ON(addr != -ENOMEM);
		info.flags = 0;
		info.low_limit = current->mm->mmap_base;
		info.high_limit = arch_get_mmap_end(addr, len, flags);
		addr = vm_unmapped_area(&info);
	}

	return addr;
}

static unsigned long
generic_dmabuf_hugetlb_get_unmapped_area(struct file *file, unsigned long addr,
				  unsigned long len, unsigned long pgoff,
				  unsigned long flags)
{
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma, *prev;
	const unsigned long mmap_end = arch_get_mmap_end(addr, len, flags);

	if (len > mmap_end - mmap_min_addr)
		return -ENOMEM;

	if (flags & MAP_FIXED) {
		return addr;
	}

	if (addr) {
		if(len >= PMD_SIZE)
			addr = ALIGN(addr, PMD_SIZE);
		else if(len >= DMABUF_64K_SIZE)
			addr = ALIGN(addr, DMABUF_64K_SIZE);
		else
			addr = PAGE_ALIGN(addr);
		vma = find_vma_prev(mm, addr, &prev);
		if (mmap_end - len >= addr && addr >= mmap_min_addr &&
				(!vma || addr + len <= vm_start_gap(vma)) &&
				(!prev || addr >= vm_end_gap(prev)))
			return addr;
	}

	/*
	 * Use mm->get_unmapped_area value as a hint to use topdown routine.
	 * If architectures have special needs, they should define their own
	 * version of hugetlb_get_unmapped_area.
	 */
	if (mm->get_unmapped_area == arch_get_unmapped_area_topdown)
		return dmabuf_hugetlb_get_unmapped_area_topdown(file, addr, len,
				pgoff, flags);
	return dmabuf_hugetlb_get_unmapped_area_bottomup(file, addr, len,
			pgoff, flags);
}

unsigned long
dma_buf_hugetlb_get_unmapped_area(struct file *file, unsigned long addr,
			  unsigned long len, unsigned long pgoff,
			  unsigned long flags)
{
	return generic_dmabuf_hugetlb_get_unmapped_area(file, addr, len, pgoff, flags);
}
EXPORT_SYMBOL(dma_buf_hugetlb_get_unmapped_area);