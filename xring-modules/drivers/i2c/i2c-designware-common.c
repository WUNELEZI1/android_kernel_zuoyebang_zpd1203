// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * XRing I2C adapter driver.
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/acpi.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/swab.h>
#include <linux/types.h>
#include <linux/units.h>
#include <soc/xring/xr_hwspinlock.h>

#include "i2c-designware-core.h"

#define XR_I2C_REG_BITS			32
#define XR_I2C_VAL_BITS			32
#define XR_I2C_REG_STRIDE		4

#define  XR_I2C_STATUS_NOT_ENABLE(x)	(((x) & 1) == 0)

static char *abort_sources[] = {
	[ABRT_7B_ADDR_NOACK] =
		"slave address not acknowledged (7bit mode)",
	[ABRT_10ADDR1_NOACK] =
		"first address byte not acknowledged (10bit mode)",
	[ABRT_10ADDR2_NOACK] =
		"second address byte not acknowledged (10bit mode)",
	[ABRT_TXDATA_NOACK] =
		"data not acknowledged",
	[ABRT_GCALL_NOACK] =
		"no acknowledgment for a general call",
	[ABRT_GCALL_READ] =
		"read after general call",
	[ABRT_SBYTE_ACKDET] =
		"start byte acknowledged",
	[ABRT_SBYTE_NORSTRT] =
		"trying to send start byte when restart is disabled",
	[ABRT_10B_RD_NORSTRT] =
		"trying to read when restart is disabled (10bit mode)",
	[ABRT_MASTER_DIS] =
		"trying to use disabled adapter",
	[ARB_LOST] =
		"lost arbitration",
	[ABRT_SLAVE_FLUSH_TXFIFO] =
		"read command so flush old data in the TX FIFO",
	[ABRT_SLAVE_ARBLOST] =
		"slave lost the bus while transmitting data to a remote master",
	[ABRT_SLAVE_RD_INTX] =
		"incorrect slave-transmitter mode configuration",
};

static int dw_reg_read(void *context, unsigned int reg, unsigned int *val)
{
	struct dw_i2c_dev *dev = context;

	*val = readl_relaxed(dev->base + reg);

	return 0;
}

static int dw_reg_write(void *context, unsigned int reg, unsigned int val)
{
	struct dw_i2c_dev *dev = context;

	writel_relaxed(val, dev->base + reg);

	return 0;
}

static int dw_reg_read_swab(void *context, unsigned int reg, unsigned int *val)
{
	struct dw_i2c_dev *dev = context;

	*val = swab32(readl_relaxed(dev->base + reg));

	return 0;
}

static int dw_reg_write_swab(void *context, unsigned int reg, unsigned int val)
{
	struct dw_i2c_dev *dev = context;

	writel_relaxed(swab32(val), dev->base + reg);

	return 0;
}

static int dw_reg_read_word(void *context, unsigned int reg, unsigned int *val)
{
	struct dw_i2c_dev *dev = context;

	*val = readw_relaxed(dev->base + reg) |
		(readw_relaxed(dev->base + reg + 2) << 16);

	return 0;
}

static int dw_reg_write_word(void *context, unsigned int reg, unsigned int val)
{
	struct dw_i2c_dev *dev = context;

	writew_relaxed(val, dev->base + reg);
	writew_relaxed(val >> 16, dev->base + reg + 2);

	return 0;
}

/**
 * i2c_dw_init_regmap() - Initialize registers map
 * @dev: device private data
 *
 * Autodetects needed register access mode and creates the regmap with
 * corresponding read/write callbacks. This must be called before doing any
 * other register access.
 */
int i2c_dw_init_regmap(struct dw_i2c_dev *dev)
{
	struct regmap_config map_cfg = {
		.reg_bits = XR_I2C_REG_BITS,
		.val_bits = XR_I2C_VAL_BITS,
		.reg_stride = XR_I2C_REG_STRIDE,
		.disable_locking = true,
		.reg_read = dw_reg_read,
		.reg_write = dw_reg_write,
		.max_register = DW_IC_COMP_TYPE,
	};
	u32 reg;
	int ret;

	/*
	 * Skip detecting the registers map configuration if the regmap has
	 * already been provided by a higher code.
	 */
	if (dev->map)
		return 0;

	ret = i2c_dw_acquire_lock(dev);
	if (ret)
		return ret;

	reg = readl(dev->base + DW_IC_COMP_TYPE);
	i2c_dw_release_lock(dev);

	if (reg == swab32(DW_IC_COMP_TYPE_VALUE)) {
		map_cfg.reg_read = dw_reg_read_swab;
		map_cfg.reg_write = dw_reg_write_swab;
	} else if (reg == (DW_IC_COMP_TYPE_VALUE & XR_IC_COMP_TYPE_MASK_16)) {
		map_cfg.reg_read = dw_reg_read_word;
		map_cfg.reg_write = dw_reg_write_word;
	} else if (reg != DW_IC_COMP_TYPE_VALUE) {
		dev_err(dev->dev,
			"Unknown Synopsys component type: 0x%08x\n", reg);
		return -ENODEV;
	}

	/*
	 * Note we'll check the return value of the regmap IO accessors only
	 * at the probe stage. The rest of the code won't do this because
	 * basically we have MMIO-based regmap so non of the read/write methods
	 * can fail.
	 */
	dev->map = devm_regmap_init(dev->dev, NULL, dev, &map_cfg);
	if (IS_ERR(dev->map)) {
		dev_err(dev->dev, "Failed to init the registers map\n");
		return PTR_ERR(dev->map);
	}

	return 0;
}

static const u32 supported_speeds[] = {
	I2C_MAX_HIGH_SPEED_MODE_FREQ,
	I2C_MAX_FAST_MODE_PLUS_FREQ,
	I2C_MAX_FAST_MODE_FREQ,
	I2C_MAX_STANDARD_MODE_FREQ,
};

int i2c_dw_validate_speed(struct dw_i2c_dev *dev)
{
	struct i2c_timings *t = &dev->timings;
	unsigned int i;

	/*
	 * Only standard mode at 100kHz, fast mode at 400kHz,
	 * fast mode plus at 1MHz and high speed mode at 3.4MHz are supported.
	 */
	for (i = 0; i < ARRAY_SIZE(supported_speeds); i++) {
		if (t->bus_freq_hz == supported_speeds[i])
			return 0;
	}

	dev_err(dev->dev,
		"%d Hz is unsupported, only 100kHz, 400kHz, 1MHz and 3.4MHz are supported\n",
		t->bus_freq_hz);

	return -EINVAL;
}
EXPORT_SYMBOL_GPL(i2c_dw_validate_speed);

u32 i2c_dw_scl_hcnt(u32 ic_clk, u32 tSYMBOL, u32 tf, int cond, int offset)
{
	/*
	 * DesignWare I2C core doesn't seem to have solid strategy to meet
	 * the tHD;STA timing spec.  Configuring _HCNT based on tHIGH spec
	 * will result in violation of the tHD;STA spec.
	 */
	if (cond)
		/*
		 * Conditional expression:
		 *
		 *   IC_[FS]S_SCL_HCNT + (1+4+3) >= IC_CLK * tHIGH
		 *
		 * This is based on the DW manuals, and represents an ideal
		 * configuration.  The resulting I2C bus speed will be
		 * faster than any of the others.
		 *
		 * If your hardware is free from tHD;STA issue, try this one.
		 */
		return DIV_ROUND_CLOSEST(ic_clk * tSYMBOL, MICRO) - 8 + offset;

	/*
	 * Conditional expression:
	 *
	 *   IC_[FS]S_SCL_HCNT + 3 >= IC_CLK * (tHD;STA + tf)
	 *
	 * This is just experimental rule; the tHD;STA period turned
	 * out to be proportinal to (_HCNT + 3).  With this setting,
	 * we could meet both tHIGH and tHD;STA timing specs.
	 *
	 * If unsure, you'd better to take this alternative.
	 *
	 * The reason why we need to take into account "tf" here,
	 * is the same as described in i2c_dw_scl_lcnt().
	 */
	return DIV_ROUND_CLOSEST(ic_clk * (tSYMBOL + tf), MICRO) - 3 + offset;
}

u32 i2c_dw_scl_lcnt(u32 ic_clk, u32 tLOW, u32 tf, int offset)
{
	/*
	 * Conditional expression:
	 *
	 *   IC_[FS]S_SCL_LCNT + 1 >= IC_CLK * (tLOW + tf)
	 *
	 * DW I2C core starts counting the SCL CNTs for the LOW period
	 * of the SCL clock (tLOW) as soon as it pulls the SCL line.
	 * In order to meet the tLOW timing spec, we need to take into
	 * account the fall time of SCL signal (tf).  Default tf value
	 * should be 0.3 us, for safety.
	 */
	return DIV_ROUND_CLOSEST(ic_clk * (tLOW + tf), MICRO) - 1 + offset;
}

int i2c_dw_set_sda_hold(struct dw_i2c_dev *dev)
{
	u32 reg;
	int ret;

	ret = i2c_dw_acquire_lock(dev);
	if (ret)
		return ret;

	/* Configure SDA Hold Time if required */
	ret = regmap_read(dev->map, DW_IC_COMP_VERSION, &reg);
	if (ret)
		goto err_release_lock;

	if (reg >= DW_IC_SDA_HOLD_MIN_VERS) {
		if (!dev->sda_hold_time) {
			/* Keep previous hold time setting if no one set it */
			ret = regmap_read(dev->map, DW_IC_SDA_HOLD,
					  &dev->sda_hold_time);
			if (ret)
				goto err_release_lock;
		}

		/*
		 * Workaround for avoiding TX arbitration lost in case I2C
		 * slave pulls SDA down "too quickly" after falling edge of
		 * SCL by enabling non-zero SDA RX hold. Specification says it
		 * extends incoming SDA low to high transition while SCL is
		 * high but it appears to help also above issue.
		 */
		if (!(dev->sda_hold_time & DW_IC_SDA_HOLD_RX_MASK))
			dev->sda_hold_time |= 1 << DW_IC_SDA_HOLD_RX_SHIFT;

		dev_dbg(dev->dev, "SDA Hold Time TX:RX = %d:%d\n",
			dev->sda_hold_time & ~(u32)DW_IC_SDA_HOLD_RX_MASK,
			dev->sda_hold_time >> DW_IC_SDA_HOLD_RX_SHIFT);
	} else if (dev->set_sda_hold_time) {
		dev->set_sda_hold_time(dev);
	} else if (dev->sda_hold_time) {
		dev_warn(dev->dev,
			"Hardware too old to adjust SDA hold time.\n");
		dev->sda_hold_time = 0;
	}

err_release_lock:
	i2c_dw_release_lock(dev);

	return ret;
}

void __i2c_dw_disable(struct dw_i2c_dev *dev)
{
	int timeout = XR_I2C_DISABLE_TIMEOUT;
	u32 status;

	do {
		__i2c_dw_disable_nowait(dev);
		/*
		 * The enable status register may be unimplemented, but
		 * in that case this test reads zero and exits the loop.
		 */
		regmap_read(dev->map, DW_IC_ENABLE_STATUS, &status);
		if (XR_I2C_STATUS_NOT_ENABLE(status))
			return;

		/*
		 * Wait 10 times the signaling period of the highest I2C
		 * transfer supported by the driver (for 400KHz this is
		 * 25us) as described in the DesignWare I2C databook.
		 */
		usleep_range(XR_I2C_DISABLE_USLEEP_RANGE_MIN, XR_I2C_DISABLE_USLEEP_RANGE_MAX);
	} while (timeout--);

	dev_warn(dev->dev, "timeout in disabling adapter\n");
}

unsigned long i2c_dw_clk_rate(struct dw_i2c_dev *dev)
{
	/*
	 * Clock is not necessary if we got LCNT/HCNT values directly from
	 * the platform code.
	 */
	if (WARN_ON_ONCE(!dev->get_clk_rate_khz))
		return 0;
	return dev->get_clk_rate_khz(dev);
}

int i2c_dw_prepare_clk(struct dw_i2c_dev *dev, u8 flag)
{
	int ret = 0;

	/**
	 * Optional interface clock.
	 * clk_prepare_enable <---> clk_disable_unprepare
	 */
	switch (flag) {
	case I2C_CLK_PREPARE_ENABLE:
		ret = clk_prepare_enable(dev->pclk);
		if (ret)
			return ret;

		ret = clk_prepare_enable(dev->clk);
		if (ret)
			clk_disable_unprepare(dev->pclk);

		return ret;
	case I2C_CLK_DISABLE_UNPREPARE:
		clk_disable_unprepare(dev->clk);
		clk_disable_unprepare(dev->pclk);
		return 0;
	case I2C_CLK_EBABLE:
		ret = clk_enable(dev->pclk);
		if (ret)
			return ret;

		ret = clk_enable(dev->clk);
		if (ret)
			clk_disable(dev->pclk);

		return ret;
	case I2C_CLK_DISABLE:
		clk_disable(dev->clk);
		clk_disable(dev->pclk);
		return 0;
	default:
		dev_err(dev->dev, "invalid clk cmd.\n");
		ret = -EINVAL;
		break;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(i2c_dw_prepare_clk);

int i2c_dw_acquire_lock(struct dw_i2c_dev *dev)
{
	int ret;

	if (!dev->acquire_lock)
		return 0;

	ret = dev->acquire_lock();
	if (!ret)
		return 0;

	dev_err(dev->dev, "couldn't acquire bus ownership\n");

	return ret;
}

void i2c_dw_release_lock(struct dw_i2c_dev *dev)
{
	if (dev->release_lock)
		dev->release_lock();
}

/*
 * Waiting for bus not busy
 */
int i2c_dw_wait_bus_not_busy(struct dw_i2c_dev *dev)
{
	u32 status;
	int ret;

	ret = regmap_read_poll_timeout(dev->map, DW_IC_STATUS, status,
				       !(status & DW_IC_STATUS_ACTIVITY),
				       XR_WAIT_NOT_BUSY_TIMEOUT_RANGE_MIN,
				       XR_WAIT_NOT_BUSY_TIMEOUT_RANGE_MAX);
	if (ret) {
		dev_warn(dev->dev, "timeout waiting for bus ready\n");

		i2c_recover_bus(&dev->adapter);

		regmap_read(dev->map, DW_IC_STATUS, &status);
		if (!(status & DW_IC_STATUS_ACTIVITY))
			ret = 0;
	}

	return ret;
}

int i2c_dw_handle_tx_abort(struct dw_i2c_dev *dev)
{
	unsigned long abort_source = dev->abort_source;
	int i;

	if (abort_source & DW_IC_TX_ABRT_NOACK) {
		for_each_set_bit(i, &abort_source, ARRAY_SIZE(abort_sources))
			dev_err(dev->dev,
				"%s: %s\n", __func__, abort_sources[i]);
		return -EREMOTEIO;
	}

	for_each_set_bit(i, &abort_source, ARRAY_SIZE(abort_sources))
		dev_err(dev->dev, "%s: %s\n", __func__, abort_sources[i]);

	if (abort_source & DW_IC_TX_ARB_LOST)
		return -EAGAIN;
	else if (abort_source & DW_IC_TX_ABRT_GCALL_READ)
		return -EINVAL; /* wrong msgs[] data */
	else
		return -EIO;
}

int i2c_dw_set_fifo_size(struct dw_i2c_dev *dev)
{
	u32 param;
	int ret;

	/**
	 * Get FIFO depth, O1 SoC is 64.
	 * Read value from IC_COMP_PARAM_1 is 63, Need to add 1.
	 * For details, refer to DW_apb_i2c 2.03a databook, Section 5.1.66.
	 */
	ret = regmap_read(dev->map, DW_IC_COMP_PARAM_1, &param);
	if (ret)
		return ret;

	dev->tx_fifo_depth = PARAM_TO_FIFO(param >> XR_TX_FIFO_DEPTH_REG_OPS);
	dev->rx_fifo_depth = PARAM_TO_FIFO(param >> XR_RX_FIFO_DEPTH_REG_OPS);

	return 0;
}

u32 i2c_dw_func(struct i2c_adapter *adap)
{
	struct dw_i2c_dev *dev = i2c_get_adapdata(adap);

	return dev->functionality;
}

void i2c_dw_disable(struct dw_i2c_dev *dev)
{
	u32 dummy;

	/* Disable controller */
	__i2c_dw_disable(dev);

	/* Disable all interrupts */
	regmap_write(dev->map, DW_IC_INTR_MASK, 0);
	regmap_read(dev->map, DW_IC_CLR_INTR, &dummy);
}

void i2c_dw_disable_int(struct dw_i2c_dev *dev)
{
	regmap_write(dev->map, DW_IC_INTR_MASK, 0);
}

int i2c_hardware_spinlock(struct dw_i2c_dev *dev)
{
	int ret = 0;
	struct hwspinlock *hwlock = dev->hwspin_lock;
	unsigned long time, timeout;
	uint32_t cur_mid_r = 0;
	int lock_id = 0;

	if (!hwlock) {
		dev_err(dev->dev, "Invalid hardware spinlock\n");
		return -ENODEV;
	}

	timeout = jiffies + msecs_to_jiffies(GET_HARDWARE_LOCK_TIMEOUT);

	do {
		ret = hwspin_trylock_raw(hwlock);
		if (ret == 0)  /* 0: success */
			break;

		time = jiffies;
		if (time_after(time, timeout)) {
			lock_id = hwspin_lock_get_id(hwlock);
			ret = xr_get_hwspinlock_mid(lock_id, &cur_mid_r);
			if (ret) {
				dev_err(dev->dev, "Get hwspinlock(%d) mid failed\n",
					lock_id);
				return -EINVAL;
			}
			dev_err(dev->dev, "Get hwspinlock(%d) timeout, used by %d\n",
				lock_id, cur_mid_r);
			return -EBUSY;
		}
		/* sleep 1000~2000 us */
		usleep_range(1000, 2000);
	} while (ret < 0);

	return ret;
}
