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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <dt-bindings/xring/platform-specific/memory_layout.h>
#include "fk-audio-memlayout.h"

#define AUDIO_CAL_MEM XR_ADSP_SHARE_MEM_BASE_LOW
#define AUDIO_MBX_MEM (AUDIO_CAL_MEM + CAL_MAX_SIZE)
#define AUDIO_USB_MEM (AUDIO_MBX_MEM + MBH_MAX_SIZE)
#define AUDIO_KWS_MEM (AUDIO_USB_MEM + USB_MAX_SIZE)
#define AUDIO_DSP_LOG_MEM (AUDIO_KWS_MEM + KWS_MAX_SIZE)
#define AUDIO_MMAP_OFFSET (AUDIO_DSP_LOG_MEM + LOG_MAX_SIZE)

void *fk_get_audio_static_mem(int mem_id, uint32_t *phy_addr, size_t size)
{
	uint8_t id = mem_id;
	uint32_t mem_addr = 0;
	void *vir_addr = NULL;

	switch (id) {
	case MBX_MEM:
		mem_addr = AUDIO_MBX_MEM;
		if (size <= MBH_MAX_SIZE) {
			vir_addr = ioremap_wc(mem_addr, size);
			memset(vir_addr, 0x0, size);
		} else
			AUD_LOG_ERR(AUD_COMM, "invalid size");
		break;
	case CAL_MEM:
		mem_addr = AUDIO_CAL_MEM;
		if (size <= CAL_MAX_SIZE) {
			vir_addr = ioremap_wc(mem_addr, size);
			memset(vir_addr, 0x0, size);
		} else
			AUD_LOG_ERR(AUD_COMM, "invalid size");
		break;
	case USB_AUD_MEM:
		mem_addr = AUDIO_USB_MEM;
		if (size <= USB_MAX_SIZE) {
			vir_addr = ioremap_wc(mem_addr, size);
			memset(vir_addr, 0x0, size);
		} else
			AUD_LOG_ERR(AUD_COMM, "invalid size");
		break;
	case KWS_MEM:
		mem_addr = AUDIO_KWS_MEM;
		if (size <= KWS_MAX_SIZE) {
			vir_addr = ioremap_wc(mem_addr, size);
			memset(vir_addr, 0x0, size);
		} else
			AUD_LOG_ERR(AUD_COMM, "invalid size");
		break;
	case LOG_MEM:
		mem_addr = AUDIO_DSP_LOG_MEM;
		if (size <= LOG_MAX_SIZE) {
			vir_addr = ioremap_wc(mem_addr, size);
			memset(vir_addr, 0x0, size);
		} else
			AUD_LOG_ERR(AUD_COMM, "invalid size");
		break;
	case MMAP_OFFSET:
		mem_addr = AUDIO_MMAP_OFFSET;
		if (size <= MMAP_OFFSET_SIZE) {
			vir_addr = ioremap_wc(mem_addr, size);
			memset(vir_addr, 0x0, size);
		} else
			AUD_LOG_ERR(AUD_COMM, "invalid size");
		break;
	default:
		AUD_LOG_ERR(AUD_COMM, "unexpect mem_id");
		break;
	}
	*phy_addr = mem_addr;
	AUD_LOG_DBG(AUD_COMM, "vir_addr=0x%llx,mem_addr = 0x%x",
		(uint64_t)vir_addr, mem_addr);
	return vir_addr;
}
EXPORT_SYMBOL(fk_get_audio_static_mem);

