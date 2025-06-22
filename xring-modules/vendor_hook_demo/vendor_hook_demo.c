// SPDX-License-Identifier: GPL-2.0-only
/*
 * vendor hook demo module.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <trace/hooks/sched.h>

static void android_rvh_get_nohz_timer_target_handler(void *unused, int *cpu, bool *done)
{
	*cpu = 0;
	*done = 1;
}
static int demo_init(void)
{
	int ret = 1;

	pr_info("ret:%d\n", ret);
	ret = register_trace_android_rvh_get_nohz_timer_target(android_rvh_get_nohz_timer_target_handler, NULL);
	pr_info("demo ret:%d!\n", ret);
	return 0;
}

static void demo_exit(void)
{
	pr_info("demo bye!\n");
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_LICENSE("GPL");
