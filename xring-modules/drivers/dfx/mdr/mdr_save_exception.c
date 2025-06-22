// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "mdr_print.h"
#include <linux/kernel.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <asm/ioctls.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include "mdr.h"
#include "mdr_subsys_rst.h"

#define NETLINK_MDR	23

static struct sock *g_netlinkfd;
static u32 g_user_pid;
static bool g_sublogsave_flag;
static bool g_productlogsave_flag;
static bool g_subsys_logsave_flag;

bool is_mdr_nlpid_valid(void)
{
	if ((get_pid_task(find_vpid(g_user_pid), PIDTYPE_PID) == NULL) || (g_user_pid == 0)) {
		pr_err("pid[%u] not exist or not inited\n", g_user_pid);
		return false;
	}

	return true;
}
EXPORT_SYMBOL(is_mdr_nlpid_valid);

int mdr_send_nl(void *info, u32 size)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	int retval = 0;

	if (info == NULL) {
		pr_err("mdr info is null\n");
		return -EINVAL;
	}

	if (!is_mdr_nlpid_valid()) {
		pr_err("send info = %s\n", (char *)info);
		dump_stack();
		return -EINVAL;
	}

	pr_info("info = 0x%p\n", info);

	/*
	 * Allocate a new skb
	 */
	skb = nlmsg_new(NLMSG_SPACE(size), GFP_ATOMIC);
	if (skb == NULL) {
		pr_err("allocate skb failed\n");
		return -ENOMEM;
	}

	pr_info("alloc new skb, data length = %d\n", size);

	/*
	 * Add a new Netlink message header to the Netlink message buffer 'skb' and return a
	 * pointer 'nlh' to that message header.
	 */
	nlh = nlmsg_put(skb, 0, 0, NETLINK_MDR, NLMSG_SPACE(size) - sizeof(struct nlmsghdr), 0);
	if (!nlh) {
		pr_err("nlmsg_put failed\n");
		nlmsg_free(skb);
		retval = -EMSGSIZE;
		goto exit;
	}

	/*
	 * Pointer to the message body after the Netlink message header, 'NLMSG_ DATA' is a macro
	 * definition used to calculate a pointer to the message body.
	 *
	 */
	memcpy(NLMSG_DATA(nlh), info, size);

	/*
	 * dst_group is a field in the 'netlink_skb_parma' structure used to specify the target
	 * group of the message. setting 'dst_group' group to 0 means sending messages to all
	 * listeners, rather than just specific groups.
	 *
	 */
	NETLINK_CB(skb).dst_group = 0;

	pr_info("skb->data:%s\n", (char *)NLMSG_DATA((struct nlmsghdr *)skb->data));

	/*
	 * 'MSG_DONTWAIT' is used to specify that the function should return immediately if the
	 * message cannot be sent, rather than blocking until it can be sent. The function returns
	 * the number of bytes sent, or an error code if an error occurs.
	 */
	retval = netlink_unicast(g_netlinkfd, skb, g_user_pid, MSG_DONTWAIT);
	if (retval < 0) {
		pr_err("sent data failed\n");
		goto exit;
	}

exit:
	pr_info("netlink_unicast return: %d\n", retval);
	return retval;
}
EXPORT_SYMBOL(mdr_send_nl);

static bool mdr_get_sublogsave_flag(void)
{
	return g_sublogsave_flag;
}

static void mdr_set_sublogsave_flag(bool flag)
{
	g_sublogsave_flag = flag;
}

static bool mdr_get_crashhistorysave_flag(void)
{
	return g_productlogsave_flag;
}

static void mdr_set_crashhistorysave_flag(bool flag)
{
	g_productlogsave_flag = flag;
}

static bool mdr_get_subsys_logsave_flag(void)
{
	return g_subsys_logsave_flag;
}

static void mdr_set_subsys_logsave_flag(bool flag)
{
	g_subsys_logsave_flag = flag;
}

/*
 * wait mdr logsave flag
 */
static void mdr_wait_common_logsave(void)
{
	u32 timeouts;

	for (timeouts = 0; timeouts < MSEC100_PER_5SEC; timeouts++) {
		msleep(100);
		if (!mdr_get_sublogsave_flag())
			continue;

		pr_info("get common logsave flag success\n");
		break;
	}

	pr_info("common logsave wait exit, flag is %d\n", mdr_get_sublogsave_flag());

	mdr_set_sublogsave_flag(false);
}

/*
 * wait mdr logsave flag and product logsave flag
 */
static void mdr_wait_all_flag(void)
{
	u32 timeouts;

	for (timeouts = 0; timeouts < MSEC100_PER_5SEC; timeouts++) {
		msleep(100);
		if (!mdr_get_sublogsave_flag())
			continue;
		if (!mdr_get_crashhistorysave_flag())
			continue;

		pr_info("get logsave flag success\n");
		break;
	}

	pr_info("sublogsave wait exit, xring log save flag is %d, product log save flag is %d\n", mdr_get_sublogsave_flag(), mdr_get_crashhistorysave_flag());

	mdr_set_sublogsave_flag(false);
	mdr_set_crashhistorysave_flag(false);
}

/*
 * Add synchronization mechanism, and wait if log writing is not completed.
 * The maximum waiting time is 5s.
 *
 */
void mdr_wait_subsys_logsave(void)
{
	u32 timeouts;

	for (timeouts = 0; timeouts < MSEC100_PER_5SEC; timeouts++) {
		msleep(100);
		if (!mdr_get_subsys_logsave_flag())
			continue;

		pr_info("get logsave flag success\n");
		break;
	}

	pr_info("subsys logsave wait exit, flag is %d\n", mdr_get_subsys_logsave_flag());

	mdr_set_subsys_logsave_flag(false);
}

void mdr_wait_logsave_before_reset(struct mdr_exception_info_s *e_info)
{
	if (check_subsys_sysrst_enable(e_info))
		mdr_wait_all_flag();
	else
		mdr_wait_common_logsave();
}

static void mdr_recv_nl(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	char *msg;

	if (skb == NULL) {
		pr_err("skb is NULL\n");
		return;
	}

	nlh = nlmsg_hdr(skb);
	msg = NLMSG_DATA(nlh);

	switch (nlh->nlmsg_flags) {
	case GET_PID_FLAG:
		g_user_pid = nlh->nlmsg_pid;
		pr_info("kernel recv message:%s get pid[%u]\n", msg, nlh->nlmsg_pid);
		break;
	case LOGSAVE_FLAG:
		pr_info("kernel recv message LOGSAVE_FLAG\n");
		g_sublogsave_flag = true;
		break;
	case LOGSAVE_FLAG_PRODUCT:
		pr_info("kernel recv message LOGSAVE_FLAG_PRODUCT\n");
		g_productlogsave_flag = true;
		break;
	case LOGSAVE_FLAG_SUBSYS:
		pr_info("kernel recv message LOGSAVE_FLAG_SUBSYS\n");
		g_subsys_logsave_flag = true;
		break;
	default:
		pr_err("nlmsg_flags is error\n");
	}
}

int mdr_netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = mdr_recv_nl,
	};

	g_netlinkfd = netlink_kernel_create(&init_net, NETLINK_MDR, &cfg);
	if (!g_netlinkfd) {
		pr_err("create netlink socket error\n");
		return -1;
	}

	pr_info(">>> success\n");
	return 0;
}

void netlink_exit(void)
{
	if (g_netlinkfd)
		netlink_kernel_release(g_netlinkfd);
	pr_info("<<< exit\n");
}
