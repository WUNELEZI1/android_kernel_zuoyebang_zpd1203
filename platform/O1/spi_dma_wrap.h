// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef SPI_DMA_WRAP_REGIF_H
#define SPI_DMA_WRAP_REGIF_H

#define SPI_DMA_WRAP_SEL                                              0x0160
#define SPI_DMA_WRAP_SEL_DMA_SEL_SHIFT                                                0
#define SPI_DMA_WRAP_SEL_DMA_SEL_MASK                                                 0x00000003
#define SPI_DMA_WRAP_BUSY                                             0x0164
#define SPI_DMA_WRAP_BUSY_BUSY_SHIFT                                                  0
#define SPI_DMA_WRAP_BUSY_BUSY_MASK                                                   0x00000001
#define SPI_DMA_WRAP_INT_ERR_SIRQ_INT_MASK                            0x0180
#define SPI_DMA_WRAP_INT_ERR_SIRQ_INT_MASK_SSI_INTR_MASK_SHIFT                        0
#define SPI_DMA_WRAP_INT_ERR_SIRQ_INT_MASK_SSI_INTR_MASK_MASK                         0x00000001
#define SPI_DMA_WRAP_INT_ERR_SIRQ_INT_MASK_INT_SPI_DMA_ERR_MASK_SHIFT                 1
#define SPI_DMA_WRAP_INT_ERR_SIRQ_INT_MASK_INT_SPI_DMA_ERR_MASK_MASK                  0x00000002
#define SPI_DMA_WRAP_INT_ERR_SIRQ_INT_STATUS                          0x0184
#define SPI_DMA_WRAP_INT_ERR_SIRQ_INT_STATUS_SSI_INTR_STATUS_SHIFT                    0
#define SPI_DMA_WRAP_INT_ERR_SIRQ_INT_STATUS_SSI_INTR_STATUS_MASK                     0x00000001
#define SPI_DMA_WRAP_INT_ERR_SIRQ_INT_STATUS_INT_SPI_DMA_ERR_STATUS_SHIFT             1
#define SPI_DMA_WRAP_INT_ERR_SIRQ_INT_STATUS_INT_SPI_DMA_ERR_STATUS_MASK              0x00000002


#endif
