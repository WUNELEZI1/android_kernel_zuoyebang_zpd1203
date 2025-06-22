/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_FLASH_CORE_H_
#define _XRISP_FLASH_CORE_H_
#include <dt-bindings/xring/platform-specific/xrflash.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/i2c.h>

/*
 * Enable specific lamp numbering for debugging
 */
enum flash_led_flag {
	XRISP_FLASH_LED1 = 0,
	XRISP_FLASH_LED2,
	XRISP_FLASH_LED,
};

enum flash_current_opration_flag {
	FLASH_LED1_CURR = 0,
	FLASH_LED2_CURR,
	TORCH_LED1_CURR,
	TORCH_LED2_CURR,
};

enum flash_mode_flag {
	XRISP_FLASH_MODE = 0,
	XRISP_TORCH_MODE,
};

enum flash_led_opration_flag {
	XRISP_FLASH_ENABLE = 0,
	XRISP_FLASH_DISABLE,
};

struct xrisp_flash_core_ops {
	int (*leds_init)(struct device *dev);
	int (*leds_query_current)(struct device *dev,
			struct xrisp_flash_querry_current_cmd *querry_current);
	int (*leds_deinit)(struct device *dev);
	int (*leds_enable)(struct device *dev, uint32_t led1Cur, uint32_t led2Cur);
	int (*leds_torch_enable)(struct device *dev, uint32_t led1Cur, uint32_t led2Cur);
	int (*leds_disable)(struct device *dev);
	int (*leds_duration_valid)(int flash_time);

	// sys debug
	int (*debug_leds_read)(struct device *dev, int *val);
	int (*debug_leds_power_up)(struct device *dev);
	int (*debug_leds_power_down)(struct device *dev, bool force_power_down_flag);
	int (*debug_leds_determ_led_num)(struct device *dev);
	int (*debug_leds_get_use_num)(struct device *dev, enum flash_led_flag led_num);
	int (*debug_leds_flash_enable)(struct device *dev, const int en, enum flash_led_flag led_flag);
	int (*debug_get_leds_torch_enable_num)(int en, int val, enum flash_led_flag led_flag);
	int (*debug_leds_torch_enable)(struct device *dev, int val);
	int (*debug_leds_get_curr)(struct device *dev, enum flash_mode_flag mode_flag,
			enum flash_current_opration_flag current_opration_flag);
	int (*debug_leds_set_curr)(struct device *dev, int curr, enum flash_mode_flag mode_flag,
			enum flash_current_opration_flag current_opration_flag);
	int (*debug_leds_timeout)(struct device *dev, int dura);
};

struct xrisp_flash_core {
	struct device *dev;
	struct xrisp_flash_core_ops *ops;
	struct xrisp_flash_connect_cmd *flash_connect_cmd;
	struct hrtimer xrisp_flash_hrtimer;
	struct completion flash_flag;
	bool flash_flag_finish;
	struct kobject *isp_kobj;
	struct kobject *flash_kobj;
};

int flash_core_init(struct device *dev, struct xrisp_flash_core_ops *flash_core_ops);
void flash_core_deinit(void);
int xrisp_flash_parse_command(struct xrisp_flash_connect_cmd *flash_cmd);

int flash_sys_node_init(void);
void flash_sys_node_exit(void);
void xrisp_flash_close(void);
#endif
