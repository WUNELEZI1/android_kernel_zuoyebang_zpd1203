/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
 * Synopsys DesignWare AXI DMA Controller driver.
 *
 * (C) 2017-2018 Synopsys, Inc. (www.synopsys.com)
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _AXI_DMA_PLATFORM_H
#define _AXI_DMA_PLATFORM_H

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/dmaengine.h>
#include <linux/types.h>

#include "dmaengine.h"
#include "virt-dma.h"
#include <dt-bindings/xring/platform-specific/DW_axi_dmac/peri_ns/DW_axi_dmac_header.h>
#include <dt-bindings/xring/platform-specific/peri_crg.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)		"XR_DMA: " fmt

#define XR_DMA_CLK_NUMS		2
#define XR_PCLK_PREFIX_NAME	"pclk"
#define XR_PCLK_PREFIX_LEN	4

#define TPC_RST_NS_BIT	BIT(PERI_CRG_RST1_W1S_IP_RST_DMA_NS_TPC_N_SHIFT)
#define RST_NS_BIT	BIT(PERI_CRG_RST1_W1S_IP_RST_DMA_NS_N_SHIFT)


/* Used to check whether the DTS configuration information is valid. */
#define DMAC_MAX_CHANNELS	30
#define DMAC_MAX_MASTERS	1
#define DMAC_MAX_BLK_SIZE	0x400000

#define XR_MEM_DEV_ADDR_ALIGN_BYTE	4
#define XR_DMA_BIT_MASK_BIT		64

#define XR_DMA_CHAN_PAUSE_TIMEOUT	20  /* count */
#define XR_DMA_CHAN_PAUSE_UDELAY	2  /* us */

#define XR_TO_HIGH_ADDRESS(x)	((x) + 4)
#define XR_TO_BLOCK_SIZE(x)	((x) - 1)

#define TO_READ_REG_VALUE(hi, lo) \
		((((hi) << 32) & 0xFFFFFFFF00000000) | ((lo) & 0xFFFFFFFF))

#define XR_CHAN_LESS_EQUAL_THAN_8(x)	((x) <= 8)
#define XR_CHAN_GREATER_THAN_16(x)	((x) > 15)

/* Wait time range 1000-10000 ms */
#define TERMINATE_TIMEOUT_RANGE_MINI	1000
#define TERMINATE_TIMEOUT_RANGE_MAX	10000

#define XR_DMA_MULTI_BLOCK_MODE		1

struct dw_axi_dma_hcfg {
	u32	nr_channels;
	u32	nr_masters;
	u32	m_data_width;
	u32	block_size[DMAC_MAX_CHANNELS];
	u32	priority[DMAC_MAX_CHANNELS];
	/* maximum supported axi burst length */
	u32	axi_rw_burst_len;
	/* Register map for DMAX_NUM_CHANNELS <= 8 */
	bool	reg_map_8_channels;
	bool	restrict_axi_burst_len;
};

struct axi_dma_chan {
	struct axi_dma_chip		*chip;
	void __iomem			*chan_regs;
	u8				id;
	u8				hw_handshake_num;
	atomic_t			descs_allocated;

	struct dma_pool			*desc_pool;
	struct virt_dma_chan		vc;

	struct axi_dma_desc		*desc;
	struct dma_slave_config		config;
	enum dma_transfer_direction	direction;
	bool				cyclic;
	/* these other elements are all protected by vc.lock */
	bool				is_paused;

	int				is_multi_block;

#ifdef CONFIG_DEBUG_FS
	u32				dma_tranfer_done_count;
	u32				block_tranfer_done_count;
	u32				xr_chan_intr_state;
#endif
};

struct dw_axi_dma {
	struct dma_device	dma;
	struct dw_axi_dma_hcfg	*hdata;
	struct device_dma_parameters	dma_parms;

	/* channels */
	struct axi_dma_chan	*chan;

	/* debugfs catalogue */
	struct dentry		*xr_dma_dentry;

	raw_spinlock_t		lock;
};

struct axi_dma_chip {
	struct device		*dev;
	int			irq;
	void __iomem		*regs;
	void __iomem		*apb_regs;
	struct clk		*core_clk;
	struct clk		*cfgr_clk;
	struct reset_control	*rst;
	struct reset_control	*tpc_rst;
	struct dw_axi_dma	*dw;
};

/* LLI address must be aligned to a 64-byte boundary */
#define XR_LLI_DMA_POOL_ALIGN		64
#define XR_LLI_DMA_POOL_BOUNDARY	0

/* LLI == Linked List Item */
struct __packed axi_dma_lli {
	__le64		sar;
	__le64		dar;
	__le32		block_ts_lo;
	__le32		block_ts_hi;
	__le64		llp;
	__le32		ctl_lo;
	__le32		ctl_hi;
	__le32		sstat;
	__le32		dstat;
	__le32		status_lo;
	__le32		status_hi;
	__le32		reserved_lo;
	__le32		reserved_hi;
};

struct axi_dma_hw_desc {
	struct axi_dma_lli	*lli;
	dma_addr_t		llp;
	u32			len;
};

struct xr_dma_single_hw_desc {
	__le64		sar;
	__le64		dar;
	__le32		block_ts;
	__le64		ctl;
	u32		len;
};

struct axi_dma_desc {
	struct axi_dma_hw_desc	*hw_desc;
	struct xr_dma_single_hw_desc *single_hw_desc;

	struct virt_dma_desc		vd;
	struct axi_dma_chan		*chan;
	u32				completed_blocks;
	u32				length;
	u32				period_len;
};

struct axi_dma_chan_config {
	u8 dst_multblk_type;
	u8 src_multblk_type;
	u8 dst_per;
	u8 src_per;
	u8 tt_fc;
	u8 prior;
	u8 hs_sel_dst;
	u8 hs_sel_src;
};

static inline struct device *dchan2dev(struct dma_chan *dchan)
{
	return &dchan->dev->device;
}

static inline struct device *chan2dev(struct axi_dma_chan *chan)
{
	return &chan->vc.chan.dev->device;
}

static inline struct axi_dma_desc *vd_to_axi_desc(struct virt_dma_desc *vd)
{
	return container_of(vd, struct axi_dma_desc, vd);
}

static inline struct axi_dma_chan *vc_to_axi_dma_chan(struct virt_dma_chan *vc)
{
	return container_of(vc, struct axi_dma_chan, vc);
}

static inline struct axi_dma_chan *dchan_to_axi_dma_chan(struct dma_chan *dchan)
{
	return vc_to_axi_dma_chan(to_virt_chan(dchan));
}

#define XRING_CHAN_REG_LEN \
		(Channel2_Registers_Address_Block_BaseAddress \
		- Channel1_Registers_Address_Block_BaseAddress)

/* DMA channel registers offset */
#define CH_SAR			CH1_SAR /* R/W Chan Source Address */
#define CH_DAR			CH1_DAR /* R/W Chan Destination Address */
#define CH_BLOCK_TS		CH1_BLOCK_TS /* R/W Chan Block Transfer Size */
#define CH_CTL			CH1_CTL /* R/W Chan Control */
#define CH_CTL_L		CH1_CTL /* R/W Chan Control 00-31 */
#define CH_CTL_H		(CH1_CTL + 4) /* R/W Chan Control 32-63 */
#define CH_CFG			CH1_CFG2 /* R/W Chan Configuration */
#define CH_CFG_L		CH1_CFG2 /* R/W Chan Configuration 00-31 */
#define CH_CFG_H		(CH1_CFG2 + 4) /* R/W Chan Configuration 32-63 */
#define CH_LLP			CH1_LLP /* R/W Chan Linked List Pointer */
#define CH_STATUS		CH1_STATUSREG /* R Chan Status */
#define CH_SWHSSRC		CH1_SWHSSRCREG /* R/W Chan SW Handshake Source */
#define CH_SWHSDST		CH1_SWHSDSTREG /* R/W Chan SW Handshake Destination */
#define CH_BLK_TFR_RESUMEREQ	CH1_BLK_TFR_RESUMEREQREG /* W Chan Block Transfer Resume Req */
#define CH_AXI_ID		CH1_AXI_IDREG /* R/W Chan AXI ID */
#define CH_AXI_QOS		CH1_AXI_QOSREG /* R/W Chan AXI QOS */
#define CH_SSTAT		CH1_SSTAT /* R Chan Source Status */
#define CH_DSTAT		CH1_DSTAT /* R Chan Destination Status */
#define CH_SSTATAR		CH1_SSTATAR /* R/W Chan Source Status Fetch Addr */
#define CH_DSTATAR		CH1_DSTATAR /* R/W Chan Destination Status Fetch Addr */
#define CH_INTSTATUS_ENA	CH1_INTSTATUS_ENABLEREG /* R/W Chan Interrupt Status Enable */
#define CH_INTSTATUS		CH1_INTSTATUS /* R/W Chan Interrupt Status */
#define CH_INTSIGNAL_ENA	CH1_INTSIGNAL_ENABLEREG /* R/W Chan Interrupt Signal Enable */
#define CH_INTCLEAR		CH1_INTCLEARREG /* W Chan Interrupt Clear */


/* DMAC_CFG */
#define DMAC_EN_POS			0
#define DMAC_EN_MASK			BIT(DMAC_EN_POS)

#define INT_EN_POS			1
#define INT_EN_MASK			BIT(INT_EN_POS)

/* DMAC_CHEN and DMAC_CHSUSP, channel < 8 */
#define DMAC_CHAN_EN_SHIFT		0
#define DMAC_CHAN_EN_WE_SHIFT		8

#define DMAC_CHAN_SUSP_SHIFT		16
#define DMAC_CHAN_SUSP_WE_SHIFT		24

/* DMAC_CHEN2 DMAC_CHENREG2 */
#define XR_CHAN_1_16_EN2_SHIFT		DMAC_CHENREG2_CH1_EN_BitAddressOffset /* 0 */
#define XR_CHAN_1_16_EN2_WE_SHIFT	DMAC_CHENREG2_CH1_EN_WE_BitAddressOffset /* 16 */

 /* To enable high 16 channels, move 16bit to the left, 32 - 16  = 16 */
#define XR_CHAN_17_32_EN2_SHIFT		(DMAC_CHENREG2_CH17_EN_BitAddressOffset - \
					 DMAC_CHENREG2_CH1_EN_WE_BitAddressOffset)
 /* To enable WE high 16 channels, move 32bit to the left, 48 - 16  = 32 */
#define XR_CHAN_17_32_EN2_WE_SHIFT	(DMAC_CHENREG2_CH17_EN_WE_BitAddressOffset - \
					 DMAC_CHENREG2_CH1_EN_WE_BitAddressOffset)

/* DMAC_CHSUSPREG */
#define DMAC_CHAN_SUSP2_SHIFT		0
#define DMAC_CHAN_SUSP2_WE_SHIFT	16

#define XR_CHAN_1_16_SUSP2_SHIFT	DMAC_CHSUSPREG_CH1_SUSP_BitAddressOffset /* 0 */
#define XR_CHAN_1_16_SUSP2_WE_SHIFT	DMAC_CHSUSPREG_CH1_SUSP_WE_BitAddressOffset /* 16 */

 /* To suspend high 16 channels, move 16bit to the left, 32 - 16  = 16 */
#define XR_CHAN_17_32_SUSP2_SHIFT	(DMAC_CHSUSPREG_CH17_SUSP_BitAddressOffset - \
					 DMAC_CHSUSPREG_CH1_SUSP_WE_BitAddressOffset)
 /* To suspend WE high 16 channels, move 32bit to the left, 48 - 16  = 32 */
#define XR_CHAN_17_32_SUSP2_WE_SHIFT	(DMAC_CHSUSPREG_CH17_SUSP_WE_BitAddressOffset - \
					 DMAC_CHSUSPREG_CH1_SUSP_WE_BitAddressOffset)

/* CH_CTL_H */
#define CH_CTL_H_ARLEN_EN		BIT(6)
#define CH_CTL_H_ARLEN_POS		7
#define CH_CTL_H_AWLEN_EN		BIT(15)
#define CH_CTL_H_AWLEN_POS		16

enum {
	DWAXIDMAC_ARWLEN_1		= 0,
	DWAXIDMAC_ARWLEN_2		= 1,
	DWAXIDMAC_ARWLEN_4		= 3,
	DWAXIDMAC_ARWLEN_8		= 7,
	DWAXIDMAC_ARWLEN_16		= 15,
	DWAXIDMAC_ARWLEN_32		= 31,
	DWAXIDMAC_ARWLEN_64		= 63,
	DWAXIDMAC_ARWLEN_128		= 127,
	DWAXIDMAC_ARWLEN_256		= 255,
	DWAXIDMAC_ARWLEN_MIN		= DWAXIDMAC_ARWLEN_1,
	DWAXIDMAC_ARWLEN_MAX		= DWAXIDMAC_ARWLEN_256
};

#define CH_CTL_H_LLI_LAST		BIT(30)
#define CH_CTL_H_LLI_VALID		BIT(31)

/* CH_CTL_L */
#define CH_CTL_L_LAST_WRITE_EN		BIT(30)

#define CH_CTL_L_DST_MSIZE_POS		18
#define CH_CTL_L_SRC_MSIZE_POS		14

enum {
	DWAXIDMAC_BURST_TRANS_LEN_1	= 0,
	DWAXIDMAC_BURST_TRANS_LEN_4,
	DWAXIDMAC_BURST_TRANS_LEN_8,
	DWAXIDMAC_BURST_TRANS_LEN_16,
	DWAXIDMAC_BURST_TRANS_LEN_32,
	DWAXIDMAC_BURST_TRANS_LEN_64,
	DWAXIDMAC_BURST_TRANS_LEN_128,
	DWAXIDMAC_BURST_TRANS_LEN_256,
	DWAXIDMAC_BURST_TRANS_LEN_512,
	DWAXIDMAC_BURST_TRANS_LEN_1024
};

#define CH_CTL_L_DST_WIDTH_POS		11
#define CH_CTL_L_SRC_WIDTH_POS		8

#define CH_CTL_L_DST_INC_POS		6
#define CH_CTL_L_SRC_INC_POS		4
enum {
	DWAXIDMAC_CH_CTL_L_INC		= 0,
	DWAXIDMAC_CH_CTL_L_NOINC
};

#define CH_CTL_L_DST_MAST		BIT(2)
#define CH_CTL_L_SRC_MAST		BIT(0)

/* CH_CFG_H */
#define CH_CFG_H_PRIORITY_POS		17
#define CH_CFG_H_DST_PER_POS		12
#define CH_CFG_H_SRC_PER_POS		7
#define CH_CFG_H_HS_SEL_DST_POS		4
#define CH_CFG_H_HS_SEL_SRC_POS		3
enum {
	DWAXIDMAC_HS_SEL_HW		= 0,
	DWAXIDMAC_HS_SEL_SW
};

#define CH_CFG_H_TT_FC_POS		0
enum {
	DWAXIDMAC_TT_FC_MEM_TO_MEM_DMAC	= 0,
	DWAXIDMAC_TT_FC_MEM_TO_PER_DMAC,
	DWAXIDMAC_TT_FC_PER_TO_MEM_DMAC,
	DWAXIDMAC_TT_FC_PER_TO_PER_DMAC,
	DWAXIDMAC_TT_FC_PER_TO_MEM_SRC,
	DWAXIDMAC_TT_FC_PER_TO_PER_SRC,
	DWAXIDMAC_TT_FC_MEM_TO_PER_DST,
	DWAXIDMAC_TT_FC_PER_TO_PER_DST
};

/* CH_CFG_L */
#define CH_CFG_L_DST_MULTBLK_TYPE_POS	2
#define CH_CFG_L_SRC_MULTBLK_TYPE_POS	0
enum {
	DWAXIDMAC_MBLK_TYPE_CONTIGUOUS	= 0,
	DWAXIDMAC_MBLK_TYPE_RELOAD,
	DWAXIDMAC_MBLK_TYPE_SHADOW_REG,
	DWAXIDMAC_MBLK_TYPE_LL
};

/* CH_CFG2 */
#define CH_CFG2_L_SRC_PER_POS		4
#define CH_CFG2_L_DST_PER_POS		11

#define CH_CFG2_H_TT_FC_POS		0
#define CH_CFG2_H_HS_SEL_SRC_POS	3
#define CH_CFG2_H_HS_SEL_DST_POS	4
#define CH_CFG2_H_PRIORITY_POS		20


#define AXI_GEN_CH_CTL(dst_burst, src_burst, dst_width, src_width, dst_inc, src_inc) \
	((dst_burst) << CH1_CTL_DST_MSIZE_BitAddressOffset | \
	(src_burst) << CH1_CTL_SRC_MSIZE_BitAddressOffset | \
	(dst_width) << CH1_CTL_DST_TR_WIDTH_BitAddressOffset | \
	(src_width) << CH1_CTL_SRC_TR_WIDTH_BitAddressOffset | \
	(dst_inc) << CH1_CTL_DINC_BitAddressOffset | \
	(src_inc) << CH1_CTL_SINC_BitAddressOffset)

#define AXI_GEN_CH_CFG2(prior, tt_fc, dst_type, src_type) \
	((prior) <<  CH1_CFG2_CH_PRIOR_BitAddressOffset | \
	(tt_fc) << CH1_CFG2_TT_FC_BitAddressOffset | \
	(dst_type) << CH1_CFG2_DST_MULTBLK_TYPE_BitAddressOffset | \
	(src_type) << CH1_CFG2_SRC_MULTBLK_TYPE_BitAddressOffset)


/* Block Transfer Done Interrupt Status Enable. 0 */
#define BLOCK_TFR_DONE \
		CH1_INTSTATUS_ENABLEREG_Enable_BLOCK_TFR_DONE_IntStat_BitAddressOffset

/* DMA Transfer Done Interrupt Status Enable. 1 */
#define DMA_TFR_DONE \
		CH1_INTSTATUS_ENABLEREG_Enable_DMA_TFR_DONE_IntStat_BitAddressOffset

/* Source Transaction Completed Status Enable. 3 */
#define SRC_TRANSCOMP \
		CH1_INTSTATUS_ENABLEREG_Enable_SRC_TRANSCOMP_IntStat_BitAddressOffset
/* Destination Transaction Completed Status Enable. 4 */
#define DST_TRANSCOMP \
		CH1_INTSTATUS_ENABLEREG_Enable_DST_TRANSCOMP_IntStat_BitAddressOffset

/* Source Decode Error Status Enable. 5 */
#define SRC_DEC_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SRC_DEC_ERR_IntStat_BitAddressOffset

/* Destination Decode Error Status Enable. 6 */
#define DST_DEC_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_DST_DEC_ERR_IntStat_BitAddressOffset

/* Source Subordinate Error Status Enable. 7 */
#define SRC_SLV_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SRC_SLV_ERR_IntStat_BitAddressOffset

/* Destination Subordinate Error Status Enable. 8 */
#define DST_SLV_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_DST_SLV_ERR_IntStat_BitAddressOffset

/* LLI Read Decode Error Status Enable. 9 */
#define LLI_RD_DEC_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_LLI_RD_DEC_ERR_IntStat_BitAddressOffset

/* LLI WRITE Decode Error Status Enable. 10 */
#define LLI_WR_DEC_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_LLI_WR_DEC_ERR_IntStat_BitAddressOffset

/* LLI Read Subordinate Error Status Enable. 11 */
#define LLI_RD_SLV_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_LLI_RD_SLV_ERR_IntStat_BitAddressOffset

/* LLI WRITE Subordinate Error Status Enable. 12 */
#define LLI_WR_SLV_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_LLI_WR_SLV_ERR_IntStat_BitAddressOffset

/* Shadow register or LLI Invalid Error Status Enable. 13 */
#define SHADOWREG_OR_LLI_INVALID_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SHADOWREG_OR_LLI_INVALID_ERR_IntStat_BitAddressOffset

/* Register Bus Interface Multi Block type Error Status Enable. 14 */
#define SLVIF_MULTIBLKTYPE_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SLVIF_MULTIBLKTYPE_ERR_IntStat_BitAddressOffset

/* Register Bus Interface Decode Error Status Enable. 16 */
#define SLVIF_DEC_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SLVIF_DEC_ERR_IntStat_BitAddressOffset

/* Register Bus Interface Write to Read Only Error Status Enable. 17 */
#define SLVIF_WR2RO_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SLVIF_WR2RO_ERR_IntStat_BitAddressOffset

/* Register Bus Interface Read to write Only Error Status Enable. 18 */
#define SLVIF_RD2RWO_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SLVIF_RD2RWO_ERR_IntStat_BitAddressOffset

/* Register Bus Interface Write On Channel Enabled Error Status Enable. 19  */
#define SLVIF_WRONCHEN_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SLVIF_WRONCHEN_ERR_IntStat_BitAddressOffset

/* Shadow Register Write On Valid Error Status Enable. 20 */
#define SLVIF_SHADOWREG_WRON_VALID_ERR \
	CH1_INTSTATUS_ENABLEREG_Enable_SLVIF_SHADOWREG_WRON_VALID_ERR_IntStat_BitAddressOffset

/* Register Bus Interface Write On Hold Error Status Enable. 21 */
#define SLVIF_WRONHOLD_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SLVIF_WRONHOLD_ERR_IntStat_BitAddressOffset

/* Register Bus Interface Address Parity Error Enable. 24 */
#define SLVIF_ADDRPARITY_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SLVIF_ADDRPARITY_ERR_IntStat_BitAddressOffset

/* Register Bus Interface Write Parity Error Enable. 25 */
#define SLVIF_WRPARITY_ERR \
		CH1_INTSTATUS_ENABLEREG_Enable_SLVIF_WRPARITY_ERR_IntStat_BitAddressOffset

/* Channel Lock Cleared Status Enable. 27 */
#define CH_LOCK_CLEARED \
		CH1_INTSTATUS_ENABLEREG_Enable_CH_LOCK_CLEARED_IntStat_BitAddressOffset

/* Channel Source Suspended Status Enable. 28 */
#define CH_SRC_SUSPENDED \
		CH1_INTSTATUS_ENABLEREG_Enable_CH_SRC_SUSPENDED_IntStat_BitAddressOffset

/* Channel Suspended Status Enable. 29 */
#define CH_SUSPENDED \
		CH1_INTSTATUS_ENABLEREG_Enable_CH_SUSPENDED_IntStat_BitAddressOffset

/* Channel Disabled Status Enable. 30 */
#define CH_DISABLED \
		CH1_INTSTATUS_ENABLEREG_Enable_CH_DISABLED_IntStat_BitAddressOffset

/* Channel Terminated Status Enable. 31 */
#define CH_ABORTED \
		CH1_INTSTATUS_ENABLEREG_Enable_CH_ABORTED_IntStat_BitAddressOffset

/* Channel x Channel Memory Interface ECC Protection Correctable Error Interrupt enable. 32 */
#define ECC_PROT_CHMem_CorrERR \
		CH1_INTSTATUS_ENABLEREG_Enable_ECC_PROT_CHMem_CorrERR_IntStat_BitAddressOffset

/* Channel x Channel Memory Interface ECC Protection Uncorrectable Error Interrupt enable. 33 */
#define ECC_PROT_CHMem_UnCorrERR \
		CH1_INTSTATUS_ENABLEREG_Enable_ECC_PROT_CHMem_UnCorrERR_IntStat_BitAddressOffset

/* Channel x Unique ID Memory Interface ECC Protection Correctable Error Interrupt enable. 34 */
#define ECC_PROT_UIDMem_CorrERR \
		CH1_INTSTATUS_ENABLEREG_Enable_ECC_PROT_UIDMem_CorrERR_IntStat_BitAddressOffset

/* Channel x Unique ID Memory Interface ECC Protection Uncorrectable Error Interrupt enable. 35 */
#define ECC_PROT_UIDMem_UnCorrERR \
		CH1_INTSTATUS_ENABLEREG_Enable_ECC_PROT_UIDMem_UnCorrERR_IntStat_BitAddressOffset

#define XR_IRQ_ALL_ERR \
		(GENMASK(SLVIF_WRONHOLD_ERR, SLVIF_DEC_ERR) | \
		GENMASK(SLVIF_MULTIBLKTYPE_ERR, SRC_DEC_ERR))

enum {
	DWAXIDMAC_IRQ_NONE		= 0,
	DWAXIDMAC_IRQ_BLOCK_TRF		= BIT(BLOCK_TFR_DONE),
	DWAXIDMAC_IRQ_DMA_TRF		= BIT(DMA_TFR_DONE),
	DWAXIDMAC_IRQ_SRC_TRAN		= BIT(SRC_TRANSCOMP),
	DWAXIDMAC_IRQ_DST_TRAN		= BIT(DST_TRANSCOMP),
	DWAXIDMAC_IRQ_SRC_DEC_ERR	= BIT(SRC_DEC_ERR),
	DWAXIDMAC_IRQ_DST_DEC_ERR	= BIT(DST_DEC_ERR),
	DWAXIDMAC_IRQ_SRC_SLV_ERR	= BIT(SRC_SLV_ERR),
	DWAXIDMAC_IRQ_DST_SLV_ERR	= BIT(DST_SLV_ERR),
	DWAXIDMAC_IRQ_LLI_RD_DEC_ERR	= BIT(LLI_RD_DEC_ERR),
	DWAXIDMAC_IRQ_LLI_WR_DEC_ERR	= BIT(LLI_WR_DEC_ERR),
	DWAXIDMAC_IRQ_LLI_RD_SLV_ERR	= BIT(LLI_RD_SLV_ERR),
	DWAXIDMAC_IRQ_LLI_WR_SLV_ERR	= BIT(LLI_WR_SLV_ERR),
	DWAXIDMAC_IRQ_INVALID_ERR	= BIT(SHADOWREG_OR_LLI_INVALID_ERR),
	DWAXIDMAC_IRQ_MULTIBLKTYPE_ERR	= BIT(SLVIF_MULTIBLKTYPE_ERR),
	DWAXIDMAC_IRQ_DEC_ERR		= BIT(SLVIF_DEC_ERR),
	DWAXIDMAC_IRQ_WR2RO_ERR		= BIT(SLVIF_WR2RO_ERR),
	DWAXIDMAC_IRQ_RD2RWO_ERR	= BIT(SLVIF_RD2RWO_ERR),
	DWAXIDMAC_IRQ_WRONCHEN_ERR	= BIT(SLVIF_WRONCHEN_ERR),
	DWAXIDMAC_IRQ_SHADOWREG_ERR	= BIT(SLVIF_SHADOWREG_WRON_VALID_ERR),
	DWAXIDMAC_IRQ_WRONHOLD_ERR	= BIT(SLVIF_WRONHOLD_ERR),
	XR_IRQ_SLVIF_ADDRPARITY_ERR	= BIT(SLVIF_ADDRPARITY_ERR),
	XR_IRQ_SLVIF_WRPARITY_ERR	= BIT(SLVIF_WRPARITY_ERR),
	XR_IRQ_CH_LOCK_CLEARED		= BIT(CH_LOCK_CLEARED),
	DWAXIDMAC_IRQ_SRC_SUSPENDED	= BIT(CH_SRC_SUSPENDED),
	DWAXIDMAC_IRQ_SUSPENDED		= BIT(CH_SUSPENDED),
	DWAXIDMAC_IRQ_DISABLED		= BIT(CH_DISABLED),
	DWAXIDMAC_IRQ_ABORTED		= BIT(CH_ABORTED),
	XR_IRQ_ECC_PROT_CHMemCorrERR	= BIT(ECC_PROT_CHMem_CorrERR),
	XR_IRQ_ECC_PROT_UIDMemCorrERR	= BIT(ECC_PROT_UIDMem_CorrERR),
	XR_IRQ_ECC_PROT_UIDMemUnCorrERR	= BIT(ECC_PROT_UIDMem_UnCorrERR),
	DWAXIDMAC_IRQ_ALL_ERR		= XR_IRQ_ALL_ERR,
	DWAXIDMAC_IRQ_ALL		= GENMASK(31, 0)
};

enum {
	DWAXIDMAC_TRANS_WIDTH_8		= 0,
	DWAXIDMAC_TRANS_WIDTH_16,
	DWAXIDMAC_TRANS_WIDTH_32,
	DWAXIDMAC_TRANS_WIDTH_64,
	DWAXIDMAC_TRANS_WIDTH_128,
	DWAXIDMAC_TRANS_WIDTH_256,
	DWAXIDMAC_TRANS_WIDTH_512,
	DWAXIDMAC_TRANS_WIDTH_MAX	= DWAXIDMAC_TRANS_WIDTH_512
};

#ifdef CONFIG_DEBUG_FS

#define DEBUGFS_CHAN_NAME_LEN		24
#define DEBUGFS_FILE_MASK		0644

#define DEBUG_FS_PRINT_FORMAT(x)	((x) > 0 && !((x) % 8))

#define DEBUG_FS_DMA_WIDTH_TO_BYTE(x)	(1ul < (x))

#define DEBUG_FS_DMA_MISE(x) \
		((x) == DWAXIDMAC_BURST_TRANS_LEN_16 ? 16 : 0xFF)

u64 xr_axi_dma_debugfs_read_reg(struct axi_dma_chip *chip, u64 reg);
u64 xr_dma_chan_debugfs_read_reg(struct axi_dma_chan *chan, u32 reg);

int xr_dma_debugfs_init(struct axi_dma_chip *chip);
void xr_dma_cleanup_debugfs(struct axi_dma_chip *chip);

#endif

#endif /* _AXI_DMA_PLATFORM_H */
