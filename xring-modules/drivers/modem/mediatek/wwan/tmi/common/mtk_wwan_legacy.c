// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#define pr_fmt(fmt) "MTK_WWAN: " fmt

#include <linux/version.h>
#include <linux/completion.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/printk.h>
#include <linux/skbuff.h>
#include <linux/skmsg.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/rcupdate.h>
#include <linux/tcp.h>
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
#include <linux/version.h>
#endif
#include <net/pkt_sched.h>
#include <net/sch_generic.h>
#include <uapi/linux/if.h>
#include <uapi/linux/if_arp.h>
#include <uapi/linux/if_link.h>
#include <linux/ip.h>
#include <net/ipv6.h>
#include "mtk_data_plane.h"
#include "mtk_dev.h"
#include "mtk_debug.h"

#ifdef CONFIG_TX00_UT_WWAN_LEGACY
#include "ut_wwan_legacy_fake.h"
#endif

/* xiaomi resize the number of netdev to 6 to reduce cost of schedule */
#define MTK_NETDEV_MAX		6
#define MTK_DFLT_MTU		1500
#define MTK_NETDEV_WDT (HZ)
#define MTK_CMD_WDT (HZ)
#define MTK_MAX_INTF_ID (MTK_NETDEV_MAX - 1)
#define MTK_NAPI_POLL_WEIGHT	128

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
#define MTK_TSQ_DFLT_SHIFT	7
#define MTK_TSQ_MAX_SHIFT	10

static unsigned int mtk_tsq_shift = MTK_TSQ_DFLT_SHIFT;
#endif

#define TAG "WWAN"

enum mtk_wwan_sioc {
	MTK_WWAN_SIOC_SET_PKTPRIO = SIOCDEVPRIVATE + 14,
	MTK_WWAN_SIOC_SET_NETTYPE = SIOCDEVPRIVATE + 15
};

static unsigned int napi_budget = MTK_NAPI_POLL_WEIGHT;
/* module parameter */
static unsigned int nic_intf_num = MTK_NETDEV_MAX;

/* struct mtk_wwan_instance - This is netdevice's private data,
 * contains information about netdevice.
 * @wcb: Contains all information about WWAN port layer.
 * @txq_stats: Statistics of netdevice's tx packets by qdisc.
 * @rxq_stats: Statistics of netdevice's rx packets by qdisc.
 * @netdev: Pointer to netdevice structure.
 * @index: The netdevice's id, such as wwan0,wwan1,...
 * @vlan: Save VLAN interface information at MBIM mode.
 * @fake_src_addr: Save a random mac address to fill in eth_header for rx_packet.
 * @vlan_res_lock: Used for add and kill VLAN id.
 */
struct mtk_wwan_instance {
	struct mtk_wwan_ctlb *wcb;
	struct rtnl_link_stats64 stats;
	unsigned long long tx_busy;
	unsigned long long tx_timeout;
	struct net_device *netdev;
	unsigned int intf_id;
	unsigned short network_type;
	unsigned int pkt_prio;
#ifdef CONFIG_MTK_ENABLE_WQ_GRO
	struct napi_struct *wq_napis;
#endif
};

/* struct mtk_wwan_ctlb - Contains WWAN port layer information and save trans information needed.
 * @data_blk: Contains data port, trans layer, md_dev structure.
 * @mdev: Pointer of mtk_md_dev.
 * @trans_ops: Contains trans layer ops: send, select_txq, napi_poll.
 * @wwan_inst: wwan instance, max is 20
 * @napis: Trans layer alloc napi structure by rx queue.
 * @dummy_dev: Used for multiple network devices share one napi.
 * @cap: Contains different hardware capabilities.
 * @napi_enabled: 1 enable, 0 disable
 * @max_mtu: The max MTU supported.
 * @active_cnt: The counter of network devices that are UP.
 * @wwan_inst_num: The number of network device.
 * @txq_num: Total TX qdisc number.
 * @rxq_num: Total RX qdisc number.
 * @reg_done: Mark for network devices register state.
 */
struct mtk_wwan_ctlb {
	struct mtk_data_blk *data_blk;
	struct mtk_md_dev *mdev;
	struct mtk_data_trans_ops *trans_ops;
	struct mtk_wwan_instance *wwan_inst[MTK_NETDEV_MAX];
	struct napi_struct **napis;
	struct net_device dummy_dev;

	u32 cap;
	atomic_t napi_enabled;
	unsigned int max_mtu;
	unsigned int active_cnt;
	unsigned short wwan_inst_num;
	unsigned char txq_num;
	unsigned char rxq_num;
	bool reg_done;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	struct mtk_data_cpu_affinity_cfg  *aff_cfg;
#endif
};

static void mtk_wwan_set_skb(struct sk_buff *skb, struct net_device *netdev)
{
	unsigned int pkt_type;

	pkt_type = skb->data[0] & 0xF0;

	if (pkt_type == IPV4_VERSION)
		skb->protocol = htons(ETH_P_IP);
	else
		skb->protocol = htons(ETH_P_IPV6);

	skb->dev = netdev;
}

/* mtk_wwan_data_recv - Collect data packet.
 * @data_blk: Save netdev information.
 * @q_id: RX queue id, used for select NAPI.
 * @intf_id: Interface id, determine this skb belong to which netdev.
 */
static int mtk_wwan_data_recv(struct mtk_data_blk *data_blk, struct sk_buff *skb,
			      unsigned char q_id, unsigned char intf_id)
{
	struct mtk_wwan_instance *wwan_inst;

	if (intf_id > MTK_MAX_INTF_ID) {
		MTK_WARN(data_blk->mdev, "Invalid interface id=%d\n", intf_id);
		MTK_BUG(data_blk->mdev);
		goto err_rx;
	}

	wwan_inst = data_blk->wcb->wwan_inst[intf_id];

	if (unlikely(!wwan_inst)) {
		MTK_ERR(data_blk->mdev, "Invalid pointer wwan_inst is NULL\n");
		goto err_rx;
	}

	mtk_wwan_set_skb(skb, wwan_inst->netdev);

	trace_mtk_tput_data_rx(q_id, "rx p4");

	wwan_inst->stats.rx_packets++;
	wwan_inst->stats.rx_bytes += skb->len;
/***********************************xiaomi modem tput start ***/
	{
		extern __u64 g_dl_total;
		g_dl_total += skb->len;
	}
/***********************************xiaomi modem tput end ***/
	skb_reset_mac_header(skb);
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	local_bh_disable();
#ifdef CONFIG_MTK_ENABLE_WQ_GRO
	napi_gro_receive(&wwan_inst->wq_napis[q_id], skb);
#else
	netif_receive_skb(skb);
#endif
	local_bh_enable();
#else
	napi_gro_receive(data_blk->wcb->napis[q_id], skb);
#endif
	trace_mtk_tput_data_rx(q_id, "rx p5");

	return 0;

err_rx:
	dev_kfree_skb_any(skb);
	return -EINVAL;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
static u8 mtk_wwan_get_napi_thread_affinity(struct mtk_wwan_ctlb *wcb,
					    unsigned char napi_id)
{
	return wcb->aff_cfg->napi_thrd_aff[napi_id];
}

static void mtk_wwan_set_napi_thread_affinity(struct mtk_wwan_ctlb *wcb,
					      unsigned char napi_id)
{
	cpumask_var_t mask;
	u32 napi_thread_affinity;
	struct napi_struct *napi = wcb->napis[napi_id];

	if (!wcb->aff_cfg || !napi->thread) {
		MTK_WARN(wcb->mdev, "wcb->aff_cfg = 0x%llx, napi->thread = 0x%llx\n",
			 (u64)wcb->aff_cfg, (u64)napi->thread);
		return;
	}

	if (!alloc_cpumask_var(&mask, GFP_KERNEL)) {
		MTK_WARN(wcb->mdev, "Failed to alloc cpumask var.\n");
		return;
	}

	napi_thread_affinity = mtk_wwan_get_napi_thread_affinity(wcb, napi_id);
       MTK_INFO(wcb->mdev, "napi work on cpu %d\n", napi_thread_affinity);
	cpumask_clear(mask);
	cpumask_set_cpu(napi_thread_affinity, mask);
	set_cpus_allowed_ptr(napi->thread, mask);

	free_cpumask_var(mask);
}
#endif
#endif

static void mtk_wwan_napi_enable(struct mtk_wwan_ctlb *wcb)
{
	int i;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	int ret;
#endif
#endif

	if (atomic_cmpxchg(&wcb->napi_enabled, 0, 1) == 0) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
		ret = dev_set_threaded(&wcb->dummy_dev, true);
		MTK_INFO(wcb->mdev, "dev_set_threaded return = %d\n", ret);
#endif
#endif
		for (i = 0; i < wcb->rxq_num; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
			if (!ret)
				mtk_wwan_set_napi_thread_affinity(wcb, i);
#endif
#endif
			napi_enable(wcb->napis[i]);
		}
	}
}

static void mtk_wwan_napi_disable(struct mtk_wwan_ctlb *wcb)
{
	int i;

	if (atomic_cmpxchg(&wcb->napi_enabled, 1, 0) == 1) {
		for (i = 0; i < wcb->rxq_num; i++) {
			napi_synchronize(wcb->napis[i]);
			napi_disable(wcb->napis[i]);
		}
	}
}


/***********************************xiaomi modem driver optimize API start***/
extern int g_xiaomi_debug_level;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
#include <soc/xring/ddr_vote_dev.h>
#define DDR_DVFS_HW_VOTE_MODEM 10
extern void xiaomi_modem_cpu_cfg_set(int level);
extern void xiaomi_modem_rx_reload(struct mtk_dpmaif_ctlb *dcb);
#define NET_NUM_MAX  (2)
static struct net_device *xiaomi_modem_net_st[NET_NUM_MAX] = {NULL};

static void xiaomi_napi_affinity(struct mtk_wwan_ctlb *wcb)
{
	int i;
	for (i = 0; i < wcb->rxq_num; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
		mtk_wwan_set_napi_thread_affinity(wcb, i);
#endif
#endif
		}
}

static void xiaomi_cpu_change(struct net_device *dev)
{
	struct mtk_wwan_instance *wwan_inst =	(struct mtk_wwan_instance *)netdev_priv(dev);
	struct mtk_wwan_ctlb *wcb = wwan_inst->wcb;
	if (wcb && wcb->active_cnt) {
		xiaomi_napi_affinity(wcb);
		if (wcb->data_blk && wcb->data_blk->dcb)
		{
			xiaomi_modem_rx_reload(wcb->data_blk->dcb);
		}
	} else {
		if (wcb) {
			pr_err("%s  failed.  %d", __func__, wcb->active_cnt);
		}
	}
}

void xiaomi_modem_cpu_set(int level)
{
	xiaomi_modem_cpu_cfg_set(level);
	for (int i = 0; i < NET_NUM_MAX; i++) {
		if (xiaomi_modem_net_st[i] != NULL) {
			xiaomi_cpu_change(xiaomi_modem_net_st[i]);
		} else {
			pr_err("%s  failed.  xiaomi_modem_net_st[%d] is null", __func__, i);
		}
	}
}

void xiaomi_modem_ddr_set(unsigned int freq)
{
	int ret = ddr_vote_dev_request(DDR_DVFS_HW_VOTE_MODEM,  VOTE_MIN_DATA_RATE_MBPS, freq);
	if (ret != 0) {
		pr_err("xiaomi_modem_ddr_set ddr  DVFS fail(err %d)", ret);
	}
}

void xiaomi_modem_budget_reload(int budget)
{
	int i, j;
	struct mtk_wwan_ctlb *wcb;
	napi_budget = budget;
	for (j = 0; j < NET_NUM_MAX; j++) {
		if (xiaomi_modem_net_st[j] == NULL) {
			continue;
		}
		struct mtk_wwan_instance *wwan_inst = (struct mtk_wwan_instance *)netdev_priv(xiaomi_modem_net_st[j]);
		if (!wwan_inst || !(wwan_inst->wcb)) {
			continue;
		}
		wcb = wwan_inst->wcb;
		for (i = 0; i < wcb->rxq_num; i++) {
			if (!wcb->napis[i]) {
				MTK_ERR(wcb->mdev, "Invalid parameter, napi%d is NULL!", i);
				continue;
			}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
			wwan_inst->wq_napis[i].weight = napi_budget;
#endif

		}
	}
}
#endif

/***********************************xiaomi modem driver optimize API end***/
/***********************************xiaomi modem tcp infor***/
int xiaomi_modem_get_socket_tcpv4(__be32 saddr, __be16 sport, __be32 daddr, __be16 dport, struct tcp_info* info)
{
	struct sock *sk;
	int result = -1;
	struct net_device *dev;
	struct net *net;
	memset(info, 0, sizeof(struct tcp_info));
	pr_info("%s  saddr= %u  sport=%d  daddr= %u  dport=%d  \n", __func__, saddr, sport, daddr, dport );
	rcu_read_lock();
	//extern struct inet_hashinfo tcp_hashinfo; //this global hashinfo also can work
	for_each_netdev(&init_net, dev) { //this loop search also work
		if(dev != NULL) {
			pr_info("%s  dev name= %s , index=%d\n", __func__, dev->name, dev->ifindex);
			net = dev_net(dev);
			if(net != NULL) {
				sk = inet_lookup_established(net, net->ipv4.tcp_death_row.hashinfo, daddr, dport, saddr, sport,  dev->ifindex);
				pr_info("%s 2 sk= %p \n", __func__, (void*)sk);
				if (sk) {
					tcp_get_info(sk, info);
					pr_info("%s  sent bytes= %llu \n", __func__, info->tcpi_bytes_sent);
					result = 0;
					break;
				}
				#if 0
				sk = inet_lookup_established(net, &tcp_hashinfo, daddr, dport, saddr, sport,  dev->ifindex);
				pr_info("%s 4 sk= %p \n", __func__, (void*)sk);
				if (sk) {
					tcp_get_info(sk, info);
					pr_info("%s  sent bytes= %llu \n", __func__, info->tcpi_bytes_sent);
					result = 0;
					//break;
				}
				#endif
			}
		}
	}
	rcu_read_unlock();
	return result;
}
/***********************************xiaomi modem tcp infor***/
static void mtk_wwan_napi_sync(struct mtk_wwan_ctlb *wcb)
{
	int i;

	if (atomic_read(&wcb->napi_enabled)) {
		for (i = 0; i < wcb->rxq_num; i++)
			napi_synchronize(wcb->napis[i]);
	}
}

static int mtk_wwan_open(struct net_device *dev)
{
	struct mtk_wwan_instance *wwan_inst =
		(struct mtk_wwan_instance *)netdev_priv(dev);
	struct mtk_wwan_ctlb *wcb = wwan_inst->wcb;
	struct mtk_data_trans_ctl trans_ctl;
	int ret;

	MTK_INFO(wcb->mdev, "Opening netdev=%s, active_cnt=%u\n",
		 dev->name, wcb->active_cnt);

	if (!wcb->active_cnt) {
		mtk_wwan_napi_enable(wcb);

		/* If enable trans failed, means this link not ready. */
		trans_ctl.enable = true;
		ret = mtk_wwan_cmd_execute(dev, DATA_CMD_TRANS_CTL, &trans_ctl);
		if (ret < 0) {
			MTK_ERR(wcb->mdev, "Failed to enable trans\n");
			goto err_ctl;
		}
	}

	/* ndo_open hold rtnl_lock() */
	wcb->active_cnt++;
/***********************************xiaomi modem driver optimize API start***/
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	{
		for (int j = 0; j < NET_NUM_MAX; j++) {
			if (xiaomi_modem_net_st[j] != NULL) {
				MTK_INFO(wcb->mdev, "xiaomi_modem_net_st[%d]->name =%s\n",
					j, xiaomi_modem_net_st[j]->name);
				continue;
			}
			xiaomi_modem_net_st[j] = dev;
		}
	}
#endif
/***********************************xiaomi modem driver optimize API end***/
	netif_tx_start_all_queues(dev);
	netif_carrier_on(dev);

	return 0;

err_ctl:
	mtk_wwan_napi_disable(wcb);
	return ret;
}

static int mtk_wwan_stop(struct net_device *dev)
{
	struct mtk_wwan_instance *wwan_inst =
		(struct mtk_wwan_instance *)netdev_priv(dev);
	struct mtk_wwan_ctlb *wcb = wwan_inst->wcb;
	struct mtk_data_trans_ctl trans_ctl;
	int ret;

	MTK_INFO(wcb->mdev, "Stopping netdev=%s, active_cnt=%u\n",
		 dev->name, wcb->active_cnt);

	netif_carrier_off(dev);
	netif_tx_disable(dev);

	if (wcb->active_cnt == 1) {
		trans_ctl.enable = false;
		ret = mtk_wwan_cmd_execute(dev, DATA_CMD_TRANS_CTL, &trans_ctl);
		if (ret < 0)
			MTK_ERR(wcb->mdev, "Failed to disable trans\n");

		mtk_wwan_napi_disable(wcb);
	}
	wcb->active_cnt--;
/***********************************xiaomi modem driver optimize API start***/
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	{
		//wait all net device stop
		if (wcb->active_cnt == 0) {
			for (int j = 0; j < NET_NUM_MAX; j++) {
				MTK_INFO(wcb->mdev, "xiaomi_modem_net_st[%d]->name =%s\n",
					j, xiaomi_modem_net_st[j]->name);
				xiaomi_modem_net_st[j] = NULL;
			}
		}
	}
#endif
/***********************************xiaomi modem driver optimize API end***/
	return 0;
}

#ifndef CONFIG_DATA_TEST_MODE
static int mtk_wwan_check_data_offset(struct sk_buff *skb)
{
	int data_offset = 0;
	struct iphdr *ip_v4;
	u32 pkt_type;

	pkt_type = skb->data[0] & 0xF0;
	if (pkt_type == IPV4_VERSION) {
		ip_v4 = (struct iphdr *)(skb->data);
		if (ip_v4->protocol == IPPROTO_UDP)
			data_offset = (ip_v4->ihl << 2) + 8;
	}

	return data_offset;
}
#endif

static netdev_tx_t mtk_wwan_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);
	union mtk_data_pkt_info pkt_info;
	unsigned int skb_len = skb->len;
#ifndef CONFIG_DATA_TEST_MODE
	int data_offset;
#endif
	u64 data;
	int ret;

	trace_mtk_tput_data_tx("tx p2");

	pkt_info.tx.intf_id = wwan_inst->intf_id;
	pkt_info.tx.network_type = wwan_inst->network_type;
	data = (u64)&pkt_info;

	if (unlikely(skb->len > dev->mtu)) {
		MTK_ERR(wwan_inst->wcb->mdev,
			"Failed to write skb,netdev=%s,len=0x%x,MTU=0x%x\n",
			dev->name, skb->len, dev->mtu);
		goto err_tx;
	}

#ifndef CONFIG_DATA_TEST_MODE
	if (pkt_info.tx.network_type) {
		data_offset = mtk_wwan_check_data_offset(skb);
		if (data_offset)
			skb_pull(skb, data_offset);
	}
#endif

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	pkt_info.tx.in_tcp_slow_start = false;
	/* sk_type and sk_protocol are only accessible when sk_fullsock is true */
	if (skb->sk && sk_fullsock(skb->sk)) {
#if (KERNEL_VERSION(5, 16, 0) <= LINUX_VERSION_CODE)
		if (sk_is_tcp(skb->sk)) {
#else
		if (skb->sk->sk_type == SOCK_STREAM && skb->sk->sk_protocol == IPPROTO_TCP) {
#endif
			if (mtk_tsq_shift <= MTK_TSQ_MAX_SHIFT)
				sk_pacing_shift_update(skb->sk, mtk_tsq_shift);
			pkt_info.tx.in_tcp_slow_start = tcp_in_slow_start(tcp_sk(skb->sk));
		}
	}
#endif
	/* Forward skb to trans layer(DPMAIF). */
	ret = wwan_inst->wcb->trans_ops->send(wwan_inst->wcb->data_blk,
											DATA_PKT,
											skb,
											data);

	if (ret == -EBUSY) {
		wwan_inst->tx_busy++;
		return NETDEV_TX_BUSY;
	} else if (ret == -EINVAL) {
		goto err_tx;
	}

	wwan_inst->stats.tx_packets++;
	wwan_inst->stats.tx_bytes += skb_len;
	goto out;

err_tx:
	wwan_inst->stats.tx_errors++;
	wwan_inst->stats.tx_dropped++;
	dev_kfree_skb_any(skb);
out:
	return NETDEV_TX_OK;
}

static void mtk_wwan_get_stats(struct net_device *dev,
			       struct rtnl_link_stats64 *stats)
{
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);

	memcpy(stats, &wwan_inst->stats, sizeof(*stats));
}

inline u16 mtk_wwan_select_queue(struct net_device *dev, struct sk_buff *skb,
			  struct net_device *sb_dev)
{
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);
	u16 qid;

	trace_mtk_tput_data_tx("tx p1");


	qid = wwan_inst->wcb->trans_ops->select_txq(wwan_inst->wcb->data_blk, skb, PURE_IP,
						    wwan_inst->pkt_prio);
	return qid;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static void mtk_wwan_tx_timeout(struct net_device *dev, unsigned int txq)
{
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);

	wwan_inst->tx_timeout++;

	MTK_WARN(wwan_inst->wcb->mdev,
		 "wwan%d tx_timeout, txq=%u, active_cnt=%u\n",
		wwan_inst->intf_id, txq, wwan_inst->wcb->active_cnt);
}
#else
static void mtk_wwan_tx_timeout(struct net_device *dev)
{
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);

	MTK_WARN(wwan_inst->wcb->mdev,
		 "wwan%d tx_timeout, active_cnt=%u\n",
		wwan_inst->intf_id, wwan_inst->wcb->active_cnt);
}
#endif

static netdev_features_t mtk_wwan_fix_feature(struct net_device *dev,
					      netdev_features_t features)
{
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);

	if (!(features & NETIF_F_RXCSUM)) {
		MTK_INFO(wwan_inst->wcb->mdev, "disabling LRO as RXCSUM is off\n");
		features &= ~NETIF_F_LRO;
	}
	return features;
}

static int mtk_wwan_set_feature(struct net_device *dev,
				netdev_features_t features)
{
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);
	bool dpmaif_txcsum_enable = !!(dev->features & NETIF_F_HW_CSUM);
	bool dpmaif_rxcsum_enable = !!(dev->features & NETIF_F_RXCSUM);
	bool dpmaif_lro_enable = !!(dev->features & NETIF_F_LRO);
	netdev_features_t diff = dev->features ^ features;
	netdev_features_t rxcsum_flag;
	netdev_features_t txcsum_flag;
	netdev_features_t lro_flag;
	int ret = 0;

	lro_flag = diff & NETIF_F_LRO;
	rxcsum_flag = diff & NETIF_F_RXCSUM;
	txcsum_flag = diff & NETIF_F_HW_CSUM;

	if (!lro_flag && !rxcsum_flag && !txcsum_flag)
		return ret;

	if (lro_flag) {
		if (features & NETIF_F_LRO)
			dpmaif_lro_enable = true;
		else
			dpmaif_lro_enable = false;

		ret = mtk_wwan_cmd_execute(dev, DATA_CMD_LRO_SET, &dpmaif_lro_enable);
	}

	if (rxcsum_flag) {
		if (features & NETIF_F_RXCSUM)
			dpmaif_rxcsum_enable = true;
		else
			dpmaif_rxcsum_enable = false;

		ret = mtk_wwan_cmd_execute(dev, DATA_CMD_RXCSUM_SET, &dpmaif_rxcsum_enable);
	}

	if (txcsum_flag) {
		if (features & NETIF_F_HW_CSUM)
			dpmaif_txcsum_enable = true;
		else
			dpmaif_txcsum_enable = false;

		ret = mtk_wwan_cmd_execute(dev, DATA_CMD_TXCSUM_SET, &dpmaif_txcsum_enable);
	}

		MTK_DBG(wwan_inst->wcb->mdev, MTK_DBG_WWAN, MTK_MEMLOG_RG_0,
			"dev->features = %pNF,features = %pNF,lro_enable = %d,rxcsum_enable = %d,txcsum_enable = %d,ret = %d\n",
			&dev->features, &features, dpmaif_lro_enable,
			dpmaif_rxcsum_enable, dpmaif_txcsum_enable, ret);

	return ret;
}

static int mtk_wwan_ioctl_handle(struct mtk_wwan_instance *wwan_inst, void __user *data, int cmd)
{
#ifndef CONFIG_DATA_TEST_MODE
	unsigned short network_type;
#endif
	unsigned char pkt_prio;
	int ret = 0;

	switch (cmd) {
#ifndef CONFIG_DATA_TEST_MODE
	case MTK_WWAN_SIOC_SET_NETTYPE:
		if (copy_from_user(&network_type, data, sizeof(network_type))) {
			ret = -EFAULT;
		} else {
			/* HW only has 3 bits */
			if (network_type > DATA_NETWORK_TYPE_MAX) {
				MTK_ERR(wwan_inst->wcb->mdev,
					"Failed to set network_type, type=%d,intf_id=%u\n",
					network_type, wwan_inst->intf_id);
				ret = -EINVAL;
			} else {
				wwan_inst->network_type = network_type;
				MTK_INFO(wwan_inst->wcb->mdev,
					 "wwan%u,network_type=%d\n",
					wwan_inst->intf_id, wwan_inst->network_type);
			}
		}
		break;
#endif
	case MTK_WWAN_SIOC_SET_PKTPRIO:
		if (copy_from_user(&pkt_prio, data, sizeof(pkt_prio))) {
			ret = -EFAULT;
		} else {
			if (pkt_prio > PKT_PRIO_INVALID) {
				MTK_ERR(wwan_inst->wcb->mdev,
					"Invalid pkt_prio, prio=%hhu,intf_id=%u\n",
					pkt_prio, wwan_inst->intf_id);
				ret = -EINVAL;
			} else {
				wwan_inst->pkt_prio = pkt_prio;
				MTK_INFO(wwan_inst->wcb->mdev,
					 "wwan%u,pkt_prio=%hhu\n",
					wwan_inst->intf_id, wwan_inst->pkt_prio);
			}
		}
		break;
	default:
		MTK_DBG(wwan_inst->wcb->mdev,
			MTK_DBG_WWAN, MTK_MEMLOG_RG_0, "Invalid cmd, cmd=%d\n", cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
static int mtk_wwan_siocdevprivate(struct net_device *dev, struct ifreq *ifr,
				   void __user *data, int cmd)
{
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);

	return mtk_wwan_ioctl_handle(wwan_inst, data, cmd);
}
#else
static int mtk_wwan_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);

	return mtk_wwan_ioctl_handle(wwan_inst, ifr->ifr_data, cmd);
}
#endif

static const struct net_device_ops mtk_netdev_ops = {
	.ndo_open		= mtk_wwan_open,
	.ndo_stop		= mtk_wwan_stop,
	.ndo_start_xmit = mtk_wwan_start_xmit,
	.ndo_tx_timeout = mtk_wwan_tx_timeout,
	.ndo_get_stats64	= mtk_wwan_get_stats,
	.ndo_select_queue	= mtk_wwan_select_queue,
	.ndo_fix_features = mtk_wwan_fix_feature,
	.ndo_set_features = mtk_wwan_set_feature,

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
	.ndo_siocdevprivate = mtk_wwan_siocdevprivate,
#else
	.ndo_do_ioctl = mtk_wwan_ioctl,
#endif
};

static void mtk_wwan_cmd_complete(void *data)
{
	struct mtk_data_cmd *event;
	struct sk_buff *skb = data;

	event = (struct mtk_data_cmd *)skb->data;

	complete(&event->done);
}

static int mtk_wwan_cmd_check(struct net_device *dev, enum mtk_data_cmd_type cmd)
{
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);
	int ret = 0;

	switch (cmd) {
	case DATA_CMD_INTR_COALESCE_GET:
		fallthrough;
	case DATA_CMD_INTR_COALESCE_SET:
		if (!(wwan_inst->wcb->cap & DATA_F_INTR_COALESCE))
			ret = -EOPNOTSUPP;
		break;

	case DATA_CMD_INDIR_SIZE_GET:
		fallthrough;
	case DATA_CMD_HKEY_SIZE_GET:
		fallthrough;
	case DATA_CMD_RXFH_GET:
		fallthrough;
	case DATA_CMD_RXFH_SET:
		if (!(wwan_inst->wcb->cap & DATA_F_RXFH))
			ret = -EOPNOTSUPP;
		break;

	case DATA_CMD_RXQ_NUM_GET:
		fallthrough;
	case DATA_CMD_TRANS_DUMP:
		fallthrough;
	case DATA_CMD_STRING_CNT_GET:
		fallthrough;
	case DATA_CMD_STRING_GET:
		break;
	case DATA_CMD_TRANS_CTL:
		break;
	case DATA_CMD_LRO_SET:
		fallthrough;
	case DATA_CMD_RXCSUM_SET:
		fallthrough;
	case DATA_CMD_TXCSUM_SET:
		break;
	default:
		ret = -EOPNOTSUPP;
		break;
	}

	return ret;
}

static struct sk_buff *mtk_wwan_cmd_alloc(enum mtk_data_cmd_type cmd, unsigned int len)

{
	struct mtk_data_cmd *event;
	struct sk_buff *skb;

	skb = dev_alloc_skb(sizeof(*event) + len);
	if (unlikely(!skb))
		return NULL;

	skb_put(skb, len + sizeof(*event));
	event = (struct mtk_data_cmd *)skb->data;
	event->cmd = cmd;
	event->len = len;

	/* Init completion for sync. */
	init_completion(&event->done);
	event->data_complete = mtk_wwan_cmd_complete;

	return skb;
}

static int mtk_wwan_cmd_send(struct net_device *dev, struct sk_buff *skb)
{
	struct mtk_data_cmd *event = (struct mtk_data_cmd *)skb->data;
	struct mtk_wwan_instance *wwan_inst = netdev_priv(dev);
	int ret;

	ret = wwan_inst->wcb->trans_ops->send(wwan_inst->wcb->data_blk, DATA_CMD, skb, 0);
	if (ret < 0)
		return ret;

	if (!wait_for_completion_timeout(&event->done, MTK_CMD_WDT)) {
		MTK_WARN(wwan_inst->wcb->mdev, "WWAN execute command=%d time out\n", event->cmd);
		return -ETIMEDOUT;
	}

	if (event->ret < 0) {
		MTK_WARN(wwan_inst->wcb->mdev,
			 "WWAN execute command=%d error=%d\n", event->cmd, event->ret);
		return event->ret;
	}

	return 0;
}

static void mtk_wwan_cmd_free(struct sk_buff *skb)
{
	if (likely(skb))
		dev_kfree_skb_any(skb);
}

int mtk_wwan_cmd_execute(struct net_device *dev,
			 enum mtk_data_cmd_type cmd, void *data)
{
	struct mtk_wwan_instance *wwan_inst;
	struct sk_buff *skb;
	int ret;

	if (mtk_wwan_cmd_check(dev, cmd))
		return -EOPNOTSUPP;

	skb = mtk_wwan_cmd_alloc(cmd, sizeof(void *));
	if (unlikely(!skb))
		return -ENOMEM;

	SKB_TO_CMD_DATA(skb) = data;

	ret = mtk_wwan_cmd_send(dev, skb);
	if (ret < 0) {
		wwan_inst = netdev_priv(dev);
		MTK_ERR(wwan_inst->wcb->mdev, "Failed to excute command:ret=%d,cmd=%d\n", ret, cmd);
	}

	mtk_wwan_cmd_free(skb);

	return ret;
}

static void mtk_wwan_dump(struct mtk_wwan_ctlb *wcb)
{
	struct mtk_wwan_instance *wwan_inst;
	struct rtnl_link_stats64 *stats;
	int i;

	if (unlikely(!wcb)) {
		pr_err("Failed to dump wcb, wcb is NULL pointer\n");
		return;
	}

	MTK_DBG(wcb->mdev,
		MTK_DBG_WWAN, MTK_MEMLOG_RG_5, "============WWAN_DUMP_START============\n");
	/* Dump wwan_ctlb. */
	MTK_DBG(wcb->mdev, MTK_DBG_WWAN, MTK_MEMLOG_RG_5,
		"cap=0x%x, active_cnt=%u, reg_done=%d\n",
		      wcb->cap, wcb->active_cnt, wcb->reg_done);

	/* Dump wwan_inst. */
	for (i = 0; i < wcb->wwan_inst_num; i++) {
		wwan_inst = wcb->wwan_inst[i];
		if (!wwan_inst) {
			MTK_DBG(wcb->mdev,
				MTK_DBG_WWAN, MTK_MEMLOG_RG_5, "wwan%d was not registered.\n");
			continue;
		}
		MTK_DBG(wcb->mdev, MTK_DBG_WWAN, MTK_MEMLOG_RG_5,
			"============%s============features=%pNF\n",
			      wwan_inst->netdev->name, &wwan_inst->netdev->features);

		/* Dump stats. */
		stats = &wwan_inst->stats;
		MTK_DBG(wcb->mdev, MTK_DBG_WWAN, MTK_MEMLOG_RG_5,
			"rx: pkts=%u, bytes=%u, dropped=%u, errors=%u\n",
			      stats->rx_packets, stats->rx_bytes,
			      stats->rx_dropped, stats->rx_errors);
		MTK_DBG(wcb->mdev, MTK_DBG_WWAN, MTK_MEMLOG_RG_5,
			"tx: pkts=%u, bytes=%u, dropped=%u, errors=%u, timeout=%u, busy=%u\n",
			      stats->tx_packets, stats->tx_bytes,
			      stats->tx_dropped, stats->tx_errors,
			      wwan_inst->tx_timeout, wwan_inst->tx_busy);
	}
	MTK_DBG(wcb->mdev,
		MTK_DBG_WWAN, MTK_MEMLOG_RG_5, "============WWAN_DUMP_END============\n");
}

static void mtk_wwan_napi_exit(struct mtk_wwan_ctlb *wcb)
{
	int i;

	for (i = 0; i < wcb->rxq_num; i++) {
		if (!wcb->napis[i]) {
			MTK_ERR(wcb->mdev, "Invalid parameter, napi%d is NULL!", i);
			continue;
		}

		MTK_INFO(wcb->mdev, "napi%d - 0x%llx, napi_id=%d del", i,
			 (u64)wcb->napis[i], wcb->napis[i]->napi_id);
		netif_napi_del(wcb->napis[i]);
	}
}

#ifdef CONFIG_MTK_ENABLE_WQ_GRO
static void mtk_wwan_gro_normal_list(struct napi_struct *napi)
{
	local_bh_disable();
	napi_gro_flush(napi, false);
	if (napi->rx_count) {
		netif_receive_skb_list(&napi->rx_list);
		INIT_LIST_HEAD(&napi->rx_list);
		napi->rx_count = 0;
	}
	local_bh_enable();
}

static void mtk_wwan_rx_flush(struct mtk_wwan_ctlb *wcb, u8 qid)
{
	struct mtk_wwan_instance *wwan_inst;
	int i;

	for (i = 0; i < wcb->wwan_inst_num; i++) {
		wwan_inst = wcb->wwan_inst[i];
		if (unlikely(!wwan_inst))
			continue;

		if (wwan_inst->netdev->flags & IFF_UP)
			mtk_wwan_gro_normal_list(&wwan_inst->wq_napis[qid]);
	}
}

static void mtk_wwan_wq_napi_exit(struct mtk_wwan_instance *wwan_inst)
{
	int i;

	for (i = 0; i < wwan_inst->wcb->rxq_num; i++) {
		if (!wwan_inst->wq_napis) {
			MTK_ERR(wwan_inst->wcb->mdev, "Invalid parameter, wq_napis%d is NULL!", i);
			continue;
		}

		MTK_INFO(wwan_inst->wcb->mdev, "wq_napis%d - 0x%llx, napi_id=%d del", i,
			 (u64)&wwan_inst->wq_napis[i], wwan_inst->wq_napis[i].napi_id);
		mtk_wwan_gro_normal_list(&wwan_inst->wq_napis[i]);
		netif_napi_del(&wwan_inst->wq_napis[i]);
	}
	devm_kfree(wwan_inst->wcb->mdev->dev, wwan_inst->wq_napis);
	wwan_inst->wq_napis = NULL;
}

int mtk_wwan_wq_napi_poll(struct napi_struct *napi, int budget)
{
	return 0;
}

static int mtk_wwan_wq_napi_init(struct mtk_wwan_instance *wwan_inst, struct net_device *dev)
{
	int i;

	wwan_inst->wq_napis = devm_kcalloc(wwan_inst->wcb->mdev->dev, wwan_inst->wcb->rxq_num,
					   sizeof(struct napi_struct), GFP_KERNEL);
	if (!wwan_inst->wq_napis) {
		MTK_ERR(wwan_inst->wcb->mdev, "Invalid parameter, wq_napis is NULL!");
		return -EINVAL;
	}

	for (i = 0; i < wwan_inst->wcb->rxq_num; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
		netif_napi_add_weight(dev, &wwan_inst->wq_napis[i],
				      mtk_wwan_wq_napi_poll, napi_budget);
#else
		netif_napi_add(dev, &wwan_inst->wq_napis[i],
			       mtk_wwan_wq_napi_poll, napi_budget);
#endif

		MTK_INFO(wwan_inst->wcb->mdev, "wq_napis%d-0x%llx napi_id=%d add done", i,
			 (u64)&wwan_inst->wq_napis[i], wwan_inst->wq_napis[i].napi_id);
	}

	return 0;
}
#endif

static int mtk_wwan_napi_init(struct mtk_wwan_ctlb *wcb, struct net_device *dev)
{
	int i;

	for (i = 0; i < wcb->rxq_num; i++) {
		if (!wcb->napis[i]) {
			MTK_ERR(wcb->mdev, "Invalid parameter, napi%d is NULL!", i);
			goto err;
		}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
	netif_napi_add_weight(dev, wcb->napis[i], wcb->trans_ops->poll, napi_budget);
#else
	netif_napi_add(dev, wcb->napis[i], wcb->trans_ops->poll, napi_budget);
#endif

		MTK_INFO(wcb->mdev, "napi%d-0x%llx napi_id=%d add done", i,
			 (u64)wcb->napis[i], wcb->napis[i]->napi_id);
	}

	return 0;

err:
	for (--i; i >= 0; i--)
		netif_napi_del(wcb->napis[i]);
	return -EINVAL;
}

static int mtk_wwan_start_txq(struct mtk_wwan_ctlb *wcb, u32 qmask)
{
	struct mtk_wwan_instance *wwan_inst;
	struct netdev_queue *txq;
	struct net_device *dev;
	int qno;
	int i;

	/* All wwan network devices share same HIF queue */
	for (i = 0; i < wcb->wwan_inst_num; i++) {
		wwan_inst = wcb->wwan_inst[i];
		if (!wwan_inst) {
			MTK_WARN(wcb->mdev, "wwan%d was not allocated\n", i);
			continue;
		}

		dev = wwan_inst->netdev;

		if (!(dev->flags & IFF_UP))
			continue;

		if (qmask == 0xFF) {
			netif_tx_wake_all_queues(dev);
			netif_carrier_on(dev);
			continue;
		}

		for (qno = 0; qno < wcb->txq_num; qno++) {
			if (qmask & BIT(qno)) {
				txq = netdev_get_tx_queue(dev, qno);
				netif_tx_wake_queue(txq);
			}
		}
	}
	return 0;
}

static int mtk_wwan_stop_txq(struct mtk_wwan_ctlb *wcb, u32 qmask)
{
	struct mtk_wwan_instance *wwan_inst;
	struct netdev_queue *txq;
	struct net_device *dev;
	int qno;
	int i;

	/* All wwan network devices share same HIF queue */
	for (i = 0; i < wcb->wwan_inst_num; i++) {
		wwan_inst = wcb->wwan_inst[i];
		if (!wwan_inst) {
			MTK_WARN(wcb->mdev, "wwan%d was not alloced\n", i);
			continue;
		}

		dev = wwan_inst->netdev;

		if (!(dev->flags & IFF_UP))
			continue;

		if (qmask == 0xFF) {
			netif_carrier_off(dev);

			/*
			 * the network transmit lock has already been held
			 * in the ndo_start_xmit context
			 */
			netif_tx_stop_all_queues(dev);
			continue;
		}

		for (qno = 0; qno < wcb->txq_num; qno++) {
			if (qmask & BIT(qno)) {
				txq = netdev_get_tx_queue(dev, qno);
				netif_tx_stop_queue(txq);
			}
		}
	}
	return 0;
}

static void mtk_wwan_netdev_exit(struct mtk_wwan_ctlb *wcb)
{
	struct mtk_wwan_instance *wwan_inst;
	int i;

	for (i = 0; i < wcb->wwan_inst_num; i++) {
		wwan_inst = wcb->wwan_inst[i];
		if (unlikely(!wwan_inst)) {
			MTK_WARN(wcb->mdev, "wwan_inst=%d not existed\n", i);
			continue;
		}
#ifdef CONFIG_MTK_ENABLE_WQ_GRO
		mtk_wwan_wq_napi_exit(wwan_inst);
#endif

		if (!wwan_inst->netdev) {
			MTK_WARN(wcb->mdev, "Net device=%d not existed\n", i);
		} else {
			MTK_INFO(wcb->mdev, "Unregister %s\n", wwan_inst->netdev->name);
			unregister_netdev(wwan_inst->netdev);
		}

		wcb->wwan_inst[i] = NULL;
	}

	wcb->reg_done = false;
}

static void mtk_wwan_setup(struct net_device *dev)
{
	dev->watchdog_timeo = MTK_NETDEV_WDT;
	dev->mtu = MTK_DFLT_MTU;
	dev->min_mtu = ETH_MIN_MTU;

	dev->features = NETIF_F_SG;
	dev->hw_features = NETIF_F_SG;

	dev->features |= NETIF_F_RXHASH;
	dev->hw_features |= NETIF_F_RXHASH;

#ifdef CONFIG_MTK_ENABLE_WQ_GRO
	dev->features |= NETIF_F_GRO_FRAGLIST;
#endif

	dev->tx_queue_len = DEFAULT_TX_QUEUE_LEN;

	dev->flags = IFF_NOARP;
	dev->type = ARPHRD_NONE;

	dev->needs_free_netdev = true;

	dev->netdev_ops = &mtk_netdev_ops;
	mtk_ethtool_set_ops(dev);
}

static int mtk_wwan_netdev_init(struct mtk_wwan_ctlb *wcb)
{
	struct mtk_wwan_instance *wwan_inst;
	struct net_device *dev;
	int ret = 0;
	int i;

	for (i = 0; i < wcb->wwan_inst_num; i++) {
		dev = alloc_netdev_mqs(sizeof(*wwan_inst), "wwan%d", NET_NAME_UNKNOWN,
				       mtk_wwan_setup, wcb->txq_num, wcb->rxq_num);
		if (unlikely(!dev)) {
			/* note that */
			MTK_ERR(wcb->mdev, "Failed to alloc netdev%d\n", i);
			ret = -ENOMEM;
			goto err;
		}

		/* Init private structure of netdev. */
		wwan_inst = netdev_priv(dev);
		wwan_inst->netdev = dev;
		wwan_inst->intf_id = i;
		wwan_inst->wcb = wcb;
		wwan_inst->pkt_prio = PKT_PRIO_INVALID;

		if (wcb->cap & DATA_F_LRO) {
			dev->features |= NETIF_F_LRO;
			dev->hw_features |= NETIF_F_LRO;
		}
		if (wcb->cap & DATA_F_RXCSUM) {
			dev->features |= NETIF_F_RXCSUM;
			dev->hw_features |= NETIF_F_RXCSUM;
		}
		if (wcb->cap & DATA_F_TXCSUM) {
			dev->features |= NETIF_F_HW_CSUM;
			dev->hw_features |= NETIF_F_HW_CSUM;
		}

		ret = register_netdev(dev);
		if (ret) {
			MTK_ERR(wcb->mdev, "Failled to register netdev,error:%d\n", ret);
			free_netdev(dev);
			goto err;
		}

		wcb->wwan_inst[i] = wwan_inst;

#ifdef CONFIG_MTK_ENABLE_WQ_GRO
		mtk_wwan_wq_napi_init(wwan_inst, dev);
#endif
	}

	wcb->reg_done = true;

	MTK_INFO(wcb->mdev, "Init wwan netdev done\n");

	return 0;

err:
	for (i = 0; i < wcb->wwan_inst_num; i++) {
		wwan_inst = wcb->wwan_inst[i];
		if (!wwan_inst)
			break;

		dev = wwan_inst->netdev;
		if (dev)
			unregister_netdev(dev);

		wcb->wwan_inst[i] = NULL;
	}

	return ret;
}

static void mtk_wwan_notify(struct mtk_data_blk *data_blk, enum mtk_data_evt evt, u64 data)
{
	struct mtk_wwan_ctlb *wcb;
	int ret = 0;

	if (unlikely(!data_blk || !data_blk->wcb)) {
		pr_err("Invalid parameter, data_blk = 0x%llx\n", (u64)data_blk);
		return;
	}

	wcb = data_blk->wcb;

	switch (evt) {
	case DATA_EVT_TX_START:
		ret = mtk_wwan_start_txq(wcb, data);
		break;

	case DATA_EVT_TX_STOP:
		ret = mtk_wwan_stop_txq(wcb, data);
		break;

	case DATA_EVT_RX_START:
		mtk_wwan_napi_enable(wcb);
		break;

	case DATA_EVT_RX_STOP:
		mtk_wwan_napi_disable(wcb);
		break;
	case DATA_EVT_RX_SYNC:
		mtk_wwan_napi_sync(wcb);
		break;
#ifdef CONFIG_MTK_ENABLE_WQ_GRO
	case DATA_EVT_RX_FLUSH:
		mtk_wwan_rx_flush(wcb, data);
		break;
#endif
	case DATA_EVT_REG_DEV:
		if (wcb->reg_done) {
			MTK_WARN(wcb->mdev, "Repeat register netdev\n");
			ret = -EINVAL;
		} else {
			ret = mtk_wwan_netdev_init(wcb);
		}
		break;

	case DATA_EVT_UNREG_DEV:
		if (wcb->reg_done)
			mtk_wwan_netdev_exit(wcb);
		else
			MTK_WARN(wcb->mdev, "Repeat unregister netdev\n");
		break;

	case DATA_EVT_DUMP:
		mtk_wwan_dump(wcb);
		break;

	default:
		MTK_INFO(wcb->mdev, "Invalid parameter, DATA_EVENT=%d\n", evt);
		MTK_BUG(wcb->mdev);
		break;
	}
}

static int mtk_wwan_init(struct mtk_data_blk *data_blk, struct mtk_data_trans_info *trans_info)
{
	struct mtk_wwan_ctlb *wcb;
	int ret;

	if (unlikely(!data_blk || !trans_info)) {
		pr_err("Invalid parameter data_blk = 0x%llx,trans_info = 0x%llx\n",
		       (u64)data_blk, (u64)trans_info);
		return -EINVAL;
	}

	wcb = devm_kzalloc(data_blk->mdev->dev, sizeof(*wcb), GFP_KERNEL);
	if (unlikely(!wcb)) {
		/* note that */
		MTK_ERR(data_blk->mdev, "Failed to alloc mtk_wwan_ctlb struct\n");
		return -ENOMEM;
	}

	wcb->trans_ops = &data_trans_ops;
	wcb->mdev = data_blk->mdev;
	wcb->data_blk = data_blk;
	wcb->napis = trans_info->napis;
	wcb->max_mtu = trans_info->max_mtu;
	wcb->cap = trans_info->cap;
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
	wcb->aff_cfg = trans_info->aff_cfg;
#endif
	if (nic_intf_num > MTK_NETDEV_MAX || nic_intf_num == 0)
		nic_intf_num = MTK_NETDEV_MAX;
	wcb->wwan_inst_num = nic_intf_num;
	wcb->rxq_num = trans_info->rxq_cnt;
	wcb->txq_num = trans_info->txq_cnt;
	atomic_set(&wcb->napi_enabled, 0);
	init_dummy_netdev(&wcb->dummy_dev);
	data_blk->wcb = wcb;

	/* Multiple virtual network devices share one physical device,
	 * so we use dummy device to enable NAPI for multiple virtual network devices.
	 */
	ret = mtk_wwan_napi_init(wcb, &wcb->dummy_dev);
	if (ret < 0)
		goto err_napi_init;

	MTK_INFO(data_blk->mdev, "mtk wwan init done\n");

#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	MTK_INFO(data_blk->mdev, "MTK TSQ Shift=%u\n", mtk_tsq_shift);
#endif
	return 0;

err_napi_init:
	data_blk->wcb = NULL;
	devm_kfree(data_blk->mdev->dev, wcb);

	return ret;
}

static void mtk_wwan_exit(struct mtk_data_blk *data_blk)
{
	struct mtk_wwan_ctlb *wcb;

	if (unlikely(!data_blk || !data_blk->wcb)) {
		pr_err("Invalid parameter data_blk = 0x%llx\n", (u64)data_blk);
		return;
	}

	wcb = data_blk->wcb;
	mtk_wwan_napi_exit(wcb);
	devm_kfree(data_blk->mdev->dev, wcb);
	data_blk->wcb = NULL;

	MTK_INFO(data_blk->mdev, "mtk wwan exit done\n");
}

struct mtk_data_port_ops data_port_ops = {
	.init = mtk_wwan_init,
	.exit = mtk_wwan_exit,
	.recv = mtk_wwan_data_recv,
	.notify = mtk_wwan_notify,
};

module_param(napi_budget, uint, 0444);
MODULE_PARM_DESC(napi_budget, "This value indicates at most packets receive once napi schedule, default 128\n");
module_param(nic_intf_num, uint, 0444);
MODULE_PARM_DESC(nic_intf_num, "This value indicates how many NIC interfaces to be created, default 20\n");
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
module_param(mtk_tsq_shift, uint, 0644);
MODULE_PARM_DESC(mtk_tsq_shift, "Default TCP small queue budget(1 second >> bit shift), default 7, ~8ms.\n");
#endif
