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

#ifndef __XR_RESET_OP_H__
#define __XR_RESET_OP_H__

#define XR_RST_OP_ASSERT    0
#define XR_RST_OP_DEASSERT  1
#define XR_RST_OP_RESET     2
#define XR_RST_OP_STATUS    3
#define XR_RST_OP_CNT       4

#define RST_BIT(n)     (1 << (n))

#define SCR_W1S_OFFSET 0
#define SCR_W1C_OFFSET 4
#define SCR_RO_OFFSET  8

/* RST_WRITE & RST_READ need implement outside, depends on the os */
#define SCR_RESET_ASSERT(base, offset, shift) \
		RST_WRITE((base) + (offset) + SCR_W1C_OFFSET, RST_BIT(shift))

#define SCR_RESET_DEASSERT(base, offset, shift) \
		RST_WRITE((base) + (offset) + SCR_W1S_OFFSET, RST_BIT(shift))

#define SCR_RESET_STATUS(base, offset, shift) \
		!!(RST_READ((base) + (offset) + SCR_RO_OFFSET) & RST_BIT(shift))

#endif /* __XR_RESET_OP_H__ */
