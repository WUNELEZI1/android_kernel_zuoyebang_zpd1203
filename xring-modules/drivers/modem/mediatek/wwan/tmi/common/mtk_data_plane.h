/* SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Copyright (c) 2022, MediaTek Inc.
 */

#ifndef __MTK_DATA_PLANE_H__
#define __MTK_DATA_PLANE_H__

#include <linux/bitops.h>
#include <linux/completion.h>
#include <linux/skbuff.h>

#define DATA_NETWORK_TYPE_MAX 7
#define SKB_TO_CMD(skb) ((struct mtk_data_cmd *)(skb)->data)
#define CMD_TO_DATA(cmd) (*(void **)(cmd)->data)
#define SKB_TO_CMD_DATA(skb) (*(void **)SKB_TO_CMD(skb)->data)

#define IPV4_VERSION 0x40
#define IPV6_VERSION 0x60

enum mtk_data_err_event {
	DATA_LINK_ERR = 0,
	DATA_HW_CHECK_ERR = 1,
	DATA_SW_CHECK_ERR = 2,
};

enum dpmaif_pkt_priority {
	PRIORITY_0 = 0, /* lowest priority, Normal pkt */
	PRIORITY_1 = 1, /* lower priority, Echo pkt */
	PRIORITY_2 = 2, /* higher priority, ACK pkt */
	PRIORITY_3 = 3, /* highest priority, IMS pkt */
	PKT_PRIO_MAX,
	PKT_PRIO_INVALID = PKT_PRIO_MAX
};

enum mtk_data_feature {
	DATA_F_LRO = BIT(0),
	DATA_F_RXFH = BIT(1),
	DATA_F_INTR_COALESCE = BIT(2),
	DATA_F_RXCSUM = BIT(3),
	DATA_F_TXCSUM = BIT(4),
	DATA_F_MULTI_NETDEV = BIT(16),
	DATA_F_ETH_PDN = BIT(17),
};

struct mtk_data_blk {
	struct mtk_md_dev *mdev;
	struct mtk_wwan_ctlb *wcb;
	struct mtk_dpmaif_ctlb *dcb;
};

enum mtk_data_type {
	DATA_PKT,
	DATA_CMD,
};

enum mtk_pkt_type {
	ETH_802_3,
	ETH_802_1Q,
	PURE_IP,
};

enum mtk_data_cmd_type {
	DATA_CMD_TRANS_CTL,
	DATA_CMD_RXFH_GET,
	DATA_CMD_RXFH_SET,
	DATA_CMD_TRANS_DUMP,
	DATA_CMD_RXQ_NUM_GET,
	DATA_CMD_HKEY_SIZE_GET,
	DATA_CMD_INDIR_SIZE_GET,
	DATA_CMD_INTR_COALESCE_GET,
	DATA_CMD_INTR_COALESCE_SET,
	DATA_CMD_STRING_CNT_GET,
	DATA_CMD_STRING_GET,
	DATA_CMD_LRO_SET,
	DATA_CMD_RXCSUM_SET,
	DATA_CMD_TXCSUM_SET
};

struct mtk_data_intr_coalesce {
	unsigned int rx_coalesce_usecs;
	unsigned int tx_coalesce_usecs;
	unsigned int rx_coalesced_frames;
	unsigned int tx_coalesced_frames;
};

struct mtk_data_rxfh {
	unsigned int *indir;
	u8 *key;
};

struct mtk_data_trans_ctl {
	bool enable;
};

struct mtk_data_cmd {
	void (*data_complete)(void *data);
	struct completion done;
	int ret;
	enum mtk_data_cmd_type cmd;
	unsigned int len;
	char data[];
};

struct mtk_tx_pkt_info {
	unsigned char intf_id;
	unsigned char drb_cnt;
	unsigned char network_type;
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	bool in_tcp_slow_start;
#endif
};

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
struct mtk_rx_pkt_info {
	unsigned char ch_id;
};
#endif

union mtk_data_pkt_info {
	struct mtk_tx_pkt_info tx;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	struct mtk_rx_pkt_info rx;
#endif
};

#define DPMAIF_SKB_CB(__skb) ((union mtk_data_pkt_info *)&((__skb)->cb[0]))

struct mtk_data_trans_ops {
	int (*poll)(struct napi_struct *napi, int budget);
	int (*select_txq)(struct mtk_data_blk *data_blk, struct sk_buff *skb,
			  enum mtk_pkt_type pkt_type, enum dpmaif_pkt_priority pkt_prio);
	int (*send)(struct mtk_data_blk *data_blk, enum mtk_data_type type,
		    struct sk_buff *skb, u64 data);
};

enum mtk_data_evt {
	DATA_EVT_MIN,
	DATA_EVT_TX_START,
	DATA_EVT_TX_STOP,
	DATA_EVT_RX_START,
	DATA_EVT_RX_STOP,
	DATA_EVT_RX_SYNC,
#ifdef CONFIG_MTK_ENABLE_WQ_GRO
	DATA_EVT_RX_FLUSH,
#endif
	DATA_EVT_REG_DEV,
	DATA_EVT_UNREG_DEV,
	DATA_EVT_DUMP,
	DATA_EVT_MAX,
};

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
/* DL queue number cannot be greater than this value */
#define MTK_DATA_NAPI_NR_MAX (2)
struct mtk_data_cpu_affinity_cfg {
	u8 napi_thrd_aff[MTK_DATA_NAPI_NR_MAX];
	u8 steer_wq_aff[MTK_DATA_NAPI_NR_MAX];
	u32 online_cpus;
};
#endif

struct mtk_data_trans_info {
	u32 cap;
	unsigned char rxq_cnt;
	unsigned char txq_cnt;
	unsigned int max_mtu;
	struct napi_struct **napis;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	struct mtk_data_cpu_affinity_cfg  *aff_cfg;
#endif
};

struct mtk_data_port_ops {
	int (*init)(struct mtk_data_blk *data_blk, struct mtk_data_trans_info *trans_info);
	void (*exit)(struct mtk_data_blk *data_blk);
	int (*recv)(struct mtk_data_blk *data_blk, struct sk_buff *skb,
		    unsigned char q_id, unsigned char if_id);
	void (*notify)(struct mtk_data_blk *data_blk, enum mtk_data_evt evt, u64 data);
};

void mtk_ethtool_set_ops(struct net_device *dev);
int mtk_wwan_cmd_execute(struct net_device *dev, enum mtk_data_cmd_type cmd, void *data);
int mtk_data_init(struct mtk_md_dev *mdev);
int mtk_data_exit(struct mtk_md_dev *mdev);

extern struct mtk_data_port_ops data_port_ops;
extern struct mtk_data_trans_ops data_trans_ops;

#endif /* __MTK_DATA_PLANE_H__ */
