// SPDX-License-Identifier: GPL-2.0-only
/*
 * Synopsys DesignWare apb-uart driver.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/console.h>
#include <linux/debugfs.h>
#include <linux/of.h>
#include <linux/pm_runtime.h>
#include <linux/kfifo.h>
#include <linux/serial.h>
#include <linux/delay.h>
#include <linux/nmi.h>
#include <linux/version.h>
#include <dt-bindings/xring/platform-specific/hwlock/ap_spin_lock_id.h>

#include "xring_uart.h"

#ifdef CONFIG_XRING_UART_CONSOLE_SUPPORT

#define TX_WORK_TIMEOUT_MS 1600000
#define TX_LOCAL_BUF_SIZE 32
#define UART_TX_BUF_SIZE_DEF 0x20000 /* 256k */
#define UART_TX_CPUON_DEF 3 /* default cpu id */
#define UART_TX_CPUON_NOTSET (-1)
#define MAX_CONSOLE_UART_NAME_LEN 8
#define MAX_DBG_FS_NAME_LEN 16

static void xr_console_putchar(struct uart_port *p, unsigned char ch)
{
	struct xr_uart_port *xp = up_to_xr(p);

	wait_for_xmitr(xp, DW_UART_LSR_THRE_MASK);
	serial_port_out(p, DW_UART_THR, ch);
}

/*
 *	Restore serial console when h/w power-off detected
 */
static void xr_console_restore(struct xr_uart_port *xp)
{
	struct uart_port *p = &xp->port;
	struct ktermios termios;
	unsigned int baud, quot, frac = 0;

	termios.c_cflag = p->cons->cflag;
	if (p->state->port.tty && termios.c_cflag == 0)
		termios.c_cflag = p->state->port.tty->termios.c_cflag;

	baud = xr_uart_get_baud_rate(p, &termios, NULL);
	quot = dwuart_get_divisor(p, baud, &frac);

	dwuart_set_divisor(p, baud, quot, frac);
	serial_port_out(p, DW_UART_LCR, xp->lcr);
	serial_port_out(p, DW_UART_MCR,
			DW_UART_MCR_DTR_MASK | DW_UART_MCR_RTS_MASK);
}

static void xr_uart_console_write_sync(struct console *co, const char *s,
				unsigned int count)
{
	struct xr_uart_port *xp = xr_uart_get_port(co->index);
	struct uart_port *p = &xp->port;
	unsigned long flags;
	unsigned int ier;
	int locked = 1;
#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
	struct hwspinlock *hwlock = xp->hwlock;
	int ret = 0;
#endif

	touch_nmi_watchdog();

	if (oops_in_progress)
		locked = spin_trylock_irqsave(&p->lock, flags);
	else
		spin_lock_irqsave(&p->lock, flags);

#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
lock_retry:
	ret = hwspin_lock_timeout(hwlock, 2);
	if (ret)
		goto lock_retry;
#endif

	/*
	 * First save the IER then disable the interrupts
	 */
	ier = serial_port_in(p, DW_UART_IER);
	serial_port_out(p, DW_UART_IER, 0);

	/* check scratch reg to see if port powered off during system sleep */
	if (xp->canary && (xp->canary != serial_port_in(p, DW_UART_SCR))) {
		xr_console_restore(xp);
		xp->canary = 0;
	}

	uart_console_write(p, s, count, xr_console_putchar);

	/*
	 * Finally, wait for transmitter to become empty
	 * and restore the IER
	 */
	wait_for_xmitr(xp, LSR_BOTH_EMPTY);

	serial_port_out(p, DW_UART_IER, ier);

	/*
	 * The receive handling will happen properly because the
	 * receive ready bit will still be set; it is not cleared
	 * on read.  However, modem control will not, we must
	 * call it if we have saved something in the saved flags
	 * while processing with interrupts off.
	 */
	if (xp->msr_saved_flags)
		xr_uart_modem_status(xp);

	/* release the lock */
#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
	hwspin_unlock(hwlock);
#endif

	if (locked)
		spin_unlock_irqrestore(&p->lock, flags);
}

static void xr_uart_tx_work(struct work_struct *work)
{
	struct uart_tx_unit *unit = container_of(work, struct uart_tx_unit, console_delay_wq.work);
	struct xr_uart_port *xp = container_of(unit, struct xr_uart_port, tx_unit);
	static char local_static_buf[TX_LOCAL_BUF_SIZE] = "";
	unsigned int out_len;
	unsigned int pos;
	unsigned long flags;
#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
	int ret;
	struct hwspinlock *hwlock = xp->hwlock;
#endif

print_retry:
	unit->tx_uart_tasklet_run++;
	wait_for_xmitr(xp, LSR_BOTH_EMPTY);

	/* fifo out and send */
	spin_lock_irqsave(&unit->tx_lock_in, flags);
	out_len = kfifo_out(&unit->tx_fifo, local_static_buf,
				TX_LOCAL_BUF_SIZE);
	spin_unlock_irqrestore(&unit->tx_lock_in, flags);

	if (out_len == 0) {
		queue_delayed_work(unit->console_wq, &unit->console_delay_wq,
				msecs_to_jiffies(50));
		return;
	}

#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
	ret = hwspin_lock_timeout_irqsave(hwlock, 2, &flags);
	if (ret)
		goto print_retry;
#endif

	unit->tx_out += out_len;
	for (pos = 0; pos < out_len; pos++) {
		if ((local_static_buf[pos] == '\n') && pos &&
			(local_static_buf[pos - 1] != '\r'))
			xr_console_putchar(&xp->port, '\r');

		xr_console_putchar(&xp->port, local_static_buf[pos]);
		unit->tx_sent++;
	}

	/* release the lock */
#if IS_ENABLED(CONFIG_XRING_UART_HWLOCK)
	hwspin_unlock_irqrestore(hwlock, &flags);
#endif
	if (kfifo_len(&unit->tx_fifo) > 0)
		goto print_retry;

	queue_delayed_work(unit->console_wq, &unit->console_delay_wq,
				msecs_to_jiffies(20));
}

static void xr_uart_tx_unit_init(struct uart_tx_unit *unit, int cpuid)
{
	unit->tx_got = 0;
	unit->tx_uart_fifo_full = 0;
	unit->tx_uart_fifo_full_lost = 0;
	unit->tx_sent = 0;
	unit->tx_out = 0;
	unit->tx_queued = 0;
	unit->tx_valid = 1;
	unit->tx_cpuid = cpuid;
	unit->max_cpus = num_possible_cpus();
	unit->tx_buf_times = 0;
	unit->tx_buf_max = 0;
	unit->tx_buf_over_cnt = 0;
	unit->tx_buf_over_size = 0;
	unit->tx_uart_tasklet_run = 0;
}

#ifdef CONFIG_XRING_UART_DEBUG_FS
int xr_uart_tx_stat_show(struct seq_file *s, void *arg)
{
	unsigned int uart_tx_buf_size;
	struct uart_tx_unit *unit = (struct uart_tx_unit *)s->private;

	uart_tx_buf_size = kfifo_size(&unit->tx_fifo);
	seq_puts(s, "console transfer statics:\n");
	seq_printf(s, "tx fifo enable: %d\n", unit->tx_valid);
	seq_printf(s, "fifo over cnt : %lu\n", unit->tx_buf_over_cnt);
	seq_printf(s, "fifo over len : %lu\n", unit->tx_buf_over_size);
	seq_printf(s, "fifo capacity : 0x%lx(%luk)\n",
		(unsigned long)uart_tx_buf_size,
		(unsigned long)uart_tx_buf_size / 1024); /* convert to K byte */
	seq_printf(s, "fifo required : 0x%lx(%luk)\n", unit->tx_buf_max,
		unit->tx_buf_max / 1024); /* convert to K byte */
	/* print fifo useage percent */
	seq_printf(s, "fifo useage : %lu%%\n",
		(unit->tx_buf_max * 100) / uart_tx_buf_size);

	/* print fifo overflow percent */
	if (unit->tx_buf_max > uart_tx_buf_size)
		seq_printf(s, "overflow : %lu%% ~ %lu%%\n",
			((unit->tx_buf_max - uart_tx_buf_size) * 100) /
			uart_tx_buf_size,
			((unit->tx_buf_max - uart_tx_buf_size) * 100) /
			uart_tx_buf_size + 1);

	seq_printf(s, "printk cnt    : %lu\n", unit->tx_buf_times);
	seq_printf(s, "printk data   : %lu\n", unit->tx_got);
	seq_printf(s, "fifo in       : %lu\n", unit->tx_queued);
	seq_printf(s, "fifo out      : %lu\n", unit->tx_out);
	seq_printf(s, "uart send     : %lu\n", unit->tx_sent);
	seq_printf(s, "tasklet run   : %lu\n", unit->tx_uart_tasklet_run);
	if (unit->tx_uart_tasklet_run)
		seq_printf(s, "p_cnt/t_run   : %lu:1\n",
			unit->tx_buf_times / unit->tx_uart_tasklet_run);

	return 0;
}

int xr_uart_tx_stat_open(struct inode *inode, struct file *file)
{
	return single_open(file, xr_uart_tx_stat_show, inode->i_private);
}

const struct file_operations xr_uart_tx_stat_ops = {
	.open = xr_uart_tx_stat_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

static int xr_uart_tx_work_init(struct xr_uart_port *xp,
	unsigned int uart_tx_buf_size, int cpuid)
{
	struct uart_tx_unit *unit = &xp->tx_unit;
	int ret;
#ifdef CONFIG_XRING_UART_DEBUG_FS
	char dbgfs_name[MAX_DBG_FS_NAME_LEN] = {'\0'};
#endif

	ret = kfifo_alloc(&unit->tx_fifo, uart_tx_buf_size, GFP_KERNEL | __GFP_ZERO);
	if (ret) {
		unit->tx_valid = 0;
		return ret;
	}

	spin_lock_init(&unit->tx_lock_in);

	INIT_DELAYED_WORK(&unit->console_delay_wq, xr_uart_tx_work);
	unit->console_wq = alloc_workqueue("console_wq", WQ_MEM_RECLAIM | WQ_HIGHPRI, 1);

	xr_uart_tx_unit_init(unit, cpuid);

	queue_delayed_work(unit->console_wq, &unit->console_delay_wq,
				msecs_to_jiffies(20));

#ifdef CONFIG_XRING_UART_DEBUG_FS
	ret = snprintf(dbgfs_name, sizeof(dbgfs_name), "uart%u_stat", xp->port.line);
	if (ret < 0) {
		dev_err(xp->port.dev, "gen dbgfs_name fail %d\n", ret);
		return ret;
	}

	/* 0444 for file read-only */
	debugfs_create_file(dbgfs_name, 0444, NULL, unit, &xr_uart_tx_stat_ops);
#endif
	dev_dbg(xp->port.dev, "tx_work init ok, max cpu=%d\n", unit->max_cpus);

	return 0;
}

static void xr_uart_console_write_async(
	struct console *co, const char *s, unsigned int count)
{
	struct xr_uart_port *xp = xr_uart_get_port(co->index);
	struct uart_tx_unit *unit = &xp->tx_unit;
	unsigned int copyed_len;
	unsigned int fifo_len;
	unsigned long flag;

	spin_lock_irqsave(&unit->tx_lock_in, flag);
	copyed_len = kfifo_in(&unit->tx_fifo, s, count);
	fifo_len = kfifo_len(&unit->tx_fifo);
	unit->tx_got += count;
	unit->tx_buf_times++;
	unit->tx_queued += copyed_len;
	if (copyed_len < count) {
		/* fifo over-flow */
		unit->tx_buf_over_cnt++;
		unit->tx_buf_over_size += (count - copyed_len);
		unit->tx_buf_max = max(unit->tx_buf_max,
			(unsigned long)(fifo_len + count - copyed_len));
	} else {
		unit->tx_buf_max =
			max(unit->tx_buf_max, (unsigned long)fifo_len);
	}

	spin_unlock_irqrestore(&unit->tx_lock_in, flag);
}

static int xr_uart_prepare_fifo_cfg(struct device *dev, struct xr_uart_port *up,
					u32 *console_fifo_size, int *console_fifo_cpuon)
{
	/* get console fifo enable flag */
	if (of_property_read_u32(dev->of_node, "console-fifo-enable",
		&up->tx_unit.tx_valid)) {
		up->tx_unit.tx_valid = 0;
		dev_dbg(dev, "console-fifo-enable is not present!\n");
		return -EINVAL;
	}

	if (!up->tx_unit.tx_valid) {
		dev_dbg(dev, "disable serial console fifo!\n");
		return -EINVAL;
	}

	/* get console fifo size */
	if (of_property_read_u32(dev->of_node, "console-fifo-size",
					console_fifo_size)) {
		*console_fifo_size = UART_TX_BUF_SIZE_DEF;
		dev_dbg(dev, "console-fifo-size not exist! set %d as default\n",
			UART_TX_BUF_SIZE_DEF);
	}

	/* get console fifo cpuon */
	if (of_property_read_u32(dev->of_node,
			"console-fifo-cpuid", (u32 *)console_fifo_cpuon)) {
		*console_fifo_cpuon = UART_TX_CPUON_NOTSET;
		dev_dbg(dev, "console-fifo-cpuid not exist!\n");
	}

	return 0;
}



void xr_uart_probe_console_enable(struct device *dev,
	struct xr_uart_port *xp, const char *xr_console_name)
{
	u32 console_fifo_size = 0;
	int console_fifo_cpuon = 0;

	if (xr_uart_prepare_fifo_cfg(dev, xp, &console_fifo_size,
		&console_fifo_cpuon)) {
		return;
	}

	dev_info(dev, "uart async tx = [%d]\n", xp->tx_unit.tx_valid);
	if (xr_uart_tx_work_init(xp, console_fifo_size, console_fifo_cpuon)) {
		dev_err(dev, "tx work init fail\n");
		return;
	}
}

void xr_uart_console_write(struct console *co, const char *s, unsigned int count)
{
	struct xr_uart_port *xp = xr_uart_get_port(co->index);

	if (xp->tx_unit.tx_valid)
		xr_uart_console_write_async(co, s, count);
	else
		xr_uart_console_write_sync(co, s, count);
}

int xr_uart_console_setup(struct console *co, char *options)
{
	struct xr_uart_port *xp = xr_uart_get_port(co->index);
	struct uart_port *p = &xp->port;
	int baud = 460800;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';
	int ret;

	if (!p->membase)
		return -ENODEV;

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	ret = uart_set_options(p, co, baud, parity, bits, flow);
	if (ret) {
		p->cons = NULL;
		return ret;
	}

	/* link port to console */
	p->cons = co;

	if (p->dev)
		pm_runtime_get_sync(p->dev);

	return ret;
}

int xr_uart_console_exit(struct console *co)
{
	struct xr_uart_port *xp = xr_uart_get_port(co->index);

	if (xp->port.dev)
		pm_runtime_put_sync(xp->port.dev);

	return 0;
}

#endif /* CONFIG_XRING_UART_CONSOLE_SUPPORT */

