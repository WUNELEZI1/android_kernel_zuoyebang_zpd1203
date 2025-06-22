/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc.
 */

#ifndef XRISP_SENSOR_UTILS_H
#define XRISP_SENSOR_UTILS_H

#include "xrisp_log.h"

#define SENSOR_PINCTRL_STATE_SLEEP  "cam_sleep"
#define SENSOR_PINCTRL_STATE_ACTIVE "cam_active"
#define SENSOR_PINCTRL_CCI_STATE_SLEEP  "cci_sleep"
#define SENSOR_PINCTRL_CCI_STATE_ACTIVE "cci_active"
#define SENSOR_PINCTRL_CCI_STATE_SLEEP_1  "cci_sleep_1"
#define SENSOR_PINCTRL_CCI_STATE_ACTIVE_1 "cci_active_1"

#define CAM_MAX_REGULATOR 16
#define CAM_MAX_CLK       8
#define INVALID_VREG      100
#define RGLTR_BYPASS      2
#define RGLTR_CTRL        1
#define RGLTR_NO_CTRL     0

#define XRISP_CHECK_NULL_RETURN_INT(_ptr) (((_ptr) == NULL) ? true : false)

enum cam_vote_level {
	CAM_SUSPEND_VOTE,
	CAM_LOW_VOTE,
	CAM_NORMAL_VOTE,
	CAM_TURBO_VOTE,
	CAM_MAX_VOTE,
};

struct cam_seq_name {
	const char *name;
	uint32_t   seq_type;
};

#define MAX_POWER_TYPE 8
struct cam_hw_power_setting {
	uint32_t seq_type;
	uint32_t seq_delay;
	uint32_t seq_val;
};

struct cam_power_ctrl {
	uint32_t count;
	struct cam_hw_power_setting *power_settings;
};

struct cam_power_ctrl_tab {
	struct cam_power_ctrl pu_settings[MAX_POWER_TYPE];
	struct cam_power_ctrl pd_settings[MAX_POWER_TYPE];
};

struct cam_gpio_info {
	struct gpio  *gpio_tbl;
	uint8_t      gpio_tbl_size;
	struct gpio  *gpio_req_tbl;
	uint8_t      gpio_req_tbl_size;
};

struct cam_pinctrl_info {
	struct pinctrl *pinctrl;
	struct pinctrl_state *gpio_state_active;
	struct pinctrl_state *gpio_state_suspend;
	struct pinctrl_state *gpio_state_cci_active;
	struct pinctrl_state *gpio_state_cci_suspend;
};

struct cam_hw_info {
	uint32_t                       cell_idx;

	uint32_t                       num_rgltr;
	const char                     *rgltr_name[CAM_MAX_REGULATOR];
	uint32_t                       rgltr_ctrl_support;
	const char                     *rgltr_nd_name[CAM_MAX_REGULATOR];
	uint32_t                       rgltr_min_volt[CAM_MAX_REGULATOR];
	uint32_t                       rgltr_max_volt[CAM_MAX_REGULATOR];
	uint32_t                       rgltr_op_mode[CAM_MAX_REGULATOR];
	uint32_t                       rgltr_type[CAM_MAX_REGULATOR];
	struct regulator               *rgltr[CAM_MAX_REGULATOR];
	uint32_t                       rgltr_delay[CAM_MAX_REGULATOR];

	uint32_t                       num_clk;
	const char                     *clk_name[CAM_MAX_CLK];
	struct clk                     *clk[CAM_MAX_CLK];
	int32_t                        clk_rate[CAM_MAX_VOTE][CAM_MAX_CLK];
	bool                           clk_level_valid[CAM_MAX_VOTE];

	struct cam_gpio_info           gpio_info;
	struct cam_pinctrl_info        pinctrl_info;
	struct cam_power_ctrl_tab      *power_tab;
	bool                           is_aoc_cam;
	bool                           has_aoc;
};

void xtisp_sensor_util_msleep(uint32_t ms);
int xrisp_sensor_util_get_dt_regulator_info(struct device *dev, struct cam_hw_info *hw_info);
int xrisp_sensor_util_get_dt_clk_info(struct device *dev, struct cam_hw_info *hw_info);
int xrisp_sensor_util_get_dt_gpio_info(struct device *dev, struct cam_hw_info *hw_info);
int xrisp_sensor_util_get_clk_resource(struct device *dev, struct cam_hw_info *hw_info);
int xrisp_sensor_util_get_regulator_resource(struct device *dev, struct cam_hw_info *hw_info);
int xrisp_sensor_util_get_pinctrl_resource(struct device *dev, struct cam_hw_info *hw_info);

int xrisp_sensor_util_clk_set(struct cam_hw_info *hw_info, const char *name,
		uint32_t enable, int level);
int xrisp_sensor_util_regulator_set(struct regulator *rgltr,
	const char *rgltr_name,
	uint32_t rgltr_min_volt,
	uint32_t rgltr_max_volt,
	uint32_t rgltr_op_mode,
	uint32_t rgltr_delay_ms,
	uint32_t enable);
int xrisp_sensor_util_regulator_set_by_name(struct cam_hw_info *hw_info,
	const char *name, uint32_t enable);

#endif /* XRISP_SENSOR_UTILS_H */
