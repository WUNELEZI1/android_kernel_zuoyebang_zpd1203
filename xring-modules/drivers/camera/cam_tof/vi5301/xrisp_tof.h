/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRISP_CAM_TOF_H__
#define __XRISP_CAM_TOF_H__

#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/completion.h>
#include <linux/regulator/consumer.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>

#define XRISP_TOF_CHIP_ADDR 0xD8

#define XRISP_TOF_REG_MCU_CFG 0x00
#define XRISP_TOF_REG_SYS_CFG 0x01
#define XRISP_TOF_REG_DEV_STAT 0x02
#define XRISP_TOF_REG_INTR_STAT 0x03
#define XRISP_TOF_REG_INTR_MASK 0x04
#define XRISP_TOF_REG_I2C_IDLE_TIME 0x05
#define XRISP_TOF_REG_DEV_ADDR 0x06
#define XRISP_TOF_REG_PW_CTRL 0x07
#define XRISP_TOF_REG_SPCIAL_PURP 0x08
#define XRISP_TOF_REG_CMD 0x0A
#define XRISP_TOF_REG_SIZE 0x0B
#define XRISP_TOF_REG_SCRATCH_PAD_BASE 0x0C
#define XRISP_TOF_REG_CHIPID_BASE 0x2C
#define XRISP_TOF_REG_RCO_AO 0x37
#define XRISP_TOF_REG_DIGLDO_VREF 0x38
#define XRISP_TOF_REG_PLLLDO_VREF 0x39
#define XRISP_TOF_REG_ANALDO_VREF 0x3A
#define XRISP_TOF_REG_PD_RESET 0x3B
#define XRISP_TOF_REG_I2C_STOP_DELAY 0x3C
#define XRISP_TOF_REG_TRIM_MODE 0x3D
#define XRISP_TOF_REG_GPIO_SINGLE 0x3E
#define XRISP_TOF_REG_ANA_TEST_SINGLE 0x3F

#define XRISP_TOF_WRITEFW_CMD 0x03
#define XRISP_TOF_USER_CFG_CMD 0x09
#define XRISP_TOF_XTALK_TRIM_CMD 0x0D
#define XRISP_TOF_SINGLE_RANGE_CMD 0x0E
#define XRISP_TOF_CONTINOUS_RANGE_CMD 0x0F
#define XRISP_TOF_STOP_RANGE_CMD 0x01F

#define XRISP_TOF_OTPW_SUBCMD 0x02
#define XRISP_TOF_OTPR_SUBCMD 0x03
#define XRISP_TOF_MAX_WAIT_RETRY 5
#define XRISP_TOF_DEFAULT_INTEGRAL_COUNTS 131072
#define XRISP_TOF_DEFAULT_FRAME_COUNTS 30


struct xrisp_tof_data {
	const char *dev_name;
	struct xrisp_cam_tof_range_data range_data;
	struct xrisp_cam_tof_calib_data calib_data;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct miscdevice miscdev;
	struct regulator *dev_regulator;
	struct mutex work_mutex;
	struct work_struct tof_work;
	struct completion is_ready;
	struct mutex range_mutex;
	int xtalk_mark;
	int offset_mark;
	int inter_gpio;
	int xshut_gpio;
	int irq;
	bool fw_loaded;
	uint8_t ma_sum;
	uint32_t chip_enable;
	uint32_t enable_debug;
	uint32_t period;
	uint32_t power_refs;
};

int xrisp_tof_write_byte(struct xrisp_tof_data *dev, uint8_t reg, uint8_t data);
int xrisp_tof_read_byte(struct xrisp_tof_data *dev, uint8_t reg, uint8_t *data);
int xrisp_tof_write_multibytes(struct xrisp_tof_data *dev, uint8_t reg,
		uint8_t *data, int32_t count);
int xrisp_tof_read_multibytes(struct xrisp_tof_data *dev, uint8_t reg,
		uint8_t *data, int32_t count);
int xrisp_tof_write_reg_offset(struct xrisp_tof_data *dev, uint8_t reg,
		uint8_t offset, uint8_t data);
int xrisp_tof_read_reg_offset(struct xrisp_tof_data *dev, uint8_t reg,
		uint8_t offset, uint8_t *data);

int xrisp_tof_set_period(struct xrisp_tof_data *dev, uint32_t period);
int xrisp_tof_single_measure(struct xrisp_tof_data *dev);
int xrisp_tof_start_continuous_measure(struct xrisp_tof_data *dev);
int xrisp_tof_stop_continuous_measure(struct xrisp_tof_data *dev);
int xrisp_tof_get_measure_data(struct xrisp_tof_data *dev);
int xrisp_tof_get_interrupt_state(struct xrisp_tof_data *dev);
int xrisp_tof_chip_init(struct xrisp_tof_data *dev);
int xrisp_tof_xtalk_calibration(struct xrisp_tof_data *dev);
int xrisp_tof_offset_calibration(struct xrisp_tof_data *dev);
int xrisp_tof_get_xtalk_parameter(struct xrisp_tof_data *dev);
int xrisp_tof_set_xtalk_parameter(struct xrisp_tof_data *dev);
int debug_dump_reg(struct xrisp_tof_data *dev);

#endif
