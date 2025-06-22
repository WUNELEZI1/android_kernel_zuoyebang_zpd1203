#ifndef _MIWILL_NETLINK_H_
#define _MIWILL_NETLINK_H_

/**********************************
 * NETLINK_GENERIC netlink family.
 **********************************/

/* netlink attributes */
enum {
	MIWILL_ATTR_UNSPEC,
	/* CMD:set/get; ATTR:option */
	MIWILL_ATTR_OPTION_MIW,
	MIWILL_ATTR_OPTION_ENABLE,
	MIWILL_ATTR_OPTION_MODE,
	MIWILL_ATTR_OPTION_GW_ADDR,
	MIWILL_ATTR_OPTION_PORT_5G,
	MIWILL_ATTR_OPTION_PORT_24G,
	/* CMD:notify; ATTR:IP or reset reason */
	MIWILL_ATTR_IP,
	MIWILL_ATTR_RESET,
	
	MINET_ATTR_COMMON,

	__MIWILL_ATTR_MAX,
	MIWILL_ATTR_MAX = __MIWILL_ATTR_MAX - 1,
};

/* netlink commend */
enum {
	MIWILL_CMD_UNSPEC,
	MIWILL_CMD_OPTIONS_SET,
	MIWILL_CMD_OPTIONS_GET,
	MIWILL_CMD_NOTIFY,
	MINET_CMD_SET,

	__MIWILL_CMD_MAX,
	MIWILL_CMD_MAX = __MIWILL_CMD_MAX - 1,
};

/*
 * NETLINK_GENERIC related info
 */
#define MIWILL_GENERIC_VERSION 0x01
#define MIWILL_GENERIC_NAME "miwill_genl"
#define MIWILL_GENERIC_EVENT_MCGRP_NAME "miwill_event"

enum {
	MIWILL_RESET_REASON_TIMEOUT = 1,
	MIWILL_RESET_REASON_PEER
};

extern int miwill_nl_init(void);
extern void miwill_nl_finit(void);
extern int miwill_nl_notify_ip(u32 ip);
extern int miwill_nl_notify_reset(u8 reset);

#endif