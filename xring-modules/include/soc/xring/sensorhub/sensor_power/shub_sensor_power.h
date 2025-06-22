/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 * Description: sensor power
 * Modify time: 2024-05-20
 */

void sensor_power_on_all(void);

void sensor_power_off_all(void);

int sensor_power_on(int index);

int sensor_power_off(int index);

/* set it up to see who is using the power, null is ok */
void sensor_power_set_dev(struct device *dev);

void sensor_power_init(void);

void sensor_power_deinit(void);

const char *sensor_power_get_names(void);
