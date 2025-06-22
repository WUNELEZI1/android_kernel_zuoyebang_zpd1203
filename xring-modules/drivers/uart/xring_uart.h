/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __XRING_UART_H__
#define __XRING_UART_H__

#include <linux/serial_core.h>
#include <linux/kthread.h>
#include <linux/kfifo.h>
#include <linux/dmaengine.h>
#include <linux/version.h>
#include <linux/hwspinlock.h>
#include <linux/workqueue.h>

#include <dt-bindings/xring/platform-specific/dw_apb_uart_reg.h>

#define LSR_BOTH_EMPTY	(DW_UART_LSR_TEMT_MASK | DW_UART_LSR_THRE_MASK)
#define LSR_SAVE_FLAGS	DW_UART_LSR_BRK_ERROR_BITS
#define MSR_SAVE_FLAGS	DW_UART_MSR_ANY_DELTA
#define MAX_RX_COUNT	256
#define WAIT_LSR_TMOUT_US	10000
#define WAIT_MSR_TMOUT_US	1000000
#define MIN_SUPPORT_BAUD_BASE	9600
#define SUSPEND_FLAG		0xA5
#define DMA_RETRY_TIMES		256

struct console;
struct ktermios;
struct xr_uart_port;

struct xr_uart_dma {
	int (*tx_dma)(struct xr_uart_port *xp);
	int (*rx_dma)(struct xr_uart_port *xp);

	/* Filter function */
	dma_filter_fn		fn;
	/* Parameter to the filter function */
	void			*rx_param;
	void			*tx_param;

	struct dma_slave_config	rxconf;
	struct dma_slave_config	txconf;

	struct dma_chan		*rxchan;
	struct dma_chan		*txchan;

	/* Device address base for DMA operations */
	phys_addr_t		rx_dma_addr;
	phys_addr_t		tx_dma_addr;

	/* DMA address of the buffer in memory */
	dma_addr_t		rx_addr;
	dma_addr_t		tx_addr;

	dma_cookie_t		rx_cookie;
	dma_cookie_t		tx_cookie;

	void			*rx_buf;

	size_t			rx_size;
	size_t			tx_size;

	unsigned char		tx_running;
	unsigned char		tx_err;
	unsigned char		rx_running;
	unsigned char		dma_released;
};

struct xr_uart_config {
	const char *name;
	unsigned short fifo_size;
	unsigned short tx_loadsz;
	unsigned char fcr;
	unsigned char fcr_dma;
	unsigned char rxtrig_bytes[DW_UART_FCR_RT_MAX_STATE];
	unsigned int flags;
};

struct uart_tx_unit {
	int tx_cpuid; /* -1:not set */
	int max_cpus;
	struct kfifo tx_fifo;
	spinlock_t tx_lock_in;
	unsigned long tx_got;
	unsigned long tx_queued;
	unsigned long tx_out;
	unsigned long tx_sent;
	unsigned long tx_buf_times;
	unsigned long tx_buf_max; /* fifo required size max */
	unsigned long tx_buf_over_cnt;
	unsigned long tx_buf_over_size;
	unsigned long tx_uart_fifo_full;
	unsigned long tx_uart_fifo_full_lost;
	unsigned long tx_uart_tasklet_run;
	int tx_valid; /* enable or not */
	struct delayed_work console_delay_wq;
	struct workqueue_struct *console_wq;
};

struct xr_uart_port {
	struct uart_port port;
	unsigned int tx_loadsz;

	struct xr_uart_dma *dma;
	unsigned int need_rx_dma;
	unsigned int need_tx_dma;

	unsigned char lsr_saved_flags;
	unsigned char msr_saved_flags;
	unsigned char ier;
	unsigned char fcr;
	unsigned char lcr;
	unsigned char mcr;

	unsigned char canary;		/* non-zero during system sleep */
	struct uart_tx_unit tx_unit;
	struct hwspinlock *hwlock;
};

struct xr_uart_port *xr_uart_get_port(int line);

static inline struct xr_uart_port *up_to_xr(struct uart_port *up)
{
	return container_of(up, struct xr_uart_port, port);
}

static inline int serial_in(struct xr_uart_port *xp, int offset)
{
	return xp->port.serial_in(&xp->port, offset);
}

static inline void serial_out(struct xr_uart_port *xp, int offset, int value)
{
	xp->port.serial_out(&xp->port, offset, value);
}

void wait_for_xmitr(struct xr_uart_port *xp, int bits);
unsigned int xr_uart_get_baud_rate(struct uart_port *port,
					struct ktermios *termios,
					const struct ktermios *old);
unsigned int dwuart_get_divisor(struct uart_port *p, unsigned int baud,
				       unsigned int *frac);

void dwuart_set_divisor(struct uart_port *p, unsigned int baud,
				unsigned int quot, unsigned int quot_frac);
unsigned int xr_uart_modem_status(struct xr_uart_port *xp);

void xr_uart_rpm_get_tx(struct xr_uart_port *xp);
void xr_uart_rpm_put_tx(struct xr_uart_port *xp);

#ifdef CONFIG_XRING_UART_DMA
int xr_uart_request_dma(struct xr_uart_port *xp);
void xr_uart_release_dma(struct xr_uart_port *xp);
void xr_uart_rx_dma_flush(struct xr_uart_port *xp);
int xr_uart_tx_dma(struct xr_uart_port *xp);
int xr_uart_rx_dma(struct xr_uart_port *xp);
#else
static inline int xr_uart_request_dma(struct xr_uart_port *xp)
{
	return -1;
}

static inline void xr_uart_release_dma(struct xr_uart_port *xp)
{

}

static inline void xr_uart_rx_dma_flush(struct xr_uart_port *xp)
{

}

static inline int xr_uart_tx_dma(struct xr_uart_port *xp)
{
	return -1;
}

static inline int xr_uart_rx_dma(struct xr_uart_port *xp)
{
	return -1;
}
#endif /* CONFIG_XRING_UART_DMA */

void xr_uart_console_write(struct console *co, const char *s, unsigned int count);
int xr_uart_console_setup(struct console *co, char *options);
int xr_uart_console_exit(struct console *co);

void xr_uart_probe_console_enable(struct device *dev,
	struct xr_uart_port *up, const char *xr_console_name);
#endif
