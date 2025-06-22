/*
 * SPDX_license-Identifier: GPL-2.0  WITH Linux-syscall-note OR BSD-3-Clause
 * Copyright (c) 2015, Verisilicon Inc. - All Rights Reserved
 * Copyright (c) 2011-2014, Google Inc. - All Rights Reserved
 *
 ********************************************************************************
 *
 * GPL-2.0
 *
 ********************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ********************************************************************************
 *
 * Alternatively, This software may be distributed under the terms of
 * BSD-3-Clause, in which case the following provisions apply instead of the ones
 * mentioned above :
 *
 ********************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************************
 */

#ifndef _HANTROMMU_H_
#define _HANTROMMU_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __FREERTOS__
#elif defined(__linux__)
#include <linux/fs.h>
#endif

extern unsigned long ddr_offset;

#define REGION_IN_START          0x0
#define REGION_IN_END            0x40000000
#define REGION_OUT_START         0x40000000
#define REGION_OUT_END           0x80000000
#define REGION_PRIVATE_START     0x80000000
#define REGION_PRIVATE_END       0xc0000000

#define REGION_IN_MMU_START      0x1000
#define REGION_IN_MMU_END        0x40000000
#define REGION_OUT_MMU_START     0x40000000
#define REGION_OUT_MMU_END       0x80000000
#define REGION_PRIVATE_MMU_START 0x80000000
#define REGION_PRIVATE_MMU_END   0xc0000000

#define MMU_REG_OFFSET              0
#define MMU_REG_HW_ID               (MMU_REG_OFFSET + 6*4)
#define MMU_REG_FLUSH               (MMU_REG_OFFSET + 97*4)
#define MMU_REG_PAGE_TABLE_ID       (MMU_REG_OFFSET + 107*4)
#define MMU_REG_CONTROL             (MMU_REG_OFFSET + 226*4)
#define MMU_REG_ADDRESS             (MMU_REG_OFFSET + 227*4)
#define MMU_REG_ADDRESS_MSB         (MMU_REG_OFFSET + 228*4)
#define MMU_REG_PDENTRY0            (MMU_REG_OFFSET + 237*4)

#ifdef EMU
#define MTLB_PCIE_START_ADDRESS  0x04100000
#define PAGE_PCIE_START_ADDRESS  0x04200000 /* page_table_entry start address */
#define STLB_PCIE_START_ADDRESS  0x04300000
#else
#define MTLB_PCIE_START_ADDRESS  (0x00100000 + ddr_offset)
#define PAGE_PCIE_START_ADDRESS  (0x00200000 + ddr_offset) /* page_table_entry start address */
#define STLB_PCIE_START_ADDRESS  (0x00300000 + ddr_offset)
#endif
#define PAGE_TABLE_ENTRY_SIZE    64

#define MMU_REG_SIZE (228 * 4)

enum MMUStatus {
	MMU_STATUS_OK = 0,

	MMU_STATUS_FALSE = -1,
	MMU_STATUS_INVALID_ARGUMENT = -2,
	MMU_STATUS_INVALID_OBJECT = -3,
	MMU_STATUS_OUT_OF_MEMORY = -4,
	MMU_STATUS_NOT_FOUND = -19,
};

struct addr_desc {
	void *virtual_address; /* buffer virtual address */
	size_t bus_address; /* buffer physical address */
	unsigned int size; /* physical size */
};

struct kernel_addr_desc {
	size_t bus_address; /* buffer virtual address */
	size_t mmu_bus_address; /* buffer physical address in MMU*/
	unsigned int size; /* physical size */
};


#define HANTRO_IOC_MMU  'm'

#define HANTRO_IOCS_MMU_MEM_MAP    _IOWR(HANTRO_IOC_MMU, 1, struct addr_desc *)
#define HANTRO_IOCS_MMU_MEM_UNMAP  _IOWR(HANTRO_IOC_MMU, 2, struct addr_desc *)
#define HANTRO_IOCS_MMU_FLUSH      _IOWR(HANTRO_IOC_MMU, 3, unsigned int *)
//#define HANTRO_IOCS_MMU_ENABLE     _IOWR(HANTRO_IOC_MMU, 4, unsigned int *)
#define HANTRO_IOC_MMU_MAXNR 3
#define MAX_SUBSYS_NUM 4 /* up to 4 subsystem (temporary) */
#define HXDEC_MAX_CORES MAX_SUBSYS_NUM /* used in hantro_dec.c */
/* Init MMU, should be called in driver init function. */
enum MMUStatus MMUInit(volatile unsigned char *hwregs);
/* Clean up all data in MMU, should be called in driver cleanup function
 * when rmmod driver
 */
enum MMUStatus MMUCleanup(volatile unsigned char *hwregs[MAX_SUBSYS_NUM][2]);
/* The function should be called in driver realease function
 * when driver exit unnormally
 */
enum MMUStatus MMURelease(void *filp, volatile unsigned char *hwregs);

enum MMUStatus MMUEnable(volatile unsigned char *hwregs[MAX_SUBSYS_NUM][2]);

/* Used in kernel to map buffer */
enum MMUStatus MMUKernelMemNodeMap(struct kernel_addr_desc *addr);

/* Used in kernel to unmap buffer */
enum MMUStatus MMUKernelMemNodeUnmap(struct kernel_addr_desc *addr);

unsigned long long GetMMUAddress(void);
long MMUIoctl(unsigned int cmd, void *filp, unsigned long arg,
	      volatile unsigned char *hwregs[MAX_SUBSYS_NUM][2]);

#ifdef __cplusplus
}
#endif
#endif
