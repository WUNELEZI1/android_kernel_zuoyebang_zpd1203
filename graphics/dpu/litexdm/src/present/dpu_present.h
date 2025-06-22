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

#ifndef _DPU_ONLINE_PRESENT_H_
#define _DPU_ONLINE_PRESENT_H_

#include "osal.h"

struct composer;
struct dpu_frame;

int32_t dpu_rch_cfg(struct composer *comp, struct dpu_frame *frame);
int32_t dpu_scene_ctl_cfg(struct composer *comp, struct dpu_frame *frame);
int32_t dpu_mixer_cfg(struct composer *comp, struct dpu_frame *frame);
int32_t dpu_post_pipe_top_cfg(struct composer *comp);
int32_t dpu_wb_cfg(struct composer *comp, struct dpu_frame *frame);
int32_t dpu_online_present(struct composer *comp, struct dpu_frame *frame);
int32_t dpu_offline_present(struct composer *comp, struct dpu_frame *frame);

int32_t dpu_scene_ctl_clear(struct composer *comp);
#endif
