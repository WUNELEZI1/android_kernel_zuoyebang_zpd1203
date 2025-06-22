/* SPDX-License-Identifier: GPL-2.0-only
 *
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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "fk-audio-log.h"

#define CAL_MAX_SIZE 0x100000
#define MBH_MAX_SIZE 0X4000
#define USB_MAX_SIZE 0x2800
#define KWS_MAX_SIZE 0x4B000
#define LOG_MAX_SIZE 0X20000
#define MMAP_OFFSET_SIZE 0x20


enum mem_id {
	MBX_MEM = 0X0,
	CAL_MEM,
	USB_AUD_MEM,
	KWS_MEM,
	LOG_MEM,
	MMAP_OFFSET,
	MEM_MAX,
};
void *fk_get_audio_static_mem(int mem_id, uint32_t *phy_addr, size_t size);

