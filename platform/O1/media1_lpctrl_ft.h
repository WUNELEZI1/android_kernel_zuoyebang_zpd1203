// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef MEDIA1_LPCTRL_FT_REGIF_H
#define MEDIA1_LPCTRL_FT_REGIF_H

#define MEDIA1_LPCTRL_FT_POWER_GROUP0                          0x0000
#define MEDIA1_LPCTRL_FT_POWER_GROUP0_SC_DPU_PART2_MTCMOS_EN_SHIFT               0
#define MEDIA1_LPCTRL_FT_POWER_GROUP0_SC_DPU_PART2_MTCMOS_EN_MASK                0x00000001
#define MEDIA1_LPCTRL_FT_POWER_GROUP0_SC_DPU_PART1_MTCMOS_EN_SHIFT               1
#define MEDIA1_LPCTRL_FT_POWER_GROUP0_SC_DPU_PART1_MTCMOS_EN_MASK                0x00000002
#define MEDIA1_LPCTRL_FT_POWER_GROUP0_SC_VEU_MTCMOS_EN_SHIFT                     2
#define MEDIA1_LPCTRL_FT_POWER_GROUP0_SC_VEU_MTCMOS_EN_MASK                      0x00000004
#define MEDIA1_LPCTRL_FT_POWER_GROUP0_SC_VDEC_MTCMOS_EN_SHIFT                    3
#define MEDIA1_LPCTRL_FT_POWER_GROUP0_SC_VDEC_MTCMOS_EN_MASK                     0x00000008
#define MEDIA1_LPCTRL_FT_POWER_STATUS0                         0x0004
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_DPU_PART2_MTCMOS_ACK_SHIFT                0
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_DPU_PART2_MTCMOS_ACK_MASK                 0x00000001
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_DPU_P1H0_MTCMOS_ACK_SHIFT                 1
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_DPU_P1H0_MTCMOS_ACK_MASK                  0x00000002
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_DPU_P1H1_MTCMOS_ACK_SHIFT                 2
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_DPU_P1H1_MTCMOS_ACK_MASK                  0x00000004
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_VEU_MTCMOS_ACK_SHIFT                      3
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_VEU_MTCMOS_ACK_MASK                       0x00000008
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_VDEC_MTCMOS_ACK_0_SHIFT                   4
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_VDEC_MTCMOS_ACK_0_MASK                    0x00000010
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_VDEC_MTCMOS_ACK_1_SHIFT                   5
#define MEDIA1_LPCTRL_FT_POWER_STATUS0_VDEC_MTCMOS_ACK_1_MASK                    0x00000020
#define MEDIA1_LPCTRL_FT_ISOLATION_GROUP0                      0x0010
#define MEDIA1_LPCTRL_FT_ISOLATION_GROUP0_SC_DPU_PART2_ISO_EN_SHIFT              0
#define MEDIA1_LPCTRL_FT_ISOLATION_GROUP0_SC_DPU_PART2_ISO_EN_MASK               0x00000001
#define MEDIA1_LPCTRL_FT_ISOLATION_GROUP0_SC_DPU_PART1_ISO_EN_SHIFT              1
#define MEDIA1_LPCTRL_FT_ISOLATION_GROUP0_SC_DPU_PART1_ISO_EN_MASK               0x00000002
#define MEDIA1_LPCTRL_FT_ISOLATION_GROUP0_SC_VEU_ISO_EN_SHIFT                    2
#define MEDIA1_LPCTRL_FT_ISOLATION_GROUP0_SC_VEU_ISO_EN_MASK                     0x00000004
#define MEDIA1_LPCTRL_FT_ISOLATION_GROUP0_SC_VDEC_ISO_EN_SHIFT                   3
#define MEDIA1_LPCTRL_FT_ISOLATION_GROUP0_SC_VDEC_ISO_EN_MASK                    0x00000008
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0                     0x0020
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_VEU_MEM_SD_SHIFT                   0
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_VEU_MEM_SD_MASK                    0x00000001
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_VEU_MEM_POFF_SHIFT                 1
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_VEU_MEM_POFF_MASK                  0x00000006
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_DPU_P2_MEM_SD_SHIFT                3
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_DPU_P2_MEM_SD_MASK                 0x00000008
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_DPU_P2_MEM_POFF_SHIFT              4
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_DPU_P2_MEM_POFF_MASK               0x00000030
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_DPU_TOP_MEM_SD_SHIFT               6
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_DPU_TOP_MEM_SD_MASK                0x00000040
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_DPU_TOP_MEM_POFF_SHIFT             7
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_DPU_TOP_MEM_POFF_MASK              0x00000180
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_MEDIA1_TOP_MEM_DS_SHIFT            9
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_MEDIA1_TOP_MEM_DS_MASK             0x00000200
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_MEDIA1_TOP_MEM_POFF_SHIFT          10
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_MEDIA1_TOP_MEM_POFF_MASK           0x00000c00
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_MEDIA1_TOP_MEM_SD_SHIFT            12
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP0_SC_MEDIA1_TOP_MEM_SD_MASK             0x00001000
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1                     0x0024
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P10_GRP1_MEM_SD_SHIFT          3
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P10_GRP1_MEM_SD_MASK           0x00000008
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P10_GRP1_MEM_POFF_SHIFT        4
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P10_GRP1_MEM_POFF_MASK         0x00000030
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P10_GRP0_MEM_SD_SHIFT          6
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P10_GRP0_MEM_SD_MASK           0x00000040
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P10_GRP0_MEM_POFF_SHIFT        7
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P10_GRP0_MEM_POFF_MASK         0x00000180
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P11_GRP1_MEM_SD_SHIFT          9
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P11_GRP1_MEM_SD_MASK           0x00000200
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P11_GRP1_MEM_POFF_SHIFT        10
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P11_GRP1_MEM_POFF_MASK         0x00000c00
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P11_GRP0_MEM_SD_SHIFT          12
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P11_GRP0_MEM_SD_MASK           0x00001000
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P11_GRP0_MEM_POFF_SHIFT        13
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP1_SC_DPU_P11_GRP0_MEM_POFF_MASK         0x00006000
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP2                     0x0028
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP2_SC_VDEC_CORE_MEM_SD_SHIFT             0
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP2_SC_VDEC_CORE_MEM_SD_MASK              0x00000001
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP2_SC_VDEC_CORE_MEM_POFF_SHIFT           1
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP2_SC_VDEC_CORE_MEM_POFF_MASK            0x00000006
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP2_SC_VDEC_CORE_MEM_DS_SHIFT             3
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP2_SC_VDEC_CORE_MEM_DS_MASK              0x00000008
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP2_SC_VDEC_CORE_MEM_DS_SEL_SHIFT         4
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_GROUP2_SC_VDEC_CORE_MEM_DS_SEL_MASK          0x00000010
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0                         0x0040
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_VDEC_CORE_MEM_SD_ACK_SHIFT                0
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_VDEC_CORE_MEM_SD_ACK_MASK                 0x00000001
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_VEU_MEM_SD_ACK_SHIFT                      1
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_VEU_MEM_SD_ACK_MASK                       0x00000002
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_P2_MEM_SD_ACK_SHIFT                   2
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_P2_MEM_SD_ACK_MASK                    0x00000004
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_P11_GRP1_MEM_SD_ACK_SHIFT             3
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_P11_GRP1_MEM_SD_ACK_MASK              0x00000008
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_P11_GRP0_MEM_SD_ACK_SHIFT             4
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_P11_GRP0_MEM_SD_ACK_MASK              0x00000010
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_P10_GRP1_MEM_SD_ACK_SHIFT             6
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_P10_GRP1_MEM_SD_ACK_MASK              0x00000040
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_P10_GRP0_MEM_SD_ACK_SHIFT             7
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_P10_GRP0_MEM_SD_ACK_MASK              0x00000080
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_TOP_MEM_SD_ACK_SHIFT                  8
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_DPU_TOP_MEM_SD_ACK_MASK                   0x00000100
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_MEDIA1_TOP_MEM_SD_ACK_SHIFT               9
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_MEDIA1_TOP_MEM_SD_ACK_MASK                0x00000200
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_VDEC_CORE_MEM_DS_ACK_SHIFT                10
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_VDEC_CORE_MEM_DS_ACK_MASK                 0x00003c00
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_MEDIA1_TOP_MEM_DS_ACK_SHIFT               14
#define MEDIA1_LPCTRL_FT_MEM_LPSTATUS0_MEDIA1_TOP_MEM_DS_ACK_MASK                0x00004000
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0                 0x0044
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P10_GRP1_MEM_SD_REQ_SHIFT  0
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P10_GRP1_MEM_SD_REQ_MASK   0x00000001
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P10_GRP1_MEM_POFF_REQ_SHIFT 1
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P10_GRP1_MEM_POFF_REQ_MASK  0x00000006
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P10_GRP0_MEM_SD_REQ_SHIFT  3
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P10_GRP0_MEM_SD_REQ_MASK   0x00000008
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P10_GRP0_MEM_POFF_REQ_SHIFT 4
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P10_GRP0_MEM_POFF_REQ_MASK  0x00000030
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P11_GRP1_MEM_SD_REQ_SHIFT  6
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P11_GRP1_MEM_SD_REQ_MASK   0x00000040
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P11_GRP1_MEM_POFF_REQ_SHIFT 7
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P11_GRP1_MEM_POFF_REQ_MASK  0x00000180
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P11_GRP0_MEM_SD_REQ_SHIFT  9
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P11_GRP0_MEM_SD_REQ_MASK   0x00000200
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P11_GRP0_MEM_POFF_REQ_SHIFT 10
#define MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0_SC_DPU_P11_GRP0_MEM_POFF_REQ_MASK  0x00000c00
#define MEDIA1_LPCTRL_FT_REGFILE_CGBYPASS                      0x0048
#define MEDIA1_LPCTRL_FT_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                 0
#define MEDIA1_LPCTRL_FT_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                  0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_dpu_part2_mtcmos_en :  1;
        unsigned int sc_dpu_part1_mtcmos_en :  1;
        unsigned int sc_veu_mtcmos_en       :  1;
        unsigned int sc_vdec_mtcmos_en      :  1;
        unsigned int reserved_0             : 12;
        unsigned int _bm_                   : 16;
    } reg;
}media1_lpctrl_ft_power_group0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dpu_part2_mtcmos_ack :  1;
        unsigned int dpu_p1h0_mtcmos_ack  :  1;
        unsigned int dpu_p1h1_mtcmos_ack  :  1;
        unsigned int veu_mtcmos_ack       :  1;
        unsigned int vdec_mtcmos_ack_0    :  1;
        unsigned int vdec_mtcmos_ack_1    :  1;
        unsigned int reserved_0           : 26;
    } reg;
}media1_lpctrl_ft_power_status0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_dpu_part2_iso_en :  1;
        unsigned int sc_dpu_part1_iso_en :  1;
        unsigned int sc_veu_iso_en       :  1;
        unsigned int sc_vdec_iso_en      :  1;
        unsigned int reserved_0          : 12;
        unsigned int _bm_                : 16;
    } reg;
}media1_lpctrl_ft_isolation_group0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_veu_mem_sd          :  1;
        unsigned int sc_veu_mem_poff        :  2;
        unsigned int sc_dpu_p2_mem_sd       :  1;
        unsigned int sc_dpu_p2_mem_poff     :  2;
        unsigned int sc_dpu_top_mem_sd      :  1;
        unsigned int sc_dpu_top_mem_poff    :  2;
        unsigned int sc_media1_top_mem_ds   :  1;
        unsigned int sc_media1_top_mem_poff :  2;
        unsigned int sc_media1_top_mem_sd   :  1;
        unsigned int reserved_0             :  3;
        unsigned int _bm_                   : 16;
    } reg;
}media1_lpctrl_ft_mem_lpctrl_group0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0               :  3;
        unsigned int sc_dpu_p10_grp1_mem_sd   :  1;
        unsigned int sc_dpu_p10_grp1_mem_poff :  2;
        unsigned int sc_dpu_p10_grp0_mem_sd   :  1;
        unsigned int sc_dpu_p10_grp0_mem_poff :  2;
        unsigned int sc_dpu_p11_grp1_mem_sd   :  1;
        unsigned int sc_dpu_p11_grp1_mem_poff :  2;
        unsigned int sc_dpu_p11_grp0_mem_sd   :  1;
        unsigned int sc_dpu_p11_grp0_mem_poff :  2;
        unsigned int reserved_1               :  1;
        unsigned int _bm_                     : 16;
    } reg;
}media1_lpctrl_ft_mem_lpctrl_group1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_vdec_core_mem_sd     :  1;
        unsigned int sc_vdec_core_mem_poff   :  2;
        unsigned int sc_vdec_core_mem_ds     :  1;
        unsigned int sc_vdec_core_mem_ds_sel :  1;
        unsigned int reserved_0              : 11;
        unsigned int _bm_                    : 16;
    } reg;
}media1_lpctrl_ft_mem_lpctrl_group2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vdec_core_mem_sd_ack    :  1;
        unsigned int veu_mem_sd_ack          :  1;
        unsigned int dpu_p2_mem_sd_ack       :  1;
        unsigned int dpu_p11_grp1_mem_sd_ack :  1;
        unsigned int dpu_p11_grp0_mem_sd_ack :  1;
        unsigned int reserved_0              :  1;
        unsigned int dpu_p10_grp1_mem_sd_ack :  1;
        unsigned int dpu_p10_grp0_mem_sd_ack :  1;
        unsigned int dpu_top_mem_sd_ack      :  1;
        unsigned int media1_top_mem_sd_ack   :  1;
        unsigned int vdec_core_mem_ds_ack    :  4;
        unsigned int media1_top_mem_ds_ack   :  1;
        unsigned int reserved_1              : 17;
    } reg;
}media1_lpctrl_ft_mem_lpstatus0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_dpu_p10_grp1_mem_sd_req   :  1;
        unsigned int sc_dpu_p10_grp1_mem_poff_req :  2;
        unsigned int sc_dpu_p10_grp0_mem_sd_req   :  1;
        unsigned int sc_dpu_p10_grp0_mem_poff_req :  2;
        unsigned int sc_dpu_p11_grp1_mem_sd_req   :  1;
        unsigned int sc_dpu_p11_grp1_mem_poff_req :  2;
        unsigned int sc_dpu_p11_grp0_mem_sd_req   :  1;
        unsigned int sc_dpu_p11_grp0_mem_poff_req :  2;
        unsigned int reserved_0                   :  4;
        unsigned int _bm_                         : 16;
    } reg;
}media1_lpctrl_ft_mem_lpctrl_req_group0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}media1_lpctrl_ft_regfile_cgbypass_t;

#endif
