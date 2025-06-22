// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * XRing I2C adapter driver.
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/debugfs.h>

#include "i2c-designware-core.h"

#define I2C_USE_RX_TL	15
#define I2C_USE_TX_TL	(dev->tx_fifo_depth / 2)

#define I2C_DMA_RX_TL	15
#define I2C_DMA_TX_TL	(dev->tx_fifo_depth / 2)

#define XR_IC_RX_FLR_MASK	0x3F
#define XR_IC_TX_FLR_MASK	0x3F

#define STOP		0x200  /* BIT(9) */
#define RESTART		0x400  /* BIT(10) */
#define READ_CMD	0x100  /* BIT(8) */

#define SCL_SS_FS_LOW_PERIODS	300
#define SDA_SS_FS_LOW_PERIODS	300
#define SS_SCL_T_HIGH		4000	/* tHD;STA = tHIGH = 4.0 us */
#define SS_SCL_T_LOW		4700	/* tLOW = 4.7 us */
#define FP_SCL_T_HIGH		260	/* tHIGH = 260 ns */
#define FP_SCL_T_LOW		500	/* tLOW = 500 ns */
#define FS_SCL_T_HIGH		600	/* tHD;STA = tHIGH = 0.6 us */
#define FS_SCL_T_LOW		1300	/* tLOW = 1.3 us */
#define HS_SCL_T_HIGH		60	/* tHIGH = 60 ns */
#define HS_SCL_T_LOW		160	/* tLOW = 160 ns */

#define USE_PACKET_ERROR_CHECKING	2
#define NOT_PACKET_ERROR_CHECKING	1

int i2c_irq_lock_clk_disable(struct dw_i2c_dev *dev, u32 state)
{
	unsigned long flags;
	int ret;

	if (!dev) {
		pr_err("%s: i2c contrller do not be init.\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&dev->irq_running_lock, flags);
	dev->irq_running = state;
	ret = i2c_dw_prepare_clk(dev, I2C_CLK_DISABLE);
	if (ret) {
		spin_unlock_irqrestore(&dev->irq_running_lock, flags);
		dev_err(dev->dev, "prepare clk failed, ret = %d\n", ret);
		return ret;
	}

	spin_unlock_irqrestore(&dev->irq_running_lock, flags);

	return 0;
}

int i2c_irq_lock(struct dw_i2c_dev *dev, u32 state)
{
	unsigned long flags;
	u32 dummy;

	if (!dev) {
		pr_err("%s: i2c contrller do not be init.\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&dev->irq_running_lock, flags);
	regmap_write(dev->map, DW_IC_INTR_MASK, 0); /* close all interrupt */
	regmap_read(dev->map, DW_IC_CLR_INTR, &dummy);
	__i2c_dw_disable_nowait(dev);
	dev->irq_running = state;
	spin_unlock_irqrestore(&dev->irq_running_lock, flags);

	return 0;
}

int i2c_pinsctrl(struct dw_i2c_dev *dev, const char *name)
{
	struct pinctrl_state *s;
	int ret;

	if (!dev) {
		pr_err("%s: i2c contrller do not be init.\n", __func__);
		return -ENODEV;
	}

	if (dev->pinctrl_flag != PINCTRL_ALREADY_INIT) {
		dev->pinctrl = devm_pinctrl_get(dev->dev);
		if (IS_ERR(dev->pinctrl))
			return PINCTRL_CONFIG_ERROR;

		dev->pinctrl_flag = PINCTRL_ALREADY_INIT;
	}

	s = pinctrl_lookup_state(dev->pinctrl, name);
	if (IS_ERR(s)) {
		devm_pinctrl_put(dev->pinctrl);
		dev->pinctrl_flag = 0;
		return PINCTRL_CONFIG_ERROR;
	}

	ret = pinctrl_select_state(dev->pinctrl, s);
	if (ret < 0) {
		devm_pinctrl_put(dev->pinctrl);
		dev->pinctrl_flag = 0;
		return PINCTRL_CONFIG_ERROR;
	}

	return 0;
}

static void i2c_dw_configure_fifo_master(struct dw_i2c_dev *dev)
{
	/* Configure Tx/Rx FIFO threshold levels */
	regmap_write(dev->map, DW_IC_TX_TL, I2C_USE_TX_TL);
	regmap_write(dev->map, DW_IC_RX_TL, I2C_USE_RX_TL);

#if defined CONFIG_XRING_I2C_DMA
	/* Configure DMA Tx/Rx FIFO threshold levels */
	regmap_write(dev->map, XR_IC_DMA_TDLR, I2C_DMA_TX_TL);
	regmap_write(dev->map, XR_IC_DMA_RDLR, I2C_DMA_RX_TL);
#endif

	/* Configure the I2C master */
	regmap_write(dev->map, DW_IC_CON, dev->master_cfg);
}

static int i2c_dw_set_timings_master(struct dw_i2c_dev *dev)
{
	u32 comp_param1;
	u32 sda_falling_time;
	u32 scl_falling_time;
	struct i2c_timings *t = &dev->timings;
	const char *fp_str = "";
	u32 ic_clk;
	int ret;

	ic_clk = i2c_dw_clk_rate(dev);

	ret = i2c_dw_acquire_lock(dev);
	if (ret)
		return ret;

	ret = regmap_read(dev->map, DW_IC_COMP_PARAM_1, &comp_param1);
	i2c_dw_release_lock(dev);
	if (ret)
		return ret;

	/* Set standard and fast speed dividers for high/low periods */
	sda_falling_time = t->sda_fall_ns ?: SDA_SS_FS_LOW_PERIODS; /* ns */
	scl_falling_time = t->scl_fall_ns ?: SCL_SS_FS_LOW_PERIODS; /* ns */

	/* Calculate SCL timing parameters for standard mode if not set */
	if (!dev->ss_hcnt || !dev->ss_lcnt) {
		dev->ss_hcnt =
			i2c_dw_scl_hcnt(ic_clk,
					SS_SCL_T_HIGH,
					sda_falling_time,
					0,	/* 0: DW default, 1: Ideal */
					0);	/* No offset */
		dev->ss_lcnt =
			i2c_dw_scl_lcnt(ic_clk,
					SS_SCL_T_LOW,
					scl_falling_time,
					0);	/* No offset */
	}
	dev_dbg(dev->dev, "Standard Mode HCNT:LCNT = %d:%d\n",
		dev->ss_hcnt, dev->ss_lcnt);

	/*
	 * Set SCL timing parameters for fast mode or fast mode plus. Only
	 * difference is the timing parameter values since the registers are
	 * the same.
	 */
	if (t->bus_freq_hz == I2C_MAX_FAST_MODE_PLUS_FREQ) {
		/*
		 * Check are Fast Mode Plus parameters available. Calculate
		 * SCL timing parameters for Fast Mode Plus if not set.
		 */
		if (dev->fp_hcnt && dev->fp_lcnt) {
			dev->fs_hcnt = dev->fp_hcnt;
			dev->fs_lcnt = dev->fp_lcnt;
		} else {
			dev->fs_hcnt =
				i2c_dw_scl_hcnt(ic_clk,
						FP_SCL_T_HIGH,
						sda_falling_time,
						0,	/* DW default */
						0);	/* No offset */
			dev->fs_lcnt =
				i2c_dw_scl_lcnt(ic_clk,
						FP_SCL_T_LOW,
						scl_falling_time,
						0);	/* No offset */
		}
		fp_str = " Plus";
	}

	/*
	 * Calculate SCL timing parameters for fast mode if not set. They are
	 * needed also in high speed mode.
	 */
	if (!dev->fs_hcnt || !dev->fs_lcnt) {
		dev->fs_hcnt =
			i2c_dw_scl_hcnt(ic_clk,
					FS_SCL_T_HIGH,
					sda_falling_time,
					0,	/* 0: DW default, 1: Ideal */
					0);	/* No offset */
		dev->fs_lcnt =
			i2c_dw_scl_lcnt(ic_clk,
					FS_SCL_T_LOW,
					scl_falling_time,
					0);	/* No offset */
	}
	dev_dbg(dev->dev, "Fast Mode%s HCNT:LCNT = %d:%d\n",
		fp_str, dev->fs_hcnt, dev->fs_lcnt);

	/* Check is high speed possible and fall back to fast mode if not */
	if ((dev->master_cfg & DW_IC_CON_SPEED_MASK) ==
		DW_IC_CON_SPEED_HIGH) {
		if ((comp_param1 & DW_IC_COMP_PARAM_1_SPEED_MODE_MASK)
			!= DW_IC_COMP_PARAM_1_SPEED_MODE_HIGH) {
			dev_err(dev->dev, "High Speed not supported!\n");
			t->bus_freq_hz = I2C_MAX_FAST_MODE_FREQ;
			dev->master_cfg &= ~DW_IC_CON_SPEED_MASK;
			dev->master_cfg |= DW_IC_CON_SPEED_FAST;
			dev->hs_hcnt = 0;
			dev->hs_lcnt = 0;
		} else if (!dev->hs_hcnt || !dev->hs_lcnt) {
			dev->hs_hcnt =
				i2c_dw_scl_hcnt(ic_clk,
						HS_SCL_T_HIGH,
						sda_falling_time,
						0,	/* DW default */
						0);	/* No offset */
			dev->hs_lcnt =
				i2c_dw_scl_lcnt(ic_clk,
						HS_SCL_T_LOW,
						scl_falling_time,
						0);	/* No offset */
		}
		dev_dbg(dev->dev, "High Speed Mode HCNT:LCNT = %d:%d\n",
			dev->hs_hcnt, dev->hs_lcnt);
	}

	ret = i2c_dw_set_sda_hold(dev);
	if (ret)
		return ret;

	dev_dbg(dev->dev, "Bus speed: %s\n", i2c_freq_mode_string(t->bus_freq_hz));
	return 0;
}

/**
 * i2c_dw_init_master() - Initialize the designware I2C master hardware
 * @dev: device private data
 *
 * This functions configures and enables the I2C master.
 * This function is called during I2C init function, and in case of timeout at
 * run time.
 */
static int i2c_dw_init_master(struct dw_i2c_dev *dev)
{
	int ret;

	ret = i2c_dw_acquire_lock(dev);
	if (ret)
		return ret;

	/* Disable the adapter */
	__i2c_dw_disable(dev);

	/* Write standard speed timing parameters */
	regmap_write(dev->map, DW_IC_SS_SCL_HCNT, dev->ss_hcnt);
	regmap_write(dev->map, DW_IC_SS_SCL_LCNT, dev->ss_lcnt);

	/* Write fast mode/fast mode plus timing parameters */
	regmap_write(dev->map, DW_IC_FS_SCL_HCNT, dev->fs_hcnt);
	regmap_write(dev->map, DW_IC_FS_SCL_LCNT, dev->fs_lcnt);

	/* Write high speed timing parameters if supported */
	if (dev->hs_hcnt && dev->hs_lcnt) {
		regmap_write(dev->map, DW_IC_HS_SCL_HCNT, dev->hs_hcnt);
		regmap_write(dev->map, DW_IC_HS_SCL_LCNT, dev->hs_lcnt);
	}

	/**
	 * Write SDA hold time if supported
	 * SDA RX HOLD, bit[23:16]:
	 *     0x1, config in the i2c_dw_set_sda_hold func
	 * SDA TX HOLD, bit[15:0]:
	 *     chip default or from DTB
	 */
	if (dev->sda_hold_time)
		regmap_write(dev->map, DW_IC_SDA_HOLD, dev->sda_hold_time);

	i2c_dw_configure_fifo_master(dev);
	i2c_dw_release_lock(dev);

	return 0;
}

void i2c_dw_xfer_init(struct dw_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 ic_con = 0;
	u32 ic_tar = 0;
	u32 dummy;

	/* Disable the adapter */
	__i2c_dw_disable(dev);

	/* If the slave address is ten bit address, enable 10BITADDR */
	if (msgs[dev->msg_write_idx].flags & I2C_M_TEN) {
		ic_con = DW_IC_CON_10BITADDR_MASTER;
		/*
		 * If I2C_DYNAMIC_TAR_UPDATE is set, the 10-bit addressing
		 * mode has to be enabled via bit 12 of IC_TAR register.
		 * We set it always as I2C_DYNAMIC_TAR_UPDATE can't be
		 * detected from registers.
		 */
		ic_tar = DW_IC_TAR_10BITADDR_MASTER;
	}

	regmap_update_bits(dev->map, DW_IC_CON, DW_IC_CON_10BITADDR_MASTER,
			   ic_con);

	/*
	 * Set the slave (target) address and enable 10-bit addressing mode
	 * if applicable.
	 */
	regmap_write(dev->map, DW_IC_TAR,
		     msgs[dev->msg_write_idx].addr | ic_tar);

	/* Enforce disabled interrupts (due to HW issues) */
	i2c_dw_disable_int(dev);

	/* Enable the adapter */
	__i2c_dw_enable(dev);

	/* Dummy read to avoid the register getting stuck on Bay Trail */
	regmap_read(dev->map, DW_IC_ENABLE_STATUS, &dummy);

	/* Clear and enable interrupts */
	regmap_read(dev->map, DW_IC_CLR_INTR, &dummy);
	regmap_write(dev->map, DW_IC_INTR_MASK, DW_IC_INTR_MASTER_MASK);
}

/*
 * Initiate (and continue) low level master read/write transaction.
 * This function is only called from i2c_dw_isr, and pumping i2c_msg
 * messages into the tx buffer.  Even if the size of i2c_msg data is
 * longer than the size of the tx buffer, it handles everything.
 */
static void i2c_dw_xfer_msg(struct dw_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 intr_mask;
	int tx_limit;
	int rx_limit;
	u32 addr = msgs[dev->msg_write_idx].addr;
	u32 buf_len = dev->tx_buf_len;
	u8 *buf = dev->tx_buf;
	bool need_restart = false;
	unsigned int flr;
	u32 cmd = 0;
	u32 flags;

	intr_mask = DW_IC_INTR_MASTER_MASK;

#if defined CONFIG_XRING_I2C_DEBUGFS
	dev->tx_cmd_intr_count++;
#endif

	for (; dev->msg_write_idx < dev->msgs_num; dev->msg_write_idx++) {
		flags = msgs[dev->msg_write_idx].flags;

		/*
		 * If target address has changed, we need to
		 * reprogram the target address in the I2C
		 * adapter when we are done with this transfer.
		 */
		if (msgs[dev->msg_write_idx].addr != addr) {
			dev_err(dev->dev,
				"%s: invalid target address\n", __func__);
			dev->msg_err = -EINVAL;
			break;
		}

		if (!(dev->status & STATUS_WRITE_IN_PROGRESS)) {
			/* new i2c_msg */
			buf = msgs[dev->msg_write_idx].buf;
			buf_len = msgs[dev->msg_write_idx].len;

			/* If both IC_EMPTYFIFO_HOLD_MASTER_EN and
			 * IC_RESTART_EN are set, we must manually
			 * set restart bit between messages.
			 */
			if ((dev->master_cfg & DW_IC_CON_RESTART_EN) &&
					(dev->msg_write_idx > 0))
				need_restart = true;
		}

		regmap_read(dev->map, DW_IC_TXFLR, &flr);
		tx_limit = dev->tx_fifo_depth - flr;

		regmap_read(dev->map, DW_IC_RXFLR, &flr);
		rx_limit = dev->rx_fifo_depth - flr;

		while (buf_len > 0 && tx_limit > 0 && rx_limit > 0) {
			cmd = 0;

			/*
			 * If IC_EMPTYFIFO_HOLD_MASTER_EN is set we must
			 * manually set the stop bit. However, it cannot be
			 * detected from the registers so we set it always
			 * when writing/reading the last byte.
			 * Unfortunately, O1 used this scheme.
			 */
			if (dev->msg_write_idx == dev->msgs_num - 1 && buf_len == 1)
				cmd |= STOP;

			if (need_restart) {
				cmd |= RESTART;
				need_restart = false;
			}

			if (msgs[dev->msg_write_idx].flags & I2C_M_RD) {

				/* Avoid rx buffer overrun */
				if (dev->rx_outstanding >= dev->rx_fifo_depth)
					break;

				regmap_write(dev->map, DW_IC_DATA_CMD,
					     cmd | READ_CMD);
				rx_limit--;
				dev->rx_outstanding++;
			} else {
				regmap_write(dev->map, DW_IC_DATA_CMD,
					     cmd | *buf++);
			}
			tx_limit--;
			buf_len--;
		}

		dev->tx_buf = buf;
		dev->tx_buf_len = buf_len;

		/*
		 * Because we don't know the buffer length in the
		 * I2C_FUNC_SMBUS_BLOCK_DATA case, we can't stop
		 * the transaction here.
		 */
		if (buf_len > 0 || flags & I2C_M_RECV_LEN) {
			/* more bytes to be written */
			dev->status |= STATUS_WRITE_IN_PROGRESS;
			break;
		}
		dev->status &= ~STATUS_WRITE_IN_PROGRESS;
	}

	/*
	 * If i2c_msg index search is completed, we don't need TX_EMPTY
	 * interrupt any more.
	 */
	if (dev->msg_write_idx == dev->msgs_num)
		intr_mask &= ~DW_IC_INTR_TX_EMPTY;

	if (dev->msg_err)
		intr_mask = 0;

	regmap_write(dev->map,  DW_IC_INTR_MASK, intr_mask);
}

static u8 i2c_dw_recv_len(struct dw_i2c_dev *dev, u8 len)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 flags = msgs[dev->msg_read_idx].flags;

	/*
	 * Adjust the buffer length and mask the flag
	 * after receiving the first byte.
	 */
	len += (flags & I2C_CLIENT_PEC) ? USE_PACKET_ERROR_CHECKING : NOT_PACKET_ERROR_CHECKING;
	dev->tx_buf_len = len - min_t(u8, len, dev->rx_outstanding);
	msgs[dev->msg_read_idx].len = len;
	msgs[dev->msg_read_idx].flags &= ~I2C_M_RECV_LEN;

	return len;
}

static void i2c_dw_read(struct dw_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	unsigned int rx_valid;
	u32 len;
	u32 tmp;
	u8 *buf = NULL;
	u32 flags;

#if defined CONFIG_XRING_I2C_DEBUGFS
	dev->rx_data_intr_count++;
#endif

	for (; dev->msg_read_idx < dev->msgs_num; dev->msg_read_idx++) {
		/* no read mode */
		if (!(msgs[dev->msg_read_idx].flags & I2C_M_RD))
			continue;

		if (!(dev->status & STATUS_READ_IN_PROGRESS)) {
			len = msgs[dev->msg_read_idx].len;
			buf = msgs[dev->msg_read_idx].buf;
		} else {
			len = dev->rx_buf_len;
			buf = dev->rx_buf;
		}

		regmap_read(dev->map, DW_IC_RXFLR, &rx_valid);

		for (; len > 0 && rx_valid > 0; len--, rx_valid--) {
			flags = msgs[dev->msg_read_idx].flags;

			regmap_read(dev->map, DW_IC_DATA_CMD, &tmp);
			/* Ensure length byte is a valid value */
			if (flags & I2C_M_RECV_LEN &&
			    (tmp & DW_IC_DATA_CMD_DAT) <= I2C_SMBUS_BLOCK_MAX && tmp > 0) {
				len = i2c_dw_recv_len(dev, tmp);
			}
			*buf++ = tmp;
			dev->rx_outstanding--;
		}

		if (dev->rx_outstanding <= TL_TO_FIFO(I2C_USE_RX_TL))
			regmap_write(dev->map, DW_IC_RX_TL, TO_RX_TX_TL(dev->rx_outstanding));

		if (len > 0) {
			dev->status |= STATUS_READ_IN_PROGRESS;
			dev->rx_buf_len = len;
			dev->rx_buf = buf;
			return;
		}
		dev->status &= ~STATUS_READ_IN_PROGRESS;
	}
}

/*
 * Prepare controller for a transaction and call i2c_dw_xfer_msg.
 * XRing I2C DMA
 */
int i2c_dw_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	struct dw_i2c_dev *dev = i2c_get_adapdata(adap);
	int ret;
	int i;

	mutex_lock(&dev->lock);

	dev_dbg(dev->dev, "%s: msgs: %d\n", __func__, num);

	if (dev->hwspin_lock) {
		ret = i2c_hardware_spinlock(dev);
		if (ret < 0) {
			dev_err(dev->dev, "check request hwspinlock\n");
			mutex_unlock(&dev->lock);
			return ret;
		}
	}

	ret = xr_i2c_runtime_resume(dev->dev);
	if (ret < 0) {
		dev_err(dev->dev, "error prepare xfer: %d\n", ret);
		if (dev->hwspin_lock)
			hwspin_unlock_raw(dev->hwspin_lock);

		mutex_unlock(&dev->lock);
		return ret;
	}

	if (dev_WARN_ONCE(dev->dev, dev->suspended, "Transfer while suspended\n")) {
		ret = -ESHUTDOWN;
		goto done_nolock;
	}

	reinit_completion(&dev->cmd_complete);
	dev->msgs = msgs;
	dev->msgs_num = num;
	dev->rx_msgs_num = 0;
	dev->cmd_err = 0;
	dev->msg_write_idx = 0;
	dev->msg_read_idx = 0;
	dev->msg_err = 0;
	dev->status = STATUS_IDLE;
	dev->abort_source = 0;
	dev->intr_status = 0;
	dev->rx_outstanding = 0;
	dev->tx_cmd_intr_count = 0;
	dev->rx_data_intr_count = 0;
	dev->using_dma = !USE_DMA;
	dev->cmd_total_len = 0;
	dev->rx_data_len = 0;
	dev->xr_i2c_dma_en_cr = 0;
	dev->irq_running = XR_I2C_IRQ_EXIT;

	ret = i2c_dw_wait_bus_not_busy(dev);
	if (ret < 0)
		goto done_nolock;

	regmap_write(dev->map, DW_IC_RX_TL, I2C_USE_RX_TL);

	if (dev->ip_has_dma == IP_HAS_DMA) {
		for (i = 0 ; i < num; i++) {
			dev->cmd_total_len += msgs[i].len;
			if (msgs[i].flags & I2C_M_RD) {
				dev->rx_data_len += msgs[i].len;
				dev->rx_msgs_num++;
			}
		}

		dev_dbg(dev->dev, "dev->cmd_total_len:%u, dev->rx_data_len:%u",
				dev->cmd_total_len, dev->rx_data_len);

		if (dev->cmd_total_len > DMA_THRESHOLD) {
			ret = xr_dma_xfer(dev);
			if (ret < 0) {
				dev_err(dev->dev, "DMA tansfer mode failed, normal mode\n");
				/* Start the transfers */
				i2c_dw_xfer_init(dev);
			}
		} else {
			/* Start the transfers */
			i2c_dw_xfer_init(dev);
		}
	} else {
		/* Start the transfers */
		i2c_dw_xfer_init(dev);
	}

	if (adap->timeout != HZ)
		adap->timeout = HZ;  /* default */

	/* Wait for tx to complete */
	if ((dev->ip_has_dma == IP_HAS_DMA) && (dev->using_dma == USE_DMA)) {
		ret = wait_for_completion_timeout(&dev->dma_complete,
						XR_I2C_DMA_TRANSFER_TIMEOUT);
		if (!ret) {
			dev_err(dev->dev, "dma transfer timed out\n");
			ret = -ETIMEDOUT;
			goto done;
		}
	} else {
		ret = wait_for_completion_timeout(&dev->cmd_complete, adap->timeout);
		if (!ret) {
			dev_err(dev->dev, "controller timed out\n");
			ret = -ETIMEDOUT;
			goto done;
		}
	}

	if ((dev->using_dma == USE_DMA) &&
		!(dev->intr_status & DW_IC_INTR_STOP_DET) &&
		!(dev->cmd_err & DW_IC_ERR_TX_ABRT)) {
		ret = regmap_read_poll_timeout(dev->map, DW_IC_INTR_STAT, dev->intr_status,
					!(dev->intr_status & DW_IC_INTR_STOP_DET),
					XR_WAIT_FOR_COMPLETE_MIN,
					R_WAIT_FOR_COMPLETE_MAX);
		if (ret) {
			dev_warn(dev->dev, "No STOP signal? msgs[0].addr: 0x%x, status: 0x%x\n",
					msgs[0].addr, dev->intr_status);
		}
	}

	i2c_irq_lock(dev, XR_I2C_TRANS_DONE);

	/*
	 * We must disable the adapter before returning and signaling the end
	 * of the current transfer. Otherwise the hardware might continue
	 * generating interrupts which in turn causes a race condition with
	 * the following transfer.  Needs some more investigation if the
	 * additional interrupts are a hardware bug or this driver doesn't
	 * handle them correctly yet.
	 */
	__i2c_dw_disable_nowait(dev);

	if (dev->msg_err) {
		ret = dev->msg_err;
		goto done;
	}

	/* No error */
	if (likely(!dev->cmd_err && !dev->status)) {
		ret = num;
		goto done;
	}

	/* We have an error */
	if (dev->cmd_err == DW_IC_ERR_TX_ABRT) {
		ret = i2c_dw_handle_tx_abort(dev);
		goto done;
	}

	if (dev->status)
		dev_err(dev->dev,
			"transfer terminated early - interrupt latency too high?, status = %d\n", dev->status);

	ret = -EIO;

done:
	if (ret == -ETIMEDOUT) {
		i2c_irq_lock(dev, XR_I2C_RECOVERING);
		/* i2c_dw_init implicitly disables the adapter */
		i2c_recover_bus(&dev->adapter);
	}

	if (dev->ip_has_dma == IP_HAS_DMA && dev->using_dma == USE_DMA) {
		if (dev->cmd_err || dev->status || dev->msg_err || ret == -ETIMEDOUT) {
			dev_err(dev->dev, "dma mode, dev->msg_err: %d, dev->cmd_err: %d, dev->status: %d",
				dev->msg_err, dev->cmd_err, dev->status);
			terminate_dma_transfer(dev);
		}
		i2c_dma_unmap(dev);
	}

done_nolock:
	xr_i2c_runtime_suspend(dev->dev);
	if (dev->hwspin_lock)
		hwspin_unlock_raw(dev->hwspin_lock);

	mutex_unlock(&dev->lock);
	return ret;
}

static const struct i2c_algorithm i2c_dw_algo = {
	.master_xfer = i2c_dw_xfer,
	.functionality = i2c_dw_func,
};

static u32 i2c_dw_read_clear_intrbits(struct dw_i2c_dev *dev)
{
	u32 stat, dummy;

	/*
	 * The IC_INTR_STAT register just indicates "enabled" interrupts.
	 * The unmasked raw version of interrupt status bits is available
	 * in the IC_RAW_INTR_STAT register.
	 *
	 * That is,
	 *   stat = readl(IC_INTR_STAT);
	 * equals to,
	 *   stat = readl(IC_RAW_INTR_STAT) & readl(IC_INTR_MASK);
	 *
	 * The raw version might be useful for debugging purposes.
	 */
	regmap_read(dev->map, DW_IC_INTR_STAT, &stat);
	dev->intr_status = stat;
	/*
	 * Do not use the IC_CLR_INTR register to clear interrupts, or
	 * you'll miss some interrupts, triggered during the period from
	 * readl(IC_INTR_STAT) to readl(IC_CLR_INTR).
	 *
	 * Instead, use the separately-prepared IC_CLR_* registers.
	 */
	if (stat & DW_IC_INTR_RX_UNDER)
		regmap_read(dev->map, DW_IC_CLR_RX_UNDER, &dummy);
	if (stat & DW_IC_INTR_RX_OVER)
		regmap_read(dev->map, DW_IC_CLR_RX_OVER, &dummy);
	if (stat & DW_IC_INTR_TX_OVER)
		regmap_read(dev->map, DW_IC_CLR_TX_OVER, &dummy);
	if (stat & DW_IC_INTR_RD_REQ)
		regmap_read(dev->map, DW_IC_CLR_RD_REQ, &dummy);
	if (stat & DW_IC_INTR_TX_ABRT) {
		/*
		 * The IC_TX_ABRT_SOURCE register is cleared whenever
		 * the IC_CLR_TX_ABRT is read.  Preserve it beforehand.
		 */
		regmap_read(dev->map, DW_IC_TX_ABRT_SOURCE, &dev->abort_source);
		regmap_read(dev->map, DW_IC_CLR_TX_ABRT, &dummy);
	}
	if (stat & DW_IC_INTR_RX_DONE)
		regmap_read(dev->map, DW_IC_CLR_RX_DONE, &dummy);
	if (stat & DW_IC_INTR_ACTIVITY)
		regmap_read(dev->map, DW_IC_CLR_ACTIVITY, &dummy);
	if (stat & DW_IC_INTR_STOP_DET)
		regmap_read(dev->map, DW_IC_CLR_STOP_DET, &dummy);
	if (stat & DW_IC_INTR_START_DET)
		regmap_read(dev->map, DW_IC_CLR_START_DET, &dummy);
	if (stat & DW_IC_INTR_GEN_CALL)
		regmap_read(dev->map, DW_IC_CLR_GEN_CALL, &dummy);

	return stat;
}

/*
 * Interrupt service routine. This gets called whenever an I2C master interrupt
 * occurs.
 */
static int i2c_dw_irq_handler_master(struct dw_i2c_dev *dev)
{
	u32 stat;

	stat = i2c_dw_read_clear_intrbits(dev);
	if (stat & DW_IC_INTR_TX_ABRT) {
		dev->cmd_err |= DW_IC_ERR_TX_ABRT;
		dev->status = STATUS_IDLE;

		/*
		 * Anytime TX_ABRT is set, the contents of the tx/rx
		 * buffers are flushed. Make sure to skip them.
		 */
		regmap_write(dev->map, DW_IC_INTR_MASK, 0);
		goto tx_aborted;
	}

	if (stat & DW_IC_INTR_RX_FULL)
		i2c_dw_read(dev);

	if (stat & DW_IC_INTR_TX_EMPTY)
		i2c_dw_xfer_msg(dev);

	if (stat & DW_IC_INTR_STOP_DET) {
#if defined CONFIG_XRING_I2C_DMA
		if (dev->using_dma != USE_DMA) {
			i2c_dw_read(dev);
			complete(&dev->cmd_complete);
		}
#else
		i2c_dw_read(dev);
		complete(&dev->cmd_complete);
#endif
	}

	/*
	 * No need to modify or disable the interrupt mask here.
	 * i2c_dw_xfer_msg() will take care of it according to
	 * the current transmit status.
	 */

tx_aborted:
	if ((stat & DW_IC_INTR_TX_ABRT) || dev->msg_err) {
#if defined CONFIG_XRING_I2C_DMA
		if (dev->using_dma != USE_DMA)
			complete(&dev->cmd_complete);
		else
			complete(&dev->dma_complete);
#else
		complete(&dev->cmd_complete);
#endif
	}

	return 0;
}

static irqreturn_t i2c_dw_isr(int this_irq, void *dev_id)
{
	struct dw_i2c_dev *dev = dev_id;
	u32 stat, enabled;
	unsigned long flags;

	spin_lock_irqsave(&dev->irq_running_lock, flags);
	if (dev->irq_running == XR_I2C_RECOVERING || dev->irq_running == XR_I2C_TRANS_DONE) {
		spin_unlock_irqrestore(&dev->irq_running_lock, flags);
		dev_info(dev->dev, "dev->irq_running = %#x\n", dev->irq_running);
		return IRQ_HANDLED;
	}

	dev->irq_running = XR_I2C_IRQ_RUNING;

	regmap_read(dev->map, DW_IC_ENABLE, &enabled);
	regmap_read(dev->map, DW_IC_RAW_INTR_STAT, &stat);
	if (!enabled || !(stat & ~DW_IC_INTR_ACTIVITY)) {
		dev->irq_running = XR_I2C_IRQ_EXIT;
		spin_unlock_irqrestore(&dev->irq_running_lock, flags);
		dev_info(dev->dev, "enabled=%#x stat=%#x\n", enabled, stat);
		return IRQ_NONE;
	}

	i2c_dw_irq_handler_master(dev);

	dev->irq_running = XR_I2C_IRQ_EXIT;
	spin_unlock_irqrestore(&dev->irq_running_lock, flags);

	return IRQ_HANDLED;
}

int i2c_dw_configure_master(struct dw_i2c_dev *dev)
{
	struct i2c_timings *t;

	if (!dev)
		return -EINVAL;

	t = &dev->timings;

	dev->functionality = I2C_FUNC_10BIT_ADDR | DW_IC_DEFAULT_FUNCTIONALITY;

	dev->master_cfg = DW_IC_CON_MASTER | DW_IC_CON_SLAVE_DISABLE |
			DW_IC_CON_RESTART_EN;

	dev->mode = DW_IC_MASTER;

	switch (t->bus_freq_hz) {
	case I2C_MAX_STANDARD_MODE_FREQ:
		dev->master_cfg |= DW_IC_CON_SPEED_STD;
		break;
	case I2C_MAX_HIGH_SPEED_MODE_FREQ:
		dev->master_cfg |= DW_IC_CON_SPEED_HIGH;
		break;
	default:
		dev->master_cfg |= DW_IC_CON_SPEED_FAST;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(i2c_dw_configure_master);

static void i2c_dw_prepare_recovery(struct i2c_adapter *adap)
{
	struct dw_i2c_dev *dev = i2c_get_adapdata(adap);
	int ret;

	i2c_dw_disable(dev);

	ret = i2c_irq_lock_clk_disable(dev, XR_I2C_RECOVERING);
	if (ret)
		dev_err(dev->dev, "failed to lock irq\n");

	ret = reset_control_assert(dev->rst);
	if (ret < 0)
		dev_err(dev->dev, "%s, Couldn't assert rst: %d\n", __func__, ret);

	ret = reset_control_assert(dev->prst);
	if (ret < 0)
		dev_err(dev->dev, "%s, Couldn't assert prst: %d\n", __func__, ret);

	if (dev->asic == true)
		i2c_pinsctrl(dev, PINCTRL_STATE_SLEEP);  /* gpio mode */
}

static void i2c_dw_unprepare_recovery(struct i2c_adapter *adap)
{
	struct dw_i2c_dev *dev = i2c_get_adapdata(adap);
	int ret;

	i2c_dw_prepare_clk(dev, I2C_CLK_EBABLE);
	ret = reset_control_deassert(dev->prst);
	if (ret < 0) {
		dev_err(dev->dev, "%s, Couldn't deassert prst: %d\n", __func__, ret);
		return;
	}

	ret = reset_control_deassert(dev->rst);
	if (ret < 0) {
		dev_err(dev->dev, "%s, Couldn't deassert rst: %d\n", __func__, ret);
		return;
	}

	i2c_dw_init_master(dev);
	if (dev->asic == true)
		i2c_pinsctrl(dev, PINCTRL_STATE_DEFAULT); /* SDA/SCL mode */
}

static int i2c_dw_init_recovery_info(struct dw_i2c_dev *dev)
{
	struct i2c_bus_recovery_info *rinfo = &dev->rinfo;
	struct i2c_adapter *adap = &dev->adapter;
	struct gpio_desc *gpio;

	gpio = devm_gpiod_get_optional(dev->dev, "scl", GPIOD_OUT_HIGH);
	if (IS_ERR_OR_NULL(gpio))
		return PTR_ERR_OR_ZERO(gpio);

	rinfo->scl_gpiod = gpio;

	gpio = devm_gpiod_get_optional(dev->dev, "sda", GPIOD_IN);
	if (IS_ERR(gpio))
		return PTR_ERR(gpio);
	rinfo->sda_gpiod = gpio;

	rinfo->recover_bus = i2c_generic_scl_recovery;
	rinfo->prepare_recovery = i2c_dw_prepare_recovery;
	rinfo->unprepare_recovery = i2c_dw_unprepare_recovery;
	adap->bus_recovery_info = rinfo;

	dev_info(dev->dev, "running with gpio recovery mode! scl%s",
		 rinfo->sda_gpiod ? ",sda" : "");

	return 0;
}

int i2c_dw_probe_master(struct dw_i2c_dev *dev)
{
	struct i2c_adapter *adap = &dev->adapter;
	unsigned long irq_flags;
	int ret;
	unsigned int hw_spinlock_id = 0;
#if defined CONFIG_XRING_I2C_DMA
	int dma_ret;

	init_completion(&dev->dma_complete);
#endif
	init_completion(&dev->cmd_complete);

	spin_lock_init(&dev->irq_running_lock);

	ret = device_property_read_u32(dev->dev, "hw-spinlock", &hw_spinlock_id);
	if (!ret && hw_spinlock_id) {
		dev->hwspin_lock = hwspin_lock_request_specific(hw_spinlock_id);
		if (!dev->hwspin_lock) {
			dev_err(dev->dev, "hwspin_lock request error\n");
			return -EBUSY;
		}
		dev_info(dev->dev, "hwspin_lock request id: %u\n", hw_spinlock_id);
	}

	dev->init = i2c_dw_init_master;
	dev->disable = i2c_dw_disable;
	dev->disable_int = i2c_dw_disable_int;
	mutex_init(&dev->lock);

	ret = i2c_dw_init_regmap(dev);
	if (ret)
		return ret;

	ret = i2c_dw_set_timings_master(dev);
	if (ret)
		return ret;

	ret = i2c_dw_set_fifo_size(dev);
	if (ret)
		return ret;

	ret = dev->init(dev);
	if (ret)
		return ret;

	snprintf(adap->name, sizeof(adap->name),
		 "Synopsys DesignWare I2C adapter");
	adap->retries = RETRIES;
	adap->algo = &i2c_dw_algo;
	adap->dev.parent = dev->dev;
	i2c_set_adapdata(adap, dev);

	irq_flags = IRQF_SHARED | IRQF_COND_SUSPEND;

	i2c_dw_disable_int(dev);
	ret = devm_request_irq(dev->dev, dev->irq, i2c_dw_isr, irq_flags,
			       dev_name(dev->dev), dev);
	if (ret) {
		dev_err(dev->dev, "failure requesting irq %i: %d\n",
			dev->irq, ret);
		return ret;
	}

	dev->pinctrl_flag = -PINCTRL_ALREADY_INIT;
	ret = i2c_dw_init_recovery_info(dev);
	if (ret)
		return ret;

	dev->asic = true;
	ret = of_property_count_strings(dev->dev->of_node, "pinctrl-names");
	if (ret < 0) {
		dev->asic = false;
	} else {
		ret = i2c_pinsctrl(dev, PINCTRL_STATE_DEFAULT); /* SDA/SCL mode */
		if (ret < 0)
			dev_warn(dev->dev, "pins are not configured to default\n");
	}

	ret = i2c_add_numbered_adapter(adap);
	if (ret)
		dev_err(dev->dev, "failure adding adapter: %d\n", ret);

#if defined CONFIG_XRING_I2C_DMA
	dma_ret = xr_i2c_dma_init(dev);
	if (dma_ret < 0) {
		dev->ip_has_dma = -IP_HAS_DMA;
		dev_warn(dev->dev, "i2c %u not support DMA\n",  adap->nr);
	} else {
		/**
		 * if set
		 *     Can directly use the current driver's "device".
		 * else
		 *     As a dma consumer driver, dma_map_xx device must follow:
		 *       struct dma_chan *xxchan;
		 *         - struct dma_device *device
		 *           - struct device *dev;
		 *     eg: dma_map_xx(xxchan->device->dev, ...);
		 */
		dma_ret = dma_set_mask_and_coherent(dev->dev,
				DMA_BIT_MASK(DMA_BIT_MASK_64));
		if (dma_ret)
			dma_ret = dma_set_mask_and_coherent(dev->dev,
					DMA_BIT_MASK(DMA_BIT_MASK_64));
		if (dma_ret) {
			dev_err(dev->dev, "could not set DMA mask\n");
			dev->ip_has_dma = -IP_HAS_DMA;
		}
	}
#endif

#if defined CONFIG_XRING_I2C_DEBUGFS
	xr_i2c_debugfs_init(dev);
#endif
	return ret;
}
EXPORT_SYMBOL_GPL(i2c_dw_probe_master);
