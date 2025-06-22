/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#ifndef __DDR_VOTE_DEV_H__
#define __DDR_VOTE_DEV_H__

#define DDR_DVFS_VOTE_IOCTRL_CMD	0x6C6C0001

enum ddr_dvfs_vote_type {
	VOTE_TYPE_START = 0,
	VOTE_MIN_BANDWIDTH_MB_S = 1,
	VOTE_DMD_BANDWIDTH_MB_S = 2,
	VOTE_MAX_BANDWIDTH_MB_S = 3,
	VOTE_MIN_FREQ_MHZ = 4,
	VOTE_DMD_FREQ_MHZ = 5,
	VOTE_MAX_FREQ_MHZ = 6,
	VOTE_MIN_DATA_RATE_MBPS = 7,
	VOTE_DMD_DATA_RATE_MBPS = 8,
	VOTE_MAX_DATA_RATE_MBPS = 9,
	VOTE_TYPE_END = 10,
};

union ddr_vote_data {
	unsigned long value;
	struct {
		unsigned short vote_ch;
		unsigned short vote_type;
		unsigned int vote_val;
	} data;
};

/*
 * vote_ch: assigned by the DDR software
 * vote_type: choose from "enum ddr_dvfs_vote_type"
 * vote_val: match with vote_type
 *
 * return: 0: success, <0: error
 */
int ddr_vote_dev_request(unsigned int vote_ch, unsigned int vote_type, unsigned int vote_val);

#endif /* __DDR_VOTE_DEV_H__ */
