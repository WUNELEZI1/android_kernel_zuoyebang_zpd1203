// SPDX-License-Identifier: GPL-2.0
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>

#include "xring_meminfo_notify_lmkd.h"
#include "xring_mem_adapter.h"

#define NETLINK_LMKD_MAGIC 31
#define NETLINK_LMKD_INIT  0x11
#define NETLINK_LMKD_NOTIFY 0x12

static struct sock *sk;
static int lmkd_nl_pid;
static struct sk_buff *out_skb;
static unsigned int seq;
static unsigned int netlink_init = XR_MEM_INIT_IDLE;

static void xr_netlink_lmkd_ready(struct sk_buff *skb);

int xr_memory_netlink_init(void)
{
	struct netlink_kernel_cfg nlcfg = {
		.input = xr_netlink_lmkd_ready,
	};

	sk = netlink_kernel_create(&init_net, NETLINK_LMKD_MAGIC, &nlcfg);
	if (!sk) {
		xrmem_err("netlink_kernel_create error.\n");
		return -1;
	}
	xrmem_info("netlink_kernel_create success.\n");
	netlink_init = XR_MEM_INIT_DONE;
	return 0;
}
EXPORT_SYMBOL(xr_memory_netlink_init);

void xr_memory_netlink_exit(void)
{
	netlink_kernel_release(sk);
	xrmem_info("netlink_kernel_release.\n");
}
EXPORT_SYMBOL(xr_memory_netlink_exit);

static void xr_netlink_lmkd_ready(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;

	nlh = nlmsg_hdr(skb);
	switch (nlh->nlmsg_type) {
	case NETLINK_LMKD_INIT:
		lmkd_nl_pid = nlh->nlmsg_pid;
		xrmem_info("NETLINK_LMKD_INIT: lmkd_nl_pid: %d\n", lmkd_nl_pid);
		break;
	default:
		xrmem_info("type: %d not supported\n", nlh->nlmsg_type);
		break;
	}
}

int lmkd_notify(unsigned int value)
{
	void *out_payload = NULL;
	struct nlmsghdr *out_nlh;
	unsigned int portid = 0; // kernel
	int ret;

	out_skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_ATOMIC | __GFP_ZERO);
	if (!out_skb) {
		xrmem_err("NETLINK_LMKD_INIT nlmsg_new fail\n");
		return -1;
	}

	out_nlh = nlmsg_put(out_skb, portid, seq++, NETLINK_LMKD_NOTIFY, sizeof(value), 0);
	out_payload = nlmsg_data(out_nlh);
	strncpy(out_payload, (char *)&value, sizeof(value));
	ret = nlmsg_unicast(sk, out_skb, lmkd_nl_pid);
	xrmem_info("port:%d type: %d, value: %u, ret: %d\n",
		lmkd_nl_pid, NETLINK_LMKD_NOTIFY, value, ret);
	return 0;
}
EXPORT_SYMBOL(lmkd_notify);

module_param(netlink_init, uint, 0444);
