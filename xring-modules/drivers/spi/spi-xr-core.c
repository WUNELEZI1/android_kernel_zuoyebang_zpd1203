// SPDX-License-Identifier: GPL-2.0-only
/*
 * XRing SPI adapter driver.
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/preempt.h>
#include <linux/highmem.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/string.h>
#include <linux/of.h>

#include "spi-xr.h"

static irqreturn_t xr_spi_irq_handler(int irq, void *dev_id)
{
	struct spi_controller *master = dev_id;
	struct dw_spi *dws = spi_controller_get_devdata(master);
	unsigned long flags;
	irqreturn_t status;
	u16 irq_status = 0;

	spin_lock_irqsave(&dws->lock, flags);
	if (dws->transfer_state != SPI_TRANSFER_PREPARED) {
		spin_unlock_irqrestore(&dws->lock, flags);
		return IRQ_HANDLED;
	}

	irq_status = dw_readl(dws,
			SPI_DMA_WRAP_INT_ERR_SIRQ_INT_STATUS) & 0x3;
	if (!irq_status) {
		spin_unlock_irqrestore(&dws->lock, flags);
		return IRQ_NONE;
	}

	if (!master->cur_msg) {
		spi_mask_intr(dws, 0xff);
		spin_unlock_irqrestore(&dws->lock, flags);
		return IRQ_HANDLED;
	}

	status = dws->transfer_handler(dws);

	spin_unlock_irqrestore(&dws->lock, flags);
	return status;
}

static void xr_spi_irq_setup(struct dw_spi *dws)
{
	u16 level;
	u8 imask;
	u32 imr = 0;

	/*
	 * Originally Tx and Rx data lengths match. Rx FIFO Threshold level
	 * will be adjusted at the final stage of the IRQ-based SPI transfer
	 * execution so not to lose the leftover of the incoming data.
	 */
	level = min_t(u16, dws->fifo_len / 2, dws->tx_len);
	dw_writel(dws, DW_SPI_TXFTLR, level);
	dw_writel(dws, DW_SPI_RXFTLR, level - 1);

	dws->transfer_handler = dw_spi_transfer_handler;

	imask = SPI_INT_TXEI | SPI_INT_TXOI | SPI_INT_RXUI | SPI_INT_RXOI |
		 SPI_INT_RXFI;
	spi_umask_intr(dws, imask);

	imr = ~SSI_INRT_MASK_EN_B;

	dw_writel(dws, SPI_DMA_WRAP_INT_ERR_SIRQ_INT_MASK, imr);
}

static int spi_dma_transfer_one(struct spi_controller *master,
		struct spi_device *spi, struct spi_transfer *transfer)
{
	struct dw_spi *dws = spi_controller_get_devdata(master);
	struct dw_spi_cfg cfg = {
		.tmode = SPI_TMOD_TR,
		.dfs = transfer->bits_per_word,
		.freq = transfer->speed_hz,
		.ndf = transfer->len / 4,
	};
	int ret;

	dws->dma_mapped = 0;
	dws->n_bytes = DIV_ROUND_UP(transfer->bits_per_word, BITS_PER_BYTE);
	dws->tx = (void *)transfer->tx_buf;
	dws->tx_len = transfer->len / dws->n_bytes;
	dws->rx = transfer->rx_buf;
	dws->rx_len = dws->tx_len;

	/* Ensure the data above is visible for all CPUs */
	smp_mb();

	spi_enable_chip(dws, 0);
	/* Check if current transfer is a DMA transaction */
	if (master->can_dma && master->can_dma(master, spi, transfer))
		dws->dma_mapped = 1;

	if ((!transfer->tx_buf) && transfer->rx_buf && dws->dma_mapped)
		cfg.tmode = SPI_TMOD_RO;

	if (cfg.tmode == SPI_TMOD_RO)
		cfg.dfs = 32;

	ret = dw_spi_update_config(dws, spi, &cfg);
	if (ret < 0)
		return ret;

	transfer->effective_speed_hz = dws->current_freq;

	/* For poll mode just disable all interrupts */
	spi_mask_intr(dws, 0xff);

	if (cfg.tmode == SPI_TMOD_RO) {
		ret = dws->dma_ops->dma_setup(dws, transfer);
		if (ret)
			return ret;
	}

	spi_enable_chip(dws, 1);

	if (cfg.tmode == SPI_TMOD_RO)
		return dws->dma_ops->dma_transfer(dws, transfer);

	xr_spi_irq_setup(dws);

	return 1;
}

static void dma_select(struct dw_spi *dws)
{
	if (dws->dma_type == INTERNAL_DMA || dws->dma_type == EXTERNAL_DMA)
		dw_writel(dws, SPI_DMA_WRAP_SEL, dws->dma_type);
}

int xr_spi_add_host(struct device *dev, struct dw_spi *dws)
{
	struct spi_controller *master = NULL;
	int ret;

	if (!dws || !dev)
		return -EINVAL;

	master = spi_alloc_master(dev, 0);
	if (!master)
		return -ENOMEM;

	dws->master = master;
	dws->dma_addr = (dma_addr_t)(dws->paddr + DW_SPI_DR);

	spi_controller_set_devdata(master, dws);

	/* Basic HW init */
	spi_hw_init(dev, dws);

	dma_select(dws);

	ret = request_irq(dws->irq, xr_spi_irq_handler, IRQF_TRIGGER_HIGH, dev_name(dev),
			master);
	if (ret < 0 && ret != -ENOTCONN) {
		dev_err(dev, "can not get IRQ\n");
		goto err_free_master;
	}

	ret = request_irq(dws->dma_complete_irq, xr_spi_dma_irq_handler,
			IRQF_TRIGGER_HIGH, "spi_dma_irq", dws);
	if (ret < 0 && ret != -ENOTCONN) {
		dev_err(dev, "can not get spi_dma IRQ\n");
		goto err_free_spi_irq;
	}

	master->use_gpio_descriptors = true;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_LOOP;
	if (dws->caps & DW_SPI_CAP_DFS32)
		master->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 32);
	else
		master->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 16);
	master->bus_num = dws->bus_num;
	master->num_chipselect = dws->num_cs;
	master->setup = dw_spi_setup;
	master->cleanup = dw_spi_cleanup;
	master->prepare_transfer_hardware = dw_spi_prepare_transfer_hardware;
	master->unprepare_transfer_hardware = dw_spi_unprepare_transfer_hardware;
	master->transfer_one = spi_dma_transfer_one;
	master->handle_err = dw_spi_handle_err;
	if (dws->mem_ops.exec_op)
		master->mem_ops = &dws->mem_ops;
	master->max_speed_hz = BUS_MAX_SPEED_HZ;
	master->dev.of_node = dev->of_node;
	master->dev.fwnode = dev->fwnode;
	master->flags = SPI_CONTROLLER_GPIO_SS;
	master->auto_runtime_pm = false;
	/* Get default rx sample delay */
	ret = device_property_read_u32(dev, "rx-sample-delay-ns",
				 &dws->def_rx_sample_dly_ns);
	if (ret < 0)
		dws->def_rx_sample_dly_ns = 0;

	if (dws->dma_ops && dws->dma_ops->dma_init) {
		ret = dws->dma_ops->dma_init(dev, dws);
		if (ret)
			dev_warn(dev, "DMA init failed\n");
		else
			master->can_dma = dws->dma_ops->can_dma;
	}

	ret = dma_set_mask(dws->dev, DMA_BIT_MASK(64));
	if (ret)
		dev_err(dws->dev, "SPI dma_set_mask failed, ret:%d\n", ret);

	ret = spi_register_controller(master);
	if (ret) {
		dev_err(&master->dev, "problem registering spi master\n");
		goto err_dma_exit;
	}

	dev_info(dev, "spi_register_controller success\n");

	return 0;

err_dma_exit:
	if (dws->dma_ops && dws->dma_ops->dma_exit)
		dws->dma_ops->dma_exit(dws);
	spi_enable_chip(dws, 0);
	free_irq(dws->dma_complete_irq, master);
err_free_spi_irq:
	free_irq(dws->irq, master);
err_free_master:
	spi_controller_put(master);
	return ret;
}
