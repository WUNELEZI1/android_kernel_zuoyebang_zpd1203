// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef PERI_CRG_REGIF_H
#define PERI_CRG_REGIF_H

#define PERI_CRG_CLKGT0_W1S                        0x0000
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_SPINLOCK_SHIFT                                0
#define PERI_CRG_CLKGT0_W1S_GT_CLK_OCM_SHIFT                                           1
#define PERI_CRG_CLKGT0_W1S_GT_CLK_NPU_OCM_NIC_SHIFT                                   2
#define PERI_CRG_CLKGT0_W1S_GT_CLK_PCIE_AUX_SHIFT                                      3
#define PERI_CRG_CLKGT0_W1S_GT_CLK_NPU_OCM_ADB_MST_SHIFT                               4
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_TIMER_NS_SHIFT                                5
#define PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_TIMER0_SHIFT                                   6
#define PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_TIMER1_SHIFT                                   7
#define PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_TIMER2_SHIFT                                   8
#define PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_TIMER3_SHIFT                                   9
#define PERI_CRG_CLKGT0_W1S_GT_CLK_CSI_SYS_SHIFT                                       10
#define PERI_CRG_CLKGT0_W1S_GT_CLK_DSI_SYS_SHIFT                                       11
#define PERI_CRG_CLKGT0_W1S_GT_CLK_SYSBUS_CSI_SHIFT                                    12
#define PERI_CRG_CLKGT0_W1S_GT_CLK_CFGBUS_CSI_SHIFT                                    13
#define PERI_CRG_CLKGT0_W1S_GT_CLK_CFGBUS_DSI_SHIFT                                    14
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_IPC1_SHIFT                                    15
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_IPC2_SHIFT                                    16
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_SPI4_SHIFT                                         17
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_SPI5_SHIFT                                         18
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_SPI6_SHIFT                                         19
#define PERI_CRG_CLKGT0_W1S_GT_CLK_SPI4_SHIFT                                          20
#define PERI_CRG_CLKGT0_W1S_GT_CLK_SPI5_SHIFT                                          21
#define PERI_CRG_CLKGT0_W1S_GT_CLK_SPI6_SHIFT                                          22
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_WDT1_SHIFT                                    23
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_WDT0_SHIFT                                    24
#define PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_WDT0_SHIFT                                     25
#define PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_WDT1_SHIFT                                     26
#define PERI_CRG_CLKGT0_W1S_GT_CLK_PERFMON_SHIFT                                       27
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_PERFMON_PERI_SHIFT                                 28
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_PERFMON_SHIFT                                      29
#define PERI_CRG_CLKGT0_W1S_GT_PCLK_PERFSTAT_SHIFT                                     30
#define PERI_CRG_CLKGT0_W1C                        0x0004
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_PERI_SPINLOCK_SHIFT                                0
#define PERI_CRG_CLKGT0_W1C_GT_CLK_OCM_SHIFT                                           1
#define PERI_CRG_CLKGT0_W1C_GT_CLK_NPU_OCM_NIC_SHIFT                                   2
#define PERI_CRG_CLKGT0_W1C_GT_CLK_PCIE_AUX_SHIFT                                      3
#define PERI_CRG_CLKGT0_W1C_GT_CLK_NPU_OCM_ADB_MST_SHIFT                               4
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_PERI_TIMER_NS_SHIFT                                5
#define PERI_CRG_CLKGT0_W1C_GT_CLK_PERI_TIMER0_SHIFT                                   6
#define PERI_CRG_CLKGT0_W1C_GT_CLK_PERI_TIMER1_SHIFT                                   7
#define PERI_CRG_CLKGT0_W1C_GT_CLK_PERI_TIMER2_SHIFT                                   8
#define PERI_CRG_CLKGT0_W1C_GT_CLK_PERI_TIMER3_SHIFT                                   9
#define PERI_CRG_CLKGT0_W1C_GT_CLK_CSI_SYS_SHIFT                                       10
#define PERI_CRG_CLKGT0_W1C_GT_CLK_DSI_SYS_SHIFT                                       11
#define PERI_CRG_CLKGT0_W1C_GT_CLK_SYSBUS_CSI_SHIFT                                    12
#define PERI_CRG_CLKGT0_W1C_GT_CLK_CFGBUS_CSI_SHIFT                                    13
#define PERI_CRG_CLKGT0_W1C_GT_CLK_CFGBUS_DSI_SHIFT                                    14
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_PERI_IPC1_SHIFT                                    15
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_PERI_IPC2_SHIFT                                    16
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_SPI4_SHIFT                                         17
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_SPI5_SHIFT                                         18
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_SPI6_SHIFT                                         19
#define PERI_CRG_CLKGT0_W1C_GT_CLK_SPI4_SHIFT                                          20
#define PERI_CRG_CLKGT0_W1C_GT_CLK_SPI5_SHIFT                                          21
#define PERI_CRG_CLKGT0_W1C_GT_CLK_SPI6_SHIFT                                          22
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_PERI_WDT1_SHIFT                                    23
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_PERI_WDT0_SHIFT                                    24
#define PERI_CRG_CLKGT0_W1C_GT_CLK_PERI_WDT0_SHIFT                                     25
#define PERI_CRG_CLKGT0_W1C_GT_CLK_PERI_WDT1_SHIFT                                     26
#define PERI_CRG_CLKGT0_W1C_GT_CLK_PERFMON_SHIFT                                       27
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_PERFMON_PERI_SHIFT                                 28
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_PERFMON_SHIFT                                      29
#define PERI_CRG_CLKGT0_W1C_GT_PCLK_PERFSTAT_SHIFT                                     30
#define PERI_CRG_CLKGT0_RO                         0x0008
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_SPINLOCK_SHIFT                                 0
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_SPINLOCK_MASK                                  0x00000001
#define PERI_CRG_CLKGT0_RO_GT_CLK_OCM_SHIFT                                            1
#define PERI_CRG_CLKGT0_RO_GT_CLK_OCM_MASK                                             0x00000002
#define PERI_CRG_CLKGT0_RO_GT_CLK_NPU_OCM_NIC_SHIFT                                    2
#define PERI_CRG_CLKGT0_RO_GT_CLK_NPU_OCM_NIC_MASK                                     0x00000004
#define PERI_CRG_CLKGT0_RO_GT_CLK_PCIE_AUX_SHIFT                                       3
#define PERI_CRG_CLKGT0_RO_GT_CLK_PCIE_AUX_MASK                                        0x00000008
#define PERI_CRG_CLKGT0_RO_GT_CLK_NPU_OCM_ADB_MST_SHIFT                                4
#define PERI_CRG_CLKGT0_RO_GT_CLK_NPU_OCM_ADB_MST_MASK                                 0x00000010
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_TIMER_NS_SHIFT                                 5
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_TIMER_NS_MASK                                  0x00000020
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_TIMER0_SHIFT                                    6
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_TIMER0_MASK                                     0x00000040
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_TIMER1_SHIFT                                    7
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_TIMER1_MASK                                     0x00000080
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_TIMER2_SHIFT                                    8
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_TIMER2_MASK                                     0x00000100
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_TIMER3_SHIFT                                    9
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_TIMER3_MASK                                     0x00000200
#define PERI_CRG_CLKGT0_RO_GT_CLK_CSI_SYS_SHIFT                                        10
#define PERI_CRG_CLKGT0_RO_GT_CLK_CSI_SYS_MASK                                         0x00000400
#define PERI_CRG_CLKGT0_RO_GT_CLK_DSI_SYS_SHIFT                                        11
#define PERI_CRG_CLKGT0_RO_GT_CLK_DSI_SYS_MASK                                         0x00000800
#define PERI_CRG_CLKGT0_RO_GT_CLK_SYSBUS_CSI_SHIFT                                     12
#define PERI_CRG_CLKGT0_RO_GT_CLK_SYSBUS_CSI_MASK                                      0x00001000
#define PERI_CRG_CLKGT0_RO_GT_CLK_CFGBUS_CSI_SHIFT                                     13
#define PERI_CRG_CLKGT0_RO_GT_CLK_CFGBUS_CSI_MASK                                      0x00002000
#define PERI_CRG_CLKGT0_RO_GT_CLK_CFGBUS_DSI_SHIFT                                     14
#define PERI_CRG_CLKGT0_RO_GT_CLK_CFGBUS_DSI_MASK                                      0x00004000
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_IPC1_SHIFT                                     15
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_IPC1_MASK                                      0x00008000
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_IPC2_SHIFT                                     16
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_IPC2_MASK                                      0x00010000
#define PERI_CRG_CLKGT0_RO_GT_PCLK_SPI4_SHIFT                                          17
#define PERI_CRG_CLKGT0_RO_GT_PCLK_SPI4_MASK                                           0x00020000
#define PERI_CRG_CLKGT0_RO_GT_PCLK_SPI5_SHIFT                                          18
#define PERI_CRG_CLKGT0_RO_GT_PCLK_SPI5_MASK                                           0x00040000
#define PERI_CRG_CLKGT0_RO_GT_PCLK_SPI6_SHIFT                                          19
#define PERI_CRG_CLKGT0_RO_GT_PCLK_SPI6_MASK                                           0x00080000
#define PERI_CRG_CLKGT0_RO_GT_CLK_SPI4_SHIFT                                           20
#define PERI_CRG_CLKGT0_RO_GT_CLK_SPI4_MASK                                            0x00100000
#define PERI_CRG_CLKGT0_RO_GT_CLK_SPI5_SHIFT                                           21
#define PERI_CRG_CLKGT0_RO_GT_CLK_SPI5_MASK                                            0x00200000
#define PERI_CRG_CLKGT0_RO_GT_CLK_SPI6_SHIFT                                           22
#define PERI_CRG_CLKGT0_RO_GT_CLK_SPI6_MASK                                            0x00400000
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_WDT1_SHIFT                                     23
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_WDT1_MASK                                      0x00800000
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_WDT0_SHIFT                                     24
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERI_WDT0_MASK                                      0x01000000
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_WDT0_SHIFT                                      25
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_WDT0_MASK                                       0x02000000
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_WDT1_SHIFT                                      26
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERI_WDT1_MASK                                       0x04000000
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERFMON_SHIFT                                        27
#define PERI_CRG_CLKGT0_RO_GT_CLK_PERFMON_MASK                                         0x08000000
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERFMON_PERI_SHIFT                                  28
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERFMON_PERI_MASK                                   0x10000000
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERFMON_SHIFT                                       29
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERFMON_MASK                                        0x20000000
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERFSTAT_SHIFT                                      30
#define PERI_CRG_CLKGT0_RO_GT_PCLK_PERFSTAT_MASK                                       0x40000000
#define PERI_CRG_CLKST0                            0x000c
#define PERI_CRG_CLKST0_ST_PCLK_PERI_SPINLOCK_SHIFT                                    0
#define PERI_CRG_CLKST0_ST_PCLK_PERI_SPINLOCK_MASK                                     0x00000001
#define PERI_CRG_CLKST0_ST_CLK_OCM_SHIFT                                               1
#define PERI_CRG_CLKST0_ST_CLK_OCM_MASK                                                0x00000002
#define PERI_CRG_CLKST0_ST_CLK_NPU_OCM_NIC_SHIFT                                       2
#define PERI_CRG_CLKST0_ST_CLK_NPU_OCM_NIC_MASK                                        0x00000004
#define PERI_CRG_CLKST0_ST_CLK_PCIE_AUX_SHIFT                                          3
#define PERI_CRG_CLKST0_ST_CLK_PCIE_AUX_MASK                                           0x00000008
#define PERI_CRG_CLKST0_ST_CLK_NPU_OCM_ADB_MST_SHIFT                                   4
#define PERI_CRG_CLKST0_ST_CLK_NPU_OCM_ADB_MST_MASK                                    0x00000010
#define PERI_CRG_CLKST0_ST_PCLK_PERI_TIMER_NS_SHIFT                                    5
#define PERI_CRG_CLKST0_ST_PCLK_PERI_TIMER_NS_MASK                                     0x00000020
#define PERI_CRG_CLKST0_ST_CLK_PERI_TIMER0_SHIFT                                       6
#define PERI_CRG_CLKST0_ST_CLK_PERI_TIMER0_MASK                                        0x00000040
#define PERI_CRG_CLKST0_ST_CLK_PERI_TIMER1_SHIFT                                       7
#define PERI_CRG_CLKST0_ST_CLK_PERI_TIMER1_MASK                                        0x00000080
#define PERI_CRG_CLKST0_ST_CLK_PERI_TIMER2_SHIFT                                       8
#define PERI_CRG_CLKST0_ST_CLK_PERI_TIMER2_MASK                                        0x00000100
#define PERI_CRG_CLKST0_ST_CLK_PERI_TIMER3_SHIFT                                       9
#define PERI_CRG_CLKST0_ST_CLK_PERI_TIMER3_MASK                                        0x00000200
#define PERI_CRG_CLKST0_ST_CLK_CSI_SYS_SHIFT                                           10
#define PERI_CRG_CLKST0_ST_CLK_CSI_SYS_MASK                                            0x00000400
#define PERI_CRG_CLKST0_ST_CLK_DSI_SYS_SHIFT                                           11
#define PERI_CRG_CLKST0_ST_CLK_DSI_SYS_MASK                                            0x00000800
#define PERI_CRG_CLKST0_ST_CLK_SYSBUS_CSI_SHIFT                                        12
#define PERI_CRG_CLKST0_ST_CLK_SYSBUS_CSI_MASK                                         0x00001000
#define PERI_CRG_CLKST0_ST_CLK_CFGBUS_CSI_SHIFT                                        13
#define PERI_CRG_CLKST0_ST_CLK_CFGBUS_CSI_MASK                                         0x00002000
#define PERI_CRG_CLKST0_ST_CLK_CFGBUS_DSI_SHIFT                                        14
#define PERI_CRG_CLKST0_ST_CLK_CFGBUS_DSI_MASK                                         0x00004000
#define PERI_CRG_CLKST0_ST_PCLK_PERI_IPC1_SHIFT                                        15
#define PERI_CRG_CLKST0_ST_PCLK_PERI_IPC1_MASK                                         0x00008000
#define PERI_CRG_CLKST0_ST_PCLK_PERI_IPC2_SHIFT                                        16
#define PERI_CRG_CLKST0_ST_PCLK_PERI_IPC2_MASK                                         0x00010000
#define PERI_CRG_CLKST0_ST_PCLK_SPI4_SHIFT                                             17
#define PERI_CRG_CLKST0_ST_PCLK_SPI4_MASK                                              0x00020000
#define PERI_CRG_CLKST0_ST_PCLK_SPI5_SHIFT                                             18
#define PERI_CRG_CLKST0_ST_PCLK_SPI5_MASK                                              0x00040000
#define PERI_CRG_CLKST0_ST_PCLK_SPI6_SHIFT                                             19
#define PERI_CRG_CLKST0_ST_PCLK_SPI6_MASK                                              0x00080000
#define PERI_CRG_CLKST0_ST_CLK_SPI4_SHIFT                                              20
#define PERI_CRG_CLKST0_ST_CLK_SPI4_MASK                                               0x00100000
#define PERI_CRG_CLKST0_ST_CLK_SPI5_SHIFT                                              21
#define PERI_CRG_CLKST0_ST_CLK_SPI5_MASK                                               0x00200000
#define PERI_CRG_CLKST0_ST_CLK_SPI6_SHIFT                                              22
#define PERI_CRG_CLKST0_ST_CLK_SPI6_MASK                                               0x00400000
#define PERI_CRG_CLKST0_ST_PCLK_PERI_WDT1_SHIFT                                        23
#define PERI_CRG_CLKST0_ST_PCLK_PERI_WDT1_MASK                                         0x00800000
#define PERI_CRG_CLKST0_ST_PCLK_PERI_WDT0_SHIFT                                        24
#define PERI_CRG_CLKST0_ST_PCLK_PERI_WDT0_MASK                                         0x01000000
#define PERI_CRG_CLKST0_ST_CLK_PERI_WDT0_SHIFT                                         25
#define PERI_CRG_CLKST0_ST_CLK_PERI_WDT0_MASK                                          0x02000000
#define PERI_CRG_CLKST0_ST_CLK_PERI_WDT1_SHIFT                                         26
#define PERI_CRG_CLKST0_ST_CLK_PERI_WDT1_MASK                                          0x04000000
#define PERI_CRG_CLKST0_ST_CLK_PERFMON_SHIFT                                           27
#define PERI_CRG_CLKST0_ST_CLK_PERFMON_MASK                                            0x08000000
#define PERI_CRG_CLKST0_ST_PCLK_PERFMON_PERI_SHIFT                                     28
#define PERI_CRG_CLKST0_ST_PCLK_PERFMON_PERI_MASK                                      0x10000000
#define PERI_CRG_CLKST0_ST_PCLK_PERFMON_SHIFT                                          29
#define PERI_CRG_CLKST0_ST_PCLK_PERFMON_MASK                                           0x20000000
#define PERI_CRG_CLKST0_ST_PCLK_PERFSTAT_SHIFT                                         30
#define PERI_CRG_CLKST0_ST_PCLK_PERFSTAT_MASK                                          0x40000000
#define PERI_CRG_CLKGT1_W1S                        0x0010
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C0_SHIFT                                         0
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C1_SHIFT                                         1
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C2_SHIFT                                         2
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C3_SHIFT                                         3
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C4_SHIFT                                         4
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C5_SHIFT                                         5
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C6_SHIFT                                         6
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C9_SHIFT                                         9
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C10_SHIFT                                        10
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C11_SHIFT                                        11
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C12_SHIFT                                        12
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C13_SHIFT                                        13
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C20_SHIFT                                        14
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_I3C0_SHIFT                                         15
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_PWM0_ABRG_SHIFT                                    16
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_PWM1_ABRG_SHIFT                                    17
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_UART3_ABRG_SHIFT                                   18
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_UART6_ABRG_SHIFT                                   19
#define PERI_CRG_CLKGT1_W1S_GT_CLK_UART3_SHIFT                                         20
#define PERI_CRG_CLKGT1_W1S_GT_CLK_UART6_SHIFT                                         21
#define PERI_CRG_CLKGT1_W1S_GT_CLK_DMA_NS_SHIFT                                        22
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_DMA_NS_SHIFT                                       23
#define PERI_CRG_CLKGT1_W1S_GT_CLK_DMA_S_SHIFT                                         24
#define PERI_CRG_CLKGT1_W1S_GT_PCLK_DMA_S_SHIFT                                        25
#define PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_DSI_RS_CORE_SHIFT                            26
#define PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_CORE_SHIFT                            27
#define PERI_CRG_CLKGT1_W1S_GT_CLK_CFGBUS_CSI_RS_PERI_SHIFT                            28
#define PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_CORE_SHIFT                            29
#define PERI_CRG_CLKGT1_W1S_GT_CLK_SYSBUS_CSI_RS_PERI_SHIFT                            30
#define PERI_CRG_CLKGT1_W1C                        0x0014
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C0_SHIFT                                         0
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C1_SHIFT                                         1
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C2_SHIFT                                         2
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C3_SHIFT                                         3
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C4_SHIFT                                         4
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C5_SHIFT                                         5
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C6_SHIFT                                         6
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C9_SHIFT                                         9
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C10_SHIFT                                        10
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C11_SHIFT                                        11
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C12_SHIFT                                        12
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C13_SHIFT                                        13
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I2C20_SHIFT                                        14
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_I3C0_SHIFT                                         15
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_PWM0_ABRG_SHIFT                                    16
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_PWM1_ABRG_SHIFT                                    17
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_UART3_ABRG_SHIFT                                   18
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_UART6_ABRG_SHIFT                                   19
#define PERI_CRG_CLKGT1_W1C_GT_CLK_UART3_SHIFT                                         20
#define PERI_CRG_CLKGT1_W1C_GT_CLK_UART6_SHIFT                                         21
#define PERI_CRG_CLKGT1_W1C_GT_CLK_DMA_NS_SHIFT                                        22
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_DMA_NS_SHIFT                                       23
#define PERI_CRG_CLKGT1_W1C_GT_CLK_DMA_S_SHIFT                                         24
#define PERI_CRG_CLKGT1_W1C_GT_PCLK_DMA_S_SHIFT                                        25
#define PERI_CRG_CLKGT1_W1C_GT_CLK_CFGBUS_DSI_RS_CORE_SHIFT                            26
#define PERI_CRG_CLKGT1_W1C_GT_CLK_CFGBUS_CSI_RS_CORE_SHIFT                            27
#define PERI_CRG_CLKGT1_W1C_GT_CLK_CFGBUS_CSI_RS_PERI_SHIFT                            28
#define PERI_CRG_CLKGT1_W1C_GT_CLK_SYSBUS_CSI_RS_CORE_SHIFT                            29
#define PERI_CRG_CLKGT1_W1C_GT_CLK_SYSBUS_CSI_RS_PERI_SHIFT                            30
#define PERI_CRG_CLKGT1_RO                         0x0018
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C0_SHIFT                                          0
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C0_MASK                                           0x00000001
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C1_SHIFT                                          1
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C1_MASK                                           0x00000002
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C2_SHIFT                                          2
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C2_MASK                                           0x00000004
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C3_SHIFT                                          3
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C3_MASK                                           0x00000008
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C4_SHIFT                                          4
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C4_MASK                                           0x00000010
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C5_SHIFT                                          5
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C5_MASK                                           0x00000020
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C6_SHIFT                                          6
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C6_MASK                                           0x00000040
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C9_SHIFT                                          9
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C9_MASK                                           0x00000200
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C10_SHIFT                                         10
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C10_MASK                                          0x00000400
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C11_SHIFT                                         11
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C11_MASK                                          0x00000800
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C12_SHIFT                                         12
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C12_MASK                                          0x00001000
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C13_SHIFT                                         13
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C13_MASK                                          0x00002000
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C20_SHIFT                                         14
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I2C20_MASK                                          0x00004000
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I3C0_SHIFT                                          15
#define PERI_CRG_CLKGT1_RO_GT_PCLK_I3C0_MASK                                           0x00008000
#define PERI_CRG_CLKGT1_RO_GT_PCLK_PWM0_ABRG_SHIFT                                     16
#define PERI_CRG_CLKGT1_RO_GT_PCLK_PWM0_ABRG_MASK                                      0x00010000
#define PERI_CRG_CLKGT1_RO_GT_PCLK_PWM1_ABRG_SHIFT                                     17
#define PERI_CRG_CLKGT1_RO_GT_PCLK_PWM1_ABRG_MASK                                      0x00020000
#define PERI_CRG_CLKGT1_RO_GT_PCLK_UART3_ABRG_SHIFT                                    18
#define PERI_CRG_CLKGT1_RO_GT_PCLK_UART3_ABRG_MASK                                     0x00040000
#define PERI_CRG_CLKGT1_RO_GT_PCLK_UART6_ABRG_SHIFT                                    19
#define PERI_CRG_CLKGT1_RO_GT_PCLK_UART6_ABRG_MASK                                     0x00080000
#define PERI_CRG_CLKGT1_RO_GT_CLK_UART3_SHIFT                                          20
#define PERI_CRG_CLKGT1_RO_GT_CLK_UART3_MASK                                           0x00100000
#define PERI_CRG_CLKGT1_RO_GT_CLK_UART6_SHIFT                                          21
#define PERI_CRG_CLKGT1_RO_GT_CLK_UART6_MASK                                           0x00200000
#define PERI_CRG_CLKGT1_RO_GT_CLK_DMA_NS_SHIFT                                         22
#define PERI_CRG_CLKGT1_RO_GT_CLK_DMA_NS_MASK                                          0x00400000
#define PERI_CRG_CLKGT1_RO_GT_PCLK_DMA_NS_SHIFT                                        23
#define PERI_CRG_CLKGT1_RO_GT_PCLK_DMA_NS_MASK                                         0x00800000
#define PERI_CRG_CLKGT1_RO_GT_CLK_DMA_S_SHIFT                                          24
#define PERI_CRG_CLKGT1_RO_GT_CLK_DMA_S_MASK                                           0x01000000
#define PERI_CRG_CLKGT1_RO_GT_PCLK_DMA_S_SHIFT                                         25
#define PERI_CRG_CLKGT1_RO_GT_PCLK_DMA_S_MASK                                          0x02000000
#define PERI_CRG_CLKGT1_RO_GT_CLK_CFGBUS_DSI_RS_CORE_SHIFT                             26
#define PERI_CRG_CLKGT1_RO_GT_CLK_CFGBUS_DSI_RS_CORE_MASK                              0x04000000
#define PERI_CRG_CLKGT1_RO_GT_CLK_CFGBUS_CSI_RS_CORE_SHIFT                             27
#define PERI_CRG_CLKGT1_RO_GT_CLK_CFGBUS_CSI_RS_CORE_MASK                              0x08000000
#define PERI_CRG_CLKGT1_RO_GT_CLK_CFGBUS_CSI_RS_PERI_SHIFT                             28
#define PERI_CRG_CLKGT1_RO_GT_CLK_CFGBUS_CSI_RS_PERI_MASK                              0x10000000
#define PERI_CRG_CLKGT1_RO_GT_CLK_SYSBUS_CSI_RS_CORE_SHIFT                             29
#define PERI_CRG_CLKGT1_RO_GT_CLK_SYSBUS_CSI_RS_CORE_MASK                              0x20000000
#define PERI_CRG_CLKGT1_RO_GT_CLK_SYSBUS_CSI_RS_PERI_SHIFT                             30
#define PERI_CRG_CLKGT1_RO_GT_CLK_SYSBUS_CSI_RS_PERI_MASK                              0x40000000
#define PERI_CRG_CLKST1                            0x001c
#define PERI_CRG_CLKST1_ST_PCLK_I2C0_SHIFT                                             0
#define PERI_CRG_CLKST1_ST_PCLK_I2C0_MASK                                              0x00000001
#define PERI_CRG_CLKST1_ST_PCLK_I2C1_SHIFT                                             1
#define PERI_CRG_CLKST1_ST_PCLK_I2C1_MASK                                              0x00000002
#define PERI_CRG_CLKST1_ST_PCLK_I2C2_SHIFT                                             2
#define PERI_CRG_CLKST1_ST_PCLK_I2C2_MASK                                              0x00000004
#define PERI_CRG_CLKST1_ST_PCLK_I2C3_SHIFT                                             3
#define PERI_CRG_CLKST1_ST_PCLK_I2C3_MASK                                              0x00000008
#define PERI_CRG_CLKST1_ST_PCLK_I2C4_SHIFT                                             4
#define PERI_CRG_CLKST1_ST_PCLK_I2C4_MASK                                              0x00000010
#define PERI_CRG_CLKST1_ST_PCLK_I2C5_SHIFT                                             5
#define PERI_CRG_CLKST1_ST_PCLK_I2C5_MASK                                              0x00000020
#define PERI_CRG_CLKST1_ST_PCLK_I2C6_SHIFT                                             6
#define PERI_CRG_CLKST1_ST_PCLK_I2C6_MASK                                              0x00000040
#define PERI_CRG_CLKST1_ST_PCLK_I2C9_SHIFT                                             9
#define PERI_CRG_CLKST1_ST_PCLK_I2C9_MASK                                              0x00000200
#define PERI_CRG_CLKST1_ST_PCLK_I2C10_SHIFT                                            10
#define PERI_CRG_CLKST1_ST_PCLK_I2C10_MASK                                             0x00000400
#define PERI_CRG_CLKST1_ST_PCLK_I2C11_SHIFT                                            11
#define PERI_CRG_CLKST1_ST_PCLK_I2C11_MASK                                             0x00000800
#define PERI_CRG_CLKST1_ST_PCLK_I2C12_SHIFT                                            12
#define PERI_CRG_CLKST1_ST_PCLK_I2C12_MASK                                             0x00001000
#define PERI_CRG_CLKST1_ST_PCLK_I2C13_SHIFT                                            13
#define PERI_CRG_CLKST1_ST_PCLK_I2C13_MASK                                             0x00002000
#define PERI_CRG_CLKST1_ST_PCLK_I2C20_SHIFT                                            14
#define PERI_CRG_CLKST1_ST_PCLK_I2C20_MASK                                             0x00004000
#define PERI_CRG_CLKST1_ST_PCLK_I3C0_SHIFT                                             15
#define PERI_CRG_CLKST1_ST_PCLK_I3C0_MASK                                              0x00008000
#define PERI_CRG_CLKST1_ST_PCLK_PWM0_ABRG_SHIFT                                        16
#define PERI_CRG_CLKST1_ST_PCLK_PWM0_ABRG_MASK                                         0x00010000
#define PERI_CRG_CLKST1_ST_PCLK_PWM1_ABRG_SHIFT                                        17
#define PERI_CRG_CLKST1_ST_PCLK_PWM1_ABRG_MASK                                         0x00020000
#define PERI_CRG_CLKST1_ST_PCLK_UART3_ABRG_SHIFT                                       18
#define PERI_CRG_CLKST1_ST_PCLK_UART3_ABRG_MASK                                        0x00040000
#define PERI_CRG_CLKST1_ST_PCLK_UART6_ABRG_SHIFT                                       19
#define PERI_CRG_CLKST1_ST_PCLK_UART6_ABRG_MASK                                        0x00080000
#define PERI_CRG_CLKST1_ST_CLK_UART3_SHIFT                                             20
#define PERI_CRG_CLKST1_ST_CLK_UART3_MASK                                              0x00100000
#define PERI_CRG_CLKST1_ST_CLK_UART6_SHIFT                                             21
#define PERI_CRG_CLKST1_ST_CLK_UART6_MASK                                              0x00200000
#define PERI_CRG_CLKST1_ST_CLK_DMA_NS_SHIFT                                            22
#define PERI_CRG_CLKST1_ST_CLK_DMA_NS_MASK                                             0x00400000
#define PERI_CRG_CLKST1_ST_PCLK_DMA_NS_SHIFT                                           23
#define PERI_CRG_CLKST1_ST_PCLK_DMA_NS_MASK                                            0x00800000
#define PERI_CRG_CLKST1_ST_CLK_DMA_S_SHIFT                                             24
#define PERI_CRG_CLKST1_ST_CLK_DMA_S_MASK                                              0x01000000
#define PERI_CRG_CLKST1_ST_PCLK_DMA_S_SHIFT                                            25
#define PERI_CRG_CLKST1_ST_PCLK_DMA_S_MASK                                             0x02000000
#define PERI_CRG_CLKST1_ST_CLK_CFGBUS_CSI_RS_PERI_SHIFT                                28
#define PERI_CRG_CLKST1_ST_CLK_CFGBUS_CSI_RS_PERI_MASK                                 0x10000000
#define PERI_CRG_CLKST1_ST_CLK_SYSBUS_CSI_RS_PERI_SHIFT                                30
#define PERI_CRG_CLKST1_ST_CLK_SYSBUS_CSI_RS_PERI_MASK                                 0x40000000
#define PERI_CRG_CLKGT2_W1S                        0x0020
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C0_SHIFT                                          0
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C1_SHIFT                                          1
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C2_SHIFT                                          2
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C3_SHIFT                                          3
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C4_SHIFT                                          4
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C5_SHIFT                                          5
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C6_SHIFT                                          6
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C9_SHIFT                                          9
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C10_SHIFT                                         10
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C11_SHIFT                                         11
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C12_SHIFT                                         12
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C13_SHIFT                                         13
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I2C20_SHIFT                                         14
#define PERI_CRG_CLKGT2_W1S_GT_CLK_I3C0_SHIFT                                          15
#define PERI_CRG_CLKGT2_W1S_GT_CLK_PWM0_SHIFT                                          19
#define PERI_CRG_CLKGT2_W1S_GT_CLK_PWM1_SHIFT                                          20
#define PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL3_MEDIA1_SHIFT                                  21
#define PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL2_MEDIA1_SHIFT                                  22
#define PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL1_MEDIA1_SHIFT                                  23
#define PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL0_MEDIA1_SHIFT                                  24
#define PERI_CRG_CLKGT2_W1S_GT_CLK_GPLL_MEDIA1_SHIFT                                   25
#define PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL3_MEDIA2_SHIFT                                  26
#define PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL2_MEDIA2_SHIFT                                  27
#define PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL1_MEDIA2_SHIFT                                  28
#define PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL0_MEDIA2_SHIFT                                  29
#define PERI_CRG_CLKGT2_W1S_GT_CLK_GPLL_MEDIA2_SHIFT                                   30
#define PERI_CRG_CLKGT2_W1C                        0x0024
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C0_SHIFT                                          0
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C1_SHIFT                                          1
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C2_SHIFT                                          2
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C3_SHIFT                                          3
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C4_SHIFT                                          4
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C5_SHIFT                                          5
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C6_SHIFT                                          6
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C9_SHIFT                                          9
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C10_SHIFT                                         10
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C11_SHIFT                                         11
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C12_SHIFT                                         12
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C13_SHIFT                                         13
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I2C20_SHIFT                                         14
#define PERI_CRG_CLKGT2_W1C_GT_CLK_I3C0_SHIFT                                          15
#define PERI_CRG_CLKGT2_W1C_GT_CLK_PWM0_SHIFT                                          19
#define PERI_CRG_CLKGT2_W1C_GT_CLK_PWM1_SHIFT                                          20
#define PERI_CRG_CLKGT2_W1C_GT_CLK_PPLL3_MEDIA1_SHIFT                                  21
#define PERI_CRG_CLKGT2_W1C_GT_CLK_PPLL2_MEDIA1_SHIFT                                  22
#define PERI_CRG_CLKGT2_W1C_GT_CLK_PPLL1_MEDIA1_SHIFT                                  23
#define PERI_CRG_CLKGT2_W1C_GT_CLK_PPLL0_MEDIA1_SHIFT                                  24
#define PERI_CRG_CLKGT2_W1C_GT_CLK_GPLL_MEDIA1_SHIFT                                   25
#define PERI_CRG_CLKGT2_W1C_GT_CLK_PPLL3_MEDIA2_SHIFT                                  26
#define PERI_CRG_CLKGT2_W1C_GT_CLK_PPLL2_MEDIA2_SHIFT                                  27
#define PERI_CRG_CLKGT2_W1C_GT_CLK_PPLL1_MEDIA2_SHIFT                                  28
#define PERI_CRG_CLKGT2_W1C_GT_CLK_PPLL0_MEDIA2_SHIFT                                  29
#define PERI_CRG_CLKGT2_W1C_GT_CLK_GPLL_MEDIA2_SHIFT                                   30
#define PERI_CRG_CLKGT2_RO                         0x0028
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C0_SHIFT                                           0
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C0_MASK                                            0x00000001
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C1_SHIFT                                           1
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C1_MASK                                            0x00000002
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C2_SHIFT                                           2
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C2_MASK                                            0x00000004
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C3_SHIFT                                           3
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C3_MASK                                            0x00000008
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C4_SHIFT                                           4
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C4_MASK                                            0x00000010
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C5_SHIFT                                           5
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C5_MASK                                            0x00000020
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C6_SHIFT                                           6
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C6_MASK                                            0x00000040
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C9_SHIFT                                           9
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C9_MASK                                            0x00000200
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C10_SHIFT                                          10
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C10_MASK                                           0x00000400
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C11_SHIFT                                          11
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C11_MASK                                           0x00000800
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C12_SHIFT                                          12
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C12_MASK                                           0x00001000
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C13_SHIFT                                          13
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C13_MASK                                           0x00002000
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C20_SHIFT                                          14
#define PERI_CRG_CLKGT2_RO_GT_CLK_I2C20_MASK                                           0x00004000
#define PERI_CRG_CLKGT2_RO_GT_CLK_I3C0_SHIFT                                           15
#define PERI_CRG_CLKGT2_RO_GT_CLK_I3C0_MASK                                            0x00008000
#define PERI_CRG_CLKGT2_RO_GT_CLK_PWM0_SHIFT                                           19
#define PERI_CRG_CLKGT2_RO_GT_CLK_PWM0_MASK                                            0x00080000
#define PERI_CRG_CLKGT2_RO_GT_CLK_PWM1_SHIFT                                           20
#define PERI_CRG_CLKGT2_RO_GT_CLK_PWM1_MASK                                            0x00100000
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL3_MEDIA1_SHIFT                                   21
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL3_MEDIA1_MASK                                    0x00200000
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL2_MEDIA1_SHIFT                                   22
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL2_MEDIA1_MASK                                    0x00400000
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL1_MEDIA1_SHIFT                                   23
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL1_MEDIA1_MASK                                    0x00800000
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL0_MEDIA1_SHIFT                                   24
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL0_MEDIA1_MASK                                    0x01000000
#define PERI_CRG_CLKGT2_RO_GT_CLK_GPLL_MEDIA1_SHIFT                                    25
#define PERI_CRG_CLKGT2_RO_GT_CLK_GPLL_MEDIA1_MASK                                     0x02000000
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL3_MEDIA2_SHIFT                                   26
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL3_MEDIA2_MASK                                    0x04000000
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL2_MEDIA2_SHIFT                                   27
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL2_MEDIA2_MASK                                    0x08000000
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL1_MEDIA2_SHIFT                                   28
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL1_MEDIA2_MASK                                    0x10000000
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL0_MEDIA2_SHIFT                                   29
#define PERI_CRG_CLKGT2_RO_GT_CLK_PPLL0_MEDIA2_MASK                                    0x20000000
#define PERI_CRG_CLKGT2_RO_GT_CLK_GPLL_MEDIA2_SHIFT                                    30
#define PERI_CRG_CLKGT2_RO_GT_CLK_GPLL_MEDIA2_MASK                                     0x40000000
#define PERI_CRG_CLKST2                            0x002c
#define PERI_CRG_CLKST2_ST_CLK_I2C0_SHIFT                                              0
#define PERI_CRG_CLKST2_ST_CLK_I2C0_MASK                                               0x00000001
#define PERI_CRG_CLKST2_ST_CLK_I2C1_SHIFT                                              1
#define PERI_CRG_CLKST2_ST_CLK_I2C1_MASK                                               0x00000002
#define PERI_CRG_CLKST2_ST_CLK_I2C2_SHIFT                                              2
#define PERI_CRG_CLKST2_ST_CLK_I2C2_MASK                                               0x00000004
#define PERI_CRG_CLKST2_ST_CLK_I2C3_SHIFT                                              3
#define PERI_CRG_CLKST2_ST_CLK_I2C3_MASK                                               0x00000008
#define PERI_CRG_CLKST2_ST_CLK_I2C4_SHIFT                                              4
#define PERI_CRG_CLKST2_ST_CLK_I2C4_MASK                                               0x00000010
#define PERI_CRG_CLKST2_ST_CLK_I2C5_SHIFT                                              5
#define PERI_CRG_CLKST2_ST_CLK_I2C5_MASK                                               0x00000020
#define PERI_CRG_CLKST2_ST_CLK_I2C6_SHIFT                                              6
#define PERI_CRG_CLKST2_ST_CLK_I2C6_MASK                                               0x00000040
#define PERI_CRG_CLKST2_ST_CLK_I2C9_SHIFT                                              9
#define PERI_CRG_CLKST2_ST_CLK_I2C9_MASK                                               0x00000200
#define PERI_CRG_CLKST2_ST_CLK_I2C10_SHIFT                                             10
#define PERI_CRG_CLKST2_ST_CLK_I2C10_MASK                                              0x00000400
#define PERI_CRG_CLKST2_ST_CLK_I2C11_SHIFT                                             11
#define PERI_CRG_CLKST2_ST_CLK_I2C11_MASK                                              0x00000800
#define PERI_CRG_CLKST2_ST_CLK_I2C12_SHIFT                                             12
#define PERI_CRG_CLKST2_ST_CLK_I2C12_MASK                                              0x00001000
#define PERI_CRG_CLKST2_ST_CLK_I2C13_SHIFT                                             13
#define PERI_CRG_CLKST2_ST_CLK_I2C13_MASK                                              0x00002000
#define PERI_CRG_CLKST2_ST_CLK_I2C20_SHIFT                                             14
#define PERI_CRG_CLKST2_ST_CLK_I2C20_MASK                                              0x00004000
#define PERI_CRG_CLKST2_ST_CLK_I3C0_SHIFT                                              15
#define PERI_CRG_CLKST2_ST_CLK_I3C0_MASK                                               0x00008000
#define PERI_CRG_CLKST2_ST_CLK_PWM0_SHIFT                                              19
#define PERI_CRG_CLKST2_ST_CLK_PWM0_MASK                                               0x00080000
#define PERI_CRG_CLKST2_ST_CLK_PWM1_SHIFT                                              20
#define PERI_CRG_CLKST2_ST_CLK_PWM1_MASK                                               0x00100000
#define PERI_CRG_CLKST2_ST_CLK_PPLL3_MEDIA1_SHIFT                                      21
#define PERI_CRG_CLKST2_ST_CLK_PPLL3_MEDIA1_MASK                                       0x00200000
#define PERI_CRG_CLKST2_ST_CLK_PPLL2_MEDIA1_SHIFT                                      22
#define PERI_CRG_CLKST2_ST_CLK_PPLL2_MEDIA1_MASK                                       0x00400000
#define PERI_CRG_CLKST2_ST_CLK_PPLL1_MEDIA1_SHIFT                                      23
#define PERI_CRG_CLKST2_ST_CLK_PPLL1_MEDIA1_MASK                                       0x00800000
#define PERI_CRG_CLKST2_ST_CLK_PPLL0_MEDIA1_SHIFT                                      24
#define PERI_CRG_CLKST2_ST_CLK_PPLL0_MEDIA1_MASK                                       0x01000000
#define PERI_CRG_CLKST2_ST_CLK_GPLL_MEDIA1_SHIFT                                       25
#define PERI_CRG_CLKST2_ST_CLK_GPLL_MEDIA1_MASK                                        0x02000000
#define PERI_CRG_CLKST2_ST_CLK_PPLL3_MEDIA2_SHIFT                                      26
#define PERI_CRG_CLKST2_ST_CLK_PPLL3_MEDIA2_MASK                                       0x04000000
#define PERI_CRG_CLKST2_ST_CLK_PPLL2_MEDIA2_SHIFT                                      27
#define PERI_CRG_CLKST2_ST_CLK_PPLL2_MEDIA2_MASK                                       0x08000000
#define PERI_CRG_CLKST2_ST_CLK_PPLL1_MEDIA2_SHIFT                                      28
#define PERI_CRG_CLKST2_ST_CLK_PPLL1_MEDIA2_MASK                                       0x10000000
#define PERI_CRG_CLKST2_ST_CLK_PPLL0_MEDIA2_SHIFT                                      29
#define PERI_CRG_CLKST2_ST_CLK_PPLL0_MEDIA2_MASK                                       0x20000000
#define PERI_CRG_CLKST2_ST_CLK_GPLL_MEDIA2_SHIFT                                       30
#define PERI_CRG_CLKST2_ST_CLK_GPLL_MEDIA2_MASK                                        0x40000000
#define PERI_CRG_CLKGT12_W1S                       0x0050
#define PERI_CRG_CLKGT12_W1S_GT_CLK_MAINBUS_DATA_DDR_W_RS_CORE_SHIFT                   0
#define PERI_CRG_CLKGT12_W1S_GT_CLK_MAINBUS_DATA_DDR_W_RS_PERI_SHIFT                   1
#define PERI_CRG_CLKGT12_W1S_GT_CLK_MAINBUS_DATA_DDR_R_RS_PERI_SHIFT                   2
#define PERI_CRG_CLKGT12_W1S_GT_CLK_SYSBUS_DATA_DDR_RS_CORE_SHIFT                      3
#define PERI_CRG_CLKGT12_W1S_GT_CLK_SYSBUS_DATA_DDR_RS_PERI_SHIFT                      4
#define PERI_CRG_CLKGT12_W1S_GT_CLK_DEBUG_ATB_RS_CORE_SHIFT                            5
#define PERI_CRG_CLKGT12_W1S_GT_CLK_DEBUG_ATB_RS_PERI_SHIFT                            6
#define PERI_CRG_CLKGT12_W1S_GT_CLK_CFGBUS_GPU_RS_PERI_SHIFT                           12
#define PERI_CRG_CLKGT12_W1S_GT_CLK_MAINBUS_DATA_GPU_RS_PERI_SHIFT                     13
#define PERI_CRG_CLKGT12_W1S_GT_CLK_SYSBUS_HSS1_RS_CORE_SHIFT                          14
#define PERI_CRG_CLKGT12_W1S_GT_CLK_SYSBUS_HSS1_RS_PERI_SHIFT                          15
#define PERI_CRG_CLKGT12_W1S_GT_CLK_CFGBUS_HSS1_RS_CORE_SHIFT                          16
#define PERI_CRG_CLKGT12_W1S_GT_CLK_SYSBUS_HSS2_RS_CORE_SHIFT                          17
#define PERI_CRG_CLKGT12_W1S_GT_CLK_SYSBUS_HSS2_RS_PERI_SHIFT                          18
#define PERI_CRG_CLKGT12_W1S_GT_CLK_CFGBUS_HSS2_RS_CORE_SHIFT                          19
#define PERI_CRG_CLKGT12_W1S_GT_CLK_CFGBUS_HSS2_RS_PERI_SHIFT                          20
#define PERI_CRG_CLKGT12_W1C                       0x0054
#define PERI_CRG_CLKGT12_W1C_GT_CLK_MAINBUS_DATA_DDR_W_RS_CORE_SHIFT                   0
#define PERI_CRG_CLKGT12_W1C_GT_CLK_MAINBUS_DATA_DDR_W_RS_PERI_SHIFT                   1
#define PERI_CRG_CLKGT12_W1C_GT_CLK_MAINBUS_DATA_DDR_R_RS_PERI_SHIFT                   2
#define PERI_CRG_CLKGT12_W1C_GT_CLK_SYSBUS_DATA_DDR_RS_CORE_SHIFT                      3
#define PERI_CRG_CLKGT12_W1C_GT_CLK_SYSBUS_DATA_DDR_RS_PERI_SHIFT                      4
#define PERI_CRG_CLKGT12_W1C_GT_CLK_DEBUG_ATB_RS_CORE_SHIFT                            5
#define PERI_CRG_CLKGT12_W1C_GT_CLK_DEBUG_ATB_RS_PERI_SHIFT                            6
#define PERI_CRG_CLKGT12_W1C_GT_CLK_CFGBUS_GPU_RS_PERI_SHIFT                           12
#define PERI_CRG_CLKGT12_W1C_GT_CLK_MAINBUS_DATA_GPU_RS_PERI_SHIFT                     13
#define PERI_CRG_CLKGT12_W1C_GT_CLK_SYSBUS_HSS1_RS_CORE_SHIFT                          14
#define PERI_CRG_CLKGT12_W1C_GT_CLK_SYSBUS_HSS1_RS_PERI_SHIFT                          15
#define PERI_CRG_CLKGT12_W1C_GT_CLK_CFGBUS_HSS1_RS_CORE_SHIFT                          16
#define PERI_CRG_CLKGT12_W1C_GT_CLK_SYSBUS_HSS2_RS_CORE_SHIFT                          17
#define PERI_CRG_CLKGT12_W1C_GT_CLK_SYSBUS_HSS2_RS_PERI_SHIFT                          18
#define PERI_CRG_CLKGT12_W1C_GT_CLK_CFGBUS_HSS2_RS_CORE_SHIFT                          19
#define PERI_CRG_CLKGT12_W1C_GT_CLK_CFGBUS_HSS2_RS_PERI_SHIFT                          20
#define PERI_CRG_CLKGT12_RO                        0x0058
#define PERI_CRG_CLKGT12_RO_GT_CLK_MAINBUS_DATA_DDR_W_RS_CORE_SHIFT                    0
#define PERI_CRG_CLKGT12_RO_GT_CLK_MAINBUS_DATA_DDR_W_RS_CORE_MASK                     0x00000001
#define PERI_CRG_CLKGT12_RO_GT_CLK_MAINBUS_DATA_DDR_W_RS_PERI_SHIFT                    1
#define PERI_CRG_CLKGT12_RO_GT_CLK_MAINBUS_DATA_DDR_W_RS_PERI_MASK                     0x00000002
#define PERI_CRG_CLKGT12_RO_GT_CLK_MAINBUS_DATA_DDR_R_RS_PERI_SHIFT                    2
#define PERI_CRG_CLKGT12_RO_GT_CLK_MAINBUS_DATA_DDR_R_RS_PERI_MASK                     0x00000004
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_DATA_DDR_RS_CORE_SHIFT                       3
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_DATA_DDR_RS_CORE_MASK                        0x00000008
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_DATA_DDR_RS_PERI_SHIFT                       4
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_DATA_DDR_RS_PERI_MASK                        0x00000010
#define PERI_CRG_CLKGT12_RO_GT_CLK_DEBUG_ATB_RS_CORE_SHIFT                             5
#define PERI_CRG_CLKGT12_RO_GT_CLK_DEBUG_ATB_RS_CORE_MASK                              0x00000020
#define PERI_CRG_CLKGT12_RO_GT_CLK_DEBUG_ATB_RS_PERI_SHIFT                             6
#define PERI_CRG_CLKGT12_RO_GT_CLK_DEBUG_ATB_RS_PERI_MASK                              0x00000040
#define PERI_CRG_CLKGT12_RO_GT_CLK_CFGBUS_GPU_RS_PERI_SHIFT                            12
#define PERI_CRG_CLKGT12_RO_GT_CLK_CFGBUS_GPU_RS_PERI_MASK                             0x00001000
#define PERI_CRG_CLKGT12_RO_GT_CLK_MAINBUS_DATA_GPU_RS_PERI_SHIFT                      13
#define PERI_CRG_CLKGT12_RO_GT_CLK_MAINBUS_DATA_GPU_RS_PERI_MASK                       0x00002000
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_HSS1_RS_CORE_SHIFT                           14
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_HSS1_RS_CORE_MASK                            0x00004000
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_HSS1_RS_PERI_SHIFT                           15
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_HSS1_RS_PERI_MASK                            0x00008000
#define PERI_CRG_CLKGT12_RO_GT_CLK_CFGBUS_HSS1_RS_CORE_SHIFT                           16
#define PERI_CRG_CLKGT12_RO_GT_CLK_CFGBUS_HSS1_RS_CORE_MASK                            0x00010000
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_HSS2_RS_CORE_SHIFT                           17
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_HSS2_RS_CORE_MASK                            0x00020000
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_HSS2_RS_PERI_SHIFT                           18
#define PERI_CRG_CLKGT12_RO_GT_CLK_SYSBUS_HSS2_RS_PERI_MASK                            0x00040000
#define PERI_CRG_CLKGT12_RO_GT_CLK_CFGBUS_HSS2_RS_CORE_SHIFT                           19
#define PERI_CRG_CLKGT12_RO_GT_CLK_CFGBUS_HSS2_RS_CORE_MASK                            0x00080000
#define PERI_CRG_CLKGT12_RO_GT_CLK_CFGBUS_HSS2_RS_PERI_SHIFT                           20
#define PERI_CRG_CLKGT12_RO_GT_CLK_CFGBUS_HSS2_RS_PERI_MASK                            0x00100000
#define PERI_CRG_CLKST12                           0x005c
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRA_W_RS_PERI_SHIFT                      0
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRA_W_RS_PERI_MASK                       0x00000001
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRA_R_RS_PERI_SHIFT                      1
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRA_R_RS_PERI_MASK                       0x00000002
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_DATA_DDRA_RS_PERI_SHIFT                         2
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_DATA_DDRA_RS_PERI_MASK                          0x00000004
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRB_W_RS_PERI_SHIFT                      3
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRB_W_RS_PERI_MASK                       0x00000008
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRB_R_RS_PERI_SHIFT                      4
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRB_R_RS_PERI_MASK                       0x00000010
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_DATA_DDRB_RS_PERI_SHIFT                         5
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_DATA_DDRB_RS_PERI_MASK                          0x00000020
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRC_W_RS_PERI_SHIFT                      6
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRC_W_RS_PERI_MASK                       0x00000040
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRC_R_RS_PERI_SHIFT                      7
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRC_R_RS_PERI_MASK                       0x00000080
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_DATA_DDRC_RS_PERI_SHIFT                         8
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_DATA_DDRC_RS_PERI_MASK                          0x00000100
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRD_W_RS_PERI_SHIFT                      9
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRD_W_RS_PERI_MASK                       0x00000200
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRD_R_RS_PERI_SHIFT                      10
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_DDRD_R_RS_PERI_MASK                       0x00000400
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_DATA_DDRD_RS_PERI_SHIFT                         11
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_DATA_DDRD_RS_PERI_MASK                          0x00000800
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_MREG2GPU_RS_PERI_SHIFT                          14
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_MREG2GPU_RS_PERI_MASK                           0x00004000
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_GPU2MAINCFG_RS_PERI_SHIFT                       15
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_GPU2MAINCFG_RS_PERI_MASK                        0x00008000
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_MAINCFG2GPU_RS_PERI_SHIFT                       16
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_MAINCFG2GPU_RS_PERI_MASK                        0x00010000
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_CFGBUS2GPU_AXI_RS_PERI_SHIFT                    17
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_CFGBUS2GPU_AXI_RS_PERI_MASK                     0x00020000
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_CFGBUS2GPU_RS_PERI_SHIFT                        18
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_CFGBUS2GPU_RS_PERI_MASK                         0x00040000
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_MAIN2GPU_RS0_PERI_SHIFT                   19
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_MAIN2GPU_RS0_PERI_MASK                    0x00080000
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_GPU2MAIN_RS0_PERI_SHIFT                   20
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_GPU2MAIN_RS0_PERI_MASK                    0x00100000
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_HSS1_RS_PERI_SHIFT                              21
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_HSS1_RS_PERI_MASK                               0x00200000
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_HSS22SYSBUS_RS_PERI_SHIFT                       22
#define PERI_CRG_CLKST12_ST_CLK_SYSBUS_HSS22SYSBUS_RS_PERI_MASK                        0x00400000
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_HSS22CFGBUS_RS_PERI_SHIFT                       23
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_HSS22CFGBUS_RS_PERI_MASK                        0x00800000
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_CFGBUS2HSS2_RS_PERI_SHIFT                       24
#define PERI_CRG_CLKST12_ST_CLK_CFGBUS_CFGBUS2HSS2_RS_PERI_MASK                        0x01000000
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_MAIN2GPU_RS1_PERI_SHIFT                   25
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_MAIN2GPU_RS1_PERI_MASK                    0x02000000
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_GPU2MAIN_RS1_PERI_SHIFT                   26
#define PERI_CRG_CLKST12_ST_CLK_MAINBUS_DATA_GPU2MAIN_RS1_PERI_MASK                    0x04000000
#define PERI_CRG_CLKGT13_W1S                       0x0060
#define PERI_CRG_CLKGT13_W1S_GT_CLK_MAINBUS_DATA_MEDIA1_RS_CORE_SHIFT                  0
#define PERI_CRG_CLKGT13_W1S_GT_CLK_MAINBUS_DATA_MEDIA1_RS_PERI_SHIFT                  1
#define PERI_CRG_CLKGT13_W1S_GT_CLK_CFGBUS_MEDIA1_RS_CORE_SHIFT                        2
#define PERI_CRG_CLKGT13_W1S_GT_CLK_CFGBUS_MEDIA1_RS_PERI_SHIFT                        3
#define PERI_CRG_CLKGT13_W1S_GT_CLK_SYSBUS_DATA_MEDIA1_RS_CORE_SHIFT                   4
#define PERI_CRG_CLKGT13_W1S_GT_CLK_MAINBUS_DATA_MEDIA2_RS_CORE_SHIFT                  5
#define PERI_CRG_CLKGT13_W1S_GT_CLK_MAINBUS_DATA_MEDIA2_RS_PERI_SHIFT                  6
#define PERI_CRG_CLKGT13_W1S_GT_CLK_CFGBUS_MEDIA2_RS_CORE_SHIFT                        7
#define PERI_CRG_CLKGT13_W1S_GT_CLK_CFGBUS_MEDIA2_RS_PERI_SHIFT                        8
#define PERI_CRG_CLKGT13_W1S_GT_CLK_SYSBUS_DATA_MEDIA2_RS_CORE_SHIFT                   9
#define PERI_CRG_CLKGT13_W1S_GT_CLK_MAINBUS_DATA_NPU_RS_CORE_SHIFT                     10
#define PERI_CRG_CLKGT13_W1S_GT_CLK_MAINBUS_DATA_NPU_RS_PERI_SHIFT                     11
#define PERI_CRG_CLKGT13_W1S_GT_CLK_CFGBUS_NPU_RS_CORE_SHIFT                           12
#define PERI_CRG_CLKGT13_W1S_GT_CLK_CFGBUS_NPU_RS_PERI_SHIFT                           13
#define PERI_CRG_CLKGT13_W1S_GT_CLK_SYSBUS_XRSE_RS_PERI_SHIFT                          14
#define PERI_CRG_CLKGT13_W1S_GT_CLK_CFGBUS_XRSE_RS_PERI_SHIFT                          15
#define PERI_CRG_CLKGT13_W1S_GT_CLK_MAINBUS_DATA_CPU_RS_CORE_SHIFT                     16
#define PERI_CRG_CLKGT13_W1S_GT_CLK_MAINBUS_DATA_CPU_RS_PERI_SHIFT                     17
#define PERI_CRG_CLKGT13_W1S_GT_CLK_CFGBUS_CPU_RS_CORE_SHIFT                           18
#define PERI_CRG_CLKGT13_W1S_GT_CLK_CFGBUS_CPU_RS_PERI_SHIFT                           19
#define PERI_CRG_CLKGT13_W1S_GT_CLK_SYSBUS_CPU_RS_CORE_SHIFT                           20
#define PERI_CRG_CLKGT13_W1S_GT_CLK_SYSBUS_CPU_RS_PERI_SHIFT                           21
#define PERI_CRG_CLKGT13_W1C                       0x0064
#define PERI_CRG_CLKGT13_W1C_GT_CLK_MAINBUS_DATA_MEDIA1_RS_CORE_SHIFT                  0
#define PERI_CRG_CLKGT13_W1C_GT_CLK_MAINBUS_DATA_MEDIA1_RS_PERI_SHIFT                  1
#define PERI_CRG_CLKGT13_W1C_GT_CLK_CFGBUS_MEDIA1_RS_CORE_SHIFT                        2
#define PERI_CRG_CLKGT13_W1C_GT_CLK_CFGBUS_MEDIA1_RS_PERI_SHIFT                        3
#define PERI_CRG_CLKGT13_W1C_GT_CLK_SYSBUS_DATA_MEDIA1_RS_CORE_SHIFT                   4
#define PERI_CRG_CLKGT13_W1C_GT_CLK_MAINBUS_DATA_MEDIA2_RS_CORE_SHIFT                  5
#define PERI_CRG_CLKGT13_W1C_GT_CLK_MAINBUS_DATA_MEDIA2_RS_PERI_SHIFT                  6
#define PERI_CRG_CLKGT13_W1C_GT_CLK_CFGBUS_MEDIA2_RS_CORE_SHIFT                        7
#define PERI_CRG_CLKGT13_W1C_GT_CLK_CFGBUS_MEDIA2_RS_PERI_SHIFT                        8
#define PERI_CRG_CLKGT13_W1C_GT_CLK_SYSBUS_DATA_MEDIA2_RS_CORE_SHIFT                   9
#define PERI_CRG_CLKGT13_W1C_GT_CLK_MAINBUS_DATA_NPU_RS_CORE_SHIFT                     10
#define PERI_CRG_CLKGT13_W1C_GT_CLK_MAINBUS_DATA_NPU_RS_PERI_SHIFT                     11
#define PERI_CRG_CLKGT13_W1C_GT_CLK_CFGBUS_NPU_RS_CORE_SHIFT                           12
#define PERI_CRG_CLKGT13_W1C_GT_CLK_CFGBUS_NPU_RS_PERI_SHIFT                           13
#define PERI_CRG_CLKGT13_W1C_GT_CLK_SYSBUS_XRSE_RS_PERI_SHIFT                          14
#define PERI_CRG_CLKGT13_W1C_GT_CLK_CFGBUS_XRSE_RS_PERI_SHIFT                          15
#define PERI_CRG_CLKGT13_W1C_GT_CLK_MAINBUS_DATA_CPU_RS_CORE_SHIFT                     16
#define PERI_CRG_CLKGT13_W1C_GT_CLK_MAINBUS_DATA_CPU_RS_PERI_SHIFT                     17
#define PERI_CRG_CLKGT13_W1C_GT_CLK_CFGBUS_CPU_RS_CORE_SHIFT                           18
#define PERI_CRG_CLKGT13_W1C_GT_CLK_CFGBUS_CPU_RS_PERI_SHIFT                           19
#define PERI_CRG_CLKGT13_W1C_GT_CLK_SYSBUS_CPU_RS_CORE_SHIFT                           20
#define PERI_CRG_CLKGT13_W1C_GT_CLK_SYSBUS_CPU_RS_PERI_SHIFT                           21
#define PERI_CRG_CLKGT13_RO                        0x0068
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_MEDIA1_RS_CORE_SHIFT                   0
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_MEDIA1_RS_CORE_MASK                    0x00000001
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_MEDIA1_RS_PERI_SHIFT                   1
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_MEDIA1_RS_PERI_MASK                    0x00000002
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_MEDIA1_RS_CORE_SHIFT                         2
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_MEDIA1_RS_CORE_MASK                          0x00000004
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_MEDIA1_RS_PERI_SHIFT                         3
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_MEDIA1_RS_PERI_MASK                          0x00000008
#define PERI_CRG_CLKGT13_RO_GT_CLK_SYSBUS_DATA_MEDIA1_RS_CORE_SHIFT                    4
#define PERI_CRG_CLKGT13_RO_GT_CLK_SYSBUS_DATA_MEDIA1_RS_CORE_MASK                     0x00000010
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_MEDIA2_RS_CORE_SHIFT                   5
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_MEDIA2_RS_CORE_MASK                    0x00000020
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_MEDIA2_RS_PERI_SHIFT                   6
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_MEDIA2_RS_PERI_MASK                    0x00000040
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_MEDIA2_RS_CORE_SHIFT                         7
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_MEDIA2_RS_CORE_MASK                          0x00000080
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_MEDIA2_RS_PERI_SHIFT                         8
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_MEDIA2_RS_PERI_MASK                          0x00000100
#define PERI_CRG_CLKGT13_RO_GT_CLK_SYSBUS_DATA_MEDIA2_RS_CORE_SHIFT                    9
#define PERI_CRG_CLKGT13_RO_GT_CLK_SYSBUS_DATA_MEDIA2_RS_CORE_MASK                     0x00000200
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_NPU_RS_CORE_SHIFT                      10
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_NPU_RS_CORE_MASK                       0x00000400
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_NPU_RS_PERI_SHIFT                      11
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_NPU_RS_PERI_MASK                       0x00000800
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_NPU_RS_CORE_SHIFT                            12
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_NPU_RS_CORE_MASK                             0x00001000
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_NPU_RS_PERI_SHIFT                            13
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_NPU_RS_PERI_MASK                             0x00002000
#define PERI_CRG_CLKGT13_RO_GT_CLK_SYSBUS_XRSE_RS_PERI_SHIFT                           14
#define PERI_CRG_CLKGT13_RO_GT_CLK_SYSBUS_XRSE_RS_PERI_MASK                            0x00004000
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_XRSE_RS_PERI_SHIFT                           15
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_XRSE_RS_PERI_MASK                            0x00008000
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_CPU_RS_CORE_SHIFT                      16
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_CPU_RS_CORE_MASK                       0x00010000
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_CPU_RS_PERI_SHIFT                      17
#define PERI_CRG_CLKGT13_RO_GT_CLK_MAINBUS_DATA_CPU_RS_PERI_MASK                       0x00020000
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_CPU_RS_CORE_SHIFT                            18
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_CPU_RS_CORE_MASK                             0x00040000
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_CPU_RS_PERI_SHIFT                            19
#define PERI_CRG_CLKGT13_RO_GT_CLK_CFGBUS_CPU_RS_PERI_MASK                             0x00080000
#define PERI_CRG_CLKGT13_RO_GT_CLK_SYSBUS_CPU_RS_CORE_SHIFT                            20
#define PERI_CRG_CLKGT13_RO_GT_CLK_SYSBUS_CPU_RS_CORE_MASK                             0x00100000
#define PERI_CRG_CLKGT13_RO_GT_CLK_SYSBUS_CPU_RS_PERI_SHIFT                            21
#define PERI_CRG_CLKGT13_RO_GT_CLK_SYSBUS_CPU_RS_PERI_MASK                             0x00200000
#define PERI_CRG_CLKST13                           0x006c
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CFGBUS2MEDIA1_RS_PERI_SHIFT                     0
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CFGBUS2MEDIA1_RS_PERI_MASK                      0x00000001
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA1_P0_RS_PERI_SHIFT                   1
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA1_P0_RS_PERI_MASK                    0x00000002
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA1_P1_RS_PERI_SHIFT                   2
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA1_P1_RS_PERI_MASK                    0x00000004
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CFGBUS2MEDIA2_RS_PERI_SHIFT                     3
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CFGBUS2MEDIA2_RS_PERI_MASK                      0x00000008
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA2_P0_RS_PERI_SHIFT                   4
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA2_P0_RS_PERI_MASK                    0x00000010
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA2_P1_RS_PERI_SHIFT                   5
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA2_P1_RS_PERI_MASK                    0x00000020
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CFGBUS2MEDIA2_MFW_RS_PERI_SHIFT                 6
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CFGBUS2MEDIA2_MFW_RS_PERI_MASK                  0x00000040
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_NPU_MFW_RS_PERI_SHIFT                           7
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_NPU_MFW_RS_PERI_MASK                            0x00000080
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_P0_RS_PERI_SHIFT                      8
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_P0_RS_PERI_MASK                       0x00000100
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_NPU2CFGBUS_RS_PERI_SHIFT                        9
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_NPU2CFGBUS_RS_PERI_MASK                         0x00000200
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_XRSE_RS_PERI_SHIFT                              10
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_XRSE_RS_PERI_MASK                               0x00000400
#define PERI_CRG_CLKST13_ST_CLK_SYSBUS_XRSE_RS_PERI_SHIFT                              11
#define PERI_CRG_CLKST13_ST_CLK_SYSBUS_XRSE_RS_PERI_MASK                               0x00000800
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_XRSE_MFW_RS_PERI_SHIFT                          12
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_XRSE_MFW_RS_PERI_MASK                           0x00001000
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CPU_MFW_RS_PERI_SHIFT                           13
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CPU_MFW_RS_PERI_MASK                            0x00002000
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CPU2CFGBUS_RS_PERI_SHIFT                        14
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CPU2CFGBUS_RS_PERI_MASK                         0x00004000
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_MAINCFG2CPU_RS_PERI_SHIFT                       15
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_MAINCFG2CPU_RS_PERI_MASK                        0x00008000
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CPU2MAINCFG_RS_PERI_SHIFT                       16
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_CPU2MAINCFG_RS_PERI_MASK                        0x00010000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MAINBUS2CPU_RS_PERI_SHIFT                 17
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MAINBUS2CPU_RS_PERI_MASK                  0x00020000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_CPU2MAINBUS_RS_PERI_SHIFT                 18
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_CPU2MAINBUS_RS_PERI_MASK                  0x00040000
#define PERI_CRG_CLKST13_ST_CLK_SYSBUS_CPU_RS_PERI_SHIFT                               19
#define PERI_CRG_CLKST13_ST_CLK_SYSBUS_CPU_RS_PERI_MASK                                0x00080000
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_MREG2CPU_RS_PERI_SHIFT                          20
#define PERI_CRG_CLKST13_ST_CLK_CFGBUS_MREG2CPU_RS_PERI_MASK                           0x00100000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_P1_RS_PERI_SHIFT                      21
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_P1_RS_PERI_MASK                       0x00200000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_P2_RS_PERI_SHIFT                      22
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_P2_RS_PERI_MASK                       0x00400000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_P3_RS_PERI_SHIFT                      23
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_P3_RS_PERI_MASK                       0x00800000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA1_MREG_P0_RS_PERI_SHIFT              24
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA1_MREG_P0_RS_PERI_MASK               0x01000000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA1_MREG_P1_RS_PERI_SHIFT              25
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA1_MREG_P1_RS_PERI_MASK               0x02000000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA2_MREG_P0_RS_PERI_SHIFT              26
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA2_MREG_P0_RS_PERI_MASK               0x04000000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA2_MREG_P1_RS_PERI_SHIFT              27
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_MEDIA2_MREG_P1_RS_PERI_MASK               0x08000000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_MREG_P0_RS_PERI_SHIFT                 28
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_MREG_P0_RS_PERI_MASK                  0x10000000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_MREG_P1_RS_PERI_SHIFT                 29
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_MREG_P1_RS_PERI_MASK                  0x20000000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_MREG_P2_RS_PERI_SHIFT                 30
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_MREG_P2_RS_PERI_MASK                  0x40000000
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_MREG_P3_RS_PERI_SHIFT                 31
#define PERI_CRG_CLKST13_ST_CLK_MAINBUS_DATA_NPU_MREG_P3_RS_PERI_MASK                  0x80000000
#define PERI_CRG_CLKST14                           0x0070
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_DDRA_RS_PERI_SHIFT                           0
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_DDRA_RS_PERI_MASK                            0x00000001
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_DDRB_RS_PERI_SHIFT                           1
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_DDRB_RS_PERI_MASK                            0x00000002
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_DDRC_RS_PERI_SHIFT                           2
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_DDRC_RS_PERI_MASK                            0x00000004
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_DDRD_RS_PERI_SHIFT                           3
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_DDRD_RS_PERI_MASK                            0x00000008
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_GPU_RS_PERI_SHIFT                            4
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_GPU_RS_PERI_MASK                             0x00000010
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_MEDIA2_RS_PERI_SHIFT                         5
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_MEDIA2_RS_PERI_MASK                          0x00000020
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_NPU_RS_PERI_SHIFT                            6
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_NPU_RS_PERI_MASK                             0x00000040
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_NPU_ELA_RS_PERI_SHIFT                        7
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_NPU_ELA_RS_PERI_MASK                         0x00000080
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_XRSE_RS_PERI_SHIFT                           8
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_XRSE_RS_PERI_MASK                            0x00000100
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_CPU_RS_PERI_SHIFT                            9
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_CPU_RS_PERI_MASK                             0x00000200
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_LPIS_RS_PERI_SHIFT                           10
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_LPIS_RS_PERI_MASK                            0x00000400
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_XRSE_ELA_RS_PERI_SHIFT                       11
#define PERI_CRG_CLKST14_ST_CLK_DEBUG_ATB_XRSE_ELA_RS_PERI_MASK                        0x00000800
#define PERI_CRG_RST0_W1S                          0x0100
#define PERI_CRG_RST0_W1S_IP_PRST_PERI_SPINLOCK_N_SHIFT                                0
#define PERI_CRG_RST0_W1S_IP_PRST_PERI_TIMER_NS_N_SHIFT                                1
#define PERI_CRG_RST0_W1S_IP_RST_OCM_N_SHIFT                                           2
#define PERI_CRG_RST0_W1S_IP_RST_PERI_TIMER0_N_SHIFT                                   6
#define PERI_CRG_RST0_W1S_IP_RST_PERI_TIMER1_N_SHIFT                                   7
#define PERI_CRG_RST0_W1S_IP_RST_PERI_TIMER2_N_SHIFT                                   8
#define PERI_CRG_RST0_W1S_IP_RST_PERI_TIMER3_N_SHIFT                                   9
#define PERI_CRG_RST0_W1S_IP_RST_CSI_N_SHIFT                                           10
#define PERI_CRG_RST0_W1S_IP_RST_CSI_CRG_N_SHIFT                                       11
#define PERI_CRG_RST0_W1S_IP_RST_DSI_N_SHIFT                                           12
#define PERI_CRG_RST0_W1S_IP_RST_DSI_CRG_N_SHIFT                                       13
#define PERI_CRG_RST0_W1S_IP_PRST_PERI_IPC1_N_SHIFT                                    15
#define PERI_CRG_RST0_W1S_IP_PRST_PERI_IPC2_N_SHIFT                                    16
#define PERI_CRG_RST0_W1S_IP_PRST_SPI4_N_SHIFT                                         17
#define PERI_CRG_RST0_W1S_IP_PRST_SPI5_N_SHIFT                                         18
#define PERI_CRG_RST0_W1S_IP_PRST_SPI6_N_SHIFT                                         19
#define PERI_CRG_RST0_W1S_IP_RST_SPI4_N_SHIFT                                          20
#define PERI_CRG_RST0_W1S_IP_RST_SPI5_N_SHIFT                                          21
#define PERI_CRG_RST0_W1S_IP_RST_SPI6_N_SHIFT                                          22
#define PERI_CRG_RST0_W1S_IP_PRST_PERI_WDT0_N_SHIFT                                    23
#define PERI_CRG_RST0_W1S_IP_PRST_PERI_WDT1_N_SHIFT                                    24
#define PERI_CRG_RST0_W1S_IP_RST_PERI_WDT0_N_SHIFT                                     25
#define PERI_CRG_RST0_W1S_IP_RST_PERI_WDT1_N_SHIFT                                     26
#define PERI_CRG_RST0_W1C                          0x0104
#define PERI_CRG_RST0_W1C_IP_PRST_PERI_SPINLOCK_N_SHIFT                                0
#define PERI_CRG_RST0_W1C_IP_PRST_PERI_TIMER_NS_N_SHIFT                                1
#define PERI_CRG_RST0_W1C_IP_RST_OCM_N_SHIFT                                           2
#define PERI_CRG_RST0_W1C_IP_RST_PERI_TIMER0_N_SHIFT                                   6
#define PERI_CRG_RST0_W1C_IP_RST_PERI_TIMER1_N_SHIFT                                   7
#define PERI_CRG_RST0_W1C_IP_RST_PERI_TIMER2_N_SHIFT                                   8
#define PERI_CRG_RST0_W1C_IP_RST_PERI_TIMER3_N_SHIFT                                   9
#define PERI_CRG_RST0_W1C_IP_RST_CSI_N_SHIFT                                           10
#define PERI_CRG_RST0_W1C_IP_RST_CSI_CRG_N_SHIFT                                       11
#define PERI_CRG_RST0_W1C_IP_RST_DSI_N_SHIFT                                           12
#define PERI_CRG_RST0_W1C_IP_RST_DSI_CRG_N_SHIFT                                       13
#define PERI_CRG_RST0_W1C_IP_PRST_PERI_IPC1_N_SHIFT                                    15
#define PERI_CRG_RST0_W1C_IP_PRST_PERI_IPC2_N_SHIFT                                    16
#define PERI_CRG_RST0_W1C_IP_PRST_SPI4_N_SHIFT                                         17
#define PERI_CRG_RST0_W1C_IP_PRST_SPI5_N_SHIFT                                         18
#define PERI_CRG_RST0_W1C_IP_PRST_SPI6_N_SHIFT                                         19
#define PERI_CRG_RST0_W1C_IP_RST_SPI4_N_SHIFT                                          20
#define PERI_CRG_RST0_W1C_IP_RST_SPI5_N_SHIFT                                          21
#define PERI_CRG_RST0_W1C_IP_RST_SPI6_N_SHIFT                                          22
#define PERI_CRG_RST0_W1C_IP_PRST_PERI_WDT0_N_SHIFT                                    23
#define PERI_CRG_RST0_W1C_IP_PRST_PERI_WDT1_N_SHIFT                                    24
#define PERI_CRG_RST0_W1C_IP_RST_PERI_WDT0_N_SHIFT                                     25
#define PERI_CRG_RST0_W1C_IP_RST_PERI_WDT1_N_SHIFT                                     26
#define PERI_CRG_RST0_RO                           0x0108
#define PERI_CRG_RST0_RO_IP_PRST_PERI_SPINLOCK_N_SHIFT                                 0
#define PERI_CRG_RST0_RO_IP_PRST_PERI_SPINLOCK_N_MASK                                  0x00000001
#define PERI_CRG_RST0_RO_IP_PRST_PERI_TIMER_NS_N_SHIFT                                 1
#define PERI_CRG_RST0_RO_IP_PRST_PERI_TIMER_NS_N_MASK                                  0x00000002
#define PERI_CRG_RST0_RO_IP_RST_OCM_N_SHIFT                                            2
#define PERI_CRG_RST0_RO_IP_RST_OCM_N_MASK                                             0x00000004
#define PERI_CRG_RST0_RO_IP_RST_PERI_TIMER0_N_SHIFT                                    6
#define PERI_CRG_RST0_RO_IP_RST_PERI_TIMER0_N_MASK                                     0x00000040
#define PERI_CRG_RST0_RO_IP_RST_PERI_TIMER1_N_SHIFT                                    7
#define PERI_CRG_RST0_RO_IP_RST_PERI_TIMER1_N_MASK                                     0x00000080
#define PERI_CRG_RST0_RO_IP_RST_PERI_TIMER2_N_SHIFT                                    8
#define PERI_CRG_RST0_RO_IP_RST_PERI_TIMER2_N_MASK                                     0x00000100
#define PERI_CRG_RST0_RO_IP_RST_PERI_TIMER3_N_SHIFT                                    9
#define PERI_CRG_RST0_RO_IP_RST_PERI_TIMER3_N_MASK                                     0x00000200
#define PERI_CRG_RST0_RO_IP_RST_CSI_N_SHIFT                                            10
#define PERI_CRG_RST0_RO_IP_RST_CSI_N_MASK                                             0x00000400
#define PERI_CRG_RST0_RO_IP_RST_CSI_CRG_N_SHIFT                                        11
#define PERI_CRG_RST0_RO_IP_RST_CSI_CRG_N_MASK                                         0x00000800
#define PERI_CRG_RST0_RO_IP_RST_DSI_N_SHIFT                                            12
#define PERI_CRG_RST0_RO_IP_RST_DSI_N_MASK                                             0x00001000
#define PERI_CRG_RST0_RO_IP_RST_DSI_CRG_N_SHIFT                                        13
#define PERI_CRG_RST0_RO_IP_RST_DSI_CRG_N_MASK                                         0x00002000
#define PERI_CRG_RST0_RO_IP_PRST_PERI_IPC1_N_SHIFT                                     15
#define PERI_CRG_RST0_RO_IP_PRST_PERI_IPC1_N_MASK                                      0x00008000
#define PERI_CRG_RST0_RO_IP_PRST_PERI_IPC2_N_SHIFT                                     16
#define PERI_CRG_RST0_RO_IP_PRST_PERI_IPC2_N_MASK                                      0x00010000
#define PERI_CRG_RST0_RO_IP_PRST_SPI4_N_SHIFT                                          17
#define PERI_CRG_RST0_RO_IP_PRST_SPI4_N_MASK                                           0x00020000
#define PERI_CRG_RST0_RO_IP_PRST_SPI5_N_SHIFT                                          18
#define PERI_CRG_RST0_RO_IP_PRST_SPI5_N_MASK                                           0x00040000
#define PERI_CRG_RST0_RO_IP_PRST_SPI6_N_SHIFT                                          19
#define PERI_CRG_RST0_RO_IP_PRST_SPI6_N_MASK                                           0x00080000
#define PERI_CRG_RST0_RO_IP_RST_SPI4_N_SHIFT                                           20
#define PERI_CRG_RST0_RO_IP_RST_SPI4_N_MASK                                            0x00100000
#define PERI_CRG_RST0_RO_IP_RST_SPI5_N_SHIFT                                           21
#define PERI_CRG_RST0_RO_IP_RST_SPI5_N_MASK                                            0x00200000
#define PERI_CRG_RST0_RO_IP_RST_SPI6_N_SHIFT                                           22
#define PERI_CRG_RST0_RO_IP_RST_SPI6_N_MASK                                            0x00400000
#define PERI_CRG_RST0_RO_IP_PRST_PERI_WDT0_N_SHIFT                                     23
#define PERI_CRG_RST0_RO_IP_PRST_PERI_WDT0_N_MASK                                      0x00800000
#define PERI_CRG_RST0_RO_IP_PRST_PERI_WDT1_N_SHIFT                                     24
#define PERI_CRG_RST0_RO_IP_PRST_PERI_WDT1_N_MASK                                      0x01000000
#define PERI_CRG_RST0_RO_IP_RST_PERI_WDT0_N_SHIFT                                      25
#define PERI_CRG_RST0_RO_IP_RST_PERI_WDT0_N_MASK                                       0x02000000
#define PERI_CRG_RST0_RO_IP_RST_PERI_WDT1_N_SHIFT                                      26
#define PERI_CRG_RST0_RO_IP_RST_PERI_WDT1_N_MASK                                       0x04000000
#define PERI_CRG_RST1_W1S                          0x0110
#define PERI_CRG_RST1_W1S_IP_PRST_I2C0_N_SHIFT                                         0
#define PERI_CRG_RST1_W1S_IP_PRST_I2C1_N_SHIFT                                         1
#define PERI_CRG_RST1_W1S_IP_PRST_I2C2_N_SHIFT                                         2
#define PERI_CRG_RST1_W1S_IP_PRST_I2C3_N_SHIFT                                         3
#define PERI_CRG_RST1_W1S_IP_PRST_I2C4_N_SHIFT                                         4
#define PERI_CRG_RST1_W1S_IP_PRST_I2C5_N_SHIFT                                         5
#define PERI_CRG_RST1_W1S_IP_PRST_I2C6_N_SHIFT                                         6
#define PERI_CRG_RST1_W1S_IP_PRST_I2C9_N_SHIFT                                         9
#define PERI_CRG_RST1_W1S_IP_PRST_I2C10_N_SHIFT                                        10
#define PERI_CRG_RST1_W1S_IP_PRST_I2C11_N_SHIFT                                        11
#define PERI_CRG_RST1_W1S_IP_PRST_I2C12_N_SHIFT                                        12
#define PERI_CRG_RST1_W1S_IP_PRST_I2C13_N_SHIFT                                        13
#define PERI_CRG_RST1_W1S_IP_PRST_I2C20_N_SHIFT                                        14
#define PERI_CRG_RST1_W1S_IP_RST_DMA_NS_N_SHIFT                                        16
#define PERI_CRG_RST1_W1S_IP_RST_DMA_S_N_SHIFT                                         17
#define PERI_CRG_RST1_W1S_IP_RST_DMA_NS_TPC_N_SHIFT                                    18
#define PERI_CRG_RST1_W1S_IP_RST_DMA_S_TPC_N_SHIFT                                     19
#define PERI_CRG_RST1_W1S_IP_RST_PERFMON_N_SHIFT                                       20
#define PERI_CRG_RST1_W1S_IP_RST_PERFSTAT_N_SHIFT                                      21
#define PERI_CRG_RST1_W1C                          0x0114
#define PERI_CRG_RST1_W1C_IP_PRST_I2C0_N_SHIFT                                         0
#define PERI_CRG_RST1_W1C_IP_PRST_I2C1_N_SHIFT                                         1
#define PERI_CRG_RST1_W1C_IP_PRST_I2C2_N_SHIFT                                         2
#define PERI_CRG_RST1_W1C_IP_PRST_I2C3_N_SHIFT                                         3
#define PERI_CRG_RST1_W1C_IP_PRST_I2C4_N_SHIFT                                         4
#define PERI_CRG_RST1_W1C_IP_PRST_I2C5_N_SHIFT                                         5
#define PERI_CRG_RST1_W1C_IP_PRST_I2C6_N_SHIFT                                         6
#define PERI_CRG_RST1_W1C_IP_PRST_I2C9_N_SHIFT                                         9
#define PERI_CRG_RST1_W1C_IP_PRST_I2C10_N_SHIFT                                        10
#define PERI_CRG_RST1_W1C_IP_PRST_I2C11_N_SHIFT                                        11
#define PERI_CRG_RST1_W1C_IP_PRST_I2C12_N_SHIFT                                        12
#define PERI_CRG_RST1_W1C_IP_PRST_I2C13_N_SHIFT                                        13
#define PERI_CRG_RST1_W1C_IP_PRST_I2C20_N_SHIFT                                        14
#define PERI_CRG_RST1_W1C_IP_RST_DMA_NS_N_SHIFT                                        16
#define PERI_CRG_RST1_W1C_IP_RST_DMA_S_N_SHIFT                                         17
#define PERI_CRG_RST1_W1C_IP_RST_DMA_NS_TPC_N_SHIFT                                    18
#define PERI_CRG_RST1_W1C_IP_RST_DMA_S_TPC_N_SHIFT                                     19
#define PERI_CRG_RST1_W1C_IP_RST_PERFMON_N_SHIFT                                       20
#define PERI_CRG_RST1_W1C_IP_RST_PERFSTAT_N_SHIFT                                      21
#define PERI_CRG_RST1_RO                           0x0118
#define PERI_CRG_RST1_RO_IP_PRST_I2C0_N_SHIFT                                          0
#define PERI_CRG_RST1_RO_IP_PRST_I2C0_N_MASK                                           0x00000001
#define PERI_CRG_RST1_RO_IP_PRST_I2C1_N_SHIFT                                          1
#define PERI_CRG_RST1_RO_IP_PRST_I2C1_N_MASK                                           0x00000002
#define PERI_CRG_RST1_RO_IP_PRST_I2C2_N_SHIFT                                          2
#define PERI_CRG_RST1_RO_IP_PRST_I2C2_N_MASK                                           0x00000004
#define PERI_CRG_RST1_RO_IP_PRST_I2C3_N_SHIFT                                          3
#define PERI_CRG_RST1_RO_IP_PRST_I2C3_N_MASK                                           0x00000008
#define PERI_CRG_RST1_RO_IP_PRST_I2C4_N_SHIFT                                          4
#define PERI_CRG_RST1_RO_IP_PRST_I2C4_N_MASK                                           0x00000010
#define PERI_CRG_RST1_RO_IP_PRST_I2C5_N_SHIFT                                          5
#define PERI_CRG_RST1_RO_IP_PRST_I2C5_N_MASK                                           0x00000020
#define PERI_CRG_RST1_RO_IP_PRST_I2C6_N_SHIFT                                          6
#define PERI_CRG_RST1_RO_IP_PRST_I2C6_N_MASK                                           0x00000040
#define PERI_CRG_RST1_RO_IP_PRST_I2C9_N_SHIFT                                          9
#define PERI_CRG_RST1_RO_IP_PRST_I2C9_N_MASK                                           0x00000200
#define PERI_CRG_RST1_RO_IP_PRST_I2C10_N_SHIFT                                         10
#define PERI_CRG_RST1_RO_IP_PRST_I2C10_N_MASK                                          0x00000400
#define PERI_CRG_RST1_RO_IP_PRST_I2C11_N_SHIFT                                         11
#define PERI_CRG_RST1_RO_IP_PRST_I2C11_N_MASK                                          0x00000800
#define PERI_CRG_RST1_RO_IP_PRST_I2C12_N_SHIFT                                         12
#define PERI_CRG_RST1_RO_IP_PRST_I2C12_N_MASK                                          0x00001000
#define PERI_CRG_RST1_RO_IP_PRST_I2C13_N_SHIFT                                         13
#define PERI_CRG_RST1_RO_IP_PRST_I2C13_N_MASK                                          0x00002000
#define PERI_CRG_RST1_RO_IP_PRST_I2C20_N_SHIFT                                         14
#define PERI_CRG_RST1_RO_IP_PRST_I2C20_N_MASK                                          0x00004000
#define PERI_CRG_RST1_RO_IP_RST_DMA_NS_N_SHIFT                                         16
#define PERI_CRG_RST1_RO_IP_RST_DMA_NS_N_MASK                                          0x00010000
#define PERI_CRG_RST1_RO_IP_RST_DMA_S_N_SHIFT                                          17
#define PERI_CRG_RST1_RO_IP_RST_DMA_S_N_MASK                                           0x00020000
#define PERI_CRG_RST1_RO_IP_RST_DMA_NS_TPC_N_SHIFT                                     18
#define PERI_CRG_RST1_RO_IP_RST_DMA_NS_TPC_N_MASK                                      0x00040000
#define PERI_CRG_RST1_RO_IP_RST_DMA_S_TPC_N_SHIFT                                      19
#define PERI_CRG_RST1_RO_IP_RST_DMA_S_TPC_N_MASK                                       0x00080000
#define PERI_CRG_RST1_RO_IP_RST_PERFMON_N_SHIFT                                        20
#define PERI_CRG_RST1_RO_IP_RST_PERFMON_N_MASK                                         0x00100000
#define PERI_CRG_RST1_RO_IP_RST_PERFSTAT_N_SHIFT                                       21
#define PERI_CRG_RST1_RO_IP_RST_PERFSTAT_N_MASK                                        0x00200000
#define PERI_CRG_RST2_W1S                          0x0120
#define PERI_CRG_RST2_W1S_IP_RST_I2C0_N_SHIFT                                          0
#define PERI_CRG_RST2_W1S_IP_RST_I2C1_N_SHIFT                                          1
#define PERI_CRG_RST2_W1S_IP_RST_I2C2_N_SHIFT                                          2
#define PERI_CRG_RST2_W1S_IP_RST_I2C3_N_SHIFT                                          3
#define PERI_CRG_RST2_W1S_IP_RST_I2C4_N_SHIFT                                          4
#define PERI_CRG_RST2_W1S_IP_RST_I2C5_N_SHIFT                                          5
#define PERI_CRG_RST2_W1S_IP_RST_I2C6_N_SHIFT                                          6
#define PERI_CRG_RST2_W1S_IP_RST_I2C9_N_SHIFT                                          9
#define PERI_CRG_RST2_W1S_IP_RST_I2C10_N_SHIFT                                         10
#define PERI_CRG_RST2_W1S_IP_RST_I2C11_N_SHIFT                                         11
#define PERI_CRG_RST2_W1S_IP_RST_I2C12_N_SHIFT                                         12
#define PERI_CRG_RST2_W1S_IP_RST_I2C13_N_SHIFT                                         13
#define PERI_CRG_RST2_W1S_IP_RST_I2C20_N_SHIFT                                         14
#define PERI_CRG_RST2_W1S_IP_RST_I3C0_N_SHIFT                                          15
#define PERI_CRG_RST2_W1S_IP_PRST_I3C0_N_SHIFT                                         16
#define PERI_CRG_RST2_W1S_IP_RST_PWM0_N_SHIFT                                          19
#define PERI_CRG_RST2_W1S_IP_RST_PWM1_N_SHIFT                                          20
#define PERI_CRG_RST2_W1S_IP_RST_UART3_N_SHIFT                                         21
#define PERI_CRG_RST2_W1S_IP_RST_UART6_N_SHIFT                                         22
#define PERI_CRG_RST2_W1C                          0x0124
#define PERI_CRG_RST2_W1C_IP_RST_I2C0_N_SHIFT                                          0
#define PERI_CRG_RST2_W1C_IP_RST_I2C1_N_SHIFT                                          1
#define PERI_CRG_RST2_W1C_IP_RST_I2C2_N_SHIFT                                          2
#define PERI_CRG_RST2_W1C_IP_RST_I2C3_N_SHIFT                                          3
#define PERI_CRG_RST2_W1C_IP_RST_I2C4_N_SHIFT                                          4
#define PERI_CRG_RST2_W1C_IP_RST_I2C5_N_SHIFT                                          5
#define PERI_CRG_RST2_W1C_IP_RST_I2C6_N_SHIFT                                          6
#define PERI_CRG_RST2_W1C_IP_RST_I2C9_N_SHIFT                                          9
#define PERI_CRG_RST2_W1C_IP_RST_I2C10_N_SHIFT                                         10
#define PERI_CRG_RST2_W1C_IP_RST_I2C11_N_SHIFT                                         11
#define PERI_CRG_RST2_W1C_IP_RST_I2C12_N_SHIFT                                         12
#define PERI_CRG_RST2_W1C_IP_RST_I2C13_N_SHIFT                                         13
#define PERI_CRG_RST2_W1C_IP_RST_I2C20_N_SHIFT                                         14
#define PERI_CRG_RST2_W1C_IP_RST_I3C0_N_SHIFT                                          15
#define PERI_CRG_RST2_W1C_IP_PRST_I3C0_N_SHIFT                                         16
#define PERI_CRG_RST2_W1C_IP_RST_PWM0_N_SHIFT                                          19
#define PERI_CRG_RST2_W1C_IP_RST_PWM1_N_SHIFT                                          20
#define PERI_CRG_RST2_W1C_IP_RST_UART3_N_SHIFT                                         21
#define PERI_CRG_RST2_W1C_IP_RST_UART6_N_SHIFT                                         22
#define PERI_CRG_RST2_RO                           0x0128
#define PERI_CRG_RST2_RO_IP_RST_I2C0_N_SHIFT                                           0
#define PERI_CRG_RST2_RO_IP_RST_I2C0_N_MASK                                            0x00000001
#define PERI_CRG_RST2_RO_IP_RST_I2C1_N_SHIFT                                           1
#define PERI_CRG_RST2_RO_IP_RST_I2C1_N_MASK                                            0x00000002
#define PERI_CRG_RST2_RO_IP_RST_I2C2_N_SHIFT                                           2
#define PERI_CRG_RST2_RO_IP_RST_I2C2_N_MASK                                            0x00000004
#define PERI_CRG_RST2_RO_IP_RST_I2C3_N_SHIFT                                           3
#define PERI_CRG_RST2_RO_IP_RST_I2C3_N_MASK                                            0x00000008
#define PERI_CRG_RST2_RO_IP_RST_I2C4_N_SHIFT                                           4
#define PERI_CRG_RST2_RO_IP_RST_I2C4_N_MASK                                            0x00000010
#define PERI_CRG_RST2_RO_IP_RST_I2C5_N_SHIFT                                           5
#define PERI_CRG_RST2_RO_IP_RST_I2C5_N_MASK                                            0x00000020
#define PERI_CRG_RST2_RO_IP_RST_I2C6_N_SHIFT                                           6
#define PERI_CRG_RST2_RO_IP_RST_I2C6_N_MASK                                            0x00000040
#define PERI_CRG_RST2_RO_IP_RST_I2C9_N_SHIFT                                           9
#define PERI_CRG_RST2_RO_IP_RST_I2C9_N_MASK                                            0x00000200
#define PERI_CRG_RST2_RO_IP_RST_I2C10_N_SHIFT                                          10
#define PERI_CRG_RST2_RO_IP_RST_I2C10_N_MASK                                           0x00000400
#define PERI_CRG_RST2_RO_IP_RST_I2C11_N_SHIFT                                          11
#define PERI_CRG_RST2_RO_IP_RST_I2C11_N_MASK                                           0x00000800
#define PERI_CRG_RST2_RO_IP_RST_I2C12_N_SHIFT                                          12
#define PERI_CRG_RST2_RO_IP_RST_I2C12_N_MASK                                           0x00001000
#define PERI_CRG_RST2_RO_IP_RST_I2C13_N_SHIFT                                          13
#define PERI_CRG_RST2_RO_IP_RST_I2C13_N_MASK                                           0x00002000
#define PERI_CRG_RST2_RO_IP_RST_I2C20_N_SHIFT                                          14
#define PERI_CRG_RST2_RO_IP_RST_I2C20_N_MASK                                           0x00004000
#define PERI_CRG_RST2_RO_IP_RST_I3C0_N_SHIFT                                           15
#define PERI_CRG_RST2_RO_IP_RST_I3C0_N_MASK                                            0x00008000
#define PERI_CRG_RST2_RO_IP_PRST_I3C0_N_SHIFT                                          16
#define PERI_CRG_RST2_RO_IP_PRST_I3C0_N_MASK                                           0x00010000
#define PERI_CRG_RST2_RO_IP_RST_PWM0_N_SHIFT                                           19
#define PERI_CRG_RST2_RO_IP_RST_PWM0_N_MASK                                            0x00080000
#define PERI_CRG_RST2_RO_IP_RST_PWM1_N_SHIFT                                           20
#define PERI_CRG_RST2_RO_IP_RST_PWM1_N_MASK                                            0x00100000
#define PERI_CRG_RST2_RO_IP_RST_UART3_N_SHIFT                                          21
#define PERI_CRG_RST2_RO_IP_RST_UART3_N_MASK                                           0x00200000
#define PERI_CRG_RST2_RO_IP_RST_UART6_N_SHIFT                                          22
#define PERI_CRG_RST2_RO_IP_RST_UART6_N_MASK                                           0x00400000
#define PERI_CRG_CLKDIV0                           0x0200
#define PERI_CRG_CLKDIV0_DIV_CLK_TIMER_H_SHIFT                                         6
#define PERI_CRG_CLKDIV0_DIV_CLK_TIMER_H_MASK                                          0x00000fc0
#define PERI_CRG_CLKDIV0_SC_GT_CLK_TIMER_H_SHIFT                                       13
#define PERI_CRG_CLKDIV0_SC_GT_CLK_TIMER_H_MASK                                        0x00002000
#define PERI_CRG_CLKDIV1                           0x0204
#define PERI_CRG_CLKDIV1_DIV_CLK_SPI4_SHIFT                                            0
#define PERI_CRG_CLKDIV1_DIV_CLK_SPI4_MASK                                             0x0000003f
#define PERI_CRG_CLKDIV1_DIV_CLK_SPI5_SHIFT                                            6
#define PERI_CRG_CLKDIV1_DIV_CLK_SPI5_MASK                                             0x00000fc0
#define PERI_CRG_CLKDIV1_SC_GT_CLK_SPI4_SHIFT                                          12
#define PERI_CRG_CLKDIV1_SC_GT_CLK_SPI4_MASK                                           0x00001000
#define PERI_CRG_CLKDIV1_SC_GT_CLK_SPI5_SHIFT                                          13
#define PERI_CRG_CLKDIV1_SC_GT_CLK_SPI5_MASK                                           0x00002000
#define PERI_CRG_CLKDIV2                           0x0208
#define PERI_CRG_CLKDIV2_DIV_CLK_SPI6_SHIFT                                            0
#define PERI_CRG_CLKDIV2_DIV_CLK_SPI6_MASK                                             0x0000003f
#define PERI_CRG_CLKDIV2_SC_GT_CLK_SPI6_SHIFT                                          12
#define PERI_CRG_CLKDIV2_SC_GT_CLK_SPI6_MASK                                           0x00001000
#define PERI_CRG_CLKDIV3                           0x020c
#define PERI_CRG_CLKDIV3_DIV_CLK_UART_SHIFT                                            0
#define PERI_CRG_CLKDIV3_DIV_CLK_UART_MASK                                             0x0000003f
#define PERI_CRG_CLKDIV3_DIV_CLK_PWM_SHIFT                                             6
#define PERI_CRG_CLKDIV3_DIV_CLK_PWM_MASK                                              0x00000fc0
#define PERI_CRG_CLKDIV3_SC_GT_CLK_UART_SHIFT                                          12
#define PERI_CRG_CLKDIV3_SC_GT_CLK_UART_MASK                                           0x00001000
#define PERI_CRG_CLKDIV3_SC_GT_CLK_PWM_SHIFT                                           13
#define PERI_CRG_CLKDIV3_SC_GT_CLK_PWM_MASK                                            0x00002000
#define PERI_CRG_CLKDIV3_SC_GT_PCLK_MEDIA1_CFG_SHIFT                                   14
#define PERI_CRG_CLKDIV3_SC_GT_PCLK_MEDIA1_CFG_MASK                                    0x00004000
#define PERI_CRG_CLKDIV3_SC_GT_PCLK_MEDIA2_CFG_SHIFT                                   15
#define PERI_CRG_CLKDIV3_SC_GT_PCLK_MEDIA2_CFG_MASK                                    0x00008000
#define PERI_CRG_CLKDIV12                          0x0210
#define PERI_CRG_CLKDIV12_DIV_CLK_I2C_SHIFT                                            6
#define PERI_CRG_CLKDIV12_DIV_CLK_I2C_MASK                                             0x00000fc0
#define PERI_CRG_CLKDIV12_SC_GT_CLK_I2C_SHIFT                                          13
#define PERI_CRG_CLKDIV12_SC_GT_CLK_I2C_MASK                                           0x00002000
#define PERI_CRG_CLKDIV15                          0x0214
#define PERI_CRG_CLKDIV15_DIV_CLK_I3C_SHIFT                                            6
#define PERI_CRG_CLKDIV15_DIV_CLK_I3C_MASK                                             0x00000fc0
#define PERI_CRG_CLKDIV15_SC_GT_CLK_I3C_SHIFT                                          13
#define PERI_CRG_CLKDIV15_SC_GT_CLK_I3C_MASK                                           0x00002000
#define PERI_CRG_CLKDIV28                          0x0218
#define PERI_CRG_CLKDIV28_DIV_CLK_OCM_GPLL_SHIFT                                       0
#define PERI_CRG_CLKDIV28_DIV_CLK_OCM_GPLL_MASK                                        0x0000003f
#define PERI_CRG_CLKDIV28_DIV_CLK_OCM_LPPLL_SHIFT                                      6
#define PERI_CRG_CLKDIV28_DIV_CLK_OCM_LPPLL_MASK                                       0x00000fc0
#define PERI_CRG_CLKDIV28_SC_GT_CLK_OCM_GPLL_SHIFT                                     12
#define PERI_CRG_CLKDIV28_SC_GT_CLK_OCM_GPLL_MASK                                      0x00001000
#define PERI_CRG_CLKDIV28_SC_GT_CLK_OCM_LPPLL_SHIFT                                    13
#define PERI_CRG_CLKDIV28_SC_GT_CLK_OCM_LPPLL_MASK                                     0x00002000
#define PERI_CRG_CLKDIV23                          0x021c
#define PERI_CRG_CLKDIV23_DIV_CLK_PCIE_AUX_SHIFT                                       0
#define PERI_CRG_CLKDIV23_DIV_CLK_PCIE_AUX_MASK                                        0x0000003f
#define PERI_CRG_CLKDIV23_SC_GT_CLK_PCIE_AUX_SHIFT                                     12
#define PERI_CRG_CLKDIV23_SC_GT_CLK_PCIE_AUX_MASK                                      0x00001000
#define PERI_CRG_CLKDIV14                          0x0220
#define PERI_CRG_CLKDIV14_DIV_CLK_CSI_SYS_SHIFT                                        6
#define PERI_CRG_CLKDIV14_DIV_CLK_CSI_SYS_MASK                                         0x00000fc0
#define PERI_CRG_CLKDIV14_SC_GT_CLK_CSI_SYS_SHIFT                                      13
#define PERI_CRG_CLKDIV14_SC_GT_CLK_CSI_SYS_MASK                                       0x00002000
#define PERI_CRG_CLKDIV16                          0x0224
#define PERI_CRG_CLKDIV16_DIV_CLK_DSI_SYS_SHIFT                                        0
#define PERI_CRG_CLKDIV16_DIV_CLK_DSI_SYS_MASK                                         0x0000003f
#define PERI_CRG_CLKDIV16_SC_GT_CLK_DSI_SYS_SHIFT                                      12
#define PERI_CRG_CLKDIV16_SC_GT_CLK_DSI_SYS_MASK                                       0x00001000
#define PERI_CRG_AUTOFSCTRL17                      0x0250
#define PERI_CRG_AUTOFSCTRL17_DPU_IDLE_FOR_OCM_BYPASS_SHIFT                            0
#define PERI_CRG_AUTOFSCTRL17_DPU_IDLE_FOR_OCM_BYPASS_MASK                             0x00000001
#define PERI_CRG_AUTOFSCTRL17_NPU_BUS_TGT_NO_PENDING_TRANS_OCM_BYPASS_SHIFT            1
#define PERI_CRG_AUTOFSCTRL17_NPU_BUS_TGT_NO_PENDING_TRANS_OCM_BYPASS_MASK             0x00000002
#define PERI_CRG_AUTOFSCTRL17_MEDIA2_BUS_TGT_NO_PENDING_TRANS_OCM_BYPASS_SHIFT         2
#define PERI_CRG_AUTOFSCTRL17_MEDIA2_BUS_TGT_NO_PENDING_TRANS_OCM_BYPASS_MASK          0x00000004
#define PERI_CRG_AUTOFSCTRL17_MEDIA1_BUS_TGT_NO_PENDING_TRANS_OCM_BYPASS_SHIFT         3
#define PERI_CRG_AUTOFSCTRL17_MEDIA1_BUS_TGT_NO_PENDING_TRANS_OCM_BYPASS_MASK          0x00000008
#define PERI_CRG_AUTOFSCTRL17_IDLE_FLAG_OCM_NIC_BYPASS_SHIFT                           4
#define PERI_CRG_AUTOFSCTRL17_IDLE_FLAG_OCM_NIC_BYPASS_MASK                            0x00000010
#define PERI_CRG_AUTOFSCTRL18                      0x0254
#define PERI_CRG_AUTOFSCTRL18_DEBOUNCE_IN_TIME_OCM_SHIFT                               0
#define PERI_CRG_AUTOFSCTRL18_DEBOUNCE_IN_TIME_OCM_MASK                                0x000003ff
#define PERI_CRG_AUTOFSCTRL18_DEBOUNCE_OUT_TIME_OCM_SHIFT                              10
#define PERI_CRG_AUTOFSCTRL18_DEBOUNCE_OUT_TIME_OCM_MASK                               0x000ffc00
#define PERI_CRG_AUTOFSCTRL18_DEBOUNCE_BYPASS_OCM_SHIFT                                20
#define PERI_CRG_AUTOFSCTRL18_DEBOUNCE_BYPASS_OCM_MASK                                 0x00100000
#define PERI_CRG_AUTOFSCTRL18_AUTOFS_EN_OCM_SHIFT                                      31
#define PERI_CRG_AUTOFSCTRL18_AUTOFS_EN_OCM_MASK                                       0x80000000
#define PERI_CRG_AUTOFSCTRL19                      0x0258
#define PERI_CRG_AUTOFSCTRL19_DIV_AUTO_CLK_OCM_GPLL_SHIFT                              0
#define PERI_CRG_AUTOFSCTRL19_DIV_AUTO_CLK_OCM_GPLL_MASK                               0x0000003f
#define PERI_CRG_AUTOFSCTRL19_DIV_AUTO_CLK_OCM_LPPLL_SHIFT                             6
#define PERI_CRG_AUTOFSCTRL19_DIV_AUTO_CLK_OCM_LPPLL_MASK                              0x00000fc0
#define PERI_CRG_PERISTAT0                         0x0300
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_TIMER_H_SHIFT                                  0
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_TIMER_H_MASK                                   0x00000001
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_SPI4_SHIFT                                     1
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_SPI4_MASK                                      0x00000002
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_SPI5_SHIFT                                     2
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_SPI5_MASK                                      0x00000004
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_SPI6_SHIFT                                     3
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_SPI6_MASK                                      0x00000008
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_UART_SHIFT                                     4
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_UART_MASK                                      0x00000010
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_PWM_SHIFT                                      5
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_PWM_MASK                                       0x00000020
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_I2C_SHIFT                                      6
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_I2C_MASK                                       0x00000040
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_I3C_SHIFT                                      7
#define PERI_CRG_PERISTAT0_DIV_DONE_CLK_I3C_MASK                                       0x00000080
#define PERI_CRG_PERISTAT1                         0x0304
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_MAINBUS_DATA_LPPLL_DIV_SHIFT                   0
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_MAINBUS_DATA_LPPLL_DIV_MASK                    0x00000001
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_MAINBUS_DATA_GPLL_DIV_SHIFT                    1
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_MAINBUS_DATA_GPLL_DIV_MASK                     0x00000002
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_SYSBUS_DATA_LPPLL_DIV_SHIFT                    2
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_SYSBUS_DATA_LPPLL_DIV_MASK                     0x00000004
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_SYSBUS_DATA_GPLL_SHIFT                         4
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_SYSBUS_DATA_GPLL_MASK                          0x00000010
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_CFGBUS_GPLL_SHIFT                              5
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_CFGBUS_GPLL_MASK                               0x00000020
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_PERICFG_BUS_SHIFT                              6
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_PERICFG_BUS_MASK                               0x00000040
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_CFGBUS_LPPLL_SHIFT                             7
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_CFGBUS_LPPLL_MASK                              0x00000080
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_SPI0_SHIFT                                     8
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_SPI0_MASK                                      0x00000100
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_SPI1_SHIFT                                     9
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_SPI1_MASK                                      0x00000200
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_SPI2_SHIFT                                     10
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_SPI2_MASK                                      0x00000400
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_DMA_BUS_DATA_LPPLL_DIV_SHIFT                   18
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_DMA_BUS_DATA_LPPLL_DIV_MASK                    0x00040000
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_DMA_BUS_DATA_GPLL_DIV_SHIFT                    19
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_DMA_BUS_DATA_GPLL_DIV_MASK                     0x00080000
#define PERI_CRG_PERISTAT1_DIV_DONE_PCLK_DMA_BUS_GPLL_DIV_SHIFT                        20
#define PERI_CRG_PERISTAT1_DIV_DONE_PCLK_DMA_BUS_GPLL_DIV_MASK                         0x00100000
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_CSI_SYS_SHIFT                                  21
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_CSI_SYS_MASK                                   0x00200000
#define PERI_CRG_PERISTAT1_DIV_DONE_PCLK_DMA_BUS_LPPLL_DIV_SHIFT                       22
#define PERI_CRG_PERISTAT1_DIV_DONE_PCLK_DMA_BUS_LPPLL_DIV_MASK                        0x00400000
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_DSI_SYS_SHIFT                                  24
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_DSI_SYS_MASK                                   0x01000000
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_DEBUG_APB_SHIFT                                25
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_DEBUG_APB_MASK                                 0x02000000
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_DEBUG_ATB_H_SHIFT                              27
#define PERI_CRG_PERISTAT1_DIV_DONE_CLK_DEBUG_ATB_H_MASK                               0x08000000
#define PERI_CRG_PERISTAT2                         0x0308
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_DEBUG_TRACE_SHIFT                              0
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_DEBUG_TRACE_MASK                               0x00000001
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_XCTRL_DDR_SHIFT                                1
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_XCTRL_DDR_MASK                                 0x00000002
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_XCTRL_CPU_SHIFT                                2
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_XCTRL_CPU_MASK                                 0x00000004
#define PERI_CRG_PERISTAT2_DIV_DONE_PCLK_CRG_CORE_LPPLL_DIV_SHIFT                      3
#define PERI_CRG_PERISTAT2_DIV_DONE_PCLK_CRG_CORE_LPPLL_DIV_MASK                       0x00000008
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_HSS1_GPLL_DIV_SHIFT                            4
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_HSS1_GPLL_DIV_MASK                             0x00000010
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_HSS1_LPPLL_DIV_SHIFT                           5
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_HSS1_LPPLL_DIV_MASK                            0x00000020
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_HSS2_GPLL_DIV_SHIFT                            6
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_HSS2_GPLL_DIV_MASK                             0x00000040
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_HSS2_LPPLL_DIV_SHIFT                           7
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_HSS2_LPPLL_DIV_MASK                            0x00000080
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_XRSE_GPLL_DIV_SHIFT                            8
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_XRSE_GPLL_DIV_MASK                             0x00000100
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_CPU_BUS_SHIFT                                  9
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_CPU_BUS_MASK                                   0x00000200
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_PCIE_AUX_SHIFT                                 10
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_PCIE_AUX_MASK                                  0x00000400
#define PERI_CRG_PERISTAT2_DIV_DONE_PCLK_CRG_CORE_GPLL_DIV_SHIFT                       11
#define PERI_CRG_PERISTAT2_DIV_DONE_PCLK_CRG_CORE_GPLL_DIV_MASK                        0x00000800
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_DDR_DFICLK_GPLL_DIV_SHIFT                      14
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_DDR_DFICLK_GPLL_DIV_MASK                       0x00004000
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_OCM_LPPLL_DIV_SHIFT                            17
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_OCM_LPPLL_DIV_MASK                             0x00020000
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_OCM_GPLL_DIV_SHIFT                             18
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_OCM_GPLL_DIV_MASK                              0x00040000
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_GPIODB_SHIFT                                   19
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_GPIODB_MASK                                    0x00080000
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_PERI2NPU_NOC_SHIFT                             20
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_PERI2NPU_NOC_MASK                              0x00100000
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_PERI2NPU_NIC_SHIFT                             21
#define PERI_CRG_PERISTAT2_DIV_DONE_CLK_PERI2NPU_NIC_MASK                              0x00200000
#define PERI_CRG_PERISTAT3                         0x0320
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_MAINBUS_DATA_GPLL_SHIFT                          0
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_MAINBUS_DATA_GPLL_MASK                           0x0000000f
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_MAINBUS_DATA_SHIFT                               4
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_MAINBUS_DATA_MASK                                0x00000070
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_SYSBUS_DATA_GPLL_SHIFT                           7
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_SYSBUS_DATA_GPLL_MASK                            0x00000780
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_SYSBUS_DATA_SHIFT                                11
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_SYSBUS_DATA_MASK                                 0x00003800
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_IP_SW_SHIFT                                      14
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_IP_SW_MASK                                       0x0001c000
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_DMA_BUS_DATA_SHIFT                               19
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_DMA_BUS_DATA_MASK                                0x00380000
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_CSI_SYS_GPLL_SHIFT                               22
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_CSI_SYS_GPLL_MASK                                0x01c00000
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_CSI_SYS_SHIFT                                    25
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_CSI_SYS_MASK                                     0x0e000000
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_OCM_SHIFT                                        28
#define PERI_CRG_PERISTAT3_SW_ACK_CLK_OCM_MASK                                         0x70000000
#define PERI_CRG_PERISTAT4                         0x0324
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_DSI_SHIFT                                        0
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_DSI_MASK                                         0x00000007
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_DEBUG_APB_SHIFT                                  3
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_DEBUG_APB_MASK                                   0x00000038
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_DEBUG_TRACE_SHIFT                                6
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_DEBUG_TRACE_MASK                                 0x000001c0
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_DEBUG_ATB_H_SHIFT                                9
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_DEBUG_ATB_H_MASK                                 0x00000e00
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_XCTRL_DDR_SHIFT                                  12
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_XCTRL_DDR_MASK                                   0x00007000
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_XCTRL_CPU_SHIFT                                  15
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_XCTRL_CPU_MASK                                   0x00038000
#define PERI_CRG_PERISTAT4_SW_ACK_PCLK_CRG_CORE_SHIFT                                  18
#define PERI_CRG_PERISTAT4_SW_ACK_PCLK_CRG_CORE_MASK                                   0x001c0000
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_HSS1_SHIFT                                       21
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_HSS1_MASK                                        0x00e00000
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_HSS2_SHIFT                                       24
#define PERI_CRG_PERISTAT4_SW_ACK_CLK_HSS2_MASK                                        0x07000000
#define PERI_CRG_PERISTAT4_SW_ACK_PCLK_DMA_BUS_SHIFT                                   27
#define PERI_CRG_PERISTAT4_SW_ACK_PCLK_DMA_BUS_MASK                                    0x38000000
#define PERI_CRG_PERISTAT5                         0x0328
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_XRSE_SHIFT                                       0
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_XRSE_MASK                                        0x00000003
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_DDR_DFICLK_SHIFT                                 3
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_DDR_DFICLK_MASK                                  0x00000038
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_CFGBUS_SHIFT                                     10
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_CFGBUS_MASK                                      0x00001c00
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_CPU_BUS_SHIFT                                    13
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_CPU_BUS_MASK                                     0x0000e000
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_PERI2NPU_NIC_SW_SHIFT                            16
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_PERI2NPU_NIC_SW_MASK                             0x000f0000
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_PERI2NPU_NOC_SW_SHIFT                            20
#define PERI_CRG_PERISTAT5_SW_ACK_CLK_PERI2NPU_NOC_SW_MASK                             0x00f00000
#define PERI_CRG_CLKDIV4                           0x0330
#define PERI_CRG_CLKDIV4_SEL_CLK_CSI_SYS_GPLL_SHIFT                                    0
#define PERI_CRG_CLKDIV4_SEL_CLK_CSI_SYS_GPLL_MASK                                     0x00000003
#define PERI_CRG_CLKDIV4_SEL_CLK_CSI_SYS_SHIFT                                         2
#define PERI_CRG_CLKDIV4_SEL_CLK_CSI_SYS_MASK                                          0x0000000c
#define PERI_CRG_CLKDIV4_SEL_CLK_DSI_SHIFT                                             4
#define PERI_CRG_CLKDIV4_SEL_CLK_DSI_MASK                                              0x00000030
#define PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER0_SHIFT                                     8
#define PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER0_MASK                                      0x00000100
#define PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER1_SHIFT                                     9
#define PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER1_MASK                                      0x00000200
#define PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER2_SHIFT                                     10
#define PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER2_MASK                                      0x00000400
#define PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER3_SHIFT                                     11
#define PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER3_MASK                                      0x00000800
#define PERI_CRG_CLKDIV4_SEL_CLK_OCM_SHIFT                                             12
#define PERI_CRG_CLKDIV4_SEL_CLK_OCM_MASK                                              0x00003000
#define PERI_CRG_PERISTAT6                         0x0340
#define PERI_CRG_PERISTAT6_AUTOGT_STATE_CLK_DDR_DFICLK_SHIFT                           0
#define PERI_CRG_PERISTAT6_AUTOGT_STATE_CLK_DDR_DFICLK_MASK                            0x00000001
#define PERI_CRG_PERISTAT6_DMA_BUS_IDLE_STATE_SHIFT                                    2
#define PERI_CRG_PERISTAT6_DMA_BUS_IDLE_STATE_MASK                                     0x00000004
#define PERI_CRG_PERISTAT6_SYSBUS_IDLE_STATE_SHIFT                                     3
#define PERI_CRG_PERISTAT6_SYSBUS_IDLE_STATE_MASK                                      0x00000008
#define PERI_CRG_PERISTAT6_CFGBUS_IDLE_STATE_SHIFT                                     4
#define PERI_CRG_PERISTAT6_CFGBUS_IDLE_STATE_MASK                                      0x00000010
#define PERI_CRG_PERISTAT6_XCTRL_CPU_IDLE_STATE_SHIFT                                  5
#define PERI_CRG_PERISTAT6_XCTRL_CPU_IDLE_STATE_MASK                                   0x00000020
#define PERI_CRG_PERISTAT6_XCTRL_DDR_IDLE_STATE_SHIFT                                  6
#define PERI_CRG_PERISTAT6_XCTRL_DDR_IDLE_STATE_MASK                                   0x00000040
#define PERI_CRG_PERISTAT6_MAINBUS_AUTOFS_IDLE_STATE_SHIFT                             7
#define PERI_CRG_PERISTAT6_MAINBUS_AUTOFS_IDLE_STATE_MASK                              0x00000080
#define PERI_CRG_PERISTAT6_MAINBUS_AUTOGT_IDLE_STATE_SHIFT                             8
#define PERI_CRG_PERISTAT6_MAINBUS_AUTOGT_IDLE_STATE_MASK                              0x00000100
#define PERI_CRG_PERISTAT6_M1_BUS_DATA_IDLE_STATE_SHIFT                                9
#define PERI_CRG_PERISTAT6_M1_BUS_DATA_IDLE_STATE_MASK                                 0x00000200
#define PERI_CRG_PERISTAT6_M1_BUS_CFG_IDLE_STATE_SHIFT                                 10
#define PERI_CRG_PERISTAT6_M1_BUS_CFG_IDLE_STATE_MASK                                  0x00000400
#define PERI_CRG_PERISTAT6_M2_BUS_DATA_IDLE_STATE_SHIFT                                11
#define PERI_CRG_PERISTAT6_M2_BUS_DATA_IDLE_STATE_MASK                                 0x00000800
#define PERI_CRG_PERISTAT6_M2_BUS_CFG_IDLE_STATE_SHIFT                                 12
#define PERI_CRG_PERISTAT6_M2_BUS_CFG_IDLE_STATE_MASK                                  0x00001000
#define PERI_CRG_PERISTAT6_M2_NOC_DATA_IDLE_STATE_SHIFT                                13
#define PERI_CRG_PERISTAT6_M2_NOC_DATA_IDLE_STATE_MASK                                 0x00002000
#define PERI_CRG_PERISTAT6_NPU_BUS_DATA_IDLE_STATE_SHIFT                               14
#define PERI_CRG_PERISTAT6_NPU_BUS_DATA_IDLE_STATE_MASK                                0x00004000
#define PERI_CRG_PERISTAT6_NPU_BUS_CFG_IDLE_STATE_SHIFT                                15
#define PERI_CRG_PERISTAT6_NPU_BUS_CFG_IDLE_STATE_MASK                                 0x00008000
#define PERI_CRG_PERISTAT6_HSS1_BUS_DATA_IDLE_STATE_SHIFT                              16
#define PERI_CRG_PERISTAT6_HSS1_BUS_DATA_IDLE_STATE_MASK                               0x00010000
#define PERI_CRG_PERISTAT6_HSS1_BUS_CFG_IDLE_STATE_SHIFT                               17
#define PERI_CRG_PERISTAT6_HSS1_BUS_CFG_IDLE_STATE_MASK                                0x00020000
#define PERI_CRG_PERISTAT6_HSS2_BUS_DATA_IDLE_STATE_SHIFT                              18
#define PERI_CRG_PERISTAT6_HSS2_BUS_DATA_IDLE_STATE_MASK                               0x00040000
#define PERI_CRG_PERISTAT6_HSS2_BUS_CFG_IDLE_STATE_SHIFT                               19
#define PERI_CRG_PERISTAT6_HSS2_BUS_CFG_IDLE_STATE_MASK                                0x00080000
#define PERI_CRG_PERISTAT6_OCM_IDLE_STATE_SHIFT                                        20
#define PERI_CRG_PERISTAT6_OCM_IDLE_STATE_MASK                                         0x00100000
#define PERI_CRG_PERISTAT6_CPU_BUS_IDLE_STATE_SHIFT                                    21
#define PERI_CRG_PERISTAT6_CPU_BUS_IDLE_STATE_MASK                                     0x00200000
#define PERI_CRG_PLL_LOCK_STATE                    0x0350
#define PERI_CRG_PLL_LOCK_STATE_GPLL_LOCK_SHIFT                                        0
#define PERI_CRG_PLL_LOCK_STATE_GPLL_LOCK_MASK                                         0x00000001
#define PERI_CRG_PLL_LOCK_STATE_PPLL0_LOCK_SHIFT                                       1
#define PERI_CRG_PLL_LOCK_STATE_PPLL0_LOCK_MASK                                        0x00000002
#define PERI_CRG_PLL_LOCK_STATE_PPLL1_LOCK_SHIFT                                       2
#define PERI_CRG_PLL_LOCK_STATE_PPLL1_LOCK_MASK                                        0x00000004
#define PERI_CRG_PLL_LOCK_STATE_PPLL2_LOCK_SHIFT                                       3
#define PERI_CRG_PLL_LOCK_STATE_PPLL2_LOCK_MASK                                        0x00000008
#define PERI_CRG_PLL_LOCK_STATE_PPLL3_LOCK_SHIFT                                       4
#define PERI_CRG_PLL_LOCK_STATE_PPLL3_LOCK_MASK                                        0x00000010
#define PERI_CRG_PLL_LOCK_STATE_DDR_PPLL0_LOCK_SHIFT                                   5
#define PERI_CRG_PLL_LOCK_STATE_DDR_PPLL0_LOCK_MASK                                    0x00000020
#define PERI_CRG_PLL_LOCK_STATE_DDR_PPLL1_LOCK_SHIFT                                   6
#define PERI_CRG_PLL_LOCK_STATE_DDR_PPLL1_LOCK_MASK                                    0x00000040
#define PERI_CRG_INTR_MASK_GPLL                    0x0360
#define PERI_CRG_INTR_MASK_GPLL_INTR_MASK_PLL_INI_ERR_GPLL_SHIFT                       0
#define PERI_CRG_INTR_MASK_GPLL_INTR_MASK_PLL_INI_ERR_GPLL_MASK                        0x00000001
#define PERI_CRG_INTR_MASK_GPLL_INTR_MASK_PLL_UNLOCK_GPLL_SHIFT                        1
#define PERI_CRG_INTR_MASK_GPLL_INTR_MASK_PLL_UNLOCK_GPLL_MASK                         0x00000002
#define PERI_CRG_INTR_MASK_GPLL_INTR_MASK_VOTE_REQ_ERR_GPLL_SHIFT                      2
#define PERI_CRG_INTR_MASK_GPLL_INTR_MASK_VOTE_REQ_ERR_GPLL_MASK                       0x00000004
#define PERI_CRG_INTR_CLEAR_GPLL                   0x0364
#define PERI_CRG_INTR_CLEAR_GPLL_INTR_CLEAR_PLL_INI_ERR_GPLL_SHIFT                     0
#define PERI_CRG_INTR_CLEAR_GPLL_INTR_CLEAR_PLL_INI_ERR_GPLL_MASK                      0x00000001
#define PERI_CRG_INTR_CLEAR_GPLL_INTR_CLEAR_PLL_UNLOCK_GPLL_SHIFT                      1
#define PERI_CRG_INTR_CLEAR_GPLL_INTR_CLEAR_PLL_UNLOCK_GPLL_MASK                       0x00000002
#define PERI_CRG_INTR_CLEAR_GPLL_INTR_CLEAR_VOTE_REQ_ERR_GPLL_SHIFT                    2
#define PERI_CRG_INTR_CLEAR_GPLL_INTR_CLEAR_VOTE_REQ_ERR_GPLL_MASK                     0x00000004
#define PERI_CRG_INTR_MASK_PPLL0                   0x0368
#define PERI_CRG_INTR_MASK_PPLL0_INTR_MASK_PLL_INI_ERR_PPLL0_SHIFT                     0
#define PERI_CRG_INTR_MASK_PPLL0_INTR_MASK_PLL_INI_ERR_PPLL0_MASK                      0x00000001
#define PERI_CRG_INTR_MASK_PPLL0_INTR_MASK_PLL_UNLOCK_PPLL0_SHIFT                      1
#define PERI_CRG_INTR_MASK_PPLL0_INTR_MASK_PLL_UNLOCK_PPLL0_MASK                       0x00000002
#define PERI_CRG_INTR_MASK_PPLL0_INTR_MASK_VOTE_REQ_ERR_PPLL0_SHIFT                    2
#define PERI_CRG_INTR_MASK_PPLL0_INTR_MASK_VOTE_REQ_ERR_PPLL0_MASK                     0x00000004
#define PERI_CRG_INTR_CLEAR_PPLL0                  0x036c
#define PERI_CRG_INTR_CLEAR_PPLL0_INTR_CLEAR_PLL_INI_ERR_PPLL0_SHIFT                   0
#define PERI_CRG_INTR_CLEAR_PPLL0_INTR_CLEAR_PLL_INI_ERR_PPLL0_MASK                    0x00000001
#define PERI_CRG_INTR_CLEAR_PPLL0_INTR_CLEAR_PLL_UNLOCK_PPLL0_SHIFT                    1
#define PERI_CRG_INTR_CLEAR_PPLL0_INTR_CLEAR_PLL_UNLOCK_PPLL0_MASK                     0x00000002
#define PERI_CRG_INTR_CLEAR_PPLL0_INTR_CLEAR_VOTE_REQ_ERR_PPLL0_SHIFT                  2
#define PERI_CRG_INTR_CLEAR_PPLL0_INTR_CLEAR_VOTE_REQ_ERR_PPLL0_MASK                   0x00000004
#define PERI_CRG_INTR_MASK_PPLL1                   0x0370
#define PERI_CRG_INTR_MASK_PPLL1_INTR_MASK_PLL_INI_ERR_PPLL1_SHIFT                     0
#define PERI_CRG_INTR_MASK_PPLL1_INTR_MASK_PLL_INI_ERR_PPLL1_MASK                      0x00000001
#define PERI_CRG_INTR_MASK_PPLL1_INTR_MASK_PLL_UNLOCK_PPLL1_SHIFT                      1
#define PERI_CRG_INTR_MASK_PPLL1_INTR_MASK_PLL_UNLOCK_PPLL1_MASK                       0x00000002
#define PERI_CRG_INTR_MASK_PPLL1_INTR_MASK_VOTE_REQ_ERR_PPLL1_SHIFT                    2
#define PERI_CRG_INTR_MASK_PPLL1_INTR_MASK_VOTE_REQ_ERR_PPLL1_MASK                     0x00000004
#define PERI_CRG_INTR_CLEAR_PPLL1                  0x0374
#define PERI_CRG_INTR_CLEAR_PPLL1_INTR_CLEAR_PLL_INI_ERR_PPLL1_SHIFT                   0
#define PERI_CRG_INTR_CLEAR_PPLL1_INTR_CLEAR_PLL_INI_ERR_PPLL1_MASK                    0x00000001
#define PERI_CRG_INTR_CLEAR_PPLL1_INTR_CLEAR_PLL_UNLOCK_PPLL1_SHIFT                    1
#define PERI_CRG_INTR_CLEAR_PPLL1_INTR_CLEAR_PLL_UNLOCK_PPLL1_MASK                     0x00000002
#define PERI_CRG_INTR_CLEAR_PPLL1_INTR_CLEAR_VOTE_REQ_ERR_PPLL1_SHIFT                  2
#define PERI_CRG_INTR_CLEAR_PPLL1_INTR_CLEAR_VOTE_REQ_ERR_PPLL1_MASK                   0x00000004
#define PERI_CRG_INTR_MASK_PPLL2                   0x0378
#define PERI_CRG_INTR_MASK_PPLL2_INTR_MASK_PLL_INI_ERR_PPLL2_SHIFT                     0
#define PERI_CRG_INTR_MASK_PPLL2_INTR_MASK_PLL_INI_ERR_PPLL2_MASK                      0x00000001
#define PERI_CRG_INTR_MASK_PPLL2_INTR_MASK_PLL_UNLOCK_PPLL2_SHIFT                      1
#define PERI_CRG_INTR_MASK_PPLL2_INTR_MASK_PLL_UNLOCK_PPLL2_MASK                       0x00000002
#define PERI_CRG_INTR_MASK_PPLL2_INTR_MASK_VOTE_REQ_ERR_PPLL2_SHIFT                    2
#define PERI_CRG_INTR_MASK_PPLL2_INTR_MASK_VOTE_REQ_ERR_PPLL2_MASK                     0x00000004
#define PERI_CRG_INTR_CLEAR_PPLL2                  0x037c
#define PERI_CRG_INTR_CLEAR_PPLL2_INTR_CLEAR_PLL_INI_ERR_PPLL2_SHIFT                   0
#define PERI_CRG_INTR_CLEAR_PPLL2_INTR_CLEAR_PLL_INI_ERR_PPLL2_MASK                    0x00000001
#define PERI_CRG_INTR_CLEAR_PPLL2_INTR_CLEAR_PLL_UNLOCK_PPLL2_SHIFT                    1
#define PERI_CRG_INTR_CLEAR_PPLL2_INTR_CLEAR_PLL_UNLOCK_PPLL2_MASK                     0x00000002
#define PERI_CRG_INTR_CLEAR_PPLL2_INTR_CLEAR_VOTE_REQ_ERR_PPLL2_SHIFT                  2
#define PERI_CRG_INTR_CLEAR_PPLL2_INTR_CLEAR_VOTE_REQ_ERR_PPLL2_MASK                   0x00000004
#define PERI_CRG_INTR_MASK_PPLL3                   0x0380
#define PERI_CRG_INTR_MASK_PPLL3_INTR_MASK_PLL_INI_ERR_PPLL3_SHIFT                     0
#define PERI_CRG_INTR_MASK_PPLL3_INTR_MASK_PLL_INI_ERR_PPLL3_MASK                      0x00000001
#define PERI_CRG_INTR_MASK_PPLL3_INTR_MASK_PLL_UNLOCK_PPLL3_SHIFT                      1
#define PERI_CRG_INTR_MASK_PPLL3_INTR_MASK_PLL_UNLOCK_PPLL3_MASK                       0x00000002
#define PERI_CRG_INTR_MASK_PPLL3_INTR_MASK_VOTE_REQ_ERR_PPLL3_SHIFT                    2
#define PERI_CRG_INTR_MASK_PPLL3_INTR_MASK_VOTE_REQ_ERR_PPLL3_MASK                     0x00000004
#define PERI_CRG_INTR_CLEAR_PPLL3                  0x0384
#define PERI_CRG_INTR_CLEAR_PPLL3_INTR_CLEAR_PLL_INI_ERR_PPLL3_SHIFT                   0
#define PERI_CRG_INTR_CLEAR_PPLL3_INTR_CLEAR_PLL_INI_ERR_PPLL3_MASK                    0x00000001
#define PERI_CRG_INTR_CLEAR_PPLL3_INTR_CLEAR_PLL_UNLOCK_PPLL3_SHIFT                    1
#define PERI_CRG_INTR_CLEAR_PPLL3_INTR_CLEAR_PLL_UNLOCK_PPLL3_MASK                     0x00000002
#define PERI_CRG_INTR_CLEAR_PPLL3_INTR_CLEAR_VOTE_REQ_ERR_PPLL3_SHIFT                  2
#define PERI_CRG_INTR_CLEAR_PPLL3_INTR_CLEAR_VOTE_REQ_ERR_PPLL3_MASK                   0x00000004
#define PERI_CRG_INTR_MASK_DDR_PPLL0               0x0388
#define PERI_CRG_INTR_MASK_DDR_PPLL0_INTR_MASK_PLL_INI_ERR_DDR_PPLL0_SHIFT             0
#define PERI_CRG_INTR_MASK_DDR_PPLL0_INTR_MASK_PLL_INI_ERR_DDR_PPLL0_MASK              0x00000001
#define PERI_CRG_INTR_MASK_DDR_PPLL0_INTR_MASK_PLL_UNLOCK_DDR_PPLL0_SHIFT              1
#define PERI_CRG_INTR_MASK_DDR_PPLL0_INTR_MASK_PLL_UNLOCK_DDR_PPLL0_MASK               0x00000002
#define PERI_CRG_INTR_MASK_DDR_PPLL0_INTR_MASK_VOTE_REQ_ERR_DDR_PPLL0_SHIFT            2
#define PERI_CRG_INTR_MASK_DDR_PPLL0_INTR_MASK_VOTE_REQ_ERR_DDR_PPLL0_MASK             0x00000004
#define PERI_CRG_INTR_CLEAR_DDR_PPLL0              0x038c
#define PERI_CRG_INTR_CLEAR_DDR_PPLL0_INTR_CLEAR_PLL_INI_ERR_DDR_PPLL0_SHIFT           0
#define PERI_CRG_INTR_CLEAR_DDR_PPLL0_INTR_CLEAR_PLL_INI_ERR_DDR_PPLL0_MASK            0x00000001
#define PERI_CRG_INTR_CLEAR_DDR_PPLL0_INTR_CLEAR_PLL_UNLOCK_DDR_PPLL0_SHIFT            1
#define PERI_CRG_INTR_CLEAR_DDR_PPLL0_INTR_CLEAR_PLL_UNLOCK_DDR_PPLL0_MASK             0x00000002
#define PERI_CRG_INTR_CLEAR_DDR_PPLL0_INTR_CLEAR_VOTE_REQ_ERR_DDR_PPLL0_SHIFT          2
#define PERI_CRG_INTR_CLEAR_DDR_PPLL0_INTR_CLEAR_VOTE_REQ_ERR_DDR_PPLL0_MASK           0x00000004
#define PERI_CRG_INTR_MASK_DDR_PPLL1               0x0390
#define PERI_CRG_INTR_MASK_DDR_PPLL1_INTR_MASK_PLL_INI_ERR_DDR_PPLL1_SHIFT             0
#define PERI_CRG_INTR_MASK_DDR_PPLL1_INTR_MASK_PLL_INI_ERR_DDR_PPLL1_MASK              0x00000001
#define PERI_CRG_INTR_MASK_DDR_PPLL1_INTR_MASK_PLL_UNLOCK_DDR_PPLL1_SHIFT              1
#define PERI_CRG_INTR_MASK_DDR_PPLL1_INTR_MASK_PLL_UNLOCK_DDR_PPLL1_MASK               0x00000002
#define PERI_CRG_INTR_MASK_DDR_PPLL1_INTR_MASK_VOTE_REQ_ERR_DDR_PPLL1_SHIFT            2
#define PERI_CRG_INTR_MASK_DDR_PPLL1_INTR_MASK_VOTE_REQ_ERR_DDR_PPLL1_MASK             0x00000004
#define PERI_CRG_INTR_CLEAR_DDR_PPLL1              0x0394
#define PERI_CRG_INTR_CLEAR_DDR_PPLL1_INTR_CLEAR_PLL_INI_ERR_DDR_PPLL1_SHIFT           0
#define PERI_CRG_INTR_CLEAR_DDR_PPLL1_INTR_CLEAR_PLL_INI_ERR_DDR_PPLL1_MASK            0x00000001
#define PERI_CRG_INTR_CLEAR_DDR_PPLL1_INTR_CLEAR_PLL_UNLOCK_DDR_PPLL1_SHIFT            1
#define PERI_CRG_INTR_CLEAR_DDR_PPLL1_INTR_CLEAR_PLL_UNLOCK_DDR_PPLL1_MASK             0x00000002
#define PERI_CRG_INTR_CLEAR_DDR_PPLL1_INTR_CLEAR_VOTE_REQ_ERR_DDR_PPLL1_SHIFT          2
#define PERI_CRG_INTR_CLEAR_DDR_PPLL1_INTR_CLEAR_VOTE_REQ_ERR_DDR_PPLL1_MASK           0x00000004
#define PERI_CRG_INTR_STATUS_PLL                   0x0398
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_GPLL_SHIFT                    0
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_GPLL_MASK                     0x00000001
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_GPLL_SHIFT                     1
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_GPLL_MASK                      0x00000002
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_GPLL_SHIFT                   2
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_GPLL_MASK                    0x00000004
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_PPLL0_SHIFT                   3
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_PPLL0_MASK                    0x00000008
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_PPLL0_SHIFT                    4
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_PPLL0_MASK                     0x00000010
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_PPLL0_SHIFT                  5
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_PPLL0_MASK                   0x00000020
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_PPLL1_SHIFT                   6
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_PPLL1_MASK                    0x00000040
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_PPLL1_SHIFT                    7
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_PPLL1_MASK                     0x00000080
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_PPLL1_SHIFT                  8
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_PPLL1_MASK                   0x00000100
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_PPLL2_SHIFT                   9
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_PPLL2_MASK                    0x00000200
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_PPLL2_SHIFT                    10
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_PPLL2_MASK                     0x00000400
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_PPLL2_SHIFT                  11
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_PPLL2_MASK                   0x00000800
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_PPLL3_SHIFT                   12
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_PPLL3_MASK                    0x00001000
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_PPLL3_SHIFT                    13
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_PPLL3_MASK                     0x00002000
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_PPLL3_SHIFT                  14
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_PPLL3_MASK                   0x00004000
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_DDR_PPLL0_SHIFT               15
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_DDR_PPLL0_MASK                0x00008000
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_DDR_PPLL0_SHIFT                16
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_DDR_PPLL0_MASK                 0x00010000
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_DDR_PPLL0_SHIFT              17
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_DDR_PPLL0_MASK               0x00020000
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_DDR_PPLL1_SHIFT               18
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_DDR_PPLL1_MASK                0x00040000
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_DDR_PPLL1_SHIFT                19
#define PERI_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_DDR_PPLL1_MASK                 0x00080000
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_DDR_PPLL1_SHIFT              20
#define PERI_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_DDR_PPLL1_MASK               0x00100000
#define PERI_CRG_INTR_STATUS_PLL_MSK               0x039c
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_GPLL_SHIFT            0
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_GPLL_MASK             0x00000001
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_GPLL_SHIFT             1
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_GPLL_MASK              0x00000002
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_GPLL_SHIFT           2
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_GPLL_MASK            0x00000004
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_PPLL0_SHIFT           3
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_PPLL0_MASK            0x00000008
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_PPLL0_SHIFT            4
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_PPLL0_MASK             0x00000010
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_PPLL0_SHIFT          5
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_PPLL0_MASK           0x00000020
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_PPLL1_SHIFT           6
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_PPLL1_MASK            0x00000040
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_PPLL1_SHIFT            7
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_PPLL1_MASK             0x00000080
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_PPLL1_SHIFT          8
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_PPLL1_MASK           0x00000100
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_PPLL2_SHIFT           9
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_PPLL2_MASK            0x00000200
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_PPLL2_SHIFT            10
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_PPLL2_MASK             0x00000400
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_PPLL2_SHIFT          11
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_PPLL2_MASK           0x00000800
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_PPLL3_SHIFT           12
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_PPLL3_MASK            0x00001000
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_PPLL3_SHIFT            13
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_PPLL3_MASK             0x00002000
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_PPLL3_SHIFT          14
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_PPLL3_MASK           0x00004000
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_DDR_PPLL0_SHIFT       15
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_DDR_PPLL0_MASK        0x00008000
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_DDR_PPLL0_SHIFT        16
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_DDR_PPLL0_MASK         0x00010000
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_DDR_PPLL0_SHIFT      17
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_DDR_PPLL0_MASK       0x00020000
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_DDR_PPLL1_SHIFT       18
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_DDR_PPLL1_MASK        0x00040000
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_DDR_PPLL1_SHIFT        19
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_DDR_PPLL1_MASK         0x00080000
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_DDR_PPLL1_SHIFT      20
#define PERI_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_DDR_PPLL1_MASK       0x00100000
#define PERI_CRG_GPLL_VOTE_EN                      0x0400
#define PERI_CRG_GPLL_VOTE_EN_GPLL_VOTE_EN_SHIFT                                       0
#define PERI_CRG_GPLL_VOTE_EN_GPLL_VOTE_EN_MASK                                        0x000000ff
#define PERI_CRG_PPLL0_VOTE_EN                     0x0410
#define PERI_CRG_PPLL0_VOTE_EN_PPLL0_VOTE_EN_SHIFT                                     0
#define PERI_CRG_PPLL0_VOTE_EN_PPLL0_VOTE_EN_MASK                                      0x000000ff
#define PERI_CRG_PPLL1_VOTE_EN                     0x0420
#define PERI_CRG_PPLL1_VOTE_EN_PPLL1_VOTE_EN_SHIFT                                     0
#define PERI_CRG_PPLL1_VOTE_EN_PPLL1_VOTE_EN_MASK                                      0x000000ff
#define PERI_CRG_PPLL2_VOTE_EN                     0x0430
#define PERI_CRG_PPLL2_VOTE_EN_PPLL2_VOTE_EN_SHIFT                                     0
#define PERI_CRG_PPLL2_VOTE_EN_PPLL2_VOTE_EN_MASK                                      0x000000ff
#define PERI_CRG_PPLL3_VOTE_EN                     0x0440
#define PERI_CRG_PPLL3_VOTE_EN_PPLL3_VOTE_EN_SHIFT                                     0
#define PERI_CRG_PPLL3_VOTE_EN_PPLL3_VOTE_EN_MASK                                      0x000000ff
#define PERI_CRG_DDR_PPLL0_VOTE_EN                 0x0450
#define PERI_CRG_DDR_PPLL0_VOTE_EN_DDR_PPLL0_VOTE_EN_SHIFT                             0
#define PERI_CRG_DDR_PPLL0_VOTE_EN_DDR_PPLL0_VOTE_EN_MASK                              0x000000ff
#define PERI_CRG_DDR_PPLL1_VOTE_EN                 0x0460
#define PERI_CRG_DDR_PPLL1_VOTE_EN_DDR_PPLL1_VOTE_EN_SHIFT                             0
#define PERI_CRG_DDR_PPLL1_VOTE_EN_DDR_PPLL1_VOTE_EN_MASK                              0x000000ff
#define PERI_CRG_CLKDIV25                          0x0500
#define PERI_CRG_CLKDIV25_DIV_CLK_DDRPPLL1_TEST_SHIFT                                  0
#define PERI_CRG_CLKDIV25_DIV_CLK_DDRPPLL1_TEST_MASK                                   0x0000003f
#define PERI_CRG_CLKDIV25_DIV_CLK_DDRPPLL0_TEST_SHIFT                                  6
#define PERI_CRG_CLKDIV25_DIV_CLK_DDRPPLL0_TEST_MASK                                   0x00000fc0
#define PERI_CRG_CLKDIV25_SC_GT_CLK_DDRPPLL1_TEST_SHIFT                                12
#define PERI_CRG_CLKDIV25_SC_GT_CLK_DDRPPLL1_TEST_MASK                                 0x00001000
#define PERI_CRG_CLKDIV25_SC_GT_CLK_DDRPPLL0_TEST_SHIFT                                13
#define PERI_CRG_CLKDIV25_SC_GT_CLK_DDRPPLL0_TEST_MASK                                 0x00002000
#define PERI_CRG_CLKDIV26                          0x0504
#define PERI_CRG_CLKDIV26_DIV_CLK_GPLL_TEST_SHIFT                                      0
#define PERI_CRG_CLKDIV26_DIV_CLK_GPLL_TEST_MASK                                       0x0000003f
#define PERI_CRG_CLKDIV26_DIV_CLK_DDR_DFICLK_GPLL_SHIFT                                6
#define PERI_CRG_CLKDIV26_DIV_CLK_DDR_DFICLK_GPLL_MASK                                 0x00000fc0
#define PERI_CRG_CLKDIV26_SC_GT_CLK_GPLL_TEST_SHIFT                                    12
#define PERI_CRG_CLKDIV26_SC_GT_CLK_GPLL_TEST_MASK                                     0x00001000
#define PERI_CRG_CLKDIV26_SC_GT_CLK_DDR_DFICLK_GPLL_SHIFT                              13
#define PERI_CRG_CLKDIV26_SC_GT_CLK_DDR_DFICLK_GPLL_MASK                               0x00002000
#define PERI_CRG_CLKDIV37                          0x0508
#define PERI_CRG_CLKDIV37_DIV_CLK_PPLL1_TEST_SHIFT                                     0
#define PERI_CRG_CLKDIV37_DIV_CLK_PPLL1_TEST_MASK                                      0x0000003f
#define PERI_CRG_CLKDIV37_DIV_CLK_PPLL0_TEST_SHIFT                                     6
#define PERI_CRG_CLKDIV37_DIV_CLK_PPLL0_TEST_MASK                                      0x00000fc0
#define PERI_CRG_CLKDIV37_SC_GT_CLK_PPLL1_TEST_SHIFT                                   12
#define PERI_CRG_CLKDIV37_SC_GT_CLK_PPLL1_TEST_MASK                                    0x00001000
#define PERI_CRG_CLKDIV37_SC_GT_CLK_PPLL0_TEST_SHIFT                                   13
#define PERI_CRG_CLKDIV37_SC_GT_CLK_PPLL0_TEST_MASK                                    0x00002000
#define PERI_CRG_CLKDIV38                          0x050c
#define PERI_CRG_CLKDIV38_DIV_CLK_PPLL3_TEST_SHIFT                                     0
#define PERI_CRG_CLKDIV38_DIV_CLK_PPLL3_TEST_MASK                                      0x0000003f
#define PERI_CRG_CLKDIV38_DIV_CLK_PPLL2_TEST_SHIFT                                     6
#define PERI_CRG_CLKDIV38_DIV_CLK_PPLL2_TEST_MASK                                      0x00000fc0
#define PERI_CRG_CLKDIV38_SC_GT_CLK_PPLL3_TEST_SHIFT                                   12
#define PERI_CRG_CLKDIV38_SC_GT_CLK_PPLL3_TEST_MASK                                    0x00001000
#define PERI_CRG_CLKDIV38_SC_GT_CLK_PPLL2_TEST_SHIFT                                   13
#define PERI_CRG_CLKDIV38_SC_GT_CLK_PPLL2_TEST_MASK                                    0x00002000
#define PERI_CRG_AUTOFSCTRL22                      0x0600
#define PERI_CRG_AUTOFSCTRL22_AUTOFS_BYPASS_DMA_BUS_SHIFT                              0
#define PERI_CRG_AUTOFSCTRL22_AUTOFS_BYPASS_DMA_BUS_MASK                               0x0000ffff
#define PERI_CRG_AUTOFSCTRL23                      0x0604
#define PERI_CRG_AUTOFSCTRL23_AUTOFS_BYPASS_SYSBUS_SHIFT                               0
#define PERI_CRG_AUTOFSCTRL23_AUTOFS_BYPASS_SYSBUS_MASK                                0x0000ffff
#define PERI_CRG_AUTOFSCTRL24                      0x0608
#define PERI_CRG_AUTOFSCTRL24_AUTOFS_BYPASS_CFGBUS_SHIFT                               0
#define PERI_CRG_AUTOFSCTRL24_AUTOFS_BYPASS_CFGBUS_MASK                                0x0000ffff
#define PERI_CRG_AUTOFSCTRL25                      0x060c
#define PERI_CRG_AUTOFSCTRL25_AUTOFS_BYPASS_XCTRL_CPU_SHIFT                            0
#define PERI_CRG_AUTOFSCTRL25_AUTOFS_BYPASS_XCTRL_CPU_MASK                             0x0000ffff
#define PERI_CRG_AUTOFSCTRL26                      0x0610
#define PERI_CRG_AUTOFSCTRL26_AUTOFS_BYPASS_XCTRL_DDR_SHIFT                            0
#define PERI_CRG_AUTOFSCTRL26_AUTOFS_BYPASS_XCTRL_DDR_MASK                             0x0000ffff
#define PERI_CRG_AUTOFSCTRL27                      0x0614
#define PERI_CRG_AUTOFSCTRL27_AUTOFS_BYPASS_MAINBUS_SHIFT                              0
#define PERI_CRG_AUTOFSCTRL27_AUTOFS_BYPASS_MAINBUS_MASK                               0x0000ffff
#define PERI_CRG_AUTOFSCTRL28                      0x0618
#define PERI_CRG_AUTOFSCTRL28_AUTOFS_BYPASS_OCM_SHIFT                                  0
#define PERI_CRG_AUTOFSCTRL28_AUTOFS_BYPASS_OCM_MASK                                   0x0000ffff
#define PERI_CRG_AUTOFSCTRL29                      0x061c
#define PERI_CRG_AUTOFSCTRL29_AUTOFS_BYPASS_CPU_BUS_SHIFT                              0
#define PERI_CRG_AUTOFSCTRL29_AUTOFS_BYPASS_CPU_BUS_MASK                               0x0000ffff
#define PERI_CRG_PERICTRL7                         0x0650
#define PERI_CRG_PERICTRL7_SEL_PERI_PLL_TEST_SHIFT                                     0
#define PERI_CRG_PERICTRL7_SEL_PERI_PLL_TEST_MASK                                      0x0000001f
#define PERI_CRG_PERICTRL7_SEL_CLK_PERI_PLL_TEST_1ST_SHIFT                             5
#define PERI_CRG_PERICTRL7_SEL_CLK_PERI_PLL_TEST_1ST_MASK                              0x00000060
#define PERI_CRG_PERICTRL7_SEL_CLK_PERI_PLL_TEST_2ND_SHIFT                             7
#define PERI_CRG_PERICTRL7_SEL_CLK_PERI_PLL_TEST_2ND_MASK                              0x00000180
#define PERI_CRG_PERISTAT7                         0x0700
#define PERI_CRG_PERICTRL8                         0x0710
#define PERI_CRG_SYSCFG_BUS_TIMEOUT_STATUS         0x0720
#define PERI_CRG_SYSCFG_BUS_TIMEOUT_STATUS_SYSCFG_BUS_TGT_TIMEOUT_SHIFT                1
#define PERI_CRG_SYSCFG_BUS_TIMEOUT_STATUS_SYSCFG_BUS_TGT_TIMEOUT_MASK                 0x007ffffe
#define PERI_CRG_SYSCFG_BUS_TRANS_STATUS           0x0724
#define PERI_CRG_SYSCFG_BUS_TRANS_STATUS_SYSCFG_BUS_INI_NO_PENDING_TRANS_SHIFT         0
#define PERI_CRG_SYSCFG_BUS_TRANS_STATUS_SYSCFG_BUS_INI_NO_PENDING_TRANS_MASK          0x000003ff
#define PERI_CRG_SYSCFG_BUS_TRANS_STATUS_SYSCFG_BUS_TGT_NO_PENDING_TRANS_SHIFT         10
#define PERI_CRG_SYSCFG_BUS_TRANS_STATUS_SYSCFG_BUS_TGT_NO_PENDING_TRANS_MASK          0xfffffc00
#define PERI_CRG_PERICFG_BUS_STATUS                0x0728
#define PERI_CRG_PERICFG_BUS_STATUS_PERICFG_BUS_INI_NO_PENDING_TRANS_SHIFT             1
#define PERI_CRG_PERICFG_BUS_STATUS_PERICFG_BUS_INI_NO_PENDING_TRANS_MASK              0x00000002
#define PERI_CRG_PERICFG_BUS_STATUS_PERICFG_BUS_TGT_NO_PENDING_TRANS_SHIFT             2
#define PERI_CRG_PERICFG_BUS_STATUS_PERICFG_BUS_TGT_NO_PENDING_TRANS_MASK              0x0000001c
#define PERI_CRG_PERICFG_BUS_STATUS_PERICFG_BUS_TGT_TIMEOUT_SHIFT                      5
#define PERI_CRG_PERICFG_BUS_STATUS_PERICFG_BUS_TGT_TIMEOUT_MASK                       0x000000e0
#define PERI_CRG_PERICFG_BUS_STATUS_PERICFG_BUS_NO_PENDING_TRANS_SHIFT                 14
#define PERI_CRG_PERICFG_BUS_STATUS_PERICFG_BUS_NO_PENDING_TRANS_MASK                  0x00004000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0            0x072c
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_XCTRLDDR_SLV_IDLEACK_SHIFT     0
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_XCTRLDDR_SLV_IDLEACK_MASK      0x00000001
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_XCTRLDDR_MST_IDLEACK_SHIFT     1
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_XCTRLDDR_MST_IDLEACK_MASK      0x00000002
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_NPU_SLV_IDLEACK_SHIFT          2
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_NPU_SLV_IDLEACK_MASK           0x00000004
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_NPU_MST_IDLEACK_SHIFT          3
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_NPU_MST_IDLEACK_MASK           0x00000008
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M85_SLV_IDLEACK_SHIFT          4
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M85_SLV_IDLEACK_MASK           0x00000010
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M2_SLV_IDLEACK_SHIFT           5
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M2_SLV_IDLEACK_MASK            0x00000020
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M2_MST_IDLEACK_SHIFT           6
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M2_MST_IDLEACK_MASK            0x00000040
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_LPIS_SLV_IDLEACK_SHIFT         7
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_LPIS_SLV_IDLEACK_MASK          0x00000080
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_LPIS_MST_IDLEACK_SHIFT         8
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_LPIS_MST_IDLEACK_MASK          0x00000100
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_HS2_SLV_IDLEACK_SHIFT          9
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_HS2_SLV_IDLEACK_MASK           0x00000200
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_HS1_SLV_IDLEACK_SHIFT          11
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_HS1_SLV_IDLEACK_MASK           0x00000800
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_CPU_SLV_IDLEACK_SHIFT          13
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_CPU_SLV_IDLEACK_MASK           0x00002000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_CPU_MST_IDLEACK_SHIFT          14
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_CPU_MST_IDLEACK_MASK           0x00004000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_PERICFG_IDLEACK_SHIFT          15
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_PERICFG_IDLEACK_MASK           0x00008000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_XCTRLDDR_SLV_IDLE_SHIFT        16
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_XCTRLDDR_SLV_IDLE_MASK         0x00010000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_XCTRLDDR_MST_IDLE_SHIFT        17
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_XCTRLDDR_MST_IDLE_MASK         0x00020000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_NPU_SLV_IDLE_SHIFT             18
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_NPU_SLV_IDLE_MASK              0x00040000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_NPU_MST_IDLE_SHIFT             19
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_NPU_MST_IDLE_MASK              0x00080000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M85_SLV_IDLE_SHIFT             20
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M85_SLV_IDLE_MASK              0x00100000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M2_SLV_IDLE_SHIFT              21
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M2_SLV_IDLE_MASK               0x00200000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M2_MST_IDLE_SHIFT              22
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_M2_MST_IDLE_MASK               0x00400000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_LPIS_SLV_IDLE_SHIFT            23
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_LPIS_SLV_IDLE_MASK             0x00800000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_LPIS_MST_IDLE_SHIFT            24
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_LPIS_MST_IDLE_MASK             0x01000000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_HS2_SLV_IDLE_SHIFT             25
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_HS2_SLV_IDLE_MASK              0x02000000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_HS1_SLV_IDLE_SHIFT             27
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_HS1_SLV_IDLE_MASK              0x08000000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_CPU_SLV_IDLE_SHIFT             29
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_CPU_SLV_IDLE_MASK              0x20000000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_CPU_MST_IDLE_SHIFT             30
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_CPU_MST_IDLE_MASK              0x40000000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_PERICFG_IDLE_SHIFT             31
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_0_SYSCFG_BUS_PDOM_PERICFG_IDLE_MASK              0x80000000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1            0x0730
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DBG_SLV_IDLEACK_SHIFT          0
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DBG_SLV_IDLEACK_MASK           0x00000001
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DBG_MST_IDLEACK_SHIFT          1
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DBG_MST_IDLEACK_MASK           0x00000002
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_M1_IDLEACK_SHIFT               2
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_M1_IDLEACK_MASK                0x00000004
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_GPU_AXI_IDLEACK_SHIFT          3
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_GPU_AXI_IDLEACK_MASK           0x00000008
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_GPU_APB_IDLEACK_SHIFT          4
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_GPU_APB_IDLEACK_MASK           0x00000010
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DMA_SLV_IDLEACK_SHIFT          5
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DMA_SLV_IDLEACK_MASK           0x00000020
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DMA_MST_IDLEACK_SHIFT          6
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DMA_MST_IDLEACK_MASK           0x00000040
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DBG_SLV_IDLE_SHIFT             7
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DBG_SLV_IDLE_MASK              0x00000080
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DBG_MST_IDLE_SHIFT             8
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DBG_MST_IDLE_MASK              0x00000100
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_M1_IDLE_SHIFT                  9
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_M1_IDLE_MASK                   0x00000200
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_GPU_AXI_IDLE_SHIFT             10
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_GPU_AXI_IDLE_MASK              0x00000400
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_GPU_APB_IDLE_SHIFT             11
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_GPU_APB_IDLE_MASK              0x00000800
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DMA_SLV_IDLE_SHIFT             12
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DMA_SLV_IDLE_MASK              0x00001000
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DMA_MST_IDLE_SHIFT             13
#define PERI_CRG_SYSCFG_BUS_PDOM_IDLE_1_SYSCFG_BUS_PDOM_DMA_MST_IDLE_MASK              0x00002000
#define PERI_CRG_HSS_SYS_PDOM_IDLE                 0x0734
#define PERI_CRG_HSS_SYS_PDOM_IDLE_SYSCFG_BUS_PDOM_HS1_MST_IDLEACK_SHIFT               0
#define PERI_CRG_HSS_SYS_PDOM_IDLE_SYSCFG_BUS_PDOM_HS1_MST_IDLEACK_MASK                0x00000001
#define PERI_CRG_HSS_SYS_PDOM_IDLE_SYSCFG_BUS_PDOM_HS2_MST_IDLEACK_SHIFT               1
#define PERI_CRG_HSS_SYS_PDOM_IDLE_SYSCFG_BUS_PDOM_HS2_MST_IDLEACK_MASK                0x00000002
#define PERI_CRG_HSS_SYS_PDOM_IDLE_SYSCFG_BUS_PDOM_HS1_MST_IDLE_SHIFT                  6
#define PERI_CRG_HSS_SYS_PDOM_IDLE_SYSCFG_BUS_PDOM_HS1_MST_IDLE_MASK                   0x00000040
#define PERI_CRG_HSS_SYS_PDOM_IDLE_SYSCFG_BUS_PDOM_HS2_MST_IDLE_SHIFT                  7
#define PERI_CRG_HSS_SYS_PDOM_IDLE_SYSCFG_BUS_PDOM_HS2_MST_IDLE_MASK                   0x00000080
#define PERI_CRG_PERISTAT8                         0x0738
#define PERI_CRG_PERISTAT8_IDLE_FLAG_PERFSTAT_PERI_SHIFT                               0
#define PERI_CRG_PERISTAT8_IDLE_FLAG_PERFSTAT_PERI_MASK                                0x00000001
#define PERI_CRG_PERISTAT8_IDLE_FLAG_PERFSTAT_GPU_SHIFT                                1
#define PERI_CRG_PERISTAT8_IDLE_FLAG_PERFSTAT_GPU_MASK                                 0x00000002
#define PERI_CRG_PERISTAT8_IDLE_FLAG_PERFSTAT_CPU_SHIFT                                2
#define PERI_CRG_PERISTAT8_IDLE_FLAG_PERFSTAT_CPU_MASK                                 0x00000004
#define PERI_CRG_PERISTAT8_CPU_WFX_SHIFT                                               3
#define PERI_CRG_PERISTAT8_CPU_WFX_MASK                                                0x00000008
#define PERI_CRG_PERISTAT8_CPU_FAST_WAKEUP_SHIFT                                       4
#define PERI_CRG_PERISTAT8_CPU_FAST_WAKEUP_MASK                                        0x000000f0
#define PERI_CRG_PERISTAT8_DPU_IDLE_FOR_OCM_SHIFT                                      8
#define PERI_CRG_PERISTAT8_DPU_IDLE_FOR_OCM_MASK                                       0x00000100
#define PERI_CRG_PERISTAT8_IDLE_FLAG_OCM_NIC_SHIFT                                     9
#define PERI_CRG_PERISTAT8_IDLE_FLAG_OCM_NIC_MASK                                      0x00000200
#define PERI_CRG_PERISTAT8_IDLE_FLAG_NDB_MST_M2_SHIFT                                  10
#define PERI_CRG_PERISTAT8_IDLE_FLAG_NDB_MST_M2_MASK                                   0x00000400
#define PERI_CRG_PERISTAT8_IDLE_FLAG_NDB_SLV_M2_SHIFT                                  11
#define PERI_CRG_PERISTAT8_IDLE_FLAG_NDB_SLV_M2_MASK                                   0x00000800
#define PERI_CRG_PERISTAT8_IDLE_FLAG_NDB_MST_M1_SHIFT                                  12
#define PERI_CRG_PERISTAT8_IDLE_FLAG_NDB_MST_M1_MASK                                   0x00001000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_NDB_SLV_M1_SHIFT                                  13
#define PERI_CRG_PERISTAT8_IDLE_FLAG_NDB_SLV_M1_MASK                                   0x00002000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_ADB_MST_NPU_SHIFT                                 14
#define PERI_CRG_PERISTAT8_IDLE_FLAG_ADB_MST_NPU_MASK                                  0x00004000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_ADB_SLV_NPU_SHIFT                                 15
#define PERI_CRG_PERISTAT8_IDLE_FLAG_ADB_SLV_NPU_MASK                                  0x00008000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_ADB_MST_GPU_SHIFT                                 16
#define PERI_CRG_PERISTAT8_IDLE_FLAG_ADB_MST_GPU_MASK                                  0x00010000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_ADB_SLV_GPU_SHIFT                                 17
#define PERI_CRG_PERISTAT8_IDLE_FLAG_ADB_SLV_GPU_MASK                                  0x00020000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_MAIN_ADB_MST_CPU_SHIFT                            18
#define PERI_CRG_PERISTAT8_IDLE_FLAG_MAIN_ADB_MST_CPU_MASK                             0x00040000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_MAIN_ADB_SLV_CPU_SHIFT                            19
#define PERI_CRG_PERISTAT8_IDLE_FLAG_MAIN_ADB_SLV_CPU_MASK                             0x00080000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_XCTRL_DDR_M3_SHIFT                                20
#define PERI_CRG_PERISTAT8_IDLE_FLAG_XCTRL_DDR_M3_MASK                                 0x00100000
#define PERI_CRG_PERISTAT8_XCTRL_DDR_BUS_NO_PENDING_TRANS_SHIFT                        21
#define PERI_CRG_PERISTAT8_XCTRL_DDR_BUS_NO_PENDING_TRANS_MASK                         0x00200000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_XCTRL_CPU_M3_SHIFT                                22
#define PERI_CRG_PERISTAT8_IDLE_FLAG_XCTRL_CPU_M3_MASK                                 0x00400000
#define PERI_CRG_PERISTAT8_XCTRL_CPU_BUS_NO_PENDING_TRANS_SHIFT                        23
#define PERI_CRG_PERISTAT8_XCTRL_CPU_BUS_NO_PENDING_TRANS_MASK                         0x00800000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_NPU_NIC2CFGBUS_SHIFT                              24
#define PERI_CRG_PERISTAT8_IDLE_FLAG_NPU_NIC2CFGBUS_MASK                               0x01000000
#define PERI_CRG_PERISTAT8_IDLE_FLAG_CFGBUS2OCM_NIC_SHIFT                              25
#define PERI_CRG_PERISTAT8_IDLE_FLAG_CFGBUS2OCM_NIC_MASK                               0x02000000
#define PERI_CRG_PERISTAT8_PERI_DMAC_BUSY_NS_SHIFT                                     27
#define PERI_CRG_PERISTAT8_PERI_DMAC_BUSY_NS_MASK                                      0x08000000
#define PERI_CRG_PERISTAT8_PERI_DMAC_BUSY_S_SHIFT                                      28
#define PERI_CRG_PERISTAT8_PERI_DMAC_BUSY_S_MASK                                       0x10000000
#define PERI_CRG_CLKGT3_W1S                        0x0800
#define PERI_CRG_CLKGT3_W1S_GT_CLK_MAINBUS_DATA_SHIFT                                  0
#define PERI_CRG_CLKGT3_W1S_GT_CLK_MAINBUS_DATA_UP_SHIFT                               1
#define PERI_CRG_CLKGT3_W1S_GT_CLK_SYSBUS_DATA_SHIFT                                   4
#define PERI_CRG_CLKGT3_W1S_GT_CLK_SYSBUS_DATA_UP_SHIFT                                5
#define PERI_CRG_CLKGT3_W1S_GT_CLK_MAINBUS_DATA_DBG_SHIFT                              6
#define PERI_CRG_CLKGT3_W1S_GT_ACLK_DDR_ADB400_SLV_SHIFT                               7
#define PERI_CRG_CLKGT3_W1S_GT_CLK_SYSBUS_DATA_DMA_SHIFT                               8
#define PERI_CRG_CLKGT3_W1S_GT_CLK_CFGBUS_SHIFT                                        9
#define PERI_CRG_CLKGT3_W1S_GT_CLK_CFGBUS_UP_SHIFT                                     11
#define PERI_CRG_CLKGT3_W1S_GT_CLK_DDR_CFG_ADB400_SLV_SHIFT                            12
#define PERI_CRG_CLKGT3_W1S_GT_CLK_PERICFG_BUS_SHIFT                                   13
#define PERI_CRG_CLKGT3_W1S_GT_CLK_DEBUG_ATB_SHIFT                                     14
#define PERI_CRG_CLKGT3_W1S_GT_CLK_DEBUG_ATB_XCTRL_DDR_SHIFT                           15
#define PERI_CRG_CLKGT3_W1S_GT_CLK_CFGBUS_DEBUG_SHIFT                                  16
#define PERI_CRG_CLKGT3_W1S_GT_CLK_DEBUG_ATB_H_SHIFT                                   17
#define PERI_CRG_CLKGT3_W1S_GT_CLK_CFGBUS_DMA_SHIFT                                    18
#define PERI_CRG_CLKGT3_W1S_GT_CLK_CFGBUS_PERI_SHIFT                                   19
#define PERI_CRG_CLKGT3_W1S_GT_CLK_SYSBUS_DATA_DBG_SHIFT                               20
#define PERI_CRG_CLKGT3_W1S_GT_PCLK_PERI_TIMER_S_SHIFT                                 24
#define PERI_CRG_CLKGT3_W1S_GT_PCLK_PERI_WDT3_SHIFT                                    25
#define PERI_CRG_CLKGT3_W1S_GT_PCLK_PERI_WDT2_SHIFT                                    26
#define PERI_CRG_CLKGT3_W1C                        0x0804
#define PERI_CRG_CLKGT3_W1C_GT_CLK_MAINBUS_DATA_SHIFT                                  0
#define PERI_CRG_CLKGT3_W1C_GT_CLK_MAINBUS_DATA_UP_SHIFT                               1
#define PERI_CRG_CLKGT3_W1C_GT_CLK_SYSBUS_DATA_SHIFT                                   4
#define PERI_CRG_CLKGT3_W1C_GT_CLK_SYSBUS_DATA_UP_SHIFT                                5
#define PERI_CRG_CLKGT3_W1C_GT_CLK_MAINBUS_DATA_DBG_SHIFT                              6
#define PERI_CRG_CLKGT3_W1C_GT_ACLK_DDR_ADB400_SLV_SHIFT                               7
#define PERI_CRG_CLKGT3_W1C_GT_CLK_SYSBUS_DATA_DMA_SHIFT                               8
#define PERI_CRG_CLKGT3_W1C_GT_CLK_CFGBUS_SHIFT                                        9
#define PERI_CRG_CLKGT3_W1C_GT_CLK_CFGBUS_UP_SHIFT                                     11
#define PERI_CRG_CLKGT3_W1C_GT_CLK_DDR_CFG_ADB400_SLV_SHIFT                            12
#define PERI_CRG_CLKGT3_W1C_GT_CLK_PERICFG_BUS_SHIFT                                   13
#define PERI_CRG_CLKGT3_W1C_GT_CLK_DEBUG_ATB_SHIFT                                     14
#define PERI_CRG_CLKGT3_W1C_GT_CLK_DEBUG_ATB_XCTRL_DDR_SHIFT                           15
#define PERI_CRG_CLKGT3_W1C_GT_CLK_CFGBUS_DEBUG_SHIFT                                  16
#define PERI_CRG_CLKGT3_W1C_GT_CLK_DEBUG_ATB_H_SHIFT                                   17
#define PERI_CRG_CLKGT3_W1C_GT_CLK_CFGBUS_DMA_SHIFT                                    18
#define PERI_CRG_CLKGT3_W1C_GT_CLK_CFGBUS_PERI_SHIFT                                   19
#define PERI_CRG_CLKGT3_W1C_GT_CLK_SYSBUS_DATA_DBG_SHIFT                               20
#define PERI_CRG_CLKGT3_W1C_GT_PCLK_PERI_TIMER_S_SHIFT                                 24
#define PERI_CRG_CLKGT3_W1C_GT_PCLK_PERI_WDT3_SHIFT                                    25
#define PERI_CRG_CLKGT3_W1C_GT_PCLK_PERI_WDT2_SHIFT                                    26
#define PERI_CRG_CLKGT3_RO                         0x0808
#define PERI_CRG_CLKGT3_RO_GT_CLK_MAINBUS_DATA_SHIFT                                   0
#define PERI_CRG_CLKGT3_RO_GT_CLK_MAINBUS_DATA_MASK                                    0x00000001
#define PERI_CRG_CLKGT3_RO_GT_CLK_MAINBUS_DATA_UP_SHIFT                                1
#define PERI_CRG_CLKGT3_RO_GT_CLK_MAINBUS_DATA_UP_MASK                                 0x00000002
#define PERI_CRG_CLKGT3_RO_GT_CLK_SYSBUS_DATA_SHIFT                                    4
#define PERI_CRG_CLKGT3_RO_GT_CLK_SYSBUS_DATA_MASK                                     0x00000010
#define PERI_CRG_CLKGT3_RO_GT_CLK_SYSBUS_DATA_UP_SHIFT                                 5
#define PERI_CRG_CLKGT3_RO_GT_CLK_SYSBUS_DATA_UP_MASK                                  0x00000020
#define PERI_CRG_CLKGT3_RO_GT_CLK_MAINBUS_DATA_DBG_SHIFT                               6
#define PERI_CRG_CLKGT3_RO_GT_CLK_MAINBUS_DATA_DBG_MASK                                0x00000040
#define PERI_CRG_CLKGT3_RO_GT_ACLK_DDR_ADB400_SLV_SHIFT                                7
#define PERI_CRG_CLKGT3_RO_GT_ACLK_DDR_ADB400_SLV_MASK                                 0x00000080
#define PERI_CRG_CLKGT3_RO_GT_CLK_SYSBUS_DATA_DMA_SHIFT                                8
#define PERI_CRG_CLKGT3_RO_GT_CLK_SYSBUS_DATA_DMA_MASK                                 0x00000100
#define PERI_CRG_CLKGT3_RO_GT_CLK_CFGBUS_SHIFT                                         9
#define PERI_CRG_CLKGT3_RO_GT_CLK_CFGBUS_MASK                                          0x00000200
#define PERI_CRG_CLKGT3_RO_GT_CLK_CFGBUS_UP_SHIFT                                      11
#define PERI_CRG_CLKGT3_RO_GT_CLK_CFGBUS_UP_MASK                                       0x00000800
#define PERI_CRG_CLKGT3_RO_GT_CLK_DDR_CFG_ADB400_SLV_SHIFT                             12
#define PERI_CRG_CLKGT3_RO_GT_CLK_DDR_CFG_ADB400_SLV_MASK                              0x00001000
#define PERI_CRG_CLKGT3_RO_GT_CLK_PERICFG_BUS_SHIFT                                    13
#define PERI_CRG_CLKGT3_RO_GT_CLK_PERICFG_BUS_MASK                                     0x00002000
#define PERI_CRG_CLKGT3_RO_GT_CLK_DEBUG_ATB_SHIFT                                      14
#define PERI_CRG_CLKGT3_RO_GT_CLK_DEBUG_ATB_MASK                                       0x00004000
#define PERI_CRG_CLKGT3_RO_GT_CLK_DEBUG_ATB_XCTRL_DDR_SHIFT                            15
#define PERI_CRG_CLKGT3_RO_GT_CLK_DEBUG_ATB_XCTRL_DDR_MASK                             0x00008000
#define PERI_CRG_CLKGT3_RO_GT_CLK_CFGBUS_DEBUG_SHIFT                                   16
#define PERI_CRG_CLKGT3_RO_GT_CLK_CFGBUS_DEBUG_MASK                                    0x00010000
#define PERI_CRG_CLKGT3_RO_GT_CLK_DEBUG_ATB_H_SHIFT                                    17
#define PERI_CRG_CLKGT3_RO_GT_CLK_DEBUG_ATB_H_MASK                                     0x00020000
#define PERI_CRG_CLKGT3_RO_GT_CLK_CFGBUS_DMA_SHIFT                                     18
#define PERI_CRG_CLKGT3_RO_GT_CLK_CFGBUS_DMA_MASK                                      0x00040000
#define PERI_CRG_CLKGT3_RO_GT_CLK_CFGBUS_PERI_SHIFT                                    19
#define PERI_CRG_CLKGT3_RO_GT_CLK_CFGBUS_PERI_MASK                                     0x00080000
#define PERI_CRG_CLKGT3_RO_GT_CLK_SYSBUS_DATA_DBG_SHIFT                                20
#define PERI_CRG_CLKGT3_RO_GT_CLK_SYSBUS_DATA_DBG_MASK                                 0x00100000
#define PERI_CRG_CLKGT3_RO_GT_PCLK_PERI_TIMER_S_SHIFT                                  24
#define PERI_CRG_CLKGT3_RO_GT_PCLK_PERI_TIMER_S_MASK                                   0x01000000
#define PERI_CRG_CLKGT3_RO_GT_PCLK_PERI_WDT3_SHIFT                                     25
#define PERI_CRG_CLKGT3_RO_GT_PCLK_PERI_WDT3_MASK                                      0x02000000
#define PERI_CRG_CLKGT3_RO_GT_PCLK_PERI_WDT2_SHIFT                                     26
#define PERI_CRG_CLKGT3_RO_GT_PCLK_PERI_WDT2_MASK                                      0x04000000
#define PERI_CRG_CLKST3                            0x080c
#define PERI_CRG_CLKST3_ST_CLK_MAINBUS_DATA_SHIFT                                      0
#define PERI_CRG_CLKST3_ST_CLK_MAINBUS_DATA_MASK                                       0x00000001
#define PERI_CRG_CLKST3_ST_CLK_MAINBUS_DATA_UP_SHIFT                                   1
#define PERI_CRG_CLKST3_ST_CLK_MAINBUS_DATA_UP_MASK                                    0x00000002
#define PERI_CRG_CLKST3_ST_CLK_SYSBUS_DATA_SHIFT                                       4
#define PERI_CRG_CLKST3_ST_CLK_SYSBUS_DATA_MASK                                        0x00000010
#define PERI_CRG_CLKST3_ST_CLK_SYSBUS_DATA_UP_SHIFT                                    5
#define PERI_CRG_CLKST3_ST_CLK_SYSBUS_DATA_UP_MASK                                     0x00000020
#define PERI_CRG_CLKST3_ST_CLK_MAINBUS_DATA_DBG_SHIFT                                  6
#define PERI_CRG_CLKST3_ST_CLK_MAINBUS_DATA_DBG_MASK                                   0x00000040
#define PERI_CRG_CLKST3_ST_ACLK_DDR_ADB400_SLV_DDRA_SHIFT                              7
#define PERI_CRG_CLKST3_ST_ACLK_DDR_ADB400_SLV_DDRA_MASK                               0x00000080
#define PERI_CRG_CLKST3_ST_CLK_SYSBUS_DATA_DMA_SHIFT                                   8
#define PERI_CRG_CLKST3_ST_CLK_SYSBUS_DATA_DMA_MASK                                    0x00000100
#define PERI_CRG_CLKST3_ST_CLK_CFGBUS_SHIFT                                            9
#define PERI_CRG_CLKST3_ST_CLK_CFGBUS_MASK                                             0x00000200
#define PERI_CRG_CLKST3_ST_CLK_CFGBUS_UP_SHIFT                                         11
#define PERI_CRG_CLKST3_ST_CLK_CFGBUS_UP_MASK                                          0x00000800
#define PERI_CRG_CLKST3_ST_CLK_DDR_CFG_ADB400_SLV_DDRA_SHIFT                           12
#define PERI_CRG_CLKST3_ST_CLK_DDR_CFG_ADB400_SLV_DDRA_MASK                            0x00001000
#define PERI_CRG_CLKST3_ST_CLK_PERICFG_BUS_SHIFT                                       13
#define PERI_CRG_CLKST3_ST_CLK_PERICFG_BUS_MASK                                        0x00002000
#define PERI_CRG_CLKST3_ST_CLK_DEBUG_ATB_SHIFT                                         14
#define PERI_CRG_CLKST3_ST_CLK_DEBUG_ATB_MASK                                          0x00004000
#define PERI_CRG_CLKST3_ST_CLK_DEBUG_ATB_XCTRL_DDR_SHIFT                               15
#define PERI_CRG_CLKST3_ST_CLK_DEBUG_ATB_XCTRL_DDR_MASK                                0x00008000
#define PERI_CRG_CLKST3_ST_CLK_CFGBUS_DEBUG_SHIFT                                      16
#define PERI_CRG_CLKST3_ST_CLK_CFGBUS_DEBUG_MASK                                       0x00010000
#define PERI_CRG_CLKST3_ST_CLK_DEBUG_ATB_H_SHIFT                                       17
#define PERI_CRG_CLKST3_ST_CLK_DEBUG_ATB_H_MASK                                        0x00020000
#define PERI_CRG_CLKST3_ST_CLK_CFGBUS_DMA_SHIFT                                        18
#define PERI_CRG_CLKST3_ST_CLK_CFGBUS_DMA_MASK                                         0x00040000
#define PERI_CRG_CLKST3_ST_CLK_CFGBUS_PERI_SHIFT                                       19
#define PERI_CRG_CLKST3_ST_CLK_CFGBUS_PERI_MASK                                        0x00080000
#define PERI_CRG_CLKST3_ST_CLK_SYSBUS_DATA_DBG_SHIFT                                   20
#define PERI_CRG_CLKST3_ST_CLK_SYSBUS_DATA_DBG_MASK                                    0x00100000
#define PERI_CRG_CLKST3_ST_PCLK_PERI_TIMER_S_SHIFT                                     24
#define PERI_CRG_CLKST3_ST_PCLK_PERI_TIMER_S_MASK                                      0x01000000
#define PERI_CRG_CLKST3_ST_PCLK_PERI_WDT3_SHIFT                                        25
#define PERI_CRG_CLKST3_ST_PCLK_PERI_WDT3_MASK                                         0x02000000
#define PERI_CRG_CLKST3_ST_PCLK_PERI_WDT2_SHIFT                                        26
#define PERI_CRG_CLKST3_ST_PCLK_PERI_WDT2_MASK                                         0x04000000
#define PERI_CRG_CLKGT4_W1S                        0x0810
#define PERI_CRG_CLKGT4_W1S_GT_PCLK_SPI0_SHIFT                                         0
#define PERI_CRG_CLKGT4_W1S_GT_PCLK_SPI1_SHIFT                                         1
#define PERI_CRG_CLKGT4_W1S_GT_PCLK_SPI2_SHIFT                                         2
#define PERI_CRG_CLKGT4_W1S_GT_CLK_PERI_TIMER4_SHIFT                                   3
#define PERI_CRG_CLKGT4_W1S_GT_CLK_PERI_TIMER5_SHIFT                                   4
#define PERI_CRG_CLKGT4_W1S_GT_CLK_PERI_TIMER6_SHIFT                                   5
#define PERI_CRG_CLKGT4_W1S_GT_CLK_PERI_TIMER7_SHIFT                                   6
#define PERI_CRG_CLKGT4_W1S_GT_PCLK_PERI_IPC0_SHIFT                                    7
#define PERI_CRG_CLKGT4_W1S_GT_PCLK_PERI_GPIO_SHIFT                                    25
#define PERI_CRG_CLKGT4_W1C                        0x0814
#define PERI_CRG_CLKGT4_W1C_GT_PCLK_SPI0_SHIFT                                         0
#define PERI_CRG_CLKGT4_W1C_GT_PCLK_SPI1_SHIFT                                         1
#define PERI_CRG_CLKGT4_W1C_GT_PCLK_SPI2_SHIFT                                         2
#define PERI_CRG_CLKGT4_W1C_GT_CLK_PERI_TIMER4_SHIFT                                   3
#define PERI_CRG_CLKGT4_W1C_GT_CLK_PERI_TIMER5_SHIFT                                   4
#define PERI_CRG_CLKGT4_W1C_GT_CLK_PERI_TIMER6_SHIFT                                   5
#define PERI_CRG_CLKGT4_W1C_GT_CLK_PERI_TIMER7_SHIFT                                   6
#define PERI_CRG_CLKGT4_W1C_GT_PCLK_PERI_IPC0_SHIFT                                    7
#define PERI_CRG_CLKGT4_W1C_GT_PCLK_PERI_GPIO_SHIFT                                    25
#define PERI_CRG_CLKGT4_RO                         0x0818
#define PERI_CRG_CLKGT4_RO_GT_PCLK_SPI0_SHIFT                                          0
#define PERI_CRG_CLKGT4_RO_GT_PCLK_SPI0_MASK                                           0x00000001
#define PERI_CRG_CLKGT4_RO_GT_PCLK_SPI1_SHIFT                                          1
#define PERI_CRG_CLKGT4_RO_GT_PCLK_SPI1_MASK                                           0x00000002
#define PERI_CRG_CLKGT4_RO_GT_PCLK_SPI2_SHIFT                                          2
#define PERI_CRG_CLKGT4_RO_GT_PCLK_SPI2_MASK                                           0x00000004
#define PERI_CRG_CLKGT4_RO_GT_CLK_PERI_TIMER4_SHIFT                                    3
#define PERI_CRG_CLKGT4_RO_GT_CLK_PERI_TIMER4_MASK                                     0x00000008
#define PERI_CRG_CLKGT4_RO_GT_CLK_PERI_TIMER5_SHIFT                                    4
#define PERI_CRG_CLKGT4_RO_GT_CLK_PERI_TIMER5_MASK                                     0x00000010
#define PERI_CRG_CLKGT4_RO_GT_CLK_PERI_TIMER6_SHIFT                                    5
#define PERI_CRG_CLKGT4_RO_GT_CLK_PERI_TIMER6_MASK                                     0x00000020
#define PERI_CRG_CLKGT4_RO_GT_CLK_PERI_TIMER7_SHIFT                                    6
#define PERI_CRG_CLKGT4_RO_GT_CLK_PERI_TIMER7_MASK                                     0x00000040
#define PERI_CRG_CLKGT4_RO_GT_PCLK_PERI_IPC0_SHIFT                                     7
#define PERI_CRG_CLKGT4_RO_GT_PCLK_PERI_IPC0_MASK                                      0x00000080
#define PERI_CRG_CLKGT4_RO_GT_PCLK_PERI_GPIO_SHIFT                                     25
#define PERI_CRG_CLKGT4_RO_GT_PCLK_PERI_GPIO_MASK                                      0x02000000
#define PERI_CRG_CLKST4                            0x081c
#define PERI_CRG_CLKST4_ST_PCLK_SPI0_SHIFT                                             0
#define PERI_CRG_CLKST4_ST_PCLK_SPI0_MASK                                              0x00000001
#define PERI_CRG_CLKST4_ST_PCLK_SPI1_SHIFT                                             1
#define PERI_CRG_CLKST4_ST_PCLK_SPI1_MASK                                              0x00000002
#define PERI_CRG_CLKST4_ST_PCLK_SPI2_SHIFT                                             2
#define PERI_CRG_CLKST4_ST_PCLK_SPI2_MASK                                              0x00000004
#define PERI_CRG_CLKST4_ST_CLK_PERI_TIMER4_SHIFT                                       3
#define PERI_CRG_CLKST4_ST_CLK_PERI_TIMER4_MASK                                        0x00000008
#define PERI_CRG_CLKST4_ST_CLK_PERI_TIMER5_SHIFT                                       4
#define PERI_CRG_CLKST4_ST_CLK_PERI_TIMER5_MASK                                        0x00000010
#define PERI_CRG_CLKST4_ST_CLK_PERI_TIMER6_SHIFT                                       5
#define PERI_CRG_CLKST4_ST_CLK_PERI_TIMER6_MASK                                        0x00000020
#define PERI_CRG_CLKST4_ST_CLK_PERI_TIMER7_SHIFT                                       6
#define PERI_CRG_CLKST4_ST_CLK_PERI_TIMER7_MASK                                        0x00000040
#define PERI_CRG_CLKST4_ST_PCLK_PERI_IPC0_SHIFT                                        7
#define PERI_CRG_CLKST4_ST_PCLK_PERI_IPC0_MASK                                         0x00000080
#define PERI_CRG_CLKST4_ST_PCLK_PERI_GPIO_SHIFT                                        25
#define PERI_CRG_CLKST4_ST_PCLK_PERI_GPIO_MASK                                         0x02000000
#define PERI_CRG_CLKST4_ST_CLK_DDR_CFG_ADB400_SLV_DDRB_SHIFT                           26
#define PERI_CRG_CLKST4_ST_CLK_DDR_CFG_ADB400_SLV_DDRB_MASK                            0x04000000
#define PERI_CRG_CLKST4_ST_CLK_DDR_CFG_ADB400_SLV_DDRC_SHIFT                           27
#define PERI_CRG_CLKST4_ST_CLK_DDR_CFG_ADB400_SLV_DDRC_MASK                            0x08000000
#define PERI_CRG_CLKST4_ST_CLK_DDR_CFG_ADB400_SLV_DDRD_SHIFT                           28
#define PERI_CRG_CLKST4_ST_CLK_DDR_CFG_ADB400_SLV_DDRD_MASK                            0x10000000
#define PERI_CRG_CLKST4_ST_ACLK_DDR_ADB400_SLV_DDRB_SHIFT                              29
#define PERI_CRG_CLKST4_ST_ACLK_DDR_ADB400_SLV_DDRB_MASK                               0x20000000
#define PERI_CRG_CLKST4_ST_ACLK_DDR_ADB400_SLV_DDRC_SHIFT                              30
#define PERI_CRG_CLKST4_ST_ACLK_DDR_ADB400_SLV_DDRC_MASK                               0x40000000
#define PERI_CRG_CLKST4_ST_ACLK_DDR_ADB400_SLV_DDRD_SHIFT                              31
#define PERI_CRG_CLKST4_ST_ACLK_DDR_ADB400_SLV_DDRD_MASK                               0x80000000
#define PERI_CRG_CLKGT5_W1S                        0x0820
#define PERI_CRG_CLKGT5_W1S_GT_PCLK_PCTRL_SHIFT                                        2
#define PERI_CRG_CLKGT5_W1S_GT_PCLK_PERI_IOCTRL_SHIFT                                  7
#define PERI_CRG_CLKGT5_W1S_GT_PCLK_PERIL_IOPADWRAP_SHIFT                              8
#define PERI_CRG_CLKGT5_W1S_GT_PCLK_PERIR_IOPADWRAP_SHIFT                              9
#define PERI_CRG_CLKGT5_W1S_GT_PCLK_ONEWIRE0_SHIFT                                     17
#define PERI_CRG_CLKGT5_W1S_GT_PCLK_ONEWIRE1_SHIFT                                     18
#define PERI_CRG_CLKGT5_W1S_GT_PCLK_ONEWIRE2_SHIFT                                     19
#define PERI_CRG_CLKGT5_W1S_GT_CLK_GPLL_PERI_SHIFT                                     22
#define PERI_CRG_CLKGT5_W1S_GT_CLK_GPLL_LPIS_SHIFT                                     23
#define PERI_CRG_CLKGT5_W1S_GT_CLK_BROADCAST_SHIFT                                     24
#define PERI_CRG_CLKGT5_W1S_GT_CLK_BROADCAST_XCTRL_DDR_ABRG_SLV_SHIFT                  25
#define PERI_CRG_CLKGT5_W1C                        0x0824
#define PERI_CRG_CLKGT5_W1C_GT_PCLK_PCTRL_SHIFT                                        2
#define PERI_CRG_CLKGT5_W1C_GT_PCLK_PERI_IOCTRL_SHIFT                                  7
#define PERI_CRG_CLKGT5_W1C_GT_PCLK_PERIL_IOPADWRAP_SHIFT                              8
#define PERI_CRG_CLKGT5_W1C_GT_PCLK_PERIR_IOPADWRAP_SHIFT                              9
#define PERI_CRG_CLKGT5_W1C_GT_PCLK_ONEWIRE0_SHIFT                                     17
#define PERI_CRG_CLKGT5_W1C_GT_PCLK_ONEWIRE1_SHIFT                                     18
#define PERI_CRG_CLKGT5_W1C_GT_PCLK_ONEWIRE2_SHIFT                                     19
#define PERI_CRG_CLKGT5_W1C_GT_CLK_GPLL_PERI_SHIFT                                     22
#define PERI_CRG_CLKGT5_W1C_GT_CLK_GPLL_LPIS_SHIFT                                     23
#define PERI_CRG_CLKGT5_W1C_GT_CLK_BROADCAST_SHIFT                                     24
#define PERI_CRG_CLKGT5_W1C_GT_CLK_BROADCAST_XCTRL_DDR_ABRG_SLV_SHIFT                  25
#define PERI_CRG_CLKGT5_RO                         0x0828
#define PERI_CRG_CLKGT5_RO_GT_PCLK_PCTRL_SHIFT                                         2
#define PERI_CRG_CLKGT5_RO_GT_PCLK_PCTRL_MASK                                          0x00000004
#define PERI_CRG_CLKGT5_RO_GT_PCLK_PERI_IOCTRL_SHIFT                                   7
#define PERI_CRG_CLKGT5_RO_GT_PCLK_PERI_IOCTRL_MASK                                    0x00000080
#define PERI_CRG_CLKGT5_RO_GT_PCLK_PERIL_IOPADWRAP_SHIFT                               8
#define PERI_CRG_CLKGT5_RO_GT_PCLK_PERIL_IOPADWRAP_MASK                                0x00000100
#define PERI_CRG_CLKGT5_RO_GT_PCLK_PERIR_IOPADWRAP_SHIFT                               9
#define PERI_CRG_CLKGT5_RO_GT_PCLK_PERIR_IOPADWRAP_MASK                                0x00000200
#define PERI_CRG_CLKGT5_RO_GT_PCLK_ONEWIRE0_SHIFT                                      17
#define PERI_CRG_CLKGT5_RO_GT_PCLK_ONEWIRE0_MASK                                       0x00020000
#define PERI_CRG_CLKGT5_RO_GT_PCLK_ONEWIRE1_SHIFT                                      18
#define PERI_CRG_CLKGT5_RO_GT_PCLK_ONEWIRE1_MASK                                       0x00040000
#define PERI_CRG_CLKGT5_RO_GT_PCLK_ONEWIRE2_SHIFT                                      19
#define PERI_CRG_CLKGT5_RO_GT_PCLK_ONEWIRE2_MASK                                       0x00080000
#define PERI_CRG_CLKGT5_RO_GT_CLK_GPLL_PERI_SHIFT                                      22
#define PERI_CRG_CLKGT5_RO_GT_CLK_GPLL_PERI_MASK                                       0x00400000
#define PERI_CRG_CLKGT5_RO_GT_CLK_GPLL_LPIS_SHIFT                                      23
#define PERI_CRG_CLKGT5_RO_GT_CLK_GPLL_LPIS_MASK                                       0x00800000
#define PERI_CRG_CLKGT5_RO_GT_CLK_BROADCAST_SHIFT                                      24
#define PERI_CRG_CLKGT5_RO_GT_CLK_BROADCAST_MASK                                       0x01000000
#define PERI_CRG_CLKGT5_RO_GT_CLK_BROADCAST_XCTRL_DDR_ABRG_SLV_SHIFT                   25
#define PERI_CRG_CLKGT5_RO_GT_CLK_BROADCAST_XCTRL_DDR_ABRG_SLV_MASK                    0x02000000
#define PERI_CRG_CLKST5                            0x082c
#define PERI_CRG_CLKST5_ST_PCLK_PCTRL_SHIFT                                            2
#define PERI_CRG_CLKST5_ST_PCLK_PCTRL_MASK                                             0x00000004
#define PERI_CRG_CLKST5_ST_PCLK_PERI_IOCTRL_SHIFT                                      7
#define PERI_CRG_CLKST5_ST_PCLK_PERI_IOCTRL_MASK                                       0x00000080
#define PERI_CRG_CLKST5_ST_PCLK_PERIL_IOPADWRAP_SHIFT                                  8
#define PERI_CRG_CLKST5_ST_PCLK_PERIL_IOPADWRAP_MASK                                   0x00000100
#define PERI_CRG_CLKST5_ST_PCLK_PERIR_IOPADWRAP_SHIFT                                  9
#define PERI_CRG_CLKST5_ST_PCLK_PERIR_IOPADWRAP_MASK                                   0x00000200
#define PERI_CRG_CLKST5_ST_PCLK_ONEWIRE0_SHIFT                                         17
#define PERI_CRG_CLKST5_ST_PCLK_ONEWIRE0_MASK                                          0x00020000
#define PERI_CRG_CLKST5_ST_PCLK_ONEWIRE1_SHIFT                                         18
#define PERI_CRG_CLKST5_ST_PCLK_ONEWIRE1_MASK                                          0x00040000
#define PERI_CRG_CLKST5_ST_PCLK_ONEWIRE2_SHIFT                                         19
#define PERI_CRG_CLKST5_ST_PCLK_ONEWIRE2_MASK                                          0x00080000
#define PERI_CRG_CLKST5_ST_CLK_GPLL_PERI_SHIFT                                         22
#define PERI_CRG_CLKST5_ST_CLK_GPLL_PERI_MASK                                          0x00400000
#define PERI_CRG_CLKST5_ST_CLK_GPLL_LPIS_SHIFT                                         23
#define PERI_CRG_CLKST5_ST_CLK_GPLL_LPIS_MASK                                          0x00800000
#define PERI_CRG_CLKST5_ST_CLK_BROADCAST_SHIFT                                         24
#define PERI_CRG_CLKST5_ST_CLK_BROADCAST_MASK                                          0x01000000
#define PERI_CRG_CLKST5_ST_CLK_BROADCAST_XCTRL_DDR_ABRG_SLV_SHIFT                      25
#define PERI_CRG_CLKST5_ST_CLK_BROADCAST_XCTRL_DDR_ABRG_SLV_MASK                       0x02000000
#define PERI_CRG_CLKGT6_W1S                        0x0830
#define PERI_CRG_CLKGT6_W1S_GT_CLK_PERI_GPIODB_SHIFT                                   8
#define PERI_CRG_CLKGT6_W1S_GT_CLKIN_REF_XCTRL_DDR_SHIFT                               12
#define PERI_CRG_CLKGT6_W1S_GT_CLKIN_REF_CORE_SHIFT                                    13
#define PERI_CRG_CLKGT6_W1S_GT_CLK_SPI0_SHIFT                                          19
#define PERI_CRG_CLKGT6_W1S_GT_CLK_SPI1_SHIFT                                          20
#define PERI_CRG_CLKGT6_W1S_GT_CLK_SPI2_SHIFT                                          21
#define PERI_CRG_CLKGT6_W1S_GT_CLK_PERI_WDT2_SHIFT                                     28
#define PERI_CRG_CLKGT6_W1S_GT_CLK_PERI_WDT3_SHIFT                                     29
#define PERI_CRG_CLKGT6_W1C                        0x0834
#define PERI_CRG_CLKGT6_W1C_GT_CLK_PERI_GPIODB_SHIFT                                   8
#define PERI_CRG_CLKGT6_W1C_GT_CLKIN_REF_XCTRL_DDR_SHIFT                               12
#define PERI_CRG_CLKGT6_W1C_GT_CLKIN_REF_CORE_SHIFT                                    13
#define PERI_CRG_CLKGT6_W1C_GT_CLK_SPI0_SHIFT                                          19
#define PERI_CRG_CLKGT6_W1C_GT_CLK_SPI1_SHIFT                                          20
#define PERI_CRG_CLKGT6_W1C_GT_CLK_SPI2_SHIFT                                          21
#define PERI_CRG_CLKGT6_W1C_GT_CLK_PERI_WDT2_SHIFT                                     28
#define PERI_CRG_CLKGT6_W1C_GT_CLK_PERI_WDT3_SHIFT                                     29
#define PERI_CRG_CLKGT6_RO                         0x0838
#define PERI_CRG_CLKGT6_RO_GT_CLK_PERI_GPIODB_SHIFT                                    8
#define PERI_CRG_CLKGT6_RO_GT_CLK_PERI_GPIODB_MASK                                     0x00000100
#define PERI_CRG_CLKGT6_RO_GT_CLKIN_REF_XCTRL_DDR_SHIFT                                12
#define PERI_CRG_CLKGT6_RO_GT_CLKIN_REF_XCTRL_DDR_MASK                                 0x00001000
#define PERI_CRG_CLKGT6_RO_GT_CLKIN_REF_CORE_SHIFT                                     13
#define PERI_CRG_CLKGT6_RO_GT_CLKIN_REF_CORE_MASK                                      0x00002000
#define PERI_CRG_CLKGT6_RO_GT_CLK_SPI0_SHIFT                                           19
#define PERI_CRG_CLKGT6_RO_GT_CLK_SPI0_MASK                                            0x00080000
#define PERI_CRG_CLKGT6_RO_GT_CLK_SPI1_SHIFT                                           20
#define PERI_CRG_CLKGT6_RO_GT_CLK_SPI1_MASK                                            0x00100000
#define PERI_CRG_CLKGT6_RO_GT_CLK_SPI2_SHIFT                                           21
#define PERI_CRG_CLKGT6_RO_GT_CLK_SPI2_MASK                                            0x00200000
#define PERI_CRG_CLKGT6_RO_GT_CLK_PERI_WDT2_SHIFT                                      28
#define PERI_CRG_CLKGT6_RO_GT_CLK_PERI_WDT2_MASK                                       0x10000000
#define PERI_CRG_CLKGT6_RO_GT_CLK_PERI_WDT3_SHIFT                                      29
#define PERI_CRG_CLKGT6_RO_GT_CLK_PERI_WDT3_MASK                                       0x20000000
#define PERI_CRG_CLKST6                            0x083c
#define PERI_CRG_CLKST6_ST_CLK_PERI_GPIODB_SHIFT                                       8
#define PERI_CRG_CLKST6_ST_CLK_PERI_GPIODB_MASK                                        0x00000100
#define PERI_CRG_CLKST6_ST_CLKIN_REF_XCTRL_DDR_SHIFT                                   12
#define PERI_CRG_CLKST6_ST_CLKIN_REF_XCTRL_DDR_MASK                                    0x00001000
#define PERI_CRG_CLKST6_ST_CLKIN_REF_CORE_SHIFT                                        13
#define PERI_CRG_CLKST6_ST_CLKIN_REF_CORE_MASK                                         0x00002000
#define PERI_CRG_CLKST6_ST_CLK_SPI0_SHIFT                                              19
#define PERI_CRG_CLKST6_ST_CLK_SPI0_MASK                                               0x00080000
#define PERI_CRG_CLKST6_ST_CLK_SPI1_SHIFT                                              20
#define PERI_CRG_CLKST6_ST_CLK_SPI1_MASK                                               0x00100000
#define PERI_CRG_CLKST6_ST_CLK_SPI2_SHIFT                                              21
#define PERI_CRG_CLKST6_ST_CLK_SPI2_MASK                                               0x00200000
#define PERI_CRG_CLKST6_ST_CLK_PERI_WDT2_SHIFT                                         28
#define PERI_CRG_CLKST6_ST_CLK_PERI_WDT2_MASK                                          0x10000000
#define PERI_CRG_CLKST6_ST_CLK_PERI_WDT3_SHIFT                                         29
#define PERI_CRG_CLKST6_ST_CLK_PERI_WDT3_MASK                                          0x20000000
#define PERI_CRG_CLKGT7_W1S                        0x0840
#define PERI_CRG_CLKGT7_W1S_GT_CLK_AXIBIST_SHIFT                                       21
#define PERI_CRG_CLKGT7_W1S_GT_CLK_DMA_BUS_DATA_SHIFT                                  25
#define PERI_CRG_CLKGT7_W1S_GT_CLK_CFGBUS_MEDIA1_SHIFT                                 27
#define PERI_CRG_CLKGT7_W1S_GT_CLK_SYSBUS_MEDIA1_SHIFT                                 28
#define PERI_CRG_CLKGT7_W1S_GT_CLK_SYSBUS_MEDIA2_SHIFT                                 29
#define PERI_CRG_CLKGT7_W1C                        0x0844
#define PERI_CRG_CLKGT7_W1C_GT_CLK_AXIBIST_SHIFT                                       21
#define PERI_CRG_CLKGT7_W1C_GT_CLK_DMA_BUS_DATA_SHIFT                                  25
#define PERI_CRG_CLKGT7_W1C_GT_CLK_CFGBUS_MEDIA1_SHIFT                                 27
#define PERI_CRG_CLKGT7_W1C_GT_CLK_SYSBUS_MEDIA1_SHIFT                                 28
#define PERI_CRG_CLKGT7_W1C_GT_CLK_SYSBUS_MEDIA2_SHIFT                                 29
#define PERI_CRG_CLKGT7_RO                         0x0848
#define PERI_CRG_CLKGT7_RO_GT_CLK_AXIBIST_SHIFT                                        21
#define PERI_CRG_CLKGT7_RO_GT_CLK_AXIBIST_MASK                                         0x00200000
#define PERI_CRG_CLKGT7_RO_GT_CLK_DMA_BUS_DATA_SHIFT                                   25
#define PERI_CRG_CLKGT7_RO_GT_CLK_DMA_BUS_DATA_MASK                                    0x02000000
#define PERI_CRG_CLKGT7_RO_GT_CLK_CFGBUS_MEDIA1_SHIFT                                  27
#define PERI_CRG_CLKGT7_RO_GT_CLK_CFGBUS_MEDIA1_MASK                                   0x08000000
#define PERI_CRG_CLKGT7_RO_GT_CLK_SYSBUS_MEDIA1_SHIFT                                  28
#define PERI_CRG_CLKGT7_RO_GT_CLK_SYSBUS_MEDIA1_MASK                                   0x10000000
#define PERI_CRG_CLKGT7_RO_GT_CLK_SYSBUS_MEDIA2_SHIFT                                  29
#define PERI_CRG_CLKGT7_RO_GT_CLK_SYSBUS_MEDIA2_MASK                                   0x20000000
#define PERI_CRG_CLKST7                            0x084c
#define PERI_CRG_CLKST7_ST_CLK_AXIBIST_SHIFT                                           21
#define PERI_CRG_CLKST7_ST_CLK_AXIBIST_MASK                                            0x00200000
#define PERI_CRG_CLKST7_ST_CLK_DMA_BUS_DATA_SHIFT                                      25
#define PERI_CRG_CLKST7_ST_CLK_DMA_BUS_DATA_MASK                                       0x02000000
#define PERI_CRG_CLKST7_ST_CLK_CFGBUS_MEDIA1_SHIFT                                     27
#define PERI_CRG_CLKST7_ST_CLK_CFGBUS_MEDIA1_MASK                                      0x08000000
#define PERI_CRG_CLKST7_ST_CLK_SYSBUS_MEDIA1_SHIFT                                     28
#define PERI_CRG_CLKST7_ST_CLK_SYSBUS_MEDIA1_MASK                                      0x10000000
#define PERI_CRG_CLKST7_ST_CLK_SYSBUS_MEDIA2_SHIFT                                     29
#define PERI_CRG_CLKST7_ST_CLK_SYSBUS_MEDIA2_MASK                                      0x20000000
#define PERI_CRG_CLKGT8_W1S                        0x0850
#define PERI_CRG_CLKGT8_W1S_GT_PCLK_DMA_BUS_SHIFT                                      0
#define PERI_CRG_CLKGT8_W1S_GT_PCLK_AXIBIST_SHIFT                                      1
#define PERI_CRG_CLKGT8_W1S_GT_CLK_ONEWIRE0_SHIFT                                      11
#define PERI_CRG_CLKGT8_W1S_GT_CLK_ONEWIRE1_SHIFT                                      12
#define PERI_CRG_CLKGT8_W1S_GT_CLK_ONEWIRE2_SHIFT                                      13
#define PERI_CRG_CLKGT8_W1S_GT_CLK_DEBUG_APB_SHIFT                                     15
#define PERI_CRG_CLKGT8_W1S_GT_CLK_DEBUG_APB_UP_SHIFT                                  16
#define PERI_CRG_CLKGT8_W1S_GT_CLK_NPU_BUS_CFG_SHIFT                                   21
#define PERI_CRG_CLKGT8_W1S_GT_CLK_HSS1_BUS_CFG_SHIFT                                  22
#define PERI_CRG_CLKGT8_W1S_GT_CLK_HSS2_BUS_CFG_SHIFT                                  23
#define PERI_CRG_CLKGT8_W1S_GT_PCLK_MEDIA1_CFG_SHIFT                                   24
#define PERI_CRG_CLKGT8_W1S_GT_PCLK_MEDIA2_CFG_SHIFT                                   25
#define PERI_CRG_CLKGT8_W1S_GT_CLK_DEBUG_APB_XCTRL_DDR_SHIFT                           26
#define PERI_CRG_CLKGT8_W1S_GT_CLK_DEBUG_TRACE_SHIFT                                   28
#define PERI_CRG_CLKGT8_W1S_GT_CLK_XCTRL_DDR_SHIFT                                     29
#define PERI_CRG_CLKGT8_W1C                        0x0854
#define PERI_CRG_CLKGT8_W1C_GT_PCLK_DMA_BUS_SHIFT                                      0
#define PERI_CRG_CLKGT8_W1C_GT_PCLK_AXIBIST_SHIFT                                      1
#define PERI_CRG_CLKGT8_W1C_GT_CLK_ONEWIRE0_SHIFT                                      11
#define PERI_CRG_CLKGT8_W1C_GT_CLK_ONEWIRE1_SHIFT                                      12
#define PERI_CRG_CLKGT8_W1C_GT_CLK_ONEWIRE2_SHIFT                                      13
#define PERI_CRG_CLKGT8_W1C_GT_CLK_DEBUG_APB_SHIFT                                     15
#define PERI_CRG_CLKGT8_W1C_GT_CLK_DEBUG_APB_UP_SHIFT                                  16
#define PERI_CRG_CLKGT8_W1C_GT_CLK_NPU_BUS_CFG_SHIFT                                   21
#define PERI_CRG_CLKGT8_W1C_GT_CLK_HSS1_BUS_CFG_SHIFT                                  22
#define PERI_CRG_CLKGT8_W1C_GT_CLK_HSS2_BUS_CFG_SHIFT                                  23
#define PERI_CRG_CLKGT8_W1C_GT_PCLK_MEDIA1_CFG_SHIFT                                   24
#define PERI_CRG_CLKGT8_W1C_GT_PCLK_MEDIA2_CFG_SHIFT                                   25
#define PERI_CRG_CLKGT8_W1C_GT_CLK_DEBUG_APB_XCTRL_DDR_SHIFT                           26
#define PERI_CRG_CLKGT8_W1C_GT_CLK_DEBUG_TRACE_SHIFT                                   28
#define PERI_CRG_CLKGT8_W1C_GT_CLK_XCTRL_DDR_SHIFT                                     29
#define PERI_CRG_CLKGT8_RO                         0x0858
#define PERI_CRG_CLKGT8_RO_GT_PCLK_DMA_BUS_SHIFT                                       0
#define PERI_CRG_CLKGT8_RO_GT_PCLK_DMA_BUS_MASK                                        0x00000001
#define PERI_CRG_CLKGT8_RO_GT_PCLK_AXIBIST_SHIFT                                       1
#define PERI_CRG_CLKGT8_RO_GT_PCLK_AXIBIST_MASK                                        0x00000002
#define PERI_CRG_CLKGT8_RO_GT_CLK_ONEWIRE0_SHIFT                                       11
#define PERI_CRG_CLKGT8_RO_GT_CLK_ONEWIRE0_MASK                                        0x00000800
#define PERI_CRG_CLKGT8_RO_GT_CLK_ONEWIRE1_SHIFT                                       12
#define PERI_CRG_CLKGT8_RO_GT_CLK_ONEWIRE1_MASK                                        0x00001000
#define PERI_CRG_CLKGT8_RO_GT_CLK_ONEWIRE2_SHIFT                                       13
#define PERI_CRG_CLKGT8_RO_GT_CLK_ONEWIRE2_MASK                                        0x00002000
#define PERI_CRG_CLKGT8_RO_GT_CLK_DEBUG_APB_SHIFT                                      15
#define PERI_CRG_CLKGT8_RO_GT_CLK_DEBUG_APB_MASK                                       0x00008000
#define PERI_CRG_CLKGT8_RO_GT_CLK_DEBUG_APB_UP_SHIFT                                   16
#define PERI_CRG_CLKGT8_RO_GT_CLK_DEBUG_APB_UP_MASK                                    0x00010000
#define PERI_CRG_CLKGT8_RO_GT_CLK_NPU_BUS_CFG_SHIFT                                    21
#define PERI_CRG_CLKGT8_RO_GT_CLK_NPU_BUS_CFG_MASK                                     0x00200000
#define PERI_CRG_CLKGT8_RO_GT_CLK_HSS1_BUS_CFG_SHIFT                                   22
#define PERI_CRG_CLKGT8_RO_GT_CLK_HSS1_BUS_CFG_MASK                                    0x00400000
#define PERI_CRG_CLKGT8_RO_GT_CLK_HSS2_BUS_CFG_SHIFT                                   23
#define PERI_CRG_CLKGT8_RO_GT_CLK_HSS2_BUS_CFG_MASK                                    0x00800000
#define PERI_CRG_CLKGT8_RO_GT_PCLK_MEDIA1_CFG_SHIFT                                    24
#define PERI_CRG_CLKGT8_RO_GT_PCLK_MEDIA1_CFG_MASK                                     0x01000000
#define PERI_CRG_CLKGT8_RO_GT_PCLK_MEDIA2_CFG_SHIFT                                    25
#define PERI_CRG_CLKGT8_RO_GT_PCLK_MEDIA2_CFG_MASK                                     0x02000000
#define PERI_CRG_CLKGT8_RO_GT_CLK_DEBUG_APB_XCTRL_DDR_SHIFT                            26
#define PERI_CRG_CLKGT8_RO_GT_CLK_DEBUG_APB_XCTRL_DDR_MASK                             0x04000000
#define PERI_CRG_CLKGT8_RO_GT_CLK_DEBUG_TRACE_SHIFT                                    28
#define PERI_CRG_CLKGT8_RO_GT_CLK_DEBUG_TRACE_MASK                                     0x10000000
#define PERI_CRG_CLKGT8_RO_GT_CLK_XCTRL_DDR_SHIFT                                      29
#define PERI_CRG_CLKGT8_RO_GT_CLK_XCTRL_DDR_MASK                                       0x20000000
#define PERI_CRG_CLKST8                            0x085c
#define PERI_CRG_CLKST8_ST_PCLK_DMA_BUS_SHIFT                                          0
#define PERI_CRG_CLKST8_ST_PCLK_DMA_BUS_MASK                                           0x00000001
#define PERI_CRG_CLKST8_ST_PCLK_AXIBIST_SHIFT                                          1
#define PERI_CRG_CLKST8_ST_PCLK_AXIBIST_MASK                                           0x00000002
#define PERI_CRG_CLKST8_ST_CLK_ONEWIRE0_SHIFT                                          11
#define PERI_CRG_CLKST8_ST_CLK_ONEWIRE0_MASK                                           0x00000800
#define PERI_CRG_CLKST8_ST_CLK_ONEWIRE1_SHIFT                                          12
#define PERI_CRG_CLKST8_ST_CLK_ONEWIRE1_MASK                                           0x00001000
#define PERI_CRG_CLKST8_ST_CLK_ONEWIRE2_SHIFT                                          13
#define PERI_CRG_CLKST8_ST_CLK_ONEWIRE2_MASK                                           0x00002000
#define PERI_CRG_CLKST8_ST_CLK_DEBUG_APB_SHIFT                                         15
#define PERI_CRG_CLKST8_ST_CLK_DEBUG_APB_MASK                                          0x00008000
#define PERI_CRG_CLKST8_ST_CLK_DEBUG_APB_UP_SHIFT                                      16
#define PERI_CRG_CLKST8_ST_CLK_DEBUG_APB_UP_MASK                                       0x00010000
#define PERI_CRG_CLKST8_ST_CLK_NPU_BUS_CFG_SHIFT                                       21
#define PERI_CRG_CLKST8_ST_CLK_NPU_BUS_CFG_MASK                                        0x00200000
#define PERI_CRG_CLKST8_ST_CLK_HSS1_BUS_CFG_SHIFT                                      22
#define PERI_CRG_CLKST8_ST_CLK_HSS1_BUS_CFG_MASK                                       0x00400000
#define PERI_CRG_CLKST8_ST_CLK_HSS2_BUS_CFG_SHIFT                                      23
#define PERI_CRG_CLKST8_ST_CLK_HSS2_BUS_CFG_MASK                                       0x00800000
#define PERI_CRG_CLKST8_ST_PCLK_MEDIA1_CFG_SHIFT                                       24
#define PERI_CRG_CLKST8_ST_PCLK_MEDIA1_CFG_MASK                                        0x01000000
#define PERI_CRG_CLKST8_ST_PCLK_MEDIA2_CFG_SHIFT                                       25
#define PERI_CRG_CLKST8_ST_PCLK_MEDIA2_CFG_MASK                                        0x02000000
#define PERI_CRG_CLKST8_ST_CLK_DEBUG_APB_XCTRL_DDR_SHIFT                               26
#define PERI_CRG_CLKST8_ST_CLK_DEBUG_APB_XCTRL_DDR_MASK                                0x04000000
#define PERI_CRG_CLKST8_ST_CLK_DEBUG_TRACE_SHIFT                                       28
#define PERI_CRG_CLKST8_ST_CLK_DEBUG_TRACE_MASK                                        0x10000000
#define PERI_CRG_CLKST8_ST_CLK_XCTRL_DDR_SHIFT                                         29
#define PERI_CRG_CLKST8_ST_CLK_XCTRL_DDR_MASK                                          0x20000000
#define PERI_CRG_CLKGT9_W1S                        0x0860
#define PERI_CRG_CLKGT9_W1S_GT_CLK_CPU_BUS_SHIFT                                       0
#define PERI_CRG_CLKGT9_W1S_GT_CLK_XCTRL_CPU_SHIFT                                     1
#define PERI_CRG_CLKGT9_W1S_GT_PCLK_CRG_NPU_SHIFT                                      11
#define PERI_CRG_CLKGT9_W1S_GT_CLK_GPLL_GPU_SHIFT                                      14
#define PERI_CRG_CLKGT9_W1S_GT_CLK_GPLL_CPU_SHIFT                                      15
#define PERI_CRG_CLKGT9_W1S_GT_CLK_GPLL_UP_SHIFT                                       16
#define PERI_CRG_CLKGT9_W1S_GT_PCLK_CRG_CORE_SHIFT                                     17
#define PERI_CRG_CLKGT9_W1S_GT_PCLK_CRG_XCTRL_DDR_SHIFT                                18
#define PERI_CRG_CLKGT9_W1S_GT_PCLK_CRG_GPU_SHIFT                                      19
#define PERI_CRG_CLKGT9_W1S_GT_PCLK_CRG_HSS2_SHIFT                                     20
#define PERI_CRG_CLKGT9_W1S_GT_PCLK_CRG_CPU_SHIFT                                      21
#define PERI_CRG_CLKGT9_W1S_GT_CLK_HSS1_SHIFT                                          22
#define PERI_CRG_CLKGT9_W1S_GT_CLK_HSS2_SHIFT                                          23
#define PERI_CRG_CLKGT9_W1S_GT_CLK_XRSE_SHIFT                                          25
#define PERI_CRG_CLKGT9_W1S_GT_PCLK_CRG_HSS1_SHIFT                                     26
#define PERI_CRG_CLKGT9_W1S_GT_PCLK_CRG_MEDIA2_SHIFT                                   27
#define PERI_CRG_CLKGT9_W1S_GT_PCLK_CRG_MEDIA1_SHIFT                                   28
#define PERI_CRG_CLKGT9_W1S_GT_CLK_DDR_DFICLK_SHIFT                                    29
#define PERI_CRG_CLKGT9_W1S_GT_CLK_PERI2NPU_NOC_SHIFT                                  30
#define PERI_CRG_CLKGT9_W1S_GT_CLK_PERI2NPU_NIC_SHIFT                                  31
#define PERI_CRG_CLKGT9_W1C                        0x0864
#define PERI_CRG_CLKGT9_W1C_GT_CLK_CPU_BUS_SHIFT                                       0
#define PERI_CRG_CLKGT9_W1C_GT_CLK_XCTRL_CPU_SHIFT                                     1
#define PERI_CRG_CLKGT9_W1C_GT_PCLK_CRG_NPU_SHIFT                                      11
#define PERI_CRG_CLKGT9_W1C_GT_CLK_GPLL_GPU_SHIFT                                      14
#define PERI_CRG_CLKGT9_W1C_GT_CLK_GPLL_CPU_SHIFT                                      15
#define PERI_CRG_CLKGT9_W1C_GT_CLK_GPLL_UP_SHIFT                                       16
#define PERI_CRG_CLKGT9_W1C_GT_PCLK_CRG_CORE_SHIFT                                     17
#define PERI_CRG_CLKGT9_W1C_GT_PCLK_CRG_XCTRL_DDR_SHIFT                                18
#define PERI_CRG_CLKGT9_W1C_GT_PCLK_CRG_GPU_SHIFT                                      19
#define PERI_CRG_CLKGT9_W1C_GT_PCLK_CRG_HSS2_SHIFT                                     20
#define PERI_CRG_CLKGT9_W1C_GT_PCLK_CRG_CPU_SHIFT                                      21
#define PERI_CRG_CLKGT9_W1C_GT_CLK_HSS1_SHIFT                                          22
#define PERI_CRG_CLKGT9_W1C_GT_CLK_HSS2_SHIFT                                          23
#define PERI_CRG_CLKGT9_W1C_GT_CLK_XRSE_SHIFT                                          25
#define PERI_CRG_CLKGT9_W1C_GT_PCLK_CRG_HSS1_SHIFT                                     26
#define PERI_CRG_CLKGT9_W1C_GT_PCLK_CRG_MEDIA2_SHIFT                                   27
#define PERI_CRG_CLKGT9_W1C_GT_PCLK_CRG_MEDIA1_SHIFT                                   28
#define PERI_CRG_CLKGT9_W1C_GT_CLK_DDR_DFICLK_SHIFT                                    29
#define PERI_CRG_CLKGT9_W1C_GT_CLK_PERI2NPU_NOC_SHIFT                                  30
#define PERI_CRG_CLKGT9_W1C_GT_CLK_PERI2NPU_NIC_SHIFT                                  31
#define PERI_CRG_CLKGT9_RO                         0x0868
#define PERI_CRG_CLKGT9_RO_GT_CLK_CPU_BUS_SHIFT                                        0
#define PERI_CRG_CLKGT9_RO_GT_CLK_CPU_BUS_MASK                                         0x00000001
#define PERI_CRG_CLKGT9_RO_GT_CLK_XCTRL_CPU_SHIFT                                      1
#define PERI_CRG_CLKGT9_RO_GT_CLK_XCTRL_CPU_MASK                                       0x00000002
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_NPU_SHIFT                                       11
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_NPU_MASK                                        0x00000800
#define PERI_CRG_CLKGT9_RO_GT_CLK_GPLL_GPU_SHIFT                                       14
#define PERI_CRG_CLKGT9_RO_GT_CLK_GPLL_GPU_MASK                                        0x00004000
#define PERI_CRG_CLKGT9_RO_GT_CLK_GPLL_CPU_SHIFT                                       15
#define PERI_CRG_CLKGT9_RO_GT_CLK_GPLL_CPU_MASK                                        0x00008000
#define PERI_CRG_CLKGT9_RO_GT_CLK_GPLL_UP_SHIFT                                        16
#define PERI_CRG_CLKGT9_RO_GT_CLK_GPLL_UP_MASK                                         0x00010000
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_CORE_SHIFT                                      17
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_CORE_MASK                                       0x00020000
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_XCTRL_DDR_SHIFT                                 18
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_XCTRL_DDR_MASK                                  0x00040000
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_GPU_SHIFT                                       19
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_GPU_MASK                                        0x00080000
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_HSS2_SHIFT                                      20
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_HSS2_MASK                                       0x00100000
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_CPU_SHIFT                                       21
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_CPU_MASK                                        0x00200000
#define PERI_CRG_CLKGT9_RO_GT_CLK_HSS1_SHIFT                                           22
#define PERI_CRG_CLKGT9_RO_GT_CLK_HSS1_MASK                                            0x00400000
#define PERI_CRG_CLKGT9_RO_GT_CLK_HSS2_SHIFT                                           23
#define PERI_CRG_CLKGT9_RO_GT_CLK_HSS2_MASK                                            0x00800000
#define PERI_CRG_CLKGT9_RO_GT_CLK_XRSE_SHIFT                                           25
#define PERI_CRG_CLKGT9_RO_GT_CLK_XRSE_MASK                                            0x02000000
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_HSS1_SHIFT                                      26
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_HSS1_MASK                                       0x04000000
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_MEDIA2_SHIFT                                    27
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_MEDIA2_MASK                                     0x08000000
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_MEDIA1_SHIFT                                    28
#define PERI_CRG_CLKGT9_RO_GT_PCLK_CRG_MEDIA1_MASK                                     0x10000000
#define PERI_CRG_CLKGT9_RO_GT_CLK_DDR_DFICLK_SHIFT                                     29
#define PERI_CRG_CLKGT9_RO_GT_CLK_DDR_DFICLK_MASK                                      0x20000000
#define PERI_CRG_CLKGT9_RO_GT_CLK_PERI2NPU_NOC_SHIFT                                   30
#define PERI_CRG_CLKGT9_RO_GT_CLK_PERI2NPU_NOC_MASK                                    0x40000000
#define PERI_CRG_CLKGT9_RO_GT_CLK_PERI2NPU_NIC_SHIFT                                   31
#define PERI_CRG_CLKGT9_RO_GT_CLK_PERI2NPU_NIC_MASK                                    0x80000000
#define PERI_CRG_CLKST9                            0x086c
#define PERI_CRG_CLKST9_ST_CLK_CPU_BUS_SHIFT                                           0
#define PERI_CRG_CLKST9_ST_CLK_CPU_BUS_MASK                                            0x00000001
#define PERI_CRG_CLKST9_ST_CLK_XCTRL_CPU_SHIFT                                         1
#define PERI_CRG_CLKST9_ST_CLK_XCTRL_CPU_MASK                                          0x00000002
#define PERI_CRG_CLKST9_ST_PCLK_CRG_NPU_SHIFT                                          11
#define PERI_CRG_CLKST9_ST_PCLK_CRG_NPU_MASK                                           0x00000800
#define PERI_CRG_CLKST9_ST_CLK_GPLL_GPU_SHIFT                                          14
#define PERI_CRG_CLKST9_ST_CLK_GPLL_GPU_MASK                                           0x00004000
#define PERI_CRG_CLKST9_ST_CLK_GPLL_CPU_SHIFT                                          15
#define PERI_CRG_CLKST9_ST_CLK_GPLL_CPU_MASK                                           0x00008000
#define PERI_CRG_CLKST9_ST_CLK_GPLL_UP_SHIFT                                           16
#define PERI_CRG_CLKST9_ST_CLK_GPLL_UP_MASK                                            0x00010000
#define PERI_CRG_CLKST9_ST_PCLK_CRG_CORE_SHIFT                                         17
#define PERI_CRG_CLKST9_ST_PCLK_CRG_CORE_MASK                                          0x00020000
#define PERI_CRG_CLKST9_ST_PCLK_CRG_XCTRL_DDR_SHIFT                                    18
#define PERI_CRG_CLKST9_ST_PCLK_CRG_XCTRL_DDR_MASK                                     0x00040000
#define PERI_CRG_CLKST9_ST_PCLK_CRG_GPU_SHIFT                                          19
#define PERI_CRG_CLKST9_ST_PCLK_CRG_GPU_MASK                                           0x00080000
#define PERI_CRG_CLKST9_ST_PCLK_CRG_HSS2_SHIFT                                         20
#define PERI_CRG_CLKST9_ST_PCLK_CRG_HSS2_MASK                                          0x00100000
#define PERI_CRG_CLKST9_ST_PCLK_CRG_CPU_SHIFT                                          21
#define PERI_CRG_CLKST9_ST_PCLK_CRG_CPU_MASK                                           0x00200000
#define PERI_CRG_CLKST9_ST_CLK_HSS1_SHIFT                                              22
#define PERI_CRG_CLKST9_ST_CLK_HSS1_MASK                                               0x00400000
#define PERI_CRG_CLKST9_ST_CLK_HSS2_SHIFT                                              23
#define PERI_CRG_CLKST9_ST_CLK_HSS2_MASK                                               0x00800000
#define PERI_CRG_CLKST9_ST_CLK_XRSE_SHIFT                                              25
#define PERI_CRG_CLKST9_ST_CLK_XRSE_MASK                                               0x02000000
#define PERI_CRG_CLKST9_ST_PCLK_CRG_HSS1_SHIFT                                         26
#define PERI_CRG_CLKST9_ST_PCLK_CRG_HSS1_MASK                                          0x04000000
#define PERI_CRG_CLKST9_ST_PCLK_CRG_MEDIA2_SHIFT                                       27
#define PERI_CRG_CLKST9_ST_PCLK_CRG_MEDIA2_MASK                                        0x08000000
#define PERI_CRG_CLKST9_ST_PCLK_CRG_MEDIA1_SHIFT                                       28
#define PERI_CRG_CLKST9_ST_PCLK_CRG_MEDIA1_MASK                                        0x10000000
#define PERI_CRG_CLKST9_ST_CLK_DDR_DFICLK_SHIFT                                        29
#define PERI_CRG_CLKST9_ST_CLK_DDR_DFICLK_MASK                                         0x20000000
#define PERI_CRG_CLKST9_ST_CLK_PERI2NPU_NOC_SHIFT                                      30
#define PERI_CRG_CLKST9_ST_CLK_PERI2NPU_NOC_MASK                                       0x40000000
#define PERI_CRG_CLKST9_ST_CLK_PERI2NPU_NIC_SHIFT                                      31
#define PERI_CRG_CLKST9_ST_CLK_PERI2NPU_NIC_MASK                                       0x80000000
#define PERI_CRG_CLKGT10_W1S                       0x0870
#define PERI_CRG_CLKGT10_W1S_GT_CLK_CFGBUS_LPIS_SHIFT                                  0
#define PERI_CRG_CLKGT10_W1S_GT_CLK_CFGBUS_HSS2_SHIFT                                  1
#define PERI_CRG_CLKGT10_W1S_GT_CLK_CFGBUS_MEDIA2_SHIFT                                3
#define PERI_CRG_CLKGT10_W1S_GT_CLK_CFGBUS_CPU_SHIFT                                   4
#define PERI_CRG_CLKGT10_W1S_GT_CLK_CFGBUS_NPU_SHIFT                                   5
#define PERI_CRG_CLKGT10_W1S_GT_CLK_CFGBUS_HSS1_SHIFT                                  10
#define PERI_CRG_CLKGT10_W1S_GT_CLK_CFGBUS_GPU_SHIFT                                   12
#define PERI_CRG_CLKGT10_W1S_GT_CLK_CFGBUS_XRSE_SHIFT                                  13
#define PERI_CRG_CLKGT10_W1S_GT_CLK_SYSBUS_HSS2_SHIFT                                  14
#define PERI_CRG_CLKGT10_W1S_GT_CLK_SYSBUS_CPU_SHIFT                                   15
#define PERI_CRG_CLKGT10_W1S_GT_CLK_SYSBUS_LPIS_SHIFT                                  17
#define PERI_CRG_CLKGT10_W1S_GT_CLK_SYSBUS_HSS1_SHIFT                                  18
#define PERI_CRG_CLKGT10_W1S_GT_CLK_SYSBUS_XRSE_SHIFT                                  19
#define PERI_CRG_CLKGT10_W1S_GT_CLK_MAINBUS_CPU_SHIFT                                  20
#define PERI_CRG_CLKGT10_W1S_GT_CLK_MAINBUS_NPU_SHIFT                                  21
#define PERI_CRG_CLKGT10_W1S_GT_CLK_MAINBUS_MEDIA2_SHIFT                               22
#define PERI_CRG_CLKGT10_W1S_GT_CLK_MAINBUS_PMPU_SHIFT                                 23
#define PERI_CRG_CLKGT10_W1S_GT_CLK_MAINBUS_GPU_SHIFT                                  27
#define PERI_CRG_CLKGT10_W1S_GT_CLK_MAINBUS_MEDIA1_SHIFT                               28
#define PERI_CRG_CLKGT10_W1C                       0x0874
#define PERI_CRG_CLKGT10_W1C_GT_CLK_CFGBUS_LPIS_SHIFT                                  0
#define PERI_CRG_CLKGT10_W1C_GT_CLK_CFGBUS_HSS2_SHIFT                                  1
#define PERI_CRG_CLKGT10_W1C_GT_CLK_CFGBUS_MEDIA2_SHIFT                                3
#define PERI_CRG_CLKGT10_W1C_GT_CLK_CFGBUS_CPU_SHIFT                                   4
#define PERI_CRG_CLKGT10_W1C_GT_CLK_CFGBUS_NPU_SHIFT                                   5
#define PERI_CRG_CLKGT10_W1C_GT_CLK_CFGBUS_HSS1_SHIFT                                  10
#define PERI_CRG_CLKGT10_W1C_GT_CLK_CFGBUS_GPU_SHIFT                                   12
#define PERI_CRG_CLKGT10_W1C_GT_CLK_CFGBUS_XRSE_SHIFT                                  13
#define PERI_CRG_CLKGT10_W1C_GT_CLK_SYSBUS_HSS2_SHIFT                                  14
#define PERI_CRG_CLKGT10_W1C_GT_CLK_SYSBUS_CPU_SHIFT                                   15
#define PERI_CRG_CLKGT10_W1C_GT_CLK_SYSBUS_LPIS_SHIFT                                  17
#define PERI_CRG_CLKGT10_W1C_GT_CLK_SYSBUS_HSS1_SHIFT                                  18
#define PERI_CRG_CLKGT10_W1C_GT_CLK_SYSBUS_XRSE_SHIFT                                  19
#define PERI_CRG_CLKGT10_W1C_GT_CLK_MAINBUS_CPU_SHIFT                                  20
#define PERI_CRG_CLKGT10_W1C_GT_CLK_MAINBUS_NPU_SHIFT                                  21
#define PERI_CRG_CLKGT10_W1C_GT_CLK_MAINBUS_MEDIA2_SHIFT                               22
#define PERI_CRG_CLKGT10_W1C_GT_CLK_MAINBUS_PMPU_SHIFT                                 23
#define PERI_CRG_CLKGT10_W1C_GT_CLK_MAINBUS_GPU_SHIFT                                  27
#define PERI_CRG_CLKGT10_W1C_GT_CLK_MAINBUS_MEDIA1_SHIFT                               28
#define PERI_CRG_CLKGT10_RO                        0x0878
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_LPIS_SHIFT                                   0
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_LPIS_MASK                                    0x00000001
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_HSS2_SHIFT                                   1
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_HSS2_MASK                                    0x00000002
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_MEDIA2_SHIFT                                 3
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_MEDIA2_MASK                                  0x00000008
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_CPU_SHIFT                                    4
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_CPU_MASK                                     0x00000010
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_NPU_SHIFT                                    5
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_NPU_MASK                                     0x00000020
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_HSS1_SHIFT                                   10
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_HSS1_MASK                                    0x00000400
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_GPU_SHIFT                                    12
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_GPU_MASK                                     0x00001000
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_XRSE_SHIFT                                   13
#define PERI_CRG_CLKGT10_RO_GT_CLK_CFGBUS_XRSE_MASK                                    0x00002000
#define PERI_CRG_CLKGT10_RO_GT_CLK_SYSBUS_HSS2_SHIFT                                   14
#define PERI_CRG_CLKGT10_RO_GT_CLK_SYSBUS_HSS2_MASK                                    0x00004000
#define PERI_CRG_CLKGT10_RO_GT_CLK_SYSBUS_CPU_SHIFT                                    15
#define PERI_CRG_CLKGT10_RO_GT_CLK_SYSBUS_CPU_MASK                                     0x00008000
#define PERI_CRG_CLKGT10_RO_GT_CLK_SYSBUS_LPIS_SHIFT                                   17
#define PERI_CRG_CLKGT10_RO_GT_CLK_SYSBUS_LPIS_MASK                                    0x00020000
#define PERI_CRG_CLKGT10_RO_GT_CLK_SYSBUS_HSS1_SHIFT                                   18
#define PERI_CRG_CLKGT10_RO_GT_CLK_SYSBUS_HSS1_MASK                                    0x00040000
#define PERI_CRG_CLKGT10_RO_GT_CLK_SYSBUS_XRSE_SHIFT                                   19
#define PERI_CRG_CLKGT10_RO_GT_CLK_SYSBUS_XRSE_MASK                                    0x00080000
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_CPU_SHIFT                                   20
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_CPU_MASK                                    0x00100000
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_NPU_SHIFT                                   21
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_NPU_MASK                                    0x00200000
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_MEDIA2_SHIFT                                22
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_MEDIA2_MASK                                 0x00400000
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_PMPU_SHIFT                                  23
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_PMPU_MASK                                   0x00800000
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_GPU_SHIFT                                   27
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_GPU_MASK                                    0x08000000
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_MEDIA1_SHIFT                                28
#define PERI_CRG_CLKGT10_RO_GT_CLK_MAINBUS_MEDIA1_MASK                                 0x10000000
#define PERI_CRG_CLKST10                           0x087c
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_LPIS_SHIFT                                      0
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_LPIS_MASK                                       0x00000001
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_HSS2_SHIFT                                      1
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_HSS2_MASK                                       0x00000002
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_MEDIA2_SHIFT                                    3
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_MEDIA2_MASK                                     0x00000008
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_CPU_SHIFT                                       4
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_CPU_MASK                                        0x00000010
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_NPU_SHIFT                                       5
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_NPU_MASK                                        0x00000020
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_HSS1_SHIFT                                      10
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_HSS1_MASK                                       0x00000400
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_GPU_SHIFT                                       12
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_GPU_MASK                                        0x00001000
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_XRSE_SHIFT                                      13
#define PERI_CRG_CLKST10_ST_CLK_CFGBUS_XRSE_MASK                                       0x00002000
#define PERI_CRG_CLKST10_ST_CLK_SYSBUS_HSS2_SHIFT                                      14
#define PERI_CRG_CLKST10_ST_CLK_SYSBUS_HSS2_MASK                                       0x00004000
#define PERI_CRG_CLKST10_ST_CLK_SYSBUS_CPU_SHIFT                                       15
#define PERI_CRG_CLKST10_ST_CLK_SYSBUS_CPU_MASK                                        0x00008000
#define PERI_CRG_CLKST10_ST_CLK_SYSBUS_LPIS_SHIFT                                      17
#define PERI_CRG_CLKST10_ST_CLK_SYSBUS_LPIS_MASK                                       0x00020000
#define PERI_CRG_CLKST10_ST_CLK_SYSBUS_HSS1_SHIFT                                      18
#define PERI_CRG_CLKST10_ST_CLK_SYSBUS_HSS1_MASK                                       0x00040000
#define PERI_CRG_CLKST10_ST_CLK_SYSBUS_XRSE_SHIFT                                      19
#define PERI_CRG_CLKST10_ST_CLK_SYSBUS_XRSE_MASK                                       0x00080000
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_CPU_SHIFT                                      20
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_CPU_MASK                                       0x00100000
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_NPU_SHIFT                                      21
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_NPU_MASK                                       0x00200000
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_MEDIA2_SHIFT                                   22
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_MEDIA2_MASK                                    0x00400000
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_PMPUA_SHIFT                                    23
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_PMPUA_MASK                                     0x00800000
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_PMPUB_SHIFT                                    24
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_PMPUB_MASK                                     0x01000000
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_PMPUC_SHIFT                                    25
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_PMPUC_MASK                                     0x02000000
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_PMPUD_SHIFT                                    26
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_PMPUD_MASK                                     0x04000000
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_GPU_SHIFT                                      27
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_GPU_MASK                                       0x08000000
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_MEDIA1_SHIFT                                   28
#define PERI_CRG_CLKST10_ST_CLK_MAINBUS_MEDIA1_MASK                                    0x10000000
#define PERI_CRG_RST3_W1S                          0x0880
#define PERI_CRG_RST3_W1S_IP_RST_LPISBUS2SYSBUS_NDB_MST_N_SHIFT                        0
#define PERI_CRG_RST3_W1S_IP_RST_LPCTRL2DDRCFG_NDB_MST_N_SHIFT                         1
#define PERI_CRG_RST3_W1S_IP_PRST_PERI_WDT2_N_SHIFT                                    7
#define PERI_CRG_RST3_W1S_IP_PRST_PERI_WDT3_N_SHIFT                                    8
#define PERI_CRG_RST3_W1S_IP_PRST_SPI0_N_SHIFT                                         9
#define PERI_CRG_RST3_W1S_IP_PRST_SPI1_N_SHIFT                                         10
#define PERI_CRG_RST3_W1S_IP_PRST_SPI2_N_SHIFT                                         11
#define PERI_CRG_RST3_W1S_IP_RST_PERI_TIMER4_N_SHIFT                                   12
#define PERI_CRG_RST3_W1S_IP_RST_PERI_TIMER5_N_SHIFT                                   13
#define PERI_CRG_RST3_W1S_IP_RST_PERI_TIMER6_N_SHIFT                                   14
#define PERI_CRG_RST3_W1S_IP_RST_PERI_TIMER7_N_SHIFT                                   15
#define PERI_CRG_RST3_W1S_IP_PRST_PERI_IPC0_N_SHIFT                                    16
#define PERI_CRG_RST3_W1C                          0x0884
#define PERI_CRG_RST3_W1C_IP_RST_LPISBUS2SYSBUS_NDB_MST_N_SHIFT                        0
#define PERI_CRG_RST3_W1C_IP_RST_LPCTRL2DDRCFG_NDB_MST_N_SHIFT                         1
#define PERI_CRG_RST3_W1C_IP_PRST_PERI_WDT2_N_SHIFT                                    7
#define PERI_CRG_RST3_W1C_IP_PRST_PERI_WDT3_N_SHIFT                                    8
#define PERI_CRG_RST3_W1C_IP_PRST_SPI0_N_SHIFT                                         9
#define PERI_CRG_RST3_W1C_IP_PRST_SPI1_N_SHIFT                                         10
#define PERI_CRG_RST3_W1C_IP_PRST_SPI2_N_SHIFT                                         11
#define PERI_CRG_RST3_W1C_IP_RST_PERI_TIMER4_N_SHIFT                                   12
#define PERI_CRG_RST3_W1C_IP_RST_PERI_TIMER5_N_SHIFT                                   13
#define PERI_CRG_RST3_W1C_IP_RST_PERI_TIMER6_N_SHIFT                                   14
#define PERI_CRG_RST3_W1C_IP_RST_PERI_TIMER7_N_SHIFT                                   15
#define PERI_CRG_RST3_W1C_IP_PRST_PERI_IPC0_N_SHIFT                                    16
#define PERI_CRG_RST3_RO                           0x0888
#define PERI_CRG_RST3_RO_IP_RST_LPISBUS2SYSBUS_NDB_MST_N_SHIFT                         0
#define PERI_CRG_RST3_RO_IP_RST_LPISBUS2SYSBUS_NDB_MST_N_MASK                          0x00000001
#define PERI_CRG_RST3_RO_IP_RST_LPCTRL2DDRCFG_NDB_MST_N_SHIFT                          1
#define PERI_CRG_RST3_RO_IP_RST_LPCTRL2DDRCFG_NDB_MST_N_MASK                           0x00000002
#define PERI_CRG_RST3_RO_IP_PRST_PERI_WDT2_N_SHIFT                                     7
#define PERI_CRG_RST3_RO_IP_PRST_PERI_WDT2_N_MASK                                      0x00000080
#define PERI_CRG_RST3_RO_IP_PRST_PERI_WDT3_N_SHIFT                                     8
#define PERI_CRG_RST3_RO_IP_PRST_PERI_WDT3_N_MASK                                      0x00000100
#define PERI_CRG_RST3_RO_IP_PRST_SPI0_N_SHIFT                                          9
#define PERI_CRG_RST3_RO_IP_PRST_SPI0_N_MASK                                           0x00000200
#define PERI_CRG_RST3_RO_IP_PRST_SPI1_N_SHIFT                                          10
#define PERI_CRG_RST3_RO_IP_PRST_SPI1_N_MASK                                           0x00000400
#define PERI_CRG_RST3_RO_IP_PRST_SPI2_N_SHIFT                                          11
#define PERI_CRG_RST3_RO_IP_PRST_SPI2_N_MASK                                           0x00000800
#define PERI_CRG_RST3_RO_IP_RST_PERI_TIMER4_N_SHIFT                                    12
#define PERI_CRG_RST3_RO_IP_RST_PERI_TIMER4_N_MASK                                     0x00001000
#define PERI_CRG_RST3_RO_IP_RST_PERI_TIMER5_N_SHIFT                                    13
#define PERI_CRG_RST3_RO_IP_RST_PERI_TIMER5_N_MASK                                     0x00002000
#define PERI_CRG_RST3_RO_IP_RST_PERI_TIMER6_N_SHIFT                                    14
#define PERI_CRG_RST3_RO_IP_RST_PERI_TIMER6_N_MASK                                     0x00004000
#define PERI_CRG_RST3_RO_IP_RST_PERI_TIMER7_N_SHIFT                                    15
#define PERI_CRG_RST3_RO_IP_RST_PERI_TIMER7_N_MASK                                     0x00008000
#define PERI_CRG_RST3_RO_IP_PRST_PERI_IPC0_N_SHIFT                                     16
#define PERI_CRG_RST3_RO_IP_PRST_PERI_IPC0_N_MASK                                      0x00010000
#define PERI_CRG_RST4_W1S                          0x0890
#define PERI_CRG_RST4_W1S_IP_PRST_PERI_GPIO_N_SHIFT                                    3
#define PERI_CRG_RST4_W1S_IP_PRST_PCTRL_N_SHIFT                                        9
#define PERI_CRG_RST4_W1S_IP_PRST_PERI_IOCTRL_N_SHIFT                                  15
#define PERI_CRG_RST4_W1S_IP_PRST_PERIL_IOPADWRAP_N_SHIFT                              16
#define PERI_CRG_RST4_W1S_IP_PRST_PERIR_IOPADWRAP_N_SHIFT                              17
#define PERI_CRG_RST4_W1S_IP_RST_ONEWIRE0_N_SHIFT                                      22
#define PERI_CRG_RST4_W1S_IP_RST_ONEWIRE1_N_SHIFT                                      23
#define PERI_CRG_RST4_W1S_IP_RST_ONEWIRE2_N_SHIFT                                      24
#define PERI_CRG_RST4_W1S_IP_PRST_ONEWIRE0_N_SHIFT                                     25
#define PERI_CRG_RST4_W1S_IP_PRST_ONEWIRE1_N_SHIFT                                     26
#define PERI_CRG_RST4_W1S_IP_PRST_ONEWIRE2_N_SHIFT                                     27
#define PERI_CRG_RST4_W1C                          0x0894
#define PERI_CRG_RST4_W1C_IP_PRST_PERI_GPIO_N_SHIFT                                    3
#define PERI_CRG_RST4_W1C_IP_PRST_PCTRL_N_SHIFT                                        9
#define PERI_CRG_RST4_W1C_IP_PRST_PERI_IOCTRL_N_SHIFT                                  15
#define PERI_CRG_RST4_W1C_IP_PRST_PERIL_IOPADWRAP_N_SHIFT                              16
#define PERI_CRG_RST4_W1C_IP_PRST_PERIR_IOPADWRAP_N_SHIFT                              17
#define PERI_CRG_RST4_W1C_IP_RST_ONEWIRE0_N_SHIFT                                      22
#define PERI_CRG_RST4_W1C_IP_RST_ONEWIRE1_N_SHIFT                                      23
#define PERI_CRG_RST4_W1C_IP_RST_ONEWIRE2_N_SHIFT                                      24
#define PERI_CRG_RST4_W1C_IP_PRST_ONEWIRE0_N_SHIFT                                     25
#define PERI_CRG_RST4_W1C_IP_PRST_ONEWIRE1_N_SHIFT                                     26
#define PERI_CRG_RST4_W1C_IP_PRST_ONEWIRE2_N_SHIFT                                     27
#define PERI_CRG_RST4_RO                           0x0898
#define PERI_CRG_RST4_RO_IP_PRST_PERI_GPIO_N_SHIFT                                     3
#define PERI_CRG_RST4_RO_IP_PRST_PERI_GPIO_N_MASK                                      0x00000008
#define PERI_CRG_RST4_RO_IP_PRST_PCTRL_N_SHIFT                                         9
#define PERI_CRG_RST4_RO_IP_PRST_PCTRL_N_MASK                                          0x00000200
#define PERI_CRG_RST4_RO_IP_PRST_PERI_IOCTRL_N_SHIFT                                   15
#define PERI_CRG_RST4_RO_IP_PRST_PERI_IOCTRL_N_MASK                                    0x00008000
#define PERI_CRG_RST4_RO_IP_PRST_PERIL_IOPADWRAP_N_SHIFT                               16
#define PERI_CRG_RST4_RO_IP_PRST_PERIL_IOPADWRAP_N_MASK                                0x00010000
#define PERI_CRG_RST4_RO_IP_PRST_PERIR_IOPADWRAP_N_SHIFT                               17
#define PERI_CRG_RST4_RO_IP_PRST_PERIR_IOPADWRAP_N_MASK                                0x00020000
#define PERI_CRG_RST4_RO_IP_RST_ONEWIRE0_N_SHIFT                                       22
#define PERI_CRG_RST4_RO_IP_RST_ONEWIRE0_N_MASK                                        0x00400000
#define PERI_CRG_RST4_RO_IP_RST_ONEWIRE1_N_SHIFT                                       23
#define PERI_CRG_RST4_RO_IP_RST_ONEWIRE1_N_MASK                                        0x00800000
#define PERI_CRG_RST4_RO_IP_RST_ONEWIRE2_N_SHIFT                                       24
#define PERI_CRG_RST4_RO_IP_RST_ONEWIRE2_N_MASK                                        0x01000000
#define PERI_CRG_RST4_RO_IP_PRST_ONEWIRE0_N_SHIFT                                      25
#define PERI_CRG_RST4_RO_IP_PRST_ONEWIRE0_N_MASK                                       0x02000000
#define PERI_CRG_RST4_RO_IP_PRST_ONEWIRE1_N_SHIFT                                      26
#define PERI_CRG_RST4_RO_IP_PRST_ONEWIRE1_N_MASK                                       0x04000000
#define PERI_CRG_RST4_RO_IP_PRST_ONEWIRE2_N_SHIFT                                      27
#define PERI_CRG_RST4_RO_IP_PRST_ONEWIRE2_N_MASK                                       0x08000000
#define PERI_CRG_RST5_W1S                          0x08a0
#define PERI_CRG_RST5_W1S_IP_RST_FC_N_SHIFT                                            0
#define PERI_CRG_RST5_W1S_IP_RST_SYS1_TPC_N_SHIFT                                      1
#define PERI_CRG_RST5_W1S_IP_RST_NPU_TPC_N_SHIFT                                       2
#define PERI_CRG_RST5_W1S_IP_RST_PERI_GPIODB_N_SHIFT                                   8
#define PERI_CRG_RST5_W1S_IP_PRST_PERI_TIMER_S_N_SHIFT                                 11
#define PERI_CRG_RST5_W1S_IP_RST_PERI_WDT2_N_SHIFT                                     14
#define PERI_CRG_RST5_W1S_IP_RST_PERI_WDT3_N_SHIFT                                     15
#define PERI_CRG_RST5_W1S_IP_RST_SPI0_N_SHIFT                                          16
#define PERI_CRG_RST5_W1S_IP_RST_SPI1_N_SHIFT                                          17
#define PERI_CRG_RST5_W1S_IP_RST_SPI2_N_SHIFT                                          18
#define PERI_CRG_RST5_W1C                          0x08a4
#define PERI_CRG_RST5_W1C_IP_RST_FC_N_SHIFT                                            0
#define PERI_CRG_RST5_W1C_IP_RST_SYS1_TPC_N_SHIFT                                      1
#define PERI_CRG_RST5_W1C_IP_RST_NPU_TPC_N_SHIFT                                       2
#define PERI_CRG_RST5_W1C_IP_RST_PERI_GPIODB_N_SHIFT                                   8
#define PERI_CRG_RST5_W1C_IP_PRST_PERI_TIMER_S_N_SHIFT                                 11
#define PERI_CRG_RST5_W1C_IP_RST_PERI_WDT2_N_SHIFT                                     14
#define PERI_CRG_RST5_W1C_IP_RST_PERI_WDT3_N_SHIFT                                     15
#define PERI_CRG_RST5_W1C_IP_RST_SPI0_N_SHIFT                                          16
#define PERI_CRG_RST5_W1C_IP_RST_SPI1_N_SHIFT                                          17
#define PERI_CRG_RST5_W1C_IP_RST_SPI2_N_SHIFT                                          18
#define PERI_CRG_RST5_RO                           0x08a8
#define PERI_CRG_RST5_RO_IP_RST_FC_N_SHIFT                                             0
#define PERI_CRG_RST5_RO_IP_RST_FC_N_MASK                                              0x00000001
#define PERI_CRG_RST5_RO_IP_RST_SYS1_TPC_N_SHIFT                                       1
#define PERI_CRG_RST5_RO_IP_RST_SYS1_TPC_N_MASK                                        0x00000002
#define PERI_CRG_RST5_RO_IP_RST_NPU_TPC_N_SHIFT                                        2
#define PERI_CRG_RST5_RO_IP_RST_NPU_TPC_N_MASK                                         0x00000004
#define PERI_CRG_RST5_RO_IP_RST_PERI_GPIODB_N_SHIFT                                    8
#define PERI_CRG_RST5_RO_IP_RST_PERI_GPIODB_N_MASK                                     0x00000100
#define PERI_CRG_RST5_RO_IP_PRST_PERI_TIMER_S_N_SHIFT                                  11
#define PERI_CRG_RST5_RO_IP_PRST_PERI_TIMER_S_N_MASK                                   0x00000800
#define PERI_CRG_RST5_RO_IP_RST_PERI_WDT2_N_SHIFT                                      14
#define PERI_CRG_RST5_RO_IP_RST_PERI_WDT2_N_MASK                                       0x00004000
#define PERI_CRG_RST5_RO_IP_RST_PERI_WDT3_N_SHIFT                                      15
#define PERI_CRG_RST5_RO_IP_RST_PERI_WDT3_N_MASK                                       0x00008000
#define PERI_CRG_RST5_RO_IP_RST_SPI0_N_SHIFT                                           16
#define PERI_CRG_RST5_RO_IP_RST_SPI0_N_MASK                                            0x00010000
#define PERI_CRG_RST5_RO_IP_RST_SPI1_N_SHIFT                                           17
#define PERI_CRG_RST5_RO_IP_RST_SPI1_N_MASK                                            0x00020000
#define PERI_CRG_RST5_RO_IP_RST_SPI2_N_SHIFT                                           18
#define PERI_CRG_RST5_RO_IP_RST_SPI2_N_MASK                                            0x00040000
#define PERI_CRG_RST6_W1S                          0x08b0
#define PERI_CRG_RST6_W1S_IP_RST_PPLL0_LOGIC_N_SHIFT                                   0
#define PERI_CRG_RST6_W1S_IP_RST_PPLL1_LOGIC_N_SHIFT                                   1
#define PERI_CRG_RST6_W1S_IP_RST_PPLL2_LOGIC_N_SHIFT                                   2
#define PERI_CRG_RST6_W1S_IP_RST_PPLL3_LOGIC_N_SHIFT                                   3
#define PERI_CRG_RST6_W1S_IP_RST_GPLL_LOGIC_N_SHIFT                                    4
#define PERI_CRG_RST6_W1S_IP_RST_DDRPPLL0_LOGIC_N_SHIFT                                5
#define PERI_CRG_RST6_W1S_IP_RST_DDRPPLL1_LOGIC_N_SHIFT                                6
#define PERI_CRG_RST6_W1S_IP_RST_SSMOD_PPLL0_N_SHIFT                                   7
#define PERI_CRG_RST6_W1S_IP_RST_SSMOD_PPLL1_N_SHIFT                                   8
#define PERI_CRG_RST6_W1S_IP_RST_SSMOD_PPLL2_N_SHIFT                                   9
#define PERI_CRG_RST6_W1S_IP_RST_SSMOD_PPLL3_N_SHIFT                                   10
#define PERI_CRG_RST6_W1S_IP_RST_SSMOD_GPLL_N_SHIFT                                    11
#define PERI_CRG_RST6_W1S_IP_RST_SSMOD_DDRPPLL0_N_SHIFT                                12
#define PERI_CRG_RST6_W1S_IP_RST_SSMOD_DDRPPLL1_N_SHIFT                                13
#define PERI_CRG_RST6_W1S_IP_RST_BROADCAST_N_SHIFT                                     21
#define PERI_CRG_RST6_W1S_IP_RST_PMPU_DDR_N_SHIFT                                      24
#define PERI_CRG_RST6_W1C                          0x08b4
#define PERI_CRG_RST6_W1C_IP_RST_PPLL0_LOGIC_N_SHIFT                                   0
#define PERI_CRG_RST6_W1C_IP_RST_PPLL1_LOGIC_N_SHIFT                                   1
#define PERI_CRG_RST6_W1C_IP_RST_PPLL2_LOGIC_N_SHIFT                                   2
#define PERI_CRG_RST6_W1C_IP_RST_PPLL3_LOGIC_N_SHIFT                                   3
#define PERI_CRG_RST6_W1C_IP_RST_GPLL_LOGIC_N_SHIFT                                    4
#define PERI_CRG_RST6_W1C_IP_RST_DDRPPLL0_LOGIC_N_SHIFT                                5
#define PERI_CRG_RST6_W1C_IP_RST_DDRPPLL1_LOGIC_N_SHIFT                                6
#define PERI_CRG_RST6_W1C_IP_RST_SSMOD_PPLL0_N_SHIFT                                   7
#define PERI_CRG_RST6_W1C_IP_RST_SSMOD_PPLL1_N_SHIFT                                   8
#define PERI_CRG_RST6_W1C_IP_RST_SSMOD_PPLL2_N_SHIFT                                   9
#define PERI_CRG_RST6_W1C_IP_RST_SSMOD_PPLL3_N_SHIFT                                   10
#define PERI_CRG_RST6_W1C_IP_RST_SSMOD_GPLL_N_SHIFT                                    11
#define PERI_CRG_RST6_W1C_IP_RST_SSMOD_DDRPPLL0_N_SHIFT                                12
#define PERI_CRG_RST6_W1C_IP_RST_SSMOD_DDRPPLL1_N_SHIFT                                13
#define PERI_CRG_RST6_W1C_IP_RST_BROADCAST_N_SHIFT                                     21
#define PERI_CRG_RST6_W1C_IP_RST_PMPU_DDR_N_SHIFT                                      24
#define PERI_CRG_RST6_RO                           0x08b8
#define PERI_CRG_RST6_RO_IP_RST_PPLL0_LOGIC_N_SHIFT                                    0
#define PERI_CRG_RST6_RO_IP_RST_PPLL0_LOGIC_N_MASK                                     0x00000001
#define PERI_CRG_RST6_RO_IP_RST_PPLL1_LOGIC_N_SHIFT                                    1
#define PERI_CRG_RST6_RO_IP_RST_PPLL1_LOGIC_N_MASK                                     0x00000002
#define PERI_CRG_RST6_RO_IP_RST_PPLL2_LOGIC_N_SHIFT                                    2
#define PERI_CRG_RST6_RO_IP_RST_PPLL2_LOGIC_N_MASK                                     0x00000004
#define PERI_CRG_RST6_RO_IP_RST_PPLL3_LOGIC_N_SHIFT                                    3
#define PERI_CRG_RST6_RO_IP_RST_PPLL3_LOGIC_N_MASK                                     0x00000008
#define PERI_CRG_RST6_RO_IP_RST_GPLL_LOGIC_N_SHIFT                                     4
#define PERI_CRG_RST6_RO_IP_RST_GPLL_LOGIC_N_MASK                                      0x00000010
#define PERI_CRG_RST6_RO_IP_RST_DDRPPLL0_LOGIC_N_SHIFT                                 5
#define PERI_CRG_RST6_RO_IP_RST_DDRPPLL0_LOGIC_N_MASK                                  0x00000020
#define PERI_CRG_RST6_RO_IP_RST_DDRPPLL1_LOGIC_N_SHIFT                                 6
#define PERI_CRG_RST6_RO_IP_RST_DDRPPLL1_LOGIC_N_MASK                                  0x00000040
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_PPLL0_N_SHIFT                                    7
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_PPLL0_N_MASK                                     0x00000080
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_PPLL1_N_SHIFT                                    8
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_PPLL1_N_MASK                                     0x00000100
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_PPLL2_N_SHIFT                                    9
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_PPLL2_N_MASK                                     0x00000200
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_PPLL3_N_SHIFT                                    10
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_PPLL3_N_MASK                                     0x00000400
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_GPLL_N_SHIFT                                     11
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_GPLL_N_MASK                                      0x00000800
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_DDRPPLL0_N_SHIFT                                 12
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_DDRPPLL0_N_MASK                                  0x00001000
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_DDRPPLL1_N_SHIFT                                 13
#define PERI_CRG_RST6_RO_IP_RST_SSMOD_DDRPPLL1_N_MASK                                  0x00002000
#define PERI_CRG_RST6_RO_IP_RST_BROADCAST_N_SHIFT                                      21
#define PERI_CRG_RST6_RO_IP_RST_BROADCAST_N_MASK                                       0x00200000
#define PERI_CRG_RST6_RO_IP_RST_PMPU_DDR_N_SHIFT                                       24
#define PERI_CRG_RST6_RO_IP_RST_PMPU_DDR_N_MASK                                        0x01000000
#define PERI_CRG_RST7_W1S                          0x08c0
#define PERI_CRG_RST7_W1S_IP_RST_AXIBIST_N_SHIFT                                       3
#define PERI_CRG_RST7_W1S_IP_PRST_DMA_BUS_N_SHIFT                                      4
#define PERI_CRG_RST7_W1S_IP_RST_DMA_BUS_N_SHIFT                                       6
#define PERI_CRG_RST7_W1S_IP_RST_DEBUG_N_SHIFT                                         12
#define PERI_CRG_RST7_W1S_IP_RST_DDR_ADB400_SLV_N_SHIFT                                13
#define PERI_CRG_RST7_W1S_IP_RST_NPU_FT_N_SHIFT                                        16
#define PERI_CRG_RST7_W1S_IP_RST_MAINBUS_N_SHIFT                                       17
#define PERI_CRG_RST7_W1S_IP_RST_SYSBUS_N_SHIFT                                        18
#define PERI_CRG_RST7_W1S_IP_RST_CFGBUS_N_SHIFT                                        19
#define PERI_CRG_RST7_W1S_IP_RST_PERICFG_BUS_N_SHIFT                                   20
#define PERI_CRG_RST7_W1S_IP_RST_NPU_ADB_MST_N_SHIFT                                   21
#define PERI_CRG_RST7_W1S_IP_RST_NPU_OCM_ADB_MST_N_SHIFT                               22
#define PERI_CRG_RST7_W1C                          0x08c4
#define PERI_CRG_RST7_W1C_IP_RST_AXIBIST_N_SHIFT                                       3
#define PERI_CRG_RST7_W1C_IP_PRST_DMA_BUS_N_SHIFT                                      4
#define PERI_CRG_RST7_W1C_IP_RST_DMA_BUS_N_SHIFT                                       6
#define PERI_CRG_RST7_W1C_IP_RST_DEBUG_N_SHIFT                                         12
#define PERI_CRG_RST7_W1C_IP_RST_DDR_ADB400_SLV_N_SHIFT                                13
#define PERI_CRG_RST7_W1C_IP_RST_NPU_FT_N_SHIFT                                        16
#define PERI_CRG_RST7_W1C_IP_RST_MAINBUS_N_SHIFT                                       17
#define PERI_CRG_RST7_W1C_IP_RST_SYSBUS_N_SHIFT                                        18
#define PERI_CRG_RST7_W1C_IP_RST_CFGBUS_N_SHIFT                                        19
#define PERI_CRG_RST7_W1C_IP_RST_PERICFG_BUS_N_SHIFT                                   20
#define PERI_CRG_RST7_W1C_IP_RST_NPU_ADB_MST_N_SHIFT                                   21
#define PERI_CRG_RST7_W1C_IP_RST_NPU_OCM_ADB_MST_N_SHIFT                               22
#define PERI_CRG_RST7_RO                           0x08c8
#define PERI_CRG_RST7_RO_IP_RST_AXIBIST_N_SHIFT                                        3
#define PERI_CRG_RST7_RO_IP_RST_AXIBIST_N_MASK                                         0x00000008
#define PERI_CRG_RST7_RO_IP_PRST_DMA_BUS_N_SHIFT                                       4
#define PERI_CRG_RST7_RO_IP_PRST_DMA_BUS_N_MASK                                        0x00000010
#define PERI_CRG_RST7_RO_IP_RST_DMA_BUS_N_SHIFT                                        6
#define PERI_CRG_RST7_RO_IP_RST_DMA_BUS_N_MASK                                         0x00000040
#define PERI_CRG_RST7_RO_IP_RST_DEBUG_N_SHIFT                                          12
#define PERI_CRG_RST7_RO_IP_RST_DEBUG_N_MASK                                           0x00001000
#define PERI_CRG_RST7_RO_IP_RST_DDR_ADB400_SLV_N_SHIFT                                 13
#define PERI_CRG_RST7_RO_IP_RST_DDR_ADB400_SLV_N_MASK                                  0x00002000
#define PERI_CRG_RST7_RO_IP_RST_NPU_FT_N_SHIFT                                         16
#define PERI_CRG_RST7_RO_IP_RST_NPU_FT_N_MASK                                          0x00010000
#define PERI_CRG_RST7_RO_IP_RST_MAINBUS_N_SHIFT                                        17
#define PERI_CRG_RST7_RO_IP_RST_MAINBUS_N_MASK                                         0x00020000
#define PERI_CRG_RST7_RO_IP_RST_SYSBUS_N_SHIFT                                         18
#define PERI_CRG_RST7_RO_IP_RST_SYSBUS_N_MASK                                          0x00040000
#define PERI_CRG_RST7_RO_IP_RST_CFGBUS_N_SHIFT                                         19
#define PERI_CRG_RST7_RO_IP_RST_CFGBUS_N_MASK                                          0x00080000
#define PERI_CRG_RST7_RO_IP_RST_PERICFG_BUS_N_SHIFT                                    20
#define PERI_CRG_RST7_RO_IP_RST_PERICFG_BUS_N_MASK                                     0x00100000
#define PERI_CRG_RST7_RO_IP_RST_NPU_ADB_MST_N_SHIFT                                    21
#define PERI_CRG_RST7_RO_IP_RST_NPU_ADB_MST_N_MASK                                     0x00200000
#define PERI_CRG_RST7_RO_IP_RST_NPU_OCM_ADB_MST_N_SHIFT                                22
#define PERI_CRG_RST7_RO_IP_RST_NPU_OCM_ADB_MST_N_MASK                                 0x00400000
#define PERI_CRG_RST8_W1S                          0x08d0
#define PERI_CRG_RST8_W1S_IP_RST_CPU_BUS_N_SHIFT                                       0
#define PERI_CRG_RST8_W1S_IP_RST_XCTRL_DDR_N_SHIFT                                     1
#define PERI_CRG_RST8_W1S_IP_RST_XCTRL_CPU_N_SHIFT                                     2
#define PERI_CRG_RST8_W1S_IP_RST_HSS2_N_SHIFT                                          3
#define PERI_CRG_RST8_W1S_IP_RST_HSS1_N_SHIFT                                          4
#define PERI_CRG_RST8_W1S_IP_RST_MEDIA2_N_SHIFT                                        5
#define PERI_CRG_RST8_W1S_IP_RST_MEDIA1_N_SHIFT                                        6
#define PERI_CRG_RST8_W1S_IP_RST_NPU_N_SHIFT                                           7
#define PERI_CRG_RST8_W1S_IP_RST_GPU_N_SHIFT                                           8
#define PERI_CRG_RST8_W1S_IP_RST_CPU_N_SHIFT                                           9
#define PERI_CRG_RST8_W1S_IP_RST_XCTRL_DDR_CRG_N_SHIFT                                 10
#define PERI_CRG_RST8_W1S_IP_RST_XCTRL_CPU_CRG_N_SHIFT                                 11
#define PERI_CRG_RST8_W1S_IP_RST_HSS2_CRG_N_SHIFT                                      12
#define PERI_CRG_RST8_W1S_IP_RST_HSS1_CRG_N_SHIFT                                      13
#define PERI_CRG_RST8_W1S_IP_RST_MEDIA2_CRG_N_SHIFT                                    15
#define PERI_CRG_RST8_W1S_IP_RST_MEDIA1_CRG_N_SHIFT                                    16
#define PERI_CRG_RST8_W1S_IP_RST_NPU_CRG_N_SHIFT                                       17
#define PERI_CRG_RST8_W1S_IP_RST_GPU_CRG_N_SHIFT                                       18
#define PERI_CRG_RST8_W1S_IP_RST_CPU_CRG_N_SHIFT                                       19
#define PERI_CRG_RST8_W1S_IP_RST_DDR_CRG_N_SHIFT                                       22
#define PERI_CRG_RST8_W1S_IP_RST_DDR_N_SHIFT                                           23
#define PERI_CRG_RST8_W1S_IP_RST_MEDIA2_FT_N_SHIFT                                     29
#define PERI_CRG_RST8_W1S_IP_RST_MEDIA1_FT_N_SHIFT                                     30
#define PERI_CRG_RST8_W1C                          0x08d4
#define PERI_CRG_RST8_W1C_IP_RST_CPU_BUS_N_SHIFT                                       0
#define PERI_CRG_RST8_W1C_IP_RST_XCTRL_DDR_N_SHIFT                                     1
#define PERI_CRG_RST8_W1C_IP_RST_XCTRL_CPU_N_SHIFT                                     2
#define PERI_CRG_RST8_W1C_IP_RST_HSS2_N_SHIFT                                          3
#define PERI_CRG_RST8_W1C_IP_RST_HSS1_N_SHIFT                                          4
#define PERI_CRG_RST8_W1C_IP_RST_MEDIA2_N_SHIFT                                        5
#define PERI_CRG_RST8_W1C_IP_RST_MEDIA1_N_SHIFT                                        6
#define PERI_CRG_RST8_W1C_IP_RST_NPU_N_SHIFT                                           7
#define PERI_CRG_RST8_W1C_IP_RST_GPU_N_SHIFT                                           8
#define PERI_CRG_RST8_W1C_IP_RST_CPU_N_SHIFT                                           9
#define PERI_CRG_RST8_W1C_IP_RST_XCTRL_DDR_CRG_N_SHIFT                                 10
#define PERI_CRG_RST8_W1C_IP_RST_XCTRL_CPU_CRG_N_SHIFT                                 11
#define PERI_CRG_RST8_W1C_IP_RST_HSS2_CRG_N_SHIFT                                      12
#define PERI_CRG_RST8_W1C_IP_RST_HSS1_CRG_N_SHIFT                                      13
#define PERI_CRG_RST8_W1C_IP_RST_MEDIA2_CRG_N_SHIFT                                    15
#define PERI_CRG_RST8_W1C_IP_RST_MEDIA1_CRG_N_SHIFT                                    16
#define PERI_CRG_RST8_W1C_IP_RST_NPU_CRG_N_SHIFT                                       17
#define PERI_CRG_RST8_W1C_IP_RST_GPU_CRG_N_SHIFT                                       18
#define PERI_CRG_RST8_W1C_IP_RST_CPU_CRG_N_SHIFT                                       19
#define PERI_CRG_RST8_W1C_IP_RST_DDR_CRG_N_SHIFT                                       22
#define PERI_CRG_RST8_W1C_IP_RST_DDR_N_SHIFT                                           23
#define PERI_CRG_RST8_W1C_IP_RST_MEDIA2_FT_N_SHIFT                                     29
#define PERI_CRG_RST8_W1C_IP_RST_MEDIA1_FT_N_SHIFT                                     30
#define PERI_CRG_RST8_RO                           0x08d8
#define PERI_CRG_RST8_RO_IP_RST_CPU_BUS_N_SHIFT                                        0
#define PERI_CRG_RST8_RO_IP_RST_CPU_BUS_N_MASK                                         0x00000001
#define PERI_CRG_RST8_RO_IP_RST_XCTRL_DDR_N_SHIFT                                      1
#define PERI_CRG_RST8_RO_IP_RST_XCTRL_DDR_N_MASK                                       0x00000002
#define PERI_CRG_RST8_RO_IP_RST_XCTRL_CPU_N_SHIFT                                      2
#define PERI_CRG_RST8_RO_IP_RST_XCTRL_CPU_N_MASK                                       0x00000004
#define PERI_CRG_RST8_RO_IP_RST_HSS2_N_SHIFT                                           3
#define PERI_CRG_RST8_RO_IP_RST_HSS2_N_MASK                                            0x00000008
#define PERI_CRG_RST8_RO_IP_RST_HSS1_N_SHIFT                                           4
#define PERI_CRG_RST8_RO_IP_RST_HSS1_N_MASK                                            0x00000010
#define PERI_CRG_RST8_RO_IP_RST_MEDIA2_N_SHIFT                                         5
#define PERI_CRG_RST8_RO_IP_RST_MEDIA2_N_MASK                                          0x00000020
#define PERI_CRG_RST8_RO_IP_RST_MEDIA1_N_SHIFT                                         6
#define PERI_CRG_RST8_RO_IP_RST_MEDIA1_N_MASK                                          0x00000040
#define PERI_CRG_RST8_RO_IP_RST_NPU_N_SHIFT                                            7
#define PERI_CRG_RST8_RO_IP_RST_NPU_N_MASK                                             0x00000080
#define PERI_CRG_RST8_RO_IP_RST_GPU_N_SHIFT                                            8
#define PERI_CRG_RST8_RO_IP_RST_GPU_N_MASK                                             0x00000100
#define PERI_CRG_RST8_RO_IP_RST_CPU_N_SHIFT                                            9
#define PERI_CRG_RST8_RO_IP_RST_CPU_N_MASK                                             0x00000200
#define PERI_CRG_RST8_RO_IP_RST_XCTRL_DDR_CRG_N_SHIFT                                  10
#define PERI_CRG_RST8_RO_IP_RST_XCTRL_DDR_CRG_N_MASK                                   0x00000400
#define PERI_CRG_RST8_RO_IP_RST_XCTRL_CPU_CRG_N_SHIFT                                  11
#define PERI_CRG_RST8_RO_IP_RST_XCTRL_CPU_CRG_N_MASK                                   0x00000800
#define PERI_CRG_RST8_RO_IP_RST_HSS2_CRG_N_SHIFT                                       12
#define PERI_CRG_RST8_RO_IP_RST_HSS2_CRG_N_MASK                                        0x00001000
#define PERI_CRG_RST8_RO_IP_RST_HSS1_CRG_N_SHIFT                                       13
#define PERI_CRG_RST8_RO_IP_RST_HSS1_CRG_N_MASK                                        0x00002000
#define PERI_CRG_RST8_RO_IP_RST_MEDIA2_CRG_N_SHIFT                                     15
#define PERI_CRG_RST8_RO_IP_RST_MEDIA2_CRG_N_MASK                                      0x00008000
#define PERI_CRG_RST8_RO_IP_RST_MEDIA1_CRG_N_SHIFT                                     16
#define PERI_CRG_RST8_RO_IP_RST_MEDIA1_CRG_N_MASK                                      0x00010000
#define PERI_CRG_RST8_RO_IP_RST_NPU_CRG_N_SHIFT                                        17
#define PERI_CRG_RST8_RO_IP_RST_NPU_CRG_N_MASK                                         0x00020000
#define PERI_CRG_RST8_RO_IP_RST_GPU_CRG_N_SHIFT                                        18
#define PERI_CRG_RST8_RO_IP_RST_GPU_CRG_N_MASK                                         0x00040000
#define PERI_CRG_RST8_RO_IP_RST_CPU_CRG_N_SHIFT                                        19
#define PERI_CRG_RST8_RO_IP_RST_CPU_CRG_N_MASK                                         0x00080000
#define PERI_CRG_RST8_RO_IP_RST_DDR_CRG_N_SHIFT                                        22
#define PERI_CRG_RST8_RO_IP_RST_DDR_CRG_N_MASK                                         0x00400000
#define PERI_CRG_RST8_RO_IP_RST_DDR_N_SHIFT                                            23
#define PERI_CRG_RST8_RO_IP_RST_DDR_N_MASK                                             0x00800000
#define PERI_CRG_RST8_RO_IP_RST_MEDIA2_FT_N_SHIFT                                      29
#define PERI_CRG_RST8_RO_IP_RST_MEDIA2_FT_N_MASK                                       0x20000000
#define PERI_CRG_RST8_RO_IP_RST_MEDIA1_FT_N_SHIFT                                      30
#define PERI_CRG_RST8_RO_IP_RST_MEDIA1_FT_N_MASK                                       0x40000000
#define PERI_CRG_CLKDIV5                           0x0900
#define PERI_CRG_CLKDIV5_DIV_CLK_MAINBUS_DATA_LPPLL_SHIFT                              0
#define PERI_CRG_CLKDIV5_DIV_CLK_MAINBUS_DATA_LPPLL_MASK                               0x0000003f
#define PERI_CRG_CLKDIV5_DIV_CLK_MAINBUS_DATA_GPLL_SHIFT                               6
#define PERI_CRG_CLKDIV5_DIV_CLK_MAINBUS_DATA_GPLL_MASK                                0x00000fc0
#define PERI_CRG_CLKDIV5_SC_GT_CLK_MAINBUS_DATA_LPPLL_DIV_SHIFT                        12
#define PERI_CRG_CLKDIV5_SC_GT_CLK_MAINBUS_DATA_LPPLL_DIV_MASK                         0x00001000
#define PERI_CRG_CLKDIV5_SC_GT_CLK_MAINBUS_DATA_GPLL_DIV_SHIFT                         13
#define PERI_CRG_CLKDIV5_SC_GT_CLK_MAINBUS_DATA_GPLL_DIV_MASK                          0x00002000
#define PERI_CRG_CLKDIV6                           0x0904
#define PERI_CRG_CLKDIV6_DIV_CLK_SYSBUS_DATA_LPPLL_SHIFT                               0
#define PERI_CRG_CLKDIV6_DIV_CLK_SYSBUS_DATA_LPPLL_MASK                                0x0000003f
#define PERI_CRG_CLKDIV6_SC_GT_CLK_SYSBUS_DATA_LPPLL_DIV_SHIFT                         12
#define PERI_CRG_CLKDIV6_SC_GT_CLK_SYSBUS_DATA_LPPLL_DIV_MASK                          0x00001000
#define PERI_CRG_CLKDIV7                           0x0908
#define PERI_CRG_CLKDIV7_DIV_CLK_SYSBUS_DATA_GPLL_SHIFT                                0
#define PERI_CRG_CLKDIV7_DIV_CLK_SYSBUS_DATA_GPLL_MASK                                 0x0000003f
#define PERI_CRG_CLKDIV7_DIV_CLK_CFGBUS_GPLL_SHIFT                                     6
#define PERI_CRG_CLKDIV7_DIV_CLK_CFGBUS_GPLL_MASK                                      0x00000fc0
#define PERI_CRG_CLKDIV7_SC_GT_CLK_SYSBUS_DATA_GPLL_DIV_SHIFT                          12
#define PERI_CRG_CLKDIV7_SC_GT_CLK_SYSBUS_DATA_GPLL_DIV_MASK                           0x00001000
#define PERI_CRG_CLKDIV7_SC_GT_CLK_CFGBUS_GPLL_SHIFT                                   13
#define PERI_CRG_CLKDIV7_SC_GT_CLK_CFGBUS_GPLL_MASK                                    0x00002000
#define PERI_CRG_CLKDIV8                           0x090c
#define PERI_CRG_CLKDIV8_DIV_CLK_PERICFG_BUS_SHIFT                                     0
#define PERI_CRG_CLKDIV8_DIV_CLK_PERICFG_BUS_MASK                                      0x0000003f
#define PERI_CRG_CLKDIV8_DIV_CLK_CFGBUS_LPPLL_SHIFT                                    6
#define PERI_CRG_CLKDIV8_DIV_CLK_CFGBUS_LPPLL_MASK                                     0x00000fc0
#define PERI_CRG_CLKDIV8_SC_GT_CLK_PERICFG_BUS_SHIFT                                   12
#define PERI_CRG_CLKDIV8_SC_GT_CLK_PERICFG_BUS_MASK                                    0x00001000
#define PERI_CRG_CLKDIV8_SC_GT_CLK_CFGBUS_LPPLL_SHIFT                                  13
#define PERI_CRG_CLKDIV8_SC_GT_CLK_CFGBUS_LPPLL_MASK                                   0x00002000
#define PERI_CRG_CLKDIV9                           0x0910
#define PERI_CRG_CLKDIV9_DIV_CLK_SPI0_SHIFT                                            0
#define PERI_CRG_CLKDIV9_DIV_CLK_SPI0_MASK                                             0x0000003f
#define PERI_CRG_CLKDIV9_DIV_CLK_SPI1_SHIFT                                            6
#define PERI_CRG_CLKDIV9_DIV_CLK_SPI1_MASK                                             0x00000fc0
#define PERI_CRG_CLKDIV9_SC_GT_CLK_SPI0_SHIFT                                          12
#define PERI_CRG_CLKDIV9_SC_GT_CLK_SPI0_MASK                                           0x00001000
#define PERI_CRG_CLKDIV9_SC_GT_CLK_SPI1_SHIFT                                          13
#define PERI_CRG_CLKDIV9_SC_GT_CLK_SPI1_MASK                                           0x00002000
#define PERI_CRG_CLKDIV10                          0x0914
#define PERI_CRG_CLKDIV10_DIV_CLK_SPI2_SHIFT                                           0
#define PERI_CRG_CLKDIV10_DIV_CLK_SPI2_MASK                                            0x0000003f
#define PERI_CRG_CLKDIV10_SC_GT_CLK_SPI2_SHIFT                                         12
#define PERI_CRG_CLKDIV10_SC_GT_CLK_SPI2_MASK                                          0x00001000
#define PERI_CRG_CLKDIV11                          0x0918
#define PERI_CRG_CLKDIV11_DIV_CLK_PERI_GPIODB_SHIFT                                    0
#define PERI_CRG_CLKDIV11_DIV_CLK_PERI_GPIODB_MASK                                     0x0000003f
#define PERI_CRG_CLKDIV11_SC_GT_CLK_PERI_GPIODB_SHIFT                                  12
#define PERI_CRG_CLKDIV11_SC_GT_CLK_PERI_GPIODB_MASK                                   0x00001000
#define PERI_CRG_CLKDIV13                          0x0924
#define PERI_CRG_CLKDIV13_DIV_CLK_DMA_BUS_DATA_LPPLL_SHIFT                             0
#define PERI_CRG_CLKDIV13_DIV_CLK_DMA_BUS_DATA_LPPLL_MASK                              0x0000003f
#define PERI_CRG_CLKDIV13_DIV_CLK_DMA_BUS_DATA_GPLL_SHIFT                              6
#define PERI_CRG_CLKDIV13_DIV_CLK_DMA_BUS_DATA_GPLL_MASK                               0x00000fc0
#define PERI_CRG_CLKDIV13_SC_GT_CLK_DMA_BUS_DATA_LPPLL_DIV_SHIFT                       12
#define PERI_CRG_CLKDIV13_SC_GT_CLK_DMA_BUS_DATA_LPPLL_DIV_MASK                        0x00001000
#define PERI_CRG_CLKDIV13_SC_GT_CLK_DMA_BUS_DATA_GPLL_DIV_SHIFT                        13
#define PERI_CRG_CLKDIV13_SC_GT_CLK_DMA_BUS_DATA_GPLL_DIV_MASK                         0x00002000
#define PERI_CRG_CLKDIV17                          0x0934
#define PERI_CRG_CLKDIV17_DIV_CLK_DEBUG_APB_SHIFT                                      0
#define PERI_CRG_CLKDIV17_DIV_CLK_DEBUG_APB_MASK                                       0x0000003f
#define PERI_CRG_CLKDIV17_SC_GT_CLK_DEBUG_APB_SHIFT                                    12
#define PERI_CRG_CLKDIV17_SC_GT_CLK_DEBUG_APB_MASK                                     0x00001000
#define PERI_CRG_CLKDIV18                          0x0938
#define PERI_CRG_CLKDIV18_DIV_CLK_DEBUG_ATB_H_SHIFT                                    0
#define PERI_CRG_CLKDIV18_DIV_CLK_DEBUG_ATB_H_MASK                                     0x0000003f
#define PERI_CRG_CLKDIV18_SC_GT_CLK_DEBUG_ATB_H_SHIFT                                  12
#define PERI_CRG_CLKDIV18_SC_GT_CLK_DEBUG_ATB_H_MASK                                   0x00001000
#define PERI_CRG_CLKDIV19                          0x093c
#define PERI_CRG_CLKDIV19_DIV_CLK_DEBUG_TRACE_SHIFT                                    0
#define PERI_CRG_CLKDIV19_DIV_CLK_DEBUG_TRACE_MASK                                     0x0000003f
#define PERI_CRG_CLKDIV19_DIV_CLK_XCTRL_DDR_SHIFT                                      6
#define PERI_CRG_CLKDIV19_DIV_CLK_XCTRL_DDR_MASK                                       0x00000fc0
#define PERI_CRG_CLKDIV19_SC_GT_CLK_DEBUG_TRACE_SHIFT                                  12
#define PERI_CRG_CLKDIV19_SC_GT_CLK_DEBUG_TRACE_MASK                                   0x00001000
#define PERI_CRG_CLKDIV19_SC_GT_CLK_XCTRL_DDR_SHIFT                                    13
#define PERI_CRG_CLKDIV19_SC_GT_CLK_XCTRL_DDR_MASK                                     0x00002000
#define PERI_CRG_CLKDIV20                          0x0940
#define PERI_CRG_CLKDIV20_DIV_CLK_XCTRL_CPU_SHIFT                                      0
#define PERI_CRG_CLKDIV20_DIV_CLK_XCTRL_CPU_MASK                                       0x0000003f
#define PERI_CRG_CLKDIV20_SC_GT_CLK_XCTRL_CPU_SHIFT                                    12
#define PERI_CRG_CLKDIV20_SC_GT_CLK_XCTRL_CPU_MASK                                     0x00001000
#define PERI_CRG_CLKDIV21                          0x0944
#define PERI_CRG_CLKDIV21_DIV_CLK_HSS1_GPLL_SHIFT                                      0
#define PERI_CRG_CLKDIV21_DIV_CLK_HSS1_GPLL_MASK                                       0x0000003f
#define PERI_CRG_CLKDIV21_DIV_CLK_HSS1_LPPLL_SHIFT                                     6
#define PERI_CRG_CLKDIV21_DIV_CLK_HSS1_LPPLL_MASK                                      0x00000fc0
#define PERI_CRG_CLKDIV21_SC_GT_CLK_HSS1_GPLL_DIV_SHIFT                                12
#define PERI_CRG_CLKDIV21_SC_GT_CLK_HSS1_GPLL_DIV_MASK                                 0x00001000
#define PERI_CRG_CLKDIV21_SC_GT_CLK_HSS1_LPPLL_DIV_SHIFT                               13
#define PERI_CRG_CLKDIV21_SC_GT_CLK_HSS1_LPPLL_DIV_MASK                                0x00002000
#define PERI_CRG_CLKDIV22                          0x0948
#define PERI_CRG_CLKDIV22_DIV_CLK_HSS2_GPLL_SHIFT                                      0
#define PERI_CRG_CLKDIV22_DIV_CLK_HSS2_GPLL_MASK                                       0x0000003f
#define PERI_CRG_CLKDIV22_DIV_CLK_HSS2_LPPLL_SHIFT                                     6
#define PERI_CRG_CLKDIV22_DIV_CLK_HSS2_LPPLL_MASK                                      0x00000fc0
#define PERI_CRG_CLKDIV22_SC_GT_CLK_HSS2_GPLL_DIV_SHIFT                                12
#define PERI_CRG_CLKDIV22_SC_GT_CLK_HSS2_GPLL_DIV_MASK                                 0x00001000
#define PERI_CRG_CLKDIV22_SC_GT_CLK_HSS2_LPPLL_DIV_SHIFT                               13
#define PERI_CRG_CLKDIV22_SC_GT_CLK_HSS2_LPPLL_DIV_MASK                                0x00002000
#define PERI_CRG_CLKDIV24                          0x0950
#define PERI_CRG_CLKDIV24_DIV_CLK_XRSE_GPLL_SHIFT                                      0
#define PERI_CRG_CLKDIV24_DIV_CLK_XRSE_GPLL_MASK                                       0x0000003f
#define PERI_CRG_CLKDIV24_SC_GT_CLK_XRSE_GPLL_DIV_SHIFT                                12
#define PERI_CRG_CLKDIV24_SC_GT_CLK_XRSE_GPLL_DIV_MASK                                 0x00001000
#define PERI_CRG_CLKDIV27                          0x095c
#define PERI_CRG_CLKDIV27_DIV_CLK_CPU_BUS_SHIFT                                        0
#define PERI_CRG_CLKDIV27_DIV_CLK_CPU_BUS_MASK                                         0x0000003f
#define PERI_CRG_CLKDIV27_SC_GT_CLK_CPU_BUS_SHIFT                                      12
#define PERI_CRG_CLKDIV27_SC_GT_CLK_CPU_BUS_MASK                                       0x00001000
#define PERI_CRG_CLKDIV29                          0x0970
#define PERI_CRG_CLKDIV29_SEL_CLK_MAINBUS_DATA_GPLL_SHIFT                              0
#define PERI_CRG_CLKDIV29_SEL_CLK_MAINBUS_DATA_GPLL_MASK                               0x00000003
#define PERI_CRG_CLKDIV29_SEL_CLK_MAINBUS_DATA_SHIFT                                   2
#define PERI_CRG_CLKDIV29_SEL_CLK_MAINBUS_DATA_MASK                                    0x0000000c
#define PERI_CRG_CLKDIV29_SEL_CLK_SYSBUS_DATA_GPLL_SHIFT                               4
#define PERI_CRG_CLKDIV29_SEL_CLK_SYSBUS_DATA_GPLL_MASK                                0x00000030
#define PERI_CRG_CLKDIV29_SEL_CLK_SYSBUS_DATA_SHIFT                                    6
#define PERI_CRG_CLKDIV29_SEL_CLK_SYSBUS_DATA_MASK                                     0x000000c0
#define PERI_CRG_CLKDIV29_SEL_CLK_CFGBUS_SHIFT                                         10
#define PERI_CRG_CLKDIV29_SEL_CLK_CFGBUS_MASK                                          0x00000c00
#define PERI_CRG_CLKDIV29_SEL_CLK_PERI_TIMER4_SHIFT                                    12
#define PERI_CRG_CLKDIV29_SEL_CLK_PERI_TIMER4_MASK                                     0x00001000
#define PERI_CRG_CLKDIV29_SEL_CLK_PERI_TIMER5_SHIFT                                    13
#define PERI_CRG_CLKDIV29_SEL_CLK_PERI_TIMER5_MASK                                     0x00002000
#define PERI_CRG_CLKDIV29_SEL_CLK_PERI_TIMER6_SHIFT                                    14
#define PERI_CRG_CLKDIV29_SEL_CLK_PERI_TIMER6_MASK                                     0x00004000
#define PERI_CRG_CLKDIV29_SEL_CLK_PERI_TIMER7_SHIFT                                    15
#define PERI_CRG_CLKDIV29_SEL_CLK_PERI_TIMER7_MASK                                     0x00008000
#define PERI_CRG_CLKDIV30                          0x0974
#define PERI_CRG_CLKDIV30_SEL_CLK_GPIODB_SHIFT                                         0
#define PERI_CRG_CLKDIV30_SEL_CLK_GPIODB_MASK                                          0x00000001
#define PERI_CRG_CLKDIV30_SEL_CLK_IP_SW_SHIFT                                          1
#define PERI_CRG_CLKDIV30_SEL_CLK_IP_SW_MASK                                           0x00000006
#define PERI_CRG_CLKDIV30_SEL_CLK_DMA_BUS_DATA_SHIFT                                   4
#define PERI_CRG_CLKDIV30_SEL_CLK_DMA_BUS_DATA_MASK                                    0x00000030
#define PERI_CRG_CLKDIV30_SEL_PCLK_DMA_BUS_SHIFT                                       10
#define PERI_CRG_CLKDIV30_SEL_PCLK_DMA_BUS_MASK                                        0x00000c00
#define PERI_CRG_CLKDIV31                          0x0978
#define PERI_CRG_CLKDIV31_SEL_CLK_DEBUG_APB_SHIFT                                      2
#define PERI_CRG_CLKDIV31_SEL_CLK_DEBUG_APB_MASK                                       0x0000000c
#define PERI_CRG_CLKDIV31_SEL_CLK_DEBUG_ATB_H_SHIFT                                    4
#define PERI_CRG_CLKDIV31_SEL_CLK_DEBUG_ATB_H_MASK                                     0x00000030
#define PERI_CRG_CLKDIV31_SEL_CLK_DEBUG_TRACE_SHIFT                                    6
#define PERI_CRG_CLKDIV31_SEL_CLK_DEBUG_TRACE_MASK                                     0x000000c0
#define PERI_CRG_CLKDIV31_SEL_CLK_XCTRL_DDR_SHIFT                                      8
#define PERI_CRG_CLKDIV31_SEL_CLK_XCTRL_DDR_MASK                                       0x00000300
#define PERI_CRG_CLKDIV31_SEL_CLK_XCTRL_CPU_SHIFT                                      10
#define PERI_CRG_CLKDIV31_SEL_CLK_XCTRL_CPU_MASK                                       0x00000c00
#define PERI_CRG_CLKDIV31_SEL_PCLK_CRG_CORE_SHIFT                                      12
#define PERI_CRG_CLKDIV31_SEL_PCLK_CRG_CORE_MASK                                       0x00003000
#define PERI_CRG_CLKDIV31_SEL_CLK_CPU_BUS_SHIFT                                        14
#define PERI_CRG_CLKDIV31_SEL_CLK_CPU_BUS_MASK                                         0x0000c000
#define PERI_CRG_CLKDIV32                          0x097c
#define PERI_CRG_CLKDIV32_SEL_CLK_HSS1_SHIFT                                           0
#define PERI_CRG_CLKDIV32_SEL_CLK_HSS1_MASK                                            0x00000003
#define PERI_CRG_CLKDIV32_SEL_CLK_HSS2_SHIFT                                           2
#define PERI_CRG_CLKDIV32_SEL_CLK_HSS2_MASK                                            0x0000000c
#define PERI_CRG_CLKDIV32_SEL_CLK_XRSE_SHIFT                                           4
#define PERI_CRG_CLKDIV32_SEL_CLK_XRSE_MASK                                            0x00000010
#define PERI_CRG_CLKDIV32_SEL_CLK_DDR_DFICLK_SHIFT                                     6
#define PERI_CRG_CLKDIV32_SEL_CLK_DDR_DFICLK_MASK                                      0x000000c0
#define PERI_CRG_PERICTRL0                         0x09a0
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_MAINBUS_DATA_BYPASS_SHIFT                     0
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_MAINBUS_DATA_BYPASS_MASK                      0x00000001
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_SYSBUS_DATA_BYPASS_SHIFT                      1
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_SYSBUS_DATA_BYPASS_MASK                       0x00000002
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_DMA_BUS_DATA_BYPASS_SHIFT                     2
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_DMA_BUS_DATA_BYPASS_MASK                      0x00000004
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_CSI_SYS_BYPASS_SHIFT                          3
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_CSI_SYS_BYPASS_MASK                           0x00000008
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_DSI_SYS_BYPASS_SHIFT                          4
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_DSI_SYS_BYPASS_MASK                           0x00000010
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_XCTRL_DDR_BYPASS_SHIFT                        5
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_XCTRL_DDR_BYPASS_MASK                         0x00000020
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_XCTRL_CPU_BYPASS_SHIFT                        6
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_XCTRL_CPU_BYPASS_MASK                         0x00000040
#define PERI_CRG_PERICTRL0_SEL_SCTRL_PCLK_CRG_CORE_BYPASS_SHIFT                        7
#define PERI_CRG_PERICTRL0_SEL_SCTRL_PCLK_CRG_CORE_BYPASS_MASK                         0x00000080
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_HSS1_BYPASS_SHIFT                             8
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_HSS1_BYPASS_MASK                              0x00000100
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_HSS2_BYPASS_SHIFT                             9
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_HSS2_BYPASS_MASK                              0x00000200
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_CPU_BUS_BYPASS_SHIFT                          10
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_CPU_BUS_BYPASS_MASK                           0x00000400
#define PERI_CRG_PERICTRL0_SEL_SCTRL_PCLK_DMA_BUS_BYPASS_SHIFT                         11
#define PERI_CRG_PERICTRL0_SEL_SCTRL_PCLK_DMA_BUS_BYPASS_MASK                          0x00000800
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_CFGBUS_BYPASS_SHIFT                           12
#define PERI_CRG_PERICTRL0_SEL_SCTRL_CLK_CFGBUS_BYPASS_MASK                            0x00001000
#define PERI_CRG_PERICTRL1                         0x09a4
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_MAINBUS_DATA_LPPLL_DIV_BYPASS_SHIFT        0
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_MAINBUS_DATA_LPPLL_DIV_BYPASS_MASK         0x00000001
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_SYSBUS_DATA_LPPLL_DIV_BYPASS_SHIFT         1
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_SYSBUS_DATA_LPPLL_DIV_BYPASS_MASK          0x00000002
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_CFGBUS_LPPLL_DIV_BYPASS_SHIFT              2
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_CFGBUS_LPPLL_DIV_BYPASS_MASK               0x00000004
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_DMA_BUS_DATA_LPPLL_DIV_BYPASS_SHIFT        3
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_DMA_BUS_DATA_LPPLL_DIV_BYPASS_MASK         0x00000008
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_PCLK_DMA_BUS_LPPLL_DIV_BYPASS_SHIFT            4
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_PCLK_DMA_BUS_LPPLL_DIV_BYPASS_MASK             0x00000010
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_HSS1_LPPLL_DIV_BYPASS_SHIFT                5
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_HSS1_LPPLL_DIV_BYPASS_MASK                 0x00000020
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_HSS2_LPPLL_DIV_BYPASS_SHIFT                6
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_HSS2_LPPLL_DIV_BYPASS_MASK                 0x00000040
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_GPLL_LPIS_BYPASS_SHIFT                     7
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_GPLL_LPIS_BYPASS_MASK                      0x00000080
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_GPLL_PERI_BYPASS_SHIFT                     8
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_GPLL_PERI_BYPASS_MASK                      0x00000100
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_DDR_DFICLK_BYPASS_SHIFT                    9
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_DDR_DFICLK_BYPASS_MASK                     0x00000200
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_PCLK_CRG_CORE_LPPLL_DIV_BYPASS_SHIFT           10
#define PERI_CRG_PERICTRL1_AUTOGT_SCTRL_PCLK_CRG_CORE_LPPLL_DIV_BYPASS_MASK            0x00000400
#define PERI_CRG_PERICTRL2                         0x09c0
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_SPINLOCK_BYPASS_SHIFT                      0
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_SPINLOCK_BYPASS_MASK                       0x00000001
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_TIMER_NS_BYPASS_SHIFT                      2
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_TIMER_NS_BYPASS_MASK                       0x00000004
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_TIMER_S_BYPASS_SHIFT                       3
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_TIMER_S_BYPASS_MASK                        0x00000008
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_UART6_BYPASS_SHIFT                              4
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_UART6_BYPASS_MASK                               0x00000010
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_UART3_BYPASS_SHIFT                              5
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_UART3_BYPASS_MASK                               0x00000020
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_MREGION_PERI_BYPASS_SHIFT                       6
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_MREGION_PERI_BYPASS_MASK                        0x00000040
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_WDT0_BYPASS_SHIFT                          7
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_WDT0_BYPASS_MASK                           0x00000080
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_WDT1_BYPASS_SHIFT                          8
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_WDT1_BYPASS_MASK                           0x00000100
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_WDT2_BYPASS_SHIFT                          9
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_WDT2_BYPASS_MASK                           0x00000200
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_WDT3_BYPASS_SHIFT                          10
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_WDT3_BYPASS_MASK                           0x00000400
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_IPC0_BYPASS_SHIFT                          11
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_IPC0_BYPASS_MASK                           0x00000800
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_IPC1_BYPASS_SHIFT                          12
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_IPC1_BYPASS_MASK                           0x00001000
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_IPC2_BYPASS_SHIFT                          13
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_PERI_IPC2_BYPASS_MASK                           0x00002000
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_CPU_MREGION_BYPASS_SHIFT                        14
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_CPU_MREGION_BYPASS_MASK                         0x00004000
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_GPU_MREGION_BYPASS_SHIFT                        15
#define PERI_CRG_PERICTRL2_CLKRST_FLAG_GPU_MREGION_BYPASS_MASK                         0x00008000
#define PERI_CRG_PERICTRL3                         0x09c4
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_PCTRL_BYPASS_SHIFT                              0
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_PCTRL_BYPASS_MASK                               0x00000001
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C0_BYPASS_SHIFT                               1
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C0_BYPASS_MASK                                0x00000002
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C1_BYPASS_SHIFT                               2
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C1_BYPASS_MASK                                0x00000004
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C2_BYPASS_SHIFT                               3
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C2_BYPASS_MASK                                0x00000008
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C3_BYPASS_SHIFT                               4
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C3_BYPASS_MASK                                0x00000010
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C4_BYPASS_SHIFT                               5
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C4_BYPASS_MASK                                0x00000020
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C5_BYPASS_SHIFT                               6
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C5_BYPASS_MASK                                0x00000040
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C6_BYPASS_SHIFT                               7
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C6_BYPASS_MASK                                0x00000080
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C9_BYPASS_SHIFT                               8
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C9_BYPASS_MASK                                0x00000100
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C10_BYPASS_SHIFT                              9
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C10_BYPASS_MASK                               0x00000200
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C11_BYPASS_SHIFT                              10
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C11_BYPASS_MASK                               0x00000400
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C12_BYPASS_SHIFT                              11
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C12_BYPASS_MASK                               0x00000800
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C13_BYPASS_SHIFT                              12
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C13_BYPASS_MASK                               0x00001000
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C20_BYPASS_SHIFT                              13
#define PERI_CRG_PERICTRL3_CLKRST_FLAG_I2C20_BYPASS_MASK                               0x00002000
#define PERI_CRG_PERICTRL4                         0x09c8
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI0_BYPASS_SHIFT                               0
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI0_BYPASS_MASK                                0x00000001
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI1_BYPASS_SHIFT                               1
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI1_BYPASS_MASK                                0x00000002
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI2_BYPASS_SHIFT                               2
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI2_BYPASS_MASK                                0x00000004
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI4_BYPASS_SHIFT                               3
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI4_BYPASS_MASK                                0x00000008
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI5_BYPASS_SHIFT                               4
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI5_BYPASS_MASK                                0x00000010
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI6_BYPASS_SHIFT                               5
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_SPI6_BYPASS_MASK                                0x00000020
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_I3C0_BYPASS_SHIFT                               6
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_I3C0_BYPASS_MASK                                0x00000040
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_PWM0_BYPASS_SHIFT                               7
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_PWM0_BYPASS_MASK                                0x00000080
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_PWM1_BYPASS_SHIFT                               8
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_PWM1_BYPASS_MASK                                0x00000100
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_ONEWIRE0_BYPASS_SHIFT                           9
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_ONEWIRE0_BYPASS_MASK                            0x00000200
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_ONEWIRE1_BYPASS_SHIFT                           10
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_ONEWIRE1_BYPASS_MASK                            0x00000400
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_ONEWIRE2_BYPASS_SHIFT                           11
#define PERI_CRG_PERICTRL4_CLKRST_FLAG_ONEWIRE2_BYPASS_MASK                            0x00000800
#define PERI_CRG_PERICTRL5                         0x09cc
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_PERI_GPIO_BYPASS_SHIFT                          0
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_PERI_GPIO_BYPASS_MASK                           0x00000001
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_PERI_IOCTRL_BYPASS_SHIFT                        1
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_PERI_IOCTRL_BYPASS_MASK                         0x00000002
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_PERIR_IOPADWRAP_BYPASS_SHIFT                    2
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_PERIR_IOPADWRAP_BYPASS_MASK                     0x00000004
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_PERIL_IOPADWRAP_BYPASS_SHIFT                    3
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_PERIL_IOPADWRAP_BYPASS_MASK                     0x00000008
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_DMA_S_BYPASS_SHIFT                              4
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_DMA_S_BYPASS_MASK                               0x00000010
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_DMA_NS_BYPASS_SHIFT                             5
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_DMA_NS_BYPASS_MASK                              0x00000020
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_PERF_MONITOR_BYPASS_SHIFT                       6
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_PERF_MONITOR_BYPASS_MASK                        0x00000040
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_AXI_BIST_BYPASS_SHIFT                           7
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_AXI_BIST_BYPASS_MASK                            0x00000080
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_MFW_XCTRL_DDR_BYPASS_SHIFT                      8
#define PERI_CRG_PERICTRL5_CLKRST_FLAG_MFW_XCTRL_DDR_BYPASS_MASK                       0x00000100
#define PERI_CRG_CLKDIV36                          0x09d0
#define PERI_CRG_CLKDIV36_SEL_CLK_PERI2NPU_NIC_SHIFT                                   2
#define PERI_CRG_CLKDIV36_SEL_CLK_PERI2NPU_NIC_MASK                                    0x0000000c
#define PERI_CRG_CLKDIV36_SEL_CLK_PERI2NPU_NOC_SHIFT                                   6
#define PERI_CRG_CLKDIV36_SEL_CLK_PERI2NPU_NOC_MASK                                    0x000000c0
#define PERI_CRG_CLKGT11_W1S                       0x09e0
#define PERI_CRG_CLKGT11_W1S_GT_CLK_PPLL0_LOGIC_SHIFT                                  0
#define PERI_CRG_CLKGT11_W1S_GT_CLK_PPLL1_LOGIC_SHIFT                                  1
#define PERI_CRG_CLKGT11_W1S_GT_CLK_PPLL2_LOGIC_SHIFT                                  2
#define PERI_CRG_CLKGT11_W1S_GT_CLK_PPLL3_LOGIC_SHIFT                                  3
#define PERI_CRG_CLKGT11_W1S_GT_CLK_GPLL_LOGIC_SHIFT                                   4
#define PERI_CRG_CLKGT11_W1S_GT_CLK_DDRPPLL0_LOGIC_SHIFT                               5
#define PERI_CRG_CLKGT11_W1S_GT_CLK_DDRPPLL1_LOGIC_SHIFT                               6
#define PERI_CRG_CLKGT11_W1C                       0x09e4
#define PERI_CRG_CLKGT11_W1C_GT_CLK_PPLL0_LOGIC_SHIFT                                  0
#define PERI_CRG_CLKGT11_W1C_GT_CLK_PPLL1_LOGIC_SHIFT                                  1
#define PERI_CRG_CLKGT11_W1C_GT_CLK_PPLL2_LOGIC_SHIFT                                  2
#define PERI_CRG_CLKGT11_W1C_GT_CLK_PPLL3_LOGIC_SHIFT                                  3
#define PERI_CRG_CLKGT11_W1C_GT_CLK_GPLL_LOGIC_SHIFT                                   4
#define PERI_CRG_CLKGT11_W1C_GT_CLK_DDRPPLL0_LOGIC_SHIFT                               5
#define PERI_CRG_CLKGT11_W1C_GT_CLK_DDRPPLL1_LOGIC_SHIFT                               6
#define PERI_CRG_CLKGT11_RO                        0x09e8
#define PERI_CRG_CLKGT11_RO_GT_CLK_PPLL0_LOGIC_SHIFT                                   0
#define PERI_CRG_CLKGT11_RO_GT_CLK_PPLL0_LOGIC_MASK                                    0x00000001
#define PERI_CRG_CLKGT11_RO_GT_CLK_PPLL1_LOGIC_SHIFT                                   1
#define PERI_CRG_CLKGT11_RO_GT_CLK_PPLL1_LOGIC_MASK                                    0x00000002
#define PERI_CRG_CLKGT11_RO_GT_CLK_PPLL2_LOGIC_SHIFT                                   2
#define PERI_CRG_CLKGT11_RO_GT_CLK_PPLL2_LOGIC_MASK                                    0x00000004
#define PERI_CRG_CLKGT11_RO_GT_CLK_PPLL3_LOGIC_SHIFT                                   3
#define PERI_CRG_CLKGT11_RO_GT_CLK_PPLL3_LOGIC_MASK                                    0x00000008
#define PERI_CRG_CLKGT11_RO_GT_CLK_GPLL_LOGIC_SHIFT                                    4
#define PERI_CRG_CLKGT11_RO_GT_CLK_GPLL_LOGIC_MASK                                     0x00000010
#define PERI_CRG_CLKGT11_RO_GT_CLK_DDRPPLL0_LOGIC_SHIFT                                5
#define PERI_CRG_CLKGT11_RO_GT_CLK_DDRPPLL0_LOGIC_MASK                                 0x00000020
#define PERI_CRG_CLKGT11_RO_GT_CLK_DDRPPLL1_LOGIC_SHIFT                                6
#define PERI_CRG_CLKGT11_RO_GT_CLK_DDRPPLL1_LOGIC_MASK                                 0x00000040
#define PERI_CRG_CLKST11                           0x09ec
#define PERI_CRG_CLKST11_ST_CLK_PPLL0_LOGIC_SHIFT                                      0
#define PERI_CRG_CLKST11_ST_CLK_PPLL0_LOGIC_MASK                                       0x00000001
#define PERI_CRG_CLKST11_ST_CLK_PPLL1_LOGIC_SHIFT                                      1
#define PERI_CRG_CLKST11_ST_CLK_PPLL1_LOGIC_MASK                                       0x00000002
#define PERI_CRG_CLKST11_ST_CLK_PPLL2_LOGIC_SHIFT                                      2
#define PERI_CRG_CLKST11_ST_CLK_PPLL2_LOGIC_MASK                                       0x00000004
#define PERI_CRG_CLKST11_ST_CLK_PPLL3_LOGIC_SHIFT                                      3
#define PERI_CRG_CLKST11_ST_CLK_PPLL3_LOGIC_MASK                                       0x00000008
#define PERI_CRG_CLKST11_ST_CLK_GPLL_LOGIC_SHIFT                                       4
#define PERI_CRG_CLKST11_ST_CLK_GPLL_LOGIC_MASK                                        0x00000010
#define PERI_CRG_CLKST11_ST_CLK_DDRPPLL0_LOGIC_SHIFT                                   5
#define PERI_CRG_CLKST11_ST_CLK_DDRPPLL0_LOGIC_MASK                                    0x00000020
#define PERI_CRG_CLKST11_ST_CLK_DDRPPLL1_LOGIC_SHIFT                                   6
#define PERI_CRG_CLKST11_ST_CLK_DDRPPLL1_LOGIC_MASK                                    0x00000040
#define PERI_CRG_GPLL_VOTE_CFG                     0x0a00
#define PERI_CRG_GPLL_VOTE_CFG_GPLL_START_FSM_BYPASS_SHIFT                             0
#define PERI_CRG_GPLL_VOTE_CFG_GPLL_START_FSM_BYPASS_MASK                              0x00000001
#define PERI_CRG_GPLL_VOTE_CFG_GPLL_LOCK_TIMEOUT_SHIFT                                 1
#define PERI_CRG_GPLL_VOTE_CFG_GPLL_LOCK_TIMEOUT_MASK                                  0x000003fe
#define PERI_CRG_GPLL_VOTE_CFG_GPLL_RETRY_NUM_SHIFT                                    10
#define PERI_CRG_GPLL_VOTE_CFG_GPLL_RETRY_NUM_MASK                                     0x00001c00
#define PERI_CRG_GPLL_VOTE_CFG_GPLL_FSM_EN_SHIFT                                       13
#define PERI_CRG_GPLL_VOTE_CFG_GPLL_FSM_EN_MASK                                        0x00002000
#define PERI_CRG_GPLL_VOTE_CFG_GPLL_CFG_RSV0_SHIFT                                     14
#define PERI_CRG_GPLL_VOTE_CFG_GPLL_CFG_RSV0_MASK                                      0xffffc000
#define PERI_CRG_GPLL_PRESS_TEST                   0x0a04
#define PERI_CRG_GPLL_PRESS_TEST_GPLL_PRESS_TEST_EN_SHIFT                              0
#define PERI_CRG_GPLL_PRESS_TEST_GPLL_PRESS_TEST_EN_MASK                               0x00000001
#define PERI_CRG_GPLL_PRESS_TEST_GPLL_PRESS_TEST_CNT_SHIFT                             1
#define PERI_CRG_GPLL_PRESS_TEST_GPLL_PRESS_TEST_CNT_MASK                              0x01fffffe
#define PERI_CRG_GPLL_PRESS_TEST_GPLL_CFG_RSV1_SHIFT                                   25
#define PERI_CRG_GPLL_PRESS_TEST_GPLL_CFG_RSV1_MASK                                    0xfe000000
#define PERI_CRG_GPLL_ERR_BOOT_CNT                 0x0a08
#define PERI_CRG_GPLL_ERR_BOOT_CNT_GPLL_ERR_BOOT_CNT_SHIFT                             0
#define PERI_CRG_GPLL_ERR_BOOT_CNT_GPLL_ERR_BOOT_CNT_MASK                              0x00ffffff
#define PERI_CRG_GPLL_ERR_BOOT_CNT_GPLL_PRESS_TEST_END_SHIFT                           24
#define PERI_CRG_GPLL_ERR_BOOT_CNT_GPLL_PRESS_TEST_END_MASK                            0x01000000
#define PERI_CRG_GPLL_VOTE_MASK                    0x0a0c
#define PERI_CRG_GPLL_VOTE_MASK_GPLL_HW_VOTE_MASK_SHIFT                                0
#define PERI_CRG_GPLL_VOTE_MASK_GPLL_HW_VOTE_MASK_MASK                                 0x000000ff
#define PERI_CRG_GPLL_VOTE_MASK_GPLL_SFT_VOTE_MASK_SHIFT                               8
#define PERI_CRG_GPLL_VOTE_MASK_GPLL_SFT_VOTE_MASK_MASK                                0x0000ff00
#define PERI_CRG_GPLL_VOTE_MASK_GPLL_CFG_RSV2_SHIFT                                    16
#define PERI_CRG_GPLL_VOTE_MASK_GPLL_CFG_RSV2_MASK                                     0xffff0000
#define PERI_CRG_GPLL_VOTE_BYPASS                  0x0a14
#define PERI_CRG_GPLL_VOTE_BYPASS_GPLL_VOTE_BYPASS_SHIFT                               0
#define PERI_CRG_GPLL_VOTE_BYPASS_GPLL_VOTE_BYPASS_MASK                                0x000000ff
#define PERI_CRG_GPLL_VOTE_GT                      0x0a18
#define PERI_CRG_GPLL_VOTE_GT_GPLL_VOTE_GT_SHIFT                                       0
#define PERI_CRG_GPLL_VOTE_GT_GPLL_VOTE_GT_MASK                                        0x000000ff
#define PERI_CRG_GPLL_SSMOD_CFG                    0x0a1c
#define PERI_CRG_GPLL_SSMOD_CFG_GPLL_SSMOD_EN_SHIFT                                    0
#define PERI_CRG_GPLL_SSMOD_CFG_GPLL_SSMOD_EN_MASK                                     0x00000001
#define PERI_CRG_GPLL_SSMOD_CFG_GPLL_DIVVAL_SHIFT                                      1
#define PERI_CRG_GPLL_SSMOD_CFG_GPLL_DIVVAL_MASK                                       0x0000007e
#define PERI_CRG_GPLL_SSMOD_CFG_GPLL_SPREAD_SHIFT                                      7
#define PERI_CRG_GPLL_SSMOD_CFG_GPLL_SPREAD_MASK                                       0x00000f80
#define PERI_CRG_GPLL_SSMOD_CFG_GPLL_DOWNSPREAD_SHIFT                                  12
#define PERI_CRG_GPLL_SSMOD_CFG_GPLL_DOWNSPREAD_MASK                                   0x00001000
#define PERI_CRG_GPLL_SSMOD_CFG_GPLL_CFG_RSV3_SHIFT                                    13
#define PERI_CRG_GPLL_SSMOD_CFG_GPLL_CFG_RSV3_MASK                                     0xffffe000
#define PERI_CRG_GPLL_CTRL0                        0x0a20
#define PERI_CRG_GPLL_CTRL0_GPLL_EN_SHIFT                                              0
#define PERI_CRG_GPLL_CTRL0_GPLL_EN_MASK                                               0x00000001
#define PERI_CRG_GPLL_CTRL0_GPLL_BYPASS_SHIFT                                          1
#define PERI_CRG_GPLL_CTRL0_GPLL_BYPASS_MASK                                           0x00000002
#define PERI_CRG_GPLL_CTRL0_GPLL_REFDIV_SHIFT                                          2
#define PERI_CRG_GPLL_CTRL0_GPLL_REFDIV_MASK                                           0x000000fc
#define PERI_CRG_GPLL_CTRL0_GPLL_FBDIV_SHIFT                                           8
#define PERI_CRG_GPLL_CTRL0_GPLL_FBDIV_MASK                                            0x000fff00
#define PERI_CRG_GPLL_CTRL0_GPLL_POSTDIV1_SHIFT                                        20
#define PERI_CRG_GPLL_CTRL0_GPLL_POSTDIV1_MASK                                         0x00700000
#define PERI_CRG_GPLL_CTRL0_GPLL_POSTDIV2_SHIFT                                        23
#define PERI_CRG_GPLL_CTRL0_GPLL_POSTDIV2_MASK                                         0x03800000
#define PERI_CRG_GPLL_CTRL0_GPLL_CFG_VALID_SHIFT                                       26
#define PERI_CRG_GPLL_CTRL0_GPLL_CFG_VALID_MASK                                        0x04000000
#define PERI_CRG_GPLL_CTRL0_GPLL_GT_SHIFT                                              27
#define PERI_CRG_GPLL_CTRL0_GPLL_GT_MASK                                               0x08000000
#define PERI_CRG_GPLL_CTRL0_GPLL_CFG_RSV4_SHIFT                                        28
#define PERI_CRG_GPLL_CTRL0_GPLL_CFG_RSV4_MASK                                         0xf0000000
#define PERI_CRG_GPLL_CTRL1                        0x0a24
#define PERI_CRG_GPLL_CTRL1_GPLL_FRAC_SHIFT                                            0
#define PERI_CRG_GPLL_CTRL1_GPLL_FRAC_MASK                                             0x00ffffff
#define PERI_CRG_GPLL_CTRL1_GPLL_DSM_EN_SHIFT                                          24
#define PERI_CRG_GPLL_CTRL1_GPLL_DSM_EN_MASK                                           0x01000000
#define PERI_CRG_GPLL_CTRL1_GPLL_CFG_RSV5_SHIFT                                        25
#define PERI_CRG_GPLL_CTRL1_GPLL_CFG_RSV5_MASK                                         0xfe000000
#define PERI_CRG_GPLL_DEBUG                        0x0a28
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETCALBYP_SHIFT                                    0
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETCALBYP_MASK                                     0x00000001
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETCALCNT_SHIFT                                    1
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETCALCNT_MASK                                     0x0000000e
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETCALEN_SHIFT                                     4
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETCALEN_MASK                                      0x00000010
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETCALIN_SHIFT                                     5
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETCALIN_MASK                                      0x0001ffe0
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETCALRSTN_SHIFT                                   17
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETCALRSTN_MASK                                    0x00020000
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETFASTCAL_SHIFT                                   18
#define PERI_CRG_GPLL_DEBUG_GPLL_OFFSETFASTCAL_MASK                                    0x00040000
#define PERI_CRG_GPLL_DEBUG_GPLL_CFG_RSV6_SHIFT                                        19
#define PERI_CRG_GPLL_DEBUG_GPLL_CFG_RSV6_MASK                                         0xfff80000
#define PERI_CRG_GPLL_DEBUG_STATE                  0x0a2c
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_EN_STATE_SHIFT                                  0
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_EN_STATE_MASK                                   0x00000001
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_OFFSETCALOUT_SHIFT                              1
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_OFFSETCALOUT_MASK                               0x00001ffe
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_OFFSETCALLOCK_SHIFT                             13
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_OFFSETCALLOCK_MASK                              0x00002000
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_OFFSETCALOVF_SHIFT                              14
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_OFFSETCALOVF_MASK                               0x00004000
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_START_FSM_STATE_SHIFT                           15
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_START_FSM_STATE_MASK                            0x00038000
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_REAL_RETRY_CNT_SHIFT                            18
#define PERI_CRG_GPLL_DEBUG_STATE_GPLL_REAL_RETRY_CNT_MASK                             0x001c0000
#define PERI_CRG_GPLL_DEBUG_STATE_ST_CLK_GPLL_SHIFT                                    21
#define PERI_CRG_GPLL_DEBUG_STATE_ST_CLK_GPLL_MASK                                     0x00200000
#define PERI_CRG_PPLL0_VOTE_CFG                    0x0a30
#define PERI_CRG_PPLL0_VOTE_CFG_PPLL0_START_FSM_BYPASS_SHIFT                           0
#define PERI_CRG_PPLL0_VOTE_CFG_PPLL0_START_FSM_BYPASS_MASK                            0x00000001
#define PERI_CRG_PPLL0_VOTE_CFG_PPLL0_LOCK_TIMEOUT_SHIFT                               1
#define PERI_CRG_PPLL0_VOTE_CFG_PPLL0_LOCK_TIMEOUT_MASK                                0x000003fe
#define PERI_CRG_PPLL0_VOTE_CFG_PPLL0_RETRY_NUM_SHIFT                                  10
#define PERI_CRG_PPLL0_VOTE_CFG_PPLL0_RETRY_NUM_MASK                                   0x00001c00
#define PERI_CRG_PPLL0_VOTE_CFG_PPLL0_FSM_EN_SHIFT                                     13
#define PERI_CRG_PPLL0_VOTE_CFG_PPLL0_FSM_EN_MASK                                      0x00002000
#define PERI_CRG_PPLL0_VOTE_CFG_PPLL0_CFG_RSV0_SHIFT                                   14
#define PERI_CRG_PPLL0_VOTE_CFG_PPLL0_CFG_RSV0_MASK                                    0xffffc000
#define PERI_CRG_PPLL0_PRESS_TEST                  0x0a34
#define PERI_CRG_PPLL0_PRESS_TEST_PPLL0_PRESS_TEST_EN_SHIFT                            0
#define PERI_CRG_PPLL0_PRESS_TEST_PPLL0_PRESS_TEST_EN_MASK                             0x00000001
#define PERI_CRG_PPLL0_PRESS_TEST_PPLL0_PRESS_TEST_CNT_SHIFT                           1
#define PERI_CRG_PPLL0_PRESS_TEST_PPLL0_PRESS_TEST_CNT_MASK                            0x01fffffe
#define PERI_CRG_PPLL0_PRESS_TEST_PPLL0_CFG_RSV1_SHIFT                                 25
#define PERI_CRG_PPLL0_PRESS_TEST_PPLL0_CFG_RSV1_MASK                                  0xfe000000
#define PERI_CRG_PPLL0_ERR_BOOT_CNT                0x0a38
#define PERI_CRG_PPLL0_ERR_BOOT_CNT_PPLL0_ERR_BOOT_CNT_SHIFT                           0
#define PERI_CRG_PPLL0_ERR_BOOT_CNT_PPLL0_ERR_BOOT_CNT_MASK                            0x00ffffff
#define PERI_CRG_PPLL0_ERR_BOOT_CNT_PPLL0_PRESS_TEST_END_SHIFT                         24
#define PERI_CRG_PPLL0_ERR_BOOT_CNT_PPLL0_PRESS_TEST_END_MASK                          0x01000000
#define PERI_CRG_PPLL0_VOTE_MASK                   0x0a3c
#define PERI_CRG_PPLL0_VOTE_MASK_PPLL0_HW_VOTE_MASK_SHIFT                              0
#define PERI_CRG_PPLL0_VOTE_MASK_PPLL0_HW_VOTE_MASK_MASK                               0x000000ff
#define PERI_CRG_PPLL0_VOTE_MASK_PPLL0_SFT_VOTE_MASK_SHIFT                             8
#define PERI_CRG_PPLL0_VOTE_MASK_PPLL0_SFT_VOTE_MASK_MASK                              0x0000ff00
#define PERI_CRG_PPLL0_VOTE_MASK_PPLL0_CFG_RSV2_SHIFT                                  16
#define PERI_CRG_PPLL0_VOTE_MASK_PPLL0_CFG_RSV2_MASK                                   0xffff0000
#define PERI_CRG_PPLL0_VOTE_BYPASS                 0x0a44
#define PERI_CRG_PPLL0_VOTE_BYPASS_PPLL0_VOTE_BYPASS_SHIFT                             0
#define PERI_CRG_PPLL0_VOTE_BYPASS_PPLL0_VOTE_BYPASS_MASK                              0x000000ff
#define PERI_CRG_PPLL0_VOTE_GT                     0x0a48
#define PERI_CRG_PPLL0_VOTE_GT_PPLL0_VOTE_GT_SHIFT                                     0
#define PERI_CRG_PPLL0_VOTE_GT_PPLL0_VOTE_GT_MASK                                      0x000000ff
#define PERI_CRG_PPLL0_SSMOD_CFG                   0x0a4c
#define PERI_CRG_PPLL0_SSMOD_CFG_PPLL0_SSMOD_EN_SHIFT                                  0
#define PERI_CRG_PPLL0_SSMOD_CFG_PPLL0_SSMOD_EN_MASK                                   0x00000001
#define PERI_CRG_PPLL0_SSMOD_CFG_PPLL0_DIVVAL_SHIFT                                    1
#define PERI_CRG_PPLL0_SSMOD_CFG_PPLL0_DIVVAL_MASK                                     0x0000007e
#define PERI_CRG_PPLL0_SSMOD_CFG_PPLL0_SPREAD_SHIFT                                    7
#define PERI_CRG_PPLL0_SSMOD_CFG_PPLL0_SPREAD_MASK                                     0x00000f80
#define PERI_CRG_PPLL0_SSMOD_CFG_PPLL0_DOWNSPREAD_SHIFT                                12
#define PERI_CRG_PPLL0_SSMOD_CFG_PPLL0_DOWNSPREAD_MASK                                 0x00001000
#define PERI_CRG_PPLL0_SSMOD_CFG_PPLL0_CFG_RSV3_SHIFT                                  13
#define PERI_CRG_PPLL0_SSMOD_CFG_PPLL0_CFG_RSV3_MASK                                   0xffffe000
#define PERI_CRG_PPLL0_CTRL0                       0x0a50
#define PERI_CRG_PPLL0_CTRL0_PPLL0_EN_SHIFT                                            0
#define PERI_CRG_PPLL0_CTRL0_PPLL0_EN_MASK                                             0x00000001
#define PERI_CRG_PPLL0_CTRL0_PPLL0_BYPASS_SHIFT                                        1
#define PERI_CRG_PPLL0_CTRL0_PPLL0_BYPASS_MASK                                         0x00000002
#define PERI_CRG_PPLL0_CTRL0_PPLL0_REFDIV_SHIFT                                        2
#define PERI_CRG_PPLL0_CTRL0_PPLL0_REFDIV_MASK                                         0x000000fc
#define PERI_CRG_PPLL0_CTRL0_PPLL0_FBDIV_SHIFT                                         8
#define PERI_CRG_PPLL0_CTRL0_PPLL0_FBDIV_MASK                                          0x000fff00
#define PERI_CRG_PPLL0_CTRL0_PPLL0_POSTDIV1_SHIFT                                      20
#define PERI_CRG_PPLL0_CTRL0_PPLL0_POSTDIV1_MASK                                       0x00700000
#define PERI_CRG_PPLL0_CTRL0_PPLL0_POSTDIV2_SHIFT                                      23
#define PERI_CRG_PPLL0_CTRL0_PPLL0_POSTDIV2_MASK                                       0x03800000
#define PERI_CRG_PPLL0_CTRL0_PPLL0_CFG_VALID_SHIFT                                     26
#define PERI_CRG_PPLL0_CTRL0_PPLL0_CFG_VALID_MASK                                      0x04000000
#define PERI_CRG_PPLL0_CTRL0_PPLL0_GT_SHIFT                                            27
#define PERI_CRG_PPLL0_CTRL0_PPLL0_GT_MASK                                             0x08000000
#define PERI_CRG_PPLL0_CTRL0_PPLL0_CFG_RSV4_SHIFT                                      28
#define PERI_CRG_PPLL0_CTRL0_PPLL0_CFG_RSV4_MASK                                       0xf0000000
#define PERI_CRG_PPLL0_CTRL1                       0x0a54
#define PERI_CRG_PPLL0_CTRL1_PPLL0_FRAC_SHIFT                                          0
#define PERI_CRG_PPLL0_CTRL1_PPLL0_FRAC_MASK                                           0x00ffffff
#define PERI_CRG_PPLL0_CTRL1_PPLL0_DSM_EN_SHIFT                                        24
#define PERI_CRG_PPLL0_CTRL1_PPLL0_DSM_EN_MASK                                         0x01000000
#define PERI_CRG_PPLL0_CTRL1_PPLL0_CFG_RSV5_SHIFT                                      25
#define PERI_CRG_PPLL0_CTRL1_PPLL0_CFG_RSV5_MASK                                       0xfe000000
#define PERI_CRG_PPLL0_DEBUG                       0x0a58
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETCALBYP_SHIFT                                  0
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETCALBYP_MASK                                   0x00000001
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETCALCNT_SHIFT                                  1
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETCALCNT_MASK                                   0x0000000e
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETCALEN_SHIFT                                   4
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETCALEN_MASK                                    0x00000010
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETCALIN_SHIFT                                   5
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETCALIN_MASK                                    0x0001ffe0
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETCALRSTN_SHIFT                                 17
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETCALRSTN_MASK                                  0x00020000
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETFASTCAL_SHIFT                                 18
#define PERI_CRG_PPLL0_DEBUG_PPLL0_OFFSETFASTCAL_MASK                                  0x00040000
#define PERI_CRG_PPLL0_DEBUG_PPLL0_CFG_RSV6_SHIFT                                      19
#define PERI_CRG_PPLL0_DEBUG_PPLL0_CFG_RSV6_MASK                                       0xfff80000
#define PERI_CRG_PPLL0_DEBUG_STATE                 0x0a5c
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_EN_STATE_SHIFT                                0
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_EN_STATE_MASK                                 0x00000001
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_OFFSETCALOUT_SHIFT                            1
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_OFFSETCALOUT_MASK                             0x00001ffe
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_OFFSETCALLOCK_SHIFT                           13
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_OFFSETCALLOCK_MASK                            0x00002000
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_OFFSETCALOVF_SHIFT                            14
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_OFFSETCALOVF_MASK                             0x00004000
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_START_FSM_STATE_SHIFT                         15
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_START_FSM_STATE_MASK                          0x00038000
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_REAL_RETRY_CNT_SHIFT                          18
#define PERI_CRG_PPLL0_DEBUG_STATE_PPLL0_REAL_RETRY_CNT_MASK                           0x001c0000
#define PERI_CRG_PPLL0_DEBUG_STATE_ST_CLK_PPLL0_SHIFT                                  21
#define PERI_CRG_PPLL0_DEBUG_STATE_ST_CLK_PPLL0_MASK                                   0x00200000
#define PERI_CRG_PPLL1_VOTE_CFG                    0x0a60
#define PERI_CRG_PPLL1_VOTE_CFG_PPLL1_START_FSM_BYPASS_SHIFT                           0
#define PERI_CRG_PPLL1_VOTE_CFG_PPLL1_START_FSM_BYPASS_MASK                            0x00000001
#define PERI_CRG_PPLL1_VOTE_CFG_PPLL1_LOCK_TIMEOUT_SHIFT                               1
#define PERI_CRG_PPLL1_VOTE_CFG_PPLL1_LOCK_TIMEOUT_MASK                                0x000003fe
#define PERI_CRG_PPLL1_VOTE_CFG_PPLL1_RETRY_NUM_SHIFT                                  10
#define PERI_CRG_PPLL1_VOTE_CFG_PPLL1_RETRY_NUM_MASK                                   0x00001c00
#define PERI_CRG_PPLL1_VOTE_CFG_PPLL1_FSM_EN_SHIFT                                     13
#define PERI_CRG_PPLL1_VOTE_CFG_PPLL1_FSM_EN_MASK                                      0x00002000
#define PERI_CRG_PPLL1_VOTE_CFG_PPLL1_CFG_RSV0_SHIFT                                   14
#define PERI_CRG_PPLL1_VOTE_CFG_PPLL1_CFG_RSV0_MASK                                    0xffffc000
#define PERI_CRG_PPLL1_PRESS_TEST                  0x0a64
#define PERI_CRG_PPLL1_PRESS_TEST_PPLL1_PRESS_TEST_EN_SHIFT                            0
#define PERI_CRG_PPLL1_PRESS_TEST_PPLL1_PRESS_TEST_EN_MASK                             0x00000001
#define PERI_CRG_PPLL1_PRESS_TEST_PPLL1_PRESS_TEST_CNT_SHIFT                           1
#define PERI_CRG_PPLL1_PRESS_TEST_PPLL1_PRESS_TEST_CNT_MASK                            0x01fffffe
#define PERI_CRG_PPLL1_PRESS_TEST_PPLL1_CFG_RSV1_SHIFT                                 25
#define PERI_CRG_PPLL1_PRESS_TEST_PPLL1_CFG_RSV1_MASK                                  0xfe000000
#define PERI_CRG_PPLL1_ERR_BOOT_CNT                0x0a68
#define PERI_CRG_PPLL1_ERR_BOOT_CNT_PPLL1_ERR_BOOT_CNT_SHIFT                           0
#define PERI_CRG_PPLL1_ERR_BOOT_CNT_PPLL1_ERR_BOOT_CNT_MASK                            0x00ffffff
#define PERI_CRG_PPLL1_ERR_BOOT_CNT_PPLL1_PRESS_TEST_END_SHIFT                         24
#define PERI_CRG_PPLL1_ERR_BOOT_CNT_PPLL1_PRESS_TEST_END_MASK                          0x01000000
#define PERI_CRG_PPLL1_VOTE_MASK                   0x0a6c
#define PERI_CRG_PPLL1_VOTE_MASK_PPLL1_HW_VOTE_MASK_SHIFT                              0
#define PERI_CRG_PPLL1_VOTE_MASK_PPLL1_HW_VOTE_MASK_MASK                               0x000000ff
#define PERI_CRG_PPLL1_VOTE_MASK_PPLL1_SFT_VOTE_MASK_SHIFT                             8
#define PERI_CRG_PPLL1_VOTE_MASK_PPLL1_SFT_VOTE_MASK_MASK                              0x0000ff00
#define PERI_CRG_PPLL1_VOTE_MASK_PPLL1_CFG_RSV2_SHIFT                                  16
#define PERI_CRG_PPLL1_VOTE_MASK_PPLL1_CFG_RSV2_MASK                                   0xffff0000
#define PERI_CRG_PPLL1_VOTE_BYPASS                 0x0a74
#define PERI_CRG_PPLL1_VOTE_BYPASS_PPLL1_VOTE_BYPASS_SHIFT                             0
#define PERI_CRG_PPLL1_VOTE_BYPASS_PPLL1_VOTE_BYPASS_MASK                              0x000000ff
#define PERI_CRG_PPLL1_VOTE_GT                     0x0a78
#define PERI_CRG_PPLL1_VOTE_GT_PPLL1_VOTE_GT_SHIFT                                     0
#define PERI_CRG_PPLL1_VOTE_GT_PPLL1_VOTE_GT_MASK                                      0x000000ff
#define PERI_CRG_PPLL1_SSMOD_CFG                   0x0a7c
#define PERI_CRG_PPLL1_SSMOD_CFG_PPLL1_SSMOD_EN_SHIFT                                  0
#define PERI_CRG_PPLL1_SSMOD_CFG_PPLL1_SSMOD_EN_MASK                                   0x00000001
#define PERI_CRG_PPLL1_SSMOD_CFG_PPLL1_DIVVAL_SHIFT                                    1
#define PERI_CRG_PPLL1_SSMOD_CFG_PPLL1_DIVVAL_MASK                                     0x0000007e
#define PERI_CRG_PPLL1_SSMOD_CFG_PPLL1_SPREAD_SHIFT                                    7
#define PERI_CRG_PPLL1_SSMOD_CFG_PPLL1_SPREAD_MASK                                     0x00000f80
#define PERI_CRG_PPLL1_SSMOD_CFG_PPLL1_DOWNSPREAD_SHIFT                                12
#define PERI_CRG_PPLL1_SSMOD_CFG_PPLL1_DOWNSPREAD_MASK                                 0x00001000
#define PERI_CRG_PPLL1_SSMOD_CFG_PPLL1_CFG_RSV3_SHIFT                                  13
#define PERI_CRG_PPLL1_SSMOD_CFG_PPLL1_CFG_RSV3_MASK                                   0xffffe000
#define PERI_CRG_PPLL1_CTRL0                       0x0a80
#define PERI_CRG_PPLL1_CTRL0_PPLL1_EN_SHIFT                                            0
#define PERI_CRG_PPLL1_CTRL0_PPLL1_EN_MASK                                             0x00000001
#define PERI_CRG_PPLL1_CTRL0_PPLL1_BYPASS_SHIFT                                        1
#define PERI_CRG_PPLL1_CTRL0_PPLL1_BYPASS_MASK                                         0x00000002
#define PERI_CRG_PPLL1_CTRL0_PPLL1_REFDIV_SHIFT                                        2
#define PERI_CRG_PPLL1_CTRL0_PPLL1_REFDIV_MASK                                         0x000000fc
#define PERI_CRG_PPLL1_CTRL0_PPLL1_FBDIV_SHIFT                                         8
#define PERI_CRG_PPLL1_CTRL0_PPLL1_FBDIV_MASK                                          0x000fff00
#define PERI_CRG_PPLL1_CTRL0_PPLL1_POSTDIV1_SHIFT                                      20
#define PERI_CRG_PPLL1_CTRL0_PPLL1_POSTDIV1_MASK                                       0x00700000
#define PERI_CRG_PPLL1_CTRL0_PPLL1_POSTDIV2_SHIFT                                      23
#define PERI_CRG_PPLL1_CTRL0_PPLL1_POSTDIV2_MASK                                       0x03800000
#define PERI_CRG_PPLL1_CTRL0_PPLL1_CFG_VALID_SHIFT                                     26
#define PERI_CRG_PPLL1_CTRL0_PPLL1_CFG_VALID_MASK                                      0x04000000
#define PERI_CRG_PPLL1_CTRL0_PPLL1_GT_SHIFT                                            27
#define PERI_CRG_PPLL1_CTRL0_PPLL1_GT_MASK                                             0x08000000
#define PERI_CRG_PPLL1_CTRL0_PPLL1_CFG_RSV4_SHIFT                                      28
#define PERI_CRG_PPLL1_CTRL0_PPLL1_CFG_RSV4_MASK                                       0xf0000000
#define PERI_CRG_PPLL1_CTRL1                       0x0a84
#define PERI_CRG_PPLL1_CTRL1_PPLL1_FRAC_SHIFT                                          0
#define PERI_CRG_PPLL1_CTRL1_PPLL1_FRAC_MASK                                           0x00ffffff
#define PERI_CRG_PPLL1_CTRL1_PPLL1_DSM_EN_SHIFT                                        24
#define PERI_CRG_PPLL1_CTRL1_PPLL1_DSM_EN_MASK                                         0x01000000
#define PERI_CRG_PPLL1_CTRL1_PPLL1_CFG_RSV5_SHIFT                                      25
#define PERI_CRG_PPLL1_CTRL1_PPLL1_CFG_RSV5_MASK                                       0xfe000000
#define PERI_CRG_PPLL1_DEBUG                       0x0a88
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETCALBYP_SHIFT                                  0
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETCALBYP_MASK                                   0x00000001
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETCALCNT_SHIFT                                  1
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETCALCNT_MASK                                   0x0000000e
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETCALEN_SHIFT                                   4
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETCALEN_MASK                                    0x00000010
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETCALIN_SHIFT                                   5
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETCALIN_MASK                                    0x0001ffe0
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETCALRSTN_SHIFT                                 17
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETCALRSTN_MASK                                  0x00020000
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETFASTCAL_SHIFT                                 18
#define PERI_CRG_PPLL1_DEBUG_PPLL1_OFFSETFASTCAL_MASK                                  0x00040000
#define PERI_CRG_PPLL1_DEBUG_PPLL1_CFG_RSV6_SHIFT                                      19
#define PERI_CRG_PPLL1_DEBUG_PPLL1_CFG_RSV6_MASK                                       0xfff80000
#define PERI_CRG_PPLL1_DEBUG_STATE                 0x0a8c
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_EN_STATE_SHIFT                                0
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_EN_STATE_MASK                                 0x00000001
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_OFFSETCALOUT_SHIFT                            1
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_OFFSETCALOUT_MASK                             0x00001ffe
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_OFFSETCALLOCK_SHIFT                           13
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_OFFSETCALLOCK_MASK                            0x00002000
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_OFFSETCALOVF_SHIFT                            14
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_OFFSETCALOVF_MASK                             0x00004000
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_START_FSM_STATE_SHIFT                         15
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_START_FSM_STATE_MASK                          0x00038000
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_REAL_RETRY_CNT_SHIFT                          18
#define PERI_CRG_PPLL1_DEBUG_STATE_PPLL1_REAL_RETRY_CNT_MASK                           0x001c0000
#define PERI_CRG_PPLL1_DEBUG_STATE_ST_CLK_PPLL1_SHIFT                                  21
#define PERI_CRG_PPLL1_DEBUG_STATE_ST_CLK_PPLL1_MASK                                   0x00200000
#define PERI_CRG_PPLL2_VOTE_CFG                    0x0a90
#define PERI_CRG_PPLL2_VOTE_CFG_PPLL2_START_FSM_BYPASS_SHIFT                           0
#define PERI_CRG_PPLL2_VOTE_CFG_PPLL2_START_FSM_BYPASS_MASK                            0x00000001
#define PERI_CRG_PPLL2_VOTE_CFG_PPLL2_LOCK_TIMEOUT_SHIFT                               1
#define PERI_CRG_PPLL2_VOTE_CFG_PPLL2_LOCK_TIMEOUT_MASK                                0x000003fe
#define PERI_CRG_PPLL2_VOTE_CFG_PPLL2_RETRY_NUM_SHIFT                                  10
#define PERI_CRG_PPLL2_VOTE_CFG_PPLL2_RETRY_NUM_MASK                                   0x00001c00
#define PERI_CRG_PPLL2_VOTE_CFG_PPLL2_FSM_EN_SHIFT                                     13
#define PERI_CRG_PPLL2_VOTE_CFG_PPLL2_FSM_EN_MASK                                      0x00002000
#define PERI_CRG_PPLL2_VOTE_CFG_PPLL2_CFG_RSV0_SHIFT                                   14
#define PERI_CRG_PPLL2_VOTE_CFG_PPLL2_CFG_RSV0_MASK                                    0xffffc000
#define PERI_CRG_PPLL2_PRESS_TEST                  0x0a94
#define PERI_CRG_PPLL2_PRESS_TEST_PPLL2_PRESS_TEST_EN_SHIFT                            0
#define PERI_CRG_PPLL2_PRESS_TEST_PPLL2_PRESS_TEST_EN_MASK                             0x00000001
#define PERI_CRG_PPLL2_PRESS_TEST_PPLL2_PRESS_TEST_CNT_SHIFT                           1
#define PERI_CRG_PPLL2_PRESS_TEST_PPLL2_PRESS_TEST_CNT_MASK                            0x01fffffe
#define PERI_CRG_PPLL2_PRESS_TEST_PPLL2_CFG_RSV1_SHIFT                                 25
#define PERI_CRG_PPLL2_PRESS_TEST_PPLL2_CFG_RSV1_MASK                                  0xfe000000
#define PERI_CRG_PPLL2_ERR_BOOT_CNT                0x0a98
#define PERI_CRG_PPLL2_ERR_BOOT_CNT_PPLL2_ERR_BOOT_CNT_SHIFT                           0
#define PERI_CRG_PPLL2_ERR_BOOT_CNT_PPLL2_ERR_BOOT_CNT_MASK                            0x00ffffff
#define PERI_CRG_PPLL2_ERR_BOOT_CNT_PPLL2_PRESS_TEST_END_SHIFT                         24
#define PERI_CRG_PPLL2_ERR_BOOT_CNT_PPLL2_PRESS_TEST_END_MASK                          0x01000000
#define PERI_CRG_PPLL2_VOTE_MASK                   0x0a9c
#define PERI_CRG_PPLL2_VOTE_MASK_PPLL2_HW_VOTE_MASK_SHIFT                              0
#define PERI_CRG_PPLL2_VOTE_MASK_PPLL2_HW_VOTE_MASK_MASK                               0x000000ff
#define PERI_CRG_PPLL2_VOTE_MASK_PPLL2_SFT_VOTE_MASK_SHIFT                             8
#define PERI_CRG_PPLL2_VOTE_MASK_PPLL2_SFT_VOTE_MASK_MASK                              0x0000ff00
#define PERI_CRG_PPLL2_VOTE_MASK_PPLL2_CFG_RSV2_SHIFT                                  16
#define PERI_CRG_PPLL2_VOTE_MASK_PPLL2_CFG_RSV2_MASK                                   0xffff0000
#define PERI_CRG_PPLL2_VOTE_BYPASS                 0x0aa4
#define PERI_CRG_PPLL2_VOTE_BYPASS_PPLL2_VOTE_BYPASS_SHIFT                             0
#define PERI_CRG_PPLL2_VOTE_BYPASS_PPLL2_VOTE_BYPASS_MASK                              0x000000ff
#define PERI_CRG_PPLL2_VOTE_GT                     0x0aa8
#define PERI_CRG_PPLL2_VOTE_GT_PPLL2_VOTE_GT_SHIFT                                     0
#define PERI_CRG_PPLL2_VOTE_GT_PPLL2_VOTE_GT_MASK                                      0x000000ff
#define PERI_CRG_PPLL2_SSMOD_CFG                   0x0aac
#define PERI_CRG_PPLL2_SSMOD_CFG_PPLL2_SSMOD_EN_SHIFT                                  0
#define PERI_CRG_PPLL2_SSMOD_CFG_PPLL2_SSMOD_EN_MASK                                   0x00000001
#define PERI_CRG_PPLL2_SSMOD_CFG_PPLL2_DIVVAL_SHIFT                                    1
#define PERI_CRG_PPLL2_SSMOD_CFG_PPLL2_DIVVAL_MASK                                     0x0000007e
#define PERI_CRG_PPLL2_SSMOD_CFG_PPLL2_SPREAD_SHIFT                                    7
#define PERI_CRG_PPLL2_SSMOD_CFG_PPLL2_SPREAD_MASK                                     0x00000f80
#define PERI_CRG_PPLL2_SSMOD_CFG_PPLL2_DOWNSPREAD_SHIFT                                12
#define PERI_CRG_PPLL2_SSMOD_CFG_PPLL2_DOWNSPREAD_MASK                                 0x00001000
#define PERI_CRG_PPLL2_SSMOD_CFG_PPLL2_CFG_RSV3_SHIFT                                  13
#define PERI_CRG_PPLL2_SSMOD_CFG_PPLL2_CFG_RSV3_MASK                                   0xffffe000
#define PERI_CRG_PPLL2_CTRL0                       0x0ab0
#define PERI_CRG_PPLL2_CTRL0_PPLL2_EN_SHIFT                                            0
#define PERI_CRG_PPLL2_CTRL0_PPLL2_EN_MASK                                             0x00000001
#define PERI_CRG_PPLL2_CTRL0_PPLL2_BYPASS_SHIFT                                        1
#define PERI_CRG_PPLL2_CTRL0_PPLL2_BYPASS_MASK                                         0x00000002
#define PERI_CRG_PPLL2_CTRL0_PPLL2_REFDIV_SHIFT                                        2
#define PERI_CRG_PPLL2_CTRL0_PPLL2_REFDIV_MASK                                         0x000000fc
#define PERI_CRG_PPLL2_CTRL0_PPLL2_FBDIV_SHIFT                                         8
#define PERI_CRG_PPLL2_CTRL0_PPLL2_FBDIV_MASK                                          0x000fff00
#define PERI_CRG_PPLL2_CTRL0_PPLL2_POSTDIV1_SHIFT                                      20
#define PERI_CRG_PPLL2_CTRL0_PPLL2_POSTDIV1_MASK                                       0x00700000
#define PERI_CRG_PPLL2_CTRL0_PPLL2_POSTDIV2_SHIFT                                      23
#define PERI_CRG_PPLL2_CTRL0_PPLL2_POSTDIV2_MASK                                       0x03800000
#define PERI_CRG_PPLL2_CTRL0_PPLL2_CFG_VALID_SHIFT                                     26
#define PERI_CRG_PPLL2_CTRL0_PPLL2_CFG_VALID_MASK                                      0x04000000
#define PERI_CRG_PPLL2_CTRL0_PPLL2_GT_SHIFT                                            27
#define PERI_CRG_PPLL2_CTRL0_PPLL2_GT_MASK                                             0x08000000
#define PERI_CRG_PPLL2_CTRL0_PPLL2_CFG_RSV4_SHIFT                                      28
#define PERI_CRG_PPLL2_CTRL0_PPLL2_CFG_RSV4_MASK                                       0xf0000000
#define PERI_CRG_PPLL2_CTRL1                       0x0ab4
#define PERI_CRG_PPLL2_CTRL1_PPLL2_FRAC_SHIFT                                          0
#define PERI_CRG_PPLL2_CTRL1_PPLL2_FRAC_MASK                                           0x00ffffff
#define PERI_CRG_PPLL2_CTRL1_PPLL2_DSM_EN_SHIFT                                        24
#define PERI_CRG_PPLL2_CTRL1_PPLL2_DSM_EN_MASK                                         0x01000000
#define PERI_CRG_PPLL2_CTRL1_PPLL2_CFG_RSV5_SHIFT                                      25
#define PERI_CRG_PPLL2_CTRL1_PPLL2_CFG_RSV5_MASK                                       0xfe000000
#define PERI_CRG_PPLL2_DEBUG                       0x0ab8
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETCALBYP_SHIFT                                  0
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETCALBYP_MASK                                   0x00000001
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETCALCNT_SHIFT                                  1
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETCALCNT_MASK                                   0x0000000e
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETCALEN_SHIFT                                   4
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETCALEN_MASK                                    0x00000010
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETCALIN_SHIFT                                   5
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETCALIN_MASK                                    0x0001ffe0
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETCALRSTN_SHIFT                                 17
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETCALRSTN_MASK                                  0x00020000
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETFASTCAL_SHIFT                                 18
#define PERI_CRG_PPLL2_DEBUG_PPLL2_OFFSETFASTCAL_MASK                                  0x00040000
#define PERI_CRG_PPLL2_DEBUG_PPLL2_CFG_RSV6_SHIFT                                      19
#define PERI_CRG_PPLL2_DEBUG_PPLL2_CFG_RSV6_MASK                                       0xfff80000
#define PERI_CRG_PPLL2_DEBUG_STATE                 0x0abc
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_EN_STATE_SHIFT                                0
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_EN_STATE_MASK                                 0x00000001
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_OFFSETCALOUT_SHIFT                            1
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_OFFSETCALOUT_MASK                             0x00001ffe
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_OFFSETCALLOCK_SHIFT                           13
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_OFFSETCALLOCK_MASK                            0x00002000
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_OFFSETCALOVF_SHIFT                            14
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_OFFSETCALOVF_MASK                             0x00004000
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_START_FSM_STATE_SHIFT                         15
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_START_FSM_STATE_MASK                          0x00038000
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_REAL_RETRY_CNT_SHIFT                          18
#define PERI_CRG_PPLL2_DEBUG_STATE_PPLL2_REAL_RETRY_CNT_MASK                           0x001c0000
#define PERI_CRG_PPLL2_DEBUG_STATE_ST_CLK_PPLL2_SHIFT                                  21
#define PERI_CRG_PPLL2_DEBUG_STATE_ST_CLK_PPLL2_MASK                                   0x00200000
#define PERI_CRG_PPLL3_VOTE_CFG                    0x0ad0
#define PERI_CRG_PPLL3_VOTE_CFG_PPLL3_START_FSM_BYPASS_SHIFT                           0
#define PERI_CRG_PPLL3_VOTE_CFG_PPLL3_START_FSM_BYPASS_MASK                            0x00000001
#define PERI_CRG_PPLL3_VOTE_CFG_PPLL3_LOCK_TIMEOUT_SHIFT                               1
#define PERI_CRG_PPLL3_VOTE_CFG_PPLL3_LOCK_TIMEOUT_MASK                                0x000003fe
#define PERI_CRG_PPLL3_VOTE_CFG_PPLL3_RETRY_NUM_SHIFT                                  10
#define PERI_CRG_PPLL3_VOTE_CFG_PPLL3_RETRY_NUM_MASK                                   0x00001c00
#define PERI_CRG_PPLL3_VOTE_CFG_PPLL3_FSM_EN_SHIFT                                     13
#define PERI_CRG_PPLL3_VOTE_CFG_PPLL3_FSM_EN_MASK                                      0x00002000
#define PERI_CRG_PPLL3_VOTE_CFG_PPLL3_CFG_RSV0_SHIFT                                   14
#define PERI_CRG_PPLL3_VOTE_CFG_PPLL3_CFG_RSV0_MASK                                    0xffffc000
#define PERI_CRG_PPLL3_PRESS_TEST                  0x0ad4
#define PERI_CRG_PPLL3_PRESS_TEST_PPLL3_PRESS_TEST_EN_SHIFT                            0
#define PERI_CRG_PPLL3_PRESS_TEST_PPLL3_PRESS_TEST_EN_MASK                             0x00000001
#define PERI_CRG_PPLL3_PRESS_TEST_PPLL3_PRESS_TEST_CNT_SHIFT                           1
#define PERI_CRG_PPLL3_PRESS_TEST_PPLL3_PRESS_TEST_CNT_MASK                            0x01fffffe
#define PERI_CRG_PPLL3_PRESS_TEST_PPLL3_CFG_RSV1_SHIFT                                 25
#define PERI_CRG_PPLL3_PRESS_TEST_PPLL3_CFG_RSV1_MASK                                  0xfe000000
#define PERI_CRG_PPLL3_ERR_BOOT_CNT                0x0ad8
#define PERI_CRG_PPLL3_ERR_BOOT_CNT_PPLL3_ERR_BOOT_CNT_SHIFT                           0
#define PERI_CRG_PPLL3_ERR_BOOT_CNT_PPLL3_ERR_BOOT_CNT_MASK                            0x00ffffff
#define PERI_CRG_PPLL3_ERR_BOOT_CNT_PPLL3_PRESS_TEST_END_SHIFT                         24
#define PERI_CRG_PPLL3_ERR_BOOT_CNT_PPLL3_PRESS_TEST_END_MASK                          0x01000000
#define PERI_CRG_PPLL3_VOTE_MASK                   0x0adc
#define PERI_CRG_PPLL3_VOTE_MASK_PPLL3_HW_VOTE_MASK_SHIFT                              0
#define PERI_CRG_PPLL3_VOTE_MASK_PPLL3_HW_VOTE_MASK_MASK                               0x000000ff
#define PERI_CRG_PPLL3_VOTE_MASK_PPLL3_SFT_VOTE_MASK_SHIFT                             8
#define PERI_CRG_PPLL3_VOTE_MASK_PPLL3_SFT_VOTE_MASK_MASK                              0x0000ff00
#define PERI_CRG_PPLL3_VOTE_MASK_PPLL3_CFG_RSV2_SHIFT                                  16
#define PERI_CRG_PPLL3_VOTE_MASK_PPLL3_CFG_RSV2_MASK                                   0xffff0000
#define PERI_CRG_PPLL3_VOTE_BYPASS                 0x0ae4
#define PERI_CRG_PPLL3_VOTE_BYPASS_PPLL3_VOTE_BYPASS_SHIFT                             0
#define PERI_CRG_PPLL3_VOTE_BYPASS_PPLL3_VOTE_BYPASS_MASK                              0x000000ff
#define PERI_CRG_PPLL3_VOTE_GT                     0x0ae8
#define PERI_CRG_PPLL3_VOTE_GT_PPLL3_VOTE_GT_SHIFT                                     0
#define PERI_CRG_PPLL3_VOTE_GT_PPLL3_VOTE_GT_MASK                                      0x000000ff
#define PERI_CRG_PPLL3_SSMOD_CFG                   0x0aec
#define PERI_CRG_PPLL3_SSMOD_CFG_PPLL3_SSMOD_EN_SHIFT                                  0
#define PERI_CRG_PPLL3_SSMOD_CFG_PPLL3_SSMOD_EN_MASK                                   0x00000001
#define PERI_CRG_PPLL3_SSMOD_CFG_PPLL3_DIVVAL_SHIFT                                    1
#define PERI_CRG_PPLL3_SSMOD_CFG_PPLL3_DIVVAL_MASK                                     0x0000007e
#define PERI_CRG_PPLL3_SSMOD_CFG_PPLL3_SPREAD_SHIFT                                    7
#define PERI_CRG_PPLL3_SSMOD_CFG_PPLL3_SPREAD_MASK                                     0x00000f80
#define PERI_CRG_PPLL3_SSMOD_CFG_PPLL3_DOWNSPREAD_SHIFT                                12
#define PERI_CRG_PPLL3_SSMOD_CFG_PPLL3_DOWNSPREAD_MASK                                 0x00001000
#define PERI_CRG_PPLL3_SSMOD_CFG_PPLL3_CFG_RSV3_SHIFT                                  13
#define PERI_CRG_PPLL3_SSMOD_CFG_PPLL3_CFG_RSV3_MASK                                   0xffffe000
#define PERI_CRG_PPLL3_CTRL0                       0x0af0
#define PERI_CRG_PPLL3_CTRL0_PPLL3_EN_SHIFT                                            0
#define PERI_CRG_PPLL3_CTRL0_PPLL3_EN_MASK                                             0x00000001
#define PERI_CRG_PPLL3_CTRL0_PPLL3_BYPASS_SHIFT                                        1
#define PERI_CRG_PPLL3_CTRL0_PPLL3_BYPASS_MASK                                         0x00000002
#define PERI_CRG_PPLL3_CTRL0_PPLL3_REFDIV_SHIFT                                        2
#define PERI_CRG_PPLL3_CTRL0_PPLL3_REFDIV_MASK                                         0x000000fc
#define PERI_CRG_PPLL3_CTRL0_PPLL3_FBDIV_SHIFT                                         8
#define PERI_CRG_PPLL3_CTRL0_PPLL3_FBDIV_MASK                                          0x000fff00
#define PERI_CRG_PPLL3_CTRL0_PPLL3_POSTDIV1_SHIFT                                      20
#define PERI_CRG_PPLL3_CTRL0_PPLL3_POSTDIV1_MASK                                       0x00700000
#define PERI_CRG_PPLL3_CTRL0_PPLL3_POSTDIV2_SHIFT                                      23
#define PERI_CRG_PPLL3_CTRL0_PPLL3_POSTDIV2_MASK                                       0x03800000
#define PERI_CRG_PPLL3_CTRL0_PPLL3_CFG_VALID_SHIFT                                     26
#define PERI_CRG_PPLL3_CTRL0_PPLL3_CFG_VALID_MASK                                      0x04000000
#define PERI_CRG_PPLL3_CTRL0_PPLL3_GT_SHIFT                                            27
#define PERI_CRG_PPLL3_CTRL0_PPLL3_GT_MASK                                             0x08000000
#define PERI_CRG_PPLL3_CTRL0_PPLL3_CFG_RSV4_SHIFT                                      28
#define PERI_CRG_PPLL3_CTRL0_PPLL3_CFG_RSV4_MASK                                       0xf0000000
#define PERI_CRG_PPLL3_CTRL1                       0x0af4
#define PERI_CRG_PPLL3_CTRL1_PPLL3_FRAC_SHIFT                                          0
#define PERI_CRG_PPLL3_CTRL1_PPLL3_FRAC_MASK                                           0x00ffffff
#define PERI_CRG_PPLL3_CTRL1_PPLL3_DSM_EN_SHIFT                                        24
#define PERI_CRG_PPLL3_CTRL1_PPLL3_DSM_EN_MASK                                         0x01000000
#define PERI_CRG_PPLL3_CTRL1_PPLL3_CFG_RSV5_SHIFT                                      25
#define PERI_CRG_PPLL3_CTRL1_PPLL3_CFG_RSV5_MASK                                       0xfe000000
#define PERI_CRG_PPLL3_DEBUG                       0x0af8
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETCALBYP_SHIFT                                  0
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETCALBYP_MASK                                   0x00000001
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETCALCNT_SHIFT                                  1
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETCALCNT_MASK                                   0x0000000e
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETCALEN_SHIFT                                   4
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETCALEN_MASK                                    0x00000010
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETCALIN_SHIFT                                   5
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETCALIN_MASK                                    0x0001ffe0
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETCALRSTN_SHIFT                                 17
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETCALRSTN_MASK                                  0x00020000
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETFASTCAL_SHIFT                                 18
#define PERI_CRG_PPLL3_DEBUG_PPLL3_OFFSETFASTCAL_MASK                                  0x00040000
#define PERI_CRG_PPLL3_DEBUG_PPLL3_CFG_RSV6_SHIFT                                      19
#define PERI_CRG_PPLL3_DEBUG_PPLL3_CFG_RSV6_MASK                                       0xfff80000
#define PERI_CRG_PPLL3_DEBUG_STATE                 0x0afc
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_EN_STATE_SHIFT                                0
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_EN_STATE_MASK                                 0x00000001
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_OFFSETCALOUT_SHIFT                            1
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_OFFSETCALOUT_MASK                             0x00001ffe
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_OFFSETCALLOCK_SHIFT                           13
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_OFFSETCALLOCK_MASK                            0x00002000
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_OFFSETCALOVF_SHIFT                            14
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_OFFSETCALOVF_MASK                             0x00004000
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_START_FSM_STATE_SHIFT                         15
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_START_FSM_STATE_MASK                          0x00038000
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_REAL_RETRY_CNT_SHIFT                          18
#define PERI_CRG_PPLL3_DEBUG_STATE_PPLL3_REAL_RETRY_CNT_MASK                           0x001c0000
#define PERI_CRG_PPLL3_DEBUG_STATE_ST_CLK_PPLL3_SHIFT                                  21
#define PERI_CRG_PPLL3_DEBUG_STATE_ST_CLK_PPLL3_MASK                                   0x00200000
#define PERI_CRG_DDR_PPLL0_VOTE_CFG                0x0b00
#define PERI_CRG_DDR_PPLL0_VOTE_CFG_DDR_PPLL0_START_FSM_BYPASS_SHIFT                   0
#define PERI_CRG_DDR_PPLL0_VOTE_CFG_DDR_PPLL0_START_FSM_BYPASS_MASK                    0x00000001
#define PERI_CRG_DDR_PPLL0_VOTE_CFG_DDR_PPLL0_LOCK_TIMEOUT_SHIFT                       1
#define PERI_CRG_DDR_PPLL0_VOTE_CFG_DDR_PPLL0_LOCK_TIMEOUT_MASK                        0x000003fe
#define PERI_CRG_DDR_PPLL0_VOTE_CFG_DDR_PPLL0_RETRY_NUM_SHIFT                          10
#define PERI_CRG_DDR_PPLL0_VOTE_CFG_DDR_PPLL0_RETRY_NUM_MASK                           0x00001c00
#define PERI_CRG_DDR_PPLL0_VOTE_CFG_DDR_PPLL0_FSM_EN_SHIFT                             13
#define PERI_CRG_DDR_PPLL0_VOTE_CFG_DDR_PPLL0_FSM_EN_MASK                              0x00002000
#define PERI_CRG_DDR_PPLL0_VOTE_CFG_DDR_PPLL0_CFG_RSV0_SHIFT                           14
#define PERI_CRG_DDR_PPLL0_VOTE_CFG_DDR_PPLL0_CFG_RSV0_MASK                            0xffffc000
#define PERI_CRG_DDR_PPLL0_PRESS_TEST              0x0b04
#define PERI_CRG_DDR_PPLL0_PRESS_TEST_DDR_PPLL0_PRESS_TEST_EN_SHIFT                    0
#define PERI_CRG_DDR_PPLL0_PRESS_TEST_DDR_PPLL0_PRESS_TEST_EN_MASK                     0x00000001
#define PERI_CRG_DDR_PPLL0_PRESS_TEST_DDR_PPLL0_PRESS_TEST_CNT_SHIFT                   1
#define PERI_CRG_DDR_PPLL0_PRESS_TEST_DDR_PPLL0_PRESS_TEST_CNT_MASK                    0x01fffffe
#define PERI_CRG_DDR_PPLL0_PRESS_TEST_DDR_PPLL0_CFG_RSV1_SHIFT                         25
#define PERI_CRG_DDR_PPLL0_PRESS_TEST_DDR_PPLL0_CFG_RSV1_MASK                          0xfe000000
#define PERI_CRG_DDR_PPLL0_ERR_BOOT_CNT            0x0b08
#define PERI_CRG_DDR_PPLL0_ERR_BOOT_CNT_DDR_PPLL0_ERR_BOOT_CNT_SHIFT                   0
#define PERI_CRG_DDR_PPLL0_ERR_BOOT_CNT_DDR_PPLL0_ERR_BOOT_CNT_MASK                    0x00ffffff
#define PERI_CRG_DDR_PPLL0_ERR_BOOT_CNT_DDR_PPLL0_PRESS_TEST_END_SHIFT                 24
#define PERI_CRG_DDR_PPLL0_ERR_BOOT_CNT_DDR_PPLL0_PRESS_TEST_END_MASK                  0x01000000
#define PERI_CRG_DDR_PPLL0_VOTE_MASK               0x0b0c
#define PERI_CRG_DDR_PPLL0_VOTE_MASK_DDR_PPLL0_HW_VOTE_MASK_SHIFT                      0
#define PERI_CRG_DDR_PPLL0_VOTE_MASK_DDR_PPLL0_HW_VOTE_MASK_MASK                       0x000000ff
#define PERI_CRG_DDR_PPLL0_VOTE_MASK_DDR_PPLL0_SFT_VOTE_MASK_SHIFT                     8
#define PERI_CRG_DDR_PPLL0_VOTE_MASK_DDR_PPLL0_SFT_VOTE_MASK_MASK                      0x0000ff00
#define PERI_CRG_DDR_PPLL0_VOTE_MASK_DDR_PPLL0_CFG_RSV2_SHIFT                          16
#define PERI_CRG_DDR_PPLL0_VOTE_MASK_DDR_PPLL0_CFG_RSV2_MASK                           0xffff0000
#define PERI_CRG_DDR_PPLL0_VOTE_BYPASS             0x0b14
#define PERI_CRG_DDR_PPLL0_VOTE_BYPASS_DDR_PPLL0_VOTE_BYPASS_SHIFT                     0
#define PERI_CRG_DDR_PPLL0_VOTE_BYPASS_DDR_PPLL0_VOTE_BYPASS_MASK                      0x000000ff
#define PERI_CRG_DDR_PPLL0_VOTE_GT                 0x0b18
#define PERI_CRG_DDR_PPLL0_VOTE_GT_DDR_PPLL0_VOTE_GT_SHIFT                             0
#define PERI_CRG_DDR_PPLL0_VOTE_GT_DDR_PPLL0_VOTE_GT_MASK                              0x000000ff
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG               0x0b1c
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG_DDR_PPLL0_SSMOD_EN_SHIFT                          0
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG_DDR_PPLL0_SSMOD_EN_MASK                           0x00000001
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG_DDR_PPLL0_DIVVAL_SHIFT                            1
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG_DDR_PPLL0_DIVVAL_MASK                             0x0000007e
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG_DDR_PPLL0_SPREAD_SHIFT                            7
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG_DDR_PPLL0_SPREAD_MASK                             0x00000f80
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG_DDR_PPLL0_DOWNSPREAD_SHIFT                        12
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG_DDR_PPLL0_DOWNSPREAD_MASK                         0x00001000
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG_DDR_PPLL0_CFG_RSV3_SHIFT                          13
#define PERI_CRG_DDR_PPLL0_SSMOD_CFG_DDR_PPLL0_CFG_RSV3_MASK                           0xffffe000
#define PERI_CRG_DDR_PPLL0_CTRL0                   0x0b20
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_EN_SHIFT                                    0
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_EN_MASK                                     0x00000001
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_BYPASS_SHIFT                                1
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_BYPASS_MASK                                 0x00000002
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_REFDIV_SHIFT                                2
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_REFDIV_MASK                                 0x000000fc
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_FBDIV_SHIFT                                 8
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_FBDIV_MASK                                  0x000fff00
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_POSTDIV1_SHIFT                              20
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_POSTDIV1_MASK                               0x00700000
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_POSTDIV2_SHIFT                              23
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_POSTDIV2_MASK                               0x03800000
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_CFG_VALID_SHIFT                             26
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_CFG_VALID_MASK                              0x04000000
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_GT_SHIFT                                    27
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_GT_MASK                                     0x08000000
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_CFG_RSV4_SHIFT                              28
#define PERI_CRG_DDR_PPLL0_CTRL0_DDR_PPLL0_CFG_RSV4_MASK                               0xf0000000
#define PERI_CRG_DDR_PPLL0_CTRL1                   0x0b24
#define PERI_CRG_DDR_PPLL0_CTRL1_DDR_PPLL0_FRAC_SHIFT                                  0
#define PERI_CRG_DDR_PPLL0_CTRL1_DDR_PPLL0_FRAC_MASK                                   0x00ffffff
#define PERI_CRG_DDR_PPLL0_CTRL1_DDR_PPLL0_DSM_EN_SHIFT                                24
#define PERI_CRG_DDR_PPLL0_CTRL1_DDR_PPLL0_DSM_EN_MASK                                 0x01000000
#define PERI_CRG_DDR_PPLL0_CTRL1_DDR_PPLL0_CFG_RSV5_SHIFT                              25
#define PERI_CRG_DDR_PPLL0_CTRL1_DDR_PPLL0_CFG_RSV5_MASK                               0xfe000000
#define PERI_CRG_DDR_PPLL0_DEBUG                   0x0b28
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETCALBYP_SHIFT                          0
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETCALBYP_MASK                           0x00000001
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETCALCNT_SHIFT                          1
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETCALCNT_MASK                           0x0000000e
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETCALEN_SHIFT                           4
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETCALEN_MASK                            0x00000010
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETCALIN_SHIFT                           5
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETCALIN_MASK                            0x0001ffe0
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETCALRSTN_SHIFT                         17
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETCALRSTN_MASK                          0x00020000
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETFASTCAL_SHIFT                         18
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_OFFSETFASTCAL_MASK                          0x00040000
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_CFG_RSV6_SHIFT                              19
#define PERI_CRG_DDR_PPLL0_DEBUG_DDR_PPLL0_CFG_RSV6_MASK                               0xfff80000
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE             0x0b2c
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_EN_STATE_SHIFT                        0
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_EN_STATE_MASK                         0x00000001
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_OFFSETCALOUT_SHIFT                    1
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_OFFSETCALOUT_MASK                     0x00001ffe
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_OFFSETCALLOCK_SHIFT                   13
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_OFFSETCALLOCK_MASK                    0x00002000
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_OFFSETCALOVF_SHIFT                    14
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_OFFSETCALOVF_MASK                     0x00004000
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_START_FSM_STATE_SHIFT                 15
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_START_FSM_STATE_MASK                  0x00038000
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_REAL_RETRY_CNT_SHIFT                  18
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_DDR_PPLL0_REAL_RETRY_CNT_MASK                   0x001c0000
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_ST_CLK_DDR_PPLL0_SHIFT                          21
#define PERI_CRG_DDR_PPLL0_DEBUG_STATE_ST_CLK_DDR_PPLL0_MASK                           0x00200000
#define PERI_CRG_DDR_PPLL1_VOTE_CFG                0x0b30
#define PERI_CRG_DDR_PPLL1_VOTE_CFG_DDR_PPLL1_START_FSM_BYPASS_SHIFT                   0
#define PERI_CRG_DDR_PPLL1_VOTE_CFG_DDR_PPLL1_START_FSM_BYPASS_MASK                    0x00000001
#define PERI_CRG_DDR_PPLL1_VOTE_CFG_DDR_PPLL1_LOCK_TIMEOUT_SHIFT                       1
#define PERI_CRG_DDR_PPLL1_VOTE_CFG_DDR_PPLL1_LOCK_TIMEOUT_MASK                        0x000003fe
#define PERI_CRG_DDR_PPLL1_VOTE_CFG_DDR_PPLL1_RETRY_NUM_SHIFT                          10
#define PERI_CRG_DDR_PPLL1_VOTE_CFG_DDR_PPLL1_RETRY_NUM_MASK                           0x00001c00
#define PERI_CRG_DDR_PPLL1_VOTE_CFG_DDR_PPLL1_FSM_EN_SHIFT                             13
#define PERI_CRG_DDR_PPLL1_VOTE_CFG_DDR_PPLL1_FSM_EN_MASK                              0x00002000
#define PERI_CRG_DDR_PPLL1_VOTE_CFG_DDR_PPLL1_CFG_RSV0_SHIFT                           14
#define PERI_CRG_DDR_PPLL1_VOTE_CFG_DDR_PPLL1_CFG_RSV0_MASK                            0xffffc000
#define PERI_CRG_DDR_PPLL1_PRESS_TEST              0x0b34
#define PERI_CRG_DDR_PPLL1_PRESS_TEST_DDR_PPLL1_PRESS_TEST_EN_SHIFT                    0
#define PERI_CRG_DDR_PPLL1_PRESS_TEST_DDR_PPLL1_PRESS_TEST_EN_MASK                     0x00000001
#define PERI_CRG_DDR_PPLL1_PRESS_TEST_DDR_PPLL1_PRESS_TEST_CNT_SHIFT                   1
#define PERI_CRG_DDR_PPLL1_PRESS_TEST_DDR_PPLL1_PRESS_TEST_CNT_MASK                    0x01fffffe
#define PERI_CRG_DDR_PPLL1_PRESS_TEST_DDR_PPLL1_CFG_RSV1_SHIFT                         25
#define PERI_CRG_DDR_PPLL1_PRESS_TEST_DDR_PPLL1_CFG_RSV1_MASK                          0xfe000000
#define PERI_CRG_DDR_PPLL1_ERR_BOOT_CNT            0x0b38
#define PERI_CRG_DDR_PPLL1_ERR_BOOT_CNT_DDR_PPLL1_ERR_BOOT_CNT_SHIFT                   0
#define PERI_CRG_DDR_PPLL1_ERR_BOOT_CNT_DDR_PPLL1_ERR_BOOT_CNT_MASK                    0x00ffffff
#define PERI_CRG_DDR_PPLL1_ERR_BOOT_CNT_DDR_PPLL1_PRESS_TEST_END_SHIFT                 24
#define PERI_CRG_DDR_PPLL1_ERR_BOOT_CNT_DDR_PPLL1_PRESS_TEST_END_MASK                  0x01000000
#define PERI_CRG_DDR_PPLL1_VOTE_MASK               0x0b3c
#define PERI_CRG_DDR_PPLL1_VOTE_MASK_DDR_PPLL1_HW_VOTE_MASK_SHIFT                      0
#define PERI_CRG_DDR_PPLL1_VOTE_MASK_DDR_PPLL1_HW_VOTE_MASK_MASK                       0x000000ff
#define PERI_CRG_DDR_PPLL1_VOTE_MASK_DDR_PPLL1_SFT_VOTE_MASK_SHIFT                     8
#define PERI_CRG_DDR_PPLL1_VOTE_MASK_DDR_PPLL1_SFT_VOTE_MASK_MASK                      0x0000ff00
#define PERI_CRG_DDR_PPLL1_VOTE_MASK_DDR_PPLL1_CFG_RSV2_SHIFT                          16
#define PERI_CRG_DDR_PPLL1_VOTE_MASK_DDR_PPLL1_CFG_RSV2_MASK                           0xffff0000
#define PERI_CRG_DDR_PPLL1_VOTE_BYPASS             0x0b44
#define PERI_CRG_DDR_PPLL1_VOTE_BYPASS_DDR_PPLL1_VOTE_BYPASS_SHIFT                     0
#define PERI_CRG_DDR_PPLL1_VOTE_BYPASS_DDR_PPLL1_VOTE_BYPASS_MASK                      0x000000ff
#define PERI_CRG_DDR_PPLL1_VOTE_GT                 0x0b48
#define PERI_CRG_DDR_PPLL1_VOTE_GT_DDR_PPLL1_VOTE_GT_SHIFT                             0
#define PERI_CRG_DDR_PPLL1_VOTE_GT_DDR_PPLL1_VOTE_GT_MASK                              0x000000ff
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG               0x0b4c
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG_DDR_PPLL1_SSMOD_EN_SHIFT                          0
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG_DDR_PPLL1_SSMOD_EN_MASK                           0x00000001
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG_DDR_PPLL1_DIVVAL_SHIFT                            1
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG_DDR_PPLL1_DIVVAL_MASK                             0x0000007e
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG_DDR_PPLL1_SPREAD_SHIFT                            7
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG_DDR_PPLL1_SPREAD_MASK                             0x00000f80
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG_DDR_PPLL1_DOWNSPREAD_SHIFT                        12
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG_DDR_PPLL1_DOWNSPREAD_MASK                         0x00001000
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG_DDR_PPLL1_CFG_RSV3_SHIFT                          13
#define PERI_CRG_DDR_PPLL1_SSMOD_CFG_DDR_PPLL1_CFG_RSV3_MASK                           0xffffe000
#define PERI_CRG_DDR_PPLL1_CTRL0                   0x0b50
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_EN_SHIFT                                    0
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_EN_MASK                                     0x00000001
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_BYPASS_SHIFT                                1
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_BYPASS_MASK                                 0x00000002
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_REFDIV_SHIFT                                2
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_REFDIV_MASK                                 0x000000fc
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_FBDIV_SHIFT                                 8
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_FBDIV_MASK                                  0x000fff00
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_POSTDIV1_SHIFT                              20
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_POSTDIV1_MASK                               0x00700000
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_POSTDIV2_SHIFT                              23
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_POSTDIV2_MASK                               0x03800000
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_CFG_VALID_SHIFT                             26
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_CFG_VALID_MASK                              0x04000000
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_GT_SHIFT                                    27
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_GT_MASK                                     0x08000000
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_CFG_RSV4_SHIFT                              28
#define PERI_CRG_DDR_PPLL1_CTRL0_DDR_PPLL1_CFG_RSV4_MASK                               0xf0000000
#define PERI_CRG_DDR_PPLL1_CTRL1                   0x0b54
#define PERI_CRG_DDR_PPLL1_CTRL1_DDR_PPLL1_FRAC_SHIFT                                  0
#define PERI_CRG_DDR_PPLL1_CTRL1_DDR_PPLL1_FRAC_MASK                                   0x00ffffff
#define PERI_CRG_DDR_PPLL1_CTRL1_DDR_PPLL1_DSM_EN_SHIFT                                24
#define PERI_CRG_DDR_PPLL1_CTRL1_DDR_PPLL1_DSM_EN_MASK                                 0x01000000
#define PERI_CRG_DDR_PPLL1_CTRL1_DDR_PPLL1_CFG_RSV5_SHIFT                              25
#define PERI_CRG_DDR_PPLL1_CTRL1_DDR_PPLL1_CFG_RSV5_MASK                               0xfe000000
#define PERI_CRG_DDR_PPLL1_DEBUG                   0x0b58
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETCALBYP_SHIFT                          0
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETCALBYP_MASK                           0x00000001
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETCALCNT_SHIFT                          1
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETCALCNT_MASK                           0x0000000e
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETCALEN_SHIFT                           4
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETCALEN_MASK                            0x00000010
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETCALIN_SHIFT                           5
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETCALIN_MASK                            0x0001ffe0
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETCALRSTN_SHIFT                         17
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETCALRSTN_MASK                          0x00020000
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETFASTCAL_SHIFT                         18
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_OFFSETFASTCAL_MASK                          0x00040000
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_CFG_RSV6_SHIFT                              19
#define PERI_CRG_DDR_PPLL1_DEBUG_DDR_PPLL1_CFG_RSV6_MASK                               0xfff80000
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE             0x0b5c
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_EN_STATE_SHIFT                        0
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_EN_STATE_MASK                         0x00000001
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_OFFSETCALOUT_SHIFT                    1
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_OFFSETCALOUT_MASK                     0x00001ffe
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_OFFSETCALLOCK_SHIFT                   13
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_OFFSETCALLOCK_MASK                    0x00002000
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_OFFSETCALOVF_SHIFT                    14
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_OFFSETCALOVF_MASK                     0x00004000
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_START_FSM_STATE_SHIFT                 15
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_START_FSM_STATE_MASK                  0x00038000
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_REAL_RETRY_CNT_SHIFT                  18
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_DDR_PPLL1_REAL_RETRY_CNT_MASK                   0x001c0000
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_ST_CLK_DDR1_PPLL1_SHIFT                         21
#define PERI_CRG_DDR_PPLL1_DEBUG_STATE_ST_CLK_DDR1_PPLL1_MASK                          0x00200000
#define PERI_CRG_CLKDIV33                          0x0c04
#define PERI_CRG_CLKDIV33_DIV_PCLK_DMA_BUS_LPPLL_SHIFT                                 0
#define PERI_CRG_CLKDIV33_DIV_PCLK_DMA_BUS_LPPLL_MASK                                  0x0000003f
#define PERI_CRG_CLKDIV33_DIV_PCLK_DMA_BUS_GPLL_SHIFT                                  6
#define PERI_CRG_CLKDIV33_DIV_PCLK_DMA_BUS_GPLL_MASK                                   0x00000fc0
#define PERI_CRG_CLKDIV33_SC_GT_PCLK_DMA_BUS_LPPLL_DIV_SHIFT                           12
#define PERI_CRG_CLKDIV33_SC_GT_PCLK_DMA_BUS_LPPLL_DIV_MASK                            0x00001000
#define PERI_CRG_CLKDIV33_SC_GT_PCLK_DMA_BUS_GPLL_DIV_SHIFT                            13
#define PERI_CRG_CLKDIV33_SC_GT_PCLK_DMA_BUS_GPLL_DIV_MASK                             0x00002000
#define PERI_CRG_CLKDIV34                          0x0c08
#define PERI_CRG_CLKDIV34_DIV_PCLK_CRG_CORE_LPPLL_SHIFT                                0
#define PERI_CRG_CLKDIV34_DIV_PCLK_CRG_CORE_LPPLL_MASK                                 0x0000003f
#define PERI_CRG_CLKDIV34_DIV_PCLK_CRG_CORE_GPLL_SHIFT                                 6
#define PERI_CRG_CLKDIV34_DIV_PCLK_CRG_CORE_GPLL_MASK                                  0x00000fc0
#define PERI_CRG_CLKDIV34_SC_GT_PCLK_CRG_CORE_LPPLL_SHIFT                              12
#define PERI_CRG_CLKDIV34_SC_GT_PCLK_CRG_CORE_LPPLL_MASK                               0x00001000
#define PERI_CRG_CLKDIV34_SC_GT_PCLK_CRG_CORE_GPLL_SHIFT                               13
#define PERI_CRG_CLKDIV34_SC_GT_PCLK_CRG_CORE_GPLL_MASK                                0x00002000
#define PERI_CRG_CLKDIV35                          0x0c10
#define PERI_CRG_CLKDIV35_DIV_CLK_PERI2NPU_NOC_SHIFT                                   0
#define PERI_CRG_CLKDIV35_DIV_CLK_PERI2NPU_NOC_MASK                                    0x0000003f
#define PERI_CRG_CLKDIV35_DIV_CLK_PERI2NPU_NIC_SHIFT                                   6
#define PERI_CRG_CLKDIV35_DIV_CLK_PERI2NPU_NIC_MASK                                    0x00000fc0
#define PERI_CRG_CLKDIV35_SC_GT_CLK_PERI2NPU_NOC_SHIFT                                 12
#define PERI_CRG_CLKDIV35_SC_GT_CLK_PERI2NPU_NOC_MASK                                  0x00001000
#define PERI_CRG_CLKDIV35_SC_GT_CLK_PERI2NPU_NIC_SHIFT                                 13
#define PERI_CRG_CLKDIV35_SC_GT_CLK_PERI2NPU_NIC_MASK                                  0x00002000
#define PERI_CRG_AUTOFSCTRL0                       0x0d00
#define PERI_CRG_AUTOFSCTRL0_DMA_BUS_TGT_NO_PENDING_TRANS_BYPASS_SHIFT                 0
#define PERI_CRG_AUTOFSCTRL0_DMA_BUS_TGT_NO_PENDING_TRANS_BYPASS_MASK                  0x0000000f
#define PERI_CRG_AUTOFSCTRL0_DMA_BUS_INI_NO_PENDING_TRANS_BYPASS_SHIFT                 4
#define PERI_CRG_AUTOFSCTRL0_DMA_BUS_INI_NO_PENDING_TRANS_BYPASS_MASK                  0x000001f0
#define PERI_CRG_AUTOFSCTRL0_IDLE_FLAG_DMA_NS_BYPASS_SHIFT                             9
#define PERI_CRG_AUTOFSCTRL0_IDLE_FLAG_DMA_NS_BYPASS_MASK                              0x00000200
#define PERI_CRG_AUTOFSCTRL0_IDLE_FLAG_DMA_S_BYPASS_SHIFT                              10
#define PERI_CRG_AUTOFSCTRL0_IDLE_FLAG_DMA_S_BYPASS_MASK                               0x00000400
#define PERI_CRG_AUTOFSCTRL1                       0x0d04
#define PERI_CRG_AUTOFSCTRL1_DEBOUNCE_IN_DMA_BUS_SHIFT                                 0
#define PERI_CRG_AUTOFSCTRL1_DEBOUNCE_IN_DMA_BUS_MASK                                  0x000003ff
#define PERI_CRG_AUTOFSCTRL1_DEBOUNCE_OUT_DMA_BUS_SHIFT                                10
#define PERI_CRG_AUTOFSCTRL1_DEBOUNCE_OUT_DMA_BUS_MASK                                 0x000ffc00
#define PERI_CRG_AUTOFSCTRL1_DEBOUNCE_BYPASS_DMA_BUS_SHIFT                             20
#define PERI_CRG_AUTOFSCTRL1_DEBOUNCE_BYPASS_DMA_BUS_MASK                              0x00100000
#define PERI_CRG_AUTOFSCTRL1_AUTOFS_EN_DMA_BUS_SHIFT                                   31
#define PERI_CRG_AUTOFSCTRL1_AUTOFS_EN_DMA_BUS_MASK                                    0x80000000
#define PERI_CRG_AUTOFSCTRL2                       0x0d08
#define PERI_CRG_AUTOFSCTRL2_DIV_AUTO_CLK_DMA_BUS_DATA_GPLL_SHIFT                      0
#define PERI_CRG_AUTOFSCTRL2_DIV_AUTO_CLK_DMA_BUS_DATA_GPLL_MASK                       0x0000003f
#define PERI_CRG_AUTOFSCTRL2_DIV_AUTO_CLK_DMA_BUS_DATA_LPPLL_SHIFT                     6
#define PERI_CRG_AUTOFSCTRL2_DIV_AUTO_CLK_DMA_BUS_DATA_LPPLL_MASK                      0x00000fc0
#define PERI_CRG_AUTOFSCTRL3                       0x0d10
#define PERI_CRG_AUTOFSCTRL3_SYS1_BUS_TGT_NO_PENDING_TRANS_BYPASS_SHIFT                0
#define PERI_CRG_AUTOFSCTRL3_SYS1_BUS_TGT_NO_PENDING_TRANS_BYPASS_MASK                 0x00000003
#define PERI_CRG_AUTOFSCTRL3_SYS1_BUS_INI_NO_PENDING_TRANS_BYPASS_SHIFT                2
#define PERI_CRG_AUTOFSCTRL3_SYS1_BUS_INI_NO_PENDING_TRANS_BYPASS_MASK                 0x00000ffc
#define PERI_CRG_AUTOFSCTRL3_SYS0_BUS_TGT_NO_PENDING_TRANS_BYPASS_SHIFT                12
#define PERI_CRG_AUTOFSCTRL3_SYS0_BUS_TGT_NO_PENDING_TRANS_BYPASS_MASK                 0x00003000
#define PERI_CRG_AUTOFSCTRL3_SYS0_BUS_INI_NO_PENDING_TRANS_BYPASS_SHIFT                14
#define PERI_CRG_AUTOFSCTRL3_SYS0_BUS_INI_NO_PENDING_TRANS_BYPASS_MASK                 0x0001c000
#define PERI_CRG_AUTOFSCTRL3_PERI_BROADCAST_BUS_NO_PENDING_TRANS_BYPASS_SHIFT          17
#define PERI_CRG_AUTOFSCTRL3_PERI_BROADCAST_BUS_NO_PENDING_TRANS_BYPASS_MASK           0x00020000
#define PERI_CRG_AUTOFSCTRL3_DMA_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_SHIFT          18
#define PERI_CRG_AUTOFSCTRL3_DMA_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_MASK           0x00040000
#define PERI_CRG_AUTOFSCTRL3_XRSE_M85_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_SHIFT     19
#define PERI_CRG_AUTOFSCTRL3_XRSE_M85_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_MASK      0x00080000
#define PERI_CRG_AUTOFSCTRL3_HSS2_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_SHIFT         20
#define PERI_CRG_AUTOFSCTRL3_HSS2_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_MASK          0x00100000
#define PERI_CRG_AUTOFSCTRL3_HSS1_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_SHIFT         21
#define PERI_CRG_AUTOFSCTRL3_HSS1_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_MASK          0x00200000
#define PERI_CRG_AUTOFSCTRL3_LPIS_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_SHIFT         22
#define PERI_CRG_AUTOFSCTRL3_LPIS_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_MASK          0x00400000
#define PERI_CRG_AUTOFSCTRL3_NPU_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_SHIFT          23
#define PERI_CRG_AUTOFSCTRL3_NPU_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_MASK           0x00800000
#define PERI_CRG_AUTOFSCTRL3_MEDIA2_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_SHIFT       24
#define PERI_CRG_AUTOFSCTRL3_MEDIA2_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_MASK        0x01000000
#define PERI_CRG_AUTOFSCTRL3_MEDIA1_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_SHIFT       25
#define PERI_CRG_AUTOFSCTRL3_MEDIA1_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_MASK        0x02000000
#define PERI_CRG_AUTOFSCTRL3_CPU_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_SHIFT          26
#define PERI_CRG_AUTOFSCTRL3_CPU_BUS_TGT_NO_PENDING_TRANS_SYSBUS_BYPASS_MASK           0x04000000
#define PERI_CRG_AUTOFSCTRL3_XCTRL_DDR_BUS_NO_PENDING_TRANS_SYSBUS_BYPASS_SHIFT        27
#define PERI_CRG_AUTOFSCTRL3_XCTRL_DDR_BUS_NO_PENDING_TRANS_SYSBUS_BYPASS_MASK         0x08000000
#define PERI_CRG_AUTOFSCTRL3_IDLE_FLAG_CFGBUS2OCM_NIC_SYSBUS_BYPASS_SHIFT              28
#define PERI_CRG_AUTOFSCTRL3_IDLE_FLAG_CFGBUS2OCM_NIC_SYSBUS_BYPASS_MASK               0x10000000
#define PERI_CRG_AUTOFSCTRL3_IDLE_FLAG_OCM_NIC_SYSBUS_BYPASS_SHIFT                     29
#define PERI_CRG_AUTOFSCTRL3_IDLE_FLAG_OCM_NIC_SYSBUS_BYPASS_MASK                      0x20000000
#define PERI_CRG_AUTOFSCTRL4                       0x0d14
#define PERI_CRG_AUTOFSCTRL4_DEBOUNCE_IN_SYSBUS_SHIFT                                  0
#define PERI_CRG_AUTOFSCTRL4_DEBOUNCE_IN_SYSBUS_MASK                                   0x000003ff
#define PERI_CRG_AUTOFSCTRL4_DEBOUNCE_OUT_SYSBUS_SHIFT                                 10
#define PERI_CRG_AUTOFSCTRL4_DEBOUNCE_OUT_SYSBUS_MASK                                  0x000ffc00
#define PERI_CRG_AUTOFSCTRL4_DEBOUNCE_BYPASS_SYSBUS_SHIFT                              20
#define PERI_CRG_AUTOFSCTRL4_DEBOUNCE_BYPASS_SYSBUS_MASK                               0x00100000
#define PERI_CRG_AUTOFSCTRL4_AUTOFS_EN_SYSBUS_SHIFT                                    31
#define PERI_CRG_AUTOFSCTRL4_AUTOFS_EN_SYSBUS_MASK                                     0x80000000
#define PERI_CRG_AUTOFSCTRL5                       0x0d18
#define PERI_CRG_AUTOFSCTRL5_DIV_AUTO_CLK_SYSBUS_DATA_GPLL_SHIFT                       0
#define PERI_CRG_AUTOFSCTRL5_DIV_AUTO_CLK_SYSBUS_DATA_GPLL_MASK                        0x0000003f
#define PERI_CRG_AUTOFSCTRL5_DIV_AUTO_CLK_SYSBUS_DATA_LPPLL_SHIFT                      6
#define PERI_CRG_AUTOFSCTRL5_DIV_AUTO_CLK_SYSBUS_DATA_LPPLL_MASK                       0x00000fc0
#define PERI_CRG_AUTOFSCTRL6                       0x0d20
#define PERI_CRG_AUTOFSCTRL6_PERICFG_BUS_TGT_NO_PENDING_TRANS_BYPASS_SHIFT             0
#define PERI_CRG_AUTOFSCTRL6_PERICFG_BUS_TGT_NO_PENDING_TRANS_BYPASS_MASK              0x00000007
#define PERI_CRG_AUTOFSCTRL6_PERICFG_BUS_INI_NO_PENDING_TRANS_BYPASS_SHIFT             3
#define PERI_CRG_AUTOFSCTRL6_PERICFG_BUS_INI_NO_PENDING_TRANS_BYPASS_MASK              0x00000008
#define PERI_CRG_AUTOFSCTRL6_SYSCFG_BUS_TGT_NO_PENDING_TRANS_BYPASS_SHIFT              4
#define PERI_CRG_AUTOFSCTRL6_SYSCFG_BUS_TGT_NO_PENDING_TRANS_BYPASS_MASK               0x03fffff0
#define PERI_CRG_AUTOFSCTRL7                       0x0d24
#define PERI_CRG_AUTOFSCTRL7_SYSCFG_BUS_INI_NO_PENDING_TRANS_BYPASS_SHIFT              0
#define PERI_CRG_AUTOFSCTRL7_SYSCFG_BUS_INI_NO_PENDING_TRANS_BYPASS_MASK               0x000003ff
#define PERI_CRG_AUTOFSCTRL7_XRSE_M85_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_SHIFT     10
#define PERI_CRG_AUTOFSCTRL7_XRSE_M85_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_MASK      0x00000400
#define PERI_CRG_AUTOFSCTRL7_MEDIA2_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_SHIFT       11
#define PERI_CRG_AUTOFSCTRL7_MEDIA2_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_MASK        0x00000800
#define PERI_CRG_AUTOFSCTRL7_LPIS_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_SHIFT         12
#define PERI_CRG_AUTOFSCTRL7_LPIS_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_MASK          0x00001000
#define PERI_CRG_AUTOFSCTRL7_HSS2_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_SHIFT         13
#define PERI_CRG_AUTOFSCTRL7_HSS2_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_MASK          0x00002000
#define PERI_CRG_AUTOFSCTRL7_HSS1_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_SHIFT         14
#define PERI_CRG_AUTOFSCTRL7_HSS1_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_MASK          0x00004000
#define PERI_CRG_AUTOFSCTRL7_DMA_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_SHIFT          15
#define PERI_CRG_AUTOFSCTRL7_DMA_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_MASK           0x00008000
#define PERI_CRG_AUTOFSCTRL7_CPU_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_SHIFT          16
#define PERI_CRG_AUTOFSCTRL7_CPU_BUS_TGT_NO_PENDING_TRANS_CFGBUS_BYPASS_MASK           0x00010000
#define PERI_CRG_AUTOFSCTRL7_IDLE_FLAG_NPU_NIC2CFGBUS_BYPASS_SHIFT                     17
#define PERI_CRG_AUTOFSCTRL7_IDLE_FLAG_NPU_NIC2CFGBUS_BYPASS_MASK                      0x00020000
#define PERI_CRG_AUTOFSCTRL8                       0x0d28
#define PERI_CRG_AUTOFSCTRL8_DEBOUNCE_IN_CFGBUS_SHIFT                                  0
#define PERI_CRG_AUTOFSCTRL8_DEBOUNCE_IN_CFGBUS_MASK                                   0x000003ff
#define PERI_CRG_AUTOFSCTRL8_DEBOUNCE_OUT_CFGBUS_SHIFT                                 10
#define PERI_CRG_AUTOFSCTRL8_DEBOUNCE_OUT_CFGBUS_MASK                                  0x000ffc00
#define PERI_CRG_AUTOFSCTRL8_DEBOUNCE_BYPASS_CFGBUS_SHIFT                              20
#define PERI_CRG_AUTOFSCTRL8_DEBOUNCE_BYPASS_CFGBUS_MASK                               0x00100000
#define PERI_CRG_AUTOFSCTRL8_AUTOFS_EN_CFGBUS_SHIFT                                    31
#define PERI_CRG_AUTOFSCTRL8_AUTOFS_EN_CFGBUS_MASK                                     0x80000000
#define PERI_CRG_AUTOFSCTRL9                       0x0d2c
#define PERI_CRG_AUTOFSCTRL9_DIV_AUTO_CLK_CFGBUS_GPLL_SHIFT                            0
#define PERI_CRG_AUTOFSCTRL9_DIV_AUTO_CLK_CFGBUS_GPLL_MASK                             0x0000003f
#define PERI_CRG_AUTOFSCTRL9_DIV_AUTO_CLK_CFGBUS_LPPLL_SHIFT                           6
#define PERI_CRG_AUTOFSCTRL9_DIV_AUTO_CLK_CFGBUS_LPPLL_MASK                            0x00000fc0
#define PERI_CRG_AUTOFSCTRL10                      0x0d30
#define PERI_CRG_AUTOFSCTRL10_IDLE_FLAG_XCTRL_CPU_M3_BYPASS_SHIFT                      12
#define PERI_CRG_AUTOFSCTRL10_IDLE_FLAG_XCTRL_CPU_M3_BYPASS_MASK                       0x00001000
#define PERI_CRG_AUTOFSCTRL10_XCTRL_CPU_BUS_NO_PENDING_TRANS_BYPASS_SHIFT              13
#define PERI_CRG_AUTOFSCTRL10_XCTRL_CPU_BUS_NO_PENDING_TRANS_BYPASS_MASK               0x00002000
#define PERI_CRG_AUTOFSCTRL10_CPU_BUS_TGT_NO_PENDING_TRANS_XCTRL_CPU_BYPASS_SHIFT      14
#define PERI_CRG_AUTOFSCTRL10_CPU_BUS_TGT_NO_PENDING_TRANS_XCTRL_CPU_BYPASS_MASK       0x00004000
#define PERI_CRG_AUTOFSCTRL11                      0x0d34
#define PERI_CRG_AUTOFSCTRL11_DEBOUNCE_IN_XCTRL_CPU_SHIFT                              0
#define PERI_CRG_AUTOFSCTRL11_DEBOUNCE_IN_XCTRL_CPU_MASK                               0x000003ff
#define PERI_CRG_AUTOFSCTRL11_DEBOUNCE_OUT_XCTRL_CPU_SHIFT                             10
#define PERI_CRG_AUTOFSCTRL11_DEBOUNCE_OUT_XCTRL_CPU_MASK                              0x000ffc00
#define PERI_CRG_AUTOFSCTRL11_DEBOUNCE_BYPASS_XCTRL_CPU_SHIFT                          20
#define PERI_CRG_AUTOFSCTRL11_DEBOUNCE_BYPASS_XCTRL_CPU_MASK                           0x00100000
#define PERI_CRG_AUTOFSCTRL11_DIV_AUTO_CLK_XCTRL_CPU_SHIFT                             21
#define PERI_CRG_AUTOFSCTRL11_DIV_AUTO_CLK_XCTRL_CPU_MASK                              0x07e00000
#define PERI_CRG_AUTOFSCTRL11_AUTOFS_EN_XCTRL_CPU_SHIFT                                31
#define PERI_CRG_AUTOFSCTRL11_AUTOFS_EN_XCTRL_CPU_MASK                                 0x80000000
#define PERI_CRG_AUTOFSCTRL12                      0x0d38
#define PERI_CRG_AUTOFSCTRL12_IDLE_FLAG_XCTRL_DDR_M3_BYPASS_SHIFT                      0
#define PERI_CRG_AUTOFSCTRL12_IDLE_FLAG_XCTRL_DDR_M3_BYPASS_MASK                       0x00000001
#define PERI_CRG_AUTOFSCTRL12_XCTRL_DDR_BUS_NO_PENDING_TRANS_BYPASS_SHIFT              1
#define PERI_CRG_AUTOFSCTRL12_XCTRL_DDR_BUS_NO_PENDING_TRANS_BYPASS_MASK               0x00000002
#define PERI_CRG_AUTOFSCTRL12_SYSCFG_BUS_TGT_NO_PENDING_TRANS_XCTRL_DDR_BYPASS_SHIFT   2
#define PERI_CRG_AUTOFSCTRL12_SYSCFG_BUS_TGT_NO_PENDING_TRANS_XCTRL_DDR_BYPASS_MASK    0x00000004
#define PERI_CRG_AUTOFSCTRL13                      0x0d3c
#define PERI_CRG_AUTOFSCTRL13_DEBOUNCE_IN_XCTRL_DDR_SHIFT                              0
#define PERI_CRG_AUTOFSCTRL13_DEBOUNCE_IN_XCTRL_DDR_MASK                               0x000003ff
#define PERI_CRG_AUTOFSCTRL13_DEBOUNCE_OUT_XCTRL_DDR_SHIFT                             10
#define PERI_CRG_AUTOFSCTRL13_DEBOUNCE_OUT_XCTRL_DDR_MASK                              0x000ffc00
#define PERI_CRG_AUTOFSCTRL13_DEBOUNCE_BYPASS_XCTRL_DDR_SHIFT                          20
#define PERI_CRG_AUTOFSCTRL13_DEBOUNCE_BYPASS_XCTRL_DDR_MASK                           0x00100000
#define PERI_CRG_AUTOFSCTRL13_DIV_AUTO_CLK_XCTRL_DDR_SHIFT                             21
#define PERI_CRG_AUTOFSCTRL13_DIV_AUTO_CLK_XCTRL_DDR_MASK                              0x07e00000
#define PERI_CRG_AUTOFSCTRL13_AUTOFS_EN_XCTRL_DDR_SHIFT                                31
#define PERI_CRG_AUTOFSCTRL13_AUTOFS_EN_XCTRL_DDR_MASK                                 0x80000000
#define PERI_CRG_AUTOFSCTRL14                      0x0d40
#define PERI_CRG_AUTOFSCTRL14_PERI_BROADCAST_BUS_TGT_NO_PENDING_TRANS_PMPU_BYPASS_SHIFT 0
#define PERI_CRG_AUTOFSCTRL14_PERI_BROADCAST_BUS_TGT_NO_PENDING_TRANS_PMPU_BYPASS_MASK  0x00000001
#define PERI_CRG_AUTOFSCTRL14_MAIN_BUS_NO_PENDING_TRANS_BYPASS_SHIFT                   1
#define PERI_CRG_AUTOFSCTRL14_MAIN_BUS_NO_PENDING_TRANS_BYPASS_MASK                    0x00000002
#define PERI_CRG_AUTOFSCTRL14_SYSCFG_BUS_TGT_NO_PENDING_TRANS_MAINBUS_BYPASS_SHIFT     2
#define PERI_CRG_AUTOFSCTRL14_SYSCFG_BUS_TGT_NO_PENDING_TRANS_MAINBUS_BYPASS_MASK      0x00000004
#define PERI_CRG_AUTOFSCTRL14_SYS1_BUS_TGT_NO_PENDING_TRANS_MAINBUS_BYPASS_SHIFT       3
#define PERI_CRG_AUTOFSCTRL14_SYS1_BUS_TGT_NO_PENDING_TRANS_MAINBUS_BYPASS_MASK        0x00000008
#define PERI_CRG_AUTOFSCTRL14_MEDIA2_BUS_TGT_NO_PENDING_TRANS_BYPASS_SHIFT             4
#define PERI_CRG_AUTOFSCTRL14_MEDIA2_BUS_TGT_NO_PENDING_TRANS_BYPASS_MASK              0x00000030
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_NDB_MST_M2_BYPASS_SHIFT                        6
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_NDB_MST_M2_BYPASS_MASK                         0x00000040
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_NDB_SLV_M2_BYPASS_SHIFT                        7
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_NDB_SLV_M2_BYPASS_MASK                         0x00000080
#define PERI_CRG_AUTOFSCTRL14_MEDIA1_BUS_TGT_NO_PENDING_TRANS_BYPASS_SHIFT             8
#define PERI_CRG_AUTOFSCTRL14_MEDIA1_BUS_TGT_NO_PENDING_TRANS_BYPASS_MASK              0x00000300
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_NDB_MST_M1_BYPASS_SHIFT                        10
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_NDB_MST_M1_BYPASS_MASK                         0x00000400
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_NDB_SLV_M1_BYPASS_SHIFT                        11
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_NDB_SLV_M1_BYPASS_MASK                         0x00000800
#define PERI_CRG_AUTOFSCTRL14_NPU_BUS_TGT_NO_PENDING_TRANS_BYPASS_SHIFT                12
#define PERI_CRG_AUTOFSCTRL14_NPU_BUS_TGT_NO_PENDING_TRANS_BYPASS_MASK                 0x0000f000
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_ADB_MST_NPU_BYPASS_SHIFT                       16
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_ADB_MST_NPU_BYPASS_MASK                        0x00010000
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_ADB_SLV_NPU_BYPASS_SHIFT                       17
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_ADB_SLV_NPU_BYPASS_MASK                        0x00020000
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_ADB_MST_GPU_BYPASS_SHIFT                       18
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_ADB_MST_GPU_BYPASS_MASK                        0x00040000
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_ADB_SLV_GPU_BYPASS_SHIFT                       19
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_ADB_SLV_GPU_BYPASS_MASK                        0x00080000
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_MAIN_ADB_MST_CPU_BYPASS_SHIFT                  20
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_MAIN_ADB_MST_CPU_BYPASS_MASK                   0x00100000
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_MAIN_ADB_SLV_CPU_BYPASS_SHIFT                  21
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_MAIN_ADB_SLV_CPU_BYPASS_MASK                   0x00200000
#define PERI_CRG_AUTOFSCTRL14_IDLE_CPU_FAST_WAKEUP_BYPASS_SHIFT                        22
#define PERI_CRG_AUTOFSCTRL14_IDLE_CPU_FAST_WAKEUP_BYPASS_MASK                         0x03c00000
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_PERFSTAT_PERI_BYPASS_SHIFT                     26
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_PERFSTAT_PERI_BYPASS_MASK                      0x04000000
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_PERFSTAT_GPU_BYPASS_SHIFT                      27
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_PERFSTAT_GPU_BYPASS_MASK                       0x08000000
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_PERFSTAT_CPU_BYPASS_SHIFT                      28
#define PERI_CRG_AUTOFSCTRL14_IDLE_FLAG_PERFSTAT_CPU_BYPASS_MASK                       0x10000000
#define PERI_CRG_AUTOFSCTRL15                      0x0d44
#define PERI_CRG_AUTOFSCTRL15_DEBOUNCE_IN_TIME_MAINBUS_SHIFT                           0
#define PERI_CRG_AUTOFSCTRL15_DEBOUNCE_IN_TIME_MAINBUS_MASK                            0x000003ff
#define PERI_CRG_AUTOFSCTRL15_DEBOUNCE_OUT_TIME_MAINBUS_SHIFT                          10
#define PERI_CRG_AUTOFSCTRL15_DEBOUNCE_OUT_TIME_MAINBUS_MASK                           0x000ffc00
#define PERI_CRG_AUTOFSCTRL15_DEBOUNCE_BYPASS_MAINBUS_SHIFT                            20
#define PERI_CRG_AUTOFSCTRL15_DEBOUNCE_BYPASS_MAINBUS_MASK                             0x00100000
#define PERI_CRG_AUTOFSCTRL15_AUTOFS_EN_MAINBUS_SHIFT                                  30
#define PERI_CRG_AUTOFSCTRL15_AUTOFS_EN_MAINBUS_MASK                                   0x40000000
#define PERI_CRG_AUTOFSCTRL15_AUTOGT_BYPASS_MAINBUS_LPCTRL_SHIFT                       31
#define PERI_CRG_AUTOFSCTRL15_AUTOGT_BYPASS_MAINBUS_LPCTRL_MASK                        0x80000000
#define PERI_CRG_AUTOFSCTRL16                      0x0d48
#define PERI_CRG_AUTOFSCTRL16_DIV_AUTO_CLK_MAINBUS_GPLL_SHIFT                          0
#define PERI_CRG_AUTOFSCTRL16_DIV_AUTO_CLK_MAINBUS_GPLL_MASK                           0x0000003f
#define PERI_CRG_AUTOFSCTRL16_DIV_AUTO_CLK_MAINBUS_LPPLL_SHIFT                         6
#define PERI_CRG_AUTOFSCTRL16_DIV_AUTO_CLK_MAINBUS_LPPLL_MASK                          0x00000fc0
#define PERI_CRG_PERICTRL9                         0x0d60
#define PERI_CRG_PERICTRL9_DEBOUNCE_IN_TIME_DFICLK_OFF_SHIFT                           0
#define PERI_CRG_PERICTRL9_DEBOUNCE_IN_TIME_DFICLK_OFF_MASK                            0x000003ff
#define PERI_CRG_PERICTRL9_DEBOUNCE_OUT_TIME_DFICLK_OFF_SHIFT                          10
#define PERI_CRG_PERICTRL9_DEBOUNCE_OUT_TIME_DFICLK_OFF_MASK                           0x000ffc00
#define PERI_CRG_PERICTRL9_DEBOUNCE_BYPASS_DFICLK_OFF_SHIFT                            20
#define PERI_CRG_PERICTRL9_DEBOUNCE_BYPASS_DFICLK_OFF_MASK                             0x00100000
#define PERI_CRG_PERICTRL9_MAINBUS_CTRL_DFICLK_BYPASS_SHIFT                            21
#define PERI_CRG_PERICTRL9_MAINBUS_CTRL_DFICLK_BYPASS_MASK                             0x00200000
#define PERI_CRG_PERICTRL9_AUTOGT_CLK_DDR_DFICLK_BYPASS_SHIFT                          22
#define PERI_CRG_PERICTRL9_AUTOGT_CLK_DDR_DFICLK_BYPASS_MASK                           0x00400000
#define PERI_CRG_PERICTRL9_PERI_BROADCAST_BUS_TGT_NO_PENDING_TRANS_DDR_BYPASS_SHIFT    24
#define PERI_CRG_PERICTRL9_PERI_BROADCAST_BUS_TGT_NO_PENDING_TRANS_DDR_BYPASS_MASK     0x0f000000
#define PERI_CRG_PERICTRL10                        0x0d64
#define PERI_CRG_PERICTRL10_AUTOGT_CLK_DDR_DFICLK_VOTE_BYPASS_SHIFT                    0
#define PERI_CRG_PERICTRL10_AUTOGT_CLK_DDR_DFICLK_VOTE_BYPASS_MASK                     0x0000ffff
#define PERI_CRG_PERICTRL6                         0x0e00
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA1_CTRL_BYPASS_SHIFT                        0
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA1_CTRL_BYPASS_MASK                         0x00000001
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA1_LPCTRL_FT_BYPASS_SHIFT                   1
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA1_LPCTRL_FT_BYPASS_MASK                    0x00000002
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA2_CTRL_BYPASS_SHIFT                        2
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA2_CTRL_BYPASS_MASK                         0x00000004
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA2_LPCTRL_FT_BYPASS_SHIFT                   3
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA2_LPCTRL_FT_BYPASS_MASK                    0x00000008
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_CPU_CRG_BYPASS_SHIFT                            4
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_CPU_CRG_BYPASS_MASK                             0x00000010
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_GPU_CRG_BYPASS_SHIFT                            5
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_GPU_CRG_BYPASS_MASK                             0x00000020
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_HSS1_CRG_BYPASS_SHIFT                           6
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_HSS1_CRG_BYPASS_MASK                            0x00000040
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_HSS2_CRG_BYPASS_SHIFT                           7
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_HSS2_CRG_BYPASS_MASK                            0x00000080
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA1_CRG_BYPASS_SHIFT                         8
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA1_CRG_BYPASS_MASK                          0x00000100
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA2_CRG_BYPASS_SHIFT                         9
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA2_CRG_BYPASS_MASK                          0x00000200
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_NPU_CRG_BYPASS_SHIFT                            10
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_NPU_CRG_BYPASS_MASK                             0x00000400
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA1_SUBSYS_BYPASS_SHIFT                      11
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA1_SUBSYS_BYPASS_MASK                       0x00000800
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA1_DBG_BLK_BYPASS_SHIFT                     12
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_MEDIA1_DBG_BLK_BYPASS_MASK                      0x00001000
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_XCTRL_DDR_CRG_BYPASS_SHIFT                      13
#define PERI_CRG_PERICTRL6_CLKRST_FLAG_XCTRL_DDR_CRG_BYPASS_MASK                       0x00002000
#define PERI_CRG_AUTOFSCTRL20                      0x0e60
#define PERI_CRG_AUTOFSCTRL20_CPU_BUS_TGT_NO_PENDING_TRANS_BYPASS_SHIFT                0
#define PERI_CRG_AUTOFSCTRL20_CPU_BUS_TGT_NO_PENDING_TRANS_BYPASS_MASK                 0x0000007f
#define PERI_CRG_AUTOFSCTRL20_CPU_BUS_INI_NO_PENDING_TRANS_BYPASS_SHIFT                7
#define PERI_CRG_AUTOFSCTRL20_CPU_BUS_INI_NO_PENDING_TRANS_BYPASS_MASK                 0x00000f80
#define PERI_CRG_AUTOFSCTRL20_CPU_WFX_BYPASS_SHIFT                                     12
#define PERI_CRG_AUTOFSCTRL20_CPU_WFX_BYPASS_MASK                                      0x00001000
#define PERI_CRG_AUTOFSCTRL20_SYSCFG_BUS_TGT_NO_PENDING_TRANS_CPUBUS_BYPASS_SHIFT      13
#define PERI_CRG_AUTOFSCTRL20_SYSCFG_BUS_TGT_NO_PENDING_TRANS_CPUBUS_BYPASS_MASK       0x00002000
#define PERI_CRG_AUTOFSCTRL21                      0x0e64
#define PERI_CRG_AUTOFSCTRL21_DEBOUNCE_IN_TIME_CPU_BUS_SHIFT                           0
#define PERI_CRG_AUTOFSCTRL21_DEBOUNCE_IN_TIME_CPU_BUS_MASK                            0x000003ff
#define PERI_CRG_AUTOFSCTRL21_DEBOUNCE_OUT_TIME_CPU_BUS_SHIFT                          10
#define PERI_CRG_AUTOFSCTRL21_DEBOUNCE_OUT_TIME_CPU_BUS_MASK                           0x000ffc00
#define PERI_CRG_AUTOFSCTRL21_DEBOUNCE_BYPASS_CPU_BUS_SHIFT                            20
#define PERI_CRG_AUTOFSCTRL21_DEBOUNCE_BYPASS_CPU_BUS_MASK                             0x00100000
#define PERI_CRG_AUTOFSCTRL21_DIV_AUTO_CLK_CPU_BUS_SHIFT                               21
#define PERI_CRG_AUTOFSCTRL21_DIV_AUTO_CLK_CPU_BUS_MASK                                0x07e00000
#define PERI_CRG_AUTOFSCTRL21_AUTOFS_EN_CPU_BUS_SHIFT                                  31
#define PERI_CRG_AUTOFSCTRL21_AUTOFS_EN_CPU_BUS_MASK                                   0x80000000
#define PERI_CRG_REGFILE_CGBYPASS                  0x0e68
#define PERI_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                               0
#define PERI_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                                0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_peri_spinlock  :  1;
        unsigned int gt_clk_ocm             :  1;
        unsigned int gt_clk_npu_ocm_nic     :  1;
        unsigned int gt_clk_pcie_aux        :  1;
        unsigned int gt_clk_npu_ocm_adb_mst :  1;
        unsigned int gt_pclk_peri_timer_ns  :  1;
        unsigned int gt_clk_peri_timer0     :  1;
        unsigned int gt_clk_peri_timer1     :  1;
        unsigned int gt_clk_peri_timer2     :  1;
        unsigned int gt_clk_peri_timer3     :  1;
        unsigned int gt_clk_csi_sys         :  1;
        unsigned int gt_clk_dsi_sys         :  1;
        unsigned int gt_clk_sysbus_csi      :  1;
        unsigned int gt_clk_cfgbus_csi      :  1;
        unsigned int gt_clk_cfgbus_dsi      :  1;
        unsigned int gt_pclk_peri_ipc1      :  1;
        unsigned int gt_pclk_peri_ipc2      :  1;
        unsigned int gt_pclk_spi4           :  1;
        unsigned int gt_pclk_spi5           :  1;
        unsigned int gt_pclk_spi6           :  1;
        unsigned int gt_clk_spi4            :  1;
        unsigned int gt_clk_spi5            :  1;
        unsigned int gt_clk_spi6            :  1;
        unsigned int gt_pclk_peri_wdt1      :  1;
        unsigned int gt_pclk_peri_wdt0      :  1;
        unsigned int gt_clk_peri_wdt0       :  1;
        unsigned int gt_clk_peri_wdt1       :  1;
        unsigned int gt_clk_perfmon         :  1;
        unsigned int gt_pclk_perfmon_peri   :  1;
        unsigned int gt_pclk_perfmon        :  1;
        unsigned int gt_pclk_perfstat       :  1;
        unsigned int reserved_0             :  1;
    } reg;
}peri_crg_clkgt0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_peri_spinlock  :  1;
        unsigned int gt_clk_ocm             :  1;
        unsigned int gt_clk_npu_ocm_nic     :  1;
        unsigned int gt_clk_pcie_aux        :  1;
        unsigned int gt_clk_npu_ocm_adb_mst :  1;
        unsigned int gt_pclk_peri_timer_ns  :  1;
        unsigned int gt_clk_peri_timer0     :  1;
        unsigned int gt_clk_peri_timer1     :  1;
        unsigned int gt_clk_peri_timer2     :  1;
        unsigned int gt_clk_peri_timer3     :  1;
        unsigned int gt_clk_csi_sys         :  1;
        unsigned int gt_clk_dsi_sys         :  1;
        unsigned int gt_clk_sysbus_csi      :  1;
        unsigned int gt_clk_cfgbus_csi      :  1;
        unsigned int gt_clk_cfgbus_dsi      :  1;
        unsigned int gt_pclk_peri_ipc1      :  1;
        unsigned int gt_pclk_peri_ipc2      :  1;
        unsigned int gt_pclk_spi4           :  1;
        unsigned int gt_pclk_spi5           :  1;
        unsigned int gt_pclk_spi6           :  1;
        unsigned int gt_clk_spi4            :  1;
        unsigned int gt_clk_spi5            :  1;
        unsigned int gt_clk_spi6            :  1;
        unsigned int gt_pclk_peri_wdt1      :  1;
        unsigned int gt_pclk_peri_wdt0      :  1;
        unsigned int gt_clk_peri_wdt0       :  1;
        unsigned int gt_clk_peri_wdt1       :  1;
        unsigned int gt_clk_perfmon         :  1;
        unsigned int gt_pclk_perfmon_peri   :  1;
        unsigned int gt_pclk_perfmon        :  1;
        unsigned int gt_pclk_perfstat       :  1;
        unsigned int reserved_0             :  1;
    } reg;
}peri_crg_clkgt0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_peri_spinlock  :  1;
        unsigned int gt_clk_ocm             :  1;
        unsigned int gt_clk_npu_ocm_nic     :  1;
        unsigned int gt_clk_pcie_aux        :  1;
        unsigned int gt_clk_npu_ocm_adb_mst :  1;
        unsigned int gt_pclk_peri_timer_ns  :  1;
        unsigned int gt_clk_peri_timer0     :  1;
        unsigned int gt_clk_peri_timer1     :  1;
        unsigned int gt_clk_peri_timer2     :  1;
        unsigned int gt_clk_peri_timer3     :  1;
        unsigned int gt_clk_csi_sys         :  1;
        unsigned int gt_clk_dsi_sys         :  1;
        unsigned int gt_clk_sysbus_csi      :  1;
        unsigned int gt_clk_cfgbus_csi      :  1;
        unsigned int gt_clk_cfgbus_dsi      :  1;
        unsigned int gt_pclk_peri_ipc1      :  1;
        unsigned int gt_pclk_peri_ipc2      :  1;
        unsigned int gt_pclk_spi4           :  1;
        unsigned int gt_pclk_spi5           :  1;
        unsigned int gt_pclk_spi6           :  1;
        unsigned int gt_clk_spi4            :  1;
        unsigned int gt_clk_spi5            :  1;
        unsigned int gt_clk_spi6            :  1;
        unsigned int gt_pclk_peri_wdt1      :  1;
        unsigned int gt_pclk_peri_wdt0      :  1;
        unsigned int gt_clk_peri_wdt0       :  1;
        unsigned int gt_clk_peri_wdt1       :  1;
        unsigned int gt_clk_perfmon         :  1;
        unsigned int gt_pclk_perfmon_peri   :  1;
        unsigned int gt_pclk_perfmon        :  1;
        unsigned int gt_pclk_perfstat       :  1;
        unsigned int reserved_0             :  1;
    } reg;
}peri_crg_clkgt0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_pclk_peri_spinlock  :  1;
        unsigned int st_clk_ocm             :  1;
        unsigned int st_clk_npu_ocm_nic     :  1;
        unsigned int st_clk_pcie_aux        :  1;
        unsigned int st_clk_npu_ocm_adb_mst :  1;
        unsigned int st_pclk_peri_timer_ns  :  1;
        unsigned int st_clk_peri_timer0     :  1;
        unsigned int st_clk_peri_timer1     :  1;
        unsigned int st_clk_peri_timer2     :  1;
        unsigned int st_clk_peri_timer3     :  1;
        unsigned int st_clk_csi_sys         :  1;
        unsigned int st_clk_dsi_sys         :  1;
        unsigned int st_clk_sysbus_csi      :  1;
        unsigned int st_clk_cfgbus_csi      :  1;
        unsigned int st_clk_cfgbus_dsi      :  1;
        unsigned int st_pclk_peri_ipc1      :  1;
        unsigned int st_pclk_peri_ipc2      :  1;
        unsigned int st_pclk_spi4           :  1;
        unsigned int st_pclk_spi5           :  1;
        unsigned int st_pclk_spi6           :  1;
        unsigned int st_clk_spi4            :  1;
        unsigned int st_clk_spi5            :  1;
        unsigned int st_clk_spi6            :  1;
        unsigned int st_pclk_peri_wdt1      :  1;
        unsigned int st_pclk_peri_wdt0      :  1;
        unsigned int st_clk_peri_wdt0       :  1;
        unsigned int st_clk_peri_wdt1       :  1;
        unsigned int st_clk_perfmon         :  1;
        unsigned int st_pclk_perfmon_peri   :  1;
        unsigned int st_pclk_perfmon        :  1;
        unsigned int st_pclk_perfstat       :  1;
        unsigned int reserved_0             :  1;
    } reg;
}peri_crg_clkst0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_i2c0              :  1;
        unsigned int gt_pclk_i2c1              :  1;
        unsigned int gt_pclk_i2c2              :  1;
        unsigned int gt_pclk_i2c3              :  1;
        unsigned int gt_pclk_i2c4              :  1;
        unsigned int gt_pclk_i2c5              :  1;
        unsigned int gt_pclk_i2c6              :  1;
        unsigned int reserved_0                :  2;
        unsigned int gt_pclk_i2c9              :  1;
        unsigned int gt_pclk_i2c10             :  1;
        unsigned int gt_pclk_i2c11             :  1;
        unsigned int gt_pclk_i2c12             :  1;
        unsigned int gt_pclk_i2c13             :  1;
        unsigned int gt_pclk_i2c20             :  1;
        unsigned int gt_pclk_i3c0              :  1;
        unsigned int gt_pclk_pwm0_abrg         :  1;
        unsigned int gt_pclk_pwm1_abrg         :  1;
        unsigned int gt_pclk_uart3_abrg        :  1;
        unsigned int gt_pclk_uart6_abrg        :  1;
        unsigned int gt_clk_uart3              :  1;
        unsigned int gt_clk_uart6              :  1;
        unsigned int gt_clk_dma_ns             :  1;
        unsigned int gt_pclk_dma_ns            :  1;
        unsigned int gt_clk_dma_s              :  1;
        unsigned int gt_pclk_dma_s             :  1;
        unsigned int gt_clk_cfgbus_dsi_rs_core :  1;
        unsigned int gt_clk_cfgbus_csi_rs_core :  1;
        unsigned int gt_clk_cfgbus_csi_rs_peri :  1;
        unsigned int gt_clk_sysbus_csi_rs_core :  1;
        unsigned int gt_clk_sysbus_csi_rs_peri :  1;
        unsigned int reserved_1                :  1;
    } reg;
}peri_crg_clkgt1_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_i2c0              :  1;
        unsigned int gt_pclk_i2c1              :  1;
        unsigned int gt_pclk_i2c2              :  1;
        unsigned int gt_pclk_i2c3              :  1;
        unsigned int gt_pclk_i2c4              :  1;
        unsigned int gt_pclk_i2c5              :  1;
        unsigned int gt_pclk_i2c6              :  1;
        unsigned int reserved_0                :  2;
        unsigned int gt_pclk_i2c9              :  1;
        unsigned int gt_pclk_i2c10             :  1;
        unsigned int gt_pclk_i2c11             :  1;
        unsigned int gt_pclk_i2c12             :  1;
        unsigned int gt_pclk_i2c13             :  1;
        unsigned int gt_pclk_i2c20             :  1;
        unsigned int gt_pclk_i3c0              :  1;
        unsigned int gt_pclk_pwm0_abrg         :  1;
        unsigned int gt_pclk_pwm1_abrg         :  1;
        unsigned int gt_pclk_uart3_abrg        :  1;
        unsigned int gt_pclk_uart6_abrg        :  1;
        unsigned int gt_clk_uart3              :  1;
        unsigned int gt_clk_uart6              :  1;
        unsigned int gt_clk_dma_ns             :  1;
        unsigned int gt_pclk_dma_ns            :  1;
        unsigned int gt_clk_dma_s              :  1;
        unsigned int gt_pclk_dma_s             :  1;
        unsigned int gt_clk_cfgbus_dsi_rs_core :  1;
        unsigned int gt_clk_cfgbus_csi_rs_core :  1;
        unsigned int gt_clk_cfgbus_csi_rs_peri :  1;
        unsigned int gt_clk_sysbus_csi_rs_core :  1;
        unsigned int gt_clk_sysbus_csi_rs_peri :  1;
        unsigned int reserved_1                :  1;
    } reg;
}peri_crg_clkgt1_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_i2c0              :  1;
        unsigned int gt_pclk_i2c1              :  1;
        unsigned int gt_pclk_i2c2              :  1;
        unsigned int gt_pclk_i2c3              :  1;
        unsigned int gt_pclk_i2c4              :  1;
        unsigned int gt_pclk_i2c5              :  1;
        unsigned int gt_pclk_i2c6              :  1;
        unsigned int reserved_0                :  2;
        unsigned int gt_pclk_i2c9              :  1;
        unsigned int gt_pclk_i2c10             :  1;
        unsigned int gt_pclk_i2c11             :  1;
        unsigned int gt_pclk_i2c12             :  1;
        unsigned int gt_pclk_i2c13             :  1;
        unsigned int gt_pclk_i2c20             :  1;
        unsigned int gt_pclk_i3c0              :  1;
        unsigned int gt_pclk_pwm0_abrg         :  1;
        unsigned int gt_pclk_pwm1_abrg         :  1;
        unsigned int gt_pclk_uart3_abrg        :  1;
        unsigned int gt_pclk_uart6_abrg        :  1;
        unsigned int gt_clk_uart3              :  1;
        unsigned int gt_clk_uart6              :  1;
        unsigned int gt_clk_dma_ns             :  1;
        unsigned int gt_pclk_dma_ns            :  1;
        unsigned int gt_clk_dma_s              :  1;
        unsigned int gt_pclk_dma_s             :  1;
        unsigned int gt_clk_cfgbus_dsi_rs_core :  1;
        unsigned int gt_clk_cfgbus_csi_rs_core :  1;
        unsigned int gt_clk_cfgbus_csi_rs_peri :  1;
        unsigned int gt_clk_sysbus_csi_rs_core :  1;
        unsigned int gt_clk_sysbus_csi_rs_peri :  1;
        unsigned int reserved_1                :  1;
    } reg;
}peri_crg_clkgt1_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_pclk_i2c0              :  1;
        unsigned int st_pclk_i2c1              :  1;
        unsigned int st_pclk_i2c2              :  1;
        unsigned int st_pclk_i2c3              :  1;
        unsigned int st_pclk_i2c4              :  1;
        unsigned int st_pclk_i2c5              :  1;
        unsigned int st_pclk_i2c6              :  1;
        unsigned int reserved_0                :  2;
        unsigned int st_pclk_i2c9              :  1;
        unsigned int st_pclk_i2c10             :  1;
        unsigned int st_pclk_i2c11             :  1;
        unsigned int st_pclk_i2c12             :  1;
        unsigned int st_pclk_i2c13             :  1;
        unsigned int st_pclk_i2c20             :  1;
        unsigned int st_pclk_i3c0              :  1;
        unsigned int st_pclk_pwm0_abrg         :  1;
        unsigned int st_pclk_pwm1_abrg         :  1;
        unsigned int st_pclk_uart3_abrg        :  1;
        unsigned int st_pclk_uart6_abrg        :  1;
        unsigned int st_clk_uart3              :  1;
        unsigned int st_clk_uart6              :  1;
        unsigned int st_clk_dma_ns             :  1;
        unsigned int st_pclk_dma_ns            :  1;
        unsigned int st_clk_dma_s              :  1;
        unsigned int st_pclk_dma_s             :  1;
        unsigned int reserved_1                :  2;
        unsigned int st_clk_cfgbus_csi_rs_peri :  1;
        unsigned int reserved_2                :  1;
        unsigned int st_clk_sysbus_csi_rs_peri :  1;
        unsigned int reserved_3                :  1;
    } reg;
}peri_crg_clkst1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_i2c0         :  1;
        unsigned int gt_clk_i2c1         :  1;
        unsigned int gt_clk_i2c2         :  1;
        unsigned int gt_clk_i2c3         :  1;
        unsigned int gt_clk_i2c4         :  1;
        unsigned int gt_clk_i2c5         :  1;
        unsigned int gt_clk_i2c6         :  1;
        unsigned int reserved_0          :  2;
        unsigned int gt_clk_i2c9         :  1;
        unsigned int gt_clk_i2c10        :  1;
        unsigned int gt_clk_i2c11        :  1;
        unsigned int gt_clk_i2c12        :  1;
        unsigned int gt_clk_i2c13        :  1;
        unsigned int gt_clk_i2c20        :  1;
        unsigned int gt_clk_i3c0         :  1;
        unsigned int reserved_1          :  3;
        unsigned int gt_clk_pwm0         :  1;
        unsigned int gt_clk_pwm1         :  1;
        unsigned int gt_clk_ppll3_media1 :  1;
        unsigned int gt_clk_ppll2_media1 :  1;
        unsigned int gt_clk_ppll1_media1 :  1;
        unsigned int gt_clk_ppll0_media1 :  1;
        unsigned int gt_clk_gpll_media1  :  1;
        unsigned int gt_clk_ppll3_media2 :  1;
        unsigned int gt_clk_ppll2_media2 :  1;
        unsigned int gt_clk_ppll1_media2 :  1;
        unsigned int gt_clk_ppll0_media2 :  1;
        unsigned int gt_clk_gpll_media2  :  1;
        unsigned int reserved_2          :  1;
    } reg;
}peri_crg_clkgt2_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_i2c0         :  1;
        unsigned int gt_clk_i2c1         :  1;
        unsigned int gt_clk_i2c2         :  1;
        unsigned int gt_clk_i2c3         :  1;
        unsigned int gt_clk_i2c4         :  1;
        unsigned int gt_clk_i2c5         :  1;
        unsigned int gt_clk_i2c6         :  1;
        unsigned int reserved_0          :  2;
        unsigned int gt_clk_i2c9         :  1;
        unsigned int gt_clk_i2c10        :  1;
        unsigned int gt_clk_i2c11        :  1;
        unsigned int gt_clk_i2c12        :  1;
        unsigned int gt_clk_i2c13        :  1;
        unsigned int gt_clk_i2c20        :  1;
        unsigned int gt_clk_i3c0         :  1;
        unsigned int reserved_1          :  3;
        unsigned int gt_clk_pwm0         :  1;
        unsigned int gt_clk_pwm1         :  1;
        unsigned int gt_clk_ppll3_media1 :  1;
        unsigned int gt_clk_ppll2_media1 :  1;
        unsigned int gt_clk_ppll1_media1 :  1;
        unsigned int gt_clk_ppll0_media1 :  1;
        unsigned int gt_clk_gpll_media1  :  1;
        unsigned int gt_clk_ppll3_media2 :  1;
        unsigned int gt_clk_ppll2_media2 :  1;
        unsigned int gt_clk_ppll1_media2 :  1;
        unsigned int gt_clk_ppll0_media2 :  1;
        unsigned int gt_clk_gpll_media2  :  1;
        unsigned int reserved_2          :  1;
    } reg;
}peri_crg_clkgt2_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_i2c0         :  1;
        unsigned int gt_clk_i2c1         :  1;
        unsigned int gt_clk_i2c2         :  1;
        unsigned int gt_clk_i2c3         :  1;
        unsigned int gt_clk_i2c4         :  1;
        unsigned int gt_clk_i2c5         :  1;
        unsigned int gt_clk_i2c6         :  1;
        unsigned int reserved_0          :  2;
        unsigned int gt_clk_i2c9         :  1;
        unsigned int gt_clk_i2c10        :  1;
        unsigned int gt_clk_i2c11        :  1;
        unsigned int gt_clk_i2c12        :  1;
        unsigned int gt_clk_i2c13        :  1;
        unsigned int gt_clk_i2c20        :  1;
        unsigned int gt_clk_i3c0         :  1;
        unsigned int reserved_1          :  3;
        unsigned int gt_clk_pwm0         :  1;
        unsigned int gt_clk_pwm1         :  1;
        unsigned int gt_clk_ppll3_media1 :  1;
        unsigned int gt_clk_ppll2_media1 :  1;
        unsigned int gt_clk_ppll1_media1 :  1;
        unsigned int gt_clk_ppll0_media1 :  1;
        unsigned int gt_clk_gpll_media1  :  1;
        unsigned int gt_clk_ppll3_media2 :  1;
        unsigned int gt_clk_ppll2_media2 :  1;
        unsigned int gt_clk_ppll1_media2 :  1;
        unsigned int gt_clk_ppll0_media2 :  1;
        unsigned int gt_clk_gpll_media2  :  1;
        unsigned int reserved_2          :  1;
    } reg;
}peri_crg_clkgt2_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_i2c0         :  1;
        unsigned int st_clk_i2c1         :  1;
        unsigned int st_clk_i2c2         :  1;
        unsigned int st_clk_i2c3         :  1;
        unsigned int st_clk_i2c4         :  1;
        unsigned int st_clk_i2c5         :  1;
        unsigned int st_clk_i2c6         :  1;
        unsigned int reserved_0          :  2;
        unsigned int st_clk_i2c9         :  1;
        unsigned int st_clk_i2c10        :  1;
        unsigned int st_clk_i2c11        :  1;
        unsigned int st_clk_i2c12        :  1;
        unsigned int st_clk_i2c13        :  1;
        unsigned int st_clk_i2c20        :  1;
        unsigned int st_clk_i3c0         :  1;
        unsigned int reserved_1          :  3;
        unsigned int st_clk_pwm0         :  1;
        unsigned int st_clk_pwm1         :  1;
        unsigned int st_clk_ppll3_media1 :  1;
        unsigned int st_clk_ppll2_media1 :  1;
        unsigned int st_clk_ppll1_media1 :  1;
        unsigned int st_clk_ppll0_media1 :  1;
        unsigned int st_clk_gpll_media1  :  1;
        unsigned int st_clk_ppll3_media2 :  1;
        unsigned int st_clk_ppll2_media2 :  1;
        unsigned int st_clk_ppll1_media2 :  1;
        unsigned int st_clk_ppll0_media2 :  1;
        unsigned int st_clk_gpll_media2  :  1;
        unsigned int reserved_2          :  1;
    } reg;
}peri_crg_clkst2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_mainbus_data_ddr_w_rs_core :  1;
        unsigned int gt_clk_mainbus_data_ddr_w_rs_peri :  1;
        unsigned int gt_clk_mainbus_data_ddr_r_rs_peri :  1;
        unsigned int gt_clk_sysbus_data_ddr_rs_core    :  1;
        unsigned int gt_clk_sysbus_data_ddr_rs_peri    :  1;
        unsigned int gt_clk_debug_atb_rs_core          :  1;
        unsigned int gt_clk_debug_atb_rs_peri          :  1;
        unsigned int reserved_0                        :  5;
        unsigned int gt_clk_cfgbus_gpu_rs_peri         :  1;
        unsigned int gt_clk_mainbus_data_gpu_rs_peri   :  1;
        unsigned int gt_clk_sysbus_hss1_rs_core        :  1;
        unsigned int gt_clk_sysbus_hss1_rs_peri        :  1;
        unsigned int gt_clk_cfgbus_hss1_rs_core        :  1;
        unsigned int gt_clk_sysbus_hss2_rs_core        :  1;
        unsigned int gt_clk_sysbus_hss2_rs_peri        :  1;
        unsigned int gt_clk_cfgbus_hss2_rs_core        :  1;
        unsigned int gt_clk_cfgbus_hss2_rs_peri        :  1;
        unsigned int reserved_1                        : 11;
    } reg;
}peri_crg_clkgt12_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_mainbus_data_ddr_w_rs_core :  1;
        unsigned int gt_clk_mainbus_data_ddr_w_rs_peri :  1;
        unsigned int gt_clk_mainbus_data_ddr_r_rs_peri :  1;
        unsigned int gt_clk_sysbus_data_ddr_rs_core    :  1;
        unsigned int gt_clk_sysbus_data_ddr_rs_peri    :  1;
        unsigned int gt_clk_debug_atb_rs_core          :  1;
        unsigned int gt_clk_debug_atb_rs_peri          :  1;
        unsigned int reserved_0                        :  5;
        unsigned int gt_clk_cfgbus_gpu_rs_peri         :  1;
        unsigned int gt_clk_mainbus_data_gpu_rs_peri   :  1;
        unsigned int gt_clk_sysbus_hss1_rs_core        :  1;
        unsigned int gt_clk_sysbus_hss1_rs_peri        :  1;
        unsigned int gt_clk_cfgbus_hss1_rs_core        :  1;
        unsigned int gt_clk_sysbus_hss2_rs_core        :  1;
        unsigned int gt_clk_sysbus_hss2_rs_peri        :  1;
        unsigned int gt_clk_cfgbus_hss2_rs_core        :  1;
        unsigned int gt_clk_cfgbus_hss2_rs_peri        :  1;
        unsigned int reserved_1                        : 11;
    } reg;
}peri_crg_clkgt12_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_mainbus_data_ddr_w_rs_core :  1;
        unsigned int gt_clk_mainbus_data_ddr_w_rs_peri :  1;
        unsigned int gt_clk_mainbus_data_ddr_r_rs_peri :  1;
        unsigned int gt_clk_sysbus_data_ddr_rs_core    :  1;
        unsigned int gt_clk_sysbus_data_ddr_rs_peri    :  1;
        unsigned int gt_clk_debug_atb_rs_core          :  1;
        unsigned int gt_clk_debug_atb_rs_peri          :  1;
        unsigned int reserved_0                        :  5;
        unsigned int gt_clk_cfgbus_gpu_rs_peri         :  1;
        unsigned int gt_clk_mainbus_data_gpu_rs_peri   :  1;
        unsigned int gt_clk_sysbus_hss1_rs_core        :  1;
        unsigned int gt_clk_sysbus_hss1_rs_peri        :  1;
        unsigned int gt_clk_cfgbus_hss1_rs_core        :  1;
        unsigned int gt_clk_sysbus_hss2_rs_core        :  1;
        unsigned int gt_clk_sysbus_hss2_rs_peri        :  1;
        unsigned int gt_clk_cfgbus_hss2_rs_core        :  1;
        unsigned int gt_clk_cfgbus_hss2_rs_peri        :  1;
        unsigned int reserved_1                        : 11;
    } reg;
}peri_crg_clkgt12_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_mainbus_data_ddra_w_rs_peri    :  1;
        unsigned int st_clk_mainbus_data_ddra_r_rs_peri    :  1;
        unsigned int st_clk_sysbus_data_ddra_rs_peri       :  1;
        unsigned int st_clk_mainbus_data_ddrb_w_rs_peri    :  1;
        unsigned int st_clk_mainbus_data_ddrb_r_rs_peri    :  1;
        unsigned int st_clk_sysbus_data_ddrb_rs_peri       :  1;
        unsigned int st_clk_mainbus_data_ddrc_w_rs_peri    :  1;
        unsigned int st_clk_mainbus_data_ddrc_r_rs_peri    :  1;
        unsigned int st_clk_sysbus_data_ddrc_rs_peri       :  1;
        unsigned int st_clk_mainbus_data_ddrd_w_rs_peri    :  1;
        unsigned int st_clk_mainbus_data_ddrd_r_rs_peri    :  1;
        unsigned int st_clk_sysbus_data_ddrd_rs_peri       :  1;
        unsigned int reserved_0                            :  2;
        unsigned int st_clk_cfgbus_mreg2gpu_rs_peri        :  1;
        unsigned int st_clk_cfgbus_gpu2maincfg_rs_peri     :  1;
        unsigned int st_clk_cfgbus_maincfg2gpu_rs_peri     :  1;
        unsigned int st_clk_cfgbus_cfgbus2gpu_axi_rs_peri  :  1;
        unsigned int st_clk_cfgbus_cfgbus2gpu_rs_peri      :  1;
        unsigned int st_clk_mainbus_data_main2gpu_rs0_peri :  1;
        unsigned int st_clk_mainbus_data_gpu2main_rs0_peri :  1;
        unsigned int st_clk_sysbus_hss1_rs_peri            :  1;
        unsigned int st_clk_sysbus_hss22sysbus_rs_peri     :  1;
        unsigned int st_clk_cfgbus_hss22cfgbus_rs_peri     :  1;
        unsigned int st_clk_cfgbus_cfgbus2hss2_rs_peri     :  1;
        unsigned int st_clk_mainbus_data_main2gpu_rs1_peri :  1;
        unsigned int st_clk_mainbus_data_gpu2main_rs1_peri :  1;
        unsigned int reserved_1                            :  5;
    } reg;
}peri_crg_clkst12_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_mainbus_data_media1_rs_core :  1;
        unsigned int gt_clk_mainbus_data_media1_rs_peri :  1;
        unsigned int gt_clk_cfgbus_media1_rs_core       :  1;
        unsigned int gt_clk_cfgbus_media1_rs_peri       :  1;
        unsigned int gt_clk_sysbus_data_media1_rs_core  :  1;
        unsigned int gt_clk_mainbus_data_media2_rs_core :  1;
        unsigned int gt_clk_mainbus_data_media2_rs_peri :  1;
        unsigned int gt_clk_cfgbus_media2_rs_core       :  1;
        unsigned int gt_clk_cfgbus_media2_rs_peri       :  1;
        unsigned int gt_clk_sysbus_data_media2_rs_core  :  1;
        unsigned int gt_clk_mainbus_data_npu_rs_core    :  1;
        unsigned int gt_clk_mainbus_data_npu_rs_peri    :  1;
        unsigned int gt_clk_cfgbus_npu_rs_core          :  1;
        unsigned int gt_clk_cfgbus_npu_rs_peri          :  1;
        unsigned int gt_clk_sysbus_xrse_rs_peri         :  1;
        unsigned int gt_clk_cfgbus_xrse_rs_peri         :  1;
        unsigned int gt_clk_mainbus_data_cpu_rs_core    :  1;
        unsigned int gt_clk_mainbus_data_cpu_rs_peri    :  1;
        unsigned int gt_clk_cfgbus_cpu_rs_core          :  1;
        unsigned int gt_clk_cfgbus_cpu_rs_peri          :  1;
        unsigned int gt_clk_sysbus_cpu_rs_core          :  1;
        unsigned int gt_clk_sysbus_cpu_rs_peri          :  1;
        unsigned int reserved_0                         : 10;
    } reg;
}peri_crg_clkgt13_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_mainbus_data_media1_rs_core :  1;
        unsigned int gt_clk_mainbus_data_media1_rs_peri :  1;
        unsigned int gt_clk_cfgbus_media1_rs_core       :  1;
        unsigned int gt_clk_cfgbus_media1_rs_peri       :  1;
        unsigned int gt_clk_sysbus_data_media1_rs_core  :  1;
        unsigned int gt_clk_mainbus_data_media2_rs_core :  1;
        unsigned int gt_clk_mainbus_data_media2_rs_peri :  1;
        unsigned int gt_clk_cfgbus_media2_rs_core       :  1;
        unsigned int gt_clk_cfgbus_media2_rs_peri       :  1;
        unsigned int gt_clk_sysbus_data_media2_rs_core  :  1;
        unsigned int gt_clk_mainbus_data_npu_rs_core    :  1;
        unsigned int gt_clk_mainbus_data_npu_rs_peri    :  1;
        unsigned int gt_clk_cfgbus_npu_rs_core          :  1;
        unsigned int gt_clk_cfgbus_npu_rs_peri          :  1;
        unsigned int gt_clk_sysbus_xrse_rs_peri         :  1;
        unsigned int gt_clk_cfgbus_xrse_rs_peri         :  1;
        unsigned int gt_clk_mainbus_data_cpu_rs_core    :  1;
        unsigned int gt_clk_mainbus_data_cpu_rs_peri    :  1;
        unsigned int gt_clk_cfgbus_cpu_rs_core          :  1;
        unsigned int gt_clk_cfgbus_cpu_rs_peri          :  1;
        unsigned int gt_clk_sysbus_cpu_rs_core          :  1;
        unsigned int gt_clk_sysbus_cpu_rs_peri          :  1;
        unsigned int reserved_0                         : 10;
    } reg;
}peri_crg_clkgt13_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_mainbus_data_media1_rs_core :  1;
        unsigned int gt_clk_mainbus_data_media1_rs_peri :  1;
        unsigned int gt_clk_cfgbus_media1_rs_core       :  1;
        unsigned int gt_clk_cfgbus_media1_rs_peri       :  1;
        unsigned int gt_clk_sysbus_data_media1_rs_core  :  1;
        unsigned int gt_clk_mainbus_data_media2_rs_core :  1;
        unsigned int gt_clk_mainbus_data_media2_rs_peri :  1;
        unsigned int gt_clk_cfgbus_media2_rs_core       :  1;
        unsigned int gt_clk_cfgbus_media2_rs_peri       :  1;
        unsigned int gt_clk_sysbus_data_media2_rs_core  :  1;
        unsigned int gt_clk_mainbus_data_npu_rs_core    :  1;
        unsigned int gt_clk_mainbus_data_npu_rs_peri    :  1;
        unsigned int gt_clk_cfgbus_npu_rs_core          :  1;
        unsigned int gt_clk_cfgbus_npu_rs_peri          :  1;
        unsigned int gt_clk_sysbus_xrse_rs_peri         :  1;
        unsigned int gt_clk_cfgbus_xrse_rs_peri         :  1;
        unsigned int gt_clk_mainbus_data_cpu_rs_core    :  1;
        unsigned int gt_clk_mainbus_data_cpu_rs_peri    :  1;
        unsigned int gt_clk_cfgbus_cpu_rs_core          :  1;
        unsigned int gt_clk_cfgbus_cpu_rs_peri          :  1;
        unsigned int gt_clk_sysbus_cpu_rs_core          :  1;
        unsigned int gt_clk_sysbus_cpu_rs_peri          :  1;
        unsigned int reserved_0                         : 10;
    } reg;
}peri_crg_clkgt13_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_cfgbus_cfgbus2media1_rs_peri        :  1;
        unsigned int st_clk_mainbus_data_media1_p0_rs_peri      :  1;
        unsigned int st_clk_mainbus_data_media1_p1_rs_peri      :  1;
        unsigned int st_clk_cfgbus_cfgbus2media2_rs_peri        :  1;
        unsigned int st_clk_mainbus_data_media2_p0_rs_peri      :  1;
        unsigned int st_clk_mainbus_data_media2_p1_rs_peri      :  1;
        unsigned int st_clk_cfgbus_cfgbus2media2_mfw_rs_peri    :  1;
        unsigned int st_clk_cfgbus_npu_mfw_rs_peri              :  1;
        unsigned int st_clk_mainbus_data_npu_p0_rs_peri         :  1;
        unsigned int st_clk_cfgbus_npu2cfgbus_rs_peri           :  1;
        unsigned int st_clk_cfgbus_xrse_rs_peri                 :  1;
        unsigned int st_clk_sysbus_xrse_rs_peri                 :  1;
        unsigned int st_clk_cfgbus_xrse_mfw_rs_peri             :  1;
        unsigned int st_clk_cfgbus_cpu_mfw_rs_peri              :  1;
        unsigned int st_clk_cfgbus_cpu2cfgbus_rs_peri           :  1;
        unsigned int st_clk_cfgbus_maincfg2cpu_rs_peri          :  1;
        unsigned int st_clk_cfgbus_cpu2maincfg_rs_peri          :  1;
        unsigned int st_clk_mainbus_data_mainbus2cpu_rs_peri    :  1;
        unsigned int st_clk_mainbus_data_cpu2mainbus_rs_peri    :  1;
        unsigned int st_clk_sysbus_cpu_rs_peri                  :  1;
        unsigned int st_clk_cfgbus_mreg2cpu_rs_peri             :  1;
        unsigned int st_clk_mainbus_data_npu_p1_rs_peri         :  1;
        unsigned int st_clk_mainbus_data_npu_p2_rs_peri         :  1;
        unsigned int st_clk_mainbus_data_npu_p3_rs_peri         :  1;
        unsigned int st_clk_mainbus_data_media1_mreg_p0_rs_peri :  1;
        unsigned int st_clk_mainbus_data_media1_mreg_p1_rs_peri :  1;
        unsigned int st_clk_mainbus_data_media2_mreg_p0_rs_peri :  1;
        unsigned int st_clk_mainbus_data_media2_mreg_p1_rs_peri :  1;
        unsigned int st_clk_mainbus_data_npu_mreg_p0_rs_peri    :  1;
        unsigned int st_clk_mainbus_data_npu_mreg_p1_rs_peri    :  1;
        unsigned int st_clk_mainbus_data_npu_mreg_p2_rs_peri    :  1;
        unsigned int st_clk_mainbus_data_npu_mreg_p3_rs_peri    :  1;
    } reg;
}peri_crg_clkst13_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_debug_atb_ddra_rs_peri     :  1;
        unsigned int st_clk_debug_atb_ddrb_rs_peri     :  1;
        unsigned int st_clk_debug_atb_ddrc_rs_peri     :  1;
        unsigned int st_clk_debug_atb_ddrd_rs_peri     :  1;
        unsigned int st_clk_debug_atb_gpu_rs_peri      :  1;
        unsigned int st_clk_debug_atb_media2_rs_peri   :  1;
        unsigned int st_clk_debug_atb_npu_rs_peri      :  1;
        unsigned int st_clk_debug_atb_npu_ela_rs_peri  :  1;
        unsigned int st_clk_debug_atb_xrse_rs_peri     :  1;
        unsigned int st_clk_debug_atb_cpu_rs_peri      :  1;
        unsigned int st_clk_debug_atb_lpis_rs_peri     :  1;
        unsigned int st_clk_debug_atb_xrse_ela_rs_peri :  1;
        unsigned int reserved_0                        : 20;
    } reg;
}peri_crg_clkst14_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_prst_peri_spinlock_n :  1;
        unsigned int ip_prst_peri_timer_ns_n :  1;
        unsigned int ip_rst_ocm_n            :  1;
        unsigned int reserved_0              :  3;
        unsigned int ip_rst_peri_timer0_n    :  1;
        unsigned int ip_rst_peri_timer1_n    :  1;
        unsigned int ip_rst_peri_timer2_n    :  1;
        unsigned int ip_rst_peri_timer3_n    :  1;
        unsigned int ip_rst_csi_n            :  1;
        unsigned int ip_rst_csi_crg_n        :  1;
        unsigned int ip_rst_dsi_n            :  1;
        unsigned int ip_rst_dsi_crg_n        :  1;
        unsigned int reserved_1              :  1;
        unsigned int ip_prst_peri_ipc1_n     :  1;
        unsigned int ip_prst_peri_ipc2_n     :  1;
        unsigned int ip_prst_spi4_n          :  1;
        unsigned int ip_prst_spi5_n          :  1;
        unsigned int ip_prst_spi6_n          :  1;
        unsigned int ip_rst_spi4_n           :  1;
        unsigned int ip_rst_spi5_n           :  1;
        unsigned int ip_rst_spi6_n           :  1;
        unsigned int ip_prst_peri_wdt0_n     :  1;
        unsigned int ip_prst_peri_wdt1_n     :  1;
        unsigned int ip_rst_peri_wdt0_n      :  1;
        unsigned int ip_rst_peri_wdt1_n      :  1;
        unsigned int reserved_2              :  5;
    } reg;
}peri_crg_rst0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_prst_peri_spinlock_n :  1;
        unsigned int ip_prst_peri_timer_ns_n :  1;
        unsigned int ip_rst_ocm_n            :  1;
        unsigned int reserved_0              :  3;
        unsigned int ip_rst_peri_timer0_n    :  1;
        unsigned int ip_rst_peri_timer1_n    :  1;
        unsigned int ip_rst_peri_timer2_n    :  1;
        unsigned int ip_rst_peri_timer3_n    :  1;
        unsigned int ip_rst_csi_n            :  1;
        unsigned int ip_rst_csi_crg_n        :  1;
        unsigned int ip_rst_dsi_n            :  1;
        unsigned int ip_rst_dsi_crg_n        :  1;
        unsigned int reserved_1              :  1;
        unsigned int ip_prst_peri_ipc1_n     :  1;
        unsigned int ip_prst_peri_ipc2_n     :  1;
        unsigned int ip_prst_spi4_n          :  1;
        unsigned int ip_prst_spi5_n          :  1;
        unsigned int ip_prst_spi6_n          :  1;
        unsigned int ip_rst_spi4_n           :  1;
        unsigned int ip_rst_spi5_n           :  1;
        unsigned int ip_rst_spi6_n           :  1;
        unsigned int ip_prst_peri_wdt0_n     :  1;
        unsigned int ip_prst_peri_wdt1_n     :  1;
        unsigned int ip_rst_peri_wdt0_n      :  1;
        unsigned int ip_rst_peri_wdt1_n      :  1;
        unsigned int reserved_2              :  5;
    } reg;
}peri_crg_rst0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_prst_peri_spinlock_n :  1;
        unsigned int ip_prst_peri_timer_ns_n :  1;
        unsigned int ip_rst_ocm_n            :  1;
        unsigned int reserved_0              :  3;
        unsigned int ip_rst_peri_timer0_n    :  1;
        unsigned int ip_rst_peri_timer1_n    :  1;
        unsigned int ip_rst_peri_timer2_n    :  1;
        unsigned int ip_rst_peri_timer3_n    :  1;
        unsigned int ip_rst_csi_n            :  1;
        unsigned int ip_rst_csi_crg_n        :  1;
        unsigned int ip_rst_dsi_n            :  1;
        unsigned int ip_rst_dsi_crg_n        :  1;
        unsigned int reserved_1              :  1;
        unsigned int ip_prst_peri_ipc1_n     :  1;
        unsigned int ip_prst_peri_ipc2_n     :  1;
        unsigned int ip_prst_spi4_n          :  1;
        unsigned int ip_prst_spi5_n          :  1;
        unsigned int ip_prst_spi6_n          :  1;
        unsigned int ip_rst_spi4_n           :  1;
        unsigned int ip_rst_spi5_n           :  1;
        unsigned int ip_rst_spi6_n           :  1;
        unsigned int ip_prst_peri_wdt0_n     :  1;
        unsigned int ip_prst_peri_wdt1_n     :  1;
        unsigned int ip_rst_peri_wdt0_n      :  1;
        unsigned int ip_rst_peri_wdt1_n      :  1;
        unsigned int reserved_2              :  5;
    } reg;
}peri_crg_rst0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_prst_i2c0_n      :  1;
        unsigned int ip_prst_i2c1_n      :  1;
        unsigned int ip_prst_i2c2_n      :  1;
        unsigned int ip_prst_i2c3_n      :  1;
        unsigned int ip_prst_i2c4_n      :  1;
        unsigned int ip_prst_i2c5_n      :  1;
        unsigned int ip_prst_i2c6_n      :  1;
        unsigned int reserved_0          :  2;
        unsigned int ip_prst_i2c9_n      :  1;
        unsigned int ip_prst_i2c10_n     :  1;
        unsigned int ip_prst_i2c11_n     :  1;
        unsigned int ip_prst_i2c12_n     :  1;
        unsigned int ip_prst_i2c13_n     :  1;
        unsigned int ip_prst_i2c20_n     :  1;
        unsigned int reserved_1          :  1;
        unsigned int ip_rst_dma_ns_n     :  1;
        unsigned int ip_rst_dma_s_n      :  1;
        unsigned int ip_rst_dma_ns_tpc_n :  1;
        unsigned int ip_rst_dma_s_tpc_n  :  1;
        unsigned int ip_rst_perfmon_n    :  1;
        unsigned int ip_rst_perfstat_n   :  1;
        unsigned int reserved_2          : 10;
    } reg;
}peri_crg_rst1_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_prst_i2c0_n      :  1;
        unsigned int ip_prst_i2c1_n      :  1;
        unsigned int ip_prst_i2c2_n      :  1;
        unsigned int ip_prst_i2c3_n      :  1;
        unsigned int ip_prst_i2c4_n      :  1;
        unsigned int ip_prst_i2c5_n      :  1;
        unsigned int ip_prst_i2c6_n      :  1;
        unsigned int reserved_0          :  2;
        unsigned int ip_prst_i2c9_n      :  1;
        unsigned int ip_prst_i2c10_n     :  1;
        unsigned int ip_prst_i2c11_n     :  1;
        unsigned int ip_prst_i2c12_n     :  1;
        unsigned int ip_prst_i2c13_n     :  1;
        unsigned int ip_prst_i2c20_n     :  1;
        unsigned int reserved_1          :  1;
        unsigned int ip_rst_dma_ns_n     :  1;
        unsigned int ip_rst_dma_s_n      :  1;
        unsigned int ip_rst_dma_ns_tpc_n :  1;
        unsigned int ip_rst_dma_s_tpc_n  :  1;
        unsigned int ip_rst_perfmon_n    :  1;
        unsigned int ip_rst_perfstat_n   :  1;
        unsigned int reserved_2          : 10;
    } reg;
}peri_crg_rst1_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_prst_i2c0_n      :  1;
        unsigned int ip_prst_i2c1_n      :  1;
        unsigned int ip_prst_i2c2_n      :  1;
        unsigned int ip_prst_i2c3_n      :  1;
        unsigned int ip_prst_i2c4_n      :  1;
        unsigned int ip_prst_i2c5_n      :  1;
        unsigned int ip_prst_i2c6_n      :  1;
        unsigned int reserved_0          :  2;
        unsigned int ip_prst_i2c9_n      :  1;
        unsigned int ip_prst_i2c10_n     :  1;
        unsigned int ip_prst_i2c11_n     :  1;
        unsigned int ip_prst_i2c12_n     :  1;
        unsigned int ip_prst_i2c13_n     :  1;
        unsigned int ip_prst_i2c20_n     :  1;
        unsigned int reserved_1          :  1;
        unsigned int ip_rst_dma_ns_n     :  1;
        unsigned int ip_rst_dma_s_n      :  1;
        unsigned int ip_rst_dma_ns_tpc_n :  1;
        unsigned int ip_rst_dma_s_tpc_n  :  1;
        unsigned int ip_rst_perfmon_n    :  1;
        unsigned int ip_rst_perfstat_n   :  1;
        unsigned int reserved_2          : 10;
    } reg;
}peri_crg_rst1_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_i2c0_n  :  1;
        unsigned int ip_rst_i2c1_n  :  1;
        unsigned int ip_rst_i2c2_n  :  1;
        unsigned int ip_rst_i2c3_n  :  1;
        unsigned int ip_rst_i2c4_n  :  1;
        unsigned int ip_rst_i2c5_n  :  1;
        unsigned int ip_rst_i2c6_n  :  1;
        unsigned int reserved_0     :  2;
        unsigned int ip_rst_i2c9_n  :  1;
        unsigned int ip_rst_i2c10_n :  1;
        unsigned int ip_rst_i2c11_n :  1;
        unsigned int ip_rst_i2c12_n :  1;
        unsigned int ip_rst_i2c13_n :  1;
        unsigned int ip_rst_i2c20_n :  1;
        unsigned int ip_rst_i3c0_n  :  1;
        unsigned int ip_prst_i3c0_n :  1;
        unsigned int reserved_1     :  2;
        unsigned int ip_rst_pwm0_n  :  1;
        unsigned int ip_rst_pwm1_n  :  1;
        unsigned int ip_rst_uart3_n :  1;
        unsigned int ip_rst_uart6_n :  1;
        unsigned int reserved_2     :  9;
    } reg;
}peri_crg_rst2_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_i2c0_n  :  1;
        unsigned int ip_rst_i2c1_n  :  1;
        unsigned int ip_rst_i2c2_n  :  1;
        unsigned int ip_rst_i2c3_n  :  1;
        unsigned int ip_rst_i2c4_n  :  1;
        unsigned int ip_rst_i2c5_n  :  1;
        unsigned int ip_rst_i2c6_n  :  1;
        unsigned int reserved_0     :  2;
        unsigned int ip_rst_i2c9_n  :  1;
        unsigned int ip_rst_i2c10_n :  1;
        unsigned int ip_rst_i2c11_n :  1;
        unsigned int ip_rst_i2c12_n :  1;
        unsigned int ip_rst_i2c13_n :  1;
        unsigned int ip_rst_i2c20_n :  1;
        unsigned int ip_rst_i3c0_n  :  1;
        unsigned int ip_prst_i3c0_n :  1;
        unsigned int reserved_1     :  2;
        unsigned int ip_rst_pwm0_n  :  1;
        unsigned int ip_rst_pwm1_n  :  1;
        unsigned int ip_rst_uart3_n :  1;
        unsigned int ip_rst_uart6_n :  1;
        unsigned int reserved_2     :  9;
    } reg;
}peri_crg_rst2_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_i2c0_n  :  1;
        unsigned int ip_rst_i2c1_n  :  1;
        unsigned int ip_rst_i2c2_n  :  1;
        unsigned int ip_rst_i2c3_n  :  1;
        unsigned int ip_rst_i2c4_n  :  1;
        unsigned int ip_rst_i2c5_n  :  1;
        unsigned int ip_rst_i2c6_n  :  1;
        unsigned int reserved_0     :  2;
        unsigned int ip_rst_i2c9_n  :  1;
        unsigned int ip_rst_i2c10_n :  1;
        unsigned int ip_rst_i2c11_n :  1;
        unsigned int ip_rst_i2c12_n :  1;
        unsigned int ip_rst_i2c13_n :  1;
        unsigned int ip_rst_i2c20_n :  1;
        unsigned int ip_rst_i3c0_n  :  1;
        unsigned int ip_prst_i3c0_n :  1;
        unsigned int reserved_1     :  2;
        unsigned int ip_rst_pwm0_n  :  1;
        unsigned int ip_rst_pwm1_n  :  1;
        unsigned int ip_rst_uart3_n :  1;
        unsigned int ip_rst_uart6_n :  1;
        unsigned int reserved_2     :  9;
    } reg;
}peri_crg_rst2_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0        :  6;
        unsigned int div_clk_timer_h   :  6;
        unsigned int reserved_1        :  1;
        unsigned int sc_gt_clk_timer_h :  1;
        unsigned int reserved_2        :  2;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_clkdiv0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_spi4   :  6;
        unsigned int div_clk_spi5   :  6;
        unsigned int sc_gt_clk_spi4 :  1;
        unsigned int sc_gt_clk_spi5 :  1;
        unsigned int reserved_0     :  2;
        unsigned int _bm_           : 16;
    } reg;
}peri_crg_clkdiv1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_spi6   :  6;
        unsigned int reserved_0     :  6;
        unsigned int sc_gt_clk_spi6 :  1;
        unsigned int reserved_1     :  3;
        unsigned int _bm_           : 16;
    } reg;
}peri_crg_clkdiv2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_uart          :  6;
        unsigned int div_clk_pwm           :  6;
        unsigned int sc_gt_clk_uart        :  1;
        unsigned int sc_gt_clk_pwm         :  1;
        unsigned int sc_gt_pclk_media1_cfg :  1;
        unsigned int sc_gt_pclk_media2_cfg :  1;
        unsigned int _bm_                  : 16;
    } reg;
}peri_crg_clkdiv3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0    :  6;
        unsigned int div_clk_i2c   :  6;
        unsigned int reserved_1    :  1;
        unsigned int sc_gt_clk_i2c :  1;
        unsigned int reserved_2    :  2;
        unsigned int _bm_          : 16;
    } reg;
}peri_crg_clkdiv12_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0    :  6;
        unsigned int div_clk_i3c   :  6;
        unsigned int reserved_1    :  1;
        unsigned int sc_gt_clk_i3c :  1;
        unsigned int reserved_2    :  2;
        unsigned int _bm_          : 16;
    } reg;
}peri_crg_clkdiv15_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_ocm_gpll    :  6;
        unsigned int div_clk_ocm_lppll   :  6;
        unsigned int sc_gt_clk_ocm_gpll  :  1;
        unsigned int sc_gt_clk_ocm_lppll :  1;
        unsigned int reserved_0          :  2;
        unsigned int _bm_                : 16;
    } reg;
}peri_crg_clkdiv28_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_pcie_aux   :  6;
        unsigned int reserved_0         :  6;
        unsigned int sc_gt_clk_pcie_aux :  1;
        unsigned int reserved_1         :  3;
        unsigned int _bm_               : 16;
    } reg;
}peri_crg_clkdiv23_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0        :  6;
        unsigned int div_clk_csi_sys   :  6;
        unsigned int reserved_1        :  1;
        unsigned int sc_gt_clk_csi_sys :  1;
        unsigned int reserved_2        :  2;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_clkdiv14_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_dsi_sys   :  6;
        unsigned int reserved_0        :  6;
        unsigned int sc_gt_clk_dsi_sys :  1;
        unsigned int reserved_1        :  3;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_clkdiv16_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dpu_idle_for_ocm_bypass                    :  1;
        unsigned int npu_bus_tgt_no_pending_trans_ocm_bypass    :  1;
        unsigned int media2_bus_tgt_no_pending_trans_ocm_bypass :  1;
        unsigned int media1_bus_tgt_no_pending_trans_ocm_bypass :  1;
        unsigned int idle_flag_ocm_nic_bypass                   :  1;
        unsigned int reserved_0                                 : 27;
    } reg;
}peri_crg_autofsctrl17_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_time_ocm  : 10;
        unsigned int debounce_out_time_ocm : 10;
        unsigned int debounce_bypass_ocm   :  1;
        unsigned int reserved_0            : 10;
        unsigned int autofs_en_ocm         :  1;
    } reg;
}peri_crg_autofsctrl18_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_auto_clk_ocm_gpll  :  6;
        unsigned int div_auto_clk_ocm_lppll :  6;
        unsigned int reserved_0             : 20;
    } reg;
}peri_crg_autofsctrl19_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_done_clk_timer_h :  1;
        unsigned int div_done_clk_spi4    :  1;
        unsigned int div_done_clk_spi5    :  1;
        unsigned int div_done_clk_spi6    :  1;
        unsigned int div_done_clk_uart    :  1;
        unsigned int div_done_clk_pwm     :  1;
        unsigned int div_done_clk_i2c     :  1;
        unsigned int div_done_clk_i3c     :  1;
        unsigned int reserved_0           : 24;
    } reg;
}peri_crg_peristat0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_done_clk_mainbus_data_lppll_div :  1;
        unsigned int div_done_clk_mainbus_data_gpll_div  :  1;
        unsigned int div_done_clk_sysbus_data_lppll_div  :  1;
        unsigned int reserved_0                          :  1;
        unsigned int div_done_clk_sysbus_data_gpll       :  1;
        unsigned int div_done_clk_cfgbus_gpll            :  1;
        unsigned int div_done_clk_pericfg_bus            :  1;
        unsigned int div_done_clk_cfgbus_lppll           :  1;
        unsigned int div_done_clk_spi0                   :  1;
        unsigned int div_done_clk_spi1                   :  1;
        unsigned int div_done_clk_spi2                   :  1;
        unsigned int reserved_1                          :  7;
        unsigned int div_done_clk_dma_bus_data_lppll_div :  1;
        unsigned int div_done_clk_dma_bus_data_gpll_div  :  1;
        unsigned int div_done_pclk_dma_bus_gpll_div      :  1;
        unsigned int div_done_clk_csi_sys                :  1;
        unsigned int div_done_pclk_dma_bus_lppll_div     :  1;
        unsigned int reserved_2                          :  1;
        unsigned int div_done_clk_dsi_sys                :  1;
        unsigned int div_done_clk_debug_apb              :  1;
        unsigned int reserved_3                          :  1;
        unsigned int div_done_clk_debug_atb_h            :  1;
        unsigned int reserved_4                          :  4;
    } reg;
}peri_crg_peristat1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_done_clk_debug_trace         :  1;
        unsigned int div_done_clk_xctrl_ddr           :  1;
        unsigned int div_done_clk_xctrl_cpu           :  1;
        unsigned int div_done_pclk_crg_core_lppll_div :  1;
        unsigned int div_done_clk_hss1_gpll_div       :  1;
        unsigned int div_done_clk_hss1_lppll_div      :  1;
        unsigned int div_done_clk_hss2_gpll_div       :  1;
        unsigned int div_done_clk_hss2_lppll_div      :  1;
        unsigned int div_done_clk_xrse_gpll_div       :  1;
        unsigned int div_done_clk_cpu_bus             :  1;
        unsigned int div_done_clk_pcie_aux            :  1;
        unsigned int div_done_pclk_crg_core_gpll_div  :  1;
        unsigned int reserved_0                       :  2;
        unsigned int div_done_clk_ddr_dficlk_gpll_div :  1;
        unsigned int reserved_1                       :  2;
        unsigned int div_done_clk_ocm_lppll_div       :  1;
        unsigned int div_done_clk_ocm_gpll_div        :  1;
        unsigned int div_done_clk_gpiodb              :  1;
        unsigned int div_done_clk_peri2npu_noc        :  1;
        unsigned int div_done_clk_peri2npu_nic        :  1;
        unsigned int reserved_2                       : 10;
    } reg;
}peri_crg_peristat2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_ack_clk_mainbus_data_gpll :  4;
        unsigned int sw_ack_clk_mainbus_data      :  3;
        unsigned int sw_ack_clk_sysbus_data_gpll  :  4;
        unsigned int sw_ack_clk_sysbus_data       :  3;
        unsigned int sw_ack_clk_ip_sw             :  3;
        unsigned int reserved_0                   :  2;
        unsigned int sw_ack_clk_dma_bus_data      :  3;
        unsigned int sw_ack_clk_csi_sys_gpll      :  3;
        unsigned int sw_ack_clk_csi_sys           :  3;
        unsigned int sw_ack_clk_ocm               :  3;
        unsigned int reserved_1                   :  1;
    } reg;
}peri_crg_peristat3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_ack_clk_dsi         :  3;
        unsigned int sw_ack_clk_debug_apb   :  3;
        unsigned int sw_ack_clk_debug_trace :  3;
        unsigned int sw_ack_clk_debug_atb_h :  3;
        unsigned int sw_ack_clk_xctrl_ddr   :  3;
        unsigned int sw_ack_clk_xctrl_cpu   :  3;
        unsigned int sw_ack_pclk_crg_core   :  3;
        unsigned int sw_ack_clk_hss1        :  3;
        unsigned int sw_ack_clk_hss2        :  3;
        unsigned int sw_ack_pclk_dma_bus    :  3;
        unsigned int reserved_0             :  2;
    } reg;
}peri_crg_peristat4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_ack_clk_xrse            :  2;
        unsigned int reserved_0                 :  1;
        unsigned int sw_ack_clk_ddr_dficlk      :  3;
        unsigned int reserved_1                 :  4;
        unsigned int sw_ack_clk_cfgbus          :  3;
        unsigned int sw_ack_clk_cpu_bus         :  3;
        unsigned int sw_ack_clk_peri2npu_nic_sw :  4;
        unsigned int sw_ack_clk_peri2npu_noc_sw :  4;
        unsigned int reserved_2                 :  8;
    } reg;
}peri_crg_peristat5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_clk_csi_sys_gpll :  2;
        unsigned int sel_clk_csi_sys      :  2;
        unsigned int sel_clk_dsi          :  2;
        unsigned int reserved_0           :  2;
        unsigned int sel_clk_peri_timer0  :  1;
        unsigned int sel_clk_peri_timer1  :  1;
        unsigned int sel_clk_peri_timer2  :  1;
        unsigned int sel_clk_peri_timer3  :  1;
        unsigned int sel_clk_ocm          :  2;
        unsigned int reserved_1           :  2;
        unsigned int _bm_                 : 16;
    } reg;
}peri_crg_clkdiv4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autogt_state_clk_ddr_dficlk :  1;
        unsigned int reserved_0                  :  1;
        unsigned int dma_bus_idle_state          :  1;
        unsigned int sysbus_idle_state           :  1;
        unsigned int cfgbus_idle_state           :  1;
        unsigned int xctrl_cpu_idle_state        :  1;
        unsigned int xctrl_ddr_idle_state        :  1;
        unsigned int mainbus_autofs_idle_state   :  1;
        unsigned int mainbus_autogt_idle_state   :  1;
        unsigned int m1_bus_data_idle_state      :  1;
        unsigned int m1_bus_cfg_idle_state       :  1;
        unsigned int m2_bus_data_idle_state      :  1;
        unsigned int m2_bus_cfg_idle_state       :  1;
        unsigned int m2_noc_data_idle_state      :  1;
        unsigned int npu_bus_data_idle_state     :  1;
        unsigned int npu_bus_cfg_idle_state      :  1;
        unsigned int hss1_bus_data_idle_state    :  1;
        unsigned int hss1_bus_cfg_idle_state     :  1;
        unsigned int hss2_bus_data_idle_state    :  1;
        unsigned int hss2_bus_cfg_idle_state     :  1;
        unsigned int ocm_idle_state              :  1;
        unsigned int cpu_bus_idle_state          :  1;
        unsigned int reserved_1                  : 10;
    } reg;
}peri_crg_peristat6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_lock      :  1;
        unsigned int ppll0_lock     :  1;
        unsigned int ppll1_lock     :  1;
        unsigned int ppll2_lock     :  1;
        unsigned int ppll3_lock     :  1;
        unsigned int ddr_ppll0_lock :  1;
        unsigned int ddr_ppll1_lock :  1;
        unsigned int reserved_0     : 25;
    } reg;
}peri_crg_pll_lock_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_gpll  :  1;
        unsigned int intr_mask_pll_unlock_gpll   :  1;
        unsigned int intr_mask_vote_req_err_gpll :  1;
        unsigned int reserved_0                  : 29;
    } reg;
}peri_crg_intr_mask_gpll_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_gpll  :  1;
        unsigned int intr_clear_pll_unlock_gpll   :  1;
        unsigned int intr_clear_vote_req_err_gpll :  1;
        unsigned int reserved_0                   : 29;
    } reg;
}peri_crg_intr_clear_gpll_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_ppll0  :  1;
        unsigned int intr_mask_pll_unlock_ppll0   :  1;
        unsigned int intr_mask_vote_req_err_ppll0 :  1;
        unsigned int reserved_0                   : 29;
    } reg;
}peri_crg_intr_mask_ppll0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_ppll0  :  1;
        unsigned int intr_clear_pll_unlock_ppll0   :  1;
        unsigned int intr_clear_vote_req_err_ppll0 :  1;
        unsigned int reserved_0                    : 29;
    } reg;
}peri_crg_intr_clear_ppll0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_ppll1  :  1;
        unsigned int intr_mask_pll_unlock_ppll1   :  1;
        unsigned int intr_mask_vote_req_err_ppll1 :  1;
        unsigned int reserved_0                   : 29;
    } reg;
}peri_crg_intr_mask_ppll1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_ppll1  :  1;
        unsigned int intr_clear_pll_unlock_ppll1   :  1;
        unsigned int intr_clear_vote_req_err_ppll1 :  1;
        unsigned int reserved_0                    : 29;
    } reg;
}peri_crg_intr_clear_ppll1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_ppll2  :  1;
        unsigned int intr_mask_pll_unlock_ppll2   :  1;
        unsigned int intr_mask_vote_req_err_ppll2 :  1;
        unsigned int reserved_0                   : 29;
    } reg;
}peri_crg_intr_mask_ppll2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_ppll2  :  1;
        unsigned int intr_clear_pll_unlock_ppll2   :  1;
        unsigned int intr_clear_vote_req_err_ppll2 :  1;
        unsigned int reserved_0                    : 29;
    } reg;
}peri_crg_intr_clear_ppll2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_ppll3  :  1;
        unsigned int intr_mask_pll_unlock_ppll3   :  1;
        unsigned int intr_mask_vote_req_err_ppll3 :  1;
        unsigned int reserved_0                   : 29;
    } reg;
}peri_crg_intr_mask_ppll3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_ppll3  :  1;
        unsigned int intr_clear_pll_unlock_ppll3   :  1;
        unsigned int intr_clear_vote_req_err_ppll3 :  1;
        unsigned int reserved_0                    : 29;
    } reg;
}peri_crg_intr_clear_ppll3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_ddr_ppll0  :  1;
        unsigned int intr_mask_pll_unlock_ddr_ppll0   :  1;
        unsigned int intr_mask_vote_req_err_ddr_ppll0 :  1;
        unsigned int reserved_0                       : 29;
    } reg;
}peri_crg_intr_mask_ddr_ppll0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_ddr_ppll0  :  1;
        unsigned int intr_clear_pll_unlock_ddr_ppll0   :  1;
        unsigned int intr_clear_vote_req_err_ddr_ppll0 :  1;
        unsigned int reserved_0                        : 29;
    } reg;
}peri_crg_intr_clear_ddr_ppll0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_ddr_ppll1  :  1;
        unsigned int intr_mask_pll_unlock_ddr_ppll1   :  1;
        unsigned int intr_mask_vote_req_err_ddr_ppll1 :  1;
        unsigned int reserved_0                       : 29;
    } reg;
}peri_crg_intr_mask_ddr_ppll1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_ddr_ppll1  :  1;
        unsigned int intr_clear_pll_unlock_ddr_ppll1   :  1;
        unsigned int intr_clear_vote_req_err_ddr_ppll1 :  1;
        unsigned int reserved_0                        : 29;
    } reg;
}peri_crg_intr_clear_ddr_ppll1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pll_ini_err_status_gpll       :  1;
        unsigned int intr_pll_unlock_status_gpll        :  1;
        unsigned int intr_vote_req_err_status_gpll      :  1;
        unsigned int intr_pll_ini_err_status_ppll0      :  1;
        unsigned int intr_pll_unlock_status_ppll0       :  1;
        unsigned int intr_vote_req_err_status_ppll0     :  1;
        unsigned int intr_pll_ini_err_status_ppll1      :  1;
        unsigned int intr_pll_unlock_status_ppll1       :  1;
        unsigned int intr_vote_req_err_status_ppll1     :  1;
        unsigned int intr_pll_ini_err_status_ppll2      :  1;
        unsigned int intr_pll_unlock_status_ppll2       :  1;
        unsigned int intr_vote_req_err_status_ppll2     :  1;
        unsigned int intr_pll_ini_err_status_ppll3      :  1;
        unsigned int intr_pll_unlock_status_ppll3       :  1;
        unsigned int intr_vote_req_err_status_ppll3     :  1;
        unsigned int intr_pll_ini_err_status_ddr_ppll0  :  1;
        unsigned int intr_pll_unlock_status_ddr_ppll0   :  1;
        unsigned int intr_vote_req_err_status_ddr_ppll0 :  1;
        unsigned int intr_pll_ini_err_status_ddr_ppll1  :  1;
        unsigned int intr_pll_unlock_status_ddr_ppll1   :  1;
        unsigned int intr_vote_req_err_status_ddr_ppll1 :  1;
        unsigned int reserved_0                         : 11;
    } reg;
}peri_crg_intr_status_pll_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pll_ini_err_status_msk_gpll       :  1;
        unsigned int intr_pll_unlock_status_msk_gpll        :  1;
        unsigned int intr_vote_req_err_status_msk_gpll      :  1;
        unsigned int intr_pll_ini_err_status_msk_ppll0      :  1;
        unsigned int intr_pll_unlock_status_msk_ppll0       :  1;
        unsigned int intr_vote_req_err_status_msk_ppll0     :  1;
        unsigned int intr_pll_ini_err_status_msk_ppll1      :  1;
        unsigned int intr_pll_unlock_status_msk_ppll1       :  1;
        unsigned int intr_vote_req_err_status_msk_ppll1     :  1;
        unsigned int intr_pll_ini_err_status_msk_ppll2      :  1;
        unsigned int intr_pll_unlock_status_msk_ppll2       :  1;
        unsigned int intr_vote_req_err_status_msk_ppll2     :  1;
        unsigned int intr_pll_ini_err_status_msk_ppll3      :  1;
        unsigned int intr_pll_unlock_status_msk_ppll3       :  1;
        unsigned int intr_vote_req_err_status_msk_ppll3     :  1;
        unsigned int intr_pll_ini_err_status_msk_ddr_ppll0  :  1;
        unsigned int intr_pll_unlock_status_msk_ddr_ppll0   :  1;
        unsigned int intr_vote_req_err_status_msk_ddr_ppll0 :  1;
        unsigned int intr_pll_ini_err_status_msk_ddr_ppll1  :  1;
        unsigned int intr_pll_unlock_status_msk_ddr_ppll1   :  1;
        unsigned int intr_vote_req_err_status_msk_ddr_ppll1 :  1;
        unsigned int reserved_0                             : 11;
    } reg;
}peri_crg_intr_status_pll_msk_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_vote_en :  8;
        unsigned int reserved_0   :  8;
        unsigned int _bm_         : 16;
    } reg;
}peri_crg_gpll_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_vote_en :  8;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}peri_crg_ppll0_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_vote_en :  8;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}peri_crg_ppll1_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_vote_en :  8;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}peri_crg_ppll2_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_vote_en :  8;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}peri_crg_ppll3_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_vote_en :  8;
        unsigned int reserved_0        :  8;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_ddr_ppll0_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_vote_en :  8;
        unsigned int reserved_0        :  8;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_ddr_ppll1_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_ddrppll1_test   :  6;
        unsigned int div_clk_ddrppll0_test   :  6;
        unsigned int sc_gt_clk_ddrppll1_test :  1;
        unsigned int sc_gt_clk_ddrppll0_test :  1;
        unsigned int reserved_0              :  2;
        unsigned int _bm_                    : 16;
    } reg;
}peri_crg_clkdiv25_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_gpll_test         :  6;
        unsigned int div_clk_ddr_dficlk_gpll   :  6;
        unsigned int sc_gt_clk_gpll_test       :  1;
        unsigned int sc_gt_clk_ddr_dficlk_gpll :  1;
        unsigned int reserved_0                :  2;
        unsigned int _bm_                      : 16;
    } reg;
}peri_crg_clkdiv26_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_ppll1_test   :  6;
        unsigned int div_clk_ppll0_test   :  6;
        unsigned int sc_gt_clk_ppll1_test :  1;
        unsigned int sc_gt_clk_ppll0_test :  1;
        unsigned int reserved_0           :  2;
        unsigned int _bm_                 : 16;
    } reg;
}peri_crg_clkdiv37_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_ppll3_test   :  6;
        unsigned int div_clk_ppll2_test   :  6;
        unsigned int sc_gt_clk_ppll3_test :  1;
        unsigned int sc_gt_clk_ppll2_test :  1;
        unsigned int reserved_0           :  2;
        unsigned int _bm_                 : 16;
    } reg;
}peri_crg_clkdiv38_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_dma_bus : 16;
        unsigned int _bm_                  : 16;
    } reg;
}peri_crg_autofsctrl22_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_sysbus : 16;
        unsigned int _bm_                 : 16;
    } reg;
}peri_crg_autofsctrl23_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_cfgbus : 16;
        unsigned int _bm_                 : 16;
    } reg;
}peri_crg_autofsctrl24_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_xctrl_cpu : 16;
        unsigned int _bm_                    : 16;
    } reg;
}peri_crg_autofsctrl25_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_xctrl_ddr : 16;
        unsigned int _bm_                    : 16;
    } reg;
}peri_crg_autofsctrl26_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_mainbus : 16;
        unsigned int _bm_                  : 16;
    } reg;
}peri_crg_autofsctrl27_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_ocm : 16;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_autofsctrl28_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autofs_bypass_cpu_bus : 16;
        unsigned int _bm_                  : 16;
    } reg;
}peri_crg_autofsctrl29_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_peri_pll_test         :  5;
        unsigned int sel_clk_peri_pll_test_1st :  2;
        unsigned int sel_clk_peri_pll_test_2nd :  2;
        unsigned int reserved_0                : 23;
    } reg;
}peri_crg_perictrl7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_crg_reserved_in : 32;
    } reg;
}peri_crg_peristat7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_crg_reserved_out : 32;
    } reg;
}peri_crg_perictrl8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0             :  1;
        unsigned int syscfg_bus_tgt_timeout : 22;
        unsigned int reserved_1             :  9;
    } reg;
}peri_crg_syscfg_bus_timeout_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int syscfg_bus_ini_no_pending_trans : 10;
        unsigned int syscfg_bus_tgt_no_pending_trans : 22;
    } reg;
}peri_crg_syscfg_bus_trans_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                       :  1;
        unsigned int pericfg_bus_ini_no_pending_trans :  1;
        unsigned int pericfg_bus_tgt_no_pending_trans :  3;
        unsigned int pericfg_bus_tgt_timeout          :  3;
        unsigned int reserved_1                       :  6;
        unsigned int pericfg_bus_no_pending_trans     :  1;
        unsigned int reserved_2                       : 17;
    } reg;
}peri_crg_pericfg_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int syscfg_bus_pdom_xctrlddr_slv_idleack :  1;
        unsigned int syscfg_bus_pdom_xctrlddr_mst_idleack :  1;
        unsigned int syscfg_bus_pdom_npu_slv_idleack      :  1;
        unsigned int syscfg_bus_pdom_npu_mst_idleack      :  1;
        unsigned int syscfg_bus_pdom_m85_slv_idleack      :  1;
        unsigned int syscfg_bus_pdom_m2_slv_idleack       :  1;
        unsigned int syscfg_bus_pdom_m2_mst_idleack       :  1;
        unsigned int syscfg_bus_pdom_lpis_slv_idleack     :  1;
        unsigned int syscfg_bus_pdom_lpis_mst_idleack     :  1;
        unsigned int syscfg_bus_pdom_hs2_slv_idleack      :  1;
        unsigned int reserved_0                           :  1;
        unsigned int syscfg_bus_pdom_hs1_slv_idleack      :  1;
        unsigned int reserved_1                           :  1;
        unsigned int syscfg_bus_pdom_cpu_slv_idleack      :  1;
        unsigned int syscfg_bus_pdom_cpu_mst_idleack      :  1;
        unsigned int syscfg_bus_pdom_pericfg_idleack      :  1;
        unsigned int syscfg_bus_pdom_xctrlddr_slv_idle    :  1;
        unsigned int syscfg_bus_pdom_xctrlddr_mst_idle    :  1;
        unsigned int syscfg_bus_pdom_npu_slv_idle         :  1;
        unsigned int syscfg_bus_pdom_npu_mst_idle         :  1;
        unsigned int syscfg_bus_pdom_m85_slv_idle         :  1;
        unsigned int syscfg_bus_pdom_m2_slv_idle          :  1;
        unsigned int syscfg_bus_pdom_m2_mst_idle          :  1;
        unsigned int syscfg_bus_pdom_lpis_slv_idle        :  1;
        unsigned int syscfg_bus_pdom_lpis_mst_idle        :  1;
        unsigned int syscfg_bus_pdom_hs2_slv_idle         :  1;
        unsigned int reserved_2                           :  1;
        unsigned int syscfg_bus_pdom_hs1_slv_idle         :  1;
        unsigned int reserved_3                           :  1;
        unsigned int syscfg_bus_pdom_cpu_slv_idle         :  1;
        unsigned int syscfg_bus_pdom_cpu_mst_idle         :  1;
        unsigned int syscfg_bus_pdom_pericfg_idle         :  1;
    } reg;
}peri_crg_syscfg_bus_pdom_idle_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int syscfg_bus_pdom_dbg_slv_idleack :  1;
        unsigned int syscfg_bus_pdom_dbg_mst_idleack :  1;
        unsigned int syscfg_bus_pdom_m1_idleack      :  1;
        unsigned int syscfg_bus_pdom_gpu_axi_idleack :  1;
        unsigned int syscfg_bus_pdom_gpu_apb_idleack :  1;
        unsigned int syscfg_bus_pdom_dma_slv_idleack :  1;
        unsigned int syscfg_bus_pdom_dma_mst_idleack :  1;
        unsigned int syscfg_bus_pdom_dbg_slv_idle    :  1;
        unsigned int syscfg_bus_pdom_dbg_mst_idle    :  1;
        unsigned int syscfg_bus_pdom_m1_idle         :  1;
        unsigned int syscfg_bus_pdom_gpu_axi_idle    :  1;
        unsigned int syscfg_bus_pdom_gpu_apb_idle    :  1;
        unsigned int syscfg_bus_pdom_dma_slv_idle    :  1;
        unsigned int syscfg_bus_pdom_dma_mst_idle    :  1;
        unsigned int reserved_0                      : 18;
    } reg;
}peri_crg_syscfg_bus_pdom_idle_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int syscfg_bus_pdom_hs1_mst_idleack :  1;
        unsigned int syscfg_bus_pdom_hs2_mst_idleack :  1;
        unsigned int reserved_0                      :  4;
        unsigned int syscfg_bus_pdom_hs1_mst_idle    :  1;
        unsigned int syscfg_bus_pdom_hs2_mst_idle    :  1;
        unsigned int reserved_1                      : 24;
    } reg;
}peri_crg_hss_sys_pdom_idle_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int idle_flag_perfstat_peri        :  1;
        unsigned int idle_flag_perfstat_gpu         :  1;
        unsigned int idle_flag_perfstat_cpu         :  1;
        unsigned int cpu_wfx                        :  1;
        unsigned int cpu_fast_wakeup                :  4;
        unsigned int dpu_idle_for_ocm               :  1;
        unsigned int idle_flag_ocm_nic              :  1;
        unsigned int idle_flag_ndb_mst_m2           :  1;
        unsigned int idle_flag_ndb_slv_m2           :  1;
        unsigned int idle_flag_ndb_mst_m1           :  1;
        unsigned int idle_flag_ndb_slv_m1           :  1;
        unsigned int idle_flag_adb_mst_npu          :  1;
        unsigned int idle_flag_adb_slv_npu          :  1;
        unsigned int idle_flag_adb_mst_gpu          :  1;
        unsigned int idle_flag_adb_slv_gpu          :  1;
        unsigned int idle_flag_main_adb_mst_cpu     :  1;
        unsigned int idle_flag_main_adb_slv_cpu     :  1;
        unsigned int idle_flag_xctrl_ddr_m3         :  1;
        unsigned int xctrl_ddr_bus_no_pending_trans :  1;
        unsigned int idle_flag_xctrl_cpu_m3         :  1;
        unsigned int xctrl_cpu_bus_no_pending_trans :  1;
        unsigned int idle_flag_npu_nic2cfgbus       :  1;
        unsigned int idle_flag_cfgbus2ocm_nic       :  1;
        unsigned int reserved_0                     :  1;
        unsigned int peri_dmac_busy_ns              :  1;
        unsigned int peri_dmac_busy_s               :  1;
        unsigned int reserved_1                     :  3;
    } reg;
}peri_crg_peristat8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_mainbus_data        :  1;
        unsigned int gt_clk_mainbus_data_up     :  1;
        unsigned int reserved_0                 :  2;
        unsigned int gt_clk_sysbus_data         :  1;
        unsigned int gt_clk_sysbus_data_up      :  1;
        unsigned int gt_clk_mainbus_data_dbg    :  1;
        unsigned int gt_aclk_ddr_adb400_slv     :  1;
        unsigned int gt_clk_sysbus_data_dma     :  1;
        unsigned int gt_clk_cfgbus              :  1;
        unsigned int reserved_1                 :  1;
        unsigned int gt_clk_cfgbus_up           :  1;
        unsigned int gt_clk_ddr_cfg_adb400_slv  :  1;
        unsigned int gt_clk_pericfg_bus         :  1;
        unsigned int gt_clk_debug_atb           :  1;
        unsigned int gt_clk_debug_atb_xctrl_ddr :  1;
        unsigned int gt_clk_cfgbus_debug        :  1;
        unsigned int gt_clk_debug_atb_h         :  1;
        unsigned int gt_clk_cfgbus_dma          :  1;
        unsigned int gt_clk_cfgbus_peri         :  1;
        unsigned int gt_clk_sysbus_data_dbg     :  1;
        unsigned int reserved_2                 :  3;
        unsigned int gt_pclk_peri_timer_s       :  1;
        unsigned int gt_pclk_peri_wdt3          :  1;
        unsigned int gt_pclk_peri_wdt2          :  1;
        unsigned int reserved_3                 :  5;
    } reg;
}peri_crg_clkgt3_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_mainbus_data        :  1;
        unsigned int gt_clk_mainbus_data_up     :  1;
        unsigned int reserved_0                 :  2;
        unsigned int gt_clk_sysbus_data         :  1;
        unsigned int gt_clk_sysbus_data_up      :  1;
        unsigned int gt_clk_mainbus_data_dbg    :  1;
        unsigned int gt_aclk_ddr_adb400_slv     :  1;
        unsigned int gt_clk_sysbus_data_dma     :  1;
        unsigned int gt_clk_cfgbus              :  1;
        unsigned int reserved_1                 :  1;
        unsigned int gt_clk_cfgbus_up           :  1;
        unsigned int gt_clk_ddr_cfg_adb400_slv  :  1;
        unsigned int gt_clk_pericfg_bus         :  1;
        unsigned int gt_clk_debug_atb           :  1;
        unsigned int gt_clk_debug_atb_xctrl_ddr :  1;
        unsigned int gt_clk_cfgbus_debug        :  1;
        unsigned int gt_clk_debug_atb_h         :  1;
        unsigned int gt_clk_cfgbus_dma          :  1;
        unsigned int gt_clk_cfgbus_peri         :  1;
        unsigned int gt_clk_sysbus_data_dbg     :  1;
        unsigned int reserved_2                 :  3;
        unsigned int gt_pclk_peri_timer_s       :  1;
        unsigned int gt_pclk_peri_wdt3          :  1;
        unsigned int gt_pclk_peri_wdt2          :  1;
        unsigned int reserved_3                 :  5;
    } reg;
}peri_crg_clkgt3_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_mainbus_data        :  1;
        unsigned int gt_clk_mainbus_data_up     :  1;
        unsigned int reserved_0                 :  2;
        unsigned int gt_clk_sysbus_data         :  1;
        unsigned int gt_clk_sysbus_data_up      :  1;
        unsigned int gt_clk_mainbus_data_dbg    :  1;
        unsigned int gt_aclk_ddr_adb400_slv     :  1;
        unsigned int gt_clk_sysbus_data_dma     :  1;
        unsigned int gt_clk_cfgbus              :  1;
        unsigned int reserved_1                 :  1;
        unsigned int gt_clk_cfgbus_up           :  1;
        unsigned int gt_clk_ddr_cfg_adb400_slv  :  1;
        unsigned int gt_clk_pericfg_bus         :  1;
        unsigned int gt_clk_debug_atb           :  1;
        unsigned int gt_clk_debug_atb_xctrl_ddr :  1;
        unsigned int gt_clk_cfgbus_debug        :  1;
        unsigned int gt_clk_debug_atb_h         :  1;
        unsigned int gt_clk_cfgbus_dma          :  1;
        unsigned int gt_clk_cfgbus_peri         :  1;
        unsigned int gt_clk_sysbus_data_dbg     :  1;
        unsigned int reserved_2                 :  3;
        unsigned int gt_pclk_peri_timer_s       :  1;
        unsigned int gt_pclk_peri_wdt3          :  1;
        unsigned int gt_pclk_peri_wdt2          :  1;
        unsigned int reserved_3                 :  5;
    } reg;
}peri_crg_clkgt3_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_mainbus_data            :  1;
        unsigned int st_clk_mainbus_data_up         :  1;
        unsigned int reserved_0                     :  2;
        unsigned int st_clk_sysbus_data             :  1;
        unsigned int st_clk_sysbus_data_up          :  1;
        unsigned int st_clk_mainbus_data_dbg        :  1;
        unsigned int st_aclk_ddr_adb400_slv_ddra    :  1;
        unsigned int st_clk_sysbus_data_dma         :  1;
        unsigned int st_clk_cfgbus                  :  1;
        unsigned int reserved_1                     :  1;
        unsigned int st_clk_cfgbus_up               :  1;
        unsigned int st_clk_ddr_cfg_adb400_slv_ddra :  1;
        unsigned int st_clk_pericfg_bus             :  1;
        unsigned int st_clk_debug_atb               :  1;
        unsigned int st_clk_debug_atb_xctrl_ddr     :  1;
        unsigned int st_clk_cfgbus_debug            :  1;
        unsigned int st_clk_debug_atb_h             :  1;
        unsigned int st_clk_cfgbus_dma              :  1;
        unsigned int st_clk_cfgbus_peri             :  1;
        unsigned int st_clk_sysbus_data_dbg         :  1;
        unsigned int reserved_2                     :  3;
        unsigned int st_pclk_peri_timer_s           :  1;
        unsigned int st_pclk_peri_wdt3              :  1;
        unsigned int st_pclk_peri_wdt2              :  1;
        unsigned int reserved_3                     :  5;
    } reg;
}peri_crg_clkst3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_spi0       :  1;
        unsigned int gt_pclk_spi1       :  1;
        unsigned int gt_pclk_spi2       :  1;
        unsigned int gt_clk_peri_timer4 :  1;
        unsigned int gt_clk_peri_timer5 :  1;
        unsigned int gt_clk_peri_timer6 :  1;
        unsigned int gt_clk_peri_timer7 :  1;
        unsigned int gt_pclk_peri_ipc0  :  1;
        unsigned int reserved_0         : 17;
        unsigned int gt_pclk_peri_gpio  :  1;
        unsigned int reserved_1         :  6;
    } reg;
}peri_crg_clkgt4_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_spi0       :  1;
        unsigned int gt_pclk_spi1       :  1;
        unsigned int gt_pclk_spi2       :  1;
        unsigned int gt_clk_peri_timer4 :  1;
        unsigned int gt_clk_peri_timer5 :  1;
        unsigned int gt_clk_peri_timer6 :  1;
        unsigned int gt_clk_peri_timer7 :  1;
        unsigned int gt_pclk_peri_ipc0  :  1;
        unsigned int reserved_0         : 17;
        unsigned int gt_pclk_peri_gpio  :  1;
        unsigned int reserved_1         :  6;
    } reg;
}peri_crg_clkgt4_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_spi0       :  1;
        unsigned int gt_pclk_spi1       :  1;
        unsigned int gt_pclk_spi2       :  1;
        unsigned int gt_clk_peri_timer4 :  1;
        unsigned int gt_clk_peri_timer5 :  1;
        unsigned int gt_clk_peri_timer6 :  1;
        unsigned int gt_clk_peri_timer7 :  1;
        unsigned int gt_pclk_peri_ipc0  :  1;
        unsigned int reserved_0         : 17;
        unsigned int gt_pclk_peri_gpio  :  1;
        unsigned int reserved_1         :  6;
    } reg;
}peri_crg_clkgt4_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_pclk_spi0                   :  1;
        unsigned int st_pclk_spi1                   :  1;
        unsigned int st_pclk_spi2                   :  1;
        unsigned int st_clk_peri_timer4             :  1;
        unsigned int st_clk_peri_timer5             :  1;
        unsigned int st_clk_peri_timer6             :  1;
        unsigned int st_clk_peri_timer7             :  1;
        unsigned int st_pclk_peri_ipc0              :  1;
        unsigned int reserved_0                     : 17;
        unsigned int st_pclk_peri_gpio              :  1;
        unsigned int st_clk_ddr_cfg_adb400_slv_ddrb :  1;
        unsigned int st_clk_ddr_cfg_adb400_slv_ddrc :  1;
        unsigned int st_clk_ddr_cfg_adb400_slv_ddrd :  1;
        unsigned int st_aclk_ddr_adb400_slv_ddrb    :  1;
        unsigned int st_aclk_ddr_adb400_slv_ddrc    :  1;
        unsigned int st_aclk_ddr_adb400_slv_ddrd    :  1;
    } reg;
}peri_crg_clkst4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                          :  2;
        unsigned int gt_pclk_pctrl                       :  1;
        unsigned int reserved_1                          :  4;
        unsigned int gt_pclk_peri_ioctrl                 :  1;
        unsigned int gt_pclk_peril_iopadwrap             :  1;
        unsigned int gt_pclk_perir_iopadwrap             :  1;
        unsigned int reserved_2                          :  7;
        unsigned int gt_pclk_onewire0                    :  1;
        unsigned int gt_pclk_onewire1                    :  1;
        unsigned int gt_pclk_onewire2                    :  1;
        unsigned int reserved_3                          :  2;
        unsigned int gt_clk_gpll_peri                    :  1;
        unsigned int gt_clk_gpll_lpis                    :  1;
        unsigned int gt_clk_broadcast                    :  1;
        unsigned int gt_clk_broadcast_xctrl_ddr_abrg_slv :  1;
        unsigned int reserved_4                          :  6;
    } reg;
}peri_crg_clkgt5_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                          :  2;
        unsigned int gt_pclk_pctrl                       :  1;
        unsigned int reserved_1                          :  4;
        unsigned int gt_pclk_peri_ioctrl                 :  1;
        unsigned int gt_pclk_peril_iopadwrap             :  1;
        unsigned int gt_pclk_perir_iopadwrap             :  1;
        unsigned int reserved_2                          :  7;
        unsigned int gt_pclk_onewire0                    :  1;
        unsigned int gt_pclk_onewire1                    :  1;
        unsigned int gt_pclk_onewire2                    :  1;
        unsigned int reserved_3                          :  2;
        unsigned int gt_clk_gpll_peri                    :  1;
        unsigned int gt_clk_gpll_lpis                    :  1;
        unsigned int gt_clk_broadcast                    :  1;
        unsigned int gt_clk_broadcast_xctrl_ddr_abrg_slv :  1;
        unsigned int reserved_4                          :  6;
    } reg;
}peri_crg_clkgt5_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                          :  2;
        unsigned int gt_pclk_pctrl                       :  1;
        unsigned int reserved_1                          :  4;
        unsigned int gt_pclk_peri_ioctrl                 :  1;
        unsigned int gt_pclk_peril_iopadwrap             :  1;
        unsigned int gt_pclk_perir_iopadwrap             :  1;
        unsigned int reserved_2                          :  7;
        unsigned int gt_pclk_onewire0                    :  1;
        unsigned int gt_pclk_onewire1                    :  1;
        unsigned int gt_pclk_onewire2                    :  1;
        unsigned int reserved_3                          :  2;
        unsigned int gt_clk_gpll_peri                    :  1;
        unsigned int gt_clk_gpll_lpis                    :  1;
        unsigned int gt_clk_broadcast                    :  1;
        unsigned int gt_clk_broadcast_xctrl_ddr_abrg_slv :  1;
        unsigned int reserved_4                          :  6;
    } reg;
}peri_crg_clkgt5_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                          :  2;
        unsigned int st_pclk_pctrl                       :  1;
        unsigned int reserved_1                          :  4;
        unsigned int st_pclk_peri_ioctrl                 :  1;
        unsigned int st_pclk_peril_iopadwrap             :  1;
        unsigned int st_pclk_perir_iopadwrap             :  1;
        unsigned int reserved_2                          :  7;
        unsigned int st_pclk_onewire0                    :  1;
        unsigned int st_pclk_onewire1                    :  1;
        unsigned int st_pclk_onewire2                    :  1;
        unsigned int reserved_3                          :  2;
        unsigned int st_clk_gpll_peri                    :  1;
        unsigned int st_clk_gpll_lpis                    :  1;
        unsigned int st_clk_broadcast                    :  1;
        unsigned int st_clk_broadcast_xctrl_ddr_abrg_slv :  1;
        unsigned int reserved_4                          :  6;
    } reg;
}peri_crg_clkst5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0             :  8;
        unsigned int gt_clk_peri_gpiodb     :  1;
        unsigned int reserved_1             :  3;
        unsigned int gt_clkin_ref_xctrl_ddr :  1;
        unsigned int gt_clkin_ref_core      :  1;
        unsigned int reserved_2             :  5;
        unsigned int gt_clk_spi0            :  1;
        unsigned int gt_clk_spi1            :  1;
        unsigned int gt_clk_spi2            :  1;
        unsigned int reserved_3             :  6;
        unsigned int gt_clk_peri_wdt2       :  1;
        unsigned int gt_clk_peri_wdt3       :  1;
        unsigned int reserved_4             :  2;
    } reg;
}peri_crg_clkgt6_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0             :  8;
        unsigned int gt_clk_peri_gpiodb     :  1;
        unsigned int reserved_1             :  3;
        unsigned int gt_clkin_ref_xctrl_ddr :  1;
        unsigned int gt_clkin_ref_core      :  1;
        unsigned int reserved_2             :  5;
        unsigned int gt_clk_spi0            :  1;
        unsigned int gt_clk_spi1            :  1;
        unsigned int gt_clk_spi2            :  1;
        unsigned int reserved_3             :  6;
        unsigned int gt_clk_peri_wdt2       :  1;
        unsigned int gt_clk_peri_wdt3       :  1;
        unsigned int reserved_4             :  2;
    } reg;
}peri_crg_clkgt6_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0             :  8;
        unsigned int gt_clk_peri_gpiodb     :  1;
        unsigned int reserved_1             :  3;
        unsigned int gt_clkin_ref_xctrl_ddr :  1;
        unsigned int gt_clkin_ref_core      :  1;
        unsigned int reserved_2             :  5;
        unsigned int gt_clk_spi0            :  1;
        unsigned int gt_clk_spi1            :  1;
        unsigned int gt_clk_spi2            :  1;
        unsigned int reserved_3             :  6;
        unsigned int gt_clk_peri_wdt2       :  1;
        unsigned int gt_clk_peri_wdt3       :  1;
        unsigned int reserved_4             :  2;
    } reg;
}peri_crg_clkgt6_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0             :  8;
        unsigned int st_clk_peri_gpiodb     :  1;
        unsigned int reserved_1             :  3;
        unsigned int st_clkin_ref_xctrl_ddr :  1;
        unsigned int st_clkin_ref_core      :  1;
        unsigned int reserved_2             :  5;
        unsigned int st_clk_spi0            :  1;
        unsigned int st_clk_spi1            :  1;
        unsigned int st_clk_spi2            :  1;
        unsigned int reserved_3             :  6;
        unsigned int st_clk_peri_wdt2       :  1;
        unsigned int st_clk_peri_wdt3       :  1;
        unsigned int reserved_4             :  2;
    } reg;
}peri_crg_clkst6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0           : 21;
        unsigned int gt_clk_axibist       :  1;
        unsigned int reserved_1           :  3;
        unsigned int gt_clk_dma_bus_data  :  1;
        unsigned int reserved_2           :  1;
        unsigned int gt_clk_cfgbus_media1 :  1;
        unsigned int gt_clk_sysbus_media1 :  1;
        unsigned int gt_clk_sysbus_media2 :  1;
        unsigned int reserved_3           :  2;
    } reg;
}peri_crg_clkgt7_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0           : 21;
        unsigned int gt_clk_axibist       :  1;
        unsigned int reserved_1           :  3;
        unsigned int gt_clk_dma_bus_data  :  1;
        unsigned int reserved_2           :  1;
        unsigned int gt_clk_cfgbus_media1 :  1;
        unsigned int gt_clk_sysbus_media1 :  1;
        unsigned int gt_clk_sysbus_media2 :  1;
        unsigned int reserved_3           :  2;
    } reg;
}peri_crg_clkgt7_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0           : 21;
        unsigned int gt_clk_axibist       :  1;
        unsigned int reserved_1           :  3;
        unsigned int gt_clk_dma_bus_data  :  1;
        unsigned int reserved_2           :  1;
        unsigned int gt_clk_cfgbus_media1 :  1;
        unsigned int gt_clk_sysbus_media1 :  1;
        unsigned int gt_clk_sysbus_media2 :  1;
        unsigned int reserved_3           :  2;
    } reg;
}peri_crg_clkgt7_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0           : 21;
        unsigned int st_clk_axibist       :  1;
        unsigned int reserved_1           :  3;
        unsigned int st_clk_dma_bus_data  :  1;
        unsigned int reserved_2           :  1;
        unsigned int st_clk_cfgbus_media1 :  1;
        unsigned int st_clk_sysbus_media1 :  1;
        unsigned int st_clk_sysbus_media2 :  1;
        unsigned int reserved_3           :  2;
    } reg;
}peri_crg_clkst7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_dma_bus            :  1;
        unsigned int gt_pclk_axibist            :  1;
        unsigned int reserved_0                 :  9;
        unsigned int gt_clk_onewire0            :  1;
        unsigned int gt_clk_onewire1            :  1;
        unsigned int gt_clk_onewire2            :  1;
        unsigned int reserved_1                 :  1;
        unsigned int gt_clk_debug_apb           :  1;
        unsigned int gt_clk_debug_apb_up        :  1;
        unsigned int reserved_2                 :  4;
        unsigned int gt_clk_npu_bus_cfg         :  1;
        unsigned int gt_clk_hss1_bus_cfg        :  1;
        unsigned int gt_clk_hss2_bus_cfg        :  1;
        unsigned int gt_pclk_media1_cfg         :  1;
        unsigned int gt_pclk_media2_cfg         :  1;
        unsigned int gt_clk_debug_apb_xctrl_ddr :  1;
        unsigned int reserved_3                 :  1;
        unsigned int gt_clk_debug_trace         :  1;
        unsigned int gt_clk_xctrl_ddr           :  1;
        unsigned int reserved_4                 :  2;
    } reg;
}peri_crg_clkgt8_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_dma_bus            :  1;
        unsigned int gt_pclk_axibist            :  1;
        unsigned int reserved_0                 :  9;
        unsigned int gt_clk_onewire0            :  1;
        unsigned int gt_clk_onewire1            :  1;
        unsigned int gt_clk_onewire2            :  1;
        unsigned int reserved_1                 :  1;
        unsigned int gt_clk_debug_apb           :  1;
        unsigned int gt_clk_debug_apb_up        :  1;
        unsigned int reserved_2                 :  4;
        unsigned int gt_clk_npu_bus_cfg         :  1;
        unsigned int gt_clk_hss1_bus_cfg        :  1;
        unsigned int gt_clk_hss2_bus_cfg        :  1;
        unsigned int gt_pclk_media1_cfg         :  1;
        unsigned int gt_pclk_media2_cfg         :  1;
        unsigned int gt_clk_debug_apb_xctrl_ddr :  1;
        unsigned int reserved_3                 :  1;
        unsigned int gt_clk_debug_trace         :  1;
        unsigned int gt_clk_xctrl_ddr           :  1;
        unsigned int reserved_4                 :  2;
    } reg;
}peri_crg_clkgt8_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_pclk_dma_bus            :  1;
        unsigned int gt_pclk_axibist            :  1;
        unsigned int reserved_0                 :  9;
        unsigned int gt_clk_onewire0            :  1;
        unsigned int gt_clk_onewire1            :  1;
        unsigned int gt_clk_onewire2            :  1;
        unsigned int reserved_1                 :  1;
        unsigned int gt_clk_debug_apb           :  1;
        unsigned int gt_clk_debug_apb_up        :  1;
        unsigned int reserved_2                 :  4;
        unsigned int gt_clk_npu_bus_cfg         :  1;
        unsigned int gt_clk_hss1_bus_cfg        :  1;
        unsigned int gt_clk_hss2_bus_cfg        :  1;
        unsigned int gt_pclk_media1_cfg         :  1;
        unsigned int gt_pclk_media2_cfg         :  1;
        unsigned int gt_clk_debug_apb_xctrl_ddr :  1;
        unsigned int reserved_3                 :  1;
        unsigned int gt_clk_debug_trace         :  1;
        unsigned int gt_clk_xctrl_ddr           :  1;
        unsigned int reserved_4                 :  2;
    } reg;
}peri_crg_clkgt8_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_pclk_dma_bus            :  1;
        unsigned int st_pclk_axibist            :  1;
        unsigned int reserved_0                 :  9;
        unsigned int st_clk_onewire0            :  1;
        unsigned int st_clk_onewire1            :  1;
        unsigned int st_clk_onewire2            :  1;
        unsigned int reserved_1                 :  1;
        unsigned int st_clk_debug_apb           :  1;
        unsigned int st_clk_debug_apb_up        :  1;
        unsigned int reserved_2                 :  4;
        unsigned int st_clk_npu_bus_cfg         :  1;
        unsigned int st_clk_hss1_bus_cfg        :  1;
        unsigned int st_clk_hss2_bus_cfg        :  1;
        unsigned int st_pclk_media1_cfg         :  1;
        unsigned int st_pclk_media2_cfg         :  1;
        unsigned int st_clk_debug_apb_xctrl_ddr :  1;
        unsigned int reserved_3                 :  1;
        unsigned int st_clk_debug_trace         :  1;
        unsigned int st_clk_xctrl_ddr           :  1;
        unsigned int reserved_4                 :  2;
    } reg;
}peri_crg_clkst8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_cpu_bus        :  1;
        unsigned int gt_clk_xctrl_cpu      :  1;
        unsigned int reserved_0            :  9;
        unsigned int gt_pclk_crg_npu       :  1;
        unsigned int reserved_1            :  2;
        unsigned int gt_clk_gpll_gpu       :  1;
        unsigned int gt_clk_gpll_cpu       :  1;
        unsigned int gt_clk_gpll_up        :  1;
        unsigned int gt_pclk_crg_core      :  1;
        unsigned int gt_pclk_crg_xctrl_ddr :  1;
        unsigned int gt_pclk_crg_gpu       :  1;
        unsigned int gt_pclk_crg_hss2      :  1;
        unsigned int gt_pclk_crg_cpu       :  1;
        unsigned int gt_clk_hss1           :  1;
        unsigned int gt_clk_hss2           :  1;
        unsigned int reserved_2            :  1;
        unsigned int gt_clk_xrse           :  1;
        unsigned int gt_pclk_crg_hss1      :  1;
        unsigned int gt_pclk_crg_media2    :  1;
        unsigned int gt_pclk_crg_media1    :  1;
        unsigned int gt_clk_ddr_dficlk     :  1;
        unsigned int gt_clk_peri2npu_noc   :  1;
        unsigned int gt_clk_peri2npu_nic   :  1;
    } reg;
}peri_crg_clkgt9_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_cpu_bus        :  1;
        unsigned int gt_clk_xctrl_cpu      :  1;
        unsigned int reserved_0            :  9;
        unsigned int gt_pclk_crg_npu       :  1;
        unsigned int reserved_1            :  2;
        unsigned int gt_clk_gpll_gpu       :  1;
        unsigned int gt_clk_gpll_cpu       :  1;
        unsigned int gt_clk_gpll_up        :  1;
        unsigned int gt_pclk_crg_core      :  1;
        unsigned int gt_pclk_crg_xctrl_ddr :  1;
        unsigned int gt_pclk_crg_gpu       :  1;
        unsigned int gt_pclk_crg_hss2      :  1;
        unsigned int gt_pclk_crg_cpu       :  1;
        unsigned int gt_clk_hss1           :  1;
        unsigned int gt_clk_hss2           :  1;
        unsigned int reserved_2            :  1;
        unsigned int gt_clk_xrse           :  1;
        unsigned int gt_pclk_crg_hss1      :  1;
        unsigned int gt_pclk_crg_media2    :  1;
        unsigned int gt_pclk_crg_media1    :  1;
        unsigned int gt_clk_ddr_dficlk     :  1;
        unsigned int gt_clk_peri2npu_noc   :  1;
        unsigned int gt_clk_peri2npu_nic   :  1;
    } reg;
}peri_crg_clkgt9_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_cpu_bus        :  1;
        unsigned int gt_clk_xctrl_cpu      :  1;
        unsigned int reserved_0            :  9;
        unsigned int gt_pclk_crg_npu       :  1;
        unsigned int reserved_1            :  2;
        unsigned int gt_clk_gpll_gpu       :  1;
        unsigned int gt_clk_gpll_cpu       :  1;
        unsigned int gt_clk_gpll_up        :  1;
        unsigned int gt_pclk_crg_core      :  1;
        unsigned int gt_pclk_crg_xctrl_ddr :  1;
        unsigned int gt_pclk_crg_gpu       :  1;
        unsigned int gt_pclk_crg_hss2      :  1;
        unsigned int gt_pclk_crg_cpu       :  1;
        unsigned int gt_clk_hss1           :  1;
        unsigned int gt_clk_hss2           :  1;
        unsigned int reserved_2            :  1;
        unsigned int gt_clk_xrse           :  1;
        unsigned int gt_pclk_crg_hss1      :  1;
        unsigned int gt_pclk_crg_media2    :  1;
        unsigned int gt_pclk_crg_media1    :  1;
        unsigned int gt_clk_ddr_dficlk     :  1;
        unsigned int gt_clk_peri2npu_noc   :  1;
        unsigned int gt_clk_peri2npu_nic   :  1;
    } reg;
}peri_crg_clkgt9_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_cpu_bus        :  1;
        unsigned int st_clk_xctrl_cpu      :  1;
        unsigned int reserved_0            :  9;
        unsigned int st_pclk_crg_npu       :  1;
        unsigned int reserved_1            :  2;
        unsigned int st_clk_gpll_gpu       :  1;
        unsigned int st_clk_gpll_cpu       :  1;
        unsigned int st_clk_gpll_up        :  1;
        unsigned int st_pclk_crg_core      :  1;
        unsigned int st_pclk_crg_xctrl_ddr :  1;
        unsigned int st_pclk_crg_gpu       :  1;
        unsigned int st_pclk_crg_hss2      :  1;
        unsigned int st_pclk_crg_cpu       :  1;
        unsigned int st_clk_hss1           :  1;
        unsigned int st_clk_hss2           :  1;
        unsigned int reserved_2            :  1;
        unsigned int st_clk_xrse           :  1;
        unsigned int st_pclk_crg_hss1      :  1;
        unsigned int st_pclk_crg_media2    :  1;
        unsigned int st_pclk_crg_media1    :  1;
        unsigned int st_clk_ddr_dficlk     :  1;
        unsigned int st_clk_peri2npu_noc   :  1;
        unsigned int st_clk_peri2npu_nic   :  1;
    } reg;
}peri_crg_clkst9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_cfgbus_lpis    :  1;
        unsigned int gt_clk_cfgbus_hss2    :  1;
        unsigned int reserved_0            :  1;
        unsigned int gt_clk_cfgbus_media2  :  1;
        unsigned int gt_clk_cfgbus_cpu     :  1;
        unsigned int gt_clk_cfgbus_npu     :  1;
        unsigned int reserved_1            :  4;
        unsigned int gt_clk_cfgbus_hss1    :  1;
        unsigned int reserved_2            :  1;
        unsigned int gt_clk_cfgbus_gpu     :  1;
        unsigned int gt_clk_cfgbus_xrse    :  1;
        unsigned int gt_clk_sysbus_hss2    :  1;
        unsigned int gt_clk_sysbus_cpu     :  1;
        unsigned int reserved_3            :  1;
        unsigned int gt_clk_sysbus_lpis    :  1;
        unsigned int gt_clk_sysbus_hss1    :  1;
        unsigned int gt_clk_sysbus_xrse    :  1;
        unsigned int gt_clk_mainbus_cpu    :  1;
        unsigned int gt_clk_mainbus_npu    :  1;
        unsigned int gt_clk_mainbus_media2 :  1;
        unsigned int gt_clk_mainbus_pmpu   :  1;
        unsigned int reserved_4            :  3;
        unsigned int gt_clk_mainbus_gpu    :  1;
        unsigned int gt_clk_mainbus_media1 :  1;
        unsigned int reserved_5            :  3;
    } reg;
}peri_crg_clkgt10_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_cfgbus_lpis    :  1;
        unsigned int gt_clk_cfgbus_hss2    :  1;
        unsigned int reserved_0            :  1;
        unsigned int gt_clk_cfgbus_media2  :  1;
        unsigned int gt_clk_cfgbus_cpu     :  1;
        unsigned int gt_clk_cfgbus_npu     :  1;
        unsigned int reserved_1            :  4;
        unsigned int gt_clk_cfgbus_hss1    :  1;
        unsigned int reserved_2            :  1;
        unsigned int gt_clk_cfgbus_gpu     :  1;
        unsigned int gt_clk_cfgbus_xrse    :  1;
        unsigned int gt_clk_sysbus_hss2    :  1;
        unsigned int gt_clk_sysbus_cpu     :  1;
        unsigned int reserved_3            :  1;
        unsigned int gt_clk_sysbus_lpis    :  1;
        unsigned int gt_clk_sysbus_hss1    :  1;
        unsigned int gt_clk_sysbus_xrse    :  1;
        unsigned int gt_clk_mainbus_cpu    :  1;
        unsigned int gt_clk_mainbus_npu    :  1;
        unsigned int gt_clk_mainbus_media2 :  1;
        unsigned int gt_clk_mainbus_pmpu   :  1;
        unsigned int reserved_4            :  3;
        unsigned int gt_clk_mainbus_gpu    :  1;
        unsigned int gt_clk_mainbus_media1 :  1;
        unsigned int reserved_5            :  3;
    } reg;
}peri_crg_clkgt10_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_cfgbus_lpis    :  1;
        unsigned int gt_clk_cfgbus_hss2    :  1;
        unsigned int reserved_0            :  1;
        unsigned int gt_clk_cfgbus_media2  :  1;
        unsigned int gt_clk_cfgbus_cpu     :  1;
        unsigned int gt_clk_cfgbus_npu     :  1;
        unsigned int reserved_1            :  4;
        unsigned int gt_clk_cfgbus_hss1    :  1;
        unsigned int reserved_2            :  1;
        unsigned int gt_clk_cfgbus_gpu     :  1;
        unsigned int gt_clk_cfgbus_xrse    :  1;
        unsigned int gt_clk_sysbus_hss2    :  1;
        unsigned int gt_clk_sysbus_cpu     :  1;
        unsigned int reserved_3            :  1;
        unsigned int gt_clk_sysbus_lpis    :  1;
        unsigned int gt_clk_sysbus_hss1    :  1;
        unsigned int gt_clk_sysbus_xrse    :  1;
        unsigned int gt_clk_mainbus_cpu    :  1;
        unsigned int gt_clk_mainbus_npu    :  1;
        unsigned int gt_clk_mainbus_media2 :  1;
        unsigned int gt_clk_mainbus_pmpu   :  1;
        unsigned int reserved_4            :  3;
        unsigned int gt_clk_mainbus_gpu    :  1;
        unsigned int gt_clk_mainbus_media1 :  1;
        unsigned int reserved_5            :  3;
    } reg;
}peri_crg_clkgt10_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_cfgbus_lpis    :  1;
        unsigned int st_clk_cfgbus_hss2    :  1;
        unsigned int reserved_0            :  1;
        unsigned int st_clk_cfgbus_media2  :  1;
        unsigned int st_clk_cfgbus_cpu     :  1;
        unsigned int st_clk_cfgbus_npu     :  1;
        unsigned int reserved_1            :  4;
        unsigned int st_clk_cfgbus_hss1    :  1;
        unsigned int reserved_2            :  1;
        unsigned int st_clk_cfgbus_gpu     :  1;
        unsigned int st_clk_cfgbus_xrse    :  1;
        unsigned int st_clk_sysbus_hss2    :  1;
        unsigned int st_clk_sysbus_cpu     :  1;
        unsigned int reserved_3            :  1;
        unsigned int st_clk_sysbus_lpis    :  1;
        unsigned int st_clk_sysbus_hss1    :  1;
        unsigned int st_clk_sysbus_xrse    :  1;
        unsigned int st_clk_mainbus_cpu    :  1;
        unsigned int st_clk_mainbus_npu    :  1;
        unsigned int st_clk_mainbus_media2 :  1;
        unsigned int st_clk_mainbus_pmpua  :  1;
        unsigned int st_clk_mainbus_pmpub  :  1;
        unsigned int st_clk_mainbus_pmpuc  :  1;
        unsigned int st_clk_mainbus_pmpud  :  1;
        unsigned int st_clk_mainbus_gpu    :  1;
        unsigned int st_clk_mainbus_media1 :  1;
        unsigned int reserved_4            :  3;
    } reg;
}peri_crg_clkst10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_lpisbus2sysbus_ndb_mst_n :  1;
        unsigned int ip_rst_lpctrl2ddrcfg_ndb_mst_n  :  1;
        unsigned int reserved_0                      :  5;
        unsigned int ip_prst_peri_wdt2_n             :  1;
        unsigned int ip_prst_peri_wdt3_n             :  1;
        unsigned int ip_prst_spi0_n                  :  1;
        unsigned int ip_prst_spi1_n                  :  1;
        unsigned int ip_prst_spi2_n                  :  1;
        unsigned int ip_rst_peri_timer4_n            :  1;
        unsigned int ip_rst_peri_timer5_n            :  1;
        unsigned int ip_rst_peri_timer6_n            :  1;
        unsigned int ip_rst_peri_timer7_n            :  1;
        unsigned int ip_prst_peri_ipc0_n             :  1;
        unsigned int reserved_1                      : 15;
    } reg;
}peri_crg_rst3_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_lpisbus2sysbus_ndb_mst_n :  1;
        unsigned int ip_rst_lpctrl2ddrcfg_ndb_mst_n  :  1;
        unsigned int reserved_0                      :  5;
        unsigned int ip_prst_peri_wdt2_n             :  1;
        unsigned int ip_prst_peri_wdt3_n             :  1;
        unsigned int ip_prst_spi0_n                  :  1;
        unsigned int ip_prst_spi1_n                  :  1;
        unsigned int ip_prst_spi2_n                  :  1;
        unsigned int ip_rst_peri_timer4_n            :  1;
        unsigned int ip_rst_peri_timer5_n            :  1;
        unsigned int ip_rst_peri_timer6_n            :  1;
        unsigned int ip_rst_peri_timer7_n            :  1;
        unsigned int ip_prst_peri_ipc0_n             :  1;
        unsigned int reserved_1                      : 15;
    } reg;
}peri_crg_rst3_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_lpisbus2sysbus_ndb_mst_n :  1;
        unsigned int ip_rst_lpctrl2ddrcfg_ndb_mst_n  :  1;
        unsigned int reserved_0                      :  5;
        unsigned int ip_prst_peri_wdt2_n             :  1;
        unsigned int ip_prst_peri_wdt3_n             :  1;
        unsigned int ip_prst_spi0_n                  :  1;
        unsigned int ip_prst_spi1_n                  :  1;
        unsigned int ip_prst_spi2_n                  :  1;
        unsigned int ip_rst_peri_timer4_n            :  1;
        unsigned int ip_rst_peri_timer5_n            :  1;
        unsigned int ip_rst_peri_timer6_n            :  1;
        unsigned int ip_rst_peri_timer7_n            :  1;
        unsigned int ip_prst_peri_ipc0_n             :  1;
        unsigned int reserved_1                      : 15;
    } reg;
}peri_crg_rst3_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                :  3;
        unsigned int ip_prst_peri_gpio_n       :  1;
        unsigned int reserved_1                :  5;
        unsigned int ip_prst_pctrl_n           :  1;
        unsigned int reserved_2                :  5;
        unsigned int ip_prst_peri_ioctrl_n     :  1;
        unsigned int ip_prst_peril_iopadwrap_n :  1;
        unsigned int ip_prst_perir_iopadwrap_n :  1;
        unsigned int reserved_3                :  4;
        unsigned int ip_rst_onewire0_n         :  1;
        unsigned int ip_rst_onewire1_n         :  1;
        unsigned int ip_rst_onewire2_n         :  1;
        unsigned int ip_prst_onewire0_n        :  1;
        unsigned int ip_prst_onewire1_n        :  1;
        unsigned int ip_prst_onewire2_n        :  1;
        unsigned int reserved_4                :  4;
    } reg;
}peri_crg_rst4_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                :  3;
        unsigned int ip_prst_peri_gpio_n       :  1;
        unsigned int reserved_1                :  5;
        unsigned int ip_prst_pctrl_n           :  1;
        unsigned int reserved_2                :  5;
        unsigned int ip_prst_peri_ioctrl_n     :  1;
        unsigned int ip_prst_peril_iopadwrap_n :  1;
        unsigned int ip_prst_perir_iopadwrap_n :  1;
        unsigned int reserved_3                :  4;
        unsigned int ip_rst_onewire0_n         :  1;
        unsigned int ip_rst_onewire1_n         :  1;
        unsigned int ip_rst_onewire2_n         :  1;
        unsigned int ip_prst_onewire0_n        :  1;
        unsigned int ip_prst_onewire1_n        :  1;
        unsigned int ip_prst_onewire2_n        :  1;
        unsigned int reserved_4                :  4;
    } reg;
}peri_crg_rst4_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                :  3;
        unsigned int ip_prst_peri_gpio_n       :  1;
        unsigned int reserved_1                :  5;
        unsigned int ip_prst_pctrl_n           :  1;
        unsigned int reserved_2                :  5;
        unsigned int ip_prst_peri_ioctrl_n     :  1;
        unsigned int ip_prst_peril_iopadwrap_n :  1;
        unsigned int ip_prst_perir_iopadwrap_n :  1;
        unsigned int reserved_3                :  4;
        unsigned int ip_rst_onewire0_n         :  1;
        unsigned int ip_rst_onewire1_n         :  1;
        unsigned int ip_rst_onewire2_n         :  1;
        unsigned int ip_prst_onewire0_n        :  1;
        unsigned int ip_prst_onewire1_n        :  1;
        unsigned int ip_prst_onewire2_n        :  1;
        unsigned int reserved_4                :  4;
    } reg;
}peri_crg_rst4_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_fc_n            :  1;
        unsigned int ip_rst_sys1_tpc_n      :  1;
        unsigned int ip_rst_npu_tpc_n       :  1;
        unsigned int reserved_0             :  5;
        unsigned int ip_rst_peri_gpiodb_n   :  1;
        unsigned int reserved_1             :  2;
        unsigned int ip_prst_peri_timer_s_n :  1;
        unsigned int reserved_2             :  2;
        unsigned int ip_rst_peri_wdt2_n     :  1;
        unsigned int ip_rst_peri_wdt3_n     :  1;
        unsigned int ip_rst_spi0_n          :  1;
        unsigned int ip_rst_spi1_n          :  1;
        unsigned int ip_rst_spi2_n          :  1;
        unsigned int reserved_3             : 13;
    } reg;
}peri_crg_rst5_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_fc_n            :  1;
        unsigned int ip_rst_sys1_tpc_n      :  1;
        unsigned int ip_rst_npu_tpc_n       :  1;
        unsigned int reserved_0             :  5;
        unsigned int ip_rst_peri_gpiodb_n   :  1;
        unsigned int reserved_1             :  2;
        unsigned int ip_prst_peri_timer_s_n :  1;
        unsigned int reserved_2             :  2;
        unsigned int ip_rst_peri_wdt2_n     :  1;
        unsigned int ip_rst_peri_wdt3_n     :  1;
        unsigned int ip_rst_spi0_n          :  1;
        unsigned int ip_rst_spi1_n          :  1;
        unsigned int ip_rst_spi2_n          :  1;
        unsigned int reserved_3             : 13;
    } reg;
}peri_crg_rst5_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_fc_n            :  1;
        unsigned int ip_rst_sys1_tpc_n      :  1;
        unsigned int ip_rst_npu_tpc_n       :  1;
        unsigned int reserved_0             :  5;
        unsigned int ip_rst_peri_gpiodb_n   :  1;
        unsigned int reserved_1             :  2;
        unsigned int ip_prst_peri_timer_s_n :  1;
        unsigned int reserved_2             :  2;
        unsigned int ip_rst_peri_wdt2_n     :  1;
        unsigned int ip_rst_peri_wdt3_n     :  1;
        unsigned int ip_rst_spi0_n          :  1;
        unsigned int ip_rst_spi1_n          :  1;
        unsigned int ip_rst_spi2_n          :  1;
        unsigned int reserved_3             : 13;
    } reg;
}peri_crg_rst5_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_ppll0_logic_n    :  1;
        unsigned int ip_rst_ppll1_logic_n    :  1;
        unsigned int ip_rst_ppll2_logic_n    :  1;
        unsigned int ip_rst_ppll3_logic_n    :  1;
        unsigned int ip_rst_gpll_logic_n     :  1;
        unsigned int ip_rst_ddrppll0_logic_n :  1;
        unsigned int ip_rst_ddrppll1_logic_n :  1;
        unsigned int ip_rst_ssmod_ppll0_n    :  1;
        unsigned int ip_rst_ssmod_ppll1_n    :  1;
        unsigned int ip_rst_ssmod_ppll2_n    :  1;
        unsigned int ip_rst_ssmod_ppll3_n    :  1;
        unsigned int ip_rst_ssmod_gpll_n     :  1;
        unsigned int ip_rst_ssmod_ddrppll0_n :  1;
        unsigned int ip_rst_ssmod_ddrppll1_n :  1;
        unsigned int reserved_0              :  7;
        unsigned int ip_rst_broadcast_n      :  1;
        unsigned int reserved_1              :  2;
        unsigned int ip_rst_pmpu_ddr_n       :  1;
        unsigned int reserved_2              :  7;
    } reg;
}peri_crg_rst6_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_ppll0_logic_n    :  1;
        unsigned int ip_rst_ppll1_logic_n    :  1;
        unsigned int ip_rst_ppll2_logic_n    :  1;
        unsigned int ip_rst_ppll3_logic_n    :  1;
        unsigned int ip_rst_gpll_logic_n     :  1;
        unsigned int ip_rst_ddrppll0_logic_n :  1;
        unsigned int ip_rst_ddrppll1_logic_n :  1;
        unsigned int ip_rst_ssmod_ppll0_n    :  1;
        unsigned int ip_rst_ssmod_ppll1_n    :  1;
        unsigned int ip_rst_ssmod_ppll2_n    :  1;
        unsigned int ip_rst_ssmod_ppll3_n    :  1;
        unsigned int ip_rst_ssmod_gpll_n     :  1;
        unsigned int ip_rst_ssmod_ddrppll0_n :  1;
        unsigned int ip_rst_ssmod_ddrppll1_n :  1;
        unsigned int reserved_0              :  7;
        unsigned int ip_rst_broadcast_n      :  1;
        unsigned int reserved_1              :  2;
        unsigned int ip_rst_pmpu_ddr_n       :  1;
        unsigned int reserved_2              :  7;
    } reg;
}peri_crg_rst6_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_ppll0_logic_n    :  1;
        unsigned int ip_rst_ppll1_logic_n    :  1;
        unsigned int ip_rst_ppll2_logic_n    :  1;
        unsigned int ip_rst_ppll3_logic_n    :  1;
        unsigned int ip_rst_gpll_logic_n     :  1;
        unsigned int ip_rst_ddrppll0_logic_n :  1;
        unsigned int ip_rst_ddrppll1_logic_n :  1;
        unsigned int ip_rst_ssmod_ppll0_n    :  1;
        unsigned int ip_rst_ssmod_ppll1_n    :  1;
        unsigned int ip_rst_ssmod_ppll2_n    :  1;
        unsigned int ip_rst_ssmod_ppll3_n    :  1;
        unsigned int ip_rst_ssmod_gpll_n     :  1;
        unsigned int ip_rst_ssmod_ddrppll0_n :  1;
        unsigned int ip_rst_ssmod_ddrppll1_n :  1;
        unsigned int reserved_0              :  7;
        unsigned int ip_rst_broadcast_n      :  1;
        unsigned int reserved_1              :  2;
        unsigned int ip_rst_pmpu_ddr_n       :  1;
        unsigned int reserved_2              :  7;
    } reg;
}peri_crg_rst6_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0               :  3;
        unsigned int ip_rst_axibist_n         :  1;
        unsigned int ip_prst_dma_bus_n        :  1;
        unsigned int reserved_1               :  1;
        unsigned int ip_rst_dma_bus_n         :  1;
        unsigned int reserved_2               :  5;
        unsigned int ip_rst_debug_n           :  1;
        unsigned int ip_rst_ddr_adb400_slv_n  :  1;
        unsigned int reserved_3               :  2;
        unsigned int ip_rst_npu_ft_n          :  1;
        unsigned int ip_rst_mainbus_n         :  1;
        unsigned int ip_rst_sysbus_n          :  1;
        unsigned int ip_rst_cfgbus_n          :  1;
        unsigned int ip_rst_pericfg_bus_n     :  1;
        unsigned int ip_rst_npu_adb_mst_n     :  1;
        unsigned int ip_rst_npu_ocm_adb_mst_n :  1;
        unsigned int reserved_4               :  9;
    } reg;
}peri_crg_rst7_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0               :  3;
        unsigned int ip_rst_axibist_n         :  1;
        unsigned int ip_prst_dma_bus_n        :  1;
        unsigned int reserved_1               :  1;
        unsigned int ip_rst_dma_bus_n         :  1;
        unsigned int reserved_2               :  5;
        unsigned int ip_rst_debug_n           :  1;
        unsigned int ip_rst_ddr_adb400_slv_n  :  1;
        unsigned int reserved_3               :  2;
        unsigned int ip_rst_npu_ft_n          :  1;
        unsigned int ip_rst_mainbus_n         :  1;
        unsigned int ip_rst_sysbus_n          :  1;
        unsigned int ip_rst_cfgbus_n          :  1;
        unsigned int ip_rst_pericfg_bus_n     :  1;
        unsigned int ip_rst_npu_adb_mst_n     :  1;
        unsigned int ip_rst_npu_ocm_adb_mst_n :  1;
        unsigned int reserved_4               :  9;
    } reg;
}peri_crg_rst7_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0               :  3;
        unsigned int ip_rst_axibist_n         :  1;
        unsigned int ip_prst_dma_bus_n        :  1;
        unsigned int reserved_1               :  1;
        unsigned int ip_rst_dma_bus_n         :  1;
        unsigned int reserved_2               :  5;
        unsigned int ip_rst_debug_n           :  1;
        unsigned int ip_rst_ddr_adb400_slv_n  :  1;
        unsigned int reserved_3               :  2;
        unsigned int ip_rst_npu_ft_n          :  1;
        unsigned int ip_rst_mainbus_n         :  1;
        unsigned int ip_rst_sysbus_n          :  1;
        unsigned int ip_rst_cfgbus_n          :  1;
        unsigned int ip_rst_pericfg_bus_n     :  1;
        unsigned int ip_rst_npu_adb_mst_n     :  1;
        unsigned int ip_rst_npu_ocm_adb_mst_n :  1;
        unsigned int reserved_4               :  9;
    } reg;
}peri_crg_rst7_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_cpu_bus_n       :  1;
        unsigned int ip_rst_xctrl_ddr_n     :  1;
        unsigned int ip_rst_xctrl_cpu_n     :  1;
        unsigned int ip_rst_hss2_n          :  1;
        unsigned int ip_rst_hss1_n          :  1;
        unsigned int ip_rst_media2_n        :  1;
        unsigned int ip_rst_media1_n        :  1;
        unsigned int ip_rst_npu_n           :  1;
        unsigned int ip_rst_gpu_n           :  1;
        unsigned int ip_rst_cpu_n           :  1;
        unsigned int ip_rst_xctrl_ddr_crg_n :  1;
        unsigned int ip_rst_xctrl_cpu_crg_n :  1;
        unsigned int ip_rst_hss2_crg_n      :  1;
        unsigned int ip_rst_hss1_crg_n      :  1;
        unsigned int reserved_0             :  1;
        unsigned int ip_rst_media2_crg_n    :  1;
        unsigned int ip_rst_media1_crg_n    :  1;
        unsigned int ip_rst_npu_crg_n       :  1;
        unsigned int ip_rst_gpu_crg_n       :  1;
        unsigned int ip_rst_cpu_crg_n       :  1;
        unsigned int reserved_1             :  2;
        unsigned int ip_rst_ddr_crg_n       :  1;
        unsigned int ip_rst_ddr_n           :  1;
        unsigned int reserved_2             :  5;
        unsigned int ip_rst_media2_ft_n     :  1;
        unsigned int ip_rst_media1_ft_n     :  1;
        unsigned int reserved_3             :  1;
    } reg;
}peri_crg_rst8_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_cpu_bus_n       :  1;
        unsigned int ip_rst_xctrl_ddr_n     :  1;
        unsigned int ip_rst_xctrl_cpu_n     :  1;
        unsigned int ip_rst_hss2_n          :  1;
        unsigned int ip_rst_hss1_n          :  1;
        unsigned int ip_rst_media2_n        :  1;
        unsigned int ip_rst_media1_n        :  1;
        unsigned int ip_rst_npu_n           :  1;
        unsigned int ip_rst_gpu_n           :  1;
        unsigned int ip_rst_cpu_n           :  1;
        unsigned int ip_rst_xctrl_ddr_crg_n :  1;
        unsigned int ip_rst_xctrl_cpu_crg_n :  1;
        unsigned int ip_rst_hss2_crg_n      :  1;
        unsigned int ip_rst_hss1_crg_n      :  1;
        unsigned int reserved_0             :  1;
        unsigned int ip_rst_media2_crg_n    :  1;
        unsigned int ip_rst_media1_crg_n    :  1;
        unsigned int ip_rst_npu_crg_n       :  1;
        unsigned int ip_rst_gpu_crg_n       :  1;
        unsigned int ip_rst_cpu_crg_n       :  1;
        unsigned int reserved_1             :  2;
        unsigned int ip_rst_ddr_crg_n       :  1;
        unsigned int ip_rst_ddr_n           :  1;
        unsigned int reserved_2             :  5;
        unsigned int ip_rst_media2_ft_n     :  1;
        unsigned int ip_rst_media1_ft_n     :  1;
        unsigned int reserved_3             :  1;
    } reg;
}peri_crg_rst8_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_cpu_bus_n       :  1;
        unsigned int ip_rst_xctrl_ddr_n     :  1;
        unsigned int ip_rst_xctrl_cpu_n     :  1;
        unsigned int ip_rst_hss2_n          :  1;
        unsigned int ip_rst_hss1_n          :  1;
        unsigned int ip_rst_media2_n        :  1;
        unsigned int ip_rst_media1_n        :  1;
        unsigned int ip_rst_npu_n           :  1;
        unsigned int ip_rst_gpu_n           :  1;
        unsigned int ip_rst_cpu_n           :  1;
        unsigned int ip_rst_xctrl_ddr_crg_n :  1;
        unsigned int ip_rst_xctrl_cpu_crg_n :  1;
        unsigned int ip_rst_hss2_crg_n      :  1;
        unsigned int ip_rst_hss1_crg_n      :  1;
        unsigned int reserved_0             :  1;
        unsigned int ip_rst_media2_crg_n    :  1;
        unsigned int ip_rst_media1_crg_n    :  1;
        unsigned int ip_rst_npu_crg_n       :  1;
        unsigned int ip_rst_gpu_crg_n       :  1;
        unsigned int ip_rst_cpu_crg_n       :  1;
        unsigned int reserved_1             :  2;
        unsigned int ip_rst_ddr_crg_n       :  1;
        unsigned int ip_rst_ddr_n           :  1;
        unsigned int reserved_2             :  5;
        unsigned int ip_rst_media2_ft_n     :  1;
        unsigned int ip_rst_media1_ft_n     :  1;
        unsigned int reserved_3             :  1;
    } reg;
}peri_crg_rst8_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_mainbus_data_lppll       :  6;
        unsigned int div_clk_mainbus_data_gpll        :  6;
        unsigned int sc_gt_clk_mainbus_data_lppll_div :  1;
        unsigned int sc_gt_clk_mainbus_data_gpll_div  :  1;
        unsigned int reserved_0                       :  2;
        unsigned int _bm_                             : 16;
    } reg;
}peri_crg_clkdiv5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_sysbus_data_lppll       :  6;
        unsigned int reserved_0                      :  6;
        unsigned int sc_gt_clk_sysbus_data_lppll_div :  1;
        unsigned int reserved_1                      :  3;
        unsigned int _bm_                            : 16;
    } reg;
}peri_crg_clkdiv6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_sysbus_data_gpll       :  6;
        unsigned int div_clk_cfgbus_gpll            :  6;
        unsigned int sc_gt_clk_sysbus_data_gpll_div :  1;
        unsigned int sc_gt_clk_cfgbus_gpll          :  1;
        unsigned int reserved_0                     :  2;
        unsigned int _bm_                           : 16;
    } reg;
}peri_crg_clkdiv7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_pericfg_bus    :  6;
        unsigned int div_clk_cfgbus_lppll   :  6;
        unsigned int sc_gt_clk_pericfg_bus  :  1;
        unsigned int sc_gt_clk_cfgbus_lppll :  1;
        unsigned int reserved_0             :  2;
        unsigned int _bm_                   : 16;
    } reg;
}peri_crg_clkdiv8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_spi0   :  6;
        unsigned int div_clk_spi1   :  6;
        unsigned int sc_gt_clk_spi0 :  1;
        unsigned int sc_gt_clk_spi1 :  1;
        unsigned int reserved_0     :  2;
        unsigned int _bm_           : 16;
    } reg;
}peri_crg_clkdiv9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_spi2   :  6;
        unsigned int reserved_0     :  6;
        unsigned int sc_gt_clk_spi2 :  1;
        unsigned int reserved_1     :  3;
        unsigned int _bm_           : 16;
    } reg;
}peri_crg_clkdiv10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_peri_gpiodb   :  6;
        unsigned int reserved_0            :  6;
        unsigned int sc_gt_clk_peri_gpiodb :  1;
        unsigned int reserved_1            :  3;
        unsigned int _bm_                  : 16;
    } reg;
}peri_crg_clkdiv11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_dma_bus_data_lppll       :  6;
        unsigned int div_clk_dma_bus_data_gpll        :  6;
        unsigned int sc_gt_clk_dma_bus_data_lppll_div :  1;
        unsigned int sc_gt_clk_dma_bus_data_gpll_div  :  1;
        unsigned int reserved_0                       :  2;
        unsigned int _bm_                             : 16;
    } reg;
}peri_crg_clkdiv13_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_debug_apb   :  6;
        unsigned int reserved_0          :  6;
        unsigned int sc_gt_clk_debug_apb :  1;
        unsigned int reserved_1          :  3;
        unsigned int _bm_                : 16;
    } reg;
}peri_crg_clkdiv17_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_debug_atb_h   :  6;
        unsigned int reserved_0            :  6;
        unsigned int sc_gt_clk_debug_atb_h :  1;
        unsigned int reserved_1            :  3;
        unsigned int _bm_                  : 16;
    } reg;
}peri_crg_clkdiv18_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_debug_trace   :  6;
        unsigned int div_clk_xctrl_ddr     :  6;
        unsigned int sc_gt_clk_debug_trace :  1;
        unsigned int sc_gt_clk_xctrl_ddr   :  1;
        unsigned int reserved_0            :  2;
        unsigned int _bm_                  : 16;
    } reg;
}peri_crg_clkdiv19_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_xctrl_cpu   :  6;
        unsigned int reserved_0          :  6;
        unsigned int sc_gt_clk_xctrl_cpu :  1;
        unsigned int reserved_1          :  3;
        unsigned int _bm_                : 16;
    } reg;
}peri_crg_clkdiv20_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_hss1_gpll        :  6;
        unsigned int div_clk_hss1_lppll       :  6;
        unsigned int sc_gt_clk_hss1_gpll_div  :  1;
        unsigned int sc_gt_clk_hss1_lppll_div :  1;
        unsigned int reserved_0               :  2;
        unsigned int _bm_                     : 16;
    } reg;
}peri_crg_clkdiv21_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_hss2_gpll        :  6;
        unsigned int div_clk_hss2_lppll       :  6;
        unsigned int sc_gt_clk_hss2_gpll_div  :  1;
        unsigned int sc_gt_clk_hss2_lppll_div :  1;
        unsigned int reserved_0               :  2;
        unsigned int _bm_                     : 16;
    } reg;
}peri_crg_clkdiv22_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_xrse_gpll       :  6;
        unsigned int reserved_0              :  6;
        unsigned int sc_gt_clk_xrse_gpll_div :  1;
        unsigned int reserved_1              :  3;
        unsigned int _bm_                    : 16;
    } reg;
}peri_crg_clkdiv24_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_cpu_bus   :  6;
        unsigned int reserved_0        :  6;
        unsigned int sc_gt_clk_cpu_bus :  1;
        unsigned int reserved_1        :  3;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_clkdiv27_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_clk_mainbus_data_gpll :  2;
        unsigned int sel_clk_mainbus_data      :  2;
        unsigned int sel_clk_sysbus_data_gpll  :  2;
        unsigned int sel_clk_sysbus_data       :  2;
        unsigned int reserved_0                :  2;
        unsigned int sel_clk_cfgbus            :  2;
        unsigned int sel_clk_peri_timer4       :  1;
        unsigned int sel_clk_peri_timer5       :  1;
        unsigned int sel_clk_peri_timer6       :  1;
        unsigned int sel_clk_peri_timer7       :  1;
        unsigned int _bm_                      : 16;
    } reg;
}peri_crg_clkdiv29_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_clk_gpiodb       :  1;
        unsigned int sel_clk_ip_sw        :  2;
        unsigned int reserved_0           :  1;
        unsigned int sel_clk_dma_bus_data :  2;
        unsigned int reserved_1           :  4;
        unsigned int sel_pclk_dma_bus     :  2;
        unsigned int reserved_2           :  4;
        unsigned int _bm_                 : 16;
    } reg;
}peri_crg_clkdiv30_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0          :  2;
        unsigned int sel_clk_debug_apb   :  2;
        unsigned int sel_clk_debug_atb_h :  2;
        unsigned int sel_clk_debug_trace :  2;
        unsigned int sel_clk_xctrl_ddr   :  2;
        unsigned int sel_clk_xctrl_cpu   :  2;
        unsigned int sel_pclk_crg_core   :  2;
        unsigned int sel_clk_cpu_bus     :  2;
        unsigned int _bm_                : 16;
    } reg;
}peri_crg_clkdiv31_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_clk_hss1       :  2;
        unsigned int sel_clk_hss2       :  2;
        unsigned int sel_clk_xrse       :  1;
        unsigned int reserved_0         :  1;
        unsigned int sel_clk_ddr_dficlk :  2;
        unsigned int reserved_1         :  8;
        unsigned int _bm_               : 16;
    } reg;
}peri_crg_clkdiv32_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_sctrl_clk_mainbus_data_bypass :  1;
        unsigned int sel_sctrl_clk_sysbus_data_bypass  :  1;
        unsigned int sel_sctrl_clk_dma_bus_data_bypass :  1;
        unsigned int sel_sctrl_clk_csi_sys_bypass      :  1;
        unsigned int sel_sctrl_clk_dsi_sys_bypass      :  1;
        unsigned int sel_sctrl_clk_xctrl_ddr_bypass    :  1;
        unsigned int sel_sctrl_clk_xctrl_cpu_bypass    :  1;
        unsigned int sel_sctrl_pclk_crg_core_bypass    :  1;
        unsigned int sel_sctrl_clk_hss1_bypass         :  1;
        unsigned int sel_sctrl_clk_hss2_bypass         :  1;
        unsigned int sel_sctrl_clk_cpu_bus_bypass      :  1;
        unsigned int sel_sctrl_pclk_dma_bus_bypass     :  1;
        unsigned int sel_sctrl_clk_cfgbus_bypass       :  1;
        unsigned int reserved_0                        : 19;
    } reg;
}peri_crg_perictrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autogt_sctrl_clk_mainbus_data_lppll_div_bypass :  1;
        unsigned int autogt_sctrl_clk_sysbus_data_lppll_div_bypass  :  1;
        unsigned int autogt_sctrl_clk_cfgbus_lppll_div_bypass       :  1;
        unsigned int autogt_sctrl_clk_dma_bus_data_lppll_div_bypass :  1;
        unsigned int autogt_sctrl_pclk_dma_bus_lppll_div_bypass     :  1;
        unsigned int autogt_sctrl_clk_hss1_lppll_div_bypass         :  1;
        unsigned int autogt_sctrl_clk_hss2_lppll_div_bypass         :  1;
        unsigned int autogt_sctrl_clk_gpll_lpis_bypass              :  1;
        unsigned int autogt_sctrl_clk_gpll_peri_bypass              :  1;
        unsigned int autogt_sctrl_clk_ddr_dficlk_bypass             :  1;
        unsigned int autogt_sctrl_pclk_crg_core_lppll_div_bypass    :  1;
        unsigned int reserved_0                                     : 21;
    } reg;
}peri_crg_perictrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clkrst_flag_peri_spinlock_bypass :  1;
        unsigned int reserved_0                       :  1;
        unsigned int clkrst_flag_peri_timer_ns_bypass :  1;
        unsigned int clkrst_flag_peri_timer_s_bypass  :  1;
        unsigned int clkrst_flag_uart6_bypass         :  1;
        unsigned int clkrst_flag_uart3_bypass         :  1;
        unsigned int clkrst_flag_mregion_peri_bypass  :  1;
        unsigned int clkrst_flag_peri_wdt0_bypass     :  1;
        unsigned int clkrst_flag_peri_wdt1_bypass     :  1;
        unsigned int clkrst_flag_peri_wdt2_bypass     :  1;
        unsigned int clkrst_flag_peri_wdt3_bypass     :  1;
        unsigned int clkrst_flag_peri_ipc0_bypass     :  1;
        unsigned int clkrst_flag_peri_ipc1_bypass     :  1;
        unsigned int clkrst_flag_peri_ipc2_bypass     :  1;
        unsigned int clkrst_flag_cpu_mregion_bypass   :  1;
        unsigned int clkrst_flag_gpu_mregion_bypass   :  1;
        unsigned int reserved_1                       : 16;
    } reg;
}peri_crg_perictrl2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clkrst_flag_pctrl_bypass :  1;
        unsigned int clkrst_flag_i2c0_bypass  :  1;
        unsigned int clkrst_flag_i2c1_bypass  :  1;
        unsigned int clkrst_flag_i2c2_bypass  :  1;
        unsigned int clkrst_flag_i2c3_bypass  :  1;
        unsigned int clkrst_flag_i2c4_bypass  :  1;
        unsigned int clkrst_flag_i2c5_bypass  :  1;
        unsigned int clkrst_flag_i2c6_bypass  :  1;
        unsigned int clkrst_flag_i2c9_bypass  :  1;
        unsigned int clkrst_flag_i2c10_bypass :  1;
        unsigned int clkrst_flag_i2c11_bypass :  1;
        unsigned int clkrst_flag_i2c12_bypass :  1;
        unsigned int clkrst_flag_i2c13_bypass :  1;
        unsigned int clkrst_flag_i2c20_bypass :  1;
        unsigned int reserved_0               : 18;
    } reg;
}peri_crg_perictrl3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clkrst_flag_spi0_bypass     :  1;
        unsigned int clkrst_flag_spi1_bypass     :  1;
        unsigned int clkrst_flag_spi2_bypass     :  1;
        unsigned int clkrst_flag_spi4_bypass     :  1;
        unsigned int clkrst_flag_spi5_bypass     :  1;
        unsigned int clkrst_flag_spi6_bypass     :  1;
        unsigned int clkrst_flag_i3c0_bypass     :  1;
        unsigned int clkrst_flag_pwm0_bypass     :  1;
        unsigned int clkrst_flag_pwm1_bypass     :  1;
        unsigned int clkrst_flag_onewire0_bypass :  1;
        unsigned int clkrst_flag_onewire1_bypass :  1;
        unsigned int clkrst_flag_onewire2_bypass :  1;
        unsigned int reserved_0                  : 20;
    } reg;
}peri_crg_perictrl4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clkrst_flag_peri_gpio_bypass       :  1;
        unsigned int clkrst_flag_peri_ioctrl_bypass     :  1;
        unsigned int clkrst_flag_perir_iopadwrap_bypass :  1;
        unsigned int clkrst_flag_peril_iopadwrap_bypass :  1;
        unsigned int clkrst_flag_dma_s_bypass           :  1;
        unsigned int clkrst_flag_dma_ns_bypass          :  1;
        unsigned int clkrst_flag_perf_monitor_bypass    :  1;
        unsigned int clkrst_flag_axi_bist_bypass        :  1;
        unsigned int clkrst_flag_mfw_xctrl_ddr_bypass   :  1;
        unsigned int reserved_0                         : 23;
    } reg;
}peri_crg_perictrl5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0           :  2;
        unsigned int sel_clk_peri2npu_nic :  2;
        unsigned int reserved_1           :  2;
        unsigned int sel_clk_peri2npu_noc :  2;
        unsigned int reserved_2           :  8;
        unsigned int _bm_                 : 16;
    } reg;
}peri_crg_clkdiv36_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_ppll0_logic    :  1;
        unsigned int gt_clk_ppll1_logic    :  1;
        unsigned int gt_clk_ppll2_logic    :  1;
        unsigned int gt_clk_ppll3_logic    :  1;
        unsigned int gt_clk_gpll_logic     :  1;
        unsigned int gt_clk_ddrppll0_logic :  1;
        unsigned int gt_clk_ddrppll1_logic :  1;
        unsigned int reserved_0            : 25;
    } reg;
}peri_crg_clkgt11_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_ppll0_logic    :  1;
        unsigned int gt_clk_ppll1_logic    :  1;
        unsigned int gt_clk_ppll2_logic    :  1;
        unsigned int gt_clk_ppll3_logic    :  1;
        unsigned int gt_clk_gpll_logic     :  1;
        unsigned int gt_clk_ddrppll0_logic :  1;
        unsigned int gt_clk_ddrppll1_logic :  1;
        unsigned int reserved_0            : 25;
    } reg;
}peri_crg_clkgt11_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_ppll0_logic    :  1;
        unsigned int gt_clk_ppll1_logic    :  1;
        unsigned int gt_clk_ppll2_logic    :  1;
        unsigned int gt_clk_ppll3_logic    :  1;
        unsigned int gt_clk_gpll_logic     :  1;
        unsigned int gt_clk_ddrppll0_logic :  1;
        unsigned int gt_clk_ddrppll1_logic :  1;
        unsigned int reserved_0            : 25;
    } reg;
}peri_crg_clkgt11_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_ppll0_logic    :  1;
        unsigned int st_clk_ppll1_logic    :  1;
        unsigned int st_clk_ppll2_logic    :  1;
        unsigned int st_clk_ppll3_logic    :  1;
        unsigned int st_clk_gpll_logic     :  1;
        unsigned int st_clk_ddrppll0_logic :  1;
        unsigned int st_clk_ddrppll1_logic :  1;
        unsigned int reserved_0            : 25;
    } reg;
}peri_crg_clkst11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_start_fsm_bypass :  1;
        unsigned int gpll_lock_timeout     :  9;
        unsigned int gpll_retry_num        :  3;
        unsigned int gpll_fsm_en           :  1;
        unsigned int gpll_cfg_rsv0         : 18;
    } reg;
}peri_crg_gpll_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_press_test_en  :  1;
        unsigned int gpll_press_test_cnt : 24;
        unsigned int gpll_cfg_rsv1       :  7;
    } reg;
}peri_crg_gpll_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_err_boot_cnt   : 24;
        unsigned int gpll_press_test_end :  1;
        unsigned int reserved_0          :  7;
    } reg;
}peri_crg_gpll_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_hw_vote_mask  :  8;
        unsigned int gpll_sft_vote_mask :  8;
        unsigned int gpll_cfg_rsv2      : 16;
    } reg;
}peri_crg_gpll_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_vote_bypass :  8;
        unsigned int reserved_0       :  8;
        unsigned int _bm_             : 16;
    } reg;
}peri_crg_gpll_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_vote_gt :  8;
        unsigned int reserved_0   :  8;
        unsigned int _bm_         : 16;
    } reg;
}peri_crg_gpll_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_ssmod_en   :  1;
        unsigned int gpll_divval     :  6;
        unsigned int gpll_spread     :  5;
        unsigned int gpll_downspread :  1;
        unsigned int gpll_cfg_rsv3   : 19;
    } reg;
}peri_crg_gpll_ssmod_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_en        :  1;
        unsigned int gpll_bypass    :  1;
        unsigned int gpll_refdiv    :  6;
        unsigned int gpll_fbdiv     : 12;
        unsigned int gpll_postdiv1  :  3;
        unsigned int gpll_postdiv2  :  3;
        unsigned int gpll_cfg_valid :  1;
        unsigned int gpll_gt        :  1;
        unsigned int gpll_cfg_rsv4  :  4;
    } reg;
}peri_crg_gpll_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_frac     : 24;
        unsigned int gpll_dsm_en   :  1;
        unsigned int gpll_cfg_rsv5 :  7;
    } reg;
}peri_crg_gpll_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_offsetcalbyp  :  1;
        unsigned int gpll_offsetcalcnt  :  3;
        unsigned int gpll_offsetcalen   :  1;
        unsigned int gpll_offsetcalin   : 12;
        unsigned int gpll_offsetcalrstn :  1;
        unsigned int gpll_offsetfastcal :  1;
        unsigned int gpll_cfg_rsv6      : 13;
    } reg;
}peri_crg_gpll_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gpll_en_state        :  1;
        unsigned int gpll_offsetcalout    : 12;
        unsigned int gpll_offsetcallock   :  1;
        unsigned int gpll_offsetcalovf    :  1;
        unsigned int gpll_start_fsm_state :  3;
        unsigned int gpll_real_retry_cnt  :  3;
        unsigned int st_clk_gpll          :  1;
        unsigned int reserved_0           : 10;
    } reg;
}peri_crg_gpll_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_start_fsm_bypass :  1;
        unsigned int ppll0_lock_timeout     :  9;
        unsigned int ppll0_retry_num        :  3;
        unsigned int ppll0_fsm_en           :  1;
        unsigned int ppll0_cfg_rsv0         : 18;
    } reg;
}peri_crg_ppll0_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_press_test_en  :  1;
        unsigned int ppll0_press_test_cnt : 24;
        unsigned int ppll0_cfg_rsv1       :  7;
    } reg;
}peri_crg_ppll0_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_err_boot_cnt   : 24;
        unsigned int ppll0_press_test_end :  1;
        unsigned int reserved_0           :  7;
    } reg;
}peri_crg_ppll0_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_hw_vote_mask  :  8;
        unsigned int ppll0_sft_vote_mask :  8;
        unsigned int ppll0_cfg_rsv2      : 16;
    } reg;
}peri_crg_ppll0_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_vote_bypass :  8;
        unsigned int reserved_0        :  8;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_ppll0_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_vote_gt :  8;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}peri_crg_ppll0_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_ssmod_en   :  1;
        unsigned int ppll0_divval     :  6;
        unsigned int ppll0_spread     :  5;
        unsigned int ppll0_downspread :  1;
        unsigned int ppll0_cfg_rsv3   : 19;
    } reg;
}peri_crg_ppll0_ssmod_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_en        :  1;
        unsigned int ppll0_bypass    :  1;
        unsigned int ppll0_refdiv    :  6;
        unsigned int ppll0_fbdiv     : 12;
        unsigned int ppll0_postdiv1  :  3;
        unsigned int ppll0_postdiv2  :  3;
        unsigned int ppll0_cfg_valid :  1;
        unsigned int ppll0_gt        :  1;
        unsigned int ppll0_cfg_rsv4  :  4;
    } reg;
}peri_crg_ppll0_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_frac     : 24;
        unsigned int ppll0_dsm_en   :  1;
        unsigned int ppll0_cfg_rsv5 :  7;
    } reg;
}peri_crg_ppll0_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_offsetcalbyp  :  1;
        unsigned int ppll0_offsetcalcnt  :  3;
        unsigned int ppll0_offsetcalen   :  1;
        unsigned int ppll0_offsetcalin   : 12;
        unsigned int ppll0_offsetcalrstn :  1;
        unsigned int ppll0_offsetfastcal :  1;
        unsigned int ppll0_cfg_rsv6      : 13;
    } reg;
}peri_crg_ppll0_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll0_en_state        :  1;
        unsigned int ppll0_offsetcalout    : 12;
        unsigned int ppll0_offsetcallock   :  1;
        unsigned int ppll0_offsetcalovf    :  1;
        unsigned int ppll0_start_fsm_state :  3;
        unsigned int ppll0_real_retry_cnt  :  3;
        unsigned int st_clk_ppll0          :  1;
        unsigned int reserved_0            : 10;
    } reg;
}peri_crg_ppll0_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_start_fsm_bypass :  1;
        unsigned int ppll1_lock_timeout     :  9;
        unsigned int ppll1_retry_num        :  3;
        unsigned int ppll1_fsm_en           :  1;
        unsigned int ppll1_cfg_rsv0         : 18;
    } reg;
}peri_crg_ppll1_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_press_test_en  :  1;
        unsigned int ppll1_press_test_cnt : 24;
        unsigned int ppll1_cfg_rsv1       :  7;
    } reg;
}peri_crg_ppll1_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_err_boot_cnt   : 24;
        unsigned int ppll1_press_test_end :  1;
        unsigned int reserved_0           :  7;
    } reg;
}peri_crg_ppll1_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_hw_vote_mask  :  8;
        unsigned int ppll1_sft_vote_mask :  8;
        unsigned int ppll1_cfg_rsv2      : 16;
    } reg;
}peri_crg_ppll1_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_vote_bypass :  8;
        unsigned int reserved_0        :  8;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_ppll1_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_vote_gt :  8;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}peri_crg_ppll1_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_ssmod_en   :  1;
        unsigned int ppll1_divval     :  6;
        unsigned int ppll1_spread     :  5;
        unsigned int ppll1_downspread :  1;
        unsigned int ppll1_cfg_rsv3   : 19;
    } reg;
}peri_crg_ppll1_ssmod_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_en        :  1;
        unsigned int ppll1_bypass    :  1;
        unsigned int ppll1_refdiv    :  6;
        unsigned int ppll1_fbdiv     : 12;
        unsigned int ppll1_postdiv1  :  3;
        unsigned int ppll1_postdiv2  :  3;
        unsigned int ppll1_cfg_valid :  1;
        unsigned int ppll1_gt        :  1;
        unsigned int ppll1_cfg_rsv4  :  4;
    } reg;
}peri_crg_ppll1_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_frac     : 24;
        unsigned int ppll1_dsm_en   :  1;
        unsigned int ppll1_cfg_rsv5 :  7;
    } reg;
}peri_crg_ppll1_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_offsetcalbyp  :  1;
        unsigned int ppll1_offsetcalcnt  :  3;
        unsigned int ppll1_offsetcalen   :  1;
        unsigned int ppll1_offsetcalin   : 12;
        unsigned int ppll1_offsetcalrstn :  1;
        unsigned int ppll1_offsetfastcal :  1;
        unsigned int ppll1_cfg_rsv6      : 13;
    } reg;
}peri_crg_ppll1_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll1_en_state        :  1;
        unsigned int ppll1_offsetcalout    : 12;
        unsigned int ppll1_offsetcallock   :  1;
        unsigned int ppll1_offsetcalovf    :  1;
        unsigned int ppll1_start_fsm_state :  3;
        unsigned int ppll1_real_retry_cnt  :  3;
        unsigned int st_clk_ppll1          :  1;
        unsigned int reserved_0            : 10;
    } reg;
}peri_crg_ppll1_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_start_fsm_bypass :  1;
        unsigned int ppll2_lock_timeout     :  9;
        unsigned int ppll2_retry_num        :  3;
        unsigned int ppll2_fsm_en           :  1;
        unsigned int ppll2_cfg_rsv0         : 18;
    } reg;
}peri_crg_ppll2_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_press_test_en  :  1;
        unsigned int ppll2_press_test_cnt : 24;
        unsigned int ppll2_cfg_rsv1       :  7;
    } reg;
}peri_crg_ppll2_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_err_boot_cnt   : 24;
        unsigned int ppll2_press_test_end :  1;
        unsigned int reserved_0           :  7;
    } reg;
}peri_crg_ppll2_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_hw_vote_mask  :  8;
        unsigned int ppll2_sft_vote_mask :  8;
        unsigned int ppll2_cfg_rsv2      : 16;
    } reg;
}peri_crg_ppll2_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_vote_bypass :  8;
        unsigned int reserved_0        :  8;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_ppll2_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_vote_gt :  8;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}peri_crg_ppll2_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_ssmod_en   :  1;
        unsigned int ppll2_divval     :  6;
        unsigned int ppll2_spread     :  5;
        unsigned int ppll2_downspread :  1;
        unsigned int ppll2_cfg_rsv3   : 19;
    } reg;
}peri_crg_ppll2_ssmod_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_en        :  1;
        unsigned int ppll2_bypass    :  1;
        unsigned int ppll2_refdiv    :  6;
        unsigned int ppll2_fbdiv     : 12;
        unsigned int ppll2_postdiv1  :  3;
        unsigned int ppll2_postdiv2  :  3;
        unsigned int ppll2_cfg_valid :  1;
        unsigned int ppll2_gt        :  1;
        unsigned int ppll2_cfg_rsv4  :  4;
    } reg;
}peri_crg_ppll2_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_frac     : 24;
        unsigned int ppll2_dsm_en   :  1;
        unsigned int ppll2_cfg_rsv5 :  7;
    } reg;
}peri_crg_ppll2_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_offsetcalbyp  :  1;
        unsigned int ppll2_offsetcalcnt  :  3;
        unsigned int ppll2_offsetcalen   :  1;
        unsigned int ppll2_offsetcalin   : 12;
        unsigned int ppll2_offsetcalrstn :  1;
        unsigned int ppll2_offsetfastcal :  1;
        unsigned int ppll2_cfg_rsv6      : 13;
    } reg;
}peri_crg_ppll2_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll2_en_state        :  1;
        unsigned int ppll2_offsetcalout    : 12;
        unsigned int ppll2_offsetcallock   :  1;
        unsigned int ppll2_offsetcalovf    :  1;
        unsigned int ppll2_start_fsm_state :  3;
        unsigned int ppll2_real_retry_cnt  :  3;
        unsigned int st_clk_ppll2          :  1;
        unsigned int reserved_0            : 10;
    } reg;
}peri_crg_ppll2_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_start_fsm_bypass :  1;
        unsigned int ppll3_lock_timeout     :  9;
        unsigned int ppll3_retry_num        :  3;
        unsigned int ppll3_fsm_en           :  1;
        unsigned int ppll3_cfg_rsv0         : 18;
    } reg;
}peri_crg_ppll3_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_press_test_en  :  1;
        unsigned int ppll3_press_test_cnt : 24;
        unsigned int ppll3_cfg_rsv1       :  7;
    } reg;
}peri_crg_ppll3_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_err_boot_cnt   : 24;
        unsigned int ppll3_press_test_end :  1;
        unsigned int reserved_0           :  7;
    } reg;
}peri_crg_ppll3_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_hw_vote_mask  :  8;
        unsigned int ppll3_sft_vote_mask :  8;
        unsigned int ppll3_cfg_rsv2      : 16;
    } reg;
}peri_crg_ppll3_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_vote_bypass :  8;
        unsigned int reserved_0        :  8;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_ppll3_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_vote_gt :  8;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}peri_crg_ppll3_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_ssmod_en   :  1;
        unsigned int ppll3_divval     :  6;
        unsigned int ppll3_spread     :  5;
        unsigned int ppll3_downspread :  1;
        unsigned int ppll3_cfg_rsv3   : 19;
    } reg;
}peri_crg_ppll3_ssmod_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_en        :  1;
        unsigned int ppll3_bypass    :  1;
        unsigned int ppll3_refdiv    :  6;
        unsigned int ppll3_fbdiv     : 12;
        unsigned int ppll3_postdiv1  :  3;
        unsigned int ppll3_postdiv2  :  3;
        unsigned int ppll3_cfg_valid :  1;
        unsigned int ppll3_gt        :  1;
        unsigned int ppll3_cfg_rsv4  :  4;
    } reg;
}peri_crg_ppll3_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_frac     : 24;
        unsigned int ppll3_dsm_en   :  1;
        unsigned int ppll3_cfg_rsv5 :  7;
    } reg;
}peri_crg_ppll3_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_offsetcalbyp  :  1;
        unsigned int ppll3_offsetcalcnt  :  3;
        unsigned int ppll3_offsetcalen   :  1;
        unsigned int ppll3_offsetcalin   : 12;
        unsigned int ppll3_offsetcalrstn :  1;
        unsigned int ppll3_offsetfastcal :  1;
        unsigned int ppll3_cfg_rsv6      : 13;
    } reg;
}peri_crg_ppll3_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ppll3_en_state        :  1;
        unsigned int ppll3_offsetcalout    : 12;
        unsigned int ppll3_offsetcallock   :  1;
        unsigned int ppll3_offsetcalovf    :  1;
        unsigned int ppll3_start_fsm_state :  3;
        unsigned int ppll3_real_retry_cnt  :  3;
        unsigned int st_clk_ppll3          :  1;
        unsigned int reserved_0            : 10;
    } reg;
}peri_crg_ppll3_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_start_fsm_bypass :  1;
        unsigned int ddr_ppll0_lock_timeout     :  9;
        unsigned int ddr_ppll0_retry_num        :  3;
        unsigned int ddr_ppll0_fsm_en           :  1;
        unsigned int ddr_ppll0_cfg_rsv0         : 18;
    } reg;
}peri_crg_ddr_ppll0_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_press_test_en  :  1;
        unsigned int ddr_ppll0_press_test_cnt : 24;
        unsigned int ddr_ppll0_cfg_rsv1       :  7;
    } reg;
}peri_crg_ddr_ppll0_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_err_boot_cnt   : 24;
        unsigned int ddr_ppll0_press_test_end :  1;
        unsigned int reserved_0               :  7;
    } reg;
}peri_crg_ddr_ppll0_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_hw_vote_mask  :  8;
        unsigned int ddr_ppll0_sft_vote_mask :  8;
        unsigned int ddr_ppll0_cfg_rsv2      : 16;
    } reg;
}peri_crg_ddr_ppll0_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_vote_bypass :  8;
        unsigned int reserved_0            :  8;
        unsigned int _bm_                  : 16;
    } reg;
}peri_crg_ddr_ppll0_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_vote_gt :  8;
        unsigned int reserved_0        :  8;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_ddr_ppll0_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_ssmod_en   :  1;
        unsigned int ddr_ppll0_divval     :  6;
        unsigned int ddr_ppll0_spread     :  5;
        unsigned int ddr_ppll0_downspread :  1;
        unsigned int ddr_ppll0_cfg_rsv3   : 19;
    } reg;
}peri_crg_ddr_ppll0_ssmod_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_en        :  1;
        unsigned int ddr_ppll0_bypass    :  1;
        unsigned int ddr_ppll0_refdiv    :  6;
        unsigned int ddr_ppll0_fbdiv     : 12;
        unsigned int ddr_ppll0_postdiv1  :  3;
        unsigned int ddr_ppll0_postdiv2  :  3;
        unsigned int ddr_ppll0_cfg_valid :  1;
        unsigned int ddr_ppll0_gt        :  1;
        unsigned int ddr_ppll0_cfg_rsv4  :  4;
    } reg;
}peri_crg_ddr_ppll0_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_frac     : 24;
        unsigned int ddr_ppll0_dsm_en   :  1;
        unsigned int ddr_ppll0_cfg_rsv5 :  7;
    } reg;
}peri_crg_ddr_ppll0_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_offsetcalbyp  :  1;
        unsigned int ddr_ppll0_offsetcalcnt  :  3;
        unsigned int ddr_ppll0_offsetcalen   :  1;
        unsigned int ddr_ppll0_offsetcalin   : 12;
        unsigned int ddr_ppll0_offsetcalrstn :  1;
        unsigned int ddr_ppll0_offsetfastcal :  1;
        unsigned int ddr_ppll0_cfg_rsv6      : 13;
    } reg;
}peri_crg_ddr_ppll0_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll0_en_state        :  1;
        unsigned int ddr_ppll0_offsetcalout    : 12;
        unsigned int ddr_ppll0_offsetcallock   :  1;
        unsigned int ddr_ppll0_offsetcalovf    :  1;
        unsigned int ddr_ppll0_start_fsm_state :  3;
        unsigned int ddr_ppll0_real_retry_cnt  :  3;
        unsigned int st_clk_ddr_ppll0          :  1;
        unsigned int reserved_0                : 10;
    } reg;
}peri_crg_ddr_ppll0_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_start_fsm_bypass :  1;
        unsigned int ddr_ppll1_lock_timeout     :  9;
        unsigned int ddr_ppll1_retry_num        :  3;
        unsigned int ddr_ppll1_fsm_en           :  1;
        unsigned int ddr_ppll1_cfg_rsv0         : 18;
    } reg;
}peri_crg_ddr_ppll1_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_press_test_en  :  1;
        unsigned int ddr_ppll1_press_test_cnt : 24;
        unsigned int ddr_ppll1_cfg_rsv1       :  7;
    } reg;
}peri_crg_ddr_ppll1_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_err_boot_cnt   : 24;
        unsigned int ddr_ppll1_press_test_end :  1;
        unsigned int reserved_0               :  7;
    } reg;
}peri_crg_ddr_ppll1_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_hw_vote_mask  :  8;
        unsigned int ddr_ppll1_sft_vote_mask :  8;
        unsigned int ddr_ppll1_cfg_rsv2      : 16;
    } reg;
}peri_crg_ddr_ppll1_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_vote_bypass :  8;
        unsigned int reserved_0            :  8;
        unsigned int _bm_                  : 16;
    } reg;
}peri_crg_ddr_ppll1_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_vote_gt :  8;
        unsigned int reserved_0        :  8;
        unsigned int _bm_              : 16;
    } reg;
}peri_crg_ddr_ppll1_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_ssmod_en   :  1;
        unsigned int ddr_ppll1_divval     :  6;
        unsigned int ddr_ppll1_spread     :  5;
        unsigned int ddr_ppll1_downspread :  1;
        unsigned int ddr_ppll1_cfg_rsv3   : 19;
    } reg;
}peri_crg_ddr_ppll1_ssmod_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_en        :  1;
        unsigned int ddr_ppll1_bypass    :  1;
        unsigned int ddr_ppll1_refdiv    :  6;
        unsigned int ddr_ppll1_fbdiv     : 12;
        unsigned int ddr_ppll1_postdiv1  :  3;
        unsigned int ddr_ppll1_postdiv2  :  3;
        unsigned int ddr_ppll1_cfg_valid :  1;
        unsigned int ddr_ppll1_gt        :  1;
        unsigned int ddr_ppll1_cfg_rsv4  :  4;
    } reg;
}peri_crg_ddr_ppll1_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_frac     : 24;
        unsigned int ddr_ppll1_dsm_en   :  1;
        unsigned int ddr_ppll1_cfg_rsv5 :  7;
    } reg;
}peri_crg_ddr_ppll1_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_offsetcalbyp  :  1;
        unsigned int ddr_ppll1_offsetcalcnt  :  3;
        unsigned int ddr_ppll1_offsetcalen   :  1;
        unsigned int ddr_ppll1_offsetcalin   : 12;
        unsigned int ddr_ppll1_offsetcalrstn :  1;
        unsigned int ddr_ppll1_offsetfastcal :  1;
        unsigned int ddr_ppll1_cfg_rsv6      : 13;
    } reg;
}peri_crg_ddr_ppll1_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_ppll1_en_state        :  1;
        unsigned int ddr_ppll1_offsetcalout    : 12;
        unsigned int ddr_ppll1_offsetcallock   :  1;
        unsigned int ddr_ppll1_offsetcalovf    :  1;
        unsigned int ddr_ppll1_start_fsm_state :  3;
        unsigned int ddr_ppll1_real_retry_cnt  :  3;
        unsigned int st_clk_ddr1_ppll1         :  1;
        unsigned int reserved_0                : 10;
    } reg;
}peri_crg_ddr_ppll1_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_pclk_dma_bus_lppll       :  6;
        unsigned int div_pclk_dma_bus_gpll        :  6;
        unsigned int sc_gt_pclk_dma_bus_lppll_div :  1;
        unsigned int sc_gt_pclk_dma_bus_gpll_div  :  1;
        unsigned int reserved_0                   :  2;
        unsigned int _bm_                         : 16;
    } reg;
}peri_crg_clkdiv33_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_pclk_crg_core_lppll   :  6;
        unsigned int div_pclk_crg_core_gpll    :  6;
        unsigned int sc_gt_pclk_crg_core_lppll :  1;
        unsigned int sc_gt_pclk_crg_core_gpll  :  1;
        unsigned int reserved_0                :  2;
        unsigned int _bm_                      : 16;
    } reg;
}peri_crg_clkdiv34_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_peri2npu_noc   :  6;
        unsigned int div_clk_peri2npu_nic   :  6;
        unsigned int sc_gt_clk_peri2npu_noc :  1;
        unsigned int sc_gt_clk_peri2npu_nic :  1;
        unsigned int reserved_0             :  2;
        unsigned int _bm_                   : 16;
    } reg;
}peri_crg_clkdiv35_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dma_bus_tgt_no_pending_trans_bypass :  4;
        unsigned int dma_bus_ini_no_pending_trans_bypass :  5;
        unsigned int idle_flag_dma_ns_bypass             :  1;
        unsigned int idle_flag_dma_s_bypass              :  1;
        unsigned int reserved_0                          : 21;
    } reg;
}peri_crg_autofsctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_dma_bus     : 10;
        unsigned int debounce_out_dma_bus    : 10;
        unsigned int debounce_bypass_dma_bus :  1;
        unsigned int reserved_0              : 10;
        unsigned int autofs_en_dma_bus       :  1;
    } reg;
}peri_crg_autofsctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_auto_clk_dma_bus_data_gpll  :  6;
        unsigned int div_auto_clk_dma_bus_data_lppll :  6;
        unsigned int reserved_0                      : 20;
    } reg;
}peri_crg_autofsctrl2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sys1_bus_tgt_no_pending_trans_bypass            :  2;
        unsigned int sys1_bus_ini_no_pending_trans_bypass            : 10;
        unsigned int sys0_bus_tgt_no_pending_trans_bypass            :  2;
        unsigned int sys0_bus_ini_no_pending_trans_bypass            :  3;
        unsigned int peri_broadcast_bus_no_pending_trans_bypass      :  1;
        unsigned int dma_bus_tgt_no_pending_trans_sysbus_bypass      :  1;
        unsigned int xrse_m85_bus_tgt_no_pending_trans_sysbus_bypass :  1;
        unsigned int hss2_bus_tgt_no_pending_trans_sysbus_bypass     :  1;
        unsigned int hss1_bus_tgt_no_pending_trans_sysbus_bypass     :  1;
        unsigned int lpis_bus_tgt_no_pending_trans_sysbus_bypass     :  1;
        unsigned int npu_bus_tgt_no_pending_trans_sysbus_bypass      :  1;
        unsigned int media2_bus_tgt_no_pending_trans_sysbus_bypass   :  1;
        unsigned int media1_bus_tgt_no_pending_trans_sysbus_bypass   :  1;
        unsigned int cpu_bus_tgt_no_pending_trans_sysbus_bypass      :  1;
        unsigned int xctrl_ddr_bus_no_pending_trans_sysbus_bypass    :  1;
        unsigned int idle_flag_cfgbus2ocm_nic_sysbus_bypass          :  1;
        unsigned int idle_flag_ocm_nic_sysbus_bypass                 :  1;
        unsigned int reserved_0                                      :  2;
    } reg;
}peri_crg_autofsctrl3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_sysbus     : 10;
        unsigned int debounce_out_sysbus    : 10;
        unsigned int debounce_bypass_sysbus :  1;
        unsigned int reserved_0             : 10;
        unsigned int autofs_en_sysbus       :  1;
    } reg;
}peri_crg_autofsctrl4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_auto_clk_sysbus_data_gpll  :  6;
        unsigned int div_auto_clk_sysbus_data_lppll :  6;
        unsigned int reserved_0                     : 20;
    } reg;
}peri_crg_autofsctrl5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pericfg_bus_tgt_no_pending_trans_bypass :  3;
        unsigned int pericfg_bus_ini_no_pending_trans_bypass :  1;
        unsigned int syscfg_bus_tgt_no_pending_trans_bypass  : 22;
        unsigned int reserved_0                              :  6;
    } reg;
}peri_crg_autofsctrl6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int syscfg_bus_ini_no_pending_trans_bypass          : 10;
        unsigned int xrse_m85_bus_tgt_no_pending_trans_cfgbus_bypass :  1;
        unsigned int media2_bus_tgt_no_pending_trans_cfgbus_bypass   :  1;
        unsigned int lpis_bus_tgt_no_pending_trans_cfgbus_bypass     :  1;
        unsigned int hss2_bus_tgt_no_pending_trans_cfgbus_bypass     :  1;
        unsigned int hss1_bus_tgt_no_pending_trans_cfgbus_bypass     :  1;
        unsigned int dma_bus_tgt_no_pending_trans_cfgbus_bypass      :  1;
        unsigned int cpu_bus_tgt_no_pending_trans_cfgbus_bypass      :  1;
        unsigned int idle_flag_npu_nic2cfgbus_bypass                 :  1;
        unsigned int reserved_0                                      : 14;
    } reg;
}peri_crg_autofsctrl7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_cfgbus     : 10;
        unsigned int debounce_out_cfgbus    : 10;
        unsigned int debounce_bypass_cfgbus :  1;
        unsigned int reserved_0             : 10;
        unsigned int autofs_en_cfgbus       :  1;
    } reg;
}peri_crg_autofsctrl8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_auto_clk_cfgbus_gpll  :  6;
        unsigned int div_auto_clk_cfgbus_lppll :  6;
        unsigned int reserved_0                : 20;
    } reg;
}peri_crg_autofsctrl9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0                                    : 12;
        unsigned int idle_flag_xctrl_cpu_m3_bypass                 :  1;
        unsigned int xctrl_cpu_bus_no_pending_trans_bypass         :  1;
        unsigned int cpu_bus_tgt_no_pending_trans_xctrl_cpu_bypass :  1;
        unsigned int reserved_1                                    : 17;
    } reg;
}peri_crg_autofsctrl10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_xctrl_cpu     : 10;
        unsigned int debounce_out_xctrl_cpu    : 10;
        unsigned int debounce_bypass_xctrl_cpu :  1;
        unsigned int div_auto_clk_xctrl_cpu    :  6;
        unsigned int reserved_0                :  4;
        unsigned int autofs_en_xctrl_cpu       :  1;
    } reg;
}peri_crg_autofsctrl11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int idle_flag_xctrl_ddr_m3_bypass                    :  1;
        unsigned int xctrl_ddr_bus_no_pending_trans_bypass            :  1;
        unsigned int syscfg_bus_tgt_no_pending_trans_xctrl_ddr_bypass :  1;
        unsigned int reserved_0                                       : 29;
    } reg;
}peri_crg_autofsctrl12_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_xctrl_ddr     : 10;
        unsigned int debounce_out_xctrl_ddr    : 10;
        unsigned int debounce_bypass_xctrl_ddr :  1;
        unsigned int div_auto_clk_xctrl_ddr    :  6;
        unsigned int reserved_0                :  4;
        unsigned int autofs_en_xctrl_ddr       :  1;
    } reg;
}peri_crg_autofsctrl13_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int peri_broadcast_bus_tgt_no_pending_trans_pmpu_bypass :  1;
        unsigned int main_bus_no_pending_trans_bypass                    :  1;
        unsigned int syscfg_bus_tgt_no_pending_trans_mainbus_bypass      :  1;
        unsigned int sys1_bus_tgt_no_pending_trans_mainbus_bypass        :  1;
        unsigned int media2_bus_tgt_no_pending_trans_bypass              :  2;
        unsigned int idle_flag_ndb_mst_m2_bypass                         :  1;
        unsigned int idle_flag_ndb_slv_m2_bypass                         :  1;
        unsigned int media1_bus_tgt_no_pending_trans_bypass              :  2;
        unsigned int idle_flag_ndb_mst_m1_bypass                         :  1;
        unsigned int idle_flag_ndb_slv_m1_bypass                         :  1;
        unsigned int npu_bus_tgt_no_pending_trans_bypass                 :  4;
        unsigned int idle_flag_adb_mst_npu_bypass                        :  1;
        unsigned int idle_flag_adb_slv_npu_bypass                        :  1;
        unsigned int idle_flag_adb_mst_gpu_bypass                        :  1;
        unsigned int idle_flag_adb_slv_gpu_bypass                        :  1;
        unsigned int idle_flag_main_adb_mst_cpu_bypass                   :  1;
        unsigned int idle_flag_main_adb_slv_cpu_bypass                   :  1;
        unsigned int idle_cpu_fast_wakeup_bypass                         :  4;
        unsigned int idle_flag_perfstat_peri_bypass                      :  1;
        unsigned int idle_flag_perfstat_gpu_bypass                       :  1;
        unsigned int idle_flag_perfstat_cpu_bypass                       :  1;
        unsigned int reserved_0                                          :  3;
    } reg;
}peri_crg_autofsctrl14_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_time_mainbus     : 10;
        unsigned int debounce_out_time_mainbus    : 10;
        unsigned int debounce_bypass_mainbus      :  1;
        unsigned int reserved_0                   :  9;
        unsigned int autofs_en_mainbus            :  1;
        unsigned int autogt_bypass_mainbus_lpctrl :  1;
    } reg;
}peri_crg_autofsctrl15_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_auto_clk_mainbus_gpll  :  6;
        unsigned int div_auto_clk_mainbus_lppll :  6;
        unsigned int reserved_0                 : 20;
    } reg;
}peri_crg_autofsctrl16_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_time_dficlk_off                        : 10;
        unsigned int debounce_out_time_dficlk_off                       : 10;
        unsigned int debounce_bypass_dficlk_off                         :  1;
        unsigned int mainbus_ctrl_dficlk_bypass                         :  1;
        unsigned int autogt_clk_ddr_dficlk_bypass                       :  1;
        unsigned int reserved_0                                         :  1;
        unsigned int peri_broadcast_bus_tgt_no_pending_trans_ddr_bypass :  4;
        unsigned int reserved_1                                         :  4;
    } reg;
}peri_crg_perictrl9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int autogt_clk_ddr_dficlk_vote_bypass : 16;
        unsigned int _bm_                              : 16;
    } reg;
}peri_crg_perictrl10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clkrst_flag_media1_ctrl_bypass      :  1;
        unsigned int clkrst_flag_media1_lpctrl_ft_bypass :  1;
        unsigned int clkrst_flag_media2_ctrl_bypass      :  1;
        unsigned int clkrst_flag_media2_lpctrl_ft_bypass :  1;
        unsigned int clkrst_flag_cpu_crg_bypass          :  1;
        unsigned int clkrst_flag_gpu_crg_bypass          :  1;
        unsigned int clkrst_flag_hss1_crg_bypass         :  1;
        unsigned int clkrst_flag_hss2_crg_bypass         :  1;
        unsigned int clkrst_flag_media1_crg_bypass       :  1;
        unsigned int clkrst_flag_media2_crg_bypass       :  1;
        unsigned int clkrst_flag_npu_crg_bypass          :  1;
        unsigned int clkrst_flag_media1_subsys_bypass    :  1;
        unsigned int clkrst_flag_media1_dbg_blk_bypass   :  1;
        unsigned int clkrst_flag_xctrl_ddr_crg_bypass    :  1;
        unsigned int reserved_0                          : 18;
    } reg;
}peri_crg_perictrl6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cpu_bus_tgt_no_pending_trans_bypass           :  7;
        unsigned int cpu_bus_ini_no_pending_trans_bypass           :  5;
        unsigned int cpu_wfx_bypass                                :  1;
        unsigned int syscfg_bus_tgt_no_pending_trans_cpubus_bypass :  1;
        unsigned int reserved_0                                    : 18;
    } reg;
}peri_crg_autofsctrl20_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int debounce_in_time_cpu_bus  : 10;
        unsigned int debounce_out_time_cpu_bus : 10;
        unsigned int debounce_bypass_cpu_bus   :  1;
        unsigned int div_auto_clk_cpu_bus      :  6;
        unsigned int reserved_0                :  4;
        unsigned int autofs_en_cpu_bus         :  1;
    } reg;
}peri_crg_autofsctrl21_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}peri_crg_regfile_cgbypass_t;

#endif
