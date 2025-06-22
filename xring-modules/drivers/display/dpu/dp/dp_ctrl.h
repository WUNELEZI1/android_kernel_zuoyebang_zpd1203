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

#ifndef _DP_CTRL_H_
#define _DP_CTRL_H_

#include <linux/types.h>
#include "dp_hw_ctrl.h"
#include "dp_link.h"
#include "dp_power.h"

struct dp_display;
struct dp_ctrl;

struct dp_ctrl_funcs {
	/**
	 * enable_video - enable/disable video stream transmission of dptx
	 * @ctrl: handle of ctrl
	 * @enable: control the switch
	 *
	 * when @enable is true, dptx will transfer video steam from dpu to sink,
	 * when @enable is false, dptx will transfer training pattern.
	 * training_pattern_none = idle pattern
	 */
	void (*enable_video)(struct dp_ctrl *ctrl, bool enable);

	/**
	 * enable_audio - enable audio transfer
	 * @ctrl: handle of dp ctrl
	 * @ainfo: audio information
	 * @enable: enable flag
	 */
	void (*enable_audio)(struct dp_ctrl *ctrl,
			struct dp_audio_fmt_info *ainfo, bool enable);

	/**
	 * start_link_training - do link training
	 * @ctrl: handle of dp ctrl
	 *
	 * do link training with sink, according to Chapter 3.5.1.2,
	 * VESA DisplayPort Standard, Version 1.4a
	 *
	 * Returns 0 on success or a negative error code on failure.
	 */
	int (*start_link_training)(struct dp_ctrl *ctrl);
};

/**
 * struct dp_ctrl - description of dptx controller
 * @display: callback pointer of dp display
 * @link: software logic for link
 * @power: hardware api for ctrl
 * @audio: hardware api for ctrl
 * @hw_ctrl: hardware api for ctrl
 * @funcs: functions provided by dp_ctrl
 */
struct dp_ctrl {
	struct dp_display *display;

	struct dp_link *link;
	struct dp_power *power;
	struct dp_hw_ctrl *hw_ctrl;

	struct dp_ctrl_funcs *funcs;
};

int dp_ctrl_init(struct dp_display *display, struct dp_ctrl **ctrl);
void dp_ctrl_deinit(struct dp_ctrl *ctrl);
void dp_ctrl_enable_video(struct dp_ctrl *ctrl, bool enable);

#endif
