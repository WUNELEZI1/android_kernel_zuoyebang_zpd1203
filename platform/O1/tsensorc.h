// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef TSENSORC_REGIF_H
#define TSENSORC_REGIF_H

#define TSENSORC_DTS_SENSOR1_MSG                       0x0000
#define TSENSORC_DTS_SENSOR2_MSG                       0x0004
#define TSENSORC_DTS_SENSOR3_MSG                       0x0008
#define TSENSORC_DTS_SENSOR4_MSG                       0x000c
#define TSENSORC_DTS_SENSOR5_MSG                       0x0010
#define TSENSORC_DTS_SENSOR6_MSG                       0x0014
#define TSENSORC_DTS_SENSOR7_MSG                       0x0018
#define TSENSORC_DTS_SENSOR8_MSG                       0x001c
#define TSENSORC_DTS_SENSOR9_MSG                       0x0020
#define TSENSORC_DTS_SENSOR10_MSG                      0x0024
#define TSENSORC_DTS_SENSOR11_MSG                      0x0028
#define TSENSORC_DTS_SENSOR12_MSG                      0x002c
#define TSENSORC_DTS_SENSOR13_MSG                      0x0030
#define TSENSORC_DTS_SENSOR14_MSG                      0x0034
#define TSENSORC_DTS_SENSOR15_MSG                      0x0038
#define TSENSORC_DTS_SENSOR16_MSG                      0x003c
#define TSENSORC_DTS_SENSOR17_MSG                      0x0040
#define TSENSORC_DTS_SENSOR18_MSG                      0x0044
#define TSENSORC_DTS_SENSOR19_MSG                      0x0048
#define TSENSORC_DTS_SENSOR20_MSG                      0x004c
#define TSENSORC_DTS_SENSOR21_MSG                      0x0050
#define TSENSORC_DTS_SENSOR22_MSG                      0x0054
#define TSENSORC_SENSOR_FAULTN                         0x0058
#define TSENSORC_SENSOR_FAULTN_SENSOR_FAULTN_SHIFT                  0
#define TSENSORC_SENSOR_FAULTN_SENSOR_FAULTN_MASK                   0x003fffff
#define TSENSORC_MONITOR_MSG0                          0x0200
#define TSENSORC_MONITOR_MSG0_MAX_TEMP_MSG_SHIFT                    0
#define TSENSORC_MONITOR_MSG0_MAX_TEMP_MSG_MASK                     0x00007fff
#define TSENSORC_MONITOR_MSG0_MIN_TEMP_MSG_SHIFT                    15
#define TSENSORC_MONITOR_MSG0_MIN_TEMP_MSG_MASK                     0x3fff8000
#define TSENSORC_INTR_MSG                              0x0204
#define TSENSORC_INTR_MSG_INTR_MSG_SHIFT                            0
#define TSENSORC_INTR_MSG_INTR_MSG_MASK                             0x7fffffff
#define TSENSORC_CFG                                   0x0800
#define TSENSORC_CFG_MODE_SHIFT                                     0
#define TSENSORC_CFG_MODE_MASK                                      0x00000001
#define TSENSORC_CFG_SOFT_EN_SHIFT                                  1
#define TSENSORC_CFG_SOFT_EN_MASK                                   0x00000002
#define TSENSORC_MONITOR_PERIOR0                       0x0804
#define TSENSORC_MONITOR_PERIOR0_PERIOD_CNT0_SHIFT                  0
#define TSENSORC_MONITOR_PERIOR0_PERIOD_CNT0_MASK                   0x0000ffff
#define TSENSORC_MONITOR_PERIOR1                       0x0808
#define TSENSORC_MONITOR_PERIOR1_PERIOD_CNT1_SHIFT                  0
#define TSENSORC_MONITOR_PERIOR1_PERIOD_CNT1_MASK                   0x0000ffff
#define TSENSORC_MONITOR_PERIOR2                       0x080c
#define TSENSORC_MONITOR_PERIOR2_PERIOD_CNT2_SHIFT                  0
#define TSENSORC_MONITOR_PERIOR2_PERIOD_CNT2_MASK                   0x0000ffff
#define TSENSORC_MONITOR_PERIOR3                       0x0810
#define TSENSORC_MONITOR_PERIOR3_PERIOD_CNT3_SHIFT                  0
#define TSENSORC_MONITOR_PERIOR3_PERIOD_CNT3_MASK                   0x0000ffff
#define TSENSORC_MONITOR_PERIOR4                       0x0814
#define TSENSORC_MONITOR_PERIOR4_PERIOD_CNT4_SHIFT                  0
#define TSENSORC_MONITOR_PERIOR4_PERIOD_CNT4_MASK                   0x0000ffff
#define TSENSORC_MONITOR_PERIOR5                       0x0818
#define TSENSORC_MONITOR_PERIOR5_PERIOD_CNT5_SHIFT                  0
#define TSENSORC_MONITOR_PERIOR5_PERIOD_CNT5_MASK                   0x0000ffff
#define TSENSORC_MONITOR_PERIOR6                       0x081c
#define TSENSORC_MONITOR_PERIOR6_PERIOD_CNT6_SHIFT                  0
#define TSENSORC_MONITOR_PERIOR6_PERIOD_CNT6_MASK                   0x0000ffff
#define TSENSORC_MONITOR_PULSE                         0x0820
#define TSENSORC_MONITOR_PULSE_PERIOD_CNT_PULSE_EN_SHIFT            0
#define TSENSORC_SENSOR_MASK                           0x0824
#define TSENSORC_SENSOR_MASK_SENSOR_MASK_SHIFT                      0
#define TSENSORC_SENSOR_MASK_SENSOR_MASK_MASK                       0x003fffff
#define TSENSORC_SENSOR_MASK_PULSE                     0x0828
#define TSENSORC_SENSOR_MASK_PULSE_SENSOR_MASK_PULSE_EN_SHIFT       0
#define TSENSORC_CTRL_SIG_MASK                         0x082c
#define TSENSORC_CTRL_SIG_MASK_ALERT_MASK_SHIFT                     0
#define TSENSORC_CTRL_SIG_MASK_ALERT_MASK_MASK                      0x000000ff
#define TSENSORC_CTRL_SIG_MASK_TSENSOR_RST_REQ_MASK_SHIFT           8
#define TSENSORC_CTRL_SIG_MASK_TSENSOR_RST_REQ_MASK_MASK            0x00000300
#define TSENSORC_CTRL_SIG_MASK_SCTRL_TSENSOR_EN_BYPASS_SHIFT        10
#define TSENSORC_CTRL_SIG_MASK_SCTRL_TSENSOR_EN_BYPASS_MASK         0x00000400
#define TSENSORC_ALARM_TEMP0                           0x0830
#define TSENSORC_ALARM_TEMP0_ALARM_DATA0_SHIFT                      0
#define TSENSORC_ALARM_TEMP0_ALARM_DATA0_MASK                       0x000001ff
#define TSENSORC_ALARM_TEMP0_ALARM_DATA1_SHIFT                      9
#define TSENSORC_ALARM_TEMP0_ALARM_DATA1_MASK                       0x0003fe00
#define TSENSORC_ALARM_TEMP0_ALARM_DATA2_SHIFT                      18
#define TSENSORC_ALARM_TEMP0_ALARM_DATA2_MASK                       0x07fc0000
#define TSENSORC_ALARM_TEMP1                           0x0834
#define TSENSORC_ALARM_TEMP1_ALARM_DATA3_SHIFT                      0
#define TSENSORC_ALARM_TEMP1_ALARM_DATA3_MASK                       0x000001ff
#define TSENSORC_ALARM_TEMP1_ALARM_DATA4_SHIFT                      9
#define TSENSORC_ALARM_TEMP1_ALARM_DATA4_MASK                       0x0003fe00
#define TSENSORC_ALARM_TEMP1_ALARM_DATA5_SHIFT                      18
#define TSENSORC_ALARM_TEMP1_ALARM_DATA5_MASK                       0x07fc0000
#define TSENSORC_ALARM_TEMP2                           0x0838
#define TSENSORC_ALARM_TEMP2_ALARM_DATA6_SHIFT                      0
#define TSENSORC_ALARM_TEMP2_ALARM_DATA6_MASK                       0x000001ff
#define TSENSORC_ALARM_TEMP2_ALARM_DATA7_SHIFT                      9
#define TSENSORC_ALARM_TEMP2_ALARM_DATA7_MASK                       0x0003fe00
#define TSENSORC_HSYT_VALUE                            0x083c
#define TSENSORC_HSYT_VALUE_HSYT_VALUE0_SHIFT                       0
#define TSENSORC_HSYT_VALUE_HSYT_VALUE0_MASK                        0x0000000f
#define TSENSORC_HSYT_VALUE_HSYT_VALUE1_SHIFT                       4
#define TSENSORC_HSYT_VALUE_HSYT_VALUE1_MASK                        0x000000f0
#define TSENSORC_HSYT_VALUE_HSYT_VALUE2_SHIFT                       8
#define TSENSORC_HSYT_VALUE_HSYT_VALUE2_MASK                        0x00000f00
#define TSENSORC_HSYT_VALUE_HSYT_VALUE3_SHIFT                       12
#define TSENSORC_HSYT_VALUE_HSYT_VALUE3_MASK                        0x0000f000
#define TSENSORC_HSYT_VALUE_HSYT_VALUE4_SHIFT                       16
#define TSENSORC_HSYT_VALUE_HSYT_VALUE4_MASK                        0x000f0000
#define TSENSORC_HSYT_VALUE_HSYT_VALUE5_SHIFT                       20
#define TSENSORC_HSYT_VALUE_HSYT_VALUE5_MASK                        0x00f00000
#define TSENSORC_HSYT_VALUE_HSYT_VALUE6_SHIFT                       24
#define TSENSORC_HSYT_VALUE_HSYT_VALUE6_MASK                        0x0f000000
#define TSENSORC_GT_AUTO_BYP                           0x0840
#define TSENSORC_GT_AUTO_BYP_GT_AUTO_CLK_HUB_BYPASS_SHIFT           0
#define TSENSORC_GT_AUTO_BYP_GT_AUTO_CLK_HUB_BYPASS_MASK            0x00000001
#define TSENSORC_ST_CLK_HUB                            0x0844
#define TSENSORC_ST_CLK_HUB_ST_CLK_HUB_SHIFT                        0
#define TSENSORC_ST_CLK_HUB_ST_CLK_HUB_MASK                         0x00000001
#define TSENSORC_TIMELINE_VAL_CLR                      0x0848
#define TSENSORC_TIMELINE_VAL_CLR_TIMELINE_MAXVAL_CLR_SHIFT         0
#define TSENSORC_TIMELINE_VAL_CLR_TIMELINE_MINVAL_CLR_SHIFT         1
#define TSENSORC_TIMELINE_VAL                          0x084c
#define TSENSORC_TIMELINE_VAL_TIMELINE_MAXVAL_SHIFT                 0
#define TSENSORC_TIMELINE_VAL_TIMELINE_MAXVAL_MASK                  0x000001ff
#define TSENSORC_TIMELINE_VAL_TIMELINE_MINVAL_SHIFT                 9
#define TSENSORC_TIMELINE_VAL_TIMELINE_MINVAL_MASK                  0x0003fe00
#define TSENSORC_SOE_Y                                 0x0900
#define TSENSORC_SOE_Y_SENSOR_SOE_Y_SHIFT                           0
#define TSENSORC_SOE_Y_SENSOR_SOE_Y_MASK                            0x0001ffff
#define TSENSORC_SOE_K0                                0x0904
#define TSENSORC_SOE_K0_SENSOR1_SOE_K_SHIFT                         0
#define TSENSORC_SOE_K0_SENSOR1_SOE_K_MASK                          0x0000ffff
#define TSENSORC_SOE_K0_SENSOR2_SOE_K_SHIFT                         16
#define TSENSORC_SOE_K0_SENSOR2_SOE_K_MASK                          0xffff0000
#define TSENSORC_SOE_K1                                0x0908
#define TSENSORC_SOE_K1_SENSOR3_SOE_K_SHIFT                         0
#define TSENSORC_SOE_K1_SENSOR3_SOE_K_MASK                          0x0000ffff
#define TSENSORC_SOE_K1_SENSOR4_SOE_K_SHIFT                         16
#define TSENSORC_SOE_K1_SENSOR4_SOE_K_MASK                          0xffff0000
#define TSENSORC_SOE_K2                                0x090c
#define TSENSORC_SOE_K2_SENSOR5_SOE_K_SHIFT                         0
#define TSENSORC_SOE_K2_SENSOR5_SOE_K_MASK                          0x0000ffff
#define TSENSORC_SOE_K2_SENSOR6_SOE_K_SHIFT                         16
#define TSENSORC_SOE_K2_SENSOR6_SOE_K_MASK                          0xffff0000
#define TSENSORC_SOE_K3                                0x0910
#define TSENSORC_SOE_K3_SENSOR7_SOE_K_SHIFT                         0
#define TSENSORC_SOE_K3_SENSOR7_SOE_K_MASK                          0x0000ffff
#define TSENSORC_SOE_K3_SENSOR8_SOE_K_SHIFT                         16
#define TSENSORC_SOE_K3_SENSOR8_SOE_K_MASK                          0xffff0000
#define TSENSORC_SOE_K4                                0x0914
#define TSENSORC_SOE_K4_SENSOR9_SOE_K_SHIFT                         0
#define TSENSORC_SOE_K4_SENSOR9_SOE_K_MASK                          0x0000ffff
#define TSENSORC_SOE_K4_SENSOR10_SOE_K_SHIFT                        16
#define TSENSORC_SOE_K4_SENSOR10_SOE_K_MASK                         0xffff0000
#define TSENSORC_SOE_K5                                0x0918
#define TSENSORC_SOE_K5_SENSOR11_SOE_K_SHIFT                        0
#define TSENSORC_SOE_K5_SENSOR11_SOE_K_MASK                         0x0000ffff
#define TSENSORC_SOE_K5_SENSOR12_SOE_K_SHIFT                        16
#define TSENSORC_SOE_K5_SENSOR12_SOE_K_MASK                         0xffff0000
#define TSENSORC_SOE_K6                                0x091c
#define TSENSORC_SOE_K6_SENSOR13_SOE_K_SHIFT                        0
#define TSENSORC_SOE_K6_SENSOR13_SOE_K_MASK                         0x0000ffff
#define TSENSORC_SOE_K6_SENSOR14_SOE_K_SHIFT                        16
#define TSENSORC_SOE_K6_SENSOR14_SOE_K_MASK                         0xffff0000
#define TSENSORC_SOE_K7                                0x0920
#define TSENSORC_SOE_K7_SENSOR15_SOE_K_SHIFT                        0
#define TSENSORC_SOE_K7_SENSOR15_SOE_K_MASK                         0x0000ffff
#define TSENSORC_SOE_K7_SENSOR16_SOE_K_SHIFT                        16
#define TSENSORC_SOE_K7_SENSOR16_SOE_K_MASK                         0xffff0000
#define TSENSORC_SOE_K8                                0x0924
#define TSENSORC_SOE_K8_SENSOR17_SOE_K_SHIFT                        0
#define TSENSORC_SOE_K8_SENSOR17_SOE_K_MASK                         0x0000ffff
#define TSENSORC_SOE_K8_SENSOR18_SOE_K_SHIFT                        16
#define TSENSORC_SOE_K8_SENSOR18_SOE_K_MASK                         0xffff0000
#define TSENSORC_SOE_K9                                0x0928
#define TSENSORC_SOE_K9_SENSOR19_SOE_K_SHIFT                        0
#define TSENSORC_SOE_K9_SENSOR19_SOE_K_MASK                         0x0000ffff
#define TSENSORC_SOE_K9_SENSOR20_SOE_K_SHIFT                        16
#define TSENSORC_SOE_K9_SENSOR20_SOE_K_MASK                         0xffff0000
#define TSENSORC_SOE_K10                               0x092c
#define TSENSORC_SOE_K10_SENSOR21_SOE_K_SHIFT                       0
#define TSENSORC_SOE_K10_SENSOR21_SOE_K_MASK                        0x0000ffff
#define TSENSORC_SOE_K10_SENSOR22_SOE_K_SHIFT                       16
#define TSENSORC_SOE_K10_SENSOR22_SOE_K_MASK                        0xffff0000
#define TSENSORC_INTR_ALERT_CPU_IRQ_INT_RAW            0x1000
#define TSENSORC_INTR_ALERT_CPU_IRQ_INT_RAW_INTR_RPT_RAW_SHIFT      0
#define TSENSORC_INTR_ALERT_CPU_IRQ_INT_FORCE          0x1004
#define TSENSORC_INTR_ALERT_CPU_IRQ_INT_FORCE_INTR_RPT_RAW_SHIFT    0
#define TSENSORC_INTR_ALERT_CPU_IRQ_INT_MASK           0x1008
#define TSENSORC_INTR_ALERT_CPU_IRQ_INT_MASK_INTR_RPT_MASK_SHIFT    0
#define TSENSORC_INTR_ALERT_CPU_IRQ_INT_MASK_INTR_RPT_MASK_MASK     0x00000001
#define TSENSORC_INTR_ALERT_CPU_IRQ_INT_STATUS         0x100c
#define TSENSORC_INTR_ALERT_CPU_IRQ_INT_STATUS_INTR_RPT_STATUS_SHIFT 0
#define TSENSORC_INTR_ALERT_CPU_IRQ_INT_STATUS_INTR_RPT_STATUS_MASK  0x00000001
#define TSENSORC_CGBYPASS                              0x1010
#define TSENSORC_CGBYPASS_CGBYPASS_SHIFT                            0
#define TSENSORC_CGBYPASS_CGBYPASS_MASK                             0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor1_msg : 32;
    } reg;
}tsensorc_dts_sensor1_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor2_msg : 32;
    } reg;
}tsensorc_dts_sensor2_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor3_msg : 32;
    } reg;
}tsensorc_dts_sensor3_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor4_msg : 32;
    } reg;
}tsensorc_dts_sensor4_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor5_msg : 32;
    } reg;
}tsensorc_dts_sensor5_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor6_msg : 32;
    } reg;
}tsensorc_dts_sensor6_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor7_msg : 32;
    } reg;
}tsensorc_dts_sensor7_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor8_msg : 32;
    } reg;
}tsensorc_dts_sensor8_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor9_msg : 32;
    } reg;
}tsensorc_dts_sensor9_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor10_msg : 32;
    } reg;
}tsensorc_dts_sensor10_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor11_msg : 32;
    } reg;
}tsensorc_dts_sensor11_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor12_msg : 32;
    } reg;
}tsensorc_dts_sensor12_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor13_msg : 32;
    } reg;
}tsensorc_dts_sensor13_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor14_msg : 32;
    } reg;
}tsensorc_dts_sensor14_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor15_msg : 32;
    } reg;
}tsensorc_dts_sensor15_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor16_msg : 32;
    } reg;
}tsensorc_dts_sensor16_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor17_msg : 32;
    } reg;
}tsensorc_dts_sensor17_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor18_msg : 32;
    } reg;
}tsensorc_dts_sensor18_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor19_msg : 32;
    } reg;
}tsensorc_dts_sensor19_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor20_msg : 32;
    } reg;
}tsensorc_dts_sensor20_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor21_msg : 32;
    } reg;
}tsensorc_dts_sensor21_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dts_sensor22_msg : 32;
    } reg;
}tsensorc_dts_sensor22_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor_faultn : 22;
        unsigned int reserved_0    : 10;
    } reg;
}tsensorc_sensor_faultn_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int max_temp_msg : 15;
        unsigned int min_temp_msg : 15;
        unsigned int reserved_0   :  2;
    } reg;
}tsensorc_monitor_msg0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_msg   : 31;
        unsigned int reserved_0 :  1;
    } reg;
}tsensorc_intr_msg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int tsensorc_mode    :  1;
        unsigned int soft_tsensorc_en :  1;
        unsigned int reserved_0       : 14;
        unsigned int _bm_             : 16;
    } reg;
}tsensorc_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int period_cnt0 : 16;
        unsigned int reserved_0  : 16;
    } reg;
}tsensorc_monitor_perior0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int period_cnt1 : 16;
        unsigned int reserved_0  : 16;
    } reg;
}tsensorc_monitor_perior1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int period_cnt2 : 16;
        unsigned int reserved_0  : 16;
    } reg;
}tsensorc_monitor_perior2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int period_cnt3 : 16;
        unsigned int reserved_0  : 16;
    } reg;
}tsensorc_monitor_perior3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int period_cnt4 : 16;
        unsigned int reserved_0  : 16;
    } reg;
}tsensorc_monitor_perior4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int period_cnt5 : 16;
        unsigned int reserved_0  : 16;
    } reg;
}tsensorc_monitor_perior5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int period_cnt6 : 16;
        unsigned int reserved_0  : 16;
    } reg;
}tsensorc_monitor_perior6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int period_cnt_pulse_en :  1;
        unsigned int reserved_0          : 31;
    } reg;
}tsensorc_monitor_pulse_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor_mask : 22;
        unsigned int reserved_0  : 10;
    } reg;
}tsensorc_sensor_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor_mask_pulse_en :  1;
        unsigned int reserved_0           : 31;
    } reg;
}tsensorc_sensor_mask_pulse_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int alert_mask              :  8;
        unsigned int tsensor_rst_req_mask    :  2;
        unsigned int sctrl_tsensor_en_bypass :  1;
        unsigned int reserved_0              : 21;
    } reg;
}tsensorc_ctrl_sig_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int alarm_data0 :  9;
        unsigned int alarm_data1 :  9;
        unsigned int alarm_data2 :  9;
        unsigned int reserved_0  :  5;
    } reg;
}tsensorc_alarm_temp0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int alarm_data3 :  9;
        unsigned int alarm_data4 :  9;
        unsigned int alarm_data5 :  9;
        unsigned int reserved_0  :  5;
    } reg;
}tsensorc_alarm_temp1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int alarm_data6 :  9;
        unsigned int alarm_data7 :  9;
        unsigned int reserved_0  : 14;
    } reg;
}tsensorc_alarm_temp2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hsyt_value0 :  4;
        unsigned int hsyt_value1 :  4;
        unsigned int hsyt_value2 :  4;
        unsigned int hsyt_value3 :  4;
        unsigned int hsyt_value4 :  4;
        unsigned int hsyt_value5 :  4;
        unsigned int hsyt_value6 :  4;
        unsigned int reserved_0  :  4;
    } reg;
}tsensorc_hsyt_value_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_auto_clk_hub_bypass :  1;
        unsigned int reserved_0             : 31;
    } reg;
}tsensorc_gt_auto_byp_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_hub :  1;
        unsigned int reserved_0 : 31;
    } reg;
}tsensorc_st_clk_hub_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int timeline_maxval_clr :  1;
        unsigned int timeline_minval_clr :  1;
        unsigned int reserved_0          : 30;
    } reg;
}tsensorc_timeline_val_clr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int timeline_maxval :  9;
        unsigned int timeline_minval :  9;
        unsigned int reserved_0      : 14;
    } reg;
}tsensorc_timeline_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor_soe_y : 17;
        unsigned int reserved_0   : 15;
    } reg;
}tsensorc_soe_y_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor1_soe_k : 16;
        unsigned int sensor2_soe_k : 16;
    } reg;
}tsensorc_soe_k0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor3_soe_k : 16;
        unsigned int sensor4_soe_k : 16;
    } reg;
}tsensorc_soe_k1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor5_soe_k : 16;
        unsigned int sensor6_soe_k : 16;
    } reg;
}tsensorc_soe_k2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor7_soe_k : 16;
        unsigned int sensor8_soe_k : 16;
    } reg;
}tsensorc_soe_k3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor9_soe_k  : 16;
        unsigned int sensor10_soe_k : 16;
    } reg;
}tsensorc_soe_k4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor11_soe_k : 16;
        unsigned int sensor12_soe_k : 16;
    } reg;
}tsensorc_soe_k5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor13_soe_k : 16;
        unsigned int sensor14_soe_k : 16;
    } reg;
}tsensorc_soe_k6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor15_soe_k : 16;
        unsigned int sensor16_soe_k : 16;
    } reg;
}tsensorc_soe_k7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor17_soe_k : 16;
        unsigned int sensor18_soe_k : 16;
    } reg;
}tsensorc_soe_k8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor19_soe_k : 16;
        unsigned int sensor20_soe_k : 16;
    } reg;
}tsensorc_soe_k9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sensor21_soe_k : 16;
        unsigned int sensor22_soe_k : 16;
    } reg;
}tsensorc_soe_k10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_rpt_raw :  1;
        unsigned int reserved_0   : 31;
    } reg;
}tsensorc_intr_alert_cpu_irq_int_raw_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_rpt_raw :  1;
        unsigned int reserved_0   : 31;
    } reg;
}tsensorc_intr_alert_cpu_irq_int_force_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_rpt_mask :  1;
        unsigned int reserved_0    : 31;
    } reg;
}tsensorc_intr_alert_cpu_irq_int_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_rpt_status :  1;
        unsigned int reserved_0      : 31;
    } reg;
}tsensorc_intr_alert_cpu_irq_int_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cgbypass   :  1;
        unsigned int reserved_0 : 31;
    } reg;
}tsensorc_cgbypass_t;

#endif
