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

#ifndef _DPU_CAP_H_
#define _DPU_CAP_H_

#define MAX_PLANE_COUNT                    32
#define MAX_CRTC_COUNT                     3
#define MAX_WB_COUNT                       2
#define MAX_PLANE_ZPOS                     15
#define MAX_MIXER_LAYER_COUNT              (MAX_PLANE_ZPOS + 1)
#define MAX_PIPELINE_COUNT                 MAX_CRTC_COUNT
#define MAX_CLONE_WB_COUNT                 MAX_WB_COUNT

#define MIN_DISPLAY_WIDTH                  0
#define MAX_DISPLAY_WIDTH                  4096
#define MIN_DISPLAY_HEIGHT                 0
#define MAX_DISPLAY_HEIGHT                 4096

#define MIN_HW_RCH_WIDTH                   1
#define MAX_HW_RCH_WIDTH                   4096
#define MIN_HW_RCH_HEIGHT                  1
#define MAX_HW_RCH_HEIGHT                  4096

#define MAX_HW_RCH_SCALE_DOWN              2
#define MAX_HW_RCH_SCALE_UP                3
#define MAX_HW_RCH_SCALE_INPUT_HEIGHT      8192
#define MIN_HW_RCH_SCALE_INPUT_HEIGHT      10
#define MAX_HW_RCH_V0_SCALE_INPUT_WIDTH    3000
#define MAX_HW_RCH_VX_SCALE_INPUT_WIDTH    4096
#define MIN_HW_RCH_SCALE_INPUT_WIDTH       10
#define MAX_HW_RCH_SCALE_OUTPUT_HEIGHT     8192
#define MIN_HW_RCH_SCALE_OUTPUT_HEIGHT     4
#define MAX_HW_RCH_SCALE_OUTPUT_WIDTH      4096
#define MIN_HW_RCH_SCALE_OUTPUT_WIDTH      4

/* MAX_RES_PIPELINE_COUNT = MAX_PIPELINE_COUNT + VIRTUAL_CTRL_COUNT(1) */
#define MAX_RES_PIPELINE_COUNT             (MAX_PIPELINE_COUNT + 1)
#define VIRTUAL_CTRL_PIPE_ID               0xff

/**
 * solid/background color format ARGB [38:0],
 * little endian: A[37:30], R[29:20], G[19:10], B[9:0]
 */
#define FMT_ARGB_VALUE_MASK                     0x3FFFFFFFFF

#define MAX_LAYER_MIXER_OUTPUT_WIDTH  4096
#define MAX_LAYER_MIXER_OUTPUT_HEIGHT 4096

#define MAX_WFD_SLICE_CNT 4

#endif /* _DPU_CAP_H_ */
