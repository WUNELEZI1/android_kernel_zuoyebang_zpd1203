// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/pm_runtime.h>
#include <linux/pm_wakeup.h>
#include <linux/sched.h>
#include <linux/sched/clock.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#include "mtk_bm.h"
#include "mtk_port.h"
#include "mtk_cldma.h"
#include "mtk_cldma_drv_t800.h"
#include "mtk_debug.h"
#include "mtk_debugfs.h"
#include "mtk_dev.h"
#include "mtk_fsm.h"
#include "mtk_reg.h"
#ifdef CONFIG_TX00_UT_CLDMA_DRV
#include "ut_cldma_drv_t800.h"
#endif

#define TAG		"CLDMA"

#define DMA_POOL_NAME_LEN	(64)
#define WAIT_QUEUE_STOP		(70)
#define WAIT_HWO_ROUND		(10)
#define WAIT_HWO_TIME		(5)
#define CLDMA_WS_NAME_LEN	(25)
#define	CLDMA_KEEP_WAKE_TIME_MS	(500)
/* VALID_RX_INTR_GAP_NS : If the interval between two interrupts exceeds 5ms, the average */
/* interrupt interval is recalculated. With seven interrupts every 5ms, there will only  */
/* be 1500 interrupts per second */
#define VALID_RX_INTR_GAP_NS	(5000000)
/* TIME_GAP_THRESHOLD_NS : Limit the number of interrupts to a maximum if 2000 per second */
#define TIME_GAP_THRESHOLD_NS	(500000)
/* WAIT_RX_INTR_TIME_US : Waiting time for an interrupt is approximately half of */
/* the maximum allowed interval between interrupts */
#define WAIT_RX_INTR_TIME_US	(300)
#define NUM_FOR_CAL_CNT		(3)
#define RX_FREQ_STAT_CNT	BIT(NUM_FOR_CAL_CNT)
#define AVG_TIME_GAP(avg, gap)	(((RX_FREQ_STAT_CNT - 1) * (avg) + (gap)) >> NUM_FOR_CAL_CNT)

#define MTK_REGSCNT_PER_LINE_TEST 4
#define MTK_BYTES_PER_LINE_TEST 16
#define MTK_REG_OFFSET_STR_LEN_TEST 7
#define MTK_REG_VAL_STR_LEN_TEST 9
#define MTK_LOG_BUFF_SIZE_TEST 256

static void MTK_REGS_DUMP_TEST(struct mtk_md_dev *mdev, enum mtk_debug_mask dbg_mask,
			       enum mtk_memlog_region_id rg_id, const char *msg,
			       unsigned long long addr, size_t len)
{
	int i = 0;
	unsigned long long base_addr = 0;
	/* Up multiples of MTK_REGSCNT_PER_LINE_TEST */
	int round_up_len = round_up(len, MTK_REGSCNT_PER_LINE_TEST);
	int b4_fix_num = round_up_len / MTK_REGSCNT_PER_LINE_TEST;
	int b16_fix_num = b4_fix_num / MTK_REGSCNT_PER_LINE_TEST;
	int b16_tail_num = b4_fix_num % MTK_REGSCNT_PER_LINE_TEST;
	unsigned char buf[MTK_LOG_BUFF_SIZE_TEST] = {0};
	if (len <= 0) {
		MTK_INFO(mdev, "Invalid len parameters!\n");
		return;
	}
	if (mdev == NULL) {
		return;
	}

	if (msg)
		MTK_INFO(mdev, "%s\n", msg);
	MTK_INFO(mdev, "===start address:0x%lx, len(32bit):0x%x===\n", addr, len);
	for (i = 0; i < b16_fix_num; i++) {
		base_addr = addr + i * MTK_BYTES_PER_LINE_TEST;
		MTK_INFO(mdev, "0x%04X: %08X %08X %08X %08X\n",
			  i * MTK_BYTES_PER_LINE_TEST,
			  mtk_hw_read32(mdev, base_addr + 0),
			  mtk_hw_read32(mdev, base_addr + 4),
			  mtk_hw_read32(mdev, base_addr + 8),
			  mtk_hw_read32(mdev, base_addr + 12));
	}
	if (b16_tail_num) {
		base_addr = addr + i * MTK_BYTES_PER_LINE_TEST;
		sprintf(buf, "0x%04X:", i * MTK_BYTES_PER_LINE_TEST);
		for (i = 0; i < b16_tail_num; i++)
			sprintf((buf + MTK_REG_OFFSET_STR_LEN_TEST) + i * MTK_REG_VAL_STR_LEN_TEST,
				" %08X", mtk_hw_read32(mdev, base_addr + i * 4));
		MTK_INFO(mdev, "%s\n", buf);
	}
}

bool mtk_cldma_irq_registed = false;
static bool cldma0_irq_registed = false;
static bool cldma1_irq_registed = false;

static void cldma_gpd_dump(struct cldma_hw *hw, int qno)
{
	int i;
	int *val;
	struct txq *txq = hw->txq[qno];
	struct rxq *rxq = hw->rxq[qno];
	struct tx_req *txreq;
	struct rx_req *rxreq;

	if (!txq || !rxq)
		return;

	/* TX GPD */
	MTK_INFO(hw->mdev,
		"CLDMA%d TXQ%d GPD:\n", hw->hif_id, qno);
	for (i = 0; i < txq->req_pool_size; i++) {
		txreq = txq->req_pool + i;
		val = (int *)txreq->gpd;
		MTK_INFO(hw->mdev,
			"%d: %08x %08x %08x %08x %08x %08x\n", i,
			*val, *(val + 1), *(val + 2), *(val + 3),
			*(val + 4), *(val + 5));
	}

	/* RX GPD */
	MTK_INFO(hw->mdev,
		"CLDMA%d RXQ%d GPD:\n", hw->hif_id, qno);
	for (i = 0; i < rxq->req_pool_size; i++) {
		rxreq = rxq->req_pool + i;
		val = (int *)rxreq->gpd;
		MTK_INFO(hw->mdev,
			"%d: %08x %08x %08x %08x %08x %08x\n", i,
			*val, *(val + 1), *(val + 2), *(val + 3),
			*(val + 4), *(val + 5));
	}
}

static void cldma_hw_dump(struct cldma_hw *hw)
{
	int base = hw->base_addr;
	struct mtk_md_dev *mdev = hw->mdev;

	MTK_INFO(mdev,
		"CLDMA%d Left Side\n", hw->hif_id);
	MTK_REGS_DUMP_TEST(mdev, MTK_DBG_CLDMA, MTK_MEMLOG_RG_9, NULL, base, 0x200);
	MTK_REGS_DUMP_TEST(mdev, MTK_DBG_CLDMA, MTK_MEMLOG_RG_9,
		      NULL, base + REG_CLDMA_SO_START_ADDRL_0, 0x190);
	MTK_REGS_DUMP_TEST(mdev, MTK_DBG_CLDMA, MTK_MEMLOG_RG_9,
		      NULL, base + REG_CLDMA_L2TISAR0, 0x208);

	MTK_INFO(mdev,
		"\nCLDMA%d Right Side\n", hw->hif_id);
	MTK_REGS_DUMP_TEST(mdev, MTK_DBG_CLDMA, MTK_MEMLOG_RG_9, NULL, base + 0x1000, 0x200);
	MTK_REGS_DUMP_TEST(mdev, MTK_DBG_CLDMA, MTK_MEMLOG_RG_9,
		      NULL, base + REG_CLDMA_SO_START_ADDRL_0 + 0x1000, 0x190);
	MTK_REGS_DUMP_TEST(mdev, MTK_DBG_CLDMA, MTK_MEMLOG_RG_9,
		      NULL, base + REG_CLDMA_L2TISAR0 + 0x1000, 0x208);
}

static ssize_t cldma_dbg_write(void *data, const char *buf, ssize_t cnt)
{
	int i;
	int err;
	int val;
	struct cldma_hw *hw = data;

	err = kstrtoint(buf, 10, &val);
	if (err)
		goto err_exit;

	switch (val) {
	case 0 ... 7:
		cldma_gpd_dump(hw, val);
		break;
	default:
		for (i = 0; i < HW_QUEUE_NUM; i++) {
			cldma_gpd_dump(hw, i);
		}
		break;
	}

	pm_runtime_get_sync(hw->mdev->dev);
	mtk_pm_ds_lock(hw->mdev, MTK_USER_CTRL);
	err = mtk_pm_ds_wait_complete(hw->mdev, MTK_USER_CTRL);
	if (unlikely(err)) {
		MTK_ERR(hw->mdev, "Failed to lock ds:%d\n", err);
		goto err_ds_lock;
	}

	MTK_DBG(hw->mdev, MTK_DBG_CLDMA, MTK_MEMLOG_RG_9, "PCI Dump\n", hw->hif_id);
	mtk_hw_dbg_dump(hw->mdev);

	cldma_hw_dump(hw);

err_ds_lock:
	mtk_pm_ds_unlock(hw->mdev, MTK_USER_CTRL, false);
	pm_runtime_put_sync(hw->mdev->dev);
	if (err == -EIO)
		mtk_except_report_evt(hw->mdev, EXCEPT_LINK_ERR);

err_exit:
	return cnt;
}

MTK_DBGFS(cldma_dbg, NULL, cldma_dbg_write);

static inline void cldma_dbgfs_init(struct cldma_hw *hw)
{
#define CLDMA_DBGFS_NAME_LEN	32
	char name[CLDMA_DBGFS_NAME_LEN] = {0};

	snprintf(name, CLDMA_DBGFS_NAME_LEN, "cldma%d", hw->hif_id);
	hw->dentry = mtk_dbgfs_create_dir(mtk_get_dev_dentry(hw->mdev), name);
	if (!hw->dentry)
		return;

	mtk_dbgfs_create_file(hw->dentry, &mtk_dbgfs_cldma_dbg, hw);
}

static inline void cldma_dbgfs_exit(struct cldma_hw *hw)
{
	mtk_dbgfs_remove(hw->dentry);
}

static void mtk_cldma_setup_start_addr(struct mtk_md_dev *mdev, int base,
				       enum mtk_tx_rx dir, int qno, dma_addr_t addr)
{
	unsigned int addr_l;
	unsigned int addr_h;

	if (dir == DIR_TX) {
		addr_l = base + REG_CLDMA_UL_START_ADDRL_0 + qno * HW_QUEUE_NUM;
		addr_h = base + REG_CLDMA_UL_START_ADDRH_0 + qno * HW_QUEUE_NUM;
	} else {
		addr_l = base + REG_CLDMA_SO_START_ADDRL_0 + qno * HW_QUEUE_NUM;
		addr_h = base + REG_CLDMA_SO_START_ADDRH_0 + qno * HW_QUEUE_NUM;
	}

	mtk_hw_write32(mdev, addr_l, (u32)addr);
	mtk_hw_write32(mdev, addr_h, (u32)((u64)addr >> 32));
}

static void mtk_cldma_mask_intr(struct mtk_md_dev *mdev, int base,
				enum mtk_tx_rx dir, int qno, enum mtk_intr_type type)
{
	u32 addr;
	u32 val;

	if (dir == DIR_TX)
		addr = base + REG_CLDMA_L2TIMSR0;
	else
		addr = base + REG_CLDMA_L2RIMSR0;

	if (qno == ALLQ)
		val = qno << type;
	else
		val = BIT(qno) << type;

	mtk_hw_write32(mdev, addr, val);
}

static void mtk_cldma_unmask_intr(struct mtk_md_dev *mdev, int base,
				  enum mtk_tx_rx dir, int qno, enum mtk_intr_type type)
{
	u32 addr;
	u32 val;

	if (dir == DIR_TX)
		addr = base + REG_CLDMA_L2TIMCR0;
	else
		addr = base + REG_CLDMA_L2RIMCR0;

	if (qno == ALLQ)
		val = qno << type;
	else
		val = BIT(qno) << type;

	mtk_hw_write32(mdev, addr, val);
}

static void mtk_cldma_clr_intr_status(struct mtk_md_dev *mdev, int base,
				      int dir, int qno, enum mtk_intr_type type)
{
	u32 addr;
	u32 val;

	if (type == QUEUE_ERROR) {
		if (dir == DIR_TX) {
			val = mtk_hw_read32(mdev, base + REG_CLDMA_L3TISAR0);
			mtk_hw_write32(mdev, base + REG_CLDMA_L3TISAR0, val);
			val = mtk_hw_read32(mdev, base + REG_CLDMA_L3TISAR1);
			mtk_hw_write32(mdev, base + REG_CLDMA_L3TISAR1, val);
		} else {
			val = mtk_hw_read32(mdev, base + REG_CLDMA_L3RISAR0);
			mtk_hw_write32(mdev, base + REG_CLDMA_L3RISAR0, val);
			val = mtk_hw_read32(mdev, base + REG_CLDMA_L3RISAR1);
			mtk_hw_write32(mdev, base + REG_CLDMA_L3RISAR1, val);
		}
	}

	if (dir == DIR_TX)
		addr = base + REG_CLDMA_L2TISAR0;
	else
		addr = base + REG_CLDMA_L2RISAR0;

	if (qno == ALLQ)
		val = qno << type;
	else
		val = BIT(qno) << type;

	mtk_hw_write32(mdev, addr, val);
	val = mtk_hw_read32(mdev, addr);
}

static u32 mtk_cldma_check_intr_status(struct mtk_md_dev *mdev, int base,
				       int dir, int qno, enum mtk_intr_type type)
{
	u32 addr;
	u32 val;
	u32 sta;

	if (dir == DIR_TX)
		addr = base + REG_CLDMA_L2TISAR0;
	else
		addr = base + REG_CLDMA_L2RISAR0;

	val = mtk_hw_read32(mdev, addr);
	if (val == LINK_ERROR_VAL)
		sta = val;
	else if (qno == ALLQ)
		sta = (val >> type) & 0xFF;
	else
		sta = (val >> type) & BIT(qno);
	return sta;
}

static void mtk_cldma_start_queue(struct mtk_md_dev *mdev, int base, enum mtk_tx_rx dir, int qno)
{
	u32 val = BIT(qno);
	u32 addr;

	if (dir == DIR_TX)
		addr = base + REG_CLDMA_UL_START_CMD;
	else
		addr = base + REG_CLDMA_SO_START_CMD;

	mtk_hw_write32(mdev, addr, val);
}

static void mtk_cldma_resume_queue(struct mtk_md_dev *mdev, int base, enum mtk_tx_rx dir, int qno)
{
	u32 val = BIT(qno);
	u32 addr;

	if (dir == DIR_TX)
		addr = base + REG_CLDMA_UL_RESUME_CMD;
	else
		addr = base + REG_CLDMA_SO_RESUME_CMD;

	mtk_hw_write32(mdev, addr, val);
}

static u32 mtk_cldma_queue_status(struct cldma_hw *hw, enum mtk_tx_rx dir, int qno)
{
	u32 addr;
	u32 val;

	if (dir == DIR_TX)
		addr = hw->base_addr + REG_CLDMA_UL_STATUS;
	else
		addr = hw->base_addr + REG_CLDMA_SO_STATUS;

	val = mtk_hw_read32(hw->mdev, addr);

	if (qno == ALLQ || val == LINK_ERROR_VAL)
		return val;
	else
		return val & BIT(qno);
}

static void mtk_cldma_stop_queue(struct cldma_hw *hw, enum mtk_tx_rx dir, int qno)
{
	u32 val = (qno == ALLQ) ? qno : BIT(qno);
	unsigned int active;
	int cnt = 0;
	u32 addr;

	if (dir == DIR_TX)
		addr = hw->base_addr + REG_CLDMA_UL_STOP_CMD;
	else
		addr = hw->base_addr + REG_CLDMA_SO_STOP_CMD;

	mtk_hw_write32(hw->mdev, addr, val);

	do {
		active = mtk_cldma_queue_status(hw, dir, qno);
		if (active == LINK_ERROR_VAL || !active)
			break;
		usleep_range(WAIT_QUEUE_STOP, 2 * WAIT_QUEUE_STOP);
	} while (++cnt < 10);
	if (active == LINK_ERROR_VAL)
		mtk_except_report_evt(hw->mdev, EXCEPT_LINK_ERR);
}

static void mtk_cldma_clear_ip_busy(struct mtk_md_dev *mdev, int base)
{
	mtk_hw_write32(mdev, base + REG_CLDMA_IP_BUSY, 0x01);
}

static void mtk_cldma_hw_init(struct mtk_md_dev *mdev, int base)
{
	/* set CLDMA to 64 bit mode GPD */
	u32 val = mtk_hw_read32(mdev, base + REG_CLDMA_UL_CFG);

	val = (val & (~(0x7 << 5))) | ((0x4) << 5);
	mtk_hw_write32(mdev, base + REG_CLDMA_UL_CFG, val);

	val = mtk_hw_read32(mdev, base + REG_CLDMA_SO_CFG);
	val = (val & (~(0x7 << 10))) | ((0x4) << 10) | (1 << 2);
	mtk_hw_write32(mdev, base + REG_CLDMA_SO_CFG, val);

	/* rx_work2reg_mask */
	mtk_hw_write32(mdev, base + REG_CLDMA_RX_WORK_TO_REG_MASK_SET, ALLQ);

	/* to_pcie */
	mtk_hw_write32(mdev, base + REG_CLDMA_IP_BUSY_TO_PCIE_MASK_SET, ALLQ << 16);
	mtk_hw_write32(mdev, base + REG_CLDMA_IP_BUSY_TO_PCIE_MASK_CLR, ALLQ << 24);

	/* enable interrupt to PCIe */
	mtk_hw_write32(mdev, base + REG_CLDMA_INT_EAP_USIP_MASK, 0);

	/* disable illegal memory check */
	mtk_hw_write32(mdev, base + REG_CLDMA_UL_DUMMY_0, 1);
	mtk_hw_write32(mdev, base + REG_CLDMA_SO_DUMMY_0, 1);
}

static void mtk_cldma_tx_done_work(struct work_struct *work)
{
	struct txq *txq = container_of(work, struct txq, tx_done_work);
	if (!txq || !txq->hw) {//add by xiaomi
		extern void t800_recovery_start(int value1, int value2);
		t800_recovery_start(2, 0);
		return;
	}
	struct mtk_md_dev *mdev = txq->hw->mdev;
	struct tx_req *req;
	unsigned int state;
	struct trb *trb;
	int i;

	pm_runtime_get(mdev->dev);
again:
	for (i = 0; i < txq->req_pool_size; i++) {
		req = txq->req_pool + txq->free_idx;

		rmb();

		if (!req || !req->data_vm_addr || (req->gpd->tx_gpd.gpd_flags & CLDMA_GPD_FLAG_HWO))
			break;

		if (!req->skb) {
			MTK_ERR(mdev, "No skb for CLDMA%d txq %d", txq->hw->hif_id, txq->txqno);
			txq->hw->cd->hw_ops->dump(txq->hw);
			MTK_BUG(mdev);
		}

		dma_unmap_single(mdev->dev, req->data_dma_addr, req->data_len, DMA_TO_DEVICE);

		trb = (struct trb *)req->skb->cb;
		trb->status = 0;
		trb->trb_complete(req->skb);

		req->data_vm_addr = NULL;
		req->data_dma_addr = 0;
		req->data_len = 0;
		req->skb = NULL;

		txq->free_idx = (txq->free_idx + 1) % txq->req_pool_size;
		mutex_lock(&txq->lock);
		txq->req_budget++;
		mutex_unlock(&txq->lock);
	}
	MTK_DBG(mdev, MTK_DBG_CLDMA, MTK_MEMLOG_RG_10, "CLDMA%d txq%d wr:%d free:%d cnt:%d\n",
		txq->hw->hif_id, txq->txqno, txq->wr_idx, txq->free_idx, i);
	state = mtk_cldma_check_intr_status(mdev, txq->hw->base_addr,
					    DIR_TX, txq->txqno, QUEUE_XFER_DONE);
	if (state) {
		if (unlikely(state == LINK_ERROR_VAL)) {
			pm_runtime_put(mdev->dev);
			mtk_except_report_evt(mdev, EXCEPT_LINK_ERR);
			return;
		}

		mtk_cldma_clr_intr_status(mdev, txq->hw->base_addr, DIR_TX,
					  txq->txqno, QUEUE_XFER_DONE);

		if (need_resched())
			cond_resched();

		goto again;
	}

	mtk_cldma_unmask_intr(mdev, txq->hw->base_addr, DIR_TX, txq->txqno, QUEUE_XFER_DONE);
	mtk_pm_ds_unlock(mdev, MTK_USER_CTRL, false);
	pm_runtime_put(mdev->dev);
}

static u64 mtk_cldma_get_curr_addr(struct rxq *rxq)
{
	u32 curr_addr_h, curr_addr_l;
	struct mtk_md_dev *mdev;
	struct cldma_hw *hw;
	u64 curr_addr = 0;
	u64 addr;

	hw = rxq->hw;
	mdev = hw->mdev;

	addr = hw->base_addr + REG_CLDMA_SO_CUR_ADDRH_0 +
		(u64)rxq->rxqno * HW_QUEUE_NUM;
	curr_addr_h = mtk_hw_read32(mdev, addr);
	addr = hw->base_addr + REG_CLDMA_SO_CUR_ADDRL_0 +
		(u64)rxq->rxqno * HW_QUEUE_NUM;
	curr_addr_l = mtk_hw_read32(mdev, addr);
	curr_addr = ((u64)curr_addr_h << 32) | curr_addr_l;
	if (curr_addr_h == LINK_ERROR_VAL || curr_addr_l == LINK_ERROR_VAL || curr_addr == 0) {
		curr_addr = 0;
		mtk_except_report_evt(mdev, EXCEPT_LINK_ERR);
	}
	return curr_addr;
}

static void mtk_cldma_rx_done_work(struct work_struct *work)
{
	struct rxq *rxq = container_of(work, struct rxq, rx_done_work);
	struct mtk_bm_pool *bm_pool;
	struct rx_req *req = NULL;
	struct mtk_md_dev *mdev;
	bool has_waited = false;
	bool link_err = false;
	struct cldma_hw *hw;
	u64 curr_addr = 0;
	bool try_again;
	int i, j, err;
	int ret = 0;
	u32 state;

	hw = rxq->hw;
	mdev = hw->mdev;

	if (rxq->vq->rx_mtu > VQ_MTU_3_5K)
		bm_pool = rxq->hw->cd->trans->ctrl_blk->bm_pool_63K;
	else
		bm_pool = rxq->hw->cd->trans->ctrl_blk->bm_pool;

	pm_runtime_get(mdev->dev);
	do {
		try_again = false;

		for (i = 0; i < rxq->req_pool_size; i++) {
			req = rxq->req_pool + rxq->free_idx;
			if ((req->gpd->rx_gpd.gpd_flags & CLDMA_GPD_FLAG_HWO)) {
				curr_addr = mtk_cldma_get_curr_addr(rxq);
				if (!curr_addr) {
					link_err = true;
					MTK_WARN(mdev, "curr addr is 0, CLDMA%d rxq%d\n",
						 rxq->hw->hif_id, rxq->rxqno);
					goto exit;
				}
				if (req->gpd_dma_addr == curr_addr)
					break;
				for (j = 0; j < WAIT_HWO_ROUND; j++) {
					udelay(WAIT_HWO_TIME);
					if (!(req->gpd->rx_gpd.gpd_flags &
					      CLDMA_GPD_FLAG_HWO))
						break;
				}
				if (j == WAIT_HWO_ROUND) {
					MTK_ERR(mdev, "Failed to check HWO=0\n");
					break;
				}
			}
			if (req->data_dma_addr) {
				dma_unmap_single(mdev->dev, req->data_dma_addr, req->mtu,
						 DMA_FROM_DEVICE);
				req->data_dma_addr = 0x0;
			}

			ret = rxq->rx_done(req->skb, le16_to_cpu(req->gpd->rx_gpd.data_recv_len),
					   rxq->arg);
			if (ret == -EPIPE) {
				MTK_INFO(mdev, "Stop to dispatch CLDMA%d rxq%d\n",
					 rxq->hw->hif_id, rxq->rxqno);
				break;
			}

			req->skb = mtk_bm_alloc(bm_pool);
			if (!req->skb) {
				MTK_WARN(mdev, "Failed to alloc SKB\n");
				break;
			}

			req->data_dma_addr = dma_map_single(mdev->dev,
							    req->skb->data,
							    req->mtu,
							    DMA_FROM_DEVICE);
			err = dma_mapping_error(mdev->dev, req->data_dma_addr);
			if (unlikely(err)) {
				mtk_bm_free(bm_pool, req->skb);
				break;
			}

			req->gpd->rx_gpd.data_recv_len = 0;
			req->gpd->rx_gpd.data_buff_ptr_h =
				cpu_to_le32((u64)req->data_dma_addr >> 32);
			req->gpd->rx_gpd.data_buff_ptr_l = cpu_to_le32(req->data_dma_addr);

			wmb(); /* ensure addr set done before HWO setup done  */

			req->gpd->rx_gpd.gpd_flags = CLDMA_GPD_FLAG_IOC | CLDMA_GPD_FLAG_HWO;
			rxq->free_idx = (rxq->free_idx + 1) % rxq->req_pool_size;
		}

		MTK_DBG(mdev, MTK_DBG_CLDMA, MTK_MEMLOG_RG_10, "CLDMA%d rxq%d free:%d cnt:%d\n",
			rxq->hw->hif_id, rxq->rxqno, rxq->free_idx, i);

		mtk_cldma_resume_queue(mdev, rxq->hw->base_addr, DIR_RX, rxq->rxqno);

		state = mtk_cldma_check_intr_status(mdev, rxq->hw->base_addr,
						    DIR_RX, rxq->rxqno, QUEUE_XFER_DONE);

		if (state) {
			if (unlikely(state == LINK_ERROR_VAL)) {
				mtk_except_report_evt(mdev, EXCEPT_LINK_ERR);
				link_err = true;
				goto exit;
			}

			mtk_cldma_clr_intr_status(mdev, rxq->hw->base_addr, DIR_RX,
						  rxq->rxqno, QUEUE_XFER_DONE);
			try_again = true;
		} else if (rxq->freq_stat.stat_cnt >= RX_FREQ_STAT_CNT &&
			   rxq->freq_stat.avg_time_gap < TIME_GAP_THRESHOLD_NS) {
			if (!has_waited)
				usleep_range(WAIT_RX_INTR_TIME_US, 2 * WAIT_RX_INTR_TIME_US);
			try_again = has_waited ? false : true;
			has_waited = true;
		}

		if (need_resched())
			cond_resched();
	} while (try_again);

exit:
	__pm_wakeup_event(rxq->ws, CLDMA_KEEP_WAKE_TIME_MS);
	if (!link_err) {
		mtk_cldma_clear_ip_busy(mdev, rxq->hw->base_addr);
		mtk_cldma_unmask_intr(mdev, rxq->hw->base_addr, DIR_RX,
				      rxq->rxqno, QUEUE_XFER_DONE);
	}
	mtk_pm_ds_unlock(mdev, MTK_USER_CTRL, false);
	pm_runtime_put(mdev->dev);
}

static void mtk_cldma_rx_calculate_freq(struct rx_freq_stat *freq_stat)
{
	u64 curr_time;
	u64 time_gap;

	curr_time = local_clock();
	time_gap = curr_time - freq_stat->last_isr_time;
	freq_stat->last_isr_time = curr_time;
	if (time_gap > VALID_RX_INTR_GAP_NS) {
		freq_stat->avg_time_gap = 0;
		freq_stat->stat_cnt = 0;
		return;
	}

	if (freq_stat->stat_cnt >= RX_FREQ_STAT_CNT) {
		freq_stat->avg_time_gap = AVG_TIME_GAP(freq_stat->avg_time_gap, time_gap);
	} else {
		freq_stat->avg_time_gap += time_gap;
		freq_stat->stat_cnt++;
		if (freq_stat->stat_cnt == RX_FREQ_STAT_CNT)
			freq_stat->avg_time_gap >>= NUM_FOR_CAL_CNT;
	}
}

static bool mtk_cldma_check_rxq_full(struct cldma_hw *hw, int que_num)
{
	struct rx_req *req;
	struct rxq *rxq;
	int i;

	rxq = hw->rxq[que_num];
	if (!rxq) {
		return false;
	}

	for (i = 0; i < rxq->req_pool_size; i++) {
		req = rxq->req_pool + i;
		if (req && req->gpd && (req->gpd->rx_gpd.gpd_flags & CLDMA_GPD_FLAG_HWO)) {
			return true;
		}
	}
	return false;
}

static int mtk_cldma_isr(int irq_id, void *param)
{
	u32 txq_xfer_done, rxq_xfer_done;
	struct cldma_hw *hw = param;
	u32 tx_mask, rx_mask;
	u32 txq_err, rxq_err;
	u32 tx_sta, rx_sta;
	struct txq *txq;
	struct rxq *rxq;
	u32 rx_qe;
	int i;

	tx_sta = mtk_hw_read32(hw->mdev, hw->base_addr + REG_CLDMA_L2TISAR0);
	tx_mask = mtk_hw_read32(hw->mdev, hw->base_addr + REG_CLDMA_L2TIMR0);
	rx_sta = mtk_hw_read32(hw->mdev, hw->base_addr + REG_CLDMA_L2RISAR0);
	rx_mask = mtk_hw_read32(hw->mdev, hw->base_addr + REG_CLDMA_L2RIMR0);

	tx_sta = tx_sta & (~tx_mask);
	rx_sta = rx_sta & (~rx_mask);

	if (tx_sta) {
		txq_err = (tx_sta >> QUEUE_ERROR) & 0xFF;
		if (txq_err) {
			mtk_cldma_clr_intr_status(hw->mdev, hw->base_addr,
						  DIR_TX, ALLQ, QUEUE_ERROR);
			mtk_hw_write32(hw->mdev, hw->base_addr + REG_CLDMA_L2TIMCR0,
				       (txq_err << QUEUE_ERROR));
		}
		/* TX mask */
		mtk_hw_write32(hw->mdev, hw->base_addr + REG_CLDMA_L2TIMSR0, tx_sta);
		/* TX clear */
		mtk_hw_write32(hw->mdev, hw->base_addr + REG_CLDMA_L2TISAR0, tx_sta);

		txq_xfer_done = (tx_sta >> QUEUE_XFER_DONE) & 0xFF;
		if (txq_xfer_done) {
			for (i = 0; i < HW_QUEUE_NUM; i++) {
				if (!(txq_xfer_done & (1 << i)))
					continue;
				txq = hw->txq[i];
				if (txq) {
					if (queue_work(hw->wq, &txq->tx_done_work))
						mtk_pm_ds_try_lock(hw->mdev, MTK_USER_CTRL);
				}
			}
		}
	}

	if (rx_sta) {
		rxq_err = (rx_sta >> QUEUE_ERROR) & 0xFF;
		if (rxq_err) {
			mtk_cldma_clr_intr_status(hw->mdev, hw->base_addr,
						  DIR_RX, ALLQ, QUEUE_ERROR);
			mtk_hw_write32(hw->mdev, hw->base_addr + REG_CLDMA_L2RIMCR0,
				       (rxq_err << QUEUE_ERROR));
		}

		/* RX mask */
		mtk_hw_write32(hw->mdev, hw->base_addr + REG_CLDMA_L2RIMSR0, rx_sta);
		rx_qe = (rx_sta >> QUEUE_EMPTY) & 0xFF;
		if (rx_qe) {
			for (i = 0; i < HW_QUEUE_NUM; i++) {
				if (rx_qe & (1 << i) && mtk_cldma_check_rxq_full(hw, i))
					mtk_cldma_resume_queue(hw->mdev, hw->base_addr,
							       DIR_RX, i);
			}
			mtk_hw_write32(hw->mdev, hw->base_addr + REG_CLDMA_L2RIMCR0,
				       (rx_qe << QUEUE_EMPTY));
		}
		/* RX clear */
		mtk_hw_write32(hw->mdev, hw->base_addr + REG_CLDMA_L2RISAR0, rx_sta);

		rxq_xfer_done = (rx_sta >> QUEUE_XFER_DONE) & 0xFF;
		if (rxq_xfer_done) {
			for (i = 0; i < HW_QUEUE_NUM; i++) {
				if (!(rxq_xfer_done & (1 << i)))
					continue;
				rxq = hw->rxq[i];
				if (rxq) {
					mtk_cldma_rx_calculate_freq(&rxq->freq_stat);
					if (queue_work(hw->wq, &rxq->rx_done_work)) {
						__pm_stay_awake(rxq->ws);
						mtk_pm_ds_try_lock(hw->mdev, MTK_USER_CTRL);
					}
				}
			}
		}
	}

	mtk_hw_clear_irq(hw->mdev, hw->pci_ext_irq_id);
	mtk_hw_unmask_irq(hw->mdev, hw->pci_ext_irq_id);

	return IRQ_HANDLED;
}

static int mtk_cldma_hw_init_t800(struct cldma_dev *cd, int hif_id)
{
	char pool_name[DMA_POOL_NAME_LEN];
	struct cldma_hw *hw;
	unsigned int flag;

	if (!cd || hif_id == NR_CLDMA)
		return -EINVAL;

	if (cd->cldma_hw[hif_id])
		return 0;

	hw = devm_kzalloc(cd->trans->mdev->dev, sizeof(*hw), GFP_KERNEL);
	if (!hw)
		return -ENOMEM;

	hw->cd = cd;
	hw->mdev = cd->trans->mdev;
	hw->hif_id = ((CLDMA_CLASS_ID) << 8) + hif_id;
	snprintf(pool_name, DMA_POOL_NAME_LEN, "cldma%d_pool_%s", hw->hif_id, hw->mdev->dev_str);
	hw->dma_pool = dma_pool_create(pool_name, hw->mdev->dev, sizeof(union gpd), 64, 0);
	if (!hw->dma_pool)
		goto err_exit;

	switch (hif_id) {
	case CLDMA0:
		hw->pci_ext_irq_id = mtk_hw_get_irq_id(hw->mdev, MTK_IRQ_SRC_CLDMA0);
		hw->base_addr = CLDMA0_BASE_ADDR;
		break;
	case CLDMA1:
		hw->pci_ext_irq_id = mtk_hw_get_irq_id(hw->mdev, MTK_IRQ_SRC_CLDMA1);
		hw->base_addr = CLDMA1_BASE_ADDR;
		break;
	default:
		goto free_mem;
	}

	flag = WQ_UNBOUND | WQ_MEM_RECLAIM | WQ_HIGHPRI;
	hw->wq = alloc_workqueue("cldma%d_workq_%s", flag, 0, hif_id, hw->mdev->dev_str);

	cldma_dbgfs_init(hw);
	mtk_cldma_hw_init(hw->mdev, hw->base_addr);

	/* mask/clear PCI CLDMA L1 interrupt */
	mtk_hw_mask_irq(hw->mdev, hw->pci_ext_irq_id);
	mtk_hw_clear_irq(hw->mdev, hw->pci_ext_irq_id);

	/* register CLDMA interrupt handler */
	mtk_hw_register_irq(hw->mdev, hw->pci_ext_irq_id, mtk_cldma_isr, hw);

	/* unmask PCI CLDMA L1 interrupt */
	mtk_hw_unmask_irq(hw->mdev, hw->pci_ext_irq_id);

	mtk_cldma_irq_registed = false;
        if (hif_id == CLDMA0)
                cldma0_irq_registed = true;
        else if (hif_id == CLDMA1)
                cldma1_irq_registed = true;
	if (cldma0_irq_registed && cldma1_irq_registed)
		mtk_cldma_irq_registed = true;
	cd->cldma_hw[hif_id] = hw;
	MTK_INFO(hw->mdev, "CLDMA%d init done\n", hif_id);
	return 0;

free_mem:
	dma_pool_destroy(hw->dma_pool);
err_exit:
	mtk_hw_mask_irq(hw->mdev, hw->pci_ext_irq_id);
	mtk_hw_unregister_irq(hw->mdev, hw->pci_ext_irq_id);
	devm_kfree(hw->mdev->dev, hw);

	return -EIO;
}

static int mtk_cldma_hw_exit_t800(struct cldma_dev *cd, int hif_id)
{
	struct mtk_md_dev *mdev;
	struct cldma_hw *hw;
	int virq_id;
	int i;

	if (!cd || hif_id == NR_CLDMA)
		return -EINVAL;

	if (!cd->cldma_hw[hif_id])
		return 0;

	/* free cldma descriptor */
	hw = cd->cldma_hw[hif_id];
	mdev = cd->trans->mdev;
	virq_id = mtk_hw_get_virq_id(mdev, hw->pci_ext_irq_id);
	if (!mtk_hw_mmio_check(mdev))
		mtk_hw_mask_irq(mdev, hw->pci_ext_irq_id);
	synchronize_irq(virq_id);
	for (i = 0; i < HW_QUEUE_NUM; i++) {
		if (hw->txq[i])
			cd->hw_ops->txq_free(hw, hw->txq[i]->vqno);
		if (hw->rxq[i])
			cd->hw_ops->rxq_free(hw, hw->rxq[i]->vqno);
	}

	cldma_dbgfs_exit(hw);
	flush_workqueue(hw->wq);
	destroy_workqueue(hw->wq);
	dma_pool_destroy(hw->dma_pool);
	mtk_hw_unregister_irq(mdev, hw->pci_ext_irq_id);
	mtk_cldma_irq_registed = false;
	mtk_hw_mask_irq(mdev, hw->pci_ext_irq_id);
        if (hif_id == CLDMA0) 
		cldma0_irq_registed = false;
	else if (hif_id == CLDMA1)
		cldma1_irq_registed = false;
	devm_kfree(mdev->dev, hw);
	cd->cldma_hw[hif_id] = NULL;
	MTK_INFO(mdev, "CLDMA%d exit done\n", hif_id);

	return 0;
}

static struct txq *mtk_cldma_txq_alloc_t800(struct cldma_hw *hw, struct sk_buff *skb)
{
	struct trb *trb = (struct trb *)skb->cb;
	struct tx_req *next;
	struct tx_req *req;
	struct txq *txq;
	int i;

	txq = devm_kzalloc(hw->mdev->dev, sizeof(*txq), GFP_KERNEL);
	if (!txq)
		return NULL;

	txq->hw = hw;
	txq->vqno = trb->vqno;
	txq->vq = hw->cd->trans->vq_tbl + trb->vqno;
	txq->txqno = txq->vq->txqno;
	txq->req_pool_size = txq->vq->tx_req_num;
	txq->req_budget = txq->vq->tx_req_num;
	txq->is_stopping = false;
	mutex_init(&txq->lock);
	if (unlikely(txq->txqno < 0 || txq->txqno >= HW_QUEUE_NUM))
		goto free_txq;

	txq->req_pool = devm_kcalloc(hw->mdev->dev, txq->req_pool_size, sizeof(*req), GFP_KERNEL);
	if (!txq->req_pool)
		goto free_txq;

	for (i = 0; i < txq->req_pool_size; i++) {
		req = txq->req_pool + i;
		req->mtu = txq->vq->tx_mtu;
		req->gpd = dma_pool_zalloc(hw->dma_pool, GFP_KERNEL, &req->gpd_dma_addr);
		if (!req->gpd)
			goto free_req;
	}

	for (i = 0; i < txq->req_pool_size; i++) {
		req = txq->req_pool + i;
		next = txq->req_pool + ((i + 1) % txq->req_pool_size);
		req->gpd->tx_gpd.next_gpd_ptr_h = cpu_to_le32((u64)(next->gpd_dma_addr) >> 32);
		req->gpd->tx_gpd.next_gpd_ptr_l = cpu_to_le32(next->gpd_dma_addr);
	}

	INIT_WORK(&txq->tx_done_work, mtk_cldma_tx_done_work);

	mtk_cldma_stop_queue(hw, DIR_TX, txq->txqno);
	txq->tx_started = false;
	mtk_cldma_setup_start_addr(hw->mdev, hw->base_addr, DIR_TX, txq->txqno,
				   txq->req_pool[0].gpd_dma_addr);
	mtk_cldma_unmask_intr(hw->mdev, hw->base_addr, DIR_TX, txq->txqno, QUEUE_ERROR);
	mtk_cldma_unmask_intr(hw->mdev, hw->base_addr, DIR_TX, txq->txqno, QUEUE_XFER_DONE);

	hw->txq[txq->txqno] = txq;
	return txq;

free_req:
	for (i--; i >= 0; i--) {
		req = txq->req_pool + i;
		dma_pool_free(hw->dma_pool, req->gpd, req->gpd_dma_addr);
	}

	devm_kfree(hw->mdev->dev, txq->req_pool);
free_txq:
	devm_kfree(hw->mdev->dev, txq);
	return NULL;
}

static int mtk_cldma_txq_free_t800(struct cldma_hw *hw, int vqno)
{
	struct virtq *vq = hw->cd->trans->vq_tbl + vqno;
	struct tx_req *req;
	struct txq *txq;
	struct trb *trb;
	int irq_id;
	int txqno;
	int i;

	txqno = vq->txqno;
	if (unlikely(txqno < 0 || txqno >= HW_QUEUE_NUM))
		return -EINVAL;
	txq = hw->txq[txqno];
	if (!txq)
		return -EINVAL;

	hw->txq[txqno] = NULL;
	/* stop HW tx transaction */
	if (!mtk_hw_mmio_check(hw->mdev))
		mtk_cldma_stop_queue(hw, DIR_TX, txqno);
	txq->tx_started = false;

	irq_id = mtk_hw_get_virq_id(hw->mdev, hw->pci_ext_irq_id);
	synchronize_irq(irq_id);
	/* flush on-going work */
	flush_work(&txq->tx_done_work);
	if (!mtk_hw_mmio_check(hw->mdev)) {
		mtk_cldma_mask_intr(hw->mdev, hw->base_addr, DIR_TX, txqno, QUEUE_XFER_DONE);
		mtk_cldma_mask_intr(hw->mdev, hw->base_addr, DIR_TX, txqno, QUEUE_ERROR);
	}

	/* free tx req resource */
	for (i = 0; i < txq->req_pool_size; i++) {
		req = txq->req_pool + txq->free_idx;
		if (req->data_dma_addr && req->data_len) {
			dma_unmap_single(hw->mdev->dev,
					 req->data_dma_addr, req->data_len, DMA_TO_DEVICE);
			trb = (struct trb *)req->skb->cb;
			trb->status = -EPIPE;
			trb->trb_complete(req->skb);
		}
		dma_pool_free(hw->dma_pool, req->gpd, req->gpd_dma_addr);
		txq->free_idx = (txq->free_idx + 1) % txq->req_pool_size;
	}

	devm_kfree(hw->mdev->dev, txq->req_pool);
	devm_kfree(hw->mdev->dev, txq);

	return 0;
}

static struct rxq *mtk_cldma_rxq_alloc_t800(struct cldma_hw *hw, struct sk_buff *skb)
{
	struct trb_open_priv *trb_open_priv = (struct trb_open_priv *)skb->data;
	struct trb *trb = (struct trb *)skb->cb;
	char ws_name[CLDMA_WS_NAME_LEN];
	struct mtk_bm_pool *bm_pool;
	struct rx_req *next;
	struct rx_req *req;
	struct rxq *rxq;
	int err;
	int i;

	rxq = devm_kzalloc(hw->mdev->dev, sizeof(*rxq), GFP_KERNEL);
	if (!rxq)
		return NULL;

	rxq->hw = hw;
	rxq->vqno = trb->vqno;
	rxq->vq = hw->cd->trans->vq_tbl + trb->vqno;
	rxq->rxqno = rxq->vq->rxqno;
	rxq->req_pool_size = rxq->vq->rx_req_num;
	rxq->arg = trb->priv;
	rxq->rx_done = trb_open_priv->rx_done;
	if (unlikely(rxq->rxqno < 0 || rxq->rxqno >= HW_QUEUE_NUM))
		goto free_rxq;

	snprintf(ws_name, CLDMA_WS_NAME_LEN, "cldma%dq%d_wakeup_source",
		 hw->hif_id, rxq->rxqno);
	rxq->ws = wakeup_source_register(NULL, ws_name);
	if (!rxq->ws) {
		MTK_ERR(hw->mdev, "Failed to register cldma%d rxq%d wakeup source\n",
			hw->hif_id, rxq->rxqno);
		goto free_rxq;
	}

	rxq->req_pool = devm_kcalloc(hw->mdev->dev, rxq->req_pool_size, sizeof(*req), GFP_KERNEL);
	if (!rxq->req_pool)
		goto unregister_ws;

	if (rxq->vq->rx_mtu > VQ_MTU_3_5K)
		bm_pool = hw->cd->trans->ctrl_blk->bm_pool_63K;
	else
		bm_pool = hw->cd->trans->ctrl_blk->bm_pool;

	/* setup rx request */
	for (i = 0; i < rxq->req_pool_size; i++) {
		req = rxq->req_pool + i;
		req->mtu = rxq->vq->rx_mtu;
		req->gpd = dma_pool_zalloc(hw->dma_pool, GFP_KERNEL, &req->gpd_dma_addr);
		if (!req->gpd)
			goto free_req;

		req->skb = mtk_bm_alloc(bm_pool);
		if (!req->skb) {
			dma_pool_free(hw->dma_pool, req->gpd, req->gpd_dma_addr);
			goto free_req;
		}

		req->data_dma_addr = dma_map_single(hw->mdev->dev,
						    req->skb->data, req->mtu, DMA_FROM_DEVICE);
		err = dma_mapping_error(hw->mdev->dev, req->data_dma_addr);
		if (unlikely(err)) {
			i++;
			goto free_req;
		}
	}

	for (i = 0; i < rxq->req_pool_size; i++) {
		req = rxq->req_pool + i;
		next = rxq->req_pool + ((i + 1) % rxq->req_pool_size);
		req->gpd->rx_gpd.gpd_flags = CLDMA_GPD_FLAG_IOC | CLDMA_GPD_FLAG_HWO;
		req->gpd->rx_gpd.data_allow_len = cpu_to_le16(req->mtu);
		req->gpd->rx_gpd.next_gpd_ptr_h = cpu_to_le32((u64)(next->gpd_dma_addr) >> 32);
		req->gpd->rx_gpd.next_gpd_ptr_l = cpu_to_le32(next->gpd_dma_addr);
		req->gpd->rx_gpd.data_buff_ptr_h = cpu_to_le32((u64)(req->data_dma_addr) >> 32);
		req->gpd->rx_gpd.data_buff_ptr_l = cpu_to_le32(req->data_dma_addr);
	}

	INIT_WORK(&rxq->rx_done_work, mtk_cldma_rx_done_work);

	hw->rxq[rxq->rxqno] = rxq;
	mtk_cldma_stop_queue(hw, DIR_RX, rxq->rxqno);
	mtk_cldma_setup_start_addr(hw->mdev, hw->base_addr, DIR_RX,
				   rxq->rxqno, rxq->req_pool[0].gpd_dma_addr);
	mtk_cldma_start_queue(hw->mdev, hw->base_addr, DIR_RX, rxq->rxqno);
	mtk_cldma_unmask_intr(hw->mdev, hw->base_addr, DIR_RX, rxq->rxqno, QUEUE_ERROR);
	mtk_cldma_unmask_intr(hw->mdev, hw->base_addr, DIR_RX, rxq->rxqno, QUEUE_XFER_DONE);
	mtk_cldma_unmask_intr(hw->mdev, hw->base_addr, DIR_RX, rxq->rxqno, QUEUE_EMPTY);

	return rxq;

free_req:
	for (i--; i >= 0; i--) {
		req = rxq->req_pool + i;
		dma_unmap_single(hw->mdev->dev, req->data_dma_addr, req->mtu, DMA_FROM_DEVICE);
		dma_pool_free(hw->dma_pool, req->gpd, req->gpd_dma_addr);
		if (req->skb)
			mtk_bm_free(bm_pool, req->skb);
	}

	devm_kfree(hw->mdev->dev, rxq->req_pool);
unregister_ws:
	wakeup_source_unregister(rxq->ws);
free_rxq:
	devm_kfree(hw->mdev->dev, rxq);
	return NULL;
}

static int mtk_cldma_rxq_free_t800(struct cldma_hw *hw, int vqno)
{
	struct mtk_bm_pool *bm_pool;
	struct mtk_md_dev *mdev;
	struct rx_req *req;
	struct virtq *vq;
	struct rxq *rxq;
	int irq_id;
	int rxqno;
	int i;

	mdev = hw->mdev;
	vq = hw->cd->trans->vq_tbl + vqno;
	rxqno = vq->rxqno;
	if (unlikely(rxqno < 0 || rxqno >= HW_QUEUE_NUM))
		return -EINVAL;
	rxq = hw->rxq[rxqno];
	if (!rxq)
		return -EINVAL;

	hw->rxq[rxqno] = NULL;
	if (rxq->vq->rx_mtu > VQ_MTU_3_5K)
		bm_pool = hw->cd->trans->ctrl_blk->bm_pool_63K;
	else
		bm_pool = hw->cd->trans->ctrl_blk->bm_pool;

	/* stop HW rx transaction */
	if (!mtk_hw_mmio_check(hw->mdev))
		mtk_cldma_stop_queue(hw, DIR_RX, rxqno);

	irq_id = mtk_hw_get_virq_id(hw->mdev, hw->pci_ext_irq_id);
	synchronize_irq(irq_id);
	/* flush on-going work */
	flush_work(&rxq->rx_done_work);
	/* mask L2 RX interrupt again to avoid race condition causing use-after-free issue */
	if (!mtk_hw_mmio_check(hw->mdev)) {
		mtk_cldma_mask_intr(mdev, hw->base_addr, DIR_RX, rxqno, QUEUE_XFER_DONE);
		mtk_cldma_mask_intr(mdev, hw->base_addr, DIR_RX, rxqno, QUEUE_ERROR);
		mtk_cldma_mask_intr(mdev, hw->base_addr, DIR_RX, rxqno, QUEUE_EMPTY);
	}

	/* free rx req resource */
	for (i = 0; i < rxq->req_pool_size; i++) {
		req = rxq->req_pool + rxq->free_idx;
		pr_info("[mtk_cldma_rxq_free_t800] data_dma_addr %llu ", req->data_dma_addr);
		if (!(req->gpd->rx_gpd.gpd_flags & CLDMA_GPD_FLAG_HWO) &&
		    le16_to_cpu(req->gpd->rx_gpd.data_recv_len) && req->data_dma_addr && (req->data_dma_addr != 0xffffff8000000000))  {
			dma_unmap_single(mdev->dev, req->data_dma_addr, req->mtu, DMA_FROM_DEVICE);
			MTK_DBG(mdev, MTK_DBG_CLDMA, MTK_MEMLOG_RG_10, "CLDMA%d rxq%d req%d in rxq_free\n",
					rxq->hw->hif_id, rxq->rxqno, rxq->free_idx);
			rxq->rx_done(req->skb, le16_to_cpu(req->gpd->rx_gpd.data_recv_len),
				     rxq->arg);
			req->skb = NULL;
		}

		dma_pool_free(hw->dma_pool, req->gpd, req->gpd_dma_addr);
		if (req->skb && req->data_dma_addr && (req->data_dma_addr != 0xffffff8000000000)) {
			dma_unmap_single(mdev->dev, req->data_dma_addr, req->mtu, DMA_FROM_DEVICE);
			mtk_bm_free(bm_pool, req->skb);
		}
		rxq->free_idx = (rxq->free_idx + 1) % rxq->req_pool_size;
	}

	devm_kfree(mdev->dev, rxq->req_pool);
	wakeup_source_unregister(rxq->ws);
	devm_kfree(mdev->dev, rxq);

	return 0;
}

static int mtk_cldma_start_xfer_t800(struct cldma_hw *hw, int qno)
{
	struct txq *txq;
	u32 addr, val;

	txq = hw->txq[qno];
	addr = hw->base_addr + REG_CLDMA_UL_START_ADDRL_0 + qno * HW_QUEUE_NUM;
	val = mtk_hw_read32(hw->mdev, addr);
	if (unlikely(!val)) {
		mtk_cldma_hw_init(hw->mdev, hw->base_addr);
		txq = hw->txq[qno];
		mtk_cldma_setup_start_addr(hw->mdev, hw->base_addr, DIR_TX, qno,
					   txq->req_pool[txq->free_idx].gpd_dma_addr);
		mtk_cldma_start_queue(hw->mdev, hw->base_addr, DIR_TX, qno);
		txq->tx_started = true;
	} else {
		if (unlikely(!txq->tx_started)) {
			mtk_cldma_start_queue(hw->mdev, hw->base_addr, DIR_TX, qno);
			txq->tx_started = true;
		} else {
			mtk_cldma_resume_queue(hw->mdev, hw->base_addr, DIR_TX, qno);
		}
	}

	return 0;
}

static void mtk_cldma_suspend_t800(struct cldma_hw *hw)
{
	struct txq *txq;
	int i;

	mtk_cldma_stop_queue(hw, DIR_TX, ALLQ);

	for (i = 0; i < HW_QUEUE_NUM; i++) {
		txq = hw->txq[i];
		if (txq)
			flush_work(&txq->tx_done_work);
	}
}

static void mtk_cldma_suspend_late_t800(struct cldma_hw *hw)
{
	struct rxq *rxq;
	int i;

	mtk_cldma_stop_queue(hw, DIR_RX, ALLQ);

	for (i = 0; i < HW_QUEUE_NUM; i++) {
		rxq = hw->rxq[i];
		if (rxq)
			flush_work(&rxq->rx_done_work);
	}

	mtk_hw_mask_irq(hw->mdev, hw->pci_ext_irq_id);
}

static void mtk_cldma_resume_t800(struct cldma_hw *hw, bool is_runtime)
{
	struct mtk_md_dev *mdev = hw->mdev;
	struct rxq *rxq;
	struct txq *txq;
	int i;

	for (i = 0; i < HW_QUEUE_NUM; i++) {
		rxq = hw->rxq[i];
		if (rxq) {
			if (is_runtime) {
				mtk_cldma_resume_queue(hw->mdev, hw->base_addr, DIR_RX,
						       hw->rxq[i]->rxqno);
			} else {
				mtk_pm_ds_try_lock(mdev, MTK_USER_CTRL);
				queue_work(hw->wq, &rxq->rx_done_work);
			}
		}
		txq = hw->txq[i];
		if (txq) {
			if (txq->req_budget < 16)
				mtk_cldma_start_xfer_t800(hw, i);
		}
	}

	mtk_hw_unmask_irq(mdev, hw->pci_ext_irq_id);
}

static void mtk_cldma_hw_reset(struct cldma_hw *hw)
{
	struct mtk_md_dev *mdev = hw->mdev;
	int hif_id = hw->hif_id;
	u32 val = 0;

	val = mtk_hw_read32(mdev, hw->base_addr + REG_CLDMA_SLP_MEM_CTL);
	val |= 1 << REG_RST_INDICATOR_BIT;
	mtk_hw_write32(mdev, hw->base_addr + REG_CLDMA_SLP_MEM_CTL, val);

	val = mtk_hw_read32(mdev, REG_DEV_INFRA_BASE + REG_INFRA_RST0_SET);
	val |= 1 << (REG_CLDMA0_RST_SET_BIT + hif_id);
	mtk_hw_write32(mdev, REG_DEV_INFRA_BASE + REG_INFRA_RST0_SET, val);
	udelay(1);
	val = mtk_hw_read32(mdev, REG_DEV_INFRA_BASE + REG_INFRA_RST0_CLR);
	val |= 1 << (REG_CLDMA0_RST_CLR_BIT + hif_id);
	mtk_hw_write32(mdev, REG_DEV_INFRA_BASE + REG_INFRA_RST0_CLR, val);
}

static void mtk_cldma_fsm_state_listener_t800(struct mtk_fsm_param *param, struct cldma_hw *hw)
{
	struct txq *txq;
	int i;

	if (!param || !hw)
		return;

	if (param->to == FSM_STATE_MDEE) {
		if (param->fsm_flag & FSM_F_MDEE_INIT) {
			for (i = 0; i < HW_QUEUE_NUM; i++) {
				cldma_gpd_dump(hw, i);
			}
			cldma_hw_dump(hw);
			mtk_cldma_stop_queue(hw, DIR_TX, ALLQ);
			for (i = 0; i < HW_QUEUE_NUM; i++) {
				txq = hw->txq[i];
				if (txq)
					txq->is_stopping = true;
			}
		} else if (param->fsm_flag & FSM_F_MDEE_CLEARQ_DONE) {
			mtk_cldma_hw_reset(hw);
		} else if (param->fsm_flag & FSM_F_MDEE_ALLQ_RESET) {
			mtk_cldma_hw_init(hw->mdev, hw->base_addr);
			for (i = 0; i < HW_QUEUE_NUM; i++) {
				txq = hw->txq[i];
				if (txq)
					txq->is_stopping = false;
			}
			/* After leaving lowpower L2 states, PCIe will reset,
			 * so CLDMA L1 register needs to be set again.
			 */
			mtk_hw_unmask_irq(hw->mdev, hw->pci_ext_irq_id);
		}
	}
}

static void mtk_cldma_dump_t800(struct cldma_hw *hw)
{
	int i;
	int err;

	for (i = 0; i < HW_QUEUE_NUM; i++) {
		cldma_gpd_dump(hw, i);
	}

	pm_runtime_get_sync(hw->mdev->dev);
	mtk_pm_ds_lock(hw->mdev, MTK_USER_CTRL);
	err = mtk_pm_ds_wait_complete(hw->mdev, MTK_USER_CTRL);
	if (unlikely(err)) {
		MTK_ERR(hw->mdev, "Failed to lock ds:%d\n", err);
		goto err_ds_lock;
	}
	cldma_hw_dump(hw);

err_ds_lock:
	mtk_pm_ds_unlock(hw->mdev, MTK_USER_CTRL, false);
	pm_runtime_put_sync(hw->mdev->dev);
	if (err == -EIO)
		mtk_except_report_evt(hw->mdev, EXCEPT_LINK_ERR);
}

struct cldma_hw_ops cldma_hw_ops_t800 = {
	.init = mtk_cldma_hw_init_t800,
	.exit = mtk_cldma_hw_exit_t800,
	.txq_alloc = mtk_cldma_txq_alloc_t800,
	.rxq_alloc = mtk_cldma_rxq_alloc_t800,
	.txq_free = mtk_cldma_txq_free_t800,
	.rxq_free = mtk_cldma_rxq_free_t800,
	.start_xfer = mtk_cldma_start_xfer_t800,
	.suspend = mtk_cldma_suspend_t800,
	.suspend_late = mtk_cldma_suspend_late_t800,
	.resume = mtk_cldma_resume_t800,
	.fsm_state_listener = mtk_cldma_fsm_state_listener_t800,
	.dump = mtk_cldma_dump_t800,
};
