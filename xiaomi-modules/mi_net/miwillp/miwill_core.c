/*
 * GPL-licensed ??
 * Core of the MiWiLL (a.k.a Mi Wireless Link-Layer Protocol) engine
 * Author: Minjun Xi (ximinjun@xiaomi.com)
 * Author: Others
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/notifier.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/ctype.h>
#include <linux/etherdevice.h>
#include <linux/moduleparam.h>
#include <linux/rtnetlink.h>
#include <net/rtnetlink.h>
#include <linux/version.h>
#include "include/miwill_debug.h"
#include "include/miwill_proto.h"
#include "include/if_miwill.h"
#include "include/miwill_netlink.h"

#define DRV_NAME "miwill"
#define DRV_VERSION "1.0"

/***************************
* declaration of function
****************************/
int miwill_change_mode(struct miwill *miw, const char *kind);
static rx_handler_result_t miwill_pass_receive(struct miwill *miw,
					      struct miwill_port *port,
					      struct sk_buff *skb);
static int miwill_port_add_by_name(struct miwill *miw, const char *port_name,
		miwill_wlan_freq_t freq);
//static int miwill_port_del_by_name(struct miwill *miw, const char *port_name);
static int miwill_port_del(struct net_device *dev, struct net_device *port_dev);
static int miwill_open(struct net_device *dev);
static int miwill_stop(struct net_device *dev);

/****************************
* miwill management frame
*****************************/
typedef enum miwill_mframe {
	MIWILL_MFRAME_TYPE_MIN = 0,
	MIWILL_MFRAME_TYPE_REQ,
	MIWILL_MFRAME_TYPE_RESP,
	MIWILL_MFRAME_TYPE_RESET,
	MIWILL_MFRAME_TYPE_MAX
} miwill_mframe_t;

struct miwill_mframe_data_req {
	unsigned char wlan_mac_5g[ETH_ALEN];
	unsigned char wlan_mac_24g[ETH_ALEN];
} __attribute__((packed));
typedef struct miwill_mframe_data_req miwill_mframe_data_req_t;

struct miwill_mframe_data_reset {
	unsigned char gw_ipv4[4];
} __attribute__((packed));
typedef struct miwill_mframe_data_reset miwill_mframe_data_reset_t;

static rx_handler_result_t miwill_mframe_handler(struct sk_buff *skb)
{
	struct miwill_port *port;
	struct miwill *miw;
	struct MIWILL_ethhdr *miw_ethhdr;
	miwill_mframe_t type;
	unsigned char *data;

	port = miwill_port_get_rcu(skb->dev);
	miw = port->miwill;
	miw_ethhdr = (struct MIWILL_ethhdr *)(skb->data - ETH_HLEN);
	data = (unsigned char *)(miw_ethhdr + 1);

	type = miw_ethhdr->header.option.cmd.sub_type;
	if (type == MIWILL_MFRAME_TYPE_RESP) {
		miwill_cmn_dbg("%s type:%d "IPv4_FMT, __func__, type, IPv4_ARG(data));
		miw->mframe_info.req_cnt = 0;
		memcpy(miw->mframe_info.gw_ipv4, data, 4);
		miwill_nl_notify_ip(*(u32 *)data);
	} else if (type == MIWILL_MFRAME_TYPE_RESET) {
		miwill_cmn_dbg("%s type:%d "IPv4_FMT, __func__, type, IPv4_ARG(data));
		miwill_nl_notify_reset(MIWILL_RESET_REASON_PEER);
	} else {
		miwill_cmn_dbg("%s type:%d unknow", __func__, type);
	}

	kfree_skb(skb);
	return RX_HANDLER_CONSUMED;
}

static netdev_tx_t _miwill_mframe_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct miwill *miw = netdev_priv(dev);
	struct miwill_port *port;

	list_for_each_entry(port, &miw->port_list, list) {
		if (unlikely(!port))
			goto err;

		if(!miwill_port_txable(port))
			continue;

		return miwill_dev_queue_xmit(miw, port, skb);
	}

err:
	kfree_skb(skb);
	return NETDEV_TX_BUSY;
}

static struct sk_buff *_miwill_mframe_init(struct miwill *miw, miwill_mframe_t type)
{
	struct sk_buff *skb;
	struct MIWILL_ethhdr *miw_ethhdr;
	miwill_mframe_data_req_t *data_req;
	miwill_mframe_data_reset_t *data_reset;
	static int seq_num = 1;

	#define MIWILL_ETHHDR_LEN (sizeof(struct MIWILL_ethhdr))
	#define MIWILL_MFRAME_REQ_DATA_LEN (sizeof(miwill_mframe_data_req_t))
	#define MIWILL_MFRAME_RESET_DATA_LEN (sizeof(miwill_mframe_data_reset_t))

	/* padding data */
	switch (type) {
		case MIWILL_MFRAME_TYPE_REQ:
			skb = netdev_alloc_skb(miw->dev, MIWILL_ETHHDR_LEN + MIWILL_MFRAME_REQ_DATA_LEN);
			skb_reserve(skb, MIWILL_ETHHDR_LEN + MIWILL_MFRAME_REQ_DATA_LEN);
			skb_push(skb, MIWILL_MFRAME_REQ_DATA_LEN);
			data_req = (miwill_mframe_data_req_t *)skb->data;
			memcpy(data_req->wlan_mac_5g, miw->mframe_info.wlan_mac_5g, ETH_ALEN);
			memcpy(data_req->wlan_mac_24g, miw->mframe_info.wlan_mac_24g, ETH_ALEN);
			break;
		case MIWILL_MFRAME_TYPE_RESET:
			skb = netdev_alloc_skb(miw->dev, MIWILL_ETHHDR_LEN + MIWILL_MFRAME_RESET_DATA_LEN);
			skb_reserve(skb, MIWILL_ETHHDR_LEN + MIWILL_MFRAME_RESET_DATA_LEN);
			skb_push(skb, MIWILL_MFRAME_RESET_DATA_LEN);
			data_reset = (miwill_mframe_data_reset_t *)skb->data;
			memcpy(data_reset->gw_ipv4, miw->mframe_info.gw_ipv4, 4);
			break;
		default:
			return NULL;
	}

	skb_set_network_header(skb, 0);
	skb_set_transport_header(skb, 0);

	/* padding eth header and miwill header */
	skb_push(skb, MIWILL_ETHHDR_LEN);
	miw_ethhdr = (struct MIWILL_ethhdr *)skb->data;
	miw_ethhdr->h_miwill_proto = htons(ETH_P_MIWILL);
	miw_ethhdr->header.version = MIWILL_VERSION;
	miw_ethhdr->header.mode = CMD_MODE;
	miw_ethhdr->header.option.cmd.sub_type = type;
	miw_ethhdr->header.option.cmd.seq_num = htons(seq_num ++);
	miw_ethhdr->header.encap_proto = htons(ETH_P_MIWILL);

	/* set skb member */
	skb->protocol = htons(ETH_P_MIWILL);
	skb->dev = miw->dev;

	return skb;
}

static netdev_tx_t miwill_mframe(struct miwill *miw, miwill_mframe_t type)
{
	struct sk_buff *skb;

	miwill_cmn_dbg("%s %d", __func__, type);
	skb = _miwill_mframe_init(miw, type);
	if (unlikely(!skb)) {
		miwill_cmn_dbg("_miwill_mframe_init type:%d err.\n", type);
	}
	return _miwill_mframe_xmit(skb, miw->dev);
}

#define MIWILL_TIMER_IVAL   (10 * HZ)

static void miwill_timer(struct timer_list *t)
{
	struct miwill *miw = from_timer(miw, t, timer);

	if ((++ miw->mframe_info.req_cnt) > 3) {
		miwill_nl_notify_reset(MIWILL_RESET_REASON_TIMEOUT);
	}
	miwill_mframe(miw, MIWILL_MFRAME_TYPE_REQ);
	mod_timer(&miw->timer, jiffies + MIWILL_TIMER_IVAL);
}

static inline void miwill_timer_init(struct miwill *miw)
{
	timer_setup(&miw->timer, miwill_timer, 0);
}

static inline void miwill_timer_start(struct miwill *miw)
{
	if (!timer_pending(&miw->timer)) {
		miw->timer.expires = jiffies + (2 * HZ);
		add_timer(&miw->timer);
		miw->mframe_info.req_cnt = 0;
	}
}

static inline void miwill_timer_stop(struct miwill *miw)
{
	del_timer_sync(&miw->timer);
}

/*******************
 * Options handling
 *******************/

struct miwill_option_inst { /* One for each option instance */
	struct list_head list;
	struct miwill_option *option;
	struct miwill_option_inst_info info;
};

static struct miwill_option *__miwill_find_option(struct miwill *miwill,
					      const char *opt_name)
{
	struct miwill_option *option;

	list_for_each_entry(option, &miwill->option_list, list) {
		if (strcmp(option->name, opt_name) == 0)
			return option;
	}
	return NULL;
}

static int __miwill_option_inst_add(struct miwill *miw, struct miwill_option *option,
				  struct miwill_port *port)
{
	struct miwill_option_inst *opt_inst;
	unsigned int array_size;
	unsigned int i;
	int err;

	array_size = option->array_size;
	if (!array_size)
		array_size = 1; /* No array but still need one instance */

	for (i = 0; i < array_size; i++) {
		opt_inst = kmalloc(sizeof(*opt_inst), GFP_KERNEL);
		if (!opt_inst)
			return -ENOMEM;
		opt_inst->option = option;
		opt_inst->info.port = port;
		opt_inst->info.array_index = i;
		list_add_tail(&opt_inst->list, &miw->option_inst_list);
		if (option->init) {
			err = option->init(miw, &opt_inst->info);
			if (err)
				return err;
		}
	}
	return 0;
}

static void __miwill_option_inst_del(struct miwill_option_inst *opt_inst)
{
	list_del(&opt_inst->list);
	kfree(opt_inst);
}

static void __miwill_option_inst_del_option(struct miwill *miwill,
					  struct miwill_option *option)
{
	struct miwill_option_inst *opt_inst, *tmp;

	list_for_each_entry_safe(opt_inst, tmp, &miwill->option_inst_list, list) {
		if (opt_inst->option == option)
			__miwill_option_inst_del(opt_inst);
	}
}

static void __miwill_option_inst_del_port(struct miwill *miw,
					struct miwill_port *port)
{
	struct miwill_option_inst *opt_inst, *tmp;

	list_for_each_entry_safe(opt_inst, tmp, &miw->option_inst_list, list) {
		if (opt_inst->option->per_port &&
		    opt_inst->info.port == port)
			__miwill_option_inst_del(opt_inst);
	}
}

static int __miwill_option_inst_add_port(struct miwill *miw,
				       struct miwill_port *port)
{
	struct miwill_option *option;
	int err;

	list_for_each_entry(option, &miw->option_list, list) {
		if (!option->per_port)
			continue;
		err = __miwill_option_inst_add(miw, option, port);
		if (err)
			goto inst_del_port;
	}
	return 0;

inst_del_port:
	__miwill_option_inst_del_port(miw, port);
	return err;
}

static int __miwill_option_inst_add_option(struct miwill *miwill,
					 struct miwill_option *option)
{
	int err;

	if (!option->per_port) {
		err = __miwill_option_inst_add(miwill, option, NULL);
		if (err)
			goto inst_del_option;
	}
	return 0;

inst_del_option:
	__miwill_option_inst_del_option(miwill, option);
	return err;
}

int miwill_options_register(struct miwill *miwill,
				   const struct miwill_option *option,
				   size_t option_count)
{
	int i;
	struct miwill_option **dst_opts;
	int err;

	dst_opts = kcalloc(option_count, sizeof(struct miwill_option *),
			   GFP_KERNEL);
	if (!dst_opts)
		return -ENOMEM;
	for (i = 0; i < option_count; i++, option++) {
		if (__miwill_find_option(miwill, option->name)) {
			err = -EEXIST;
			goto alloc_rollback;
		}
		dst_opts[i] = kmemdup(option, sizeof(*option), GFP_KERNEL);
		if (!dst_opts[i]) {
			err = -ENOMEM;
			goto alloc_rollback;
		}
	}

	for (i = 0; i < option_count; i++) {
		err = __miwill_option_inst_add_option(miwill, dst_opts[i]);
		if (err)
			goto inst_rollback;
		list_add_tail(&dst_opts[i]->list, &miwill->option_list);
	}

	kfree(dst_opts);
	return 0;

inst_rollback:
	for (i--; i >= 0; i--)
		__miwill_option_inst_del_option(miwill, dst_opts[i]);

	i = option_count;
alloc_rollback:
	for (i--; i >= 0; i--)
		kfree(dst_opts[i]);

	kfree(dst_opts);
	return err;
}
EXPORT_SYMBOL(miwill_options_register);

void miwill_options_unregister(struct miwill *miwill,
				      const struct miwill_option *option,
				      size_t option_count)
{
	int i;

	for (i = 0; i < option_count; i++, option++) {
		struct miwill_option *del_opt;

		del_opt = __miwill_find_option(miwill, option->name);
		if (del_opt) {
			__miwill_option_inst_del_option(miwill, del_opt);
			list_del(&del_opt->list);
			kfree(del_opt);
		}
	}
}
EXPORT_SYMBOL(miwill_options_unregister);

// static int miwill_option_get(struct miwill *miwill,
// 			   struct miwill_option_inst *opt_inst,
// 			   struct miwill_gsetter_ctx *ctx)
// {
// 	if (!opt_inst->option->getter)
// 		return -EOPNOTSUPP;
// 	return opt_inst->option->getter(miwill, ctx);
// }

static int miwill_option_set(struct miwill *miwill,
			   struct miwill_option_inst *opt_inst,
			   struct miwill_gsetter_ctx *ctx)
{
	if (!opt_inst->option->setter)
		return -EOPNOTSUPP;
	return opt_inst->option->setter(miwill, ctx);
}

/* Options define */
static int miwill_option_enable_get(struct miwill *miwill,
				   struct miwill_gsetter_ctx *ctx)
{
	return 0;
}

static int miwill_option_enable_set(struct miwill *miwill,
				   struct miwill_gsetter_ctx *ctx)
{
	int ret;

	miwill_cmn_dbg("%s %d", __func__, ctx->data.bool_val);
	if (ctx->data.bool_val) {
		rtnl_lock();
		ret = dev_open(miwill->dev, NULL);
		rtnl_unlock();
	} else {
		rtnl_lock();
		ret = miwill_stop(miwill->dev);
		rtnl_unlock();
		miwill->dev->flags &= (~IFF_UP);
	}

	return ret;
}

static int miwill_option_mode_get(struct miwill *miwill, struct miwill_gsetter_ctx *ctx)
{
	ctx->data.str_val = miwill->mode->kind;
	return 0;
}

static int miwill_option_mode_set(struct miwill *miwill, struct miwill_gsetter_ctx *ctx)
{
	miwill_cmn_dbg("%s %s", __func__, ctx->data.str_val);
	return miwill_change_mode(miwill, ctx->data.str_val);
}

static int miwill_option_gw_addr_get(struct miwill *miwill, struct miwill_gsetter_ctx *ctx)
{
	return 0;
}

static int miwill_option_gw_addr_set(struct miwill *miwill, struct miwill_gsetter_ctx *ctx)
{
	miwill_cmn_dbg("%s "MAC_FMT, __func__, MAC_ARG(ctx->data.bin_val.ptr));
	memcpy(miwill->gw_addr, ctx->data.bin_val.ptr, ETH_ALEN);
	return 0;
}

static int miwill_option_port_5g_get(struct miwill *miwill, struct miwill_gsetter_ctx *ctx)
{
	return 0;
}

static int miwill_option_port_5g_set(struct miwill *miwill, struct miwill_gsetter_ctx *ctx)
{
	miwill_cmn_dbg("%s %s", __func__, ctx->data.str_val);
	return miwill_port_add_by_name(miwill, ctx->data.str_val, WIFI_FREQ_5G);
}

static int miwill_option_port_24g_get(struct miwill *miwill, struct miwill_gsetter_ctx *ctx)
{
	return 0;
}

static int miwill_option_port_24g_set(struct miwill *miwill, struct miwill_gsetter_ctx *ctx)
{
	miwill_cmn_dbg("%s %s", __func__, ctx->data.str_val);
	return miwill_port_add_by_name(miwill, ctx->data.str_val, WIFI_FREQ_24G);
}

static const struct miwill_option miwill_options[] = {
	{
		.name = "enable",
		.type = MIWILL_OPTION_TYPE_BOOL,
		//.per_port = true,
		.getter = miwill_option_enable_get,
		.setter = miwill_option_enable_set,
	},
	{
		.name = "mode",
		.type = MIWILL_OPTION_TYPE_STRING,
		.getter = miwill_option_mode_get,
		.setter = miwill_option_mode_set,
	},
	{
		.name = "gw_addr",
		.type = MIWILL_OPTION_TYPE_BINARY,
		.getter = miwill_option_gw_addr_get,
		.setter = miwill_option_gw_addr_set,
	},
	{
		.name = "port_5g",
		.type = MIWILL_OPTION_TYPE_STRING,
		.getter = miwill_option_port_5g_get,
		.setter = miwill_option_port_5g_set,
	},
	{
		.name = "port_24g",
		.type = MIWILL_OPTION_TYPE_STRING,
		.getter = miwill_option_port_24g_get,
		.setter = miwill_option_port_24g_set,
	},
};

int miwill_options_set(struct miwill *miw, unsigned char enable,
		char *mode, unsigned char *gw_addr, char *port_5g, char *port_24g) {
	struct miwill_option_inst *opt_inst;
	struct miwill_option *option;
	struct miwill_gsetter_ctx ctx;
	bool enable2 = false;
	int err = 0;

	if (enable != 0xFF) { /* enable is valid, set enable first */
		enable2 = enable ? true : false;
		list_for_each_entry(opt_inst, &miw->option_inst_list, list) {
			option = opt_inst->option;
			if (!strcmp(option->name, "enable")) {
				ctx.data.bool_val = enable2;
				err = miwill_option_set(miw, opt_inst, &ctx);
			}
		}
		if (!enable2)
			return err;
	}

	list_for_each_entry(opt_inst, &miw->option_inst_list, list) {
		option = opt_inst->option;
		if (mode && !strcmp(option->name, "mode")) {
			ctx.data.str_val = mode;
			err = miwill_option_set(miw, opt_inst, &ctx);
		} else if (port_5g && !strcmp(option->name, "gw_addr")) {
			ctx.data.bin_val.ptr = gw_addr;
			ctx.data.bin_val.len = ETH_ALEN;
			err = miwill_option_set(miw, opt_inst, &ctx);
		} else if (port_5g && !strcmp(option->name, "port_5g")) {
			ctx.data.str_val = port_5g;
			err = miwill_option_set(miw, opt_inst, &ctx);
		} else if (port_24g && !strcmp(option->name, "port_24g")) {
			ctx.data.str_val = port_24g;
			err = miwill_option_set(miw, opt_inst, &ctx);
		}
	}
	return err;
}

#define MIWILL_ADD_NEW_PACK 0
#if MIWILL_ADD_NEW_PACK
/*************************
 * packet type definition
 *************************/

int miwill_rcv(struct sk_buff *skb, struct net_device *dev,
	       struct packet_type *ptype, struct net_device *orig_dev)
{
	/*TODO: add actions*/
	return RX_HANDLER_PASS;
}

static struct packet_type miwill_packet_type __read_mostly = {
	.type = cpu_to_be16(ETH_P_MIWILL),
	.func = miwill_rcv,
};
#endif

/*
 * Port Handling
 */

static bool miwill_port_find(const struct miwill *miw,
			     const struct miwill_port *port)
{
	struct miwill_port *cur;
	list_for_each_entry(cur, &miw->port_list, list)
	if (cur == port)
	    return true;
	return false;
}

static void miwill_lower_state_changed(struct miwill_port *port)
{
	struct netdev_lag_lower_state_info info;

	info.link_up = port->state.linkup;
	info.tx_enabled = miwill_port_enabled(port);
	netdev_lower_state_changed(port->dev, &info);
}

static void miwill_queue_override_port_add(struct miwill *miw,
					 struct miwill_port *port);
static void miwill_port_enable(struct miwill *miw,
			       struct miwill_port *port)
{

	if (miwill_port_enabled(port))
		return;
	port->index = miw->en_port_count++;
	hlist_add_head_rcu(&port->hlist,
	miwill_port_index_hash(miw, port->index));
	miwill_queue_override_port_add(miw, port);
	if (MIWILL_MODE_OPS_EXIST(miw, port_enabled))
		miw->mode->ops->port_enabled(miw, port);
	miwill_lower_state_changed(port);
}

static void __reconstruct_port_hlist(struct miwill *miw, int rm_index)
{
	int i;
	struct miwill_port *port;

	for (i = rm_index + 1; i < miw->en_port_count; i++) {
		port = miwill_get_port_by_index_rcu(miw, i);
		hlist_del_rcu(&port->hlist);
		port->index--;
		hlist_add_head_rcu(&port->hlist,
				   miwill_port_index_hash(miw, port->index));
	}
}

static int miwill_port_enter(struct miwill *miw, struct miwill_port *port)
{
	int err;

	dev_hold(miw->dev);

	if (MIWILL_MODE_OPS_EXIST(miw, port_enter)) {
		err = miw->mode->ops->port_enter(miw, port);
		if (err) {
			miwill_cmn_err("%s %s", __func__, port->dev->name);
			goto err_port_enter;
		}
	}

	return 0;
err_port_enter:
	dev_put(miw->dev);
	return err;
}

static void miwill_port_leave(struct miwill *miw, struct miwill_port *port)
{
	if (MIWILL_MODE_OPS_EXIST(miw, port_leave))
		miw->mode->ops->port_leave(miw, port);

	dev_put(miw->dev);
}

/*
 * Default Rx path frame handler for MiWill port device
 */

/* note: already called with rcu_read_lock */
static rx_handler_result_t miwill_handle_frame(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	struct miwill_port *port;
	struct miwill *miw;
	struct MIWILL_ethhdr *miw_ethhdr;
	rx_handler_result_t res;


	skb = skb_share_check(skb, GFP_ATOMIC);
	if (!skb)
	    return RX_HANDLER_CONSUMED;

	*pskb = skb;

	port = miwill_port_get_rcu(skb->dev);
	miw = port->miwill;
	miw_ethhdr = (struct MIWILL_ethhdr *)(skb->data - ETH_HLEN);

	if (!miwill_port_enabled(port)) {
		miwill_cmn_dbg("%s port:%s disabled.", __func__, port->dev->name);
	    res = RX_HANDLER_EXACT;
	} else if (ntohs(miw_ethhdr->h_miwill_proto) != ETH_P_MIWILL) {
#ifdef DEBUG_PACKET
		miwill_cmn_dbg("%s protocol:%04X pass.",
			__func__, ntohs(miw_ethhdr->h_miwill_proto));
#endif
		res = RX_HANDLER_PASS;
	} else if (miw_ethhdr->header.mode == CMD_MODE) {
		res = miwill_mframe_handler(skb);
	} else if (miw_ethhdr->header.mode == PASS_MODE) {
		res = miwill_pass_receive(miw, port, skb);
	} else {
		res = miw->mode->ops->receive(miw, port, skb);
	}

	/* DO: follow team_handle_frame */
	if (res == RX_HANDLER_ANOTHER) {
		struct miwill_pcpu_stats *pcpu_stats;

		pcpu_stats = this_cpu_ptr(miw->pcpu_stats);
		u64_stats_update_begin(&pcpu_stats->syncp);
		pcpu_stats->rx_packets++;
		pcpu_stats->rx_bytes += skb->len;
		if (skb->pkt_type == PACKET_MULTICAST)
			pcpu_stats->rx_multicast++;
		u64_stats_update_end(&pcpu_stats->syncp);

		skb->dev = miw->dev;
		skb->mark |= 0x01000000; // set MARK for minet receive stat.
	} else if (res == RX_HANDLER_EXACT)
		this_cpu_inc(miw->pcpu_stats->rx_nohandler);
	  else if(res == RX_HANDLER_PASS)
		this_cpu_inc(miw->pcpu_stats->rx_nohandler);
	  else
		this_cpu_inc(miw->pcpu_stats->rx_dropped);
	return res;
}


/****************
 * Mode handling
 ****************/

static LIST_HEAD(mode_list);
static DEFINE_SPINLOCK(mode_list_lock);

struct miwill_mode_item {
	struct list_head list;
	const struct miwill_mode *mode;
};

static rx_handler_result_t miwill_dummy_receive(struct miwill *miw,
					      struct miwill_port *port,
					      struct sk_buff *skb)
{
	return RX_HANDLER_ANOTHER;
}

static bool miwill_dummy_transmit(struct miwill *miw, struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
	return false;
}

static const struct miwill_mode_ops dummy_mode_ops = {
	.receive = miwill_dummy_receive,
	.transmit = miwill_dummy_transmit,
};

static const struct miwill_mode dummy_mode = {
	.kind = "dummy",
	.owner = THIS_MODULE,
	.ops = &dummy_mode_ops
};

static bool miwill_is_mode_set(struct miwill *miw)
{
	return (miw->mode && (miw->mode != &dummy_mode));
}

static bool is_good_mode_name(const char *name)
{
	while (*name != '\0') {
		if (!isalpha(*name) && !isdigit(*name) && *name != '_')
			return false;
		name++;
	}
	return true;
}

static struct miwill_mode_item *__find_mode(const char *kind)
{
	struct miwill_mode_item *mitem;

	list_for_each_entry(mitem, &mode_list, list) {
		if (strcmp(mitem->mode->kind, kind) == 0)
			return mitem;
	}
	return NULL;
}

static void miwill_mode_put(const struct miwill_mode *mode)
{
	module_put(mode->owner);
}

static const struct miwill_mode *miwill_mode_get(const char *kind)
{
	struct miwill_mode_item *mitem;
	const struct miwill_mode *mode = NULL;

	if (!try_module_get(THIS_MODULE))
		return NULL;

	spin_lock(&mode_list_lock);
	mitem = __find_mode(kind);
	if (!mitem) {
		spin_unlock(&mode_list_lock);
		request_module("miwill-mode-%s", kind);
		spin_lock(&mode_list_lock);
		mitem = __find_mode(kind);
	}
	if (mitem) {
		mode = mitem->mode;
		if (!try_module_get(mode->owner))
			mode = NULL;
	}

	spin_unlock(&mode_list_lock);
	module_put(THIS_MODULE);
	return mode;
}
static void miwill_set_no_mode(struct miwill *miw);

/*
 * We can benefit from the fact that it's ensured no port is present
 * at the time of mode change. Therefore no packets are in fly so there's no
 * need to set mode operations in any special way.
 */
static int __miwill_change_mode(struct miwill *miwill,
			      const struct miwill_mode *new_mode)
{
	int err = 0;

	if (!new_mode)
		return 0;

	miwill_set_no_mode(miwill);
	if (new_mode->ops->init && (err = new_mode->ops->init(miwill))) {
		miwill_mode_put(new_mode);
		return err;
	}
	miwill->mode = new_mode;

	return 0;
}

int miwill_change_mode(struct miwill *miw, const char *kind)
{
	const struct miwill_mode *new_mode;
	int err;

	/*
	if (list_empty(&miw->port_list)) {
		miwill_cmn_err("%s no port", __func__);
		return -EBUSY;
	}
	*/

	if (miwill_is_mode_set(miw) && strcmp(miw->mode->kind, kind) == 0) {
		miwill_cmn_err("%s %s, do nothing", __func__, kind);
		return -EINVAL;
	}

	new_mode = miwill_mode_get(kind);
	if (!new_mode) {
		miwill_cmn_err("%s %s, not found", __func__, kind);
		return -EINVAL;
	}

	err = __miwill_change_mode(miw, new_mode);
	if (err) {
		miwill_cmn_err("%s %s, err", __func__, kind);
		return err;
	}

	miwill_cmn_dbg("%s %s, ok", __func__, kind);
	return 0;
}
EXPORT_SYMBOL(miwill_change_mode);

int miwill_mode_register(const struct miwill_mode *mode)
{
	int err = 0;
	struct miwill_mode_item *mitem;

	if (!is_good_mode_name(mode->kind))
		return -EINVAL;

	mitem = kmalloc(sizeof(*mitem), GFP_KERNEL);
	if (!mitem)
		return -ENOMEM;

	spin_lock(&mode_list_lock);
	if (__find_mode(mode->kind)) {
		err = -EEXIST;
		kfree(mitem);
		spin_unlock(&mode_list_lock);
		return err;
	}
	mitem->mode = mode;
	list_add_tail(&mitem->list, &mode_list);
	spin_unlock(&mode_list_lock);

	miwill_cmn_dbg("miwill_mode_register %s", mode->kind);
	return err;
}
EXPORT_SYMBOL(miwill_mode_register);

void miwill_mode_unregister(const struct miwill_mode *mode)
{
	struct miwill_mode_item *mitem;

	spin_lock(&mode_list_lock);
	mitem = __find_mode(mode->kind);
	if (mitem) {
		list_del_init(&mitem->list);
		kfree(mitem);
	}
	spin_unlock(&mode_list_lock);
	miwill_cmn_dbg("miwill %s-mode module cleanup",mode->kind);
}
EXPORT_SYMBOL(miwill_mode_unregister);

static void miwill_set_no_mode(struct miwill *miw)
{
	miwill_cmn_dbg("%s", __func__);
	if (miwill_is_mode_set(miw)) {
		if (MIWILL_MODE_OPS_EXIST(miw, exit))
			miw->mode->ops->exit(miw);
		miwill_mode_put(miw->mode);
	}
	miw->mode = &dummy_mode;
}

/*
* PASS MODE
* This mode don't need registration.
* The packets transmitted through the miw interface
* work in the registered mode by default.
* Only packets with special mark follow this mode.
*/

static inline bool is_pass_mode_skb(struct sk_buff *skb)
{
	return ((skb->mark & 0x0CE00000) == 0x8000000);
}

static rx_handler_result_t miwill_pass_receive(struct miwill *miw,
					      struct miwill_port *port,
					      struct sk_buff *skb)
{
	struct MIWILL_ethhdr *miw_ethhdr;
#ifdef DEBUG_PACKET
	miwill_cmn_dbg("%s before", __func__);
	skb_dump(KERN_ERR,skb,true);
#endif
	miw_ethhdr = (struct MIWILL_ethhdr *)(skb->data-ETH_HLEN);
#ifdef DEBUG_PACKET
	miwill_cmn_dbg("%s dmac "MAC_FMT" ", __func__, MAC_ARG(miw_ethhdr->h_dest));
	miwill_cmn_dbg("%s smac "MAC_FMT" ", __func__, MAC_ARG(miw_ethhdr->h_source));
#endif
	skb->protocol = miw_ethhdr->header.encap_proto;
	memmove(skb->data - ETH_ALEN, miw_ethhdr, 2 * ETH_ALEN);
	skb_pull(skb, MIWILL_HEAD_LEN);
	skb->mac_header += MIWILL_HEAD_LEN;
	skb->network_header += MIWILL_HEAD_LEN;
	skb->transport_header += MIWILL_HEAD_LEN;
	skb_reset_mac_len(skb);
#ifdef DEBUG_PACKET
	miwill_cmn_dbg("%s after", __func__);
	skb_dump(KERN_ERR,skb,true);
#endif
	return RX_HANDLER_ANOTHER;
}

static bool miwill_pass_transmit(struct miwill *miw, struct sk_buff *skb)
{
	struct miwill_port *port;
	struct MIWILL_ethhdr *miw_ethhdr;
	int ret = 0;

#ifdef DEBUG_PACKET
	miwill_cmn_dbg("%s", __func__);
	skb_dump(KERN_ERR,skb,true);
#endif
	skb = miwill_add_head(skb, miw);
	if(unlikely(!skb)){
		miwill_cmn_err("%s, miwill_add_head", __func__);
		return false;
	}

	miw_ethhdr = (struct MIWILL_ethhdr *)(skb->data);
	miw_ethhdr->header.mode = PASS_MODE;
	memset(miw_ethhdr->header.option.raw, 0x00, MIWILL_OPT_LEN);

	list_for_each_entry(port, &miw->port_list, list) {
		if (unlikely(!port))
			goto drop;

		if(!miwill_port_txable(port))
			continue;

		ret = miwill_dev_queue_xmit(miw, port, skb);
		if (ret) {
			miwill_cmn_err("%s, miwill_dev_queue_xmit %s, ret:%d",
				__func__, port->dev->name, ret);
			return false;
		} else {
			return true;
		}
	}
drop:
	dev_kfree_skb_any(skb);
	return false;
}

/*************************************
 * Multiqueue Tx port select override
 *************************************/

static int miwill_queue_override_init(struct miwill *miw)
{
	struct list_head *listarr;
	unsigned int queue_cnt = miw->dev->num_tx_queues - 1;
	unsigned int i;

	if (!queue_cnt)
		return 0;
	listarr = kmalloc_array(queue_cnt, sizeof(struct list_head),
				GFP_KERNEL);
	if (!listarr)
		return -ENOMEM;
	miw->qom_lists = listarr;
	for (i = 0; i < queue_cnt; i++)
		INIT_LIST_HEAD(listarr++);
	return 0;
}

static struct list_head *__miwill_get_qom_list(struct miwill *miw, u16 queue_id)
{
	return &miw->qom_lists[queue_id - 1];
}

static bool miwill_queue_override_port_has_gt_prio_than(struct miwill_port *port,
						      struct miwill_port *cur)
{
	if (port->priority < cur->priority)
		return true;
	if (port->priority > cur->priority)
		return false;
	if (port->index < cur->index)
		return true;
	return false;
}

static void __miwill_queue_override_port_add(struct miwill *miw,
					   struct miwill_port *port)
{
	struct miwill_port *cur;
	struct list_head *qom_list;
	struct list_head *node;

	if (!port->queue_id)
		return;
	qom_list = __miwill_get_qom_list(miw, port->queue_id);
	node = qom_list;
	list_for_each_entry(cur, qom_list, qom_list) {
		if (miwill_queue_override_port_has_gt_prio_than(port, cur))
			break;
		node = &cur->qom_list;
	}
	list_add_tail_rcu(&port->qom_list, node);
}

static void __miwill_queue_override_enabled_check(struct miwill *miw)
{
	struct miwill_port *port;
	bool enabled = false;

	list_for_each_entry(port, &miw->port_list, list) {
		if (port->queue_id) {
			enabled = true;
			break;
		}
	}
	if (enabled != miw->queue_override_enabled)
		miw->queue_override_enabled = enabled;

	return;
}

static void __miwill_queue_override_port_del(struct miwill *miw,
					   struct miwill_port *port)
{
	if (!port->queue_id)
		return;
	list_del_rcu(&port->qom_list);
}

static void miwill_queue_override_port_add(struct miwill *miw,
					 struct miwill_port *port)
{
	__miwill_queue_override_port_add(miw, port);
	__miwill_queue_override_enabled_check(miw);
}

static void miwill_queue_override_port_del(struct miwill *miw,
					 struct miwill_port *port)
{
	__miwill_queue_override_port_del(miw, port);
	__miwill_queue_override_enabled_check(miw);
}

static void miwill_port_disable(struct miwill *miw,
			      struct miwill_port *port)
{
	if (!miwill_port_enabled(port))
		return;
	if (MIWILL_MODE_OPS_EXIST(miw, port_disabled))
		miw->mode->ops->port_disabled(miw, port);
	hlist_del_rcu(&port->hlist);
	__reconstruct_port_hlist(miw, port->index);
	port->index = -1;
	miw->en_port_count--;
	miwill_queue_override_port_del(miw, port);
	miwill_lower_state_changed(port);
}

#define MIWILL_ENC_FEATURES	(NETIF_F_HW_CSUM | NETIF_F_SG | \
				 NETIF_F_RXCSUM | NETIF_F_ALL_TSO)

static void __miwill_compute_features(struct miwill *miw)
{
	struct miwill_port *port;
	//TODO : vlan relates
	// netdev_features_t vlan_features = TEAM_VLAN_FEATURES &
	// 				  NETIF_F_ALL_FOR_ALL;
	netdev_features_t enc_features  = MIWILL_ENC_FEATURES;
	unsigned short max_hard_header_len = ETH_HLEN;
	unsigned int dst_release_flag = IFF_XMIT_DST_RELEASE |
					IFF_XMIT_DST_RELEASE_PERM;

	list_for_each_entry(port, &miw->port_list, list) {
		// vlan_features = netdev_increment_features(vlan_features,
		// 			port->dev->vlan_features,
		// 			TEAM_VLAN_FEATURES);
		enc_features =
			netdev_increment_features(enc_features,
						  port->dev->hw_enc_features,
						  MIWILL_ENC_FEATURES);


		dst_release_flag &= port->dev->priv_flags;
		if (port->dev->hard_header_len > max_hard_header_len)
			max_hard_header_len = port->dev->hard_header_len;
	}

	//team->dev->vlan_features = vlan_features;
	miw->dev->hw_enc_features = enc_features | NETIF_F_GSO_ENCAP_ALL |
				     NETIF_F_HW_VLAN_CTAG_TX |
				     NETIF_F_HW_VLAN_STAG_TX |
				     NETIF_F_GSO_UDP_L4;
	miw->dev->hard_header_len = max_hard_header_len;

	miw->dev->priv_flags &= ~IFF_XMIT_DST_RELEASE;
	if (dst_release_flag == (IFF_XMIT_DST_RELEASE | IFF_XMIT_DST_RELEASE_PERM))
		miw->dev->priv_flags |= IFF_XMIT_DST_RELEASE;
}
/*
 * following team_port helpers
 * helpers for miwill_port_dev setup
 */
static struct miwill_port *miwill_port_get_rtnl(const struct net_device *dev)
{
	struct miwill_port *port;
	port = rtnl_dereference(dev->rx_handler_data);
	return netif_is_miwill_port(dev) ? port : NULL;
}

static int miwill_port_set_miwill_src_id(struct miwill *miwill,
					 struct miwill_port *port)
{
	/* the src_id is the last 2-byte of miwill dev_addr */
	if (likely(miwill->dev && miwill->dev->dev_addr)) {
		memcpy(port->src_id,
		       miwill->dev->dev_addr + ETH_ALEN - MIWILL_SRC_ID_LEN,
		       MIWILL_SRC_ID_LEN);
		return 0;
	} else {
		miwill_cmn_dbg("failed to set src_id of port_dev %s.",
			       port->dev->name);
		return -1;
	}
}

int miwill_modeop_port_enter(struct miwill *miwill, struct miwill_port *port)
{
	/*
	 * using the last 2-byte of miwill mac_addr
	 * as the src_id attribute of the  port_dev
	 * this is the miwill way to mark port_dev device;
	 */
	return miwill_port_set_miwill_src_id(miwill, port);
}
EXPORT_SYMBOL(miwill_modeop_port_enter);

#ifdef CONFIG_NET_POLL_CONTROLLER
static void miwill_port_disable_netpoll(struct miwill_port *port)
{
	struct netpoll *np = port->np;

	if (!np)
		return;
	port->np = NULL;

	__netpoll_free(np);
}

static int __miwill_port_enable_netpoll(struct miwill_port *port)
{
	struct netpoll *np;
	int err;

	np = kzalloc(sizeof(*np), GFP_KERNEL);
	if (!np)
		return -ENOMEM;

	err = __netpoll_setup(np, port->dev);
	if (err) {
		kfree(np);
		return err;
	}
	port->np = np;
	return err;
}

static void miwill_poll_controller(struct net_device *dev)
{
}

static void __miwill_netpoll_cleanup(struct miwill *miw)
{
	struct miwill_port *port;

	list_for_each_entry(port, &miw->port_list, list)
		miwill_port_disable_netpoll(port);
}

static void miwill_netpoll_cleanup(struct net_device *dev)
{
	struct miwill *miw = netdev_priv(dev);

	spin_lock_bh(&miw->lock);
	__miwill_netpoll_cleanup(team);
	spin_unlock_bh(&miw->lock);
}

static int miwill_netpoll_setup(struct net_device *dev,
			      struct netpoll_info *npifo)
{
	struct miwill *miw = netdev_priv(dev);
	struct miwill_port *port;
	int err = 0;

	spin_lock_bh(&miw->lock);
	list_for_each_entry(port, &miw->port_list, list) {
		err = __miwill_port_enable_netpoll(port);
		if (err) {
			__miwill_netpoll_cleanup(team);
			break;
		}
	}
	spin_unlock_bh(&miw->lock);
	return err;
}
#else
static void miwill_port_disable_netpoll(struct miwill_port *port)
{
}
#endif

static int miwill_dev_type_check_change(struct net_device *dev,
				      struct net_device *port_dev);
static void __miwill_port_change(struct miwill_port *port, bool linkup);

static int __miwill_port_add(struct miwill *miwill, struct net_device *port_dev,
			   miwill_wlan_freq_t freq, struct netlink_ext_ack *extack)
{
	struct net_device *dev = miwill->dev;
	struct miwill_port *port;
	char *portname = port_dev->name;

	int err;
	if (port_dev->flags & IFF_LOOPBACK) {
		NL_SET_ERR_MSG(extack,
			"Loopback device can't be added as a miwill port");
		miwill_cmn_err("%s %s, loopback device", __func__, portname);
		return -EINVAL;
	}

	if (netif_is_miwill_port(port_dev)) {
		NL_SET_ERR_MSG(extack,
			       "Device is already a port of a miwill device");
		miwill_cmn_err("%s %s, added already", __func__, portname);
		return -EBUSY;
	}

	if (dev == port_dev) {
		NL_SET_ERR_MSG(extack,
			       "cannot enslave miwill device to itself");
		miwill_cmn_err("%s %s, don't add itself", __func__, portname);
		return -EINVAL;
	}

	err = miwill_dev_type_check_change(dev, port_dev);
	if (err)
		return err;

	port = kzalloc(sizeof(struct miwill_port) +
			       miwill->mode->port_priv_size,
		       GFP_KERNEL);
	if (!port)
		return -ENOMEM;

	port->dev = port_dev;
	port->miwill = miwill;
	port->frequency = freq;
	INIT_LIST_HEAD(&port->qom_list); /*TODO:  meaning ?? */

	/* adopt the miwill mtu to its port dev */
	port->orig.mtu = port_dev->mtu;
	err = dev_set_mtu(port_dev, dev->mtu);
	if (err) {
		miwill_cmn_err("%s %s, dev_set_mtu", __func__, portname);
		goto err_set_mtu;
	}

	err = miwill_port_enter(miwill, port);
	if (err) {
		miwill_cmn_err("%s %s, miwill_port_enter", __func__, portname);
		goto err_port_enter;
	}

	if (!(dev->features & NETIF_F_LRO)) {
		miwill_cmn_err("%s need to disable lro", __func__);
		//dev_disable_lro(port_dev);
	}

	port_dev->priv_flags |= IFF_MIWILL_PORT;
	err = netdev_rx_handler_register(port_dev, miwill_handle_frame, port);
	if (err) {
		miwill_cmn_err("%s %s, netdev_rx_handler_register", __func__, portname);
		goto err_handler_register;
	}

	err = __miwill_option_inst_add_port(miwill, port);
	if (err) {
		miwill_cmn_err("%s %s, __miwill_option_inst_add_port", __func__, portname);
		goto err_option_port_add;
	}

	/* set allmulti level to new port */
	if (dev->flags & IFF_ALLMULTI) {
		miwill_cmn_err("%s need to set allmulti level", __func__);
		/*
		err = dev_set_allmulti(port_dev, 1);
		if (err) {
			if (dev->flags & IFF_PROMISC)
				dev_set_promiscuity(port_dev, -1);
			goto err_set_slave_promisc;
		}
		*/
	}
	netif_addr_lock_bh(dev);
	dev_uc_sync_multiple(port_dev, dev);
	dev_mc_sync_multiple(port_dev, dev);
	netif_addr_unlock_bh(dev);

	port->index = -1;
	spin_lock_bh(&miwill->lock);
	list_add_tail_rcu(&port->list, &miwill->port_list);
	miwill_port_enable(miwill, port);
	__miwill_port_change(port, !!netif_oper_up(port_dev));
	spin_unlock_bh(&miwill->lock);

	miwill_cmn_info("%s %s, ok", __func__, portname);
	return 0;

/*
err_set_slave_promisc:
	__miwill_option_inst_del_port(miwill, port);
*/
err_option_port_add:
	netdev_rx_handler_unregister(port_dev);

err_handler_register:
	port_dev->priv_flags &= ~IFF_MIWILL_PORT;
	miwill_port_disable_netpoll(port);

//err_dev_open:
	miwill_port_leave(miwill, port);

err_port_enter:
	dev_set_mtu(port_dev, port->orig.mtu);

err_set_mtu:
	kfree(port);
	return err;
}

static int __miwill_port_del(struct miwill *miwill, struct net_device *port_dev)
{
	struct net_device *dev = miwill->dev;
	struct miwill_port *port;
	char *portname = port_dev->name;

	port = miwill_port_get_rtnl(port_dev);
	if (!port || !miwill_port_find(miwill, port)) {
		miwill_cmn_err("%s %s, not a port", __func__, portname);
		return -ENOENT;
	}

	spin_lock_bh(&miwill->lock);
	miwill_port_disable(miwill, port);
	list_del_rcu(&port->list);
	spin_unlock_bh(&miwill->lock);

	/*
	if (dev->flags & IFF_PROMISC)
		dev_set_promiscuity(port_dev, -1);
	if (dev->flags & IFF_ALLMULTI)
		dev_set_allmulti(port_dev, -1);
	*/

	port_dev->priv_flags &= ~IFF_MIWILL_PORT;
	netdev_rx_handler_unregister(port_dev);
	miwill_port_disable_netpoll(port);
	dev_uc_unsync(port_dev, dev);
	dev_mc_unsync(port_dev, dev);
	//dev_close(port_dev);
	miwill_port_leave(miwill, port);

	__miwill_option_inst_del_port(miwill, port);
	__miwill_port_change(port, false);

	dev_set_mtu(port_dev, port->orig.mtu);
	kfree_rcu(port, rcu);
	__miwill_compute_features(miwill);

	miwill_cmn_info("%s %s, ok", __func__, portname);
	return 0;
}

static u16 miwill_select_queue(struct net_device *dev, struct sk_buff *skb,
			     struct net_device *sb_dev)
{
	/*
	 * This helper function exists to help dev_pick_tx get the correct
	 * destination queue.  Using a helper function skips a call to
	 * skb_tx_hash and will put the skbs in the queue we expect on their
	 * way down to the team driver.
	 */
	u16 txq = skb_rx_queue_recorded(skb) ? skb_get_rx_queue(skb) : 0;
	/*
	 * Save the original txq to restore before passing to the driver
	 */
	qdisc_skb_cb(skb)->slave_dev_queue_mapping = skb->queue_mapping;

	if (unlikely(txq >= dev->real_num_tx_queues)) {
		do {
			txq -= dev->real_num_tx_queues;
		} while (txq >= dev->real_num_tx_queues);
	}
	return txq;
}

static void miwill_change_rx_flags(struct net_device *dev, int change)
{
	struct miwill *miw = netdev_priv(dev);
	struct miwill_port *port;
	int inc;
	rcu_read_lock();
	list_for_each_entry_rcu (port, &miw->port_list, list) {
		if (change & IFF_PROMISC) {
			inc = dev->flags & IFF_PROMISC ? 1 : -1;
			miwill_cmn_err("%s %s, need to set promiscuity level",
				__func__, dev->name);
			//dev_set_promiscuity(port->dev, inc);
		}
		if (change & IFF_ALLMULTI) {
			inc = dev->flags & IFF_ALLMULTI ? 1 : -1;
			miwill_cmn_err("%s %s, need to set allmulti level",
				__func__, dev->name);
			//dev_set_allmulti(port->dev, inc);
		}
	}
	rcu_read_unlock();
}

static void miwill_set_rx_mode(struct net_device *dev)
{
	struct miwill *miw = netdev_priv(dev);
	struct miwill_port *port;
	rcu_read_lock();
	list_for_each_entry_rcu (port, &miw->port_list, list) {
		dev_uc_sync_multiple(port->dev, dev);
		dev_mc_sync_multiple(port->dev, dev);
	}
	rcu_read_unlock();
}

static int miwill_change_mtu(struct net_device *dev, int new_mtu)
{
	struct miwill *miw = netdev_priv(dev);
	struct miwill_port *port;
	int err;
	spin_lock_bh(&miw->lock);
	list_for_each_entry (port, &miw->port_list, list) {
		err = dev_set_mtu(port->dev, new_mtu);
		if (err) {
			miwill_cmn_err("%s %s, dev_set_mtu", __func__, port->dev->name);
			goto unwind;
		}
	}
	spin_unlock_bh(&miw->lock);

	dev->mtu = new_mtu;
	return 0;

unwind:
	list_for_each_entry_continue_reverse (port, &miw->port_list, list)
		dev_set_mtu(port->dev, dev->mtu);
	spin_unlock_bh(&miw->lock);
	return err;
}

static void miwill_get_stats64(struct net_device *dev,
			       struct rtnl_link_stats64 *stats)
{
	struct miwill *miw = netdev_priv(dev);
	struct miwill_pcpu_stats *p;
	u64 rx_packets, rx_bytes, rx_multicast, tx_packets, tx_bytes;
	u32 rx_dropped = 0, tx_dropped = 0, rx_nohandler = 0;
	unsigned int start;
	int i;
	for_each_possible_cpu(i) {
		p = per_cpu_ptr(miw->pcpu_stats, i);
		do {
			start = u64_stats_fetch_begin(&p->syncp);
			rx_packets	= p->rx_packets;
			rx_bytes	= p->rx_bytes;
			rx_multicast	= p->rx_multicast;
			tx_packets	= p->tx_packets;
			tx_bytes	= p->tx_bytes;
		} while (u64_stats_fetch_retry(&p->syncp, start));

		stats->rx_packets	+= rx_packets;
		stats->rx_bytes		+= rx_bytes;
		stats->multicast	+= rx_multicast;
		stats->tx_packets	+= tx_packets;
		stats->tx_bytes		+= tx_bytes;
		/*
		 * rx_dropped, tx_dropped & rx_nohandler are u32,
		 * updated without syncp protection.
		 */
		rx_dropped	+= p->rx_dropped;
		tx_dropped	+= p->tx_dropped;
		rx_nohandler	+= p->rx_nohandler;
	}
	stats->rx_dropped	= rx_dropped;
	stats->tx_dropped	= tx_dropped;
	stats->rx_nohandler	= rx_nohandler;
}

static bool miwill_queue_override_transmit(struct miwill *miw, struct sk_buff *skb)
{
	struct list_head *qom_list;
	struct miwill_port *port;
	return false;
	if (!miw->queue_override_enabled || !skb->queue_mapping)
		return false;
	qom_list = __miwill_get_qom_list(miw, skb->queue_mapping);
	list_for_each_entry_rcu(port, qom_list, qom_list) {
		if (!miwill_dev_queue_xmit(miw, port, skb))
			return true;
	}
	return false;
}

static netdev_tx_t miwill_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct miwill *miw = netdev_priv(dev);
	struct miwill_pcpu_stats *pcpu_stats = this_cpu_ptr(miw->pcpu_stats);
	unsigned int skb_len = skb->len;
	bool tx_success = false;

	tx_success = miwill_queue_override_transmit(miw, skb);
	if (!tx_success) {
#ifdef DEBUG_PACKET
		miwill_cmn_dbg("%s, start mode transmit", __func__);
#endif
		if (is_pass_mode_skb(skb)) {
			tx_success = miwill_pass_transmit(miw, skb);
		} else {
			tx_success = miw->mode->ops->transmit(miw, skb);
		}
	}
	if (tx_success) {
		u64_stats_update_begin(&pcpu_stats->syncp);
		pcpu_stats->tx_packets++;
#ifdef DEBUG_PACKET
		miwill_cmn_dbg("%s, tx_packets:%ld", __func__, pcpu_stats->tx_packets);
#endif
		pcpu_stats->tx_bytes += skb_len;
		u64_stats_update_end(&pcpu_stats->syncp);
	} else {
		this_cpu_inc(miw->pcpu_stats->tx_dropped);
	}

	return NETDEV_TX_OK;
}

static int miwill_dev_init(struct net_device *dev)
{
	int err;

	struct miwill *miw = netdev_priv(dev);
	miw->dev = dev;
	spin_lock_init(&miw->lock);
	miwill_set_no_mode(miw);
	INIT_LIST_HEAD(&miw->port_list);
	INIT_LIST_HEAD(&miw->option_list);
	INIT_LIST_HEAD(&miw->option_inst_list);
	miw->pcpu_stats = netdev_alloc_pcpu_stats(struct miwill_pcpu_stats);

	err = miwill_options_register(miw, miwill_options, ARRAY_SIZE(miwill_options));
	if (err) {
		miwill_cmn_err("%s miwill_options_register err:%d", __func__, err);
		return err;
	}

	if (!miw->pcpu_stats)
		return -ENOMEM;
	miwill_queue_override_init(miw);
	miwill_timer_init(miw);
	return 0;
}

static void miwill_dev_uninit(struct net_device *dev)
{
	struct miwill *miw = netdev_priv(dev);
	miwill_options_unregister(miw, miwill_options, ARRAY_SIZE(miwill_options));
	free_percpu(dev->dstats);
}

static int miwill_open(struct net_device *dev)
{
	struct miwill *miw = netdev_priv(dev);

	miwill_cmn_dbg("%s", __func__);
	netif_carrier_on(dev);
	miwill_timer_start(miw);
	return 0;
}

static int miwill_stop(struct net_device *dev)
{
	struct miwill *miw = netdev_priv(dev);
	struct miwill_port *port, *tmp;

	miwill_cmn_dbg("%s", __func__);
	miwill_mframe(miw, MIWILL_MFRAME_TYPE_RESET);
	netif_carrier_off(dev);
	list_for_each_entry_safe(port, tmp, &miw->port_list, list) {
		miwill_port_del(miw->dev, port->dev);
	}
	miwill_set_no_mode(miw);
	miwill_timer_stop(miw);
	return 0;
}

static netdev_features_t miwill_fix_features(struct net_device *dev,
					   netdev_features_t features)
{
	struct miwill_port *port;
	struct miwill *miw = netdev_priv(dev);
	netdev_features_t mask;
	mask = features;
	features &= ~NETIF_F_ONE_FOR_ALL;
	features |= NETIF_F_ALL_FOR_ALL;

	rcu_read_lock();
	list_for_each_entry_rcu(port, &miw->port_list, list) {
		features = netdev_increment_features(features,
						     port->dev->features,
						     mask);
	}
	rcu_read_unlock();

	features = netdev_add_tso_features(features, mask);
	return features;
}

static int miwill_change_carrier(struct net_device *dev, bool new_carrier)
{
	miwill_cmn_dbg("%s %s %d", __func__, dev->name, new_carrier);
	if (new_carrier)
		netif_carrier_on(dev);
	else
		netif_carrier_off(dev);
	return 0;
}

/*
* The caller must hold the RTNL lock.
*/
static int miwill_port_add(struct net_device *dev, struct net_device *port_dev,
			    miwill_wlan_freq_t freq, struct netlink_ext_ack *extack)
{
	struct miwill *miw = netdev_priv(dev);
	int err;

	err = __miwill_port_add(miw, port_dev, freq, extack);
	if (err)
		return err;

	netdev_change_features(dev);
	return err;
}

/*
* The caller must hold the RTNL lock.
*/
static int miwill_port_del(struct net_device *dev, struct net_device *port_dev)
{
	struct miwill *miw = netdev_priv(dev);
	int err;

	err = __miwill_port_del(miw, port_dev);
	if (err)
		return err;

	netdev_change_features(dev);
	return err;
}

struct net_device * miwill_name_to_dev(const char *name)
{
	struct net_device *dev;
	dev = dev_get_by_name(&init_net, name);
	if (!dev) {
		miwill_cmn_dbg("get %s dev err.", name);
		return NULL;
	}
	return dev;
}

static int miwill_port_add_by_name(struct miwill *miw, const char *port_name,
		miwill_wlan_freq_t freq)
{
	struct net_device *port_dev;
	int ret = 0;

	miwill_cmn_dbg("%s %s %d",__func__, port_name, freq);
	port_dev = miwill_name_to_dev(port_name);
	if (!port_dev) {
		return -ENXIO;
	}

	rtnl_lock();
	ret = miwill_port_add(miw->dev, port_dev, freq, NULL);
	rtnl_unlock();
	if ((!ret) && (port_dev->dev_addr)) {
		miwill_cmn_dbg("%s addr:"MAC_FMT, port_name, MAC_ARG(port_dev->dev_addr));
		if (freq == WIFI_FREQ_5G) {
			memcpy(miw->mframe_info.wlan_mac_5g, port_dev->dev_addr, ETH_ALEN);
		} else if (freq == WIFI_FREQ_24G) {
			memcpy(miw->mframe_info.wlan_mac_24g, port_dev->dev_addr, ETH_ALEN);
		}
	}

	dev_put(port_dev);
	return ret;
}

/*
static int miwill_port_del_by_name(struct miwill *miw, const char *port_name)
{
	struct net_device *port_dev;
	int ret = 0;

	miwill_cmn_dbg("%s %s",__func__, port_name);
	port_dev = miwill_name_to_dev(port_name);
	if (!port_dev) {
		return -ENXIO;
	}

	ret = miwill_port_del(miw->dev, port_dev);

	dev_put(port_dev);
	return ret;
}
*/

static const struct net_device_ops miwill_netdev_ops = {
	.ndo_init = miwill_dev_init,
	.ndo_uninit = miwill_dev_uninit,
	.ndo_open = miwill_open,
	.ndo_stop = miwill_stop,
	.ndo_start_xmit = miwill_xmit,
	.ndo_select_queue = miwill_select_queue,
	.ndo_change_rx_flags = miwill_change_rx_flags,
	.ndo_set_rx_mode = miwill_set_rx_mode,
	.ndo_set_mac_address = eth_mac_addr,
	.ndo_change_mtu = miwill_change_mtu,
	.ndo_get_stats64 = miwill_get_stats64,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller = miwill_poll_controller,
	.ndo_netpoll_setup = miwill_netpoll_setup,
	.ndo_netpoll_cleanup = miwill_netpoll_cleanup,
#endif
	.ndo_fix_features	= miwill_fix_features,
	.ndo_change_carrier = miwill_change_carrier,
	.ndo_features_check = passthru_features_check,
};

static void miwill_get_drvinfo(struct net_device *dev,
			       struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, DRV_NAME, sizeof(info->driver));
	strlcpy(info->version, DRV_VERSION, sizeof(info->version));
}

static const struct ethtool_ops miwill_ethtool_ops = {
	.get_drvinfo = miwill_get_drvinfo,
	.get_ts_info = ethtool_op_get_ts_info,
};

static void miwill_setup_by_port(struct net_device *dev,
			       struct net_device *port_dev)
{
	dev->header_ops	= port_dev->header_ops;
	dev->type = port_dev->type;
	dev->hard_header_len = port_dev->hard_header_len;
	dev->needed_headroom = port_dev->needed_headroom;
	dev->addr_len = port_dev->addr_len;
	dev->mtu = port_dev->mtu;
	memcpy(dev->broadcast, port_dev->broadcast, port_dev->addr_len);
	eth_hw_addr_inherit(dev, port_dev);
}

static int miwill_dev_type_check_change(struct net_device *dev,
				      struct net_device *port_dev)
{
	struct miwill *miw = netdev_priv(dev);
	char *portname = port_dev->name;
	int err;

	if (dev->type == port_dev->type)
		return 0;

	if (!list_empty(&miw->port_list)) {
		miwill_cmn_err("%s %s, not first port", __func__, portname);
		return -EBUSY;
	}

	err = call_netdevice_notifiers(NETDEV_PRE_TYPE_CHANGE, dev);
	err = notifier_to_errno(err);
	if (err) {
		miwill_cmn_err("%s %s, change device type", __func__, portname);
		return err;
	}

	miwill_setup_by_port(dev, port_dev);
	call_netdevice_notifiers(NETDEV_POST_TYPE_CHANGE, dev);
	return 0;
}

/******************
 * Change checkers
 ******************/

static void __miwill_port_state_refresh(struct miwill_port *port, bool linkup)
{
	int err;

	if (port->state.linkup != linkup) {
		port->state.linkup = linkup;
		miwill_lower_state_changed(port);
	}

	if (linkup) {
		struct ethtool_link_ksettings ecmd;

		err = __ethtool_get_link_ksettings(port->dev, &ecmd);
		if (!err) {
			port->state.speed = ecmd.base.speed;
			port->state.duplex = ecmd.base.duplex;
		}
	} else {
		port->state.speed = 0;
		port->state.duplex = 0;
	}
}

static void __miwill_carrier_check(struct miwill *miw)
{
	struct miwill_port *port;
	bool miwill_linkup = false;

	list_for_each_entry(port, &miw->port_list, list) {
		if (port->state.linkup) {
			miwill_linkup = true;
			break;
		}
	}

	miwill_cmn_dbg("%s %d", __func__, miwill_linkup);
	if (miwill_linkup)
		netif_carrier_on(miw->dev);
	else
		netif_carrier_off(miw->dev);
}

static void __miwill_port_change(struct miwill_port *port, bool linkup)
{
	__miwill_port_state_refresh(port, linkup);
	__miwill_carrier_check(port->miwill);
}

static void miwill_port_change(struct miwill_port *port, bool linkup)
{
	struct miwill *miw = port->miwill;

	spin_lock_bh(&miw->lock);
	__miwill_port_change(port, linkup);
	spin_unlock_bh(&miw->lock);
}

/*
 * miwill device notifier event handler
 */
static const char *__netdev_cmd_to_name(enum netdev_cmd cmd)
{
#define N(val) 						\
	case NETDEV_##val:				\
		return "NETDEV_" __stringify(val);
	switch (cmd) {
	N(UP) N(DOWN) N(REBOOT) N(CHANGE) N(REGISTER) N(UNREGISTER)
	N(CHANGEMTU) N(CHANGEADDR) N(GOING_DOWN) N(CHANGENAME) N(FEAT_CHANGE)
	N(BONDING_FAILOVER) N(PRE_UP) N(PRE_TYPE_CHANGE) N(POST_TYPE_CHANGE)
	N(POST_INIT) N(RELEASE) N(NOTIFY_PEERS) N(JOIN) N(CHANGEUPPER)
	N(RESEND_IGMP) N(PRECHANGEMTU) N(CHANGEINFODATA) N(BONDING_INFO)
	N(PRECHANGEUPPER) N(CHANGELOWERSTATE) N(UDP_TUNNEL_PUSH_INFO)
	N(UDP_TUNNEL_DROP_INFO) N(CHANGE_TX_QUEUE_LEN)
	N(CVLAN_FILTER_PUSH_INFO) N(CVLAN_FILTER_DROP_INFO)
	N(SVLAN_FILTER_PUSH_INFO) N(SVLAN_FILTER_DROP_INFO)
	N(PRE_CHANGEADDR)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0))
        N(OFFLOAD_XSTATS_ENABLE)
	N(OFFLOAD_XSTATS_DISABLE) N(OFFLOAD_XSTATS_REPORT_USED)
	N(OFFLOAD_XSTATS_REPORT_DELTA)
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 17))
        N(PRE_UNINIT)
	N(XDP_FEAT_CHANGE)
#endif
        }
#undef N
	return "UNKNOWN_NETDEV_EVENT";
}

static int miwill_device_event(struct notifier_block *unused,
			       unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct miwill_port *port;

	if(!netif_is_miwill_port(dev)){
		return NOTIFY_DONE;
	}

	port = miwill_port_get_rtnl(dev);
	if (!port)
		return NOTIFY_DONE;

	miwill_cmn_dbg("%s %s", __func__, __netdev_cmd_to_name(event));
	miwill_cmn_dbg("%s dev->operstate:%d", __func__, dev->operstate);

	switch (event) {
	case NETDEV_UP:
	case NETDEV_CHANGE:
		if (netif_oper_up(dev)){
			miwill_port_change(port, true);
		}
		break;
	case NETDEV_DOWN:
		miwill_port_change(port, false);
		break;
	case NETDEV_REGISTER:
		break;
	case NETDEV_UNREGISTER:
		miwill_port_change(port, false);
		miwill_port_del(port->miwill->dev, dev);
		break;
	default :
		break;
	}
	return NOTIFY_DONE;
}

static struct notifier_block miwill_notifier_block __read_mostly = {
	.notifier_call = miwill_device_event,
};

static void miwill_setup(struct net_device *dev);
static int miwill_validate(struct nlattr *tb[], struct nlattr *data[],
			   struct netlink_ext_ack *extack);

static struct rtnl_link_ops miwill_link_ops __read_mostly = {
	.kind = DRV_NAME,
	.setup = miwill_setup,
	.validate = miwill_validate,
};

static void miwill_setup(struct net_device *dev)
{
	eth_hw_addr_random(dev);
	ether_setup(dev);
	dev->max_mtu = ETH_MAX_MTU;
	dev->netdev_ops = &miwill_netdev_ops;
	dev->ethtool_ops = &miwill_ethtool_ops;
	dev->rtnl_link_ops = &miwill_link_ops;
	dev->needs_free_netdev = true;
	//TODO		dev->priv_destructor = miwill_destructor;

	dev->flags |= IFF_NOARP;
	dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);

	dev->priv_flags |= IFF_MIWILL;
	dev->priv_flags &= ~(IFF_XMIT_DST_RELEASE | IFF_TX_SKB_SHARING);
	dev->priv_flags |= IFF_NO_QUEUE;
	/*
	 * Indicate we support unicast address filtering. That way core won't
	 * bring us to promisc mode in case a unicast addr is added.
	 * Let this up to underlay drivers.
	 */
	dev->priv_flags |= IFF_UNICAST_FLT | IFF_LIVE_ADDR_CHANGE;

	dev->features |= NETIF_F_LLTX;
	dev->features |= NETIF_F_GRO;
	/* Don't allow team devices to change network namespaces. */
	dev->features |= NETIF_F_NETNS_LOCAL;

	dev->hw_features |= NETIF_F_GSO_ENCAP_ALL | NETIF_F_GSO_UDP_L4;
	dev->features |= dev->hw_features;
}

static int miwill_validate(struct nlattr *tb[], struct nlattr *data[],
			   struct netlink_ext_ack *extack)
{
	if (tb[IFLA_ADDRESS]) {
		if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
			return -EINVAL;
		if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
			return -EADDRNOTAVAIL;
	}
	return 0;
}

/* Number of miwill devices to be set up by this module. */
static int miwill_cnt = 1;
module_param(miwill_cnt, int, 00400);
MODULE_PARM_DESC(miwill_cnt, "Number of miwill devices");

static int __init miwill_init_one(void)
{
	struct net_device *dev_miwill;
	int err;

	dev_miwill = alloc_netdev_mqs(sizeof(struct miwill), "miw_oem%d",
		NET_NAME_ENUM, miwill_setup,16,16);
	if (unlikely(!dev_miwill))
		return -ENOMEM;

	rtnl_lock();
	err = register_netdevice(dev_miwill);
	rtnl_unlock();
	if (err < 0)
		goto err;
	return 0;

err:
	free_netdev(dev_miwill);
	return err;
}

extern int minet_init(void);
extern void minet_cleanup(void);

/* Module init and cleanup */
static int __init miwill_init_module(void)
{
	int i = 0;
	int errno = 0;

#if MIWILL_ADD_NEW_PACK
	dev_add_pack(&miwill_packet_type);
#endif
	register_netdevice_notifier(&miwill_notifier_block);

	errno =rtnl_link_register(&miwill_link_ops);
	if (errno < 0)
		goto err_rtnl_reg;

	for (; i < miwill_cnt && !errno; i++) {
		errno = miwill_init_one();
		cond_resched();
	}
	if (errno < 0)
		goto err_nl_init;

	errno = miwill_nl_init();
	if (errno)
		goto err_nl_init;

	errno = minet_init();
	if (errno)
		goto err_nl_init;

	miwill_cmn_dbg("%s ok", __func__);
	return 0;

err_nl_init:
	rtnl_link_unregister(&miwill_link_ops);
err_rtnl_reg:
	unregister_netdevice_notifier(&miwill_notifier_block);
#if MIWILL_ADD_NEW_PACK
	dev_remove_pack(&miwill_packet_type);
#endif
	return errno;
}

static void __exit miwill_cleanup_module(void)
{
	minet_cleanup();
	miwill_nl_finit();
	rtnl_link_unregister(&miwill_link_ops);
	unregister_netdevice_notifier(&miwill_notifier_block);
#if MIWILL_ADD_NEW_PACK
	dev_remove_pack(&miwill_packet_type);
#endif
	miwill_cmn_dbg("%s ok", __func__);
}

module_init(miwill_init_module);
module_exit(miwill_cleanup_module);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Minjun Xi <ximinjun@xiaomi.com>");
MODULE_DESCRIPTION("driver for MiWiLL-capable device");
MODULE_ALIAS_RTNL_LINK(DRV_NAME);
MODULE_VERSION(DRV_VERSION);
