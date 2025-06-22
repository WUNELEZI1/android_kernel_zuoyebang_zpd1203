/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IF_MIWILL_H_
#define _IF_MIWILL_H_

#include <linux/timer.h>
#include <linux/netpoll.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/spinlock_types.h>
#include <linux/ethtool.h>
#include <net/sch_generic.h>

#include "miwill_proto.h"
#include "miwill_debug.h"

#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARG(x)                                                             \
	((u8 *)(x))[0], ((u8 *)(x))[1], ((u8 *)(x))[2], ((u8 *)(x))[3],        \
		((u8 *)(x))[4], ((u8 *)(x))[5]

#define IPv4_FMT "%d.%d.%d.%d"
#define IPv4_ARG(x)                                                             \
	((u8 *)(x))[0], ((u8 *)(x))[1], ((u8 *)(x))[2], ((u8 *)(x))[3]

#define IFF_MIWILL IFF_TEAM
#define IFF_MIWILL_PORT IFF_TEAM_PORT
#define MIWILL_HEAD_LEN 8

struct miwill_pcpu_stats {
	u64			rx_packets;
	u64			rx_bytes;
	u64			rx_multicast;
	u64			tx_packets;
	u64			tx_bytes;
	struct u64_stats_sync	syncp;
	u32			rx_dropped;
	u32			tx_dropped;
	u32			rx_nohandler;
};

struct miwill;

typedef enum {
	WIFI_FREQ_UNKNOW = 0,
	WIFI_FREQ_5G,
	WIFI_FREQ_24G
} miwill_wlan_freq_t;

struct miwill_port {
	struct net_device *dev;
	struct hlist_node hlist; /* node in enabled ports hash list */
	struct list_head list; /*node in ordinary list*/
	struct miwill *miwill;
	int index; /* index of enabled port. If disabled, it's set to -1 */
	unsigned char src_id[MIWILL_SRC_ID_LEN]; /* last 2byte of miw mac_addr */

	struct {
		bool linkup;
		u32 speed;
		u8 duplex;
	} state;

	/*
	 * A place for storing original values of the device before it
	 * become a port.
	 */
	struct {
		unsigned int mtu;
	} orig;

#ifdef CONFIG_NET_POLL_CONTROLLER
	struct netpoll *np;
#endif

	s32 priority; /* lower number ~ higher priority */
	u16 queue_id;
	struct list_head qom_list; /* node in queue override mapping list */
	struct rcu_head rcu;
	miwill_wlan_freq_t frequency;
};

/*
 * Helpers for struct miwill_port
 */

/*
 * Note: here we re-use the private flags used by Team and Team Port Dev
 */
static inline bool netif_is_miwill_port(const struct net_device *dev)
{
	return dev->priv_flags & IFF_MIWILL_PORT;
}

static inline struct miwill_port *
miwill_port_get_rcu(const struct net_device *dev)
{
	return rcu_dereference(dev->rx_handler_data);
}

static inline bool miwill_port_enabled(struct miwill_port *port)
{
	return port->index != -1;
}

static inline bool miwill_port_txable(struct miwill_port *port)
{
	return port->state.linkup && miwill_port_enabled(port);
}

static inline bool miwill_port_dev_txable(const struct net_device *dev)
{
	struct miwill_port *port;
	bool txable;

	rcu_read_lock();
	port = miwill_port_get_rcu(dev);
	txable = port ? miwill_port_txable(port) : false;
	rcu_read_unlock();

	return txable;
}

/*
 * for net_poll, unlikely used
 */
#ifdef CONFIG_NET_POLL_CONTROLLER
static inline void miwill_netpoll_send_skb(struct miwill_port *port,
					   struct sk_buff *skb)
{
	struct netpoll *np = port->np;

	if (np)
		netpoll_send_skb(np, skb);
}
#else
static inline void miwill_netpoll_send_skb(struct miwill_port *port,
					   struct sk_buff *skb)
{
}
#endif

/*
 * The operations defined by the miwill mode specified;
 * following team_mode_ops,
 * No port_change_dev_addr func_ptr;
 */
struct miwill_mode_ops {
	int (*init)(struct miwill *miwill);
	void (*exit)(struct miwill *miwill);
	rx_handler_result_t (*receive)(struct miwill *miwill,
				       struct miwill_port *port,
				       struct sk_buff *skb);
	bool (*transmit)(struct miwill *miwill, struct sk_buff *skb);
	int (*port_enter)(struct miwill *miwill, struct miwill_port *port);
	void (*port_leave)(struct miwill *miwill, struct miwill_port *port);
	void (*port_enabled)(struct miwill *miwill, struct miwill_port *port);
	void (*port_disabled)(struct miwill *miwill, struct miwill_port *port);
};


enum miwill_option_type {
	MIWILL_OPTION_TYPE_U32,
	MIWILL_OPTION_TYPE_STRING,
	MIWILL_OPTION_TYPE_BINARY,
	MIWILL_OPTION_TYPE_BOOL,
	MIWILL_OPTION_TYPE_S32,
};

struct miwill_option_inst_info {
	u32 array_index;
	struct miwill_port *port; /* != NULL if per-port */
};

struct miwill_gsetter_ctx {
	union {
		u32 u32_val;
		const char *str_val;
		struct {
			const void *ptr;
			u32 len;
		} bin_val;
		bool bool_val;
		s32 s32_val;
	} data;
	struct miwill_option_inst_info *info;
};

struct miwill_option {
	struct list_head list;
	const char *name;
	bool per_port;
	unsigned int array_size; /* != 0 means the option is array */
	enum miwill_option_type type;
	int (*init)(struct miwill *miw, struct miwill_option_inst_info *info);
	int (*getter)(struct miwill *miw, struct miwill_gsetter_ctx *ctx);
	int (*setter)(struct miwill *miw, struct miwill_gsetter_ctx *ctx);
};

/*
 * miwill mode description
 * netdev_lag_tx_type used by most multi-link techs (team, binding)
 */
struct miwill_mode {
	const char *kind;
	struct module *owner;
	size_t priv_size;
	size_t port_priv_size;
	const struct miwill_mode_ops *ops;
	void *priv_data;
};

extern int miwill_modeop_port_enter(struct miwill *miwill,
				    struct miwill_port *port);

#define MODULE_ALIAS_MIWILL_MODE(kind) MODULE_ALIAS("miwill-mode-" kind)

/*
 *  Definition of  the miwill net_device
 */
#define MIWILL_PORT_HASHBITS 2
#define MIWILL_PORT_HASHENTRIES (1 << MIWILL_PORT_HASHBITS)

typedef struct miwill_minfo{
	unsigned char wlan_mac_5g[ETH_ALEN];
	unsigned char wlan_mac_24g[ETH_ALEN];
	unsigned char gw_ipv4[4];
	unsigned char req_cnt;
} miwill_minfo_t;

struct miwill {
	struct net_device *dev; /*associated netdevice*/
	struct miwill_pcpu_stats __percpu *pcpu_stats;

	spinlock_t lock; /* used for overall locking, e.g. port list write */
	/*
	 * List of enabled ports and their count
	 */
	int en_port_count;
	struct hlist_head
		en_port_hlist[MIWILL_PORT_HASHENTRIES];
	struct list_head port_list; /* list of all ports */

	struct list_head option_list;
	struct list_head option_inst_list; /* list of option instances */

	const struct miwill_mode *mode;

	/*TODO: check the usage of the following options */
	bool queue_override_enabled;
	struct list_head *qom_lists; /* array of queue override mapping lists */

	unsigned char gw_addr[ETH_ALEN];
	miwill_minfo_t mframe_info;
	struct timer_list timer;
};

#define MIWILL_MODE_OPS_EXIST(miw, func) \
		((miw)->mode && (miw)->mode->ops->func)

static inline int miwill_dev_queue_xmit(struct miwill *miw,
					struct miwill_port *port,
					struct sk_buff *skb)
{
	struct MIWILL_ethhdr *miw_ethhdr;
	/* TODO: check how qdisc handle the queue_mapping */
	// BUILD_BUG_ON(sizeof(skb->queue_mapping) !=
	// 	     sizeof(qdisc_skb_cb(skb)->slave_dev_queue_mapping));
	// skb_set_queue_mapping(skb, qdisc_skb_cb(skb)->slave_dev_queue_mapping);
	if(unlikely(!port->dev)){
		miwill_cmn_err("%s port->dev is NULL", __func__);
		dev_kfree_skb(skb);
		return -1;
	}

	skb->dev = port->dev;
	miw_ethhdr = (struct MIWILL_ethhdr *)(skb->data);

	memcpy(miw_ethhdr->h_dest, miw->gw_addr, ETH_ALEN);
	memcpy(miw_ethhdr->h_source, port->dev->dev_addr, ETH_ALEN);
	memcpy(miw_ethhdr->header.src_id,
		       miw->dev->dev_addr + ETH_ALEN - MIWILL_SRC_ID_LEN,
		       MIWILL_SRC_ID_LEN);

#ifdef DEBUG_PACKET
	miwill_cmn_dbg("%s", __func__);
	skb_dump(KERN_ERR,skb,true);
#endif

	/* TODO:
	 * remaining steps before transmission
	 * How about adding the MiWiLL header here???
	 */
	//TODO : just test dev_queue_xmit now
	// if (unlikely(netpoll_tx_running(miw->dev))) {
	// 	miwill_netpoll_send_skb(port, skb);
	// 	return 0;
	// }
	return dev_queue_xmit(skb);
}

static inline struct sk_buff *miwill_add_head(struct sk_buff *skb,struct miwill *miw){
	struct MIWILL_ethhdr *miw_ethhdr;

	if(skb_cow_head(skb,MIWILL_HEAD_LEN)){
		dev_kfree_skb(skb);
		return NULL;
	}

	/*
	miwill_cmn_dbg("%s before", __func__);
	skb_dump(KERN_ERR,skb,true);
	*/

	skb_push(skb, MIWILL_HEAD_LEN);
	miw_ethhdr = (struct MIWILL_ethhdr *)(skb->data);

	skb_reset_mac_header(skb);
	skb->network_header-=MIWILL_HEAD_LEN;

	memmove(skb->data, skb->data + MIWILL_HEAD_LEN, ETH_HLEN);
	miw_ethhdr->header.encap_proto = miw_ethhdr->h_miwill_proto;
	miw_ethhdr->h_miwill_proto = htons(ETH_P_MIWILL);
	miw_ethhdr->header.version = 1;

	/* src_id,src_mac,mode,seq_num need to change */

	skb->protocol = htons(ETH_P_MIWILL);
	skb->priority = 7;

	/*
	miwill_cmn_dbg("%s after", __func__);
	skb_dump(KERN_ERR,skb,true);
	*/

	return skb;
}

static inline struct hlist_head *miwill_port_index_hash(struct miwill *miw,
							int port_idx)
{
	return &miw->en_port_hlist[port_idx & (MIWILL_PORT_HASHENTRIES - 1)];
}

static inline struct miwill_port *
miwill_get_port_by_index_rcu(struct miwill *miw, int port_idx)
{
	struct miwill_port *port;
	struct hlist_head *head = miwill_port_index_hash(miw, port_idx);

	hlist_for_each_entry_rcu (port, head, hlist)
		if (port->index == port_idx)
			return port;
	return NULL;
}

static inline struct miwill_port *
miwill_get_first_port_txable_rcu(struct miwill *miw, struct miwill_port *port)
{
	struct miwill_port *cur;

	if (likely(miwill_port_txable(port)))
		return port;

	cur = port;
	list_for_each_entry_continue_rcu (cur, &miw->port_list, list)
		if (miwill_port_txable(cur))
			return cur;

	list_for_each_entry_rcu (cur, &miw->port_list, list) {
		if (cur == port)
			break;
		if (miwill_port_txable(cur))
			return cur;
	}
	return NULL;
}

extern int miwill_mode_register(const struct miwill_mode *mode);
extern void miwill_mode_unregister(const struct miwill_mode *mode);
extern struct net_device * miwill_name_to_dev(const char *name);
extern int miwill_options_set(struct miwill *miw, unsigned char enable,
		char *mode, unsigned char *gw_addr, char *port_5g, char *port_24g);

#endif /* _IF_MIWILL_H_ */
