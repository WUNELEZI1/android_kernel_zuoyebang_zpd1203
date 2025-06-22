// SPDX-License-Identifier: GPL-2.0-only
/*
 * XRing SPI adapter driver.
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/completion.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/irqreturn.h>
#include <linux/jiffies.h>
#include <linux/types.h>

#include "spi-xr.h"

static void xr_spi_dma_maxburst_init(struct dw_spi *dws)
{
	struct dma_slave_caps caps;
	u32 max_burst, def_burst;
	int ret;

	def_burst = dws->fifo_len / 2;

	ret = dma_get_slave_caps(dws->rxchan, &caps);
	if (!ret && caps.max_burst)
		max_burst = caps.max_burst;
	else
		max_burst = RX_BURST_LEVEL;

	dws->rxburst = min(max_burst, def_burst);
	dw_writel(dws, DW_SPI_DMARDLR, dws->rxburst - 1);
}

static void xr_spi_dma_sg_burst_init(struct dw_spi *dws)
{
	struct dma_slave_caps rx = {0};

	dma_get_slave_caps(dws->rxchan, &rx);

	if (rx.max_sg_burst > 0)
		dws->dma_sg_burst = rx.max_sg_burst;
	else
		dws->dma_sg_burst = 0;
}

static int xr_spi_dma_init(struct device *dev, struct dw_spi *dws)
{
	if (dws->dma_type == EXTERNAL_DMA) {
		dws->rxchan = dma_request_slave_channel(dev, "rx");
		if (!dws->rxchan)
			return -ENODEV;
		dws->master->dma_rx = dws->rxchan;
	}
	init_completion(&dws->dma_completion);

	xr_spi_dma_maxburst_init(dws);

	xr_spi_dma_sg_burst_init(dws);

	return 0;
}

static void xr_spi_dma_exit(struct dw_spi *dws)
{
	if (dws->dma_type != EXTERNAL_DMA)
		return;
	if (dws->rxchan) {
		dmaengine_terminate_sync(dws->rxchan);
		dma_release_channel(dws->rxchan);
	}
}

static int xr_spi_dma_check_status(struct dw_spi *dws)
{
	u32 irq_status;
	int ret = 0;

	irq_status = dw_readl(dws, SPI_DMA_REGIF_INT_SPI_DMA_ERR_IRQ_INT_STATUS);
	if (irq_status & SPI_DMA_GIF_DEC_ERR) {
		dev_err(dws->dev, "GIF_DEC_ERR detected\n");
		ret = -EIO;
	}

	if (irq_status & SPI_DMA_GIF_ADDR_ERR) {
		dev_err(dws->dev, "GIF_ADDR_ERR detected\n");
		ret = -EIO;
	}

	if (irq_status & SPI_DMA_APB_ERR) {
		dev_err(dws->dev, "APB_ERR detected\n");
		ret = -EIO;
	}

	if (irq_status & SPI_DMA_FIFO_UD_ERR) {
		dev_err(dws->dev, "FIFO_UD_ERR detected\n");
		ret = -EIO;
	}

	if (irq_status & SPI_DMA_FIFO_OV_ERR) {
		dev_err(dws->dev, "FIFO_OV_ERR detected\n");
		ret = -EIO;
	}

	return ret;
}

static irqreturn_t xr_spi_dma_transfer_handler(struct dw_spi *dws)
{
	u16 irq_status = dw_readl(dws,
		SPI_DMA_WRAP_INT_ERR_SIRQ_INT_STATUS) & 0x3;

	if (irq_status & SSI_INRT_MASK_EN_B)
		dw_spi_check_status(dws, false);
	if (irq_status & SPI_DMA_ERR_MASK_EN_B)
		xr_spi_dma_check_status(dws);

	complete(&dws->dma_completion);
	return IRQ_HANDLED;
}

static int xr_spi_dma_wait(struct dw_spi *dws, unsigned int len, u32 speed)
{
	unsigned long long ms;

	ms = len * MSEC_PER_SEC * BITS_PER_BYTE;
	do_div(ms, speed);
	ms += ms + 200;

	if (ms > UINT_MAX)
		ms = UINT_MAX;

	ms = wait_for_completion_timeout(&dws->dma_completion,
					 msecs_to_jiffies(ms));
	if (ms == 0) {
		dev_err(&dws->master->cur_msg->spi->dev,
			"DMA transaction timed out\n");
		return -ETIMEDOUT;
	}

	if (dws->dma_type == INTERNAL_DMA) {
		if (dws->rx_dma)
			dma_unmap_single(dws->dev, dws->rx_dma, dws->rx_len, DMA_FROM_DEVICE);
	}

	return 0;
}

static int xr_spi_dma_config_rx(struct dw_spi *dws)
{
	struct dma_slave_config rxconf;

	memset(&rxconf, 0, sizeof(rxconf));
	rxconf.direction = DMA_DEV_TO_MEM;
	rxconf.src_addr = dws->dma_addr;
	rxconf.src_maxburst = dws->rxburst;
	rxconf.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	rxconf.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	rxconf.device_fc = false;

	return dmaengine_slave_config(dws->rxchan, &rxconf);
}

static int xr_spi_dma_setup(struct dw_spi *dws, struct spi_transfer *xfer)
{
	u16 imr = 0;
	u16 dma_ctrl;
	int ret;

	if (dws->dma_type == EXTERNAL_DMA) {
		ret = xr_spi_dma_config_rx(dws);
		if (ret)
			return ret;
	}

	dma_ctrl = SPI_DMA_RDMAE;
	dw_writel(dws, DW_SPI_DMACR, dma_ctrl);
	/* Set the interrupt mask */
	imr = SPI_INT_TXOI | SPI_INT_RXUI | SPI_INT_RXOI;
	spi_umask_intr(dws, imr);

	imr = ~SSI_INRT_MASK_EN_B;
	dw_writel(dws, SPI_DMA_WRAP_INT_ERR_SIRQ_INT_MASK, imr);

	if (dws->dma_type == INTERNAL_DMA) {
		imr = ~(SPI_DMA_GIF_DEC_ERR | SPI_DMA_GIF_ADDR_ERR
			| SPI_DMA_APB_ERR | SPI_DMA_FIFO_UD_ERR | SPI_DMA_FIFO_OV_ERR);
		dw_writel(dws, SPI_DMA_REGIF_INT_SPI_DMA_ERR_IRQ_INT_MASK, imr);
		imr = dw_readl(dws, SPI_DMA_WRAP_INT_ERR_SIRQ_INT_MASK);
		imr &= ~SPI_DMA_ERR_MASK_EN_B;
		dw_writel(dws, SPI_DMA_WRAP_INT_ERR_SIRQ_INT_MASK, imr);
	}

	reinit_completion(&dws->dma_completion);

	dws->transfer_handler = xr_spi_dma_transfer_handler;

	return 0;
}

static void xr_spi_dma_prep(struct dw_spi *dws)
{
	xr_spi_dma_irq_enable(dws);
	xr_spi_dma_iowrite64(dws, SPI_DMA_REGIF_SPI_DMA_AXI_ADDR_L, dws->rx_dma);
	dw_writel(dws, SPI_DMA_REGIF_SPI_DMA_FRAME, dws->rx_len / 4);
	xr_spi_dma_enable(dws);
}

static int xr_spi_dma_transfer_all(struct dw_spi *dws,
				   struct spi_transfer *xfer)
{
	int ret;
	u32 txw = 0;

	if (dws->dma_type == INTERNAL_DMA) {
		if (!IS_ALIGNED((size_t)dws->rx, DMA_ADDR_ALIGN_SIZE)) {
			dev_err(dws->dev, "dma transfer address not align\n");
			return -EINVAL;
		}
		if (!IS_ALIGNED(dws->rx_len, DMA_LENGTH_ALIGN_SIZE)) {
			dev_err(dws->dev, "dma transfer length not align\n");
			return -EINVAL;
		}
		dws->rx_dma = dma_map_single(dws->dev, dws->rx, dws->rx_len, DMA_FROM_DEVICE);
		if (dma_mapping_error(dws->dev, dws->rx_dma)) {
			dev_err(dws->dev, "unable to map rx dma\n");
			return -ENOMEM;
		}
		xr_spi_dma_prep(dws);
	} else {
		ret = dw_spi_dma_submit_rx(dws, xfer->rx_sg.sgl,
					   xfer->rx_sg.nents);
		if (ret)
			goto err_clear_dmac;

		/* rx must be started before tx due to spi instinct */
		dma_async_issue_pending(dws->rxchan);
	}

	dw_write_io_reg(dws, DW_SPI_DR, txw);
	ret = xr_spi_dma_wait(dws, xfer->len, xfer->effective_speed_hz);

err_clear_dmac:
	dw_writel(dws, DW_SPI_DMACR, 0);

	return ret;
}

static int xr_spi_dma_transfer(struct dw_spi *dws, struct spi_transfer *xfer)
{
	int ret;

	ret = xr_spi_dma_transfer_all(dws, xfer);
	if (ret) {
		if (dws->dma_type == INTERNAL_DMA) {
			xr_spi_dma_irq_clear(dws);
			xr_spi_dma_irq_disable(dws);
			if (dws->rx_dma)
				dma_unmap_single(dws->dev, dws->rx_dma,
						dws->rx_len, DMA_FROM_DEVICE);
		}
		return ret;
	}

	if (dws->dma_type == EXTERNAL_DMA) {
		if (xfer->rx_buf && dws->master->cur_msg->status == -EINPROGRESS)
			ret = dw_spi_dma_wait_rx_done(dws);
	}

	return ret;
}

static void xr_spi_dma_stop(struct dw_spi *dws)
{
	if (!dws || dws->dma_type != EXTERNAL_DMA)
		return;
	if (test_bit(RX_BUSY, &dws->dma_chan_busy)) {
		dmaengine_terminate_sync(dws->rxchan);
		clear_bit(RX_BUSY, &dws->dma_chan_busy);
	}
}

irqreturn_t xr_spi_dma_irq_handler(int irq, void *dev_id)
{
	u32 status;
	struct dw_spi *dws = dev_id;
	unsigned long flags;

	spin_lock_irqsave(&dws->lock, flags);
	if (dws->transfer_state != SPI_TRANSFER_PREPARED) {
		spin_unlock_irqrestore(&dws->lock, flags);
		return IRQ_HANDLED;
	}
	status = xr_spi_dma_irq_read(dws);

	if (status & SPI_DMA_INT_FLAG)
		xr_spi_dma_irq_clear(dws);

	complete(&dws->dma_completion);
	spin_unlock_irqrestore(&dws->lock, flags);

	return IRQ_HANDLED;
}

static const struct dw_spi_dma_ops xr_spi_dma_ops = {
	.dma_init	= xr_spi_dma_init,
	.dma_exit	= xr_spi_dma_exit,
	.dma_setup	= xr_spi_dma_setup,
	.can_dma	= dw_spi_can_dma,
	.dma_transfer	= xr_spi_dma_transfer,
	.dma_stop	= xr_spi_dma_stop,
};

void xr_spi_dma_setup_generic(struct dw_spi *dws)
{
	dws->dma_ops = &xr_spi_dma_ops;
}
