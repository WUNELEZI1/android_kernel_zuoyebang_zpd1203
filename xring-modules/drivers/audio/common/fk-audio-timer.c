// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/rtc.h>

#include "fk-audio-timer.h"
#include "fk-audio-log.h"

static inline unsigned int timer_reg_read32(const void __iomem *reg)
{
	return __raw_readl(reg);
}

static uint64_t get_time_stamp(void)
{
	uint64_t cur_cnt;
	uint32_t cnt_l[2];
	uint32_t cnt_h[2];
	void __iomem *t_vddr;

	t_vddr = ioremap(TIME_STAMP_SYS_CNT, TIMER_REGISTER_SIZE);
	if (!t_vddr)
		return 0;

	cnt_l[0] = timer_reg_read32((t_vddr + TIME_STAMP_SYS_CNT_VAL_L_OFFSET));
	cnt_h[0] = timer_reg_read32((t_vddr + TIME_STAMP_SYS_CNT_VAL_H_OFFSET));
	cnt_l[1] = timer_reg_read32((t_vddr + TIME_STAMP_SYS_CNT_VAL_L_OFFSET));
	cnt_h[1] = timer_reg_read32((t_vddr + TIME_STAMP_SYS_CNT_VAL_H_OFFSET));

	if (cnt_h[0] == cnt_h[1])
		cur_cnt = ((uint64_t)cnt_h[1] << 32) | cnt_l[1];
	else
		cur_cnt = ((uint64_t)cnt_h[0] << 32) | cnt_l[0];

	return cur_cnt;
}

void get_adsp_time(void)
{
	uint64_t cnt;
	struct time_stamp_s sys_time;

	cnt = get_time_stamp();
	if (!cnt)
		return;

	sys_time.sec = cnt / TIMESTAMP_FREQ;
	sys_time.usec = (cnt % TIMESTAMP_FREQ) * TIMESTAMP_USEC_PER_SEC / TIMESTAMP_FREQ;

	AUD_LOG_INFO(AUD_COMM, "[%llu.%06llu]", sys_time.sec, sys_time.usec);
}
EXPORT_SYMBOL(get_adsp_time);

void get_utc_time(void)
{
	struct timespec64 tv;
	struct rtc_time tm;

	ktime_get_real_ts64(&tv);
	rtc_time64_to_tm(tv.tv_sec, &tm);

	/* system time = utc time + 8 hours */
	AUD_LOG_INFO(AUD_COMM, "UTC time: [%04d-%02d-%02d %02d:%02d:%02d.%06lu]",
			tm.tm_year+1900,
			tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min,
			tm.tm_sec, tv.tv_nsec / 1000);
}
EXPORT_SYMBOL(get_utc_time);

/* Module information */
MODULE_DESCRIPTION("XRING AUDIO TIMER");
MODULE_LICENSE("Dual BSD/GPL");
