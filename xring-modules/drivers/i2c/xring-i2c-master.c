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

#include <soc/xring/securelib/securec.h>
#include <soc/xring/securelib/securectype.h>

#include "i2c-designware-core.h"

#define STOP		0x200  /* BIT(9) */
#define RESTART		0x400  /* BIT(10) */
#define READ_CMD	0x100  /* BIT(8) */

#define PRIOR_MSG(x)	((x) - 1)
#define LAST_BYTE(x)	((x) - 1)

/**
 * XRing I2C DMA
 */
#if defined CONFIG_XRING_I2C_DMA
static int config_dma_tx_chan(struct dw_i2c_dev *dev)
{
	struct xr_exteranl_dma *i2c_dma = dev->exteranl_dma;
	int ret;

	i2c_dma->tx_config.dst_addr = dev->fifo_dma_addr;
	i2c_dma->tx_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	i2c_dma->tx_config.direction = DMA_MEM_TO_DEV;
	ret = dmaengine_slave_config(i2c_dma->tx_fifo_chan, &i2c_dma->tx_config);

	return ret;
}

static int config_dma_rx_chan(struct dw_i2c_dev *dev)
{
	struct xr_exteranl_dma *i2c_dma = dev->exteranl_dma;
	int ret;

	i2c_dma->rx_config.src_addr = dev->fifo_dma_addr;
	i2c_dma->rx_config.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
	i2c_dma->rx_config.direction = DMA_DEV_TO_MEM;
	ret = dmaengine_slave_config(i2c_dma->rx_fifo_chan, &i2c_dma->rx_config);

	return ret;
}

int xr_i2c_dma_init(struct dw_i2c_dev *dev)
{
	struct xr_exteranl_dma *i2c_dma = NULL;
	int ret;
	int chan_count;

	if (!dev) {
		pr_err("%s, invalid parameter.\n", __func__);
		return -EINVAL;
	}

	chan_count = of_property_count_strings(dev->dev->of_node, "dma-names");
	if (chan_count < 0)
		return chan_count;

	i2c_dma = devm_kzalloc(dev->dev, sizeof(struct xr_exteranl_dma), GFP_KERNEL);
	if (!i2c_dma)
		return -ENOMEM;

	dev->exteranl_dma = i2c_dma;

	i2c_dma->rx_fifo_chan = dma_request_chan(dev->dev, "rx");
	if (IS_ERR(i2c_dma->rx_fifo_chan)) {
		ret = PTR_ERR(i2c_dma->rx_fifo_chan);
		dev_warn(dev->dev, "can't get the rx channel, %d\n", ret);
		i2c_dma->rx_fifo_chan = NULL;
		return ret;
	}

	i2c_dma->tx_fifo_chan = dma_request_chan(dev->dev, "tx");
	if (IS_ERR(i2c_dma->tx_fifo_chan)) {
		ret = PTR_ERR(i2c_dma->tx_fifo_chan);
		dev_warn(dev->dev, "can't get the tx channel, %d\n", ret);
		goto i2c_ch_request_error;
	}

	ret = config_dma_rx_chan(dev);
	if (ret < 0) {
		dev_err(dev->dev, "can't configure rx channel (%d)\n", ret);
		goto i2c_config_error;
	}

	ret = config_dma_tx_chan(dev);
	if (ret < 0) {
		dev_err(dev->dev, "can't configure tx channel (%d)\n", ret);
		goto i2c_config_error;
	}

	dev->ip_has_dma = IP_HAS_DMA;

	return 0;

i2c_config_error:
	dma_release_channel(i2c_dma->tx_fifo_chan);

i2c_ch_request_error:
	dma_release_channel(i2c_dma->rx_fifo_chan);
	i2c_dma->rx_fifo_chan = NULL;
	i2c_dma->tx_fifo_chan = NULL;

	return ret;
}

void xr_i2c_release_dma(struct dw_i2c_dev *dev)
{
	struct xr_exteranl_dma *i2c_dma = NULL;

	if (!dev) {
		pr_err("%s, invalid parameter.\n", __func__);
		return;
	}

	if (!dev->exteranl_dma)
		return;

	i2c_dma = dev->exteranl_dma;
	if (i2c_dma->tx_fifo_chan) {
		dma_release_channel(i2c_dma->tx_fifo_chan);
		i2c_dma->tx_fifo_chan = NULL;
	}

	if (i2c_dma->rx_fifo_chan) {
		dma_release_channel(i2c_dma->rx_fifo_chan);
		i2c_dma->rx_fifo_chan = NULL;
	}
}

static void dma_tx_callback(void *data)
{
	struct dw_i2c_dev *dev = (struct dw_i2c_dev *)data;

	dev->xr_i2c_dma_en_cr &= ~XR_IC_DMA_CR_TDMAE;
	regmap_write(dev->map, XR_IC_DMA_CR, dev->xr_i2c_dma_en_cr);

	if (!(dev->xr_i2c_dma_en_cr & XR_IC_DMA_CR_RDMAE))
		complete(&dev->dma_complete);
}

static void dma_rx_callback(void *data)
{
	struct dw_i2c_dev *dev = (struct dw_i2c_dev *)data;

	dev->xr_i2c_dma_en_cr &= ~XR_IC_DMA_CR_RDMAE;
	regmap_write(dev->map, XR_IC_DMA_CR, dev->xr_i2c_dma_en_cr);

	if (!(dev->xr_i2c_dma_en_cr & XR_IC_DMA_CR_TDMAE))
		complete(&dev->dma_complete);
}

static void dma_xfer_init(struct dw_i2c_dev *dev, u32 dma_en_mask)
{
	struct xr_exteranl_dma *i2c_dma = dev->exteranl_dma;
	struct i2c_msg *msgs = dev->msgs;
	u32 ic_con = 0;
	u32 ic_tar = 0;
	u32 dummy;
	u32 intr_mask;

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

		regmap_update_bits(dev->map, DW_IC_CON, DW_IC_CON_10BITADDR_MASTER,
				ic_con);
	}

	/*
	 * Set the slave (target) address and enable 10-bit addressing mode
	 * if applicable.
	 */
	regmap_write(dev->map, DW_IC_TAR, msgs[dev->msg_write_idx].addr | ic_tar);

	/* Enforce disabled interrupts (due to HW issues) */
	i2c_dw_disable_int(dev);

	/* Dummy read to avoid the register getting stuck on Bay Trail */
	regmap_read(dev->map, DW_IC_ENABLE_STATUS, &dummy);

	/* Clear and enable interrupts */
	regmap_read(dev->map, DW_IC_CLR_INTR, &dummy);

	if (i2c_dma->using_rx_dma == USE_DMA)
		intr_mask = XR_DMA_INTR_MASK;
	else
		intr_mask = XR_DMA_INTR_MASK | DW_IC_INTR_RX_FULL;

	regmap_write(dev->map, DW_IC_INTR_MASK, intr_mask);

	dev->xr_i2c_dma_en_cr = dma_en_mask;
	regmap_write(dev->map, XR_IC_DMA_CR, dma_en_mask);

	/* Enable the adapter */
	__i2c_dw_enable(dev);
}

static int free_i2c_dma_source(struct dw_i2c_dev *dev)
{
	u32 i;
	struct xr_exteranl_dma *i2c_dma = dev->exteranl_dma;

	if (i2c_dma->dma_rx_msg_idx == 0)
		return 0;

	for (i = 0; i < i2c_dma->dma_rx_msg_idx; i++) {
		dma_unmap_single(dev->dev,
			sg_dma_address(&i2c_dma->rx_data_sg[i]),
			sg_dma_len(&i2c_dma->rx_data_sg[i]),
			DMA_FROM_DEVICE);
		kfree(i2c_dma->dma_rx_msgs[i].dma_buf);
	}

	i2c_dma->dma_rx_msg_idx = 0;

	return 0;
}

int xr_dma_xfer(struct dw_i2c_dev *dev)
{
	struct xr_exteranl_dma *i2c_dma = NULL;
	dma_addr_t buf_dma_addr;
	u32 dma_en_mask;
	dma_cookie_t rx_cookie;
	dma_cookie_t tx_cookie;
	int ret;
	int i;
	int j;
	int buf_index;
	u16 cmd;
	int dir_flag;

	if (!dev) {
		pr_err("%s, invalid parameter.\n", __func__);
		return -EINVAL;
	}

	if (!dev->exteranl_dma) {
		dev_err(dev->dev, "this i2c does not support dma.\n");
		return -EINVAL;
	}

	i2c_dma = dev->exteranl_dma;

	if (dev->rx_msgs_num > I2C_DMA_MAX_MSGS) {
		dev_err(dev->dev, "not support, max msgs is:%u\n", I2C_DMA_MAX_MSGS);
		return -USE_DMA;
	}

	reinit_completion(&dev->dma_complete);

	ret = config_dma_rx_chan(dev);
	if (ret < 0) {
		dev_err(dev->dev, "can't configure rx channel (%d)\n", ret);
		return ret;
	}

	ret = config_dma_tx_chan(dev);
	if (ret < 0) {
		dev_err(dev->dev, "can't configure tx channel (%d)\n", ret);
		return ret;
	}

	i2c_dma->dma_tx_msg_idx = 0;
	i2c_dma->dma_rx_msg_idx = 0;
	i2c_dma->using_rx_dma = !USE_DMA;
	i2c_dma->dma_tx_buf = NULL;

	dev->cmd_dma_buf_size = CMD_DMA_WIDTH_2BYTE(dev->cmd_total_len);
	i2c_dma->dma_tx_buf = kzalloc(dev->cmd_dma_buf_size, GFP_KERNEL);
	if (!i2c_dma->dma_tx_buf)
		return -ENOMEM;

	/**
	 * Xring O1 SOC
	 * restart is generated only when the transmission direction changes
	 */
	buf_index = 0;
	for (i = 0; i < dev->msgs_num; i++) {
		cmd = 0;
		if (dev->msgs[i].flags & I2C_M_RD)
			cmd = READ_CMD;

		/* need restart */
		if (i > 0) {
			dir_flag = dev->msgs[i].flags & I2C_M_RD -
					dev->msgs[PRIOR_MSG(i)].flags & I2C_M_RD;
			if (!!dir_flag)
				cmd |= RESTART;
		}

		for (j = 0; j < dev->msgs[i].len; j++) {
			i2c_dma->dma_tx_buf[buf_index] = dev->msgs[i].buf[j] | cmd;
			buf_index++;
			cmd &= ~(RESTART);
		}
	}

	/* last byte, stop */
	i2c_dma->dma_tx_buf[LAST_BYTE(buf_index)] |= STOP;

	buf_dma_addr = dma_map_single(dev->dev,
					(void *)i2c_dma->dma_tx_buf,
					dev->cmd_dma_buf_size,
					DMA_TO_DEVICE);
	if (dma_mapping_error(dev->dev, buf_dma_addr)) {
		dev_err(dev->dev, "tx cmd/data buf, DMA mapping failed\n");
		goto cmd_map_error;
	}
	sg_dma_address(&i2c_dma->tx_data_cmd_sg) = buf_dma_addr;
	sg_dma_len(&i2c_dma->tx_data_cmd_sg) = dev->cmd_dma_buf_size;
	i2c_dma->dma_tx_msg_idx++;

	dev_dbg(dev->dev, "%d, dma tx_msgs:%u, dma addr:0x%llx, dma buf len:%u\n",
			i, i2c_dma->dma_tx_msg_idx, buf_dma_addr, dev->cmd_dma_buf_size);

	for (i = 0; i < dev->msgs_num; i++) {
		if (!(dev->msgs[i].flags & I2C_M_RD))
			continue;

		if (dev->msgs[i].len < DMA_THRESHOLD) {
			free_i2c_dma_source(dev);
			break;
		}

		i2c_dma->dma_rx_msgs[i2c_dma->dma_rx_msg_idx].dma_buf = kzalloc(dev->msgs[i].len, GFP_KERNEL);
		if (!i2c_dma->dma_rx_msgs[i2c_dma->dma_rx_msg_idx].dma_buf)
			goto rx_buf_map_error;

		i2c_dma->dma_rx_msgs[i2c_dma->dma_rx_msg_idx].index = i; /* records the subscript of rx_msg */

		buf_dma_addr = dma_map_single(dev->dev,
						i2c_dma->dma_rx_msgs[i2c_dma->dma_rx_msg_idx].dma_buf,
						dev->msgs[i].len,
						DMA_FROM_DEVICE);
		if (dma_mapping_error(dev->dev, buf_dma_addr)) {
			dev_err(dev->dev, "rx data buf, DMA mapping failed\n");
			goto rx_buf_map_error;
		}

		sg_dma_address(&i2c_dma->rx_data_sg[i2c_dma->dma_rx_msg_idx]) = buf_dma_addr;
		sg_dma_len(&i2c_dma->rx_data_sg[i2c_dma->dma_rx_msg_idx]) = dev->msgs[i].len;
		i2c_dma->dma_rx_msg_idx++;

		dev_dbg(dev->dev, "%d, rx_msgs:%u, dma addr:0x%llx, msg buf len:%u\n",
			i, i2c_dma->dma_rx_msg_idx, buf_dma_addr, dev->msgs[i].len);
	}

	i2c_dma->tx_desc = dmaengine_prep_slave_sg(
					i2c_dma->tx_fifo_chan,
					&i2c_dma->tx_data_cmd_sg,
					i2c_dma->dma_tx_msg_idx,
					DMA_MEM_TO_DEV,
					DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!i2c_dma->tx_desc) {
		dev_err(dev->dev, "Not able to get desc for DMA xfer, TX\n");
		goto tx_pre_error;
	}
	i2c_dma->tx_desc->callback = dma_tx_callback;
	i2c_dma->tx_desc->callback_param = (void *)dev;

	tx_cookie = dmaengine_submit(i2c_dma->tx_desc);
	ret = dma_submit_error(tx_cookie);
	if (ret) {
		dmaengine_terminate_sync(i2c_dma->tx_fifo_chan);
		dev_err(dev->dev, "dmaengine_submit err, TX\n");
		goto tx_pre_error;
	}
	dev->using_dma = USE_DMA;
	dma_en_mask = XR_IC_DMA_CR_TDMAE;

	dev_dbg(dev->dev, "i2c tx_desc dmaengine_submit OK, using dma:%u\n", dev->using_dma);

	if (i2c_dma->dma_rx_msg_idx != 0) {
		i2c_dma->rx_desc = dmaengine_prep_slave_sg(
						i2c_dma->rx_fifo_chan,
						i2c_dma->rx_data_sg,
						i2c_dma->dma_rx_msg_idx,
						DMA_DEV_TO_MEM,
						DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
		if (!i2c_dma->rx_desc) {
			dev_err(dev->dev, "Not able to get desc for DMA xfer, RX\n");
			goto rx_pre_error;
		}
		i2c_dma->rx_desc->callback = dma_rx_callback;
		i2c_dma->rx_desc->callback_param = dev;

		rx_cookie = dmaengine_submit(i2c_dma->rx_desc);
		ret = dma_submit_error(rx_cookie);
		if (ret) {
			dmaengine_terminate_sync(i2c_dma->rx_fifo_chan);
			dev_err(dev->dev, "dmaengine_submit err, RX\n");
			goto rx_pre_error;
		}

		i2c_dma->using_rx_dma = USE_DMA;
		dma_en_mask = XR_IC_DMA_CR_RDMAE | XR_IC_DMA_CR_TDMAE;

		dev_dbg(dev->dev, "i2c rx_desc dmaengine_submit OK, using dma:%u, rx dma:%u\n",
					dev->using_dma, i2c_dma->using_rx_dma);
	}

	if (i2c_dma->using_rx_dma == USE_DMA)
		dma_async_issue_pending(i2c_dma->rx_fifo_chan);
	dma_async_issue_pending(i2c_dma->tx_fifo_chan);

	dma_xfer_init(dev, dma_en_mask);

	return 0;

rx_pre_error:
	dev->using_dma = !USE_DMA;
	i2c_dma->using_rx_dma = !USE_DMA;
	i2c_dma->rx_desc = NULL;
	rx_cookie = 0;
tx_pre_error:
	i2c_dma->tx_desc = NULL;
	tx_cookie = 0;

rx_buf_map_error:
	free_i2c_dma_source(dev);

	dma_unmap_single(dev->dev,
			sg_dma_address(&i2c_dma->tx_data_cmd_sg),
			sg_dma_len(&i2c_dma->tx_data_cmd_sg),
			DMA_TO_DEVICE);

cmd_map_error:
	kfree(i2c_dma->dma_tx_buf);
	i2c_dma->dma_tx_buf = NULL;

	return -USE_DMA;
}

int i2c_dma_unmap(struct dw_i2c_dev *dev)
{
	struct xr_exteranl_dma *i2c_dma = NULL;
	int i;
	u32 rx_msg_index;

	if (!dev) {
		pr_err("%s, invalid parameter.\n", __func__);
		return -EINVAL;
	}

	if (!dev->exteranl_dma) {
		dev_err(dev->dev, "this i2c does not support dma.\n");
		return -EINVAL;
	}
	i2c_dma = dev->exteranl_dma;

	if (dev->xr_i2c_dma_en_cr) {
		dev->xr_i2c_dma_en_cr &= (~XR_IC_DMA_CR_RDMAE) | (~XR_IC_DMA_CR_TDMAE);
		regmap_write(dev->map, XR_IC_DMA_CR, dev->xr_i2c_dma_en_cr);
	}

	dev->using_dma = !USE_DMA;
	i2c_dma->using_rx_dma = !USE_DMA;

	if (i2c_dma->dma_tx_buf) {
		dma_unmap_single(dev->dev,
				sg_dma_address(&i2c_dma->tx_data_cmd_sg),
				sg_dma_len(&i2c_dma->tx_data_cmd_sg),
				DMA_TO_DEVICE);

		kfree(i2c_dma->dma_tx_buf);
		i2c_dma->dma_tx_buf = NULL;
	}

	if (i2c_dma->dma_rx_msg_idx == 0) {
		dev_err(dev->dev, "i2c current transaction, just use tx dma.\n");
		return 0;
	}

	for (i = 0; i < i2c_dma->dma_rx_msg_idx; i++) {
		dma_unmap_single(dev->dev,
				sg_dma_address(&i2c_dma->rx_data_sg[i]),
				sg_dma_len(&i2c_dma->rx_data_sg[i]),
				DMA_FROM_DEVICE);

		rx_msg_index = i2c_dma->dma_rx_msgs[i].index;

		if (dev->msgs[rx_msg_index].flags & I2C_M_RD) {
			memcpy(dev->msgs[rx_msg_index].buf,  /* slave alloc rx msg buf */
				i2c_dma->dma_rx_msgs[i].dma_buf, /* i2c driver alloc rx msg buf */
				dev->msgs[rx_msg_index].len);
			kfree(i2c_dma->dma_rx_msgs[i].dma_buf);
		}
	}

	return 0;
}

int terminate_dma_transfer(struct dw_i2c_dev *dev)
{
	if (!dev) {
		pr_err("%s, invalid parameter.\n", __func__);
		return -EINVAL;
	}

	if (dev->using_dma == USE_DMA) {
		dmaengine_terminate_sync(dev->exteranl_dma->tx_fifo_chan);
		if (dev->exteranl_dma->using_rx_dma == USE_DMA)
			dmaengine_terminate_sync(dev->exteranl_dma->rx_fifo_chan);
	}

	return 0;
}

#endif
