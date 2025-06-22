// SPDX-License-Identifier: GPL-2.0-or-later
//
// Copyright (c) 2024 Xring.
#ifndef __XR_PMIC_H__
#define __XR_PMIC_H__

#include <linux/regulator/consumer.h>

int xr_pmic_gpio_set_value(int gpio, int value);

int xr_pmic_gpio_direction_output(int gpio, int value);

/*
 * set the buckboost FPWM(Force PWM Mode) enable
 *
 * @bob_regulator:  the buckboost regulator consumer
 * @en:             1: enable FPWM mode
 *                  0: disable FPWM mode
 *
 */
int xr_bob_fpwm_set_enable(struct regulator *bob_regulator, bool en);
#endif /* __XR_PMIC_H__ */
