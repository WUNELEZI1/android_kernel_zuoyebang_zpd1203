// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * miwill_mode_random.c
 * The support for random mode
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include "include/if_miwill.h"
#include "include/miwill_debug.h"
#include "include/miwill_proto.h"
	unsigned long pac;
static bool rnd_transmit(struct miwill *miw, struct sk_buff *skb)
{
	struct miwill_port *port;
	int port_idx;

	port_idx = prandom_u32_max(miw->en_port_count);
	port = miwill_get_port_by_index_rcu(miw, port_idx);
	if (unlikely(!port))
		goto drop;
	port = miwill_get_first_port_txable_rcu(miw, port);
	if (unlikely(!port))
		goto drop;

	if (miwill_dev_queue_xmit(miw, port, skb))
		return false;

	return true;
drop:
	dev_kfree_skb_any(skb);
	return false;
}

/*
 * unlike team, miwill don't intend to change port mac address
 */

static const struct miwill_mode_ops rnd_mode_ops = {
	.transmit = rnd_transmit,
	.port_enter = miwill_modeop_port_enter,
};

static const struct miwill_mode rnd_mode = {
	.kind = "random",
	.owner = THIS_MODULE,
	.ops = &rnd_mode_ops,
};

static int __init miwill_random_init_module(void)
{
	return miwill_mode_register(&rnd_mode);
}

static void __exit miwill_random_cleanup_module(void)
{
	miwill_mode_unregister(&rnd_mode);
}

module_init(miwill_random_init_module);
module_exit(miwill_random_cleanup_module);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Minjun Xi <ximinjun@xiaomi.com> ");
MODULE_DESCRIPTION("random mode support for MiWiLL protocol");
