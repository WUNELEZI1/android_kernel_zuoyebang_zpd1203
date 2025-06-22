// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "tof_stmvl53l8_utils", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "tof_stmvl53l8_utils", __func__, __LINE__

#include <linux/spi/spi.h>
#include <linux/delay.h>

#include "xrisp_tof.h"

#define XRISP_TOF_BLOCK_SIZE 1024
#define SPI_READWRITE_BIT 0x8000
#define SPI_WRITE_MASK(x) (x | SPI_READWRITE_BIT)
#define SPI_READ_MASK(x)  (x & ~SPI_READWRITE_BIT)

#ifdef XRISP_TOF_SPI_MODE
static int tof_spi_write(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
			uint8_t *data, uint16_t len, uint32_t speed_hz)
{
	int ret = 0;
	uint8_t reg_buf[2] = {0};
	struct spi_message msg;
	struct spi_transfer transfer[2];

	spi_message_init(&msg);
	memset(&transfer, 0, sizeof(transfer));

	reg_buf[0] = ((SPI_WRITE_MASK(reg_addr) & 0xff00) >> 8);
	reg_buf[1] = (SPI_WRITE_MASK(reg_addr) & 0xff);

	transfer[0].tx_buf = reg_buf;
	transfer[0].len = 2;
	transfer[0].speed_hz = speed_hz;

	transfer[1].tx_buf = data;
	transfer[1].len = (uint32_t)len;
	transfer[1].speed_hz = speed_hz;

	spi_message_add_tail(&transfer[0], &msg);
	spi_message_add_tail(&transfer[1], &msg);

	ret = spi_sync(tof_data->device, &msg);
	if (ret)
		XRISP_PR_ERROR("spi_sync error, ret=%d.", ret);

	return ret;
}

static int tof_spi_read(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
			uint8_t *data, uint16_t len, uint32_t speed_hz)
{
	int ret = 0;
	uint8_t reg_buf[2] = {0};
	struct spi_message msg;
	struct spi_transfer transfer[2];

	spi_message_init(&msg);
	memset(&transfer, 0, sizeof(transfer));

	reg_buf[0] = ((SPI_READ_MASK(reg_addr) >> 8) & 0xff);
	reg_buf[1] = (SPI_READ_MASK(reg_addr) & 0xff);

	transfer[0].tx_buf = reg_buf;
	transfer[0].len = 2;
	transfer[0].speed_hz = speed_hz;

	transfer[1].rx_buf = data;
	transfer[1].len = (uint32_t)len;
	transfer[1].speed_hz = speed_hz;

	spi_message_add_tail(&transfer[0], &msg);
	spi_message_add_tail(&transfer[1], &msg);

	ret = spi_sync(tof_data->device, &msg);
	if (ret)
		XRISP_PR_ERROR("spi_sync error, ret=%d.", ret);

	return ret;
}
#else
static int tof_i2c_write(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
			uint8_t *data, uint16_t len)
{
	int ret = 0;
	uint8_t *buffer = NULL;
	struct i2c_msg msg;

	buffer = kzalloc(len + 2, GFP_KERNEL);
	if (!buffer) {
		XRISP_PR_ERROR("Failed to allocate memory");
		return -ENOMEM;
	}

	buffer[0] = (reg_addr >> 8) & 0xFF;
	buffer[1] = reg_addr & 0xFF;
	memcpy(buffer + 2, data, len);

	msg.addr = tof_data->device->addr;
	msg.flags = tof_data->device->flags;
	msg.buf = buffer;
	msg.len = len + 2;

	ret = i2c_transfer(tof_data->device->adapter, &msg, 1);
	if (ret != 1)
		XRISP_PR_ERROR("i2c transfer error, ret=%d.", ret);

	kfree(buffer);
	return ret < 0 ? ret : (ret != 1 ? -EIO : 0);
}

static int tof_i2c_read(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
			uint8_t *data, uint16_t len)
{
	int ret = 0;
	uint8_t buffer[2] = {0};
	struct i2c_msg msg[2];

	buffer[0] = (reg_addr >> 8) & 0xFF;
	buffer[1] = reg_addr & 0xFF;

	msg[0].addr = tof_data->device->addr;
	msg[0].flags = tof_data->device->flags;
	msg[0].buf = buffer;
	msg[0].len = 2;

	msg[1].addr = tof_data->device->addr;
	msg[1].flags = I2C_M_RD | tof_data->device->flags;
	msg[1].buf = data;
	msg[1].len = len;

	ret = i2c_transfer(tof_data->device->adapter, msg, 2);
	if (ret != 2)
		XRISP_PR_ERROR("i2c transfer error, ret=%d.", ret);

	return ret < 0 ? ret : (ret != 2 ? -EIO : 0);
}
#endif

int tof_write_byte(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
			uint8_t reg_val)
{
#ifdef XRISP_TOF_SPI_MODE
	return tof_spi_write(tof_data, reg_addr, &reg_val, 1,
			tof_data->transfer_speed_hz);
#else
	return tof_i2c_write(tof_data, reg_addr, &reg_val, 1);
#endif
}

int tof_read_byte(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
			uint8_t *reg_val)
{
#ifdef XRISP_TOF_SPI_MODE
	return tof_spi_read(tof_data, reg_addr, reg_val, 1,
			tof_data->transfer_speed_hz);
#else
	return tof_i2c_read(tof_data, reg_addr, reg_val, 1);
#endif
}

int tof_write_block(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
			uint8_t *data, uint32_t count)
{
	int ret = 0;
	uint16_t blocks = 0;
	uint16_t bytes = 0;
	uint16_t offset = 0;

	blocks = count / XRISP_TOF_BLOCK_SIZE + ((count % XRISP_TOF_BLOCK_SIZE) != 0);
	for (uint16_t i = 0; i < blocks; i++) {
		bytes = (i + 1) * XRISP_TOF_BLOCK_SIZE > count ?
			(count - i * XRISP_TOF_BLOCK_SIZE) : XRISP_TOF_BLOCK_SIZE;
		offset = i * XRISP_TOF_BLOCK_SIZE;
#ifdef XRISP_TOF_SPI_MODE
		ret = tof_spi_write(tof_data, reg_addr + offset, data + offset, bytes,
			tof_data->transfer_speed_hz);
#else
		ret = tof_i2c_write(tof_data, reg_addr + offset, data + offset, bytes);
#endif
		if (ret) {
			XRISP_PR_ERROR("read block error, ret=%d.", ret);
			break;
		}
	}

	return ret;
}

int tof_read_block(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
			uint8_t *data, uint32_t count)
{
	int ret = 0;
	uint16_t blocks = 0;
	uint16_t bytes = 0;
	uint16_t offset = 0;

	blocks = count / XRISP_TOF_BLOCK_SIZE + ((count % XRISP_TOF_BLOCK_SIZE) != 0);
	for (uint16_t i = 0; i < blocks; i++) {
		bytes = (i + 1) * XRISP_TOF_BLOCK_SIZE > count ?
			(count - i * XRISP_TOF_BLOCK_SIZE) : XRISP_TOF_BLOCK_SIZE;
		offset = i * XRISP_TOF_BLOCK_SIZE;
#ifdef XRISP_TOF_SPI_MODE
		ret = tof_spi_read(tof_data, reg_addr + offset, data + offset, bytes,
			tof_data->transfer_speed_hz);
#else
		ret = tof_i2c_read(tof_data, reg_addr + offset, data + offset, bytes);
#endif
		if (ret) {
			XRISP_PR_ERROR("read block error, ret=%d.", ret);
			break;
		}
	}

	return ret;
}

int tof_get_tick_count(uint32_t *time_ms)
{
	*time_ms = jiffies_to_msecs(jiffies);
	return 0;
}

int tof_check_for_timeout(uint32_t start_ms, uint32_t end_ms, uint32_t timeout)
{
	int ret = 0;
	unsigned int diff_ms;

	if (start_ms <= end_ms)
		diff_ms = end_ms - start_ms;
	else
		diff_ms = ~start_ms + 0x01 + end_ms;

	ret = diff_ms <= timeout ? 0 : 1;
	return ret;
}
