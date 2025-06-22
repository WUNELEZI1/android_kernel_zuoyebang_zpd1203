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

#ifndef _DPU_IDLE_CMD_H_
#define _DPU_IDLE_CMD_H_

struct dpu_idle_ctx;

/**
 * dpu_cmd_idle_init - init cmd idle object
 *
 * @idle_ctx: idle_ctx object
 * Return: 0 on success, else on failure
 */
int dpu_cmd_idle_init(struct dpu_idle_ctx *idle_ctx);

/**
 * dpu_cmd_idle_check_idle_policy - check idle policy valid for cmd idle
 * @idle_policy: input idle policy
 *
 * Return: true for success, false for failure
 */
bool dpu_cmd_idle_check_idle_policy(u32 idle_policy);

/**
 * dpu_cmd_idle_get_basic_policy - get basic policy.
 *
 * Return: u32. basic policy
 */
u32 dpu_cmd_idle_get_basic_policy(void);

/**
 * dpu_cmd_idle_prepare - prepare cmd idle (including exit hw_dvfs)
 *
 * @idle_ctx: idle_ctx object
 * Return: 0 on success, else on failure
 */
int dpu_cmd_idle_prepare(struct dpu_idle_ctx *idle_ctx);

/**
 * dpu_cmd_idle_enter - enter cmd idle status
 * including close isr/ enter ulps/close clk or regulator
 * @idle_ctx: idle_ctx object
 */
void dpu_cmd_idle_enter(struct dpu_idle_ctx *idle_ctx);

/**
 * dpu_cmd_idle_exit - exit cmd idle status
 * including open clk or regulator/open isr/ exit ulps
 * @idle_ctx: idle_ctx object
 */
void dpu_cmd_idle_exit(struct dpu_idle_ctx *idle_ctx);

/**
 * dpu_cmd_idle_deinit - deinit cmd idle object
 *
 * @idle_ctx: idle_ctx object
 */
void dpu_cmd_idle_deinit(struct dpu_idle_ctx *idle_ctx);
#endif
