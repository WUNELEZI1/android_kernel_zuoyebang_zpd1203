// SPDX-License-Identifier: GPL-2.0-only
/*
 * UFS Host driver for Xring-UFS
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 *
 */
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/tracepoint.h>
#include <linux/delay.h>
#include <scsi/scsi_cmnd.h>
#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <linux/msi.h>
#include <linux/interrupt.h>
#include "ufshcd.h"
#include "ufshcd-crypto.h"
#include "ufs-xring-mcq.h"
#include "ufs-xring.h"
#include <trace/hooks/ufshcd.h>

static void ufs_xring_mcq_dump_register(struct ufs_hba *hba);
int ufs_xring_mcq_dump_regs(struct ufs_hba *hba, size_t offset, size_t len,
		     const char *prefix)
{
	u32 *regs;
	size_t pos;
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	if (offset % 4 != 0 || len % 4 != 0) /* keep readl happy */
		return -EINVAL;
	regs = kzalloc(len, GFP_ATOMIC);
	if (!regs)
		return -ENOMEM;
	for (pos = 0; pos < len; pos += 4) {
		if (offset == 0 &&
		    pos >= REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER &&
		    pos <= REG_UIC_ERROR_CODE_DME)
			continue;
		regs[pos / 4] = ufsmcq_readl(mcq_info, offset + pos);
	}
	ufs_xring_mcq_hex_dump(prefix, regs, len);
	kfree(regs);
	return 0;
}

static void ufs_xring_mcq_print_evt(struct ufs_hba *hba, u32 id,
			     char *err_name)
{
	int i;
	bool found = false;
	struct ufs_event_hist *e;

	if (id >= UFS_EVT_CNT) {
		dev_err(hba->dev, "input id:%d is invalid\n", id);
		return;
	}
	e = &hba->ufs_stats.event[id];
	for (i = 0; i < UFS_EVENT_HIST_LENGTH; i++) {
		int p = (i + e->pos) % UFS_EVENT_HIST_LENGTH;

		if (e->tstamp[p] == 0)
			continue;
		dev_err(hba->dev, "%s[%d] = 0x%x at %lld us\n", err_name, p,
			e->val[p], ktime_to_us(e->tstamp[p]));
		found = true;
	}
	if (!found)
		dev_err(hba->dev, "No record of %s\n", err_name);
	else
		dev_err(hba->dev, "%s: total cnt=%llu\n", err_name, e->cnt);
}

static void ufs_xring_mcq_print_evt_hist(struct ufs_hba *hba)
{
	ufshcd_dump_regs(hba, 0, UFSHCI_REG_SPACE_SIZE, "host_regs: ");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_PA_ERR, "pa_err");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_DL_ERR, "dl_err");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_NL_ERR, "nl_err");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_TL_ERR, "tl_err");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_DME_ERR, "dme_err");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_AUTO_HIBERN8_ERR,
			 "auto_hibern8_err");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_FATAL_ERR, "fatal_err");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_LINK_STARTUP_FAIL,
			 "link_startup_fail");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_RESUME_ERR, "resume_fail");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_SUSPEND_ERR,
			 "suspend_fail");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_DEV_RESET, "dev_reset");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_HOST_RESET, "host_reset");
	ufs_xring_mcq_print_evt(hba, UFS_EVT_ABORT, "task_abort");
	ufshcd_vops_dbg_register_dump(hba);
}

static void ufs_xring_mcq_print_clk_freqs(struct ufs_hba *hba)
{
	struct ufs_clk_info *clki = NULL;
	struct list_head *head = &hba->clk_list_head;

	if (list_empty(head))
		return;
	list_for_each_entry(clki, head, list) {
		if (!IS_ERR_OR_NULL(clki->clk) && clki->min_freq &&
				clki->max_freq)
			dev_err(hba->dev, "clk: %s, rate: %u\n",
					clki->name, clki->curr_freq);
	}
}

static void ufs_xring_mcq_print_host_state(struct ufs_hba *hba)
{
	struct scsi_device *sdev_ufs = hba->sdev_ufs_device;

	dev_err(hba->dev, "UFS Host state=%d\n", hba->ufshcd_state);
	dev_err(hba->dev, "outstanding reqs=0x%lx tasks=0x%lx\n",
		hba->outstanding_reqs, hba->outstanding_tasks);
	dev_err(hba->dev, "saved_err=0x%x, saved_uic_err=0x%x\n",
		hba->saved_err, hba->saved_uic_err);
	dev_err(hba->dev, "Device power mode=%d, UIC link state=%d\n",
		hba->curr_dev_pwr_mode, hba->uic_link_state);
	dev_err(hba->dev, "PM in progress=%d, sys. suspended=%d\n",
		hba->pm_op_in_progress, hba->is_sys_suspended);
	dev_err(hba->dev, "Auto BKOPS=%d, Host self-block=%d\n",
		hba->auto_bkops_enabled, hba->host->host_self_blocked);
	dev_err(hba->dev, "Clk gate=%d\n", hba->clk_gating.state);
	dev_err(hba->dev,
		"last_hibern8_exit_tstamp at %lld us, hibern8_exit_cnt=%d\n",
		ktime_to_us(hba->ufs_stats.last_hibern8_exit_tstamp),
		hba->ufs_stats.hibern8_exit_cnt);
	dev_err(hba->dev, "last intr at %lld us, last intr status=0x%x\n",
		ktime_to_us(hba->ufs_stats.last_intr_ts),
		hba->ufs_stats.last_intr_status);
	dev_err(hba->dev, "error handling flags=0x%x, req. abort count=%d\n",
		hba->eh_flags, hba->req_abort_count);
	dev_err(hba->dev, "hba->ufs_version=0x%x, Host capabilities=0x%x, caps=0x%x\n",
		hba->ufs_version, hba->capabilities, hba->caps);
	dev_err(hba->dev, "quirks=0x%x, dev. quirks=0x%x\n", hba->quirks,
		hba->dev_quirks);
	if (sdev_ufs)
		dev_err(hba->dev, "UFS dev info: %.8s %.16s rev %.4s\n",
			sdev_ufs->vendor, sdev_ufs->model, sdev_ufs->rev);
	ufs_xring_mcq_print_clk_freqs(hba);
}

static void ufs_xring_mcq_print_pwr_info(struct ufs_hba *hba)
{
	static const char * const names[] = {
		"INVALID MODE",
		"FAST MODE",
		"SLOW_MODE",
		"INVALID MODE",
		"FASTAUTO_MODE",
		"SLOWAUTO_MODE",
		"INVALID MODE",
	};
	/*
	 * Using dev_dbg to avoid messages during runtime PM to avoid
	 * never-ending cycles of messages written back to storage by user space
	 * causing runtime resume, causing more messages and so on.
	 */
	dev_dbg(hba->dev, "%s:[RX, TX]: gear=[%d, %d], lane[%d, %d], pwr[%s, %s], rate = %d\n",
		 __func__,
		 hba->pwr_info.gear_rx, hba->pwr_info.gear_tx,
		 hba->pwr_info.lane_rx, hba->pwr_info.lane_tx,
		 names[hba->pwr_info.pwr_rx],
		 names[hba->pwr_info.pwr_tx],
		 hba->pwr_info.hs_rate);
}

static void ufs_xring_mcq_print_trs_tag(struct ufs_hba *hba, int tag,
					bool pr_prdt)
{
	struct ufshcd_lrb *lrbp;
	int prdt_length;

	lrbp = &hba->lrb[tag];
	dev_err(hba->dev, "UPIU[%d] - issue time %lld us\n",
			tag, ktime_to_us(lrbp->issue_time_stamp));
	dev_err(hba->dev, "UPIU[%d] - complete time %lld us\n",
			tag, ktime_to_us(lrbp->compl_time_stamp));
	dev_err(hba->dev,
		"UPIU[%d] - Transfer Request Descriptor phys@0x%llx\n",
		tag, (u64)lrbp->utrd_dma_addr);
	ufs_xring_mcq_hex_dump("UPIU TRD: ", lrbp->utr_descriptor_ptr,
			sizeof(struct utp_transfer_req_desc));
	dev_err(hba->dev, "UPIU[%d] - Request UPIU phys@0x%llx\n", tag,
		(u64)lrbp->ucd_req_dma_addr);
	ufs_xring_mcq_hex_dump("UPIU REQ: ", lrbp->ucd_req_ptr,
			sizeof(struct utp_upiu_req));
	dev_err(hba->dev, "UPIU[%d] - Response UPIU phys@0x%llx\n", tag,
		(u64)lrbp->ucd_rsp_dma_addr);
	ufs_xring_mcq_hex_dump("UPIU RSP: ", lrbp->ucd_rsp_ptr,
			sizeof(struct utp_upiu_rsp));
	prdt_length = le16_to_cpu(
		lrbp->utr_descriptor_ptr->prd_table_length);
	if (hba->quirks & UFSHCD_QUIRK_PRDT_BYTE_GRAN)
		prdt_length /= hba->sg_entry_size;
	dev_err(hba->dev,
		"UPIU[%d] - PRDT - %d entries  phys@0x%llx\n",
		tag, prdt_length,
		(u64)lrbp->ucd_prdt_dma_addr);
	if (pr_prdt)
		ufs_xring_mcq_hex_dump("UPIU PRDT: ", lrbp->ucd_prdt_ptr,
			hba->sg_entry_size * prdt_length);
}

static void ufs_xring_mcq_print_trs(void *data, struct ufs_hba *hba,
					bool pr_prdt)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	unsigned long bitmap = hba->outstanding_reqs;
	int tag = 0;

	for_each_set_bit(tag, &bitmap, mcq_info->mcq_queue_depth) {
		ufs_xring_mcq_print_trs_tag(hba, tag, pr_prdt);
	}
}

static int ufs_xring_mcq_init_queue(struct ufs_hba *hba)
{
	struct ufs_hw_queue *hwq;
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	int i;

	mcq_info->uhq = devm_kzalloc(hba->dev,
			mcq_info->mcq_nr_hw_queue * sizeof(struct ufs_hw_queue),
			GFP_KERNEL);
	if (!mcq_info->uhq) {
		dev_err(hba->dev, "ufs hw queue memory allocation failed\n");
		return -ENOMEM;
	}
	for (i = 0; i < mcq_info->mcq_nr_hw_queue; i++) {
		hwq = &mcq_info->uhq[i];
		hwq->max_entries = mcq_info->mcq_queue_depth;
		spin_lock_init(&hwq->sq_lock);
		spin_lock_init(&hwq->cq_lock);
		mutex_init(&hwq->sq_mutex);
	}
	return 0;
}

/* Resources */
static const struct ufshcd_res_info ufs_res_info[RES_MAX] = {
	{.name = "ufs_ctrl_mem",},
	{.name = "mcq",},
	/* Submission Queue DAO */
	{.name = "mcq_sqd",},
	/* Submission Queue Interrupt Status */
	{.name = "mcq_sqis",},
	/* Completion Queue DAO */
	{.name = "mcq_cqd",},
	/* Completion Queue Interrupt Status */
	{.name = "mcq_cqis",},
	/* MCQ vendor specific */
	{.name = "mcq_vs",},
};

static int ufs_xring_mcq_config_resource(struct ufs_hba *hba)
{
	struct platform_device *pdev = to_platform_device(hba->dev);
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct ufshcd_res_info *res;
	struct resource *res_mem, *res_mcq;
	int i, ret = 0;

	memcpy(mcq_info->res, ufs_res_info, sizeof(ufs_res_info));
	for (i = 0; i < RES_MAX; i++) {
		res = &mcq_info->res[i];
		res->resource = platform_get_resource_byname(pdev,
							     IORESOURCE_MEM,
							     res->name);
		if (!res->resource) {
			dev_info(hba->dev, "Resource %s not provided\n",
								res->name);
			if (i == RES_UFS)
				return -ENOMEM;
			continue;
		} else if (i == RES_UFS) {
			res_mem = res->resource;
			res->base = hba->mmio_base;
			continue;
		}
		res->base = devm_ioremap_resource(hba->dev, res->resource);
		if (IS_ERR(res->base)) {
			dev_err(hba->dev, "Failed to map res %s, err=%d\n",
					 res->name, (int)PTR_ERR(res->base));
			ret = PTR_ERR(res->base);
			res->base = NULL;
			return ret;
		}
	}
	/* MCQ resource provided in DT */
	res = &mcq_info->res[RES_MCQ];
	/* Bail if MCQ resource is provided */
	if (res->base)
		goto out;
	/* Explicitly allocate MCQ resource from ufs_mem */
	res_mcq = devm_kzalloc(hba->dev, sizeof(*res_mcq), GFP_KERNEL);
	if (!res_mcq)
		return -ENOMEM;
	res_mcq->start = res_mem->start + FIELD_GET(MCQ_QCFGPTR_MASK,
				mcq_info->mcq_capabilities) * MCQ_QCFGPTR_UNIT;
	res_mcq->end = res_mcq->start + mcq_info->mcq_nr_hw_queue *
						MCQ_QCFG_SIZE - 1;
	res_mcq->flags = res_mem->flags;
	res_mcq->name = "mcq";
	ret = insert_resource(&iomem_resource, res_mcq);
	if (ret) {
		dev_err(hba->dev, "Failed to insert MCQ resource, err=%d\n",
			ret);
		return ret;
	}
	res->base = devm_ioremap_resource(hba->dev, res_mcq);
	if (IS_ERR(res->base)) {
		dev_err(hba->dev, "MCQ registers mapping failed, err=%d\n",
			(int)PTR_ERR(res->base));
		ret = PTR_ERR(res->base);
		goto ioremap_err;
	}
out:
	mcq_info->mcq_base = res->base;
	return 0;
ioremap_err:
	res->base = NULL;
	remove_resource(res_mcq);
	return ret;
}

static int ufs_xring_mcq_runtime_config(struct ufs_hba *hba)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct ufshcd_res_info *mem_res, *sqdao_res;
	struct ufshcd_mcq_opr_info_t *opr;
	int i;

	mem_res = &mcq_info->res[RES_UFS];
	sqdao_res = &mcq_info->res[RES_MCQ_SQD];
	if (!mem_res->base || !sqdao_res->base)
		return -EINVAL;
	/* per type size: 0x40, per queue size: 0x100 */
	for (i = 0; i < OPR_MAX; i++) {
		opr = &mcq_info->mcq_opr[i];
		opr->offset = sqdao_res->resource->start -
			      mem_res->resource->start + 0x40 * i;
		opr->stride = 0x100;
		opr->base = sqdao_res->base + 0x40 * i;
	}
	return 0;
}

static int ufs_xring_mcq_memory_alloc(struct ufs_hba *hba)
{
	struct ufs_hw_queue *hwq;
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	size_t utrdl_size, cqe_size;
	int i;
	/* utrd && cqe */
	for (i = 0; i < mcq_info->mcq_nr_hw_queue; i++) {
		hwq = &mcq_info->uhq[i];
		utrdl_size = sizeof(struct utp_transfer_req_desc) *
			     hwq->max_entries;
		hwq->sqe_base_addr = dmam_alloc_coherent(hba->dev, utrdl_size,
							 &hwq->sqe_dma_addr,
							 GFP_KERNEL);
		if (!hwq->sqe_dma_addr) {
			dev_err(hba->dev, "SQE allocation failed\n");
			return -ENOMEM;
		}
		cqe_size = sizeof(struct cq_entry) * hwq->max_entries;
		hwq->cqe_base_addr = dmam_alloc_coherent(hba->dev, cqe_size,
							 &hwq->cqe_dma_addr,
							 GFP_KERNEL);
		if (!hwq->cqe_dma_addr) {
			dev_err(hba->dev, "CQE allocation failed\n");
			return -ENOMEM;
		}
	}
	return 0;
}

static void __iomem *mcq_opr_base(struct ufs_hba *hba,
					 enum ufshcd_mcq_opr n, int i)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct ufshcd_mcq_opr_info_t *opr = &mcq_info->mcq_opr[n];

	return opr->base + opr->stride * i;
}

static void ufs_xring_mcq_make_queues_operational(struct ufs_hba *hba)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct ufs_hw_queue *hwq;
	u16 qsize;
	u32 i;

	for (i = 0; i < mcq_info->mcq_nr_hw_queue; i++) {
		hwq = &mcq_info->uhq[i];
		hwq->id = i;
		qsize = hwq->max_entries * MCQ_ENTRY_SIZE_IN_DWORD - 1;
		/* Submission Queue Lower Base Address */
		ufsmcq_writel(mcq_info, lower_32_bits(hwq->sqe_dma_addr),
			      MCQ_CFG_n(REG_SQLBA, i));
		/* Submission Queue Upper Base Address */
		ufsmcq_writel(mcq_info, upper_32_bits(hwq->sqe_dma_addr),
			      MCQ_CFG_n(REG_SQUBA, i));
		/* Submission Queue Doorbell Address Offset */
		ufsmcq_writel(mcq_info, MCQ_OPR_OFFSET_n(OPR_SQD, i),
			      MCQ_CFG_n(REG_SQDAO, i));
		/* Submission Queue Interrupt Status Address Offset */
		ufsmcq_writel(mcq_info, MCQ_OPR_OFFSET_n(OPR_SQIS, i),
			      MCQ_CFG_n(REG_SQISAO, i));
		/* Completion Queue Lower Base Address */
		ufsmcq_writel(mcq_info, lower_32_bits(hwq->cqe_dma_addr),
			      MCQ_CFG_n(REG_CQLBA, i));
		/* Completion Queue Upper Base Address */
		ufsmcq_writel(mcq_info, upper_32_bits(hwq->cqe_dma_addr),
			      MCQ_CFG_n(REG_CQUBA, i));
		/* Completion Queue Doorbell Address Offset */
		ufsmcq_writel(mcq_info, MCQ_OPR_OFFSET_n(OPR_CQD, i),
			      MCQ_CFG_n(REG_CQDAO, i));
		/* Completion Queue Interrupt Status Address Offset */
		ufsmcq_writel(mcq_info, MCQ_OPR_OFFSET_n(OPR_CQIS, i),
			      MCQ_CFG_n(REG_CQISAO, i));
		/* Save the base addresses for quicker access */
		hwq->mcq_sq_head = mcq_opr_base(hba, OPR_SQD, i) + REG_SQHP;
		hwq->mcq_sq_tail = mcq_opr_base(hba, OPR_SQD, i) + REG_SQTP;
		hwq->mcq_cq_head = mcq_opr_base(hba, OPR_CQD, i) + REG_CQHP;
		hwq->mcq_cq_tail = mcq_opr_base(hba, OPR_CQD, i) + REG_CQTP;
		/* Reinitializing is needed upon HC reset */
		hwq->sq_tail_slot = hwq->cq_tail_slot = hwq->cq_head_slot = 0;
		/* CQ y Interrupt Enable */
		writel(1, mcq_opr_base(hba, OPR_CQIS, i) + REG_CQIE);
		/* Completion Queue Enable|Size to Completion Queue Attribute */
		ufsmcq_writel(mcq_info, ((u32)1 << QUEUE_EN_OFFSET) | qsize,
			      MCQ_CFG_n(REG_CQATTR, i));
		/*
		 * Submission Qeueue Enable|Size|Completion Queue ID to
		 * Submission Queue Attribute
		 */
		ufsmcq_writel(mcq_info, ((u32)1 << QUEUE_EN_OFFSET) | qsize |
			      (i << QUEUE_ID_OFFSET),
			      MCQ_CFG_n(REG_SQATTR, i));
	}
}

/*
 * sq/cq depth = blk shared tags = device queue depth
 * should use sdb mode get device queue depth first
 */
static void ufs_xring_mcq_config_mac(struct ufs_hba *hba, u32 max_active_cmds)
{
	u32 val;

	val = ufshcd_readl(hba, REG_UFS_MCQ_CFG);
	val &= ~MCQ_CFG_MAC_MASK;
	val |= FIELD_PREP(MCQ_CFG_MAC_MASK, max_active_cmds);
	ufshcd_writel(hba, val, REG_UFS_MCQ_CFG);
}

static void ufs_xring_mcq_config(void *data, struct ufs_hba *hba, int *err)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct Scsi_Host *host = hba->host;
	int ret;

	ret = ufs_xring_mcq_config_resource(hba);
	if (ret)
		goto out;
	ret = ufs_xring_mcq_init_queue(hba);
	if (ret)
		goto out;
	ret = ufs_xring_mcq_memory_alloc(hba);
	if (ret)
		goto out;
	ret = ufs_xring_mcq_runtime_config(hba);
	if (ret)
		goto out;
	/* Reserve one depth to judge empty or full */
	host->can_queue = mcq_info->mcq_queue_depth - UFSHCD_MCQ_NUM_RESERVED;
	host->cmd_per_lun = mcq_info->mcq_queue_depth - UFSHCD_MCQ_NUM_RESERVED;
	host->nr_hw_queues = mcq_info->mcq_nr_hw_queue;
	host->host_tagset = 1;
	dev_err(hba->dev, "MCQ configured, nr_queues=%d, queue_depth=%d\n",
		 mcq_info->mcq_nr_hw_queue, mcq_info->mcq_queue_depth);
out:
	*err = ret;
}

static void ufs_xring_mcq_hba_capabilities(void *data, struct ufs_hba *hba,
					int *err)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	int max_q;

	hba->reserved_slot = mcq_info->mcq_queue_depth - 1;
	mcq_info->mcq_capabilities = ufshcd_readl(hba, REG_MCQCAP);
	max_q = FIELD_GET(MAX_Q, mcq_info->mcq_capabilities);
	if (mcq_info->mcq_nr_hw_queue > max_q + 1) {
		dev_err(hba->dev, "HCI maxq %d is less than %d\n",
				max_q, mcq_info->mcq_nr_hw_queue);
		*err = -EINVAL;
		return;
	}
}

static void ufs_xring_mcq_use_mcq_hooks(void *data, struct ufs_hba *hba,
					bool *use_mcq)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	*use_mcq = mcq_info->mcq_is_enabled;
}

static void ufs_xring_mcq_max_tag(void *data, struct ufs_hba *hba, int *max_tag)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	if (mcq_info->mcq_is_enabled)
		*max_tag = mcq_info->mcq_queue_depth;
	else
		*max_tag = hba->nutrs;
}

static void ufs_xring_mcq_set_sqid(void *data, struct ufs_hba *hba, int index,
					struct ufshcd_lrb *lrbp)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	if (!mcq_info->mcq_is_enabled)
		return;
	/* save hw queue index in lrbp */
	lrbp->android_vendor_data1 = index;
}

static void ufs_xring_mcq_send_command(void *data, struct ufs_hba *hba,
					unsigned int task_tag)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct ufshcd_lrb *lrbp = &hba->lrb[task_tag];
	int uhq_index = lrbp->android_vendor_data1;
	struct ufs_hw_queue *hwq = &mcq_info->uhq[uhq_index];
	int utrd_size = sizeof(struct utp_transfer_req_desc);
	unsigned long flags;

	lrbp->issue_time_stamp = ktime_get();
	lrbp->compl_time_stamp = ktime_set(0, 0);
	trace_android_vh_ufs_send_command(hba, lrbp);
	ufshcd_add_command_trace(hba, task_tag, UFS_CMD_SEND);
	ufshcd_clk_scaling_start_busy(hba);
	spin_lock_irqsave(&hba->outstanding_lock, flags);
	__set_bit(task_tag, &hba->outstanding_reqs);
	spin_unlock_irqrestore(&hba->outstanding_lock, flags);
	spin_lock(&hwq->sq_lock);
	memcpy(hwq->sqe_base_addr + (hwq->sq_tail_slot * utrd_size),
		       lrbp->utr_descriptor_ptr, utrd_size);
	ufs_xring_mcq_inc_sq_tail(hwq);
	spin_unlock(&hwq->sq_lock);
}

static u32 ufs_xring_mcq_read_cqis(struct ufs_hba *hba, int i)
{
	return readl(mcq_opr_base(hba, OPR_CQIS, i) + REG_CQIS);
}

static void ufs_xring_mcq_write_cqis(struct ufs_hba *hba, u32 val, int i)
{
	writel(val, mcq_opr_base(hba, OPR_CQIS, i) + REG_CQIS);
}

void ufs_xring_mcq_compl_one_cqe(struct ufs_hba *hba, bool retry_requests,
				int task_tag, struct ufs_hw_queue *hwq)
{
	struct ufshcd_lrb *lrbp;
	struct scsi_cmnd *cmd;
	int result;
	bool update_scaling = false;
	struct cq_entry *cqe = ufs_xring_mcq_cur_cqe(hwq);
	u32 mcq_cqe_ocs;

	lrbp = &hba->lrb[task_tag];
	lrbp->compl_time_stamp = ktime_get();
	cmd = lrbp->cmd;
	mcq_cqe_ocs = (le32_to_cpu(cqe->status)) & UTRD_OCS_MASK;
	lrbp->utr_descriptor_ptr->header.dword_2 = cpu_to_le32(mcq_cqe_ocs);
	if (cmd) {
		trace_android_vh_ufs_compl_command(hba, lrbp);
		ufshcd_add_command_trace(hba, task_tag, UFS_CMD_COMP);
		result = retry_requests ? DID_BUS_BUSY << 16 :
			ufshcd_transfer_rsp_status(hba, lrbp);
		scsi_dma_unmap(cmd);
		cmd->result = result;
		ufshcd_crypto_clear_prdt(hba, lrbp);
		/* Mark completed command as NULL in LRB */
		lrbp->cmd = NULL;
		/* Do not touch lrbp after scsi done */
		cmd->scsi_done(cmd);
		ufshcd_release(hba);
		update_scaling = true;
	} else if (lrbp->command_type == UTP_CMD_TYPE_DEV_MANAGE ||
		   lrbp->command_type == UTP_CMD_TYPE_UFS_STORAGE) {
		if (hba->dev_cmd.complete) {
			trace_android_vh_ufs_compl_command(hba, lrbp);
			ufshcd_add_command_trace(hba, task_tag, UFS_DEV_COMP);
			complete(hba->dev_cmd.complete);
			update_scaling = true;
		}
	}
	if (update_scaling)
		ufshcd_clk_scaling_update_busy(hba);
}

/*
 * Current MCQ specification doesn't provide a Task Tag or its equivalent in
 * the Completion Queue Entry. Find the Task Tag using an indirect method.
 */
static u32 ufs_xring_mcq_get_tag(struct ufs_hba *hba,
				     struct ufs_hw_queue *hwq,
				     struct cq_entry *cqe)
{
	u64 addr;
	/* Bits 63:7 UCD base address, 6:5 are reserved, 4:0 is SQ ID */
	addr = (le64_to_cpu(cqe->command_desc_base_addr) & CQE_UCD_BA) -
		hba->ucdl_dma_addr;
	return div_u64(addr, ufshcd_get_ucd_size(hba));
}

static void ufs_xring_mcq_process_cqe(struct ufs_hba *hba,
					struct ufs_hw_queue *hwq)
{
	unsigned long flags;
	struct cq_entry *cqe = ufs_xring_mcq_cur_cqe(hwq);
	u32 task_tag = ufs_xring_mcq_get_tag(hba, hwq, cqe);

	spin_lock_irqsave(&hba->outstanding_lock, flags);
	__clear_bit(task_tag, &hba->outstanding_reqs);
	spin_unlock_irqrestore(&hba->outstanding_lock, flags);
	ufs_xring_mcq_compl_one_cqe(hba, false, task_tag, hwq);
}

static void ufs_xring_mcq_poll_cqe(struct ufs_hba *hba,
					 struct ufs_hw_queue *hwq)
{
	unsigned long flags;

	spin_lock_irqsave(&hwq->cq_lock, flags);
	ufs_xring_mcq_update_cq_tail_slot(hwq);
	while (!ufs_xring_mcq_is_cq_empty(hwq)) {
		ufs_xring_mcq_process_cqe(hba, hwq);
		ufs_xring_mcq_inc_cq_head_slot(hwq);
	}
	ufs_xring_mcq_update_cq_head(hwq);
	spin_unlock_irqrestore(&hwq->cq_lock, flags);
}

static void ufs_xring_mcq_leagcy_handler(void *data, struct ufs_hba *hba,
					u32 intr_status, irqreturn_t *retval)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct ufs_hw_queue *hwq;
	int i;
	u32 events;

	if (!mcq_info->mcq_is_enabled)
		return;
	/* if esi is enabled, should excute esi routine */
	if (mcq_info->esi_is_enabled) {
		*retval |= IRQ_HANDLED;
		return;
	}
	if (intr_status & MCQ_CQ_EVENT_STATUS) {
		/* interrupt routine needs to scan all CQISy registers */
		for (i = 0; i < mcq_info->mcq_nr_hw_queue; i++) {
			hwq = &mcq_info->uhq[i];
			events = ufs_xring_mcq_read_cqis(hba, i);
			if (events)
				ufs_xring_mcq_write_cqis(hba, events, i);
			if (events & UFSHCD_MCQ_CQIS_TAIL_ENT_PUSH_STS)
				ufs_xring_mcq_poll_cqe(hba, hwq);
		}
	}
	*retval |= IRQ_HANDLED;
}

void ufs_xring_mcq_enable_intr(struct ufs_hba *hba, u32 intrs)
{
	u32 set = ufshcd_readl(hba, REG_INTERRUPT_ENABLE);

	if (hba->ufs_version == ufshci_version(1, 0)) {
		u32 rw;

		rw = set & INTERRUPT_MASK_RW_VER_10;
		set = rw | ((set ^ intrs) & intrs);
	} else {
		set |= intrs;
	}
	ufshcd_writel(hba, set, REG_INTERRUPT_ENABLE);
}

static void ufs_xring_mcq_make_hba_operational(void *data, struct ufs_hba *hba,
						int *err)
{
	u32 reg;

	ufs_xring_mcq_make_queues_operational(hba);
	ufs_xring_mcq_config_mac(hba, MAX_ACTIVE_CMD);
	/* Select MCQ mode */
	ufshcd_writel(hba, ufshcd_readl(hba, REG_UFS_MEM_CFG) | 0x1,
		      REG_UFS_MEM_CFG);
	ufshcd_writel(hba, lower_32_bits(hba->utmrdl_dma_addr),
			REG_UTP_TASK_REQ_LIST_BASE_L);
	ufshcd_writel(hba, upper_32_bits(hba->utmrdl_dma_addr),
			REG_UTP_TASK_REQ_LIST_BASE_H);
	/*
	 * Make sure base address and interrupt setup are updated before
	 * enabling the run/stop registers below.
	 */
	wmb();
	/*
	 * UCRDY, UTMRLDY and UTRLRDY bits must be 1
	 */
	reg = ufshcd_readl(hba, REG_CONTROLLER_STATUS);
	if ((reg & UFSHCD_STATUS_READY) == UFSHCD_STATUS_READY) {
		ufshcd_writel(hba, UTP_TASK_REQ_LIST_RUN_STOP_BIT,
				  REG_UTP_TASK_REQ_LIST_RUN_STOP);
	} else {
		dev_err(hba->dev,
			"Host controller not ready to process requests");
		*err = -EIO;
	}
}

static void ufs_xring_mcq_has_oustanding_reqs(void *data, struct ufs_hba *hba,
						bool *ret)
{
	*ret = hba->outstanding_reqs ? 1 : 0;
}

static void ufs_xring_mcq_get_outstanding_reqs(void *data, struct ufs_hba *hba,
				unsigned long **outstanding_reqs, int *nr_tag)
{
	*outstanding_reqs = &hba->outstanding_reqs;
}

static int ufs_xring_mcq_sq_stop(struct ufs_hba *hba, struct ufs_hw_queue *hwq)
{
	void __iomem *reg;
	u32 id = hwq->id;
	u32 val = 0;
	int err;

	writel(SQ_STOP, mcq_opr_base(hba, OPR_SQD, id) + REG_SQRTC);
	reg = mcq_opr_base(hba, OPR_SQD, id) + REG_SQRTS;
	err = read_poll_timeout(readl, val, val & SQ_STS, 20,
				MCQ_POLL_US, false, reg);
	if (err)
		dev_err(hba->dev, "%s: failed. hwq-id=%d, err=%d\n",
			__func__, id, err);
	return err;
}

static int ufs_xring_mcq_sq_start(struct ufs_hba *hba, struct ufs_hw_queue *hwq)
{
	void __iomem *reg;
	u32 id = hwq->id;
	u32 val = 0;
	int err;

	val |= SQ_STS;
	writel(SQ_START, mcq_opr_base(hba, OPR_SQD, id) + REG_SQRTC);
	reg = mcq_opr_base(hba, OPR_SQD, id) + REG_SQRTS;
	err = read_poll_timeout(readl, val, !(val & SQ_STS), 20,
				MCQ_POLL_US, false, reg);
	if (err)
		dev_err(hba->dev, "%s: failed. hwq-id=%d, err=%d\n",
			__func__, id, err);
	return err;
}

/* called by ufshcd_clear_cmd routine */
static void ufs_xring_mcq_clear_cmd(void *data, struct ufs_hba *hba,
					int task_tag, int *ret)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct ufshcd_lrb *lrbp = &hba->lrb[task_tag];
	u32 index = lrbp->android_vendor_data1;
	struct ufs_hw_queue *hwq = &mcq_info->uhq[index];
	void __iomem *reg, *opr_sqd_base;
	u32 nexus, id;
	u32 val = 0;
	int err = 0;

	id = hwq->id;
	mutex_lock(&hwq->sq_mutex);
	/* stop the SQ fetching before working on it */
	err = ufs_xring_mcq_sq_stop(hba, hwq);
	if (err)
		goto unlock;
	/* SQCTI = EXT_IID, IID, LUN, Task Tag */
	nexus = lrbp->lun << 8 | task_tag;
	opr_sqd_base = mcq_opr_base(hba, OPR_SQD, id);
	writel(nexus, opr_sqd_base + REG_SQCTI);
	/* SQRTCy.ICU = 1 */
	writel(SQ_ICU, opr_sqd_base + REG_SQRTC);
	/* Poll SQRTSy.CUS = 1. Return result from SQRTSy.RTC */
	reg = opr_sqd_base + REG_SQRTS;
	err = read_poll_timeout(readl, val, val & SQ_CUS, 20,
				MCQ_POLL_US, false, reg);
	if (err)
		dev_err(hba->dev, "%s: failed. hwq=%d, tag=%d err=%ld\n",
			__func__, id, task_tag,
			FIELD_GET(SQ_ICU_ERR_CODE_MASK, readl(reg)));
	if (ufs_xring_mcq_sq_start(hba, hwq))
		err = -ETIMEDOUT;
unlock:
	mutex_unlock(&hwq->sq_mutex);
	*ret = err;
}

static void ufs_xring_mcq_set_abort_skip(struct ufs_hba *hba,
					unsigned long bitmap)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct ufshcd_lrb *lrbp;
	int tag = 0;

	for_each_set_bit(tag, &bitmap, mcq_info->mcq_queue_depth) {
		lrbp = &hba->lrb[tag];
		lrbp->req_abort_skip = true;
	}
}

static int ufs_xring_mcq_try_to_abort_task(struct ufs_hba *hba, int tag)
{
	/* need bug fix2: ufshcd_try_to_abort_task should export symbol */
	return 0;
}

static void ufs_xring_mcq_abort(void *data, struct scsi_cmnd *cmd, int *ret)
{
	struct Scsi_Host *host = cmd->device->host;
	struct ufs_hba *hba = shost_priv(host);
	int tag = scsi_cmd_to_rq(cmd)->tag;
	struct ufshcd_lrb *lrbp = &hba->lrb[tag];
	unsigned long flags;
	int err = 0;

	ufshcd_hold(hba, false);
	/* If command is already aborted/completed, return FAILED. */
	if (!(test_bit(tag, &hba->outstanding_reqs))) {
		dev_err(hba->dev,
			"%s: cmd at tag %d already completed, outstanding=0x%lx\n",
			__func__, tag, hba->outstanding_reqs);
		goto release;
	}
	/* Print Transfer Request of aborted task */
	dev_info(hba->dev, "%s: Device abort task at tag %d\n", __func__, tag);
	/*
	 * Print detailed info about aborted request.
	 * As more than one request might get aborted at the same time,
	 * print full information only for the first aborted request in order
	 * to reduce repeated printouts. For other aborted requests only print
	 * basic details.
	 */
	scsi_print_command(cmd);
	if (!hba->req_abort_count) {
		ufshcd_update_evt_hist(hba, UFS_EVT_ABORT, tag);
		ufs_xring_mcq_print_evt_hist(hba);
		ufs_xring_mcq_print_host_state(hba);
		ufs_xring_mcq_print_pwr_info(hba);
		ufs_xring_mcq_print_trs_tag(hba, 1 << tag, true);
	} else {
		ufs_xring_mcq_print_trs_tag(hba, 1 << tag, false);
	}
	hba->req_abort_count++;
	/*
	 * Task abort to the device W-LUN is illegal. When this command
	 * will fail, due to spec violation, scsi err handling next step
	 * will be to send LU reset which, again, is a spec violation.
	 * To avoid these unnecessary/illegal steps, first we clean up
	 * the lrb taken by this cmd and re-set it in outstanding_reqs,
	 * then queue the eh_work and bail.
	 */
	if (lrbp->lun == UFS_UPIU_UFS_DEVICE_WLUN) {
		ufshcd_update_evt_hist(hba, UFS_EVT_ABORT, lrbp->lun);
		spin_lock_irqsave(host->host_lock, flags);
		hba->force_reset = true;
		ufs_xring_mcq_schedule_eh_work(hba);
		spin_unlock_irqrestore(host->host_lock, flags);
		goto release;
	}
	/* Skip task abort in case previous aborts failed and report failure */
	if (lrbp->req_abort_skip) {
		dev_err(hba->dev, "%s: skipping abort\n", __func__);
		ufs_xring_mcq_set_abort_skip(hba, hba->outstanding_reqs);
		goto release;
	}
	/*
	 * The command should not be "stuck" in SQ for a long time which
	 * resulted in command being aborted.
	 * So we skip sqe search here.
	 */
	err = ufs_xring_mcq_try_to_abort_task(hba, tag);
	if (err) {
		dev_err(hba->dev, "%s: failed with err %d\n", __func__, err);
		ufs_xring_mcq_set_abort_skip(hba, hba->outstanding_reqs);
		err = FAILED;
		goto release;
	}
	lrbp->cmd = NULL;
	err = SUCCESS;
release:
	/* Matches the ufshcd_hold() call at the start of this function. */
	ufshcd_release(hba);
	*ret = err;
}

void ufs_xring_mcq_get_dts_info(struct ufs_hba *hba)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct device_node *np = hba->dev->of_node;

	if (of_property_read_bool(np, "xring,ufs-mcq-enabled"))
		mcq_info->mcq_is_enabled = true;
	else
		mcq_info->mcq_is_enabled = false;

	mcq_info->esi_is_enabled = false;
	if (mcq_info->mcq_is_enabled) {
		if (of_property_read_bool(np, "xring,ufs-esi-enabled"))
			mcq_info->esi_is_enabled = true;
		else
			mcq_info->esi_is_enabled = false;

		if (of_property_read_u32(np, "xring,ufs-mcq-nr-hwq",
			&mcq_info->mcq_nr_hw_queue))
			mcq_info->mcq_nr_hw_queue = 8;

		if (of_property_read_u32(np, "xring,ufs-mcq-q-depth",
			&mcq_info->mcq_queue_depth))
			mcq_info->mcq_queue_depth = 32;
		dev_info(hba->dev, "%s, mcq hwq: %d, mcq q-depth: %d\n",
				__func__,
				mcq_info->mcq_nr_hw_queue,
				mcq_info->mcq_queue_depth);
	}
	dev_info(hba->dev, "MCQ enabled: %s\n", mcq_info->mcq_is_enabled ? "yes" : "no");
}

int ufs_xring_mcq_alloc_priv(struct ufs_hba *hba)
{
	struct ufs_xring_mcq_info *mcq_info;

	mcq_info = kzalloc(sizeof(struct ufs_xring_mcq_info), GFP_KERNEL);
	if (!mcq_info)
		return -ENOMEM;

	hba->android_vendor_data1 = (u64)mcq_info;
	return 0;
}

struct tracepoints_table {
	const char *name;
	void *func;
	struct tracepoint *tp;
	bool init;
};

static struct tracepoints_table mcq_interests[] = {
	{
		.name = "android_vh_ufs_use_mcq_hooks",
		.func = ufs_xring_mcq_use_mcq_hooks
	},
	{
		.name = "android_vh_ufs_mcq_max_tag",
		.func = ufs_xring_mcq_max_tag
	},
	{
		.name = "android_vh_ufs_mcq_set_sqid",
		.func = ufs_xring_mcq_set_sqid
	},
	{
		.name = "android_vh_ufs_mcq_handler",
		.func = ufs_xring_mcq_leagcy_handler
	},
	{
		.name = "android_vh_ufs_mcq_make_hba_operational",
		.func = ufs_xring_mcq_make_hba_operational
	},
	{
		.name = "android_vh_ufs_mcq_hba_capabilities",
		.func = ufs_xring_mcq_hba_capabilities
	},
	{
		.name = "android_vh_ufs_mcq_print_trs",
		.func = ufs_xring_mcq_print_trs
	},
	{
		.name = "android_vh_ufs_mcq_send_command",
		.func = ufs_xring_mcq_send_command
	},
	{
		.name = "android_vh_ufs_mcq_config",
		.func = ufs_xring_mcq_config
	},
	{
		.name = "android_vh_ufs_mcq_has_oustanding_reqs",
		.func = ufs_xring_mcq_has_oustanding_reqs
	},
	{
		.name = "android_vh_ufs_mcq_get_outstanding_reqs",
		.func = ufs_xring_mcq_get_outstanding_reqs
	},
	{
		.name = "android_vh_ufs_mcq_abort",
		.func = ufs_xring_mcq_abort
	},
	{
		.name = "android_vh_ufs_mcq_clear_cmd",
		.func = ufs_xring_mcq_clear_cmd
	},
};

#define FOR_EACH_INTEREST(i) \
	for ((i) = 0; (i) < sizeof(mcq_interests) / sizeof(struct tracepoints_table); \
	(i)++)

static void ufs_xring_mcq_lookup_tracepoints(struct tracepoint *tp,
					   void *ignore)
{
	int i;

	FOR_EACH_INTEREST(i) {
		if (strcmp(mcq_interests[i].name, tp->name) == 0)
			mcq_interests[i].tp = tp;
	}
}

void ufs_xring_mcq_uninstall_tracepoints(void)
{
	int i;

	FOR_EACH_INTEREST(i) {
		if (mcq_interests[i].init) {
			tracepoint_probe_unregister(mcq_interests[i].tp,
							mcq_interests[i].func,
							NULL);
		}
	}
}

int ufs_xring_mcq_install_tracepoints(struct ufs_hba *hba)
{
	int i;

	struct ufs_xring_mcq_info *mcq_info =
		(struct ufs_xring_mcq_info *)hba->android_vendor_data1;

	if (mcq_info->mcq_is_enabled == false)
		return 0;

	/* Install the tracepoints */
	for_each_kernel_tracepoint(ufs_xring_mcq_lookup_tracepoints, NULL);
	FOR_EACH_INTEREST(i) {
		if (mcq_interests[i].tp == NULL) {
			pr_info("Error: tracepoint %s not found\n",
					mcq_interests[i].name);
			continue;
		}
		tracepoint_probe_register(mcq_interests[i].tp,
					  mcq_interests[i].func,
					  NULL);
		mcq_interests[i].init = true;
	}
	return 0;
}

static void ufs_xring_write_msi_msg(struct msi_desc *desc, struct msi_msg *msg)
{
	/* hw config, sw do nothing here */
	return;
}

static unsigned int ufs_xring_irq_to_queue_id(unsigned int base,
						unsigned int irq)
{
	return ((irq - base) / ESI_IRQ_TYPE);
}

void ufs_xring_irq_set_affinity(struct ufs_hba *hba)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	u32 cpu, irq, _irq, queue_id, queue_map;
	int ret, i;
	struct blk_mq_tag_set *tag_set = &hba->host->tag_set;
	struct blk_mq_queue_map *qmap;

	if (!mcq_info->esi_is_enabled)
		return;

	for (queue_map = 0; queue_map < HCTX_MAX_TYPES; queue_map++) {
		qmap = &tag_set->map[queue_map];
		if (qmap->mq_map == NULL)
			continue;
		for_each_possible_cpu(cpu) {
			queue_id = qmap->mq_map[cpu];
			irq = queue_id * ESI_IRQ_TYPE + mcq_info->esi_base;
			for (i = 0; i < ESI_IRQ_TYPE; i++) {
				_irq = irq + i;
				ret = irq_set_affinity(_irq, cpumask_of(cpu));
				if (ret)
					dev_err(hba->dev, "ufs: irq_set_affinity irq %d on CPU %d failed\n",
						_irq, cpu);
			}
		}
	}
}

static irqreturn_t ufs_xring_esi_handler(int irq, void *__hba)
{
	struct ufs_hba *hba = __hba;
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	u32 id = ufs_xring_irq_to_queue_id(mcq_info->esi_base, irq);
	struct ufs_hw_queue *hwq = &mcq_info->uhq[id];

	ufs_xring_mcq_write_cqis(hba, 0x1, id);
	ufs_xring_mcq_poll_cqe(hba, hwq);
	return IRQ_HANDLED;
}

static void ufs_xring_enable_esi(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	unsigned int ufs_esi_reg[3];
	unsigned int ufs_esi_mask[4];
	int i;

	ufs_esi_reg[0] = UFS_ESI_MASK_2_OFFSET;
	ufs_esi_reg[1] = UFS_ESI_MASK_1_OFFSET;
	ufs_esi_reg[2] = UFS_ESI_MASK_0_OFFSET;
	ufs_esi_mask[0] = 0xffffffff;
	ufs_esi_mask[1] = 0xffffffff;
	ufs_esi_mask[2] = 0xffffffff;
	ufs_sctrl_rmwl(host, UFS_ESI_DEVICE_ID_MASK, (0xff << 16),
				UFS_ESI_DEVICE_ID_OFFSET);

	for (i = 0; i < ESI_NR_IRQS; i++) {
		if (i % ESI_IRQ_TYPE == CQ_TEPS_OFFSET)
			__clear_bit(i % 32, (unsigned long *)(&(ufs_esi_mask[i / 32])));
	}
	for (i = 0; i < 3; i++)
		ufs_sctrl_writel(host, ufs_esi_mask[i], ufs_esi_reg[i]);

	ufshcd_writel(hba, ufshcd_readl(hba, REG_UFS_MEM_CFG) | 0x2,
		      REG_UFS_MEM_CFG);
}

int ufs_xring_register_esi(struct ufs_hba *hba)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	struct msi_desc *desc = NULL;
	struct msi_desc *failed_desc = NULL;
	int ret = 0;

	if (!mcq_info->esi_is_enabled) {
		dev_notice(hba->dev, "%s: ESI mode is not enabled\n", __func__);
		return -1;
	}
	ret = platform_msi_domain_alloc_irqs(hba->dev, ESI_NR_IRQS,
					     ufs_xring_write_msi_msg);
	if (ret) {
		pr_err("%s alloc esi irq fail ret=%d\n", __func__, ret);
		mcq_info->esi_is_enabled = 0;
		return ret;
	}
	for_each_msi_entry(desc, hba->dev) {
		if (!desc->platform.msi_index)
			mcq_info->esi_base = desc->irq;
		ret = devm_request_irq(hba->dev, desc->irq,
				       ufs_xring_esi_handler,
				       IRQF_SHARED, "xring-mcq-esi", hba);
		if (ret) {
			dev_err(hba->dev, "%s: Fail to request IRQ for %d, err = %d\n",
				__func__, desc->irq, ret);
			failed_desc = desc;
			mcq_info->esi_is_enabled = 0;
			break;
		}
	}
	if (ret) {
		for_each_msi_entry(desc, hba->dev) {
			if (desc == failed_desc)
				break;
			devm_free_irq(hba->dev, desc->irq, hba);
		}
		mcq_info->esi_is_enabled = false;
		dev_warn(hba->dev, "Failed to request Platform MSI %d\n", ret);
	} else
		ufs_xring_enable_esi(hba);
	ufs_xring_irq_set_affinity(hba);
	return ret;
}

static void __maybe_unused ufs_xring_mcq_dump_register(struct ufs_hba *hba)
{
	struct ufs_xring_mcq_info *mcq_info = (struct ufs_xring_mcq_info *)
						hba->android_vendor_data1;
	u32 i;
	u32 offset;
	/* dump mcq sq/cq registers */
	for (i = 0; i < mcq_info->mcq_nr_hw_queue; i++) {
		offset = MCQ_QCFG_SIZE * i;
		dev_err(hba->dev, "Dump mcq related register, queue id: %d, offset: 0x%x\n",
						i, offset);
		ufs_xring_mcq_dump_regs(hba, offset, MCQ_QCFG_SIZE, "mcq_related_regs: ");
	}
	/* dump mcq operation&runtime registers */
	for (i = 0; i < mcq_info->mcq_nr_hw_queue; i++) {
		dev_err(hba->dev, "Dump mcq opr register, queue id: %d\n", i);
		dev_err(hba->dev, "OPR_SQD addr: 0x%x\n", mcq_opr_base(hba, OPR_SQD, i));
		ufs_xring_mcq_hex_dump("OPR_SQD", mcq_opr_base(hba, OPR_SQD, i), 0x14);
		dev_err(hba->dev, "OPR_SQIS addr: 0x%x\n", mcq_opr_base(hba, OPR_SQIS, i));
		ufs_xring_mcq_hex_dump("OPR_SQIS", mcq_opr_base(hba, OPR_SQIS, i), 0x14);
		dev_err(hba->dev, "OPR_CQD addr: 0x%x\n", mcq_opr_base(hba, OPR_CQD, i));
		ufs_xring_mcq_hex_dump("OPR_CQD", mcq_opr_base(hba, OPR_CQD, i), 0x14);
		dev_err(hba->dev, "OPR_CQIS addr: 0x%x\n", mcq_opr_base(hba, OPR_CQIS, i));
		ufs_xring_mcq_hex_dump("OPR_CQIS", mcq_opr_base(hba, OPR_CQIS, i), 0x14);
	}
}
