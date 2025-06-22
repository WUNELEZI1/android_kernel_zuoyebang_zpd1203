// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/module.h>
#include <linux/sched.h>

#include <trace/hooks/sched.h>

#include "walt_debug.h"

static void android_rvh_schedule_bug(void *unused, void *unused2)
{
	WARN_ON(1);
}

static int __init walt_debug_init(void)
{
	int ret;

	ret = preemptirq_long_init();
	if (ret)
		return ret;

	register_trace_android_rvh_schedule_bug(android_rvh_schedule_bug, NULL);

	return 0;
}
module_init(walt_debug_init);

MODULE_DESCRIPTION("WALT Debug Module");
MODULE_LICENSE("GPL v2");
