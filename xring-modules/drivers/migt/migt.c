// SPDX-License-Identifier: GPL-2.0-only
/*
 * hello demo module.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>


static int hello_init(void)
{
	return 0;
}
static void hello_exit(void)
{

}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("Dual BSD/GPL");
