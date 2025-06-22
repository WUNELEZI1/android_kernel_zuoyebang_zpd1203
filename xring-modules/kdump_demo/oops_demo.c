// SPDX-License-Identifier: GPL-2.0-only
/*
 * msleep trigger softlockup demo
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>



static int __init oops_demo_init(void)
{
	int *p = NULL;
	*p = 1;
	return 0;
}

static void __exit oops_demo_exit(void)
{
}

module_init(oops_demo_init);
module_exit(oops_demo_exit);

MODULE_LICENSE("GPL");
