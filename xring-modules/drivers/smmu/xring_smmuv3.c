// SPDX-License-Identifier: GPL-2.0
/*
 * IOMMU API for ARM architected SMMUv3 implementations.
 *
 * Copyright (C) 2015 ARM Limited
 *
 * Author: Will Deacon <will.deacon@arm.com>
 *
 * This driver is powered by bad coffee and bombay mix.
 */

#include <linux/acpi.h>
#include <linux/acpi_iort.h>
#include <linux/bitmap.h>
#include <linux/bitops.h>
#include <linux/crash_dump.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io-pgtable.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/pci-ats.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/amba/bus.h>
#include <linux/mutex.h>
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
#include <linux/dma-iommu.h>
#else
#include "../../../common/drivers/iommu/dma-iommu.h"
#endif

#include "xring_smmu.h"
#include "xring_iommu_dma.h"
#include "xring_smmu_dump_pgtable.h"
#include "xring_smmu_debugfs.h"
#include "xring_io_pgtable.h"

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
#include "iommu-sva-lib.h"
#else
#include "iommu-sva.h"
#endif

extern bool dumpmapunmap;
static struct iommu_domain *isp_iommu_domain;

static bool disable_bypass = true;

module_param(disable_bypass, bool, 0444);
MODULE_PARM_DESC(disable_bypass,
	"Disable bypass streams such that incoming transactions from devices that are not attached to an iommu domain will report an abort back to the device and will not be allowed to pass through the SMMU.");

struct arm_smmu_option_prop {
	u32 opt;
	const char *prop;
};

DEFINE_XARRAY_ALLOC1(arm_smmu_asid_xa);
DEFINE_MUTEX(arm_smmu_asid_lock);

/*
 * Special value used by SVA when a process dies, to quiesce a CD without
 * disabling it.
 */
struct arm_smmu_ctx_desc quiet_cd = { 0 };

static struct arm_smmu_option_prop arm_smmu_options[] = {
	{ ARM_SMMU_OPT_SKIP_PREFETCH, "hisilicon,broken-prefetch-cmd" },
	{ ARM_SMMU_OPT_PAGE0_REGS_ONLY, "cavium,cn9900-broken-page1-regspace"},
	{ 0, NULL},
};

static void xring_smmu_dump_reg_val(struct arm_smmu_device *smmu);

static void parse_driver_options(struct arm_smmu_device *smmu)
{
	int i = 0;

	do {
		if (of_property_read_bool(smmu->dev->of_node,
						arm_smmu_options[i].prop)) {
			smmu->options |= arm_smmu_options[i].opt;
			dev_notice(smmu->dev, "option %s\n",
				arm_smmu_options[i].prop);
		}
	} while (arm_smmu_options[++i].opt);
}

#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
static int xring_smmu_tcu_link_check(struct arm_smmu_device *smmu)
{
	if (atomic_read(&smmu->tcu_link_cnt) == 0)
		return -1;

	return 0;
}
#endif

static int xring_smmu_tcu_irpt_clear(struct arm_smmu_device *smmu,
					u32 irpt_clear);
#ifdef XRING_SMMU_TBU_IRQ
static int xring_smmu_tbu_irpt_clear(struct xring_smmu_tbu *tbu,
					u32 irpt_clear);
#endif
static void xring_smmu_wrapper_setup_irqs(struct arm_smmu_device *smmu);

/* Low-level queue manipulation functions */
static bool queue_has_space(struct arm_smmu_ll_queue *q, u32 n)
{
	u32 space, prod, cons;

	prod = Q_IDX(q, q->prod);
	cons = Q_IDX(q, q->cons);

	if (Q_WRP(q, q->prod) == Q_WRP(q, q->cons))
		space = (1 << q->max_n_shift) - (prod - cons);
	else
		space = cons - prod;

	return space >= n;
}

static bool queue_full(struct arm_smmu_ll_queue *q)
{
	return Q_IDX(q, q->prod) == Q_IDX(q, q->cons) &&
	       Q_WRP(q, q->prod) != Q_WRP(q, q->cons);
}

static bool queue_empty(struct arm_smmu_ll_queue *q)
{
	return Q_IDX(q, q->prod) == Q_IDX(q, q->cons) &&
	       Q_WRP(q, q->prod) == Q_WRP(q, q->cons);
}

static bool queue_consumed(struct arm_smmu_ll_queue *q, u32 prod)
{
	return ((Q_WRP(q, q->cons) == Q_WRP(q, prod)) &&
		(Q_IDX(q, q->cons) > Q_IDX(q, prod))) ||
	       ((Q_WRP(q, q->cons) != Q_WRP(q, prod)) &&
		(Q_IDX(q, q->cons) <= Q_IDX(q, prod)));
}

static void queue_sync_cons_out(struct arm_smmu_queue *q)
{
	/*
	 * Ensure that all CPU accesses (reads and writes) to the queue
	 * are complete before we update the cons pointer.
	 */
	__iomb();
	writel_relaxed(q->llq.cons, q->cons_reg);
}

static void queue_inc_cons(struct arm_smmu_ll_queue *q)
{
	u32 cons;

	cons = (Q_WRP(q, q->cons) | Q_IDX(q, q->cons)) + 1;
	q->cons = Q_OVF(q->cons) | Q_WRP(q, cons) | Q_IDX(q, cons);
}

static int queue_sync_prod_in(struct arm_smmu_queue *q)
{
	u32 prod;
	int ret = 0;

	/*
	 * We can't use the _relaxed() variant here, as we must prevent
	 * speculative reads of the queue before we have determined that
	 * prod has indeed moved.
	 */
	prod = readl(q->prod_reg);

	if (Q_OVF(prod) != Q_OVF(q->llq.prod))
		ret = -EOVERFLOW;

	q->llq.prod = prod;
	return ret;
}

static u32 queue_inc_prod_n(struct arm_smmu_ll_queue *q, int n)
{
	u32 prod;

	prod = (Q_WRP(q, q->prod) | Q_IDX(q, q->prod)) + n;
	return Q_OVF(q->prod) | Q_WRP(q, prod) | Q_IDX(q, prod);
}

static void queue_poll_init(struct arm_smmu_device *smmu,
			    struct arm_smmu_queue_poll *qp)
{
	qp->delay = 1;
	qp->spin_cnt = 0;
	qp->wfe = !!(smmu->features & ARM_SMMU_FEAT_SEV);
	qp->timeout = ktime_add_us(ktime_get(), ARM_SMMU_POLL_TIMEOUT_US);
}

static int queue_poll(struct arm_smmu_queue_poll *qp)
{
	if (ktime_compare(ktime_get(), qp->timeout) > 0)
		return -ETIMEDOUT;

	if (qp->wfe) {
		wfe();
	} else if (++qp->spin_cnt < ARM_SMMU_POLL_SPIN_COUNT) {
		cpu_relax();
	} else {
		udelay(qp->delay);
		qp->delay *= 2;
		qp->spin_cnt = 0;
	}

	return 0;
}

static void queue_write(__le64 *dst, u64 *src, size_t n_dwords)
{
	int i;

	for (i = 0; i < n_dwords; ++i)
		*dst++ = cpu_to_le64(*src++);
}

static void queue_read(u64 *dst, __le64 *src, size_t n_dwords)
{
	int i;

	for (i = 0; i < n_dwords; ++i)
		*dst++ = le64_to_cpu(*src++);
}

static int queue_remove_raw(struct arm_smmu_queue *q, u64 *ent)
{
	if (queue_empty(&q->llq))
		return -EAGAIN;

	queue_read(ent, Q_ENT(q, q->llq.cons), q->ent_dwords);
	queue_inc_cons(&q->llq);
	queue_sync_cons_out(q);
	return 0;
}

/* High-level queue accessors */
static int arm_smmu_cmdq_build_cmd(u64 *cmd, struct arm_smmu_cmdq_ent *ent)
{
	memset(cmd, 0, 1 << CMDQ_ENT_SZ_SHIFT);
	cmd[0] |= FIELD_PREP(CMDQ_0_OP, ent->opcode);

	switch (ent->opcode) {
	case CMDQ_OP_TLBI_EL2_ALL:
	case CMDQ_OP_TLBI_NSNH_ALL:
		break;
	case CMDQ_OP_PREFETCH_CFG:
		cmd[0] |= FIELD_PREP(CMDQ_PREFETCH_0_SID, ent->prefetch.sid);
		break;
	case CMDQ_OP_CFGI_CD:
		cmd[0] |= FIELD_PREP(CMDQ_CFGI_0_SSID, ent->cfgi.ssid);
		fallthrough;
	case CMDQ_OP_CFGI_STE:
		cmd[0] |= FIELD_PREP(CMDQ_CFGI_0_SID, ent->cfgi.sid);
		cmd[1] |= FIELD_PREP(CMDQ_CFGI_1_LEAF, ent->cfgi.leaf);
		break;
	case CMDQ_OP_CFGI_CD_ALL:
		cmd[0] |= FIELD_PREP(CMDQ_CFGI_0_SID, ent->cfgi.sid);
		break;
	case CMDQ_OP_CFGI_ALL:
		/* Cover the entire SID range */
		cmd[1] |= FIELD_PREP(CMDQ_CFGI_1_RANGE, 31);
		break;
	case CMDQ_OP_TLBI_NH_VA:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_VMID, ent->tlbi.vmid);
		fallthrough;
	case CMDQ_OP_TLBI_EL2_VA:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_NUM, ent->tlbi.num);
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_SCALE, ent->tlbi.scale);
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_ASID, ent->tlbi.asid);
		cmd[1] |= FIELD_PREP(CMDQ_TLBI_1_LEAF, ent->tlbi.leaf);
		cmd[1] |= FIELD_PREP(CMDQ_TLBI_1_TTL, ent->tlbi.ttl);
		cmd[1] |= FIELD_PREP(CMDQ_TLBI_1_TG, ent->tlbi.tg);
		cmd[1] |= ent->tlbi.addr & CMDQ_TLBI_1_VA_MASK;
		break;
	case CMDQ_OP_TLBI_S2_IPA:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_NUM, ent->tlbi.num);
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_SCALE, ent->tlbi.scale);
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_VMID, ent->tlbi.vmid);
		cmd[1] |= FIELD_PREP(CMDQ_TLBI_1_LEAF, ent->tlbi.leaf);
		cmd[1] |= FIELD_PREP(CMDQ_TLBI_1_TTL, ent->tlbi.ttl);
		cmd[1] |= FIELD_PREP(CMDQ_TLBI_1_TG, ent->tlbi.tg);
		cmd[1] |= ent->tlbi.addr & CMDQ_TLBI_1_IPA_MASK;
		break;
	case CMDQ_OP_TLBI_NH_ASID:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_ASID, ent->tlbi.asid);
		fallthrough;
	case CMDQ_OP_TLBI_S12_VMALL:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_VMID, ent->tlbi.vmid);
		break;
	case CMDQ_OP_TLBI_EL2_ASID:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_ASID, ent->tlbi.asid);
		break;
	case CMDQ_OP_ATC_INV:
		cmd[0] |= FIELD_PREP(CMDQ_0_SSV, ent->substream_valid);
		cmd[0] |= FIELD_PREP(CMDQ_ATC_0_GLOBAL, ent->atc.global);
		cmd[0] |= FIELD_PREP(CMDQ_ATC_0_SSID, ent->atc.ssid);
		cmd[0] |= FIELD_PREP(CMDQ_ATC_0_SID, ent->atc.sid);
		cmd[1] |= FIELD_PREP(CMDQ_ATC_1_SIZE, ent->atc.size);
		cmd[1] |= ent->atc.addr & CMDQ_ATC_1_ADDR_MASK;
		break;
	case CMDQ_OP_PRI_RESP:
		cmd[0] |= FIELD_PREP(CMDQ_0_SSV, ent->substream_valid);
		cmd[0] |= FIELD_PREP(CMDQ_PRI_0_SSID, ent->pri.ssid);
		cmd[0] |= FIELD_PREP(CMDQ_PRI_0_SID, ent->pri.sid);
		cmd[1] |= FIELD_PREP(CMDQ_PRI_1_GRPID, ent->pri.grpid);
		switch (ent->pri.resp) {
		case PRI_RESP_DENY:
		case PRI_RESP_FAIL:
		case PRI_RESP_SUCC:
			break;
		default:
			return -EINVAL;
		}
		cmd[1] |= FIELD_PREP(CMDQ_PRI_1_RESP, ent->pri.resp);
		break;
	case CMDQ_OP_RESUME:
		cmd[0] |= FIELD_PREP(CMDQ_RESUME_0_SID, ent->resume.sid);
		cmd[0] |= FIELD_PREP(CMDQ_RESUME_0_RESP, ent->resume.resp);
		cmd[1] |= FIELD_PREP(CMDQ_RESUME_1_STAG, ent->resume.stag);
		break;
	case CMDQ_OP_CMD_SYNC:
		cmd[0] |= FIELD_PREP(CMDQ_SYNC_0_CS, CMDQ_SYNC_0_CS_SEV);
		cmd[0] |= FIELD_PREP(CMDQ_SYNC_0_MSH, ARM_SMMU_SH_ISH);
		cmd[0] |= FIELD_PREP(CMDQ_SYNC_0_MSIATTR, ARM_SMMU_MEMATTR_OIWB);
		break;
	default:
		return -ENOENT;
	}

	return 0;
}

static struct arm_smmu_cmdq *arm_smmu_get_cmdq(struct arm_smmu_device *smmu)
{
	return &smmu->cmdq;
}

static void arm_smmu_cmdq_build_sync_cmd(u64 *cmd, struct arm_smmu_device *smmu,
					 struct arm_smmu_queue *q, u32 prod)
{
	struct arm_smmu_cmdq_ent ent = {
		.opcode = CMDQ_OP_CMD_SYNC,
	};

	arm_smmu_cmdq_build_cmd(cmd, &ent);
}

static void __arm_smmu_cmdq_skip_err(struct arm_smmu_device *smmu,
				     struct arm_smmu_queue *q)
{
	static const char * const cerror_str[] = {
		[CMDQ_ERR_CERROR_NONE_IDX]	= "No error",
		[CMDQ_ERR_CERROR_ILL_IDX]	= "Illegal command",
		[CMDQ_ERR_CERROR_ABT_IDX]	= "Abort on command fetch",
		[CMDQ_ERR_CERROR_ATC_INV_IDX]	= "ATC invalidate timeout",
	};

	int i;
	u64 cmd[CMDQ_ENT_DWORDS];
	u32 cons = readl_relaxed(q->cons_reg);
	u32 idx = FIELD_GET(CMDQ_CONS_ERR, cons);
	struct arm_smmu_cmdq_ent cmd_sync = {
		.opcode = CMDQ_OP_CMD_SYNC,
	};

	dev_err(smmu->dev, "CMDQ error (cons 0x%08x): %s\n", cons,
		idx < ARRAY_SIZE(cerror_str) ?  cerror_str[idx] : "Unknown");

	switch (idx) {
	case CMDQ_ERR_CERROR_ABT_IDX:
		dev_err(smmu->dev, "retrying command fetch\n");
		return;
	case CMDQ_ERR_CERROR_NONE_IDX:
		return;
	case CMDQ_ERR_CERROR_ATC_INV_IDX:
		/*
		 * ATC Invalidation Completion timeout. CONS is still pointing
		 * at the CMD_SYNC. Attempt to complete other pending commands
		 * by repeating the CMD_SYNC, though we might well end up back
		 * here since the ATC invalidation may still be pending.
		 */
		return;
	case CMDQ_ERR_CERROR_ILL_IDX:
	default:
		break;
	}

	/*
	 * We may have concurrent producers, so we need to be careful
	 * not to touch any of the shadow cmdq state.
	 */
	queue_read(cmd, Q_ENT(q, cons), q->ent_dwords);
	dev_err(smmu->dev, "skipping command in error state:\n");
	for (i = 0; i < ARRAY_SIZE(cmd); ++i)
		dev_err(smmu->dev, "\t0x%016llx\n", (unsigned long long)cmd[i]);

	/* Convert the erroneous command into a CMD_SYNC */
	if (arm_smmu_cmdq_build_cmd(cmd, &cmd_sync)) {
		dev_err(smmu->dev, "failed to convert to CMD_SYNC\n");
		return;
	}

	queue_write(Q_ENT(q, cons), cmd, q->ent_dwords);
}

static void arm_smmu_cmdq_skip_err(struct arm_smmu_device *smmu)
{
	__arm_smmu_cmdq_skip_err(smmu, &smmu->cmdq.q);
}

/*
 * Command queue locking.
 * This is a form of bastardised rwlock with the following major changes:
 *
 * - The only LOCK routines are exclusive_trylock() and shared_lock().
 *   Neither have barrier semantics, and instead provide only a control
 *   dependency.
 *
 * - The UNLOCK routines are supplemented with shared_tryunlock(), which
 *   fails if the caller appears to be the last lock holder (yes, this is
 *   racy). All successful UNLOCK routines have RELEASE semantics.
 */
static void arm_smmu_cmdq_shared_lock(struct arm_smmu_cmdq *cmdq)
{
	int val;

	/*
	 * We can try to avoid the cmpxchg() loop by simply incrementing the
	 * lock counter. When held in exclusive state, the lock counter is set
	 * to INT_MIN so these increments won't hurt as the value will remain
	 * negative.
	 */
	if (atomic_fetch_inc_relaxed(&cmdq->lock) >= 0)
		return;

	do {
		val = atomic_cond_read_relaxed(&cmdq->lock, VAL >= 0);
	} while (atomic_cmpxchg_relaxed(&cmdq->lock, val, val + 1) != val);
}

static void arm_smmu_cmdq_shared_unlock(struct arm_smmu_cmdq *cmdq)
{
	(void)atomic_dec_return_release(&cmdq->lock);
}

static bool arm_smmu_cmdq_shared_tryunlock(struct arm_smmu_cmdq *cmdq)
{
	if (atomic_read(&cmdq->lock) == 1)
		return false;

	arm_smmu_cmdq_shared_unlock(cmdq);
	return true;
}

#define arm_smmu_cmdq_exclusive_trylock_irqsave(cmdq, flags)		\
({									\
	bool __ret;							\
	local_irq_save(flags);						\
	__ret = !atomic_cmpxchg_relaxed(&cmdq->lock, 0, INT_MIN);	\
	if (!__ret)							\
		local_irq_restore(flags);				\
	__ret;								\
})

#define arm_smmu_cmdq_exclusive_unlock_irqrestore(cmdq, flags)		\
({									\
	atomic_set_release(&cmdq->lock, 0);				\
	local_irq_restore(flags);					\
})


/*
 * Command queue insertion.
 * This is made fiddly by our attempts to achieve some sort of scalability
 * since there is one queue shared amongst all of the CPUs in the system.  If
 * you like mixed-size concurrency, dependency ordering and relaxed atomics,
 * then you'll *love* this monstrosity.
 *
 * The basic idea is to split the queue up into ranges of commands that are
 * owned by a given CPU; the owner may not have written all of the commands
 * itself, but is responsible for advancing the hardware prod pointer when
 * the time comes. The algorithm is roughly:
 *
 *      1. Allocate some space in the queue. At this point we also discover
 *         whether the head of the queue is currently owned by another CPU,
 *         or whether we are the owner.
 *
 *      2. Write our commands into our allocated slots in the queue.
 *
 *      3. Mark our slots as valid in arm_smmu_cmdq.valid_map.
 *
 *      4. If we are an owner:
 *              a. Wait for the previous owner to finish.
 *              b. Mark the queue head as unowned, which tells us the range
 *                 that we are responsible for publishing.
 *              c. Wait for all commands in our owned range to become valid.
 *              d. Advance the hardware prod pointer.
 *              e. Tell the next owner we've finished.
 *
 *      5. If we are inserting a CMD_SYNC (we may or may not have been an
 *         owner), then we need to stick around until it has completed:
 *              a. If we have MSIs, the SMMU can write back into the CMD_SYNC
 *                 to clear the first 4 bytes.
 *              b. Otherwise, we spin waiting for the hardware cons pointer to
 *                 advance past our command.
 *
 * The devil is in the details, particularly the use of locking for handling
 * SYNC completion and freeing up space in the queue before we think that it is
 * full.
 */
static void __arm_smmu_cmdq_poll_set_valid_map(struct arm_smmu_cmdq *cmdq,
					       u32 sprod, u32 eprod, bool set)
{
	u32 swidx, sbidx, ewidx, ebidx;
	struct arm_smmu_ll_queue llq = {
		.max_n_shift	= cmdq->q.llq.max_n_shift,
		.prod		= sprod,
	};

	ewidx = BIT_WORD(Q_IDX(&llq, eprod));
	ebidx = Q_IDX(&llq, eprod) % BITS_PER_LONG;

	while (llq.prod != eprod) {
		unsigned long mask;
		atomic_long_t *ptr;
		u32 limit = BITS_PER_LONG;

		swidx = BIT_WORD(Q_IDX(&llq, llq.prod));
		sbidx = Q_IDX(&llq, llq.prod) % BITS_PER_LONG;

		ptr = &cmdq->valid_map[swidx];

		if ((swidx == ewidx) && (sbidx < ebidx))
			limit = ebidx;

		mask = GENMASK(limit - 1, sbidx);

		/*
		 * The valid bit is the inverse of the wrap bit. This means
		 * that a zero-initialised queue is invalid and, after marking
		 * all entries as valid, they become invalid again when we
		 * wrap.
		 */
		if (set) {
			atomic_long_xor(mask, ptr);
		} else { /* Poll */
			unsigned long valid;

			valid = (ULONG_MAX + !!Q_WRP(&llq, llq.prod)) & mask;
			atomic_long_cond_read_relaxed(ptr, (VAL & mask) == valid);
		}

		llq.prod = queue_inc_prod_n(&llq, limit - sbidx);
	}
}

/* Mark all entries in the range [sprod, eprod) as valid */
static void arm_smmu_cmdq_set_valid_map(struct arm_smmu_cmdq *cmdq,
					u32 sprod, u32 eprod)
{
	__arm_smmu_cmdq_poll_set_valid_map(cmdq, sprod, eprod, true);
}

/* Wait for all entries in the range [sprod, eprod) to become valid */
static void arm_smmu_cmdq_poll_valid_map(struct arm_smmu_cmdq *cmdq,
					 u32 sprod, u32 eprod)
{
	__arm_smmu_cmdq_poll_set_valid_map(cmdq, sprod, eprod, false);
}

/* Wait for the command queue to become non-full */
static int arm_smmu_cmdq_poll_until_not_full(struct arm_smmu_device *smmu,
					     struct arm_smmu_ll_queue *llq)
{
	unsigned long flags;
	struct arm_smmu_queue_poll qp;
	struct arm_smmu_cmdq *cmdq = arm_smmu_get_cmdq(smmu);
	int ret = 0;

	/*
	 * Try to update our copy of cons by grabbing exclusive cmdq access. If
	 * that fails, spin until somebody else updates it for us.
	 */
	if (arm_smmu_cmdq_exclusive_trylock_irqsave(cmdq, flags)) {
		WRITE_ONCE(cmdq->q.llq.cons, readl_relaxed(cmdq->q.cons_reg));
		arm_smmu_cmdq_exclusive_unlock_irqrestore(cmdq, flags);
		llq->val = READ_ONCE(cmdq->q.llq.val);
		return 0;
	}

	queue_poll_init(smmu, &qp);
	do {
		llq->val = READ_ONCE(cmdq->q.llq.val);
		if (!queue_full(llq))
			break;

		ret = queue_poll(&qp);
	} while (!ret);

	return ret;
}

/*
 * Wait until the SMMU cons index passes llq->prod.
 * Must be called with the cmdq lock held in some capacity.
 */
static int __arm_smmu_cmdq_poll_until_consumed(struct arm_smmu_device *smmu,
					       struct arm_smmu_ll_queue *llq)
{
	struct arm_smmu_queue_poll qp;
	struct arm_smmu_cmdq *cmdq = arm_smmu_get_cmdq(smmu);
	u32 prod = llq->prod;
	int ret = 0;

	queue_poll_init(smmu, &qp);
	llq->val = READ_ONCE(cmdq->q.llq.val);
	do {
		if (queue_consumed(llq, prod))
			break;

		ret = queue_poll(&qp);

		/*
		 * This needs to be a readl() so that our subsequent call
		 * to arm_smmu_cmdq_shared_tryunlock() can fail accurately.
		 *
		 * Specifically, we need to ensure that we observe all
		 * shared_lock()s by other CMD_SYNCs that share our owner,
		 * so that a failing call to tryunlock() means that we're
		 * the last one out and therefore we can safely advance
		 * cmdq->q.llq.cons. Roughly speaking:
		 *
		 * CPU 0		CPU1			CPU2 (us)
		 *
		 * if (sync)
		 *         shared_lock();
		 *
		 * dma_wmb();
		 * set_valid_map();
		 *
		 *                      if (owner) {
		 *                              poll_valid_map();
		 *                              <control dependency>
		 *                              writel(prod_reg);
		 *
		 *                                              readl(cons_reg);
		 *                                              tryunlock();
		 *
		 * Requires us to see CPU 0's shared_lock() acquisition.
		 */
		llq->cons = readl(cmdq->q.cons_reg);
	} while (!ret);

	return ret;
}

static int arm_smmu_cmdq_poll_until_sync(struct arm_smmu_device *smmu,
					 struct arm_smmu_ll_queue *llq)
{
	return __arm_smmu_cmdq_poll_until_consumed(smmu, llq);
}

static void arm_smmu_cmdq_write_entries(struct arm_smmu_cmdq *cmdq, u64 *cmds,
					u32 prod, int n)
{
	int i;
	struct arm_smmu_ll_queue llq = {
		.max_n_shift	= cmdq->q.llq.max_n_shift,
		.prod		= prod,
	};

	for (i = 0; i < n; ++i) {
		u64 *cmd = &cmds[i * CMDQ_ENT_DWORDS];

		prod = queue_inc_prod_n(&llq, i);
		queue_write(Q_ENT(&cmdq->q, prod), cmd, CMDQ_ENT_DWORDS);
	}
}

/*
 * This is the actual insertion function, and provides the following
 * ordering guarantees to callers:
 *
 * - There is a dma_wmb() before publishing any commands to the queue.
 *   This can be relied upon to order prior writes to data structures
 *   in memory (such as a CD or an STE) before the command.
 *
 * - On completion of a CMD_SYNC, there is a control dependency.
 *   This can be relied upon to order subsequent writes to memory (e.g.
 *   freeing an IOVA) after completion of the CMD_SYNC.
 *
 * - Command insertion is totally ordered, so if two CPUs each race to
 *   insert their own list of commands then all of the commands from one
 *   CPU will appear before any of the commands from the other CPU.
 */
static int arm_smmu_cmdq_issue_cmdlist(struct arm_smmu_device *smmu,
				       u64 *cmds, int n, bool sync)
{
	u64 cmd_sync[CMDQ_ENT_DWORDS];
	u32 prod;
	unsigned long flags;
	bool owner;
	struct arm_smmu_cmdq *cmdq = arm_smmu_get_cmdq(smmu);
	struct arm_smmu_ll_queue llq, head;
	struct arm_smmu_queue *q = &smmu->cmdq.q;
	u64 cmd_notcons[CMDQ_ENT_DWORDS];
	u32 cmd_cons = 0;
	int ret = 0;

	llq.max_n_shift = cmdq->q.llq.max_n_shift;

	/* 1. Allocate some space in the queue */
	local_irq_save(flags);
	llq.val = READ_ONCE(cmdq->q.llq.val);
	do {
		u64 old;

		while (!queue_has_space(&llq, n + sync)) {
			local_irq_restore(flags);
			if (arm_smmu_cmdq_poll_until_not_full(smmu, &llq))
				dev_err_ratelimited(smmu->dev, "CMDQ timeout\n");
			local_irq_save(flags);
		}

		head.cons = llq.cons;
		head.prod = queue_inc_prod_n(&llq, n + sync) |
					     CMDQ_PROD_OWNED_FLAG;

		old = cmpxchg_relaxed(&cmdq->q.llq.val, llq.val, head.val);
		if (old == llq.val)
			break;

		llq.val = old;
	} while (1);
	owner = !(llq.prod & CMDQ_PROD_OWNED_FLAG);
	head.prod &= ~CMDQ_PROD_OWNED_FLAG;
	llq.prod &= ~CMDQ_PROD_OWNED_FLAG;

	/*
	 * 2. Write our commands into the queue
	 * Dependency ordering from the cmpxchg() loop above.
	 */
	arm_smmu_cmdq_write_entries(cmdq, cmds, llq.prod, n);
	if (sync) {
		prod = queue_inc_prod_n(&llq, n);
		arm_smmu_cmdq_build_sync_cmd(cmd_sync, smmu, &cmdq->q, prod);
		queue_write(Q_ENT(&cmdq->q, prod), cmd_sync, CMDQ_ENT_DWORDS);

		/*
		 * In order to determine completion of our CMD_SYNC, we must
		 * ensure that the queue can't wrap twice without us noticing.
		 * We achieve that by taking the cmdq lock as shared before
		 * marking our slot as valid.
		 */
		arm_smmu_cmdq_shared_lock(cmdq);
	}

	/* 3. Mark our slots as valid, ensuring commands are visible first */
	dma_wmb();
	arm_smmu_cmdq_set_valid_map(cmdq, llq.prod, head.prod);

	/* 4. If we are the owner, take control of the SMMU hardware */
	if (owner) {
		/* a. Wait for previous owner to finish */
		atomic_cond_read_relaxed(&cmdq->owner_prod, VAL == llq.prod);

		/* b. Stop gathering work by clearing the owned flag */
		prod = atomic_fetch_andnot_relaxed(CMDQ_PROD_OWNED_FLAG,
						   &cmdq->q.llq.atomic.prod);
		prod &= ~CMDQ_PROD_OWNED_FLAG;

		/*
		 * c. Wait for any gathered work to be written to the queue.
		 * Note that we read our own entries so that we have the control
		 * dependency required by (d).
		 */
		arm_smmu_cmdq_poll_valid_map(cmdq, llq.prod, prod);

		/*
		 * d. Advance the hardware prod pointer
		 * Control dependency ordering from the entries becoming valid.
		 */
		writel_relaxed(prod, cmdq->q.prod_reg);

		/*
		 * e. Tell the next owner we're done
		 * Make sure we've updated the hardware first, so that we don't
		 * race to update prod and potentially move it backwards.
		 */
		atomic_set_release(&cmdq->owner_prod, prod);
	}

	/* 5. If we are inserting a CMD_SYNC, we must wait for it to complete */
	if (sync) {
		llq.prod = queue_inc_prod_n(&llq, n);
		ret = arm_smmu_cmdq_poll_until_sync(smmu, &llq);
		if (ret) {
			dev_err_ratelimited(smmu->dev,
					    "CMD_SYNC timeout at 0x%08x 0x%08x [hwprod 0x%08x, hwcons 0x%08x]\n",
					    llq.prod, llq.cons,
					    readl_relaxed(cmdq->q.prod_reg),
					    readl_relaxed(cmdq->q.cons_reg));

			cmd_cons = readl_relaxed(q->cons_reg);
			queue_read(cmd_notcons, Q_ENT(q, (cmd_cons - 1)), q->ent_dwords);
			dev_err(smmu->dev, "pre cmd cons:0x%08x, not cons command:\n", cmd_cons - 1);
			for (int i = 0; i < ARRAY_SIZE(cmd_notcons); ++i)
				dev_err(smmu->dev, "\t0x%016llx\n",
					(unsigned long long)cmd_notcons[i]);

			queue_read(cmd_notcons, Q_ENT(q, cmd_cons), q->ent_dwords);
			dev_err(smmu->dev, "current cmd cons:0x%08x, not cons command -- cmd_sync:\n", cmd_cons);
			for (int i = 0; i < ARRAY_SIZE(cmd_notcons); ++i)
				dev_err(smmu->dev, "\t0x%016llx\n",
					(unsigned long long)cmd_notcons[i]);

			u32 evt_prod = readl(smmu->page1 + ARM_SMMU_EVTQ_PROD);
			u32 evt_cons = readl(smmu->page1 + ARM_SMMU_EVTQ_CONS);

			xring_smmu_err("EVTQ_PROD:0x%x\n", evt_prod);
			xring_smmu_err("EVTQ_CONS:0x%x\n", evt_cons);
			if (evt_prod != evt_cons) {
				u64 evt[EVTQ_ENT_DWORDS];
				struct arm_smmu_queue *evtq = &smmu->evtq.q;

				queue_read(evt, Q_ENT(evtq, evt_prod), evtq->ent_dwords);
				dev_err(smmu->dev, "event queue:\n");
				for (int i = 0; i < ARRAY_SIZE(evt); ++i)
					dev_err(smmu->dev, "\t0x%016llx\n",
						(unsigned long long)evt[i]);
			}
			xring_smmu_dump_reg_val(smmu);
		}

		/*
		 * Try to unlock the cmdq lock. This will fail if we're the last
		 * reader, in which case we can safely update cmdq->q.llq.cons
		 */
		if (!arm_smmu_cmdq_shared_tryunlock(cmdq)) {
			WRITE_ONCE(cmdq->q.llq.cons, llq.cons);
			arm_smmu_cmdq_shared_unlock(cmdq);
		}
	}

	local_irq_restore(flags);
	return ret;
}

static int __arm_smmu_cmdq_issue_cmd(struct arm_smmu_device *smmu,
				     struct arm_smmu_cmdq_ent *ent,
				     bool sync)
{
	u64 cmd[CMDQ_ENT_DWORDS];

	if (arm_smmu_cmdq_build_cmd(cmd, ent)) {
		dev_warn(smmu->dev, "ignoring unknown CMDQ opcode 0x%x\n",
			 ent->opcode);
		return -EINVAL;
	}

	return arm_smmu_cmdq_issue_cmdlist(smmu, cmd, 1, sync);
}

static int arm_smmu_cmdq_issue_cmd(struct arm_smmu_device *smmu,
				   struct arm_smmu_cmdq_ent *ent)
{
	return __arm_smmu_cmdq_issue_cmd(smmu, ent, false);
}

static int arm_smmu_cmdq_issue_cmd_with_sync(struct arm_smmu_device *smmu,
					     struct arm_smmu_cmdq_ent *ent)
{
	return __arm_smmu_cmdq_issue_cmd(smmu, ent, true);
}

static void arm_smmu_cmdq_batch_add(struct arm_smmu_device *smmu,
				    struct arm_smmu_cmdq_batch *cmds,
				    struct arm_smmu_cmdq_ent *cmd)
{
	if (cmds->num == CMDQ_BATCH_ENTRIES) {
		arm_smmu_cmdq_issue_cmdlist(smmu, cmds->cmds, cmds->num, false);
		cmds->num = 0;
	}
	arm_smmu_cmdq_build_cmd(&cmds->cmds[cmds->num * CMDQ_ENT_DWORDS], cmd);
	cmds->num++;
}

static int arm_smmu_cmdq_batch_submit(struct arm_smmu_device *smmu,
				      struct arm_smmu_cmdq_batch *cmds)
{
	return arm_smmu_cmdq_issue_cmdlist(smmu, cmds->cmds, cmds->num, true);
}

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
static int arm_smmu_page_response(struct device *dev,
				  struct iommu_fault_event *unused,
				  struct iommu_page_response *resp)
{
	struct arm_smmu_cmdq_ent cmd = {0};
	struct arm_smmu_master *master = dev_iommu_priv_get(dev);

	if (master->stall_enabled) {
		cmd.opcode		= CMDQ_OP_RESUME;
		cmd.resume.sid		= master->sid;
		cmd.resume.stag		= resp->grpid;
		switch (resp->code) {
		case IOMMU_PAGE_RESP_INVALID:
		case IOMMU_PAGE_RESP_FAILURE:
			cmd.resume.resp = CMDQ_RESUME_0_RESP_ABORT;
			break;
		case IOMMU_PAGE_RESP_SUCCESS:
			cmd.resume.resp = CMDQ_RESUME_0_RESP_RETRY;
			break;
		default:
			return -EINVAL;
		}

		arm_smmu_cmdq_issue_cmd(master->smmu, &cmd);
	} else {
		return -ENODEV;
	}

	return 0;
}
#else
static int arm_smmu_page_response(struct device *dev,
				  struct iommu_fault_event *unused,
				  struct iommu_page_response *resp)
{
	struct arm_smmu_cmdq_ent cmd = {0};
	struct arm_smmu_master *master = dev_iommu_priv_get(dev);
	int sid = master->streams[0].id;

	if (master->stall_enabled) {
		cmd.opcode		= CMDQ_OP_RESUME;
		cmd.resume.sid		= sid;
		cmd.resume.stag		= resp->grpid;
		switch (resp->code) {
		case IOMMU_PAGE_RESP_INVALID:
		case IOMMU_PAGE_RESP_FAILURE:
			cmd.resume.resp = CMDQ_RESUME_0_RESP_ABORT;
			break;
		case IOMMU_PAGE_RESP_SUCCESS:
			cmd.resume.resp = CMDQ_RESUME_0_RESP_RETRY;
			break;
		default:
			return -EINVAL;
		}
	} else {
		return -ENODEV;
	}

	arm_smmu_cmdq_issue_cmd(master->smmu, &cmd);
	/*
	 * Don't send a SYNC, it doesn't do anything for RESUME or PRI_RESP.
	 * RESUME consumption guarantees that the stalled transaction will be
	 * terminated... at some point in the future. PRI_RESP is fire and
	 * forget.
	 */

	return 0;
}
#endif
#endif

/* Context descriptor manipulation functions */
void arm_smmu_tlb_inv_asid(struct arm_smmu_device *smmu, u16 asid)
{
	struct arm_smmu_cmdq_ent cmd = {
		.opcode	= smmu->features & ARM_SMMU_FEAT_E2H ?
			CMDQ_OP_TLBI_EL2_ASID : CMDQ_OP_TLBI_NH_ASID,
		.tlbi.asid = asid,
	};

	arm_smmu_cmdq_issue_cmd_with_sync(smmu, &cmd);
}

static void arm_smmu_sync_cd(struct arm_smmu_domain *smmu_domain,
			     int ssid, bool leaf)
{
#ifndef CONFIG_XRING_SMMU_DOMAIN_SHARE
	size_t i;
#endif
	unsigned long flags;
	struct arm_smmu_master *master = NULL;
	struct arm_smmu_cmdq_batch cmds;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_cmdq_ent cmd = {
		.opcode	= CMDQ_OP_CFGI_CD,
		.cfgi	= {
			.ssid	= ssid,
			.leaf	= leaf,
		},
	};

	cmds.num = 0;

	spin_lock_irqsave(&smmu_domain->devices_lock, flags);
	list_for_each_entry(master, &smmu_domain->devices, domain_head) {
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
		cmd.cfgi.sid = master->sid;
		arm_smmu_cmdq_batch_add(smmu, &cmds, &cmd);
#else
		for (i = 0; i < master->num_streams; i++) {
			cmd.cfgi.sid = master->streams[i].id;
			arm_smmu_cmdq_batch_add(smmu, &cmds, &cmd);
		}
#endif
	}
	spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);

	arm_smmu_cmdq_batch_submit(smmu, &cmds);
}

static int arm_smmu_alloc_cd_leaf_table(struct arm_smmu_device *smmu,
					struct arm_smmu_l1_ctx_desc *l1_desc)
{
	size_t size = CTXDESC_L2_ENTRIES * (CTXDESC_CD_DWORDS << 3);

	l1_desc->l2ptr = dmam_alloc_coherent(smmu->dev, size,
					     &l1_desc->l2ptr_dma, GFP_KERNEL);
	if (!l1_desc->l2ptr) {
		dev_warn(smmu->dev,
			 "failed to allocate context descriptor table\n");
		return -ENOMEM;
	}
	return 0;
}

static void arm_smmu_write_cd_l1_desc(__le64 *dst,
				      struct arm_smmu_l1_ctx_desc *l1_desc)
{
	u64 val = (l1_desc->l2ptr_dma & CTXDESC_L1_DESC_L2PTR_MASK) |
		  CTXDESC_L1_DESC_V;

	/* See comment in arm_smmu_write_ctx_desc() */
	WRITE_ONCE(*dst, cpu_to_le64(val));
}

__le64 *arm_smmu_get_cd_ptr(struct arm_smmu_domain *smmu_domain,
				   u32 ssid)
{
	__le64 *l1ptr;
	unsigned int idx;
	struct arm_smmu_l1_ctx_desc *l1_desc;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	/* get global cd table from smmu */
	struct arm_smmu_ctx_desc_cfg *cdcfg = &smmu->cdcfg;

	if (smmu->cdcfg.s1fmt == STRTAB_STE_0_S1FMT_LINEAR)
#else
	struct arm_smmu_ctx_desc_cfg *cdcfg = &smmu_domain->s1_cfg.cdcfg;

	if (smmu_domain->s1_cfg.s1fmt == STRTAB_STE_0_S1FMT_LINEAR)
#endif
		return cdcfg->cdtab + ssid * CTXDESC_CD_DWORDS;

	idx = ssid >> CTXDESC_SPLIT;
	l1_desc = &cdcfg->l1_desc[idx];
	if (!l1_desc->l2ptr) {
		if (arm_smmu_alloc_cd_leaf_table(smmu, l1_desc))
			return NULL;

		l1ptr = cdcfg->cdtab + idx * CTXDESC_L1_DESC_DWORDS;
		arm_smmu_write_cd_l1_desc(l1ptr, l1_desc);
		/* An invalid L1CD can be cached */
		arm_smmu_sync_cd(smmu_domain, ssid, false);
	}
	idx = ssid & (CTXDESC_L2_ENTRIES - 1);
	return l1_desc->l2ptr + idx * CTXDESC_CD_DWORDS;
}

bool arm_smmu_is_shared_domain(struct device *dev)
{
	struct iommu_domain *domain = iommu_get_domain_for_dev(dev);
	struct arm_smmu_domain *smmu_domain = NULL;

	if (!domain) {
		dev_err(dev, "%s iommu domain is null\n", __func__);
		return false;
	}

	smmu_domain = to_smmu_domain(domain);
	if (!smmu_domain) {
		dev_err(dev, "%s smmu_domain is null\n", __func__);
		return false;
	}
	return smmu_domain->shared_flag;
}

int arm_smmu_write_ctx_desc(struct arm_smmu_domain *smmu_domain, int ssid,
			    struct arm_smmu_ctx_desc *cd)
{
	/*
	 * This function handles the following cases:
	 *
	 * (1) Install primary CD, for normal DMA traffic (SSID = 0).
	 * (2) Install a secondary CD, for SID+SSID traffic.
	 * (3) Update ASID of a CD. Atomically write the first 64 bits of the
	 *     CD, then invalidate the old entry and mappings.
	 * (4) Quiesce the context without clearing the valid bit. Disable
	 *     translation, and ignore any translation fault.
	 * (5) Remove a secondary CD.
	 */
	u64 val;
	bool cd_live;
	__le64 *cdptr;

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	if (WARN_ON(ssid >= (1 << smmu_domain->s1_cfg.cdcfg->s1cdmax)))
#else
	if (WARN_ON(ssid >= (1 << smmu_domain->s1_cfg.s1cdmax)))
#endif
		return -E2BIG;

	cdptr = arm_smmu_get_cd_ptr(smmu_domain, ssid);
	if (!cdptr)
		return -ENOMEM;

	val = le64_to_cpu(cdptr[0]);
	cd_live = !!(val & CTXDESC_CD_0_V);

	if (!cd) { /* (5) */
		val = 0;
	} else if (cd == &quiet_cd) { /* (4) */
		val |= CTXDESC_CD_0_TCR_EPD0;
	} else if (cd_live) { /* (3) */
		val &= ~CTXDESC_CD_0_ASID;
		val |= FIELD_PREP(CTXDESC_CD_0_ASID, cd->asid);
		/*
		 * Until CD+TLB invalidation, both ASIDs may be used for tagging
		 * this substream's traffic
		 */
	} else { /* (1) and (2) */
		cdptr[1] = cpu_to_le64(cd->ttbr & CTXDESC_CD_1_TTB0_MASK);
		cdptr[2] = 0;
		cdptr[3] = cpu_to_le64(cd->mair);

		/*
		 * STE is live, and the SMMU might read dwords of this CD in any
		 * order. Ensure that it observes valid values before reading
		 * V=1.
		 */
#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
		arm_smmu_sync_cd(smmu_domain, ssid, true);
#endif
		val = cd->tcr |
#ifdef __BIG_ENDIAN
			CTXDESC_CD_0_ENDI |
#endif
			CTXDESC_CD_0_R | CTXDESC_CD_0_A |
			(cd->mm ? 0 : CTXDESC_CD_0_ASET) |
			CTXDESC_CD_0_AA64 |
			FIELD_PREP(CTXDESC_CD_0_ASID, cd->asid) |
			CTXDESC_CD_0_V;

		if (smmu_domain->stall_enabled)
			val |= CTXDESC_CD_0_S;
	}

	/*
	 * The SMMU accesses 64-bit values atomically. See IHI0070Ca 3.21.3
	 * "Configuration structures and configuration invalidation completion"
	 *
	 *   The size of single-copy atomic reads made by the SMMU is
	 *   IMPLEMENTATION DEFINED but must be at least 64 bits. Any single
	 *   field within an aligned 64-bit span of a structure can be altered
	 *   without first making the structure invalid.
	 */
	WRITE_ONCE(cdptr[0], cpu_to_le64(val));
#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	arm_smmu_sync_cd(smmu_domain, ssid, true);
#endif
	return 0;
}

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
static int arm_smmu_alloc_cd_table(struct arm_smmu_device *smmu)
{
	int ret;
	size_t l1size;
	size_t max_contexts;
	struct arm_smmu_ctx_desc_cfg *cdcfg = &smmu->cdcfg;

	cdcfg->s1cdmax = MAXCONTEXT;
	max_contexts = 1 << cdcfg->s1cdmax;
	if (!(smmu->features & ARM_SMMU_FEAT_2_LVL_CDTAB) ||
		max_contexts <= CTXDESC_L2_ENTRIES) {
		cdcfg->s1fmt = STRTAB_STE_0_S1FMT_LINEAR;
		cdcfg->num_l1_ents = max_contexts;

		l1size = max_contexts * (CTXDESC_CD_DWORDS << 3);
	}  else {
		cdcfg->s1fmt = STRTAB_STE_0_S1FMT_64K_L2;
		cdcfg->num_l1_ents = DIV_ROUND_UP(cdcfg->s1cdmax,
						  CTXDESC_L2_ENTRIES);

		cdcfg->l1_desc = devm_kcalloc(smmu->dev, cdcfg->num_l1_ents,
					      sizeof(*cdcfg->l1_desc),
					      GFP_KERNEL);
		if (!cdcfg->l1_desc)
			return -ENOMEM;

		l1size = cdcfg->num_l1_ents * (CTXDESC_L1_DESC_DWORDS << 3);
	}

	cdcfg->cdtab = dmam_alloc_coherent(smmu->dev, l1size, &cdcfg->cdtab_dma,
					   GFP_KERNEL);
	if (!cdcfg->cdtab) {
		dev_warn(smmu->dev, "failed to allocate context descriptor\n");
		ret = -ENOMEM;
		goto err_free_l1;
	}

	return 0;

err_free_l1:
	if (cdcfg->l1_desc) {
		devm_kfree(smmu->dev, cdcfg->l1_desc);
		cdcfg->l1_desc = NULL;
	}
	return ret;
}
#else
static int arm_smmu_alloc_cd_tables(struct arm_smmu_domain *smmu_domain)
{
	int ret;
	size_t l1size;
	size_t max_contexts;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;
	struct arm_smmu_ctx_desc_cfg *cdcfg = &cfg->cdcfg;

	max_contexts = 1 << cfg->s1cdmax;

	if (!(smmu->features & ARM_SMMU_FEAT_2_LVL_CDTAB) ||
	    max_contexts <= CTXDESC_L2_ENTRIES) {
		cfg->s1fmt = STRTAB_STE_0_S1FMT_LINEAR;
		cdcfg->num_l1_ents = max_contexts;

		l1size = max_contexts * (CTXDESC_CD_DWORDS << 3);
	} else {
		cfg->s1fmt = STRTAB_STE_0_S1FMT_64K_L2;
		cdcfg->num_l1_ents = DIV_ROUND_UP(max_contexts,
						  CTXDESC_L2_ENTRIES);

		cdcfg->l1_desc = devm_kcalloc(smmu->dev, cdcfg->num_l1_ents,
					      sizeof(*cdcfg->l1_desc),
					      GFP_KERNEL);
		if (!cdcfg->l1_desc)
			return -ENOMEM;

		l1size = cdcfg->num_l1_ents * (CTXDESC_L1_DESC_DWORDS << 3);
	}

	cdcfg->cdtab = dmam_alloc_coherent(smmu->dev, l1size, &cdcfg->cdtab_dma,
					   GFP_KERNEL);
	if (!cdcfg->cdtab) {
		dev_warn(smmu->dev, "failed to allocate context descriptor\n");
		ret = -ENOMEM;
		goto err_free_l1;
	}

	return 0;

err_free_l1:
	if (cdcfg->l1_desc) {
		devm_kfree(smmu->dev, cdcfg->l1_desc);
		cdcfg->l1_desc = NULL;
	}
	return ret;
}

static void arm_smmu_free_cd_tables(struct arm_smmu_domain *smmu_domain)
{
	int i;
	size_t size, l1size;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_ctx_desc_cfg *cdcfg = &smmu_domain->s1_cfg.cdcfg;

	if (cdcfg->l1_desc) {
		size = CTXDESC_L2_ENTRIES * (CTXDESC_CD_DWORDS << 3);

		for (i = 0; i < cdcfg->num_l1_ents; i++) {
			if (!cdcfg->l1_desc[i].l2ptr)
				continue;

			dmam_free_coherent(smmu->dev, size,
					   cdcfg->l1_desc[i].l2ptr,
					   cdcfg->l1_desc[i].l2ptr_dma);
		}
		devm_kfree(smmu->dev, cdcfg->l1_desc);
		cdcfg->l1_desc = NULL;

		l1size = cdcfg->num_l1_ents * (CTXDESC_L1_DESC_DWORDS << 3);
	} else {
		l1size = cdcfg->num_l1_ents * (CTXDESC_CD_DWORDS << 3);
	}

	dmam_free_coherent(smmu->dev, l1size, cdcfg->cdtab, cdcfg->cdtab_dma);
	cdcfg->cdtab_dma = 0;
	cdcfg->cdtab = NULL;
}
#endif

bool arm_smmu_free_asid(struct arm_smmu_ctx_desc *cd)
{
	bool free;
	struct arm_smmu_ctx_desc *old_cd;

	if (!cd->asid)
		return false;

	free = refcount_dec_and_test(&cd->refs);
	if (free) {
		old_cd = xa_erase(&arm_smmu_asid_xa, cd->asid);
		WARN_ON(old_cd != cd);
	}
	return free;
}

/* Stream table manipulation functions */
static void
arm_smmu_write_strtab_l1_desc(__le64 *dst, struct arm_smmu_strtab_l1_desc *desc)
{
	u64 val = 0;

	val |= FIELD_PREP(STRTAB_L1_DESC_SPAN, desc->span);
	val |= desc->l2ptr_dma & STRTAB_L1_DESC_L2PTR_MASK;

	/* See comment in arm_smmu_write_ctx_desc() */
	WRITE_ONCE(*dst, cpu_to_le64(val));
}

static void arm_smmu_sync_ste_for_sid(struct arm_smmu_device *smmu, u32 sid)
{
	struct arm_smmu_cmdq_ent cmd = {
		.opcode	= CMDQ_OP_CFGI_STE,
		.cfgi	= {
			.sid	= sid,
			.leaf	= true,
		},
	};

	arm_smmu_cmdq_issue_cmd_with_sync(smmu, &cmd);
}

static void arm_smmu_write_strtab_ent(struct arm_smmu_master *master, u32 sid,
				      __le64 *dst)
{
	/*
	 * This is hideously complicated, but we only really care about
	 * three cases at the moment:
	 *
	 * 1. Invalid (all zero) -> bypass/fault (init)
	 * 2. Bypass/fault -> translation/bypass (attach)
	 * 3. Translation/bypass -> bypass/fault (detach)
	 *
	 * Given that we can't update the STE atomically and the SMMU
	 * doesn't read the thing in a defined order, that leaves us
	 * with the following maintenance requirements:
	 *
	 * 1. Update Config, return (init time STEs aren't live)
	 * 2. Write everything apart from dword 0, sync, write dword 0, sync
	 * 3. Update Config, sync
	 */
	u64 val = le64_to_cpu(dst[0]);
	bool ste_live = false;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_s1_cfg *s1_cfg = NULL;
	struct arm_smmu_s2_cfg *s2_cfg = NULL;
	struct arm_smmu_domain *smmu_domain = NULL;
#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	struct arm_smmu_cmdq_ent prefetch_cmd = {
		.opcode		= CMDQ_OP_PREFETCH_CFG,
		.prefetch	= {
			.sid	= sid,
		},
	};
#endif
	if (master) {
		smmu_domain = master->domain;
		smmu = master->smmu;
	}

	if (smmu_domain) {
		switch (smmu_domain->stage) {
		case ARM_SMMU_DOMAIN_S1:
			s1_cfg = &smmu_domain->s1_cfg;
			break;
		case ARM_SMMU_DOMAIN_S2:
		case ARM_SMMU_DOMAIN_NESTED:
			s2_cfg = &smmu_domain->s2_cfg;
			break;
		default:
			break;
		}
	}

	if (val & STRTAB_STE_0_V) {
		switch (FIELD_GET(STRTAB_STE_0_CFG, val)) {
		case STRTAB_STE_0_CFG_BYPASS:
			break;
		case STRTAB_STE_0_CFG_S1_TRANS:
		case STRTAB_STE_0_CFG_S2_TRANS:
			ste_live = true;
			break;
		case STRTAB_STE_0_CFG_ABORT:
			WARN_ON(!disable_bypass);
			break;
		default:
			WARN_ON(true); /* STE corruption */
			break;
		}
	}

	/* Nuke the existing STE_0 value, as we're going to rewrite it */
	val = STRTAB_STE_0_V;

	/* Bypass/fault */
	if (!smmu_domain || !(s1_cfg || s2_cfg)) {
		if (!smmu_domain && disable_bypass)
			val |= FIELD_PREP(STRTAB_STE_0_CFG, STRTAB_STE_0_CFG_ABORT);
		else
			val |= FIELD_PREP(STRTAB_STE_0_CFG, STRTAB_STE_0_CFG_BYPASS);

		dst[0] = cpu_to_le64(val);
		dst[1] = cpu_to_le64(FIELD_PREP(STRTAB_STE_1_SHCFG,
						STRTAB_STE_1_SHCFG_INCOMING));
		dst[2] = 0; /* Nuke the VMID */
		/*
		 * The SMMU can perform negative caching, so we must sync
		 * the STE regardless of whether the old value was live.
		 */
#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
		if (smmu)
			arm_smmu_sync_ste_for_sid(smmu, sid);
#endif
		return;
	}

	if (s1_cfg) {
		u64 strw = smmu->features & ARM_SMMU_FEAT_E2H ?
			STRTAB_STE_1_STRW_EL2 : STRTAB_STE_1_STRW_NSEL1;

		WARN_ON(ste_live);
		dst[1] = cpu_to_le64(
			 FIELD_PREP(STRTAB_STE_1_S1DSS, STRTAB_STE_1_S1DSS_SSID0) |
			 FIELD_PREP(STRTAB_STE_1_S1CIR, STRTAB_STE_1_S1C_CACHE_WBRA) |
			 FIELD_PREP(STRTAB_STE_1_S1COR, STRTAB_STE_1_S1C_CACHE_WBRA) |
			 FIELD_PREP(STRTAB_STE_1_S1CSH, ARM_SMMU_SH_ISH) |
			 FIELD_PREP(STRTAB_STE_1_STRW, strw) |
			 FIELD_PREP(STRTAB_STE_1_PRIVCFG, STRTAB_STE_1_PRIVCFG_UNPRIVILEGED));

		if (smmu->features & ARM_SMMU_FEAT_STALLS &&
		    !master->stall_enabled)
			dst[1] |= cpu_to_le64(STRTAB_STE_1_S1STALLD);

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
		val |= (s1_cfg->cdcfg->cdtab_dma & STRTAB_STE_0_S1CTXPTR_MASK) |
			FIELD_PREP(STRTAB_STE_0_CFG, STRTAB_STE_0_CFG_S1_TRANS) |
			FIELD_PREP(STRTAB_STE_0_S1CDMAX, smmu->cdcfg.s1cdmax) |
			FIELD_PREP(STRTAB_STE_0_S1FMT, smmu->cdcfg.s1fmt);
#else
		val |= (s1_cfg->cdcfg.cdtab_dma & STRTAB_STE_0_S1CTXPTR_MASK) |
			FIELD_PREP(STRTAB_STE_0_CFG, STRTAB_STE_0_CFG_S1_TRANS) |
			FIELD_PREP(STRTAB_STE_0_S1CDMAX, s1_cfg->s1cdmax) |
			FIELD_PREP(STRTAB_STE_0_S1FMT, s1_cfg->s1fmt);
#endif
	}

	if (s2_cfg) {
		WARN_ON(ste_live);
		dst[2] = cpu_to_le64(
			 FIELD_PREP(STRTAB_STE_2_S2VMID, s2_cfg->vmid) |
			 FIELD_PREP(STRTAB_STE_2_VTCR, s2_cfg->vtcr) |
#ifdef __BIG_ENDIAN
			 STRTAB_STE_2_S2ENDI |
#endif
			 STRTAB_STE_2_S2PTW | STRTAB_STE_2_S2AA64 |
			 STRTAB_STE_2_S2R);

		dst[3] = cpu_to_le64(s2_cfg->vttbr & STRTAB_STE_3_S2TTB_MASK);

		val |= FIELD_PREP(STRTAB_STE_0_CFG, STRTAB_STE_0_CFG_S2_TRANS);
	}

	if (master && master->ats_enabled)
		dst[1] |= cpu_to_le64(FIELD_PREP(STRTAB_STE_1_EATS,
						 STRTAB_STE_1_EATS_TRANS));

#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	arm_smmu_sync_ste_for_sid(smmu, sid);
	/* See comment in arm_smmu_write_ctx_desc() */
	WRITE_ONCE(dst[0], cpu_to_le64(val));
	arm_smmu_sync_ste_for_sid(smmu, sid);

	/* It's likely that we'll want to use the new STE soon */
	if (!(smmu->options & ARM_SMMU_OPT_SKIP_PREFETCH))
		arm_smmu_cmdq_issue_cmd(smmu, &prefetch_cmd);
#else
	WRITE_ONCE(dst[0], cpu_to_le64(val));
#endif
}

static void arm_smmu_init_bypass_stes(__le64 *strtab, unsigned int nent)
{
	unsigned int i;

	for (i = 0; i < nent; ++i) {
		arm_smmu_write_strtab_ent(NULL, -1, strtab);
		strtab += STRTAB_STE_DWORDS;
	}
}

static int arm_smmu_init_l2_strtab(struct arm_smmu_device *smmu, u32 sid)
{
	size_t size;
	void *strtab;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;
	struct arm_smmu_strtab_l1_desc *desc = &cfg->l1_desc[sid >> STRTAB_SPLIT];

	if (desc->l2ptr)
		return 0;

	size = 1 << (STRTAB_SPLIT + ilog2(STRTAB_STE_DWORDS) + 3);
	strtab = &cfg->strtab[(sid >> STRTAB_SPLIT) * STRTAB_L1_DESC_DWORDS];

	desc->span = STRTAB_SPLIT + 1;
	desc->l2ptr = dmam_alloc_coherent(smmu->dev, size, &desc->l2ptr_dma,
					  GFP_KERNEL);
	if (!desc->l2ptr) {
		dev_err(smmu->dev,
			"failed to allocate l2 stream table for SID %u\n",
			sid);
		return -ENOMEM;
	}

	arm_smmu_init_bypass_stes(desc->l2ptr, 1 << STRTAB_SPLIT);
	arm_smmu_write_strtab_l1_desc(strtab, desc);
	return 0;
}

static void xring_smmu_dump_reg_val(struct arm_smmu_device *smmu)
{
	struct xring_smmu_wrapper *wrapper = NULL;
	int i = 0;

	wrapper = smmu->wrapper;
	xring_smmu_err("IDR0:0x%x\n", readl(smmu->base + ARM_SMMU_IDR0));
	xring_smmu_err("IDR1:0x%x\n", readl(smmu->base + ARM_SMMU_IDR1));
	xring_smmu_err("IDR3:0x%x\n", readl(smmu->base + ARM_SMMU_IDR3));
	xring_smmu_err("IDR5:0x%x\n", readl(smmu->base + ARM_SMMU_IDR5));
	xring_smmu_err("CR0:0x%x\n", readl(smmu->base + ARM_SMMU_CR0));
	xring_smmu_err("CR0ACK:0x%x\n", readl(smmu->base + ARM_SMMU_CR0ACK));
	xring_smmu_err("CR1:0x%x\n", readl(smmu->base + ARM_SMMU_CR1));
	xring_smmu_err("CR2:0x%x\n", readl(smmu->base + ARM_SMMU_CR2));
	xring_smmu_err("GBPA:0x%x\n", readl(smmu->base + ARM_SMMU_GBPA));
	xring_smmu_err("IRQ_CTRL:0x%x\n", readl(smmu->base + ARM_SMMU_IRQ_CTRL));
	xring_smmu_err("IRQ_CTRLACK:0x%x\n", readl(smmu->base + ARM_SMMU_IRQ_CTRLACK));
	xring_smmu_err("GERROR:0x%x\n", readl(smmu->base + ARM_SMMU_GERROR));
	xring_smmu_err("GERRORN:0x%x\n", readl(smmu->base + ARM_SMMU_GERRORN));
	xring_smmu_err("STRTAB_BASE:0x%llx\n", readq(smmu->base + ARM_SMMU_STRTAB_BASE));
	xring_smmu_err("STRTAB_BASE_CFG:0x%x\n", readl(smmu->base + ARM_SMMU_STRTAB_BASE_CFG));
	xring_smmu_err("CMDQ_BASE:0x%llx\n", readq(smmu->base + ARM_SMMU_CMDQ_BASE));
	xring_smmu_err("CMDQ_PROD:0x%x\n", readl(smmu->base + ARM_SMMU_CMDQ_PROD));
	xring_smmu_err("CMDQ_CONS:0x%x\n", readl(smmu->base + ARM_SMMU_CMDQ_CONS));
	xring_smmu_err("EVTQ_BASE:0x%llx\n", readq(smmu->base + ARM_SMMU_EVTQ_BASE));
	xring_smmu_err("EVTQ_PROD:0x%x\n", readl(smmu->page1 + ARM_SMMU_EVTQ_PROD));
	xring_smmu_err("EVTQ_CONS:0x%x\n", readl(smmu->page1 + ARM_SMMU_EVTQ_CONS));

	xring_smmu_err("TCU status:\n");
	xring_smmu_err("TCU_QCH_CG_CTRL:0x%x\n",
		readl(wrapper->tcu.base + ARM_SMMU_TCU_QCH_CG_CTRL));
	xring_smmu_err("TCU_QCH_CG_STATE:0x%x\n",
		readl(wrapper->tcu.base + ARM_SMMU_TCU_QCH_CG_STATE));
	xring_smmu_err("TCU_QCH_PD_STATE:0x%x\n",
		readl(wrapper->tcu.base + ARM_SMMU_TCU_QCH_PD_CTRL));
	xring_smmu_err("TCU_QCH_PD_STATE:0x%x\n",
		readl(wrapper->tcu.base + ARM_SMMU_TCU_QCH_PD_STATE));
	for (i = 0; i < XRING_SMMU_TBU_MAX_NUMBER; i++) {
		if (!wrapper->tbu[i].base)
			continue;
		xring_smmu_err("TBU %d Status:\n", i);
		xring_smmu_err("TBU_QCH_CG_CTRL:0x%x\n",
			readl(wrapper->tbu[i].base + ARM_SMMU_TBU_QCH_CG_CTRL));
		xring_smmu_err("TBU_QCH_CG_STATE:0x%x\n",
			readl(wrapper->tbu[i].base + ARM_SMMU_TBU_QCH_CG_STATE));
		xring_smmu_err("TBU_QCH_PD_STATE:0x%x\n",
			readl(wrapper->tbu[i].base + ARM_SMMU_TBU_QCH_PD_CTRL));
		xring_smmu_err("TBU_QCH_PD_STATE:0x%x\n",
			readl(wrapper->tbu[i].base + ARM_SMMU_TBU_QCH_PD_STATE));
	}
}

static irqreturn_t arm_smmu_evtq_thread(int irq, void *dev)
{
	int i, ret;
	struct arm_smmu_device *smmu = dev;
	struct arm_smmu_queue *q = &smmu->evtq.q;
	struct arm_smmu_ll_queue *llq = &q->llq;
	u64 pre_evt = 0;
	u64 evt[EVTQ_ENT_DWORDS];
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	unsigned long flags;

	spin_lock_irqsave(&smmu->tcu_lock, flags);

	ret = xring_smmu_tcu_link_check(smmu);
	if (ret < 0) {
		spin_unlock_irqrestore(&smmu->tcu_lock, flags);
		xring_smmu_err("xring_smmu_tcu_link_check fail\n");
		return IRQ_HANDLED;
	}
#endif

	do {
		while (!queue_remove_raw(q, evt)) {
			u8 id = FIELD_GET(EVTQ_0_ID, evt[0]);

			if (pre_evt == evt[0])
				continue;
			pre_evt = evt[0];

			dev_err(smmu->dev, "event 0x%02x received:\n", id);
			for (i = 0; i < ARRAY_SIZE(evt); ++i)
				dev_err(smmu->dev, "\t0x%016llx\n",
					 (unsigned long long)evt[i]);

			xring_smmu_dump_reg_val(smmu);
			xring_smmu_dump_struct(smmu, evt, EVTQ_ENT_DWORDS);
			xring_smmu_dump_pgtbl(smmu, evt, EVTQ_ENT_DWORDS, 1);
		}

		/*
		 * Not much we can do on overflow, so scream and pretend we're
		 * trying harder.
		 */
		if (queue_sync_prod_in(q) == -EOVERFLOW)
			dev_err(smmu->dev, "EVTQ overflow detected -- events lost\n");
	} while (!queue_empty(llq));

	/* Sync our overflow flag, as we believe we're up to speed */
	llq->cons = Q_OVF(llq->prod) | Q_WRP(llq, llq->cons) |
		    Q_IDX(llq, llq->cons);

	xring_smmu_tcu_irpt_clear(smmu, SC_SMMU_TCU_EVENTQ_IRPT_CLR);

#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);
#endif
	return IRQ_HANDLED;
}

static void arm_smmu_handle_ppr(struct arm_smmu_device *smmu, u64 *evt)
{
	u32 sid, ssid;
	u16 grpid;
	bool ssv, last;

	sid = FIELD_GET(PRIQ_0_SID, evt[0]);
	ssv = FIELD_GET(PRIQ_0_SSID_V, evt[0]);
	ssid = ssv ? FIELD_GET(PRIQ_0_SSID, evt[0]) : 0;
	last = FIELD_GET(PRIQ_0_PRG_LAST, evt[0]);
	grpid = FIELD_GET(PRIQ_1_PRG_IDX, evt[1]);

	dev_info(smmu->dev, "unexpected PRI request received:\n");
	dev_info(smmu->dev,
		 "\tsid 0x%08x.0x%05x: [%u%s] %sprivileged %s%s%s access at iova 0x%016llx\n",
		 sid, ssid, grpid, last ? "L" : "",
		 evt[0] & PRIQ_0_PERM_PRIV ? "" : "un",
		 evt[0] & PRIQ_0_PERM_READ ? "R" : "",
		 evt[0] & PRIQ_0_PERM_WRITE ? "W" : "",
		 evt[0] & PRIQ_0_PERM_EXEC ? "X" : "",
		 evt[1] & PRIQ_1_ADDR_MASK);

	if (last) {
		struct arm_smmu_cmdq_ent cmd = {
			.opcode			= CMDQ_OP_PRI_RESP,
			.substream_valid	= ssv,
			.pri			= {
				.sid	= sid,
				.ssid	= ssid,
				.grpid	= grpid,
				.resp	= PRI_RESP_DENY,
			},
		};

		arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	}
}

static irqreturn_t arm_smmu_priq_thread(int irq, void *dev)
{
	struct arm_smmu_device *smmu = dev;
	struct arm_smmu_queue *q = &smmu->priq.q;
	struct arm_smmu_ll_queue *llq = &q->llq;
	u64 evt[PRIQ_ENT_DWORDS];

	do {
		while (!queue_remove_raw(q, evt))
			arm_smmu_handle_ppr(smmu, evt);

		if (queue_sync_prod_in(q) == -EOVERFLOW)
			dev_err(smmu->dev, "PRIQ overflow detected -- requests lost\n");
	} while (!queue_empty(llq));

	/* Sync our overflow flag, as we believe we're up to speed */
	llq->cons = Q_OVF(llq->prod) | Q_WRP(llq, llq->cons) |
		      Q_IDX(llq, llq->cons);
	queue_sync_cons_out(q);
	return IRQ_HANDLED;
}

static int arm_smmu_device_disable(struct arm_smmu_device *smmu);

static irqreturn_t arm_smmu_gerror_handler(int irq, void *dev)
{
	u32 gerror, gerrorn, active;
	struct arm_smmu_device *smmu = dev;
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	int ret;

	spin_lock(&smmu->tcu_lock);
	ret = xring_smmu_tcu_link_check(smmu);
	if (ret < 0) {
		spin_unlock(&smmu->tcu_lock);
		xring_smmu_err("xring_smmu_tcu_link_check fail\n");
		return IRQ_HANDLED;
	}
#endif

	gerror = readl_relaxed(smmu->base + ARM_SMMU_GERROR);
	gerrorn = readl_relaxed(smmu->base + ARM_SMMU_GERRORN);

	active = gerror ^ gerrorn;
	if (!(active & GERROR_ERR_MASK)) {
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
		spin_unlock(&smmu->tcu_lock);
#endif
		return IRQ_NONE; /* No errors pending */
	}

	dev_warn(smmu->dev,
		 "unexpected global error reported (0x%08x), this could be serious\n",
		 active);

	if (active & GERROR_SFM_ERR) {
		dev_err(smmu->dev, "device has entered Service Failure Mode!\n");
		arm_smmu_device_disable(smmu);
	}

	if (active & GERROR_MSI_GERROR_ABT_ERR)
		dev_warn(smmu->dev, "GERROR MSI write aborted\n");

	if (active & GERROR_MSI_PRIQ_ABT_ERR)
		dev_warn(smmu->dev, "PRIQ MSI write aborted\n");

	if (active & GERROR_MSI_EVTQ_ABT_ERR)
		dev_warn(smmu->dev, "EVTQ MSI write aborted\n");

	if (active & GERROR_MSI_CMDQ_ABT_ERR)
		dev_warn(smmu->dev, "CMDQ MSI write aborted\n");

	if (active & GERROR_PRIQ_ABT_ERR)
		dev_err(smmu->dev, "PRIQ write aborted -- events may have been lost\n");

	if (active & GERROR_EVTQ_ABT_ERR)
		dev_err(smmu->dev, "EVTQ write aborted -- events may have been lost\n");

	if (active & GERROR_CMDQ_ERR)
		arm_smmu_cmdq_skip_err(smmu);

	writel(gerror, smmu->base + ARM_SMMU_GERRORN);

	xring_smmu_tcu_irpt_clear(smmu, SC_SMMU_TCU_GERROR_IRPT_CLR);

#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	spin_unlock(&smmu->tcu_lock);
#endif
	return IRQ_HANDLED;
}

static irqreturn_t arm_smmu_cmdq_sync_handler(int irq, void *dev)
{
	struct arm_smmu_device *smmu = dev;
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	int ret;

	spin_lock(&smmu->tcu_lock);
	ret = xring_smmu_tcu_link_check(smmu);
	if (ret < 0) {
		spin_unlock(&smmu->tcu_lock);
		xring_smmu_err("xring_smmu_tcu_link_check fail\n");
		return IRQ_HANDLED;
	}
#endif
	xring_smmu_tcu_irpt_clear(smmu, SC_SMMU_TCU_CMDQ_SYNC_IRPT_CLR);
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	spin_unlock(&smmu->tcu_lock);
#endif
	return IRQ_HANDLED;
}

static irqreturn_t arm_smmu_tcu_ras_handler(int irq, void *dev)
{
	struct arm_smmu_device *smmu = dev;

	/* check if regulator is power on, return warning or continue */
	if (!regulator_is_enabled(smmu->master_rg)) {
		xring_smmu_err("regulator is not power on, do nothing\n");
		return -1;
	}

	xring_smmu_tcu_irpt_clear(smmu, SC_SMMU_TCU_RAS_IRPT_CLR);

	return IRQ_HANDLED;
}

#ifdef XRING_SMMU_TBU_IRQ
static irqreturn_t arm_smmu_tbu_ras_handler(int irq, void *dev)
{
	struct xring_smmu_tbu *tbu = dev;
	struct xring_smmu_wrapper *wrapper =
			container_of(tbu, struct xring_smmu_wrapper, tbu);
	struct arm_smmu_device *smmu = wrapper->smmu;

	/* check if regulator is power on, return warning or continue */
	if (!regulator_is_enabled(smmu->master_rg)) {
		xring_smmu_err("regulator is not power on, do nothing\n");
		return -1;
	}

	xring_smmu_tbu_irpt_clear(tbu, SC_SMMU_TBU_RAS_IRPT_CLR);

	return IRQ_HANDLED;
}
#endif

static irqreturn_t arm_smmu_combined_irq_thread(int irq, void *dev)
{
	struct arm_smmu_device *smmu = dev;

	arm_smmu_evtq_thread(irq, dev);

	if (smmu->features & ARM_SMMU_FEAT_PRI)
		arm_smmu_priq_thread(irq, dev);

	return IRQ_HANDLED;
}

static irqreturn_t arm_smmu_combined_irq_handler(int irq, void *dev)
{
	arm_smmu_gerror_handler(irq, dev);
	return IRQ_WAKE_THREAD;
}

static void
arm_smmu_atc_inv_to_cmd(int ssid, unsigned long iova, size_t size,
			struct arm_smmu_cmdq_ent *cmd)
{
	size_t log2_span;
	size_t span_mask;
	/* ATC invalidates are always on 4096-bytes pages */
	size_t inval_grain_shift = 12;
	unsigned long page_start, page_end;

	/*
	 * ATS and PASID:
	 *
	 * If substream_valid is clear, the PCIe TLP is sent without a PASID
	 * prefix. In that case all ATC entries within the address range are
	 * invalidated, including those that were requested with a PASID! There
	 * is no way to invalidate only entries without PASID.
	 *
	 * When using STRTAB_STE_1_S1DSS_SSID0 (reserving CD 0 for non-PASID
	 * traffic), translation requests without PASID create ATC entries
	 * without PASID, which must be invalidated with substream_valid clear.
	 * This has the unpleasant side-effect of invalidating all PASID-tagged
	 * ATC entries within the address range.
	 */
	*cmd = (struct arm_smmu_cmdq_ent) {
		.opcode			= CMDQ_OP_ATC_INV,
		.substream_valid	= !!ssid,
		.atc.ssid		= ssid,
	};

	if (!size) {
		cmd->atc.size = ATC_INV_SIZE_ALL;
		return;
	}

	page_start	= iova >> inval_grain_shift;
	page_end	= (iova + size - 1) >> inval_grain_shift;

	/*
	 * In an ATS Invalidate Request, the address must be aligned on the
	 * range size, which must be a power of two number of page sizes. We
	 * thus have to choose between grossly over-invalidating the region, or
	 * splitting the invalidation into multiple commands. For simplicity
	 * we'll go with the first solution, but should refine it in the future
	 * if multiple commands are shown to be more efficient.
	 *
	 * Find the smallest power of two that covers the range. The most
	 * significant differing bit between the start and end addresses,
	 * fls(start ^ end), indicates the required span. For example:
	 *
	 * We want to invalidate pages [8; 11]. This is already the ideal range:
	 *		x = 0b1000 ^ 0b1011 = 0b11
	 *		span = 1 << fls(x) = 4
	 *
	 * To invalidate pages [7; 10], we need to invalidate [0; 15]:
	 *		x = 0b0111 ^ 0b1010 = 0b1101
	 *		span = 1 << fls(x) = 16
	 */
	log2_span	= fls_long(page_start ^ page_end);
	span_mask	= (1ULL << log2_span) - 1;

	page_start	&= ~span_mask;

	cmd->atc.addr	= page_start << inval_grain_shift;
	cmd->atc.size	= log2_span;
}

static int arm_smmu_atc_inv_master(struct arm_smmu_master *master)
{
#ifndef CONFIG_XRING_SMMU_DOMAIN_SHARE
	int i;
#endif
	struct arm_smmu_cmdq_ent cmd;
	struct arm_smmu_cmdq_batch cmds = {};

	arm_smmu_atc_inv_to_cmd(0, 0, 0, &cmd);

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	cmd.atc.sid = master->sid;
	arm_smmu_cmdq_batch_add(master->smmu, &cmds, &cmd);
#else
	for (i = 0; i < master->num_streams; i++) {
		cmd.atc.sid = master->streams[i].id;
		arm_smmu_cmdq_batch_add(master->smmu, &cmds, &cmd);
	}
#endif

	return arm_smmu_cmdq_batch_submit(master->smmu, &cmds);
}

int arm_smmu_atc_inv_domain(struct arm_smmu_domain *smmu_domain, int ssid,
			    unsigned long iova, size_t size)
{
#ifndef CONFIG_XRING_SMMU_DOMAIN_SHARE
	int i;
#endif
	unsigned long flags;
	struct arm_smmu_cmdq_ent cmd;
	struct arm_smmu_master *master = NULL;
	struct arm_smmu_cmdq_batch cmds;

	if (!(smmu_domain->smmu->features & ARM_SMMU_FEAT_ATS))
		return 0;

	/*
	 * Ensure that we've completed prior invalidation of the main TLBs
	 * before we read 'nr_ats_masters' in case of a concurrent call to
	 * arm_smmu_enable_ats():
	 *
	 *	// unmap()			// arm_smmu_enable_ats()
	 *	TLBI+SYNC			atomic_inc(&nr_ats_masters);
	 *	smp_mb();			[...]
	 *	atomic_read(&nr_ats_masters);	pci_enable_ats() // writel()
	 *
	 * Ensures that we always see the incremented 'nr_ats_masters' count if
	 * ATS was enabled at the PCI device before completion of the TLBI.
	 */
	smp_mb();
	if (!atomic_read(&smmu_domain->nr_ats_masters))
		return 0;

	arm_smmu_atc_inv_to_cmd(ssid, iova, size, &cmd);

	cmds.num = 0;

	spin_lock_irqsave(&smmu_domain->devices_lock, flags);
	list_for_each_entry(master, &smmu_domain->devices, domain_head) {
		if (!master->ats_enabled)
			continue;

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
		cmd.atc.sid = master->sid;
		arm_smmu_cmdq_batch_add(smmu_domain->smmu, &cmds, &cmd);
#else
		for (i = 0; i < master->num_streams; i++) {
			cmd.atc.sid = master->streams[i].id;
			arm_smmu_cmdq_batch_add(smmu_domain->smmu, &cmds, &cmd);
		}
#endif
	}
	spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);

	return arm_smmu_cmdq_batch_submit(smmu_domain->smmu, &cmds);
}

/* IO_PGTABLE API */
static void arm_smmu_tlb_inv_context(void *cookie)
{
	struct arm_smmu_domain *smmu_domain = cookie;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_cmdq_ent cmd;

	/*
	 * NOTE: when io-pgtable is in non-strict mode, we may get here with
	 * PTEs previously cleared by unmaps on the current CPU not yet visible
	 * to the SMMU. We are relying on the dma_wmb() implicit during cmd
	 * insertion to guarantee those are observed before the TLBI. Do be
	 * careful, 007.
	 */
	if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		arm_smmu_tlb_inv_asid(smmu, smmu_domain->s1_cfg.cd.asid);
	} else {
		cmd.opcode	= CMDQ_OP_TLBI_S12_VMALL;
		cmd.tlbi.vmid	= smmu_domain->s2_cfg.vmid;
		arm_smmu_cmdq_issue_cmd_with_sync(smmu, &cmd);
	}
	arm_smmu_atc_inv_domain(smmu_domain, 0, 0, 0);
}

static void arm_smmu_tlb_inv_context_with_lock(void *cookie)
{
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	struct arm_smmu_domain *smmu_domain = cookie;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&smmu->tcu_lock, flags);

	ret = xring_smmu_tcu_link_check(smmu);
	if (ret < 0) {
		spin_unlock_irqrestore(&smmu->tcu_lock, flags);
		return;
	}
#endif
	arm_smmu_tlb_inv_context(cookie);
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);
#endif
}

static void __arm_smmu_tlb_inv_range_with_lock(struct arm_smmu_cmdq_ent *cmd,
				     unsigned long iova, size_t size,
				     size_t granule,
				     struct arm_smmu_domain *smmu_domain)
{
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	unsigned long end = iova + size, num_pages = 0, tg = 0;
	size_t inv_range = granule;
	struct arm_smmu_cmdq_batch cmds;
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	unsigned long flags;
	int ret;
#endif

	if (!size)
		return;

	if (smmu->features & ARM_SMMU_FEAT_RANGE_INV) {
		/* Get the leaf page size */
		tg = __ffs(smmu_domain->domain.pgsize_bitmap);

		/* Convert page size of 12,14,16 (log2) to 1,2,3 */
		cmd->tlbi.tg = (tg - 10) / 2;

		/* Determine what level the granule is at */
		cmd->tlbi.ttl = 4 - ((ilog2(granule) - 3) / (tg - 3));

		num_pages = size >> tg;
	}

	cmds.num = 0;

	while (iova < end) {
		if (smmu->features & ARM_SMMU_FEAT_RANGE_INV) {
			/*
			 * On each iteration of the loop, the range is 5 bits
			 * worth of the aligned size remaining.
			 * The range in pages is:
			 *
			 * range = (num_pages & (0x1f << __ffs(num_pages)))
			 */
			unsigned long scale, num;

			/* Determine the power of 2 multiple number of pages */
			scale = __ffs(num_pages);
			cmd->tlbi.scale = scale;

			/* Determine how many chunks of 2^scale size we have */
			num = (num_pages >> scale) & CMDQ_TLBI_RANGE_NUM_MAX;
			cmd->tlbi.num = num - 1;

			/* range is num * 2^scale * pgsize */
			inv_range = num << (scale + tg);

			/* Clear out the lower order bits for the next iteration */
			num_pages -= num << scale;
		}

		cmd->tlbi.addr = iova;
		if (cmds.num == CMDQ_BATCH_ENTRIES) {
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
			spin_lock_irqsave(&smmu->tcu_lock, flags);
			ret = xring_smmu_tcu_link_check(smmu);
			if (ret < 0) {
				spin_unlock_irqrestore(&smmu->tcu_lock, flags);
				return;
			}
#endif
			arm_smmu_cmdq_issue_cmdlist(smmu, cmds.cmds, cmds.num, false);

#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
			spin_unlock_irqrestore(&smmu->tcu_lock, flags);
#endif
			cmds.num = 0;
		}

		arm_smmu_cmdq_build_cmd(&cmds.cmds[cmds.num * CMDQ_ENT_DWORDS], cmd);
		cmds.num++;

		iova += inv_range;
	}

#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	spin_lock_irqsave(&smmu->tcu_lock, flags);

	ret = xring_smmu_tcu_link_check(smmu);
	if (ret < 0) {
		spin_unlock_irqrestore(&smmu->tcu_lock, flags);
		return;
	}
#endif
	arm_smmu_cmdq_batch_submit(smmu, &cmds);
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);
#endif
}

static void arm_smmu_tlb_inv_range_domain(unsigned long iova, size_t size,
					  size_t granule, bool leaf,
					  struct arm_smmu_domain *smmu_domain)
{
	struct arm_smmu_cmdq_ent cmd = {
		.tlbi = {
			.leaf	= leaf,
		},
	};

	if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		cmd.opcode	= smmu_domain->smmu->features & ARM_SMMU_FEAT_E2H ?
				  CMDQ_OP_TLBI_EL2_VA : CMDQ_OP_TLBI_NH_VA;
		cmd.tlbi.asid	= smmu_domain->s1_cfg.cd.asid;
	} else {
		cmd.opcode	= CMDQ_OP_TLBI_S2_IPA;
		cmd.tlbi.vmid	= smmu_domain->s2_cfg.vmid;
	}
	__arm_smmu_tlb_inv_range_with_lock(&cmd, iova, size, granule, smmu_domain);

	/*
	 * Unfortunately, this can't be leaf-only since we may have
	 * zapped an entire table.
	 */
	arm_smmu_atc_inv_domain(smmu_domain, 0, iova, size);
}

void arm_smmu_tlb_inv_range_asid(unsigned long iova, size_t size, int asid,
				 size_t granule, bool leaf,
				 struct arm_smmu_domain *smmu_domain)
{
	struct arm_smmu_cmdq_ent cmd = {
		.opcode	= smmu_domain->smmu->features & ARM_SMMU_FEAT_E2H ?
			  CMDQ_OP_TLBI_EL2_VA : CMDQ_OP_TLBI_NH_VA,
		.tlbi = {
			.asid	= asid,
			.leaf	= leaf,
		},
	};

	__arm_smmu_tlb_inv_range_with_lock(&cmd, iova, size, granule, smmu_domain);
}

static void arm_smmu_tlb_inv_page_nosync(struct iommu_iotlb_gather *gather,
					 unsigned long iova, size_t granule,
					 void *cookie)
{
	struct arm_smmu_domain *smmu_domain = cookie;
	struct iommu_domain *domain = &smmu_domain->domain;

	iommu_iotlb_gather_add_page(domain, gather, iova, granule);
}

static void arm_smmu_tlb_inv_walk(unsigned long iova, size_t size,
				  size_t granule, void *cookie)
{
	arm_smmu_tlb_inv_range_domain(iova, size, granule, false, cookie);
}

static const struct iommu_flush_ops arm_smmu_flush_ops = {
	.tlb_flush_all	= arm_smmu_tlb_inv_context_with_lock,
	.tlb_flush_walk = arm_smmu_tlb_inv_walk,
	.tlb_add_page	= arm_smmu_tlb_inv_page_nosync,
};

/* IOMMU API */
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
static bool arm_smmu_capable(enum iommu_cap cap)
#else
static bool arm_smmu_capable(struct device *dev, enum iommu_cap cap)
#endif
{
	switch (cap) {
	case IOMMU_CAP_CACHE_COHERENCY:
		return true;
	case IOMMU_CAP_NOEXEC:
		return true;
	default:
		return false;
	}
}

static struct iommu_domain *arm_smmu_domain_alloc(unsigned int type)
{
	struct arm_smmu_domain *smmu_domain;

	if (type != IOMMU_DOMAIN_UNMANAGED &&
	    type != IOMMU_DOMAIN_DMA &&
	    type != IOMMU_DOMAIN_DMA_FQ &&
	    type != IOMMU_DOMAIN_IDENTITY)
		return NULL;

	/*
	 * Allocate the domain and initialise some of its data structures.
	 * We can't really do anything meaningful until we've added a
	 * master.
	 */
	smmu_domain = kzalloc(sizeof(*smmu_domain), GFP_KERNEL);
	if (!smmu_domain)
		return NULL;

	mutex_init(&smmu_domain->init_mutex);
	INIT_LIST_HEAD(&smmu_domain->devices);
	spin_lock_init(&smmu_domain->devices_lock);
	INIT_LIST_HEAD(&smmu_domain->mmu_notifiers);

	return &smmu_domain->domain;
}

static int arm_smmu_bitmap_alloc(unsigned long *map, int span)
{
	int idx, size = 1 << span;

	do {
		idx = find_first_zero_bit(map, size);
		if (idx == size)
			return -ENOSPC;
	} while (test_and_set_bit(idx, map));

	return idx;
}

static void arm_smmu_bitmap_free(unsigned long *map, int idx)
{
	clear_bit(idx, map);
}

static void arm_smmu_domain_free(struct iommu_domain *domain)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct arm_smmu_device *smmu = smmu_domain->smmu;

	free_io_pgtable_ops(smmu_domain->pgtbl_ops);

	/* Free the CD and ASID, if we allocated them */
	if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;

		/* Prevent SVA from touching the CD while we're freeing it */
		mutex_lock(&arm_smmu_asid_lock);
#ifndef CONFIG_XRING_SMMU_DOMAIN_SHARE
		if (cfg->cdcfg.cdtab)
			arm_smmu_free_cd_tables(smmu_domain);
#endif
		arm_smmu_free_asid(&cfg->cd);
		mutex_unlock(&arm_smmu_asid_lock);
	} else {
		struct arm_smmu_s2_cfg *cfg = &smmu_domain->s2_cfg;

		if (cfg->vmid)
			arm_smmu_bitmap_free(smmu->vmid_map, cfg->vmid);
	}

	kfree(smmu_domain);
}

static int arm_smmu_domain_finalise_s1(struct arm_smmu_domain *smmu_domain,
				       struct arm_smmu_master *master,
				       struct io_pgtable_cfg *pgtbl_cfg)
{
	int ret;
	u32 asid;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;
	typeof(&pgtbl_cfg->arm_lpae_s1_cfg.tcr) tcr = &pgtbl_cfg->arm_lpae_s1_cfg.tcr;

	refcount_set(&cfg->cd.refs, 1);

	/* Prevent SVA from modifying the ASID until it is written to the CD */
	mutex_lock(&arm_smmu_asid_lock);
	ret = xa_alloc(&arm_smmu_asid_xa, &asid, &cfg->cd,
		       XA_LIMIT(1, (1 << smmu->asid_bits) - 1), GFP_KERNEL);
	if (ret)
		goto out_unlock;

	smmu_domain->stall_enabled = master->stall_enabled;
#ifndef CONFIG_XRING_SMMU_DOMAIN_SHARE
	cfg->s1cdmax = master->ssid_bits;
	ret = arm_smmu_alloc_cd_tables(smmu_domain);
	if (ret)
		goto out_free_asid;
#endif

	cfg->cd.asid	= (u16)asid;
	cfg->cd.ttbr	= pgtbl_cfg->arm_lpae_s1_cfg.ttbr;
	cfg->cd.tcr	= FIELD_PREP(CTXDESC_CD_0_TCR_T0SZ, tcr->tsz) |
			  FIELD_PREP(CTXDESC_CD_0_TCR_TG0, tcr->tg) |
			  FIELD_PREP(CTXDESC_CD_0_TCR_IRGN0, tcr->irgn) |
			  FIELD_PREP(CTXDESC_CD_0_TCR_ORGN0, tcr->orgn) |
			  FIELD_PREP(CTXDESC_CD_0_TCR_SH0, tcr->sh) |
			  FIELD_PREP(CTXDESC_CD_0_TCR_IPS, tcr->ips) |
			  CTXDESC_CD_0_TCR_EPD1 | CTXDESC_CD_0_AA64;
	cfg->cd.mair	= pgtbl_cfg->arm_lpae_s1_cfg.mair;

#ifndef CONFIG_XRING_SMMU_DOMAIN_SHARE
	/*
	 * Note that this will end up calling arm_smmu_sync_cd() before
	 * the master has been added to the devices list for this domain.
	 * This isn't an issue because the STE hasn't been installed yet.
	 */
	ret = arm_smmu_write_ctx_desc(smmu_domain, 0, &cfg->cd);
	if (ret)
		goto out_free_cd_tables;
#endif

	mutex_unlock(&arm_smmu_asid_lock);
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	xring_smmu_info("sid:%u, asid:0x%x\n", master->sid, cfg->cd.asid);
#endif
	return 0;

#ifndef CONFIG_XRING_SMMU_DOMAIN_SHARE
out_free_cd_tables:
	arm_smmu_free_cd_tables(smmu_domain);
out_free_asid:
	arm_smmu_free_asid(&cfg->cd);
#endif
out_unlock:
	mutex_unlock(&arm_smmu_asid_lock);
	return ret;
}

static int arm_smmu_domain_finalise_s2(struct arm_smmu_domain *smmu_domain,
				       struct arm_smmu_master *master,
				       struct io_pgtable_cfg *pgtbl_cfg)
{
	int vmid;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_s2_cfg *cfg = &smmu_domain->s2_cfg;
	typeof(&pgtbl_cfg->arm_lpae_s2_cfg.vtcr) vtcr;

	vmid = arm_smmu_bitmap_alloc(smmu->vmid_map, smmu->vmid_bits);
	if (vmid < 0)
		return vmid;

	vtcr = &pgtbl_cfg->arm_lpae_s2_cfg.vtcr;
	cfg->vmid	= (u16)vmid;
	cfg->vttbr	= pgtbl_cfg->arm_lpae_s2_cfg.vttbr;
	cfg->vtcr	= FIELD_PREP(STRTAB_STE_2_VTCR_S2T0SZ, vtcr->tsz) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2SL0, vtcr->sl) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2IR0, vtcr->irgn) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2OR0, vtcr->orgn) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2SH0, vtcr->sh) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2TG, vtcr->tg) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2PS, vtcr->ps);
	return 0;
}

static int arm_smmu_domain_finalise(struct iommu_domain *domain,
				    struct arm_smmu_master *master)
{
	int ret;
	unsigned long ias, oas;
	enum io_pgtable_fmt fmt;
	struct io_pgtable_cfg pgtbl_cfg;
	struct io_pgtable_ops *pgtbl_ops;
	int (*finalise_stage_fn)(struct arm_smmu_domain *domain,
				 struct arm_smmu_master *master,
				 struct io_pgtable_cfg *cfg);
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct arm_smmu_device *smmu = smmu_domain->smmu;

	if (domain->type == IOMMU_DOMAIN_IDENTITY) {
		smmu_domain->stage = ARM_SMMU_DOMAIN_BYPASS;
		return 0;
	}

	/* Restrict the stage to what we can actually support */
	if (!(smmu->features & ARM_SMMU_FEAT_TRANS_S1))
		smmu_domain->stage = ARM_SMMU_DOMAIN_S2;
	if (!(smmu->features & ARM_SMMU_FEAT_TRANS_S2))
		smmu_domain->stage = ARM_SMMU_DOMAIN_S1;

	switch (smmu_domain->stage) {
	case ARM_SMMU_DOMAIN_S1:
		ias = (smmu->features & ARM_SMMU_FEAT_VAX) ? 52 : 48;
		ias = min_t(unsigned long, ias, VA_BITS);
		oas = smmu->ias;
		fmt = XRING_ARM_64_LPAE_S1;
		finalise_stage_fn = arm_smmu_domain_finalise_s1;
		break;
	case ARM_SMMU_DOMAIN_NESTED:
	case ARM_SMMU_DOMAIN_S2:
		ias = smmu->ias;
		oas = smmu->oas;
		fmt = XRING_ARM_64_LPAE_S2;
		finalise_stage_fn = arm_smmu_domain_finalise_s2;
		break;
	default:
		return -EINVAL;
	}

	pgtbl_cfg = (struct io_pgtable_cfg) {
		.pgsize_bitmap	= smmu->pgsize_bitmap,
		.ias		= ias,
		.oas		= oas,
		.coherent_walk	= smmu->features & ARM_SMMU_FEAT_COHERENCY,
		.tlb		= &arm_smmu_flush_ops,
		.iommu_dev	= smmu->dev,
	};

	pgtbl_ops = xring_alloc_io_pgtable_ops(fmt, &pgtbl_cfg, smmu_domain);
	if (!pgtbl_ops)
		return -ENOMEM;

	domain->pgsize_bitmap = pgtbl_cfg.pgsize_bitmap;
	domain->geometry.aperture_end = (1UL << pgtbl_cfg.ias) - 1;
	domain->geometry.force_aperture = true;

	ret = finalise_stage_fn(smmu_domain, master, &pgtbl_cfg);
	if (ret < 0) {
		free_io_pgtable_ops(pgtbl_ops);
		return ret;
	}

	smmu_domain->pgtbl_ops = pgtbl_ops;
	return 0;
}

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

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
static void arm_smmu_install_ste_for_dev(struct arm_smmu_master *master)
{
	struct arm_smmu_device *smmu = master->smmu;

	u32 sid = master->sid;
	__le64 *step = arm_smmu_get_step_for_sid(smmu, sid);

	arm_smmu_write_strtab_ent(master, sid, step);
}
#else
static void arm_smmu_install_ste_for_dev(struct arm_smmu_master *master)
{
	int i, j;
	struct arm_smmu_device *smmu = master->smmu;

	for (i = 0; i < master->num_streams; ++i) {
		u32 sid = master->streams[i].id;
		__le64 *step = arm_smmu_get_step_for_sid(smmu, sid);

		/* Bridged PCI devices may end up with duplicated IDs */
		for (j = 0; j < i; j++)
			if (master->streams[j].id == sid)
				break;
		if (j < i)
			continue;

		arm_smmu_write_strtab_ent(master, sid, step);
	}
}
#endif

static bool arm_smmu_ats_supported(struct arm_smmu_master *master)
{
	struct device *dev = master->dev;
	struct arm_smmu_device *smmu = master->smmu;
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);

	if (!fwspec)
		return false;

	if (!(smmu->features & ARM_SMMU_FEAT_ATS))
		return false;

	if (!(fwspec->flags & IOMMU_FWSPEC_PCI_RC_ATS))
		return false;

	return dev_is_pci(dev) && pci_ats_supported(to_pci_dev(dev));
}

static void arm_smmu_enable_ats(struct arm_smmu_master *master)
{
	size_t stu;
	struct pci_dev *pdev;
	struct arm_smmu_device *smmu = master->smmu;
	struct arm_smmu_domain *smmu_domain = master->domain;

	/* Don't enable ATS at the endpoint if it's not enabled in the STE */
	if (!master->ats_enabled)
		return;

	/* Smallest Translation Unit: log2 of the smallest supported granule */
	stu = __ffs(smmu->pgsize_bitmap);
	pdev = to_pci_dev(master->dev);

	atomic_inc(&smmu_domain->nr_ats_masters);
	arm_smmu_atc_inv_domain(smmu_domain, 0, 0, 0);
	if (pci_enable_ats(pdev, stu))
		dev_err(master->dev, "Failed to enable ATS (STU %zu)\n", stu);
}

static void arm_smmu_disable_ats(struct arm_smmu_master *master)
{
	struct arm_smmu_domain *smmu_domain = master->domain;

	if (!master->ats_enabled)
		return;

	pci_disable_ats(to_pci_dev(master->dev));
	/*
	 * Ensure ATS is disabled at the endpoint before we issue the
	 * ATC invalidation via the SMMU.
	 */
	wmb();
	arm_smmu_atc_inv_master(master);
	atomic_dec(&smmu_domain->nr_ats_masters);
}

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
static void arm_smmu_detach_dev(struct arm_smmu_master *master)
{
	unsigned long flags;
	struct arm_smmu_domain *smmu_domain = master->domain;

	if (!smmu_domain)
		return;

	arm_smmu_disable_ats(master);

	if (master->domain_head.next) {
		spin_lock_irqsave(&smmu_domain->devices_lock, flags);
		list_del(&master->domain_head);
		spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);
	}

	master->domain = NULL;
	master->ats_enabled = false;
	arm_smmu_install_ste_for_dev(master);
	master->assigned = false;
}

static void arm_smmu_install_cd_for_dev(struct arm_smmu_domain *smmu_domain,
				struct arm_smmu_master *master)
{
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;
	int i;

	cfg->cdcfg = &master->smmu->cdcfg;
	if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		for (i = 0; i < master->num_ssids; i++)
			arm_smmu_write_ctx_desc(smmu_domain, master->ssids[i], &cfg->cd);
	}
}
#else
static void arm_smmu_detach_dev(struct arm_smmu_master *master)
{
	unsigned long flags;
	struct arm_smmu_domain *smmu_domain = master->domain;

	if (!smmu_domain)
		return;

	arm_smmu_disable_ats(master);

	spin_lock_irqsave(&smmu_domain->devices_lock, flags);
	list_del(&master->domain_head);
	spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);

	master->domain = NULL;
	master->ats_enabled = false;
	arm_smmu_install_ste_for_dev(master);
}
#endif

static int arm_smmu_attach_dev(struct iommu_domain *domain, struct device *dev)
{
	int ret = 0;
	unsigned long flags;
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);
	struct arm_smmu_device *smmu;
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct arm_smmu_master *master;

	if (!fwspec)
		return -ENOENT;

	master = dev_iommu_priv_get(dev);
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	if (master->assigned) {
		dev_err(dev, "Device already attached to a domain\n");
		return ret;
	}
#endif

	if (strstr(dev_name(dev), "xrisp") && (isp_iommu_domain != domain))
		isp_iommu_domain = domain;

	smmu = master->smmu;

	/*
	 * Checking that SVA is disabled ensures that this device isn't bound to
	 * any mm, and can be safely detached from its old domain. Bonds cannot
	 * be removed concurrently since we're holding the group mutex.
	 */
	if (arm_smmu_master_sva_enabled(master)) {
		dev_err(dev, "cannot attach - SVA enabled\n");
		return -EBUSY;
	}

	arm_smmu_detach_dev(master);

	mutex_lock(&smmu_domain->init_mutex);

	if (!smmu_domain->smmu) {
		smmu_domain->smmu = smmu;

		ret = xring_smmu_init_domain_mapping(dev, domain);
		if (ret) {
			dev_err(dev, "%s domain alloc cookie return %d error\n",
				dev_name(smmu->dev), ret);
			goto out_unlock;
		}

		ret = arm_smmu_domain_finalise(domain, master);
		if (ret) {
			smmu_domain->smmu = NULL;
			goto out_unlock;
		}
	} else if (smmu_domain->smmu != smmu) {
		dev_err(dev,
			"cannot attach to SMMU %s (upstream of %s)\n",
			dev_name(smmu_domain->smmu->dev),
			dev_name(smmu->dev));
		ret = -ENXIO;
		goto out_unlock;
	} else if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1 &&
		   smmu_domain->stall_enabled != master->stall_enabled) {
		dev_err(dev, "cannot attach to stall-%s domain\n",
			smmu_domain->stall_enabled ? "enabled" : "disabled");
		ret = -EINVAL;
		goto out_unlock;
#ifndef CONFIG_XRING_SMMU_DOMAIN_SHARE
	} else if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1 &&
		   master->ssid_bits != smmu_domain->s1_cfg.s1cdmax) {
		dev_err(dev,
			"cannot attach to incompatible domain (%u SSID bits != %u)\n",
			smmu_domain->s1_cfg.s1cdmax, master->ssid_bits);
		ret = -EINVAL;
		goto out_unlock;
#else
	} else {
		arm_smmu_install_cd_for_dev(smmu_domain, master);
		ret = xring_smmu_set_domain_mapping(dev, domain);
		if (ret) {
			dev_err(dev, "%s domain set cookie return %d error\n",
				dev_name(dev), ret);
			goto out_unlock;
		}
		master->domain = smmu_domain;
		master->assigned = true;
		smmu_domain->shared_flag = true;
		goto out_unlock;
#endif
	}

	master->domain = smmu_domain;
	if (smmu_domain->stage != ARM_SMMU_DOMAIN_BYPASS)
		master->ats_enabled = arm_smmu_ats_supported(master);

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	arm_smmu_install_cd_for_dev(smmu_domain, master);
#endif
	arm_smmu_install_ste_for_dev(master);

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	master->assigned = true;
#endif
	spin_lock_irqsave(&smmu_domain->devices_lock, flags);
	list_add(&master->domain_head, &smmu_domain->devices);
	spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);

	arm_smmu_enable_ats(master);

out_unlock:
	mutex_unlock(&smmu_domain->init_mutex);
	return ret;
}

static int arm_smmu_map_pages(struct iommu_domain *domain, unsigned long iova,
			      phys_addr_t paddr, size_t pgsize, size_t pgcount,
			      int prot, gfp_t gfp, size_t *mapped)
{
	struct io_pgtable_ops *ops = to_smmu_domain(domain)->pgtbl_ops;

	if (!ops)
		return -ENODEV;

	return ops->map_pages(ops, iova, paddr, pgsize, pgcount, prot, gfp, mapped);
}

static size_t arm_smmu_unmap_pages(struct iommu_domain *domain, unsigned long iova,
				   size_t pgsize, size_t pgcount,
				   struct iommu_iotlb_gather *gather)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct io_pgtable_ops *ops = smmu_domain->pgtbl_ops;

	if (!ops)
		return 0;

	return ops->unmap_pages(ops, iova, pgsize, pgcount, gather);
}

static void arm_smmu_flush_iotlb_all(struct iommu_domain *domain)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (smmu_domain->smmu)
		arm_smmu_tlb_inv_context_with_lock(smmu_domain);
}

static void arm_smmu_iotlb_sync(struct iommu_domain *domain,
				struct iommu_iotlb_gather *gather)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (!gather->pgsize)
		return;

	arm_smmu_tlb_inv_range_domain(gather->start,
				      gather->end - gather->start + 1,
				      gather->pgsize, true, smmu_domain);
}

static phys_addr_t
arm_smmu_iova_to_phys(struct iommu_domain *domain, dma_addr_t iova)
{
	struct io_pgtable_ops *ops = to_smmu_domain(domain)->pgtbl_ops;

	if (!ops)
		return 0;

	return ops->iova_to_phys(ops, iova);
}

static struct platform_driver arm_smmu_driver;

static
struct arm_smmu_device *arm_smmu_get_by_fwnode(struct fwnode_handle *fwnode)
{
	struct device *dev = driver_find_device_by_fwnode(&arm_smmu_driver.driver,
							  fwnode);
	put_device(dev);
	return dev ? dev_get_drvdata(dev) : NULL;
}

static bool arm_smmu_sid_in_range(struct arm_smmu_device *smmu, u32 sid)
{
	unsigned long limit = smmu->strtab_cfg.num_l1_ents;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB)
		limit *= 1UL << STRTAB_SPLIT;

	return sid < limit;
}

#ifndef CONFIG_XRING_SMMU_DOMAIN_SHARE
static int arm_smmu_insert_master(struct arm_smmu_device *smmu,
				  struct arm_smmu_master *master)
{
	int i;
	int ret = 0;
	struct arm_smmu_stream *new_stream, *cur_stream;
	struct rb_node **new_node, *parent_node = NULL;
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(master->dev);

	master->streams = kcalloc(fwspec->num_ids, sizeof(*master->streams),
				  GFP_KERNEL);
	if (!master->streams)
		return -ENOMEM;
	master->num_streams = fwspec->num_ids;

	mutex_lock(&smmu->streams_mutex);
	for (i = 0; i < fwspec->num_ids; i++) {
		u32 sid = fwspec->ids[i];

		new_stream = &master->streams[i];
		new_stream->id = sid;
		new_stream->master = master;

		/*
		 * Check the SIDs are in range of the SMMU and our stream table
		 */
		if (!arm_smmu_sid_in_range(smmu, sid)) {
			ret = -ERANGE;
			break;
		}

		/* Ensure l2 strtab is initialised */
		if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB) {
			ret = arm_smmu_init_l2_strtab(smmu, sid);
			if (ret)
				break;
		}

		/* Insert into SID tree */
		new_node = &(smmu->streams.rb_node);
		while (*new_node) {
			cur_stream = rb_entry(*new_node, struct arm_smmu_stream,
					      node);
			parent_node = *new_node;
			if (cur_stream->id > new_stream->id) {
				new_node = &((*new_node)->rb_left);
			} else if (cur_stream->id < new_stream->id) {
				new_node = &((*new_node)->rb_right);
			} else {
				dev_warn(master->dev,
					 "stream %u already in tree\n",
					 cur_stream->id);
				ret = -EINVAL;
				break;
			}
		}
		if (ret)
			break;

		rb_link_node(&new_stream->node, parent_node, new_node);
		rb_insert_color(&new_stream->node, &smmu->streams);
	}

	if (ret) {
		for (i--; i >= 0; i--)
			rb_erase(&master->streams[i].node, &smmu->streams);
		kfree(master->streams);
	}
	mutex_unlock(&smmu->streams_mutex);

	return ret;
}

static void arm_smmu_remove_master(struct arm_smmu_master *master)
{
	int i;
	struct arm_smmu_device *smmu = master->smmu;
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(master->dev);

	if (!smmu || !master->streams)
		return;

	mutex_lock(&smmu->streams_mutex);
	for (i = 0; i < fwspec->num_ids; i++)
		rb_erase(&master->streams[i].node, &smmu->streams);
	mutex_unlock(&smmu->streams_mutex);

	kfree(master->streams);
}
#endif

static struct iommu_ops arm_smmu_ops;

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
static void arm_smmu_master_free(struct arm_smmu_master *master)
{
	kfree(master->ssids);
	kfree(master);
}

static int get_ids_from_fwspec(struct iommu_fwspec *fwspec, u32 cell_size,
				struct arm_smmu_master *master)
{
	int i, ssid;
	int num_sids = 0;
	int num_ssids = 0;
	int num_cell = fwspec->num_ids / cell_size;

	if (num_cell <= 0 || (fwspec->num_ids % cell_size)) {
		dev_err(master->dev, "Error ids, please check");
		return -EINVAL;
	}

	master->sid = fwspec->ids[num_sids];
	master->ssids = kcalloc(num_cell, sizeof(u32), GFP_KERNEL);
	if (!master->ssids) {
		dev_err(master->dev, "ssids array memory alloc failed");
		return -ENOMEM;
	}

	for (i = 0; i < fwspec->num_ids && num_ssids <= num_cell; i++) {
		/* fwspec->ids = [sid1, ssid1, sid2, ssid2, ....] */
		if (i % 2) {
			ssid = fwspec->ids[i];
			if (test_and_set_bit(ssid, master->smmu->ssid_bitmap)) {
				dev_err(master->dev, "ssid already exist: %d", ssid);
				return -EINVAL;
			}
			master->ssids[num_ssids] = ssid;
			num_ssids++;
		} else {
			if (num_sids > 0 &&
				master->sid != fwspec->ids[i]) {
				dev_err(master->dev, "configured with different sids, %d %d",
					master->sid, fwspec->ids[i]);
				return -EINVAL;
			}
			num_sids++;
		}
	}
	master->num_ssids = num_ssids;

	xring_smmu_info("sid: 0x%x\n", master->sid);
	xring_smmu_info("num_ssids: %d\n", master->num_ssids);
	for (i = 0; i < master->num_ssids; i++)
		xring_smmu_info("ssid: 0x%x\n", master->ssids[i]);

	return 0;
}
#endif

static struct iommu_device *arm_smmu_probe_device(struct device *dev)
{
	int ret;
	struct arm_smmu_device *smmu;
	struct arm_smmu_master *master;
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);

	if (!fwspec || fwspec->ops != &arm_smmu_ops)
		return ERR_PTR(-ENODEV);

	if (WARN_ON_ONCE(dev_iommu_priv_get(dev)))
		return ERR_PTR(-EBUSY);

	smmu = arm_smmu_get_by_fwnode(fwspec->iommu_fwnode);
	if (!smmu)
		return ERR_PTR(-ENODEV);

	master = kzalloc(sizeof(*master), GFP_KERNEL);
	if (!master)
		return ERR_PTR(-ENOMEM);

	master->dev = dev;
	master->smmu = smmu;
	INIT_LIST_HEAD(&master->bonds);
	dev_iommu_priv_set(dev, master);
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	ret = get_ids_from_fwspec(fwspec, smmu->cell_size, master);
	if (ret)
		goto err_free_master;

	if (!arm_smmu_sid_in_range(smmu, master->sid)) {
		xring_smmu_err("invalid sid range\n");
		ret = -ERANGE;
		goto err_free_master;
	}

	/* Ensure l2 strtab is initialised */
	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB) {
		ret = arm_smmu_init_l2_strtab(smmu, master->sid);
		if (ret) {
			xring_smmu_err("init_l2_strtab failed\n");
			goto err_free_master;
		}
	}
#else
	ret = arm_smmu_insert_master(smmu, master);
	if (ret)
		goto err_free_master;

	device_property_read_u32(dev, "pasid-num-bits", &master->ssid_bits);
	master->ssid_bits = min(smmu->ssid_bits, master->ssid_bits);
#endif

	if (!(smmu->features & ARM_SMMU_FEAT_2_LVL_CDTAB))
		master->ssid_bits = min_t(u8, master->ssid_bits,
					  CTXDESC_LINEAR_CDMAX);

	if ((smmu->features & ARM_SMMU_FEAT_STALLS &&
		device_property_read_bool(dev, "dma-can-stall")) ||
		smmu->features & ARM_SMMU_FEAT_STALL_FORCE)
		master->stall_enabled = true;

	return &smmu->iommu;

err_free_master:
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	arm_smmu_master_free(master);
#else
	kfree(master);
#endif
	dev_iommu_priv_set(dev, NULL);
	return ERR_PTR(ret);
}

static void arm_smmu_release_device(struct device *dev)
{
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);
	struct arm_smmu_master *master;

	if (!fwspec || fwspec->ops != &arm_smmu_ops)
		return;

	master = dev_iommu_priv_get(dev);
	if (WARN_ON(arm_smmu_master_sva_enabled(master)))
		iopf_queue_remove_device(master->smmu->evtq.iopf, dev);
	arm_smmu_detach_dev(master);
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	arm_smmu_master_free(master);
#else
	arm_smmu_remove_master(master);
	kfree(master);
#endif
	iommu_fwspec_free(dev);
}

struct arm_smmu_device *arm_smmu_get_dev_smmu(struct device *dev)
{
	struct arm_smmu_master *master = NULL;

	if (!dev_iommu_fwspec_get(dev)) {
		dev_err(dev, "%s, iommu_fwspec is null\n", __func__);
		return NULL;
	}
	master = dev_iommu_priv_get(dev);
	if (!master) {
		dev_err(dev, "%s, iommu_priv is null\n", __func__);
		return NULL;
	}

	return master->smmu;
}
EXPORT_SYMBOL(arm_smmu_get_dev_smmu);

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
struct iommu_domain *arm_smmu_get_iommu_domain_by_sid(
			struct arm_smmu_device *smmu, int sid)
{
	struct arm_smmu_iommu_group *arm_smmu_group = NULL;

	if (!arm_smmu_sid_in_range(smmu, sid)) {
		dev_err(smmu->dev, "%s: %u out of %s sid range\n", __func__, sid,
			dev_name(smmu->dev));
		return NULL;
	}

	list_for_each_entry(arm_smmu_group, &(smmu->iommu_groups), list)
		if (arm_smmu_group->sid == sid && arm_smmu_group->group)
			return ((struct xring_iommu_group *)
				(arm_smmu_group->group))->default_domain;

	return NULL;
}

static struct iommu_group *arm_smmu_get_iommu_group_by_sid(
					struct device *dev, int sid)
{
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_iommu_group *arm_smmu_group = NULL;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu) {
		dev_err(dev, "%s, dev smmu is null\n", __func__);
		return NULL;
	}
	if (!arm_smmu_sid_in_range(smmu, sid)) {
		dev_err(dev, "%s: %u out of %s sid range\n", __func__, sid,
			dev_name(smmu->dev));
		return NULL;
	}

	list_for_each_entry(arm_smmu_group, &(smmu->iommu_groups), list)
		if (arm_smmu_group->sid == sid)
			return  arm_smmu_group->group;

	return NULL;
}

static void arm_smmu_iommu_group_register(struct device *dev,
				struct iommu_group *group, int sid)
{
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_iommu_group *arm_smmu_group = NULL;

	if (IS_ERR_OR_NULL(group))
		return;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu) {
		dev_err(dev, "%s, dev smmu is null\n", __func__);
		return;
	}
	if (!arm_smmu_sid_in_range(smmu, sid)) {
		dev_err(dev, "%s: %u out of %s sid range\n", __func__, sid,
			dev_name(smmu->dev));
		return;
	}

	arm_smmu_group = devm_kzalloc(smmu->dev, sizeof(*arm_smmu_group), GFP_KERNEL);
	if (!arm_smmu_group)
		return;

	arm_smmu_group->group = group;
	arm_smmu_group->sid = sid;
	spin_lock(&smmu->iommu_groups_lock);
	list_add_tail(&arm_smmu_group->list, &(smmu->iommu_groups));
	spin_unlock(&smmu->iommu_groups_lock);
}

static struct iommu_group *arm_smmu_device_group(struct device *dev)
{
	struct iommu_group *group = NULL;
	struct arm_smmu_master *master = NULL;

	master = dev_iommu_priv_get(dev);
	if (!master) {
		dev_err(dev, "%s, iommu_priv is null\n", __func__);
		return NULL;
	}

	group = arm_smmu_get_iommu_group_by_sid(dev, master->sid);
	if (group) {
		dev_info(dev, "%s, share group, sid %d\n", __func__, master->sid);
		return group;
	}

	dev_info(dev, "%s, create new group, sid %d\n", __func__, master->sid);
	if (dev_is_pci(dev))
		group = pci_device_group(dev);
	else
		group = generic_device_group(dev);

	arm_smmu_iommu_group_register(dev, group, master->sid);

	return group;
}
#else
static int __bus_lookup_iommu_group(struct device *dev, void *priv)
{
	struct lookup_iommu_group_data *data = priv;
	struct device_node *np;
	struct iommu_group *group;

	group = iommu_group_get(dev);
	if (!group)
		return 0;

	np = of_parse_phandle(dev->of_node, "xring,iommu-group", 0);
	if (np != data->np) {
		iommu_group_put(group);
		return 0;
	}

	xring_smmu_info("smmu: found dev: %s has the same group\n", dev_name(dev));

	data->group = group;
	return 1;
}

static struct iommu_group *of_get_device_group(struct device *dev)
{
	struct lookup_iommu_group_data data;
	int ret;

	data.np = of_parse_phandle(dev->of_node, "xring,iommu-group", 0);
	if (!data.np)
		return NULL;

	ret = bus_for_each_dev(&platform_bus_type, NULL, &data,
				__bus_lookup_iommu_group);
	if (ret > 0)
		return data.group;

	ret = bus_for_each_dev(&amba_bustype, NULL, &data,
				__bus_lookup_iommu_group);
	if (ret > 0)
		return data.group;

	return NULL;
}

static struct iommu_group *arm_smmu_device_group(struct device *dev)
{
	struct iommu_group *group;

	group = of_get_device_group(dev);
	if (group) {
		xring_smmu_info("smmu: share group %s\n", dev_name(dev));
		return group;
	}

	xring_smmu_info("smmu: create new group %s\n", dev_name(dev));
	if (dev_is_pci(dev))
		group = pci_device_group(dev);
	else
		group = generic_device_group(dev);

	return group;
}
#endif

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
static int arm_smmu_enable_nesting(struct iommu_domain *domain)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	int ret = 0;

	mutex_lock(&smmu_domain->init_mutex);
	if (smmu_domain->smmu)
		ret = -EPERM;
	else
		smmu_domain->stage = ARM_SMMU_DOMAIN_NESTED;
	mutex_unlock(&smmu_domain->init_mutex);

	return ret;
}
#endif

static int arm_smmu_of_xlate(struct device *dev, struct of_phandle_args *args)
{
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	dev_err(dev, "%s, args_count %d\n", __func__, args->args_count);
	return iommu_fwspec_add_ids(dev, args->args, args->args_count);
#else
	return iommu_fwspec_add_ids(dev, args->args, 1);
#endif
}

static void arm_smmu_get_resv_regions(struct device *dev,
				      struct list_head *head)
{
	struct iommu_resv_region *region;
	int prot = IOMMU_WRITE | IOMMU_NOEXEC | IOMMU_MMIO;

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	region = iommu_alloc_resv_region(MSI_IOVA_BASE, MSI_IOVA_LENGTH,
					 prot, IOMMU_RESV_SW_MSI);
#else
	region = iommu_alloc_resv_region(MSI_IOVA_BASE, MSI_IOVA_LENGTH,
					 prot, IOMMU_RESV_SW_MSI, GFP_KERNEL);
#endif
	if (!region)
		return;

	list_add_tail(&region->list, head);

	iommu_dma_get_resv_regions(dev, head);
}

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
static bool arm_smmu_dev_has_feature(struct device *dev,
				     enum iommu_dev_features feat)
{
	struct arm_smmu_master *master = dev_iommu_priv_get(dev);

	if (!master)
		return false;

	switch (feat) {
	case IOMMU_DEV_FEAT_IOPF:
		return arm_smmu_master_iopf_supported(master);
	case IOMMU_DEV_FEAT_SVA:
		return arm_smmu_master_sva_supported(master);
	default:
		return false;
	}
}

static bool arm_smmu_dev_feature_enabled(struct device *dev,
					 enum iommu_dev_features feat)
{
	struct arm_smmu_master *master = dev_iommu_priv_get(dev);

	if (!master)
		return false;

	switch (feat) {
	case IOMMU_DEV_FEAT_IOPF:
		return master->iopf_enabled;
	case IOMMU_DEV_FEAT_SVA:
		return arm_smmu_master_sva_enabled(master);
	default:
		return false;
	}
}
#endif

static int arm_smmu_dev_enable_feature(struct device *dev,
					enum iommu_dev_features feat)
{
	struct arm_smmu_master *master = dev_iommu_priv_get(dev);

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	if (!arm_smmu_dev_has_feature(dev, feat))
		return -ENODEV;

	if (arm_smmu_dev_feature_enabled(dev, feat))
		return -EBUSY;
#endif

	switch (feat) {
	case IOMMU_DEV_FEAT_IOPF:
		master->iopf_enabled = true;
		return 0;
	case IOMMU_DEV_FEAT_SVA:
		return arm_smmu_master_enable_sva(master);
	default:
		return -EINVAL;
	}
}

static int arm_smmu_dev_disable_feature(struct device *dev,
					enum iommu_dev_features feat)
{
	struct arm_smmu_master *master = dev_iommu_priv_get(dev);

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	if (!arm_smmu_dev_feature_enabled(dev, feat))
		return -EINVAL;
#endif

	switch (feat) {
	case IOMMU_DEV_FEAT_IOPF:
		if (master->sva_enabled)
			return -EBUSY;
		master->iopf_enabled = false;
		return 0;
	case IOMMU_DEV_FEAT_SVA:
		return arm_smmu_master_disable_sva(master);
	default:
		return -EINVAL;
	}
}

#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
struct iommu_domain_ops arm_domain_ops = {
	.attach_dev		= arm_smmu_attach_dev,
	.map_pages		= arm_smmu_map_pages,
	.unmap_pages		= arm_smmu_unmap_pages,
	.flush_iotlb_all	= arm_smmu_flush_iotlb_all,
	.iotlb_sync		= arm_smmu_iotlb_sync,
	.iova_to_phys		= arm_smmu_iova_to_phys,
	.free			= arm_smmu_domain_free,
};
#endif

static struct iommu_ops arm_smmu_ops = {
	.capable		= arm_smmu_capable,
	.domain_alloc		= arm_smmu_domain_alloc,
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	.domain_free		= arm_smmu_domain_free,
	.attach_dev		= arm_smmu_attach_dev,
	.map_pages		= arm_smmu_map_pages,
	.unmap_pages		= arm_smmu_unmap_pages,
	.flush_iotlb_all	= arm_smmu_flush_iotlb_all,
	.iotlb_sync		= arm_smmu_iotlb_sync,
	.iova_to_phys		= arm_smmu_iova_to_phys,
#else
	.default_domain_ops	= &arm_domain_ops,
#endif
	.probe_device		= arm_smmu_probe_device,
	.release_device		= arm_smmu_release_device,
	.device_group		= arm_smmu_device_group,
	.of_xlate		= arm_smmu_of_xlate,
	.get_resv_regions	= arm_smmu_get_resv_regions,
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	.put_resv_regions	= generic_iommu_put_resv_regions,
	.dev_has_feat		= arm_smmu_dev_has_feature,
	.dev_feat_enabled	= arm_smmu_dev_feature_enabled,
	.sva_bind		= arm_smmu_sva_bind,
	.sva_unbind		= arm_smmu_sva_unbind,
	.enable_nesting		= arm_smmu_enable_nesting,
	.sva_get_pasid		= arm_smmu_sva_get_pasid,
	.page_response		= arm_smmu_page_response,
#endif
	.dev_enable_feat	= arm_smmu_dev_enable_feature,
	.dev_disable_feat	= arm_smmu_dev_disable_feature,
	.pgsize_bitmap		= -1UL, /* Restricted during device attach */
	.owner			= THIS_MODULE,
};

/* Probing and initialisation functions */
static int arm_smmu_init_one_queue(struct arm_smmu_device *smmu,
				   struct arm_smmu_queue *q,
				   void __iomem *page,
				   unsigned long prod_off,
				   unsigned long cons_off,
				   size_t dwords, const char *name)
{
	size_t qsz;

	do {
		qsz = ((1 << q->llq.max_n_shift) * dwords) << 3;
		q->base = dmam_alloc_coherent(smmu->dev, qsz, &q->base_dma,
					      GFP_KERNEL);
		if (q->base || qsz < PAGE_SIZE)
			break;

		q->llq.max_n_shift--;
	} while (1);

	if (!q->base) {
		dev_err(smmu->dev,
			"failed to allocate queue (0x%zx bytes) for %s\n",
			qsz, name);
		return -ENOMEM;
	}

	if (!WARN_ON(q->base_dma & (qsz - 1))) {
		dev_info(smmu->dev, "allocated %u entries for %s\n",
			 1 << q->llq.max_n_shift, name);
	}

	q->prod_reg	= page + prod_off;
	q->cons_reg	= page + cons_off;
	q->ent_dwords	= dwords;

	q->q_base  = Q_BASE_RWA;
	q->q_base |= q->base_dma & Q_BASE_ADDR_MASK;
	q->q_base |= FIELD_PREP(Q_BASE_LOG2SIZE, q->llq.max_n_shift);

	q->llq.prod = q->llq.cons = 0;
	dev_info(smmu->dev, "queue name:%s, q virt base:%p, q phy base:0x%llx",
		name, q->base, q->base_dma);
	return 0;
}

static void arm_smmu_cmdq_free_bitmap(void *data)
{
	unsigned long *bitmap = data;

	bitmap_free(bitmap);
}

static int arm_smmu_cmdq_init(struct arm_smmu_device *smmu)
{
	int ret = 0;
	struct arm_smmu_cmdq *cmdq = &smmu->cmdq;
	unsigned int nents = 1 << cmdq->q.llq.max_n_shift;
	atomic_long_t *bitmap;

	atomic_set(&cmdq->owner_prod, 0);
	atomic_set(&cmdq->lock, 0);

	bitmap = (atomic_long_t *)bitmap_zalloc(nents, GFP_KERNEL);
	if (!bitmap) {
		dev_err(smmu->dev, "failed to allocate cmdq bitmap\n");
		ret = -ENOMEM;
	} else {
		cmdq->valid_map = bitmap;
		devm_add_action(smmu->dev, arm_smmu_cmdq_free_bitmap, bitmap);
	}

	return ret;
}

static int arm_smmu_init_queues(struct arm_smmu_device *smmu)
{
	int ret;

	/* cmdq */
	ret = arm_smmu_init_one_queue(smmu, &smmu->cmdq.q, smmu->base,
				      ARM_SMMU_CMDQ_PROD, ARM_SMMU_CMDQ_CONS,
				      CMDQ_ENT_DWORDS, "cmdq");
	if (ret)
		return ret;

	ret = arm_smmu_cmdq_init(smmu);
	if (ret)
		return ret;

	/* evtq */
	ret = arm_smmu_init_one_queue(smmu, &smmu->evtq.q, smmu->page1,
				      ARM_SMMU_EVTQ_PROD, ARM_SMMU_EVTQ_CONS,
				      EVTQ_ENT_DWORDS, "evtq");
	if (ret)
		return ret;

	if ((smmu->features & ARM_SMMU_FEAT_SVA) &&
	    (smmu->features & ARM_SMMU_FEAT_STALLS)) {
		smmu->evtq.iopf = iopf_queue_alloc(dev_name(smmu->dev));
		if (!smmu->evtq.iopf)
			return -ENOMEM;
	}

	/* priq */
	if (!(smmu->features & ARM_SMMU_FEAT_PRI))
		return 0;

	return arm_smmu_init_one_queue(smmu, &smmu->priq.q, smmu->page1,
					ARM_SMMU_PRIQ_PROD, ARM_SMMU_PRIQ_CONS,
					PRIQ_ENT_DWORDS, "priq");
}

static int arm_smmu_init_l1_strtab(struct arm_smmu_device *smmu)
{
	unsigned int i;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;
	size_t size = sizeof(*cfg->l1_desc) * cfg->num_l1_ents;
	void *strtab = smmu->strtab_cfg.strtab;

	cfg->l1_desc = devm_kzalloc(smmu->dev, size, GFP_KERNEL);
	if (!cfg->l1_desc)
		return -ENOMEM;

	for (i = 0; i < cfg->num_l1_ents; ++i) {
		arm_smmu_write_strtab_l1_desc(strtab, &cfg->l1_desc[i]);
		strtab += STRTAB_L1_DESC_DWORDS << 3;
	}

	return 0;
}

static int arm_smmu_init_strtab_2lvl(struct arm_smmu_device *smmu)
{
	void *strtab;
	u64 reg;
	u32 size, l1size;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	/* Calculate the L1 size, capped to the SIDSIZE. */
	size = STRTAB_L1_SZ_SHIFT - (ilog2(STRTAB_L1_DESC_DWORDS) + 3);
	size = min(size, smmu->sid_bits - STRTAB_SPLIT);
	cfg->num_l1_ents = 1 << size;

	size += STRTAB_SPLIT;
	if (size < smmu->sid_bits)
		dev_warn(smmu->dev,
			 "2-level strtab only covers %u/%u bits of SID\n",
			 size, smmu->sid_bits);

	l1size = cfg->num_l1_ents * (STRTAB_L1_DESC_DWORDS << 3);
	strtab = dmam_alloc_coherent(smmu->dev, l1size, &cfg->strtab_dma,
				     GFP_KERNEL);
	if (!strtab) {
		dev_err(smmu->dev,
			"failed to allocate l1 stream table (%u bytes)\n",
			l1size);
		return -ENOMEM;
	}
	cfg->strtab = strtab;

	/* Configure strtab_base_cfg for 2 levels */
	reg  = FIELD_PREP(STRTAB_BASE_CFG_FMT, STRTAB_BASE_CFG_FMT_2LVL);
	reg |= FIELD_PREP(STRTAB_BASE_CFG_LOG2SIZE, size);
	reg |= FIELD_PREP(STRTAB_BASE_CFG_SPLIT, STRTAB_SPLIT);
	cfg->strtab_base_cfg = reg;

	return arm_smmu_init_l1_strtab(smmu);
}

static int arm_smmu_init_strtab_linear(struct arm_smmu_device *smmu)
{
	void *strtab;
	u64 reg;
	u32 size;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	size = (1 << smmu->sid_bits) * (STRTAB_STE_DWORDS << 3);
	strtab = dmam_alloc_coherent(smmu->dev, size, &cfg->strtab_dma,
				     GFP_KERNEL);
	if (!strtab) {
		dev_err(smmu->dev,
			"failed to allocate linear stream table (%u bytes)\n",
			size);
		return -ENOMEM;
	}
	cfg->strtab = strtab;
	cfg->num_l1_ents = 1 << smmu->sid_bits;

	/* Configure strtab_base_cfg for a linear table covering all SIDs */
	reg  = FIELD_PREP(STRTAB_BASE_CFG_FMT, STRTAB_BASE_CFG_FMT_LINEAR);
	reg |= FIELD_PREP(STRTAB_BASE_CFG_LOG2SIZE, smmu->sid_bits);
	cfg->strtab_base_cfg = reg;

	arm_smmu_init_bypass_stes(strtab, cfg->num_l1_ents);
	return 0;
}

static int arm_smmu_init_strtab(struct arm_smmu_device *smmu)
{
	u64 reg;
	int ret;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB)
		ret = arm_smmu_init_strtab_2lvl(smmu);
	else
		ret = arm_smmu_init_strtab_linear(smmu);

	if (ret)
		return ret;

	/* Set the strtab base address */
	reg  = smmu->strtab_cfg.strtab_dma & STRTAB_BASE_ADDR_MASK;
	reg |= STRTAB_BASE_RA;
	smmu->strtab_cfg.strtab_base = reg;

	/* Allocate the first VMID for stage-2 bypass STEs */
	set_bit(0, smmu->vmid_map);
	return 0;
}

static int arm_smmu_init_structures(struct arm_smmu_device *smmu)
{
	int ret;

	mutex_init(&smmu->streams_mutex);
	smmu->streams = RB_ROOT;

	ret = arm_smmu_init_queues(smmu);
	if (ret)
		return ret;
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	ret = arm_smmu_init_strtab(smmu);
	if (ret) {
		dev_err(smmu->dev, "Failed to init STE\n");
		return ret;
	}

	ret = arm_smmu_alloc_cd_table(smmu);
	if (ret) {
		dev_err(smmu->dev, "Failed to alloc cd table\n");
		return ret;
	}

	return ret;
#else
	return arm_smmu_init_strtab(smmu);
#endif
}

static int arm_smmu_write_reg_sync(struct arm_smmu_device *smmu, u32 val,
				   unsigned int reg_off, unsigned int ack_off)
{
	u32 reg;

	writel_relaxed(val, smmu->base + reg_off);
	return readl_relaxed_poll_timeout_atomic(smmu->base + ack_off,
				reg, reg == val, 1, ARM_SMMU_POLL_TIMEOUT_US);
}

/* GBPA is "special" */
static int arm_smmu_update_gbpa(struct arm_smmu_device *smmu, u32 set, u32 clr)
{
	int ret;
	u32 reg, __iomem *gbpa = smmu->base + ARM_SMMU_GBPA;

	ret = readl_relaxed_poll_timeout_atomic(gbpa, reg, !(reg & GBPA_UPDATE),
					 1, ARM_SMMU_POLL_TIMEOUT_US);
	if (ret)
		return ret;

	reg &= ~clr;
	reg |= set;
	writel_relaxed(reg | GBPA_UPDATE, gbpa);
	ret = readl_relaxed_poll_timeout_atomic(gbpa, reg, !(reg & GBPA_UPDATE),
					 1, ARM_SMMU_POLL_TIMEOUT_US);

	if (ret)
		dev_err(smmu->dev, "GBPA not responding to update\n");
	return ret;
}

static void arm_smmu_setup_unique_irqs(struct arm_smmu_device *smmu)
{
	int irq, ret;

	/* Request interrupt lines */
	irq = smmu->evtq.q.irq;
	if (irq) {
		ret = devm_request_threaded_irq(smmu->dev, irq, NULL,
						arm_smmu_evtq_thread,
						IRQF_ONESHOT,
						"arm-smmu-v3-evtq", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable evtq irq\n");
	} else {
		dev_warn(smmu->dev, "no evtq irq - events will not be reported!\n");
	}

	/* cmdq-sync irq */
	irq = smmu->cmd_sync_irq;
	if (irq) {
		ret = devm_request_irq(smmu->dev, irq,
				arm_smmu_cmdq_sync_handler, IRQF_ONESHOT,
				"arm-smmu-v3-cmdq_sync", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable cmdq_sync irq\n");
	} else {
		dev_warn(smmu->dev, "no cmdq_sync irq - errors will not be reported!\n");
	}

	irq = smmu->gerr_irq;
	if (irq) {
		ret = devm_request_irq(smmu->dev, irq, arm_smmu_gerror_handler,
					0, "arm-smmu-v3-gerror", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable gerror irq\n");
	} else {
		dev_warn(smmu->dev, "no gerr irq - errors will not be reported!\n");
	}

	if (smmu->features & ARM_SMMU_FEAT_PRI) {
		irq = smmu->priq.q.irq;
		if (irq) {
			ret = devm_request_threaded_irq(smmu->dev, irq, NULL,
							arm_smmu_priq_thread,
							IRQF_ONESHOT,
							"arm-smmu-v3-priq",
							smmu);
			if (ret < 0)
				dev_warn(smmu->dev,
					 "failed to enable priq irq\n");
		} else {
			dev_warn(smmu->dev, "no priq irq - PRI will be broken\n");
		}
	}

	xring_smmu_wrapper_setup_irqs(smmu);
}

static int arm_smmu_setup_irqs_handler(struct arm_smmu_device *smmu)
{
	int ret = 0, irq = 0;

	irq = smmu->combined_irq;
	if (irq) {
		ret = devm_request_threaded_irq(smmu->dev, irq,
					arm_smmu_combined_irq_handler,
					arm_smmu_combined_irq_thread,
					IRQF_ONESHOT,
					"arm-smmu-v3-combined-irq", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable combined irq\n");
	} else {
		arm_smmu_setup_unique_irqs(smmu);
	}
	return ret;
}

static int arm_smmu_setup_irqs(struct arm_smmu_device *smmu)
{
	int ret = 0;
	u32 irqen_flags = IRQ_CTRL_EVTQ_IRQEN | IRQ_CTRL_GERROR_IRQEN;

	/* Disable IRQs first */
	ret = arm_smmu_write_reg_sync(smmu, 0, ARM_SMMU_IRQ_CTRL,
				      ARM_SMMU_IRQ_CTRLACK);
	if (ret) {
		dev_err(smmu->dev, "failed to disable irqs\n");
		return ret;
	}

	if (smmu->features & ARM_SMMU_FEAT_PRI)
		irqen_flags |= IRQ_CTRL_PRIQ_IRQEN;

	/* Enable interrupt generation on the SMMU */
	ret = arm_smmu_write_reg_sync(smmu, irqen_flags,
				      ARM_SMMU_IRQ_CTRL, ARM_SMMU_IRQ_CTRLACK);
	if (ret)
		dev_warn(smmu->dev, "failed to enable irqs\n");

	return 0;
}

static int arm_smmu_device_disable(struct arm_smmu_device *smmu)
{
	int ret;

	ret = arm_smmu_write_reg_sync(smmu, 0, ARM_SMMU_CR0, ARM_SMMU_CR0ACK);
	if (ret)
		dev_err(smmu->dev, "failed to clear cr0\n");

	return ret;
}

static int arm_smmu_device_reset(struct arm_smmu_device *smmu, bool bypass)
{
	int ret;
	u32 reg, enables;
	struct arm_smmu_cmdq_ent cmd = {0};

	/* Clear CR0 and sync (disables SMMU and queue processing) */
	reg = readl_relaxed(smmu->base + ARM_SMMU_CR0);
	if (reg & CR0_SMMUEN) {
		dev_warn(smmu->dev, "SMMU currently enabled! Resetting...\n");
		WARN_ON(is_kdump_kernel() && !disable_bypass);
		arm_smmu_update_gbpa(smmu, GBPA_ABORT, 0);
	}

	ret = arm_smmu_device_disable(smmu);
	if (ret)
		return ret;

	/* CR1 (table and queue memory attributes) */
	reg = FIELD_PREP(CR1_TABLE_SH, ARM_SMMU_SH_ISH) |
	      FIELD_PREP(CR1_TABLE_OC, CR1_CACHE_WB) |
	      FIELD_PREP(CR1_TABLE_IC, CR1_CACHE_WB) |
	      FIELD_PREP(CR1_QUEUE_SH, ARM_SMMU_SH_ISH) |
	      FIELD_PREP(CR1_QUEUE_OC, CR1_CACHE_WB) |
	      FIELD_PREP(CR1_QUEUE_IC, CR1_CACHE_WB);
	writel_relaxed(reg, smmu->base + ARM_SMMU_CR1);

	/* CR2 (random crap) */
	reg = CR2_PTM | CR2_RECINVSID;

	if (smmu->features & ARM_SMMU_FEAT_E2H)
		reg |= CR2_E2H;

	writel_relaxed(reg, smmu->base + ARM_SMMU_CR2);

	/* Stream table */
	writeq_relaxed(smmu->strtab_cfg.strtab_base,
			smmu->base + ARM_SMMU_STRTAB_BASE);
	writel_relaxed(smmu->strtab_cfg.strtab_base_cfg,
			smmu->base + ARM_SMMU_STRTAB_BASE_CFG);

	/* Command queue */
	writeq_relaxed(smmu->cmdq.q.q_base, smmu->base + ARM_SMMU_CMDQ_BASE);
	writel_relaxed(smmu->cmdq.q.llq.prod, smmu->base + ARM_SMMU_CMDQ_PROD);
	writel_relaxed(smmu->cmdq.q.llq.cons, smmu->base + ARM_SMMU_CMDQ_CONS);

	enables = CR0_CMDQEN;
	ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
				      ARM_SMMU_CR0ACK);
	if (ret) {
		dev_err(smmu->dev, "failed to enable command queue\n");
		return ret;
	}

	/* Invalidate any cached configuration */
	cmd.opcode = CMDQ_OP_CFGI_ALL;
	arm_smmu_cmdq_issue_cmd_with_sync(smmu, &cmd);

	/* Invalidate any stale TLB entries */
	if (smmu->features & ARM_SMMU_FEAT_HYP) {
		cmd.opcode = CMDQ_OP_TLBI_EL2_ALL;
		arm_smmu_cmdq_issue_cmd_with_sync(smmu, &cmd);
	}

	cmd.opcode = CMDQ_OP_TLBI_NSNH_ALL;
	arm_smmu_cmdq_issue_cmd_with_sync(smmu, &cmd);

	/* Event queue */
	writeq_relaxed(smmu->evtq.q.q_base, smmu->base + ARM_SMMU_EVTQ_BASE);
	writel_relaxed(smmu->evtq.q.llq.prod, smmu->page1 + ARM_SMMU_EVTQ_PROD);
	writel_relaxed(smmu->evtq.q.llq.cons, smmu->page1 + ARM_SMMU_EVTQ_CONS);

	enables |= CR0_EVTQEN;
	ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
				      ARM_SMMU_CR0ACK);
	if (ret) {
		dev_err(smmu->dev, "failed to enable event queue\n");
		return ret;
	}

	/* PRI queue */
	if (smmu->features & ARM_SMMU_FEAT_PRI) {
		writeq_relaxed(smmu->priq.q.q_base,
				smmu->base + ARM_SMMU_PRIQ_BASE);
		writel_relaxed(smmu->priq.q.llq.prod,
				smmu->page1 + ARM_SMMU_PRIQ_PROD);
		writel_relaxed(smmu->priq.q.llq.cons,
				smmu->page1 + ARM_SMMU_PRIQ_CONS);

		enables |= CR0_PRIQEN;
		ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
					      ARM_SMMU_CR0ACK);
		if (ret) {
			dev_err(smmu->dev, "failed to enable PRI queue\n");
			return ret;
		}
	}

	if (smmu->features & ARM_SMMU_FEAT_ATS) {
		enables |= CR0_ATSCHK;
		ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
					      ARM_SMMU_CR0ACK);
		if (ret) {
			dev_err(smmu->dev, "failed to enable ATS check\n");
			return ret;
		}
	}

	ret = arm_smmu_setup_irqs(smmu);
	if (ret) {
		dev_err(smmu->dev, "failed to setup irqs\n");
		return ret;
	}

	if (is_kdump_kernel())
		enables &= ~(CR0_EVTQEN | CR0_PRIQEN);

	/* Enable the SMMU interface, or ensure bypass */
	if (!bypass && disable_bypass) {
		enables |= CR0_SMMUEN;
	} else {
		ret = arm_smmu_update_gbpa(smmu, 0, GBPA_ABORT);
		if (ret)
			return ret;
	}

	ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
				      ARM_SMMU_CR0ACK);
	if (ret) {
		dev_err(smmu->dev, "failed to enable SMMU interface\n");
		return ret;
	}

	dev_info(smmu->dev, "SMMU_CR0:%x, SMMU_CR0_ACK:%x, SMMU_GPPA:0x%x\n",
		readl(smmu->base + ARM_SMMU_CR0), readl(smmu->base + ARM_SMMU_CR0ACK),
		readl(smmu->base + ARM_SMMU_GBPA));

	return 0;
}

static int arm_smmu_device_hw_probe(struct arm_smmu_device *smmu)
{
	u32 reg;
	bool coherent = smmu->features & ARM_SMMU_FEAT_COHERENCY;
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	struct device *dev = smmu->dev;
	struct device_node *node;
	int ret;

	node = dev->of_node;
	if (!node) {
		xring_smmu_err("arm smmu-v3 node not found\n");
		return -EINVAL;
	}

	/* IDR0 */
	ret = of_property_read_u32(node, "ARM_SMMU_IDR0", &reg);
	if (ret < 0) {
		xring_smmu_err("ARM_SMMU_IDR0 fail\n");
		return -EINVAL;
	}
#else
	reg = readl_relaxed(smmu->base + ARM_SMMU_IDR0);
#endif

	/* 2-level structures */
	if (FIELD_GET(IDR0_ST_LVL, reg) == IDR0_ST_LVL_2LVL)
		smmu->features |= ARM_SMMU_FEAT_2_LVL_STRTAB;

	if (reg & IDR0_CD2L)
		smmu->features |= ARM_SMMU_FEAT_2_LVL_CDTAB;

	/*
	 * Translation table endianness.
	 * We currently require the same endianness as the CPU, but this
	 * could be changed later by adding a new IO_PGTABLE_QUIRK.
	 */
	switch (FIELD_GET(IDR0_TTENDIAN, reg)) {
	case IDR0_TTENDIAN_MIXED:
		smmu->features |= ARM_SMMU_FEAT_TT_LE | ARM_SMMU_FEAT_TT_BE;
		break;
#ifdef __BIG_ENDIAN
	case IDR0_TTENDIAN_BE:
		smmu->features |= ARM_SMMU_FEAT_TT_BE;
		break;
#else
	case IDR0_TTENDIAN_LE:
		smmu->features |= ARM_SMMU_FEAT_TT_LE;
		break;
#endif
	default:
		dev_err(smmu->dev, "unknown/unsupported TT endianness!\n");
		return -ENXIO;
	}

	/* Boolean feature flags */
	if (IS_ENABLED(CONFIG_PCI_PRI) && reg & IDR0_PRI)
		smmu->features |= ARM_SMMU_FEAT_PRI;

	if (IS_ENABLED(CONFIG_PCI_ATS) && reg & IDR0_ATS)
		smmu->features |= ARM_SMMU_FEAT_ATS;

	if (reg & IDR0_SEV)
		smmu->features |= ARM_SMMU_FEAT_SEV;

	if (reg & IDR0_HYP) {
		smmu->features |= ARM_SMMU_FEAT_HYP;
		if (cpus_have_cap(ARM64_HAS_VIRT_HOST_EXTN))
			smmu->features |= ARM_SMMU_FEAT_E2H;
	}

	/*
	 * The coherency feature as set by FW is used in preference to the ID
	 * register, but warn on mismatch.
	 */
	if (!!(reg & IDR0_COHACC) != coherent)
		dev_warn(smmu->dev, "IDR0.COHACC overridden by FW configuration (%s)\n",
			 coherent ? "true" : "false");

	switch (FIELD_GET(IDR0_STALL_MODEL, reg)) {
	case IDR0_STALL_MODEL_FORCE:
		smmu->features |= ARM_SMMU_FEAT_STALL_FORCE;
		fallthrough;
	case IDR0_STALL_MODEL_STALL:
		smmu->features |= ARM_SMMU_FEAT_STALLS;
	}

	if (reg & IDR0_S1P)
		smmu->features |= ARM_SMMU_FEAT_TRANS_S1;

	if (reg & IDR0_S2P)
		smmu->features |= ARM_SMMU_FEAT_TRANS_S2;

	if (!(reg & (IDR0_S1P | IDR0_S2P))) {
		dev_err(smmu->dev, "no translation support!\n");
		return -ENXIO;
	}

	/* We only support the AArch64 table format at present */
	switch (FIELD_GET(IDR0_TTF, reg)) {
	case IDR0_TTF_AARCH32_64:
		smmu->ias = 40;
		fallthrough;
	case IDR0_TTF_AARCH64:
		break;
	default:
		dev_err(smmu->dev, "AArch64 table format not supported!\n");
		return -ENXIO;
	}

	/* ASID/VMID sizes */
	smmu->asid_bits = reg & IDR0_ASID16 ? 16 : 8;
	smmu->vmid_bits = reg & IDR0_VMID16 ? 16 : 8;

	/* IDR1 */
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	ret = of_property_read_u32(node, "ARM_SMMU_IDR1", &reg);
	if (ret < 0) {
		xring_smmu_err("ARM_SMMU_IDR1 fail\n");
		return -EINVAL;
	}
#else
	reg = readl_relaxed(smmu->base + ARM_SMMU_IDR1);
#endif
	if (reg & (IDR1_TABLES_PRESET | IDR1_QUEUES_PRESET | IDR1_REL)) {
		dev_err(smmu->dev, "embedded implementation not supported\n");
		return -ENXIO;
	}

	/* Queue sizes, capped to ensure natural alignment */
	smmu->cmdq.q.llq.max_n_shift = min_t(u32, CMDQ_MAX_SZ_SHIFT,
					     FIELD_GET(IDR1_CMDQS, reg));
	if (smmu->cmdq.q.llq.max_n_shift <= ilog2(CMDQ_BATCH_ENTRIES)) {
		/*
		 * We don't support splitting up batches, so one batch of
		 * commands plus an extra sync needs to fit inside the command
		 * queue. There's also no way we can handle the weird alignment
		 * restrictions on the base pointer for a unit-length queue.
		 */
		dev_err(smmu->dev, "command queue size <= %d entries not supported\n",
			CMDQ_BATCH_ENTRIES);
		return -ENXIO;
	}

	smmu->evtq.q.llq.max_n_shift = min_t(u32, EVTQ_MAX_SZ_SHIFT,
					     FIELD_GET(IDR1_EVTQS, reg));
	smmu->priq.q.llq.max_n_shift = min_t(u32, PRIQ_MAX_SZ_SHIFT,
					     FIELD_GET(IDR1_PRIQS, reg));

	/* SID/SSID sizes */
	smmu->ssid_bits = FIELD_GET(IDR1_SSIDSIZE, reg);
	smmu->sid_bits = FIELD_GET(IDR1_SIDSIZE, reg);

	/*
	 * If the SMMU supports fewer bits than would fill a single L2 stream
	 * table, use a linear table instead.
	 */
	if (smmu->sid_bits <= STRTAB_SPLIT)
		smmu->features &= ~ARM_SMMU_FEAT_2_LVL_STRTAB;

	/* IDR3 */
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	ret = of_property_read_u32(node, "ARM_SMMU_IDR3", &reg);
	if (ret < 0) {
		xring_smmu_err("ARM_SMMU_IDR3 fail\n");
		return -EINVAL;
	}
#else
	reg = readl_relaxed(smmu->base + ARM_SMMU_IDR3);
#endif
	if (FIELD_GET(IDR3_RIL, reg))
		smmu->features |= ARM_SMMU_FEAT_RANGE_INV;

	/* IDR5 */
#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	ret = of_property_read_u32(node, "ARM_SMMU_IDR5", &reg);
	if (ret < 0) {
		xring_smmu_err("ARM_SMMU_IDR5 fail\n");
		return -EINVAL;
	}
#else
	reg = readl_relaxed(smmu->base + ARM_SMMU_IDR5);
#endif
	/* Maximum number of outstanding stalls */
	smmu->evtq.max_stalls = FIELD_GET(IDR5_STALL_MAX, reg);

	/* Page sizes */
	if (reg & IDR5_GRAN64K)
		smmu->pgsize_bitmap |= SZ_64K | SZ_512M;
	if (reg & IDR5_GRAN16K)
		smmu->pgsize_bitmap |= SZ_16K | SZ_32M;
	if (reg & IDR5_GRAN4K)
		smmu->pgsize_bitmap |= SZ_4K | SZ_2M | SZ_1G;

	/* Input address size */
	if (FIELD_GET(IDR5_VAX, reg) == IDR5_VAX_52_BIT)
		smmu->features |= ARM_SMMU_FEAT_VAX;

	/* Output address size */
	switch (FIELD_GET(IDR5_OAS, reg)) {
	case IDR5_OAS_32_BIT:
		smmu->oas = 32;
		break;
	case IDR5_OAS_36_BIT:
		smmu->oas = 36;
		break;
	case IDR5_OAS_40_BIT:
		smmu->oas = 40;
		break;
	case IDR5_OAS_42_BIT:
		smmu->oas = 42;
		break;
	case IDR5_OAS_44_BIT:
		smmu->oas = 44;
		break;
	case IDR5_OAS_52_BIT:
		smmu->oas = 52;
		smmu->pgsize_bitmap |= 1ULL << 42; /* 4TB */
		break;
	default:
		dev_info(smmu->dev,
			"unknown output address size. Truncating to 48-bit\n");
		fallthrough;
	case IDR5_OAS_48_BIT:
		smmu->oas = 48;
	}

	if (arm_smmu_ops.pgsize_bitmap == -1UL)
		arm_smmu_ops.pgsize_bitmap = smmu->pgsize_bitmap;
	else
		arm_smmu_ops.pgsize_bitmap |= smmu->pgsize_bitmap;

	/* Set the DMA mask for our table walker */
	if (dma_set_mask_and_coherent(smmu->dev, DMA_BIT_MASK(smmu->oas)))
		dev_warn(smmu->dev,
			 "failed to set DMA mask for table walker\n");

	smmu->ias = max(smmu->ias, smmu->oas);

	if (arm_smmu_sva_supported(smmu))
		smmu->features |= ARM_SMMU_FEAT_SVA;

	dev_info(smmu->dev, "ias %lu-bit, oas %lu-bit (features 0x%08x)\n",
		 smmu->ias, smmu->oas, smmu->features);
	return 0;
}

#ifdef CONFIG_ACPI
static void acpi_smmu_get_options(u32 model, struct arm_smmu_device *smmu)
{
	switch (model) {
	case ACPI_IORT_SMMU_V3_CAVIUM_CN99XX:
		smmu->options |= ARM_SMMU_OPT_PAGE0_REGS_ONLY;
		break;
	case ACPI_IORT_SMMU_V3_HISILICON_HI161X:
		smmu->options |= ARM_SMMU_OPT_SKIP_PREFETCH;
		break;
	}

	dev_notice(smmu->dev, "option mask 0x%x\n", smmu->options);
}

static int arm_smmu_device_acpi_probe(struct platform_device *pdev,
				      struct arm_smmu_device *smmu)
{
	struct acpi_iort_smmu_v3 *iort_smmu;
	struct device *dev = smmu->dev;
	struct acpi_iort_node *node;

	node = *(struct acpi_iort_node **)dev_get_platdata(dev);

	/* Retrieve SMMUv3 specific data */
	iort_smmu = (struct acpi_iort_smmu_v3 *)node->node_data;

	acpi_smmu_get_options(iort_smmu->model, smmu);

	if (iort_smmu->flags & ACPI_IORT_SMMU_V3_COHACC_OVERRIDE)
		smmu->features |= ARM_SMMU_FEAT_COHERENCY;

	return 0;
}
#else
static inline int arm_smmu_device_acpi_probe(struct platform_device *pdev,
					     struct arm_smmu_device *smmu)
{
	return -ENODEV;
}
#endif

static int arm_smmu_device_dt_probe(struct platform_device *pdev,
				    struct arm_smmu_device *smmu)
{
	struct device *dev = &pdev->dev;
	u32 cells;
	int ret = -EINVAL;

	if (of_property_read_u32(dev->of_node, "#iommu-cells", &cells))
		dev_err(dev, "missing #iommu-cells property\n");
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	else {
		dev_info(dev, "#iommu-cells value (%d)\n", cells);
		smmu->cell_size = cells;
		ret = 0;
	}
#else
	else if (cells != 1)
		dev_err(dev, "invalid #iommu-cells value (%d)\n", cells);
	else
		ret = 0;
#endif
	parse_driver_options(smmu);

	if (of_dma_is_coherent(dev->of_node))
		smmu->features |= ARM_SMMU_FEAT_COHERENCY;

	return ret;
}

static unsigned long arm_smmu_resource_size(struct arm_smmu_device *smmu)
{
	if (smmu->options & ARM_SMMU_OPT_PAGE0_REGS_ONLY)
		return SZ_64K;
	else
		return SZ_128K;
}

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
static int arm_smmu_set_bus_ops(struct iommu_ops *ops)
{
	int err;

#ifdef CONFIG_PCI
	if (pci_bus_type.iommu_ops != ops) {
		err = bus_set_iommu(&pci_bus_type, ops);
		if (err)
			return err;
	}
#endif
#ifdef CONFIG_ARM_AMBA
	if (amba_bustype.iommu_ops != ops) {
		err = bus_set_iommu(&amba_bustype, ops);
		if (err)
			goto err_reset_pci_ops;
	}
#endif
	if (platform_bus_type.iommu_ops != ops) {
		err = bus_set_iommu(&platform_bus_type, ops);
		if (err)
			goto err_reset_amba_ops;
	}

	return 0;

err_reset_amba_ops:
#ifdef CONFIG_ARM_AMBA
	bus_set_iommu(&amba_bustype, NULL);
#endif
err_reset_pci_ops: __maybe_unused;
#ifdef CONFIG_PCI
	bus_set_iommu(&pci_bus_type, NULL);
#endif
	return err;
}
#endif

static void __iomem *arm_smmu_ioremap(struct device *dev, resource_size_t start,
				      resource_size_t size)
{
	struct resource res = DEFINE_RES_MEM(start, size);

	return devm_ioremap_resource(dev, &res);
}

#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
static int xring_smmu_restore_reg_ctx(struct arm_smmu_device *smmu)
{
	struct arm_smmu_reg_context *context = &smmu->context;
	u32 reg;
	int ret;

	if (!smmu->base) {
		xring_smmu_warn("smmu not initialize\n");
		return 0;
	}

	if (context->IDR0 == 0) {
		xring_smmu_warn("smmu not initialilze\n");
		return 0;
	}

	reg = readl_relaxed(smmu->base + ARM_SMMU_CR0);
	if (reg & CR0_SMMUEN) {
		xring_smmu_err("smmu already enabled, abort all transation\n");
		arm_smmu_update_gbpa(smmu, GBPA_ABORT, 0);
	}

	/* disable smmu */
	ret = arm_smmu_write_reg_sync(smmu, 0, ARM_SMMU_CR0, ARM_SMMU_CR0ACK);
	if (ret) {
		xring_smmu_err("disable smmu fail\n");
		return -1;
	}

	/* restore smmu regs */
	writel(context->CR1, smmu->base + ARM_SMMU_CR1);
	writel(context->CR2, smmu->base + ARM_SMMU_CR2);

	writeq(context->STRTAB_BASE, smmu->base + ARM_SMMU_STRTAB_BASE);
	writel(context->STRTAB_BASE_CFG, smmu->base + ARM_SMMU_STRTAB_BASE_CFG);

	writeq(context->CMDQ_BASE, smmu->base + ARM_SMMU_CMDQ_BASE);
	writel(context->CMDQ_PROD, smmu->base + ARM_SMMU_CMDQ_PROD);
	writel(context->CMDQ_CONS, smmu->base + ARM_SMMU_CMDQ_CONS);

	writeq(context->EVTQ_BASE, smmu->base + ARM_SMMU_EVTQ_BASE);
	writel(context->EVTQ_PROD, smmu->page1 + ARM_SMMU_EVTQ_PROD);
	writel(context->EVTQ_CONS, smmu->page1 + ARM_SMMU_EVTQ_CONS);

	ret = arm_smmu_write_reg_sync(smmu, context->IRQ_CTRL, ARM_SMMU_IRQ_CTRL,
							ARM_SMMU_IRQ_CTRLACK);
	if (ret) {
		xring_smmu_err("enable irq fail\n");
		return -1;
	}

	ret = arm_smmu_write_reg_sync(smmu, CR0_CMDQEN | CR0_EVTQEN | CR0_SMMUEN,
						ARM_SMMU_CR0, ARM_SMMU_CR0ACK);
	if (ret) {
		xring_smmu_err("enable smmu fail\n");
		return -1;
	}

	return 0;
}

static void xring_smmu_save_reg_ctx(struct arm_smmu_device *smmu)
{
	struct arm_smmu_reg_context *context = &smmu->context;

	context->IDR0 = readl(smmu->base + ARM_SMMU_IDR0);
	context->IDR1 = readl(smmu->base + ARM_SMMU_IDR1);
	context->IDR3 = readl(smmu->base + ARM_SMMU_IDR3);
	context->IDR5 = readl(smmu->base + ARM_SMMU_IDR5);
	context->CR0 = readl(smmu->base + ARM_SMMU_CR0);
	context->CR0ACK = readl(smmu->base + ARM_SMMU_CR0ACK);
	context->CR1 = readl(smmu->base + ARM_SMMU_CR1);
	context->CR2 = readl(smmu->base + ARM_SMMU_CR2);
	context->GBPA = readl(smmu->base + ARM_SMMU_GBPA);
	context->IRQ_CTRL = readl(smmu->base + ARM_SMMU_IRQ_CTRL);
	context->IRQ_CTRLACK = readl(smmu->base + ARM_SMMU_IRQ_CTRLACK);
	context->GERROR = readl(smmu->base + ARM_SMMU_GERROR);
	context->GERRORN = readl(smmu->base + ARM_SMMU_GERRORN);
	context->STRTAB_BASE = readq(smmu->base + ARM_SMMU_STRTAB_BASE);
	context->STRTAB_BASE_CFG = readl(smmu->base + ARM_SMMU_STRTAB_BASE_CFG);
	context->CMDQ_BASE = readq(smmu->base + ARM_SMMU_CMDQ_BASE);
	context->CMDQ_PROD = readl(smmu->base + ARM_SMMU_CMDQ_PROD);
	context->CMDQ_CONS = readl(smmu->base + ARM_SMMU_CMDQ_CONS);
	context->EVTQ_BASE = readq(smmu->base + ARM_SMMU_EVTQ_BASE);
	context->EVTQ_PROD = readl(smmu->page1 + ARM_SMMU_EVTQ_PROD);
	context->EVTQ_CONS = readl(smmu->page1 + ARM_SMMU_EVTQ_CONS);
}
#endif

static int xring_smmu_wrapper_reg_polling(void __iomem *addr,
					u32 bit_mask, u32 bit_flag)
{
	unsigned long expires;
	u32 val;

	expires = jiffies + msecs_to_jiffies(500);
	val = readl(addr);
	while ((val & bit_mask) != bit_flag) {
		if (time_after(jiffies, expires)) {
			xring_smmu_err("timeout, addr: %p, bit_mask: 0x%x, bit_flag: %d\n",
					addr, bit_mask, bit_flag);
			return -1;
		}

		udelay(50);
		val = readl(addr);
	}

	return 0;
}

static int xring_smmu_tcu_ctrl_power(void __iomem *base, bool on)
{
	u32 val;
	int ret = 0;

	if (!base) {
		xring_smmu_err("fail\n");
		return -1;
	}

	if (on) {
		val = readl(base + ARM_SMMU_TCU_QCH_PD_STATE);
		if (val & SC_SMMU_TCU_QACCEPTN_PD) {
			xring_smmu_err("smmu tcu already linkup\n"
				"ARM_SMMU_TCU_QCH_PD_STATE:0x%x\n", val);
			return -1;
		}
	}

	/* CG control */
	writel(SC_SMMU_TCU_QREQN_CG_SET, base + ARM_SMMU_TCU_QCH_CG_CTRL);

	/* CG state */
	ret = xring_smmu_wrapper_reg_polling(base + ARM_SMMU_TCU_QCH_CG_STATE,
					SC_SMMU_TCU_QACCEPTN_CG_ST,
					SC_SMMU_TCU_QACCEPTN_CG_ST);
	if (ret < 0) {
		xring_smmu_err("CG control fail\n");
		xring_smmu_err("ARM_SMMU_TCU_QCH_CG_CTRL:0x%x, ARM_SMMU_TCU_QCH_CG_STATE:0x%x\n",
				readl(base + ARM_SMMU_TCU_QCH_CG_CTRL),
				readl(base + ARM_SMMU_TCU_QCH_CG_STATE));
		return ret;
	}

	if (on) {
		/* PD control */
		writel(SC_SMMU_TCU_QREQN_PD, base + ARM_SMMU_TCU_QCH_PD_CTRL);

		/* PD state */
		ret = xring_smmu_wrapper_reg_polling(
					base + ARM_SMMU_TCU_QCH_PD_STATE,
					SC_SMMU_TCU_QACCEPTN_PD,
					SC_SMMU_TCU_QACCEPTN_PD);
		if (ret < 0) {
			xring_smmu_err("PD control fail\n");
			xring_smmu_err("ARM_SMMU_TCU_QCH_PD_CTRL:0x%x, ARM_SMMU_TCU_QCH_PD_STATE:0x%x\n",
				readl(base + ARM_SMMU_TCU_QCH_PD_CTRL),
				readl(base + ARM_SMMU_TCU_QCH_PD_STATE));
			return ret;
		}

		/* CG control: Turn on automatic clock gating */
		writel(SC_SMMU_TCU_QREQN_CG_SET | SC_SMMU_TCU_QCH_CG_CTRL_EN,
				base + ARM_SMMU_TCU_QCH_CG_CTRL);
	} else {
		/* PD control */
		writel(0, base + ARM_SMMU_TCU_QCH_PD_CTRL);

		/* PD state */
		ret = xring_smmu_wrapper_reg_polling(
					base + ARM_SMMU_TCU_QCH_PD_STATE,
					SC_SMMU_TCU_QACCEPTN_PD, 0);
		if (ret < 0) {
			xring_smmu_err("PD control fail\n");
			xring_smmu_err("ARM_SMMU_TCU_QCH_PD_CTRL:0x%x, ARM_SMMU_TCU_QCH_PD_STATE:0x%x\n",
				readl(base + ARM_SMMU_TCU_QCH_PD_CTRL),
				readl(base + ARM_SMMU_TCU_QCH_PD_STATE));
			return ret;
		}

		/* CG control */
		writel(0, base + ARM_SMMU_TCU_QCH_CG_CTRL);

		/* CG state */
		ret = xring_smmu_wrapper_reg_polling(
					base + ARM_SMMU_TCU_QCH_CG_STATE,
					SC_SMMU_TCU_QACCEPTN_CG_ST, 0);
		if (ret < 0) {
			xring_smmu_err("CG control fail\n");
			xring_smmu_err("ARM_SMMU_TCU_QCH_CG_CTRL:0x%x, ARM_SMMU_TCU_QCH_CG_STATE:0x%x\n",
					readl(base + ARM_SMMU_TCU_QCH_CG_CTRL),
					readl(base + ARM_SMMU_TCU_QCH_CG_STATE));
			return ret;
		}
	}

	xring_smmu_debug("ARM_SMMU_TCU_QCH_CG_CTRL:0x%x, ARM_SMMU_TCU_QCH_CG_STATE:0x%x\n"
		"ARM_SMMU_TCU_QCH_PD_CTRL:0x%x, ARM_SMMU_TCU_QCH_PD_STATE:0x%x\n",
		readl(base + ARM_SMMU_TCU_QCH_CG_CTRL),
		readl(base + ARM_SMMU_TCU_QCH_CG_STATE),
		readl(base + ARM_SMMU_TCU_QCH_PD_CTRL),
		readl(base + ARM_SMMU_TCU_QCH_PD_STATE));
	xring_smmu_debug("%s ok\n", (on == true) ? "on" : "off");
	return ret;
}

static int xring_smmu_tbu_ctrl_power(void __iomem *base, bool on)
{
	int ret = 0;

	/* CG control */
	writel(SC_SMMU_TBU_QREQN_CG_SET, base + ARM_SMMU_TBU_QCH_CG_CTRL);

	/* CG state */
	ret = xring_smmu_wrapper_reg_polling(base + ARM_SMMU_TBU_QCH_CG_STATE,
					SC_SMMU_TBU_QACCEPTN_CG_ST,
					SC_SMMU_TBU_QACCEPTN_CG_ST);
	if (ret < 0) {
		xring_smmu_err("CG control fail\n");
		xring_smmu_err("ARM_SMMU_TBU_QCH_CG_CTRL:0x%x, ARM_SMMU_TBU_QCH_CG_STATE:0x%x\n",
			readl(base + ARM_SMMU_TBU_QCH_CG_CTRL),
			readl(base + ARM_SMMU_TBU_QCH_CG_STATE));
		return ret;
	}

	if (on) {
		/* PD control */
		writel(SC_SMMU_TBU_QREQN_PD, base + ARM_SMMU_TBU_QCH_PD_CTRL);

		/* PD state */
		ret = xring_smmu_wrapper_reg_polling(
					base + ARM_SMMU_TBU_QCH_PD_STATE,
					SC_SMMU_TBU_QACCEPTN_PD,
					SC_SMMU_TBU_QACCEPTN_PD);
		if (ret < 0) {
			xring_smmu_err("PD control fail\n");
			xring_smmu_err("ARM_SMMU_TBU_QCH_PD_CTRL:0x%x, ARM_SMMU_TBU_QCH_PD_STATE:0x%x\n",
				readl(base + ARM_SMMU_TBU_QCH_PD_CTRL),
				readl(base + ARM_SMMU_TBU_QCH_PD_STATE));
			return ret;
		}

		/* CG control: Turn on automatic clock gating */
		writel(SC_SMMU_TBU_QREQN_CG_SET | SC_SMMU_TBU_QCH_CG_CTRL_EN,
				base + ARM_SMMU_TBU_QCH_CG_CTRL);
	} else {
		/* PD control */
		writel(0, base + ARM_SMMU_TBU_QCH_PD_CTRL);

		/* PD state */
		ret = xring_smmu_wrapper_reg_polling(
					base + ARM_SMMU_TBU_QCH_PD_STATE,
					SC_SMMU_TBU_QACCEPTN_PD,
					0);
		if (ret < 0) {
			xring_smmu_err("PD control fail\n");
			xring_smmu_err("ARM_SMMU_TBU_QCH_PD_CTRL:0x%x, ARM_SMMU_TBU_QCH_PD_STATE:0x%x\n",
				readl(base + ARM_SMMU_TBU_QCH_PD_CTRL),
				readl(base + ARM_SMMU_TBU_QCH_PD_STATE));
			return ret;
		}

		/* CG control */
		writel(0, base + ARM_SMMU_TBU_QCH_CG_CTRL);

		/* CG state */
		ret = xring_smmu_wrapper_reg_polling(
					base + ARM_SMMU_TBU_QCH_CG_STATE,
					SC_SMMU_TBU_QACCEPTN_CG_ST,
					0);
		if (ret < 0) {
			xring_smmu_err("CG control fail\n");
			xring_smmu_err("ARM_SMMU_TBU_QCH_CG_CTRL:0x%x, ARM_SMMU_TBU_QCH_CG_STATE:0x%x\n",
				readl(base + ARM_SMMU_TBU_QCH_CG_CTRL),
				readl(base + ARM_SMMU_TBU_QCH_CG_STATE));
			return ret;
		}
	}

	xring_smmu_debug("ARM_SMMU_TBU_QCH_CG_CTRL:0x%x, ARM_SMMU_TBU_QCH_CG_STATE:0x%x\n"
		"ARM_SMMU_TBU_QCH_PD_CTRL:0x%x, ARM_SMMU_TBU_QCH_PD_STATE:0x%x\n",
		readl(base + ARM_SMMU_TBU_QCH_CG_CTRL),
		readl(base + ARM_SMMU_TBU_QCH_CG_STATE),
		readl(base + ARM_SMMU_TBU_QCH_PD_CTRL),
		readl(base + ARM_SMMU_TBU_QCH_PD_STATE));
	xring_smmu_debug("%s ok\n", (on == true) ? "on" : "off");
	return ret;
}

#ifdef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
int xring_smmu_tcu_ctrl(struct device *dev, enum xring_smmu_tcu_option option)
{
	struct arm_smmu_master *master = NULL;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_domain *smmu_domain = NULL;
	struct xring_smmu_wrapper *wrapper = NULL;
	unsigned long flags;
#ifndef CONFIG_XRING_SMMU_DOMAIN_SHARE
	u32 sid;
#endif
	int ret, i;

	dev_info(dev, "%s %s\n", __func__, (option == true) ? "off" : "on");
	master = dev_iommu_priv_get(dev);
	if (!master) {
		xring_smmu_err("master has not initialize\n");
		return -1;
	}

	smmu = master->smmu;
	smmu_domain = master->domain;
	wrapper = smmu->wrapper;
	switch (option) {
	case XRING_SMMU_TCU_POWER_ON:
		spin_lock_irqsave(&smmu->tcu_lock, flags);
		if (atomic_read(&smmu->tcu_link_cnt) == 0) {
			xring_smmu_tcu_ctrl_power(wrapper->tcu.base, true);
			arm_smmu_device_reset(smmu, smmu->bypass);
		}
		atomic_inc(&smmu->tcu_link_cnt);
		/* invalidate CD */
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
		for (i = 0; i < master->num_ssids; i++)
			arm_smmu_sync_cd(smmu_domain, master->ssids[i], true);
#else
		arm_smmu_sync_cd(smmu_domain, 0, true);
#endif
		/* invalidate STE */
#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
		arm_smmu_sync_ste_for_sid(smmu, master->sid);
#else
		for (i = 0; i < master->num_streams; i++) {
			sid = master->streams[i].id;
			arm_smmu_sync_ste_for_sid(smmu, sid);
		}
#endif
		/* flush TLB */
		arm_smmu_tlb_inv_context(smmu_domain);
		spin_unlock_irqrestore(&smmu->tcu_lock, flags);
		break;
	case XRING_SMMU_TCU_POWER_OFF:
		spin_lock_irqsave(&smmu->tcu_lock, flags);
		atomic_dec(&smmu->tcu_link_cnt);
		if (atomic_read(&smmu->tcu_link_cnt) == 0) {
			ret = arm_smmu_device_disable(smmu);
			if (ret) {
				spin_unlock_irqrestore(&smmu->tcu_lock, flags);
				return -1;
			}
			xring_smmu_tcu_ctrl_power(wrapper->tcu.base, false);
		}
		spin_unlock_irqrestore(&smmu->tcu_lock, flags);
		break;
	default:
		break;
	}

	return 0;
}
EXPORT_SYMBOL(xring_smmu_tcu_ctrl);
#endif

int xring_smmu_tbu_ctrl(struct device *dev, char *tbu_name, enum xring_smmu_tbu_option option)
{
	struct xring_smmu_wrapper *wrapper = NULL;
	struct arm_smmu_master *master = NULL;
	struct arm_smmu_device *smmu = NULL;
	bool found = false;
	int i;

	dev_info(dev, "%s %s\n", __func__, (option == true) ? "off" : "on");
	master = dev_iommu_priv_get(dev);
	if (!master) {
		xring_smmu_err("master has not initialize\n");
		return -1;
	}
	smmu = master->smmu;
	wrapper = smmu->wrapper;
	xring_smmu_info(" %s\n", tbu_name);
	for (i = 0; i < XRING_SMMU_TBU_MAX_NUMBER; i++) {
		if (!strcmp(wrapper->tbu[i].name, tbu_name)) {
			found = true;
			break;
		}
	}
	if (!found) {
		xring_smmu_err("tbu not find\n");
		return -1;
	}
	xring_smmu_info("name=%s, start=0x%llx\n",
			wrapper->tbu[i].name, wrapper->tbu[i].start);
	switch (option) {
	case XRING_SMMU_TBU_POWER_ON:
		mutex_lock(&wrapper->mutex);
#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
		if (wrapper->tbu_linkup_num == 0) {
			int ret = xring_smmu_tcu_ctrl_power(wrapper->tcu.base, true);

			if (!ret)
				xring_smmu_restore_reg_ctx(smmu);
		}
#endif
		xring_smmu_tbu_ctrl_power(wrapper->tbu[i].base, true);
		wrapper->tbu_linkup_num++;
		mutex_unlock(&wrapper->mutex);
		break;
	case XRING_SMMU_TBU_POWER_OFF:
		mutex_lock(&wrapper->mutex);
		xring_smmu_tbu_ctrl_power(wrapper->tbu[i].base, false);
		wrapper->tbu_linkup_num--;
#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
		if (wrapper->tbu_linkup_num == 0) {
			xring_smmu_save_reg_ctx(smmu);
			xring_smmu_tcu_ctrl_power(wrapper->tcu.base, false);
		}
#endif
		mutex_unlock(&wrapper->mutex);
		break;
	default:
		xring_smmu_err("unknown option: %d\n", option);
		break;
	}

	return 0;
}
EXPORT_SYMBOL(xring_smmu_tbu_ctrl);

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
int xring_smmu_sid_set(struct device *dev, char *sid_name,
				int sid, int ssid, int ssidv)
{
#else
int xring_smmu_sid_set(struct device *dev, char *sid_name, int sid)
{
	int ssid = 0;
	int ssidv = 0;
#endif
	struct xring_smmu_wrapper *wrapper = NULL;
	struct arm_smmu_master *master = NULL;
	struct arm_smmu_device *smmu = NULL;
	void __iomem *base = NULL;

	master = dev_iommu_priv_get(dev);
	if (!master) {
		xring_smmu_err("master has not initialize\n");
		return -EINVAL;
	}
	smmu = master->smmu;
	wrapper = smmu->wrapper;
	base = wrapper->id.base;
	if (!base) {
		xring_smmu_err("%s has no sid\n", wrapper->name);
		return -EINVAL;
	}
	xring_smmu_info("sid name is %s, sid: %d, ssid: %d, ssidv: %d\n",
		sid_name, sid, ssid, ssidv);
	if (!strcmp(sid_name, "media2_smmu_sid0")) {
		writel(sid, base + ARM_SMMU_SC_ISP_NS_SID0);
		writel(ssid, base + ARM_SMMU_SC_ISP_NS_SSID0);
		writel(ssidv, base + ARM_SMMU_SC_ISP_NS_SSIDV0);
	} else if (!strcmp(sid_name, "media2_smmu_sid1")) {
		writel(sid, base + ARM_SMMU_SC_ISP_NS_SID1);
		writel(ssid, base + ARM_SMMU_SC_ISP_NS_SSID1);
		writel(ssidv, base + ARM_SMMU_SC_ISP_NS_SSIDV1);
	} else if (!strcmp(sid_name, "media2_smmu_sid2")) {
		writel(sid, base + ARM_SMMU_SC_ISP_NS_SID2);
		writel(ssid, base + ARM_SMMU_SC_ISP_NS_SSID2);
		writel(ssidv, base + ARM_SMMU_SC_ISP_NS_SSIDV2);
	} else if (!strcmp(sid_name, "media2_smmu_sid3")) {
		writel(sid, base + ARM_SMMU_SC_VENC_NS_SID0);
		writel(ssid, base + ARM_SMMU_SC_VENC_NS_SSID0);
		writel(ssidv, base + ARM_SMMU_SC_VENC_NS_SSIDV0);
		writel(sid, base + ARM_SMMU_SC_VENC_NS_SID1);
		writel(ssid, base + ARM_SMMU_SC_VENC_NS_SSID1);
		writel(ssidv, base + ARM_SMMU_SC_VENC_NS_SSIDV1);
		writel(sid, base + ARM_SMMU_SC_VENC_NS_SID2);
		writel(ssid, base + ARM_SMMU_SC_VENC_NS_SSID2);
		writel(ssidv, base + ARM_SMMU_SC_VENC_NS_SSIDV2);
	} else if (!strcmp(sid_name, "media1_smmu_sid0")) {
		writel(sid, base + ARM_SMMU_SC_VDEC_NS_SID0);
		writel(ssid, base + ARM_SMMU_SC_VDEC_NS_SSID0);
		writel(ssidv, base + ARM_SMMU_SC_VDEC_NS_SSIDV0);
		writel(sid, base + ARM_SMMU_SC_VDEC_NS_SID1);
		writel(ssid, base + ARM_SMMU_SC_VDEC_NS_SSID1);
		writel(ssidv, base + ARM_SMMU_SC_VDEC_NS_SSIDV1);
	} else if (!strcmp(sid_name, "pcie_smmu_sid0")) {
		writel((ssidv << 16) | (ssid << 8) | sid,
				base + ARM_SMMU_SC_PCIE_NS_SID0);
	} else if (!strcmp(sid_name, "pcie_smmu_sid1")) {
		writel((ssidv << 16) | (ssid << 8) | sid,
				base + ARM_SMMU_SC_PCIE_NS_SID1);
	} else {
		xring_smmu_err("sid properity is not set in dts!!!\n");
	}

	return 0;
}
EXPORT_SYMBOL(xring_smmu_sid_set);

static int xring_smmu_tcu_irpt_clear(struct arm_smmu_device *smmu,
					u32 irpt_clear)
{
	struct xring_smmu_wrapper *wrapper = NULL;
	void __iomem *base = NULL;
	unsigned long expires;
	u32 val;

	wrapper = smmu->wrapper;
	base = wrapper->tcu.base;

	val = readl(base + ARM_SMMU_TCU_IRPT_STATE);
	xring_smmu_info("state=0x%x\n", val);

	if (val == 0)
		return 0;

	/* write 0 to corresponding tcu irpt clear bit */
	val &= ~irpt_clear;
	writel(val, base + ARM_SMMU_TCU_IRPT_CLEAR);

	/* polling until 0 of corresponding tcu clear irpt bit */
	expires = jiffies + msecs_to_jiffies(1);
	do {
		val = readl(base + ARM_SMMU_TCU_IRPT_CLEAR);

		if (time_after(jiffies, expires)) {
			xring_smmu_err("timeout\n");
			return -1;
		}
	} while (val & irpt_clear);

	/* write 1 to corresponding tcu irpt clear bit */
	val |= irpt_clear;
	writel(val, base + ARM_SMMU_TCU_IRPT_CLEAR);

	val = readl(base + ARM_SMMU_TCU_IRPT_STATE);
	xring_smmu_info("state=0x%x\n", val);

	return 0;
}

#ifdef XRING_SMMU_TBU_IRQ
static int xring_smmu_tbu_irpt_clear(struct xring_smmu_tbu *tbu, u32 irpt_clear)
{
	void __iomem *base = NULL;
	unsigned long expires;
	u32 val;

	base = tbu->base;
	if (!base) {
		xring_smmu_err("get base fail\n");
		return -1;
	}

	val = readl(base + ARM_SMMU_TBU_IRPT_STATE);
	xring_smmu_info("state=0x%x\n", val);

	if (val == 0)
		return 0;

	/* write 0 to corresponding tcu irpt clear bit */
	val &= ~irpt_clear;
	writel(val, base + ARM_SMMU_TBU_IRPT_CLEAR);

	/* polling until 0 of corresponding tcu clear irpt bit */
	expires = jiffies + msecs_to_jiffies(1);
	do {
		val = readl(base + ARM_SMMU_TBU_IRPT_CLEAR);

		if (time_after(jiffies, expires)) {
			xring_smmu_err("timeout\n");
			return -1;
		}
	} while (val & irpt_clear);

	/* write 1 to corresponding tcu irpt clear bit */
	val |= irpt_clear;
	writel(val, base + ARM_SMMU_TBU_IRPT_CLEAR);

	val = readl(base + ARM_SMMU_TBU_IRPT_STATE);
	xring_smmu_info("state=0x%x\n", val);

	return 0;
}
#endif

#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
static int xring_smmu_power_on(struct arm_smmu_device *smmu)
{
	struct xring_smmu_wrapper *wrapper = smmu->wrapper;
	int ret;

	ret = xring_smmu_tcu_ctrl_power(wrapper->tcu.base, true);
	if (ret < 0) {
		xring_smmu_err("power on fail\n", wrapper->tcu.name);
		return ret;
	}

	xring_smmu_info("ok\n");

	return 0;
}
#endif

static void xring_smmu_wrapper_get_irqs(struct platform_device *pdev,
					struct arm_smmu_device *smmu)
{
	struct xring_smmu_wrapper *wrapper = NULL;
	char irq_name[16];
	int irq;
	int i;

	wrapper = smmu->wrapper;
	if (!wrapper) {
		dev_warn(smmu->dev, "%s smmu has no wrapper\n", __func__);
		return;
	}

	irq = platform_get_irq_byname_optional(pdev, "tcu_ras");
	if (irq > 0)
		wrapper->tcu.ras_irq = irq;

	for (i = 0; i < XRING_SMMU_TBU_MAX_NUMBER; i++) {
		if (wrapper->tbu[i].use != true)
			continue;

		snprintf(irq_name, sizeof(irq_name), "tbu%d_ras", i);
		irq = platform_get_irq_byname_optional(pdev, irq_name);
		if (irq > 0)
			wrapper->tbu[i].ras_irq = irq;
	}
}

static void xring_smmu_wrapper_setup_irqs(struct arm_smmu_device *smmu)
{
	struct xring_smmu_wrapper *wrapper = NULL;
	int irq;
	int ret;
#ifdef XRING_SMMU_TBU_IRQ
	char irq_name[16];
	int i;
#endif

	wrapper = smmu->wrapper;
	if (!wrapper) {
		dev_warn(smmu->dev, "%s smmu has no wrapper\n", __func__);
		return;
	}

	irq = wrapper->tcu.ras_irq;
	if (irq > 0) {
		ret = devm_request_irq(smmu->dev, irq,
				arm_smmu_tcu_ras_handler, IRQF_ONESHOT,
				"arm-smmu-v3-tcu_ras", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable tcu_ras irq\n");
	}

#ifdef XRING_SMMU_TBU_IRQ
	for (i = 0; i < XRING_SMMU_TBU_MAX_NUMBER; i++) {
		if (wrapper->tbu[i].use != true)
			continue;

		irq = wrapper->tbu[i].ras_irq;
		snprintf(irq_name, sizeof(irq_name), "tbu%d_ras", i);
		ret = devm_request_irq(smmu->dev, irq,
				arm_smmu_tbu_ras_handler, IRQF_ONESHOT,
				irq_name, &wrapper->tbu[i]);
		if (ret < 0)
			dev_warn(smmu->dev,
					"failed to enable %s irq\n", irq_name);
	}
#endif
}

static int xring_smmu_wrapper_init(struct platform_device *pdev,
				struct arm_smmu_device *smmu)
{
	struct xring_smmu_wrapper *wrapper = NULL;
	struct resource res;
	struct device *dev = smmu->dev;
	struct device_node *np = dev->of_node, *tmp = NULL;
	int i = 0;

	wrapper = vzalloc(sizeof(struct xring_smmu_wrapper));
	if (!wrapper)
		return -1;

	strscpy(wrapper->name, np->name, sizeof(wrapper->name));
	xring_smmu_info(" wrapper name %s\n", wrapper->name);

	for_each_available_child_of_node(np, tmp) {
		char *name;
		char *status;

		name = (char *)of_get_property(tmp, "compatible", 0);
		status = (char *)of_get_property(tmp, "status", 0);

		if (!strcmp(status, "okay")) {
			if (!strcmp(name, "xring,smmu_tcu")) {
				strscpy(wrapper->tcu.name, tmp->name,
							sizeof(wrapper->tcu.name));
				wrapper->tcu.base = of_iomap(tmp, 0);
				of_address_to_resource(tmp, 0, &res);
				wrapper->tcu.start = res.start;
				wrapper->tcu.len = res.end - res.start + 1;
				xring_smmu_info(" tcu name: %s, start=0x%llx, len=0x%llx\n",
							wrapper->tcu.name,
							wrapper->tcu.start,
							wrapper->tcu.len);
			} else if (!strcmp(name, "xring,smmu_tbu")) {
				strscpy(wrapper->tbu[i].name, tmp->name,
							sizeof(wrapper->tbu[i].name));
				wrapper->tbu[i].base = of_iomap(tmp, 0);
				of_address_to_resource(tmp, 0, &res);
				wrapper->tbu[i].start = res.start;
				wrapper->tbu[i].len = res.end - res.start + 1;
				wrapper->tbu[i].use = true;
				xring_smmu_info(" tbu name: %s, start=0x%llx, len=0x%llx\n",
							wrapper->tbu[i].name,
							wrapper->tbu[i].start,
							wrapper->tbu[i].len);
				i++;
			} else if (!strcmp(name, "xring,smmu_id")) {
				strscpy(wrapper->id.name, tmp->name,
							sizeof(wrapper->id.name));
				wrapper->id.base = of_iomap(tmp, 0);
				of_address_to_resource(tmp, 0, &res);
				wrapper->id.start = res.start;
				wrapper->id.len = res.end - res.start + 1;
				xring_smmu_info(" id name: %s, start=0x%llx, len=0x%llx\n",
							wrapper->id.name,
							wrapper->id.start,
							wrapper->id.len);
			}
		}
	}

	wrapper->tbu_linkup_num = 0;
	mutex_init(&wrapper->mutex);
	smmu->wrapper = wrapper;
	wrapper->smmu = smmu;
	atomic_set(&smmu->tcu_link_cnt, 0);
	return 0;
}

static void xring_smmu_wrapper_exit(struct arm_smmu_device *smmu)
{
	if (smmu->wrapper) {
		mutex_destroy(&smmu->wrapper->mutex);
		vfree(smmu->wrapper);
		smmu->wrapper = NULL;
	}
}

static int arm_smmu_device_probe(struct platform_device *pdev)
{
	int irq, ret;
	struct resource *res;
	resource_size_t ioaddr;
	struct arm_smmu_device *smmu;
	struct device *dev = &pdev->dev;
	struct device_node *node = NULL;
	const char *regulator_name = NULL;

	node = dev->of_node;
	if (!node) {
		xring_smmu_err("arm smmu-v3 node not found\n");
		return -EINVAL;
	}

	smmu = devm_kzalloc(dev, sizeof(*smmu), GFP_KERNEL);
	if (!smmu)
		return -ENOMEM;
	smmu->dev = dev;

	if (dev->of_node) {
		ret = arm_smmu_device_dt_probe(pdev, smmu);
		if (ret == -EINVAL)
			return ret;
	} else {
		return -ENODEV;
	}

	/* Set bypass mode according to firmware probing result */
	smmu->bypass = !!ret;
	if (of_property_read_bool(node, "smmu-global-bypass"))
		smmu->bypass = true;

	xring_smmu_info(" smmu->bypass:%d\n", smmu->bypass);

	/* Base address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -EINVAL;
	if (resource_size(res) < arm_smmu_resource_size(smmu)) {
		dev_err(dev, "MMIO region too small (%pr)\n", res);
		return -EINVAL;
	}
	ioaddr = res->start;

	/*
	 * Don't map the IMPLEMENTATION DEFINED regions, since they may contain
	 * the PMCG registers which are reserved by the PMU driver.
	 */
	smmu->base = arm_smmu_ioremap(dev, ioaddr, ARM_SMMU_REG_SZ);
	if (IS_ERR(smmu->base))
		return PTR_ERR(smmu->base);

	if (arm_smmu_resource_size(smmu) > SZ_64K) {
		smmu->page1 = arm_smmu_ioremap(dev, ioaddr + SZ_64K,
						ARM_SMMU_REG_SZ);
		if (IS_ERR(smmu->page1))
			return PTR_ERR(smmu->page1);
	} else {
		smmu->page1 = smmu->base;
	}

	ret = xring_smmu_wrapper_init(pdev, smmu);
	if (ret < 0) {
		xring_smmu_err("xring_smmu_wrapper_init fail\n");
		return ret;
	}

	ret = of_property_read_string(node, "regulator-name", &regulator_name);
	if (!regulator_name) {
		xring_smmu_err("Smmu: %s node regulator name is empty\n", node->name);
		return ret;
	}

	xring_smmu_info(" regulator_name=%s\n", regulator_name);

	smmu->master_rg = regulator_get(dev, regulator_name);
	if (IS_ERR_OR_NULL(smmu->master_rg)) {
		xring_smmu_err("regulator get %s fail\n", regulator_name);
		return -EINVAL;
	}

#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	ret = regulator_enable(smmu->master_rg);
	if (ret < 0) {
		xring_smmu_err("Failed to enable %s regulator\n", regulator_name);
		goto err_regulator;
	}

	xring_smmu_power_on(smmu);
#endif

	/* Interrupt lines */

	irq = platform_get_irq_byname_optional(pdev, "combined");
	if (irq > 0)
		smmu->combined_irq = irq;
	else {
		irq = platform_get_irq_byname_optional(pdev, "eventq");
		if (irq > 0)
			smmu->evtq.q.irq = irq;

		irq = platform_get_irq_byname_optional(pdev, "cmdq-sync");
		if (irq > 0)
			smmu->cmd_sync_irq = irq;

		irq = platform_get_irq_byname_optional(pdev, "gerror");
		if (irq > 0)
			smmu->gerr_irq = irq;

		irq = platform_get_irq_byname_optional(pdev, "priq");
		if (irq > 0)
			smmu->priq.q.irq = irq;

		xring_smmu_wrapper_get_irqs(pdev, smmu);
	}

	arm_smmu_setup_irqs_handler(smmu);

	/* Probe the h/w */
	ret = arm_smmu_device_hw_probe(smmu);
	if (ret)
		goto err_regulator;

	/* Initialise in-memory data structures */
	ret = arm_smmu_init_structures(smmu);
	if (ret)
		goto err_regulator;

	/* Record our private device structure */
	platform_set_drvdata(pdev, smmu);
	spin_lock_init(&smmu->tcu_lock);

#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	/* Reset the device */
	ret = arm_smmu_device_reset(smmu, smmu->bypass);
	if (ret)
		goto err_regulator;
#endif

	/* And we're up. Go go! */
	ret = iommu_device_sysfs_add(&smmu->iommu, dev, NULL,
				     "smmu3.%pa", &ioaddr);
	if (ret)
		goto err_regulator;

	ret = iommu_device_register(&smmu->iommu, &arm_smmu_ops, dev);
	if (ret) {
		dev_err(dev, "Failed to register iommu\n");
		goto err_sysfs_remove;
	}

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	ret = arm_smmu_set_bus_ops(&arm_smmu_ops);
	if (ret)
		goto err_unregister_device;
#endif

#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	xring_smmu_save_reg_ctx(smmu);
#endif

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
	INIT_LIST_HEAD(&smmu->iommu_groups);
	spin_lock_init(&smmu->iommu_groups_lock);
#endif

#ifdef CONFIG_XRING_SMMU_DEBUGFS
	ret = xring_smmu_debugfs_init(smmu);
	if (ret) {
		dev_err(dev, "Failed to init smmu_dump_debugfs!\n");
		goto err_unregister_device;
	}
#endif
	isp_iommu_domain = NULL;
	return 0;

err_unregister_device:
	iommu_device_unregister(&smmu->iommu);
err_sysfs_remove:
	iommu_device_sysfs_remove(&smmu->iommu);

err_regulator:
	regulator_put(smmu->master_rg);

	return ret;
}

static int arm_smmu_device_remove(struct platform_device *pdev)
{
	struct arm_smmu_device *smmu = platform_get_drvdata(pdev);

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	arm_smmu_set_bus_ops(NULL);
#endif
	iommu_device_unregister(&smmu->iommu);
	iommu_device_sysfs_remove(&smmu->iommu);
#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	arm_smmu_device_disable(smmu);
#endif
	xring_smmu_wrapper_exit(smmu);
	iopf_queue_free(smmu->evtq.iopf);
#ifndef CONFIG_XRING_SMMU_TCU_TBU_RECONSTRUCT
	if (regulator_is_enabled(smmu->master_rg))
		regulator_disable(smmu->master_rg);
#endif
	if (smmu->master_rg)
		regulator_put(smmu->master_rg);

#ifdef CONFIG_XRING_SMMU_DEBUGFS
	xring_smmu_debugfs_exit();
#endif

	return 0;
}

static void arm_smmu_device_shutdown(struct platform_device *pdev)
{
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	arm_smmu_device_remove(pdev);
#else
	return;
#endif
}

static const struct of_device_id arm_smmu_of_match[] = {
	{ .compatible = "arm,smmu-v3", },
	{ },
};
MODULE_DEVICE_TABLE(of, arm_smmu_of_match);

static struct platform_driver arm_smmu_driver = {
	.driver	= {
		.name			= "arm-smmu-v3",
		.of_match_table		= arm_smmu_of_match,
		.suppress_bind_attrs	= true,
	},
	.probe	= arm_smmu_device_probe,
	.remove	= arm_smmu_device_remove,
	.shutdown = arm_smmu_device_shutdown,
};

static int __init arm_smmu_init(void)
{
	xring_dma_map_ops_init();
	return platform_driver_register(&arm_smmu_driver);
}

static void __exit arm_smmu_exit(void)
{
	arm_smmu_sva_notifier_synchronize();
	platform_driver_unregister(&arm_smmu_driver);
}
module_init(arm_smmu_init);
module_exit(arm_smmu_exit);

MODULE_DESCRIPTION("IOMMU API for ARM architected SMMUv3 implementations");
MODULE_AUTHOR("Will Deacon <will@kernel.org>");
MODULE_ALIAS("platform:arm-smmu-v3");
MODULE_LICENSE("GPL v2");
