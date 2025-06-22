// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __O1_DW_APB_UART_REG_H__
#define __O1_DW_APB_UART_REG_H__

#define DW_CC_UART_RS485_INTERFACE_EN           0
#define DW_CC_UART_9BIT_DATA_EN                 0
#define DW_CC_UART_APB_DATA_WIDTH               32
#define DW_CC_UART_MAX_APB_DATA_WIDTH           32
#define DW_CC_UART_FIFO_MODE                    64
#define DW_CC_UART_FIFO_ACCESS                  0
#define DW_CC_UART_MEM_SELECT                   1
#define DW_CC_UART_MEM_MODE                     0
#define DW_CC_UART_CLOCK_MODE                   1
#define DW_CC_UART_AFCE_MODE                    0x1
#define DW_CC_UART_THRE_MODE                    0x1
#define DW_CC_UART_SIR_MODE                     0x0
#define DW_CC_UART_CLK_GATE_EN                  0
#define DW_CC_UART_DMA_EXTRA                    0x1
#define DW_CC_UART_DMA_POL                      0
#define DW_CC_UART_SIR_LP_MODE                  0x0
#define DW_CC_UART_DEBUG                        1
#define DW_CC_UART_BAUD_CLK                     1
#define DW_CC_UART_ADDITIONAL_FEATURES          0x1
#define DW_CC_UART_16550_COMPATIBLE             0
#define DW_CC_UART_FRACTIONAL_BAUD_DIVISOR_EN   1
#define DW_CC_UART_FIFO_STAT                    0x1
#define DW_CC_UART_SHADOW                       0x1
#define DW_CC_UART_ADD_ENCODED_PARAMS           0x1
#define DW_CC_UART_LATCH_MODE                   0
#define DW_CC_UART_ADDR_SLICE_LHS               8
#define DW_CC_UART_COMP_VERSION                 0x3430332a


#define DW_UART_RBR                         (0x0)
#define DW_UART_RBR_OFFSET                  0
#define DW_UART_RBR_SIZE                    8


#define DW_UART_THR                         (0x0)
#define DW_UART_THR_OFFSET                  0
#define DW_UART_THR_SIZE                    8


#define DW_UART_DLL                         (0x0)
#define DW_UART_DLL_OFFSET                  0
#define DW_UART_DLL_SIZE                    8


#define DW_UART_DLH                         (0x4)
#define DW_UART_DLH_OFFSET                  0
#define DW_UART_DLH_SIZE                    8

#define DW_UART_DIV_MAX                     0xffff
#define DW_UART_DIV_LEVEL                   16


#define DW_UART_IER                         (0x4)

#define DW_UART_IER_PTIME_OFFSET            7
#define DW_UART_IER_PTIME_SIZE              1
#define DW_UART_IER_PTIME_MASK              (0x100)

#define DW_UART_IER_EDSSI_OFFSET            3
#define DW_UART_IER_EDSSI_SIZE              1
#define DW_UART_IER_EDSSI_MASK              (0x8)

#define DW_UART_IER_ELSI_OFFSET             2
#define DW_UART_IER_ELSI_SIZE               1
#define DW_UART_IER_ELSI_MASK               (0x4)

#define DW_UART_IER_ETBEI_OFFSET            1
#define DW_UART_IER_ETBEI_SIZE              1
#define DW_UART_IER_ETBEI_MASK              (0x2)

#define DW_UART_IER_ERBFI_OFFSET            0
#define DW_UART_IER_ERBFI_SIZE              1
#define DW_UART_IER_ERBFI_MASK              0x1

#define DW_UART_IER_ALL_INTR_MASK           0x0f



#define DW_UART_IIR                         (0x8)

#define DW_UART_IIR_FIFOSE_OFFSET           6
#define DW_UART_IIR_FIFOSE_SIZE             2
#define DW_UART_IIR_FIFOSE_MASK             0xC0

#define DW_UART_IIR_IID_OFFSET              0
#define DW_UART_IIR_IID_SIZE                4
#define DW_UART_IIR_IID_MASK                0x0e

#define DW_UART_IIR_MASK                    0x3f
#define DW_UART_IIR_NO_INT                  0x1
#define DW_UART_IIR_MSI                     0x00
#define DW_UART_IIR_THRI                    0x02
#define DW_UART_IIR_RDI                     0x04
#define DW_UART_IIR_RLSI                    0x06
#define DW_UART_IIR_BUSY                    0x07
#define DW_UART_IIR_RX_TIMEOUT              0x0c
#define DW_UART_IIR_XOFFE                   0x10
#define DW_UART_IIR_CTS_RTS_DSR             0x20




#define DW_UART_FCR                         (0x8)

#define DW_UART_FCR_RT_OFFSET               6
#define DW_UART_FCR_RT_SIZE                 2
#define DW_UART_FCR_RT_MASK                 0xC0
#define DW_UART_FCR_RT_00                   0x00
#define DW_UART_FCR_RT_01                   0x40
#define DW_UART_FCR_RT_10                   0x80
#define DW_UART_FCR_RT_11                   0xc0
#define DW_UART_FCR_RT_BITS(x)              \
    (((x) & DW_UART_FCR_RT_MASK) >> DW_UART_FCR_RT_OFFSET)
#define DW_UART_FCR_RT_MAX_STATE            4

#define DW_UART_FCR_TET_OFFSET              4
#define DW_UART_FCR_TET_SIZE                2
#define DW_UART_FCR_TET_MASK                0x30
#define DW_UART_FCR_TET_00                  0x00
#define DW_UART_FCR_TET_01                  0x10
#define DW_UART_FCR_TET_10                  0x20
#define DW_UART_FCR_TET_11                  0x30

#define DW_UART_FCR_DMAM_OFFSET             3
#define DW_UART_FCR_DMAM_SIZE               1
#define DW_UART_FCR_DMAM_MASK               0x8

#define DW_UART_FCR_XFIFOR_OFFSET           2
#define DW_UART_FCR_XFIFOR_SIZE             1
#define DW_UART_FCR_XFIFOR_MASK             0x4

#define DW_UART_FCR_RFIFOR_OFFSET           1
#define DW_UART_FCR_RFIFOR_SIZE             1
#define DW_UART_FCR_RFIFOR_MASK             0x2

#define DW_UART_FCR_FIFOE_OFFSET            0
#define DW_UART_FCR_FIFOE_SIZE              1
#define DW_UART_FCR_FIFOE_MASK              0x1



#define DW_UART_LCR                         (0xC)

#define DW_UART_LCR_DLAB_OFFSET             7
#define DW_UART_LCR_DLAB_SIZE               1
#define DW_UART_LCR_DLAB_MASK               0x80

#define DW_UART_LCR_BC_OFFSET               6
#define DW_UART_LCR_BC_SIZE                 1
#define DW_UART_LCR_BC_MASK                 0x40

#define DW_UART_LCR_SP_OFFSET               5
#define DW_UART_LCR_SP_SIZE                 1
#define DW_UART_LCR_SP_MASK                 0x20

#define DW_UART_LCR_EPS_OFFSET              4
#define DW_UART_LCR_EPS_SIZE                1
#define DW_UART_LCR_EPS_MASK                0x10

#define DW_UART_LCR_PEN_OFFSET              3
#define DW_UART_LCR_PEN_SIZE                1
#define DW_UART_LCR_PEN_MASK                0x8

#define DW_UART_LCR_STOP_OFFSET             2
#define DW_UART_LCR_STOP_SIZE               1
#define DW_UART_LCR_STOP_MASK               0x4

#define DW_UART_LCR_DLS_OFFSET              0
#define DW_UART_LCR_DLS_SIZE                2
#define DW_UART_LCR_DLS_MASK                0x3

#define DW_UART_LCR_WLEN5                   0x0
#define DW_UART_LCR_WLEN6                   0x1
#define DW_UART_LCR_WLEN7                   0x2
#define DW_UART_LCR_WLEN8                   0x3


#define DW_UART_MCR                         (0x10)

#define DW_UART_MCR_SIRE_OFFSET             6
#define DW_UART_MCR_SIRE_SIZE               1
#define DW_UART_MCR_SIRE_MASK               0x40

#define DW_UART_MCR_AFCE_OFFSET             5
#define DW_UART_MCR_AFCE_SIZE               1
#define DW_UART_MCR_AFCE_MASK               0x20

#define DW_UART_MCR_LB_OFFSET               4
#define DW_UART_MCR_LB_SIZE                 1
#define DW_UART_MCR_LB_MASK                 0x10

#define DW_UART_MCR_OUT2_OFFSET             3
#define DW_UART_MCR_OUT2_SIZE               1
#define DW_UART_MCR_OUT2_MASK               0x8

#define DW_UART_MCR_OUT1_OFFSET             2
#define DW_UART_MCR_OUT1_SIZE               1
#define DW_UART_MCR_OUT1_MASK               0x4

#define DW_UART_MCR_RTS_OFFSET              1
#define DW_UART_MCR_RTS_SIZE                1
#define DW_UART_MCR_RTS_MASK                0x2

#define DW_UART_MCR_DTR_OFFSET              0
#define DW_UART_MCR_DTR_SIZE                1
#define DW_UART_MCR_DTR_MASK                0x1



#define DW_UART_LSR                         (0x14)

#define DW_UART_LSR_RFE_OFFSET              7
#define DW_UART_LSR_RFE_SIZE                1
#define DW_UART_LSR_RFE_MASK                0x80

#define DW_UART_LSR_TEMT_OFFSET             6
#define DW_UART_LSR_TEMT_SIZE               1
#define DW_UART_LSR_TEMT_MASK               0x40

#define DW_UART_LSR_THRE_OFFSET             5
#define DW_UART_LSR_THRE_SIZE               1
#define DW_UART_LSR_THRE_MASK               0x20

#define DW_UART_LSR_BI_OFFSET               4
#define DW_UART_LSR_BI_SIZE                 1
#define DW_UART_LSR_BI_MASK                 0x10

#define DW_UART_LSR_FE_OFFSET               3
#define DW_UART_LSR_FE_SIZE                 1
#define DW_UART_LSR_FE_MASK                 0x8

#define DW_UART_LSR_PE_OFFSET               2
#define DW_UART_LSR_PE_SIZE                 1
#define DW_UART_LSR_PE_MASK                 0x4

#define DW_UART_LSR_OE_OFFSET               1
#define DW_UART_LSR_OE_SIZE                 1
#define DW_UART_LSR_OE_MASK                 0x2

#define DW_UART_LSR_DR_OFFSET               0
#define DW_UART_LSR_DR_SIZE                 1
#define DW_UART_LSR_DR_MASK                 0x1


#define DW_UART_LSR_BRK_ERROR_BITS          0x1E



#define DW_UART_MSR                         (0x18)

#define DW_UART_MSR_DCD_OFFSET              7
#define DW_UART_MSR_DCD_SIZE                1
#define DW_UART_MSR_DCD_MASK                0x80

#define DW_UART_MSR_RI_OFFSET               6
#define DW_UART_MSR_RI_SIZE                 1
#define DW_UART_MSR_RI_MASK                 0x40

#define DW_UART_MSR_DSR_OFFSET              5
#define DW_UART_MSR_DSR_SIZE                1
#define DW_UART_MSR_DSR_MASK                0x20

#define DW_UART_MSR_CTS_OFFSET              4
#define DW_UART_MSR_CTS_SIZE                1
#define DW_UART_MSR_CTS_MASK                0x10

#define DW_UART_MSR_DDCD_OFFSET             3
#define DW_UART_MSR_DDCD_SIZE               1
#define DW_UART_MSR_DDCD_MASK               0x8

#define DW_UART_MSR_TERI_OFFSET             2
#define DW_UART_MSR_TERI_SIZE               1
#define DW_UART_MSR_TERI_MASK               0x4

#define DW_UART_MSR_DDSR_OFFSET             1
#define DW_UART_MSR_DDSR_SIZE               1
#define DW_UART_MSR_DDSR_MASK               0x2

#define DW_UART_MSR_DCTS_OFFSET             0
#define DW_UART_MSR_DCTS_SIZE               1
#define DW_UART_MSR_DCTS_MASK               0x1

#define DW_UART_MSR_ANY_DELTA               0x0F

#define DW_UART_MSR_ANY_STATUS              0xF0



#define DW_UART_SCR                         (0x1C)
#define DW_UART_SCR_SIZE                    9


#define DW_UART_SRBR(n)                     (0x30 + ((n) & 0xF) << 2)
#define DW_UART_SRBR_OFFSET                 0
#define DW_UART_SRBR_SIZE                   8


#define DW_UART_STHR(n)                     (0x30 + ((n) & 0xF) << 2)
#define DW_UART_STHR_OFFSET                 0
#define DW_UART_STHR_SIZE                   8


#define DW_UART_FAR                         (0x70)
#define DW_UART_FAR_OFFSET                  0
#define DW_UART_FAR_SIZE                    1


#define DW_UART_USR                         (0x7C)

#define DW_UART_USR_RFF_OFFSET              4
#define DW_UART_USR_RFF_SIZE                1
#define DW_UART_USR_RFF_MASK                0x10

#define DW_UART_USR_RFNE_OFFSET             3
#define DW_UART_USR_RFNE_SIZE               1
#define DW_UART_USR_RFNE_MASK               0x8

#define DW_UART_USR_TFE_OFFSET              2
#define DW_UART_USR_TFE_SIZE                1
#define DW_UART_USR_TFE_MASK                0x4

#define DW_UART_USR_TFNF_OFFSET             1
#define DW_UART_USR_TFNF_SIZE               1
#define DW_UART_USR_TFNF_MASK               0x2

#define DW_UART_USR_BUSY_OFFSET             0
#define DW_UART_USR_BUSY_SIZE               1
#define DW_UART_USR_BUSY_MASK               0x1


#define DW_UART_TFL                         (0x80)

#define DW_UART_TFL_OFFSET                  0
#define DW_UART_TFL_SIZE                    5


#define DW_UART_RFL                         (0x84)

#define DW_UART_RFL_OFFSET                  0
#define DW_UART_RFL_SIZE                    5


#define DW_UART_SRR                         (0x88)

#define DW_UART_SRR_XFR_OFFSET              2
#define DW_UART_SRR_XFR_SIZE                1

#define DW_UART_SRR_RFR_OFFSET              1
#define DW_UART_SRR_RFR_SIZE                1

#define DW_UART_SRR_UR_OFFSET               0
#define DW_UART_SRR_UR_SIZE                 1


#define DW_UART_SRTS                        (0x8C)
#define DW_UART_SRTS_OFFSET                  0
#define DW_UART_SRTS_SIZE                    1


#define DW_UART_SBCR                        (0x90)
#define DW_UART_SBCR_OFFSET                 0
#define DW_UART_SBCR_SIZE                   1


#define DW_UART_SDMAM                       (0x94)
#define DW_UART_SDMAM_OFFSET                0
#define DW_UART_SDMAM_SIZE                  1


#define DW_UART_SFE                         (0x98)
#define DW_UART_SFE_OFFSET                  0
#define DW_UART_SFE_SIZE                    1


#define DW_UART_SRT                         (0x9C)
#define DW_UART_SRT_OFFSET                  0
#define DW_UART_SRT_SIZE                    2


#define DW_UART_STET                        (0xA0)
#define DW_UART_STET_OFFSET                 0
#define DW_UART_STET_SIZE                   2


#define DW_UART_HTX                         (0xA4)
#define DW_UART_HTX_OFFSET                  0
#define DW_UART_HTX_SIZE                    1


#define DW_UART_DMASA                       (0xA8)
#define DW_UART_DMASA_OFFSET                0
#define DW_UART_DMASA_SIZE                  1

#if (DW_CC_UART_RS485_INTERFACE_EN == 1)
#define DW_UART_TCR                         (0xAC)
#define DW_UART_DE_EN                       (0xB0)
#define DW_UART_RE_EN                       (0xB4)
#define DW_UART_DET                         (0xB8)
#define DW_UART_TAT                         (0xBC)
#endif


#define DW_UART_DLF                         (0xC0)
#define DW_UART_DLF_OFFSET                  0
#define DW_UART_DLF_SIZE                    4

#if (DW_CC_UART_9BIT_DATA_EN == 1)
#define DW_UART_RAR                         (0xC4)
#define DW_UART_TAR                         (0xC8)
#define DW_UART_LCR_EXT                     (0xCC)
#endif


#define DW_UART_UART_PROT_LEVEL             (0xD0)
#define DW_UART_UART_PROT_LEVEL_OFFSET      0
#define DW_UART_UART_PROT_LEVEL_SIZE        3


#define DW_UART_REG_TIMEOUT_RST             (0xD4)
#define DW_UART_REG_TIMEOUT_RST_OFFSET      0
#define DW_UART_REG_TIMEOUT_RST_SIZE        4


#define DW_UART_CPR                         (0xF4)

#define DW_UART_CPR_FIFO_MODE_OFFSET                16
#define DW_UART_CPR_FIFO_MODE_SIZE                  8
#define DW_UART_CPR_FIFO_MODE_MASK                  0xff0000

#define DW_UART_CPR_FIFO_MODE_CALC(a)               (((a >> 16) & 0xff) * 16)

#define DW_UART_CPR_DMA_EXTRA_OFFSET                13
#define DW_UART_CPR_DMA_EXTRA_SIZE                  1
#define DW_UART_CPR_DMA_EXTRA_MASK                  0x2000

#define DW_UART_CPR_UART_ADD_ENCODED_PARAMS_OFFSET  12
#define DW_UART_CPR_UART_ADD_ENCODED_PARAMS_SIZE    1
#define DW_UART_CPR_UART_ADD_ENCODED_PARAMS_MASK    0x1000

#define DW_UART_CPR_SHADOW_OFFSET                   11
#define DW_UART_CPR_SHADOW_SIZE                     1
#define DW_UART_CPR_SHADOW_MASK                     0x800

#define DW_UART_CPR_FIFO_STAT_OFFSET                10
#define DW_UART_CPR_FIFO_STAT_SIZE                  1
#define DW_UART_CPR_FIFO_STAT_MASK                  0x400

#define DW_UART_CPR_FIFO_ACCESS_OFFSET              9
#define DW_UART_CPR_FIFO_ACCESS_SIZE                1
#define DW_UART_CPR_FIFO_ACCESS_MASK                0x200

#define DW_UART_CPR_ADDITIONAL_FEAT_OFFSET          8
#define DW_UART_CPR_ADDITIONAL_FEAT_SIZE            1
#define DW_UART_CPR_ADDITIONAL_FEAT_MASK            0x100

#define DW_UART_CPR_SIR_LP_MODE_OFFSET              7
#define DW_UART_CPR_SIR_LP_MODE_SIZE                1
#define DW_UART_CPR_SIR_LP_MODE_MASK                0x80

#define DW_UART_CPR_SIR_MODE_OFFSET                 6
#define DW_UART_CPR_SIR_MODE_SIZE                   1
#define DW_UART_CPR_SIR_MODE_MASK                   0x40

#define DW_UART_CPR_THRE_MODE_OFFSET                5
#define DW_UART_CPR_THRE_MODE_SIZE                  1
#define DW_UART_CPR_THRE_MODE_MASK                  0x20

#define DW_UART_CPR_AFCE_MODE_OFFSET                4
#define DW_UART_CPR_AFCE_MODE_SIZE                  1
#define DW_UART_CPR_AFCE_MODE_MASK                  0x10

#define DW_UART_CPR_APB_DATA_WIDTH_OFFSET           0
#define DW_UART_CPR_APB_DATA_WIDTH_SIZE             2
#define DW_UART_CPR_APB_DATA_WIDTH_MASK             0x3


#define DW_UART_UCV                         (0xF8)
#define DW_UART_UCV_OFFSET                  0
#define DW_UART_UCV_SIZE                    32


#define DW_UART_CTR                         (0xFC)
#define DW_UART_CTR_OFFSET                  0
#define DW_UART_CTR_SIZE                    32

#endif
