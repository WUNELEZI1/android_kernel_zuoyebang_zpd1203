// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef DW_APB_SSI_HEADER_H
#define DW_APB_SSI_HEADER_H


#define DW_SPI_CTRLR0			0x00
#define DW_SPI_CTRLR1			0x04
#define DW_SPI_SSIENR			0x08
#define DW_SPI_MWCR			0x0c
#define DW_SPI_SER			0x10
#define DW_SPI_BAUDR			0x14
#define DW_SPI_TXFTLR			0x18
#define DW_SPI_RXFTLR			0x1c
#define DW_SPI_TXFLR			0x20
#define DW_SPI_RXFLR			0x24
#define DW_SPI_SR			0x28
#define DW_SPI_IMR			0x2c
#define DW_SPI_ISR			0x30
#define DW_SPI_RISR			0x34
#define DW_SPI_TXOICR			0x38
#define DW_SPI_RXOICR			0x3c
#define DW_SPI_RXUICR			0x40
#define DW_SPI_MSTICR			0x44
#define DW_SPI_ICR			0x48
#define DW_SPI_DMACR			0x4c
#define DW_SPI_DMATDLR			0x50
#define DW_SPI_DMARDLR			0x54
#define DW_SPI_IDR			0x58
#define DW_SPI_VERSION			0x5c
#define DW_SPI_DR			0x60
#define DW_SPI_RX_SAMPLE_DLY		0xf0
#define DW_SPI_CS_OVERRIDE		0xf4


#define SPI_DFS_OFFSET			0
#define SPI_DFS_MASK			0
#define SPI_DFS32_OFFSET		16
#define SPI_SSTE_OFFSET			(1 << 24)

#define SPI_FRF_OFFSET			4
#define SPI_FRF_SPI			0x0
#define SPI_FRF_SSP			0x1
#define SPI_FRF_MICROWIRE		0x2
#define SPI_FRF_RESV			0x3

#define SPI_MODE_OFFSET			6
#define SPI_SCPH_OFFSET			6
#define SPI_SCOL_OFFSET			7

#define SPI_TMOD_OFFSET			8
#define SPI_TMOD_MASK			(0x3 << SPI_TMOD_OFFSET)
#define	SPI_TMOD_TR			0x0
#define SPI_TMOD_TO			0x1
#define SPI_TMOD_RO			0x2
#define SPI_TMOD_EPROMREAD		0x3

#define SPI_SLVOE_OFFSET		10
#define SPI_SRL_OFFSET			11
#define SPI_CFS_OFFSET			12


#define DWC_SSI_CTRLR0_SRL_OFFSET	13
#define DWC_SSI_CTRLR0_TMOD_OFFSET	10
#define DWC_SSI_CTRLR0_TMOD_MASK	GENMASK(11, 10)
#define DWC_SSI_CTRLR0_SCPOL_OFFSET	9
#define DWC_SSI_CTRLR0_SCPH_OFFSET	8
#define DWC_SSI_CTRLR0_FRF_OFFSET	6
#define DWC_SSI_CTRLR0_DFS_OFFSET	0


#define DWC_SSI_CTRLR0_KEEMBAY_MST	BIT(31)


#define SPI_NDF_MASK			GENMASK(15, 0)


#define SR_MASK				0x7f
#define SR_BUSY				(1 << 0)
#define SR_TF_NOT_FULL			(1 << 1)
#define SR_TF_EMPT			(1 << 2)
#define SR_RF_NOT_EMPT			(1 << 3)
#define SR_RF_FULL			(1 << 4)
#define SR_TX_ERR			(1 << 5)
#define SR_DCOL				(1 << 6)


#define SPI_INT_TXEI			(1 << 0)
#define SPI_INT_TXOI			(1 << 1)
#define SPI_INT_RXUI			(1 << 2)
#define SPI_INT_RXOI			(1 << 3)
#define SPI_INT_RXFI			(1 << 4)
#define SPI_INT_MSTI			(1 << 5)

#define SSI_INRT_MASK_EN_B		(1 << 0)


#define SPI_DMA_RDMAE			(1 << 0)
#define SPI_DMA_TDMAE			(1 << 1)
#define CLOCK_PERIOD_COUNT      4U

#define SPI_WAIT_RETRIES		5
#define SPI_BUF_SIZE \
	(sizeof_field(struct spi_mem_op, cmd.opcode) + \
	 sizeof_field(struct spi_mem_op, addr.val) + 256)
#define SPI_GET_BYTE(_val, _idx) \
	((_val) >> (BITS_PER_BYTE * (_idx)) & 0xff)


#define DW_SPI_CAP_CS_OVERRIDE		BIT(0)
#define DW_SPI_CAP_KEEMBAY_MST		BIT(1)
#define DW_SPI_CAP_DWC_SSI		BIT(2)
#define DW_SPI_CAP_DFS32		BIT(3)

#endif
