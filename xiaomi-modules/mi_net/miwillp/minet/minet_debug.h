/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __MINET_DEBUG_H_
#define __MINET_DEBUG_H_

#define minet_dbg(fmt, ...)                                               \
	printk(KERN_DEBUG "miwill-minet-d: " fmt, ##__VA_ARGS__)

#define minet_info(fmt, ...)                                               \
	printk(KERN_INFO "miwill-minet-i: " fmt, ##__VA_ARGS__)

#define minet_err(fmt, ...)                                               \
	printk(KERN_ERR "miwill-minet-e: " fmt, ##__VA_ARGS__)

#define IPv4_FMT "%d.%d.%d.%d"
#define IPv4_ARG(x)                                                             \
	((u8 *)(x))[0], ((u8 *)(x))[1], ((u8 *)(x))[2], ((u8 *)(x))[3]

#define IPv6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#define IPv6_ARG(x)                                                             \
	ntohs((x.s6_addr16)[0]), ntohs((x.s6_addr16)[1]), 						\
	ntohs((x.s6_addr16)[2]), ntohs((x.s6_addr16)[3]),						\
	ntohs((x.s6_addr16)[4]), ntohs((x.s6_addr16)[5]), 						\
	ntohs((x.s6_addr16)[6]), ntohs((x.s6_addr16)[7])

#if 0
#define miwill_genl_dbg(fmt, ...)                                               \
	printk(KERN_DEBUG "minet_genl: " fmt, ##__VA_ARGS__)

#define miwill_genl_err(fmt, ...)                                               \
	printk(KERN_ERR "minet_genl: " fmt, ##__VA_ARGS__)

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
#endif

#endif /* __MIWILL_DEBUG_H__*/
