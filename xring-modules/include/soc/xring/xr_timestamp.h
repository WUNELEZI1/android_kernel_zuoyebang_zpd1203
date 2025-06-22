/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#ifndef __XR_TIMESTAMP_H__
#define __XR_TIMESTAMP_H__

ktime_t xr_timestamp_gettime(void);
u64 xr_timestamp_count2ns(u64 count);
u64 xr_timestamp_getcount(void);

#endif /* __XR_TIMESTAMP_H__ */
