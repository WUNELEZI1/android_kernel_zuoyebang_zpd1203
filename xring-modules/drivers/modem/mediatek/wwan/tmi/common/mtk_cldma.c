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
#include <linux/sched.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include "mtk_cldma.h"
#ifdef CONFIG_TX00_UT_CLDMA
#include "ut_cldma.h"
#endif
#include "mtk_dev.h"
#include "mtk_debug.h"
#include "mtk_cldma_hw_interface.h"
#define TAG "TRANS"

#define cldma_hw_ops_null NULL

extern int mtk_except_linkerr_type;

static struct cldma_hw_ops *mtk_cldma_get_hw_ops(u32 hw_ver)
{
	struct cldma_hw_ops_desc *p_hw_ops;
	u8 i;

	for (i = 0; (p_hw_ops = &cldma_hw_ops_tbl[i]) && p_hw_ops && p_hw_ops->hw_ops; i++)
		if (p_hw_ops->hw_ver == hw_ver)
			return p_hw_ops->hw_ops;
	return NULL;
}

/* cldma_init() - Initialize CLDMA
 *
 * @trans: pointer to transaction structure
 *
 * Return:
 * 0 - OK
 * -ENOMEM - out of memory
 * -ENXIO - no PCI device
 */
static int mtk_cldma_init(struct mtk_ctrl_trans *trans)
{
	struct mtk_md_dev *mdev;
	struct cldma_dev *cd;

	cd = devm_kzalloc(trans->mdev->dev, sizeof(*cd), GFP_KERNEL);
	if (!cd)
		return -ENOMEM;

	cd->trans = trans;
	mdev = trans->mdev;
	cd->hw_ops = mtk_cldma_get_hw_ops(mdev->hw_ver);

	if (!cd->hw_ops) {
		MTK_ERR(mdev, "Failed to find CLDMA Driver for PCI %x\n", mdev->hw_ver);
		devm_kfree(mdev->dev, cd);
		return -ENXIO;
	}

	trans->dev[CLDMA_CLASS_ID] = cd;

	return 0;
}

/* cldma_exit() - De-Initialize CLDMA
 *
 * @trans: pointer to transaction structure
 *
 * Return:
 * 0 - OK
 */
static int mtk_cldma_exit(struct mtk_ctrl_trans *trans)
{
	struct cldma_dev *cd;

	cd = trans->dev[CLDMA_CLASS_ID];
	if (!cd)
		return 0;

	devm_kfree(trans->mdev->dev, cd);
	trans->dev[CLDMA_CLASS_ID] = NULL;

	return 0;
}

/* cldma_open() - Initialize CLDMA hardware queue
 *
 * @cd: pointer to CLDMA device
 * @skb: pointer to socket buffer
 *
 * Return:
 * 0 - OK
 * -EBUSY - hardware queue is busy
 * -EIO - failed to initialize hardware queue
 * -EINVAL - invalid input parameters
 */
static int mtk_cldma_open(struct cldma_dev *cd, struct sk_buff *skb)
{
	struct trb_open_priv *trb_open_priv = (struct trb_open_priv *)skb->data;
	struct trb *trb = (struct trb *)skb->cb;
	struct cldma_hw *hw;
	struct virtq *vq;
	struct txq *txq;
	struct rxq *rxq;
	int err = 0;

	vq = cd->trans->vq_tbl + trb->vqno;
	hw = cd->cldma_hw[vq->hif_id & HIF_ID_BITMASK];
	if (!hw) {
		err = -EIO;
		goto exit;
	}

	trb_open_priv->tx_mtu = vq->tx_mtu;
	trb_open_priv->rx_mtu = vq->rx_mtu;
	if (unlikely(vq->rxqno < 0 || vq->rxqno >= HW_QUEUE_NUM) ||
	    unlikely(vq->txqno < 0 || vq->txqno >= HW_QUEUE_NUM)) {
		err = -EINVAL;
		goto exit;
	}

	if (hw->txq[vq->txqno] || hw->rxq[vq->rxqno]) {
		err = -EBUSY;
		goto exit;
	}

	txq = cd->hw_ops->txq_alloc(hw, skb);
	if (!txq) {
		err = -EIO;
		goto exit;
	}

	rxq = cd->hw_ops->rxq_alloc(hw, skb);
	if (!rxq) {
		err = -EIO;
		cd->hw_ops->txq_free(hw, trb->vqno);
		goto exit;
	}

exit:
	trb->status = err;
	trb->trb_complete(skb);

	return err;
}

/* cldma_tx() - start CLDMA TX transaction
 *
 * @cd: pointer to CLDMA device
 * @skb: pointer to socket buffer
 *
 * Return:
 * 0 - OK
 * -EPIPE - hardware queue is broken
 * -EIO - PCI link error
 */
static int mtk_cldma_tx(struct cldma_dev *cd, struct sk_buff *skb)
{
	struct trb *trb = (struct trb *)skb->cb;
	struct cldma_hw *hw;
	struct virtq *vq;
	struct txq *txq;
	int err = 0;

	vq = cd->trans->vq_tbl + trb->vqno;
	hw = cd->cldma_hw[vq->hif_id & HIF_ID_BITMASK];
	txq = hw->txq[vq->txqno];
	if (txq->is_stopping)
		return -EPIPE;

	pm_runtime_get_sync(hw->mdev->dev);
	mtk_pm_ds_lock(hw->mdev, MTK_USER_CTRL);
	err = mtk_pm_ds_wait_complete(hw->mdev, MTK_USER_CTRL);
	if (unlikely(err)) {
		MTK_ERR(hw->mdev, "Failed to lock ds:%d\n", err);
		goto exit;
	}

	cd->hw_ops->start_xfer(hw, vq->txqno);

exit:
	mtk_pm_ds_unlock(hw->mdev, MTK_USER_CTRL, false);
	pm_runtime_put_sync(hw->mdev->dev);
	if (err == -EIO) {
		mtk_except_linkerr_type = 2;
		mtk_except_report_evt(hw->mdev, EXCEPT_LINK_ERR);
	}
	return err;
}

/* cldma_close() - De-Initialize CLDMA hardware queue
 *
 * @cd: pointer to CLDMA device
 * @skb: pointer to socket buffer
 *
 * Return:
 * 0 - OK
 */
static int mtk_cldma_close(struct cldma_dev *cd, struct sk_buff *skb)
{
	struct trb *trb = (struct trb *)skb->cb;
	struct cldma_hw *hw;
	struct virtq *vq;

	vq = cd->trans->vq_tbl + trb->vqno;
	hw = cd->cldma_hw[vq->hif_id & HIF_ID_BITMASK];

	cd->hw_ops->txq_free(hw, trb->vqno);
	cd->hw_ops->rxq_free(hw, trb->vqno);

	trb->status = 0;
	trb->trb_complete(skb);

	return 0;
}

static int mtk_cldma_submit_tx(void *dev, struct sk_buff *skb)
{
	struct trb *trb = (struct trb *)skb->cb;
	struct cldma_dev *cd = dev;
	dma_addr_t data_dma_addr;
	struct cldma_hw *hw;
	struct tx_req *req;
	struct virtq *vq;
	struct txq *txq;
	int ret = 0;
	int err;

	vq = cd->trans->vq_tbl + trb->vqno;
	hw = cd->cldma_hw[vq->hif_id & HIF_ID_BITMASK];

	txq = hw->txq[vq->txqno];
	if (!txq) {
		ret = -EFAULT;
		goto err;
	}

	if (!txq->req_budget) {
		if (mtk_hw_mmio_check(hw->mdev)) {
			mtk_except_report_evt(hw->mdev, EXCEPT_LINK_ERR);
			ret = -EFAULT;
		} else {
			ret = -EAGAIN;
		}
		goto err;
	}

	data_dma_addr = dma_map_single(hw->mdev->dev, skb->data, skb->len, DMA_TO_DEVICE);
	err = dma_mapping_error(hw->mdev->dev, data_dma_addr);
	if (unlikely(err)) {
		dev_err(hw->mdev->dev, "Failed to map dma!\n");
		return err;
	}

	mutex_lock(&txq->lock);
	txq->req_budget--;
	mutex_unlock(&txq->lock);
	req = txq->req_pool + txq->wr_idx;
	req->gpd->tx_gpd.debug_id = 0x01;
	req->gpd->tx_gpd.data_buff_ptr_h = cpu_to_le32((u64)(data_dma_addr) >> 32);
	req->gpd->tx_gpd.data_buff_ptr_l = cpu_to_le32(data_dma_addr);
	req->gpd->tx_gpd.data_buff_len = cpu_to_le16(skb->len);
	req->gpd->tx_gpd.gpd_flags = CLDMA_GPD_FLAG_IOC | CLDMA_GPD_FLAG_HWO;

	wmb(); /* ensure HWO setup done before data addr set done */

	req->data_dma_addr = data_dma_addr;
	req->data_len = skb->len;
	req->skb = skb;

	wmb();

	req->data_vm_addr = skb->data;
	txq->wr_idx = (txq->wr_idx + 1) % txq->req_pool_size;

err:
	return ret;
}

static int mtk_cldma_suspend(struct mtk_ctrl_trans *trans)
{
	struct cldma_dev *cd = trans->dev[CLDMA_CLASS_ID];
	struct cldma_hw *hw;
	int i;

	for (i = 0; i < NR_CLDMA; i++) {
		hw = cd->cldma_hw[i];
		if (hw)
			cd->hw_ops->suspend(hw);
	}

	return 0;
}

static int mtk_cldma_suspend_late(struct mtk_ctrl_trans *trans)
{
	struct cldma_dev *cd = trans->dev[CLDMA_CLASS_ID];
	struct cldma_hw *hw;
	int i;

	for (i = 0; i < NR_CLDMA; i++) {
		hw = cd->cldma_hw[i];
		if (hw)
			cd->hw_ops->suspend_late(hw);
	}

	return 0;
}

static int mtk_cldma_resume(struct mtk_ctrl_trans *trans, bool is_runtime)
{
	struct cldma_dev *cd = trans->dev[CLDMA_CLASS_ID];
	struct cldma_hw *hw;
	int i;

	for (i = 0; i < NR_CLDMA; i++) {
		hw = cd->cldma_hw[i];
		if (hw)
			cd->hw_ops->resume(hw, is_runtime);
	}

	return 0;
}

/* cldma_trb_process() - Dispatch trb request to low-level CLDMA routine
 *
 * @cd: pointer to CLDMA device
 * @skb: pointer to socket buffer
 *
 * Return:
 * 0 - OK
 * -EINVAL - invalid input
 */
static int mtk_cldma_trb_process(void *dev, struct sk_buff *skb)
{
	struct cldma_dev *cd;
	struct trb *trb;
	int err;

	if (!dev || !skb)
		return -EINVAL;

	cd = (struct cldma_dev *)dev;
	trb = (struct trb *)skb->cb;

	switch (trb->cmd) {
	case TRB_CMD_ENABLE:
		err = mtk_cldma_open(cd, skb);
		break;
	case TRB_CMD_TX:
		err = mtk_cldma_tx(cd, skb);
		break;
	case TRB_CMD_DISABLE:
		err = mtk_cldma_close(cd, skb);
		break;
	default:
		err = -EINVAL;
	}

	return err;
}

static void mtk_cldma_fsm_state_listener(struct mtk_fsm_param *param, struct mtk_ctrl_trans *trans)
{
	struct cldma_dev *cd = trans->dev[CLDMA_CLASS_ID];
	struct cldma_hw *hw;
	int i;

	switch (param->to) {
	case FSM_STATE_POSTDUMP:
		cd->hw_ops->init(cd, CLDMA0);
		break;
	case FSM_STATE_DOWNLOAD:
		if ((param->fsm_flag & FSM_F_DL_PORT_CREATE) ||
		    (param->fsm_flag & FSM_F_DL_PL) ||
		    (param->fsm_flag & FSM_F_DL_FB))
			cd->hw_ops->init(cd, CLDMA0);
		break;
	case FSM_STATE_BOOTUP:
		for (i = 0; i < NR_CLDMA; i++)
			cd->hw_ops->init(cd, i);
		break;
	case FSM_STATE_OFF:
		for (i = 0; i < NR_CLDMA; i++)
			cd->hw_ops->exit(cd, i);
		break;
	case FSM_STATE_MDEE:
		if (param->fsm_flag & FSM_F_MDEE_INIT)
			cd->hw_ops->init(cd, CLDMA1);
		hw = cd->cldma_hw[CLDMA1 & HIF_ID_BITMASK];
		cd->hw_ops->fsm_state_listener(param, hw);
		break;
	default:
		break;
	}
}

static void mtk_cldma_dump(struct mtk_ctrl_trans *trans)
{
	struct cldma_dev *cd = trans->dev[CLDMA_CLASS_ID];
	struct cldma_hw *hw;
	int i;

	for (i = 0; i < NR_CLDMA; i++) {
		hw = cd->cldma_hw[i];
		if (hw)
			cd->hw_ops->dump(hw);
	}
}

struct hif_ops cldma_ops = {
	.init = mtk_cldma_init,
	.exit = mtk_cldma_exit,
	.suspend = mtk_cldma_suspend,
	.suspend_late = mtk_cldma_suspend_late,
	.resume = mtk_cldma_resume,
	.trb_process = mtk_cldma_trb_process,
	.submit_tx = mtk_cldma_submit_tx,
	.fsm_state_listener = mtk_cldma_fsm_state_listener,
	.dump = mtk_cldma_dump,
};
