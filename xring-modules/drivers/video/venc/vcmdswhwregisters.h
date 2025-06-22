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

 /*------------------------------------------------------------------------------
  *
  *   Table of contents
  *
  *   1. Include headers
  *   2. External compiler flags
  *   3. Module defines
  *
  *-----------------------------------------------------------------------------
  */
#ifndef VCMD_SWHWREGISTERS_H
#define VCMD_SWHWREGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif
/*------------------------------------------------------------------------------
 *   1. Include headers
 *------------------------------------------------------------------------------
 */
#ifdef __FREERTOS__
#include "base_type.h"
#include "io_tools.h"
#elif defined(__linux__)
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>
#endif

#include "venc_log.h"
#ifdef __FREERTOS__
//ptr_t has been defined in base_type.h //Now the FreeRTOS mem need to support 64bit env
#elif defined(__linux__)
typedef int i32;
typedef size_t xptr_t;
#endif

#undef PDEBUG /* undef it, just in case */
#ifdef REGISTER_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) venc_reg_klog(LOGLVL_VERBOSE, fmt, ##args)
#else
/* This one for user space */
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ##args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

/*------------------------------------------------------------------------------
 *   2. External compiler flags
 *------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------
 *   3. Module defines
 *------------------------------------------------------------------------------
 */
#ifdef HANTROVCMD_ENABLE_IP_SUPPORT
#define VCMD_REG_ID_SW_INIT_CMD0        32
#define ASIC_VCMD_SWREG_AMOUNT                  64
#else
#define ASIC_VCMD_SWREG_AMOUNT                  31
#endif
#define VCMD_REGISTER_SW_EXT_INT_SRC_OFFSET      0X8
#define VCMD_REGISTER_EXE_CMDBUF_COUNT_OFFSET    0X0C
#define VCMD_REGISTER_HW_APB_ARBITER_MODE_OFFSET 0x3C
#define VCMD_REGISTER_HW_INIT_MODE_OFFSET        0x3C
#define VCMD_REGISTER_CONTROL_OFFSET             0X40
#define VCMD_REGISTER_INT_STATUS_OFFSET          0X44
#define VCMD_REGISTER_INT_CTL_OFFSET             0X48
#define VCMD_REGISTER_EXT_INT_GATE_OFFSET        0X64
#define VCMD_REGISTER_ARBITER_CONFIG_OFFSET      0X70
#define VCMD_REGISTER_ARB_OFFSET                 0X74

#define VCMD_ARBITER_WEIGHT(w) ((w) << 16)
#define VCMD_ARBITER_URGENT(u) ((u) << 9)
#define VCMD_ARBITER_ENABLE (0x1 << 8)
#define VCMD_ARBITER_BW_OVERFLOW(o) ((o) << 10)
#define VCMD_ARBITER_TIME_WINDOW_EXP(t) (t)
#define VCMD_ARBITER_PARAMS(w, u, o, t)               \
							(VCMD_ARBITER_WEIGHT(w) | \
							 VCMD_ARBITER_URGENT(u) | \
							 VCMD_ARBITER_BW_OVERFLOW(o) | \
							 VCMD_ARBITER_ENABLE | \
							 VCMD_ARBITER_TIME_WINDOW_EXP(t))

#define HW_APB_ARBITER_MODE_BIT         (10)
#define HW_INIT_MODE_BIT                (9)

#define VCMD_IRQ_ARBITER_RESET	(1 << 7)
#define VCMD_IRQ_JMP			(1 << 6)
#define VCMD_IRQ_ARBITER_ERR	(1 << 5)
#define VCMD_IRQ_RESET			(1 << 5)		//valid before HW_ID_1_1_1
#define VCMD_IRQ_ABORT			(1 << 4)
#define VCMD_IRQ_CMD_ERR		(1 << 3)
#define VCMD_IRQ_TIMEOUT		(1 << 2)
#define VCMD_IRQ_BUS_ERR		(1 << 1)
#define VCMD_IRQ_END			(1 << 0)

#define VCMD_IRQ_ERR_MASK	(VCMD_IRQ_ARBITER_RESET | \
	 						 VCMD_IRQ_ARBITER_ERR | \
							 VCMD_IRQ_ABORT | \
							 VCMD_IRQ_CMD_ERR | \
							 VCMD_IRQ_TIMEOUT | \
							 VCMD_IRQ_BUS_ERR)

#define SEND_SLICE_FRAME_READY 0
#define SEND_SLICE_SLICE_READY 1

/* HW Register field names */
typedef enum {
#include "vcmdregisterenum.h"
	VcmdRegisterAmount
} regVcmdName;

/* HW Register field descriptions */
typedef struct {
	u32 name; /* Register name and index  */
	i32 base; /* Register base address  */
	u32 mask; /* Bitmask for this field */
	i32 lsb; /* LSB for this field [31..0] */
	i32 trace; /* Enable/disable writing in swreg_params.trc */
	i32 rw; /* 1=Read-only 2=Write-only 3=Read-Write */
	char *description; /* Field description */
} regVcmdField_s;

/* Flags for read-only, write-only and read-write */
#define RO 1
#define WO 2
#define RW 3

#define REGBASE(reg) (asicVcmdRegisterDesc[reg].base)

/* Description field only needed for system model build. */
#ifdef TEST_DATA
#define VCMDREG(name, base, mask, lsb, trace, rw, desc)                        \
	{                                                                      \
		name, base, mask, lsb, trace, rw, desc                         \
	}
#else
#define VCMDREG(name, base, mask, lsb, trace, rw, desc)                        \
	{                                                                      \
		name, base, mask, lsb, trace, rw, ""                           \
	}
#endif

/*------------------------------------------------------------------------------
 *   4. Function prototypes
 *------------------------------------------------------------------------------
 */
extern const regVcmdField_s asicVcmdRegisterDesc[];

/*------------------------------------------------------------------------------
 *
 *   EncAsicSetRegisterValue
 *
 *   Set a value into a defined register field
 *
 *------------------------------------------------------------------------------
 */
static inline void vcmd_set_reg_mirror(u32 *reg_mirror,
						  regVcmdName name, u32 value)
{
	const regVcmdField_s *field;
	u32 regVal;

	field = &asicVcmdRegisterDesc[name];

#ifdef DEBUG_PRINT_REGS
	printf("vcmd_set_register_mirror_value 0x%2x  0x%08x  Value: %10u  %s\n",
	       field->base, field->mask, value, field->description);
#endif

	/* Check that value fits in field */
	venc_reg_klog(LOGLVL_VERBOSE, "field->name == name=%d\n", field->name == name);
	venc_reg_klog(LOGLVL_VERBOSE, "((field->mask >> field->lsb) << field->lsb) == field->mask=%d\n",
	       ((field->mask >> field->lsb) << field->lsb) == field->mask);
	venc_reg_klog(LOGLVL_VERBOSE, "(field->mask >> field->lsb) >= value=%d\n",
	       (field->mask >> field->lsb) >= value);
	venc_reg_klog(LOGLVL_VERBOSE, "field->base < ASIC_VCMD_SWREG_AMOUNT * 4=%d\n",
	       field->base < ASIC_VCMD_SWREG_AMOUNT * 4);

	/* Clear previous value of field in register */
	regVal = reg_mirror[field->base / 4] & ~(field->mask);

	/* Put new value of field in register */
	reg_mirror[field->base / 4] =
		regVal | ((value << field->lsb) & field->mask);
}

static inline u32 vcmd_get_reg_mirror(u32 *reg_mirror,
						 regVcmdName name)
{
	const regVcmdField_s *field;
	u32 regVal;

	field = &asicVcmdRegisterDesc[name];

	/* Check that value fits in field */
	venc_reg_klog(LOGLVL_VERBOSE, "field->name == name=%d\n", field->name == name);
	venc_reg_klog(LOGLVL_VERBOSE, "((field->mask >> field->lsb) << field->lsb) == field->mask=%d\n",
	       ((field->mask >> field->lsb) << field->lsb) == field->mask);
	venc_reg_klog(LOGLVL_VERBOSE, "field->base < ASIC_VCMD_SWREG_AMOUNT * 4=%d\n",
	       field->base < ASIC_VCMD_SWREG_AMOUNT * 4);

	regVal = reg_mirror[field->base / 4];
	regVal = (regVal & field->mask) >> field->lsb;

#ifdef DEBUG_PRINT_REGS
	venc_reg_klog(LOGLVL_VERBOSE, "vcmd_get_register_mirror_value 0x%2x  0x%08x  Value: %10d  %s\n",
	       field->base, field->mask, regVal, field->description);
#endif
	return regVal;
}

u32 vcmd_read_reg(const void *hwregs, u32 offset);

void vcmd_write_reg(const void *hwregs, u32 offset, u32 val);

void vcmd_write_register_value(const void *hwregs, u32 *reg_mirror,
			       regVcmdName name, u32 value);

u32 vcmd_get_register_value(const void *hwregs, u32 *reg_mirror,
			    regVcmdName name);

#define vcmd_set_addr_register_value(reg_base, reg_mirror, name, value)        \
	do {                                                                   \
		if (sizeof(xptr_t) == 8) {                                      \
			vcmd_write_register_value((reg_base), (reg_mirror), name, (u32)((xptr_t)value));  \
			vcmd_write_register_value((reg_base), (reg_mirror), name##_MSB, (u32)(((xptr_t)value) >> 32));  \
		} else {                                                       \
			vcmd_write_register_value((reg_base), (reg_mirror), name, (u32)((xptr_t)value));  \
		}                                                              \
	} while (0)

#define VCMDGetAddrRegisterValue(reg_base, reg_mirror, name)                   \
	((sizeof(xptr_t) == 8) ?                                                \
		 ((((xptr_t)vcmd_get_register_value((reg_base), (reg_mirror), name)) |            \
		   (((xptr_t)vcmd_get_register_value((reg_base), (reg_mirror), name##_MSB)) << 32))) :                                                 \
		 ((xptr_t)vcmd_get_register_value((reg_base), (reg_mirror), (name))))

#ifdef __cplusplus
}
#endif
#endif /* VCMD_SWHWREGISTERS_H */
