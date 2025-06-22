// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * Copyright (c) 2018 Synopsys, Inc. and/or its affiliates.
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 *
 * Author: Vitor Soares <vitor.soares@synopsys.com>
 */

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/i3c/master.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/iopoll.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/slab.h>

#include "dw-i3c-master.h"

#define NBYTE				4
#define NBYTE_MASK			3
#define NRX_WORDS_DIV_BASE		4

#define DEQUEUE_XFER_DELAY_10US		10	/* 10us */
#define DEQUEUE_XFER_TIMEOUT		1000000	/* 1000000us */

#define XR_IBI_DATA_THLD		QUEUE_THLD_CTRL_IBI_DATA(31)
#define XR_IBI_STATUS_THLD		QUEUE_THLD_CTRL_IBI_STAT(1)

#define EVEN_PARITY_VAL			0x9669

static u8 even_parity(u8 p)
{
	p ^= p >> 4;
	p &= 0xf;

	return (EVEN_PARITY_VAL >> p) & 1;
}

static bool dw_i3c_master_supports_ccc_cmd(struct i3c_master_controller *m,
					const struct i3c_ccc_cmd *cmd)
{
	if (cmd->ndests > 1)
		return false;

	switch (cmd->id) {
	case I3C_CCC_ENEC(true):
	case I3C_CCC_ENEC(false):
	case I3C_CCC_DISEC(true):
	case I3C_CCC_DISEC(false):
	case I3C_CCC_ENTAS(0, true):
	case I3C_CCC_ENTAS(0, false):
	case I3C_CCC_RSTDAA(true):
	case I3C_CCC_RSTDAA(false):
	case I3C_CCC_ENTDAA:
	case I3C_CCC_SETMWL(true):
	case I3C_CCC_SETMWL(false):
	case I3C_CCC_SETMRL(true):
	case I3C_CCC_SETMRL(false):
	case I3C_CCC_ENTHDR(0):
	case I3C_CCC_SETDASA:
	case I3C_CCC_SETNEWDA:
	case I3C_CCC_GETMWL:
	case I3C_CCC_GETMRL:
	case I3C_CCC_GETPID:
	case I3C_CCC_GETBCR:
	case I3C_CCC_GETDCR:
	case I3C_CCC_GETSTATUS:
	case I3C_CCC_GETMXDS:
	case I3C_CCC_GETHDRCAP:
		return true;
	default:
		return false;
	}
}

static inline struct dw_i3c_master *to_dw_i3c_master(struct i3c_master_controller *master)
{
	return container_of(master, struct dw_i3c_master, base);
}

static void dw_i3c_master_disable(struct dw_i3c_master *master)
{
	writel(readl(master->regs + DEVICE_CTRL) & ~DEV_CTRL_ENABLE,
		master->regs + DEVICE_CTRL);
}

static void dw_i3c_master_enable(struct dw_i3c_master *master)
{
	writel(readl(master->regs + DEVICE_CTRL) | DEV_CTRL_ENABLE,
		master->regs + DEVICE_CTRL);
}

static int dw_i3c_master_get_addr_pos(struct dw_i3c_master *master, u8 addr)
{
	int pos;

	for (pos = 0; pos < master->maxdevs; pos++) {
		if (addr == master->devs[pos].addr)
			return pos;
	}

	return -EINVAL;
}

static int dw_i3c_master_get_free_pos(struct dw_i3c_master *master)
{
	if (!(master->free_pos & GENMASK(master->maxdevs - 1, 0)))
		return -ENOSPC;

	return ffs(master->free_pos) - 1;
}

static void dw_i3c_master_wr_tx_fifo(struct dw_i3c_master *master,
					const u8 *bytes, int nbytes)
{
	u32 tmp = 0;

	writesl(master->regs + RX_TX_DATA_PORT, bytes, nbytes / NBYTE);
	if (nbytes & NBYTE_MASK) {
		memcpy(&tmp, bytes + (nbytes & ~NBYTE_MASK), nbytes & NBYTE_MASK);
		writesl(master->regs + RX_TX_DATA_PORT, &tmp, 1);
	}
}

static void dw_i3c_master_read_fifo(struct dw_i3c_master *master, int reg, u8 *bytes, int nbytes)
{
	u32 tmp;

	readsl(master->regs + reg, bytes, nbytes / NBYTE);
	if (nbytes & NBYTE_MASK) {
		readsl(master->regs + reg, &tmp, 1);
		memcpy(bytes + (nbytes & ~NBYTE_MASK), &tmp, nbytes & NBYTE_MASK);
	}
}

static void dw_i3c_master_read_rx_fifo(struct dw_i3c_master *master, u8 *bytes, int nbytes)
{
	return dw_i3c_master_read_fifo(master, RX_TX_DATA_PORT, bytes, nbytes);
}

static void dw_i3c_master_read_ibi_fifo(struct dw_i3c_master *master,
					u8 *bytes, int nbytes)
{
	return dw_i3c_master_read_fifo(master, IBI_QUEUE_STATUS, bytes, nbytes);
}

static struct dw_i3c_xfer *
dw_i3c_master_alloc_xfer(struct dw_i3c_master *master, unsigned int ncmds)
{
	struct dw_i3c_xfer *xfer = NULL;

	xfer = kzalloc(struct_size(xfer, cmds, ncmds), GFP_KERNEL);
	if (!xfer)
		return NULL;

	INIT_LIST_HEAD(&xfer->node);
	xfer->ncmds = ncmds;
	xfer->ret = -ETIMEDOUT;

	return xfer;
}

static void dw_i3c_master_free_xfer(struct dw_i3c_xfer *xfer)
{
	kfree(xfer);
}

void dump_reg(struct dw_i3c_master *master)
{
	dev_info(master->dev, "================dump i3c reg================\n");
	dev_info(master->dev, "DATA_BUFFER_THLD_CTRL: 0x%x\n",
			readl(master->regs + DATA_BUFFER_THLD_CTRL));
	dev_info(master->dev, "INTR_STATUS: 0x%x\n", readl(master->regs + INTR_STATUS));
	dev_info(master->dev, "INTR_STATUS_EN: 0x%x\n", readl(master->regs + INTR_STATUS_EN));
	dev_info(master->dev, "INTR_SIGNAL_EN: 0x%x\n", readl(master->regs + INTR_SIGNAL_EN));
	dev_info(master->dev, "DEVICE_ADDR: 0x%x\n", readl(master->regs + DEVICE_ADDR));
	dev_info(master->dev, "IBI_SIR_REQ_REJECT: 0x%x\n", readl(master->regs + IBI_SIR_REQ_REJECT));
	dev_info(master->dev, "IBI_MR_REQ_REJECT: 0x%x\n", readl(master->regs + IBI_MR_REQ_REJECT));
	dev_info(master->dev, "DEVICE_CTRL: 0x%x\n", readl(master->regs + DEVICE_CTRL));
	dev_info(master->dev, "QUEUE_THLD_CTRL: 0x%x\n", readl(master->regs + QUEUE_THLD_CTRL));
	dev_info(master->dev, "SCL_I3C_PP_TIMING: 0x%x\n", readl(master->regs + SCL_I3C_PP_TIMING));
	dev_info(master->dev, "BUS_FREE_TIMING: 0x%x\n", readl(master->regs + BUS_FREE_TIMING));
	dev_info(master->dev, "SCL_I3C_OD_TIMING: 0x%x\n", readl(master->regs + SCL_I3C_OD_TIMING));
	dev_info(master->dev, "SCL_EXT_LCNT_TIMING: 0x%x\n", readl(master->regs + SCL_EXT_LCNT_TIMING));
	dev_info(master->dev, "QUEUE_STATUS_LEVEL: 0x%x\n", readl(master->regs + QUEUE_STATUS_LEVEL));
	dev_info(master->dev, "DATA_BUFFER_STATUS_LEVEL: 0x%x\n",
			readl(master->regs + DATA_BUFFER_STATUS_LEVEL));
	dev_info(master->dev, "DEVICE_ADDR_TABLE_POINTER: 0x%x\n",
			readl(master->regs + DEVICE_ADDR_TABLE_POINTER));
	dev_info(master->dev, "COMMAND_QUEUE_PORT: 0x%x\n", readl(master->regs + COMMAND_QUEUE_PORT));
}

static void dw_i3c_master_start_xfer_locked(struct dw_i3c_master *master)
{
	struct dw_i3c_xfer *xfer = master->xferqueue.cur;
	unsigned int i;
	u32 thld_ctrl;

	if (!xfer)
		return;

	for (i = 0; i < xfer->ncmds; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		dw_i3c_master_wr_tx_fifo(master, cmd->tx_buf, cmd->tx_len);
	}

	thld_ctrl = readl(master->regs + QUEUE_THLD_CTRL);
	thld_ctrl &= ~QUEUE_THLD_CTRL_RESP_BUF_MASK;
	thld_ctrl |= QUEUE_THLD_CTRL_RESP_BUF(xfer->ncmds);
	writel(thld_ctrl, master->regs + QUEUE_THLD_CTRL);

	for (i = 0; i < xfer->ncmds; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		writel(cmd->cmd_hi, master->regs + COMMAND_QUEUE_PORT);
		writel(cmd->cmd_lo, master->regs + COMMAND_QUEUE_PORT);
	}
}

static void dw_i3c_master_enqueue_xfer(struct dw_i3c_master *master, struct dw_i3c_xfer *xfer)
{
	unsigned long flags;

	init_completion(&xfer->comp);
	spin_lock_irqsave(&master->xferqueue.lock, flags);
	if (master->xferqueue.cur) {
		list_add_tail(&xfer->node, &master->xferqueue.list);
	} else {
		master->xferqueue.cur = xfer;
		dw_i3c_master_start_xfer_locked(master);
	}
	spin_unlock_irqrestore(&master->xferqueue.lock, flags);
}

static void dw_i3c_master_dequeue_xfer_locked(struct dw_i3c_master *master,
						struct dw_i3c_xfer *xfer)
{
	if (master->xferqueue.cur == xfer) {
		u32 status;

		master->xferqueue.cur = NULL;

		writel(RESET_CTRL_RX_FIFO | RESET_CTRL_TX_FIFO |
			RESET_CTRL_RESP_QUEUE | RESET_CTRL_CMD_QUEUE,
			master->regs + RESET_CTRL);

		readl_poll_timeout_atomic(master->regs + RESET_CTRL, status,
			!status, DEQUEUE_XFER_DELAY_10US, DEQUEUE_XFER_TIMEOUT);
	} else {
		list_del_init(&xfer->node);
	}
}

static void dw_i3c_master_dequeue_xfer(struct dw_i3c_master *master,
					struct dw_i3c_xfer *xfer)
{
	unsigned long flags;

	spin_lock_irqsave(&master->xferqueue.lock, flags);
	dw_i3c_master_dequeue_xfer_locked(master, xfer);
	spin_unlock_irqrestore(&master->xferqueue.lock, flags);
}

static void dw_i3c_master_end_xfer_locked(struct dw_i3c_master *master, u32 isr)
{
	struct dw_i3c_xfer *xfer = master->xferqueue.cur;
	int i;
	int ret = 0;
	u32 nresp;
	struct dw_i3c_cmd *cmd = NULL;
	u32 resp;

	if (!xfer)
		return;

	nresp = readl(master->regs + QUEUE_STATUS_LEVEL);
	nresp = QUEUE_STATUS_LEVEL_RESP(nresp);

	for (i = 0; i < nresp; i++) {
		resp = readl(master->regs + RESPONSE_QUEUE_PORT);

		cmd = &xfer->cmds[RESPONSE_PORT_TID(resp)];
		cmd->rx_len = RESPONSE_PORT_DATA_LEN(resp);
		cmd->error = RESPONSE_PORT_ERR_STATUS(resp);

		if (cmd->rx_len && !cmd->error)
			dw_i3c_master_read_rx_fifo(master, cmd->rx_buf, cmd->rx_len);
	}

	for (i = 0; i < nresp; i++) {
		switch (xfer->cmds[i].error) {
		case RESPONSE_NO_ERROR:
			break;
		case RESPONSE_ERROR_PARITY:
		case RESPONSE_ERROR_IBA_NACK:
		case RESPONSE_ERROR_TRANSF_ABORT:
		case RESPONSE_ERROR_CRC:
		case RESPONSE_ERROR_FRAME:
			ret = -EIO;
			break;
		case RESPONSE_ERROR_OVER_UNDER_FLOW:
			ret = -ENOSPC;
			break;
		case RESPONSE_ERROR_I2C_W_NACK_ERR:
		case RESPONSE_ERROR_ADDRESS_NACK:
		default:
			ret = -EINVAL;
			break;
		}
	}

	xfer->ret = ret;
	complete(&xfer->comp);

	if (ret < 0) {
		dw_i3c_master_dequeue_xfer_locked(master, xfer);
		writel(readl(master->regs + DEVICE_CTRL) | DEV_CTRL_RESUME,
			master->regs + DEVICE_CTRL);
	}

	xfer = list_first_entry_or_null(&master->xferqueue.list,
					struct dw_i3c_xfer,
					node);
	if (xfer)
		list_del_init(&xfer->node);

	master->xferqueue.cur = xfer;
	dw_i3c_master_start_xfer_locked(master);
}

static int dw_i3c_clk_cfg(struct dw_i3c_master *master)
{
	unsigned long core_rate;
	unsigned long core_period;
	u32 scl_timing;
	u32 hcnt;
	u32 lcnt;

	core_rate = clk_get_rate(master->core_clk);
	if (!core_rate)
		return -EINVAL;

	pr_debug("i3c clk: %lu", core_rate);

	core_period = DIV_ROUND_UP(1000000000, core_rate);

	hcnt = DIV_ROUND_UP(I3C_BUS_THIGH_MAX_NS, core_period) - 1;
	if (hcnt < SCL_I3C_TIMING_CNT_MIN)
		hcnt = SCL_I3C_TIMING_CNT_MIN;

	lcnt = DIV_ROUND_UP(core_rate, master->base.bus.scl_rate.i3c) - hcnt;
	if (lcnt < SCL_I3C_TIMING_CNT_MIN)
		lcnt = SCL_I3C_TIMING_CNT_MIN;

	scl_timing = SCL_I3C_TIMING_HCNT(hcnt) | SCL_I3C_TIMING_LCNT(lcnt);
	writel(scl_timing, master->regs + SCL_I3C_PP_TIMING);

	/*
	 * In pure i3c mode, MST_FREE represents tCAS. In shared mode, this
	 * will be set up by dw_i2c_clk_cfg as tLOW.
	 */
	if (master->base.bus.mode == I3C_BUS_MODE_PURE)
		writel(BUS_I3C_MST_FREE(lcnt), master->regs + BUS_FREE_TIMING);

	lcnt = max_t(u8, DIV_ROUND_UP(I3C_BUS_TLOW_OD_MIN_NS, core_period), lcnt);
	scl_timing = SCL_I3C_TIMING_HCNT(hcnt) | SCL_I3C_TIMING_LCNT(lcnt);
	writel(scl_timing, master->regs + SCL_I3C_OD_TIMING);

	lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR1_SCL_RATE) - hcnt;
	scl_timing = SCL_EXT_LCNT_1(lcnt);
	lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR2_SCL_RATE) - hcnt;
	scl_timing |= SCL_EXT_LCNT_2(lcnt);
	lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR3_SCL_RATE) - hcnt;
	scl_timing |= SCL_EXT_LCNT_3(lcnt);
	lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR4_SCL_RATE) - hcnt;
	scl_timing |= SCL_EXT_LCNT_4(lcnt);
	writel(scl_timing, master->regs + SCL_EXT_LCNT_TIMING);

	return 0;
}

static int dw_i2c_clk_cfg(struct dw_i3c_master *master)
{
	unsigned long core_rate;
	unsigned long core_period;
	u32 hcnt;
	u32 lcnt;
	u32 scl_timing;

	core_rate = clk_get_rate(master->core_clk);
	if (!core_rate)
		return -EINVAL;

	core_period = DIV_ROUND_UP(1000000000, core_rate);

	lcnt = DIV_ROUND_UP(I3C_BUS_I2C_FMP_TLOW_MIN_NS, core_period);
	hcnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_FM_PLUS_SCL_RATE) - lcnt;
	scl_timing = SCL_I2C_FMP_TIMING_HCNT(hcnt) |
			SCL_I2C_FMP_TIMING_LCNT(lcnt);
	writel(scl_timing, master->regs + SCL_I2C_FMP_TIMING);

	lcnt = DIV_ROUND_UP(I3C_BUS_I2C_FM_TLOW_MIN_NS, core_period);
	hcnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_FM_SCL_RATE) - lcnt;
	scl_timing = (u32)SCL_I2C_FM_TIMING_HCNT(hcnt) |
			SCL_I2C_FM_TIMING_LCNT(lcnt);
	writel(scl_timing, master->regs + SCL_I2C_FM_TIMING);

	writel(BUS_I3C_MST_FREE(lcnt), master->regs + BUS_FREE_TIMING);
	writel(readl(master->regs + DEVICE_CTRL) | DEV_CTRL_I2C_SLAVE_PRESENT,
		master->regs + DEVICE_CTRL);

	return 0;
}

static int dw_i3c_master_bus_init(struct i3c_master_controller *m)
{
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	struct i3c_bus *bus = i3c_master_get_bus(m);
	struct i3c_device_info info = { };
	u32 thld_ctrl;
	int ret;

	ret = master->platform_ops->init(master);
	if (ret)
		return ret;

	switch (bus->mode) {
	case I3C_BUS_MODE_MIXED_FAST:
	case I3C_BUS_MODE_MIXED_LIMITED:
		ret = dw_i2c_clk_cfg(master);
		if (ret)
			return ret;
		fallthrough;
	case I3C_BUS_MODE_PURE:
		ret = dw_i3c_clk_cfg(master);
		if (ret)
			return ret;
		break;
	default:
		return -EINVAL;
	}

	thld_ctrl = readl(master->regs + QUEUE_THLD_CTRL);
	thld_ctrl &= ~(QUEUE_THLD_CTRL_RESP_BUF_MASK |
			QUEUE_THLD_CTRL_IBI_STAT_MASK |
			QUEUE_THLD_CTRL_IBI_STAT_MASK);
	thld_ctrl |= XR_IBI_STATUS_THLD | XR_IBI_DATA_THLD;
	writel(thld_ctrl, master->regs + QUEUE_THLD_CTRL);

	thld_ctrl = readl(master->regs + DATA_BUFFER_THLD_CTRL);
	thld_ctrl &= ~DATA_BUFFER_THLD_CTRL_RX_BUF;
	writel(thld_ctrl, master->regs + DATA_BUFFER_THLD_CTRL);

	writel(INTR_ALL, master->regs + INTR_STATUS);
	writel(INTR_MASTER_MASK, master->regs + INTR_STATUS_EN);
	writel(INTR_MASTER_MASK, master->regs + INTR_SIGNAL_EN);

	ret = i3c_master_get_free_addr(m, 0);
	if (ret < 0)
		return ret;

	pr_debug("%s, DEV_ADDR_DYNAMIC_ADDR_VALID | DEV_ADDR_DYNAMIC(ret) : 0x%lx, ret: %d\n",
		__func__,
		DEV_ADDR_DYNAMIC_ADDR_VALID | DEV_ADDR_DYNAMIC(ret),
		ret);

	writel(DEV_ADDR_DYNAMIC_ADDR_VALID | DEV_ADDR_DYNAMIC(ret),
		master->regs + DEVICE_ADDR);

	memset(&info, 0, sizeof(info));
	info.dyn_addr = ret;

	pr_debug("%s, info.dyn_addr: 0x%x, info.static_addr: 0x%x\n",
			__func__, info.dyn_addr, info.static_addr);

	ret = i3c_master_set_info(&master->base, &info);
	if (ret)
		return ret;

	writel(IBI_REQ_REJECT_ALL, master->regs + IBI_SIR_REQ_REJECT);
	writel(IBI_REQ_REJECT_ALL, master->regs + IBI_MR_REQ_REJECT);

	/* For now don't support Hot-Join */
	writel(readl(master->regs + DEVICE_CTRL) | DEV_CTRL_HOT_JOIN_NACK,
		master->regs + DEVICE_CTRL);

	dw_i3c_master_enable(master);

	return 0;
}

static void dw_i3c_master_bus_cleanup(struct i3c_master_controller *m)
{
	struct dw_i3c_master *master = to_dw_i3c_master(m);

	dw_i3c_master_disable(master);
}

static int dw_i3c_ccc_set(struct dw_i3c_master *master, struct i3c_ccc_cmd *ccc)
{
	struct dw_i3c_xfer *xfer = NULL;
	struct dw_i3c_cmd *cmd = NULL;
	int ret;
	int pos = 0;

	if (ccc->id & I3C_CCC_DIRECT) {
		pos = dw_i3c_master_get_addr_pos(master, ccc->dests[0].addr);
		if (pos < 0)
			return pos;
	}

	xfer = dw_i3c_master_alloc_xfer(master, 1);
	if (!xfer)
		return -ENOMEM;

	cmd = xfer->cmds;
	cmd->tx_buf = ccc->dests[0].payload.data;
	cmd->tx_len = ccc->dests[0].payload.len;

	cmd->cmd_hi = (u32)COMMAND_PORT_ARG_DATA_LEN(ccc->dests[0].payload.len) |
			COMMAND_PORT_TRANSFER_ARG;

	cmd->cmd_lo = COMMAND_PORT_CP |
			COMMAND_PORT_DEV_INDEX(pos) |
			COMMAND_PORT_CMD(ccc->id) |
			COMMAND_PORT_TOC |
			COMMAND_PORT_ROC;

	dw_i3c_master_enqueue_xfer(master, xfer);
	if (!wait_for_completion_timeout(&xfer->comp, XFER_TIMEOUT))
		dw_i3c_master_dequeue_xfer(master, xfer);

	ret = xfer->ret;
	if (xfer->cmds[0].error == RESPONSE_ERROR_IBA_NACK)
		ccc->err = I3C_ERROR_M2;

	dw_i3c_master_free_xfer(xfer);

	return ret;
}

static int dw_i3c_ccc_get(struct dw_i3c_master *master, struct i3c_ccc_cmd *ccc)
{
	struct dw_i3c_xfer *xfer = NULL;
	struct dw_i3c_cmd *cmd = NULL;
	int ret;
	int pos;

	pos = dw_i3c_master_get_addr_pos(master, ccc->dests[0].addr);
	if (pos < 0)
		return pos;

	xfer = dw_i3c_master_alloc_xfer(master, 1);
	if (!xfer)
		return -ENOMEM;

	cmd = xfer->cmds;
	cmd->rx_buf = ccc->dests[0].payload.data;
	cmd->rx_len = ccc->dests[0].payload.len;

	cmd->cmd_hi = (u32)COMMAND_PORT_ARG_DATA_LEN(ccc->dests[0].payload.len) |
			COMMAND_PORT_TRANSFER_ARG;

	cmd->cmd_lo = COMMAND_PORT_READ_TRANSFER |
			COMMAND_PORT_CP |
			COMMAND_PORT_DEV_INDEX(pos) |
			COMMAND_PORT_CMD(ccc->id) |
			COMMAND_PORT_TOC |
			COMMAND_PORT_ROC;

	dw_i3c_master_enqueue_xfer(master, xfer);
	if (!wait_for_completion_timeout(&xfer->comp, XFER_TIMEOUT))
		dw_i3c_master_dequeue_xfer(master, xfer);

	ret = xfer->ret;
	if (xfer->cmds[0].error == RESPONSE_ERROR_IBA_NACK)
		ccc->err = I3C_ERROR_M2;
	dw_i3c_master_free_xfer(xfer);

	return ret;
}

static int dw_i3c_master_send_ccc_cmd(struct i3c_master_controller *m,
					struct i3c_ccc_cmd *ccc)
{
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	int ret = 0;

	if (ccc->id == I3C_CCC_ENTDAA)
		return -EINVAL;

	if (ccc->rnw)
		ret = dw_i3c_ccc_get(master, ccc);
	else
		ret = dw_i3c_ccc_set(master, ccc);

	return ret;
}

static int dw_i3c_master_daa(struct i3c_master_controller *m)
{
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	struct dw_i3c_xfer *xfer = NULL;
	struct dw_i3c_cmd *cmd = NULL;
	u32 olddevs;
	u32 newdevs;
	u8 p;
	u8 last_addr = 0;
	int ret;
	int pos;

	olddevs = ~(master->free_pos);

	/* Prepare DAT before launching DAA. */
	for (pos = 0; pos < master->maxdevs; pos++) {
		if (olddevs & BIT(pos))
			continue;

		ret = i3c_master_get_free_addr(m, last_addr + 1);
		if (ret < 0)
			return -ENOSPC;

		master->devs[pos].addr = ret;
		p = even_parity(ret);
		last_addr = ret;
		ret |= (p << 7);

		pr_debug("%s, DEV_ADDR_TABLE_DYNAMIC_ADDR(0x%x): 0x%lx, datstartaddr: 0x%x, pos: %u\n",
			__func__,
			ret,
			DEV_ADDR_TABLE_DYNAMIC_ADDR(ret),
			master->datstartaddr,
			pos);

		writel(DEV_ADDR_TABLE_DYNAMIC_ADDR(ret),
			master->regs +
			DEV_ADDR_TABLE_LOC(master->datstartaddr, pos));
	}

	xfer = dw_i3c_master_alloc_xfer(master, 1);
	if (!xfer)
		return -ENOMEM;

	pos = dw_i3c_master_get_free_pos(master);
	if (pos < 0) {
		dw_i3c_master_free_xfer(xfer);
		return pos;
	}
	cmd = &xfer->cmds[0];
	cmd->cmd_hi = 0x1;
	cmd->cmd_lo = COMMAND_PORT_DEV_COUNT(master->maxdevs - pos) |
			COMMAND_PORT_DEV_INDEX(pos) |
			COMMAND_PORT_CMD(I3C_CCC_ENTDAA) |
			COMMAND_PORT_ADDR_ASSGN_CMD |
			COMMAND_PORT_TOC |
			COMMAND_PORT_ROC;

	dw_i3c_master_enqueue_xfer(master, xfer);
	if (!wait_for_completion_timeout(&xfer->comp, XFER_TIMEOUT))
		dw_i3c_master_dequeue_xfer(master, xfer);

	newdevs = GENMASK(master->maxdevs - cmd->rx_len - 1, 0);
	newdevs &= ~olddevs;

	for (pos = 0; pos < master->maxdevs; pos++) {
		if (newdevs & BIT(pos))
			i3c_master_add_i3c_dev_locked(m, master->devs[pos].addr);
	}

	dw_i3c_master_free_xfer(xfer);

	return 0;
}

static int dw_i3c_master_priv_xfers(struct i3c_dev_desc *dev,
				struct i3c_priv_xfer *i3c_xfers,
				int i3c_nxfers)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	unsigned int nrxwords = 0;
	unsigned int ntxwords = 0;
	struct dw_i3c_xfer *xfer = NULL;
	int i;
	int ret = 0;
	struct dw_i3c_cmd *cmd = NULL;

	if (!i3c_nxfers)
		return 0;

	if (i3c_nxfers > master->caps.cmdfifodepth)
		return -ENOTSUPP;

	for (i = 0; i < i3c_nxfers; i++) {
		if (i3c_xfers[i].rnw)
			nrxwords += DIV_ROUND_UP(i3c_xfers[i].len, NRX_WORDS_DIV_BASE);
		else
			ntxwords += DIV_ROUND_UP(i3c_xfers[i].len, NRX_WORDS_DIV_BASE);
	}

	if (ntxwords > master->caps.datafifodepth ||
		nrxwords > master->caps.datafifodepth)
		return -ENOTSUPP;

	xfer = dw_i3c_master_alloc_xfer(master, i3c_nxfers);
	if (!xfer)
		return -ENOMEM;

	for (i = 0; i < i3c_nxfers; i++) {
		cmd = &xfer->cmds[i];

		cmd->cmd_hi = (u32)COMMAND_PORT_ARG_DATA_LEN(i3c_xfers[i].len) |
			COMMAND_PORT_TRANSFER_ARG;

		if (i3c_xfers[i].rnw) {
			cmd->rx_buf = i3c_xfers[i].data.in;
			cmd->rx_len = i3c_xfers[i].len;
			cmd->cmd_lo = COMMAND_PORT_READ_TRANSFER |
					COMMAND_PORT_SPEED(dev->info.max_read_ds);

		} else {
			cmd->tx_buf = i3c_xfers[i].data.out;
			cmd->tx_len = i3c_xfers[i].len;
			cmd->cmd_lo =
				COMMAND_PORT_SPEED(dev->info.max_write_ds);
		}

		cmd->cmd_lo |= COMMAND_PORT_TID(i) |
				COMMAND_PORT_DEV_INDEX(data->index) |
				COMMAND_PORT_ROC;

		if (i == (i3c_nxfers - 1))
			cmd->cmd_lo |= COMMAND_PORT_TOC;
	}

	pr_debug("%s, DYNAMIC_ADDR: 0x%lx, index: %u, addr: 0x%x\n",
		__func__,
		DEV_ADDR_TABLE_DYNAMIC_ADDR(master->devs[data->index].addr),
		data->index,
		master->devs[data->index].addr);

	dw_i3c_master_enqueue_xfer(master, xfer);
	if (!wait_for_completion_timeout(&xfer->comp, XFER_TIMEOUT))
		dw_i3c_master_dequeue_xfer(master, xfer);

	for (i = 0; i < i3c_nxfers; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		if (i3c_xfers[i].rnw)
			i3c_xfers[i].len = cmd->rx_len;
	}

	ret = xfer->ret;

	dw_i3c_master_free_xfer(xfer);

	return ret;
}

static int dw_i3c_master_reattach_i3c_dev(struct i3c_dev_desc *dev, u8 old_dyn_addr)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	int pos;

	pos = dw_i3c_master_get_free_pos(master);

	if (data->index > pos && pos > 0) {
		writel(0, master->regs + DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

		master->devs[data->index].addr = 0;
		master->free_pos |= BIT(data->index);

		data->index = pos;
		master->devs[pos].addr = dev->info.dyn_addr;
		master->free_pos &= ~BIT(pos);
	}

	writel(DEV_ADDR_TABLE_DYNAMIC_ADDR(dev->info.dyn_addr),
		master->regs + DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

	master->devs[data->index].addr = dev->info.dyn_addr;

	return 0;
}

static int dw_i3c_master_attach_i3c_dev(struct i3c_dev_desc *dev)
{
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	struct dw_i3c_i2c_dev_data *data = NULL;
	int pos;

	pos = dw_i3c_master_get_free_pos(master);
	if (pos < 0)
		return pos;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->index = pos;
	master->devs[pos].addr = dev->info.dyn_addr ? : dev->info.static_addr;
	master->free_pos &= ~BIT(pos);
	i3c_dev_set_master_data(dev, data);

	writel(DEV_ADDR_TABLE_DYNAMIC_ADDR(master->devs[pos].addr),
		master->regs + DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

	pr_debug("%s, DYNAMIC_ADDR: 0x%lx, index: %u, addr: 0x%x, static_addr: 0x%x, dyn_addr: 0x%x\n",
		__func__,
		DEV_ADDR_TABLE_DYNAMIC_ADDR(master->devs[pos].addr),
		data->index,
		master->devs[pos].addr,
		dev->info.static_addr,
		dev->info.dyn_addr);

	return 0;
}

static void dw_i3c_master_detach_i3c_dev(struct i3c_dev_desc *dev)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);

	writel(0, master->regs + DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

	i3c_dev_set_master_data(dev, NULL);
	master->devs[data->index].addr = 0;
	master->free_pos |= BIT(data->index);
	kfree(data);
}

static int
dw_i3c_master_i2c_xfers(struct i2c_dev_desc *dev, const struct i2c_msg *i2c_xfers, int i2c_nxfers)
{
	struct dw_i3c_i2c_dev_data *data = i2c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i2c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	unsigned int nrxwords = 0;
	unsigned int ntxwords = 0;
	struct dw_i3c_xfer *xfer = NULL;
	int i;
	int ret = 0;
	struct dw_i3c_cmd *cmd = NULL;

	if (!i2c_nxfers)
		return 0;

	if (i2c_nxfers > master->caps.cmdfifodepth)
		return -ENOTSUPP;

	for (i = 0; i < i2c_nxfers; i++) {
		if (i2c_xfers[i].flags & I2C_M_RD)
			nrxwords += DIV_ROUND_UP(i2c_xfers[i].len, 4);
		else
			ntxwords += DIV_ROUND_UP(i2c_xfers[i].len, 4);
	}

	if (ntxwords > master->caps.datafifodepth || nrxwords > master->caps.datafifodepth)
		return -ENOTSUPP;

	xfer = dw_i3c_master_alloc_xfer(master, i2c_nxfers);
	if (!xfer)
		return -ENOMEM;

	for (i = 0; i < i2c_nxfers; i++) {
		cmd = &xfer->cmds[i];

		cmd->cmd_hi = (u32)COMMAND_PORT_ARG_DATA_LEN(i2c_xfers[i].len) |
			COMMAND_PORT_TRANSFER_ARG;

		cmd->cmd_lo = COMMAND_PORT_TID(i) |
				COMMAND_PORT_DEV_INDEX(data->index) |
				COMMAND_PORT_ROC;

		if (i2c_xfers[i].flags & I2C_M_RD) {
			cmd->cmd_lo |= COMMAND_PORT_READ_TRANSFER;
			cmd->rx_buf = i2c_xfers[i].buf;
			cmd->rx_len = i2c_xfers[i].len;
		} else {
			cmd->tx_buf = i2c_xfers[i].buf;
			cmd->tx_len = i2c_xfers[i].len;
		}

		if (i == (i2c_nxfers - 1))
			cmd->cmd_lo |= COMMAND_PORT_TOC;
	}

	dw_i3c_master_enqueue_xfer(master, xfer);
	if (!wait_for_completion_timeout(&xfer->comp, XFER_TIMEOUT))
		dw_i3c_master_dequeue_xfer(master, xfer);

	ret = xfer->ret;
	dw_i3c_master_free_xfer(xfer);

	return ret;
}

static int dw_i3c_master_attach_i2c_dev(struct i2c_dev_desc *dev)
{
	struct i3c_master_controller *m = i2c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	struct dw_i3c_i2c_dev_data *data = NULL;
	int pos;

	pos = dw_i3c_master_get_free_pos(master);
	if (pos < 0)
		return pos;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->index = pos;
	master->devs[pos].addr = dev->addr;
	master->free_pos &= ~BIT(pos);
	i2c_dev_set_master_data(dev, data);

	writel(DEV_ADDR_TABLE_LEGACY_I2C_DEV |
		DEV_ADDR_TABLE_STATIC_ADDR(dev->addr),
		master->regs + DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

	return 0;
}

static void dw_i3c_master_detach_i2c_dev(struct i2c_dev_desc *dev)
{
	struct dw_i3c_i2c_dev_data *data = i2c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i2c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);

	writel(0, master->regs + DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

	i2c_dev_set_master_data(dev, NULL);
	master->devs[data->index].addr = 0;
	master->free_pos |= BIT(data->index);
	kfree(data);
}

static int dw_i3c_master_request_ibi(struct i3c_dev_desc *dev, const struct i3c_ibi_setup *req)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	unsigned long flags;

	data->ibi_pool = i3c_generic_ibi_alloc_pool(dev, req);
	if (IS_ERR(data->ibi_pool))
		return PTR_ERR(data->ibi_pool);

	spin_lock_irqsave(&master->devs_lock, flags);
	master->devs[data->index].ibi_dev = dev;
	spin_unlock_irqrestore(&master->devs_lock, flags);

	return 0;
}

static void dw_i3c_master_free_ibi(struct i3c_dev_desc *dev)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	unsigned long flags;

	spin_lock_irqsave(&master->devs_lock, flags);
	master->devs[data->index].ibi_dev = NULL;
	spin_unlock_irqrestore(&master->devs_lock, flags);

	i3c_generic_ibi_free_pool(data->ibi_pool);
	data->ibi_pool = NULL;
}

static void dw_i3c_master_set_sir_enabled(struct dw_i3c_master *master,
				struct i3c_dev_desc *dev, u8 idx, bool enable)
{
	unsigned long flags;
	u32 dat_entry;
	u32 reg;
	bool global;

	dat_entry = DEV_ADDR_TABLE_LOC(master->datstartaddr, idx);

	spin_lock_irqsave(&master->devs_lock, flags);
	reg = readl(master->regs + dat_entry);
	if (enable) {
		reg &= ~DEV_ADDR_TABLE_SIR_REJECT;
		if (dev->info.bcr & I3C_BCR_IBI_PAYLOAD)
			reg |= DEV_ADDR_TABLE_IBI_MDB;
	} else {
		reg |= DEV_ADDR_TABLE_SIR_REJECT;
	}
	master->platform_ops->set_dat_ibi(master, dev, enable, &reg);
	writel(reg, master->regs + dat_entry);

	reg = readl(master->regs + IBI_SIR_REQ_REJECT);
	if (enable) {
		global = reg == 0xffffffff;
		reg &= ~BIT(idx);
	} else {
		global = reg == 0;
		reg |= BIT(idx);
	}
	writel(reg, master->regs + IBI_SIR_REQ_REJECT);

	if (global) {
		reg = readl(master->regs + INTR_STATUS_EN);
		reg &= ~INTR_IBI_THLD_STAT;
		if (enable)
			reg |= INTR_IBI_THLD_STAT;
		writel(reg, master->regs + INTR_STATUS_EN);

		reg = readl(master->regs + INTR_SIGNAL_EN);
		reg &= ~INTR_IBI_THLD_STAT;
		if (enable)
			reg |= INTR_IBI_THLD_STAT;
		writel(reg, master->regs + INTR_SIGNAL_EN);
	}

	spin_unlock_irqrestore(&master->devs_lock, flags);
}

static int dw_i3c_master_enable_ibi(struct i3c_dev_desc *dev)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	int rc;

	dw_i3c_master_set_sir_enabled(master, dev, data->index, true);

	rc = i3c_master_enec_locked(m, dev->info.dyn_addr, I3C_CCC_EVENT_SIR);

	if (rc)
		dw_i3c_master_set_sir_enabled(master, dev, data->index, false);

	return rc;
}

static int dw_i3c_master_disable_ibi(struct i3c_dev_desc *dev)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	int rc;

	rc = i3c_master_disec_locked(m, dev->info.dyn_addr, I3C_CCC_EVENT_SIR);
	if (rc)
		return rc;

	dw_i3c_master_set_sir_enabled(master, dev, data->index, false);

	return 0;
}

static void dw_i3c_master_recycle_ibi_slot(struct i3c_dev_desc *dev, struct i3c_ibi_slot *slot)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);

	i3c_generic_ibi_recycle_slot(data->ibi_pool, slot);
}

static void dw_i3c_master_drain_ibi_queue(struct dw_i3c_master *master, int len)
{
	int i;

	for (i = 0; i < DIV_ROUND_UP(len, 4); i++)
		readl(master->regs + IBI_QUEUE_STATUS);
}

static void dw_i3c_master_handle_ibi_sir(struct dw_i3c_master *master, u32 status)
{
	struct dw_i3c_i2c_dev_data *data = NULL;
	struct i3c_ibi_slot *slot = NULL;
	struct i3c_dev_desc *dev = NULL;
	unsigned long flags;
	u8 addr;
	u8 len;
	int idx;

	addr = IBI_QUEUE_IBI_ADDR(status);
	len = IBI_QUEUE_STATUS_DATA_LEN(status);

	/*
	 * We be tempted to check the error status in bit 30; however, due
	 * to the PEC errata workaround on some platform implementations (see
	 * ast2600_i3c_set_dat_ibi()), those will almost always have a PEC
	 * error on IBI payload data, as well as losing the last byte of
	 * payload.
	 *
	 * If we implement error status checking on that bit, we may need
	 * a new platform op to validate it.
	 */

	spin_lock_irqsave(&master->devs_lock, flags);
	idx = dw_i3c_master_get_addr_pos(master, addr);
	if (idx < 0) {
		dev_dbg_ratelimited(&master->base.dev, "IBI from unknown addr 0x%x\n", addr);
		goto err_drain;
	}

	dev = master->devs[idx].ibi_dev;
	if (!dev || !dev->ibi) {
		dev_dbg_ratelimited(&master->base.dev, "IBI from non-requested dev idx %d\n", idx);
		goto err_drain;
	}

	data = i3c_dev_get_master_data(dev);
	slot = i3c_generic_ibi_get_free_slot(data->ibi_pool);
	if (!slot) {
		dev_dbg_ratelimited(&master->base.dev, "No IBI slots available\n");
		goto err_drain;
	}

	if (dev->ibi->max_payload_len < len) {
		dev_dbg_ratelimited(&master->base.dev,
				"IBI payload len %d greater than max %d\n",
				len, dev->ibi->max_payload_len);
		goto err_drain;
	}

	if (len) {
		dw_i3c_master_read_ibi_fifo(master, slot->data, len);
		slot->len = len;
	}
	i3c_master_queue_ibi(dev, slot);

	spin_unlock_irqrestore(&master->devs_lock, flags);

	return;

err_drain:
	dw_i3c_master_drain_ibi_queue(master, len);

	spin_unlock_irqrestore(&master->devs_lock, flags);
}

/* "ibis": referring to In-Band Interrupts, and not
 * https://en.wikipedia.org/wiki/Australian_white_ibis. The latter should
 * not be handled.
 */
static void dw_i3c_master_irq_handle_ibis(struct dw_i3c_master *master)
{
	u32 i;
	u32 len;
	u32 n_ibis;
	u32 reg;

	reg = readl(master->regs + QUEUE_STATUS_LEVEL);
	n_ibis = QUEUE_STATUS_IBI_STATUS_CNT(reg);
	if (!n_ibis)
		return;

	for (i = 0; i < n_ibis; i++) {
		reg = readl(master->regs + IBI_QUEUE_STATUS);

		if (IBI_TYPE_SIRQ(reg)) {
			dw_i3c_master_handle_ibi_sir(master, reg);
		} else {
			len = IBI_QUEUE_STATUS_DATA_LEN(reg);
			dev_dbg(&master->base.dev,
				"unsupported IBI type 0x%lx len %d\n",
				IBI_QUEUE_STATUS_IBI_ID(reg), len);
			dw_i3c_master_drain_ibi_queue(master, len);
		}
	}
}

static irqreturn_t dw_i3c_master_irq_handler(int irq, void *dev_id)
{
	struct dw_i3c_master *master = dev_id;
	u32 status;

	status = readl(master->regs + INTR_STATUS);
	if (!(status & readl(master->regs + INTR_STATUS_EN))) {
		writel(INTR_ALL, master->regs + INTR_STATUS);
		return IRQ_NONE;
	}

	spin_lock(&master->xferqueue.lock);
	dw_i3c_master_end_xfer_locked(master, status);
	if (status & INTR_TRANSFER_ERR_STAT)
		writel(INTR_TRANSFER_ERR_STAT, master->regs + INTR_STATUS);
	spin_unlock(&master->xferqueue.lock);

	if (status & INTR_IBI_THLD_STAT)
		dw_i3c_master_irq_handle_ibis(master);

	return IRQ_HANDLED;
}

static const struct i3c_master_controller_ops dw_mipi_i3c_ops = {
	.bus_init = dw_i3c_master_bus_init,
	.bus_cleanup = dw_i3c_master_bus_cleanup,
	.attach_i3c_dev = dw_i3c_master_attach_i3c_dev,
	.reattach_i3c_dev = dw_i3c_master_reattach_i3c_dev,
	.detach_i3c_dev = dw_i3c_master_detach_i3c_dev,
	.do_daa = dw_i3c_master_daa,
	.supports_ccc_cmd = dw_i3c_master_supports_ccc_cmd,
	.send_ccc_cmd = dw_i3c_master_send_ccc_cmd,
	.priv_xfers = dw_i3c_master_priv_xfers,
	.attach_i2c_dev = dw_i3c_master_attach_i2c_dev,
	.detach_i2c_dev = dw_i3c_master_detach_i2c_dev,
	.i2c_xfers = dw_i3c_master_i2c_xfers,
};

static const struct i3c_master_controller_ops dw_mipi_i3c_ibi_ops = {
	.bus_init = dw_i3c_master_bus_init,
	.bus_cleanup = dw_i3c_master_bus_cleanup,
	.attach_i3c_dev = dw_i3c_master_attach_i3c_dev,
	.reattach_i3c_dev = dw_i3c_master_reattach_i3c_dev,
	.detach_i3c_dev = dw_i3c_master_detach_i3c_dev,
	.do_daa = dw_i3c_master_daa,
	.supports_ccc_cmd = dw_i3c_master_supports_ccc_cmd,
	.send_ccc_cmd = dw_i3c_master_send_ccc_cmd,
	.priv_xfers = dw_i3c_master_priv_xfers,
	.attach_i2c_dev = dw_i3c_master_attach_i2c_dev,
	.detach_i2c_dev = dw_i3c_master_detach_i2c_dev,
	.i2c_xfers = dw_i3c_master_i2c_xfers,
	.request_ibi = dw_i3c_master_request_ibi,
	.free_ibi = dw_i3c_master_free_ibi,
	.enable_ibi = dw_i3c_master_enable_ibi,
	.disable_ibi = dw_i3c_master_disable_ibi,
	.recycle_ibi_slot = dw_i3c_master_recycle_ibi_slot,
};

/* default platform ops implementations */
static int dw_i3c_platform_init_nop(struct dw_i3c_master *i3c)
{
	return 0;
}

static void dw_i3c_platform_set_dat_ibi_nop(struct dw_i3c_master *i3c,
					struct i3c_dev_desc *dev,
					bool enable, u32 *dat)
{
}

static const struct dw_i3c_platform_ops dw_i3c_platform_ops_default = {
	.init = dw_i3c_platform_init_nop,
	.set_dat_ibi = dw_i3c_platform_set_dat_ibi_nop,
};

int i3c_prepare_clk(struct dw_i3c_master *master, bool prepare)
{
	int ret;

	if (prepare) {
		/* Optional interface clock */
		ret = clk_prepare_enable(master->pclk);
		if (ret)
			return ret;

		ret = clk_prepare_enable(master->core_clk);
		if (ret)
			clk_disable_unprepare(master->pclk);

		return ret;
	}

	clk_disable_unprepare(master->core_clk);
	clk_disable_unprepare(master->pclk);

	return 0;
}

int dw_i3c_common_probe(struct dw_i3c_master *master,
			struct platform_device *pdev)
{
	const struct i3c_master_controller_ops *ops = NULL;
	int ret;
	int irq;
	int int_flag = 0;

	master->dev = &pdev->dev;

	if (!master->platform_ops)
		master->platform_ops = &dw_i3c_platform_ops_default;

	master->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(master->regs))
		return PTR_ERR(master->regs);

	master->core_clk = devm_clk_get(&pdev->dev, "clk_i3c");
	if (IS_ERR(master->core_clk))
		return PTR_ERR(master->core_clk);

	master->pclk = devm_clk_get(&pdev->dev, "pclk_i3c");
	if (IS_ERR(master->pclk))
		return PTR_ERR(master->pclk);

	master->prst = devm_reset_control_get_exclusive(&pdev->dev, "prst");
	if (IS_ERR(master->prst))
		return PTR_ERR(master->prst);

	master->core_rst = devm_reset_control_get_exclusive(&pdev->dev, "rst");
	if (IS_ERR(master->core_rst))
		return PTR_ERR(master->core_rst);

	ret = reset_control_deassert(master->prst);
	if (ret < 0) {
		dev_err(&pdev->dev, "Couldn't deassert prst control: %d\n", ret);
		return ret;
	}

	ret = reset_control_deassert(master->core_rst);
	if (ret < 0) {
		dev_err(&pdev->dev, "Couldn't deassert core_rst control: %d\n", ret);
		return ret;
	}

	ret = i3c_prepare_clk(master, true);
	if (ret)
		goto err_disable_core_clk;

	spin_lock_init(&master->xferqueue.lock);
	INIT_LIST_HEAD(&master->xferqueue.list);

	writel(INTR_ALL, master->regs + INTR_STATUS);
	irq = platform_get_irq(pdev, 0);
	ret = devm_request_irq(&pdev->dev, irq,
				dw_i3c_master_irq_handler, int_flag,
				dev_name(&pdev->dev), master);
	if (ret)
		goto err_assert_rst;

	platform_set_drvdata(pdev, master);

	/* Information regarding the FIFOs/QUEUEs depth */
	ret = readl(master->regs + QUEUE_STATUS_LEVEL);
	master->caps.cmdfifodepth = QUEUE_STATUS_LEVEL_CMD(ret);

	ret = readl(master->regs + DATA_BUFFER_STATUS_LEVEL);
	master->caps.datafifodepth = DATA_BUFFER_STATUS_LEVEL_TX(ret);

	dev_dbg(&pdev->dev, "master->caps.cmdfifodepth: 0x%x, master->caps.datafifodepth: 0x%x\n",
		master->caps.cmdfifodepth, master->caps.datafifodepth);

	ret = readl(master->regs + DEVICE_ADDR_TABLE_POINTER);
	master->datstartaddr = ret;
	master->maxdevs = ret >> DEV_ADDR_TABLE_DEPTH_OFFSET;
	master->free_pos = GENMASK(master->maxdevs - 1, 0);

	dev_dbg(&pdev->dev, "master->datstartaddr: 0x%x\n", master->datstartaddr);
	dev_dbg(&pdev->dev, "master->maxdevs: 0x%x, master->free_pos: 0x%x\n", master->maxdevs, master->free_pos);

	ops = &dw_mipi_i3c_ops;
	if (master->ibi_capable)
		ops = &dw_mipi_i3c_ibi_ops;

	ret = i3c_master_register(&master->base, &pdev->dev, ops, false);
	if (ret)
		goto err_assert_rst;

	dev_info(&pdev->dev, "i3c init success.\n");

	return 0;

err_assert_rst:
	reset_control_assert(master->prst);
	reset_control_assert(master->core_rst);

err_disable_core_clk:
	i3c_prepare_clk(master, false);

	return ret;
}
EXPORT_SYMBOL_GPL(dw_i3c_common_probe);

void dw_i3c_common_remove(struct dw_i3c_master *master)
{
	i3c_master_unregister(&master->base);

	reset_control_assert(master->prst);
	reset_control_assert(master->core_rst);

	i3c_prepare_clk(master, false);
}
EXPORT_SYMBOL_GPL(dw_i3c_common_remove);

/* base platform implementation */

static int dw_i3c_probe(struct platform_device *pdev)
{
	struct dw_i3c_master *master = NULL;

	master = devm_kzalloc(&pdev->dev, sizeof(*master), GFP_KERNEL);
	if (!master)
		return -ENOMEM;

	return dw_i3c_common_probe(master, pdev);
}

static void dw_i3c_remove(struct platform_device *pdev)
{
	struct dw_i3c_master *master = platform_get_drvdata(pdev);

	dw_i3c_common_remove(master);
}

static const struct of_device_id dw_i3c_master_of_match[] = {
	{ .compatible = "snps,dw-i3c-master-1.00a", },
	{},
};
MODULE_DEVICE_TABLE(of, dw_i3c_master_of_match);

static struct platform_driver dw_i3c_driver = {
	.probe = dw_i3c_probe,
	.remove_new = dw_i3c_remove,
	.driver = {
		.name = "dw-i3c-master",
		.of_match_table = dw_i3c_master_of_match,
	},
};
module_platform_driver(dw_i3c_driver);

MODULE_AUTHOR("Vitor Soares <vitor.soares@synopsys.com>");
MODULE_AUTHOR("Hu Wei <huwei19@xiaomi.com>");
MODULE_DESCRIPTION("DesignWare MIPI I3C driver");
MODULE_LICENSE("GPL v2");
