// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_RESET_RESOURCE_H__
#define __XR_RESET_RESOURCE_H__

#define XR_RESET_ID_PERI  0
#define XR_RESET_ID_LPIS  1
#define XR_RESET_ID_LMS   2
#define XR_RESET_ID_CNT   3

#define XR_PERI_PRST_SPINLOCK   0
#define XR_PERI_PRST_TIMER_NS   1
#define XR_PERI_RST_TIMER0      2
#define XR_PERI_RST_TIMER1      3
#define XR_PERI_RST_TIMER2      4
#define XR_PERI_RST_TIMER3      5
#define XR_PERI_PRST_IPC1       6
#define XR_PERI_PRST_IPC2       7
#define XR_PERI_PRST_SPI4       8
#define XR_PERI_PRST_SPI5       9
#define XR_PERI_PRST_SPI6       10
#define XR_PERI_RST_SPI4        11
#define XR_PERI_RST_SPI5        12
#define XR_PERI_RST_SPI6        13
#define XR_PERI_PRST_I2C0       14
#define XR_PERI_PRST_I2C1       15
#define XR_PERI_PRST_I2C2       16
#define XR_PERI_PRST_I2C3       17
#define XR_PERI_PRST_I2C4       18
#define XR_PERI_PRST_I2C5       19
#define XR_PERI_PRST_I2C6       20
#define XR_PERI_PRST_I2C9       21
#define XR_PERI_PRST_I2C10      22
#define XR_PERI_PRST_I2C11      23
#define XR_PERI_PRST_I2C12      24
#define XR_PERI_PRST_I2C13      25
#define XR_PERI_PRST_I2C20      26
#define XR_PERI_RST_DMA_NS      27
#define XR_PERI_RST_DMA_S       28
#define XR_PERI_RST_DMA_NS_TPC  29
#define XR_PERI_RST_DMA_S_TPC   30
#define XR_PERI_RST_I2C0        31
#define XR_PERI_RST_I2C1        32
#define XR_PERI_RST_I2C2        33
#define XR_PERI_RST_I2C3        34
#define XR_PERI_RST_I2C4        35
#define XR_PERI_RST_I2C5        36
#define XR_PERI_RST_I2C6        37
#define XR_PERI_RST_I2C9        38
#define XR_PERI_RST_I2C10       39
#define XR_PERI_RST_I2C11       40
#define XR_PERI_RST_I2C12       41
#define XR_PERI_RST_I2C13       42
#define XR_PERI_RST_I2C20       43
#define XR_PERI_RST_I3C0        44
#define XR_PERI_PRST_I3C0       45
#define XR_PERI_RST_PWM0        46
#define XR_PERI_RST_PWM1        47
#define XR_PERI_RST_UART3       48
#define XR_PERI_RST_UART6       49


#define PERI_SEC_OFFSET         64
#define XR_PERI_PRST_SPI0           (PERI_SEC_OFFSET + 0)
#define XR_PERI_PRST_SPI1           (PERI_SEC_OFFSET + 1)
#define XR_PERI_PRST_SPI2           (PERI_SEC_OFFSET + 2)
#define XR_PERI_RST_TIMER4          (PERI_SEC_OFFSET + 3)
#define XR_PERI_RST_TIMER5          (PERI_SEC_OFFSET + 4)
#define XR_PERI_RST_TIMER6          (PERI_SEC_OFFSET + 5)
#define XR_PERI_RST_TIMER7          (PERI_SEC_OFFSET + 6)
#define XR_PERI_PRST_IPC0           (PERI_SEC_OFFSET + 7)
#define XR_PERI_PRST_GPIO           (PERI_SEC_OFFSET + 8)
#define XR_PERI_PRST_IOCTRL         (PERI_SEC_OFFSET + 9)
#define XR_PERI_PRST_PERIL_IOPAD    (PERI_SEC_OFFSET + 10)
#define XR_PERI_PRST_PERIR_IOPAD    (PERI_SEC_OFFSET + 11)
#define XR_PERI_RST_ONEWIRE0        (PERI_SEC_OFFSET + 12)
#define XR_PERI_RST_ONEWIRE1        (PERI_SEC_OFFSET + 13)
#define XR_PERI_RST_ONEWIRE2        (PERI_SEC_OFFSET + 14)
#define XR_PERI_PRST_ONEWIRE0       (PERI_SEC_OFFSET + 15)
#define XR_PERI_PRST_ONEWIRE1       (PERI_SEC_OFFSET + 16)
#define XR_PERI_PRST_ONEWIRE2       (PERI_SEC_OFFSET + 17)
#define XR_PERI_RST_GPIODB          (PERI_SEC_OFFSET + 18)
#define XR_PERI_PRST_TIMER_S        (PERI_SEC_OFFSET + 19)
#define XR_PERI_RST_SPI0            (PERI_SEC_OFFSET + 20)
#define XR_PERI_RST_SPI1            (PERI_SEC_OFFSET + 21)
#define XR_PERI_RST_SPI2            (PERI_SEC_OFFSET + 22)



#define XR_LPIS_PRST_IPC_NS     0
#define XR_LPIS_PRST_SPI_DMAC   1
#define XR_LPIS_RST_SPI_DMAC    2



#define XR_LMS_RST_RTC1      0
#define XR_LMS_PRST_RTC1     1
#define XR_LMS_PRST_TIMER2   2
#define XR_LMS_RST_TIMER60   3


#endif
