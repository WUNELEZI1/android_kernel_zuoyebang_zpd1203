/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc.
 */

#ifndef XRISP_SENSOR_DEV_H
#define XRISP_SENSOR_DEV_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/completion.h>
#include "xrisp_sensor_utils.h"
#include "xrisp_sensor_ctrl.h"
#include <dt-bindings/xring/platform-specific/xrisp.h>

#define PARKLENS_RETURN (0x1)

struct cam_power_ctrl_gpio {
	uint16_t gpio_num[SENSOR_SEQ_TYPE_MAX];
	uint8_t  valid[SENSOR_SEQ_TYPE_MAX];
};

struct cam_power_setting {
	enum xr_camera_power_seq_type seq_type;
	unsigned short seq_val;
	long config_val;
	unsigned short delay;
};

struct cam_power_ctrl_record {
	atomic_t power_cnt;
	int count;
	struct cam_power_setting *power_settings;
};

struct cam_power_ctrl_ois {
	bool wait;
	uint32_t sensor_id;
	struct completion complete;
};

struct cam_parklens_priv_t {
	uint8_t power_settings_cnt;
	uint32_t type;
	struct cam_pm_dev *cam_pm;
	struct cam_power_setting *power_settings;
};

enum parklens_states {
	PARKLENS_INVALID,   //init
	PARKLENS_RUN,	    //parklens thread create
	PARKLENS_POWERDOWN, //sensor power down start
	PARKLENS_STOP,	    //parklens thread stop
};

struct cam_parklens_ctrl_t {
	struct task_struct *thread;
	struct cam_parklens_priv_t priv;
	uint8_t sensor_id;
	struct completion complete;
	enum parklens_states state;
	struct mutex lock;
};

struct cam_power_ctrl_info {
	struct cam_power_ctrl_gpio gpio_seq;
	struct cam_power_ctrl_record power_record[POWER_TYPE_MAX];
	struct cam_power_ctrl_ois ois;
	struct cam_parklens_ctrl_t parklens;
};

struct cam_power_sync_info {
	struct device        *dev;
	void __iomem         *baseaddr;
	struct mutex         mlock;
	atomic_t             power_cnt[XRISP_AOC_POWER_SYNC_BIT_MAX];
};

struct cam_pm_dev {
	struct platform_device     *pdev;
	struct device              *dev;
	struct cam_hw_info         *hw_info;
	struct cam_power_ctrl_info power_info;
	uint32_t                   id;   //sensor_id
	struct mutex               mlock;
	struct dentry              *dentry;
	struct cam_power_sync_info *sync_info;
};

void xrisp_sensor_debugfs_exit(struct cam_pm_dev *cam_pm);
int xrisp_sensor_debugfs_init(struct cam_pm_dev *cam_pm);
struct cam_pm_dev *xrisp_sensor_get_context(uint32_t idx);
bool xrisp_sensor_get_has_aoc_status(void);

void xrisp_sensor_ois_init(struct cam_pm_dev *cam_pm);

int xrisp_sensor_core_power_down(struct cam_hw_info *hw_info, struct cam_power_ctrl_gpio *gpio_seq,
				 uint32_t power_setting_count,
				 struct cam_power_setting *power_settings, uint32_t force);

void xrisp_parklens_init(struct cam_pm_dev *cam_pm);
void xrisp_parklens_exit(struct cam_pm_dev *cam_pm);
void xrisp_parklens_state_reset(struct cam_parklens_ctrl_t *parklens_s);
int xrisp_parklens_sensor_power_up(struct cam_parklens_ctrl_t *parklens_s);
int xrisp_parklens_sensor_power_down(struct cam_parklens_ctrl_t *parklens_s,
				     struct xrisp_sensor_power_ctrl *ctrl,
				     struct cam_pm_dev *cam_pm,
				     struct cam_power_setting *power_settings);

int xrisp_power_sync_set(uint8_t status, uint8_t bitmask);
int xrisp_power_sync_get(uint8_t bitmask);
int xrisp_power_sync_clear(uint8_t bitmask);
int xrisp_power_sync_clear_force(uint8_t bitmask);
int xrisp_power_sync_init(struct device *dev);
int xrisp_power_sync_exit(void);

#endif /* XRISP_SENSOR_DEV_H */
