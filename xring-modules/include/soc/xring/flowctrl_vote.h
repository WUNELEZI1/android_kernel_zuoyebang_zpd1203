/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#ifndef __FLOWCTRL_VOTE_H__
#define __FLOWCTRL_VOTE_H__

#define FLOWCTRL_VOTE_IOCTRL_CMD	0x6C6C0001

enum flowctrl_vote_id {
	VOTE_MEDIA_MODE = 0,
	VOTE_SOC_MODE = 1,
	VOTE_MAX_MODE = 2,
};

union flowctrl_vote_info {
	unsigned long value;
	struct {
		unsigned int vote_id;
		unsigned int vote_bw;
	} data;
};

#endif /* __FLOWCTRL_VOTE_H__ */
