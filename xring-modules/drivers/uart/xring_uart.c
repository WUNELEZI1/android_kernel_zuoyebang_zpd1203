// SPDX-License-Identifier: GPL-2.0-only
/*
 * Synopsys DesignWare apb-uart driver.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/dmaengine.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/nmi.h>
#include <linux/tty.h>
#include <linux/ratelimit.h>
#include <linux/tty_flip.h>
#include <linux/acpi.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/pm_runtime.h>
#include <linux/version.h>

#include <asm/byteorder.h>
#include "xring_uart.h"
#include <dt-bindings/xring/platform-specific/peri_crg.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/hwlock/ap_spin_lock_id.h>

#include <soc/xring/doze.h>
#include <dt-bindings/xring/platform-specific/pm/include/sys_doze_plat.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h>

#define UART_RESET_BIT 21

struct xr_uart_data {
	/* Port properties */
	int		line;

	/* Hardware configuration */
	u8		dlf_size;

	unsigned int usr_reg;
	int msr_mask_on;
	int msr_mask_off;
	struct clk *clk;
	struct clk *pclk;
	struct reset_control *rst;
	struct xr_uart_dma dma;
	unsigned int clk_count;
	unsigned int in_idle:1;
};

static struct xr_uart_config xr_uart_config = {
	.name		= "DW16550A",
	.fifo_size	= 64,
	.tx_loadsz	= 64,
	.fcr		= DW_UART_FCR_FIFOE_MASK | DW_UART_FCR_RT_10,
	.fcr_dma	= DW_UART_FCR_FIFOE_MASK |
				DW_UART_FCR_RT_01 | DW_UART_FCR_TET_10,
	.rxtrig_bytes	= {1, 16, 32, 64},
};

#define UART_NR		CONFIG_XRING_UART_NR_UARTS

static struct xr_uart_port xr_uart_ports[UART_NR];

static struct uart_driver xr_uart_driver;

static atomic_t xr_uart_next_id = ATOMIC_INIT(0);

struct xr_uart_port *xr_uart_get_port(int line)
{
	return &xr_uart_ports[line];
}

static inline u32 xr_uart_readl_ext(struct uart_port *p, int offset)
{
	return readl(p->membase + offset);
}

static inline void xr_uart_writel_ext(struct uart_port *p, int offset, u32 reg)
{
	writel(reg, p->membase + offset);
}

static inline int xr_uart_modify_msr(struct uart_port *p, int offset, int value)
{
	struct xr_uart_data *d = p->private_data;

	/* Override any modem control signals if needed */
	if (offset == DW_UART_MSR) {
		value |= d->msr_mask_on;
		value &= ~d->msr_mask_off;
	}

	return value;
}

#ifdef CONFIG_PM
static int xr_uart_runtime_suspend(struct device *dev)
{
	struct xr_uart_data *data = dev_get_drvdata(dev);

	/* uart6 is also used by other subsystems, do not operate it */
	if (data->line != 1) {
		if (data->clk_count == 0)
			dev_dbg(dev, "%s clock count is 0\n", __func__);
		else {
			clk_disable_unprepare(data->clk);
			clk_disable_unprepare(data->pclk);

			data->clk_count--;
		}

	}

	return 0;
}

static int xr_uart_runtime_resume(struct device *dev)
{
	struct xr_uart_data *data = dev_get_drvdata(dev);
	int ret = 0;

	/* uart6 is also used by other subsystems, do not operate it */
	if (data->line != 1) {
		if (data->clk_count > 0)
			dev_dbg(dev, "[%s]: data->line[%d] clock count is %d\n", __func__,
				data->line, data->clk_count);
		else {
			ret = clk_prepare_enable(data->pclk);
			if (ret) {
				dev_err(dev, "could not enable apb_pclk\n");
				return ret;
			}

			ret = clk_prepare_enable(data->clk);
			if (ret) {
				dev_err(dev, "could not enable baudclk\n");
				return ret;
			}

			data->clk_count++;
		}
	}
	return 0;
}
#endif

static void xr_uart_set_divisor(struct uart_port *p, unsigned int baud,
				unsigned int quot, unsigned int quot_frac)
{
	struct xr_uart_port *xp = up_to_xr(p);

	p->serial_out(p, DW_UART_LCR, xp->lcr | DW_UART_LCR_DLAB_MASK);

	p->serial_out(p, DW_UART_DLL, quot & 0xff);
	/* set the high 8 bits */
	p->serial_out(p, DW_UART_DLH, quot >> 8 & 0xff);

	xr_uart_writel_ext(p, DW_UART_DLF, quot_frac);
}
/*
 * FIFO support.
 */
static void xr_uart_clear_fifos(struct xr_uart_port *p)
{
	serial_out(p, DW_UART_FCR, DW_UART_FCR_FIFOE_MASK);
	/* rcvr fifo reset and xmit fifo reset */
	serial_out(p, DW_UART_FCR, DW_UART_FCR_FIFOE_MASK |
				DW_UART_FCR_RFIFOR_MASK | DW_UART_FCR_XFIFOR_MASK);
	serial_out(p, DW_UART_FCR, 0);
}

static void xr_uart_force_idle(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);

	xr_uart_clear_fifos(xp);

	(void)p->serial_in(p, DW_UART_RBR);
}

static void xr_uart_check_lcr(struct uart_port *p, int value)
{
	int tries = 1000;

	while (tries--) {
		unsigned int lcr = p->serial_in(p, DW_UART_LCR);

		if ((value & ~DW_UART_LCR_SP_MASK) == (lcr & ~DW_UART_LCR_SP_MASK))
			return;

		xr_uart_force_idle(p);

		xr_uart_writel_ext(p, DW_UART_LCR, value);
	}
	/*
	 * FIXME: this deadlocks if port->lock is already held
	 * dev_err(p->dev, "Couldn't set LCR to %d\n", value);
	 */
}

static void xr_uart_serial_out(struct uart_port *p, int offset, int value)
{
	writel(value, p->membase + offset);

	if (offset == DW_UART_LCR)
		xr_uart_check_lcr(p, value);
}

static unsigned int xr_uart_serial_in(struct uart_port *p, int offset)
{
	unsigned int value = readl(p->membase + offset);

	return xr_uart_modify_msr(p, offset, value);
}

/* Enable Transmit Holding Register Empty Interrupt */
static inline bool xr_uart_enable_tx_intr(struct xr_uart_port *xp)
{
	if (xp->ier & DW_UART_IER_ETBEI_MASK)
		return false;
	xp->ier |= DW_UART_IER_ETBEI_MASK;
	serial_out(xp, DW_UART_IER, xp->ier);
	return true;
}

/* Disable Transmit Holding Register Empty Interrupt */
static inline bool xr_uart_disable_tx_intr(struct xr_uart_port *xp)
{
	if (!(xp->ier & DW_UART_IER_ETBEI_MASK))
		return false;
	xp->ier &= ~DW_UART_IER_ETBEI_MASK;
	serial_out(xp, DW_UART_IER, xp->ier);
	return true;
}

static void xr_uart_rpm_get(struct xr_uart_port *xp)
{
}

static void xr_uart_rpm_put(struct xr_uart_port *xp)
{
}

unsigned int xr_uart_get_baud_rate(struct uart_port *p,
					struct ktermios *termios,
					const struct ktermios *old)
{
	unsigned int tolerance = p->uartclk / 100;
	unsigned int min;
	unsigned int max;

	min = p->uartclk / DW_UART_DIV_LEVEL / DW_UART_DIV_MAX;
	max = (p->uartclk + tolerance) / DW_UART_DIV_LEVEL;

	/*
	 * Ask the core to calculate the divisor for us.
	 * Allow 1% tolerance at the upper limit so uart clks marginally
	 * slower than nominal still match standard baud rates without
	 * causing transmission errors.
	 */
	return uart_get_baud_rate(p, termios, old, min, max);
}


/*
 * divisor = div(I) + div(F)
 * "I" means integer, "F" means fractional
 * quot = div(I) = clk / (16 * baud)
 * frac = div(F) * 2^dlf_size
 *
 * let rem = clk % (16 * baud)
 * we have: div(F) * (16 * baud) = rem
 * so frac = 2^dlf_size * rem / (16 * baud) = (rem << dlf_size) / (16 * baud)
 */
unsigned int dwuart_get_divisor(struct uart_port *p, unsigned int baud,
				       unsigned int *frac)
{
	unsigned int quot, rem, base_baud = baud * DW_UART_DIV_LEVEL;
	struct xr_uart_data *d = p->private_data;

	quot = p->uartclk / base_baud;
	rem = p->uartclk % base_baud;
	*frac = DIV_ROUND_CLOSEST(rem << d->dlf_size, base_baud);

	return quot;
}

void dwuart_set_divisor(struct uart_port *p, unsigned int baud,
				unsigned int quot, unsigned int quot_frac)
{
	xr_uart_set_divisor(p, baud, quot, quot_frac);
}

static void xr_uart_set_sleep(struct xr_uart_port *xp, int sleep)
{
}

static unsigned char xr_uart_compute_lcr(struct xr_uart_port *xp,
					    tcflag_t c_cflag)
{
	unsigned char cval;

	switch (c_cflag & CSIZE) {
	case CS5:
		cval = DW_UART_LCR_WLEN5;
		break;
	case CS6:
		cval = DW_UART_LCR_WLEN6;
		break;
	case CS7:
		cval = DW_UART_LCR_WLEN7;
		break;
	default:
	case CS8:
		cval = DW_UART_LCR_WLEN8;
		break;
	}

	if (c_cflag & CSTOPB)
		cval |= DW_UART_LCR_STOP_MASK;
	if (c_cflag & PARENB)
		cval |= DW_UART_LCR_PEN_MASK;
	if (!(c_cflag & PARODD))
		cval |= DW_UART_LCR_EPS_MASK;
#ifdef CMSPAR
	if (c_cflag & CMSPAR)
		cval |= DW_UART_LCR_SP_MASK;
#endif

	return cval;
}

/* MCR <-> TIOCM conversion */
static inline int xr_uart_TIOCM_to_MCR(int tiocm)
{
	int mcr = 0;

	if (tiocm & TIOCM_RTS)
		mcr |= DW_UART_MCR_RTS_MASK;
	if (tiocm & TIOCM_DTR)
		mcr |= DW_UART_MCR_DTR_MASK;
	if (tiocm & TIOCM_OUT1)
		mcr |= DW_UART_MCR_OUT1_MASK;
	if (tiocm & TIOCM_OUT2)
		mcr |= DW_UART_MCR_OUT2_MASK;
	if (tiocm & TIOCM_LOOP)
		mcr |= DW_UART_MCR_LB_MASK;

	return mcr;
}

/* MSR <-> TIOCM conversion */
static inline int xr_uart_msr_to_tiocm(int msr)
{
	int tiocm = 0;

	if (msr & DW_UART_MSR_DCD_MASK)
		tiocm |= TIOCM_CAR;
	if (msr & DW_UART_MSR_RI_MASK)
		tiocm |= TIOCM_RNG;
	if (msr & DW_UART_MSR_DSR_MASK)
		tiocm |= TIOCM_DSR;
	if (msr & DW_UART_MSR_CTS_MASK)
		tiocm |= TIOCM_CTS;

	return tiocm;
}

static void xr_uart_set_mctrl(struct uart_port *p, unsigned int mctrl)
{
	struct xr_uart_port *xp = up_to_xr(p);
	unsigned char mcr;

	mcr = xr_uart_TIOCM_to_MCR(mctrl);

	mcr = mcr | xp->mcr;

	serial_out(xp, DW_UART_MCR, mcr);
}

static void xr_uart_do_set_termios(struct uart_port *p,
					struct ktermios *termios, const struct ktermios *old)
{
	struct xr_uart_port *xp = up_to_xr(p);
	unsigned char cval;
	unsigned long flags;
	unsigned int baud;
	unsigned int quot;
	unsigned int frac = 0;
	struct xr_uart_data *data = p->private_data;
#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
	int ret = 0;
	struct hwspinlock *hwlock = xp->hwlock;
#endif

	if (data->clk_count == 0) {
		pr_info("debug: %s: clk_count = %d, please open ttyXR[%d]\n", __func__,
			data->clk_count, data->line);
		return;
	}

	cval = xr_uart_compute_lcr(xp, termios->c_cflag);
	baud = xr_uart_get_baud_rate(p, termios, old);
	quot = dwuart_get_divisor(p, baud, &frac);

	/*
	 * Ok, we're now changing the port state.  Do it with
	 * interrupts disabled.
	 */
	xr_uart_rpm_get(xp);
	spin_lock_irqsave(&p->lock, flags);

	xp->lcr = cval;	/* Save computed LCR */

	/*
	 * MCR-based auto flow control.  When AFCE is enabled, RTS will be
	 * deasserted when the receive FIFO contains more characters than
	 * the trigger, or the MCR RTS bit is cleared.
	 */

	xp->mcr &= ~DW_UART_MCR_AFCE_MASK;
	if (termios->c_cflag & CRTSCTS)
		xp->mcr |= DW_UART_MCR_AFCE_MASK;

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(p, termios->c_cflag, baud);

	p->read_status_mask = DW_UART_LSR_OE_MASK |
					DW_UART_LSR_THRE_MASK |
					DW_UART_LSR_DR_MASK;
	if (termios->c_iflag & INPCK)
		p->read_status_mask |= DW_UART_LSR_FE_MASK | DW_UART_LSR_PE_MASK;
	if (termios->c_iflag & (IGNBRK | BRKINT | PARMRK))
		p->read_status_mask |= DW_UART_LSR_BI_MASK;

	/*
	 * Characteres to ignore
	 */
	p->ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		p->ignore_status_mask |= DW_UART_LSR_PE_MASK | DW_UART_LSR_FE_MASK;
	if (termios->c_iflag & IGNBRK) {
		p->ignore_status_mask |= DW_UART_LSR_BI_MASK;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			p->ignore_status_mask |= DW_UART_LSR_OE_MASK;
	}

	/*
	 * ignore all characters if CREAD is not set
	 */
	if ((termios->c_cflag & CREAD) == 0)
		p->ignore_status_mask |= DW_UART_LSR_DR_MASK;

	/*
	 * CTS flow control flag and modem status interrupts
	 */
	xp->ier &= ~DW_UART_IER_EDSSI_MASK;
	if (UART_ENABLE_MS(&xp->port, termios->c_cflag))
		xp->ier |= DW_UART_IER_EDSSI_MASK;

#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
	if (uart_console(p)) {
lock_retry:
		ret = hwspin_lock_timeout(hwlock, 2);
		if (ret)
			goto lock_retry;
	}
#endif

	serial_out(xp, DW_UART_IER, xp->ier);

	dwuart_set_divisor(p, baud, quot, frac);

	serial_out(xp, DW_UART_LCR, xp->lcr);

	if ((xp->ier & (DW_UART_IER_ELSI_MASK | DW_UART_IER_ERBFI_MASK)) ==
	    (DW_UART_IER_ELSI_MASK | DW_UART_IER_ERBFI_MASK))
		serial_port_out(p, DW_UART_FCR, xp->fcr);

	xr_uart_set_mctrl(p, p->mctrl);

	/* release the lock */
#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
	if (uart_console(p))
		hwspin_unlock(hwlock);
#endif

	spin_unlock_irqrestore(&p->lock, flags);

	xr_uart_rpm_put(xp);

	/* Don't rewrite B0 */
	if (tty_termios_baud_rate(termios))
		tty_termios_encode_baud_rate(termios, baud, baud);
}

static void xr_uart_set_termios(struct uart_port *p, struct ktermios *termios,
			       const struct ktermios *old)
{
	p->status &= ~UPSTAT_AUTOCTS;
	if (termios->c_cflag & CRTSCTS)
		p->status |= UPSTAT_AUTOCTS;

	xr_uart_do_set_termios(p, termios, old);
}

/* enable modem status interrupt */
static void xr_uart_enable_ms(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);

	xp->ier |= DW_UART_IER_EDSSI_MASK;

	xr_uart_rpm_get(xp);
	serial_port_out(p, DW_UART_IER, xp->ier);
	xr_uart_rpm_put(xp);
}

/* disable modem status interrupt */
static void xr_uart_disable_ms(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);

	xp->ier &= ~DW_UART_IER_EDSSI_MASK;
	serial_port_out(p, DW_UART_IER, xp->ier);
}

static void xr_uart_do_set_ldisc(struct uart_port *p, struct ktermios *termios)
{
	if (termios->c_line == N_PPS) {
		p->flags |= UPF_HARDPPS_CD;
		spin_lock_irq(&p->lock);
		xr_uart_enable_ms(p);
		spin_unlock_irq(&p->lock);
	} else {
		p->flags &= ~UPF_HARDPPS_CD;
		if (!UART_ENABLE_MS(p, termios->c_cflag)) {
			spin_lock_irq(&p->lock);
			xr_uart_disable_ms(p);
			spin_unlock_irq(&p->lock);
		}
	}
}

static void xr_uart_set_ldisc(struct uart_port *p, struct ktermios *termios)
{
	xr_uart_do_set_ldisc(p, termios);
}

static void xr_uart_param_parse(struct uart_port *p)
{
	struct xr_uart_data *d = p->private_data;
	struct xr_uart_port *xp = up_to_xr(p);
	u32 reg, old_dlf;

	/* default value based on the platform */
	d->dlf_size = DW_UART_DLF_SIZE;
	p->get_divisor = dwuart_get_divisor;
	p->set_divisor = dwuart_set_divisor;
	p->type = PORT_16550A;
	p->flags |= UPF_FIXED_TYPE;
	p->fifosize = DW_CC_UART_FIFO_MODE;

	/*
	 * Check the Component Version Register
	 */
	reg = xr_uart_readl_ext(p, DW_UART_UCV);
	if (!reg)
		return;

	dev_dbg(p->dev, "Designware UART version %c.%c%c\n",
		(reg >> 24) & 0xff, (reg >> 16) & 0xff, (reg >> 8) & 0xff);

	old_dlf = xr_uart_readl_ext(p, DW_UART_DLF);
	xr_uart_writel_ext(p, DW_UART_DLF, ~0U);
	reg = xr_uart_readl_ext(p, DW_UART_DLF);
	xr_uart_writel_ext(p, DW_UART_DLF, old_dlf);

	if (reg) {
		d->dlf_size = fls(reg);
		p->get_divisor = dwuart_get_divisor;
		p->set_divisor = dwuart_set_divisor;
	}

	reg = xr_uart_readl_ext(p, DW_UART_CPR);
	if (!reg)
		return;

	/* Select the type based on FIFO */
	if (reg & DW_UART_CPR_FIFO_MODE_MASK) {
		p->type = PORT_16550A;
		p->flags |= UPF_FIXED_TYPE;
		p->fifosize = DW_UART_CPR_FIFO_MODE_CALC(reg);
		xp->fcr = DW_UART_FCR_FIFOE_MASK | DW_UART_FCR_RT_10;
	}

	dev_dbg(p->dev, "dw_uart: fifo size=%d, dlf_size=%d\n",
		p->fifosize, d->dlf_size);
}

/*
 * xr_uart_fallback_dma_filter will prevent the UART from getting just any free
 * channel on platforms that have DMA engines, but don't have any channels
 * assigned to the UART.
 *
 * REVISIT: This is a work around for limitation in the DMA Engine API. Once the
 * core problem is fixed, this function is no longer needed.
 */
static bool xr_uart_fallback_dma_filter(struct dma_chan *chan, void *param)
{
	return false;
}

static unsigned int xr_uart_tx_empty(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);
	unsigned long flags;
	unsigned int lsr;

	xr_uart_rpm_get(xp);

	spin_lock_irqsave(&p->lock, flags);
	lsr = serial_port_in(p, DW_UART_LSR);
	xp->lsr_saved_flags |= lsr & LSR_SAVE_FLAGS;
	spin_unlock_irqrestore(&p->lock, flags);

	xr_uart_rpm_put(xp);

	return (lsr & LSR_BOTH_EMPTY) == LSR_BOTH_EMPTY ? TIOCSER_TEMT : 0;
}

/* Caller holds uart port lock */
unsigned int xr_uart_modem_status(struct xr_uart_port *xp)
{
	struct uart_port *p = &xp->port;
	unsigned int status = serial_in(xp, DW_UART_MSR);

	status |= xp->msr_saved_flags;
	xp->msr_saved_flags = 0;
	if (status & DW_UART_MSR_ANY_DELTA && xp->ier & DW_UART_IER_EDSSI_MASK &&
		p->state != NULL) {
		if (status & DW_UART_MSR_RI_MASK)
			p->icount.rng++;
		if (status & DW_UART_MSR_DDSR_MASK)
			p->icount.dsr++;
		if (status & DW_UART_MSR_DDCD_MASK)
			uart_handle_dcd_change(p, status & DW_UART_MSR_DCD_MASK);
		if (status & DW_UART_MSR_DCTS_MASK)
			uart_handle_cts_change(p, status & DW_UART_MSR_CTS_MASK);

		wake_up_interruptible(&p->state->port.delta_msr_wait);
	}

	return status;
}

static unsigned int xr_uart_get_mctrl(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);
	unsigned int status;
	unsigned int val;

	xr_uart_rpm_get(xp);
	status = xr_uart_modem_status(xp);
	xr_uart_rpm_put(xp);

	val = xr_uart_msr_to_tiocm(status);

	return val;
}

void xr_uart_rpm_get_tx(struct xr_uart_port *xp)
{
}

void xr_uart_rpm_put_tx(struct xr_uart_port *xp)
{
}

static inline void __stop_tx(struct xr_uart_port *xp)
{
	if (xr_uart_disable_tx_intr(xp))
		xr_uart_rpm_put_tx(xp);
}

static void xr_uart_stop_tx(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);

	xr_uart_rpm_get(xp);
	__stop_tx(xp);
	xr_uart_rpm_put(xp);
}

static void xr_uart_stop_rx(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);

	xr_uart_rpm_get(xp);

	xp->ier &= ~(DW_UART_IER_ELSI_MASK | DW_UART_IER_ERBFI_MASK);
	xp->port.read_status_mask &= ~DW_UART_LSR_DR_MASK;
	serial_port_out(p, DW_UART_IER, xp->ier);

	xr_uart_rpm_put(xp);
}

static void xr_uart_read_char(struct xr_uart_port *xp, unsigned char lsr)
{
	struct uart_port *p = &xp->port;
	unsigned char ch = 0;
	char flag = TTY_NORMAL;

	if (likely(lsr & DW_UART_LSR_DR_MASK))
		ch = serial_in(xp, DW_UART_RBR);

	p->icount.rx++;

	lsr |= xp->lsr_saved_flags;
	xp->lsr_saved_flags = 0;

	if (unlikely(lsr & DW_UART_LSR_BRK_ERROR_BITS)) {
		if (lsr & DW_UART_LSR_BI_MASK) {
			lsr &= ~(DW_UART_LSR_FE_MASK | DW_UART_LSR_PE_MASK);
			p->icount.brk++;
			/*
			 * We do the SysRQ and SAK checking
			 * here because otherwise the break
			 * may get masked by ignore_status_mask
			 * or read_status_mask.
			 */
			if (uart_handle_break(p))
				return;
		} else if (lsr & DW_UART_LSR_PE_MASK)
			p->icount.parity++;
		else if (lsr & DW_UART_LSR_FE_MASK)
			p->icount.frame++;
		if (lsr & DW_UART_LSR_OE_MASK)
			p->icount.overrun++;

		/*
		 * Mask off conditions which should be ignored.
		 */
		lsr &= p->read_status_mask;

		if (lsr & DW_UART_LSR_BI_MASK) {
			dev_dbg(p->dev, "handling break\n");
			flag = TTY_BREAK;
		} else if (lsr & DW_UART_LSR_PE_MASK)
			flag = TTY_PARITY;
		else if (lsr & DW_UART_LSR_FE_MASK)
			flag = TTY_FRAME;
	}
	if (uart_prepare_sysrq_char(p, ch))
		return;

	uart_insert_char(p, lsr, DW_UART_LSR_OE_MASK, ch, flag);
}

static unsigned char xr_uart_rx_chars(struct xr_uart_port *xp, unsigned char lsr)
{
	struct uart_port *p = &xp->port;
	int max_count = MAX_RX_COUNT;

	do {
		xr_uart_read_char(xp, lsr);
		if (--max_count == 0)
			break;
		lsr = serial_in(xp, DW_UART_LSR);
	} while (lsr & (DW_UART_LSR_DR_MASK | DW_UART_LSR_BI_MASK));

	tty_flip_buffer_push(&p->state->port);
	return lsr;
}

static void xr_uart_tx_chars(struct xr_uart_port *xp)
{
	struct uart_port *p = &xp->port;
	struct circ_buf *xmit = &p->state->xmit;
	int count;

	if (p->x_char) {
		uart_xchar_out(p, DW_UART_THR);
		return;
	}
	if (uart_tx_stopped(p)) {
		xr_uart_stop_tx(p);
		return;
	}
	if (uart_circ_empty(xmit)) {
		__stop_tx(xp);
		return;
	}

	count = xp->tx_loadsz;
	do {
		if (p->serial_in(p, DW_UART_TFL) <= DW_CC_UART_FIFO_MODE - 1) {
			serial_out(xp, DW_UART_THR, xmit->buf[xmit->tail]);
			xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
			p->icount.tx++;
			if (uart_circ_empty(xmit))
				break;
		}
	} while (--count > 0);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(p);

	if (uart_circ_empty(xmit))
		__stop_tx(xp);
}

static inline void __start_tx(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);

	if (xp->dma) {
		if (xp->need_rx_dma)
			if (xp->dma->rx_dma(xp))
				return;

		if (xp->need_tx_dma)
			if (xp->dma->tx_dma(xp))
				return;
	}

	xr_uart_enable_tx_intr(xp);
}

static void xr_uart_start_tx(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);

	if (!p->x_char && uart_circ_empty(&p->state->xmit))
		return;

	xr_uart_rpm_get_tx(xp);

	__start_tx(p);
}

static void xr_uart_throttle(struct uart_port *p)
{
	p->throttle(p);
}

static void xr_uart_unthrottle(struct uart_port *p)
{
	p->unthrottle(p);
}

static void xr_uart_break_ctl(struct uart_port *p, int break_state)
{
	struct xr_uart_port *xp = up_to_xr(p);
	unsigned long flags;

	xr_uart_rpm_get(xp);
	spin_lock_irqsave(&p->lock, flags);
	if (break_state == -1)
		xp->lcr |= DW_UART_LCR_BC_MASK;
	else
		xp->lcr &= ~DW_UART_LCR_BC_MASK;
	serial_port_out(p, DW_UART_LCR, xp->lcr);
	spin_unlock_irqrestore(&p->lock, flags);
	xr_uart_rpm_put(xp);
}

static bool handle_rx_dma(struct xr_uart_port *xp, unsigned int iir)
{
	switch (iir & DW_UART_IIR_MASK) {
	case DW_UART_IIR_THRI:
		/*
		 * Postpone DMA or not decision to IIR_RDI or IIR_RX_TIMEOUT
		 * because it's impossible to do an informed decision about
		 * that with IIR_THRI.
		 *
		 * This also fixes one known DMA Rx corruption issue where
		 * DR is asserted but DMA Rx only gets a corrupted zero byte
		 * (too early DR?).
		 */
		return false;
	case DW_UART_IIR_RDI:
		if (!xp->dma->rx_running)
			break;
		fallthrough;
	case DW_UART_IIR_RLSI:
	case DW_UART_IIR_RX_TIMEOUT:
		xr_uart_rx_dma_flush(xp);
		return true;
	default:
		break;
	}

	return xp->dma->rx_dma(xp);
}

static int dw_uart_handle_irq(struct uart_port *p, unsigned int iir)
{
	unsigned int status;
	struct xr_uart_port *xp = up_to_xr(p);
	bool skip_rx = false;
	unsigned long flags;

	if (iir & DW_UART_IIR_NO_INT)
		return IRQ_NONE;

	spin_lock_irqsave(&p->lock, flags);

	status = p->serial_in(p, DW_UART_LSR);

	/*
	 * If port is stopped and there are no error conditions in the
	 * FIFO, then don't drain the FIFO, as this may lead to TTY buffer
	 * overflow. Not servicing, RX FIFO would trigger auto HW flowxring_ua
	 * control when FIFO occupancy reaches preset threshold, thus
	 * halting RX. This only works when auto HW flow control is
	 * available.
	 */
	if (!(status & (DW_UART_LSR_RFE_MASK | DW_UART_LSR_BRK_ERROR_BITS)) &&
	    (p->status & (UPSTAT_AUTOCTS | UPSTAT_AUTORTS)) &&
	    !(p->read_status_mask & DW_UART_LSR_DR_MASK))
		skip_rx = true;
	if (status & (DW_UART_LSR_DR_MASK | DW_UART_LSR_BI_MASK)  && !skip_rx) {
		if (!xp->need_rx_dma) {
			status = xr_uart_rx_chars(xp, status);
		} else {
			if (handle_rx_dma(xp, iir)) {
				status = p->serial_in(p, DW_UART_LSR);
				status = xr_uart_rx_chars(xp, status);
			}
		}
	}

	xr_uart_modem_status(xp);

	if ((status & DW_UART_LSR_THRE_MASK) &&
		(xp->ier & DW_UART_IER_ETBEI_MASK)) {
		if (!xp->need_tx_dma || xp->dma->tx_err)
			xr_uart_tx_chars(xp);
		else if (!xp->dma->tx_running)
			__stop_tx(xp);
	}

	uart_unlock_and_check_sysrq_irqrestore(p, flags);

	return IRQ_HANDLED;
}

static int xr_uart_handle_irq(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);
	struct xr_uart_data *d = p->private_data;
	unsigned int iir;
	unsigned int status;
	unsigned long flags;
	bool rx_timeout;

	/*
	 * There are ways to get Designware-based UARTs into a state where
	 * they are asserting UART_IIR_RX_TIMEOUT but there is no actual
	 * data available.  If we see such a case then we'll do a bogus
	 * read.  If we don't do this then the "RX TIMEOUT" interrupt will
	 * fire forever.
	 *
	 * This problem has only been observed so far when not in DMA mode
	 * so we limit the workaround only to non-DMA mode.
	 */
	spin_lock_irqsave(&p->lock, flags);
	iir = p->serial_in(p, DW_UART_IIR);
	rx_timeout = (iir & 0x3f) == DW_UART_IIR_RX_TIMEOUT;
	if (!xp->need_rx_dma && rx_timeout) {
		status = p->serial_in(p, DW_UART_LSR);

		if (!(status & (DW_UART_LSR_DR_MASK | DW_UART_LSR_BI_MASK))) {
			p->serial_out(p, DW_UART_FCR, 0);
			(void) p->serial_in(p, DW_UART_RBR);
			p->serial_out(p, DW_UART_FCR, xp->fcr);
		}
	}
	spin_unlock_irqrestore(&p->lock, flags);

	if (dw_uart_handle_irq(p, iir))
		return 1;

	if ((iir & DW_UART_IIR_BUSY) == DW_UART_IIR_BUSY) {
		/* Clear the USR */
		(void)p->serial_in(p, d->usr_reg);

		return 1;
	}

	return 0;

}

static irqreturn_t xr_uart_interrupt(int irq, void *dev_id)
{
	struct uart_port *p = dev_id;
	int ret;

	ret = xr_uart_handle_irq(p);

	return IRQ_RETVAL(ret);
}


/*
 *	Wait for transmitter & holding register to empty
 */
void wait_for_xmitr(struct xr_uart_port *xp, int bits)
{
	unsigned int status, tmout = WAIT_LSR_TMOUT_US;

	if (!xp)
		return;

	/* Wait up to 10ms for the character(s) to be sent. */
	for (;;) {
		status = serial_in(xp, DW_UART_LSR);

		xp->lsr_saved_flags |= status & LSR_SAVE_FLAGS;

		if ((status & bits) == bits)
			break;
		if (--tmout == 0)
			break;
		udelay(1);
		touch_nmi_watchdog();
	}

	/* Wait up to 1s for flow control if necessary */
	if (xp->port.flags & UPF_CONS_FLOW) {
		for (tmout = WAIT_MSR_TMOUT_US; tmout; tmout--) {
			unsigned int msr = serial_in(xp, DW_UART_MSR);

			xp->msr_saved_flags |= msr & MSR_SAVE_FLAGS;
			if (msr & DW_UART_MSR_CTS_MASK)
				break;
			udelay(1);
			touch_nmi_watchdog();
		}
	}
}

static int xr_uart_setup_irq(struct xr_uart_port *xp)
{
	struct uart_port *p = &xp->port;
	int ret;

	ret = request_irq(p->irq, xr_uart_interrupt, p->irqflags,
				p->name, p);
	if (ret)
		dev_err(p->dev, "irq request failed\n");

	return ret;
}

static void xr_uart_release_irq(struct xr_uart_port *xp)
{
	struct uart_port *p = &xp->port;

	if (p->irq)
		free_irq(p->irq, p);
}

static int xr_uart_startup(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);
	unsigned long flags;
	int retval;

	if (!p->fifosize || !xp->tx_loadsz) {
		dev_err(p->dev, "Invalid port configs\n");
		return -EINVAL;
	}

	xp->mcr = 0;

	xr_uart_rpm_get(xp);

	/*
	 * Clear the FIFO buffers and disable them.
	 * (they will be reenabled in set_termios())
	 */
	xr_uart_clear_fifos(xp);

	/*
	 * Clear the interrupt registers.
	 */
	serial_port_in(p, DW_UART_LSR);
	serial_port_in(p, DW_UART_RBR);
	serial_port_in(p, DW_UART_IIR);
	serial_port_in(p, DW_UART_MSR);

	/*
	 * At this point, there's no way the LSR could still be 0xff;
	 * if it is, then bail out, because there's likely no UART
	 * here.
	 */
	if (!(p->flags & UPF_BUGGY_UART) &&
	    (serial_port_in(p, DW_UART_LSR) == 0xff)) {
		dev_info_ratelimited(p->dev, "LSR safety check engaged!\n");
		retval = -ENODEV;
		goto out;
	}

	/* Check if we need to have shared IRQs */
	if (p->irq && (xp->port.flags & UPF_SHARE_IRQ))
		xp->port.irqflags |= IRQF_SHARED;

	retval = xr_uart_setup_irq(xp);
	if (retval)
		goto out;

	/*
	 * Now, initialize the UART
	 */
	serial_port_out(p, DW_UART_LCR, DW_UART_LCR_WLEN8);

	spin_lock_irqsave(&p->lock, flags);
	if (xp->port.flags & UPF_FOURPORT) {
		if (!xp->port.irq)
			xp->port.mctrl |= TIOCM_OUT1;
	} else {
		/*
		 * Most PC uarts need OUT2 raised to enable interrupts.
		 */
		if (p->irq)
			xp->port.mctrl |= TIOCM_OUT2;
	}

	xr_uart_set_mctrl(p, p->mctrl);

	spin_unlock_irqrestore(&p->lock, flags);

	/*
	 * Clear the interrupt registers again for luck, and clear the
	 * saved flags to avoid getting false values from polling
	 * routines or the previous session.
	 */
	serial_port_in(p, DW_UART_LSR);
	serial_port_in(p, DW_UART_RBR);
	serial_port_in(p, DW_UART_IIR);
	serial_port_in(p, DW_UART_MSR);
	xp->lsr_saved_flags = 0;
	xp->msr_saved_flags = 0;

	/*
	 * Request DMA channels for both RX and TX.
	 */
	if (xp->dma) {
		const char *msg = NULL;

		if (uart_console(p)) {
			msg = "forbid DMA for kernel console";
		} else {
			if (xr_uart_request_dma(xp))
				msg = "failed to request DMA";
		}

		if (msg) {
			dev_warn_ratelimited(p->dev, "%s\n", msg);
			xp->dma = NULL;
		}
	}

	/*
	 * Set the IER shadow for rx interrupts but defer actual interrupt
	 * enable until after the FIFOs are enabled; otherwise, an already-
	 * active sender can swamp the interrupt handler with "too much work".
	 */
	xp->ier = DW_UART_IER_ELSI_MASK | DW_UART_IER_ERBFI_MASK;

	retval = 0;
out:
	xr_uart_rpm_put(xp);
	return retval;
}

static void xr_uart_shutdown(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);
	unsigned long flags;

	xr_uart_rpm_get(xp);
	/*
	 * Disable interrupts from this port
	 */
	spin_lock_irqsave(&p->lock, flags);
	xp->ier = 0;
	serial_port_out(p, DW_UART_IER, 0);
	spin_unlock_irqrestore(&p->lock, flags);

	synchronize_irq(p->irq);

	if (xp->dma)
		xr_uart_release_dma(xp);

	spin_lock_irqsave(&p->lock, flags);
	if (p->flags & UPF_FOURPORT) {
		/* reset interrupts on the AST Fourport board */
		inb((p->iobase & 0xfe0) | 0x1f);
		p->mctrl |= TIOCM_OUT1;
	} else {
		p->mctrl &= ~TIOCM_OUT2;
	}

	xr_uart_set_mctrl(p, p->mctrl);
	spin_unlock_irqrestore(&p->lock, flags);

	/*
	 * Disable break condition and FIFOs
	 */
	serial_port_out(p, DW_UART_LCR,
		serial_port_in(p, DW_UART_LCR) & ~DW_UART_LCR_BC_MASK);
	xr_uart_clear_fifos(xp);

	/*
	 * Read data port to reset things, and then unlink from
	 * the IRQ chain.
	 */
	serial_port_in(p, DW_UART_RBR);
	xr_uart_rpm_put(xp);

	xr_uart_release_irq(xp);
}

static int xr_uart_request_port(struct uart_port *p)
{
	unsigned int size = p->mapsize;
	int ret = 0;

	switch (p->iotype) {
	case UPIO_MEM32:
	case UPIO_MEM:
		if (!p->mapbase) {
			ret = -EINVAL;
			break;
		}

		if (!request_mem_region(p->mapbase, size, "serial")) {
			ret = -EBUSY;
			break;
		}

		if (p->flags & UPF_IOREMAP) {
			p->membase = ioremap(p->mapbase, size);
			if (!p->membase) {
				release_mem_region(p->mapbase, size);
				ret = -ENOMEM;
			}
		}
		break;

	default:
		dev_err(p->dev, "Invalid iotype %d\n", p->iotype);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static void xr_uart_release_port(struct uart_port *p)
{
	unsigned int size = p->mapsize;

	switch (p->iotype) {
	case UPIO_MEM32:
	case UPIO_MEM:
		if (!p->mapbase)
			break;

		if (p->flags & UPF_IOREMAP) {
			iounmap(p->membase);
			p->membase = NULL;
		}

		release_mem_region(p->mapbase, size);
		break;

	default:
		dev_err(p->dev, "Invalid iotype %d\n", p->iotype);
		break;
	}
}

static void autoconfig(struct xr_uart_port *xp)
{
	unsigned char status1, scratch, scratch2, scratch3;
	unsigned char save_lcr, save_mcr;
	struct uart_port *p = &xp->port;
	unsigned long flags;

	if (!p->iobase && !p->mapbase && !p->membase)
		return;

	/*
	 * We really do need global IRQs disabled here - we're going to
	 * be frobbing the chips IRQ enable register to see if it exists.
	 */
	spin_lock_irqsave(&p->lock, flags);

	if (!(p->flags & UPF_BUGGY_UART)) {
		/*
		 * Do a simple existence test first; if we fail this,
		 * there's no point trying anything else.
		 *
		 * 0x80 is used as a nonsense port to prevent against
		 * false positives due to ISA bus float.  The
		 * assumption is that 0x80 is a non-existent port;
		 * which should be safe since include/asm/io.h also
		 * makes this assumption.
		 *
		 * Note: this is safe as long as MCR bit 4 is clear
		 * and the device is in "PC" mode.
		 */
		scratch = serial_in(xp, DW_UART_IER);
		serial_out(xp, DW_UART_IER, 0);
		/*
		 * Mask out IER[7:4] bits for test as some UARTs (e.g. TL
		 * 16C754B) allow only to modify them if an EFR bit is set.
		 */
		scratch2 = serial_in(xp, DW_UART_IER) & DW_UART_IER_ALL_INTR_MASK;
		serial_out(xp, DW_UART_IER, 0x0F);
		scratch3 = serial_in(xp, DW_UART_IER) & DW_UART_IER_ALL_INTR_MASK;
		serial_out(xp, DW_UART_IER, scratch);
		if (scratch2 != 0 || scratch3 != 0x0F) {
			/*
			 * We failed; there's nothing here
			 */
			goto out_lock;
		}
	}

	save_mcr = serial_in(xp, DW_UART_MCR);
	save_lcr = serial_in(xp, DW_UART_LCR);

	/*
	 * Check to see if a UART is really there.  Certain broken
	 * internal modems based on the Rockwell chipset fail this
	 * test, because they apparently don't implement the loopback
	 * test mode.  So this test is skipped on the COM 1 through
	 * COM 4 ports.  This *should* be safe, since no board
	 * manufacturer would be stupid enough to design a board
	 * that conflicts with COM 1-4 --- we hope!
	 */
	if (!(p->flags & UPF_SKIP_TEST)) {
		serial_out(xp, DW_UART_MCR, DW_UART_MCR_LB_MASK |
						DW_UART_MCR_OUT2_MASK |
						DW_UART_MCR_RTS_MASK);
		status1 = serial_in(xp, DW_UART_MSR) & DW_UART_MSR_ANY_STATUS;
		serial_out(xp, DW_UART_MCR, save_mcr);
		if (status1 != (DW_UART_MSR_CTS_MASK | DW_UART_MSR_DCD_MASK))
			goto out_lock;
	}

	serial_out(xp, DW_UART_FCR, DW_UART_FCR_FIFOE_MASK);
	serial_out(xp, DW_UART_LCR, save_lcr);

	if (p->type == PORT_UNKNOWN)
		goto out_lock;

	/*
	 * Reset the UART.
	 */
	serial_out(xp, DW_UART_MCR, save_mcr);
	xr_uart_clear_fifos(xp);
	serial_in(xp, DW_UART_RBR);
	serial_out(xp, DW_UART_IER, 0);

out_lock:
	spin_unlock_irqrestore(&p->lock, flags);
}

static void autoconfig_irq(struct xr_uart_port *xp)
{
	struct uart_port *p = &xp->port;
	unsigned char save_mcr, save_ier;
	unsigned long irqs;
	int irq;

	if (uart_console(p))
		console_lock();

	/* forget possible initially masked and pending IRQ */
	probe_irq_off(probe_irq_on());
	save_mcr = serial_in(xp, DW_UART_MCR);
	save_ier = serial_in(xp, DW_UART_IER);
	serial_out(xp, DW_UART_MCR, DW_UART_MCR_OUT1_MASK | DW_UART_MCR_OUT2_MASK);

	irqs = probe_irq_on();
	serial_out(xp, DW_UART_MCR, 0);
	udelay(10);

	serial_out(xp, DW_UART_MCR, DW_UART_MCR_DTR_MASK |
				DW_UART_MCR_RTS_MASK | DW_UART_MCR_OUT2_MASK);
	serial_out(xp, DW_UART_IER, DW_UART_IER_ALL_INTR_MASK);
	serial_in(xp, DW_UART_LSR);
	serial_in(xp, DW_UART_RBR);
	serial_in(xp, DW_UART_IIR);
	serial_in(xp, DW_UART_MSR);
	/* dummy byte: 0xff */
	serial_out(xp, DW_UART_THR, 0xFF);
	udelay(20);
	irq = probe_irq_off(irqs);

	serial_out(xp, DW_UART_MCR, save_mcr);
	serial_out(xp, DW_UART_IER, save_ier);

	if (uart_console(p))
		console_unlock();

	p->irq = (irq > 0) ? irq : 0;
}

static int fcr_get_rxtrig_bytes(struct xr_uart_port *xp)
{
	const struct xr_uart_config *conf = &xr_uart_config;
	unsigned char bytes;

	bytes = conf->rxtrig_bytes[DW_UART_FCR_RT_BITS(xp->fcr)];

	if (bytes)
		return bytes;

	return -EOPNOTSUPP;
}

static int bytes_to_fcr_rxtrig(struct xr_uart_port *xp, unsigned char bytes)
{
	const struct xr_uart_config *conf = &xr_uart_config;
	int i;

	if (!conf->rxtrig_bytes[DW_UART_FCR_RT_BITS(DW_UART_FCR_RT_00)])
		return -EOPNOTSUPP;

	for (i = 1; i < DW_UART_FCR_RT_MAX_STATE; i++) {
		if (bytes < conf->rxtrig_bytes[i])
			/* Use the nearest lower value */
			return (--i) << DW_UART_FCR_RT_OFFSET;
	}

	return DW_UART_FCR_RT_11;
}

static int do_get_rxtrig(struct tty_port *port)
{
	struct uart_state *state = container_of(port, struct uart_state, port);
	struct uart_port *p = state->uart_port;
	struct xr_uart_port *xp = up_to_xr(p);

	if (p->fifosize <= 1)
		return -EINVAL;

	return fcr_get_rxtrig_bytes(xp);
}

static int do_xr_uart_get_rxtrig(struct tty_port *port)
{
	int rxtrig_bytes;

	mutex_lock(&port->mutex);
	rxtrig_bytes = do_get_rxtrig(port);
	mutex_unlock(&port->mutex);

	return rxtrig_bytes;
}

static ssize_t rx_trig_bytes_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tty_port *port = dev_get_drvdata(dev);
	int rxtrig_bytes;

	rxtrig_bytes = do_xr_uart_get_rxtrig(port);
	if (rxtrig_bytes < 0)
		return rxtrig_bytes;

	return snprintf(buf, PAGE_SIZE, "%d\n", rxtrig_bytes);
}

static int do_set_rxtrig(struct tty_port *port, unsigned char bytes)
{
	struct uart_state *state = container_of(port, struct uart_state, port);
	struct uart_port *p = state->uart_port;
	struct xr_uart_port *xp = up_to_xr(p);
	int rxtrig;

	if (p->fifosize <= 1)
		return -EINVAL;

	rxtrig = bytes_to_fcr_rxtrig(xp, bytes);
	if (rxtrig < 0)
		return rxtrig;

	xr_uart_clear_fifos(xp);
	xp->fcr &= ~DW_UART_FCR_RT_MASK;
	xp->fcr |= (unsigned char)rxtrig;
	serial_out(xp, DW_UART_FCR, xp->fcr);

	return 0;
}

static int do_xr_uart_set_rxtrig(struct tty_port *port, unsigned char bytes)
{
	int ret;

	mutex_lock(&port->mutex);
	ret = do_set_rxtrig(port, bytes);
	mutex_unlock(&port->mutex);

	return ret;
}

static ssize_t rx_trig_bytes_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tty_port *port = dev_get_drvdata(dev);
	unsigned char bytes;
	int ret;

	if (!count)
		return -EINVAL;

	/* base 10 */
	ret = kstrtou8(buf, 10, &bytes);
	if (ret < 0)
		return ret;

	ret = do_xr_uart_set_rxtrig(port, bytes);
	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR_RW(rx_trig_bytes);

static struct attribute *xr_uart_dev_attrs[] = {
	&dev_attr_rx_trig_bytes.attr,
	NULL
};

static struct attribute_group xr_uart_dev_attr_group = {
	.attrs = xr_uart_dev_attrs,
};

static void register_dev_spec_attr_grp(struct xr_uart_port *xp)
{
	const struct xr_uart_config *conf = &xr_uart_config;

	if (conf->rxtrig_bytes[0])
		xp->port.attr_group = &xr_uart_dev_attr_group;
}

static void xr_uart_config_port(struct uart_port *p, int flags)
{
	struct xr_uart_port *xp = up_to_xr(p);

	if (flags & UART_CONFIG_TYPE)
		autoconfig(xp);

	if (p->type != PORT_UNKNOWN && flags & UART_CONFIG_IRQ)
		autoconfig_irq(xp);

	if (p->type == PORT_UNKNOWN)
		xr_uart_release_port(p);

	register_dev_spec_attr_grp(xp);
	xp->fcr = xr_uart_config.fcr;
	if (xp->need_tx_dma || xp->need_rx_dma)
		xp->fcr = xr_uart_config.fcr_dma;
}

static int xr_uart_verify_port(struct uart_port *p, struct serial_struct *ser)
{
	if (ser->irq >= nr_irqs || ser->irq < 0 ||
		ser->baud_base < MIN_SUPPORT_BAUD_BASE ||
		ser->type < PORT_UNKNOWN || ser->type == PORT_CIRRUS ||
		ser->type == PORT_STARTECH)
		return -EINVAL;
	return 0;
}

#ifdef CONFIG_CONSOLE_POLL
/*
 * Console polling routines for writing and reading from the uart while
 * in an interrupt or debug context.
 */

static int xr_uart_get_poll_char(struct uart_port *p)
{
	struct xr_uart_port *xp = up_to_xr(p);
	unsigned char lsr;
	int status;

	xr_uart_rpm_get(xp);

	lsr = serial_port_in(p, DW_UART_LSR);

	if (!(lsr & DW_UART_LSR_DR_MASK)) {
		status = NO_POLL_CHAR;
		goto out;
	}

	status = serial_port_in(p, DW_UART_RBR);
out:
	xr_uart_rpm_put(xp);
	return status;
}

static void xr_uart_put_poll_char(struct uart_port *p, unsigned char c)
{
	unsigned int ier;
	struct xr_uart_port *xp = up_to_xr(p);

	xr_uart_rpm_get(xp);
	/*
	 *	First save the IER then disable the interrupts
	 */
	ier = serial_port_in(p, DW_UART_IER);
	serial_port_out(p, DW_UART_IER, 0);

	wait_for_xmitr(xp, LSR_BOTH_EMPTY);
	/*
	 *	Send the character out.
	 */
	serial_port_out(p, DW_UART_THR, c);

	/*
	 *	Finally, wait for transmitter to become empty
	 *	and restore the IER
	 */
	wait_for_xmitr(xp, LSR_BOTH_EMPTY);
	serial_port_out(p, DW_UART_IER, ier);
	xr_uart_rpm_put(xp);
}
#endif /* CONFIG_CONSOLE_POLL */

static void xr_uart_do_pm(struct uart_port *p, unsigned int state,
				unsigned int oldstate)
{
	struct xr_uart_port *xp = up_to_xr(p);

	if (!state)
		if (!xr_uart_runtime_resume(p->dev))
			pm_runtime_get_sync(p->dev);

	xr_uart_set_sleep(xp, state != 0);

	if (state)
		if (!pm_runtime_put_sync_suspend(p->dev))
			xr_uart_runtime_suspend(p->dev);
}

static const char *xr_uart_port_type(struct uart_port *p)
{
	return "dw-apb-uart";
}

static const struct uart_ops xr_uart_pops = {
	.tx_empty	= xr_uart_tx_empty,
	.set_mctrl	= xr_uart_set_mctrl,
	.get_mctrl	= xr_uart_get_mctrl,
	.stop_tx	= xr_uart_stop_tx,
	.start_tx	= xr_uart_start_tx,
	.throttle	= xr_uart_throttle,
	.unthrottle	= xr_uart_unthrottle,
	.stop_rx	= xr_uart_stop_rx,
	.enable_ms	= xr_uart_enable_ms,
	.break_ctl	= xr_uart_break_ctl,
	.startup	= xr_uart_startup,
	.shutdown	= xr_uart_shutdown,
	.set_termios	= xr_uart_set_termios,
	.set_ldisc	= xr_uart_set_ldisc,
	.pm		= xr_uart_do_pm,
	.type		= xr_uart_port_type,
	.release_port	= xr_uart_release_port,
	.request_port	= xr_uart_request_port,
	.config_port	= xr_uart_config_port,
	.verify_port	= xr_uart_verify_port,
#ifdef CONFIG_CONSOLE_POLL
	.poll_get_char = xr_uart_get_poll_char,
	.poll_put_char = xr_uart_put_poll_char,
#endif
};

static int xr_uart_register_port(struct xr_uart_port *xp)
{
	int ret = -ENOSPC;

	xp->port.flags |= UPF_BOOT_AUTOCONF;
	if (xp->port.fifosize && !xp->tx_loadsz)
		xp->tx_loadsz = xp->port.fifosize;

	ret = uart_add_one_port(&xr_uart_driver, &xp->port);
	if (ret == 0)
		ret = xp->port.line;

	return ret;
}

void xr_uart_unregister_port(int line)
{
	struct xr_uart_port *xp = &xr_uart_ports[line];

	uart_remove_one_port(&xr_uart_driver, &xp->port);
	xp->port.dev = NULL;
}

#ifdef CONFIG_XRING_UART_CONSOLE_SUPPORT
static struct console xr_console = {
	.name = "ttyXR",
	.write = xr_uart_console_write,
	.device = uart_console_device,
	.setup = xr_uart_console_setup,
	.exit = xr_uart_console_exit,
	.flags = CON_PRINTBUFFER,
	.index = -1,
	.data = &xr_uart_driver,
};

static struct uart_driver xr_uart_driver = {
	.owner			= THIS_MODULE,
	.driver_name		= "xr_uart",
	.dev_name		= "ttyXR",
	.cons			= &xr_console,
};

#define XR_CONSOLE	(&xr_console)

#else
#define XR_CONSOLE	NULL
#endif

static int xr_uart_probe(struct platform_device *pdev)
{
	struct resource *regs;
	struct xr_uart_data *data;
	struct xr_uart_port *xp;
	struct uart_port *p;
	struct device *dev = &pdev->dev;
	int irq;
	int err;
	int line;
	const char *clk_name = NULL;
	const char *pclk_name = NULL;
	struct device_node *plat_np = NULL;

	if (pdev->dev.of_node)
		line = of_alias_get_id(pdev->dev.of_node, "serial");
	else
		line = pdev->id;

	if (line < 0)
		line = atomic_inc_return(&xr_uart_next_id) - 1;

	if (unlikely(line < 0 || line >= UART_NR))
		return -ENXIO;

	dev_info(dev, "detected port #%d of %d\n", line, UART_NR);

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs) {
		dev_err(dev, "reg nout found\n");
		return -EINVAL;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	xp = xr_uart_get_port(line);
	p = &xp->port;

	spin_lock_init(&p->lock);
	p->name		= "xring_uart";
	p->line		= line;
	p->mapbase	= regs->start;
	p->irq		= irq;
	p->handle_irq	= xr_uart_handle_irq;
	p->pm		= xr_uart_do_pm;
	p->type		= PORT_8250;
	p->flags	= UPF_SHARE_IRQ | UPF_FIXED_PORT;
	p->dev		= dev;
	p->iotype	= UPIO_MEM32;
	p->serial_in	= xr_uart_serial_in;
	p->serial_out	= xr_uart_serial_out;
	p->set_ldisc	= xr_uart_set_ldisc;
	p->set_termios	= xr_uart_set_termios;
	p->set_divisor = xr_uart_set_divisor;
	p->ops = &xr_uart_pops;
	p->has_sysrq = IS_ENABLED(CONFIG_XRING_UART_CONSOLE_SUPPORT);

	p->membase = devm_ioremap(dev, regs->start, resource_size(regs));
	if (!p->membase)
		return -ENOMEM;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->dma.fn = xr_uart_fallback_dma_filter;
	data->usr_reg = DW_UART_USR;
	p->private_data = data;

	if (!xr_uart_driver.state) {
		xr_uart_driver.nr = UART_NR;
		err = uart_register_driver(&xr_uart_driver);
		if (err < 0) {
			dev_err(dev, " Failed to register Xring-UART driver\n");
			return err;
		}
	}

	err = of_property_read_string_index(
						(&pdev->dev)->of_node,
						"clock-names",
						0,
						&pclk_name);
	if (err < 0) {
		dev_err(&pdev->dev, "clock_name get failed.\n");
		return err;
	}

	err = of_property_read_string_index(
						(&pdev->dev)->of_node,
						"clock-names",
						1,
						&clk_name);
	if (err < 0) {
		dev_err(&pdev->dev, "pclk_name get failed.\n");
		return err;
	}

	data->clk = devm_clk_get_optional(&pdev->dev, clk_name);
	if (IS_ERR(data->clk))
		return PTR_ERR(data->clk);

	err = clk_prepare_enable(data->clk);
	if (err)
		dev_warn(dev, "could not enable optional baudclk: %d\n", err);

	/* if fpga runs, sw_ack and pll lock check feature will be shut down */
	plat_np = of_find_node_by_name(NULL, "fpga-dumyclk");
	if (plat_np) {
		err = device_property_read_u32(dev, "clock-frequency", &p->uartclk);
		if (err < 0) {
			dev_err(dev, "clock-frequency not defined\n");
			goto err_clk;
		}
	} else {
		p->uartclk = (unsigned int)clk_get_rate(data->clk);
	}
	/* If no clock rate is defined, fail. */
	if (!p->uartclk) {
		dev_err(dev, "clock rate not defined\n");
		err = -EINVAL;
		goto err_clk;
	}

	data->pclk = devm_clk_get_optional(&pdev->dev, pclk_name);
	if (IS_ERR(data->pclk)) {
		err = PTR_ERR(data->pclk);
		goto err_clk;
	}

	err = clk_prepare_enable(data->pclk);
	if (err) {
		dev_err(dev, "could not enable apb_pclk\n");
		goto err_clk;
	}

	data->rst = devm_reset_control_get_optional_exclusive(dev, NULL);
	if (IS_ERR(data->rst)) {
		err = PTR_ERR(data->rst);
		goto err_pclk;
	}
	err = reset_control_deassert(data->rst);
	if (err < 0) {
		dev_err(dev, "Couldn't deassert rst control: %d\n", err);
		goto err_pclk;
	}

	data->clk_count = 1;
	data->line = line;

	if (line == 1) {
		sys_state_doz2nor_vote(VOTER_DOZAP_UART6);
#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
		xp->hwlock = hwspin_lock_request_specific(AP_LPIS_SPIN_LOCK_ID_1_1);
		if (!xp->hwlock)
			goto err_reset;
#endif
	}

	xr_uart_param_parse(p);

#ifdef CONFIG_XRING_UART_CONSOLE_SUPPORT
	xr_uart_probe_console_enable(dev, xp, xr_console.name);
#endif

	/* Try to enable DMA */
	xp->need_tx_dma = device_property_read_bool(dev, "uart-tx-dma-enable") ? 1 : 0;
	xp->need_rx_dma = device_property_read_bool(dev, "uart-rx-dma-enable") ? 1 : 0;
	if ((xp->need_tx_dma || xp->need_rx_dma) && p->fifosize) {
		if (xp->need_tx_dma) {
			data->dma.txconf.dst_maxburst = p->fifosize / 4;
			data->dma.tx_dma = xr_uart_tx_dma;
		}

		if (xp->need_rx_dma) {
			data->dma.rxconf.src_maxburst = p->fifosize / 4;
			data->dma.rx_dma = xr_uart_rx_dma;
		}

		xp->dma = &data->dma;
		dev_info(dev, "DMA enabled\n");
	}

	platform_set_drvdata(pdev, data);

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	err = xr_uart_register_port(xp);
	if (err < 0) {
		dev_err(dev, "Register port failed\n");
		goto err_reset;
	}

	dev_info(p->dev, "Xring UART probed OK\n");
	return 0;

err_reset:
	reset_control_assert(data->rst);

err_pclk:
	clk_disable_unprepare(data->pclk);

err_clk:
	clk_disable_unprepare(data->clk);
	uart_unregister_driver(&xr_uart_driver);

	return err;
}

static int xr_uart_remove(struct platform_device *pdev)
{
	struct xr_uart_data *data = platform_get_drvdata(pdev);
	struct device *dev = &pdev->dev;
#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
	struct xr_uart_port *xp = xr_uart_get_port(data->line);
#endif

	pm_runtime_get_sync(dev);

	xr_uart_unregister_port(data->line);

	reset_control_assert(data->rst);

	clk_disable_unprepare(data->pclk);

	clk_disable_unprepare(data->clk);

#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
	hwspin_lock_free(xp->hwlock);
#endif

	pm_runtime_disable(dev);
	pm_runtime_put_noidle(dev);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static void xr_uart_suspend_port(int line)
{
	struct xr_uart_port *xp = &xr_uart_ports[line];
	struct uart_port *p = &xp->port;

	if (!console_suspend_enabled && uart_console(p) &&
		p->type != PORT_8250) {
		unsigned char canary = SUSPEND_FLAG;

		serial_out(xp, DW_UART_SCR, canary);
		if (serial_in(xp, DW_UART_SCR) == canary)
			xp->canary = canary;
	}

	uart_suspend_port(&xr_uart_driver, p);
}

static int xr_uart_suspend(struct device *dev)
{
	struct xr_uart_data *data = dev_get_drvdata(dev);

	xr_uart_suspend_port(data->line);

	dev_info(dev, "%s --\n", __func__);

	return 0;
}

static void xr_uart_resume_port(int line)
{
	struct xr_uart_port *xp = &xr_uart_ports[line];
	struct uart_port *p = &xp->port;

	xp->canary = 0;

	uart_resume_port(&xr_uart_driver, p);
}

static int xr_uart_resume(struct device *dev)
{
	struct xr_uart_data *data = dev_get_drvdata(dev);
	int ret = 0;

	if (data->line != 1) {
		ret = reset_control_deassert(data->rst);
		if (ret) {
			dev_err(dev, "unable to reset_control_deassert\n");
			return ret;
		}
	}

	xr_uart_resume_port(data->line);
	dev_info(dev, "%s --\n", __func__);

	return 0;
}
#endif /* CONFIG_PM_SLEEP */

static const struct dev_pm_ops xr_uart_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(xr_uart_suspend, xr_uart_resume)
	SET_RUNTIME_PM_OPS(xr_uart_runtime_suspend, xr_uart_runtime_resume, NULL)
};

static const struct of_device_id xr_uart_of_match[] = {
	{ .compatible = "xring,dw-apb-uart",},
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, xr_uart_of_match);

static struct platform_driver xr_uart_platform_driver = {
	.driver = {
		.name		= "xring-dw-apb-uart",
		.pm		= &xr_uart_pm_ops,
		.of_match_table	= xr_uart_of_match,
	},
	.probe			= xr_uart_probe,
	.remove			= xr_uart_remove,
};

module_platform_driver(xr_uart_platform_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Synopsys DesignWare serial port driver by Xring");

#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
MODULE_SOFTDEP("pre: xring_hwspinlock");
#endif
