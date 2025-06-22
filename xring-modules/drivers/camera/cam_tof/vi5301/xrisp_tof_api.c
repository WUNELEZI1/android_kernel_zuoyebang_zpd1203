// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "tof_vi5301_api", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "tof_vi5301_api", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/time.h>
#include<linux/string.h>
#include <linux/firmware.h>
#include <soc/xring/xr_timestamp.h>
#include "xrisp_tof.h"
#include "xrisp_log.h"

#define STATUS_TOF_CONFIDENT 0
#define STATUS_TOF_SEMI_CONFIDENT 6
#define STATUS_TOF_NOT_CONFIDENT 7
#define STATUS_TOF_NO_OBJECT 255

#define XRISP_TOF_FIRMWARE_NUM 8192
#define XRISP_TOF_FIRMWARE_NAME "com.xring.cam_tof.vi5301_v3_20241108.bin"

union inte_data {
	uint32_t intecnts;
	uint8_t buf[4];
};

static int load_firmware(struct xrisp_tof_data *dev, uint8_t *fw_buf)
{
	int ret = 0;
	int fw_size;
	const struct firmware *vi5301_firmware;

	ret = request_firmware(&vi5301_firmware, XRISP_TOF_FIRMWARE_NAME, &dev->client->dev);
	if (ret || !vi5301_firmware) {
		XRISP_PR_ERROR("Firmware request failed!");
		return ret;
	}
	fw_size = (int)vi5301_firmware->size;
	XRISP_PR_INFO("vi5301_firmware size: %d bytes", fw_size);

	memcpy(fw_buf, vi5301_firmware->data, vi5301_firmware->size);
	release_firmware(vi5301_firmware);
	return fw_size;
}

static int read_fw_version(struct xrisp_tof_data *dev)
{
	int ret = 0;
	uint8_t fw_version[4];

	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0, 0x06);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_USER_CFG_CMD);
	if (ret)
		goto exit;
	mdelay(5);
	ret = xrisp_tof_read_multibytes(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, fw_version, 4);
	if (ret)
		goto exit;
	XRISP_PR_INFO("fw version: %#x %#x %#x %#x", fw_version[0], fw_version[1],
			fw_version[2], fw_version[3]);
exit:
	return ret;
}

static int read_chip_version(struct xrisp_tof_data *dev)
{
	int ret = 0;
	uint8_t buf[3] = {0};
	uint32_t chip_version = 0;

	ret = xrisp_tof_read_multibytes(dev, XRISP_TOF_REG_CHIPID_BASE, buf, 3);
	if (ret)
		return ret;
	chip_version = (buf[1] << 16) + (buf[0] << 8) + buf[2];
	XRISP_PR_INFO("VI5301 chip_version: 0x%x", chip_version);
	return ret;
}

int debug_dump_reg(struct xrisp_tof_data *dev)
{
	int ret = 0;
	uint8_t buff[0x4B];

	ret = xrisp_tof_read_multibytes(dev, 0x00, buff, 0x4B);
	if (ret)
		return ret;
	for (int i = 0; i < 0x4B / 5; i++)
		XRISP_PR_INFO(
"reg[0x%X]=0x%X, reg[0x%X]=0x%X, reg[0x%X]=0x%X, reg[0x%X]=0x%X, reg[0x%X]=0x%X,",
		i * 5, buff[i * 5],
		i * 5 + 1, buff[i * 5 + 1],
		i * 5 + 2, buff[i * 5 + 2],
		i * 5 + 3, buff[i * 5 + 3],
		i * 5 + 4, buff[i * 5 + 4]);

	return ret;
}

static int wait_for_cpu_ready(struct xrisp_tof_data *dev)
{
	int ret = 0;
	int retry = 0;
	uint8_t cpu_status;

	do {
		mdelay(1);
		ret = xrisp_tof_read_byte(dev, XRISP_TOF_REG_DEV_STAT, &cpu_status);
		if (ret)
			return ret;
	} while ((retry++ < XRISP_TOF_MAX_WAIT_RETRY) && (cpu_status & 0x01));

	if (retry >= XRISP_TOF_MAX_WAIT_RETRY) {
		XRISP_PR_ERROR("CPU is busy, cpu_status = %d", cpu_status);
		(void)debug_dump_reg(dev);
		ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_PW_CTRL, 0x0E);
		if (ret)
			return ret;
		mdelay(2);
		ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_PW_CTRL, 0x0F);
		if (ret)
			return ret;
		mdelay(2);

		retry = 0;
		do {
			mdelay(1);
			ret = xrisp_tof_read_byte(dev, XRISP_TOF_REG_DEV_STAT, &cpu_status);
			if (ret)
				return ret;
		} while ((retry++ < XRISP_TOF_MAX_WAIT_RETRY) && (cpu_status & 0x01));

		if (retry >= XRISP_TOF_MAX_WAIT_RETRY) {
			XRISP_PR_ERROR("CPU is busy, cpu_status = %d", cpu_status);
			(void)debug_dump_reg(dev);
			ret = -EFAULT;
			return ret;
		}
	}

	return ret;
}

static int init_firmware(struct xrisp_tof_data *dev)
{
	int ret = 0;
	int fw_size = 0;
	uint16_t fw_send = 0;
	uint8_t  sys_cfg_data = 0;
	uint8_t val;
	uint8_t *fw_buf;

	fw_buf = kzalloc(XRISP_TOF_FIRMWARE_NUM, GFP_KERNEL);
	if (!fw_buf) {
		XRISP_PR_ERROR("Failed to alloc memory.");
		return -ENOMEM;
	}

	fw_size = load_firmware(dev, fw_buf);
	if (fw_size <= 0) {
		XRISP_PR_ERROR("Firmware load failed!");
		ret = -1;
		goto exit;
	}

	XRISP_PR_INFO("Firmware load begin:%s", XRISP_TOF_FIRMWARE_NAME);
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_PW_CTRL, 0x08);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_PW_CTRL, 0x0a);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_MCU_CFG, 0x06);
	if (ret)
		goto exit;
	ret = xrisp_tof_read_byte(dev, XRISP_TOF_REG_SYS_CFG, &sys_cfg_data);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_SYS_CFG, sys_cfg_data | (0x01 << 0));
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, 0x01);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_SIZE, 0x02);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0, 0x0);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x01, 0x0);
	if (ret)
		goto exit;

	while (fw_size >= 32) {
		ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_CMD, 0, XRISP_TOF_WRITEFW_CMD);
		if (ret)
			goto exit;
		ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SIZE, 0, 0x20);
		if (ret)
			goto exit;
		ret = xrisp_tof_write_multibytes(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE,
									fw_buf + fw_send*32, 32);
		if (ret)
			goto exit;

		udelay(10);
		fw_send += 1;
		fw_size -= 32;
	}

	if (fw_size > 0) {
		ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_CMD, 0, XRISP_TOF_WRITEFW_CMD);
		if (ret)
			goto exit;
		ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SIZE, 0, (uint8_t)fw_size);
		if (ret)
			goto exit;
		ret = xrisp_tof_write_multibytes(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE,
				fw_buf + fw_send*32, fw_size);
		if (ret)
			goto exit;
	}

	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_SYS_CFG, sys_cfg_data & ~(0x01 << 0));
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_MCU_CFG, 0x06);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_PD_RESET, 0xA0);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_PD_RESET, 0x80);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_MCU_CFG, 0x07);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_PW_CTRL, 0x02);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_PW_CTRL, 0x00);
	if (ret)
		goto exit;

	mdelay(5);
	ret = xrisp_tof_read_byte(dev, XRISP_TOF_REG_SPCIAL_PURP, &val);
	if (ret || (val != 0x66)) {
		XRISP_PR_ERROR("Load firmware failed, value=%d", val);
		ret = -1;
	}

exit:
	kfree(fw_buf);
	return ret;
}

static int interrupt_enable(struct xrisp_tof_data *dev)
{
	int ret = 0;
	int loop = 0;
	uint8_t enable = 0;

	do {
		ret = xrisp_tof_read_byte(dev, XRISP_TOF_REG_INTR_MASK, &enable);
		if (ret)
			goto exit;

		enable |=  0x01;
		ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_INTR_MASK, enable);
		if (ret)
			goto exit;
		ret = xrisp_tof_read_byte(dev, XRISP_TOF_REG_INTR_MASK, &enable);
		if (ret)
			goto exit;

		loop++;
	} while ((loop < XRISP_TOF_MAX_WAIT_RETRY)
		&& (!(enable & 0x01)));

	if (loop >= XRISP_TOF_MAX_WAIT_RETRY) {
		XRISP_PR_ERROR("Enable interrupt failed, ret=%d.", ret);
		ret = -1;
	}

exit:
	return ret;
}

__maybe_unused static int integral_counts_write(struct xrisp_tof_data *dev, uint32_t inte_counts)
{
	int ret = 0;
	union inte_data intedata = {0};

	intedata.intecnts = inte_counts;

	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0, 0x01);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 1, 0x03);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 2, 0x14);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 3, intedata.buf[0]);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 4, intedata.buf[1]);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 5, intedata.buf[2]);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_USER_CFG_CMD);
	if (ret)
		goto exit;

	mdelay(5);
exit:
	return ret;
}

__maybe_unused static int delay_count_write(struct xrisp_tof_data *dev, uint16_t delay_count)
{
	int ret = 0;
	union delay_data {
		uint16_t delay;
		uint8_t buf[2];
	} delaydata;

	delaydata.delay = delay_count;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0, 0x01);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 1, 0x02);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 2, 0x17);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 3, delaydata.buf[0]);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 4, delaydata.buf[1]);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_USER_CFG_CMD);
	if (ret)
		goto exit;

	mdelay(5);
exit:
	return ret;
}

__maybe_unused static int set_integralcounts_frame(struct xrisp_tof_data *dev, uint8_t fps,
						   uint32_t intecoutns)
{
	int ret = 0;
	uint32_t inte_time = 0;
	uint32_t fps_time = 0;
	uint32_t delay_time = 0;
	uint16_t delay_counts = 0;

	inte_time = intecoutns * 1463/10;
	fps_time = 1000000000/fps;
	delay_time = fps_time - inte_time - 5200000;
	delay_counts = (uint16_t)(delay_time/3400);
	ret = integral_counts_write(dev, intecoutns);
	if (ret) {
		XRISP_PR_ERROR("integral counts write failed!");
		goto exit;
	}

	ret = delay_count_write(dev, delay_counts);
	if (ret) {
		XRISP_PR_ERROR("delay count write failed!");
		goto exit;
	}

exit:
	return ret;
}

static int temperature_enable(struct xrisp_tof_data *dev, uint8_t enable)
{
	int ret = 0;

	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0, 0x01);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x01, 0x01);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x02, 0x24);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x03, enable);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_USER_CFG_CMD);
	if (ret)
		goto exit;

	mdelay(5);
exit:
	return ret;
}

static int get_ma_window_data(struct xrisp_tof_data *dev, uint8_t *sum_ma)
{
	int ret = 0;
	uint8_t ma_val[8];

	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x00, 0x00);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x01, 0x08);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x02, 0x1A);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_USER_CFG_CMD);
	if (ret)
		goto exit;

	mdelay(5);
	ret = xrisp_tof_read_multibytes(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, ma_val, 8);
	if (ret) {
		XRISP_PR_ERROR("Get ma window failed, ret=%d.", ret);
		goto exit;
	}

	for (int i = 0; i < 8; i++)
		*sum_ma += ((ma_val[i] & 0x0F)+((ma_val[i] >> 4) & 0x0F));

exit:
	return ret;
}

static int32_t calculate_pileup_bias(struct xrisp_tof_data *dev, uint32_t peak,
				uint32_t noise, uint32_t integral_times)
{
	uint32_t peak_tmp = 0;
	const int16_t xth1[] = { 0, 500, 800, 1128, 1734, 2000 };
	const int16_t pth1[] = { -15, -4, 0, 8, 25, 35 };
	const int16_t xth2[] = { 0, 160, 350, 700, 1200, 2000 };
	const int16_t pth2[] = { -6, -4, 0, 13, 40, 70 };
	const int16_t *xth, *pth;
	int32_t bias = 0;
	int len = 0;
	uint8_t i = 0;

	if (noise > 8000) {
		xth = xth2;
		pth = pth2;
		len = ARRAY_SIZE(xth2);
	} else {
		xth = xth1;
		pth = pth1;
		len = ARRAY_SIZE(xth1);
	}

	if (integral_times == 0)
		return bias;

	noise = noise / 8;
	if (peak > noise * dev->ma_sum)
		peak_tmp = (peak - noise * dev->ma_sum) * 16 / integral_times;

	for (i = 0; i < len  - 1; i++) {
		if (peak_tmp < xth[i + 1]) {
			bias = (int32_t)(pth[i + 1] - pth[i]) * (int32_t)(peak_tmp - xth[i]) /
				(int32_t)(xth[i + 1] - xth[i]) + (int32_t)pth[i];
			return bias;
		}
	}
	if (peak_tmp >= xth[i])
		bias = (int32_t)(pth[i] - pth[i - 1]) * (int32_t)(peak_tmp - xth[i-1]) /
			(int32_t)(xth[i] - xth[i - 1])+(int32_t)pth[i - 1];

	return bias;
}

static uint32_t calculate_confidence(struct xrisp_tof_data *dev, uint32_t peak,
				uint32_t noise, uint32_t integral_times)
{
	uint32_t confidence = 0;
	int64_t noise_r = 0;
	int64_t peak_r = 0;
	int64_t lower = 0;
	int64_t upper = 0;
	int i;
	int len;
	const int32_t xth[] = {4, 32, 114, 175, 313, 482, 539, 657, 1472, 2421, 3223, 6777, 7217,
			12326, 14946, 20906, 25976, 32287, 41121, 44258, 51439, 56032, 80216};
	const int32_t ylower1[] = {4, 7, 16, 22, 34, 49, 54, 66, 136, 211, 279, 566, 600, 1025,
			1221, 1682, 2086, 2559, 3336, 3779, 4338, 4796, 6806};
	const int32_t yupper1[] = {7, 9, 20, 28, 43, 60, 66, 80, 162, 243, 321, 630, 666, 1138,
			1338, 1828, 2266, 2743, 3542, 3995, 4580, 5050, 7171};
	const int32_t ylower2[] = {4, 7, 16, 22, 34, 49, 54, 66, 136, 211, 279, 566, 600, 1025,
			1221, 1682, 1986, 2309, 3036, 3500, 4338, 4896, 5706};
	const int32_t yupper2[] = {7, 9, 20, 28, 43, 60, 66, 80, 162, 243, 321, 630, 666, 1138,
			1338, 1828, 2116, 2499, 3442, 3686, 4480, 5050, 6871};
	const int32_t *ylower, *yupper;

	len = ARRAY_SIZE(xth);

	if (noise > 8000) {
		ylower = ylower2;
		yupper = yupper2;
	} else {
		ylower = ylower1;
		yupper = yupper1;
	}

	peak_r = (int64_t)peak * 1024 * 100 / integral_times;
	noise_r = (int64_t)noise * 131072 / integral_times;
	for (i = 0; i < (len - 1); i++) {
		if (noise_r < xth[i + 1]) {
			lower = 100 * (int64_t)(ylower[i + 1] - ylower[i]) * (int64_t)abs(noise_r -
				xth[i]) / (int64_t)(xth[i + 1] - xth[i]) + 100 * (int64_t)ylower[i];
			upper = 100 * (int64_t)(yupper[i + 1] - yupper[i]) * (int64_t)abs(noise_r -
				xth[i]) / (int64_t)(xth[i + 1] - xth[i]) + 100 * (int64_t)yupper[i];
			break;
		} else if (noise_r >= xth[len - 1]) {
			lower = 100 * (int64_t)(ylower[len - 1] - ylower[len - 2]) *
				(int64_t)abs(noise_r - xth[len - 2]) / (int64_t)(xth[len - 1] -
				xth[len - 2]) + 100 * (int64_t)ylower[len - 2];
			upper = 100 * (int64_t)(yupper[len - 1] - yupper[len - 2]) *
				(int64_t)abs(noise_r - xth[len - 2]) / (int64_t)(xth[len - 1] -
				xth[len - 2]) + 100 * (int64_t)yupper[len - 2];
			break;
		}
	}
	if (peak_r < lower)
		confidence = 0;
	else if (peak_r > upper)
		confidence = 100;
	else
		confidence = 100 * (peak_r - lower) / (upper - lower);

	return confidence;
}

int xrisp_tof_set_period(struct xrisp_tof_data *dev, uint32_t period)
{
	int ret = 0;
	static uint32_t last_period;
	uint32_t inte_time = 0;
	uint32_t fps_time = 0;
	uint32_t delay_time = 0;
	uint16_t delay_counts = 0;
	union inte_data pdata = {0};
	union delay_data {
		uint16_t delay;
		uint8_t buf[2];
	} delaydata;

	if (period == last_period)
		return ret;
	last_period = period;

	if (period == 0) {
		ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0, 0x00);
		if (ret)
			goto error;
		ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 1, 0x02);
		if (ret)
			goto error;
		ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 2, 0x17);
		if (ret)
			goto error;
		ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_USER_CFG_CMD);
		if (ret)
			goto error;

		mdelay(5);
		ret = xrisp_tof_read_multibytes(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, delaydata.buf, 2);
		if (ret)
			goto error;
	} else {
		ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0, 0x00);
		if (ret)
			goto error;
		ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 1, 0x03);
		if (ret)
			goto error;
		ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 2, 0x14);
		if (ret)
			goto error;
		ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_USER_CFG_CMD);
		if (ret)
			goto error;

		mdelay(5);
		ret = xrisp_tof_read_multibytes(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, pdata.buf, 3);
		if (ret)
			goto error;

		inte_time = pdata.intecnts*1463/10;
		fps_time = 1000000000 / period;
		delay_time = fps_time - inte_time - 5200000;
		delay_counts = (uint16_t)(delay_time/3400);
		delaydata.delay = delay_counts;
	}

	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0, 0x01);
	if (ret)
		goto error;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 1, 0x02);
	if (ret)
		goto error;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 2, 0x17);
	if (ret)
		goto error;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 3, delaydata.buf[0]);
	if (ret)
		goto error;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 4, delaydata.buf[1]);
	if (ret)
		goto error;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_USER_CFG_CMD);
	if (ret)
		goto error;

	mdelay(5);
	XRISP_PR_INFO("set period=%d Hz.", period);

	return ret;

error:
	XRISP_PR_ERROR("set period failed.");
	return ret;
}

int xrisp_tof_single_measure(struct xrisp_tof_data *dev)
{
	int ret = 0;

	ret = wait_for_cpu_ready(dev);
	if (ret) {
		XRISP_PR_ERROR("CPU status is busy, ret=%d.", ret);
		return ret;
	}

	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_SINGLE_RANGE_CMD);
	if (ret) {
		XRISP_PR_ERROR("Single measure failed, ret=%d", ret);
		return ret;
	}

	return ret;
}

int xrisp_tof_start_continuous_measure(struct xrisp_tof_data *dev)
{
	int ret = 0;

	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_STOP_RANGE_CMD);
	if (ret) {
		XRISP_PR_ERROR("Stop range before start failed, ret=%d.", ret);
		return ret;
	}

	ret = wait_for_cpu_ready(dev);
	if (ret) {
		XRISP_PR_ERROR("CPU status is busy, ret=%d.", ret);
		return ret;
	}

	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_CONTINOUS_RANGE_CMD);
	if (ret) {
		XRISP_PR_ERROR("Start continuous measure failed, ret=%d.", ret);
		return ret;
	}
	XRISP_PR_INFO("Start Ranging.");

	return ret;
}

int xrisp_tof_stop_continuous_measure(struct xrisp_tof_data *dev)
{
	int ret = 0;

	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_STOP_RANGE_CMD);
	if (ret) {
		XRISP_PR_ERROR("Stop continuous measure failed, ret=%d.", ret);
		return ret;
	}
	XRISP_PR_INFO("Stop Ranging.");

	return ret;
}

int xrisp_tof_get_measure_data(struct xrisp_tof_data *dev)
{
	int ret = 0;
	uint8_t buf[32];
	int16_t millimeter = 0;
	uint32_t noise = 0;
	uint32_t ambient_rate = 0;
	uint32_t peak1 = 0, peak2 = 0;
	uint32_t peak = 0;
	uint32_t integral_times = 0;
	int32_t bias = 0;
	uint32_t confidence = 0;
	uint32_t ratio = 0;
	int16_t tof1 = 0, tof2 = 0;
	uint32_t tmp_peak = 0, tmp_bin = 0;
	uint8_t tof1_bin = 0, tof2_bin = 0;
	uint8_t reftof_bin = 0, flag = 0;
	int8_t xtalk_bin = 0;

	ret = xrisp_tof_read_multibytes(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, buf, 32);
	if (ret) {
		XRISP_PR_ERROR("Get range data failed, ret=%d.", ret);
		return ret;
	}

	tof1_bin = buf[17];
	tof2_bin = buf[0];
	reftof_bin = buf[6];
	tof2 = ((int16_t)buf[2] << 8) | ((int16_t)buf[1]);   // *((int16_t *)(buf + 1));
	tof1 = ((int16_t)buf[13] << 8) | ((int16_t)buf[12]); // *((int16_t *)(buf + 12));
	integral_times = (((uint32_t)buf[24]) << 16) | (((uint32_t)buf[23]) << 8) |
		((uint32_t)buf[22]);
	peak2 = (((uint32_t)buf[11]) << 24) | (((uint32_t)buf[10]) << 16) |
		(((uint32_t)buf[9]) << 8) | ((uint32_t)buf[8]); // *((uint32_t *)(buf + 8));
	peak1 = (((uint32_t)buf[31]) << 24) | (((uint32_t)buf[30]) << 16) |
		(((uint32_t)buf[29]) << 8) | ((uint32_t)buf[28]); // *((uint32_t *)(buf + 28));
	integral_times = integral_times & 0x00ffffff;
	noise = (((uint32_t)buf[27]) << 16) | (((uint32_t)buf[26]) << 8) |
		((uint32_t)buf[25]); // *((uint32_t *)(buf + 25));
	noise = noise & 0x00ffffff;
	ambient_rate = (((uint32_t)buf[16]) << 16) | (((uint32_t)buf[15]) << 8) |
		((uint32_t)buf[14]);

	xtalk_bin = (int8_t)reftof_bin + dev->calib_data.xtalk_cal;
	if (tof1_bin <= xtalk_bin + 2) {
		if (peak1 >= 3000 * dev->ma_sum) {
			millimeter = tof1;
			peak = peak1;
			tmp_peak = peak2;
			tmp_bin = tof2_bin;
		} else {
			millimeter = tof2;
			peak = peak2;
			tmp_peak = peak1;
			tmp_bin = tof1_bin;
		}
	} else {
		millimeter = tof1;
		peak = peak1;
		tmp_peak = peak2;
		tmp_bin = tof2_bin;
	}

	bias = calculate_pileup_bias(dev, peak, noise, integral_times);
	confidence = calculate_confidence(dev, peak, noise, integral_times);

	if (tof1 > 3500)
		confidence = 0;
	ratio = 100 * noise * dev->ma_sum / peak;
	if (noise > 90000 && buf[3] <= 50 && ratio < 850)
		confidence = 0;

	millimeter = millimeter + (int16_t)bias;
	millimeter = millimeter - (int16_t)dev->calib_data.offset_cal;
	if (millimeter < 0)
		confidence = 0;

	if (millimeter >= 300) {
		if ((tmp_peak > 10000) && (tmp_bin >= xtalk_bin - 2) && (tmp_bin <= xtalk_bin + 2))
			flag = 1;
	}

	mutex_lock(&dev->range_mutex);
	dev->range_data.distance = millimeter;
	dev->range_data.peak = peak;
	dev->range_data.confidence = confidence;
	dev->range_data.noise = noise;
	dev->range_data.ambient_rate = ambient_rate;
	dev->range_data.integral_times = integral_times;
	dev->range_data.time_ns = xr_timestamp_gettime();
	mutex_unlock(&dev->range_mutex);
	if (dev->range_data.time_ns == 0) {
		XRISP_PR_ERROR("get timestamp failed.");
		return -1;
	}

	if (dev->enable_debug)
		XRISP_PR_INFO(
"distance=%d, confidence=%d, peak=%u, noise=%u, integral_times=%u, bias=%d, ambient_rate=%d, flag=%d, time_ns=%lld",
millimeter, confidence, peak, noise, integral_times, bias, ambient_rate, flag, dev->range_data.time_ns);

	return ret;
}

int xrisp_tof_get_interrupt_state(struct xrisp_tof_data *dev)
{
	int ret = 0;
	uint8_t state;

	ret = xrisp_tof_read_byte(dev, XRISP_TOF_REG_INTR_STAT, &state);
	if (ret || !(state & 0x01)) {
		XRISP_PR_ERROR("Get interrupt state failed, ret=%d.", ret);
		return -1;
	}

	return ret;
}

int xrisp_tof_chip_init(struct xrisp_tof_data *dev)
{
	int ret = 0;

	ret = read_chip_version(dev);
	if (ret) {
		XRISP_PR_ERROR("Read chip version failed.");
		return ret;
	}

	ret = wait_for_cpu_ready(dev);
	if (ret) {
		XRISP_PR_ERROR("CPU status is busy.");
		return ret;
	}

	ret = interrupt_enable(dev);
	if (ret) {
		XRISP_PR_ERROR("Clear interrupt mask failed.");
		return ret;
	}

	ret = init_firmware(dev);
	if (ret) {
		XRISP_PR_ERROR("Load firmware failed.");
		return ret;
	}

	ret = get_ma_window_data(dev, &dev->ma_sum);
	if (ret) {
		XRISP_PR_ERROR("Get ma_sum failed.");
		return ret;
	}

	ret = read_fw_version(dev);
	if (ret) {
		XRISP_PR_ERROR("Read firmware version failed.");
		return ret;
	}

	return ret;
}

int xrisp_tof_xtalk_calibration(struct xrisp_tof_data *dev)
{
	int ret = 0;

	ret = wait_for_cpu_ready(dev);
	if (ret) {
		XRISP_PR_ERROR("CPU status is busy, ret=%d.", ret);
		return ret;
	}

	ret = temperature_enable(dev, 0);
	if (ret) {
		XRISP_PR_ERROR("Temperature enable failed, ret=%d.", ret);
		return ret;
	}

	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_XTALK_TRIM_CMD);
	if (ret) {
		XRISP_PR_ERROR("Xtalk calibration failed, ret=%d.", ret);
		return ret;
	}

	return ret;
}

int xrisp_tof_offset_calibration(struct xrisp_tof_data *dev)
{
	int ret = 0;
	uint8_t buf[32];
	uint32_t noise = 0;
	uint32_t peak1 = 0, peak2 = 0;
	uint32_t peak = 0;
	uint32_t integral_times = 0;
	int32_t bias = 0;
	int16_t tof1 = 0, tof2 = 0;
	int16_t millimeter = 0;
	int16_t total = 0;
	int16_t offset = 0;
	int cnt = 0;
	uint8_t stat = 0;

	ret = temperature_enable(dev, 0);
	if (ret) {
		XRISP_PR_ERROR("Temperature enable failed.");
		return ret;
	}
	ret = xrisp_tof_start_continuous_measure(dev);
	if (ret) {
		XRISP_PR_ERROR("Offset calibtration start failed, ret=%d.", ret);
		goto exit;
	}

	while (1) {
		msleep(35);
		ret = xrisp_tof_read_byte(dev, XRISP_TOF_REG_INTR_STAT, &stat);
		if (!ret) {
			if ((stat & 0x01) == 0x01) {
				ret = xrisp_tof_read_multibytes(dev,
						XRISP_TOF_REG_SCRATCH_PAD_BASE, buf, 32);
				if (ret) {
					XRISP_PR_ERROR("Read range data failed.");
					goto exit;
				}

				tof2 = ((int16_t)buf[2] << 8) | ((int16_t)buf[1]);
				tof1 = ((int16_t)buf[13] << 8) | ((int16_t)buf[12]);
				integral_times = (((uint32_t)buf[24]) << 16) |
						(((uint32_t)buf[23]) << 8) | ((uint32_t)buf[22]);
				peak2 = (((uint32_t)buf[11]) << 24) | (((uint32_t)buf[10]) << 16) |
						(((uint32_t)buf[9]) << 8) | ((uint32_t)buf[8]);
				peak1 = (((uint32_t)buf[31]) << 24) | (((uint32_t)buf[30]) << 16) |
						(((uint32_t)buf[29]) << 8) | ((uint32_t)buf[28]);
				integral_times = integral_times & 0x00ffffff;
				noise = (((uint32_t)buf[27]) << 16) | (((uint32_t)buf[26]) << 8) |
						((uint32_t)buf[25]);
				noise = noise & 0x00ffffff;
				if (tof1 <= 35) {
					if (peak1 >= 10000 * dev->ma_sum) {
						millimeter = tof1;
						peak = peak1;
					} else {
						millimeter = tof2;
						peak = peak2;
					}
				} else {
					millimeter = tof1;
					peak = peak1;
				}
				bias = calculate_pileup_bias(dev, peak, noise, integral_times);
				millimeter = millimeter + (int16_t)bias;
				total += millimeter;
				++cnt;
			} else
				continue;
		} else {
			XRISP_PR_ERROR("Failed to get interrupt state.");
			goto exit;
		}
		if (cnt >= 30)
			break;
	}

	ret = xrisp_tof_stop_continuous_measure(dev);
	if (ret) {
		XRISP_PR_ERROR("Offset calibtration stop failed.");
		goto exit;
	}

	offset = total / 30;
	dev->calib_data.offset_cal = offset - dev->calib_data.offset_mili;
	if (dev->enable_debug)
		XRISP_PR_INFO("offset mili: %d, offset cal: %d\n", dev->calib_data.offset_mili,
				dev->calib_data.offset_cal);

exit:
	(void)temperature_enable(dev, 1);
	return ret;
}

int xrisp_tof_get_xtalk_parameter(struct xrisp_tof_data *dev)
{
	int ret = 0;
	uint8_t val;
	uint8_t cg_buf[5];

	ret = xrisp_tof_read_byte(dev, XRISP_TOF_REG_SPCIAL_PURP, &val);
	if (ret == 0 && val == 0xaa) {
		ret = xrisp_tof_read_multibytes(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, cg_buf, 4);
		if (ret) {
			XRISP_PR_ERROR("Read Xtalk parameter failed, ret=%d.", ret);
			goto exit;
		}
		dev->calib_data.xtalk_cal = *((int8_t *)(cg_buf + 0));
		dev->calib_data.xtalk_peak = *((uint16_t *)(cg_buf + 1));
		dev->calib_data.maxratio = *((uint8_t *)(cg_buf + 3));
	} else
		XRISP_PR_ERROR("Xtalk calibration failed, ret=%d, val=0x%02x.", ret, val);

exit:
	(void)temperature_enable(dev, 1);
	return ret;
}

int xrisp_tof_set_xtalk_parameter(struct xrisp_tof_data *dev)
{
	int ret = 0;

	ret = wait_for_cpu_ready(dev);
	if (ret) {
		XRISP_PR_ERROR("CPU status is busy, ret=%d.", ret);
		return ret;
	}

	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0, 0x01);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x01, 0x01);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x02, 0x26);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x03,
			*((uint8_t *)(&dev->calib_data.xtalk_cal)));
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_USER_CFG_CMD);
	if (ret)
		goto exit;

	mdelay(5);
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0, 0x01);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x01, 0x01);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x02, 0x25);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_reg_offset(dev, XRISP_TOF_REG_SCRATCH_PAD_BASE, 0x03,
			dev->calib_data.maxratio);
	if (ret)
		goto exit;
	ret = xrisp_tof_write_byte(dev, XRISP_TOF_REG_CMD, XRISP_TOF_USER_CFG_CMD);
	if (ret)
		goto exit;

	mdelay(5);
exit:
	if (ret)
		XRISP_PR_ERROR("Set Xtalk failed, ret=%d.", ret);
	return ret;
}
