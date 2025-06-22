// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef ISP_MCU_CTL_REGIF_H
#define ISP_MCU_CTL_REGIF_H

#define ISP_MCU_CTL_CPU_HALT0                           0x0000
#define ISP_MCU_CTL_CPU_HALT0_CPU_HALT0_SHIFT                              0
#define ISP_MCU_CTL_CPU_HALT0_CPU_HALT0_MASK                               0x00000001
#define ISP_MCU_CTL_CPU_HALT1                           0x0004
#define ISP_MCU_CTL_CPU_HALT1_CPU_HALT1_SHIFT                              0
#define ISP_MCU_CTL_CPU_HALT1_CPU_HALT1_MASK                               0x00000001
#define ISP_MCU_CTL_ISP_MCU_CLKQ_STATE                  0x0008
#define ISP_MCU_CTL_ISP_MCU_CLKQ_STATE_ISP_MCU_CLKQ_STATE_SHIFT            0
#define ISP_MCU_CTL_ISP_MCU_CLKQ_STATE_ISP_MCU_CLKQ_STATE_MASK             0x00007fff
#define ISP_MCU_CTL_CFG_RV_BARADDR0_LOW                 0x000c
#define ISP_MCU_CTL_CFG_RV_BARADDR0_HIGH                0x0010
#define ISP_MCU_CTL_CFG_RV_BARADDR0_HIGH_CFG_RV_BARADDR0_H_SHIFT           0
#define ISP_MCU_CTL_CFG_RV_BARADDR0_HIGH_CFG_RV_BARADDR0_H_MASK            0x0000003f
#define ISP_MCU_CTL_CFG_RV_BARADDR1_LOW                 0x0014
#define ISP_MCU_CTL_CFG_RV_BARADDR1_HIGH                0x0018
#define ISP_MCU_CTL_CFG_RV_BARADDR1_HIGH_CFG_RV_BARADDR1_H_SHIFT           0
#define ISP_MCU_CTL_CFG_RV_BARADDR1_HIGH_CFG_RV_BARADDR1_H_MASK            0x0000003f
#define ISP_MCU_CTL_GIC_GICT_ALLOW_NS                   0x001c
#define ISP_MCU_CTL_GIC_GICT_ALLOW_NS_GICT_ALLOW_NS_SHIFT                  0
#define ISP_MCU_CTL_GIC_GICT_ALLOW_NS_GICT_ALLOW_NS_MASK                   0x00000001
#define ISP_MCU_CTL_GIC_GICP_ALLOW_NS                   0x0020
#define ISP_MCU_CTL_GIC_GICP_ALLOW_NS_GICP_ALLOW_NS_SHIFT                  0
#define ISP_MCU_CTL_GIC_GICP_ALLOW_NS_GICP_ALLOW_NS_MASK                   0x00000001
#define ISP_MCU_CTL_GIC_GICD_CTLR_DS                    0x0024
#define ISP_MCU_CTL_GIC_GICD_CTLR_DS_GICD_CTLR_DS_SHIFT                    0
#define ISP_MCU_CTL_GIC_GICD_CTLR_DS_GICD_CTLR_DS_MASK                     0x00000001
#define ISP_MCU_CTL_CPU_SPP_QOS                         0x0028
#define ISP_MCU_CTL_CPU_SPP_QOS_CPU_SPP_ARQOS_SHIFT                        0
#define ISP_MCU_CTL_CPU_SPP_QOS_CPU_SPP_ARQOS_MASK                         0x0000000f
#define ISP_MCU_CTL_CPU_SPP_QOS_CPU_SPP_AWQOS_SHIFT                        4
#define ISP_MCU_CTL_CPU_SPP_QOS_CPU_SPP_AWQOS_MASK                         0x000000f0
#define ISP_MCU_CTL_ISP_MCU_CLKQ_BYPASS                 0x002c
#define ISP_MCU_CTL_ISP_MCU_CLKQ_BYPASS_ISP_MCU_CLKQ_BYPASS_SHIFT          0
#define ISP_MCU_CTL_ISP_MCU_CLKQ_BYPASS_ISP_MCU_CLKQ_BYPASS_MASK           0x0000001f
#define ISP_MCU_CTL_ISP_MCU_CLKQREQN_SOFT               0x0030
#define ISP_MCU_CTL_ISP_MCU_CLKQREQN_SOFT_ISP_MCU_CLKQREQN_SOFT_SHIFT      0
#define ISP_MCU_CTL_ISP_MCU_CLKQREQN_SOFT_ISP_MCU_CLKQREQN_SOFT_MASK       0x0000001f
#define ISP_MCU_CTL_ISP_MCU_CLKQ_IDLE                   0x0034
#define ISP_MCU_CTL_ISP_MCU_CLKQ_IDLE_MCU_CLKQ_IDLE_SHIFT                  0
#define ISP_MCU_CTL_ISP_MCU_CLKQ_IDLE_MCU_CLKQ_IDLE_MASK                   0x0000001f
#define ISP_MCU_CTL_CLK_ATB_QSM_CTL                     0x0038
#define ISP_MCU_CTL_CLK_ATB_QSM_CTL_CLK_ATB_DATA_QDLY_CNT_SHIFT            0
#define ISP_MCU_CTL_CLK_ATB_QSM_CTL_CLK_ATB_DATA_QDLY_CNT_MASK             0x000000ff
#define ISP_MCU_CTL_CLK_ATB_QSM_CTL_CLK_ATB_QDLY_BYP_SHIFT                 8
#define ISP_MCU_CTL_CLK_ATB_QSM_CTL_CLK_ATB_QDLY_BYP_MASK                  0x00000100
#define ISP_MCU_CTL_CLK_DBG_QSM_CTL                     0x003c
#define ISP_MCU_CTL_CLK_DBG_QSM_CTL_CLK_DBG_DATA_QDLY_CNT_SHIFT            0
#define ISP_MCU_CTL_CLK_DBG_QSM_CTL_CLK_DBG_DATA_QDLY_CNT_MASK             0x000000ff
#define ISP_MCU_CTL_CLK_DBG_QSM_CTL_CLK_DBG_QDLY_BYP_SHIFT                 8
#define ISP_MCU_CTL_CLK_DBG_QSM_CTL_CLK_DBG_QDLY_BYP_MASK                  0x00000100
#define ISP_MCU_CTL_CLK_GIC_QSM_CTL                     0x0040
#define ISP_MCU_CTL_CLK_GIC_QSM_CTL_CLK_GIC_DATA_QDLY_CNT_SHIFT            0
#define ISP_MCU_CTL_CLK_GIC_QSM_CTL_CLK_GIC_DATA_QDLY_CNT_MASK             0x000000ff
#define ISP_MCU_CTL_CLK_GIC_QSM_CTL_CLK_GIC_QDLY_BYP_SHIFT                 8
#define ISP_MCU_CTL_CLK_GIC_QSM_CTL_CLK_GIC_QDLY_BYP_MASK                  0x00000100
#define ISP_MCU_CTL_CLK_PDBG_QSM_CTL                    0x0044
#define ISP_MCU_CTL_CLK_PDBG_QSM_CTL_CLK_PDBG_DATA_QDLY_CNT_SHIFT          0
#define ISP_MCU_CTL_CLK_PDBG_QSM_CTL_CLK_PDBG_DATA_QDLY_CNT_MASK           0x000000ff
#define ISP_MCU_CTL_CLK_PDBG_QSM_CTL_CLK_PDBG_QDLY_BYP_SHIFT               8
#define ISP_MCU_CTL_CLK_PDBG_QSM_CTL_CLK_PDBG_QDLY_BYP_MASK                0x00000100
#define ISP_MCU_CTL_CLK_SYS_QSM_CTL                     0x0048
#define ISP_MCU_CTL_CLK_SYS_QSM_CTL_CLK_SYS_DATA_QDLY_CNT_SHIFT            0
#define ISP_MCU_CTL_CLK_SYS_QSM_CTL_CLK_SYS_DATA_QDLY_CNT_MASK             0x000000ff
#define ISP_MCU_CTL_CLK_SYS_QSM_CTL_CLK_SYS_QDLY_BYP_SHIFT                 8
#define ISP_MCU_CTL_CLK_SYS_QSM_CTL_CLK_SYS_QDLY_BYP_MASK                  0x00000100
#define ISP_MCU_CTL_MCU_ISP_DATA_QOS                    0x004c
#define ISP_MCU_CTL_MCU_ISP_DATA_QOS_MCU_ISP_DATA_ARQOS_SHIFT              0
#define ISP_MCU_CTL_MCU_ISP_DATA_QOS_MCU_ISP_DATA_ARQOS_MASK               0x0000000f
#define ISP_MCU_CTL_MCU_ISP_DATA_QOS_MCU_ISP_DATA_AWQOS_SHIFT              4
#define ISP_MCU_CTL_MCU_ISP_DATA_QOS_MCU_ISP_DATA_AWQOS_MASK               0x000000f0
#define ISP_MCU_CTL_MCU_ISP_DATA_AXCACHE                0x0050
#define ISP_MCU_CTL_MCU_ISP_DATA_AXCACHE_MCU_ISP_DATA_ARCACHE_SHIFT        0
#define ISP_MCU_CTL_MCU_ISP_DATA_AXCACHE_MCU_ISP_DATA_ARCACHE_MASK         0x0000000f
#define ISP_MCU_CTL_MCU_ISP_DATA_AXCACHE_MCU_ISP_DATA_AWCACHE_SHIFT        4
#define ISP_MCU_CTL_MCU_ISP_DATA_AXCACHE_MCU_ISP_DATA_AWCACHE_MASK         0x000000f0
#define ISP_MCU_CTL_MCU_ISP_DATA_AXCACHE_SEL            0x0054
#define ISP_MCU_CTL_MCU_ISP_DATA_AXCACHE_SEL_MCU_ISP_DATA_AXCACHE_SEL_SHIFT 0
#define ISP_MCU_CTL_MCU_ISP_DATA_AXCACHE_SEL_MCU_ISP_DATA_AXCACHE_SEL_MASK  0x00000001
#define ISP_MCU_CTL_MCU_ISP_CFG_AXCACHE                 0x0058
#define ISP_MCU_CTL_MCU_ISP_CFG_AXCACHE_MCU_ISP_CFG_ARCACHE_SHIFT          0
#define ISP_MCU_CTL_MCU_ISP_CFG_AXCACHE_MCU_ISP_CFG_ARCACHE_MASK           0x0000000f
#define ISP_MCU_CTL_MCU_ISP_CFG_AXCACHE_MCU_ISP_CFG_AWCACHE_SHIFT          4
#define ISP_MCU_CTL_MCU_ISP_CFG_AXCACHE_MCU_ISP_CFG_AWCACHE_MASK           0x000000f0
#define ISP_MCU_CTL_MCU_ISP_CFG_AXCACHE_SEL             0x005c
#define ISP_MCU_CTL_MCU_ISP_CFG_AXCACHE_SEL_MCU_ISP_CFG_AXCACHE_SEL_SHIFT  0
#define ISP_MCU_CTL_MCU_ISP_CFG_AXCACHE_SEL_MCU_ISP_CFG_AXCACHE_SEL_MASK   0x00000001
#define ISP_MCU_CTL_ISP_STROBE                          0x0060
#define ISP_MCU_CTL_ISP_STROBE_ISP_STROBE_SHIFT                            0
#define ISP_MCU_CTL_ISP_STROBE_ISP_STROBE_MASK                             0x00000001
#define ISP_MCU_CTL_NIC_LP_CTL_BUS                      0x0064
#define ISP_MCU_CTL_NIC_LP_CTL_BUS_DEL_VAL_CLK_BUS_SHIFT                   0
#define ISP_MCU_CTL_NIC_LP_CTL_BUS_DEL_VAL_CLK_BUS_MASK                    0x0000003f
#define ISP_MCU_CTL_NIC_LP_CTL_BUS_CFG_ASYNC_MODE_CLK_BUS_SHIFT            6
#define ISP_MCU_CTL_NIC_LP_CTL_BUS_CFG_ASYNC_MODE_CLK_BUS_MASK             0x00000040
#define ISP_MCU_CTL_NIC_LP_CTL_DIV4                     0x0068
#define ISP_MCU_CTL_NIC_LP_CTL_DIV4_DEL_VAL_CLK_DIV4_SHIFT                 0
#define ISP_MCU_CTL_NIC_LP_CTL_DIV4_DEL_VAL_CLK_DIV4_MASK                  0x0000003f
#define ISP_MCU_CTL_NIC_LP_CTL_DIV4_CFG_ASYNC_MODE_CLK_DIV4_SHIFT          6
#define ISP_MCU_CTL_NIC_LP_CTL_DIV4_CFG_ASYNC_MODE_CLK_DIV4_MASK           0x00000040
#define ISP_MCU_CTL_NIC_LP_CTL_PERI                     0x006c
#define ISP_MCU_CTL_NIC_LP_CTL_PERI_DEL_VAL_CLK_PERI_SHIFT                 0
#define ISP_MCU_CTL_NIC_LP_CTL_PERI_DEL_VAL_CLK_PERI_MASK                  0x0000003f
#define ISP_MCU_CTL_NIC_LP_CTL_PERI_CFG_ASYNC_MODE_CLK_PERI_SHIFT          6
#define ISP_MCU_CTL_NIC_LP_CTL_PERI_CFG_ASYNC_MODE_CLK_PERI_MASK           0x00000040
#define ISP_MCU_CTL_NIC_LP_STAT                         0x0070
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_00             0x0074
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_00_MCU_ISP_CFG_MAP_00_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_00_MCU_ISP_CFG_MAP_00_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_01             0x0078
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_01_MCU_ISP_CFG_MAP_01_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_01_MCU_ISP_CFG_MAP_01_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_10             0x007c
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_10_MCU_ISP_CFG_MAP_10_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_10_MCU_ISP_CFG_MAP_10_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_11             0x0080
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_11_MCU_ISP_CFG_MAP_11_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_11_MCU_ISP_CFG_MAP_11_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_20             0x0084
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_20_MCU_ISP_CFG_MAP_20_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_20_MCU_ISP_CFG_MAP_20_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_21             0x0088
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_21_MCU_ISP_CFG_MAP_21_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_21_MCU_ISP_CFG_MAP_21_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_30             0x008c
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_30_MCU_ISP_CFG_MAP_30_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_30_MCU_ISP_CFG_MAP_30_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_31             0x0090
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_31_MCU_ISP_CFG_MAP_31_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_31_MCU_ISP_CFG_MAP_31_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_40             0x0094
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_40_MCU_ISP_CFG_MAP_40_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_40_MCU_ISP_CFG_MAP_40_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_41             0x0098
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_41_MCU_ISP_CFG_MAP_41_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_41_MCU_ISP_CFG_MAP_41_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_60             0x009c
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_60_MCU_ISP_CFG_MAP_60_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_60_MCU_ISP_CFG_MAP_60_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_61             0x00a0
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_61_MCU_ISP_CFG_MAP_61_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_61_MCU_ISP_CFG_MAP_61_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_70             0x00a4
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_70_MCU_ISP_CFG_MAP_70_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_70_MCU_ISP_CFG_MAP_70_MASK        0x3f800000
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_71             0x00a8
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_71_MCU_ISP_CFG_MAP_71_SHIFT       23
#define ISP_MCU_CTL_MCU_ISP_CFG_ADDR_MAP_71_MCU_ISP_CFG_MAP_71_MASK        0x3f800000
#define ISP_MCU_CTL_UART_DEBUG                          0x00ac
#define ISP_MCU_CTL_I2C_DEBUG_0                         0x00b0
#define ISP_MCU_CTL_I2C_DEBUG_0_I2C_DEBUG_0_SHIFT                          0
#define ISP_MCU_CTL_I2C_DEBUG_0_I2C_DEBUG_0_MASK                           0x000fffff
#define ISP_MCU_CTL_I2C_DEBUG_1                         0x00b4
#define ISP_MCU_CTL_I2C_DEBUG_1_I2C_DEBUG_1_SHIFT                          0
#define ISP_MCU_CTL_I2C_DEBUG_1_I2C_DEBUG_1_MASK                           0x000fffff
#define ISP_MCU_CTL_I3C_DEBUG_0_31_0                    0x00b8
#define ISP_MCU_CTL_I3C_DEBUG_0_55_32                   0x00bc
#define ISP_MCU_CTL_I3C_DEBUG_0_55_32_I3C_DEBUG_0_55_32_SHIFT              0
#define ISP_MCU_CTL_I3C_DEBUG_0_55_32_I3C_DEBUG_0_55_32_MASK               0x00ffffff
#define ISP_MCU_CTL_I3C_DEBUG_1_31_0                    0x00c0
#define ISP_MCU_CTL_I3C_DEBUG_1_55_32                   0x00c4
#define ISP_MCU_CTL_I3C_DEBUG_1_55_32_I3C_DEBUG_1_55_32_SHIFT              0
#define ISP_MCU_CTL_I3C_DEBUG_1_55_32_I3C_DEBUG_1_55_32_MASK               0x00ffffff
#define ISP_MCU_CTL_I3C_DEBUG_2_31_0                    0x00c8
#define ISP_MCU_CTL_I3C_DEBUG_2_55_32                   0x00cc
#define ISP_MCU_CTL_I3C_DEBUG_2_55_32_I3C_DEBUG_2_55_32_SHIFT              0
#define ISP_MCU_CTL_I3C_DEBUG_2_55_32_I3C_DEBUG_2_55_32_MASK               0x00ffffff
#define ISP_MCU_CTL_I3C_DEBUG_3_31_0                    0x00d0
#define ISP_MCU_CTL_I3C_DEBUG_3_55_32                   0x00d4
#define ISP_MCU_CTL_I3C_DEBUG_3_55_32_I3C_DEBUG_3_55_32_SHIFT              0
#define ISP_MCU_CTL_I3C_DEBUG_3_55_32_I3C_DEBUG_3_55_32_MASK               0x00ffffff
#define ISP_MCU_CTL_DMA_MSG_31_0                        0x00d8
#define ISP_MCU_CTL_DMA_MSG_63_32                       0x00dc
#define ISP_MCU_CTL_DMA_MSG_95_64                       0x00e0
#define ISP_MCU_CTL_DMA_MSG_127_96                      0x00e4
#define ISP_MCU_CTL_DMA_DEBUG_CFG                       0x00e8
#define ISP_MCU_CTL_DMA_DEBUG_CFG_DMA_DEBUG_CFG_2_0_SHIFT                  0
#define ISP_MCU_CTL_DMA_DEBUG_CFG_DMA_DEBUG_CFG_2_0_MASK                   0x00000007
#define ISP_MCU_CTL_CPU_MON_0                           0x00ec
#define ISP_MCU_CTL_CPU_MON_1                           0x00f0
#define ISP_MCU_CTL_CPU_MON_2                           0x00f4
#define ISP_MCU_CTL_GIC_MON_0                           0x00f8
#define ISP_MCU_CTL_GIC_MON_1                           0x00fc
#define ISP_MCU_CTL_GIC_MON_2                           0x0100
#define ISP_MCU_CTL_GIC_MON_3                           0x0104
#define ISP_MCU_CTL_REGFILE_CGBYPASS                    0x0108
#define ISP_MCU_CTL_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                0
#define ISP_MCU_CTL_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                 0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int cpu_halt0  :  1;
        unsigned int reserved_0 : 31;
    } reg;
}isp_mcu_ctl_cpu_halt0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cpu_halt1  :  1;
        unsigned int reserved_0 : 31;
    } reg;
}isp_mcu_ctl_cpu_halt1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int isp_mcu_clkq_state : 15;
        unsigned int reserved_0         : 17;
    } reg;
}isp_mcu_ctl_isp_mcu_clkq_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_rv_baraddr0_l : 32;
    } reg;
}isp_mcu_ctl_cfg_rv_baraddr0_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_rv_baraddr0_h :  6;
        unsigned int reserved_0        : 26;
    } reg;
}isp_mcu_ctl_cfg_rv_baraddr0_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_rv_baraddr1_l : 32;
    } reg;
}isp_mcu_ctl_cfg_rv_baraddr1_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cfg_rv_baraddr1_h :  6;
        unsigned int reserved_0        : 26;
    } reg;
}isp_mcu_ctl_cfg_rv_baraddr1_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gict_allow_ns :  1;
        unsigned int reserved_0    : 31;
    } reg;
}isp_mcu_ctl_gic_gict_allow_ns_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gicp_allow_ns :  1;
        unsigned int reserved_0    : 31;
    } reg;
}isp_mcu_ctl_gic_gicp_allow_ns_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gicd_ctlr_ds :  1;
        unsigned int reserved_0   : 31;
    } reg;
}isp_mcu_ctl_gic_gicd_ctlr_ds_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cpu_spp_arqos :  4;
        unsigned int cpu_spp_awqos :  4;
        unsigned int reserved_0    : 24;
    } reg;
}isp_mcu_ctl_cpu_spp_qos_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int isp_mcu_clkq_bypass :  5;
        unsigned int reserved_0          : 27;
    } reg;
}isp_mcu_ctl_isp_mcu_clkq_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int isp_mcu_clkqreqn_soft :  5;
        unsigned int reserved_0            : 27;
    } reg;
}isp_mcu_ctl_isp_mcu_clkqreqn_soft_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mcu_clkq_idle :  5;
        unsigned int reserved_0    : 27;
    } reg;
}isp_mcu_ctl_isp_mcu_clkq_idle_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clk_atb_data_qdly_cnt :  8;
        unsigned int clk_atb_qdly_byp      :  1;
        unsigned int reserved_0            : 23;
    } reg;
}isp_mcu_ctl_clk_atb_qsm_ctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clk_dbg_data_qdly_cnt :  8;
        unsigned int clk_dbg_qdly_byp      :  1;
        unsigned int reserved_0            : 23;
    } reg;
}isp_mcu_ctl_clk_dbg_qsm_ctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clk_gic_data_qdly_cnt :  8;
        unsigned int clk_gic_qdly_byp      :  1;
        unsigned int reserved_0            : 23;
    } reg;
}isp_mcu_ctl_clk_gic_qsm_ctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clk_pdbg_data_qdly_cnt :  8;
        unsigned int clk_pdbg_qdly_byp      :  1;
        unsigned int reserved_0             : 23;
    } reg;
}isp_mcu_ctl_clk_pdbg_qsm_ctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clk_sys_data_qdly_cnt :  8;
        unsigned int clk_sys_qdly_byp      :  1;
        unsigned int reserved_0            : 23;
    } reg;
}isp_mcu_ctl_clk_sys_qsm_ctl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mcu_isp_data_arqos :  4;
        unsigned int mcu_isp_data_awqos :  4;
        unsigned int reserved_0         : 24;
    } reg;
}isp_mcu_ctl_mcu_isp_data_qos_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mcu_isp_data_arcache :  4;
        unsigned int mcu_isp_data_awcache :  4;
        unsigned int reserved_0           : 24;
    } reg;
}isp_mcu_ctl_mcu_isp_data_axcache_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mcu_isp_data_axcache_sel :  1;
        unsigned int reserved_0               : 31;
    } reg;
}isp_mcu_ctl_mcu_isp_data_axcache_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mcu_isp_cfg_arcache :  4;
        unsigned int mcu_isp_cfg_awcache :  4;
        unsigned int reserved_0          : 24;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_axcache_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mcu_isp_cfg_axcache_sel :  1;
        unsigned int reserved_0              : 31;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_axcache_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int isp_strobe :  1;
        unsigned int reserved_0 : 31;
    } reg;
}isp_mcu_ctl_isp_strobe_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int del_val_clk_bus        :  6;
        unsigned int cfg_async_mode_clk_bus :  1;
        unsigned int reserved_0             : 25;
    } reg;
}isp_mcu_ctl_nic_lp_ctl_bus_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int del_val_clk_div4        :  6;
        unsigned int cfg_async_mode_clk_div4 :  1;
        unsigned int reserved_0              : 25;
    } reg;
}isp_mcu_ctl_nic_lp_ctl_div4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int del_val_clk_peri        :  6;
        unsigned int cfg_async_mode_clk_peri :  1;
        unsigned int reserved_0              : 25;
    } reg;
}isp_mcu_ctl_nic_lp_ctl_peri_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int nic_lp_stat : 32;
    } reg;
}isp_mcu_ctl_nic_lp_stat_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_00 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_00_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_01 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_01_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_10 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_11 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_20 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_20_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_21 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_21_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_30 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_30_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_31 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_31_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_40 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_40_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_41 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_41_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_60 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_60_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_61 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_61_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_70 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_70_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0         : 23;
        unsigned int mcu_isp_cfg_map_71 :  7;
        unsigned int reserved_1         :  2;
    } reg;
}isp_mcu_ctl_mcu_isp_cfg_addr_map_71_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int uart_debug : 32;
    } reg;
}isp_mcu_ctl_uart_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c_debug_0 : 20;
        unsigned int reserved_0  : 12;
    } reg;
}isp_mcu_ctl_i2c_debug_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i2c_debug_1 : 20;
        unsigned int reserved_0  : 12;
    } reg;
}isp_mcu_ctl_i2c_debug_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i3c_debug_0_31_0 : 32;
    } reg;
}isp_mcu_ctl_i3c_debug_0_31_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i3c_debug_0_55_32 : 24;
        unsigned int reserved_0        :  8;
    } reg;
}isp_mcu_ctl_i3c_debug_0_55_32_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i3c_debug_1_31_0 : 32;
    } reg;
}isp_mcu_ctl_i3c_debug_1_31_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i3c_debug_1_55_32 : 24;
        unsigned int reserved_0        :  8;
    } reg;
}isp_mcu_ctl_i3c_debug_1_55_32_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i3c_debug_2_31_0 : 32;
    } reg;
}isp_mcu_ctl_i3c_debug_2_31_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i3c_debug_2_55_32 : 24;
        unsigned int reserved_0        :  8;
    } reg;
}isp_mcu_ctl_i3c_debug_2_55_32_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i3c_debug_3_31_0 : 32;
    } reg;
}isp_mcu_ctl_i3c_debug_3_31_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int i3c_debug_3_55_32 : 24;
        unsigned int reserved_0        :  8;
    } reg;
}isp_mcu_ctl_i3c_debug_3_55_32_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dma_msg_31_0 : 32;
    } reg;
}isp_mcu_ctl_dma_msg_31_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dma_msg_63_32 : 32;
    } reg;
}isp_mcu_ctl_dma_msg_63_32_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dma_msg_95_64 : 32;
    } reg;
}isp_mcu_ctl_dma_msg_95_64_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dma_msg_127_96 : 32;
    } reg;
}isp_mcu_ctl_dma_msg_127_96_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dma_debug_cfg_2_0 :  3;
        unsigned int reserved_0        : 29;
    } reg;
}isp_mcu_ctl_dma_debug_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cpu_mon_31_0 : 32;
    } reg;
}isp_mcu_ctl_cpu_mon_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cpu_mon_63_32 : 32;
    } reg;
}isp_mcu_ctl_cpu_mon_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cpu_mon_95_64 : 32;
    } reg;
}isp_mcu_ctl_cpu_mon_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gic_mon_31_0 : 32;
    } reg;
}isp_mcu_ctl_gic_mon_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gic_mon_63_32 : 32;
    } reg;
}isp_mcu_ctl_gic_mon_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gic_mon_95_64 : 32;
    } reg;
}isp_mcu_ctl_gic_mon_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gic_mon_127_96 : 32;
    } reg;
}isp_mcu_ctl_gic_mon_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}isp_mcu_ctl_regfile_cgbypass_t;

#endif
