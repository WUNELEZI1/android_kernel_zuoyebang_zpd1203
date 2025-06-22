// SPDX-License-Identifier: GPL-2.0-only
/*
 * hello demo module.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>


int func1(void)
{
	pr_info("export FUNC1\n");
	return 0;
}
EXPORT_SYMBOL(func1);


static int hello_init(void)
{
	pr_info("hello world!\n");
	return 0;
}
static void hello_exit(void)
{
	pr_info("good bye, kernel\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("Dual BSD/GPL");
