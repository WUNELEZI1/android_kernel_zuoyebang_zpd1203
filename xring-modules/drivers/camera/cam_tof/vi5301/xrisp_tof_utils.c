// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "tof_vi5301_utils", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "tof_vi5301_utils", __func__, __LINE__

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "xrisp_tof.h"
#include "xrisp_log.h"

static int tof_i2c_write(struct xrisp_tof_data *dev, uint8_t reg, uint8_t *data, uint8_t len)
{
	int ret = 0;
	uint8_t *addr_buf;
	struct i2c_msg msg;
	struct i2c_client *client = dev->client;

	if (!client)
		return -EINVAL;
	addr_buf = kmalloc(len + 1, GFP_KERNEL);
	if (!addr_buf)
		return -ENOMEM;
	addr_buf[0] = reg;
	memcpy(&addr_buf[1], data, len);
	msg.addr = client->addr;
	msg.flags = 0;
	msg.buf = addr_buf;
	msg.len = len+1;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret != 1)
		XRISP_PR_ERROR("%s: i2c_transfer err:%d, addr:0x%x, reg:0x%x\n", __func__, ret,
			       client->addr, reg);

	kfree(addr_buf);
	return ret < 0 ? ret : (ret != 1 ? -EIO : 0);
}

static int tof_i2c_read(struct xrisp_tof_data *dev, uint8_t reg, uint8_t *data, uint8_t len)
{
	int ret = 0;
	struct i2c_msg msg[2];
	struct i2c_client *client = dev->client;

	if (!client)
		return -EINVAL;
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].buf = &reg;
	msg[0].len = 1;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = data;
	msg[1].len = len;
	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret != 2)
		XRISP_PR_ERROR("%s: i2c_transfer err:%d, addr:0x%x, reg:0x%x\n", __func__, ret,
			       client->addr, reg);

	return ret < 0 ? ret : (ret != 2 ? -EIO : 0);
}

int xrisp_tof_write_byte(struct xrisp_tof_data *dev, uint8_t reg, uint8_t data)
{
	return tof_i2c_write(dev, reg, &data, 1);
}

int xrisp_tof_read_byte(struct xrisp_tof_data *dev, uint8_t reg, uint8_t *data)
{
	return tof_i2c_read(dev, reg, data, 1);
}

int xrisp_tof_write_multibytes(struct xrisp_tof_data *dev, uint8_t reg,
		uint8_t *data, int32_t count)
{
	return tof_i2c_write(dev, reg, data, count);
}

int xrisp_tof_read_multibytes(struct xrisp_tof_data *dev, uint8_t reg,
		uint8_t *data, int32_t count)
{
	return tof_i2c_read(dev, reg, data, count);
}

int xrisp_tof_write_reg_offset(struct xrisp_tof_data *dev, uint8_t reg,
		uint8_t offset, uint8_t data)
{
	return xrisp_tof_write_byte(dev, reg + offset, data);
}

int xrisp_tof_read_reg_offset(struct xrisp_tof_data *dev, uint8_t reg,
		uint8_t offset, uint8_t *data)
{
	return xrisp_tof_read_byte(dev, reg + offset, data);
}
