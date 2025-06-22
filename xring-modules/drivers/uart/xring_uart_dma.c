// SPDX-License-Identifier: GPL-2.0-only
/*
 * Synopsys DesignWare apb-uart driver.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include "xring_uart.h"

#ifdef CONFIG_XRING_UART_DMA

#define UART_RX_DMA_SIZE	256

int xr_uart_request_dma(struct xr_uart_port *xp)
{
	struct xr_uart_dma *dma = xp->dma;
	phys_addr_t rx_dma_addr;
	phys_addr_t tx_dma_addr;
	dma_cap_mask_t mask;
	struct dma_slave_caps caps;
	int ret;

	/* Default slave configuration parameters */
	if (xp->need_rx_dma) {
		rx_dma_addr = dma->rx_dma_addr ? dma->rx_dma_addr : xp->port.mapbase;
		dma->rxconf.direction = DMA_DEV_TO_MEM;
		dma->rxconf.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		dma->rxconf.src_addr = rx_dma_addr + DW_UART_RBR;
	}

	if (xp->need_tx_dma) {
		tx_dma_addr = dma->tx_dma_addr ? dma->tx_dma_addr : xp->port.mapbase;
		dma->txconf.direction = DMA_MEM_TO_DEV;
		dma->txconf.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		dma->txconf.dst_addr = tx_dma_addr + DW_UART_THR;
	}


	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);

	/* Get a channel for RX */
	if (xp->need_rx_dma) {
		dma->rxchan = dma_request_slave_channel_compat(mask, dma->fn,
								dma->rx_param,
								xp->port.dev, "rx");
		if (!dma->rxchan) {
			xp->need_rx_dma = 0;
			return -ENODEV;
		}

		/* 8250 rx dma requires dmaengine driver to support pause/terminate */
		ret = dma_get_slave_caps(dma->rxchan, &caps);
		if (ret)
			goto release_rx;
		if (!caps.cmd_pause || !caps.cmd_terminate ||
		    caps.residue_granularity == DMA_RESIDUE_GRANULARITY_DESCRIPTOR) {
			ret = -EINVAL;
			goto release_rx;
		}

		dmaengine_slave_config(dma->rxchan, &dma->rxconf);
	}

	/* Get a channel for TX */
	if (xp->need_tx_dma) {
		dma->txchan = dma_request_slave_channel_compat(mask, dma->fn,
								dma->tx_param,
								xp->port.dev, "tx");
		if (!dma->txchan) {
			ret = -ENODEV;
			goto release_rx;
		}

		/* 8250 tx dma requires dmaengine driver to support terminate */
		ret = dma_get_slave_caps(dma->txchan, &caps);
		if (ret)
			goto err;
		if (!caps.cmd_terminate) {
			ret = -EINVAL;
			goto err;
		}

		dmaengine_slave_config(dma->txchan, &dma->txconf);
	}

	/* RX buffer */
	if (xp->need_rx_dma) {
		if (!dma->rx_size)
			dma->rx_size = UART_RX_DMA_SIZE;
		dma->rx_buf = dma_alloc_coherent(dma->rxchan->device->dev, dma->rx_size,
						&dma->rx_addr, GFP_KERNEL);
		if (!dma->rx_buf) {
			ret = -ENOMEM;
			goto err;
		}

		dev_dbg_ratelimited(xp->port.dev, "got rx dma channel\n");
	}

	/* TX buffer */
	if (xp->need_tx_dma) {
		dma->tx_addr = dma_map_single(dma->txchan->device->dev,
						xp->port.state->xmit.buf,
						UART_XMIT_SIZE,
						DMA_TO_DEVICE);
		if (dma_mapping_error(dma->txchan->device->dev, dma->tx_addr)) {
			if (xp->need_rx_dma)
				dma_free_coherent(dma->rxchan->device->dev, dma->rx_size,
						  dma->rx_buf, dma->rx_addr);
			ret = -ENOMEM;
			goto err;
		}

		dev_dbg_ratelimited(xp->port.dev, "got tx dma channel\n");
	}

	dma->dma_released = 0;

	return 0;
err:
	if (xp->need_tx_dma) {
		dma_release_channel(dma->txchan);
		xp->need_tx_dma = 0;
	}

release_rx:
	if (xp->need_rx_dma) {
		dma_release_channel(dma->rxchan);
		xp->need_rx_dma = 0;
	}

	return ret;
}

void xr_uart_release_dma(struct xr_uart_port *xp)
{
	struct xr_uart_dma *dma = xp->dma;
	enum dma_status state;
	u32 count = DMA_RETRY_TIMES;

	if (!dma)
		return;

	dma->dma_released = 1;

	do {
		state = dmaengine_tx_status(dma->txchan, dma->tx_cookie, NULL);
		if (state == DMA_COMPLETE)
			break;

		/* xmit fifo reset */
		serial_out(xp, DW_UART_FCR, DW_UART_FCR_FIFOE_MASK |
				DW_UART_FCR_XFIFOR_MASK);

		udelay(10);
		count--;
	} while (count);

	if (xp->need_rx_dma) {
		/* Release RX resources */
		dmaengine_terminate_sync(dma->rxchan);
		dma_free_coherent(dma->rxchan->device->dev, dma->rx_size, dma->rx_buf,
				  dma->rx_addr);
		dma_release_channel(dma->rxchan);
		dma->rxchan = NULL;
		dma->rx_running = 0;
	}

	if (xp->need_tx_dma) {
		/* Release TX resources */
		dmaengine_terminate_sync(dma->txchan);
		dma_unmap_single(dma->txchan->device->dev, dma->tx_addr,
			UART_XMIT_SIZE, DMA_TO_DEVICE);
		dma_release_channel(dma->txchan);
		dma->txchan = NULL;
		dma->tx_running = 0;
	}
	dev_dbg_ratelimited(xp->port.dev, "dma channels released\n");

}

static void __dma_tx_complete(void *param)
{
	struct xr_uart_port *xp = param;
	struct xr_uart_dma *dma = xp->dma;
	struct circ_buf *xmit = &xp->port.state->xmit;
	unsigned long flags;
	int ret;

	dma_sync_single_for_cpu(dma->txchan->device->dev, dma->tx_addr,
				UART_XMIT_SIZE, DMA_TO_DEVICE);

	spin_lock_irqsave(&xp->port.lock, flags);

	dma->tx_running = 0;

	xmit->tail += dma->tx_size;
	xmit->tail &= UART_XMIT_SIZE - 1;
	xp->port.icount.tx += dma->tx_size;

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&xp->port);

	ret = xr_uart_tx_dma(xp);
	if (ret || !dma->tx_running) {
		xp->ier |= DW_UART_IER_ETBEI_MASK;
		serial_port_out(&xp->port, DW_UART_IER, xp->ier);
	}

	spin_unlock_irqrestore(&xp->port.lock, flags);
}

static void __dma_rx_complete(struct xr_uart_port *xp)
{
	struct xr_uart_dma *dma = xp->dma;
	struct tty_port *tty_port = &xp->port.state->port;
	struct dma_tx_state state;
	enum dma_status dma_status;
	int count;
	int ret;

	/*
	 * New DMA Rx can be started during the completion handler before it
	 * could acquire port's lock and it might still be ongoing. Don't to
	 * anything in such case.
	 */
	dma_status = dmaengine_tx_status(dma->rxchan, dma->rx_cookie, &state);
	if (dma_status == DMA_IN_PROGRESS)
		return;

	count = dma->rx_size - state.residue + state.in_flight_bytes;
	if (count < dma->rx_size) {
		dmaengine_terminate_async(dma->rxchan);

		/*
		 * Poll for teardown to complete which guarantees in
		 * flight data is drained.
		 */
		if (state.in_flight_bytes) {
			int poll_count = 25;

			while (dmaengine_tx_status(dma->rxchan, dma->rx_cookie, NULL) &&
					poll_count--)
				cpu_relax();

			if (poll_count == -1)
				dev_err(xp->port.dev, "teardown incomplete\n");
		}
	}
	if (!count)
		goto out;

	ret = tty_insert_flip_string(tty_port, dma->rx_buf, count);
	xp->port.icount.rx += ret;
	xp->port.icount.buf_overrun += count - ret;
	dma->rx_running = 0;

out:
	tty_flip_buffer_push(tty_port);
}

static void dma_rx_complete(void *param)
{
	struct xr_uart_port *xp = param;
	struct xr_uart_dma *dma = xp->dma;
	unsigned long flags;

	spin_lock_irqsave(&xp->port.lock, flags);
	if (dma->rx_running)
		__dma_rx_complete(xp);

	/*
	 * Cannot be combined with the previous check because __dma_rx_complete()
	 * changes dma->rx_running.
	 */
	if (!dma->rx_running && (serial_in(xp, DW_UART_LSR) & DW_UART_LSR_DR_MASK))
		dma->rx_dma(xp);

	spin_unlock_irqrestore(&xp->port.lock, flags);
}

int xr_uart_tx_dma(struct xr_uart_port *xp)
{
	struct xr_uart_dma *dma = xp->dma;
	struct circ_buf *xmit = &xp->port.state->xmit;
	struct dma_async_tx_descriptor *desc;
	struct uart_port *up = &xp->port;
	int ret;

	if (!dma)
		return -EINVAL;

	if (dma->tx_running) {
		if (up->x_char) {
			dmaengine_pause(dma->txchan);
			uart_xchar_out(up, DW_UART_THR);
			dmaengine_resume(dma->txchan);
		}
		return 0;
	} else if (up->x_char) {
		uart_xchar_out(up, DW_UART_THR);
	}

	if (uart_tx_stopped(&xp->port) || uart_circ_empty(xmit) || dma->dma_released) {
		/* We have been called from __dma_tx_complete() */
		return 0;
	}

	dma->tx_size = CIRC_CNT_TO_END(xmit->head, xmit->tail, UART_XMIT_SIZE);

	desc = dmaengine_prep_slave_single(dma->txchan,
					dma->tx_addr + xmit->tail,
					dma->tx_size, DMA_MEM_TO_DEV,
					DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!desc) {
		ret = -EBUSY;
		goto err;
	}

	dma->tx_running = 1;
	desc->callback = __dma_tx_complete;
	desc->callback_param = xp;

	dma->tx_cookie = dmaengine_submit(desc);

	dma_sync_single_for_device(dma->txchan->device->dev, dma->tx_addr,
					UART_XMIT_SIZE, DMA_TO_DEVICE);

	dma_async_issue_pending(dma->txchan);
	if (xp->ier & DW_UART_IER_ETBEI_MASK) {
		xp->ier &= ~DW_UART_IER_ETBEI_MASK;
		serial_out(xp, DW_UART_IER, xp->ier);
	}
	dma->tx_err = 0;

	return 0;
err:
	dma->tx_err = 1;
	return ret;
}

int xr_uart_rx_dma(struct xr_uart_port *xp)
{
	struct xr_uart_dma *dma = xp->dma;
	struct dma_async_tx_descriptor *desc;

	if (!dma)
		return -EINVAL;

	if (dma->rx_running)
		return 0;

	desc = dmaengine_prep_slave_single(dma->rxchan, dma->rx_addr,
						dma->rx_size, DMA_DEV_TO_MEM,
						DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!desc)
		return -EBUSY;

	dma->rx_running = 1;
	desc->callback = dma_rx_complete;
	desc->callback_param = xp;

	dma->rx_cookie = dmaengine_submit(desc);

	dma_async_issue_pending(dma->rxchan);

	return 0;
}

void xr_uart_rx_dma_flush(struct xr_uart_port *xp)
{
	struct xr_uart_dma *dma = xp->dma;
	struct dma_tx_state state;
	int ret;

	if (!dma)
		return;

	if (dma->rx_running) {
		ret = dmaengine_tx_status(dma->rxchan, dma->rx_cookie, &state);
		if (ret == DMA_IN_PROGRESS)
			dmaengine_pause(dma->rxchan);
		__dma_rx_complete(xp);
		dmaengine_terminate_async(dma->rxchan);
	}
}

#endif /* CONFIG_XRING_UART_DMA */
