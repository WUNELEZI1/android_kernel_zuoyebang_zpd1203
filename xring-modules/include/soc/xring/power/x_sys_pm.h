/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#ifndef __X_SYS_PM_H__
#define __X_SYS_PM_H__

/* priority of notifier block is an integer type,the default is PRIO_NOTIFIER_DEFAULT
 * x_sys_pm use PRIO_NOTIFIER_DEFAULT
 */
#define PRIO_NOTIFIER_SYS_PM		-1	/* only reserved for x_sys_pm */
#define PRIO_NOTIFIER_DEFAULT		0x0	/* other modules use */

#endif
