/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SMMU pagetable dump.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __XRING_SMMU_DUMP_PGTABLE_H__
#define __XRING_SMMU_DUMP_PGTABLE_H__

#include "xring_smmu.h"

#define CD_MASK				0xfffffffffffc0
#define TTBR_MASK			0xffffffffffff0
#define DUMP_PGTABLE_MASK		0xfffffffff000

/* Event queue */
#define EVTQ_0_ID_SHIFT			0
#define EVTQ_0_ID_MASK			0xffUL

#define EVTQ_0_STREAM_ID_SHIFT	        32
#define EVTQ_0_STREAM_ID_MASK		0xffffffffUL

#define EVTQ_0_SSID_SHIFT               12
#define EVTQ_0_SSID_MASK                0xffff

#define EVTQ_TYPE_STREAM_DISABLE           0x06
#define EVTQ_TYPE_TRANSL_FORBIDDEN         0x07
#define EVTQ_TYPE_WALK_EABT                0x0b
#define EVTQ_TYPE_TRANSLATION              0x10
#define EVTQ_TYPE_ADDR_SIZE                0x11
#define EVTQ_TYPE_ACCESS                   0x12
#define EVTQ_TYPE_PERMISSION               0x13

#define EVTQ_TYPE_WITH_ADDR(id)     ((EVTQ_TYPE_TRANSLATION == (id))   \
				|| (EVTQ_TYPE_WALK_EABT == (id))   \
				|| (EVTQ_TYPE_TRANSL_FORBIDDEN == (id)) \
				|| (EVTQ_TYPE_ADDR_SIZE == (id))   \
				|| (EVTQ_TYPE_ACCESS == (id))      \
				|| (EVTQ_TYPE_PERMISSION == (id)))

#define EVTQ_TYPE_WITHOUT_SSID(id)  ((EVTQ_TYPE_STREAM_DISABLE == (id))   \
				|| (EVTQ_TYPE_TRANSL_FORBIDDEN == (id)))

/* PRI queue */
#define PRIQ_0_SID_SHIFT		32
#define PRIQ_0_SID_MASK			0xffffffffUL

#define XRING_PGDIR_SHIFT		39
#define XRING_PUDIR_SHIFT		30
#define XRING_PMDIR_SHIFT		21
#define XRING_PTEIR_SHIFT		12
#define XRING_LPAE_LVL_START		1
#define XRING_LPAE_TYPE			(1 << 1)
#define XRING_LPAE_BLOCK_TYPE		0x2
#define XRING_LPAE_VALID		(1 << 0)
#define XRING_LPAE_INVALID		0
#define XRING_LPAE_MAX_LEVELS		4
#define XRING_LPAE_PER_BITS		512

#ifndef xring_pgd_index
#define xring_pgd_index(a)  (((a) >> XRING_PGDIR_SHIFT) & (XRING_LPAE_PER_BITS - 1))
#endif
#ifndef xring_pud_index
#define xring_pud_index(a)  (((a) >> XRING_PUDIR_SHIFT) & (XRING_LPAE_PER_BITS - 1))
#endif
#ifndef xring_pmd_index
#define xring_pmd_index(a)  (((a) >> XRING_PMDIR_SHIFT) & (XRING_LPAE_PER_BITS - 1))
#endif
#ifndef xring_pte_index
#define xring_pte_index(a)  (((a) >> XRING_PTEIR_SHIFT) & (XRING_LPAE_PER_BITS - 1))
#endif

#ifdef CONFIG_XRING_SMMU_DUMP_PGTABLE
void xring_smmu_dump_struct(struct arm_smmu_device *smmu,
				u64 *evt, int evt_len);
void xring_smmu_dump_pgtbl(struct arm_smmu_device *smmu, u64 *evt,
		int evt_len, unsigned long num_entries);
void xring_smmu_dump_cd(struct arm_smmu_device *smmu, u32 sid, u32 ssid);
void xring_smmu_dump_ste(struct arm_smmu_device *smmu, u32 sid);
void xring_smmu_dump_va_range(struct arm_smmu_device *smmu, unsigned int sid,
		unsigned int ssid, unsigned long iova, size_t num_entries);
void xring_smmu_dump_pgtable_vmap(struct iommu_domain *domain, unsigned long iova,
		unsigned long num_entries);
#else
void xring_smmu_dump_struct(struct arm_smmu_device *smmu,
				u64 *evt, int evt_len)
{
}
void xring_smmu_dump_pgtbl(struct arm_smmu_device *smmu, u64 *evt,
		int evt_len, unsigned long num_entries)
{
}
void xring_smmu_dump_cd(struct arm_smmu_device *smmu, u32 sid, u32 ssid)
{
}
void xring_smmu_dump_ste(struct arm_smmu_device *smmu, u32 sid)
{
}
void xring_smmu_dump_va_range(struct arm_smmu_device *smmu, unsigned int sid,
		unsigned int ssid, unsigned long iova, size_t num_entries)
{
}
#endif

#endif /* __XRING_SMMU_DUMP_PGTABLE_H__ */
