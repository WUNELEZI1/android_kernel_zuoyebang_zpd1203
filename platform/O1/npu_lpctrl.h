// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef NPU_LPCTRL_REGIF_H
#define NPU_LPCTRL_REGIF_H

#define NPU_LPCTRL_POWER_GROUP0                0x0000
#define NPU_LPCTRL_POWER_GROUP0_SC_NPU_MTCMOS_EN_SHIFT        0
#define NPU_LPCTRL_POWER_GROUP0_SC_NPU_MTCMOS_EN_MASK         0x00000001
#define NPU_LPCTRL_POWER_STATUS0               0x0004
#define NPU_LPCTRL_POWER_STATUS0_NPU_MTCMOS_ACK0_SHIFT        0
#define NPU_LPCTRL_POWER_STATUS0_NPU_MTCMOS_ACK0_MASK         0x00000001
#define NPU_LPCTRL_POWER_STATUS0_NPU_MTCMOS_ACK1_SHIFT        1
#define NPU_LPCTRL_POWER_STATUS0_NPU_MTCMOS_ACK1_MASK         0x00000002
#define NPU_LPCTRL_ISOLATION_GROUP0            0x0010
#define NPU_LPCTRL_ISOLATION_GROUP0_SC_NPU_ISO_EN_SHIFT       0
#define NPU_LPCTRL_ISOLATION_GROUP0_SC_NPU_ISO_EN_MASK        0x00000001
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0           0x0028
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM3_SD_SHIFT     0
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM3_SD_MASK      0x00000001
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM3_DS_SHIFT     1
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM3_DS_MASK      0x00000002
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM3_POFF_SHIFT   2
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM3_POFF_MASK    0x0000000c
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM2_SD_SHIFT     4
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM2_SD_MASK      0x00000010
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM2_DS_SHIFT     5
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM2_DS_MASK      0x00000020
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM2_POFF_SHIFT   6
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM2_POFF_MASK    0x000000c0
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM1_SD_SHIFT     8
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM1_SD_MASK      0x00000100
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM1_DS_SHIFT     9
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM1_DS_MASK      0x00000200
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM1_POFF_SHIFT   10
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM1_POFF_MASK    0x00000c00
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM0_SD_SHIFT     12
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM0_SD_MASK      0x00001000
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM0_DS_SHIFT     13
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM0_DS_MASK      0x00002000
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM0_POFF_SHIFT   14
#define NPU_LPCTRL_MEM_LPCTRL_GROUP0_SC_NPU_OCM0_POFF_MASK    0x0000c000
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1           0x0030
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_TOP_MEM_SD_SHIFT  0
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_TOP_MEM_SD_MASK   0x00000001
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_TOP_MEM_DS_SHIFT  1
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_TOP_MEM_DS_MASK   0x00000002
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_TOP_MEM_POFF_SHIFT 2
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_TOP_MEM_POFF_MASK  0x0000000c
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_DMA_TBU_SD_SHIFT  4
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_DMA_TBU_SD_MASK   0x00000010
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_DMA_TBU_DS_SHIFT  5
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_DMA_TBU_DS_MASK   0x00000020
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_DMA_TBU_POFF_SHIFT 6
#define NPU_LPCTRL_MEM_LPCTRL_GROUP1_SC_NPU_DMA_TBU_POFF_MASK  0x000000c0
#define NPU_LPCTRL_MEM_LPSTATUS0               0x0040
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM3_SD_ACK_SHIFT        0
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM3_SD_ACK_MASK         0x00000001
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM2_SD_ACK_SHIFT        1
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM2_SD_ACK_MASK         0x00000002
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM1_SD_ACK_SHIFT        2
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM1_SD_ACK_MASK         0x00000004
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM0_SD_ACK_SHIFT        3
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM0_SD_ACK_MASK         0x00000008
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_TOP_MEM_SD_ACK_SHIFT     4
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_TOP_MEM_SD_ACK_MASK      0x00000010
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_DMA_TBU_SD_ACK_SHIFT     5
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_DMA_TBU_SD_ACK_MASK      0x00000020
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM3_DS_ACK_SHIFT        6
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM3_DS_ACK_MASK         0x00000040
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM2_DS_ACK_SHIFT        7
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM2_DS_ACK_MASK         0x00000080
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM1_DS_ACK_SHIFT        8
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM1_DS_ACK_MASK         0x00000100
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM0_DS_ACK_SHIFT        9
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_OCM0_DS_ACK_MASK         0x00000200
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_TOP_MEM_DS_ACK_SHIFT     10
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_TOP_MEM_DS_ACK_MASK      0x00000400
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_DMA_TBU_DS_ACK_SHIFT     11
#define NPU_LPCTRL_MEM_LPSTATUS0_NPU_DMA_TBU_DS_ACK_MASK      0x00000800
#define NPU_LPCTRL_CGBYPASS                    0x0044
#define NPU_LPCTRL_CGBYPASS_CGBYPASS_SHIFT                    0
#define NPU_LPCTRL_CGBYPASS_CGBYPASS_MASK                     0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_npu_mtcmos_en :  1;
        unsigned int reserved_0       : 15;
        unsigned int _bm_             : 16;
    } reg;
}npu_lpctrl_power_group0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_mtcmos_ack0 :  1;
        unsigned int npu_mtcmos_ack1 :  1;
        unsigned int reserved_0      : 30;
    } reg;
}npu_lpctrl_power_status0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_npu_iso_en :  1;
        unsigned int reserved_0    : 15;
        unsigned int _bm_          : 16;
    } reg;
}npu_lpctrl_isolation_group0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_npu_ocm3_sd   :  1;
        unsigned int sc_npu_ocm3_ds   :  1;
        unsigned int sc_npu_ocm3_poff :  2;
        unsigned int sc_npu_ocm2_sd   :  1;
        unsigned int sc_npu_ocm2_ds   :  1;
        unsigned int sc_npu_ocm2_poff :  2;
        unsigned int sc_npu_ocm1_sd   :  1;
        unsigned int sc_npu_ocm1_ds   :  1;
        unsigned int sc_npu_ocm1_poff :  2;
        unsigned int sc_npu_ocm0_sd   :  1;
        unsigned int sc_npu_ocm0_ds   :  1;
        unsigned int sc_npu_ocm0_poff :  2;
        unsigned int _bm_             : 16;
    } reg;
}npu_lpctrl_mem_lpctrl_group0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_npu_top_mem_sd   :  1;
        unsigned int sc_npu_top_mem_ds   :  1;
        unsigned int sc_npu_top_mem_poff :  2;
        unsigned int sc_npu_dma_tbu_sd   :  1;
        unsigned int sc_npu_dma_tbu_ds   :  1;
        unsigned int sc_npu_dma_tbu_poff :  2;
        unsigned int reserved_0          :  8;
        unsigned int _bm_                : 16;
    } reg;
}npu_lpctrl_mem_lpctrl_group1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int npu_ocm3_sd_ack    :  1;
        unsigned int npu_ocm2_sd_ack    :  1;
        unsigned int npu_ocm1_sd_ack    :  1;
        unsigned int npu_ocm0_sd_ack    :  1;
        unsigned int npu_top_mem_sd_ack :  1;
        unsigned int npu_dma_tbu_sd_ack :  1;
        unsigned int npu_ocm3_ds_ack    :  1;
        unsigned int npu_ocm2_ds_ack    :  1;
        unsigned int npu_ocm1_ds_ack    :  1;
        unsigned int npu_ocm0_ds_ack    :  1;
        unsigned int npu_top_mem_ds_ack :  1;
        unsigned int npu_dma_tbu_ds_ack :  1;
        unsigned int reserved_0         : 20;
    } reg;
}npu_lpctrl_mem_lpstatus0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cgbypass   :  1;
        unsigned int reserved_0 : 31;
    } reg;
}npu_lpctrl_cgbypass_t;

#endif
