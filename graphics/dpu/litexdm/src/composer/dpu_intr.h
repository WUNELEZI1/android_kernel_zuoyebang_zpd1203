// SPDX-License-Identifier: GPL-2.0-only
/*
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
#ifndef _DPU_INTR_H_
#define _DPU_INTR_H_
#include "dpu_hw_intr.h"

#define WAIT_INTR_TRIGGER_MAX_MS 100

struct composer;

/**
 * dsi_intr_state - the dsi interrupt state
 * @eof_id: the interrupt id of frame timging eof
 * @vsync_id: the interrupt id of frame timging vsync
 * @vstart_id: the interrupt id of frame timging vstart
 * @underflow_id: the interrupt id of frame timging underflow
 * @cfg_rdy_clr_id: the interrupt id of frame config ready clear
 */
struct dsi_intr_state {
	u32 eof_id;
	u32 vsync_id;
	u32 vstart_id;
	u32 underflow_id;
	u32 cfg_rdy_clr_id;
};

/**
 * wb_intr_state - the wb interrupt state
 * @wb_overflow_intr_id: the interrupt id of wb overflow
 * @pipe_intr_id: the interrupt id of wb pipe
 * @wb_intr_id: the interrupt id of wb frame done
 */
struct wb_intr_state {
	u32 wb_overflow_intr_id;
	u32 pipe_intr_id;
	u32 wb_intr_id;
};

void dpu_irq_init(struct composer *comp);
void dpu_irq_deinit(struct composer *comp);
#endif
