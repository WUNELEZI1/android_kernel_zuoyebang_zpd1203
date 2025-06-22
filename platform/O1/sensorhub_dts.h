// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __SENSORHUB_DTS_H__
#define __SENSORHUB_DTS_H__

#if defined(__KERNEL__) && !defined(__NuttX__)
#include <linux/stddef.h>
#else
#include <stddef.h>
#endif

#define SH_EXCLUDED_TYPE_NUM 25
#define SH_I3C_MASTER_NUM    8
#define SH_DRIVERID_LEN 15
#define SH_UART_NUM 6
#define SH_AOC_SEQ_MAX      16
#define SH_AOC_SUB_LDO_MAX  10
#define SH_AOC_GPIO_MAX     5
#define SH_AOC_NAME_LEN     12

enum sensor_index {
	SH_SENSOR_AMG_INDEX = 0,
	SH_SENSOR_MAG_INDEX = 1,
	SH_SENSOR_ALS_INDEX = 2,
	SH_SENSOR_FLICKER_INDEX = 3,
	SH_SENSOR_BAR_INDEX = 4,
	SH_SENSOR_SAR_INDEX = 5,
	SH_SENSOR_TOF_INDEX = 6,
	SH_SENSOR_HALL_INDEX = 7,
	SH_SENSOR_TOUCH_INDEX = 8,
	SH_SENSOR_OIS_INDEX = 9,
	SH_SENSOR_MAX_INDEX
};

enum sensor_bus_type {
	SH_BUS_I2C = 0,
	SH_BUS_I3C = 1,
	SH_BUS_SPI = 2,
	SH_BUS_TYPE_MAX
};



struct sh_i3c_master {
	uint16_t i3c_master_id;
	uint16_t i3c_slave_num;
	uint16_t i2c_slave_num;
	uint16_t reserved;
};

struct sh_dts_i3c_info {
	uint16_t i3c_master_num;
	uint16_t reserved[3];
	struct sh_i3c_master i3c_master[SH_I3C_MASTER_NUM];
};


struct sh_uart {
	uint16_t tx_gpio;
	uint16_t rx_gpio;
	uint32_t baud;
	uint16_t is_enable;
	uint16_t reserved[3];
};

struct sh_dts_uart_info {
	uint16_t uart_num;
	uint16_t reserved[3];
	struct sh_uart uart[SH_UART_NUM];
};

struct sh_dts_conn_info {
	uint16_t is_conn_exist;
	uint16_t is_wcn_exist;
	uint16_t is_gnss_exist;
	uint16_t is_modem_exist;
};

struct sh_dts_sensor_common_info {
	uint16_t bus_speed_khz;
	uint16_t chip_id_addr;
	uint8_t chip_id_val;
	uint8_t bus_type;
	uint8_t bus_num;
	uint8_t slave_address;
	uint8_t driver_id[SH_DRIVERID_LEN];
	uint8_t index;
};

struct sh_dts_amg_info {
	struct sh_dts_sensor_common_info common_info;
	uint64_t i3c_device_id_pid;
	uint8_t i3c_device_id_flags;
	uint8_t i3c_device_id_dcr;

	uint8_t orientation[3];
	uint8_t irq_num;
	uint8_t resv[2];
};

struct sh_dts_mag_info {
	struct sh_dts_sensor_common_info common_info;

	uint8_t orientation[3];
	uint8_t resv[5];
};

struct sh_dts_ois_info {
	struct sh_dts_sensor_common_info common_info;
	uint16_t bus_speed_khz_x;
	uint16_t bus_speed_khz_y;
	uint8_t bus_type_x;
	uint8_t bus_num_x;
	uint8_t slave_address_x;
	uint8_t bus_type_y;
	uint8_t bus_num_y;
	uint8_t slave_address_y;

	uint8_t orientation[3];
	uint8_t resv[3];
};

struct sh_dts_hall_info {
	struct sh_dts_sensor_common_info common_info;
	uint8_t gpio_n;
	uint8_t gpio_s;
	uint8_t resv[6];
};

struct sh_dts_touch_info {
	struct sh_dts_sensor_common_info common_info;
	uint8_t irq_gpio;
	uint8_t cs_gpio;
	uint8_t spi_mode;
	uint8_t spi_bits_per_word;
	uint8_t resv[4];
};

struct sh_dts_kernel_time_sync_info {
	uint64_t kernel_syscount_ns;
	uint64_t shub_timestamp_ns;
	uint64_t utc_time_ns;
};
struct cam_power_seq_s {
	uint32_t   seq_type;
	uint32_t   config_val;
	uint32_t   delay_ms;
};
struct mpmic_rgltr_config_s {
	uint16_t type;
	uint8_t  ldo_name[SH_AOC_NAME_LEN];
	uint16_t vset_addr;
	uint16_t vset_read_addr;
	uint16_t power_en_addr;
	uint16_t power_ramp_addr;
	uint16_t regval_def;
	uint16_t voltval_def_mv;
	uint16_t vset_step_mv;
	uint16_t voltval_min_mv;
	uint16_t voltval_max_mv;
	uint16_t voltval_set_mv;
};
struct mpmic_param_s {
	uint16_t pwr_en_read_bit_shift;
	uint16_t pwr_en_bit_shift;
	uint16_t vset_valid_addr;
	uint16_t rampup_valid_addr;
	uint16_t rampdn_valid_addr;
};
struct subldo_info_s {
	uint8_t  sub_ldo_name[SH_AOC_NAME_LEN];
	uint16_t type;
	uint16_t ldo_num;
	uint16_t ldo_regaddr;
	uint32_t ldo_volt_mv;
};
struct wldo_info_s {
	uint8_t     wldo_name[SH_AOC_NAME_LEN];
	uint8_t     i2c_mst_name[SH_AOC_NAME_LEN];
	uint16_t    i2c_mst_port;
	uint16_t    wldo_chipid;
	uint16_t    wldo_chipid_addr;
	uint16_t    wldo_en_addr;
	uint16_t    wldo_slave_addr;
	uint32_t    en_gpio;
	uint32_t    wldo_i2c_freq;
	uint32_t    delay_after_rst;
	uint16_t    sub_ldo_num;
	uint16_t    type;
};
struct cam_wldo_info_s {
	struct wldo_info_s   wldo_info;
	struct subldo_info_s subldo_info[SH_AOC_SUB_LDO_MAX];
};
struct csi_clk_info_s {
	uint16_t   csi_clk_id;
	uint32_t   csi_clk_normal;
	uint32_t   csi_clk_low;
};
struct clk_io_config_s {
	uint8_t    name[SH_AOC_NAME_LEN];
	uint32_t   id;
	uint32_t   gpio_num;
	uint32_t   clk_rate;
	uint8_t    is_osc_clk;
};
struct gpio_config_s {
	uint8_t     name[SH_AOC_NAME_LEN];
	uint32_t    gpio_num;
	uint16_t    mode;
	uint8_t     is_low_def;
};
struct cam_power_io_s {
	struct clk_io_config_s gpio_mclk;
	struct gpio_config_s   gpios[SH_AOC_GPIO_MAX];
	uint32_t               gpio_count;
};
struct cam_power_map_s {
	uint32_t  power_seq;
	uint32_t  power_type;
	union {
		uint8_t     sub_ldo_name[SH_AOC_NAME_LEN];
		uint32_t    gpio_num;
	};
};
struct mpmic_rgltr_dev_s {
	struct mpmic_rgltr_config_s mpmic;
	uint8_t                     is_mpmic_used;
	uint32_t                    mpmic_num;
};
struct cam_map_s {
	struct cam_power_map_s power_map[SH_AOC_SEQ_MAX];
	uint32_t               map_num;
};
struct cam_seq_s {
	struct cam_power_seq_s cam_seq[SH_AOC_SEQ_MAX];
	uint32_t               seq_num;
};
struct sh_dts_aoc_cam_power_info {
	struct mpmic_rgltr_dev_s mpmic_dev;
	struct mpmic_param_s     mpmic_param;
	struct cam_wldo_info_s   wldo_dev;
	struct cam_power_io_s    cam_io;
	struct csi_clk_info_s    csi_clk_info;
	struct cam_map_s         cam_power_map;
	struct cam_seq_s         cam_power_seq[SH_AOC_SEQ_MAX];
	uint8_t                  sensor_type[SH_AOC_NAME_LEN];
};

struct sh_bsp_dts {

	uint32_t bsp_dts_size;
	struct sh_dts_i3c_info i3c_info;
	struct sh_dts_conn_info conn_info;
	uint8_t wdt_switch;
	struct sh_dts_kernel_time_sync_info time_sync_info;
	struct sh_dts_uart_info uart_info;
	struct sh_dts_aoc_cam_power_info aoc_power_info;
};

struct sh_dts_virtualsensor_info {
	uint32_t excluded_types[SH_EXCLUDED_TYPE_NUM];
};

struct sh_app_dts {
	uint32_t app_dts_start;
	uint32_t app_dts_size;
	struct sh_dts_virtualsensor_info virtualsensor_info;
	uint32_t thp_sub_enabled;

};

#endif
