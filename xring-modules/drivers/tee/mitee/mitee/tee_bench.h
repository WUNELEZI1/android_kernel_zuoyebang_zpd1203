/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2015 Google, Inc.
 * Copyright (C) 2024 XiaoMi, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _TEE_BENCH_H
#define _TEE_BENCH_H

#include <linux/rwsem.h>

#define MITEE_TEST_TIMESTAMP 0
#define TEE_MAX_CATA_NUM 15
#define TEE_MAX_TS_NUM 20
#define TEE_IOCTL_UUID_LEN 16

struct tee_bench_info {
	bool is_used;
	uint8_t uuid[TEE_IOCTL_UUID_LEN];
	uint32_t session_id;
	uint32_t tee_ts_index;
	ktime_t tee_ts[TEE_MAX_TS_NUM];
};

#if MITEE_TEST_TIMESTAMP
void mitee_bench_enable(void);
void mitee_bench_disable(void);
void mitee_bench_add_node(uint32_t session, uint8_t *uuid);
void mitee_bench_del_node(uint32_t session);
void mitee_bench_timestamp(uint32_t session, char str[]);
#else
static inline void mitee_bench_enable(void)
{

}
static inline void mitee_bench_disable(void)
{

}
static inline void mitee_bench_add_node(uint32_t session, uint8_t *uuid)
{

}
static inline void mitee_bench_del_node(uint32_t session)
{

}
static inline void mitee_bench_timestamp(uint32_t session, char str[])
{
}
#endif

#endif /* _TEE_BENCH_H */
