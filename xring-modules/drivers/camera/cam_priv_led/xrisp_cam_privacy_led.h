/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_CAM_PRIVACY_LED_H_
#define _XRISP_CAM_PRIVACY_LED_H_

#include <linux/mutex.h>
#include <linux/atomic.h>

struct privacy_led_node {
	struct kobject *isp_kobj;
	struct kobject *priv_led_kobj;
};
struct privacy_led_dev {
	struct device             *dev;
	struct mutex              mlock;
	atomic_t                  power_cnt;
	struct privacy_led_node   node;
	struct pinctrl            *pinctrl;
};

int xrisp_privacy_reminder_led_enable(void);
int xrisp_privacy_reminder_led_disable(void);

#endif
