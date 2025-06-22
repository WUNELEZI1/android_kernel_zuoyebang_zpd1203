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

#ifndef _DPU_LOG_H_
#define _DPU_LOG_H_

#include "osal.h"

extern uint32_t g_dpu_log_level;

enum {
	DPU_LOG_LVL_ERR = 0,
	DPU_LOG_LVL_WARNING,
	DPU_LOG_LVL_INFO,
	DPU_LOG_LVL_DEBUG,
	DPU_LOG_LVL_MAX,
};

#define _DPU_DEBUG(EfiD, Expression, ...) DEBUG((EfiD, Expression, ##__VA_ARGS__))
#define dpu_pr_err(msg, ...) _DPU_DEBUG (EFI_D_ERROR, "LITEXDM [E][%a:%d]"msg"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define dpu_pr_info(msg, ...) _DPU_DEBUG (EFI_D_XRINFO, "LITEXDM [I][%a:%d]"msg"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define dpu_pr_warn(msg, ...) _DPU_DEBUG (EFI_D_ERROR, "LITEXDM [W][%a:%d]"msg"", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef CONFIG_DPU_ENG_DEBUG
#define dpu_pr_debug(msg, ...) _DPU_DEBUG (EFI_D_ERROR, "LITEXDM [D][%a:%d]"msg"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define dpu_pr_debug(msg, ...) _DPU_DEBUG (EFI_D_INFO, "LITEXDM [D][%a:%d]"msg"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#ifndef DPU_ERROR
#define DPU_ERROR dpu_pr_err
#define DPU_INFO dpu_pr_info
#define DPU_DEBUG dpu_pr_debug
#define DPU_WARN dpu_pr_warn

#define DPU_CORE_DEBUG dpu_pr_debug
#define DPU_REG_DEBUG dpu_pr_debug
#define DPU_PERF_DEBUG dpu_pr_debug
#define DPU_PERF_INFO dpu_pr_info

#define DSI_DEBUG dpu_pr_debug
#define DSI_INFO dpu_pr_info
#define DSI_ERROR dpu_pr_err
#define DSI_WARN dpu_pr_warn
#endif

/* for lowlevel build */
#define DPU_COLOR_INFO dpu_pr_info
#define DPU_COLOR_DEBUG dpu_pr_debug
#define DPU_COLOR_WARN dpu_pr_warn
#define DPU_COLOR_ERROR dpu_pr_err

#define PERF_INFO dpu_pr_info
#define PERF_DEBUG dpu_pr_debug
#define PERF_ERROR dpu_pr_err

#define DFX_INFO dpu_pr_info
#define DFX_DEBUG dpu_pr_debug
#define DFX_ERROR dpu_pr_err

#define IOMMU_DEBUG dpu_pr_debug

#define COMPOSER_DEBUG dpu_pr_debug

#define dpu_check_and_return(condition, ret, msg, ...) \
	do { \
		if (condition) { \
			dpu_pr_err(msg, ##__VA_ARGS__);\
			return ret; \
		} \
	} while (0)

#define dpu_check_and_no_retval(condition, msg, ...) \
	do { \
		if (condition) { \
			dpu_pr_err(msg, ##__VA_ARGS__);\
			return; \
		} \
	} while (0)

#endif /* _DPU_LOG_H_ */
