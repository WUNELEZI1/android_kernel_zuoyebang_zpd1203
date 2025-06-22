/* SPDX-License-Identifier: GPL-2.0 */
/****************************************************************************
 *
 *    The MIT License (MIT)
 *
 *    Copyright (C) 2020  VeriSilicon Microelectronics Co., Ltd.
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a
 *    copy of this software and associated documentation files (the "Software"),
 *    to deal in the Software without restriction, including without limitation
 *    the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *    and/or sell copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *    DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 *    The GPL License (GPL)
 *
 *    Copyright (C) 2020  VeriSilicon Microelectronics Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *****************************************************************************
 *
 *    Note: This software is released under dual MIT and GPL licenses. A
 *    recipient may use this file under the terms of either the MIT license or
 *    GPL License. If you wish to use only one license not the other, you can
 *    indicate your decision by deleting one of the above license notices
 *    in your version of this file.
 *
 *****************************************************************************
 */

#ifndef _SUBSYS_H_
#define _SUBSYS_H_
#include <linux/version.h>
#ifdef __FREERTOS__
#define BIT(a) (1 << (a))
#elif defined(__linux__)
#include <linux/fs.h>
#include <linux/platform_device.h>
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "hantrodec.h"
#else
#include <dt-bindings/xring/platform-specific/vpu/vdec/hantrodec.h>
#endif
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif
/* Functions provided by all other subsystem IP - hantrodec_xxx.c */

/***********************************************/
/* subsys level */
/***********************************************/

#define MAX_SUBSYS_NUM 4 /* up to 4 subsystem (temporary) */
#define HXDEC_MAX_CORES MAX_SUBSYS_NUM /* used in hantro_dec_xxx.c */
#define MAX_REG_COUNT 768
#define VCMD_IRQ_NAME "vdec_irq"
/* SubsysDesc & CoreDesc are used for configuration */
struct SubsysDesc {
	int slice_index; /* slice this subsys belongs to */
	int index; /* subsystem index */
	long base;
};

struct CoreDesc {
	int slice;
	int subsys; /* subsys this core belongs to */
	enum CoreType core_type;
	int offset; /* offset to subsystem base */
	int iosize;
	int irq;
	int has_apb;
};

/* internal config struct (translated from SubsysDesc & CoreDesc) */
struct subsys_config {
	unsigned long base_addr;
	int irq;
	/* identifier for each subsys vc8000e=0,
	 * IM=1,vc8000d=2,jpege=3,jpegd=4
	 */
	u32 subsys_type;
	u32 submodule_offset[HW_CORE_MAX]; /* in bytes */
	u16 submodule_iosize[HW_CORE_MAX]; /* in bytes */

	volatile u8 *submodule_hwregs[HW_CORE_MAX]; /* virtual address */
	int has_apbfilter[HW_CORE_MAX];
};

int ParseVpuDtsiInfo(struct subsys_config *subsys, int *subsys_num,
					int *vcmd, struct platform_device *pdev);

extern unsigned int has_apb_arbiter;
extern unsigned long ddr_offset;
extern char *dec_dev_n;
extern int vcmd_irq_flags;

#ifdef __FREERTOS__
/* nothing */
#elif defined(__linux__)
int hantrovcmd_open(struct inode *inode, struct file *filp);
int hantrovcmd_release(struct inode *inode, struct file *filp);
long hantrovcmd_ioctl(struct file *filp,
				  unsigned int cmd, unsigned long arg);
void *hantrovcmd_init(struct subsys_config *subsys, int subsys_num, void *platformdev);
void hantrovcmd_cleanup(void *_vcmd_mgr);
void vcmd_online_for_arbiter(const volatile u8 *hwregs);
void vcmd_offline_for_arbiter(const volatile u8 *hwregs);
void vcmd_request_arbiter(const volatile u8 *hwregs);
int vcmd_pm_suspend(void *_vcmd_mgr);
int vcmd_pm_resume(void *_vcmd_mgr);
void vcmd_reset_asic(void *vcmd_mgr);
void read_main_module_all_registers(void *vcmd_mgr);
int hantrodec_reset(struct device *dev, bool vcmd_dec);

/******************************************************************************/
/* MMU */
/******************************************************************************/

/* Init MMU, should be called in driver init function. */
enum MMUStatus MMUInit(volatile unsigned char *hwregs);
/* Clean up all data in MMU, should be called in driver cleanup function
 * when rmmod driver
 */
enum MMUStatus MMUCleanup(volatile unsigned char *hwregs[MAX_SUBSYS_NUM][2], void *_platformdev);
/* The function should be called in driver realease function
 * when driver exit unnormally
 */
enum MMUStatus MMURelease(void *filp, volatile unsigned char *hwregs);

enum MMUStatus MMUEnable(volatile unsigned char *hwregs[MAX_SUBSYS_NUM][2], void *_platformdev);

/* Used in kernel to map buffer */
enum MMUStatus MMUKernelMemNodeMap(struct kernel_addr_desc *addr);
/* Used in kernel to unmap buffer */
enum MMUStatus MMUKernelMemNodeUnmap(struct kernel_addr_desc *addr);

long MMUIoctl(unsigned int cmd, void *filp, unsigned long arg,
	      volatile unsigned char *hwregs[MAX_SUBSYS_NUM][2]);

unsigned long long GetMMUAddress(void);
/******************************************************************************/
/* DEC400 */
/******************************************************************************/

/******************************************************************************/
/* AXI FE */
/******************************************************************************/
#endif

#endif
