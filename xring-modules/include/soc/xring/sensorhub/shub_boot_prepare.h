/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#ifndef __SHUB_BOOT_PREPARE_H__
#define __SHUB_BOOT_PREPARE_H__

#include <dt-bindings/xring/platform-specific/sensorhub_dts.h>
#include <linux/debugfs.h>
#include <linux/notifier.h>

struct sh_bsp_dts *shub_get_bsp_dts(void);
struct sh_app_dts *shub_get_app_dts(void);
bool is_shub_boot_done(void);
bool is_shub_dts_enable(void);
int sh_boot_state_notifier_register(struct notifier_block *nb);
int sh_boot_state_notifier_unregister(struct notifier_block *nb);
int sh_boot_state_notifier_call_chain(unsigned long value);
#endif /* __SHUB_BOOT_PREPARE_H__ */
