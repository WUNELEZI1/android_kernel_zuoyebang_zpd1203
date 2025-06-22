// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#define pr_fmt(fmt) "DATA_TRANS: " fmt
#define XIAOMI_DPMAIF_MEMORY_OPTI

#include <linux/bitfield.h>
#include <linux/dma-direct.h>
#include <linux/hrtimer.h>
#include <linux/icmp.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/kthread.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/pm_wakeup.h>
#include <linux/skbuff.h>
#include <linux/tcp.h>
#include <linux/hrtimer.h>
#include <linux/sched/clock.h>
#include <linux/pci.h>
#include <linux/pm_runtime.h>
#include <net/ipv6.h>
#include <net/pkt_sched.h>
#include <linux/rcupdate.h>
#include <linux/tcp.h>

#include "mtk_bm.h"
#include "mtk_data_plane.h"
#include "mtk_dev.h"
#include "mtk_debug.h"
#include "mtk_debugfs.h"
#include "mtk_dpmaif_drv.h"
#include "mtk_fsm.h"
#include "mtk_reg.h"
#include "mtk_dpmaif_drv_interface.h"
#ifdef CONFIG_TX00_UT_DPMAIF
#include "ut_dpmaif_fake.h"
#endif

#define TAG "DATA_TRANS"

#define DPMAIF_PIT_CNT_UPDATE_THRESHOLD 60
#define DPMAIF_SKB_TX_WEIGHT		32
#define DPMAIF_WS_NAME_LEN		32

/* Interrupt coalesce default value */
#define DPMAIF_DFLT_INTR_RX_COA_FRAMES 0
#define DPMAIF_DFLT_INTR_TX_COA_FRAMES 0
#define DPMAIF_DFLT_INTR_RX_COA_USECS 0
#define DPMAIF_DFLT_INTR_TX_COA_USECS 0
#define DPMAIF_INTR_EN_TIME BIT(0)
#define DPMAIF_INTR_EN_PKT BIT(1)

#define DPMAIF_DUMP_DRB_CNT 25
#define DPMAIF_DUMP_PIT_CNT 25
#define DPMAIF_DUMP_BAT_CNT 25

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
#define DPMAIF_RING_TPUT_CALC(CNT, MS_SHIFT)	((CNT) >> (MS_SHIFT))
#endif

extern int mtk_except_linkerr_type;
void *xiaomi_kcalloc_dpmaif_memory(size_t cout, size_t size);
void xiaomi_free_dpmaif_memory(void *ptr);

#ifdef CONFIG_DATA_TEST_MODE
/* Test mode */
/* MD Tput setting Mask. */
#define DPMAIF_MD_TPUT_MODE_SET_MASK BIT(31)
#define DPMAIF_MD_TPUT_CTL_SET_MASK BIT(30)
#define DPMAIF_MD_TPUT_PKT_SET_MASK BIT(29)
#define DPMAIF_MD_TPUT_TIME_SET_MASK BIT(28)

/*  MD TPUT mode [bit12-15]. */
#define DPMAIF_MD_TPUT_MODE_OFFSET 12
#define DPMAIF_MD_TPUT_MODE_MASK 0x0F
#define DPMAIF_MD_INVALID_MODE 0x0F

/* Start/stop MD DL [bit23]. */
#define DPMAIF_MD_DL_CTL_OFFSET 23
#define DPMAIF_MD_DL_CTL_MASK 0x01

/* pkt_count/ms mask [bit16-22]. */
#define DPMAIF_MD_PKT_OFFSET 16
#define DPMAIF_MD_PKT_MASK 0x7F

/* MD DL test time [bit24-26]. */
#define DPMAIF_MD_DL_TIME_OFFSET 24
#define DPMAIF_MD_DL_TIME_MASK 0x7

struct dpmaif_test_mode_cfg {
	unsigned char md_tput_mode;
	unsigned char md_start_dl_tput;
	unsigned int md_pkt_number_per_ms;
	unsigned char md_dl_tput_test_time; /*1 -> 10s*/
};

enum dpmaif_tx_rec_point {
	TX_REC_SW_DONE,
	TX_REC_HW_DONE
};

struct dpmaif_tput_stats {
	bool ul_tput_monitor_enabled;
	bool dl_tput_monitor_enabled;

	/*
	 * Two different recording points:
	 * TX_REC_SW_DONE, records tx packets number tx_num[0] after
	 * DRB updated;
	 * TX_REC_HW_DONE, records tx packets number tx_num[1] after
	 * tx_done interrupts.
	 */
	u64 tx_num[TX_REC_HW_DONE + 1];
	bool tx_rec_enabled;
	bool first_tx_pkt;
	u64 tx_rec_start_tm;

	u64 rx_num;
	u64 rx_rec_start_tm;
	bool rx_rec_enabled;
	bool first_rx_pkt;
};

#endif

/* Dpmaif hardware DMA descriptor structure. */
enum dpmaif_rcsum_state {
	CS_RESULT_INVALID = -1,
	CS_RESULT_PASS = 0,
	CS_RESULT_FAIL = 1,
	CS_RESULT_NOTSUPP = 2,
	CS_RESULT_RSV = 3
};

struct dpmaif_msg_pit {
	__le32 dword1;
	__le32 dword2;
	__le32 dword3;
	__le32 dword4;
};

#define PIT_MSG_DP		BIT(31) /* Indicates software to drop this packet if set. */
#define PIT_MSG_DW1_RSV1	GENMASK(30, 27)
#define PIT_MSG_NET_TYPE	GENMASK(26, 24)
#define PIT_MSG_CHNL_ID		GENMASK(23, 16) /* channel index */
#define PIT_MSG_DW1_RSV2	GENMASK(15, 12)
#define PIT_MSG_HPC_IDX		GENMASK(11, 8)
#define PIT_MSG_SRC_QID		GENMASK(7, 5)
#define PIT_MSG_ERR		BIT(4)
#define PIT_MSG_CHECKSUM	GENMASK(3, 2)
#define PIT_MSG_CONT		BIT(1) /* 0b: last entry; 1b: more entry */
#define PIT_MSG_PKT_TYPE	BIT(0) /* 0b: normal PIT entry; 1b: message PIT entry */

#define PIT_MSG_HP_IDX		GENMASK(31, 27)
#define PIT_MSG_CMD		GENMASK(26, 24)
#define PIT_MSG_DW2_RSV		GENMASK(23, 21)
#define PIT_MSG_FLOW		GENMASK(20, 16)
#define PIT_MSG_COUNT_L		GENMASK(15, 0)

#define PIT_MSG_HASH		GENMASK(31, 24) /* Hash value calculated by Hardware using packet */
#define PIT_MSG_DW3_RSV1	GENMASK(23, 18)
#define PIT_MSG_PRO		GENMASK(17, 16)
#define PIT_MSG_VBID		GENMASK(15, 3)
#define PIT_MSG_DW3_RSV2	GENMASK(2, 0)

#define PIT_MSG_DLQ_DONE	GENMASK(31, 30)
#define PIT_MSG_ULQ_DONE	GENMASK(29, 24)
#define PIT_MSG_IP		BIT(23)
#define PIT_MSG_DW4_RSV1	BIT(22)
#define PIT_MSG_MR		GENMASK(21, 20)
#define PIT_MSG_DW4_RSV2	GENMASK(19, 17)
#define PIT_MSG_IG		BIT(16)
#define PIT_MSG_DW4_RSV3	GENMASK(15, 11)
#define PIT_MSG_H_BID		GENMASK(10, 8)
/* An incremental number for each PIT, updated for each PIT entries.
 * It is reset to 0 when its value reaches the maximum value.
 */
#define PIT_MSG_PIT_SEQ		GENMASK(7, 0)

/* c_bit */
#define DPMAIF_PIT_LASTONE	0x00
#define DPMAIF_PIT_MORE		0x01

/* pit type */
enum dpmaif_pit_type {
	PD_PIT = 0,
	MSG_PIT,
};

/* buffer type */
enum dpmaif_bat_type {
	NORMAL_BAT = 0,
	FRAG_BAT = 1,
};

struct dpmaif_pd_pit {
	__le32 pd_header;
	__le32 addr_low;
	__le32 addr_high;
	__le32 pd_footer;
};

#define PIT_PD_DATA_LEN		GENMASK(31, 16) /* Indicates the data length of current packet. */
#define PIT_PD_BUF_ID		GENMASK(15, 3) /* The low order of buffer index */
#define PIT_PD_BUF_TYPE		BIT(2) /* 0b: normal BAT entry; 1b: fragment BAT entry */
#define PIT_PD_CONT		BIT(1) /* 0b: last entry; 1b: more entry */
#define PIT_PD_PKT_TYPE		BIT(0) /* 0b: normal PIT entry; 1b: message PIT entry */

#define PIT_PD_DLQ_DONE		GENMASK(31, 30)
#define PIT_PD_ULQ_DONE		GENMASK(29, 24)
/* The header length of transport layer and internet layer. */
#define PIT_PD_HD_OFFSET	GENMASK(23, 19)
#define PIT_PD_BI_F		GENMASK(18, 17)
#define PIT_PD_IG		BIT(16)
#define PIT_PD_RSV		GENMASK(15, 11)
#define PIT_PD_H_BID		GENMASK(10, 8) /* The high order of buffer index */
#define PIT_PD_SEQ		GENMASK(7, 0) /* PIT sequence */

/* RX: buffer address table */
struct dpmaif_bat {
	__le32 buf_addr_low;
	__le32 buf_addr_high;
};

/* drb->type */
enum dpmaif_drb_type {
	PD_DRB,
	MSG_DRB,
};

#define DPMAIF_DRB_LASTONE	0x00
#define DPMAIF_DRB_MORE		0x01

struct dpmaif_msg_drb {
	__le32 msg_header1;
	__le32 msg_header2;
	__le32 msg_rsv1;
	__le32 msg_rsv2;
};

#define DRB_MSG_PKT_LEN		GENMASK(31, 16) /* The length of a whole packet. */
#define DRB_MSG_DW1_RSV		GENMASK(15, 3)
#define DRB_MSG_CONT		BIT(2) /* 0b: last entry; 1b: more entry */
#define DRB_MSG_DTYP		GENMASK(1, 0) /* 00b: normal DRB entry; 01b: message DRB entry */

#define DRB_MSG_DW2_RSV1	GENMASK(31, 30)
#define DRB_MSG_L4_CHK		BIT(29) /* 0b: disable layer4 checksum offload; 1b: enable */
#define DRB_MSG_IP_CHK		BIT(28) /* 0b: disable IP checksum, 1b: enable IP checksum */
#define DRB_MSG_DW2_RSV2	BIT(27)
#define DRB_MSG_NET_TYPE	GENMASK(26, 24)
#define DRB_MSG_CHNL_ID		GENMASK(23, 16) /* channel index */
#define DRB_MSG_COUNT_L		GENMASK(15, 0)

struct dpmaif_pd_drb {
	__le32 pd_header;
	__le32 addr_low;
	__le32 addr_high;
	__le32 pd_rsv;
};

#define DRB_PD_DATA_LEN		GENMASK(31, 16) /* the length of a payload. */
#define DRB_PD_RSV		GENMASK(15, 3)
#define DRB_PD_CONT		BIT(2)/* 0b: last entry; 1b: more entry */
#define DRB_PD_DTYP		GENMASK(1, 0) /* 00b: normal DRB entry; 01b: message DRB entry. */

enum dpmaif_state {
	DPMAIF_STATE_MIN,
	DPMAIF_STATE_PWROFF,
	DPMAIF_STATE_PWRON,
	DPMAIF_STATE_MAX
};

struct dpmaif_vq {
	unsigned char q_id;
	u32 max_len; /* align network tx qdisc 1000 */
	struct sk_buff_head list;
};

struct dpmaif_cmd_srv {
	struct mtk_dpmaif_ctlb *dcb;
	struct work_struct work;
	struct dpmaif_vq *vq;
};

struct dpmaif_tx_srv {
	struct mtk_dpmaif_ctlb *dcb;
	unsigned char id;
	int prio;
	wait_queue_head_t wait;
	struct task_struct *srv;

	unsigned long txq_drb_lack_sta;
	unsigned char cur_vq_id;
	unsigned char vq_cnt;
	struct dpmaif_vq *vq[DPMAIF_TXQ_CNT_MAX];
};

struct dpmaif_drb_skb {
	struct sk_buff *skb;
	dma_addr_t data_dma_addr;
	unsigned short data_len;
	unsigned short drb_idx:13;
	unsigned short is_msg:1;
	unsigned short is_frag:1;
	unsigned short is_last:1;
};

struct dpmaif_txq {
	struct mtk_dpmaif_ctlb *dcb;
	unsigned char id;
	atomic_t budget;
	atomic_t to_submit_cnt;
	struct dpmaif_pd_drb *drb_base;
	dma_addr_t drb_dma_addr;
	unsigned int drb_cnt;
	unsigned short drb_wr_idx;
	unsigned short drb_rd_idx;
	unsigned short drb_rel_rd_idx;
	unsigned long long dma_map_errs;
	unsigned short last_ch_id;
	struct dpmaif_drb_skb *sw_drb_base;
	unsigned int doorbell_delay;
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	unsigned int exit_tcp_ss_counter;
	unsigned int burst_submit_cnt;
	bool drb_poll_enable;
	atomic_t drb_stats;
#else
	struct delayed_work doorbell_work;
#endif
	struct delayed_work tx_done_work;
	unsigned int intr_coalesce_frame;
};

struct dpmaif_rx_record {
	bool msg_pit_recv;
	struct sk_buff *cur_skb;
	struct sk_buff *lro_parent;
	struct sk_buff *lro_last_skb;
	unsigned int lro_pkt_cnt;
	unsigned int cur_ch_id;
	unsigned int checksum;
	unsigned int hash;
	unsigned char pit_dp;
	unsigned char err_payload;
};

struct dpmaif_rxq {
	struct mtk_dpmaif_ctlb *dcb;
	unsigned char id;
	bool started;
	struct dpmaif_pd_pit *pit_base;
	dma_addr_t pit_dma_addr;
	unsigned int pit_cnt;
	unsigned short pit_wr_idx;
	unsigned short pit_rd_idx;
	unsigned short pit_rel_rd_idx;
	unsigned char pit_seq_expect;
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	bool pit_poll_enable;
	atomic_t pit_rel_cnt;
	atomic_t pit_stats;
#else
	unsigned int pit_rel_cnt;
#endif
	bool pit_cnt_err_intr_set;
	unsigned int pit_burst_rel_cnt;
	unsigned int pit_seq_fail_cnt;
	struct napi_struct napi;
	struct dpmaif_rx_record rx_record;
	unsigned int intr_coalesce_frame;
	/* Record the latest BID polled by this DLQ pit ring. */
	unsigned int pit_bid;
	struct  wakeup_source *ws;

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	struct work_struct steer_work;
	struct workqueue_struct *steer_wq;
	int cpu_id;
	struct dpmaif_vq steer_vq;
	struct sk_buff_head list;
#endif
} ____cacheline_aligned;

struct skb_mapped_t {
	struct sk_buff *skb;
	dma_addr_t data_dma_addr;
	unsigned int data_len;
};

struct page_mapped_t {
	struct page *page;
	dma_addr_t data_dma_addr;
	unsigned int offset;
	unsigned int data_len;
};

union dpmaif_bat_record {
	struct skb_mapped_t normal;
	struct page_mapped_t frag;
};

struct dpmaif_bat_ring {
	enum dpmaif_bat_type type;
	struct dpmaif_bat *bat_base;
	dma_addr_t bat_dma_addr;
	unsigned int bat_cnt;
	unsigned short bat_wr_idx;
	unsigned short bat_rd_idx;
	unsigned short bat_rel_rd_idx;
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	atomic_t bat_rel_cnt;
#endif
	union dpmaif_bat_record *sw_record_base;
	unsigned int buf_size;
	unsigned char *mask_tbl;
	struct work_struct reload_work;
	bool bat_cnt_err_intr_set;
};

struct dpmaif_bat_info {
	struct mtk_dpmaif_ctlb *dcb;
	unsigned int max_mtu;
	bool frag_bat_enabled;

	struct dpmaif_bat_ring normal_bat_ring;
	struct dpmaif_bat_ring frag_bat_ring;

	struct workqueue_struct *reload_wq;
};

struct dpmaif_irq_param {
	unsigned char idx;
	struct mtk_dpmaif_ctlb *dcb;
	enum mtk_irq_src dpmaif_irq_src;
	int dev_irq_id;
};

struct dpmaif_tx_evt {
	unsigned long long ul_done;
	unsigned long long ul_drb_empty;
};

struct dpmaif_rx_evt {
	unsigned long long dl_done;
	unsigned long long pit_len_err;
};

struct dpmaif_other_evt {
	unsigned long long ul_md_notready;
	unsigned long long ul_md_pwr_notready;
	unsigned long long ul_len_err;

	unsigned long long dl_skb_len_err;
	unsigned long long dl_bat_cnt_len_err;
	unsigned long long dl_pkt_empty;
	unsigned long long dl_frag_empty;
	unsigned long long dl_mtu_err;
	unsigned long long dl_frag_cnt_len_err;
	unsigned long long hpc_ent_type_err;
};

struct dpmaif_traffic_stats {
	/* txq traffic */
	unsigned long long tx_sw_packets[DPMAIF_TXQ_CNT_MAX];
	unsigned long long tx_hw_packets[DPMAIF_TXQ_CNT_MAX];
	unsigned long long tx_done_last_time[DPMAIF_TXQ_CNT_MAX];
	unsigned int tx_done_last_cnt[DPMAIF_TXQ_CNT_MAX];

	/* rxq traffic */
	unsigned long long rx_packets[DPMAIF_RXQ_CNT_MAX];
	unsigned long long rx_errors[DPMAIF_RXQ_CNT_MAX];
	unsigned long long rx_dropped[DPMAIF_RXQ_CNT_MAX];
	unsigned long long rx_hw_ind_dropped[DPMAIF_RXQ_CNT_MAX];
	unsigned long long rx_done_last_time[DPMAIF_RXQ_CNT_MAX];
	unsigned int rx_done_last_cnt[DPMAIF_RXQ_CNT_MAX];

	/* irq traffic */
	unsigned long long irq_total_cnt[DPMAIF_IRQ_CNT_MAX];
	unsigned long long irq_last_time[DPMAIF_IRQ_CNT_MAX];
	struct dpmaif_tx_evt irq_tx_evt[DPMAIF_TXQ_CNT_MAX];
	struct dpmaif_rx_evt irq_rx_evt[DPMAIF_RXQ_CNT_MAX];
	struct dpmaif_other_evt irq_other_evt;
};

enum dpmaif_dump_flag {
	DPMAIF_DUMP_TX_PKT = 0,
	DPMAIF_DUMP_RX_PKT,
	DPMAIF_DUMP_DRB,
	DPMAIF_DUMP_PIT
};

/* TRAS event data parse */
#define TRAS_MSG_TYPE		GENMASK(31, 29)
#define TRAS_MSG0_QID		GENMASK(28, 26)
#define TRAS_MSG0_RST		BIT(25)
#define TRAS_MSG0_TIME_TH	GENMASK(24, 16)
#define TRAS_MSG1_QMASK		GENMASK(28, 23)

enum dpmaif_irq_tras_sync_type {
	DPMAIF_TRAS_ULQ_CFG	= 0,
	DPMAIF_TRAS_ULQ_RST	= 1,
};

struct mtk_dpmaif_ctlb {
	struct mtk_data_blk *data_blk;
	struct mtk_data_port_ops *port_ops;
	struct dpmaif_drv_info *drv_info;
	struct mtk_pm_entity pm_entity;
	struct napi_struct *napi[DPMAIF_RXQ_CNT_MAX];

	enum dpmaif_state dpmaif_state;
	bool dpmaif_pm_ready;
	bool dpmaif_user_ready;
	bool dpmaif_suspending;
	bool trans_enabled;
	/* lock for enable/disable routine */
	struct mutex trans_ctl_lock;
	const struct dpmaif_res_cfg *res_cfg;

	struct dpmaif_cmd_srv cmd_srv;
	struct dpmaif_vq cmd_vq;
	struct dpmaif_tx_srv *tx_srvs;
	struct dpmaif_vq *tx_vqs;

	struct workqueue_struct *tx_done_wq;
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	struct workqueue_struct *doorbell_wq;
	struct work_struct doorbell_work;
	/* lock for timer modify */
	struct mutex doorbell_timer_lock;
	struct timer_list ring_rel_ctrl_timer;
	struct hrtimer doorbell_timer;
#else
	struct workqueue_struct *tx_doorbell_wq;
#endif
	struct dpmaif_txq *txqs;
	struct dpmaif_rxq *rxqs;
	struct dpmaif_bat_info bat_info;
	bool irq_enabled;
	struct dpmaif_irq_param *irq_params;
	unsigned long long doorbell_base_ts;

	struct mtk_bm_pool *skb_pool;
	struct mtk_bm_pool *page_pool;

	struct dpmaif_traffic_stats traffic_stats;
	struct mtk_data_intr_coalesce intr_coalesce;
	unsigned long dump_flag;
	atomic_t dump_once;
	unsigned long err_event;

#ifdef CONFIG_DEBUG_FS
	struct dentry *dpmaif_dir;
#endif
#ifdef CONFIG_DATA_TEST_MODE
	struct dpmaif_test_mode_cfg test_mode_cfg;

	struct timer_list traffic_monitor;
	struct dpmaif_tput_stats tput_stats;
#endif
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	struct mtk_data_cpu_affinity_cfg  *aff_cfg;
	u32 online_cpus;
#endif
};

#define DCB_TO_DEV(dcb) ((dcb)->data_blk->mdev->dev)
#define DCB_TO_MDEV(dcb) ((dcb)->data_blk->mdev)
#define DCB_TO_DEV_STR(dcb) ((dcb)->data_blk->mdev->dev_str)
#define DPMAIF_GET_HW_VER(dcb) ((dcb)->data_blk->mdev->hw_ver)
#define DPMAIF_GET_DRB_CNT(__skb) (skb_shinfo(__skb)->nr_frags + 1 + 1)

#define DPMAIF_JUMBO_SIZE 9000
#define DPMAIF_DFLT_MTU 3000
#define DPMAIF_DL_BUF_MIN_SIZE 128
#define DPMAIF_BUF_THRESHOLD (DPMAIF_DL_BUF_MIN_SIZE * 28) /* 3.5k, should be less than page size */
#define DPMAIF_NORMAL_BUF_SIZE_IN_JUMBO (128 * 13) /* 1664 */
#define DPMAIF_FRAG_BUF_SIZE_IN_JUMBO (128 * 15) /* 1920 */

#ifdef CONFIG_REG_ACCESS_OPTIMIZE

static unsigned int traffic_stats_shift = 7;

#define DPMAIF_MAX_UL_TTH	10	/* the time threshold for merge tx packets */
#define DPMAIF_MAX_UL_PTH	(833 * 2)  /* tx packets merge threshold, align 1Gbps * 10ms */

/* the time threshold for normal priority tx queue */
static unsigned int normal_prio_tth = 10;
/* the time threshold for high priority tx queue */
static unsigned int high_prio_tth = 2;
/* the packets count threshold for normal priority tx queue */
static unsigned int normal_prio_pth = 128;
/* the packets count threshold for high priority tx queue */
static unsigned int high_prio_pth = 32;
/* the PIT speed threshold for enable polling register at rx done interrupt bottom-half */
static unsigned int rx_poll_th = 400;
#define MAX_RX_POLL_TH	1024
/* the DRB speed threshold for enable polling reister at tx done interrupt bottom-half */
static unsigned int tx_poll_th = 200;
#define MAX_TX_POLL_TH	512

static unsigned int max_pit_burst_cnt = 1024;

/* Driver stops to delay doorbell when detects TCP slow start,
 * and restarts the doorbell delay after the specified doorbell count expires.
 */
static unsigned int doorbell_reset_count = 500;
#endif

static unsigned int dpmaif_mtu = DPMAIF_DFLT_MTU;

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
static struct mtk_data_cpu_affinity_cfg   data_cpu_loading_cfg[] = {
	{
	.napi_thrd_aff = {0, 1},
	.steer_wq_aff = {0, 1},
	},
	{
	.napi_thrd_aff = {0, 1},
	.steer_wq_aff = {2, 3},
	},
	{
	.napi_thrd_aff = {2, 3},
	.steer_wq_aff = {4, 5},
	},
	{
	.napi_thrd_aff = {0, 0},
	.steer_wq_aff = {0, 0},
	},
};
#endif

static unsigned int mtk_dpmaif_ring_buf_get_next_idx(unsigned int buf_len, unsigned int buf_idx)
{
	return (++buf_idx) % buf_len;
}

static unsigned int mtk_dpmaif_ring_buf_readable(unsigned int total_cnt, unsigned int rd_idx,
						 unsigned int  wr_idx)
{
	unsigned int pkt_cnt;

	if (wr_idx >= rd_idx)
		pkt_cnt = wr_idx - rd_idx;
	else
		pkt_cnt = total_cnt + wr_idx - rd_idx;

	return pkt_cnt;
}

static unsigned int mtk_dpmaif_ring_buf_writable(unsigned int total_cnt, unsigned int rel_idx,
						 unsigned int wr_idx)
{
	unsigned int pkt_cnt;

	if (wr_idx < rel_idx)
		pkt_cnt = rel_idx - wr_idx - 1;
	else
		pkt_cnt = total_cnt + rel_idx - wr_idx - 1;

	return pkt_cnt;
}

static unsigned int mtk_dpmaif_ring_buf_releasable(unsigned int total_cnt, unsigned int rel_idx,
						   unsigned int rd_idx)
{
	unsigned int pkt_cnt;

	if (rel_idx <= rd_idx)
		pkt_cnt = rd_idx - rel_idx;
	else
		pkt_cnt = total_cnt + rd_idx - rel_idx;

	return pkt_cnt;
}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
static int mtk_dpmaif_calc_pit_tput(struct dpmaif_rxq *rxq)
{
	unsigned int tmp_pit_stats = atomic_read(&rxq->pit_stats);

	atomic_sub(tmp_pit_stats, &rxq->pit_stats);

	return DPMAIF_RING_TPUT_CALC(tmp_pit_stats, traffic_stats_shift);
}

static int mtk_dpmaif_calc_drb_tput(struct dpmaif_txq *txq)
{
	unsigned int tmp_drb_stats = atomic_read(&txq->drb_stats);

	atomic_sub(tmp_drb_stats, &txq->drb_stats);

	return DPMAIF_RING_TPUT_CALC(tmp_drb_stats, traffic_stats_shift);
}

/**
 ***************************************************************
 ** pit_speed(pit/ms) *   burst_rel_cnt                       **
 ***************************************************************
 **   [1088, - )      *    64                                 **
 **   [500, 1088)     *   (64, min(137, max_pit_burst_cnt)]   **
 **   [128, 500)      *   (140, min(512, max_pit_burst_cnt)]  **
 **   [32, 128)       *   (512, min(2048, max_pit_burst_cnt)] **
 ***************************************************************
 */
static void mtk_dpmaif_pit_rel_ctrl(struct dpmaif_rxq *rxq, unsigned int pit_speed)
{
	const unsigned int max_update_cnt = max_pit_burst_cnt;
	const unsigned int min_update_cnt = 64;
	unsigned int tmp_pit_burst_rel_cnt;
	bool tmp_pit_poll_enable;

	if (pit_speed > rx_poll_th)
		tmp_pit_poll_enable = true;
	else
		tmp_pit_poll_enable = false;

	if (pit_speed < 128) {
		tmp_pit_burst_rel_cnt = 2560 - (pit_speed << 4);
		goto out;
	}
	if (pit_speed < 500) {
		tmp_pit_burst_rel_cnt = 640 - pit_speed;
		goto out;
	}
	if (pit_speed < 1088) {
		tmp_pit_burst_rel_cnt = 200 - (pit_speed >> 3);
		goto out;
	}

	/* for scenario of pit_speed >= 1088 */
	tmp_pit_burst_rel_cnt = min_update_cnt;

out:
	if (tmp_pit_burst_rel_cnt > max_update_cnt)
		tmp_pit_burst_rel_cnt = max_update_cnt;

	rxq->pit_burst_rel_cnt = tmp_pit_burst_rel_cnt;
	rxq->pit_poll_enable = tmp_pit_poll_enable;
	trace_mtk_data_pit_burst_cnt(rxq->id, rxq->pit_burst_rel_cnt);
}

static void mtk_dpmaif_drb_rel_ctrl(struct dpmaif_txq *txq, unsigned int drb_speed)
{
	if (drb_speed > tx_poll_th)
		txq->drb_poll_enable = true;
	else
		txq->drb_poll_enable = false;
}

static void mtk_dpmaif_ring_rel_ctrl_timer_func(struct timer_list *t)
{
	struct mtk_dpmaif_ctlb *dcb = from_timer(dcb, t, ring_rel_ctrl_timer);
	struct dpmaif_rxq *rxq;
	struct dpmaif_txq *txq;
	unsigned int tmp_tput;
	int i;
/***********************************xiaomi modem tput start ***/
	{
		extern void xiaomi_dltput_calc(int ms);
		static int times = 1;
		int n = 1023/(BIT(traffic_stats_shift)*times);
		if (n <= 0) {
			xiaomi_dltput_calc(times*BIT(traffic_stats_shift));
			times = 1;
		} else {
			times++;
		}
	}
/***********************************xiaomi modem tput end ***/
	if (!dcb || !dcb->res_cfg || !dcb->rxqs || !dcb->txqs) {//added by xiaomi BUGO2S-28648
		return;
	}
	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		tmp_tput = mtk_dpmaif_calc_pit_tput(rxq);
		mtk_dpmaif_pit_rel_ctrl(rxq, tmp_tput);
	}

	for (i = 0; i < dcb->res_cfg->txq_cnt; i++) {
		txq = &dcb->txqs[i];
		tmp_tput = mtk_dpmaif_calc_drb_tput(txq);
		mtk_dpmaif_drb_rel_ctrl(txq, tmp_tput);
	}

	mod_timer(&dcb->ring_rel_ctrl_timer,
		  jiffies + msecs_to_jiffies(BIT(traffic_stats_shift)));
}
#endif

#ifdef CONFIG_DATA_TEST_MODE
static inline void dl_stats_update(struct mtk_dpmaif_ctlb *dcb, unsigned int data_len)
{
	struct dpmaif_tput_stats *tput_stats = &dcb->tput_stats;

	if (tput_stats->dl_tput_monitor_enabled && tput_stats->rx_rec_enabled) {
		if (unlikely(tput_stats->first_rx_pkt)) {
			tput_stats->first_rx_pkt = false;
			tput_stats->rx_rec_start_tm = sched_clock();
		}
		tput_stats->rx_num += data_len;
	}
}

static inline void ul_stats_update(struct mtk_dpmaif_ctlb *dcb,
				   enum dpmaif_tx_rec_point rec_point, unsigned int data_len)
{
	struct dpmaif_tput_stats *tput_stats = &dcb->tput_stats;

	if (tput_stats->ul_tput_monitor_enabled && tput_stats->tx_rec_enabled) {
		if (unlikely(tput_stats->first_tx_pkt)) {
			tput_stats->first_tx_pkt = false;
			tput_stats->tx_rec_start_tm = sched_clock();
		}
		tput_stats->tx_num[rec_point] += data_len;
	}
}

static void tput_calc(struct mtk_dpmaif_ctlb *dcb)
{
	struct dpmaif_tput_stats *tput_stats = &dcb->tput_stats;
	u64 tm_now;

	if (tput_stats->dl_tput_monitor_enabled) {
		if (tput_stats->rx_rec_enabled) {
			tm_now = sched_clock();
			tput_stats->rx_rec_enabled = false;

			MTK_INFO(DCB_TO_MDEV(dcb),
				 "DPMAIF-DL-TPUT,total bytes(%llu),total time(%llu)\n",
				tput_stats->rx_num, tm_now - tput_stats->rx_rec_start_tm);
		}

		/* reset DL tput information */
		tput_stats->rx_num = 0;
		tput_stats->rx_rec_start_tm = 0;
		tput_stats->first_rx_pkt = true;
		tput_stats->rx_rec_enabled = true;
	}

	if (tput_stats->ul_tput_monitor_enabled) {
		if (tput_stats->tx_rec_enabled) {
			tput_stats->tx_rec_enabled = false;
			tm_now = sched_clock();

			MTK_INFO(DCB_TO_MDEV(dcb),
				 "DPMAIF-UL-TPUT,SW done total bytes(%llu),HW done total bytes(%llu),total time(%llu)\n",
				tput_stats->tx_num[TX_REC_SW_DONE],
				tput_stats->tx_num[TX_REC_HW_DONE],
				tm_now - tput_stats->tx_rec_start_tm);
		}

		/* reset UL tput information */
		memset(tput_stats->tx_num, 0x00, sizeof(tput_stats->tx_num));
		tput_stats->tx_rec_start_tm = 0;
		tput_stats->first_tx_pkt = true;
		tput_stats->tx_rec_enabled = true;
	}
}

#define DPMAIF_TRAFFIC_MONITOR_INTERVAL 10
static void dpmaif_traffic_monitor_func(struct timer_list *t)
{
	struct mtk_dpmaif_ctlb *dcb = from_timer(dcb, t, traffic_monitor);

	tput_calc(dcb);

	mod_timer(&dcb->traffic_monitor,
		  jiffies + DPMAIF_TRAFFIC_MONITOR_INTERVAL * HZ);
}

#endif

static void dpmaif_dump_txq_drb_info(struct dpmaif_txq *txq, unsigned int cnt)
{
	unsigned int drb_dump_ridx = txq->drb_rd_idx;
	struct mtk_dpmaif_ctlb *dcb = txq->dcb;
	unsigned int drb_dump_widx;
	unsigned int *drb_info;
	unsigned int drb_idx;
	unsigned int j;

	if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb))) {
		drb_dump_widx = mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
							    DPMAIF_DRB_WIDX, txq->id);
		drb_dump_ridx = mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
							    DPMAIF_DRB_RIDX, txq->id);

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"hw_drb: w= %d, r= %d\n", drb_dump_widx, drb_dump_ridx);
	}

	if (cnt > (txq->drb_cnt / 2))
		cnt = txq->drb_cnt / 2;

	drb_idx = (txq->drb_cnt + drb_dump_ridx - cnt) % txq->drb_cnt;
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"txq(%u),dump drb info,start_idx=%u,rd_idx=%u\n",
		txq->id, drb_idx, drb_dump_ridx);

	if (txq->drb_base) {
		for (j = 0; j < cnt * 2; j++) {
			drb_info = (unsigned int *)(txq->drb_base + drb_idx);
			MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
				"drb(%u): 0x%08x, 0x%08x, 0x%08x,0x%08x\n",
				drb_idx, drb_info[0], drb_info[1], drb_info[2], drb_info[3]);
			drb_idx = mtk_dpmaif_ring_buf_get_next_idx(txq->drb_cnt, drb_idx);
		}
	}
}

static void dpmaif_dump_drb(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_txq *txq,
			    unsigned int dump_cnt)
{
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"== dump txq%d=0x%llx ==\n", txq->id, (u64)txq);

	/* dump DRB info */
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"drb base=0x%llx(%d*%d), sw record base=0x%llx(%d*%d)\n",
		      (u64)txq->drb_base, (int)sizeof(struct dpmaif_pd_drb), txq->drb_cnt,
		      (u64)txq->sw_drb_base, (int)sizeof(struct dpmaif_drb_skb), txq->drb_cnt);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"drb: w=%u,r=%u,rel=%u,to_submit=%d\n",
		      txq->drb_wr_idx, txq->drb_rd_idx, txq->drb_rel_rd_idx,
		      atomic_read(&txq->to_submit_cnt));

	/* Dump stats */
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"tx_sw_packets=%llu, tx_hw_packets=%llu, tx_done_last_time=%llu\n",
		dcb->traffic_stats.tx_sw_packets[txq->id],
		dcb->traffic_stats.tx_hw_packets[txq->id],
		dcb->traffic_stats.tx_done_last_time[txq->id]);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"tx_done_last_cnt=%u, ul_done=%llu, ul_drb_empty=%llu\n",
		      dcb->traffic_stats.tx_done_last_cnt[txq->id],
		dcb->traffic_stats.irq_tx_evt[txq->id].ul_done,
		dcb->traffic_stats.irq_tx_evt[txq->id].ul_drb_empty);

	dpmaif_dump_txq_drb_info(txq, dump_cnt);

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5, "== dump txq%d end ==\n", txq->id);
}

static void dpmaif_dump_txq_info(struct mtk_dpmaif_ctlb *dcb, unsigned int q_mask)
{
	int i;

	for (i = 0; i < dcb->res_cfg->txq_cnt; i++) {
		if (!((1 << i) & q_mask))
			continue;

		dpmaif_dump_drb(dcb, &dcb->txqs[i], DPMAIF_DUMP_DRB_CNT);
	}
}

static void dpmaif_dump_rxq_pit_info(struct dpmaif_rxq *rxq, unsigned int cnt)
{
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	unsigned int *pit_info;
	unsigned int pit_idx;
	unsigned int j;

	if (cnt > (rxq->pit_cnt / 2))
		cnt = rxq->pit_cnt / 2;

	pit_idx = (rxq->pit_cnt + rxq->pit_rd_idx - cnt) % rxq->pit_cnt;
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"rxq(%u), dump pit info,start_idx=%u,rd_idx=%u\n",
		      rxq->id, pit_idx, rxq->pit_rd_idx);

	if (rxq->pit_base) {
		for (j = 0; j < cnt * 2; j++) {
			pit_info = (unsigned int *)(rxq->pit_base + pit_idx);
			MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
				"pit(%u): 0x%08x, 0x%08x, 0x%08x,0x%08x\n",
				pit_idx, pit_info[0], pit_info[1], pit_info[2], pit_info[3]);
			pit_idx = mtk_dpmaif_ring_buf_get_next_idx(rxq->pit_cnt, pit_idx);
		}
	}
}

static void dpmaif_dump_normal_bat_skb_info(struct dpmaif_rxq *rxq, unsigned int cnt)
{
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	union dpmaif_bat_record *bat_record;
	struct dpmaif_bat_ring *bat_ring;
	struct skb_mapped_t *skb_info;
	unsigned int bidx;
	int j;

	bat_ring = &dcb->bat_info.normal_bat_ring;
	if (cnt > (bat_ring->bat_cnt / 2))
		cnt = bat_ring->bat_cnt / 2;

	bidx = (bat_ring->bat_cnt + rxq->pit_bid - cnt) % bat_ring->bat_cnt;
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"rxq(%u),dump normal bat skb info,start_idx=%u,pit_bid=%u\n",
		rxq->id, bidx, rxq->pit_bid);
	for (j = 0; j < cnt * 2; j++) {
		bat_record = bat_ring->sw_record_base + bidx;
		skb_info = &bat_record->normal;
		if (skb_info->skb)
			MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
				"rxq(%u),bid=%u,skb=%llx,len=%u,dma=0x%llx\n",
					  rxq->id, bidx, (u64)skb_info->skb,
					  skb_info->data_len, (u64)skb_info->data_dma_addr);
		else
			MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
				"skb is NULL,bid=%u\n", bidx);
		bidx = mtk_dpmaif_ring_buf_get_next_idx(bat_ring->bat_cnt, bidx);
	}
}


static void dpmaif_dump_rxq_info(struct mtk_dpmaif_ctlb *dcb,
				 unsigned int q_mask)
{
	struct dpmaif_rxq *rxq;
	int i;

	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		if (!((1 << i) & q_mask))
			continue;

		rxq = &dcb->rxqs[i];
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"== dump rxq%d=0x%llx ==\n", i, (u64)rxq);

		/* PIT information dump. */
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"pit base=0x%llx(%d*%d), rel_cnt=%d\n",
			      (u64)rxq->pit_base, (int)sizeof(struct dpmaif_pd_pit),
			      rxq->pit_cnt, atomic_read(&rxq->pit_rel_cnt));
#else
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"pit base=0x%llx(%d*%d), rel_cnt=%d\n",
			      (u64)rxq->pit_base, (int)sizeof(struct dpmaif_pd_pit),
			      rxq->pit_cnt, rxq->pit_rel_cnt);
#endif
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"pit: w=%u,r=%u,rel=%u\n",
			      rxq->pit_wr_idx, rxq->pit_rd_idx, rxq->pit_rel_rd_idx);

		if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
			MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
				"hw_pit: w=%d,r=%d\n",
				      mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
								  DPMAIF_PIT_WIDX, i),
				      mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
								  DPMAIF_PIT_RIDX, i));

		/* Dump stats */
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"rx_packets=%llu, rx_errors=%llu, rx_dropped=%llu\n",
			      dcb->traffic_stats.rx_packets[i],
			      dcb->traffic_stats.rx_errors[i],
			      dcb->traffic_stats.rx_dropped[i]);
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"rx_hw_ind_dropped=%u, rx_done_last_time=%llu, rx_done_last_cnt=%u\n",
			      dcb->traffic_stats.rx_hw_ind_dropped[i],
			      dcb->traffic_stats.rx_done_last_time[i],
			      dcb->traffic_stats.rx_done_last_cnt[i]);
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"dl_done=%llu, pit_len_err=%llu\n",
			      dcb->traffic_stats.irq_rx_evt[i].dl_done,
			      dcb->traffic_stats.irq_rx_evt[i].pit_len_err);

		dpmaif_dump_rxq_pit_info(rxq, DPMAIF_DUMP_PIT_CNT);
		dpmaif_dump_normal_bat_skb_info(rxq, DPMAIF_DUMP_BAT_CNT);
	}
}

#define MASK_TBL_SIZE 32

static void dpmaif_dump_u32_buffer(char *title, u32 *array, unsigned int cnt,
				   struct mtk_dpmaif_ctlb *dcb)
{
	int i, rem;

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"dump_buffer :%s\n", title);

	rem = cnt % 4;
	for (i = 0; i < cnt; i += 4) {
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"buffer[%d~%d]: %08x %08x %08x %08x\n", i, i + 3, array[i],
			array[i + 1], array[i + 2], array[i + 3]);
	}

	i -= 4;
	if (rem == 3)
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"buffer[%d~%d]: %08x %08x %08x\n", i, i + 2,
			array[i], array[i + 1], array[i + 2]);

	if (rem == 2)
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"buffer[%d~%d]: %08x %08x\n", i, i + 1, array[i], array[i + 1]);

	if (rem == 1)
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"buffer[%d]: %08x\n", i, array[i]);
}

static void dpmaif_dump_bat_info(struct mtk_dpmaif_ctlb *dcb)
{
	struct dpmaif_bat_ring *bat_ring;
	int mask_cnt = 0, mask_cont = 0;
	u32 array[MASK_TBL_SIZE];
	int arr_index = 0;
	char title[64];
	int cur_idx;
	u32 mask;
	int i, j;

	/* Normal BAT information dump. */
	bat_ring = &dcb->bat_info.normal_bat_ring;
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"== dump normal bat base=0x%llx(%d*%u) ==\n",
		      (u64)bat_ring->bat_base, (int)sizeof(struct dpmaif_bat), bat_ring->bat_cnt);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"normal bat: w=%u,r=%u,rel=%u\n", bat_ring->bat_wr_idx,
		      bat_ring->bat_rd_idx, bat_ring->bat_rel_rd_idx);
	if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"hw_bat: w=%d,r=%d\n",
			      mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_BAT_WIDX, 0),
			      mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_BAT_RIDX, 0));

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"normal bat sw record base =0x%llx(%d*%u)\n",
		      (u64)bat_ring->sw_record_base, (int)sizeof(union dpmaif_bat_record),
		      bat_ring->bat_cnt);

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"== dump normal bat mask tbl status ==\n");

	for (i = 0; i < bat_ring->bat_cnt; i += 32) {
		mask = 0;
		for (j = 0; j < 32; ++j) {
			if (bat_ring->mask_tbl[i + j]) {
				mask_cnt++;
				mask |= (1 << j);
			}
		}

		array[arr_index] = mask;
		arr_index++;

		if (arr_index == MASK_TBL_SIZE) {
			snprintf(title, sizeof(title), "bat mask tbl [%d~%d]",
				 i - 32 * MASK_TBL_SIZE + 32, i + 32 - 1);
			dpmaif_dump_u32_buffer(title, array, arr_index, dcb);
			arr_index = 0;
		}
	}

	if (arr_index > 0) {
		snprintf(title, sizeof(title), "bat mask tbl [%d~%d]",
			 i - 32 * MASK_TBL_SIZE + 32, i + 32 - 1);
		dpmaif_dump_u32_buffer(title, array, arr_index, dcb);
	}

	/* Check and get the continuous used entries,
	 * and it is also the count that will be recycle.
	 */
	cur_idx = bat_ring->bat_rel_rd_idx;
	for (i = 0; i < bat_ring->bat_cnt; i++) {
		if (bat_ring->mask_tbl[cur_idx])
			mask_cont++;
		else
			break;
		cur_idx = mtk_dpmaif_ring_buf_get_next_idx(bat_ring->bat_cnt, cur_idx);
	}

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"== bat rel_idx:%d rel_cnt: %d bat all_cnt: %d ==\n", bat_ring->bat_rel_rd_idx,
		mask_cont, mask_cnt);

	/* Frag BAT information dump. */
	bat_ring = &dcb->bat_info.frag_bat_ring;
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"== frag bat base=0x%llx(%d*%u) ==\n",
		      (u64)bat_ring->bat_base, (int)sizeof(struct dpmaif_bat), bat_ring->bat_cnt);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"frag bat: w=%u,r=%u,rel=%u\n", bat_ring->bat_wr_idx,
		      bat_ring->bat_rd_idx, bat_ring->bat_rel_rd_idx);

	if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"hw_farg_bat: w=%d,r=%d\n",
			      mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_FRAG_WIDX, 0),
			      mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_FRAG_RIDX, 0));

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"frag bat sw record base: 0x%llx(%d*%u)\n",
		      (u64)bat_ring->sw_record_base, (int)sizeof(union dpmaif_bat_record),
		      bat_ring->bat_cnt);
}

static void dpmaif_dump_intr_info(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char i;

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"== dump interrupt ==\n");

	for (i = 0; i < dcb->res_cfg->irq_cnt; i++) {
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
			"irq_total_cnt=%lld, irq_last_time=%lld\n",
			      dcb->traffic_stats.irq_total_cnt[i],
			      dcb->traffic_stats.irq_last_time[i]);
	}

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"ul_md_notready=%llu, ul_md_pwr_notready=%llu, ul_len_err=%llu\n",
		       dcb->traffic_stats.irq_other_evt.ul_md_notready,
		       dcb->traffic_stats.irq_other_evt.ul_md_pwr_notready,
		       dcb->traffic_stats.irq_other_evt.ul_len_err);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"dl_skb_len_err=%llu, dl_bat_cnt_len_err=%llu, dl_pkt_empty=%llu\n",
		      dcb->traffic_stats.irq_other_evt.dl_skb_len_err,
		      dcb->traffic_stats.irq_other_evt.dl_bat_cnt_len_err,
		      dcb->traffic_stats.irq_other_evt.dl_pkt_empty);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"dl_frag_empty=%llu, dl_mtu_err=%llu, dl_frag_cnt_len_err=%llu\n",
		      dcb->traffic_stats.irq_other_evt.dl_frag_empty,
		      dcb->traffic_stats.irq_other_evt.dl_mtu_err,
		      dcb->traffic_stats.irq_other_evt.dl_frag_cnt_len_err);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"hpc_ent_type_err=%llu\n",
		      dcb->traffic_stats.irq_other_evt.hpc_ent_type_err);
}

static void dpmaif_dump(struct mtk_dpmaif_ctlb *dcb)
{
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"===start dump mtk dpmaif(cb=0x%llx) information===\n", (u64)dcb);

	/* Dump dpmaif drv information. */
	if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
		dpmaif_drv_dump(dcb->drv_info);

	/* Dump dpmaif tx/rx information. */
	dpmaif_dump_txq_info(dcb, 0xffffffff);
	dpmaif_dump_rxq_info(dcb, 0xffffffff);
	dpmaif_dump_bat_info(dcb);
	dpmaif_dump_intr_info(dcb);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"===end dump mtk dpmaif information===\n");
}

static inline void dpmaif_dump_once(struct mtk_dpmaif_ctlb *dcb)
{
	if (atomic_cmpxchg(&dcb->dump_once, 0, 1) == 0)
		dpmaif_dump(dcb);
}

static inline void mtk_dpmaif_trigger_dev_exception(struct mtk_dpmaif_ctlb *dcb)
{
	mtk_hw_send_ext_evt(DCB_TO_MDEV(dcb), EXT_EVT_H2D_RESERVED_FOR_DPMAIF);
}

static void mtk_dpmaif_common_err_handle(struct mtk_dpmaif_ctlb *dcb, bool is_hw)
{
	MTK_ERR(DCB_TO_MDEV(dcb), "Enter common error handle: %ps, is_hw=%d\n",
		__builtin_return_address(0), is_hw);

	if (is_hw) {
		if (mtk_hw_mmio_check(DCB_TO_MDEV(dcb))) {
			if (!test_and_set_bit(DATA_LINK_ERR, &dcb->err_event)) {
				MTK_INFO(DCB_TO_MDEV(dcb), "Failed to access mmio\n");
				mtk_except_report_evt(DCB_TO_MDEV(dcb), EXCEPT_LINK_ERR);
			}
		} else {
			if (!test_and_set_bit(DATA_HW_CHECK_ERR, &dcb->err_event)) {
				dpmaif_dump_once(dcb);
				mtk_dpmaif_trigger_dev_exception(dcb);
			}
		}
	} else {
		if (!test_and_set_bit(DATA_SW_CHECK_ERR, &dcb->err_event)) {
			dpmaif_dump_once(dcb);
			MTK_BUG(DCB_TO_MDEV(dcb));
		}
	}
}

static unsigned int mtk_dpmaif_pit_bid(struct dpmaif_pd_pit *pit_info)
{
	unsigned int buf_id = FIELD_GET(PIT_PD_H_BID, le32_to_cpu(pit_info->pd_footer)) << 13;

	return buf_id + FIELD_GET(PIT_PD_BUF_ID, le32_to_cpu(pit_info->pd_header));
}

static void mtk_dpmaif_disable_irq(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char irq_cnt = dcb->res_cfg->irq_cnt;
	struct dpmaif_irq_param *irq_param;
	int i;

	if (!dcb->irq_enabled)
		return;

	dcb->irq_enabled = false;
	for (i = 0; i < irq_cnt; i++) {
		irq_param = &dcb->irq_params[i];
		if (mtk_hw_mask_irq(DCB_TO_MDEV(dcb), irq_param->dev_irq_id) != 0)
			MTK_ERR(DCB_TO_MDEV(dcb),
				"Failed to mask dev irq%d\n", irq_param->dev_irq_id);
	}
}

static void mtk_dpmaif_enable_irq(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char irq_cnt = dcb->res_cfg->irq_cnt;
	struct dpmaif_irq_param *irq_param;
	int i;

	if (dcb->irq_enabled)
		return;

	dcb->irq_enabled = true;
	for (i = 0; i < irq_cnt; i++) {
		irq_param = &dcb->irq_params[i];
		if (mtk_hw_unmask_irq(DCB_TO_MDEV(dcb), irq_param->dev_irq_id) != 0)
			MTK_ERR(DCB_TO_MDEV(dcb),
				"Failed to unmask dev irq%d\n", irq_param->dev_irq_id);
	}
}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
static enum hrtimer_restart mtk_dpmaif_doorbell_timer_func(struct hrtimer *t)
{
	struct mtk_dpmaif_ctlb *dcb = from_timer(dcb, t, doorbell_timer);

	queue_work(dcb->doorbell_wq, &dcb->doorbell_work);
	return HRTIMER_NORESTART;
}

static void mtk_dpmaif_book_doorbell_work(struct mtk_dpmaif_ctlb *dcb,
					  unsigned int delay_ms)
{
	unsigned long long now, delay_ns = 0;

	mutex_lock(&dcb->doorbell_timer_lock);

	if (!delay_ms) {
		/* check if timer callback is running, skip queue doorbell work */
		if (hrtimer_try_to_cancel(&dcb->doorbell_timer) != -1)
			queue_work(dcb->doorbell_wq, &dcb->doorbell_work);
		goto out;
	}

	now = local_clock();
	delay_ns = delay_ms * 1000000;

	if (!hrtimer_active(&dcb->doorbell_timer)) {
		if (dcb->res_cfg->cap & DATA_HW_F_TRAS_ALIGN)
			delay_ns -= (now - dcb->doorbell_base_ts) % delay_ns;
		hrtimer_start(&dcb->doorbell_timer, ns_to_ktime(delay_ns), HRTIMER_MODE_REL);
		goto out;
	}

	if (ktime_after(hrtimer_get_expires(&dcb->doorbell_timer), ns_to_ktime(now + delay_ns))) {
		hrtimer_start(&dcb->doorbell_timer, ns_to_ktime(delay_ns), HRTIMER_MODE_REL);
		goto out;
	}

	mutex_unlock(&dcb->doorbell_timer_lock);
	return;
out:
	mutex_unlock(&dcb->doorbell_timer_lock);
	trace_mtk_tras_data_tx(dcb->doorbell_base_ts, delay_ns);
}

static void mtk_dpmaif_book_tx_doorbell(struct mtk_dpmaif_ctlb *dcb,
					struct dpmaif_txq *txq)
{
	int to_submit_cnt;

	to_submit_cnt = atomic_read(&txq->to_submit_cnt);
	if (!to_submit_cnt)
		return;

	if (to_submit_cnt < txq->burst_submit_cnt && txq->exit_tcp_ss_counter <= 0)
		mtk_dpmaif_book_doorbell_work(dcb, txq->doorbell_delay);
	else
		mtk_dpmaif_book_doorbell_work(dcb, 0);

	if (txq->exit_tcp_ss_counter > 0)
		txq->exit_tcp_ss_counter--;
	trace_mtk_tput_data_tx("tx p5");
}
#endif

static int mtk_dpmaif_reload_rx_skb(struct mtk_dpmaif_ctlb *dcb,
				    struct dpmaif_bat_ring *bat_ring, unsigned int buf_cnt)
{
	union dpmaif_bat_record *cur_bat_record;
	struct skb_mapped_t *skb_info;
	unsigned short cur_bat_idx;
	struct dpmaif_bat *cur_bat;
	unsigned int i;
	int ret;

	/* Pin rx buffers to BAT entries */
	cur_bat_idx = bat_ring->bat_wr_idx;
	for (i = 0 ; i < buf_cnt; i++) {
		/* For re-init flow, in re-init flow, we don't release
		 * the rx buffer on FSM_STATE_OFF state.
		 * because we will pin rx buffers to BAT entries
		 * again on FSM_STATE_BOOTUP state.
		 */
		cur_bat_record = bat_ring->sw_record_base + cur_bat_idx;
		skb_info = &cur_bat_record->normal;
		if (!skb_info->skb) {
			skb_info->skb = mtk_bm_alloc(dcb->skb_pool);
			if (unlikely(!skb_info->skb)) {
				MTK_WARN(DCB_TO_MDEV(dcb),
					 "Failed to alloc skb, bat%d buf_cnt:%u/%u\n",
					bat_ring->type, buf_cnt, i);
				break;
			}

			skb_info->data_len = bat_ring->buf_size;
			skb_info->data_dma_addr = dma_map_single(DCB_TO_DEV(dcb),
								 skb_info->skb->data,
								 skb_info->data_len,
								 DMA_FROM_DEVICE);
			ret = dma_mapping_error(DCB_TO_DEV(dcb), skb_info->data_dma_addr);
			if (unlikely(ret)) {
				MTK_ERR(DCB_TO_MDEV(dcb), "Failed to map dma!\n");
				dev_kfree_skb_any(skb_info->skb);
				skb_info->skb = NULL;
				break;
			}
		}

		cur_bat = bat_ring->bat_base + cur_bat_idx;
		cur_bat->buf_addr_high = cpu_to_le32(upper_32_bits(skb_info->data_dma_addr));
		cur_bat->buf_addr_low = cpu_to_le32(lower_32_bits(skb_info->data_dma_addr));

		cur_bat_idx = mtk_dpmaif_ring_buf_get_next_idx(bat_ring->bat_cnt, cur_bat_idx);
	}

	ret = i;
	if (unlikely(ret == 0))
		ret = -DATA_LOW_MEM_SKB;
	bat_ring->bat_wr_idx = cur_bat_idx;

	return ret;
}

static int mtk_dpmaif_reload_rx_page(struct mtk_dpmaif_ctlb *dcb,
				     struct dpmaif_bat_ring *bat_ring, unsigned int buf_cnt)
{
	union dpmaif_bat_record *cur_bat_record;
	struct page_mapped_t *page_info;
	unsigned short cur_bat_idx;
	struct dpmaif_bat *cur_bat;
	unsigned int i;
	void *data;
	int ret;

	/* Pin rx buffers to BAT entries */
	cur_bat_idx = bat_ring->bat_wr_idx;
	for (i = 0 ; i < buf_cnt; i++) {
		/* For re-init flow, In re-init flow, we don't release
		 * the rx buffer on FSM_STATE_OFF state.
		 * because we will pin rx buffers to BAT
		 * entries again on FSM_STATE_BOOTUP state.
		 */
		cur_bat_record = bat_ring->sw_record_base + cur_bat_idx;
		page_info = &cur_bat_record->frag;

		if (!page_info->page) {
			data = mtk_bm_alloc(dcb->page_pool);
			if (unlikely(!data)) {
				MTK_WARN(DCB_TO_MDEV(dcb),
					 "Failed to alloc page, bat%d buf_cnt:%u/%u\n",
					bat_ring->type, buf_cnt, i);
				break;
			}

			page_info->page = virt_to_head_page(data);
			page_info->offset = data - page_address(page_info->page);
			page_info->data_len = bat_ring->buf_size;
			page_info->data_dma_addr = dma_map_page(DCB_TO_DEV(dcb),
								page_info->page,
								page_info->offset,
								page_info->data_len,
								DMA_FROM_DEVICE);
			ret = dma_mapping_error(DCB_TO_DEV(dcb), page_info->data_dma_addr);
			if (unlikely(ret)) {
				MTK_ERR(DCB_TO_MDEV(dcb), "Failed to map dma!\n");
				put_page(page_info->page);
				page_info->page = NULL;
				break;
			}
		}
		cur_bat = bat_ring->bat_base + cur_bat_idx;
		cur_bat->buf_addr_high = cpu_to_le32(upper_32_bits(page_info->data_dma_addr));
		cur_bat->buf_addr_low = cpu_to_le32(lower_32_bits(page_info->data_dma_addr));
		cur_bat_idx = mtk_dpmaif_ring_buf_get_next_idx(bat_ring->bat_cnt, cur_bat_idx);
	}

	ret = i;
	if (unlikely(ret == 0))
		ret = -DATA_LOW_MEM_SKB;
	bat_ring->bat_wr_idx = cur_bat_idx;

	return ret;
}

static int mtk_dpmaif_reload_rx_buf(struct mtk_dpmaif_ctlb *dcb,
				    struct dpmaif_bat_ring *bat_ring,
				    unsigned int buf_cnt, bool send_doorbell)
{
	unsigned int reload_cnt, bat_cnt;
	int ret;

	if (unlikely(buf_cnt == 0 || buf_cnt > bat_ring->bat_cnt)) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Invalid alloc bat buffer count\n");
		return -DATA_FLOW_CHK_ERR;
	}

	/* Get bat count that be reloaded rx buffer and check
	 * Rx buffer count should not be greater than bat entry count,
	 * because one rx buffer is pined to one bat entry.
	 */
	bat_cnt = mtk_dpmaif_ring_buf_writable(bat_ring->bat_cnt, bat_ring->bat_rel_rd_idx,
					       bat_ring->bat_wr_idx);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_4,
		"alloc_cnt=%u, writable bat_cnt=%u\n", buf_cnt, bat_cnt);
	if (unlikely(buf_cnt > bat_cnt)) {
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Invalid parameter,bat%d: rx_buff>bat_entries(%u>%u), w=%u,r=%u,rel=%u\n",
			bat_ring->type, buf_cnt, bat_cnt, bat_ring->bat_wr_idx,
			bat_ring->bat_rd_idx, bat_ring->bat_rel_rd_idx);
		return -DATA_FLOW_CHK_ERR;
	}

	/* Allocate rx buffer and pin it to bat entry. */
	if (bat_ring->type == NORMAL_BAT)
		ret = mtk_dpmaif_reload_rx_skb(dcb, bat_ring, buf_cnt);
	else
		ret = mtk_dpmaif_reload_rx_page(dcb, bat_ring, buf_cnt);

	if (ret < 0)
		return -DATA_LOW_MEM_SKB;

	/* Check and update bat_wr_idx */
	reload_cnt = ret;

	/* Make sure all frag bat information write done before notify HW. */
	dma_wmb();

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	/* booking doorbell work to notify hw the available buffer count. */
	if (send_doorbell) {
		atomic_add(reload_cnt, &bat_ring->bat_rel_cnt);
		mtk_dpmaif_book_doorbell_work(dcb, 0);
	}
#else
	/* Notify hw the available frag bat buffer count. */
	if (send_doorbell) {
		if (bat_ring->type == NORMAL_BAT)
			ret = mtk_dpmaif_drv_send_doorbell(dcb->drv_info,
							   DPMAIF_BAT, 0, reload_cnt);
		else
			ret = mtk_dpmaif_drv_send_doorbell(dcb->drv_info,
							   DPMAIF_FRAG, 0, reload_cnt);
		if (unlikely(ret < 0)) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to send frag bat doorbell\n");
			mtk_dpmaif_common_err_handle(dcb, true);
			return ret;
		}
	}
#endif
	return 0;
}

static unsigned int mtk_dpmaif_chk_rel_bat_cnt(struct mtk_dpmaif_ctlb *dcb,
					       struct dpmaif_bat_ring *bat_ring)
{
	unsigned int i, cur_idx;
	unsigned int count = 0;
	unsigned char mask_val;

	/* Check and get the continuous used entries,
	 * and it is also the count that will be recycle.
	 */
	cur_idx = bat_ring->bat_rel_rd_idx;
	for (i = 0; i < bat_ring->bat_cnt; i++) {
		mask_val = bat_ring->mask_tbl[cur_idx];
		if (mask_val == 1)
			count++;
		else
			break;

		cur_idx = mtk_dpmaif_ring_buf_get_next_idx(bat_ring->bat_cnt, cur_idx);
	}

	return count;
}

static int mtk_dpmaif_recycle_bat(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_bat_ring *bat_ring,
				  unsigned int rel_bat_cnt)
{
	unsigned short old_sw_rel_idx, new_sw_rel_idx, hw_rd_idx;
	bool type = bat_ring->type == NORMAL_BAT;
	unsigned int cur_idx;
	unsigned int i;
	int ret;

	old_sw_rel_idx = bat_ring->bat_rel_rd_idx;
	new_sw_rel_idx = old_sw_rel_idx + rel_bat_cnt;

	ret = mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
					  type ? DPMAIF_BAT_RIDX : DPMAIF_FRAG_RIDX, 0);
	if (unlikely(ret < 0)) {
		MTK_WARN(DCB_TO_MDEV(dcb), "Failed to read bat%d ridx\n", bat_ring->type);
		mtk_dpmaif_common_err_handle(dcb, true);
		return ret;
	}

	hw_rd_idx = ret;
	bat_ring->bat_rd_idx = hw_rd_idx;

	/* Queue is empty and no need to release. */
	if (bat_ring->bat_wr_idx == old_sw_rel_idx) {
		ret = -DATA_FLOW_CHK_ERR;
		goto out;
	}

	/* bat_rel_rd_idx should not exceed bat_rd_idx. */
	if (hw_rd_idx > old_sw_rel_idx) {
		if (new_sw_rel_idx > hw_rd_idx) {
			ret = -DATA_FLOW_CHK_ERR;
			goto out;
		}
	} else if (hw_rd_idx < old_sw_rel_idx) {
		if (new_sw_rel_idx >= bat_ring->bat_cnt) {
			new_sw_rel_idx = new_sw_rel_idx - bat_ring->bat_cnt;
			if (new_sw_rel_idx > hw_rd_idx) {
				ret = -DATA_FLOW_CHK_ERR;
				goto out;
			}
		}
	}

	/* Reset bat mask value. */
	cur_idx = bat_ring->bat_rel_rd_idx;
	for (i = 0; i < rel_bat_cnt; i++) {
		bat_ring->mask_tbl[cur_idx] = 0;
		cur_idx = mtk_dpmaif_ring_buf_get_next_idx(bat_ring->bat_cnt, cur_idx);
	}

	bat_ring->bat_rel_rd_idx = new_sw_rel_idx;

	return rel_bat_cnt;

out:
	MTK_ERR(DCB_TO_MDEV(dcb),
		"Failed to check bat%d rel_rd_idx, bat_rd=%u,old_sw_rel=%u, new_sw_rel=%u\n",
		bat_ring->type, bat_ring->bat_rd_idx, old_sw_rel_idx, new_sw_rel_idx);

	return ret;
}

static int mtk_dpmaif_reload_bat(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_bat_ring *bat_ring)
{
	unsigned int rel_bat_cnt;
	int ret = 0;

	rel_bat_cnt = mtk_dpmaif_chk_rel_bat_cnt(dcb, bat_ring);
	if (unlikely(rel_bat_cnt == 0))
		goto out;

	/* Check and update bat_rd_idx, bat_rel_rd_idx. */
	ret = mtk_dpmaif_recycle_bat(dcb, bat_ring, rel_bat_cnt);
	if (unlikely(ret < 0)) {
		MTK_WARN(DCB_TO_MDEV(dcb),
			 "Failed to recycle bat%d, ret=%d\n", bat_ring->type, ret);
		goto out;
	}

	/* Reload rx buffer, pin buffer to bat entries.
	 * update bat_wr_idx
	 * send doorbell to HW about new available BAT entries.
	 */
	ret = mtk_dpmaif_reload_rx_buf(dcb, bat_ring, rel_bat_cnt, true);

out:
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_4,
		"bat(%d),wr=%d,rd=%d,rel_rd=%d,rel_cnt=%d\n",
			bat_ring->type, bat_ring->bat_wr_idx, bat_ring->bat_rd_idx,
			bat_ring->bat_rel_rd_idx, rel_bat_cnt);

	trace_mtk_tput_data_bat(bat_ring->type, rel_bat_cnt,
				bat_ring->bat_wr_idx,
				bat_ring->bat_rd_idx,
				bat_ring->bat_rel_rd_idx);

	return ret;
}

static void mtk_dpmaif_bat_reload_work(struct work_struct *work)
{
	struct dpmaif_bat_ring *bat_ring;
	struct dpmaif_bat_info *bat_info;
	struct mtk_dpmaif_ctlb *dcb;
	int ret;

	bat_ring = container_of(work, struct dpmaif_bat_ring, reload_work);

	if (bat_ring->type == NORMAL_BAT)
		bat_info = container_of(bat_ring, struct dpmaif_bat_info, normal_bat_ring);
	else
		bat_info = container_of(bat_ring, struct dpmaif_bat_info, frag_bat_ring);

	dcb = bat_info->dcb;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	trace_mtk_tput_ds_lock("ds_lock, d1");
#endif
	pm_runtime_get(DCB_TO_DEV(dcb));
	mtk_pm_ds_lock(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF);
	ret = mtk_pm_ds_wait_complete(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF);
	if (unlikely(ret < 0)) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to wait ds_lock\n");
		mtk_dpmaif_common_err_handle(dcb, true);
		goto out;
	}
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	trace_mtk_tput_ds_lock("ds_lock, d2");
#endif
	if (bat_ring->type == NORMAL_BAT) {
		/* Recycle normal bat and reload rx normal buffer. */
		ret = mtk_dpmaif_reload_bat(dcb, bat_ring);
		if (unlikely(ret < 0)) {
			MTK_ERR(DCB_TO_MDEV(dcb),
				"Failed to recycle normal bat and reload rx buffer\n");
			goto out;
		}

#ifndef CONFIG_REG_ACCESS_OPTIMIZE
		if (bat_ring->bat_cnt_err_intr_set) {
			bat_ring->bat_cnt_err_intr_set = false;
			mtk_dpmaif_drv_intr_complete(dcb->drv_info,
						     DPMAIF_INTR_DL_BATCNT_LEN_ERR, 0, 0);
		}
#endif

	} else {
		/* Recycle frag bat and reload rx page buffer. */
		if (dcb->bat_info.frag_bat_enabled) {
			ret = mtk_dpmaif_reload_bat(dcb, bat_ring);
			if (unlikely(ret < 0)) {
				MTK_ERR(DCB_TO_MDEV(dcb),
					"Failed to recycle frag bat and reload rx buffer\n");
				goto out;
			}

#ifndef CONFIG_REG_ACCESS_OPTIMIZECCESS
			if (bat_ring->bat_cnt_err_intr_set) {
				bat_ring->bat_cnt_err_intr_set = false;
				mtk_dpmaif_drv_intr_complete(dcb->drv_info,
							     DPMAIF_INTR_DL_FRGCNT_LEN_ERR, 0, 0);
			}
#endif
		}
	}

out:
	mtk_pm_ds_unlock(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF, false);
	pm_runtime_put(DCB_TO_DEV(dcb));
}

static void mtk_dpmaif_queue_bat_reload_work(struct mtk_dpmaif_ctlb *dcb)
{
	/* Recycle normal bat and reload rx skb buffer. */
	queue_work(dcb->bat_info.reload_wq, &dcb->bat_info.normal_bat_ring.reload_work);
	/* Recycle frag bat and reload rx page buffer. */
	if (dcb->bat_info.frag_bat_enabled)
		queue_work(dcb->bat_info.reload_wq, &dcb->bat_info.frag_bat_ring.reload_work);
}

static void mtk_dpmaif_set_bat_buf_size(struct mtk_dpmaif_ctlb *dcb, unsigned int mtu)
{
	struct dpmaif_bat_info *bat_info = &dcb->bat_info;
	unsigned int buf_size;

	bat_info->max_mtu = mtu;

	/* Normal and frag BAT buffer size setting. */
	buf_size = mtu + DPMAIF_HW_PKT_ALIGN + DPMAIF_HW_BAT_RSVLEN;
	if (buf_size <= DPMAIF_BUF_THRESHOLD) {
		bat_info->frag_bat_enabled = false;
		bat_info->normal_bat_ring.buf_size = ALIGN(buf_size, DPMAIF_DL_BUF_MIN_SIZE);
		bat_info->frag_bat_ring.buf_size = 0;
	} else {
		bat_info->frag_bat_enabled = true;
		bat_info->normal_bat_ring.buf_size = DPMAIF_NORMAL_BUF_SIZE_IN_JUMBO;
		bat_info->frag_bat_ring.buf_size = DPMAIF_FRAG_BUF_SIZE_IN_JUMBO;
	}

	MTK_INFO(DCB_TO_MDEV(dcb),
		 "dpmaif mtu=%u, frag_enable=%d, normal_buf_size=%u, frag_buf_size=%u\n",
		bat_info->max_mtu, bat_info->frag_bat_enabled,
		bat_info->normal_bat_ring.buf_size,
		bat_info->frag_bat_ring.buf_size);
}

static int mtk_dpmaif_bat_init(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_bat_ring *bat_ring,
			       enum dpmaif_bat_type type)
{
	int ret;

	bat_ring->type = type;
	if (bat_ring->type == FRAG_BAT)
		bat_ring->bat_cnt = dcb->res_cfg->frag_bat_cnt;
	else
		bat_ring->bat_cnt = dcb->res_cfg->normal_bat_cnt;

	bat_ring->bat_cnt_err_intr_set = false;
	bat_ring->bat_rd_idx = 0;
	bat_ring->bat_wr_idx = 0;
	bat_ring->bat_rel_rd_idx = 0;

	/* Allocate BAT memory for HW and SW. */
	bat_ring->bat_base = dma_alloc_coherent(DCB_TO_DEV(dcb), bat_ring->bat_cnt *
						sizeof(*bat_ring->bat_base),
						&bat_ring->bat_dma_addr, GFP_KERNEL);
	if (!bat_ring->bat_base) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate bat%d\n", bat_ring->type);
		return -ENOMEM;
	}

	/* Allocate buffer for SW to record skb information */
#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
	bat_ring->sw_record_base =  xiaomi_kcalloc_dpmaif_memory(bat_ring->bat_cnt, sizeof(*bat_ring->sw_record_base));
#else
	bat_ring->sw_record_base = devm_kcalloc(DCB_TO_DEV(dcb), bat_ring->bat_cnt,
						sizeof(*bat_ring->sw_record_base), GFP_KERNEL);
#endif
	if (!bat_ring->sw_record_base) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate bat%d buffer", bat_ring->type);
		ret = -ENOMEM;
		goto free_bat_buf;
	}

	/* Alloc buffer for SW to recycle BAT. */
#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
	bat_ring->mask_tbl =  xiaomi_kcalloc_dpmaif_memory(bat_ring->bat_cnt, sizeof(*bat_ring->mask_tbl));
#else
	bat_ring->mask_tbl = devm_kcalloc(DCB_TO_DEV(dcb), bat_ring->bat_cnt,
					  sizeof(*bat_ring->mask_tbl), GFP_KERNEL);
#endif
	if (!bat_ring->mask_tbl) {
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Failed to allocate bat%d mask table\n", bat_ring->type);
		ret = -ENOMEM;
		goto free_mask_tbl;
	}

	INIT_WORK(&bat_ring->reload_work, mtk_dpmaif_bat_reload_work);

	MTK_INFO(DCB_TO_MDEV(dcb),
		 "bat%d: base=0x%llx, size=%u, addr=0x%llx,cnt=%u, swbase=0x%llx, tbl=0x%llx\n",
		bat_ring->type, (u64)bat_ring->bat_base,
		bat_ring->buf_size, bat_ring->bat_dma_addr,
		bat_ring->bat_cnt, (u64)bat_ring->sw_record_base, (u64)bat_ring->mask_tbl);

	return 0;

free_mask_tbl:
#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
	xiaomi_free_dpmaif_memory(bat_ring->sw_record_base);
#else
	devm_kfree(DCB_TO_DEV(dcb), bat_ring->sw_record_base);
#endif
	bat_ring->sw_record_base = NULL;
free_bat_buf:
	dma_free_coherent(DCB_TO_DEV(dcb), bat_ring->bat_cnt * sizeof(*bat_ring->bat_base),
			  bat_ring->bat_base, bat_ring->bat_dma_addr);
	bat_ring->bat_base = NULL;

	return ret;
}

static void mtk_dpmaif_flush_bat_reload(struct mtk_dpmaif_ctlb *dcb)
{
	struct dpmaif_bat_ring *normal_bat_ring;
	struct dpmaif_bat_ring *frag_bat_ring;

	/* Wait bat/frag reload process done. */
	flush_workqueue(dcb->bat_info.reload_wq);

	normal_bat_ring = &dcb->bat_info.normal_bat_ring;
	frag_bat_ring = &dcb->bat_info.frag_bat_ring;
	MTK_INFO(DCB_TO_MDEV(dcb),
		 "normal bat:r=%u,w=%u,rel=%u;frag bat:r=%u,w=%u,rel=%u\n",
		normal_bat_ring->bat_rd_idx, normal_bat_ring->bat_wr_idx,
		normal_bat_ring->bat_rel_rd_idx, frag_bat_ring->bat_rd_idx,
		frag_bat_ring->bat_wr_idx, frag_bat_ring->bat_rel_rd_idx);
}

static void mtk_dpmaif_bat_exit(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_bat_ring *bat_ring,
				enum dpmaif_bat_type type)
{
	union dpmaif_bat_record *bat_record;
	struct page *page;
	unsigned int i;

	flush_work(&bat_ring->reload_work);
#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
	xiaomi_free_dpmaif_memory(bat_ring->mask_tbl);
#else
	devm_kfree(DCB_TO_DEV(dcb), bat_ring->mask_tbl);
#endif
	bat_ring->mask_tbl = NULL;

	if (bat_ring->sw_record_base) {
		if (type == NORMAL_BAT) {
			for (i = 0; i < bat_ring->bat_cnt; i++) {
				bat_record = bat_ring->sw_record_base + i;
				if (bat_record->normal.skb) {
					dma_unmap_single(DCB_TO_DEV(dcb),
							 bat_record->normal.data_dma_addr,
							 bat_record->normal.data_len,
							 DMA_FROM_DEVICE);
					dev_kfree_skb_any(bat_record->normal.skb);
				}
			}
		} else {
			for (i = 0; i < bat_ring->bat_cnt; i++) {
				bat_record = bat_ring->sw_record_base + i;
				page = bat_record->frag.page;
				if (page) {
					dma_unmap_page(DCB_TO_DEV(dcb),
						       bat_record->frag.data_dma_addr,
						       bat_record->frag.data_len,
						       DMA_FROM_DEVICE);
					put_page(page);
				}
			}
		}
#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
		xiaomi_free_dpmaif_memory(bat_ring->sw_record_base);
#else
		devm_kfree(DCB_TO_DEV(dcb), bat_ring->sw_record_base);
#endif
		bat_ring->sw_record_base = NULL;
	}

	if (bat_ring->bat_base) {
		dma_free_coherent(DCB_TO_DEV(dcb), bat_ring->bat_cnt * sizeof(*bat_ring->bat_base),
				  bat_ring->bat_base, bat_ring->bat_dma_addr);
		bat_ring->bat_base = NULL;
	}
}

static void mtk_dpmaif_bat_ring_reset(struct dpmaif_bat_ring *bat_ring)
{
	bat_ring->bat_cnt_err_intr_set = false;
	bat_ring->bat_wr_idx = 0;
	bat_ring->bat_rd_idx = 0;
	bat_ring->bat_rel_rd_idx = 0;
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	atomic_set(&bat_ring->bat_rel_cnt, 0);
#endif
	memset(bat_ring->bat_base, 0x00, (bat_ring->bat_cnt * sizeof(*bat_ring->bat_base)));
	memset(bat_ring->mask_tbl, 0x00, (bat_ring->bat_cnt * sizeof(*bat_ring->mask_tbl)));
}

static void mtk_dpmaif_bat_res_reset(struct dpmaif_bat_info *bat_info)
{
	mtk_dpmaif_bat_ring_reset(&bat_info->normal_bat_ring);
	if (bat_info->frag_bat_enabled)
		mtk_dpmaif_bat_ring_reset(&bat_info->frag_bat_ring);
}

static int mtk_dpmaif_bat_res_init(struct mtk_dpmaif_ctlb *dcb)
{
	struct dpmaif_bat_info *bat_info = &dcb->bat_info;
	int ret;

	bat_info->dcb = dcb;
	ret = mtk_dpmaif_bat_init(dcb, &bat_info->normal_bat_ring, NORMAL_BAT);
	if (ret < 0) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to initialize normal bat resource\n");
		goto out;
	}

	if (bat_info->frag_bat_enabled) {
		ret = mtk_dpmaif_bat_init(dcb, &bat_info->frag_bat_ring, FRAG_BAT);
		if (ret < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to initialize frag bat resource\n");
			goto exit_frag_bat;
		}
	}

	bat_info->reload_wq =
		alloc_workqueue("dpmaif_bat_reload_wq_%s", WQ_HIGHPRI | WQ_UNBOUND |
				WQ_MEM_RECLAIM, FRAG_BAT + 1, DCB_TO_DEV_STR(dcb));
	if (!bat_info->reload_wq) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate bat reload workqueue\n");
		ret = -ENOMEM;
		goto exit_bat;
	}

	return 0;

exit_frag_bat:
	mtk_dpmaif_bat_exit(dcb, &bat_info->frag_bat_ring, FRAG_BAT);

exit_bat:
	mtk_dpmaif_bat_exit(dcb, &bat_info->normal_bat_ring, NORMAL_BAT);

out:
	return ret;
}

static void mtk_dpmaif_bat_res_exit(struct mtk_dpmaif_ctlb *dcb)
{
	struct dpmaif_bat_info *bat_info = &dcb->bat_info;

	if (bat_info->reload_wq) {
		flush_workqueue(bat_info->reload_wq);
		destroy_workqueue(bat_info->reload_wq);
		bat_info->reload_wq = NULL;
	}

	if (bat_info->frag_bat_enabled)
		mtk_dpmaif_bat_exit(dcb, &bat_info->frag_bat_ring, FRAG_BAT);

	mtk_dpmaif_bat_exit(dcb, &bat_info->normal_bat_ring, NORMAL_BAT);
}

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
static void mtk_dpmaif_rx_steer(struct work_struct *work)
{
	struct dpmaif_rxq *rxq = container_of(work, struct dpmaif_rxq, steer_work);
	unsigned long timeout = jiffies + msecs_to_jiffies(2000);
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	union mtk_data_pkt_info *pkt_info;
	struct sk_buff *skb;
	int ret;

	while (!skb_queue_empty(&rxq->list)) {
		trace_mtk_tput_data_rx(rxq->id, "rx p3.3");
		skb = skb_dequeue(&rxq->list);

		pkt_info = DPMAIF_SKB_CB(skb);
		trace_mtk_tput_data_rx(rxq->id, "rx p3.4");
		ret = dcb->port_ops->recv(dcb->data_blk, skb, rxq->id, pkt_info->rx.ch_id);

		trace_mtk_tput_data_rx(rxq->id, "rx p6");
		if (unlikely(ret < 0)) {
			MTK_WARN(DCB_TO_MDEV(dcb), "Failed to send skb to data port\n");
			return;
		}

		if (time_after_eq(jiffies, timeout)) {
			queue_work_on(rxq->cpu_id, rxq->steer_wq, &rxq->steer_work);
			return;
		}

		if (need_resched())
			cond_resched();
	}
#ifdef CONFIG_MTK_ENABLE_WQ_GRO
	dcb->port_ops->notify(dcb->data_blk, DATA_EVT_RX_FLUSH, rxq->id);
#endif
}

static u8 mtk_dpmaif_get_work_cpu(struct dpmaif_rxq *rxq)
{
	return rxq->dcb->aff_cfg->steer_wq_aff[rxq->id];
}

static int mtk_dpmaif_rxq_steer_work_init(struct dpmaif_rxq *rxq)
{
	char wq_name[48];

	snprintf(wq_name, sizeof(wq_name), "mtk_rx_push_%s", DCB_TO_MDEV(rxq->dcb)->dev_str);
	rxq->steer_wq = alloc_workqueue(wq_name,
					WQ_CPU_INTENSIVE | WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);
	if (!rxq->steer_wq)
		return -ENOMEM;

	INIT_WORK(&rxq->steer_work, mtk_dpmaif_rx_steer);

	rxq->cpu_id = mtk_dpmaif_get_work_cpu(rxq);

	skb_queue_head_init(&rxq->list);

	return 0;
}
#endif
/***********************************xiaomi modem driver memory optimize  start***/
extern void* t800_get_pwrdev(void);
typedef struct {
    void* pointer;
    size_t cout;
    size_t size;
    bool isUsed;
} DpmaifPointerInfo;
#define MAX_DPMAIF_POINTERS 50
DpmaifPointerInfo g_dpmaifMemoArray[MAX_DPMAIF_POINTERS]={{NULL, 0, 0, false}};

void xiaomi_free_dpmaif_memory(void* ptr) {
	for (int i = 0; i < MAX_DPMAIF_POINTERS; i++) {
		if (g_dpmaifMemoArray[i].pointer == ptr) {
			g_dpmaifMemoArray[i].isUsed = false;
			memset(ptr, 0x00, (g_dpmaifMemoArray[i].cout * g_dpmaifMemoArray[i].size));
			pr_info("%s  index = %d\n", __func__, i);
		}
	}
}

void* xiaomi_kcalloc_dpmaif_memory(size_t cout, size_t size) {
	int i = 0;
	for (i = 0; i < MAX_DPMAIF_POINTERS; i++) {
		if (g_dpmaifMemoArray[i].cout == cout && g_dpmaifMemoArray[i].size == size && g_dpmaifMemoArray[i].isUsed == false && g_dpmaifMemoArray[i].pointer != NULL) {
			break;
		}
	}
	if (i < MAX_DPMAIF_POINTERS) {
		g_dpmaifMemoArray[i].isUsed = true;
		pr_info("%s  index = %d: count=%zu, size=%zu\n", __func__, i, g_dpmaifMemoArray[i].cout, g_dpmaifMemoArray[i].size);
		return g_dpmaifMemoArray[i].pointer;
	}
	for (i = 0; i < MAX_DPMAIF_POINTERS; i++) {
		if (  g_dpmaifMemoArray[i].pointer == NULL) {
			g_dpmaifMemoArray[i].pointer = kcalloc(cout, size, GFP_KERNEL);
			//g_dpmaifMemoArray[i].pointer = devm_kcalloc(t800_get_pwrdev(), cout, size, GFP_KERNEL);
			if (g_dpmaifMemoArray[i].pointer) {
				pr_info("%s  use new index = %d: count=%zu, size=%zu\n", __func__, i, g_dpmaifMemoArray[i].cout, g_dpmaifMemoArray[i].size);
				g_dpmaifMemoArray[i].isUsed = true;
				g_dpmaifMemoArray[i].cout = cout;
				g_dpmaifMemoArray[i].size = size;
				return g_dpmaifMemoArray[i].pointer;
			} else {
				pr_info("%s  kcalloc fail = %d\n", __func__, i);
			}
		}
	}
	pr_info("%s  Failed to allocate\n", __func__);
	return NULL;
}

void xiaomi_release_dpmaif_memory(void) {
	for (int i = 0; i < MAX_DPMAIF_POINTERS; i++) {
		if (g_dpmaifMemoArray[i].pointer != NULL) {
			kfree(g_dpmaifMemoArray[i].pointer);
			//devm_kfree(t800_get_pwrdev(), g_dpmaifMemoArray[i].pointer);
			g_dpmaifMemoArray[i].isUsed = false;
			g_dpmaifMemoArray[i].cout = 0;
			g_dpmaifMemoArray[i].size = 0;
			g_dpmaifMemoArray[i].pointer = NULL;
		}
	}
}

/***********************************xiaomi modem driver memory optimize end***/
static int mtk_dpmaif_rxq_init(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_rxq *rxq)
{
	char ws_name[DPMAIF_WS_NAME_LEN];
	int ret;

	rxq->started = false;
	rxq->pit_cnt = dcb->res_cfg->pit_cnt[rxq->id];
	rxq->pit_wr_idx = 0;
	rxq->pit_rd_idx = 0;
	rxq->pit_rel_rd_idx = 0;
	rxq->pit_seq_expect = 0;
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	rxq->pit_poll_enable = false;
	atomic_set(&rxq->pit_rel_cnt, 0);
	atomic_set(&rxq->pit_stats, 0);
#else
	rxq->pit_rel_cnt = 0;
#endif
	rxq->pit_cnt_err_intr_set = false;
	rxq->pit_burst_rel_cnt = DPMAIF_PIT_CNT_UPDATE_THRESHOLD;
	rxq->intr_coalesce_frame = dcb->intr_coalesce.rx_coalesced_frames;
	rxq->pit_seq_fail_cnt = 0;

	memset(&rxq->rx_record, 0x00, sizeof(rxq->rx_record));
	snprintf(ws_name, sizeof(ws_name), "dpmaif_rxq%dws", rxq->id);
	rxq->ws = wakeup_source_register(NULL, ws_name);
	if (!rxq->ws) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to register rxq%u wakeup source\n",
			rxq->id);
		return -ENOMEM;
	}

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE

#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
	rxq->pit_base =  xiaomi_kcalloc_dpmaif_memory(rxq->pit_cnt, sizeof(*rxq->pit_base));
#else
	rxq->pit_base =  devm_kcalloc(DCB_TO_DEV(dcb), rxq->pit_cnt,
				      sizeof(*rxq->pit_base), GFP_KERNEL);
#endif
	if (!rxq->pit_base) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate rxq%u pit resource\n",
			rxq->id);
		ret = -ENOMEM;
		goto unregister_ws;
	}

	rxq->pit_dma_addr = dma_map_single(DCB_TO_DEV(dcb),
					   rxq->pit_base,
					   rxq->pit_cnt * sizeof(*rxq->pit_base),
					   DMA_FROM_DEVICE);
	ret = dma_mapping_error(DCB_TO_DEV(dcb), rxq->pit_dma_addr);
	if (unlikely(ret)) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to map dma!\n");
#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
		xiaomi_free_dpmaif_memory(rxq->pit_base);
#else
		devm_kfree(DCB_TO_DEV(dcb), rxq->pit_base);
#endif
		ret =  -DATA_DMA_MAP_ERR;
		goto unregister_ws;
	}
#else
	rxq->pit_base = dma_alloc_coherent(DCB_TO_DEV(dcb),
					   rxq->pit_cnt * sizeof(*rxq->pit_base),
					   &rxq->pit_dma_addr, GFP_KERNEL);
	if (!rxq->pit_base) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate rxq%u pit resource\n",
			rxq->id);
		ret = -ENOMEM;
		goto unregister_ws;
	}
#endif

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	ret = mtk_dpmaif_rxq_steer_work_init(rxq);
	if (ret < 0) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to init rxq%u steer work\n",
			rxq->id);
		goto err_init_steer_work;
	}
#endif
	MTK_INFO(DCB_TO_MDEV(dcb),
		 "rxq%d: pit_base=0x%llx, pit_dma_addr=0x%llx, pit_cnt=%u\n",
		rxq->id, (u64)rxq->pit_base,
		rxq->pit_dma_addr, rxq->pit_cnt);
	
	return 0;

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
err_init_steer_work:
	dma_unmap_single(DCB_TO_DEV(dcb), rxq->pit_dma_addr,
			 rxq->pit_cnt * sizeof(*rxq->pit_base), DMA_TO_DEVICE);
#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
		xiaomi_free_dpmaif_memory(rxq->pit_base);
#else
	devm_kfree(DCB_TO_DEV(dcb), rxq->pit_base);
#endif

	return ret;
#endif

unregister_ws:
	wakeup_source_unregister(rxq->ws);
	return ret;
}

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
static void mtk_dpmaif_rxq_push_work_exit(struct dpmaif_rxq *rxq)
{
	flush_workqueue(rxq->steer_wq);
	destroy_workqueue(rxq->steer_wq);

	/* Drop all packet in rx virtual queues. */
	skb_queue_purge(&rxq->list);
}
#endif

static void mtk_dpmaif_rxq_exit(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_rxq *rxq)
{
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	mtk_dpmaif_rxq_push_work_exit(rxq);
#endif
	if (rxq->pit_base) {
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
		dma_unmap_single(DCB_TO_DEV(dcb), rxq->pit_dma_addr,
				 rxq->pit_cnt * sizeof(*rxq->pit_base), DMA_TO_DEVICE);
#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
		xiaomi_free_dpmaif_memory(rxq->pit_base);
#else
		devm_kfree(DCB_TO_DEV(dcb), rxq->pit_base);
#endif
#else
		dma_free_coherent(DCB_TO_DEV(dcb),
				  rxq->pit_cnt * sizeof(*rxq->pit_base),
				  rxq->pit_base, rxq->pit_dma_addr);
#endif
		rxq->pit_base = NULL;
	}
	wakeup_source_unregister(rxq->ws);
}

static int mtk_dpmaif_sw_stop_rxq(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_rxq *rxq)
{
	/* Rxq done process will check this flag, if rxq->started is false, process will stop. */
	rxq->started = false;
	MTK_INFO(DCB_TO_MDEV(dcb), "rxq%u pit:r=%u,w=%u,rel=%u\n",
		 rxq->id, rxq->pit_rd_idx, rxq->pit_wr_idx,
		rxq->pit_rel_rd_idx);

	/* Make sure rxq->started value update done. */
	smp_mb();

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	flush_workqueue(rxq->steer_wq);
#endif
	return 0;
}

static void mtk_dpmaif_sw_stop_rx(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char rxq_cnt = dcb->res_cfg->rxq_cnt;
	struct dpmaif_rxq *rxq;
	int i;

	/* Stop all rx process. */
	for (i = 0; i < rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		mtk_dpmaif_sw_stop_rxq(dcb, rxq);
	}
	/* Wait rxq process done. */
	dcb->port_ops->notify(dcb->data_blk, DATA_EVT_RX_SYNC, 0xff);
}

static void mtk_dpmaif_sw_start_rx(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char rxq_cnt = dcb->res_cfg->rxq_cnt;
	struct dpmaif_rxq *rxq;
	int i;

	for (i = 0; i < rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		rxq->started = true;
	}
}

static void mtk_dpmaif_sw_reset_rxq(struct dpmaif_rxq *rxq)
{
	memset(rxq->pit_base, 0x00, (rxq->pit_cnt * sizeof(*rxq->pit_base)));
	memset(&rxq->rx_record, 0x00, sizeof(rxq->rx_record));

	rxq->started = false;
	rxq->pit_wr_idx = 0;
	rxq->pit_rd_idx = 0;
	rxq->pit_rel_rd_idx = 0;
	rxq->pit_seq_expect = 0;
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	atomic_set(&rxq->pit_rel_cnt, 0);
#else
	rxq->pit_rel_cnt = 0;
#endif
	rxq->pit_cnt_err_intr_set = false;
	rxq->pit_seq_fail_cnt = 0;
}

static void mtk_dpmaif_rx_res_reset(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char rxq_cnt = dcb->res_cfg->rxq_cnt;
	struct dpmaif_rxq *rxq;
	int i;

	for (i = 0; i < rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		mtk_dpmaif_sw_reset_rxq(rxq);
	}
}

static int mtk_dpmaif_rx_res_init(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char rxq_cnt = dcb->res_cfg->rxq_cnt;
	struct dpmaif_rxq *rxq;
	int i, j;
	int ret;

	dcb->rxqs = devm_kcalloc(DCB_TO_DEV(dcb), rxq_cnt, sizeof(*rxq), GFP_KERNEL);
	if (!dcb->rxqs) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate rxqs\n");
		return -ENOMEM;
	}

	for (i = 0; i < rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		rxq->id = i;
		rxq->dcb = dcb;
		ret = mtk_dpmaif_rxq_init(dcb, rxq);
		if (ret < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to init rxq%u resource\n", rxq->id);
			goto exit_rxq;
		}
	}

	return 0;

exit_rxq:
	for (j = i - 1; j >= 0; j--)
		mtk_dpmaif_rxq_exit(dcb, &dcb->rxqs[j]);

	devm_kfree(DCB_TO_DEV(dcb), dcb->rxqs);
	dcb->rxqs = NULL;

	return ret;
}

static void mtk_dpmaif_rx_res_exit(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char rxq_cnt = dcb->res_cfg->rxq_cnt;
	int i;

	for (i = 0; i < rxq_cnt; i++)
		mtk_dpmaif_rxq_exit(dcb, &dcb->rxqs[i]);

	devm_kfree(DCB_TO_DEV(dcb), dcb->rxqs);
	dcb->rxqs = NULL;
}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
/*
 * Merge all register WRITE operations,
 * including adding DRB count, PIT count and BAT count.
 */
static void mtk_dpmaif_doorbell_work(struct work_struct *work)
{
	struct dpmaif_bat_ring *bat_ring;
	struct mtk_dpmaif_ctlb *dcb;
	unsigned int doorbell_cnt;
	int i, ret;

	dcb = container_of(work, struct mtk_dpmaif_ctlb, doorbell_work);
	trace_mtk_tput_data_tx("tx p6");

	pm_runtime_get_sync(DCB_TO_DEV(dcb));
	mtk_pm_ds_lock(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF);

	ret = mtk_pm_ds_wait_complete(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF);
	if (unlikely(ret < 0)) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to wait ds_lock\n");
		mtk_except_linkerr_type = 3;
		mtk_dpmaif_common_err_handle(dcb, true);
		goto out;
	}

	/* First of all, send Tx packets. */
	for (i = 0; i < dcb->res_cfg->txq_cnt; i++) {
		doorbell_cnt = atomic_read(&dcb->txqs[i].to_submit_cnt);
		if (doorbell_cnt > 0) {
#ifdef CONFIG_DEBUG_FS
			if (test_bit(DPMAIF_DUMP_DRB, &dcb->dump_flag))
				dpmaif_dump_drb(dcb, &dcb->txqs[i], 6);
#endif
			ret = mtk_dpmaif_drv_send_doorbell(dcb->drv_info,
							   DPMAIF_DRB, i, doorbell_cnt);
			if (unlikely(ret < 0)) {
				MTK_ERR(DCB_TO_MDEV(dcb), "Failed to send txq%d doorbell\n", i);
				mtk_dpmaif_common_err_handle(dcb, true);
				goto out;
			}
			atomic_sub(doorbell_cnt, &dcb->txqs[i].to_submit_cnt);
			MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
				"txq%u doorbell drb cnt=%u\n",
				    dcb->txqs[i].id, doorbell_cnt);
		}
	}

	/* recycle PIT */
	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		doorbell_cnt = atomic_read(&dcb->rxqs[i].pit_rel_cnt);
		if (doorbell_cnt > 0) {
			ret = mtk_dpmaif_drv_send_doorbell(dcb->drv_info,
							   DPMAIF_PIT, i, doorbell_cnt);
			if (unlikely(ret < 0)) {
				MTK_ERR(DCB_TO_MDEV(dcb), "Failed to send pit%d doorbell\n", i);
				mtk_dpmaif_common_err_handle(dcb, true);
				goto out;
			}
			atomic_sub(doorbell_cnt, &dcb->rxqs[i].pit_rel_cnt);

			if (dcb->rxqs[i].pit_cnt_err_intr_set) {
				dcb->rxqs[i].pit_cnt_err_intr_set = false;
				mtk_dpmaif_drv_intr_complete(dcb->drv_info,
							     DPMAIF_INTR_DL_PITCNT_LEN_ERR, i, 0);
			}
			MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_4,
				"rxq%u, doorbell pit cnt=%u\n",
					dcb->rxqs[i].id, doorbell_cnt);
		}
	}

	/* recycle BAT */
	bat_ring = &dcb->bat_info.normal_bat_ring;
	doorbell_cnt = atomic_read(&bat_ring->bat_rel_cnt);
	if (doorbell_cnt > 0) {
		ret = mtk_dpmaif_drv_send_doorbell(dcb->drv_info, DPMAIF_BAT, 0, doorbell_cnt);
		if (unlikely(ret < 0)) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to send bat doorbell\n");
			mtk_dpmaif_common_err_handle(dcb, true);
			goto out;
		}
		atomic_sub(doorbell_cnt, &bat_ring->bat_rel_cnt);

		if (bat_ring->bat_cnt_err_intr_set) {
			bat_ring->bat_cnt_err_intr_set = false;
			mtk_dpmaif_drv_intr_complete(dcb->drv_info,
						     DPMAIF_INTR_DL_BATCNT_LEN_ERR, 0, 0);
		}
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_4,
			"bat%u, doorbell bat cnt=%u\n",
				bat_ring->type, doorbell_cnt);
	}
	/* recycle FRAG BAT */
	if (dcb->bat_info.frag_bat_enabled) {
		bat_ring = &dcb->bat_info.frag_bat_ring;
		doorbell_cnt = atomic_read(&bat_ring->bat_rel_cnt);
		if (doorbell_cnt > 0) {
			ret = mtk_dpmaif_drv_send_doorbell(dcb->drv_info,
							   DPMAIF_FRAG, 0, doorbell_cnt);
			if (unlikely(ret < 0)) {
				MTK_ERR(DCB_TO_MDEV(dcb), "Failed to send frag_bat doorbell\n");
				mtk_dpmaif_common_err_handle(dcb, true);
				goto out;
			}
			atomic_sub(doorbell_cnt, &bat_ring->bat_rel_cnt);

			if (bat_ring->bat_cnt_err_intr_set) {
				bat_ring->bat_cnt_err_intr_set = false;
				mtk_dpmaif_drv_intr_complete(dcb->drv_info,
							     DPMAIF_INTR_DL_FRGCNT_LEN_ERR, 0, 0);
			}
			MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_4,
				"bat%u, doorbell frag_bat cnt=%u\n",
					bat_ring->type, doorbell_cnt);
		}
	}

out:
	mtk_pm_ds_unlock(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF, false);
	pm_runtime_put_sync(DCB_TO_DEV(dcb));
}
#endif

#ifndef CONFIG_REG_ACCESS_OPTIMIZE
static void mtk_dpmaif_tx_doorbell(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct mtk_dpmaif_ctlb *dcb;
	unsigned int to_submit_cnt;
	struct dpmaif_txq *txq;
	int ret;

	txq = container_of(dwork, struct dpmaif_txq, doorbell_work);
	dcb = txq->dcb;

	pm_runtime_get_sync(DCB_TO_DEV(dcb));
	mtk_pm_ds_lock(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF);

	ret = mtk_pm_ds_wait_complete(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF);
	if (unlikely(ret < 0)) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to wait ds_lock\n");
		mtk_dpmaif_common_err_handle(dcb, true);
		goto out;
	}

	to_submit_cnt = atomic_read(&txq->to_submit_cnt);

	if (to_submit_cnt > 0) {
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
			"txq%u doorbell drb cnt=%u\n", txq->id, to_submit_cnt);
#ifdef CONFIG_DEBUG_FS
		if (test_bit(DPMAIF_DUMP_DRB, &dcb->dump_flag))
			dpmaif_dump_drb(dcb, txq, 6);
#endif
		ret = mtk_dpmaif_drv_send_doorbell(dcb->drv_info,
						   DPMAIF_DRB, txq->id, to_submit_cnt);
		if (unlikely(ret < 0)) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to send txq%u doorbell\n", txq->id);
			mtk_dpmaif_common_err_handle(dcb, true);
		}

		atomic_sub(to_submit_cnt, &txq->to_submit_cnt);
	}

out:
	mtk_pm_ds_unlock(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF, false);
	pm_runtime_put_sync(DCB_TO_DEV(dcb));

	trace_mtk_tput_data_tx("tx p6");
}
#endif

static unsigned int mtk_dpmaif_poll_tx_drb(struct dpmaif_txq *txq)
{
	unsigned short old_sw_rd_idx, new_hw_rd_idx;
	struct mtk_dpmaif_ctlb *dcb = txq->dcb;
	unsigned int drb_cnt;
	int ret;

	old_sw_rd_idx = txq->drb_rd_idx;
	ret = mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_DRB_RIDX, txq->id);
	if (unlikely(ret < 0)) {
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Failed to read txq%u drb_rd_idx, ret=%d\n", txq->id, ret);
		mtk_dpmaif_common_err_handle(dcb, true);
		return 0;
	}

	new_hw_rd_idx = ret;

	if (old_sw_rd_idx <= new_hw_rd_idx)
		drb_cnt = new_hw_rd_idx - old_sw_rd_idx;
	else
		drb_cnt = txq->drb_cnt - old_sw_rd_idx + new_hw_rd_idx;

	txq->drb_rd_idx = new_hw_rd_idx;

	return drb_cnt;
}

static void mtk_dpmaif_wake_up_tx_srv(struct dpmaif_tx_srv *tx_srv)
{
	wake_up(&tx_srv->wait);
}

static int mtk_dpmaif_tx_rel_internal(struct dpmaif_txq *txq,
				      unsigned int rel_cnt, unsigned int *real_rel_cnt)
{
	struct dpmaif_pd_drb *cur_drb = NULL, *drb_base = txq->drb_base;
	struct mtk_dpmaif_ctlb *dcb = txq->dcb;
	struct dpmaif_drb_skb *cur_drb_skb;
	struct dpmaif_msg_drb *msg_drb;
	struct dpmaif_tx_srv *tx_srv;
	struct sk_buff *skb_free;
	unsigned short cur_idx;
	unsigned char srv_id;
	unsigned int i;

	srv_id = dcb->res_cfg->tx_vq_srv_map[txq->id];
	tx_srv = &dcb->tx_srvs[srv_id];
	cur_idx = txq->drb_rel_rd_idx;
	for (i = 0 ; i < rel_cnt; i++) {
		cur_drb = drb_base + cur_idx;
		cur_drb_skb = txq->sw_drb_base + cur_idx;
		if (FIELD_GET(DRB_PD_DTYP, le32_to_cpu(cur_drb->pd_header)) == PD_DRB) {
			dma_unmap_single(DCB_TO_DEV(dcb), cur_drb_skb->data_dma_addr,
					 cur_drb_skb->data_len, DMA_TO_DEVICE);

			/* The last one drb entry of one tx packet, so, skb will be released. */
			if (FIELD_GET(DRB_PD_CONT, le32_to_cpu(cur_drb->pd_header)) ==
			    DPMAIF_DRB_LASTONE) {
				skb_free = cur_drb_skb->skb;
				if (!skb_free) {
					MTK_ERR(DCB_TO_MDEV(dcb),
						"txq%u pkt(%u), drb check fail,w=%u,r=%u,rel=%u,cnt=%u\n",
						txq->id, cur_idx, txq->drb_wr_idx,
						txq->drb_rd_idx, txq->drb_rel_rd_idx, rel_cnt);

					mtk_dpmaif_common_err_handle(dcb, false);
					return -DATA_FLOW_CHK_ERR;
				}

#ifdef CONFIG_DATA_TEST_MODE
				ul_stats_update(dcb, TX_REC_HW_DONE, skb_free->len);
#endif
				dev_kfree_skb_any(skb_free);
				dcb->traffic_stats.tx_hw_packets[txq->id]++;
			}
		} else {
			msg_drb = (struct dpmaif_msg_drb *)cur_drb;
			txq->last_ch_id = FIELD_GET(DRB_MSG_CHNL_ID,
						    le32_to_cpu(msg_drb->msg_header2));
		}

		cur_drb_skb->skb = NULL;
		cur_idx = mtk_dpmaif_ring_buf_get_next_idx(txq->drb_cnt, cur_idx);
		txq->drb_rel_rd_idx = cur_idx;

		clear_bit(txq->id, &tx_srv->txq_drb_lack_sta);
		atomic_inc(&txq->budget);
		if (atomic_read(&txq->budget) > txq->drb_cnt / 8) {
			mtk_dpmaif_wake_up_tx_srv(tx_srv);
			dcb->port_ops->notify(dcb->data_blk, DATA_EVT_TX_START, (u64)1 << txq->id);
		}
	}

	*real_rel_cnt = i;

	if (likely(cur_drb)) {
		if (unlikely(FIELD_GET(DRB_PD_CONT, le32_to_cpu(cur_drb->pd_header)) !=
			DPMAIF_DRB_LASTONE)) {
			MTK_WARN(DCB_TO_MDEV(dcb), "txq%u done, last one c_bit != 0\n", txq->id);
			mtk_dpmaif_common_err_handle(dcb, true);
		}
	}

	return 0;
}

static int mtk_dpmaif_tx_rel(struct dpmaif_txq *txq)
{
	struct mtk_dpmaif_ctlb *dcb = txq->dcb;
	unsigned int real_rel_cnt = 0;
	int ret = 0, rel_cnt;

#ifndef CONFIG_REG_ACCESS_OPTIMIZE
	/* Update drb_rd_idx. */
	mtk_dpmaif_poll_tx_drb(txq);
#endif

	rel_cnt = mtk_dpmaif_ring_buf_releasable(txq->drb_cnt, txq->drb_rel_rd_idx,
						 txq->drb_rd_idx);

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
		"txq%u drb: w=%u,r=%u,rel=%u, rel_cnt=%u\n",
		    txq->id, txq->drb_wr_idx, txq->drb_rd_idx, txq->drb_rel_rd_idx, rel_cnt);

	if (likely(rel_cnt > 0)) {
		/* Release tx data buffer. */
		ret = mtk_dpmaif_tx_rel_internal(txq, rel_cnt, &real_rel_cnt);
		dcb->traffic_stats.tx_done_last_cnt[txq->id] = real_rel_cnt;
		trace_mtk_tput_data_drb_rel(txq->id, real_rel_cnt, (int)atomic_read(&txq->budget));
	}

	return ret;
}

static void mtk_dpmaif_tx_done(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct mtk_dpmaif_ctlb *dcb;
	struct dpmaif_txq *txq;

	txq = container_of(dwork, struct dpmaif_txq, tx_done_work);
	dcb = txq->dcb;

	dcb->traffic_stats.tx_done_last_time[txq->id] = local_clock();

	pm_runtime_get(DCB_TO_DEV(dcb));

	/* Recycle drb and release hardware tx done buffer around drb. */
	mtk_dpmaif_tx_rel(txq);

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	/* try best to recycle drb */
	if (txq->drb_poll_enable && mtk_dpmaif_poll_tx_drb(txq) > 0) {
		mtk_dpmaif_drv_intr_complete(dcb->drv_info, DPMAIF_INTR_UL_DONE,
					     txq->id, DPMAIF_CLEAR_INTR);
		queue_delayed_work(dcb->tx_done_wq, &txq->tx_done_work, msecs_to_jiffies(0));
	} else {
		mtk_dpmaif_drv_intr_complete(dcb->drv_info, DPMAIF_INTR_UL_DONE,
					     txq->id, DPMAIF_UNMASK_INTR);
	}
#else
	/* try best to recycle drb */
	if (mtk_dpmaif_poll_tx_drb(txq) > 0) {
		mtk_dpmaif_drv_intr_complete(dcb->drv_info, DPMAIF_INTR_UL_DONE,
					     txq->id, DPMAIF_CLEAR_INTR);
		queue_delayed_work(dcb->tx_done_wq, &txq->tx_done_work, msecs_to_jiffies(0));
	} else {
		mtk_dpmaif_drv_intr_complete(dcb->drv_info, DPMAIF_INTR_UL_DONE,
					     txq->id, DPMAIF_UNMASK_INTR);
	}
#endif

	pm_runtime_put(DCB_TO_DEV(dcb));
}

static int mtk_dpmaif_txq_init(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_txq *txq)
{
	unsigned int drb_cnt = dcb->res_cfg->drb_cnt[txq->id];
	int ret;

	atomic_set(&txq->budget, drb_cnt);
	atomic_set(&txq->to_submit_cnt, 0);
	txq->drb_cnt = drb_cnt;
	txq->drb_wr_idx = 0;
	txq->drb_rd_idx = 0;
	txq->drb_rel_rd_idx = 0;
	txq->dma_map_errs = 0;
	txq->last_ch_id = 0;
	txq->doorbell_delay = dcb->res_cfg->txq_doorbell_delay[txq->id];
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	txq->burst_submit_cnt = dcb->res_cfg->txq_burst_pkts[txq->id];
	txq->drb_poll_enable = false;
	atomic_set(&txq->drb_stats, 0);
#endif
	txq->intr_coalesce_frame = dcb->intr_coalesce.tx_coalesced_frames;

	/* Allocate DRB memory for HW and SW. */
	txq->drb_base = dma_alloc_coherent(DCB_TO_DEV(dcb),
					   txq->drb_cnt * sizeof(*txq->drb_base),
					   &txq->drb_dma_addr, GFP_KERNEL);
	if (!txq->drb_base) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate txq%u drb resource\n", txq->id);
		return -ENOMEM;
	}

	/* Allocate buffer for SW to record the skb information. */
#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
	txq->sw_drb_base =  xiaomi_kcalloc_dpmaif_memory(txq->drb_cnt, sizeof(*txq->sw_drb_base));
#else
	txq->sw_drb_base = devm_kcalloc(DCB_TO_DEV(dcb), txq->drb_cnt,
					sizeof(*txq->sw_drb_base), GFP_KERNEL);
#endif
	if (!txq->sw_drb_base) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Faile to allocate txq%u drb buffer\n", txq->id);
		ret = -ENOMEM;
		goto free_drb_buf;
	}

	MTK_INFO(DCB_TO_MDEV(dcb),
		 "txq%u: drb_base=0x%llx, drb_dma_addr=0x%llx, drb_cnt=%u, skb_ptr=0x%llx\n",
		txq->id, (u64)txq->drb_base, txq->drb_dma_addr,
		txq->drb_cnt, (u64)txq->sw_drb_base);

	/* It belongs to dcb->tx_done_wq. */
	INIT_DELAYED_WORK(&txq->tx_done_work, mtk_dpmaif_tx_done);

#ifndef CONFIG_REG_ACCESS_OPTIMIZE
	/* It belongs to dcb->tx_doorbell_wq. */
	INIT_DELAYED_WORK(&txq->doorbell_work, mtk_dpmaif_tx_doorbell);
#endif

	return 0;

free_drb_buf:
	dma_free_coherent(DCB_TO_DEV(dcb), txq->drb_cnt * sizeof(*txq->drb_base),
			  txq->drb_base, txq->drb_dma_addr);
	txq->drb_base = NULL;

	return ret;
}

static void mtk_dpmaif_txq_exit(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_txq *txq)
{
	struct dpmaif_drb_skb *drb_skb;
	int i;

	if (txq->drb_base) {
		dma_free_coherent(DCB_TO_DEV(dcb), txq->drb_cnt * sizeof(*txq->drb_base),
				  txq->drb_base, txq->drb_dma_addr);
		txq->drb_base = NULL;
	}

	if (txq->sw_drb_base) {
		for (i = 0; i < txq->drb_cnt; i++) {
			drb_skb = txq->sw_drb_base + i;
			if (drb_skb->skb) {
				/* Verify msg drb or payload drb,
				 * and only payload drb need to unmap dma.
				 */
				if (drb_skb->data_dma_addr)
					dma_unmap_single(DCB_TO_DEV(dcb),
							 drb_skb->data_dma_addr,
							 drb_skb->data_len, DMA_TO_DEVICE);
				if (drb_skb->is_last) {
					dev_kfree_skb_any(drb_skb->skb);
					drb_skb->skb = NULL;
				}
			}
		}
#ifdef XIAOMI_DPMAIF_MEMORY_OPTI
		xiaomi_free_dpmaif_memory(txq->sw_drb_base);
#else
		devm_kfree(DCB_TO_DEV(dcb), txq->sw_drb_base);
#endif
		txq->sw_drb_base = NULL;
	}
}

static int mtk_dpmaif_sw_wait_txq_stop(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_txq *txq)
{
	/* Wait tx done work done. */
	flush_delayed_work(&txq->tx_done_work);

#ifndef CONFIG_REG_ACCESS_OPTIMIZE
	/* Wait tx doorbell work done. */
	if (!dcb->dpmaif_suspending)
		flush_delayed_work(&txq->doorbell_work);
#endif

	MTK_INFO(DCB_TO_MDEV(dcb),
		 "txq%u, drb:r=%u,w=%u,rel=%u; to_submit_cnt=%d; budget=%d\n",
			txq->id, txq->drb_rd_idx, txq->drb_wr_idx, txq->drb_rel_rd_idx,
			atomic_read(&txq->to_submit_cnt),
			atomic_read(&txq->budget));

	return 0;
}

static void mtk_dpmaif_sw_wait_tx_stop(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char txq_cnt = dcb->res_cfg->txq_cnt;
	int i;

	/* Wait all tx handle complete */
	for (i = 0; i < txq_cnt; i++)
		mtk_dpmaif_sw_wait_txq_stop(dcb, &dcb->txqs[i]);

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	/* Wait send doorbell work done. */
	if (!dcb->dpmaif_suspending)
		flush_work(&dcb->doorbell_work);
#endif
}

static void mtk_dpmaif_sw_reset_txq(struct dpmaif_txq *txq)
{
	struct dpmaif_drb_skb *drb_skb;
	int i;

	/* Drop all tx buffer around drb. */
	for (i = 0; i < txq->drb_cnt; i++) {
		drb_skb = txq->sw_drb_base + i;
		if (drb_skb->skb) {
			if (drb_skb->data_dma_addr)
				dma_unmap_single(DCB_TO_DEV(txq->dcb), drb_skb->data_dma_addr,
						 drb_skb->data_len, DMA_TO_DEVICE);
			if (drb_skb->is_last) {
				dev_kfree_skb_any(drb_skb->skb);
				drb_skb->skb = NULL;
			}
		}
	}

	/* Reset all txq resource. */
	memset(txq->drb_base, 0x00, (txq->drb_cnt * sizeof(*txq->drb_base)));
	memset(txq->sw_drb_base, 0x00, (txq->drb_cnt * sizeof(*txq->sw_drb_base)));

	atomic_set(&txq->budget, txq->drb_cnt);
	atomic_set(&txq->to_submit_cnt, 0);
	txq->drb_rd_idx = 0;
	txq->drb_wr_idx = 0;
	txq->drb_rel_rd_idx = 0;
	txq->last_ch_id = 0;
}

static void mtk_dpmaif_tx_res_reset(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char txq_cnt = dcb->res_cfg->txq_cnt;
	struct dpmaif_txq *txq;
	int i;

	for (i = 0; i < txq_cnt; i++) {
		txq = &dcb->txqs[i];
		mtk_dpmaif_sw_reset_txq(txq);
	}
}

static int mtk_dpmaif_tx_res_init(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char txq_cnt = dcb->res_cfg->txq_cnt;
	struct dpmaif_txq *txq;
	int i, j;
	int ret;

	dcb->txqs = devm_kcalloc(DCB_TO_DEV(dcb), txq_cnt, sizeof(*txq), GFP_KERNEL);
	if (!dcb->txqs) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate txqs\n");
		return -ENOMEM;
	}

	for (i = 0; i < txq_cnt; i++) {
		txq = &dcb->txqs[i];
		txq->id = i;
		txq->dcb = dcb;
		ret = mtk_dpmaif_txq_init(dcb, txq);
		if (ret < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to init txq%d resource\n", txq->id);
			goto exit_txq;
		}
	}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	/* should be remove after tuning done */
	if (normal_prio_tth <= DPMAIF_MAX_UL_TTH) {
		dcb->txqs[0].doorbell_delay = normal_prio_tth;
		dcb->txqs[4].doorbell_delay = normal_prio_tth;
	}
	if (normal_prio_pth <= DPMAIF_MAX_UL_PTH) {
		dcb->txqs[0].burst_submit_cnt = normal_prio_pth;
		dcb->txqs[4].burst_submit_cnt = normal_prio_pth;
	}
	if (high_prio_tth <= DPMAIF_MAX_UL_TTH)
		dcb->txqs[1].doorbell_delay = high_prio_tth;

	if (high_prio_pth <= DPMAIF_MAX_UL_PTH)
		dcb->txqs[1].burst_submit_cnt = high_prio_pth;
#endif

	dcb->tx_done_wq = alloc_workqueue("dpmaif_tx_done_wq_%s",
					  WQ_UNBOUND | WQ_MEM_RECLAIM | WQ_HIGHPRI,
					  txq_cnt, DCB_TO_DEV_STR(dcb));
	if (!dcb->tx_done_wq) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate tx done workqueue\n");
		ret = -ENOMEM;
		goto exit_txq;
	}

#ifndef CONFIG_REG_ACCESS_OPTIMIZE
	dcb->tx_doorbell_wq = alloc_workqueue("dpmaif_tx_doorbell_wq_%s",
					      WQ_FREEZABLE | WQ_UNBOUND |
					      WQ_MEM_RECLAIM | WQ_HIGHPRI,
					      txq_cnt, DCB_TO_DEV_STR(dcb));
	if (!dcb->tx_doorbell_wq) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate tx doorbell workqueue\n");
		ret = -ENOMEM;
		goto flush_tx_doorbell_wq;
	}
#endif

	return 0;

#ifndef CONFIG_REG_ACCESS_OPTIMIZE
flush_tx_doorbell_wq:
	flush_workqueue(dcb->tx_done_wq);
	destroy_workqueue(dcb->tx_done_wq);
#endif

exit_txq:
	for (j = i - 1; j >= 0; j--)
		mtk_dpmaif_txq_exit(dcb, &dcb->txqs[j]);

	devm_kfree(DCB_TO_DEV(dcb), dcb->txqs);
	dcb->txqs = NULL;

	return ret;
}

static void mtk_dpmaif_tx_res_exit(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char txq_cnt = dcb->res_cfg->txq_cnt;
	struct dpmaif_txq *txq;
	int i;

	for (i = 0; i < txq_cnt; i++) {
		txq = &dcb->txqs[i];
		flush_delayed_work(&txq->tx_done_work);
#ifndef CONFIG_REG_ACCESS_OPTIMIZE
		flush_delayed_work(&txq->doorbell_work);
#endif
	}

#ifndef CONFIG_REG_ACCESS_OPTIMIZE
	if (dcb->tx_doorbell_wq) {
		flush_workqueue(dcb->tx_doorbell_wq);
		destroy_workqueue(dcb->tx_doorbell_wq);
		dcb->tx_doorbell_wq = NULL;
	}
#endif

	if (dcb->tx_done_wq) {
		flush_workqueue(dcb->tx_done_wq);
		destroy_workqueue(dcb->tx_done_wq);
		dcb->tx_done_wq = NULL;
	}

	for (i = 0; i < txq_cnt; i++)
		mtk_dpmaif_txq_exit(dcb, &dcb->txqs[i]);

	devm_kfree(DCB_TO_DEV(dcb), dcb->txqs);
	dcb->txqs = NULL;
}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
static int mtk_dpmaif_doorbell_task_init(struct mtk_dpmaif_ctlb *dcb)
{
	dcb->doorbell_wq =
		alloc_workqueue("dpmaif_doorbell_wq_%s",
				WQ_HIGHPRI | WQ_FREEZABLE | WQ_UNBOUND | WQ_MEM_RECLAIM,
				 1, DCB_TO_DEV_STR(dcb));

	if (!dcb->doorbell_wq) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate send doorbell workqueue\n");
		return -ENOMEM;
	}

	INIT_WORK(&dcb->doorbell_work, mtk_dpmaif_doorbell_work);

	hrtimer_init(&dcb->doorbell_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dcb->doorbell_timer.function = mtk_dpmaif_doorbell_timer_func;

	return 0;
}

static void mtk_dpmaif_doorbell_task_exit(struct mtk_dpmaif_ctlb *dcb)
{
	hrtimer_cancel(&dcb->doorbell_timer);

	if (dcb->doorbell_wq) {
		flush_workqueue(dcb->doorbell_wq);
		destroy_workqueue(dcb->doorbell_wq);
		dcb->doorbell_wq = NULL;
	}
}
#endif

static int mtk_dpmaif_sw_res_init(struct mtk_dpmaif_ctlb *dcb)
{
	int ret;

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	ret = mtk_dpmaif_doorbell_task_init(dcb);
	if (ret < 0) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to initialize common reource, ret=%d\n", ret);
		return ret;
	}
#endif

	ret = mtk_dpmaif_bat_res_init(dcb);
	if (ret < 0) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to initialize bat reource, ret=%d\n", ret);
		goto out;
	}

	ret = mtk_dpmaif_rx_res_init(dcb);
	if (ret < 0) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to initialize rx reource, ret=%d\n", ret);
		goto exit_rx_res;
	}

	ret = mtk_dpmaif_tx_res_init(dcb);
	if (ret < 0) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to initialize tx reource, ret=%d\n", ret);
		goto exit_tx_res;
	}

	return 0;

exit_tx_res:
	mtk_dpmaif_rx_res_exit(dcb);

exit_rx_res:
	mtk_dpmaif_bat_res_exit(dcb);

out:
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	mtk_dpmaif_doorbell_task_exit(dcb);
#endif

	return ret;
}

static void mtk_dpmaif_sw_res_exit(struct mtk_dpmaif_ctlb *dcb)
{
	mtk_dpmaif_tx_res_exit(dcb);
	mtk_dpmaif_rx_res_exit(dcb);
	mtk_dpmaif_bat_res_exit(dcb);
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	mtk_dpmaif_doorbell_task_exit(dcb);
#endif
}

static int mtk_dpmaif_bm_pool_init(struct mtk_dpmaif_ctlb *dcb)
{
	int ret;

	dcb->skb_pool = mtk_bm_pool_create(DCB_TO_MDEV(dcb), MTK_BUFF_SKB,
					   dcb->bat_info.normal_bat_ring.buf_size,
					   dcb->res_cfg->normal_bat_cnt,
					   MTK_BM_HIGH_PRIO, "dpmaif_skb_bm_pool_%s",
					   DCB_TO_DEV_STR(dcb));
	if (!dcb->skb_pool) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to create skb bm pool\n");
		return -ENOMEM;
	}

	if (dcb->bat_info.frag_bat_enabled) {
		dcb->page_pool = mtk_bm_pool_create(DCB_TO_MDEV(dcb), MTK_BUFF_PAGE,
						    dcb->bat_info.frag_bat_ring.buf_size,
						    dcb->res_cfg->frag_bat_cnt,
						    MTK_BM_HIGH_PRIO, "dpmaif_page_bm_pool_%s",
						    DCB_TO_DEV_STR(dcb));
		if (!dcb->page_pool) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to create page bm pool\n");
			ret = -ENOMEM;
			goto err_create_page_pool;
		}
	}

	return 0;

err_create_page_pool:
	mtk_bm_pool_destroy(DCB_TO_MDEV(dcb), dcb->skb_pool);
	dcb->skb_pool = NULL;

	return ret;
}

static void mtk_dpmaif_bm_pool_exit(struct mtk_dpmaif_ctlb *dcb)
{
	if (dcb->skb_pool)
		mtk_bm_pool_destroy(DCB_TO_MDEV(dcb), dcb->skb_pool);

	if (dcb->bat_info.frag_bat_enabled) {
		if (dcb->page_pool)
			mtk_bm_pool_destroy(DCB_TO_MDEV(dcb), dcb->page_pool);
	}
}

static bool mtk_dpmaif_all_vqs_empty_or_busy(struct dpmaif_tx_srv *tx_srv)
{
	bool is_empty_or_busy = true;
	struct dpmaif_vq *vq;
	int i;

	for (i = 0; i < tx_srv->vq_cnt; i++) {
		vq = tx_srv->vq[i];
		if (!skb_queue_empty(&vq->list) && !test_bit(vq->q_id, &tx_srv->txq_drb_lack_sta)) {
			is_empty_or_busy = false;
			break;
		}
	}

	return is_empty_or_busy;
}

static bool mtk_dpmaif_all_txqs_drb_lack(struct dpmaif_tx_srv *tx_srv)
{
	unsigned char q_cnt = tx_srv->dcb->res_cfg->tx_vq_cnt;
	unsigned char vq_cnt = 0;
	int i;

	for (i = 0; i < q_cnt; i++) {
		if (test_bit(i, &tx_srv->txq_drb_lack_sta)) {
			if (++vq_cnt == tx_srv->vq_cnt)
				return true;
		}
	}

	return false;
}

static void mtk_dpmaif_set_drb_msg(struct mtk_dpmaif_ctlb *dcb, unsigned char q_id,
				   unsigned short cur_idx, unsigned int pkt_len,
				   unsigned short count_l, unsigned char channel_id,
				   unsigned short network_type)
{
	struct dpmaif_msg_drb *drb = (struct dpmaif_msg_drb *)dcb->txqs[q_id].drb_base + cur_idx;
	unsigned int *tmp;
	u32 features;
	u32 txcsum;

#ifdef CONFIG_DATA_TEST_MODE
	if (dcb->test_mode_cfg.md_tput_mode != DPMAIF_MD_INVALID_MODE) {
		count_l = 0;

		/* md mode */
		count_l |= (dcb->test_mode_cfg.md_tput_mode << DPMAIF_MD_TPUT_MODE_OFFSET);

		/* Start/stop dl */
		count_l |= (dcb->test_mode_cfg.md_start_dl_tput << DPMAIF_MD_DL_CTL_OFFSET);

		/* Packets count per ms */
		count_l |= (dcb->test_mode_cfg.md_pkt_number_per_ms << DPMAIF_MD_PKT_OFFSET);
	}
#endif
	drb->msg_header1 = cpu_to_le32(FIELD_PREP(DRB_MSG_DTYP, MSG_DRB) |
				       FIELD_PREP(DRB_MSG_CONT, DPMAIF_DRB_MORE) |
				       FIELD_PREP(DRB_MSG_PKT_LEN, pkt_len));

	mtk_dpmaif_drv_feature_cmd(dcb->drv_info, DATA_HW_FEATURES_GET, &features);
	if (likely(features & DATA_HW_F_TXCSUM))
		txcsum = FIELD_PREP(DRB_MSG_L4_CHK, 1);
	else
		txcsum = FIELD_PREP(DRB_MSG_L4_CHK, 0);

	drb->msg_header2 = cpu_to_le32(FIELD_PREP(DRB_MSG_COUNT_L, count_l) |
				       FIELD_PREP(DRB_MSG_CHNL_ID, channel_id) |
				       txcsum | FIELD_PREP(DRB_MSG_NET_TYPE, network_type));
#ifdef CONFIG_DATA_TEST_MODE
	drb->msg_header2 |= cpu_to_le32(FIELD_PREP(DRB_MSG_NET_TYPE,
					dcb->test_mode_cfg.md_dl_tput_test_time));
#endif

	tmp = (unsigned int *)drb;
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
		"txq%u, drb msg(%u): 0x%x, 0x%x\n",
		    q_id, cur_idx, tmp[0], tmp[1]);
}

static void mtk_dpmaif_set_drb_payload(struct mtk_dpmaif_ctlb *dcb, unsigned char q_id,
				       unsigned short cur_idx, unsigned long long data_addr,
				       unsigned int pkt_size, char last_one)
{
	struct dpmaif_pd_drb *drb = dcb->txqs[q_id].drb_base + cur_idx;
	unsigned int *tmp;

	drb->pd_header = cpu_to_le32(FIELD_PREP(DRB_PD_DTYP, PD_DRB));
	if (last_one)
		drb->pd_header |= cpu_to_le32(FIELD_PREP(DRB_PD_CONT, DPMAIF_DRB_LASTONE));
	else
		drb->pd_header |= cpu_to_le32(FIELD_PREP(DRB_PD_CONT, DPMAIF_DRB_MORE));

	drb->pd_header |= cpu_to_le32(FIELD_PREP(DRB_PD_DATA_LEN, pkt_size));
	drb->addr_low = cpu_to_le32(lower_32_bits(data_addr));
	drb->addr_high = cpu_to_le32(upper_32_bits(data_addr));

	tmp = (unsigned int *)drb;
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
		"txq%u, drb pd(%u): 0x%x, 0x%x, 0x%x, 0x%x\n",
		    q_id, cur_idx, tmp[0], tmp[1], tmp[2], tmp[3]);
}

static void mtk_dpmaif_record_drb_skb(struct mtk_dpmaif_ctlb *dcb, unsigned char q_id,
				      unsigned short cur_idx, struct sk_buff *skb,
				      unsigned short is_msg, unsigned short is_frag,
				      unsigned short is_last, dma_addr_t data_dma_addr,
				      unsigned int data_len)
{
	struct dpmaif_drb_skb *drb_skb = dcb->txqs[q_id].sw_drb_base + cur_idx;

	drb_skb->skb = skb;
	drb_skb->data_dma_addr = data_dma_addr;
	drb_skb->data_len = data_len;
	drb_skb->drb_idx = cur_idx;
	drb_skb->is_msg = is_msg;
	drb_skb->is_frag = is_frag;
	drb_skb->is_last = is_last;
}

static void mtk_dpmaif_skb_dump(struct mtk_dpmaif_ctlb *dcb, struct sk_buff *skb, bool full_pkt)
{
	struct skb_shared_info *sh = skb_shinfo(skb);
	struct net_device *dev = skb->dev;
	struct sock *sk = skb->sk;
	bool has_mac, has_trans;
	int headroom, tailroom;

	headroom = skb_headroom(skb);
	tailroom = skb_tailroom(skb);

	has_mac = skb_mac_header_was_set(skb);
	has_trans = skb_transport_header_was_set(skb);

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
		"skb len=%u headroom=%u headlen=%u tailroom=%u\n",
		skb->len, headroom, skb_headlen(skb), tailroom);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
		"mac=(%d,%d) net=(%d,%d) trans=%d\n",
		has_mac ? skb->mac_header : -1,
		has_mac ? skb_mac_header_len(skb) : -1,
		skb->network_header,
		has_trans ? skb_network_header_len(skb) : -1,
		has_trans ? skb->transport_header : -1);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
		"shinfo(txflags=%u nr_frags=%u gso(size=%hu type=%u segs=%hu))\n",
		sh->tx_flags, sh->nr_frags,
		sh->gso_size, sh->gso_type, sh->gso_segs);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
		"csum(0x%x ip_summed=%u complete_sw=%u valid=%u level=%u)\n",
		skb->csum, skb->ip_summed, skb->csum_complete_sw,
		skb->csum_valid, skb->csum_level);
	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
		"hash(0x%x sw=%u l4=%u) proto=0x%04x pkttype=%u iif=%d\n",
		skb->hash, skb->sw_hash, skb->l4_hash,
		ntohs(skb->protocol), skb->pkt_type, skb->skb_iif);

	if (dev)
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
			"dev name=%s feat=%pNF\n", dev->name, &dev->features);
	if (sk)
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_MEMLOG_RG_1,
			"sk family=%hu type=%u proto=%u\n",
			sk->sk_family, sk->sk_type, sk->sk_protocol);
}

static int mtk_dpmaif_tx_fill_drb(struct mtk_dpmaif_ctlb *dcb,
				  unsigned char q_id, struct sk_buff *skb)
{
	unsigned short cur_idx, cur_backup_idx, is_frag, is_last;
	unsigned int send_drb_cnt, wt_cnt, payload_cnt;
	struct dpmaif_txq *txq = &dcb->txqs[q_id];
	struct dpmaif_drb_skb *cur_drb_skb;
	struct skb_shared_info *info;
	unsigned int data_len;
	dma_addr_t data_dma_addr;
	skb_frag_t *frag;
	void *data_addr;
	int i, ret;

#ifdef CONFIG_DEBUG_FS
	if (test_bit(DPMAIF_DUMP_TX_PKT, &dcb->dump_flag))
		mtk_dpmaif_skb_dump(dcb, skb, false);
#endif

#ifdef CONFIG_DATA_TEST_MODE
	ul_stats_update(dcb, TX_REC_SW_DONE, skb->len);
#endif
/***********************************xiaomi modem tput start ***/
	{
		extern __u64 g_ul_total;
		g_ul_total += skb->len;
	}
/***********************************xiaomi modem tput end ***/
	info = skb_shinfo(skb);
	if (unlikely(info->frag_list))
		MTK_WARN(DCB_TO_MDEV(dcb), "txq%d not support skb frag_list\n", q_id);

	send_drb_cnt = DPMAIF_SKB_CB(skb)->tx.drb_cnt;
	payload_cnt = send_drb_cnt - 1;
	cur_idx = txq->drb_wr_idx;
	cur_backup_idx = cur_idx;

	/* Update tx drb, a msg drb first, then payload drb. */
	/* Update and record payload drb information. */
	mtk_dpmaif_set_drb_msg(dcb, txq->id, cur_idx, skb->len, 0,
			       DPMAIF_SKB_CB(skb)->tx.intf_id, DPMAIF_SKB_CB(skb)->tx.network_type);
	mtk_dpmaif_record_drb_skb(dcb, txq->id, cur_idx, skb, 1, 0, 0, 0, 0);

	/* Payload drb: skb->data + frags[]. */
	cur_idx = mtk_dpmaif_ring_buf_get_next_idx(txq->drb_cnt, cur_idx);
	for (wt_cnt = 0; wt_cnt < payload_cnt; wt_cnt++) {
		/* Get data_addr and data_len. */
		if (wt_cnt == 0) {
			data_len = skb_headlen(skb);
			data_addr = skb->data;
			is_frag = 0;
		} else {
			frag = info->frags + wt_cnt - 1;
			data_len = skb_frag_size(frag);
			data_addr = skb_frag_address(frag);
			is_frag = 1;
		}

		if (wt_cnt == payload_cnt - 1)
			is_last = 1;
		else
			is_last = 0;

		data_dma_addr = dma_map_single(DCB_TO_DEV(dcb),
					       data_addr, data_len, DMA_TO_DEVICE);
		ret = dma_mapping_error(DCB_TO_DEV(dcb), data_dma_addr);
		if (unlikely(ret)) {
			MTK_WARN(DCB_TO_MDEV(dcb), "dma mapping fail\n");
			txq->dma_map_errs++;
			ret = -DATA_DMA_MAP_ERR;
			goto unmap_dma;
		}

		/* Update and record payload drb information. */
		mtk_dpmaif_set_drb_payload(dcb, txq->id, cur_idx, data_dma_addr, data_len, is_last);
		mtk_dpmaif_record_drb_skb(dcb, txq->id, cur_idx, skb, 0, is_frag, is_last,
					  data_dma_addr, data_len);

		cur_idx = mtk_dpmaif_ring_buf_get_next_idx(txq->drb_cnt, cur_idx);
	}

	txq->drb_wr_idx += send_drb_cnt;
	if (txq->drb_wr_idx >= txq->drb_cnt)
		txq->drb_wr_idx -= txq->drb_cnt;

	/* Make sure host write memory done before adding to_submit_cnt */
	smp_mb();

	atomic_sub(send_drb_cnt, &txq->budget);
	atomic_add(send_drb_cnt, &txq->to_submit_cnt);

	return 0;

unmap_dma:
	mtk_dpmaif_record_drb_skb(dcb, txq->id, cur_backup_idx, NULL, 0, 0, 0, 0, 0);
	cur_backup_idx = mtk_dpmaif_ring_buf_get_next_idx(txq->drb_cnt, cur_backup_idx);
	for (i = 0; i < wt_cnt; i++) {
		cur_drb_skb = txq->sw_drb_base + cur_backup_idx;

		dma_unmap_single(DCB_TO_DEV(dcb),
				 cur_drb_skb->data_dma_addr, cur_drb_skb->data_len,
				 DMA_TO_DEVICE);

		cur_backup_idx = mtk_dpmaif_ring_buf_get_next_idx(txq->drb_cnt, cur_backup_idx);
		mtk_dpmaif_record_drb_skb(dcb, txq->id, cur_backup_idx, NULL, 0, 0, 0, 0, 0);
	}

	return ret;
}

static int mtk_dpmaif_tx_update_ring(struct mtk_dpmaif_ctlb *dcb, struct dpmaif_tx_srv *tx_srv,
				     struct dpmaif_vq *vq)
{
	struct dpmaif_txq *txq = &dcb->txqs[vq->q_id];
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	bool in_tcp_slow_start = false;
#endif
	unsigned char q_id = vq->q_id;
	unsigned char skb_drb_cnt;
	int i, drb_available_cnt;
	struct sk_buff *skb;
	int ret;

	drb_available_cnt = mtk_dpmaif_ring_buf_writable(txq->drb_cnt,
							 txq->drb_rel_rd_idx, txq->drb_wr_idx);

	for (i = 0; i < DPMAIF_SKB_TX_WEIGHT; i++) {
		skb = skb_dequeue(&vq->list);
		if (!skb) {
			ret = 0;
			break;
		}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
		/* if any packet in tcp slow start, the doorbell delay timer will be set to 0 */
		if (DPMAIF_SKB_CB(skb)->tx.in_tcp_slow_start)
			in_tcp_slow_start = true;
#endif

		skb_drb_cnt = DPMAIF_SKB_CB(skb)->tx.drb_cnt;
		if (drb_available_cnt < skb_drb_cnt) {
			skb_queue_head(&vq->list, skb);
			set_bit(q_id, &tx_srv->txq_drb_lack_sta);
			ret = -DATA_LOW_MEM_DRB;
			MTK_WARN_RATELIMITED(DCB_TO_MDEV(dcb), "txq%u drb lack\n", q_id);
			break;
		}

		ret = mtk_dpmaif_tx_fill_drb(dcb, q_id, skb);
		if (ret < 0) {
			skb_queue_head(&vq->list, skb);
			break;
		}
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
		atomic_add(skb_drb_cnt, &txq->drb_stats);
#endif
		drb_available_cnt -= skb_drb_cnt;
		dcb->traffic_stats.tx_sw_packets[q_id]++;
	}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	if (in_tcp_slow_start)
		txq->exit_tcp_ss_counter = doorbell_reset_count;
#endif

	trace_mtk_tput_data_drb_fill(tx_srv->id, q_id, i,
				     skb_queue_len(&vq->list), drb_available_cnt);
	return ret;
}

static struct dpmaif_vq *mtk_dpmaif_srv_select_vq(struct dpmaif_tx_srv *tx_srv)
{
	struct dpmaif_vq *vq;
	int i;

	/* Round robin select tx vqs. */
	for (i = 0; i < tx_srv->vq_cnt; i++) {
		tx_srv->cur_vq_id = tx_srv->cur_vq_id % tx_srv->vq_cnt;
		vq = tx_srv->vq[tx_srv->cur_vq_id];
		tx_srv->cur_vq_id++;
		if (!skb_queue_empty(&vq->list) && !test_bit(vq->q_id, &tx_srv->txq_drb_lack_sta))
			return vq;
	}

	return NULL;
}

static void mtk_dpmaif_tx(struct dpmaif_tx_srv *tx_srv)
{
	struct mtk_dpmaif_ctlb *dcb = tx_srv->dcb;
#ifndef CONFIG_REG_ACCESS_OPTIMIZE
	struct dpmaif_txq *txq;
#endif
	struct dpmaif_vq *vq;
	int ret;

	do {
		if (likely(!dcb->err_event)) {
			vq = mtk_dpmaif_srv_select_vq(tx_srv);
			if (!vq)
				break;

			ret = mtk_dpmaif_tx_update_ring(dcb, tx_srv, vq);
			if (unlikely(ret < 0)) {
				if (ret == -DATA_LOW_MEM_DRB &&
				    mtk_dpmaif_all_txqs_drb_lack(tx_srv)) {
					MTK_WARN_RATELIMITED(DCB_TO_MDEV(dcb),
							     "tx_srv%u all txqs drb lack\n",
							     tx_srv->id);
					usleep_range(50, 100);
				}
			}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
			mtk_dpmaif_book_tx_doorbell(dcb, &dcb->txqs[vq->q_id]);
#else
			/* Kick off tx doorbell workqueue. */
			txq = &dcb->txqs[vq->q_id];
			if (atomic_read(&txq->to_submit_cnt) > 0) {
				trace_mtk_tput_data_tx("tx p5");
				queue_delayed_work(dcb->tx_doorbell_wq, &txq->doorbell_work,
						   msecs_to_jiffies(txq->doorbell_delay));
			}
#endif
		}
		cond_resched();
	} while (!kthread_should_stop() && (dcb->dpmaif_state == DPMAIF_STATE_PWRON));
}

static int mtk_dpmaif_tx_thread(void *arg)
{
	struct dpmaif_tx_srv *tx_srv = arg;
	struct mtk_dpmaif_ctlb *dcb;
	int ret;

	dcb = tx_srv->dcb;
	set_user_nice(current, tx_srv->prio);
	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(tx_srv->wait,
					       (!mtk_dpmaif_all_vqs_empty_or_busy(tx_srv) &&
					       (dcb->dpmaif_state == DPMAIF_STATE_PWRON)) ||
					       kthread_should_stop());
		trace_mtk_tput_data_tx("tx p4");

		if (ret == -ERESTARTSYS)
			continue;

		/* Send packets of all tx virtual queues belong to the tx service. */
		mtk_dpmaif_tx(tx_srv);
	}

	return 0;
}
static int mtk_dpmaif_tx_srvs_start(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char srvs_cnt = dcb->res_cfg->tx_srv_cnt;
	struct dpmaif_tx_srv *tx_srv;
	int i, j, ret;
	for (i = 0; i < srvs_cnt; i++) {
		tx_srv = &dcb->tx_srvs[i];
		tx_srv->cur_vq_id = 0;
		tx_srv->txq_drb_lack_sta = 0;
		if (tx_srv->srv) {
			MTK_WARN(DCB_TO_MDEV(dcb), "The tx_srv:%d existed", i);
			continue;
		}
		tx_srv->srv = kthread_run(mtk_dpmaif_tx_thread,
					  tx_srv, "dpmaif_tx_srv%u_%s",
					  tx_srv->id, DCB_TO_DEV_STR(dcb));
		if (IS_ERR(tx_srv->srv)) {
			MTK_ERR(DCB_TO_MDEV(dcb),
				"Failed to alloc dpmaif tx_srv%u\n", tx_srv->id);
			ret = PTR_ERR(tx_srv->srv);
			tx_srv->srv = NULL; //add by xiaomi for FRAN1J-5250
			goto free_tx_srvs;
		}
	}

	return 0;

free_tx_srvs:
	for (j = i - 1; j >= 0; j--) {
		if (tx_srv->srv)
			kthread_stop(tx_srv->srv);
		tx_srv->srv = NULL;
	}

	return ret;
}

static void mtk_dpmaif_tx_srvs_stop(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char srvs_cnt = dcb->res_cfg->tx_srv_cnt;
	struct dpmaif_tx_srv *tx_srv;
	int i;

	for (i = 0; i < srvs_cnt; i++) {
		tx_srv = &dcb->tx_srvs[i];
		if (tx_srv->srv)
			kthread_stop(tx_srv->srv);

		tx_srv->srv = NULL;
	}
}

static int mtk_dpmaif_tx_srvs_init(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char srvs_cnt = dcb->res_cfg->tx_srv_cnt;
	unsigned char vqs_cnt = dcb->res_cfg->tx_vq_cnt;
	struct dpmaif_tx_srv *tx_srv;
	struct dpmaif_vq *tx_vq;
	int i, j, vq_id;
	int ret;

	/* Initialize all data packet tx vitrual queue. */
	dcb->tx_vqs = devm_kcalloc(DCB_TO_DEV(dcb), vqs_cnt, sizeof(*dcb->tx_vqs), GFP_KERNEL);
	if (!dcb->tx_vqs) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to alloc tx_vqs\n");
		return -ENOMEM;
	}

	for (i = 0; i < vqs_cnt; i++) {
		tx_vq = &dcb->tx_vqs[i];
		tx_vq->q_id = i;
		tx_vq->max_len = DEFAULT_TX_QUEUE_LEN;
		skb_queue_head_init(&tx_vq->list);
	}

	/* Initialize all data packet tx services. */
	dcb->tx_srvs = devm_kcalloc(DCB_TO_DEV(dcb), srvs_cnt, sizeof(*dcb->tx_srvs), GFP_KERNEL);
	if (!dcb->tx_srvs) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to alloc tx_srvs\n");
		ret = -ENOMEM;
		goto free_tx_srvs;
	}

	for (i = 0; i < srvs_cnt; i++) {
		tx_srv = &dcb->tx_srvs[i];
		tx_srv->dcb = dcb;
		tx_srv->id = i;
		tx_srv->prio = dcb->res_cfg->srv_prio_tbl[i];
		tx_srv->cur_vq_id = 0;
		tx_srv->txq_drb_lack_sta = 0;
		init_waitqueue_head(&tx_srv->wait);

		/* Set virtual queues and tx service mapping. */
		vq_id = 0;
		for (j = 0; j < vqs_cnt; j++) {
			if (tx_srv->id == dcb->res_cfg->tx_vq_srv_map[j]) {
				tx_srv->vq[vq_id] = &dcb->tx_vqs[j];
				vq_id++;
				MTK_INFO(DCB_TO_MDEV(dcb),
					 "tx_srv%u: +vq%d, vq_cnt=%u\n", tx_srv->id,
					j, vq_id);
			}
		}

		tx_srv->vq_cnt = vq_id;
		if (unlikely(!tx_srv->vq_cnt)) {
			MTK_WARN(DCB_TO_MDEV(dcb),
				 "Invalid vq_cnt of tx_srv%u\n", tx_srv->id);
			MTK_BUG(DCB_TO_MDEV(dcb));
		}
	}

	return 0;

free_tx_srvs:
	devm_kfree(DCB_TO_DEV(dcb), dcb->tx_vqs);
	dcb->tx_vqs = NULL;

	return ret;
}

static void mtk_dpmaif_tx_vqs_reset(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char vqs_cnt = dcb->res_cfg->tx_vq_cnt;
	struct dpmaif_vq *tx_vq;
	int i;

	/* Drop all packet in tx virtual queues. */
	for (i = 0; i < vqs_cnt; i++) {
		tx_vq = &dcb->tx_vqs[i];
		if (tx_vq)
			skb_queue_purge(&tx_vq->list);
	}
}

static void mtk_dpmaif_tx_srvs_exit(struct mtk_dpmaif_ctlb *dcb)
{
	mtk_dpmaif_tx_srvs_stop(dcb);

	devm_kfree(DCB_TO_DEV(dcb), dcb->tx_srvs);
	dcb->tx_srvs = NULL;

	/* Drop all packet in tx virtual queues. */
	mtk_dpmaif_tx_vqs_reset(dcb);

	devm_kfree(DCB_TO_DEV(dcb), dcb->tx_vqs);
	dcb->tx_vqs = NULL;
}

static void mtk_dpmaif_trans_enable(struct mtk_dpmaif_ctlb *dcb)
{
	mtk_dpmaif_sw_start_rx(dcb);
	mtk_dpmaif_enable_irq(dcb);
	if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb))) {
		if (mtk_dpmaif_drv_start_queue(dcb->drv_info, DPMAIF_RX) < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to start dpmaif hw rx\n");
			mtk_dpmaif_common_err_handle(dcb, true);
			return;
		}

		if (mtk_dpmaif_drv_start_queue(dcb->drv_info, DPMAIF_TX) < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to start dpmaif hw tx\n");
			mtk_dpmaif_common_err_handle(dcb, true);
			return;
		}
	}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	mod_timer(&dcb->ring_rel_ctrl_timer,
		  jiffies + msecs_to_jiffies(BIT(traffic_stats_shift)));
#endif

#ifdef CONFIG_DATA_TEST_MODE
	mod_timer(&dcb->traffic_monitor,
		  jiffies + DPMAIF_TRAFFIC_MONITOR_INTERVAL * HZ);
#endif
}

static void mtk_dpmaif_trans_disable(struct mtk_dpmaif_ctlb *dcb)
{
	bool io_err = false;

	/* Wait tx doorbell and tx done work complete */
	mtk_dpmaif_sw_wait_tx_stop(dcb);

	/* Stop dpmaif hw tx and rx. */
	if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb))) {
		if (mtk_dpmaif_drv_stop_queue(dcb->drv_info, DPMAIF_TX) < 0) {
			io_err = true;
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to stop dpmaif hw tx\n");
		}

		if (mtk_dpmaif_drv_stop_queue(dcb->drv_info, DPMAIF_RX) < 0) {
			io_err = true;
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to stop dpmaif hw rx\n");
		}

		if (io_err)
			mtk_dpmaif_common_err_handle(dcb, true);
	}

	/* Disable all dpmaif L1 interrupt. */
	mtk_dpmaif_disable_irq(dcb);

	/* Stop and wait rx handle done  */
	mtk_dpmaif_sw_stop_rx(dcb);

	/* Wait bat reload work done */
	mtk_dpmaif_flush_bat_reload(dcb);

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	del_timer_sync(&dcb->ring_rel_ctrl_timer);
#endif

#ifdef CONFIG_DATA_TEST_MODE
	del_timer(&dcb->traffic_monitor);
#endif
}

static void mtk_dpmaif_trans_ctl(struct mtk_dpmaif_ctlb *dcb, bool enable)
{
	mutex_lock(&dcb->trans_ctl_lock);

	MTK_INFO(DCB_TO_MDEV(dcb),
		 "%ps: pm_ready=%d, dpmaif_state=%d, port_ready=%d, trans_enabled=%d, enable=%d\n",
		__builtin_return_address(0), dcb->dpmaif_pm_ready,
		dcb->dpmaif_state, dcb->dpmaif_user_ready, dcb->trans_enabled, enable);

	if (enable) {
		if (!dcb->trans_enabled) {
			if (dcb->dpmaif_pm_ready &&
			    dcb->dpmaif_state == DPMAIF_STATE_PWRON &&
			    dcb->dpmaif_user_ready) {
				mtk_dpmaif_trans_enable(dcb);
				dcb->trans_enabled = true;
			}
		}
	} else {
		if (dcb->trans_enabled) {
			if (!dcb->dpmaif_pm_ready ||
			    !(dcb->dpmaif_state == DPMAIF_STATE_PWRON) ||
			    !dcb->dpmaif_user_ready) {
				mtk_dpmaif_trans_disable(dcb);
				dcb->trans_enabled = false;
			}
		}
	}

	mutex_unlock(&dcb->trans_ctl_lock);
}

static void mtk_dpmaif_cmd_trans_ctl(struct mtk_dpmaif_ctlb *dcb, void *data)
{
	struct mtk_data_trans_ctl *trans_ctl = data;

	dcb->dpmaif_user_ready = trans_ctl->enable;

	/* Try best to drop all tx vq packets when disable trans */
	if (!trans_ctl->enable)
		mtk_dpmaif_tx_vqs_reset(dcb);

	mtk_dpmaif_trans_ctl(dcb, trans_ctl->enable);
}

static void mtk_dpmaif_cmd_intr_coalesce_write(struct mtk_dpmaif_ctlb *dcb,
					       unsigned int qid, enum dpmaif_drv_dir dir)
{
	struct dpmaif_drv_intr drv_intr;

	if (dir == DPMAIF_TX) {
		drv_intr.pkt_threshold = dcb->txqs[qid].intr_coalesce_frame;
		drv_intr.time_threshold = dcb->intr_coalesce.tx_coalesce_usecs;
	} else {
		drv_intr.pkt_threshold = dcb->rxqs[qid].intr_coalesce_frame;
		drv_intr.time_threshold = dcb->intr_coalesce.rx_coalesce_usecs;
	}

	drv_intr.dir = dir;
	drv_intr.q_mask = BIT(qid);

	drv_intr.mode = 0;
	if (drv_intr.pkt_threshold)
		drv_intr.mode |= DPMAIF_INTR_EN_PKT;
	if (drv_intr.time_threshold)
		drv_intr.mode |= DPMAIF_INTR_EN_TIME;
	MTK_DBG(DCB_TO_MDEV(dcb),
		MTK_DBG_DPMF, MTK_MEMLOG_RG_0, "mode=%u, ptk_th=%u, time_th=%u\n",
		drv_intr.mode, drv_intr.pkt_threshold, drv_intr.time_threshold);
	dcb->drv_info->drv_ops->feature_cmd(dcb->drv_info, DATA_HW_INTR_COALESCE_SET, &drv_intr);
}

static int mtk_dpmaif_cmd_intr_coalesce_set(struct mtk_dpmaif_ctlb *dcb, void *data)
{
	struct mtk_data_intr_coalesce *dpmaif_intr_cfg = &dcb->intr_coalesce;
	struct mtk_data_intr_coalesce *user_intr_cfg = data;
	int i;

	memcpy(dpmaif_intr_cfg, data, sizeof(*dpmaif_intr_cfg));

	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		dcb->rxqs[i].intr_coalesce_frame = user_intr_cfg->rx_coalesced_frames;
		mtk_dpmaif_cmd_intr_coalesce_write(dcb, i, DPMAIF_RX);
	}

	for (i = 0; i < dcb->res_cfg->txq_cnt; i++) {
		dcb->txqs[i].intr_coalesce_frame = user_intr_cfg->tx_coalesced_frames;
		mtk_dpmaif_cmd_intr_coalesce_write(dcb, i, DPMAIF_TX);
	}

	return 0;
}

static int mtk_dpmaif_cmd_intr_coalesce_get(struct mtk_dpmaif_ctlb *dcb, void *data)
{
	struct mtk_data_intr_coalesce *dpmaif_intr_cfg = &dcb->intr_coalesce;

	memcpy(data, dpmaif_intr_cfg, sizeof(*dpmaif_intr_cfg));

	return 0;
}

static int mtk_dpmaif_cmd_rxfh_set(struct mtk_dpmaif_ctlb *dcb, void *data)
{
	struct mtk_data_rxfh *indir_rxfh = data;
	int ret;

	if (indir_rxfh->key) {
		ret = mtk_dpmaif_drv_feature_cmd(dcb->drv_info, DATA_HW_HASH_SET, indir_rxfh->key);
		if (ret < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to set hash key\n");
			return ret;
		}
	}

	if (indir_rxfh->indir) {
		ret = mtk_dpmaif_drv_feature_cmd(dcb->drv_info, DATA_HW_INDIR_SET,
						 indir_rxfh->indir);
		if (ret < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to set indirection table\n");
			return ret;
		}
	}

	return 0;
}

static int mtk_dpmaif_cmd_rxfh_get(struct mtk_dpmaif_ctlb *dcb, void *data)
{
	struct mtk_data_rxfh *indir_rxfh = data;
	int ret;

	if (indir_rxfh->key) {
		ret = mtk_dpmaif_drv_feature_cmd(dcb->drv_info, DATA_HW_HASH_GET, indir_rxfh->key);
		if (ret < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to get hash key\n");
			return ret;
		}
	}

	if (indir_rxfh->indir) {
		ret = mtk_dpmaif_drv_feature_cmd(dcb->drv_info, DATA_HW_INDIR_GET,
						 indir_rxfh->indir);
		if (ret < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to get indirection table\n");
			return ret;
		}
	}

	return 0;
}

static inline void mtk_dpmaif_cmd_rxq_num_get(struct mtk_dpmaif_ctlb *dcb, void *data)
{
	*(unsigned int *)data = dcb->res_cfg->rxq_cnt;
}

#define DATA_TRANS_STRING_LEN 32

static const char dpmaif_tx_stats[][DATA_TRANS_STRING_LEN] = {
	"txq%u.tx_sw_packets", "txq%u.tx_hw_packets", "txq%u.tx_done_last_time",
	"txq%u.tx_done_last_cnt", "txq%u.irq_evt.ul_done", "txq%u.irq_evt.ul_drb_empty",
};

static const char dpmaif_rx_stats[][DATA_TRANS_STRING_LEN] = {
	"rxq%u.rx_packets", "rxq%u.rx_errors", "rxq%u.rx_dropped",
	"rxq%u.rx_hw_ind_dropped", "rxq%u.rx_done_last_time",
	"rxq%u.rx_done_last_cnt", "rxq%u.irq_evt.dl_done",
	"rxq%u.irq_evt.pit_len_err",
};

static const char dpmaif_irq_stats[][DATA_TRANS_STRING_LEN] = {
	"irq%u.irq_total_cnt", "irq%u.irq_last_time",
};

static const char dpmaif_misc_stats[][DATA_TRANS_STRING_LEN] = {
	"ul_md_notready", "ul_md_pwr_notready", "ul_len_err", "dl_skb_len_err",
	"dl_bat_cnt_len_err", "dl_pkt_empty", "dl_frag_empty",
	"dl_mtu_err", "dl_frag_cnt_len_err", "hpc_ent_type_err",
};

#define DATA_TX_STATS_LEN	ARRAY_SIZE(dpmaif_tx_stats)
#define DATA_RX_STATS_LEN	ARRAY_SIZE(dpmaif_rx_stats)
#define DATA_IRQ_STATS_LEN	ARRAY_SIZE(dpmaif_irq_stats)
#define DATA_MISC_STATS_LEN	ARRAY_SIZE(dpmaif_misc_stats)

static unsigned int mtk_dpmaif_describe_stats(struct mtk_dpmaif_ctlb *dcb, u8 *strings)
{
	unsigned int i, j, n_stats = 0;

	for (i = 0; i < dcb->res_cfg->txq_cnt; i++) {
		n_stats += DATA_TX_STATS_LEN;
		if (strings) {
			for (j = 0; j < DATA_TX_STATS_LEN; j++) {
				snprintf(strings, DATA_TRANS_STRING_LEN, dpmaif_tx_stats[j], i);
				strings += DATA_TRANS_STRING_LEN;
			}
		}
	}

	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		n_stats += DATA_RX_STATS_LEN;
		if (strings) {
			for (j = 0; j < DATA_RX_STATS_LEN; j++) {
				snprintf(strings, DATA_TRANS_STRING_LEN, dpmaif_rx_stats[j], i);
				strings += DATA_TRANS_STRING_LEN;
			}
		}
	}

	for (i = 0; i < dcb->res_cfg->irq_cnt; i++) {
		n_stats += DATA_IRQ_STATS_LEN;
		if (strings) {
			for (j = 0; j < DATA_IRQ_STATS_LEN; j++) {
				snprintf(strings, DATA_TRANS_STRING_LEN, dpmaif_irq_stats[j], i);
				strings += DATA_TRANS_STRING_LEN;
			}
		}
	}

	n_stats += DATA_MISC_STATS_LEN;
	if (strings)
		memcpy(strings, *dpmaif_misc_stats, sizeof(dpmaif_misc_stats));

	return n_stats;
}

static void mtk_dpmaif_read_stats(struct mtk_dpmaif_ctlb *dcb, u64 *data)
{
	unsigned int i, j = 0;

	for (i = 0; i < dcb->res_cfg->txq_cnt; i++) {
		data[j++] = dcb->traffic_stats.tx_sw_packets[i];
		data[j++] = dcb->traffic_stats.tx_hw_packets[i];
		data[j++] = dcb->traffic_stats.tx_done_last_time[i];
		data[j++] = dcb->traffic_stats.tx_done_last_cnt[i];
		data[j++] = dcb->traffic_stats.irq_tx_evt[i].ul_done;
		data[j++] = dcb->traffic_stats.irq_tx_evt[i].ul_drb_empty;
	}

	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		data[j++] = dcb->traffic_stats.rx_packets[i];
		data[j++] = dcb->traffic_stats.rx_errors[i];
		data[j++] = dcb->traffic_stats.rx_dropped[i];
		data[j++] = dcb->traffic_stats.rx_hw_ind_dropped[i];
		data[j++] = dcb->traffic_stats.rx_done_last_time[i];
		data[j++] = dcb->traffic_stats.rx_done_last_cnt[i];
		data[j++] = dcb->traffic_stats.irq_rx_evt[i].dl_done;
		data[j++] = dcb->traffic_stats.irq_rx_evt[i].pit_len_err;
	}

	for (i = 0; i < dcb->res_cfg->irq_cnt; i++) {
		data[j++] = dcb->traffic_stats.irq_total_cnt[i];
		data[j++] = dcb->traffic_stats.irq_last_time[i];
	}

	data[j++] = dcb->traffic_stats.irq_other_evt.ul_md_notready;
	data[j++] = dcb->traffic_stats.irq_other_evt.ul_md_pwr_notready;
	data[j++] = dcb->traffic_stats.irq_other_evt.ul_len_err;
	data[j++] = dcb->traffic_stats.irq_other_evt.dl_skb_len_err;
	data[j++] = dcb->traffic_stats.irq_other_evt.dl_bat_cnt_len_err;
	data[j++] = dcb->traffic_stats.irq_other_evt.dl_pkt_empty;
	data[j++] = dcb->traffic_stats.irq_other_evt.dl_frag_empty;
	data[j++] = dcb->traffic_stats.irq_other_evt.dl_mtu_err;
	data[j++] = dcb->traffic_stats.irq_other_evt.dl_frag_cnt_len_err;
	data[j++] = dcb->traffic_stats.irq_other_evt.hpc_ent_type_err;
}

static inline void mtk_dpmaif_cmd_string_cnt_get(struct mtk_dpmaif_ctlb *dcb, void *data)
{
	*(unsigned int *)data = mtk_dpmaif_describe_stats(dcb, NULL);
}

static void mtk_dpmaif_cmd_handle(struct dpmaif_cmd_srv *srv)
{
	struct mtk_dpmaif_ctlb *dcb = srv->dcb;
	struct dpmaif_vq *cmd_vq = srv->vq;
	struct mtk_data_cmd *cmd_info;
	struct sk_buff *skb;
	int ret;

	while ((skb = skb_dequeue(&cmd_vq->list))) {
		ret = 0;
		cmd_info = SKB_TO_CMD(skb);
		if (dcb->dpmaif_state == DPMAIF_STATE_PWRON) {
			switch (cmd_info->cmd) {
			case DATA_CMD_TRANS_CTL:
				mtk_dpmaif_cmd_trans_ctl(dcb, CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_INTR_COALESCE_GET:
				ret = mtk_dpmaif_cmd_intr_coalesce_get(dcb,
								       CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_INTR_COALESCE_SET:
				ret = mtk_dpmaif_cmd_intr_coalesce_set(dcb,
								       CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_RXFH_GET:
				ret = mtk_dpmaif_cmd_rxfh_get(dcb,
							      CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_RXFH_SET:
				ret = mtk_dpmaif_cmd_rxfh_set(dcb,
							      CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_INDIR_SIZE_GET:
				ret = mtk_dpmaif_drv_feature_cmd(dcb->drv_info,
								 DATA_HW_INDIR_SIZE_GET,
								 CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_HKEY_SIZE_GET:
				ret = mtk_dpmaif_drv_feature_cmd(dcb->drv_info,
								 DATA_HW_HASH_KEY_SIZE_GET,
								 CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_RXQ_NUM_GET:
				mtk_dpmaif_cmd_rxq_num_get(dcb, CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_STRING_CNT_GET:
				mtk_dpmaif_cmd_string_cnt_get(dcb, CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_STRING_GET:
				ret = mtk_dpmaif_describe_stats(dcb, (u8 *)CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_TRANS_DUMP:
				mtk_dpmaif_read_stats(dcb, (u64 *)CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_LRO_SET:
				ret = mtk_dpmaif_drv_feature_cmd(dcb->drv_info, DATA_HW_LRO_SET,
								 CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_RXCSUM_SET:
				ret = mtk_dpmaif_drv_feature_cmd(dcb->drv_info, DATA_HW_RXCSUM_SET,
								 CMD_TO_DATA(cmd_info));
				break;
			case DATA_CMD_TXCSUM_SET:
				ret = mtk_dpmaif_drv_feature_cmd(dcb->drv_info, DATA_HW_TXCSUM_SET,
								 CMD_TO_DATA(cmd_info));
				break;
			default:
				MTK_WARN(DCB_TO_MDEV(dcb),
					 "Unknown cmd type=%d\n", cmd_info->cmd);
				ret = -EOPNOTSUPP;
				break;
			}
		}
		cmd_info->ret = ret;
		if (cmd_info->data_complete)
			cmd_info->data_complete(skb);
	}
}

static void mtk_dpmaif_cmd_srv(struct work_struct *work)
{
	struct dpmaif_cmd_srv *srv = container_of(work, struct dpmaif_cmd_srv, work);
	struct mtk_dpmaif_ctlb *dcb = srv->dcb;
	int ret;

	pm_runtime_get_sync(DCB_TO_DEV(dcb));
	mtk_pm_ds_lock(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF);
	ret = mtk_pm_ds_wait_complete(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF);
	if (unlikely(ret < 0)) {
		/* Exception scenario, but should always do command handler. */
		MTK_WARN(DCB_TO_MDEV(dcb), "Failed to wait ds_lock\n");
		mtk_dpmaif_common_err_handle(dcb, true);
	}

	mtk_dpmaif_cmd_handle(srv);

	mtk_pm_ds_unlock(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF, false);
	pm_runtime_put_sync(DCB_TO_DEV(dcb));
}

static int mtk_dpmaif_cmd_srvs_init(struct mtk_dpmaif_ctlb *dcb)
{
	struct dpmaif_cmd_srv *cmd_srv = &dcb->cmd_srv;
	struct dpmaif_vq *cmd_vq = &dcb->cmd_vq;

	cmd_vq->max_len = DEFAULT_TX_QUEUE_LEN;
	skb_queue_head_init(&cmd_vq->list);

	cmd_srv->dcb = dcb;
	cmd_srv->vq = cmd_vq;

	/* The cmd handle work will be scheduled by schedule_work(), use system workqueue. */
	INIT_WORK(&cmd_srv->work, mtk_dpmaif_cmd_srv);

	return 0;
}

static void mtk_dpmaif_cmd_srvs_exit(struct mtk_dpmaif_ctlb *dcb)
{
	flush_work(&dcb->cmd_srv.work);
	skb_queue_purge(&dcb->cmd_vq.list);
}

static struct dpmaif_drv_ops *mtk_dpmaif_get_drv_ops(u32 hw_ver)
{
	struct dpmaif_drv_ops_desc *p_drv_ops;
	unsigned char i;

	for (i = 0; (p_drv_ops = &dpmaif_drv_ops_tbl[i]) && p_drv_ops && p_drv_ops->drv_ops; i++)
		if (p_drv_ops->hw_ver == hw_ver)
			return p_drv_ops->drv_ops;

	return NULL;
}

static int mtk_dpmaif_drv_res_init(struct mtk_dpmaif_ctlb *dcb)
{
	dcb->drv_info = devm_kzalloc(DCB_TO_DEV(dcb), sizeof(*dcb->drv_info), GFP_KERNEL);
	if (!dcb->drv_info) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate dpmaif_drv info\n");
		return -ENOMEM;
	}

	dcb->drv_info->mdev = DCB_TO_MDEV(dcb);

	dcb->drv_info->drv_ops = mtk_dpmaif_get_drv_ops(DPMAIF_GET_HW_VER(dcb));
	if (!dcb->drv_info->drv_ops) {
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Unsupported mdev, hw_ver=0x%x\n", DPMAIF_GET_HW_VER(dcb));
		return -EFAULT;
	}

	mtk_dpmaif_drv_feature_cmd(dcb->drv_info, DATA_HW_CFG_GET, (void *)&dcb->res_cfg);

	return 0;
}

static void mtk_dpmaif_drv_res_exit(struct mtk_dpmaif_ctlb *dcb)
{
	devm_kfree(DCB_TO_DEV(dcb), dcb->drv_info);
	dcb->drv_info = NULL;
}

static void mtk_dpmaif_irq_tx_done(struct mtk_dpmaif_ctlb *dcb, unsigned int q_mask)
{
	unsigned int ulq_done;
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	int drb_rd_idx;
#endif
	int i;

	/* All txq done share one interrupt, and then,
	 * one interrupt will check all ulq done status and schedule corresponding bottom half.
	 */
	for (i = 0; i < dcb->res_cfg->txq_cnt; i++) {
		ulq_done = q_mask & (1 << i);
		if (ulq_done) {
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
			drb_rd_idx = mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_DRB_RIDX, i);
			if (unlikely(drb_rd_idx < 0)) {
				MTK_ERR(DCB_TO_MDEV(dcb),
					"Failed to read txq%u drb_rd_idx, ret=%d\n", i, drb_rd_idx);
				mtk_dpmaif_common_err_handle(dcb, true);
				break;
			}

			dcb->txqs[i].drb_rd_idx = drb_rd_idx;
#endif
			queue_delayed_work(dcb->tx_done_wq,
					   &dcb->txqs[i].tx_done_work,
					msecs_to_jiffies(0));

			dcb->traffic_stats.irq_tx_evt[i].ul_done++;
		}
	}
}

static void mtk_dpmaif_irq_rx_done(struct mtk_dpmaif_ctlb *dcb, unsigned int q_mask)
{
	struct dpmaif_rxq *rxq;
	unsigned int dlq_done;
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	int pit_widx;
#endif
	int i;

	/* RSS: one dlq done belongs to one interrupt, and then,
	 * one interrupt will only check one dlq done status and schedule bottom half.
	 */
	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		dlq_done = q_mask & (1 << i);
		if (dlq_done) {
			dcb->traffic_stats.rx_done_last_time[i] = local_clock();
			rxq = &dcb->rxqs[i];
			__pm_stay_awake(rxq->ws);
			trace_mtk_tput_data_rx(rxq->id, "rx p2");

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
			pit_widx = mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
							       DPMAIF_PIT_WIDX, rxq->id);
			if (unlikely(pit_widx < 0)) {
				MTK_ERR(DCB_TO_MDEV(dcb),
					"Failed to read rxq%u hw pit_wr_idx, ret=%d\n",
					rxq->id, pit_widx);
				mtk_dpmaif_common_err_handle(dcb, true);
				break;
			}

			rxq->pit_wr_idx = pit_widx;
#endif

			dcb->traffic_stats.rx_done_last_cnt[i] = 0;
			napi_schedule(&rxq->napi);
			dcb->traffic_stats.irq_rx_evt[i].dl_done++;
			break;
		}
	}
}

static void mtk_dpmaif_irq_pit_len_err(struct mtk_dpmaif_ctlb *dcb, unsigned int q_mask)
{
	unsigned int pit_len_err;
	int i;

	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		pit_len_err = q_mask & (1 << i);
		if (pit_len_err)
			break;
	}

	dcb->traffic_stats.irq_rx_evt[i].pit_len_err++;
	dcb->rxqs[i].pit_cnt_err_intr_set = true;
}

static void mtk_dpmaif_tras_ulq_cfg(struct mtk_dpmaif_ctlb *dcb, unsigned int cfg_data,
				    ktime_t now)
{
	unsigned char txq_cnt = dcb->res_cfg->txq_cnt;
	unsigned char q_id, rst;
	struct dpmaif_txq *txq;

	rst = FIELD_GET(TRAS_MSG0_RST, cfg_data);
	if (rst)
		dcb->doorbell_base_ts = now;

	q_id = FIELD_GET(TRAS_MSG0_QID, cfg_data);
	if (q_id >= txq_cnt) {
		MTK_WARN(DCB_TO_MDEV(dcb), "Invalid tras cfg, q_id=%d.\n", q_id);
		return;
	}

	txq = &dcb->txqs[q_id];
	txq->doorbell_delay = FIELD_GET(TRAS_MSG0_TIME_TH, cfg_data);
}

static void mtk_dpmaif_tras_ulq_rst(struct mtk_dpmaif_ctlb *dcb, unsigned int cfg_data)
{
	unsigned char txq_cnt = dcb->res_cfg->txq_cnt;
	unsigned char q_mask;
	int i;

	q_mask = FIELD_GET(TRAS_MSG1_QMASK, cfg_data);
	for (i = 0; i < txq_cnt; i++) {
		if (q_mask & BIT(i))
			dcb->txqs[i].doorbell_delay = dcb->res_cfg->txq_doorbell_delay[i];
	}
}

static void mtk_dpmaif_irq_tras_sync(struct mtk_dpmaif_ctlb *dcb, ktime_t now)
{
	unsigned char cfg_type;
	unsigned int cfg_data;

	cfg_data = mtk_hw_get_ext_traffic_cfg(DCB_TO_MDEV(dcb));

	cfg_type = FIELD_GET(TRAS_MSG_TYPE, cfg_data);
	switch (cfg_type) {
	case DPMAIF_TRAS_ULQ_CFG:
		mtk_dpmaif_tras_ulq_cfg(dcb, cfg_data, now);
		break;

	case DPMAIF_TRAS_ULQ_RST:
		mtk_dpmaif_tras_ulq_rst(dcb, cfg_data);
		break;

	default:
		MTK_WARN(DCB_TO_MDEV(dcb), "Invalid parameter, unknown tras config type\n");
		break;
	}
}

static int mtk_dpmaif_irq_handle(int irq_id, void *data)
{
	struct dpmaif_drv_intr_info intr_info;
	struct dpmaif_irq_param *irq_param;
	struct dpmaif_bat_ring *bat_ring;
	struct mtk_dpmaif_ctlb *dcb;
	int ret;
	int i;

	irq_param = data;
	dcb = irq_param->dcb;

	dcb->traffic_stats.irq_last_time[irq_param->idx] = local_clock();
	dcb->traffic_stats.irq_total_cnt[irq_param->idx]++;

	if (unlikely(dcb->dpmaif_state != DPMAIF_STATE_PWRON)) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Invalid parameter, unexpected dpmaif irq\n");
		goto out;
	}

	memset(&intr_info, 0x00, sizeof(struct dpmaif_drv_intr_info));
	ret = mtk_dpmaif_drv_intr_handle(dcb->drv_info, &intr_info, irq_param->dpmaif_irq_src);
	if (ret < 0) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to get dpmaif drv irq info\n");
		goto clean_drv_irq_info;
	}

	for (i = 0; i < intr_info.intr_cnt; i++) {
		switch (intr_info.intr_types[i]) {
		case DPMAIF_INTR_UL_DONE:
			MTK_DBG(DCB_TO_MDEV(dcb),
				MTK_DBG_DATA_IRQ, MTK_DATA_IRQ_MEMLOG_RG(irq_param->idx),
				"ul_done, q_mask=0x%x\n", intr_info.intr_queues[i]);
			mtk_dpmaif_irq_tx_done(dcb, intr_info.intr_queues[i]);
			break;
		case DPMAIF_INTR_DL_BATCNT_LEN_ERR:
			MTK_DBG(DCB_TO_MDEV(dcb),
				MTK_DBG_DATA_IRQ, MTK_DATA_IRQ_MEMLOG_RG(irq_param->idx),
				"batcnt len err\n");
			dcb->traffic_stats.irq_other_evt.dl_bat_cnt_len_err++;
			bat_ring = &dcb->bat_info.normal_bat_ring;
			bat_ring->bat_cnt_err_intr_set = true;
			queue_work(dcb->bat_info.reload_wq, &bat_ring->reload_work);
			dpmaif_dump_once(dcb);
			break;
		case DPMAIF_INTR_DL_FRGCNT_LEN_ERR:
			MTK_DBG(DCB_TO_MDEV(dcb),
				MTK_DBG_DATA_IRQ, MTK_DATA_IRQ_MEMLOG_RG(irq_param->idx),
				"frgcnt len err\n");
			dcb->traffic_stats.irq_other_evt.dl_frag_cnt_len_err++;
			bat_ring = &dcb->bat_info.frag_bat_ring;
			bat_ring->bat_cnt_err_intr_set = true;
			if (dcb->bat_info.frag_bat_enabled)
				queue_work(dcb->bat_info.reload_wq, &bat_ring->reload_work);
			dpmaif_dump_once(dcb);
			break;
		case DPMAIF_INTR_DL_PITCNT_LEN_ERR:
			MTK_DBG(DCB_TO_MDEV(dcb),
				MTK_DBG_DATA_IRQ, MTK_DATA_IRQ_MEMLOG_RG(irq_param->idx),
				"pitcnt len err, q_mask=0x%x\n",
				intr_info.intr_queues[i]);
			mtk_dpmaif_irq_pit_len_err(dcb, intr_info.intr_queues[i]);
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
			queue_work(dcb->doorbell_wq, &dcb->doorbell_work);
#endif
			dpmaif_dump_once(dcb);
			break;
		case DPMAIF_INTR_DL_DONE:
			MTK_DBG(DCB_TO_MDEV(dcb),
				MTK_DBG_DATA_IRQ, MTK_DATA_IRQ_MEMLOG_RG(irq_param->idx),
				"dl done, q_mask=0x%x\n", intr_info.intr_queues[i]);
			mtk_dpmaif_irq_rx_done(dcb, intr_info.intr_queues[i]);
			break;
		case DPMAIF_INTR_TRAS_SYNC:
			mtk_dpmaif_irq_tras_sync(dcb,
						 dcb->traffic_stats.irq_last_time[irq_param->idx]);
			break;
		default:
			break;
		}
	}

clean_drv_irq_info:
	mtk_hw_clear_irq(DCB_TO_MDEV(dcb), irq_param->dev_irq_id);
	mtk_hw_unmask_irq(DCB_TO_MDEV(dcb), irq_param->dev_irq_id);
out:
	return IRQ_HANDLED;
}

static int mtk_dpmaif_irq_init(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char irq_cnt = dcb->res_cfg->irq_cnt;
	struct dpmaif_irq_param *irq_param;
	enum mtk_irq_src irq_src;
	int ret = 0;
	int i, j;

	dcb->irq_params = devm_kcalloc(DCB_TO_DEV(dcb), irq_cnt, sizeof(*irq_param), GFP_KERNEL);
	if (!dcb->irq_params) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to allocate dpmaif interrupt parameters\n");
		return -ENOMEM;
	}

	for (i = 0; i < irq_cnt; i++) {
		irq_src = dcb->res_cfg->irq_src[i];
		irq_param = &dcb->irq_params[i];
		irq_param->idx = i;
		irq_param->dcb = dcb;
		irq_param->dpmaif_irq_src = irq_src;
		irq_param->dev_irq_id = mtk_hw_get_irq_id(DCB_TO_MDEV(dcb), irq_src);
		if (irq_param->dev_irq_id < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb),
				"Failed to allocate irq id, irq_src=%d\n", irq_src);
			ret = -EINVAL;
			goto clean_reg_irq;
		}

		ret = mtk_hw_register_irq(DCB_TO_MDEV(dcb), irq_param->dev_irq_id,
					  mtk_dpmaif_irq_handle, irq_param);
		if (ret < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb),
				"Failed to register irq, irq_src=%d\n", irq_src);
			goto clean_reg_irq;
		}
	}

	/* HW layer default mask dpmaif interrupt. */
	dcb->irq_enabled = false;

	return 0;
clean_reg_irq:
	for (j = i - 1; j >= 0; j--) {
		irq_param = &dcb->irq_params[j];
		mtk_hw_unregister_irq(DCB_TO_MDEV(dcb), irq_param->dev_irq_id);
	}

	devm_kfree(DCB_TO_DEV(dcb), dcb->irq_params);
	dcb->irq_params = NULL;

	return ret;
}

static int mtk_dpmaif_irq_exit(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char irq_cnt = dcb->res_cfg->irq_cnt;
	struct dpmaif_irq_param *irq_param;
	int i;

	for (i = 0; i < irq_cnt; i++) {
		irq_param = &dcb->irq_params[i];
		mtk_hw_unregister_irq(DCB_TO_MDEV(dcb), irq_param->dev_irq_id);
	}

	devm_kfree(DCB_TO_DEV(dcb), dcb->irq_params);
	dcb->irq_params = NULL;

	return 0;
}

static int mtk_dpmaif_port_init(struct mtk_dpmaif_ctlb *dcb)
{
	struct mtk_data_trans_info trans_info;
	struct dpmaif_rxq *rxq;
	int i;

	memset(&trans_info, 0x00, sizeof(struct mtk_data_trans_info));
	if (dcb->res_cfg->cap & DATA_HW_F_LRO)
		trans_info.cap |= DATA_F_LRO;
	if (dcb->res_cfg->cap & DATA_HW_F_INDR_TBL)
		trans_info.cap |= DATA_F_RXFH;
	if (dcb->res_cfg->cap & DATA_HW_F_INTR_COALESCE)
		trans_info.cap |= DATA_F_INTR_COALESCE;
	if (dcb->res_cfg->cap & DATA_HW_F_RXCSUM)
		trans_info.cap |= DATA_F_RXCSUM;
	if (dcb->res_cfg->cap & DATA_HW_F_TXCSUM)
		trans_info.cap |= DATA_F_TXCSUM;

	trans_info.txq_cnt = dcb->res_cfg->txq_cnt;
	trans_info.rxq_cnt = dcb->res_cfg->rxq_cnt;
	trans_info.max_mtu = dcb->bat_info.max_mtu;

	for (i = 0; i < trans_info.rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		dcb->napi[i] = &rxq->napi;
	}
	trans_info.napis = dcb->napi;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	trans_info.aff_cfg = dcb->aff_cfg;
#endif
	/* Initialize data port layer. */
	dcb->port_ops = &data_port_ops;

	return dcb->port_ops->init(dcb->data_blk, &trans_info);
}

static void mtk_dpmaif_port_exit(struct mtk_dpmaif_ctlb *dcb)
{
	dcb->port_ops->exit(dcb->data_blk);
}

static int mtk_dpmaif_hw_init(struct mtk_dpmaif_ctlb *dcb)
{
	struct dpmaif_bat_ring *bat_ring;
	struct dpmaif_drv_cfg drv_cfg;
	struct dpmaif_rxq *rxq;
	struct dpmaif_txq *txq;
	int ret;
	int i;

	memset(&drv_cfg, 0x00, sizeof(struct dpmaif_drv_cfg));

	bat_ring = &dcb->bat_info.normal_bat_ring;
	drv_cfg.normal_bat_base = bat_ring->bat_dma_addr;
	drv_cfg.normal_bat_cnt = bat_ring->bat_cnt;
	drv_cfg.normal_bat_buf_size = bat_ring->buf_size;

	if (dcb->bat_info.frag_bat_enabled) {
		drv_cfg.features |= DATA_HW_F_FRAG;
		bat_ring = &dcb->bat_info.frag_bat_ring;
		drv_cfg.frag_bat_base = bat_ring->bat_dma_addr;
		drv_cfg.frag_bat_cnt = bat_ring->bat_cnt;
		drv_cfg.frag_bat_buf_size = bat_ring->buf_size;
	}

	if (dcb->res_cfg->cap & DATA_HW_F_LRO)
		drv_cfg.features |= DATA_HW_F_LRO;
	if (dcb->res_cfg->cap & DATA_HW_F_RXCSUM)
		drv_cfg.features |= DATA_HW_F_RXCSUM;
	if (dcb->res_cfg->cap & DATA_HW_F_TXCSUM)
		drv_cfg.features |= DATA_HW_F_TXCSUM;

	drv_cfg.max_mtu = dcb->bat_info.max_mtu;

	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		drv_cfg.pit_base[i] = rxq->pit_dma_addr;
		drv_cfg.pit_cnt[i] = rxq->pit_cnt;
	}

	for (i = 0; i < dcb->res_cfg->txq_cnt; i++) {
		txq = &dcb->txqs[i];
		drv_cfg.drb_base[i] = txq->drb_dma_addr;
		drv_cfg.drb_cnt[i] = txq->drb_cnt;
	}

	ret = mtk_dpmaif_drv_init(dcb->drv_info, &drv_cfg);
	if (ret < 0)
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to initialize dpmaif hw\n");

	return ret;
}

static int mtk_dpmaif_start(struct mtk_dpmaif_ctlb *dcb)
{
	struct dpmaif_bat_ring *bat_ring;
	unsigned int normal_buf_cnt;
	unsigned int frag_buf_cnt;
	int ret;

	if (dcb->dpmaif_state == DPMAIF_STATE_PWRON) {
		MTK_WARN(DCB_TO_MDEV(dcb), "Invalid parameters, dpmaif_state in PWRON\n");
		ret = -EINVAL;
		goto out;
	}

	/* Reload all buffer around normal bat. */
	bat_ring = &dcb->bat_info.normal_bat_ring;
	normal_buf_cnt = bat_ring->bat_cnt - 1;
	ret = mtk_dpmaif_reload_rx_buf(dcb, bat_ring, normal_buf_cnt, false);
	if (ret < 0) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to reload normal bat buffer\n");
		goto out;
	}

	/* Reload all buffer around normal bat. */
	if (dcb->bat_info.frag_bat_enabled) {
		bat_ring = &dcb->bat_info.frag_bat_ring;
		frag_buf_cnt = bat_ring->bat_cnt - 1;
		ret = mtk_dpmaif_reload_rx_buf(dcb, bat_ring, frag_buf_cnt, false);
		if (ret < 0) {
			MTK_ERR(DCB_TO_MDEV(dcb), "Failed to reload frag bat buffer\n");
			goto out;
		}
	}

	/* Initialize dpmaif hw. */
	ret = mtk_dpmaif_hw_init(dcb);
	if (ret < 0) {
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to initialize dpmaif hw\n");
		goto out;
	}

	/* Send doorbell to dpmaif HW about normal bat buffer count. */
	ret = mtk_dpmaif_drv_send_doorbell(dcb->drv_info, DPMAIF_BAT, 0, normal_buf_cnt);
	if (unlikely(ret < 0)) {
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Failed to send normal bat buffer count doorbell\n");
		mtk_dpmaif_common_err_handle(dcb, true);
		goto out;
	}

	/* Send doorbell to dpmaif HW about frag bat buffer count. */
	if (dcb->bat_info.frag_bat_enabled) {
		ret = mtk_dpmaif_drv_send_doorbell(dcb->drv_info, DPMAIF_FRAG, 0, frag_buf_cnt);
		if (unlikely(ret < 0)) {
			MTK_ERR(DCB_TO_MDEV(dcb),
				"Failed to send frag bat buffer count doorbell\n");
			mtk_dpmaif_common_err_handle(dcb, true);
			goto out;
		}
	}

	/* Initialize and run all tx services. */
	ret = mtk_dpmaif_tx_srvs_start(dcb);
	if (ret < 0) {
		MTK_WARN(DCB_TO_MDEV(dcb), "Failed to start all tx srvs\n");
		goto out;
	}

	dcb->dpmaif_state = DPMAIF_STATE_PWRON;
	mtk_dpmaif_disable_irq(dcb);

	MTK_INFO(DCB_TO_MDEV(dcb), "dpmaif start done\n");

	return 0;
out:
	{
		extern void t800_recovery_start(int value1, int value2);
		t800_recovery_start(1, ret);
	}
	return ret;
}

static void mtk_dpmaif_sw_reset(struct mtk_dpmaif_ctlb *dcb)
{
	mtk_dpmaif_tx_res_reset(dcb);
	mtk_dpmaif_rx_res_reset(dcb);
	mtk_dpmaif_bat_res_reset(&dcb->bat_info);
	mtk_dpmaif_tx_vqs_reset(dcb);
	memset(&dcb->traffic_stats, 0x00, sizeof(struct dpmaif_traffic_stats));
	dcb->dpmaif_pm_ready = true;
	dcb->dpmaif_user_ready = false;
	dcb->trans_enabled = false;
	dcb->dump_flag = 0;
	atomic_set(&dcb->dump_once, 0);
	dcb->err_event = 0;
}

static int mtk_dpmaif_stop(struct mtk_dpmaif_ctlb *dcb)
{
	if (dcb->dpmaif_state == DPMAIF_STATE_PWROFF)
		goto out;

	/* The flow of trans control as follow depends on dpmaif state,
	 * so change state firstly.
	 */
	dcb->dpmaif_state = DPMAIF_STATE_PWROFF;

	/* Stop data port layer tx. */
	dcb->port_ops->notify(dcb->data_blk, DATA_EVT_TX_STOP, 0xff);

	/* Stop all tx service. */
	mtk_dpmaif_tx_srvs_stop(dcb);

	/* Stop dpmaif tx/rx handle. */
	mtk_dpmaif_trans_ctl(dcb, false);

	/* Stop data port layer rx. */
	dcb->port_ops->notify(dcb->data_blk, DATA_EVT_RX_STOP, 0xff);

out:
	return 0;
}

static void mtk_dpmaif_fsm_callback(struct mtk_fsm_param *fsm_param, void *data)
{
	struct mtk_dpmaif_ctlb *dcb = data;

	if (!dcb || !fsm_param) {
		pr_warn("Invalid fsm parameter\n");
		return;
	}

	switch (fsm_param->to) {
	case FSM_STATE_OFF:
		mtk_dpmaif_stop(dcb);

		/* Unregister data port, because data port will be
		 * registered again in FSM_STATE_READY stage.
		 */
		dcb->port_ops->notify(dcb->data_blk, DATA_EVT_UNREG_DEV, 0);

		/* Flush all cmd process. */
		flush_work(&dcb->cmd_srv.work);

		/* clear data structure */
		mtk_dpmaif_sw_reset(dcb);
		break;
	case FSM_STATE_BOOTUP:
		if (fsm_param->fsm_flag == FSM_F_MD_HS_START)
			mtk_dpmaif_start(dcb);
		break;
	case FSM_STATE_READY:
		dcb->port_ops->notify(dcb->data_blk, DATA_EVT_REG_DEV, 0);
/***********************************xiaomi modem recovery start***/
		{
			extern void t800_recovery_complete(void);
			t800_recovery_complete();
		}
/***********************************xiaomi modem recovery end***/
		break;
	case FSM_STATE_MDEE:
		if (fsm_param->fsm_flag == FSM_F_MDEE_INIT) {
			mtk_dpmaif_stop(dcb);

			/* Dump dpmaif and drv information. */
			dpmaif_dump(dcb);
			dcb->port_ops->notify(dcb->data_blk, DATA_EVT_DUMP, 0);
		}
		break;
	default:
		break;
	}
}

static int mtk_dpmaif_fsm_init(struct mtk_dpmaif_ctlb *dcb)
{
	int ret;

	ret = mtk_fsm_notifier_register(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF,
					mtk_dpmaif_fsm_callback, dcb, FSM_PRIO_1, false);
	if (ret < 0)
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Failed to register dpmaif fsm notifier\n");

	return ret;
}

static int mtk_dpmaif_fsm_exit(struct mtk_dpmaif_ctlb *dcb)
{
	int ret;

	ret = mtk_fsm_notifier_unregister(DCB_TO_MDEV(dcb), MTK_USER_DPMAIF);
	if (ret < 0)
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Failed to unregister dpmaif fsm notifier\n");

	return ret;
}

static int mtk_dpmaif_suspend(struct mtk_md_dev *mdev, void *param, bool is_runtime)
{
	struct mtk_dpmaif_ctlb *dcb = param;
	struct dpmaif_rxq *rxq;
	int i;

	dcb->dpmaif_pm_ready = false;
	dcb->dpmaif_suspending = true;
	mtk_dpmaif_trans_ctl(dcb, false);
	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		MTK_INFO(DCB_TO_MDEV(dcb), "== dump rxq%d=0x%llx ==\n", i, (u64)rxq);
 
		if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
			MTK_INFO(DCB_TO_MDEV(dcb), "hw_pit: w=%d,r=%d\n",
				 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
							     DPMAIF_PIT_WIDX, i),
				 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
							     DPMAIF_PIT_RIDX, i));
	}
	if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
		MTK_INFO(DCB_TO_MDEV(dcb), "hw_bat: w=%d,r=%d\n",
			 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_BAT_WIDX, 0),
			 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_BAT_RIDX, 0));

	dcb->dpmaif_suspending = false;
	MTK_INFO(DCB_TO_MDEV(dcb), "dpmaif suspend done");

	return 0;
}

static int mtk_dpmaif_resume(struct mtk_md_dev *mdev, void *param, bool is_runtime,
			     bool link_ready)
{
	bool dev_is_reset = mtk_pm_check_dev_reset(mdev);
	struct mtk_dpmaif_ctlb *dcb = param;
	struct dpmaif_rxq *rxq;
	int i;

	/* If device resume after device power off, we don't need to enable trans.
	 * Since host driver will run re-init flow, we will get back to normal.
	 */
	if (!dev_is_reset) {
		dcb->dpmaif_pm_ready = true;
		for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
			rxq = &dcb->rxqs[i];
			MTK_INFO(DCB_TO_MDEV(dcb), "== dump rxq%d=0x%llx ==\n", i, (u64)rxq);

			if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
				MTK_INFO(DCB_TO_MDEV(dcb), "hw_pit: w=%d,r=%d\n",
					 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
								     DPMAIF_PIT_WIDX, i),
					 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
								     DPMAIF_PIT_RIDX, i));
		}
		if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
			MTK_INFO(DCB_TO_MDEV(dcb), "hw_bat: w=%d,r=%d\n",
				 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_BAT_WIDX, 0),
				 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_BAT_RIDX, 0));
		mtk_dpmaif_trans_ctl(dcb, true);
	}

	MTK_INFO(DCB_TO_MDEV(dcb),
		 "dpmaif resume done, dev_is_reset=%d", dev_is_reset);

	return 0;
}

static int mtk_dpmaif_suspend_late(struct mtk_md_dev *mdev, void *param, bool is_runtime)
{
	struct mtk_dpmaif_ctlb *dcb = param;
	struct dpmaif_rxq *rxq;
	int i;

	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		MTK_INFO(DCB_TO_MDEV(dcb), "== dump rxq%d=0x%llx ==\n", i, (u64)rxq);

		if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
			MTK_INFO(DCB_TO_MDEV(dcb), "hw_pit: w=%d,r=%d\n",
				 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
							     DPMAIF_PIT_WIDX, i),
				 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
							     DPMAIF_PIT_RIDX, i));
	}
	if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
		MTK_INFO(DCB_TO_MDEV(dcb), "hw_bat: w=%d,r=%d\n",
			 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_BAT_WIDX, 0),
			 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_BAT_RIDX, 0));
	
	return 0;
}

static int mtk_dpmaif_resume_early(struct mtk_md_dev *mdev, void *param, bool is_runtime,
				   bool link_ready)
{
	struct mtk_dpmaif_ctlb *dcb = param;
	struct dpmaif_rxq *rxq;
	int i;

	for (i = 0; i < dcb->res_cfg->rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		MTK_INFO(DCB_TO_MDEV(dcb), "== dump rxq%d=0x%llx ==\n", i, (u64)rxq);

		if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
			MTK_INFO(DCB_TO_MDEV(dcb), "hw_pit: w=%d,r=%d\n",
				 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
							     DPMAIF_PIT_WIDX, i),
				 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info,
							     DPMAIF_PIT_RIDX, i));
	}
	if (!mtk_hw_mmio_check(DCB_TO_MDEV(dcb)))
		MTK_INFO(DCB_TO_MDEV(dcb), "hw_bat: w=%d,r=%d\n",
			 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_BAT_WIDX, 0),
			 mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_BAT_RIDX, 0));
	
	return 0;
}

static int mtk_dpmaif_pm_init(struct mtk_dpmaif_ctlb *dcb)
{
	struct mtk_pm_entity *pm_entity;
	int ret;

	pm_entity = &dcb->pm_entity;
	INIT_LIST_HEAD(&pm_entity->entry);
	pm_entity->user = MTK_USER_DPMAIF;
	pm_entity->param = dcb;
	pm_entity->suspend = &mtk_dpmaif_suspend;
	pm_entity->suspend_late = &mtk_dpmaif_suspend_late;
	pm_entity->resume_early = &mtk_dpmaif_resume_early;
	pm_entity->resume = &mtk_dpmaif_resume;

	ret = mtk_pm_entity_register(DCB_TO_MDEV(dcb), pm_entity);
	if (ret < 0)
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to register dpmaif pm_entity\n");

	return ret;
}

static int mtk_dpmaif_pm_exit(struct mtk_dpmaif_ctlb *dcb)
{
	int ret;

	ret = mtk_pm_entity_unregister(DCB_TO_MDEV(dcb), &dcb->pm_entity);
	if (ret < 0)
		MTK_ERR(DCB_TO_MDEV(dcb), "Failed to unregister dpmaif pm_entity\n");

	return ret;
}

#ifdef CONFIG_DEBUG_FS
#ifdef CONFIG_DATA_TEST_MODE
#define MTK_DEBUGFS_BUF_SIZE 256
static ssize_t dpmaif_test_mode_read(void *data, char *buf, ssize_t max_cnt)
{
	struct mtk_dpmaif_ctlb *dcb = data;
	struct dpmaif_test_mode_cfg *test_mode_cfg = &dcb->test_mode_cfg;

	return snprintf(buf, max_cnt,
			"test configuration: mode=%u, dl_start=%u, pkts/ms=%u, test_time=%u\n",
		test_mode_cfg->md_tput_mode, test_mode_cfg->md_start_dl_tput,
		test_mode_cfg->md_pkt_number_per_ms, test_mode_cfg->md_dl_tput_test_time);
}

static ssize_t dpmaif_test_mode_write(void *data, const char *buf, ssize_t cnt)
{
	struct mtk_dpmaif_ctlb *dcb = data;
	struct dpmaif_test_mode_cfg *test_mode_cfg = &dcb->test_mode_cfg;
	unsigned int mode_cfg;
	int ret;

	ret = kstrtou32(buf, 16, &mode_cfg);
	if (ret) {
		pr_notice("Invalid parameter, Please check input string format\n");
		return -EFAULT;
	}

	pr_notice("test mode: 0x%08x\n", mode_cfg);

	/* Tput mode */
	if (mode_cfg & DPMAIF_MD_TPUT_MODE_SET_MASK)
		test_mode_cfg->md_tput_mode = (mode_cfg >> DPMAIF_MD_TPUT_MODE_OFFSET) &
			DPMAIF_MD_TPUT_MODE_MASK;

	/* Start/stop dl test */
	if (mode_cfg & DPMAIF_MD_TPUT_CTL_SET_MASK)
		test_mode_cfg->md_start_dl_tput = (mode_cfg >> DPMAIF_MD_DL_CTL_OFFSET) &
			DPMAIF_MD_DL_CTL_MASK;

	/* pkt_count/ms */
	if (mode_cfg & DPMAIF_MD_TPUT_PKT_SET_MASK)
		test_mode_cfg->md_pkt_number_per_ms = (mode_cfg >> DPMAIF_MD_PKT_OFFSET) &
			DPMAIF_MD_PKT_MASK;

	/* DL test time */
	if (mode_cfg & DPMAIF_MD_TPUT_TIME_SET_MASK)
		test_mode_cfg->md_dl_tput_test_time = (mode_cfg >> DPMAIF_MD_DL_TIME_OFFSET) &
			DPMAIF_MD_DL_TIME_MASK;

	pr_notice("test configuration: mode=%u, dl_start=%u, pkts/ms=%u, test_time=%u\n",
		  test_mode_cfg->md_tput_mode, test_mode_cfg->md_start_dl_tput,
		test_mode_cfg->md_pkt_number_per_ms, test_mode_cfg->md_dl_tput_test_time);

	return cnt;
}

MTK_DBGFS(test_mode, &dpmaif_test_mode_read, &dpmaif_test_mode_write);

static ssize_t dpmaif_tput_monitor_read(void *data, char *buf, ssize_t max_cnt)
{
	struct mtk_dpmaif_ctlb *dcb = data;

	return snprintf(buf, max_cnt, "ul:%d, dl:%d\n",
		dcb->tput_stats.ul_tput_monitor_enabled, dcb->tput_stats.dl_tput_monitor_enabled);
}

#define UL_TPUT_CFG_MASK BIT(0)
#define DL_TPUT_CFG_MASK BIT(1)
static ssize_t dpmaif_tput_monitor_write(void *data, const char *buf, ssize_t cnt)
{
	struct mtk_dpmaif_ctlb *dcb = data;
	unsigned int user_setting;
	int ret;

	ret = kstrtou32(buf, 16, &user_setting);
	if (ret) {
		pr_notice("Invalid parameter, Please check input string format\n");
		return -EFAULT;
	}

	dcb->tput_stats.ul_tput_monitor_enabled = !!(user_setting & UL_TPUT_CFG_MASK);
	dcb->tput_stats.dl_tput_monitor_enabled = !!(user_setting & DL_TPUT_CFG_MASK);

	return cnt;
}

MTK_DBGFS(tput_monitor, &dpmaif_tput_monitor_read, &dpmaif_tput_monitor_write);

#endif

static ssize_t dpmaif_dump_flag_read(void *data, char *buf, ssize_t max_cnt)
{
	struct mtk_dpmaif_ctlb *dcb = data;

	return snprintf(buf, max_cnt, "dpmaif dump flag=0x%lx\n", dcb->dump_flag);
}

static ssize_t dpmaif_dump_flag_write(void *data, const char *buf, ssize_t cnt)
{
	struct mtk_dpmaif_ctlb *dcb = data;
	unsigned int user_mask;
	int ret;

	ret = kstrtou32(buf, 16, &user_mask);
	if (ret) {
		pr_notice("Invalid parameter, Please check input string format\n");
		return -EFAULT;
	}

	dcb->dump_flag = user_mask;

	return cnt;
}

MTK_DBGFS(dump_flag, &dpmaif_dump_flag_read, &dpmaif_dump_flag_write);

static ssize_t dpmaif_dump_read(void *data, char *buf, ssize_t max_cnt)
{
	if (data)
		dpmaif_dump((struct mtk_dpmaif_ctlb *)data);

	return 0;
}

MTK_DBGFS(dump, &dpmaif_dump_read, NULL);

static int dpmaif_debugfs_init(struct mtk_dpmaif_ctlb *dcb)
{
	struct dentry *parent = mtk_get_dev_dentry(DCB_TO_MDEV(dcb));

	if (!parent) {
		pr_notice("Failed to get host device debugfs parent dir.\n");
		return -EFAULT;
	}

	/* Create dpmaif debugfs folder. */
	dcb->dpmaif_dir = mtk_dbgfs_create_dir(parent, "data_plane");
	if (!dcb->dpmaif_dir) {
		pr_notice("Failed to create dir.\n");
		return -EFAULT;
	}

#ifdef CONFIG_DATA_TEST_MODE
	mtk_dbgfs_create_file(dcb->dpmaif_dir, &mtk_dbgfs_test_mode, dcb);
	mtk_dbgfs_create_file(dcb->dpmaif_dir, &mtk_dbgfs_tput_monitor, dcb);
#endif
	mtk_dbgfs_create_file(dcb->dpmaif_dir, &mtk_dbgfs_dump_flag, dcb);
	mtk_dbgfs_create_file(dcb->dpmaif_dir, &mtk_dbgfs_dump, dcb);

	return 0;
}

static void dpmaif_debugfs_exit(struct mtk_dpmaif_ctlb *dcb)
{
	mtk_dbgfs_remove(dcb->dpmaif_dir);
	dcb->dpmaif_dir = NULL;
}
#else
static inline int dpmaif_debugfs_init(struct mtk_dpmaif_ctlb *dcb)
{
	return 0;
}

static inline void dpmaif_debugfs_exit(struct mtk_dpmaif_ctlb *dcb)
{
}
#endif

static int mtk_dpmaif_sw_init(struct mtk_data_blk *data_blk)
{
	struct mtk_dpmaif_ctlb *dcb;
	int ret;

	dcb = devm_kzalloc(data_blk->mdev->dev, sizeof(*dcb), GFP_KERNEL);
	if (!dcb) {
		MTK_ERR(data_blk->mdev, "Failed to allocate dpmaif_ctlb\n");
		return -ENOMEM;
	}

	data_blk->dcb = dcb;
	dcb->data_blk = data_blk;
	dcb->dpmaif_state = DPMAIF_STATE_PWROFF;
	dcb->dpmaif_pm_ready = true;
	dcb->dpmaif_user_ready = false;
	dcb->trans_enabled = false;
	dcb->dump_flag = 0;
	dcb->err_event = 0;
	atomic_set(&dcb->dump_once, 0);
	mutex_init(&dcb->trans_ctl_lock);
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	dcb->online_cpus = num_online_cpus();
	MTK_INFO(data_blk->mdev, "dcb->online_cpus = %u\n", dcb->online_cpus);
	if (dcb->online_cpus <= 1) {
		MTK_INFO(data_blk->mdev, "Only one CPU online, no need to set CPU affinity.\n");
		//modified by wpg for single cpu debug
		//dcb->aff_cfg = NULL;
		dcb->aff_cfg = &data_cpu_loading_cfg[4];
	} else if (dcb->online_cpus < 4) {
		dcb->aff_cfg = &data_cpu_loading_cfg[0];
	} else if (dcb->online_cpus < 8) {
		dcb->aff_cfg = &data_cpu_loading_cfg[1];
	} else {
		dcb->aff_cfg = &data_cpu_loading_cfg[2];
	}
#endif
#ifdef CONFIG_DATA_TEST_MODE
	memset(&dcb->test_mode_cfg, 0x00, sizeof(struct dpmaif_test_mode_cfg));
	dcb->test_mode_cfg.md_tput_mode = DPMAIF_MD_INVALID_MODE;

	timer_setup(&dcb->traffic_monitor,
		    dpmaif_traffic_monitor_func, 0);
#endif

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	mutex_init(&dcb->doorbell_timer_lock);
	timer_setup(&dcb->ring_rel_ctrl_timer, mtk_dpmaif_ring_rel_ctrl_timer_func, 0);
#endif

	/* interrupt coalesce init */
	dcb->intr_coalesce.rx_coalesced_frames = DPMAIF_DFLT_INTR_RX_COA_FRAMES;
	dcb->intr_coalesce.tx_coalesced_frames = DPMAIF_DFLT_INTR_TX_COA_FRAMES;
	dcb->intr_coalesce.rx_coalesce_usecs = DPMAIF_DFLT_INTR_RX_COA_USECS;
	dcb->intr_coalesce.tx_coalesce_usecs = DPMAIF_DFLT_INTR_TX_COA_USECS;

	dpmaif_debugfs_init(dcb);

	/* Check and set normal and frag bat buffer size. */
	mtk_dpmaif_set_bat_buf_size(dcb, dpmaif_mtu);

	ret = mtk_dpmaif_drv_res_init(dcb);
	if (ret < 0)
		goto free_drv_res;

	ret = mtk_dpmaif_bm_pool_init(dcb);
	if (ret < 0)
		goto free_bm_pool;

	ret = mtk_dpmaif_sw_res_init(dcb);
	if (ret < 0)
		goto free_sw_res;

	ret = mtk_dpmaif_tx_srvs_init(dcb);
	if (ret < 0)
		goto free_tx_res;

	ret = mtk_dpmaif_cmd_srvs_init(dcb);
	if (ret < 0)
		goto free_ctl_res;

	ret = mtk_dpmaif_port_init(dcb);
	if (ret < 0)
		goto free_port;

	ret = mtk_dpmaif_pm_init(dcb);
	if (ret < 0)
		goto free_pm;

	ret = mtk_dpmaif_fsm_init(dcb);
	if (ret < 0)
		goto free_fsm;

	ret = mtk_dpmaif_irq_init(dcb);
	if (ret < 0)
		goto free_irq;

	MTK_INFO(data_blk->mdev, "dpmaif sw init done\n");

	return 0;

free_irq:
	mtk_dpmaif_fsm_exit(dcb);
free_fsm:
	mtk_dpmaif_pm_exit(dcb);
free_pm:
	mtk_dpmaif_port_exit(dcb);
free_port:
	mtk_dpmaif_cmd_srvs_exit(dcb);
free_ctl_res:
	mtk_dpmaif_tx_srvs_exit(dcb);
free_tx_res:
	mtk_dpmaif_sw_res_exit(dcb);
free_sw_res:
	mtk_dpmaif_bm_pool_exit(dcb);
free_bm_pool:
	mtk_dpmaif_drv_res_exit(dcb);

free_drv_res:
	dpmaif_debugfs_exit(dcb);

	devm_kfree(DCB_TO_DEV(dcb), dcb);
	data_blk->dcb = NULL;

	return ret;
}

static int mtk_dpmaif_sw_exit(struct mtk_data_blk *data_blk)
{
	struct mtk_dpmaif_ctlb *dcb = data_blk->dcb;
	int ret = 0;

	if (!data_blk->dcb) {
		pr_err("Invalid parameter\n");
		return -EINVAL;
	}

	mtk_dpmaif_irq_exit(dcb);
	mtk_dpmaif_fsm_exit(dcb);
	mtk_dpmaif_pm_exit(dcb);
	mtk_dpmaif_port_exit(dcb);
	mtk_dpmaif_cmd_srvs_exit(dcb);
	mtk_dpmaif_tx_srvs_exit(dcb);
	mtk_dpmaif_sw_res_exit(dcb);
	mtk_dpmaif_bm_pool_exit(dcb);
	mtk_dpmaif_drv_res_exit(dcb);
	dpmaif_debugfs_exit(dcb);

	devm_kfree(DCB_TO_DEV(dcb), dcb);
	MTK_INFO(data_blk->mdev, "dpmaif sw exit done\n");

	return ret;
}

static int mtk_dpmaif_poll_rx_pit(struct dpmaif_rxq *rxq)
{
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	unsigned int sw_rd_idx, hw_wr_idx;
	unsigned int pit_cnt;
	int ret;

	sw_rd_idx = rxq->pit_rd_idx;
	ret = mtk_dpmaif_drv_get_ring_idx(dcb->drv_info, DPMAIF_PIT_WIDX, rxq->id);
	if (unlikely(ret < 0)) {
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Failed to read rxq%u hw pit_wr_idx, ret=%d\n", rxq->id, ret);
		mtk_dpmaif_common_err_handle(dcb, true);
		goto out;
	}

	hw_wr_idx = ret;
	pit_cnt = mtk_dpmaif_ring_buf_readable(rxq->pit_cnt, sw_rd_idx, hw_wr_idx);
	rxq->pit_wr_idx = hw_wr_idx;

	return pit_cnt;

out:
	return ret;
}

#define DPMAIF_POLL_STEP 20
#define DPMAIF_POLL_PIT_CNT_MAX 100
#define DPMAIF_PIT_SEQ_CHECK_FAIL_CNT 2500

static int mtk_dpmaif_check_pit_seq(struct dpmaif_rxq *rxq, struct dpmaif_pd_pit *pit)
{
	unsigned int expect_pit_seq, cur_pit_seq;
	unsigned int count = 0;
	int ret;

	expect_pit_seq = rxq->pit_seq_expect;
	/* The longest check time is 2ms, step is 20us */
	while (count <= DPMAIF_POLL_PIT_CNT_MAX) {
		cur_pit_seq = FIELD_GET(PIT_PD_SEQ, le32_to_cpu(pit->pd_footer));
		if (cur_pit_seq > DPMAIF_PIT_SEQ_MAX) {
			MTK_ERR(DCB_TO_MDEV(rxq->dcb),
				"Invalid rxq%u pit sequence number, cur_seq(%u) > max_seq(%u)\n",
				rxq->id, cur_pit_seq, DPMAIF_PIT_SEQ_MAX);
			break;
		}
		if (cur_pit_seq == expect_pit_seq) {
			rxq->pit_seq_expect++;
			if (rxq->pit_seq_expect >= DPMAIF_PIT_SEQ_MAX)
				rxq->pit_seq_expect = 0;

			rxq->pit_seq_fail_cnt = 0;
			ret = 0;

			goto out;
		} else {
			count++;
		}

		udelay(DPMAIF_POLL_STEP);
	}

	MTK_INFO_RATELIMITED(DCB_TO_MDEV(rxq->dcb),
			     "Failed to check rxq%u pit seq, cur_seq(%u) != exp_seq(%u)\n",
			rxq->id, cur_pit_seq, expect_pit_seq);

	/* If pit sequence doesn't pass in 5 seconds. */
	ret = -DATA_PIT_SEQ_CHK_FAIL;
	rxq->pit_seq_fail_cnt++;
	if (rxq->pit_seq_fail_cnt >= DPMAIF_PIT_SEQ_CHECK_FAIL_CNT) {
		extern void t800_send_platform_uevent(int id, int sub_id, int value1, int value2, int value3);
		t800_send_platform_uevent(5/*MTK_UEVENT_MODEM_MONITOR*/, 8/*MODEM_MONITOR_865CREASON*/, 5, 0, 0);
		mtk_dpmaif_common_err_handle(rxq->dcb, true);
		rxq->pit_seq_fail_cnt = 0;
	}

out:
	return ret;
}

static void mtk_dpmaif_rx_msg_pit(struct dpmaif_rxq *rxq, struct dpmaif_msg_pit *msg_pit,
				  struct dpmaif_rx_record *rx_record)
{
	rx_record->cur_ch_id = FIELD_GET(PIT_MSG_CHNL_ID, le32_to_cpu(msg_pit->dword1));
	rx_record->checksum = FIELD_GET(PIT_MSG_CHECKSUM, le32_to_cpu(msg_pit->dword1));
	rx_record->pit_dp = FIELD_GET(PIT_MSG_DP, le32_to_cpu(msg_pit->dword1));
	rx_record->hash = FIELD_GET(PIT_MSG_HASH, le32_to_cpu(msg_pit->dword3));

	MTK_DBG(DCB_TO_MDEV(rxq->dcb), MTK_DBG_DPMF, MTK_DATA_RX_MEMLOG_RG(rxq->id),
		"rxq%u msg pit: ch=%u, cs=%u, dp=%u\n",
		rxq->id, rx_record->cur_ch_id,
		rx_record->checksum, rx_record->pit_dp);
}

static int mtk_dpmaif_pit_bid_check(struct dpmaif_rxq *rxq, unsigned int cur_bid)
{
	union dpmaif_bat_record *cur_bat_record;
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	struct dpmaif_bat_ring *bat_ring;
	int ret = 0;

	bat_ring = &rxq->dcb->bat_info.normal_bat_ring;
	cur_bat_record = bat_ring->sw_record_base + cur_bid;

	if (unlikely(!cur_bat_record->normal.skb || cur_bid >= bat_ring->bat_cnt)) {
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Invalid parameter rxq%u bat%d, bid=%u, bat_cnt=%u\n",
			rxq->id, bat_ring->type, cur_bid, bat_ring->bat_cnt);

		ret = -DATA_FLOW_CHK_ERR;
	} else {
		rxq->pit_bid = cur_bid;
	}

	MTK_DBG(DCB_TO_MDEV(rxq->dcb), MTK_DBG_DPMF, MTK_DATA_RX_MEMLOG_RG(rxq->id),
		"rxq%u bat%d, cur_bid=%u\n",
		    rxq->id, bat_ring->type, cur_bid);

	return ret;
}

static int mtk_dpmaif_rx_set_data_to_skb(struct dpmaif_rxq *rxq, struct dpmaif_pd_pit *pit_info,
					 struct dpmaif_rx_record *rx_record)
{
	struct dpmaif_bat_ring *bat_ring = &rxq->dcb->bat_info.normal_bat_ring;
	unsigned long long data_dma_addr, data_dma_base_addr;
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	union dpmaif_bat_record *bat_record;
	struct sk_buff *new_skb;
	unsigned int data_len, hd_offset;
	int data_offset;

	bat_record = bat_ring->sw_record_base + mtk_dpmaif_pit_bid(pit_info);
	new_skb = bat_record->normal.skb;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	prefetchw(new_skb);
#endif
	data_dma_base_addr = (unsigned long long)bat_record->normal.data_dma_addr;

	dma_unmap_single(dcb->skb_pool->dev, bat_record->normal.data_dma_addr,
			 bat_record->normal.data_len, DMA_FROM_DEVICE);

	/* Calculate data address and data length. */
	data_dma_addr = le32_to_cpu(pit_info->addr_high);
	data_dma_addr = (data_dma_addr << 32) + le32_to_cpu(pit_info->addr_low);
	data_offset = (int)(data_dma_addr - data_dma_base_addr);
	data_len = FIELD_GET(PIT_PD_DATA_LEN, le32_to_cpu(pit_info->pd_header));

	/* Only the header_offset of the first packet of lro skb is zero,
	 * and other packet's header_offset is not zero.
	 * The data_len is the packet len that has subtracted the packet header length.
	 */
	hd_offset = (FIELD_GET(PIT_PD_HD_OFFSET, le32_to_cpu(pit_info->pd_footer)) << 2);

	/* Check and rebuild skb. */
	new_skb->len = 0;
	if (unlikely((new_skb->tail + data_offset + hd_offset + data_len) > new_skb->end)) {
		MTK_ERR(DCB_TO_MDEV(dcb),
			"pkt(%u/%u):data_len=%u,hd_offset=%u,offset=0x%llx-0x%llx,skb(%llx,%llx,%u,%u)\n",
		rxq->pit_rd_idx, mtk_dpmaif_pit_bid(pit_info), data_len, hd_offset,
		data_dma_addr, data_dma_base_addr, (u64)new_skb->head,
		(u64)new_skb->data, (unsigned int)new_skb->tail,
		(unsigned int)new_skb->end);

		dpmaif_dump_rxq_pit_info(rxq, 2);

		return -DATA_FLOW_CHK_ERR;
	}

	skb_put(new_skb, data_offset + hd_offset + data_len);
	skb_pull(new_skb, data_offset + hd_offset);

	rx_record->cur_skb = new_skb;
	bat_record->normal.skb = NULL;

	return 0;
}

static int mtk_dpmaif_bat_ring_set_mask(struct mtk_dpmaif_ctlb *dcb, enum dpmaif_bat_type type,
					unsigned int bat_idx)
{
	struct dpmaif_bat_ring *bat_ring;
	int ret = 0;

	if (type == NORMAL_BAT)
		bat_ring = &dcb->bat_info.normal_bat_ring;
	else
		bat_ring = &dcb->bat_info.frag_bat_ring;

	if (likely(bat_ring->mask_tbl[bat_idx] == 0)) {
		bat_ring->mask_tbl[bat_idx] = 1;
	} else {
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Invalid bat%u mask_table[%u] value\n", type, bat_idx);
		ret = -DATA_FLOW_CHK_ERR;
	}

	return ret;
}

static void mtk_dpmaif_lro_add_skb(struct dpmaif_rxq *rxq,
				   struct dpmaif_rx_record *rx_record)
{
	struct sk_buff *parent = rx_record->lro_parent;
	struct sk_buff *last = rx_record->lro_last_skb;
	struct sk_buff *cur_skb = rx_record->cur_skb;

	if (!cur_skb) {
		MTK_ERR(DCB_TO_MDEV(rxq->dcb), "Invalid cur_skb\n");
		return;
	}

	if (parent) {
		/* Update the len, data_len, truesize of the lro skb. */
		parent->len += cur_skb->len;
		parent->data_len += cur_skb->len;
		parent->truesize += cur_skb->truesize;
		if (last)
			last->next = cur_skb;
		else
			skb_shinfo(parent)->frag_list = cur_skb;

		last = cur_skb;
		rx_record->lro_last_skb = last;
	} else {
		parent = cur_skb;
		rx_record->lro_parent = parent;
	}

	rx_record->lro_pkt_cnt++;
}

static int mtk_dpmaif_get_rx_pkt(struct dpmaif_rxq *rxq, struct dpmaif_pd_pit *pit_info,
				 struct dpmaif_rx_record *rx_record)
{
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	unsigned int cur_bid;
	int ret;

	cur_bid = mtk_dpmaif_pit_bid(pit_info);

	/* Check the bid in pit information, don't exceed bat size. */
	ret = mtk_dpmaif_pit_bid_check(rxq, cur_bid);
	if (unlikely(ret < 0))
		goto out;

	/* Receive data from bat and save to rx_record. */
	ret = mtk_dpmaif_rx_set_data_to_skb(rxq, pit_info, rx_record);
	if (unlikely(ret < 0))
		goto out;

	/* Set bat mask that have been received. */
	ret = mtk_dpmaif_bat_ring_set_mask(dcb, NORMAL_BAT, cur_bid);
	if (unlikely(ret < 0))
		goto out;

	/* Set current skb to lro skb. */
	mtk_dpmaif_lro_add_skb(rxq, rx_record);

	return 0;

out:
	return ret;
}

static int mtk_dpmaif_pit_bid_frag_check(struct dpmaif_rxq *rxq, unsigned int cur_bid)
{
	union dpmaif_bat_record *cur_bat_record;
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	struct dpmaif_bat_ring *bat_ring;
	int ret = 0;

	bat_ring = &rxq->dcb->bat_info.frag_bat_ring;
	cur_bat_record = bat_ring->sw_record_base + cur_bid;
	if (unlikely(!cur_bat_record->frag.page || cur_bid >= bat_ring->bat_cnt)) {
		MTK_ERR(DCB_TO_MDEV(dcb),
			"Invalid parameter rxq%u bat%d, bid=%u, bat_cnt=%u\n",
			rxq->id, bat_ring->type, cur_bid, bat_ring->bat_cnt);
		ret = -DATA_FLOW_CHK_ERR;
	}

	MTK_DBG(DCB_TO_MDEV(rxq->dcb), MTK_DBG_DPMF, MTK_DATA_RX_MEMLOG_RG(rxq->id),
		"rxq%u bat%d, cur_bid=%u\n",
		    rxq->id, bat_ring->type, cur_bid);

	return ret;
}

static void mtk_dpmaif_lro_add_frag(struct dpmaif_rx_record *rx_record, unsigned int frags_len)
{
	struct sk_buff *frags_base_skb = rx_record->cur_skb;
	struct sk_buff *parent = rx_record->lro_parent;

	/*
	 * The frags item do not belong to the lro parent skb,
	 * it belongs to the lro frags skb, so, must update the lro parent skb data.
	 */
	if (parent != frags_base_skb) {
		/* Non-linear zone data length(frags[] and frag_list). */
		parent->data_len += frags_len;
		/* Non-linear zone data length + linear zone data length. */
		parent->len += frags_len;
		/* The all data length. */
		parent->truesize += frags_len;
	}
}

static int mtk_dpmaif_rx_set_frag_to_skb(struct dpmaif_rxq *rxq, struct dpmaif_pd_pit *pit_info,
					 struct dpmaif_rx_record *rx_record)
{
	struct dpmaif_bat_ring *bat_ring = &rxq->dcb->bat_info.frag_bat_ring;
	unsigned long long data_dma_addr, data_dma_base_addr;
	struct sk_buff *base_skb = rx_record->cur_skb;
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	union dpmaif_bat_record *bat_record;
	struct page_mapped_t *cur_frag;
	unsigned int page_offset;
	unsigned int data_len;
	struct page *page;
	int data_offset;

	bat_record = bat_ring->sw_record_base + mtk_dpmaif_pit_bid(pit_info);
	cur_frag = &bat_record->frag;
	page = cur_frag->page;
	page_offset = cur_frag->offset;
	data_dma_base_addr = (unsigned long long)cur_frag->data_dma_addr;

	dma_unmap_page(DCB_TO_DEV(dcb), cur_frag->data_dma_addr,
		       cur_frag->data_len, DMA_FROM_DEVICE);

	/* Calculate data address and data length. */
	data_dma_addr = le32_to_cpu(pit_info->addr_high);
	data_dma_addr = (data_dma_addr << 32) + le32_to_cpu(pit_info->addr_low);
	data_offset = (int)(data_dma_addr - data_dma_base_addr);
	data_len = FIELD_GET(PIT_PD_DATA_LEN, le32_to_cpu(pit_info->pd_header));

	/* Add fragment data to cur_skb->frags[]. */
	skb_add_rx_frag(base_skb, skb_shinfo(base_skb)->nr_frags, page,
			page_offset + data_offset, data_len, cur_frag->data_len);

	/* Record data length to lro parent. */
	mtk_dpmaif_lro_add_frag(rx_record, data_len);

	cur_frag->page = NULL;

	return 0;
}

static int mtk_dpmaif_get_rx_frag(struct dpmaif_rxq *rxq, struct dpmaif_pd_pit *pit_info,
				  struct dpmaif_rx_record *rx_record)
{
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	unsigned int cur_bid;
	int ret;

	cur_bid = mtk_dpmaif_pit_bid(pit_info);

	/* Check the bid in pit information, don't exceed frag bat size. */
	ret = mtk_dpmaif_pit_bid_frag_check(rxq, cur_bid);
	if (unlikely(ret < 0))
		goto out;

	/* Receive data from frag bat and save to currunt skb. */
	ret = mtk_dpmaif_rx_set_frag_to_skb(rxq, pit_info, rx_record);
	if (unlikely(ret < 0))
		goto out;

	/* Set bat mask that have been received. */
	ret = mtk_dpmaif_bat_ring_set_mask(dcb, FRAG_BAT, cur_bid);
	if (unlikely(ret < 0))
		goto out;

	return 0;
out:
	return ret;
}

static void mtk_dpmaif_set_rcsum(struct sk_buff *skb, unsigned int hw_checksum_state)
{
	if (hw_checksum_state == CS_RESULT_PASS)
		skb->ip_summed = CHECKSUM_UNNECESSARY;
	else
		skb->ip_summed = CHECKSUM_NONE;
}

static void mtk_dpmaif_set_rxhash(struct sk_buff *skb, u32 hw_hash)
{
	skb_set_hash(skb, hw_hash, PKT_HASH_TYPE_L4);
}

static int mtk_dpmaif_rx_skb(struct dpmaif_rxq *rxq, struct dpmaif_rx_record *rx_record)
{
	struct sk_buff *new_skb = rx_record->lro_parent;
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	int ret = 0;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	union mtk_data_pkt_info *pkt_info;
#endif

	if (unlikely(rx_record->pit_dp)) {
		MTK_WARN(DCB_TO_MDEV(dcb), "sw should drop this packet\n");
		dcb->traffic_stats.rx_hw_ind_dropped[rxq->id]++;
		dev_kfree_skb_any(new_skb);
		goto out;
	}

#ifdef CONFIG_DATA_TEST_MODE
	dl_stats_update(dcb, new_skb->len);
#endif

	/* Check HW rx checksum offload status. */
	mtk_dpmaif_set_rcsum(new_skb, rx_record->checksum);

	/* Set skb hash from HW. */
	mtk_dpmaif_set_rxhash(new_skb, rx_record->hash);

	skb_record_rx_queue(new_skb, rxq->id);

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_DATA_RX_MEMLOG_RG(rxq->id),
		"full lro skb:%u+%u=%u -> %u, lro_pkt_cnt=%u\n",
		    skb_headlen(new_skb), new_skb->data_len, new_skb->len,
		    new_skb->truesize, rx_record->lro_pkt_cnt);
#ifdef CONFIG_DEBUG_FS
	if (test_bit(DPMAIF_DUMP_RX_PKT, &dcb->dump_flag))
		skb_dump(KERN_DEBUG, new_skb, false);
#endif

	trace_mtk_tput_data_lro_cnt(rxq->id, rx_record->lro_pkt_cnt);
	trace_mtk_tput_data_rx(rxq->id, "rx p3");

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	pkt_info = DPMAIF_SKB_CB(new_skb);
	pkt_info->rx.ch_id = rx_record->cur_ch_id;

	skb_queue_tail(&rxq->list, new_skb);
	trace_mtk_tput_data_rx(rxq->id, "rx p3.1");

	queue_work_on(rxq->cpu_id, rxq->steer_wq, &rxq->steer_work);
	trace_mtk_tput_data_rx(rxq->id, "rx p3.2");
#else
	/* Send skb to data port. */
	ret = dcb->port_ops->recv(dcb->data_blk, new_skb, rxq->id, rx_record->cur_ch_id);
#endif
	dcb->traffic_stats.rx_packets[rxq->id]++;
out:
	rx_record->lro_parent = NULL;
	return ret;
}

static int mtk_dpmaif_recycle_pit_internal(struct dpmaif_rxq *rxq, unsigned short pit_rel_cnt)
{
	unsigned short old_sw_rel_idx, new_sw_rel_idx, old_hw_wr_idx;
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	int ret = 0;

	MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_DATA_RX_MEMLOG_RG(rxq->id),
		"pit(%u): w=%u,r=%u,rel=%u,rel_cnt=%u\n",
		    rxq->id, rxq->pit_wr_idx, rxq->pit_rd_idx, rxq->pit_rel_rd_idx, pit_rel_cnt);

	trace_mtk_tput_data_pit(rxq->id, rxq->pit_rel_rd_idx,
				rxq->pit_wr_idx, rxq->pit_rd_idx, pit_rel_cnt);

	old_sw_rel_idx = rxq->pit_rel_rd_idx;
	new_sw_rel_idx = old_sw_rel_idx + pit_rel_cnt;
	old_hw_wr_idx = rxq->pit_wr_idx;

	/* pit_rel_rd_idx should not exceed pit_wr_idx. */
	if (old_hw_wr_idx > old_sw_rel_idx) {
		if (new_sw_rel_idx > old_hw_wr_idx)
			MTK_WARN(DCB_TO_MDEV(dcb), "new_rel_idx > old_hw_wr_idx\n");
	} else if (old_hw_wr_idx < old_sw_rel_idx) {
		if (new_sw_rel_idx >= rxq->pit_cnt) {
			new_sw_rel_idx = new_sw_rel_idx - rxq->pit_cnt;
			if (new_sw_rel_idx > old_hw_wr_idx)
				MTK_WARN(DCB_TO_MDEV(dcb), "new_rel_idx > old_wr_idx\n");
		}
	}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	atomic_add(pit_rel_cnt, &rxq->pit_rel_cnt);
#else
	/* Notify the available pit count to HW. */
	ret = mtk_dpmaif_drv_send_doorbell(dcb->drv_info, DPMAIF_PIT, rxq->id, pit_rel_cnt);
	if (unlikely(ret < 0)) {
		MTK_ERR(DCB_TO_MDEV(rxq->dcb),
			"Failed to send pit doorbell,pit:r=%u,w=%u,rel=%u, rel_cnt=%u, ret=%d\n",
			rxq->pit_rd_idx, rxq->pit_wr_idx, rxq->pit_rel_rd_idx, pit_rel_cnt, ret);
		mtk_dpmaif_common_err_handle(dcb, true);
	}
#endif

	rxq->pit_rel_rd_idx = new_sw_rel_idx;

	return ret;
}

static int mtk_dpmaif_recycle_rx_ring(struct dpmaif_rxq *rxq)
{
	unsigned int pit_rel_cnt;
	int ret = 0;

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	pit_rel_cnt = mtk_dpmaif_ring_buf_releasable(rxq->pit_cnt,
						     rxq->pit_rel_rd_idx,
						     rxq->pit_rd_idx);
#else
	pit_rel_cnt = rxq->pit_rel_cnt;
#endif

	/* burst recycle check */
	if (pit_rel_cnt < rxq->pit_burst_rel_cnt)
		return 0;

	if (unlikely(pit_rel_cnt > rxq->pit_cnt)) {
		MTK_ERR(DCB_TO_MDEV(rxq->dcb),
			"Invalid rxq%u pit release count, %u>%u\n",
			rxq->id, pit_rel_cnt, rxq->pit_cnt);
		mtk_dpmaif_common_err_handle(rxq->dcb, false);
		ret = -DATA_FLOW_CHK_ERR;
		goto out;
	}

	ret = mtk_dpmaif_recycle_pit_internal(rxq, pit_rel_cnt);

#ifndef CONFIG_REG_ACCESS_OPTIMIZE
	rxq->pit_rel_cnt = 0;

	if (rxq->pit_cnt_err_intr_set) {
		rxq->pit_cnt_err_intr_set = false;
		mtk_dpmaif_drv_intr_complete(rxq->dcb->drv_info,
					     DPMAIF_INTR_DL_PITCNT_LEN_ERR, rxq->id, 0);
	}
#endif

	mtk_dpmaif_queue_bat_reload_work(rxq->dcb);
out:
	return ret;
}

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
static void rxq_pit_cache_memory_flush(struct dpmaif_rxq *rxq,
				       unsigned short cnt)
{
	unsigned int cur_pit = rxq->pit_rd_idx;
	dma_addr_t cache_start_addr;

	/* flush pit base memory cache for read pit data */
	cache_start_addr = rxq->pit_dma_addr + (sizeof(*rxq->pit_base) * cur_pit);

	if ((cur_pit + cnt) <= rxq->pit_cnt) {
		dma_sync_single_for_cpu(DCB_TO_DEV(rxq->dcb), cache_start_addr,
					sizeof(*rxq->pit_base) * cnt, DMA_FROM_DEVICE);

	} else {
		dma_sync_single_for_cpu(DCB_TO_DEV(rxq->dcb), cache_start_addr,
					sizeof(*rxq->pit_base) * (rxq->pit_cnt - cur_pit),
			DMA_FROM_DEVICE);

		dma_sync_single_for_cpu(DCB_TO_DEV(rxq->dcb), rxq->pit_dma_addr,
					sizeof(*rxq->pit_base) * (cur_pit + cnt - rxq->pit_cnt),
			DMA_FROM_DEVICE);
	}
}
#endif

static int mtk_dpmaif_rx_data_collect_internal(struct dpmaif_rxq *rxq, int pit_cnt, int budget,
					       unsigned int *pkt_cnt)
{
	struct dpmaif_rx_record *rx_record = &rxq->rx_record;
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	struct dpmaif_pd_pit *pit_info;
	unsigned int recv_pkt_cnt = 0;
	unsigned int rx_cnt, cur_pit;
	int ret;

#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	rxq_pit_cache_memory_flush(rxq, pit_cnt);
	prefetch(rxq);
#endif
	cur_pit = rxq->pit_rd_idx;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	prefetch((char *)rxq->pit_base + cur_pit);
#endif
	for (rx_cnt = 0; rx_cnt < pit_cnt; rx_cnt++) {
		/* Check if reach rx packet budget. */
		if (!rx_record->msg_pit_recv) {
			if (recv_pkt_cnt >= budget)
				break;
		}

		/* Pit sequence check. */
		pit_info = rxq->pit_base + cur_pit;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
		prefetch((char *)pit_info + SMP_CACHE_BYTES);
#endif
		ret = mtk_dpmaif_check_pit_seq(rxq, pit_info);
		if (unlikely(ret < 0))
			break;

		/* Parse message pit. */
		if (FIELD_GET(PIT_PD_PKT_TYPE, le32_to_cpu(pit_info->pd_header)) == MSG_PIT) {
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
			prefetchw(rx_record);
#endif
			if (unlikely(rx_record->msg_pit_recv)) {
				if (rx_record->lro_parent) {
					dcb->traffic_stats.rx_errors[rxq->id]++;
					dcb->traffic_stats.rx_dropped[rxq->id]++;
					dev_kfree_skb_any(rx_record->lro_parent);
				}
				memset(&rxq->rx_record, 0x00, sizeof(rxq->rx_record));
				MTK_ERR(DCB_TO_MDEV(dcb),
					"rxq%u two continuous message pit\n", rxq->id);
				mtk_dpmaif_common_err_handle(dcb, true);
			}

			rx_record->msg_pit_recv = true;
			mtk_dpmaif_rx_msg_pit(rxq, (struct dpmaif_msg_pit *)pit_info, rx_record);
		} else {
			/* Parse normal pit or frag pit. */
			if (FIELD_GET(PIT_PD_BUF_TYPE, le32_to_cpu(pit_info->pd_header)) !=
			    FRAG_BAT) {
				ret = mtk_dpmaif_get_rx_pkt(rxq, pit_info, rx_record);
			} else {
				/* Pit sequence: normal pit + frag pit. */
				if (likely(rx_record->cur_skb)) {
					ret = mtk_dpmaif_get_rx_frag(rxq, pit_info, rx_record);
				} else {
					/* Unexpected pit sequence: message pit + frag pit. */
					MTK_WARN(DCB_TO_MDEV(dcb),
						 "unexpected rxq%u frag pit, pit=%u,bid=%u; rx_cnt=%u, pit_cnt=%u\n",
							rxq->id, cur_pit,
							mtk_dpmaif_pit_bid(pit_info),
							rx_cnt, pit_cnt);
					ret = -DATA_FLOW_CHK_ERR;
				}
			}

			if (unlikely(ret < 0)) {
				/* Move on pit index to skip error data. */
				rx_record->err_payload = 1;
				MTK_ERR(DCB_TO_MDEV(dcb),
					"rxq%u error payload, pit_idx=%d\n",
						     rxq->id, cur_pit);
				mtk_dpmaif_common_err_handle(dcb, true);
			}

			/* Last one pit of a packet. */
			if (FIELD_GET(PIT_PD_CONT, le32_to_cpu(pit_info->pd_header)) ==
			    DPMAIF_PIT_LASTONE) {
				if (likely(rx_record->err_payload == 0)) {
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
					prefetchw(rx_record);
#endif
					mtk_dpmaif_rx_skb(rxq, rx_record);
				} else {
					if (rx_record->cur_skb) {
						dcb->traffic_stats.rx_errors[rxq->id]++;
						dcb->traffic_stats.rx_dropped[rxq->id]++;
						dev_kfree_skb_any(rx_record->lro_parent);
						rx_record->lro_parent = NULL;
					}
				}
				memset(&rxq->rx_record, 0x00, sizeof(rxq->rx_record));
				recv_pkt_cnt++;
			}
		}

		cur_pit = mtk_dpmaif_ring_buf_get_next_idx(rxq->pit_cnt, cur_pit);
		rxq->pit_rd_idx = cur_pit;
	}

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	atomic_add(rx_cnt, &rxq->pit_stats);
#else
	rxq->pit_rel_cnt += rx_cnt;
#endif

	*pkt_cnt = recv_pkt_cnt;

	/* Recycle pit in batches (rx packet budget). */
	mtk_dpmaif_recycle_rx_ring(rxq);

	return ret;
}

static int mtk_dpmaif_rx_data_collect(struct dpmaif_rxq *rxq, int budget, unsigned int *pkt_cnt)
{
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	unsigned int pit_cnt;
	int ret = 0;

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	pit_cnt = mtk_dpmaif_ring_buf_readable(rxq->pit_cnt, rxq->pit_rd_idx, rxq->pit_wr_idx);
	/* only enable rxq->pit_poll_enable will polling pit */
	if (rxq->pit_poll_enable && !pit_cnt) {
		ret = mtk_dpmaif_poll_rx_pit(rxq);
		if (unlikely(ret < 0))
			return ret;

		pit_cnt = ret;
	}
#else
	/* Get pit count that will be collected and update pit_wr_idx from hardware. */
	ret = mtk_dpmaif_poll_rx_pit(rxq);
	if (unlikely(ret < 0))
		return ret;

	pit_cnt = ret;
#endif

	trace_mtk_tput_data_napi_per_poll(rxq->id, pit_cnt);

	/* Collect rx packets. */
	if (likely(pit_cnt > 0)) {
		ret = mtk_dpmaif_rx_data_collect_internal(rxq, pit_cnt, budget, pkt_cnt);
		if (ret <= -DATA_DL_ONCE_MORE) {
			ret = -DATA_DL_ONCE_MORE;
		} else if (ret <= -DATA_ERR_STOP_MAX) {
			ret = -DATA_ERR_STOP_MAX;
			mtk_dpmaif_common_err_handle(dcb, true);
		} else {
			ret = 0;
		}
	}

	return ret;
}

static int mtk_dpmaif_rx_data_collect_more(struct dpmaif_rxq *rxq, int budget, int *work_done)
{
	unsigned int total_pkt_cnt = 0, pkt_cnt;
	int each_budget;
	int ret = 0;

	do {
		each_budget = budget - total_pkt_cnt;
		pkt_cnt = 0;
		ret = mtk_dpmaif_rx_data_collect(rxq, each_budget, &pkt_cnt);
		total_pkt_cnt += pkt_cnt;
		if (ret < 0)
			break;
	} while (total_pkt_cnt < budget && pkt_cnt > 0 && rxq->started);

	*work_done = total_pkt_cnt;

	return ret;
}

static int mtk_dpmaif_rx_napi_poll(struct napi_struct *napi, int budget)
{
	struct dpmaif_rxq *rxq = container_of(napi, struct dpmaif_rxq, napi);
	struct dpmaif_traffic_stats *stats = &rxq->dcb->traffic_stats;
	struct mtk_dpmaif_ctlb *dcb = rxq->dcb;
	int work_done = 0;
	int ret;

	pm_runtime_get(DCB_TO_DEV(dcb));
	if (likely(rxq->started)) {
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
		if (rxq->pit_poll_enable)
			ret = mtk_dpmaif_rx_data_collect_more(rxq, budget, &work_done);
		else
			ret = mtk_dpmaif_rx_data_collect(rxq, budget, &work_done);
#else
		ret = mtk_dpmaif_rx_data_collect_more(rxq, budget, &work_done);
#endif
		stats->rx_done_last_cnt[rxq->id] += work_done;

		if (ret == -DATA_DL_ONCE_MORE) {
			napi_gro_flush(napi, false);
			work_done = budget;
		}
	}

	if (work_done < budget) {
		napi_complete_done(napi, work_done);
		__pm_wakeup_event(rxq->ws, jiffies_to_msecs(HZ));
		mtk_dpmaif_drv_intr_complete(dcb->drv_info, DPMAIF_INTR_DL_DONE, rxq->id, 0);
		trace_mtk_tput_data_napi(rxq->id, stats->rx_done_last_cnt[rxq->id]);
		MTK_DBG(DCB_TO_MDEV(dcb), MTK_DBG_DPMF, MTK_DATA_RX_MEMLOG_RG(rxq->id),
			"pit(%u): w=%u,r=%u,rel=%u,\n",
			    rxq->id, rxq->pit_wr_idx, rxq->pit_rd_idx, rxq->pit_rel_rd_idx);
	}

	pm_runtime_put(DCB_TO_DEV(dcb));

	return work_done;
}

enum dpmaif_pkt_type {
	PKT_UNKNOWN,
	PKT_EMPTY_ACK,
	PKT_ECHO
};

static enum dpmaif_pkt_type mtk_dpmaif_check_skb_type(struct sk_buff *skb, enum mtk_pkt_type type)
{
	int ret = PKT_UNKNOWN;
	struct tcphdr *tcph;
	int inner_offset;
	__be16 frag_off;
	u32 total_len;
	u32 pkt_type;
	u8 l2_hlen;
	u8 nexthdr;

	union {
		struct iphdr *v4;
		struct ipv6hdr *v6;
		unsigned char *hdr;
	} ip;
	union {
		struct icmphdr *v4;
		struct icmp6hdr *v6;
		unsigned char *hdr;
	} icmp;

	switch (type) {
	case ETH_802_3:
		l2_hlen = ETH_HLEN;
		break;
	case ETH_802_1Q:
		l2_hlen = VLAN_ETH_HLEN;
		break;
	case PURE_IP:
		l2_hlen = 0;
		break;
	default:
		l2_hlen = 0;
		break;
	}

	if (l2_hlen)
		skb_pull(skb, l2_hlen);

	pkt_type = skb->data[0] & 0xF0;
	if (pkt_type == IPV4_VERSION) {
		ip.v4 = (struct iphdr *)(skb->data);
		if (ip.v4->protocol == IPPROTO_ICMP) {
			icmp.v4 = (struct icmphdr *)(skb->data + (ip.v4->ihl << 2));
			if (icmp.v4->type == ICMP_ECHO || icmp.v4->type == ICMP_ECHOREPLY)
				ret = PKT_ECHO;
		} else if (ip.v4->protocol == IPPROTO_TCP) {
			tcph = (struct tcphdr *)(skb->data + (ip.v4->ihl << 2));
			if (((ip.v4->ihl << 2) + (tcph->doff << 2)) == (ntohs(ip.v4->tot_len)) &&
			    !tcph->syn && !tcph->fin && !tcph->rst)
				ret = PKT_EMPTY_ACK;
		}
	} else if (pkt_type == IPV6_VERSION) {
		ip.v6 = (struct ipv6hdr *)skb->data;
		nexthdr = ip.v6->nexthdr;
		if (ipv6_ext_hdr(nexthdr)) {
			/* Now skip over extension headers. */
			inner_offset = ipv6_skip_exthdr(skb, sizeof(struct ipv6hdr),
							&nexthdr, &frag_off);
			if (inner_offset < 0)
				goto out;
		} else {
			inner_offset = sizeof(struct ipv6hdr);
		}

		if (nexthdr == IPPROTO_ICMPV6) {
			icmp.v6 = (struct icmp6hdr *)(skb->data + inner_offset);
			if (icmp.v6->icmp6_type == ICMPV6_ECHO_REQUEST ||
			    icmp.v6->icmp6_type == ICMPV6_ECHO_REPLY)
				ret = PKT_ECHO;
		} else if (nexthdr == IPPROTO_TCP) {
			total_len = sizeof(struct ipv6hdr) + ntohs(ip.v6->payload_len);
			tcph = (struct tcphdr *)(skb->data + inner_offset);
			if (((total_len - inner_offset) == (tcph->doff << 2)) &&
			    !tcph->syn && !tcph->fin && !tcph->rst)
				ret = PKT_EMPTY_ACK;
		}
	}

out:
	if (l2_hlen)
		skb_push(skb, l2_hlen);
	return ret;
}

static enum dpmaif_pkt_priority dpmaif_get_pkt_priority(struct sk_buff *skb, enum mtk_pkt_type type)
{
	enum dpmaif_pkt_type pkt_type;
	int pkt_priority;
	u32 mark;

	if (skb->mark) {
		mark = skb->mark >> 21 & 0x7ff;
		if (mark == 0x600) {
			pkt_priority = PRIORITY_3;
			goto exit;
		}
		if (mark == 0x400) {
			pkt_priority = PRIORITY_2;
			goto exit;
		}
		if (mark == 0x200) {
			pkt_priority = PRIORITY_1;
			goto exit;
		}
	}

	pkt_type = mtk_dpmaif_check_skb_type(skb, type);
	if (pkt_type == PKT_EMPTY_ACK)
		pkt_priority = PRIORITY_2;
	else if (pkt_type == PKT_ECHO)
		pkt_priority = PRIORITY_1;
	else
		pkt_priority = PRIORITY_0;

exit:
	return pkt_priority;
}

static int dpmaif_select_txq(struct mtk_data_blk *data_blk,
			     struct sk_buff *skb, enum mtk_pkt_type type,
			     enum dpmaif_pkt_priority pkt_prio)
{
	struct dpmaif_drv_pkt_info pkt_info = {0};

	if (pkt_prio < PKT_PRIO_INVALID)
		pkt_info.prio = pkt_prio;
	else
		pkt_info.prio = dpmaif_get_pkt_priority(skb, type);
	pkt_info.skb_hash = skb_get_hash(skb);

	return mtk_dpmaif_drv_feature_cmd(data_blk->dcb->drv_info,
		DATA_HW_TXQ_GET, (void *)&pkt_info);
}

static int mtk_dpmaif_send_pkt(struct mtk_dpmaif_ctlb *dcb, struct sk_buff *skb,
			       u64 data)
{
	union mtk_data_pkt_info *input_pkt_info = (union mtk_data_pkt_info *)data;
	unsigned char vq_id = skb_get_queue_mapping(skb);
	union mtk_data_pkt_info *pkt_info;
	unsigned char srv_id;
	struct dpmaif_vq *vq;
	int ret = 0;

	pkt_info = DPMAIF_SKB_CB(skb);
	pkt_info->tx.intf_id = input_pkt_info->tx.intf_id;
	pkt_info->tx.network_type = input_pkt_info->tx.network_type;
	pkt_info->tx.drb_cnt = DPMAIF_GET_DRB_CNT(skb);
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	pkt_info->tx.in_tcp_slow_start = input_pkt_info->tx.in_tcp_slow_start;
#endif

	vq = &dcb->tx_vqs[vq_id];
	srv_id = dcb->res_cfg->tx_vq_srv_map[vq_id];
	if (likely(skb_queue_len(&vq->list) < vq->max_len)) {
		skb_queue_tail(&vq->list, skb);
	} else {
		/* Notify to data port layer, data port should carry off the net device tx queue. */
		dcb->port_ops->notify(dcb->data_blk, DATA_EVT_TX_STOP, (u64)1 << vq_id);
		MTK_INFO_RATELIMITED(DCB_TO_MDEV(dcb),
				     "wwan%u: pkt tx vq%u full\n", pkt_info->tx.intf_id, vq_id);
		ret = -EBUSY;
	}

	mtk_dpmaif_wake_up_tx_srv(&dcb->tx_srvs[srv_id]);

	trace_mtk_tput_data_tx("tx p3");
	return ret;
}

static int mtk_dpmaif_send_cmd(struct mtk_dpmaif_ctlb *dcb, struct sk_buff *skb)
{
	struct dpmaif_vq *vq = &dcb->cmd_vq;
	int ret = 0;

	if (likely(skb_queue_len(&vq->list) < vq->max_len)) {
		skb_queue_tail(&vq->list, skb);
	} else {
		MTK_INFO_RATELIMITED(DCB_TO_MDEV(dcb), "cmd tx vq full\n");
		ret = -EBUSY;
	}

	schedule_work(&dcb->cmd_srv.work);

	return ret;
}

static int mtk_dpmaif_send(struct mtk_data_blk *data_blk, enum mtk_data_type type,
			   struct sk_buff *skb, u64 data)
{
	int ret;

	if (unlikely(!data_blk || !skb || !data_blk->dcb)) {
		pr_warn("Invalid parameter\n");
		return -EINVAL;
	}

	if (data_blk->dcb->dpmaif_state == DPMAIF_STATE_PWROFF)
		return -EINVAL;

	if (likely(type == DATA_PKT))
		ret = mtk_dpmaif_send_pkt(data_blk->dcb, skb, data);
	else
		ret = mtk_dpmaif_send_cmd(data_blk->dcb, skb);

	return ret;
}

struct mtk_data_trans_ops data_trans_ops = {
	.poll = mtk_dpmaif_rx_napi_poll,
	.select_txq = dpmaif_select_txq,
	.send = mtk_dpmaif_send,
};

static void mtk_dpmaif_param_check(void)
{
	/* Set max mtu, DPMAIF_DFLT_MTU ~ DPMAIF_JUMBO_SIZE. */
	if (dpmaif_mtu > DPMAIF_JUMBO_SIZE)
		dpmaif_mtu = DPMAIF_JUMBO_SIZE;
	if (dpmaif_mtu < DPMAIF_DFLT_MTU)
		dpmaif_mtu = DPMAIF_DFLT_MTU;

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	if (rx_poll_th > MAX_RX_POLL_TH)
		rx_poll_th = MAX_RX_POLL_TH;

	if (tx_poll_th > MAX_TX_POLL_TH)
		tx_poll_th = MAX_TX_POLL_TH;

	if (max_pit_burst_cnt > 2048)
		max_pit_burst_cnt = 2048;
	if (max_pit_burst_cnt < 64)
		max_pit_burst_cnt = 64;

	if (traffic_stats_shift > 9)
		traffic_stats_shift = 9;
	if (traffic_stats_shift < 3)
		traffic_stats_shift = 3;
#endif
}
/***********************************xiaomi modem driver optimize API start***/
enum modem_optimize_para_id {
	MODEM_PARA_ID_CPU = 1,
	MODEM_PARA_ID_BUDGET = 2,
	MODEM_PARA_ID_MTU = 3,
	MODEM_PARA_ID_RX_THRESH = 4,
	MODEM_PARA_ID_DDRFREQ = 5,
	MODEM_PARA_ID_MODE = 6, // 1:tput 2:normal; 3:low power
	MODEM_PARA_ID_DUMP = 7, //0: base thinmd logic; 1:no dump no reason; 2: no dump with reason; 3: mini 4:full 5: with panic
	MODEM_PARA_ID_DEBUG_LEVEL = 8, // 0:normal 1: print xiaomi warning 2: print xiaomi infor
	MODEM_PARA_ID_POWER_REQ = 9, // 1: power off; 2: power on
	MODEM_PARA_ID_NVRECOVERY_REQ = 10, //
	MODEM_PARA_ID_ME_STATE_REQ = 11, //
	MODEM_PARA_ID_PCIE_GEN_LEVEL = 12, // 1:gen1 2:gen2 3:gen3 4:gen4
	MODEM_PARA_ID_PCIE_RPM = 13, // 0:disable -> resume 1:enable -> suspend
	MODEM_PARA_ID_TMI_LOG = 14, //
	MODEM_PARA_ID_SOCKET = 15, //
	MODEM_PARA_ID_SET = 16, //SET ID for read
	MODEM_PARA_ID_PANIC = 17, //SET ID for read
	MODEM_PARA_ID_MAX
};
enum modem_optimize_modem_mode {
	MODEM_MODE_ID_TPUT = 1,
	MODEM_MODE_ID_NORMAL = 2,
	MODEM_MODE_ID_LP = 3,
	MODEM_MODE_ID_TPUT2 = 4,
	MODEM_MODE_ID_TPUT3 = 5,
};
#define MODEM_OPTIMIZE_PARA_COUNT_MAX 100
extern void xiaomi_modem_cpu_set(int level);
extern void xiaomi_modem_budget_reload(int budget);
extern void xiaomi_modem_ddr_set(unsigned int freq);

int g_xiaomi_debug_level;
void xiaomi_modem_cpu_cfg_set(int level)
{
	switch (level) {
	case 0:
		data_cpu_loading_cfg[2].napi_thrd_aff[0] = 0;
		data_cpu_loading_cfg[2].napi_thrd_aff[1] = 1;
		data_cpu_loading_cfg[2].steer_wq_aff[0] = 2;
		data_cpu_loading_cfg[2].steer_wq_aff[1] = 3;
		break;
	case 1:
		data_cpu_loading_cfg[2].napi_thrd_aff[0] = 2;
		data_cpu_loading_cfg[2].napi_thrd_aff[1] = 3;
		data_cpu_loading_cfg[2].steer_wq_aff[0] = 4;
		data_cpu_loading_cfg[2].steer_wq_aff[1] = 5;
		break;
	case 2:
		data_cpu_loading_cfg[2].napi_thrd_aff[0] = 6;
		data_cpu_loading_cfg[2].napi_thrd_aff[1] = 7;
		data_cpu_loading_cfg[2].steer_wq_aff[0] = 4;
		data_cpu_loading_cfg[2].steer_wq_aff[1] = 5;
		break;
	case 3:
		data_cpu_loading_cfg[2].napi_thrd_aff[0] = 4;
		data_cpu_loading_cfg[2].napi_thrd_aff[1] = 5;
		data_cpu_loading_cfg[2].steer_wq_aff[0] = 6;
		data_cpu_loading_cfg[2].steer_wq_aff[1] = 7;
		break;
	case 4:
		data_cpu_loading_cfg[2].napi_thrd_aff[0] = 8;
		data_cpu_loading_cfg[2].napi_thrd_aff[1] = 9;
		data_cpu_loading_cfg[2].steer_wq_aff[0] = 6;
		data_cpu_loading_cfg[2].steer_wq_aff[1] = 7;
		break;
	case 5:
		data_cpu_loading_cfg[2].napi_thrd_aff[0] = 6;
		data_cpu_loading_cfg[2].napi_thrd_aff[1] = 7;
		data_cpu_loading_cfg[2].steer_wq_aff[0] = 8;
		data_cpu_loading_cfg[2].steer_wq_aff[1] = 9;
		break;
	default:
		break;
	}
}

 void xiaomi_modem_rx_reload(struct mtk_dpmaif_ctlb *dcb)
{
	unsigned char rxq_cnt = dcb->res_cfg->rxq_cnt;
	struct dpmaif_rxq *rxq;
	int i;
	for (i = 0; i < rxq_cnt; i++) {
		rxq = &dcb->rxqs[i];
		rxq->cpu_id = mtk_dpmaif_get_work_cpu(rxq);
		//cancel_work_sync(&rxq->steer_work);
		//queue_work_on(rxq->cpu_id, rxq->steer_wq, &rxq->steer_work);
		MTK_INFO(DCB_TO_MDEV(dcb), "work on cpu %d\n", rxq->cpu_id);
	}
}


#define VOTE_MAX_VALUE          0xFFFF
extern void t800_send_uevent(int id, int sub_id, int value1, int value2, int value3);
typedef ssize_t (*XiaomiModemParaFuncPtr)(struct file *, const char __user *, size_t, loff_t *);
typedef ssize_t (*XiaomiModemParaReadFuncPtr)(struct file *file, char __user *buf, size_t size, loff_t *ppos);
extern int mtk_pwrctl_set_para_cb(XiaomiModemParaFuncPtr func, XiaomiModemParaReadFuncPtr readFunc);
extern int mtk_pwrctl_remove_para_cb(XiaomiModemParaFuncPtr func, XiaomiModemParaReadFuncPtr readFunc);
extern struct mtk_md_dev *g_t800_mdev;
static int read_id;
static struct tcp_info tcp_info;
/* modem_drv_para_set() - set optimization parameters
 * @buffer : parameter ID(one byte) + parameter value (one byte)
 * Return: return value is 0 on success, a negative error code on failure.
 */
 //int static test_count = 0;
ssize_t modem_drv_para_set(struct file *file, const char __user *buffer, size_t count, loff_t *ops)
{
	char para[MODEM_OPTIMIZE_PARA_COUNT_MAX];
	static bool is_tput_mode;
	void t800_send_set_CPU_freq_uevent(int id, int value1, int value2, int value3);
	//pr_err("%s   input para count: %lu,%d", __func__,count,test_count);
	//test_count++;
	if (count > MODEM_OPTIMIZE_PARA_COUNT_MAX) {
		pr_err("%s  failed.  unsupport count: %lu", __func__, count);
		return -1;
	}
	if (copy_from_user((void *)para, (const void __user *)buffer, count))
		return -EFAULT;
	pr_info("%s  id= %x, para=%x\n", __func__, para[0], para[1]);

	switch (para[0]) {
	case MODEM_PARA_ID_CPU:
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
		xiaomi_modem_cpu_set(para[1]);
#endif
		break;
	case MODEM_PARA_ID_BUDGET:
		xiaomi_modem_budget_reload(para[1]*128);
		break;
	case MODEM_PARA_ID_MTU:
		break;
	case MODEM_PARA_ID_RX_THRESH:
		rx_poll_th = para[1]*100;
		break;
	case MODEM_PARA_ID_DDRFREQ:
		if (para[1]) {
			xiaomi_modem_ddr_set(9600);
		} else {
			xiaomi_modem_ddr_set(0);
		}
		break;
	case MODEM_PARA_ID_MODE: {
		switch (para[1]) {
		case MODEM_MODE_ID_TPUT: {
			xiaomi_modem_cpu_set(5);
			xiaomi_modem_budget_reload(2*128);
			xiaomi_modem_ddr_set(VOTE_MAX_VALUE);
			rx_poll_th = 100;
			break;
		}
		case MODEM_MODE_ID_TPUT2: {
			xiaomi_modem_cpu_set(3);
			xiaomi_modem_budget_reload(1*128);
			xiaomi_modem_ddr_set(VOTE_MAX_VALUE);
			rx_poll_th = 400;
			break;
		}
		case MODEM_MODE_ID_TPUT3: {
			xiaomi_modem_cpu_set(4);
			xiaomi_modem_budget_reload(2*128);
			xiaomi_modem_ddr_set(VOTE_MAX_VALUE);
			rx_poll_th = 100;
			t800_send_set_CPU_freq_uevent(1, 0, 0, 0);
			is_tput_mode = true;
			pr_info("%s:  is_tput_mode=%d, flag=%d", __func__, is_tput_mode, MODEM_MODE_ID_TPUT3);
			break;
		}
		case MODEM_MODE_ID_NORMAL: {
			xiaomi_modem_cpu_set(1);
			xiaomi_modem_budget_reload(2*128);
			xiaomi_modem_ddr_set(0);
			rx_poll_th = 100;
			pr_info("%s:  before is_tput_mode=%d, flag=%d", __func__, is_tput_mode, MODEM_MODE_ID_NORMAL);
			if (is_tput_mode) {
				t800_send_set_CPU_freq_uevent(2, 0, 0, 0);
				is_tput_mode = false;
			}
			pr_info("%s:  after is_tput_mode=%d, flag=%d", __func__, is_tput_mode, MODEM_MODE_ID_NORMAL);
			break;
		}
		case MODEM_MODE_ID_LP: {
			xiaomi_modem_cpu_set(0);
			xiaomi_modem_budget_reload(1*128);
			xiaomi_modem_ddr_set(0);
			rx_poll_th = 400;
			pr_info("%s:  before is_tput_mode=%d, flag=%d", __func__, is_tput_mode, MODEM_MODE_ID_LP);
			if (is_tput_mode) {
				t800_send_set_CPU_freq_uevent(2, 0, 0, 0);
				is_tput_mode = false;
			}
			pr_info("%s:  after is_tput_mode=%d, flag=%d", __func__, is_tput_mode, MODEM_MODE_ID_LP);
			break;
		}
		default:
			break;
		}
		break;
	}
	case MODEM_PARA_ID_DUMP:
		{
			extern bool g_t800_recovery_nodump_flag;
			if (para[1] == (char)1) {
				g_t800_recovery_nodump_flag = true;
			} else {
				g_t800_recovery_nodump_flag = false;
			}
		}
		break;
	case MODEM_PARA_ID_DEBUG_LEVEL:
		g_xiaomi_debug_level = (int)para[1];
		break;
	case MODEM_PARA_ID_POWER_REQ:
		/*MTK_UEVENT_THINMD_CMD == 6*/
		t800_send_uevent(6, (int)para[1], 0, 0, 0);
		break;
	case MODEM_PARA_ID_PCIE_GEN_LEVEL:
		if (para[1]) {
			extern int xring_pcie_set_speed(struct pci_dev *dev, int speed);
			if (g_t800_mdev) {
				xring_pcie_set_speed(to_pci_dev(g_t800_mdev->dev), (int)para[1]);
				pr_info("%s:  set pcie gen%d", __func__, para[1]);
			}
		}
		break;
	case MODEM_PARA_ID_PCIE_RPM:
		if (para[1]) {
			extern void pm_runtime_allow(struct device *dev);
			extern void xring_pcie_pm_runtime_allow(int port, bool en);
			if (g_t800_mdev) {
				pm_runtime_allow(g_t800_mdev->dev);
				xring_pcie_pm_runtime_allow(0, true);
				pr_info("%s:  MODEM_PARA_ID_PCIE_RPM: %d", __func__, para[1]);
			}
		} else {
			extern void pm_runtime_forbid(struct device *dev);
			extern void xring_pcie_pm_runtime_allow(int port, bool en);
			if (g_t800_mdev) {
				xring_pcie_pm_runtime_allow(0, false);
				pm_runtime_forbid(g_t800_mdev->dev);
				pr_info("%s:  MODEM_PARA_ID_PCIE_RPM: %d", __func__, para[1]);
			}
		}
		break;
       case MODEM_PARA_ID_SOCKET: {
		short sport, dport;
		int saddress, daddress;
		char *p = para;
		saddress = *((int*)(p+1));
		sport = *((short*)(p+5));
		daddress = *((int*)(p+9));
		dport = *((short*)(p+13));
		extern int xiaomi_modem_get_socket_tcpv4(__be32 saddr, __be16 sport, __be32 daddr, __be16 dport, struct tcp_info* info);
		xiaomi_modem_get_socket_tcpv4(htonl(saddress), htons(sport), htonl(daddress), htons(dport), &tcp_info);
		break;
	}
	case MODEM_PARA_ID_SET: {
		read_id = (int)para[1];
		pr_info("%s:  MODEM_PARA_ID_SET: %d", __func__, para[1]);
		break;
	}
	default:
	break;
	}

    return count;
}

#define TCP_INFOR_LEN_MAX 100
static ssize_t xiaomi_modem_drv_para_get(struct file *file, char __user *buf,
				    size_t size, loff_t *ppos)
{
	size_t  readlen = 0;
	int ret;
	pr_info("%s  read_id= %x\n", __func__, read_id);

	switch (read_id) {
	case MODEM_PARA_ID_SOCKET: {
		char info[TCP_INFOR_LEN_MAX] = {0};
		readlen = 4 * 4;//
		memcpy(info, &readlen, sizeof(readlen));
		memcpy(info + 4, &tcp_info.tcpi_bytes_sent, sizeof(tcp_info.tcpi_bytes_sent));
		memcpy(info + 8, &tcp_info.tcpi_snd_wnd, sizeof(tcp_info.tcpi_snd_wnd));
		memcpy(info + 12, &tcp_info.tcpi_rcv_wnd, sizeof(tcp_info.tcpi_rcv_wnd));
		//memcpy(info + 20, &tcp_info.tcpi_snd_nxt, sizeof(tcp_info.tcpi_snd_nxt));
		//memcpy(info + 16, &tcp_info.tcpi_rcv_nxt, sizeof(tcp_info.tcpi_rcv_nxt));
		ret = copy_to_user(buf, info, readlen + 4);
		if (ret) {
			pr_info("%s:  MODEM_PARA_ID_SET: %d", __func__, ret);
			return 0;
		}
		break;
	}
	default:
		break;
	}

	return readlen;
}
/***********************************xiaomi modem driver optimize API end***/

/***********************************xiaomi modem recovery start***/
//MTK_UEVENT_MODEM_MONITOR
enum modem_monitor_subid_type {
	MODEM_MONITOR_START = 0,
	MODEM_MONITOR_RECOVERY_FAIL,//discard
	MODEM_MONITOR_TPUT_SPEED,
	MODEM_MONITOR_CRASH,
	MODEM_MONITOR_REBOOT,
	MODEM_MONITOR_PCIE_ERR,
	MODEM_MONITOR_FATAL,
	MODEM_MONITOR_FAULTB = 7,
	MODEM_MONITOR_865CREASON = 8
};

//MTK_UEVENT_RECOVERY
enum modem_recovery_subid_type {
	MODEM_RECOVERY_START = 0,
	MODEM_RECOVERY_DPMAIF_FAIL,
	MODEM_RECOVERY_TX_MEMORY,
	MODEM_RECOVERY_NODUMP_CRASH,
	MODEM_RECOVERY_LINKERR,
	MODEM_RECOVERY_LINKERR2,
	MODEM_RECOVERY_PROBE_FAIL
};
bool g_t800_recovery_ongoing_flag;
bool g_t800_recovery_nodump_flag;
#include "mtk_common.h"
/***********************************************************************
* t800_send_uevent() - send  uevent to thinmd
 * @id: mtk_uevent_id
    @sub_id:
    value1
***********************************************************************/
void t800_send_uevent(int id, int sub_id, int value1, int value2, int value3)
{
	char uevent_info[MTK_UEVENT_INFO_LEN];
	snprintf(uevent_info, MTK_UEVENT_INFO_LEN,
		"sub_id=0x%x, value1=0x%x, value2=0x%x, value3=0x%x", sub_id, value1, value2, value3);
	if (g_t800_mdev) {
		mtk_uevent_notify(g_t800_mdev->dev, id, uevent_info);
	}
}
/***********************************************************************
* t800_send_recovery_uevent() - send recovery uevent
 * @id: reason for recovery
	1: dtmaif tx init err
***********************************************************************/
void t800_send_recovery_uevent(int value1, int value2)
{
	char uevent_info[MTK_UEVENT_INFO_LEN];
	snprintf(uevent_info, MTK_UEVENT_INFO_LEN,
		"sub_id=0x%x, value1=0x%x, value2=0x%x, value3=0x%x", value1, value1, value2, 0);
	if (g_t800_mdev) {
		mtk_uevent_notify(g_t800_mdev->dev, MTK_UEVENT_RECOVERY, uevent_info);
	}
}
void t800_recovery_start(int value1, int value2)
{
	pr_err("%s:  value1=%d, value2=%d, flag=%d", __func__, value1, value2, g_t800_recovery_ongoing_flag);
	if (g_t800_recovery_ongoing_flag) {//only retry one time
		pr_err("%s: recovery ignored", __func__);
		return;
	}
	g_t800_recovery_ongoing_flag = true;
	t800_send_recovery_uevent(value1, value2);
}
void t800_recovery_complete(void)
{
	pr_err("%s: , flag=%d", __func__, g_t800_recovery_ongoing_flag);
	g_t800_recovery_ongoing_flag = false;
}
/***********************************xiaomi modem recovery end***/
/***********************************xiaomi modem monitor start***/
//send from fsm thread

void t800_send_monitor_uevent(int id, int value1, int value2, int value3)
{
	char uevent_info[MTK_UEVENT_INFO_LEN];
	snprintf(uevent_info, MTK_UEVENT_INFO_LEN,
		 "sub_id=0x%x, value1=0x%x, value2=0x%x, value3=0x%x", id, value1, value2, value3);
	if (g_t800_mdev) {
		mtk_uevent_notify(g_t800_mdev->dev, MTK_UEVENT_MODEM_MONITOR, uevent_info);
	}
}
static int event_data[4];
void t800_send_monitor_to_fsm(int id, int value1, int value2, int value3)
{
	event_data[0] = id;
	event_data[1] = value1;
	event_data[2] = value2;
	event_data[3] = value3;
	if (g_t800_mdev) {
		mtk_fsm_evt_submit(g_t800_mdev, FSM_EVT_XIAOMI_MODEM, FSM_F_DFLT, (void *)event_data, sizeof(event_data), 0);
	}
}
__u64 g_dl_total;
__u64 g_ul_total;
void xiaomi_dltput_calc(int ms)
{
	int speed = (int)(g_dl_total*1000*8/(ms*0x100000));
	int speed_ul = (int)(g_ul_total*1000*8/(ms*0x100000));
	static int level = -1;
	int tmp = 0;

	if (speed > 0x800) {
		tmp = 3;
	} else if (speed > 0x400) {
		tmp = 2;
	} else if (speed > 0x1) {
		tmp = 1;
	} else {
		tmp = 0;
	}

	if (g_xiaomi_debug_level == 2) {
		pr_info("%s  level=%d,  speed=%d, speed_ul=%d, dlcount=%llu, ulcount=%llu, time=%d", __func__,  level, speed, speed_ul, g_dl_total, g_ul_total, ms);
	}
	if (level != tmp) {
		level = tmp;
		t800_send_monitor_to_fsm(2, speed, speed_ul, 0);
		pr_info("%s report  level=%d,  speed=%d, dlcount=%llu, time=%d", __func__,  level, speed, g_dl_total, ms);
	}
	g_dl_total = 0;
	g_ul_total = 0;

}
/***********************************xiaomi modem monitor end***/
/***********************************xiaomi modem set CPU freq start***/
void t800_send_set_CPU_freq_uevent(int id, int value1, int value2, int value3)
{
	char uevent_info[MTK_UEVENT_INFO_LEN];
	snprintf(uevent_info, MTK_UEVENT_INFO_LEN,
		 "sub_id=0x%x, value1=0x%x, value2=0x%x, value3=0x%x", id, value1, value2, value3);
	pr_info("%s:  sub_id=%d", __func__, id);
	if (g_t800_mdev) {
		mtk_uevent_notify(g_t800_mdev->dev, MTK_UEVENT_CPU_FREQ, uevent_info);
	}
}
/***********************************xiaomi modem set CPU freq end***/
/* mtk_data_init() - initialize data path
 * @mdev: pointer to mtk_md_dev
 * Allocate and initialize all software resource of data transction layer and data port layer.
 * Return: return value is 0 on success, a negative error code on failure.
 */
int mtk_data_init(struct mtk_md_dev *mdev)
{
	struct mtk_data_blk *data_blk;
	int ret;

	if (!mdev) {
		pr_err("Invalid parameter, mdev is NULL\n");
		return -ENODEV;
	}

	mtk_dpmaif_param_check();

	data_blk = devm_kzalloc(mdev->dev, sizeof(*data_blk), GFP_KERNEL);
	if (!data_blk) {
		MTK_ERR(mdev, "Failed to allocate data_blk\n");
		return -ENOMEM;
	}
	data_blk->mdev = mdev;
	mdev->data_blk = data_blk;

	ret = mtk_dpmaif_sw_init(data_blk);
	if (ret < 0)
		goto out;
/***********************************xiaomi modem driver optimize API start***/
	mtk_pwrctl_set_para_cb(modem_drv_para_set, xiaomi_modem_drv_para_get);
	return 0;
/***********************************xiaomi modem driver optimize API end***/
out:
	devm_kfree(mdev->dev, data_blk);
	mdev->data_blk = NULL;

	return ret;
}

/* mtk_data_exit() - deinitialize data path
 * @mdev: pointer to mtk_md_dev
 * deinitialize and release all software resource of data transction layer and data port layer.
 * Return: return value is 0 on success, a negative error code on failure.
 */
int mtk_data_exit(struct mtk_md_dev *mdev)
{
	int ret;

	if (!mdev || !mdev->data_blk) {
		pr_err("Invalid parameter\n");
		return -EINVAL;
	}
/***********************************xiaomi modem driver optimize API start***/
	mtk_pwrctl_remove_para_cb(modem_drv_para_set, xiaomi_modem_drv_para_get);
/***********************************xiaomi modem driver optimize API end***/
	ret = mtk_dpmaif_sw_exit(mdev->data_blk);

	devm_kfree(mdev->dev, mdev->data_blk);
	mdev->data_blk = NULL;
	return ret;
}

module_param(dpmaif_mtu, uint, 0444);
MODULE_PARM_DESC(dpmaif_mtu,
		 "This is used to specify the data trans mtu, default 3000\n");
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
module_param(normal_prio_tth, uint, 0444);
MODULE_PARM_DESC(normal_prio_tth,
		 "This timer is used to merge packets on normal priority queue, default 10(ms), max 10(ms)\n");
module_param(high_prio_tth, uint, 0444);
MODULE_PARM_DESC(high_prio_tth,
		 "This timer is used to merge packets on normal priority queue, default 2(ms), max 10(ms)\n");
module_param(normal_prio_pth, uint, 0444);
MODULE_PARM_DESC(normal_prio_pth,
		 "This threshold is used to force trigger tx and restart timer, default 128(<= 64 packets), max 1666(<= 833 packets)\n");
module_param(high_prio_pth, uint, 0444);
MODULE_PARM_DESC(high_prio_pth,
		 "This threshold is used to force trigger tx and restart timer, default 32(<= 16 packets), max 1666(<= 833 packets)\n");
module_param(rx_poll_th, uint, 0444);
MODULE_PARM_DESC(rx_poll_th,
		 "This threshold(PIT speed) is used to enable polling register at rx done interrupt bottom-half, default 400(PITs/ms), max 1024(PITs/ms)");
module_param(tx_poll_th, uint, 0444);
MODULE_PARM_DESC(tx_poll_th,
		 "This threshold(DRB speed) is used to enable polling register at tx done interrupt bottom-half, default 200(DRBs/ms), max 512(DRBs/ms)");
module_param(max_pit_burst_cnt, uint, 0444);
MODULE_PARM_DESC(max_pit_burst_cnt,
		 "This max PIT burst release count, default 1024, max 2048");
module_param(traffic_stats_shift, uint, 0444);
MODULE_PARM_DESC(traffic_stats_shift,
		 "This is timer for traffic statistic, 1ms shift, default 5 (32ms), min 3 (8ms), max 9 (512ms)");
module_param(doorbell_reset_count, uint, 0644);
MODULE_PARM_DESC(doorbell_reset_count,
		 "The doorbell delay reset after specified doorbell count, used in TCP slow start stage, default 500");
#endif
