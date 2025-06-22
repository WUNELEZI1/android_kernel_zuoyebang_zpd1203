/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_CAM_TOF_H_
#define _XRISP_CAM_TOF_H_
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "xrisp_log.h"

#define XRISP_TOF_SPI_MODE

union mcu_register_status_0 {
	uint8_t bytes;
	struct {
		uint8_t mcu_boot_complete : 1;
		uint8_t mcu_analog_checks_ok : 1;
		uint8_t mcu_threshold_triggered : 1;
		uint8_t mcu_error_flag: 1;
		uint8_t mcu_ui_range_data_present : 1;
		uint8_t mcu_ui_new_range_data_avail : 1;
		uint8_t mcu_ui_update_blocked : 1;
		uint8_t mcu_hw_trap_flag : 1;
	};
};

union mcu_register_status_1 {
	uint8_t bytes;
	struct {
		uint8_t mcu_avdd_reg_ok : 1;
		uint8_t mcu_pll_lock_ok : 1;
		uint8_t mcu_ls_watchdog_pass : 1;
		uint8_t mcu_warning_flag : 1;
		uint8_t mcu_cp_collapse_flag : 1;
		uint8_t mcu_spare0 : 1;
		uint8_t mcu_initial_ram_boot_complete : 1;
		uint8_t mcu_spare1 : 1;
	};
};

enum xrisp_tof_power_mode {
	XRISP_TOF_POWER_MODE_OFF = 0,
	XRISP_TOF_POWER_MODE_ULP = 1,
	XRISP_TOF_POWER_MODE_LP = 2,
	XRISP_TOF_POWER_MODE_HP = 3,
	XRISP_TOF_POWER_MODE_RANGING = 4
};

struct xrisp_tof_firmware {
	const char *fw_name;
	bool fw_loaded;
	bool device_booted;
	uint8_t device_id;
	uint8_t revision_id;
	union mcu_register_status_0 mcu_reg0;
	union mcu_register_status_1 mcu_reg1;
};

struct xrisp_tof_data {
#ifdef XRISP_TOF_SPI_MODE
	struct spi_device *device;
#else
	struct i2c_client *device;
#endif
	struct miscdevice miscdev;
	struct xrisp_tof_firmware firmware;
	struct mutex tof_mutex;
	struct regulator *regulator_1V2;
	struct regulator *regulator_1V8;
	struct regulator *regulator_3V3;
	enum xrisp_tof_power_mode power_mode;
	uint32_t power_refs;
	uint32_t transfer_speed_hz;
	int inter_gpio;
	int inter_irq;
};

int tof_write_byte(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
		uint8_t reg_val);
int tof_read_byte(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
		uint8_t *reg_val);
int tof_write_block(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
		uint8_t *data, uint32_t count);
int tof_read_block(struct xrisp_tof_data *tof_data, uint16_t reg_addr,
		uint8_t *data, uint32_t count);

int tof_get_tick_count(uint32_t *time_ms);
int tof_check_for_timeout(uint32_t start_ms, uint32_t end_ms, uint32_t timeout);

int tof_read_chip_info(struct xrisp_tof_data *tof_data);
int tof_set_power_mode(struct xrisp_tof_data *tof_data,
		enum xrisp_tof_power_mode mode);
int check_rom_firmware_boot(struct xrisp_tof_data *tof_data);
int tof_load_firmware(struct xrisp_tof_data *tof_data);

#endif
