// SPDX-License-Identifier: GPL-2.0
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

/*
 * Table of contents
 * 1. Include header
 * 2. External compiler flags
 * 3. Module defines
 */

/*  1. Include headers  */
#ifdef __FREERTOS__
#define __iomem
#elif defined(__linux__)
#include <asm/io.h>
#endif
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "vcmdswhwregisters.h"
#else
#include <dt-bindings/xring/platform-specific/vpu/vdec/vcmdswhwregisters.h>
#endif
/* NOTE: Don't use ',' in descriptions, because it
 * is used as separator in csv parsing.
 */
const regVcmdField_s asicVcmdRegisterDesc[] = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "vcmdregistertable.h"
#else
#include <dt-bindings/xring/platform-specific/vpu/vdec/vcmdregistertable.h>
#endif
};
#include "vdec_log.h"

/*	2. External compiler flags  */

/*  3. Module defines  */

/* Define this to print debug info for every register write. */
// #define DEBUG_PRINT_REGS

/*
 * Function name   : vcmd_read_reg
 * Description     : Retrive the content of a hadware register
 *		Note: The status register will be read after every MB
 *		so it may be needed to buffer it's content if reading
 *		the HW register is slow.
 * Return type     : u32
 * Argument        : u32 offset
 */
u32 vcmd_read_reg(const void *hwregs, u32 offset)
{
	u32 val;

	val = (u32)ioread32((void __iomem *)hwregs + offset);

	vdec_reg_klog(LOGLVL_VERBOSE, "0x%02x --> %08x\n", offset, val);

	return val;
}

/*
 * Function name   : vcmd_write_reg
 * Description     : Set the content of a hadware register
 * Return type     : void
 * Argument        : u32 offset
 * Argument        : u32 val
 */
void vcmd_write_reg(const void *hwregs, u32 offset, u32 val)
{
	iowrite32(val, (void __iomem *)hwregs + offset);

	vdec_reg_klog(LOGLVL_VERBOSE, "0x%02x with value %08x\n", offset, val);
}

/*
 * vcmd_write_register_value
 *
 * Write a value into a defined register field (write will happens actually).
 *
 */
void vcmd_write_register_value(const void *hwregs, u32 *reg_mirror,
			       regVcmdName name, u32 value)
{
	int base = asicVcmdRegisterDesc[name].base;

	vcmd_set_reg_mirror(reg_mirror, name, value);
	/* write it into HW registers */
	vcmd_write_reg(hwregs, base, reg_mirror[base / 4]);
}

/*
 * vcmd_get_register_value
 *
 * Get an unsigned value from the ASIC registers
 *
 */
u32 vcmd_get_register_value(const void *hwregs, u32 *reg_mirror,
			    regVcmdName name)
{
	const regVcmdField_s *field;
	u32 value;

	field = &asicVcmdRegisterDesc[name];

	vdec_reg_klog(LOGLVL_VERBOSE, "field->base < ASIC_VCMD_SWREG_AMOUNT * 4=%d\n",
	       field->base < ASIC_VCMD_SWREG_AMOUNT * 4);

	value = reg_mirror[field->base / 4] =
		vcmd_read_reg(hwregs, field->base);
	value = (value & field->mask) >> field->lsb;

	return value;
}
