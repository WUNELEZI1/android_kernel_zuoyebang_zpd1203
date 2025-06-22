/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */
#ifndef __DVFS_GOV_H__
#define __DVFS_GOV_H__

#define VOTE_MSG_SIZE	16
#define NODE_MODE_RW	0640

enum {
	XSEE,
	MEMLAT,
};

enum {
	SET_TYPE,
	GET_TYPE,
};

struct vote_msg {
	u8 dev_type;
	u8 cluster_id;
	u8 msg_type;
	u8 para_type;
	u8 mem_type;
	u8 data_size;
	u16 value;
	u16 value2;
	u16 reserved[3];
};

/* dvfs_gov */
int dvfs_gov_msg_trans(struct vote_msg *msg);
int dvfs_gov_data_send(struct vote_msg *header, u8 *data);
int dvfs_gov_data_receive(struct vote_msg *header, u8 *buf, u32 buf_size);
u32 get_cluster_num(void);
/* xsee */
int xsee_init(void);
/* memlat */
int memlat_init(void);
#endif /* __DVFS_GOV_H__ */
