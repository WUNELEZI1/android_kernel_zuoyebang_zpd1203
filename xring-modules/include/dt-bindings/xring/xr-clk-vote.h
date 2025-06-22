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

#ifndef __DT_BINDINGS_CLOCK_XR_O1_VOTE_H__
#define __DT_BINDINGS_CLOCK_XR_O1_VOTE_H__

#define CLK_DPUCORE0_VOTE_ID     1
#define CLK_DPUVEU_VOTE_ID       2
#define CLK_VDEC_VOTE_ID	 3
#define CLK_ISPFUNC1_VOTE_ID	 4
#define CLK_ISPFUNC2_VOTE_ID	 5
#define CLK_ISPFUNC3_VOTE_ID	 6
#define CLK_VENC_VOTE_ID	 7

#define CLK_OCM_VOTE_ID	         8
#define CLK_RSV_VOTE_ID		 9

#define CLK_MAX_VOTE_ID          10

#define PERI_MIN_VOTE_ID CLK_OCM_VOTE_ID
#define PERI_MAX_VOTE_ID CLK_OCM_VOTE_ID
#define MEDIA_MIN_VOTE_ID CLK_DPUCORE0_VOTE_ID
#define MEDIA_MAX_VOTE_ID CLK_VENC_VOTE_ID

#define PERI_VOLT_TABLE_ITEM_NUM 4
#define MEDIA_VOLT_TABLE_ITEM_NUM 21

#endif /* __DT_BINDINGS_CLOCK_XR_O1_VOTE_H__ */
