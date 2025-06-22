// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#define pr_fmt(fmt) "MTK_WWAN: " fmt

#include <linux/completion.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/printk.h>
#include <linux/skbuff.h>
#include <linux/skmsg.h>
#include <linux/slab.h>
#include <linux/types.h>
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
#include <linux/version.h>
#endif
#include <linux/wwan.h>
#include <net/pkt_sched.h>
#include <net/sch_generic.h>
#include <uapi/linux/if.h>
#include <uapi/linux/if_arp.h>
#include <uapi/linux/if_link.h>
#include "mtk_data_plane.h"
#include "mtk_dev.h"
#include "mtk_debug.h"

#ifdef CONFIG_TX00_UT_WWAN
#include "ut_wwan_fake.h"
#endif

#define MTK_NETDEV_MAX		20
#define MTK_DFLT_INTF_ID	0
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

/* struct mtk_wwan_instance - This is netdevice's private data,
 * contains information about netdevice.
 * @wcb: Contains all information about WWAN port layer.
 * @stats: Statistics of netdevice's tx/rx packets.
 * @tx_busy: Statistics of netdevice's busy counts.
 * @netdev: Pointer to netdevice structure.
 * @intf_id: The netdevice's interface id
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
};

/* struct mtk_wwan_ctlb - Contains WWAN port layer information and save trans information needed.
 * @data_blk: Contains data port, trans layer, md_dev structure.
 * @mdev: Pointer of mtk_md_dev.
 * @trans_ops: Contains trans layer ops: send, select_txq, napi_poll.
 * @napis: Trans layer alloc napi structure by rx queue.
 * @dummy_dev: Used for multiple network devices share one napi.
 * @cap: Contains different hardware capabilities.
 * @max_mtu: The max MTU supported.
 * @napi_enable: Mark for napi state.
 * @active_cnt: The counter of network devices that are UP.
 * @txq_num: Total TX qdisc number.
 * @rxq_num: Total RX qdisc number.
 * @reg_done: Mark for ntwork devices register state.
 */
struct mtk_wwan_ctlb {
	struct mtk_data_blk *data_blk;
	struct mtk_md_dev *mdev;
	struct mtk_data_trans_ops *trans_ops;
	struct mtk_wwan_instance __rcu *wwan_inst[MTK_NETDEV_MAX];
	struct napi_struct **napis;
	struct net_device dummy_dev;

	u32 cap;
	atomic_t napi_enabled;
	unsigned int max_mtu;
	unsigned int active_cnt;
	unsigned char txq_num;
	unsigned char rxq_num;
	bool reg_done;
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
	struct net_device *netdev;
	struct napi_struct *napi;

	if (intf_id > MTK_MAX_INTF_ID) {
		MTK_WARN(data_blk->mdev, "Invalid interface id=%d\n", intf_id);
		MTK_BUG(data_blk->mdev);
		goto err_rx;
	}

	rcu_read_lock();
	wwan_inst = rcu_dereference(data_blk->wcb->wwan_inst[intf_id]);

	if (unlikely(!wwan_inst)) {
		MTK_ERR(data_blk->mdev, "Invalid pointer wwan_inst is NULL\n");
		rcu_read_unlock();
		goto err_rx;
	}

	napi = data_blk->wcb->napis[q_id];
	netdev = wwan_inst->netdev;

	mtk_wwan_set_skb(skb, netdev);

	trace_mtk_tput_data_rx(q_id, "rx p4");

	wwan_inst->stats.rx_packets++;
	wwan_inst->stats.rx_bytes += skb->len;

	skb_reset_mac_header(skb);
	napi_gro_receive(napi, skb);

	trace_mtk_tput_data_rx(q_id, "rx p5");

	rcu_read_unlock();
	return 0;

err_rx:
	dev_kfree_skb_any(skb);
	return -EINVAL;
}

static void mtk_wwan_napi_enable(struct mtk_wwan_ctlb *wcb)
{
	int i;

	if (atomic_cmpxchg(&wcb->napi_enabled, 0, 1) == 0) {
		for (i = 0; i < wcb->rxq_num; i++)
			napi_enable(wcb->napis[i]);
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
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);
	struct mtk_wwan_ctlb *wcb = wwan_inst->wcb;
	struct mtk_data_trans_ctl trans_ctl;
	int ret;

	MTK_INFO(wcb->mdev, "Opening netdev=%s, active_cnt=%u\n",
		 dev->name, wcb->active_cnt);

	if (wcb->active_cnt == 0) {
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

	netif_tx_start_all_queues(dev);
	netif_carrier_on(dev);

	return 0;

err_ctl:
	mtk_wwan_napi_disable(wcb);
	return ret;
}

static int mtk_wwan_stop(struct net_device *dev)
{
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);
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

	/* ndo_stop hold rtnl_lock() */
	wcb->active_cnt--;

	return 0;
}

static void mtk_wwan_select_txq(struct mtk_wwan_instance *wwan_inst, struct sk_buff *skb,
				enum mtk_pkt_type pkt_type)
{
	u16 qid;

	trace_mtk_tput_data_tx("tx p1");

	qid = wwan_inst->wcb->trans_ops->select_txq(wwan_inst->wcb->data_blk, skb, pkt_type,
						    wwan_inst->pkt_prio);
	if (qid > wwan_inst->wcb->txq_num)
		qid = 0;
	skb_set_queue_mapping(skb, qid);
}

static netdev_tx_t mtk_wwan_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);
	union mtk_data_pkt_info pkt_info;
	unsigned int skb_len = skb->len;
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
	/* select trans layer virtual queue */
	mtk_wwan_select_txq(wwan_inst, skb, PURE_IP);

	/* Forward skb to trans layer(DPMAIF). */
	ret = wwan_inst->wcb->trans_ops->send(wwan_inst->wcb->data_blk, DATA_PKT, skb, data);
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

static void mtk_wwan_get_stats(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);

	memcpy(stats, &wwan_inst->stats, sizeof(*stats));
}

static void mtk_wwan_tx_timeout(struct net_device *dev, unsigned int txq)
{
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);

	wwan_inst->tx_timeout++;

	MTK_WARN(wwan_inst->wcb->mdev,
		 "wwan%d tx_timeout, txq=%u, active_cnt=%u\n",
		wwan_inst->intf_id, txq, wwan_inst->wcb->active_cnt);
}

static netdev_features_t mtk_wwan_fix_feature(struct net_device *dev,
					      netdev_features_t features)
{
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);

	if (!(features & NETIF_F_RXCSUM)) {
		MTK_INFO(wwan_inst->wcb->mdev, "disabling LRO as RXCSUM is off\n");
		features &= ~NETIF_F_LRO;
	}
	return features;
}

static int mtk_wwan_set_feature(struct net_device *dev,
				netdev_features_t features)
{
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
static int mtk_wwan_siocdevprivate(struct net_device *dev, struct ifreq *ifr,
				   void __user *data, int cmd)
{
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);
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
			if (network_type > DATA_NETWORK_TYPE_MAX) {
				MTK_ERR(wwan_inst->wcb->mdev,
					"Invalid network_type, type=%d,intf_id=%u\n",
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
#endif

static const struct net_device_ops mtk_netdev_ops = {
	.ndo_open = mtk_wwan_open,
	.ndo_stop = mtk_wwan_stop,
	.ndo_start_xmit = mtk_wwan_start_xmit,
	.ndo_tx_timeout = mtk_wwan_tx_timeout,
	.ndo_get_stats64	= mtk_wwan_get_stats,
	.ndo_fix_features = mtk_wwan_fix_feature,
	.ndo_set_features = mtk_wwan_set_feature,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
	.ndo_siocdevprivate = mtk_wwan_siocdevprivate,
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
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);
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

	init_completion(&event->done);
	event->data_complete = mtk_wwan_cmd_complete;

	return skb;
}

static int mtk_wwan_cmd_send(struct net_device *dev, struct sk_buff *skb)
{
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);
	struct mtk_data_cmd *event = (struct mtk_data_cmd *)skb->data;
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
		wwan_inst = wwan_netdev_drvpriv(dev);
		MTK_ERR(wwan_inst->wcb->mdev, "Failed to excute command:ret=%d,cmd=%d\n", ret, cmd);
	}

	if (likely(skb))
		dev_kfree_skb_any(skb);

	return ret;
}

static void mtk_wwan_dump(struct mtk_wwan_ctlb *wcb)
{
	int i;
	struct mtk_wwan_instance *wwan_inst;
	struct rtnl_link_stats64 *stats;

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

	rcu_read_lock();
	/* Dump wwan_inst. */
	for (i = 0; i < MTK_NETDEV_MAX; i++) {
		wwan_inst = rcu_dereference(wcb->wwan_inst[i]);
		if (!wwan_inst) {
			MTK_DBG(wcb->mdev,
				MTK_DBG_WWAN, MTK_MEMLOG_RG_5, "wwan%d was not registered.\n", i);
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
	rcu_read_unlock();
	MTK_DBG(wcb->mdev,
		MTK_DBG_WWAN, MTK_MEMLOG_RG_5, "============WWAN_DUMP_END============\n");
}

static int mtk_wwan_start_txq(struct mtk_wwan_ctlb *wcb, u32 qmask)
{
	struct mtk_wwan_instance *wwan_inst;
	struct net_device *dev;
	int i;

	rcu_read_lock();
	/* All wwan network devices share same HIF queue */
	for (i = 0; i < MTK_NETDEV_MAX; i++) {
		wwan_inst = rcu_dereference(wcb->wwan_inst[i]);
		if (!wwan_inst)
			continue;

		dev = wwan_inst->netdev;

		if (!(dev->flags & IFF_UP))
			continue;

		netif_tx_wake_all_queues(dev);
		netif_carrier_on(dev);
	}
	rcu_read_unlock();

	return 0;
}

static int mtk_wwan_stop_txq(struct mtk_wwan_ctlb *wcb, u32 qmask)
{
	struct mtk_wwan_instance *wwan_inst;
	struct net_device *dev;
	int i;

	rcu_read_lock();
	/* All wwan network devices share same HIF queue */
	for (i = 0; i < MTK_NETDEV_MAX; i++) {
		wwan_inst = rcu_dereference(wcb->wwan_inst[i]);
		if (!wwan_inst)
			continue;

		dev = wwan_inst->netdev;

		if (!(dev->flags & IFF_UP))
			continue;

		netif_carrier_off(dev);
		/* the network transmit lock has already been held in the ndo_start_xmit context */
		netif_tx_stop_all_queues(dev);
	}
	rcu_read_unlock();

	return 0;
}

static void mtk_wwan_napi_exit(struct mtk_wwan_ctlb *wcb)
{
	int i;

	for (i = 0; i < wcb->rxq_num; i++) {
		if (!wcb->napis[i])
			continue;
		MTK_INFO(wcb->mdev, "napi%d - 0x%llx, napi_id=%d del", i,
			 (u64)wcb->napis[i], wcb->napis[i]->napi_id);
		netif_napi_del(wcb->napis[i]);
	}
}

static int mtk_wwan_napi_init(struct mtk_wwan_ctlb *wcb, struct net_device *dev)
{
	int i;

	for (i = 0; i < wcb->rxq_num; i++) {
		if (!wcb->napis[i]) {
			MTK_ERR(wcb->mdev, "Invalid napi pointer, napi=%d", i);
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

static void mtk_wwan_setup(struct net_device *dev)
{
	dev->watchdog_timeo = MTK_NETDEV_WDT;
	dev->mtu = ETH_DATA_LEN;
	dev->min_mtu = ETH_MIN_MTU;

	dev->features = NETIF_F_SG;
	dev->hw_features = NETIF_F_SG;

	dev->features |= NETIF_F_GRO;
	dev->hw_features |= NETIF_F_GRO;

	dev->features |= NETIF_F_RXHASH;
	dev->hw_features |= NETIF_F_RXHASH;

	dev->tx_queue_len = DEFAULT_TX_QUEUE_LEN;

	dev->flags = IFF_NOARP;
	dev->type = ARPHRD_NONE;

	dev->needs_free_netdev = true;

	dev->netdev_ops = &mtk_netdev_ops;
	mtk_ethtool_set_ops(dev);
}

static int mtk_wwan_newlink(void *ctxt, struct net_device *dev, u32 intf_id,
			    struct netlink_ext_ack *extack)
{
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);
	struct mtk_wwan_ctlb *wcb = ctxt;
	int ret;

	if (intf_id > MTK_MAX_INTF_ID) {
		MTK_WARN(wcb->mdev, "interface id=%u invalid\n", intf_id);
		ret = -EINVAL;
		goto out;
	}

	dev->max_mtu = wcb->max_mtu;
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

	wwan_inst->wcb = wcb;
	wwan_inst->netdev = dev;
	wwan_inst->intf_id = intf_id;
	wwan_inst->pkt_prio = PKT_PRIO_INVALID;

	if (rcu_access_pointer(wcb->wwan_inst[intf_id])) {
		MTK_WARN(wcb->mdev, "failed to access wwan_inst, interface id=%u\n", intf_id);
		ret = -EBUSY;
		goto out;
	}

	ret = register_netdevice(dev);
	if (ret)
		goto out;

	rcu_assign_pointer(wcb->wwan_inst[intf_id], wwan_inst);

	netif_device_attach(dev);

	MTK_INFO(wcb->mdev, "%s registered, interface id=%u\n", dev->name, intf_id);

	return 0;
out:
	return ret;
}

static void mtk_wwan_dellink(void *ctxt, struct net_device *dev,
			     struct list_head *head)
{
	struct mtk_wwan_instance *wwan_inst = wwan_netdev_drvpriv(dev);
	int intf_id = wwan_inst->intf_id;
	struct mtk_wwan_ctlb *wcb = ctxt;

	if (WARN_ON(rcu_access_pointer(wcb->wwan_inst[intf_id]) != wwan_inst))
		return;

	RCU_INIT_POINTER(wcb->wwan_inst[intf_id], NULL);
	unregister_netdevice_queue(dev, head);

	MTK_INFO(wcb->mdev, "%s unregistered, interface id=%u\n", dev->name, intf_id);
}

static const struct wwan_ops mtk_wwan_ops = {
	.priv_size = sizeof(struct mtk_wwan_instance),
	.setup = mtk_wwan_setup,
	.newlink = mtk_wwan_newlink,
	.dellink = mtk_wwan_dellink,
};

static void mtk_wwan_notify(struct mtk_data_blk *data_blk, enum mtk_data_evt evt, u64 data)
{
	struct mtk_wwan_ctlb *wcb;

	if (unlikely(!data_blk || !data_blk->wcb)) {
		pr_err("Invalid parameter, data_blk = 0x%llx\n", (u64)data_blk);
		return;
	}

	wcb = data_blk->wcb;

	switch (evt) {
	case DATA_EVT_TX_START:
		mtk_wwan_start_txq(wcb, data);
		break;
	case DATA_EVT_TX_STOP:
		mtk_wwan_stop_txq(wcb, data);
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
	case DATA_EVT_REG_DEV:
		if (!wcb->reg_done) {
			wwan_register_ops(wcb->mdev->dev, &mtk_wwan_ops, wcb, MTK_DFLT_INTF_ID);
			wcb->reg_done = true;
		}
		break;
	case DATA_EVT_UNREG_DEV:
		if (wcb->reg_done) {
			wwan_unregister_ops(wcb->mdev->dev);
			wcb->reg_done = false;
		}
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
	if (unlikely(!wcb))
		return -ENOMEM;

	wcb->trans_ops = &data_trans_ops;
	wcb->mdev = data_blk->mdev;
	wcb->data_blk = data_blk;
	wcb->napis = trans_info->napis;
	wcb->max_mtu = trans_info->max_mtu;
	wcb->cap = trans_info->cap;
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
		goto out;

	MTK_INFO(data_blk->mdev, "mtk wwan init done\n");
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	MTK_INFO(data_blk->mdev, "MTK TSQ Shift=%u\n", mtk_tsq_shift);
#endif
	return 0;
out:
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
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
module_param(mtk_tsq_shift, uint, 0644);
MODULE_PARM_DESC(mtk_tsq_shift, "Default TCP small queue budget(1 second >> bit shift), default 7, ~8ms.\n");
#endif
