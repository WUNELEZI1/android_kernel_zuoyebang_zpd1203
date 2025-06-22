/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#ifndef __XRSE_RIP_DRV_H__
#define __XRSE_RIP_DRV_H__

#include <linux/types.h>
#include <dt-bindings/xring/platform-specific/xrse_platform.h>

u32 xrse_rip_base_request(u32 module_id, u64 addr, u32 len, struct rip_measure_strategy_str *strategy);
u32 xrse_rip_realtime_request(u32 module_id, struct rip_measure_status_str *rip_sta, u32 rip_sta_size);
u32 xrse_rip_get_status(struct rip_measure_status_str *rip_sta, u32 rip_sta_size);

#endif
