// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef IP_REGULATOR_DEFINE_H
#define IP_REGULATOR_DEFINE_H

#define VDD_GPU_ID                      0
#define GPU_SUBSYS_ID                   1
#define NPU_SUBCHIP_ID                  2
#define NPU_OCM_ID                      3
#define VDD_NPU_ID                      4
#define NPU_SUBSYS_ID                   5
#define NPU_VDSP_ID                     6
#define VDD_MEDIA_ID                    7
#define MEDIA1_SUBSYS_ID                8
#define DPU1_ID                         9
#define DPU2_ID                         10
#define VEU_TOP_SUBSYS_ID               11
#define VDEC_CORE_ID                    12
#define MEDIA2_SUBSYS_ID                13
#define ISP_MCU_TOP_ID                  14
#define VENC_SUBSYS_ID                  15
#define ASP_ID                          16
#define VDEC_SUBSYS_ID                  17
#define DPU0_ID                         18
#define ISP_SUBSYS_ID                   19
#define ISP_TOP_FE_CORE0_ID             20
#define ISP_TOP_FE_CORE1_ID             21
#define ISP_TOP_FE_CORE2_ID             22
#define ISP_TOP_BE_ID                   23
#define ISP_TOP_PE_ID                   24
#define RELULATOR_MAX_COUNT             25

#define GPC_ENABLE                      0x00010001
#define GPC_DISABLE                     0x00010000
#define GPC_TIMEOUT                     500
#define GPC_REG_LENGTH                  4

#define GPU_AUTOFS_ENABLE               0
#define GPU_AUTOFS_ENABLE_MASK          16
#define ADB_PWR_SLV3_CLK                21
#define ADB_PWR_SLV2_CLK                20
#define ADB_PWR_SLV1_CLK                19
#define ADB_PWR_SLV0_CLK                18
#define GPU_ADB_PWR_SLV_STATE_ON        0x249
#define GPU_ADB_PWR_SLV_STATE_OFF       0xFFF

#define DPU1_GPC_MEMORY_ON              0x0FFF0000
#define DPU1_GPC_MEMORY_OFF             0x0FFF0EBA
#define DPU1_GPC_P1H0                   5
#define DPU1_GPC_P1H2                   7
#define DPU_GLB_GRP_DS                  0x28

#define VDEC_GPC_M1_BISR                0x1

#define MEDIA_POWER_OFF                 0
#define MEDIA_POWER_ON                  1
#define MEDIA_POWER_GET_STATE           2
#define MEDIA_POWER_VOLT_SET            3
#define MEDIA_POWER_VOLT_GET            4
#define MEDIA_POWER_DVS_SET             5
#define GPC_POWER_ON_SUCESS             0x4
#define GPC_POWER_OFF_SUCESS            0x8

#define GPU_POWER_OFF                   0
#define GPU_POWER_ON                    1

#define RG_BIT(nr)                      (1UL << (nr))
#define CLR(nr)                         (~(1UL << (nr)))
#define ALL_MASK                        0xFFFFFFFF
#define BMRW_MASK(nr)           ((unsigned int)(nr) << 16)

#endif
