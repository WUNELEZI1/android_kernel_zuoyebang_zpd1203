// SPDX-License-Identifier: GPL-2.0+
/*
 * hypsys netlink
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/socket.h>
#include <net/netlink.h>
#include <net/net_namespace.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>

#include "hypsys_netlink.h"

static struct sock *hypsys_nlsk = NULL;
static int target_pid = 0;

static int hypsys_socket_unicast(struct report_event *event);

static bool inline event_type_is_ivaild(enum report_type type)
{
	if (type < CMA_REPORT || type > MAX_REPORT)
		return false;
	return true;
}

static bool inline event_is_ivaild(struct report_event *event)
{
	if (!event)
		return false;

	if(!event_type_is_ivaild(event->msg_type))
		return false;

	if (event->msg_buf == NULL || event->msg_len <= 0)
		return false;

	return true;
}

struct report_event *hypsys_event_alloc(enum report_type type)
{
	struct report_event * event = NULL;

	if (!event_type_is_ivaild(type)) {
		hypsys_err("alloc event type invaild: %d\n", type);
		return NULL;
	}

	event = (struct report_event *)vmalloc(sizeof(struct report_event));
	if (!event) {
		hypsys_err("%s alloc report message fail\n", current->comm);
		return NULL;
	}
	memset(event, 0, sizeof(struct report_event));
	event->msg_type = type;

	event->msg_buf = vmalloc(HYPSYS_MAX_BUF_SIZE);
	if (!event->msg_buf) {
		hypsys_err("alloc msg_buf fail, free event\n");
		vfree(event);
		return NULL;
	}
	memset(event->msg_buf, 0, HYPSYS_MAX_BUF_SIZE);
	event->msg_len += snprintf(event->msg_buf, FORMAT_COMMA_BRACE, "{");

	return event;
}
EXPORT_SYMBOL(hypsys_event_alloc);

void hypsys_event_destroy(struct report_event *event)
{
	if (!event) {
		hypsys_err("destroy invaild event\n");
		return;
	}
	if (event->msg_buf)
		vfree(event->msg_buf);
	vfree(event);
}
EXPORT_SYMBOL(hypsys_event_destroy);

bool hypsys_event_report(struct report_event *event)
{

	if (event == NULL){
		hypsys_err("event is NULL, %d\n", current->pid);
		return false;
	}
	if (!event_is_ivaild(event)) {
		hypsys_err("invaild event\n");
		// hypsys_event_destroy(event);
		return false;
	}

	event->msg_len += snprintf((event->msg_buf + event->msg_len),
								FORMAT_COMMA_BRACE, "}");

	hypsys_socket_unicast(event);

	return true;
}
EXPORT_SYMBOL(hypsys_event_report);

int hypsys_event_add_str(struct report_event *event,
							const char *key, const char *value)
{
	int last_len  = 0;
	int len;
	int json_len = FORMAT_QUOTES_SIZE *2 + FORMAT_COLON_SIZE;

	if (!event) {
		hypsys_err("add invaild event\n");
		return -1;
	}
	if (!event_type_is_ivaild(event->msg_type)) {
		hypsys_err("invaild mesg type\n");
		return -EINVAL;
	}

	if (event->msg_buf == NULL) {
		hypsys_err("add int: invaild msg_buf\n");
		return -EINVAL;
	}

	if (event->msg_len != 1)
		json_len += FORMAT_COMMA_SIZE;

	last_len = HYPSYS_MAX_BUF_SIZE - event->msg_len;
	if (strlen(key) + strlen(value) + json_len >= last_len - 1) {
		hypsys_err("add str: msg size overload\n");
		return -EINVAL;
	}

	if (event->msg_len == 1 ) {
		len = snprintf((event->msg_buf + event->msg_len), last_len,
						"\"%s\":\"%s\"", key, value);
	} else {
		len = snprintf((event->msg_buf + event->msg_len), last_len,
						",\"%s\":\"%s\"", key, value);
	}

	if (len <= 0) {
		hypsys_err("add str: %s add %s fail\n", key, value);
		return -ENOMEM;
	}
	event->msg_len += len;

	return 0;
}
EXPORT_SYMBOL(hypsys_event_add_str);

static inline unsigned long get_integer_len(long num)
{
	unsigned long len = 0;

	if (num < 0)
		len++;

	for (; num != 0; ++len)
		num /= 10;

	return len;
}

int hypsys_event_add_int(struct report_event *event,
							const char *key, const long value)
{
	int last_len  = 0;
	int len = 0;
	int json_len = FORMAT_QUOTES_SIZE *2 + FORMAT_COLON_SIZE;

	if (!event) {
		hypsys_err("add invaild event\n");
		return -1;
	}
	if (!event_type_is_ivaild(event->msg_type)) {
		hypsys_err("invaild mesg type\n");
		return -EINVAL;
	}

	if (event->msg_buf == NULL) {
		hypsys_err("add int: invaild msg_buf\n");
		return -EINVAL;
	}

	if (event->msg_len != 1)
		json_len += FORMAT_COMMA_SIZE;


	last_len = HYPSYS_MAX_BUF_SIZE - event->msg_len;
	if (strlen(key) + get_integer_len(value) + json_len >= last_len - 1) {
		hypsys_err("add int: msg size overload\n");
		return -EINVAL;
	}

	if (event->msg_len == 1) {
		len = snprintf((event->msg_buf + event->msg_len), last_len,
						"\"%s\":\"%ld\"", key, value);
	} else {
		len = snprintf((event->msg_buf + event->msg_len), last_len,
						",\"%s\":\"%ld\"", key, value);
	}

	if (len <= 0) {
		hypsys_err("add int: %s add %ld fail\n", key, value);
		return -ENOMEM;
	}
	event->msg_len += len;

	return 0;
}
EXPORT_SYMBOL(hypsys_event_add_int);

static int hypsys_socket_unicast(struct report_event *event)
{
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb;

	if (!hypsys_nlsk || !target_pid) {
		hypsys_err("hypsys_nlsk or target_pid is NULL\n");
		return -EINVAL;
	}

	if (!event_is_ivaild(event)) {
		hypsys_err("invaild mesg type\n");
		return -EINVAL;
	}

	skb = nlmsg_new(event->msg_len, GFP_KERNEL);
	if (!skb) {
		hypsys_err("nlmsg_new failed\n");
		return -ENOMEM;
	}

	nlh = nlmsg_put(skb, 0, 0, event->msg_type, event->msg_len, 0);
	if (!nlh) {
		nlmsg_free(skb);
		hypsys_err("nlmsg_put failed\n");
		return -ENOMEM;
	}
	memset(NLMSG_DATA(nlh), 0, event->msg_len);
	memcpy(NLMSG_DATA(nlh), event->msg_buf, event->msg_len);
	nlmsg_end(skb, nlh);

	if (netlink_unicast(hypsys_nlsk, skb, target_pid, MSG_DONTWAIT) <= 0) {
		hypsys_err("netlink_unicast failed, type: %d\n", event->msg_type);
		return -EBUSY;
	}

	return 0;
}

static void hypsys_netklink_input(struct sk_buff *_skb)
{
	struct nlmsghdr *nlh;
	struct sk_buff *skb;

	skb = skb_get(_skb);
	if (skb->len >= NLMSG_SPACE(0)) {
		nlh = nlmsg_hdr(skb);
		target_pid = nlh->nlmsg_pid;
		hypsys_err("target_pid=%d, Netlink received msg payload:%s\n",
				target_pid, (char *)nlmsg_data(nlh));
	}
	nlmsg_free(skb);
	hypsys_err("hypsys_netklink_input\n");
}

static int hypsys_netlink_init(void)
{
	struct netlink_kernel_cfg netlink_cfg = {
		.input = hypsys_netklink_input,
	};

	hypsys_nlsk = netlink_kernel_create(&init_net, NETLINK_USERSOCK, &netlink_cfg);
	if (!hypsys_nlsk) {
		hypsys_err("create netlink socket failed\n");
		return -1;
	}

	hypsys_info("create netlink socket success\n");
	return 0;
}

module_init(hypsys_netlink_init);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("yangshiguang <yangshiguang@xiaomi.com>");
MODULE_DESCRIPTION("hypsys netlink");
