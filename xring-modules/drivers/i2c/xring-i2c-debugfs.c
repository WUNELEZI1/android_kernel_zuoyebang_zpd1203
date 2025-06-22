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


#if defined CONFIG_XRING_I2C_DEBUGFS
static int xr_i2c_intr_show(struct seq_file *s, void *p)
{
	struct dw_i2c_dev *dev = NULL;
	u32 tmp;
	int ret;

	dev = s->private;
	if (dev == NULL)
		return -EINVAL;

	xr_i2c_runtime_resume(dev->dev);

	ret = i2c_dw_acquire_lock(dev);
	if (ret)
		return -EINVAL;

	regmap_read(dev->map, DW_IC_INTR_MASK, &tmp);
	regmap_read(dev->map, DW_IC_RAW_INTR_STAT, &tmp);

	i2c_dw_release_lock(dev);

	xr_i2c_runtime_suspend(dev->dev);

	seq_printf(s, "DW_IC_INTR_MASK = \t0x%x\n", tmp);
	seq_printf(s, "DW_IC_RAW_INTR_STAT = \t0x%x\n", tmp);

	seq_printf(s, "i2c TX EMPTY Number of interrupts:\t %u\n", dev->tx_cmd_intr_count);
	seq_printf(s, "i2c RX FULL Number of interrupts:\t %u\n", dev->rx_data_intr_count);

	seq_printf(s, "dev->cmd_err:\t 0x%x\n", dev->cmd_err);
	seq_printf(s, "dev->status:\t %d\n", dev->status);
	seq_printf(s, "prev dev->intr_status:\t 0x%x\n", dev->intr_status);
	seq_printf(s, "prev ev->abort_source:\t 0x%x\n", dev->abort_source);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(xr_i2c_intr);

static int xr_i2c_dump_registers(struct dw_i2c_dev *dev, struct seq_file *s)
{
	u32 comp_param1;

	regmap_read(dev->map, DW_IC_ENABLE_STATUS, &comp_param1);
	seq_printf(s, "DW_IC_ENABLE_STATUS \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_ENABLE, &comp_param1);
	seq_printf(s, "DW_IC_ENABLE \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_CON, &comp_param1);
	seq_printf(s, "DW_IC_CON \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_TAR, &comp_param1);
	seq_printf(s, "DW_IC_TAR \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_SS_SCL_HCNT, &comp_param1);
	seq_printf(s, "DW_IC_SS_SCL_HCNT \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_SS_SCL_LCNT, &comp_param1);
	seq_printf(s, "DW_IC_SS_SCL_LCNT \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_FS_SCL_HCNT, &comp_param1);
	seq_printf(s, "DW_IC_FS_SCL_HCNT \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_FS_SCL_LCNT, &comp_param1);
	seq_printf(s, "DW_IC_FS_SCL_LCNT \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_HS_SCL_HCNT, &comp_param1);
	seq_printf(s, "DW_IC_HS_SCL_HCNT \t = 0x%x\n", comp_param1);
	regmap_read(dev->map, DW_IC_HS_SCL_LCNT, &comp_param1);
	seq_printf(s, "DW_IC_HS_SCL_LCNT \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, IC_SCL_STUCK_AT_LOW_TIMEOUT, &comp_param1);
	seq_printf(s, "IC_SCL_STUCK_AT_LOW_TIMEOUT \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_INTR_MASK, &comp_param1);
	seq_printf(s, "DW_IC_INTR_MASK \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_RAW_INTR_STAT, &comp_param1);
	seq_printf(s, "DW_IC_RAW_INTR_STAT \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_INTR_STAT, &comp_param1);
	seq_printf(s, "DW_IC_INTR_STAT \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_RX_TL, &comp_param1);
	seq_printf(s, "DW_IC_RX_TL \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_TX_TL, &comp_param1);
	seq_printf(s, "DW_IC_TX_TL \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_STATUS, &comp_param1);
	seq_printf(s, "DW_IC_STATUS \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_TXFLR, &comp_param1);
	seq_printf(s, "DW_IC_TXFLR \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_RXFLR, &comp_param1);
	seq_printf(s, "DW_IC_RXFLR \t = 0x%x\n", comp_param1);

#if defined CONFIG_XRING_I2C_DMA
	regmap_read(dev->map, XR_IC_DMA_TDLR, &comp_param1);
	seq_printf(s, "XR_IC_DMA_TDLR \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, XR_IC_DMA_RDLR, &comp_param1);
	seq_printf(s, "XR_IC_DMA_RDLR \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, XR_IC_DMA_CR, &comp_param1);
	seq_printf(s, "XR_IC_DMA_CR \t = 0x%x\n", comp_param1);
#endif

	regmap_read(dev->map, DW_IC_COMP_PARAM_1, &comp_param1);
	seq_printf(s, "DW_IC_COMP_PARAM_1 \t = 0x%x\n", comp_param1);

	regmap_read(dev->map, DW_IC_TX_ABRT_SOURCE, &comp_param1);
	seq_printf(s, "DW_IC_TX_ABRT_SOURCE \t = 0x%x\n", comp_param1);

	return 0;
}

static int xr_i2c_dump_reg_show(struct seq_file *s, void *p)
{
	struct dw_i2c_dev *dev = NULL;
	int ret;

	dev = s->private;
	if (dev == NULL)
		return -EINVAL;

	xr_i2c_runtime_resume(dev->dev);

	ret = i2c_dw_acquire_lock(dev);
	if (ret)
		return -EINVAL;

	xr_i2c_dump_registers(dev, s);

	i2c_dw_release_lock(dev);

	xr_i2c_runtime_suspend(dev->dev);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(xr_i2c_dump_reg);

static int xr_i2c_dump_timing_show(struct seq_file *s, void *p)
{
	struct dw_i2c_dev *dev = NULL;
	struct i2c_timings *timing;

	dev = s->private;
	if (dev == NULL)
		return -EINVAL;

	timing = &dev->timings;

	seq_printf(s, "ic_clk   = %lu KHz\n", i2c_dw_clk_rate(dev));
	seq_printf(s, "bus_freq = %u Hz\n", timing->bus_freq_hz);
	seq_printf(s, "scl_fall = %u ns\n", timing->scl_fall_ns);
	seq_printf(s, "sda_fall = %u ns\n", timing->sda_fall_ns);

	return 0;
};
DEFINE_SHOW_ATTRIBUTE(xr_i2c_dump_timing);

int xr_i2c_debugfs_init(struct dw_i2c_dev *dev)
{
	char node_name[DEBUGFS_CHAN_NAME_LEN];

	scnprintf(node_name, sizeof(node_name), "xring-i2c-%d", dev->adapter.nr);
	dev->xr_i2c_dentry = debugfs_create_dir(node_name, dev->xr_i2c_dentry);
	if (!dev->xr_i2c_dentry) {
		pr_err("xring-i2c is NULL, debugfs init failed.");
		return -EINVAL;
	}

	debugfs_create_file("reg", DEBUGFS_FILE_MASK,
			dev->xr_i2c_dentry, dev, &xr_i2c_dump_reg_fops);

	debugfs_create_file("timing", DEBUGFS_FILE_MASK,
			dev->xr_i2c_dentry, dev, &xr_i2c_dump_timing_fops);

	debugfs_create_file("intr", DEBUGFS_FILE_MASK,
			dev->xr_i2c_dentry, dev, &xr_i2c_intr_fops);

	return 0;
}

void xr_i2c_cleanup_debugfs(struct dw_i2c_dev *dev)
{
	debugfs_remove_recursive(dev->xr_i2c_dentry);
}
#endif
