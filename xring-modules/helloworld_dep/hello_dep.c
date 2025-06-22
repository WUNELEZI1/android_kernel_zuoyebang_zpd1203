// SPDX-License-Identifier: GPL-2.0-only
/*
 * hello_dep demo module.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

extern int func1(void);

static int hello_dep_init(void)
{
	pr_info("hello world dep!\n");
	func1();
	return 0;
}
static void hello_dep_exit(void)
{
	pr_info("good bye, kernel dep\n");
}

module_init(hello_dep_init);
module_exit(hello_dep_exit);

MODULE_LICENSE("Dual BSD/GPL");
