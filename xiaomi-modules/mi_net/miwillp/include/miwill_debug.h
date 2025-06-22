/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __MIWILL_DEBUG_H_
#define __MIWILL_DEBUG_H_

#define miwill_cmn_dbg(fmt, ...)                                               \
	printk(KERN_DEBUG "miwill-d: " fmt, ##__VA_ARGS__)

#define miwill_cmn_info(fmt, ...)                                               \
	printk(KERN_INFO "miwill-i: " fmt, ##__VA_ARGS__)

#define miwill_cmn_err(fmt, ...)                                               \
	printk(KERN_ERR "miwill-e: " fmt, ##__VA_ARGS__)

#define miwill_genl_dbg(fmt, ...)                                               \
	printk(KERN_DEBUG "miwill_genl: " fmt, ##__VA_ARGS__)

#define miwill_genl_err(fmt, ...)                                               \
	printk(KERN_ERR "miwill_genl: " fmt, ##__VA_ARGS__)

#define miw_netdev_dbg(miwill_dev, port_dev, fmt, ...)                         \
	netdev_dbg(miwill_dev, "(port %s): " fmt, (port_dev)->name,            \
		   ##__VA_ARGS__)

#define miw_netdev_err(miwill_dev, port_dev, fmt, ...)                         \
	netdev_err(miwill_dev, "(port %s): " fmt, (port_dev)->name,            \
		   ##__VA_ARGS__)

//#define DEBUG_PACKET
//#define DEBUG_SKB
#ifdef DEBUG_PACKET
#ifndef DEBUG_SKB
static inline void miwill_skb_dump(const char *level, const struct sk_buff *skb, bool full_pkt)
{
	//miwill_cmn_dbg("miwill_skb_dump");
	return;
}
#define skb_dump(level, skb, full_pkt)  miwill_skb_dump(level, skb, full_pkt)
#endif
#endif

#endif /* __MIWILL_DEBUG_H__*/
