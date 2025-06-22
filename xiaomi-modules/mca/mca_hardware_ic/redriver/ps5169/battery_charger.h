// SPDX-License-Identifier: GPL-2.0
/*
 *battery_charger.h
 *
 * battery_charger driver
 *
 * Copyright (c) 2024-2024 Xiaomi Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */


#ifndef _BATTERY_CHARGER_H
#define _BATTERY_CHARGER_H

#include <linux/notifier.h>

#define CONFIG_MI_ENABLE_DP
//#define CONFIG_ADD_PS5169_REDRIVER

enum battery_charger_prop {
	BATTERY_RESISTANCE,
#if defined(CONFIG_MI_ENABLE_DP)
	USB_CC_ORIENTATION,
	HAS_DP_PS5169,
#endif
	BATTERY_CHARGER_PROP_MAX,
};

enum bc_hboost_event {
	VMAX_CLAMP,
};

#if IS_ENABLED(CONFIG_QTI_BATTERY_CHARGER)
int qti_battery_charger_get_prop(const char *name,
				enum battery_charger_prop prop_id, int *val);
int register_hboost_event_notifier(struct notifier_block *nb);
int unregister_hboost_event_notifier(struct notifier_block *nb);
#else
static inline int
qti_battery_charger_get_prop(const char *name,
				enum battery_charger_prop prop_id, int *val)
{
	return -EINVAL;
}

static inline int register_hboost_event_notifier(struct notifier_block *nb)
{
	return -EOPNOTSUPP;
}

static inline int unregister_hboost_event_notifier(struct notifier_block *nb)
{
	return -EOPNOTSUPP;
}
#endif

#endif
