#ifndef _MIWILL_PROTO_H_
#define _MIWILL_PROTO_H_

#include <linux/types.h>
#include <linux/if_ether.h>

/*
 * The experimental protocol number for MiWiLL (Mi Wireless Link-Layer) protocol
 */
#define ETH_P_MIWILL 0xeee1

#define MIWILL_VERSION 1
#define MIWILL_OPT_LEN 3

/* the source id of packets sent out,  
 * the last-4-byte value of miwill netdevice 
 */
#define MIWILL_SRC_ID_LEN 2

/*
 * the miwill mode
 */
enum MIWILL_MODE {
	RAW_MODE = 0,
	REDUD_MODE = 1,
	PASS_MODE = 0xE,
	CMD_MODE = 0xF,
};

struct MIWILL_red_opt {
	unsigned char sub_type; /* no used */
	__be16 seq_num;
} __attribute__((packed));

struct MIWILL_cmd_opt {
	unsigned char sub_type; /* miwill_mframe_t */
	__be16 seq_num;
} __attribute__((packed));

union MIWILL_mode_opt {
	unsigned char raw[MIWILL_OPT_LEN];
	struct MIWILL_red_opt redud;
	struct MIWILL_cmd_opt cmd;
};

/*
 * MiWiLL Header definition V0 : total length = 1 + 3 + 2 + 2 = 8
 * +--------+--------+------------+----------+-------------+-------------
 * | version|  MODE  |   OPTION   |  SRC_ID  | ENCAP_PROTO | PAYLOAD
 * +--------+--------+------------+----------+-------------+-------------
 * |  0.5   |   0.5  |      3     |     2    |       2     |
 */

struct MIWILL_header {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	unsigned char mode : 4;
	unsigned char version : 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	unsigned char version : 4;
	unsigned char mode : 4;
#else
#error "Please fix <asm/byteorder.h>"
#endif
	union MIWILL_mode_opt option;
	unsigned char src_id[MIWILL_SRC_ID_LEN];
	__be16 encap_proto; /* the original L3 protocol */
} __attribute__((packed));

struct MIWILL_ethhdr
{
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	__be16			h_miwill_proto;		/* packet type ID field	*/
	struct MIWILL_header header;
} __attribute__((packed));


/* The helper functions */

static inline unsigned char get_miwill_ver(struct MIWILL_header *hdr)
{
	/* ??? */
	return hdr->version;
}

static inline enum MIWILL_MODE get_miwill_mode(struct MIWILL_header *hdr)
{
	return hdr->mode;
}

/* the func defined for new packet type */
int miwill_rcv(struct sk_buff *, struct net_device *, struct packet_type *,
	       struct net_device *);

#endif //_MIWILL_PROTO_H_
