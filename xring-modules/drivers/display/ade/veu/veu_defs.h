/* SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#ifndef _VEU_DEFS_H_
#define _VEU_DEFS_H_

#include <linux/delay.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/bug.h>
#include <linux/printk.h>

#include "veu_dbg.h"

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

/* align */
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, al)  ((val) & ~((typeof(val))(al)-1))
#endif
#ifndef ALIGN_UP
#define ALIGN_UP(val, al)    (((val) + ((al)-1)) & ~((typeof(val))(al)-1))
#endif

#define VEU_ERR(msg, ...) \
		do { if (g_veu_msg_level >= 0) \
			pr_err("[VEU E]:%s %d: "msg, __func__, __LINE__, ## __VA_ARGS__); \
		} while (0)

#define VEU_WARN(msg, ...) \
		do { if (g_veu_msg_level >= 1) \
			pr_warn("[VEU W]:%s %d: "msg, __func__, __LINE__, ## __VA_ARGS__); \
		} while (0)

#define VEU_INFO(msg, ...) \
		do { if (g_veu_msg_level >= 2) \
			pr_info("[VEU I]:%s %d: "msg, __func__, __LINE__, ## __VA_ARGS__); \
		} while (0)

#define VEU_DBG(msg, ...) \
		do { if (g_veu_msg_level >= 3) \
			pr_info("[VEU D]:%s %d: "msg, __func__, __LINE__, ## __VA_ARGS__); \
		} while (0)

#define VEU_LOG_IF(condition, level, msg, ...) \
	do { if (condition) \
		VEU_##level(msg, ##__VA_ARGS__); \
	} while (0)

#define VEU_ISR_DBG(msg, ...) \
		VEU_LOG_IF(g_veu_isr_debug, DBG, msg, ## __VA_ARGS__)

#define VEU_TIMECOST(msg, ...) \
		do { if (g_veu_msg_level >= 3) \
			pr_info("[VEU TIMECOST]:%s %d: "msg, __func__, __LINE__, ## __VA_ARGS__); \
		} while (0)

#define veu_check_and_return(condition, ret, msg, ...) \
	do { \
		if (condition) { \
			VEU_ERR(msg, ##__VA_ARGS__);\
			return ret; \
		} \
	} while (0)

#define veu_check_and_void_return(condition, msg, ...) \
	do { \
		if (condition) { \
			VEU_ERR(msg, ##__VA_ARGS__);\
			return; \
		} \
	} while (0)

#endif /* _VEU_DEFS_H_ */
