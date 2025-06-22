/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/time.h>
#include <linux/timer.h>
#include "include/if_miwill.h"
#include "include/miwill_proto.h"
#include "include/miwill_debug.h"

#define TIME_OUT (HZ)
#define DUPLI_TABLE_LEN 4
#define MIWILL_SEQ_NUM 65536



DECLARE_BITMAP(dupi_bitmap1, MIWILL_SEQ_NUM);
DECLARE_BITMAP(dupi_bitmap2, MIWILL_SEQ_NUM);
DECLARE_BITMAP(dupi_bitmap3, MIWILL_SEQ_NUM);
DECLARE_BITMAP(dupi_bitmap4, MIWILL_SEQ_NUM);
static DEFINE_SPINLOCK(bitmap_lock);

static struct red_priv{
	unsigned long sent_packets;
	unsigned long* dupi_table[DUPLI_TABLE_LEN];
	unsigned long timer_number;
	struct timer_list timer;
}red_priv_data;

static inline struct red_priv *get_red_priv(struct miwill *miw)
{
	return (struct red_priv *) miw->mode->priv_data;
}

bool is_duplicate(struct miwill *miw,unsigned long seq_num)
{
	int i = 0;
	size_t set_bit_num;
	bool res = false;
	struct red_priv *data = get_red_priv(miw);
	for(;i < DUPLI_TABLE_LEN;i++)
		res = res || test_and_clear_bit(seq_num,data->dupi_table[i]);
	if(!res){
		set_bit_num = (DUPLI_TABLE_LEN - data->timer_number%DUPLI_TABLE_LEN)%DUPLI_TABLE_LEN;
		set_bit(seq_num,data->dupi_table[set_bit_num]);
#ifdef DEBUG_PACKET
		miwill_cmn_dbg("%s %lu, first", __func__, seq_num);
#endif
		return false;
	}
#ifdef DEBUG_PACKET
	miwill_cmn_dbg("%s %lu, second", __func__, seq_num);
#endif
	return true;
}

static bool red_transmit(struct miwill *miw, struct sk_buff *skb)
{
	struct miwill_port *port;
	struct sk_buff *skb2;
	struct MIWILL_ethhdr *miw_ethhdr;
	struct red_priv* data = get_red_priv(miw);
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
	miw_ethhdr->header.mode = REDUD_MODE;
	miw_ethhdr->header.option.redud.sub_type = 0x00;
	miw_ethhdr->header.option.redud.seq_num = htons(data->sent_packets);

	spin_lock_bh(&miw->lock);
	list_for_each_entry(port, &miw->port_list, list) {
		if (unlikely(!port))
			goto drop;

		if(!miwill_port_txable(port))
			continue;

		skb2 = skb_copy(skb, GFP_ATOMIC);
		if (unlikely(!skb2))
			continue;

		ret = miwill_dev_queue_xmit(miw, port, skb2);
		if (ret) {
			miwill_cmn_dbg("%s, miwill_dev_queue_xmit %s, ret:%d",
				__func__, port->dev ? port->dev->name : "port->dev is NULL", ret);
			continue;
		}
	}
	spin_unlock_bh(&miw->lock);
#ifdef DEBUG_PACKET
	miwill_cmn_dbg("%s sent_packets:%ld", __func__, data->sent_packets);
#endif
	data->sent_packets ++;
	dev_kfree_skb(skb);
	return true;
drop:
	spin_unlock_bh(&miw->lock);
	dev_kfree_skb_any(skb);
	return false;
}

rx_handler_result_t red_receive(struct miwill *miw,struct miwill_port *port,struct sk_buff *skb)
{
	struct MIWILL_ethhdr *miw_ethhdr;
	unsigned long cur_packets;

#ifdef DEBUG_PACKET
	miwill_cmn_dbg("%s before", __func__);
	skb_dump(KERN_ERR,skb,true);
#endif
	miw_ethhdr = (struct MIWILL_ethhdr *)(skb->data-ETH_HLEN);
	cur_packets = ntohs(miw_ethhdr->header.option.redud.seq_num);
	if(is_duplicate(miw, cur_packets%MIWILL_SEQ_NUM)) {
		dev_kfree_skb(skb);
		return RX_HANDLER_CONSUMED;
	}
#ifdef DEBUG_PACKET
	miwill_cmn_dbg("%s dmac "MAC_FMT" ", __func__, MAC_ARG(miw_ethhdr->h_dest));
	miwill_cmn_dbg("%s smac "MAC_FMT" ", __func__, MAC_ARG(miw_ethhdr->h_source));
	miwill_cmn_dbg("%s seq %04x", __func__, ntohs(miw_ethhdr->header.option.redud.seq_num));
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

void timer_callback(struct timer_list* tm)
{
	size_t clear_bit_num;
	struct red_priv *data = from_timer(data,tm,timer);
	spin_lock(&bitmap_lock);
	clear_bit_num = (DUPLI_TABLE_LEN-1-(data->timer_number)%DUPLI_TABLE_LEN)%DUPLI_TABLE_LEN;
	bitmap_zero(data->dupi_table[clear_bit_num],MIWILL_SEQ_NUM);
	data->timer_number++;
	spin_unlock(&bitmap_lock);

	mod_timer(tm,jiffies+TIME_OUT/DUPLI_TABLE_LEN);
}

void timer_exit(struct miwill *miw)
{
	struct red_priv* data = get_red_priv(miw);
	del_timer(&data->timer);
}

static void red_exit(struct miwill *miw)
{
	int i = 0;
	struct red_priv *data = get_red_priv(miw);
	for(;i<DUPLI_TABLE_LEN;i++){
		bitmap_zero(data->dupi_table[i],MIWILL_SEQ_NUM);
	}
	timer_exit(miw);
	miwill_cmn_dbg("%s", __func__);
}

static int red_init(struct miwill *miw);
static const struct miwill_mode_ops red_mode_ops = {
	.init			= red_init,
	.exit			= red_exit,
	.receive		= red_receive,
	.transmit		= red_transmit,
	.port_enter = miwill_modeop_port_enter,
};

static const struct miwill_mode red_mode = {
	.kind = "redudancy",
	.owner = THIS_MODULE,
	.ops = &red_mode_ops,
	.priv_data = (void *)&red_priv_data,
};

void timer_init(struct miwill *miw)
{
	struct red_priv *data = (struct red_priv *)red_mode.priv_data;
	timer_setup(&data->timer,timer_callback,0);
	data->timer.expires = jiffies + TIME_OUT/DUPLI_TABLE_LEN;
	add_timer(&data->timer);
	data->timer_number = 0;
}

static int red_init(struct miwill *miw)
{
	int i = 0;
	struct red_priv *data = (struct red_priv *)red_mode.priv_data;
	data->sent_packets = 0;
	data->dupi_table[0] = dupi_bitmap1;
	data->dupi_table[1] = dupi_bitmap2;
	data->dupi_table[2] = dupi_bitmap3;
	data->dupi_table[3] = dupi_bitmap4;
	for(;i<DUPLI_TABLE_LEN;i++)
		bitmap_zero(data->dupi_table[i],MIWILL_SEQ_NUM);

	timer_init(miw);
	miwill_cmn_dbg("%s", __func__);
	return 0;
}

static int __init miwill_red_init_module(void)
{
	return miwill_mode_register(&red_mode);
}

static void __exit miwill_red_cleanup_module(void)
{
	miwill_mode_unregister(&red_mode);
}

module_init(miwill_red_init_module);
module_exit(miwill_red_cleanup_module);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Minjun Xi <ximinjun@xiaomi.com> ");
MODULE_DESCRIPTION("redudancy mode support for MiWiLL protocol");
