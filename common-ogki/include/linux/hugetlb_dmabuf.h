/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _LINUX_HUGETLB_DMABUF_H
#define _LINUX_HUGETLB_DMABUF_H

#include <linux/mm.h>

void split_dmabuf_huge_range(struct vm_area_struct *vma);
int zap_dmabuf_huge_pmd(struct mmu_gather *tlb, struct vm_area_struct *vma,
		 pmd_t *pmd, unsigned long addr);
void vma_adjust_dmabuf_huge(struct vm_area_struct *vma,
			     unsigned long start,
			     unsigned long end,
			     long adjust_next);
bool move_dmabuf_huge_pmd(struct vm_area_struct *vma, unsigned long old_addr,
		  unsigned long new_addr, pmd_t *old_pmd, pmd_t *new_pmd, bool need_rmap_locks);
void zap_split_dmabuf_huge_pmd(struct vm_area_struct *vma, pmd_t *pmd,
		unsigned long address, bool freeze, struct folio *folio);
void __split_dmabuf_huge_pmd(struct vm_area_struct *vma, pmd_t *pmd,
		unsigned long address, bool freeze, struct folio *folio);
int dmabuf_huge_remap_pfn_range(struct vm_area_struct *vma, unsigned long addr,
		    unsigned long pfn, unsigned long size, pgprot_t prot, unsigned int map_type);
unsigned long
dma_buf_hugetlb_get_unmapped_area(struct file *file, unsigned long addr,
			  unsigned long len, unsigned long pgoff,
			  unsigned long flags);
bool dmabuf_hugetlb_enabled(void);

static inline void vm_dmabuf_hugetlb_clear(struct vm_area_struct *vma)
{
	vm_flags_clear(vma, VM_DMABUF_HUGETLB);
}

static inline bool is_vm_dmabuf_hugetlb_page(struct vm_area_struct *vma)
{
	return !!(vma->vm_flags & VM_DMABUF_HUGETLB);
}

#ifdef CONFIG_XIAOMI_DMABUF_HUGETLB
static inline int pmd_dmabuf_huge(pmd_t pmd)
{
	return pmd_val(pmd) && pmd_present(pmd) && !(pmd_val(pmd) & PMD_TABLE_BIT);
}
#else
static inline int pmd_dmabuf_huge(pmd_t pmd)
{
	return 0;
}
#endif /* CONFIG_XIAOMI_DMABUF_HUGETLB */
#endif /* _LINUX_HUGETLB_DMABUF_H */