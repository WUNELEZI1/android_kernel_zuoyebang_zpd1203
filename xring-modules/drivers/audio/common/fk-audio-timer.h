/* SPDX-License-Identifier: GPL-2.0-only
 *
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

#ifndef _FK_AUDIF_TIMER_
#define _FK_AUDIF_TIMER_

#include "../acore/fk-acore-define.h"

#define LMS_TIME_STAMP					(0xE0010000)
#define TIME_STAMP_SYS_CNT				(LMS_TIME_STAMP + 0X1000)
#define TIME_STAMP_SYS_CNT_VAL_L_OFFSET	(0X00)
#define TIME_STAMP_SYS_CNT_VAL_H_OFFSET	(0X04)
/* timestamp frequency must be aligned with adsp */
#define TIMESTAMP_FREQ					(2730000)
#define TIMESTAMP_USEC_PER_SEC			(1000000)

#define TIMER_REGISTER_SIZE				(0x4)

struct time_stamp_s {
	uint64_t sec;
	uint64_t usec;
};

void get_adsp_time(void);
void get_utc_time(void);

#endif
