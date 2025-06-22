/* SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Copyright (c) 2022, MediaTek Inc.
 */

#ifndef __MTK_CTRL_PLANE_H__
#define __MTK_CTRL_PLANE_H__

#include <linux/kref.h>
#include <linux/skbuff.h>

#include "mtk_dev.h"
#include "mtk_fsm.h"

#define VQ(N)				(N)
#define GRP(N)				(N)
#define VQ_NUM				(19)
#define TRB_SRV_NUM			(1)
#define TX_REQ_NUM			(16)
#define RX_REQ_NUM			(TX_REQ_NUM)

#define VQ_MTU_2K			(0x800)
#define VQ_MTU_3_5K			(0xE00)
#define VQ_MTU_7K			(0x1C00)
#define VQ_MTU_32K			(0x8000)
#define VQ_MTU_63K			(0xFC00)

#define TRB_NUM_PER_ROUND		(16)
#define SKB_LIST_MAX_LEN		(16)
#define TX_BURST_MAX_CNT		(5)

#define HIF_CLASS_NUM			(1)
#define HIF_CLASS_SHIFT			(8)
#define HIF_CLASS_WIDTH			(8)
#define HIF_ID_BITMASK			(0x01)

#define MTU_RSV_ROOM			(0x100)

#define VQ_LIST_FULL(trans, vqno)	((trans)->skb_list[vqno].qlen >= SKB_LIST_MAX_LEN)

enum mtk_trb_cmd_type {
	TRB_CMD_ENABLE = 1,
	TRB_CMD_TX,
	TRB_CMD_DISABLE,
};

struct trb_open_priv {
	u16 tx_mtu;
	u16 rx_mtu;
	int (*rx_done)(struct sk_buff *skb, int len, void *priv);
};

struct trb {
	u8 vqno;
	enum mtk_trb_cmd_type cmd;
	int status;
	struct kref kref;
	void *priv;
	int (*trb_complete)(struct sk_buff *skb);
};

struct trb_srv {
	int srv_id;
	int vq_cnt;
	int vq_start;
	struct mtk_ctrl_trans *trans;
	wait_queue_head_t trb_waitq;
	struct task_struct *trb_thread;
};

struct virtq {
	int vqno;
	int hif_id;
	int txqno;
	int rxqno;
	int grp;
	int tx_mtu;
	int rx_mtu;
	int tx_req_num;
	int rx_req_num;
	int tx_burst_cnt;
};

struct hif_ops {
	int (*init)(struct mtk_ctrl_trans *trans);
	int (*exit)(struct mtk_ctrl_trans *trans);
	int (*suspend)(struct mtk_ctrl_trans *trans);
	int (*suspend_late)(struct mtk_ctrl_trans *trans);
	int (*resume)(struct mtk_ctrl_trans *trans, bool is_runtime);
	int (*submit_tx)(void *dev, struct sk_buff *skb);
	int (*trb_process)(void *dev, struct sk_buff *skb);
	void (*fsm_state_listener)(struct mtk_fsm_param *param, struct mtk_ctrl_trans *trans);
	void (*dump)(struct mtk_ctrl_trans *trans);
};

struct mtk_ctrl_trans {
	struct sk_buff_head skb_list[VQ_NUM];
	struct trb_srv *trb_srv[TRB_SRV_NUM];
	struct virtq *vq_tbl;
	void *dev[HIF_CLASS_NUM];
	struct hif_ops *ops[HIF_CLASS_NUM];
	struct mtk_ctrl_blk *ctrl_blk;
	struct mtk_md_dev *mdev;
	atomic_t available;
};

struct mtk_ctrl_blk {
	struct mtk_md_dev *mdev;
	struct mtk_port_mngr *port_mngr;
	struct mtk_ctrl_trans *trans;
	struct mtk_bm_pool *bm_pool;
	struct mtk_bm_pool *bm_pool_63K;
	struct mtk_pm_entity pm_entity;
};

int mtk_ctrl_vq_search(struct mtk_ctrl_blk *ctrl_blk, unsigned char peer_id,
		       unsigned char tx_hwq, unsigned char rx_hwq);
int mtk_ctrl_vq_color_paint(struct mtk_ctrl_blk *ctrl_blk, unsigned char peer_id,
			    unsigned char tx_hwq, unsigned char rx_hwq,
			    unsigned int tx_mtu, unsigned int rx_mtu);
int mtk_ctrl_vq_color_cleanup(struct mtk_ctrl_blk *ctrl_blk, unsigned char peer_id);
int mtk_ctrl_trb_submit(struct mtk_ctrl_blk *blk, struct sk_buff *skb, bool is_blocking);
int mtk_ctrl_init(struct mtk_md_dev *mdev);
int mtk_ctrl_exit(struct mtk_md_dev *mdev);
void mtk_ctrl_dump(struct mtk_md_dev *mdev);

#endif /* __MTK_CTRL_PLANE_H__ */
