/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#ifndef __DDR_FLUX_H__
#define __DDR_FLUX_H__

struct ddr_throughput {
	unsigned long long rd_bytes;
	unsigned long long wr_bytes;
};

int get_ddr_throughput(struct ddr_throughput *throughput);

#endif
