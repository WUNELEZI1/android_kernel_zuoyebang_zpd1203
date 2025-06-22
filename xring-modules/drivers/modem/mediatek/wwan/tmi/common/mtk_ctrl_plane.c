// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include <linux/device.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "mtk_bm.h"
#include "mtk_cldma.h"
#include "mtk_ctrl_plane.h"
#include "mtk_debug.h"
#include "mtk_port.h"
#ifdef CONFIG_TX00_UT_CLDMA
#include "ut_trb.h"
#endif
#include "mtk_reg.h"

#define TAG			"CTRL"

static struct virtq vq_tbl[] = {
	{VQ(0), CLDMA0, TXQ(0), RXQ(0), GRP(0),
	 VQ_MTU_2K, VQ_MTU_2K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(1), CLDMA0, TXQ(0), RXQ(0), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(2), CLDMA0, TXQ(1), RXQ(1), GRP(0),
	 VQ_MTU_2K, VQ_MTU_2K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(3), CLDMA0, TXQ(1), RXQ(1), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(4), CLDMA0, TXQ(2), RXQ(2), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(5), CLDMA0, TXQ(3), RXQ(3), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(6), CLDMA0, TXQ(5), RXQ(5), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(7), CLDMA1, TXQ(0), RXQ(0), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(8), CLDMA1, TXQ(1), RXQ(1), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(9), CLDMA1, TXQ(2), RXQ(2), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(10), CLDMA1, TXQ(3), RXQ(3), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(11), CLDMA1, TXQ(5), RXQ(5), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(12), CLDMA1, TXQ(7), RXQ(7), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_63K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(13), CLDMA0, TXQ(1), RXQ(1), GRP(0),
	 VQ_MTU_7K, VQ_MTU_63K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(14), CLDMA1, TXQ(4), RXQ(4), GRP(0),
	 VQ_MTU_63K, VQ_MTU_63K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(15), CLDMA0, TXQ(0), RXQ(0), GRP(0),
	 VQ_MTU_32K, VQ_MTU_32K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(16), CLDMA0, TXQ(6), RXQ(6), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(17), CLDMA1, TXQ(6), RXQ(6), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
	{VQ(18), CLDMA0, TXQ(4), RXQ(4), GRP(0),
	 VQ_MTU_3_5K, VQ_MTU_3_5K, TX_REQ_NUM, RX_REQ_NUM, 0},
};

static int mtk_ctrl_get_hif_id(unsigned char peer_id)
{
	if (peer_id == MTK_PEER_ID_SAP)
		return CLDMA0;
	else if (peer_id == MTK_PEER_ID_MD)
		return CLDMA1;
	else
		return -EINVAL;
}

int mtk_ctrl_vq_search(struct mtk_ctrl_blk *ctrl_blk, unsigned char peer_id,
		       unsigned char tx_hwq, unsigned char rx_hwq)
{
	struct mtk_port_mngr *port_mngr = ctrl_blk->port_mngr;
	struct mtk_ctrl_trans *trans = ctrl_blk->trans;
	int hif_id = mtk_ctrl_get_hif_id(peer_id);
	struct virtq *vq;
	int vq_num = 0;

	if (hif_id < 0)
		return -EINVAL;

	do {
		vq = trans->vq_tbl + vq_num;
		if (port_mngr->vq_info[vq_num].color && vq->txqno == tx_hwq &&
		    vq->rxqno == rx_hwq && vq->hif_id == hif_id)
			return vq_num;

		vq_num++;
	} while (vq_num < VQ_NUM);

	return -ENOENT;
}

int mtk_ctrl_vq_color_paint(struct mtk_ctrl_blk *ctrl_blk, unsigned char peer_id,
			    unsigned char tx_hwq, unsigned char rx_hwq,
			    unsigned int tx_mtu, unsigned int rx_mtu)
{
	struct mtk_port_mngr *port_mngr = ctrl_blk->port_mngr;
	struct mtk_ctrl_trans *trans = ctrl_blk->trans;
	int hif_id = mtk_ctrl_get_hif_id(peer_id);
	struct virtq *vq;
	int vq_num = 0;

	if (hif_id < 0)
		return -EINVAL;

	do {
		vq = trans->vq_tbl + vq_num;
		/* host and device hardware queue mtu need to sync */
		if (vq->hif_id == hif_id && vq->txqno == tx_hwq && vq->rxqno == rx_hwq &&
		    vq->tx_mtu == (tx_mtu - MTU_RSV_ROOM) &&
		    vq->rx_mtu == (rx_mtu - MTU_RSV_ROOM)) {
			port_mngr->vq_info[vq_num].color = true;
			break;
		}

		vq_num++;
	} while (vq_num < VQ_NUM);

	return 0;
}

int mtk_ctrl_vq_color_cleanup(struct mtk_ctrl_blk *ctrl_blk, unsigned char peer_id)
{
	struct mtk_port_mngr *port_mngr = ctrl_blk->port_mngr;
	struct mtk_ctrl_trans *trans = ctrl_blk->trans;
	int hif_id = mtk_ctrl_get_hif_id(peer_id);
	struct virtq *vq;
	int vq_num = 0;

	if (hif_id < 0)
		return -EINVAL;

	do {
		vq = trans->vq_tbl + vq_num;
		if (vq->hif_id == hif_id)
			port_mngr->vq_info[vq_num].color = false;

		vq_num++;
	} while (vq_num < VQ_NUM);

	return 0;
}

static bool mtk_ctrl_vqs_is_empty(struct trb_srv *srv)
{
	int i;

	for (i = srv->vq_start; i < srv->vq_cnt; i++) {
		if (!skb_queue_empty(&srv->trans->skb_list[i]))
			return false;
	}

	return true;
}

static void mtk_ctrl_vq_flush(struct trb_srv *srv, int vqno)
{
	struct mtk_ctrl_trans *trans = srv->trans;
	struct sk_buff *skb;
	struct trb *trb;

	while (!skb_queue_empty(&trans->skb_list[vqno])) {
		skb = skb_dequeue(&trans->skb_list[vqno]);
		trb = (struct trb *)skb->cb;
		trb->status = -EIO;
		trb->trb_complete(skb);
	}
}

static void mtk_ctrl_vqs_flush(struct trb_srv *srv)
{
	int i;

	for (i = srv->vq_start; i < srv->vq_cnt; i++)
		mtk_ctrl_vq_flush(srv, i);
}

static void mtk_ctrl_trb_process(struct trb_srv *srv)
{
	struct mtk_ctrl_trans *trans = srv->trans;
	struct sk_buff *skb, *skb_next;
	struct trb *trb, *trb_next;
	bool kick = false;
	struct virtq *vq;
	int loop;
	int idx;
	int err;
	int i;
	if (!trans) {
		MTK_ERR(trans->mdev, "trans is NULL\n");
		return;
	}
	for (i = srv->vq_start; i < srv->vq_start + srv->vq_cnt; i++) {
		loop = 0;
		do {
			if (skb_queue_empty(&trans->skb_list[i]))
				break;

			skb = skb_peek(&trans->skb_list[i]);
			trb = (struct trb *)skb->cb;
			kref_get(&trb->kref);
			vq = trans->vq_tbl + trb->vqno;
			idx = (vq->hif_id >> HIF_CLASS_SHIFT) & (HIF_CLASS_WIDTH - 1);
			if (idx < 0 || idx >= HIF_CLASS_NUM) {
				kref_put(&trb->kref, mtk_port_trb_free);
				break;
			}

			switch (trb->cmd) {
			case TRB_CMD_ENABLE:
			case TRB_CMD_DISABLE:
				skb_unlink(skb, &trans->skb_list[i]);
				err = mtk_port_mngr_vq_status_check(skb);
				if (!err) {
					kick = true;
					if (trb->cmd == TRB_CMD_DISABLE)
						mtk_ctrl_vq_flush(srv, i);
				}
				break;
			case TRB_CMD_TX:
				mtk_port_add_header(skb);
				err = trans->ops[idx]->submit_tx(trans->dev[idx], skb);
				if (err) {
					if (vq->tx_burst_cnt)
						kick = true;
					break;
				}

				vq->tx_burst_cnt++;
				if (vq->tx_burst_cnt >= TX_BURST_MAX_CNT ||
				    skb_queue_is_last(&trans->skb_list[i], skb)) {
					kick = true;
				} else {
					skb_next = skb_peek_next(skb, &trans->skb_list[i]);
					trb_next = (struct trb *)skb_next->cb;
					if (trb_next->cmd != TRB_CMD_TX)
						kick = true;
				}

				skb_unlink(skb, &trans->skb_list[i]);
				break;
			default:
				err = -EFAULT;
			}

			if (kick) {
				trans->ops[idx]->trb_process(trans->dev[idx], skb);
				vq->tx_burst_cnt = 0;
				kick = false;
			}

			kref_put(&trb->kref, mtk_port_trb_free);

			loop++;
		} while (loop < TRB_NUM_PER_ROUND);
	}
}

static int mtk_ctrl_trb_thread(void *args)
{
	struct trb_srv *srv = args;

	while (!kthread_should_stop()) {
		if (mtk_ctrl_vqs_is_empty(srv))
			wait_event_freezable(srv->trb_waitq,
					     !mtk_ctrl_vqs_is_empty(srv) ||
					     kthread_should_stop() || kthread_should_park());

		if (kthread_should_stop())
			break;

		if (kthread_should_park())
			kthread_parkme();

		do {
			mtk_ctrl_trb_process(srv);
			if (need_resched())
				cond_resched();
		} while (!mtk_ctrl_vqs_is_empty(srv) && !kthread_should_stop() &&
			 !kthread_should_park());
	}
	mtk_ctrl_vqs_flush(srv);
	return 0;
}

static int mtk_ctrl_trb_srv_init(struct mtk_ctrl_trans *trans)
{
	struct trb_srv *srv;
	int i;

	for (i = 0; i < TRB_SRV_NUM; i++) {
		srv = devm_kzalloc(trans->mdev->dev, sizeof(*srv), GFP_KERNEL);
		if (!srv) {
			for (i--; i >= 0; i--) {
				kthread_stop(trans->trb_srv[i]->trb_thread);
				devm_kfree(trans->mdev->dev, trans->trb_srv[i]);
				trans->trb_srv[i] = NULL;
			}
			return -ENOMEM;
		}

		srv->trans = trans;
		srv->srv_id = i;
		srv->vq_start = i * (VQ_NUM / TRB_SRV_NUM);
		srv->vq_cnt = (VQ_NUM / TRB_SRV_NUM);
		if (i == TRB_SRV_NUM - 1)
			srv->vq_cnt += (VQ_NUM % TRB_SRV_NUM);

		init_waitqueue_head(&srv->trb_waitq);
		srv->trb_thread = kthread_run(mtk_ctrl_trb_thread, srv, "mtk_trb_srv%d_%s",
					      i, trans->mdev->dev_str);
		trans->trb_srv[i] = srv;
	}

	return 0;
}

static void mtk_ctrl_trb_srv_exit(struct mtk_ctrl_trans *trans)
{
	struct trb_srv *srv;
	int i;

	for (i = 0; i < TRB_SRV_NUM; i++) {
		srv = trans->trb_srv[i];
		kthread_stop(srv->trb_thread);
		devm_kfree(trans->mdev->dev, srv);
		trans->trb_srv[i] = NULL;
	}
}

int mtk_ctrl_trb_submit(struct mtk_ctrl_blk *blk, struct sk_buff *skb, bool is_blocking)
{
	struct mtk_ctrl_trans *trans = blk->trans;
	struct virtq *vq;
	struct trb *trb;
	int vqno;

	trb = (struct trb *)skb->cb;
	if (trb->vqno < 0 || trb->vqno >= VQ_NUM)
		return -EINVAL;

	vq = trans->vq_tbl + trb->vqno;
	if (vq->grp < 0 || vq->grp >= TRB_SRV_NUM)
		return -EINVAL;

	if (!atomic_read(&trans->available))
		return -EIO;

	vqno = trb->vqno;
	if (VQ_LIST_FULL(trans, vqno) && !is_blocking)
		return -EAGAIN;

	if (vqno == VQ(9) && trb->cmd == TRB_CMD_TX && trans->skb_list[9].qlen > 1000) {
		MTK_ERR(blk->mdev, "tx list hold too much data");
		//mtk_hw_send_ext_evt(blk->mdev, EXT_EVT_H2D_RESERVED_FOR_DPMAIF);
	}

	if (trb->cmd == TRB_CMD_DISABLE)
		skb_queue_head(&trans->skb_list[vqno], skb);
	else
		skb_queue_tail(&trans->skb_list[vqno], skb);

	pm_runtime_get_sync(blk->mdev->dev);
	wake_up(&trans->trb_srv[vq->grp]->trb_waitq);
	pm_runtime_put_sync(blk->mdev->dev);

	return 0;
}

static int mtk_ctrl_trans_init(struct mtk_ctrl_blk *ctrl_blk)
{
	struct mtk_ctrl_trans *trans;
	int err;
	int i;

	trans = devm_kzalloc(ctrl_blk->mdev->dev, sizeof(*trans), GFP_KERNEL);
	if (!trans)
		return -ENOMEM;

	trans->ctrl_blk = ctrl_blk;
	trans->vq_tbl = vq_tbl;
	trans->ops[CLDMA_CLASS_ID] = &cldma_ops;
	trans->mdev = ctrl_blk->mdev;

	for (i = 0; i < VQ_NUM; i++)
		skb_queue_head_init(&trans->skb_list[i]);

	for (i = 0; i < HIF_CLASS_NUM; i++) {
		err = trans->ops[i]->init(trans);
		if (err)
			goto err_exit;
	}

	err = mtk_ctrl_trb_srv_init(trans);
	if (err)
		goto err_exit;

	ctrl_blk->trans = trans;
	atomic_set(&trans->available, 1);

	return 0;

err_exit:
	for (i--; i >= 0; i--)
		trans->ops[i]->exit(trans);

	devm_kfree(ctrl_blk->mdev->dev, trans);
	return err;
}

static int mtk_ctrl_trans_exit(struct mtk_ctrl_blk *ctrl_blk)
{
	struct mtk_ctrl_trans *trans = ctrl_blk->trans;
	int i;

	atomic_set(&trans->available, 0);
	mtk_ctrl_trb_srv_exit(trans);

	for (i = 0; i < HIF_CLASS_NUM; i++)
		trans->ops[i]->exit(trans);

	devm_kfree(ctrl_blk->mdev->dev, trans);
	return 0;
}

static void mtk_ctrl_trans_fsm_state_handler(struct mtk_fsm_param *param,
					     struct mtk_ctrl_blk *ctrl_blk)
{
	int i;

	switch (param->to) {
	case FSM_STATE_OFF:
		for (i = 0; i < HIF_CLASS_NUM; i++)
			ctrl_blk->trans->ops[i]->fsm_state_listener(param, ctrl_blk->trans);
		mtk_ctrl_trans_exit(ctrl_blk);
		break;
	case FSM_STATE_ON:
		mtk_ctrl_trans_init(ctrl_blk);
		fallthrough;
	default:
		for (i = 0; i < HIF_CLASS_NUM; i++)
			ctrl_blk->trans->ops[i]->fsm_state_listener(param, ctrl_blk->trans);
	}
}

static int mtk_ctrl_pm_suspend(struct mtk_md_dev *mdev, void *param, bool is_runtime)
{
	struct mtk_ctrl_blk *ctrl_blk = param;
	int i;

	ctrl_blk->port_mngr->ops->suspend(ctrl_blk->port_mngr);

	for (i = 0; i < TRB_SRV_NUM; i++)
		kthread_park(ctrl_blk->trans->trb_srv[i]->trb_thread);

	for (i = 0; i < HIF_CLASS_NUM; i++)
		ctrl_blk->trans->ops[i]->suspend(ctrl_blk->trans);

	return 0;
}

static int mtk_ctrl_pm_suspend_late(struct mtk_md_dev *mdev, void *param, bool is_runtime)
{
	struct mtk_ctrl_blk *ctrl_blk = param;
	int i;

	for (i = 0; i < HIF_CLASS_NUM; i++)
		ctrl_blk->trans->ops[i]->suspend_late(ctrl_blk->trans);

	return 0;
}

static int mtk_ctrl_pm_resume(struct mtk_md_dev *mdev, void *param, bool is_runtime,
			      bool link_ready)
{
	struct mtk_ctrl_blk *ctrl_blk = param;
	int i;

	ctrl_blk->port_mngr->ops->resume(ctrl_blk->port_mngr);

	for (i = 0; i < HIF_CLASS_NUM; i++)
		ctrl_blk->trans->ops[i]->resume(ctrl_blk->trans, is_runtime);

	for (i = 0; i < TRB_SRV_NUM; i++)
		kthread_unpark(ctrl_blk->trans->trb_srv[i]->trb_thread);

	return 0;
}

static int mtk_ctrl_pm_init(struct mtk_ctrl_blk *ctrl_blk)
{
	struct mtk_pm_entity *pm_entity;
	int ret;

	pm_entity = &ctrl_blk->pm_entity;
	INIT_LIST_HEAD(&pm_entity->entry);
	pm_entity->user = MTK_USER_CTRL;
	pm_entity->param = ctrl_blk;
	pm_entity->suspend = mtk_ctrl_pm_suspend;
	pm_entity->suspend_late = mtk_ctrl_pm_suspend_late;
	pm_entity->resume = mtk_ctrl_pm_resume;
	ret = mtk_pm_entity_register(ctrl_blk->mdev, pm_entity);
	if (ret < 0)
		MTK_ERR(ctrl_blk->mdev, "Failed to register ctrl pm_entity\n");

	return ret;
}

static int mtk_ctrl_pm_exit(struct mtk_ctrl_blk *ctrl_blk)
{
	int ret;

	ret = mtk_pm_entity_unregister(ctrl_blk->mdev, &ctrl_blk->pm_entity);
	if (ret < 0)
		MTK_ERR(ctrl_blk->mdev, "Failed to unregister ctrl pm_entity\n");

	return ret;
}

static void mtk_ctrl_fsm_state_listener(struct mtk_fsm_param *param, void *data)
{
	struct mtk_ctrl_blk *ctrl_blk = data;

	if ((param->to == FSM_STATE_MDEE && param->fsm_flag & FSM_F_MDEE_ALLQ_RESET) ||
	    (param->to == FSM_STATE_MDEE && param->fsm_flag & FSM_F_MDEE_INIT) ||
	    param->to == FSM_STATE_POSTDUMP || param->to == FSM_STATE_DOWNLOAD ||
	    param->to == FSM_STATE_BOOTUP) {
		mtk_ctrl_trans_fsm_state_handler(param, ctrl_blk);
		mtk_port_mngr_fsm_state_handler(param, ctrl_blk->port_mngr);
	} else {
		mtk_port_mngr_fsm_state_handler(param, ctrl_blk->port_mngr);
		mtk_ctrl_trans_fsm_state_handler(param, ctrl_blk);
	}
}

void mtk_ctrl_dump(struct mtk_md_dev *mdev)
{
	struct mtk_ctrl_blk *ctrl_blk = mdev->ctrl_blk;
	int i;

	if (!ctrl_blk->trans)
		return;

	for (i = 0; i < HIF_CLASS_NUM; i++)
		ctrl_blk->trans->ops[i]->dump(ctrl_blk->trans);
}

int mtk_ctrl_init(struct mtk_md_dev *mdev)
{
	struct mtk_ctrl_blk *ctrl_blk;
	int err;

	ctrl_blk = devm_kzalloc(mdev->dev, sizeof(*ctrl_blk), GFP_KERNEL);
	if (!ctrl_blk)
		return -ENOMEM;

	ctrl_blk->mdev = mdev;
	mdev->ctrl_blk = ctrl_blk;

	ctrl_blk->bm_pool = mtk_bm_pool_create(mdev, MTK_BUFF_SKB,
					       VQ_MTU_3_5K, 100, MTK_BM_LOW_PRIO,
					       "ctrl_bm_pool_%s", mdev->dev_str);
	if (!ctrl_blk->bm_pool) {
		err = -ENOMEM;
		goto err_free_mem;
	}

	ctrl_blk->bm_pool_63K = mtk_bm_pool_create(mdev, MTK_BUFF_SKB,
						   VQ_MTU_63K, 64, MTK_BM_LOW_PRIO,
						   "ctrl_bm_pool_63K_%s", mdev->dev_str);

	if (!ctrl_blk->bm_pool_63K) {
		err = -ENOMEM;
		goto err_destroy_pool;
	}

	err = mtk_port_mngr_init(ctrl_blk);
	if (err)
		goto err_destroy_pool_63K;

	err = mtk_fsm_notifier_register(mdev, MTK_USER_CTRL, mtk_ctrl_fsm_state_listener,
					ctrl_blk, FSM_PRIO_1, false);
	if (err) {
		MTK_ERR(mdev, "Failed to register fsm notify(ret = %d)\n", err);
		goto err_port_exit;
	}

	err = mtk_ctrl_pm_init(ctrl_blk);
	if (err)
		goto err_unregister_notifiers;

	return 0;

err_unregister_notifiers:
	mtk_fsm_notifier_unregister(mdev, MTK_USER_CTRL);
err_port_exit:
	mtk_port_mngr_exit(ctrl_blk);
err_destroy_pool_63K:
	mtk_bm_pool_destroy(mdev, ctrl_blk->bm_pool_63K);
err_destroy_pool:
	mtk_bm_pool_destroy(mdev, ctrl_blk->bm_pool);
err_free_mem:
	devm_kfree(mdev->dev, ctrl_blk);

	return err;
}

int mtk_ctrl_exit(struct mtk_md_dev *mdev)
{
	struct mtk_ctrl_blk *ctrl_blk = mdev->ctrl_blk;

	mtk_ctrl_pm_exit(ctrl_blk);
	mtk_fsm_notifier_unregister(mdev, MTK_USER_CTRL);
	mtk_port_mngr_exit(ctrl_blk);
	mtk_bm_pool_destroy(mdev, ctrl_blk->bm_pool);
	mtk_bm_pool_destroy(mdev, ctrl_blk->bm_pool_63K);
	devm_kfree(mdev->dev, ctrl_blk);

	return 0;
}
