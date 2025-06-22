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

#ifndef __XR_RESET_COMMON_H__
#define __XR_RESET_COMMON_H__

#define XR_RST_METHOD_SCR   0
#define XR_RST_METHOD_CNT   1

/* 8bit for id, 8bit for shift, 16bit for offset */
#define XR_RST_SHIFT_BASE    0
#define XR_RST_SHIFT_WIDTH   8
#define XR_RST_OFFSET_BASE   (XR_RST_SHIFT_BASE + XR_RST_SHIFT_WIDTH)
#define XR_RST_OFFSET_WIDTH  16
#define XR_RST_ID_BASE       (XR_RST_OFFSET_BASE + XR_RST_OFFSET_WIDTH)
#define XR_RST_ID_WIDTH      8

#define MASK_GEN(width)      ((1 << width) - 1)
#define FIELD_PACK(field, base, width) (((u32)(field) & MASK_GEN(width)) << (base))
#define FIELD_UNPACK(para, base, width) (((para) >> (base)) & MASK_GEN(width))

#define XR_RST_SMC_PARA_PACK(id, offset, shift) \
		(FIELD_PACK(id, XR_RST_ID_BASE, XR_RST_ID_WIDTH) |             \
		 FIELD_PACK(offset, XR_RST_OFFSET_BASE, XR_RST_OFFSET_WIDTH) | \
		 FIELD_PACK(shift, XR_RST_SHIFT_BASE, XR_RST_SHIFT_WIDTH))

#define XR_RST_SMC_PARA_UNPACK(para, id, offset, shift)                       \
{                                                                             \
	id     = FIELD_UNPACK(para, XR_RST_ID_BASE, XR_RST_ID_WIDTH);         \
	offset = FIELD_UNPACK(para, XR_RST_OFFSET_BASE, XR_RST_OFFSET_WIDTH); \
	shift  = FIELD_UNPACK(para, XR_RST_SHIFT_BASE, XR_RST_SHIFT_WIDTH);   \
}

#endif /* __XR_RESET_COMMON_H__ */
