// SPDX-License-Identifier: GPL-2.0-only
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
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/posix-timers.h>
#include <linux/time_namespace.h>
#include "tee_bench.h"
#include <soc/xring/xr_timestamp.h>

#if MITEE_TEST_TIMESTAMP
struct tee_bench_info *g_tee_bench_data;
static struct mutex g_tee_bench_mutex;

static struct tee_bench_info *get_tee_bench_data(void)
{
	return g_tee_bench_data;
}

void mitee_bench_enable(void)
{
	int ret = 0;
	struct tee_bench_info *bench_data = NULL;

	mutex_init(&g_tee_bench_mutex);
	bench_data = kzalloc(sizeof(*bench_data) * TEE_MAX_CATA_NUM, GFP_KERNEL);
	if (!bench_data) {
		ret = -ENOMEM;
		pr_err("[TIMESTAMP] failed to kzalloc\n");
	}
	g_tee_bench_data = bench_data;
}

void mitee_bench_disable(void)
{
	struct tee_bench_info *bench_info = get_tee_bench_data();

	kfree(bench_info);
	mutex_destroy(&g_tee_bench_mutex);
}

void mitee_bench_add_node(uint32_t session, uint8_t *uuid)
{
	struct tee_bench_info *bench_info = get_tee_bench_data();
	bool tee_bench_find = false;

	mutex_lock(&g_tee_bench_mutex);
	for (uint32_t i = 0; i < TEE_MAX_CATA_NUM; i++) {
		if (bench_info[i].is_used == false) {
			tee_bench_find = true;
			bench_info[i].is_used = true;
			bench_info[i].session_id = session;
			memcpy(bench_info[i].uuid, uuid, sizeof(uint8_t) * TEE_IOCTL_UUID_LEN);
			bench_info[i].tee_ts_index = 0;
			break;
		}
	}
	mutex_unlock(&g_tee_bench_mutex);
	if (tee_bench_find == false)
		pr_err("[TIMESTAMP] failed to find tee bench space\n");
}

void mitee_bench_del_node(uint32_t session)
{
	struct tee_bench_info *bench_info = get_tee_bench_data();
	bool tee_bench_find = false;

	mutex_lock(&g_tee_bench_mutex);
	for (uint32_t i = 0; i < TEE_MAX_CATA_NUM; i++) {
		if (bench_info[i].session_id == session && bench_info[i].is_used == true) {
			tee_bench_find = true;
			bench_info[i].is_used = false;
			bench_info[i].session_id = 0;
			break;
		}
	}
	mutex_unlock(&g_tee_bench_mutex);
	if (tee_bench_find == false)
		pr_err("[TIMESTAMP] failed to find tee bench data\n");
}

void mitee_bench_timestamp(uint32_t session, char str[])
{
	int find_id = -1;
	struct tee_bench_info *bench_data = get_tee_bench_data();
	ktime_t ts_time;

	mutex_lock(&g_tee_bench_mutex);
	for (int i = 0; i < TEE_MAX_CATA_NUM; i++) {
		if (bench_data[i].session_id == session && bench_data[i].is_used == true) {
			find_id = i;
			break;
		}
	}
	if (find_id == -1) {
		ts_time = xr_timestamp_gettime();
		pr_info("[TIMESTAMP][%-25s] cur = %lld.%09llds\n", str, ts_time / NSEC_PER_SEC,
			ts_time - ts_time / NSEC_PER_SEC * NSEC_PER_SEC);
		mutex_unlock(&g_tee_bench_mutex);
		return;
	}

	uint32_t index = bench_data[find_id].tee_ts_index;

	ts_time = xr_timestamp_gettime();
	bench_data[find_id].tee_ts[index] = ts_time;
	if (index > 0) {
		pr_info("[TIMESTAMP][UUID: 0x%x%x%x%x-0x%x%x-0x%x%x-0x%x%x%x%x%x%x%x%x][%-25s] index = %02d  cur = %lld.%09llds  diff = %llu us\n",
			bench_data[find_id].uuid[0], bench_data[find_id].uuid[1], bench_data[find_id].uuid[2], bench_data[find_id].uuid[3],
			bench_data[find_id].uuid[4], bench_data[find_id].uuid[5], bench_data[find_id].uuid[6], bench_data[find_id].uuid[7],
			bench_data[find_id].uuid[8], bench_data[find_id].uuid[9], bench_data[find_id].uuid[10], bench_data[find_id].uuid[11],
			bench_data[find_id].uuid[12], bench_data[find_id].uuid[13], bench_data[find_id].uuid[14], bench_data[find_id].uuid[15],
			str, index,
			ts_time / NSEC_PER_SEC, ts_time - ts_time / NSEC_PER_SEC * NSEC_PER_SEC,
			bench_data[find_id].tee_ts[index] / NSEC_PER_USEC - bench_data[find_id].tee_ts[index - 1] / NSEC_PER_USEC);
	} else {
		pr_info("[TIMESTAMP][UUID: 0x%x%x%x%x-0x%x%x-0x%x%x-0x%x%x%x%x%x%x%x%x][%-25s] index = %02d  cur = %lld.%09llds  diff = %llu us\n",
			bench_data[find_id].uuid[0], bench_data[find_id].uuid[1], bench_data[find_id].uuid[2], bench_data[find_id].uuid[3],
			bench_data[find_id].uuid[4], bench_data[find_id].uuid[5], bench_data[find_id].uuid[6], bench_data[find_id].uuid[7],
			bench_data[find_id].uuid[8], bench_data[find_id].uuid[9], bench_data[find_id].uuid[10], bench_data[find_id].uuid[11],
			bench_data[find_id].uuid[12], bench_data[find_id].uuid[13], bench_data[find_id].uuid[14], bench_data[find_id].uuid[15],
			str, index,
			ts_time / NSEC_PER_SEC, ts_time - ts_time / NSEC_PER_SEC * NSEC_PER_SEC,
			bench_data[find_id].tee_ts[index] / NSEC_PER_USEC - bench_data[find_id].tee_ts[TEE_MAX_TS_NUM - 1] / NSEC_PER_USEC);
	}
	bench_data[find_id].tee_ts_index++;
	if (bench_data[find_id].tee_ts_index >= TEE_MAX_TS_NUM)
		bench_data[find_id].tee_ts_index = 0;

	mutex_unlock(&g_tee_bench_mutex);
}
#endif
