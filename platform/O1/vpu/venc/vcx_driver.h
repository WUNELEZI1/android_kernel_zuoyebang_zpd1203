/*
 * SPDX_license-Identifier: GPL-2.0  WITH Linux-syscall-note OR BSD-3-Clause
 * Copyright (c) 2015, Verisilicon Inc. - All Rights Reserved
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

#ifndef _VCX_DRIVER_H_
#define _VCX_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __FREERTOS__
/* needed for the _IOW etc stuff used later */
#include "base_type.h"
#include "osal.h"
#include "dev_common_freertos.h"
#elif defined(__linux__)
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#else //For other os
//TODO...
#endif
#include "vcx_mmu.h"
#include "vcx_axife.h"

#ifdef __FREERTOS__
//ptr_t has been defined in base_type.h //Now the FreeRTOS mem need to support 64bit env
#define BIT(a) (1 << (a))
#elif defined(__linux__)
#undef ptr_t
#define ptr_t PTR_T_KERNEL
typedef size_t ptr_t;
typedef size_t addr_t;
#endif
/*
 * Macros to help debugging
 */

#undef PDEBUG /* undef it, just in case */
#ifdef HANTRO_DRIVER_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) pr_info("vcx: " fmt, ##args)
#else
/* This one for user space */
#define PDEBUG(fmt, args...) printf(__FILE__ ":%d: " fmt, __LINE__, ##args)
#endif
#else
#define PDEBUG(fmt, ...) /* not debugging: nothing */
#endif

extern char *enc_dev_n;

#define ENC_HW_ID1                  0x48320100
#define ENC_HW_ID2                  0x80006000
#define CORE_INFO_MODE_OFFSET       31
#define CORE_INFO_AMOUNT_OFFSET     28

/* Use 'k' as magic number */
#define HANTRO_IOC_MAGIC 'k'

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */

#define HANTRO_IOCG_HWOFFSET                           _IOR(HANTRO_IOC_MAGIC,  3, unsigned long *)
#define HANTRO_IOCG_HWIOSIZE                           _IOR(HANTRO_IOC_MAGIC,  4, unsigned int *)
#define HANTRO_IOC_CLI                                 _IO(HANTRO_IOC_MAGIC,  5)
#define HANTRO_IOC_STI                                 _IO(HANTRO_IOC_MAGIC,  6)
#define HANTRO_IOCX_VIRT2BUS                           _IOWR(HANTRO_IOC_MAGIC,  7, unsigned long *)
#define HANTRO_IOCH_ARDRESET                           _IO(HANTRO_IOC_MAGIC, 8)   /* debugging tool */
#define HANTRO_IOCG_SRAMOFFSET                         _IOR(HANTRO_IOC_MAGIC,  9, unsigned long *)
#define HANTRO_IOCG_SRAMEIOSIZE                        _IOR(HANTRO_IOC_MAGIC,  10, unsigned int *)
#define HANTRO_IOCH_ENC_RESERVE                        _IOR(HANTRO_IOC_MAGIC, 11, unsigned int *)
#define HANTRO_IOCH_ENC_RELEASE                        _IOR(HANTRO_IOC_MAGIC, 12, unsigned int *)
#define HANTRO_IOCG_CORE_NUM                           _IOR(HANTRO_IOC_MAGIC, 13, unsigned int *)
#define HANTRO_IOCG_CORE_INFO                          _IOR(HANTRO_IOC_MAGIC, 14, SUBSYS_CORE_INFO *)
#define HANTRO_IOCG_CORE_WAIT                          _IOR(HANTRO_IOC_MAGIC, 15, unsigned int *)
#define HANTRO_IOCG_ANYCORE_WAIT                       _IOR(HANTRO_IOC_MAGIC, 16, CORE_WAIT_OUT *)
#define HANTRO_IOCG_ANYCORE_WAIT_POLLING               _IOR(HANTRO_IOC_MAGIC, 17, CORE_WAIT_OUT *)
#define HANTRO_IOCG_ENABLE_CORE                        _IOR(HANTRO_IOC_MAGIC, 18, unsigned int *)

#define HANTRO_IOCH_GET_CMDBUF_PARAMETER               _IOWR(HANTRO_IOC_MAGIC, 25, struct cmdbuf_mem_parameter *)
#define HANTRO_IOCH_GET_CMDBUF_POOL_SIZE               _IOWR(HANTRO_IOC_MAGIC, 26, unsigned long)
#define HANTRO_IOCH_SET_CMDBUF_POOL_BASE               _IOWR(HANTRO_IOC_MAGIC, 27, unsigned long)
#define HANTRO_IOCH_GET_VCMD_PARAMETER                 _IOWR(HANTRO_IOC_MAGIC, 28, struct config_parameter *)
#define HANTRO_IOCH_RESERVE_CMDBUF                     _IOWR(HANTRO_IOC_MAGIC, 29, struct exchange_parameter *)
#define HANTRO_IOCH_LINK_RUN_CMDBUF                    _IOR(HANTRO_IOC_MAGIC, 30, u16 *)
#define HANTRO_IOCH_WAIT_CMDBUF                        _IOR(HANTRO_IOC_MAGIC, 31, u16 *)
#define HANTRO_IOCH_RELEASE_CMDBUF                     _IOR(HANTRO_IOC_MAGIC, 32, u16 *)
#define HANTRO_IOCH_POLLING_CMDBUF                     _IOR(HANTRO_IOC_MAGIC, 33, u16 *)

#define HANTRO_IOCH_GET_VCMD_ENABLE                    _IOWR(HANTRO_IOC_MAGIC, 50, unsigned long)
#define HANTRO_IOCH_GET_MMU_ENABLE                     _IOWR(HANTRO_IOC_MAGIC, 51, unsigned long)
#define HANTRO_IOCH_SET_FIXED_FREQ                     _IOWR(HANTRO_IOC_MAGIC, 52, u32)
#define HANTRO_IOCH_GET_PM_SUPPORT                     _IOWR(HANTRO_IOC_MAGIC, 52, unsigned long)

#define GET_ENCODER_IDX(type_info)                    (CORE_VCE)
#define CORETYPE(core)                                (1 << (core))
#define HANTRO_IOC_MAXNR                              60

/*priority support*/

#define MAX_CMDBUF_PRIORITY_TYPE   2 //0:normal priority,1:high priority
#define MAX_VCMD_CORE_NUM 4

#define CMDBUF_PRIORITY_NORMAL     0
#define CMDBUF_PRIORITY_HIGH       1

/* power management support: 0 - not support, 1 - support
 * if defined CONFIG_ENC_PM, should set it as 1
 */
#define HANTRO_PM_SUPPORT          0

/******************************************************************************
 * VCMD Command Definitions
 ******************************************************************************/
#define UNSIGNED_OPCODE(A)        ((u32)(A))
#define OPCODE_MASK               (UNSIGNED_OPCODE(0x1F) << 27)
#define OPCODE_WREG               (UNSIGNED_OPCODE(0x01) << 27)
#define OPCODE_END                (UNSIGNED_OPCODE(0x02) << 27)
#define OPCODE_NOP                (UNSIGNED_OPCODE(0x03) << 27)
#define OPCODE_RREG               (UNSIGNED_OPCODE(0x16) << 27)
#define OPCODE_INT                (UNSIGNED_OPCODE(0x18) << 27)
#define OPCODE_JMP                (UNSIGNED_OPCODE(0x19) << 27)
#define OPCODE_STALL              (UNSIGNED_OPCODE(0x09) << 27)
#define OPCODE_CLRINT             (UNSIGNED_OPCODE(0x1a) << 27)
#define OPCODE_JMP_RDY0           (UNSIGNED_OPCODE(0x19) << 27)
#define OPCODE_JMP_RDY1           ((UNSIGNED_OPCODE(0x19) << 27) | (1 << 26))
#define OPCODE_M2M                (UNSIGNED_OPCODE(0x1b) << 27)
#define OPCODE_MSET               (UNSIGNED_OPCODE(0x1c) << 27)
#define OPCODE_M2MP               (UNSIGNED_OPCODE(0x1d) << 27)
#define JMP_IE_1                  BIT(25)
#define JMP_RDY_1                 BIT(26)

#define CLRINT_OPTYPE_READ_WRITE_1_CLEAR         0
#define CLRINT_OPTYPE_READ_WRITE_0_CLEAR         1
#define CLRINT_OPTYPE_READ_CLEAR                 2

#define VCE_FRAME_RDY_INT_MASK                        0x0001
#define VCE_CUTREE_RDY_INT_MASK                       0x0002
#define VCE_DEC400_INT_MASK                           0x0004
#define VCE_L2CACHE_INT_MASK                          0x0008
#define VCE_MMU_INT_MASK                              0x0010
#define CUTREE_MMU_INT_MASK                               0x0020

#define VCD_FRAME_RDY_INT_MASK                        0x0100
#define VCD_DEC400_INT_MASK                           0x0400
#define VCD_L2CACHE_INT_MASK                          0x0800
#define VCD_MMU_INT_MASK                              0x1000

#define VCD_DEC400_INT_MASK_1_1_1                      0x0200
#define VCD_L2CACHE_INT_MASK_1_1_1                     0x0400
#define VCD_MMU_INT_MASK_1_1_1                         0x0800

/******************************************************************************
 * VCMD HW version IDs
 ******************************************************************************/
#define VCMD_HW_ID                 0x4342
#define HW_ID_1_0_C                0x43421001
#define HW_ID_1_1_1                0x43421101
#define HW_ID_1_1_2                0x43421102
#define HW_ID_1_1_3                0x43421103
#define HW_ID_1_2_1                0x43421201
#define HW_ID_1_5_0                0x43421500
#define HW_ID_1_5_2                0x43421502
#define HW_ID_1_5_3                0x43421503
#define HW_ID_1_5_9                0x43421509

/* VCE HW ID */
#define VCE_HW_ID                  0x9000

/****************************************************************************
 * VCARB version
 ****************************************************************************/
#define VCARB_VERSION_2_0         0x2  // arbiter2.0, dual-os version
#define VCARB_VERSION_3_0         0x3  // arbiter3.0, virtulization version

#define VCMD_SLICE_RDY_INTERRUPT 510
#define VCMD_SLICE_RDY_NUM       511

/* time to wotchdog wait for one job */
#ifdef VPU_PLATFORM_FPGA
#define ONE_JOB_WAIT_TIME                        (500 * 10 * 10 * 12)	//For FPGA platform
#else
#define ONE_JOB_WAIT_TIME                        (500)	//For SoC platform
#endif

/* for 5Mhz fpga platform, one hw cycle is 200ns
 */
#define ARB_CYCLE_TO_CPU_TIME                     (200L)
/* if vcmd arbiter is hang, reset it.
 * reset time is one time_window.
 */
extern u32 arbiter_timewindow;
#define VCMD_ARBITER_RESET_TIME ((1 << arbiter_timewindow) * \
								  ARB_CYCLE_TO_CPU_TIME / 1000000) //ms

/* VF timeout time for arbiter fe_timeout/bus_hack to reset vcmd */
#define SW_TIMEOUT_TIME_FOR_ARBITER              (ONE_JOB_WAIT_TIME * 10) // customer to set.

#define VCMD_TIMER_TIMEOUT_INTERVAL              (350) // ms

#ifndef ASIC_SWREG_AMOUNT
#define ASIC_SWREG_AMOUNT                  512 //from encswhwregister.h
#endif

#ifdef HANTROVCMD_ENABLE_IP_SUPPORT
#define VCMD_REG_ID_SW_INIT_CMD0        32
#define ASIC_VCMD_SWREG_AMOUNT          64
#else
#define ASIC_VCMD_SWREG_AMOUNT          31
#endif

/*these size need to be modified according to hw config.*/
#define ENCODER_REGISTER_SIZE              ASIC_SWREG_AMOUNT
#define DECODER_REGISTER_SIZE              512
#define IM_REGISTER_SIZE                   ASIC_SWREG_AMOUNT
#define JPEG_ENCODER_REGISTER_SIZE         ASIC_SWREG_AMOUNT
#define JPEG_DECODER_REGISTER_SIZE         512

#define VCMD_REGISTER_SIZE                 ASIC_VCMD_SWREG_AMOUNT

#define DEC400_REGISTER_SIZE               1600
#define MMU_REGISTER_SIZE                  500
#define L2CACHE_REGISTER_SIZE              500
#define AXIFE_REGISTER_SIZE                500
#define UFBC_REGISTER_SIZE                 10
#define SLCIE_ADDR_OFFSET                  0x100

#define ASIC_STATUS_SEGMENT_READY       0x1000
#define ASIC_STATUS_FUSE_ERROR          0x200
#define ASIC_STATUS_SLICE_READY         0x100
#define ASIC_STATUS_LINE_BUFFER_DONE    0x080  /* low latency */
#define ASIC_STATUS_POLL_SLICEINFO_TIMEOUT 0x2000
#ifdef LOW_LATENCY_SLICEINFO_SUPPORT
#define SLICEINFO_ADDR_OFFSET              0x200
#endif

#define ASIC_STATUS_UFBC_DEC_ERR 0x4000  //bit14
#define ASIC_STATUS_UFBC_CFG_ERR 0x8000  //bit15

#ifndef ASIC_STATUS_HW_TIMEOUT
#define ASIC_STATUS_HW_TIMEOUT 0x040
#endif

#define ASIC_STATUS_BUFF_FULL           0x020
#define ASIC_STATUS_HW_RESET            0x010
#define ASIC_STATUS_ERROR               0x008
#define ASIC_STATUS_FRAME_READY         0x004
#define ASIC_IRQ_LINE                   0x001

#ifndef ASIC_STATUS_ALL
#define ASIC_STATUS_ALL       (ASIC_STATUS_SEGMENT_READY |\
			       ASIC_STATUS_FUSE_ERROR |\
			       ASIC_STATUS_SLICE_READY |\
			       ASIC_STATUS_LINE_BUFFER_DONE |\
			       ASIC_STATUS_HW_TIMEOUT |\
			       ASIC_STATUS_BUFF_FULL |\
			       ASIC_STATUS_HW_RESET |\
			       ASIC_STATUS_ERROR |\
			       ASIC_STATUS_FRAME_READY |\
				   ASIC_STATUS_UFBC_DEC_ERR |\
				   ASIC_STATUS_UFBC_CFG_ERR |\
				   ASIC_STATUS_POLL_SLICEINFO_TIMEOUT)
#endif

enum {
	CORE_VCE = 0,
	CORE_VCEJ = 1,
	CORE_CUTREE = 2,
	CORE_DEC400 = 3,
	CORE_MMU = 4,
	CORE_L2CACHE = 5,
	CORE_AXIFE = 6,
	CORE_APBFT = 7,
	CORE_MMU_1 = 8,
	CORE_AXIFE_1 = 9,
	CORE_UFBC = 10,
	CORE_VCMD = 11,
	CORE_MAX
};

//#define CORE_MAX  (CORE_MMU)

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
	u32 *cmd_virt_addr; //cmdbuf pool base virtual address
	ptr_t cmd_phy_addr; //cmdbuf pool base physical address, it's for cpu
	ptr_t cmd_hw_addr; //cmdbuf pool base hardware address, it's for hardware ip
	u32 cmd_total_size; //cmdbuf pool total size in bytes.
	u16 cmd_unit_size; //one cmdbuf size in bytes. all cmdbuf have same size.
	u32 *status_virt_addr;
	ptr_t status_phy_addr; //status cmdbuf pool base physical address, it's for cpu
	ptr_t status_hw_addr; //status cmdbuf pool base hardware address, it's for hardware ip
	u32 status_total_size; //status cmdbuf pool total size in bytes.
	u16 status_unit_size; //one status cmdbuf size in bytes. all status cmdbuf have same size.
	ptr_t base_ddr_addr; //for pcie interface, hw can only access phy_cmdbuf_addr-pcie_base_ddr_addr.
		//for other interface, this value should be 0?
	u32 *reg_virt_addr; //register cmdbuf pool base virtual address
	ptr_t reg_phy_addr; //register cmdbuf pool base physical address, it's for cpu
	ptr_t reg_hw_addr; //register cmdbuf pool base hardware address, it's for hardware ip
	u32 reg_total_size; //register cmdbuf pool total size in bytes.
	u32 reg_unit_size; //one reg cmdbuf size in bytes. all status cmdbuf have same size.
};

struct config_parameter {
	u16 module_type; //input vce=0,cutree=1,vcd=2,pege=3, jpegd=4
	u16 vcmd_core_num; //output, how many vcmd cores are there with corresponding module_type.
	u16 submodule_main_addr; //output,if submodule addr == 0xffff, this submodule does not exist.
	u16 status_main_addr; //output, main ip offset instatus buffer.
	u16 submodule_dec400_addr; //output ,if submodule addr == 0xffff, this submodule does not exist.
	u16 status_dec400_addr; //output, dec400 ip offset instatus buffer.
	u16 submodule_L2Cache_addr; //output,if submodule addr == 0xffff, this submodule does not exist.
	u16 status_L2Cache_addr; //output, L2Cache ip offset instatus buffer.

	u16 submodule_MMU_addr[2]; //output,if submodule addr == 0xffff, this submodule does not exist.
	u16 status_MMU_addr[2]; //output, MMU ip offset instatus buffer.
	u16 submodule_axife_addr[2]; //output,if submodule addr == 0xffff, this submodule does not exist.
	u16 status_axife_addr[2]; //output, axife ip offset instatus buffer.
	u16 submodule_ufbc_addr; //output ,if submodule addr == 0xffff, this submodule does not exist.
	u16 status_ufbc_addr; //output, ufbc ip offset instatus buffer.
	u32 vcmd_hw_version_id;
	u32 vcmd_priority[MAX_VCMD_CORE_NUM]; //specify the priority of vcmd
};

/*need to consider how many memory should be allocated for status.*/
struct exchange_parameter {
	/** control interrupt mode when generate JMP command
	 * bit31 is mode_flag.
	 *	- when mode_flag is 0, adapative interrupt mode is selected. in such
	 *	  mode, bit[30:0] is executing time estimated for current job;
	 *	- when mode_flag is 1, manual interrupt mode is selected. in such mode,
	 *	  bit[0] is used to set IE flag in JMP command.
	 *	  bit[39:32] is batch count.
	 */
	u64 interrupt_ctrl; //input ;executing_time=encoded_image_size*(rdoLevel+1)*(rdoq+1);
	u16 module_type; //input input vce=0,IM=1,vcd=2,pege=3, jpegd=4
	u16 cmdbuf_size; //input, reserve is not used; link and run is input.
	u16 priority; //input,normal=0, high/live=1
	u16 cmdbuf_id; //output ,it is unique in driver.
	u16 core_id; //just used for polling.
	u16 core_mask; //core_mask for user to select cores
	/* last cmd is JMP (0) or END (1) command */
	u16 has_end_cmd;
	u32 slice_num; //slice number used by wfd
	u32 slice_en; //slice enable
	u64 frequency; //frequency
	u64 timestamp; // input timestamp
};

typedef struct CoreWaitOut {
	u32 job_id[4];
	u32 irq_status[4];
	u32 irq_num;
} CORE_WAIT_OUT;

typedef struct {
	u32 subsys_idx;
	u32 core_type;
	unsigned long offset;
	u32 reg_size;
	int irq;
} CORE_CONFIG;

typedef struct {
	unsigned long base_addr;
	u32 iosize;
	u32 resource_shared; //indicate the core share resources with other cores or not.If 1, means cores can not work at the same time.
	u32 asic_id;
} SUBSYS_CONFIG;

typedef struct {
	u32 type_info; //indicate which IP is contained in this subsystem and each uses one bit of this variable
	unsigned long offset[CORE_MAX];
	unsigned long regSize[CORE_MAX];
	int irq[CORE_MAX];
} SUBSYS_CORE_INFO;

typedef struct {
	SUBSYS_CONFIG cfg;
	SUBSYS_CORE_INFO core_info;
} SUBSYS_DATA;

#ifdef __cplusplus
}
#endif

#endif /* !_VCX_DRIVER_H_ */
