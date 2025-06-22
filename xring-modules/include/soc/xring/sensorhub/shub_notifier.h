/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#ifndef __SHUB_NOTIFIER_H__
#define __SHUB_NOTIFIER_H__
#include <linux/notifier.h>

#define ACTION_SH_RESET                 (0x1)
#define ACTION_SH_RECOVERY_OS_READY     (0x2)
#define ACTION_SH_RECOVERY_ALL_READY    (0x3)

int sh_reset_notifier_register(struct notifier_block *nb);
int sh_reset_notifier_unregister(struct notifier_block *nb);
int sh_recovery_notifier_register(struct notifier_block *nb);
int sh_recovery_notifier_unregister(struct notifier_block *nb);
int sh_reset_notifier_call_chain(unsigned long value);
int sh_recovery_notifier_call_chain(unsigned long value);
int shub_boot_atomic_notifier_register(struct notifier_block *nb);
int shub_boot_atomic_notifier_unregister(struct notifier_block *nb);
int shub_boot_atomic_notifier_call_chain(unsigned long value);

#endif /* __SHUB_NOTIFIER_H__ */
