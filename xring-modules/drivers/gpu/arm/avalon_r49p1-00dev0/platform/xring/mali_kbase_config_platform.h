/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#ifndef _MALI_KBASE_CONFIG_PLATFORM_H_
#define _MALI_KBASE_CONFIG_PLATFORM_H_

#define POWER_MANAGEMENT_CALLBACKS (&pm_callbacks)
#define KBASE_PLATFORM_CALLBACKS ((uintptr_t)&platform_funcs)
#define PLATFORM_FUNCS (KBASE_PLATFORM_CALLBACKS)
#define MAX_LOOP                  3
#define DELAY_STEP                5
#define RETRY_MAX_COUNT          20
extern struct kbase_pm_callback_conf pm_callbacks;
extern struct kbase_platform_funcs_conf platform_funcs;
/*
 * @brif Define external module base address
 */
#define SYS_REG_GPU_SUB_BASE_ADDR           0xE9F80000
/* gpu power registers base address*/
#define SYS_REG_GPU_CTRL_BASE_ADDR          0xE9E00000
#define SYS_REG_GPU_CRG_BASE_ADDR           0xE9E01000
#define SYS_REG_GPU_DVFS_BASE_ADDR          0xE9E02000
#define SYS_REG_GPU_LPCTRL_BASE_ADDR        0xE9E03000
/* gpu subchip power on register base address */
#define APB_INNER_REG_PERI_CRG_BASE_ADDR    0xEC001000         //0xEC001000 ~ 0xEC001FFF  4KB
#define LMS_APB_DCDR_REG_LMS_CRG_BASE_ADDR  0xE0000000         //0xE0000000 ~ 0xE0000FFF  4KB
#define XCTRL_REG_XCTRLCPUBASE_ADDR         0xEB005000
/* APB_INNER PCTRL registers base address */
#define APB_INNER_REG_PCTRL_BASE_ADDR       0xEC002000         //0xEC002000 ~ 0xEC003FFF  8KB
/* LPIS ACTRL registers base address */
#define LPIS_ACTRL_REG_BASE_ADDR            0xE1508000         //0xE1508000 ~ 0xE1508FFF  4KB
/*
 * @brif some registers offset value
 */
#define SYS_REG_GPU_SUB_SIZE                0x1000000
#define SYS_REG_POWER_SIZE                  0x1000
#define PERI_STAT_FPGA_GPU_EXIST            0xBC
#define PERI_STAT_FPGA_GPU_EXIST_MASK       0x1
#define APB_INNER_REG_PCTRL_SIZE            0x2000             //8KB
#define LPIS_ACTRL_REG_SIZE                 0x1000             //4KB

/* xr platform remap registers offset */
#define GPU_CTRL_MEMREPAIR_OFFSET           0x008C
#define GPU_PERI_CRG_W1S_OFFSET             0x860
#define GPU_PERI_R1_OFFSET                  0x86C
#define GPU_PERI_R2_OFFSET                  0x8D0
#define GPU_SUBCHIP_CRG_RST0_OFFSET         0x24
#define GPU_SUBCHIP_CRG_CLKGT0_OFFSET       0x10
#define LPCTRL_GPU_TOP_GPC_EN_OFFSET        0x800
#define LPCTRL_GPU_TOP_GPC_REQ_OFFSET       0x0
#define LPCTRL_GPU_TOP_INTR_OFFSET          0x00C
#define LPCTRL_GPU_R1_OFFSET                0x20

#define GPU_SUB_R1_OFFSET                   0x0
#endif /* MALI_KBASE_CONFIG_PLATFORM_H */