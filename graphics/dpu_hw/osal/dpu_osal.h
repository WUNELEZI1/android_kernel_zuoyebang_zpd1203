/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DPU_OSAL_H_
#define _DPU_OSAL_H_

/**
 * dpu_log.h need to be implemented in their respective display frameworks
 * The following func/macro must be defined
 * DSI_DEBUG(msg, ...)
 * DPU_ERROR(msg, ...)
 */
#include "dpu_log.h"

/**
 * dpu_reg_ops.h need to be implemented in their respective display frameworks
 * The following func/macro must be defined
 * #define DPU_REG_WRITE(hw, offset, val, force_cpu_write)
 * #define DPU_BIT_WRITE(hw, offset, val, bit_start, bit_num, force_cpu_write)
 * #define DPU_BITMASK_WRITE(hw, offset, val, bit_start, bit_mask, force_cpu_write)
 * #define DPU_BIT_WRITE_NO_LOG(hw, offset, val, bit_start, bit_num)
 * #define DPU_REG_READ(hw, offset)
 * #define DPU_BIT_READ(hw, offset, bit_start, bit_num)
 * #define DPU_USLEEP(us)
 * #define DPU_MSLEEP(ms)
 * #define DPU_WRITE_REG(addr, value)
 * #define DPU_READ_POLL_TIMEOUT(hw, offset, val, cond, delay_us, timeout_us)
 * #define DPU_READ_POLL_TIMEOUT_ATOMIC(hw, offset, val, cond, delay_us, timeout_us)
 */
#include "dpu_reg_ops.h"

#ifdef __KERNEL__
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

typedef void __iomem * DPU_IOMEM;

#define DPU_REG_WRITE_BARE(addr, value) writel((value), (addr))
#define DPU_REG_READ_BARE(addr) readl(addr)

#define REG_MAP(mapped_addr, addr, len) \
	do { \
		(mapped_addr) = ioremap((addr), (len)); \
		if (!(mapped_addr)) \
			DPU_ERROR("addr:0x%x map failed\n", (addr)); \
	} while (0)

#define REG_UNMAP(addr) \
	do { \
		if (addr) { \
			iounmap(addr); \
			(addr) = NULL; \
		} \
	} while (0)
#define dpu_mem_cpy memcpy
#else /* UEFI */

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmSmcLib.h>
#include <smc_id/bl2_smc_id.h>

#define BIT(a) (1UL << (a))

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef u32 DPU_IOMEM;
typedef u64 UINTPTR;

typedef int64_t __s64;
typedef int32_t __s32;
typedef int16_t __s16;
typedef uint64_t __u64;
typedef uint32_t __u32;
typedef uint16_t __u16;
typedef uint8_t __u8;

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define DPU_REG_WRITE_BARE(addr, value) \
	do { \
		*(volatile uint32_t *)(UINTPTR)(addr) = (value); \
		DPU_DEBUG("[W] addr:0x%x value:0x%x\n", (addr), (value)); \
	} while (0)

#define DPU_REG_READ_BARE(addr) ({ \
	uint32_t value; \
	value = *(volatile uint32_t *)(UINTPTR)(addr); \
	DPU_DEBUG("[R] addr:0x%x value:0x%x\n", (addr), (value)); \
	value; \
})

#define __maybe_unused                  __attribute__((__unused__))
#define fallthrough                     __attribute__((__fallthrough__))

#define REG_MAP(mapped_addr, addr, len) \
	do { \
		(mapped_addr) = (addr); \
	} while (0)

#define REG_UNMAP(addr) \
	do { \
		if (addr) \
			addr = 0; \
	} while (0)

#ifndef memcpy
#define memcpy(dst, src, size) CopyMem(dst, src, size)
#endif

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#endif /* __KERNEL__ */

/*
 * if the display framework does not define some unnecessary functions,
 * the following macros are defined to avoid compilation failures
 */
#ifndef DPU_WARN
#define DPU_WARN
#endif

#ifndef DPU_INFO
#define DPU_INFO
#endif

#define EPERM  1        /* Operation not permitted */
#define ENOENT  2       /* No such file or directory */
#define ESRCH  3        /* No such process */
#define EINTR  4        /* Interrupted system call */
#define EIO  5	        /* I/O error */
#define ENXIO  6        /* No such device or address */
#define E2BIG  7        /* Argument list too long */
#define ENOEXEC  8      /* Exec format error */
#define EBADF  9        /* Bad file number */
#define ECHILD 10       /* No child processes */
#define EAGAIN 11       /* Try again */
#define ENOMEM 12       /* Out of memory */
#define EACCES 13       /* Permission denied */
#define EFAULT 14       /* Bad address */
#define ENOTBLK 15      /* Block device required */
#define EBUSY 16        /* Device or resource busy */
#define EEXIST 17       /* File exists */
#define EXDEV 18        /* Cross-device link */
#define ENODEV 19       /* No such device */
#define ENOTDIR 20      /* Not a directory */
#define EISDIR 21       /* Is a directory */
#define EINVAL 22       /* Invalid argument */
#define ENFILE 23       /* File table overflow */
#define EMFILE 24       /* Too many open files */
#define ENOTTY 25       /* Not a typewriter */

#endif /* _DPU_OSAL_H */
