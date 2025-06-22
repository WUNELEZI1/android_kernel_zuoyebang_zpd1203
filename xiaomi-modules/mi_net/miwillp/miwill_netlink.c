#include <net/netlink.h>
#include <net/genetlink.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/socket.h>
#include <linux/rtnetlink.h>
#include <net/rtnetlink.h>

#include "include/if_miwill.h"
#include "include/miwill_netlink.h"
#include "include/miwill_debug.h"

static int miwill_nl_cmd_options_set(struct sk_buff *skb, struct genl_info *info);
static int miwill_nl_cmd_options_get(struct sk_buff *skb, struct genl_info *info);
static int minet_nl_cmd_set(struct sk_buff *skb, struct genl_info *info);

/* attribute policy */
static struct nla_policy miwill_genl_policy[MIWILL_ATTR_MAX + 1] = {
	[MIWILL_ATTR_UNSPEC]	     	= { .type = NLA_UNSPEC, },
	[MIWILL_ATTR_OPTION_MIW]    	= { .type = NLA_STRING },
	[MIWILL_ATTR_OPTION_ENABLE]  	= { .type = NLA_U8 },
	[MIWILL_ATTR_OPTION_MODE]    	= { .type = NLA_STRING },
	[MIWILL_ATTR_OPTION_GW_ADDR]    = { .type = NLA_BINARY },
	[MIWILL_ATTR_OPTION_PORT_5G]    = { .type = NLA_STRING },
	[MIWILL_ATTR_OPTION_PORT_24G]   = { .type = NLA_STRING },
	[MIWILL_ATTR_IP]    			= { .type = NLA_U32 },
	[MIWILL_ATTR_RESET]    			= { .type = NLA_U8 },
	[MINET_ATTR_COMMON]    			= { .type = NLA_UNSPEC },
};

/* operation definition Correspond the command echo to the specific handler */
static struct genl_ops miwill_nl_ops[] = {
	{
		.cmd = MIWILL_CMD_OPTIONS_SET,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = miwill_nl_cmd_options_set,
		.flags = GENL_ADMIN_PERM,
	},
	{
		.cmd = MIWILL_CMD_OPTIONS_GET,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = miwill_nl_cmd_options_get,
		.flags = GENL_ADMIN_PERM,
	},
	{
		.cmd = MINET_CMD_SET,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = minet_nl_cmd_set,
		.flags = GENL_ADMIN_PERM,
	},
};

static struct genl_multicast_group miwill_nl_mcgrps[] = {
		{ .name = MIWILL_GENERIC_EVENT_MCGRP_NAME, },
};

static struct genl_family miwill_nl_family __ro_after_init = {
	.name       = MIWILL_GENERIC_NAME,
	.version    = MIWILL_GENERIC_VERSION,
	.maxattr    = MIWILL_ATTR_MAX,
	.policy     = miwill_genl_policy,
	.module		= THIS_MODULE,
	.ops		= miwill_nl_ops,
	.n_ops		= ARRAY_SIZE(miwill_nl_ops),
	.mcgrps		= miwill_nl_mcgrps,
	.n_mcgrps	= ARRAY_SIZE(miwill_nl_mcgrps),
	#if (LINUX_VERSION_CODE > KERNEL_VERSION(6, 1, 0))
	.resv_start_op	= __MIWILL_CMD_MAX + 1,
	#endif
};

/*
* miwill_nl_msg - send massage by generic netlink
* @data: Send data buffer
* @len:  Data length unit: byte
* @pid:  Pid sent to the client
* @cmd:  Generic netlink cmd
* @attr: Generic netlink attribute
*/
int miwill_nl_msg(void *data, int len, pid_t pid, u8 cmd, u8 attr)
{
	struct sk_buff *skb;
	void *hdr;

	skb = genlmsg_new(nla_total_size(len), GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	hdr = genlmsg_put(skb, pid, 0, &miwill_nl_family, 0, cmd);
	if (!hdr)
		goto out_free;

	if (nla_put(skb, attr, len, data))
		goto out_cancel;

	genlmsg_end(skb, hdr);

	if (pid) {
		return genlmsg_unicast(&init_net, skb, pid);
	} else {
		return genlmsg_multicast(&miwill_nl_family, skb, 0, 0, GFP_ATOMIC);
	}

out_cancel:
	genlmsg_cancel(skb, hdr);
out_free:
	nlmsg_free(skb);
	return -EMSGSIZE;
}

static inline int __miwill_nl_notify(void *data, int len, u8 attr)
{
	return miwill_nl_msg(data, len, 0, MIWILL_CMD_NOTIFY, attr);
}

int miwill_nl_notify_ip(u32 ip)
{
	miwill_genl_dbg("%s "IPv4_FMT, __func__, IPv4_ARG(&ip));
	return __miwill_nl_notify((void *)&ip, sizeof(ip), MIWILL_ATTR_IP);
}

int miwill_nl_notify_reset(u8 reset)
{
	miwill_genl_dbg("%s %d", __func__, reset);
	return __miwill_nl_notify((void *)&reset, sizeof(reset), MIWILL_ATTR_RESET);
}

static int miwill_nl_cmd_options_set(struct sk_buff *skb, struct genl_info *info)
{
	struct miwill *miw;
	struct net_device *dev;
	struct nlmsghdr *nlhdr;
	struct genlmsghdr *genlhdr;
	struct nlattr *opt_attrs[MIWILL_ATTR_MAX + 1];

	u8 enable = 0xFF;
	unsigned char *gw_addr = NULL;
	char *mode = NULL, *miw_name = NULL, *port_5g = NULL, *port_24g = NULL;
	int err = 0;

	nlhdr = nlmsg_hdr(skb);
	genlhdr = nlmsg_data(nlhdr);
	err = nla_parse_deprecated(opt_attrs, MIWILL_ATTR_MAX, genlmsg_data(genlhdr),
								genlmsg_len(genlhdr), miwill_genl_policy, info->extack);
	if (err)
		return err;

	/* get miw instance */
	if (opt_attrs[MIWILL_ATTR_OPTION_MIW]) {
		miw_name = nla_data(opt_attrs[MIWILL_ATTR_OPTION_MIW]);
		miwill_genl_dbg("%s miw: %s", __func__, miw_name);
		dev = miwill_name_to_dev(miw_name);
		if (!dev) {
			return -1;
		}
		miw = netdev_priv(dev);
		dev_put(dev);
	} else {
		miwill_genl_dbg("%s miw: NULL", __func__);
		return -1;
	}

	/* get attr */
	if (opt_attrs[MIWILL_ATTR_OPTION_ENABLE]) {
		enable = nla_get_u8(opt_attrs[MIWILL_ATTR_OPTION_ENABLE]);
		miwill_genl_dbg("%s enable: %d", __func__, enable);
	}
	if (opt_attrs[MIWILL_ATTR_OPTION_MODE]) {
		mode = nla_data(opt_attrs[MIWILL_ATTR_OPTION_MODE]);
		miwill_genl_dbg("%s mode: %s", __func__, mode);
	}
	if (opt_attrs[MIWILL_ATTR_OPTION_GW_ADDR]) {
		gw_addr = nla_data(opt_attrs[MIWILL_ATTR_OPTION_GW_ADDR]);
		miwill_genl_dbg("%s gw_addr: "MAC_FMT, __func__, MAC_ARG(gw_addr));
	}
	if (opt_attrs[MIWILL_ATTR_OPTION_PORT_5G]) {
		port_5g = nla_data(opt_attrs[MIWILL_ATTR_OPTION_PORT_5G]);
		miwill_genl_dbg("%s port_5g: %s", __func__, port_5g);
	}
	if (opt_attrs[MIWILL_ATTR_OPTION_PORT_24G]) {
		port_24g = nla_data(opt_attrs[MIWILL_ATTR_OPTION_PORT_24G]);
		miwill_genl_dbg("%s port_24g: %s", __func__, port_24g);
	}

	return miwill_options_set(miw, enable, mode, gw_addr, port_5g, port_24g);
}

static int miwill_nl_cmd_options_get(struct sk_buff *skb, struct genl_info *info)
{
	struct nlmsghdr *nlhdr;
	struct genlmsghdr *genlhdr;
	struct nlattr *nlh;
	char *str;

	nlhdr = nlmsg_hdr(skb);
	genlhdr = nlmsg_data(nlhdr);

	nlh = genlmsg_data(genlhdr);
	str = nla_data(nlh);
	miwill_genl_dbg("%s: %s\n", __func__, str);

	return 0;
	//return genlmsg_reply(skb, info);
}

int minet_nl_notify(void *data, int len)
{
	miwill_genl_dbg("%s %d", __func__, len);
	return __miwill_nl_notify(data, len, MINET_ATTR_COMMON);
}

extern int minet_set_app(void *arg);
static int minet_nl_cmd_set(struct sk_buff *skb, struct genl_info *info)
{
	struct nlmsghdr *nlhdr;
	struct genlmsghdr *genlhdr;
	struct nlattr *opt_attrs[MIWILL_ATTR_MAX + 1];
	int err = 0;

	nlhdr = nlmsg_hdr(skb);
	genlhdr = nlmsg_data(nlhdr);
	err = nla_parse_deprecated(opt_attrs, MIWILL_ATTR_MAX, genlmsg_data(genlhdr),
								genlmsg_len(genlhdr), miwill_genl_policy, info->extack);
	if (err)
		return err;

	if (opt_attrs[MINET_ATTR_COMMON]) {
		return minet_set_app(nla_data(opt_attrs[MINET_ATTR_COMMON]));
	}
	
	return 0;
}

int  __init miwill_nl_init(void)
{
	int rc;
	if ((rc = genl_register_family(&miwill_nl_family))) {
		miwill_genl_err("%s genl_register_family:%d", __func__, rc);
		return rc;
	}
	miwill_genl_dbg("%s OK", __func__);
	return 0;
}

void miwill_nl_finit(void)
{
	miwill_genl_dbg("%s", __func__);
	genl_unregister_family(&miwill_nl_family);
}