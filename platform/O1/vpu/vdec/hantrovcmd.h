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

#ifndef _VC8000_VCMD_DRIVER_H_
#define _VC8000_VCMD_DRIVER_H_
#ifdef __FREERTOS__
#include "basetype.h"
#include "dev_common_freertos.h"
#elif defined(__linux__)
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#endif

#ifdef __FREERTOS__
/* addr_t has been defined in basetype.h
 * Now the FreeRTOS mem need to support 64bit env
 */
#elif defined(__linux__)
#undef addr_t
#define addr_t ADDR_T_VCMD
typedef size_t addr_t;
#endif

/* Use 'v' as magic number for vcmd */
#define HANTRO_VCMD_IOC_MAGIC  'v'
/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */

#define HANTRO_VCMD_IOCH_GET_CMDBUF_PARAMETER                                  \
	_IOWR(HANTRO_VCMD_IOC_MAGIC, 20, struct cmdbuf_mem_parameter *)
#define HANTRO_VCMD_IOCH_GET_CMDBUF_POOL_SIZE                                  \
	_IOWR(HANTRO_VCMD_IOC_MAGIC, 21, unsigned long)
#define HANTRO_VCMD_IOCH_SET_CMDBUF_POOL_BASE                                  \
	_IOWR(HANTRO_VCMD_IOC_MAGIC, 22, unsigned long)

#define HANTRO_VCMD_IOCH_GET_VCMD_PARAMETER                                    \
	_IOWR(HANTRO_VCMD_IOC_MAGIC, 24, struct config_parameter *)

#define HANTRO_VCMD_IOCH_RESERVE_CMDBUF                                        \
	_IOWR(HANTRO_VCMD_IOC_MAGIC, 25, struct exchange_parameter *)

#define HANTRO_VCMD_IOCH_LINK_RUN_CMDBUF _IOWR(HANTRO_VCMD_IOC_MAGIC, 26, u16 *)
#define HANTRO_VCMD_IOCH_WAIT_CMDBUF _IOWR(HANTRO_VCMD_IOC_MAGIC, 27, struct wait_cmdbuf)
#define HANTRO_VCMD_IOCH_RELEASE_CMDBUF _IOR(HANTRO_VCMD_IOC_MAGIC, 28, u16 *)

#define HANTRO_VCMD_IOCH_POLLING_CMDBUF _IOR(HANTRO_VCMD_IOC_MAGIC, 40, u16 *)
#define HANTRO_VCMD_IOCH_PUSH_SLICE_REG _IOR(HANTRO_VCMD_IOC_MAGIC, 41, void *)
#define HANTRO_VCMD_IOCH_ABORT_CMDBUF _IOR(HANTRO_VCMD_IOC_MAGIC, 42, u16 *)
#define HANTRO_VCMD_IOCH_DROP_OWNER   _IOWR(HANTRO_VCMD_IOC_MAGIC, 43, void *)
#define HANTRO_VCMD_IOCH_WAIT_OWNER_DONE                                       \
	_IOR(HANTRO_VCMD_IOC_MAGIC, 44, void *)
#define HANTRO_VCMD_TOCH_GET_CMDBUF_STATUS \
 _IOWR(HANTRO_VCMD_IOC_MAGIC, 45, struct wait_cmdbuf)

#define HANTRO_VCMD_IOC_MAXNR 50

/*priority support*/

/* 0:normal priority,1:high priority */
#define MAX_CMDBUF_PRIORITY_TYPE          2

#define CMDBUF_PRIORITY_NORMAL            0
#define CMDBUF_PRIORITY_HIGH              1

/* TBD: need to match with MAX_VCMD_ENTRIES in user space */
#define SLOT_NUM_CMDBUF						(127)

/* VCMD memory pool layout as:
 *	0: CMDBUF memory, size is SLOT_NUM_CMDBUF * SLOT_SIZE_CMDBUF
 *	1: STATUSBUF memory, size is SLOT_NUM_CMDBUF * SLOT_SIZE_STATUSBUF
 *	2: REGBUF memory, size is MAX_VCMD_NUM * SLOT_SIZE_REGBUF
 */
#define VCMD_POOL_TOTAL_SIZE     (5 * 1024 * 1024)	/* match with vcmd_size which is module param of memalloc */

#define SLOT_SIZE_CMDBUF         (512 * 4 * 4)
#define SLOT_SIZE_CMDBUF_MIN     (32)
#define SLOT_SIZE_STATUSBUF      (512 * 4 * 4)
#define SLOT_SIZE_REGBUF         (4 * 1024)

/******************************************************************************
 * VCMD HW version IDs
 ******************************************************************************/
#define VCMD_HW_ID                  0x4342
#define HW_ID_1_0_C                 0x43421001
#define HW_ID_1_1_1                 0x43421101
#define HW_ID_1_1_2                 0x43421102
#define HW_ID_1_1_3                 0x43421103
#define HW_ID_1_2_1                 0x43421201
#define HW_ID_1_5_0                 0x43421500

/* VCD HW ID */
#define VCD_HW_ID                  0x9001

/******************************************************************************
 * CMDBUF executing status
 ******************************************************************************/
#define CMDBUF_EXE_STATUS_OK                     0
#define CMDBUF_EXE_STATUS_CMDERR                 1
#define CMDBUF_EXE_STATUS_BUSERR                 2
#define CMDBUF_EXE_STATUS_ABORTED                3
#define CMDBUF_EXE_STATUS_SLICE_DECODING_SUSPEND 4
#define CMDBUF_EXE_STATUS_SLICE_DECODING_ABORTED 5

/* Used in vcmd initialization in hantro_vcmd_xxx.c. */
/* May be unified in next step. */
struct vcmd_config {
	unsigned long vcmd_base_addr;
	u32 vcmd_iosize;
	int vcmd_irq;
	/*input vce=0,IM=1,vcd=2,jpege=3, jpegd=4*/
	u32 sub_module_type;
	u16 submodule_main_addr; // in byte
	/* if submodule addr == 0xffff,
	 * this submodule does not exist.// in byte
	 */
	u16 submodule_dec400_addr;
	u16 submodule_MMU_addr; // in byte
	u16 submodule_MMUWrite_addr; // in byte
	u16 submodule_axife_addr; // in byte

	/* for Hw Register Print */
	volatile u8 *submodule_vcmd_virtual_address;
	volatile u8 *submodule_vcd_virtual_address;
	volatile u8 *submodule_dec400_virtual_address;
	volatile u8 *submodule_MMU_virtual_address;
	volatile u8 *submodule_MMUWrite_virtual_address;
	volatile u8 *submodule_axife_virtual_address;
	u32 submodule_vcd_iosize;
	u32 submodule_dec400_iosize;
	u32 submodule_MMU_iosize;
	u32 submodule_MMUWrite_iosize;
	u32 submodule_axife_iosize;

};
#define ANY_CMDBUF_ID 0xFFFF

/* for 5Mhz fpga platform, one hw cycle is 200ns
 */
#define ARB_CYCLE_TO_CPU_TIME                      (200L)
/* if vcmd arbiter is hang, reset it.
 * reset time is one time_window.
 */
extern u32 arbiter_timewindow;
#define VCMD_ARBITER_RESET_TIME ((1 << arbiter_timewindow) * \
								  ARB_CYCLE_TO_CPU_TIME / 1000000) //ms

/* time to wotchdog wait for one job */
#ifdef VPU_PLATFORM_FPGA
#define ONE_JOB_WAIT_TIME                        (500 * 400)	//For FPGA platform (in ms)
#define ONE_SLICE_WAIT_TIME                      (500) //For FPGA platform (in ms)
#else
#define ONE_JOB_WAIT_TIME                        (800)	//For SoC platform (in ms)
#define ONE_SLICE_WAIT_TIME                      (50) //For SoC platform (in ms)
#endif
#define VCMD_TIMER_TIMEOUT_INTERVAL (350) // ms
#define VCMD_TIMER_BUFFER_EMPTY_INTERVAL (ONE_SLICE_WAIT_TIME * 10)

/*module_type support*/
enum vcmd_module_type {
	VCMD_TYPE_ENCODER = 0,
	VCMD_TYPE_CUTREE,
	VCMD_TYPE_DECODER,
	VCMD_TYPE_JPEG_ENCODER,
	VCMD_TYPE_JPEG_DECODER,
	MAX_VCMD_TYPE
};

struct cmdbuf_mem_parameter {
	u32 *virt_cmdbuf_addr;
	//cmdbuf pool base physical address
	addr_t phy_cmdbuf_addr;
	//cmdbuf pool base mmu mapping address
	u32 mmu_phy_cmdbuf_addr;
	//cmdbuf pool total size in bytes.
	u32 cmdbuf_total_size;
	//one cmdbuf size in bytes. all cmdbuf have same size.
	u16 cmdbuf_unit_size;
	u32 *virt_status_cmdbuf_addr;
	//status cmdbuf pool base physical address
	addr_t phy_status_cmdbuf_addr;
	//status cmdbuf pool base mmu mapping address
	u32 mmu_phy_status_cmdbuf_addr;
	//status cmdbuf pool total size in bytes.
	u32 status_cmdbuf_total_size;
	//one status cmdbuf size in bytes. all status cmdbuf have same size.
	u16 status_cmdbuf_unit_size;
	// reg regbuf pool virtual adress
	u32 *virt_vcmd_regbuf_addr;
	// reg regbuf pool base physical address
	addr_t phy_vcmd_regbuf_addr;
	// reg regbuf pool base mmu mapping address
	u32 mmu_phy_vcmd_regbuf_addr;
	// reg regbuf pool total size in bytes.
	u32 vcmd_regbuf_total_size;
	//one reg regbuf size in bytes. all status cmdbuf have same size.
	u32 vcmd_regbuf_unit_size;
	/* for pcie interface, hw can only access
	 * phy_cmdbuf_addr-pcie_base_ddr_addr.
	 * for other interface, this value should be 0?
	 */
	addr_t base_ddr_addr;
};

struct config_parameter {
	/*input vce=0,cutree=1,vcd=2,jpege=3, jpegd=4 */
	u16 module_type;
	/* output, how many vcmd cores are there
	 * with corresponding module_type.
	 */
	u16 vcmd_core_num;
	/*output,if submodule addr == 0xffff, this submodule does not exist.*/
	u16 submodule_main_addr;
	/* output ,if submodule addr == 0xffff, this submodule does not exist.*/
	u16 submodule_dec400_addr;
	/* output,if submodule addr == 0xffff, this submodule does not exist. */
	u16 submodule_MMU_addr;
	/* output,if submodule addr == 0xffff, this submodule does not exist. */
	u16 submodule_MMUWrite_addr;
	/* output,if submodule addr == 0xffff, this submodule does not exist. */
	u16 submodule_axife_addr;
	u32 vcmd_hw_version_id;
};

/*need to consider how many memory should be allocated for status.*/
struct exchange_parameter {
   /** control interrupt mode when generate JMP command
    * bit31 is mode_flag.
    *  - when mode_flag is 0, adapative interrupt mode is selected. in such
    * 	  mode, bit[30:0] is executing time estimated for current job;
    *	- when mode_flag is 1, manual interrupt mode is selected. in such mode,
    *	  bit[0] is used to set IE flag in JMP command.
    *	  bit[39:32] is batch count.
   */
	u64 interrupt_ctrl;
	/*input input vce=0,IM=1,vcd=2, jpege=3, jpegd=4 */
	u16 module_type;
	/*input, reserve is not used; link and run is input.*/
	u16 cmdbuf_size;
	/* input,normal=0, high/live=1 */
	u16 priority;
	/* output, it is unique in driver.*/
	u16 cmdbuf_id;
	/* just used for polling. */
	u16 core_id;
	/* core_mask for user to select cores: [0,15]core mask, [16,31]client type. */
	u16 core_mask;
	/* last cmd is JMP (0) or END (1) command */
	u16 has_end_cmd;
	/* the instance ctx */
	void *owner;
	/* frequency */
	u64 frequency;
};

/* the wait cmdbuf executing status and id */
struct wait_cmdbuf {
	u32 exe_status; //cmdbuf executing status
	u16 id;  //the waited cmdbuf id
};

struct vcmd_priv_ctx {
	void *vcmd_mgr;
	/* process object */
	void *po;
};

enum {
	DUMP_REGISTER_LEVEL_VERBOSE = 0,			//dump all registers in isr
	DUMP_REGISTER_LEVEL_BRIEF,					//dump brief registers in isr
	DUMP_REGISTER_LEVEL_VERBOSE_ONERROR,		//dump all registers on error
	DUMP_REGISTER_LEVEL_BRIEF_ONERROR,			//dump brief registers on error
};

#endif /* !_VC8000_VCMD_DRIVER_H_ */
