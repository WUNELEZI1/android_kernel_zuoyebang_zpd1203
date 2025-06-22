/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __UFS_XRING_MCQ_H__
#define __UFS_XRING_MCQ_H__

//#include "ufs-xring-sys-ctrl.h"
//#include "ufs-xring-fpga-mphy.h"

/* ufs4.0 mcq */
#define MCQ_ENTRY_SIZE_IN_DWORD		8

#define ufsmcq_writel(mcq_info, val, reg)	\
	writel((val), (mcq_info)->mcq_base + (reg))
#define ufsmcq_readl(mcq_info, reg)	\
	readl((mcq_info)->mcq_base + (reg))

#define MCQ_CFG_n(r, i)	((r) + MCQ_QCFG_SIZE * (i))
#define MCQ_OPR_OFFSET_n(p, i) \
	(mcq_info->mcq_opr[(p)].offset + mcq_info->mcq_opr[(p)].stride * (i))

#define MCQ_CFG_MAC_MASK	GENMASK(16, 8)
#define MAX_Q			GENMASK(4, 0)
#define MCQ_QCFGPTR_MASK	GENMASK(23, 16)
#define MCQ_QCFGPTR_UNIT	0x200
#define MCQ_QCFG_SIZE		0x40

#define MCQ_CQ_EVENT_STATUS	0x100000
/* CQISy - CQ y Interrupt Status Register  */
#define UFSHCD_MCQ_CQIS_TAIL_ENT_PUSH_STS	0x1

#define CQE_UCD_BA		GENMASK_ULL(63, 7)
#define SQ_ICU_ERR_CODE_MASK	GENMASK(7, 4)

/* Max mcq register polling time in microseconds */
#define MCQ_POLL_US	500000

#define UFSHCD_ENABLE_MCQ_INTRS	(UTP_TASK_REQ_COMPL |\
				UFSHCD_ERROR_MASK |\
				MCQ_CQ_EVENT_STATUS)

#define UFSHCD_ENABLE_MCQ_INTRS_ESI	(UTP_TASK_REQ_COMPL |\
					UFSHCD_ERROR_MASK)

#define MAX_ACTIVE_CMD 32

#define ESI_NR_IRQS 96
#define ESI_IRQ_TYPE 6
#define CQ_TEPS_OFFSET 4

#define ufs_xring_mcq_hex_dump(prefix_str, buf, len) do {                \
	size_t __len = (len);                                            \
	print_hex_dump(KERN_ERR, prefix_str,                             \
		       __len > 4 ? DUMP_PREFIX_OFFSET : DUMP_PREFIX_NONE,\
		       16, 4, buf, __len, false);                        \
} while (0)

#define ESI_AXI_RESP_ERR_STATUS 0x1
#define ESI_GIF_SACCEPT_TIMEOUT_STATUS 0x2
#define HS2LS_ERROR_INT_STATUS 0x4

#define UTRD_OCS_MASK		UFS_MASK(0xff, 0)

enum {
	REG_SQATTR		= 0x0,
	REG_SQLBA		= 0x4,
	REG_SQUBA		= 0x8,
	REG_SQDAO		= 0xC,
	REG_SQISAO		= 0x10,
	REG_SQCFG		= 0x14,

	REG_CQATTR		= 0x20,
	REG_CQLBA		= 0x24,
	REG_CQUBA		= 0x28,
	REG_CQDAO		= 0x2C,
	REG_CQISAO		= 0x30,
};

enum ufshcd_mcq_opr {
	OPR_SQD,
	OPR_SQIS,
	OPR_CQD,
	OPR_CQIS,
	OPR_MAX,
};

/* OPR_SQD */
enum {
	REG_SQHP		= 0x0,
	REG_SQTP		= 0x4,
	REG_SQRTC		= 0x8,
	REG_SQCTI		= 0xC,
	REG_SQRTS		= 0x10,
};

/* OPR_CQD */
enum {
	REG_CQHP		= 0x0,
	REG_CQTP		= 0x4,
};

/* OPR_CQIS */
enum {
	REG_CQIS		= 0x0,
	REG_CQIE		= 0x4,
};

enum {
	SQ_START		= 0x0,
	SQ_STOP			= 0x1,
	SQ_ICU			= 0x2,
};

enum {
	SQ_STS			= 0x1,
	SQ_CUS			= 0x2,
};

/* UFS4.0 register */
enum {
	REG_MCQCAP				= 0x04,
	REG_UFS_MEM_CFG				= 0x300,
	REG_UFS_MCQ_CFG				= 0x380,
};

enum {
	/* one for empty, one for devman command*/
	UFSHCD_MCQ_NUM_RESERVED = 2,
};

enum ufshcd_res {
	RES_UFS,
	RES_MCQ,
	RES_MCQ_SQD,
	RES_MCQ_SQIS,
	RES_MCQ_CQD,
	RES_MCQ_CQIS,
	RES_MCQ_VS,
	RES_MAX,
};

enum {
	QUEUE_EN_OFFSET = 31,
	QUEUE_ID_OFFSET = 16,
};

enum {
	UFSHCD_UIC_DL_PA_INIT_ERROR = (1 << 0), /* Data link layer error */
};

/* ufs_sys_ctrl register */
enum {
	UFS_ERR_INT_CLEAR_OFFSET = 0x64,
	UFS_ERR_INT_MASK_OFFSET = 0x68,
	UFS_ERR_INT_STATUS_OFFSET = 0x6c,
	UFS_ESI_MASK_0_OFFSET = 0x70,
	UFS_ESI_MASK_1_OFFSET = 0x74,
	UFS_ESI_MASK_2_OFFSET = 0x78,
	UFS_ESI_DEVICE_ID_OFFSET = 0x8c,
};

enum {
	UFS_ESI_AXI_RESP_ERR_CLEAR_MASK = GENMASK(2, 0),
	UFS_ERR_INT_MASK_MASK = GENMASK(1, 0),
	UFS_ESI_DEVICE_ID_MASK = GENMASK(31, 16),
};

struct ufshcd_res_info {
	const char *name;
	struct resource *resource;
	void __iomem *base;
};

struct ufshcd_mcq_opr_info_t {
	unsigned long offset;
	unsigned long stride;
	void __iomem *base;
};

struct ufs_hw_queue {
	void __iomem *mcq_sq_head;
	void __iomem *mcq_sq_tail;
	void __iomem *mcq_cq_head;
	void __iomem *mcq_cq_tail;

	void *sqe_base_addr;
	dma_addr_t sqe_dma_addr;
	struct cq_entry *cqe_base_addr;
	dma_addr_t cqe_dma_addr;
	u32 max_entries;
	u32 id;
	u32 sq_tail_slot;
	spinlock_t sq_lock;
	u32 cq_tail_slot;
	u32 cq_head_slot;
	spinlock_t cq_lock;
	struct mutex sq_mutex;
};

struct ufs_xring_mcq_info {
	u32 mcq_capabilities;
	void __iomem *mcq_base;

	u32 mcq_nr_hw_queue;
	u32 mcq_queue_depth;
	bool mcq_is_enabled;

	struct ufs_hw_queue *uhq;
	struct ufshcd_res_info res[RES_MAX];
	struct ufshcd_mcq_opr_info_t mcq_opr[OPR_MAX];

	bool esi_is_enabled;
	unsigned int esi_base;
};

/* MCQ Completion Queue Entry */
struct cq_entry {
	/* DW 0-1 */
	__le64 command_desc_base_addr;

	/* DW 2 */
	__le16  response_upiu_length;
	__le16  response_upiu_offset;

	/* DW 3 */
	__le16  prd_table_length;
	__le16  prd_table_offset;

	/* DW 4 */
	__le32 status;

	/* DW 5-7 */
	__le32 reserved[3];
};

static inline void ufs_xring_mcq_inc_sq_tail(struct ufs_hw_queue *q)
{
	u32 val;

	q->sq_tail_slot++;
	if (q->sq_tail_slot == q->max_entries)
		q->sq_tail_slot = 0;
	val = q->sq_tail_slot * sizeof(struct utp_transfer_req_desc);
	writel(val, q->mcq_sq_tail);
}

static inline void ufs_xring_mcq_update_cq_tail_slot(struct ufs_hw_queue *q)
{
	u32 val = readl(q->mcq_cq_tail);

	q->cq_tail_slot = val / sizeof(struct cq_entry);
}

static inline bool ufs_xring_mcq_is_cq_empty(struct ufs_hw_queue *q)
{
	return q->cq_head_slot == q->cq_tail_slot;
}

static inline void ufs_xring_mcq_inc_cq_head_slot(struct ufs_hw_queue *q)
{
	q->cq_head_slot++;
	if (q->cq_head_slot == q->max_entries)
		q->cq_head_slot = 0;
}

static inline void ufs_xring_mcq_update_cq_head(struct ufs_hw_queue *q)
{
	writel(q->cq_head_slot * sizeof(struct cq_entry), q->mcq_cq_head);
}

static inline struct cq_entry *ufs_xring_mcq_cur_cqe(struct ufs_hw_queue *q)
{
	struct cq_entry *cqe = q->cqe_base_addr;

	return cqe + q->cq_head_slot;
}

/* host lock must be held before calling this func */
static inline bool ufs_xring_mcq_is_saved_err_fatal(struct ufs_hba *hba)
{
	return (hba->saved_uic_err & UFSHCD_UIC_DL_PA_INIT_ERROR) ||
	       (hba->saved_err & (INT_FATAL_ERRORS | UFSHCD_UIC_HIBERN8_MASK));
}

/* host lock must be held before calling this func */
static inline void ufs_xring_mcq_schedule_eh_work(struct ufs_hba *hba)
{
	/* handle fatal errors only when link is not in error state */
	if (hba->ufshcd_state != UFSHCD_STATE_ERROR) {
		if (hba->force_reset || ufshcd_is_link_broken(hba) ||
		    ufs_xring_mcq_is_saved_err_fatal(hba))
			hba->ufshcd_state = UFSHCD_STATE_EH_SCHEDULED_FATAL;
		else
			hba->ufshcd_state = UFSHCD_STATE_EH_SCHEDULED_NON_FATAL;
		queue_work(hba->eh_wq, &hba->eh_work);
	}
}

static inline size_t ufshcd_sg_entry_size(const struct ufs_hba *hba)
{
	return sizeof(struct ufshcd_sg_entry);
}

static inline size_t ufshcd_get_ucd_size(const struct ufs_hba *hba)
{
	return sizeof(struct utp_transfer_cmd_desc) + SG_ALL * ufshcd_sg_entry_size(hba);
}


int ufs_xring_mcq_alloc_priv(struct ufs_hba *hba);
void ufs_xring_mcq_get_dts_info(struct ufs_hba *hba);
int ufs_xring_mcq_install_tracepoints(struct ufs_hba *hba);
void ufs_xring_irq_set_affinity(struct ufs_hba *hba);
int ufs_xring_register_esi(struct ufs_hba *hba);
void ufs_xring_mcq_enable_intr(struct ufs_hba *hba, u32 intrs);

#endif
