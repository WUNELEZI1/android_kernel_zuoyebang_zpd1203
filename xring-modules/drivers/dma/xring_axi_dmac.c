// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * Synopsys DesignWare AXI DMA Controller driver.
 *
 * (C) 2017-2018 Synopsys, Inc. (www.synopsys.com)
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dmaengine.h>
#include <linux/dmapool.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/io-64-nonatomic-lo-hi.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_dma.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/reset.h>

#include "xring_axi_dmac.h"

/*
 * The set of bus widths supported by the DMA controller. DW AXI DMAC supports
 * master data bus width up to 512 bits (for both AXI master interfaces), but
 * it depends on IP block configuration.
 */
#define AXI_DMA_BUSWIDTHS		  \
	(DMA_SLAVE_BUSWIDTH_1_BYTE	| \
	DMA_SLAVE_BUSWIDTH_2_BYTES	| \
	DMA_SLAVE_BUSWIDTH_4_BYTES	| \
	DMA_SLAVE_BUSWIDTH_8_BYTES	| \
	DMA_SLAVE_BUSWIDTH_16_BYTES	| \
	DMA_SLAVE_BUSWIDTH_32_BYTES	| \
	DMA_SLAVE_BUSWIDTH_64_BYTES)

static inline void
axi_dma_iowrite32(struct axi_dma_chip *chip, u32 reg, u32 val)
{
	iowrite32(val, chip->regs + reg);
}

static inline void
axi_dma_iowrite64(struct axi_dma_chip *chip, u64 reg, u64 val)
{
	iowrite32(lower_32_bits(val), chip->regs + reg);
	iowrite32(upper_32_bits(val), XR_TO_HIGH_ADDRESS(chip->regs + reg));
}

static inline u32 axi_dma_ioread32(struct axi_dma_chip *chip, u32 reg)
{
	return ioread32(chip->regs + reg);
}

static inline u64 axi_dma_ioread64(struct axi_dma_chip *chip, u64 reg)
{
	u64 val_hi;
	u64 val_lo;

	val_hi = ioread32(XR_TO_HIGH_ADDRESS(chip->regs + reg));
	val_lo = ioread32(chip->regs + reg);
	val_hi = TO_READ_REG_VALUE(val_hi, val_lo);

	return val_hi;
}

static inline void
axi_chan_iowrite32(struct axi_dma_chan *chan, u32 reg, u32 val)
{
	iowrite32(val, chan->chan_regs + reg);
}

static inline u32 axi_chan_ioread32(struct axi_dma_chan *chan, u32 reg)
{
	return ioread32(chan->chan_regs + reg);
}

static inline void
axi_chan_iowrite64(struct axi_dma_chan *chan, u32 reg, u64 val)
{
	/*
	 * We split one 64 bit write for two 32 bit write as some HW doesn't
	 * support 64 bit access.
	 */
	iowrite32(lower_32_bits(val), chan->chan_regs + reg);
	iowrite32(upper_32_bits(val), XR_TO_HIGH_ADDRESS(chan->chan_regs + reg));
}

static inline void axi_chan_config_write(struct axi_dma_chan *chan,
					 struct axi_dma_chan_config *config)
{
	u32 cfg_lo;
	u32 cfg_hi;

	cfg_lo = (config->dst_multblk_type << CH_CFG_L_DST_MULTBLK_TYPE_POS |
		  config->src_multblk_type << CH_CFG_L_SRC_MULTBLK_TYPE_POS);
	if (XR_CHAN_LESS_EQUAL_THAN_8(chan->chip->dw->hdata->nr_channels)) {
		cfg_hi = config->tt_fc << CH_CFG_H_TT_FC_POS |
			 config->hs_sel_src << CH_CFG_H_HS_SEL_SRC_POS |
			 config->hs_sel_dst << CH_CFG_H_HS_SEL_DST_POS |
			 config->src_per << CH_CFG_H_SRC_PER_POS |
			 config->dst_per << CH_CFG_H_DST_PER_POS |
			 config->prior << CH_CFG_H_PRIORITY_POS;
	} else {
		cfg_lo |= config->src_per << CH_CFG2_L_SRC_PER_POS |
			  config->dst_per << CH_CFG2_L_DST_PER_POS;
		cfg_hi = config->tt_fc << CH_CFG2_H_TT_FC_POS |
			 config->hs_sel_src << CH_CFG2_H_HS_SEL_SRC_POS |
			 config->hs_sel_dst << CH_CFG2_H_HS_SEL_DST_POS |
			 config->prior << CH_CFG2_H_PRIORITY_POS;
	}
	axi_chan_iowrite32(chan, CH_CFG_L, cfg_lo);
	axi_chan_iowrite32(chan, CH_CFG_H, cfg_hi);

	dev_dbg(chan2dev(chan), "%s, %s, CH_CFG_H, CH_CFG_L: 0x%08x, 0x%08x\n",
		dma_chan_name(&chan->vc.chan), __func__, cfg_hi, cfg_lo);
}

static inline void axi_dma_disable(struct axi_dma_chip *chip)
{
	u32 val;
	unsigned long flags;

	raw_spin_lock_irqsave(&chip->dw->lock, flags);
	val = axi_dma_ioread32(chip, DMAC_CFGREG);
	val &= ~DMAC_EN_MASK;
	axi_dma_iowrite32(chip, DMAC_CFGREG, val);
	raw_spin_unlock_irqrestore(&chip->dw->lock, flags);
}

static inline void axi_dma_enable(struct axi_dma_chip *chip)
{
	u32 val;
	unsigned long flags;

	raw_spin_lock_irqsave(&chip->dw->lock, flags);
	val = axi_dma_ioread32(chip, DMAC_CFGREG);
	val |= DMAC_EN_MASK;
	axi_dma_iowrite32(chip, DMAC_CFGREG, val);
	raw_spin_unlock_irqrestore(&chip->dw->lock, flags);
}

static inline void axi_dma_irq_disable(struct axi_dma_chip *chip)
{
	u32 val;
	unsigned long flags;

	raw_spin_lock_irqsave(&chip->dw->lock, flags);
	val = axi_dma_ioread32(chip, DMAC_CFGREG);
	val &= ~INT_EN_MASK;
	axi_dma_iowrite32(chip, DMAC_CFGREG, val);
	raw_spin_unlock_irqrestore(&chip->dw->lock, flags);
}

static inline void axi_dma_irq_enable(struct axi_dma_chip *chip)
{
	u32 val;
	unsigned long flags;

	raw_spin_lock_irqsave(&chip->dw->lock, flags);
	val = axi_dma_ioread32(chip, DMAC_CFGREG);
	val |= INT_EN_MASK;
	axi_dma_iowrite32(chip, DMAC_CFGREG, val);
	raw_spin_unlock_irqrestore(&chip->dw->lock, flags);
}

static inline void axi_chan_irq_disable(struct axi_dma_chan *chan, u32 irq_mask)
{
	u32 val;

	if (likely(irq_mask == DWAXIDMAC_IRQ_ALL)) {
		axi_chan_iowrite32(chan, CH_INTSTATUS_ENA, DWAXIDMAC_IRQ_NONE);
	} else {
		val = axi_chan_ioread32(chan, CH_INTSTATUS_ENA);
		val &= ~irq_mask;
		axi_chan_iowrite32(chan, CH_INTSTATUS_ENA, val);
	}
}

static inline void axi_chan_irq_set(struct axi_dma_chan *chan, u32 irq_mask)
{
	axi_chan_iowrite32(chan, CH_INTSTATUS_ENA, irq_mask);
}

static inline void axi_chan_irq_sig_set(struct axi_dma_chan *chan, u32 irq_mask)
{
	axi_chan_iowrite32(chan, CH_INTSIGNAL_ENA, irq_mask);
}

static inline void axi_chan_irq_clear(struct axi_dma_chan *chan, u32 irq_mask)
{
	axi_chan_iowrite32(chan, CH_INTCLEAR, irq_mask);
}

static inline u32 axi_chan_irq_read(struct axi_dma_chan *chan)
{
	return axi_chan_ioread32(chan, CH_INTSTATUS);
}

static inline void axi_chan_disable(struct axi_dma_chan *chan)
{
	u64 val;

	val = axi_dma_ioread64(chan->chip, DMAC_CHENREG2);
	if (XR_CHAN_LESS_EQUAL_THAN_8(chan->chip->dw->hdata->nr_channels)) {
		val &= ~(BIT_ULL(chan->id) << DMAC_CHAN_EN_SHIFT);
		val |= BIT_ULL(chan->id) << DMAC_CHAN_EN_WE_SHIFT;
	} else {
		if (XR_CHAN_GREATER_THAN_16(chan->id)) {
			val &= (~((BIT_ULL(chan->id)) << XR_CHAN_17_32_EN2_SHIFT));
			val |= BIT_ULL(chan->id) << XR_CHAN_17_32_EN2_WE_SHIFT;
		} else {
			val &= (~((BIT_ULL(chan->id) << XR_CHAN_1_16_EN2_SHIFT)));
			val |= BIT_ULL(chan->id) << XR_CHAN_1_16_EN2_WE_SHIFT;
		}
	}
	axi_dma_iowrite64(chan->chip, DMAC_CHENREG2, val);
}

static inline void axi_chan_enable(struct axi_dma_chan *chan)
{
	u64 val;

	val = axi_dma_ioread64(chan->chip, DMAC_CHENREG2);
	if (XR_CHAN_LESS_EQUAL_THAN_8(chan->chip->dw->hdata->nr_channels)) {
		val |= BIT_ULL(chan->id) << DMAC_CHAN_EN_SHIFT |
			BIT_ULL(chan->id) << DMAC_CHAN_EN_WE_SHIFT;
	} else {
		if (XR_CHAN_GREATER_THAN_16(chan->id)) {
			val |= BIT_ULL(chan->id) << XR_CHAN_17_32_EN2_SHIFT |
				BIT_ULL(chan->id) << XR_CHAN_17_32_EN2_WE_SHIFT;
		} else {
			val |= BIT_ULL(chan->id) << XR_CHAN_1_16_EN2_SHIFT |
				BIT_ULL(chan->id) << XR_CHAN_1_16_EN2_WE_SHIFT;
		}
	}
	axi_dma_iowrite64(chan->chip, DMAC_CHENREG2, val);

	dev_dbg(chan2dev(chan), "%s, %s, enable OK, DMAC_CHENREG2:0x%016llx\n",
		dma_chan_name(&chan->vc.chan), __func__, val);
}

static inline bool axi_chan_is_hw_enable(struct axi_dma_chan *chan)
{
	u64 val;
	bool result;

	val = axi_dma_ioread64(chan->chip, DMAC_CHENREG2);

	if (XR_CHAN_LESS_EQUAL_THAN_8(chan->chip->dw->hdata->nr_channels)) {
		result = !!(val & (BIT(chan->id) << DMAC_CHAN_EN_SHIFT));
	} else {
		if (XR_CHAN_GREATER_THAN_16(chan->id))
			result = !!(val & (BIT_ULL(chan->id) << XR_CHAN_17_32_EN2_SHIFT));
		else
			result = !!(val & (BIT_ULL(chan->id) << XR_CHAN_1_16_EN2_SHIFT));
	}

	return result;
}

static void axi_dma_hw_init(struct axi_dma_chip *chip)
{
	int ret;
	u32 i;

	for (i = 0; i < chip->dw->hdata->nr_channels; i++) {
		axi_chan_irq_disable(&chip->dw->chan[i], DWAXIDMAC_IRQ_ALL);
		axi_chan_disable(&chip->dw->chan[i]);
	}
	ret = dma_set_mask_and_coherent(chip->dev, DMA_BIT_MASK(XR_DMA_BIT_MASK_BIT));
	if (ret)
		dev_warn(chip->dev, "Unable to set coherent mask\n");
}

static u32 axi_chan_get_xfer_width(struct axi_dma_chan *chan, dma_addr_t src,
				   dma_addr_t dst, size_t len)
{
	u32 max_width = chan->chip->dw->hdata->m_data_width;

	return __ffs(src | dst | len | BIT(max_width));
}

static inline const char *axi_chan_name(struct axi_dma_chan *chan)
{
	return dma_chan_name(&chan->vc.chan);
}

static struct axi_dma_desc *axi_desc_alloc(u32 num)
{
	struct axi_dma_desc *desc = NULL;

	desc = kzalloc(sizeof(*desc), GFP_NOWAIT);
	if (!desc)
		return NULL;

	desc->hw_desc = kcalloc(num, sizeof(*desc->hw_desc), GFP_NOWAIT);
	if (!desc->hw_desc) {
		kfree(desc);
		return NULL;
	}

	return desc;
}

static struct axi_dma_desc *xr_axi_desc_alloc(u32 num, struct axi_dma_chan *chan)
{
	struct axi_dma_desc *desc = NULL;

	desc = kzalloc(sizeof(*desc), GFP_NOWAIT);
	if (!desc)
		return NULL;

	if (chan->is_multi_block == XR_DMA_MULTI_BLOCK_MODE) {
		desc->single_hw_desc = NULL;
		desc->hw_desc = kcalloc(num, sizeof(*desc->hw_desc), GFP_NOWAIT);
		if (!desc->hw_desc) {
			kfree(desc);
			return NULL;
		}
	} else {
		desc->hw_desc = NULL;
		desc->single_hw_desc = kcalloc(num, sizeof(*desc->single_hw_desc), GFP_NOWAIT);
		if (!desc->single_hw_desc) {
			kfree(desc);
			return NULL;
		}
	}

	return desc;
}

static struct axi_dma_lli *axi_desc_get(struct axi_dma_chan *chan,
					dma_addr_t *addr)
{
	struct axi_dma_lli *lli = NULL;
	dma_addr_t phys;

	lli = dma_pool_zalloc(chan->desc_pool, GFP_NOWAIT, &phys);
	if (unlikely(!lli)) {
		dev_err(chan2dev(chan), "%s: not enough descriptors available\n",
			axi_chan_name(chan));
		return NULL;
	}

	atomic_inc(&chan->descs_allocated);
	*addr = phys;

	return lli;
}

static void axi_desc_put(struct axi_dma_desc *desc)
{
	struct axi_dma_chan *chan = desc->chan;
	int count = atomic_read(&chan->descs_allocated);
	struct axi_dma_hw_desc *hw_desc = NULL;
	int descs_put = 0;

	if (desc->hw_desc) {
		for (descs_put = 0; descs_put < count; descs_put++) {
			hw_desc = &desc->hw_desc[descs_put];
			dma_pool_free(chan->desc_pool, hw_desc->lli, hw_desc->llp);
		}

		kfree(desc->hw_desc);
	}

	kfree(desc->single_hw_desc);
	kfree(desc);
	atomic_sub(descs_put, &chan->descs_allocated);
	dev_vdbg(chan2dev(chan), "%s: %d descs put, %d still allocated\n",
		axi_chan_name(chan), descs_put,
		atomic_read(&chan->descs_allocated));
}

static void vchan_desc_put(struct virt_dma_desc *vdesc)
{
	axi_desc_put(vd_to_axi_desc(vdesc));
}

static enum dma_status
dma_chan_tx_status(struct dma_chan *dchan, dma_cookie_t cookie,
		  struct dma_tx_state *txstate)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);
	struct virt_dma_desc *vdesc = NULL;
	enum dma_status status;
	u32 completed_length;
	unsigned long flags;
	u32 completed_blocks;
	size_t bytes = 0;
	u32 length;
	u32 len;

	status = dma_cookie_status(dchan, cookie, txstate);
	if (status == DMA_COMPLETE || !txstate)
		return status;

	spin_lock_irqsave(&chan->vc.lock, flags);

	vdesc = vchan_find_desc(&chan->vc, cookie);
	if (vdesc) {
		length = vd_to_axi_desc(vdesc)->length;
		completed_blocks = vd_to_axi_desc(vdesc)->completed_blocks;

		if (chan->is_multi_block == XR_DMA_MULTI_BLOCK_MODE)
			len = vd_to_axi_desc(vdesc)->hw_desc[0].len;
		else
			len = vd_to_axi_desc(vdesc)->single_hw_desc->len;

		completed_length = completed_blocks * len;
		bytes = length - completed_length;
	}

	spin_unlock_irqrestore(&chan->vc.lock, flags);
	dma_set_residue(txstate, bytes);

	return status;
}

static void write_desc_llp(struct axi_dma_hw_desc *desc, dma_addr_t adr)
{
	desc->lli->llp = cpu_to_le64(adr);
}

static void write_chan_llp(struct axi_dma_chan *chan, dma_addr_t adr)
{
	axi_chan_iowrite64(chan, CH_LLP, adr);
}

/* Called in chan locked context */
static void axi_chan_block_xfer_start(struct axi_dma_chan *chan,
				      struct axi_dma_desc *first)
{
	u32 priority = chan->chip->dw->hdata->priority[chan->id];
	struct axi_dma_chan_config config = {};
	u32 irq_mask;
	u8 lms = 0; /* Select AXI0 master for LLI fetching */

	if (unlikely(axi_chan_is_hw_enable(chan))) {
		dev_err(chan2dev(chan), "%s is non-idle!\n",
			axi_chan_name(chan));

		return;
	}

	axi_dma_enable(chan->chip);

	if (chan->is_multi_block == XR_DMA_MULTI_BLOCK_MODE) {
		config.dst_multblk_type = DWAXIDMAC_MBLK_TYPE_LL;
		config.src_multblk_type = DWAXIDMAC_MBLK_TYPE_LL;
	} else {
		config.dst_multblk_type = DWAXIDMAC_MBLK_TYPE_CONTIGUOUS;
		config.src_multblk_type = DWAXIDMAC_MBLK_TYPE_CONTIGUOUS;
	}

	config.tt_fc = DWAXIDMAC_TT_FC_MEM_TO_MEM_DMAC;
	config.prior = priority;
	config.hs_sel_dst = DWAXIDMAC_HS_SEL_HW;
	config.hs_sel_src = DWAXIDMAC_HS_SEL_HW;
	switch (chan->direction) {
	case DMA_MEM_TO_DEV:
		config.tt_fc = chan->config.device_fc ?
				DWAXIDMAC_TT_FC_MEM_TO_PER_DST :
				DWAXIDMAC_TT_FC_MEM_TO_PER_DMAC;
		config.dst_per = chan->hw_handshake_num;
		break;
	case DMA_DEV_TO_MEM:
		config.tt_fc = chan->config.device_fc ?
				DWAXIDMAC_TT_FC_PER_TO_MEM_SRC :
				DWAXIDMAC_TT_FC_PER_TO_MEM_DMAC;
		config.src_per = chan->hw_handshake_num;
		break;
	default:
		break;
	}
	axi_chan_config_write(chan, &config);

	dev_dbg(chan2dev(chan), "%s, %s, hw_handshake_num:%u\n",
		axi_chan_name(chan), __func__, chan->hw_handshake_num);

	if (chan->is_multi_block == XR_DMA_MULTI_BLOCK_MODE) {
		write_chan_llp(chan, first->hw_desc[0].llp | lms);
	} else {
		/* src/dst address */
		axi_chan_iowrite64(chan, CH_SAR, first->single_hw_desc->sar);
		axi_chan_iowrite64(chan, CH_DAR, first->single_hw_desc->dar);

		/* block size */
		axi_chan_iowrite32(chan, CH_BLOCK_TS,
			cpu_to_le32(XR_TO_BLOCK_SIZE(first->single_hw_desc->block_ts)));

		axi_chan_iowrite64(chan, CH_CTL, first->single_hw_desc->ctl);
	}

	irq_mask = DWAXIDMAC_IRQ_DMA_TRF | DWAXIDMAC_IRQ_ALL_ERR;
	axi_chan_irq_sig_set(chan, irq_mask);

	/* Generate 'suspend' status but don't generate interrupt */
	irq_mask |= DWAXIDMAC_IRQ_SUSPENDED;
	axi_chan_irq_set(chan, irq_mask);

	axi_chan_enable(chan);
}

static void axi_chan_start_first_queued(struct axi_dma_chan *chan)
{
	struct axi_dma_desc *desc = NULL;
	struct virt_dma_desc *vd = NULL;

	vd = vchan_next_desc(&chan->vc);
	if (!vd)
		return;

	desc = vd_to_axi_desc(vd);

	dev_vdbg(chan2dev(chan), "%s: started %u\n", axi_chan_name(chan),
		vd->tx.cookie);

	axi_chan_block_xfer_start(chan, desc);
}

static void dma_chan_issue_pending(struct dma_chan *dchan)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);
	unsigned long flags;

	spin_lock_irqsave(&chan->vc.lock, flags);
	if (vchan_issue_pending(&chan->vc))
		axi_chan_start_first_queued(chan);
	spin_unlock_irqrestore(&chan->vc.lock, flags);
}

static void dw_axi_dma_synchronize(struct dma_chan *dchan)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);

	vchan_synchronize(&chan->vc);
}

static int dma_chan_alloc_chan_resources(struct dma_chan *dchan)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);

	/* ASSERT: channel is idle */
	if (axi_chan_is_hw_enable(chan)) {
		dev_err(chan2dev(chan), "%s is non-idle!\n",
			axi_chan_name(chan));
		return -EBUSY;
	}

	/* LLI address must be aligned to a 64-byte boundary */
	chan->desc_pool = dma_pool_create(dev_name(chan2dev(chan)),
					  chan->chip->dev,
					  sizeof(struct axi_dma_lli),
					  XR_LLI_DMA_POOL_ALIGN,
					  XR_LLI_DMA_POOL_BOUNDARY);
	if (!chan->desc_pool) {
		dev_err(chan2dev(chan), "No memory for descriptors\n");
		return -ENOMEM;
	}
	dev_vdbg(dchan2dev(dchan), "%s: allocating\n", axi_chan_name(chan));

	return 0;
}

static void dma_chan_free_chan_resources(struct dma_chan *dchan)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);

	/* ASSERT: channel is idle */
	if (axi_chan_is_hw_enable(chan))
		dev_err(dchan2dev(dchan), "%s is non-idle!\n",
			axi_chan_name(chan));

	axi_chan_disable(chan);
	axi_chan_irq_disable(chan, DWAXIDMAC_IRQ_ALL);

	vchan_free_chan_resources(&chan->vc);

	dma_pool_destroy(chan->desc_pool);
	chan->desc_pool = NULL;
	dev_vdbg(dchan2dev(dchan),
		 "%s: free resources, descriptor still allocated: %u\n",
		 axi_chan_name(chan), atomic_read(&chan->descs_allocated));
}

/*
 * If DW_axi_dmac sees CHx_CTL.ShadowReg_Or_LLI_Last bit of the fetched LLI
 * as 1, it understands that the current block is the final block in the
 * transfer and completes the DMA transfer operation at the end of current
 * block transfer.
 */
static void set_desc_last(struct axi_dma_hw_desc *desc)
{
	u32 val;

	val = le32_to_cpu(desc->lli->ctl_hi);
	val |= CH_CTL_H_LLI_LAST;
	desc->lli->ctl_hi = cpu_to_le32(val);
}

static void write_desc_sar(struct axi_dma_hw_desc *desc, dma_addr_t adr)
{
	desc->lli->sar = cpu_to_le64(adr);
}

static void write_desc_dar(struct axi_dma_hw_desc *desc, dma_addr_t adr)
{
	desc->lli->dar = cpu_to_le64(adr);
}

static void set_desc_src_master(struct axi_dma_hw_desc *desc)
{
	u32 val;

	/* Select AXI0 for source master */
	val = le32_to_cpu(desc->lli->ctl_lo);
	val &= ~CH_CTL_L_SRC_MAST;
	desc->lli->ctl_lo = cpu_to_le32(val);
}

static void set_desc_dest_master(struct axi_dma_hw_desc *hw_desc,
				 struct axi_dma_desc *desc)
{
	u32 val;

	/* Select AXI1 for source master if available */
	val = le32_to_cpu(hw_desc->lli->ctl_lo);
	if (desc->chan->chip->dw->hdata->nr_masters > 1)
		val |= CH_CTL_L_DST_MAST;
	else
		val &= ~CH_CTL_L_DST_MAST;

	hw_desc->lli->ctl_lo = cpu_to_le32(val);
}

static int dw_axi_dma_set_hw_desc(struct axi_dma_chan *chan,
				  struct axi_dma_hw_desc *hw_desc,
				  dma_addr_t mem_addr, size_t len)
{
	u32 data_width = BIT(chan->chip->dw->hdata->m_data_width);
	u32 reg_width;
	u32 mem_width;
	dma_addr_t device_addr;
	size_t axi_block_ts;
	size_t block_ts;
	u32 ctllo;
	u32 ctlhi;

	axi_block_ts = chan->chip->dw->hdata->block_size[chan->id];

	mem_width = __ffs(data_width | mem_addr | len);
	if (mem_width > DWAXIDMAC_TRANS_WIDTH_32)
		mem_width = DWAXIDMAC_TRANS_WIDTH_32;

	dev_dbg(chan2dev(chan), "%s, mem_width:0x%08x\n",
		axi_chan_name(chan), mem_width);

	if (!IS_ALIGNED(mem_addr, XR_MEM_DEV_ADDR_ALIGN_BYTE)) {
		dev_err(chan->chip->dev, "invalid buffer alignment\n");
		return -EINVAL;
	}

	switch (chan->direction) {
	case DMA_MEM_TO_DEV:
		if (chan->config.dst_addr_width == 0)
			return -EINVAL;

		reg_width = __ffs(chan->config.dst_addr_width);
		device_addr = chan->config.dst_addr;
		ctllo = reg_width << CH_CTL_L_DST_WIDTH_POS |
			mem_width << CH_CTL_L_SRC_WIDTH_POS |
			DWAXIDMAC_CH_CTL_L_NOINC << CH_CTL_L_DST_INC_POS |
			DWAXIDMAC_CH_CTL_L_INC << CH_CTL_L_SRC_INC_POS;
		block_ts = len >> mem_width;
		break;
	case DMA_DEV_TO_MEM:
		if (chan->config.src_addr_width == 0)
			return -EINVAL;

		reg_width = __ffs(chan->config.src_addr_width);
		device_addr = chan->config.src_addr;
		ctllo = reg_width << CH_CTL_L_SRC_WIDTH_POS |
			mem_width << CH_CTL_L_DST_WIDTH_POS |
			DWAXIDMAC_CH_CTL_L_INC << CH_CTL_L_DST_INC_POS |
			DWAXIDMAC_CH_CTL_L_NOINC << CH_CTL_L_SRC_INC_POS;
		block_ts = len >> reg_width;
		break;
	default:
		return -EINVAL;
	}

	dev_dbg(chan2dev(chan), "%s, consumer reg_width:0x%08x\n",
		axi_chan_name(chan), reg_width);

	dev_dbg(chan2dev(chan), "%s, item block size:0x%08zx\n",
		axi_chan_name(chan), block_ts);

	if (block_ts > axi_block_ts)
		return -EINVAL;

	hw_desc->lli = axi_desc_get(chan, &hw_desc->llp);
	if (unlikely(!hw_desc->lli))
		return -ENOMEM;

	ctlhi = CH_CTL_H_LLI_VALID;

	hw_desc->lli->ctl_hi = cpu_to_le32(ctlhi);

	if (chan->direction == DMA_MEM_TO_DEV) {
		write_desc_sar(hw_desc, mem_addr);
		write_desc_dar(hw_desc, device_addr);
	} else {
		write_desc_sar(hw_desc, device_addr);
		write_desc_dar(hw_desc, mem_addr);
	}

	dev_dbg(chan2dev(chan), "%s, mem_addr:0x%llx, device_addr:0x%llx\n",
		axi_chan_name(chan), mem_addr, device_addr);

	hw_desc->lli->block_ts_lo = cpu_to_le32(XR_TO_BLOCK_SIZE(block_ts));

	ctllo |= DWAXIDMAC_BURST_TRANS_LEN_16 << CH_CTL_L_DST_MSIZE_POS |
		 DWAXIDMAC_BURST_TRANS_LEN_16 << CH_CTL_L_SRC_MSIZE_POS;
	hw_desc->lli->ctl_lo = cpu_to_le32(ctllo);

	set_desc_src_master(hw_desc);

	hw_desc->len = len;

	dev_dbg(chan2dev(chan), "%s, CTL-H:0x%08x, CTL-L:0x%08x\n\n",
		axi_chan_name(chan), hw_desc->lli->ctl_hi, hw_desc->lli->ctl_lo);

	return 0;
}

static int xr_axi_dma_set_single_hw_desc(struct axi_dma_chan *chan,
				struct xr_dma_single_hw_desc *single_hw_desc,
				dma_addr_t mem_addr, size_t len)
{
	u32 data_width = BIT(chan->chip->dw->hdata->m_data_width);
	u32 reg_width;
	u32 mem_width;
	dma_addr_t device_addr;
	size_t axi_block_ts;
	size_t block_ts;
	u32 ctllo = 0;

	axi_block_ts = chan->chip->dw->hdata->block_size[chan->id];

	mem_width = __ffs(data_width | mem_addr | len);
	if (mem_width > DWAXIDMAC_TRANS_WIDTH_32)
		mem_width = DWAXIDMAC_TRANS_WIDTH_32;

	dev_dbg(chan2dev(chan), "%s, mem_width:0x%08x\n",
		axi_chan_name(chan), mem_width);

	if (!IS_ALIGNED(mem_addr, BIT(mem_width))) {
		dev_err(chan->chip->dev, "invalid buffer alignment\n");
		return -EINVAL;
	}

	ctllo &= ~CH_CTL_L_SRC_MAST;
	ctllo &= ~CH_CTL_L_DST_MAST;

	switch (chan->direction) {
	case DMA_MEM_TO_DEV:
		if (chan->config.dst_addr_width == 0)
			return -EINVAL;

		reg_width = __ffs(chan->config.dst_addr_width);
		device_addr = chan->config.dst_addr;
		ctllo = reg_width << CH_CTL_L_DST_WIDTH_POS |
			mem_width << CH_CTL_L_SRC_WIDTH_POS |
			DWAXIDMAC_CH_CTL_L_NOINC << CH_CTL_L_DST_INC_POS |
			DWAXIDMAC_CH_CTL_L_INC << CH_CTL_L_SRC_INC_POS;
		block_ts = len >> mem_width;
		break;
	case DMA_DEV_TO_MEM:
		if (chan->config.src_addr_width == 0)
			return -EINVAL;

		reg_width = __ffs(chan->config.src_addr_width);
		device_addr = chan->config.src_addr;
		ctllo = reg_width << CH_CTL_L_SRC_WIDTH_POS |
			mem_width << CH_CTL_L_DST_WIDTH_POS |
			DWAXIDMAC_CH_CTL_L_INC << CH_CTL_L_DST_INC_POS |
			DWAXIDMAC_CH_CTL_L_NOINC << CH_CTL_L_SRC_INC_POS;
		block_ts = len >> reg_width;
		break;
	default:
		return -EINVAL;
	}

	dev_dbg(chan2dev(chan), "%s, consumer reg_width:0x%08x\n",
		axi_chan_name(chan), reg_width);

	dev_dbg(chan2dev(chan), "%s, item block size:0x%08zx\n",
		axi_chan_name(chan), block_ts);

	if (block_ts > axi_block_ts)
		return -EINVAL;

	single_hw_desc->block_ts = block_ts;

	ctllo |= DWAXIDMAC_BURST_TRANS_LEN_16 << CH_CTL_L_DST_MSIZE_POS |
		 DWAXIDMAC_BURST_TRANS_LEN_16 << CH_CTL_L_SRC_MSIZE_POS;

	single_hw_desc->ctl = cpu_to_le32(ctllo);

	if (chan->direction == DMA_MEM_TO_DEV) {
		single_hw_desc->sar = mem_addr;
		single_hw_desc->dar = device_addr;
	} else {
		single_hw_desc->sar = device_addr;
		single_hw_desc->dar = mem_addr;
	}

	dev_dbg(chan2dev(chan), "%s, mem_addr:0x%llx, device_addr:0x%llx\n",
		axi_chan_name(chan), mem_addr, device_addr);

	dev_dbg(chan2dev(chan), "%s, CTL:0x%016llx\n",
		axi_chan_name(chan), single_hw_desc->ctl);

	return 0;
}

static size_t calculate_block_len(struct axi_dma_chan *chan,
				  dma_addr_t dma_addr, size_t buf_len,
				  enum dma_transfer_direction direction)
{
	u32 data_width;
	u32 reg_width;
	u32 mem_width;
	size_t axi_block_ts;
	size_t block_len;

	axi_block_ts = chan->chip->dw->hdata->block_size[chan->id];

	switch (direction) {
	case DMA_MEM_TO_DEV:
		data_width = BIT(chan->chip->dw->hdata->m_data_width);
		mem_width = __ffs(data_width | dma_addr | buf_len);
		if (mem_width > DWAXIDMAC_TRANS_WIDTH_32)
			mem_width = DWAXIDMAC_TRANS_WIDTH_32;

		block_len = axi_block_ts << mem_width;
		break;
	case DMA_DEV_TO_MEM:
		if (chan->config.src_addr_width == 0) {
			block_len = 0;
		} else {
			reg_width = __ffs(chan->config.src_addr_width);
			block_len = axi_block_ts << reg_width;
		}
		break;
	default:
		block_len = 0;
		break;
	}

	dev_dbg(chan2dev(chan), "%s, axi_block_len, dts:0x%x, calculate(bts*width):0x%zx\n",
		axi_chan_name(chan),
		chan->chip->dw->hdata->block_size[chan->id],
		block_len);

	return block_len;
}

static struct dma_async_tx_descriptor *
dw_axi_dma_chan_prep_cyclic(struct dma_chan *dchan, dma_addr_t dma_addr,
			    size_t buf_len, size_t period_len,
			    enum dma_transfer_direction direction,
			    unsigned long flags)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);
	struct axi_dma_hw_desc *hw_desc = NULL;
	struct axi_dma_desc *desc = NULL;
	dma_addr_t src_addr = dma_addr;
	u32 num_periods;
	u32 num_segments;
	size_t axi_block_len;
	u32 total_segments;
	u32 segment_len;
	unsigned int i;
	int status;
	u64 llp = 0;
	u8 lms = 0; /* Select AXI0 master for LLI fetching */

	num_periods = buf_len / period_len;

	axi_block_len = calculate_block_len(chan, dma_addr, buf_len, direction);
	if (axi_block_len == 0)
		return NULL;

	chan->is_multi_block = XR_DMA_MULTI_BLOCK_MODE;

	num_segments = DIV_ROUND_UP(period_len, axi_block_len);
	segment_len = DIV_ROUND_UP(period_len, num_segments);

	total_segments = num_periods * num_segments;

	desc = axi_desc_alloc(total_segments);
	if (unlikely(!desc))
		goto err_desc_get;

	chan->direction = direction;
	desc->chan = chan;
	chan->cyclic = true;
	desc->length = 0;
	desc->period_len = period_len;

	for (i = 0; i < total_segments; i++) {
		hw_desc = &desc->hw_desc[i];

		status = dw_axi_dma_set_hw_desc(chan, hw_desc, src_addr,
						segment_len);
		if (status < 0)
			goto err_desc_get;

		desc->length += hw_desc->len;
		/* Set end-of-link to the linked descriptor, so that cyclic
		 * callback function can be triggered during interrupt.
		 */
		set_desc_last(hw_desc);

		src_addr += segment_len;
	}

	llp = desc->hw_desc[0].llp;

	/* Managed transfer list */
	do {
		hw_desc = &desc->hw_desc[--total_segments];
		write_desc_llp(hw_desc, llp | lms);
		llp = hw_desc->llp;
	} while (total_segments);

	return vchan_tx_prep(&chan->vc, &desc->vd, flags);

err_desc_get:
	if (desc)
		axi_desc_put(desc);

	return NULL;
}

static struct dma_async_tx_descriptor *
dw_axi_dma_chan_prep_slave_sg(struct dma_chan *dchan, struct scatterlist *sgl,
			      unsigned int sg_len,
			      enum dma_transfer_direction direction,
			      unsigned long flags, void *context)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);
	struct axi_dma_hw_desc *hw_desc = NULL;
	struct axi_dma_desc *desc = NULL;
	u32 num_segments;
	u32 segment_len;
	unsigned int loop = 0;
	struct scatterlist *sg = NULL;
	size_t axi_block_len;  /* item size, block * width */
	u32 len;
	u32 num_sgs = 0;
	unsigned int i;
	dma_addr_t mem;
	int status;
	u64 llp = 0;
	u8 lms = 0; /* Select AXI0 master for LLI fetching */

	if (unlikely(!is_slave_direction(direction) || !sg_len))
		return NULL;

	mem = sg_dma_address(sgl);
	len = sg_dma_len(sgl);

	axi_block_len = calculate_block_len(chan, mem, len, direction);
	if (axi_block_len == 0)
		return NULL;

	for_each_sg(sgl, sg, sg_len, i)
		num_sgs += DIV_ROUND_UP(sg_dma_len(sg), axi_block_len);

	chan->is_multi_block = XR_DMA_MULTI_BLOCK_MODE;
	/**
	 * If the number of sg list members is 1 and
	 * the data to be moved is less than (block size * width),
	 * that is, the number of data transmission descriptors to be constructed is 1,
	 * use contiguous to contiguous mode.
	 */
	if ((sg_len == 1) && (num_sgs == 1) && (len < axi_block_len))
		chan->is_multi_block = !XR_DMA_MULTI_BLOCK_MODE;

	desc = xr_axi_desc_alloc(num_sgs, chan);
	if (unlikely(!desc))
		goto err_desc_get;

	chan->direction = direction;
	desc->chan = chan;

	if (chan->is_multi_block != XR_DMA_MULTI_BLOCK_MODE) {
		num_segments = DIV_ROUND_UP(len, axi_block_len);
		segment_len = DIV_ROUND_UP(len, num_segments);

		dev_dbg(chan2dev(chan),
			"%s, %u, sg_dma_address(sg):0x%llx, sg_dma_len(sg):0x%x\n",
			axi_chan_name(chan), i, mem, len);
		dev_dbg(chan2dev(chan),
			"%s, %u, num_segments:0x%x, segment_len:0x%x\n",
			axi_chan_name(chan), i, num_segments, segment_len);

		status = xr_axi_dma_set_single_hw_desc(chan,
				desc->single_hw_desc, mem, segment_len);
		if (status < 0)
			goto err_desc_get;

		desc->length = segment_len;
		desc->single_hw_desc->len = segment_len;

		return vchan_tx_prep(&chan->vc, &desc->vd, flags);
	}

	desc->length = 0;
	chan->direction = direction;

	for_each_sg(sgl, sg, sg_len, i) {
		mem = sg_dma_address(sg);
		len = sg_dma_len(sg);
		num_segments = DIV_ROUND_UP(sg_dma_len(sg), axi_block_len);
		segment_len = DIV_ROUND_UP(sg_dma_len(sg), num_segments);

		dev_dbg(chan2dev(chan), "%s, %u, sg_dma_address(sg):0x%llx, sg_dma_len(sg):0x%x\n",
			axi_chan_name(chan), i, mem, len);
		dev_dbg(chan2dev(chan), "%s, %u, num_segments:0x%x, segment_len:0x%x\n",
			axi_chan_name(chan), i, num_segments, segment_len);

		do {
			hw_desc = &desc->hw_desc[loop++];
			status = dw_axi_dma_set_hw_desc(chan, hw_desc, mem, segment_len);
			if (status < 0)
				goto err_desc_get;

			desc->length += hw_desc->len;
			len -= segment_len;
			mem += segment_len;
		} while (len >= segment_len);
	}

	/* Set end-of-link to the last link descriptor of list */
	set_desc_last(&desc->hw_desc[num_sgs - 1]);

	/* Managed transfer list */
	do {
		hw_desc = &desc->hw_desc[--num_sgs];
		write_desc_llp(hw_desc, llp | lms);
		llp = hw_desc->llp;
	} while (num_sgs);

	return vchan_tx_prep(&chan->vc, &desc->vd, flags);

err_desc_get:
	if (desc)
		axi_desc_put(desc);

	return NULL;
}

static struct dma_async_tx_descriptor *
dma_chan_prep_dma_memcpy(struct dma_chan *dchan, dma_addr_t dst_adr,
			 dma_addr_t src_adr, size_t len, unsigned long flags)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);
	size_t block_ts;
	size_t max_block_ts;
	size_t xfer_len;
	size_t item_size;
	struct axi_dma_hw_desc *hw_desc = NULL;
	struct axi_dma_desc *desc = NULL;
	u32 xfer_width;
	u32 reg = 0;
	u32 num;
	u64 llp = 0;
	u8 lms = 0; /* Select AXI0 master for LLI fetching */

	dev_dbg(chan2dev(chan), "%s: memcpy: src: %pad dst: %pad length: %zd flags: %#lx",
		axi_chan_name(chan), &src_adr, &dst_adr, len, flags);

	max_block_ts = chan->chip->dw->hdata->block_size[chan->id];
	xfer_width = axi_chan_get_xfer_width(chan, src_adr, dst_adr, len);

	if (!IS_ALIGNED(dst_adr, BIT_ULL(xfer_width))) {
		dev_err(chan->chip->dev, "dst, invalid buffer alignment\n");
		return NULL;
	}

	if (!IS_ALIGNED(src_adr, BIT_ULL(xfer_width))) {
		dev_err(chan->chip->dev, "src, invalid buffer alignment\n");
		return NULL;
	}

	chan->direction = DMA_MEM_TO_MEM;
	chan->is_multi_block = XR_DMA_MULTI_BLOCK_MODE;

	item_size = max_block_ts << xfer_width;
	if (len < item_size)
		chan->is_multi_block = !XR_DMA_MULTI_BLOCK_MODE;

	num = DIV_ROUND_UP(len, item_size);
	desc = xr_axi_desc_alloc(num, chan);
	if (unlikely(!desc))
		goto err_desc_get;

	desc->chan = chan;
	if (len < item_size) {
		desc->single_hw_desc->sar = src_adr;
		desc->single_hw_desc->dar = dst_adr;

		desc->single_hw_desc->block_ts = len >> xfer_width;

		reg = (DWAXIDMAC_BURST_TRANS_LEN_16 << CH_CTL_L_DST_MSIZE_POS |
		       DWAXIDMAC_BURST_TRANS_LEN_16 << CH_CTL_L_SRC_MSIZE_POS |
		       xfer_width << CH_CTL_L_DST_WIDTH_POS |
		       xfer_width << CH_CTL_L_SRC_WIDTH_POS |
		       DWAXIDMAC_CH_CTL_L_INC << CH_CTL_L_DST_INC_POS |
		       DWAXIDMAC_CH_CTL_L_INC << CH_CTL_L_SRC_INC_POS);
		reg &= ~CH_CTL_L_SRC_MAST;
		reg &= ~CH_CTL_L_DST_MAST;
		desc->single_hw_desc->ctl = cpu_to_le32(reg);

		desc->single_hw_desc->len = len;
		desc->length = len;

		return vchan_tx_prep(&chan->vc, &desc->vd, flags);
	}

	num = 0;
	desc->length = 0;
	while (len) {
		xfer_len = len;

		hw_desc = &desc->hw_desc[num];
		/*
		 * Take care for the alignment.
		 * Actually source and destination widths can be different, but
		 * make them same to be simpler.
		 */
		xfer_width = axi_chan_get_xfer_width(chan, src_adr, dst_adr, xfer_len);

		/*
		 * block_ts indicates the total number of data of width
		 * to be transferred in a DMA block transfer.
		 * BLOCK_TS register should be set to block_ts - 1
		 */
		block_ts = xfer_len >> xfer_width;
		if (block_ts > max_block_ts) {
			block_ts = max_block_ts;
			xfer_len = max_block_ts << xfer_width;
		}

		hw_desc->lli = axi_desc_get(chan, &hw_desc->llp);
		if (unlikely(!hw_desc->lli))
			goto err_desc_get;

		write_desc_sar(hw_desc, src_adr);
		write_desc_dar(hw_desc, dst_adr);

		hw_desc->lli->block_ts_lo = cpu_to_le32(block_ts - 1);

		reg = CH_CTL_H_LLI_VALID;
		if (chan->chip->dw->hdata->restrict_axi_burst_len) {
			u32 burst_len = chan->chip->dw->hdata->axi_rw_burst_len;

			reg |= (CH_CTL_H_ARLEN_EN |
				burst_len << CH_CTL_H_ARLEN_POS |
				CH_CTL_H_AWLEN_EN |
				burst_len << CH_CTL_H_AWLEN_POS);
		}
		hw_desc->lli->ctl_hi = cpu_to_le32(reg);

		reg = (DWAXIDMAC_BURST_TRANS_LEN_4 << CH_CTL_L_DST_MSIZE_POS |
		       DWAXIDMAC_BURST_TRANS_LEN_4 << CH_CTL_L_SRC_MSIZE_POS |
		       xfer_width << CH_CTL_L_DST_WIDTH_POS |
		       xfer_width << CH_CTL_L_SRC_WIDTH_POS |
		       DWAXIDMAC_CH_CTL_L_INC << CH_CTL_L_DST_INC_POS |
		       DWAXIDMAC_CH_CTL_L_INC << CH_CTL_L_SRC_INC_POS);
		hw_desc->lli->ctl_lo = cpu_to_le32(reg);

		set_desc_src_master(hw_desc);
		set_desc_dest_master(hw_desc, desc);

		hw_desc->len = xfer_len;
		desc->length += hw_desc->len;
		/* update the length and addresses for the next loop cycle */
		len -= xfer_len;
		dst_adr += xfer_len;
		src_adr += xfer_len;
		num++;
	}

	/* Set end-of-link to the last link descriptor of list */
	set_desc_last(&desc->hw_desc[num - 1]);
	/* Managed transfer list */
	do {
		hw_desc = &desc->hw_desc[--num];
		write_desc_llp(hw_desc, llp | lms);
		llp = hw_desc->llp;
	} while (num);

	return vchan_tx_prep(&chan->vc, &desc->vd, flags);

err_desc_get:
	if (desc)
		axi_desc_put(desc);
	return NULL;
}

static int dw_axi_dma_chan_slave_config(struct dma_chan *dchan,
					struct dma_slave_config *config)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);

	memcpy(&chan->config, config, sizeof(*config));

	return 0;
}

static void axi_chan_dump_lli(struct axi_dma_chan *chan,
			      struct axi_dma_hw_desc *desc)
{
	if (!desc->lli) {
		dev_err(dchan2dev(&chan->vc.chan), "NULL LLI\n");
		return;
	}

	dev_err(dchan2dev(&chan->vc.chan),
		"SAR: 0x%llx DAR: 0x%llx LLP: 0x%llx BTS 0x%x CTL: 0x%x:%08x",
		le64_to_cpu(desc->lli->sar),
		le64_to_cpu(desc->lli->dar),
		le64_to_cpu(desc->lli->llp),
		le32_to_cpu(desc->lli->block_ts_lo),
		le32_to_cpu(desc->lli->ctl_hi),
		le32_to_cpu(desc->lli->ctl_lo));
}

static void axi_chan_list_dump_lli(struct axi_dma_chan *chan,
				   struct axi_dma_desc *desc_head)
{
	int count = atomic_read(&chan->descs_allocated);
	int i;

	if (chan->is_multi_block == XR_DMA_MULTI_BLOCK_MODE) {
		for (i = 0; i < count; i++)
			axi_chan_dump_lli(chan, &desc_head->hw_desc[i]);
	} else {
		dev_info(chan2dev(chan), "ctl: 0x%llx\n", desc_head->single_hw_desc->ctl);
		dev_info(chan2dev(chan), "block_ts: 0x%x\n", desc_head->single_hw_desc->block_ts);
		dev_info(chan2dev(chan), "src_addr: 0x%llx\n", desc_head->single_hw_desc->sar);
		dev_info(chan2dev(chan), "dst_addr: 0x%llx\n", desc_head->single_hw_desc->dar);
	}
}

static noinline void axi_chan_handle_err(struct axi_dma_chan *chan, u32 status)
{
	struct virt_dma_desc *vd = NULL;
	unsigned long flags;

	spin_lock_irqsave(&chan->vc.lock, flags);

	axi_chan_disable(chan);

	/* The bad descriptor currently is in the head of vc list */
	vd = vchan_next_desc(&chan->vc);
	if (!vd) {
		dev_err(chan2dev(chan), "BUG: %s, IRQ with no descriptors\n",
			axi_chan_name(chan));
		goto out;
	}

	/* Remove the completed descriptor from issued list */
	list_del(&vd->node);

	/* WARN about bad descriptor */
	dev_err(chan2dev(chan),
		"Bad descriptor submitted for %s, cookie: %d, irq: 0x%08x\n",
		axi_chan_name(chan), vd->tx.cookie, status);
	axi_chan_list_dump_lli(chan, vd_to_axi_desc(vd));

	vchan_cookie_complete(vd);

	/* Try to restart the controller */
	axi_chan_start_first_queued(chan);

out:
	spin_unlock_irqrestore(&chan->vc.lock, flags);
}

static void axi_chan_block_xfer_complete(struct axi_dma_chan *chan)
{
	int count = atomic_read(&chan->descs_allocated);
	struct axi_dma_hw_desc *hw_desc = NULL;
	struct axi_dma_desc *desc = NULL;
	struct virt_dma_desc *vd = NULL;
	unsigned long flags;
	u64 llp;
	int i;

	spin_lock_irqsave(&chan->vc.lock, flags);
	if (unlikely(axi_chan_is_hw_enable(chan))) {
		dev_warn(chan2dev(chan), "BUG: %s caught DWAXIDMAC_IRQ_DMA_TRF, but channel not idle!\n",
			axi_chan_name(chan));
		axi_chan_disable(chan);
	}

	/* The completed descriptor currently is in the head of vc list */
	vd = vchan_next_desc(&chan->vc);
	if (!vd) {
		dev_err(chan2dev(chan), "BUG: %s, IRQ with no descriptors\n",
			axi_chan_name(chan));
		goto out;
	}

	if (chan->cyclic) {
		desc = vd_to_axi_desc(vd);
		if (desc) {
			llp = lo_hi_readq(chan->chan_regs + CH_LLP);
			for (i = 0; i < count; i++) {
				hw_desc = &desc->hw_desc[i];
				if (hw_desc->llp == llp) {
					axi_chan_irq_clear(chan, hw_desc->lli->status_lo);
					hw_desc->lli->ctl_hi |= CH_CTL_H_LLI_VALID;
					desc->completed_blocks = i;

					if (((hw_desc->len * (i + 1)) % desc->period_len) == 0)
						vchan_cyclic_callback(vd);
					break;
				}
			}

			axi_chan_enable(chan);
		}
	} else {
		/* Remove the completed descriptor from issued list before completing */
		list_del(&vd->node);
		vchan_cookie_complete(vd);

		/* Submit queued descriptors after processing the completed ones */
		axi_chan_start_first_queued(chan);
	}

out:
	spin_unlock_irqrestore(&chan->vc.lock, flags);
}

static irqreturn_t dw_axi_dma_interrupt(int irq, void *dev_id)
{
	struct axi_dma_chip *chip = dev_id;
	struct dw_axi_dma *dw = chip->dw;
	struct axi_dma_chan *chan = NULL;

	u32 status, i;

	/* Disable DMAC interrupts. We'll enable them after processing channels */
	axi_dma_irq_disable(chip);

	/* Poll, clear and process every channel interrupt status */
	for (i = 0; i < dw->hdata->nr_channels; i++) {
		chan = &dw->chan[i];
		status = axi_chan_irq_read(chan);
		axi_chan_irq_clear(chan, status);

		if (status & DWAXIDMAC_IRQ_ALL_ERR) {
			axi_chan_handle_err(chan, status);
		} else if (status & DWAXIDMAC_IRQ_DMA_TRF) {
#ifdef CONFIG_DEBUG_FS
			chan->xr_chan_intr_state = status;
			chan->dma_tranfer_done_count++;
#endif

			dev_dbg(chip->dev, "%s %u IRQ status: 0x%08x\n",
				axi_chan_name(chan), i, status);

			axi_chan_block_xfer_complete(chan);
		}
	}

	/* Re-enable interrupts */
	axi_dma_irq_enable(chip);

	return IRQ_HANDLED;
}

static int dma_chan_terminate_all(struct dma_chan *dchan)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);
	u32 chan_active = BIT(chan->id) << DMAC_CHAN_EN_SHIFT;
	unsigned long flags;
	u32 val;
	int ret;
	LIST_HEAD(head);

	axi_chan_disable(chan);

	/* Wait time range 1000-10000 ms */
	ret = readl_poll_timeout_atomic(chan->chip->regs + DMAC_CHENREG2,
					val,
					!(val & chan_active),
					TERMINATE_TIMEOUT_RANGE_MINI,
					TERMINATE_TIMEOUT_RANGE_MAX);
	if (ret == -ETIMEDOUT)
		dev_warn(dchan2dev(dchan),
			 "%s failed to stop\n", axi_chan_name(chan));

	spin_lock_irqsave(&chan->vc.lock, flags);

	vchan_get_all_descriptors(&chan->vc, &head);

	chan->cyclic = false;
	spin_unlock_irqrestore(&chan->vc.lock, flags);

	vchan_dma_desc_free_list(&chan->vc, &head);

	dev_vdbg(dchan2dev(dchan), "terminated: %s\n", axi_chan_name(chan));

	return 0;
}

static int dma_chan_pause(struct dma_chan *dchan)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);
	unsigned long flags;
	unsigned int timeout = XR_DMA_CHAN_PAUSE_TIMEOUT; /* timeout iterations */
	u64 val;

	spin_lock_irqsave(&chan->vc.lock, flags);

	if (XR_CHAN_LESS_EQUAL_THAN_8(chan->chip->dw->hdata->nr_channels)) {
		val = axi_dma_ioread32(chan->chip, DMAC_CHENREG2);
		val |= BIT(chan->id) << DMAC_CHAN_SUSP_SHIFT |
			BIT(chan->id) << DMAC_CHAN_SUSP_WE_SHIFT;
		axi_dma_iowrite32(chan->chip, DMAC_CHENREG2, val);
	} else {
		val = axi_dma_ioread64(chan->chip, DMAC_CHSUSPREG);
		if (XR_CHAN_GREATER_THAN_16(chan->id)) {
			val |= BIT_ULL(chan->id) << XR_CHAN_17_32_SUSP2_SHIFT |
				BIT_ULL(chan->id) << XR_CHAN_17_32_SUSP2_WE_SHIFT;
		} else {
			val |= BIT_ULL(chan->id) << XR_CHAN_1_16_SUSP2_SHIFT |
				BIT_ULL(chan->id) << XR_CHAN_1_16_SUSP2_WE_SHIFT;
		}
		axi_dma_iowrite64(chan->chip, DMAC_CHSUSPREG, val);
	}

	do  {
		if (axi_chan_irq_read(chan) & DWAXIDMAC_IRQ_SUSPENDED)
			break;

		udelay(XR_DMA_CHAN_PAUSE_UDELAY);
	} while (--timeout);

	axi_chan_irq_clear(chan, DWAXIDMAC_IRQ_SUSPENDED);

	chan->is_paused = true;

	spin_unlock_irqrestore(&chan->vc.lock, flags);

	return timeout ? 0 : -EAGAIN;
}

/* Called in chan locked context */
static inline void axi_chan_resume(struct axi_dma_chan *chan)
{
	u64 val;

	if (XR_CHAN_LESS_EQUAL_THAN_8(chan->chip->dw->hdata->nr_channels)) {
		val = axi_dma_ioread32(chan->chip, DMAC_CHENREG2);
		val &= ~(BIT(chan->id) << DMAC_CHAN_SUSP_SHIFT);
		val |=  (BIT(chan->id) << DMAC_CHAN_SUSP_WE_SHIFT);
		axi_dma_iowrite32(chan->chip, DMAC_CHENREG2, val);
	} else {
		val = axi_dma_ioread64(chan->chip, DMAC_CHSUSPREG);
		if (XR_CHAN_GREATER_THAN_16(chan->id)) {
			val &= (~((BIT_ULL(chan->id)) << XR_CHAN_17_32_SUSP2_SHIFT));
			val |= BIT_ULL(chan->id) << XR_CHAN_17_32_SUSP2_WE_SHIFT;
		} else {
			val &= ~(BIT_ULL(chan->id) << DMAC_CHAN_SUSP2_SHIFT);
			val |=  (BIT_ULL(chan->id) << DMAC_CHAN_SUSP2_WE_SHIFT);
		}
		axi_dma_iowrite64(chan->chip, DMAC_CHSUSPREG, val);
	}

	chan->is_paused = false;
}

static int dma_chan_resume(struct dma_chan *dchan)
{
	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);
	unsigned long flags;

	spin_lock_irqsave(&chan->vc.lock, flags);

	if (chan->is_paused)
		axi_chan_resume(chan);

	spin_unlock_irqrestore(&chan->vc.lock, flags);

	return 0;
}

static int __maybe_unused axi_dma_runtime_suspend(struct device *dev)
{
	struct axi_dma_chip *chip = dev_get_drvdata(dev);

	axi_dma_irq_disable(chip);
	axi_dma_disable(chip);

	clk_disable_unprepare(chip->core_clk);
	clk_disable_unprepare(chip->cfgr_clk);

	return 0;
}

static int __maybe_unused axi_dma_runtime_resume(struct device *dev)
{
	struct axi_dma_chip *chip = dev_get_drvdata(dev);
	int ret;

	ret = clk_prepare_enable(chip->cfgr_clk);
	if (ret < 0)
		return ret;

	ret = clk_prepare_enable(chip->core_clk);
	if (ret < 0)
		return ret;

	axi_dma_enable(chip);
	axi_dma_irq_enable(chip);

	return ret;
}

int xr_dma_suspend(struct device *dev)
{
	struct axi_dma_chip *chip = dev_get_drvdata(dev);
	struct dw_axi_dma *dw = NULL;
	struct axi_dma_chan *chan = NULL;
	int i;

	if (!chip) {
		dev_err(dev, "%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dw = chip->dw;

	dev_info(dev, "%s ++\n", __func__);

	axi_dma_irq_disable(chip);
	for (i = 0; i < dw->hdata->nr_channels; i++) {
		chan = &dw->chan[i];
		if (unlikely(axi_chan_is_hw_enable(chan))) {
			dev_warn(chan2dev(chan),
				"%s, channel not idle!\n", axi_chan_name(chan));
			axi_chan_disable(&chip->dw->chan[i]);
		}
		axi_chan_irq_disable(&chip->dw->chan[i], DWAXIDMAC_IRQ_ALL);
	}
	axi_dma_disable(chip);

	clk_disable_unprepare(chip->core_clk);
	clk_disable_unprepare(chip->cfgr_clk);

	dev_info(dev, "%s --\n", __func__);

	return 0;
}

int xr_dma_resume(struct device *dev)
{
	struct axi_dma_chip *chip = dev_get_drvdata(dev);
	int ret;
	int i;

	if (!chip) {
		dev_err(dev, "%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dev_info(dev, "%s ++\n", __func__);

	ret = reset_control_deassert(chip->rst);
	if (ret < 0) {
		dev_err(dev, "Couldn't deassert rst control: %d\n", ret);
		return ret;
	}

	ret = reset_control_deassert(chip->tpc_rst);
	if (ret < 0) {
		dev_err(dev, "Couldn't deassert tpc_rst control: %d\n", ret);
		return ret;
	}

	ret = clk_prepare_enable(chip->cfgr_clk);
	if (ret < 0)
		return ret;

	ret = clk_prepare_enable(chip->core_clk);
	if (ret < 0)
		return ret;

	axi_dma_enable(chip);
	axi_dma_irq_enable(chip);
	for (i = 0; i < chip->dw->hdata->nr_channels; i++) {
		axi_chan_irq_disable(&chip->dw->chan[i], DWAXIDMAC_IRQ_ALL);
		axi_chan_disable(&chip->dw->chan[i]);
	}

	dev_info(dev, "%s --\n", __func__);

	return 0;
}

static struct dma_chan *dw_axi_dma_of_xlate(struct of_phandle_args *dma_spec,
					    struct of_dma *ofdma)
{
	struct dw_axi_dma *dw = ofdma->of_dma_data;
	struct axi_dma_chan *chan = NULL;
	struct dma_chan *dchan = NULL;

	dchan = dma_get_any_slave_channel(&dw->dma);
	if (!dchan)
		return NULL;

	chan = dchan_to_axi_dma_chan(dchan);
	chan->hw_handshake_num = dma_spec->args[0];

	dev_info(dchan2dev(dchan), "%s, hw_handshake_num:%u, chan id:%u\n",
		 __func__, chan->hw_handshake_num, chan->id);

	return dchan;
}

static int parse_device_properties(struct axi_dma_chip *chip)
{
	struct device *dev = chip->dev;
	u32 tmp;
	u32 carr[DMAC_MAX_CHANNELS];
	int ret;

	ret = device_property_read_u32(dev, "dma-channels", &tmp);
	if (ret)
		return ret;
	if (tmp == 0 || tmp > DMAC_MAX_CHANNELS)
		return -EINVAL;

	chip->dw->hdata->nr_channels = tmp;

	ret = device_property_read_u32(dev, "snps,dma-masters", &tmp);
	if (ret)
		return ret;
	if (tmp == 0 || tmp > DMAC_MAX_MASTERS)
		return -EINVAL;

	chip->dw->hdata->nr_masters = tmp;

	ret = device_property_read_u32(dev, "snps,data-width", &tmp);
	if (ret)
		return ret;
	if (tmp > DWAXIDMAC_TRANS_WIDTH_MAX)
		return -EINVAL;

	chip->dw->hdata->m_data_width = tmp;

	ret = device_property_read_u32_array(dev, "snps,block-size", carr,
					     chip->dw->hdata->nr_channels);
	if (ret)
		return ret;

	for (tmp = 0; tmp < chip->dw->hdata->nr_channels; tmp++) {
		if (carr[tmp] == 0 || carr[tmp] > DMAC_MAX_BLK_SIZE)
			return -EINVAL;

		chip->dw->hdata->block_size[tmp] = carr[tmp];
	}

	ret = device_property_read_u32_array(dev, "snps,priority", carr,
					     chip->dw->hdata->nr_channels);
	if (ret)
		return ret;

	/* Priority value must be programmed within [0:nr_channels-1] range */
	for (tmp = 0; tmp < chip->dw->hdata->nr_channels; tmp++) {
		if (carr[tmp] >= chip->dw->hdata->nr_channels)
			return -EINVAL;

		chip->dw->hdata->priority[tmp] = carr[tmp];
	}

	return 0;
}

static int dw_probe(struct platform_device *pdev)
{
	struct axi_dma_chip *chip = NULL;
	struct resource *mem = NULL;
	struct dw_axi_dma *dw = NULL;
	struct dw_axi_dma_hcfg *hdata = NULL;
	struct axi_dma_chan *chan = NULL;
	const char *clk_name = NULL;
	u32 i;
	int ret;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	dw = devm_kzalloc(&pdev->dev, sizeof(*dw), GFP_KERNEL);
	if (!dw)
		return -ENOMEM;

	hdata = devm_kzalloc(&pdev->dev, sizeof(*hdata), GFP_KERNEL);
	if (!hdata)
		return -ENOMEM;

	chip->dw = dw;
	chip->dev = &pdev->dev;
	chip->dw->hdata = hdata;
	raw_spin_lock_init(&chip->dw->lock);

	chip->irq = platform_get_irq(pdev, 0);
	if (chip->irq < 0)
		return chip->irq;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	chip->regs = devm_ioremap_resource(chip->dev, mem);
	if (IS_ERR(chip->regs))
		return PTR_ERR(chip->regs);

	chip->rst = devm_reset_control_get_exclusive(&pdev->dev, "rst");
	if (IS_ERR(chip->rst)) {
		ret = PTR_ERR(chip->rst);
		dev_err(chip->dev, "Couldn't get reset control: %d\n", ret);
		return ret;
	}

	chip->tpc_rst = devm_reset_control_get_exclusive(&pdev->dev, "tpc_rst");
	if (IS_ERR(chip->tpc_rst)) {
		ret = PTR_ERR(chip->tpc_rst);
		dev_err(chip->dev, "Couldn't get tpc reset control: %d\n", ret);
		return ret;
	}

	ret = reset_control_deassert(chip->rst);
	if (ret < 0) {
		dev_err(chip->dev, "Couldn't deassert rst control: %d\n", ret);
		return ret;
	}

	ret = reset_control_deassert(chip->tpc_rst);
	if (ret < 0) {
		dev_err(chip->dev, "Couldn't deassert tpc_rst control: %d\n", ret);
		return ret;
	}

	for (i = 0; i < XR_DMA_CLK_NUMS; i++) {
		ret = of_property_read_string_index(
					(&pdev->dev)->of_node,
					"clock-names",
					i,
					&clk_name);
		if (ret < 0) {
			dev_err(&pdev->dev, "clock-names get failed.\n");
			return ret;
		}

		if (memcmp(clk_name, XR_PCLK_PREFIX_NAME, XR_PCLK_PREFIX_LEN) == 0) {
			chip->cfgr_clk = devm_clk_get(&pdev->dev, clk_name);
			if (IS_ERR(chip->cfgr_clk)) {
				dev_err(&pdev->dev, "get %s error.", clk_name);
				return PTR_ERR(chip->cfgr_clk);
			}
		} else {
			chip->core_clk = devm_clk_get(&pdev->dev, clk_name);
			if (IS_ERR(chip->core_clk)) {
				dev_err(&pdev->dev, "get %s error.", clk_name);
				return PTR_ERR(chip->core_clk);
			}
		}
	}

	ret = parse_device_properties(chip);
	if (ret)
		return ret;

	dw->chan = devm_kcalloc(chip->dev, hdata->nr_channels,
				sizeof(*dw->chan), GFP_KERNEL);
	if (!dw->chan)
		return -ENOMEM;

	ret = devm_request_irq(chip->dev, chip->irq, dw_axi_dma_interrupt,
			       IRQF_SHARED, KBUILD_MODNAME, chip);
	if (ret)
		return ret;

	INIT_LIST_HEAD(&dw->dma.channels);
	for (i = 0; i < hdata->nr_channels; i++) {
		chan = &dw->chan[i];

		chan->chip = chip;
		chan->id = i;
		chan->chan_regs = chip->regs + i * XRING_CHAN_REG_LEN;

		atomic_set(&chan->descs_allocated, 0);

		chan->vc.desc_free = vchan_desc_put;
		vchan_init(&chan->vc, &dw->dma);
	}

	/* Set capabilities */
	dma_cap_set(DMA_MEMCPY, dw->dma.cap_mask);
	dma_cap_set(DMA_SLAVE, dw->dma.cap_mask);
	dma_cap_set(DMA_CYCLIC, dw->dma.cap_mask);

	/* DMA capabilities */
	dw->dma.chancnt = hdata->nr_channels;
	dw->dma.max_burst = hdata->axi_rw_burst_len;
	dw->dma.src_addr_widths = AXI_DMA_BUSWIDTHS;
	dw->dma.dst_addr_widths = AXI_DMA_BUSWIDTHS;
	dw->dma.directions = BIT(DMA_MEM_TO_MEM);
	dw->dma.directions |= BIT(DMA_MEM_TO_DEV) | BIT(DMA_DEV_TO_MEM);
	dw->dma.residue_granularity = DMA_RESIDUE_GRANULARITY_BURST;

	dw->dma.dev = chip->dev;
	dw->dma.device_tx_status = dma_chan_tx_status;
	dw->dma.device_issue_pending = dma_chan_issue_pending;
	dw->dma.device_terminate_all = dma_chan_terminate_all;
	dw->dma.device_pause = dma_chan_pause;
	dw->dma.device_resume = dma_chan_resume;

	dw->dma.device_alloc_chan_resources = dma_chan_alloc_chan_resources;
	dw->dma.device_free_chan_resources = dma_chan_free_chan_resources;

	dw->dma.device_prep_dma_memcpy = dma_chan_prep_dma_memcpy;
	dw->dma.device_synchronize = dw_axi_dma_synchronize;
	dw->dma.device_config = dw_axi_dma_chan_slave_config;
	dw->dma.device_prep_slave_sg = dw_axi_dma_chan_prep_slave_sg;
	dw->dma.device_prep_dma_cyclic = dw_axi_dma_chan_prep_cyclic;

	platform_set_drvdata(pdev, chip);

	ret = clk_prepare_enable(chip->cfgr_clk);
	if (ret < 0)
		return ret;

	ret = clk_prepare_enable(chip->core_clk);
	if (ret < 0)
		return ret;

	axi_dma_enable(chip);
	axi_dma_irq_enable(chip);
	axi_dma_hw_init(chip);

	ret = dma_async_device_register(&dw->dma);
	if (ret)
		return ret;

	/* Register with OF helpers for DMA lookups */
	ret = of_dma_controller_register(pdev->dev.of_node,
					 dw_axi_dma_of_xlate, dw);
	if (ret < 0) {
		dev_warn(&pdev->dev,
			 "Failed to register OF DMA controller, fallback to MEM_TO_MEM mode\n");
		dma_async_device_unregister(&dw->dma);
		return ret;
	}

	xr_dma_debugfs_init(chip);

	dev_info(chip->dev, "DesignWare AXI DMA Controller, %d channels\n",
		 dw->hdata->nr_channels);

	return 0;
}

static int dw_remove(struct platform_device *pdev)
{
	struct axi_dma_chip *chip = platform_get_drvdata(pdev);
	struct dw_axi_dma *dw = chip->dw;
	struct axi_dma_chan *chan = NULL;
	struct axi_dma_chan *_chan = NULL;
	u32 i;

	axi_dma_irq_disable(chip);
	for (i = 0; i < dw->hdata->nr_channels; i++) {
		axi_chan_disable(&chip->dw->chan[i]);
		axi_chan_irq_disable(&chip->dw->chan[i], DWAXIDMAC_IRQ_ALL);
	}
	axi_dma_disable(chip);

	devm_free_irq(chip->dev, chip->irq, chip);

	dma_async_device_unregister(&dw->dma);
	of_dma_controller_free(chip->dev->of_node);

	list_for_each_entry_safe(chan, _chan, &dw->dma.channels,
			vc.chan.device_node) {
		list_del(&chan->vc.chan.device_node);
		tasklet_kill(&chan->vc.task);
	}

	xr_dma_cleanup_debugfs(chip);

	clk_disable_unprepare(chip->core_clk);
	clk_disable_unprepare(chip->cfgr_clk);

	dev_info(chip->dev, "X-Ring AXI DMA Controller driver remove.\n");

	return 0;
}

#ifdef CONFIG_DEBUG_FS
u64 xr_axi_dma_debugfs_read_reg(struct axi_dma_chip *chip, u64 reg)
{
	return axi_dma_ioread64(chip, reg);
}

u64 xr_dma_chan_debugfs_read_reg(struct axi_dma_chan *chan, u32 reg)
{
	u64 val_hi;
	u64 val_lo;

	val_hi = ioread32(XR_TO_HIGH_ADDRESS(chan->chan_regs + reg));
	val_lo = ioread32(chan->chan_regs + reg);
	val_hi = TO_READ_REG_VALUE(val_hi, val_lo);

	return val_hi;
}

int active_chan_show(struct seq_file *s, void *p)
{
	struct axi_dma_chip *chip = NULL;
	struct dw_axi_dma *dw = NULL;
	struct axi_dma_chan *chan = NULL;
	struct dw_axi_dma_hcfg *hdata = NULL;
	u64 val;
	int i;
	int result;

	chip = s->private;
	if (chip != NULL)
		dw = chip->dw;
	else
		return -EINVAL;

	hdata = dw->hdata;

	seq_puts(s, "==================================\n");
	seq_printf(s, "X-Ring AXI DMA channel nums: %u\n", hdata->nr_channels);
	val = xr_axi_dma_debugfs_read_reg(chip, DMAC_CHENREG2);
	seq_printf(s, "DMAC_CHENREG2: 0x%llx\n", val);

	for (i = 0; i < hdata->nr_channels; i++) {
		result = 0;
		chan = &dw->chan[i];
		if (XR_CHAN_GREATER_THAN_16(chan->id))
			result = !!(val & (BIT_ULL(chan->id) << XR_CHAN_17_32_EN2_SHIFT));
		else
			result = !!(val & (BIT_ULL(chan->id) << XR_CHAN_1_16_EN2_SHIFT));

		if (result)
			seq_printf(s, "\tChannel %u is in use.\n", chan->id);
	}

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(active_chan);

int dma_info_show(struct seq_file *s, void *p)
{
	struct axi_dma_chip *chip = NULL;
	struct dw_axi_dma *dw = NULL;
	struct dw_axi_dma_hcfg *hdata = NULL;
	int i;
	u64 val;

	chip = s->private;
	if (chip != NULL)
		dw = chip->dw;
	else
		return -EINVAL;

	hdata = dw->hdata;

	seq_puts(s, "===============X-Ring AXI DMA===================\n");
	seq_printf(s, "channel nums:\t %u\n", hdata->nr_channels);

	seq_puts(s, "max block size(Byte):\n\t");
	for (i = 0; i < hdata->nr_channels; i++) {
		if (DEBUG_FS_PRINT_FORMAT(i))
			seq_puts(s, "\n\t");

		seq_printf(s, "0x%x, ", hdata->block_size[i]);
	}

	seq_printf(s, "\nmax data width(byte):\t %d\n",
			DEBUG_FS_DMA_WIDTH_TO_BYTE(hdata->m_data_width));

	seq_puts(s, "priority(0 Max):\n\t");
	for (i = 0; i < hdata->nr_channels; i++) {
		if (DEBUG_FS_PRINT_FORMAT(i))
			seq_puts(s, "\n\t");

		seq_printf(s, "%u, ", hdata->priority[i]);
	}

	seq_printf(s, "\nMSIZE(m2p/p2m burst size):\t %u\n",
			DEBUG_FS_DMA_MISE(DWAXIDMAC_BURST_TRANS_LEN_16));

	val = xr_axi_dma_debugfs_read_reg(chip, DMAC_CFGREG);
	seq_printf(s, "DMAC_CFG_REG:\t 0x%llx\n", val);
	seq_printf(s, "\tDMAC_EN, enable(1)/disable(0):\t %u\n",
			!!(val & BIT(DMAC_CFGREG_DMAC_EN_BitAddressOffset)));

	seq_printf(s, "\tINT_EN, enable(1)/disable(0):\t %u\n",
			!!(val & BIT(DMAC_CFGREG_INT_EN_BitAddressOffset)));

	val = xr_axi_dma_debugfs_read_reg(chip, DMAC_CHENREG2);
	seq_printf(s, "DMAC_CHENREG2:\t 0x%llx\n", val);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(dma_info);

int chan_info_show(struct seq_file *s, void *p)
{
	struct axi_dma_chan *chan = NULL;

	chan = s->private;
	if (chan == NULL)
		return -EINVAL;

	seq_printf(s, "transfer type and flow ctrl(TT_FC): %u\n", chan->config.device_fc);
	seq_printf(s, "\t %u, XRAXIDMAC_TT_FC_MEM_TO_MEM_DMAC\n", DWAXIDMAC_TT_FC_MEM_TO_MEM_DMAC);
	seq_printf(s, "\t %u, XRAXIDMAC_TT_FC_MEM_TO_PER_DMAC\n", DWAXIDMAC_TT_FC_MEM_TO_PER_DMAC);
	seq_printf(s, "\t %u, XRAXIDMAC_TT_FC_PER_TO_MEM_DMAC\n", DWAXIDMAC_TT_FC_PER_TO_MEM_DMAC);

	seq_printf(s, "hw_handshake_num: %u\n", chan->hw_handshake_num);

	seq_printf(s, "direction: %u\n", chan->direction);
	seq_printf(s, "\t %u, DMA_MEM_TO_MEM\n", DMA_MEM_TO_MEM);
	seq_printf(s, "\t %u, DMA_MEM_TO_DEV\n", DMA_MEM_TO_DEV);
	seq_printf(s, "\t %u, DMA_DEV_TO_MEM\n", DMA_DEV_TO_MEM);
	seq_printf(s, "\t %u, DMA_DEV_TO_DEV\n", DMA_DEV_TO_DEV);

	seq_printf(s, "src width: %u\n", chan->config.src_addr_width);
	seq_printf(s, "dst width: %u\n", chan->config.dst_addr_width);
	seq_printf(s, "\t %u, DMA_SLAVE_BUSWIDTH_1_BYTE\n", DMA_SLAVE_BUSWIDTH_1_BYTE);
	seq_printf(s, "\t %u, DMA_SLAVE_BUSWIDTH_2_BYTES\n", DMA_SLAVE_BUSWIDTH_2_BYTES);
	seq_printf(s, "\t %u, DMA_SLAVE_BUSWIDTH_3_BYTES\n", DMA_SLAVE_BUSWIDTH_3_BYTES);
	seq_printf(s, "\t %u, DMA_SLAVE_BUSWIDTH_4_BYTES\n", DMA_SLAVE_BUSWIDTH_4_BYTES);
	seq_printf(s, "\t %u, DMA_SLAVE_BUSWIDTH_8_BYTES\n", DMA_SLAVE_BUSWIDTH_8_BYTES);
	seq_printf(s, "\t %u, DMA_SLAVE_BUSWIDTH_16_BYTES\n", DMA_SLAVE_BUSWIDTH_16_BYTES);
	seq_printf(s, "\t %u, DMA_SLAVE_BUSWIDTH_32_BYTES\n", DMA_SLAVE_BUSWIDTH_32_BYTES);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(chan_info);

int chan_reg_show(struct seq_file *s, void *p)
{
	struct axi_dma_chan *chan = NULL;

	chan = s->private;
	if (chan == NULL)
		return -EINVAL;

	seq_printf(s, "CH_CFG:\t 0x%016llx\n", xr_dma_chan_debugfs_read_reg(chan, CH_CFG));
	seq_printf(s, "CH_CTL:\t 0x%016llx\n", xr_dma_chan_debugfs_read_reg(chan, CH_CTL));
	seq_printf(s, "CH_SAR:\t 0x%016llx\n", xr_dma_chan_debugfs_read_reg(chan, CH_SAR));
	seq_printf(s, "CH_DAR:\t 0x%016llx\n", xr_dma_chan_debugfs_read_reg(chan, CH_DAR));
	seq_printf(s, "CH_BLOCK_TS: 0x%llx\n", xr_dma_chan_debugfs_read_reg(chan, CH_BLOCK_TS));

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(chan_reg);

int chan_intr_show(struct seq_file *s, void *p)
{
	struct axi_dma_chan *chan = NULL;

	chan = s->private;
	if (chan == NULL)
		return -EINVAL;

	seq_printf(s, "Channel %u last interrupt status:\t 0x%08x\n",
			chan->id, chan->xr_chan_intr_state);
	seq_printf(s, "Channel %u current interrupt status:\t 0x%08x\n",
			chan->id, axi_chan_irq_read(chan));
	seq_printf(s, "Block transfer done count:\t %u\n", chan->block_tranfer_done_count);
	seq_printf(s, "DMA transfer done count:\t %u\n", chan->dma_tranfer_done_count);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(chan_intr);

int chan_next_desc_show(struct seq_file *s, void *p)
{
	struct axi_dma_chan *chan = NULL;
	struct axi_dma_desc *desc;
	struct axi_dma_hw_desc *hw_desc;
	struct virt_dma_desc *vd;
	int i;
	int desc_count;

	chan = s->private;
	if (chan == NULL)
		return -EINVAL;

	vd = vchan_next_desc(&chan->vc);
	if (!vd) {
		seq_puts(s, "Not have next description.\n");
		return -EINVAL;
	}
	desc = vd_to_axi_desc(vd);

	desc_count = atomic_read(&chan->descs_allocated);
	seq_printf(s, "dma transfer description nums: %u\nn", desc_count);
	for (i = 0; i < desc_count; i++) {
		hw_desc = &desc->hw_desc[i];
		seq_printf(s, "==============desc-%u==============\n", i);
		seq_printf(s, "\tSAR: 0x%llx\n", le64_to_cpu(desc->hw_desc->lli->sar));
		seq_printf(s, "\tDAR: 0x%llx\n", le64_to_cpu(desc->hw_desc->lli->dar));
		seq_printf(s, "\tLLP: 0x%llx\n", le64_to_cpu(desc->hw_desc->lli->llp));
		seq_printf(s, "\tBTS: 0x%llx\n", le64_to_cpu(desc->hw_desc->lli->block_ts_lo));
		seq_printf(s, "\tCTL: 0x%x-%x\n",
						le32_to_cpu(desc->hw_desc->lli->ctl_hi),
						le32_to_cpu(desc->hw_desc->lli->ctl_lo));
	}

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(chan_next_desc);

int xr_dma_debugfs_init(struct axi_dma_chip *chip)
{
	struct dentry *xr_chan = NULL;
	struct axi_dma_chan *chan = NULL;
	char chan_name[DEBUGFS_CHAN_NAME_LEN];
	int i;

	/* create dir */
	chip->dw->xr_dma_dentry = debugfs_create_dir("xring-dmac", NULL);
	if (!chip->dw->xr_dma_dentry) {
		pr_err("xring-dmac is NULL");
		return -EINVAL;
	}

	debugfs_create_file("info", DEBUGFS_FILE_MASK,
			chip->dw->xr_dma_dentry, chip, &dma_info_fops);

	debugfs_create_file("active", DEBUGFS_FILE_MASK,
			chip->dw->xr_dma_dentry, chip, &active_chan_fops);

	for (i = 0; i < chip->dw->hdata->nr_channels; i++) {
		chan = &chip->dw->chan[i];

		chan->dma_tranfer_done_count = 0;
		chan->block_tranfer_done_count = 0;
		chan->xr_chan_intr_state = 0;

		scnprintf(chan_name, sizeof(chan_name), "dma-chan-%d", i);
		xr_chan = debugfs_create_dir(chan_name, chip->dw->xr_dma_dentry);
		if (!xr_chan) {
			dev_info(chip->dev, "xring-dmac is NULL");
			return -EINVAL;
		}

		debugfs_create_file("status", DEBUGFS_FILE_MASK,
				xr_chan, chan, &chan_info_fops);
		debugfs_create_file("reg", DEBUGFS_FILE_MASK,
				xr_chan, chan, &chan_reg_fops);
		debugfs_create_file("intr", DEBUGFS_FILE_MASK,
				xr_chan, chan, &chan_intr_fops);
		debugfs_create_file("next-desc", DEBUGFS_FILE_MASK,
				xr_chan, chan, &chan_next_desc_fops);
	}

	return 0;
}

void xr_dma_cleanup_debugfs(struct axi_dma_chip *chip)
{
	debugfs_remove_recursive(chip->dw->xr_dma_dentry);
}
#else
int xr_dma_debugfs_init(struct axi_dma_chip *chip) { return 0; }
void xr_dma_cleanup_debugfs(struct axi_dma_chip *chip) {}
#endif

static const struct dev_pm_ops dw_axi_dma_pm_ops = {
	NOIRQ_SYSTEM_SLEEP_PM_OPS(xr_dma_suspend, xr_dma_resume)
	SET_RUNTIME_PM_OPS(axi_dma_runtime_suspend, axi_dma_runtime_resume, NULL)
};

static const struct of_device_id dw_dma_of_id_table[] = {
	{ .compatible = "xring,axi-dma" },
	{}
};
MODULE_DEVICE_TABLE(of, dw_dma_of_id_table);

static struct platform_driver dw_driver = {
	.probe		= dw_probe,
	.remove		= dw_remove,
	.driver = {
		.name	= KBUILD_MODNAME,
		.of_match_table = dw_dma_of_id_table,
		.pm = &dw_axi_dma_pm_ops,
	},
};
module_platform_driver(dw_driver);

MODULE_SOFTDEP("pre: xr_ip_reset");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Synopsys DesignWare AXI DMA Controller platform driver");
MODULE_AUTHOR("Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>");
MODULE_AUTHOR("Hu Wei <huwei19@xiaomi.com>");
