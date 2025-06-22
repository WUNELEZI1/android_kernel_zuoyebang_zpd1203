/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DPU_LOG_H_
#define _DPU_LOG_H_

#include <drm/drm_print.h>
#include <linux/ktime.h>
#include "dpu_dbg.h"

enum {
	DPU_LOG_LEVEL_ERROR = 1,
	DPU_LOG_LEVEL_WARNING = 2,
	DPU_LOG_LEVEL_INFO = 3,
	DPU_LOG_LEVEL_DEBUG = 4,
};

enum dpu_debug_category {
	DPU_BASE =        BIT(0),   /* used for dpu core log */
	DPU_CPU_REG =     BIT(1),   /* used for dpu cpu config register dump */
	DPU_CMDLIST =     BIT(2),   /* used in dpu cmdlist code */
	DPU_ISR =         BIT(3),   /* used in dpu isr code */
	DPU_SYNC =        BIT(4),   /* used in dpu fence code */
	DPU_COLOR =       BIT(5),   /* used in dpu color code */
	DPU_DSI =         BIT(6),   /* used in dpu dsi code */
	DPU_PROPERTY =    BIT(7),   /* used in dpu property code */
	DPU_DP =          BIT(8),   /* used in dpu dp code */
	DPU_PERF =        BIT(9),   /* used in dpu core perf control code */
	DPU_DFX =         BIT(10),  /* used in dfx log */
	DPU_CLK =         BIT(11),  /* used in dpu clk */
	DPU_POWER =       BIT(12),  /* used in dpu power */
	DPU_IDLE =        BIT(13),  /* used in dpu idle */
	DPU_FLOWCTRL =    BIT(14),  /* used in dpu flow ctrl */
	DPU_IOMMU =       BIT(15),  /* used in dpu iommu map info */
	DPU_LAYER =       BIT(16),  /* used in dpu layer info */
	DPU_RES =         BIT(17),  /* used in dpu res mgr */
	DPU_EVENT =       BIT(18),  /* used in dpu crtc event */
	DPU_STRIPE =      BIT(19),  /* used in dpu stripe func */
	DPU_COMPOSER =    BIT(20),  /* used in dpu mixer */
};

#define DPU_DEBUG(msg, ...) \
	do { if (g_dpu_msg_level >= DPU_LOG_LEVEL_DEBUG || g_enable_basic_dfx) \
		pr_err("[DPU D]: [CORE]: %s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_INFO(msg, ...) \
	do { if (g_dpu_msg_level >= DPU_LOG_LEVEL_INFO) \
		pr_err("[DPU I]: [CORE]: %s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_WARN(msg, ...) \
	do { if (g_dpu_msg_level >= DPU_LOG_LEVEL_WARNING) \
		pr_err("[DPU W]: [CORE]: %s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_ERROR(msg, ...) \
	do { if (g_dpu_msg_level >= DPU_LOG_LEVEL_ERROR) \
		pr_err("[DPU E]: [CORE]: %s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_LOGD_IF(tag, msg, ...) \
	do { if (g_dpu_msg_level >= DPU_LOG_LEVEL_DEBUG && (g_dpu_msg_category & tag)) \
		pr_err("[DPU D]: [%s]: %s: "msg, (dpu_category_string_get(tag)), __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_LOGI_IF(tag, msg, ...) \
	do { if (g_dpu_msg_level >= DPU_LOG_LEVEL_INFO) \
		pr_err("[DPU I]: [%s]: %s: "msg, (dpu_category_string_get(tag)), __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_REG_DEBUG(msg, ...) \
	DPU_LOGD_IF(DPU_CPU_REG, msg, ## __VA_ARGS__)

#define DPU_CMDLIST_DEBUG(msg, ...) \
	DPU_LOGD_IF(DPU_CMDLIST, msg, ## __VA_ARGS__)

#define DPU_CMDLIST_INFO(msg, ...) \
	DPU_LOGI_IF(DPU_CMDLIST, msg, ## __VA_ARGS__)

#define DPU_ISR_DEBUG(msg, ...) \
	DPU_LOGD_IF(DPU_ISR, msg, ## __VA_ARGS__)

#define DPU_SYNC_DEBUG(msg, ...) \
	DPU_LOGD_IF(DPU_SYNC, msg, ## __VA_ARGS__)

#define DPU_PROPERTY_DEBUG(msg, ...) \
	DPU_LOGD_IF(DPU_PROPERTY, msg, ## __VA_ARGS__)

#define DPU_COLOR_INFO(msg, ...) DPU_LOGI_IF(DPU_COLOR, msg, ## __VA_ARGS__)
#define DPU_COLOR_DEBUG(msg, ...) DPU_LOGD_IF(DPU_COLOR, msg, ## __VA_ARGS__)

#define DSI_INFO(msg, ...) DPU_LOGI_IF(DPU_DSI, msg, ## __VA_ARGS__)
#define DSI_DEBUG(msg, ...) DPU_LOGD_IF(DPU_DSI, msg, ## __VA_ARGS__)
#define DSI_ERROR DPU_ERROR
#define DSI_WARN DPU_WARN

#define DP_INFO(msg, ...) DPU_LOGI_IF(DPU_DP, msg, ## __VA_ARGS__)
#define DP_DEBUG(msg, ...) DPU_LOGD_IF(DPU_DP, msg, ## __VA_ARGS__)
#define DP_ERROR DPU_ERROR
#define DP_WARN DPU_WARN

#define PERF_INFO(msg, ...) DPU_LOGI_IF(DPU_PERF, msg, ## __VA_ARGS__)
#define PERF_DEBUG(msg, ...) DPU_LOGD_IF(DPU_PERF, msg, ## __VA_ARGS__)
#define PERF_ERROR DPU_ERROR

#define CLK_INFO(msg, ...) DPU_LOGI_IF(DPU_CLK, msg, ## __VA_ARGS__)
#define CLK_DEBUG(msg, ...) DPU_LOGD_IF(DPU_CLK, msg, ## __VA_ARGS__)
#define CLK_ERROR DPU_ERROR

#define POWER_INFO(msg, ...) DPU_LOGI_IF(DPU_POWER, msg, ## __VA_ARGS__)
#define POWER_DEBUG(msg, ...) DPU_LOGD_IF(DPU_POWER, msg, ## __VA_ARGS__)
#define POWER_ERROR DPU_ERROR

#define IDLE_INFO(msg, ...) DPU_LOGI_IF(DPU_IDLE, msg, ## __VA_ARGS__)
#define IDLE_DEBUG(msg, ...) DPU_LOGD_IF(DPU_IDLE, msg, ## __VA_ARGS__)
#define IDLE_ERROR DPU_ERROR

#define DFX_INFO(msg, ...) DPU_LOGI_IF(DPU_DFX, msg, ## __VA_ARGS__)
#define DFX_DEBUG(msg, ...) DPU_LOGD_IF(DPU_DFX, msg, ## __VA_ARGS__)
#define DFX_ERROR DPU_ERROR

#define FLOWCTRL_DEBUG(msg, ...) DPU_LOGD_IF(DPU_FLOWCTRL, msg, ## __VA_ARGS__)

#define IOMMU_DEBUG(msg, ...) DPU_LOGD_IF(DPU_IOMMU, msg, ## __VA_ARGS__)

#define EVENT_DEBUG(msg, ...) DPU_LOGD_IF(DPU_EVENT, msg, ## __VA_ARGS__)

#define STRIPE_DEBUG(msg, ...) DPU_LOGD_IF(DPU_STRIPE, msg, ## __VA_ARGS__)

#define COMPOSER_DEBUG(msg, ...) DPU_LOGD_IF(DPU_COMPOSER, msg, ## __VA_ARGS__)

static inline void dpu_get_timestamp(u64 *start_ns)
{
	*start_ns = ktime_get_mono_fast_ns();
}

#define DPU_TRACE_TS_BEGIN(start_ns) dpu_get_timestamp(start_ns)
#define DPU_PRINT_TIMESTAMP(start_ns, msg)  do { \
		if (g_dpu_mdr_en) { \
			u64 end_ns; \
			dpu_get_timestamp(&end_ns); \
			DPU_INFO("%s timediff=%llu us", msg, div_u64(end_ns - *start_ns, 1000)); \
		} \
	} while (0)

#define LAYER_DEBUG(msg, ...) DPU_LOGD_IF(DPU_LAYER, msg, ## __VA_ARGS__)

#define RES_DEBUG(msg, ...) DPU_LOGD_IF(DPU_RES, msg, ## __VA_ARGS__)

#define INIT_START_TIMING(time_var) ((time_var) = ktime_get())

#define END_TIMING_AND_PRINT_IF_EXCEEDS(msg, max_duration_ms, time_var) do { \
	ktime_t __end_time = ktime_get(); \
	s64 __elapsed_ns = ktime_to_ns(ktime_sub(__end_time, (time_var))); \
	u64 __elapsed_ms = div_u64(__elapsed_ns, NSEC_PER_MSEC); \
	if (__elapsed_ms > (max_duration_ms)) { \
		DPU_INFO("%s took %llu ms (exceeds threshold[%u])\n", \
			msg, (unsigned long long)__elapsed_ms, (max_duration_ms)); \
	} \
} while (0)

#endif /* _DPU_LOG_H_ */
