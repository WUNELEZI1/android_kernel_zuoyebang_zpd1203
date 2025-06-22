// SPDX-License-Identifier: GPL-2.0
/*
 * SMMU pagetable dump.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#include <asm/cacheflush.h>
#include <asm/pgtable-prot.h>
#include <asm-generic/errno-base.h>
#include <linux/dma-map-ops.h>
#include <linux/device/bus.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/genalloc.h>
#include <linux/iova.h>
#include <linux/io-pgtable.h>
#include <linux/kthread.h>
#include <linux/pci.h>
#include <linux/rwlock.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/version.h>

#include "xring_smmu.h"
#include "xring_smmu_dump_pgtable.h"
#include "xring_io_pgtable.h"

static __le64 *arm_smmu_get_step_for_sid(struct arm_smmu_device *smmu, u32 sid)
{
	__le64 *step;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB) {
		struct arm_smmu_strtab_l1_desc *l1_desc;
		int idx;
		/* Two-level walk */
		idx = (sid >> STRTAB_SPLIT) * STRTAB_L1_DESC_DWORDS;
		l1_desc = &cfg->l1_desc[idx];
		idx = (sid & ((1 << STRTAB_SPLIT) - 1)) * STRTAB_STE_DWORDS;
		step = &l1_desc->l2ptr[idx];
	} else {
		/* Simple linear lookup */
		step = &cfg->strtab[sid * STRTAB_STE_DWORDS];
	}

	return step;
}

static __le64 *arm_smmu_get_cd_from_ste(struct arm_smmu_device *smmu,
				u32 sid, u32 ssid)
{
	u64 cd;
	__le64 *step = NULL;
	__le64 *cdp = NULL;
	__le64 *cde = NULL;
	const u64 cd_mask = CD_MASK;

	if (sid >= (1UL << smmu->sid_bits) ||
		ssid >= (1UL << smmu->ssid_bits)) {
		xring_smmu_err(" bad sid:%u, ssid:%u\n", sid, ssid);
		return NULL;
	}

	step = arm_smmu_get_step_for_sid(smmu, sid);
	if (!step) {
		xring_smmu_err("Err: strtab is null\n");
		return NULL;
	}

	cd = (step[0] & cd_mask); /* cd phys */
	cdp = phys_to_virt(cd); /* cd virt */
	cde = &cdp[ssid * CTXDESC_CD_DWORDS]; /* cd entry */
	return cde;
}

static void _xring_smmu_dump_cd(struct arm_smmu_device *smmu,
				u32 sid, u32 ssid, u32 evt_id, bool evtid_validtag)
{
	int i;
	__le64 *cde = NULL;
#ifdef XRING_SMMU_TEST_DUMP
	uint64_t cd[2] = {0x0001E202C0002019, 0xffffffffffff0};
	bool flag = false;
#endif

	if (evtid_validtag && EVTQ_TYPE_WITHOUT_SSID(evt_id)) {
		xring_smmu_err(" event_id:%u not support\n", evt_id);
		return;
	}

	cde = arm_smmu_get_cd_from_ste(smmu, sid, ssid);
	if (!cde) {
		xring_smmu_err("ssid:%u, get cde null\n", ssid);
		return;
	}

	xring_smmu_err("dump ssid:%u cde:%p\n", ssid, cde);
	for (i = 0; i < CTXDESC_CD_DWORDS; i++) {
		xring_smmu_err("0x%016llx\n", cde[i]);
#ifdef XRING_SMMU_TEST_DUMP
		if ((i == 0) && (cd[0] == cde[i])) {
			flag = true;
		} else if ((i == 1) && (cd[1] == (cde[i] | TTBR_MASK))) {
			if (!flag)
				xring_smmu_err("dump cd err!\n");
			else
				xring_smmu_err("dump cd success!\n");
		}
#endif
	}
}

static void xring_smmu_dump_pgtbl_debug_info(struct arm_smmu_device *smmu, uint64_t *pgtbl, unsigned long va,
				int pgtbl_level, size_t num_entries)
{
	uint64_t *ptep = pgtbl;
	uint64_t phys;
	uint tlb_type;
	int i;
	char *pgtbl_name[4] = {"pgd", "pud", "pmd", "pte"};

	xring_smmu_err("input va:0x%lx, num_entries:0x%zx\n", va, num_entries);

	do {
		if (pgtbl_level == 0) {
			ptep += xring_pgd_index(va);
		} else if (pgtbl_level == 1) {
			ptep += xring_pud_index(va);
		} else if (pgtbl_level == 2) {
			ptep += xring_pmd_index(va);
		} else if (pgtbl_level == 3) {
			ptep += xring_pte_index(va);
		} else {
			xring_smmu_err("invalid pgtbl_level = %d\n", pgtbl_level);
			return;
		}
		phys = *ptep;
		if (phys <= 0) {
			xring_smmu_err("USER_DEBUG_INFO: %s = 0x%llx\n", pgtbl_name[pgtbl_level], phys);
			return;
		}

		tlb_type = phys & (XRING_LPAE_VALID | XRING_LPAE_TYPE);
		if (tlb_type == XRING_LPAE_INVALID) {
			xring_smmu_err("USER_DEBUG_INFO: %s = 0x%llx\n", pgtbl_name[pgtbl_level], phys);
			return;
		}
		if (tlb_type == XRING_LPAE_BLOCK_TYPE) {
			for (i = 0; i < num_entries; i++)
				xring_smmu_err("USER_DEBUG_INFO: %s = 0x%llx\n",
					pgtbl_name[pgtbl_level], ptep[i]);
			return;
		}

		if (pgtbl_level != 3) {
			xring_smmu_err("USER_DEBUG_INFO: %s = 0x%llx\n",
				pgtbl_name[pgtbl_level], phys);
		} else {
			size_t current_entries = iopte_pte_index(va);
			size_t max_entries = min_t(size_t, (512 - current_entries), num_entries);

			xring_smmu_err("current_entries:0x%zx, num_entries:0x%zx, max_entries:0x%zx\n",
				 current_entries, num_entries, max_entries);
			for (i = 0; i < max_entries; i++)
				xring_smmu_err("USER_DEBUG_INFO: i:%d, va:0x%lx, %s = 0x%llx\n",
					i, va + i * PAGE_SIZE, pgtbl_name[pgtbl_level], ptep[i]);
			if ((num_entries - max_entries) != 0)
				xring_smmu_dump_pgtbl_debug_info(smmu, pgtbl, va + PAGE_SIZE * max_entries,
					1, (num_entries - max_entries));
			else
				return;
		}
		ptep = phys_to_virt((uint64_t)(phys & DUMP_PGTABLE_MASK));
		dma_sync_single_for_cpu(smmu->dev, (unsigned long)(phys & DUMP_PGTABLE_MASK), PAGE_SIZE, DMA_FROM_DEVICE);
	} while (++pgtbl_level < XRING_LPAE_MAX_LEVELS);

#ifdef XRING_SMMU_TEST_DUMP
	xring_smmu_err("dump pgtable success\n");
#endif
}

static uint64_t *arm_smmu_get_pgd(struct arm_smmu_device *smmu, u32 sid,
				u32 ssid, u32 evt_id, bool evtid_validtag)
{
	u64 ttbr;
	__le64 *cde = NULL;
	uint64_t *pgd = NULL;
	u64 ttbr_mask = TTBR_MASK;

	if (evtid_validtag && (!EVTQ_TYPE_WITH_ADDR(evt_id))) {
		xring_smmu_err("event_id:%u not support\n", evt_id);
		return NULL;
	}

	cde = arm_smmu_get_cd_from_ste(smmu, sid, ssid);
	if (!cde) {
		xring_smmu_err("ssid:%u, get cde null\n", ssid);
		return NULL;
	}

	ttbr = (cde[1] & ttbr_mask);
	if (!ttbr || !pfn_valid(__phys_to_pfn(ttbr))) {
		xring_smmu_err("ssid:%u, ttbr:0x%llx fail\n", ssid, ttbr);
		return NULL;
	}

	pgd = phys_to_virt(ttbr);
	dma_sync_single_for_cpu(smmu->dev, (unsigned long)ttbr, PAGE_SIZE, DMA_FROM_DEVICE);
	return pgd;
}

void xring_smmu_dump_ste(struct arm_smmu_device *smmu, u32 sid)
{
	int i;
	__le64 *step = NULL;
#ifdef XRING_SMMU_TEST_DUMP
	uint64_t ste[2] = {0x200fffffffffffcb, 0x00020000080000d6};
	bool flag = false;
#endif

	if (sid >= (1 << smmu->sid_bits)) {
		dev_info(smmu->dev, "sid %u invalid, sid_bits: %u\n",
			sid, smmu->sid_bits);
		return;
	}

	step = arm_smmu_get_step_for_sid(smmu, sid);
	if (!step) {
		xring_smmu_err("Err: strtab is null\n");
		return;
	}

	dev_info(smmu->dev, "Dump sid %u STE:\n", sid);
	for (i = 0; i < STRTAB_STE_DWORDS; i++) {
		dev_info(smmu->dev, "0x%016llx\n", le64_to_cpu(step[i]));
#ifdef XRING_SMMU_TEST_DUMP
		if ((i == 0) && (ste[0] == (step[i] | CD_MASK))) {
			flag = true;
		} else if ((i == 1) && (ste[1] == step[i])) {
			if (!flag)
				xring_smmu_err("dump ste err!\n");
			else
				xring_smmu_err("dump ste success!\n");
		}
#endif
	}
}
EXPORT_SYMBOL(xring_smmu_dump_ste);

void xring_smmu_dump_cd(struct arm_smmu_device *smmu,
				u32 sid, u32 ssid)
{
	return _xring_smmu_dump_cd(smmu, sid, ssid, 0, false);
}
EXPORT_SYMBOL(xring_smmu_dump_cd);

void xring_smmu_dump_va_range(struct arm_smmu_device *smmu, unsigned int sid,
		unsigned int ssid, unsigned long iova, unsigned long num_entries)
{
	uint64_t *pgd = NULL;

	xring_smmu_dump_ste(smmu, sid);
	xring_smmu_dump_cd(smmu, sid, ssid);
	pgd = arm_smmu_get_pgd(smmu, sid, ssid, 0, false);
	if (!pgd) {
		xring_smmu_err("get pgd fail,sid:%u,ssid:%u,va:0x%lx\n",
			sid, ssid, iova);
		return;
	}

	xring_smmu_dump_pgtbl_debug_info(smmu, pgd, iova, 1, num_entries);
}
EXPORT_SYMBOL(xring_smmu_dump_va_range);

void xring_smmu_dump_pgtbl(struct arm_smmu_device *smmu, u64 *evt, int evt_len,
		unsigned long num_entries)
{
	u32 evt_id;
	u32 sid, ssid;
	int lvl = XRING_LPAE_LVL_START;
	uint64_t *pgd = NULL;

	if (evt_len > EVTQ_ENT_DWORDS)
		return;

	evt_id = (evt[0] >> EVTQ_0_ID_SHIFT) & EVTQ_0_ID_MASK;
	if (!evt_id)
		return;
	sid = (evt[0] >> PRIQ_0_SID_SHIFT) & PRIQ_0_SID_MASK;
	ssid = (evt[0] >> EVTQ_0_SSID_SHIFT) & EVTQ_0_SSID_MASK;

	pgd = arm_smmu_get_pgd(smmu, sid, ssid, evt_id, true);
	if (!pgd) {
		xring_smmu_err("get pgd fail,sid:%u,ssid:%u,va:0x%llx\n",
			sid, ssid, evt[2]);
		return;
	}

	xring_smmu_dump_pgtbl_debug_info(smmu, pgd, evt[2], lvl, num_entries);
}
EXPORT_SYMBOL(xring_smmu_dump_pgtbl);

void xring_smmu_dump_pgtable_vmap(struct iommu_domain *domain, unsigned long iova,
		unsigned long num_entries)
{
	struct io_pgtable_ops *ops = to_smmu_domain(domain)->pgtbl_ops;
	struct xring_io_pgtable *data = iof_pgtable_ops_to_data(ops);
	xring_iopte *pmd_ptep = iopte_pmd_offset(data->pmds, data->base, iova);
	unsigned long pmd_num = GET_PMD_NUM(iova, (iova + (num_entries << PAGE_SHIFT)));
	unsigned long pte_page_currut = (iova >> PMD_SHIFT) + data->pte_pg_index;
	unsigned long pgd_index = iopte_pgd_index(iova);
	unsigned long pte_index = iopte_pte_index(iova);
	unsigned long iova_tmp = iova;

	char *pgtbl_name[4] = {"pgd", "pud", "pmd", "pte"};
	xring_iopte *ptep = NULL, *tmp_ptep = NULL;
	unsigned long entries = num_entries;
	size_t max_entries = 0;

	ptep = vmap(&data->pages[pte_page_currut], pmd_num, VM_MAP, pgprot_writecombine(PAGE_KERNEL));
	if (!ptep) {
		xring_smmu_err("vmap ptep is null\n");
		return;
	}

	tmp_ptep = ptep;
	ptep += pte_index;

	for (int i = 0; i < pmd_num; i++) {
		size_t current_entries = iopte_pte_index(iova_tmp);

		xring_smmu_err("USER_DEBUG_INFO: %s = 0x%llx\n", pgtbl_name[2], *(pmd_ptep + i));
		xring_smmu_err("USER_DEBUG_INFO: %s = 0x%llx\n",
					pgtbl_name[1], *(data->pgd + pgd_index));
		max_entries = min_t(size_t, (512 - current_entries), entries);
		for (int j = 0; j < max_entries; j++) {
			xring_smmu_err("USER_DEBUG_INFO: j:%d, va:0x%lx, %s = 0x%llx\n",
				j, iova_tmp, pgtbl_name[3], ptep[j]);
			iova_tmp += PAGE_SIZE;
		}
		entries -= max_entries;
	}

	vunmap(tmp_ptep);
}
EXPORT_SYMBOL(xring_smmu_dump_pgtable_vmap);

void xring_smmu_dump_struct(struct arm_smmu_device *smmu, u64 *evt, int evt_len)
{
	u32 evt_id;
	u32 sid, ssid;

	if (evt_len > EVTQ_ENT_DWORDS)
		return;

	evt_id = (evt[0] >> EVTQ_0_ID_SHIFT) & EVTQ_0_ID_MASK;
	sid = (evt[0] >> PRIQ_0_SID_SHIFT) & PRIQ_0_SID_MASK;
	ssid = (evt[0] >> EVTQ_0_SSID_SHIFT) & EVTQ_0_SSID_MASK;
	if (!evt_id)
		return;

	xring_smmu_dump_ste(smmu, sid);
	_xring_smmu_dump_cd(smmu, sid, ssid, evt_id, true);
}
EXPORT_SYMBOL(xring_smmu_dump_struct);
