// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __DFX_MEMORY_LAYOUT__
#define __DFX_MEMORY_LAYOUT__

#include "asic/memory_layout.h"

#define SZ_K(n)         ((n) * 1024)

#define DFX_RESERVED_MEM_ADDR               0x43480000
#define DFX_RESERVED_MEM_SIZE               0xa00000

#define DFX_MEM_RESERVED_HEADER_ADDR        DFX_RESERVED_MEM_ADDR
#define DFX_MEM_RESERVED_HEADER_SIZE        SZ_K(1)

#define DFX_MEM_KERNEL_PARAM_ADDR           (DFX_MEM_RESERVED_HEADER_ADDR + DFX_MEM_RESERVED_HEADER_SIZE)
#define DFX_MEM_KERNEL_PARAM_SIZE           SZ_K(4)

#define DFX_MEM_FASTBOOTLOG_ADDR            (DFX_MEM_KERNEL_PARAM_ADDR + DFX_MEM_KERNEL_PARAM_SIZE)
#define DFX_MEM_FASTBOOTLOG_SIZE            SZ_K(256)

#define DFX_MEM_BOOT_INFO_ADDR              (DFX_MEM_FASTBOOTLOG_ADDR + DFX_MEM_FASTBOOTLOG_SIZE)
#define DFX_MEM_BOOT_INFO_SIZE              SZ_K(1)

#define DFX_MEM_RSV_ADDR                    (DFX_MEM_BOOT_INFO_ADDR + DFX_MEM_BOOT_INFO_SIZE)
#define DFX_MEM_RSV_SIZE                    SZ_K(2)

#define DFX_MEM_RSV1_ADDR                   (DFX_MEM_RSV_ADDR + DFX_MEM_RSV_SIZE)
#define DFX_MEM_RSV1_SIZE                   SZ_K(128)

#define DFX_MEM_RSV2_ADDR                   (DFX_MEM_RSV1_ADDR + DFX_MEM_RSV1_SIZE)
#define DFX_MEM_RSV2_SIZE                   SZ_K(16)

#define DFX_MEM_BL31_MDR_ADDR               (DFX_MEM_RSV2_ADDR + DFX_MEM_RSV2_SIZE)
#define DFX_MEM_BL31_MDR_SIZE               SZ_K(4)

#define DFX_MEM_RSV3_ADDR                   (DFX_MEM_BL31_MDR_ADDR + DFX_MEM_BL31_MDR_SIZE)
#define DFX_MEM_RSV3_SIZE                   SZ_K(108)

#define DFX_MEM_RSV4_ADDR                   (DFX_MEM_RSV3_ADDR + DFX_MEM_RSV3_SIZE)
#define DFX_MEM_RSV4_SIZE                   SZ_K(128)

#define DFX_MEM_RSV5_ADDR                   (DFX_MEM_RSV4_ADDR + DFX_MEM_RSV4_SIZE)
#define DFX_MEM_RSV5_SIZE                   SZ_K(128)

#define DFX_MEM_RSV6_ADDR                   (DFX_MEM_RSV5_ADDR + DFX_MEM_RSV5_SIZE)
#define DFX_MEM_RSV6_SIZE                   SZ_K(128)

#define DFX_MEM_RSV7_ADDR                   (DFX_MEM_RSV6_ADDR + DFX_MEM_RSV6_SIZE)
#define DFX_MEM_RSV7_SIZE                   SZ_K(516)

#define DFX_MEM_XRSE_LOG_ADDR               (DFX_MEM_RSV7_ADDR + DFX_MEM_RSV7_SIZE)
#define DFX_MEM_XRSE_LOG_SIZE               SZ_K(128)

#define DFX_MEM_RSV8_ADDR                   (DFX_MEM_XRSE_LOG_ADDR + DFX_MEM_XRSE_LOG_SIZE)
#define DFX_MEM_RSV8_SIZE                   SZ_K(128)

#define DFX_MEM_RSV9_ADDR                   (DFX_MEM_RSV8_ADDR + DFX_MEM_RSV8_SIZE)
#define DFX_MEM_RSV9_SIZE                   SZ_K(128)

#define DFX_MEM_LPCTRL_ADDR                 (DFX_MEM_RSV9_ADDR + DFX_MEM_RSV9_SIZE)
#define DFX_MEM_LPCTRL_SIZE                 SZ_K(548)

#define DFX_MEM_NPU_ADDR                    (DFX_MEM_LPCTRL_ADDR + DFX_MEM_LPCTRL_SIZE)
#define DFX_MEM_NPU_SIZE                    SZ_K(128)

#define DFX_MEM_VDSP_ADDR                   (DFX_MEM_NPU_ADDR + DFX_MEM_NPU_SIZE)
#define DFX_MEM_VDSP_SIZE                   SZ_K(128)

#define DFX_MEM_ISP_ADDR                    (DFX_MEM_VDSP_ADDR + DFX_MEM_VDSP_SIZE)
#define DFX_MEM_ISP_SIZE                    SZ_K(256)

#define DFX_MEM_AUDIO_ADDR                  (DFX_MEM_ISP_ADDR + DFX_MEM_ISP_SIZE)
#define DFX_MEM_AUDIO_SIZE                  SZ_K(64)

#define DFX_MEM_ACPU_ADDR                   (DFX_MEM_AUDIO_ADDR + DFX_MEM_AUDIO_SIZE)
#define DFX_MEM_ACPU_SIZE                   SZ_K(3072)

#define BOOT_UP_SIZE                        (0x4)
#define BOOT_UP_ADDR                        (DFX_MEM_KERNEL_PARAM_ADDR - BOOT_UP_SIZE)
#define SUB_REBOOT_REASON_SIZE              (0x4)
#define SUB_REBOOT_REASON_ADDR              (BOOT_UP_ADDR - SUB_REBOOT_REASON_SIZE)
#define REBOOT_REASON_SIZE                  (0x4)
#define REBOOT_REASON_ADDR                  (SUB_REBOOT_REASON_ADDR - REBOOT_REASON_SIZE)

#define DFX_LOGBUF_ADDR                     (DFX_MEM_KERNEL_PARAM_ADDR)
#define DFX_LOGBUF_SIZE                     (0x8)
#define DFX_MEMDUMP_PARAM_ADDR              (DFX_MEM_KERNEL_PARAM_ADDR + DFX_LOGBUF_SIZE)
#define DFX_MEMDUMP_PARAM_SIZE              (0x1F0)

#define DFX_MEM_ACPU_REGS_ADDR              (DFX_MEM_ACPU_ADDR)
#define DFX_MEM_ACPU_REGS_SIZE              SZ_K(16)

#define DFX_MEM_HOOK_STRUCT_ADDR            (DFX_MEM_ACPU_REGS_ADDR + DFX_MEM_ACPU_REGS_SIZE)
#define DFX_MEM_HOOK_STRUCT_SIZE            SZ_K(4)

#define DFX_MEM_HOOK_ADDR                   (DFX_MEM_HOOK_STRUCT_ADDR + DFX_MEM_HOOK_STRUCT_SIZE)
#define DFX_MEM_HOOK_SIZE                   SZ_K(2048)

#define ATF_IRQ_HOOK_ADDR                   (DFX_MEM_HOOK_ADDR + DFX_MEM_HOOK_SIZE)
#define ATF_IRQ_HOOK_SIZE                   SZ_K(16)

#define DFX_MEM_KDUMP_ADDR                  (ATF_IRQ_HOOK_ADDR + ATF_IRQ_HOOK_SIZE)
#define DFX_MEM_KDUMP_SIZE                  SZ_K(4)

#define DFX_MEM_MINIDUMP_ADDR               (DFX_MEM_KDUMP_ADDR + DFX_MEM_KDUMP_SIZE)
#define DFX_MEM_MINIDUMP_SIZE               SZ_K(4)

#endif
