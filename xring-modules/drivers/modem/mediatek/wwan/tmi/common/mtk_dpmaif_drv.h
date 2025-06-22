/* SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Copyright (c) 2022, MediaTek Inc.
 */

#ifndef __MTK_DPMAIF_DRV_H__
#define __MTK_DPMAIF_DRV_H__

#include <linux/bitops.h>

enum dpmaif_drv_dir {
	DPMAIF_TX,
	DPMAIF_RX,
};

struct dpmaif_drv_pkt_info {
	unsigned char prio;
	__u32 skb_hash;
};

enum mtk_data_hw_feature_type {
	DATA_HW_F_LRO = BIT(0),
	DATA_HW_F_INDR_TBL = BIT(1),
	DATA_HW_F_INTR_COALESCE = BIT(2),
	DATA_HW_F_FRAG = BIT(3),
	DATA_HW_F_RXCSUM = BIT(4),
	DATA_HW_F_TXCSUM = BIT(5),
	DATA_HW_F_TRAS_ALIGN = BIT(6),
};

enum dpmaif_drv_cmd {
	DATA_HW_INTR_COALESCE_SET,
	DATA_HW_HASH_GET,
	DATA_HW_HASH_SET,
	DATA_HW_HASH_KEY_SIZE_GET,
	DATA_HW_INDIR_GET,
	DATA_HW_INDIR_SET,
	DATA_HW_INDIR_SIZE_GET,
	DATA_HW_LRO_SET,
	DATA_HW_RXCSUM_SET,
	DATA_HW_TXCSUM_SET,
	DATA_HW_FEATURES_GET,
	DATA_HW_CFG_GET,
	DATA_HW_TXQ_GET,
};

struct dpmaif_drv_intr {
	enum dpmaif_drv_dir dir;
	unsigned int q_mask;
	unsigned int mode;
	unsigned int pkt_threshold;
	unsigned int time_threshold;
};

struct dpmaif_hpc_rule {
	unsigned int type:4;
	unsigned int flow_lab:20; /* only use for ipv6 */
	unsigned int hop_lim:8; /* only use for ipv6 */
	unsigned short src_port;
	unsigned short dst_port;
	union{
		struct{
			unsigned int v4src_addr;
			unsigned int v4dst_addr;
			unsigned int resv[6];
		};
		struct{
			unsigned int v6src_addr3;
			unsigned int v6dst_addr3;
			unsigned int v6src_addr0;
			unsigned int v6src_addr1;
			unsigned int v6src_addr2;
			unsigned int v6dst_addr0;
			unsigned int v6dst_addr1;
			unsigned int v6dst_addr2;
		};
	};

#define TCP_OPTION_MAX_LEN	10
	unsigned int tcp_opt[TCP_OPTION_MAX_LEN];
};

enum mtk_drv_err {
	DATA_ERR_STOP_MAX = 10,
	DATA_HW_REG_TIMEOUT,
	DATA_HW_REG_CHK_FAIL,
	DATA_FLOW_CHK_ERR,
	DATA_DMA_MAP_ERR,
	DATA_DL_ONCE_MORE,
	DATA_PIT_SEQ_CHK_FAIL,
	DATA_LOW_MEM_TYPE_MAX,
	DATA_LOW_MEM_DRB,
	DATA_LOW_MEM_BAT,
	DATA_LOW_MEM_PIT,
	DATA_LOW_MEM_SKB,
};

#define DPMAIF_RXQ_CNT_MAX 2
#define DPMAIF_TXQ_CNT_MAX 5
#define DPMAIF_IRQ_CNT_MAX 3

#define DPMAIF_PIT_SEQ_MAX 251

#define DPMAIF_HW_PKT_ALIGN	64
#define DPMAIF_HW_BAT_RSVLEN	0

enum {
	DPMAIF_CLEAR_INTR,
	DPMAIF_UNMASK_INTR
};

enum dpmaif_drv_dlq_id {
	DPMAIF_DLQ0 = 0,
	DPMAIF_DLQ1
};

struct dpmaif_drv_dlq {
	bool q_started;
	dma_addr_t pit_base;
	u32 pit_size;
};

struct dpmaif_drv_ulq {
	bool q_started;
	dma_addr_t drb_base;
	u32 drb_size;
};

struct dpmaif_drv_data_ring {
	dma_addr_t normal_bat_base;
	u32 normal_bat_size;
	dma_addr_t frag_bat_base;
	u32 frag_bat_size;
	u32 normal_bat_remain_size;
	u32 normal_bat_pkt_bufsz;
	u32 frag_bat_pkt_bufsz;
	u32 normal_bat_rsv_length;
	u32 pkt_bid_max_cnt;
	u32 pkt_alignment;
	u32 mtu;
	u32 chk_pit_num;
	u32 chk_normal_bat_num;
	u32 chk_frag_bat_num;
};

struct dpmaif_drv_property {
	u32 features;
	struct dpmaif_drv_dlq dlq[DPMAIF_RXQ_CNT_MAX];
	struct dpmaif_drv_ulq ulq[DPMAIF_TXQ_CNT_MAX];
	struct dpmaif_drv_data_ring ring;
};

enum dpmaif_drv_ring_type {
	DPMAIF_PIT,
	DPMAIF_BAT,
	DPMAIF_FRAG,
	DPMAIF_DRB
};

enum dpmaif_drv_ring_idx {
	DPMAIF_PIT_WIDX,
	DPMAIF_PIT_RIDX,
	DPMAIF_BAT_WIDX,
	DPMAIF_BAT_RIDX,
	DPMAIF_FRAG_WIDX,
	DPMAIF_FRAG_RIDX,
	DPMAIF_DRB_WIDX,
	DPMAIF_DRB_RIDX
};

struct dpmaif_drv_irq_en_mask {
	u32 ap_ul_l2intr_en_mask;
	u32 ap_dl_l2intr_en_mask;
	u32 ap_udl_ip_busy_en_mask;
};

struct dpmaif_drv_info {
	struct mtk_md_dev *mdev;
	bool ulq_all_enable, dlq_all_enable;
	struct dpmaif_drv_property drv_property;
	struct dpmaif_drv_irq_en_mask drv_irq_en_mask;
	struct dpmaif_drv_ops *drv_ops;
};

struct dpmaif_drv_cfg {
	dma_addr_t drb_base[DPMAIF_TXQ_CNT_MAX];
	u32 drb_cnt[DPMAIF_TXQ_CNT_MAX];
	dma_addr_t pit_base[DPMAIF_RXQ_CNT_MAX];
	u32 pit_cnt[DPMAIF_RXQ_CNT_MAX];
	dma_addr_t normal_bat_base;
	u32 normal_bat_cnt;
	dma_addr_t frag_bat_base;
	u32 frag_bat_cnt;
	u32 normal_bat_buf_size;
	u32 frag_bat_buf_size;
	u32 max_mtu;
	u32 features;
};

enum dpmaif_drv_intr_type {
	DPMAIF_INTR_MIN = 0,
	/* uplink part */
	DPMAIF_INTR_UL_DONE,
	DPMAIF_INTR_UL_DRB_EMPTY,
	DPMAIF_INTR_UL_MD_NOTREADY,
	DPMAIF_INTR_UL_MD_PWR_NOTREADY,
	DPMAIF_INTR_UL_LEN_ERR,

	/* downlink part */
	DPMAIF_INTR_DL_LEGACY_DONE,
	DPMAIF_INTR_DL_SKB_LEN_ERR,
	DPMAIF_INTR_DL_BATCNT_LEN_ERR,
	DPMAIF_INTR_DL_PKT_EMPTY_SET,
	DPMAIF_INTR_DL_FRG_EMPTY_SET,
	DPMAIF_INTR_DL_MTU_ERR,
	DPMAIF_INTR_DL_FRGCNT_LEN_ERR,

	DPMAIF_INTR_DL_PITCNT_LEN_ERR,
	DPMAIF_INTR_DL_HPC_ENT_TYPE_ERR,
	DPMAIF_INTR_DL_DONE,

	/* traffic sync */
	DPMAIF_INTR_TRAS_SYNC,
	DPMAIF_INTR_MAX,
};

#define DPMAIF_INTR_COUNT ((DPMAIF_INTR_MAX) - (DPMAIF_INTR_MIN) - 1)

struct dpmaif_drv_intr_info {
	unsigned char intr_cnt;
	enum dpmaif_drv_intr_type intr_types[DPMAIF_INTR_COUNT];
	/* it's a queue mask or queue index */
	u32 intr_queues[DPMAIF_INTR_COUNT];
};

/* software resource structure */
#define DPMAIF_SRV_CNT_MAX DPMAIF_TXQ_CNT_MAX

/* struct dpmaif_res_cfg - dpmaif resource configuration
 * @tx_srv_cnt: Transmit services count.
 * @tx_vq_cnt: Transmit virtual queue count.
 * @tx_vq_srv_map: Transmit virtual queue and service map.
 * Array index indicates virtual queue id, Array value indicates service id.
 * @srv_prio_tbl: Transmit services priority
 * Array index indicates service id, Array value indicates kthread nice value.
 * @irq_cnt: Dpmaif interrupt source count.
 * @irq_src: Dpmaif interrupt source id.
 * @txq_cnt: Dpmaif Transmit queue count.
 * @rxq_cnt: Dpmaif Receive queue count.
 * @normal_bat_cnt: Dpmaif normal bat entry count.
 * @frag_bat_cnt: Dpmaif frag bat entry count.
 * @pit_cnt: Dpmaif pit entry count per receive queue.
 * @drb_cnt: Dpmaif drb entry count per transmit queue.
 * @cap: Dpmaif capability.
 */
struct dpmaif_res_cfg {
	unsigned char tx_srv_cnt;
	unsigned char tx_vq_cnt;
	unsigned char tx_vq_srv_map[DPMAIF_TXQ_CNT_MAX];
	int srv_prio_tbl[DPMAIF_SRV_CNT_MAX];
	unsigned int txq_doorbell_delay[DPMAIF_TXQ_CNT_MAX];
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	unsigned int txq_burst_pkts[DPMAIF_TXQ_CNT_MAX];
#endif
	unsigned char irq_cnt;
	enum mtk_irq_src irq_src[DPMAIF_IRQ_CNT_MAX];
	unsigned char txq_cnt;
	unsigned char rxq_cnt;
	unsigned int normal_bat_cnt;
	unsigned int frag_bat_cnt;
	unsigned int pit_cnt[DPMAIF_RXQ_CNT_MAX];
	unsigned int drb_cnt[DPMAIF_TXQ_CNT_MAX];
	unsigned int cap;
};

/* This structure defines the management hooks for dpmaif devices. */
struct dpmaif_drv_ops {
	/* Initialize dpmaif hardware. */
	int (*init)(struct dpmaif_drv_info *drv_info, void *data);
	/* Start dpmaif hardware transaction and unmask dpmaif interrupt. */
	int (*start_queue)(struct dpmaif_drv_info *drv_info, enum dpmaif_drv_dir dir);
	/* Stop dpmaif hardware transaction and mask dpmaif interrupt. */
	int (*stop_queue)(struct dpmaif_drv_info *drv_info, enum dpmaif_drv_dir dir);
	/* Check, mask and clear the dpmaif interrupts,
	 * and then, collect interrupt information for data plane transaction layer.
	 */
	int (*intr_handle)(struct dpmaif_drv_info *drv_info, void *data, u8 irq_id);
	/* Unmask or clear dpmaif interrupt. */
	int (*intr_complete)(struct dpmaif_drv_info *drv_info, enum dpmaif_drv_intr_type type,
			     u8 q_id, u64 data);
	int (*send_doorbell)(struct dpmaif_drv_info *drv_info, enum dpmaif_drv_ring_type type,
			     u8 q_id, u32 cnt);
	int (*get_ring_idx)(struct dpmaif_drv_info *drv_info, enum dpmaif_drv_ring_idx index,
			    u8 q_id);
	int (*feature_cmd)(struct dpmaif_drv_info *drv_info, enum dpmaif_drv_cmd cmd, void *data);
	void (*dump)(struct dpmaif_drv_info *drv_info);
};

static inline int mtk_dpmaif_drv_init(struct dpmaif_drv_info *drv_info, void *data)
{
	return drv_info->drv_ops->init(drv_info, data);
}

static inline int mtk_dpmaif_drv_start_queue(struct dpmaif_drv_info *drv_info,
					     enum dpmaif_drv_dir dir)
{
	return drv_info->drv_ops->start_queue(drv_info, dir);
}

static inline int mtk_dpmaif_drv_stop_queue(struct dpmaif_drv_info *drv_info,
					    enum dpmaif_drv_dir dir)
{
	return drv_info->drv_ops->stop_queue(drv_info, dir);
}

static inline int mtk_dpmaif_drv_intr_handle(struct dpmaif_drv_info *drv_info,
					     void *data, u8 irq_id)
{
	trace_mtk_irq_entry(irq_id);
	return drv_info->drv_ops->intr_handle(drv_info, data, irq_id);
}

static inline int mtk_dpmaif_drv_intr_complete(struct dpmaif_drv_info *drv_info,
					       enum dpmaif_drv_intr_type type, u8 q_id, u64 data)
{
	trace_mtk_irq_exit(type, q_id);
	return drv_info->drv_ops->intr_complete(drv_info, type, q_id, data);
}

static inline int mtk_dpmaif_drv_send_doorbell(struct dpmaif_drv_info *drv_info,
					       enum dpmaif_drv_ring_type type, u8 q_id, u32 cnt)
{
	trace_mtk_data_doorbell(type, q_id, cnt);
	return drv_info->drv_ops->send_doorbell(drv_info, type, q_id, cnt);
}

static inline int mtk_dpmaif_drv_get_ring_idx(struct dpmaif_drv_info *drv_info,
					      enum dpmaif_drv_ring_idx index, u8 q_id)
{
	return drv_info->drv_ops->get_ring_idx(drv_info, index, q_id);
}

static inline int mtk_dpmaif_drv_feature_cmd(struct dpmaif_drv_info *drv_info,
					     enum dpmaif_drv_cmd cmd, void *data)
{
	return drv_info->drv_ops->feature_cmd(drv_info, cmd, data);
}

static inline void dpmaif_drv_dump(struct dpmaif_drv_info *drv_info)
{
	return drv_info->drv_ops->dump(drv_info);
}

struct dpmaif_drv_ops_desc {
	u32 hw_ver;
	struct dpmaif_drv_ops *drv_ops;
};

#define drv_ops_name(NAME) dpmaif_drv_ops_##NAME

#define MTK_DATA_IRQ_MEMLOG_RG(irq_id)  ((MTK_MEMLOG_RG_6) + (irq_id))
#define MTK_DATA_RX_MEMLOG_RG(rxq_id)  ((MTK_MEMLOG_RG_2) + (rxq_id))
#endif
