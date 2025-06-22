/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
 */

#ifndef __SHUB_TIME_SYNC_H__
#define __SHUB_TIME_SYNC_H__

void shub_timesync_init(void);
void shub_timesync_start(void);
void sh_timesync_cancel(void);
#endif /* __SHUB_TIME_SYNC_H__ */
