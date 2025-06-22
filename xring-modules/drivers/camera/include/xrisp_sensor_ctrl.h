/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc.
 */

#ifndef XRISP_SENSOR_CTRL_H
#define XRISP_SENSOR_CTRL_H

#include <dt-bindings/xring/platform-specific/xrisp.h>

#define CAM_SENSOR_NUM_MAX  8

int xrisp_sensor_power_up(struct xrisp_sensor_power_ctrl *ctrl);
int xrisp_sensor_power_down(struct xrisp_sensor_power_ctrl *ctrl);
int xrisp_sensor_power_down_force(uint32_t sensor_id, enum cam_power_type type);

int xrisp_sensor_init(void);
void xrisp_sensor_exit(void);

int xrisp_sensor_ois_complete(uint32_t sensor_id);
int xrisp_parklens_complete(uint32_t sensor_id);

int xrisp_notify_shub_ois_power_down(void);

#endif /* XRISP_SENSOR_CTRL_H */
