/* SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Copyright (c) 2022, MediaTek Inc.
 */

#ifndef __MTK_CLDMA_H__
#define __MTK_CLDMA_H__

#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#include "mtk_ctrl_plane.h"

#define HW_QUEUE_NUM				8
#define ALLQ					(0XFF)
#define LINK_ERROR_VAL				(0XFFFFFFFF)

#define CLDMA_CLASS_ID				0

#define NR_CLDMA				2
#define CLDMA0					(((CLDMA_CLASS_ID) << HIF_CLASS_SHIFT) + 0)
#define CLDMA1					(((CLDMA_CLASS_ID) << HIF_CLASS_SHIFT) + 1)

#define TXQ(N)					(N)
#define RXQ(N)					(N)

#define CLDMA_GPD_FLAG_HWO			BIT(0)
#define CLDMA_GPD_FLAG_BDP			BIT(1)
#define CLDMA_GPD_FLAG_BPS			BIT(2)
#define CLDMA_GPD_FLAG_IOC			BIT(7)

enum mtk_ip_busy_src {
	IP_BUSY_TXDONE = 0,
	IP_BUSY_TXEMPTY = 8,
	IP_BUSY_TXACTIVE = 16,
	IP_BUSY_RXDONE = 24
};

enum mtk_intr_type {
	QUEUE_XFER_DONE = 0,
	QUEUE_EMPTY = 8,
	QUEUE_ERROR = 16,
	QUEUE_ACTIVE_START = 24,
	INVALID_TYPE
};

enum mtk_tx_rx {
	DIR_TX,
	DIR_RX,
	INVALID_DIR
};

union gpd {
	struct {
		u8 gpd_flags;
		u8 non_used1;
		__le16 data_allow_len;
		__le32 next_gpd_ptr_h;
		__le32 next_gpd_ptr_l;
		__le32 data_buff_ptr_h;
		__le32 data_buff_ptr_l;
		__le16 data_recv_len;
		u8 non_used2;
		u8 debug_id;
	} rx_gpd;

	struct {
		u8 gpd_flags;
		u8 non_used1;
		u8 non_used2;
		u8 debug_id;
		__le32 next_gpd_ptr_h;
		__le32 next_gpd_ptr_l;
		__le32 data_buff_ptr_h;
		__le32 data_buff_ptr_l;
		__le16 data_buff_len;
		__le16 non_used3;
	} tx_gpd;
} __packed;

struct rx_req {
	union gpd *gpd;
	int mtu;
	struct sk_buff *skb;
	size_t data_len;
	dma_addr_t gpd_dma_addr;
	dma_addr_t data_dma_addr;
};

struct rx_freq_stat {
	u64 last_isr_time;
	u64 avg_time_gap;
	u8 stat_cnt;
};

struct rxq {
	struct cldma_hw *hw;
	int rxqno;
	int vqno;
	struct virtq *vq;
	struct work_struct rx_done_work;
	struct rx_req *req_pool;
	int req_pool_size;
	int free_idx;
	unsigned short rx_done_cnt;
	void *arg;
	int (*rx_done)(struct sk_buff *skb, int len, void *priv);
	struct wakeup_source *ws;
	struct rx_freq_stat freq_stat;
};

struct tx_req {
	union gpd *gpd;
	int mtu;
	void *data_vm_addr;
	size_t data_len;
	dma_addr_t data_dma_addr;
	dma_addr_t gpd_dma_addr;
	struct sk_buff *skb;
	int (*trb_complete)(struct sk_buff *skb);
};

struct txq {
	struct cldma_hw *hw;
	int txqno;
	int vqno;
	struct virtq *vq;
	struct mutex lock; /* protect structure fields */
	struct work_struct tx_done_work;
	struct tx_req *req_pool;
	int req_pool_size;
	int req_budget;
	int wr_idx;
	int free_idx;
	bool tx_started;
	bool is_stopping;
	unsigned short tx_done_cnt;
};

struct cldma_dev;
struct cldma_hw;

struct cldma_hw_ops {
	u32 hw_ver;
	int (*init)(struct cldma_dev *cd, int hif_id);
	int (*exit)(struct cldma_dev *cd, int hif_id);
	struct txq* (*txq_alloc)(struct cldma_hw *hw, struct sk_buff *skb);
	struct rxq* (*rxq_alloc)(struct cldma_hw *hw, struct sk_buff *skb);
	int (*txq_free)(struct cldma_hw *hw, int vqno);
	int (*rxq_free)(struct cldma_hw *hw, int vqno);
	int (*start_xfer)(struct cldma_hw *hw, int qno);
	void (*suspend)(struct cldma_hw *hw);
	void (*suspend_late)(struct cldma_hw *hw);
	void (*resume)(struct cldma_hw *hw, bool is_runtime);
	void (*fsm_state_listener)(struct mtk_fsm_param *param, struct cldma_hw *hw);
	void (*dump)(struct cldma_hw *hw);
};

struct cldma_hw {
	int hif_id;
	int base_addr;
	int pci_ext_irq_id;
	struct mtk_md_dev *mdev;
	struct cldma_dev *cd;
	struct txq *txq[HW_QUEUE_NUM];
	struct rxq *rxq[HW_QUEUE_NUM];
	struct dma_pool *dma_pool;
	struct workqueue_struct *wq;
	struct dentry *dentry;
	atomic_t irq_enable;
};

struct cldma_dev {
	struct cldma_hw *cldma_hw[NR_CLDMA];
	struct mtk_ctrl_trans *trans;
	struct cldma_hw_ops *hw_ops;
};

extern struct hif_ops cldma_ops;

struct cldma_hw_ops_desc {
	u32 hw_ver;
	struct cldma_hw_ops *hw_ops;
};

#define hw_ops_name(NAME) cldma_hw_ops_##NAME

#endif
