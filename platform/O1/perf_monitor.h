// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef PERF_MONITOR_REGIF_H
#define PERF_MONITOR_REGIF_H

#define PERF_MONITOR_P_COMMON                            0x0000
#define PERF_MONITOR_P_COMMON_REG_WIN_EN_SHIFT                      0
#define PERF_MONITOR_P_COMMON_REG_WIN_EN_MASK                       0x00000001
#define PERF_MONITOR_P_COMMON_REG_CFG_STOP_SHIFT                    1
#define PERF_MONITOR_P_COMMON_REG_CFG_STOP_MASK                     0x00000002
#define PERF_MONITOR_P_COMMON_REG_MODE_SEL_SHIFT                    2
#define PERF_MONITOR_P_COMMON_REG_MODE_SEL_MASK                     0x00000004
#define PERF_MONITOR_P_COMMON_REG_CHN_NUM_SHIFT                     3
#define PERF_MONITOR_P_COMMON_REG_CHN_NUM_MASK                      0x000003f8
#define PERF_MONITOR_P_COMMON_REG_WIN_LEN_SHIFT                     10
#define PERF_MONITOR_P_COMMON_REG_WIN_LEN_MASK                      0x03fffc00
#define PERF_MONITOR_P_COMMON_DATA_OVERFLOW_SHIFT                   26
#define PERF_MONITOR_P_COMMON_DATA_OVERFLOW_MASK                    0x04000000
#define PERF_MONITOR_P_MASK1                             0x0004
#define PERF_MONITOR_P_MASK2                             0x0008
#define PERF_MONITOR_P_BASE_ADDR_L                       0x000c
#define PERF_MONITOR_P_BASE_ADDR_H                       0x0010
#define PERF_MONITOR_P_ADDR_SIZE                         0x0014
#define PERF_MONITOR_WIN_LEN                             0x0018
#define PERF_MONITOR_P_MID_MEDI2                         0x001c
#define PERF_MONITOR_P_MID_MEDI2_REG_CHN10_MID_SHIFT                0
#define PERF_MONITOR_P_MID_MEDI2_REG_CHN10_MID_MASK                 0x000000ff
#define PERF_MONITOR_P_MID_MEDI2_REG_CHN11_MID_SHIFT                8
#define PERF_MONITOR_P_MID_MEDI2_REG_CHN11_MID_MASK                 0x0000ff00
#define PERF_MONITOR_P_MID_MEDI2_REG_CHN12_MID_SHIFT                16
#define PERF_MONITOR_P_MID_MEDI2_REG_CHN12_MID_MASK                 0x00ff0000
#define PERF_MONITOR_P_MID_MEDI2_REG_CHN13_MID_SHIFT                24
#define PERF_MONITOR_P_MID_MEDI2_REG_CHN13_MID_MASK                 0xff000000
#define PERF_MONITOR_P_MID_NPU                           0x0020
#define PERF_MONITOR_P_MID_NPU_REG_CHN30_MID_SHIFT                  0
#define PERF_MONITOR_P_MID_NPU_REG_CHN30_MID_MASK                   0x000000ff
#define PERF_MONITOR_P_MID_NPU_REG_CHN31_MID_SHIFT                  8
#define PERF_MONITOR_P_MID_NPU_REG_CHN31_MID_MASK                   0x0000ff00
#define PERF_MONITOR_P_MID_NPU_REG_CHN32_MID_SHIFT                  16
#define PERF_MONITOR_P_MID_NPU_REG_CHN32_MID_MASK                   0x00ff0000
#define PERF_MONITOR_P_MID_NPU_REG_CHN33_MID_SHIFT                  24
#define PERF_MONITOR_P_MID_NPU_REG_CHN33_MID_MASK                   0xff000000
#define PERF_MONITOR_P_MID_CPU                           0x0024
#define PERF_MONITOR_P_MID_CPU_REG_CHN38_MID_SHIFT                  0
#define PERF_MONITOR_P_MID_CPU_REG_CHN38_MID_MASK                   0x000000ff
#define PERF_MONITOR_P_MID_CPU_REG_CHN39_MID_SHIFT                  8
#define PERF_MONITOR_P_MID_CPU_REG_CHN39_MID_MASK                   0x0000ff00
#define PERF_MONITOR_P_MID_CPU_REG_CHN40_MID_SHIFT                  16
#define PERF_MONITOR_P_MID_CPU_REG_CHN40_MID_MASK                   0x00ff0000
#define PERF_MONITOR_P_MID_CPU_REG_CHN41_MID_SHIFT                  24
#define PERF_MONITOR_P_MID_CPU_REG_CHN41_MID_MASK                   0xff000000
#define PERF_MONITOR_P_MID_DMC                           0x0028
#define PERF_MONITOR_P_MID_DMC_REG_CHN42_MID_SHIFT                  0
#define PERF_MONITOR_P_MID_DMC_REG_CHN42_MID_MASK                   0x000000ff
#define PERF_MONITOR_P_MID_DMC_REG_CHN43_MID_SHIFT                  8
#define PERF_MONITOR_P_MID_DMC_REG_CHN43_MID_MASK                   0x0000ff00
#define PERF_MONITOR_P_MID_DMC_REG_CHN44_MID_SHIFT                  16
#define PERF_MONITOR_P_MID_DMC_REG_CHN44_MID_MASK                   0x00ff0000
#define PERF_MONITOR_P_MID_DMC_REG_CHN45_MID_SHIFT                  24
#define PERF_MONITOR_P_MID_DMC_REG_CHN45_MID_MASK                   0xff000000
#define PERF_MONITOR_P_MID_MAINBUS                       0x002c
#define PERF_MONITOR_P_MID_MAINBUS_REG_CHN47_MID_SHIFT              0
#define PERF_MONITOR_P_MID_MAINBUS_REG_CHN47_MID_MASK               0x000000ff
#define PERF_MONITOR_P_MID_MAINBUS_REG_CHN0_MID_SHIFT               8
#define PERF_MONITOR_P_MID_MAINBUS_REG_CHN0_MID_MASK                0x0000ff00
#define PERF_MONITOR_P_MID_MAINBUS_FILTER_MID_EN_SHIFT              16
#define PERF_MONITOR_P_MID_MAINBUS_FILTER_MID_EN_MASK               0x3fff0000
#define PERF_MONITOR_P_GATE_TIMEOUT_CFG                  0x0030
#define PERF_MONITOR_P_PD_TIMEOUT_CFG                    0x0034
#define PERF_MONITOR_P_PD_TIMEOUT_BYPASS                 0x0038
#define PERF_MONITOR_P_PD_TIMEOUT_BYPASS_PD_BYPASS_CFG_SHIFT        0
#define PERF_MONITOR_P_PD_TIMEOUT_BYPASS_PD_BYPASS_CFG_MASK         0x00000001
#define PERF_MONITOR_LOOP_RD_BACK_ADDR_VALID             0x0040
#define PERF_MONITOR_LOOP_RD_BACK_ADDR_VALID_RD_BACK_ADDR_VLD_SHIFT 0
#define PERF_MONITOR_LOOP_RD_BACK_ADDR_VALID_RD_BACK_ADDR_VLD_MASK  0x00000001
#define PERF_MONITOR_LOOP_RD_BACK_ADDR                   0x0060
#define PERF_MONITOR_LOOP_RD_BACK_END_ADDR               0x0064
#define PERF_MONITOR_P_PD_STAT_L                         0x0068
#define PERF_MONITOR_P_PD_STAT_H                         0x006c
#define PERF_MONITOR_IRQ_IRQ_INT_RAW                     0x0080
#define PERF_MONITOR_IRQ_IRQ_INT_RAW_PERF_DATA_INT_RAW_SHIFT        0
#define PERF_MONITOR_IRQ_IRQ_INT_RAW_PERF_DATA_ERROR_RAW_SHIFT      1
#define PERF_MONITOR_IRQ_IRQ_INT_RAW_STAT_PD_FLAG_RAW_SHIFT         2
#define PERF_MONITOR_IRQ_IRQ_INT_RAW_BRESP_PULSE_RAW_SHIFT          3
#define PERF_MONITOR_IRQ_IRQ_INT_FORCE                   0x0084
#define PERF_MONITOR_IRQ_IRQ_INT_FORCE_PERF_DATA_INT_RAW_SHIFT      0
#define PERF_MONITOR_IRQ_IRQ_INT_FORCE_PERF_DATA_ERROR_RAW_SHIFT    1
#define PERF_MONITOR_IRQ_IRQ_INT_FORCE_STAT_PD_FLAG_RAW_SHIFT       2
#define PERF_MONITOR_IRQ_IRQ_INT_FORCE_BRESP_PULSE_RAW_SHIFT        3
#define PERF_MONITOR_IRQ_IRQ_INT_MASK                    0x0088
#define PERF_MONITOR_IRQ_IRQ_INT_MASK_PERF_DATA_INT_MASK_SHIFT      0
#define PERF_MONITOR_IRQ_IRQ_INT_MASK_PERF_DATA_INT_MASK_MASK       0x00000001
#define PERF_MONITOR_IRQ_IRQ_INT_MASK_PERF_DATA_ERROR_MASK_SHIFT    1
#define PERF_MONITOR_IRQ_IRQ_INT_MASK_PERF_DATA_ERROR_MASK_MASK     0x00000002
#define PERF_MONITOR_IRQ_IRQ_INT_MASK_STAT_PD_FLAG_MASK_SHIFT       2
#define PERF_MONITOR_IRQ_IRQ_INT_MASK_STAT_PD_FLAG_MASK_MASK        0x00000004
#define PERF_MONITOR_IRQ_IRQ_INT_MASK_BRESP_PULSE_MASK_SHIFT        3
#define PERF_MONITOR_IRQ_IRQ_INT_MASK_BRESP_PULSE_MASK_MASK         0x00000008
#define PERF_MONITOR_IRQ_IRQ_INT_STATUS                  0x008c
#define PERF_MONITOR_IRQ_IRQ_INT_STATUS_PERF_DATA_INT_STATUS_SHIFT  0
#define PERF_MONITOR_IRQ_IRQ_INT_STATUS_PERF_DATA_INT_STATUS_MASK   0x00000001
#define PERF_MONITOR_IRQ_IRQ_INT_STATUS_PERF_DATA_ERROR_STATUS_SHIFT 1
#define PERF_MONITOR_IRQ_IRQ_INT_STATUS_PERF_DATA_ERROR_STATUS_MASK  0x00000002
#define PERF_MONITOR_IRQ_IRQ_INT_STATUS_STAT_PD_FLAG_STATUS_SHIFT   2
#define PERF_MONITOR_IRQ_IRQ_INT_STATUS_STAT_PD_FLAG_STATUS_MASK    0x00000004
#define PERF_MONITOR_IRQ_IRQ_INT_STATUS_BRESP_PULSE_STATUS_SHIFT    3
#define PERF_MONITOR_IRQ_IRQ_INT_STATUS_BRESP_PULSE_STATUS_MASK     0x00000008
#define PERF_MONITOR_REGFILE_CGBYPASS                    0x0090
#define PERF_MONITOR_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT        0
#define PERF_MONITOR_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK         0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_win_en    :  1;
        unsigned int reg_cfg_stop  :  1;
        unsigned int reg_mode_sel  :  1;
        unsigned int reg_chn_num   :  7;
        unsigned int reg_win_len   : 16;
        unsigned int data_overflow :  1;
        unsigned int reserved_0    :  5;
    } reg;
}perf_monitor_p_common_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_chn_mask1 : 32;
    } reg;
}perf_monitor_p_mask1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_chn_mask2 : 32;
    } reg;
}perf_monitor_p_mask2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_base_addr_l : 32;
    } reg;
}perf_monitor_p_base_addr_l_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_base_addr_h : 32;
    } reg;
}perf_monitor_p_base_addr_h_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_addr_size : 32;
    } reg;
}perf_monitor_p_addr_size_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_win_loop_num : 32;
    } reg;
}perf_monitor_win_len_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_chn10_mid :  8;
        unsigned int reg_chn11_mid :  8;
        unsigned int reg_chn12_mid :  8;
        unsigned int reg_chn13_mid :  8;
    } reg;
}perf_monitor_p_mid_medi2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_chn30_mid :  8;
        unsigned int reg_chn31_mid :  8;
        unsigned int reg_chn32_mid :  8;
        unsigned int reg_chn33_mid :  8;
    } reg;
}perf_monitor_p_mid_npu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_chn38_mid :  8;
        unsigned int reg_chn39_mid :  8;
        unsigned int reg_chn40_mid :  8;
        unsigned int reg_chn41_mid :  8;
    } reg;
}perf_monitor_p_mid_cpu_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_chn42_mid :  8;
        unsigned int reg_chn43_mid :  8;
        unsigned int reg_chn44_mid :  8;
        unsigned int reg_chn45_mid :  8;
    } reg;
}perf_monitor_p_mid_dmc_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reg_chn47_mid :  8;
        unsigned int reg_chn0_mid  :  8;
        unsigned int filter_mid_en : 14;
        unsigned int reserved_0    :  2;
    } reg;
}perf_monitor_p_mid_mainbus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gate_timeout_cfg : 32;
    } reg;
}perf_monitor_p_gate_timeout_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pd_timeout_cfg : 32;
    } reg;
}perf_monitor_p_pd_timeout_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pd_bypass_cfg :  1;
        unsigned int reserved_0    : 31;
    } reg;
}perf_monitor_p_pd_timeout_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rd_back_addr_vld :  1;
        unsigned int reserved_0       : 31;
    } reg;
}perf_monitor_loop_rd_back_addr_valid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rd_back_addr : 32;
    } reg;
}perf_monitor_loop_rd_back_addr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rd_back_end_addr : 32;
    } reg;
}perf_monitor_loop_rd_back_end_addr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pd_stat_l  : 32;
    } reg;
}perf_monitor_p_pd_stat_l_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pd_stat_h  : 32;
    } reg;
}perf_monitor_p_pd_stat_h_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int perf_data_int_raw   :  1;
        unsigned int perf_data_error_raw :  1;
        unsigned int stat_pd_flag_raw    :  1;
        unsigned int bresp_pulse_raw     :  1;
        unsigned int reserved_0          : 28;
    } reg;
}perf_monitor_irq_irq_int_raw_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int perf_data_int_raw   :  1;
        unsigned int perf_data_error_raw :  1;
        unsigned int stat_pd_flag_raw    :  1;
        unsigned int bresp_pulse_raw     :  1;
        unsigned int reserved_0          : 28;
    } reg;
}perf_monitor_irq_irq_int_force_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int perf_data_int_mask   :  1;
        unsigned int perf_data_error_mask :  1;
        unsigned int stat_pd_flag_mask    :  1;
        unsigned int bresp_pulse_mask     :  1;
        unsigned int reserved_0           : 28;
    } reg;
}perf_monitor_irq_irq_int_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int perf_data_int_status   :  1;
        unsigned int perf_data_error_status :  1;
        unsigned int stat_pd_flag_status    :  1;
        unsigned int bresp_pulse_status     :  1;
        unsigned int reserved_0             : 28;
    } reg;
}perf_monitor_irq_irq_int_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}perf_monitor_regfile_cgbypass_t;

#endif
